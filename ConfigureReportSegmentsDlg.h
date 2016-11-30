#if !defined(AFX_CONFIGUREREPORTSEGMENTSDLG_H__1552092C_A309_4C30_B772_05399FA23561__INCLUDED_)
#define AFX_CONFIGUREREPORTSEGMENTSDLG_H__1552092C_A309_4C30_B772_05399FA23561__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureReportSegmentsDlg.h : header file
//
// (j.gruber 2008-07-22 10:29) - PLID 30695 - created for
/////////////////////////////////////////////////////////////////////////////
// CConfigureReportSegmentsDlg dialog
class CConfigureReportSegmentsDlg : public CNxDialog
{
// Construction
public:
	CConfigureReportSegmentsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureReportSegmentsDlg)
	enum { IDD = IDD_CONFIGURE_REPORT_SEGMENTS };
	CNxStatic	m_stSegmentType;
	NxButton	m_rdSegmentCategories;
	NxButton	m_rdSegmentProviders;
	NxButton	m_rdSegmentLocations;
	NxButton	m_rdSegmentApptTypes;
	CNxIconButton	m_btnClose;
	CNxIconButton m_btnSegmentRename;
	CNxIconButton m_btnSegmentOneRight;
	CNxIconButton m_btnSegmentOneLeft;
	CNxIconButton m_btnSegmentDelete;
	CNxIconButton m_btnSegmentAllLeft;
	CNxIconButton m_btnSegmentAdd;
	CNxIconButton m_btnSegmentAllRight;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureReportSegmentsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:


	NXDATALIST2Lib::_DNxDataListPtr m_pSegmentList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;

	long GetSegmentType();
	void MoveOneLeft(NXDATALIST2Lib::IRowSettingsPtr pSelRow);
	void MoveOneRight(NXDATALIST2Lib::IRowSettingsPtr pSelRow);

	// Generated message map functions
	//{{AFX_MSG(CConfigureReportSegmentsDlg)
	afx_msg void OnSegmentCategories();
	afx_msg void OnSegmentApptTypes();
	afx_msg void OnSegmentLocations();
	afx_msg void OnSegmentProviders();
	afx_msg void OnSegementAllRight();
	afx_msg void OnSegmentAllLeft();
	afx_msg void OnSegmentOneLeft();
	afx_msg void OnSegmentOneRight();
	afx_msg void OnSegmentAdd();
	afx_msg void OnSegmentDelete();
	afx_msg void OnSegmentRename();
	afx_msg void OnDblClickCellSegmentAvail(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellSegmentSelected(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChangingSegmentGroupList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenSegmentGroupList(LPDISPATCH lpRow);
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGUREREPORTSEGMENTSDLG_H__1552092C_A309_4C30_B772_05399FA23561__INCLUDED_)
