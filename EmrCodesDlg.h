#pragma once
#include "Client.h"
#include "EmrHelpers.h"
#include "EmrTreeWnd.h"
#include "EmrRc.h"
#include "EmrCodesDlg.h"

// (a.walling 2014-03-12 12:31) - PLID 61334 - #import of API in stdafx causes crash in cl.exe
//#include "NxAPI.h"


// CEmrCodesDlg dialog

//TES 2/11/2014 - PLID 60740 - New dialog for just the coding-related parts of CEMNMoreInfoDlg
// (s.dhole 2014-02-14 14:48) - PLID 60742 Move Code from <More Info> - Procedure 
class CEmrCodesDlg : public CNxDialog, public Emr::InterfaceAccessImpl<CEmrCodesDlg>
{
	DECLARE_DYNAMIC(CEmrCodesDlg)

public:
	CEmrCodesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEmrCodesDlg();

	

	//TES 2/12/2014 - PLID 60740 - Load this dialog from the given EMN
	void Initialize(CEMN *pEMN);
	void RefreshChargeList();

	// (r.farnworth 2014-02-18 12:29) - PLID 60746
	//TES 2/26/2014 - PLID 60807 - Added ICD10
	void RemoveDiagCode(long nICD9DiagCodeID, long nICD10DiagCodeID);
	void RefreshDiagCodeList();

	//TES 10/30/2008 - PLID 31269 - Call after a problem gets changed to refresh all the problem icons.
	void HandleProblemChange(CEmrProblem* pChangedProblem);

	// (a.walling 2007-07-03 13:12) - PLID 23714 - Updates any info on the moreinfo dialog due to table checkers
	void UpdateChangedInfo();

	//TES 2/20/2014 - PLID 60748 - For functionality moed from CEMNMoreInfoDlg::OnMoreInfoChanged()
	void OnCodesChanged();
	
	//TES 2/12/2014 - PLID 60740 - The EMN associated with this dialog
	CEMN *m_pEMN;
	CEMN* GetEMN()
	{
		return m_pEMN;
	}

	void SetReadOnly(BOOL bReadOnly);

	BOOL m_bIsTemplate; // (r.farnworth 2014-02-14 15:15) - PLID 60745


	enum { IDD = IDD_EMR_CODES_DLG };
	CNxIconButton	m_btnUseEMChecklist; // (r.farnworth 2014-02-14 15:15) - PLID 60745
	void RemoveCharge(EMNCharge *pCharge);	//DRT 1/11/2007 - PLID 24220
	CBoolGuard m_bIsSaving; //(j.camacho 2014-07-30) plid 62641 - Prevent multiple saving of EMN when codes dlg is opened.

protected:
// (a.walling 2007-07-03 11:46) - PLID 23714 - Use table checkers
	// (a.walling 2007-10-04 10:17) - PLID 23714 - Added CPTCode checker
	// (c.haag 2008-07-07 12:50) - PLID 30607 - Todo alarms
	// (d.lange 2011-03-24 11:52) - PLID 42136 - Added checker for Assistant/Tech
	CTableChecker m_checkCPTCodeT;
	CTableChecker m_checkDiagCodes;

	BOOL m_bInitialized;
	BOOL m_bReadOnly;
	// (s.dhole 2014-02-14 14:48) - PLID 60742 Move Code from <More Info>
	CBrush m_brush;
	COLORREF m_bkgColor;
	long m_nPatientID;

	
	CNxIconButton	m_btnCreateQuote;
	CNxIconButton	m_btnCreateBill;
	CNxIconButton	m_btnAddCharge;
	CNxIconButton	m_btnAssignInsResp; 
	CNxIconButton	m_btnMoveDiagUp;
	CNxIconButton	m_btnMoveDiagDown;
	CNxIconButton	m_btnEditServices;
	// (b.savon 2014-02-26 10:40) - PLID 60805 - UPDATE - Add new button, "Existing Quote" in the procedure codes section of the <Codes> topic
	CNxIconButton	m_btnExistingQuote;
	// (c.haag 2014-03-05) - PLID 60930 - QuickList button
	CNxIconButton m_btnQuickList;
	// (b.savon 2014-03-31 08:20) - PLID 60991 - Add a button to access NexCode from the <Codes> topic in the EMN and add the code to the datalist
	CNxIconButton m_btnNexCode;
	// (c.haag 2014-07-16) - PLID 54905 - Add a button to link all EMN diagnosis codes to all EMN charges
	CNxIconButton m_btnLinkCodesToAllCharges;

	CNxIconButton m_btnSearchQueue;//(j.camacho 7/16/2014) - plid 62636 - Adding Search Queue button to dialog.

	NxButton	m_checkSendBillToHL7;
	NxButton	m_checkHideUnbillableCPTCodes; // (j.jones 2012-02-17 10:33) - PLID 47886

	CNxColor	m_bkg2;
	CNxColor	m_bkg4;
	CNxColor	m_bkg5;

	HICON m_hInfoButtonIcon;

