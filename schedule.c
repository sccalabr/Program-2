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
int numProcesses = 0;
double quantum = 0;
struct itimerval timer;

typedef struct Node {
   unsigned int pid;
   struct Node *next;
   struct Node *prev;
} Node;

Node *startNode = NULL;
Node *lastNode = NULL;
Node *currentNode = NULL;

Node *findNextNode() {
   
   if(numProcesses == 0) {
      return NULL;
   }
   
   return currentNode = currentNode->next ? currentNode->next : startNode;
}

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
      printf("IN SIGALARM\n");
   
   setitimer(0, &timer, 0);
   kill(currentNode->pid, SIGSTOP);
   Node *next = findNextNode();
   kill(next->pid, SIGCONT);
   setitimer(ITIMER_REAL, &timer, 0);
}

void SIGCHLD_handler(int arg) {
   if(LOGGER)
      printf("IN SIGCHLD\n");
}

void *cleanArgs(char *args[]) {
   int i = 0;

   for(i = 0; i < MAX_ARGUMENTS; i++) {
      args[i] = (char *)0;
   }
}

int forkAndClean() {
   int status;
   signal(SIGALRM, SIGALRM_handler);
   if((pid = fork())) {
      if(startNode == NULL) {
         startNode = calloc(sizeof(Node), 1);
         startNode->pid = pid;
         startNode->next = NULL;
         startNode->prev = NULL;
         lastNode = startNode;
         currentNode = startNode;
      }
      else {
         Node *temp = (Node *)calloc(sizeof(Node), 1);
         temp->pid = pid;
         temp->next = startNode;
         temp->prev = lastNode;
         
         lastNode->next = temp;
         lastNode = temp;
      }
      filenameFlag = 1;
      argCounter = 0;
      cleanArgs(args);
   }
   else {
      kill(getpid(), SIGSTOP);
      printf("%d\n",execvp(filename, args));
      printf("SHOULD NOT EVER BE HERE\n");
   }
   counter++;
   numProcesses++;
   waitpid(pid, &status, WUNTRACED);
}

void runProgram() {
   int status;
   timer.it_value.tv_sec = quantum / 1000;
   timer.it_value.tv_usec = quantum;
   timer.it_interval = timer.it_value;
   
   signal(SIGALRM, SIGALRM_handler);
   
   while(numProcesses > 0) {
      setitimer(ITIMER_REAL, &timer, 0);
      kill(currentNode->pid, SIGCONT);
      
      if(waitpid(currentNode->pid, &status, 0)) {
         setitimer(0, &timer, 0);
         currentNode->prev->next = currentNode->next;
         currentNode->next->prev = currentNode->prev;
         Node *nodeToFree = currentNode;
         currentNode = currentNode->next;
         free(nodeToFree);
         numProcesses--;
      }
   }
}

int main(int argc, char *argv[]) {

   quantum = atol(argv[1]);

   cleanArgs(args);
   
   while(counter < argc) {
      if(!strcmp(argv[counter], ":")) {
         forkAndClean();
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
   
   forkAndClean();
   startNode->prev = lastNode;
   runProgram();
}


