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
*    L2.H   -   Headerfile Level 2                                         *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 27.04.91                                       *
*                   L2CDAMA = DAMA-Bit in SSID (0=DAMA-Master)             *
*                   LNKBLK: RTT, IRTT, SRTT fuer RTT-Messung zugefuegt     *
*                                                                          *
*                   DL8ZAW, 07.05.91                                       *
*                   L2FDPOLL = Im DAMA-Modus ersten Poll uebergehen        *
*                              (siehe L2A.C)                               *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   TheNet spezifischer Code entfernt                      *
*                                                                          *
*                   DL4YBG, 23.10.93                                       *
*                   LNKBLK: IT1 hinzugefuegt                               *
*                   RTTMAX,RTTMIN neu                                      *
*                                                                          *
*                   DG2FEF, 18.06.94                                       *
*                   L2FJSENT = 1, wenn ein Frame noch in der TX-Queue ist  *
*                              das bei DAMA wegen sendok==0 noch nicht     *
*                              rausgegangen ist                            *
*                                                                          *
*                   DG2FEF, 18.07.94                                       *
*                   L2FRETRY,L2FMAXF1: Flags fr Framesammler-             *
*                                      Untersttzung                       *
*                   L2FDIRTY: RTT-Messung ungltig                         *
*                   L2FPOLL: Link wurde unter DAMA gepollt                 *
*                   LNKBLK: oldVS, RTTNR angeh„ngt                         *
*                   L2FL3LNK: Entfernt, wird nicht ben”tigt                *
*                                                                          *
*                   DG2FEF, 13.08.94                                       *
*                   L2FDAMA ersetzt L2FRETRY                               *
*                                                                          *
*                   DB2OS, 12.08.94                                        *
*                   berflssige noatou-Varibale und MHEARD entfernt.      *
*                                                                          *
*                   DL8HBS, 19.10.94                                       *
*                   LNKBLK: txPID hinzugefuegt. TXPIDDEFAULT definiert     *
*                                                                          *
*                   DL4YBG, 06.11.94                                       *
*                   LNKBLK: rxPID hinzugefuegt. L2MPIDCH definiert         *
*                                                                          *
\**************************************************************************/





/*                                                         Festlegungen   */
/**************************************************************************/

#define L2PNUM   1                      /* TheFirmware :  1 Level-2-Port  */

#define L2CALEN  6                      /* Laenge Call im Level 2         */
#define L2IDLEN  (L2CALEN + 1)          /* Laenge Call + SSID = ID        */
#define L2INUM   2                      /* Anzahl ID's im an/von-Feld     */
#define L2VNUM   8                      /* Anzahl ID's im via-Feld        */
#define L2ILEN   (L2INUM * L2IDLEN)     /* Laenge an/von-Feld             */
#define L2VLEN   (L2VNUM * L2IDLEN)     /* Laenge via-Feld                */
#define L2AFLEN  (L2ILEN + L2VLEN)      /* Laenge Level 2 Adressfeld      */
#define L2MFLEN  328                    /* maximale Framelaenge,          */
                                        /*   10 * 7 =    70 Bytes Adresse */
                                        /*            +   1 Byte Control  */
                                        /*            +   1 Byte PID      */
                                        /*            + 256 Byte Info     */
                                        /*            -----               */
                                        /*              328               */



                              /* "layer 2 state", (state, s.u.) :         */
#define L2SDSCED    0         /*    disconnected                          */
#define L2SLKSUP    1         /*    link setup                            */
#define L2SFRREJ    2         /*    frame reject                          */
#define L2SDSCRQ    3         /*    disconnect request                    */
#define L2SIXFER    4         /*    information transfer                  */
#define L2SRS       5         /*    REJ sent                              */
#define L2SWA       6         /*    waiting acknowledge                   */
#define L2SDBS      7         /*    device busy                           */
#define L2SRBS      8         /*    remote busy                           */
#define L2SBBS      9         /*    both busy                             */
#define L2SWADBS   10         /*    waiting ack and device busy           */
#define L2SWARBS   11         /*    waiting ack and remote busy           */
#define L2SWABBS   12         /*    waiting ack and both busy             */
#define L2SRSDBS   13         /*    REJ sent and device busy              */
#define L2SRSRBS   14         /*    REJ sent and remote busy              */
#define L2SRSBBS   15         /*    REJ sent and both busy                */



                              /* "layer 2 message", Status vom Level 2 :  */
