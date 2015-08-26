#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "ep1.h"
#include "rr.h"

/*Round Robin*/
void *rr(void *args)
{
  Process *process = ((Process*) args);

	if(process->coordinator) {
    unsigned int count = 0, cores = sysconf(_SC_NPROCESSORS_ONLN);
    float quantum = 4.0;
    Process *next = NULL;
    Rotation *list;
    Core *core;

    core = malloc(cores * sizeof(*core));
    list = malloc(sizeof(*list));
    list->process = NULL; list->next = list;
    initialize_cores_rr(core, cores);

    start = clock();
    while(count != process->total) {

      release_cores_rr(process->process, process->total, core, cores, quantum);
      fetch_process_rr(process->process, process->total, list);
      next = select_rr(next, list);

      if(next != NULL) use_core_rr(next, core, cores);
      count = finished_processes_rr(process->process, process->total);
      check_cores_available_rr(core, cores); /*
      printf("\n\n\nAQUI %u\n\n\n", count);*/
    }
    /*TODO!!! free list AQUI*/
    free(core); core = NULL;
  }
	else {
    int completed = 0;
    /*Wait the system know the process has arrived*/
    sem_wait(&(process->next_stage));
    /*Wait the system assigns a CPU to the process*/
    while(!completed) {
      sem_wait(&(process->next_stage));
      /*do task here*/
      completed = do_task_rr(process);
    }
    /*This thread is done. Mutex to write 'done' safely*/
    pthread_mutex_lock(&(process->mutex));
    process->done = True;
    pthread_mutex_unlock(&(process->mutex));
  }
	return NULL;
}

/*Assigns a process to a core*/
void use_core_rr(Process *process, Core *core, unsigned int cores)
{
  unsigned int i = 0;
  while(i < cores) {
    if(core[i].available && !process->working) {
      core[i].available = False;
      core[i].process = process;
      core[i].process->working = True;
      if(paramd) printf("Process '%s' assigned to core %d\n", core[i].process->name, i);
      sem_post(&(core[i].process->next_stage));
      core[i].timer = clock();
      break;
    }
    else i++;
  }
}

/*System checks if a CPU that was previously in use is available*/
unsigned int check_cores_available_rr(Core *core, unsigned int cores)
{
  unsigned int i, count = 0;
  for(i = 0; i < cores; i++) {
    if(core[i].process != NULL) {
      /*Mutex to read 'done' safely*/
      pthread_mutex_lock(&(core[i].process->mutex));
      if(core[i].process->done) {
        if(paramd) printf("Process '%s' has released CPU %u.\n", core[i].process->name, i);
        core[i].available = True;
      }
      pthread_mutex_unlock(&(core[i].process->mutex));
      if(core[i].available) core[i].process = NULL;
    }
    if(core[i].available) count++;
  }
  return count;
}

/*Get the number of finished processes*/
unsigned int finished_processes_rr(Process *process, unsigned int total)
{
  unsigned int i, count = 0;
  for(i = 0; i < total; i++) {
    /*Mutex to read 'done' safely*/
    pthread_mutex_lock(&(process[i].mutex));
    if(process[i].done) {
      count++;
      if(process[i].working) {
        process[i].working = False;
        if(paramd) printf("Must print the contents of the output for this process here. Substitute this message.\n");
      }
    }
    pthread_mutex_unlock(&(process[i].mutex));
  }
  return count;
}

/*Initialize cores*/
void initialize_cores_rr(Core *core, unsigned int cores)
{
  unsigned int count;
  for(count = 0; count < cores; count++) {
    core[count].available = True;
    core[count].process = NULL;
    core[count].timer = clock();
  }
}

/*Initiate threads to run rr scheduling*/
void do_rr(pthread_t *threads, Process *process, unsigned int *total)
{
  unsigned int i;
  for(i = 0; i <= *total; i++) {
    if(pthread_create(&threads[i], NULL, rr, &process[i])) {
      printf("Error creating thread.\n");
      return;
    }
  }
  for(i = 0; i <= *total; i++) {
    if(pthread_join(threads[i], NULL)) {
      printf("Error joining process thread.\n");
      return;
    }
  }
}

/*Running process*/
int do_task_rr(Process *process)
{
  float sec, dl;
  clock_t duration = clock();

  while((sec = (((float)(clock() - duration)) / CLOCKS_PER_SEC)) < process->remaining) {
    if(!process->working) {
      process->remaining -= sec;
      return 0;
    }
    if((dl = (((float)(clock() - start)) / CLOCKS_PER_SEC)) > process->deadline) {
      /*TODO: Must print this only the first time*/
      /*printf("Process '%s' deadline. duration: %f  deadline: %f\n", process->name, dl, process->deadline);*/
    }
  }
  return 1;
}

/*Look up for new processes*/
void fetch_process_rr(Process *process, unsigned int total, Rotation *list)
{
  Rotation *next;
  unsigned int i;
  float sec = ((float)(clock() - start)) / CLOCKS_PER_SEC;

  next = list;
  while(next->next != list) next = next->next;
  for(i = 0; i < total; i++)
    if(sec >= process[i].arrival && !process[i].arrived) {
      if(list->process == NULL) {
        list->process = &process[i];
        list->next = list;
      }
      else {
        Rotation *new;
        new = malloc(sizeof(*new));
        next->next = new;
        new->process = &process[i];
        new->next = list;
        next = next->next;
      }

      process[i].arrived = True;
      sem_post(&(process[i].next_stage));
      if(paramd) printf("%s has arrived (trace file line %u)\n", process[i].name, i + 1);
    }
}

/*Selects next process to get a CPU*/
Process *select_rr(Process *next, Rotation *list)
{
  if(next == NULL && list->process != NULL) return list->process;
  else if(next != NULL) {
    Rotation *r;
    for(r = list; r->process != next; r = r->next) continue;
    return r->next->process;
  }
  return NULL;
}

/*Release cores from process that are exceeding the quantum*/
void release_cores_rr(Process *process, unsigned int total, Core *core, unsigned int cores, float quantum)
{
  unsigned int i;
  float sec;

  for(i = 0; i < cores; i++) {
    if(core[i].process != NULL && (sec = ((float)(clock() - core[i].timer)) / CLOCKS_PER_SEC) > quantum) {
      int j;
      for(j = 0; j < total; j++) if(core[i].process == &process[j]) {
        process[j].working = False;
        printf("Process '%s' has been removed from CPU %u. Quantum time (4.0s) expired.\n", core[i].process->name, i);
        break;
      }
      core[i].available = True;
      core[i].process = NULL;
    }
  }
}
