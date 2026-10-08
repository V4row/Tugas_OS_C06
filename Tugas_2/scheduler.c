#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct State {
    char state[12];
    struct State *nextState;
    int time_of_state;
} State;

typedef struct Process {
    State states;
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int first_start_time;
    int response_time;
} Process;

typedef struct Node {
    // int pid;
    Process *process;
    // int remaining_time;
    struct Node *next;
} Node;

typedef struct executionTimeline{
    int pid;
    int end;
    struct executionTimeline *next;
} executionTimeline;

typedef struct LinkedList{
    executionTimeline *head_timeline;
    executionTimeline *tail_timeline;
    Node *head_node;
} LinkedList;

const char ready[] = "READY";
const char running[] = "RUNNING";
const char waiting[] = "WAITING";
const char terminated[] = "TERMINATED";
int processCount;
int time = 0;
int context_switch_count = 0;
int preemption_count = 0;

//================= Vebian ==================//
void countTurnaroundTime(Process* process){
    process->turnaround_time = process->completion_time - process->arrival_time;
}

void countWaitingTime(Process* process){
    process->waiting_time = process->turnaround_time - process->burst_time;
}

void countResponseTime(Process* process){
    process->response_time = process->first_start_time - process->arrival_time;
}

//================= Vebian ==================//

//================= Rafa ==================//
void inputProcesses(Process processes[]) {
    for (int i = 0; i < processCount; i++) {
        processes[i].pid = i + 1;

        printf("P%d - masukkan Arrival Time dan Burst Time: ", processes[i].pid);
        scanf("%d %d", &processes[i].arrival_time, &processes[i].burst_time);

        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = 0;
        processes[i].first_start_time = -1;

        processes[i].states.nextState = NULL;
    }
}

void printProcessInput(Process processes[]) {
    printf("\n============================================================\n");
    printf("PROCESS INPUT\n");
    printf("============================================================\n");
    printf("PID\tArrival Time\tBurst Time\n");
    
    for (int i = 0; i < processCount; i++) {
        printf("P%-9d %-15d %-15d\n", 
            processes[i].pid, 
            processes[i].arrival_time, 
            processes[i].burst_time);
        }
        printf("============================================================\n\n");
    }
    
//================= Rafa ==================//
    
void printGanttChart(Process processes[], LinkedList* timeline) {
    printf("\n============================================================\n");
    printf("CPU EXECUTION TIMELINE\n");
    printf("============================================================\n");

    // print gantt chart buat yang | P1 | P2 | P3 | P2 | P1 |
    executionTimeline *current = timeline->head_timeline;
    while (current != NULL) {
        printf("| P%-4d", current->pid);
        current = current->next;
    }
    printf("|\n");
    
    // print gantt chart buat yang 0    3    5    7    8    13
    current = timeline;
    printf("%-7d", 0);
    while (current != NULL) {
        if (current->next == NULL) {
            printf("%d\n", current->end);
        } else {
            printf("%-7d", current->end); 
        }
        executionTimeline *temp = current;
        current = current->next;
        // free(temp);
    }
}

//================= Velicia ==================//
void printContextSwitchInfo() {
    printf("=======================================================================\n");
    printf("CONTEXT SWITCH INFORMATION\n");
    printf("=======================================================================\n");

    printf("Total Context Switch: %d\n\n", context_switch_count);
}

void printStateTransitions(Process processes[]) {
    printf("=======================================================================\n");
    printf("PROCESS STATE TRANSITIONS\n");
    printf("=======================================================================\n");

    // Loop setiap proses dan print perubahan statenya
    for (int i = 0; i < processCount; i++) {
        State *s = &processes[i].states;
        printf("P%d : NEW ", processes[i].pid);
        while (s != NULL) {
            printf("-> %s (t=%d) ", s->state, s->time_of_state);
            State *temp = s;
            free(s);
            s = s->nextState;
        }
        printf("\n");
    }
    printf("\n");
}

