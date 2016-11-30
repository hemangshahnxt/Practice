#if !defined(AFX_EMRACTIONDLG_H__32BDA6A3_1921_4A14_9EF0_C6D470895DFF__INCLUDED_)
#define AFX_EMRACTIONDLG_H__32BDA6A3_1921_4A14_9EF0_C6D470895DFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrActionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrActionDlg dialog

#include <NxUILib/NxStaticIcon.h>
#include "EmrUtils.h"
#include "EMRTodoActionConfigDlg.h"

class CEMN;

enum ActionListColumn {	//Applies to all the lists of selected actions EXCEPT charges
	alcActionID = 0,
	alcID = 1,
	alcName = 2,
	alcName2 = 3,
	alcSortOrder = 4,
	alcDataType = 5, // (b.savon 2014-07-08 08:04) - PLID 62760 
	alcPopup = 6,
	alcProblemBitmap = 7, 	// (c.haag 2008-07-16 17:15) - PLID 30723
	alcProblemArray = 8,	// (c.haag 2008-07-17 10:17) - PLID 30723
	alcLabAnatomyIDs, // (z.manning 2011-06-28 16:08) - PLID 44347
};

enum ChargeListColumn {	//Applies to charge list only
	clcActionID = 0,
	clcID,
	clcName,
	clcSubCode,			//TES 5/12/2008 - PLID 29577 - Added in the SubCode
	clcDescription,
	clcPrice,			// (j.jones 2010-01-11 09:56) - PLID 24504 - added Price
	clcSortOrder,
	clcPopup,
	clcPrompt,			//Charge List Only
	clcDefQty,			//Charge List Only
	clcDefMod1,			//Charge List Only
	clcDefMod2,			//Charge List Only
	clcDefMod3,			//Charge List Only
	clcDefMod4,			//Charge List Only
	clcProblemBitmap,	// (c.haag 2008-07-16 17:15) - PLID 30723
	clcProblemArray,	// (c.haag 2008-07-17 10:17) - PLID 30723
	clcLabAnatomyIDs, // (z.manning 2011-06-28 16:08) - PLID 44347
};

// (c.haag 2008-06-03 08:35) - PLID 30221 - Columns for the To-Do Alarm list
enum TodoActionListColumn {
	ectlActionID = 0,
	ectlCategoryID = 1,
	ectlMethod = 2,
	ectlNotes = 3,
	ectlAssignTo = 4,
	ectlPriority = 5,
	ectlRemindType = 6,
	ectlRemindInterval = 7,
	ectlDeadlineType = 8,
	ectlDeadlineInterval = 9,
	ectlSortOrder = 10,
	ectlLabAnatomyIDs, // (z.manning 2011-06-28 16:08) - PLID 44347
};

// (j.jones 2010-01-11 08:54) - PLID 24504 - added more enums
enum CPTDropdownColumns {

	cdcID = 0,
	cdcCode,
	cdcSubCode,
	cdcDescription,
	cdcPrice,
};

enum ProductDropdownColumns {

	pdcID = 0,
	pdcCategory,
	pdcSubCode,
	pdcSupplier,
	pdcName,
	pdcPrice,
	pdcLastCost,
};

class CEmrActionDlg : public CNxDialog
{
// Construction
public:
	CEmrActionDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrActionDlg();
	EmrActionObject m_SourceType;
	long m_nSourceID;

	// (c.haag 2006-02-28 16:34) - PLID 12763 - If we are editing the actions
	// of an existing EMRInfo item, then this is the ID of that item. We use
	// this value to ensure the item we're editing does not show up in any
	// dropdowns (otherwise, you could have an item spawn itself into infinity)
	long m_nOriginatingID;

	void ChangeChargeType();

