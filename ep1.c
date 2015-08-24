#define _XOPEN_SOURCE 500 /*To compile without nanosleep implicit declaration warning*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include "ep1.h"
#include "sjf.h"

int run(char **argv, char *wd)
{
  unsigned int all = 0, *total = &all;
  Process *process;
  process = malloc(100 * sizeof(*process));

  printf("Run argument 2 = %s\n", argv[1]);
  printf("Run argument 3 = %s\n", argv[2]);
  printf("Run argument 4 = %s\n", argv[3]);

  process = read_trace_file(process, wd, argv[1], total);
  if(argv[3] != NULL) paramd = True;
  else paramd = False;

  if(!initialize_mutex(process, total)) return 0;

  if(process != NULL) {
    unsigned int scheduler;
    pthread_t *threads;

    threads = malloc((*total + 1) * sizeof(*threads));

    switch(scheduler = atoi(argv[0])) {
      case FCFS:
        /*do FCFS*/
        printf("1. FCFS\n");
        break;
      case SJF:
        /*do SJF*/
        printf("2. SJF\n");
        do_sjf(threads, process, total);
        printf("\n\n* * * * * * * * * *\n\n");
        printf("SJF simulation has finished. Your output file can be found in 'outputs' folder.\n");
        break;
      case SRTN:
        /*do SRTN*/
        printf("3. SRTN\n");
        break;
      case RR:
        /*do RR*/
        printf("4. RR\n");
        break;
      case PS:
        /*do PS*/
        printf("5. PS\n");
        break;
      case RRTS:
        /*do RRTS*/
        printf("6. RRTS\n");
        break;
      free(threads); threads = NULL;
      free_mutex(process, total);
      free(process); process = NULL;
    }
  }
  else {
    printf("\nError reading file. Make sure it is written in expected ");
    printf("format and you are executing ep1 command from the right folder.\n");
  }
  return 0;
}

/*Free mutexes*/
void free_mutex(Process *process, unsigned int *total)
{
  unsigned int i;
  for(i = 0; i < *total; i++) {
    sem_destroy(&process[i].next_stage);
    pthread_mutex_destroy(&process[i].mutex);
  }
}

/*Initializes mutexes*/
int initialize_mutex(Process *process, unsigned int *total)
{
  unsigned int i;
  for(i = 0; i < *total; i++) {
    unsigned int j;
    if(sem_init(&process[i].next_stage, 0, 0)) {
      printf("Error initializing semaphore.\n");
      for(j = 0; j < i; j++) {
        sem_destroy(&process[j].next_stage);
        pthread_mutex_destroy(&process[j].mutex);
      }
      free(process); process = NULL;
      return 0;
    }
    if(pthread_mutex_init(&process[i].mutex, NULL) != 0) {
      printf("\nError initializing MutEx.\n");
      for(j = 0; j < i; j++) {
        sem_destroy(&process[j].next_stage);
        pthread_mutex_destroy(&process[j].mutex);
      }
      free(process); process = NULL;
      return 0;
    }
  }
  return 1;
}

/*Read the trace file 'tfile' and store its content in the array 'process',
  creating process "objects"*/
Process *read_trace_file(Process *process, char *wd, char *tfile, unsigned int *total)
{
  FILE *fptr;
  char input[256];

  strcat(strcat(strcpy(input, wd), "/inputs/"), tfile);
  if((fptr = fopen(input, "r")) != NULL) {
    char c, tmp[64];
    unsigned int i = 0, j = 0, size = 100, item = 1, dots = 0, done = 0;

    while(!done) {
      if((c = fgetc(fptr)) == EOF) { done = 1; continue; }
      switch (item) {
        case 1: /*Get Arrival*/
          if(isdigit(c) || c == '.') {
            tmp[i++] = c;
            if(c == '.') dots++;
            if(dots > 1) { free(process); fclose(fptr); return NULL; }
          }
          else if(is_blank(c) && i > 0) {
            tmp[i] = '\0'; i = 0; item = 2; dots = 0;
            process[j].arrival = atof(tmp);
            continue;
          }
          else { free(process); fclose(fptr); return NULL; }
          break;

        case 2: /*Get name*/
          if(!isspace(c)) tmp[i++] = c;
          else if(is_blank(c) && i > 0) {
            tmp[i] = '\0'; i = 0; item = 3;
            strcpy(process[j].name, tmp);
            continue;
          }
          else { free(process); fclose(fptr); return NULL; }
          break;

        case 3: /*Get duration*/
          if(isdigit(c) || c == '.') {
            tmp[i++] = c;
            if(c == '.') dots++;
            if(dots > 1) { free(process); fclose(fptr); return NULL; }
          }
          else if(is_blank(c) && i > 0) {
            tmp[i] = '\0'; i = 0; item = 4; dots = 0;
            process[j].duration = atof(tmp);
            continue;
          }
          else { free(process); fclose(fptr); return NULL; }
          break;

        case 4: /*Get deadline*/
          if(isdigit(c) || c == '.') {
            tmp[i++] = c;
            if(c == '.') dots++;
            if(dots > 1) { free(process); fclose(fptr); return NULL; }
          }
          else if(is_blank(c) && i > 0) {
            tmp[i] = '\0'; i = 0; item = 5; dots = 0;
            process[j].deadline = atof(tmp);
            continue;
          }
          else { free(process); fclose(fptr); return NULL; }
          break;

        case 5: /*Get Priority*/
          if(isdigit(c) || c == '-') {
            tmp[i++] = c;
            if(c == '-') dots++;
            if(dots > 1) { free(process); fclose(fptr); return NULL; }
          }
          else if(isspace(c) && i > 0) {
            tmp[i] = '\0'; i = 0; item = 1; dots = 0; *total += 1;
            process[j].priority = atoi(tmp);
            process[j].arrived = False;
            process[j].done = False;
            process[j].working = False;
            process[j++].coordinator = False;
            if(j == size / 2) {
              process = realloc(process, (size * 2) * sizeof(*process));
              size *= 2;
            }
          }
        else { free(process); fclose(fptr); return NULL; }
        break;
      }
    }
    process[j].coordinator = True;
    process[j].total = *total;
    process[j++].process = process;
    fclose(fptr);
    return process = realloc(process, j * sizeof(*process));
  }
  else {
    printf("EP1: error opening requested file '%s'.\n", input);
    return 0;
  }
}

/*Checks if the character is a space or tab*/
int is_blank(char c)
{
  return c == ' ' || c == '\t';
}

/*
TODO:
1) Escrever as mensagens de 'paramd' em stderr
2) Outros 5 escalonamentos
3) Readme
4) Slides
5) Escrever output
*/
