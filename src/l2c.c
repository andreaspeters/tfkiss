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
*          *****          *****          The Firmware. The Net.            *
*        *****              *****        Portable. Compatible.             *
*      *****                  *****      Public Domain.                    *
*    *****                      *****    By NORD><LINK.                    *
*                                                                          *
*                                                                          *
*                                                                          *
*    L2C.C   -   Level 2, Teil 3                                           *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 25.04.91                                       *
*                   sdl2fr():  Bei DAMA Betrieb kein kicktx(0)             *
*                   l2tolx():  DAMA Betrieb einschalten bei Connect und    *
*                              DAMA-Master, DAMA Betrieb ausschalten, wenn *
*                              Disconnect und keine weiteren DAMA Masters  *
*                                                                          *
*                   DL8ZAW, 04.05.91                                       *
*                   clrT1():   RTT-Messung beenden.                        *
*                              SRTT aus gemessenem RTT-Wert berechnen:     *
*                              - bei gestiegenem RTT:                      *
*                                SRTT' = (a1 x SRTT + RTT) / (a1 + 1)      *
*                              - bei gefallenem RTT:                       *
*                                SRTT' = (a2 x SRTT + RTT) / (a2 + 1)      *
*                   setT1():   RTT-Messung starten.                        *
*                              T1 nach aktuellem SRTT-Wert setzen:         *
*                              T1 = a3 x SRTT                              *
*                   (a1, a2 und a3 koennen mit @A1, @A2 und @A3-Befehlen   *
*                    geaendert werden.)                                    *
*                                                                          *
*                   setT2():   Im DAMA-Modus T2 auf 1 setzen, d.h.         *
*                              praktisch auf T2-Timer verzichten.          *
*                                                                          *
*                   DB2OS, 03.09.91:                                       *
*                   clrT1():   Neue Grenzen in der Plausibilit„tsabfrage   *
*                              fr den RTT, aufrund verbesserter RTT-      *
*                              Zeitbestimmung im L2B.C, sdi().             *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   Code, der nicht fuer TheFirmware benoetigt wird        *
*                   entfernt.                                              *
*                                                                          *
*                   DB2OS, 21.07.93                                        *
*                   clrT1():  DG6MAY - m”glicher Zahlenberlauf bei        *
*                             SRTT-Berechnung. Daher in der Zwischen-      *
*                             berechnung in 100ms rechnen...               *
*                   DL4YBG, 23.10.93                                       *
*                   setT1():  T1 aus IT1 fuellen                           *
*                   clrT1():  Aenderung nach DG6MAY rueckgaengig gemacht,  *
*                             Grenzwerte RTTMIN und RTTMAX eingefuehrt     *
*                                                                          *
*                   DG2FEF, 18.07.94                                       *
*                   clrT1(),: SRTT-Berechnung rausgeworfen.                *
*                   setT1()                                                *
*                   clrRTT(): SRTT-Berechung findet jetzt hier statt       *
*                   stxcfr(): Als Slave DISC^ bzw. SABM^ senden statt      *
*                             DISC+ bzw. SABM+                             *
*                   istome(): Alias-Abfrage entfernt.                      *
*                                                                          *
*                   DG2FEF, 13.08.94                                       *
*                   stxcfr(): Žnderung wieder rckg„ngig gemacht, dafr    *
*                             nun Patches in xrrc() xsabm() xdisc()        *
*                                                                          *
*                   DB2OS, 12.08.94                                        *
*                             Bei der SRTT-Berechnung keine getrennten     *
*                             A1,A2 mehr.                                  *
*                                                                          *
*                   DB2OS/DJ9KC, 15.09.94                                  *
*                   xrrr():   Best„tigen mit I-Frames erm”glichen.         *
*                                                                          *
*                   DB2OS, 16.09.94                                        *
*                   xrrr():   Žnderung noch einmal zurckgestellt, da noch *
*                             nicht mit FLEXNET-DAMA kompatibel.           *
*                             Žnderung ist aber in Aussicht gestellt..     *
*                                                                          *
*                   DL4YBG, 26.02.95 (DG0FT, 16.01.95)                     *
*                   takfhd(): bei Fehler wird Frame an takflx() weiter-    *
*                             gereicht, sonst rxfflx = FALSE setzen        *
*                   takflx(): FlexNet-Frame mit Headerkompression          *
*                             analysieren, nur fuer Monitor                *
*                                                                          *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "l2s.h"         /* Zugriff auf die State-Tabellen                */
#include "l2ext.h"       /* globale Variable / nicht int-Funktionen       */

/* external function declarations */
extern void DIinc();
extern void decEI();
extern void kicktx();
extern void l2tol7();
extern void putchr();
extern void rwndmb();
extern LEHEAD *tfunlink();

/* local function declarations */
void clrT3();
void cpyid();
void cpyidl();
void dealmb();
void dealoc();
void putfid();
void putvia();
void sdl2fr();
void sendS();
void setT2();
void setT3();
void stxcfr();
void stxfad();
void xrnrr();
void xrrr();


