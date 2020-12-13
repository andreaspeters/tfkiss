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
*    TFD.C   -   The Firmware, Teil 4                                      *
*                                                                          *
*                Dieser Teil ist eine Zusammenfassung der                  *
*                zeitunkritischsten Funktionen und kann somit mit der      *
*                groessten Platzersparnis optimiert werden.                *
*                                                                          *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 04.05.91                                       *
*                   sl2par(): Initialisieren von ...                       *
*                             Fpar:  IRTT in 10 ms - Schritten             *
*                             A1par: A1 fuer SRTT-Berechnung               *
*                                    (steigendes RTT)                      *
*                             A2par: A2 fuer SRTT-Berechnung               *
*                                    (fallendes RTT)                       *
*                             Bpar:  B fuer Setzen von T1 (T1 = B x SRTT)  *
*                             Ipar:  max. Framelaenge fuer I-Poll          *
*                             DApar: DAMA-Timer in Sekunden                *
*                                    (nach Ablauf wird DAMA-Modus          *
*                                     abgeschaltet)                        *
*                             ... mit Werten aus Eprom                     *
*                   Bcmd():   Setzen/anzeigen des DAMA-Timers (s. DApar)   *
*                                                                          *
*                   DL8ZAW, 18.05.91                                       *
*                   sl2par(): Bpar durch A3par erstezt. (B = A3)           *
*                             M7par - Maskieren mit 7 bit (Terminalmodus)  *
*                                                                          *
*                   DL8ZAW, 05.06.91                                       *
*                   Bcmd(): ohne Parameter Anzeige in der Form:            *
*                           "aktueller Wert / Anfangswert" vom DAMA-Timer  *
*                                                                          *
*                   DL8ZAW, 10.06.91                                       *
*                   Bcmd(): beim Setzen von DApar gleichzeitig damati      *
*                           auf Wert von DApar setzen.                     *
*                                                                          *
*                   DL8ZAW, 25.07.91                                       *
*                   sl2par(): Initialisieren von Walt und Palt:            *
*                             Werte fuer Slottime und P-Persistance vor    *
*                             dem automatischen Umschalten in den DAMA-    *
*                             Modus                                        *
*                                                                          *
*                   DB2OS,  30.08.91                                       *
*                   Bcmd(): ohne Parameter jetzt Anzeige in der Form:      *
*                           "Anfangswert (aktueller Wert)", wie Ycmd()     *
*                                                                          *
*                   DB2OS,  17.09.91                                       *
*                   UIpar und defUIp fÅr UI+.                              *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   l3init(): entfernt (wird nicht benoetigt fuer TF)      *
*                                                                          *
*                   DB2OS,  23.06.92                                       *
*                   sl2par(): Walt entfernt, wegen Problemen mit DCD       *
*                                                                          *
*                   DB2OS,  18.10.92                                       *
*                   hpinch(): Echo - ^Zeichenwandlung nur bei 7Bit.        *
*                                                                          *
*                   DB2OS,  20.10.92                                       *
*                   delich(): BS/DEL - Spezialbehandlung bei '^'Ausgabe.   *
*                                                                          *
*                   DG0FT, DB2OS, 21.6.93                                  *
*                   Ucmd() geÑndert, damit nur noch bei Upar == 2 die      *
*                   //Q-Funktion eingeschaltet ist.                        *
*                                                                          *
*                   DB2OS, 27.6.93                                         *
*                   Vcmd(): Hatte keine Funktion mehr..                    *
*                           Jetzt Ausgabe der Firmware Version (ALL.H)     *
*                                                                          *
*                   DB2OS,  23.11.93                                       *
*                   M7par: 7Bit Ausgabe im Terminal Mode entfernt.         *
*                                                                          *
*                   DG2FEF, 18.07.94                                       *
*                   Bcmd(): Stillgelegt, gibt keinen Timeout mehr.         *
*                                                                          *
*                   DB2OS,  12.08.94                                       *
*                   A1par und A2par f¸r den SRTT entfernt.                 *
*                   MHEARD entfernt.                                       *
*                                                                          *
*                   DB2OS,  27.08.94                                       *
*                   Bcmd(): Jetzt Anzeige fÅr Anzahl der Hauptschleifen-   *
*                   durchgÑnge in Rounds per Second.                       *
*                   lxinit(): Variablen loops und rps initialisieren.      *                                                       *
*                   sl2par(): alias und defID (Alias-Call) entfernt.       *
*                                                                          *
*                   DB2OS,  02.09.94                                       *
*                   hpinch(): CTRL-Zeichen im Terminal-Mode werden als     *
*                             "." ausgeben. Lediglich BELL und TAB werden  *
*                             transparent im Echo ans Terminal geschickt.  *
*                                                                          *
*                   DL8HBS, 19.10.94                                       *
*                   lxinit(): UItxPID auf Default 0xf0 setzen              *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "version.h"     /* FEF enthÑlt nur Versionsstring                */
#include "tf.h"          /* Festlegungen/Datenstrukturen fuer TheFirmware */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "tfext.h"       /* globale Variable / nicht int-Funktionen       */
#include "kiss.h"

