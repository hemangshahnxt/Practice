#if !defined(AFX_GOSTATEMENTS_H__A5B65604_DC6A_11D2_B68F_0000C0832801__INCLUDED_)
#define AFX_GOSTATEMENTS_H__A5B65604_DC6A_11D2_B68F_0000C0832801__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GoStatements.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CGoStatements dialog

class CGoStatements : public CNxDialog
{
// Construction
public:
	CGoStatements(CWnd* pParent);   // standard constructor
	void ResetReportList();
	
	long m_nPatientID;

// Dialog Data
	// (a.walling 2008-04-02 16:01) - PLID 29497 - Added NxButtons
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CGoStatements)
	enum { IDD = IDD_LAUNCHSTATEMENTS };
	NxButton	m_btnByPatient;
	NxButton	m_btnByLocation;
	NxButton	m_btnByProvider;
	NxButton	m_billCheck;
	NxButton	m_dateCheck;
	NxButton	m_locationCheck;
	NxButton	m_detailedRadio;
	NxButton	m_summaryRadio;
	CDateTimePicker	m_to;
	CDateTimePicker	m_from;
	CNxStatic	m_nxstaticFromLabel;
	CNxStatic	m_nxstaticToLabel;
	CNxIconButton	m_btnStmtPreview;
	CNxIconButton	m_btnRunConfig;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoStatements)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

	NXDATALISTLib::_DNxDataListPtr m_ptrLocations;
	NXDATALISTLib::_DNxDataListPtr m_ptrBills;
	NXDATALISTLib::_DNxDataListPtr m_pReportList;
	// (j.gruber 2009-02-17 14:56) - PLID 32534 - add Date type filter
	NXDATALIST2Lib::_DNxDataListPtr m_pDateFilterList;
	long m_nLocID;
	long m_nBillID;
	

protected:
	CBrush m_brush;
	// Generated message map functions
	//{{AFX_MSG(CGoStatements)
	afx_msg void OnRunConfig();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickPreview();
	afx_msg void OnCancel();
	afx_msg void OnDetailed();
	afx_msg void OnSummary();
	afx_msg void OnTransaction();
	afx_msg void OnLocfilterchk();
	afx_msg void OnBillFilterCheck();
	afx_msg void OnBylocation();
	afx_msg void OnBypatient();
	afx_msg void OnByprovider();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:

	void SelChangingDateFilterTypeList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOSTATEMENTS_H__A5B65604_DC6A_11D2_B68F_0000C0832801__INCLUDED_)
