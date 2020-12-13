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
*    TFA.C   -   The Firmware, Teil 1                                      *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 07.05.91                                       *
*                   Signon V 2.2 DAMA                                      *
*                   lx(): DAMA-Timer alle Sekunde runterzaehlen            *
*                         (gehoert eigentlich in L2!)                      *
*                                                                          *
*                   DL8ZAW, 05.06.91                                       *
*                   Signon V 2.2a DAMA/UI+                                 *
*                                                                          *
*                   DL8ZAW, 19.06.91                                       *
*                   Signon V 2.2b DAMA/UI+                                 *
*                   Aenderung gegenueber TF22a:                            *
*                   L2A.C, l2timr(): DISC senden bei Link-Failure          *
*                   L2B.C, clrlnk(): DAMA-Timer ruecksetzen, wenn alle     *
*                                    Links aufgeloest                      *
*                   TFB.C, Dcmd():   DISC senden bei ESC-D waehrend        *
*                                    LINK-SETUP                            *
*                                                                          *
*                   DK6PX, 20.07.91                                        *
*                   Signon V 2.2c DAMA/UI+                                 *
*                   Aenderung gegenueber TF22b:                            *
*                   L2B.C, sdi(): dynamisches MAXFRAME, je nach            *
*                                 Info-Framelaenge                         *
*                   TFL1.MAC:     DWAIT-Funktion nach DL4YBG               *
*                   TFKISS.MAC:   DWAIT-Funktion nach DL4YBG               *
*                                                                          *
*                   DL8ZAW, 25.07.91                                       *
*                   Signon V 2.2d DAMA/UI+                                 *
*                   Aenderung gegenueber TF22c:                            *
*                   L2A.C: Beim Einschalten von DAMA P auf 255 und W auf 0,*
*                          Beim Auschalten wieder auf alte Werte setzen.   *
*                          Fehler beim DAMA-Einschalten behoben            *
*                                                                          *
*                   DL8ZAW, DB2OS, 02.05.92                                *
*                   Signon V 2.4 DAMA/SMACK                                *
*                   Aenderungen gegenueber TF 2.3:                         *
*                   Nicht verwendeten Layer 3 entfernt.                    *
*                   TFKISS.MAC: SMACK-Code eigebaut von Jan, DL5UE         *
*                   L2A.C:      Frames nur dann in Monitor, wenn mehr als  *
*                               256 Buffer frei sind                       *
*                                                                          *
*                   DG0FT, DB2OS, 07.02.93                                 *
*                   In fmlink() ist jetzt eine //Q-Funktion verfuegbar,    *
*                   wenn sich der TNC im Terminal-Mode befindet und        *
*                   der Unattended-Mode (U1) eingeschaltet ist.            *
*                                                                          *
*                   DG0FT, DB2OS, 21.6.93                                  *
*                   im l2tol7() und fmlink() Abfrage auf Upar ge„ndert,    *
*                   sodaá die //Q-Funktion nur noch aktiv ist, wenn        *
*                   Upar == 2 ist.                                         *
*                                                                          *
*                   DB2OS, 01.07.93                                        *
*                   im lx() den erweiterten Hostmode nach DG3DBI eingebaut.*
*                   Siehe auch xGcmd() in TFB.C                            *
*                                                                          *
*                   DB2OS,  27.08.94                                       *
*                   mainf() und lx(): Z„hler loops und rps fr RPS-Anzeige *
*                                     eingebaut.                           *
*                   DL8HBS, 19.10.94                                       *
*                   in lx() und l2tol7() wird die txPID nun aus            *
*                   lnkpoi->txPID genommen bzw. im Monitor aus UItxPID     *
*                                                                          *
*                   DL4YBG, 06.11.94                                       *
*                   in l2tol7() Auswertung von L2MPIDCH (PID changed)      *
*                                                                          *
*                   DB2OS,  23.05.95                                       *
*                   l2timr() von lx() nach mainf() verlegt..               *
*                                                                          *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "tf.h"          /* Festlegungen/Datenstrukturen fuer TheFirmware */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "tfext.h"       /* globale Variable / nicht int-Funktionen       */
#include "version.h"     /* Versions und Copyright                        */

