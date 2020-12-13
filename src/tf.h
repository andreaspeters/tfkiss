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
*    TF.H   -   Headerfile fuer The Firmware                               *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:                                                          *
*                                                                          *
\**************************************************************************/





/*                                                         Festlegungen   */
/**************************************************************************/

#define UTXTLEN     72        /* Anzahl Zeichen maximal im U-Text         */
#define HBUFLEN     256       /* Anzahl Zeichen maximal Hosteingabebuffer */



                              /* "host mode response",                    */
                              /* Hostmodeframe-Antworttypen :             */
#define HMRSUCC     0         /*   "success"                              */
#define HMRSMSG     1         /*   "success message"                      */
#define HMRFMSG     2         /*   "failure message"                      */
#define HMRSTAT     3         /*   "status"                               */
#define HMRMONH     4         /*   "monitor header"                       */
#define HMRMONIH    5         /*   "monitor I frame header"               */
#define HMRMONI     6         /*   "monitor I frame"                      */
#define HMRCONI     7         /*   "connected I frame"                    */



                              /* "host mode state",                       */
                              /* Hostmode-Eingabezustaende :              */
#define HMSCHNL     0         /*   naechstes Eingabebyte ist Kanal        */
#define HMSCMD      1         /*   naechstes Eingabebyte ist Kommando     */
#define HMSLEN      2         /*   naechstes Eingabebyte ist Laenge       */
#define HMSINPUT    3         /*   naechste Eingabebytes sind Eingabetext */



                              /* Zpar-Flags :                             */
#define FZFLOW      0x01      /*   Flow ein                               */
#define FZXONOFF    0x02      /*   XON/XOFF ein                           */



                              /* ASCII-Kontrollzeichen :                  */
#define BELL        0x07      /*   Klingel                                */
#define BS          0x08      /*   Backspace                              */
#define TAB         0x09      /*   Tab                                    */
#define LF          0x0A      /*   Linefeed                               */
#define CR          0x0D      /*   Carriage Return                        */
#define CONTROLR    0x12      /*   DC2                                    */
#define CONTROLU    0x15      /*   NAK                                    */
#define CONTROLX    0x18      /*   CAN                                    */
#define DEL         0x7F      /*   Delete                                 */



                              /* Messagebuffer-Anwahl :                   */
#define MBINFO      0         /*   alle Info-Pakete                       */
#define MBSTATUS    1         /*   alle Statuspakete                      */
#define MBALL       2         /*   alle Pakete                            */



                              /* Flags Mpar :                             */
#define MONI        0x01      /*   I                                      */
#define MONU        0x02      /*   U                                      */
#define MONS        0x04      /*   S                                      */
#define MONC        0x08      /*   C                                      */



/* Ende von TF.H */
