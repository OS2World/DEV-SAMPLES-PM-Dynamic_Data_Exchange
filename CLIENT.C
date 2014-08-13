/*
  $Workfile:   client.c  $
  $Revision:   1.1.1.0  $
  $Date:   09 May 1991 18:10:02  $
  $Logfile:   W:/dde/client.c_v  $
  $Author:   mikem  $
  $Modtime:   09 May 1991 18:07:04  $

   Sample Client DDE application
   Copyright (C) Personal Systems Developer, IBM Corporation 1991
   Author: Michael R. MacFaden
   Internet: macfaden@paloic1.vnet.ibm.com

*/

#pragma  comment(compiler)
#pragma  comment(user, "Copyright (C) IBM Corp 1991")

#define  INCL_WIN       
#define  INCL_ERRORS    
#define  INCL_DOS
#include <os2.h>       
#include <stdio.h>       
#include <string.h>     
#include "pmutils.h"
#include "client.h"

MRESULT EXPENTRY ClientWndProc(HWND,USHORT,MPARAM,MPARAM);
VOID EXPENTRY PaintWindow(HWND hwnd,CHAR *szMsg);
PDDESTRUCT EXPENTRY MakeDDEReqSeg(USHORT usFormat,PSZ pszItemName,BOOL fAck);
MRESULT EXPENTRY InitDlgProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY AboutDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);

/* globals */
#define  MAX_MSG_SZ    300             // max size of text string
CHAR szClientClass[] = "DDE-Sample";
CHAR szBuf[MAX_MSG_SZ+1];                       // formatting buffer
HWND hwndFrame,hwndClient,hwndMenu,hwndDDEserver;
USHORT cbDDEMsg;                       // length of message
BOOL fConnected = FALSE;               // status of a DDE conversation
BOOL fAdvise = FALSE;                  // status of hot link

int main()
{
  static ULONG flFrameFlags = FCF_TITLEBAR|FCF_SYSMENU|FCF_SIZEBORDER|
     FCF_MINMAX|FCF_TASKLIST|FCF_MENU;
  HAB hab = (VOID *) 0;
  HMQ hmq = (VOID *) 0;
  QMSG qmsg;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);
  if(!WinRegisterClass(hab, szClientClass,(PFNWP)ClientWndProc,0,0))
   {
      WinAlarm(HWND_DESKTOP, WA_ERROR);        /* Register failed */
      DosExit(EXIT_PROCESS,1);
   }
   hwndFrame = 0;
   hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
                                  &flFrameFlags, szClientClass, szClientClass,
                                  0, (HMODULE)0, ID_MAINWND, &hwndClient);
   if(!hwndFrame)
   {
      WinAlarm(HWND_DESKTOP, WA_ERROR); /* Window create failed */
      DosExit(EXIT_PROCESS,1);
   }

 /* Use default system icon */
  WinSendMsg(hwndFrame, WM_SETICON, WinQuerySysPointer(HWND_DESKTOP, 
    SPTR_APPICON, FALSE), NIL);

 while(WinGetMsg(hab,&qmsg,NULL,0,0))        /* Message loop */
    WinDispatchMsg(hab,&qmsg);

  WinDestroyWindow(hwndFrame);     /* clean up */
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return 0;
} 


