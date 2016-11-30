#pragma once

#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomFieldsDlg.h"

//TES 11/17/2009 - PLID 36190 - Created.  The vast majority of the code in this file was moved here from LabEntryDlg.
// (c.haag 2010-12-14 9:11) - PLID 41806 - Removed Completed fields
enum LabType;
struct SharedLabInformation;
class CLabEntryDlg;
class CLabRequisitionsTabDlg;
// CLabRequisitionDlg dialog

class CLabRequisitionDlg : public CNxDialog
{
	friend class CEMRProblemListDlg; // (z.manning 2009-07-17 19:07) - PLID 34345

	DECLARE_DYNAMIC(CLabRequisitionDlg)

	CLabCustomFieldsDlg *m_pFieldsDlg;

public:
	CLabRequisitionDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabRequisitionDlg();

	//TES 11/17/2009 - PLID 36190 - Data passed in from LabEntryDlg
	void SetPatientID(long nPatientID);
	void SetLabID(long nLabID);
	void SetLabProcedureID(long nLabProcedureID);
	void SetLabProcedureType(LabType ltType);	// (r.galicki 2008-10-17 11:46) - PLID 31552
	// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
	void SetDefaultLocationID(long nLocationID);
	// (r.gonet 07/22/2013) - PLID 57683 - Sets the providers from the source EMN.
	void SetEMNProviders(CDWordArray &dwProviderIDs);

	// (z.manning 2008-10-06 14:43) - PLID 21094
	void SetSourceActionID(const long nSourceActionID);
	void SetSourceDetailID(const long nSourceDetailID);
	// (z.manning 2009-02-27 10:08) - PLID 33141 - SourceDataGroupID
	void SetSourceDataGroupID(const long nSourceDataGroupID);

	// (z.manning 2009-05-08 09:39) - PLID 28554 - Sets the order set ID if this lab is part of an order set.
	void SetOrderSetID(const long nOrderSetID);

	// (z.manning 2009-05-19 09:17) - PLID 28554 - Set the default to be ordered text
	void SetDefaultToBeOrdered(const CString &strToBeOrdered);

	void SetCurrentDate(_variant_t varCurDate);

	//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
	void SetInitialAnatomicLocation(long nAnatomicLocationID, long nAnatomicQualifierID, AnatomySide asSide);

	bool HasDataChanged(); // (a.walling 2006-07-14 10:25) - PLID 21073 True if any data has been saved

	void LoadNew();
	void LoadExisting();

	//TES 11/17/2009 - PLID 36190 - Call to do any processing that needs to wait until data has loaded.
	void PostLoad();

	BOOL IsNew();

	//TES 11/25/2009 - PLID 36193 - Just calls CLabEntryDlg::Save()
	BOOL Save();
	//TES 11/25/2009 - PLID 36193 - Need to have shared information (e.g., form number, patient name) provided.
	//TES 12/1/2009 - PLID 36452 - Added an output parameter for the new ID
	//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
	void SaveNew(long &nAuditTransactionID, IN const SharedLabInformation &sli, OUT long &nNewLabID, IN OUT CDWordArray &arNewCDSInterventions);
	void SaveExisting(long &nAuditTransID, IN OUT CDWordArray &arNewCDSInterventions);

	BOOL IsLabValid();

	CString GenerateCompletionAuditOld();

	COleDateTime GetBiopsyDate();

	// (j.jones 2010-05-06 09:37) - PLID 38520 - added GetInputDate
	COleDateTime GetInputDate();

	//TES 11/25/2009 - PLID 36193 - Access the current Specimen
	CString GetSpecimen();

	//TES 11/17/2009 - PLID 36190 - Called by CLabEntryDlg when the user chooses to cancel.
	void OnLabEntryCancelled();

	//TES 11/25/2009 - PLID 36191 - All EMR Problem code moved here from CLabEntryDlg

	// (z.manning 2009-05-29 09:42) - PLID 34345
	void GetAllProblemLinks(OUT CArray<CEmrProblemLink*,CEmrProblemLink*> &arypProblemLinks, CEmrProblem *pFilterProblem = NULL, BOOL bIncludeDeleted = FALSE);

	// (z.manning 2009-05-29 14:09) - PLID 29911
	BOOL HasProblems();

	// (j.jones 2009-06-08 09:12) - PLID 34487 - added ability to add a problem link from the problem list
	void AddProblemLink(CEmrProblemLink* pNewLink);

