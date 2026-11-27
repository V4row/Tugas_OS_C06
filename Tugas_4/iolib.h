#ifndef IOLIB_H
#define IOLIB_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

// Struct untuk menyimpan metrik /proc/meminfo & /sys/block/loopX/stat
typedef struct {
    long dirty_kb;
    long writeback_kb;
    unsigned long long write_ios;
    unsigned long long write_merges;
    unsigned long long write_sectors;
} IoOsMetrics;

// Struct untuk hasil perhitungan statistik
typedef struct {
    double avg;
    double min;
    double max;
    double p50;
    double p95;
    double p99;
} IoStats;

// 1. Pengukur waktu (nanodetik menggunakan CLOCK_MONOTONIC)
uint64_t io_now_ns(void);
uint64_t io_diff_ns(uint64_t start_ns, uint64_t end_ns);

// 2. Pembaca metrik OS
// loop_dev bisa diisi nama device seperti "loop0" (atau NULL untuk auto-detect loop device aktif)
int io_read_meminfo(long *dirty_kb, long *writeback_kb);
int io_read_block_stat(const char *loop_dev, unsigned long long *write_ios,
                       unsigned long long *write_merges, unsigned long long *write_sectors);
int io_capture_os_metrics(const char *loop_dev, IoOsMetrics *out);

// 3. Perhitungan statistik (mengurutkan array samples secara in-place)
IoStats io_calc_stats(double *samples, size_t count);

// 4. Penulis baris CSV ke folder logs/
// Format file: logs/<group_id>-<program>-<scenario>-<timestamp>.csv
// Jika header != NULL dan file baru dibuat, header akan ditulis otomatis di baris pertama.
int io_write_csv(const char *group_id, const char *program, const char *scenario,
                 const char *timestamp, const char *header, const char *row_fmt, ...);

// 5. Penyimpan spesifikasi OS ke logs/<group_id>-env-<timestamp>.txt
// Otomatis membuat folder logs/ jika belum ada.
int io_save_env_spec(const char *group_id, const char *timestamp);

// Helper tambahan: menghasilkan timestamp format YYYYMMDD_HHMMSS
void io_get_timestamp(char *buf, size_t len);

#endif // IOLIB_H