/**************************************************************************\
*                                                                          *
* action      :  Zustandsuebergangsfunktionen der Level-2-Statetable       *
*                (x.../t...), Level-2-Timer setzen/aufloesen (setT./clrT.) *
*                und Utilities fuer diese Funktionen.                      *
*                                                                          *
*   t2rrr()   -  Timer 2 setzen, nach Ablauf RR als Response zu senden     *
*   t2rnrr()  -  Timer 2 setzen, nach Ablauf RNR als Response zu senden    *
*   t2rejr()  -  Timer 2 setzen, nach Ablauf REJ als Response zu senden    *
*                                                                          *
*   xnull()   -  nichts tun                                                *
*                                                                          *
*   xrrc()    -  RR als Command senden      [DG2FEF Nicht bei DAMA]        *
*   xrrr()    -  RR als Response senden                                    *
*   xrnrc()   -  RNR als Command senden                                    *
*   xrnrr()   -  RNR als Response senden                                   *
*   xrejr()   -  REJ als Response senden                                   *
*                                                                          *
*   xdm()     -  DM senden                                                 *
*   xua()     -  UA senden                                                 *
*   xsabm()   -  SABM senden                [DG2FEF Bei DAMA ohne Pollbit] *
*   xdisc()   -  DISC senden                [DG2FEF Bei DAMA ohne Pollbit] *
*                                                                          *
*   xfrmr()   -  FRMR senden (-> L2E.C)                                    *
*                                                                          *
*   setT1()   -  Timer 1 setzen und Timer 3 loeschen                       *
*   clrT1()   -  Timer 1 und tries loeschen, Timer 3 setzen                *
*   setT2()   -  Timer 2 und nach Ablauf zu sendenden Frametyp setzen      *
*   clrT2()   -  Timer 2 und nach Ablauf zu sendenden Frametyp loeschen    *
*   setT3()   -  Timer 3 setzen, wenn Version 2 Protokoll benutzt wird     *
*   clrT3()   -  Timer 3 loeschen                                          *
*                                                                          *
*   sendS()   -  Supervisory-Frame fuer Sendung aufbauen, Timer 2          *
*                loeschen, Frame senden                                    *
*   sdfrmr()  -  FRMR-Frame fuer Sendung aufbauen und senden (-> L2E.C)    *
*                                                                          *
\**************************************************************************/

void t2rrr()  { setT2(L2CRR);                                              }

void t2rnrr() { setT2(L2CRNR);                                             }

void t2rejr() { setT2(L2CREJ);                                             }

void setT1()
{
  lnkpoi->T1 = lnkpoi->IT1;     /* Timer1 aktualisieren [DL4YBG] */
  clrT3();
}

/**************************************************************************\
*                                                                          *
* "clear Round Trip Timer" [DG2FEF]                                        *
*                                                                          *
* RTT-Messung beenden, SRTT und T1-Startwert berechnen                     *
* RTT-Messung ist nur gltig, wenn T1 zwischendurch nicht abgelaufen war   *
* ansonsten ist irgendwas schiefgelaufen und die ermittelte RTT m”glicher- *
* weise fehlerhaft                                                         *
*                                                                          *
\**************************************************************************/
void clrRTT()
{
  if (   !(lnkpoi->flag & L2FDIRTY)  /* RTT-Messung "schmutzig", T1 war  */
      && lnkpoi->RTT != 0)           /* wohl mal abgelaufen              */
    {
    if (lnkpoi->RTT < RTTMIN)
      lnkpoi->RTT = RTTMIN;
    else if (lnkpoi->RTT > RTTMAX)
      lnkpoi->RTT = RTTMAX;
    lnkpoi->SRTT = (10*(lnkpoi->SRTT)+(lnkpoi->RTT))/(11);    /* [DB2OS] */
    lnkpoi->IT1 = A3par * lnkpoi->SRTT;
    }
  lnkpoi->RTT = 0;
  lnkpoi->flag &= ~L2FDIRTY;
}


/**************************************************************************\
* "clear Timer1"                                                           *
* FEF                                                                      *
*                                                                          *
\**************************************************************************/
void clrT1()
{
  lnkpoi->T1  = 0;
  lnkpoi->tries = 0;
  setT3();
}

void setT2(Stype)
char Stype;
{
  lnkpoi->RStype = Stype;
  lnkpoi->T2 = T2par;
}

void clrT2()  { lnkpoi->RStype = 0; lnkpoi->T2 = 0;                        }

void setT3()  { lnkpoi->T3 = damaok ? 18000 : T3par;                       }

void clrT3()  { lnkpoi->T3 = 0;                                            }

void xnull()  {                                                            }

void xrrc()   { if (!(lnkpoi->flag & L2FDAMA)) { stxcfr(); xrrr(); }       }

#ifdef FLEXNETOK          /* Zur Zeit nur mit TheNetNode kompatibel!!!    */
void xrrr()               /* Bei DAMA nur RR senden, wenn keine Infos zu  */
{                         /* senden sind oder kein IXFER m”glich ist..    */
  if (    damaok == 0
      || (damaok == 1 && (lnkpoi->tosend == 0 || lnkpoi->state != L2SIXFER))
     )
    sendS(L2CRR);
}
#else
void xrrr()   { sendS(L2CRR);                                              }
#endif


void xrnrc()  { stxcfr(); xrnrr();                                         }

void xrnrr()  { sendS(L2CRNR);                                             }

void xrejr()  { sendS(L2CREJ);                                             }

