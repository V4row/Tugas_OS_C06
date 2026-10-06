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

typedef struct Linkedlist{
    Node *head;
} Linkedlist;

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

int execution(Process processes[]){
    int time = 0;
    Linkedlist queue;
}

int main() {
    int len;
    // scanf("%d", &len);
    // printf("%d", len);
    len = 4;
    Process processes[len];
    processes[0].pid = 1;
    processes[0].arrival_time = 0;
    processes[0].burst_time = 8;
    processes[1].pid = 2;
    processes[1].arrival_time = 4;
    processes[1].burst_time = 1;
    processes[2].pid = 3;
    processes[2].arrival_time = 2;
    processes[2].burst_time = 2;
    processes[3].pid = 4;
    processes[3].arrival_time = 5;
    processes[3].burst_time = 3;
    
    execution(processes);
    // for (int i = 0; i < len; i++){
    //     processes[i].pid = i;
    //     processes[i].burst_time = i+1;
    // }


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


    printf("%d\n", processes[2].pid);

    return 0;
}