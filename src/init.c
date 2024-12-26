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

#ifdef USE_AXIP
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __NetBSD__
#include <sys/socket.h>
#endif
#include "axip.h"
#endif

/* local function declarations */
int parse_line(char *buf);

extern int use_socket;
extern int tnc_to_kiss;

char tfkiss_initfile[MAXCHAR];
char tfkiss_socket[MAXCHAR];

/* variables filled from init-file */
char device[MAXCHAR];
unsigned int speed;
int speedflag;
int kisstype;
int kiss_active;
int axip_active;
int fulldup_on_dama;
char tfkiss_conf_dir[MAXCHAR];
char tfkiss_log_dir[MAXCHAR];
char tfkiss_run_dir[MAXCHAR];
char tfkiss_lock_dir[MAXCHAR];
char tfkiss_lockfile[MAXCHAR];
char tfkiss_errfile[MAXCHAR];
char tfkiss_parafile[MAXCHAR];
char tfkiss_procfile[MAXCHAR];
char tfkiss_axipconfig[MAXCHAR];

int use_terminal;

/* magic number to verify parameter file (type: unsigned short, 2 byte) */
#define PARAFILE_MAGIC 0x5E5E
#define PARAFILE_MAGICLEN 2
/* number of bytes used for parameters + length of magic number */
#define PARAFILE_LEN 34+PARAFILE_MAGICLEN

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
 

void load_para()
{
  unsigned short magic_nr;
  int parafd;
  char buffer[PARAFILE_LEN];
  int len;
  int i;
  char *bufptr;
  
  if (*tfkiss_parafile == '\0') return;
  parafd = open(tfkiss_parafile,O_RDONLY);
  if (parafd == -1) return;
  len = read(parafd,buffer,PARAFILE_LEN);
  close(parafd);
  if (len != PARAFILE_LEN) return;
  magic_nr = *((unsigned short *)buffer);
  if (magic_nr != PARAFILE_MAGIC) return;
  bufptr = buffer + PARAFILE_MAGICLEN;

  defESC = *bufptr++;
  for (i=0;i<7;i++)
    defIp[i] = *bufptr++;
  defYp = *bufptr++;
  defMp = *bufptr++;
  defRp = *bufptr++;
  defPp = *bufptr++;
  defWp = *bufptr++;
  defTp = *bufptr++;
  defZp = *bufptr++;
  defXp = *bufptr++;
  defAp = *bufptr++;
  defEp = *bufptr++;
  defOp = *bufptr++;
  defNp = *bufptr++;
  defVCp = *bufptr++;
  defDp = *bufptr++;
  defUIp = *bufptr++;
  defxSp = *bufptr++;
  defxFp = *bufptr++;
  defT2p = *((short *)bufptr);
  bufptr += 2;
  defT3p = *((short *)bufptr);
  bufptr += 2;
  defA3p = *bufptr++;
  defFp = *((short *)bufptr);
  bufptr += 2;
  defIPp = *bufptr++;
  defxTAp = *bufptr++;
}

void save_para()
{
  unsigned short magic_nr;
  int parafd;
  
  if (*tfkiss_parafile == '\0') return;
  parafd = creat(tfkiss_parafile,0600);
  if (parafd == -1) return;
  magic_nr = PARAFILE_MAGIC;

  write(parafd,&magic_nr,PARAFILE_MAGICLEN);
  write(parafd,&defESC,1);
  write(parafd,myid,7);
  write(parafd,&Ypar,1);
  write(parafd,&Mpar,1);
  write(parafd,&Rpar,1);
  write(parafd,&Ppar,1);
  write(parafd,&Wpar,1);
  write(parafd,&Tpar,1);
  write(parafd,&Zpar,1);
  write(parafd,&Xpar,1);
  write(parafd,&Apar,1);
  write(parafd,&Epar,1);
  write(parafd,&Opar,1);
  write(parafd,&Npar,1);
  write(parafd,&VCpar,1);
  write(parafd,&Dpar,1);
  write(parafd,&UIpar,1);
  write(parafd,&stamp,1);
  write(parafd,&xFpar,1);
  write(parafd,&T2par,2);
  write(parafd,&T3par,2);
  write(parafd,&A3par,1);
  write(parafd,&Fpar,2);
  write(parafd,&Ipar,1);
  write(parafd,&xTApar,1);
  close(parafd);
}

#ifdef USE_AXIP
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

#endif