/* external function declarations */
extern void DIinc();
extern BOOLEAN STAled();
extern BOOLEAN CONled();
extern void dealmb();
extern void decEI();
extern void delich();
extern void frhmon();
extern void hcrlf();
extern unsigned hgetc();
extern void hmputr();
extern void hpinch();
extern void hputc();
extern void hputmb();
extern void hputs();
extern BOOLEAN ishget();
extern BOOLEAN ishput();
extern void l2init();
extern void l2rest();
extern void l2rx();
extern void l2timr();
extern void l2tx();
extern void lxinit();
extern void putchr();
extern void putid();
extern void putstr();
extern void putv();
extern void rspic();
extern void rsplig();
extern void rspnsc();
extern void rspsuc();
extern void rwndmb();
extern void sdui();
extern void selmfl();
extern void sl2par();
extern void srand();
extern void stampb();
extern void sttoch();
extern void trpmbb();
extern short upcase();
extern void xGcmd();
extern void xonctl();

/* local function declarations */
void cmd();
void lx();
void tolink();




/**************************************************************************\
*                                                                          *
* "main function"                                                          *
*                                                                          *
\**************************************************************************/
short mainf()
  {
    unsigned    cs;
    char       *csp;

    DIinc();
    for (cs = 0, csp = (char *)0; csp < (char *)&p1end; cs += *csp++ & 0xFF);
    for (csp = (char *)&p2strt; csp < (char *)&end; cs += *csp++ & 0xFF) ;
    sl2par();
    l2init();
    lxinit();
    srand();
    decEI();

    /*-------------- Version, Copyright und Checksumme ausgeben ------------*/
/*
    hputs(COPY1);
    hputud(LINKNMBR);
    hputs(COPY2);
    hputs(COPY3);
    hputs(COPY4);
    hputs("Checksum (0000) = ");
    hputby(cs >> 8);
    hputby(cs);
    hputs("\015\012\012");
*/
    LOOP
    {
      l2rx();

      DIinc();
      newtic = ticks - oldtic;
      oldtic = ticks;
      decEI();
      l2timr(newtic);

      l2tx();
      l2rest();
      lx();
      loops++;                      /* Schleifenz„hler fr RPS-Berechnung */
    }
  }



/**************************************************************************\
*                                                                          *
* "level x"                                                                *
*                                                                          *
\**************************************************************************/

