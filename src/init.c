/* tfkiss: TNC-emulation for Linux
   Copyright (C) 1995-96 by Mark Wahl
   Procedures for initialization (init.c)
   created: Mark Wahl DL4YBG 95/09/17
   updated: Mark Wahl DL4YBG 96/10/05
   updated: mayer hans, oe1smc - 3.6.1999
   updated: mayer hans, oe1smc - 10.7.1999
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <unistd.h>
#include "config.h"

#ifdef USE_HIBAUD
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/serial.h>
#endif

#include "init.h"
#include "kiss.h"
#include "version.h"

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __NetBSD__
#include <sys/socket.h>
#endif
#include "axip.h"

/* local function declarations */
int parse_line(char *buf);

extern int use_socket;
extern int tnc_to_kiss;

char tfkiss_socket[MAXCHAR];

/* variables configured through command-line options */
char device[MAXCHAR];
unsigned int speed;
int speedflag;
int kisstype;
int kiss_active;
int axip_active;
int fulldup_on_dama;
int pakratt232_enable;
int debug;
char bluetooth_mac[MAXCHAR];
int use_bluetooth;
char tfkiss_conf_dir[MAXCHAR];
char tfkiss_log_dir[MAXCHAR];
char tfkiss_run_dir[MAXCHAR];
char tfkiss_errfile[MAXCHAR];
char tfkiss_axipconfig[MAXCHAR];

int use_terminal;
int use_foreground;

/* init variables for TF */
char defESC;
char defIp[7];
char defYp;
char defMp;
char defRp;
char defPp;
char defWp;
char defTp;
char defZp;
char defXp;
char defAp;
char defEp;
char defOp;
char defNp;
char defVCp;
char defDp;
char defUIp;
char defxSp;
char defxFp;
short defT2p;
short defT3p;
char defA3p;
short defFp;
char defIPp;
char defxTAp;

/* real variables for TF */
extern char myid[7];
extern char Ypar;
extern char Mpar;
extern char Rpar;
extern char Ppar;
extern char Wpar;
extern char Tpar;
extern char Zpar;
extern char Xpar;
extern char Apar;
extern char Epar;
extern char Opar;
extern char Npar;
extern char VCpar;
extern char Dpar;
extern char UIpar;
extern char stamp;
extern char xFpar;
extern short T2par;
extern short T3par;
extern char A3par;
extern short Fpar;
extern char Ipar;
extern char xTApar;
 

/* Initialize the config table */
static void
axip_config_init()
{
  int i;

  for(i=0;i<7;i++)mycallsign[i]='\0';
  digi = 0;
  loglevel = 0;
  my_udp = htons(0);
  udp_mode = 0;
  ip_mode = 0;

  route_init();
  process_init();
}

/* Open and read the config file */
static int
axip_config_read(f)
char *f;
{
  FILE *cf;
  char buf[256], cbuf[256];
  int errflag, e, lineno;
  char *fname;

  if (f) fname = f;
  else return(1);

  if ((cf = fopen(fname,"r")) == NULL) {
    (void)printf("Config file %s not found or could not be opened\n",fname);
    return(1);
  }

  errflag = 0;
  lineno = 0;
  while (fgets(buf, 255, cf) != NULL) {
    (void)strcpy(cbuf, buf);
    lineno++;
    if ((e = parse_line(buf)) < 0) {
      (void)printf("Config error at line %d: ",lineno);
      if(e==-1)(void)printf("Missing argument\n");
      else if(e==-2)(void)printf("Bad callsign format\n");
      else if(e==-4)(void)printf("Bad option - tnc/digi\n");
      else if(e==-5)(void)printf("Host not known\n");
      else if(e==-9)(void)printf("Bad option - ip/udp\n");
      else (void)printf("Unknown error\n");
      (void)printf("%s",cbuf);
      errflag++;
    }
  }
  if(errflag)return(1);

  if((udp_mode == 0) && (ip_mode == 0)){
    (void)printf("Must specify ip and/or udp sockets\n");
    return(1);
  }

  if(digi){
    if(mycallsign[0]=='\0'){
      (void)printf("No mycall line in config file\n");
      return(1);
    }
  }
  return(0);
}