#define L2MNIX     0          /*    keine Nachricht                       */
#define L2MCONNT   1          /*    CONNECTED to                          */
#define L2MDISCF   2          /*    DISCONNECTED from                     */
#define L2MBUSYF   3          /*    BUSY from                             */
#define L2MFAILW   4          /*    LINK FAILURE with                     */
#define L2MLRESF   5          /*    LINK RESET from                       */
#define L2MLREST   6          /*    LINK RESET to                         */
#define L2MFRMRF   7          /*    FRAME REJECT from                     */
#define L2MFRMRT   8          /*    FRAME REJECT to                       */
#define L2MBUSYT   9          /*    BUSY to                               */
#ifdef USE_XPID
#define L2MPIDCH  10          /*    PID changed to                 DL4YBG */
#endif
 


                              /* "layer 2 control", Frametypen :          */
                              /*                                          */
                              /*                       Command/   Poll/   */
                              /*    Typ       Gruppe   Response   Final   */
                              /* ---------------------------------------- */
#define L2CI       0x00       /*      I         I         C         P     */
#define L2CUI      0x03       /*     UI         U        C/R       P/F    */
#define L2CSABM    0x2F       /*   SABM         U         C         P     */
#define L2CDISC    0x43       /*   DISC         U         C         P     */
#define L2CUA      0x63       /*     UA         U         R         F     */
#define L2CDM      0x0F       /*     DM         U         R         F     */
#define L2CFRMR    0x87       /*   FRMR         U         R         F     */
#define L2CRR      0x01       /*     RR         S        C/R       P/F    */
#define L2CREJ     0x09       /*    REJ         S        C/R       P/F    */
#define L2CRNR     0x05       /*    RNR         S        C/R       P/F    */

                              /* "layer 2 control", spezielle Bits :      */
#define L2CPF      0x10       /*   Poll/Final                             */
#define L2CCR      0x80       /*   Command/Response                       */
#define L2CH       0x80       /*   "has been repeated"                    */
#define L2CEOA     0x01       /*   End of Address                         */
#define L2CDAMA    0x20       /*   DAMA-Bit in SSID (0=DAMA Master)       */

                              /* "layer 2 control", Masken :              */
#define L2CNOIM    0x01       /*   "no I mask", kein I-Frame              */
#define L2CNOSM    0x02       /*   "no S mask", kein S-Frame              */
#define L2CNONRM   0x03       /*   "no N(R) mask", kein N(R)-Frame        */

                              /* "layer 2 control", Protokollidentifier   */
                              /* wird aber ersetzt durch TX-PID       HBS */
#define L2CPID     0xF0       /*   PID                                    */
#ifdef USE_XPID
#define TXPIDDEFAULT 0xF0     /* Init-Wert fAr die TX-PID             HBS */
#endif
                              /* "layer 2 control", Flags (flag, s.u.) :  */
#define L2FIRCVD   0x01       /* FEF es wurde ein I-Frame empfangen       */
#define L2FDIRTY   0x02       /* FEF Die lfd. RTT-Messung ist ungltig    */
#define L2FMAXF1   0x04       /* FEF Auf dem Link gilt Maxframe 1         */
#define L2FDAMA    0x08       /* FEF Auf dem Link stehen noch Frames aus  */
#define L2FJSENT   0x10       /* FEF Auf dem Link wurde gerade gesendet,  */
                              /*     es ist noch nichts zu retryen        */
#define L2FPOLL    0x20       /* FEF Link wurde unter DAMA gepollt        */
#define L2FBUSY    0x40       /*   Device busy (ich !)                    */
#define L2FDSLE    0x80       /*   "disc if send list empty"              */

                              /* im Framebufferkopf (l2fflag, s.u.) :     */
#define L2FT1ST    0x01       /*   nach Aussendung ist T1 zu starten      */
#define L2FUS      0x02       /*   Sendeframe ist U- oder S-Frame (nicht  */
                              /*   digipeatet)                            */
#define L2FRTTST   0x04

#define RTTMAX     4095       /* max-wert RTT 40950ms (DL4YBG)            */
#define RTTMIN     10         /* min-wert RTT 100ms (DL4YBG)              */

                              /* Anzahl Buffer pro Link nach DB7KG        */
#define MAXLBUF ((nmbfre*sizeof(MB))/(nmblks+1))

/*                                                                Typen   */
/**************************************************************************/



typedef struct lhead          /* "list head", Listenkopf :                */
  {
    struct lhead   *head;     /*   Zeiger auf ersten Eintrag in Liste     */
    struct lhead   *tail;     /*   Zeiger auf letzten Eintrag in Liste    */
  } LHEAD;



typedef struct lehead         /* "list entry head", Kopf eines Eintrags   */
  {                           /* in Liste :                               */
    struct lehead   *nextle;  /*   Zeiger auf naechsten Listeneintrag     */
    struct lehead   *prevle;  /*   Zeiger auf vorherigen Listeneintrag    */
  } LEHEAD;