void lx()
  {
    static unsigned    inchar;
    static unsigned    n;
    static MBHEAD     *mbp;

    DIinc();
    xonctl();
    decEI();
    if (newtic != 0)
      {
        if ((sec100 += newtic) >= 100)          /* 1 Sekunde abgelaufen?   */
          {
            sec100 -= 100;
            if (damati > 0)                     /* DAMA-Timeout            */
              --damati;
            rps = loops;                        /* Umdrehungen pro Sekunde */
            loops = 0;

            if (++tftime.second == 60)            /* TNC-Uhr:                */
              {
                tftime.second = 0;
                if (++tftime.minute == 60)
                  {
                    tftime.minute = 0;
                    if (++tftime.hour == 24)
                      {
                        tftime.hour = 0;
                        switch(tftime.month)
                          {
                            case  2 : if (tftime.year % 4)
                                        n = 29;
                                      else
                                        n = 30;
                                      break;

                            case  4 :
                            case  6 :
                            case  9 :
                            case 11 :   n = 31;   break;

                            case  1 :
                            case  3 :
                            case  5 :
                            case  7 :
                            case  8 :
                            case 10 :
                            case 12 :   n = 32;   break;
                          }
                        if (++tftime.day == n)
                          {
                            tftime.day = 1;
                            if (++tftime.month == 13)
                              {
                                tftime.month = 1;
                                if (++tftime.year == 100) tftime.year = 0;
                              }
                          }
                      }
                  }
              }
          }
      }



    CONled(        !ishmod
                && actch != 0
                && lnktbl[actch - 1].state == L2SLKSUP
            ? 0
            : nmblks != 0
          );



    for (n = 0; n < LINKNMBR; ++n)
      if (chnlml[n].head != &chnlml[n]) break;
    STAled(n != LINKNMBR);



    if (!ishmod)
      {
        if (actch != 0) lnkpoi = &lnktbl[actch - 1];
        selmfl();

        if (incnt == 0 || !(Zpar & FZFLOW))
          {

            while (!ishput() && 
                   (mbp = (MBHEAD *)statml.head) != (MBHEAD *)&statml)
               {
                tfunlink(mbp);
                trpmbb(mbp);
                dealmb(mbp);
              }

            while (!ishput() && 
                   (mbp = (MBHEAD *)smonfl.head) != (MBHEAD *)&smonfl)
              {
                tfunlink(mbp);
                if (ismonf(mbp) == YES)
                  {
                    hcrlf();
                    frhmon(mbp);
                    hcrlf();
                    if (mbp->mbgc < mbp->mbpc)
                      {
                        hputmb(mbp);
                        hcrlf();
                      }
                  }
                dealmb(mbp);
              }

            if (actch != 0)
              while (    !ishput()
                      &&    (mbp = (MBHEAD *)chnlml[actch-1].head)
                         != (MBHEAD *)&chnlml[actch - 1]
                    )
                {
                  tfunlink(mbp);
                  if (!mbp->type)
                    hputmb(mbp);
                  else
                    trpmbb(mbp);
                  chnlbc[actch - 1] -= mbp->mbpc; /* FEF Flow-Control */
                  dealmb(mbp);
                }

          } /* end if (incnt == 0 || !(Zpar & FZFLOW)) */

        if (ishget() == YES && !ishput())
          {
            inchar = hgetc() & 0xFF;
            if (!isctlr || inchar == CONTROLR)
              {
                if (!incnt) inbufp = hinbuf;
                switch (inchar)
                  {



                    case CR :
                    case LF :
                      if (Epar == YES)
                        {
                          hputc(CR);
                          if (Apar == YES) hputc(LF);
                        }
                      inbufp = hinbuf;
                      if (incnt != 0 && *inbufp == defESC)
                        {
                          ++inbufp;
                          --incnt;
                          cmd();
                        }
                      else
                        if (nmbfre > HBREMSE) /* FEF */
                          {
#ifndef USE_XPID
                            (mbp = (MBHEAD *)allocb())->l2fflg = L2CPID;
#else
                            if (!actch || !lnkpoi->state)
                              (mbp = (MBHEAD *)allocb())->l2fflg = UItxPID;
                            else
                              (mbp = (MBHEAD *)allocb())->l2fflg = lnkpoi->txPID;
#endif
                            while (incnt--) putchr(*inbufp++,mbp);
                            putchr(CR,mbp);
                            rwndmb(mbp);
                            if (!actch)
                              {
                                if (myid[0] != ' ')
                                  sdui(ch0via,ch0id,myid,HDLCPORT,mbp);
                                else
                                  rspnsc();
                                dealmb(mbp);
                              }
                            else
                              if (lnkpoi->state != L2SDSCED)
                                tolink(mbp);
                          }
                        else
                          rsplig();
                      incnt = 0;
                    break;



                    case BS  :
                    case DEL :
                      if (incnt != 0) delich();
                    break;



                    case CONTROLU :
                    case CONTROLX :
                      while (incnt != 0) delich();
                    break;



                    case CONTROLR :
                      if (!isctlr)
                        {
                          if (incnt != 0)
                            while (incnt != 0)
                              {
                                delich();
                                ++isctlr;
                              }
                        }
                      else
                        while (isctlr != 0)
                          {
                            hpinch(*inbufp++);
                            ++incnt;
                            --isctlr;
                          }
                    break;



                    default :
                      if (incnt < HBUFLEN - 1)
                        {
                          hpinch(inchar);
                          *inbufp++ = inchar;
                          ++incnt;
                        }
                      else
                        hputc(BELL);
                    break;



                  } /* end switch (inchar) */

              } /* end if (!isctlr || inchar == CONTROLR) */
            else
              hputc(BELL);

          } /* end if (ishget() == YES && !ishput()) */

      } /* end if (!ishmod) */
    else
      { /* HOSTMODE ist eingeschaltet */
        actch = 0;
        selmfl();
        while (ishget() == YES)
          {
            inchar = hgetc() & 0xFF;
            switch (hmstat)
              {

                case 0 :
                  hmch = inchar;
                  ++hmstat;
                break;



                case 1 :
                  hmcmd = inchar;
                  ++hmstat;
                break;



                case 2 :
                  hmlen = inchar;
                  ++hmstat;
                  inbufp = hinbuf;
                  incnt = 0;
                break;



                case 3 :
                  *inbufp++ = inchar;
                  ++incnt;
                  if (hmlen != 0)
                    --hmlen;
                  else
                    {
                      hmstat = 0;
                      /*-------- extended HOSTMODE nach DG3DBI ---------*/
                      if (   (hmch & 0xFF) == 0xFF         /* Kanal 255 */
                          && hmcmd == YES                  /* Command   */
                          && upcase(*hinbuf) == 'G' )
                        {                        /* spezielle G Abfrage */
                          actch = hmch;          /* Channel setzen      */
                          xGcmd();               /* extended G Kommando */
                          return;                /* Ende..              */
                        }

                      if (((actch = hmch) & 0xFF) <= LINKNMBR)
                        {
                          if (actch != 0)
                            lnkpoi = &lnktbl[actch - 1];
                          inbufp = hinbuf;
                          if (hmcmd == YES)
                            cmd();
                          else
                            if (nmbfre > HBREMSE) /* FEF */
                              {
#ifndef USE_XPID
                                (mbp = (MBHEAD *)allocb())->l2fflg = L2CPID;
#else
                                if (!actch)
                                  (mbp = (MBHEAD *)allocb())->l2fflg = UItxPID;
                                else
                                  (mbp = (MBHEAD *)allocb())->l2fflg = lnkpoi->txPID;
#endif
                                while (incnt--) putchr(*inbufp++,mbp);
                                rwndmb(mbp);
                                if (!actch)
                                  {
                                    if (myid[0] != ' ')
                                      {
                                        sdui( ch0via,
                                              ch0id,
                                              myid,
                                              HDLCPORT,
                                              mbp
                                            );
                                        rspsuc();
                                      }
                                    else
                                      rspnsc();
                                    dealmb(mbp);
                                  }
                                else
                                  {
                                    if (lnkpoi->state != L2SDSCED)
                                      tolink(mbp);
                                    else
                                      dealmb(mbp);
                                    rspsuc();
                                  }
                              } /* end if (nmbfre >= NOTBREMSE) */
                            else
                              rsplig();
                        } /* end if ((actch = hmch) <= LINKNMBR) */
                      else
                        {
                          hmputr(HMRFMSG);
                          hputs("INVALID CHANNEL NUMBER");
                          hputc(0);
                        }
                      return;
                    } /* end else from if (hmlen != 0) */
                break;



              } /* end switch (hmstat) */
          } /* end if (ishget() == YES) */
      } /* end else from if (!ishmod) */
  }





