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
*      *****                  *****      Proteced by ALAS                  *
*    *****                      *****    By NORD><LINK.                    *
*                                                                          *
*                                                                          *
*                                                                          *
*    L2B.C   -   Level 2, Teil 2                                           *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 04.05.91                                       *
*                   setiSRTT(): Anfangswert fuer SRTT setzen; ersetzt      *
*                               setiT1().                                  *
*                   inilbl():   IRTT mit Fpar initialisieren               *
*                   sdipoll():  I-Frame als Poll aussenden (DK6PX)         *
*                                                                          *
*                   DL8ZAW, 31.05.91                                       *
*                   setiSRTT(): Digipeateranzahl n ermitteln,              *
*                               SRTT = (2*n+1) * IRTT                      *
*                               (nur fuer Connect!)                        *
*                   reslnk():   SRTT = IRTT setzen                         *
*                                                                          *
*                   DL8ZAW, 05.06.91                                       *
*                   sdui(): Wenn UIPOLL definiert ist, UI-Frames mit       *
*                           gesetzem POLL-Bit aussenden.                   *
*                                                                          *
*                   DL8ZAW, 19.06.91                                       *
*                   itolnk(): nur compilieren, wenn FIRMWARE nicht         *
*                             definiert ist, da diese Funktion in TF       *
*                             NICHT benutzt wird!                          *
*                                                                          *
*                   clrlnk(): Wenn alle Links aufgeloest sind, dann        *
*                             DAMA-Timer ruecksetzen                       *
*                                                                          *
*                   DK6PX, 20.07.91                                        *
*                   sdi(): Je nach Laenge der I-Frames MAXFRAME erhoehen   *
*                                                                          *
*                   DB2OS, 29.08.91                                        *
*                   digipt(): Zu "digipeatende" Frames trotz DAMA sofort   *
*                             senden, auch wenn der eigene TNC nicht       *
*                             gepollt wurde.                               *
*                                                                          *
*                   DB2OS, 03.09.91                                        *
*                   sdi(): Timer T1 und RTT drfen nur nach dem LETZTEN    *
*                          zu sendenden I-Frame gestartet werden!!!        *
*                          Nach dieser Modifikation funtioniert SRTT nun   *
*                          richtig bzw. deutlich besser.                   *
*                                                                          *
*                   DB2OS, 17.09.91                                        *
*                   sdui(): UIPOLL jetzt ber Parameter @U gesteuert,      *
*                           mittels Variable UIpar.                        *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   tol3sw():  Weitergabe des Frames an L3 entfernt, weil  *
*                              es in TheFirmware keinen L3 gibt.           *
*                   itolnk():  entfernt, weil nicht benoetigt              *
*                                                                          *
*                   DB2OS, 21.6.93                                         *
*                   sdi(): Dynam. Maxframe von DK6PX gestrichen, da        *
*                          es damit besonders auf KW und schlechten        *
*                          Links erhebliche Probleme gibt..                *
*                                                                          *
*                   DL4YBG, 23.10.93                                       *
*                   setiSRTT(): An neue Strategie angepasst                *
*                                                                          *
*                   DG2FEF, 18.06.94                                       *
*                   disc(): Bei DAMA kein DISC+ senden, sondern direkt auf *                                              *
*                           Status DSCRQ.                                  *
*                   l2stma(): Bei DAMA und State DSCRQ auf einem Link,     *
*                             auf alle folgenden Frames mit DM- antworten  *
*                             und auf State DSCED gehen. Nachricht ans     *
*                             Terminal senden.                             *
*                                                                          *
*                   DG2FEF,18.07.94                                        *
*                   disc(): Doch wieder DISC  senden, auáer beim letzten   *
*                           Link, um Deadlock bei DAMA zu verhindern       *         *
*                   sdipoll(): Angepaát fr Aufruf aus der Statemachine    *
*                   sdoi(): Angepaát fr Framesammler-Untersttzung        *
*                   srxdNR(): dito                                         *
*                   tol3sw(): entfernt.                                    *
*                   dsclnk(): Code optimiert, ein paar Sachen aus Dcmd()   *
*                             hier untergebracht.                          *
*                   setiSRTT(): "initial SRTT" jetzt IRTT * (Digianzahl+1) *
*                               nicht mehr IRTT * (Digianzahl*2+1)         *
*                                                                          *
*                   DG2FEF, 13.08.94                                       *
*                   isnxti(): Framesammler nach OE1HHC/OE3GMW.             *
*                                                                          *
*                   DB2OS,  19.08.94                                       *
*                   i2tolx(): noatou Timeout nicht benutzt und entfernt.   *
*                                                                          *
*                   DL4YBG, 06.11.94                                       *
*                   reslnk(): init von lnkpoi->rxPID.                      *
*                   inilbl(): init von lnkpoi->txPID.                      *
*                                                                          *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "l2s.h"         /* Zugriff auf die State-Tabellen                */
#include "l2ext.h"       /* globale Variable / nicht int-Funktionen       */

