
#include "coordinator.h"


/* Semaphore down operation, using semop */
int sem_wait(int sem_id,int index) {   //down semaphore
   struct sembuf sem_d;
   
   sem_d.sem_num = index;
   sem_d.sem_op = -1;
   sem_d.sem_flg = 0;
   if (semop(sem_id,&sem_d,1) == -1) {
       perror("# Semaphore down operation ");
       return -1;
   }
   return 0;
}


/* Semaphore up operation, using semop */ 
int sem_signal(int sem_id,int index) {  //up semaphore
   struct sembuf sem_d;
   
   sem_d.sem_num = index;
   sem_d.sem_op = 1;
   sem_d.sem_flg = 0;
   if (semop(sem_id,&sem_d,1) == -1) {
       perror("# Semaphore up operation ");
       return -1;
   }
   return 0;
}


/* Semaphore Init - set a semaphore's value to val */
int sem_init(int sem_id, int val,int index ) {  

   union semun arg;
   
   arg.val = val;
   if (semctl(sem_id,index,SETVAL,arg) == -1) {
       perror("# Semaphore setting value ");
       return -1;
   }
   return 0;
}



void free_resources(){  

	if (shmctl(shm_entries,IPC_RMID,(struct shmid_ds *)NULL) < 0){
		perror("Failed deallocating memory for data \n");
		exit(1);
	}

	if (semctl(sem_entry,args.entries,IPC_RMID,0) < 0){
		perror("Failed deallocating memory for mutex \n");
		exit(1);
	}
	if (semctl(sem_db,args.entries,IPC_RMID,0) < 0){
		perror("Failed deallocating memory for database \n");
		exit(1);
	}
}



void create_resources(){  

	shm_entries = shmget (SHMEM_KEY, sizeof(Entry)*args.entries, PERMISSIONS | IPC_CREAT);
	if ( shm_entries < 0 ) {
		perror("Shared memory allocation failed\n");
    	exit(1);
    }

	//kanoume attach tis mnimes 
	entries_ptr = (Entry*)shmat(shm_entries, (char *) 0, 0);
	if ( entries_ptr == NULL ) {
    	perror("Shared memory allocation failed\n");
    	exit(1);
        
    }

    //Create a new semaphore
	sem_entry = semget(SEMENTRY_KEY,args.entries,IPC_CREAT | PERMISSIONS);
   	if (sem_entry == -1) {
       	perror("Failed create semaphore\n");
       	exit(1);
   	}

   	sem_db = semget(SEMDB_KEY,args.entries,IPC_CREAT | PERMISSIONS);
   	if (sem_db == -1) {
       	perror("Failed create semaphore\n");
       	exit(1);
   	}
}



void semaphores_init(){  

	int i;

	for (i = 0; i < args.entries; ++i){
	
		if (sem_init(sem_entry, 1, i) == -1) { 
	       		perror("Failed initializing semaphore\n");
	       		exit(1);
	   	}

	   	if (sem_init(sem_db, 1, i) == -1) { 
	       		perror("Failed initializing semaphore\n");
	       		exit(1);
	   	}
   }

}



void arguments(int x ,char ** y){ 

	int opt_value = 0;


	if (x != 5){
		cout<<"Wrong input number"<<endl;
		exit(1);
		}
	while ((opt_value = getopt (x, y, "e:a:")) != -1){      
	 	switch(opt_value)
	 	{
	  	case 'e':                      
	      	args.entries = atoi(optarg);  
	      	break;
	  	case 'a':                      
	      	args.analogy = atoi(optarg);  
	  		break;
	  	default:
	  		exit(1);
		}
	} 


}



void peer(){  

	srand(getpid());
	int i, analog = RAND_MAX/(args.analogy+1);
	int counter_reader = 0, counter_writer = 0;
	
	float time_r_sum = 0., time_w_sum = 0., average_time;
	for(i = 0; i<LOOPS; i++){  
		
		if(analog<rand()){  //if is reader
			time_r_sum += reader();  
			counter_reader++;   

		}
		else{      //if is writer
			time_w_sum += writer();   
			counter_writer++;  
		}
	}
	
	if(counter_writer!=0){  
		printf("Readers count = %d\nWriters count = %d\nAverage waiting time for readers = %f\nAverage waiting time for writers = %f\n\n"
			,counter_reader, counter_writer, time_r_sum/counter_reader/CLOCKS_PER_SEC,time_w_sum/counter_writer/CLOCKS_PER_SEC);
	}
	else{  
		printf("Readers count = %d\nWriters count = %d\nAverage waiting time for readers = %f\nAverage waiting time for writers = 0\n\n"
			,counter_reader, counter_writer, time_r_sum/counter_reader/CLOCKS_PER_SEC);
	}
	exit(0);
}




long time_distribution(){  


	float u = ((float)rand())/RAND_MAX; //u
	return -(log(u)/PEERS_NUM*LOOPS)*L;    //T = -ln(u)/l

}



float reader(){

	clock_t timer1 ,timer2;      
	int pos = rand()%args.entries;   
	sem_wait(sem_entry, pos);  //down mutex
	entries_ptr[pos].reader_count++;  
	timer1 = clock();
	if(entries_ptr[pos].rc == 1) sem_wait(sem_db, pos); 
	timer2 = clock();
	sem_signal(sem_entry, pos);  //up mutex
	usleep(time_distribution());   
	sem_wait(sem_entry, pos);    
	entries_ptr[pos].rc--;
	if(entries_ptr[pos].rc == 0) sem_signal(sem_db, pos);  
	sem_signal(sem_entry, pos);
	return timer2-timer1;

}


float writer(){

	clock_t timer1 ,timer2;
	int pos = rand()%args.entries;
	long t = time_distribution();
	timer1 = clock();
	sem_wait(sem_db, pos);  //down
	timer2 = clock();
	usleep(t);
	entries_ptr[pos].writer_count++; 
	sem_signal(sem_db, pos);   //up 
	return timer2-timer1;

}




int main(int argc, char ** argv){
	cout<<"Shared Memory Allocated, process statistics are: "<<endl;
	int i, status;
	arguments(argc,argv);   
	int readers = 0, writers = 0;

	create_resources(); // allocating memory
	semaphores_init();  //initializing semaphores
	
	pid_t childpid;   

	
	for(i = 0; i<args.entries; i++){   
		
		entries_ptr[i].reader_count = 0;
		entries_ptr[i].writer_count = 0;

	}


	for(i = 0; i<PEERS_NUM; i++){   

		if((childpid = fork()) < 0){   
			perror("Failed fork\n");
			exit(1);
		}

		if( childpid == 0){   

			peer();  

		}
	}

	while((childpid = wait(&status)) > 0);   

	for (int i = 0; i < args.entries; ++i)   
	{

		readers += entries_ptr[i].reader_count;        
		writers += entries_ptr[i].writer_count;

	}
	//tipwse ta statistika
	cout<<"Total readings = "<<readers<<endl<<"Total writings = "<<writers<<endl;  
	cout<<"Program now terminates,all memory is being deallocated...\n"<<endl;
	free_resources(); 
}
