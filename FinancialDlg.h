#if !defined(AFX_FINANCIALDLG_H__41EE6B33_32CE_11D2_B20B_0000C0832801__INCLUDED_)
#define AFX_FINANCIALDLG_H__41EE6B33_32CE_11D2_B20B_0000C0832801__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "PatientDialog.h"
#include "Client.h"
#include <afxtempl.h>
#include "BillingRc.h"
#include "SearchBillingTabDlg.h"

class CBillingModuleDlg;
class CPatientView;
class CPackages;
class CPendingQuotesDlg;
class CSearchBillingTabDlg;

// FinancialDlg.h : header file
//

// (j.dinatale 2010-10-27) - PLID 28773 - For the new
//		structure put in place, the Column Type must be defined and never change, if you wish
//		to add columns to this dialog, you MUST APPEND them to the bottom since the SQL table
//		structure relies on these values being constant.
// (j.jones 2008-09-16 17:50) - PLID 19623 - changed the column defines into a proper enum
// if you wish to change the order of the columns, you must change the order of the enum and
// change the order of the InsertColumn calls in OnInitDialog
// Also if you add a new column, you may wish to revisit the GetColorForColumn() function to
// decide what color you wish to use, if a special color is desired (otherwise it's yellow).
enum BillingColumnType {
	btcLineID = 0,
	btcPayToPayID = 1,
	btcPayID = 2,
	btcApplyID = 3,
	btcLineType = 4,
	btcChargeID = 5,
	btcExpandChar = 6,
	btcExpandSubChar = 7,
	btcBillID = 8,
	btcDate = 9,	
	btcNoteIcon = 10,
	btcDiscountIcon = 11,
	btcOnHoldIcon = 12,
	btcDescription = 13,
	btcCharge = 14,
	// (j.jones 2015-02-27 08:44) - PLID 64944 - added TotalPaymentAmount,
	// which caused all the indexes to go up by one
	btcTotalPaymentAmount = 15,
	btcPayment = 16,
	btcAdjustment = 17,
	btcRefund = 18,
	// NOTE: The column type of Insurance contains the following:
	//		Patient Balance
	//		Primary Insurance
	//		Secondary Insurance
	//		Other Insurance
	//		Any other insurance in RespTypeT
	btcInsurance = 19,
	btcBalance = 20,
	btcProvider = 21,
	btcLocation = 22,
	btcPOSName = 23,
	btcPOSCode = 24,
	btcInputDate = 25,
	btcCreatedBy = 26,
	btcFirstChargeDate = 27, 
	btcDiagCode1 = 28,
	btcDiagCode1WithName = 29,
	btcDiagCodeList = 30,
	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
	// and added ChargeAllowable
	btcFeeSchedAllowable = 31,
	btcChargeAllowable = 32,
	// (j.jones 2011-12-20 11:37) - PLID 47119 - added deductible and coinsurance
	btcChargeDeductible = 33,
	btcChargeCoinsurance = 34,
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	btcInvoiceNumber = 35,

	// (j.jones 2011-04-27 13:03) - PLID 43449 - added btcMaxValue, which is used in filters,
	// but never stored in data, so this value will change when we add new columns

	btcMaxValue,
};

// (j.jones 2010-06-18 10:06) - PLID 39150 - added struct for ins. resps
struct DynInsResp {

	long nRespTypeID;
	_variant_t varResp;		//should be a currency, but could possibly be NULL
};

// (j.jones 2010-06-18 10:23) - PLID 39150 - converted to a class, so we can have a destructor
// (a.wilson 2014-07-24 11:17) - PLID 63015 - added onhold icon and flag.
class FinTabItem {

public:
	_variant_t LineID;			//long
	_variant_t LineType;		//CString
	_variant_t Expandable;		//bit
	_variant_t ExpandChar;		//CString
	_variant_t ExpandSubChar;	//CString
	_variant_t Date;			//datetime		
	_variant_t NoteIcon;		//CString
	_variant_t HasNotes;		//bit
	_variant_t OnHold;			//bit
	// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
	//	IsVoided is true if the item is an original line item in a correction, or the voiding line item
	//	IsCorrected is true if the item is the new corrected line item and has not been re-voided, thus it is still editable
	_variant_t IsVoided;		//long (of a bit)
	_variant_t IsCorrected;		//long (of a bit)
	_variant_t Description;		//CString
	_variant_t ShowChargeAmount;//bit
	_variant_t BillID;			//long
	_variant_t ChargeID;		//long
	_variant_t PayID;			//long
	_variant_t PayToPayID;		//long
	_variant_t ApplyID;			//long
	_variant_t ShowBalanceAmount;//bit
	_variant_t ChargeAmount;	//money
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
	_variant_t TotalPayAmount;	//money
	_variant_t PayAmount;		//money
	_variant_t Adjustment;		//money
	_variant_t Refund;			//money
	_variant_t BalanceAmount;	//money
	_variant_t InsResp1;		//money
	_variant_t InsResp2;		//money

