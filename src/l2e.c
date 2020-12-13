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
*          *****          *****          The Firmware                      *
*        *****              *****        Portable. Compatible.             *
*      *****                  *****      Public Domain.                    *
*    *****                      *****    By NORD><LINK.                    *
*                                                                          *
*                                                                          *
*                                                                          *
*    L2E.C   -   Level 2, Teil 5                                           *
*                                                                          *
*                Dieser Teil ist eine Zusammenfassung der                  *
*                zeitunkritischsten Funktionen und kann somit mit der      *
*                groessten Platzersparnis optimiert werden.                *
*                                                                          *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 25.04.91                                       *
*                   Beim Initialisieren damaok auf 0 setzen : kein DAMA    *
*                   Kein Senden erlaubt bei DAMA Betrieb: sendok = 0,      *
*                   tosend = 0 und startx = 0.                             *
*                                                                          *
*                   DL8ZAW, 10.06.91                                       *
*                   Beim Initialisieren zusaetzlich noch damati auf 0      *
*                   setzen.                                                *
*                                                                          *
*                   DB2OS,  28.08.91                                       *
*                   Die Variable startx (25.04.91) gibt es nicht mehr.     *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   Code entfernt, der nicht fuer TheFirmware benoetigt    *
*                   wird.                                                  *
*                                                                          *
*                   DB2OS,  19.08.94                                       *
*                   MHEARD entfernt.                                       *
*                                                                          *
*                   DB2OS,  22.08.94                                       *
*                   l2init(): Da es keine Routingbuffer mehr gibt,         *
*                             die Buffer IMMER initialisieren (Warmstart)  *
*                                                                          *
*                   DL8HBS, 19.10.94                                       *
*                   l2init(): txPID auf jedem Kanal mit TXPIDDEFAULT       *
*                             initialisiert                                *
*                                                                          *
*                   DL4YBG, 06.11.94                                       *
*                   l2init(): rxPID auf jedem Kanal mit L2CPID             *
*                             initialisiert                                *
*                                                                          *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "l2s.h"         /* Zugriff auf die State-Tabellen                */
#include "l2ext.h"       /* globale Variable / nicht int-Funktionen       */

/* extern function declarations */
extern void clrT1();
extern void dealoc();
extern void inilbl();
extern void inithd();
extern void l2tolx();
extern void putchr();
extern void reslnk();
extern void sdl2fr();

/* local function declarations */
void xfrmr();


/**************************************************************************\
*                                                                          *
* action      :  "level 2 initialize"                                      *
*                                                                          *
*                Initialisieren des dynamischen Freispeichers sowie        *
*                aller Level-2-spezifischer Listen- und Framekoepfe.       *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  -                                                         *
*                                                                          *
* r/o globals :  (Routing-Table)                                           *
*                                                                          *
* r/w globals :  lnkpoi   - Zeiger in lnktbl                               *
*                rxfl     - "leere Liste"                                  *
*                monfl    - "leere Liste"                                  *
*                stfl     - "leere Liste"                                  *
*                trfl     - "leere Liste"                                  *
*                freel    - Liste freier 32-Byte-Buffer dynam. Speicher,   *
*                           jeder Buffer ist initialisiert :  mbpc, mbgc   *
*                           geloescht, mbbp initialisiert, mbl             *
*                           "leere Liste", Buffer der Routing-Table        *
*                           werden nicht neu allokiert bei Warmstart       *
*                nmblks   - 0                                              *
*                nmbfre   - Anzahl freie 32-Byte-Buffer dynam. Speicher    *
*                txl2fl[] - alle Listenkoepfe "leere Liste"                *
*                rxfhd[]  - alle Eintrage NULL                             *
*                txfhd[]  - alle Eintrage NULL                             *
*                istraf[] - alle Eintrage NULL                             *
*                lnktbl   - alle Linkbloecke werden initialisiert          *
*                           (rcvdil, sendil, rcvd, tosend, state, srcid    *
*                           loeschen, IRTT, k, N2 setzen,                  *
*                           reslnk() wird aufgerufen)                      *
*                                                                          *
* locals      :  s.u.                                                      *
*                                                                          *
* returns     :  -                                                         *
*                                                                          *
\**************************************************************************/

