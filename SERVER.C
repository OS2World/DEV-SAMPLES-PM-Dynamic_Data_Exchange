/*
  $Workfile:   server.c  $
  $Revision:   1.1.1.0  $
  $Date:   09 May 1991 18:11:26  $
  $Logfile:   W:/dde/server.c_v  $
  $Author:   mikem  $
  $Modtime:   09 May 1991 18:07:04  $
   Quote of the day server - Personal Systems Developer
   Sample Server DDE application
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
#include <stdlib.h>
#include <string.h>
#include "pmutils.h"
#include "server.h"
MRESULT EXPENTRY ClientWndProc(HWND,USHORT,MPARAM,MPARAM);

VOID EXPENTRY PaintWindow(HWND hwnd,CHAR *szMsg);
PDDESTRUCT EXPENTRY MakeDDEDataSeg(USHORT usFormat,PSZ pszItemName,PVOID
                                   pvData,USHORT cbData,USHORT fsStatus);

/* globals */

#define  MAX_MSG_SZ    300             // max size of text string
#define  ID_MAINWND    256             // Frame window ID
CHAR szClientClass[] = "DDE-Sample";
HAB hab;
HWND hwndFrame,hwndClient,hwndMenu,hwndDDEserver;
USHORT cbQuote;                        // length of message
USHORT ndx = 0;                        // index to quote struct
CHAR szBuf[256];                       // temporary buffer
PSZ szQuote[] =
      {
      "Play it again, Sam", "Peace in our time", "Step on it, Mac",
      "Error free code", "All men are created equal", "It's Miller Time",
      "Read my lips", "Come up and see me some time", "You only live twice",
      "Land Ho!", "You can't get there from here",
      "Please fasten you seat belts", "I like Ike", "He's dead Jim",
      "...Talking 'bout my generation",
      } ;

#define  CB_QUOTES     (sizeof szQuote / sizeof szQuote[0])
#define  ID_TIMER      1

int main()
{
  static ULONG flFrameFlags = FCF_TITLEBAR|FCF_SYSMENU|FCF_SIZEBORDER|
     FCF_MINMAX|FCF_TASKLIST;
  HMQ hmq;
  QMSG qmsg;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);
  WinRegisterClass(hab, szClientClass, ClientWndProc, 0L, 0);
  hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &flFrameFlags,
     szClientClass, NULL, 0L, (HMODULE)0, ID_MAINWND, &hwndClient);

  WinSendMsg(hwndFrame, WM_SETICON, WinQuerySysPointer(HWND_DESKTOP,
     SPTR_APPICON, FALSE), NULL);

  while (WinGetMsg(hab, &qmsg, NULL, 0, 0))
    WinDispatchMsg(hab, &qmsg);

  WinDestroyWindow(hwndFrame);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return 0;
}


