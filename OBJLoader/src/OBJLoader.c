// The file format for .obj is as follows:
// # - comment
//
// v - vertex coordinates (x, y, z [, w]) if no w, value is 1.0
// v 0.1 0.222 0.0321
//
// vt - texture coordinates (u, v [, w]) if no w, value is 0
// vt 0.5 0.73
//
// vn - vertex normals (x, y, z) (possibly not unit vectors)
// vn 0.123 0.000 0.442
//
// vp - parameter space vertices TODO: add support
//
// f - face, format = vertex_id/texture_id/normal_id
// f 1 2 3				-> face between vertex 1, 2, and 3
// f 1 2 3 4 5			-> face between vertex 1, 2, 3, 4, and 5
// f 2/1 3/2 6/4  		-> face between vertex 2, 3, and 6.  with texture 1, 2, and 4 mapped on the vertices
// f 2/3/4 3/4/5 4/5/6	-> face between vertex 2, 3, and 4.  with texture 3, 4, and 5 mapped on the vertices.  with normals 4, 5, and 6 on the vertices.
// f 7//2 3//1 9//4		-> face between vertex 7, 3, and 9.  with no texture map.  with normals 2, 1, and 4 on the vertices.
//
// l - line builds a line between each vertex in the order they are listed TODO: add support
// l 1 4 5 6 2 5 -> line from 1 to 4 to 5...
//
// mtllib - TODO: add support
// usemtl - TODO: add support
// o & g & s - TODO: really don't need this
//
// support for other BS:
// - relative indexes (will add a fuckload of size to the library..)
//

// TODO: obj pre-processor can decrease load speed.
// TODO: or just create a custom mesh binary file

// If debug printing should be done, define this:
#define DEBUG_OBJL

#include <tamtypes.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sifrpc.h>
#include "loadfile.h"
#include <fileio.h>

//TODO: Sioman

#include <OBJLoader.h>

/**
 * Parses an obj file into mesh data
 * TODO: flags to disable reading certain types, saving space used by mesh, and processing time.
 *
 * file--> The fully qualified path to read the file from.
 * mesh--> The mesh_data object to read the mesh into
 *
 * returns:
 *     0 -> Successfully read file
 *    -1 -> Couldn't open file
 *    -2 -> Couldn't write allocate space for the file data
 *    -3 -> Couldn't allocate enough memory for pre-parser (literally 7 bytes..)
 *    -4 -> Couldn't allocate enough memory for mesh data
 */
