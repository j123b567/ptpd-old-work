#ifndef CONSTANTS_RTSS_H
#define CONSTANTS_RTSS_H

#include "../ptpd.h"

#define SOCKETS_WSA2_RTSS

#define TIMESTAMPING_GET_TIME

#include <limits.h>

/* minimal windows version */
#define _WIN32_WINNT_WS03   0x0502
#define WINVER    _WIN32_WINNT_WS03
#define __INDRVUTL__
#define NOGDI

#include <stdint.h>
#include <io.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

/* RTX include*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <Drvutl.h>
#include <rtnapi.h>
#include <rtapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtxprop.h>
#include <wchar.h>
#include"timescale.h"

#if !defined(PATH_MAX)
#define PATH_MAX MAX_PATH
#endif

/* Microsoft Visual C */
#if defined(_MSC_VER)
 #ifdef  _WIN64
  typedef __int64    ssize_t;
 #else
  typedef _W64 int   ssize_t;
 #endif


//RTX define

  #define SZBUFFER_SIZE 100
  #define PPB_MAX 1000000000
  #define TIMER_UNITS_100ns  100

// not ideal define
 #define snprintf sprintf_s
 #define  printf  RtPrintf
 #define strncpy(d, s, n) strcpy_s(d, n, s)

 #define STDOUT_FILENO 1
 #define STDERR_FILENO 2
 #include "freegetopt/getopt.h"
  

#else
 #include <getopt.h>
#endif

// have to be implemented
#define openlog(a,b,c)
#define fdatasync(a)
#define vsyslog(a,b,c) vfprintf(stderr, b, c)
#define setlinebuf(a)

#define gethostbyname2(a, b) gethostbyname(a)

/* UNIX compatibility */
#define LOG_EMERG	0
#define LOG_ALERT	1
#define LOG_CRIT	2
#define LOG_ERR		3
#define LOG_WARNING	4
#define LOG_NOTICE	5
#define LOG_INFO	6
#define LOG_DEBUG	7

#define LOG_USER    0

/* findIface*/
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#undef TRUE
#undef FALSE

#define IFACE_NAME_LENGTH         64
#define NET_ADDRESS_LENGTH        16

#define IFCONF_LENGTH             10

#define MAXHOSTNAMELEN            256

# if BYTE_ORDER == LITTLE_ENDIAN
#   define PTPD_LSBF
# elif BYTE_ORDER == BIG_ENDIAN
#   define PTPD_MSBF
# endif

extern HANDLE TimerSelectInterrupt;

#define ETHER_ADDR_LEN          6
struct  ether_addr {
	unsigned char octet[ETHER_ADDR_LEN];
};
int ether_ntohost(char *hostname, const struct ether_addr *e);


#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4706)

#endif