	// (j.jones 2010-06-18 10:03) - PLID 39150 - track dynamic ins. resp. columns
	CArray<DynInsResp*, DynInsResp*> arypDynInsResps;

	_variant_t InsRespOther;	//money		// (j.jones 2010-06-18 09:19) - PLID 39150 - renamed to InsRespOther
	_variant_t PatResp;			//money
	_variant_t IsPrePayment;	//bit
	_variant_t Discount;		//money (a.wetta 2007-04-24 10:07) - PLID 25170 - Discounts on the bill
	_variant_t HasPercentOff;	//bit (a.wetta 2007-04-24 10:07) - PLID 25170 - Has percent off discount on the bill
	_variant_t Provider;		//CString	// (j.jones 2008-09-17 09:56) - PLID 19623
	_variant_t Location;		//CString
	_variant_t POSName;			//CString	// (j.jones 2008-09-17 17:21) - PLID 19623
	_variant_t POSCode;			//CString	// (j.jones 2008-09-17 17:21) - PLID 19623
	_variant_t InputDate;		//datetime	// (j.jones 2008-09-18 09:23) - PLID 19623
	_variant_t CreatedBy;		//CString	// (j.jones 2008-09-18 10:37) - PLID 19623
	_variant_t FirstChargeDate;	//datetime	// (j.jones 2008-09-18 11:05) - PLID 19623
	// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
	_variant_t DiagCode1;			//CString
	_variant_t DiagCode1WithName;	//CString
	_variant_t DiagCodeList;		//CString
	// (j.jones 2010-09-01 15:37) - PLID 40331 - added allowable	
	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
	// and added ChargeAllowable
	_variant_t FeeSchedAllowable;	//COleCurrency (can be null)
	_variant_t ChargeAllowable;		//COleCurrency (can be null)
	// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
	_variant_t ChargeDeductible;	//COleCurrency (can be null)
	_variant_t ChargeCoinsurance;	//COleCurrency (can be null)
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	_variant_t InvoiceNumber;

	FinTabItem();
	~FinTabItem();
};

static CPtrArray g_aryFinancialTabInfoT,g_aryFinancialTabInfoTemp,g_aryFinancialTabInfoTemp2;

class CFinancialCorrection;

/////////////////////////////////////////////////////////////////////////////
// CFinancialDlg dialog

class CFinancialDlg : public CPatientDialog
{
	// (j.jones 2011-07-15 12:32) - PLID 43117 - needs to be dynamic
	DECLARE_DYNAMIC(CFinancialDlg);

// Construction
public:

	// (j.jones 2015-03-16 14:46) - PLID 64926 - added the search dialog, and
	// converted Packages & Quotes to be references
	CPackages &m_dlgPackages;
	CPendingQuotesDlg &m_dlgQuotes;
	CSearchBillingTabDlg &m_dlgSearchBillingTab;

	// (j.jones 2015-03-16 16:14) - PLID 64927 - Called by the search dialog to search
	// visible cells by the provided search term.
	// Returns the information for all matching cells.
	BillingTabSearchResults SearchBillingTab(CiString strSearchTerm);

	// (j.jones 2015-03-20 08:37) - PLID 64931 - Given a search result,
	// find the cell the result is pointing to. Returns the found row if successful.
	// This function does not select or highlight the cell.
	NXDATALIST2Lib::IRowSettingsPtr FindSearchResult(BillingTabSearchResultPtr pResult);

	// (j.jones 2015-03-17 12:24) - PLID 64929 - Given a search result, highlight the cell
	// the result is pointing to. Returns true if successful.
	bool HighlightSearchResult(BillingTabSearchResultPtr pResult);

	// (j.jones 2015-03-17 14:09) - PLID 64929 - if we have a highlighted cell, revert its color
	void RevertSearchResultCellColor();

