#include<stdio.h>
#include<stdlib.h>

/*Convert ASCII text to HEX format(64 32bits array), output to file*/ 
int a2hF(FILE *fread){ 
    FILE *fwrite; 
    int i=0, c=0; 

    fwrite = fopen("./gen/pTextHEX", "w"); 

    for(i=0; i<(2048/8); i++){ 
        if((c = getc(fread)) != EOF)fprintf(fwrite, "%02x", c); 
        else                        fprintf(fwrite, "%02x", 0); 

        if((i+1)%4==0)              fprintf(fwrite, "\r\n"); 
    } 

    fclose(fwrite); 
    return 1; 
} 

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

