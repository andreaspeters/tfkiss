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
*    TFB.C   -   The Firmware, Teil 2, Kommandos                           *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 04.05.91                                       *
*                   extcmd(): @A1 - setzen/anzeigen von A1                 *
*                                   (SRTT-Berechnung bei steigendem RTT)   *
*                             @A2 - setzen/anzeigen von A2                 *
*                                   (SRTT-Berechnung bei fallendem RTT)    *
*                             @B  - setzen/anzeigen von B                  *
*                                   (T1 = A3 x SRTT)                       *
*                             @I  - maximale Laenge der I-Frames fuer      *
*                                   IPOLL (0 = IPOLL abgeschaltet)         *
*                             @U  - freie Buffer anzeigen (war frueher @B) *
*                   Fcmd():   Anstelle FRACK in Sekunden jetzt Setzen von  *
*                             IRTT, bzw. Anzeigen vom aktuellen SRTT-Wert. *
*                                                                          *
*                   DL8ZAW, 18.05.91                                       *
*                   extcmd(): @A3 - setzen/anzeigen von A3par (war: @B)    *
*                             @B  - wieder anzeigen der freien Buffer      *
*                             @M  - Zeichen mit 7 bit im Terminal-Mode     *
*                                   maskieren, 1 = ja, 0 = nein            *
*                                                                          *
*                   DL8ZAW, 31.05.91                                       *
*                   Ypar():   Neue Form der Anzeige: "nmblks / Ypar"       *
*                             also: "aktive Links / max. zulaessige Links" *
*                                                                          *
*                   DL8ZAW, 19.06.91                                       *
*                   Aenderung nach DL1MEN:                                 *
*                   Dcmd(): Waehrend LINK SETUP bei ESC-D ein DISC senden  *
*                                                                          *
*                   DB2OS,  26.08.91                                       *
*                   Fcmd(): Keine Werte <16 zulassen, um Probleme mit der  *
*                           InkompatibilitÑt zum ehemaligen FRACK zu       *
*                           beseitigen (z.B. alte Config-Files).           *
*                                                                          *
*                   DB2OS,  26.08.91                                       *
*                   Ycmd(): Anderes Format wegen InkompatibilÑtsproblemen  *
*                           bei einigen Terminalprogrammen.                *
*                           Nun: "Ypar (nmblks)"                           *
*                                                                          *
*                   DB2OS,  03.09.91                                       *
*                   Pcmd(): Neues Format:  "Ppar (Palt)"                   *
*                   Wcmd(): Neues Format:  "Wpar (Walt)"                   *
*                           Bei DAMA-Betrieb werden nur Palt und Walt      *
*                           Åberschrieben und n i c h t Ppar und Wpar!     *
*                                                                          *
*                   DB2OS,  17.09.91                                       *
*                   extcmd(): @U  Parameter fur UIPOLL ein/aus.            *
*                                                                          *
*                   DL8ZAW, DB2OS, 02.05.92                                *
*                   par:    globaler Parameter, um Platz im Eprom zu sparen*
*                   Fcmd(): Eingaben < 16 werden in FRACK (1..15 Sekunden) *
*                           umgerechnet.                                   *
*                                                                          *
*                   DB2OS,  23.06.92                                       *
*                   Pcmd(): Wieder ursprÅngliches Ausgabeformat, angezeigt *
*                           werden jedoch nur die aktuellen Parameter.     *
*                   Wcmd(): Walt entfernt wegen DCD-Problemen.             *
*                                                                          *
*                   DB2OS,  01.07.93                                       *
*                   xGcmd(): erweiterter HOSTMODE nach DG3DBI.             *
*                                                                          *
*                   DB2OS,  23.11.93                                       *
*                   M7par: 7Bit Ausgabe im Terminal Mode entfernt.         *
*                                                                          *
*                   DG2FEF, 18.07.94                                       *
*                   extcmd(): Untere Grenze von A3par zu Testzwecken nun 1 *
*                   Dcmd(): Code optimiert.                                *
*                                                                          *
*                   DB2OS,  12.08.94                                       *
*                   extcmd(): Keine Manipulation von A1, A2 und A3 mehr.   *
*                   Tcmd()  : Tpar (TXDELAY) auf maximal 64 begrenzt       *
*                                                                          *
*                   DG2FEF, 13.08.94                                       *
*                   Gcmd(): Flow-Control eingebaut                         *
*                   Icmd(): Code optimiert, mîglich durch énderung von     *
*                           inilbl().                                      *
*                                                                          *
*                   DB2OS,  19.08.94                                       *
*                   Gcmd(): Im Monitor-Mode werden Frames mit mehr als     *
*                           256Byte DatenlÑnge unterdrÅckt und stattdessen *
*                           eine Fehlermeldung ausgegeben. TCPIP verwendet *
*                           teilweise Åberlange AX.25 Frames, die bei      *
*                           einigen Hostmode-Programmen dann zu einem      *
*                           Crash fÅhren konnten.                          *
*                                                                          *
*                   DB2OS,  27.08.94                                       *
*                   @B-Befehl:Ausgabe von nmbfre mit 2 multipliziert,      *
*                             damit Anzeige der freien Buffer wieder       *
*                             mit TF2.6 kompatibel ist. Die Buffer sind    *
*                             jetzt doppelt so gross wie bei der TF2.6     *
*                                                                          *
*                   DL8HBS, 19.10.94                                       *
*                   @P-Befehl zum Einstellen der TX-PID, kanalselektiv     *
*                                                                          *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "tf.h"          /* Festlegungen/Datenstrukturen fuer TheFirmware */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "tfext.h"       /* globale Variable / nicht int-Funktionen       */

