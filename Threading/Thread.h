/**
 * NOTE: EE threads not IOP
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <tamtypes.h>

// TODO: Ideas:
//     - Implement semaphores
//     - It would be interesting to make a thread-safe printf/IO operations (using semaphores)
//     - I would like to have some more useful threading functionality.

//----------------------------------------------------------------------------------------------------
// Default thread parameter values
//
// TODO: find out how aligned works, some thread stacks aligned 64?
// To define a thread stack:
//     char default_stack[16*1024] __attribute__((aligned(16)));
//
// TODO: find out what attr is (and add to signature for createThread)
// To define attr:
//     u32 default_attr = 0x02000000;
//
// TODO: find out what option is (and add to signature for createThread)
// To define option:
//    u32 default_option = 0;

//TODO: figure out what this is
// It seems possible that the gp register is used for alarms.
extern void * _gp;

//----------------------------------------------------------------------------------------------------
// TODO: unimplemented methods
//     - basically all the i* and _* methods
//     - semaphores
//     - alarms
//
// TODO: find purpose of all i* methods, and document the i* method fixes and why.
//     - s32 iWakeupThread(s32 thread_id);
//     - s32 iRotateThreadReadyQueue(s32 priority);
//     - s32 iSuspendThread(s32 thread_id);

//----------------------------------------------------------------------------------------------------
// Thread property operations

s32 getCurrentThread(void);
s32 setThreadPriority(s32 t_id, s32 priority);
int getThreadStackSize(s32 t_id);
char * getThreadStack(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread management functions

s32 createThread(void * function, char stack[], void * gp_register, s32 priority, u32 attr, u32 option);
s32 startThread(s32 t_id, void * args);
void exitThread(void);
void exitDeleteThread(void);
s32 deleteThread(s32 t_id);
s32 terminateThread(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread scheduling functions

s32 rotateThreadReadyQueue(s32 priority);
s32 releaseWaitThread(s32 t_id);
s32 referThreadStatus(s32 t_id, ee_thread_status_t *info);

//----------------------------------------------------------------------------------------------------
// Thread sleep functions

s32 sleepThread(void);
s32	wakeupThread(s32 t_id);
s32	cancelWakeupThread(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread suspension functions

s32	suspendThread(s32 t_id);
s32	resumeThread(s32 t_id);

//----------------------------------------------------------------------------------------------------

#endif /* THREAD_H_ */
