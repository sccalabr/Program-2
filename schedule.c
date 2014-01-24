#include <stdio.h>
#include <stdlib.h>
#include "schedule.h"

void *cleanArgs(char *args[]) {
   int i = 0;

   for(i = 0; i < MAX_ARGUMENTS; i++) {
      args[i] = 0;
   }
}

int main(int argc, char *argv[]) {

   int quantum = atoi(argv[1]);
   
   int pid = 0;
   int counter = 2;
   int filenameFlag = 1;
   int argCounter = 0;
   
   char *filename = 0;
   char *args[MAX_ARGUMENTS + 1];

   cleanArgs(args);
   
   while(counter < argc) {
      // if we hit a colon we are going to need to forx and exec
      if(!strcmp(argv[counter], ":")) {
         if((pid = fork())) {
            filenameFlag = 1;
            argCounter = 0;
            counter++;
            cleanArgs(args);
         }
         else {
            execvp(filename, args);
         }
      }
   
      // save the file name for exec
      if(filenameFlag) {
         filename = argv[counter];
         filenameFlag = 0;
      }
      //save the arguments for exec
      else {
         args[argCounter++] = argv[counter];
      }
 
      counter++;
   }
   
   // need to get the last one
      if((pid = fork())) {
         filenameFlag = 1;
         argCounter = 0;
         counter++;
         cleanArgs(args);
      }
      else {
         execvp(filename, args);
      }







}




