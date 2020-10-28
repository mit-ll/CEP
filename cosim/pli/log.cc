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

#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <string>	       
#include <errno.h>
#include <libgen.h>

// FIXME!!!
int debug_mode = 0;
#include <unistd.h>
#include "VecVal.h"
#include "shIpc.h"
#include "vpp2shMem.h"
#include "veriuser.h"
#include "acc_user.h"

#include "log.h"

// for io_printf thread safe

MY_MUTEX_TYPE  Global_mutex         =  MY_DEF_MUTEX;
MY_MUTEX_TYPE  Global_mutexPrintf   =  MY_DEF_MUTEX;
MY_MUTEX_TYPE  Global_mutexGettime  =  MY_DEF_MUTEX;
MY_MUTEX_TYPE  Global_mutexGetName  =  MY_DEF_MUTEX;

logDescription logTable[] = {	 
    {},
    // header	                time  max    action
    //           mask              inst   num
    {"DEBUG:", 0x00000000,	1, 1, -1, 0, LOG_ACTION_NONE},
    {"INFO:",  0xFFFFFFFF,	1, 1, -1, 0, LOG_ACTION_NONE},
    {"WARN:",  0xFFFFFFFF,	1, 1, -1, 0, LOG_ACTION_NONE},
    {"ERROR:", 0xFFFFFFFF,	1, 1, -1, 0, LOG_ACTION_NONE},
    {"FATAL:", 0xFFFFFFFF,	1, 1, -1, 0, LOG_ACTION_NONE}
    // header	                time  max    action
    //           mask              inst   num
};

#if 1
//
void vpp_debug_mode() {
  debug_mode = tf_getp(1);
}
int vpp_debug() {
  tf_putp(0, debug_mode);
  return(debug_mode);
}

//
// Error logging, global
//
#define MAX_ERROR 5
int error_count = 0;
int errorTS_list[5];
int *errorTS = errorTS_list;

int inc_error() {
  /*
  if (error_count < MAX_ERROR) {
    errorTS_list[error_count] = tf_gettime();
  }
  */
  error_count++;
  return(0);
}

void vpp_getErrorCount() {
   tf_putp(1, error_count);
}
void vpp_setErrorCount() {
  int numErr = tf_getp ( 1 );   
  for (int i=0;i<numErr;i++) {
    inc_error();
  }
}

int getInt ( string &sParam,
	     int &where ) {
    int result = 0;
    
    while ( ( where < sParam.length ()  ) &&
	    ( isdigit ( sParam[where] ) ) ) {
	result = 10*result + ( sParam[where] - '0' );
	where++;
    }
 
    return result;
}


void formatInfo ( string &sParam, 
		  int percentLocation, 
		  char &type, 
		  int &leftJustify, 
		  int &minWidth, 
		  int &precision, 
		  int &endFormatLocation ) {

    int where = percentLocation + 1;

    // If next char is -, then left justify.
    if ( sParam[where] == '-' ) {
	leftJustify = 1;
	where++;
    } else {
	leftJustify = 0;
    }
    
    // If next char is a number, this is a min width dec.
    // No min width -> -1
    if ( isdigit ( sParam[where] ) ) {
	minWidth = getInt ( sParam, where );
	
	// minWidth can't really be 0. 0 means 1. so...
	if ( minWidth == 0 ) {
	    minWidth = 1;
	}
    } else {
	minWidth = -1;
    }

    // If the next thing is a ., then we have a precision. 
    if ( sParam[where] == '.' ) {
	// eat .
	where++;
	precision = getInt ( sParam, where );

	// minWidth should be 6 to match printf
	if ( precision == 0 ) {
	    precision = 6;
	}

    } else {
	precision = 6;
    }

    // OK, the next thing is the type.
    type = sParam[where];

    // Just like VCS, treat x like h
    // Unlike VCS, treat t time d.
    switch ( type ) {
    case 'x' : type = 'h'; break;
    case 't' : type = 'd'; break;
    }

    endFormatLocation = where;
}


