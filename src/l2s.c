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
*    L2S.C   -   Level 2 Statetable                                        *
*                                                                          *
*    angelegt:      DC4OX                                                  *
*    modifiziert:   DG2FEF, 18.07.94                                       *
*                   stbl23: sdipoll eingebaut                              *
*                                                                          *
*                   DG2FEF, 13.08.94                                       *
*                   stbl21, RNR-recovery ge„ndert                          *
*                   stbl22                                                 *
*                                                                          *
\**************************************************************************/





/*                                                             Includes   */
/**************************************************************************/

#include "all.h"         /* allgemeine Festlegungen                       */
#include "l2.h"          /* Festlegungen/Datenstrukturen fuer den Level 2 */





/*                                                              Defines   */
/**************************************************************************/

#define NUMSTATS 16           /* Anzahl States AX.25 Level 2              */



/*
 *    0 L2SDSCED  -  disconnected
 *    1 L2SLKSUP  -  link setup
 *    2 L2SFRREJ  -  frame reject
 *    3 LS2DSCRQ  -  disconnect request
 *    4 L2SIXFER  -  information transfer
 *    5 L2SRS     -  REJ sent
 *    6 L2SWA     -  waiting acknowledge
 *    7 L2SDBS    -  device busy
 *    8 L2SRBS    -  remote busy
 *    9 L2SBBS    -  both busy
 *   10 L2SWADBS  -  waiting ack and device busy
 *   11 L2SWARBS  -  waiting ack and remote busy
 *   12 L2SWABBS  -  waiting ack and both busy
 *   13 L2SRSDBS  -  REJ sent and device busy
 *   14 L2SRSRBS  -  REJ sent and remote busy
 *   15 L2SRSBBS  -  REJ sent and both busy
 */





/*                                                            Externals   */
/**************************************************************************/

extern void xdm();
extern void xnull();
extern void xfrmr();
extern void xrrr();
extern void xrnrr();
extern void t2rrr();
extern void t2rnrr();
extern void xua();
extern void xsabm();
extern void xdisc();
extern void xrrc();
extern void xrnrc();
extern void t2rejr();
extern void xrejr();
extern void sdipoll();




/**************************************************************************\
*                                                                          *
*   I WITH POLL (COMMAND)                                                  *
*                                                                          *
\**************************************************************************/

