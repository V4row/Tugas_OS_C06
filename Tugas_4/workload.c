#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define BLOCK_SIZE 4096
#define DEFAULT_FILE_SIZE_MB 32
#define PRNG_SEED 42

typedef enum {
    MODE_SEQ_WRITE,
    MODE_SEQ_READ,
    MODE_RAND_WRITE,
    MODE_RAND_READ
} WorkloadMode;

static int cmp_double(const void *a, const void *b){
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da >db) - (da < db);
}

static double get_percentile(const double *sorted, size_t n, double p){
    if(n ==0) return 0.0;
    size_t idx = (size_t)((p/100.0) * (double)(n-1));
    if (idx >= n) idx = n -1;
    return sorted[idx];
}

static inline double timespec_diff_us(struct timespec start, struct timespec end){
    double sec = (double)(end.tv_sec - start.tv_sec);
    double nsec = (double)(end.tv_nsec - start.tv_nsec);
    return (sec * 1e6) + (nsec /1e3);
}

static int parse_mode(const char *str, WorkloadMode *mode){
    if (strcmp(str, "seq_write") == 0){
        *mode = MODE_SEQ_WRITE;

    } else if (strcmp(str, "seq_read") == 0){
        *mode = MODE_SEQ_READ;
    } else if (strcmp(str, "rand_write") == 0){
        *mode = MODE_RAND_WRITE;
    } else if (strcmp(str, "rand_read") == 0){
        *mode = MODE_RAND_READ; 
    } else {
        return -1;
    }
    return 0;
}

static void write_csv_row(const char *csv_path, const char *mode_str,
                        const char *cache_state, int run_id, size_t file_mb,
                        size_t num_ops, double total_sec, double avg_us,
                        double p50_us, double p95_us, double p99_us,
                        double iops, double throughput_mbs){
    struct stat st;
    int need_header = (stat(csv_path, &st) !=0 || st.st_size == 0);

    FILE *fp = fopen(csv_path, "a");
    if (!fp){
        perror("Gagal membuka file CSV");
        return;
    }

    if(need_header){
        fprintf(fp, "mode,cache_state,run,file_size_mb,block_size_b,ops,"
                "total_time_s,avg_lat_us,p50_lat_us,p95_lat_us,p99_lat_us,"
                "iops,throughput_mb_s\n");
    }

    fprintf(fp, "%s,%s,%d,%zu,%d,%zu,%.6f,%.3f,%.3f,%.3f,%.3f,%.2f,%.2f\n",
            mode_str, cache_state, run_id, file_mb, BLOCK_SIZE, num_ops,
            total_sec, avg_us, p50_us, p95_us, p99_us, iops, throughput_mbs);

    fclose(fp);
}

