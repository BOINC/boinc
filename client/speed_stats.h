//#define RUN_TEST

#define THOUSAND    1000
#define MILLION     THOUSAND*THOUSAND

#define D_LOOP_ITERS			1*MILLION
#define I_LOOP_ITERS			1*MILLION
#define MEM_SIZE			1*MILLION

#define NUM_DOUBLES       28
#define NUM_INTS          28

#define CACHE_MIN 1024					/* smallest cache (in words) */
#define CACHE_MAX 512*1024					/* largest cache */
#define STRIDE_MIN 1   /* smallest stride (in words) */
#define STRIDE_MAX 128 /* largest stride */
#define SAMPLE 10						/* to get a larger time sample */
#define SECS_PER_RUN 0.2

#define MAX_TIME_TESTS_SECONDS	60
#define TIME_TESTS_RUNNING		0
#define TIME_TESTS_COMPLETE		1
#define TIME_TESTS_NOT_RUNNING	2	
#define TIME_TESTS_ERROR		3

int check_cache_size( int mem_size );
double double_flop_test( int iterations, int print_debug );
double int_op_test( int iterations, int print_debug );
double bandwidth_test( int iterations, int print_debug );
void run_test_suite( double num_secs_per_test );
double run_double_prec_test( double num_secs );
double run_int_test( double num_secs );
double run_mem_bandwidth_test( double num_secs );
int set_test_timer(double num_secs);
int destroy_test_timer();


