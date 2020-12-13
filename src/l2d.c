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
*    L2D.C   -   Level 2, Teil 4, zeitkritische Routinen                   *
*                                                                          *
*                Achtung: Ronner nicht auf diesen Code anwenden !          *
*                                                                          *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW/DB2OS, 27.08.91                                 *
*                   l1put(): sendok auf 0 setzen (Sende-Freigabe sperren), *
*                            wenn neues Frame in RX-Liste engeh„ngt wird!  *
*                            (siehe L2A.C)                                 *
*                                                                          *
*                   DL4YBG, 23.10.93                                       *
*                   l1get(): Nach Aussenden eines Frames wird T1-Timer     *
*                            direkt gestartet, wenn dies gefordert wird.   *
*                            (wurde bisher in l2tx gemacht und ver-        *
*                             faelschte SRTT-Messung)                      *
*                   * ACHTUNG * In der Assembleroptimierung werden         *
*                               Offsets der Strukturen MBHEAD und          *
*                               LNKBLK benutzt, werden diese Strukturen    *
*                               veraendert, muessen die Offsets angepasst  *
*                               werden!!!                                  *
*                                                                          *
*                   DG2FEF, 18.07.94                                       *
*                   l1get(): RTT-Messung nur starten, wenn nicht schon     *
*                            eine l„uft.                                   *
*                                                                          *
*                   DG2FEF, 13.08.94                                       *
*                   putchr(), : Umgestellt auf 64 Byte Bufferl„nge         *
*                   getchr(),                                              *
*                   rwndmb()                                               *
*                                                                          *
\**************************************************************************/



#define PORTABLE              /* nicht portabler Code, einige Routinen,   */
                              /* die einzigen laengeren Routinen, die aus */
                              /* Interrupts heraus aufgerufen werden,     */
                              /* werden assemberoptimiert (#asm/#endasm)  */





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */
#include "l2ext.h"

/* external function declarations */
extern void reset();
extern void DIinc();
extern void decEI();
extern char sendok;
extern unsigned short nmbfre;
extern LHEAD freel;
extern LHEAD rxfl;
extern LHEAD stfl;
extern LHEAD trfl;
extern LHEAD txl2fl[];
extern MBHEAD *rxfhd[];
extern MBHEAD *txfhd[];

#ifdef PORTABLE
extern LEHEAD     *allocb();
extern LEHEAD     *tfunlink();
extern LEHEAD     *relink();
#endif

/* local function declarations */
unsigned short l1get();
void putchr();
void rlmlsl();
void rwndmb();



/**************************************************************************\
*                                                                          *
* action      :  "level 1 put"                                             *
*                                                                          *
*                Je nach action Zeichen in den aktiven RX-Framebuffer      *
*                schreiben, RX-Framebuffer auf den Muell werfen, oder      *
*                RX-Framebuffer in RX-Frameliste einhaengen.               *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  action  -                                                 *
*                                                                          *
*                  7  6543210 7654321  0                                   *
*                +---+-------+-------+---+  RX-Framebuffer in RX-          *
*                | 1 |  port |       | 0 |  Frameliste einhaengen,         *
*                +---+-------+-------+---+  return NO                      *
*                                                                          *
*                +---+-------+-------+---+  falls RX-Framebuffer angelegt, *
*                | 1 |  port |       | 1 |  diesen auf den Muell werfen    *
*                +---+-------+-------+---+  (neuen beginnen), return NO    *
*                                                                          *
*                +---+-------+-----------+  Zeichen ch in aktiven RX-      *
*                | 0 |  port |     ch    |  Framebuffer schreiben, ist es  *
*                +---+-------+-----------+  das erste Zeichen, neuen       *
*                                           RX-Framebuffer beginnen,       *
*                                           entsprechend der einlaufenden  *
*                                           Zeichen neue Datenbuffer       *
*                                           allokieren und anhaengen,      *
*                                           normal return NO, return YES,  *
*                                           wenn Frame zu gross geworden   *
*                                                                          *
*                                           port gibt den verwendeten      *
*                                           Empfangskanal an (HDLC/RS232)  *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  rxfhd   - Zeiger auf den Kopf des aktiven RX-Frames       *
*                          0, wenn kein RX-Frame aktiv                     *
*                rxfl    - Liste der empfangenen Frames                    *
*                trfl    - Liste der Muell-Frames                          *
*                                                                          *
* locals      :  s.u.                                                      *
*                                                                          *
* returns     :  s.o.                                                      *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

BOOLEAN l1put(action)

unsigned short action;

  {
    unsigned short    port;         /* port aus action                          */
    MBHEAD    **rxfbpp;       /* Zeiger auf Zeiger auf aktuelles RX-Frame */

    rxfbpp = &rxfhd[port = (action>>8) & 0x7F]; /* Adresse RX-Framezeiger */
    if (!(action & 0x8000))                     /* Zeichen oder Befehl ?  */
      {                                         /* Zeichen :              */
        if (!*rxfbpp)                           /* RX-Frame aktiv ?       */
          {
            *rxfbpp = (MBHEAD *)allocb();       /* nein - neues anlegen   */
            (*rxfbpp)->l2port = port;           /*        fuer port       */
          }
        putchr(action & 0xFF,*rxfbpp);          /* Zeichen in Frame       */
        if ((*rxfbpp)->mbpc > L2MFLEN)          /* Framelaengencheck      */
          return (YES);
      }
    else                                        /* Befehl :               */
      if (*rxfbpp)                              /* nur wenn Frame aktiv   */
        {                                       /* Befehl ausfuehren      */
          if (action & 0x0001)
              relink(*rxfbpp, trfl.tail);       /* Frame in Muelleimer    */
          else
            {
              relink(*rxfbpp, rxfl.tail);       /* Frame in RX-Liste      */
              sendok = 0;                       /* Sendefreigabe sperren  */
            }
          *rxfbpp = NULL;                       /* kein RX-Frame aktiv    */
        }
    return (NO);                                /* default return NO      */
  }