/**************************************************************************\
*                                                                          *
* "level 2 to level 7"                                                     *
*                                                                          *
\**************************************************************************/

void l2tol7(msg,lnk,sel)

char        msg;
LNKBLK     *lnk;
unsigned    sel;

  {
    char       *txt;
    unsigned    n;
    MBHEAD     *mbp;

    switch (msg)
      {
        case L2MCONNT :
          if (Upar && !lnkpoi->state)
            {
#ifndef USE_XPID
              (mbp = (MBHEAD *)allocb())->l2fflg = L2CPID;
#else
              (mbp = (MBHEAD *)allocb())->l2fflg = lnkpoi->txPID;
#endif
              for (txt = Utxt, n = 0; n < Utcnt; ++n) putchr(*txt++,mbp);
              putchr(CR,mbp);
              rwndmb(mbp);
              tolink(mbp);
            }

        case L2MDISCF :
        case L2MBUSYF :
        case L2MFAILW :
        case L2MLRESF :
        case L2MLREST :
        case L2MFRMRF :
        case L2MFRMRT :
#ifdef USE_XPID
        case L2MPIDCH :
#endif
          sttoch(msg);
        break;

        case L2MBUSYT :
          putstr("CONNECT REQUEST fm ",mbp = (MBHEAD *)allocb());
          putid(txfhdr,mbp);
          putv(NO,txfhdr + L2ILEN,mbp);
          stampb(&mbp->btime);
          mbp->type = msg;
          relink(mbp,statml.tail);
        break;
      }
  }





