#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

typedef struct executionTimeline{
    int pid;
    int end;
    struct executionTimeline *next;
} executionTimeline;

typedef struct Timeline{
    executionTimeline *head_timeline;
    executionTimeline *tail_timeline;
} Timeline;

const char ready[] = "READY";
const char running[] = "RUNNING";
const char waiting[] = "WAITING";
const char terminated[] = "TERMINATED";
int context_switch_count = 0;
// const N = 3; // inisiasi jumlah proses

void dequeue(ProcessList *queue) {
    // Jika queue tidak kosong
    if (queue->head != NULL) {
        // Remove elemen pertama
        queue->head = queue->head->nextProcess;
    }
}

void enqueue(ProcessList *queue, Process *p) {
    // Jika queue kosong, p jadi elemen pertama
    if (queue->head == NULL) {
        queue->head = p;
        p->nextProcess = NULL;
        return;
    }

    Process *curr = queue->head;
    // Cari process terakhir
    while (curr->nextProcess != NULL) {
        curr = curr->nextProcess;
    }
    curr->nextProcess = p;
    p->nextProcess = NULL;
}

void save_state(Process *process, char state_str[], int time) {
    // Initiate new State
    State *new_state = malloc(sizeof(State));
    
    // Menyalin nilai state_str ke state pada new_state
    strncpy(new_state->state, state_str, sizeof(new_state->state) - 1);
    new_state->state[sizeof(new_state->state) - 1] = '\0';

    // Menyatakan bahwa state terjadi pada waktu time
    new_state->time_of_state = time;

    new_state->nextState = NULL;

    // Looping sampai ketemu state terakhir
    State *curr_state = &process->states;
    while (curr_state->nextState != NULL) {
        curr_state = curr_state->nextState;
    }
    // Letakkan state baru setelah state terakhir
    curr_state->nextState = new_state;
}

void removeFromQueue(ProcessList *queue, Process *p) {
    if (queue->head == NULL || p == NULL) return;
    if (queue->head == p) {
        queue->head = queue->head->nextProcess;
        p->nextProcess = NULL;
        return;
    }
    Process *curr = queue->head;
    // Cari proses sebelum p
    while (curr->nextProcess != p && curr->nextProcess != NULL) {
        curr = curr->nextProcess;
    }
    // Kalau ketemu, proses sebelum p menunjuk ke proses setelah p (menghapus p)
    if (curr->nextProcess == p) {
        curr->nextProcess = p->nextProcess;
        p->nextProcess = NULL;
    }
}

// Process *all_processes[N]; // Simpan semua proses agar dapat diakses nilai2 akhirnya
// Process *current_process; // Proses yang sedang running
// ProcessList ready_queue; // Proses yang ready, running, atau waiting
// int t = 0; // Waktu saat ini
// ProcessList not_arrived; // Proses yang belum datang

enum { N = 4 };
Process *all_processes[N]; // Simpan semua proses agar dapat diakses nilai2 akhirnya
Process *current_process;
ProcessList ready_queue;
int t = 0;
ProcessList not_arrived;
Timeline timeline;

bool eval_arrived(int time) {
    bool flag = false; // Jika true, ada proses baru datang (bisa lebih dari 1)
    not_arrived.curr = not_arrived.head;
    while (not_arrived.curr != NULL) {
        Process *next = not_arrived.curr->nextProcess;
        // Jika proses datang saat time
        if (not_arrived.curr->arrival_time == time) {
            enqueue(&ready_queue, not_arrived.curr);
            save_state(not_arrived.curr, ready, time);
            removeFromQueue(&not_arrived, not_arrived.curr);
            flag = true;
        }
        // Mengunjungi setiap proses pada not_arrived
        not_arrived.curr = next;
    }
    return flag;
}

void insertToQueue(ProcessList* sorted_ready_queue, Process* process) {
    process->nextProcess = NULL;
    //kalau masih kosong
    if (sorted_ready_queue->head == NULL) {
        sorted_ready_queue->head = process;
    }
    // Process yang dingin dimasukkan lebih singkat waktunya
    else if (process->remaining_time < sorted_ready_queue->head)
    {
        process->nextProcess = sorted_ready_queue->head;
        sorted_ready_queue->head = process;
    }
    //Selain itu (ditengah-tengah)
    else{
        sorted_ready_queue->curr = sorted_ready_queue->head;
        while (sorted_ready_queue->curr->nextProcess != NULL && sorted_ready_queue->curr->nextProcess->remaining_time <= process->remaining_time){
            //current maju
            sorted_ready_queue->curr= sorted_ready_queue->curr->nextProcess;
        }

        process->nextProcess = sorted_ready_queue->curr->nextProcess;
        sorted_ready_queue->curr->nextProcess = process;
    }
    
}

ProcessList reevaluate_queue() {
    // TODO: implementasi penentuan urutan eksekusi
    ProcessList sorted_ready_queue;
    sorted_ready_queue.head = NULL;
    sorted_ready_queue.curr = NULL;
    Process* current = ready_queue.head;
    while (current != NULL) {
        // Ambil next Process dari current
        Process *next = current->nextProcess;
        insertToQueue(&sorted_ready_queue, current);
        // Lanjut ke proses berikutnya
        current = next;
    }
    sorted_ready_queue.curr = sorted_ready_queue.head;
    return sorted_ready_queue;
}