	void SetCurrentEMN(CEMN* pEMN);

// Dialog Data
	//{{AFX_DATA(CEmrActionDlg)
	enum { IDD = IDD_EMR_ACTION_DLG };
	CNxIconButton	m_btnTodoDown;
	CNxIconButton	m_btnTodoUp;
	NxButton	m_btnSvcCode;
	NxButton	m_btnProduct;
	CNxIconButton	m_btnMintItemDown;
	CNxIconButton	m_btnMintItemUp;
	CNxIconButton	m_btnItemDown;
	CNxIconButton	m_btnItemUp;
	CNxIconButton	m_btnChargeDown;
	CNxIconButton	m_btnChargeUp;
	CNxIconButton	m_btnDiagDown;
	CNxIconButton	m_btnDiagUp;
	CNxIconButton	m_btnLabDown;
	CNxIconButton	m_btnLabUp;
	CNxIconButton	m_btnMedicationDown;
	CNxIconButton	m_btnMedicationUp;
	CNxIconButton	m_btnEmnDown;
	CNxIconButton	m_btnEmnUp;
	CNxIconButton	m_btnProcedureDown;
	CNxIconButton	m_btnProcedureUp;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnChargeEdit;
	CNxIconButton	m_btnDiagCodeEditBtn;
	CNxIconButton	m_btnNewTodoTask;
	CNxStatic	m_nxstaticActionCaption;
	CNxStatic	m_nxstaticMedicationLabel;
	CNxStatic	m_nxstaticEmrItemLabel;
	CNxStatic	m_nxstaticEmnTemplateLabel;
	CNxStatic	m_nxstaticEmrActionAnchorSmall;
	CNxStatic	m_nxstaticEmrActionAnchorLarge;
	CNxStatic	m_nxstaticProcedureLabel;
	CNxStatic	m_nxstaticEmnTemplateItemLabel;
	CNxStatic	m_nxstaticEmrTodoLabel;
	// (j.jones 2016-01-25 15:18) - PLID 67998 - added checkbox to include free text meds in the medication search
	NxButton	m_checkIncludeFreeTextMeds;
	// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color
	CNxStaticIcon m_icoAboutEMRActiontMedsColors;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrActionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	void UpdateTodoRow(NXDATALISTLib::IRowSettingsPtr& pRow, CEMRTodoActionConfigDlg& dlg);

	// (z.manning 2011-06-28 09:59) - PLID 44347
	BOOL SourceTypeAllowsPerAnatomicLocation();
	short GetAnatomicLocationColumnFromDestType(EmrActionObject eaoDestType);
	short GetAnatomicLocationColumnFromDatalist(NXDATALISTLib::_DNxDataListPtr pdl);
	void UpdateRowForAnatomyChange(NXDATALISTLib::IRowSettingsPtr pRow, const short nLabAnatomyCol);

	// (j.jones 2012-08-24 15:14) - PLID 52091 - Added so you could get the DestID column index
	// from any given datalist. Not valid on todo lists.
	short GetDestIDColumnFromDatalist(NXDATALISTLib::_DNxDataListPtr pdl);

	void AppendCommonRightClickMenuOptions(CMenu *pMenu);

protected:
	// (c.haag 2008-07-17 10:39) - PLID 30723 - Update the appearance of the problem flag column in a given row
	void UpdateActionProblemRow(NXDATALISTLib::IRowSettingsPtr& pRow, short nColProblemBitmap, short nColProblemAry);

	// (c.haag 2008-07-17 10:43) - PLID 30723 - Deletes all arrays of problems embedded in all lists
	void DeleteProblemArrays();

	// (c.haag 2008-07-17 10:43) - PLID 30723 - Deletes all arrays of problems embedded in a specified list
	void DeleteProblemArrays(NXDATALISTLib::_DNxDataListPtr& dl, short nColProblemAry);
	// (z.manning 2009-03-24 11:48) - PLID 33647
	void DeleteProblemArrayByRow(NXDATALISTLib::_DNxDataListPtr& dl, const long nRow, const short nColProblemAry);

	// (c.haag 2008-07-18 09:56) - PLID 30723 - Pulls problem action data from pRow and stores it in ea
	void StoreProblemActions(NXDATALISTLib::IRowSettingsPtr& pRow, short nColProblemAry, OUT EmrAction& ea);

	void StoreProblemActions(NXDATALIST2Lib::IRowSettingsPtr& pRow, short nColProblemAry, OUT EmrAction& ea);

public:
	enum EBuiltInSourceIdentifier {
		bisidNotBoundToData = -1,
	};
	
public:
	// If m_nSourceID is set to anything other than bisidNotBoundToData then m_aryActions is treated as OUT-only, 
	// anything you place in it at load time will be ignored unless m_nSourceID is bisidNotBoundToData.  On return, 
	// however, this variable is always populated with all (and only) the actions that were selected when the user 
	// clicked OK.  If the user clicked Cancel, the contents of m_aryActions is left untouched.
	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
	IN OUT MFCArray<EmrAction> m_arActions;
	// If m_nSourceID is bisidNotBoundToData the caller may fill in this m_strSourceObjectName string to give a 
	// more descriptive name of the object whose actions are being modified, because if m_nSourceID were bound to 
	// data, this dialog would automatically determine the name of the source object.  If this is unspecified, the 
	// system will default it to "unknown object".
	IN CString m_strSourceObjectName;

private:
	// (j.jones 2012-08-27 11:37) - PLID 52091 - we calculate this item name to use in the dialog frame,
	// but now we track it for later use in the dialog
	CString m_strSourceActionName;

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pCptCombo, m_pProductCombo, m_pChargeList, 
		  