STENTRY stbl00[NUMSTATS] =
  {
    { xdm    , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xfrmr  , L2SFRREJ },
    { xdm    , L2SDSCRQ },
    { xrrr   , L2SIXFER },
    { xrrr   , L2SIXFER },
    { xrrr   , L2SWA    },
    { xrnrr  , L2SDBS   },
    { xrrr   , L2SRBS   },
    { xrnrr  , L2SBBS   },
    { xrnrr  , L2SWADBS },
    { xrrr   , L2SWARBS },
    { xrnrr  , L2SWABBS },
    { xrnrr  , L2SRSDBS },
    { xrrr   , L2SRBS   },
    { xrnrr  , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
* I WITHOUT POLL (COMMAND)                                                 *
*                                                                          *
\**************************************************************************/

STENTRY stbl01[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { t2rrr  , L2SIXFER },
    { t2rrr  , L2SIXFER },
    { t2rrr  , L2SWA    },
    { t2rnrr , L2SDBS   },
    { t2rrr  , L2SRBS   },
    { t2rnrr , L2SBBS   },
    { t2rnrr , L2SWADBS },
    { t2rrr  , L2SWARBS },
    { t2rnrr , L2SWABBS },
    { t2rnrr , L2SRSDBS },
    { t2rrr  , L2SRBS   },
    { t2rnrr , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
* RR WITH POLL (COMMAND)                                                   *
*                                                                          *
\**************************************************************************/

STENTRY stbl02[NUMSTATS] =
  {
    { xdm    , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xfrmr  , L2SFRREJ },
    { xdm    , L2SDSCRQ },
    { xrrr   , L2SIXFER },
    { xrrr   , L2SRS    },
    { xrrr   , L2SWA    },
    { xrnrr  , L2SDBS   },
    { xrrr   , L2SIXFER },
    { xrnrr  , L2SDBS   },
    { xrnrr  , L2SWADBS },
    { xrrr   , L2SWA    },
    { xrnrr  , L2SWADBS },
    { xrnrr  , L2SRSDBS },
    { xrrr   , L2SRS    },
    { xrnrr  , L2SRSDBS }
  };



/**************************************************************************\
*                                                                          *
*   RR WITHOUT POLL (COMMAND)                                              *
*                                                                          *
\**************************************************************************/

STENTRY stbl03[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SIXFER },
    { xnull  , L2SRS    },
    { xnull  , L2SWA    },
    { xnull  , L2SDBS   },
    { xnull  , L2SIXFER },
    { xnull  , L2SDBS   },
    { xnull  , L2SWADBS },
    { xnull  , L2SWA    },
    { xnull  , L2SWADBS },
    { xnull  , L2SRSDBS },
    { xnull  , L2SRS    },
    { xnull  , L2SRSDBS }
  };


/**************************************************************************\
*                                                                          *
*   RNR WITH POLL (COMMAND)                                                *
*                                                                          *
\**************************************************************************/

STENTRY stbl06[NUMSTATS] =
  {
    { xdm    , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xfrmr  , L2SFRREJ },
    { xdm    , L2SDSCRQ },
    { xrrr   , L2SRBS   },
    { xrrr   , L2SRSRBS },
    { xrrr   , L2SWARBS },
    { xrnrr  , L2SBBS   },
    { xrrr   , L2SRBS   },
    { xrnrr  , L2SBBS   },
    { xrnrr  , L2SWABBS },
    { xrrr   , L2SWARBS },
    { xrnrr  , L2SWABBS },
    { xrnrr  , L2SRSBBS },
    { xrrr   , L2SRSRBS },
    { xrnrr  , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
*   RNR WITHOUT POLL (COMMAND)                                             *
*                                                                          *
\**************************************************************************/

STENTRY stbl07[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SRBS   },
    { xnull  , L2SRSRBS },
    { xnull  , L2SWARBS },
    { xnull  , L2SBBS   },
    { xnull  , L2SRBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SWABBS },
    { xnull  , L2SWARBS },
    { xnull  , L2SWABBS },
    { xnull  , L2SRSBBS },
    { xnull  , L2SRSRBS },
    { xnull  , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
*   SABM EITHER (COMMAND)                                                  *
*                                                                          *
\**************************************************************************/

STENTRY stbl08[NUMSTATS] =
  {
    { xua    , L2SIXFER },
    { xua    , L2SIXFER },
    { xua    , L2SIXFER },
    { xua    , L2SDSCED },
    { xua    , L2SIXFER },
    { xua    , L2SIXFER },
    { xua    , L2SIXFER },
    { xua    , L2SDBS   },
    { xua    , L2SIXFER },
    { xua    , L2SDBS   },
    { xua    , L2SDBS   },
    { xua    , L2SIXFER },
    { xua    , L2SDBS   },
    { xua    , L2SDBS   },
    { xua    , L2SIXFER },
    { xua    , L2SIXFER }
  };



/**************************************************************************\
*                                                                          *
*   DISC EITHER (COMMAND)                                                  *
*                                                                          *
\**************************************************************************/

STENTRY stbl09[NUMSTATS] =
  {
    { xdm    , L2SDSCED },
    { xdm    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED },
    { xua    , L2SDSCED }
  };



/**************************************************************************\
*                                                                          *
*   RR WITH FINAL (RESPONSE)                                               *
*                                                                          *
\**************************************************************************/

STENTRY stbl10[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SIXFER },
    { xnull  , L2SRS    },
    { xnull  , L2SIXFER },
    { xnull  , L2SDBS   },
    { xnull  , L2SIXFER },
    { xnull  , L2SDBS   },
    { xnull  , L2SDBS   },
    { xnull  , L2SIXFER },
    { xnull  , L2SDBS   },
    { xnull  , L2SRSDBS },
    { xnull  , L2SRS    },
    { xnull  , L2SRSDBS }
  };

/**************************************************************************\
*                                                                          *
*   DAMA POLL                                                              *
*                                                                          *
\**************************************************************************/

STENTRY stb10a[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SIXFER },
    { xnull  , L2SRS    },
    { xnull  , L2SIXFER },
    { xnull  , L2SDBS   },
    { xnull  , L2SRBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SDBS   },
    { xnull  , L2SRBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SRSDBS },
    { xnull  , L2SRSRBS },
    { xnull  , L2SRSBBS }
  };


/**************************************************************************\
*                                                                          *
*   RNR WITH FINAL (RESPONSE)                                              *
*                                                                          *
\**************************************************************************/

STENTRY stbl14[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SRBS   },
    { xnull  , L2SRSRBS },
    { xnull  , L2SRBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SRBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SRBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SRSBBS },
    { xnull  , L2SRSRBS },
    { xnull  , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
* UA EITHER (RESPONSE)                                                     *
*                                                                          *
\**************************************************************************/

STENTRY stbl16[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SIXFER },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCED },
    { xnull  , L2SIXFER },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP }
  };



/**************************************************************************\
*                                                                          *
* DM EITHER (RESPONSE)                                                     *
*                                                                          *
\**************************************************************************/

STENTRY stbl17[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED },
    { xnull  , L2SDSCED }
  };



/**************************************************************************\
*                                                                          *
*   FRMR EITHER (RESPONSE)                                                 *
*                                                                          *
\**************************************************************************/

STENTRY stbl18[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xnull  , L2SDSCRQ },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP },
    { xsabm  , L2SLKSUP }
  };



