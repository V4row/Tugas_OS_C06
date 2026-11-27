#ifndef BANNER_H
#define BANNER_H

#include <stddef.h>

// Mencetak header utama program / modul pengujian
void print_banner(const char *group_id, const char *task_name, const char *module_name);

// Mencetak sub-header untuk setiap skenario atau run pengujian
void print_section_banner(const char *scenario_title, int run_id, const char *cache_state);

// Mencetak garis pemisah / footer ringkasan eksekusi
void print_footer_banner(const char *status_msg, double elapsed_sec);

// Helper opsional: mencetak kotak teks rata tengah/kiri dengan lebar tetap
void print_box_line(const char *text, size_t width);

#endif // BANNER_H