	// (j.jones 2015-03-17 14:02) - PLID 64929 - caches the current highlighted cell's back & fore color
	NXDATALIST2Lib::IRowSettingsPtr m_pCurSearchResultsHighlightedRow;
	short m_nCurSearchResultsHighlightedColumn;
	OLE_COLOR m_clrCurSearchResultCellBackgroundColor;
	OLE_COLOR m_clrCurSearchResultCellForegroundColor;
	OLE_COLOR m_clrCurSearchResultCellBackgroundSelColor;
	OLE_COLOR m_clrCurSearchResultCellForegroundSelColor;
	
	void PostPackagePayment();
	void PostEditBill(long iBillID, int iSaveType);
	bool m_bNoteChanged;
	bool m_bStatementNoteChanged;
	void StoreDetails();
	void BuildBalanceFilterCombo();
	CString GetBalanceFilter(CString strFilterTable);
	// (j.jones 2010-06-07 14:28) - PLID 38933 - converted to a datalist 2
	NXDATALIST2Lib::_DNxDataListPtr m_List;
	NXDATALISTLib::_DNxDataListPtr m_StatementNoteCombo;
	NXDATALISTLib::_DNxDataListPtr m_BalanceFilterCombo;
	NXDATALISTLib::_DNxDataListPtr m_GlobalPeriodList;
	CFinancialDlg(CWnd* pParent);   // standard constructor
	~CFinancialDlg();

	// (j.jones 2010-06-16 16:14) - PLID 39150 - every column after secondary insurance is dynamic,
	// the column ID may be different at any given time
	short m_nOtherInsuranceColID;
	short m_nBalanceColID;
	short m_nProviderColID;
	short m_nLocationColID;
	short m_nPOSNameColID;
	short m_nPOSCodeColID;
	short m_nInputDateColID;
	short m_nCreatedByColID;
	short m_nFirstChargeDateColID;
	// (j.jones 2010-05-26 16:50 - PLID 28184 - added diagnosis code columns
	short m_nDiagCode1ColID;
	short m_nDiagCode1WithNameColID;
	short m_nDiagCodeListColID;
	// (j.jones 2010-09-01 15:33) - PLID 40331 - added allowable column
	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
	// and added ChargeAllowable
	short m_nFeeSchedAllowableColID;
	short m_nChargeAllowableColID;
	// (j.jones 2011-12-20 11:37) - PLID 47119 - added deductible and coinsurance
	short m_nChargeDeductibleColID;
	short m_nChargeCoinsuranceColID;
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	short m_nInvoiceNumberColID;
	// (j.jones 2015-02-27 08:44) - PLID 64944 - added TotalPaymentAmount
	short m_nTotalPaymentAmountColID;
	
	//invalid column, must always be the last entry
	short m_nMaxColID;

	// (j.dinatale 2010-10-27) - PLID 28773
	bool m_bRememberColWidth; // whether we using remember column widths
	void SetUpColumns(bool RemoveColumns); // Sets up column in the datalist on the form
	void SetUpColumnByType(long nColumnType, short &nIndex, long nWidth, BOOL Visible); // sets up a column of a particular type
	void SetUpInsuranceColumns(short &nIndex); // handles when insurance columns need to be inserted
	void EnsureBillColTSQLStructure(); // Ensure that the BillingColumnsT table is set up properly for the user
	void EnsureRespTypeColWidthTSQLStruct(); // Ensure that the RespTypeColumnWidthsT table is set up properly for this user

	// (j.dinatale 2010-11-05) - PLID 39226 - Needed so that way the view can be reconstructed from outside the dialog
	// (j.jones 2013-07-12 16:50) - PLID 57550 - removed bForceSave
	void ReconstructView();

	// (j.dinatale 2010-10-27) - PLID 28773 - map to track what column type each column is
	//		Column Index -> Column Type
	CMap<short, short, long, long> m_mapColIndexToColType;

	// (j.dinatale 2010-11-04) - PLID 28773 - map of column index to the last known user width
	CMap<short, short, long, long> m_mapColIndexToStoredWidth;

	// (j.dinatale 2010-10-27) - PLID 28773 - Used to update our sql structure with new column width information
	void SaveColumnWidths();

	// (j.dinatale 2010-11-01) - PLID 28773 - clears all member variables reflecting the index of a column
	void ClearColumnIndices();

	// (j.dinatale 2010-11-01) - PLID 28773 - Verifies that all non insurance column indices are set so that way issues do not arise, more of a fail-safe than anything so that way
	//		the billing tab will run even if some of our data is corrupt
	void VerifyNonInsuranceColumns(short &nIndex);

	// (j.dinatale 2010-11-01) - PLID 28773 - Verifies that patient resp. and the primary, secondary, and other insurance column indices are set so that way issues do not arise, 
	//		more of a fail-safe than anything so that way the billing tab will run even if some of our data is corrupt
	void VerifyInsuranceColumns(short &nIndex);

