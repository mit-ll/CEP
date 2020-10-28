//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

#ifndef __VECVAL__
#define __VECVAL__

using namespace std;
#include <string.h>

#ifndef TF_NULLPARAM
extern "C" {
#include "veriuser.h"
}
#endif

typedef unsigned int uint32;

class VecVal {
public:
    VecVal ( ) {
	width = 0;
	words = 0;
	data = 0;
    };

    VecVal ( const int Width ) {
	width = Width;
	words = ( (width-1) / 32 ) + 1; 
	data = new s_vecval [words];
    };


    VecVal ( const int Width, p_vecval inData ) {
	int i;
	width = Width;
	words = ( (width-1) / 32 ) + 1; 
	data = new s_vecval [words];

	for (i=0; i<words; i++) {
	    data[i].avalbits = inData[i].avalbits;
	    data[i].bvalbits = inData[i].bvalbits;
	}
    };

    VecVal ( const VecVal &inVecVal ) {
	int i;
	width = inVecVal.width;
	words = inVecVal.words;
	data = new s_vecval [words];

	for (i=0; i<words; i++) {
	    data[i].avalbits = inVecVal.data[i].avalbits;
	    data[i].bvalbits = inVecVal.data[i].bvalbits;
	}
    };

    ~VecVal () {
	delete [] data;
    };

    VecVal& operator= ( const VecVal &right ) {
	int i;

	// If different number or words, delete data
	if ( ( data                 ) &&
	     ( right.words != words ) ) {
	    delete [] data;
	    data = 0;
	}

	width = right.width;
	words = right.words;

	// Create space
	if ( ! data ) {
	    data = new  s_vecval [words];
	}

	for (i=0; i<words; i++) {
	    data[i].avalbits = right.data[i].avalbits;
	    data[i].bvalbits = right.data[i].bvalbits;
	}

	return *this;
    };
    
    VecVal& operator= ( const int &right ) {
	// If different number or words, delete data
	if ( ( data       ) &&
	     ( words != 1 ) ) {
	    delete [] data;
	    data = 0;
	}

	width = 32;
	words = 1;

	// Create space
	if ( ! data ) {
	    data = new  s_vecval [words];
	}

	data[0].avalbits = right;
	data[0].bvalbits = 0;

	return *this;
    };
    
    VecVal& operator= ( const long long &right ) {
	// If different number or words, delete data
	if ( ( data       ) &&
	     ( words != 1 ) ) {
	    delete [] data;
	    data = 0;
	}

	width = 64;
	words = 2;

	// Create space
	if ( ! data ) {
	    data = new  s_vecval [words];
	}

	data[0].avalbits = (int)right;
	data[0].bvalbits = 0;
	data[1].avalbits = (int)(right>>32);
	data[1].bvalbits = 0;

	return *this;
    };
    
    VecVal& operator= ( const long long unsigned int &right ) {
	return operator= ( (const long long) right );
    }

    VecVal& operator= ( const char *s ) {
	int charsToCopy;
	int i;
	int word;

	// Zero it out
	set0 ();

	// Copy min of ( size of VecVal, size of char * )
	if ( strlen ( s ) < (uint32)width/8 ) {
	    charsToCopy = strlen ( s );
	} else {
	    charsToCopy = width/8;
	}

	// Copy it over.  
	// Note:  May copy too much
	word = 0;
	i = charsToCopy-1;
	while ( i >=0 ) {
	    *( ((char*)&data[word].avalbits) + 3 ) = s[i--];
	    *( ((char*)&data[word].avalbits) + 2 ) = s[i--];
	    *( ((char*)&data[word].avalbits) + 1 ) = s[i--];
	    *( ((char*)&data[word].avalbits) + 0 ) = s[i--];
	    word++;
	}

	return *this;
    };

    VecVal& operator= ( p_vecval inData ) {
	int i;

	for (i=0; i<words; i++) {
	    data[i].avalbits = inData[i].avalbits;
	    data[i].bvalbits = inData[i].bvalbits;
	}

	return *this;
    }
    
    void set0 ( ) {
	int i;

	for (i=0; i<words; i++) {
	    data[i].avalbits = 0;
	    data[i].bvalbits = 0;
	}
    }

