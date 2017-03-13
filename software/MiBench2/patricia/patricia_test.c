/*
 * patricia_test.c
 *
 * Patricia trie test code.
 *
 * This code is an example of how to use the Patricia trie library for
 * doing longest-prefix matching.  We begin by adding a default
 * route/default node as the head of the Patricia trie.  This will become
 * an initialization functin (pat_init) in the future.  We then read in a
 * set of IPv4 addresses and network masks from "pat_test.txt" and insert
 * them into the Patricia trie.  I haven't _yet_ added example of searching
 * and removing nodes.
 *
 * Compiling the library:
 *     gcc -g -Wall -c patricia.c
 *     ar -r libpatricia.a patricia.o
 *     ranlib libpatricia.a
 *
 * Compiling the test code (or any other file using libpatricia):
 *     gcc -g -Wall -I. -L. -o ptest patricia_test.c -lpatricia
 *
 * Matthew Smart <mcsmart@eecs.umich.edu>
 *
 * Copyright (c) 2000
 * The Regents of the University of Michigan
 * All rights reserved
 *
 * $Id: patricia_test.c,v 1.1.1.1 2000/11/06 19:53:17 mguthaus Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

//#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

//#include <rpc/rpc.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

#include "patricia.h"
#include "../bareBench.h"
#include "input.h"
#include <ctype.h>

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};

int isascii(char c) { return 1;}

unsigned int htonl(unsigned int x)
{
    return x;
}

int
inet_aton(const char *cp, struct in_addr *addr)
{
    register unsigned int val;
    register int base, n;
    register char c;
    unsigned int parts[4];
    register unsigned int *pp = parts;
    
    c = *cp;
    for (;;) {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, isdigit=decimal.
         */
        if (!isdigit(c))
            return (0);
        val = 0; base = 10;
        if (c == '0') {
            c = *++cp;
            if (c == 'x' || c == 'X')
                base = 16, c = *++cp;
            else
                base = 8;
        }
        for (;;) {
            if (isascii(c) && isdigit(c)) {
                val = (val * base) + (c - '0');
                c = *++cp;
            } else if (base == 16 && isascii(c) && isxdigit(c)) {
                val = (val << 4) |
                (c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            } else
                break;
        }
        if (c == '.') {
            /*
             * Internet format:
             *	a.b.c.d
             *	a.b.c	(with c treated as 16 bits)
             *	a.b	(with b treated as 24 bits)
             */
            if (pp >= parts + 3)
                return (0);
            *pp++ = val;
            c = *++cp;
        } else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && (!isascii(c) || !isspace(c)))
        return (0);
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    n = pp - parts + 1;
    switch (n) {
            
        case 0:
            return (0);		/* initial nondigit */
            
        case 1:				/* a -- 32 bits */
            break;
            
        case 2:				/* a.b -- 8.24 bits */
            if ((val > 0xffffff) || (parts[0] > 0xff))
                return (0);
            val |= parts[0] << 24;
            break;
            
        case 3:				/* a.b.c -- 8.8.16 bits */
            if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
                return (0);
            val |= (parts[0] << 24) | (parts[1] << 16);
            break;
            
        case 4:				/* a.b.c.d -- 8.8.8.8 bits */
            if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
                return (0);
            val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
            break;
    }
    if (addr)
        addr->s_addr = htonl(val);
    return (1);
}

struct MyNode {
	int foo;
	double bar;
};

int
main()
{
	struct ptree *phead;
	struct ptree *p,*pfind;
	struct ptree_mask *pm;
	struct in_addr addr;
	unsigned long mask=0xffffffff;
	float time;

	/*
	 * Initialize the Patricia trie by doing the following:
	 *   1. Assign the head pointer a default route/default node
	 *   2. Give it an address of 0.0.0.0 and a mask of 0x00000000
	 *      (matches everything)
	 *   3. Set the bit position (p_b) to 0.
	 *   4. Set the number of masks to 1 (the default one).
	 *   5. Point the head's 'left' and 'right' pointers to itself.
	 * NOTE: This should go into an intialization function.
	 */
	phead = (struct ptree *)malloc(sizeof(struct ptree));
	if (!phead) {
		perror("Allocating p-trie node");
		return(0);
	}
	bzero(phead, sizeof(*phead));
	phead->p_m = (struct ptree_mask *)malloc(
			sizeof(struct ptree_mask));
	if (!phead->p_m) {
		perror("Allocating p-trie mask data");
		return(0);
	}
	bzero(phead->p_m, sizeof(*phead->p_m));
	pm = phead->p_m;
	pm->pm_data = (struct MyNode *)malloc(sizeof(struct MyNode));
	if (!pm->pm_data) {
		perror("Allocating p-trie mask's node data");
		return(0);
	}
	bzero(pm->pm_data, sizeof(*pm->pm_data));
	/*******
	 *
	 * Fill in default route/default node data here.
	 *
	 *******/
	phead->p_mlen = 1;
	phead->p_left = phead->p_right = phead;


	/*
	 * The main loop to insert nodes.
	 */
    fakeFile = input_data;
	while (fakeFile < (input_data + sizeof(input_data)/sizeof(struct input_data_format))) {
		/*
		 * Read in each IP address and mask and convert them to
		 * more usable formats.
		 */
        time = fakeFile->time;
        inet_aton(fakeFile->addr, &addr);
        ++fakeFile;
        
		/*
		 * Create a Patricia trie node to insert.
		 */
		p = (struct ptree *)malloc(sizeof(struct ptree));
		if (!p) {
			perror("Allocating p-trie node");
			return(0);
		}
		bzero(p, sizeof(*p));

		/*
		 * Allocate the mask data.
		 */
		p->p_m = (struct ptree_mask *)malloc(
				sizeof(struct ptree_mask));
		if (!p->p_m) {
			perror("Allocating p-trie mask data");
			return(0);
		}
		bzero(p->p_m, sizeof(*p->p_m));

		/*
		 * Allocate the data for this node.
		 * Replace 'struct MyNode' with whatever you'd like.
		 */
		pm = p->p_m;
		pm->pm_data = (struct MyNode *)malloc(sizeof(struct MyNode));
		if (!pm->pm_data) {
			perror("Allocating p-trie mask's node data");
			return(0);
		}
		bzero(pm->pm_data, sizeof(*pm->pm_data));

		/*
		 * Assign a value to the IP address and mask field for this
		 * node.
		 */
		p->p_key = addr.s_addr;		/* Network-byte order */
		p->p_m->pm_mask = htonl(mask);

		pfind=pat_search(addr.s_addr,phead);
		//printf("%08x %08x %08x\n",p->p_key, addr.s_addr, p->p_m->pm_mask);
		//if(pfind->p_key==(addr.s_addr&pfind->p_m->pm_mask))
		if(pfind->p_key==addr.s_addr)
		{
			printf("%f %08x: ", time, addr.s_addr);
			printf("Found.\n");
		}
		else
		{
			/*
		 	* Insert the node.
		 	* Returns the node it inserted on success, 0 on failure.
		 	*/
			//printf("%08x: ", addr.s_addr);
			//printf("Inserted.\n");
			p = pat_insert(p, phead);
		}
		if (!p) {
			printf("Failed on pat_insert\n");
			return(0);
		}
	}

	return(1);
}
