/*Convert ASCII text to HEX format(64 32bits array), output to memory*/
void a2h(const char* TEXT, const int SIZE, uint32_t (*OUTPUT)[64]){
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
