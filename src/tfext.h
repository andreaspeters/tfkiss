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
*    TFEXT.H   -   Externals fuer TheFirmware                              *
*                  (globale Variable und nicht-int-Funktionen)             *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*                                                                          *
*    modifiziert:   DL8ZAW, versch. Referenzen auf neue Variablen          *
*                   Erlaeuterung dazu: siehe C-Module                      *
*                                                                          *
*                   DB2OS, 23.06.92                                        *
*                   Parameter Walt entfernt wegen DCD-Problemen.           *
*                                                                          *
*                   DB2OS, 19.08.94                                        *
*                   A1par, A2par, ininat, MHEARD entfernt.                 *
*                                                                          *
*                   DB2OS, 21.08.94                                        *
*                   defDAp entfernt.                                       *
*                                                                          *
*                   DB2OS,  27.08.94                                       *
*                   Variablen loops und rps fr RPS-Anzeige                *
*                   alias[n], defID, defVp, defHp, defHmp entfernt.        *
*                                                                          *
*                   DL8HBS, 19.10.94                                       *
*                   UItxPID eingetragen                                    *
\**************************************************************************/





/*                                                           Funktionen   */
/**************************************************************************/



extern char       *minmem();                      /* L1...                */
extern char       *maxmem();



extern BOOLEAN     digipt();                      /* L2...                */
extern BOOLEAN     srxdNR();
extern BOOLEAN     isnxti();
extern BOOLEAN     splcpy();
extern unsigned short    setNR();
extern MBHEAD     *cpyfb();
extern BOOLEAN     takfhd();
extern BOOLEAN     getfid();
extern MBHEAD     *makfhd();
extern BOOLEAN     istome();
extern BOOLEAN     cmpcal();
extern BOOLEAN     cmpidl();
extern BOOLEAN     cmpid();
extern BOOLEAN     l1put();
extern unsigned short    l1get();
extern LEHEAD     *allocb();
extern LEHEAD     *tfunlink();
extern LEHEAD     *relink();

extern short       getchr();


extern BOOLEAN     fmlink();                      /* TF...                */
extern unsigned short    bgetp();
extern BOOLEAN     ismonf();
extern unsigned short    mbcnt();
extern MBHEAD     *selmb();
extern BOOLEAN     invial();
extern BOOLEAN     nxtnos();
extern unsigned short    nbrprt();
extern BOOLEAN     iswarm();

extern short       mainf();
extern short       fvalca();
extern short       bgetvl();
extern short       bgetid();

extern void        send_kisscmd();

/*                                                             Variable   */
/**************************************************************************/



extern char        defESC;                        /* TFL1...              */
extern char        defIp[];
extern char        defYp;
extern char        defMp;
extern char        defRp;
extern char        defPp;
extern char        defWp;
extern char        defTp;
extern char        defZp;
extern char        defXp;
extern char        defAp;
extern char        defEp;
extern char        defOp;
extern char        defNp;
extern char        defVCp;
extern char        defDp;
extern char        defUIp;
extern char        defxSp;
extern char        defxFp;
extern unsigned short    defT2p;
extern unsigned short    defT3p;
extern unsigned short    defFp;
extern char        defIPp;
extern char        defA3p;

extern char        defxTAp;

extern unsigned short    p1end;
extern unsigned short    p2strt;



extern char        myid[];                        /* L2...                */
extern char        nmblks;
extern char        aktlks;
extern char        Ypar;
extern char        rxfhdr[];
extern char        rxfctl;
extern char        rxfPF;
extern char        rxfV2;
extern char        rxfCR;
extern char        rxfprt;
extern char        txfhdr[];
extern char        txfctl;
extern char        txfPF;
extern char        txfV2;
extern char        txfCR;
extern char        txfprt;
extern unsigned short    Fpar;
extern unsigned short    Ipar;
extern unsigned short    Opar;
extern unsigned short    Npar;
extern unsigned short    T2par;
extern unsigned short    T3par;
extern unsigned short    A3par;
extern char        UIpar;
extern char        damaok;
extern unsigned short    Rpar;
extern unsigned short    damati;
extern unsigned short    nmbfre;
extern LHEAD       freel;
extern LHEAD       rxfl;
extern LHEAD       stfl;
extern LHEAD       trfl;
extern LHEAD       txl2fl[];
extern MBHEAD     *rxfhd[];
extern MBHEAD     *txfhd[];
extern LNKBLK      lnktbl[];
extern LNKBLK     *lnkpoi;



extern LHEAD       monfl;                         /* TF...                */
extern char        ch0id[];
extern char        ch0via[];
extern char        hinbuf[];
extern char       *inbufp;
extern char        Apar;
extern char        Epar;
extern char        actch;
extern char        Xpar;
extern char        Zpar;
extern char        Dpar;
extern char        Tpar;
extern char        Ppar;
extern char        Wpar;
extern char        Upar;
extern char        Utxt[];
extern char        Mpar;
extern char        mftsel;
extern char        mftidl[];
extern char        ishmod;
extern char        hmstat;
extern char        hmch;
extern char        hmcmd;
extern char        hmlen;
extern unsigned short    incnt;
extern unsigned short    isctlr;
extern unsigned short    ticks;
extern unsigned short    oldtic;
extern unsigned short    newtic;
extern unsigned short    Utcnt;
extern unsigned short    VCpar;
extern unsigned short    magicn;
extern MBHEAD     *mifmbp;
extern LHEAD       statml;
extern LHEAD       smonfl;
extern LHEAD       chnlml[];
extern unsigned short    chnlbc[];  /* FEF fr Flow-Control */

extern unsigned short    loops;     /* Hauptschleifendurchl„ufe */
extern unsigned short    rps;       /* Rounds per Second */
#ifdef USE_XPID
extern char        UItxPID;
#endif
extern unsigned short    sec100;
#ifndef TFKISS_MAIN
extern TIMEBL      tftime;
#endif
extern char        eudate;
extern char        stamp;
extern char        xFpar;
extern char        xTApar;

extern char        stmem[];
extern char        stack;
extern unsigned short    fremem;

extern char        cqid[];
extern unsigned short    conctl;
extern char       *l2msgs[];
extern void      (*cmdtab[])();

extern int         fulldup_on_dama;


extern unsigned short    end;                           /* CRUNTIME             */

/* Ende von TFEXT.H */