#else
/*
#asm
          public l1put?         ; globales Symbol

l1put?:   call  ?ens            ; IX/BC Stackframezeiger (Funktionsparameter)
          defw  -4              ; und 4 Byte lokaler Variablenspeicher
          ld    A,(IX+9)        ; A = MSB(action) & 0x7F
          and   127             ; (0x7F)
          ld    (IX+0),A        ; port = A
          add   A,A             ; rxfbpp = HL = &rxfhd[port]
          ld    E,A
          ld    D,0
          ld    HL,rxfhd?
          add   HL,DE
          ld    (IX+2),L
          ld    (IX+3),H
          bit   7,(IX+9)        ; Befehl oder Zeichen ?
          jr    NZ,?lput2       ; Befehl ...

          ld    E,(HL)          ; Zeichen, DE = *rxfbpp = rxfhd[port]
          inc   HL
          ld    D,(HL)
          ld    A,D             ; Empfangsframe aktiv (*rxfbpp != 0) ?
          or    E
          jr    NZ,?lput1       ; ja   -
          push  HL              ; nein - rxfbpp merken
          call  allocb?         ;        Freibuffer allokieren
          ex    DE,HL           ;        DE = Adresse Freibuffer
          pop   HL              ;        HL = rxfbpp
          ld    (HL),D          ;        *rxfbpp = Adresse Freibuffer
          dec   HL
          ld    (HL),E
          ld    HL,18           ;        Freibuffer->l2port = port
          add   HL,DE
          ld    A,(IX+0)
          ld    (HL),A

?lput1:   push  DE              ; putchr(LSB(action),*rxfbpp);
          ld    L,(IX+8)
          push  HL
          call  putchr?
          pop   DE
          pop   DE              ; DE = *rxfbpp
          ld    HL,10           ; DE = *rxfbpp->mbpc
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          ld    HL,328          ; DE <= L2MFLEN, Frame noch nicht zu lang ?
          or    A
          sbc   HL,DE
          jr    NC,?lput5       ; ja   - Frame ok, return(NO);
          ld    HL,1            ; nein - Frame zu lang, return(YES);
          jr    ?lput6

?lput2:   ld    E,(HL)          ; Befehl, DE = *rxfbpp
          inc   HL
          ld    D,(HL)
          ld    A,D             ; Empfangsframe aktiv (*rxfbpp != 0) ?
          or    E
          jr    Z,?lput5        ; nein - return(NO);
          bit   0,(IX+8)        ; ja   - auf den Muell oder in RX-Liste ?
          jr    NZ,?lput3       ;
          ld    HL,(rxfl?+2)    ; -> in RX-Liste, HL = rxfl.tail
          push  AF              ;* Akku sichern
          ld    A,0             ;*
          ld    (sendok?),A     ;* Sendefreigabe sperren
          pop   AF              ;* Akku zurckholen
          jr    ?lput4
?lput3:   ld    HL,(trfl?+2)    ; -> auf den Muell, HL = trfl.tail
?lput4:   push  HL              ; relink(HL,*rxfbpp); (DE = *rxfbpp)
          push  DE
          call  relink?
          pop   DE
          pop   DE
          ld    L,(IX+2)        ; HL = rxfbpp
          ld    H,(IX+3)
          xor   A
          ld    (HL),A          ; *rxfbpp = 0
          inc   HL
          ld    (HL),A
?lput5:   ld    HL,0            ; return(NO);
?lput6:   ld    A,H
          or    L
          call  ?exs            ; alte Stackframezeiger einsetzen, lokalen
          defw  4               ; Speicher zurueckgeben
#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  "level 1 get"                                             *
*                                                                          *
*                Sendeframebuffer verwalten. Je nach action Zeichen aus    *
*                aktivem Sendeframe holen, Sendeframe rewind-en, oder      *
*                alle zu sendenden Frames ohne Sendung in Gesendet-Liste   *
*                umhaengen.                                                *
*                Sind alle Zeichen aus einem Frame gelesen, wird es        *
*                in die Gesendet-Liste umgehaengt.                         *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  action  -                                                 *
*                                                                          *
*                  7  6543210 7654321  0                                   *
*                +---+-------+-------+---+  falls ein Framebuffer TX-aktiv *
*                | 1 |  port |       | 0 |  ist, diesen rewind-en, kein    *
*                +---+-------+-------+---+  return-Wert                    *
*                                                                          *
*                +---+-------+-------+---+  aktiven Framebuffer und alle   *
*                | 1 |  port |       | 1 |  zu sendenden                   *
*                +---+-------+-------+---+  Frames in Gesendet-Liste       *
*                                           umhaengen, kein return-Wert    *
*                                                                          *
*                +---+-------+-------+---+  Zeichen aus aktivem Frame      *
*                | 0 |  port |       | 0 |  holen, dann return-Wert (s.u.) *
*                +---+-------+-------+---+                                 *
*                                                                          *
*                                           Frame aktiv = haengt an txfr   *
*                                                                          *
*                                           port gibt den verwendeten      *
*                                           Sendekanal an (HDLC/RS232)     *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  txfhdr  - Zeiger auf den Kopf des aktiven TX-Frames,      *
*                          0, wenn kein TX-Frame aktiv                     *
*                txl2fl  - Liste der zu sendenden (DWAIT) Level-2-Frames   *
*                stfl    - Liste der gesendeten Frames                     *
*                                                                          *
* locals      :  s.u                                                       *
*                                                                          *
* returns     :  0x8001 - aktives Frame zuende, kein weiteren vorhanden    *
*                0x8000 - aktives Frame zuende, noch weitere vorhanden     *
*                0x00cc - naechstes Zeichen aus aktivem Frame              *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

unsigned short l1get(action)

unsigned short action;

  {
    unsigned short    port;         /* port aus action                          */
    MBHEAD    **txfbpp;       /* Zeiger auf Zeiger auf aktuelles TX-Frame */
    LHEAD      *l2flp;        /* Zeiger auf DWAIT-Framelistenkopf         */
    LNKBLK     *lnktmp;       /* Zeiger auf Link-Block [DL4YBG]           */

    txfbpp = &txfhd[port = (action>>8) & 0x7F]; /* Adresse TX-Framezeiger */
    l2flp = &txl2fl[port];                      /* Sendeframelistenkopf   */
    if (!(action & 0x8000))                     /* Zeichen oder Befehl ?  */
      {                                         /* Zeichen :              */
        if (!*txfbpp) {                          /* wenn TX-Frame inaktiv  */
          if (l2flp->head != l2flp)             /* aus der TX-Frameliste  */
            tfunlink(*txfbpp = (MBHEAD *)l2flp->head);
                                                /* neues TX-Frame holen   */
          else                                  /* sonst "keine weiteren  */
            return (0x8001);                    /* Frames zu senden"      */
        }
        if ((*txfbpp)->mbgc < (*txfbpp)->mbpc)  /* wenn noch Zeichen im   */
          return(getchr(*txfbpp) & 0xFF);       /* TX-Frame, eins holen   */
        else
          {
            /* [DL4YBG] */
            if (((*txfbpp)->l2fflg & L2FT1ST) != NO) /* wenn T1 zu starten*/
              {
                lnktmp = (*txfbpp)->l2link;  /* T1-Timer starten          */
                lnktmp->T1 = lnktmp->IT1;    /* setT1 kann nicht benutzt  */
                lnktmp->T3 = 0;              /* wird                      */
              }
            /* ende */

            if (((*txfbpp)->l2fflg & L2FRTTST))
            {
              lnktmp = (*txfbpp)->l2link;
              lnktmp->RTT = 1;
            }
                                             /* sonst TX-Frame in         */
            relink(*txfbpp,stfl.tail);       /* Gesendet-Liste,           */
            *txfbpp = NULL;                  /* TX-Frame "inaktivieren",  */
            if (l2flp->head != l2flp)        /* Status "noch Frames da /  */
              return (0x8000);               /* nicht da" ermitteln und   */
            return (0x8001);                 /* zurueckgeben              */
          }
      }
    else                                     /* Befehl :                  */
      if (!(action & 0x0001))
        {                                    /* wenn TX-Frame aktiv, dies */
          if (*txfbpp) rwndmb(*txfbpp);      /* rewind-en                 */
        }
      else                                   /* alles in Gesendet-Liste   */
        {
          if (*txfbpp)                       /*   wenn TX-Frame aktiv     */
            {
              /* [DL4YBG] */
              if (((*txfbpp)->l2fflg & L2FT1ST) != NO) /* wenn T1 zu starten*/
                {
                  lnktmp = (*txfbpp)->l2link; /* T1-Timer starten         */
                  lnktmp->T1 = lnktmp->IT1;  /* setT1 kann nicht benutzt  */
                  lnktmp->T3 = 0;            /* werden                    */
                }
              /* ende */

              if (((*txfbpp)->l2fflg & L2FRTTST))
              {
                lnktmp = (*txfbpp)->l2link;
                lnktmp->RTT = 1;
              }
                                             /*   dies in Gesendet-Liste  */
              relink(*txfbpp,stfl.tail);     /*   umhaengen und           */
              *txfbpp = NULL;                /*   TX-Frame "inaktivieren" */
            }
          rlmlsl(l2flp);                     /*   def: alles gesendet !   */
        }
   }

