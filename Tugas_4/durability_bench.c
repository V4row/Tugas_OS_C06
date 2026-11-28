#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "iolib.h"

char logLine[] = "2026-11-25T13:01:00Z [INFO] auth_service: User login success for uid=48291 from ip=192.168.1.42 (session_id=9f8a7b6c5d4e3f2a).\n";
uint64_t start;
uint64_t end;

double countLatency(long start_s, long end_s, long start_ns, long end_ns) {
    return (end_s * 1e9 - start_s * 1e9) + (end_ns - start_ns);
}

void write_no_sync() {
    size_t strLen = strlen(logLine);
    double *dataA = malloc(10000 * sizeof(double));
    double *dataO = malloc(10000 * sizeof(double));

    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        write(fd, logLine, strLen);
        // =================
        end = io_now_ns();
        dataA[i] = io_diff_ns(start, end);
    }
    close(fd);

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        pwrite(fd2, logLine, strLen, 0);
        // =================
        end = io_now_ns();
        dataO[i] = io_diff_ns(start, end);
    }
    close(fd2);
    sync();

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "no_sync_A", "no,latency", "%d,%.0f", i+1, dataA[i]);
    }
    IoStats statA = io_calc_stats(dataA, 10000);
    printf("Latency no sync (A): %.0f ns\n", statA.sum);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "no_sync_O", "no,latency", "%d,%.0f", i+1, dataO[i]);
    }
    IoStats statO = io_calc_stats(dataO, 10000);
    printf("Latency no sync (O): %.0f ns\n", statO.sum);

    free(dataA);
    free(dataO);
}

void write_sync_file_range() {
    size_t strLen = strlen(logLine);
    double *dataA = malloc(10000 * sizeof(double));
    double *dataO = malloc(10000 * sizeof(double));

    // Offset tracker
    int offset = 0;
    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        ssize_t writeEnd = write(fd, logLine, strLen);
        sync_file_range(fd, offset, writeEnd, SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
        // =================
        end = io_now_ns();
        dataA[i] = io_diff_ns(start, end);
        offset += writeEnd;
    }
    close(fd);
    sync();

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        ssize_t writeEnd = pwrite(fd2, logLine, strLen, 0);
        sync_file_range(fd2, 0, writeEnd, SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
        // =================
        end = io_now_ns();
        dataO[i] = io_diff_ns(start, end);
    }
    close(fd2);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "sync_file_range_A", "no,latency", "%d,%.0f", i+1, dataA[i]);
    }
    IoStats statA = io_calc_stats(dataA, 10000);
    printf("Latency w sync_file_range (A): %.0f ns\n", statA.sum);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "sync_file_range_O", "no,latency", "%d,%.0f", i+1, dataO[i]);
    }
    IoStats statO = io_calc_stats(dataO, 10000);
    printf("Latency w sync_file_range (O): %.0f ns\n", statO.sum);

    free(dataA);
    free(dataO);
}

void write_fdatasync() {
    size_t strLen = strlen(logLine);
    double *dataA = malloc(10000 * sizeof(double));
    double *dataO = malloc(10000 * sizeof(double));

    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        write(fd, logLine, strLen);
        fdatasync(fd);
        // =================
        end = io_now_ns();
        dataA[i] = io_diff_ns(start, end);
    }
    close(fd);
    sync();

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        pwrite(fd2, logLine, strLen, 0);
        fdatasync(fd2);
        // =================
        end = io_now_ns();
        dataO[i] = io_diff_ns(start, end);
    }
    close(fd2);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "fdatasync_A", "no,latency", "%d,%.0f", i+1, dataA[i]);
    }
    IoStats statA = io_calc_stats(dataA, 10000);
    printf("Latency w fdatasync (A): %.0f ns\n", statA.sum);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "fdatasync_O", "no,latency", "%d,%.0f", i+1, dataO[i]);
    }
    IoStats statO = io_calc_stats(dataO, 10000);
    printf("Latency w fdatasync (O): %.0f ns\n", statO.sum);

    free(dataA);
    free(dataO);
}

void write_fsync() {
    size_t strLen = strlen(logLine);
    double *dataA = malloc(10000 * sizeof(double));
    double *dataO = malloc(10000 * sizeof(double));

    // Case append
    int fd = open("output_append.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        write(fd, logLine, strLen);
        fsync(fd);
        // =================
        end = io_now_ns();
        dataA[i] = io_diff_ns(start, end);
    }
    close(fd);
    sync();

    // Case overwrite
    int fd2 = open("output_overwrite.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    for (int i = 0; i < 10000; i++) {
        start = io_now_ns();
        // === Transaksi ===
        pwrite(fd2, logLine, strLen, 0);
        fsync(fd2);
        // =================
        end = io_now_ns();
        dataO[i] = io_diff_ns(start, end);
    }
    close(fd2);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "fsync_A", "no,latency", "%d,%.0f", i+1, dataA[i]);
    }
    IoStats statA = io_calc_stats(dataA, 10000);
    printf("Latency w fsync (A): %.0f ns\n", statA.sum);

    for (int i = 0; i < 10000; i++) {
        io_write_csv("durability_bench", "fsync_O", "no,latency", "%d,%.0f", i+1, dataO[i]);
    }
    IoStats statO = io_calc_stats(dataO, 10000);
    printf("Latency w fsync (O): %.0f ns\n", statO.sum);

    free(dataA);
    free(dataO);
}

int main() {
    printf("Results\n");
    write_no_sync();
    sync();
    write_sync_file_range();
    sync();
    write_fdatasync();
    sync();
    write_fsync();
    return 0;
}