/* external function declarations */
extern void clrRTT();
extern void cpyid();
extern void cpyidl();
extern void dealmb();
extern void dealml();
extern void l2tolx();
extern void putchr();
extern void rwndmb();
extern void sdfrmr();
extern void sdl2fr();
extern void setT1();
extern void stxfad();
extern LEHEAD *tfunlink();
extern void xdisc();
extern void xsabm();

/* local function declarations */
void clrlnk();
void clrT1();
void clrT2();
void clrT3();
void disc();
void inilbl();
void l2stma();
void mclrlk();
void reslnk();
void sdi();
void sdipoll();
void setiSRTT();
void xrrc();
void xdm();
/**************************************************************************\
*                                                                          *
* "new link"                                                               *
*                                                                          *
* Link (lnkpoi) neu aufbauen. Wenn Link noch nicht aktiv war, die Anzahl   *
* aktiver Links (nmblks) erhoehen.                                         *
*                                                                          *
\**************************************************************************/

void newlnk()
  {
    reslnk();                           /* Sequenzvars/Timer ruecksetzen  */
    setiSRTT();                         /* Anfangswert SRTT               */
    txfV2 = YES;                        /* welche Protokollversion        */
    xsabm();                            /* SABM senden                    */
    if (lnkpoi->state == L2SDSCED)      /* neuer Link ?                   */
      ++nmblks;                         /* ja ...                         */
    lnkpoi->state = L2SLKSUP;           /* Linkstatus "Link Setup"        */
  }





/**************************************************************************\
*                                                                          *
* "disconnect link"                                                        *
*                                                                          *
* Disconnect-Wunsch an aktuellen Link (lnkpoi) :                           *
*                                                                          *
*   Linkstatus "Disconnected"                                              *
*   -> Ax.25-Parameter "frisch"                                            *
*   FEF Gel”scht, passiert schon in Dcmd(), TFB.C                          *
*                                                                          *
*   Linkstatus "Link Setup" oder "Disconnect Request"                      *
*   -> Link aufloesen, neuer Linkstatus "Disconnected"                     *
*                                                                          *
*   sonst                                                                  *
*   -> Empfangsinfoframeliste loeschen, Linkstatus bleibt, Flag "nach      *
*      Loswerden aller zu sendenden Infoframes disconnecten" setzen        *
*                                                                          *
\**************************************************************************/

void dsclnk()                                        /* FEF */
  {
    static unsigned short lstate;

    if (   (lstate = lnkpoi->state) == L2SLKSUP
        || lstate == L2SDSCRQ
        || lnkpoi->flag & L2FDSLE
       )
      {
        if (lnkpoi->flag & L2FDSLE)              /* Link aufzul”sen?   */
          disc();                                /* Dann los ...       */
        else
          {
            if (lstate == L2SLKSUP)              /* Falls "Link setup" */
              xdisc();                           /*    DISC senden.    */
            mclrlk();                            /*    Link aufloesen  */
            lnkpoi->state = L2SDSCED;            /*    Disconnected !  */
          }
      }
    else
      {                                              /* sonst           */
        dealml(&lnkpoi->rcvdil);                     /* RX-Infoframe-   */
        lnkpoi->rcvd = 0;                            /* loeschen und    */
        dealml(&lnkpoi->collil);                     /* den Framesammler*/
        lnkpoi->colled = 0;
        lnkpoi->flag |= L2FDSLE;                     /* Flag, s.o.      */
      }
  }





/**************************************************************************\
*                                                                          *
* "digipeat"                                                               *
*                                                                          *
* Framebuffer, auf den fbp zeigt, abtesten, ob er ein zu digipeatendes     *
* Paket enthaelt. Wenn ja, Paket an den entsprechenden Port (falls ein     *
* bekannter Nachbarknoten direkt in der Digiliste folgt oder nach          *
* kompletten Digipeaten das Ziel ist, kann das Paket auch auf einem        *
* anderen als dem normalen HDLC-Port digipeatet werden) ausgeben.          *
* Das Paket muss schon durch takfhd() analysiert sein, die rxf...-         *
* Parameter muessen gesetzt sein.                                          *
*                                                                          *
* Return:  YES - das Frame wurde entweder digipeatet, oder der             *
*                Framebuffer wurde deallokiert, weil das Frame noch von    *
*                einer anderen Station digipeatet werden muss oder ich     *
*                nicht digipeaten darf                                     *
*          NO  - das Frame muss nicht mehr digipeated werden, hat alle     *
*                noetigen Digipeater durchlaufen, ist zur Auswertung frei  *
*                                                                          *
* FEF Bei DAMA kein sendok = 1 mehr !!!                                    *
*                                                                          *
\**************************************************************************/

