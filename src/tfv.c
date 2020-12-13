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
*    TFV.C   -   The Firmware, globale Variable                            *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*                                                                          *
*    modifiziert:   DL8ZAW, 25.07.91                                       *
*                   Walt, Palt: Werte von Slottime und P-Persistance vor   *
*                               dem Umschalten in DAMA-Betrieb.            *
*                               Nach Ausschalten von DAMA werden diese     *
*                               Werte wieder uebernommen.                  *
*                                                                          *
*                   DB2OS,  17.09.91                                       *
*                   UIpar:  UI+ @U-Parameter.                              *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   l3rxfl: entfernt, da nicht verwendet in TF             *
*                                                                          *
*                   DB2OS,  23.06.92                                       *
*                   Walt:   entfernt, Slottime=0 gibt Probleme..           *
*                                                                          *
*                   DB2OS,  19.08.94                                       *
*                   ininat/noatou Timeout entfernt.                        *
*                                                                          *
*                   DB2OS,  27.08.94                                       *
*                   Variablen loops und rps fr RPS-Anzeige                *
*                                                                          *
*                   DL8HBS, 19.10.94                                       *
*                   UItxPID als Variable definiert                         *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "tf.h"          /* Festlegungen/Datenstrukturen fuer TheFirmware */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */





/*                                             dynamische Globals (RAM)   */
/**************************************************************************/

LHEAD       monfl;            /* "monitor frame list", Listenkopf der     */
                              /* Monitor-Frameliste, bei der Firmware     */
                              /* werden hier hintereinander alle          */
                              /* empfangenen und gesendeten Frames        */
                              /* abgelegt                                 */
char        ch0id[L2IDLEN];   /* ID unproto-Kanal 0, Call normal, SSID 1  */
                              /* Bit linksgeschoben                       */
char        ch0via[L2VLEN+1]; /* via-ID's unproto-Kanal 0, 0-terminiert   */
char        hinbuf[HBUFLEN];  /* Host-Eingabebuffer                       */
char       *inbufp;           /* Zeiger in den Host-Eingabebuffer         */
char        Apar;             /* AUTOLF     0..1              A-Befehl    */
                              /* 1 = LF automatisch auf CR bei            */
                              /* Hosteingaben, 0 kein automatisches LF    */
char        Epar;             /* ECHO       0..1              E-Befehl    */
                              /* 1 = Echo ein bei Hosteingaben, 0 aus     */
char        UIpar;            /* UIPOLL     0=UI  1=UI+       @U-Befehl   */
char        actch;            /* aktueller Kanal (0..127)                 */
char        Xpar;             /* PTTON      0..1              X-Befehl    */
char        Zpar;             /* FLOW       0..3              Z-Befehl    */
                              /*   0 = Flow aus, XON/XOFF aus             */
                              /*   1 = Flow ein, XON/XOFF aus             */
                              /*   2 = Flow aus, XON/XOFF ein             */
                              /*   3 = Flow ein, XON/XOFF ein             */
char        Dpar;             /* DUPLEX     0..1             @D-Befehl    */
char        Tpar;             /* TXDELAY    10ms              T-Befehl    */
char        Ppar;             /* P-P-Wert   0..255            P-Befehl    */
char        Wpar;             /* WAIT       10ms              W-Befehl    */
char        Upar;             /* UMODE      0..1              U-Befehl    */
                              /* 1 = unattended Modus ein, 0 aus          */
char        Utxt[UTXTLEN];    /* CTEXT fuer unattended Modus              */
char        Mpar;             /* Monitorparameter             M-Befehl    */
                              /*   Bit 0 - I                              */
                              /*   Bit 1 - U                              */
                              /*   Bit 2 - S                              */
                              /*   Bit 3 - C                              */
                              /*   Bit 4 - nicht benutzt                  */
                              /*   Bit 5 - nicht benutzt                  */
                              /*   Bit 6 - nicht benutzt                  */
                              /*   Bit 7 - nicht benutzt                  */
char        mftsel;           /* "monitor from/to select",                */
                              /*   0 = nichts                             */
                              /*   1 = + (Monitor nur mftidl-Stationen)   */
                              /*   2 = - (Monitor nicht mftidl-Stationen) */
char        mftidl[L2VLEN+1]; /* "monitor from to id list", die ID's der  */
                              /* Stationen fuer +/-, 0-terminiert         */
char        ishmod;           /* 1 = Hostmode ist an, 0 sonst             */
char        hmstat;           /* Zustand waehrend Hostmodeingabe (HMS...) */
char        hmch;             /* Hostmode-Kanal                           */
char        hmcmd;            /* Hostmode-Kommandobyte                    */
char        hmlen;            /* Laenge der Hostmode-Eingabe              */
unsigned    incnt;            /* "in counter", Laenge einer Host-Eingabe  */
unsigned    isctlr;           /* 1 = Control R war eingegeben, 0 sonst    */
unsigned    ticks;            /* 10ms-Ticks, Systemtakt                   */
unsigned    oldtic;           /* alter ticks-Stand fuer Sekundentakt      */
unsigned    newtic;           /* neuer ticks-Stand                        */
unsigned    Utcnt;            /* Anzahl Zeichen in Utxt                   */
unsigned    VCpar;            /* Validate Callsign Parameter, @-V Befehl, */
                              /* 1 = Rufzeichencheck ein, 0 = aus         */