/* Process each line from the config file.  The return value is encoded. */
int
parse_line(buf)
char *buf;
{
  char *p, *q;
  unsigned char tcall[7], tip[4];
  struct hostent *he;
  int i,j, uport, dfalt;
  p = strtok(buf, " \t\n\r");

  if(p==NULL)return 0;
  if(*p=='#')return 0;

  if(strcmp(p,"mycall")==0){
    q = strtok(NULL, " \t\n\r");
    if(q==NULL)return -1;
    if(a_to_call(q, mycallsign)!=0)return -2;
    return 0;
  } else if(strcmp(p,"mode")==0){
    q = strtok(NULL, " \t\n\r");
    if(q==NULL)return -1;
    if(strcmp(q,"digi")==0) digi = 1;
    else if(strcmp(q,"tnc")==0) digi = 0;
    else return -4;
    return 0;
  } else if(strcmp(p,"socket")==0){
    q = strtok(NULL, " \t\n\r");
    if(q==NULL)return -1;
    if(strcmp(q,"ip")==0){
      ip_mode = 1;
    }else if(strcmp(q,"udp")==0) {
      udp_mode = 1;
      my_udp = htons(DEFAULT_UDP_PORT);
      q = strtok(NULL, " \t\n\r");
      if(q!=NULL){
        i = atoi(q);
        if(i>0)my_udp = htons(i);
      }
    }else return -9;
  return 0;

  } else if(strcmp(p,"loglevel")==0){
    q = strtok(NULL, " \t\n\r");
    if(q==NULL)return -1;
    loglevel = atoi(q);
    return 0;
  } else if(strcmp(p,"route")==0){
    uport = 0;
    dfalt = 0;
    q = strtok(NULL, " \t\n\r");
    if(q==NULL)return -1;
    if(strcmp(q,"default")==0) dfalt = 1;
    else {
      if(a_to_call(q, tcall)!=0)return -2;
    }

    q = strtok(NULL, " \t\n\r");
    if(q==NULL)return -1;
    he = gethostbyname(q);
    if(he!=NULL){
      (void)memcpy(tip, he->h_addr_list[0], 4);
    } else {        /* maybe user specified a numeric addr? */
      j = inet_addr(q);
      if(j==-1)return -5;     /* if -1, bad deal! */
      (void)memcpy(tip, (char *)&j, 4);
    }

    q = strtok(NULL, " \t\n\r");
    if(q!=NULL){
      if(strcmp(q,"udp")==0){
        uport = DEFAULT_UDP_PORT;

        q = strtok(NULL, " \t\n\r");
        if(q!=NULL){
          i = atoi(q);
          if(i>0)uport = i;
        }
      }
    }

    route_add(tip, tcall, uport, dfalt);
    return 0;
  }
  return -999;
}


static int
parse_speed(value, baud, baudflag)
char *value;
unsigned int *baud;
int *baudflag;
{
  int requested_speed;

  requested_speed = atoi(value);
  *baudflag = 0;
  switch (requested_speed) {
  case 150:
    *baud = B150;
    break;
  case 300:
    *baud = B300;
    break;
  case 600:
    *baud = B600;
    break;
  case 1200:
    *baud = B1200;
    break;
  case 2400:
    *baud = B2400;
    break;
  case 4800:
    *baud = B4800;
    break;
  case 9600:
    *baud = B9600;
    break;
  case 19200:
    *baud = B19200;
    break;
  case 38400:
    *baud = B38400;
    break;
#ifdef USE_HIBAUD
  case 57600:
    *baud = B38400;
    *baudflag = ASYNC_SPD_HI;
    break;
  case 115200:
    *baud = B38400;
    *baudflag = ASYNC_SPD_VHI;
    break;
#endif
  default:
    return(1);
  }
  return(0);
}

static void
set_directory(directory, value)
char *directory;
char *value;
{
  int length;

  strcpy(directory, value);
  length = strlen(directory);
  if (length > 0 && directory[length - 1] != '/') {
    directory[length] = '/';
    directory[length + 1] = '\0';
  }
}

void add_dir(char *dir,char *str)
{
  char temp[MAXCHAR];

  if (str[0] == '\0')
    return;
  if (str[0] != '/') {
    strcpy(temp,dir);
    strcat(temp,str);
    strcpy(str,temp);
  }
}

