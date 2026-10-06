#include <stdio.h>
#include "sched_perf.h"


void calculate_process_metrics(Process p[], int n){
	for(int i=0;i<n;i++){
		p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
		p[i].waiting_time =  p[i].turnaround_time - p[i].burst_time;
		p[i].response_time = p[i].first_start_time - p[i].arrival_time;

	}
}
void print_performance(Process p[], int n){
	double total_wt = 0.0;
	double total_tat = 0.0;
	double total_rt = 0.0;
	for(int i=0;i<n;i++){
		total_wt += p[i].waiting_time;
		total_tat += p[i].turnaround_time;
		total_rt += p[i].response_time;
	}
	double avg_wt = total_wt/n;
	double avg_tat = total_tat/n;
	double avg_rt = total_rt/n;

	printf("=======================================================================\n");
	printf("SHCEDULING PERFORMANCE\n");
	printf("=======================================================================\n");
	printf("Average Waiting Time	: %.2f\n", avg_wt);
	printf("Average Turnaround Time	: %.2f\n", avg_tat);
	printf("Avergae Response TIme	: %.2f\n", avg_rt);
	printf("========================================================================\n\n");

}

void print_util_throughput(Process p[], int n, int total_simulation_time, int cpu_busy_time){
	double cpu_utilization = 0.0;
	double throughput = 0.0;

	if(total_simulation_time > 0){
		cpu_utilization = ((double)cpu_busy_time / total_simulation_time) * 100.0;
		throughput = (double)n / total_simulation_time;
	}
	printf("===============================================================\n");
	printf("CPU Utilization and Throughput");
	printf("===============================================================\n");
	printf("CPU Utilization	: %.2f\n", cpu_utilization);
	printf("Throughput	: %.2f\n", throughput);
	printf("===============================================================\n\n");

}
