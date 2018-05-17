#ifndef THREAD_C_
#define THREAD_C_

#include <kernel.h>
#include <tamtypes.h>
#include <string.h>

#include <Thread.h>

s32 getCurrentThread() {
	return GetThreadId();
}

int getThreadStatus(s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	return status->status;
}

s32 setThreadPriority(s32 t_id, s32 priority) {
	return ChangeThreadPriority(t_id, priority);
}

int getThreadStackSize(s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	return status->stack_size;
}

char * getStackContents(s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	return status->stack;
}

//----------------------------------------------------------------------------------------------------
// Thread management functions

s32 createThread(void * function, char stack[], void * gp_register, int priority, u32 attr, u32 option) {
	ee_thread_t * t;
	t = malloc(sizeof(ee_thread_t));
	t->func = function;
	t->stack = stack;
	t->stack_size = sizeof(stack);
	t->gp_reg = gp_register;
	t->initial_priority = priority;
	t->attr = attr;
	t->option = option;
	return CreateThread(t);
}

s32 startThread(s32 t_id, void * args) {
	return StartThread(t_id, args);
}

void exitThread() {
	ExitThread();
}

void exitDeleteThread() {
	ExitDeleteThread();
}

s32 deleteThread(s32 t_id) {
	return DeleteThread(t_id);
}

s32 terminateThread(s32 t_id) {
	return TerminateThread(t_id);
}

//----------------------------------------------------------------------------------------------------
// Thread scheduling functions

s32 rotateThreadReadyQueue(s32 priority) {
	return RotateThreadReadyQueue(priority);
}

s32 releaseWaitThread(s32 t_id) {
	return ReleaseWaitThread(t_id);
}

s32 referThreadStatus(s32 t_id, ee_thread_status_t *info) {
	return ReferThreadStatus(t_id, info);
}

//----------------------------------------------------------------------------------------------------
// Thread sleep functions

s32 sleepThread() {
	return SleepThread();
}

s32	wakeupThread(s32 t_id) {
	return WakeupThread(t_id);
}

s32	cancelWakeupThread(s32 t_id) {
	return CancelWakeupThread(t_id);
}

//----------------------------------------------------------------------------------------------------
// Thread suspension functions
// NOTE: Suspend is often deprecated (although not documented here) as it is prone to deadlocks
// The nature of sleep in PS2's context doesn't seem to add any safer functionality though.

s32	suspendThread(s32 t_id) {
	return SuspendThread(t_id);
}

s32	resumeThread(s32 t_id) {
	return ResumeThread(t_id);
}

//----------------------------------------------------------------------------------------------------

#endif /* THREAD_C_ */