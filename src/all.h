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
*    ALL.H   -   Headerfile fuer alle C-Sources                            *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DL8ZAW, 07.05.91                                       *
*                   Wegen der Aenderungen fuer DAMA-Slave-Betrieb ist es   *
*                   nicht mehr sinnvoll das geaenderte L2-Paket fuer       *
*                   TheNet zu compilieren!!!                               *
*                                                                          *
*                   DAMA-Betrieb ist zunaechst nur fuer einen HDLC-Port    *
*                   programmiert, so dass fuer mehrere HDLC-Ports noch     *
*                   Aenderungen vorgenommen werden muessen!!               *
*                                                                          *
*                   DL8ZAW, 05.06.91                                       *
*                   #define UIPOLL, wenn UI-Frames mit gesetztem POLL-Bit  *
*                   ausgesendet werden sollen (s. L2B.C: sdui() ).         *
*                                                                          *
*                   DB2OS,  17.09.91                                       *
*                   #define UIPOLL ersetzt durch @U Befehl und Variable    *
*                   UIpoll.                                                *
*                                                                          *
\**************************************************************************/

#ifndef TFKISS_MAIN
#include "config.h"
#endif

#ifndef HAVE_SOCKLEN_T
#define socklen_t int
#endif

//&&&#define unsigned unsigned short


/*                                                         Festlegungen   */
/**************************************************************************/
#define LINKNMBR   50         /* maximale Anzahl Links = Connect-Kanaele  */

#define L2BREMSE   60         /* Freie Buffer bei deren Unterschreitung   */
                              /* der TNC BUSY wird und alles wegwirft..   */
#define L2GAS      80         /* Gasgeben, wieder genug Buffer frei..     */
#define HBREMSE    40         /* Wie oben, aber fr Terminal-Seite        */
#define MBREMSE   128         /* keine Frames mehr in den Monitorbuffer   */
#define CBREMSE    64         /* Connect nur annehmen wenn genug frei..   */
#define TXBREMSE   32         /* jetzt auch noch TXFRAMES wegwerfen..     */

#define TRUE        1         /* BOOLEAN TRUE                             */
#define FALSE       0         /* BOOLEAN FALSE                            */
#define YES         1         /* TRUE-Synonym                             */
#define NO          0         /* FALSE-Synonym                            */
#define ERROR       (-1)      /* Fehler                                   */
#define LOOP        for(;;)   /* Endlosschleife                           */
#define STACKLEN    384       /* Anzahl Bytes fuer Systemstack            */
#define MAGIC       0x0825    /* "magic number", Warmstartfeststellung    */
                              /* (siehe auch TFL1.MAC)                    */

                              /* Portnummern :                            */
#define HDLCPORT    0         /*   Packet-Port                            */
#define ASYNPORT    1         /*   RS232 Crosslink-Port                   */

#ifndef TFKISS_MAIN
#define NULL        0         /* leerer Inhalt, leerer Zeiger,            */
#endif                        /* sollte bei 8-Bit-Systemen 0, bei 16-Bit- */
                              /* Systemen 0L heissen, 0L weil NULL auch   */
                              /* als Parameter moeglich ist, 0L bei       */
                              /* 8-Bit-Systemen kann aber dazu fuehren,   */
                              /* dass die 32-Bit-Bibliothek geladen wird, */
                              /* ohne dass sie benutzt wird               */



/*                                                       einfache Typen   */
/**************************************************************************/

typedef unsigned short  BOOLEAN;   /* 1 = TRUE, 0 = FALSE                 */



/* Ende von ALL.H */
