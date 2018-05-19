#include <kernel.h>
#include <tamtypes.h>
#include <stdio.h>
#include <string.h>

#include <Thread.c>

static char thread1_stack[16*1024] __attribute__((aligned(16)));
static char thread2_stack[16*1024] __attribute__((aligned(16)));
static char thread3_stack[16*1024] __attribute__((aligned(16)));

static void updateThreadStatus(s32 * threads, int count);
static char * getThreadStatusAsString(s32 t_id);

static void thread1fun(void * args);
static void thread2fun(void * args);
static void thread3fun(void * args);

static int volatile t1_sleep = 0;
static int volatile t2_sleep = 0;
static int volatile t3_sleep = 0;

static int volatile t1_done = 0;
static int volatile t2_done = 0;
static int volatile t3_done = 0;

int main(int argc, char **argv) {
	s32 thread[4];
	int thread_count = sizeof(thread) / sizeof(s32);

	thread[0] = getCurrentThread();
	printf("Setting main threads priority to 0. Result = %d\n", setThreadPriority(thread[0], 0));

	printf("Creating other threads...\n");
	// Create some threads
	thread[1] = createThread(thread1fun, thread1_stack, (void *)&_gp, 10, 0x02000000, 0);
	thread[2] = createThread(thread2fun, thread2_stack, (void *)&_gp, 50, 0x02000000, 0);
	thread[3] = createThread(thread3fun, thread3_stack, (void *)&_gp, 50, 0x02000000, 0);

	// Make sure they got created successfully
	if(thread[1] < 0) printf("Failed to create thread 1.\n");
	if(thread[2] < 0) printf("Failed to create thread 2.\n");
	if(thread[3] < 0) printf("Failed to create thread 3.\n");

	printf("Thread id's are: \nMain Thread: %d\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", thread[0], thread[1], thread[2], thread[3]);

	// Print the status of each thread.
	printf("The status of each thread before being started is:\n");
	updateThreadStatus(thread, thread_count);

	// Start the threads
	printf("Starting threads...:\n");
	// NOTE: when starting threads, the order they are started is how they are scheduled, priority only matters when they get rescheduled.
	startThread(thread[1], (void *) thread[0]);
	startThread(thread[2], (void *) thread[0]);
	startThread(thread[3], (void *) thread[0]);

	int t1_notify = 0;
	int t2_notify = 0;
	int t3_notify = 0;

	printf("Threads sleeping:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_sleep, t2_sleep, t3_sleep);

	printf("Rotating queue (50)...:\n");
	//rotateThreadReadyQueue(50);

	printf("Threads sleeping:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_sleep, t2_sleep, t3_sleep);

	// Use the main thread to keep track of completion of the other threads.
	// NOTE: For some reason the loop never executes if there is no operation inside that can always be done, I assume the scheduler does this?
	do {
		if(t1_done == 1 && t1_notify == 0){
			t1_notify = 1;
			wakeupThread(thread[3]);
			printf("Thread 1 has completed.\n");
			printf("Threads completed:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_done, t2_done, t3_done);
			printf("Threads sleeping:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_sleep, t2_sleep, t3_sleep);
		}

		if(t2_done == 1 && t2_notify == 0) {
			t2_notify = 1;
			printf("Thread 2 has completed.\n");
			printf("Threads completed:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_done, t2_done, t3_done);
			printf("Threads sleeping:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_sleep, t2_sleep, t3_sleep);
		}

		if(t3_done == 1 && t3_notify == 0) {
			t3_notify = 1;
			// wakeupThread(thread[2]); -> causes a crash...
			printf("Thread 3 has completed.\n");
			printf("Threads completed:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_done, t2_done, t3_done);
			printf("Threads sleeping:\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", t1_sleep, t2_sleep, t3_sleep);
		}
	} while(t1_done == 0 || t2_done == 0 || t3_done == 0);

	printf("The status of each thread after execution is:\n");
	updateThreadStatus(thread, thread_count);

	printf("All 3 threads have completed execution.\n");

	printf("Cleaning up threads...\n");
	deleteThread(thread[1]);
	deleteThread(thread[2]);
	deleteThread(thread[3]);

	printf("Threads cleaned up...\nNow I will free the stack memory they where using.\n");
	free(thread1_stack);
	free(thread2_stack);
	free(thread3_stack);

	printf("Threads demo has completed!");
	return (0);
}

// WARNING: printf in multi-threading is a bad time.
// I think what happens is the buffer gets over-written before it can complete / or actual concurrency is causing an issue.
// Occasionally it will tell me the memory address cannot be written.
static void thread1fun(void * args) {
	s32 * main_thread = (s32 *)(args);
	t1_done = 1;
	exitThread();
}

static void thread2fun(void * args) {
	t2_sleep = 1;
	sleepThread();
	t2_sleep = 0;
	s32 * main_thread = (s32 *)(args);
	t2_done = 1;
	exitThread();
}

static void thread3fun(void * args) {
	t3_sleep = 1;
	sleepThread();
	t3_sleep = 0;
	s32 * main_thread = (s32 *)(args);
	t3_done = 1;
	exitThread();
}

static void updateThreadStatus(s32 * threads, int count) {
	int index = 0;
	while(index < count) {
		printf("Thread %d: %s\n", threads[index], getThreadStatusAsString(threads[index]));
		index++;
	}
}

static char * getThreadStatusAsString(s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	switch(status->status) {
		case(0x01): return "THS_RUN\0";
		case(0x02): return "THS_READY\0";
		case(0x04): return "THS_WAIT\0";
		case(0x08): return "THS_SUSPEND\0";
		case(0x0c): return "THS_WAITSUSPEND\0";
		case(0x10): return "THS_DORMANT\0";
		default: return "COULD_NOT_READ\0";
	}
}