BOOLEAN digipt(fbp)

MBHEAD *fbp;

  {
    static char       *viap;                      /* Zeiger in via-Liste  */
    static unsigned short    n;                         /* Zaehler              */

    viap = rxfhdr + L2ILEN;                       /* Anfang via-Liste     */
    while (*viap != '\0')                         /* via-Liste durchgehen */
      {
        if (!(viap[L2IDLEN - 1] & L2CH))          /* zu digipeaten ?      */
          {
            if (Rpar == YES && istome(viap) == TRUE)   /* ja, darf ich    */
              {                                        /* und muss ich :  */
                rwndmb(fbp);                           /* Zeiger auf Hbit */
                n = (unsigned short)(viap - (rxfhdr+L2ILEN) + (L2ILEN+L2IDLEN));
                while (n-- != 0) getchr(fbp);          /* berechnen       */
                *(fbp->mbbp - 1) |= L2CH;              /* Hbit setzen     */
                viap += L2IDLEN;                       /* Nachbardigi     */
                fbp->l2port = nbrprt(   *viap != '\0'  /* ich letzter ?   */
                                      ? viap           /* nein, Port Digi */
                                      : rxfhdr);       /* ja, Port Ziel   */
                fbp->l2fflg = 0;                       /* kein Linkframe  */
                sdl2fr(fbp);                           /* Frame senden <-+*/
              }
            else              /* Frame ist noch nicht komplett digipeatet */
              dealmb(fbp);    /* und/oder nix fuer mich, Frame wegwerfen  */
            return (YES);     /* fuer aufrufende Funk.: Frame ist weg !   */
          }
        viap += L2IDLEN;      /* naechsten Digi in Digiliste untersuchen  */
      }
    return (NO);              /* fuer aufrufende Funk.: Frame auswerten ! */
  }



/**************************************************************************\
*                                                                          *
* "level 2 information to level x"                                         *
*                                                                          *
* Infopakete aus dem aktuellen Link (lnkpoi) an hoehere Level weiter-      *
* reichen. nocgnc gibt an, ob der hoehere Level die "Erstickungskontrolle" *
* (hier = Beruecksichtigung der maximal noch anzunehmenden I-Pakete)       *
* machen soll (NO) oder in jedem Fall alle uebermittelten I-Pakete         *
* annehmen muss (YES). Falls die I-Pakete vom hoeheren Level angenommen    *
* wurden, Empfangszaehler rcvd entsprechend hochz„hlen.                    *
* Es wird l2link in den Framekoepfen der weitergereichten Pakete           *
* auf lnkpoi gesetzt und type auf 2 fuer "Level 2".                        *
*                                                                          *
\**************************************************************************/
void i2tolx(nocgnc)

unsigned short nocgnc;

  {
    static MBHEAD *rcvdip;   /* Zeiger auf Framekopf weiterzureichendes I */

    while (lnkpoi->rcvd != 0)           /* solange I's aus Link vorhanden */
      {
        (rcvdip = (MBHEAD *)lnkpoi->rcvdil.head)->l2link = lnkpoi;
                                                            /* Linkzeiger */
        rcvdip->type = 2;                                   /* Level 2 !  */
        if (!fmlink(nocgnc,rcvdip))     /* I an hoeheren Level geben      */
          return;                       /* Abbruch, wenn nicht angenommen */
        --lnkpoi->rcvd;                 /* Empfangspaketezaehler updaten  */
      }
  }





/**************************************************************************\
*                                                                          *
* "serve received N(R)"                                                    *
*                                                                          *
* Aktuell empfangenes N(R) (rxfctl) des aktuellen Links (lnkpoi) auswerten *
* und entsprechend verfahren (s.u.).                                       *
*                                                                          *
* Return:  YES - aktuell empfangenes N(R) ist okay oder Linkzustand        *
*                laesst N(R)-Empfang nicht zu                              *
*          NO  - aktuell empfangenes N(R) ist falsch                       *
*                                                                          *
\**************************************************************************/