		m_pItemCombo, m_pItemList, 
		m_pMintActionCombo, m_pMintActionList,
		m_pMedicationList,
		m_pProcedureCombo, m_pProcedureList,
		m_pMintItemActionCombo, m_pMintItemActionList,
		m_pLabCombo, m_pLabList, // (z.manning 2008-10-01 12:26) - PLID 31556 - Added lab lists
		m_pTodoList; // (c.haag 2008-05-30 12:41) - PLID 30221 - Added m_pTodoList

	// (b.savon 2015-12-30 09:25) - PLID 67758
	NXDATALIST2Lib::_DNxDataListEventsPtr m_nxdlMedicationSearch;

		// (z.manning 2011-06-28 16:31) - PLID 44347 - We now keep track of the datalist most recently right clicked on
		NXDATALISTLib::_DNxDataListPtr m_pdlLastRightClickedDatalist;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - we also keep track of the EmrAction type of the list you clicked on
		// (ie. right click on CPT codes returns eaoCpt)
		EmrActionObject m_LastRightClickedAction;

		CEMN* m_pCurrentEMN;

	
	// (j.jones 2012-08-24 14:10) - PLID 52091 - added ability to copy the anatomic location filter from another action
	void CopyAnatomicFilters(BOOL bOnlyForThisAction);

	// (b.savon 2015-12-30 11:16) - PLID 67758
	void AddMedicationFromSearch(LPDISPATCH lpRow);
	void AddMedicationToList(
		const long &nDrugListID, const CString& strMedName, const long &nFDBID, const BOOL &bFDBOutOfDate, 
		const long &nActionID = -1, const VARIANT_BOOL vbPopup = VARIANT_FALSE, const CProblemActionAry* paryActionProblems = new CProblemActionAry, const long &nSortOrder = -1
	);

	// (j.jones 2016-01-25 15:26) - PLID 67998 - resets the medication search provider based
	// on the value of the 'include free text' checkbox
	void ResetMedicationSearchProvider();

protected:
	// (s.dhole 07\08\2014 8:56) - PLID 62593

		struct DiagCodeList
		{
			long nActionID;
			long nICD9;
			long nICD10;
			CString strICD9Code;
			CString strICD10Code;
			CString strICD9Description;
			CString strICD10Description;
			NexGEMMatchType matchType;

			DiagCodeList()
			{
				nActionID = -1;
				nICD9 = -1;
				nICD10 = -1;
				strICD9Code = "";
				strICD10Code = "";
				strICD9Description = "";
				strICD10Description = "";
				matchType = nexgemtDone;
				
			}
			void operator =(const DiagCodeList &oDiagCodeList) {
				nActionID = oDiagCodeList.nActionID;
				nICD9 = oDiagCodeList.nICD9;
				nICD10 = oDiagCodeList.nICD10;
				strICD9Code = oDiagCodeList.strICD9Code;
				strICD10Code = oDiagCodeList.strICD10Code;
				strICD9Description = oDiagCodeList.strICD9Description;
				strICD10Description = oDiagCodeList.strICD10Description;
				matchType = oDiagCodeList.matchType;
			}
		};
	protected:
		// (s.dhole 07\08\2014 8:56) - PLID 62592 Uodate Datalist2 
		NXDATALIST2Lib::_DNxDataListPtr m_pDiagSearch, 	m_pDiagList; 

		// (s.dhole 07\09\2014 8:56) - PLID 62593  
		void UpdateDiagnosisListColumnSizes();
		//(s.dhole 3/6/2015 10:42 AM ) - PLID 64549
		BOOL  AddDiagCodeBySearchResults(class CDiagSearchResults &results);

		NXDATALIST2Lib::IRowSettingsPtr FindRowInDiagList(long nICD9ID, long nICD10ID);



		//(s.dhole 7/14/2014 2:18 PM ) - PLID 62723
		void UpdateDiagCodeArrows();

