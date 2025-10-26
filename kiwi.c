#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"
#include <pthread.h>

// Define database
DB *db;

// Initialize mutex for read and write
pthread_mutex_t mutexofreads=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexofwrites=PTHREAD_MUTEX_INITIALIZER;

// Function that returns the cost of writes
double costwrites(double cost,double newcost){
	costofwrites=cost+newcost;
	return costofwrites;
}

// Function that returns the cost of reads
double costreads(double cost,double newcost){
	costofreads=cost+newcost;
	return costofreads;
}

// Write test function, with threads as additional argument to split operations according to threads
void _write_test(long int count, int r,long int threads)
{
	int i;
	double cost;
	long long start,end;
	Variant sk, sv;
	

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	start = get_ustime_sec();
	// Counter for each operation according to threads
	long int counter=(long int)count/threads;

	for (i = 0; i < counter; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	end = get_ustime_sec();
	cost = end -start;
	
	// Different locks for each read and write
	// Lock/unlock critical section
	pthread_mutex_lock(&mutexofwrites);
	// Total cost is added for each thread performing write. The new cost is updated to ensure
	// no other thread can access the same point simultaneously and execution time is updated accordingly
	costofwrites=costwrites(cost,costofwrites);
	pthread_mutex_unlock(&mutexofwrites);
		
}

// Read test function, also with threads as additional argument to split operations according to threads
void _read_test(long int count, int r,long int threads)
{
	int i;
	int ret;
	int found = 0;
	double cost;
	long long start,end;
	Variant sk;
	Variant sv;
	char key[KSIZE + 1];
	long int counter;
	
	start = get_ustime_sec();
	// Counter for each operation according to threads
	counter=(long int)count/threads;
	for (i = 0; i < counter;i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r){
			_random_key(key, KSIZE);
		}else{
			snprintf(key, KSIZE, "key-%d", i);
		}
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret =db_get(db, &sk, &sv);
		if (ret) {
			// Count found entries
			// db_free_data(sv.mem);	
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	end = get_ustime_sec();
	cost = end - start;

	// Lock/unlock critical section, mutual exclusion for each thread for total read cost
	pthread_mutex_lock(&mutexofreads);
	// Store in variable costofreads, updated separately by each thread
	costofreads=costreads(cost,costofreads);
	pthread_mutex_unlock(&mutexofreads);
	
}
