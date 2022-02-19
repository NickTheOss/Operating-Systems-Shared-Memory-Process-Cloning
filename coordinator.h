#ifndef COORDINATOR_H
#define COORDINATOR_H
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <math.h>


#define PEERS_NUM 10  //Number of processes
#define SHMEM_KEY (key_t) 12345678   
#define SEMENTRY_KEY (key_t) 87654321 
#define SEMDB_KEY (key_t) 384958372
#define SEMRC_KEY (key_t) 495869482
#define PERMISSIONS 0660
#define LOOPS 10
#define L 1000 //stathera

using namespace std;



typedef struct{

	int entries, analogy;  

}Args;



typedef struct{

	int reader_count, writer_count, rc;  

}Entry;



union semun {            
   int val;                  /* value for SETVAL */
   struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
   unsigned short *array;    /* array for GETALL, SETALL */

};


Args args;       
int shm_entries; //shared memory
Entry * entries_ptr;   
int sem_entry, sem_db; //semaphores





void arguments(int x ,char ** y);    
int sem_wait(int sem_id,int index);    //DOWN
int sem_signal(int sem_id,int index);   //UP
int sem_init(int sem_id, int val,int index);  //INIT
void create_resources();   
void semaphores_init();  
void peer();   
float reader();  // reader
float writer();  //writer
long time_distribution();


#endif