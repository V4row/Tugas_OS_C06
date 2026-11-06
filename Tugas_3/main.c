#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096


// ====== Tempat buat fungsi ======

void print_malloc_free_output(int numOfInts) {
    // Membuat timestamp untuk nama file
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char stdLibName[100];
    char sysCallName[100];
    strftime(stdLibName, sizeof(stdLibName), "C6-malloc_std-%d-%m-%Y_%H:%M:%S.txt", t);
    strftime(sysCallName, sizeof(sysCallName), "C6-malloc_sys-%d-%m-%Y_%H:%M:%S.txt", t);
    
    // Menjalankan logika malloc/free serta mencetak output ke terminal
    // Output dicetak ke file dengan menggunakan Standard C Library
    FILE *file = fopen(stdLibName, "w+");
    fprintf(file, "You picked malloc/free\n");
    // Print VmSize & VmRSS ke terminal dan file log
    printf("VmSize Before: %ld KB | VmRSS Before: %ld KB\n", get_vmsize_kb(), get_vmrss_kb());
    fprintf(file, "VmSize Before: %ld KB | VmRSS Before: %ld KB\n\n", get_vmsize_kb(), get_vmrss_kb());
    int *integers = malloc(numOfInts * sizeof(int));
    // Isi 10 nilai terakhir saja
    for (int i = 1; i <= 10; i++) {
        integers[numOfInts-i] = numOfInts - (i-1);
    }
    // Print output
    printf("Starting memory address: %p\n", integers);
    fprintf(file, "Starting memory address: %p\n", integers);
    printf("Printing first 10 values in reverse order: \n");
    fprintf(file, "Printing first 10 values in reverse order: \n");
    for (int i = 1; i <= 10; i++) {
        printf("%d\n", integers[numOfInts-i]);
        fprintf(file, "%d\n", integers[numOfInts-i]);
    }
    // Print VmSize & VmRSS ke terminal dan file log
    printf("\nVmSize After: %ld KB | VmRSS After: %ld KB\n", get_vmsize_kb(), get_vmrss_kb());
    fprintf(file, "\nVmSize After: %ld KB | VmRSS After: %ld KB\n", get_vmsize_kb(), get_vmrss_kb());

    free(integers);
    fclose(file);

    // Menyalin isi file C6-malloc_std-[timestamp].txt ke file C6-malloc_sys-[timestamp].txt
    file = fopen(stdLibName, "r");
    char buffer[BUFFER_SIZE];
    int fileSys = open(sysCallName, O_WRONLY | O_CREAT, 0666);
    size_t bytes_read;
    ssize_t bytes_written;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        bytes_written = write(fileSys, buffer, bytes_read);
    }
    fclose(file);
    close(fileSys);
}


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
    printf("Masukkan pilihan (ketik '2' untuk malloc/free): ");
    scanf("%d",&option);

    while(running){

        switch (option)
        {
        case 1:
            /* allocate malloc */
            break;

        case 2:
            /* allocate mmap */
            int allocation_ammount;
            printf("You picked malloc/free\n");
            printf("How many integers to allocate: ");
            scanf("%d", &allocation_ammount);
            printf("\n");
            print_malloc_free_output(allocation_ammount);
            running = false;
            break;

        case 3:
            /* varian 6 */
            break;

        case 4:
            running = false;
            break;
        
        default:
            printf("Tidak valid!");
            break;
        }
    }
    return 0;
}