// ShowNxErrorFeedbackDlg.h: interface for the CShowNxErrorFeedbackDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHOWNXERRORFEEDBACKDLG_H__A54B2084_4469_438A_8724_9A706582A9BB__INCLUDED_)
#define AFX_SHOWNXERRORFEEDBACKDLG_H__A54B2084_4469_438A_8724_9A706582A9BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CShowNxErrorFeedbackDlg  
{
public:
	CShowNxErrorFeedbackDlg(const CString &message, const CString &title, ErrorLevel level);
	~CShowNxErrorFeedbackDlg();

public:
	BOOL UserAborted();
	// (a.walling 2010-08-04 16:37) - PLID 38964 - This blocks until one of our events are set. 
	void WaitForUser();

public:
	HANDLE m_hEventWaitForTerm;
	HANDLE m_hEventUserAbort;
	// (a.walling 2010-08-04 16:28) - PLID 38964
	HANDLE m_hEventUserContinue;
	CWinThread *m_pThread;

public:
	CString m_strMessage;
	CString m_strTitle;
	ErrorLevel m_level;
};

#endif // !defined(AFX_SHOWNXERRORFEEDBACKDLG_H__A54B2084_4469_438A_8724_9A706582A9BB__INCLUDED_)