MRESULT EXPENTRY ClientWndProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2)
{
  static DDECONV ddec;              // DDE Server name and requested topic
  static CHAR szDDEItemName[CV_MAXDDENAME]; // DDE item (piece of data) in topic format
  static CHAR szMsg[MAX_MSG_SZ];       // text string
  static USHORT usItem = 0;            // Menu item
  PDDEINIT pddei = NULL;               // DDE init struct ptr
  PDDESTRUCT pdde = NULL,pddeOut = NULL; // DDE Transaction struct ptr

  switch (msg)
    {
    case  WM_CREATE :
      hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
      hwndMenu = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT), (USHORT)FID_MENU);

      strcpy(ddec.szDDEServerName, CV_DDEAPPNAME);
      strcpy(ddec.szDDETopicName, CV_DDETOPIC);
      strcpy(szDDEItemName, CV_DDEITEMSG);
      WinSetWindowText(hwndFrame, "...Not Connected...");
      /* position window on users screen */
      WinSetWindowPos(hwndFrame,HWND_TOP,
        50,50,    /* window position (x,y) */
        250,250,  /* window size (width/height) */
        SWP_SIZE | SWP_MOVE | SWP_SHOW);
      return 0;

    case  WM_COMMAND :
      switch (SHORT1FROMMP(mp1))
        {
        case  IDM_STARTCONV :
          if (WinDlgBox(HWND_DESKTOP,
                        hwndFrame,
                        (PFNWP) InitDlgProc,
                        0, 
                        IDD_INIT_DLG,
                        &ddec))
            if (!WinDdeInitiate(hwnd, ddec.szDDEServerName, 
               ddec.szDDETopicName))
              {
              WinAlarm(HWND_DESKTOP, WA_ERROR);
              WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, 
                 "WinDdeInitiate() failed!", "API Failure", 1, MB_OK|
                 MB_ICONEXCLAMATION|MB_APPLMODAL|MB_MOVEABLE);
              } 
          return 0;

        case  IDM_STOPCONV :
          WinDdePostMsg(hwndDDEserver, hwnd, WM_DDE_TERMINATE, NULL, TRUE);
          EnableMenuItem(hwndMenu, IDM_REQUEST, FALSE);
          EnableMenuItem(hwndMenu, IDM_ADVISE, FALSE);
          EnableMenuItem(hwndMenu, IDM_ENDADVISE, FALSE);
          EnableMenuItem(hwndMenu, IDM_STOPCONV, FALSE);
          EnableMenuItem(hwndMenu, IDM_STARTCONV, TRUE);
          hwndDDEserver = NULL;        // store client window handle
          WinSetWindowText(hwndFrame, "...Not Connected...");
			  fConnected = FALSE;
          return 0;

        case  IDM_REQUEST :
          /* if a conversation on the System topic is esablished,
           issuing a request causes the current status to be retrieved
          */
			  if (strcmp("System", ddec.szDDETopicName) == 0)
                pdde = MakeDDEReqSeg(CF_TEXT, "Status", 0);
			  else	  
                 pdde = MakeDDEReqSeg(CF_TEXT, szDDEItemName, 0);
          if (pdde)
            {
            WinDdePostMsg(hwndDDEserver, hwnd, WM_DDE_REQUEST, pdde, TRUE);
            usItem = IDM_REQUEST;
            } 
          else
            {
            WinAlarm(HWND_DESKTOP, WA_ERROR);
            WinSetWindowText(hwndFrame, "...MakeDDEReqSeg() failed..");
            break;
            }
          return 0;

        case  IDM_ADVISE :
          fAdvise = TRUE;
          pddeOut = MakeDDEReqSeg(CF_TEXT, szDDEItemName, DDE_FACKREQ);
          if (pddeOut)
            {
            WinDdePostMsg(hwndDDEserver, hwnd, WM_DDE_ADVISE, pddeOut, TRUE);
            WinSetWindowText(hwndFrame, "...sent hot link request...");
            EnableMenuItem(hwndMenu, IDM_ADVISE, FALSE);
            } 
          usItem = IDM_ADVISE;
          break;

        case  IDM_ENDADVISE :
          fAdvise = FALSE;
          pddeOut = MakeDDEReqSeg(CF_TEXT, szDDEItemName, DDE_FACKREQ);
          if (pddeOut)
            {
            WinDdePostMsg(hwndDDEserver, hwnd, WM_DDE_UNADVISE, pddeOut, 
               TRUE);
            WinSetWindowText(hwndFrame, "...sent hot link terminate request...");
            usItem = IDM_ENDADVISE;
            } 
          break;

        case IDM_ABOUT:
          WinDlgBox (HWND_DESKTOP, hwnd, (PFNWP) AboutDlgProc,
                     (HMODULE) NIL, IDD_ABOUT, NULL) ;
          return 0 ;

        } 
      break; /* wm_command */

    case  WM_DDE_INITIATEACK :
      WinAlarm(HWND_DESKTOP, WA_NOTE);
      if (hwndDDEserver != (HWND)mp1 && fConnected) // is this our server
        {
        WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_TERMINATE, NULL, TRUE);
			break;
        } 

      pddei = (PDDEINIT)mp2;           // MS V2 PG 430
      if (!strcmp(ddec.szDDEServerName, pddei->pszAppName))
        {
        if (!strcmp(ddec.szDDETopicName, pddei->pszTopic))
          {
          hwndDDEserver = (HWND)mp1;   // store client window handle
          EnableMenuItem(hwndMenu, IDM_STARTCONV, FALSE);
          EnableMenuItem(hwndMenu, IDM_STOPCONV, TRUE);
          EnableMenuItem(hwndMenu, IDM_REQUEST, TRUE);
          EnableMenuItem(hwndMenu, IDM_ADVISE, TRUE);
          WinSetWindowText(hwndFrame, "Connected...");
          fConnected = TRUE;
          } 
        } 
      break;

    case  WM_DDE_ACK :                 /* server sent DDE Acknowledgement   */
      pdde = (PDDESTRUCT)mp2;
      if (!pdde)
        break;

      switch (usItem)                 // what was this ack in response to?
        {
        case  IDM_ADVISE :
          if (pdde->fsStatus & DDE_FACK)
            {
            EnableMenuItem(hwndMenu, usItem, FALSE);
            EnableMenuItem(hwndMenu, IDM_REQUEST, FALSE);
            EnableMenuItem(hwndMenu, IDM_ENDADVISE, TRUE);
            } 
          else
            {
            EnableMenuItem(hwndMenu, usItem, TRUE);
            EnableMenuItem(hwndMenu, IDM_REQUEST, TRUE);
            EnableMenuItem(hwndMenu, IDM_ENDADVISE, FALSE);
			    fAdvise = FALSE;
            } 
          break;

        case  IDM_ENDADVISE :
          if (pdde->fsStatus & DDE_FACK)
            {
            EnableMenuItem(hwndMenu, usItem, FALSE);
            EnableMenuItem(hwndMenu, IDM_REQUEST, TRUE);
            EnableMenuItem(hwndMenu, IDM_ADVISE, TRUE);
            } 
          break;

        case  IDM_REQUEST :
          if (!(pdde->fsStatus&DDE_FACK))
            WinAlarm(HWND_DESKTOP, WA_ERROR);
          break;
        }                              // switch()
      break;

    case  WM_DDE_DATA :                // server sent a chunk of data
      if (hwndDDEserver != (HWND)mp1) // ignore responses from other servers
        break;
      pdde = (PDDESTRUCT)mp2;
      if (!pdde)                       // check ptr
        break;
      // write incomming text message to screen
      strcpy(szMsg, (PSZ)DDES_PABDATA(pdde)); // Extract data
      if (strlen(szMsg) == 0)
        strcpy(szMsg, "<no text message from server>");
      PaintWindow(hwnd, szMsg);
      WinInvalidateRect(hwnd, NULL, FALSE);

      if (pdde->fsStatus&DDE_FACKREQ)  // does server requre ack?
        {
        pddeOut = MakeDDEReqSeg(CF_TEXT, szDDEItemName, DDE_FACK);
        if (pddeOut)
          {
          WinDdePostMsg(hwndDDEserver, hwnd, WM_DDE_ACK, pdde, TRUE);
          } 
        } 
      break;

    case  WM_CLOSE :
      if (hwndDDEserver)
        WinDdePostMsg(hwndDDEserver, hwnd, WM_DDE_TERMINATE, NULL, TRUE);
      WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
      return 0;

    case  WM_PAINT :
      PaintWindow(hwnd, szMsg);
      return 0;

    case  WM_DDE_TERMINATE :
      if (hwndDDEserver != (HWND)mp1) // ignore responses from other servers
        break;
      EnableMenuItem(hwndMenu, IDM_STARTCONV, TRUE);
      EnableMenuItem(hwndMenu, IDM_STOPCONV, FALSE);
		 if (fAdvise)
      EnableMenuItem(hwndMenu, IDM_ENDADVISE, FALSE);
      hwndDDEserver = NULL;            // store client window handle
      WinSetWindowText(hwndFrame, "...Not Connected...");
      fConnected = FALSE;
      break;

    case  WM_ERASEBACKGROUND :
      return (MRESULT)1;
    } 
  return  WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/*
Pre - hwnd is a valid window handle, szMsg is a null terminated string (PSZ)
Post - writes szMsg in hwnd centered vertically and horizontally
*/

