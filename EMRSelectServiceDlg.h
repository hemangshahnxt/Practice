#if !defined(AFX_EMRSELECTSERVICEDLG_H__5512814A_E871_4D3F_B156_9B9F2A8761F6__INCLUDED_)
#define AFX_EMRSELECTSERVICEDLG_H__5512814A_E871_4D3F_B156_9B9F2A8761F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRSelectServiceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRSelectServiceDlg dialog

// (j.dinatale 2012-01-11 13:20) - PLID 47464 - enum for the columns in the resp list
enum AssignRespListCol{
	arlcInsPartyID = 0,
	arlcRespName = 1,
	arlcPlacement = 2,
	arlcPriority = 3,
};

// (b.savon 2014-02-26 10:10) - PLID 60805 - DefaultRadioSelction
enum DefaultRadioSelection{
	drsServiceCode = 0,
	drsInventoryItem,
	drsQuote,
};

class CEMRSelectServiceDlg : public CNxDialog
{
// Construction
public:
	// (b.savon 2014-02-26 10:13) - PLID 60805 - Added DefaultRadioSelction
	CEMRSelectServiceDlg(CWnd* pParent, DefaultRadioSelection = drsServiceCode);   // standard constructor

	BOOL m_bGeneric; // (a.walling 2007-04-24 08:59) - PLID 25356 - Prevent "add to bill" etc messages

	// (z.manning 2011-07-06 12:01) - PLID 44421 - Added a flag to only show CPT codes
	BOOL m_bCptCodesOnly;

	// (j.jones 2008-06-03 10:26) - PLID 30062 - added m_nPatientID, to filter quotes,
	// and m_nQuoteID, so one can be selected
	long m_nPatientID;
	long m_nQuoteID;

	long m_ServiceID;
	CString m_strCode;
	CString m_strSubCode;
	CString m_strDescription;
	COleCurrency m_cyPrice;
	// (j.jones 2011-03-28 15:20) - PLID 42575 - added m_bBillable
	BOOL m_bBillable;
	long m_nCategory;// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
	long m_nCategoryCount;

	BOOL m_bShowRespList; // (j.dinatale 2012-01-11 11:51) - PLID 47464
	long m_nAssignedInsuredPartyID; // (j.dinatale 2012-01-11 11:59) - PLID 47464

	void ChangeChargeList();

	// (j.jones 2008-06-03 10:21) - PLID 30062 - added m_radioQuotes, and renamed the others to be radios
// Dialog Data
	//{{AFX_DATA(CEMRSelectServiceDlg)
	enum { IDD = IDD_EMR_SELECT_SERVICE_DLG };
	NxButton	m_radioQuotes;
	NxButton	m_radioSvcCode;
	NxButton	m_radioProductCode;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRSelectServiceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-06-03 10:17) - PLID 30062 - added Quote combo
	NXDATALISTLib::_DNxDataListPtr m_CPTCombo, m_ProductCombo, m_QuotesCombo;

	// (j.dinatale 2012-01-11 12:16) - PLID 47464 - added resp combo
	NXDATALIST2Lib::_DNxDataListPtr m_pRespList;

	BOOL m_bProductComboLoaded;
	BOOL m_bQuoteComboLoaded;
	// (b.savon 2014-02-26 11:00) - PLID 60805 - Add CPT code because now it may/may not be the default
	BOOL m_bCPTCodeComboLoaded;

	// (b.savon 2014-02-26 10:13) - PLID 60805 - Added DefaultRadioSelction
	DefaultRadioSelection m_drsDefaultSelection;

	// (j.jones 2008-06-03 10:22) - PLID 30062 - added OnRadioQuotes
	// Generated message map functions
	//{{AFX_MSG(CEMRSelectServiceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRadioServiceCode();
	afx_msg void OnRadioProduct();
	afx_msg void OnRadioQuotes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenAssignRespToCharge(LPDISPATCH lpRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRSELECTSERVICEDLG_H__5512814A_E871_4D3F_B156_9B9F2A8761F6__INCLUDED_)