void sendS(control) char control;
              { clrT2(); txfctl=setNR(control);
                sdl2fr(makfhd(!txfCR ? L2FUS : (L2FUS | L2FT1ST))); }

void xdm()    { txfctl = L2CDM; sdl2fr(makfhd(L2FUS));                     }

void xua()    { txfctl = L2CUA; sdl2fr(makfhd(L2FUS));                     }

void xsabm()  { if (damaok) { stxfad(); txfPF = 0; txfCR = L2CCR; } else
                stxcfr(); txfctl = L2CSABM; sdl2fr(makfhd(L2FUS|L2FT1ST)); }

void xdisc()  { if (damaok) { stxfad(); txfPF = 0; txfCR = L2CCR; } else
                stxcfr(); txfctl = L2CDISC; sdl2fr(makfhd(L2FUS|L2FT1ST)); }





/**************************************************************************\
*                                                                          *
* "set tx command frame"                                                   *
*                                                                          *
* TX-Frame-Adressierung setzen (siehe stxfad()) und Frame zum Kommando-    *
* frame machen mit gesetztem Pollbit (txfCR,txfPF).                        *
*                                                                          *
\**************************************************************************/

void stxcfr()
  {
    stxfad();                           /* Adressierung                   */
    txfCR = L2CCR;                      /* Command !                      */
    txfPF = L2CPF;                      /* Pollbit !                      */
  }


/**************************************************************************\
*                                                                          *
* "set tx frame address"                                                   *
*                                                                          *
* Adressierung des aktuellen Sendeframes (txfhdr, txfprt) setzen aus den   *
* im aktuellen Linkblock (lnkpoi) gegebenen Parametern (srcid, destid,     *
* viaidl, liport).                                                         *
*                                                                          *
\**************************************************************************/

void stxfad()
  {
    cpyid(txfhdr + L2IDLEN,lnkpoi->srcid);        /* von ...              */
    cpyid(txfhdr,lnkpoi->dstid);                  /* nach ...             */
    cpyidl(txfhdr + L2ILEN,lnkpoi->viaidl);       /* ueber ...            */
    txfprt = lnkpoi->liport;                      /* auf Port ...         */
  }





/**************************************************************************\
*                                                                          *
* "set NR"                                                                 *
*                                                                          *
* Im aktuellen Linkblock (lnkpoi) die zuletzt gesendete N(R) (ltxdNR) auf  *
* V(R) (VR) setzen und Framecontrolbyte control fuer Frameaussendung mit   *
* der N(R) versehen und zurueckgeben.                                      *
*                                                                          *
* Return :  control mit N(R) versehen                                      *
*                                                                          *
\**************************************************************************/

unsigned setNR(control)

char control;

  {
    lnkpoi->ltxdNR = lnkpoi->VR;             /* neue N(R)                 */
    return((lnkpoi->VR << 5) | control);     /* N(R) ins Kontrollfeld     */
  }





/**************************************************************************\
*                                                                          *
* "send level 2 frame"                                                     *
*                                                                          *
* Framebuffer, auf dessen Kopf fbp zeigt, rewinden und in die dem Port     *
* (l2port) entsprechende Level-2-Sendeframeliste einhaengen, wenn noch     *
* genug Buffer im System frei sind. Andernfalls nicht senden, sondern      *
* sofort in die Gesendet-Liste (stfl) einhaengen. Bei TheNet die           *
* Sendeaktiviatetsvariable (istraf) des entsprechenden Ports setzen.       *
* Bei DAMA Betrieb kein kicktx()                                           *
*                                                                          *
\**************************************************************************/

void sdl2fr(fbp)

MBHEAD *fbp;

  {
    static unsigned port;               /* Portnummer                     */

    port = fbp->l2port;                 /* Portnummer holen               */
    if (nmbfre > TXBREMSE)              /* noch genug Buffer ?            */
      {
        rwndmb(fbp);                    /* ja   - Framebuffer rewinden    */
        DIinc();                        /*        Listenkonsistenz !      */
        relink(fbp,txl2fl[port].tail);  /*        Frame in Sendeliste     */
        if (!damaok || sendok)          /* DAMA:  nicht senden            */
          {
            kicktx(port);               /*        es ist sofort zu senden */
            tosend = 0;
          }
        else
          tosend = 1;                   /* DAMA:  es ist spaeter zu senden*/
        decEI();                        /*        Interrupts w. erlauben  */

      }
    else                                /* nein - Frame einfach sofort    */
      relink(fbp,stfl.tail);            /*        als gesendet betrachten */
  }





/**************************************************************************\
*                                                                          *
* "copy frame buffer"                                                      *
*                                                                          *
* Framebuffer, auf den fbp zeigt, komplett mit Inhalt kopieren. Dazu freie *
* Buffer allokieren, Portnummer (l1port) wird kopiert, Bufferzeiger (mbbp) *
* und Getcounter (mbgc) werden nicht kopiert, bleiben aber im Quellframe   *
* erhalten.                                                                *
*                                                                          *
* Return :  Zeiger auf Kopf des kopierten Framebuffers                     *
*                                                                          *
\**************************************************************************/

MBHEAD *cpyfb(fbp)