int main(int argc, char *argv[]){
    if (argc < 5){
        fprintf(stderr,
                "Penggunaan: %s <file_path> <mode> <cache_state> <run_id> [csv_out] [size_mb]\n"
                "   mode        : seq_write | seq_read | rand_write | rand_read\n"
                "   cache_state : cold | warm\n"
                "   run_id      : 1 | 2 | 2\n"
                "   csv_out     : default 'results_workload.csv'\n"
                "   size_mb     : default 32 (minimal 32 MB)\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    const char *file_path = argv[1];
    const char *mode_str = argv[2];
    const char *cache_state = argv[3];
    int run_id = atoi(argv[4]);
    const char *csv_out = (argc >= 6) ? argv[5] : " results_workload.csv"; 
    size_t file_mb = (argc >= 7) ? (size_t)atol(argv[6]) : DEFAULT_FILE_SIZE_MB;

    WorkloadMode mode;
    if (parse_mode(mode_str, &mode) != 0){
        fprintf(stderr, "[ERROR] Mode tidak valid: %s\n", mode_str);
        return EXIT_FAILURE;
    }

    if (file_mb < 32){
        fprintf(stderr, "[WARN] Ukuran file < 32 MB, di-set otomatis ke 32 MB sesuai ketentuan.\n");
        file_mb = 32;
    }

    size_t total_bytes = file_mb * 1024UL * 1024UL;
    size_t num_blocks = total_bytes / BLOCK_SIZE;

    unsigned char *buf = malloc(BLOCK_SIZE);
    double *latencies = malloc(num_blocks * sizeof(double));
    if (!buf || !latencies){
        perror("malloc gagal");
        free(buf);
        free(latencies);
        return EXIT_FAILURE;
    }
    memset(buf, 0xAB, BLOCK_SIZE);

    int flags = 0;
    if (mode == MODE_SEQ_WRITE){
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if(mode == MODE_SEQ_READ){
        flags = O_WRONLY | O_CREAT;
    } else {
        flags = O_RDONLY;
    }

    int fd = open(file_path, flags, 0644);
    if (fd < 0){
        perror("opem gagal (untuk read, pastikan file sudah dibuat dengan seq_write)");
        free(buf);
        free(latencies);
        return EXIT_FAILURE;
    }
    
    
    if(mode == MODE_SEQ_READ || MODE_RAND_READ){
        struct stat st;
        if (fstat(fd, &st) != 0 || (size_t)st.st_size < total_bytes){
            fprintf(stderr, "[ERROR] Ukuran file (%lld B) lebih kecil dari target (%zu B). Jalankan seq_write dulu.\n",
                    (long long)st.st_size, total_bytes);
            close(fd);
            free(buf);
            free(latencies);
            return EXIT_FAILURE;
        }
    }

    srand(PRNG_SEED);

    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);

    double sum_lat_us = 0.0;
    for(size_t i =0; i <num_blocks;i++){
        size_t block_idx = (mode == MODE_RAND_WRITE || mode == MODE_RAND_READ)
                            ? ((size_t)rand() % num_blocks)
                            : i;
        off_t offset = (off_t)(block_idx * BLOCK_SIZE);

        struct timespec op_start, op_end;
        clock_gettime(CLOCK_MONOTONIC, &op_start);

        ssize_t n;
        if (mode == MODE_SEQ_WRITE || mode == MODE_SEQ_READ){
            n = pwrite(fd, buf, BLOCK_SIZE, offset);
        } else {
            n = pread(fd, buf, BLOCK_SIZE, offset);
        }

        clock_gettime(CLOCK_MONOTONIC, &op_end);
        if(n != BLOCK_SIZE){
            fprintf(stderr, "[ERROR] I/O gagal pada blok %zu (offset %lld): %s\n",
                    i, (long long)offset, (n < 0) ? strerror(errno) : "short read/write");
            close(fd);
            free(buf);
            free(latencies);
            return EXIT_FAILURE;
        }
        
        double lat_us = timespec_diff_us(op_start, op_end);
        latencies[i] = lat_us;
        sum_lat_us += lat_us;
        
    }

    clock_gettime(CLOCK_MONOTONIC, &total_end);

    if(mode == MODE_SEQ_WRITE || MODE_RAND_WRITE){
        fsync(fd);
    }
    close(fd);

    double total_sec = timespec_diff_us(total_start, total_end) / 1e6;
    double avg_us = sum_lat_us/ (double)num_blocks;

    qsort(latencies, num_blocks, sizeof(double), cmp_double);
    double p50_us = get_percentile(latencies, num_blocks, 50.0);
    double p95_us = get_percentile(latencies, num_blocks, 95.0);
    double p99_us = get_percentile(latencies, num_blocks, 99.0);

    double iops = (double)num_blocks / total_sec;
    double throughput_mbs = ((double)total_bytes / (1024.0 * 1024.0)) / total_sec;

    printf("[%s | %s | run %d] Time: %.4f s | Avg: %.2f us | P50: %.2f us | P95: %.2f us | P99: %.2f us | IOPS: %.2f | Throughput: %.2f MB/s\n",
            mode_str, cache_state, run_id, total_sec, avg_us, p50_us, p95_us, p99_us, iops, throughput_mbs);
    write_csv_row(csv_out, mode_str, cache_state, run_id, file_mb, num_blocks,
                    total_sec, avg_us, p50_us, p95_us, p99_us, iops, throughput_mbs);
    free(buf);
    free(latencies);
    return EXIT_SUCCESS;
}