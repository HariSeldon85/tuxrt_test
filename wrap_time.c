#include "wrap_time.h"


/*
 * Sufficiently recent versions of GNU libc and the Linux kernel support the following clocks:

CLOCK_REALTIME
System-wide realtime clock. Setting this clock requires appropriate privileges.
CLOCK_MONOTONIC
Clock that cannot be set and represents monotonic time since some unspecified starting point.
CLOCK_PROCESS_CPUTIME_ID
High-resolution per-process timer from the CPU.
CLOCK_THREAD_CPUTIME_ID
Thread-specific CPU-time clock.
 * 
 */

void get_time(T_TIME * now)
{    
    clock_gettime(CLOCK_MONOTONIC, now);
}



struct timespec diff_time(T_TIME start, T_TIME end)
{
	struct timespec temp;
	temp.tv_sec = end.tv_sec - start.tv_sec;

	if ((end.tv_nsec-start.tv_nsec) < 0)
        temp.tv_sec--, temp.tv_nsec = NSEC_MAX - start.tv_nsec + end.tv_nsec;        
	else
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	return temp;
}



unsigned long diff_time_nsec(T_TIME start, T_TIME end)
{    
        return ( NSEC_MAX * (end.tv_sec - start.tv_sec)  + 
                end.tv_nsec - start.tv_nsec);


}
