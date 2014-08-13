 /*
  $Workfile:   server.h  $
  $Revision:   1.1.1.0  $
  $Date:   09 May 1991 18:11:08  $
  $Logfile:   W:/dde/server.h_v  $
  $Author:   mikem  $
  $Modtime:   09 May 1991 18:07:04  $
*/

/* Maximum string sizes for various names and text */
#define MAX_DDEAPPL_SZ    64		// Arbritrary max string size
#define MAX_DDETOPIC_SZ   64		// not defined by protocol
#define MAX_DDEITEM_SZ    64		// "   " 
  
/* Default DDE communications parameters */

/* This DDE Applications Name */
#define CV_DDEAPPNAME "QuoteServer"

/* List of Topics supported  (this list can be changed by the
application at any time) */

PSZ szDDEtopics[] =
{
SZDDESYS_TOPIC, 	  
"Quotes"	  
};

#define CB_DDETOPICS (sizeof szDDEtopics / sizeof szDDEtopics[0])
#define CV_DDETOPICLIST       szDDEtopics
#define CV_DDESYSTEMTOPIC     szDDEtopics[0]
#define CV_DDEUSRTOPIC        szDDEtopics[1]

/* List of topic: "System" items supported */
PSZ szSysItems[] =
{   SZDDESYS_ITEM_SYSITEMS,
    SZDDESYS_ITEM_TOPICS,
    SZDDESYS_ITEM_RTNMSG,
    SZDDESYS_ITEM_STATUS,
    SZDDESYS_ITEM_FORMATS
};

#define CB_SYSITEMS (sizeof szSysItems / sizeof szSysItems[0])
#define CV_SYSITEMLIST           szSysItems
#define CV_DDESYSITEMS		       szSysItems[0]
#define CV_DDESYSTOPICS		    szSysItems[1]
#define CV_DDESYSRETURNMSG	    szSysItems[2]
#define CV_DDESYSSTATUS          szSysItems[3]
#define CV_DDEFORMATS            szSysItems[4]

/* Application specific topics */
#define CV_DDEUSRITEM "Quote"    /* Format is DDEFMT_TEXT */

/* Application needs to keep trace of clients */
/* this simple structure is used for a single conversation */
typedef struct _DDESRV { /* ddesrv */
   HWND hwndDDEClient;
   BOOL fAck;				// client requires receipt for messages sent
   BOOL fAdvise;			// status of a hot link
   BOOL fConnected;		// status of this connection
   BOOL fRefData;		// send update messages only, don't send any data
	 USHORT usFormat;    // mutually agreed upon data format to use
	 CHAR szTopic[MAX_DDETOPIC_SZ];		// Conversation item type
	 CHAR szItem[MAX_DDEITEM_SZ];		// Conversation item type
} DDESRV;
typedef DDESRV FAR *PDDESRV;

CHAR szDDEsysReply[500];  // system topic return data (formatted buffer)

// $Workfile:   server.h  $
