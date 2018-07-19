#include "verilated.h"
#include "VFIR_filter.h"

#include "FIR.h"

#if VM_TRACE 
#include "verilated_vcd_c.h"
VerilatedVcdC* tfp = new VerilatedVcdC;
#endif 

VFIR_filter* top;

void evalModel() {
    top->eval();
#if VM_TRACE
    tfp->dump(main_time);
#endif
}

void toggleClock() {
    top->clk = ~top->clk;
}

void setInData(uint32_t idata){
    top->inData=idata;
    runForClockCycles(1);
}

uint32_t getOutData() {
    uint32_t odata = top->outData;
    return odata;
}

bool readyValid(void) {
    return true;
}

void init(){
    // Initialize Inputs
    top->clk     = 0;
    top->reset   = 0;
    top->inData  = 0x00000000;
    top->outData = 0x00000000;
    runForClockCycles(10);

    top->reset    = 1;
    runForClockCycles(10);

    printf("Reset complete\r\n");
}

int main(int argc, char **argv, char **env) {
    int test=2, j=0, i=0;
    uint32_t dout=0x00000000;
    bool success=true;

    Verilated::commandArgs(argc, argv);
#if VM_TRACE
    Verilated::traceEverOn(true);
#endif
    top = new VFIR_filter;

#if VM_TRACE
    printf("Initializing traces and opening VCD file\r\n");
    top->trace (tfp, 99);//VCD file gen 
    tfp->open("./obj_dir/VFIR_filter.vcd");//VCD file gen
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
    for (j=0; j < 32+8; j++){
        if(j<32) setInData(j*4);
        else     setInData(0);
        
        if(j>7){
            dout=getOutData();
            success=success&assertEquals(j-8, exp_1[j-8], dout);
        }
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

