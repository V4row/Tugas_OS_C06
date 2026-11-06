#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include "malloc_free.h"

#define BUFFER_SIZE 4096

void print_vmsize_vmrss(void) {
    // TODO
}

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
    // TODO: print VmSize & VmRSS to file
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
    // TODO: print VmSize & VmRSS to file

    free(integers);
    fclose(file);

    // Menyalin isi file C6-malloc_std-[timestamp].txt ke file C6-malloc_sys-[timestamp].txt
    file = fopen(stdLibName, "r");
    char buffer[BUFFER_SIZE];
    int fileSys = open(sysCallName, O_WRONLY | O_CREAT);
    size_t bytes_read;
    ssize_t bytes_written;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        bytes_written = write(fileSys, buffer, bytes_read);
    }
    fclose(file);
    close(fileSys);
}