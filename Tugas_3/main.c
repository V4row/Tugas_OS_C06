#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define BUFFER_SIZE 4096
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define GROUP_ID "C6"


// ====== Tempat buat fungsi ======
long get_vmsize_kb() {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return -1;
    
    char line[128];
    long vmsize = 0;
    while (fgets(line, sizeof(line), file)) {
        // Mencari string "VmSize:"
        if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%ld", &vmsize);
            break;
        }
    }
    fclose(file);
    return vmsize;
}

long get_vmrss_kb() {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return -1;
    
    // Mencari baris VmRSS lalu mengambil nilainya
    char line[128];
    long vmrss = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &vmrss);
            break;
        }
    }
    fclose(file);
    return vmrss;
}

void generate_log_filenames(const char *metode, char *std_name, char *sys_name, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Format: C6-[metode]_std-DD-MM-YYYY_HH:MM:SS.txt
    snprintf(std_name, size, "%s-%s_std-%02d-%02d-%d_%02d:%02d:%02d.txt",
             GROUP_ID, metode, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec);
             
    snprintf(sys_name, size, "%s-%s_sys-%02d-%02d-%d_%02d:%02d:%02d.txt",
             GROUP_ID, metode, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec);
}

void write_log_std(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w+");
    if (!file) {
        perror("[ERROR] Gagal membuat log std");
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
}

void write_log_sys(const char *filename, const char *content) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("[ERROR] Gagal membuat log sys");
        return;
    }
    write(fd, content, strlen(content));
    close(fd);
}

void copy_std_to_sys(char *stdLib, char *sysCall){
    FILE *file = fopen(stdLib, "r");
    char buffer[BUFFER_SIZE];
    int fileSys = open(sysCall, O_WRONLY | O_CREAT, 0666);
    size_t bytes_read;
    ssize_t bytes_written;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        bytes_written = write(fileSys, buffer, bytes_read);
    }
    fclose(file);
    close(fileSys);
}

// ===== Velicia ====
void print_malloc_free_output(int numOfInts) {
    // Catat VmSize dan VmRSS sebelum allocate memori
    long vmSizeBefore = get_vmsize_kb();
    long vmRssBefore = get_vmrss_kb();

    // Allocate ke memori dengan malloc
    int *integers = malloc(numOfInts * sizeof(int));

    if (integers == NULL) { // Jika gagal allocate memori
        printf("Gagal melakukan alokasi memori dengan malloc.\n");
        return; // Fungsi berhenti
    }

    // Membuat nama file dengan timestamp
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

    // Print VmSize & VmRSS ke terminal dan file log
    printf("VmSize Before: %ld KB | VmRSS Before: %ld KB\n\n", vmSizeBefore, vmRssBefore);
    fprintf(file, "VmSize Before: %ld KB | VmRSS Before: %ld KB\n\n", vmSizeBefore, vmRssBefore);

    // Isi nilai-nilai integer
    for (int i = 1; i <= numOfInts; i++) {
        integers[numOfInts-i] = numOfInts - (i-1);
    }

    // Print starting address dan maksimum 10 integer terakhir
    int showAmmount = MIN(10, numOfInts);
    printf("Starting memory address: %p\n", integers);
    fprintf(file, "Starting memory address: %p\n", integers);
    printf("Printing first %d values in reverse order: \n", showAmmount);
    fprintf(file, "Printing first %d values in reverse order: \n", showAmmount);
    for (int i = 1; i <= showAmmount; i++) {
        printf("%d\n", integers[numOfInts-i]);
        fprintf(file, "%d\n", integers[numOfInts-i]);
    }

    // Catat VmSize dan VmRSS setelah allocate memori
    long vmSizeAfter = get_vmsize_kb();
    long vmRssAfter = get_vmrss_kb();

    // Print VmSize dan VmRSS ke terminal dan file log
    printf("\nVmSize After: %ld KB | VmRSS After: %ld KB\n\n", vmSizeAfter, vmRssAfter);
    fprintf(file, "\nVmSize After: %ld KB | VmRSS After: %ld KB\n", vmSizeAfter, vmRssAfter);

    // Free ruang memori yang dialokasikan dan menutup file log
    free(integers);
    fclose(file);

    // Menyalin isi file C6-malloc_std-[timestamp].txt ke file C6-malloc_sys-[timestamp].txt
    copy_std_to_sys(stdLibName, sysCallName);
}


