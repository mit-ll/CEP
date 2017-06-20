/* ---- Base64 Encoding/Decoding Table --- */
unsigned char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void decodeblock(unsigned char in[], unsigned char *out, int i) {
    out[i+0] = in[0] << 2 | in[1] >> 4;
    out[i+1] = in[1] << 4 | in[2] >> 2;
    out[i+2] = in[2] << 6 | in[3] >> 0;
}

/*Update base64 buffer and convert to Hex buffer */
int read(int *shift,  unsigned char *in,  unsigned char *out, const char* BASE64, int *SCUR, int SMAX){
    unsigned char b64src[3];
    unsigned char c;
    int i=0, y=0, t=0;

    //Update Buffer Pointers
    if(*shift > 5){
        i=0;
        *shift=0;
    }else if(*shift>2 && *shift<6){
        out[0] = out[3];
        out[1] = out[4];
        out[2] = out[5];
        *shift=*shift-3;
        in[0] = in[4];
        in[1] = in[5];
        in[2] = in[6];
        in[3] = in[7];
        i=4;
    } else
        return 1;

    //Load new data into base64 buffer and update hex buffer
    for(; i<8 && i<SMAX; i++){
        c=BASE64[*SCUR];*SCUR=*SCUR+1;
        if(c=='\n'){c=BASE64[*SCUR];*SCUR=*SCUR+1;}
        in[i] = c;

        for(t=0;((unsigned) t<sizeof(b64))&(c!=b64[t]);t++);

        b64src[y++] = t;
        if(y == 4) {
            y=0;
            decodeblock(b64src, out, ((i/4)*3));
        }

        //if(c == '=') return 0;
    }

    return 1;
}

/*Write extracted values to memory and display*/
int write(int *shift, int *length, unsigned char *in, unsigned char *out, const char* BASE64, int *SCUR, int SMAX, const char* name, int write, uint32_t (*OUTPUT)[64]){
    int i=0, y=0, result=1;
    uint32_t BUF=0x00000000;
    
    //Skip header byte if 00
    if(out[*shift]==0x00){
        *shift=*shift+1;
        *length=*length-1;
    }

#if DISPLAY
    printf("%s: \r\n", name);
#endif

    //Extract Hex data from base64 input
    for(i=0; i<*length; i++){
#if DISPLAY
        printf("%02x ", out[*shift]);
#endif
        if(write){
            BUF=(BUF<<8)|out[*shift];
            (*OUTPUT)[y]=BUF;
            if((i+1)%4==0){
                BUF=0x00000000;
                y++;
            }
        }
        
        //Update base64 and hex buffer
        *shift=*shift+1;
        if(*shift>5)
            result = read(shift, in, out, BASE64, SCUR, SMAX);
    }
#if DISPLAY
    printf("\r\n");
#endif

    return result;
}