void fixString ( string &sFormat, 
		     int leftJustify, 
		     int minWidth ) {
    int ptr;

    // Don't bother unless user specified width
    if ( minWidth == -1 ) {
	return;
    }

    // Ok, now remove leading 0's or spaces, if any.
    ptr = 0;
    while ( ( ptr < sFormat.length () ) &&
	    ( ( sFormat[ptr] == '0' ) ||
	      ( sFormat[ptr] == ' ' ) ) ) {
	ptr++;
    }

    // Leave at least one 0.
    if ( ptr > ( sFormat.length() - 1 ) ) {
	ptr = sFormat.length() - 1;
    }

    //sFormat.remove ( 0, ptr ); g++ v2.7.2
    sFormat.replace (0, ptr, (char *)0, 0);

    // Ok, now justify it if it is shorter then min width
    //printf ( "sFormat.length()=%d\n", sFormat.length() );
    if ( sFormat.length() < minWidth ) {
	if ( leftJustify ) {
	    sFormat = sFormat + string ( ( minWidth - sFormat.length() ), ' ');
	} else {	
	    sFormat = string ( ( minWidth - sFormat.length()), ' ' ) + sFormat;
	}
    }

}

void getFormatParam ( char type, 
		      int leftJustify, 
		      int minWidth, 
		      int precision, 
		      int &curParam, 
		      string &sFormat ) {
    char formatForScanf[10];
    char scanfBuf[128]; // Hope this is big enough...

    switch ( type ) {
    case 'b' : case 'd' : case 'e' : case 'f' : case 'g' :
    case 'h' : case 'o' : case 'r' : case 's' : case 't' :
	if ( curParam > tf_nump ( ) ) {
	    sFormat = "<UNKNOW>";
	    return;
	}
    }

    switch ( type ) {
    case 'b' : case 'd' : case 'h' : case 'o' :
	sFormat = tf_strgetp ( curParam, type );
	curParam++;
	
	fixString ( sFormat, leftJustify, minWidth );
	break;

    case 'e' : case 'f' : case 'g' : case 'r' :
	sprintf ( formatForScanf, "%%%d.%dg", minWidth, precision );
	sprintf ( scanfBuf, formatForScanf, tf_getrealp ( curParam ) );
	curParam++;

	sFormat = scanfBuf;

	fixString ( sFormat, leftJustify, minWidth );
	break;
		    
    case 'm' :
	sFormat = tf_spname ();
	fixString ( sFormat, leftJustify, minWidth );
	break;

    case 's' :
	sFormat = tf_getcstringp ( curParam );

	// tf_getcstringp truncates.  
	// If min width not specified, pad it out 
	if ( minWidth == -1 ) {
	    minWidth = tf_sizep ( curParam ) / 8;
	}	

	curParam++;

	fixString ( sFormat, leftJustify, minWidth );
	break;

    case 'T' :
      //sFormat = tf_strgettime ();
      sprintf(formatForScanf,"%08d",tf_gettime());
      sFormat = formatForScanf;
      fixString ( sFormat, 0, 8 ); // minWidth );
      break;

    case '%' :
	sFormat = META_PERCENT;
	break;

    default :
	sFormat = "??? what does a ";
	sFormat += type;
	sFormat += " do???";
    }

}


void expandFormatedVariables ( string &sParam, int &curParam ) {
    char type;
    int leftJustify;
    int minWidth;
    int precision; 
    int percentLocation;
    int endFormatLocation;
    string fParam;

    percentLocation = sParam.find ( "%" );
    while ( percentLocation != -1 ) {
	
	// Get rest of format information
	formatInfo ( sParam, percentLocation, type, leftJustify, 
		     minWidth, precision, endFormatLocation );

	// Get the param in correct format
	getFormatParam ( type, leftJustify, minWidth, 
			 precision, curParam, fParam );

	// Substitute it in
	sParam.replace ( percentLocation, 
			 (endFormatLocation - percentLocation + 1), 
			 fParam );
	
	percentLocation = sParam.find ( "%" );
    }

}


