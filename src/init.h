/* tfkiss: TNC-emulation for Linux
   Copyright (C) 1995-96 by Mark Wahl
   include file for initialization (init.h)
   created: Mark Wahl DL4YBG 95/09/17
   updated: Mark Wahl DL4YBG 96/03/11
*/

/* files used for init */
#define INIT_FILE "tfkiss.ini"

/* default-values if init-file not found or corrupt */
#define DEF_LOCK_FILE "/usr/spool/uucp/LCK..cua0"
#define DEF_CONF_DIR "./"
#define DEF_LOG_DIR "./"
#define DEF_RUN_DIR "./"
#define DEF_LOCK_DIR "./"
#define DEF_ERR_FILE "tfkiss.err"
#define DEF_PARA_FILE "tfkiss.par"
#define DEF_PROC_FILE "tfkiss.pid"
#define DEF_AXIPCONFIG "tfkiss.cfg"
#define DEF_SPEED B19200
#define DEF_SPEEDFLAG 0
#define DEF_DEVICE "/dev/cua0"
#define DEF_FULLDUP_ON_DAMA 1

/* init variables for TF */
#define DEF_ESC 0x1B
#define DEF_Yp 10
#define DEF_Mp 0
#define DEF_Rp 1
#define DEF_Pp 32
#define DEF_Wp 10
#define DEF_Tp 30
#define DEF_Zp 0x01
#define DEF_Xp 1
#define DEF_Ap 1
#define DEF_Ep 1
#define DEF_Op 2
#define DEF_Np 10
#define DEF_VCp 0
#define DEF_Dp 0
#define DEF_UIp 0
#define DEF_xSp 0
#define DEF_xFp 0
#define DEF_T2p 200
#define DEF_T3p 18000
#define DEF_A3p 2
#define DEF_Fp 500
#define DEF_IPp 60
#define DEF_xTAp 4
