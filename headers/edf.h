void *edf(void *);
void initialize_cores_edf(Core *, unsigned int);
void fetch_process_edf(Process *, unsigned int);
void use_core_edf(Process *, Core *, unsigned int);
unsigned int finished_processes_edf(Process *, unsigned int);
unsigned int check_cores_available_edf(Core *, unsigned int);
void do_edf(pthread_t *, Process *, unsigned int *);
Process *select_edf(Process *, unsigned int);
int do_task_edf(Process *process);
unsigned int release_core_edf(Process *, Core *, unsigned int);