MRESULT EXPENTRY ClientWndProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2)
{
  static USHORT usItem = 0;            // Menu item
  static DDESRV ddesrv;                // Keep info on conversations
  static BOOL fTimerOn = FALSE;        // Got a system timer
  PDDEINIT pddei = NULL;               // DDE init struct ptr
  PDDESTRUCT pddeIn = NULL,pddeOut = NULL; // DDE Transaction struct ptr
  register int i;                      // work counter variable

  switch (msg)
    {
    case  WM_CREATE :
      hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
      hwndMenu = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
         FID_MENU);

      WinSetWindowText(hwndFrame, "...DDE Server Not Connected...");
      /* place the window on the users screen */
      WinSetWindowPos(hwndFrame,HWND_TOP,
          350,  50,  /*  window position (x,y)*/
          250, 250,  /*  window size (width/height) */
          SWP_SIZE | SWP_MOVE | SWP_SHOW);

      /**********************************************************************/
      /* DDE initialization                                                 */
      /**********************************************************************/

      if (!WinStartTimer(hab, hwnd, ID_TIMER, 5000))
        WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
           "Try closing a few applications and restart", "Out of Timers!",
           12, MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL|MB_MOVEABLE);
      else
        fTimerOn = TRUE;

      ddesrv.fAdvise = FALSE;          // hot link
      ddesrv.fAck = FALSE;             // acknoledgement
      ddesrv.fConnected = FALSE;       // conversation established
               // let the all the know we exist and are ready...
      WinDdeInitiate(hwnd, CV_DDEAPPNAME, CV_DDESYSTEMTOPIC);
      return 0;

    case  WM_PAINT :
      PaintWindow(hwnd, szQuote[ndx]);
      return 0;

    case  WM_TIMER :   // change current quote every 5 seconds
      if (++ndx == CB_QUOTES)       // increment to next quote or loop back
        ndx = 0;
      WinInvalidateRect(hwnd, NULL, FALSE);
      if (ddesrv.fConnected)		  // does a conversation exist?
      if (strcmp(ddesrv.szTopic, "Quotes") == 0)   	 // ...on this topic?
          if (ddesrv.fAdvise)       // if we have a 	 // ..and is hot linked?
            {
            if (ddesrv.fRefData) // No data required?
              cbQuote = 0;
            else
              cbQuote =(USHORT) strlen(szQuote[ndx]) + (USHORT)1;
            pddeOut = MakeDDEDataSeg(CF_TEXT, "Quote", (PVOID)
              szQuote[ndx], (USHORT)cbQuote, 0);
           WinDdePostMsg(ddesrv.hwndDDEClient, hwnd, WM_DDE_DATA, pddeOut, TRUE);
           }
      return 0;

    case  WM_ERASEBACKGROUND :
      return (MRESULT)1;

    case  WM_DDE_INITIATE : // A DDE client is broadcasting to find a server
         // is this our own broadcast? then don't process it
      if ((HWND)mp1 == hwnd)
        break;
         // create ptr to client's specified appplication and topic

      pddei = (PDDEINIT)mp2;

      if (!ddesrv.fConnected)          // Can we handle another client?
        {
        /* check to see if an the "application" is explicit or general */
        if (strcmp(CV_DDEAPPNAME, pddei->pszAppName) == 0 ||
           (pddei->pszAppName == NULL))
          {
           /* OK, now Check to see if what topic the conversion will use...
              first check to see if it uses our default topic          */
           sprintf(szBuf, "Request in for for appl: %s Topic: %s",
             pddei->pszAppName, pddei->pszTopic);

           strlen(pddei->pszTopic);
           WinAlarm(HWND_DESKTOP, WA_NOTE);
           WinSetWindowText(hwndFrame, szBuf);

           if (strcmp("Quotes", pddei->pszTopic) == 0)
             {                          /* Server and topic names matched    */
             ddesrv.hwndDDEClient = (HWND)mp1; // store client window handle
             WinDdeRespond((HWND)mp1, hwnd, CV_DDEAPPNAME, "Quotes");
             strcpy(ddesrv.szTopic, "Quotes"); // record what topic is used
             ddesrv.fConnected = TRUE;  // conversation established
             WinSetWindowText(hwndFrame, "...Connected on \"Quotes\" Topic...");
             }
           else
            // next, check to see if it wants to converse using the system topic
            if (strcmp("System", pddei->pszTopic) == 0)
              {                        /* Server and topic names matched    */
              ddesrv.hwndDDEClient = (HWND)mp1; // store client window handle
              WinDdeRespond((HWND) mp1, hwnd, CV_DDEAPPNAME, "System");
              strcpy(ddesrv.szTopic, "System"); // record topic
              ddesrv.fConnected = TRUE; // conversation established
              WinSetWindowText(hwndFrame, "...Connected on System Topic...");
              }
        /* if both the appl and system topic are null respond with all topics?
           this is different than using a conversation on the system topic
           and responding to the item called Topics and Formats */
          }
        }
      break;

    case  WM_DDE_REQUEST :             /* one time DDE data request         */
      pddeIn = (PDDESTRUCT)mp2;
      if (pddeIn == NULL)
        break;        // no DDESTRUCT passed, ignore and don't bomb out
         // Do we have an established conversation with this client?

      if (ddesrv.hwndDDEClient != (HWND)mp1)
        {
        break;
        }

      if (strcmp(SZDDESYS_TOPIC, ddesrv.szTopic) == 0) // process system topic items
        {
			szDDEsysReply[0] = '\0';
        // Check to see what item the client requires
        if (strcmp(DDES_PSZITEMNAME(pddeIn), CV_DDESYSITEMS) == 0)
          {
               // Return list of system items supported
               // create a string with each item delimeted by tab char
          for (i = 0; i < CB_SYSITEMS; ++i)
            sprintf(szDDEsysReply,"%s\t%s", szDDEsysReply, szSysItems+i);
          pddeOut = MakeDDEDataSeg(CF_TEXT, CV_DDESYSITEMS, (PVOID)
             szDDEsysReply, (USHORT)strlen(szDDEsysReply)+(USHORT)1, 0);
          WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_DATA, pddeOut, TRUE);
          break;
          }
        else
          if (strcmp(DDES_PSZITEMNAME(pddeIn), CV_DDESYSTOPICS) == 0)
            {
               // return list of current topics supported
            for (i = 0; i < CB_DDETOPICS; ++i)
              sprintf(szDDEsysReply, "%s\t%s", szDDEsysReply, szDDEtopics+i);
            pddeOut = MakeDDEDataSeg(CF_TEXT, CV_DDESYSTOPICS, (PVOID)
               szDDEsysReply, (USHORT)strlen(szDDEsysReply)+(USHORT)1, 0);
            WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_DATA, pddeOut, TRUE);
            break;
            }
          else
            if (strcmp(DDES_PSZITEMNAME(pddeIn), CV_DDESYSRETURNMSG) == 0)
              {
               // additional detail for most recently issued WM_DDE_ACK
               // build an error msg and return a string & number or whatever
					// this example just passes back an empty dde segment
              pddeOut = MakeDDEDataSeg(CF_TEXT, CV_DDESYSRETURNMSG,
                 (PVOID)NULL, (USHORT)0, 0);
              WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_DATA, pddeOut, TRUE);
              break;
              }
            else
              if (strcmp(DDES_PSZITEMNAME(pddeIn), CV_DDESYSSTATUS) == 0)
                {
               // Return string dumping contents of the DDESRV struct
               // both DDESRV and the format of this item are application defined
                if (ddesrv.fConnected)
                  {
                  sprintf(szDDEsysReply, "%s", "Connected");
                  sprintf(szDDEsysReply, "%s\tTopic is %s", szDDEsysReply,
							ddesrv.szTopic);
                  }
                else
                  sprintf(szDDEsysReply,"%s\tNot Connected", szDDEsysReply);
                if (ddesrv.fAdvise)
                  sprintf(szDDEsysReply,"%s\tHot Link established", szDDEsysReply);
                else
                  sprintf(szDDEsysReply,"%s\tNo hot link", szDDEsysReply);
                if (ddesrv.fAck)
                  sprintf(szDDEsysReply, "%s\tAcknowledgement Required", szDDEsysReply);
                else
                  sprintf(szDDEsysReply,"%s\tAcknowlegment Not Required", szDDEsysReply);
                if (ddesrv.fRefData)
                  sprintf(szDDEsysReply,"%s\tReference to data only", szDDEsysReply);
                else
                  sprintf(szDDEsysReply,"%s\tPass copy data", szDDEsysReply);
                sprintf(szDDEsysReply,"%s\tRender data as %d", szDDEsysReply,
							 ddesrv.usFormat);

                pddeOut = MakeDDEDataSeg(CF_TEXT, CV_DDESYSTOPICS,
                   (PVOID)szDDEsysReply, (USHORT)strlen(szDDEsysReply)+(USHORT)1, 0);
                WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_DATA, pddeOut, TRUE);
                break;
                }
              else
                if (strcmp(DDES_PSZITEMNAME(pddeIn), CV_DDEFORMATS) == 0)
                  {

                  /**********************************************************/
                  /* put each format number in the list, user formats should*/
                  /* be registered first with the atom table per Microsoft  */
                  /* Systems Journal Volume 4 Number 3 (May 89) pg 4-8      */
                  /**********************************************************/

                  itoa((int)CF_TEXT, szBuf, 10);
                  sprintf(szDDEsysReply, "%s", szDDEsysReply, szBuf);
                  pddeOut = MakeDDEDataSeg(CF_TEXT, CV_DDEFORMATS,
                     (PVOID)szDDEsysReply, (USHORT)strlen(szDDEsysReply)+(USHORT)1,
                     0);
                  WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_DATA, pddeOut, TRUE);
                  break;
                  }
        } // finished with "System" topics

      if (strcmp("Quotes", ddesrv.szTopic) == 0) // process application topic
        {
        if (strcmp(DDES_PSZITEMNAME(pddeIn), CV_DDEUSRITEM) == 0)
          {
            // data or message only?
          if (pddeIn->fsStatus & DDE_FNODATA) // No data required?
            cbQuote = 0;
          else
            {
            cbQuote = (USHORT) strlen(szQuote[ndx])+ (USHORT)1;
            }
          pddeOut = MakeDDEDataSeg(CF_TEXT, CV_DDEUSRITEM, (PVOID)
             szQuote[ndx], (USHORT)cbQuote, 0);
          WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_DATA, pddeOut, TRUE);
          break;
          }
        else
          {

          /******************************************************************/
          /* Data item requested isn't supported just change status bit and */
          /* pass back same segment                                         */
          /******************************************************************/

          pddeIn->fsStatus &= (~DDE_FACK);
          WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_ACK, pddeIn, TRUE);
          }
        }
      break;                           /* dde_request                       */
    case  WM_DDE_ADVISE :              /* DDE "hot" link request            */
      if (ddesrv.hwndDDEClient != (HWND)mp1)
			  break;
      pddeIn = (PDDESTRUCT)mp2;

      if (ddesrv.fAdvise)              /* if Advise already active - signal
                                          "busy"                            */
        {
        pddeIn->fsStatus &= (~DDE_FACK);
        pddeIn->fsStatus |= DDE_FBUSY;
        WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_ACK, pddeIn, TRUE);
        }
      else
        {
        if (pddeIn->usFormat == CF_TEXT)
          {
          ddesrv.fAdvise = TRUE;
          ddesrv.fConnected = TRUE;

          if (pddeIn->fsStatus&DDE_FNODATA) // client requires data or just inform
            {
            ddesrv.fRefData = TRUE;
            WinSetWindowText(hwndFrame, "...Advise - Reference data...");
            }
          else
            {
            ddesrv.fRefData = FALSE;
            WinSetWindowText(hwndFrame, "...Advise - pass copy of data...");
            }

          if (pddeIn->fsStatus&DDE_FACKREQ) // Does Client want an ACK?
            {
            ddesrv.fAck = TRUE;        /* Acknowledgement was requested     */
            pddeIn->fsStatus |= DDE_FACK;
            WinDdePostMsg(ddesrv.hwndDDEClient, hwnd, WM_DDE_ACK, pddeIn,
               TRUE);
            }
          else
            {
            ddesrv.fAck = FALSE;       /* No acknowledgement was requested  */
            }
          }                            // if
        else
          {

          /******************************************************************/
          /* Data format requested is not supported - send NAK              */
          /******************************************************************/

          pddeIn->fsStatus &= (~DDE_FACK);
          WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_ACK, pddeIn, TRUE);
          }
        }
      break;                           /* dde_advise                        */
    case  WM_DDE_UNADVISE :            /* Terminate DDE hot link request    */
      if (ddesrv.hwndDDEClient != (HWND)mp1)
			  break;
      pddeIn = (PDDESTRUCT)mp2;
      if ((mp1 == ddesrv.hwndDDEClient) && (pddeIn->usFormat == CF_TEXT))
        {
        ddesrv.fConnected = FALSE;
        ddesrv.fAdvise = FALSE;
        pddeIn->fsStatus |= DDE_FACK;
        WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_ACK, pddeIn, TRUE);
        WinSetWindowText(hwndFrame, "...Connected ...");
        }
      else
        {
        pddeIn->fsStatus &= (~DDE_FACK);
        WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_ACK, pddeIn, TRUE);
        }
      break;

    case  WM_DDE_ACK :                 /* DDE Acknowledgement               */
      if (ddesrv.hwndDDEClient != (HWND)mp1)
			  break;
      pddeIn = (PDDESTRUCT)mp2;
      if ((mp1 == ddesrv.hwndDDEClient) && (pddeIn->fsStatus & DDE_FACK))
        {
        ddesrv.fAck = TRUE;
        }
      break;

    case  WM_DDE_TERMINATE :           /* DDE conversation termination
                                          request                           */
      if (ddesrv.hwndDDEClient != (HWND)mp1)
			  break;
      ddesrv.hwndDDEClient = NULL;     // store client window handle
      ddesrv.fAdvise = FALSE;			 // terminate hot link
      WinDdePostMsg((HWND)mp1, hwnd, WM_DDE_TERMINATE, NULL, TRUE);
      ddesrv.fConnected = FALSE;
      WinSetWindowText(hwndFrame, "...DDE Server Not Connected...");
      break;

    case  WM_CLOSE : // bring down application gracefully, release resources
      if (ddesrv.fConnected)
        WinDdePostMsg(ddesrv.hwndDDEClient, hwnd, WM_DDE_TERMINATE, NULL,
           TRUE);
      if (fTimerOn)                    // release sytem timer
        WinStopTimer(hab, hwnd, ID_TIMER);
      WinPostMsg(hwnd, WM_QUIT, 0L, 0L);
      return 0;

    }
  return  WinDefWindowProc(hwnd, msg, mp1, mp2);
}

