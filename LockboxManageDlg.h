#pragma once
#include "FinancialRc.h"

// CLockboxManageDlg dialog

// (b.spivey - July 21st, 2014) - PLID 62959 - Created. 

class CLockboxManageDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLockboxManageDlg)

	
public:
	CLockboxManageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLockboxManageDlg();

// Dialog Data
	enum { IDD = IDD_LOCKBOX_MANAGE_DLG };

protected:

	enum ELockBoxBatchColumns {
		elbbcID = 0,
		elbbcPaymentDate,
		elbbcBankName, 
		elbbcLocationID, 
		elbbcLocationName, 
		elbbcDepositAmount, 
		elbbcRemainingAmount, 
		elbbcPaymentCount, 
	};

	enum ELockBoxLocationColumns {
		elblcID = 0,
		elblcName,
	};
																										
																										
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support								
	virtual BOOL OnInitDialog();																		
																										
	NXDATALIST2Lib::_DNxDataListPtr m_dlBatchList; 														
	NXDATALIST2Lib::_DNxDataListPtr m_dlLocationFilter;													
																										
	NxButton m_checkStartDate;
	NxButton m_checkEndDate; 
	CNxIconButton m_Close; 
	CNxIconButton m_PrintPreview; // (b.spivey, August 18, 2014) - PLID 62965
	CNxIconButton m_Refresh; // (b.spivey, August 18, 2014) - PLID 62959 
	CNxIconButton m_Edit; // (b.spivey, August 18, 2014) - PLID 62960  
	CNxIconButton m_Delete; // (b.spivey, August 18, 2014) - PLID 63361
	CDateTimePicker m_dtpStartDate;
	CDateTimePicker m_dtpEndDate; 

	// (b.spivey - October 1, 2014) - PLID 63814 - Radio buttons to change date filtering. 
	NxButton m_RadioDepostDate;
	NxButton m_RadioPaymentDate; 

	int m_LastRadioChecked;

	// (b.spivey - July 21st, 2014) - PLID 62958 - filter functionality. 
	afx_msg void OnBnClickedCheckEndDate();
	afx_msg void OnBnClickedCheckBeginDate();
	afx_msg void OnChangeStartDateFilter(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeEndDateFilter(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRadioDepositDate();
	afx_msg void OnBnClickedRadioPaymentInputDate();

	void OnSelChosenLocationFilter(LPDISPATCH lpRow);

	// (b.spivey - September26, 2014) - PLID 62958
	CString BuildWhereClause();

	// (b.spivey, August 18, 2014) - PLID 62959 - Control state
	void SetControlState(BOOL bControlState);

	// (b.spivey, August 13th, 2014) - PLID 63361 - Delete lockobox batches. 
	bool DeleteLockboxBatch(long nBatchID);

	// (b.spivey - July 21st, 2014) - PLID 62960 - Edit payment functionality. 
	void OnDblClickCellLockBoxBatchList(LPDISPATCH lpRow, short nColIndex);
	void OnRButtonDownLockBoxBatchList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	
	// (b.spivey, August 18, 2014) - PLID 62959 - event handler to make sure control state is set every time the selection changes. 
	void CurSelWasSetLockboxBatchList();

	// (b.spivey, September 30, 2014) - PLID 62965 - Print preview of the report. 
	void PrintPreviewReport(long nBatchID, COleDateTime dtBatchDate);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	afx_msg void OnBnClickedPrintPreview(); // (b.spivey, August 18, 2014) - PLID 62965
	afx_msg void OnBnClickedRefreshLockbox(); // (b.spivey, August 18, 2014) - PLID 62959
	afx_msg void OnBnClickedEditLockboxPayment(); // (b.spivey, August 18, 2014) - PLID 62960 
	afx_msg void OnBnClickedDeleteLockboxPayment(); // (b.spivey, August 18, 2014) - PLID 63361
	afx_msg void OnSelChangingLocationFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

};
