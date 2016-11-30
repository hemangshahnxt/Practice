#pragma once

// (j.armen 2012-05-24 17:36) - PLID 50520 - Created dlg for reversing multiple payments on a batch payment

class CBatchPaymentInsReversalDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBatchPaymentInsReversalDlg)

public:
	CBatchPaymentInsReversalDlg(CWnd* pParent, const long& m_nBatchPaymentID); 
	virtual ~CBatchPaymentInsReversalDlg();

	enum { IDD = IDD_INSURANCE_REVERSAL };

private:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlPatientList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlPaymentList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSelectedList;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnMoveRight;
	CNxIconButton m_btnMoveLeft;
	CNxColor m_NxColor;

	const long& m_nBatchPaymentID;

	void EnsureControlState();
	void GetSelectedLineItemIDs(CArray<long>& aryLineItemIDs);
	void SelectLineItem(const _variant_t& vtLineItemID);
	void DeselectLineItem(const _variant_t& vtLineItemID);
	void ValidateLineItemCorrectionStatus();

	// Overrides
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedReversalSelect();
	afx_msg void OnBnClickedReversalDeselect();
	afx_msg void OnOk();

	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedReversalPatient(short nFlags);
	void SelChosenReversalPatient(LPDISPATCH lpRow);
	void RequeryFinishedReversalPaymentAdjustment(short nFlags);
	void DblClickCellReversalPaymentAdjustment(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellReversalSelectedPaymentAdjustment(LPDISPATCH lpRow, short nColIndex);
	void SelChangingEnsureSelection(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);	
};