#include "kiss.h"

static unsigned par;

/* external function declarations */
extern void DIinc();
extern void cpyid();
extern void dealmb();
extern void decEI();
extern void dsclnk();
extern void frhmon();
extern void inilks();
extern void rsppar();
extern void hmputr();
extern void hputbt();
extern void hputc();
extern void hputid();
extern void hputmb();
extern void hputs();
extern void hputud();
extern void hputvl();
extern void inilbl();
extern void kiss();
extern void listch();
extern void newlnk();
extern void pushtx();
extern void rspalc();
extern void rspcs();
extern void rspex();
extern void rspic();
extern void rspics();
extern void rspiec();
extern void rspini();
extern void rspipa();
extern void rspiv();
extern void rspnsc();
extern void rspnwc();
extern void rsppar();
extern void rspsuc();
extern void rwndmb();
extern void setiSRTT();
extern short upcase();

/**************************************************************************\
*                                                                          *
*  Auswertung der '@'-Kommandos                                            *
*                                                                          *
\**************************************************************************/
void extcmd()
  {
    static unsigned   parnr;
    static unsigned   cmdch;

    if (incnt != 0)
      {
        cmdch = upcase(*inbufp++ & 0xFF);
        --incnt;
        nxtnos();
        switch (cmdch)
          {
            case 'B' :
              rsppar(nmbfre << 1);  /* TF2.6 KompatibilitÑt: nmbfre * 2 !! */
            break;

            case 'D' :
              if (!incnt)
                rsppar(Dpar);
              else
                if ((par = bgetp()) <= 1)
                  {
                    if ((Dpar = par) == YES)
                      {
                        DIinc();
                        pushtx();
                        decEI();
                      }
                    if (!(damaok && fulldup_on_dama)) {
                      send_kisscmd(CMD_FULLDUP,par);
                    }
                    rspsuc();
                  }
                else
                  rspiv(par);
            break;

            case 'F' :
              if (!incnt)
                rsppar(xFpar);
              else
                if ((par = bgetp()) <= 1)
                  {
                    xFpar = par;
                    rspsuc();
                  }
                else
                  rspiv(par);
            break;

            case 'I' :
              if (!incnt)
                rsppar(Ipar);
              else
                if ((par = bgetp()) <= 256)
                  {
                    Ipar = par;
                    rspsuc();
                  }
                else
                  rspiv(par);
            break;

            case 'K' :
              if (ishmod == YES)                  /* im Hostmode ist @K   */
                rspiec('K');                      /* ungueltig            */
              else                                /* ansonsten            */
                kiss();                           /* und KISS einschalten */
            break;

#ifdef USE_XPID
            case 'P' :                            /* PID setzen */
              if (!incnt)
              {
                if (!actch) rsppar(UItxPID & 0x0ff);
                else rsppar(lnkpoi->txPID & 0x0ff);
              }
              else
              if ((par = bgetp()) <= 255)
              {
                if (!actch) UItxPID = par;
                else lnkpoi->txPID = par;
                rspsuc();
              }
              else
                rspiv(par);
            break;
#endif

            case 'S' :
              if (actch != 0)
              {
                rsppar(lnkpoi->state);
              }
              else
                rspiec(cmdch);
            break;

            case 'T' :
              if (incnt != 0 && 
                  (*inbufp == '2' || *inbufp == '3' ||
                   *inbufp == 'A' || *inbufp == 'a'))
                {
                  --incnt;
                  parnr = *inbufp++ & 0xFF;
                  if (!incnt)
                    if (parnr == '2')
                      rsppar(T2par);
                    else if (parnr == '3')
                      rsppar(T3par);
                    else
                      rsppar(xTApar);
                  else
                    {
                      par = bgetp();
                      if (parnr == 'A' || parnr == 'a') {
                        if (par <= 255) {
                          xTApar = par;
                          send_kisscmd(CMD_TXTAIL,par);
                          rspsuc();
                        }
                        else {
                          rspiv(par);
                        }
                      }
                      else {
                        if (parnr == '2')
                          T2par = par;
                        else
                          T3par = par;
                        rspsuc();
                      }
                    }
                }
              else
                rspiec(cmdch);
            break;

            case 'U' :                            /* UIPOLL ein/aus?      */
              if (!incnt)
                rsppar(UIpar);
              else
                if ((par = bgetp()) <= 1)
                {
                  UIpar = par;
                  rspsuc();
                }
                else
                  rspiv(par);
            break;

            case 'V' :
              if (!incnt)
                rsppar(VCpar);
              else
                if ((par = bgetp()) <= 1)
                  {
                    VCpar = par;
                    rspsuc();
                  }
                else
                  rspiv(par);
            break;


            default :
              rspiec(cmdch);
            break;

          } /* end switch (cmdch) */
      } /* end if (incnt > 0) */
    else
      rspsuc();
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/

void Ccmd()
  {
    char        id[L2IDLEN];
    char       *idp;
    unsigned    n;
    LNKBLK     *linkp;

    if (!incnt)
      rspcs();
    else
      {
        if (!actch)
          {
            if (bgetid(NO,id) == YES && bgetvl(VCpar,ch0via) != ERROR)
              {
                cpyid(ch0id,id);
                rspsuc();
                return;
              }
          }
        else
          {
            if (myid[0] == ' ')
              {
                rspnsc();
                return;
              }
            if (bgetid(VCpar,id) == TRUE)
              {
                idp = !*lnkpoi->srcid ? myid : lnkpoi->srcid;
                for (n = 0, linkp = lnktbl; n < LINKNMBR; ++n, ++linkp)
                  if (linkp->state != L2SDSCED) {
                    if (cmpid(linkp->dstid,id) == TRUE)
                      {
                        if (    linkp != lnkpoi
                             && cmpid(   linkp->srcid[0] == '\0'
                                       ? myid
                                       : linkp->srcid,idp
                                     ) == TRUE
                           )
                          {
                            rspini(HMRFMSG);
                            hputs("STATION");
                            rspalc();
                            return;
                          }
                      }
                    else
                      if (linkp == lnkpoi)
                        {
                          rspini(HMRFMSG);
                          hputs("CHANNEL");
                          rspalc();
                          return;
                        }
		  }
                if (bgetvl(VCpar,lnkpoi->viaidl) != ERROR)
                  {
                    cpyid(lnkpoi->dstid,id);
                    lnkpoi->liport = HDLCPORT;
                    if (!*lnkpoi->srcid) cpyid(lnkpoi->srcid,myid);
                    newlnk();
                    rspsuc();
                    return;
                  }
              } /* end if (bgetid(VCpar,id)) */
          } /* end else from if (!actch) */
        rspics();
      } /* end else from if (!incnt) */
  }




/**************************************************************************\
* FEF                                                                      *
*                                                                          *
*                                                                          *
\**************************************************************************/
void Dcmd()
  {

    if (actch != 0)
      if (lnkpoi->state != L2SDSCED)
        {
          dsclnk();
          rspsuc();
        }
      else
        {
          inilbl();
          rspini(HMRSMSG);
          hputs("CHANNEL NOT CONNECTED");
          rspex();
        }
    else
      rspic('D');
  }



/**************************************************************************\
*  Fcmd()    Setzen des IRTT bzw. Anzeigen des aktuellen SRTT.             *
*            FrÅher FRACK in Sekunden, nun IRTT in 10ms Schritten!         *
*            Eingaben < 16 werden mit 100 multipliziert!                   *
\**************************************************************************/
void Fcmd()
{
    unsigned par;

    if (!incnt)
      rsppar(!actch ? Fpar : (lnkpoi->state != L2SDSCED ? lnkpoi->SRTT :
                                                          lnkpoi->IRTT));
    else 
      if ((par = bgetp()))                             /* darf nicht 0 sein */
        {
          if (par < 16)                       /* alte Parameter umrechnen */
            par *= 100 / A3par;        /* wenn par in Sekunden eingegeben */

          if (!actch)
            {
              Fpar = par;
              inilks();
            }
          else
            {
              lnkpoi->IRTT = par;
              if (lnkpoi->state != L2SDSCED) setiSRTT();
            }
          rspsuc();
        }
      else
        rspiv(par);
}



/**************************************************************************\
*                                                                          *
* action      :  G-Kommando (nur fuer Hostmode).                           *
*                                                                          *
*                Infoframedaten, Linkstatus-Meldungen, Monitorheader       *
*                und Monitorframeinfodaten im Hostmode abholen.            *
*                                                                          *
*                Kanal 0   :  G  - Linkstatus/Monitorheader/Monitorinfo    *
*                             G0 - Monitorheader/Monitorinfos              *
*                             G1 - Linkstatus                              *
*                                                                          *
*                Kanal > 0 :  G  - Linkstati/Infoframedaten                *
*                             G0 - Infoframedaten                          *
*                             G1 - Linkstati                               *
*                                                                          *
*                Im Kanal 0 ist die einzig moegliche Linkstatusmeldung ein *
*                nicht angenommener Connect-Request, in den anderen        *
*                Kanaelen werden keine Connect-Request-Meldungen           *
*                ausgegeben.                                               *
*                                                                          *
*                Es wird eine der Anforderung entsprechende Hostmode-      *
*                antwort ausgegeben.                                       *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  -                                                         *
*                                                                          *
* r/o globals :  ishmod  -  YES = wir sind im Hostmode, sonst Terminalmode *
*                incnt   -  Anzahl Zeichen hinter 'G' im Eingabebuffer     *
*                inbufp  -  Zeiger hinter 'G' in Eingabebuffer             *
*                actch   -  Kanal des Hostmodekommandos                    *
*                                                                          *
* r/w globals :  mifmbp  -  Zeiger auf Framekopf eines I/UI-Frames, von    *
*                           dem der Rumpf (die Daten) noch gemonitort      *
*                           werden muessen                                 *
*                statml  -  Liste der auszugebenden Statusmeldungen fuer   *
*                           Kanal 0 (Hostmode nur Connect-Requests)        *
*                smonfl  -  Liste der zu monitorenden Frames (Vorauswahl)  *
*                chnlml[] - Kanal-Frame/Meldungs-Listen                    *
*                rxf...  -  werden gesetzt, wenn neues Frame zu monitoren  *
*                rxfctl  -  wird gesetzt und abgefragt,  "  "  "  "  "     *
*                                                                          *
* locals      :  s.u.                                                      *
*                                                                          *
* returns     :  -                                                         *
*                                                                          *
\**************************************************************************/

void Gcmd()
  {
    static BOOLEAN     done;  /* Flag "Hostmodeantwort ausgegeben"        */
    static MBHEAD     *mbp;   /* Zeiger auf Kopf des aktuellen Frames     */
    static unsigned    n;

    if (ishmod == YES)                            /* im Hostmode ?        */
      {
        if (incnt != 0)                           /* ja, Parameter da ?   */
          {
            if ((par = bgetp()) > 1)              /* ja, holen            */
              {
                rspiv(par);                       /* ... falsch angegeben */
                return;
              }
          }
        else                                      /* kein Parameter, alle */
          par = MBALL;                            /* Typen sind gemeint   */

        /* G : par = MBALL   G0 : par = MBINFO   G1 : par = MBSTATUS */
        if (!actch)                               /* Monitorkanal ?       */
          if (    mifmbp != NULL                  /* ja, noch alter Rest  */
               && par != MBSTATUS                 /*     kein Status ?    */
             )
            {
              hmputr(HMRMONI);

              n = mifmbp->mbpc - mifmbp->mbgc;    /* DatenlÑnge ermitteln */
              if (n <= 257 )   /* Nicht mehr als 256 Datenbytes zulÑ·ig!  */
              {
                hputc(n-1);            /* Anzahl der folgenden Datenbytes */
                while (mifmbp->mbgc < mifmbp->mbpc)
                  hputc(getchr(mifmbp));
              }
              else
              {
                hputc(15);    /* stattdessen eine Fehlermeldung ausgeben. */
                hputs("[frame too long]");
              }
              dealmb(mifmbp);
              mifmbp = NULL;                      /*     (kein Rest mehr) */
              }
          else                                         /* ... oder        */
            if (    par != MBINFO                      /* Status/Alles    */
                 && (mbp = selmb(&statml,par)) != NULL /* angefordert     */
               )                                       /* und Status da ? */
              {
                tfunlink(mbp);                           /* ja, holen       */
                rwndmb(mbp);                           /* und ausgeben    */
                hmputr(HMRSTAT);                       /* (Antworttyp)    */
                hputmb(mbp);                           /* (Daten)         */
                if (stamp)
                  {
                    hputs(" - ");
                    hputbt(&mbp->btime);
                  }
                hputc(0);                              /* (Ende)          */
                dealmb(mbp);                           /* (freigeben)     */
              }
            else
              {
                if (par != MBSTATUS)                   /* ... oder        */
                  {                                    /* Info/Alles      */
                    done = NO;                         /* angefordert und */
                    while (    !done
                            && (mbp = selmb(&smonfl,MBALL)) != NULL
                          )
                      {                                /* da ?            */
                        tfunlink(mbp);                   /* ja, holen, und  */
                        if (ismonf(mbp) == YES)        /* ggf. monitoren  */
                          {
                            if (    (!(rxfctl & L2CNOIM) || rxfctl == L2CUI)
                                 && (mbp->mbpc - mbp->mbgc > 1)
                               )
                              {                        /* bei I/UI erst   */
                                hmputr(HMRMONIH);      /* Header und beim */
                                mifmbp = mbp;          /* naechsten G die */
                              }                        /* Daten (Rest)    */
                            else
                              hmputr(HMRMONH);         /* sonst nur Head. */
                            frhmon(mbp);               /* Header ausgeben */
                            hputc(0);
                            done = YES;
                          }
                        if (!mifmbp) dealmb(mbp);      /* war kein UI/I   */
                      } /* end while */
                    if (!done) rspsuc();               /* wenn nix getan  */
                  } /* end if (par != MBSTATUS) */
                else                                   /* Status angef.,  */
                  rspsuc();                            /* aber keiner da  */
              }
        else /* from if (!actch) */
          {                                       /* Kanal > 0, passendes */
            done = NO;
            while (!done && (mbp = selmb(&chnlml[actch - 1],par)) != NULL)
              {
                tfunlink(mbp);                      /* Frame/Meldung holen  */
                if (!mbp->type)
                  {                               /* ist Infoframe, wenn  */
                    if (mbp->mbgc < mbp->mbpc)    /* Daten drin, alle     */
                      {                           /* ausgeben             */
                        hmputr(HMRCONI);
                        hputc(mbp->mbpc - mbp->mbgc - 1);
                        while (mbp->mbgc < mbp->mbpc) hputc(getchr(mbp));
                        chnlbc[actch-1] -= mbp->mbpc; /* FEF Flow-Control */
                        done = YES;
                      }
                  }
                else
                  {
                    rwndmb(mbp);                  /* ist Linkstatus-      */
                    hmputr(HMRSTAT);              /* meldung, diese       */
                    hputmb(mbp);                  /* ausgeben             */
                    chnlbc[actch-1] -= mbp->mbpc; /* FEF Flow-Control     */
                    if (stamp)
                      {
                        hputs(" - ");
                        hputbt(&mbp->btime);
                      }
                    hputc(0);
                    done = YES;
                  }
                dealmb(mbp);                      /* Buffer freigeben     */
              }
            if (!done) rspsuc();                  /* wenn nix getan ...   */
          } /* end else from if (!actch) */
      } /* end if (ishmod == YES) */
    else
      rspic('G');                                 /* T.-Mode: G ungueltig */
  }


/**************************************************************************\
*                                                                          *
* EXTENDED HOSTMODE COMMAND    nach DG3DBI, Åberarbeitet von DB2OS         *
*                                                                          *
* Der Rechner sendet ein 'G'-Kommando an den Kanal 255 des TNC.            *
* Handelt es sich um einen TNC ohne Erweiterung, so antwortet dieser dann  *
* mit der Fehlermeldung "INVALID CHANNEL NUMBER".                          *
* Bei dem erweiterten Hostmode wird stattdessen ein Null-terminierter      *
* String zurÅckgeliefert, der eine Liste von KanÑlen enthÑlt, bei denen    *
* noch Infos in den Buffern abholbereit sind. Dies gilt auch fÅr den       *
* Monitor-Kanal und Statusdaten. Die Kanalnummern im String sind wegen     *
* der Null-Terminierung um 1 erhîht.                                       *
* Beispiele fÅr Antwort des TNC:                                           *
*       0xFF 0x01 0x00   - Keine Daten verfÅgbar.                          *
*       0xFF 0x01 0x01 0x00 - Es sind Daten im Monitor-Buffer.             *
*       0xFF 0x01 0x01 0x03 0x04 0x00 - Es sind Daten im Monitor und       *
*                                       in den KanÑlen 2 und 3.            *
\**************************************************************************/
void xGcmd()
{
  static unsigned n;                /* index auf KanÑle               */

  hmputr(HMRSMSG);                  /* Success with Response          */

  if (   (mifmbp != NULL)           /* Ist was im Monitor-Buffer?     */
      || (statml.head != &statml)
      || (smonfl.head != &smonfl)   )
    hputc(1);                       /* Ja, melden...                  */

  for (n = 0; n < LINKNMBR; ++n)    /* Haben wir etwas auf den Links? */
    if (chnlbc[n])                  /* FEF ByteCounter != 0 ?         */
      hputc(n+2);                   /* Ja, diesen Kanal melden..      */

    hputc(0);                       /* Zum Schlu· die 'Ende'-Kennung  */
}


/**************************************************************************\
*                                                                          *
* FEF                                                                      *
*                                                                          *
\**************************************************************************/
void Icmd()
  {
    if (!incnt)
      {
        rspini(HMRSMSG);
        hputid(   !actch
                ? myid
                : lnkpoi->srcid
              );
        rspex();
      }
    else
      if (!actch)
        if (bgetid(VCpar,myid) == TRUE)
          {
            inilks();
            rspsuc();
          }
        else
          rspics();
      else
        if (!lnkpoi->state)
          if (bgetid(VCpar,lnkpoi->srcid) == TRUE)
            {
              if (myid[0] == ' ')
                {
                  cpyid(myid,lnkpoi->srcid);
                  inilks();
                }
              rspsuc();
            }
          else
            rspics();
        else
          rspnwc();
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/
void Lcmd()
  {
    static unsigned unacked;
    static unsigned n;

    if (!ishmod)
      if (!incnt)
        for (n = 0; n < LINKNMBR + 1; ++n)
          listch(n);
      else
        if ((par = bgetp()) <= LINKNMBR)
          listch(par);
        else
          rspiv(par);
    else
      {
        hmputr(HMRSMSG);
        if (!actch)
          {
            hputud(mbcnt(&statml,MBALL));
            hputc(' ');
            hputud(mbcnt(&smonfl,MBALL));
          }
        else
          {
            hputud(mbcnt(&chnlml[actch - 1],MBSTATUS));
            hputc(' ');
            hputud(mbcnt(&chnlml[actch - 1],MBINFO));
            hputc(' ');
            unacked = (lnkpoi->VS - lnkpoi->lrxdNR) & 7;
            hputud(lnkpoi->tosend - unacked);
            hputc(' ');
            hputud(unacked);
            hputc(' ');
            hputud(   !lnkpoi->state
                    ? 0
                    : (!lnkpoi->T1 ? lnkpoi->tries : lnkpoi->tries + 1)
                  );
            hputc(' ');
            hputud(lnkpoi->state);
          }
        hputc(0);
      }
  }




/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/
void Mcmd()
  {
    short getres;
    unsigned flag;
    unsigned inchr;

    if (!incnt)
      {
        rspini(HMRSMSG);
        if ((Mpar & (MONI | MONU | MONS)) != NO)
          {
            if ((Mpar & MONI) != NO) hputc('I');
            if ((Mpar & MONU) != NO) hputc('U');
            if ((Mpar & MONS) != NO) hputc('S');
            if ((Mpar & MONC) != NO) hputc('C');
          }
        else
          hputc('N');
        if (mftsel != NO)
          {
            hputc(' ');
            hputc(!(mftsel - 1) ? '+' : '-');
            hputvl(NO,mftidl);
          }
        rspex();
      } /* end if (!incnt) */
    else
      {
        flag = 0;
        while (incnt != 0 && flag != 0xFFFF)
          {
            --incnt;
            if ((inchr = upcase(*inbufp++ & 0xFF)) == ' ' || inchr == ',')
              continue;
            if (inchr == 'N')
              {
                if (flag == 0x00FF) continue;
                if (!flag)
                  flag = 0x00FF;
                else
                  flag = 0xFFFF;
              }
            else
              if (inchr == '+' || inchr == '-')
                {
                  if ((getres = bgetvl(NO,mftidl)) != ERROR)
                    mftsel = !getres ? 0 : (inchr == '+' ? 1 : 2);
                }
              else
                if (flag != 0x00FF)
                  switch (inchr)
                    {
                      case 'I' :   flag |= MONI;   break;
                      case 'U' :   flag |= MONU;   break;
                      case 'S' :   flag |= MONS;   break;
                      case 'C' :   flag |= MONC;   break;
                      default  :   flag = 0xFFFF;
                    }
                else
                  flag = 0xFFFF;
          } /* end while */
        if (flag != 0xFFFF)
          {
            if (flag != 0) Mpar = (flag == 0x00FF) ? 0 : flag;
            rspsuc();
          }
        else
          rspipa();
      } /* end else from if (!incnt) */
  }



/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/
void Ncmd()
  {
    if (!incnt)
      rsppar(!actch ? Npar : lnkpoi->N2);
    else
      if ((par = bgetp()) <= 127)
        {
          if (!actch)
            {
              Npar = par;
              inilks();
            }
          else
            lnkpoi->N2 = par;
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
void Ocmd()
  {
    if (!incnt)
      rsppar(!actch ? Opar : lnkpoi->k);
    else
      if ((par = bgetp()) >= 1 && par <= 7)
        {
          if (!actch)
            {
              Opar = par;
              inilks();
            }
          else
            lnkpoi->k = par;
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
void Pcmd()
  {
    if (!incnt)
      {
        rspini(HMRSMSG);
        hputud(Ppar & 0xFF);          /* aktuelle P-Persistence.          */
        rspex();
      }
    else
      if ((par = bgetp()) <= 255)
        {
          Ppar = par;                 /* Neue P-Persistence               */
          send_kisscmd(CMD_PERSIST,par);
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
void Rcmd()
  {
    if (!incnt)
      rsppar(Rpar);
    else
      if ((par = bgetp()) <= 1)
        {
          Rpar = par;
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
void Scmd()
  {
    if (!ishmod)
      if (!incnt)
        rsppar(actch);
      else
        if ((par = bgetp()) <= LINKNMBR)
          {
            lnkpoi = &lnktbl[(actch = par) - 1];
            rspcs();
          }
        else
          rspiv(par);
    else
      rspic('S');
  }





/**************************************************************************\
*                                                                          *
*                                                                          *
*                                                                          *
\**************************************************************************/
void Tcmd()
  {
    if (!incnt)
      rsppar(Tpar);
    else
      if ((par = bgetp()) <= 64)
        {
          Tpar = par;
          send_kisscmd(CMD_TXDELAY,par);
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
void Wcmd()
  {
    if (!incnt)
      {
        rspini(HMRSMSG);
        hputud(Wpar);                   /* aktuelle Slot-Time               */
        rspex();
      }
    else
      if ((par = bgetp()) <= 127)
        {
          Wpar = par;
          send_kisscmd(CMD_SLOTTIME,par);
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
void Ycmd()
  {
    if (!incnt)
      {
        rspini(HMRSMSG);
        hputud(Ypar);                 /* Maximale Anzahl von Connects     */
        hputs(" (");
        hputud(nmblks);               /* Aktuelle Anzahl von Verbindungen */
        hputs(")");
        rspex();
      }
    else
      if ((par = bgetp()) <= LINKNMBR)
        {
          Ypar = par;
          rspsuc();
        }
      else
        rspiv(par);
  }



/* Ende von TFB.C */
