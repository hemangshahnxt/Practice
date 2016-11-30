#if !defined(AFX_SELECTAPPTDLG_H__07A676B8_6DD7_47B1_B0F6_81184CE2EA4F__INCLUDED_)
#define AFX_SELECTAPPTDLG_H__07A676B8_6DD7_47B1_B0F6_81184CE2EA4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectApptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectApptDlg dialog

class CSelectApptDlg : public CNxDialog
{
// Construction
public:
	CSelectApptDlg(CWnd* pParent);   // standard constructor
	CString m_strWhere;
	CArray<int, int> m_arSelectedIds;

	BOOL m_bAllowMultiSelect;

	// (j.jones 2008-03-19 10:32) - PLID 29324 - added ability to show a patient column,
	// and to select no appt. at all
	BOOL m_bShowPatientName;
	BOOL m_bShowNoneSelectedRow;

	CString m_strLabel;

// Dialog Data
	//{{AFX_DATA(CSelectApptDlg)
	enum { IDD = IDD_SELECT_APPT };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticSelectApptLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectApptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pApptList;

	// Generated message map functions
	//{{AFX_MSG(CSelectApptDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRequeryFinishedApptList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTAPPTDLG_H__07A676B8_6DD7_47B1_B0F6_81184CE2EA4F__INCLUDED_)
