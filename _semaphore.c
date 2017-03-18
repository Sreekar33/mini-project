#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <minix/callnr.h>



PUBLIC int seminit (int sem) {
   message m;
   m.m1_i1 = sem;
   //m.m1_i2 = value;
   return (_syscall (PM_PROC_NR, SEM_INIT, &m));
}


PUBLIC int semup (int sem) {
   message m;
   m.m1_i1 = sem;
   return (_syscall (PM_PROC_NR, SEM_SIGNAL, &m));
}
PUBLIC int semdown (int sem) {
   message m;
   m.m1_i1 = sem;
   return (_syscall (PM_PROC_NR, SEM_WAIT, &m));
}


/*PUBLIC int semfree (int sem) {
   message m;
   m.m1_i1 = sem;
   return (_syscall (PM_PROC_NR, SEMFREE, &m));
}

    Contact GitHub API Training Shop Blog About 

*/
