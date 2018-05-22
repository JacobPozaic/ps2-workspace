/**
 *
 */

#include <OBJLoader.c>

int main(int argc, char **argv) {
	mesh_data * mesh = malloc(sizeof(mesh_data));
	printf("Reading OBJ result: %d\n", parseOBJ("host:Meshes/cube.obj", mesh));
	return (0);
}
