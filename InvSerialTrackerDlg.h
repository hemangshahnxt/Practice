#if !defined(AFX_INVSERIALTRACKERDLG_H__BD709812_3F60_4026_B86B_B04E89AA453D__INCLUDED_)
#define AFX_INVSERIALTRACKERDLG_H__BD709812_3F60_4026_B86B_B04E89AA453D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvSerialTrackerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvSerialTrackerDlg dialog

#define AllSerialNumsValue "<All Serial Numbers>"
#define AllPatientsValue "<All Patients>"
#define AllProductsValue "<All Products>"
#define AllCategoriesValue "<All Categories>"

class CInvSerialTrackerDlg : public CNxDialog
{
// Construction
public:
	CInvSerialTrackerDlg(CWnd* pParent);   // standard constructor

	long m_nDefaultPatientID;

// Dialog Data
	//{{AFX_DATA(CInvSerialTrackerDlg)
	enum { IDD = IDD_SERIAL_TRACKER };
	// (j.jones 2007-11-13 15:00) - PLID 28081 - added ability to filter on Consignment products
	NxButton	m_checkConsignment;
	NxButton	m_btnFilterDate;
	COleDateTime	m_dtEnd;
	COleDateTime	m_dtStart;
	CNxIconButton	m_btnOK;
	BOOL	m_bFilterOnDates;
	// (j.jones 2007-11-13 15:00) - PLID 28081 - added ability to filter on Consignment products
	BOOL	m_bFilterOnConsignment;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvSerialTrackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_CategoryCombo;
	NXDATALISTLib::_DNxDataListPtr m_ProductCombo;
	NXDATALISTLib::_DNxDataListPtr m_PatientCombo;
	NXDATALISTLib::_DNxDataListPtr m_SerialCombo;
	NXDATALISTLib::_DNxDataListPtr m_SerialList;
	NXDATALISTLib::_DNxDataListPtr m_StatusCombo;		//DRT 11/28/2007 - PLID 28215

	_variant_t m_vtNull; //used to define a null variant for defaults

	void Refilter(NXDATALISTLib::_DNxDataListPtr dlFinishedRequerying = NULL);

	// (j.jones 2007-11-13 15:00) - PLID 28081 - added ability to filter on Consignment products
	// Generated message map functions
	//{{AFX_MSG(CInvSerialTrackerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenPatientCombo(long nRow);
	afx_msg void OnDatetimechangeStartdate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEnddate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenCategoryList(long nRow);
	afx_msg void OnSelChosenItem(long nRow);
	afx_msg void OnRequeryFinishedCategoryList(short nFlags);
	afx_msg void OnRequeryFinishedItem(short nFlags);
	afx_msg void OnRequeryFinishedPatientCombo(short nFlags);
	afx_msg void OnRequeryFinishedSerialCombo(short nFlags);
	afx_msg void OnSelChosenSerialCombo(long nRow);
	afx_msg void OnCheckFilterOnDates();
	afx_msg void OnDblClickCellListSerialized(long nRowIndex, short nColIndex);
	afx_msg void OnCheckFilterConsignment();
	afx_msg void OnSelChosenTrackerStatusList(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVSERIALTRACKERDLG_H__BD709812_3F60_4026_B86B_B04E89AA453D__INCLUDED_)
