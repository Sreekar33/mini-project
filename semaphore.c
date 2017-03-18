#include "pm.h"
#include "param.h"
#include "glo.h"
#include "mproc.h"
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NR_SEMS      100          // number of semaphores available in kernel
#define SZ_PLIST     1000         // size of each semaphore's process list
#define NO_SEM      -1            // a semaphore id set to -1 is unallocated
#define INIT_SEMVAL  1000         // a bound for the maximum initial semaphore value
#define MAX_SEMVAL   1000000      // a bound for the maximum semaphore value
#define INT_LIMIT    0x7fffffff   // the maximum value of an int primitive value
#define ESEMVAL      0x8000000    // the return value for an error in do_semvalue
#define NO_PROC_NR   0            // a proc number cannot be 0
#define RETERR       0            // functions return 0 on error, except do_semvalue


FORWARD _PROTOTYPE( int sem_exists, (int sem)    			);
FORWARD _PROTOTYPE( void init_sems, (void)      			);


struct semaphore {
   int id, value; // semaphore id and value
   int plist[SZ_PLIST]; // "queue" of sleeping processes
   int begin; // index to the next to wake process, highest priority
   int end; // index to the most recently put to sleep process, low priority
   int pcount; // number of sleeping processes
} semarray[NR_SEMS];
int semcount = 0; // number of allocated semaphores in semarray

PRIVATE void init_sems (void) {
   // Initialize all the semaphores to NO_SEM to know they are free to use.
   int i;
   for (i = 0; i < NR_SEMS; ++i) {
      // "mark" the semaphores as free
      semarray[i].id = NO_SEM;
      semarray[i].value = 0;
      // initialize the process list
      semarray[i].pcount = 0; // no processes in process list
      semarray[i].plist[0] = NO_PROC_NR; // an invalid proc number 
      semarray[i].begin = 0;
      semarray[i].end = 0;
   }
   semcount = 0;
}


PRIVATE int sem_exists (sem)
   int sem;
{
   if (sem <= 0 || sem > INT_LIMIT - 1) return 0;

   if (semcount < 1) return 0;

   int i;
   for (i = 0; i < NR_SEMS; ++i) {
      if (semarray[i].id == sem) return 1; // found the id
   }
   // did not find the id
   return 0;
}


PUBLIC int do_semvalue (void) {
   int exitstatus = ESEMVAL; // not yet successful, assume error
   int sem = m_in.m1_i1;

   if (sem <= 0 || sem > INT_LIMIT - 1 || semcount < 1) return ESEMVAL;
   
   int index;
   index = (sem % NR_SEMS) - 1;
   if (index < 0) index = NR_SEMS - 1;
   int i;
   for (i = 0; i < NR_SEMS && exitstatus == ESEMVAL; ++i) {
      if (semarray[index].id == sem) exitstatus = semarray[index].value;
      else if (index == (NR_SEMS - 1)) index = 0;
      else ++index;
   }

   if (exitstatus == ESEMVAL) return ESEMVAL;

   return exitstatus;
}



PUBLIC int do_seminit (void) {
   if (semcount == 0) init_sems ();

   // return value: should be set to the sem id or 0 if an error occurred
   int exitstatus = RETERR; // not yet successful
   // get message
   int sem = m_in.m1_i1;
   int value = m_in.m1_i2;

   if (semcount >= NR_SEMS) {
      m_in.m_type = EAGAIN;
      return RETERR;
   }
   if (sem < 0 || sem > INT_LIMIT - 1 || value > INIT_SEMVAL || value < -INIT_SEMVAL) {
      m_in.m_type = EINVAL;
      return RETERR;
   }
   if (sem != 0 && sem_exists (sem)) {
      m_in.m_type = EEXIST;
      return RETERR;
   }
   int index;
   if (sem == 0) {
      int i;
      for (i = 0; i < NR_SEMS && exitstatus == RETERR; ++i) {
         if (semarray[i].id == NO_SEM) {
            index = i;
            sem = i + 1;
            // the sem id already exists, alter sem id chosen
            while (sem_exists (sem)) sem += 100;
            ++semcount; // increment the number of allocated semaphores
            exitstatus = sem; // will return the id to the user, but cannot be 0 or 101
            // initialize the semaphore
            semarray[i].id = sem; // set id
            semarray[i].value = value; // set value
            // initialize the process list to empty
            semarray[i].pcount = 0; // no processes in the process list
            semarray[i].plist[0] = NO_PROC_NR; // an invalid pid
            semarray[i].begin = 0;
            semarray[i].end = 0;
         }
      }
   }else {
      index = ((sem % NR_SEMS) - 1);
      if (index < 0) index = NR_SEMS - 1;
      // find an unused semaphore
      int j;
      for (j = 0; j < NR_SEMS && exitstatus == RETERR; ++j) {
         if (semarray[index].id == NO_SEM) {
            ++semcount; // increment the number of allocated semaphores
            exitstatus = sem; // will return success by sem id
            // initialize the semaphore
            semarray[index].id = sem; // set id
            semarray[index].value = value; // set value
            // initialize the process list
            semarray[index].pcount = 0; // no processes in the process list
            semarray[index].plist[0] = NO_PROC_NR; // an invalid pid
            semarray[index].begin = 0;
            semarray[index].end = 0;
         }else if (index == (NR_SEMS - 1)) index = 0;
         else ++index; // increment the index
      }
   }
   
   return exitstatus;
}