    void setX ( ) {
	int i;

	for (i=0; i<words; i++) {
	    data[i].avalbits = 0xFFFFFFFF;
	    data[i].bvalbits = 0xFFFFFFFF;
	}
    }

    void set ( p_vecval outData ) {
	int i;

	for (i=0; i<words; i++) {
	    outData[i].avalbits = data[i].avalbits;
	    outData[i].bvalbits = data[i].bvalbits;
	}

    };


    int ascii2hex ( const char c, const int mode ) {

	switch ( mode ) {
	case 0: // simple/normal

	    switch ( c ) {
	    case '0' : return 0x0; break;
	    case '1' : return 0x1; break;
	    case '2' : return 0x2; break;
	    case '3' : return 0x3; break;
	    case '4' : return 0x4; break;
	    case '5' : return 0x5; break;
	    case '6' : return 0x6; break;
	    case '7' : return 0x7; break;
	    case '8' : return 0x8; break;
	    case '9' : return 0x9; break;
	    case 'a' : return 0xa; break;
	    case 'b' : return 0xb; break;
	    case 'c' : return 0xc; break;
	    case 'd' : return 0xd; break;
	    case 'e' : return 0xe; break;
	    case 'f' : return 0xf; break;
			      
	    case 'x' : return 0xf; break;
	    case 'z' : return 0x0; break;

	    default: {
	      tf_warning ( (char *)"Illegal character in hex string=%c", c );
		return (char) 0x0;
	    }

	    }

	case 1: // data type/xz/abnormal:

	    switch ( c ) {
	    case '0' : 
	    case '1' : 
	    case '2' : 
	    case '3' : 
	    case '4' : 
	    case '5' : 
	    case '6' : 
	    case '7' : 
	    case '8' : 
	    case '9' : 
	    case 'a' : 
	    case 'b' : 
	    case 'c' : 
	    case 'd' : 
	    case 'e' : 
	    case 'f' : return 0x0; break;
			      
	    case 'x' : 	      
	    case 'z' : return 0xf; break;

	    default: {
		tf_warning ( (char *)"Illegal xz character in hex string=%c", c );
		return (char) 0x0;
	    }

	    }

	default: {
		tf_warning ( (char *)"Illegal mode=%c", mode );
		return (char) 0x0;
	    }	
	}
    }

    // Converts a string to number.
    // Start point is right most char, 
    // Done when either 32 bits full or at end of string (id=0).
    //

    // Base	digits 	limit
    // --------------------------
    // 2	32 	2^32 = 4,294,967,296
    // 3	20	3^20 = 3,486,784,401
    // 4	16	2^32 = 4,294,967,296  (4^16=2^4^4=2^32)
    // 5	13	5^13 = 1,220,703,125
    // 6	12	6^12 = 2,176,782,336
    // 7	11	7^11 = 1,977,326,743
    // 8	10 	2^30 = 1,073,741,824  (8^10=2^3^10=2^30)
    // 9	10	9^10 = 3,486,784,401
    // 10	 9	10^9 = 1,000,000,000	
    // 11	 9	11^9 = 2,357,947,691   
    // 12	 8	12^8 =   429,981,696
    // 13	 8	13^8 =   815,730,721
    // 14	 8	14^8 = 1,475,789,056
    // 15	 8	15^8 = 2,562,890,625
    // 16	 8	2^32 = 4,294,967,296  (16^8=2^4^8=2^32)

