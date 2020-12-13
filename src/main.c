/* 
   tfkiss: TNC-emulation for Linux
   Copyright (C) 1995-96 by Mark Wahl
   Main procedure (main.c)

   created: Mark Wahl DL4YBG 95/09/17
   updated: Mark Wahl DL4YBG 96/10/05
   updated: Berndt Josef Wulf 99/03/12
   updated: mayer hans, oe1smc - 3.6.1999 
   updated: mayer hans, oe1smc - 10.7.1999
   updated: mayer hans, oe1smc - 15.8.1999
   updated: Berndt Josef Wulf 99/08/26

   Most of the code for AXIP-mode is taken from the axip-package,
   the copyright notice for this code follows below:
 *
 * Copyright 1991, Michael Westerhof, Sun Microsystems, Inc.
 * This software may be freely used, distributed, or modified, providing
 * this header is not removed.
 *

*/
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include "config.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef USE_HIBAUD
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/serial.h>
#endif

#define TFKISS_MAIN
#include "all.h"
#include "tf.h"
#include "l2.h"
#include "tfext.h"
#include "kiss.h"
#include "version.h"

//&&& hb9xar
//&&&#undef unsigned

#ifdef USE_AXIP
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>
#include "axip.h"
#endif

/* external function declarations */
extern void append_crc_16();
extern void append_crc_rmnc();
extern int check_crc_16();
extern int check_crc_rmnc();
extern void exit_proc();
extern void hputs();
extern void hputud();
extern int init_proc();
/*extern int isdigit(); */
extern void l2init();
extern void l2rest();
extern void l2rx();
extern void l2timr();
extern void l2tx();
extern void lx();
extern void lxinit();
extern int read_init_file();
extern void save_para();
extern void sl2par();

/* local function declarations */
void send_kisscmd();
void dotokiss();

#undef LOOK_OVERRUN

/* memory used for buffers */
#define BUFFERSIZE 327680
/* maximum length of a received frame, The Firmware does not accept more! */
#define MAXFRAMELEN 1+72+256
#define CRCLEN 2
#define MAXKISSLEN MAXFRAMELEN+CRCLEN

extern char tfkiss_lockfile[];
extern char tfkiss_errfile[];
extern char tfkiss_socket[];
extern char tfkiss_run_dir[];
extern int kisstype;
extern int kiss_active;
extern int axip_active;

#define HOSTQ_BUFLEN 1024
struct hostqueue {
  int first;
  int last;
  char buffer[HOSTQ_BUFLEN];
  struct hostqueue *next;
};

static struct hostqueue *hostq_root;
static struct hostqueue *hostq_last;
static int hostq_len;

char device[MAXCHAR];
int speed;
int speedflag;
int use_socket;
static int terminated;
static int kisslink;
static int lock;
static int sockfd;
static int consockfd;
static int connected;
static struct termios org_termios;
static struct termios wrk_termios;
static struct termios okbd_termios;
static struct termios nkbd_termios;
static int rx_state;
static short rx_port;
static char rx_buffer[MAXKISSLEN];
static char *rx_bufptr;
static int rx_buflen;
static int tx_kick;
static short tx_port;
static struct timeval tv;
static struct timezone tz;
static struct timeval tv_old;
static struct timezone tz_old;
static char *buffers;

int tnc_to_kiss = 0 ; 

static union {
  struct sockaddr sa;
  struct sockaddr_in si;
  struct sockaddr_un su;
} addr;

#ifdef USE_AXIP
static char helpstr[256];
int udpsock;
int sock;
struct sockaddr_in udpbind;
struct sockaddr_in to;
struct sockaddr_in from;
int fromlen;
#endif

/* defines for rx_state */
#define ST_BEGIN 0
#define ST_PORT 1
#define ST_DATA 2
#define ST_ESC 3

/* special codes used for KISS-protocol */
#define FEND 0xC0
#define FESC 0xDB
#define TFEND 0xDC
#define TFESC 0xDD

#define TTKISS "\033@k\r\n"

static int exit_all();

/* dummy data */
unsigned short p1end;
unsigned short p2strt;

/* warning/error logging */
void put_error(char *str)
{
  FILE *fp;
  
  if (!use_socket) {
    fprintf(stderr,"%s\r\n",str);
  }
  else {
    if (*tfkiss_errfile != '\0') {
      fp = fopen(tfkiss_errfile,"a+");
      if (fp != NULL) {
        fprintf(fp,"%s\n",str);
        fclose(fp);
      }
    }
  }
}

/* dummy procedures */
void DIinc()
{
}

void decEI()
{
}

BOOLEAN iscd()
{
  return(FALSE);
}

BOOLEAN CONled(BOOLEAN dummy)
{
  return(FALSE);
}

BOOLEAN STAled(BOOLEAN dummy)
{
  return(FALSE);
}

void xonctl()
{
}

