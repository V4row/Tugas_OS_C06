#include <stdio.h>
#include <stdlib.h>

int main() {
    float mem_usage;
    float load_average;
    int core_count;

    if (scanf("%f %f %d", &mem_usage, &load_average, &core_count) != 3) {
        return 1;
    }


    if (mem_usage >= 90) {
        printf("FAIL ");
    } else if (mem_usage >= 75) {
        printf("WARN ");
    } else {
        printf("PASS ");
    }

    if (load_average > (2.0 * core_count)) {
        printf("FAIL\n");
    } else if (load_average > (1.0 * core_count)) {
        printf("WARN\n");
    } else {
        printf("PASS\n");
    }

    return 0;
}