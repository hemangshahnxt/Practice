#if !defined(AFX_CASEHISTORYDLG_H__14B0BAA3_A91B_4DA1_BFC7_726E5B85F332__INCLUDED_)
#define AFX_CASEHISTORYDLG_H__14B0BAA3_A91B_4DA1_BFC7_726E5B85F332__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaseHistoryDlg.h : header file
//

#include "CaseHistoriesDlg.h"
#include "InvUtils.h"

// (j.jones 2009-08-07 08:40) - PLID 7397 - track when we close via print previewing
#define RETURN_PREVIEW_CASE_HISTORY		10042

enum ECaseHistoryDetailItemType
{
	chditProduct = -1,
	chditCptCode = -2,
	chditPerson = -3,
};

struct CHProductItems {
	long ProductItemID;
	int SaveStatus; //uses CPI_ info
};

struct CaseHistoryDetailProductItemList {
	long ID;
	long CaseHistoryDetailID;
	CArray<CHProductItems*,CHProductItems*> ProductItemAry;
};

struct SavedCaseHistoryDetail {
	long nCaseHistoryDetailID;
	ECaseHistoryDetailItemType eType;
	CString strName;
	double dblQuantity;
	COleCurrency cyAmount;
	COleCurrency cyCost;
	BOOL bBillable;
	// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
	//BOOL bPayToPractice;
};

#define CHDPI_SAVENEW	1
#define	CHDPI_DELETE	2
#define	CHDPI_NONE		3

/////////////////////////////////////////////////////////////////////////////
// CCaseHistoryDlg dialog

class CCaseHistoryDlg : public CNxDialog
{
// Construction
public:
	CCaseHistoryDlg(CWnd* pParent);   // standard constructor
	~CCaseHistoryDlg();

	long m_nCaseHistoryID;
	long m_nPersonID;
	long m_nAppointmentID;
	
	// (j.jones 2006-12-13 15:46) - PLID 23578 - created variables to assist
	// in creating new cases
	BOOL m_bIsNew;

	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
	CArray<long, long> m_arynDefaultPreferenceCardIDs;

	CString m_strDefaultName;
	long m_nDefaultProviderID;
	COleDateTime m_dtDefaultSurgeryDate;

	CCaseHistoriesDlg *m_pParent;

// Dialog Data
	//{{AFX_DATA(CCaseHistoryDlg)
	enum { IDD = IDD_CASE_HISTORY_DLG };
	NxButton	m_btnCompleted;
	CNxIconButton	m_btnCreateInvAllocation;
	CNxIconButton	m_btnAutoTime_Out_Surgeon;
	CNxIconButton	m_btnAutoTime_Out_Recovery;
	CNxIconButton	m_btnAutoTime_Out_PreOp;
	CNxIconButton	m_btnAutoTime_Out_OpRoom;
	CNxIconButton	m_btnAutoTime_Out_Fac;
	CNxIconButton	m_btnAutoTime_Out_Anes;
	CNxIconButton	m_btnAutoTime_Out_23;
	CNxIconButton	m_btnAutoTime_In_Surgeon;
	CNxIconButton	m_btnAutoTime_In_Recovery;
	CNxIconButton	m_btnAutoTime_In_PreOp;
	CNxIconButton	m_btnAutoTime_In_OpRoom;
	CNxIconButton	m_btnAutoTime_In_Fac;
	CNxIconButton	m_btnAutoTime_In_Anes;
	CNxIconButton	m_btnAutoTime_In_23;
	CDateTimePicker	m_ctrlSurgeryDate;
	CNxEdit	m_nxeditNameEdit;
	CNxEdit	m_nxeditAppointmentDesc;
	CNxEdit	m_nxeditEditTotalCaseAnesthMinutes;
	CNxEdit	m_nxeditEditTotalCaseFacilityMinutes;
	CNxEdit	m_nxeditNotesEdit;
	CNxStatic	m_nxstaticMultiProcedureLabel;
	CNxStatic	m_nxstaticAllocationSelectLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnPrint;
	CNxIconButton	m_btnChangeAppt;
	CNxIconButton	m_btnClearAppt;
	CNxIconButton	m_btnGotoAppt;
	CNxIconButton	m_btnDischargeStatus;
	NxButton	m_btnPreopLine;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaseHistoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Interface
public:
	// (j.jones 2006-12-13 15:44) - PLID 23578 - removed the DoModal function
	// in lieu of these two functions for a new or existing case history
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-26 09:52) - PLID 34943 - we no longer pass in location ID, a preference inside this function will calculate it
	// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
	int OpenNewCase(long nPersonID, CArray<long, long> &arynCopyFromPreferenceCardIDs, const CString &strName, const long nProviderID, const COleDateTime &dtSurgeryDate, const long nAppointmentID = -1);
	int OpenExistingCase(long nCaseHistoryID);

