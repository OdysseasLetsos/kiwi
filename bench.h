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

//arxikopoiw struct etsi vste na parw ta orismata gia ta nhmata gia thn kathe leitourgia
struct data {
	long int count;
	int r;
	long int threads;	
};

//arxikopoiw sunarthsh etsi wste na ektupwsw ta statistika xronou thw kathe leitourgias read kai write
void printer(char* action,long int count, struct  data threads_args);

//sunarthsh etsi vste ta nhmata na treksoun aythn me ta orismata pou tou exw orisei
void * write_thread(void* arg);
void * read_thread(void* arg);

//arxikopoiw sunarthsh pou ylopoioun to sunoliko kostow gia thn kathe leitourgia
double costwrites(double cost,double newcost);
double costreads(double cost,double newcost);

//arxikopoiw kleidaria gia reads kai writes
pthread_mutex_t mutexofreads;
pthread_mutex_t mutexofwrites;

//arxikopoiw antistoixa to kostos gia ta read kai ta write
double costofreads;
double costofwrites;
