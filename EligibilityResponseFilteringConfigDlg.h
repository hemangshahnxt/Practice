#pragma once

// CEligibilityResponseFilteringConfigDlg dialog

// (j.jones 2010-03-25 17:40) - PLID 37905 - created

#include "FinancialRc.h"

class CEligibilityResponseFilteringConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEligibilityResponseFilteringConfigDlg)

public:
	CEligibilityResponseFilteringConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CEligibilityResponseFilteringConfigDlg();

// Dialog Data
	enum { IDD = IDD_ELIGIBILITY_RESPONSE_FILTERING_CONFIG_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSelectOneServiceType;
	CNxIconButton	m_btnSelectAllServiceType;
	CNxIconButton	m_btnUnselectOneServiceType;
	CNxIconButton	m_btnUnselectAllServiceType;
	CNxIconButton	m_btnSelectOneCoverageLevel;
	CNxIconButton	m_btnSelectAllCoverageLevel;
	CNxIconButton	m_btnUnselectOneCoverageLevel;
	CNxIconButton	m_btnUnselectAllCoverageLevel;
	CNxIconButton	m_btnSelectOneBenefitType;
	CNxIconButton	m_btnSelectAllBenefitTypes;
	CNxIconButton	m_btnUnselectOneBenefitType;
	CNxIconButton	m_btnUnselectAllBenefitType;
	// (j.jones 2010-04-19 09:56) - PLID 38202 - added option to exclude from the output file
	NxButton		m_checkExcludeFromOutputFile;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedServiceTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedServiceTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedCoverageLevelList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedCoverageLevelList;
	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedBenefitTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedBenefitTypeList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellUnselectedServicetypeList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedServicetypeList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellUnselectedCoveragelevelList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedCoveragelevelList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellUnselectedBenefittypeList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedBenefittypeList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnSelectOneTos();
	afx_msg void OnBtnSelectAllTos();
	afx_msg void OnBtnUnselectOneTos();
	afx_msg void OnBtnUnselectAllTos();
	afx_msg void OnBtnSelectOneCoverage();
	afx_msg void OnBtnSelectAllCoverage();
	afx_msg void OnBtnUnselectOneCoverage();
	afx_msg void OnBtnUnselectAllCoverage();
	afx_msg void OnBtnSelectOneBenefit();
	afx_msg void OnBtnSelectAllBenefit();
	afx_msg void OnBtnUnselectOneBenefit();
	afx_msg void OnBtnUnselectAllBenefit();
};
