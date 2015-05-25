/* TuxRT_test 
 * A simple program to test different scheduling policy under
 * GNU/Linux.
 * Copyright (C) 2015
 * 
 * Authors: 
 * Gianluigi Biancucci 
 * 
 * License: 
 * This program comes with ABSOLUTELY NO WARRANTY;
 * This is free software under the terms of GPLV2 license
 * and you are welcome to redistribute it under certain condition
 */

/*
 * access to lots of nonstandard GNU/Linux extension functions.
 * NON-POSIX standard beaviour. Use with caution.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include "wrap_time.h"

void *thread1_loop(void *ptr);
void *thread2_loop(void *ptr);

int count1 = 0;
int count2 = 0;

//times execution
int times = 100000; 

double th_duration1;
double th_duration2;

#define MIN_PRIO(x) ( sched_get_priority_min(x) )
#define MAX_PRIO(x) ( sched_get_priority_max(x) )

void  print_usage(){
    printf("Linux RT test bench\n");
    printf("Author: Gianluigi Biancucci (gianluigi85[at]gmail.com)\n");
    printf("License: GPLV2\n\n");
    printf("Usage: ./TuxRT_test [options]\n\n");
    printf("Options:\n");
    printf("  -o,  SCHED_OTHER T1 = T2\n");
    printf("  -f1, SCHED_FIFO  T1 > T2\n");
    printf("  -f2, SCHED_FIFO  T1 < T2\n");
    printf("  -f3, SCHED_FIFO  T1 = T2\n");
    printf("  -r1, SCHED_RR    T1 > T2\n");
    printf("  -r2, SCHED_RR    T1 < T2\n");
    printf("  -r3, SCHED_RR    T1 = T2\n");    
    
}


int set_single_core(){
    
    //Setting CPU affinity
	int num_CPUs = 1; // only run on CPU 1 [CPU from 0 to 3 ]
	cpu_set_t mask;

	/* CPU_ZERO initializes all the bits in the mask to zero. */
	CPU_ZERO(&mask);
	/* CPU_SET sets only the bit corresponding to cpu. */
	CPU_SET(num_CPUs, &mask);
	/* sched_setaffinity returns 0 in success */
	return ( sched_setaffinity(0, sizeof(mask), &mask) );
    
}

int set_sched_options(pthread_t* thread, pthread_attr_t* attr, 
                      struct sched_param* param, unsigned char policy,
                      unsigned char priority)
{    
    pthread_attr_init(attr);
    int min_prio = sched_get_priority_min(policy);
    int max_prio = sched_get_priority_max(policy);
    /*
     * Return error code if requested priority is invalid, or
     * requested scheduler policy is invalid
     */
    if ( priority < min_prio || priority > max_prio || 
         max_prio == -1      || min_prio == -1        )
        return (-1);
        
    param->sched_priority = priority;
        
	pthread_attr_setschedpolicy(attr, policy);

	pthread_attr_setschedparam(attr, param);

	return (0);
    
}


enum {
    OTHER_TEST, // T1 = T2
    FIFO_TEST1, // T1 > T2
    FIFO_TEST2, // T2 > T1
    FIFO_TEST3, // T1 = T2
    RR_TEST1, // T1 > T2
    RR_TEST2, // T2 > T1
    RR_TEST3  // T2 = T1

};

struct timespec tim, tim2;

void do_something(){
    int k,j = 0;     
    for ( k = 0; k < 1000000; k++)
        j++;
}