VOID EXPENTRY PaintWindow(HWND hwnd,CHAR *szMsg)
{
  HPS hps = NULL;
  RECTL rcl;

  hps = WinBeginPaint(hwnd, NULL, NULL);
  if (!hps)
    return ;
  WinQueryWindowRect(hwnd, &rcl);
  WinDrawText(hps, -1, szMsg, &rcl, CLR_NEUTRAL, CLR_BACKGROUND, DT_CENTER|
     DT_VCENTER|DT_ERASERECT);

  WinEndPaint(hps);
} // PaintWindow()

/*
Pre  - this routine is called directly by PM
Post - handles all window processing for dialog box: IDD_INIT_DLG
*/

MRESULT EXPENTRY InitDlgProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2)
{
  static PDDECONV pddec = NULL;        // ptr to dde server name and topic
  static HWND hwndServer = NULL,hwndTopic = NULL; // text entry window handles

  switch (msg)
    {
    case  WM_INITDLG :
      pddec = MPFROMP(mp2);
      if (pddec == NULL)
        {
        WinMessageBox(HWND_DESKTOP, hwndClient, "Invalid pointer", 
           "PMMSG - Error", 10, MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL|
           MB_MOVEABLE);
        return (MRESULT)WinDismissDlg(hwnd, FALSE);
        } 
      /* 1. Set up text entry field default strings and lengths */
      SetEFTextLength(hwnd, ID_SERVER_CBX, CV_MAXDDENAME);
      SetEFTextLength(hwnd, ID_TOPIC_CBX, CV_MAXTOPICNAME);
      hwndServer = WinWindowFromID(hwnd, ID_SERVER_CBX);
      if (hwndServer)
        {
        WinSetWindowText(hwndServer, pddec->szDDEServerName);
        WinSendMsg(hwndServer, EM_SETSEL,  // highlight for easy discard
           MPFROM2SHORT((USHORT)0, (USHORT)CV_MAXDDENAME), (MPARAM)0);
        /* insert app name into combo box */
        WinSendMsg(hwndServer, LM_INSERTITEM, MPFROMSHORT((SHORT) LIT_END),
           MPFROMP(pddec->szDDEServerName));
        } 
      hwndTopic = WinWindowFromID(hwnd, ID_TOPIC_CBX);
      if (hwndTopic)
        {
        // 2. Enter default topics into combo box
        WinSendMsg(hwndTopic, LM_INSERTITEM, MPFROMSHORT((SHORT) LIT_END), MPFROMP(CV_DDETOPIC));
        WinSendMsg(hwndTopic, LM_INSERTITEM, MPFROMSHORT((SHORT) LIT_END), MPFROMP(SZDDESYS_TOPIC));
        WinSetWindowText(hwndTopic, pddec->szDDETopicName);
        }
      return 0;  /* wm_initdlg */

    case  WM_COMMAND :
      switch (SHORT1FROMMP(mp1))
        {
        case  DID_OK :
           // obtain new values to use 
          WinQueryWindowText(hwndServer, CV_MAXDDENAME,  
             pddec->szDDEServerName);
          WinQueryWindowText(hwndTopic, CV_MAXTOPICNAME,  
             pddec->szDDETopicName);
          return (MRESULT)WinDismissDlg(hwnd, TRUE);

        case  DID_CANCEL :
          return (MRESULT)WinDismissDlg(hwnd, FALSE);

        }                              // wm_command
      break;

    case  WM_SYSCOMMAND :              // process dlg box system menu
      switch (LOUSHORT(mp1))
        {
        case  SC_CLOSE :
          return (MRESULT)WinDismissDlg(hwnd, FALSE);

        default  :
          return  WinDefDlgProc(hwnd, msg, mp1, mp2);
        }                              // wm_syscommand

      break;                           // wm_syscommand
    }                                  // switch()
  return  WinDefDlgProc(hwnd, msg, mp1, mp2);
}                                      // InitDlgProc()