#else

/* #asm
          public l1get?         ; globales Symbol

l1get?:   call  ?ens            ; IX/BC Stackframezeiger (Funktionsparameter)
          defw  -6              ; und 6 Byte lokaler Variablenspeicher
          ld    A,(IX+11)       ; A = MSB(action) & 0x7F
          and   127             ; (0x7F)
          ld    (IX+0),A        ; port = A
          add   A,A             ; txfbpp = HL = &txfhd[port];
          ld    E,A
          ld    D,0
          ld    HL,txfhd?
          add   HL,DE
          ld    (IX+2),L
          ld    (IX+3),H
          ld    HL,txl2fl?      ; l2flp = &txl2fl[port]
          add   HL,DE
          add   HL,DE
          ld    (IX+4),L
          ld    (IX+5),H
          bit   7,(IX+11)       ; Befehl oder Zeichen ?
          jp    NZ,?lget5       ; Befehl ...

          ld    L,(IX+2)        ; Zeichen, HL = txfbpp
          ld    H,(IX+3)
          ld    E,(HL)          ; DE = *txfbpp = txfhd[port]
          inc   HL
          ld    D,(HL)
          ld    A,D             ; Sendeframe aktiv, (*txfbpp != 0) ?
          or    E
          jr    NZ,?lget2       ; ja   -
          push  HL              ; nein - (txfbpp merken)
          ld    L,(IX+4)        ;        (HL = l2flp)
          ld    H,(IX+5)        ;        (DE = l2flp->head)
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          dec   HL              ;        (Korrektur)
          or    A               ;        l2flp->head == l2flp, d.h.
          sbc   HL,DE           ;        TX-Frameliste leer ?
          jr    NZ,?lget1
          pop   HL              ;        ja   - Stack korrigieren
          ld    HL,32769        ;               return(0x8001);
          jp    ?lget8
?lget1:   pop   HL              ;        nein - *txfbpp = DE
          ld    (HL),D          ;               (l2flp->head/diflp->head)
          dec   HL
          ld    (HL),E
          push  DE
          call  unlink?         ;               unlink(*txfbpp);
          pop   DE              ;               (DE = *txfbpp)

?lget2:   push  DE              ; (DE = *txfbpp)
          push  DE
          ld    HL,12           ; (push (*txfbpp)->mbgc)
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          pop   HL              ; (HL = *txfbpp)
          push  DE
          ld    DE,10           ; (DE = (*txfbpp)->mbpc)
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          pop   HL
          or    A               ; if ((*txfbpp)->mbgc < (*txfbpp)->mbpc)
          sbc   HL,DE           ;   return(getchr(*txfbpp) & 0xFF);
          pop   DE              ; (DE = *txfbpp)
          jr    NC,?lget3
          push  DE
          call  getchr?
          pop   DE
          ld    H,0
          jp    ?lget8

?lget3:
          call  ?T1test         ; [DL4YBG] T1 starten, wenn gewuenscht
          ld    HL,(stfl?+2)    ; else
          push  HL              ;   {
          push  DE              ;     relink(*txfbpp,stfl.tail);
          call  relink?
          pop   DE
          pop   DE
          ld    L,(IX+2)        ;     *txfbpp = NULL;
          ld    H,(IX+3)
          xor   A
          ld    (HL),A
          inc   HL
          ld    (HL),A
          ld    L,(IX+4)        ;     if (l2flp->head != l2flp)
          ld    H,(IX+5)        ;       return (0x8000)
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          dec   HL
          or    A
          sbc   HL,DE
          ld    HL,0
          jr    NZ,?lget4
          inc   HL              ;     return (0x8001);
?lget4:   set   7,H             ;   }
          jr    ?lget8

?lget5:   bit   0,(IX+10)       ; Befehl, alles in Gesendet-Liste umhaengen ?
          jr    NZ,?lget6
          ld    L,(IX+2)        ; nein - if (*txfbpp) rwndmb(*txfbpp);
          ld    H,(IX+3)
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          ld    A,D
          or    E
          jr    Z,?lget8
          push  DE
          call  rwndmb?
          pop   DE
          jr    ?lget8
?lget6:   ld    L,(IX+2)        ; ja - if (*txfbpp)
          ld    H,(IX+3)
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          ld    A,D
          or    E
          jr    Z,?lget7
          push  HL              ;        {
          call  ?T1test         ; [DL4YBG] T1 starten, wenn gewuenscht
          ld    HL,(stfl?+2)    ;          relink(*txfbpp,stfl.tail);
          push  HL
          push  DE
          call  relink?
          pop   DE
          pop   DE
          pop   HL              ;          *txfbpp = NULL;
          xor   A
          ld    (HL),A
          dec   HL
          ld    (HL),A          ;        }
?lget7:   ld    L,(IX+4)        ;      rlmlsl(l2flp);
          ld    H,(IX+5)
          push  HL
          call  rlmlsl?
          pop   DE
?lget8:   ld    A,H
          or    L
          call  ?exs            ; alte Stackframezeiger einsetzen, lokalen
          defw  6               ; Speicher zurueckgeben
; [DL4YBG] [DG2FEF]
?T1test:
          ld    HL,17           ; l2fflg
          add   HL,DE
          ld    A,(HL)          ; wert holen
          and   01h             ; L2FT1ST gesetzt?
          jr    Z,?setrtt       ; nein, fertig [FEF]
          push  DE
          ld    HL,14           ; l2link
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          ld    HL,91           ; von IT1
          add   HL,DE           ; nach T1 kopieren
          ld    A,(HL)
          inc   HL
          inc   HL
          ld    (HL),A
          dec   HL
          ld    A,(hl)
          inc   HL
          inc   HL
          ld    (HL),A
          ld    HL,97           ; T3 auf 0 setzen
          add   HL,DE
          xor   A
          ld    (HL),A
          inc   HL
          ld    (HL),A
          pop   DE
;
; Ab hier: [FEF] Das gleiche Spiel fuer due RTT-Messung
;
?setrtt:  ld    HL,17
          add   HL,DE
          ld    A,(HL)
          and   04h
          ret   z

          push  DE
          ld    HL,14
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          ld    HL,87
          add   HL,DE
          ld    (HL),1
          inc   HL
          ld    (HL),0
          pop   DE
          ret
; ende
#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  "relink message list to sent list"                        *
*                                                                          *
*                Frameliste in Gesendete-Frames-Liste umhaengen.           *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  mlp     - in stfl umzuhaengende Frameliste                *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  stfl    - Liste der schon gesendeten Frames               *
*                                                                          *
* locals      :  -                                                         *
*                                                                          *
* returns     :  -                                                         *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

void rlmlsl(mlp)

LEHEAD *mlp;

  {
    while (mlp->nextle != mlp) relink(tfunlink(mlp->nextle),stfl.tail);
  }

#else

/* #asm
          public rlmlsl?        ; globales Symbol

rlmlsl?:  call  ?en             ; IX setzen (Funktionsparameterzeiger)
?rlml1:   ld    L,(IX+4)        ; HL = mlp
          ld    H,(IX+5)
          ld    E,(HL)          ; DE = mlp->nextle
          inc   HL
          ld    D,(HL)
          dec   HL              ; (HL korrigieren)
          or    A
          sbc   HL,DE           ; mlp->nextle != mlp ?
          jr    Z,?rlml2        ; nein - dann war's das
          ld    HL,(stfl?+2)    ; ja   - relink(unlink(DE),stfl.tail);
          push  HL
          push  DE
          call  unlink?
          pop   DE
          push  HL
          call  relink?
          pop   DE
          pop   DE
          jr    ?rlml1          ; while-Schleife
?rlml2:   pop   BC              ; Stack korrigieren
          push  BC              ; Stackframepointer auf Wert vor
          pop   IX              ; Aufruf
          ret                   ; das war's

#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  "put character"                                           *
*                                                                          *
*                Zeichen in Messagebuffer schreiben, Put-Counter erhoehen  *
*                und Buffer-Pointer setzen. Ist der aktuelle Datenbuffer   *
*                im Messagebuffer voll, dann neuen Datenbuffer allokieren  *
*                und ans Datenbufferlistenende des Messagebuffers          *
*                anhaengen.                                                *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  ch      - in den Buffer zu schreibendes Zeichen           *
*                mbhd    - Zeiger auf den Messagebuffer-Kopf, in den ch    *
*                          zu schreiben ist                                *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  -                                                         *
*                                                                          *
* locals      :  -                                                         *
*                                                                          *
* returns     :  -                                                         *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

void putchr(ch,mbhd)       /* FEF */