static void
usage(void)
{
  printf("Usage : tfkiss [OPTIONS]\n\n");
  printf("KISS transport (default: active serial KISS on /dev/cua0 at 19200 baud):\n");
  printf("  -d DEVICE              serial KISS device, e.g. /dev/ttyUSB0\n");
  printf("  -b BAUD                150, 300, 600, 1200, 2400, 4800, 9600, 19200 or 38400\n");
  printf("                          57600 and 115200 are available with high-baud support\n");
  printf("  -k TYPE                KISS framing: 0=normal, 1=SMACK, 2=RMNC\n");
  printf("  --kiss-active 0|1      disable or enable the KISS transport (default: 1)\n");

  printf("  -x                     switch a TheFirmware TNC from terminal mode to KISS\n");
  printf("  --pakratt232           use AEA PK-232 host mode; defaults to 9600 baud\n");
  printf("                          unless an explicit -b BAUD is supplied\n\n");
  printf("Bluetooth:\n");
  printf("  -bt MAC                use Bluetooth RFCOMM device, e.g. 00:11:22:33:44:55\n\n");
  printf("Console and socket interface:\n");
  printf("  -t                     use the local console (Ctrl-C exits, Ctrl-Z suspends)\n");
  printf("  -s PATH                listen on a Unix-domain socket (can be combined with -t)\n");
  printf("  --extsocket ADDRESS    listen on an external socket address (can be combined with -t)\n");
  printf("  -f                     do not daemonize when socket mode is selected\n\n");
  printf("AXIP:\n");
  printf("  --axip-active 0|1      disable or enable AXIP (default: 0)\n");
  printf("  -a FILE                AXIP route configuration; required when AXIP is enabled\n");
  printf("  --fulldup-on-dama 0|1  enable full duplex when DAMA is detected (default: 1)\n\n");
  printf("Paths and logging:\n");
  printf("  -c DIR                 base directory for a relative AXIP configuration file\n");
  printf("  -l DIR                 base directory for a relative error log file\n");
  printf("  -R DIR                 base directory for a relative Unix socket path\n");
  printf("  -e FILE                append socket-mode errors to FILE (default: tfkiss.err)\n\n");
  printf("Other:\n");
  printf("  --debug                enable diagnostic output and keep socket mode in foreground\n");
  printf("  -h, --help             show this help and exit\n");
  printf("  --version              show the tfkiss version and exit\n\n");
  printf("Examples:\n");
  printf("  tfkiss -t -d /dev/ttyUSB0 -b 9600\n");
  printf("  tfkiss -t -bt 00:11:22:33:44:55\n");
  printf("  tfkiss --kiss-active 0 --axip-active 1 -a /etc/tfkiss.cfg\n");
}

static void
print_version(void)
{
  printf("tfkiss %s\n", TFKISS_VERSION);
}

