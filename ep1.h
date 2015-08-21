#define FCFS 1       /*First Come First Served*/
#define SJF  2       /*Shortest Job First*/
#define SRTN 3       /*Shortest Remaining Time Next*/
#define RR   4       /*Round Robin*/
#define PS   5       /*Priority Scheduling*/
#define RRTS 6       /*Rigid Real Time Scheduling (with rigid deadlines)*/

typedef int boolean;
#define False 0
#define True 1

typedef struct process {
  boolean coordinator;   /*Is this a coordinator thread?*/
  float   arrival;       /*The time the process takes to reach the system*/
  boolean arrived;       /*Have the process arrived?*/
  char   name[64];       /*The name of the process. Works as an identifier*/
  float  duration;       /*CPU's real time consumed by the process in the simulation*/
  float  deadline;       /*The maximum amount of time this process can take to finish*/
  int    priority;       /*Priority. An integer in the range [-20, 19]*/
  boolean working;       /*Process is running in a CPU?*/
  boolean    done;       /*CPU done with the process?*/
  /*Used only by the coordinator.
    For i processes, the coordinator is at the index i and the rest of the processes
    are at indexes 0 to i - 1 of the array pointes by *process*/
  unsigned int total;      /*Total number of processes running*/
  boolean paramd;          /*Register if the 4th optional parameter was passed*/
  struct process *process; /*All the processes*/
} Process;

typedef struct core {
  boolean available;       /*Is this core available?*/
  Process *process;        /*If the core is busy, register the process using this core*/
} Core;

/*Simulator general functions*/
int run(char **, char *);
Process *readtfile(Process *, char *, char *, unsigned int *, char *);
void fetchprocess(Process *, unsigned int, clock_t);
void use_core(Process *, Core *, unsigned int);

/*Helper functions*/
int isblank(char c);

/*SJF-specific functions*/
void initiate_sjf(pthread_t *, Process *, unsigned int *);
void *sjf(void *);
Process *select_sjf(Process *, unsigned int, clock_t);
