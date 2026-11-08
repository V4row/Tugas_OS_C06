#include <stdio.h>
#include <string.h>
#include <fcntl.h>   // Untuk system call: open()
#include <unistd.h>  // Untuk system call: write(), close()
#include "logger.h"

#define LOG_FILE_STD "log_std.txt"
#define LOG_FILE_SYSCALL "log_syscall.txt"

// Implementasi menggunakan Standard C Library
void write_log_std(const char* message) {
    // Mode "a" (append) membuka file untuk ditulis di akhir file. 
    // Jika file belum ada, file baru akan dibuat otomatis.
    FILE *file = fopen(LOG_FILE_STD, "a");
    if (file == NULL) {
        perror("Gagal membuka log_std.txt");
        return;
    }
    
    fprintf(file, "%s\n", message);
    fclose(file);
}

// Implementasi menggunakan System Call
void write_log_syscall(const char* message) {
    // O_WRONLY : Buka dalam mode Write Only
    // O_CREAT  : Buat file jika belum ada
    // O_APPEND : Tulis di akhir file
    // 0644     : Permission file (rw-r--r--)
    int fd = open(LOG_FILE_SYSCALL, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Gagal membuka log_syscall.txt");
        return;
    }

    // Menulis isi pesan log
    write(fd, message, strlen(message));
    
    // Menulis karakter newline (\n) agar log berikutnya berada di baris baru
    write(fd, "\n", 1);
    
    close(fd);
}