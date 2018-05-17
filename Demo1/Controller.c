/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
#-----------------------------------------------------------------------
# Author: Jacob Pozaic
#
# A configurable controller mapping module.
#
# Based on mtap sample included in the base PS2SDK.
#
# IRX requirements
#   - sio2man.irx
#   - mtapman.irx
#   - padman.irx
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

#include <gs_privileged.h>
#include <libpad.h>

#include <stdio.h>
#include <string.h>

#include <Controller.h>

// Values
static char* padBuf[2][4];		// ?
static u32 padConnected[2][4]; 	// 2 ports, 4 slots
static u32 padOpen[2][4];		// ?
static u32 mtapConnected[2];	// ?
static u32 maxslot[2];			// ?

static void wait_vsync() {
	// Enable the vsync interrupt.
	*GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

	// Wait for the vsync interrupt.
	while (!(*GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

	// Disable the vsync interrupt.
	*GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void loadModules() {
	int ret;
	#ifdef CUSTOM_IRX
		if((ret = SifLoadModule(SIO2MAN_IRX, 0, NULL)) < 0) {
			#ifdef DEBUG
				printf("Failed to load sio2man.irx module (%d)\n", ret);
			#endif
			SleepThread();
		}

		if((ret = SifLoadModule(MTAPMAN_IRX, 0, NULL)) < 0) {
			#ifdef DEBUG
				printf("Failed to load mtapman.irx module (%d)\n", ret);
			#endif
			SleepThread();
		}

		if((ret = SifLoadModule(PADMAN_IRX, 0, NULL)) < 0) {
			#ifdef DEBUG
				printf("Failed to load padman.irx module (%d)\n", ret);
			#endif
			SleepThread();
		}
	#else
		if((ret = SifLoadModule("rom0:XSIO2MAN", 0, NULL)) < 0) {
			#ifdef DEBUG
				printf("Failed to load XSIO2MAN module (%d)\n", ret);
			#endif
			SleepThread();
		}

		if((ret = SifLoadModule("rom0:XMTAPMAN", 0, NULL)) < 0) {
			#ifdef DEBUG
				printf("Failed to load XMTAPMAN module (%d)\n", ret);
			#endif
			SleepThread();
		}

		if((ret = SifLoadModule("rom0:XPADMAN", 0, NULL)) < 0) {
			#ifdef DEBUG
				printf("Failed to load XPADMAN module (%d)\n", ret);
			#endif
			SleepThread();
		}
	#endif
}

static void find_controllers() {
	u32 port, slot;
	u32 mtapcon;

	// Look for multitaps and controllers on both ports
	for(port = 0; port < 2; port++) {
		mtapcon = mtapGetConnection(port);

		#ifdef DEBUG
			if((mtapcon == 1) && (mtapConnected[port] == 0))
				printf("Multitap (%i) connected\n", (int) port);

			if((mtapcon == 0) && (mtapConnected[port] == 1))
				printf("Multitap (%i) disconnected\n", (int) port);
		#endif

		mtapConnected[port] = mtapcon;

		// Check for multitap
		if(mtapConnected[port] == 1) maxslot[port] = 4;
		else maxslot[port] = 1;

		// Find any connected controllers
		for(slot = 0; slot < maxslot[port]; slot++) {
			if(padOpen[port][slot] == 0)
				padOpen[port][slot] = padPortOpen(port, slot, padBuf[port][slot]);

			if(padOpen[port][slot] == 1) {
				if(padGetState(port, slot) == PAD_STATE_STABLE) {
					#ifdef DEBUG
						if(padConnected[port][slot] == 0)
							printf("Controller (%i,%i) connected\n", (int)port, (int)slot);
					#endif
					padConnected[port][slot] = 1;
				} else {
					if((padGetState(port, slot) == PAD_STATE_DISCONN) && (padConnected[port][slot] == 1)) {
						#ifdef DEBUG
							printf("Controller (%i,%i) disconnected\n", (int)port, (int)slot);
						#endif
						padConnected[port][slot] = 0;
					}
				}
			}
		}

		// Close controllers when multitap is disconnected
		if(mtapConnected[port] == 0) {
			for(slot = 1; slot < 4; slot++) {
				if(padOpen[port][slot] == 1) {
					padPortClose(port, slot);
					padOpen[port][slot] = 0;
				}
			}
		}
	}
}

void initController() {
	u32 i;

	struct padButtonStatus buttons;
	u32 paddata;
	u32 old_pad[2][4];
	u32 new_pad[2][4];
	s32 ret;

	SifInitRpc(0);

	loadModules();

	mtapInit();
	padInit(0);

	mtapConnected[0] = 0;
	mtapConnected[1] = 0;

	mtapPortOpen(0);
	mtapPortOpen(1);

	for(i = 0; i < 4; i++) {
		padConnected[0][i] = 0;
		padConnected[1][i] = 0;
		padOpen[0][i] = 0;
		padOpen[1][i] = 0;
		old_pad[0][i] = 0;
		old_pad[1][i] = 0;
		new_pad[0][i] = 0;
		new_pad[1][i] = 0;

		padBuf[0][i] = memalign(64, 256);
		padBuf[1][i] = memalign(64, 256);
	}
}

void handleInput(control_map con) {
	//TODO: create thread, start thread, way to stop & delete thread
	//TODO: event system and a way to register events in an application using this, that is Async
	// register as an event-receiver with this 'class', give each register an id-of-last-event-received
	// when button/stick event occurs, add event to event queue, each event having an incrementing id
	// when all registered event-receivers id-of-last-event-received >= event_id, pop event off the queue
	// maybe?

	u32 port, slot;

	find_controllers();

	for(port = 0; port < 2 ; port++) {
		for(slot = 0; slot < maxslot[port]; slot++) {
			if(padOpen[port][slot] && padConnected[port][slot]) {
				/* This checks controller type / mode
				if((state == PAD_STATE_STABLE) && (portConnected[port] == 0)) {
					u32 i;
					u8 mTable[8];
					u32 ModeTableNum = padInfoMode(port, 0, PAD_MODETABLE, -1);

					#if DEBUG == 1
						u32 ModeCurId;
						u32 ModeCurOffs;
						u32 ModeCurExId;

						printf("Controller (%i) connected\n", port);
					#endif

					// Check if dualshock and if so, activate analog mode
					for(i = 0; i < ModeTableNum; i++)
						mTable[i] = padInfoMode(port, 0, PAD_MODETABLE, i);

					// Works for dualshock2
					if((mTable[0] == 4) && (mTable[1] == 7) && (ModeTableNum == 2))
						dualshock[port] = 1;

					// Active and lock analog mode
					if(dualshock[port] == 1) {
						padSetMainMode(port, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
						padWait(port);
					}

					#if DEBUG == 1
						ModeCurId = padInfoMode(port, 0, PAD_MODECURID, 0);
						ModeCurOffs = padInfoMode(port, 0, PAD_MODECUROFFS, 0);
						ModeCurExId = padInfoMode(port, 0, PAD_MODECUREXID, 0);
						ModeTableNum = padInfoMode(port, 0, PAD_MODETABLE, -1);
					#endif

					acts[port] = padInfoAct(port, 0, -1, 0);

					#if DEBUG == 1
						printf("  ModeCurId      : %i (%s)\n", (int)ModeCurId, padTypeStr[ModeCurId]);
						printf("  ModeCurExId    : %i\n", (int)ModeCurExId);
						printf("  ModeTable      : ");

						for(i = 0; i < ModeTableNum; i++) {
							mTable[i] = padInfoMode(port, 0, PAD_MODETABLE, i);
							printf("%i ", (int)mTable[i]);
						}

						printf("\n");
						printf("  ModeTableNum   : %i\n", (int)ModeTableNum);
						printf("  ModeCurOffs    : %i\n", (int)ModeCurOffs);
						printf("  NumOfAct       : %i\n", (int)acts[port]);
						printf("  PressMode      : %i\n", (int)padInfoPressMode(port, 0));
					#endif

					if(acts[port] > 0) {
						u8 actAlign[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
						u32 i;

						// Set offsets for motor parameters for SetActDirect.
						for(i = 0; i < acts[port]; i++) actAlign[i] = i;

						padSetActAlign(port, 0, actAlign);
						padWait(port);
					}

					#if DEBUG == 1
						printf("  EnterPressMode : %i\n", (int)padEnterPressMode(port, 0));
						padWait(port);

						printf("Ready\n");
					#endif

					portConnected[port] = 1;
				}

				if((state == PAD_STATE_DISCONN) && (portConnected[port] == 1)) {
					#if DEBUG == 1
						printf("Controller (%i) disconnected\n", port);
					#endif
					portConnected[port] = 0;
				}
				*/

				ret = padRead(port, slot, &buttons);

				if (ret != 0) {
					paddata = 0xffff ^ buttons.btns;

					new_pad[port][slot] = paddata & ~old_pad[port][slot];
					old_pad[port][slot] = paddata;

					// Values 50 and 200 used because my controllers are worn out :-)
					/*if((buttons.ljoy_h <= 50) || (buttons.ljoy_h >= 200)) printf("Left Analog  X: %i\n", (int)buttons.ljoy_h);
					if((buttons.ljoy_v <= 50) || (buttons.ljoy_v >= 200)) printf("Left Analog  Y: %i\n", (int)buttons.ljoy_v);
					if((buttons.rjoy_h <= 50) || (buttons.rjoy_h >= 200)) printf("Right Analog X: %i\n", (int)buttons.rjoy_h);
					if((buttons.rjoy_v <= 50) || (buttons.rjoy_v >= 200)) printf("Right Analog Y: %i\n", (int)buttons.rjoy_v);
					*/

					// To make the vibration start/stop:
					//padStartAct(port, 0, 1);
					//padStartAct(port, 0, 0);
					//padStartAct(port, 1, 255);
					//padStopAct(port, 1);

					if(new_pad[port] & PAD_LEFT) 	con.onPressLeft();
					if(new_pad[port] & PAD_RIGHT) 	con.onPressRight();
					if(new_pad[port] & PAD_UP) 		con.onPressUp();
					if(new_pad[port] & PAD_DOWN) 	con.onPressDown();

					if(new_pad[port] & PAD_START) 	con.onPressStart();
					if(new_pad[port] & PAD_SELECT) 	con.onPressSelect();

					if(new_pad[port] & PAD_SQUARE) 	con.onPressSquare();
					if(new_pad[port] & PAD_TRIANGLE)con.onPressTriangle();
					if(new_pad[port] & PAD_CIRCLE)	con.onPressCircle();
					if(new_pad[port] & PAD_CROSS)	con.onPressCross();

					if(new_pad[port] & PAD_L1)      con.onPressL1();
					if(new_pad[port] & PAD_L2)      con.onPressL2();
					if(new_pad[port] & PAD_L3)		con.onPressL3();

					if(new_pad[port] & PAD_R1) 		con.onPressR1();
					if(new_pad[port] & PAD_R2)		con.onPressR2();
					if(new_pad[port] & PAD_R3)		con.onPressR2();
				}
			}
		}
	}

	wait_vsync();
}

void padStartAct(int port, int act, int speed) {
	if(actDirect[port][act] != speed) {
		actDirect[port][act] = speed;

		padSetActDirect(port, 0, actDirect[port]);
		padWait(port);
	}
}

void padStopAct(int port, int act) {
	padStartAct(port, act, 0);
}