/**************************************************************************\
*                                                                          *
* "from link"      FEF                                                     *
*                                                                          *
\**************************************************************************/
BOOLEAN fmlink(par,fbp)
unsigned    par;
MBHEAD     *fbp;
  {

   static unsigned chnr;

/* Einschub fuer Remote-Kommando //Q (DG0FT 17.01.93) */
    unsigned    savmbgc;
    char       *savmbbp;

    if (!ishmod && Upar == 2)              /* kein Terminal aktiv      */
      {
        savmbgc = fbp->mbgc;               /* get counter sichern      */
        savmbbp = fbp->mbbp;               /* buffer pointer sichern   */

        if (   fbp->mbpc - savmbgc >= 3    /* mind. 3 Zeichen im Frame */
            &&        getchr(fbp)  == '/'
            &&        getchr(fbp)  == '/'
            && upcase(getchr(fbp)) == 'Q')

          fbp->l2link->flag |= L2FDSLE;    /* Disconnect anmelden      */

        fbp->mbgc = savmbgc;               /* get counter zurueck      */
        fbp->mbbp = savmbbp;               /* buffer pointer zurueck   */
      }
/* Ende des Einschubs */


    chnr = fbp->l2link - lnktbl;           /* Kanalnummer des Links .. */

    if (!(par || Upar)) {               /* Wenn Flow-Control erwnscht */

             /* Bytezahl im Kanal testen, wenn mehr als erlaubt,  */
             /* Frame vom L2 zurckweisen                         */
             /* Falls bereits BUSY, warten, bis der Kanal frei ist*/

      if (fbp->l2link->flag & L2FBUSY)
        {
          if (chnlbc[chnr]) return(FALSE);
        }
      else
        if (chnlbc[chnr] > MAXLBUF) return(FALSE);
    }
    chnlbc[chnr] += fbp->mbpc;        /* Bytezahl im Kanal aufaddieren */
    fbp->type = L2MNIX;                    /* Buffer-Typ setzen        */
    relink(tfunlink(fbp),chnlml[chnr].tail); /* einh„ngen am Listenende  */
    return(TRUE);                          /* alles klar ...           */
  }


/**************************************************************************\
*                                                                          *
* "to link"                                                                *
*                                                                          *
\**************************************************************************/

void tolink(mbp)

MBHEAD *mbp;

  {
    relink(mbp,lnkpoi->sendil.tail);
    ++lnkpoi->tosend;
  }





/**************************************************************************\
*                                                                          *
* "command"                                                                *
*                                                                          *
\**************************************************************************/

void cmd()
  {
    static unsigned c;

    if (nxtnos() == YES)
      {
        c = upcase(*inbufp++ & 0xFF);
        --incnt;
        nxtnos();
        if (c >= '@' && c <= 'Z')
          (*cmdtab[c - '@'])(c);        /* "(c)" fuer rspcic() ...        */
        else
          rspic(c);
      }
    else
      rspsuc();
  }



/* Ende von TFA.C */
