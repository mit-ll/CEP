//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    to be used with openocd
// Notes:          
//
//************************************************************************

#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dpi_bitbang.h"

typedef struct {
    int sockFd;
    int openOcdFd;
    int socket_portId;
    int jtagEncode; // concate of {TRST,TCK,TMS,TDI} // NOTE TRST = active hi
    int jtagTdo;
    int quit;
    int comOn; // is open
} bitbang;

//
// global
//
bitbang jtag_bitbang;
bitbang *jtag = &jtag_bitbang;

static void dpi_bitbang_open(uint16_t port)  
{
    jtag->sockFd = 0;
    jtag->openOcdFd = 0;
        //
    jtag->socket_portId = 0;
    jtag->sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (jtag->sockFd == -1) {
        printf("%s: failed to make socket: %s (%d)\n",__FUNCTION__,strerror(errno), errno);
        abort();
    }
    fcntl(jtag->sockFd, F_SETFL, O_NONBLOCK);
    int reuseaddr = 1;
    if (setsockopt(jtag->sockFd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(int)) == -1) {
        printf("%s: failed setsockopt: %s (%d)\n",__FUNCTION__,strerror(errno), errno);
        abort();
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (::bind(jtag->sockFd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("%s failed to bind socket: %s (%d)\n",__FUNCTION__,strerror(errno), errno);
        abort();
    }
    
    if (listen(jtag->sockFd, 1) == -1) {
        printf("%s failed to listen on socket: %s (%d)\n",__FUNCTION__,strerror(errno), errno);
        abort();
    }
    
    socklen_t addrlen = sizeof(addr);
    if (getsockname(jtag->sockFd, (struct sockaddr *) &addr, &addrlen) == -1) {
        printf("%s getsockname failed: %s (%d)\n",__FUNCTION__,strerror(errno), errno);
        abort();
    }
    jtag->jtagEncode = 0xf;
    jtag->quit  = 0;
    jtag->comOn = 0;    
        // bit[31] = valid
    jtag->socket_portId = (1 << 31) | (int)ntohs(addr.sin_port);
        //
    printf("%s: Listening on port %d\n",__FUNCTION__,jtag->socket_portId & 0xFFFF);
}

static void dpi_bitbang_wait2accept(void)
{
    if (!jtag->comOn) {
        printf("%s: Waiting to accept client socket (openocd)\n",__FUNCTION__);
        int again = 1;
        while (again != 0) {
            jtag->openOcdFd = ::accept(jtag->sockFd, NULL, NULL);
            if (jtag->openOcdFd == -1) {
                if (errno == EAGAIN) {
                        // No client waiting to connect right now.
                } else {
                    printf("%s: failed to accept on socket: %s (%d)\n", __FUNCTION__,strerror(errno), errno);
                    again = 0;
                    abort();
                }
            } else {
                fcntl(jtag->openOcdFd, F_SETFL, O_NONBLOCK);
                printf("%s: Accepted successfully.",__FUNCTION__);
                again = 0;
            }
        }
        jtag->comOn = 1;
    }
}

static void dpi_bitbang_run(void)
{
    char cmd;
    int wrBack = 0;
    char tdoChar = '?';
        //
    int again = 1;
    while (again) {
        ssize_t num_read = read(jtag->openOcdFd, &cmd, sizeof(cmd));
        if (num_read == -1) {
            if (errno != EAGAIN) {
                printf("%s: failed to read on socket: %s (%d)\n",__FUNCTION__,strerror(errno), errno);
                again = 0;
                abort();
            }
        } else if (num_read == 0) {
            printf("%s: No Command Received.\n",__FUNCTION__);
            again = 1;
        } else {
            again = 0;
        }
    }
        // see openocd's bitbang.h 's write interface
        // openocd will send 1 character encoding {tck,tms,tdi} in the 3 LSB bits the character.. "0" <-> "7"
        // will some others
        //
    switch (cmd) {
        case '0': jtag->jtagEncode = 0;break;
        case '1': jtag->jtagEncode = 1;break;
        case '2': jtag->jtagEncode = 2;break;
        case '3': jtag->jtagEncode = 3;break;
        case '4': jtag->jtagEncode = 4;break;
        case '5': jtag->jtagEncode = 5;break;
        case '6': jtag->jtagEncode = 6;break;
        case '7': jtag->jtagEncode = 7;break;
        case 'r': jtag->jtagEncode = 0xf; break;// reset
                //
        case 'R': wrBack = 1; tdoChar = jtag->jtagTdo ? '1' : '0'; break;
        case 'Q': jtag->quit = 1; break;
        case 'B':
        case 'b': break; // do nothing
                //
        default:
            printf("%s: unsupported command '%c'\n",__FUNCTION__,cmd);
    }
    
    if (wrBack){
        ssize_t bytes = write(jtag->openOcdFd, &tdoChar, sizeof(tdoChar));
        if (bytes == -1) {
            printf("%s: failed to write to socket: %s (%d)\n", __FUNCTION__,strerror(errno), errno);
            abort();
        }
    }
        //
    if (jtag->quit) {
        printf("%s: Remote end disconnected\n",__FUNCTION__);
        close(jtag->openOcdFd);
        jtag->openOcdFd = 0;
        jtag->comOn = 0;
    }
}

//
// DPI
//

int jtag_init(void) 
{
    dpi_bitbang_open(0);
    return 0;
}

int jtag_quit(void) {
    jtag->quit = 1;
    return 0;
}

int jtag_getSocketPortId (void) {  return jtag->socket_portId; }

int jtag_cmd(const int tdo_in, int *encode)
{

    if (jtag->openOcdFd > 0) {
        jtag->jtagTdo = tdo_in;
        dpi_bitbang_run();
    } else if (!jtag->quit && !jtag->comOn) {
        dpi_bitbang_wait2accept();
    }
    *encode = jtag->jtagEncode;
    return 0;
}

