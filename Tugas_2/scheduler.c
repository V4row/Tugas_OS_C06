#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct State {
    char state[10];
    int time_of_state;
    struct State *nextState;
} State;

typedef struct StateList {
    State head;
    State last;
} StateList;

typedef struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int first_start_time;
    int response_time;
    StateList states;
} Process;

typedef struct Node {
    int pid;
    int remaining_time;
    struct Node *next;
} Node;

typedef struct Linkedlist{
    Node *head;
} Linkedlist;

const char ready[] = "READY";
const char running[] = "RUNNING";
const char waiting[] = "WAITING";
const char exit_state[] = "EXIT";

void count_turnaround_time(Process* process){
    process->turnaround_time = process->completion_time - process->arrival_time;
}

void count_waiting_time(Process* process){
    process->waiting_time = process->turnaround_time - process->burst_time;
}

void count_response_time(Process* process){
    process->response_time = process->first_start_time - process->arrival_time;
}

void save_state(Process *process, char state_str[], int time) {
    // Get string length
    int length = sizeof(state_str) / sizeof(state_str[0]);
    // Initiate new State
    State *new_state = malloc(sizeof(State));
    
    strncpy(new_state->state, state_str, sizeof(new_state->state) - 1);
    new_state->state[sizeof(new_state->state) - 1] = '\0';
    new_state->time_of_state = time;
    new_state->nextState = NULL;

    State *curr_state = &process->states.head;
    while (curr_state->nextState != NULL) {
        curr_state = curr_state->nextState;
    }
    curr_state->nextState = new_state;
}

void addQueue(Linkedlist *ready_queue, Process *process){
    Node *new_process = malloc(sizeof(Node));
    new_process->pid = process->pid;
    new_process->remaining_time = process->remaining_time;
    new_process->next = NULL;
    //masih kosong atau lebih kecil dari head
    if (ready_queue->head == NULL || new_process->remaining_time < ready_queue->head->remaining_time){
        ready_queue->head = &new_process;
        return;
    }

    //tidak kosong ready_queuenya
    Node *current = ready_queue->head;
    while (current->next != NULL && current->next->remaining_time <= new_process->remaining_time){
        current = current->next; //catat context swict harusnya catat context swicth dan preemp juga kalau terganti di head TODO
    }
    new_process->next = current->next;   
    current->next = new_process;
}

void add_process_at_time(Process processes[], int total_process, Linkedlist *ready_queue, int time){
    for (int i = 0; i < total_process; i++){
        if (processes[i].arrival_time == time){
            addQueue(ready_queue, &processes[i]);
        }
    }
}

void removeHead(Linkedlist *ready_queue){
    Node *process = ready_queue->head;
    ready_queue->head = process->next;
    free(process);
}

bool new_process_arrived(Process processes[], int total_process, int time){
    for (int i = 0; i < total_process; i++){
        if (processes[i].arrival_time == time){
            return true;
        }
    }
    return false;
}

Process* find_process_by_pid(Process processes[], int pid_target, int len) {
    if (pid_target <= len){
        return &processes[pid_target - 1];
    }
    return NULL;
}

int execution(Process processes[], int total_process){
    int time = 0;
    Linkedlist ready_queue;
    ready_queue.head = NULL;
    int current_proccess = -1; //melihat pidnya
    int prev_process = -1;
    int completed_process = 0;

    while (completed_process <= total_process) {
        if (ready_queue.head != NULL){
            Process *head_process = find_process_by_pid(processes, ready_queue.head->pid, total_process);
            if (head_process->remaining_time == 0){
                head_process->completion_time = time;
                removeHead(&ready_queue);      //catat context swict
                completed_process++;
                prev_process = -1;             // preemp tidak dihitung jika headnya sudah habis sendiri
            }
        }
        
        if (new_process_arrived(processes, total_process, time)){
            add_process_at_time(processes, total_process, &ready_queue, time);
        }

        //kalau masih kosong ready_queuenya lanjut ke time berikutnya
        if (ready_queue.head == NULL){ 
            time++;
            continue;
        }

        current_proccess = ready_queue.head->pid;
        //cek preemp
        if (current_proccess != prev_process){ 
            if (prev_process != -1){
                //preem kalau sebelumnya bukan -1 atau sebelumnya proses lain
            }
            Process *currentProcess = find_process_by_pid(processes, current_proccess, total_process);
            if (currentProcess->first_start_time == -1){
                currentProcess->first_start_time = time; // catat pertama kali dimulai
            }
        }
        prev_process = current_proccess;

        //kurangi remaining time
        ready_queue.head->remaining_time--;
        find_process_by_pid(processes, current_proccess, total_process)->remaining_time--;


        //lanjut time berikutnya
        time++;

    }
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
    processes[0].remaining_time = 8;
    processes[1].pid = 2;
    processes[1].arrival_time = 4;
    processes[1].burst_time = 1;
    processes[0].remaining_time = 1;
    processes[2].pid = 3;
    processes[2].arrival_time = 2;
    processes[2].burst_time = 2;
    processes[0].remaining_time = 2;
    processes[3].pid = 4;
    processes[3].arrival_time = 5;
    processes[3].burst_time = 3;
    processes[0].remaining_time = 3;
    
    // execution(processes);
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
    // Head jadi dummy var. Baru keisi mulai head.nextState
    p.states.head.nextState = NULL;
    p.arrival_time = 4;
    Process *p_ptr = &p;


    printf("%d\n", p.arrival_time);
    printf("%d\n", p_ptr->arrival_time);
    printf("%p\n", p_ptr);


    printf("%d\n", processes[2].pid);
    printf("Test save_state\n");
    save_state(p_ptr, exit_state, 5);
    printf("%s\n", p.states.head.nextState->state);
    return 0;
}