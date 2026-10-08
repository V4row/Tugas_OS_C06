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
    struct Process *nextProcess;
} Process;

typedef struct ProcessList{
    Process *head;
    Process *curr;
} ProcessList;

const char ready[] = "READY";
const char running[] = "RUNNING";
const char waiting[] = "WAITING";
const char terminated[] = "TERMINATED";
int context_switch_count = 0;
const int N = 3; // inisiasi jumlah proses

void dequeue(ProcessList *queue) {
    if (queue->head != NULL) {
        queue->head = queue->head->nextProcess;
    }
}

void save_state(Process *process, char state_str[], int time) {
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

void removeFromQueue(ProcessList *queue, Process *p) {
    if (queue->head == NULL || p == NULL) return;
    if (queue->head == p) {
        queue->head = queue->head->nextProcess;
        return;
    }
    Process *curr = queue->head;
    while (curr->nextProcess != p && curr->nextProcess != NULL) {
        curr = curr->nextProcess;
    }
    if (curr->nextProcess == p) {
        curr->nextProcess = p->nextProcess;
        p->nextProcess = NULL;
    }
}

void enqueue(ProcessList *queue, Process *p) {
    if (queue->head == NULL) {
        queue->head = p;
        return;
    }
    Process *curr = queue->head;
    while (curr->nextProcess != NULL) {
        curr = curr->nextProcess;
    }
    curr->nextProcess = p;
    p->nextProcess = NULL;
}

Process all_processes[N]; // Simpan semua proses agar dapat diakses nilai2 akhirnya
Process *current_process;
ProcessList ready_queue;
int t = 0;
ProcessList not_arrived;

bool eval_arrived(int time) {
    bool flag = false;
    not_arrived.curr = not_arrived.head;
    while (not_arrived.curr != NULL) {
        if (not_arrived.curr->arrival_time == time) {
            if (ready_queue.head != NULL) {
                enqueue(&ready_queue, not_arrived.curr);
            } else {
                ready_queue.head = not_arrived.curr;
            }
            removeFromQueue(&not_arrived, not_arrived.curr);
            flag = true;
        }
        not_arrived.curr = not_arrived.curr->nextProcess;
    }
    return flag;
}

ProcessList reevaluate_queue() {
    // TODO: implementasi penentuan urutan eksekusi
    return ready_queue;
}

void execute() {
    while (ready_queue.head != NULL || not_arrived.head != NULL || current_process != NULL) {
        if (eval_arrived(t)) {
            ready_queue = reevaluate_queue();
            if (ready_queue.head != NULL && current_process == NULL) {
                current_process = ready_queue.head;
            }
            if (ready_queue.head != NULL && current_process != NULL) {
                if (ready_queue.head != current_process) {
                    save_state(current_process, waiting, t);
                    enqueue(&ready_queue, current_process);
                    current_process = ready_queue.head;
                    save_state(current_process, running, t);
                }
            }
        }
        if (current_process != NULL) {
            current_process->remaining_time--;
            if (current_process->remaining_time == 0) {
                save_state(current_process, terminated, t);
                dequeue(&ready_queue);
                current_process = NULL;
                if (ready_queue.head != NULL) {
                    current_process = ready_queue.head;
                }
            }
        }
        t++;
    }
}


int main() {
    ready_queue.head = NULL;
    not_arrived.head = NULL;
    Process p1 = {1, 0, 3, 3};
    not_arrived.head = &p1;

    execute();

    printf("%d\n", p1.pid);
    printf("%d\n", p1.remaining_time);

    return 0;
}