int parseOBJ(char * file, mesh_data * mesh) {
	SifInitRpc(0);
	SifLoadModule("rom0:XSIO2MAN", 0, NULL);
	fioInit();

	#ifdef DEBUG_OBJL
	printf("Loading OBJ file: %s\n", file);
	#endif

	int file_handle = fioOpen(file, O_RDONLY);
	if(file_handle == -1) return -1;

	int data_size = fioLseek(file_handle, 0, SEEK_END);
	fioLseek(file_handle, 0, SEEK_SET);

	u8 * data = (u8 *) malloc(data_size);
	if(data == NULL) return -2;

	fioRead(file_handle, data, data_size);
	fioClose(file_handle);

	#ifdef DEBUG_OBJL
	printf("The file has been read:\nsize: %d\n", data_size);
	#endif

	// Start parsing the file
	char * prefix_buffer = (char *) malloc(7);			// 6 is the largest prefix length + 1 for null character space
	if(prefix_buffer == NULL) return -3;

	int buffer_index = 0;								// The index to write to in the prefix buffer
	int i = 0;											// The index in the data to read from
	int skip_line = 0;									// If the line has been skipped
	int skip_index = 0;									// Marks the beginning of lines that should not be parsed
	while(i++ < data_size) {							// For every byte in the data
		char value = data[i];							// Store the next byte to check for end of prefix
		if(value == ' ') {								// The prefix_buffer contains prefix for this line
			prefix_buffer[buffer_index] = '\0';			// Instead of copying the space character just terminate the string.

			#ifdef DEBUG_OBJL
			printf("Prefix buffer contains: %s\n", prefix_buffer);
			#endif

			// Increment the count for the type of data the line represents
			if(!strcmp(prefix_buffer, "v")) 		mesh->vertex_count++;
			else if(!strcmp(prefix_buffer, "vn"))	mesh->normal_count++;
			else if(!strcmp(prefix_buffer, "vt")) 	mesh->texture_count++;
			else if(!strcmp(prefix_buffer, "f"))	mesh->face_count++;
			//else if(!strcmp(prefix_buffer, "l")) 	mesh->line_count++;		// Not implemented
			else {
				skip_line = 1;
				data[skip_index] = '\0';				// Sets the first character of the line to null, so when parsing the line can be skipped skip a line
			}

			buffer_index = 0;												// Reset the buffer index so we write at the beginning again
			while(i++ < data_size) {										// Seek i to next line as we are only trying to count the number of each type of data here.
				if(data[i] == '\n') {										// If newline is found then we know that we are looking at the next line
					if(skip_line) {
						int * skip_value = (int *) &data[skip_index + 1];
						*skip_value = i + 1;								// Set the next character after the null to the index of the next line, this is to make skipline O(1) instead of O(n)
					}
					skip_index = i + 1;										// Update the location of the skip_line index
					skip_line = 0;											// Reset line skip flag
					break;													// Start reading the next line
				}
			}
		} else prefix_buffer[buffer_index++] = value;	// The prefix is not fully read yet, so copy the character into the prefix buffer and increment the buffer index
	}

	free(prefix_buffer);								// Free the prefix buffer, wont need it anymore

	#ifdef DEBUG_OBJL
	printf("Done pre-parsing OBJ file... data counts are:\nVertices: %d\nNormals: %d\nTexture: %d\nFaces: %d\n", mesh->vertex_count, mesh->normal_count, mesh->texture_count, mesh->face_count);
	#endif

	// Allocate space for the mesh data in mesh...
	mesh->vertices 		 = (vertex *)	malloc(mesh->vertex_count  * sizeof(vertex));
	mesh->normals		 = (V3 *)		malloc(mesh->normal_count  * sizeof(V3));
	mesh->faces			 = (face *)		malloc(mesh->face_count    * sizeof(face));
	mesh->texture_coords = (texture *)	malloc(mesh->texture_count * sizeof(texture));

	if(mesh->vertices == NULL || mesh->normals == NULL || mesh->faces == NULL || mesh->texture_coords == NULL) return -4;

	#ifdef DEBUG_OBJL
	printf("Space has been allocated for the mesh data...\nBeginning to parse...\n");
	#endif

	int data_index = 0;										// Index in memory the line is being read from
	while(data_index < data_size) {							// Read through the entire obj file from memory, line by line
		if(data[data_index] == '\0') {						// If the line should be skipped
			int * jumpto = (int *) &data[data_index + 1];
			data_index = *jumpto;							// Increment to the index of the next line

			#ifdef DEBUG_OBJL
			printf("A line has been skipped, jumping to: %d\n", data_index + 1);
			#endif

			continue;										// Start parsing the next line
		}

		char * line = &data[data_index];
		int line_length = 0;								// Determines the length of the line that will be read
		while(data_index + line_length < data_size)
			if(data[data_index + line_length] == '\n') break;
			else line_length++;
		line[line_length] = '\0';							// Terminate the line

		#ifdef DEBUG_OBJL
		printf("A line has been read: %s\n", line);
		#endif

		data_index += line_length + 1; // Point data_index to the first character of next line.

		// Parse line
		int line_index = 0;
		int space_index = 0;
		int parameter_count = 0;

		data_container * d = malloc(sizeof(data_container));
		int data_type = -1;
		int use_w = 0;
		while(line_index++ < line_length) {
			if(line[line_index] == ' ') {
				line[line_index] = '\0';
				char * param = &line[space_index];

				#ifdef DEBUG_OBJL
				printf("A Parameter has been read: %s\n", param);
				#endif

				// Check for data types, otherwise its a value.
				if(data_type == -1) {
					if(!strcmp(prefix_buffer, "v"))			data_type = 0;
					else if(!strcmp(prefix_buffer, "vn"))	data_type = 1;
					else if(!strcmp(prefix_buffer, "vt"))	data_type = 2;
					else if(!strcmp(prefix_buffer, "f"))	data_type = 3;
					else { /* SHOULD NEVER HAPPEN */ }
				} else {
					switch(data_type) {
					case(0):
						if(parameter_count == 4) {
							float x = d->v->data[0];
							float y = d->v->data[1];
							float z = d->v->data[2];
							d->v->data_w[0] = x;
							d->v->data_w[1] = y;
							d->v->data_w[2] = z;
							d->v->data_w[3] = (float) strtod(param, NULL);
							use_w = 1;
						} else d->v->data[parameter_count - 1] = (float) strtod(param, NULL);
						break;
					case(1):
						*(d->n[parameter_count - 1]) = (float) strtod(param, NULL);
						break;
					case(2):
						if(parameter_count == 3) {
							float u = d->t->data[0];
							float v = d->t->data[1];
							d->t->data_w[0] = u;
							d->t->data_w[1] = v;
							d->t->data_w[2] = (float) strtod(param, NULL);
							use_w = 1;
						} else d->t->data[parameter_count - 1] = (float) strtod(param, NULL);
						break;
					case(3):
						//TODO: parse face data...
						break;
					}
				}

				free(param);
				space_index = line_index + 1;
				parameter_count++;
			}
		}

		// Copy the parsed data into mesh_data
		switch(data_type) {
		case(0):
			if(use_w) {
				mesh->vertices = malloc(sizeof(V4));
				memcpy(mesh->vertices, d->v->data_w, sizeof(d->v->data_w));
			} else {
				mesh->vertices = malloc(sizeof(V3));
				memcpy(mesh->vertices, d->v->data, sizeof(d->v->data));
			}
			break;
		case(1):
			memcpy(mesh->normals, d->n, sizeof(d->n));
			break;
		case(2):
			if(use_w) {
				mesh->texture_coords = malloc(sizeof(V3));
				memcpy(mesh->texture_coords, d->t->data_w, sizeof(d->t->data_w));
			} else {
				mesh->texture_coords = malloc(sizeof(V2));
				memcpy(mesh->texture_coords, d->t->data, sizeof(d->t->data));
			}
			break;
		case(3):
			mesh->faces = malloc(sizeof(face));
			memcpy(mesh->faces, d->f, sizeof(d->f));
			break;
		}

		free(d);
		use_w = 0;
		data_type = -1;
		line_index++;
	}

	free(data);	// Free the data, as we are done parsing
	return 0;
}