void pushtx()
{
}

void reset()
{
  terminated = 1;
/*
  put_error("reset forced");
  exit_all();
  exit(1);
*/
}

void kiss()
{
}

/* real procedures */

char *minmem()
{
  return(buffers);
}

char *maxmem()
{
  return(buffers+BUFFERSIZE-1);
}

/* from buildsaddr.c */
struct sockaddr *build_sockaddr(const char *name, int *addrlen)
{

  char *host_name;
  char *serv_name;
  char buf[1024];

  memset((char *) &addr, 0, sizeof(addr));
  *addrlen = 0;

  host_name = strcpy(buf, name);
  serv_name = strchr(buf, ':');
  if (!serv_name) return 0;
  *serv_name++ = 0;
  if (!*host_name || !*serv_name) return 0;

  if (!strcmp(host_name, "local") || !strcmp(host_name, "unix")) {
    addr.su.sun_family = AF_UNIX;
    *addr.su.sun_path = 0;
    if (*serv_name != '/') strcpy(addr.su.sun_path,tfkiss_run_dir);
    strcat(addr.su.sun_path, serv_name);
    *addrlen = sizeof(struct sockaddr_un);
    return &addr.sa;
  }

  addr.si.sin_family = AF_INET;

  if (!strcmp(host_name, "*")) {
    addr.si.sin_addr.s_addr = INADDR_ANY;
  } else if (!strcmp(host_name, "loopback")) {
    addr.si.sin_addr.s_addr = inet_addr("127.0.0.1");
  } else if ((addr.si.sin_addr.s_addr = inet_addr(host_name)) == -1) {
    struct hostent *hp = gethostbyname(host_name);
    endhostent();
    if (!hp) return 0;
    addr.si.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
  }

  if (isdigit(*serv_name & 0xff)) {
    addr.si.sin_port = htons(atoi(serv_name));
  } else {
    struct servent *sp = getservbyname(serv_name, (char *) 0);
    endservent();
    if (!sp) return 0;
    addr.si.sin_port = sp->s_port;
  }

  *addrlen = sizeof(struct sockaddr_in);
  return &addr.sa;
}

void init_console()
{
  tcgetattr(0,&okbd_termios);
  nkbd_termios = okbd_termios;
  nkbd_termios.c_cc[VTIME] = 0;
  nkbd_termios.c_cc[VMIN] = 1;
  nkbd_termios.c_cc[VSTART] = -1;
  nkbd_termios.c_cc[VSTOP] = -1;
  nkbd_termios.c_iflag = 0;
  nkbd_termios.c_iflag |= (IGNBRK|ICRNL);
  nkbd_termios.c_oflag = 0;
  nkbd_termios.c_lflag = 0;
  nkbd_termios.c_cflag |= (CS8|CREAD|CLOCAL);
#ifdef HAVE_CRTSCTS
  nkbd_termios.c_cflag &= ~(CSTOPB|PARENB|PARODD|CRTSCTS|HUPCL); 
#else
  nkbd_termios.c_cflag &= ~(CSTOPB|PARENB|PARODD|HUPCL);
#endif
  tcsetattr(0,TCSADRAIN,&nkbd_termios);
  hostq_root = NULL;
  hostq_last = NULL;
  hostq_len = 0;
}

void exit_console()
{
  struct hostqueue *hostq_ptr;
  
  while (hostq_root != NULL) {
    hostq_ptr = hostq_root;
    hostq_root = hostq_ptr->next;
    free(hostq_ptr);
  }
  hostq_last = NULL;
  hostq_len = 0;
  tcsetattr(0,TCSADRAIN,&okbd_termios);
}

