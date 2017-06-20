#include <stdio.h>
#include <stdlib.h>

#include "file.h"

/*Store input from file */
int main(void) {
    char *string="";
    int   size=0;

    printf("-- Extracting File\r\n");
    
    if(rFile(&string, &size, "./test/res"))exit(EXIT_FAILURE);//Extract data from file
    pString(string, size);                                    //Print data from file
    wText(string, size, "./gen/output.txt");                  //Store data in another file

    printf("\r\n-- COMPLETED\r\n");
    return EXIT_SUCCESS;
}

