#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


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