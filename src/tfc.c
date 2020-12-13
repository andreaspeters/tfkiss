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
*    TFC.C   -   The Firmware, Teil 3, Utilities                           *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DF7ZE / 01.01.90 / modul selmfl                        *
*                   - bei voller heardliste Ñltesten eintrag Åberschreiben *
*                   DL8ZAW, 27.04.91                                       *
*                   frhmon(): [DAMA] bei DAMA-Master anzeigen              *
*                                                                          *
*                   DL8ZAW, 18.05.91                                       *
*                   hputmb(): M7par=0: 7Bit-Maske verwenden                *
*                                                                          *
*                   DB2OS,  26.07.92                                       *
*                   hputmb(): M7par=0: Kontrollzeichen als '^C' darstellen *
*                             M7par=1: transparente 8-Bit Ausgabe.         *
*                                                                          *
*                   DB2OS,  18.10.92                                       *
*                   hputmb(): BS|DEL Zeichenkorrektur ist dort bei AUSGABE *
*                             ÅberflÅssig und stîrt BOXBIN im Terminal     *
*                             8-Bit Modus.                                 *
*                                                                          *
*                   DB2OS,  17.7.92                                        *
*                   invial(): Bei Monitor-Betrieb mit CALL-Filter die SSID *
*                             nicht mehr berÅcksichtigen.                  *
*                                                                          *
*                   DB2OS,  23.11.93                                       *
*                   M7par: 7Bit Ausgabe im Terminal Mode entfernt.         *
*                                                                          *
*                   DB2OS/DG2FEF, 13.08.94                                 *
*                   frhmon(): l3/4 Klartextausgabe entfernt                *
*                                                                          *
*                   DL4YBG, 06.11.94                                       *
*                   sttoch(): PID changed Ausgabe hinzugefuegt             *
*                                                                          *
*                   DL4YBG, 13.02.95 (DG0FT, 16.01.95)                     *
*                   frhmon(): bei SABM und UA ggf. FlexNet-QSO-Nummer      *
*                             ausgeben                                     *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "tf.h"          /* Festlegungen/Datenstrukturen fuer TheFirmware */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "tfext.h"       /* globale Variable / nicht int-Funktionen       */

/* external function declarations */
extern void dealmb();
extern void hputc();
extern void inilbl();
extern void putchr();
extern void rwndmb();

/* local function declarations */
void hmputr();
void hputbt();
void hputby();
void hputcc();
void hputid();
void hputs();
void hputmb();
void hputni();
void hputud();
void hputv();
void hputvl();
void putbyt();
void putid();
void putnib();
void putstr();
void putudc();
void putv();
void rspini();
void rspex();
void rspexb();
void stampb();

/**************************************************************************\
*                                                                          *
* "initialize links"                                                       *
*                                                                          *
\**************************************************************************/

void inilks()
  {
    unsigned short    n;
    LNKBLK     *savelp;

    savelp = lnkpoi;
    for (n = 0, lnkpoi = lnktbl; n < LINKNMBR; ++n, ++lnkpoi)
      if (!lnkpoi->state) inilbl();
    lnkpoi = savelp;
  }





/**************************************************************************\
*                                                                          *
* "response invalid command"                                               *
*                                                                          *
\**************************************************************************/

void rspic(c)