/* external function declarations */
extern void cpyid();
extern void cpyidl();
extern void inithd();
extern void hcrlf();
extern void hpbsb();
extern void hputbt();
extern void hputc();
extern void hputcc();
extern void hputid();
extern void hputs();
extern void hputud();
extern void hputv();
extern void reset();
extern void rspex();
extern void rspexb();
extern void rspic();
extern void rspini();
extern void rspipa();
extern void rspiv();
extern void rsppar();
extern void rspsuc();
extern void skpvia();
extern short upcase();


static unsigned short par;


/**************************************************************************\
*                                                                          *
* "set level 2 parameter"                                                  *
*                                                                          *
\**************************************************************************/

void sl2par()
  {
    if (!iswarm())
      {
        cpyid(myid,defIp);
        Ypar  = defYp;
        Tpar  = defTp;
        send_kisscmd(CMD_TXDELAY,defTp);
        Wpar  = defWp;
        send_kisscmd(CMD_SLOTTIME,defWp);
        Ppar  = defPp;
        send_kisscmd(CMD_PERSIST,defPp);
        Opar  = defOp;
        Npar  = defNp;
        T2par = defT2p;
        T3par = defT3p;
        Rpar  = defRp;
        Fpar  = defFp;
        A3par = defA3p;
        Ipar  = defIPp;
        UIpar = defUIp;
        xTApar = defxTAp;
        send_kisscmd(CMD_TXTAIL,defxTAp);
      }
  }





/**************************************************************************\
*                                                                          *
* "level x initialize"             FEF                                     *
*                                                                          *
\**************************************************************************/

void lxinit()
  {
    unsigned short n;

    inithd(&statml);
    inithd(&smonfl);
    for (n = 0; n < LINKNMBR; ++n)
      {
        inithd(&chnlml[n]);
        chnlbc[n] = 0;             /* FEF */
      }
    loops       =
    rps         =
    sec100      =
    incnt       =
    isctlr      = 0;
    mifmbp      = (MBHEAD *) NULL;
    ishmod      =
    hmstat      =
    actch       =
    ticks       =
    oldtic      = 0;
#ifdef USE_XPID
    UItxPID     = TXPIDDEFAULT;
 #endif
    if (!iswarm())
      {
        Apar        = defAp;
        Epar        = defEp;
        Mpar        = defMp;
        Xpar        = defXp;
        Zpar        = defZp;
        VCpar       = defVCp;
        Dpar        = defDp;
        send_kisscmd(CMD_FULLDUP,defDp);
        xFpar       = defxFp;
        eudate      = 0;
        tftime.second = 0;
        tftime.minute = 0;
        tftime.hour   = 0;
        tftime.day    = 0;
        tftime.month  = 0;
        tftime.year   = 0;
        mftsel      = 0;
        Upar        = 0;
        Utcnt       = 0;
        ch0via[0]   = '\0';
        mftidl[0]   = '\0';
        cpyid(ch0id,cqid);
        magicn      = MAGIC;
        stamp       = defxSp;
      }
  }




