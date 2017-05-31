#include "verilated.h"
#include "Vmodexp.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "AES.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"//VCD file gen
#endif 

Vmodexp* top;
#if VM_TRACE 
VerilatedVcdC* tfp = new VerilatedVcdC; //VCD file gen
#endif

const uint8_t ADDR_MODULUS_LENGTH   = 0x20;
const uint8_t ADDR_EXPONENT_LENGTH  = 0x21;
const uint8_t ADDR_MODULUS_PTR_RST  = 0x30;
const uint8_t ADDR_MODULUS_DATA     = 0x31;
const uint8_t ADDR_EXPONENT_PTR_RST = 0x40;
const uint8_t ADDR_EXPONENT_DATA    = 0x41;
const uint8_t ADDR_MESSAGE_PTR_RST  = 0x50;
const uint8_t ADDR_MESSAGE_DATA     = 0x51;
const uint8_t ADDR_RESULT_PTR_RST   = 0x60;
const uint8_t ADDR_RESULT_DATA      = 0x61;


void evalModel() {
    top->eval();
}

void toggleClock() {
    top->clk = ~top->clk;
#if VM_TRACE
    tfp->dump(main_time);//VCD file gen
#endif
}

uint32_t readFromAddress(uint32_t pAddress) {
    top->address = pAddress;
    runForClockCycles(10);
    uint32_t data = top->read_data;
    runForClockCycles(10);
    
    return data;
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
    top->address = pAddress;
    top->write_data = pData;
    runForClockCycles(10);
}

bool ciphertextValid(void) {
    return (readFromAddress(AES_DONE) != 0) ? true : false;
}

void start(void) {
    writeToAddress(AES_START, 0x1);
    writeToAddress(AES_START, 0x0);
}

void reportCiphertext(void) {
    printf("Ciphertext:\t0x");
    for(int i = (BLOCK_BITS / 32) - 1; i >= 0; --i) {
        printf("%08X", readFromAddress(AES_CT_BASE + (i * 4)));
    }
    printf("\n");
}

void saveCiphertext(uint32_t *pCT) {
    for(int i = 0; i < (BLOCK_BITS / 32); ++i) {
        *pCT++ = readFromAddress(AES_CT_BASE + (i * 4));
    }
}

void setPlaintext(const char* pPT) {
    cout << "Plaintext:\t0x";
    for(int i = 0; i < (BLOCK_BITS / 8); ++i) {
        uint32_t temp;
        for(int j = 0; j < 4; ++j) {
            ((char *)(&temp))[3 - j] = *pPT++;
        }
        writeToAddress(AES_PT_BASE + ((3 - i) * 4), temp);
        cout << hex << setfill('0') << setw(8) << temp;
    }
    cout << endl;
}

void setPlaintext(const uint32_t* pPT) {
    cout << "Plaintext:\t0x";
    for(int i = 0; i < (BLOCK_BITS / 32); ++i) {
        writeToAddress(AES_PT_BASE + ((3 - i) * 4), pPT[i]);
        cout << hex << setfill('0') << setw(8) << pPT[i];
    }
    cout << endl;
}

void setKey(const uint32_t* pKey) {
    cout << "Key:\t\t0x";
    for(int i = 0; i < (BLOCK_BITS / 32); ++i) {
        writeToAddress(AES_KEY_BASE + ((3 - i) * 4), pKey[i]);
        cout << hex << setfill('0') << setw(8) << pKey[i];
    }
    cout << endl;
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true); //VCD file gen
#endif
    top = new Vmodexp;
  
    uint32_t exponent = 0x000000ff; 
    uint32_t mod[64]  = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
    uint32_t msg[64]  = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000, 
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
   uint32_t out[64]  = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x3fffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xc0000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t read[64] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000, 
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t exp_1[64]= {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x3fffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                         0xc0000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};


#if VM_TRACE
  top->trace (tfp, 99);//VCD file gen 
  tfp->open("./obj_dir/Vmodexp.vcd");//VCD file gen
#endif

    cout << "Initializing interface and resetting core" << endl;
    
    // Initialize Inputs
    top->clk = 0;
    top->reset_n = 0;
    top->cs = 0;
    top->we = 0;
    top->address = 0x0;
    top->write_data = 0x00000000;
    runForClockCycles(100);
    
    cout << "Reset complete" << endl;

    top->reset_n = 1;
    top->cs=1;
    top->we=1;
    runForClockCycles(1);

    cout << "Modulus    -- Writing" <<endl;

    top->address = 0x40; top->write_data = 0x00000000; //ADDR_EXPONENT_PTR_RST
    runForClockCycles(1);
    top->address = 0x41; top->write_data = 0x000000ff; //ADDR_EXPONENT_DATA
    runForClockCycles(1);

    top->address = 0x30; top->write_data = 0x00000000; //ADDR_MODULUS_PTR_RST
    runForClockCycles(1);
    top->address = 0x31; top->write_data = 0x00000000; //ADDR_MODULUS_DATA
    runForClockCycles(1);
    
    for(int i=0;i<64;i++){
        top->address = 0x31; top->write_data = mod[i]; runForClockCycles(1); //ADDR_MODULUS_DATA
        runForClockCycles(1);
    }

    cout << "           -- Complete" <<endl;
    cout << "Message    -- Writing" <<endl;

    top->address = 0x50; top->write_data = 0x00000000; //ADDR_MESSAGE__PTR_RST
    runForClockCycles(1);
    top->address = 0x51; top->write_data = 0x00000000; //ADDR_MESSAGE_DATA
    runForClockCycles(1);

    for(int i=0;i<64;i++){
        top->address = 0x51; top->write_data = msg[i]; runForClockCycles(1); //ADDR_MESSAGE_DATA
        runForClockCycles(1);
    }

    top->address = 0x21; top->write_data = 0x00000001; //ADDR_EXPONENT_LENGTH
    runForClockCycles(1);
    top->address = 0x20; top->write_data = 0x00000041; //ADDR_MODULUS_LENGTH
    runForClockCycles(1);

    cout << "           -- Complete" <<endl;
    cout << "Processing -- Start" <<endl;

top->cs=0; top->we=0;
runForClockCycles(1);

    
    top->cs=1; top->we=1;
    top->address = 0x08; top->write_data = 0x00000001; //Start processing and wait for ready
    runForClockCycles(1);
 
    top->cs=1; top->we=0;
    top->address = 0x09;

    //Wait for ready
    while(top->read_data != 0x00000001){
        runForClockCycles(1);
    }
 
    cout << "           -- Complete" <<endl;
    cout << "Read Data  -- Start" <<endl;

    top->cs=1; top->we=1;
    top->address = 0x60; top->write_data = 0x00000000;
    runForClockCycles(1);
    
    
    top->cs=1; top->we=0;
    top->address = 0x61;
    read[0]=top->read_data;
    for(int i=0;i<64;i++){
       runForClockCycles(1);
       read[i]=top->read_data;
        
        if(read[i] != exp_1[i]) printf("INCORRECT OUTPUT: %08X != %08X \n\r", read[i], exp_1[i]);
    }
cout << "COMPLETE" <<endl;

    //setPlaintext(pt1);
    //setKey(key1);
    //start();
    //waitForValidOutput();
    //reportCiphertext();
    //verifyCiphertext("3925841d02dc09fbdc118597196a0b32", "test 1");
#if VM_TRACE    
    tfp->close(); //VCD file gen
#endif
    top->final();
    delete top;
    exit(0);
}
