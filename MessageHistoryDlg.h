#if !defined(AFX_MESSAGEHISTORYDLG_H__3217EA72_15BA_457E_89B5_8ABD32F72CD6__INCLUDED_)
#define AFX_MESSAGEHISTORYDLG_H__3217EA72_15BA_457E_89B5_8ABD32F72CD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessageHistoryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessageHistoryDlg dialog
#define ID_GOTO_MESSAGE	32768
class CMessageHistoryDlg : public CNxDialog
{
// Construction
public:
	CMessageHistoryDlg(CWnd* pParent);   // standard constructor
	long m_nOtherUserID;
	CString m_strOtherUserName;
	void SetOtherUserID(const IN long nOtherUserID);
	CMessagerDlg *m_pMainYakDlg;
	
// Dialog Data
	//{{AFX_DATA(CMessageHistoryDlg)
	enum { IDD = IDD_MESSAGE_HISTORY };
	CNxIconButton	m_btnClose;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_pMessages;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageHistoryDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	
protected:

	void SetMessageColors(); 
	void Load();
	
	// Generated message map functions
	//{{AFX_MSG(CMessageHistoryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedMessageList(short nFlags);
	afx_msg void OnDblClickCellMessageList(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonUpMessageList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGotoMessage();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGEHISTORYDLG_H__3217EA72_15BA_457E_89B5_8ABD32F72CD6__INCLUDED_)