public:
	CString m_strName;

// APIs
public:
	
	// Returns the ID of the new case history object
	// Throws _com_error
	//TES 1/10/2007 - PLID 23575 - This doesn't appear to be called anywhere, and I'm not going to add auditing to it,
	// so I'm commenting it out.
	//static long CreateCaseHistory(long nPersonID, long nCopyFromSurgeryID, const CString &strName, const long nProviderID, const long nLocationID, const COleDateTime &dtSurgeryDate, const long nAppointmentID = -1);
	
	// Deletes the case history
	// Throws _com_error, CNxException*
	static void DeleteCaseHistory(long nID, BOOL bDeleteMailSent = TRUE);
	
	// Returns the comma-delimited list of procedure names reprsented by the cpt details of the given surgery
	// Throws _com_error
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	static CString GuessProcedureListFromPreferenceCard(long nPreferenceCardID);

	// Prompts the user to choose a surgery
	// Throws _com_error, CException*
	// (c.haag 2007-03-09 11:12) - PLID 25138 - We now need a patient ID as well
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
	// (j.jones 2009-08-31 17:54) - PLID 17734 - this optionally takes in an appointment ID
	static BOOL ChoosePreferenceCards(CArray<long, long> &arynPreferenceCardIDs, OUT CString &strNewCaseHistoryName, OUT long &nProviderID, long nPatientID, long nAppointmentID = -1);

	// Just checks to see if the given case history is attached to a bill
	// Thows _com_error
	static BOOL IsCaseHistoryBilled(IN const long nCaseHistoryID);	

	// (j.jones 2009-08-06 11:10) - PLID 7397 - added check to see if the case is linked to a PIC
	static BOOL IsCaseHistoryInProcInfo(IN const long nCaseHistoryID);

