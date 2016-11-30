#pragma once
#include "PracticeRc.h"
#include "ReconcileMergeMedicationsDlg.h"
#include "NxAPI.h"
// CReconciliationDlg dialog
// (s.dhole 2013-09-18 14:16) - PLID 56625 create
class CReconciliationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReconciliationDlg)

public:
	CReconciliationDlg(long nPatientID, long nMailSentID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CReconciliationDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_RECONCILIATION };


	long m_nReconciliationType;
	
enum ReconciliationType
{
	erNone = 0L,
	erAllergy  ,
	erProblem,// (s.dhole 2013-10-30 15:12) - PLID 56933
	erMedication,// (s.dhole 2013-10-30 15:10) - PLID 59235 
};
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void CReconciliationDlg::SetColumnHeaders( );
	void UpdateColumnCaption(NXDATALIST2Lib::_DNxDataListPtr ListPtr, int  nColumn, CString strCaption );	
	long m_nPatientID;
	CNxColor m_nxcBackReconcilation;
	CNxIconButton m_btnOkReconcilation;
	CNxIconButton m_btnCancelReconcilation;
	OLE_COLOR m_clrBackReconcilation;
	NXDATALIST2Lib::_DNxDataListPtr	m_dlReconciliationNew;
	NXDATALIST2Lib::_DNxDataListPtr	m_dlReconciliationExist;
	CNxEdit m_nxeNewlabel,m_nxeCurrentlabel; // (s.dhole 2013-11-01 12:46) - PLID 59278
	DECLARE_MESSAGE_MAP()
public:
	struct ReconciledItem
	{
		enum { eKeepCurItem  , eAddItem, eDeleteItem, eMergeCurItem , eExcludeCurItem } Action; // Indicates what we're doing to the record
		long nInternalID;
		long nCurrentID; 
		CString strName;
		CString strDescription;
		CString strCode;
		BOOL bIsActive;
		COleDateTime dtLastDate;
	};
	typedef CArray<ReconciledItem,ReconciledItem&> CReconciledItemsArray;

	// (b.spivey, October 30, 2015) PLID 67423
	static void ReconcileAll(long nPatientID, long nMailSentID, CWnd* pParent = NULL);

	// (b.spivey, October 30, 2015) PLID 67514
	CString m_strPreviewDlgOkButtonTextOverride;

protected:
	// This class represents a single current medication used as an input in this dialog
	struct ReconcilationList
	{
		long nInternelID; 
		long nCodeID; 
		CString strName; 
		CString strDesc;
		BOOL bActive;
		COleDateTime dtLastDate;
		CString strCompTxt;
	};
	typedef CArray<ReconcilationList,ReconcilationList&> CListArray;
	
protected: 
	CListArray m_aCurrentListArray;
	CListArray m_aNewListArray;
	BOOL LoadData();
	// (s.dhole 2013-10-30 15:10) - PLID 59235 
	void LoadCurrentMedicationData();
	// (s.dhole 2013-10-30 15:10) - PLID 59235 
	void LoadNewMedicationData();
	// (s.dhole 2013-10-30 15:12) - PLID 56625
	void LoadNewAllergyData();
	void LoadCurrentAllergyData();
	// (s.dhole 2013-10-30 15:10) - PLID 56933
	void LoadNewProblemData();
	// (s.dhole 2013-10-30 15:10) - PLID 56626
	void LoadCurrentProblemData();
	void ValidateAndSaveData();
	ReconciledItem LoadReconcileNewItem(NXDATALIST2Lib::IRowSettingsPtr pRow );
	// (s.dhole 2013-10-30 13:46) - PLID 59232
	CReconcileMergeMedicationsDlg::ReconcileValidationItem CReconciliationDlg::LoadReconcileValidationNewItem(NXDATALIST2Lib::IRowSettingsPtr pRow );
	// (s.dhole 2013-10-30 13:46) - PLID 59232
	ReconciledItem CReconciliationDlg::LoadReconcileCurrentItem(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bKeepRecord);
	// (s.dhole 2013-10-30 13:46) - PLID 59232
	CReconcileMergeMedicationsDlg::ReconcileValidationItem LoadReconcileValidationCurrentItem(NXDATALIST2Lib::IRowSettingsPtr pRow , BOOL bKeepRecord);
	// (s.dhole 2013-10-30 13:46) - PLID 59232
	// (s.dhole 2014-01-13 17:40) - PLID 59643 
	BOOL GetDataArray(CReconciliationDlg::CReconciledItemsArray &aReconciledItemsArray,CReconcileMergeMedicationsDlg::CReconcileValidationItemArray &aReconcileValidationItemArray,
											 OUT BOOL &bNeedCreatePerms,OUT BOOL &bNeedDeletePerms,OUT BOOL &bNeedEditPerms);
	// (s.dhole 2013-10-30 13:46) - PLID 59232
	void ValidateAndSave(EBuiltInObjectIDs eBuiltInID);

	
	// (s.dhole 2013-10-30 13:46) - PLID 59232
	void SaveAllergyData(CReconciledItemsArray &aReconciledItemsArray);
	void SaveMedicationData(CReconciledItemsArray &aReconciledItemsArray);
	// (s.dhole 2013-10-30 13:46) - PLID 56933
	void ImportNewProblemCodes(CReconciledItemsArray &aReconciledItemsArray);

	void SaveProblemData(CReconciledItemsArray &aReconciledItemsArray);
	// (s.dhole 2013-10-30 11:27) - PLID 56935 Add Problem
	void AddProblemListData(ReconciledItem &aReconciledItem,CString strPatientName,long nAuditTransID, CString &strSqlBatch,CNxParamSqlArray &aryParams);
	// (s.dhole 2013-10-30 11:44) - PLID 59229
	void UpdateProblemListData(ReconciledItem &aReconciledItem,CString strPatientName,long nAuditTransID, CString &strSqlBatch,CNxParamSqlArray &aryParams);
	// (s.dhole 2013-10-30 11:50) - PLID 59230 Delete Problems
	void DeleteProblemListData(ReconciledItem &aReconciledItem,CString strPatientName,long nAuditTransID, CString &strSqlBatch,CNxParamSqlArray &aryParams);
	afx_msg void OnBnClickedBtnReconciliationPreviewMerge();
	afx_msg void OnBnClickedBtnReconciliationCancel();
	void SetExistingItemFlag(NXDATALIST2Lib::IRowSettingsPtr pRow,BOOL bShoCheckBox);
	// (s.dhole 2013-10-30 11:44) - PLID 59229
	BOOL IsDateTimeValid(const COleDateTime& dt) const;
	long m_nMailSentID;
	NexTech_Accessor::_ParsedCCDAResultsPtr GetParsedCCDAResultsPtr (ReconciliationType nType); 
	COleDateTime  GetParsedDate(_variant_t varStartDate,_variant_t varEndDate,_variant_t varModifiedDate) ;
	CString GetParsedStatus(_variant_t varEndDate,const CString strStatus); 
	// (s.dhole 2014-02-04 10:15) - PLID 60201
	BOOL CanAddOrEditProblems(CReconciliationDlg::CReconciledItemsArray &aReconciledItemsArray);

public:
	CString m_strResultText;
	inline void SetBackColor(OLE_COLOR clr) { m_clrBackReconcilation = clr; }
	DECLARE_EVENTSINK_MAP()
	void EditingFinishingNewReconciliationList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	

};
