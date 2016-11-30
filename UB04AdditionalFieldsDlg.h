#pragma once

#include "UB04Utils.h"

//(j.camacho 2016-3-3) PLID 68501 - Create new UB04 Dialog
// CUB04AdditionalFieldsDlg dialog

class CUB04AdditionalFieldsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CUB04AdditionalFieldsDlg)

public:
	// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg
	CUB04AdditionalFieldsDlg(CWnd* pParent, UB04::ClaimInfo claimInfo, UBFormType formType, const CString& strUB92Box79 = "");   // standard constructor
	virtual ~CUB04AdditionalFieldsDlg();
	
	void SetReadOnly(bool bIsReadOnly = true)
	{
		m_bReadOnly = bIsReadOnly;
	}

	UB04::ClaimInfo GetClaimInfo() const
	{
		return m_claimInfo;
	}

	CString GetUB92Box79() const
	{
		return m_strUB92Box79;
	}

	// (r.goldschmidt 2016-03-08 11:32) - PLID 68497 - UB04 Enhancements - Apply default dates
	void SetDefaultDates(COleDateTime dtDate, COleDateTime dtFrom, COleDateTime dtThrough);
	
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UB04_ADD_FIELDS };
#endif
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

protected:

	// (r.goldschmidt 2016-03-03 12:27) - PLID 68511 - UB04 Enhancements - set up datalists of new dialog
	NXDATALIST2Lib::_DNxDataListPtr m_pConditionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pValueList;
	NXDATALIST2Lib::_DNxDataListPtr m_pOccurrenceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pOccurrenceSpanList;

	bool m_bReadOnly = false;

	UBFormType m_formType = eUB04;

	void SecureControls(bool bResetData);
	void EmptyAllData();
	void LoadUB04ClaimInfo(); // (r.goldschmidt 2016-03-04 10:32) - PLID 68506
		
	UB04::ClaimInfo m_claimInfo;
	CString m_strUB92Box79;

	// (r.goldschmidt 2016-03-08 11:32) - PLID 68497 - UB04 Enhancements - default dates
	_variant_t varDefaultDate = g_cvarNull;
	_variant_t varDefaultFrom = g_cvarNull;
	_variant_t varDefaultThrough = g_cvarNull;

	virtual BOOL OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnOK();
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishingUb04Conditional(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishingUb04Valuecode(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishingUb04Occurence(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishingUb04Occurencespan(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