// Implementation
protected:
	// Pull the values from the database, and put them onto the dialog
	void Load(long nCaseHistoryID);
	// (j.jones 2006-12-13 16:09) - PLID 23578 - load new info. from a surgery
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-31 15:03) - PLID 35378 - we now support multiple preference cards
	void InitializeFromPreferenceCards(CArray<long, long> &arynPreferenceCardIDs);
	void AddPreferenceCard(long nPreferenceCardID);
	// Save the values from the dialog into the database
	BOOL Save();

	BOOL SaveAndClose();	// (j.jones 2008-02-27 17:09) - PLID 29108 - calls Save and closes the dialog

	void DisplayAppointmentDesc();

	// Print this particular case history (this will save and close the dialog (unless something fails in which case it'll remain open))
	BOOL PrintCaseHistory(IN const BOOL bPreview);

	// Validates the on-screen data, and builds a big sql string, containing at 
	// least one, but maybe several INSERT INTO, UPDATE, and DELETE statements
	// This is a helper function used by the Save()
	// Throws _com_error, CException*
	//TES 1/8/2007 - PLID 23575 - Added an nAuditTransactionID (like how the EMR code does it), which the code that calls
	// this function needs to either Commit or Rollback.
	BOOL ValidateAndBuildSaveString(OUT CString &strSqlString, OUT long &nAuditTransactionID);
	// (j.jones 2006-12-13 17:04) - PLID 23578 - made two separate functions for detail saving
	BOOL ValidateAndBuildSaveDetailsCreateString(OUT CString &strSqlString, OUT long &nAuditTransactionID); // Used by ValidateAndBuildSaveString
	BOOL ValidateAndBuildSaveDetailsUpdateString(OUT CString &strSqlString, OUT long &nAuditTransactionID); // Used by ValidateAndBuildSaveString

	// Makes sure a provider is selected if so, returns TRUE and passes the provider id back, otherwise gives a message and returns FALSE
	//TES 1/8/2007 - PLID 23575 - Also outputs the name, for auditing purposes.
	BOOL ValidateCurProviderID(OUT long &nProviderID, OUT CString &strProviderName);

	// Makes sure a location is selected if so, returns TRUE and passes the location id back, otherwise gives a message and returns FALSE
	//TES 1/9/2007 - PLID 23575 - Also outputs the name, for auditing purposes.
	BOOL ValidateCurLocationID(OUT long &nLocationID, OPTIONAL OUT CString *pstrLocationName = NULL);

	// Makes sure a POS is selected if so, returns TRUE and passes the POS id back, otherwise gives a message and returns FALSE
	//TES 1/9/2007 - PLID 23575 - Also outputs the name, for auditing purposes.
	BOOL ValidateCurPOSID(OUT long &nPOSID, OUT CString &strPOSName);

	// Warns the user if the procedures don't match the attached surgery
	BOOL ValidateCurProcedures();

	// (j.jones 2008-05-21 16:20) - PLID 29223 - we validate anesthesia and 
	// facility dates independently, as the checks will only occur if we have
	// anesthesia or facility fee items
	BOOL ValidateAnesthesiaDates();
	BOOL ValidateFacilityDates();

	// (a.walling 2007-02-27 14:02) - PLID 24944 - Return the span between the two times in minutes
	long MinuteSpan(IN COleDateTime dtStart, IN COleDateTime dtEnd);

	// For easy and consistent addition/removal of case history details
	void RemoveDetail(const long nIndex);
	//TES 2/2/2005 - If bTryToIncrement is true, then if nItemID and nItemType is in the list, it will add dblQuantity to the quantity, 
	//and ignore the other fields (cost and whatnot).
	// (j.jones 2008-05-20 17:44) - PLID 29249 - added bIsSerializedProduct parameter
	//TES 7/16/2008 - PLID 27983 - Added bIsLinkedToProducts parameter
	// (j.jones 2008-09-15 11:15) - PLID 31375 - added varIsBillableProductForLoc which will be
	// TRUE if it is a product that is billable for the current location,
	// FALSE if not billable for the location (or not a product)
	// and NULL if we didn't check the setting
	// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
	void AddDetail(const long nItemID, const ECaseHistoryDetailItemType nItemType, const CString &strItemName, const double &dblQuantity, const COleCurrency &cyAmount, const COleCurrency &cyCost, const bool &bBillable, bool bTryToIncrement, BOOL bIsSerializedProduct, BOOL bIsLinkedToProducts, _variant_t varIsBillableProductForLoc);

	// Array of ChargedProductItemLists
	CArray<CaseHistoryDetailProductItemList*,CaseHistoryDetailProductItemList*> m_aryCaseHistoryDetailProductItems;
	void PromptForCaseHistoryDetailProductItems(BOOL bAllowQuantityChange = TRUE);
	BOOL AddCaseHistoryDetailProductItem(long ServiceID, double &dblQuantity, long CaseHistoryDetailID, long &CaseHistoryDetailProductItemListID, BOOL bAllowQuantityChange = TRUE);
	void DeleteAllFromCaseHistoryDetailProductItemsArray();
	BOOL MarkAllDeletedFromCaseHistoryDetailProductItemsArray();
	void MarkCaseHistoryDetailProductItemListDeleted(long CaseHistoryDetailProductItemListID);
	CString GetProductItemWhereClause();
	long NewCaseHistoryDetailProductItemListID();
	void AddToCaseHistoryDetailProductItemsArray(long CaseHistoryDetailID, long CaseHistoryDetailProductItemListID, CDWordArray &adwProductItemIDs);
	long LoadIntoCaseHistoryDetailProductItemsArray(long CaseHistoryDetailID);
	BOOL ChangeProductItems(long nRow, double dblQuantity);
	
	// For easy and consistent access to the time edit boxes
	void SetDlgItemTime(UINT nID, const COleDateTime &dtTime, BOOL bDontSetIfMidnight);
	void SetDlgItemTime(UINT nID, const CString &strTime, BOOL bDontSetIfMidnight);
	COleDateTime GetDlgItemTime(UINT nID);

	CArray<long,long> m_arProcedureIDs;
	//TES 1/9/2007 - PLID 23575 - Goes through m_arProcedureIDs, compares it to m_arLastSavedProcedureIDs, fills the arrays
	// with the IDs and Names of the procedures that have been added and removed.
	void ParseProcedures(OUT CArray<long,long> &arAddedIDs, OUT CStringArray &saAddedNames, OUT CArray<long,long> &arRemovedIDs, OUT CStringArray &saRemovedNames);
	CString m_strProcedureList;
	void DrawProcedureLabel(CDC *pdc);
	void DisplayProcedureInfo();
	void OnMultiProcedure();
	CString GetStringOfProcedures();
	BOOL m_bIsProcedureListHidden;
	void DoClickHyperlink(UINT nFlags, CPoint point);
	CRect m_rcMultiProcedureLabel;
	BOOL m_bHasProcedureBeenChanged;

	void OnAnesthesiaTimeChanged(long nTotalMinutes, BOOL bPlaceOfServiceChanged = FALSE);
	void OnFacilityTimeChanged(long nTotalMinutes, BOOL bPlaceOfServiceChanged = FALSE);

	BOOL CheckAllowAddAnesthesiaFacilityCharge(long nServiceID);

	// (a.walling 2007-02-23 11:50) - PLID 24451 - Sets the control's value to the current time.
	void SetCurrentTime(IN long nControlID);

	// (j.jones 2008-02-27 16:34) - PLID 29126 - if we have a linked allocation,
	// and it is uncompleted, try to complete it, and if we don't have one
	// then search for one, try to link one, and complete that
	BOOL TryCompleteInvAllocation();

	// (j.jones 2008-03-06 09:56) - PLID 29203 - compares products in the allocation to the
	// case history, will ask to add any that don't exist in the allocation
	void CheckAddProductsFromAllocation(InvUtils::AllocationMasterInfo *pInfo);

	// (j.jones 2008-03-07 12:50) - PLID 29231 - compares products in the allocation to the
	// case history, will warn if quantities don't match, return FALSE if they wish to cancel
	// completing the case history
	BOOL CheckWarnAllocationQuantityMismatch(InvUtils::AllocationMasterInfo *pInfo);

	// (j.jones 2008-02-29 08:49) - PLID 29102 - lets us know if we're allowed to unlink the allocation
	BOOL CanUnlinkAllocation();

