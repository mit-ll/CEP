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

/*Convert HEX format(64 32bits array) to ASCII, output to memory*/
int h2a(uint32_t HEX[64], const int SIZE, char **OUTPUT){
    uint32_t BUF=HEX[0];
    int i=0, y=0;

    for(i=0; i<SIZE; i++){
        switch((i+1)%4){
            case 1:BUF=HEX[y]>>24; break;
            case 2:BUF=HEX[y]>>16; break;
            case 3:BUF=HEX[y]>>8; break;
            case 0:BUF=HEX[y]; y++; break;
        }
        (*OUTPUT)[i]=BUF;
    }
}

