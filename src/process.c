/* process.c     Handle processing and routing of AX25 frames
 *
 * Copyright 1991, Michael Westerhof, Sun Microsystems, Inc.
 * This software may be freely used, distributed, or modified, providing
 * this header is not removed.
 *
 * This is the only module that knows about the internal structure of
 * AX25 frames.
 */
/*
 * Adpated for use with TFKISS by Mark Wahl, DL4YBG, 960309
 */

#include "config.h"

#ifdef USE_AXIP
#include "axip.h"
#include <stdio.h>
#include <string.h>

#define IS_LAST(p)      (((*(p+6))&0x01)!=0)
#define NOT_LAST(p)     (((*(p+6))&0x01)==0)
#define REPEATED(p)     (((*(p+6))&0x80)!=0)
#define NOTREPEATED(p)  (((*(p+6))&0x80)==0)
#define IS_ME(p)        addrmatch(p,mycallsign)
#define NOT_ME(p)       (!addrmatch(p,mycallsign))
#define ARE_DIGIS(f)    (((*(f+13))&0x01)==0)
#define NO_DIGIS(f)     (((*(f+13))&0x01)!=0)
#define SETREPEATED(p)  (*(p+6))|=0x80
#define SETLAST(p)      (*(p+6))|=0x01

/*
 * Declaration of external functions
 */
extern void send_kiss(char type,char *buf, int len);
extern void put_error(char *str);

/*
 * Initialize the process variables
 */

void
process_init()
{
}

/*
 * handle a frame given us by the kiss routines.  The buf variable is
 * a pointer to an AX25 frame.  Note that the AX25 frame from kiss does
 * not include the CRC bytes.  These are computed by this routine, and
 * it is expected that the buffer we have has room for the CRC bytes.
 * We will either dump this frame, or send it via the IP interface.
 * 
 * If we are in digi mode, we validate in several ways:
 *   a) we must be the next digi in line to pick up the packet
 *   b) the next site to get the packet (the next listed digi, or
 *      the destination if we are the last digi) must be known to
 *      us via the route table.
 * If we pass validation, we then set the digipeated bit for our entry
 * in the packet, compute the CRC, and send the packet to the IP
 * interface.
 *
 * If we are in tnc mode, we have less work to do.
 *   a) the next site to get the packet (the next listed digi, or
 *      the destination) must be known to us via the route table.
 * If we pass validation, we compute the CRC, and send the packet to
 * the IP interface.
 */

int
from_kiss(buf, l)
unsigned char *buf;
int l;
{
	unsigned char *a, *ipaddr;

	if(l<15){
		LOGL2("from_kiss: dumped - length wrong!");
		return 0;
	}

	if(loglevel>2)dump_ax25frame("from_kiss: ", buf, l);

	if(digi){               /* if we are in digi mode */
		a = next_addr(buf);
		if(NOT_ME(a)){
			return 0;
		}
		if(a==buf){     /* must be a digi */
			return 0;
		}
		SETREPEATED(a);
		a = next_addr(buf);     /* find who gets it after us */
	} else {                /* must be tnc mode */
		a = next_addr(buf);
	}                       /* end of tnc mode */

	ipaddr = call_to_ip(a);
	if(ipaddr==NULL){
		if (digi) return 1;
		return 0;
	}
	/* WARNING -- we assume here that the buffer is large enough to have
	 * two bytes tacked on... This is assured in the current version by
	 * "from_kiss" -- the packet is dumped as too large if there will not
	 * be sufficient space.  Use caution!
	 */
	add_crc(buf, l);
	send_ip(buf, l+2, ipaddr);
	return 1;
}

/*
 * handle a frame given us by the IP routines.  The buf variable is
 * a pointer to an AX25 frame.
 * Note that the frame includes the CRC bytes, which we dump ASAP.
 * We will either dump this frame, or send it via the KISS interface.
 * 
 * If we are in digi mode, we only validate that:
 *   a) we must be the next digi in line to pick up the packet
 * If we pass validation, we then set the digipeated bit for our entry
 * in the packet, and send the packet to the KISS send routine.
 *
 * If we are in tnc mode, we validate pretty well nothing, just like a
 * real TNC...  #define FILTER_TNC will change this.
 * We simply send the packet to the KISS send routine.
 */