void l2init()
  {
    char       *actbp;        /* Zeiger auf aktuellen Freibuffer          */
    char       *nextbp;       /* Zeiger auf naechsten Freibuffer          */
    char       *minm;         /* niedrigste nutzbare Freispeicherstelle   */
    char       *maxm;         /* hoechste nutzbare Freispeicherstelle     */
    unsigned short    n;            /* Laufindex                                */


    inithd(&rxfl);            /* diverse Listenkoepfe = "leere Liste"     */
    inithd(&monfl);
    inithd(&stfl);
    inithd(&trfl);
    inithd(&freel);

    nmblks = nmbfre = 0;      /* keine Links aktiv, noch keine Freibuffer */
    minm = actbp = minmem();  /* niedrigste nutzbare Freispeicherstelle   */
    maxm = maxmem();          /* hoechste nutzbare Freispeicherstelle     */
    damaok = 0;               /* Default: kein DAMA-Betrieb               */
    sendok = 0;               /* Default: Senden nicht erlaubt bei DAMA   */
    tosend = 0;               /* Default: nichts zu senden da bei DAMA    */
    damati = 0;               /* Default: DAMA-Timeout auf 0              */

    while (    ((nextbp = actbp + sizeof(MB) - 1) <= maxm)
            && (nextbp >= minm)
          )                                  /* solange Buffer in Frei-   */
      {                                      /* speicher passen..         */
        dealoc(actbp);                       /* initialisieren und in     */
        actbp = nextbp + 1;                  /* Freispeicherliste haengen */
      }               

    for (n = 0; n < L2PNUM; ++n)             /* fuer alle Ports TX-Frame- */
      {                                      /* listen "leer", RX- und    */
       inithd(&txl2fl[n]);                   /* TX-Framekoepfe unbesetzt, */
                                             /* noch nichts gesendet      */

       rxfhd[n] = txfhd[n] = (MBHEAD *)NULL;
      }

    for (n = 0, lnkpoi = lnktbl; n < LINKNMBR; ++n, ++lnkpoi)  /* Link-   */
      {                                                        /* bloecke */
        lnkpoi->colled =                                       /* FEF     */
        lnkpoi->state = lnkpoi->rcvd = lnkpoi->tosend = 0;     /* initia- */
        inithd(&lnkpoi->rcvdil);                               /* lisier- */
        inithd(&lnkpoi->sendil);                               /* en      */
        inithd(&lnkpoi->collil);                               /* FEF     */
        lnkpoi->IRTT = Fpar;
#ifdef USE_XPID
        lnkpoi->txPID = TXPIDDEFAULT;   /* txPIDs auf default setzen  HBS */
        lnkpoi->rxPID = L2CPID;      /* txPIDs auf default setzen  DL4YBG */
#endif
        inilbl();
        reslnk(); /* damit L-Kommando nach Kaltstart im Hostmode richtig  */
      }
  }





/**************************************************************************\
*                                                                          *
* "send frame reject"                                                      *
*                                                                          *
* Wenn Linkzustand des aktuellen Linkblocks (lnkpoi) nicht Disconnectet/   *
* Linksetup/Disconnectrequest/Framereject ist, FRMR-Zustand einnehmen      *
* (state), FRMR den hoeheren Leveln melden, FRMR-Infobytes aufbauen mit    *
* den Flags ZYXW und merken im Linkblock (frmr), FRACK-Timer abschalten,   *
* FRMR senden.                                                             *
*                                                                          *
\**************************************************************************/

void sdfrmr(ZYXW)

char ZYXW;

  {
    char *frmrip;             /* Zeiger in Linkblock-FRMR-Infobytes       */

    if (lnkpoi->state >= L2SIXFER)           /* nur ausfuehren, wenn s.o. */
      {
        frmrip = lnkpoi->frmr;               /* Zeiger auf FRMR-Infobytes */
        *frmrip++ = rxfctl | rxfPF;          /* die FRMR-Infobytes        */
        *frmrip++ =   lnkpoi->VR << 5        /* aufbauen                  */
                    | (!rxfCR ? 0x10 : 0)
                    | lnkpoi->VS << 1;
        *frmrip   = ZYXW;
        lnkpoi->state = L2SFRREJ;            /* Linkzustand: Framereject  */
        clrT1();                             /* FRACK-Timer abschalten    */
        l2tolx(L2MFRMRT);                    /* FRMR melden               */
        xfrmr();                             /* FRMR aussenden            */
      }
  }





/**************************************************************************\
*                                                                          *
* FRMR senden mit den im aktuellen Linkblock (lnkpoi) gegebenen            *
* Parametern (frmr). Die txf...-Parameter muessen gesetzt sein bis auf     *
* txfctl.                                                                  *
*                                                                          *
\**************************************************************************/

void xfrmr()
  {
    char     *frmrip;         /* Zeiger in Linkblock-FRMR-Infobytes       */
    MBHEAD   *fbp;            /* Framemessageheader fuer Frameaufbau      */

    txfctl = L2CFRMR;                   /* FRMR-Frame !                   */
    fbp = makfhd(L2FUS | L2FT1ST);      /* Buffer holen, Framekopf bauen  */
    frmrip = lnkpoi->frmr;              /* Zeiger auf FRMR-Infobytes      */
    putchr(*frmrip++,fbp);              /* FRMR-Infobytes in Frame        */
    putchr(*frmrip++,fbp);
    putchr(*frmrip  ,fbp);
    sdl2fr(fbp);                        /* Frame aussenden                */
  }



/* Ende von L2E.C */
