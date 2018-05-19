#ifndef THREAD_C_
#define THREAD_C_

#include <kernel.h>
#include <tamtypes.h>
#include <string.h>

#include <Thread.h>

/**
 * Returns the t_id of the calling thread.
 */
s32 getCurrentThread() {
	return GetThreadId();
}

/**
 * Sets the priority of a given thread.
 * A threads priority can be between 0..127 inclusively (unchecked by implementation)
 */
s32 setThreadPriority(s32 t_id, s32 priority) {
	return ChangeThreadPriority(t_id, priority);
}

/**
 * Returns the size of the stack for a given thread.
 */
int getThreadStackSize(s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	return status->stack_size;
}

/**
 * Returns a pointer to the contents of a given threads stack.
 * WARNING: I highly recommend you don't mess with this, cause you will probably break something.
 */
char * getThreadStack(s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	return status->stack;
}

/**
 * Creates a thread with a given priority, stack, gp register, attr, and option.
 *
 * Thread.h has comments for working default values.  In all honesty I still have no idea what some
 * of these do.
 *
 * @param function Pointer to a function that contains the code this thread will execute.
 * @param stack The memory location that will be used for the threads stack.
 * @param gp_register Points to a general purpose register, by defining 'extern void * _gp;' PS2SDK
 *                    probably takes care of it.
 * @param priority The priority that this thread should use when being scheduled.
 * @param attr TODO: ?
 * @param option TODO: ?
 */
s32 createThread(void * function, char stack[], void * gp_register, int priority, u32 attr, u32 option) {
	ee_thread_t * t;
	t = malloc(sizeof(ee_thread_t));
	t->func = function;
	t->stack = stack;
	t->stack_size = sizeof(stack);
	t->gp_reg = (void *) gp_register;
	t->initial_priority = priority;
	t->attr = attr;
	t->option = option;
	return CreateThread(t);
}

/**
 * Start the given thread with arguments.
 * Arguments can be anything, you pass it to your thread, and define them in your thread definition.
 *
 * NOTE: when starting threads, the order they are started is how they are scheduled, priority only
 * matters when they get rescheduled.
 *
 * NOTE: It would seem that you cannot start a Thread that was started previously whose status has
 * resulted in THS_DORMANT by being exited, terminated, or completed successfully.
 */
s32 startThread(s32 t_id, void * args) {
	return StartThread(t_id, args);
}

/**
 * Stops this thread, un-scheduling it, and setting its status to THS_DORMANT.
 */
void exitThread() {
	ExitThread();
}

/**
 * Stops this thread and deletes it from memory.
 */
void exitDeleteThread() {
	ExitDeleteThread();
}

/**
 * Deletes a given thread from memory.
 * A thread that has finished execution is not deleted by default.
 */
s32 deleteThread(s32 t_id) {
	return DeleteThread(t_id);
}

/**
 * Stops a given thread, un-scheduling it, and setting its status to THS_DORMANT.
 *
 * NOTE: terminateThread can NOT terminate the calling thread, instead use exitThread().
 */
s32 terminateThread(s32 t_id) {
	return TerminateThread(t_id);
}

/**
 * Stop executing the current thread and start the next ready thread whose priority <= the priority
 * parameter.  If any threads are the same priority they operate in round-robin.
 *
 * NOTE: Crashes if no threads with status THS_READY are remaining with priority <= priority parameter.
 */
s32 rotateThreadReadyQueue(s32 priority) {
	return RotateThreadReadyQueue(priority);
}

/**
 * Releases a given thread that is waiting for a semaphore to unblock.
 *
 * TODO: don't know what this does, I assume (if THS_WAIT means that the thread is waiting for
 *       a semaphore) that this will stop that waiting thread from trying to access whatever
 *       the semaphore is blocking.
 */
s32 releaseWaitThread(s32 t_id) {
	return ReleaseWaitThread(t_id);
}

/**
 * Gets the status of a thread and saves it into *info
 *
 * Statuses are: TODO: fact check statuses
 * THS_RUN		    0x01    Thread is currently running.
 * THS_READY	    0x02    Thread is ready to run and has been queued.
 * TODO: wait might mean that it is waiting on a semaphore?
 * THS_WAIT	        0x04    Thread is waiting?
 * THS_SUSPEND	    0x08    Thread has been suspended.
 * THS_WAITSUSPEND	0x0c    Thread is waiting and suspended?
 * THS_DORMANT	    0x10    Thread has not been started yet, exited, or terminated.
 *
 * WARNING: It appears that trying to call this on a sleeping thread does not work
 * (the i* version also does not seem to work) Syscall: undefined (0)
 */
s32 referThreadStatus(s32 t_id, ee_thread_status_t *info) {
	return ReferThreadStatus(t_id, info);
}

/**
 * Makes the current thread sleep until WakeupThread is called on its t_id.
 *
 * NOTE: Sometimes threads can ignore sleep to prevent deadlocking by all threads sleeping.
 */
s32 sleepThread() {
	return SleepThread();
}

/**
 * Wakes up a given thread that is sleeping.
 *
 * TODO: check time of sleep and compare length when waking to confirm this.
 * NOTE: Sometimes the wrong thread can wake up instead if the other thread has equal or lower
 * priority and has been sleeping longer.  Meaning this doesn't wake up t_id, just wake up longest
 * sleeping thread with equal or lower priority.
 *
 * NOTE: Does nothing if t_id is the t_id of the calling thread, supposedly this is a bug fixed by
 * the i* equivalent anyways...
 *
 * WARNING: Causes a Trap exception if the thread is not sleeping.
 */
s32	wakeupThread(s32 t_id) {
	return WakeupThread(t_id);
}

/**
 * Cancels a wake-up call on a given thread.
 *
 * I assume this functions is to cancel a call to WakeupThread, where later during execution of the
 * calling thread it is determined that the other thread should not be woken up, before the calling
 * thread releases its execution slot to the scheduler.
 *
 * TODO: prove this works
 */
s32	cancelWakeupThread(s32 t_id) {
	return CancelWakeupThread(t_id);
}

/**
 * Suspends a given thread.
 * A suspended thread is paused and will not be rescheduled or continue executing.
 *
 * TODO: It is unclear if there is a way to restart/continue a suspended thread..
 */
s32	suspendThread(s32 t_id) {
	return SuspendThread(t_id);
}

/**
 * TODO: doesn't seem to be resuming a thread that has been suspended..?
 *
 * Appears to re-initialize the VIF0, VIF1, and GIF
 */
s32	resumeThread(s32 t_id) {
	return ResumeThread(t_id);
}

#endif /* THREAD_C_ */
