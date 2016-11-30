#if !defined(AFX_HL7CONFIGLOCATIONSDLG_H__D60C9874_D651_4981_A429_18725E73C344__INCLUDED_)
#define AFX_HL7CONFIGLOCATIONSDLG_H__D60C9874_D651_4981_A429_18725E73C344__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7ConfigLocationsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHL7ConfigLocationsDlg dialog

class CHL7ConfigLocationsDlg : public CNxDialog
{
// Construction
public:
	CHL7ConfigLocationsDlg(CWnd* pParent = NULL);   // standard constructor

	long m_nHL7GroupID;

	// (j.jones 2008-05-08 09:38) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CHL7ConfigLocationsDlg)
	enum { IDD = IDD_HL7_CONFIG_LOCATIONS };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7ConfigLocationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;

	// (j.jones 2010-05-12 09:25) - PLID 36527 - for auditing,
	// cache the group name and the initial values
	CString m_strHL7GroupName;
	CStringArray m_arystrOldCodes;
	CMap<CString, LPCTSTR, long, long> m_mapOldCodesToLocationIDs;

	// Generated message map functions
	//{{AFX_MSG(CHL7ConfigLocationsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddHl7Location();
	afx_msg void OnRemoveHl7Location();
	virtual void OnOK();
	afx_msg void OnSelChangedLocationMap(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRButtonUpLocationMap(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingLocationMap(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7CONFIGLOCATIONSDLG_H__D60C9874_D651_4981_A429_18725E73C344__INCLUDED_)