// ===== Vebian =====

int in_heap(void *addr){
    FILE *file = fopen("/proc/self/maps", "r");
    if (!file){
        return -1;
    }

    int result = 0;
    char line[128];
    unsigned long target = (unsigned long)addr;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "[heap]")) {
            unsigned long start, end;
            if (sscanf(line, "%lx-%lx", &start, &end) == 2 && target >= start && target < end){
                result = 1; // true karena ada didalam rentang
            }
        }
    }
    // tetap 0/false jika tidak didalam rentang heap (tidak pernah masuk ifnya)
    fclose(file);
    return result;
}

void varian_6(){
    const size_t KB = 1024;
    size_t threshold = 128;
    size_t sizes_kb[] = {1,2,4,8, 16, 32, 64, 96, 120, 127, 128, 129, 256, 512, 1024};
    int len = sizeof(sizes_kb) / sizeof(sizes_kb[0]);
    void *pointers[len];
    long first_mmap_used = -1;

    if (mallopt(M_MMAP_THRESHOLD, (int)(threshold * KB)) == 0) {
        perror("mallopt");
        return;
    }

    for (int i = 0; i < len; i++) {
        char *brk_before = (char *)sbrk(0);
        pointers[i] = malloc(sizes_kb[i] * KB);

        if (pointers[i] == NULL) {
            printf("%-11zuKB Alokasi gagal\n", sizes_kb[i]);

            continue;
        }
        char *brk_after = (char *)sbrk(0);
        long delta_brk = brk_after - brk_before;

        int is_heap = in_heap(pointers[i]);
        if (is_heap == -1){
            printf("%-11zuKB %-+10ld %-16p %-16p %s\n",
               sizes_kb[i], delta_brk, pointers[i], (void *)brk_after,
               "gagal menkonfirmasi");
        }
        else{
            if (!is_heap && first_mmap_used < 0){
                first_mmap_used = (long) sizes_kb[i];
            }

            printf("%-11zuKB %-+10ld %-16p %-16p %s\n",
               sizes_kb[i], delta_brk, pointers[i], (void *)brk_after,
               is_heap ? "heap" : "mmap");
        }
    }

    if (first_mmap_used >= 0)
    {
        printf("\n[HASIL] malloc mulai beralih ke mmap pada ukuran %ld KB (threshold %zu KB)\n\n",first_mmap_used, threshold);
    }
    else {
        printf("[HASIL] malloc tidak pernah beralih ke mmap\n\n");
    }

    for (int i = 0; i < len;i++){
        free(pointers[i]);
    }
    // fclose(file);
    // copy_std_to_sys(stdLibName,sysCallName);
}

