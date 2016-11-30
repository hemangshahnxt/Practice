#include "PracticeRc.h"
#pragma once


// CConfigurePrimaryEMNProviderDlg dialog
//(r.wilson 7/29/2013) PLID 48684 - Created Dialog

class CConfigurePrimaryEMNProviderDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigurePrimaryEMNProviderDlg)

public:
	CConfigurePrimaryEMNProviderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigurePrimaryEMNProviderDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_PRIMARY_EMR_PROVIDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void RefreshLinkedToProviderList();
	void PopulateLinkedProviderList();
	CString CConfigurePrimaryEMNProviderDlg::CreateLicensedProviderInClauseSqlSnippet(BOOL bIN);
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedProviderList;
	NXDATALIST2Lib::_DNxDataListPtr m_LinkedToProviderList;
	CNxIconButton	m_btnClose;		

	CDWordArray m_dwaLicensedProvIDs;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenLinkedToProviderDatalist(LPDISPATCH lpRow);
	void SelChosenSelectedProviderDatalist(LPDISPATCH lpRow);
	void SelChangingSelectedProviderDatalist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingLinkedToProviderDatalist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
