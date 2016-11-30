#pragma once
#include <NxUILib/NxStaticIcon.h> //(r.wilson 4/22/2014) PLID 61828 -


// CConfigureProviderTypesDlg dialog
//(r.wilson 4/22/2014) PLID 61826 - Created Dialog

class CConfigureProviderTypesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureProviderTypesDlg)

public:
	CConfigureProviderTypesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigureProviderTypesDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_PROVIDER_TYPES };

private:
	struct ProvidersSetTypes
	{
	//	long Id;

		BOOL Optician;
		BOOL AffiliateProvider;
		BOOL ReferringProvider;
		BOOL OrderingProvider;
		BOOL SupervisingProvider;

		ProvidersSetTypes(/*long id,*/ BOOL bOptician, BOOL bAffiliateProvider, BOOL bReferringProvider, BOOL bOrderingProvider, BOOL bSupervisingProvider)
		{
			Optician = bOptician;
			AffiliateProvider = bAffiliateProvider;
			ReferringProvider = bReferringProvider;
			OrderingProvider = bOrderingProvider;
			SupervisingProvider = bSupervisingProvider;
		}

	};

	CString mode;
protected:
	
	
	enum ProviderTypes{
			eptProvider = 0,
			eptReferringPhysician,
		};
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_DL_ProviderTypes;
	NXDATALIST2Lib::_DNxDataListPtr m_DL_SelectedProviders;
	NXDATALIST2Lib::_DNxDataListPtr m_DL_UnselectedProviders;;

	CNxIconButton m_btnSelectAll;
	CNxIconButton m_btnSelectOne;
	CNxIconButton m_btnUnselectOne;
	CNxIconButton m_btnUnselectAll;
	
	NxButton m_cbAffPhy_Optician;
	NxButton m_cbReferringProvider;
	NxButton m_cbOrderingProvider;
	NxButton m_cbSupervisingProvider;

	CNxIconButton m_btnClearTypes;
	CNxIconButton m_btnApply;
	CNxIconButton m_btnClose;

	CNxStaticIcon m_icoConfigProvTypeInfo;

	long m_nProvTypeCurentId;

	

	std::map<long, ProvidersSetTypes> m_listOldProviderTypes;
	std::map<long, ProvidersSetTypes> m_listOldReferringPhysicianTypes;

	
	void ClearSelectedProvidersDatalist();
	virtual BOOL OnInitDialog();
	void PopulateUnselectedDatalist();
	void UpdateSelectedDatalist();
	ProviderTypes GetSelectedProviderType();
	void PopulateDataLists();
	void UncheckAllBoxes();
	void CreateUpdateSQL(BOOL bupdate);
	void HideOrShowOpticianCB();
	void CreateAndSetToolTipText();
	void AuditChanges(BOOL bupdate);
	void UpdateSelectBox();
	//void FormatUpdateString(CString strtable, CString );
	void AuditField(long nTransactionAuditID,CString ContactName, AuditEventItems aeiItem, CString strOld, CString strNew, AuditEventPriorities aepPriority = aepMedium, AuditEventTypes aetType = aetChanged);








	DECLARE_MESSAGE_MAP()
public:
	CArray<long> arrID;
	CString strCurSelContactType;
	ProviderTypes CurSelContactType;
	afx_msg void OnBnClickedBtnSelOne();
	
	afx_msg void OnBnClickedBtnSelAll();
	afx_msg void OnBnClickedBtnUnselOne();
	afx_msg void OnBnClickedBtnUnselAll();
	DECLARE_EVENTSINK_MAP()
	void SelChosenDlProviderType(LPDISPATCH lpRow);
	afx_msg void OnBnClickedBtnCloseCptDlg();
	afx_msg void OnBnClickedBtnClearTypes();
	afx_msg void OnBnClickedBtnApplyChanges();
//	void SelChangedDlProviderType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);

	

	void DblClickCellDlUnselectedProviders(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellDlSelectedProviders(LPDISPATCH lpRow, short nColIndex);
};
