#pragma once

// CBlankAssignmentBenefitsDlg dialog

// (j.jones 2010-07-26 11:45) - PLID 34105 - created

#include "FinancialRc.h"

// (j.jones 2010-07-27 09:23) - PLID 39854 - created an enum
// for whether we want to validate HCFA setup, UB setup, or both
enum BlankAssignmentBenefitsType
{
	babtHCFA = 0,
	babtUB,
	babtBothForms,
};

// (j.jones 2010-07-23 14:09) - PLID 34105 - this function returns true/false if at least one
// HCFA group exists that can send claims with the "Assignments Of Benefits" not filled
// (j.jones 2010-07-27 09:19) - PLID 39854 - added an enum parameter, and moved from
// GlobalFinancialUtils to this class, it's global still, you just have to include this
// class to use it, both for using enums, and to remind developers that you shouldn't call
// this function without providing the ability to open this dialog
BOOL CanAssignmentOfBenefitsBeBlank(BlankAssignmentBenefitsType babtType = babtBothForms);

class CBlankAssignmentBenefitsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBlankAssignmentBenefitsDlg)

public:
	CBlankAssignmentBenefitsDlg(CWnd* pParent);   // standard constructor
	virtual ~CBlankAssignmentBenefitsDlg();

	// (j.jones 2010-07-27 09:24) - PLID 39854 - take in a filter for
	// whether we want to validate HCFA setup, UB setup, or both
	BlankAssignmentBenefitsType m_babtType;

// Dialog Data
	enum { IDD = IDD_BLANK_ASSIGNMENT_BENEFITS_DLG };
	CNxIconButton	m_btnClose;
	// (j.jones 2010-07-27 09:55) - PLID 39894 - now the labels can be toggled to show/hide
	CNxStatic		m_nxstaticHCFALabel;
	CNxStatic		m_nxstaticUBLabel;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
};