static int analyse_value(str1,str2)
char str1[];
char str2[];
{
  int tmp;
  
  if (strcmp(str1,"kiss_active") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    kiss_active = tmp;
    return(0);
  }
  else if (strcmp(str1,"axip_active") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    axip_active = tmp;
    return(0);
  }
  else if (strcmp(str1,"device") == 0) {
    strcpy(device,str2);
    return(0);
  }
  else if (strcmp(str1,"speed") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    speedflag = 0;
    switch (tmp) {
    case 150:
      speed = B150;
      return(0);
    case 300:
      speed = B300;
      return(0);
    case 600:
      speed = B600;
      return(0);
    case 1200:
      speed = B1200;
      return(0);
    case 2400:
      speed = B2400;
      return(0);
    case 4800:
      speed = B4800;
      return(0);
    case 9600:
      speed = B9600;
      return(0);
    case 19200:
      speed = B19200;
      return(0);
    case 38400:
      speed = B38400;
      return(0);
#ifdef USE_HIBAUD
    case 57600:
      speed = B38400;
      speedflag = ASYNC_SPD_HI;
      return(0);
    case 115200:
      speed = B38400;
      speedflag = ASYNC_SPD_VHI;
      return(0);
#endif
    default:
      return(1);
    }
  }
  else if (strcmp(str1,"tfkiss_conf_dir") == 0) {
    strcpy(tfkiss_conf_dir,str2);
    tmp = strlen(tfkiss_conf_dir);
    if (tfkiss_conf_dir[tmp-1] != '/') {
      tfkiss_conf_dir[tmp] = '/';
      tfkiss_conf_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tfkiss_log_dir") == 0) {
    strcpy(tfkiss_log_dir,str2);
    tmp = strlen(tfkiss_log_dir);
    if (tfkiss_log_dir[tmp-1] != '/') {
      tfkiss_log_dir[tmp] = '/';
      tfkiss_log_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tfkiss_run_dir") == 0) {
    strcpy(tfkiss_run_dir,str2);
    tmp = strlen(tfkiss_run_dir);
    if (tfkiss_run_dir[tmp-1] != '/') {
      tfkiss_run_dir[tmp] = '/';
      tfkiss_run_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tfkiss_lock_dir") == 0) {
    strcpy(tfkiss_lock_dir,str2);
    tmp = strlen(tfkiss_lock_dir);
    if (tfkiss_lock_dir[tmp-1] != '/') {
      tfkiss_lock_dir[tmp] = '/';
      tfkiss_lock_dir[tmp+1] = '\0';
    }
    return(0);
  }
  else if (strcmp(str1,"tfkiss_lockfile") == 0) {
    strcpy(tfkiss_lockfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tfkiss_errfile") == 0) {
    strcpy(tfkiss_errfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tfkiss_parafile") == 0) {
    strcpy(tfkiss_parafile,str2);
    return(0);
  }
  else if (strcmp(str1,"tfkiss_procfile") == 0) {
    strcpy(tfkiss_procfile,str2);
    return(0);
  }
  else if (strcmp(str1,"tfkiss_axipconfig") == 0) {
    strcpy(tfkiss_axipconfig,str2);
    return(0);
  }
  else if (strcmp(str1,"tfkiss_socket") == 0) {
    if ((!use_socket) && (!use_terminal)) {
      strcpy(tfkiss_socket,str2);
      use_socket = 1;
    }
    return(0);
  }
  else if (strcmp(str1,"tfkiss_extsocket") == 0) {
    if ((!use_socket) && (!use_terminal)) {
      strcpy(tfkiss_socket,str2);
      use_socket = 2;
    }
    return(0);
  }
  else if (strcmp(str1,"fulldup_on_dama") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    fulldup_on_dama = tmp;
    return(0);
  }
  else if (strcmp(str1,"kisstype") == 0) {
    if (sscanf(str2,"%d",&tmp) != 1) return(1);
    kisstype = tmp;
    if ((kisstype > KISS_RMNC) || (kisstype < KISS_NORMAL)) {
      kisstype = KISS_NORMAL;
    }
    return(0);
  }
  else {
    return(1);
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

int read_init_file(argc,argv,unlock)
int argc;
char *argv[];
int *unlock;
{
  FILE *init_file_fp;
  int file_end;
  int file_corrupt;
  char line[82];
  char str1[82];
  char str2[82];
  char tmp_str[MAXCHAR];
  int rslt;
  int wrong_usage;
  char *str_ptr;
  int scanned;
  int explicit_ini=0;
  int i;
  int reset;

  strcpy(device,DEF_DEVICE);
  speed = DEF_SPEED;
  speedflag = DEF_SPEEDFLAG;
  strcpy(tfkiss_conf_dir,DEF_CONF_DIR);
  strcpy(tfkiss_log_dir,DEF_LOG_DIR);
  strcpy(tfkiss_run_dir,DEF_RUN_DIR);
  strcpy(tfkiss_lock_dir,DEF_LOCK_DIR);
  strcpy(tfkiss_lockfile,DEF_LOCK_FILE);
  strcpy(tfkiss_procfile,DEF_PROC_FILE);
  strcpy(tfkiss_errfile,DEF_ERR_FILE);
  strcpy(tfkiss_parafile,DEF_PARA_FILE);
  strcpy(tfkiss_axipconfig,DEF_AXIPCONFIG);
  kisstype = KISS_NORMAL;
  kiss_active = 1;
  axip_active = 0;
  fulldup_on_dama = DEF_FULLDUP_ON_DAMA;
  tfkiss_socket[0] = '\0';
#ifdef USE_AXIP
  axip_config_init();
#endif

  defESC = DEF_ESC;
  for (i=0;i<6;i++)
    defIp[i] = ' ';
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
  
  strcpy(tfkiss_initfile,INIT_FILE);
  use_socket = 0;
  use_terminal = 0;
  wrong_usage = 0;
  scanned = 1;
  *unlock = 0;
  reset = 0;
  while ((scanned < argc) && (!wrong_usage)) {
    if (strcmp(argv[scanned],"-i") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(tfkiss_initfile,argv[scanned]);
	explicit_ini=1;
      }
      else wrong_usage = 1;
    }
    else if (strcmp(argv[scanned],"-s") == 0) {
      scanned++;
      if (scanned < argc) {
        strcpy(tfkiss_socket,argv[scanned]);
        use_socket = 1;
        if (use_terminal) wrong_usage = 1;
      }
      else wrong_usage = 1;
    }
    else if (strcmp(argv[scanned],"-u") == 0) {
      *unlock = 1;
    }
    else if (strcmp(argv[scanned],"-x") == 0) {
      tnc_to_kiss = 1;
    }
    else if (strcmp(argv[scanned],"-r") == 0) {
      reset = 1;
    }
    else if (strcmp(argv[scanned],"-t") == 0) {
      use_terminal = 1;
      if (use_socket) wrong_usage = 1;
    }
    else {
      wrong_usage = 1;
    }
    scanned++;
  }
  if (wrong_usage) {
    printf("Usage : tfkiss [-i <init-file>] [-s <tfkiss-socket>] [-u] [-r] [-t] [-x]\n");
    printf("        -u  ignore an existing lockfile for the KISS device \n");
    printf("        -r  ignore and delete all parameter stored in tfkiss.para file \n");
    printf("        -t  startup in console-mode \n");
    printf("        -x  switch tnc2 with thefirm from terminal-mode to kiss \n");
    printf ( "\n" ) ;
    return(1);
  }

  if (explicit_ini ==1) {
    if (!(init_file_fp = fopen(tfkiss_initfile,"r"))) {
      printf("Error: explicit configuration file \"%s\" not found. \n",
              tfkiss_initfile);
      return(1);
    }
  }  
  if (!(init_file_fp = fopen(tfkiss_initfile,"r"))) {
    str_ptr = getenv("HOME");
    if (str_ptr != NULL) {
      strcpy(tmp_str,str_ptr);
      strcat(tmp_str,"/");
      strcat(tmp_str,tfkiss_initfile);
      init_file_fp = fopen(tmp_str,"r");
      
    }
    if(init_file_fp == NULL) {    /* Try to find in Installpath (hardcoded) */
      strcpy(tmp_str,DEF_INI_DIR);
      strcat(tmp_str,"/");
      strcat(tmp_str,tfkiss_initfile);
      init_file_fp = fopen(tmp_str,"r");
    }
  }
  if(init_file_fp == NULL) { 
    printf("ERROR: Configuration file \"%s\" not found",tmp_str);
    return(1);
  }
  file_end = 0;
  file_corrupt = 0;
  while (!file_end) {
    if (fgets(line,82,init_file_fp) == NULL) {
      file_end = 1;
    }
    else {
      if (strlen(line) == 82) {
        file_end = 1;
        file_corrupt = 1;
      }
      else {
        if (line[0] != '#') { /* ignore comment-lines */
          rslt = sscanf(line,"%s %s",str1,str2);
          switch (rslt) {
          case EOF: /* ignore blank lines */
            break;
          case 2:
            if (analyse_value(str1,str2)) {
              file_end = 1;
              file_corrupt = 1;
            }
            break;
          default:
            file_end = 1;
            file_corrupt = 1;
            break;
          }
        }
      }
    }
  }
  fclose(init_file_fp);
  if (file_corrupt) {
    if (line == NULL) line[0] = '\0';
    printf("ERROR: %s is in wrong format, wrong line:\n%s\n\n",
           tfkiss_initfile,line);
    return(1);
  }
  else {

    add_dir(tfkiss_lock_dir,tfkiss_lockfile);
    add_dir(tfkiss_log_dir,tfkiss_errfile);
    add_dir(tfkiss_conf_dir,tfkiss_parafile);
    add_dir(tfkiss_run_dir,tfkiss_procfile);
    if (use_socket != 2)
      add_dir(tfkiss_run_dir,tfkiss_socket);

#ifdef USE_AXIP
    if (axip_active) {
      add_dir(tfkiss_conf_dir,tfkiss_axipconfig);
      if (axip_config_read(tfkiss_axipconfig)) return(1);
    }
#endif

    if (!reset)
      load_para();
    return(0);
  }
}

int init_proc()
{
  FILE *fp;
  pid_t pid;
#ifdef DEBUG          
  printf("Init\n");
#endif          

  fp = fopen(tfkiss_procfile,"w+");
  if (fp == NULL) {
    printf("ERROR: Can't create process file\n\n");
    return(1);
  }
  pid = getpid();
  fprintf(fp,"%d",pid);
  fclose(fp);
  return(0);
}

void exit_proc()
{
  unlink(tfkiss_procfile);
}
