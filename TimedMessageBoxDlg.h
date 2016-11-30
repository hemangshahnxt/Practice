#pragma once


// CTimedMessageBoxDlg dialog
// (d.thompson 2009-07-08) - PLID 34809 - Created.
//See also:  http://www.codeproject.com/KB/dialog/xmessagebox.aspx
//	This is a really nice general type dialog that would be handy.  We should consider doing something
//	like that, or see if that is usable in Practice.  For now, this dialog only supports ok/cancel, 
//	and auto timeout.

#include "PracticeRc.h"

class CTimedMessageBoxDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CTimedMessageBoxDlg)

public:
	CTimedMessageBoxDlg(CWnd* pParent);   // standard constructor
	virtual ~CTimedMessageBoxDlg();

// Dialog Data
	enum { IDD = IDD_TIMED_MESSAGE_BOX_DLG };

	//Dialog Options
	bool m_bAllowTimeout;
	unsigned long m_nTimeoutSeconds;
	bool m_bShowCancel;
	CString m_strMessageText;

protected:
	//Timer data
	DWORD m_dwStartTime;

	//Controls
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_staticUserMessage;
	CNxStatic m_staticDestroyMessage;

	//Functionality
	void EnableOptionedButtons();
	void EnableTimer();
	void ReformatDisplay();
	void UpdateTimerMessage();

	//Interface messages
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