void saveState(Process *process, const char state_str[], int time) {
    // Inisiasi objek State baru
    // Memory akan di-free setelah state di-print
    State *new_state = malloc(sizeof(State));
    
    // Menyalin nilai state_str ke state pada new_state
    strncpy(new_state->state, state_str, sizeof(new_state->state) - 1);
    new_state->state[sizeof(new_state->state) - 1] = '\0';

    // Menyatakan bahwa state terjadi pada waktu time
    new_state->time_of_state = time;

    new_state->nextState = NULL;
    
    // Looping sampai ketemu state terakhir
    State *curr_state = &process->states;
    if (curr_state != NULL) {
        while (curr_state->nextState != NULL) {
            curr_state = curr_state->nextState;
        }
        // Letakkan state baru setelah state terakhir
        curr_state->nextState = new_state;
    }
}
//================= Velicia ==================//

//================= Vebian ==================//
void newTimeline(LinkedList* timeline, int pid, int end) {
    // Membuat timeline baru
    executionTimeline *new_timeline = malloc(sizeof(executionTimeline));
    new_timeline->pid = pid;
    new_timeline->end = end;

    // Kalau masih kosong langsung buat sebagai head/timeline pertama dan tail
    if (timeline->head_timeline == NULL){
        timeline->head_timeline = new_timeline;
        timeline->tail_timeline = new_timeline;
        return;
    } 

    // Kalau tidak tambah dibagian terakhir 
    timeline->tail_timeline->next = new_timeline;
    timeline->tail_timeline= new_timeline;
}


void addQueue(LinkedList *ready_queue, Process *process, LinkedList *timeline){
    Node *new_process_node = malloc(sizeof(Node));
    new_process_node->process = process;
    new_process_node->next = NULL;
    //masih kosong 
    if (ready_queue->head_node == NULL){
        ready_queue->head_node = new_process_node;
        return;
    }
    //ready_queue lebih kecil dari head_node (preemp)
    if (new_process_node->process->remaining_time < ready_queue->head_node->process->remaining_time) {
        // newTimeline(timeline, ready_queue->head_node->process->pid, time);
        new_process_node->next = ready_queue->head_node;
        ready_queue->head_node = new_process_node;
        context_switch_count++;
        return;
    }

    //tidak kosong ready_queuenya, current akan terus maju sampai 
    Node *current = ready_queue->head_node;
    while (current->next != NULL && current->next->process->remaining_time <= new_process_node->process->remaining_time){
        current = current->next; //catat context swict harusnya catat context swicth dan preemp juga kalau terganti di head_node TODO
    }
    new_process_node->next = current->next;   
    current->next = new_process_node;
}

void addProcessAtTime(Process processes[], int total_process, LinkedList *ready_queue, LinkedList* timeline){
    for (int i = 0; i < total_process; i++){
        if (processes[i].arrival_time == time){
            addQueue(ready_queue, &processes[i], timeline);
        }
    }
}

void removeHead_node(LinkedList *ready_queue){
    Node *process_node = ready_queue->head_node;
    ready_queue->head_node = process_node->next;
    free(process_node);
}

bool checkProcessArrival(Process processes[], int total_process){
    for (int i = 0; i < total_process; i++){
        if (processes[i].arrival_time == time){
            return true;
        }
    }
    return false;
}

//================= Vebian ==================//


//================= Ali ==================//
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
	printf("SCHEDULING PERFORMANCE\n");
	printf("=======================================================================\n");
	printf("Average Waiting Time	: %.2f\n", avg_wt);
	printf("Average Turnaround Time	: %.2f\n", avg_tat);
	printf("Average Response TIme	: %.2f\n", avg_rt);
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
//================= Ali ==================//



