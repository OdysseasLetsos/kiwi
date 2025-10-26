#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

// Initialize struct to pass arguments to threads for each operation
struct data {
	long int count;
	int r;
	long int threads;	
};

// Function to print timing statistics for each read and write operation
void printer(char* action,long int count, struct  data threads_args);

// Function for threads to run with the arguments we defined
void * write_thread(void* arg);
void * read_thread(void* arg);

// Function to calculate total cost for each operation
double costwrites(double cost,double newcost);
double costreads(double cost,double newcost);

// Initialize mutexes for reads and writes
pthread_mutex_t mutexofreads;
pthread_mutex_t mutexofwrites;

// Corresponding variables to store total cost for reads and writes
double costofreads;
double costofwrites;