/*
Pre - format, item name, and fsStatus are valid
Post - a valid data segment for WM_DDE_REQUEST message is built
       and a pddestruct is returned else (error) null is returned
*/

PDDESTRUCT EXPENTRY MakeDDEReqSeg(USHORT usFormat,PSZ pszItemName,USHORT 
                                  fsStatus)
{
  PULONG pulSharedObj = NULL;     /* pointer to shared object */
  PDDESTRUCT pdde = NULL;
  ULONG rc = 0;
  USHORT cbObjSize = 0;                // count of bytes to request

  /**************************************************************************/
  /* 1) Allocate a givable memory block                                     */
  /**************************************************************************/

  cbObjSize = (USHORT)strlen(pszItemName)+ (USHORT) 1;
  rc = DosAllocSharedMem(&pulSharedObj, NULL, cbObjSize,
             PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_GIVEABLE);
  if (rc && pulSharedObj)          // check api return code and ptr
    return  NULL;

  /**************************************************************************/
  /* 2) Fill in the new DDE structure                                       */
  /**************************************************************************/

  pdde = (PDDESTRUCT) pulSharedObj;
  pdde->cbData = cbObjSize;
  pdde->offszItemName = sizeof(DDESTRUCT);
  pdde->usFormat = usFormat;
  strcpy(DDES_PSZITEMNAME(pdde), pszItemName); // item name
  pdde->offabData = (USHORT) pdde->offszItemName+ (USHORT) strlen(pszItemName)+ (USHORT)1;
  pdde->fsStatus = fsStatus;           // set status flags
  pdde->cbData = 0;                    // no data is needed
  pdde->offabData = 0;
  return  pdde;
}  /* MakeDDESeg() */

/*
Pre - valid PM client exists
Post - processing of about dialog box completed
*/

MRESULT EXPENTRY AboutDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg)
      {
      case WM_COMMAND:
      switch (SHORT1FROMMP(mp1))
           {
           case DID_OK:
           case DID_CANCEL:
           WinDismissDlg (hwnd, TRUE);
           return 0;
           }
      break ;
      }
  return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
} /* AboutDlgProc */


// $Workfile:   client.c  $

