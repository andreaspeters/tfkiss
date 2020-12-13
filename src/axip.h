/* config.h    general configuration info
 *
 * Copyright 1991, Michael Westerhof, Sun Microsystems, Inc.
 * This software may be freely used, distributed, or modified, providing
 * this header is not removed.
 * 
 */

/* 
 * Adapted for use with TFKISS and renamed to axip.h 
 * by Mark Wahl, DL4YBG, 960309
 */


#define IPPROTO_AX25 93
#define DEFAULT_UDP_PORT 10093

#define IP_MODE		0x10
#define UDP_MODE	0x20
#define TTY_MODE	0x30

#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif

#ifdef TFKISS_MAIN
#define vextern
#else
#define vextern extern
#endif

vextern int udp_mode;                   /* true if we need a UDP socket */
vextern int ip_mode;                    /* true if we need the raw IP socket */
vextern unsigned short my_udp;          /* the UDP port to use (network byte order) */
vextern unsigned char mycallsign[7];    /* My callsign, shifted ASCII with SSID */
vextern int digi;                       /* True if we are connected to a TNC */
vextern int loglevel;                   /* Verbosity level */

#define MAX_FRAME 2048

#define LOGL1 if(loglevel>0)put_error
#define LOGL2 if(loglevel>1)put_error
#define LOGL3 if(loglevel>2)put_error
#define LOGL4 if(loglevel>3)put_error

/* routing.c */
void route_init();
void route_add();
unsigned char *call_to_ip();
void dump_routes();

/* process.c */
void process_init();
void from_ip();
int from_kiss();
int addrmatch();
unsigned char *next_addr();
void add_crc();
void dump_ax25frame();

void send_ip();
int a_to_call();
char *call_to_a();
unsigned short int compute_crc();
int ok_crc();

