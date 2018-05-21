/*
 * OBJLoader.h
 *
 *  Created on: May 20, 2018
 *      Author: Jacob
 */

#ifndef OBJLOADER_H_
#define OBJLOADER_H_

typedef float V4[] = { 0.0f, 0.0f, 0.0f, 1.0f };
typedef float V3[] = { 0.0f, 0.0f, 0.0f };
typedef float V2[] = { 0.0f, 0.0f };

typedef union {
	V4 data_w;
	V3 data;
} vertex;

typedef union {
	V3 data_w;
	V2 data;
} texture;

typedef struct {
	int vertex_index;
	int texture_index;
	int normal_index;
} face_component;

typedef struct {
	int component_count;
	face_component face_components[];
} face;

typedef struct {
	int size;
	u8 * data;
} obj_file;

typedef struct {
	int vertex_count = 0;
	vertex vertices[];

	int normal_count = 0;
	V3 normals[];

	int face_count = 0;
	face faces[];

	int texture_count = 0;
	texture texture_coords[];
} mesh_data;

int parseOBJ(char * file, mesh_data * mesh);
obj_file * openOBJ(char * file);

#endif /* OBJLOADER_H_ */