	// (j.dinatale 2010-10-27) - PLID 28773 - We must maintain the index of all the major columns
	//		that this dialog used to use an enum for.
	short m_nBillIDColID;
	short m_nExpandCharColID;
	short m_nExpandSubCharColID;
	short m_nDateColID;
	short m_nNoteIconColID;
	short m_nDiscountIconColID;
	short m_nOhHoldIconColID;	// (a.wilson 2014-07-24 11:16) - PLID 63015
	short m_nDescriptionColID;
	short m_nChargeColID;
	short m_nPaymentColID;
	short m_nAdjustmentColID;
	short m_nRefundColID;
	short m_nPatientBalanceColID;
	short m_nPrimInsColID;
	short m_nSecInsColID;

	// (j.jones 2010-06-18 09:37) - PLID 39150 - track dynamic resp IDs
	CArray<long, long> m_aryDynamicRespTypeIDs;

	// (j.jones 2010-06-17 14:29) - PLID 39150 - map insurance column IDs to resp IDs and back again
	CMap<short, short, long, long> m_mapColumnIndexToRespTypeID;
	CMap<long, long, short, short> m_mapRespTypeIDToColumnIndex;
	CMap<long, long, CString, CString> m_mapRespTypeIDToRespName;

	// (j.jones 2010-06-18 11:27) - PLID 39150 - added map for other guarantor IDs
	CMap<long, long, long, long> m_mapRespTypeIDsToGuarantorIDs;

	// (j.dinatale 2010-10-27) - PLID 28773 - no longer needed
	// (j.jones 2010-06-17 14:43) - PLID 39150 - update our visible insurance columns
	//void ReconfigureInsuranceColumns();
	
	//PLID 16640 - remove the minor discrepancy message
	//BOOL m_bCheckApplies;

	// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
	void InvokePostingDlg(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bPostToWholeBill);

	//TES 7/1/2008 - PLID 26143 - Added a paramater to exchange the product rather than simply returning it.
	// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
	void ReturnProduct(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bExchangingProduct);

	// Keyboard interface functions ////////////////
	int Hotkey(int key);
	void ExpandOnHighlight();
	void CollapseOnHighlight();
	void MoveHighlight(int nRows, int nPages);

	// (j.jones 2010-06-07 14:54) - PLID 39042 - deprecated
	//NXDATALIST2Lib::IRowSettingsPtr m_pBrightenedRow;
	//int m_iLine;

	bool m_bOneClickExpansion;

	CPatientView *m_pParent;

	// (j.jones 2008-09-17 08:51) - PLID 19623 - added m_ConfigBillColumnsChecker
	// (j.jones 2010-06-17 14:41) - PLID 39150 - added m_RespTypeChecker
	// (j.dinatale 2010-11-02) - PLID 28773 - removed the ConfigBillColumnsT table checker, no longer needed
	//CTableChecker m_ConfigBillColumnsChecker;
	CTableChecker m_LocationChecker, m_RespTypeChecker;

	LRESULT BillPackage(WPARAM wParam, LPARAM lParam);

	void OpenPayment(int nPayID);

	// (j.jones 2008-09-17 15:56) - PLID 31405 - required to detect when the screen resized
	virtual int SetControlPositions();

