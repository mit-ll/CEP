#include "verilated.h"
#include "Vmodexp.h"
#include "./src/RSA.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

Vmodexp* top;

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);
#endif
}

void toggleClock() {
    top->clk = ~top->clk;
}

uint32_t read_word(uint32_t pAddress) {
    top->cs=1;
    top->we=0;
    top->address = pAddress>>2;
    runForClockCycles(1);
    uint32_t data = top->read_data;
    top->cs=0;
    top->we=0;
    return data;
}

void write_word(uint32_t pAddress, uint32_t pData) {
    top->cs=1;
    top->we=1;
    top->address = pAddress>>2;
    top->write_data = pData;
    runForClockCycles(1);
    top->cs=0;
    top->we=0;
}

void init(){
    top->clk = 0;
    top->reset_n = 0;
    top->cs = 0;
    top->we = 0;
    top->address = 0x00;
    top->write_data = 0x00000000;
    runForClockCycles(10);

    printf("Reset complete\r\n");

    top->reset_n = 1;
    runForClockCycles(10);
}

int main(int argc, char **argv, char **env) {
    int i=0;

    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true);
#endif
    top = new Vmodexp;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/Vmodexp.vcd");//VCD file gen
#endif

    printf("Initializing interface and resetting core\r\n");

    // Initialize Inputs
    init();
    
    //Convert input from ASCII to HEX
    a2h(IMSG, SMSG, &PMSG);
    printf("Input:%s\r\n", IMSG);
    DMSG = (char*)malloc(SMSG);

    //Extract Public and Private Key information
    if(priKey(FPRI, SPRI)); else exit(EXIT_FAILURE);
    if(pubKey(FPUB, SPUB)); else exit(EXIT_FAILURE);
    
    //Use MSGs and Keys extract to perform RSA encrypt and decrypt
    //modexp_encrypt();
    //modexp_decrypt();
    
    //Preset Test: 32bit exponent with 2048bit modulus
    //exp32bit_mod2048bit_test();
    
    //32-bit modulo exponentiation tests
    modexp_32bits(0x00000001, 0x00000002, 0x00000005, 0x00000001); //msg^exp < mod -> 1^2 < 5    
    //modexp_32bits(0x00000001, 0x00000002, 0x00000003, 0x00000001); //msg^exp < mod -> 1^2 < 3
    //modexp_32bits(0x00000002, 0x00000002, 0x00000005, 0x00000004); //msg^exp < mod -> 2^2 < 5
    //modexp_32bits(0x00000002, 0x00000002, 0x00000003, 0x00000001); //msg^exp > mod -> 2^2 > 3
    //modexp_32bits(0x00000004, 0x0000000D, 0x000001F1, 0x000001bd); //msg^exp > mod -> 4^13 > 497
    //modexp_32bits(0x01234567, 0x89ABCDEF, 0x11111111, 0x0D9EF081); //msg^exp > mod -> 19088743^2309737967 > 286331153
    //modexp_32bits(0x30000000, 0xC0000000, 0x00A00001, 0x0000CC3F); //msg^exp > mod ->
    
    //For Testing -> Generated Text equal to Plain Text
    for(i=0; i<64*4; i++) GMSG[i]=PMSG[i];

    //Convert input from ASCII to HEX
    h2a(PMSG, SMSG, &DMSG);
    printf("OUTPUT: %s\r\n", DMSG);
    
    printf("Completed\n\r");

#if VM_TRACE    
    tfp->close();
#endif
    top->final();
    delete top;
    exit(0);
}
