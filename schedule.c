#include <stdio.h>
#include <stdlib.h>
#include "schedule.h"


int filenameFlag = 1;
int argCounter = 0;
int counter = 2;
char *filename = 0;
char *args[MAX_ARGUMENTS + 1];
int pid = 0;
int pidList[MAX_PROCESSES];
int numProcesses = 0;

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
      pause();
      printf("%d\n",execvp(filename, args));
      printf("SHOULD NOT EVER BE HERE\n");
   }
   
   return pid;
}

void runProgram() {



}

int main(int argc, char *argv[]) {

   int quantum = atoi(argv[1]);

   cleanArgs(args);
   
   while(counter < argc) {
      // if we hit a colon we are going to need to forx and exec
      if(!strcmp(argv[counter], ":")) {
         pid = forkAndClean();
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
   
   runProgram();
}