static int init_kisslink(char *serstr,int speed,
                         int speedflag,int unlock)
{
#ifdef USE_HIBAUD
  struct serial_struct ser_io;
#endif
  
  if (unlock) unlink(tfkiss_lockfile);
  if ((lock = open(tfkiss_lockfile,O_CREAT|O_EXCL,0600)) == -1) {
    printf("Error: kisslink port locked by other user"
           " or unable to create lockfile\n");
    return(1);
  }
  if ((kisslink = open(serstr,O_RDWR|O_NONBLOCK)) == -1) {
      printf("Error: can't open kisslink port %s \n", serstr );
      close(lock);
      unlink(tfkiss_lockfile);
      return(1);
  }
#ifdef DEBUG
  else 
      printf ( "successfully opened kisslink port %s \n", serstr );
#endif 
  tcgetattr(kisslink,&org_termios);
#ifdef USE_HIBAUD
  if (speed == B38400) {
    if (ioctl(kisslink,TIOCGSERIAL, &ser_io) < 0) {
      printf("Error: can't get kisslink info\n");
      close(kisslink);
      close(lock);
      unlink(tfkiss_lockfile);
      return(1);
    }
  }
#endif
  wrk_termios = org_termios;
  wrk_termios.c_cc[VTIME] = 0;
  wrk_termios.c_cc[VMIN] = 0;
  wrk_termios.c_iflag = 0;
#ifdef LOOK_OVERRUN
  wrk_termios.c_iflag |= (INPCK|IGNBRK);
#else
  wrk_termios.c_iflag |= IGNBRK;
#endif
  wrk_termios.c_oflag = 0;
  wrk_termios.c_lflag = 0;
  wrk_termios.c_cflag |= (CS8|CREAD|CLOCAL);
#ifdef HAVE_CRTSCTS
  wrk_termios.c_cflag &= ~(CSTOPB|PARENB|PARODD|CRTSCTS|HUPCL);
#else
  wrk_termios.c_cflag &= ~(CSTOPB|PARENB|PARODD|HUPCL);
#endif
  cfsetispeed(&wrk_termios,speed);
  cfsetospeed(&wrk_termios,speed);
#ifdef USE_HIBAUD
  if (speed == B38400) {
    ser_io.flags &= ~ASYNC_SPD_MASK;
    ser_io.flags |= speedflag;
    if (ioctl(kisslink,TIOCSSERIAL, &ser_io) < 0) {
      printf("Error: can't set kisslink info\n");
      tcsetattr(kisslink,TCSADRAIN,&org_termios);
      close(kisslink);
      close(lock);
      unlink(tfkiss_lockfile);
      return(1);
    }
  }
#endif
  tcsetattr(kisslink,TCSADRAIN,&wrk_termios);
  rx_state = ST_BEGIN;
  rx_port = 0;
  tx_kick = 0;
  tx_port = 0;

  dotokiss() ; 

  return(0);
}

static int exit_kisslink()
{
  tcsetattr(kisslink,TCSADRAIN,&org_termios);
  close(lock);
  unlink(tfkiss_lockfile);
  return(0);
}

static int init_axip()
{
#ifdef USE_AXIP

/*
 * The bzero is not strictly required - it simply zeros out the
 * address structure.  Since both to and from are static, they are
 * already clear.
 */
  bzero( (char *)&to, sizeof(struct sockaddr) );
  to.sin_family = AF_INET;

  bzero( (char *)&from, sizeof(struct sockaddr) );
  from.sin_family = AF_INET;

  bzero( (char *)&udpbind, sizeof(struct sockaddr) );
  udpbind.sin_family = AF_INET;

  if (ip_mode) {
    sock = socket(AF_INET,SOCK_RAW,IPPROTO_AX25);
    if (sock < 0) {
      printf("Error: can't open raw socket");
      return(1);
    }
    if (fcntl(sock,F_SETFL,FNDELAY) < 0) {
      printf("Error: can't set non-blocking I/O on raw socket");
      close(sock);
      exit(1);
    }
  }

  if (udp_mode) {
    udpsock = socket(AF_INET,SOCK_DGRAM,0);
    if (udpsock < 0) {
      printf("Error: can't open udp socket");
      return(1);
    }
    if (fcntl(udpsock,F_SETFL,FNDELAY) < 0) {
      printf("Error: can't set non-blocking I/O on UDP socket");
      close(udpsock);
      return(1);
    }
/*
 * Ok, the udp socket is open.  Now express our interest in receiving
 * data destined for a particular socket.
 */
    udpbind.sin_addr.s_addr = INADDR_ANY;
    udpbind.sin_port = my_udp;
    if (bind(udpsock,(struct sockaddr *)&udpbind,sizeof udpbind) < 0){
      printf("Error: can't bind udp socket");
      close(udpsock);
      return(1);
    }
  }
  return(0);
#endif
}

static void exit_axip()
{
#ifdef USE_AXIP
  if (ip_mode) close(sock);
  if (udp_mode) close(udpsock);
#endif
}

static void alloc_hostqbuf()
{
  struct hostqueue *hostq_ptr;
  
  hostq_ptr = (struct hostqueue *) malloc(sizeof(struct hostqueue));
  if (hostq_ptr == NULL) {
    put_error("malloc failed for hostqueue!");
    exit_all();
    exit(1);
  }
  hostq_ptr->first = 0;
  hostq_ptr->last = 0;
  hostq_ptr->next = NULL;
  if (hostq_root == NULL) {
    hostq_root = hostq_ptr;
  }
  else {
    hostq_last->next = hostq_ptr;
    hostq_last = hostq_ptr;
  }
  hostq_last = hostq_ptr;
}

/* put one character in host-queue */
static void puthostq(char ch)
{
  struct hostqueue *hostq_ptr;
  
  if (hostq_root == NULL) {
    alloc_hostqbuf();
  }
  hostq_ptr = hostq_last;
  if (hostq_ptr->last >= HOSTQ_BUFLEN) {
    alloc_hostqbuf();
    hostq_ptr = hostq_last;
  }
  hostq_ptr->buffer[hostq_ptr->last] = ch;
  hostq_ptr->last++;
  hostq_len++;
}