char      ch;
MBHEAD   *mbhd;

  {
    if (!(mbhd->mbpc++ % 64))           /* neuer Datenbuffer faellig ?    */
      mbhd->mbbp = ((MB *)(relink(allocb(),mbhd->mbl.tail)))->data; /* ja */
    *mbhd->mbbp++ = ch;                 /* Zeichen in Buffer schreiben    */
  }


#else

/* #asm
          public putchr?        ; globales Symbol

putchr?:  call  ?en             ; IX/BC Stackframezeiger (Funktionsparameter)
          ld    L,(IX+6)        ; HL = mbhd
          ld    H,(IX+7)
          push  HL              ; (merken)
          ld    DE,10           ; ->mbpc
          add   HL,DE
          ld    E,(HL)          ; DE = mbhd->mbpc++
          inc   HL
          ld    D,(HL)
          inc   DE
          ld    (HL),D
          dec   HL
          ld    (HL),E
          dec   DE              ; (DE wieder korrigieren)
          pop   HL              ; HL = mbhd
          ld    A,E
          and   63              ; DE % 64 = 0, d.h. neuer Buffer erforderlich ?
          jr    NZ,?put1        ; nein -
          push  HL              ; ja   -
          push  HL
          ld    DE,8            ;        (SP) = mbhd->mbbp
          add   HL,DE
          ex    (SP),HL
          ld    DE,6            ;        DE = mbhd->mbl.tail
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          push  DE
          call  allocb?         ;        HL = Freibuffer (Adresse)
          push  HL
          call  relink?         ;        relink(Freibuffer,mbhd->mbl.tail)
          pop   DE
          pop   DE
          ld    DE,4            ;        HL = Freibuffer->data
          add   HL,DE
          ex    DE,HL           ;        DE = Freibuffer->data
          pop   HL              ;        HL = mbhd->mbbp (s.o. (SP) = ... )
          ld    (HL),E          ;        mbhd->mbbp = DE
          inc   HL
          ld    (HL),D
          pop   HL              ;        HL = mbhd

?put1:    ld    DE,8            ; *mbhd->mbbp++ = ch
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          inc   DE              ; (++mbhd->mbbp)
          ld    (HL),D
          dec   HL
          ld    (HL),E
          dec   DE
          ld    A,(IX+4)        ; (A = ch)
          ld    (DE),A
          pop   BC              ; alte Stackframezeiger wiedereinsetzen
          push  BC
          pop   IX
          ret                   ; das war's
#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  "get character"                                           *
*                                                                          *
*                Zeichen aus einem Messagebuffer holen. Datenbuffer-Poiner *
*                setzen und Get-Count erhoehen. Uebergang in der           *
*                Datenbufferliste vom Ende eines Datenbuffers zum          *
*                naechsten ausfuehren.                                     *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  mbhd    - Zeiger auf Kopf des Messagebuffers, aus dem     *
*                          das Zeichen gelesen werden soll                 *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  -                                                         *
*                                                                          *
* locals      :  -                                                         *
*                                                                          *
* returns     :  aus dem Buffer gelesenes Zeichen                          *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

short getchr(mbhd)

MBHEAD *mbhd;

  {
    if (!(mbhd->mbgc++ % 64))                            /* Ende Buffer ? */
      mbhd->mbbp = ((MB *)(mbhd->mbbp)-1)->nextmb->data; /* ja, naechster */
    return (*mbhd->mbbp++);                              /* Zeichen holen */
  }

