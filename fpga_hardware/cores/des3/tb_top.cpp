#include "verilated.h"
#include "Vdes3_top.h"

#include "DES3.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

Vdes3_top* top;

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);
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

bool readyValid(void) {
    return (readFromAddress(DES3_DONE) != 0) ? true : false;
}

void start(void) {
    writeToAddress(DES3_START, 0x1);
    writeToAddress(DES3_START, 0x0);
}

void writeToDecrypt(int i) {
    if(i) writeToAddress(DES3_DECRYPT, 0x1);
    else  writeToAddress(DES3_DECRYPT, 0x0);
}

void setPlaintext(uint32_t* pPT) {
    for(unsigned int i = 0; i < BLOCK_WORDS; ++i)
        writeToAddress(DES3_IN_BASE + (((BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), pPT[i]);
}

void setKey(uint32_t* pKey) {
    unsigned int i;
    for(i=0;i<5;i++){
        remove_bit32(&pKey[0], (8*i)-i);
        remove_bit32(&pKey[1], (8*i)-i);
        remove_bit32(&pKey[2], (8*i)-i);
        remove_bit32(&pKey[3], (8*i)-i);
        remove_bit32(&pKey[4], (8*i)-i);
        remove_bit32(&pKey[5], (8*i)-i);
    }
    for(i = 0; i < KEY_WORDS; ++i)
        writeToAddress(DES3_KEY_BASE + (((KEY_WORDS - 1) - i) * BYTES_PER_WORD), pKey[i]);
}

void saveCiphertext(uint32_t *pCT) {
    for(unsigned int i = 0; i < BLOCK_WORDS; ++i) {
        pCT[(BLOCK_WORDS - 1) - i] = readFromAddress(DES3_CT_BASE + (i * BYTES_PER_WORD));
    }
}

void init(){
    // Initialize Inputs
    top->wb_clk_i = 0;
    top->wb_dat_i = 0;
    top->wb_we_i = 0;
    top->wb_adr_i = 0;
    top->wb_stb_i = 0;
    runForClockCycles(100);

    printf("Reset complete\r\n");
}

int main(int argc, char **argv, char **env) {
    int decrypt=0, select=0;
    uint32_t ct[BLOCK_WORDS];
    bool success=true;

	uint64_t x[10][10]={
	//       key1                key2                key3             Test data           Out data
	{0x0101010101010101, 0x0101010101010101, 0x0101010101010101, 0x95F8A5E5DD31D900, 0x8000000000000000},
	{0x0101010101010101, 0x0101010101010101, 0x0101010101010101, 0x9D64555A9A10B852, 0x0000001000000000},
	{0x3849674C2602319E, 0x3849674C2602319E, 0x3849674C2602319E, 0x51454B582DDF440A, 0x7178876E01F19B2A},
	{0x04B915BA43FEB5B6, 0x04B915BA43FEB5B6, 0x04B915BA43FEB5B6, 0x42FD443059577FA2, 0xAF37FB421F8C4095},
	{0x0123456789ABCDEF, 0x0123456789ABCDEF, 0x0123456789ABCDEF, 0x736F6D6564617461, 0x3D124FE2198BA318},
	{0x0123456789ABCDEF, 0x5555555555555555, 0x0123456789ABCDEF, 0x736F6D6564617461, 0xFBABA1FF9D05E9B1},
	{0x0123456789ABCDEF, 0x5555555555555555, 0xFEDCBA9876543210, 0x736F6D6564617461, 0x18d748e563620572},
	{0x0352020767208217, 0x8602876659082198, 0x64056ABDFEA93457, 0x7371756967676C65, 0xc07d2a0fa566fa30},
	{0x0101010101010101, 0x8001010101010101, 0x0101010101010102, 0x0000000000000000, 0xe6e6dd5b7e722974},
	{0x1046103489988020, 0x9107D01589190101, 0x19079210981A0101, 0x0000000000000000, 0xe1ef62c332fe825b}};

    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true);
#endif
    top = new Vdes3_top;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\r\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/Vdes3_top.vcd");//VCD file gen
#endif

    printf("Initializing interface and resetting core\r\n");

    // Initialize Inputs
    init();
    
	printf("\r\n");
	printf("*********************************************************\r\n");
	printf("* Area Optimized DES core simulation started ...        *\r\n");
	printf("*********************************************************\r\n");
	printf("\r\n");

	for(decrypt=0;decrypt<2;decrypt=decrypt+1){
	    writeToDecrypt(decrypt);
		if(decrypt)	printf("Running Encrypt test ...\r\n\r\n");
    	else		printf("Running Decrypt test ...\r\n\r\n");

	    for(select=0;select<10;select=select+1){
	        uint32_t key[6]={x[select][0]>>32, x[select][0], x[select][1]>>32, x[select][1], x[select][2]>>32, x[select][2]};
	   	    setKey(key);
	   	    uint32_t pt[2]={x[select][3+decrypt]>>32, x[select][3+decrypt]};
	   	    setPlaintext(pt);
   	    
            start();
            waitForValidOutput();
            saveCiphertext(ct);

		    success=success&assertEquals(select, x[select][4-decrypt]>>32, x[select][4-decrypt], ct[1], ct[0]);
        }
	}

	printf("\r\n");
    assertSuccess(success);
	printf("\r\n");
	printf("**************************************\r\n");
	printf("* DES Test done ...                  *\r\n");
	printf("**************************************\r\n");
	printf("\r\n");
    
    printf("Completed\r\n");

#if VM_TRACE    
    tfp->close();
#endif
    top->final();
    delete top;
    exit(0);
}

