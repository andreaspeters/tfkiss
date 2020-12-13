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
*    L2S.H   -   Definitionen aus L2S.C fuer andere Module                 *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:                                                          *
*                                                                          *
\**************************************************************************/



/*                        Zugriff anderer Module auf die State-Tabellen   */
/**************************************************************************/

/*
 *   nicht aufgefuehrte Tabellen werden durch das Programm direkt
 *   ausgefuehrt
 */

extern    STENTRY stbl00[];             /* I WITH POLL (COMMAND)          */
extern    STENTRY stbl01[];             /* I WITHOUT POLL (COMMAND)       */
extern    STENTRY stbl02[];             /* RR WITH POLL (COMMAND)         */
extern    STENTRY stbl03[];             /* RR WITHOUT POLL (COMMAND)      */
#define           stbl04      stbl02    /* REJ WITH POLL (COMMAND)        */
#define           stbl05      stbl03    /* REJ WITHOUT POLL (COMMAND)     */
extern    STENTRY stbl06[];             /* RNR WITH POLL (COMMAND)        */
extern    STENTRY stbl07[];             /* RNR WITHOUT POLL (COMMAND)     */
extern    STENTRY stbl08[];             /* SABM EITHER (COMMAND)          */
extern    STENTRY stbl09[];             /* DISC EITHER (COMMAND)          */
extern    STENTRY stbl10[];             /* RR WITH FINAL (RESPONSE)       */
extern    STENTRY stb10a[];             /* DAMA POLL                      */
#define           stbl11      stbl03    /* RR WITHOUT FINAL (RESPONSE)    */
#define           stbl12      stbl10    /* REJ WITH FINAL (RESPONSE)      */
#define           stbl13      stbl03    /* REJ WITHOUT FINAL (RESPONSE)   */
extern    STENTRY stbl14[];             /* RNR WITH FINAL (RESPONSE)      */
#define           stbl15      stbl07    /* RNR WITHOUT FINAL (RESPONSE)   */
extern    STENTRY stbl16[];             /* UA EITHER (RESPONSE)           */
extern    STENTRY stbl17[];             /* DM EITHER (RESPONSE)           */
extern    STENTRY stbl18[];             /* FRMR EITHER (RESPONSE)         */
/*                stbl19      */        /* LOCAL START COMMAND            */
/*                stbl20      */        /* LOCAL STOP COMMAND             */
extern    STENTRY stbl21[];             /* STATION BECOMES BUSY           */
extern    STENTRY stbl22[];             /* BUSY CONDITION CLEARS          */
extern    STENTRY stbl23[];             /* T1 EXPIRES (VERSION 2)         */
extern    STENTRY stbl24[];             /* T3 EXPIRES                     */
/*                stbl25      */        /* N2 IS EXCEEDED                 */
extern    STENTRY stbl26[];             /* INVALID N(S) RECEIVED  no Poll */
extern    STENTRY stb26b[];             /* INVALID N(S) RECEIVED  Poll    */
/*                stbl27      */        /* INVALID N(R) RECEIVED          */
/*                stbl28      */        /* UNRECOGNIZED FRAME RECEIVED    */



/* Ende von L2S.H */