/* get one character out of host-queue */
static char gethostq()
{
  struct hostqueue *hostq_ptr;
  char ch;
  
  if ((hostq_len == 0) || (hostq_root == NULL)) return(0); /* sanity check */
  hostq_ptr = hostq_root;
  ch = hostq_ptr->buffer[hostq_ptr->first];
  hostq_ptr->first++;
  hostq_len--;
  if ((hostq_ptr->first == hostq_ptr->last) ||
      (hostq_ptr->first == HOSTQ_BUFLEN)) {
    if (hostq_ptr->next == NULL) {
      hostq_root = NULL;
      hostq_last = NULL;
    }
    else {
      hostq_root = hostq_ptr->next;
    }
    free(hostq_ptr);
  }
  return(ch);
}

/* txqueue always empty */
BOOLEAN ishput()
{
  return(FALSE);
}

/* send one character to console */
void hputc(char ch)
{
  int res;

  if (use_socket) {
    if (connected) {
      res = write(consockfd,&ch,1);
      if (res == -1) {
        close(consockfd);
        connected = 0;
      }
    }
  }
  else {
    write(1,&ch,1);
  }
}

/* return if character available */
BOOLEAN ishget()
{
  if (hostq_len) return(TRUE);
  else return(FALSE);
}

/* get one character from console */
unsigned short hgetc()
{
  char ch;
  
  ch = gethostq();
  return(ch);
}

/* put received characters in hostbuffer */
static void host_to_queue(char *buffer,int len)
{
  char *bufptr;
  int i;
  
  bufptr = buffer;
  i = 0;
  while (i < len) {
    puthostq(*bufptr);
    i++;
    bufptr++;
  }
}

/* send a command over kisslink */
void send_kisscmd(int cmd,int value)
{
  char tx_buffer[10];
  char *tx_bufptr;
  int len;
  char val2;

  if (!kiss_active) return;
  if (kisstype == KISS_RMNC) return;
  if ((cmd < CMD_TXDELAY) || (cmd > CMD_FULLDUP)) return;
  tx_bufptr = tx_buffer;
  *tx_bufptr++ = FEND;
  *tx_bufptr++ = (char)cmd;
  len = 2;
  val2 = (char)(value & 0xFF);
  switch (val2) {
  case FEND:
    *tx_bufptr++ = FESC;
    *tx_bufptr++ = TFEND;
    len += 2;
    break;
  case FESC:
    *tx_bufptr++ = FESC;
    *tx_bufptr++ = TFESC;
    len += 2;
    break;
  default:
    *tx_bufptr++ = val2;
    len++;
    break;
  }
  *tx_bufptr++ = FEND;
  len++;
  write(kisslink,tx_buffer,len);
}

/* send all frames in txbuffer over kisslink */
static void kissframe_to_tnc()
{
  char tx_buffer[1024];
  char *tx_bufptr;
  int len;
  unsigned short ch1;
  int frame_end;
  int i;
  char tmp_buffer[MAXKISSLEN];
  char *tmp_bufptr;
  int tmp_buflen;
  
  while (tx_kick) {
    tx_kick = 0;
    frame_end = 0;
    tmp_bufptr = tmp_buffer;
    *tmp_bufptr++ = 0x00;
    tmp_buflen = 1;
    while (!frame_end) {
      ch1 = l1get((short)(0x0000 + tx_port * 0x100));
      if (ch1 & 0x8000) {
        frame_end = 1;
        if ((ch1 & 0xFF) != 0x01) {
          tx_kick = 1;
        }
      }
      else {
        *tmp_bufptr++ = (char)(ch1 & 0xFF);
        tmp_buflen++;
      }
    }
#ifdef USE_AXIP
    if (axip_active) {
      if (from_kiss(tmp_buffer + 1,tmp_buflen - 1)) continue;
    }
#endif
    if (!kiss_active) continue;
    switch (kisstype) {
    case KISS_NORMAL:
      *tmp_buffer = 0x00;
      break;
    case KISS_SMACK:
      *tmp_buffer = 0x80;
      append_crc_16(tmp_buffer,&tmp_buflen);
      break;
    case KISS_RMNC:
      *tmp_buffer = 0x20;
      append_crc_rmnc(tmp_buffer,&tmp_buflen);
      break;
    }
    tx_bufptr = tx_buffer;
    *tx_bufptr++ = FEND;
    len = 1;
    tmp_bufptr = tmp_buffer;
    for (i=0;i<tmp_buflen;i++) {
      switch (*tmp_bufptr) {
      case FEND:
        *tx_bufptr++ = FESC;
        *tx_bufptr++ = TFEND;
        len += 2;
        break;
      case FESC:
        *tx_bufptr++ = FESC;
        *tx_bufptr++ = TFESC;
        len += 2;
        break;
      default:
        *tx_bufptr++ = *tmp_bufptr;
        len++;
        break;
      }
      tmp_bufptr++;
    }
    *tx_bufptr++ = FEND;
    len++;
    write(kisslink,tx_buffer,len);
  }
}

