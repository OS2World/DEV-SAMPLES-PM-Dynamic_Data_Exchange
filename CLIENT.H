/*
  $Workfile:   client.h  $
  $Revision:   1.1.1.0  $
  $Date:   09 May 1991 18:09:54  $
  $Logfile:   W:/dde/client.h_v  $
  $Author:   mikem  $
  $Modtime:   09 May 1991 18:07:04  $

   Sample Client DDE application 
   Copyright (C) Personal Systems Developer, IBM Corporation 1991
   Author: Michael R. MacFaden
   Internet: macfaden@paloic1.vnet.ibm.com
*/

/* Maximum string sizes for various names and text */

#define  CV_MAXDDENAME 64              /* arbritary max sizes  */
#define  CV_MAXTOPICNAME 64            /* not defined by protocol */

/* Default DDE communications parameters  */

#define  CV_DDEAPPNAME "QuoteServer"
#define  CV_DDETOPIC   "Quotes"
#define  CV_DDEITEMSG  "Quote"

/* struct def  */

typedef struct _DDECONV
      {                                /* ddec */
      CHAR szDDEServerName[CV_MAXDDENAME];
      CHAR szDDETopicName[CV_MAXDDENAME];
      } DDECONV;

typedef DDECONV FAR *PDDECONV;

/* PM Menu  */

#define  ID_MAINWND    256
#define  IDM_DDE       1
#define  IDM_STARTCONV 10
#define  IDM_STOPCONV  11
#define  IDM_REQUEST   12
#define  IDM_ADVISE    13
#define  IDM_ENDADVISE 14
#define  IDM_ABOUT     15

/* PM dialog box */

#define  IDD_INIT_DLG  500
#define  ID_SERVER_CBX 501
#define  ID_TOPIC_CBX  502


/* About dialog box */
#define IDD_ABOUT 600
