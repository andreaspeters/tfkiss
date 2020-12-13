/* tfkiss: TNC-emulation for Linux
   Copyright (C) 1995-96 by Mark Wahl
   KISS headerfile (kiss.h)
   created: Mark Wahl DL4YBG 95/09/18
   updated: Mark Wahl DL4YBG 96/03/09
*/

/* define maximum line length */
#define MAXCHAR 80

/* definitions for kiss-commands */
#define CMD_TXDELAY 1
#define CMD_PERSIST 2
#define CMD_SLOTTIME 3
#define CMD_TXTAIL 4
#define CMD_FULLDUP 5

/* definitions for different KISS-types */
#define KISS_NORMAL 0
#define KISS_SMACK 1
#define KISS_RMNC 2
