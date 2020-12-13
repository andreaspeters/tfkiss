/* routing.c    Routing table manipulation routines
 *
 * Copyright 1991, Michael Westerhof, Sun Microsystems, Inc.
 * This software may be freely used, distributed, or modified, providing
 * this header is not removed.
 *
 */

/*
 * Adpated for use with TFKISS by Mark Wahl, DL4YBG, 960131
 */


#include <stdio.h>
#include "config.h"
#ifdef USE_AXIP
#include "axip.h"
#include <sys/types.h>
#include <memory.h>
#include <netinet/in.h>

//&&&#ifdef __NetBSD__
#include <sys/socket.h>
#include <arpa/inet.h>
//&&&#endif

/*
 * Declaration of external functions
 */
extern void put_error(char *str);

/* The routing table structure is not visible outside this module. */

#define TABLE_SIZE 128

struct route_table_entry {
	unsigned char callsign[7];		/* the callsign and ssid */
	unsigned char padcall;			/* always set to zero */
	unsigned char ip_addr[4];		/* the IP address */
	unsigned short udp_port;		/* the port number if udp */
	unsigned char pad1;
	unsigned char pad2;
};

static char helpstr[256];
static char helpstr2[256];

int route_tbl_top;
struct route_table_entry route_tbl[TABLE_SIZE];

struct route_table_entry default_route;

/* Initialize the routing table */
void
route_init()
{
	default_route.callsign[0] = '\0';	/* set to non-zero if valid */
	route_tbl_top = 0;
}

/* Add a new route entry */
void
route_add(ip, call, udpport, default_rt)
unsigned char *ip;
unsigned char *call;
int udpport;
int default_rt;
{
	int i;

	if(ip==NULL) return;

	if(default_rt){
		default_route.callsign[0] = 1;		/* mark valid */
		for(i=1;i<7;i++)default_route.callsign[i] = 0;
		default_route.padcall = 0;
		(void)memcpy(default_route.ip_addr, ip, 4);
		default_route.udp_port = htons(udpport);
		default_route.pad1 = 0;
		default_route.pad2 = 0;
		sprintf(helpstr,"added default route: \t%s\t%s\t%d",
			(char *)inet_ntoa(*(struct in_addr *)default_route.ip_addr),
			default_route.udp_port ? "udp" : "ip",
			ntohs(default_route.udp_port));
		LOGL4(helpstr);
		return;
	}

	if(call==NULL) return;
	if(route_tbl_top >= TABLE_SIZE){
		(void)fprintf(stderr,"Routing table is full; entry ignored.\n");
	}

	for(i=0;i<6;i++)route_tbl[route_tbl_top].callsign[i] = call[i] & 0xfe;
	route_tbl[route_tbl_top].callsign[6] = (call[6] & 0x1e) | 0x60;
	route_tbl[route_tbl_top].padcall = 0;
	(void)memcpy(route_tbl[route_tbl_top].ip_addr, ip, 4);
	route_tbl[route_tbl_top].udp_port = htons(udpport);
	route_tbl[route_tbl_top].pad1 = 0;
	route_tbl[route_tbl_top].pad2 = 0;
	sprintf(helpstr,"added route: %s\t%s\t%s\t%d",
			call_to_a(route_tbl[route_tbl_top].callsign),
			(char *)inet_ntoa(*(struct in_addr *)route_tbl[route_tbl_top].ip_addr),
			route_tbl[route_tbl_top].udp_port ? "udp" : "ip",
			ntohs(route_tbl[route_tbl_top].udp_port));
	LOGL4(helpstr);
	route_tbl_top++;
	return;
}

/*
 * Return an IP address and port number given a callsign.
 * We return a pointer to the address; the port number can be found
 * immediately following the IP address. (UGLY coding; to be fixed later!)
 */

unsigned char *
call_to_ip(call)
unsigned char *call;
{
	int i;
	unsigned char mycall[7];

	if(call==NULL)return NULL;

	for(i=0;i<6;i++)mycall[i] = call[i] & 0xfe;
	mycall[6] = (call[6] & 0x1e) | 0x60;

	sprintf(helpstr,"lookup call %s ",call_to_a(mycall));
	for(i=0;i<route_tbl_top;i++){
		if(addrmatch(mycall,route_tbl[i].callsign)){
			sprintf(helpstr2,"found ip addr %s",
				(char *)inet_ntoa(*(struct in_addr *)route_tbl[i].ip_addr));
			strcat(helpstr,helpstr2);
			LOGL4(helpstr);
			return route_tbl[i].ip_addr;
		}
	}

	if(default_route.callsign[0]){
		sprintf(helpstr2,"failed, using default ip addr %s",
				(char *)inet_ntoa(*(struct in_addr *)default_route.ip_addr));
			strcat(helpstr,helpstr2);
			LOGL4(helpstr);
			return default_route.ip_addr;
	}

	strcat(helpstr,"failed.");
	return NULL;
}

/* print out the list of routes */
void
dump_routes()
{
	int i;

	sprintf(helpstr,"\n%d active routes, %d maximum",route_tbl_top,TABLE_SIZE);
	LOGL1(helpstr);
	for(i=0;i<route_tbl_top;i++){
		sprintf(helpstr2,"  %s\t%s\t%s\t%d",
			call_to_a(route_tbl[i].callsign),
			(char *)inet_ntoa(*(struct in_addr *)route_tbl[i].ip_addr),
			route_tbl[i].udp_port ? "udp" : "ip",
			ntohs(route_tbl[i].udp_port));
		LOGL1(helpstr);
	}
	if(default_route.callsign[0]){
		sprintf(helpstr2,"default route\t%s\t%s\t%d",
			(char *)inet_ntoa(*(struct in_addr *)default_route.ip_addr),
			default_route.udp_port ? "udp" : "ip",
			ntohs(default_route.udp_port));
		LOGL1(helpstr);
	}
	(void)fflush(stdout);
}
#endif

