#include <kernel.h>
#include <tamtypes.h>
#include <stdio.h>
#include <string.h>

#include <Thread.c>

static char thread1_stack[16*1024] __attribute__((aligned(16)));
static char thread2_stack[16*1024] __attribute__((aligned(16)));
static char thread3_stack[16*1024] __attribute__((aligned(16)));

static void updateThreadStatus(void);
static const char * getThreadStatusAsString(s32 t_id);

static void thread1fun(void);
static void thread2fun(void);
static void thread3fun(void);

static s32 thread_main = 0;
static s32 thread_1 = 0;
static s32 thread_2 = 0;
static s32 thread_3 = 0;

static char t1_status[15];
static char t2_status[15];
static char t3_status[15];

static int t1_done = 0;
static int t2_done = 0;
static int t3_done = 0;

//TODO: check out SDL CLOSELY caus emaybe theres somthign important there
// also maube gp is important for sleep and whatever, IDK

int main(int argc, char **argv) {
	thread_main = getCurrentThread();
	printf("Creating threads...\n");
	// Create some threads
	thread_1 = createThread(thread1fun, thread1_stack, default_gp_reg, 50, default_attr, default_option);
	thread_2 = createThread(thread2fun, thread2_stack, default_gp_reg, 40, default_attr, default_option);
	thread_3 = createThread(thread3fun, thread3_stack, default_gp_reg, 10, default_attr, default_option);

	// Make sure they got created successfully
	if(thread_1 < 0) printf("Failed to create thread 1.\n");
	if(thread_2 < 0) printf("Failed to create thread 2.\n");
	if(thread_3 < 0) printf("Failed to create thread 3.\n");

	updateThreadStatus();
	printf("The status of each thread before being started is:\nThread 1: %s\nThread 2: %s\nThread 3: %s\n", t1_status, t2_status, t3_status);

	// Start the threads, since PS2 is non-premptive the threads cannot be executed
	// concurrently with the thread this code runs in, so they are just set to THS_READY
	startThread(thread_1, NULL);
	startThread(thread_2, NULL);
	startThread(thread_3, NULL);

	updateThreadStatus();
	printf("The status of each thread after being started is:\nThread 1: %s\nThread 2: %s\nThread 3: %s\n", t1_status, t2_status, t3_status);

	// exitThread(); - crashes
	// sleepThread(); // crash after printing 'Th'

	while (1) {
		if(t1_done != 0 && t2_done != 0 && t3_done != 0) {
			printf("All 3 threads have completed execution.");
		}
	}
	return 0;
}

static void thread1fun() {
	printf("Thread 1 started!\n");
	t1_done = 1;
	exitThread();
	return;
}
static void thread2fun() {
	printf("Thread 2 started!\n");
	t2_done = 1;
	exitThread();
	return;
}
static void thread3fun() {
	printf("Thread 1 started!\n");
	t3_done = 1;
	exitThread();
	return;
}

static void updateThreadStatus() {
	strcpy(t1_status, getThreadStatusAsString(thread_1));
	strcpy(t2_status, getThreadStatusAsString(thread_2));
	strcpy(t3_status, getThreadStatusAsString(thread_3));
}

static const char * getThreadStatusAsString(s32 t_id) {
	switch(getThreadStatus(t_id)) {
		case(0x01): return "THS_RUN\0";
		case(0x02): return "THS_READY\0";
		case(0x04): return "THS_WAIT\0";
		case(0x08): return "THS_SUSPEND\0";
		case(0x0c): return "THS_WAITSUSPEND\0";
		case(0x10): return "THS_DORMANT\0";
	}
	return "\0";
}