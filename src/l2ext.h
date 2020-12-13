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
*    L2EXT.H   -   Externals fuer Level 2                                  *
*                  (globale Variable und nicht-int-Funktionen)             *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*                                                                          *
*    modifiziert:   DL8ZAW, versch. Referenzen auf neue Variablen          *
*                   Erlaeuterung dazu: siehe C-Module                      *
*                                                                          *
*                   DL8ZAW, 27.03.92                                       *
*                   TheNet spezifischer Code entfernt                      *
*                                                                          *
*                   DB2OS,  23.06.92                                       *
*                   Parameter Walt entfernt wegen DCD-Problemen.           *
*                                                                          *
*                   DB2OS,  19.08.92                                       *
*                   A1par, A2par, ininat, MHEARD entfernt.                 *
*                                                                          *
*                   DL4YBG, 13.02.95                                       *
*                   takflx(),rxfflx fuer FlexNet-Monitor.                  *
*                                                                          *
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
#ifdef USE_FLEXDEC
extern BOOLEAN     takflx();
#endif
extern BOOLEAN     getfid();
extern MBHEAD     *makfhd();
extern BOOLEAN     istome();
extern BOOLEAN     cmpcal();
extern BOOLEAN     cmpidl();
extern BOOLEAN     cmpid();
extern BOOLEAN     l1put();
extern unsigned short    l1get();
extern LEHEAD     *allocb();
extern LEHEAD     *unlink();
extern LEHEAD     *relink();

extern short      getchr();


extern BOOLEAN     fmlink();                      /* L3... / TF...        */



extern unsigned short    nbrprt();                      /* L7... / TF...        */
extern BOOLEAN     iswarm();





/*                                                             Variable   */
/**************************************************************************/



extern char        myid[];                        /* L2...                */
extern char        nmblks;
extern char        aktlks; /* FEF */
extern char        Ypar;
extern char        damaok;
extern char        sendok;
extern char        tosend;
extern char        rxfhdr[];
extern char        rxfctl;
extern char        rxfPF;
extern char        rxfV2;
extern unsigned short    damati;
extern char        rxfCR;
extern char        rxfprt;
#ifdef USE_FLEXDEC
extern char        rxfflx;
#endif
extern char        txfhdr[];
extern char        txfctl;
extern char        txfPF;
extern char        txfV2;
extern char        txfCR;
extern char        txfprt;
extern char        Wpar;
extern char        Ppar;
extern char        UIpar;
extern unsigned short    Opar;
extern unsigned short    Npar;
extern unsigned short    T2par;
extern unsigned short    T3par;
extern unsigned short    Rpar;
extern unsigned short    T3par;
extern unsigned short    Fpar;
extern unsigned short    Ipar;
extern unsigned short    A3par;
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

extern unsigned short    conctl;


extern unsigned short    VCpar;                         /* L7... / TF...        */

extern LHEAD       monfl;


/* Ende von L2EXT.H */