		void AddDiagCodeBySearchResultsData(class CDiagSearchResults &results, NexGEMMatchType matchType);
		//(s.dhole 7/18/2014 11:16 AM ) - PLID 62593
		void DeleteDiagProblemArrays(NXDATALIST2Lib::_DNxDataListPtr& dl, short nColProblemAry);
		void DeleteDiagProblemArrayByRow(NXDATALIST2Lib::IRowSettingsPtr pRow, const short nColProblemAry);
		void UpdateDiagActionProblemRow(NXDATALIST2Lib::IRowSettingsPtr& pRow, short nColProblemBitmap, short nColProblemAry);
		//(s.dhole 7/18/2014 11:16 AM ) - PLID 62593
		void UpdateDiagRowForAnatomyChange(NXDATALIST2Lib::IRowSettingsPtr pRow, const short nLabAnatomyCol);
		void HandelDatalistRowUpdate(NXDATALIST2Lib::IRowSettingsPtr pRow, DiagCodeList &digCodeList);
		//(s.dhole 7/17/2014 3:32 PM ) - PLID 62597
		void UpdateDiagCode(NXDATALIST2Lib::IRowSettingsPtr pRow);
		void HandleDiagCodeColumn(NXDATALIST2Lib::IRowSettingsPtr pRow);
		DiagCodeList  GetDigCodeList(NXDATALIST2Lib::IRowSettingsPtr pRow);
		void HandleDiagMatchColumn(NXDATALIST2Lib::IRowSettingsPtr pRow);
		//(s.dhole 7/18/2014 10:12 AM ) - PLID 62593
		void MoveUpOrDownDiagRow(BOOL bIsMoveUp = TRUE);

		BOOL IsDuplicateDiagnosisCode(short nColumn); //(s.dhole 7/30/2014 3:50 PM ) - PLID 63114
		//(s.dhole 7/17/2014 4:57 PM ) - PLID 62597
		struct DiagnosisCodeDrpoDown;
		scoped_ptr<DiagnosisCodeDrpoDown> m_pDiagCodeDDMultiMatch;
		
		void SizeDiagnosisListColumnsBySearchPreference(NXDATALIST2Lib::_DNxDataListPtr pDataList,
			short iColICD9CodeID, short iColICD10CodeID,
			short iColICD9Code, short iColICD10Code,long nMinICD9CodeWidth , long nMinICD10CodeWidth ,
			CString strICD9EmptyFieldText , CString strICD10EmptyFieldText ,
			short iColICD9Desc , short iColICD10Desc ,
			bool bShowDescriptionColumnsInCrosswalk  ,
			bool bAllowCodeWidthAuto ,
			bool bRenameDescriptionColumns ,
			enum DiagCodeSearchStyle eSearchStyle);

		//(s.dhole 7/18/2014 11:41 AM ) - PLID 62597
		void LoadMissingICD9DiagCodeInfo(NXDATALIST2Lib::IRowSettingsPtr pRow);
		//(s.dhole 7/29/2014 8:24 AM ) - PLID 63083
		void CopyDiagAnatomicFilters(BOOL bOnlyForThisAction);
		NXDATALIST2Lib::_DNxDataListPtr m_pdlLastRightClickedDatalistDiag;
		void OnEditDiagActionAnatomicLocations();

		CString  GetDiagCodeDescription(NXDATALIST2Lib::IRowSettingsPtr pRow);
		BOOL IsIncludeThisRow(BOOL bOnlyForThisAction, NXDATALIST2Lib::IRowSettingsPtr pCurrrentRow, NXDATALIST2Lib::IRowSettingsPtr pRow);
		CSqlFragment  GetDiagCurrentList(BOOL bOnlyForThisAction, NXDATALIST2Lib::IRowSettingsPtr pRow);
		CSqlFragment GetDiagFilterSQl(BOOL bOnlyForThisAction,  NXDATALIST2Lib::IRowSettingsPtr pRow);

		BOOL IsDiagCodeMissing();

protected:
	// Generated message map functions
	//{{AFX_MSG(CEmrActionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenActionCptCombo(long nRow);
	afx_msg void OnSelChosenActionLabCombo(long nRow);
	afx_msg void OnRButtonUpActionCptList(long nRow, short nCol, long x, long y, long nFlags);
	
	afx_msg void OnRButtonUpActionLabList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveCpt();
	afx_msg void OnRemoveDiag();
	afx_msg void OnRemoveLab();
	afx_msg void OnRemoveItem();
	afx_msg void OnRemoveMint();
	afx_msg void OnRemoveMintItems();
	afx_msg void OnSelChosenActionItemCombo(long nRow);
	afx_msg void OnRButtonUpActionItemList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChosenActionEMNTemplateCombo(long nRow);
	afx_msg void OnRButtonUpActionEMNTemplateList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChosenActionEMNTemplateItemCombo(long nRow);
	afx_msg void OnRButtonUpActionEMNTemplateItemList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRadioServiceCode();
	afx_msg void OnRadioProduct();
	afx_msg void OnSelChosenActionProductCombo(long nRow);
	afx_msg void OnRButtonDownActionMedicationList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveMedication();
	afx_msg void OnSelChosenActionProcedureCombo(long nRow);
	afx_msg void OnRButtonDownActionProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveProcedure();
	afx_msg void OnDblClickCellActionItemList(long nRowIndex, short nColIndex);
	afx_msg void OnEditItem();
	afx_msg void OnNewItem();
	afx_msg void OnRequeryFinishedActionCptList(short nFlags);
	
