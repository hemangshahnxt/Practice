#if !defined(AFX_BATCHPAYMENTS_H__9207FD24_88BE_48E5_8C89_B6F5353DE65A__INCLUDED_)
#define AFX_BATCHPAYMENTS_H__9207FD24_88BE_48E5_8C89_B6F5353DE65A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BatchPayments.h : header file
//

// (b.eyers 2015-10-23) - PLID 67384
#include <NxUILib/NxStaticIcon.h>

namespace PatientComboSortType
{
	enum SortType
	{
		LastFirstASC = 0,
		LastFirstDESC = 1,
		UserDefinedIdASC = 2,
		UserDefinedIdDESC = 3,
		FirstLastASC = 4,
		FirstLastDESC = 5,
		OtherASC = 6,
		OtherDESC = 7,
	};
}

enum EBatchPaymentPayType;

// (b.spivey, September 22, 2014) - PLID 62924 - Holder to close the modal dialog. 
namespace WaitingOnImportDialog
{
	struct Holder
	{
		Holder(CWnd* pWnd);
		~Holder();

		CWnd* pWnd = nullptr;
	};

	void Close();
}

/////////////////////////////////////////////////////////////////////////////
// CBatchPayments dialog

class CBatchPayments : public CNxDialog
{
// Construction
public:
	void CreateAdjustment();
	void ApplyToPatient(BOOL bApplyToWholeBill);
	void PreviewStatement();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	BOOL m_bFilteredByBill;

	CBrush m_brush;

