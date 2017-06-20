#include<stdio.h>
#include<stdlib.h>

/*Convert HEX format(64 32bits array) to ASCII, output to file*/
int h2aF(FILE *fread){
    FILE *fwrite;
    int i=0, c=0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char buf[2];

    fwrite = fopen("./gen/gText.txt", "w");

    while ((read = getline(&line, &len, fread)) != -1) {
        for(i = 0; i < read-2; i++){
            buf[1]=line[i];
            if(i % 2 != 0 & !(buf[0]=='0' & buf[1]=='0')) fprintf(fwrite, "%c", strtol(buf, NULL, 16));
            else                                          buf[0] = line[i];
        }
    }

    fclose(fwrite);
    return 1;
}

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

