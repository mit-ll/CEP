/*Convert HEX format(64 32bits array) to ASCII, output to memory*/
void h2a(uint32_t HEX[64], const int SIZE, char **OUTPUT){
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

