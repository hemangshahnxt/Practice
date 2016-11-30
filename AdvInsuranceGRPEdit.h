#if !defined(AFX_ADVINSURANCEGRPEDIT_H__C4881ACD_4A17_4E8E_9E45_2CD3E1C4C45F__INCLUDED_)
#define AFX_ADVINSURANCEGRPEDIT_H__C4881ACD_4A17_4E8E_9E45_2CD3E1C4C45F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvInsuranceGRPEdit.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdvInsuranceGRPEdit dialog

class CAdvInsuranceGRPEdit : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_UnselectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr m_UnselectedProviderList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedProviderList;

	long m_IDType; //1 - GRP, 2 - Box24J, 3 - Network ID, 4 - Box51, 5 - Box31

	void UpdateBox24J(long ProvID,long InsID,CString strNewID,CString strNewQual);
	void UpdateGRP(long ProvID,long InsID,CString strNewID);
	void UpdateNetworkID(long ProvID,long InsID,CString strNewID);
	void UpdateBox51(long ProvID,long InsID,CString strNewID);
	void UpdateBox31(long ProvID,long InsID,CString strNewID);

	CAdvInsuranceGRPEdit(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAdvInsuranceGRPEdit)
	enum { IDD = IDD_ADV_INSURANCE_GRP_EDIT };
	NxButton	m_btnGrp;
	NxButton	m_btn24J;
	NxButton	m_btnBox31;
	NxButton	m_btnNetworkID;
	NxButton	m_btnBox51;
	CNxIconButton	m_btnUnselOneProv;
	CNxIconButton	m_btnUnselOneInsco;
	CNxIconButton	m_btnUnselAllProv;
	CNxIconButton	m_btnUnselAllInsco;
	CNxIconButton	m_btnSelAllProv;
	CNxIconButton	m_btnSelAllInsco;
	CNxIconButton	m_btnSelOneProv;
	CNxIconButton	m_btnSelOneInsco;
	CNxEdit	m_nxeditEditBox24iQual;
	CNxEdit	m_nxeditNewIdNumber;
	CNxStatic	m_nxstaticBox24iQualLabel;
	CNxStatic	m_nxstaticIdLabel;
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnOK;
	NxButton	m_btnInsProvGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvInsuranceGRPEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void ShowBox24IInfo(BOOL bShow);

	// Generated message map functions
	//{{AFX_MSG(CAdvInsuranceGRPEdit)
	afx_msg void OnRadioBox24J();
	afx_msg void OnRadioGrp();
	afx_msg void OnDblClickCellUnselectedInsList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedInsList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselectedProvidersList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedProvidersList(long nRowIndex, short nColIndex);
	afx_msg void OnSelectOneInsco();
	afx_msg void OnSelectAllInsco();
	afx_msg void OnUnselectOneInsco();
	afx_msg void OnUnselectAllInsco();
	afx_msg void OnSelectOneProv();
	afx_msg void OnSelectAllProv();
	afx_msg void OnUnselectOneProv();
	afx_msg void OnUnselectAllProv();
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	afx_msg void OnRadioNetworkid();
	afx_msg void OnRadioBox51();
	afx_msg void OnRadioBox31();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVINSURANCEGRPEDIT_H__C4881ACD_4A17_4E8E_9E45_2CD3E1C4C45F__INCLUDED_)
