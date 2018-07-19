#include "DFT.h"

// Need this to fully implement expected interface
void toggleClock() {;}
void evalModel() {;}

// Need volatile to make sure the compiler doesn't make bus writes/read disappear
uint32_t readFromAddress(uint32_t pAddress) {
    return *((volatile uint32_t *)pAddress);
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
    *((volatile uint32_t *)pAddress) = pData;
}

bool readyValid(void) {
    return (readFromAddress(DFT_DONE) != 0) ? true : false;
}

void start(void) {
    writeToAddress(DFT_START, 0x1);
    writeToAddress(DFT_START, 0x0);
}

void setX(uint16_t i, uint16_t pX0, uint16_t pX1, uint16_t pX2, uint16_t pX3){
    writeToAddress(DFT_IN_DATA, pX1<<16|pX0);             //Write data
    writeToAddress(DFT_IN_ADDR, i);                       //Write addr
    writeToAddress(DFT_IN_WRITE, 0x1);                    //Load data
    writeToAddress(DFT_IN_WRITE, 0x0);                    //Stop
    
    writeToAddress(DFT_IN_DATA+BLOCK_BYTES, pX3<<16|pX2); //Write data
    writeToAddress(DFT_IN_ADDR, i);                       //Write addr
    writeToAddress(DFT_IN_WRITE, 0x1);                    //Load data
    writeToAddress(DFT_IN_WRITE, 0x0);                    //Stop
}

void getY(uint16_t i, uint16_t *pY0, uint16_t *pY1, uint16_t *pY2, uint16_t *pY3) {
    uint32_t temp;
    writeToAddress(DFT_OUT_ADDR, i);                //Write addr

    temp=readFromAddress(DFT_OUT_DATA);             //Read data
    *pY0=temp;
    *pY1=temp>>16;

    temp=readFromAddress(DFT_OUT_DATA+BLOCK_BYTES); //Read data
    *pY2=temp;
    *pY3=temp>>16;
}

int main(int argc, char **argv, char **env) {
    int test=2, j=0, i=0;
    uint16_t dout[4]={0x0000, 0x0000, 0x0000, 0x0000};
    bool success=true;

    printf("Initializing interface and resetting core\r\n");

	printf("\r\n");
	printf("*********************************************************\r\n");
	printf("* DFT core simulation started ...                       *\r\n");
	printf("*********************************************************\r\n");
	printf("\r\n");

    //Set Inputs
    for(i=0; i<test; i++){
        //start();
        // The 64 complex data points enter the system over 32 cycles
        for (j=0; j < 32; j++){
            switch(i){
            case 0: setX(j, j*4, j*4+1, j*4+2, j*4+3); break;// Input: 2 complex words per cycle
            case 1: setX(j, 128+j*4, 128+j*4+1, 128+j*4+2, 128+j*4+3); break;// Input 4 words per cycle
            }
        }
       
        start();

    //Check Outputs
        waitForValidOutput();
        runForClockCycles(1);
        printf("--- begin output %02d---\r\n", i+1);
        for(j=0;j<32;j++){
            getY(j, &dout[0], &dout[1], &dout[2], &dout[3]);
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
	printf("* DFT Test done ...                  *\r\n");
	printf("**************************************\r\n");
	printf("\r\n");
    
    printf("Completed\r\n");

    exit(0);
}