void convertFormatString ( string &result, int curParam ) {
    int numParams = tf_nump();
    string sParam;
    int metaPercentLocation;

    while ( curParam <= tf_nump() ) {
	
	// If the parameter is null (this is ,,) then print a space
	// else just get it.  This mostly does the right thing, except
	// when you want a variable format string.  Too bad, no known solution.
	if ( tf_typep ( curParam ) == tf_nullparam ) {
	    sParam = " ";
	} else {
	    sParam = tf_strgetp ( curParam, 'd' );
	}
	curParam++;
	
	// Deal with formatted variables i.e. %*
	expandFormatedVariables ( sParam, curParam );
	
	// Add it to result
	result += sParam;
    }

    // OK, get %'s back.  
    // Need two of them 'cause io_printf and fprintf eat them.
    while ( ( metaPercentLocation = result.find ( META_PERCENT ) ) != -1 ) {
	result.replace ( metaPercentLocation, 1, "%%" );
    }

}


int logD (){
    // if 1st param is a number, or with mask to see if print
    if ( ( tf_typep ( 1 ) == tf_readonly  ) ||
	 ( tf_typep ( 1 ) == tf_readwrite ) ) {

      //printf ( "ERROR - logD id=%x mas=%x\n",
      //	 tf_getp ( 1 ), logTable[LOG_TYPE_DEBUG].printMask );

      //if ( ! ( tf_getp ( 1 ) & logTable[LOG_TYPE_DEBUG].printMask ) ){
      //printf ( "exit logD 'cause mask doesn't match.  %x vs %x\n",
      //	     tf_getp ( 1 ), logTable[LOG_TYPE_DEBUG].printMask );
      //return 0;
      //}

	//printf ( "didn't exit logD, mask matches.  %x vs %x\n",
	//	 tf_getp ( 1 ), logTable[LOG_TYPE_DEBUG].printMask );


	my_printLog ( LOG_TYPE_DEBUG, 2 );
	return 0;
    } else { 
	my_printLog ( LOG_TYPE_DEBUG, 1 );
	return 0;
    }
}

void my_io_printf(const char *fmt, ...) {
  //MY_MUTEX_LOCK(&Global_mutex);
  // ==============
  char str[1024];
  va_list  arglist;
  va_start(arglist,fmt);
  vsprintf(str,fmt, arglist);
  io_printf(str);
  va_end(arglist);
  // ==============
  //MY_MUTEX_UNLOCK(&Global_mutex);
}


// return 1 if found
int FoundAThread(MY_THREAD_TYPE tid, int *semaId) {
    int Found = 0;
    int i = 0;
#if 0
    if (tidInitialized) {
	while ((i < MAX_SEMAPHORE) && (!Found)) {
	  if (Gtid[i] == tid) { // found it
	    Found = 1;
	    *semaId = i;
	  }
	    i++;
	}
    }
#endif
    return(Found);
}

//
// *************************************
//
//
void my_io_logI(const char *fmt, ...) {
  //MY_MUTEX_LOCK(&Global_mutex);
  // ==============
  char str[1024];
  // get str
  va_list  arglist;
  va_start(arglist,fmt);
  vsprintf(str,fmt, arglist);
  // 
  print_nameNtime(MY_THREAD_ID(),"INFO:",str);
  va_end(arglist);
  // ==============
  //MY_MUTEX_UNLOCK(&Global_mutex);
}

void my_io_logD(const char *fmt, ...) {
    if (debug_mode) {
	//MY_MUTEX_LOCK(&Global_mutex);
	// ==============
	char str[1024];
	// get str
	va_list  arglist;
	va_start(arglist,fmt);
	vsprintf(str,fmt, arglist);
	// 
	print_nameNtime(MY_THREAD_ID(),"DBG:",str);
	va_end(arglist);
	// ==============
	//MY_MUTEX_UNLOCK(&Global_mutex);
    }
}

void my_io_logW(const char *fmt, ...) {
    //MY_MUTEX_LOCK(&Global_mutex);
    // ==============
    char str[1024];
    // get str
    va_list  arglist;
    va_start(arglist,fmt);
    vsprintf(str,fmt, arglist);
    // 
    print_nameNtime(MY_THREAD_ID(),"WARN:",str);
    va_end(arglist);
    // ==============
    //MY_MUTEX_UNLOCK(&Global_mutex);
}