	// (c.haag 2014-03-17) - PLID 60929 - QuickList star icons
	HICON m_hStarFilledIcon;
	HICON m_hStarUnfilledIcon;

	// (r.farnworth 2014-02-14 15:15) - PLID 60745
	NXDATALIST2Lib::_DNxDataListPtr m_pVisitTypeCombo; //(r.farnworth) 2/14/2014
	NXDATALIST2Lib::_DNxDataListPtr  m_BillList;
	//TES 2/26/2014 - PLID 60807 - This is an NxDataList2 now
	NXDATALIST2Lib::_DNxDataListPtr m_DiagCodeList; // (r.farnworth 2014-02-17 15:48) - PLID 60746
	//TES 2/26/2014 - PLID 60806 - Replaced combo with search box
	NXDATALIST2Lib::_DNxDataListPtr m_DiagSearch;
	
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	// (a.walling 2007-08-06 12:07) - PLID 23714 - Refresh the procedure list
	void RefreshProcedureList();

	void LinkChargeProblems();
	void EditChargeProblems();
	

	// (j.jones 2008-07-22 09:05) - PLID 30792 - this function will show or hide
	// the problem icon column in the charge list, based on whether any charges have the icon
	void ShowHideProblemIconColumn_ChargeList();
		// (c.haag 2009-05-28 15:22) - PLID 34312 - This function is called when the user wants to link something
	// in the more info section with one or more existing problems. Returns TRUE if at least one problem was
	// added to the output array
	BOOL LinkProblems(CArray<CEmrProblemLink*,CEmrProblemLink*>& aryEMRObjectProblemLinks,
					  EMRProblemRegardingTypes eprtType, long nEMRRegardingID,
					  OUT CArray<CEmrProblemLink*,CEmrProblemLink*>& aNewProblemLinks);
	// (j.jones 2008-06-03 12:01) - PLID 30062 - added ability to add quote charges
	BOOL AddQuoteToBillList(long nQuoteID);

	// (r.farnworth 2014-04-01 08:37) - PLID 61608
	void SetICD9ForDiagCodeInfo(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (r.farnworth 2014-02-17 16:14) - PLID 60746 - MOVE - Move the diag codes section of <More Info> to the new <Codes> topic
	// (c.haag 2014-03-24) - PLID 60930 - Return the new row
	NXDATALIST2Lib::IRowSettingsPtr AddDiagCode(EMNDiagCode *pDiag);
	//TES 2/26/2014 - PLID 61079 - Pulled most "add code" functionality into this function
	//(s.dhole 3/6/2015 10:42 AM ) - PLID 64549
	bool AddDiagCodeBySearchResults(class CDiagSearchResults &results);
	// (c.haag 2014-03-06) - PLID 60930 - Adds a CDiagSearchResults object to the EMN diagnosis codes list
	NXDATALIST2Lib::IRowSettingsPtr AddDiagCodeBySearchResults_Raw(class CDiagSearchResults &results, NexGEMMatchType matchType);
	// (c.haag 2014-03-17) - PLID 60929 - Updates our QuickList ID for a given diagnosis code
	void UpdateDiagQuickListID(EMNDiagCode *pDiag);
	// (c.haag 2014-03-17) - PLID 60929 - Updates the QuickList icon for a given row
	void UpdateRowQuickListIcon(LPDISPATCH lpRow);
	void EnsureDiagCodeColors();
	void ShowHideProblemIconColumn_DiagList();
	void UpdateDiagCodeArrows();
	void EditDiagnosisProblems();
	void LinkDiagnosisProblems();
	//TES 2/28/2014 - PLID 61080 - This now supports ICD-10 codes
	void RemoveDiagCodeFromWhichCodes(long nID, CString strCode, long nICD10ID, CString strICD10Code);
	//TES 2/27/2014 - PLID 60807 - Resize based on the user's preferences
	void UpdateDiagnosisListColumnSizes();
	//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
	CString GetDiagCodeListForInterface(EMNCharge* pCharge);
	NXDATALIST2Lib::IFormatSettingsPtr GetCPTMultiCategoryCombo(EMNCharge* pCharge);
	void SetCPTCategoryCombo(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNCharge* pCharge);
	void ForceShowColumn(short iColumnIndex, long nPercentWidth, long nPixelWidth);

	//TES 2/26/2014 - PLID 60807 - Added utility function to search by both IDs
	NXDATALIST2Lib::IRowSettingsPtr FindRowInDiagList(long nICD9ID, long nICD10ID);

	void EnableControls();

		// (a.walling 2007-07-03 13:08) - PLID 23714
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);

	// (j.gruber 2009-12-24 10:17) - PLID 15329 - warning if the emn is already billed
	BOOL m_bHadBillWarning;
	BOOL WarnChargeAlreadyBilled();

