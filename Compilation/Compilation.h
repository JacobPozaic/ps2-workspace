#ifndef COMPILATION_H_
#define COMPILATION_H_

VECTOR object_position = { 0.00f, 0.00f, 0.00f, 1.00f };
VECTOR object_rotation = { 0.00f, 0.00f, 0.00f, 1.00f };

VECTOR camera_position = { 0.00f, 0.00f, 200.00f, 1.00f };
VECTOR camera_rotation = { 0.00f, 0.00f,   0.00f, 1.00f };

int follow = 0;
VECTOR follow_offset = { 0.00f, 0.00f, 0.00f };

// Stores what functions to map to each controller key
//control_map con;

/**
 * Initializes the Graphics Synthesizer
 */
void init_gs(framebuffer_t *frame, zbuffer_t *z);

/**
 * Prepares the space to draw in and loads the mesh data
 */
void init_drawing_environment(framebuffer_t *frame, zbuffer_t *z);

/**
 * Renders a frame.
 */
int render(framebuffer_t *frame, zbuffer_t *z);

/**
 * Entry point
 */
int main(int argc, char **argv);

/**
 * Displaces the camera
 */
void moveLeft();
void moveRight();
void moveUp();
void moveDown();

/**
 * Makes the camera follow the cube
 */
void toggleFollow();

#endif
