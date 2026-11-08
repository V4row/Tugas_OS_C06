#include <stdio.h>
#include "banner.h"

void print_banner() {
    printf("=== KELOMPOK XX - TUGAS MEMORI ===\n");
    printf("1. Allocate with malloc/free\n");
    printf("2. Allocate with mmap/munmap\n");
    printf("3. Eksekusi Fitur Variasi Kelompok\n");
    printf("4. Exit\n");
}

int get_option() {
    int option;
    printf("Input: ");
    if (scanf("%d", &option) != 1) {
        // Membersihkan buffer jika input bukan angka
        while (getchar() != '\n');
        return -1; 
    }
    return option;
}