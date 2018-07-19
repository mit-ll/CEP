#include "verilated.h"
#include "Vidft_top.h"

#include "IDFT.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

Vidft_top* top;

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);
#endif
}

void toggleClock() {
    top->clk = ~top->clk;
}

void setX(uint16_t pX0, uint16_t pX1, uint16_t pX2, uint16_t pX3){
    top->X0=pX0;
    top->X1=pX1;
    top->X2=pX2;
    top->X3=pX3;
    runForClockCycles(1);
}

uint16_t getY0() {
    uint16_t data = top->Y0;
    return data;
}

uint16_t getY1() {
    uint16_t data = top->Y1;
    return data;
}

uint16_t getY2() {
    uint16_t data = top->Y2;
    return data;
}

uint16_t getY3() {
    uint16_t data = top->Y3;
    return data;
}

void getY(uint16_t* pY0, uint16_t* pY1, uint16_t* pY2, uint16_t* pY3) {
    *pY0 = getY0();
    *pY1 = getY1();
    *pY2 = getY2();
    *pY3 = getY3();
    runForClockCycles(1);
}

void start(void) {
    top->next = 1;
    runForClockCycles(1);
    top->next = 0;
}

bool readyValid(void) {
    return (top->next_out != 0) ? true : false;
}

void init(){
    // Initialize Inputs
    top->clk      = 0;
    top->reset    = 0;
    top->next     = 0;
    top->next_out = 0;
    top->X0       = 0x0000;
    top->X1       = 0x0000;
    top->X2       = 0x0000;
    top->X3       = 0x0000;
    runForClockCycles(10);

    top->reset    = 1;
    runForClockCycles(10);

    top->reset    = 0;
    runForClockCycles(10);
    printf("Reset complete\r\n");
}

int main(int argc, char **argv, char **env) {
    int test=2, j=0, i=0;
    uint16_t dout[4]={0x0000, 0x0000, 0x0000, 0x0000};
    bool success=true;

    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true);
#endif
    top = new Vidft_top;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\r\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/Vidft_top.vcd");//VCD file gen
#endif

    printf("Initializing interface and resetting core\r\n");

    // Initialize Inputs
    init();
    
	printf("\r\n");
	printf("*********************************************************\r\n");
	printf("* IDFT core simulation started ...                       *\r\n");
	printf("*********************************************************\r\n");
	printf("\r\n");

    //Set Inputs
    for(i=0; i<test; i++){
        start();
        // The 64 complex data points enter the system over 32 cycles
        for (j=0; j < 32; j++){
            switch(i){
            case 0: setX(j*4, j*4+1, j*4+2, j*4+3); break;// Input: 2 complex words per cycle
            case 1: setX(128+j*4, 128+j*4+1, 128+j*4+2, 128+j*4+3); break;// Input 4 words per cycle
            }
        }
        // Wait until the next data vector can be entered
        runForClockCycles(1);
    }
    
    //Check Outputs
    for(i=0; i<test; i++){
        waitForValidOutput();
        runForClockCycles(1);
        printf("--- begin output %02d---\r\n", i+1);
        for(j=0;j<32;j++){
            getY(&dout[0], &dout[1], &dout[2], &dout[3]);
            switch(i){
            case 0: success=success&assertEquals(j, exp_1[j], dout); break;
            case 1: success=success&assertEquals(j, exp_2[j], dout); break;
            }
        }
    }
    
	printf("\r\n");
    assertSuccess(success);
	printf("\r\n");
	printf("**************************************\r\n");
	printf("* IDFT Test done ...                  *\r\n");
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