/* put received frame into The Firmware buffers */
static void frame_to_l1(char *buffer,int len)
{
  int i;
  
  l1put((short)(0x8001 + rx_port * 0x100));
  for (i=0;i<len;i++) {
    l1put((short)(0x0000+(buffer[i]&0xFF)+rx_port*0x100));
  }
  l1put((short)(0x8000 + rx_port * 0x100));
}

static void frame_valid(char *buffer,int len,int type)
{
  switch (type) {
  case KISS_NORMAL:
    if (len > MAXFRAMELEN)
      put_error("frame too long");
    else
      frame_to_l1(buffer+1,len-1);
    break;
  case KISS_SMACK:
    if (check_crc_16(buffer,&len))
      put_error("SMACK: CRC-error");
    else
      frame_to_l1(buffer+1,len-1);
    break;
  case KISS_RMNC:
    if (check_crc_rmnc(buffer,&len))
      put_error("RMNC: CRC-error");
    else
      frame_to_l1(buffer+1,len-1);
    break;
  }
}

#ifdef USE_AXIP

/* Convert ascii callsign to internal format */
int
a_to_call(text, tcall)
char *text;
unsigned char *tcall;
{
  int i;
  int ssid;
  unsigned char c;

  if (strlen(text) == 0) return -1;

  ssid = 0;
  for (i = 0;i < 6;i++) {
    tcall[i]=(' '<<1);
  }
  tcall[6] = '\0';

  for (i = 0;i < strlen(text);i++) {
    c = text[i];
    if (c == '-') {
      ssid = atoi(&text[i+1]);
      if (ssid > 15) return -1;
      tcall[6] = (ssid << 1);
      return 0;
    }
    if (islower(c)) c = toupper(c);
    if (i > 5) return -1;
    tcall[i] = (c << 1);
  }
  return 0;
}

/* Convert internal callsign to printable format */
char *
call_to_a(tcall)
unsigned char *tcall;
{
  int i;
  int ssid;
  char *tptr;
  static char t[10];

  for(i = 0,tptr = t;i < 6;i++) {
    if (tcall[i]==(' ' << 1)) break;
    *tptr = tcall[i]>>1;
    tptr++;
  }

  ssid = (tcall[6] >> 1) & 0x0f;
  if (ssid > 0) {
    *tptr = '-';
    tptr++;
    if (ssid > 9) {
      *tptr = '1';
      tptr++;
      ssid -= 10;
    }
    *tptr = '0' + ssid;
    tptr++;
  }

  *tptr = '\0';
  return &t[0];
}

/* interface-procedure for axip */
void send_kiss(type,buf,len)
unsigned char type;
unsigned char *buf;
int len;
{
  rx_port = (type & 0x70) >> 4;
  if (len > MAXKISSLEN) {
    put_error("AXIP: received frame too long");
    return;
  }
  *rx_buffer = rx_port;
  memcpy(rx_buffer+1,buf,len);
  frame_valid(rx_buffer,len+1,KISS_NORMAL);
}

/* process an I/O error; return true if a retry is needed */
static int
io_error(oops,mode)
int oops;			/* the error flag; < 0 indicates a problem */
int mode;			/* the fd on which we got the error */
{

  if (oops >= 0) return 0;	/* do we have an error ? */

  if (errno == EINTR) return 1;	/* always retry on writes */
  if (mode == IP_MODE) {
    if (errno == EMSGSIZE) {	/* msg too big, drop it */
      put_error("AXIP: message too big");
      return 0;
    }
    if (errno == ENOBUFS) {	/* congestion; sleep + retry */
      put_error("AXIP: send congestion on raw ip, sleeping and retrying!");
      (void)usleep(100000);
      return 1;
    }
    if (errno == EWOULDBLOCK) {
      put_error("AXIP: send on raw ip would block, sleeping and retrying!");
      (void)usleep(100000);	/* sleep a bit */
      return 1;		/* and retry */
    }
    put_error("AXIP: can't write to raw ip socket");
    return 0;
  } else if(mode == UDP_MODE){
   if (errno == EMSGSIZE) {	/* msg too big, drop it */
     put_error("AXIP: message dropped on udp socket");
     return 0;
   }
   if (errno == ENOBUFS) {	/* congestion; sleep + retry */
     put_error("AXIP: send congestion on udp, sleeping and retrying!");
     (void)usleep(100000);
     return 1;
   }
   if (errno == EWOULDBLOCK) {
     put_error("AXIP: send on udp would block, sleeping and retrying!");
     (void)usleep(100000);	/* sleep a bit */
     return 1;		/* and retry */
   }
   put_error("AXIP: can't write to udp socket");
   return(0);
 }
 return(0); /* unspecified error */
}

