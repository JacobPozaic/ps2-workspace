/**
 *
 */

#include <OBJLoader.c>

int main(int argc, char **argv) {
	mesh_data * mesh = malloc(sizeof(mesh_data));
	printf("Reading OBJ result: %d\n", parseOBJ("host:Meshes/cube.obj", mesh));


	while(1){} // DEBUG: prevent reset
	return (0);
}
