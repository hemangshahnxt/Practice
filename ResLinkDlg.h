#if !defined(AFX_RESLINKDLG_H__226970F0_F167_48D4_A3D1_95330E30EF79__INCLUDED_)
#define AFX_RESLINKDLG_H__226970F0_F167_48D4_A3D1_95330E30EF79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResLinkDlg.h : header file
//

#include "schedulerrc.h"

/////////////////////////////////////////////////////////////////////////////
// CResLinkDlg dialog

class CResLinkDlg : public CNxDialog
{
// Construction
public:
	int Open(long nResID, long nPatientID);
	void Load();
	CResLinkDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResLinkDlg)
	enum { IDD = IDD_RES_LINK_DLG };
		// NOTE: the ClassWizard will add data members here
	NxButton	m_btnGroupedGroupbox;
	NxButton	m_btnNotGroupedGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResLinkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr	m_listGrouped, m_listUngrouped;

	long m_nResID;
	long m_nPatientID;

	void SetDays(bool bGroup);
	void SetAppointmentColors(bool bGroup);

	//map id's to true/false - true means it is added to the group, false means removed
	CMap<long, long, bool, bool> m_mapChanged;

	// Generated message map functions
	//{{AFX_MSG(CResLinkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnCancel();
	afx_msg void OnOK();
	afx_msg void OnSelectOne();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselectOne();
	afx_msg void OnUnselectAll();
	afx_msg void OnRequeryFinishedGroupedList(short nFlags);
	afx_msg void OnRequeryFinishedUngroupedList(short nFlags);
	afx_msg void OnDblClickGrouped(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickUngrouped(long nRowIndex, short nColIndex);
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESLINKDLG_H__226970F0_F167_48D4_A3D1_95330E30EF79__INCLUDED_)
