#if !defined(AFX_NOTIFICATIONDLG_H__A391C86B_2748_4C4F_B8FE_D4B8391E2CE6__INCLUDED_)
#define AFX_NOTIFICATIONDLG_H__A391C86B_2748_4C4F_B8FE_D4B8391E2CE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NotificationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNotificationDlg dialog
struct Notification
{
	int nNotificationType;  //What type of notification?
	CString strMessage;		//What is the message for this notification? (can include #NOTIFICATION_COUNT#)
	int nCount;				//How many instances of this notification are there?
	bool bIncrement;		//Should nCount be added to any existing nCounts, or override them?
};

class CNotificationDlg : public CNxDialog
{
// Construction
public:
	CNotificationDlg(CWnd* pParent);   // standard constructor

	void AddNotification(Notification stNotification);
	//Clears all notifications of the given type (or all notifications if dwTypesToClear == -1), and hides the dialog if there are
	//no more notifications.
	void ClearNotifications(DWORD dwTypesToClear = -1);
	void ReleaseNotifications(); //Same as if user clicked on dialog.

// Dialog Data
	//{{AFX_DATA(CNotificationDlg)
	enum { IDD = IDD_NOTIFICATION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotificationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strCaption;
	BOOL m_bWindowClicked, m_bMouseOver;

	CArray<Notification, Notification&> m_arNotifications;

// (b.cardillo 2005-03-03 16:46) - PLID 15827 - Made the refresh function public so it can be 
// calld by anyone when necessary, and then added HasNotifications() so anyone can tell whether 
// it needs to be called.
public:
	//Regenerates the message.
	void Refresh();
	// (j.jones 2011-03-15 15:28) - PLID 42738 - added a function to see if
	// any notifications exist of a given type, and renamed the "all" function
	// for clarity
	BOOL HasNotification(int nNotificationType);
	BOOL HasAnyNotifications();

protected:
	
	// Generated message map functions
	//{{AFX_MSG(CNotificationDlg)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOTIFICATIONDLG_H__A391C86B_2748_4C4F_B8FE_D4B8391E2CE6__INCLUDED_)
