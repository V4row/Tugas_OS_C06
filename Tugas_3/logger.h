#ifndef LOGGER_H
#define LOGGER_H

// Menulis log menggunakan fungsi Standard C Library (stdio.h)
void write_log_std(const char* message);

// Menulis log menggunakan System Call POSIX (unistd.h, fcntl.h)
void write_log_syscall(const char* message);

#endif