int main(int argc, char *argv[])
{

    int selected_test = 0;
    
    if( argc < 2){
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    if (strcmp(argv[1], "-o") == 0)
        selected_test = OTHER_TEST;
    else if (strcmp(argv[1], "-f1") == 0)
        selected_test = FIFO_TEST1;
    else if (strcmp(argv[1], "-f2") == 0)
        selected_test = FIFO_TEST2;
    else if (strcmp(argv[1], "-f3") == 0)
        selected_test = FIFO_TEST3;
    else if (strcmp(argv[1], "-r1") == 0)
        selected_test = RR_TEST1;
    else if (strcmp(argv[1], "-r2") == 0)
        selected_test = RR_TEST2;
    else if (strcmp(argv[1], "-r3") == 0)
        selected_test = RR_TEST3;
    else {
        print_usage();
        exit(EXIT_FAILURE);
    }

	pthread_t th1, th2;
    pthread_attr_t attr1, attr2;
	struct sched_param parm1, parm2;
	int nice1 = 0;
	int nice2 = 0;
	int th1_r, th2_r;

	if (set_single_core() == -1)
	{
		printf("Could not set CPU Affinity \n");
        exit(EXIT_FAILURE);
	}
    
        printf("executing...\n");
    switch (selected_test) {
        
        case OTHER_TEST:
            printf ("selected OTHER_TEST\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_OTHER, MAX_PRIO(SCHED_OTHER) ); // Max and min prio

            set_sched_options(&th2, &attr2, &parm2, SCHED_OTHER, MIN_PRIO(SCHED_OTHER) ); // Are the same on sched_other
            //Nice range: from -20 to 19
            nice1 = -20;
            
            nice2 =  19;
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_OTHER, &parm1);
            
            do_something(); 
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_OTHER, &parm2);
            break;
            
        case FIFO_TEST1:
            printf ("selected FIFO_TEST1\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_FIFO, MAX_PRIO(SCHED_FIFO) );

            set_sched_options(&th2, &attr2, &parm2, SCHED_FIFO, MIN_PRIO(SCHED_FIFO) );
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_FIFO, &parm1);
            
            do_something(); 
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_FIFO, &parm2);
            break;
            
        case FIFO_TEST2:
            printf ("selected FIFO_TEST2\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_FIFO, MIN_PRIO(SCHED_FIFO) );

            set_sched_options(&th2, &attr2, &parm2, SCHED_FIFO, MAX_PRIO(SCHED_FIFO) );
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_FIFO, &parm1);
                    
            do_something();                    
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_FIFO, &parm2);
            break;
            
        case FIFO_TEST3:
            printf ("selected FIFO_TEST3\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_FIFO, MIN_PRIO(SCHED_FIFO) );

            set_sched_options(&th2, &attr2, &parm2, SCHED_FIFO, MIN_PRIO(SCHED_FIFO) );
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_FIFO, &parm1);
                    
            do_something(); 
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_FIFO, &parm2);
            break;
            
        case RR_TEST1:
            printf ("selected RR_TEST1\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_RR, MAX_PRIO(SCHED_RR) );

            set_sched_options(&th2, &attr2, &parm2, SCHED_RR, MIN_PRIO(SCHED_RR) );
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_RR, &parm1);
            
            do_something();                     
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_RR, &parm2);
            break;
        
        case RR_TEST2:
            printf ("selected RR_TEST2\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_RR, MIN_PRIO(SCHED_RR) );

            set_sched_options(&th2, &attr2, &parm2, SCHED_RR, MAX_PRIO(SCHED_RR) );
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_RR, &parm1);
            
            do_something(); 
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_RR, &parm2);
            break;
            
        case RR_TEST3:
            printf ("selected RR_TEST3\n\n");
            sleep(1);
            set_sched_options(&th1, &attr1, &parm1, SCHED_RR, 90);//MIN_PRIO(SCHED_RR) );

            set_sched_options(&th2, &attr2, &parm2, SCHED_RR, 90);//MIN_PRIO(SCHED_RR) );
            /* 
             * Launching the 2 threads
             */
            th1_r = pthread_create(&th1, &attr1, (void*) thread1_loop, (void*) &nice1);
            pthread_setschedparam(th1, SCHED_RR, &parm1);
            
            do_something(); 
                    
            th2_r = pthread_create(&th2, &attr2, (void*) thread2_loop, (void*) &nice2);
            pthread_setschedparam(th2, SCHED_RR, &parm2);
            break;
        
        default:
            break;
            
    }

   /*
    *  Wait until thread1 and thread2 are complete.
    */    
   	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	printf("Thread 1 runs %d times,\nThread 2 runs %d times.\n\n", count1, count2);
    printf("Thread 1 return value: %d\n", th1_r);
	printf("Thread 2 return value: %d\n", th2_r);
    printf("Thread 1 duration mean value [ns]: %12.0f\n", th_duration1);
    printf("Thread 2 duration mean value [ns]: %12.0f\n", th_duration2);

    exit(EXIT_SUCCESS);
}


T_TIME before1;
T_TIME after1;
char nice1_set = 0;
void * thread1_loop(void *ptr) {
    if(!nice1_set){
        nice1_set = 1;
        int *n;
        n = (int *) ptr;
        nice(*n);
    }
    sleep(1);
	while (times > 0) {
		int i = 0;
        get_time(&before1);
        //delay, thread_body
		for (i = 0; i < 20000; i++)
			i++; 
        get_time(&after1);
        count1++;
        unsigned long diff = diff_time_nsec(before1, after1);
        th_duration1 = ( (th_duration1 * (count1 - 1) + (double) diff ) / count1 );
		times--;
	}
	return (void*) NULL;
}

T_TIME before2;
T_TIME after2;
char nice2_set = 0;
void * thread2_loop(void *ptr) {
    if(!nice2_set){
        nice2_set = 1;
        int *n;
        n = (int *) ptr;
        nice(*n);        
    }
    sleep(1);
	while (times > 0) {
		int i = 0;
        get_time(&before2);
        //delay, thread_body
		for (i = 0; i < 20000; i++)
			i++;
        get_time(&after2);
        count2++;
        unsigned long diff = diff_time_nsec(before2, after2);
        th_duration2 = ( (th_duration2 * (count2 - 1) + (double) diff ) / count2 );
		times--;
	}

	return (void*) NULL;
}