protected:	
	BOOL m_bIsModified;
	NXDATALISTLib::_DNxDataListPtr m_dlCaseHistoryDetails;
	CDWordArray m_aryDeleteDetailIDs;
	NXDATALISTLib::_DNxDataListPtr m_dlProviders;
	NXDATALISTLib::_DNxDataListPtr m_dlLocations;
	NXDATALISTLib::_DNxDataListPtr m_dlPOS;
	NXDATALISTLib::_DNxDataListPtr m_dlProcedures;
	NXDATALIST2Lib::_DNxDataListPtr m_DischargeStatusCombo;
	// (j.jones 2008-02-26 13:59) - PLID 29102 - added allocation combo
	NXDATALIST2Lib::_DNxDataListPtr m_InvAllocationCombo;
	// (j.jones 2008-03-10 14:38) - PLID 29233 - added allocation contents list
	NXDATALIST2Lib::_DNxDataListPtr m_InvAllocationContentsList;
	_variant_t m_varCompletedDate;

	NXTIMELib::_DNxTimePtr m_nxtAnesthStart;
	NXTIMELib::_DNxTimePtr m_nxtAnesthEnd;
	NXTIMELib::_DNxTimePtr m_nxtFacilityStart;
	NXTIMELib::_DNxTimePtr m_nxtFacilityEnd;

	//Fake mutex
	BOOL m_bIsScanningBarcode;

	long m_CurAnesthMinutes;
	long m_CurFacilityMinutes;

	long m_nCurLocationID;
	void PromptInventoryLocationChanged();
	double GetUnsavedQuantityCount(long nServiceID);

	//TES 1/8/2007 - PLID 23575 - We're auditing now, so we need to remember the initial value of every variable.
	long m_nLastSavedProviderID;
	CString m_strLastSavedProviderName;
	long m_nLastSavedLocationID;
	CString m_strLastSavedLocationName;
	long m_nLastSavedPOSID;
	CString m_strLastSavedPOSName;
	CArray<long,long> m_arLastSavedProcedureIDs;
	long m_nLastSavedAppointmentID;
	CString m_strLastSavedAppointmentDesc;
	COleDateTime m_dtLastSavedSurgeryDate;
	CString m_strLastSavedNotes;
	COleDateTime m_dtLastSavedPreOpIn, m_dtLastSavedPreOpOut, m_dtLastSavedSurgeonIn, m_dtLastSavedSurgeonOut, 
		m_dtLastSavedOpRoomIn, m_dtLastSavedOpRoomOut, m_dtLastSavedRecoveryIn, m_dtLastSavedRecoveryOut,
		m_dtLastSavedAnesthesiaIn, m_dtLastSavedAnesthesiaOut, m_dtLastSavedFacilityIn, m_dtLastSavedFacilityOut,
		m_dtLastSaved23HourRoomIn, m_dtLastSaved23HourRoomOut;
	COleDateTime	m_dtLastEnteredAnesthesiaIn,	m_dtLastEnteredFacilityIn,
					m_dtLastEnteredAnesthesiaOut,	m_dtLastEnteredFacilityOut;
	BOOL m_bLastSavedCompleted;
	CArray<SavedCaseHistoryDetail,SavedCaseHistoryDetail&> m_arLastSavedDetails;
	bool m_bLastSavedDetailsLoaded;
	CString m_strLastSavedName;
	long m_nLastSavedAnesthesiaMinutes;
	long m_nLastSavedFacilityMinutes;
	long m_nLastSavedDischargeStatusID;
	CString m_strLastSavedDischargeStatus;

	// (j.jones 2008-02-27 09:26) - PLID 29104 - added allocation link information
	long m_nLastSavedAllocationID;
	CString m_strLastSavedAllocationDescription;
	

	// (j.gruber 2008-07-07 17:30) - PLID 15807
	NXDATALIST2Lib::_DNxDataListPtr m_pAnesType;

	// (a.wilson 02/19/2014) PLID 60774
	NXDATALIST2Lib::_DNxDataListPtr m_pPreOpDiagnosisSearch;
	NXDATALIST2Lib::_DNxDataListPtr m_pPostOpDiagnosisSearch;
	NXDATALIST2Lib::_DNxDataListPtr m_pPreOpDiagnosisList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPostOpDiagnosisList;
	// (a.wilson 2014-02-25 16:07) - PLID 61034
	bool m_bPreOpDiagCodesChanged, m_bPostOpDiagCodesChanged;

	CString m_strLastAnesType;

	void LoadAnesthesia();

	//TES 7/16/2008 - PLID 27983 - Goes through all details, and for any linked service codes, removes them and prompts the user
	// for which products to replace them with.
	void ReplaceLinkedServices();

	//(c.copits 2010-09-17) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	ADODB::_RecordsetPtr GetBestUPCProduct(_variant_t barcode);

