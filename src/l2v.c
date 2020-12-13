/**************************************************************************\
*                                                                          *
*                                                                          *
*    *****                      *****                                      *
*      *****                  *****                                        *
*        *****              *****                                          *
*          *****          *****                                            *
*            *****      *****                                              *
*              *****  *****                                                *
*            *****      *****                                              *
*          *****          *****          The Firmware.                     *
*        *****              *****        Portable. Compatible.             *
*      *****                  *****      Public Domain.                    *
*    *****                      *****    By NORD><LINK.                    *
*                                                                          *
*                                                                          *
*                                                                          *
*    L2V.C   -   Level 2, globale Variable                                 *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 25.04.91                                       *
*                   Globale Variable damaok = 1, wenn DAMA-Master erkannt  *
*                                           = 0, wenn kein DAMA-Master     *
*                                                                          *
*                   DL8ZAW, 27.04.91                                       *
*                   Variablen fuer RTT-Messung: A1par, A2par, Bpar, Fpar   *
*                                                                          *
*                   DL8ZAW, 04.05.91                                       *
*                   DAMA-Timer - Startwert: DApar in Sekunden              *
*                                                                          *
*                   DL8ZAW, 18.05.91                                       *
*                   Bpar ist nun A3par!                                    *
*                                                                          *
*                   DB2OS,  28.08.91                                       *
*                   Variable startx gibt es nicht mehr!                    *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   istraf[L2PNUM] entfernt, weil nicht benoetigt.         *
*                                                                          *
*                   DB2OS, 12.08.94                                        *
*                   Variablen A1par und A2par entfernt.                    *
*                                                                          *
*                   DB2OS, 28.08.94                                        *
*                   alias[] entfernt.                                      *
*                                                                          *
*                   DL4YBG, 13.02.95                                       *
*                   rxfflx fuer FlexNet-Monitor                            *
*                                                                          *
*                   WS1AC, 01.09.01                                        *
*                   define for rxfflx fuer FLEXDEC geaendert               *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */





/*                                                              Globals   */
/**************************************************************************/

char        myid[L2IDLEN];    /* Call (normal) + SSID (1 Bit linksgesch.) */

char        nmblks;           /* Anzahl aktiver Level-2-Links (1..127)    */
char        aktlks;           /* FEF Anzahl Links mit Status > L2SIXFER   */
char        Ypar;             /* Anzahl moeglicher Links (Y-Befehl)       */
char        damaok;           /* DAMA Betrieb ( 1 = ja, 0 = nein )        */
char        sendok;           /* DAMA Betrieb ( 1 = senden erlaubt )      */
char        tosend;           /* DAMA Betrieb ( 1 = was zu senden da)     */
unsigned    damati;           /* DAMA Timer   ( 1 sec Downcounter)        */

                              /* empfangenes Frame :                      */
char        rxfhdr[L2AFLEN+1];/*   Header (Ziel/Quell/via-Id's), 0-term.  */
char        rxfctl;           /*   Kontrollbyte ohne P/F-Bit              */
char        rxfPF;            /*   V1-Frame: 0                            */
                              /*   V2-Frame: 0x10 = P/F gesetzt, 0 sonst  */
char        rxfV2;            /*   0 = V1-Frame, 1 = V2-Frame             */
char        rxfCR;            /*   V1-Frame: 0                            */
                              /*   V2-Frame: 0x80 = Command-Frame         */
                              /*             0x00 = Response-Frame        */
char        rxfprt;           /*   0 = HDLC-Frame, 1 = Crosslink-Frame    */
#ifdef USE_FLEXDEC
char        rxfflx;           /*   0 = normaler Frame, 1 = FlexNet-Frame  */
#endif
                              /* zu sendendes Frame :                     */
char        txfhdr[L2AFLEN+1];/*   Header (Ziel/Quell/via-Id's), 0-term.  */
char        txfctl;           /*   Kontrollbyte ohne P/F-Bit              */
char        txfPF;            /*   V1-Frame: 0                            */
                              /*   V2-Frame: 0x10 = P/F gesetzt, 0 sonst  */
char        txfV2;            /*   0 = V1-Frame, 1 = V2-Frame             */
char        txfCR;            /*   V1-Frame: 0                            */
                              /*   V2-Frame: 0x80 = Command-Frame         */
                              /*             0x00 = Response-Frame        */
char        txfprt;           /*   0 = HDLC-Frame, 1 = Crosslink-Frame    */

unsigned    Fpar;             /* IRTT       10 ms             F-Befehl    */
unsigned    A3par;            /* Alpha3     1..16             @-A3-Befehl */
unsigned    Ipar;             /* IPOLL      0..256            @-I-Befehl  */
unsigned    Opar;             /* MAXFRAME   0..7              O-Befehl    */
unsigned    Npar;             /* RETRY      0..127            N-Befehl    */
unsigned    T2par;            /* T2         10 ms             @-T2-Befehl */
unsigned    T3par;            /* T3         10 ms             @-T3-Befehl */
unsigned    Rpar;             /* DIGION     0..1              R-Befehl    */
                              /* 1 = Digipeaten ein, 0 = aus              */
unsigned    nmbfre;           /* "number free", Anzahl freier 32-Byte-    */
                              /* Buffer (36 Byte mit Kopf)                */
LHEAD       freel;            /* "free list",                             */
                              /* Listenkopf Freibuffer                    */
LHEAD       rxfl;             /* "rx frame list",                         */
                              /* Listenkopf empfangene Frames             */
LHEAD       stfl;             /* "sent frame list",                       */
                              /* Listenkopf gesendete Frames              */
LHEAD       trfl;             /* "trash frame list",                      */
                              /* Listenkopfe Frames fuer den Muelleimer   */
LHEAD       txl2fl[L2PNUM];   /* "tx level 2 frame list",                 */
                              /* Listenkoepfe (je Port einer) zu sendende */
                              /* Frames                                   */
MBHEAD     *rxfhd[L2PNUM];    /* "rx frame head",                         */
                              /* Zeiger (je Port einer) auf das gerade    */
                              /* aktuelle Frame waherend Empfang,         */
                              /* 0 = es wird gerade kein Frame empfangen  */
MBHEAD     *txfhd[L2PNUM];    /* "tx frame head",                         */
                              /* Zeiger (je Port einer) auf das gerade    */
                              /* aktuelle Frame waherend Sendung,         */
                              /* 0 = es wird gerade kein Frame gesendet   */
LNKBLK      lnktbl[LINKNMBR]; /* "link table", fuer jeden moeglichen      */
                              /* Level-2-Link ein Eintrag                 */
LNKBLK     *lnkpoi;           /* "link pointer", globaler Zeiger auf den  */
                              /* gerade aktuellen Linkblock (in lnktbl)   */



/* Ende von L2V.C */
