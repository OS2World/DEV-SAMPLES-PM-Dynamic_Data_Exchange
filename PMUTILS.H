 /*
  $Workfile:   pmutils.h  $
  $Revision:   1.1.1.0  $
  $Date:   09 May 1991 18:10:22  $
  $Logfile:   W:/dde/pmutils.h_v  $
  $Author:   mikem  $
  $Modtime:   09 May 1991 18:07:04  $
*/

/* used to grey-out or enable a action bar menu item                  */
#define EnableMenuItem(hwndMenu, idMenuItem, fEnable) \
	  ( WinSendMsg(hwndMenu, MM_SETITEMATTR, 	  \
              MPFROM2SHORT (idMenuItem, TRUE),	\
              MPFROM2SHORT (MIA_DISABLED, fEnable ? 0 : MIA_DISABLED)) )

#define CheckMenuItem(hwndMenu, idMenuItem, fChecked) \
 		( WinSendMsg(hwndMenu, MM_SETITEMATTR, \
   			  MPFROM2SHORT(idMenuItem, TRUE), \
	           MPFROM2SHORT(MIA_CHECKED, fChecked ? MIA_CHECKED : 0)) )

#define QCheckMenuItem(hwndMenu, idMenuItem) \
 		( (BOOL) WinSendMsg(hwndMenu, MM_QUERYITEMATTR, \
		   			  MPFROM2SHORT(idMenuItem, TRUE),	\
			           MPFROMSHORT(MIA_CHECKED)) )

#define CopyToClipboard(hwndMLE) \
		( (ULONG) WinSendMsg (hwndMLE, MLM_COPY, \
                            MPFROMSHORT(NULL),	\
	   	                    MPFROMSHORT(NULL)) )

#define EnableMLEWordWrap(hwndMLE, fWrapOn) \
		((MRESULT) WinSendMsg(hwndMLE, MLM_SETWRAP, \
   			  MPFROMSHORT(fWrapOn ? TRUE : FALSE),	\
	           MPFROMSHORT(NULL)) ) 

#define SetEFTextLength(hwndDlg, id, size) \
		(	WinSendDlgItemMsg(hwndDlg, id, EM_SETTEXTLIMIT, \
          MPFROM2SHORT(size,0), (MPARAM) NULL)) 

/* function declarations  */
BOOL GetWinPos(PSZ pszApplName, HINI hiniProfile, PSWP swpWinPos);
BOOL SetWinPos(PCHAR pszApplName, HINI hiniProfile, HWND hwndFrame);
VOID SetHelpStruct(PHELPINIT hmiHelpData, PSZ pszHLPfileName, PSZ pszHlpWinTitle, INT ridHlpTbl);
VOID CenterDlg(HWND hwndDlg);
BOOL	DeleteMLE(PHWND phwndMLE);
BOOL	CreateMLE(HWND hParent, HWND hOwner, PHWND phwndMLE, USHORT id, PRECTL prcl);
MRESULT EXPENTRY FontDlgProc(HWND, USHORT, MRESULT, MRESULT);
BOOL InitFontDlg(HWND hwndDlg);
VOID SelFont(HWND hwndDlg);
VOID DisplayErrMsg(HAB hab, HWND hwndOwner, PSZ pszCaption, SHORT sErrID, SHORT sMsgWinID);
VOID CenterDlg(HWND hwndDlg);
MRESULT EXPENTRY AboutDlgProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2);