void
from_ip(buf, l)
unsigned char *buf;
int l;
{
	int port = 0;
	unsigned char *a;

	if(!ok_crc(buf, l)){
		LOGL2("from_ip: dumped - CRC incorrect!");
		return;
	}
	l = l - 2;      /* dump the blasted CRC */

	if(l<15){
		LOGL2("from_ip: dumped - length wrong!");
		return;
	}

	if(loglevel>2)dump_ax25frame("from_ip: ", buf, l);

	if(digi){               /* if we are in digi mode */
		a = next_addr(buf);
		if(NOT_ME(a)){
			LOGL2("from_ip: (digi) dumped - not for me!");
			return;
		}
		if(a==buf){     /* must be a digi */
			LOGL2("from_ip: (digi) dumped - I am destination!");
			return;
		}
		SETREPEATED(a);
	} else {                /* must be tnc mode */
		a = next_addr(buf);
	}                       /* end of tnc mode */
	send_kiss(port, buf, l);
}

/*
 * return true if the addresses supplied match
 */
int
addrmatch(a,b)
unsigned char *a, *b;
{
	if((*a=='\0') || (*b=='\0'))return 0;

	if((*a++^*b++)&0xfe)return 0;   /* "K" */
	if((*a++^*b++)&0xfe)return 0;   /* "A" */
	if((*a++^*b++)&0xfe)return 0;   /* "9" */
	if((*a++^*b++)&0xfe)return 0;   /* "W" */
	if((*a++^*b++)&0xfe)return 0;   /* "S" */
	if((*a++^*b++)&0xfe)return 0;   /* "B" */
	if((*a++^*b++)&0x1e)return 0;   /* ssid */
	return 1;
}

/*
 * return pointer to the next station to get this packet
 */
unsigned char *
next_addr(f)
unsigned char *f;
{
	unsigned char *a;

/* If no digis, return the destination address */
	if(NO_DIGIS(f))return f;

/* check each digi field.  The first one that hasn't seen it is the one */
	a = f + 7;
	do {
		a += 7;
		if(NOTREPEATED(a))return a;
	}while(NOT_LAST(a));

/* all the digis have seen it.  return the destination address */
	return f;       
}

/*
 * tack on the CRC for the frame.  Note we assume the buffer is long
 * enough to have the two bytes tacked on.
 */
void
add_crc(buf, l)
unsigned char *buf;
int l;
{
	unsigned short int u;

	u = compute_crc(buf, l);
	buf[l] = u&0xff;                /* lsb first */
	buf[l+1] = (u>>8)&0xff;         /* msb next */
}

/*
 * Dump AX25 frame.
 */
void
dump_ax25frame(t, buf, l)
unsigned char *buf;
char *t;
int l;
{
#ifdef DEBUG
	int i;
#endif
	unsigned char *a;
	char tmpstr[256];
	char hlpstr[256];

	(void)sprintf(hlpstr,"%s AX25: (l=%3d)   ", t, l);

	if(l<15){
		(void)printf(tmpstr,"Bogus size...");
		strcat(hlpstr,tmpstr);
		put_error(hlpstr);
		return;
	}

	(void)sprintf(tmpstr,"%s -> ", call_to_a(buf+7));
	strcat(hlpstr,tmpstr);
	(void)sprintf(tmpstr,"%s", call_to_a(buf));
	strcat(hlpstr,tmpstr);

	if(ARE_DIGIS(buf)){
		(void)sprintf(tmpstr," v");
		strcat(hlpstr,tmpstr);
		a = buf+7;
		do{
			a+=7;
			(void)printf(" %s", call_to_a(a));
			strcat(hlpstr,tmpstr);
			if(REPEATED(a))strcat(hlpstr,"*");
		}while(NOT_LAST(a));
	}

	put_error(hlpstr);

#ifdef DEBUG
	for(i=0;i<l;i++)(void)printf("%02x ",buf[i]);
	(void)printf("\n");
#endif

	(void)fflush(stdout);
}
#endif
