#include <time.h>

#define NSEC_MAX 1000000000

typedef struct timespec T_TIME;

void get_time(T_TIME * now);

struct timespec diff_time(T_TIME start, T_TIME end);

unsigned long diff_time_nsec(T_TIME start, T_TIME end);