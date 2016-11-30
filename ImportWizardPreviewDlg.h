#if !defined(AFX_IMPORTWIZARDPREVIEWDLG_H__C4A29484_1E70_430D_9D55_8621E5999382__INCLUDED_)
#define AFX_IMPORTWIZARDPREVIEWDLG_H__C4A29484_1E70_430D_9D55_8621E5999382__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportWizardPreviewDlg.h : header file
//

#include "ImportWizardDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CImportWizardPreviewDlg dialog

// (b.savon 2015-04-28 15:19) - PLID 65485 - Derive from our CNxPropertyPage
class CImportWizardPreviewDlg : public CNxPropertyPage
{
	DECLARE_DYNCREATE(CImportWizardPreviewDlg)

// Construction
public:
	CImportWizardPreviewDlg();
	~CImportWizardPreviewDlg();

	// (b.cardillo 2015-07-15 22:36) - PLID 66545 - The preview page now removes most rows from the 
	// source. 
	// Tell the preview page what datalist to copy rows from, and since it now removes most, tell 
	// it how many to leave intact (counting from the first row in the list).
	void SetCopyFromDatalist(NXDATALIST2Lib::_DNxDataListPtr pCopyFrom, long nLeaveCountInFromList);
	void SetMappingFromFields(CMap<CString, LPCTSTR, long, long> *pMap);

	// (r.goldschmidt 2016-01-28 16:53) - PLID 67976 - Use to carry over settings from fields panel to preview panel
	void SetApptConversionCheckbox(BOOL bChecked);
	void SetApptNoteCheckbox(BOOL bChecked);
	void SetApptNoteCheckboxText(CString strText);
	void AddExplanatoryText(CString strText);
	// (r.goldschmidt 2016-01-28 11:56) - PLID 67976 - get name of type used for conversion button option
	CString GetApptTypeNameConversion();

// Dialog Data
	//{{AFX_DATA(CImportWizardPreviewDlg)
	enum { IDD = IDD_IMPORT_WIZARD_PREVIEW };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImportWizardPreviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	BOOL   m_bNeedInit;
	CSize  m_ClientSize;
	NXDATALIST2Lib::_DNxDataListPtr m_pPreviewList, m_pCopyFromList;
	long m_nLeaveCountInFromList;
	NxButton m_chkPerformBackup; // (b.savon 2015-03-19 11:16) - PLID 65248
	NxButton m_chkPerformCapsFix; // (b.savon 2015-03-24 12:26) - PLID 65250
	CString m_strPersonFields, m_strPersonValues;
	CString m_strPatientFields, m_strPatientValues;
	CString m_strProviderFields, m_strProviderValues;
	CString m_strRefPhysFields, m_strRefPhysValues;
	CString m_strUserFields, m_strUserValues;
	CString m_strSupplierFields, m_strSupplierValues;
	CString m_strOtherContactFields, m_strOtherContactValues;
	CString m_strAuditPersonName, m_strAuditNewValue;
	// (j.jones 2010-04-02 17:54) - PLID 16717 - supported service codes
	CString m_strServiceFields, m_strServiceValues;
	CString m_strCPTCodeFields, m_strCPTCodeValues;
	// (r.farnworth 2015-03-16 15:04) - PLID 65197 - Add a new import type, Resources, to the import utility
	CString m_strResourceFields, m_strResourceValues;
	// (r.farnworth 2015-03-19 09:17) - PLID 65238 - Add a new import type, Products, to the import utility
	CString m_strProductServiceTFields, m_strProductServiceTValues;
	CString m_strProductProductTFields, m_strProductProductTValues;
	CString m_strProductProductLocationInfoTFields, m_strProductProductLocationInfoTValues;
	CString m_strProductProductAdjustmentsTFields, m_strProductProductAdjustmentsTValues;
	long m_nProductOnHandAmt; // (r.farnworth 2015-03-20 14:03) - PLID 65244
	// (b.savon 2015-03-20 08:45) - PLID 65153 - Add Race
	CString m_strPersonRaceFields, m_strPersonRaceValues;
	// (b.savon 2015-03-20 15:01) - PLID 65154 - Add Ethnicity
	CString m_strPersonEthnicityFields, m_strPersonEthnicityValues;
	// (b.savon 2015-03-23 07:32) - PLID 65155 - Add Language
	CString m_strPersonLanguageFields, m_strPersonLanguageValues;
	// (b.savon 2015-03-23 10:41) - PLID 65157 - Add Location
	CString m_strLocationFields, m_strLocationValues;
	// (b.savon 2015-03-23 11:28) - PLID 65158 - Add Referral Source Name
	CString m_strReferralSourceFields, m_strReferralSourceValues;
	// (b.savon 2015-03-25 07:39) - PLID 65144 - Add Custom Fields
	CString m_strCustomFieldsQuery;
	// (b.savon 2015-03-25 13:26) - PLID 65150 - Add ProviderID
	CString m_strPatientProviderFields, m_strPatientProviderValues;
	// (b.savon 2015-03-26 13:26) - PLID 65151 - Add Referring Physician ID
	CString m_strPatientReferringPhysFields, m_strPatientReferringPhysValues;
	// (b.savon 2015-03-26 14:23) - PLID 65152 - Add PCP Name 
	CString m_strPatientPCPFields, m_strPatientPCPValues;
	// (b.savon 2015-04-01 11:38) - PLID 65235 - Add Recall date
	CString m_strPatientRecallFields, m_strPatientRecallValues;
	// (b.savon 2015-04-02 09:17) - PLID 65236 - Add Recall Template Name
	CString m_strPatientRecallTemplateFields, m_strPatientRecallTemplateValues;
	// (r.farnworth 2015-04-01 14:51) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
	CString m_strInsuranceCoTFields, m_strInsuranceCoTValues;
	CString m_strInsuranceContactFields, m_strInsuranceContactValues;

