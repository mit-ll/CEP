#include "verilated.h"
#include "Vdes3.h"

#include "DES3.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

Vdes3* top;

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);
#endif
}

void toggleClock() {
    top->clk = ~top->clk;
}

void setKey(uint64_t pKey1, uint64_t pKey2, uint64_t pKey3){
    int i=0;
    for(i=0;i<9;i++){
        remove_bit64(&pKey1, (8*i)-i);
        remove_bit64(&pKey2, (8*i)-i);
        remove_bit64(&pKey3, (8*i)-i);
    }
    
    top->key1=pKey1;
    top->key2=pKey2;
    top->key3=pKey3;
}

void setDecrypt(int pData){
    top->decrypt=pData;
}

void setPlainText(uint64_t pData){
    top->desIn=pData;
}

uint64_t readDesOut() {
    uint64_t data = top->desOut;
    return data;
}

void start(void) {
    top->start = 1;
    runForClockCycles(5);
    top->start = 0;
}
bool readyValid(void) {
    return (top->out_valid != 0) ? true : false;
}

void init(){
    // Initialize Inputs
    top->desOut   = 0x0000000000000000;
    top->out_valid= 0;
    top->start    = 0;
    top->desIn    = 0x0000000000000000;
    top->key1     = 0x00000000000000;
    top->key2     = 0x00000000000000;
    top->key3     = 0x00000000000000;
    top->decrypt  = 0;
    top->clk      = 0;
    runForClockCycles(10);

    printf("Reset complete\r\n");
}

int main(int argc, char **argv, char **env) {
    int decrypt=0, select=0;
    uint64_t des_out;
    bool success=true;

	uint64_t x[10][5]={
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
    top = new Vdes3;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\r\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/Vdes3.vcd");//VCD file gen
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
	    setDecrypt(decrypt);
		if(decrypt)	printf("Running Encrypt test ...\r\n\r\n");
    	else		printf("Running Decrypt test ...\r\n\r\n");

	    for(select=0;select<10;select=select+1){
	        setKey(x[select][0], x[select][1], x[select][2]);
	   	    setPlainText(x[select][3+decrypt]);
   	    
            start();
            waitForValidOutput();
            des_out=readDesOut();
            runForClockCycles(10);

		    success=success&assertEquals(select, x[select][4-decrypt]>>32, x[select][4-decrypt], des_out>>32, des_out);
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