PUBLIC int do_semup (void) {
   // return 1 if successful, 0 otherwise
   int exitstatus = RETERR; // not yet successful
   // get message
   int sem = m_in.m1_i1;
   if (sem <= 0 || sem > INT_LIMIT - 1 || semcount < 1) {
      m_in.m_type = EINVAL;
      return RETERR;
   }
   
   // find index around sem modulo NR_SEMS
   int index;
   index = (sem % NR_SEMS) - 1;
   if (index < 0) index = NR_SEMS - 1;
   int i;
   for (i = 0; i < NR_SEMS && exitstatus == RETERR; ++i) {
      if (semarray[index].id == sem) exitstatus = 1;
      else if (index == (NR_SEMS - 1)) index = 0;
      else ++index;
   }

   // MORE ERROR CHECKING //
   // semaphore id could not be found
   if (exitstatus == RETERR) {
      //m_in.m_type = EEXIST;
      return RETERR;
   }
   //   check if value will exceed bounds of 10^6
   if (semarray[index].value == MAX_SEMVAL) {
      m_in.m_type = EOVERFLOW;
      return RETERR;
   }

   // increment value
   ++(semarray[index].value);
   // wake up a sleeping process if there is one
   if (semarray[index].pcount != 0) {
      int next = semarray[index].begin;
      int proc_nr = semarray[index].plist[next];
      // check if a valid process id number
      if (proc_nr <= NO_PROC_NR || proc_nr >= NR_PROCS) return RETERR;
      
      // increment to the next sleeping process, unless there are no more
      if (next == semarray[index].end) {
         semarray[index].plist[next] = NO_PROC_NR; // an invalid pid
         semarray[index].begin = 0; // make process list start at beginning of list
         semarray[index].end = 0; // make process list start at beginning of list
      }else if (next == (SZ_PLIST - 1)) {
         semarray[index].plist[SZ_PLIST - 1] = NO_PROC_NR;
         semarray[index].begin = 0; // loop back around to index 0
      }else {
         semarray[index].plist[next] = NO_PROC_NR;
         ++next; // increment next index
         semarray[index].begin = next;
      }
      // decrement process list count
      --(semarray[index].pcount); // one less sleeping process

      // now wake the process with the proc_nr, check for error
      register struct mproc *rmp = &mproc[proc_nr];
      setreply (proc_nr, exitstatus);
   }
   
   return exitstatus;
}


PUBLIC int do_semdown (void) {
   int exitstatus = RETERR; // not yet successful
   int sem = m_in.m1_i1;

   if (sem <= 0 || sem > INT_LIMIT - 1 || semcount < 1) {
      m_in.m_type = EINVAL;
      return RETERR;
   }
   
   int index;
   index = (sem % NR_SEMS) - 1;
   if (index < 0) index = NR_SEMS - 1;
   int i;
   for (i = 0; i < NR_SEMS && exitstatus == RETERR; ++i) {
      if (semarray[index].id == sem) exitstatus = 1;
      else if (index == (NR_SEMS - 1)) index = 0;
      else ++index;
   }

   if (exitstatus == RETERR) {
      //m_in.m_type = EEXIST;
      return RETERR;
   }
   // check if value will exceed bounds of -10^6
   if (semarray[index].value == -MAX_SEMVAL) {
      m_in.m_type = EOVERFLOW;
      return RETERR;
   }
   // check if process will fit on the process list
   if (semarray[index].pcount == SZ_PLIST) {
      m_in.m_type = EOVERFLOW;
      return RETERR;
   }
   
   // decrement sem value
   --(semarray[index].value);
   // go to sleep if value is less than 0 after decrementing
    if (semarray[index].value < 0) {
      int last = semarray[index].end;
      int proc_nr = who_p;

      // add the process to the process list
      if (semarray[index].pcount == 0) {
         semarray[index].plist[last] = proc_nr;
      }else if (last == (SZ_PLIST - 1)) {
         last = 0;
         semarray[index].end = last;
         semarray[index].plist[last] = proc_nr;
      }else {
         ++last;
         semarray[index].end = last;
         semarray[index].plist[last] = proc_nr;
      }
      ++(semarray[index].pcount); // one more sleeping process
      return (SUSPEND);
   }

   return exitstatus;
}


PUBLIC int do_semfree (void) {
   // return 1 if successful, 0 otherwise
   int exitstatus = RETERR; // not yet successful
   // get message
   int sem = m_in.m1_i1;
   if (sem <= 0 || sem > INT_LIMIT - 1) {
      m_in.m_type = EINVAL;
      return RETERR;
   }
   
   // find the index for the semaphore array if it exists
   int index;
   index = (sem % NR_SEMS) - 1;
   if (index < 0) index = NR_SEMS - 1;
   int i;
   for (i = 0; i < NR_SEMS && exitstatus == RETERR; ++i) {
      if (semarray[index].id == sem) exitstatus = 1; // could successfully free
      else if (index == (NR_SEMS - 1)) index = 0;
      else ++index; // increment the index
   }

   if (exitstatus == RETERR) {
      m_in.m_type = EEXIST;
      return RETERR;
   }
   if (semarray[index].pcount != 0) {
      m_in.m_type = EBUSY;
      return RETERR;
   }

   // go ahead and free the semaphore
   --semcount;
   // "mark" the semaphore as free
   semarray[index].id = NO_SEM;
   semarray[index].value = 0;
   // "empty" the process list
   semarray[index].pcount = 0; // there are no processes in the process list
   semarray[index].plist[0] = NO_PROC_NR; // 
   semarray[index].begin = 0;
   semarray[index].pcount = 0;
   
   return exitstatus;
}

