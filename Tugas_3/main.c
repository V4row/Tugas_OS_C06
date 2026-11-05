#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// ====== Tempat buat fungsi ======
long get_vmrss_kb() {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return -1;
    
    // Mencari baris VmRSS lalu mengambil nilainya
    char line[128];
    long vmrss = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &vmrss);
            break;
        }
    }
    fclose(file);
    return vmrss;
}

long get_vmsize_kb() {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return -1;
    
    char line[128];
    long vmsize = 0;
    while (fgets(line, sizeof(line), file)) {
        // Mencari string "VmSize:"
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%ld", &vmsize);
            break;
        }
    }
    fclose(file);
    return vmsize;
}

// ====== Tempat buat fungsi ======

int main(){
    int option;
    bool running = true;
    scanf("%d",&option);

    while(running){

        switch (option)
        {
        case 1:
            /* allocate malloc */
            break;

        case 2:
            /* allocate mmap */
            break;

        case 3:
            /* varian 6 */
            break;

        case 4:
            running = false;
            break;
        
        default:
            printf("Tdak valid!");
            break;
        }
    }
    return 0;
}