    unsigned int ascii2int ( const string &s, // The string constant
			     int &index,      // points to n'th digit
			     const int base,  // Base of the number
			     const int mode ) { // 0-normal, 1-xz

	unsigned int result=0;

	if ( ( base > 16 ) || ( base < 2 ) ) {
	    tf_error ( (char *)"base=%d is out of range, should be 2-16", base );
	}

	int digits;
	int mask;

	switch ( base ) {
	// Common bases first
	case 16:	digits = 8;	mask = 0xf;	break;	   
	case 10:	digits = 9;	mask = 0xf;	break;	   
	case 2:		digits = 32;	mask = 0x1;	break;	   
	// Bogus bases last
	case 3:		digits = 20;	mask = 0x3;	break;	   
	case 4:		digits = 16;	mask = 0x3;	break;	   
	case 5:		digits = 13;	mask = 0x7;	break;	   
	case 6:		digits = 12;	mask = 0x7;	break;	   
	case 7:		digits = 11;	mask = 0x7;	break;	   
	case 8:		digits = 10;	mask = 0x7;	break;	   
	case 9:		digits = 10;	mask = 0xf;	break;	   
	case 11: 	digits = 8;	mask = 0xf;	break;	   
	case 12:	digits = 8;	mask = 0xf;	break;	   
	case 13:	digits = 8;	mask = 0xf;	break;	   
	case 14:	digits = 8;	mask = 0xf;	break;	   
	case 15:	digits = 8;	mask = 0xf;	break;	   
	default : {
	    tf_error ( (char *)"base=%d is out of range, should be 2-16", base );
	}
	}

	int mult = 1;
	while ( ( digits > 0 ) && ( index >= 0 ) ) {
	    result = result + ( ( ascii2hex (s[index], mode) & mask ) * mult );
	    digits--;
	    index--;
	    mult *= base;
	}

	return result;
    }



    // Supports these forms:
    // 
    //  form		result	Removes		Notes
    // ---------------------------------------------
    //  <n*>'<b><n+>	b	<n*>'<b>	b=d, o, b, h, or x
    //  0x		16	Ox
    //  All others	10	Nothing

    // Note: could support octal C format 0[0-7]* but might not be
    //       what folks expect.

    int findBase ( string &s ) {
	int location;
	int base;

	location = s.find ( "'" );
	if ( location != -1 ) { // Verilog form

	    // figure out base
	    switch ( s[location+1] ) {
		case 'b' : base = 2; break;
		case 'd' : base = 10; break;
		case 'h' : base = 16; break;
		case 'o' : base = 8; break;
		case 'x' : base = 16; break;
	    default: {
		    base = 10;
		    tf_error ( (char *)"Ill-formed number=%s", s.c_str () );
		    break;
		}
	    }

	    // Remove from string
	    s.replace (0, location+2, (char *)0, 0);
	} else { 

	    location = s.find ( "0x" );
	    if ( location != -1 ) { // C from for hex
		base = 16;
		// Remove from string
		s.replace (0, location+2, (char *)0, 0);
	    } else { // Decimal form
		base = 10;
	    }
	}

	return base;
    }

    // Input is string constant in hex format.
    // Format is [[<size>]'h][0-9A-Fa-F]*
    //        OR [0[xX]][0-9A-Fa-F]*
    // Size is ignored.  
    // 'h and 0x prefaces are ignored.
    // White space and '_' are ignored
    // In fact, everything to left of h and 0x and 0X is chucked.

    // NOTE: doesn't not support negative numbers.
    void setHex ( const char *S ) {
	string s (S);
	int location;
	int locationCopy;
	int base;

	// Zero it out
	set0 ();

	// Remove whitespace and _'s
	// lower case string
	for ( location=s.size()-1; location>=0; location-- ) {
	    if ( ( isspace ( s[location] ) ) ||
		 ( s[location] == '_'      ) ) {
		//s.erase ( location, 1 ); g++ v2.8.1
		//s.remove ( location, 1 ); g++ v2.7.2
	      s.replace (location, 1, (char *)0, 0);
	    } else if ( isupper ( s[location] ) ) {
		s[location] = s[location] - ( 'A' - 'a' );
	    }
	}

	// Determine base
	base = findBase ( s );

	// Now do it.
	location = s.size() - 1;
	int word = 0;
	//printf ( "s=%s, s.size=%d, location=%d", s.c_str (), s.size(), location );
	while ( location >= 0 ) {
	    if ( word > words ) {
		tf_warning ( (char *)"Truncating string=%s, width=%d", S, width );
		break;
	    }

	    //assert ( word < words );
	    locationCopy = location;
	    data[word].avalbits	= ascii2int ( s, locationCopy, base, 0 );
	    data[word].bvalbits	= ascii2int ( s, location, base, 1 );

	    word++;
	}
	    
    };
  
    // returns bottom 32 bits ignoring x/z's
    int getInt ( ) {
	return data[0].avalbits;
    }
  
    int getWidth ( ) {
	return width;
    }
    
