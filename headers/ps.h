void *ps(void *);
void initialize_cores_ps(Core *, unsigned int);
void fetch_process_ps(Process *, unsigned int);
void use_core_ps(Process *, Core *, unsigned int);
unsigned int finished_processes_ps(Process *, unsigned int);
unsigned int check_cores_available_ps(Core *, unsigned int);
void do_ps(pthread_t *, Process *, unsigned int *);
Process *select_ps(Process *, unsigned int);
int do_task_ps(Process *process);
unsigned int release_core_ps(Process *, Core *, unsigned int);