#else

/* #asm
          public getchr?        ; globales Symbol

getchr?:  call  ?en             ; IX/BC Stackframezeiger (Funktionsparameter)
          ld    L,(IX+4)        ; HL = mbhd
          ld    H,(IX+5)
          push  HL              ; (merken)
          ld    DE,12           ; A = mbhd->mbgc++ % 64
          add   HL,DE
          ld    E,(HL)          ; (DE = mbhd->mbgc)
          inc   HL
          ld    D,(HL)
          inc   DE              ; (++mbhd->mbgc)
          ld    (HL),D
          dec   HL
          ld    (HL),E
          dec   DE              ; (DE wieder korrigieren)
          pop   HL              ; (HL = mbhd)
          ld    A,E             ; A = (D)E % 64 [DG2FEF]
          and   63              ; noch Zeichen im aktuellen Datenbuffer ?
          jr    NZ,?get1        ; ja   -
          push  HL              ; nein - mbhd sichern
          ld    DE,8            ;        DE = mbhd->mbbp
          add   HL,DE
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          push  HL              ;        (&mbhd->mbbp + 1 sichern)
          ld    HL,-68          ;        HL = mbhd->mbbp - 68 [DG2FEF]
          add   HL,DE           ;           = Zeiger auf den Datenbufferkopf
          ld    E,(HL)          ;        DE = Datenbufferkopf.next
          inc   HL
          ld    D,(HL)
          ld    HL,4            ;             + 4
          add   HL,DE           ;        = Zeiger auf Anfang naechste Daten
          ex    DE,HL           ;        in DE
          pop   HL              ;        HL = &mbhd->mbbp + 1
          ld    (HL),D          ;        (HL-1) = DE, also mbhd->mbbp =
          dec   HL              ;        Anfang des naechsten Datenbuffers
          ld    (HL),E
          pop   HL              ;        HL = mbhd

?get1:    ld    DE,8            ; HL = *mbhd->mbbp++
          add   HL,DE           ; (DE = mbhd->mbbp)
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          inc   DE              ; (++mbhd->mbbp)
          ld    (HL),D
          dec   HL
          ld    (HL),E
          dec   DE              ; (DE wieder korrigieren)
          ld    A,(DE)          ; (A = *DE)
          ld    L,A             ; (HL = A)
          ld    H,0
          or    A               ; (C-Konvention)
          pop   BC              ; alte Stackframezeiger wiedereinsetzen
          push  BC
          pop   IX
          ret                   ; das war's
#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  "rewind message buffer"                                   *
*                                                                          *
*                Message-Buffer (Kopf und Datenbufferliste) zuruecksetzen. *
*                Get-Counter auf 0 setzen, Buffer-Pointer so setzen, dass  *
*                beim naechsten getchr() auf das das erste Datenbyte des   *
*                ersten Datenbuffers zugegriffen wird.                     *
*                (Dies muss wie folgt geschehen, da der Get-Counter in     *
*                jedem Fall auf 0 stehen muss und in getchr() auf % 32     *
*                fuer das Positionieren auf den naechsten Datenbuffer      *
*                abgetestet wird.)                                         *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  mbhd    - Zeiger auf Kopf des Message-Buffers             *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  -                                                         *
*                                                                          *
* locals      :  -                                                         *
*                                                                          *
* returns     :  -                                                         *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

void rwndmb(mbhd)

MBHEAD *mbhd;

  {
    /* Kopf wie Datenbufferende */
    mbhd->mbbp = (char *)(&((MBHEAD *)mbhd + 1)->mbl);
    mbhd->mbgc = 0;                                    /* Bufferanfang    */
  }