    int getWords ( ) {
	return words;
    }
    
    void print ( FILE *logFilePtr=stdout ) {
	int i;
	for (i=words-1; i>=0; i--) {
	    if ( logFilePtr == (FILE*)-1 ) {
	      io_printf ( (char *)"%08X_", data[i].avalbits );
	    } else {
		fprintf ( logFilePtr, "%08X_", data[i].avalbits );
	    }
	}
    };

    void printB ( FILE *logFilePtr=stdout ) {
	int i;
	for (i=words-1; i>=0; i--) {
	    fprintf ( logFilePtr, "%08X_", data[i].bvalbits );
	}
    };


    // May not be exactly right for mix of Z and 2 state
    char hex2ascii ( const int aval, const int bval ) {
	char h2a[] = { '0', '1', '2', '3', '4', '5', '6', '7', 
		       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	// If 2 state, easy
	if ( bval == 0 ) {
	    return h2a[aval];
	}

	// If bval=F and aVal=0, -> Z
	if ( ( bval == 0xF ) &&
	     ( aval == 0x0 ) ) {
	    return 'Z';
	}

	// if pure X, return X
	if ( ( bval == 0xF ) &&
	     ( aval == 0xF ) ) {
	    return 'X';
	}

	// else, mis-mash, return x
	return 'x';
    }


    // May not be exactly right for mix of Z and 2 state
    char bin2ascii ( const int aval, const int bval ) {
	char b2a[] = { '0', '1', 'Z', 'X' };

	return b2a[ (bval<<1) | aval ];
    }

    // Returns Ve3cVal as 4 state Hex
    void getAsciiHex ( string &s ) {

	s = "";

	int i;
	for ( i=0; i<(words-1); i++ ) {
	    int shift;
	    for ( shift=0; shift<32; shift+=4 ) {
		s = hex2ascii ( ( ( data[i].avalbits >> shift ) & 0xF ), 
				( ( data[i].bvalbits >> shift ) & 0xF ) ) + s;
	    }
	}

	// Last word, only do some...
	uint32 widthMod32 = ( ( width - 1 ) % 32 ) + 1;
	uint32 shift;
	for ( shift=0; shift<widthMod32; shift+=4 ) {
	    s = hex2ascii ( ( ( data[i].avalbits >> shift ) & 0xF ), 
			    ( ( data[i].bvalbits >> shift ) & 0xF ) ) + s;
	}

    }

    // Returns VecVal as 4 state Binary
    void getAsciiBin ( string &s ) {

	s = "";

	int i;
	for ( i=0; i<(words-1); i++ ) {
	    int shift;
	    for ( shift=0; shift<32; shift+=1 ) {
		s = hex2ascii ( ( ( data[i].avalbits >> shift ) & 1 ), 
				( ( data[i].bvalbits >> shift ) & 1 ) ) + s;
	    }
	}

	// Last word, only do some...
	uint32 widthMod32 = ( ( width - 1 ) % 32 ) + 1;
	uint32 shift;
	for ( shift=0; shift<widthMod32; shift+=1 ) {
	    s = hex2ascii ( ( ( data[i].avalbits >> shift ) & 1 ), 
			    ( ( data[i].bvalbits >> shift ) & 1 ) ) + s;
	}

    }

    // Returns vecval as a string.  Ignores bvalbits.
    void getAsciiString ( string &s ) {

	s = "";

	int i;
	for ( i=0; i<(words-1); i++ ) {
	    int shift;
	    for ( shift=0; shift<32; shift+=8 ) {
		s = (char) ( ( data[i].avalbits >> shift ) & 0xFF ) + s;
	    }
	}

	// Last word, only do some...
	uint32 widthMod32 = ( ( width - 1 ) % 32 ) + 1;
	int done = 0;
	uint32 shift;
	for ( shift=0; (shift<widthMod32 && !done ); shift+=4 ) {
	    int shift;
	    for ( shift=0; (shift<32 && !done); shift+=8 ) {
		char c = (char) ( ( data[i].avalbits >> shift ) & 0xFF );
		if ( c  ) {
		    s = c + s;
		} else {
		    done = 1;
		}
	    }
	}

    }


    int width;
    int words;
    p_vecval data;
};

#endif
