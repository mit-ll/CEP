#include<stdio.h>
#include<stdlib.h>

#include"input.h"
#include"hex2ascii.h"

/*Main*/
int main(void){
    FILE *fread;
    
    //Read Hex input
    fread = fopen("./gen/gTextHEX", "r");
    if (fread == NULL){
        printf("*** ERROR -> Input file not found \"gTextHEX.txt\"\r\n");
        exit(EXIT_FAILURE);
    }
    
    //Convert Hex to ascii, output to file
    h2aF(fread);
    
    fclose(fread);
    
    printf("Completed\n\r");
    exit(EXIT_SUCCESS);
    
    return 0;
}