#else

/* #asm
          public rwndmb?        ; globales Symbol

rwndmb?:  call  ?en             ; IX/BC Stackframezeiger (Funktionsparameter)
          ld    E,(IX+4)        ; DE = mbhd
          ld    D,(IX+5)
          push  DE              ; (fuer spaeter merken)
          ld    HL,72           ; HL = &((MB *)mbhd + 1)->mbl [DG2FEF]
          add   HL,DE
          ex    DE,HL           ; DE = HL, HL = mbhd
          ld    BC,8            ; mbhd->mbgc = DE
          add   HL,BC
          ld    (HL),E
          inc   HL
          ld    (HL),D
          pop   HL              ; HL = mbhd
          ld    DE,12           ; mbhd->mbgc = 0
          add   HL,DE
          xor   A
          ld    (HL),A
          inc   HL
          ld    (HL),A
          pop   BC              ; alte Stackframezeiger wiedereinsetzen
          push  BC
          pop   IX
          ret                   ; das war's
#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  "allocate buffer"                                         *
*                                                                          *
*                Leeren Buffer aus der Freiliste holen, Pogrammneustart    *
*                wenn keine Buffer mehr in Freiliste.                      *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  -                                                         *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  nmbfre  - Anzahl der Buffer in Freiliste freel            *
*                freel   - verkettete Liste der freien Buffer              *
*                                                                          *
* locals      :  s.u.                                                      *
*                                                                          *
* returns     :  Zeiger auf freien Buffer, fall vorhanden                  *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

LEHEAD *allocb()
  {
    LEHEAD *ret;                        /* Buffer Rueckgabewert           */

    DIinc();                            /* keine Ints. > Listenkonsistenz */
    if (!nmbfre--) reset();             /* Freibufferz., ggf. Neustart    */
    ret = tfunlink(freel.head);           /* Buffer aus Liste aushaengen    */
    decEI();                            /* Interrupts nun wieder erlaubt  */
    return(ret);                        /* Zeiger auf Freibuffer zurueck  */
  }

