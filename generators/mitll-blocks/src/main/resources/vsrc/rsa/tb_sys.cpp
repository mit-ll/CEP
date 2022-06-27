#include "./src/RSA.h"

// Need this to fully implement expected interface
void evalModel() {;}
void toggleClock() {;}

// Need volatile to make sure the compiler doesn't make bus writes/read disappear
uint32_t read_word(uint32_t pAddress) {
    return *((volatile uint32_t *)pAddress);
}

void write_word(uint32_t pAddress, uint32_t pData) {
    *((volatile uint32_t *)pAddress) = pData;
}

int main(int argc, char **argv, char **env) {
    //int i=0;

    printf("Initializing interface and resetting core\r\n");

    //32-bit modulo exponentiation tests
    printf("****************************************\r\n");
    printf("  32-bit Mod EXP Tests\r\n");
    printf("****************************************\r\n");
    modexp_32bits(0x00000001, 0x00000002, 0x00000005, 0x00000001); //msg^exp < mod -> 1^2 < 5    
    modexp_32bits(0x00000001, 0x00000002, 0x00000003, 0x00000001); //msg^exp < mod -> 1^2 < 3
    modexp_32bits(0x00000002, 0x00000002, 0x00000005, 0x00000004); //msg^exp < mod -> 2^2 < 5
    modexp_32bits(0x00000002, 0x00000002, 0x00000003, 0x00000001); //msg^exp > mod -> 2^2 > 3
    modexp_32bits(0x00000004, 0x0000000D, 0x000001F1, 0x000001bd); //msg^exp > mod -> 4^13 > 497
    modexp_32bits(0x01234567, 0x89ABCDEF, 0x11111111, 0x0D9EF081); //msg^exp > mod -> 19088743^2309737967 > 286331153
    modexp_32bits(0x30000000, 0xC0000000, 0x00A00001, 0x0000CC3F); //msg^exp > mod ->
 
    //Preset Test: 32bit exponent with 2048bit modulus
    printf("****************************************\r\n");
    printf("  32-bit Exp 2048-bit Mod Test\r\n");
    printf("****************************************\r\n");
    exp32bit_mod2048bit_test();
 
    //Convert input from ASCII to HEX
    printf("****************************************\r\n");
    printf("  Allocating Input and Output\r\n");
    printf("****************************************\r\n");
    a2h(IMSG, SMSG, &PMSG);
    printf("Input:%s\r\n", IMSG);
    DMSG = (char*)malloc(SMSG);

    //Extract Public and Private Key information
    printf("****************************************\r\n");
    printf("  Extracting Key - Pri\r\n");
    printf("****************************************\r\n");
    if(priKey(FPRI, SPRI)); else exit(EXIT_FAILURE);
    printf("****************************************\r\n");
    printf("  Extracting Key - Pub\r\n");
    printf("****************************************\r\n");
    if(pubKey(FPUB, SPUB)); else exit(EXIT_FAILURE);
    
    //Use MSGs and Keys extract to perform RSA encrypt and decrypt
    printf("****************************************\r\n");
    printf("  RSA Encrypting\r\n");
    printf("****************************************\r\n");
    modexp_encrypt();
    printf("****************************************\r\n");
    printf("  RSA Decrypt\r\n");
    printf("****************************************\r\n");
    modexp_decrypt();
    
    //For Testing -> Generated Text equal to Plain Text
    //for(i=0; i<64; i++) GMSG[i]=PMSG[i];

    //Convert input from ASCII to HEX
    printf("****************************************\r\n");
    printf("  Converting Output\r\n");
    printf("****************************************\r\n");
    h2a(GMSG, SMSG, &DMSG);
    printf("OUTPUT: %s\r\n", DMSG);
    
    printf("Completed\n\r");

    return 0;
}