	void SetLabEntryDlg(CLabEntryDlg *pDlg);

	// (z.manning 2009-05-26 15:41) - PLID 34340 - Generates the sql to save all relevant problem and problem links
	//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
	void SaveProblems(long &nAuditTransactionID, IN OUT CDWordArray &arNewCDSInterventions);

	//TES 12/5/2009 - PLID 36193 - We never needed to know if this was launched from EMR, I just didn't understand how the integration worked.
	//bool m_bLaunchedFromEmr;

	//TES 11/30/2009 - PLID 36193 - Access this requisition's LabID (may be -1 for new labs)
	long GetLabID();

	//TES 2/1/2010 - PLID 37143 - Get the default request form associated with the current lab's Receving Lab.
	long GetRequestForm();

	//TES 7/27/2012 - PLID 51849 - Same for the Results Form
	long GetResultsForm();

	void Close();

	// (z.manning 2010-05-05 10:14) - PLID 37190
	void SetDefaultInitialDiagnosis(CString strDefaultInitialDiagnosis);
	CString GetLastSavedInitialDiagnosis();

	//TES 9/29/2010 - PLID 40644
	void SetDefaultComments(const CString &strDefaultComments);
	CString GetLastSavedComments();
	void SetDefaultLabLocationID(long nLabLocationID);
	long GetLastSavedLabLocationID();
	long GetLastSavedLocationID();
	void SetDefaultProviders(const CDWordArray &dwDefaultProviderIDs, const CString &strDefaultProviderNames);
	void GetLastSavedProviderIDs(OUT CDWordArray &dwProviderIDs);
	CString GetLastSavedProviderNames();

	// (j.jones 2016-02-22 10:01) - PLID 68348 - added ability to get the current provider IDs
	void GetCurrentProviderIDs(OUT CDWordArray &dwProviderIDs);
	// (j.jones 2016-02-22 10:01) - PLID 68348 - added ability to get the current location ID
	long GetCurrentLocationID();

	// (z.manning 2010-05-13 13:57) - PLID 37405
	void AddPendingTodoID(const long nTaskID);
	void ClearPendingTodos();
	void DeletePendingTodos();
	int GetPendingTodoCount();

	// (b.spivey, August 27, 2013) - PLID 46295 - Get the anatomical location as a string.
	CString GetAnatomicLocationString(); 

// Dialog Data
	enum { IDD = IDD_LAB_REQUISITION_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void SortDWordArray(CDWordArray &dw);
	CString GetIDStringFromArray(CDWordArray *dw, CString delim = ", ");
	

	long m_nLabID;
	long m_nPatientID;
	long m_nLabProcedureID;
	// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
	long m_nDefaultLocationID;
	// (r.gonet 07/22/2013) - PLID 45187 - The primary providers from the source EMN. 
	CDWordArray m_dwEMNProviderIDs;
	LabType m_ltType;
	CString m_strDefaultToBeOrdered;

	// (j.jones 2010-05-06 09:41) - PLID 38520 - added m_dtInputDate
	COleDateTime m_dtInputDate;
	
	_variant_t m_varCurDate;
	CDWordArray m_dwProviders;
	// (d.lange 2010-12-30 17:52) - PLID 29065 - Added Biopsy Type
	long m_nMedAssistantID, m_nInsuredPartyID, m_nCurAnatomyID, m_nCurAnatomyQualifierID, m_nSavedAnatomySide, m_nCurBiopsyTypeID, m_nSavedBiopsyTypeID;
	BOOL m_bChangedProvider;
	//TES 9/29/2010 - PLID 40644 - We need to keep track of the names as well as the IDs.
	CString m_strProviderNames;

	//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
	long m_nInitialAnatomicLocationID, m_nInitialAnatomicQualifierID;
	AnatomySide m_asInitialAnatomicSide;

	// (z.manning 2010-05-05 10:14) - PLID 37190
	CString m_strDefaultInitialDiagnosis;

	//TES 9/29/2010 - PLID 40644
	CString m_strDefaultComments;
	long m_nDefaultLabLocationID;
	long m_nLabLocationID;
	CDWordArray m_dwDefaultProviderIDs;
	CString m_strDefaultProviderNames;

	//TES 11/25/2009 - PLID 36193 - Need to have access to our parents
	CLabEntryDlg *m_pLabEntryDlg;
	CLabRequisitionsTabDlg *m_pLabRequisitionsTabDlg;

