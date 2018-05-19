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

static void thread1fun(void);
static void thread2fun(void);
static void thread3fun(void);

static int t1_done = 0;
static int t2_done = 0;
static int t3_done = 0;

static int t1_notify = 0;
static int t2_notify = 0;
static int t3_notify = 0;

int main(int argc, char **argv) {
	s32 thread[4];
	int thread_count = sizeof(thread) / sizeof(s32);

	thread[0] = getCurrentThread();
	printf("Setting main threads priority to 0. Result = %d\n", setThreadPriority(thread[0], 0));

	printf("Creating other threads...\n");
	// Create some threads
	// TODO: prove this -> Since Thread 3 has a higher priority than Thread 1, Thread 3 should be the Thread that wakes Thread 2.
	thread[1] = createThread(thread1fun, thread1_stack, (void *)&_gp, 50, default_attr, default_option);
	thread[2] = createThread(thread2fun, thread2_stack, (void *)&_gp, 50, default_attr, default_option);
	thread[3] = createThread(thread3fun, thread3_stack, (void *)&_gp, 10, default_attr, default_option);

	printf("Thread id's are: \nMain Thread: %d\nThread 1: %d\nThread 2: %d\nThread 3: %d\n", thread[0], thread[1], thread[2], thread[3]);

	// Make sure they got created successfully
	if(thread[1] < 0) printf("Failed to create thread 1.\n");
	if(thread[2] < 0) printf("Failed to create thread 2.\n");
	if(thread[3] < 0) printf("Failed to create thread 3.\n");

	// Print the status of each thread.
	printf("The status of each thread before being started is:\n");
	updateThreadStatus(thread, thread_count);

	// Start the threads
	startThread(thread[1], NULL);
	startThread(thread[2], NULL);
	startThread(thread[3], NULL);

	// Print the status again.
	printf("The status of each thread after being started is:\n");
	updateThreadStatus(thread, thread_count);

	// Use the main thread to keep track of completion of the other threads.
	while(t1_done == 0 || t2_done == 0 || t3_done == 0) {
		if(t1_done == 1 && t1_notify == 0){
			printf("Thread 1 has completed.\nI will now wake up Thread 2.\n");
			//wakeupThread(thread[2]);
			t1_notify = 1;
		}
		if(t2_done == 1 && t2_notify == 0) {
			printf("Thread 2 has completed.\n");
			t2_notify = 1;
		}
		if(t3_done == 1 && t3_notify == 0) {
			printf("Thread 3 has completed.\nI will now wake up Thread 2.\n");
			//wakeupThread(thread_2);
			t3_notify = 1;
		}

		printf("The status of each thread is:\n");
		updateThreadStatus(thread, thread_count);
		//sleepThread();
		//rotateThreadReadyQueue(10); -> nothing???
	}

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
// I think what happens is the buffer gets over-written before it can complete.
// Occasionally it will tell me the memory address cannot be written.
static void thread1fun() {
	t1_done = 1;
	exitThread();
}

static void thread2fun() {
	sleepThread();
	t2_done = 1;
	exitThread();
}

static void thread3fun() {
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