BOOLEAN srxdNR()
  {
    static unsigned short rxdNR;   /* empfangenes N(R)                          */
    static unsigned short newok;   /* Anzahl neu bestaetigte I's                */
    static unsigned short outstd;  /* Anzahl ausstehende (nicht bestaetigte) I's*/
    static unsigned short l2state; /* Linkstatus des aktuellen Links            */

    if ((l2state = lnkpoi->state) >= L2SIXFER)    /* darf N(R) kommen ?   */
      {
        if (    ( outstd = (lnkpoi->VS - lnkpoi->lrxdNR) & 0x07
                ) != 0
             && ( newok = ( (rxdNR = (rxfctl >> 5) & 0x07) - lnkpoi->lrxdNR
                          ) & 0x07
                ) != 0             /* N(R) nur auswerten, wenn I's aus-   */
           ) {                     /* stehen und neue bestaetigt werden   */
          if (newok <= outstd)     /* N(R) okay ?                         */
            {
              if (((lnkpoi->RTTNR - lnkpoi->lrxdNR) & 0x07) >= newok)
                clrRTT();                      /* FEF RTT-Messung stoppen */
              lnkpoi->lrxdNR = rxdNR;   /* ja, N(R) annehmen              */
              clrT1();                  /* T1 stoppen                     */
              if (newok != outstd)      /* wenn immer noch I's ausstehend */
                if (lnkpoi->flag & L2FMAXF1)     /* Retryen wir ?         */
                  lnkpoi->VS = rxdNR;            /* V(S) resetten         */
                else
                  setT1();              /* T1 neu starten                 */
              else
                lnkpoi->flag &= ~L2FMAXF1;     /* FEF alles best„tigt ... */

              while (newok-- != 0)
                {                                      /* alle neu        */
                  dealmb(tfunlink(lnkpoi->sendil.head)); /* bestaetigten    */
                  --lnkpoi->tosend;                    /* I's wegwerfen   */
                }
            }
          else
            {                           /* nein,                          */
              sdfrmr(0x08);             /* Kontrollfeld hat falsches N(R) */
              return (FALSE);           /* N(R) nicht okay !              */
            }
        }

        if (!rxfCR && rxfPF != 0)          /* wenn empfangenes Frame    */
          {                                /* Response mit Final war,   */
            clrT1();                       /* Timer 1 stoppen und       */
            lnkpoi->RTT = 0;               /* FEF RTT wieder zulassen   */
            lnkpoi->flag &= ~L2FDIRTY;
            lnkpoi->VS = lnkpoi->lrxdNR;   /* V(S) updaten              */
          }
        else
          if (   l2state == L2SWA          /* FEF umgestellt wegen RNR- */
              || l2state == L2SWADBS       /* recovery                  */
              || l2state == L2SWARBS
              || l2state == L2SWABBS
             )
            if (!lnkpoi->T1) setT1();
    }

    return (TRUE);                      /* N(R) okay oder nicht benutzt   */
  }





/**************************************************************************\
*                                                                          *
* "is next I"                                                              *
*                                                                          *
* Testen, ob das aktuell empfangene I-Frame (rxf...) das naechste fuer den *
* aktuellen Linkblock (lnkpoi) erwartete I-Frame ist, wenn der Linkzustand *
* Informationstransfer zulaesst. Bei nicht erwarteter Sequenznummer        *
* entsprechende Statetable abarbeiten.                                     *
*                                                                          *
* Return :  YES - I-Frame ist das naechste erwartete oder Linkzustand      *
*                 laesst keinen Informationstransfer zu                    *
*           NO  - sonst                                                    *
* DG2FEF:                                                                  *
* Umgebaut fr Framesammler.                                               *
*                                                                          *
* Return nun:   0 - I-Frame ist falsch, nicht zu gebrauchen fr Frame-     *
*                   sammler, also verwerfen                                *
*               1 - I-Frame ist OK, in normale Linkliste einh„ngen, bzw.   *
*                   Linkzustand erlaubt keinen I-Transfer                  *
*               2 - I-Frame ist falsch, kann aber in den Framesammler      *
*                                                                          *
*                                                                          *
\**************************************************************************/