	//DRT 1/12/2007 - PLID 24234 - Sync a single datalist row with an EMNCharge pointer
	void SyncBillListRowWithEMNCharge(EMNCharge *pCharge, NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (j.dinatale 2012-01-19 09:11) - PLID 47620 - moved the show charge split dialog logic into a function
	long ShowChargeRespDlg();
// (j.jones 2007-08-30 11:26) - PLID 27221 - added optional parameter for pending E/M checklist audit information
	// (j.jones 2008-06-03 12:14) - PLID 30062 - added quantity
	// (j.jones 2008-06-04 16:20) - PLID 30255 - added nQuoteChargeID
	// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
	// (j.dinatale 2012-01-05 10:12) - PLID 47464 - added Insured Party ID
	// (j.jones 2012-08-22 09:38) - PLID 50486 - moved insured party ID to be a required parameter
	BOOL AddChargeToBillList(long nServiceID, CString strCode, CString strSubCode, CString strDescription, long nInsuredPartyID, BOOL bBillable, COleCurrency cyPrice, long nCategory=-1, long nCategoryCount=0, double dblQuantity = 1.0, CArray<CPendingAuditInfo*, CPendingAuditInfo*> *paryPendingEMAuditInfo = NULL, long nQuoteChargeID = -1);
	
	CWinThread *m_pPrepareBillListThread;
	BOOL m_bPreparingBillList;

	void TrySetVisitType(long nVisitTypeID, CString strVisitTypeID); // (r.farnworth 2014-02-14 15:15) - PLID 60745

	// (b.savon 2014-03-06 08:22) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
	void AddICD10DisplayColumn(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNDiagCode *pDiag);
	void SetNexGEMMatchColumnFormat(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNDiagCode *pDiag);
	void HandleNexGEMMultiMatchColumn(NXDATALIST2Lib::IRowSettingsPtr pRow);
	struct DiagnosisCodeCommit;
	scoped_ptr<DiagnosisCodeCommit> m_pDiagCodeCommitMultiMatch;

	// (b.savon 2014-03-12 07:34) - PLID 61310 - Handle adding a NexCode from the <Codes> pane datalist
	void HandleNexCodeColumn(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (b.savon 2014-03-12 12:43) - PLID 61330 - When an ICD10 code is matched from Multimatch or NexCode and the
	// ICD9 is tied to a charge, update the which codes to reflect the matched ICD10
	void EnsureEMRChargesToDiagCodes();

	// (r.farnworth 2014-03-10 09:16) - PLID 61246
	// (c.haag 2014-03-24) - PLID 60930 - Added a silent mode
	void UpdateLegacyCode(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bSilent);

	// (c.haag 2014-04-09) - PLID 60929 - Update the QuickList icon for every row.
	void UpdateAllQuickListIcons();

	//(j.camacho 7/23/2014) - search queue
	std::vector<CString> vSearchQueueKeywords;
	void SetSearchQueueList();
	void OnPostShowWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnSaveSearchQueueList(WPARAM wParam, LPARAM lParam); //(j.camacho 2014-07-30) plid 62641 - Handler for when codes dlg is selected.
	COleDateTime m_dtLastSearchUpdate;
	
	// (j.jones 2014-12-23 13:45) - PLID 64491 - moved the ability to remove a diag code to its own function
	void RemoveDiagCode(NXDATALIST2Lib::IRowSettingsPtr pDiagRow);

	// (j.jones 2008-07-22 09:05) - PLID 30792 - this function will show or hide
	// the problem icon column in the charge list, based on whether any charges have the icon
	// (c.haag 2014-03-05) - PLID 60930 - Added QuickList button

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditServices();
	afx_msg void OnEditCPT();

	
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedBtnUseEmChecklist();
	afx_msg void OnBnClickedBtnCreateBillCode();
	afx_msg void OnBnClickedBtnCreateQuoteCode();
	afx_msg void OnBnClickedBtnAddChargeCode();
	afx_msg void OnBnClickedBtnChargeSplitCode();
	afx_msg void OnBnClickedHideUnbillableCptCodesCode();
	afx_msg void OnBnClickedSendBillToHl7Code();
	afx_msg LRESULT OnPrepareBillListCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelChosenDiagDropdown(long nRow);
	afx_msg void OnBtnMoveDiagUp();
	afx_msg void OnBtnMoveDiagDown();
	afx_msg void OnBtnQuickList();
	
	void EditingFinishedBillCode(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingBillCode(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingStartingBillCode(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void LeftClickBillCode(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownBillCode(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
		
	afx_msg void OnEditInventory();
	afx_msg void OnSelChosenVisitType(LPDISPATCH lpRow);
	afx_msg void OnTrySetSelFinishedVisitType(long nRowEnum, long nFlags);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnEmnChargeAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnChargeChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnDiagAdded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedExistingQuote();
	void OnSelChosenEmnDiagSearch(LPDISPATCH lpRow);
	void DragEndDiags(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	void SelChangedDiags(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void LeftClickDiags(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownDiags(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishedDiags(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedAddNexcodeBtn();
	afx_msg void OnLinkCodesToAllCharges();
	afx_msg void OnBtnClickedSearchQueue(); //(j.camacho 8/12/2014) plid 62638
	// (j.jones 2014-12-23 13:20) - PLID 64491 - added ability to replace a diagnosis code
	afx_msg void OnReplaceDiagCode();	
};
