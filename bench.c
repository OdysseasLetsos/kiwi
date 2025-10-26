#include "bench.h"
#include <pthread.h>//βιβλιοθηκη για νηματα 1
#include "../engine/db.h"
#define DATAS ("testdb")

//orizw thn vash
DB* db;


void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}







//orizw sunarthsh pou tha pairnei san orisma to struct me to count to r kai ta threads gia thn read_test
void * read_thread(void* arg){
	struct data *dataset=(struct data *) arg;
	//pernaw sthn synarthsh kai ta threads pou thelw
	_read_test(dataset->count,dataset->r,dataset->threads);
	return 0;
}

//orizw sunarthsh pou tha pairnei san orisma to struct me to count to r kai ta threads gia thn write_test
void * write_thread(void* arg){
	struct data *dataset=(struct data *) arg;
	_write_test(dataset->count,dataset->r,dataset->threads);
	return 0;
}






//dhmiourgw sunarthsh etsi wste na ektypwnw tou xronous gia thn kathe mia leitoyrgia
void printer(char* action,long int count, struct  data threads_args ){

    if (strcmp(action,"write") == 0) { //an h leitourgia einai write
        printf("SELECTED ACTION: __WRITE__\n");
        printf(LINE);
        printf("|Random-WRITE    (done:%ld, found:%ld): %.6f sec/op; %.1f writes per sec(estimated); cost:%.3f(sec)\n",threads_args.count, count,
	(double)(costofwrites / threads_args.count),(double)(threads_args.count / costofwrites),costofwrites);
    
    }else if (strcmp(action,"read")==0){ //an h leitourgia einai read
        printf("SELECTED ACTION:__READ__\n");
        printf(LINE);
        printf("|Random-READ    (done:%ld, found:%ld): %.6f sec/op; %.1f reads per sec(estimated); cost:%.3f(sec)\n",threads_args.count, count,(double)(costofreads / threads_args.count),(double)(threads_args.count / costofreads),costofreads);
    }
    
}