/**************************************************************************\
*                                                                          *
* STATION BECOMES BUSY FEF                                                 **
*                                                                          *
\**************************************************************************/

STENTRY stbl21[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SDBS   },
    { xnull  , L2SRSDBS },
    { xnull  , L2SWADBS },
    { xnull  , L2SDBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SBBS   },
    { xnull  , L2SWADBS },
    { xnull  , L2SWABBS },
    { xnull  , L2SWABBS },
    { xnull  , L2SRSDBS },
    { xnull  , L2SRSBBS },
    { xnull  , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
* BUSY CONDITION CLEARS  FEF                                               *
*                                                                          *
\**************************************************************************/

STENTRY stbl22[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { xnull  , L2SIXFER },
    { xnull  , L2SRS    },
    { xnull  , L2SWA    },
    { xrrc   , L2SIXFER },
    { xnull  , L2SRBS   },
    { xrrc   , L2SRBS   },
    { xrrc   , L2SWA    },
    { xnull  , L2SWARBS },
    { xrrc   , L2SWARBS },
    { xrrc   , L2SRS    },
    { xnull  , L2SRSRBS },
    { xrrc   , L2SRSRBS }
  };



/**************************************************************************\
*                                                                          *
* T1 EXPIRES (VERSION 2)                                                   *
*                                                                          *
\**************************************************************************/

STENTRY stbl23[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xsabm  , L2SLKSUP },
    { xfrmr  , L2SFRREJ },
    { xdisc  , L2SDSCRQ },
    { sdipoll, L2SWA    },
    { sdipoll, L2SWA    },
    { sdipoll, L2SWA    },
    { xrnrc  , L2SWADBS },
    { xrrc   , L2SWARBS },
    { xrnrc  , L2SWABBS },
    { xrnrc  , L2SWADBS },
    { xrrc   , L2SWARBS },
    { xrnrc  , L2SWABBS },
    { xrnrc  , L2SWADBS },
    { xrrc   , L2SWARBS },
    { xrnrc  , L2SWADBS }
  };



/**************************************************************************\
*                                                                          *
*   T3 EXPIRES                                                             *
*                                                                          *
\**************************************************************************/

STENTRY stbl24[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xsabm  , L2SLKSUP },
    { xfrmr  , L2SFRREJ },
    { xdisc  , L2SDSCRQ },
    { xrrc   , L2SWA    },
    { xrrc   , L2SWA    },
    { xnull  , L2SWA    },
    { xrnrc  , L2SWADBS },
    { xrrc   , L2SWARBS },
    { xrnrc  , L2SWABBS },
    { xnull  , L2SWADBS },
    { xnull  , L2SWARBS },
    { xnull  , L2SWABBS },
    { xrnrc  , L2SWADBS },
    { xrrc   , L2SWARBS },
    { xrnrc  , L2SWADBS }
  };



/**************************************************************************\
*                                                                          *
*   INVALID N(S) RECEIVED WITHOUT POLL                                     *
*                                                                          *
\**************************************************************************/

STENTRY stbl26[NUMSTATS] =
  {
    { xnull  , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xnull  , L2SFRREJ },
    { xnull  , L2SDSCRQ },
    { t2rejr , L2SRS    },
    { xnull  , L2SRS    },
    { xnull  , L2SWA    },
    { t2rnrr , L2SDBS   },
    { t2rejr , L2SRSRBS },
    { t2rnrr , L2SBBS   },
    { xnull  , L2SWADBS },
    { xnull  , L2SWARBS },
    { xnull  , L2SWABBS },
    { t2rnrr , L2SRSDBS },
    { xnull  , L2SRSRBS },
    { t2rnrr , L2SRSBBS }
  };



/**************************************************************************\
*                                                                          *
*   INVALID N(S) RECEIVED WITH POLL                                        *
*                                                                          *
\**************************************************************************/

STENTRY stb26b[NUMSTATS] =
  {
    { xdm    , L2SDSCED },
    { xnull  , L2SLKSUP },
    { xfrmr  , L2SFRREJ },
    { xdm    , L2SDSCRQ },
    { xrejr  , L2SRS    },
    { xrejr  , L2SRS    },
    { xrejr  , L2SWA    },
    { xrnrr  , L2SDBS   },
    { xrejr  , L2SRSRBS },
    { xrnrr  , L2SBBS   },
    { xrnrr  , L2SWADBS },
    { xrejr  , L2SWARBS },
    { xrnrr  , L2SWABBS },
    { xrnrr  , L2SRSDBS },
    { xrejr  , L2SRSRBS },
    { xrnrr  , L2SRSBBS }
  };



/* Ende von L2S.C */
