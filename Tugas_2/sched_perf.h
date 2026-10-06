#ifndef SCHED_PERF_H
#define SCHED_PERF_H

typedef struct {
	int pid;
	int arrival_time;
	int burst_time;
	int priority;
	
	int completion_time;
	int turnaround_time;
	int waiting_time;
	int response_time;

	int first_start_time;
} Process;

void calculate_process_metrics(Process p[], int n);
void print_performance(Process p[], int n);
void print_util_throughput(Process p[], int n, int total_simulation_time, int cpu_burst_time);

#endif