/**************************************************************************\
*                                                                          *
* "host put input character"                                               *
*                                                                          *
\**************************************************************************/
void hpinch(ch)
unsigned short ch;
{
  if (Epar == YES) {                 /* Echo eingeschaltet?               */
    if (!incnt && ch == defESC)      /* Kein Hostmode?                    */
      hputs("* ");                   /* Prompt fÅr ESC-Zeichen ausgeben.  */
    else
      if (    ch >= ' '
           || ch == BELL
           || ch == TAB  )           /* darstellbare Zeichen ausgeben     */
        hputc(ch);
      else
        hputc('.');                  /* Ersatz fÅr Control-Zeichen        */
  }
}



/**************************************************************************\
*                                                                          *
* "delete input character"                                                 *
*                                                                          *
\**************************************************************************/

void delich()
  {
    unsigned short ch;

    --incnt;
    ch = *--inbufp & 0xFF;
    if (!incnt && ch == defESC)
      {
        hpbsb();
        hpbsb();
      }
    else 
      if (Epar == YES) {              /* Echo eingeschaltet           */
        if (ch != BELL)               /* und kein Bell..              */
          hpbsb();                    /* Zeichen lîschen           */
        else
          hputc(ch);
        }
      
  }





/**************************************************************************\
*                                                                          *
* "host put backspace space backspace"                                     *
*                                                                          *
\**************************************************************************/

