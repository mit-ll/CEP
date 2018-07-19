#include "verilated.h"
#include "Vmd5_top.h"
#include <iostream>
#include <fstream>
#include "MD5.h"
#include "input.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"//VCD file gen
#endif 

Vmd5_top* top;
#if VM_TRACE 
VerilatedVcdC* tfp = new VerilatedVcdC; //VCD file gen
#endif

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);//VCD file gen
#endif
}

void toggleClock() {
    top->wb_clk_i = ~top->wb_clk_i;
}

void waitForACK() {
  while(top->wb_ack_o == 0) {
    runForClockCycles(1);
  }
}

uint32_t readFromAddress(uint32_t pAddress) {
  top->wb_adr_i = pAddress;
  top->wb_dat_i = 0x00000000;
  top->wb_we_i = 0;
  top->wb_stb_i = 1;
  runForClockCycles(10);
  waitForACK();
  uint32_t data = top->wb_dat_o;
  top->wb_stb_i = 0;
  runForClockCycles(10);

  return data;
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
  top->wb_adr_i = pAddress;
  top->wb_dat_i = pData;
  top->wb_we_i = 1;
  top->wb_stb_i = 1;
  runForClockCycles(10);
  waitForACK();
  top->wb_stb_i = 0;
  top->wb_we_i = 0;
  runForClockCycles(10);
}

void updateHash(unsigned char *pHash) {
    uint32_t* hPtr = (uint32_t *)pHash;
    
    for(int i = HASH_WORDS - 1; i >= 0; --i) {
        *hPtr++ = readFromAddress(MD5_HASH_BASE + (i * BYTES_PER_WORD));
    }
}

void reportAppended() {
    printf("Padded input:\n");
    for(int i = MESSAGE_WORDS - 1; i >= 0; --i) {
	    uint32_t dataPart = readFromAddress(MD5_MSG_BASE + (i * BYTES_PER_WORD));
        printf("0x%08X\n", dataPart);
    }
}

void waitForReady() {
  while(readFromAddress(MD5_READY) == 0) {
    runForClockCycles(15);
  }
}

void waitForValidOut() {
  waitForReady();
}

void resetAndReady() {
  writeToAddress(MD5_RST, 0x1);
  runForClockCycles(30);
  writeToAddress(MD5_RST, 0x0);
  waitForReady();
}

void strobeMsgValid() {
  writeToAddress(MD5_READY, 0x1);
  writeToAddress(MD5_READY, 0x0);
}

void loadPaddedMessage(const char* msg_ptr) {
  for(int i = 0; i < MESSAGE_WORDS; ++i) {
    uint32_t temp = 0;
    for(int j = 0; j < BYTES_PER_WORD; ++j) {
      ((char *)(&temp))[j] = *msg_ptr++;
    }
    writeToAddress(MD5_MSG_BASE + (i * BYTES_PER_WORD), temp);
  }
}

int main(int argc, char **argv, char **env) {
  Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true); //VCD file gen
#endif
  top = new Vmd5_top;
  unsigned char hash[HASH_BYTES];
    
#if VM_TRACE
  printf("Initializing traces and opening VCD file\n");
  top->trace (tfp, 99);//VCD file gen 
  tfp->open("./obj_dir/Vmd5_top.vcd");//VCD file gen
#endif
  cout << "Resetting the wishbone interface..." << endl;

  // Unused/constant signals
  top->wb_rst_i = 0;
  top->wb_cyc_i = 0;
  top->wb_sel_i = 15;
    
  // Reset bus interface
  top->wb_rst_i = 1;
  runForClockCycles(10);
  top->wb_rst_i = 0;

  cout << "Reset complete" << endl;
  cout << "Resetting the MD5 module..." << endl;
    
  // Test reset behavior
  resetAndReady();

  cout << "Reset complete" << endl;

  hashString("a", hash);
  compareHash(hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
    
  hashString("abc", hash);
  compareHash(hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
    
  // Test a message that requires an extra chunk to be added to hold padding
  hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", hash);
  compareHash(hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");

  // Hash a test file
  ifstream inFile("input.txt", ios::in|ios::binary|ios::ate);
  if(inFile.is_open()) {
    streampos size = inFile.tellg();

    inFile.seekg(0, ios::beg);

    int messageBits = (size - inFile.tellg()) * BITS_PER_BYTE;
    printf("Message length: %d bits\n", messageBits);
      
    char * block = new char[messageBits / BITS_PER_BYTE];
    memset(block, 0, messageBits/BITS_PER_BYTE);
      
    inFile.read(block, size);
    inFile.close();
      
    hashString(block, hash);
    compareHash(hash, "a3af880db6067a6c7592fb01e877e767", "text file");
  } else {
    cout << "ERROR: unable to open input.txt" << endl;
  }

#if VM_TRACE    
    tfp->close(); //VCD file gen
#endif
  top->final();
  delete top;
  exit(0);
}
