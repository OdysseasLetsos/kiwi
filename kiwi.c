#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"
#include <pthread.h>

//orizw vash 
DB *db;

//arxikopoiw mutex gia read kai write
pthread_mutex_t mutexofreads=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexofwrites=PTHREAD_MUTEX_INITIALIZER;

//dhmiourgw sunarthsh pou epistrefei to kostos apo ta writes
double costwrites(double cost,double newcost){
	costofwrites=cost+newcost;
	return costofwrites;
}
//dhmiourgw sunarthsh pou epistrefei to kostos apo ta reads
double costreads(double cost,double newcost){
	costofreads=cost+newcost;
	return costofreads;
}

//sunarthsh write test san epipleon orisma ta threads etsi wste na ta spasw analoga me tiw leitourgies
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
	//metrhths gia kathe leitorugia analoga me threads
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
	

	//diaforetikes kleidaries gia kathe read kai write 
	//lock unlock krisimh perioxh 
	pthread_mutex_lock(&mutexofwrites);
	//sunoliko kostos prosthetete se kathe nhma pou ylopoiei to write to neo kostos amoibaios apokleismous etsi este na mhn vreuei kapoio allo nhma tautoxrona sto idio shmeio kai ananewsei ton xrono symfvna me ayto
	costofwrites=costwrites(cost,costofwrites);
	pthread_mutex_unlock(&mutexofwrites);
	
	
		
}
//sunarthsh read test epishs san epipleon orisma ta threads etsi wste na ta spasw analoga me tiw leitourgies
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
	//metrhths gia kathe leitorugia analoga me threads
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
			//posa vrethikan
			//db_free_data(sv.mem);	
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

	//lock unlock krisimh peioxh amoibaios apokleismos gia kathe nhma gia to sunoliko kostow twn read
	pthread_mutex_lock(&mutexofreads);
	//apothikeyetai sthn metavlith costofreads ananewnetai kathe fora apo kath nhma ksexwrista
	costofreads=costreads(cost,costofreads);
	pthread_mutex_unlock(&mutexofreads);
	
	
}
