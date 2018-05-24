#include <kernel.h>
#include <stdlib.h>
#include <tamtypes.h>
#include <math3d.h>

#include <packet.h>

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <dma.h>

#include <graph.h>

#include <draw.h>
#include <draw3d.h>

//#include "Controller.c" TODO: I must have messed something up in controller, fix it later

#include <Compilation.h>

// The mesh to load
#include "Mesh/mesh_data.c"

/*
 * Things to know:
 * - z-buffer is a buffer that is the same size as the frame buffer, and stores the z position of each pixel
 *     - The z-buffer can be 16, 24, or 32 bit.. allowing for more depth layers.
 *
 */

/**
 * Pixel storage formats:
 *  - in *16/S formats the High/Low each represent a pixel, *16S is high, *16 is low
 *  - TODO: No idea what GS_PSM_PS24 actually does.. maybe gets around the wasted space from align?
 *
 * Frame-Buffer:
 *  - PSMCT32   (GS_PSM_32) -> RGBA32: Supports RBGA channels, 8 bits per channel
 *  - PSMCT24   (GS_PSM_24) -> BGR24:  Supports RGB only, 8 bits per channel, note reversed order (bits 24-31 are unused, memory not compressed as it is aligned)
 *  - PSMCT16/S (GS_PSM_16) -> ABGR16: Supports RGBA, 1 bit alpha, 5 bits per channel, note reversed order (half color depth for half the memory usage)
 *
 * Z-Buffer:
 *  - PSMZ32   -> Z32: 32 bits of depth per pixel
 *  - PSMZ24   -> Z24: 24 bits of depth per pixel (bits 24-31 are unused, memory not compressed, as it is aligned)
 *  - PSMZ16/S -> Z16: 16 bits of depth per pixel (half depth, half the space requirement)
 *
 * The combination of Frame-Buffer and Z-Buffer formats is limited by compatibility as follows:
 *  - PSMCT32  & PSMZ32 - PSMCT32  & PSMZ24 - PSMCT32  & PSMZ16S
 * 	- PSMCT24  & PSMZ32 - PSMCT24  & PSMZ24 - PSMCT24  & PSMZ16S
 *  - PSMCT16S & PSMZ32 - PSMCT16S & PSMZ24 - PSMCT16S & PSMZ16S
 *  - PSMCT16  & PSMZ16
 */

/**
 * Depth Test:
 * ZTEST_METHOD_ALLFAIL       - First pixel mapped to a location is always on top
 * ZTEST_METHOD_ALLPASS       - Last pixel mapped to a location is always on top
 * ZTEST_METHOD_GREATER_EQUAL - Pixel with z >= current mapped to a location is on top
 * ZTEST_METHOD_GREATER       - Pixel with z > current mapped to a location is on top
 */

/**
 * Primitive render types:
 * PRIM_POINT			- Single vertex positions, no faces rendered
 * PRIM_LINE			- Single line between each pair of vertices
 * PRIM_LINE_STRIP		- Single line connecting each vertex
 * PRIM_TRIANGLE		- Triangle between each triple of vertices
 * PRIM_TRIANGLE_STRIP	- Triangle between each vertex and the previous 2, except for the first triangle
 * PRIM_TRIANGLE_FAN	- Triangle between each vertex and the previous vertex and the first vertex
 * PRIM_SPRITE			- TODO: not sure how sprite renders
 */

/**
 * Initializes the Graphic Synthesizer
 *
 * framebuffer_t * frame -> The buffer storing frame information in EE memory
 * zbuffer_t * z -> The buffer storing z information in EE memory
 */