// Post - writes szMsg in hwnd centered vertically and horizontally

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
}  // PaintWindow()


// pre - format, item name, and fsStatus are valid
// post - a valid data segment for WM_DDE_DATA message is built
//       and a pddestruct is returned else (error) null is returned

PDDESTRUCT EXPENTRY MakeDDEDataSeg(USHORT usFormat,PSZ pszItemName,PVOID
                                   pvData,USHORT cbData,USHORT fsStatus)
{
  PDDESTRUCT pdde = NULL;              // ptr to dde data structure
  ULONG rc = 0;                        // api return code holder
  PULONG pulSharedObj = NULL;          // pointer to shared object
  PCHAR psz = NULL;                    // string ptr for text format
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
  /* Fill in the new DDE structure                                          */
  /**************************************************************************/

  pdde = (PDDESTRUCT) pulSharedObj;
  pdde->usFormat = usFormat;
  pdde->offszItemName = (USHORT)sizeof(DDESTRUCT); // ptr to item name
  strcpy(DDES_PSZITEMNAME(pdde), pszItemName); // item name
  pdde->cbData = cbData;               // count of bytes of user data
  pdde->offabData = (USHORT) pdde->offszItemName+ (USHORT)strlen(pszItemName)+(USHORT)1;
  pdde->fsStatus = fsStatus;           // set status flags
  if (usFormat == CF_TEXT)
    {
    if (pvData == NULL || cbData == 0)
      psz = '\0';
    else
       psz = pvData;
    strcpy(DDES_PABDATA(pdde), psz);
    }
  else
    {
	  if (pvData != NULL || cbData != 0)
       memcpy(DDES_PABDATA(pdde), pvData, (size_t)cbData);
    }
  return  pdde;

} // MakeDDEDataSeg()

/* $Workfile:   server.c  $ */