/*Extract Private Key*/
int priKey(const char *KPRI, int KSIZE){
    int i=0, y=0, shift=6, length=0, result=0, loc=0;
    unsigned char out[6], in[8];
    const char *header="-----BEGIN RSA PRIVATE KEY-----\n";

    //Extract starting header line
    for(loc=0; KPRI[loc]!='\n'; loc++){
        //Verify header line is for private key
        if(KPRI[loc]!=header[loc]){
            #if DISPLAY
            printf("Invalid PRIVATE RSA key file\r\n");
            #endif
            return 0;
        }
    }
    loc++;
  
    #if DISPLAY      
    printf("%s", header);
    #endif
    
    //Extract RSA private key information
    for(i=0; (result=read(&shift, in, out, KPRI, &loc, KSIZE)) !=0; i++){
        switch(i){
            case 0: //header
                if(out[0]==0x30 && out[1]==0x82){
                    #if DISPLAY
                    printf("Header: ASN.1 Sequence\r\n");
                    #endif
                    shift=4;
                    break;
                }
                else{
                    #if DISPLAY
                    printf("Header: Invalid\r\n");
                    #endif
                    return 0;
                }
            case 2: //algorithm version
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "version", PRI_ALGORITHM_VERSION, &priALG))) break;
                else return 0;
                break;
            case 4: //modulus
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "priModulus", PRI_MODULUS, &priMOD))) break;
                else return 0;
                break;
            case 6: //public exponent
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "pubExponent", PRI_PUBLIC_EXPONENT, &priPUBEXP))) break;
                else return 0;
                break;
            case 8: //private exponent
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "priExponent", PRI_PRIVATE_EXPONENT, &priPRIEXP))) break;
                else return 0;
                break;
            case 10: //prime1
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "prime1", PRI_PRIME_1, &priPRI1))) break;
                else return 0;
                break;
            case 12: //prime2
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "prime2", PRI_PRIME_2, &priPRI2))) break;
                else return 0;
                break;
            case 14: //exponent1
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "exponent1", PRI_EXPONENT_1, &priEXP1))) break;
                else return 0;
                break;
            case 16: //exponent2
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "exponent2", PRI_EXPONENT_2, &priEXP2))) break;
                else return 0;
                break;
            case 18://coefficient
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "coefficient", PRI_COEFFICIENT, &priCOE))) return 1;//LAST INPUT
                else return 0;
                break;
            default://Seperator
                if(out[shift]==0x02){
                    shift++;
                    if(out[shift]>>7){
                        length=(out[shift+1]&0x7F)<<8|(out[shift+2]&0x7F);
                        length=0;
                        for(y=0; y<(out[shift]&0x7F); y++)
                            length=length<<8|out[shift+y+1];
                        shift=shift+y+1;
                    }else{
                        length = out[shift];
                        shift++;
                    }
                    #if DISPLAY
                    printf("Seperator: Integer -> %d bytes long\r\n", length);
                    #endif
                }else{
                    #if DISPLAY
                    printf("Seperator: Invalid\r\n");
                    #endif
                    return 0;
                }
        }
    }
    return 0;
}

/*Extract Public Key*/
int pubKey(const char *KPRI, int KSIZE){
    int i=0, y=0, shift=6, length=0, result=0, loc=0;
    unsigned char out[6], in[8];
    const char *header="-----BEGIN PUBLIC KEY-----\n";

    //Extract starting header line
    for(loc=0; KPRI[loc]!='\n'; loc++){
        //Verify header line is for private key
        if(KPRI[loc]!=header[loc]){
            #if DISPLAY
            printf("Invalid PUBLIC RSA key file\r\n");
            #endif
            return 0;
        }
    }
    loc++;
  
    #if DISPLAY      
    printf("%s", header);
    #endif

    //Extract RSA public key information
    for(i=0; (result=read(&shift, in, out, KPRI, &loc, KSIZE)) !=0; i++){
        switch(i){
            case 0: //header
                length=28;
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "Header", PUB_MODULUS, &pubMOD))) break;
                else return 0;
            case 2: //modulus
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "modulus", PUB_MODULUS, &pubMOD))) break;
                else return 0;
            case 4: //public exponent
                if((result=write(&shift, &length, in, out, KPRI, &loc, KSIZE, "exponent", PUB_EXPONENT, &pubEXP))) return 1;
                else return 0;
            default://Seperator
                if(out[shift]==0x02){
                    shift++;
                    if(out[shift]>>7){
                        length=(out[shift+1]&0x7F)<<8|(out[shift+2]&0x7F);
                        length=0;
                        for(y=0; y<(out[shift]&0x7F); y++)
                            length=length<<8|out[shift+y+1];
                        shift=shift+y+1;
                    }else{
                        length = out[shift];
                        shift++;
                    }
                    #if DISPLAY
                    printf("Seperator: Integer -> %d bytes long\r\n", length);
                    #endif
                }else{
                    #if DISPLAY
                    printf("Seperator: Invalid \r\n");
                    #endif
                    return 0;
                }
        }
    }
    return 0;
}