MBHEAD *fbp;

  {
    static char       *savmbbp;         /* mbbp-Sicherung                 */
    static unsigned    savmbgc;         /* mbgc-Sicherung                 */
    static MBHEAD     *newfbp;          /* Zeiger auf die Kopie           */

    savmbbp = fbp->mbbp;                /* mbbp sichern                   */
    savmbgc = fbp->mbgc;                /* mbgc sichern                   */
    rwndmb(fbp);                        /* Quellframe rewinden            */
    newfbp = (MBHEAD *)allocb();        /* Kopf der Kopie allokieren      */
    while (fbp->mbgc < fbp->mbpc)       /* Daten byteweise kopieren       */
      putchr(getchr(fbp),newfbp);
    newfbp->l2port = fbp->l2port;       /* Portnummer kopieren            */
    fbp->mbbp = savmbbp;                /* mbbp wieder auf alten Wert     */
    fbp->mbgc = savmbgc;                /* mbgc wieder auf alten Wert     */
    return (newfbp);                    /* Zeiger auf Kopf der Kopie      */
  }





/**************************************************************************\
*                                                                          *
* "take frame head"                                                        *
*                                                                          *
* Adresskopf und Kontrollbyte des Frames aus dem Framebuffer, auf dessen   *
* Kopf fbp zeigt, analysieren. Diese Funktion ist die erste, die auf ein   *
* empfangenes Frame angewandt wird.                                        *
*                                                                          *
*                                                                          *
* Folgende Parameter werden bei der Analyse gesetzt (siehe auch L2V.C) :   *
*                                                                          *
*    rxfhdr, rxfV2, rxfPF, rxfCR, rxfctl, rxfprt                           *
*                                                                          *
*                                                                          *
* Folgende Parameter werden nach der Analyse gesetzt fuer ein moegliches   *
* Antwortframe :                                                           *
*                                                                          *
*   txfhdr  = Quell- und Zielcall aus rxfhdr, aber vertauscht, plus        *
*             reverse via-Liste aus rxfhdr                                 *
*   txfV2   = rxfV2                                                        *
*   txfPF   = rxfPF                                                        *
*   txfCR   = 0, Response !                                                *
*   txfprt  = rxfprt                                                       *
*                                                                          *
*                                                                          *
* Return :  TRUE  - das Frame hat einen gueltigen AX.25-Framekopf          *
*           FALSE - sonst                                                  *
*                                                                          *
\**************************************************************************/

BOOLEAN takfhd(fbp)

