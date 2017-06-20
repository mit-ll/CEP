#include<stdio.h>
#include<stdlib.h>

#include"input.h"
#include"ascii2hex.h"

/*Main*/
int main(void){
    FILE *FIN;
    int i=0, c=0;

    //Read Input ascii
    FIN = fopen("./gen/pText.txt", "r");
    if (FIN == NULL){
        printf("*** ERROR -> Input file not found \"pText.txt\"\r\n");
        exit(EXIT_FAILURE);
    }
    
    //Convert ascii to hex, output to file
    a2hF(FIN);
   
    fclose(FIN);

    printf("Completed\n\r");
    exit(EXIT_SUCCESS);
    return 0;
}

