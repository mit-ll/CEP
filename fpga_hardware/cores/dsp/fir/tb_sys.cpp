#include "FIR.h"

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
    return (readFromAddress(DES3_START+0x10) != 0) ? true : false;
}

void start(void) {
    writeToAddress(DES3_START, 0x1);
    writeToAddress(DES3_START, 0x0);
}

void setInData(uint32_t i, uint32_t idata){
    writeToAddress(DES3_START+0xC, idata); //Write data
    writeToAddress(DES3_START+0x8, i);     //Write addr
    writeToAddress(DES3_START+0x4, 0x1);   //Load
    writeToAddress(DES3_START+0x4, 0x0);   //Stop
}

uint32_t getOutData(uint32_t i) {
    uint32_t temp;
    
    writeToAddress(DES3_START+0x10, i);    //000100 00
    temp=readFromAddress(DES3_START+0x1C); //000111 00
    return temp;
}

int main(int argc, char **argv, char **env) {
    int i=0;
    uint32_t dout=0x00000000;
    bool success=true;

    printf("Initializing interface and resetting core\r\n");

    // Initialize Inputs
    
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

	printf("\r\n");
    assertSuccess(success);
	printf("\r\n");
	printf("**************************************\r\n");
	printf("* FIR Test done ...                  *\r\n");
	printf("**************************************\r\n");
	printf("\r\n");
    
    printf("Completed\r\n");

    exit(0);
}

