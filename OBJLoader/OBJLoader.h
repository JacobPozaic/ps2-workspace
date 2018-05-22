/*
 * OBJLoader.h
 *
 *  Created on: May 20, 2018
 *      Author: Jacob
 */

#ifndef OBJLOADER_H_
#define OBJLOADER_H_

typedef float V4[4];
typedef float V3[3];
typedef float V2[2];

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
	face_component * face_components;
} face;

typedef struct {
	int size;
	u8 * data;
} obj_file;

typedef struct {
	int vertex_count;
	vertex * vertices;

	int normal_count;
	V3 * normals;

	int face_count;
	face * faces;

	int texture_count;
	texture * texture_coords;
} mesh_data;

typedef union {
	vertex * v;
	V3 * n;
	texture * t;
	face * f;
} data_container;

int parseOBJ(char * file, mesh_data * mesh);
obj_file * openOBJ(char * file);
float parseFloat(char * string_value);

#endif /* OBJLOADER_H_ */