int read_init_file(argc,argv)
int argc;
char *argv[];
{
  int scanned;
  int wrong_usage;
  int speed_set;
  int value;

  strcpy(device,DEF_DEVICE);
  speed = DEF_SPEED;
  speedflag = DEF_SPEEDFLAG;
  strcpy(tfkiss_conf_dir,DEF_CONF_DIR);
  strcpy(tfkiss_log_dir,DEF_LOG_DIR);
  strcpy(tfkiss_run_dir,DEF_RUN_DIR);
  strcpy(tfkiss_errfile,DEF_ERR_FILE);
  strcpy(tfkiss_axipconfig,DEF_AXIPCONFIG);
  kisstype = KISS_NORMAL;
  kiss_active = 1;
  axip_active = 0;
  fulldup_on_dama = DEF_FULLDUP_ON_DAMA;
  strcpy(bluetooth_mac,DEF_BLUETOOTH_MAC);
  use_bluetooth = 0;
  pakratt232_enable = 0;
  debug = 0;
  tfkiss_socket[0] = '\0';
  use_socket = 0;
  use_terminal = 0;
  use_foreground = 0;

  speed_set = 0;
  wrong_usage = 0;
  axip_config_init();

  defESC = DEF_ESC;
  for (scanned=0;scanned<6;scanned++)
    defIp[scanned] = ' ';
  defIp[6] = 0x60;
  defYp = DEF_Yp;
  defMp = DEF_Mp;
  defRp = DEF_Rp;
  defPp = DEF_Pp;
  defWp = DEF_Wp;
  defTp = DEF_Tp;
  defZp = DEF_Zp;
  defXp = DEF_Xp;
  defAp = DEF_Ap;
  defEp = DEF_Ep;
  defOp = DEF_Op;
  defNp = DEF_Np;
  defVCp = DEF_VCp;
  defDp = DEF_Dp;
  defUIp = DEF_UIp;
  defxSp = DEF_xSp;
  defxFp = DEF_xFp;
  defT2p = DEF_T2p;
  defT3p = DEF_T3p;
  defA3p = DEF_A3p;
  defFp = DEF_Fp;
  defIPp = DEF_IPp;
  defxTAp = DEF_xTAp;

  scanned = 1;
  while (scanned < argc && !wrong_usage) {
    if (strcmp(argv[scanned],"-h") == 0 ||
        strcmp(argv[scanned],"--help") == 0) {
      usage();
      exit(0);
    }
    else if (strcmp(argv[scanned],"--version") == 0) {
      print_version();
      exit(0);
    }
    else if (strcmp(argv[scanned],"--debug") == 0) {
      debug = 1;
    }

    else if (strcmp(argv[scanned],"-x") == 0) {
      tnc_to_kiss = 1;
    }
    else if (strcmp(argv[scanned],"-t") == 0) {
      use_terminal = 1;
    }
    else if (strcmp(argv[scanned],"-f") == 0) {
      use_foreground = 1;
    }
    else if (strcmp(argv[scanned],"--pakratt232") == 0) {
      pakratt232_enable = 1;
    }
    else {
      scanned++;
      if (scanned >= argc) {
        wrong_usage = 1;
        break;
      }
      if (strcmp(argv[scanned - 1],"-s") == 0) {
        strcpy(tfkiss_socket,argv[scanned]);
        use_socket = 1;
      }
      else if (strcmp(argv[scanned - 1],"--extsocket") == 0) {
        strcpy(tfkiss_socket,argv[scanned]);
        use_socket = 2;
      }
      else if (strcmp(argv[scanned - 1],"-d") == 0) {
        strcpy(device,argv[scanned]);
      }
      else if (strcmp(argv[scanned - 1],"-b") == 0) {
        if (parse_speed(argv[scanned],&speed,&speedflag))
          wrong_usage = 1;
        else
          speed_set = 1;
      }
      else if (strcmp(argv[scanned - 1],"-k") == 0) {
        kisstype = atoi(argv[scanned]);
        if (kisstype > KISS_RMNC || kisstype < KISS_NORMAL)
          wrong_usage = 1;
      }
      else if (strcmp(argv[scanned - 1],"-c") == 0) {
        set_directory(tfkiss_conf_dir,argv[scanned]);
      }
      else if (strcmp(argv[scanned - 1],"-l") == 0) {
        set_directory(tfkiss_log_dir,argv[scanned]);
      }
      else if (strcmp(argv[scanned - 1],"-R") == 0) {
        set_directory(tfkiss_run_dir,argv[scanned]);
      }
      else if (strcmp(argv[scanned - 1],"-e") == 0) {
        strcpy(tfkiss_errfile,argv[scanned]);
      }
      else if (strcmp(argv[scanned - 1],"-a") == 0) {
        strcpy(tfkiss_axipconfig,argv[scanned]);
      }
      else if (strcmp(argv[scanned - 1],"-bt") == 0) {
        strcpy(bluetooth_mac,argv[scanned]);
        use_bluetooth = 1;
      }
      else if (strcmp(argv[scanned - 1],"--kiss-active") == 0) {
        value = atoi(argv[scanned]);
        if (value != 0 && value != 1)
          wrong_usage = 1;
        else
          kiss_active = value;
      }
      else if (strcmp(argv[scanned - 1],"--axip-active") == 0) {
        value = atoi(argv[scanned]);
        if (value != 0 && value != 1)
          wrong_usage = 1;
        else
          axip_active = value;
      }
      else if (strcmp(argv[scanned - 1],"--fulldup-on-dama") == 0) {
        value = atoi(argv[scanned]);
        if (value != 0 && value != 1)
          wrong_usage = 1;
        else
          fulldup_on_dama = value;
      }
      else {
        wrong_usage = 1;
      }
    }
    scanned++;
  }

  if (wrong_usage) {
    usage();
    return(1);
  }

  if (pakratt232_enable && !speed_set) {
    speed = B9600;
    speedflag = 0;
  }

  add_dir(tfkiss_log_dir,tfkiss_errfile);
  if (use_socket == 1)
    add_dir(tfkiss_run_dir,tfkiss_socket);

  if (axip_active) {
    add_dir(tfkiss_conf_dir,tfkiss_axipconfig);
    if (axip_config_read(tfkiss_axipconfig))
      return(1);
  }
  return(0);
}
