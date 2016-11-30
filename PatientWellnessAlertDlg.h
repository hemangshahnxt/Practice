#pragma once
#include "PatientsRc.h"

#define PREQUALIFIED_ALERT RGB(222, 225, 231)
#define PATIENT_ALERT  RGB(254, 235, 178)
#define COMPLETED_ALERT RGB(189,253,190)

// CPatientWellnessAlertDlg dialog
// (j.gruber 2009-05-26 15:27) - PLID 34349 - created for
class CPatientWellnessAlertDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientWellnessAlertDlg)

public:
	// (j.gruber 2009-06-03 12:04) - PLID 34457 - make it read only if it is completed
	CPatientWellnessAlertDlg(long nWellnessID, BOOL bIsTemplate, long nPatientID, BOOL bReadOnly, CWnd* pParent);   // standard constructor
	virtual ~CPatientWellnessAlertDlg();

// Dialog Data
	enum { IDD = IDD_PATIENT_WELLNESS_ALERT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL OnInitDialog();

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_chkCompleted;
	CNxIconButton m_btnGuidePreview;
	CNxIconButton m_btnRefPreview;
	CNxEdit m_edtNotes;
	CNxIconButton m_btnClose;

	NXDATALIST2Lib::_DNxDataListPtr m_pCompletionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pEMRItemList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCriteriaList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailCriteriaList;
	NXTIMELib::_DNxTimePtr m_pStartDate;

	RICHTEXTEDITORLib::_DRichTextEditorPtr m_reGuidelines;
	RICHTEXTEDITORLib::_DRichTextEditorPtr m_reReferenceMaterials;

	//declare the backgrounds
	CNxColor m_bkg1;
	CNxColor m_bkg2;
	CNxColor m_bkg3;
	CNxColor m_bkg4;
	CNxColor m_bkg5;

	long m_nWellnessID;
	BOOL m_bIsTemplate;
	long m_nPatientID;
	BOOL m_bHasChanged;
	long m_nSpecificToPatientID;
	CString m_strTemplateName;
	BOOL m_bComplete;
	COleDateTime m_dtComplete;
	// (j.gruber 2009-06-03 12:58) - PLID 34457 - added read only option
	BOOL m_bReadOnly;

	void SetColor();

	void Load();

	BOOL Save();
	BOOL GenerateCriteriaSaveString(CString &strSqlBatch);
	BOOL SaveTemplate();
	BOOL GenerateTemplateCompletionSaveString(CString &strSqlBatch);
	void CheckCompleted();
	BOOL ValidateCurrentTemplate();

	CDWordArray m_aryDeletedCompletionItems;
	CDWordArray m_aryCompletionRecordIDs;
	//TES 6/8/2009 - PLID 34505 - Need to also track the types of completion items that we're completing
	CDWordArray m_aryCompletionRecordTypes;
	CDWordArray m_aryInitialCriteria;
	CDWordArray m_aryDeletedCriteria;
	CDWordArray m_aryChangedCriteria;

	void AddToChangedCriteriaList(LPDISPATCH lpRow);
	void RemoveFromChangedCriteriaList(long nCriteriaID);
	void GetChangedCriteriaOnly(CDWordArray *pCriteriaArray);


	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenPatWellAlertEmrItemList(LPDISPATCH lpRow);
	void RButtonDownPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishedPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingStartingPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void RButtonDownPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RequeryFinishedPatWellAlertCriteria(short nFlags);
	void LeftClickPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);	
	void TextChangedPatWellAlertGuidelines();
	void TextChangedPatWellAlertReferences();
	void EditingStartingPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void SelChosenPatWellAvailableCriteriaList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedOk();
	void EditingFinishedPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void RequeryFinishedPatWellAlertCompleteItems(short nFlags);
	afx_msg void OnBnClickedPatWellnessPreviewGuide();
	afx_msg void OnBnClickedPatWellnessPreviewRef();
	void EditingFinishingPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnBnClickedClose();
	void EditingFinishingPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);	
};