typedef struct lnkblk         /* "link block", fuer jeden Level-2-Link :  */
  {
/*    char     state; */          /* Link-State, s.o. L2S...                  */
    int      state;           /* Link-State, s.o. L2S...                  */
    char     srcid[L2IDLEN];  /* "source id", eigenes Call/SSID oder      */
                              /* Ident/SSID                               */
    char     dstid[L2IDLEN];  /* "destination id", Call/SSID Gegenstation */
    char     viaidl[L2VLEN+1];/* "via id list", Digipeaterstrecke,        */
                              /* 0-terminiert, Weg zur Gegenstation       */
    char     liport;          /* "link port" - 0 : HDLC, 1 : Crosslink    */
    char     VR;              /* "receive sequence variable", Sequenz-    */
                              /* nummer des naechsten zu empfangenden     */
                              /* I-Frames                                 */
    char     VS;              /* "send sequence variable", Sequenznummer  */
                              /* des naechsten zu sendenden I-Frames      */
    char     lrxdNR;          /* "last received N(R)", zuletzt            */
                              /* empfangenes N(R) = eigene gesendete      */
                              /* I-Frames bis lrxdNR-1 bestaetigt         */
    char     ltxdNR;          /* "last transmitted N(R)", zuletzt         */
                              /* gesendetes N(R) = empfangene I-Frames    */
                              /* bis ltxdNR-1 bestaetigt                  */
    char     tries;           /* aktuelle Anzahl Versuche (RETRY),        */
                              /* hochzaehlend                             */
    char     N2;              /* RETRY, maximale Anzahl Retries           */
    char     k;               /* MAXFRAME, maximale Anzahl unbestaetigter */
                              /* I-Frames                                 */
    char     RStype;          /* "response supervisory frametype", nach   */
                              /* T2-Ablauf zu sendendes Antwortframe      */
                              /* (RR=0x01, RNR=0x05, REJ=0x09)            */
    char     frmr[3];         /* die 3 FRMR-Infobytes, Sendung u. Empfang */
                              /*   frmr[0] : zurueckgewies. Kontrollfeld  */
                              /*   frmr[1] : V(R) CR V(S) 0               */
                              /*   frmr[2] : 0000ZYXW                     */
    char     flag;            /* Flag (s.o. L2F... )                      */
                              /*   Bit 0 : nicht benutzt                  */
                              /*   Bit 1 : FEF RTT-Messung ungltig       */
                              /*   Bit 2 : FEF Maxframe 1 wird erzwungen  */
                              /*   Bit 3 : Signal, daá auf dem Link noch  */
                              /*   FEF     unbest„tigte Frames ausstehen  */
                              /*                                          */
                              /*   Bit 4 : Auf dem Link befindet sich ein */
                              /*           Frame in der TX-Queue, das aber*/
                              /*   FEF     wegen sendok==0 m”glicherweise */
                              /*           nicht gesendet wurde           */
                              /*   Bit 5 : FEF Link wurde unter DAMA ge-  */
                              /*           pollt                          */
                              /*   Bit 6 : 1 = (eigenes) Device busy      */
                              /*   Bit 7 : 1 = Link disconnecten, sobald  */
                              /*           Sendeliste (sendil) leer ist   */
    unsigned IRTT;            /* "Initial Round Trip Time" = Anfangswert  */
    unsigned RTT;             /* Round-Trip-Timer (10 ms)                 */
    unsigned SRTT;            /* Smoothed Round Trip Timer                */
                              /* SRTT = (Alpha x SRTT + RTT)/(Alpha + 1)  */
    /* [DL4YBG] */
    unsigned IT1;             /* Init-Wert fuer Timer 1 (a3*SRTT)         */
                              /* noetig, da T1-Timer jetzt aus Interrupt  */
                              /* gestartet wird (l1get) und eine Multi-   */
                              /* plikation mit a3 zuviel Zeit kostet      */
    /* ende */
    unsigned T1;              /* Timer 1, "frame acknowledge interval",   */
                              /* Start :  SRTT,                           */
                              /* 0 = inaktiv, 10 msec Downcounter         */
    unsigned T2;              /* Timer T2, "response delay timer",        */
                              /* 0 = inaktiv, 10 msec Downcounter         */
    unsigned T3;              /* Timer T3, "inactive link timer",         */
                              /* 0 = inaktiv, 10 msec Downcounter         */
    unsigned rcvd;            /* "received", Anzahl empfangener I-Frames  */
                              /* in rcvdil                                */
    unsigned tosend;          /* Anzahl noch nicht gesendete oder         */
                              /* unbestaetigte Frames in sendil           */
    LHEAD    rcvdil;          /* "received info list", richtig            */
                              /* empfangene I-Frames, mit Header/PID      */
    LHEAD    sendil;          /* "send info list", zu sendende I-Frames,  */
                              /* ohne Header/PID, nur Info                */
    char     oldVS;           /* FEF V(S) Backup fr Framesammler         */
    char     RTTNR;           /* FEF V(S), bei der RTT-Messung anlief     */

                              /* Framesammler:                            */
    char     colled;          /* "collected", Anzahl I-Frames in collil   */
    LHEAD    collil;          /* "collector info list", gesammelte I's    */
    char     lastVR;          /* V(R) nach Aufr„umen des Framesammlers    */
    char     xpctVR;          /* FEF */
                              /* n”tigt fr die Best„tigung via Statetbl. */
#ifdef USE_XPID
    char     txPID;           /* TX-PID fAr diesen Kanal              HBS */
    char     rxPID;           /* RX-PID fuer diesen Kanal          DL4YBG */
#endif
   } LNKBLK;