#else

/* #asm
          public allocb?        ; globales Symbol

allocb?:  call  DIinc?          ; Interrupts verbieten (Listenkonsistenz)
          ld    HL,(nmbfre?)    ; nmbfre--
          dec   HL
          ld    (nmbfre?),HL
          inc   HL
          ld    A,H             ; noch freie Buffer da ?
          or    L
          jr    NZ,?all1        ; ja   -
          call  reset?          ; nein - Programmneustart (Crash)
?all1:    ld    HL,(freel?)     ; unlink(freel.head)
          push  HL
          call  unlink?
          call  decEI?          ; Interrupts wieder erlauben
          pop   HL              ; Zeiger auf Buffer zurueck
          ld    A,H             ; (C-Konvention)
          or    L
          ret                   ; das war's

#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  Element aus Liste aushaengen.                             *
*                                                                          *
*                                                                          *
*                      le ---+                                             *
*              vor           |     raus                hinter              *
*            +--------+      +-->+--------+          +--------+            *
*      ----->|        |--------->| prevle |--------->|        |----->      *
*            +--------+          +--------+          +--------+            *
*      <-----|        |<---------| nextle |<---------|        |<-----      *
*            +--------+          +--------+          +--------+            *
*            |        |          |        |          |        |            *
*                                                                          *
*                                                                          *
*                                \/                                        *
*                                                                          *
*                                                                          *
*             raus                    vor             hinter               *
*           +--------+              +--------+      +--------+             *
*    le --->| nextle |        ----->|        |----->|        |----->       *
*           +--------+              +--------+      +--------+             *
*           | prevle |        <-----|        |<-----|        |<-----       *
*           +--------+              +--------+      +--------+             *
*           |        |              |        |      |        |             *
*                                                                          *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  le      - Zeiger auf auszuhaengendes Listenelement        *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  -                                                         *
*                                                                          *
* locals      :  -                                                         *
*                                                                          *
* returns     :  le                                                        *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

LEHEAD *tfunlink(le)

LEHEAD *le;

  {
    DIinc();                            /* keine Ints. > Listenkonsistenz */
    le->prevle->nextle = le->nextle;    /* Hinliste ohne le               */
    le->nextle->prevle = le->prevle;    /* Rueckliste ohne le             */
    decEI();                            /* Interrupts wieder erlaubt      */
    return(le);                         /* Zeiger auf das Element zurueck */
  }

#else

