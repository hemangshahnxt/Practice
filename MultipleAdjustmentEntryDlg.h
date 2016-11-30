#pragma once

// (j.jones 2012-05-07 09:58) - PLID 50215 - created

// CMultipleAdjustmentEntryDlg dialog

#include "FinancialRc.h"

class AdjustmentInfo {

public:
	COleCurrency cyAmount;
	long nGroupCodeID;
	long nReasonCodeID;
	long nAdjustmentID; //filled once the adjustments are created in data

	AdjustmentInfo::AdjustmentInfo() {
		cyAmount = COleCurrency(0,0);
		nGroupCodeID = -1;
		nReasonCodeID = -1;
		nAdjustmentID = -1;
	}
};

class CMultipleAdjustmentEntryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMultipleAdjustmentEntryDlg)

public:
	CMultipleAdjustmentEntryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMultipleAdjustmentEntryDlg();

	CString m_strInsuranceCoName;
	CString m_strLineType;
	CString m_strItemCode;
	COleCurrency m_cyChargeTotal;
	COleDateTime m_dtDateOfService;
	COleCurrency m_cyInsResp;
	COleCurrency m_cyInsBalance;

	CArray<AdjustmentInfo*, AdjustmentInfo*> m_aryAdjustmentInfo;

// Dialog Data
	enum { IDD = IDD_MULTIPLE_ADJUSTMENT_ENTRY_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	CNxStatic		m_nxstaticAdjTotal;
	CNxStatic		m_nxstaticChargeType;
	CNxStatic		m_nxstaticChargeCode;
	CNxStatic		m_nxstaticChargeAmount;
	CNxStatic		m_nxstaticChargeDate;
	CNxStatic		m_nxstaticInsResp;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	//add up the total and update the label, returns the total amount
	COleCurrency CalculateAdjustmentTotal();

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnBtnAddAdjustment();
	afx_msg void OnBtnRemoveAdjustment();
	DECLARE_EVENTSINK_MAP()
	void OnRButtonDownMultiAdjustmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingMultiAdjustmentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedMultiAdjustmentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