typedef struct timebl              /* "time block", Zeit/Datums-Stamp :   */
  {
    char             second;       /*   Zeit :    Sekunde   (0..59)       */
    char             minute;       /*             Minute    (0..59)       */
    char             hour;         /*             Stunde    (0..23)       */
    char             day;          /*   Datum :   Tag       (1..31)       */
    char             month;        /*             Monat     (1..12)       */
    char             year;         /*             Jahr      (0..99)       */
  } TIMEBL;

/* ACHTUNG: mbhead und mb nun 32 Bytes l„nger ! DG2FEF, 13.08.94 */

/* ACHTUNG: mbhead muss genauso lang sein wie mb !                        */

typedef struct mbhead         /* "message buffer head",                   */
  {                           /* Datenbuffer-Liste, Kopf :                */
    struct mbhead   *nextmh;  /*   naechster Eintrag in Liste             */
    struct mbhead   *prevmh;  /*   vorheriger Eintrag in Liste            */
    LHEAD            mbl;     /*   "message buffer list", Zeiger auf      */
                              /*   ersten Infobuffer dieser Message       */
    char            *mbbp;    /*   "message buffer buffer pointer",       */
                              /*   Zeiger auf aktuelles Zeichen in Buffer */
    unsigned         mbpc;    /*   "message buffer put count",            */
                              /*   Einschreibzaehler, aufwaertszaehlend   */
    unsigned         mbgc;    /*   "message buffer get count",            */
                              /*   Lesezaehler, aufwaertszaehlend         */
    struct lnkblk   *l2link;  /*   Zeiger auf assozierten Linkblock       */
    char             type;    /*   Typ des Buffers (User, Status)         */
                              /*          L2M...                          */
                              /*     oder MB...                           */
                              /*     oder 0 User, 2 Level-2, 4 Level-4    */
    char             l2fflg;  /*   Level 2 Frameflag / PID :              */
                              /*     RX: PID                              */
                              /*     TX: PID / s.o. T2FT1ST/T2FUS         */
    char             l2port;  /*   0 = HDLC, 1 = RS232-Crosslink          */
    char             morflg;  /*   "more follows flag" fuer Pakete, die   */
                              /*   durch zusaetzlichen Netzwerkheader zu  */
                              /*   lang wuerden und in 2 Frames           */
                              /*   aufgetrennt wurden                     */
                              /*     YES = das naechste Frame gehoert zu  */
                              /*           diesem Paket                   */
                              /*     NO  = sonst                          */
    char             rsvd[10];/*   (damit insgesamt Laenge wie mb)        */
    TIMEBL           btime;   /*   Buffer-Time fuer Zeit/Datum-Stamps     */
    char            rsvd2[24];/* FEF */

  } MBHEAD;


/* ACHTUNG: mb muss genauso lang sein wie mbhead !                        */

typedef struct mb             /* "message buffer",                        */
  {                           /* allgemeiner Datenbuffer :                */
    struct mb *nextmb;        /*   naechster Eintrag in Liste             */
    struct mb *prevmb;        /*   vorheriger Eintrag in Liste            */
    char       data[64];      /*   Daten           [DG2FEF]               */
  } MB;



typedef struct stentry        /* "state table entry",                     */
  {                           /* ein Eintrag in der State-Table :         */
    void       (* func)();    /*   Zustandsuebergangsfunktion             */
    char       newstate;      /*   neuer Zustand                          */
  } STENTRY;


/* Ende von L2.H */
