#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int context_switch_count = 0;

typedef struct State {
    char state[10];
    int time_of_state;
    struct State *nextState;
} State;

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
    State states;
    struct Process *nextProcess;
} Process;

typedef struct ProcessList{
    Process *head;
    Process *curr;
} ProcessList;

const int N = 3;

Process all_processes[N];

void printContextSwitchInfo() {
    printf("=======================================================================\n");
    printf("CONTEXT SWITCH INFORMATION\n");
    printf("=======================================================================\n");

    printf("Total Context Switch: %d\n\n", context_switch_count);
}

void printStateTransitions() {
    printf("=======================================================================\n");
    printf("PROCESS STATE TRANSITIONS\n");
    printf("=======================================================================\n");

    for (int i = 0; i < N; i++) {
        State *s = &all_processes[i].states;
        printf("P%d : NEW ", all_processes[i].pid);
        while (s != NULL) {
            printf("-> %s (t=%d) ", s->state, s->time_of_state);
            s = s->nextState;
        }
        printf("\n");
    }
    printf("\n");
}

const char ready[] = "READY";
const char running[] = "RUNNING";
const char waiting[] = "WAITING";
const char terminated[] = "TERMINATED";

void save_state(Process *process, char state_str[], int time) {
    // Initiate new State
    State *new_state = malloc(sizeof(State));
    
    // Menyalin nilai state_str ke state pada new_state
    strncpy(new_state->state, state_str, sizeof(new_state->state) - 1);
    new_state->state[sizeof(new_state->state) - 1] = '\0';

    // Menyatakan bahwa state terjadi pada waktu time
    new_state->time_of_state = time;

    new_state->nextState = NULL;

    
    State *curr_state = &process->states;
    if (curr_state != NULL) {
        // Looping sampai ketemu state terakhir
        while (curr_state->nextState != NULL) {
            curr_state = curr_state->nextState;
        }
    }
    // Letakkan state baru setelah state terakhir
    curr_state->nextState = new_state;
}

int main() {
    // Testing fungsi printContextSwitchInfo dan printStateTransitions()
    printContextSwitchInfo();
    Process p1 = {1, 0, 3, 3};
    save_state(&p1, ready, 0);
    save_state(&p1, running, 0);
    save_state(&p1, terminated, 3);
    all_processes[0] = p1;
    printStateTransitions();
    return 0;
}