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
*    L2A.C   -   Level 2, Teil 1                                           *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 06.05.91, Aenderungen fuer DAMA-Betrieb        *
*                   l2tx():   I-Frames nur senden, wenn senden erlaubt     *
*                   l2rx():   DAMA-Modus einschalten, wenn Frame von       *
*                             DAMA-Master an mich gerichtet ist und        *
*                             Sendung freigeben                            *
*                   l2timr(): T1: bei DAMA nicht auf Ablauf warten,        *
*                                 sondern sofort nach Sendeaufforderung    *
*                                 vom DAMA-Master POLL senden.             *
*                                 IPOLL nach DK6PX eingebaut.              *
*                             T2: bei DAMA nur senden, wenn erlaubt.       *
*                             Bei DAMA Aussendung starten, wenn senden     *
*                             erlaubt und was zu senden da.                *
*                                                                          *
*                   DL8ZAW, 07.05.91                                       *
*                   Neues Flag fuer lnktbl.flag: L2FDPOLL                  *
*                   Im DAMA-Modus wird nach Sendeaufforderung vom DAMA-    *
*                   Master nicht auf Ablauf von T1 gewartet um zu pollen.  *
*                   Damit nicht sofort nach Setzen von T1 gepollt wird,    *
*                   wird mit L2FDPOLL angezeigt, dass bei ersten Aufruf    *
*                   von l2timr() nach Setzen von T1 nicht gepollt werden   *
*                   soll. L2FDPOLL wird am Ende von l2timr() geloescht.    *
*                                                                          *
*                   DL8ZAW, 19.05.91                                       *
*                   kicktx(0) geschuetzt gegen Interrupts.                 *
*                                                                          *
*                   DL8ZAW, 19.06.91                                       *
*                   Aenderung nach DL1MEN:                                 *
*                   l2timr(): Bei Link Failure DISC aussenden              *
*                                                                          *
*                   DL8ZAW, 23.07.91                                       *
*                   l2rx(): Einschalten des DAMA-Modus nur bei nicht       *
*                           disconnecteten Links                           *
*                                                                          *
*                   DL8ZAW, 23.07.91                                       *
*                   l2rx(): Beim Einschalten von DAMA Persistance auf 255  *
*                           und Slottime auf 0 setzen; alte Werte merken   *
*                   l2rest(): Beim Auschalten von DAMA Persistance und     *
*                             Slottime wieder auf alte Werte setzen        *
*                                                                          *
*                   DB2OS,  23.08.91                                       *
*                   l2rx(): Alte Werte fuer Persistence und Slottime       *
*                           nur merken, wenn DAMA nicht bereits aktiv war. *
*                                                                          *
*                   DL8ZAW/DB2OS, 28.08.91                                 *
*                   l2rx():  Auf das Freigeben der Sendung nach DCD-Abfall *
*                            mittels 'startx' wird ganz verzichtet. Statt- *
*                            dessen wird nach Empfang eines DAMA-Frames,   *
*                            dass an das eigene Call gerichtet ist, die    *
*                            Erlaubnis zu senden (sendok=1) SOFORT frei-   *
*                            gegeben.                       ------         *
*                            Sendok wird, wie bisher, wieder auf 0 gesetzt,*
*                            wenn alles gesendet ist (l2timr()), bzw. ein  *
*                            neues Frame empfangen wird (L2D.C, l1put()).  *
*                                                                          *
*                   DB2OS,  23.09.91                                       *
*                   l2rx():  Auf ein empfangenes SABM auch im DAMA-Mode    *
*                            _sofort_ antworten.                           *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                            #ifdef FIRMWARE entfernt weil der Source nur  *
*                            noch fuer TheFirmware verwendbar ist.         *
*                   l2tx():                                                *
*                   l2rx():  Frames nur dann in Monitor, wenn mehr als     *
*                            256 Buffer frei sind (vorher 64)              *
*                                                                          *
*                   DB2OS, 23.06.92                                        *
*                   l2trx():  Wpar - Slottime wird nicht mehr 0 gesetzt,   *
*                             da der TNC oft mit Knoten kollidiert.        *
*                                                                          *
*                   DB2OS, 26.06.93                                        *
*                   l2timr(): Fehler bei der Reihenfolge in der IF-Abfrage *
*                             behoben (falsche Klammer!)                   *
*                             Dadurch gab es bei Multiconnect falsche      *
*                             Pollframes und die "Meckermeldungen" trotz   *
*                             DAMA-Mode...                                 *
*                                                                          *
*                   DL4YBG, 23.10.93                                       *
*                   l2tx(): T1-Timer wird in l1get gestartet, nachdem      *
*                           Frame gesendet wurde und nicht hier, wenn      *
*                           Gesendet-Liste abgearbeitet wird. Dieses       *
*                           fuehrt zu Verfaelschungen der SRTT-Berechnung. *
*                                                                          *
*                                                                          *
*                   DL8AAU, 12.6.94                                        *
*                   l2rx(): Problem mit Flexnet-DAMA. Nur noch auf Sendung,*
*                           wenn Frame mit gesetztem Poll-Bit empfangen    *
*                           wurde (analog DK4EG Beschreibung)              *
*                   l2timr(): IPOLL bei DAMA ist sinnlos, gesperrt         *
*                                                                          *
*                   DG2FEF: 18.06.94                                       *
*                   l2rx(): DAMA-Poll Reaktion Åberarbeitet. Nur noch wenn *
*                           gepollt auf DAMA schalten, Slottime auf 50  ms *
*                           setzen und alten Wert merken. Dann srxdNR()    *
*                           aufrufen und die noch unbestÑtigten Frames aus *
*                           der letzten Runde zum Senden freigeben         *
*                           Wenn UA+ mit DAMA-Bit kommt, sendok = 1        *
*                   l2tx(): L2FDPOLL hei·t jetzt L2FJSENT, wird in l2rx()  *
*                           gebraucht.                                     *
*                   l2timr(): Keine Timerreaktionen mehr bei DAMA          *
*                             L2FDPOLL-Lîschen entfernt                    *
*                                                                          *
*                   DG2FEF, 18.07.94                                       *
*                   l2rx(): Bei Umschalten auf DAMA werden jetzt gar keine *
*                           Parameter mehr verÑndert. Durch einen Patch im *
*                           L1 geht der TNC mit sendok = 1 ohne Warten     *
*                           und ohne DCD-Beachtung auf Sendung.            *
*                           öberflÅssigen Code in der Suchroutine fÅr den  *
*                           richtigen Linkblock entfernt.                  *
*                           tol3sw() entfernt, es gibt eh keinen l3 mehr.  *
*                           I-Frame Behandlung angepasst.                  *
*                           UI-Behandlung nach weiter hinten verlegt.      *
*                           Auch KanÑle, die ein anderes Rufzeichen als    *
*                           das auf Kanal 0 haben, werden jetzt beachtet.  *
*                           L2FPOLL eingefÅhrt: Gesetzt, wenn Link unter   *
*                           DAMA gepollt wird.                             *
*                           Der DAMA-Slave verwirft nun alle Frames, die   *
*                           nicht vom Master an ihn gesendet werden.       *
*                                                                          *
*                   l2tx(): ZusÑtzlich L2FMAXF1 und L2FRETRY eingefÅhrt,   *
*                           fÅr Framesammler-UnterstÅtzung unter DAMA      *
*                                                                          *
*                   l2timr(): T1 wieder zugelassen fÅr alle Links im Auf-  *
*                             bzw. Abbau. T1,T2 gestoppt, wenn DCD on.     *
*                             Ablaufkriterium fÅr T2 unter DAMA ist jetzt  *
*                             der Master-Poll, damit I-Frames auf alle     *
*                             FÑlle bestÑtigt werden kînnen.               *
*                             Aufruf von sdipoll() in State-Machine ver-   *
*                             lagert.                                      *
*                             DAMA-Relevante Flags werden jetzt hier ge-   *
*                             lîscht, nicht mehr in l2rest();              *
*                                                                          *
*                   DG2FEF 13.08.94                                        *
*                   l2rx(): Framesammler nach OE1HHC/OE3GMW eingebaut      *
*                           und verbessert (Datensicherheit).              *
*                           Suchroutine nach dem nÑchsten freien           *
*                           Linkblock gekÅrzt. Es wird nun nur noch        *
*                           abgefragt, ob ein freier Linkblock  mit dem    *
*                           entsprechenden Quellcall vorhanden ist.        *
*                           Eine énderung in inilbl() sorgt dafÅr, da· die *
*                           Linkblîcke stets mit dem in myid[] gespeicher- *
*                           ten Call initialisiert werden.                 *
*                           `damati` fest auf 255 Sekunden, Bpar entfallen.*
*                                                                          *
*                   DG2FEF 21.08.94                                        *
*                   l2rx(): schwerwiegenden Fehler beim Einbau obiger      *
*                           Routinen behoben.                              *
*                                                                          *
*                   DG2FEF/DB2OS 22.08.94                                  *
*                   l2rx(): Die fehlende Abfrage auf L2CNONRM konnte bei   *
*                           DAMA zu einem FRMR beim Empfang eines DISC     *
*                           fÅhren..                                       *
*                                                                          *
*                   DB2OS  29.08.94                                        *
*                   l2tx(), l2rest(), l2timr():                            *
*                   Modifikation zur Geschwindigkeitssteigerung nach       *
*                   DL1HAZ aus TheNetNode Åbernommen.                      *
*                                                                          *
*                   DB2OS  08.09.94                                        *
*                   l2tx(), l2rest(), l2timr():                            *
*                   Modifikation zur Geschwindigkeitssteigerung noch       *
*                   einmal Åberarbeitet bzw. ÅberflÅssige Abfrage nach     *
*                   (lnkpoi < &lnktbl[LINKNMBR]) entfernt..                *
*                                                                          *
*                   DG2FEF/DB2OS  19.09.94                                 *
*                   Time T3 wieder aktiviert, damit es bei "abgefaulten"   *
*                   DAMA Links (speziell wenn noch ein weiteres QSO lÑuft  *
*                   keinen HÑnger gibt..                                   *
*                                                                          *
*                   DL4YBG 06.11.94                                        *
*                   PID-Auswertung von empfangenen Frames, wenn            *
*                   Aenderung, dann Link-Message senden.                   *
*                                                                          *
*                   DL4YBG 13.02.95 (DG0FT, 16.01.95)                      *
*                   l2rx(): empfangene FlexNet-Frames werden verworfen     *
*                                                                          *
\**************************************************************************/




/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "l2s.h"         /* Zugriff auf die State-Tabellen                */
#include "l2ext.h"       /* globale Variable / nicht int-Funktionen       */
#include "tfext.h"       /* globale Variable / nicht int-Funktionen       */
#include "kiss.h"

/* external function declarations */
extern void DIinc();
extern void clrT3();
extern void clrlnk();
extern void cpyid();
extern void dealmb();
extern void dealml();
extern void decEI();
extern void disc();
extern void i2tolx();
extern void inilnk();
extern BOOLEAN iscd();
extern void kicktx();
extern void l2stma();
extern void l2tolx();
extern void mclrlk();
extern void reslnk();
extern void sdfrmr();
extern void sdi();
extern void sdl2fr();
extern void sdoi();
extern void setT3();
extern void stxfad();
extern void xdisc();
extern void xdm();
extern void xua();

#ifdef DUMMY
int iscd(int i) { i = i; return FALSE; };
#endif

/**************************************************************************\
*                                                                          *
* "level 2"                                                                *
*                                                                          *
* Der Level 2. Es werden alle Level-2-internen Aktionen ausgefuehrt und    *
* Meldungen an hoehere Level weitergegeben (Informationstransfer von/zum   *
* Level 2 und Kommandos an den Level 2 geschehen von ausserhalb).          *
*                                                                          *
*                                                                          *
* Der Level 2 laeuft wie folgt ab :                                        *
*                                                                          *
*   - Aufruf von l2init()                                                  *
*                                                                          *
*   - zyklisches Aufrufen von l2() und l2timr(ticks)                       *
*                                                                          *
*   Statusaenderungen im Level 2 (Connects, Disconnects, Failures, usw.)   *
*   werden hoeheren Leveln vom Level 2 aus ueber                           *
*                                                                          *
*     l2tolx(<status>)  ->  l2tol3(<status>), l2tol7(<status>,lnkpoi,2)    *
*                                                                          *
*   mitgeteilt.                                                            *
*                                                                          *
*   Ein Connectwunsch wird dem Level 2 ueber das Besetzen eines leeren     *
*   Linkblocks mit Quell- und Ziel- sowie Digicalls und Aufrufen von       *
*   newlnk() mitgeteilt (lnkpoi zeigt auf Linkblock !).                    *
*   Ein newlnk() auf einen bestehenden Link erzeugt einen Link Reset.      *
*                                                                          *
*   Ein Disconnectwunsch (oder bei mehrmaligem Aufruf der sofortige        *
*   Disconnect) wird ueber das Setzen von lnkpoi auf den jeweiligen        *
*   Linkblock und Aufruf von dsclnk() erreicht.                            *
*                                                                          *
*   Der Informationstransfer zum Level 2 geschieht von aussen durch        *
*   Aufruf von itolnk(...), vom Level 2 durch itolx(..), welches dann      *
*   fmlink() aus dem hoeheren Level aufruft.                               *
*                                                                          *
*   Ueber sdui(..) koennen unproto-Pakete (UI-Frames) gesendet werden.     *
*                                                                          *
*   Level-3-Pakete (Level-3-UI-Pakete oder Infopakete in Sequenz eines     *
*   Level-2-3-Links) werden ausgefiltert und in die Level-3-Frameliste     *
*   eingehaengt.                                                           *
*                                                                          *
\**************************************************************************/


/**************************************************************************\
*                                                                          *
* "level 2 transmitter"                                                    *
*                                                                          *
* Falls Infopakete zu senden sind, laut Sendefenster keine Infopakete      *
* ausstehen, und laut Sendestatus auch gesendet werden duerfen, diese      *
* senden.                                                                  *
* Im DAMA-Modus nur senden, wenn senden erlaubt.                           *
*                                                                          *
* Frames aus Gesendet-Liste holen und in die Monitorframeliste umhaengen   *
* Entsprechend dem Frameinhalt ggf. Timer 1 starten                        *
*                                                                          *
\**************************************************************************/
void l2tx()
{
  static unsigned short    l2state;        /* aktueller Linkstate               */
  static unsigned short    n;              /* Laufindex                         */
  static MBHEAD     *sfbp;           /* Sendeframebufferpointer           */

  /************************************************************************/
  /* Modifikation zur Geschwindigkeitssteigerung: In dieser Routine sind  */
  /*  nur diejenigen Linkbloecke von Interesse, deren Zustand nicht dis-  */
  /*  connected ist. Die Anzahl der aktiven Linkbloecke steht in der glo- */
  /*  balen Variable "nmblks". Sind alle aktiven Linkbloecke bearbeitet   */
  /*  worden, dann kann die Schleife abgebrochen werden.                  */
  /************************************************************************/
  for (n = nmblks, lnkpoi = lnktbl; n > 0; ++lnkpoi)
  {
    if (lnkpoi->state != L2SDSCED)
    {
      --n;               /* einen aktiven Linkblock weniger zu bearbeiten */
      if (    (l2state = lnkpoi->state) == L2SIXFER           /* duerfen  */
           ||                   l2state == L2SRS              /* wir was  */
           ||                   l2state == L2SDBS             /* senden ? */
           ||                   l2state == L2SRSDBS
         )
      {
        if (sendok)
          lnkpoi->flag &= ~(L2FPOLL|L2FJSENT);

        if ( lnkpoi->VS == lnkpoi->lrxdNR ) {    /* nichts ausstehend ?   */
          if (!(lnkpoi->flag & L2FMAXF1))        /* FEF wenn nicht MAXF1  */
            sdi(lnkpoi->k);                      /* ...dann Infos senden  */
          else
          {
            sdi(1);                              /* FEF sonst nur 1 Frame */
            lnkpoi->VS = lnkpoi->oldVS;          /* FEF V(S) restaurieren */
          }
        }
      } /* if (was zu senden) */
    } /* if (aktiver Link und nicht Disconnected) */
  } /* for (alle aktiven Links) */

  if (damaok)                                /* DAMA-Sender           */
  {
    if (sendok)                   /* senden erlaubt und was zu senden */
    {
      if (tosend)
      {
        DIinc();
        kicktx(0);
        decEI();
        tosend = 0;
      }
      sendok = 0;
    }

    if (!nmblks || !damati)         /* letzter Link oder Timerablauf  */
    {                               /* Alle DAMA-Flags lîschen        */
      for (n = 0, lnkpoi = lnktbl; n < LINKNMBR; ++n, ++lnkpoi)
        if (lnkpoi->state) lnkpoi->flag &= ~L2FDAMA;
          damaok = 0;               /* DAMA Modus ausschalten         */
          if (fulldup_on_dama) {
            send_kisscmd(CMD_FULLDUP,Dpar);
          }
          if (tosend)               /* Sendqueue leeren               */
          {
            DIinc();
            kicktx(0);
            decEI();
            tosend = 0;
          }
    }
  } /* if (DAMA Sender) */

  while ((sfbp = (MBHEAD *)stfl.head) != (MBHEAD *)&stfl)
                                                  /* Gesendetliste        */
  {                                               /* aufraeumen :         */
    tfunlink(sfbp);                                 /* Frame holen          */
                                /* DL4YBG: T1 wird nun in l1get gestartet */
    if (nmbfre > MBREMSE)                         /* falls noch genug     */
      relink(sfbp,monfl.tail);                    /* Platz, Frame in den  */
    else                                          /* Monitor              */
      dealmb(sfbp);                               /* oder deallokieren    */
  }

}




/**************************************************************************\
*                                                                          *
* "level 2 receiver"                                                       *
*                                                                          *
* Alle Frames aus der RX-Frameliste holen und analysieren. Kopie an        *
* Monitorliste, digipeaten oder in Level-3-Liste, falls erforderlich.      *
* Auf UI-Frames antworten, falls erforderlich.                             *
*                                                                          *
* Reaktion entsprechend Protokoll, siehe unten.                            *
*                                                                          *
\**************************************************************************/

void l2rx()
  {
    static unsigned short    l2state;      /* aktueller Level 2 Linkstatus      */
    static BOOLEAN     tome;         /* YES = Frame ist an mich           */
    static BOOLEAN     raus;         /* FEF Erstickunsbremsflag           */
    static unsigned short    n;            /* Laufindex                         */
    static MBHEAD     *fbp;          /* Framebufferpointer lokal          */
    static LNKBLK     *lblkp;        /* Linkblockpointer lokal            */
    static unsigned short    i;            /* FEF Framesammler Kontrollflag     */

    raus = NO;                /* FEF Mindestens einmal mÅssen wir schon.. */

    while (   !raus          /* FEF solange wir nicht an Frames ersticken */
           && (fbp = (MBHEAD *)rxfl.head) != (MBHEAD *)&rxfl) 
                                                  /* solange empfangene   */
      {                                           /* Frames vorhanden     */
        tfunlink(fbp);                              /* eins aus Liste holen */

        if (!takfhd(fbp))                         /* Kopf analysieren,    */
          {                                       /* wenn nicht ok, dann  */
            dealmb(fbp);                          /* wegwerfen und zum    */
            continue;                             /* naechsten            */
          }

        fbp->type = 2;                            /* wir sind im Level 2  */

        if (nmbfre > MBREMSE)                     /* wenn genug Platz,    */
          relink(cpyfb(fbp),monfl.tail);          /* Kopie an Monitor     */

#ifdef USE_FLEXDEC
        if (rxfflx)                               /* wenn FlexNet-Frame   */
          {
           dealmb(fbp);                           /* dann wegwerfen und   */
           continue;                              /* zum naechsten Frame  */
          }
#endif

         if (digipt(fbp) == YES) continue;         /* ... nur Digipeater   */


        /* Haben wir einen zum Frame passenden Linkblock ?                */
        /*                                                                */
        /* Alle Linkbloecke durchgucken. Wenn ein aktiver Linkblock       */
        /* gefunden wurde, dessen Quellcall mit dem Framezielcall         */
        /* uebereinstimmt, tome auf TRUE setzen. Wenn auch noch Blockport */
        /* mit Frameport und Blockzielcall mit Framequellcall             */
        /* uebereinstimmen, dann ist der aktive passende Link gefunden,   */
        /* Schleife abbrechen.                                            */
        /* FEF                                                            */
        /* Falls ein Link inaktiv ist, aber das Blockquellcall mit dem    */
        /* Framezielcall uebereinstimmt, dann Blockadresse in lblkp       */
        /* merken. Es wird nur der erste solche Block genommen.           */

        for ( tome = NO, lblkp = NULL, n = 0, lnkpoi = lnktbl;
              n < LINKNMBR;
              ++n, ++lnkpoi
            )
          if (lnkpoi->state != L2SDSCED)
            {
              if (cmpid(lnkpoi->srcid,rxfhdr) == TRUE)
                {
                  tome = YES;
                  if (    lnkpoi->liport == rxfprt
                       && cmpid(lnkpoi->dstid,rxfhdr + L2IDLEN) == TRUE
                     ) break;
                }
            }
          else
            if (    !lblkp
                 && cmpid(lnkpoi->srcid,rxfhdr)
               ) lblkp = lnkpoi;


        if (n == LINKNMBR) {                  /* wenn kein aktiver Link    */
          if (lblkp)                         /* passt, aber inaktiver     */
            lnkpoi = lblkp;                  /* Link, dann diesen nehmen  */
          else
            {
             if (tome || istome(rxfhdr))     /* Kein freier Kanal mehr fÅr*/
               if (rxfPF && rxfCR)           /* QSO aber Antwort erwÅnscht*/
                {
                 xdm();                      /* dann DM- senden           */
                 if (rxfctl == L2CSABM)      /* wenn SABM, zusÑtzlich Msg */
                   l2tolx(L2MBUSYT);         /* ans Terminal              */
                }

             dealmb(fbp);                    /* empfangenes Frame weg-    */
             continue;                       /* werfen und zum naechsten  */
            }
        }


        /* Falls Timer 3 aktiv, diesen neu setzen, es ist wieder          */
        /* Aktivitaet auf dem Link                                        */

        if (lnkpoi->T3 != 0) setT3();

        l2state = lnkpoi->state;        /* Linkstatus zur Abfrage         */


        /* Fuer nicht disconnectete Links:                                */
        /* Falls RX-Frame von DAMA-Master, DAMA Timer neu setzen und      */
        /* DAMA Modus einschalten und Sendung freigeben nach DCD-Abfall.  */
        /* FEF Sendung nur freigeben, wenn auch gepollt. DCD wird nicht   */
        /* mehr abgefragt.                                                */
        /* FEF Ausserdem ÅberprÅfen, ob noch Frames unbestÑtigt ausstehen */
        /* Wenn ja, V(S) resetten, ++lnkpoi->retries                      */
        /* Wenn inaktiver Link gepollt wurde, Sender trotzdem freigeben   */
        /* fÅr UA- oder DM-                                               */

        if (!(rxfhdr[L2ILEN-1] & L2CDAMA))             /* FEF DAMA-Frame? */
         {
          damaok = 1;                                 /* dann DAMA an     */
          if (fulldup_on_dama) {
            send_kisscmd(CMD_FULLDUP,1);
          }
          damati = 255;                             /* Timeout neu        */
          lnkpoi->flag |= L2FDAMA;                  /* Link ist DAMA-Slave*/
          if (rxfPF && rxfCR)                       /* FEF Wenn gepollt   */
           {
            l2stma(stb10a);                         /* DAMA POLL RECEIVED */
            sendok = 1;                             /* Sender freigeben   */
            lnkpoi->flag |= L2FPOLL;                /* Link wurde gepollt */
            if (l2state)                            /* aktiver Link ?     */
             if (   (rxfctl & L2CNONRM) != 0x03     /* Frame enthÑlt N(R) */
                 && !(lnkpoi->flag & L2FJSENT)      /* wenn Flag stimmt   */
                 && srxdNR() == YES           /* und gÅltiger ZÑhlerstand */
                 && lnkpoi->VS != lnkpoi->lrxdNR    /* was zu senden da?  */
                )
               {
                if (!(lnkpoi->flag & L2FMAXF1)) /* Nur wenn 1. Retry      */
                  {
                   lnkpoi->flag |= L2FMAXF1;    /* Maxframe = 1           */
                   lnkpoi->oldVS = lnkpoi->VS;  /* V(S) retten            */
                  }
                ++lnkpoi->tries;                /* RetryzÑhler erhîhen    */
                lnkpoi->VS = lnkpoi->lrxdNR;    /* beim nÑchsten Aufruf   */
               }                                /* von l2tx() geht's raus */
           }
         }

        if (rxfctl == L2CUI)                 /* Falls UI-Frame ...        */
          {
            if (rxfPF && rxfCR)              /* und Antwort erwÅnscht ... */
              xdm();                         /* dann DM- senden ...       */
            dealmb(fbp);                     /* Frame wegwerfen ...       */
            continue;                        /* und zum nÑchsten Frame.   */
          }
        else
        if (!(rxfctl & L2CNOIM))        /* I-Frame ?                      */
          {

            /* I-Frame :                                                  */
            /*                                                            */
            /* Nur annehmen, wenn empfangene N(R) des Frames ok,          */
            /* srxdNR(), und das I-Frame das naechste erwartete in der    */
            /* Sequenz ist, isntxi().                                     */
            /* Wenn alles ok, Laenge pruefen und ggf. auf falsche Laenge  */
            /* mit Frame-Reject reagieren, sonst Antwort entsprechend     */
            /* Statetable und I-Frame verarbeiten.                        */

            if (    srxdNR() == TRUE                   /* N(R) ok ?       */
                 && (i = isnxti())                     /* erwartet ?      */
               ) {
              if (fbp->mbpc - fbp->mbgc <= 257)        /* Laengenpruefung */
                {

                  fbp->l2fflg = getchr(fbp);           /* Rest von tol3sw */

                  if (lnkpoi->rcvd > 2) raus = YES; /* FEF L2 zu langsam..*/

                  /* Linkzustand I-Transfer moeglich und nicht busy ? */

                  if (i == 1)                             /* erwartetes I */
                    if (   l2state >= L2SIXFER
                        && !(lnkpoi->flag & L2FDSLE)
#ifndef USE_XPID
                        && (fbp->l2fflg & 0xFF) == L2CPID
#endif
                        && nmbfre > L2BREMSE
                       )
                      {
#ifdef USE_XPID
                        if (fbp->l2fflg != lnkpoi->rxPID) {
                          lnkpoi->rxPID = fbp->l2fflg;
                          l2tolx(L2MPIDCH);
                        }
#endif
                        relink(fbp,lnkpoi->rcvdil.tail); /*in Liste hÑngen*/
                        ++lnkpoi->rcvd;

                        lnkpoi->flag |= L2FIRCVD; /* FEF */

                        if (lnkpoi->colled)     /* FEF ist der Sammler an */
                          {
                            while (lnkpoi->colled) /* Alle Frames aus dem */
                              {                    /* Sammler umhÑngen    */
                                relink(tfunlink(lnkpoi->collil.head),
                                              lnkpoi->rcvdil.tail);
                                ++lnkpoi->rcvd;
                                --lnkpoi->colled;
                              }
                            lnkpoi->VR = lnkpoi->lastVR; /* V(R) updaten  */
                          }
                        else
                          lnkpoi->xpctVR = 8;          /* FEF */
                      }
                    else
                      dealmb(fbp);
                  else                              /* I ausser der Reihe */
                    {                               /* aber Ok fÅr Sammler*/
                      relink(fbp,lnkpoi->collil.tail); /* in Framesammler */
                      continue;                        /* zum NÑchsten    */
                    }

                  l2stma(!rxfPF ? stbl01 : stbl00);    /* Statetable      */
                  continue;           /* Frame abgearbeitet, zum nÑchsten */
                }
                /* FIXME: Das hier ist nonsens und kaschiert lediglich die
                 *  Unzulaenglichkeiten des Host-Mode
                 */
              else                      /* Frame zu lang :                */
                sdfrmr(0x03);           /* "U/S-Frame mit unerlaubtem     */
            }                           /* Infofeld"                      */
          }



        else                                 /* kein I-Frame :            */
          if (!(rxfctl & L2CNOSM))
            {

              /* S-Frame :                                                */
              /*                                                          */
              /* Nur annehmen, wenn empfangene N(R) des Frames ok,        */
              /* srxdNR(), und wenn das Frame kein Infofeld enthaelt.     */
              /*                                                          */
              /* Auf RR, RNR, REJ entsprechend Statetable antworten, auf  */
              /* andere mit Frame-Reject antworten.                       */

              if (srxdNR() == YES)                     /* N(R) ok ?       */
                {

                  if (fbp->mbgc == fbp->mbpc)          /* kein I-Feld ?   */
                    switch ((rxfctl >> 2) & 0x03)
                      {
                        case 0 :                       /* L2CRR >> 2      */
                          l2stma(   !rxfCR
                                  ? (!rxfPF ? stbl11 : stbl10)
                                  : (!rxfPF ? stbl03 : stbl02)
                                 );
                        break;

                        case 1 :                       /* L2CRNR >> 2     */
                          l2stma(   !rxfCR
                                  ? (!rxfPF ? stbl15 : stbl14)
                                  : (!rxfPF ? stbl07 : stbl06)
                                );
                        break;

                        case 2 :                       /* L2CREJ >> 2     */
                          l2stma(   !rxfCR
                                  ? (!rxfPF ? stbl13 : stbl12)
                                  : (!rxfPF ? stbl05 : stbl04)
                                );
                          if (l2state >= L2SIXFER)
                            sdoi();
                        break;

                        default :
                          sdfrmr(0x01);   /* "Kontrollfeld falsch oder      */
                        break;            /* nicht implementiert"           */

                    } /* end switch ((rxfctl >> 2) & 0x03) */
                  else
                    sdfrmr(0x03);         /* "U/S-Frame mit unerlaubtem     */
                                          /* Infofeld"                      */
              }
            } /* end S-Frame */



          else                               /* kein I- oder S-Frame :    */
            if ((rxfctl & 0xFF) != L2CFRMR)

              /* Kein FRMR-Frame, Frame nur annehmen, wenn kein Infofeld  */
              /* vorhanden.                                               */
              /*                                                          */
              /* Frame auswerten, reagieren, nach Statetable antworten.   */

              if (fbp->mbgc == fbp->mbpc)
                switch (rxfctl)
                  {

                    case L2CSABM :             /* neuer Link / Linkreset  */
                      switch (l2state)
                        {                      /* neuer Link (Connect) ?  */
                          case L2SDSCED :
                            if (    fvalca(VCpar,rxfhdr + L2IDLEN) == TRUE
                                 && nmblks < Ypar
                                 && nmbfre > CBREMSE   /* annehmbar ?     */
                                 && rxfV2 == YES       /* und V2-Link ?   */
                               )
                              {
                                inilnk();              /* ja, Link init.  */
                                ++nmblks;
                                l2tolx(L2MCONNT);      /* melden          */
                                break;                 /* -> Statetable   */
                              }
                            l2tolx(L2MBUSYT);          /* nein, melden    */
                            xdm();                     /* mit DM antworten*/
                            dealmb(fbp);               /* Frame vergessen */
                            continue;                  /* naechstes Paket */
                          break;

                          case L2SLKSUP :              /* beide connecten */

                            /* anderer Weg als selbst benutzt ? */

                            if (   !cmpidl(   cmpid(rxfhdr + L2IDLEN,
                                                    lnkpoi->srcid) == TRUE
                                            ? rxfhdr + L2ILEN
                                            : txfhdr + L2ILEN,
                                            lnkpoi->viaidl
                                          )
                                 || rxfV2 == NO        /* oder V1-Link    */
                               )
                              {
                                l2tolx(L2MBUSYF);           /* abbrechen  */
                                clrlnk();                   /* ja, alles  */
                                xdm();
                                lnkpoi->state = L2SDSCED;
                                dealmb(fbp);
                                continue;
                              }
                            else
                              {
                                reslnk();                   /* nein,      */
                                cpyid(lnkpoi->dstid,txfhdr);
                                l2tolx(L2MCONNT);           /* gelungen   */
                              }
                          break;

                          case L2SDSCRQ :         /* sind ge-disct, Link  */
                            mclrlk();             /* aufloesen und melden */
                          break;

                          default :
                            if (lnkpoi->flag & L2FIRCVD)
                              {                   /* normaler Linkreset   */
                            inilnk();             /* vom Partner          */
                            l2tolx(L2MLRESF);
                              }
                          break;

                        } /* end switch (l2state) */

                      l2stma(stbl08);             /* SABM EITHER COMMAND  */
                    break;



                    case L2CDISC :
                      if (!l2state)               /* Link aktiv ?         */
                        {
                          if (    rxfPF != 0      /* nein, wenn Command   */
                               && rxfCR != 0      /* mit Poll, dann mit   */
                             ) xdm();             /* DM antworten         */
                          else
                            xua();                /* sonst mit UA         */
                          dealmb(fbp);            /* Frame wegwerfen      */
                          continue;               /* naechstes Paket      */
                        }
                      else                        /* ja,                  */
                        if (l2state == L2SLKSUP)  /* wenn im Linkaufbau,  */
                          {                       /* dann Link sofort     */
                            l2tolx(L2MBUSYF);     /* aufloesen und melden */
                            clrlnk();
                          }
                        else                      /* sonst erst restliche */
                          {                       /* I-Frames an hoeheren */
                            i2tolx(YES);          /* Level geben und dann */
                            mclrlk();             /* Link loesen / melden */
                          }
                      l2stma(stbl09);             /* DISC EITHER COMMAND  */
                    break;



                    case L2CUA :
                      if (l2state < L2SRS)             /* V1-Zustand ?    */
                        {
                          if (l2state == L2SLKSUP)     /* ja, wenn im     */
                            {                          /* Link-Setup      */
                              reslnk();                /* uebernehmen,    */
                              cpyid(lnkpoi->dstid,txfhdr);
                              l2tolx(L2MCONNT);        /* Link neu        */
                            }
                          else                         /* sonst wenn im   */
                            if (l2state == L2SDSCRQ)   /* Disc-Request    */
                               mclrlk();               /* Link aufloesen  */
                        }
                      else
                        {
                          reslnk();                    /* nein, Linkreset */
                          l2tolx(L2MLREST);            /* ausf. / melden  */
                        }
                      l2stma(stbl16);             /* UA EITHER RESPONSE   */
                    break;



                    case L2CDM :
                      if (l2state) {              /* wenn Link aktiv ...  */
                        if (l2state == L2SLKSUP)  /* wenn DM beim Link-   */
                          {                       /* Setup, dann Link     */
                            l2tolx(L2MBUSYF);     /* sofort aufloesen und */
                            clrlnk();             /* "Busy from" melden   */
                          }
                        else                      /* sonst Link aufloesen */
                          mclrlk();               /* mit Meldung          */
                      }
                      l2stma(stbl17);             /* DM EITHER RESPONSE   */
                    break;



                    default :           /* unbekanntes Kontrollfeld :     */
                      sdfrmr(0x01);     /* "Kontrollfeld falsch oder      */
                    break;              /* nicht implementiert"           */



                  } /* end switch (rxfctl) */
              else                                /* Frametyp unbekannt   */
                sdfrmr(0x03);                     /* "U/S-Frame mit un-   */
                                                  /* erlaubtem Infofeld"  */



            else /* from if (rxfctl != L2CFRMR) */
              {

                /* FRMR-Frame :                                           */
                /*                                                        */
                /* Wird nur im Frame-Reject-Zustand oder bei moeglichem   */
                /* Informationstransfer angenommen.                       */
                /* Es werden die FRMR-Infobytes gelesen, FRMR an die      */
                /* hoeheren Level gemeldet, nach Statetable geantwortet.  */

                if (l2state >= L2SIXFER || l2state == L2SFRREJ)
                  {
                    /* FRMR-Infobytes im Linkblock merken */
                    for (n = 0; n < 3; ++n)
                      lnkpoi->frmr[n] = (fbp->mbgc < fbp->mbpc) ? getchr(fbp) : 0;
                    l2tolx(L2MFRMRF);
                  }
                l2stma(stbl18);                   /* FRMR EITHER RESPONSE */

              }



        dealmb(fbp);          /* aktuelles Frame verarbeitet, wegwerfen   */

      } /* end while ((fbp = rxfl.head) != &rxfl) */
  }





/**************************************************************************\
*                                                                          *
* "level 2 rest"                                                           *
*                                                                          *
* Fuer alle aktiven Links Busyzustand pruefen/setzen/aufloesen, I-Pakete   *
* unter Beruecksichtigung der "Erstickungskontrolle" an hoehere Level      *
* weiterreichen. Fall Zustand "Disconnecten nach Uebertragung der          *
* restlichen I-Pakete" und keine I-Pakete mehr zu senden, Disconnect       *
* einleiten.                                                               *
* Muellbufferliste frei machen (aus Interruptroutinen entstandener Muell,  *
* der besser ausserhalb der Interrupts deallokiert wird aus Zeitgruenden). *
*                                                                          *
* Bei abgelaufenenem DAMA-Timer sofort DAMA-Modus abschalten und alle noch *
* zu sendenden Frames senden                                               *
*                                                                          *
\**************************************************************************/

void l2rest()
  {
    static unsigned short  n;
    static unsigned short  i;       /* FEF */
    static char      lflag;
    static unsigned short  l2state;

  /************************************************************************/
  /* Modifikation zur Geschwindigkeitssteigerung: In dieser Routine sind  */
  /*  nur diejenigen Linkbloecke von Interesse, deren Zustand nicht dis-  */
  /*  connected ist. Die Anzahl der aktiven Linkbloecke steht in der glo- */
  /*  balen Variable "nmblks". Sind alle aktiven Linkbloecke bearbeitet   */
  /*  worden, dann kann die Schleife abgebrochen werden.                  */
  /************************************************************************/
    for (i = 0, n = nmblks, lnkpoi = lnktbl; n > 0; ++lnkpoi)
      if ((l2state=lnkpoi->state) != L2SDSCED)
        {
          /* fuer alle aktiven (= nicht disconnecteten) Links :           */
          --n;           /* einen aktiven Linkblock weniger zu bearbeiten */
          txfV2 = YES;                             /* Protokollversion    */

          /* wenn Zustand "nachdem alle restliche I's uebertragen wurden, */
          /* disconnecten" und alle I's uebertragen, DISC einleiten       */

          if (     (lflag=lnkpoi->flag) & L2FDAMA
               && l2state >= L2SIXFER) ++i; /* FEF */

          if (     (lflag) & L2FDSLE
               && !lnkpoi->tosend
             ) disc();

          /* sonst empfangene I-Pakete an hoeheren Level uebertragen und  */
          /* Busy-Condition pruefen / setzen / aufheben                   */
          /*                                                              */
          /* "Busy werden"      -   weniger als 80 Freibuffer             */
          /*                        oder Frames im L2, die der LX nicht   */
          /*                        Åbernommen hat                        */
          /*                                                              */
          /* "Busy aufloesen"   -   wieder mehr als 112 Freibuffer        */
          /*                        und alle Frames raus aus dem L2       */

          else
            {
              i2tolx(NO);      /* Weitergabe an Level X, Flow-Control ein */
              if ( !(lflag & L2FBUSY) )          /* nicht busy            */
                {
                  if (nmbfre < L2BREMSE || lnkpoi->rcvd)
                    {
                      lnkpoi->flag |= L2FBUSY;   /* busy werden           */
                      l2stma(stbl21);            /* STATION BECOMES BUSY  */
                    }
                }
              else
                if (nmbfre > L2GAS && !lnkpoi->rcvd)
                  {
                    lnkpoi->flag &= ~L2FBUSY;    /* "busy" aufloesen      */
                    l2stma(stbl22);              /* BUSY CONDITION CLEARS */
                  }
            }
        } /* end if (lnkpoi->state) */

    aktlks = i;
    dealml(&trfl);                      /* Muellbufferliste frei machen   */

  }





/**************************************************************************\
*                                                                          *
* "level 2 timer"                                                          *
*                                                                          *
* Ausfuehren der Level-2-Millisekundentimer 1, 2, 3 in allen aktiven       *
* Links (herunterzaehlen und bei Ablauf reagieren).                        *
* In ticks wird die Anzahl der vergangenen 10ms-Intervalle (Ticks) seit    *
* dem letzten Aufruf dieser Routine angegeben.                             *
* [DG2FEF]                                                                 *        *
* Bei eingeschaltetem DAMA-Modus:                                          *
* Keine Timerbeahndlung mehr, es sei denn, wir haben "Link Setup"          *
* oder "disc. req." auf dem Link.                                          *
\**************************************************************************/

void l2timr(ticks)

unsigned short ticks;

  {
    static unsigned short  n;
    static unsigned short  l2state; /* FEF */
    static char      lflag;

  /************************************************************************/
  /* Modifikation zur Geschwindigkeitssteigerung: In dieser Routine sind  */
  /*  nur diejenigen Linkbloecke von Interesse, deren Zustand nicht dis-  */
  /*  connected ist. Die Anzahl der aktiven Linkbloecke steht in der glo- */
  /*  balen Variable "nmblks". Sind alle aktiven Linkbloecke bearbeitet   */
  /*  worden, dann kann die Schleife abgebrochen werden.                  */
  /************************************************************************/
  for (n = nmblks, lnkpoi = lnktbl; n > 0; ++lnkpoi)
      if ((l2state = lnkpoi->state) != L2SDSCED)
        {
                    /* fuer alle aktiven (= nicht disconnecteten) Links : */
          --n;           /* einen aktiven Linkblock weniger zu bearbeiten */
          txfV2 = YES;                  /* Merker ob Version-2-Protokoll  */

          lflag = lnkpoi->flag;         /* FEF */

          if (lnkpoi->T1 != 0) {        /* wenn Timer 1 aktiv ...         */
            if (lnkpoi->T1 <= ticks)       /*   ... und abgelaufen        */
             {
              if (   !(lflag & L2FDAMA)         /* Bei DAMA nur wenn ..  */
                  || (l2state == L2SLKSUP)      /* Link-Setup oder       */
                  || (l2state == L2SDSCRQ)      /* Disc-Request          */
                 )
               {
                lnkpoi->T1 = 0;            /*   ... Timer 1 stoppen       */
                lnkpoi->flag |= L2FDIRTY;  /*   RTT-Messung ungÅltig.     */
                setT3();                   /*   Timer 3 neu starten       */
                ++lnkpoi->tries;           /*   Retryzaehler              */
                if (    !lnkpoi->N2
                     || lnkpoi->tries < lnkpoi->N2 /* zu viele Retries ?  */
                   )
                  l2stma(stbl23);                  /* Statet. T1 EXPIRES  */
                else
                  {                                /* zu viele Retries :  */
                    lnkpoi->tries = 0;             /* Retryzaehler leer   */
                    xdisc();                       /* DISC aussenden      */
                    l2tolx(L2MFAILW);              /* "Link failure"      */
                    clrlnk();                      /* Link sofort loesch. */
                    lnkpoi->state = L2SDSCED;      /* DISCONNECTED        */
                  }
               }
             }
            else
              if (lnkpoi->T1 > ticks)
                if (   !iscd(lnkpoi->liport)    /* FEF nur wenn keine DCD */
                    || l2state == L2SLKSUP      /* FEF ..."link setup"    */
                    || l2state == L2SDSCRQ)     /* FEF oder "disc. req."  */
                  lnkpoi->T1 -= ticks;    /*   sonst herunterzaehlen      */
          }

          /* Wenn RTT-Messung freigegeben ist, dann RTT um ticks erhîhen  */
          if (lnkpoi->RTT != 0)
            lnkpoi->RTT += ticks;

          if (lnkpoi->RStype != 0) {    /* Wenn Response zu senden ist    */
            if (   (   lnkpoi->T2 <= ticks  /* wenn Timer 2 abgelaufen .. */
                    && !(lflag & L2FDAMA)) /* FEF und kein DAMA          */
                || sendok)                  /* FEF oder Master-Poll       */
             {
              stxfad();                 /* ... Responseframe bauen        */
              txfCR = txfPF = 0;
              txfctl = setNR(   !(lflag & L2FBUSY)
                              ? lnkpoi->RStype
                              : L2CRNR
                           );
              sdl2fr(makfhd(L2FUS));    /* und senden                     */
              lnkpoi->RStype = 0;       /* Responsemodus loeschen         */
              lnkpoi->T2 = 0;           /*   ... Timer 2 stoppen          */
             }
            else
             if (!iscd(lnkpoi->liport))  /* FEF wenn keine DCD            */
               lnkpoi->T2 -= ticks;      /*   dann herunterzaehlen        */
          }

          if (lnkpoi->T3 != 0) {        /* wenn Timer 3 aktiv ...         */
            if (lnkpoi->T3 <= ticks)    /*   wenn Timer 3 abgelaufen ...  */
            {
              lnkpoi->flag &= ~L2FDAMA; /* DAMA abschalten, pollen lassen */
              clrT3();                  /*   ... Timer 3 stoppen und      */
              l2stma(stbl24);           /*       Statetable T3 EXPIRES    */
            }                           /*       ausfuehren               */
            else
              lnkpoi->T3 -= ticks;      /*   sonst herunterzaehlen        */
          }
        }
  }

/* Ende von L2A.C */