void execution(Process processes[], int total_process, LinkedList* timeline){
    LinkedList ready_queue;
    
    ready_queue.head_node = NULL;
    // int current_proccess = -1; //melihat pidnya
    Process *current_proccess;

    Process *prev_process;
    int completed_process = 0;

    printf("Rafael");
    while (completed_process < total_process) {
        if (ready_queue.head_node != NULL){
            Process *head_node_process = ready_queue.head_node->process;
            // Process *head_node_process = findProcessById(processes, ready_queue.head_node->pid, total_process);
            if (head_node_process->remaining_time == 0){
                head_node_process->completion_time = time;
                removeHead_node(&ready_queue);      //catat context swict
                context_switch_count++;
                completed_process++;
                // newTimeline(timeline, head_node_process->pid, time);
                prev_process = NULL;             // preemp tidak dihitung jika head_nodenya sudah habis sendiri
            }
        }
        
        

        if (checkProcessArrival(processes, total_process)){
            addProcessAtTime(processes, total_process, &ready_queue, timeline);
        }

        
        //kalau masih kosong ready_queuenya lanjut ke time berikutnya
        if (ready_queue.head_node == NULL){ 
            printf("Waktu ke: %d\n", time);
            printf("PID : P%d\n\n", ready_queue.head_node->process->pid);
            time++;
            continue;
        }

        current_proccess = ready_queue.head_node->process;
        //cek preemp
        if (current_proccess != prev_process){ 
            if (prev_process != NULL){
                //preem kalau sebelumnya bukan -1 atau sebelumnya proses
                // newTimeline(timeline, prev_process->pid, time);
            }

            if (current_proccess->first_start_time == -1){
                current_proccess->first_start_time = time; // catat pertama kali dimulai
            }
        }
        prev_process = current_proccess;

        //kurangi remaining time
        ready_queue.head_node->process->remaining_time--;

        //lanjut time berikutnya
        printf("Waktu ke: %d\n", time);
        printf("PID : P%d\n\n", ready_queue.head_node->process->pid);
        time++;
    }
}

int main() {
    // scanf("%d", &len);
    // printf("%d", len);
    printf("Masukkan banyaknya process: ");
    scanf("%d", &processCount); 
    
    
    Process processes[processCount];
    LinkedList timeline;
    inputProcesses(processes);

    execution(processes, processCount, &timeline); 

    printGanttChart(processes, &timeline); //siapa pulak yang run ini 
    // processes[0].pid = 1;
    // processes[0].arrival_time = 0;
    // processes[0].burst_time = 8;
    // processes[0].remaining_time = 8;
    // processes[1].pid = 2;
    // processes[1].arrival_time = 4;
    // processes[1].burst_time = 1;
    // processes[0].remaining_time = 1;
    // processes[2].pid = 3;
    // processes[2].arrival_time = 2;
    // processes[2].burst_time = 2;
    // processes[0].remaining_time = 2;
    // processes[3].pid = 4;
    // processes[3].arrival_time = 5;
    // processes[3].burst_time = 3;
    // processes[0].remaining_time = 3;

    // execution(processes);
    // for (int i = 0; i < len; i++){
    //     processes[i].pid = i;
    //     processes[i].burst_time = i+1;
    // }


    // int x = 3;
    // int *ptr = &x;
    
    // printf("%d\n", *ptr);
    // printf("%p\n", ptr);
    // Process p = {1,2,3,4,5,6,7,8};
    // Process p;
    // Head_node jadi dummy var. Baru keisi mulai head_node.nextState
    // p.states.nextState = NULL;
    // p.arrival_time = 4;
    // Process *p_ptr = &p;


    // printf("%d\n", p.arrival_time);
    // printf("%d\n", p_ptr->arrival_time);
    // printf("%p\n", p_ptr);


    // printf("%d\n", processes[2].pid);
    // printf("Test saveState\n");
    // saveState(p_ptr, terminated, 5);
    // printf("%s\n", p.states.nextState->state);
    return 0;
}