	// (z.manning 2008-10-16 10:30) - PLID 21082 - Signature info
	CString m_strSignatureFileName;
	_variant_t m_varSignatureInkData;
	// (j.jones 2010-04-12 17:33) - PLID 38166 - added a date/timestamp
	_variant_t m_varSignatureTextData;

	// (j.gruber 2008-10-02 10:56) - PLID 31332 - take out any variables not used anymore
	long m_nSavedInputBy;
	COleDateTime m_dtSavedInputDate;

	// (c.haag 2010-11-23 10:50) - PLID 41590 - Signature date
	long m_nSignedByUserID;
	long m_nSavedSignedByUserID;
	COleDateTime m_dtSigned;
	COleDateTime m_dtSavedSigned;

	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	CString m_strSavedSpecimen, m_strSavedClinicalData, m_strSavedInstructions, m_strSavedToBeOrdered, m_strProvStartString,
		m_strSavedInitialDiagnosis, m_strSavedProviderNames, m_strSavedLoincCode, m_strSavedLoincDescription;
	COleDateTime m_dtSavedBiopsyDate;
	BOOL m_bSavedMelanomaHistory, m_bSavedPreviousBiopsy, m_bSavedCCPatient, m_bSavedCCRefPhys, m_bSavedCCPCP;
	long m_nSavedLocationID, m_nSavedLabLocationID, m_nSavedAnatomyID, m_nSavedAnatomyQualifierID;
	CDWordArray m_dwSavedProviderIDs;

	// (r.gonet 09/21/2011) - PLID 45124 - The lab custom fields template instance that 
	CCFTemplateInstance *m_pcfTemplateInstance;

	// (z.manning 2010-05-13 13:56) - PLID 37405 - Array to keep track on to-dos created for this lab
	CArray<long,long> m_arynPendingTodoIDs;
	
	//TES 7/12/2011 - PLID 44107 - Added LOINC fields
	CNxLabel m_nxlProviderLabel, m_nxlLoincLabel;
	CNxEdit m_nxeditClinicalData, m_nxeditInstructions, m_nxeditToBeOrderedText, m_nxeInitialDiagnosis, 
		m_nxeditSpecimen, m_nxeditLoincCode, m_nxeditLoincDescription;
	CString m_strEditInstructions;
	// (d.lange 2010-12-30 16:01) - PLID 29065 - Added button to edit Biopsy Types
	CNxIconButton m_EditAnatomyBtn, m_btnEditToBeOrdered, m_nxbAddInitialDiagnosis, m_nxbEditInitialDiagnosis,
		m_nxbEditAnatomyQualifiers, m_btnProblemList, m_btnEditBiopsyType; // (z.manning 2009-05-26 10:20) - PLID 34340;
	//TES 12/8/2009 - PLID 36470 - Restored the Left/Right checkboxes.
	//TES 7/11/2011 - PLID 36445 - Added CC Fields
	NxButton m_nxbtnMelanomaHistory, m_nxbtnPreviousBiopsy, m_nxbtnSideLeft, m_nxbtnSideRight, m_nxbtnCCPatient, m_nxbtnCCRefPhys, m_nxbtnCCPCP;
	// (d.lange 2010-12-30 16:01) - PLID 29065 - Added label for Biopsy Types
	CNxStatic m_nxstaticToBeOrderedLabel, m_nxstaticBiopsyTypeLabel;
	CDateTimePicker	m_dtpBiopsy;

	// (c.haag 2010-11-30 10:55) - PLID 38633 - We only have signature fields now, and those will only be
	// used by users who elect to sign requistions in addition to results
	CNxIconButton m_signBtn;
	CNxEdit m_nxeditLabSignedBy, m_nxeditLabSignedDate;
	CNxStatic m_nxstaticSignedBy, m_nxstaticSignedDate;

	// (j.jones 2013-10-22 12:53) - PLID 58979 - added infobutton features
	CNxIconButton m_btnPatientEducation;
	CNxLabel m_nxlabelPatientEducation;

	// (d.lange 2010-12-30 16:01) - PLID 29065 - Added dropdown for Biopsy Types
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderCombo, m_pLocationCombo, m_pReceivingLabCombo, m_pAnatomyCombo, m_pMedAssistantCombo,
		m_pInsuranceCombo, m_pToBeOrderedCombo, m_pAnatomyQualifiersCombo, m_pBiopsyTypeCombo;