	NXDATALISTLib::_DNxDataListPtr m_BatchPayments,
					m_PatientCombo,
					m_ChargeList;
	CBatchPayments(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBatchPayments)
	enum { IDD = IDD_BATCH_PAYMENTS };
	NxButton	m_checkShowAllPatientInsResps;
	CNxIconButton	m_btnCreateAdjustment;
	CNxIconButton	m_btnPostToPatient;
	NxButton	m_checkShowPaysWithZeroBal;
	CNxIconButton	m_btnFilterByBillID;
	CNxIconButton	m_btnRefundedPays;
	CNxIconButton	m_btnLockBox;  // (b.spivey - July 21st, 2014) - PLID 62957
	CNxIconButton	m_btnCreateRefund;
	CNxIconButton	m_btnNewEOB;
	CNxIconButton	m_btnDelPay;
	CNxIconButton	m_btnEditPay;
	CNxIconButton	m_btnAddPay;
	NxButton	m_radioAllPatients;
	NxButton	m_radioPatientsInGroup;
	NxButton	m_radioIndivPatient;
	CNxStatic	m_nxstaticPayAmount;
	CNxStatic	m_nxstaticRemAmount;
	// (j.jones 2009-06-10 15:48) - PLID 30478 - added ability to unapply an EOB
	CNxIconButton	m_btnUnapplyEOB;
	// (j.jones 2009-06-26 12:59) - PLID 33856 - added original amount
	CNxStatic	m_nxstaticOriginalAmtLabel;
	CNxStatic	m_nxstaticOriginalAmt;
	// (j.jones 2012-02-16 17:21) - PLID 48137 - added buttons to go to patient accounts
	CNxIconButton	m_btnGoToPatient;
	CNxIconButton	m_btnGoToPatientID;
	CNxIconButton	m_btnInsuranceReversal;	// (j.armen 2012-05-24 14:47) - PLID 50520 - Added button for insurance reversals
	// (j.jones 2012-08-22 15:10) - PLID 52153 - added ability to group by bill, which also turned the rev. code group into a radio button
	NxButton	m_radioGroupByCharge;
	NxButton	m_radioGroupByBill;
	NxButton	m_radioGroupByRevenueCode;
	CNxStatic	m_nxstaticChargeListLabel;
	// (b.eyers 2015-10-23) - PLID 67384
	CNxStatic	m_nxstaticTotalAllowableLabel;
	CNxStatic	m_nxstaticTotalAllowableAmt;
	CNxStatic	m_nxstaticReimbursementRateLabel;
	CNxStatic	m_nxstaticReimbursementPercentage;
	CNxStaticIcon m_icoReimbursementInfo;
	//}}AFX_DATA
	// (s.tullis 2014-08-12 16:53) - PLID 63240 - Batch Payments needs a CTableChecker object for PatCombo.
	CTableChecker m_patients;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchPayments)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2015-10-20 08:48) - PLID 67377 - added auto-posting enum
	enum EAutoPostType {
		eDoNotAutoPost = 0,				//default: no automatic posting
		eTryAutoPost_ElseShowDlg = 1,	//attempts automatic posting, opens the line item posting dialog if auto-posting fails
		eAutoPostOnly_NoDlg = 2,		//attempts automatic posting, cancels the dialog if auto-posting fails
	};

	// (j.jones 2012-08-16 09:06) - PLID 52116 - Added bAutoPost, which will try
	// to open line item posting invisible and auto-post the expected amount.
	// The dialog would be forcibly displayed if there is a problem or warning
	// and it can't silently post successfully.
	// (j.jones 2015-10-20 08:48) - PLID 67377 - changed the autopost parameter to an enum of options
	void InvokePostingDlg(BOOL bPostToWholeBill, EAutoPostType eAutoPostType = eDoNotAutoPost);

	// (j.jones 2012-08-15 14:06) - PLID 52115 - added ability to auto-select the first charge when requery finishes
	void ChangePatientSelectionType(BOOL bSelectFirstCharge = FALSE);

	// (j.jones 2012-08-15 14:06) - PLID 52115 - added ability to auto-select the first charge when requery finishes
	void RequeryChargeList(BOOL bSelectFirstCharge = FALSE);
	
	// (j.jones 2012-08-15 14:06) - PLID 52115 - added ability to auto-select the first charge when requery finishes
	BOOL m_bAutoSelectFirstChargeOnRequeryFinished;

	// (b.savon 2012-06-13 15:57) - PLID 49879 - Handle updating all the payments/line items with the new batch date
	void UpdatePaymentDatesToReflectBatchDate(long nBatchPaymentID, CString strBatchDate);

	void PostToPatient(long nPatientID);

	//the following variables and functions aid in proper redrawing of the screen

	//is the screen currently redrawing?
	BOOL m_bIsScreenEnabled;

	//reference count of below functions
	long m_RedrawRefCount;

	//the row that needs to be set once the screen is redrawn
	long m_SavedRow;

	//the top row, prior to any redrawing
	long m_SavedTopRow;

	void DisableScreen();

	void EnableScreen();

	// (z.manning, 03/23/2007) - PLID 25323 - Added these functions as an alternative to requerying the charge
	// list every time we do something to it.
	void UpdateChargeListByChargeID(long nChargeID);
	void UpdateChargeListByBillID(long nBillID);
	void AddChargesToListFromQuery(CString strQuery);

	// (j.jones 2007-05-10 17:58) - PLID 25973 - used to determine when to requery the patient combo
	BOOL m_bNeedRequeryPatientCombo;

	// (j.jones 2012-02-16 17:23) - PLID 48137 - this now takes in a patient ID
	void GotoPatient(long nPatientID);// (a.vengrofski 2010-05-04 12:20) - PLID <36205> - Function to goto the selected patient.

	// (j.jones 2010-05-17 13:05) - PLID 33941 - added ability to prompt for a patient warning,
	// also we track who we last warned about
	void PromptPatientWarning(long nPatientID);
	long m_nLastPatientIDWarned;

	// (j.jones 2012-08-15 15:24) - PLID 52151 - added ability to remember the last sort column
	void RestoreLastChargeSortOrder();

	// (j.jones 2012-08-20 09:56) - PLID 52116 - moved the defined from clauses into a function
	// (j.jones 2012-08-29 10:16) - PLID 52351 - added ability to calculate secondary payments as well
	CString GenerateFromClause(BOOL bShowEstPaymentColumns, BOOL bEstSecondaryPayments);

	// (j.jones 2012-08-20 13:42) - PLID 52116 - updates the Est. Payment column display
	void UpdateEstPaymentColumns(BOOL bShowEstPaymentColumns);

	// (j.jones 2012-08-20 16:40) - PLID 52116 - added ability to auto-post, this will
	// just open line item posting normally if we have no est. payment
	void TryAutoPostPayment(BOOL bPostToWholeBill);

	// (j.jones 2012-08-22 15:10) - PLID 52153 - added ability to group by bill, which also turned the rev. code group into a radio button,
	// all three radio button handlers will just call this function
	void OnChargeGroupingChanged(BOOL bRequery = TRUE);

	// (j.jones 2012-08-22 17:26) - PLID 52153 - We have a lot of possible default double-click actions,
	// but we can configure the screen such that some are not possible in certain situations.
	// This function will convert the desired default action into the next best alternative.
	void ConvertDefBatchPayClickAction(IN OUT long &nDefBatchPayClickAction, BOOL bShowEstPaymentColumns, BOOL bIsPatientResp);
	// (s.tullis 2014-06-23 11:21) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
	void SecureControls();

	// (j.jones 2014-06-26 17:31) - PLID 62545 - added a dedicated AddNewPayment function
	void AddNewPayment(EBatchPaymentPayType ePayType);

	// (j.jones 2014-06-27 09:20) - PLID 62547 - tells us if the selected payment is a vision payment,
	// always false if they do not have the vision payment license
	bool IsVisionPayment();

	// Generated message map functions
	
	void EnsurePatComboUpdated();
	
	PatientComboSortType::SortType GetPatientSortType(long nColumn, BOOL bSortAscending);
	void PatientDropdownSetSortCol();
	PatientComboSortType::SortType m_ePatientComboSortType;
	BOOL m_bDialogInitializing;

	// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
	// Est. Payment column preference is on, false if a vision payment
	bool IsCapitationPayment();

	// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
	// Est. Payment column preference is on
	bool ShowEstPaymentColumns();

	// (j.jones 2015-10-16 15:24) - PLID 67382 - Added function to show/hide controls for capitation payments.
	// Send in true if we are posting a capitation payment, false if not posting a capitation payment.
	void UpdateControlsForCapitation(bool bCapitationInUse);

	// (j.jones 2015-10-19 13:40) - PLID 67376 - Calculates the capitation payment amounts per charge
	// from the allowables, and fills the Est. Payment columns with the results.
	// Returns true if successful, false if a calculation was not possible.
	bool CalculateCapitation();

	// (j.jones 2015-10-20 10:37) - PLID 67385 - returns true if the current payment is
	// a locked capitation payment
	bool IsLockedCapitationPayment();

	// (j.jones 2015-10-22 08:41) - PLID 67385 - locks the current capitation payment,
	// returns false if the batch could not be locked
	bool LockCapitationPayment();

	// (b.eyers 2015-10-27) - PLID 67384 - calculate the reimbursement rate
	double CalculateReimbursementRate(COleCurrency cyBatchPaymentAmount, COleCurrency cyTotalCopays, COleCurrency cyTotalAllowable);

	// Generated message map functions
	//{{AFX_MSG(CBatchPayments)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenBatchPaymentsCombo(long nRow);
	afx_msg void OnRadioAllPatients();
	afx_msg void OnRadioIndivPatient();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChosenSelectedPatientCombo(long nRow);
	afx_msg void OnDblClickCellApplyList(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonDownApplyList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedApplyList(short nFlags);
	// (j.jones 2014-06-26 17:30) - PLID 62545 - renamed to indicate this is the button handler only
	afx_msg void OnBtnAddPayment();
	afx_msg void OnEditPayment();
	afx_msg void OnDeletePayment();
	afx_msg void OnRadioAllPatientsInGroup();
	afx_msg void OnBtnNewEob();
	afx_msg void OnBtnNewRefund();
	afx_msg void OnBtnViewAppliedRefunds();
	afx_msg void OnBtnFilterByBillId();
	afx_msg void OnCheckShowZeroBalancePayments();
	afx_msg void OnBtnPostToPatient();
	afx_msg void OnBtnNewAdjustment();
	// (j.jones 2007-03-27 14:34) - PLID 23987 - added ability to show
	//all insurance responsibilities for the selected patient
	afx_msg void OnCheckShowAllPatientInsResps();
	// (j.jones 2007-05-10 17:34) - PLID 25973 - supported OnTableChanged
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2009-06-10 15:56) - PLID 30478 - added ability to unapply an EOB
	afx_msg void OnBtnUnapplyBatchPayment(); // (b.eyers 2015-10-14) - PLID 67308 - renamed function 
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2012-02-16 17:21) - PLID 48137 - added buttons to go to patient accounts
	afx_msg void OnBtnGoToPatient();
	afx_msg void OnBtnGoToPatientId();
	afx_msg void OnBnClickedInsuranceReversal();	// (j.armen 2012-05-24 14:47) - PLID 50520 - Button handler for Insurance Reversals
	// (j.jones 2012-08-15 15:24) - PLID 52151 - added ability to remember the last sort column
	void OnColumnClickingApplyList(short nCol, BOOL* bAllowSort);
	// (j.jones 2012-08-16 09:02) - PLID 52116 - added left click handler for hyperlink cells
	// (j.jones 2015-10-29 16:06) - PLID 67431 - converted to left click, not lbuttondown
	void OnLeftClickApplyList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	// (j.jones 2012-08-22 15:10) - PLID 52153 - added ability to group by bill, which also turned the rev. code group into a radio button
	afx_msg void OnRadioGroupByCharge();
	afx_msg void OnRadioGroupByBill();
	afx_msg void OnRadioGroupByRevCode();
	// (j.jones 2012-08-23 17:53) - PLID 52152 - if a user tabs to the charge list, select a row
	void OnFocusGainedApplyList();
	//TES 8/13/2014 - PLID 63520 - Added support for EX tablecheckers
	afx_msg LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	void FocusGainedSelectedPatientCombo();
	void ChangeColumnSortFinishedSelectedPatientCombo(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending);
	// (b.spivey - July 21st, 2014) - PLID 62957
	afx_msg void OnBnClickedBtnManageLockboxPayments();
	// (j.jones 2015-10-19 16:13) - PLID 67309 - added ability to show/hide Capitation column
	afx_msg void OnRequeryFinishedBatchPaymentsCombo(short nFlags);
	// (j.jones 2015-10-29 15:24) - PLID 67431 - added dragging abilities
	void OnDragBeginApplyList(BOOL* pbShowDrag, long nRow, short nCol, long nFlags);
	void OnDragOverCellApplyList(BOOL* pbShowDrop, long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
	void OnDragEndApplyList(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPAYMENTS_H__9207FD24_88BE_48E5_8C89_B6F5353DE65A__INCLUDED_)