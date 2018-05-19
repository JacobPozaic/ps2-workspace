/**
 * NOTE: EE threads not IOP
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <tamtypes.h>

// TODO: I would like to have some more useful threading functionality.
// TODO: I can make a sort of thread manager with high priority that can suspend/resume or sleep/wakeup to fake stronger scheduling
// TODO: interestingly enough, it would seem that multiple threads cant use printf all at once or else there is an error.
// It would be interesting to make a thread-safe printf/IO operations (using SEMA)

// TODO: find out how aligned works, some thread stacks aligned 64?
static char default_stack[16*1024] __attribute__((aligned(16)));
extern void * _gp; //TODO: figure out what this is
static s32 default_priority = 50;
static u32 default_attr = 0x02000000;
static u32 default_option = 0;

// TODO: find purpose of:
//s32 iWakeupThread(s32 thread_id);
//s32 iRotateThreadReadyQueue(s32 priority);
//s32 iSuspendThread(s32 thread_id);

/**
 * Returns the t_id of the current thread (the one this line of code resides in when called)
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
char * getStackContents(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread management functions

/**
 * Creates a thread with a given priority, stack, gp register, attr, and option.
 *
 * Defined in Thread.h are default values for each field, so that you don't have to spend a week trying to find what
 * half these parameters do, and what an appropriate value is.  In all honesty I still have no idea what these are/do
 * gp_register - points to a general purpose register, but no idea why.  It appears its value doesn't exactly matter,
 *     and by defining 'extern void * _gp;' PS2SDK probably takes care of it.
 * attr - TODO: absolutely no idea
 * option - TODO: no idea, PS2SDK says it doesn't work, but I have seen code that does use it.
 */
s32 createThread(void * function, char stack[], void * gp_register, s32 priority, u32 attr, u32 option);

/**
 * Start the given thread with arguments.
 * Arguments can be anything, you pass it to your thread, and define them in your thread definition.
 * TODO: an example that uses arguments.
 *
 * NOTE: it would seem that you cannot start a Thread that has started before and resulted in its status
 * being THS_DORMANT, I have no idea why.
 */
s32 startThread(s32 t_id, void * args);

/**
 * Stops this thread, un-scheduling it, and setting its status to THS_DORMANT.
 */
void exitThread(void);

/**
 * Exits this thread and deletes it.
 */
void exitDeleteThread(void);

/**
 * Deletes a given thread from memory.  A thread that has finished execution is not deleted by default.
 */
s32 deleteThread(s32 t_id);

/**
 * Stops a given thread, un-scheduling it, and setting its status to THS_DORMANT.
 *
 * NOTE: terminateThread can NOT terminate the calling thread in this way:
 *     terminateThread(getCurrentThread());
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
 * Releases a given thread that is waiting for a SEMA to unblock.
 *
 * TODO: don't know what this does, I assume (if THS_WAIT means that the thread is waiting for a SEMA)
 * that this will stop that waiting thread from trying to access whatever the SEMA is blocking.
 */
s32 releaseWaitThread(s32 t_id);

/**
 * Gets the status of a thread and saves it into *info
 *
 * TODO: fact check statuses
 * TODO: wait might mean that it is waiting on a SEMA?
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