	////////////////////////////////////////////////

// Dialog Data
	//{{AFX_DATA(CFinancialDlg)
	enum { IDD = IDD_FINANCIAL_DLG };
	// (j.jones 2015-02-23 12:48) - PLID 64934 - added checkbox to hide voided items
	NxButton	m_checkHideVoidedItems;
	NxButton	m_checkHideZeroBalances;
	NxButton	m_btnSupressSt;
	NxButton	m_btnShowRefunds;
	CNxIconButton	m_btnShowQuotes;
	CNxIconButton	m_claimhistorybutton;
	CNxIconButton	m_btnResponsibleParty;
	CNxIconButton	m_btnShowPackages;
	CNxIconButton	m_showInsBalanceButton;
	CNxIconButton	m_applyManagerButton;
	CNxIconButton	m_printHistoryButton;
	CNxIconButton	m_previewStatementButton;
	CNxIconButton	m_newPaymentButton;
	CNxIconButton	m_newBillButton;
	CNxIconButton	m_btnEMNsToBeBilled; // (j.gruber 2010-09-07 11:10) - PLID 39571
	CNxColor	m_bkg;
	CNxEdit	m_nxeditEditNotes;
	CNxEdit	m_nxeditStatementNotes;
	// (j.jones 2009-08-24 13:04) - PLID 14229 - now we break this down into patient and insurance
	CNxEdit	m_nxeditLastPatPayDate;
	CNxEdit	m_nxeditLastInsPayDate;
	CNxStatic	m_nxstaticLabel2;
	CNxStatic	m_nxstaticLabelBalanceDue;
	CNxStatic	m_nxstaticLabel3;
	CNxStatic	m_nxstaticLabelPrepayments;
	CNxStatic	m_nxstaticLabelLine;
	CNxStatic	m_nxstaticLabel5;
	CNxStatic	m_nxstaticLabelTotalBalance;
	CNxStatic	m_nxstaticLabelCharges;
	CNxStatic	m_nxstaticLabelPayments;
	CNxStatic	m_nxstaticLabelAdjustments;
	CNxStatic	m_nxstaticLabelRefunds;
	CNxStatic	m_nxstaticLabelNetCharges;
	CNxStatic	m_nxstaticLabelNetPayments;
	CNxIconButton	m_btnGlassesOrderHistory;
	// (j.jones 2015-03-16 14:20) - PLID 64926 - added ability to search the billing tab
	CNxIconButton	m_btnSearchBilling;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinancialDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	// (j.jones 2015-03-16 15:08) - PLID 64926 - renamed to reflect that this
	// hides all billing tool windows
	void HideToolWindows();	//needed public to hide from other modules

protected:
	// Billing dialog
	CBillingModuleDlg *m_BillingDlg;

	// (j.jones 2010-06-08 15:54) - PLID 39042 - these are now member variables
	HICON m_hDiscount;
	HICON m_hBlank;
	HICON m_hNotes;
	HICON m_hOnHold;	// (a.wilson 2014-07-24 11:14) - PLID 63015
	// (j.jones 2015-02-20 11:22) - PLID 64935 - added void & correct icons
	HICON m_hVoided;
	HICON m_hCorrected;