unsigned short c;

  {
    rspini(HMRFMSG);
    hputs("INVALID COMMAND: ");
    hputcc(c);
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response invalid extended command"                                      *
*                                                                          *
\**************************************************************************/

void rspiec(c)

unsigned short c;

  {
    rspini(HMRFMSG);
    hputs("INVALID EXTENDED COMMAND: ");
    hputcc(c);
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response invalid value"                                                 *
*                                                                          *
\**************************************************************************/

void rspiv(value)

unsigned short value;

  {
    rspini(HMRFMSG);
    hputs("INVALID VALUE: ");
    hputud(value);
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response invalid callsign"                                              *
*                                                                          *
\**************************************************************************/

void rspics()
  {
    rspini(HMRFMSG);
    hputs("INVALID CALLSIGN");
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response no source callsign"                                            *
*                                                                          *
\**************************************************************************/

void rspnsc()
  {
    rspini(HMRFMSG);
    hputs("NO SOURCE CALLSIGN");
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response already connected"                                             *
*                                                                          *
\**************************************************************************/

void rspalc()
  {
    hputs(" ALREADY CONNECTED");
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response not while connected"                                           *
*                                                                          *
\**************************************************************************/

void rspnwc()
  {
    rspini(HMRFMSG);
    hputs("NOT WHILE CONNECTED");
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response line ignored"                                                  *
*                                                                          *
\**************************************************************************/

void rsplig()
  {
    rspini(HMRFMSG);
    hputs("TNC BUSY - LINE IGNORED");
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "response invalid parameter"                                             *
*                                                                          *
\**************************************************************************/

void rspipa()
  {
    rspini(HMRFMSG);
    hputs("INVALID PARAMETER");
    rspexb();
  }





/**************************************************************************\
*                                                                          *
* "frame header monitor"                                                   *
*                                                                          *
\**************************************************************************/

void frhmon(fbp)

MBHEAD *fbp;

  {
    static unsigned short    pid;
#ifdef USE_FLEXDEC
    unsigned short           qso;
#endif



#ifdef USE_FLEXDEC
    qso = 0xFFFF;
#endif
    hputs("fm ");
    hputid(rxfhdr + L2IDLEN);
    hputs(" to ");
    hputid(rxfhdr);
    hputv(YES,rxfhdr + L2ILEN);
    hputs(" ctl ");

    if (!(rxfctl & L2CNOIM))
      hputc('I');
    else
      if (!(rxfctl & L2CNOSM))
        switch ((rxfctl >> 2) & 0x3)
          {
            case 0  :   hputs("RR");    break;
            case 1  :   hputs("RNR");   break;
            case 2  :   hputs("REJ");   break;
            default :
              hputc('?');
              hputby(rxfctl | rxfPF);
              hputc('H');
            break;
          }
      else
        switch (rxfctl & 0xFF)
          {
            case L2CUI   :   hputs("UI");     break;
            case L2CDM   :   hputs("DM");     break;
#ifndef USE_FLEXDEC
            case L2CSABM :   hputs("SABM");   break;
#endif
            case L2CDISC :   hputs("DISC");   break;
#ifndef USE_FLEXDEC
            case L2CUA   :   hputs("UA");     break;
#else
            case L2CSABM :
            case L2CUA   :
              hputs((rxfctl & 0xFF) == L2CSABM ? "SABM" : "UA");
              if (fbp->mbpc - fbp->mbgc == 2)
                qso = (getchr(fbp) << 8 | getchr(fbp)) & 0x3FFF;
            break;
#endif
            case L2CFRMR :
              hputs("FRMR");
              while (fbp->mbgc < fbp->mbpc) hputby(getchr(fbp));
            break;
            default      :
              hputc('?');
              hputby(rxfctl | rxfPF);
              hputc('H');
            break;
          }

    if ((rxfctl & 0x3) != 3)
      {
        hputud((rxfctl >> 5) & 0x7);
        if (!(rxfctl & L2CNOIM)) hputud((rxfctl >> 1) & 0x7);
      }

    if (rxfPF != 0)
      if (!rxfV2)
        hputc('!');
      else
        hputc(rxfCR != 0 ? '+' : '-');
    else
      if (rxfV2 == YES)
        hputc(rxfCR != 0 ? '^' : 'v');

    if (!(rxfctl & L2CNOIM) || rxfctl == L2CUI)
      {
        hputs(" pid ");
        hputby(pid = fbp->mbgc < fbp->mbpc ? getchr(fbp) : 0);
      }

#ifdef USE_FLEXDEC
    if (qso != 0xFFFF)
      {
        hputs(" #");
        hputud(qso);
      }
#endif

    if ((rxfhdr[L2ILEN-1] & L2CDAMA) == 0)
       hputs(" [DAMA]");

    if (stamp == 2)
      {
        hputs(" - ");
        hputbt(&fbp->btime);
      }
  }





/**************************************************************************\
*                                                                          *
* "terminal mode response message buffer bell"                             *
*                                                                          *
\**************************************************************************/

void trpmbb(mbp)

MBHEAD *mbp;

  {
    rwndmb(mbp);
    hputs("\015\012* ");
    hputmb(mbp);
    if (stamp)
      {
        hputs(" - ");
        hputbt(&mbp->btime);
      }
    hputs(" *\007\015\012");
  }





/**************************************************************************\
*                                                                          *
* "host put message buffer"                                                *
*                                                                          *
\**************************************************************************/
void hputmb(mbp)
MBHEAD *mbp;
{
  static unsigned short    ch;

  while (mbp->mbgc < mbp->mbpc)
  {
    ch = getchr(mbp) & 0xFF;         /* 8-Bit ASCII                 */
    hputc(ch);                       /* transparent binary ausgeben */
    if (ch == CR && Apar == YES )    /* Carriage Return + Linefeed  */
      hputc(LF);
  }
}





/**************************************************************************\
*                                                                          *
* "response channel status"                                                *
*                                                                          *
\**************************************************************************/

void rspcs()
  {
    rspini(HMRSMSG);
    if (!actch)
      {
        hputid(ch0id);
        hputv(NO,ch0via);
      }
    else
      if (lnkpoi->state != L2SDSCED)
        {
          hputid(lnkpoi->dstid);
          hputv(NO,lnkpoi->viaidl);
        }
      else
        hputs("CHANNEL NOT CONNECTED");
    rspex();
  }





/**************************************************************************\
*                                                                          *
* "host put via"                                                           *
*                                                                          *
\**************************************************************************/

void hputv(dmark,vial)

unsigned short    dmark;
char       *vial;

  {
    if (*vial != '\0')
      {
        hputs(" via");
        hputvl(dmark,vial);
      }
  }





/**************************************************************************\
*                                                                          *
* "host put via list"                                                      *
*                                                                          *
\**************************************************************************/

void hputvl(dmark,vial)

unsigned short    dmark;
char       *vial;

  {
    while (*vial != '\0')
      {
        hputc(' ');
        hputid(vial);
        if (dmark == YES)
          if ((vial[L2IDLEN - 1] & L2CH) != 0)
            if (!vial[L2IDLEN] || !(vial[L2ILEN - 1] & L2CH))
              {
                hputc('*');
                dmark = NO;
              }
        vial += L2IDLEN;
      }
  }





/**************************************************************************\
*                                                                          *
* "host put ID"                                                            *
*                                                                          *
\**************************************************************************/

void hputid(id)

char *id;

  {
    static unsigned short ssid;
    static unsigned short n;
    static unsigned short ch;

    for (n = 0; n < L2CALEN; ++n)
      if ((ch = *id++ & 0xFF) > ' ')
        hputc(ch);
      else
        if (ch < ' ')
          {
            hputc('^');
            hputc(ch + '@');
          }
   if ((ssid = (*id >> 1) & 0xF) != 0)
     {
       hputc('-');
       hputud(ssid);
     }
  }





/**************************************************************************\
*                                                                          *
* "response parameter"                                                     *
*                                                                          *
\**************************************************************************/

void rsppar(par)

unsigned short par;

  {
    rspini(HMRSMSG);
    hputud(par);
    rspex();
  }





/**************************************************************************\
*                                                                          *
* "host put unsigned decimal"                                              *
*                                                                          *
\**************************************************************************/

void hputud(u)

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
        u  %= div;
        div /= 10;
      }
  }





/**************************************************************************\
*                                                                          *
* "host put buffer time"                                                   *
*                                                                          *
\**************************************************************************/



void hput2d(c)

char c;

  {
    hputud((c & 0xff) / 10);
    hputud((c & 0xff) % 10);
  }



void hputbt(tbp)

TIMEBL *tbp;

  {
    if (eudate)
      {
        hput2d(tbp->day);
        hputc('.');
        hput2d(tbp->month);
        hputc('.');
      }
    else
      {
        hput2d(tbp->month);
        hputc('/');
        hput2d(tbp->day);
        hputc('/');
      }
    hput2d(tbp->year%100);
    hputc(' ');
    hput2d(tbp->hour);
    hputc(':');
    hput2d(tbp->minute);
    hputc(':');
    hput2d(tbp->second);
  }





/**************************************************************************\
*                                                                          *
* "host put byte"                                                          *
*                                                                          *
\**************************************************************************/

void hputby(byte)

unsigned short byte;

  {
    hputni(byte >> 4);
    hputni(byte);
  }





/**************************************************************************\
*                                                                          *
* "host put nibble"                                                        *
*                                                                          *
\**************************************************************************/

void hputni(nibble)

unsigned short nibble;

  {
    nibble &= 0xF;
    if (nibble > 9) nibble += 7;
    hputc(nibble + '0');
  }





/**************************************************************************\
*                                                                          *
* "response init"                                                          *
*                                                                          *
\**************************************************************************/

void rspini(r)

unsigned short r;

  {
    if (!ishmod)
      hputs("* ");
    else
      hmputr(r);
  }





/**************************************************************************\
*                                                                          *
* "response success"                                                       *
*                                                                          *
\**************************************************************************/

void rspsuc()
  {
    if (ishmod == YES) hmputr(0);
  }





/**************************************************************************\
*                                                                          *
* "host mode put response"                                                 *
*                                                                          *
\**************************************************************************/

void hmputr(r)

unsigned short r;

  {
    hputc(actch);
    hputc(r);
  }





/**************************************************************************\
*                                                                          *
* "response exit"                                                          *
*                                                                          *
\**************************************************************************/

void rspex()
  {
    if (!ishmod)
      hputs(" *\015\012");
    else
      hputc(0);
  }





/**************************************************************************\
*                                                                          *
* "response exit bell"                                                     *
*                                                                          *
\**************************************************************************/

void rspexb()
  {
    if (!ishmod)
      hputs(" *\007\015\012");
    else
      hputc(0);
  }





/**************************************************************************\
*                                                                          *
* "host carriage return linefeed"                                          *
*                                                                          *
\**************************************************************************/

void hcrlf()
  {
    hputs("\015\012");
  }





/**************************************************************************\
*                                                                          *
* "host put string"                                                        *
*                                                                          *
\**************************************************************************/

void hputs(str)

char *str;

  {
    while (*str != '\0') hputc(*str++);
  }





/**************************************************************************\
*                                                                          *
* "host put control character"                                             *
*                                                                          *
\**************************************************************************/

void hputcc(c)

unsigned short c;

  {
    if (c >= ' ')
      hputc(c);
    else
      {
        hputc('^');
        hputc(c + '@');
      }
  }





/**************************************************************************\
*                                                                          *
* "status to channel"                                                      *
*                                                                          *
\**************************************************************************/

void sttoch(msg)

unsigned short msg;

  {
    static char     *frmrp;
    static MBHEAD   *mbp;

    putchr('(',mbp = (MBHEAD *)allocb());
    putudc((lnkpoi - lnktbl) + 1,mbp);
    putstr(") ",mbp);
#ifdef USE_XPID
    if (msg == L2MPIDCH) {
      putstr("PID changed to (",mbp);
      putbyt(lnkpoi->rxPID,mbp);
      putchr(')',mbp);
    }
    else {
#endif
    putstr(l2msgs[msg - 1],mbp);
    putchr(' ',mbp);
    putid(lnkpoi->dstid,mbp);
    putv(NO,lnkpoi->viaidl,mbp);
    if (msg == L2MFRMRF || msg == L2MFRMRT)
      {
        putstr(" (",mbp);
        frmrp = lnkpoi->frmr;
        putbyt(*frmrp++,mbp);
        putbyt(*frmrp++,mbp);
        putbyt(*frmrp++,mbp);
        putchr(')',mbp);
      }
#ifdef USE_XPID
    }
#endif
    stampb(&mbp->btime);
    mbp->type = msg;
    if (!ishmod && !Upar)
      relink(mbp,statml.tail);
    else
      {
        relink(mbp,chnlml[lnkpoi-lnktbl].tail);
        chnlbc[lnkpoi-lnktbl] += mbp->mbpc;
      }
  }





/**************************************************************************\
*                                                                          *
* "put via ID's"                                                           *
*                                                                          *
\**************************************************************************/

void putv(dmark,vial,mbp)

unsigned short    dmark;
char       *vial;
MBHEAD     *mbp;

  {
    if (*vial != '\0')
      {
        putstr(" via",mbp);
        while (*vial != '\0')
          {
            putchr(' ',mbp);
            putid(vial,mbp);
            if (dmark == YES)
              if ((vial[L2IDLEN - 1] & L2CH) != 0)
                if ((!vial[L2IDLEN] || !(vial[L2ILEN - 1] & L2CH)))
                  {
                    putchr('*',mbp);
                    dmark = NO;
                  }
            vial += L2IDLEN;
          }
      }
  }





/**************************************************************************\
*                                                                          *
* "put ID"                                                                 *
*                                                                          *
\**************************************************************************/

void putid(id,mbp)

char     *id;
MBHEAD   *mbp;

  {
    static unsigned short ssid;
    static unsigned short n;
    static unsigned short c;

    for (n = 0; n < L2CALEN; ++n)
      if ((c = *id++ & 0xFF) > ' ')
        putchr(c,mbp);
      else
        if (c < ' ')
          {
            putchr('^',mbp);
            putchr(c + '@',mbp);
          }
    if ((ssid = (*id >> 1) & 0xF) != 0)
      {
        putchr('-',mbp);
        putudc(ssid,mbp);
      }
  }





/**************************************************************************\
*                                                                          *
* "put unsigned decimal"                                                   *
*                                                                          *
\**************************************************************************/

void putudc(u,mbp)

unsigned short    u;
MBHEAD     *mbp;

  {
    static BOOLEAN    out;
    static unsigned short   div;
    static unsigned short   digit;
    static unsigned short   n;

    for (out = FALSE, div = 10000, n = 0; n < 5; ++n)
      {
        if ((digit = u/div) != 0 || out == TRUE || div == 1)
          {
            putchr(digit + '0',mbp);
            out = TRUE;
          }
        u %= div;
        div /= 10;
      }
  }





/**************************************************************************\
*                                                                          *
* "put byte"                                                               *
*                                                                          *
\**************************************************************************/

void putbyt(byte,mbp)

unsigned short    byte;
MBHEAD     *mbp;

  {
    putnib(byte >> 4,mbp);
    putnib(byte,mbp);
  }





/**************************************************************************\
*                                                                          *
* "put nibble"                                                             *
*                                                                          *
\**************************************************************************/

void putnib(nibble,mbp)

unsigned short    nibble;
MBHEAD     *mbp;

  {
    nibble &= 0xF;
    if (nibble > 9) nibble += 7;
    putchr(nibble + '0',mbp);
  }





/**************************************************************************\
*                                                                          *
* "put string"                                                             *
*                                                                          *
\**************************************************************************/

void putstr(str,mbp)

char     *str;
MBHEAD   *mbp;

  {
    while (*str != '\0') putchr(*str++,mbp);
  }





/**************************************************************************\
*                                                                          *
* "buffer get parameter"                                                   *
*                                                                          *
\**************************************************************************/

unsigned short bgetp()
  {
    static unsigned short par;

    nxtnos();
    par = 0;
    while (incnt != 0 && *inbufp >= '0' && *inbufp <= '9')
      {
        --incnt;
        par *= 10;
        par += *inbufp++ - '0';
      }
    return (par);
  }





/**************************************************************************\
*                                                                          *
* "flagged validate callsign"                                              *
*                                                                          *
\**************************************************************************/

short fvalca(check,call)

unsigned short    check;
char       *call;

  {
    char       *lnpoi;
    char       *cpoi;
    unsigned short    nmbn;
    unsigned short    n;
    unsigned short    c;

    if (*call == ' ') return(FALSE);
    if (!check) return(TRUE);
    for (nmbn = 0, n = 0, cpoi = call; n < L2CALEN; ++n, ++cpoi)
      {
        if ((c = *cpoi & 0xFF) == ' ') break;
        if (!(c >= 'A' && c <= 'Z')) {
          if (c >= '0' && c <= '9')
            {
              ++nmbn;
              lnpoi = cpoi;
            }
          else
            return (ERROR);
        }
      }
    if (    cpoi - call < 4
         || !nmbn
         || nmbn > 2
         || lnpoi == call
         || cpoi - 1 == lnpoi
       )
      return (ERROR);
    else
      return (TRUE);
  }





/**************************************************************************\
*                                                                          *
* "stamp buffer"                                                           *
*                                                                          *
\**************************************************************************/

void stampb(dtp)

char *dtp;

  {
    static unsigned short   n;
    static char      *stp;

    for (stp = (char *)&tftime, n = 0; n < 6; ++n) *dtp++ = *stp++;
  }





/**************************************************************************\
*                                                                          *
* "select monitor frame list"                                              *
*                                                                          *
\**************************************************************************/

void selmfl()
  {
    static MBHEAD     *fbp;


    while ((fbp = (MBHEAD *)monfl.head) != (MBHEAD *)&monfl)
      {
        tfunlink(fbp);
        stampb(&fbp->btime);
        if (ismonf(fbp) == YES)
          relink(fbp,smonfl.tail);
        else
          dealmb(fbp);

      } /* end  while ((fbp = monfl.head) != &monfl) */
  }



/**************************************************************************\
*                                                                          *
* "insert into heard - list"                                               *
*  return new number of entries                                            *
*                                                                          *
\**************************************************************************/


/**************************************************************************\
*                                                                          *
* "is monitor frame"                                                       *
*                                                                          *
\**************************************************************************/

BOOLEAN ismonf(fbp)

MBHEAD *fbp;

  {
    if (!actch || !lnkpoi->state || (Mpar & MONC) != NO)
      {
        takfhd(fbp);
        if (    (!(rxfctl & L2CNOIM) && (Mpar & MONI) != 0)
             || ((rxfctl & 3) == 1 && (Mpar & MONS) != 0)
             || (((rxfctl & 3) == 3 && rxfctl != L2CUI) && (Mpar & MONS) != 0)
             || (rxfctl == L2CUI && (Mpar & MONU) != 0)
           )
          {
            if (mftsel != 0) {
              if (    invial(mftidl,rxfhdr + L2IDLEN) == YES
                   || invial(mftidl,rxfhdr) == YES
                 )
                {
                  if (mftsel == 2) return (NO);
                }
              else
                {
                 if (mftsel == 1) return (NO);
                }
            }
            return (YES);
          }
      }
    return (NO);
  }





/**************************************************************************\
*                                                                          *
* "message buffer count"                                                   *
*                                                                          *
\**************************************************************************/

unsigned short mbcnt(msgl,select)

LHEAD      *msgl;
unsigned short    select;

  {
    static unsigned short    count;
    static MBHEAD     *mb;

    for (count = 0, mb = (MBHEAD *)msgl->head; mb != (MBHEAD *)msgl; 
         mb = mb->nextmh)
      if (    select == MBALL
           || (select == MBINFO    &&  mb->type == MBINFO)
           || (select == MBSTATUS  &&  mb->type != MBINFO)
         ) ++count;
     return (count);
   }





/**************************************************************************\
*                                                                          *
* "select message buffer"                                                  *
*                                                                          *
\**************************************************************************/

MBHEAD *selmb(msgl,select)

LHEAD      *msgl;
unsigned short    select;

  {
    static MBHEAD *mb;

    for (mb = (MBHEAD *)msgl->head; mb != (MBHEAD *)msgl; mb = mb->nextmh)
      if (    (select == MBALL)
           || (select == MBINFO    &&  mb->type == MBINFO)
           || (select == MBSTATUS  &&  mb->type != MBINFO)
         )
        return(mb);
    return (NULL);
  }





/**************************************************************************\
*                                                                          *
* "in via list"     FÅr CALL-Filter im Monitor-Betrieb.                    *
*                                                                          *
\**************************************************************************/

BOOLEAN invial(vial,id)

char *vial;
char *id;

  {
    while (*vial != '\0')
      if (cmpcal(vial,id) == TRUE)  /* Nur CALL, nicht SSID vergleichen */
        return (TRUE);
      else
        vial += L2IDLEN;
    return (FALSE);
  }





/**************************************************************************\
*                                                                          *
* "next no space"                                                          *
*                                                                          *
\**************************************************************************/

BOOLEAN nxtnos()
  {
    while (incnt != 0 && *inbufp == ' ')
      {
        ++inbufp;
        --incnt;
      }
    return (incnt != 0);
  }





/**************************************************************************\
*                                                                          *
* "neighbor port"                                                          *
*                                                                          *
\**************************************************************************/

unsigned short nbrprt(id)

char *id;

  {
    return(HDLCPORT);
  }





/**************************************************************************\
*                                                                          *
* "is warm reset"                                                          *
*                                                                          *
\**************************************************************************/

BOOLEAN iswarm()
  {
    return (magicn == MAGIC);
  }





/**************************************************************************\
*                                                                          *
* "timer"                                                                  *
*                                                                          *
\**************************************************************************/

void timer()
  {
    ++ticks;
  }



/* Ende von TFC.C */
