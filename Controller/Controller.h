#include <ControllerMapping.h>

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

// Enable debug code
// #define DEBUG

// Use custom IRX files included with the elf file, otherwise use the default IRX files.
// #define CUSTOM_IRX
#ifdef CUSTOM_IRX
    #define SIO2MAN_IRX = "host0:sio2man.irx"
    #define MTAPMAN_IRX = "host0:mtapman.irx"
    #define PADMAN_IRX = "host0:padman.irx"
#endif

// Functions
static void wait_vsync();
static void loadModules();
static void find_controllers();

// Interfacing with the controller
void initController();
void handleInput(control_map con);

#endif

/* CONTROLLER_H_ */