MBHEAD *fbp;

  {
    static char *viap;                            /* Zeiger in via-Liste  */
    static char *source;                          /* Quellzeiger Kopien   */
    static char *dest;                            /* Zielzeiger Kopien    */

    rwndmb(fbp);                                  /* Frame von vorne      */
#ifndef USE_FLEXDEC
    if (    !getfid(rxfhdr,fbp)                   /* Zielcall holen       */
         || ((rxfhdr[L2IDLEN - 1] & L2CEOA) != 0) /* (Ende nach 1. Call ?)*/
#else
    if (!getfid(rxfhdr,fbp))                      /* Zielcall holen       */
      return (takflx(fbp));                       /* ... FlexNet-Frame?   */
    if (    ((rxfhdr[L2IDLEN - 1] & L2CEOA) != 0) /* (Ende nach 1. Call ?)*/
 #endif
         || !getfid(rxfhdr + L2IDLEN,fbp)         /* Quellcall holen      */
       ) return (FALSE);                          /* ... schon Fehler     */
    viap = rxfhdr + L2ILEN;                       /* ab hier via-Liste    */
    if (!(rxfhdr[L2ILEN - 1] & L2CEOA))           /* via-Liste da ?       */
      LOOP
        {                                               /* alle via's     */
          if (!getfid(viap,fbp)) return (FALSE);        /* Call holen     */
          viap += L2IDLEN;                              /* naechstes Call */
          if ((*(viap - 1) & L2CEOA) != 0) break;       /* Ende der Liste */
          if (viap >= rxfhdr + L2AFLEN) return (FALSE); /* zu lange Liste */
        }
    *viap = '\0';                                       /* Listenende !   */
    if (fbp->mbgc == fbp->mbpc) return (FALSE);         /* Frame zu kurz  */
    rxfctl = getchr(fbp);                               /* Controlbyte    */


    /* Protokollversion feststellen und danach C/R und P/F festlegen */

    if ( (rxfV2 = ((rxfhdr[L2IDLEN - 1] ^ rxfhdr[L2ILEN - 1]) & L2CCR) != 0)
         == YES
       )
      {                                           /* nur Version 2 :      */
        rxfCR = rxfhdr[L2IDLEN - 1] & L2CCR;      /*   Command/Response   */
        rxfPF = rxfctl & L2CPF;                   /*   Poll/Final         */
      }
    else                                          /* Version 1 :          */
      rxfPF = rxfCR = 0;                          /* P/F u. C/R sinnlos   */

    rxfctl &= ~L2CPF;                             /* P/F Control loeschen */
    rxfprt = fbp->l2port;                         /* Portnummer holen     */


    /* Antwort-Sendeframeaufbau */

    txfCR = 0;                                    /* Response !           */
    txfV2 = rxfV2;                                /* Version              */
    txfPF = rxfPF;                                /* Poll/Final           */
    txfprt = rxfprt;                              /* Portnummer           */
    cpyid(txfhdr,rxfhdr + L2IDLEN);               /* TX-Ziel = RX-Quelle  */
    cpyid(txfhdr + L2IDLEN,rxfhdr);               /* TX-Quelle = RX-Ziel  */
    source = rxfhdr + L2ILEN;                     /* TX-Antwort-via-Liste */
    dest = txfhdr + L2ILEN;                       /* ist, falls vorhanden */
    while (*source != '\0') source += L2IDLEN;    /* reverse RX-via-Liste */
    while (source != rxfhdr + L2ILEN)
      {
        source -= L2IDLEN;
        cpyid(dest,source);
        dest += L2IDLEN;
      }
    *dest = '\0';                                 /* Listenende !         */

#ifdef USE_FLEXDEC
    rxfflx = FALSE;                               /* normaler Frame       */
#endif
    return (TRUE);                                /* Frame soweit okay !  */
  }


#ifdef USE_FLEXDEC
/**************************************************************************\
*                                                                          *
* "take FlexNet frame head"                                      (DG0FT)   *
*                                                                          *
* Adresskopf und Kontrollbyte eines FlexNet-Frames mit Headerkompression   *
* analysieren. Wird von takfhd() aufgerufen, wenn Frame keinen normalen    *
* AX.25-Header hat.                                                        *
*                                                                          *
*                                                                          *
* Folgende Parameter werden bei der Analyse gesetzt (siehe auch L2V.C) :   *
*                                                                          *
*    rxfhdr, rxfV2, rxfPF, rxfCR, rxfctl, rxfprt                           *
*                                                                          *
*                                                                          *
* Es werden keine Parameter fuer ein Antwortframe gesetzt!                 *
*                                                                          *
*                                                                          *
* Return :  TRUE  - das Frame hat einen gueltigen FlexNet-Framekopf        *
*           FALSE - sonst                                                  *
*                                                                          *
\**************************************************************************/

BOOLEAN takflx(fbp)

MBHEAD *fbp;

  {
    static char       flx[7], *hdr;
    static unsigned   qso, div, digit, n;
    static BOOLEAN    out;

    rwndmb(fbp);                                   /* Frame von vorne     */
    if ((fbp->mbpc - fbp->mbgc) < 8) return (FALSE); /* Frame zu kurz       */
    for (n = 0; n < 7; n++) flx[n] = getchr(fbp);  /* Header einlesen     */
    if (!(flx[1] & L2CEOA)) return (FALSE);        /* Ende-Bit fehlt      */

    hdr = rxfhdr;
    *hdr++ = 0x20 + ((flx[2] >> 2) & 0x3F);        /* Zielcall dekodieren */
    *hdr++ = 0x20 + (((flx[2] << 4) & 0x30) | ((flx[3] >> 4) & 0x0F));
    *hdr++ = 0x20 + (((flx[3] << 2) & 0x3C) | ((flx[4] >> 6) & 0x03));
    *hdr++ = 0x20 + (flx[4]      & 0x3F);
    *hdr++ = 0x20 + ((flx[5] >> 2) & 0x3F);
    *hdr++ = 0x20 + (((flx[5] << 4) & 0x30) | ((flx[6] >> 4) & 0x0F));
    *hdr++ = 0x60 | ((flx[6] << 1) & 0x1E);        /* SSID (ohne C/R-Bit) */

    qso = (unsigned) (((flx[0] << 6) & 0x3FC0) | ((flx[1] >> 2) & 0x3F));
                                                   /* ermittle QSO-Nummer */

    *hdr++ = '#';                                  /* Prefix fuer Nummer  */
    for (out = FALSE, n = 5, div = 10000; div; div /= 10)
                                                   /* Nummer konvertieren */
      if ((digit = qso / div) || n < 5 || div == 1 || out == TRUE)
        {
          *hdr++ = '0' + digit;
          qso %= div;
          n--;
          out = TRUE;
        }
    while (n--) *hdr++ = ' ';                      /* mit ' ' auffuellen  */
    *hdr++ = 0x61;                                 /* SSID = 0, EOA-Bit   */
    *hdr++ = '\0';                                 /* keine via-Liste     */

    rxfctl  = getchr(fbp);                         /* Controlbyte         */
    rxfV2   = TRUE;                                /* immer Version 2     */
    rxfCR   = (flx[1] & 2) ? L2CCR : 0;            /* Command/Response    */
    rxfPF   = rxfctl & L2CPF;                      /* Poll/Final          */
    rxfctl &= ~L2CPF;                              /* in Control loeschen */
    rxfprt  = fbp->l2port;                         /* Portnummer holen    */

    rxfflx = TRUE;                                 /* FlexNet-Frame       */
    return (TRUE);                                 /* Frame soweit okay ! */
  }
#endif


/**************************************************************************\
*                                                                          *
* "get frame ID"                                                           *
*                                                                          *
* Die naechste ID nach dest (Call + SSID, SSID wie im Frame) holen aus dem *
* Buffer (Call + SSID, beide wie im Frame), auf dessen Kopf mbhd zeigt.    *
* Die geholte SSID enthaelt das End-Of-Address-Bit unveraendert.           *
*                                                                          *
* Return :  TRUE  - die naechste ID (Call + SSID) wurde richtig geholt     *
*           FALSE - es hat sich ein Fehler ereignet                        *
*                                                                          *
\**************************************************************************/

BOOLEAN getfid(dest,mbhd)

char     *dest;
MBHEAD   *mbhd;

  {
    static char       c;                /* aktuelles Zeichen aus Buffer   */
    static unsigned   n;                /* Zaehler Call-Laenge            */

    if (mbhd->mbpc - mbhd->mbgc < L2IDLEN)        /* im Buffer nicht mehr */
      return (FALSE);                             /* genug Bytes fuer ID  */
    for (n = 0; n < L2CALEN; ++n)                 /* Call byteweise holen */
      {
        if (((c = getchr(mbhd)) & L2CEOA) != 0)   /* Adressfeld zu frueh  */
          return (FALSE);                         /* zuende               */
        *dest++ = (c >> 1) & 0x7F;                /* Framecall -> ASCII   */
      }
    *dest = getchr(mbhd);               /* SSID holen, EOA bleibt         */
    return (TRUE);
  }





/**************************************************************************\
*                                                                          *
* "make frame head"                                                        *
*                                                                          *
* Neues Frame aufbauen aus den txf...-Parametern. Es werden neue Buffer    *
* fuer das Frame allokiert, der aktuelle Linkblock (lnkpoi) wird           *
* eingetragen und fflag fuer das Frameflag l2fflg.                         *
*                                                                          *
* Return :  Zeiger auf Framebufferkopf des neu erzeugten Frames            *
*                                                                          *
\**************************************************************************/

MBHEAD *makfhd(fflag)

unsigned fflag;

  {
    static MBHEAD *fbp;                           /* Zeiger auf Kopf      */

    if (txfV2 == YES)                             /* wenn Version 2 ...   */
      {
        txfhdr[L2IDLEN - 1] |= txfCR;             /* ... C-Bits setzen    */
        txfhdr[L2ILEN - 1] |= txfCR ^ L2CCR;
      }
    putfid(txfhdr,fbp = (MBHEAD *)allocb());      /* neuer Buffer, Ziel   */
    putfid(txfhdr + L2IDLEN,fbp);                 /* Quellcall            */
    putvia(txfhdr + L2ILEN,fbp);                  /* via-Liste            */
    putchr(!txfV2 ? txfctl : txfctl | txfPF,fbp); /* Control + P/F        */
    fbp->l2link = lnkpoi;                         /* Verweis Linkblock    */
    fbp->type = 2;                                /* Level 2 !            */
    fbp->l2fflg = fflag;                          /* Frameflag            */
    fbp->l2port = txfprt;                         /* Portnummer           */
    return (fbp);                                 /* Kopfzeiger zurueck   */
  }





/**************************************************************************\
*                                                                          *
* "put via"                                                                *
*                                                                          *
* Nullterminierte via-Liste, auf die idl zeigt, in den Framebuffer, auf    *
* dessen Kopf mbhd zeigt, uebertragen. Die Nullterminierung nicht ueber-   *
* tragen, aber am Ende der via-Liste das letzte Zeichen der via-Liste mit  *
* dem HDLC End-Of-Address-Bit uebertragen.                                 *
*                                                                          *
\**************************************************************************/

void putvia(idl,mbhd)

char     *idl;
MBHEAD   *mbhd;

  {
    while (*idl != '\0')                /* gesamte via-Liste in den       */
      {                                 /* Framebuffer uebertragen        */
        putfid(idl,mbhd);
        idl += L2IDLEN;
      }                                 /* dann                           */
    *(mbhd->mbbp - 1) |= L2CEOA;        /* EoA direkt im Buffer setzen    */
  }





/**************************************************************************\
*                                                                          *
* "put frame id"                                                           *
*                                                                          *
* ID (Call und SSID, SSID wie im Frame), auf die id zeigt, in den          *
* Framebuffer, auf dessen Kopf mbhd zeigt, uebertragen. Dabei Call von     *
* ASCII in Frameformat (1 Bit linksgeschoben) umwandeln.                   *
*                                                                          *
\**************************************************************************/

void putfid(id,mbhd)

char     *id;
MBHEAD   *mbhd;

  {
    static unsigned n;                  /* Zaehler Call-Laenge            */

    for (n = 0; n < L2CALEN; ++n)       /* Call uebertragen in Buffer,    */
      putchr(*id++ << 1,mbhd);          /* 1 Bit linksgeschoben           */
    putchr(*id,mbhd);                   /* SSID unveraendert uebertragen  */
  }





/**************************************************************************\
*                                                                          *
* "is to me"                                                               *
*                                                                          *
* Pruefen, ob die ID (Call + SSID, SSID wie im Frame), auf die id zeigt,   *
* mit der ID der eigenen Station (myid) uebereinstimmt (SSID wird ohne     *
* Steuerbits verglichen), oder ob das Call, auf das id zeigt, mit dem      *
* symbolischen Namen (alias) der eigenen Station uebereinstimmt.           *
*                                                                          *
* Return :  TRUE  - myid stimmt mit id ueberein oder alias mit dem         *
*                   call in id                                             *
*           FALSE - sonst                                                  *
*                                                                          *
* FEF Alias-Name wird nicht mehr getestet                                  *
*                                                                          *
\**************************************************************************/

BOOLEAN istome(id)

char *id;

  {
    return (cmpid(myid,id) == TRUE);
  }





/**************************************************************************\
*                                                                          *
* "compare calls"                                                          *
*                                                                          *
* Calls miteinander vergleichen.                                           *
*                                                                          *
* Return :  TRUE  - die Calls stimmen ueberein                             *
*           FALSE - die Calls stimmen nicht ueberein oder mindestens eins  *
*                   der Calls beginnt mit einem Blank                      *
*                                                                          *
\**************************************************************************/

BOOLEAN cmpcal(call1,call2)

char *call1;
char *call2;

  {
    static unsigned n;                                 /* Zaehler         */

    for (n = 0; n < L2CALEN; ++n)                      /* jedes Zeichen   */
      if (    (!n && (*call2 == ' ' || *call1 == ' ')) /* 1. Zeich. ' ' ? */
           || (*call2++ != *call1++)                   /* sonst gleich ?  */
         ) return (NO);                                /* nein            */
    return (YES);                                      /* ja, alle gleich */
  }





/**************************************************************************\
*                                                                          *
* "compare ID list"                                                        *
*                                                                          *
* Nullterminierte ID-Listen (Calls + SSID's, SSID wie im Frame)            *
* miteinander vergleichen (SSID nur reine SSID 0-15 vergleichen ohne       *
* Steuerbits).                                                             *
*                                                                          *
* Return :  TRUE  - die ID-Listen stimmen ueberein                         *
*           FALSE - sonst                                                  *
*                                                                          *
\**************************************************************************/

BOOLEAN cmpidl(idl1,idl2)

char *idl1;
char *idl2;

  {
    while (*idl2 != '\0')                         /* bis Liste 2 zuende   */
      {
        if (!cmpid(idl1,idl2)) return (NO);       /* ID's vergleichen     */
        idl2 += L2IDLEN;                          /* Zeiger auf naechste  */
        idl1 += L2IDLEN;                          /* ID's                 */
      }                                           /* Listen gleich, wenn  */
    return (!*idl1);                              /* beide zuende         */
  }





/**************************************************************************\
*                                                                          *
* "compare ID's"                                                           *
*                                                                          *
* ID's (Call + SSID, SSID wie im Frame) miteinander vergleichen (SSID nur  *
* reine SSID 0-15 vergleichen ohne Steuerbits).                            *
*                                                                          *
* Return :  TRUE  - die ID's stimmen ueberein                              *
*           FALSE - sonst                                                  *
*                                                                          *
\**************************************************************************/

BOOLEAN cmpid(id1,id2)

char *id1;
char *id2;

  {
    static unsigned n;                            /* Zaehler              */

    for (n = 0; n < L2CALEN; ++n)                 /* Calls vergleichen    */
      if (*id2++ != *id1++) return (NO);
    return ((*id2 & 0x1E) == (*id1 & 0x1E));      /* reine SSID vergl.    */
  }





/**************************************************************************\
*                                                                          *
* "copy ID list"                                                           *
*                                                                          *
* Nullterminierte ID-Liste (Calls + SSID's, SSID wie im Frame) von source  *
* nach dest kopieren, Zielliste mit '\0' abschliessen.                     *
*                                                                          *
\**************************************************************************/

void cpyidl(dest,source)

char *dest;
char *source;

  {
    while (*source != '\0')             /* solange Liste nicht zuende     */
      {
        cpyid(dest,source);             /* ID kopieren                    */
        source += L2IDLEN;              /* Zeiger um eine ID-Laenge       */
        dest += L2IDLEN;                /* weiter                         */
      }
    *dest = '\0';                       /* Zielliste abschliessen         */
  }





/**************************************************************************\
*                                                                          *
* "copy ID"                                                                *
*                                                                          *
* Komplette ID (Call + SSID, SSID wie im Frame), auf die source zeigt,     *
* nach dest kopieren. In der kopierten SSID das End-Of-Address-Bit und das *
* Command/Response/Has-Been-Repeated-Bit loeschen.                         *
*                                                                          *
\**************************************************************************/

void cpyid(dest,source)

char *dest;
char *source;

  {
    static unsigned n;                    /* Laengenzaehler               */

    for (n = 0; n < L2CALEN; ++n)         /* Call kopieren                */
      *dest++ = *source++;
    *dest = *source & ~(L2CEOA | L2CCR);  /* SSID kopieren, Bits loeschen */
  }





/**************************************************************************\
*                                                                          *
* "deallocate message list"                                                *
*                                                                          *
* Komplette Messageliste, auf deren Listenkopf mlp zeigt, deallokieren.    *
* D.h. alle Messagespeicher (jeweils Kopf und daran haengende Datenbuffer) *
* deallokieren.                                                            *
*                                                                          *
*                                                                          *
*            +--------+    +--------+               +--------+             *
*    mlp --->| head   |--->|        |--->       --->|        |---> mlp     *
*            +--------+    +--------+      ...      +--------+             *
*      b <---| tail   |<---|        |<---       <---|        |<--- b       *
*            +--------+    +--------+               +--------+             *
*                          |        |---> \         |        |---> \       *
*                          +        +      |        +        +      |      *
*                          |        |<--- /|        |        |<--- /|      *
*                          +--------+      |        +--------+      |      *
*                          |        |      |        |        |      |      *
*                                          |                        |      *
*      deallokieren                        |------------------------|      *
*                                            siehe unten dealmb()          *
*                                                                          *
\**************************************************************************/

void dealml(mlp)

LEHEAD *mlp;

  {
    static MBHEAD *bp;                  /* Zeiger auf Messagebufferhead   */

    LOOP                                /* fuer alle Messagebufferheads   */
      {                                 /* in Messagespeicherliste :      */
        DIinc();                        /* Listenkonsistenz !             */
        bp = (MBHEAD *)mlp->nextle;     /* Zeiger auf naechsten Msgbhead  */
        decEI();                        /* Interrupts wieder erlauben     */
        if (mlp == (LEHEAD *)bp) break; /* Schwanz beisst Kopf -> fertig  */
        dealmb(tfunlink(bp));             /* sonst Messagespeicher deallok. */
      }
  }





/**************************************************************************\
*                                                                          *
* "deallocate message buffer"                                              *
*                                                                          *
* Einen kompletten Messagespeicher, auf dessen Kopf mbhd zeigt,            *
* deallokieren, d.h. sowohl den Messagebufferhead als auch alle an dessen  *
* Messagebufferliste haengende Datenbuffer deallokieren.                   *
*                                                                          *
*                                                                          *
*            +--------+           deallokieren                             *
*    mbhd -->|        |                                                    *
*            +--------+                                                    *
*            |        |                                                    *
*            +--------+      +--------+                +--------+          *
*      a --->|        |----->|        |--->        --->|        |---> a    *
*            +  mbl   +      +--------+      ...       +--------+          *
*      b <---|        |<-----|        |<---        <---|        |<--- b    *
*            +--------+      +--------+                +--------+          *
*            |        |      |        |                |        |          *
*                                                                          *
\**************************************************************************/

void dealmb(mbhd)

MBHEAD *mbhd;

  {
    static MB *bp;                                /* Datenbufferzeiger    */

    while ((bp = (MB *)mbhd->mbl.head) != (MB *)&mbhd->mbl)
                                                  /* alle Datenbuffer     */
      dealoc(tfunlink(bp));
    dealoc(mbhd);                                 /* am Ende den Kopf     */
  }





/**************************************************************************\
*                                                                          *
* "deallocate"                                                             *
*                                                                          *
* Buffer, auf den bp zeigt, initialisieren als neuen Messagebufferhead     *
* (rwndmb()) und deallokieren, d.h. in die Freiliste freel einhaengen und  *
* den Freibufferzaehler nmbfre inkrementieren.                             *
*                                                                          *
*                                                                          *
*            +--------+                                                    *
*     bp --->|        |         deallokieren                               *
*            +--------+                                                    *
*            |        |                                                    *
*            +--------+                                                    *
*            |        |                                                    *
*                                                                          *
\**************************************************************************/

void dealoc(bp)

MBHEAD *bp;

  {
      bp->mbl.head                      /* als Messagehead initialisieren */
    = bp->mbl.tail                      /*   Bufferlistenkopf             */
    = &bp->mbl;                         /*   initialisieren               */
    bp->mbpc = 0;                       /*   Message leer                 */
    rwndmb(bp);                         /*   Rest initialisieren          */
    DIinc();                            /* Listenkonsistenz !             */
    relink(bp,freel.tail);              /* Buffer an Freiliste anhaengen  */
    ++nmbfre;                           /* 1 Freibuffer mehr              */
    decEI();                            /* Interrupts wieder erlauben     */
  }





/**************************************************************************\
*                                                                          *
* "initialize head"                                                        *
*                                                                          *
* Listenkopf, auf den hd zeigt, initialisieren :                           *
*                                                                          *
*                                                                          *
*                                              +----------------------+    *
*                                              | +------------------+ |    *
*                                              | |                  | |    *
*            +--------+                        v v   +--------+     | |    *
*     hd --->|        |         ->        hd ------->|        |-----+ |    *
*            +--------+                              +--------+       |    *
*            |        |                              |        |-------+    *
*            +--------+                              +--------+            *
*            |        |                              |        |            *
*                                                                          *
\**************************************************************************/

void inithd(hd)

LHEAD *hd;

  {
    hd->head = hd->tail = hd;
  }





/**************************************************************************\
*                                                                          *
* "level 2 to level x"                                                     *
*                                                                          *
* Meldung msg (L2M...) an Layer 7 weitergeben.                             *
*                                                                          *
\**************************************************************************/

void l2tolx(msg)

unsigned msg;

  {
    l2tol7(msg,lnkpoi,2);               /* Layer 2  ->  Layer 7            */
  }





/**************************************************************************\
*                                                                          *
* "uppercase"                                                              *
*                                                                          *
* Zeichen c in Grossbuchstabe umwandeln, wenn Kleinbuchstabe, und          *
* zurueckgeben.                                                            *
*                                                                          *
\**************************************************************************/

short upcase(c)

char c;

  {
    return ( ('a' > c || 'z' < c) ? c : c - ('a' - 'A') );
  }



/* Ende von L2C.C */