	//(s.dhole 4/7/2015 10:30 AM ) - PLID  65224 new patient note 
	CString m_strPatientNotesFields, m_strPatientNotesValues ;
	CString m_strPatientNoteCategoryFields, m_strPatientNoteCategoryValues;
	CString m_strPatientNoteIDFields, m_strPatientNoteIDValues;
	CString m_strPatientNotePriorityFields, m_strPatientNotePriorityValues;
	CString m_strPatientNotesTextFields, m_strPatientNotesTextValues;
	CString m_strPatientIDFields, m_strPatientIDValues;

	
	//(s.dhole 4/28/2015 10:44 AM ) - PLID 65190 Insured party 
	CString m_strInsurdPartyFields, m_strInsurdPartyValues;
	CString  m_strInsurdPartyInsuCoNameValues;
	CString  m_strInsurdPartyInsuConvIDValues;
	CString m_strInsurdPartyCoPayFields, m_strInsurdPartyCoPayValues;
	CString m_strInsurdPartyInsuCoName, m_strInsurdPartyInsuConvId;
	//(s.dhole 4/28/2015 10:20 AM ) - PLID 65195  Insured Reposiblity type
	CString m_strInsurdRespoTypeValue;

	// (b.savon 2015-04-07 10:19) - PLID 65216 - Create fields for the Appointment object for the import utility.
	CString m_strAppointmentFields, m_strAppointmentValues;
	// (b.savon 2015-04-07 15:24) - PLID 65220 - Add Appointment Type
	CString m_strAppointmentTypeFields, m_strAppointmentTypeValues;
	// (b.savon 2015-04-10 10:58) - PLID 65221 - Add Appointment Purpose 
	CString m_strAppointmentPurposeFields, m_strAppointmentPurposeValues;
	// (b.savon 2015-04-13 08:13) - PLID 65219 - Store appointment data for group dependency insertion
	CString m_strAppointmentDate, m_strAppointmentStartTime, m_strAppointmentEndTime, m_strAppointmentDuration, m_strAppointmentEvent;
	// (b.savon 2015-04-22 14:05) - PLID 65219 - Add Appointment audit
	CString m_strAuditAppointmentValues;
	// (r.goldschmidt 2016-01-28 11:56) - PLID 67974 - hold values of items that go into rebuilt appt note
	CString m_strAppointmentTypeReplacedValue, m_strAppointmentPurposeReplacedValue, m_strAppointmentNoteReplacedValue;
	// (r.goldschmidt 2016-01-28 15:56) - PLID 67976 - carry over settings from fields to preview
	BOOL m_bApptConversionChecked, m_bApptNotesChecked;
	bool m_bAddedApptType;
	CString m_strApptNotesCheckboxText, m_strOriginalInstructionText, m_strAdditionalExplanatoryText;
	// (r.goldschmidt 2016-01-28 12:37) - PLID 67974 - used to build note from type and purpose and note when the setting is enabled
	CString ConstructAppointmentNote();

