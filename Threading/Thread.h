/**
 * NOTE: EE threads not IOP
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <tamtypes.h>

// TODO: I would like to have some more useful threading functionality.
// TODO: I can make a sort of thread manager with high priority that can suspend/resume or sleep/wakeup to fake stronger scheduling

// TODO: find out how aligned works, some thread stacks aligned 64?
static char default_stack[16*1024] __attribute__((aligned(16)));
static void * default_gp_reg;
static s32 default_priority = 50;
static u32 default_attr = 0x02000000;
static u32 default_option = 0;

/**
 * Returns the t_id of the current thread (the one this line of code resides in when called)
 */
s32 getCurrentThread(void);

/**
 * Returns the status of the thread.
 * TODO: fact check statuses
 *
 * THS_RUN		    0x01    Thread is running
 * THS_READY	    0x02    Thread is ready to run
 * THS_WAIT	        0x04    Thread is waiting
 * THS_SUSPEND	    0x08    Thread has been suspended
 * THS_WAITSUSPEND	0x0c    Thread is waiting and suspended
 * THS_DORMANT	    0x10    Thread is not doing anything
 */
int getThreadStatus(s32 t_id);

/**
 * Sets the priority of a given thread.
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
 * Defined in Thread.h are default values for each feild, so that you dont have to spend a week trying to find what
 * half these parameters do, and what an appropriate value is.  In all honesty I still have no idea what these are/do
 * gp_register - points to a register usually, but no idea why
 * attr - absolutely no idea
 * option - no idea, PS2SDK says it doesn't work, but I have seen code that does use it.
 */
s32 createThread(void * function, char stack[], void * gp_register, s32 priority, u32 attr, u32 option);

/**
 * Start the given thread with arguments.
 * Arguments can be anything, you pass it to your thread, and define them in your thread definition.
 */
s32 startThread(s32 t_id, void * args);

/**
 * Exits the thread this thread.
 * TODO: unsure if this means thread has completed, or release thread so another can be scheduled
 */
void exitThread(void);

/**
 * Exits and deletes this thread.
 * TODO: unsure if this means thread has completed, or release thread so another can be scheduled
 */
void exitDeleteThread(void);

/**
 * Deletes a given thread from memory.  A thread that has finished execution is not deleted.
 */
s32 deleteThread(s32 t_id);

/**
 * Terminates a given thread.
 * TODO: unsure if this means the thread will no longer be scheduled.
 */
s32 terminateThread(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread scheduling functions

/**
 * TODO: I assume this cycles through the threads with state READY for the scheduler, or maybe it does something to the priority?
 */
s32 rotateThreadReadyQueue(s32 priority);

/**
 * Releases a given waiting thread.
 * A thread that is waiting means that it is ready to be scheduled, just not currently executing.
 * TODO: no idea what this actually does, possibly drops a waiting thread, or gives it execution?
 */
s32 releaseWaitThread(s32 t_id);

/**
 * Gets the status of a thread and saves it into *info
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
 */
s32	wakeupThread(s32 t_id);

/**
 * Cancels a wake-up call on a given thread.
 *
 * I assume this functions is to cancel a call to WakeupThread, where later during execution of the
 * calling thread it is determined that the other thread should not be woken up, before the calling
 * thread releases its execution slot to the scheduler.
 */
s32	cancelWakeupThread(s32 t_id);

//----------------------------------------------------------------------------------------------------
// Thread suspension functions
// NOTE: Suspend is often deprecated (although not documented here) as it is prone to deadlocks
// The nature of sleep in PS2's context doesn't seem to add any safer functionality though.

/**
 * Suspends a given thread.
 * A suspended thread is paused and will not be rescheduled or continue executing until it is resumed.
 */
s32	suspendThread(s32 t_id);

/**
 * Resumes a given suspended thread.
 */
s32	resumeThread(s32 t_id);

//----------------------------------------------------------------------------------------------------

#endif /* THREAD_H_ */
