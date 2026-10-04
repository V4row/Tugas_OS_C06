#include <stdio.h>
#include <stdlib.h>

typedef struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int first_start_time;
    int response_time;
} Process;

typedef struct Node {
    int pid;
    struct Node *next;
} Node;

void count_turnaround_time(Process* process){
    process->turnaround_time = process->completion_time - process->arrival_time;
}

void count_waiting_time(Process* process){
    process->waiting_time = process->turnaround_time - process->burst_time;
}

void count_response_time(Process* process){
    process->response_time = process->first_start_time - process->arrival_time;
}



Process* find_process_by_pid(Process processes[], int pid_target, int len) {
    for (int i = 0; i < len; i++){
        if (processes[i].pid == pid_target) {
            return &processes[i];
        }
    }
    return NULL;
}

int main() {
    int x = 3;
    int *ptr = &x;
    
    printf("%d\n", *ptr);
    printf("%p\n", ptr);
    // Process p = {1,2,3,4,5,6,7,8};
    Process p;
    p.arrival_time = 4;
    Process *p_ptr = &p;


    printf("%d\n", p.arrival_time);
    printf("%d\n", p_ptr->arrival_time);
    printf("%p\n", p_ptr);

    int len;
    // scanf("%d", &len);
    // printf("%d", len);
    len = 5;
    Process processes[len];
    for (int i = 0; i < len; i++){
        processes[i].pid = i;
        processes[i].burst_time = i+1;
    }

    printf("%d\n", processes[2].pid);

    return 0;
}