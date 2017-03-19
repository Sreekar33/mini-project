#include <lib.h>      // provides _syscall and message
#include <unistd.h>   // provides function prototype (see step 1 below)

int sem_init(int val) {
	message m;      // Minix message to pass parameters to a system call    
	m.m1_i1 = val;  // set first integer of message to val
    
	return _syscall(PM_PROC_NR, SEMINIT, &m);  // invoke underlying system call
}

int sem_wait(int val){
	message m;      // Minix message to pass parameters to a system call    
	m.m1_i1 = val;  // set first integer of message to val
    
	return _syscall(PM_PROC_NR, SEMWAIT, &m);  // invoke underlying system call
	
}
int sem_signal(int val){
	message m;      // Minix message to pass parameters to a system call    
	m.m1_i1 = val;  // set first integer of message to val
    
	return _syscall(PM_PROC_NR, SEMSIGNAL, &m);  // invoke underlying system call
	
}

int sem_free(int val){
	message m;      // Minix message to pass parameters to a system call    
	m.m1_i1 = val;  // set first integer of message to val
    
	return _syscall(PM_PROC_NR, SEMFREE, &m);  // invoke underlying system call
	
}
