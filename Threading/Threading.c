#include <kernel.h>
#include <tamtypes.h>
#include <stdio.h>
#include <string.h>

#include <Thread.c>

static char thread1_stack[16*1024] __attribute__((aligned(16)));
static char thread2_stack[16*1024] __attribute__((aligned(16)));
static char thread3_stack[16*1024] __attribute__((aligned(16)));

static void updateThreadStatus(void);
static void getThreadStatusAsString(char * t_status, s32 t_id);

static void thread1fun(void);
static void thread2fun(void);
static void thread3fun(void);

static s32 thread_main = 0;
static s32 thread_1 = 0;
static s32 thread_2 = 0;
static s32 thread_3 = 0;

static char * t1_status;
static char * t2_status;
static char * t3_status;

static int t1_done = 0;
static int t2_done = 0;
static int t3_done = 0;

static int t1_notify = 0;
static int t2_notify = 0;
static int t3_notify = 0;

int main(int argc, char **argv) {
	t1_status = malloc(15);
	t2_status = malloc(15);
	t3_status = malloc(15);

	thread_main = getCurrentThread();
	printf("Setting main threads priority to 0. Result = %d\n", setThreadPriority(thread_main, 0));

	printf("Creating other threads...\n");
	// Create some threads
	// Since Thread 3 has a higher priority than Thread 1, Thread 3 should be the Thread that wakes Thread 2.
	thread_1 = createThread(thread1fun, thread1_stack, (void *)&_gp, 50, default_attr, default_option);
	thread_2 = createThread(thread2fun, thread2_stack, (void *)&_gp, 50, default_attr, default_option);
	thread_3 = createThread(thread3fun, thread3_stack, (void *)&_gp, 10, default_attr, default_option);

	// Make sure they got created successfully
	if(thread_1 < 0) printf("Failed to create thread 1.\n");
	if(thread_2 < 0) printf("Failed to create thread 2.\n");
	if(thread_3 < 0) printf("Failed to create thread 3.\n");

	// Print the status of each thread.
	updateThreadStatus();
	printf("The status of each thread before being started is:\nThread 1: %s\nThread 2: %s\nThread 3: %s\n", t1_status, t2_status, t3_status);

	// Start the threads, since PS2 is non-premptive the threads cannot be executed
	// concurrently with the main thread, so they are just set to THS_READY
	startThread(thread_1, NULL);
	startThread(thread_2, NULL);
	startThread(thread_3, NULL);

	// Print the status again.
	updateThreadStatus();
	printf("The status of each thread after being started is:\nThread 1: %s\nThread 2: %s\nThread 3: %s\n", t1_status, t2_status, t3_status);

	// Use the main thread to keep track of completion of the other threads.
	// Note that because all 3 threads were ready while main was running,
	// all of them are scheduled before restarting main thread.
	while(t1_done == 0 || t2_done == 0 || t3_done == 0) {
		// TODO: need to stop thread here to allow others to run, I don't understand what its doing as it stands.
		if(t1_done == 1 && t1_notify == 0){
			printf("Thread 1 has completed.\nI will now wake up Thread 2.\n");
			wakeupThread(thread_2);
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

		updateThreadStatus();
		printf("The status of each thread is:\nThread 1: %s\nThread 2: %s\nThread 3: %s\n", t1_status, t2_status, t3_status);

		//sleepThread();
		//rotateThreadReadyQueue(10); -> nothing???
	}

	updateThreadStatus();
	printf("The status of each thread after execution is:\nThread 1: %s\nThread 2: %s\nThread 3: %s\n", t1_status, t2_status, t3_status);
	printf("All 3 threads have completed execution.\n");

	printf("Cleaning up threads...\n");
	deleteThread(thread_1);
	deleteThread(thread_2);
	deleteThread(thread_3);

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
	//sleepThread();
	t2_done = 1;
	exitThread();
}

static void thread3fun() {
	t3_done = 1;
	exitThread();
}

static void updateThreadStatus() {
	getThreadStatusAsString(t1_status, thread_1);
	getThreadStatusAsString(t2_status, thread_2);
	getThreadStatusAsString(t3_status, thread_3);
}

static void getThreadStatusAsString(char * t_status, s32 t_id) {
	ee_thread_status_t * status;
	status = malloc(sizeof(ee_thread_status_t));
	referThreadStatus(t_id, status);
	switch(status->status) {
		case(0x01): strcpy(t_status, "THS_RUN\0");         break;
		case(0x02): strcpy(t_status, "THS_READY\0");       break;
		case(0x04): strcpy(t_status, "THS_WAIT\0");        break;
		case(0x08): strcpy(t_status, "THS_SUSPEND\0");     break;
		case(0x0c): strcpy(t_status, "THS_WAITSUSPEND\0"); break;
		case(0x10): strcpy(t_status, "THS_DORMANT\0");     break;
		default: strcpy(t_status, "COULD_NOT_READ\0");
	}
}