unsigned short isnxti()
  {
    static unsigned short iseqno;                       /* I Sequence Number    */

    if (lnkpoi->state >= L2SIXFER) {                      /* I-Transfer ? */
      if ((iseqno = (rxfctl >> 1) & 0x07) == lnkpoi->VR)  /* I erwartet ? */
        if (((lnkpoi->ltxdNR + 7) & 0x07) != iseqno)      /* kein Ueber-  */
          {                                               /* lauf ?       */
            if (nmbfre > L2BREMSE)                        /* noch Platz?  */
              lnkpoi->VR = (iseqno + 1) & 0x07;           /* V(R) setzen  */
          }
        else
          {
            sdfrmr(0x01);      /* Kontrollfeld falsch/nicht implementiert */
            return (0);
          }
      else                                       /* unerwartetes I (nicht */
        {                                        /* naechstes in Reihe)   */
          l2stma(!rxfPF ? stbl26 : stb26b);      /* INVALID N(S) RECEIVED */

          if (!lnkpoi->colled)                   /* Framesammler noch aus */
            {                                    /* nur 1 Frame verloren? */
              if (   iseqno == ((lnkpoi->VR+1) & 0x07)
                  && iseqno != lnkpoi->xpctVR)
                {
                  ++lnkpoi->colled;              /* dann Framesammler ein */
                  lnkpoi->xpctVR = lnkpoi->VR;
                }
            }
          else                                   /* schon ein Frame drin  */
            if (iseqno == lnkpoi->lastVR)        /* paát an das letzt im  */
              ++lnkpoi->colled;                  /* Sammler, also rein    */
            else                                 /* sonst:                */
              return(0);                         /* weg damit!            */

          if (lnkpoi->colled)               /* Sammler ist ein, Frame ok  */
            {
              lnkpoi->lastVR = (iseqno+1) & 0x07;  /* V(R) merken */
              return(2);                       /* Frame in Sammler, bitte */
            }

          return (0);                            /* unerwartetes I !      */
        }
    }
    return (1);          /* I richtig oder Linkzustand ohne I-Transfer    */
  }





/**************************************************************************\
*                                                                          *
* "initialize link"                                                        *
*                                                                          *
* Aktuellen Linkblock (lnkpoi) initialisieren. Sequenzvariablen und Timer  *
* initialisieren, Quellcall/Zielcall/via-Liste/ Port setzen aus der        *
* txf...-Liste.                                                            *
*                                                                          *
\**************************************************************************/

void inilnk()
  {
    reslnk();                                /* Sequenzvars/Timer init.   */
    cpyid(lnkpoi->srcid,txfhdr + L2IDLEN);   /* Quellcall                 */
    cpyid(lnkpoi->dstid,txfhdr);             /* Zielcall                  */
    cpyidl(lnkpoi->viaidl,txfhdr + L2ILEN);  /* via-Liste                 */
    lnkpoi->liport = txfprt;                 /* Port                      */
  }





/**************************************************************************\
*                                                                          *
* "set initial SRTT"                                                       *
* FEF                                                                      *
* Anfangswert fuer Smoothed Round Trip Timer setzen                        *
*                                                                          *
\**************************************************************************/

void setiSRTT()
  {
    char       *viap;                   /* Zeiger in via-Liste            */
    unsigned short    n;                      /* Digizaehler                    */

    viap = lnkpoi->viaidl;              /* Anfang via-Liste               */
    n = 0;                              /* noch kein Digi gezaehlt        */
    while (*viap != '\0')               /* Digianzahl ermitteln           */
      {
        ++n;
        viap += L2IDLEN;
      }
    ++n;                                      /* FEF  Digianzahl + 1      */

    lnkpoi->SRTT = n * lnkpoi->IRTT;
    lnkpoi->IT1 = A3par * lnkpoi->SRTT; /* initial T1-timerwert */
    lnkpoi->RTT = 0;
   /* ende */
  }





/**************************************************************************\
*                                                                          *
* "messaged clear link"                                                    *
*                                                                          *
* Aktuellen Link aufloesen (siehe clrlnk()) und entsprechende Meldung an   *
* hoehere Level geben ("Disconnected from").                               *
*                                                                          *
\**************************************************************************/

void mclrlk()
  {
    l2tolx(L2MDISCF);                   /* und hoehere Level informieren  */
    clrlnk();                           /* Link aufloesen                 */
  }





/**************************************************************************\
*                                                                          *
* "clear link"                                                             *
*                                                                          *
* Aktuellen Link (lnkpoi) aufloesen. Alle Sequenzvariablen und Timer       *
* zuruecksetzen, Sende- und Empfangsinfoframelise loeschen, Linkblock neu  *
* mit AX.25-Parametern besetzen, Anzahl der aktiven Links (nmblks) um 1    *
* erniedrigen.                                                             *
*                                                                          *
\**************************************************************************/

void clrlnk()
  {
    reslnk();                           /* Sequenzvars/Timer ruecksetzen  */
    dealml(&lnkpoi->rcvdil);            /* Empfangsinfoliste loeschen     */
    dealml(&lnkpoi->sendil);            /* Sendeinfoliste loeschen        */
    dealml(&lnkpoi->collil);            /* FEF Framesammler l”schen       */
    --nmblks;                           /* nun ein aktiver Link weniger   */
    lnkpoi->colled =
    lnkpoi->rcvd = lnkpoi->tosend = 0;  /* entsprechende Zaehler loeschen */
    inilbl();                           /* Linkblock "frisch"             */
  }





