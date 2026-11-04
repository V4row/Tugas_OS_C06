#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "malloc_free.h"

void print_vmsize_vmrss(void) {
    // TODO
}

void print_malloc_free_output(int numOfInts) {
    int *integers = malloc(numOfInts * sizeof(int));
    // Isi 10 nilai terakhir saja
    for (int i = 1; i <= 10; i++) {
        integers[numOfInts-i] = numOfInts - (i-1);
    }
    // Print output
    printf("Starting memory address: %p\n", integers);
    printf("Printing first 10 values in reverse order: \n");
    for (int i = 1; i <= 10; i++) {
        printf("%d\n", integers[numOfInts-i]);
    }
    free(integers);
}