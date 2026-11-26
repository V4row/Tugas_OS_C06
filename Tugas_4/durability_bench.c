#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char logLine[] = "2026-11-25T13:01:00Z [INFO] auth_service: User login success for uid=48291 from ip=192.168.1.42 (session_id=9f8a7b6c5d4e3f2a).\n";
struct timespec start;
struct timespec end;

double countLatency(long start_s, long end_s, long start_ns, long end_ns) {
    return (end_s * 1e9 - start_s * 1e9) + (end_ns - start_ns);
}

void write_no_sync() {
    size_t strLen = strlen(logLine);

    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        write(fd, logLine, strLen);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd);
    printf("Latency no sync (A): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        pwrite(fd2, logLine, strLen, 0);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd2);
    printf("Latency no sync (O): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));
}

void write_sync_file_range() {
    size_t strLen = strlen(logLine);

    // Offset tracker
    int offset = 0;
    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        ssize_t writeEnd = write(fd, logLine, strLen);
        sync_file_range(fd, offset, writeEnd, SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
        offset += writeEnd;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd);
    printf("Latency w sync_file_range (A): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        ssize_t writeEnd = pwrite(fd2, logLine, strLen, 0);
        sync_file_range(fd2, 0, writeEnd, SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd2);
    printf("Latency w sync_file_range (O): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));
}

void write_fdatasync() {
    size_t strLen = strlen(logLine);

    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        write(fd, logLine, strLen);
        fdatasync(fd);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd);
    printf("Latency w fdatasync (A): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        pwrite(fd2, logLine, strLen, 0);
        fdatasync(fd2);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd2);
    printf("Latency w fdatasync (O): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));
}

void write_fsync() {
    size_t strLen = strlen(logLine);

    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        write(fd, logLine, strLen);
        fsync(fd);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd);
    printf("Latency w fsync (A): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10000; i++) {
        pwrite(fd2, logLine, strLen, 0);
        fsync(fd2);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd2);
    printf("Latency w fsync (O): %.0f ns\n", countLatency(start.tv_sec, end.tv_sec, start.tv_nsec, end.tv_nsec));
}

int main() {
    printf("Results\n");
    write_no_sync();
    write_sync_file_range();
    write_fdatasync();
    write_fsync();
    return 0;
}