/**************************************************************************\
*                                                                          *
* "disconnect"                                                             *
* FEF                                                                      *
* Disconnect des aktuellen Links (lnkpoi) einleiten. Alle Sequenzvariablen *
* und Timer zuruecksetzen, DISC senden, neuer Status "Disconnect request". *
*                                                                          *
\**************************************************************************/

void disc()
  {
    if (damaok)
     {
      if (aktlks > 1)                    /* mehr als 1 QSO mit dem Master */
        {
          xdisc();
          reslnk();
          lnkpoi->state = L2SDSCRQ;
        }
      else                                 /* aktlks <= 1 */
        if (nmblks == 1)                   /* letztes QSO mit DAMA-Master */
          {
           reslnk();
           lnkpoi->state = L2SDSCRQ;
          }
     }
    else
     {
       xdisc();
       reslnk();
       lnkpoi->state = L2SDSCRQ;
     }
  }




/**************************************************************************\
*                                                                          *
* "reset link"                                                             *
*                                                                          *
* Aktuellen Link (lnkpoi) zuruecksetzen. Alle Sequenzvariablen und Timer   *
* initialisieren.                                                          *
*                                                                          *
\**************************************************************************/

void reslnk()
  {
       lnkpoi->VS
     = lnkpoi->RTT      /* FEF */
     = lnkpoi->VR
     = lnkpoi->ltxdNR
     = lnkpoi->lrxdNR
     = lnkpoi->flag
     = 0;
     lnkpoi->xpctVR = 8;   /* FEF */
     lnkpoi->SRTT = lnkpoi->IRTT;
#ifdef USE_XPID
     lnkpoi->rxPID = L2CPID;  /* DL4YBG */
#endif
     clrT1();
     clrT2();
  }





/**************************************************************************\
*                                                                          *
* "initialize link block"                                                  *
*                                                                          *
* Aktuellen Linkblock (lnkpoi) mit AX.25-Parametern initialisieren.        *
* Loeschen des Quellrufzeichens, Setzen von FRACK, MAXFRAME, RETRY.        *
*                                                                          *
\**************************************************************************/

void inilbl()
  {
    cpyid(lnkpoi->srcid,myid); /* FEF Quellcall des Linkblocks auf myid[] */
    lnkpoi->IRTT    = Fpar;
    lnkpoi->k       = Opar;
    lnkpoi->N2      = Npar;
#ifdef USE_XPID
    lnkpoi->txPID   = TXPIDDEFAULT; /* DL4YBG */
#endif
  }





/**************************************************************************\
* [DG2FEF]                                                                 *        *
* "send one outstanding I"                                                 *                     *
* Aus dem Aktuellen Linkblock wird ein Frame gesendet. Der alte Z„hler-    *
* stand V(S) wird gerettet und anschlieáend wieder hergestellt             *
*                                                                          *
\**************************************************************************/

void sdoi()
  {
   lnkpoi->RTT = 0;
   if (lnkpoi->VS != lnkpoi->lrxdNR)
    {
     lnkpoi->flag |= L2FMAXF1;                      /* MaxFrame 1 !    */
     lnkpoi->oldVS = lnkpoi->VS;                    /* FEF V(S) retten */
     lnkpoi->VS = lnkpoi->lrxdNR;                   /* V(S) resetten   */
     sdi(1);                                        /* 1 I senden      */
     lnkpoi->VS = lnkpoi->oldVS;         /* FEF V(S) wieder herstellen */
    }
  }





/**************************************************************************\
*                                                                          *
* "send I"                                                                 *
*                                                                          *
* Aus dem aktuellen Linkblock (lnkpoi) maximal max I-Frames aus der        *
* Infomessageliste aufbauen und senden. Infos ueber 256 Bytes werden       *
* gesplittet und dann gesendet. Die Frames werden als Commandframes ohne   *
* Poll/Final-Bit gesendet. V(S) wird fuer jedes gesendete Frame erhoeht    *
* modulo 7. Timer 2 und Timer 3 werden abgeschaltet.                       *
*                                                                          *
\**************************************************************************/

void sdi(max)