void init_gs(framebuffer_t * frame, zbuffer_t * z) {
	// Setup frame parameters
	frame->width = 1280;	// Width of the frame
	frame->height = 720;	// Height of the frame
	frame->mask = 0;		// Mask TODO: what does this do?
	frame->psm = GS_PSM_32;	// The pixel mode, see Pixel storage formats
	frame->address = graph_vram_allocate(frame->width, frame->height, frame->psm, GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	z->enable = DRAW_ENABLE;				// Enables the use of the Z-Buffer
	z->mask = 0;							// TODO
	z->method = ZTEST_METHOD_GREATER_EQUAL;	// Z-Buffer calculation method, see Depth Test
	z->zsm = GS_ZBUF_32;					// The pixel mode for Z-Buffer, see pixel storage formats
	z->address = graph_vram_allocate(frame->width, frame->height, z->zsm, GRAPH_ALIGN_PAGE);

	// Setup graphics for 720p video (TODO: can't seem to get 1080i working at this time)
	graph_set_mode(GRAPH_MODE_NONINTERLACED, GRAPH_MODE_HDTV_720P, GRAPH_MODE_FRAME, GRAPH_DISABLE);
	graph_set_screen(0, 0, 1280, 720);
	graph_set_bgcolor(0, 0, 0);
	graph_set_framebuffer_filtered(frame->address, frame->width, frame->psm, 0, 0);
	graph_enable_output();
}

/**
 * Initializes the draw region, sets the origin for the mesh and allocates space in the GIF for mesh data.
 *
 * framebuffer_t * frame -> The buffer storing frame information in EE memory
 * zbuffer_t * z -> The buffer storing z information in EE memory
 */
void init_drawing_environment(framebuffer_t * frame, zbuffer_t * z) {
	packet_t * mesh_packet = packet_init(16, PACKET_NORMAL);		// Create a DMA packet to be used to send the mesh data to GIF, of size 16 x quad_words (2048 bits)
	qword_t * q = mesh_packet->data;								// Create a pointer to the packet data

	q = draw_setup_environment(q, 0, frame, z);						// Create the drawing environment, using the frame/z buffers
	q = draw_primitive_xyoffset(q, 0, (2048 - 640), (2048 - 360));	// Now reset the primitive origin to 2048-width/2,2048-height/2
	q = draw_finish(q);												// Finish setting up the environment

	dma_channel_send_normal(DMA_CHANNEL_GIF, 						// Send the mesh data packet to the GIF, don't need to wait since it's the first DMA packet to be sent (so we know the DMA is not in use)
			mesh_packet->data, q - mesh_packet->data, 0, 0);
	dma_wait_fast();												// Wait for the packet to be transfered to the GIF
	packet_free(mesh_packet);										// Free the packet from EE memory, cause its now transfered to the GIF
}

/**
 * The render loop
 *
 * framebuffer_t * frame -> The buffer storing frame information in EE memory
 * zbuffer_t * z -> The buffer storing z information in EE memory
 */
int render(framebuffer_t * frame, zbuffer_t * z) {
	int i;
	int context = 0;

	MATRIX local_world;		// Matrix for calculating position in local space
	MATRIX world_view;		// Matrix for calculating position in world space
	MATRIX view_screen;		// Matrix for calculating the screen view (camera?)
	MATRIX local_screen;	// Matrix for calculating the local screen space

	VECTOR * temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);	// Vertex data for the mesh before calculation

	prim_t prim;						// Defines how the object is rendered
	prim.type = PRIM_TRIANGLE;			// The type of primitive that is used when rendering a mesh
	prim.shading = PRIM_SHADE_GOURAUD;	// The type of shading to use, see Primitive render types
	prim.mapping = DRAW_DISABLE;		// TODO: Texture mapping
	prim.fogging = DRAW_DISABLE;		// TODO: Fog rendering
	prim.blending = DRAW_DISABLE;		// TODO: Alpha blending
	prim.antialiasing = DRAW_ENABLE;	// TODO: AA
	prim.mapping_type = PRIM_MAP_ST;	// TODO: Texture mapping method
	prim.colorfix = PRIM_UNFIXED;		// TODO: color-fix? post processing?

	color_t color;						// Configure the color of the mesh
	color.r = 0x80; color.g = 0x80; color.b = 0x80; color.a = 0x80;
	color.q = 1.0f;						// Quaternion color, just leave it 1 if you don't know what you are doing.

	xyz_t   * verts = memalign(128, sizeof(vertex_t) * vertex_count);
	color_t * colors = memalign(128, sizeof(color_t)  * vertex_count);

	// The data packets for double buffering dma sends.
	packet_t * packets[2];
	packets[0] = packet_init(100, PACKET_NORMAL); // 12800 bytes
	packets[1] = packet_init(100, PACKET_NORMAL); // 12800 bytes

	packet_t * current;
	qword_t * q;
	qword_t * dmatag;

	// Create the view_screen matrix.
	create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

	// Wait for any previous dma transfers to finish before starting.
	dma_wait_fast();

	// Distance to move / direction
	float move_dist_x = 0.3f;
	float move_dist_y = 0.25f;
	float move_dist_z = 0.3f;

	// The main loop...
	for (;;) {
		current = packets[context];

		// Check for input
		//handleInput(con);

		// Moves the object
		if(object_position[0] <= -100.0f && move_dist_x < 0.0f) {
            move_dist_x = 0.3f;
		} else if(object_position[0] >= 100.0f && move_dist_x > 0.0f) {
		    move_dist_x = -0.3f;
		}
		object_position[0] += move_dist_x;

		if(object_position[1] <= 0.0f && move_dist_y < 0.0f) {
            move_dist_y = 0.25f;
		} else if(object_position[1] >= 100.0f && move_dist_y > 0.0f) {
		    move_dist_y = -0.25f;
		}
		object_position[1] += move_dist_y;

		if(object_position[2] <= -100.0f && move_dist_z < 0.0f) {
            move_dist_z = 0.3f;
		} else if(object_position[2] >= 20.0f && move_dist_z > 0.0f) {
		    move_dist_z = -0.3f;
		}
		object_position[2] += move_dist_z;

		// update camera position if following
		if(follow == 1) {
			camera_position[0] = object_position[0] + follow_offset[0];
			camera_position[1] = object_position[1] + follow_offset[1];
			camera_position[2] = object_position[2] + follow_offset[2];
		}

		// Spin the cube a bit.
		object_rotation[0] += 0.008f; //while (object_rotation[0] > 3.14f) { object_rotation[0] -= 6.28f; }
		object_rotation[1] += 0.012f; //while (object_rotation[1] > 3.14f) { object_rotation[1] -= 6.28f; }

		// Create the local_world matrix.
		create_local_world(local_world, object_position, object_rotation);

		// Create the world_view matrix.
		create_world_view(world_view, camera_position, camera_rotation);

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world, world_view, view_screen);

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 32, vertex_count, (vertex_f_t*)temp_vertices);

		// Convert floating point colous to fixed point.
		draw_convert_rgbq(colors, vertex_count, (vertex_f_t*)temp_vertices, (color_f_t*)colours, 0x80);

		// Grab our dmatag pointer for the dma chain.
		dmatag = current->data;

		// Now grab our qword pointer and increment past the dmatag.
		q = dmatag;
		q++;

		// Clear framebuffer but don't update zbuffer.
		q = draw_disable_tests(q, 0, z);
		q = draw_clear(q, 0, 2048.0f - 640.0f, 2048.0f - 360.0f, frame->width, frame->height, 0x80, 0x80, 0x80);
		q = draw_enable_tests(q, 0, z);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		for(i = 0; i < points_count; i++) {
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);

		// Setup a finish event.
		q = draw_finish(q);

		// Define our dmatag for the dma chain.
		DMATAG_END(dmatag, (q - current->data) - 1, 0, 0, 0);

		// Now send our current dma chain.
		dma_wait_fast();
		dma_channel_send_chain(DMA_CHANNEL_GIF, current->data, q - current->data, 0, 0);

		// Now switch our packets so we can process data while the DMAC is working.
		context ^= 1;

		// Wait for scene to finish drawing
		draw_wait_finish();

		graph_wait_vsync();
	}

	packet_free(packets[0]);
	packet_free(packets[1]);

	return 0;
}

int main(int argc, char **argv) {
	// The buffers to be used.
	framebuffer_t frame;
	zbuffer_t z;

	// Init Controller
	//initController();
	//con.onPressLeft = &moveLeft;
	//con.onPressRight = &moveRight;
	//con.onPressUp = &moveUp;
	//con.onPressDown = &moveDown;
	//con.onPressCross = &toggleFollow;

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(&frame, &z);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(&frame, &z);

	// Render the cube
	render(&frame, &z);

	// Sleep
	SleepThread();

	// End program.
	return 0;
}

/*void moveLeft() {
	camera_position[0] -= 10.0f;
}

void moveRight() {
	camera_position[0] += 10.0f;
}

void moveUp() {
	camera_position[1] += 10.0f;
}

void moveDown() {
	camera_position[1] -= 10.0f;
}

void toggleFollow() {
	if(follow == 1) follow = 0;
	else {
		follow = 1;
		follow_offset[0] = camera_position[0] - object_position[0];
		follow_offset[1] = camera_position[1] - object_position[1];
		follow_offset[2] = camera_position[2] - object_position[2];
	}
}*/