void hpbsb()
  {
    hputs("\010 \010");
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Acmd()
  {
    if (!incnt)
      rsppar(Apar);
    else
      if ((par = bgetp()) <= 1)
        {
          Apar = par;
          rspsuc();
        }
      else
        rspiv(par);
  }




/**************************************************************************\
*                                                                          *
* Bcmd:  RPS-Anzeige (Rounds per Seconds) frÅher DAMA-Timer.               *
*        HauptschleifendurchgÑnge pro Sekunde.                             *
*                                                                          *
\**************************************************************************/
void Bcmd()
  {
    rspini(HMRSMSG);
    hputud(rps);
    rspex();
  }



/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Ecmd()
  {
    if (!incnt)
      rsppar(Epar);
    else
      if ((par = bgetp()) <= 1)
        {
          Epar = par;
          rspsuc();
        }
      else
        rspiv(par);
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Hcmd()
  {
    rspic('H');
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Jcmd()
  {
    if (incnt >= 4)
      if (upcase(*inbufp++) == 'H')
        if (upcase(*inbufp++) == 'O')
          if (upcase(*inbufp++) == 'S')
            if (upcase(*inbufp++) == 'T')
              {
                incnt -= 4;
                if (!nxtnos())
                  rsppar(ishmod);
                else
                  if ((par = bgetp()) <= 1)
                    {
                      rspsuc();
                      ishmod = par;
                    }
                  else
                    rspiv(par);
                return;
              }
    rspic('J');
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/



void bgetti(p1,p2,p3)                                /* "buffer get time" */

char *p1;
char *p2;
char *p3;

  {
    *p1 = bgetp();
    --incnt;
    ++inbufp;
    *p2 = bgetp();
    --incnt;
    ++inbufp;
    *p3 = bgetp();
  }



void Kcmd()
  {
    if (!incnt)
      {
        rspini(HMRSMSG);
        hputud(stamp);
        hputc(' ');
        hputbt(&tftime);
        rspex();
      }
    else
      {
        par = bgetp();
        if (!incnt)
          {
            if (par <= 2)
              {
                stamp = par;
                rspsuc();
              }
            else
              rspiv(par);
          }
        else
          {
            par = *inbufp & 0xFF;
            incnt += 2;
            inbufp -= 2;
            switch (par)
              {
                case '.' :  eudate = YES;
                            bgetti(&tftime.day,&tftime.month,&tftime.year);
                            break;
                case '/' :  eudate = NO;
                            bgetti(&tftime.month,&tftime.day,&tftime.year);
                            break;
                case ':' :  bgetti(&tftime.hour,&tftime.minute,&tftime.second);
                            break;
              }
            rspsuc();
          }
      }
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Qcmd()
  {
    if (incnt >= 3)
      if (upcase(*inbufp++) == 'R')
        if (upcase(*inbufp++) == 'E')
          if (upcase(*inbufp++) == 'S')
            {
              magicn = 0;
              reset();
              return;
            }
    rspic('Q');
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Ucmd()
  {
    char       *txt;
    unsigned short    n;
    unsigned short    ch;

    if (!incnt)
      {
        rspini(HMRSMSG);
        hputud(Upar);
        if (Utcnt != 0)
          {
            hputc(' ');
            for (txt = Utxt, n = 0; n < Utcnt; ++n)
              hputcc(*txt++);
          }
        rspex();
      }
    else
      if ((ch = *inbufp & 0xFF) >= '0' && ch <= '9')
        if ((par = bgetp()) <= 2)
          {
            nxtnos();
            if (incnt <= UTXTLEN)
              {
                Upar = par;
                if (incnt != 0)
                  for (Utcnt = 0, txt = Utxt; incnt != 0; )
                    {
                      *txt++ = *inbufp++;
                      ++Utcnt;
                      --incnt;
                    }
                if (Upar && !Utcnt)
                  {
                    rspini(HMRFMSG);
                    hputs("NO MESSAGE AVAILABLE");
                    rspexb();
                    Upar = NO;
                  }
                else
                  rspsuc();
              }
            else
              {
                rspini(HMRFMSG);
                hputs("MESSAGE TOO LONG");
                rspexb();
              }
          }
        else
          rspiv(par);
      else
        rspipa();
  }





/**************************************************************************\
*                                                                          *
*  VERSION                                                                 *
*                                                                          *
\**************************************************************************/

void Vcmd()
  {
    rspini(HMRSMSG);
    hputs(VERSION);           /* Ausgabe der Versions-Nummer, siehe ALL.H */
    rspex();
  }



/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Xcmd()
  {
    if (!incnt)
      rsppar(Xpar);
    else
      if ((par = bgetp()) <= 1)
        {
          Xpar = par;
          rspsuc();
        }
      else
        rspiv(par);
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Zcmd()
  {
    if (!incnt)
      rsppar(Zpar);
    else
      if ((par = bgetp()) <= (FZFLOW | FZXONOFF))
        {
          Zpar = par;
          rspsuc();
        }
      else
        rspiv(par);
  }





/**************************************************************************\
*                                                                          *
* "list channel"                                                           *
*                                                                          *
\**************************************************************************/

void listch(chnl)

unsigned short chnl;

  {
    unsigned short    lstate;
    unsigned short    mbnmbr;
    unsigned short    unacked;
    LNKBLK     *linkp;

    if (chnl != 0)
      {
        linkp = &lnktbl[chnl - 1];
        lstate = linkp->state;
      }
    hputc(chnl == actch ? '+' : ' ');
    hputc('(');
    hputud(chnl);
    hputs(") ");
    if (!chnl)
      {
        hputid(ch0id);
        hputv(NO,ch0via);
      }
    else
      if (lstate != L2SDSCED)
        {
          hputid(linkp->dstid);
          hputv(NO,linkp->viaidl);
        }
    if (    (mbnmbr = mbcnt(!chnl ? &smonfl : &chnlml[chnl - 1],MBALL)) != 0
         || (chnl != 0 && lstate != L2SDSCED)
       )
      {
        hcrlf();
        hputs("      receive ");
        hputud(mbnmbr);
      }
    if (chnl != 0 && lstate != L2SDSCED)
      {
        unacked = (linkp->VS - linkp->lrxdNR) & 0x7;
        hputs("   send ");
        hputud(linkp->tosend - unacked);
        hputs("   unacked ");
        hputud(unacked);
        if (    unacked != 0
             || lstate < L2SIXFER
             || linkp->T1 != 0
           )
          {
            hputs("   retries ");
            hputud(linkp->tries);
          }
      }
    hcrlf();
  }





/**************************************************************************\
*                                                                          *
* "buffer get via list"                                                    *
*                                                                          *
\**************************************************************************/

short bgetvl(check,vial)

unsigned short    check;
char       *vial;

  {
    static char        viabuf[L2VLEN + 1];
    static char       *viabp;
    static unsigned short    n;
    static short       getres;

    skpvia();
    for (n = 0, viabp = viabuf; n < L2VNUM; ++n, viabp += L2IDLEN)
      if ((getres = bgetid(check,viabp)) == ERROR)
        return (ERROR);
      else
        if (!getres) break;
    *viabp = '\0';
    while (incnt != 0)
      if (*inbufp != ' ')
        return (ERROR);
      else
        {
          ++inbufp;
          --incnt;
        }
    cpyidl(vial,viabuf);
    return (*vial != '\0');
  }





/**************************************************************************\
*                                                                          *
* "skip via"                                                               *
*                                                                          *
\**************************************************************************/

void skpvia()
  {
    static char        id[L2IDLEN];
    static char       *bpsav;
    static char       *idp;
    static unsigned short    cntsav;
    static unsigned short    n;
    static unsigned short    chr;

    bpsav = inbufp;
    cntsav = incnt;
    if (bgetid(NO,id) == TRUE)
      {
        idp = id;
        if (*idp++ == 'V')
          {
            for (n = 0; n < L2CALEN - 1; ++n)
              if ((chr = *idp++ & 0xFF) != ' ')
                if (!((!n && chr == 'I') || (n == 1 && chr == 'A'))) break;
            if (n == L2CALEN - 1) return;
          }
      }
    inbufp = bpsav;
    incnt = cntsav;
  }





/**************************************************************************\
*                                                                          *
* "buffer get ID"                                                          *
*                                                                          *
\**************************************************************************/

short bgetid(check,gidp)

unsigned short    check;
char       *gidp;

  {
    static char        id[L2IDLEN];
    static char       *idp;
    static unsigned short    innmbr;
    static unsigned short    n;
    static unsigned short    inchr;

    for (idp = id, n = 0; n < L2CALEN; ++n) *idp++ = ' ';
    *idp = 0x60;
    nxtnos();
    idp = id;
    n = 0;

    while (incnt != 0)
      {
        if ((inchr = upcase(*inbufp & 0xFF)) == ' ' || inchr == ',') break;
        if (inchr < ' ') return (ERROR);
        if (inchr == '-')
          {
            if (n == 0 || incnt == 0) return (ERROR);
            ++inbufp;
            --incnt;
            if ((inchr = *inbufp & 0xFF) < '0' || inchr > '9') return (ERROR);
            ++inbufp;
            --incnt;
            innmbr = inchr - '0';
            if (incnt != 0)
              if ((inchr = *inbufp & 0xFF) >= '0' && inchr <= '9')
                {
                  innmbr *= 10;
                  innmbr += (inchr - '0');
                  if (innmbr > 15) return (ERROR);
                  ++inbufp;
                  --incnt;
                }
            id[L2IDLEN - 1] = (innmbr << 1) | 0x60;
            break;
          }
        else
          {
            if (n++ == L2CALEN) return (ERROR);
            *idp++ = inchr;
            ++inbufp;
            --incnt;
          }
      } /* end while */

    while (incnt != 0)
      {
        if (!((inchr = *inbufp & 0xFF) == ' ' || inchr == ',')) break;
        ++inbufp;
        --incnt;
        if (inchr == ',') break;
      }

    if (!n) return (FALSE);
    if (fvalca(check,id) == ERROR) return (ERROR);
    cpyid(gidp,id);
    return (TRUE);
  }





/**************************************************************************\
*                                                                          *
* "hpost put fixed unsigned"                                               *
*                                                                          *
\**************************************************************************/

void hputfu(u)

unsigned short u;

  {
    static BOOLEAN    out;
    static unsigned short   div;
    static unsigned short   digit;
    static unsigned short   n;

    for (out = FALSE, div = 10000, n = 0; n < 5; ++n)
      {
        if ((digit = u/div) != 0 || out == TRUE || div == 1)
          {
            hputc(digit + '0');
            out = TRUE;
          }
        else
          hputc(' ');
        u  %= div;
        div /= 10;
      }
  }


/* Ende von TFD.C */

