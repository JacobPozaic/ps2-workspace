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

extern void * _gp; //TODO: figure out what this is

//----------------------------------------------------------------------------------------------------
// TODO: unimplemented methods
//
// TODO: find purpose of all i* methods, and document the i* method fixes and why.
//     - s32 iWakeupThread(s32 thread_id);
//     - s32 iRotateThreadReadyQueue(s32 priority);
//     - s32 iSuspendThread(s32 thread_id);

//----------------------------------------------------------------------------------------------------
// Thread property operations

/**
 * Returns the t_id of the calling thread.
 */
s32 getCurrentThread(void);

/**
 * Sets the priority of a given thread.
 * A threads priority can be between 0..127 (inclusively)
 */
s32 setThreadPriority(s32 t_id, s32 priority);

/**
 * Returns the size of the stack for a given thread.
 */
int getThreadStackSize(s32 t_id);

/**
 * Returns a pointer to the contents of a given threads stack.
 * WARNING: I highly recommend you don't mess with this, cause you will probably break something.
 */
char * getThreadStack(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread management functions

/**
 * Creates a thread with a given priority, stack, gp register, attr, and option.
 *
 * Thread.h has comments for working default values.  In all honesty I still have no idea what some of these do.
 *
 * @param function Pointer to a function that contains the code this thread will execute.
 * @param stack The memory location that will be used for the threads stack.
 * @param gp_register Points to a general purpose register, by defining 'extern void * _gp;' PS2SDK probably takes care of it.
 * @param priority The priority that this thread should use when being scheduled.
 * @param attr TODO: ?
 * @param option TODO: ?
 */
s32 createThread(void * function, char stack[], void * gp_register, s32 priority, u32 attr, u32 option);

/**
 * Start the given thread with arguments.
 * Arguments can be anything, you pass it to your thread, and define them in your thread definition.
 * TODO: an example that uses arguments.
 *
 * NOTE: It would seem that you cannot start a Thread that was started previously whose status has resulted in THS_DORMANT
 * by being exited, terminated, or completed successfully.
 */
s32 startThread(s32 t_id, void * args);

/**
 * Stops this thread, un-scheduling it, and setting its status to THS_DORMANT.
 */
void exitThread(void);

/**
 * Stops this thread and deletes it from memory.
 */
void exitDeleteThread(void);

/**
 * Deletes a given thread from memory.
 * A thread that has finished execution is not deleted by default.
 */
s32 deleteThread(s32 t_id);

/**
 * Stops a given thread, un-scheduling it, and setting its status to THS_DORMANT.
 *
 * NOTE: terminateThread can NOT terminate the calling thread, instead use exitThread().
 */
s32 terminateThread(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread scheduling functions

/**
 * Stop executing the current thread and start the next ready thread.
 * If any threads are the same priority they operate in round-robin.
 *
 * NOTE: It is unclear what the priority parameter is for.
 * TODO: prove this works
 */
s32 rotateThreadReadyQueue(s32 priority);

/**
 * Releases a given thread that is waiting for a semaphore to unblock.
 *
 * TODO: don't know what this does, I assume (if THS_WAIT means that the thread is waiting for a semaphore)
 * that this will stop that waiting thread from trying to access whatever the SEMA is blocking.
 */
s32 releaseWaitThread(s32 t_id);

/**
 * Gets the status of a thread and saves it into *info
 *
 * TODO: fact check statuses
 * TODO: wait might mean that it is waiting on a semaphore?
 *
 * THS_RUN		    0x01    Thread is currently running.
 * THS_READY	    0x02    Thread is ready to run and has been queued.
 * THS_WAIT	        0x04    Thread is waiting?
 * THS_SUSPEND	    0x08    Thread has been suspended.
 * THS_WAITSUSPEND	0x0c    Thread is waiting and suspended?
 * THS_DORMANT	    0x10    Thread has not been started yet, exited, or terminated.
 */
s32 referThreadStatus(s32 t_id, ee_thread_status_t *info);

//----------------------------------------------------------------------------------------------------
// Thread sleep functions

/**
 * Makes the current thread sleep until WakeupThread is called on its t_id.
 */
s32 sleepThread(void);

/**
 * Wakes up a given thread that is sleeping.
 *
 * TODO: causes a System trap apparently???
 */
s32	wakeupThread(s32 t_id);

/**
 * Cancels a wake-up call on a given thread.
 *
 * I assume this functions is to cancel a call to WakeupThread, where later during execution of the
 * calling thread it is determined that the other thread should not be woken up, before the calling
 * thread releases its execution slot to the scheduler.
 *
 * TODO: prove this works
 */
s32	cancelWakeupThread(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread suspension functions
// NOTE: Suspend is often deprecated (although not documented here) as it is prone to deadlocks

/**
 * Suspends a given thread. A suspended thread is paused and will not be rescheduled or continue executing.
 *
 * It is unclear if there is a way to restart/continue a suspended thread..
 *
 * It is unclear if it restarts from the beginning or from the point where it had been suspended.
 */
s32	suspendThread(s32 t_id);

/**
 * TODO: doesn't seem to be capable of resuming a thread that has been suspended?
 *
 * Appears to re-initialize the VIF0, VIF1, and GIF
 */
s32	resumeThread(s32 t_id);

//----------------------------------------------------------------------------------------------------

#endif /* THREAD_H_ */
