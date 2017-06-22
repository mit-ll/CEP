#include "verilated.h"
#include "Vdes3_top.h"

#include "./src/des3.h"

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

bool textValid(void) {
    return (readFromAddress(DES3_DONE) != 0) ? true : false;
}

void start(void) {
    writeToAddress(DES3_START, 0x1);
    writeToAddress(DES3_START, 0x0);
}

void writeToKey(uint64_t pKey1, uint64_t pKey2, uint64_t pKey3){
    int i=0;

    for(i=0;i<9;i++){
        remove_bit(&pKey1, (8*i)-i);
        remove_bit(&pKey2, (8*i)-i);
        remove_bit(&pKey3, (8*i)-i);
    }
    
    setKey(, pKey1, , pKey2, , pKey3)
}

void setPlaintext(uint32_t* pPT) {
    printf("Plaintext:\t0x");
    for(unsigned int i = 0; i < BLOCK_WORDS; ++i) {
        writeToAddress(DES3_PT_BASE + (((BLOCK_WORDS - 1) - i) * BYTES_PER_WORD), pPT[i]);
        printf("%08X", pPT[i]);
    }
    printf("\n");
}

void setKey(uint32_t* pKey) {
    printf("Key:\t\t0x");
    for(unsigned int i = 0; i < KEY_WORDS; ++i) {
        writeToAddress(DES3_KEY_BASE + (((KEY_WORDS - 1) - i) * BYTES_PER_WORD), pKey[i]);
        printf("%08X", pKey[i]);
    }
    printf("\n");
}

void write_decrypt(int pData){
    top->decrypt=pData;
}

void write_desIn(uint64_t pData){
    top->desIn=pData;
}
uint64_t read_desOut() {
    uint64_t data = top->desOut;
    return data;
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
    uint64_t des_out;
    bool success=true;

	uint32_t x[10][10]={
	//        key1                    key2                    key3                 Test data               Out data
	{0x0101010, 0x101010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x95F8A5E5, 0xDD31D900, 0x80000000, 0x00000000},
	{0x0101010, 0x101010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x9D64555A, 0x9A10B852, 0x00000010, 0x00000000},
	{0x3849674, 0xC2602319E, 0x3849674C, 0x2602319E, 0x3849674C, 0x2602319E, 0x51454B58, 0x2DDF440A, 0x7178876E, 0x01F19B2A},
	{0x04B915B, 0xA43FEB5B6, 0x04B915BA, 0x43FEB5B6, 0x04B915BA, 0x43FEB5B6, 0x42FD4430, 0x59577FA2, 0xAF37FB42, 0x1F8C4095},
	{0x0123456, 0x789ABCDEF, 0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF, 0x736F6D65, 0x64617461, 0x3D124FE2, 0x198BA318},
	{0x0123456, 0x789ABCDEF, 0x55555555, 0x55555555, 0x01234567, 0x89ABCDEF, 0x736F6D65, 0x64617461, 0xFBABA1FF, 0x9D05E9B1},
	{0x0123456, 0x789ABCDEF, 0x55555555, 0x55555555, 0xFEDCBA98, 0x76543210, 0x736F6D65, 0x64617461, 0x18d748e5, 0x63620572},
	{0x0352020, 0x767208217, 0x86028766, 0x59082198, 0x64056ABD, 0xFEA93457, 0x73717569, 0x67676C65, 0xc07d2a0f, 0xa566fa30},
	{0x0101010, 0x101010101, 0x80010101, 0x01010101, 0x01010101, 0x01010102, 0x00000000, 0x00000000, 0xe6e6dd5b, 0x7e722974},
	{0x1046103, 0x489988020, 0x9107D015, 0x89190101, 0x19079210, 0x981A0101, 0x00000000, 0x00000000, 0xe1ef62c3, 0x32fe825b}};

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
	    write_decrypt(decrypt);
		if(decrypt)	printf("Running Encrypt test ...\r\n\r\n");
    	else		printf("Running Decrypt test ...\r\n\r\n");

	    for(select=0;select<10;select=select+1){
	   	    write_key(x[select][0], x[select][1], x[select][2]);
	   	    write_desIn(x[select][3+decrypt]);
   	    
            start();
            wait_ready();
            des_out=read_desOut();

		    success=success&assertEquals(select, x[select][4-decrypt], des_out);
        }
	}

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

