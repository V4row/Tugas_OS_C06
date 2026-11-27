#include "banner.h"

#include <stdio.h>
#include <string.h>

#define BANNER_WIDTH 64

static void print_repeat(char ch, size_t count) {
    for (size_t i = 0; i < count; i++) {
        putchar(ch);
    }
    putchar('\n');
}

void print_box_line(const char *text, size_t width) {
    if (!text) text = "";
    size_t len = strlen(text);
    putchar('|');
    putchar(' ');

    if (len >= width - 4) {
        for (size_t i = 0; i < width - 7; i++) {
            putchar(text[i]);
        }
        printf("...");
    } else {
        printf("%s", text);
        for (size_t i = len; i < width - 4; i++) {
            putchar(' ');
        }
    }

    putchar(' ');
    putchar('|');
    putchar('\n');
}

void print_banner(const char *group_id, const char *task_name, const char *module_name) {
    char line1[128];
    char line2[128];

    const char *grp = (group_id && group_id[0] != '\0') ? group_id : "C06";
    const char *tsk = (task_name && task_name[0] != '\0') ? task_name : "Tugas 4 - Filesystem & I/O";
    const char *mod = (module_name && module_name[0] != '\0') ? module_name : "IO Benchmark Suite";

    snprintf(line1, sizeof(line1), "Kelompok : %s", grp);
    snprintf(line2, sizeof(line2), "%s | %s", tsk, mod);

    putchar('+');
    for (int i = 0; i < BANNER_WIDTH - 2; i++) putchar('=');
    puts("+");

    print_box_line(line1, BANNER_WIDTH);
    print_box_line(line2, BANNER_WIDTH);

    putchar('+');
    for (int i = 0; i < BANNER_WIDTH - 2; i++) putchar('=');
    puts("+");
}

void print_section_banner(const char *scenario_title, int run_id, const char *cache_state) {
    char info[160];
    const char *scen = (scenario_title && scenario_title[0] != '\0') ? scenario_title : "Scenario";

    if (cache_state && cache_state[0] != '\0' && run_id > 0) {
        snprintf(info, sizeof(info), "[RUN %d | %s] %s", run_id, cache_state, scen);
    } else if (run_id > 0) {
        snprintf(info, sizeof(info), "[RUN %d] %s", run_id, scen);
    } else {
        snprintf(info, sizeof(info), "%s", scen);
    }

    print_repeat('-', BANNER_WIDTH);
    printf(">> %s\n", info);
    print_repeat('-', BANNER_WIDTH);
}

void print_footer_banner(const char *status_msg, double elapsed_sec) {
    const char *st = (status_msg && status_msg[0] != '\0') ? status_msg : "DONE";
    print_repeat('=', BANNER_WIDTH);
    if (elapsed_sec >= 0.0) {
        printf("Status: %s | Total Elapsed: %.4f s\n", st, elapsed_sec);
    } else {
        printf("Status: %s\n", st);
    }
    print_repeat('=', BANNER_WIDTH);
}