	afx_msg void OnRequeryFinishedActionLabList(short nFlags);
	afx_msg void OnRequeryFinishedActionMedicationList(short nFlags);
	afx_msg void OnRequeryFinishedActionItemList(short nFlags);
	afx_msg void OnRequeryFinishedActionProcedureList(short nFlags);
	afx_msg void OnRequeryFinishedActionEmnTemplateItemList(short nFlags);
	afx_msg void OnRequeryFinishedActionEmnTemplateList(short nFlags);
	afx_msg void OnBtnEmrMintItemUp();
	afx_msg void OnBtnEmrMintItemDown();
	afx_msg void OnSelChangedActionEmnTemplateItemList(long nNewSel);
	afx_msg void OnEditingFinishedActionItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnChargeEditBtn();
	afx_msg void OnEditingFinishedActionEmnTemplateItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingActionCptList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedActionItemList(long nNewSel);
	afx_msg void OnBtnEmrItemUp();
	afx_msg void OnBtnEmrItemDown();
	afx_msg void OnSelChangedChargeList(long nNewSel);
	afx_msg void OnBtnChargeUp();
	afx_msg void OnBtnChargeDown();
	
	afx_msg void OnSelChangedLabList(long nNewSel);
	afx_msg void OnBtnDiagUp();
	afx_msg void OnBtnDiagDown();
	afx_msg void OnBtnLabUp();
	afx_msg void OnBtnLabDown();
	afx_msg void OnSelChangedMedicationList(long nNewSel);
	afx_msg void OnBtnMedicationUp();
	afx_msg void OnBtnMedicationDown();
	afx_msg void OnSelChangedEmnList(long nNewSel);
	afx_msg void OnBtnEmnUp();
	afx_msg void OnBtnEmnDown();
	afx_msg void OnSelChangedProcedureList(long nNewSel);
	afx_msg void OnBtnProcedureUp();
	afx_msg void OnBtnProcedureDown();
	afx_msg void OnBtnNewTodoTask();
	afx_msg void OnSelChangedActionTodoList(long nNewSel);
	afx_msg void OnRButtonUpActionTodoList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnNewTodo();
	afx_msg void OnEditTodo();
	afx_msg void OnRemoveTodo();
	afx_msg void OnDblClickCellActionTodoList(long nRowIndex, short nColIndex);
	afx_msg void OnBtnTodoUp();
	afx_msg void OnBtnTodoDown();
	afx_msg void OnLButtonDownActionCptList(long nRow, short nCol, long x, long y, long nFlags);
	
	afx_msg void OnLButtonDownActionLabList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonDownActionMedicationList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonDownActionEmnTemplateItemList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonDownActionItemList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonDownActionEmnTemplateList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedActionTodoList(short nFlags);
	afx_msg void OnConfigureCptProblems();
	afx_msg void OnConfigureDiagProblems();
	afx_msg void OnConfigureMedProblems();
	afx_msg void OnConfigureItemProblems();
	afx_msg void OnConfigureMintProblems();
	afx_msg void OnConfigureMintItemProblems();
	afx_msg void OnEditActionAnatomicLocations();
	// (j.jones 2012-08-24 14:10) - PLID 52091 - added ability to copy the anatomic location filter from another action
	afx_msg void OnCopyThisActionAnatomicLocations();
	afx_msg void OnCopyAnyActionAnatomicLocations();
	//(s.dhole 8/1/2014 10:16 AM ) - PLID 62597 Serch fro matching ICD-10 code if it is missing
	afx_msg void OnSearchMMatchingICD10();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SelChosenActionDiagSearch(LPDISPATCH lpRow);
	void SelChangedActionDiagList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void LButtonDownActionDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonUpActionDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	void EditingFinishedActionDiagList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void SelChosenNxdlAddMedAction(LPDISPATCH lpRow);
	// (j.jones 2016-01-25 15:18) - PLID 67998 - added checkbox to include free text meds in the medication search
	afx_msg void OnCheckIncludeFreeTextMeds();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRACTIONDLG_H__32BDA6A3_1921_4A14_9EF0_C6D470895DFF__INCLUDED_)