void my_io_logE(const char *fmt, ...) {
    //MY_MUTEX_LOCK(&Global_mutex);
    inc_error();
    // ==============
    char str[1024];
    // get str
    va_list  arglist;
    va_start(arglist,fmt);
    vsprintf(str,fmt, arglist);
    // 
    print_nameNtime(MY_THREAD_ID(),"ERROR:",str);
    va_end(arglist);
    // ==============
    //MY_MUTEX_UNLOCK(&Global_mutex);
}

void my_io_logF(const char *fmt, ...) {
    //MY_MUTEX_LOCK(&Global_mutex);
    inc_error();
    // ==============
    char str[1024];
    // get str
    va_list  arglist;
    va_start(arglist,fmt);
    vsprintf(str,fmt, arglist);
    // 
    print_nameNtime(MY_THREAD_ID(),"FATAL:",str);
    va_end(arglist);
    // ==============
    //MY_MUTEX_UNLOCK(&Global_mutex);
}




// return string contain time and instance name
int print_nameNtime(MY_THREAD_TYPE tid, const char *prefixStr, char *subfixStr) {
    int semaId;
    char oStr[1024];
    if (FoundAThread(tid,&semaId)) {
	sprintf(oStr,"%s %8d %s %s",prefixStr,tf_igettime(GInstance[semaId]),tf_imipname(GInstance[semaId]),subfixStr);

    } else { // top => MUST be in scope
	sprintf(oStr,"%s %8d %s %s",prefixStr,tf_gettime(),tf_mipname(),subfixStr);
    }
    io_printf(oStr);
    return 0;
}
#endif

void printLog ( int logType, int curParam ) {
    string s;

    // If any bit in mask, print
    if ( ! ( logTable[logType].printMask ) ) {
	//printf ( "exit printLog 'cause mask 0.  %x\n",
	//         logTable[logType].printMask );
	return;
    }

    // Always start with header
    string result = logTable[logType].header;
    //result += "\t";
    result += " ";

    // Next, optional time
    if ( logTable[logType].timeEnabled ) {
      //s = tf_strgettime ();
      char formatForScanf[10];
      sprintf(formatForScanf,"%08d",tf_gettime ());
      s = formatForScanf;
	fixString ( s, 0, 8 ); // right justify, 10 chars wide
	result += s;
	result += " ";
    }

    // Next, optional instance
    if ( logTable[logType].instEnabled ) {
	s = tf_spname ();
	//fixString ( s, 1, 20 ); // left justify, 20 chars wide
	result += s;
	result += " ";
    }
    
    // Now, get the message
    convertFormatString ( result, curParam );

    // Print it
    io_printf ( (char*)result.c_str() );

    // optionally call tf_warning and error
    if ( logTable[logType].msgAction & LOG_ACTION_TF_WARNING ) {
	tf_warning ( (char *)"" );
    }
    if ( logTable[logType].msgAction & LOG_ACTION_TF_ERROR ) {
	tf_error ( (char *)"" );
    }

    // Incrament counter, and see if need to exit.
    logTable[logType].numCalls++;

    // Print a CR to complete it all.
    io_printf ( (char *)"\n" );

    // If STOP or FINISH
    if ( ( logTable[logType].msgAction & LOG_ACTION_STOP ) &&
	 ( logTable[logType].numCalls >= 
	   logTable[logType].maxCalls                    ) ) {
	tf_dostop ();
    } else if ( ( logTable[logType].msgAction & LOG_ACTION_FINISH ) &&
		( logTable[logType].numCalls >= 
		  logTable[logType].maxCalls                      ) ) {
	tf_dofinish ();
    }
}

void my_printLog ( int logType, int curParam ) {
  //MY_MUTEX_LOCK(&Global_mutexPrintf);
  // ==============
  printLog (  logType,  curParam );
  // ==============
  //MY_MUTEX_UNLOCK(&Global_mutexPrintf);
}

int logI (){
    my_printLog ( LOG_TYPE_INFO, 1 );
    return 0;
}

int logW (){
    my_printLog ( LOG_TYPE_WARNING, 1 );
    return 0;
}

int logE (){
  inc_error();
  my_printLog ( LOG_TYPE_ERROR, 1 );
  return 0;
}

int logF (){
  // fill it up and stop
  for (int i=error_count;i<=MAX_ERROR;i++) 
    inc_error();
  my_printLog ( LOG_TYPE_FATAL, 1 );
  return 0;
}

