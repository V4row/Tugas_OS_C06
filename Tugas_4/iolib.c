#define _POSIX_C_SOURCE 200809L
#include "iolib.h"

#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Helper internal untuk memastikan direktori logs/ tersedia
static void ensure_logs_dir(void) {
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0755);
    }
}

void io_get_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (tm_info) {
        strftime(buf, len, "%Y%m%d_%H%M%S", tm_info);
    } else if (len > 0) {
        snprintf(buf, len, "00000000_000000");
    }
}

// ============================================================================
// 1. Pengukur Waktu (Nanodetik)
// ============================================================================

uint64_t io_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

uint64_t io_diff_ns(uint64_t start_ns, uint64_t end_ns) {
    return (end_ns >= start_ns) ? (end_ns - start_ns) : 0;
}

// ============================================================================
// 2. Pembaca Metrik OS
// ============================================================================

int io_read_meminfo(long *dirty_kb, long *writeback_kb) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    char line[256];
    int found = 0;
    if (dirty_kb) *dirty_kb = -1;
    if (writeback_kb) *writeback_kb = -1;

    while (fgets(line, sizeof(line), fp)) {
        if (dirty_kb && strncmp(line, "Dirty:", 6) == 0) {
            sscanf(line + 6, "%ld", dirty_kb);
            found++;
        } else if (writeback_kb && strncmp(line, "Writeback:", 10) == 0) {
            sscanf(line + 10, "%ld", writeback_kb);
            found++;
        }
        if (found >= 2) break;
    }

    fclose(fp);
    return 0;
}

// Auto-detect loop device yang sedang ter-mount di /mnt/ext4_lab atau loop device aktif pertama
static void detect_loop_device(char *out_dev, size_t len) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp) {
        char dev[128], mnt[256];
        while (fscanf(fp, "%127s %255s %*[^\n]\n", dev, mnt) == 2) {
            if (strncmp(dev, "/dev/loop", 9) == 0 && strcmp(mnt, "/mnt/ext4_lab") == 0) {
                snprintf(out_dev, len, "%s", dev + 5); // ambil "loopX"
                fclose(fp);
                return;
            }
        }
        rewind(fp);
        while (fscanf(fp, "%127s %255s %*[^\n]\n", dev, mnt) == 2) {
            if (strncmp(dev, "/dev/loop", 9) == 0) {
                snprintf(out_dev, len, "%s", dev + 5);
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }
    snprintf(out_dev, len, "loop0"); // fallback default
}

int io_read_block_stat(const char *loop_dev, unsigned long long *write_ios,
                       unsigned long long *write_merges, unsigned long long *write_sectors) {
    char dev_name[64];
    if (loop_dev && loop_dev[0] != '\0') {
        snprintf(dev_name, sizeof(dev_name), "%s", loop_dev);
    } else {
        detect_loop_device(dev_name, sizeof(dev_name));
    }

    char path[256];
    snprintf(path, sizeof(path), "/sys/block/%s/stat", dev_name);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        // Jika fallback (WSL2/container tanpa loop device), set ke 0 agar tidak crash
        if (write_ios) *write_ios = 0;
        if (write_merges) *write_merges = 0;
        if (write_sectors) *write_sectors = 0;
        return -1;
    }

    // Format kolom /sys/block/<dev>/stat (kernel Linux):
    // 1: read I/Os, 2: read merges, 3: read sectors, 4: read ticks,
    // 5: write I/Os, 6: write merges, 7: write sectors, ...
    unsigned long long r_ios, r_merges, r_sec, r_ticks;
    unsigned long long w_ios = 0, w_merges = 0, w_sec = 0;

    int matched = fscanf(fp, "%llu %llu %llu %llu %llu %llu %llu",
                         &r_ios, &r_merges, &r_sec, &r_ticks,
                         &w_ios, &w_merges, &w_sec);
    fclose(fp);

    if (matched < 7) return -1;

    if (write_ios) *write_ios = w_ios;
    if (write_merges) *write_merges = w_merges;
    if (write_sectors) *write_sectors = w_sec;
    return 0;
}

