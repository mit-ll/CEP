#include "verilated.h"
#include "Vmodexp_top.h"

#include "./src/RSA.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

Vmodexp_top* top;

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);
#endif
}

void toggleClock() {
    top->wb_clk_i = ~top->wb_clk_i;
}

uint32_t read_word(uint32_t pAddress) {
    top->wb_stb_i=1;
    top->wb_we_i=0;
    top->wb_adr_i = pAddress;
    runForClockCycles(1);
    uint32_t data = top->wb_dat_o;
    top->wb_stb_i=0;
    top->wb_we_i=0;
    return data;
}

void write_word(uint32_t pAddress, uint32_t pData) {
    top->wb_stb_i=1;
    top->wb_we_i=1;
    top->wb_adr_i = pAddress;
    top->wb_dat_i = pData;
    runForClockCycles(1);
    top->wb_stb_i=0;
    top->wb_we_i=0;
}

void init(){
    top->wb_clk_i = 0;
    top->wb_rst_i = 0;
    top->wb_stb_i = 0;
    top->wb_we_i  = 0;
    top->wb_adr_i = 0x00000000;
    top->wb_dat_i = 0x00000000;
    runForClockCycles(10);

    printf("Reset complete\r\n");

    top->wb_rst_i = 1;
    runForClockCycles(10);
}

int main(int argc, char **argv, char **env) {
    //int i=0;

    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true);
#endif
    top = new Vmodexp_top;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/Vmodexp_top.vcd");//VCD file gen
#endif

    // Initialize Inputs
    printf("Initializing interface and resetting core\r\n");
    init();

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

#if VM_TRACE    
    tfp->close();
#endif
    top->final();
    delete top;
    exit(0);
}
