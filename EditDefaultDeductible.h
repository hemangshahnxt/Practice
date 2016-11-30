#pragma once

// CEditDefaultDeductible dialog
//Created for PLID 50636 by r.wilson 8/13/2012

class CEditDefaultDeductible : public CNxDialog
{
	
	DECLARE_DYNAMIC(CEditDefaultDeductible)

public:
	CEditDefaultDeductible(CWnd* pParent = NULL);   // standard constructor
	virtual BOOL OnInitDialog();
	virtual ~CEditDefaultDeductible();
	
	OLE_COLOR m_nColor;
	CNxColor m_nxcTop;
	CNxColor m_nxcBottom;
		
	// Set to true if the dialog is initializing itself
	BOOL m_bDialogInitializing;
	BOOL m_bNeedsToSave;
	
	long m_nInsuranceCoID;
	NXDATALIST2Lib::_DNxDataListPtr m_InsuranceCo;
	NXDATALIST2Lib::_DNxDataListPtr m_PayGroupsList;
	
	BOOL m_bPerPayGroup;
	CString m_strDefaultTotalDeductible;
	CString m_strDefaultTotalOOP;
	
	NxButton m_radioAllPaygroups;
	NxButton m_radioIndividualPaygroups;

	CNxEdit m_nxeditTotalDeductible;
	CNxEdit m_nxeditTotalOOP;

	CNxIconButton m_nxibOK;
	CNxIconButton m_nxibCancel;


// Dialog Data
	enum { IDD = IDD_EDIT_DEFAULT_DEDUCTIBLE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void CEditDefaultDeductible::SetMemberVariables();
	void CEditDefaultDeductible::FormatDataListCols();

	DECLARE_MESSAGE_MAP()

private:
	void CEditDefaultDeductible::UpdatePayGroupsList();
	void CEditDefaultDeductible::RadioButtonLogic();
	BOOLEAN CEditDefaultDeductible::Save();

public:
	afx_msg void OnBnClickedAllPayGroupsRadio();
	afx_msg void OnBnClickedIndividualPayGroupsRadio();
	DECLARE_EVENTSINK_MAP()
	void SelChangedInsuranceCoDatalist(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void EditingFinishingPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);

	
	afx_msg void OnBnClickedOkButton();
	afx_msg void OnBnClickedCancelButton();
	afx_msg void OnEnKillfocusTotalDeductibleEdit();
	afx_msg void OnEnKillfocusTotalOopEdit();
	void EditingStartingPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void SelChangingInsuranceCoDatalist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RequeryFinishedPayGroupList(short nFlags);
};