int io_capture_os_metrics(const char *loop_dev, IoOsMetrics *out) {
    if (!out) return -1;
    int r1 = io_read_meminfo(&out->dirty_kb, &out->writeback_kb);
    int r2 = io_read_block_stat(loop_dev, &out->write_ios, &out->write_merges, &out->write_sectors);
    return (r1 == 0 && r2 == 0) ? 0 : -1;
}

// ============================================================================
// 3. Perhitungan Statistik
// ============================================================================

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

static double percentile_sorted(const double *sorted, size_t n, double p) {
    if (n == 0) return 0.0;
    size_t idx = (size_t)((p / 100.0) * (double)(n - 1));
    if (idx >= n) idx = n - 1;
    return sorted[idx];
}

IoStats io_calc_stats(double *samples, size_t count) {
    IoStats st = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    if (!samples || count == 0) return st;

    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += samples[i];
    }

    qsort(samples, count, sizeof(double), cmp_double);

    st.avg = sum / (double)count;
    st.min = samples[0];
    st.max = samples[count - 1];
    st.p50 = percentile_sorted(samples, count, 50.0);
    st.p95 = percentile_sorted(samples, count, 95.0);
    st.p99 = percentile_sorted(samples, count, 99.0);
    return st;
}

// ============================================================================
// 4. Penulis Baris CSV
// ============================================================================

int io_write_csv(const char *group_id, const char *program, const char *scenario,
                 const char *timestamp, const char *header, const char *row_fmt, ...) {
    ensure_logs_dir();

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "logs/%s-%s-%s-%s.csv",
             group_id, program, scenario, timestamp);

    struct stat st;
    int need_header = (stat(filepath, &st) != 0 || st.st_size == 0);

    FILE *fp = fopen(filepath, "a");
    if (!fp) {
        perror("Gagal membuka file CSV");
        return -1;
    }

    if (need_header && header && header[0] != '\0') {
        fprintf(fp, "%s\n", header);
    }

    va_list args;
    va_start(args, row_fmt);
    vfprintf(fp, row_fmt, args);
    va_end(args);
    fprintf(fp, "\n");

    fclose(fp);
    return 0;
}

// ============================================================================
// 5. Penyimpan Spesifikasi OS
// ============================================================================

static void append_cmd_output(FILE *out, const char *label, const char *cmd) {
    fprintf(out, "=== %s ===\n", label);
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        fprintf(out, "(gagal menjalankan perintah: %s)\n\n", cmd);
        return;
    }
    char buf[512];
    while (fgets(buf, sizeof(buf), pipe)) {
        fputs(buf, out);
    }
    pclose(pipe);
    fprintf(out, "\n");
}

int io_save_env_spec(const char *group_id, const char *timestamp) {
    ensure_logs_dir();

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "logs/%s-env-%s.txt", group_id, timestamp);

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        perror("Gagal membuat file env spec");
        return -1;
    }

    fprintf(fp, "Timestamp: %s\nGroup ID : %s\n\n", timestamp, group_id);

    append_cmd_output(fp, "Kernel Release (uname -r)", "uname -r");
    append_cmd_output(fp, "OS Info (/etc/os-release)", "cat /etc/os-release 2>/dev/null || uname -a");
    append_cmd_output(fp, "CPU Info", "lscpu 2>/dev/null | grep -E 'Model name|CPU\\(s\\)|Architecture' || grep -m1 'model name' /proc/cpuinfo");
    append_cmd_output(fp, "Memory Info (/proc/meminfo)", "head -n 5 /proc/meminfo");
    append_cmd_output(fp, "Block Devices (lsblk)", "lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT");
    append_cmd_output(fp, "Mount Info (/mnt/ext4_lab)", "df -Th /mnt/ext4_lab 2>/dev/null || df -Th .");
    append_cmd_output(fp, "VM Dirty Parameters (sysctl)", "sysctl vm.dirty_ratio vm.dirty_background_ratio vm.dirty_expire_centisecs vm.dirty_writeback_centisecs 2>/dev/null");

    fclose(fp);
    return 0;
}