/* #asm
          public unlink?        ; globales Symbol

unlink?:  call  ?en             ; IX/BC Stackframezeiger (Funktionsparameter)
          call  DIinc?          ; Interrupts verbieten (Listenkonsistenz !)
          ld    L,(IX+4)        ;   HL -> | raus |
          ld    H,(IX+5)
          push  HL              ;   le fuer return() merken
          ld    C,(HL)          ;   BC -> | hinter |
          inc   HL
          ld    B,(HL)
          inc   HL              ;   DE -> | vor |
          ld    E,(HL)
          inc   HL
          ld    D,(HL)
          dec   HL              ;   ( wieder HL -> | raus | )
          dec   HL
          dec   HL
          ex    DE,HL           ;   HL -> | vor |, DE -> | hinter|
          ld    (HL),C          ; * vor.nextle -> | hinter |
          inc   HL
          ld    (HL),B
          dec   HL
          inc   BC              ;   BC -> | hinter.prevle |
          inc   BC
          ld    A,L             ; * hinter.prevle -> | vor |
          ld    (BC),A
          inc   BC
          ld    A,H
          ld    (BC),A
          call  decEI?          ;   Interrupts wieder erlauben
          pop   HL              ;   le holen ...
          ld    A,H             ;   ... und zurueckgeben
          or    L
          pop   BC              ;   alte Stackframezeiger wiedereinsetzen
          push  BC
          pop   IX
          ret                   ;   das war's
#endasm */

#endif





/**************************************************************************\
*                                                                          *
* action      :  Element in Liste einhaengen.                              *
*                                                                          *
*                                                                          *
*             neu                     vor             hinter               *
*           +--------+              +--------+      +--------+             *
*   new --->| nextle |     pred --->|        |----->|        |----->       *
*           +--------+              +--------+      +--------+             *
*           | prevle |        <-----|        |<-----|        |<-----       *
*           +--------+              +--------+      +--------+             *
*           |        |              |        |      |        |             *
*                                                                          *
*                                                                          *
*                                \/                                        *
*                                                                          *
*                                                                          *
*                     new ---+                                             *
*              vor           |     neu                 hinter              *
*            +--------+      +-->+--------+          +--------+            *
*   pred --->|        |--------->| prevle |--------->|        |----->      *
*            +--------+          +--------+          +--------+            *
*      <-----|        |<---------| nextle |<---------|        |<-----      *
*            +--------+          +--------+          +--------+            *
*            |        |          |        |          |        |            *
*                                                                          *
*                                                                          *
 **************************************************************************
*                                                                          *
* parameter   :  new     - Zeiger auf einzuhaengendes Listenelement        *
*                pred    - Zeiger auf Listenelement, hinter dem new        *
*                          eingehaengt werden soll                         *
*                                                                          *
* r/o globals :  -                                                         *
*                                                                          *
* r/w globals :  -                                                         *
*                                                                          *
* locals      :  -                                                         *
*                                                                          *
* returns     :  new                                                       *
*                                                                          *
\**************************************************************************/

#ifdef PORTABLE

LEHEAD *relink(new,pred)

LEHEAD *new;
LEHEAD *pred;

  {
    DIinc();                            /* keine Ints. > Listenkonsistenz */
    new->nextle = pred->nextle;         /* Vorzeiger im neuen Element     */
    new->prevle = pred;                 /* Rueckzeiger im neuen Element   */
    new->nextle->prevle = new;          /* Rueckzeiger dahinter           */
    pred->nextle = new;                 /* Vorzeiger davor                */
    decEI();                            /* Interrupts wieder erlaubt      */
    return(new);                        /* Zeiger auf neues Element       */
  }

#else

/* #asm
          public relink?        ; globales Symbol

relink?:  call  ?en             ; IX/BC Stackframezeiger (Funktionsparameter)
          call  DIinc?          ; Interrupts verbieten (Listenkonsistenz !)
          ld    L,(IX+6)        ;   HL -> | vor |
          ld    H,(IX+7)
          ld    E,(IX+4)        ;   DE -> | neu |
          ld    D,(IX+5)
          push  DE              ;   new fuer return() merken
          ld    C,(HL)          ;   BC -> | hinter |
          inc   HL
          ld    B,(HL)
          ld    (HL),D          ; * vor.nextle -> | neu |
          dec   HL
          ld    (HL),E
          ex    DE,HL           ;   HL -> | neu |, DE -> | vor |
          ld    (HL),C          ; * neu.nextle -> | hinter |
          inc   HL
          ld    (HL),B
          dec   HL
          inc   BC              ;   BC -> | hinter.prevle |
          inc   BC
          ld    A,L             ; * hinter.prevle -> | neu |
          ld    (BC),A
          inc   BC
          ld    A,H
          ld    (BC),A
          inc   HL              ;   HL -> | neu.prevle |
          inc   HL
          ld    (HL),E          ; * neu.prevle -> | vor |
          inc   HL
          ld    (HL),D
          call  decEI?          ;   Interrupts wieder erlauben
          pop   HL              ;   new holen ...
          ld    A,H             ;   ... und zurueckgeben
          or    L
          pop   BC              ;   alte Stackframezeiger wiedereinsetzen
          push  BC
          pop   IX
          ret                   ;   das war's
#endasm */

#endif



/* Ende von L2D.C */
