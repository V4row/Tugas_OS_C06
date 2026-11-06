#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "malloc_free.h"


// ====== Tempat buat fungsi ======


// ====== Tempat buat fungsi ======

int main(){
    int option;
    bool running = true;
    scanf("%d",&option);

    while(running){

        switch (option)
        {
        case 1:
            /* allocate malloc */
            break;

        case 2:
            /* allocate mmap */
            int allocation_ammount;
            printf("You picked malloc/free\n");
            // TODO: print VmSize & VmRSS
            printf("How many integers to allocate: ");
            scanf("%d", &allocation_ammount);
            printf("\n");
            print_malloc_free_output(allocation_ammount);
            // TODO: print VmSize & VmRSS
            running = false;
            break;

        case 3:
            /* varian 6 */
            break;

        case 4:
            running = false;
            break;
        
        default:
            printf("Tdak valid!");
            break;
        }
    }
    return 0;
}