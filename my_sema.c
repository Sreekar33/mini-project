#include<stdio.h>
#include "pm.h"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sema_count=0;
/*
struct process {
	int pid;
	struct process *next;
}
*/

struct semaphore{
	//int sem_check
	int value;
	int process [1000];
	int num_of_process;
};

struct semaphore sem[10];

int do_init(void){
	int sem_id=sema_count;
	sema_count++;
	int val=m_in.m1_i1;
	//sem[sem_id].sem_check=1;
	sem[sem_id].value=val;
	sem[sem_id].num_of_process=0;
return sem_id;
}

int do_wait(void){
	int val=m_in.m1_i1;
	int sem_id=val;
	int pid;
	time_t t,t1;
	sem[sem_id].value--;
	time(&t);
	printf("current value of S value : %d at time : %s\n",sem[sem_id].value,ctime(&t));
	if(sem[sem_id].value < 0){
		pid=getpid();
		sem[sem_id].process[sem[sem_id].num_of_process]=pid;
		sem[sem_id].num_of_process++;
		time(&t1);
		printf("Process to be blocked: %d at time : %s\n",pid,ctime(&t));
		return(SUSPEND);
	}
return 0;
}

int do_signal(void){
	int val=m_in.m1_i1;
	int sem_id=val;
	int pid;
	int i;
	time_t t,t1;
	sem[sem_id].value++;
	time(&t);
	printf("current value of S value : %d at time : %s\n",sem[sem_id].value,ctime(&t));
	if(sem[sem_id].value <= 0){
		if(sem[sem_id].num_of_process >0){
			pid=sem[sem_id].process[0];
			sem[sem_id].num_of_process--;
			for(i=0;i<sem[sem_id].num_of_process;i++){
				sem[sem_id].process[i]=sem[sem_id].process[i+1];
			}
			time(&t1);
			printf("Process to be woken up : %d by process : %d at time : %s",pid,getpid(),ctime(&t1));
			setreply (pid,0); 
		}
	}
return 0;
}

int do_free(void){
	int val=m_in.m1_i1;
	int sem_id=val;
	int i;
	for(i=sem_id;i<sema_count;i++){
		sem[i]=sem[i+1];
	}
	sema_count--;
}
