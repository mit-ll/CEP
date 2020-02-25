/*Extract data from file*/
int rFile(char **string, int *size, char *path){
    FILE *Fread;

    Fread = fopen(path, "r");  //open an existing file for reading
    if(Fread == NULL) return 1;//quit if the file does not exist
    fseek(Fread, 0L, SEEK_END);//Get the number of bytes
    *size = ftell(Fread);

    fseek(Fread, 0L, SEEK_SET);//reset the file position indicator to the beginning of the file
    *string = malloc(*size);
    if(*string == NULL) return 1;//memory error
    fread(*string, sizeof(char), *size, Fread);//copy all the text into the string

    fclose(Fread);

    return 0;
}

/*Write Data to File as ASCII*/
int wText(char *string, int size, char *path){
    FILE *fwrite;
    int i;

    fwrite = fopen(path, "w");
    for(i=0; i<size; i++)
        fprintf(fwrite, "%c", string[i]);

    fclose(fwrite);

    return 1;
}

/*Write Data to file as HEX*/
int wHex(unsigned char *hex, int size, char *path){
    FILE *fwrite;
    int i;

    fwrite = fopen(path, "w");
    for(i=0; i<2048/8; i++){
        fprintf(fwrite, "%02x", hex[i]);
        if(i%4==0)fprintf(fwrite, "\r\n");
    }

    fclose(fwrite);

    return 1;
}

/*Print Data*/
void pString(char *string, int size){
    int i;
    printf("Size  : %ld\r\n", size);
    printf("String:\r\n");

    for(i=0; i<size; i++)
        printf("%c", string[i]);

    printf("\r\n");
}

/*Wrote Data to file*/
int wFile(uint32_t DATA[64], int size, char *path){
    FILE *Fwrite;
    int i=0;

    Fwrite = fopen(path, "w");  //open an existing file for writing
    if(Fwrite == NULL) return 1;//quit if the file does not exist

    for(i=0; i<size; i++)
        fprintf(Fwrite, "%08x\r\n", DATA[i]);

    fclose(Fwrite);

    return 0;
}

