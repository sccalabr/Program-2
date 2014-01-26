#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include "schedule.h"

#define LOGGER 0

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
double quantum = 0;
int alarmFlag = 0;


void SIGCONT_handler(int arg) {
   if(LOGGER)
      printf("IN SIGCONT\n");
   signal(arg, SIGCONT_handler);
}

void SIGSTOP_handler(int arg) {
   if(LOGGER)
      printf("IN SIGSTOP\n");
   signal(arg, SIGSTOP_handler);
}

void SIGALRM_handler(int arg) {
   /* The quantum ran out, pause the process that was running.
    * Then select the next process and send a singal to revive 
    * it.
    */
    if(LOGGER)
      printf("SIGALARM\n");
    alarmFlag = 1;
    kill(currentPid, SIGSTOP);
    counter = (counter + 1) % numProcesses;
    currentPid = pidList[counter];
}

void SIGCHLD_handler(int arg) {
  /* If this is received we are going to want to 
   * remove the process since it either was killed
   * or finished executing.
   */
   if(LOGGER)
      printf("====>IN SIGCHLD<====\ncounter: %d   numProcesses: %d\n", counter, numProcesses);
   int i;
   int status;
   

   if(LOGGER) {  
      for(i = counter; i < numProcesses; i++) {
         printf("%d\n", pidList[i]);
      }
   }
   for(i = counter; i < numProcesses; i++) {
      pidList[i] = pidList[i + 1];
   }     
   
   if(LOGGER) {
      for(i = counter; i < numProcesses; i++) {
         printf("%d\n", pidList[i]);
      }
   }
   
   numProcesses--;
   
   if(numProcesses > 0) {
      counter %=  numProcesses;
      currentPid = pidList[counter];
   }
   else {
      exit(0);
   }
    //printf("DONE WITH SIGCHILD.... WHY DIDN'T PRINT\n");
}

void *cleanArgs(char *args[]) {
   int i = 0;

   for(i = 0; i < MAX_ARGUMENTS; i++) {
      args[i] = (char *)0;
   }
}

int forkAndClean() {
   if((pid = fork())) {
      pidList[numProcesses++] = pid;
      filenameFlag = 1;
      argCounter = 0;
      counter++;
      cleanArgs(args);
      sleep(2);
   }
   else {
      signal(SIGCONT, SIGCONT_handler);
      signal(SIGSTOP, SIGSTOP_handler);
      if(LOGGER)
         printf("CHILD: %d\n", getpid());
      kill(getpid(), SIGSTOP);
      //pause();
      printf("%d\n",execvp(filename, args));
      printf("SHOULD NOT EVER BE HERE\n");
   }
   
   return pid;
}

void runProgram() {
   counter = 0;
/*   struct itimerval timer;
   
   timer.it_interval.tv_sec = 0;
   timer.it_interval.tv_usec = 0;
   timer.it_value.tv_sec = quantum;
   timer.it_value.tv_usec = 0;
  
   signal(SIGALRM, SIGALRM_handler);
*/   signal(SIGCHLD, SIGCHLD_handler);
   signal(SIGCONT, SIGCONT_handler);
   signal(SIGSTOP, SIGSTOP_handler);
   
   while(numProcesses > 0) {
     //printf("top of while loop\n");
 /*    
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;
      timer.it_value.tv_sec = quantum;
      timer.it_value.tv_usec = 0;
  
      setitimer(ITIMER_REAL, &timer,0);
         if(LOGGER)
 */   if(LOGGER)  
         printf("currentPID: %d\n", currentPid);
      currentPid = pidList[counter % numProcesses];
      
      if(LOGGER)
         printf("currentPID: %d\n", currentPid);
      
      int status;
      while(!alarmFlag && waitpid(currentPid, &status, WNOHANG ) == 0) {
         kill(currentPid, SIGCONT);
         if(LOGGER)
            printf("waiting on child...\n");
         pause();
      }
      alarmFlag = 0;
   }
   
   if(LOGGER)
      printf("DONE WITH PROGRAM\n\n\n\n\n");
}

int main(int argc, char *argv[]) {

   quantum = atol(argv[1]) / 1000;

   cleanArgs(args);
   
   while(counter < argc) {
      if(!strcmp(argv[counter], ":")) {
         pid = forkAndClean();
         pidList[pidListCounter++] = pid;
      }
   
      if(filenameFlag) {
         filename = argv[counter];
         args[argCounter++] = filename;
         filenameFlag = 0;
      }
      else {
         args[argCounter++] = argv[counter];
      }
      counter++;
   }
   pid = forkAndClean();
   pidList[pidListCounter++] = pid;
   
   runProgram();
}


