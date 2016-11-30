#if !defined(AFX_MEDINOTESLINK_H__A559E5B1_9ECD_4D49_A9B6_EE13ACEEB16D__INCLUDED_)
#define AFX_MEDINOTESLINK_H__A559E5B1_9ECD_4D49_A9B6_EE13ACEEB16D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MediNotesLink.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMediNotesLink dialog

class CMediNotesLink : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_nextechList, m_exportList;

	void UpdateCount();
	CString GetExportString(long PatientID);

	CMediNotesLink(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMediNotesLink)
	enum { IDD = IDD_MEDINOTES_LINK };
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnPracRemoveAll;
	CNxIconButton	m_btnPracRemove;
	CNxIconButton	m_btnPracAdd;
	CNxStatic	m_nxstaticNextechCount;
	CNxStatic	m_nxstaticNextechCount2;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMediNotesLink)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMediNotesLink)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellNextech(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellExport(long nRowIndex, short nColIndex);
	afx_msg void OnExportBtn();
	afx_msg void OnPracAdd();
	afx_msg void OnPracRemove();
	afx_msg void OnPracRemoveAll();
	afx_msg void OnRequeryFinishedNextech(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDINOTESLINK_H__A559E5B1_9ECD_4D49_A9B6_EE13ACEEB16D__INCLUDED_)
