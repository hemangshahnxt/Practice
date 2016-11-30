#pragma once

// CBillClaimFieldsDlg dialog

// (j.jones 2013-08-13 11:11) - PLID 57902 - created
// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg

#include "BillingRc.h"

enum FieldType;

class CBillClaimFieldsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillClaimFieldsDlg)

public:
	CBillClaimFieldsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBillClaimFieldsDlg();

	bool m_bReadOnly;
	bool m_bUseNewForm; //true if using the new HCFA	

	CString m_strHCFABox8;
	CString m_strHCFABox9b;
	CString m_strHCFABox9c;
	CString m_strHCFABox10d;
	CString m_strHCFABox11bQual;
	CString m_strHCFABox11b;

	// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36

	bool m_bChanged;

// Dialog Data
	enum { IDD = IDD_BILL_CLAIM_FIELDS_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_ClaimFieldList;

	void AddHCFAFields();
	void AddFieldRow(FieldType eFieldType, CString strFieldName, CString strValue);

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishingClaimFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedClaimFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnOK();
	afx_msg void OnCancel();
};
