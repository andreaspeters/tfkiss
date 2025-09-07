/*
 * definitions of startup messages
 *
 *  updated: mayer hans, oe1smc - 10.7.1999
 *  updated: oe1smc - 8/99 version 1.2 
 *  updated: Berndt Josef Wulf, VK5ABN - 26.8.1999
 */

#define VERSION   "TF2.7b/Linux 05Oct96 Copyright (C) by NORD><LINK e.V. 1988-96, DL4YBG 1995/96"
#ifdef USE_AXIP
# ifdef USE_XPID
#  ifdef USE_FLEXDEC
#define SIG1 "\015\012    TheFirmware (Linux/AXIP/XPID/FLEXDEC) Version 2.7b DAMA ("
#  else
#define SIG1 "\015\012        TheFirmware (Linux/AXIP/XPID) Version 2.7b DAMA ("
#  endif
# else
#  ifdef USE_FLEXDEC
#define SIG1 "\015\012      TheFirmware (Linux/AXIP/FLEXDEC) Version 2.7b DAMA ("
#  else
#define SIG1 "\015\012           TheFirmware (Linux/AXIP) Version 2.7b DAMA ("
#  endif
# endif
#else
# ifdef USE_XPID
#  ifdef USE_FLEXDEC
#define SIG1 "\015\012      TheFirmware (Linux/XPID/FLEXDEC) Version 2.7b DAMA ("
#  else
#define SIG1 "\015\012           TheFirmware (Linux/XPID) Version 2.7b DAMA ("
#  endif
# else
#  ifdef USE_FLEXDEC
#define SIG1 "\015\012         TheFirmware (Linux/FLEXDEC) Version 2.7b DAMA ("
#  else
#define SIG1 "\015\012             TheFirmware (Linux) Version 2.7b DAMA ("
#  endif
# endif
#endif
#define SIG2 " Channel)\015\012            Copyright (c) 1988-1996 by NORD><LINK e.V. (05Oct96)\015\012"
#define SIG3 "   -*- Es gilt die allgemeine Lizenz fuer Amateurfunk Software (ALAS) -*-\015\012"
#define SIG4 "                     ONLY for non-commercial usage\015\012"
#define SIG5 "         Linux-porting copyright (c) 1995-1996 by Mark Wahl, DL4YBG\015\012"
#define SIG7 "          solaris-porting copyright (c) 1999 by hans mayer, oe1smc \015\012"
#define SIG8 "        netbsd-porting copyright (c) 1999 by Berndt Josef Wulf, VK5ABN\015\012"
#define SIG9 "                Copyright (c) 2025 by Andreas Peters, DC6AP\015\012"
#define SIG6 "\015\012"
#define SIG_D "                         version 1.3.0 - 07/09/2025 \015\012"