unsigned short max;

  {
    static unsigned short    n;                    /* Zaehler zu sendende Infos */
    static MBHEAD     *sendip;               /* Kopfzeiger Infobuffer     */
    static MBHEAD     *fbp;                  /* Kopfzeiger Framebuffer    */

    for ( n = 0, sendip = (MBHEAD *)lnkpoi->sendil.head;
                                                  /* maximal max I-Frames */
          n < lnkpoi->tosend && n < max;          /* aus der Linkblock-   */
          ++n, sendip = sendip->nextmh            /* infoliste senden     */
        )                                         /* wenn vorhanden       */
      {
        stxfad();                            /* Frameadresse aufbauen     */
        txfV2 = YES;                         /* Version                   */
        txfCR = L2CCR;                       /* Command !                 */
        txfPF = 0;                           /* kein Poll/Final           */
        txfctl = setNR(lnkpoi->VS << 1);     /* Controlbyte I setzen      */
        ++lnkpoi->VS;                        /* V(S) erhoehen             */
        lnkpoi->VS &= 0x07;                  /* modulo 7                  */

        if (n == max-1 || n == lnkpoi->tosend-1)   /* letzter Info-Frame? */
         {
          fbp = makfhd(L2FT1ST|L2FRTTST);    /* Ja, T1 und RTT-Messung an */
          if (!lnkpoi->RTT)                  /* FEF wenn keine Messung    */
            lnkpoi->RTTNR = lnkpoi->VS;      /* FEF VS fr RTT merken     */
         }
        else
          fbp = makfhd(0x00);                /* Nein, T1/RTT nicht starten!*/

        putchr(sendip->l2fflg, fbp);         /* Frame aufbauen, PID       */
        if (splcpy(256,fbp,sendip) == YES)   /* Message hineinkopieren    */
          ++lnkpoi->tosend;                  /* falls Split eine mehr     */
        sdl2fr(fbp);                         /* Frame senden              */
        if (!sendok)                         /* FEF noch nicht gepollt?   */
          lnkpoi->flag |= L2FJSENT;          /* FEF Frames in TX-Liste    */
        clrT2();                             /* Timer 2 abschalten        */
        clrT3();                             /* Timer 3 abschalten        */
      }
  }



/**************************************************************************\
*                                                                          *
* "send I with POLL-Bit"  (DK6PX)                                          *
* FEF                                                                      *
* Ein I-Frame aus Infomessageliste aufbauen, POLL-Bit setzen und senden.   *
*                                                                          *
\**************************************************************************/

void sdipoll()
 {
    static MBHEAD     *sendip;               /* Kopfzeiger Infobuffer     */
    static MBHEAD     *fbp;                  /* Kopfzeiger Framebuffer    */

    sendip = (MBHEAD *)lnkpoi->sendil.head;
    stxfad();                                /* Frameadresse aufbauen     */
    if (   sendip->mbpc < Ipar               /* FEF wird's ein IPoll ?    */
        && lnkpoi->tries < 3
        && lnkpoi->tosend)
     {
      txfV2 = YES;                           /* Version                   */
      txfCR = L2CCR;                         /* Command !                 */
      txfPF = L2CPF;                         /* Poll/Final !              */
      txfctl = setNR(lnkpoi->lrxdNR << 1);   /* Controlbyte I wie zuvor   */
      putchr(sendip->l2fflg,                 /* Frame aufbauen, PID       */
             fbp = makfhd(L2FT1ST));
      splcpy(256,fbp,sendip);                /* Message umkopieren        */
      sdl2fr(fbp);                           /* Frame senden              */
      clrT2();                               /* Timer 2 abschalten        */
      clrT3();                               /* Timer 3 abschalten        */
     }
    else                                     /* FEF Nein, kein Ipoll...   */
      xrrc();                                /* FEF ... RR+ senden        */
 }


/**************************************************************************\
*                                                                          *
* "split copy"                                                             *
*                                                                          *
* Die Bytes aus dem Messagebuffer, auf dessen Kopf mbhd zeigt, in den      *
* Framebuffer, auf dessen Kopf fbp zeigt, kopieren. Es werden hoechstens   *
* max Bytes kopiert, hat die Message mehr Bytes, so wird ein neuer         *
* Messagebuffer angelegt, die restlichen Messagebytes werden in diesen     *
* Buffer kopiert, der neue Buffer wird hinter den alten Messagebuffer      *
* gehaengt, der Putcount des alten Buffers wird auf max gestellt, das      *
* "more follows"-Flag morflg des neuen Buffers wird geloescht, l2fflg wird *
* uebertragen.                                                             *
*                                                                          *
* Return :  YES - der Messagebuffer wurde aufgesplittet                    *
*           NO  - sonst                                                    *
*                                                                          *
\**************************************************************************/

BOOLEAN splcpy(max,fbp,mbhd)