unsigned    magicn;           /* Magic Number fuer Warmstartfeststellung  */
MBHEAD     *mifmbp;           /* "monitor I frame message buffer pointer" */
                              /* Zeiger auf I-Frame, welches hinter einem */
                              /* Monitor-Header auf dem Monitorkanal      */
                              /* ausgegeben werden soll als naechstes,    */
                              /* 0 = kein solches I-Frame vorhanden       */
LHEAD       statml;           /* "status message list",                   */
                              /* Listenkopf Statusmeldungsliste Kanal 0   */
LHEAD       smonfl;           /* "selected monitor frame list",           */
                              /* Listenkopf Monitorframeliste, nur Mpar   */
                              /* entsprechende Frames                     */
LHEAD       chnlml[LINKNMBR]; /* "channel message list",                  */
                              /* Listenkoepfe fuer jeden Kanal fuer die   */
                              /* Empfangsframes und Statusmedungen        */
unsigned    chnlbc[LINKNMBR]; /* FEF Anzahl der gespeicherten Bytes auf   */
                              /* jedem Kanal fr Flow-Control             */
unsigned    loops;            /* Hauptschleifendurchl„ufe                 */
unsigned    rps;              /* Rounds per Second                        */
#ifdef USE_XPID
char        UItxPID;          /* PID fuer UI-Frames                   HBS */
#endif
unsigned    sec100;           /* aktuelle Zeit :    1/100 Sekunde (0..99) */
TIMEBL      tftime;             /* aktuelle Zeit/Datum                      */
char        eudate;           /* Datumsausgabe :  europaeisch tt.mm.yy    */
                              /*                  amerikanisch mm/dd/yy   */
char        stamp;            /* Zeit/Datum-Stamp :                       */
                              /*   0 = Stamp aus                          */
                              /*   1 = bei Statusmeldungen                */
                              /*   2 = bei Status- und Monitormeldungen   */
char        xFpar;            /* Flagsenden in Sendepausen, 1 = an, 0 aus */
                              /* Heard-Buffer bei Ausgabe der Heardliste  */
char        stmem[STACKLEN-1];/* Stackspeicher                            */
char        stack;            /* hoechste Speicherstelle des Stacks       */
unsigned    fremem;           /* erste Speicherstelle des Freispeichers   */

char        xTApar;




/*                                              statische Globals (ROM)   */
/**************************************************************************/

char cqid[] = { 'C','Q',' ',' ',' ',' ',0x60 };   /* ID UI-Frame an CQ    */

unsigned conctl = 6;          /* FEF 5  Pakete Congestion-Control fest    */


extern char l2tct[];          /* forward-Referenzen fuer l2msgs           */
extern char l2tdf[];
extern char l2tbf[];
extern char l2tlfw[];
extern char l2tlrf[];
extern char l2tlrt[];
extern char l2tfrf[];
extern char l2tfrt[];

char *l2msgs[] =              /* Tabelle Stringadressen Statusmeldungen   */
  {                           /* entsprechend L2M... :                    */
    l2tct,                    /*   L2MCONNT - 1                           */
    l2tdf,                    /*   L2MDISCF - 1                           */
    l2tbf,                    /*   L2MBUSYF - 1                           */
    l2tlfw,                   /*   L2MFAILW - 1                           */
    l2tlrf,                   /*   L2MLRESF - 1                           */
    l2tlrt,                   /*   L2MLREST - 1                           */
    l2tfrf,                   /*   L2MFRMRF - 1                           */
    l2tfrt                    /*   L2MFRMRT - 1                           */
  };



extern void extcmd();         /* Referenzen fuer cmdtab                   */
extern void Acmd();
extern void Bcmd();
extern void Ccmd();
extern void Dcmd();
extern void Ecmd();
extern void Fcmd();
extern void Gcmd();
extern void Hcmd();
extern void Icmd();
extern void Jcmd();
extern void Kcmd();
extern void Lcmd();
extern void Mcmd();
extern void Ncmd();
extern void Ocmd();
extern void Pcmd();
extern void Qcmd();
extern void Rcmd();
extern void Scmd();
extern void Tcmd();
extern void Ucmd();
extern void Vcmd();
extern void Wcmd();
extern void Xcmd();
extern void Ycmd();
extern void Zcmd();

void (*cmdtab[])() =          /* Tabelle der Kommandofunktionen :         */
  {
    extcmd,                   /*   @   (T2, T3, B, D, K, S, V)            */
    Acmd,
    Bcmd,
    Ccmd,
    Dcmd,
    Ecmd,
    Fcmd,
    Gcmd,
    Hcmd,
    Icmd,
    Jcmd,
    Kcmd,
    Lcmd,
    Mcmd,
    Ncmd,
    Ocmd,
    Pcmd,
    Qcmd,
    Rcmd,
    Scmd,
    Tcmd,
    Ucmd,
    Vcmd,
    Wcmd,
    Xcmd,
    Ycmd,
    Zcmd
  };



char l2tct[]  = "CONNECTED to";         /* die Strings fuer l2msgs        */
char l2tdf[]  = "DISCONNECTED fm";
char l2tbf[]  = "BUSY fm";
char l2tlfw[] = "LINK FAILURE with";
char l2tlrf[] = "LINK RESET fm";
char l2tlrt[] = "LINK RESET to";
char l2tfrf[] = "FRAME REJECT fm";
char l2tfrt[] = "FRAME REJECT to";



/* Ende von TFV.C */