void newTimeline(Timeline* timeline, int pid, int end) {
    // Membuat timeline baru
    executionTimeline *new_timeline = malloc(sizeof(executionTimeline));
    new_timeline->pid = pid;
    new_timeline->end = end;
    new_timeline->next = NULL;
 
    // Kalau masih kosong langsung buat sebagai head/timeline pertama dan tail
    if (timeline->head_timeline == NULL){
        timeline->head_timeline = new_timeline;
        timeline->tail_timeline = new_timeline;
        return;
    }
 
    // Kalau tidak tambah dibagian terakhir
    timeline->tail_timeline->next = new_timeline;
    timeline->tail_timeline = new_timeline;
}

void addTimeline(Timeline* timeline, int pid, int time) {
    if (timeline->tail_timeline != NULL && timeline->tail_timeline->pid == pid) {
        timeline->tail_timeline->end = time + 1;
        return;
    }
    newTimeline(timeline, pid, time + 1);
}

void execute() {
    // Selama masih ada proses di ready_queue atau not_arrived atau yang sedang dijalankan
    while (ready_queue.head != NULL || not_arrived.head != NULL || current_process != NULL) {
        // Jika ada proses yang baru datang
        if (eval_arrived(t)) {
            // Evaluasi ulang urutan eksekusi proses
            ready_queue = reevaluate_queue();
            // Jika ada proses di ready_queue dan tidak ada proses yang sedang dijalankan
            if (ready_queue.head != NULL && current_process == NULL) {
                // jalankan proses pertama di ready_queue
                current_process = ready_queue.head;
                save_state(current_process, running, t);
            }
            // Jika ada proses di ready_queue dan ada proses yang sedang dijalankan
            if (ready_queue.head != NULL && current_process != NULL) {
                // Jika proses pertama di ready_queue berbeda dengan proses yang sedang dijalankan
                if (ready_queue.head != current_process) {
                    // Preempt proses sekarang dan ganti dengan elemen pertama ready_queue
                    save_state(current_process, waiting, t);
                    // enqueue(&ready_queue, current_process);
                    current_process = ready_queue.head;
                    save_state(current_process, running, t);
                    context_switch_count++; // catat context switch
                }
            }
        }
        // Jika sedang ada proses yang berjalan
        if (current_process != NULL) {
            // Eksekusi proses (kurangi remaining_time)
            current_process->remaining_time--;
            addTimeline(&timeline, current_process->pid, t);
            // Jika proses sudah selesai dieksekusi
            if (current_process->remaining_time == 0) {
                // Simpan state dan keluarkan dari ready_queue
                save_state(current_process, terminated, t);
                dequeue(&ready_queue);
                // Kosongkan current_process
                // Misalnya, setelah ini CPU idle, proses belum datang dan 
                // ready_queue sudah kosong. Kalau current_process masih
                // menunjuk ke proses sebelumnya, remaining_time proses
                // akan terus di-decrement (jadi kurang dari 0)
                current_process = NULL;
                // Jika ada proses berikutnya di ready_queue
                if (ready_queue.head != NULL) {
                    // jalankan proses tersebut
                    current_process = ready_queue.head;
                    save_state(current_process, running, t);
                    context_switch_count++;
                }
            }
        }
        // Increment waktu
        t++;
    }
}

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
        State *s = &all_processes[i]->states;
        printf("P%d : NEW ", all_processes[i]->pid);
        while (s != NULL) {
            printf("-> %s (t=%d) ", s->state, s->time_of_state);
            s = s->nextState;
        }
        printf("\n");
    }
    printf("\n");
}

void printGanttChart(Timeline* timeline) {
    printf("\n============================================================\n");
    printf("CPU EXECUTION TIMELINE\n");
    printf("============================================================\n");

    // print gantt chart buat yang | P1 | P2 | P3 | P2 | P1 |
    executionTimeline *current = timeline->head_timeline;
    while (current != NULL) {
        // kalau ternyata tidak ada proses masuk diawal
         if (current->pid == 0) {
            printf("| IDLE ");
        } else {
            printf("| P%-4d", current->pid);
        }
        current = current->next;
    }
    printf("|\n");
    
    // print gantt chart buat yang 0    3    5    7    8    13
    current = timeline->head_timeline;
    printf("%-7d", 0);
    while (current != NULL) {
        executionTimeline *next = current->next;
        if (next == NULL) {
            printf("%d\n", current->end);
        } else {
            printf("%-7d", current->end);
        }
        free(current);
        current = next;
    }
    //kosongi
    timeline->head_timeline = NULL;
    timeline->tail_timeline = NULL;
}


int main() {
    ready_queue.head = NULL;
    not_arrived.head = NULL;
    Process p1 = {1, 0, 8, 8};
    Process p2 = {2, 1, 4, 4};
    Process p3 = {3, 2, 2, 2};
    Process p4 = {4, 3, 5, 5};
    not_arrived.head = &p1;
    p1.nextProcess = &p2;
    p2.nextProcess = &p3;
    p3.nextProcess = &p4;
    p4.nextProcess = NULL;
    all_processes[0] = &p1;
    all_processes[1] = &p2;
    all_processes[2] = &p3;
    all_processes[3] = &p4;

    // Process p1 = {1, 0, 3, 3};
    // Process p2 = {2, 0, 5, 5};
    // Process p3 = {3, 0, 2, 2};
    // not_arrived.head = &p1;
    // p1.nextProcess = &p2;
    // p2.nextProcess = &p3;
    // p3.nextProcess = NULL;
    // all_processes[0] = &p1;
    // all_processes[1] = &p2;
    // all_processes[2] = &p3;

    execute();

    printGanttChart(&timeline);
    printContextSwitchInfo();
    printStateTransitions();

    return 0;
}
