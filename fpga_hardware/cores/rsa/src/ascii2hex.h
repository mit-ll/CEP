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

/*Convert ASCII text to HEX format(64 32bits array), output to memory*/
int a2h(const char* TEXT, const int SIZE, uint32_t (*OUTPUT)[64]){
    uint32_t BUF=0x00000000;
    int i=0, y=0;
    
    for(i=0; i<64*4; i++){
        if(i<SIZE) BUF=(BUF<<8)|TEXT[i];
        else       BUF=(BUF<<8)|0x00;
        (*OUTPUT)[y]=BUF;
        if((i+1)%4==0){
            BUF=0x00000000;
            y++;
        }
    }
}