	//TES 7/16/2010 - PLID 39575 - Added the ability to include the biopsy time, as well as the date.
	NXTIMELib::_DNxTimePtr	m_nxtBiopsyTime;

	// (r.gonet 09/21/2011) - PLID 45124 - Gets you into the dialog that lets you edit the custom fields associated with this lab.
	CNxIconButton m_nxbtnAdditionalFields;

	CArray<CEmrProblemLink*,CEmrProblemLink*> m_arypProblemLinks;
	// (z.manning 2009-05-26 10:57) - PLID 34340 - Load any problems and links that are associated with this lab.
	void LoadExistingProblems();
	// (z.manning 2009-05-26 12:14) - PLID 34340 - Updates the icon of the problem button depending on whether
	// or not there are problems associated with this lab.
	void UpdateProblemButton();
	// (z.manning 2009-05-27 10:12) - PLID 34297 - Added patient ID
	// (b.spivey, October 23, 2013) - PLID 58677 - Added CodeID
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	// (s.tullis 2015-02-23 15:44) - PLID 64723 
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added bDoNotShowOnProblemPrompt. True to have the problem show in the prompt when we switch patients
	// or go to the EMR tab. False to not show in the prompt.
	CEmrProblem* AllocateEmrProblem(long nID, long nPatientID, CString strDescription, COleDateTime dtEnteredDate, COleDateTime dtModifiedDate, COleDateTime dtOnsetDate,
		long nStatusID, long nDiagCodeID_ICD9, long nDiagCodeID_ICD10, long nChronicityID, BOOL bIsModified, long nCodeID, BOOL bDoNotShowOnCCDA, BOOL bDoNotShowOnProblemPrompt);
	CEmrProblem* AllocateEmrProblem(ADODB::FieldsPtr& f);
	void GetAllProblems(OUT CArray<CEmrProblem*,CEmrProblem*> &arypProblems, BOOL bIncludeDeleted);
	CEmrProblem* GetProblemByID(const long nProblemID);
	// (z.manning 2009-05-28 15:10) - PLID 34345
	void UpdateProblemLinkLabText();

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnEditAnatomy();
	DECLARE_EVENTSINK_MAP()
	void OnTrySetSelFinishedLabLocation(long nRowEnum, long nFlags);
	void OnTrySetSelFinishedReceivingLab(long nRowEnum, long nFlags);
	void OnTrySetSelFinishedAnatomicLocation(long nRowEnum, long nFlags);
	void OnTrySetSelFinishedLabMedassistant(long nRowEnum, long nFlags);
	void OnTrySetSelFinishedLabInsurance(long nRowEnum, long nFlags);
	void OnRequeryFinishedLabProvider(short nFlags);
	void OnSelChosenLabProvider(LPDISPATCH lpRow);
	void OnMultiSelectProvs();
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	void OnSelChosenLabsToBeOrdered(LPDISPATCH lpRow);
	afx_msg void OnEditToBeOrdered();
	void OnRequeryFinishedAnatomicLocation(short nFlags);
	afx_msg void OnEditAnatomicQualifiers();
	void OnRequeryFinishedAnatomicQualifier(short nFlags);
	void OnTrySetSelFinishedAnatomicQualifier(long nRowEnum, long nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void OnSelChangingLabProvider(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnAddInitialDiagnosis();
	afx_msg void OnEditInitialDiagnosis();
	afx_msg void OnBnClickedLabEntryProblemList();
	afx_msg void OnKillFocusSpecimen();
	afx_msg void OnLeftSide();
	afx_msg void OnRightSide();
	void OnSelChosenReceivingLab(LPDISPATCH lpRow);
	afx_msg void OnSize(UINT nType, int cx, int cy); // (z.manning 2010-04-29 16:04) - PLID 38420
	// (j.jones 2010-05-06 11:15) - PLID 38524 - added OnDtnDatetimechangeBiopsyDate
	afx_msg void OnDtnDatetimechangeBiopsyDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSignRequisition();
	afx_msg void OnBnClickedEditBiopsyType();
	void TrySetSelFinishedBiopsyType(long nRowEnum, long nFlags);
	afx_msg void OnBnClickedLabCustomFieldsButton();	
	// (j.jones 2013-10-22 12:52) - PLID 58979 - added infobutton
	afx_msg void OnBtnPtEducation();
};
