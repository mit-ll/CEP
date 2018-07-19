#include "verilated.h"
#include "Vfir_top.h"

#include "FIR.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

Vfir_top* top;

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
    return (readFromAddress(FIR_DONE) != 0) ? true : false;
}

void start(void) {
    writeToAddress(FIR_START, 0x1);
    writeToAddress(FIR_START, 0x0);
}

void setInData(uint32_t i, uint32_t idata){
    writeToAddress(FIR_IN_DATA, idata); //Write data
    writeToAddress(FIR_IN_ADDR, i);     //Write addr
    writeToAddress(FIR_IN_WRITE, 0x1);  //Load data
    writeToAddress(FIR_IN_WRITE, 0x0);  //Stop
}

uint32_t getOutData(uint32_t i) {
    uint32_t temp;
    
    writeToAddress(FIR_OUT_ADDR, i);    //Write addr
    temp=readFromAddress(FIR_OUT_DATA); //Read data
    return temp;
}

void init(){
    // Initialize Inputs
    top->wb_clk_i = 0;
    top->wb_dat_i = 0;
    top->wb_we_i  = 0;
    top->wb_adr_i = 0;
    top->wb_stb_i = 0;
    runForClockCycles(100);

    printf("Reset complete\r\n");
}

int main(int argc, char **argv, char **env) {
    int i=0;
    uint32_t dout=0x00000000;
    bool success=true;

    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true);
#endif
    top = new Vfir_top;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\r\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/Vfir_top.vcd");//VCD file gen
#endif

    printf("Initializing interface and resetting core\r\n");

    // Initialize Inputs
    init();
    
	printf("\r\n");
	printf("*********************************************************\r\n");
	printf("* FIR core simulation started ...                       *\r\n");
	printf("*********************************************************\r\n");
	printf("\r\n");

    //Set Inputs and check Outputs
    for (i=0; i < 32; i++)
        setInData(i, i*4);
    
    start();
    waitForValidOutput();
       
    for (i=0; i < 32; i++){
        dout=getOutData(i);
        success=success&assertEquals(i, exp_1[i], dout);
    }
    
    
    runForClockCycles(10);

	printf("\r\n");
    assertSuccess(success);
	printf("\r\n");
	printf("**************************************\r\n");
	printf("* FIR Test done ...                  *\r\n");
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

