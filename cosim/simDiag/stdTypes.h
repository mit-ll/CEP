//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: MIT
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <limits.h>



/*
 *  Fixed-size integral types.
 *
 *  Int8   -  8 bits, signed
 *  Int8U  -  8 bits, unsigned
 *  Int16  - 16 bits, signed
 *  Int16U - 16 bits, unsigned
 *  Int32  - 32 bits, signed
 *  Int32U - 32 bits, unsigned
 *  Int64  - 64 bits, signed
 *  Int64U - 64 bits, unsigned
 */

#if SCHAR_MAX == 127
	typedef signed char Int8;
#else
	#error "Unable to define Int8 - no suitable built-in type"
#endif

#if UCHAR_MAX == 255U
	typedef unsigned char Int8U;
#else
	#error "Unable to define Int8U - no suitable built-in type"
#endif

#if SHRT_MAX == 32767
	typedef short int Int16;
#else
	#error "Unable to define Int16 - no suitable built-in type"
#endif

#if USHRT_MAX == 65535U
	typedef unsigned short int Int16U;
#else
	#error "Unable to define Int16U - no suitable built-in type"
#endif

#if LONG_MAX == 2147483647L
	typedef long int Int32;
#elif INT_MAX == 2147483647
	typedef int Int32;
#else
	#error "Unable to define Int32 - no suitable built-in type"
#endif

#if ULONG_MAX == 4294967295UL
	typedef unsigned long int Int32U;
#elif UINT_MAX == 4294967295U
	typedef unsigned int Int32U;
#else
	#error "Unable to define Int32U - no suitable built-in type"
#endif

#if defined(__GNUC__) && INT_MAX == 2147483647
	typedef long long int Int64;
#else
	#error "Unable to define Int64 - no suitable built-in type"
#endif

#if defined(__GNUC__) && UINT_MAX == 4294967295U
	typedef unsigned long long int Int64U;
#else
	#error "Unable to define Int64U - no suitable built-in type"
#endif


//
// for JSPEC
//
//typedef unsigned int u_int32_t;

// For log stuffs
#define TELNET_LOG_LEVEL   0x40
#define DUMP_LOG_LEVEL     0x20
#define INIT_LOG_LEVEL     0x10
#define ACCESS_LOG_LEVEL   0x08
#define MEM_LOG_LEVEL      0x04
#define ENTRY_LOG_LEVEL    0x04
#define DRIVER_LOG_LEVEL   0x04
#define BASE_LOG_LEVEL     0x02
#define TEST_LOG_LEVEL     0x01

#endif