int main(int argc,char** argv)
{
	//arxikopoiw tiw leitourgies
	long int count,countofwrites,countofreads;
	
	//arxikopoiw ta threads	
	long int threads,threadsofwrites,threadsofreads;
	
	//dhlvsh gia thn epilogh	
	int epilogh=0;
        
	//arxikopoiw ta struct pou xreiazomai
	struct data threads_args,threads_args_writes,threads_args_reads;
	
	srand(time(NULL));
	
	//thelw na dinei o xrhsths nhmata ara arc<4
	if (argc < 4) {
		fprintf(stderr,"Usage: db-bench <write | read | readwrite> <count> <threads> <random>\n");
		exit(1);
	}
	
	int r = 0;
	
	//elegxos apo thn grammh entolwn
	if ((strcmp(argv[1],"write") != 0) && (strcmp(argv[1],"read") != 0) && (strcmp(argv[1],"readwrite") != 0)){
        fprintf(stderr,"Usage: db-bench <write | read |readwrite> <count> <threads> <random>\n");
            exit(1);

    	}
	
	//elegxos gia thn kathe mia periptwsh read,write h readwrite
	switch(strlen(argv[1])){
		
		case 5://periptwsh pou einai write h leitourgia

			count = atoi(argv[2]);
			_print_header(count);
			_print_environment();


			if (argc == 5)
				r = 1;

			//pairnw to threads apo thn grammh entolwn			
			threads=atoi(argv[3]);

			//arxikopoiw ta nhmata gia to write me malloc
			pthread_t *tidofwrite = (pthread_t*)malloc(threads * sizeof(pthread_t));
			
			//anoigw vash
			db = db_open(DATAS);
			
			//dinw sto struct ta orismata
			threads_args.count=count;
			threads_args.r=r;
			threads_args.threads=threads;		

			//dhmiourgw ta nhmata
			for(int i=0;i<threads;i++){
				pthread_create(&tidofwrite[i],NULL,write_thread,&threads_args);
			}
			for(int i=0;i<threads;i++){
				pthread_join(tidofwrite[i],NULL);
			}

			//apodesmeuw gia thn malloc
			free(tidofwrite);

			//kleinw vash
			db_close(db);


			//ektypwnw ta statistika apodoshs ths leitourgias write
			printer("write",count,threads_args);
			


			//_write_test(count, r);
			


			break;


		case 4:
			
			count = atoi(argv[2]);	  	


			_print_header(count);
			_print_environment();


			if (argc == 5)
				r = 1;
			
			//pairnw to threads apo thn grammh entolwn
			threads=atoi(argv[3]);
			//desmeyw xwro gia ta thread ths read 
			pthread_t *tidofread = (pthread_t*)malloc(threads * sizeof(pthread_t));
			
			//anoigw vash
			db = db_open(DATAS);

			//dinw sto struct ta orismata
			threads_args.count=count;
			threads_args.r=r;
			threads_args.threads=threads;

			//dhmioyrgw ta nhmata gia thn read			
			for(int i=0;i<threads;i++){
				pthread_create(&tidofread[i],NULL,read_thread,&threads_args);
			}
			for(int i=0;i<threads;i++){
				pthread_join(tidofread[i],NULL);
			}
				
			//apodesmeyw ton xwro apo thn malloc
			free(tidofread);

			//kleinw vash
			db_close(db);

			//ektypwnw ta statika xronou ths leitoyrgias read
			printer("read",count,threads_args);


			//_read_test(count, r);
			break;
		


		case 9:// periptwsh readwrite

			count = atoi(argv[2]);
			_print_header(count);
			_print_environment();


			if (argc == 5)
				r = 1;

			//pairnw to threads apo thn grammh entolwn
			threads = atoi(argv[3]);

			//dinw ston xrhsth thn epilogh na epileksei stiw treiw epilogew pou exoume ulopoihsh gia to meigma leitourgiwn
			printf("If you want the 50 percect of put and 50 percent of get choose 1.If you want the 90 percent put and 10 percent of get choose 2.If you want the 40 percent put and the 60 percent of get choose 3.  \n");
                	scanf("%d",&epilogh);


			//an epileksei epileksei 50-50
			if(epilogh==1){	
				
				//metrhths gia ta write 50
				countofwrites=count*50/100;
				//methrths gia ta read 50
				countofreads=count*50/100;
				
				//threads gia ta write
				threadsofwrites=(long int)threads*50/100;
				//threads gia ta read
				threadsofreads=(long int)threads*50/100;
				
				//deysmeyw xwro gia ta threads tou write kai tou read
				pthread_t *tidofwrites = (pthread_t*)malloc(threads * sizeof(pthread_t));
				pthread_t *tidofreads = (pthread_t*)malloc(threads * sizeof(pthread_t));
				
				//anoigw vash
				db = db_open(DATAS);
				//twra einai me 50 write 50 read meta tha to allaksw
				
				//struct gia ta write	
				threads_args_writes.count=countofwrites;
				threads_args_writes.r=r;
				threads_args_writes.threads=threadsofwrites;
				
				//struct gia ta read
				threads_args_reads.count=countofreads;
				threads_args_reads.r=r;
				threads_args_reads.threads=threadsofreads;
				
				//dhmiourgw nhmata gia ta write kai gia read
				for(int i=0;i<threadsofwrites;i++){
					pthread_create(&tidofwrites[i],NULL,write_thread,&threads_args_writes);
				}
				for(int i=0;i<threadsofreads;i++){
					pthread_create(&tidofreads[i],NULL,read_thread,&threads_args_reads);
				}
				for(int i=0;i<threadsofwrites;i++){
					pthread_join(tidofwrites[i],NULL);
				}
				for(int i=0;i<threadsofreads;i++){
					pthread_join(tidofreads[i],NULL);
				}
				
				//apodeysmeyw malloc
				free(tidofwrites);
				free(tidofreads);
	
				//kleinw vash
				db_close(db);

				//ektypwnw statistika apodoshs antistoixa
				printer("read",countofreads,threads_args_reads);
				printer("write",countofwrites,threads_args_writes);


		
				break;

			}else if(epilogh==2){


				//poses diergsies sumfwna me pososto
				//metrhths gia ta write 90
				countofwrites=count*90/100;
				//metrhths gia ta read 10
				countofreads=count*10/100;

				//threads gia ta write
				threadsofwrites=(long int)threads*90/100;
				//threads gia ta read
				threadsofreads=(long int)threads*10/100;

				printf("Threads of reads is %ld",threadsofreads);
				printf("Threads of writes is %ld \n",threadsofwrites);

				//deysmeyw xwro gia ta threads tou write kai tou read
				pthread_t *tidofwrites = (pthread_t*)malloc(threadsofwrites * sizeof(pthread_t));
				pthread_t *tidofreads = (pthread_t*)malloc(threadsofreads * sizeof(pthread_t));
				
				//anoigw vash
				db = db_open(DATAS);
				
				//struct gia ta write
				threads_args_writes.count=countofwrites;
				threads_args_writes.r=r;
				threads_args_writes.threads=threadsofwrites;

				//struct gia ta read
				threads_args_reads.count=countofreads;
				threads_args_reads.r=r;
				threads_args_reads.threads=threadsofreads;
		
				//dhmiourgw nhmata gia ta write kai gia read
				for(int i=0;i<threadsofwrites;i++){
					pthread_create(&tidofwrites[i],NULL,write_thread,&threads_args_writes);
				}
				for(int i=0;i<threadsofreads;i++){
					pthread_create(&tidofreads[i],NULL,read_thread,&threads_args_reads);
				}
				for(int i=0;i<threadsofwrites;i++){
					pthread_join(tidofwrites[i],NULL);
				}
				for(int i=0;i<threadsofreads;i++){
					pthread_join(tidofreads[i],NULL);
				}

				//apodeysmeyw malloc
				free(tidofwrites);
				free(tidofreads);


				db_close(db);
				


				//ektypwnw statistika apodoshs antistoixa
				printer("read",countofreads,threads_args_reads);
				printer("write",countofwrites,threads_args_writes);

				break;
				
			}else if(epilogh==3){

				//metrhths gia ta write 40
				countofwrites=count*40/100;
				//metrhths gia ta read 60
				countofreads=count*60/100;

				//threads gia ta write				
				threadsofwrites=(long int)threads*40/100;
				//threads gia ta read
				threadsofreads=(long int)threads*60/100;

				printf("Threads of reads is %ld",threadsofreads);
				printf("Threads of writes is %ld \n",threadsofwrites);
			
				//deysmeyw xwro gia ta threads tou write kai tou read
				pthread_t *tidofwrites = (pthread_t*)malloc(threadsofwrites * sizeof(pthread_t));
				pthread_t *tidofreads = (pthread_t*)malloc(threadsofreads * sizeof(pthread_t));

				//anoigw vash
				db = db_open(DATAS);
				
				
				//struct gia ta write
				threads_args_writes.count=countofwrites;
				threads_args_writes.r=r;
				threads_args_writes.threads=threadsofwrites;

				//struct gia ta read
				threads_args_reads.count=countofreads;
				threads_args_reads.r=r;
				threads_args_reads.threads=threadsofreads;

				//dhmiourgw nhmata gia ta write kai gia read		
				for(int i=0;i<threadsofwrites;i++){
					pthread_create(&tidofwrites[i],NULL,write_thread,&threads_args_writes);
				}
				for(int i=0;i<threadsofreads;i++){
					pthread_create(&tidofreads[i],NULL,read_thread,&threads_args_reads);
				}
				for(int i=0;i<threadsofwrites;i++){
					pthread_join(tidofwrites[i],NULL);
				}
				for(int i=0;i<threadsofreads;i++){
					pthread_join(tidofreads[i],NULL);
				}
				
				//apodesmeyw malloc
				free(tidofwrites);
				free(tidofreads);


				db_close(db);
				
				//ektypwnw statistika apodoshs antistoixa
				printer("read",countofreads,threads_args_reads);
				printer("write",countofwrites,threads_args_writes);
				
				break;
				
			}
		default:
        		fprintf(stderr,"Usage: db-bench <write | read |readwrite> <count> <threads> <random>\n");
        		exit(1);
		}

	return 1;
}