void varian_6_with_log(){
    // timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char stdLibName[128];
    char sysCallName[128];
    strftime(stdLibName, sizeof(stdLibName), "C6-varian_6-mmap_threshold_std-%d-%m-%Y_%H:%M:%S.txt", t);
    strftime(sysCallName, sizeof(sysCallName), "C6-varian_6-mmap_threshold_sys-%d-%m-%Y_%H:%M:%S.txt", t);

    FILE *file = fopen(stdLibName, "w+");
    fprintf(file,"--- Varian 6: mmap Threshold ---\n");
    fprintf(file,"M_MMAP_THRESHOLD = 128 KB\n\n");
    fprintf(file,"Ukuran     Delta brk     Alamat           Ujung brk        Hasil\n");
    
    const size_t KB = 1024;
    size_t threshold = 128;
    size_t sizes_kb[] = {1,2,4,8, 16, 32, 64, 96, 120, 127, 128, 129, 256, 512, 1024};
    int len = sizeof(sizes_kb) / sizeof(sizes_kb[0]);
    void *pointers[len];
    long first_mmap_used = -1;

    if (mallopt(M_MMAP_THRESHOLD, (int)(threshold * KB)) == 0) {
        perror("mallopt");
        return;
    }

    for (int i = 0; i < len; i++) {
        char *brk_before = (char *)sbrk(0);
        pointers[i] = malloc(sizes_kb[i] * KB);

        if (pointers[i] == NULL) {
            printf("%-11zuKB Alokasi gagal\n", sizes_kb[i]);
            fprintf(file, "%-11zuKB Alokasi gagal\n", sizes_kb[i]);
            continue;
        }
        char *brk_after = (char *)sbrk(0);
        long delta_brk = brk_after - brk_before;

        int is_heap = in_heap(pointers[i]);
        if (is_heap == -1){
            printf("%-11zuKB %-+10ld %-16p %-16p %s\n",
               sizes_kb[i], delta_brk, pointers[i], (void *)brk_after,
               "gagal menkonfirmasi");
            fprintf(file,"%-11zuKB %-+10ld %-16p %-16p %s\n",
               sizes_kb[i], delta_brk, pointers[i], (void *)brk_after,
               "gagal menkonfirmasi");
        }
        else{
            if (!is_heap && first_mmap_used < 0){
                first_mmap_used = (long) sizes_kb[i];
            }

            printf("%-11zuKB %-+10ld %-16p %-16p %s\n",
               sizes_kb[i], delta_brk, pointers[i], (void *)brk_after,
               is_heap ? "heap" : "mmap");
            fprintf(file, "%-11zuKB %-+10ld %-16p %-16p %s\n",
               sizes_kb[i], delta_brk, pointers[i], (void *)brk_after,
               is_heap ? "heap" : "mmap");
        }
    }

    if (first_mmap_used >= 0)
    {
        printf("\n[HASIL] malloc mulai beralih ke mmap pada ukuran %ld KB (threshold %zu KB)\n\n",first_mmap_used, threshold);
        fprintf(file,"\n[HASIL] malloc mulai beralih ke mmap pada ukuran %ld KB (threshold %zu KB)\n\n",first_mmap_used, threshold);
    }
    else {
        printf("[HASIL] malloc tidak pernah beralih ke mmap\n\n");
        fprintf(file,"\n[HASIL] malloc tidak pernah beralih ke mmap\n\n");
    }

    for (int i = 0; i < len;i++){
        free(pointers[i]);
    }
    fclose(file);
    copy_std_to_sys(stdLibName,sysCallName);
}


// ====== Tempat buat fungsi ======

int main(){
    int option;
    bool running = true;
    
    while(running){
        // ganti opsinya nanti
        print_banner();
        option = get_option();

        switch (option)
        {
        case 1:
            /* allocate malloc */
            int allocation_ammount;
            printf("You picked malloc/free\n");
            printf("How many integers to allocate: ");
            scanf("%d", &allocation_ammount);
            printf("\n");
            print_malloc_free_output(allocation_ammount);
            // running = false;
            break;
            
        case 2:
            /* allocate mmap */
            break;

        case 3:
            /* varian 6 */
            int log_option;
            printf("\nIngin mencatat log?");
            printf("\n1. Tidak perlu");
            printf("\n2. Lakukan pencatatan");
            printf("\n(Keluaran mungkin saja berbeda)");
            printf("\nMasukkan pilihan (ketik '1' untuk malloc/free): ");
            if (scanf("%d",&log_option) !=1){
                return 1;
            }
            switch (log_option)
            {
            case 1:
                /* code */
                printf("\n--- Varian 6: mmap Threshold ---\n");
                printf("M_MMAP_THRESHOLD = 128 KB\n\n");
                printf("Ukuran     Delta brk     Alamat           Ujung brk        Hasil\n");
                varian_6();
                break;
                
                case 2:
                /* code */
                printf("\n--- Varian 6: mmap Threshold ---\n");
                printf("M_MMAP_THRESHOLD = 128 KB\n\n");
                printf("Ukuran     Delta brk     Alamat           Ujung brk        Hasil\n");
                varian_6_with_log();
                break;
            
            default:
                printf("Pilihan tidak valid!\n");
                break;
            }
            break;

        case 4:
            running = false;
            printf("Keluar!\n");
            break;
        
        default:
            printf("Tidak valid!");
        }
    }
    return 0;
}