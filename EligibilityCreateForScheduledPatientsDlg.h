#pragma once

// CEligibilityCreateForScheduledPatientsDlg dialog

// (j.jones 2009-09-15 17:37) - PLID 26481 - created

#include "FinancialRc.h"

class CEligibilityCreateForScheduledPatientsDlg : public CNxDialog
{

public:
	CEligibilityCreateForScheduledPatientsDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2010-07-08 08:58) - PLID 39515 - allow passing in which format to export in
	long m_nFormatID;

// Dialog Data
	enum { IDD = IDD_ELIGIBILITY_CREATE_FOR_SCHED_PATIENTS_DLG };
	CNxIconButton m_btnCreateRequests;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnSelectOneApptType;
	CNxIconButton m_btnSelectAllApptTypes;
	CNxIconButton m_btnUnselectOneApptType;
	CNxIconButton m_btnUnselectAllApptTypes;
	CNxIconButton m_btnSelectOneResource;
	CNxIconButton m_btnSelectAllResources;
	CNxIconButton m_btnUnselectOneResource;
	CNxIconButton m_btnUnselectAllResources;
	CNxIconButton m_btnSelectOneInsco;
	CNxIconButton m_btnSelectAllInscos;
	CNxIconButton m_btnUnselectOneInsco;
	CNxIconButton m_btnUnselectAllInscos;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	NxButton	m_radioUseApptResourceProvider;
	NxButton	m_radioUseG1Provider;
	NxButton	m_radioUseApptLocation;
	NxButton	m_radioUseG2Location;
	NxButton	m_radioUseCurLocation;
	NxButton	m_radioAllInsuredParties;
	NxButton	m_radioPrimaryInsuredParties;
	NxButton	m_checkUseG2DiagCodes;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo,
		m_UnselectedApptTypeList, m_SelectedApptTypeList,
		m_UnselectedResourceList, m_SelectedResourceList,
		m_UnselectedInsCoList, m_SelectedInsCoList,
		m_BenefitCategoryCombo;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBtnClose();
	afx_msg void OnBtnCreateRequests();
	afx_msg void OnBtnSelectOneApptType();
	afx_msg void OnBtnSelectAllApptTypes();
	afx_msg void OnBtnUnselectOneApptType();
	afx_msg void OnBtnUnselectAllApptTypes();
	afx_msg void OnBtnSelectOneResource();
	afx_msg void OnBtnSelectAllResources();
	afx_msg void OnBtnUnselectOneResource();
	afx_msg void OnBtnUnselectAllResources();
	afx_msg void OnBtnSelectOneInsco();
	afx_msg void OnBtnSelectAllInscos();
	afx_msg void OnBtnUnselectOneInsco();
	afx_msg void OnBtnUnselectAllInscos();	
	void OnDblClickCellApptTypeUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellApptTypeSelectedList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellApptResourceUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellApptResourceSelectedList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellInscosUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellInscosSelectedList(LPDISPATCH lpRow, short nColIndex);
	void OnSelChosenApptLocationFilter(LPDISPATCH lpRow);
	void OnSelChosenSchedCategoryCombo(LPDISPATCH lpRow);
};
