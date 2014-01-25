#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include "schedule.h"


int filenameFlag = 1;
int argCounter = 0;
int counter = 2;
char *filename = 0;
char *args[MAX_ARGUMENTS + 1];
int pid = 0;
int pidList[MAX_PROCESSES];
int pidListCounter = 0;
int numProcesses = 0;
int currentPid = 0;
int quantum = 0;


void SIGCONT_handler(int arg) {
   signal(arg, SIGCONT_handler);
}

void SIGSTOP_handler(int arg) {
   signal(arg, SIGSTOP_handler);
}

void SIGALRM_handler(int arg) {
   /* The quantum ran out, pause the process that was running.
    * Then select the next process and send a singal to revive 
    * it.
    */
    kill(pidList[counter], SIGSTOP);
    counter = (counter + 1) % numProcesses;
    currentPid = pidList[counter];
}

void SIGCHLD_handler(int arg) {
  /* If this is received we are going to want to 
   * remove the process since it either was killed
   * or finished executing.
   */
   int i;
   
   for(i = counter; i < numProcesses; i++) {
      pidList[i] = pidList[i + 1];
   }     

    numProcesses--;
   
    counter %=  numProcesses;
    currentPid = pidList[counter];
    

}

void *cleanArgs(char *args[]) {
   int i = 0;

   for(i = 0; i < MAX_ARGUMENTS; i++) {
      args[i] = (char *)0;
   }
}

int forkAndClean() {
   if((pid = fork())) {
      pidList[numProcesses++];
      filenameFlag = 1;
      argCounter = 0;
      counter++;
      cleanArgs(args);
   }
   else {
      signal(SIGCONT, SIGCONT_handler);
      signal(SIGSTOP, SIGSTOP_handler);
      pause();
      printf("%d\n",execvp(filename, args));
      printf("SHOULD NOT EVER BE HERE\n");
   }
   
   return pid;
}

void runProgram() {
}

int main(int argc, char *argv[]) {

   quantum = atoi(argv[1]);

   cleanArgs(args);
   
   while(counter < argc) {
      // if we hit a colon we are going to need to forx and exec
      if(!strcmp(argv[counter], ":")) {
         pid = forkAndClean();
         pidList[pidListCounter++] = pid;
      }
   
      // save the file name for exec
      if(filenameFlag) {
         filename = argv[counter];
         args[argCounter++] = filename;
         filenameFlag = 0;
      }
      //save the arguments for exec
      else {
         args[argCounter++] = argv[counter];
      }
 
      counter++;
   }
   
   // need to get the last one



   pid = forkAndClean();
   pidList[pidListCounter++] = pid;
   
   counter = 0;
   struct itimerval timer;
   
   timer.it_interval.tv_sec = 0;
   timer.it_interval.tv_usec = 0;
   timer.it_value.tv_sec = quantum;
   timer.it_value.tv_usec = 0; 
  
   
   while(numProcesses > 0) {
      int status;
      signal(SIGALRM, SIGALRM_handler);
      signal(SIGCHLD, SIGCHLD_handler);
      signal(SIGCONT, SIGCONT_handler);
      signal(SIGSTOP, SIGSTOP_handler);
      
      setitimer(ITIMER_REAL, &timer,0);
      currentPid = pidList[counter % numProcesses];
      kill(currentPid, SIGCONT);
      sleep(quantum);
   }
}


