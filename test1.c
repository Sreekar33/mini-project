#include <lib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void){

	message m;
	m.m1_i1=1;

	int val=_syscall(PM_PROC_NR,SEMA_INIT,&m);
	m.m1_i2=0;
	
	_syscall(PM_PROC_NR,SEMA_WAIT,&m);
	
//	_syscall(PM_PROC_NR,SEMA_SIGNAL,&m1);

	}