protected:

	// (j.jones 2008-02-26 13:52) - PLID 29108 - added ability to create allocations from the case
	// (j.jones 2008-03-10 15:39) - PLID 29233 - added OnSelChosenCaseAllocationList
	// Generated message map functions
	//{{AFX_MSG(CCaseHistoryDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRequeryFinishedCasehistoryDetailsList(short nFlags);
	afx_msg void OnRButtonDownCasehistoryDetailsList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCasehistorydetailsAdd();
	afx_msg void OnCasehistorydetailsRemove();
	afx_msg void OnEditProductItems();
	afx_msg void OnLButtonDownCasehistoryDetailsList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnPrintBtn();
	afx_msg void OnSelChangedProviderList(long nNewSel);
	afx_msg void OnChangeSurgeryDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillFocusPreopInEdit();
	afx_msg void OnKillFocusOproomInEdit();
	afx_msg void OnKillFocusOproomOutEdit();
	afx_msg void OnKillFocusPreopOutEdit();
	afx_msg void OnKillFocusRecoveryInEdit();
	afx_msg void OnKillFocusRecoveryOutEdit();
	afx_msg void OnKillFocusAnesthesiaStartEdit();
	afx_msg void OnKillFocusAnesthesiaStopEdit();
	afx_msg void OnKillFocusFacilityStartEdit();
	afx_msg void OnKillFocusFacilityStopEdit();
	afx_msg void OnChangeNotesEdit();
	afx_msg void OnChangeNameEdit();
	afx_msg void OnCompletedCheck();
	afx_msg void OnEditingFinishingCasehistoryDetailsList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedCasehistoryDetailsList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingStartingCasehistoryDetailsList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnSelChosenLocationCombo(long nRow);
	afx_msg void OnSelChangedPOSCombo(long nNewSel);
	afx_msg void OnColumnClickingCasehistoryDetailsList(short nCol, BOOL FAR* bAllowSort);
	virtual void OnCancel();
	afx_msg void OnKillFocusSurgeonInEdit();
	afx_msg void OnKillFocusSurgeonOutEdit();
	afx_msg void OnKillFocus23hourroomInEdit();
	afx_msg void OnKillFocus23hourroomOutEdit();
	afx_msg void OnBtnChangeAppt();
	afx_msg void OnBtnClearAppt();
	afx_msg void OnBtnGotoAppt();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSelChosenProcedureList(long nRow);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnChangeEditTotalCaseAnesthMinutes();
	afx_msg void OnChangeEditTotalCaseFacilityMinutes();
	afx_msg void OnKillfocusEditTotalCaseAnesthMinutes();
	afx_msg void OnKillfocusEditTotalCaseFacilityMinutes();
	afx_msg void OnSelChosenCaseHistoryPosCombo(long nRow);
	afx_msg void OnChangeColumnSortFinishedCasehistoryDetailsList(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending);
	afx_msg void OnRequeryFinishedDischargeStatusCombo(short nFlags);
	afx_msg void OnBtnEditDischargeStatus();
	afx_msg void OnAutotimeIn23();
	afx_msg void OnAutotimeInAnes();
	afx_msg void OnAutotimeInFac();
	afx_msg void OnAutotimeInOproom();
	afx_msg void OnAutotimeInPreop();
	afx_msg void OnAutotimeInRecovery();
	afx_msg void OnAutotimeInSurgeon();
	afx_msg void OnAutotimeOut23();
	afx_msg void OnAutotimeOutAnes();
	afx_msg void OnAutotimeOutFac();
	afx_msg void OnAutotimeOutOproom();
	afx_msg void OnAutotimeOutPreop();
	afx_msg void OnAutotimeOutRecovery();
	afx_msg void OnAutotimeOutSurgeon();
	afx_msg void OnBtnCreateAllocation();
	afx_msg void OnRequeryFinishedCaseAllocationList(short nFlags);
	afx_msg void OnSelChangingCaseHistoryLocationCombo(long FAR* nNewSel);
	afx_msg void OnSelChangingCaseAllocationList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenCaseAllocationList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingCaseHistoryAnesType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnRequeryFinishedCaseHistoryAnesType(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (a.wilson 2014-02-24 16:55) - PLID 60774
	void SelChosenPreopDiagnosisSearch(LPDISPATCH lpRow);
	void SelChosenPostopDiagnosisSearch(LPDISPATCH lpRow);
	void GenerateDiagnosisIDListStrings(OUT CString &strPreOpIDs, OUT CString &strPostOpIDs);
	void GenerateDiagnosisCodeListStrings(OUT CString &strPreOpCodes, OUT CString &strPostOpCodes);
	void UpdateDiagnosisListColumnSizes(bool bUpdatePreOp = true, bool bUpdatePostOp = true); 
	void RButtonDownPreopDiagnosisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownPostopDiagnosisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (a.wilson 2014-02-25 16:06) - PLID 61034
	void AuditDiagnosisListChanges(long & nAuditTransactionID);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CASEHISTORYDLG_H__14B0BAA3_A91B_4DA1_BFC7_726E5B85F332__INCLUDED_)