unsigned short    max;
MBHEAD     *fbp;
MBHEAD     *mbhd;

  {
    char       *mbbpsa;                 /* Sicherung mbbp                 */
    BOOLEAN     split;                  /* TRUE: Split erfolgt            */
    unsigned short    mbgcsa;                 /* Sicherung mbgc                 */
    unsigned short    mbgc2;                  /* mbgc alt -> mbpc alt           */
    unsigned short    n;                      /* Zaehler                        */
    MBHEAD     *mbhd2;                  /* Kopfzeiger neuer Messagebuffer */

    split = NO;                         /* zunaechst nichts gesplittet    */
    mbbpsa = mbhd->mbbp;                /* Bufferpointer sichern          */
    mbgcsa = mbhd->mbgc;                /* Getcounter sichern             */
    for (n = 0; mbhd->mbgc < mbhd->mbpc && n < max; ++n)
      putchr(getchr(mbhd),fbp);         /* maximal max Bytes kopieren     */
    if (mbhd->mbgc < mbhd->mbpc)        /* noch Bytes ueber -> Split !    */
      {
        mbgc2 = mbhd->mbgc;             /* Getcount fuer spaeter merken   */
        mbhd2 = (MBHEAD *)allocb();     /* neuen Buffer erzeugen          */
        while (mbhd->mbgc < mbhd->mbpc) /* die restlichen Bytes in diesen */
          putchr(getchr(mbhd),mbhd2);   /* Buffer kopieren                */
        rwndmb(mbhd2);                  /* neuen Buffer rewinden          */
        mbhd2->morflg = NO;             /* noch dem neuen folgt keiner    */
        mbhd2->l2fflg = mbhd->l2fflg;   /* Frameflag uebertragen          */
        relink(mbhd2,mbhd);             /* neu. Buf. hinter alten haengen */
        mbhd->mbpc = mbgc2;             /* alter Buffer nur max Zeichen ! */
        split = YES;                    /* wir mussten splitten           */
      }
    mbhd->mbbp = mbbpsa;                /* Bufferpointer restaurieren     */
    mbhd->mbgc = mbgcsa;                /* Getcount restaurieren          */
    return (split);                     /* Split oder nicht               */
  }





/**************************************************************************\
*                                                                          *
* "send UI"                                                                *
*                                                                          *
* UI-Frame aufbauen und senden. Das UI-Frame wird an ID dest geschickt     *
* ueber den Port port und die via-Liste (nullterminiert) vial, als Quelle  *
* wird source genommen, die Infobytes des Frames stehen im Messagebuffer,  *
* auf dessen Kopf mbhd zeigt, die PID wird aus l2fflg dieses Buffers       *
* genommen.                                                                *
*                                                                          *
\**************************************************************************/

void sdui(vial,dest,source,port,mbhd)

char     *vial;
char     *dest;
char     *source;
unsigned short  port;
MBHEAD   *mbhd;

  {
    MBHEAD        *fbp;                           /* Zeiger auf Framekopf */

    cpyid(txfhdr + L2IDLEN,source);               /* Quelle setzen        */
    cpyid(txfhdr,dest);                           /* Ziel setzen          */
    cpyidl(txfhdr + L2ILEN,vial);                 /* via-Liste setzen     */
    txfprt = port;                                /* Port setzen          */
    txfV2 = YES;                                  /* Protokollversion     */
    txfCR = L2CCR;                                /* Command !            */
    if(UIpar)
      txfPF = L2CPF;                              /* Poll-Bit gesetzt!    */
    else
      txfPF = 0;                                  /* kein Poll/Final      */
    txfctl = L2CUI;                               /* UI-Frame !           */
    putchr(mbhd->l2fflg,fbp = makfhd(0));         /* Frame aufbauen, PID  */
    while (mbhd->mbgc < mbhd->mbpc)               /* Message -> Frame     */
      putchr(getchr(mbhd),fbp);
    sdl2fr(fbp);                                  /* Frame senden         */
  }





/**************************************************************************\
*                                                                          *
* "level 2 state machine"                                                  *
*                                                                          *
* Ausfuehren der Zustandsuebergangsfunktion des Linkstatus (state) des     *
* aktuellen Linkblocks (lnkpoi) in der Statetable stbl, danach einnehmen   *
* des durch die Statetable gegebenen neuen Zustands.                       *
*                                                                          *
* In der Protokollversion 1 (dort gibt es nur 5 Zustaende), alle Zustaende *
* oberhalb Informationstransfer auf Informationstransfer setzen.           *
*                                                                          *
*                                                                          *
\**************************************************************************/

void l2stma(stbl)

STENTRY stbl[];

  {                                                     /* FEF */
    if (   (lnkpoi->flag & L2FPOLL)               /* wenn gepollt ...     */
        && lnkpoi->state == L2SDSCRQ)             /* und State DSCRQ      */
      {                                           /* State 3 nach 0       */
       xdm();                                     /* DM- senden           */
       lnkpoi->state = L2SDSCED;                  /* State DISCED         */
       mclrlk();                                  /* aufl”sen/melden      */
      }
    else
      {
       (*stbl[lnkpoi->state].func)();                 /* Funktion     */
       lnkpoi->state = stbl[lnkpoi->state].newstate;  /* neuer Status */
      }
  }



/* Ende von L2B.C */
