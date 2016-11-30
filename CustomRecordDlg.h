#pragma once
// CustomRecordDlg.h : header file
//

#include "CustomRecordDetailDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordDlg dialog

class CCustomRecordDlg : public CNxDialog
{
// Construction
public:
	// (j.armen 2013-05-08 13:05) - PLID 56603 - These params were being set by the caller anyways.  Just do it on construction
	CCustomRecordDlg(CWnd* pParent, const long& nPatID, const long& nMasterID, CBillingModuleDlg* pBillingDlg);   // standard constructor
	~CCustomRecordDlg();

private:
	BOOL Save();
	void ClearDetails();
	void CalculateAge();

	CCustomRecordDetailDlg &m_EMRDetailDlg;

	BOOL m_bInitialized;

	long m_nMasterID;
	long m_nPatID;

	NXDATALISTLib::_DNxDataListPtr m_ProviderCombo;
	NXDATALISTLib::_DNxDataListPtr m_ProcedureCombo;
	NXDATALISTLib::_DNxDataListPtr m_LocationCombo;
	NXDATALISTLib::_DNxDataListPtr m_DiagCodeCombo;
	NXDATALISTLib::_DNxDataListPtr m_DiagCodeList;
	NXDATALISTLib::_DNxDataListPtr m_BillList;
	NXDATALISTLib::_DNxDataListPtr m_MedicationList;
	NXDATALISTLib::_DNxDataListPtr m_TemplateCombo;
	NXDATALISTLib::_DNxDataListPtr m_FUTimeCombo;
	NXDATALISTLib::_DNxDataListPtr m_SchedProcedureCombo;

	CBillingModuleDlg *m_pBillingDlg;

	COleDateTime m_bdate;

	BOOL DeleteICD9(long DiagID);
	void AddChargeToBillList(long ServiceID);
	void ProcessProcedureActions(long ProcedureID);
	void ProcessEMRInfoActions(long EMRInfoID);
	void ProcessEMRDataActions(long EMRDataID);
	void ProcessEMRActions(EmrActionObject eaoSourceType, long nSourceID);

	BOOL m_bIsSpawning;
	CDWordArray m_dwaryOldOfficeVisitServiceIDs;
	CDWordArray m_dwaryNewOfficeVisitServiceIDs;

	void TryToIncrementOfficeVisit();

	void SaveMedications();

	CDWordArray m_dwDeletedMedications;

// Dialog Data
	//{{AFX_DATA(CCustomRecordDlg)
	enum { IDD = IDD_CUSTOM_RECORD_DLG };
	NxButton	m_btnProcedure;
	NxButton	m_btnFollowup;
	CNxIconButton	m_btnPrintReport;
	CNxIconButton	m_btnWritePrescriptions;
	CNxIconButton	m_btnMerge;
	CNxIconButton	m_btnCreateBill;
	CNxIconButton	m_btnAddMedication;
	CNxIconButton	m_btnAddCharge;
	CNxIconButton	m_btnClearDetails;
	CNxIconButton	m_btnAddEditItems;
	CNxIconButton	m_btnPrint;
	CNxEdit	m_strGender;
	CNxEdit	m_strAge;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxStatic	m_strPatientName;
	CDateTimePicker	m_dtPicker;
	CNxEdit	m_nxeditEditEmrNotes;
	CNxEdit	m_nxeditEditFuNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomRecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SynchronizeTodoAlarms();

	// Generated message map functions
	//{{AFX_MSG(CCustomRecordDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenProcedureCombo(long nRow);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPrint();
	afx_msg void OnAddEmrItems();
	afx_msg void OnClearDetails();
	afx_msg void OnTrySetSelFinishedEmrLocationsCombo(long nRowEnum, long nFlags);
	afx_msg void OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenDiagDropdown(long nRow);
	afx_msg void OnBtnAddCharge();
	afx_msg void OnBtnCreateBill();
	afx_msg void OnBtnAddMedication();
	afx_msg void OnBtnWritePrescriptions();
	afx_msg void OnBtnMerge();
	afx_msg void OnCheckFu();
	afx_msg void OnCheckProcedure();
	afx_msg void OnRButtonDownDiags(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownBill(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingBill(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedBill(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownMedications(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnPrintReport();
	afx_msg void OnCloseUpDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTrySetSelFinishedEmrProviderCombo(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};