	// (r.goldschmidt 2016-02-10 12:30) - PLID 68163 - store race data
	bool m_bPairedToRaceCDCCode, m_bHasRacePreferredName;
	CString m_strRaceCDCCodeValue, m_strRacePreferredNameValue;

	BOOL m_bHasPatientID;
	// (j.jones 2010-04-05 11:27) - PLID 16717 - we will fill in the SubCode if the user does not
	BOOL m_bHasSubCode;

	CMap<CString, LPCTSTR, long, long> m_mapPatientIDs; // (r.farnworth 2015-03-23 11:49) - PLID 65246

	void LoadMediNotesFile();
	void CreateColumns(ImportRecordType irtSelected);
	long GetNumberOfPatients(CFile *InFile);
	void CreatePreviewFromCopy();
	void SaveImportedData();
	void AppendValue(ImportRecordType irtCurrent, CString strDataField, _variant_t varValue);
	void ShowIgnoredFields(BOOL bShow);
	void ClearPreview();
	// (b.savon 2015-03-24 12:34) - PLID 65251
	bool ShouldFixCaps(const CString &strField);

	// (b.savon 2015-04-30 16:00) - PLID 65511
	void AddImportDeclareVariablesToBatch(CString &strSqlBatch);
	void ResetFieldsAndValues();
	void RunBatch(long nAuditTransactionID, const CString &strSqlBatch);

	void AddAppointmentToBatch(CString &strSqlBatch);
	void AddPatientNoteToBatch(CString &strSqlBatch);
	void AddRecallToBatch(CString &strSqlBatch);
	void AddServiceCodeToBatch(CString &strSqlBatch);
	void AddProductToBatch(CString &strSqlBatch);
	void AddResourceToBatch(CString &strSqlBatch);

	void AddPersonToBatch(CString &strSqlBatch);
	void AddInsuranceCompanyToBatch(CString &strSqlBatch);
	void AddInsuredPartyToBatch(CString &strSqlBatch);
	void AddOtherContactToBatch(CString &strSqlBatch);
	void AddSupplierToBatch(CString &strSqlBatch);
	void AddUserToBatch(CString &strSqlBatch);
	void AddReferringPhysicianToBatch(CString &strSqlBatch);
	void AddProviderToBatch(CString &strSqlBatch);
	void AddPatientToBatch(CString &strSqlBatch);

	// (r.goldschmidt 2016-02-10 11:23) - PLID 68163 - Split batch logic out
	void AddRaceToBatch(CString &strSqlBatch);

	// Generated message map functions
	//{{AFX_MSG(CImportWizardPreviewDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnWizardFinish();
	virtual LRESULT OnWizardBack();
	afx_msg void OnImportShowIgnored();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTWIZARDPREVIEWDLG_H__C4A29484_1E70_430D_9D55_8621E5999382__INCLUDED_)