	// Adds bills to FinancialTabInfoT
	// (may add 1 or all depending on [Bill ID])
	// (j.jones 2009-12-30 09:14) - PLID 36729 - replaced with functions that append select statements to a batch
	// (j.jones 2015-02-19 09:43) - PLID 64942 - removed the where clause param
	void AddCollapsedBillsToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, _variant_t varBillID, long nPatientID);
	void AddExpandedBillsToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, _variant_t varBillID, long nPatientID);

	// Adds charges to FinancialTabInfoT
	// (j.jones 2009-12-30 09:14) - PLID 36729 - replaced with a function that appends a select statement to a batch
	void AddChargesToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, _variant_t varBillID, _variant_t varChargeID, long nPatientID, BOOL bExpanded);

	// Adds the payments below all the bills
	// to FinancialTabInfoT
	// (j.jones 2009-12-30 10:34) - PLID 36729 - replaced with a function that returns an opened recordset
	ADODB::_RecordsetPtr CreateAddUnappliedPaysRecordset(long nPatientID);

	// Adds the payment applies in expanded charges
	// to FinancialTabInfoT
	// (j.jones 2009-12-30 12:24) - PLID 36729 - replaced with a function that appends a select statement to a batch
	void AddAppliedPaysToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, long nBillID, long nChargeID, long nPatientID);

	// Adds payments applied to other payments
	// in FinancialTabInfoT (includes insurance)
	// (j.jones 2009-12-30 12:33) - PLID 36729 - replaced with a function that returns an opened recordset
	ADODB::_RecordsetPtr CreateAddAppliedPayToPayRecordset(long nPaymentID, long nPatientID);

	// Calculates the total of all bills and all payments, in two recordsets, one data access
	// (j.jones 2009-12-30 13:14) - PLID 36729 - replaced with a function that returns an opened recordset
	ADODB::_RecordsetPtr CreatePatientTotalsRecordset(long nPatientID);

	// (z.manning 2010-06-10 16:45) - PLID 29214 - Function to transfer payments to another patient
	void TransferPaymentToAnotherPatient(LPDISPATCH lpRow);
	// (z.manning 2010-06-11 12:12) - PLID 29214 - Function to transfer bills to another patient
	// (z.manning 2010-06-17 09:06) - Too dangerous, removed.
	//void TransferBillToAnotherPatient(LPDISPATCH lpRow);

	// (j.jones 2011-09-21 09:53) - PLID 45462 - I don't know where this came from, but it is never used,
	// we already have m_iCurPatient for this purpose
	// The patient ID value
	//long m_nPatientID;	

	// Zero-based index to the row that the drag-and-
	// drop began on.
	// (j.jones 2010-06-07 16:42) - PLID 39042 - deprecated, this apparently
	// either never was used, or hasn't been used in years
	//long m_lSourceDragRow;


	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void SetColor(OLE_COLOR nNewColor);

	// Generated message map functions
	//{{AFX_MSG(CFinancialDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusEditNotes();
	afx_msg void OnKillfocusStatementNotes();
	afx_msg void OnChangeEditNotes();
	afx_msg void OnChangeStatementNotes();
	afx_msg void OnShowRefunds();
	afx_msg void OnCheckBatch();
	afx_msg void OnSelChosenStatementNoteCombo(long nRow);
	afx_msg void OnEditStatementNotes();
	afx_msg void OnNewBill();
	afx_msg void OnNewPayment();
	afx_msg void OnPrintHistory();
	afx_msg void OnPreviewStatement();
	afx_msg void OnShowInsBalance();
	afx_msg void OnApplyManager();
	afx_msg void OnShowPackages();
	afx_msg void OnResponsibleParty();
	afx_msg void OnSelChosenBalanceFilterCombo(long nRow);
	afx_msg void OnClaimHistory();
	afx_msg void OnRequeryFinishedGlobalPeriodList(short nFlags);
	afx_msg void OnShowQuotes();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCheckHideZeroBalances();
	afx_msg void OnTimer(UINT nIDEvent);
	// (j.dinatale 2010-10-29) - PLID 28773 - covers when the remember column width checkbox and on destroy to save column widths
	afx_msg void OnRememberColWidth();
	afx_msg void OnDestroy();
	// (j.jones 2010-06-07 15:18) - PLID 39042 - converted these into datalist 2 versions
	void OnRButtonDownBillingTabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnColumnClickingBillingTabList(short nCol, BOOL* bAllowSort);
	void OnDragBeginBillingTabList(BOOL* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	void OnDragOverCellBillingTabList(BOOL* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	void OnDragEndBillingTabList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	void OnLeftClickBillingTabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnColumnResize(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// These must have been added by Brad before 9/24 ///
	CString m_FinancialNotes;
	bool m_SuppressStatement;
	long m_GuarantorID1;
	long m_GuarantorID2;
	BOOL m_bOtherInsExists;
	
	/////////////////////////////////////////////////////

	// (j.jones 2015-02-19 09:35) - PLID 64942 - This is used for the Collapsed bills recordset
	// and the Unapplied pays recordset, the return value is different for each.
	// Set the parameter to true for bills, false for pays.
	CString GetZeroBalanceFilter(bool bBillsRecordset);

	// (j.jones 2015-02-23 15:22) - PLID 64934 - This is used in all recordsets, the return value
	// is different for bills vs. charges/payments.
	// Set the parameter to true for bills, false for any LineItemT records.
	CSqlFragment GetHideVoidedItemsFilter(bool bBillsRecordset);

	// (j.jones 2008-09-17 16:00) - PLID 19623 - renamed this function into something more accurate
	void DisplayInsuranceAndRefundColumns();

	// (j.jones 2009-12-30 09:31) - PLID 36729 - changed to a boolean
	BOOL m_bSortASC;

	// Describes which insurance columns are visible
	int m_iInsuranceViewToggle;

	// (d.thompson 2008-12-09) - PLID 32370 - Split this out from the insurance view
	long m_nShowRefunds;

	// Called when the user clicks on a payment,
	// adjustment or refund.
	// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
	void Pay_Adj_Ref_Click(NXDATALIST2Lib::IRowSettingsPtr pRow, long iColumn);

	// Used to create a new pay/adj/ref
	// (j.jones 2009-08-18 08:44) - PLID 24569 - added pre-payment option
	void CreateNewPayment(BOOL bIsPrePayment = FALSE);
	void CreateNewAdjustment();
	void CreateNewRefund();

	// Shows all the applies made to a charge.
	// Currently disabled in lieu of Apply Manager
	// progress (9/24)
	// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
	void ShowChargeApplies(NXDATALIST2Lib::IRowSettingsPtr pRow, int iComboMode);

	// Shows all the applies made to a bill
	// Currently disabled in lieu of Apply Manager
	// progress (9/24)
	// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
	void ShowBillApplies(NXDATALIST2Lib::IRowSettingsPtr pRow, int iComboMode);

	// Sets the blank row colors white. Currently
	// not supported. (9/24)
	// (j.jones 2010-06-07 15:32) - PLID 39042 - deprecated
	//void ColorList();

	// (j.jones 2010-06-07 15:33) - PLID 39042 - added the calculation
	// to lighten a color into its own function
	COLORREF LightenSelColor(COLORREF clrToChange);

	// Current bound row
	COleVariant m_varBoundItem;

	// TRUE if a charge is expanded
	BOOL InExpandedSubList(long itemID);

	// TRUE if a payment is expanded
	BOOL InExpandedPayList(long itemID);

	// Emptys the expand lists
	void ClearExpandLists();

	// Current patient displayed on the tab
	long m_iCurPatient;

	// TRUE if we can repaint the tab
	BOOL m_boAllowUpdate;

	// These must have been added by Brad before 9/24 ///
	long m_iLastPatID;
	int m_iCurrentOrder;
	BOOL InExpandedList(long ItemID);
	/////////////////////////////////////////////////////

	CArray<long, long> m_aiExpandedBranches;	// Expanded bill list
	CArray<long, long> m_aiExpandedPayBranches; // Expanded pay/adj/ref list
	CArray<long, long> m_aiExpandedSubBranches; // Expanded charges list

	CArray<long, long> aiTempExpandedBranches;	  // temp Expanded bill list
	CArray<long, long> aiTempExpandedPayBranches; // temp Expanded pay/adj/ref list
	CArray<long, long> aiTempExpandedSubBranches; // temp Expanded charges list

	// copy the m_aiExpanded arrays into the aiTempExpanded arrays
	void SaveExpansionInfo();
	// restore the above info
	void RestoreExpansionInfo();

	// Calculates the totals at the bottom of the tab
	void CalculateTotals();
	//long AddListItem(CDaoRecordset* rs, CDaoRecordset* rsList);

/*	void ReadBill(int& iBillID, COleCurrency& cyBillChargeTotal,
		COleCurrency& cyBillPaymentTotal, COleCurrency& cyBillAdjustmentTotal);

	void ReadCharge(int iBillID, int& iChargeID, COleCurrency& cyBillChargeTotal,
		COleCurrency& cyBillAdjustmentTotal, COleCurrency& cyChargePaymentTotal,
		COleCurrency& cyChargeAdjustmentTotal, COleCurrency& cyBillPaymentTotal,
		COleCurrency& cyCharge);

	void ReadPayment(int iBillID, int iChargeID, COleCurrency& cyBillPaymentTotal,
		COleCurrency& cyChargePaymentTotal,	COleCurrency& cyBillAdjustmentTotal,
		COleCurrency& cyChargeAdjustmentTotal);
*/
	// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
	void LeftClickList(NXDATALIST2Lib::IRowSettingsPtr pRow, long iColumn);
	void RightClickList(NXDATALIST2Lib::IRowSettingsPtr pRow, long iColumn);

	// Expands and redraws a bill
	void ExpandBill(long iBillID, long iLineID);

	// Expands and redraws a charge
	void ExpandCharge(long iChargeID, long iBillID, long iLineID);

	// Expands and redraws a payment
	void ExpandPayment(long iPaymentID);

	// Redraws line items
	void RedrawBill(long iBillID);
	void RedrawCharge(long iBillID, long iChargeID, long iLineID);
	void RedrawAllPayments();

	// (j.jones 2010-06-07 14:49) - PLID 39042 - deprecated
	//int GetRow(COleVariant varBoundValue);
	
	// Fills FinancialTabInfoT with bills and payments
	void FillTabInfo();

	// Empties FinancialTabInfoT, requeries all the
	// member queries, and fills it from scratch.
	void FullRefresh();

	// Empties FinancialTabInfoT and fills it from
	// scratch
	void QuickRefresh();

	// (a.walling 2006-06-23 10:14) - PLID 21053 Refresh the package list if it is visible
	// (j.jones 2006-08-07 15:13) - PLID 21813 - changed to also refresh the quotes list
	void RefreshPackagesQuotes();

	// (j.jones 2015-03-17 13:25) - PLID 64931 - if the search results window exists,
	// clear the results
	void ClearSearchResults();

	// (j.jones 2015-03-19 17:23) - PLID 64931 - checks to see if our search results are still
	// potentially valid, clears the results if we know they are not
	void RevalidateSearchResults();

	//Refills the datalist
	void RefreshList();

	// Gets new LineID for FinancialTabInfo tables
	int GetNewNum(CString arrayname);

	// Brightens and darkens a row (keyboard interface)
	// (j.jones 2010-06-07 14:54) - PLID 39042 - deprecated
	//void BrightenRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	//void DarkenRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	//void ResetBrightenedRow();
	//////////////////////////////////////////////

	//the following variables and functions aid in proper redrawing of the screen

	//is the screen currently redrawing?
	BOOL m_bIsScreenEnabled;

	//reference count of below functions
	long m_RedrawRefCount;

	// (j.jones 2010-06-07 17:07) - PLID 39042 - renamed these to mean the row's LineID

	//the row that needs to be set once the screen is redrawn
	long m_SavedRow_LineID;

	//the top row, prior to any redrawing
	long m_SavedTopRow_LineID;

	void DisableFinancialScreen();

	void EnableFinancialScreen();

	void AdjustVisibleRows();

	BOOL DoesLineHaveNotes(CString strLineType, long nItemID);

	// (j.jones 2008-06-13 17:50) - PLID 29872 - used to track what payment launched a new bill,
	// so we can properly apply the correct payment after the bill is saved
	long m_nPostEditBill_ApplyPaymentID;

	// (j.armen 2012-04-23 11:16) - PLID 14102 - Handle Insurance Reversals
	void ProcessInsuranceReversal(const long& nChargeID);

	// (r.gonet 07/07/2014) - PLID 62571 - Changes a bill's on hold state and reflects it on the billing tab.
	// - nBillID: ID of the BillsT record to change.
	// - bOnHold: TRUE to change the bill to On Hold. FALSE to change it to not On Hold.
	void SetBillOnHold(long nBillID, BOOL bHold);

	//////////////////////////////////////////////////////////////////////////////

	// (j.jones 2007-04-05 14:57) - PLID 25514 - added ability to print a claim for any resp type
	void PrintClaimForInsurance(long nBillID, long nInsuredPartyID);

	void ApplyRefundToAppliedPayment(long nPaymentID, long nApplyID, long nRefundID, BOOL bAutoApply = FALSE);

	void TryShowPackageWindow();

	// (j.dinatale 2010-10-27) - PLID 28773 - No longer needed
	// (j.jones 2008-09-16 17:27) - PLID 19623 - this function will reload shown/hidden column information
	// and update all visible columns accordingly
	//void ReconfigureVisibleColumns();

	// (j.jones 2008-09-17 15:01) - PLID 31405 - after refreshing the list, we must resize the description column
	void ResizeDescriptionColumn();

	// (j.jones 2008-09-18 09:07) - PLID 19623 - put column coloring in one function
	// (j.jones 2010-06-16 16:59) - PLID 39102 - this now takes in a short, not an enum
	COLORREF GetColorForColumn(short iColumn, BOOL bGrayOutColumns);

	// (j.gruber 2010-09-07 11:09) - PLID 39571 - added button to get the EMns to be billed screen
	afx_msg void OnBnClickedPtsToBeBilled();

	//(c.copits 2010-09-24) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	ADODB::_RecordsetPtr GetBestUPCProduct(LPARAM lParam);

	// (j.jones 2011-10-28 14:04) - PLID 45462 - added function to recursively undo applied payments,
	// anytime we undo a correction that undoes a payment correction, we have to check for cases where
	// that source payment has a partial apply, recursively
	// Returns FALSE only if the correction cannot be undone, and a message will have explained why not.
	BOOL UndoAppliedCorrections(CFinancialCorrection &finCor, long nOriginalPaymentIDToCheck,
								CString strOriginalCorrectionsTableToUndo, long nOriginalCorrectionID_ToUndo,
								CString strOriginalCorrectionUndoType);

	afx_msg void OnGlassesOrderHistory();
	
	// (b.spivey, September 11, 2014) - PLID 63652 - Check for linked status. 
	bool IsLinkedToLockbox(long nPaymentID); 
	
	// (j.jones 2015-02-23 12:48) - PLID 64934 - added checkbox to hide voided items
	afx_msg void OnCheckHideVoidedItems();

	// (j.jones 2015-02-25 09:45) - PLID 64939 - made one modular function to open Apply Manager
	void OpenApplyManager(long nBillID, long nChargeID, long nPayAdjRefID, long nApplyID);
	
	// (j.jones 2015-03-16 17:06) - PLID 64927 - one function to expand them all
	void ExpandAllRows();

	// (j.jones 2015-03-16 14:20) - PLID 64926 - added ability to search the billing tab
	afx_msg void OnBtnSearchBilling();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINANCIALDLG_H__41EE6B33_32CE_11D2_B20B_0000C0832801__INCLUDED_)