/* Send an IP frame */
void
send_ip(buf, l, targetip)
unsigned char *buf;
int l;
unsigned char *targetip;
{
  int n;

  if (l <= 0) return;
  (void)memcpy((char *)&to.sin_addr,targetip,4);
  (void)memcpy((char *)&to.sin_port,&targetip[4],2);
  sprintf(helpstr,"sendipdata to=%s %s %d l=%d",
        (char *)inet_ntoa(to.sin_addr),
        to.sin_port ? "udp" : "ip",
        ntohs(to.sin_port), l);
  LOGL4(helpstr);
  if (to.sin_port) {
    if (udp_mode) {
      do {
        n = sendto(udpsock, buf, l, 0,
                   (struct sockaddr *)&to, sizeof to);
      } while(io_error(n,UDP_MODE));
    }
  } else {
    if (ip_mode) {
      do {
        n = sendto(sock, buf, l, 0,
                   (struct sockaddr *)&to, sizeof to);
      } while(io_error(n,IP_MODE));
    }
  }
}

#endif

/* put data received over kisslink in rxbuffer */
static void framedata_to_queue(char *buffer,int len)
{
  char *bufptr;
  int i;
  char ch;
  char tmpstr[MAXCHAR];
  
  i = 0;
  bufptr = buffer;
  while (i < len) {
    ch = *bufptr;
    switch (rx_state) {
    case ST_BEGIN:
      if (ch == FEND) {
        rx_state = ST_PORT;
      }
      break;
    case ST_PORT:
      /* ignore additional FEND */
      if (ch != FEND) {
        switch (kisstype) {
        case KISS_NORMAL:
          if ((ch & 0x8F) == 0x00) {
            rx_port = (ch & 0x70) >> 4;
          }
          else
            rx_state = ST_BEGIN;
          break;
        case KISS_SMACK:
          if ((ch & 0x8F) == 0x80) {
            rx_port = (ch & 0x70) >> 4;
          }
          else
            rx_state = ST_BEGIN;
          break;
        case KISS_RMNC:
          if ((ch & 0xFF) == 0x20) {
            rx_port = 0;
          }
          else
            rx_state = ST_BEGIN;
          break;
        }
        if (rx_state != ST_BEGIN) {
          /* only port 0 allowed */
          if (rx_port != 0x00) {
            rx_state = ST_BEGIN;
            sprintf(tmpstr,"illegal port received: %d",rx_port);
            put_error(tmpstr);
          }
          else {
            rx_state = ST_DATA;
            rx_bufptr = rx_buffer;
            *rx_bufptr++ = ch;
            rx_buflen = 1;
          }
        }
      }
      break;
    case ST_DATA:
      switch (ch) {
      case FEND:
        frame_valid(rx_buffer,rx_buflen,kisstype);
        rx_state = ST_PORT;
        break;
      case FESC:
        rx_state = ST_ESC;
        break;
      default:
        *rx_bufptr = ch;
        rx_bufptr++;
        rx_buflen++;
        if (rx_buflen > MAXKISSLEN) {
          rx_state = ST_BEGIN;
          put_error("frame too long");
        }
        break;
      }
      break;
    case ST_ESC:
      switch (ch) {
      case TFEND:
        *rx_bufptr = FEND;
        rx_bufptr++;
        rx_buflen++;
        if (rx_buflen > MAXKISSLEN) {
          rx_state = ST_BEGIN;
          put_error("frame too long");
        }
        else
          rx_state = ST_DATA;
        break;
      case TFESC:
        *rx_bufptr = FESC;
        rx_bufptr++;
        rx_buflen++;
        if (rx_buflen > MAXKISSLEN) {
          rx_state = ST_BEGIN;
          put_error("frame too long");
        }
        else
          rx_state = ST_DATA;
        break;
      default:
        rx_state = ST_BEGIN;
        sprintf(tmpstr,"illegal character after FESC: %x",ch);
        put_error(tmpstr);
        break;
      }
      break;  
    }
    bufptr++;
    i++;
  }
}

/* there is something in txbuffer */
void kicktx(char port)
{
  char tmpstr[MAXCHAR];
  
  if (port == 0) {
    tx_kick = 1;
  }
  else {
    sprintf(tmpstr,"kicktx on invalid port: %d",port);
    put_error(tmpstr);
  }
}

/* oe1smc */ 
static void switchback()
{
  char buf[9] ; 

  sprintf ( buf, "%c%c%c%c\n" , (char)255 , (char)255 , (char)0 , (char)0 ) ; 
  if ( tnc_to_kiss ) 
      {
#ifdef DEBUG
	printf ( "trying to switch tnc back to hostmode \n" ) ; 
#endif
        write ( kisslink, buf , strlen ( buf ) ) ; 
	/* printf ( buf ) ; */
      } ; 
  exit_all() ; 
  exit ( 0 ) ; 
}

static void sigterm()
{
  terminated = 1;
  signal(SIGTERM, SIG_IGN);
}

static int exit_all()
{
  free(buffers);
  
  save_para();

  if (use_socket) {
    if (connected) {
      close(consockfd);
      connected = 0;
    }
    close(sockfd);
  }
  else {
    exit_console();
  }

  exit_proc();

  if (axip_active) {
    exit_axip();
  }
  if (kiss_active) {
    if (exit_kisslink()) {
      return(1);
    }
  }
  return(0);
}

void dotokiss(void)
  {
  if ( tnc_to_kiss ) 
      {
#ifdef DEBUG
	printf ( "trying to switch tnc to kiss \n" ) ; 
#endif
        write ( kisslink, TTKISS , strlen ( TTKISS ) ) ; 
	/* printf ( TTKISS ) ; */
      } ; 
  } ; 


int main(int argc,char *argv[])
{
  int len;
  char buffer[1024];
  int unlock;
  socklen_t clilen;
  int servlen;
  struct sockaddr_un serv_addr;
  struct sockaddr_un cli_addr;
  struct sockaddr *saddr;
  fd_set rmask;
  struct timeval timevalue;
  int max_fd;
  int count;

#ifdef USE_AXIP
  int n, hdr_len;
  unsigned char buf[MAX_FRAME];
#ifdef __NetBSD__
  struct ip *ipptr;
#else
  struct iphdr *ipptr; 
#endif
#endif
  umask(0); /* don't filter file-permissions */

  buffers = (char *)malloc(BUFFERSIZE);
  if (buffers == NULL) {
    fprintf(stderr,"malloc for buffers failed\n");
    exit(1);
  }
    
  if (read_init_file(argc,argv,&unlock)) {
    free(buffers);
    exit(1);
  }

  if (axip_active) {
    if (init_axip()) {
      free(buffers);
      exit(1);
    }
  }
  if (kiss_active) {
    if (init_kisslink(device,speed,speedflag,unlock)) {
      free(buffers);
      if (axip_active)
        exit_axip();
      exit(1);
    }
  }
  
  if (use_socket) {
    if (use_socket == 2) {
      saddr = build_sockaddr(tfkiss_socket,&servlen);
      if (!saddr) {
        printf("Invalid definition of socket address\n");
        free(buffers);
        if (axip_active)
          exit_axip();
        if (kiss_active)
          exit_kisslink();
        exit(1);
      }
    }
    else {
      bzero((char *) &serv_addr,sizeof(serv_addr));
      serv_addr.sun_family = AF_UNIX;
      strcpy(serv_addr.sun_path,tfkiss_socket);
      servlen = sizeof(serv_addr);
      saddr = (struct sockaddr *)&serv_addr;
    }
    
    if ((sockfd = socket(saddr->sa_family,SOCK_STREAM,0)) < 0) {
      printf("Can't open stream socket\n");
      free(buffers);
      if (axip_active)
        exit_axip();
      if (kiss_active)
        exit_kisslink();
      exit(1);
    }
    switch (saddr->sa_family) {
    case AF_UNIX:
      unlink(saddr->sa_data);
      break;
    case AF_INET:
      break;
    }

    if (bind(sockfd,saddr,servlen) < 0) {
      printf("Can't bind socket\n");
      free(buffers);
      if (axip_active)
        exit_axip();
      if (kiss_active)
        exit_kisslink();
      exit(1);
    }

    listen(sockfd,5);
    
    /* printf signon message */
    printf(SIG1);
    printf("%d",LINKNMBR);
    printf(SIG2);
    printf(SIG3);
    printf(SIG4);
    printf(SIG5);
    printf(SIG7);
    printf(SIG8);
    printf(SIG_D);
    printf(SIG6);

    if (fork() != 0)
      exit(0);

    if (init_proc()) {
      free(buffers);
      close(sockfd);
      if (axip_active)
        exit_axip();
      if (kiss_active)
        exit_kisslink();
      exit(1);
    }

    close(0);
    close(1);
    close(2);
    chdir("/");
    setsid();
    signal(SIGPIPE, SIG_IGN);
  }
  else {

    if (init_proc()) {
      free(buffers);
      if (axip_active)
        exit_axip();
      if (kiss_active)
        exit_kisslink();
      exit(1);
    }

    init_console();
  } ;

  signal(SIGHUP, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTERM, sigterm);
  signal(SIGUSR1, switchback );  /* oe1smc */ 

  terminated = 0;
  connected = 0;
  magicn = 0;

  gettimeofday(&tv,&tz);
  tv_old = tv;
  tz_old = tz;

  sl2par();
  l2init();
  lxinit();
  if (!use_socket) {
    /* print signon message */
    hputs(SIG1);
    hputud(LINKNMBR);
    hputs(SIG2);
    hputs(SIG3);
    hputs(SIG4);
    hputs(SIG5);
    hputs(SIG7);
    hputs(SIG8);
    hputs(SIG_D);
    hputs(SIG6);
  }
                                                            
  for (;;) {
    if (terminated) {
      break;
    }
    
    max_fd = 0;
    FD_ZERO(&rmask);
    if (!use_socket) {
      FD_SET(0,&rmask);
      max_fd = 1;
    }
    if (axip_active) {
#ifdef USE_AXIP
      if (ip_mode) {
        FD_SET(sock,&rmask);
        if (sock > max_fd - 1)
          max_fd = sock + 1;
      }
      if (udp_mode) {
        FD_SET(udpsock,&rmask);
        if (udpsock > max_fd - 1)
          max_fd = udpsock + 1;
      }
#endif
    }
    if (kiss_active) {
      FD_SET(kisslink,&rmask);
      if (kisslink > max_fd -1)
        max_fd = kisslink + 1;
    }
    if (use_socket) {
      if (!connected) {
        FD_SET(sockfd,&rmask);
        if (sockfd > max_fd - 1) {
          max_fd = sockfd + 1;
        }
      }
      else {
        FD_SET(consockfd,&rmask);
        if (consockfd > max_fd - 1) {
          max_fd = consockfd + 1;
        }
      }
    }

/*&&&hb9xar: 10ms cycle gibt eine relativ hohe CPU Last. 
             Daher auf 100ms erhöht.
    timevalue.tv_usec = 10000; */
    timevalue.tv_usec = 100000;
    timevalue.tv_sec = 0;
    count = select(max_fd,&rmask,(fd_set *) 0,
                   (fd_set *) 0,&timevalue);
    if (count == -1) {
      continue;
    }

    if (!use_socket) {
      if (FD_ISSET(0,&rmask)) {
        if ((len = read(0,buffer,1024))) {
          host_to_queue(buffer,len);
        }
      }
    }
    if (axip_active) {
#ifdef USE_AXIP
      if (udp_mode) {
        if (FD_ISSET(udpsock,&rmask)) {
          fromlen = sizeof from;
          n = recvfrom(udpsock,buf,MAX_FRAME,0,
                       (struct sockaddr *)&from,&fromlen);
          sprintf(helpstr,"udpdata from=%s port=%d l=%d",
                (char *)inet_ntoa(from.sin_addr),
                ntohs(from.sin_port),n);
          LOGL4(helpstr);
          if (n > 0) from_ip(buf,n);
        }
      } /* if udp_mode */

      if (ip_mode) {
        if (FD_ISSET(sock,&rmask)) {
          fromlen = sizeof from;
          n = recvfrom(sock,buf,MAX_FRAME,0,
                       (struct sockaddr *)&from,&fromlen);
#ifdef __NetBSD__
          ipptr = (struct ip *)buf;
          hdr_len = 4 * ipptr->ip_hl;
#else
          ipptr = (struct iphdr *)buf;
          hdr_len = 4 * ipptr->ihl;
#endif
          sprintf(helpstr,"ipdata from=%s l=%d, hl=%d",
                (char *)inet_ntoa(from.sin_addr),n,hdr_len);
          LOGL4(helpstr);
          if (n > hdr_len) from_ip(buf+hdr_len,n-hdr_len);
        }
      } /* if ip_mode */
#endif
    }
    if (kiss_active) {
      if (FD_ISSET(kisslink,&rmask)) {
        if ((len = read(kisslink,buffer,1024))) {
          framedata_to_queue(buffer,len);
        }
      }
    }
    if (use_socket) {
      if (!connected) {
        if (FD_ISSET(sockfd,&rmask)) {
          clilen = sizeof(cli_addr);
          consockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
          if (consockfd < 0) {
            put_error("Accept error");
          }
          else {
            connected = 1;
          }
        }
      }
      else {
        if (FD_ISSET(consockfd,&rmask)) {
          len = read(consockfd,buffer,1024);
          if ((len == -1) || (len == 0)) {
            close(consockfd);
            connected = 0;
          }
          else {
            host_to_queue(buffer,len);
          }
        }
      }
    }

    /* update ticker */
    gettimeofday(&tv,&tz);
    ticks = (((tv.tv_sec - tv_old.tv_sec) * 100) +
             ((tv.tv_usec - tv_old.tv_usec) / 10000));
    
    l2rx();

    newtic = ticks - oldtic;
    oldtic = ticks;
    l2timr(newtic);
                             
    l2tx();
    l2rest();
    lx();
    loops++;
    
    /* any frames to send? */
    if (tx_kick) {
      kissframe_to_tnc();
    }
  }

  if (exit_all())
    exit(1);
  else
    exit(0);
}
