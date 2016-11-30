// ReconciliationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ReconciliationDlg.h"
#include "AuditTrail.h"
#include "EmrUtils.h"
#include "DecisionRuleUtils.h"
#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/ref.hpp"
#include "boost/make_shared.hpp"
#include "foreach.h"


// (s.dhole 2013-09-18 14:16) - PLID 56625 create
using namespace ADODB;
using namespace NXDATALIST2Lib;

// CReconciliationDlg dialog

IMPLEMENT_DYNAMIC(CReconciliationDlg, CNxDialog)

#define COLOR_PRESCRIPTION_MED RGB(239,228,176)
#define COLOR_CURRENT_MED_BLUE RGB(153,217,234)
#define COLOR_CURRENT_MED_DISCONTINUED RGB(222, 225, 231)
enum ReconciliationTBColumns
{
	erctcInternalID =0 ,
	erctcCheckbox,
	erctcCodeID,
	erctcName ,
	erctcDescription ,
	erctcStatus ,
	erctcIsActive ,
	erctcDate ,
	erctcCompText,
};


// (s.dhole 2013-11-26 10:14) - PLID 56625
typedef std::map<CString,long> mapListIDToRxNormCodes;
mapListIDToRxNormCodes m_mapListIDToRxNormCodes;


CReconciliationDlg::CReconciliationDlg(long nPatientID,long  nMailSentID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CReconciliationDlg::IDD, pParent)
{
	m_clrBackReconcilation =0;
	m_nReconciliationType =erNone ;
	m_nPatientID = nPatientID;
	m_strResultText="";
	m_nMailSentID = nMailSentID;
}


CReconciliationDlg::~CReconciliationDlg()
{
	m_mapListIDToRxNormCodes.clear();
}

void CReconciliationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_BTN_RECONCILIATION_PREVIEW_MERGE2, m_btnOkReconcilation);
	DDX_Control(pDX, ID_BTN_RECONCILIATION_CANCEL2, m_btnCancelReconcilation);
	DDX_Control(pDX, IDC_RECONCILIATION_BKG, m_nxcBackReconcilation);	
	DDX_Control(pDX, IDC_LBL_NEW, m_nxeNewlabel);	
	DDX_Control(pDX, IDC_LBL_CURRENT, m_nxeCurrentlabel);	
}

BEGIN_MESSAGE_MAP(CReconciliationDlg, CNxDialog)
	ON_BN_CLICKED(ID_BTN_RECONCILIATION_PREVIEW_MERGE2, &CReconciliationDlg::OnBnClickedBtnReconciliationPreviewMerge)
	ON_BN_CLICKED(ID_BTN_RECONCILIATION_CANCEL2, &CReconciliationDlg::OnBnClickedBtnReconciliationCancel)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CReconciliationDlg, CNxDialog)
	ON_EVENT(CReconciliationDlg, IDC_NEW_RECONCILIATION_LIST, 9, CReconciliationDlg::EditingFinishingNewReconciliationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// (s.dhole 2013-10-30 11:44) - PLID 59229
BOOL CReconciliationDlg::IsDateTimeValid(const COleDateTime& dt) const
{
	return (dt.GetStatus() == COleDateTime::valid && dt.m_dt > 0) ? TRUE : FALSE;
}

// (b.spivey, October 30, 2015) PLID 67423 - reconcile all option. 
void CReconciliationDlg::ReconcileAll(long nPatientID, long nMailSentID, CWnd* pParent)
{
	COLORREF cr = GetNxColor(GNC_PATIENT_STATUS, nPatientID);
	{
		CReconciliationDlg dlgMeds(nPatientID, nMailSentID, pParent);
		dlgMeds.m_nReconciliationType = CReconciliationDlg::erMedication;
		dlgMeds.SetBackColor(cr);
		dlgMeds.m_strPreviewDlgOkButtonTextOverride = "Continue to Allergies";
		if (dlgMeds.DoModal() == IDCANCEL) {
			return;
		}
		else if (!dlgMeds.m_strResultText.IsEmpty()) {
			AfxMessageBox(dlgMeds.m_strResultText);
		}
	}

	{
		CReconciliationDlg dlgAllergies(nPatientID, nMailSentID, pParent);
		dlgAllergies.m_nReconciliationType = CReconciliationDlg::erAllergy;
		dlgAllergies.SetBackColor(cr);
		dlgAllergies.m_strPreviewDlgOkButtonTextOverride = "Continue to Problem";
		if (dlgAllergies.DoModal() == IDCANCEL) {
			return;
		}
		else if (!dlgAllergies.m_strResultText.IsEmpty()) {
			AfxMessageBox(dlgAllergies.m_strResultText);
		}
	}

	{
		CReconciliationDlg dlgProblems(nPatientID, nMailSentID, pParent);
		dlgProblems.m_nReconciliationType = CReconciliationDlg::erProblem;
		dlgProblems.SetBackColor(cr);
		dlgProblems.m_strPreviewDlgOkButtonTextOverride = "Finish";
		if (dlgProblems.DoModal() == IDCANCEL) {
			return;
		}
		else if (!dlgProblems.m_strResultText.IsEmpty()) {
			AfxMessageBox(dlgProblems.m_strResultText); 
		}
	}
}

// (s.dhole 2013-11-18 09:55) - PLID 59235
// Check Last date. Modified date  at end start date
COleDateTime  CReconciliationDlg::GetParsedDate(_variant_t varStartDate,_variant_t varEndDate,_variant_t varModifiedDate) 
{
	COleDateTime dtReturn;
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	if (varEndDate != g_cvarNull) {
		dtReturn= VarDateTime( varEndDate);
	}
	else if (varModifiedDate != g_cvarNull) {
		dtReturn= VarDateTime( varModifiedDate);
	}
	else if (varStartDate != g_cvarNull) {
		dtReturn= VarDateTime( varStartDate);
	}
	else{
		dtReturn=dtInvalid;
	}
	return dtReturn;
}

// (s.dhole 2013-11-18 14:33) - PLID 59235 Parse status  info
// most of the time we will recive status as copleted , based on date we will find out status
// if end data is missing then it is active, even status is completed, wewil always recive status as completed
// if end date if exist then discontinue
CString  CReconciliationDlg::GetParsedStatus(_variant_t varEndDate,const CString strStatus) 
{
	CString  strResult ="Active";;
	if ((varEndDate != g_cvarNull) && (strStatus.CompareNoCase("Completed")==0 )) {
		strResult="Inactive";
	}
	// in case there is another status
	else if ((varEndDate == g_cvarNull) && (strStatus.CompareNoCase("Completed")!=0 )) {
		strResult="Active";
	}
	
	return strResult;
}

// (s.dhole 2013-11-18 14:34) - PLID 59235 load CCDA file data an retul ponter
NexTech_Accessor::_ParsedCCDAResultsPtr CReconciliationDlg::GetParsedCCDAResultsPtr (ReconciliationType nType) 
{
	NexTech_Accessor::_PracticeMethodsPtr pAPI = GetAPI();
	if(pAPI == NULL) {
		ThrowNxException("Failed to connect to the API.");
	}
	NexTech_Accessor::_CCDAParseOptionsPtr  CCDAParseOption(__uuidof(NexTech_Accessor::CCDAParseOptions));
	CCDAParseOption->ParseAllergies =FALSE;
	CCDAParseOption->ParseMedications =FALSE;
	CCDAParseOption->ParseProblems =FALSE;
	CCDAParseOption->ParseAuthor =TRUE;
	switch (m_nReconciliationType)
	{
	case erAllergy:
		{
			CCDAParseOption->ParseAllergies=TRUE;
		}
		break;
		case erProblem:
		{
			CCDAParseOption->ParseProblems=TRUE;
		}
		break;
		case erMedication:
		{
			CCDAParseOption->ParseMedications=TRUE;
		}
		break;
	}
	NexTech_Accessor::_ParsedCCDAResultsPtr  pParsedCCDA= pAPI->ParseCCDA(GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString( m_nMailSentID)),CCDAParseOption);
	// load author information
	if(pParsedCCDA){
		NexTech_Accessor::_ParsedCCDAAuthorPtr  ParsedCCDAAuthorPtr = pParsedCCDA->GetParsedAuthor();
		if (ParsedCCDAAuthorPtr){
		CString  strImportFrom =FormatString("From CCDA Document= %s\n\rName= %s; Office= %s",
			VarString(ParsedCCDAAuthorPtr->DocumentTitle,""),VarString(ParsedCCDAAuthorPtr->AuthorName,""),VarString(ParsedCCDAAuthorPtr->AuthorCompanyName,"")  ) ;
		SetDlgItemText(IDC_LBL_NEW, strImportFrom );
		}
	}
	return pParsedCCDA;
}
	
BOOL CReconciliationDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();
		m_btnOkReconcilation.AutoSet(NXB_MODIFY );
		m_btnCancelReconcilation.AutoSet(NXB_CANCEL);
		extern CPracticeApp theApp;
		GetDlgItem(IDC_HEADER_LBL)->SetFont(&theApp.m_boldFont);
		m_nxcBackReconcilation.SetColor(m_clrBackReconcilation);
		m_dlReconciliationNew = BindNxDataList2Ctrl(IDC_NEW_RECONCILIATION_LIST, false);		
		m_dlReconciliationExist = BindNxDataList2Ctrl(IDC_EXISTING_RECONCILIATION_LIST, false);		
		SetColumnHeaders();
		// (s.dhole 2013-11-26 16:52) - PLID 59235
		if (LoadData()==FALSE){
			switch (m_nReconciliationType)
			{
			case erAllergy:
				{
					m_strResultText = "There are no valid allergies (Allergies with valid RxNorm cCodes) available in the selected CCDA document.";
				}
				break;
			case erProblem:
				{
					m_strResultText = "There are no valid problems (Problems with valid SNOMED codes) available in the selected CCDA document.";
				}
					break;
			case erMedication:
				{
					m_strResultText = "There are no valid medications (Medications with valid RxNorm codes) available in the selected CCDA document." ;
				}
				break;
			}
		EndDialog(IDOK);
		}
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CReconciliationDlg::EditingFinishingNewReconciliationList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		SetExistingItemFlag(pRow,VarBool(pRow->GetValue(erctcCheckbox)));
	}NxCatchAll(__FUNCTION__);
}
// CReconciliationDlg message handlers

void CReconciliationDlg::OnBnClickedBtnReconciliationPreviewMerge()
{
	// TODO: Add your control notification handler code here
	ValidateAndSaveData();

}

void CReconciliationDlg::OnBnClickedBtnReconciliationCancel()
{
	try {
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	}
	NxCatchAll(__FUNCTION__);
}

// Based on matchin record , shoew check bos to exiting item list
void CReconciliationDlg::SetExistingItemFlag(NXDATALIST2Lib::IRowSettingsPtr pRow,BOOL bShoCheckBox)
{
	IRowSettingsPtr pRowExist = m_dlReconciliationExist->FindByColumn(erctcCompText, _bstr_t(VarString(pRow->GetValue(erctcCompText), "")), NULL, VARIANT_FALSE);
	if (pRowExist)
	{
		IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
		pfs->PutDataType(VT_BOOL);
		pfs->PutAlignV(vaVCenter);  
		if (bShoCheckBox==FALSE){
			pfs->PutFieldType(cftBoolYesNo);
			pfs->PutEditable(VARIANT_FALSE);
			pRowExist->PutValue(erctcCheckbox,g_cvarTrue);
		}
		else{
			pfs->PutFieldType(cftBoolCheckbox);
			pfs->PutEditable(VARIANT_TRUE);
		}
		pRowExist->PutRefCellFormatOverride(erctcCheckbox,pfs) ;
	}
}

// (s.dhole 2013-10-30 15:10) - PLID 59235 Load current medication
void CReconciliationDlg::LoadCurrentMedicationData()
{
// Load discontinued medication
	NexTech_Accessor::_PracticeMethodsPtr pAPI = GetAPI();
	if(pAPI == NULL) {
		ThrowNxException("Failed to connect to the API.");
	}
	NexTech_Accessor::_PatientCurrentMedicationsForReconciliationPtr pMedication= pAPI->GetPatientCurrentMedicationsForReconciliation(GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString( m_nPatientID )));
	Nx::SafeArray<IUnknown *> saryMedications = pMedication->GetMedications();
		foreach(NexTech_Accessor::_PatientCurrentMedicationForReconciliationPtr    pMedicationPtr, saryMedications )
		{
			ReconcilationList rl;
			rl.nInternelID = atol(VarString(pMedicationPtr->ID));
			rl.nCodeID = atol(VarString(pMedicationPtr->MedID));
			rl.strName = VarString(pMedicationPtr->MedicationName );
			rl.strDesc = VarString(pMedicationPtr->Sig);
			rl.strCompTxt = VarString(pMedicationPtr->RXCUI);
			if (pMedicationPtr->Discontinued == VARIANT_TRUE){
				rl.bActive = VARIANT_FALSE;
			}
			else{
				rl.bActive = VARIANT_TRUE;
			}
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			COleDateTime dtUpdatDate(pMedicationPtr->LastUpdateDate);
			if (dtUpdatDate && dtUpdatDate.GetStatus() == COleDateTime::valid) {
				rl.dtLastDate = dtUpdatDate;  
			}
			else{
				rl.dtLastDate =dtInvalid;  
			}
			m_aCurrentListArray.Add(rl);
		}
}


// (s.dhole 2013-10-30 15:04) - PLID 56928 Load medication from CCDA document
void CReconciliationDlg::LoadNewMedicationData()
{
	NexTech_Accessor::_ParsedCCDAResultsPtr  pParsedCCDA= GetParsedCCDAResultsPtr(erMedication);
	Nx::SafeArray<IUnknown *> saryMedications = pParsedCCDA->GetParsedMedications();

	std::vector<CString> aryRXCUIs;

	foreach(NexTech_Accessor::_ParsedCCDAMedicationPtr pCCDACCDAMedicationPtr, saryMedications)
	{
		CString strRXCUI = VarString(pCCDACCDAMedicationPtr->RXCUI, "");

		// Skip any record if it misssing RXNorm code , this should not happen but it is possible due to issue exporting CCDA
		if (strRXCUI.IsEmpty()) {
			continue;
		}

		// (j.jones 2015-12-30 10:23) - PLID 67771 - don't add the same RXCUI twice
		bool bFound = false;
		foreach(CString strCheckRXCUI, aryRXCUIs) {
			if (strRXCUI == strCheckRXCUI) {
				bFound = true;
				break;
			}
		}
		if (bFound) {
			continue;
		}
		//cache this RXCUI
		aryRXCUIs.push_back(strRXCUI);

		ReconcilationList rl;
		rl.nInternelID= -1;
		rl.nCodeID = -1;
		rl.strName = VarString( pCCDACCDAMedicationPtr->Name);
		CString strNote = VarString(pCCDACCDAMedicationPtr->note,"");
		if (!strNote.IsEmpty()){
			strNote.Replace(rl.strName,"");
			// remove "-" if it is at begining
			if (strNote.Find("-")==0){
				strNote =strNote.Right(strNote.GetLength()-1);
			}
		}
		rl.strDesc= Trim(strNote);
		rl.strCompTxt = strRXCUI;
		CString strActive= GetParsedStatus(pCCDACCDAMedicationPtr->EndDate->GetValue(),
											VarString(pCCDACCDAMedicationPtr->Getstatus()));
		if (strActive.CollateNoCase("Active")==0 ){ 
			rl.bActive= VARIANT_TRUE;
		}
		else{
			rl.bActive = VARIANT_FALSE;
		}
		// this value is always exist, 
		rl.dtLastDate =GetParsedDate(pCCDACCDAMedicationPtr->StartDate->GetValue() ,
		pCCDACCDAMedicationPtr->EndDate->GetValue(),
		pCCDACCDAMedicationPtr->ModifiedDate->GetValue());
		m_aNewListArray.Add(rl);
	}
}

// Load current allergies
void CReconciliationDlg::LoadCurrentAllergyData()
{
	NexTech_Accessor::_PracticeMethodsPtr pAPI = GetAPI();
	if(pAPI == NULL) {
		ThrowNxException("Failed to connect to the API.");
	}
	NexTech_Accessor::_PatientAllergiesForReconciliationPtr pAllergy= pAPI->GetPatientAllergiesForReconciliation(GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString( m_nPatientID )));
	Nx::SafeArray<IUnknown *> saryAllergies = pAllergy->Allergies  ;

	std::vector<CString> aryAllergyIDs;

	foreach(NexTech_Accessor::_PatientAllergyForReconciliationPtr    pAllergyPtr, saryAllergies )
	{
		CString strAllergyID = VarString(pAllergyPtr->ID);

		// (j.jones 2015-12-30 10:23) - PLID 67771 - don't add the same allergy ID twice
		bool bFound = false;
		foreach(CString strCheckAllergyID, aryAllergyIDs) {
			if (!strAllergyID.IsEmpty() && strAllergyID == strCheckAllergyID) {
				bFound = true;
				break;
			}
		}
		if (bFound) {
			continue;
		}
		//cache this ID
		if (!strAllergyID.IsEmpty()) {
			aryAllergyIDs.push_back(strAllergyID);
		}

		ReconcilationList rl;
		rl.nInternelID= atol(VarString(pAllergyPtr->PatientAllergyID) );
		rl.nCodeID = atol(strAllergyID);
		rl.strName= VarString(pAllergyPtr->Name) ;
		rl.strDesc= VarString(pAllergyPtr->RactionNote) ;
		rl.strCompTxt= VarString(pAllergyPtr->RXCUI) ;
		if (pAllergyPtr->Discontinued == VARIANT_TRUE){
			rl.bActive= VARIANT_FALSE;
		}
		else{
			rl.bActive = VARIANT_TRUE;
		}
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		COleDateTime dtUpdatDate(pAllergyPtr->LastUpdateDate);
		if (dtUpdatDate && dtUpdatDate.GetStatus() == COleDateTime::valid) {
			rl.dtLastDate = dtUpdatDate;  
		}
		else{
			rl.dtLastDate =dtInvalid;  
		}
		m_aCurrentListArray.Add(rl);
	}
}
// Load new allergies
// (s.dhole 2013-10-30 14:48) - PLID 56625 Load allergy from CCDA
void CReconciliationDlg::LoadNewAllergyData()
{
	NexTech_Accessor::_ParsedCCDAResultsPtr  pParsedCCDA= GetParsedCCDAResultsPtr (erAllergy);
	Nx::SafeArray<IUnknown *> saryAllergies = pParsedCCDA->GetParsedAllergies();
	foreach(NexTech_Accessor::_ParsedCCDAAllergyPtr   pCCDACCDAAllergyPtr, saryAllergies)
	{
		// Skip any record if it misssing RXNorm code , this should not happen but it is possible due to issue exporting CCDA
		if (VarString(pCCDACCDAAllergyPtr->GetRXCUI(),"")!=""){
			ReconcilationList rl;
			rl.nInternelID= -1;
			rl.nCodeID = -1;
			rl.strName= VarString( pCCDACCDAAllergyPtr->GetName());
			CString strReaction ="";
			Nx::SafeArray<IUnknown *> saryAllergiesReaction = pCCDACCDAAllergyPtr->GetReactions() ;
			foreach(NexTech_Accessor::_ParsedCCDAAllergyReactionPtr    pAllergyReactionPtr, saryAllergiesReaction )
			{
				strReaction += VarString( pAllergyReactionPtr->Name,""); 
				strReaction += " ";  
			}
			if (VarString(pCCDACCDAAllergyPtr->Getnote(),"")!="" )
			{
				strReaction += " ";  
				strReaction += VarString(pCCDACCDAAllergyPtr->Getnote(),"");
			}
			rl.strDesc= strReaction ;
			rl.strCompTxt= VarString(pCCDACCDAAllergyPtr->GetRXCUI());
			CString strActive= GetParsedStatus(pCCDACCDAAllergyPtr->EndDate->GetValue(),
												VarString(pCCDACCDAAllergyPtr->Getstatus()));
			if (strActive.CollateNoCase("Active")==0 ){ 
				rl.bActive= VARIANT_TRUE;
			}
			else{
				rl.bActive = VARIANT_FALSE;
			}
			rl.dtLastDate =GetParsedDate(pCCDACCDAAllergyPtr->StartDate->GetValue() ,
			pCCDACCDAAllergyPtr->EndDate->GetValue(),
			pCCDACCDAAllergyPtr->ModifiedDate->GetValue());
			m_aNewListArray.Add(rl);
		}
	}
}



/* (s.dhole 2013-10-30 10:06) - PLID 56626 Load Snomed Code problem List
1) associated with patient and not link to any other items(EMN, Lab ext)
2)  Problem have Snomed Code
3)  If there are problem with same  code multiple time then use only current problem , not old one
*/
// (a.walling 2014-10-16 16:59) - PLID 62911 - Update for SNOMEDCT_US
void CReconciliationDlg::LoadCurrentProblemData()
{
	// Load discontinued unsigned problem =EMRRegardingType = 9  

	_RecordsetPtr prs = CreateParamRecordset(
		" SET NOCOUNT ON;  \r\n"
		" DECLARE @ProblemsTempT TABLE( \r\n"
		" InternalID INT NOT NULL, \r\n"
		" CodeID INT NOT NULL, \r\n"
		" CodeDescription NVARCHAR(MAX) NOT NULL, \r\n"
		" Code  NVARCHAR(100) NOT NULL, \r\n"
		" vocab  NVARCHAR(50) NOT NULL, \r\n"
		" ProblemDescription  NVARCHAR(MAX) NULL, \r\n"
		" PatientID INT NOT NULL, \r\n"
		" Discontinued INT NOT NULL, \r\n"
		" ModifiedDate datetime NOT NULL \r\n"
		" ); \r\n"
		" INSERT INTO @ProblemsTempT (InternalID,CodeID,CodeDescription,Code,vocab,ProblemDescription,PatientID,Discontinued,ModifiedDate) \r\n"
		" SELECT  EMRProblemsT.ID AS InternalID,  \r\n"
		" EMRProblemsT.CodeID  AS CodeID,   \r\n"
		"  ISNULL(CodesT.Name,'') AS CodeDescription,   \r\n"
		" ISNULL(CodesT.Code,'') AS code,  \r\n"
		" ISNULL(CodesT.vocab,'') AS vocab,  \r\n"
		" EMRProblemsT.Description  AS ProblemDescription,    \r\n"
		" EMRProblemsT.PatientID,    \r\n"
		" EMRProblemsT.StatusID AS Discontinued,    \r\n"
		" EMRProblemsT.ModifiedDate    \r\n"
		" FROM EMRProblemLinkT INNER JOIN EMRProblemsT    \r\n"
		" ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID    \r\n"
		" INNER  JOIN CodesT    \r\n"
		" ON EMRProblemsT.CodeID = CodesT.ID   \r\n"
		//" WHERE EMRProblemsT.Deleted=0 AND  EMRProblemLinkT.EMRRegardingType = 9   \r\n"
		" WHERE EMRProblemsT.Deleted=0 and CodesT.vocab = 'SNOMEDCT_US'\r\n"
		//" AND EMRProblemsT.ID   IN( SELECT EMRProblemLinkT.EMRProblemID from EMRProblemLinkT  \r\n"
		//" INNER JOIN EMRProblemsT ON \r\n"
		//" EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID   AND EMRProblemsT.PatientID = {INT} \r\n"
		//" Where EMRRegardingType != 9  AND  CodesT.vocab = 'SNOMEDCT_US' )  \r\n"
		//" Where CodesT.vocab = 'SNOMEDCT_US' )  \r\n"
		" AND EMRProblemsT.PatientID = {INT} AND ISNULL(CodesT.Code,'')!='';  \r\n"
		" SET NOCOUNT OFF;  \r\n"
		" SELECT * FROM \r\n"
		" (SELECT DISTINCT TempQ.* FROM @ProblemsTempT AS TempQ \r\n"
		" INNER JOIN  \r\n"
		" (Select CodeID,MAX(ModifiedDate) AS ModifiedDate   from @ProblemsTempT Group By CodeID  having COUNT (CodeID)>1) AS TempMultiQ \r\n"
		" ON (TempQ.CodeID = TempMultiQ.CodeID AND TempQ.ModifiedDate = TempMultiQ.ModifiedDate) \r\n"
		" GROUP BY TempQ.InternalID, TempQ.CodeID, TempQ.CodeDescription, TempQ.Code, TempQ.vocab, \r\n"
		"  TempQ.ProblemDescription, TempQ.PatientID, TempQ.Discontinued, TempQ.ModifiedDate  \r\n"
		" UNION ALL \r\n"
		" SELECT TempQ.* FROM @ProblemsTempT AS TempQ \r\n"
		" INNER JOIN  \r\n"
		" (Select CodeID,MAX(ModifiedDate) AS ModifiedDate   from @ProblemsTempT Group By CodeID  having COUNT (CodeID)=1) AS TempMultiQ \r\n"
		" ON (TempQ.CodeID = TempMultiQ.CodeID AND TempQ.ModifiedDate = TempMultiQ.ModifiedDate) \r\n"
		" GROUP BY TempQ.InternalID, TempQ.CodeID, TempQ.CodeDescription, TempQ.Code, TempQ.vocab, \r\n"
		"  TempQ.ProblemDescription, TempQ.PatientID, TempQ.Discontinued, TempQ.ModifiedDate ) AS _Q \r\n"
		,m_nPatientID ,m_nPatientID);
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		 
		ReconcilationList rl;
		rl.nInternelID= AdoFldLong(f, "InternalID");
		rl.nCodeID = AdoFldLong(f, "CodeID");
		CString strCodeType =  AdoFldString(f, "vocab", ""); 
		if (strCodeType.Find("SNOMEDCT")==0) {
			rl.strName = FormatString("SNOMED CT® Code: %s",AdoFldString(f, "code", ""));
		}
		rl.strDesc= AdoFldString(f, "CodeDescription", "");
		rl.strCompTxt= AdoFldString(f, "code", "");
		
		if (AdoFldLong (f, "Discontinued",-1) != 2){ //EMRProblemsT.StatusID =2 close;  Discontinue
			rl.bActive= VARIANT_TRUE;
		}
		else{
			rl.bActive = VARIANT_FALSE;
		}
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		rl.dtLastDate = AdoFldDateTime (f, "ModifiedDate" ,dtInvalid);
		m_aCurrentListArray.Add(rl);
		prs->MoveNext();
	}
}

// (s.dhole 2013-10-30 09:53) - PLID 56933 Place Holder to Snomed code import
void CReconciliationDlg::LoadNewProblemData()
{

	NexTech_Accessor::_ParsedCCDAResultsPtr  pParsedCCDA= GetParsedCCDAResultsPtr (erProblem );
	Nx::SafeArray<IUnknown *> saryProblems = pParsedCCDA->GetParsedProblems();
		foreach(NexTech_Accessor::_ParsedCCDAProblemPtr    pCCDAProblemPtr, saryProblems)
		{
			if( VarString(pCCDAProblemPtr->SNOMEDCode,"")!=""){
				ReconcilationList rl;
				rl.nInternelID= -1;
				rl.nCodeID = -1;
				// there is no code found in CCDA to support US Extension to SNOMED , wil keep this code in case we need to change
				//CString strCodeType  =  VarString(pCCDAProblemPtr->) ); 
				//if (strCodeType.Find("SNOMEDCT")==0) {
					rl.strName = FormatString("SNOMED CT® Code: %s",VarString(pCCDAProblemPtr->SNOMEDCode) );
				//}
				rl.strDesc= VarString(pCCDAProblemPtr->Name) ;
				rl.strCompTxt= VarString(pCCDAProblemPtr->SNOMEDCode);
				CString strActive= GetParsedStatus(pCCDAProblemPtr->EndDate->GetValue(),
				VarString(pCCDAProblemPtr->Getstatus()));
				if (strActive.CollateNoCase("Active")==0 ){ 
					rl.bActive= VARIANT_TRUE;
				}
				else{
					rl.bActive = VARIANT_FALSE;
				}
				rl.dtLastDate =GetParsedDate(pCCDAProblemPtr->StartDate->GetValue() ,
				pCCDAProblemPtr->EndDate->GetValue(),
				pCCDAProblemPtr->ModifiedDate->GetValue());
				m_aNewListArray.Add(rl);
			}
		}

	
}


void CReconciliationDlg::UpdateColumnCaption(NXDATALIST2Lib::_DNxDataListPtr ListPtr, int  nColumn, CString strCaption )	
{
	IColumnSettingsPtr pCol = ListPtr->GetColumn(nColumn);
	pCol->PutColumnTitle(_bstr_t( strCaption) );
}

void CReconciliationDlg::SetColumnHeaders()
{
	switch (m_nReconciliationType)
	{
	case erAllergy:
		{
			SetWindowText("Reconcile Allergies");
			UpdateColumnCaption(m_dlReconciliationNew,erctcName,"Allergy");
			UpdateColumnCaption(m_dlReconciliationNew,erctcDescription,"Note/Reaction");
			UpdateColumnCaption(m_dlReconciliationNew,erctcStatus,"Status");
			UpdateColumnCaption(m_dlReconciliationNew,erctcDate,"Last modification date");
			UpdateColumnCaption(m_dlReconciliationExist,erctcName,"Allergy");
			UpdateColumnCaption(m_dlReconciliationExist,erctcDescription,"Note/Reaction");
			UpdateColumnCaption(m_dlReconciliationExist,erctcStatus,"Status");
			UpdateColumnCaption(m_dlReconciliationExist,erctcDate,"Last modification date");
		}
		break;
	case erProblem:
		{
			// (s.dhole 2013-10-30 10:34) - PLID PLID 56626  Load column info
			SetWindowText("Reconcile Problems");
			UpdateColumnCaption(m_dlReconciliationNew,erctcName,"Code");
			UpdateColumnCaption(m_dlReconciliationNew,erctcDescription,"Description");
			UpdateColumnCaption(m_dlReconciliationNew,erctcStatus,"Status");
			UpdateColumnCaption(m_dlReconciliationNew,erctcDate,"Last modification Date");
			UpdateColumnCaption(m_dlReconciliationExist,erctcName,"Code");
			UpdateColumnCaption(m_dlReconciliationExist,erctcDescription,"Description");
			UpdateColumnCaption(m_dlReconciliationExist,erctcStatus,"Status");
			UpdateColumnCaption(m_dlReconciliationExist,erctcDate,"Last modification Date");
		}
		break;
	case erMedication:
		{
			// (s.dhole 2013-10-30 15:10) - PLID 59235
			SetWindowText("Reconcile Medications");
			UpdateColumnCaption(m_dlReconciliationNew,erctcName,"Name");
			UpdateColumnCaption(m_dlReconciliationNew,erctcDescription,"Sig");
			UpdateColumnCaption(m_dlReconciliationNew,erctcStatus,"Status");
			UpdateColumnCaption(m_dlReconciliationNew,erctcDate,"Date");
			UpdateColumnCaption(m_dlReconciliationExist,erctcName,"Name");
			UpdateColumnCaption(m_dlReconciliationExist,erctcDescription,"Sig");
			UpdateColumnCaption(m_dlReconciliationExist,erctcStatus,"Status");
			UpdateColumnCaption(m_dlReconciliationExist,erctcDate,"Last Update");
		}
		break;
	default:
		break;
	}
}


BOOL CReconciliationDlg::LoadData()
{
	BOOL bNewRecordExist = FALSE;
	switch (m_nReconciliationType)
	{
	case erAllergy:
		{
			SetDlgItemText(IDC_HEADER_LBL, "Please check the item which is to be placed in the patient's allergy list, and press Preview And Reconcile... to review your changes.\nItems in beige are patient allergies from selected CCDA document. Items in blue are existing patient current allergies." );
			// (s.dhole 2013-11-01 12:46) - PLID 59278 Set lable based on import
			SetDlgItemText(IDC_LBL_CURRENT, "Patient Current Allergy List" );
			//SetDlgItemText(IDC_LBL_NEW, "List Of Patient Allergies Imported From Selected CCDA Document" );
			LoadCurrentAllergyData();
			LoadNewAllergyData();
		}
		break;
	case erProblem:
		{
			// (s.dhole 2013-10-30 10:08) - PLID 56626 Load proble 
			SetDlgItemText(IDC_HEADER_LBL, "Please check the item which is to be placed in the patient's problem list, and press Preview And Reconcile... to review your changes.\nItems in beige are patient problems from selected CCDA document. Items in blue are existing patient current problems." );
			// (s.dhole 2013-11-01 12:46) - PLID 59278 Set lable based on import
			//SetDlgItemText(IDC_LBL_NEW, "List Of Patient Problems Imported From Selected CCDA Document" );
			SetDlgItemText(IDC_LBL_CURRENT, "Patient Current Problem List" );
			LoadCurrentProblemData();
			LoadNewProblemData();
		}
		break;
	case erMedication:
		{
			// (s.dhole 2013-10-30 15:10) - PLID 59235 
			// (s.dhole 2013-11-01 12:46) - PLID 59278 Set lable based on import
			//SetDlgItemText(IDC_LBL_NEW, "List Of Patient Medications Imported From Selected CCDA Document" );	
			SetDlgItemText(IDC_HEADER_LBL, "Please check the item which is to be placed in the patient's current medication list, and press Preview And Reconcile... to review your changes.\nItems in beige are patient medications from selected CCDA document. Items in blue are existing patient current medications." );
			SetDlgItemText(IDC_LBL_CURRENT, "Patient Current Medication List" );
			LoadCurrentMedicationData();
			LoadNewMedicationData();
		}
		break;
	default:
		break;
	}
	// load existing allergy/problem/Medication list
	for (int i=0; i < m_aCurrentListArray.GetSize(); i++) {
		IRowSettingsPtr pRow = m_dlReconciliationExist->GetNewRow();
		pRow->PutValue(erctcInternalID, m_aCurrentListArray[i].nInternelID);
		pRow->PutValue(erctcCheckbox , g_cvarTrue);
		pRow->PutValue(erctcCodeID, m_aCurrentListArray[i].nCodeID );
		pRow->PutValue(erctcName, _bstr_t(m_aCurrentListArray[i].strName ));
		pRow->PutValue(erctcDescription,_bstr_t( m_aCurrentListArray[i].strDesc));
		pRow->PutValue(erctcStatus, _variant_t(m_aCurrentListArray[i].bActive ? "Active": "Discontinued"));
		if (m_aCurrentListArray[i].bActive == VARIANT_FALSE) {
				pRow->PutCellBackColor(erctcStatus , COLOR_CURRENT_MED_DISCONTINUED );
			}
		else{
			pRow->PutCellBackColor(erctcStatus , COLOR_CURRENT_MED_BLUE);
		}
		pRow->PutValue(erctcIsActive, _variant_t(m_aCurrentListArray[i].bActive ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
    	pRow->PutValue(erctcDate, (m_aCurrentListArray[i].dtLastDate.GetStatus()==COleDateTime::valid)?_variant_t(m_aCurrentListArray[i].dtLastDate,VT_DATE): g_cvarNull);
		pRow->PutValue(erctcCompText, _bstr_t(m_aCurrentListArray[i].strCompTxt));
		pRow->PutCellBackColor(erctcCheckbox, COLOR_CURRENT_MED_BLUE);
		pRow->PutCellBackColor(erctcName, COLOR_CURRENT_MED_BLUE);
		pRow->PutCellBackColor(erctcDescription, COLOR_CURRENT_MED_BLUE);
		pRow->PutCellBackColor(erctcDate, COLOR_CURRENT_MED_BLUE);
		m_dlReconciliationExist->AddRowSorted(pRow, NULL);
		
	}

	// If new item is not active then uncheck selection
	for (int i=0; i < m_aNewListArray.GetSize(); i++) {
		IRowSettingsPtr pRow = m_dlReconciliationNew->GetNewRow();
		pRow->PutValue(erctcInternalID, m_aNewListArray[i].nInternelID);
		pRow->PutValue(erctcCheckbox , _variant_t(m_aNewListArray[i].bActive ? g_cvarTrue: g_cvarFalse) );
		pRow->PutValue(erctcCodeID, m_aNewListArray[i].nCodeID );
		pRow->PutValue(erctcName, _bstr_t(m_aNewListArray[i].strName ));
		pRow->PutValue(erctcDescription,_bstr_t( m_aNewListArray[i].strDesc));
		pRow->PutValue(erctcStatus, _variant_t(m_aNewListArray[i].bActive ? "Active": "Discontinued"));
		pRow->PutValue(erctcIsActive, _variant_t(m_aNewListArray[i].bActive ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
    	pRow->PutValue(erctcDate, (m_aNewListArray[i].dtLastDate.GetStatus()==COleDateTime::valid)?_variant_t(m_aNewListArray[i].dtLastDate,VT_DATE): g_cvarNull);
		pRow->PutValue(erctcCompText, _bstr_t(m_aNewListArray[i].strCompTxt));
		pRow->PutBackColor( COLOR_PRESCRIPTION_MED);
		SetExistingItemFlag(pRow,_variant_t(m_aNewListArray[i].bActive ? FALSE:TRUE));
		m_dlReconciliationNew->AddRowSorted(pRow, NULL);
		bNewRecordExist = TRUE;
	}
	return bNewRecordExist;
}


// (s.dhole 2013-10-30 09:51) - PLID 56935
// (s.dhole 2013-10-30 13:45) - PLID 59232
void CReconciliationDlg::ValidateAndSaveData()
{
	try {
		m_strResultText="";
		// check if there is any medication is checked
		CReconciledItemsArray aReconciledItemsArray;
		CReconcileMergeMedicationsDlg::CReconcileValidationItemArray aReconcileValidationItemArray;
		BOOL bNeedCreatePerms = FALSE; // Create and merge is same action
		BOOL bNeedDeletePerms = FALSE;
		EBuiltInObjectIDs eBuiltInID;
		// (s.dhole 2014-01-13 17:40) - PLID 59643 set edit permission
		BOOL bNeedEditPerms = FALSE;
		// (s.dhole 2014-01-13 17:40) - PLID 59643 
		CString strMsg;
		switch (m_nReconciliationType){
		case erAllergy:
			{
				// (s.dhole 2013-10-30 13:46) - PLID 59232
				eBuiltInID = bioPatientAllergies;
				strMsg = "patient allergies"; // (s.dhole 2014-01-13 17:40) - PLID 59643 
				break;
			}
		case erProblem:
			{
				// (s.dhole 2013-10-30 09:51) - PLID 56935
				eBuiltInID = bioEMRProblems;
				strMsg = "patient problems"; // (s.dhole 2014-01-13 17:40) - PLID 59643 
				break;
			}
		case erMedication:
			{
				eBuiltInID = bioPatientCurrentMeds; 
				strMsg = "patient current medications";// (s.dhole 2014-01-13 17:40) - PLID 59643 
			break;
			}
		default:
			break;
		}
		// (s.dhole 2014-01-13 17:40) - PLID 59643  pass edit permission
		if (GetDataArray(aReconciledItemsArray,aReconcileValidationItemArray,
											 OUT bNeedCreatePerms,OUT bNeedDeletePerms,OUT bNeedEditPerms)){
			// Now that we have the change list, check any necessary permissions. Don't
			// let the user proceed if they don't have access.
			BOOL bCreateAccess = (GetCurrentUserPermissions(eBuiltInID ) & sptCreate) ? TRUE : FALSE;
			BOOL bCreateAccessWithPass = (GetCurrentUserPermissions(eBuiltInID ) & sptCreateWithPass) ? TRUE : FALSE;
			BOOL bDeleteAccess = (GetCurrentUserPermissions(eBuiltInID ) & sptDelete) ? TRUE : FALSE;
			BOOL bDeleteAccessWithPass = (GetCurrentUserPermissions(eBuiltInID ) & sptDeleteWithPass) ? TRUE : FALSE;
			// (s.dhole 2014-01-13 17:34) - PLID 59643 edit permission
			BOOL bEditAccess = (GetCurrentUserPermissions(eBuiltInID ) & sptWrite ) ? TRUE : FALSE;
			BOOL bEditAccessWithPass = (GetCurrentUserPermissions(eBuiltInID ) & sptWriteWithPass) ? TRUE : FALSE;
			BOOL bNeedPassword = FALSE;
			
			BOOL bAllowMerge = TRUE;
			CString strActionType="";
				// Quit if they simply don't have permission of any kind
			// (s.dhole 2014-01-13 17:36) - PLID 59643 check write\edit permission
			if (bNeedCreatePerms && !bCreateAccess && !bCreateAccessWithPass){
				strActionType = "add";
				bAllowMerge = FALSE;
			}
			if  (bNeedEditPerms && !bEditAccess && !bEditAccessWithPass){
				strActionType += (strActionType=="")?"merge":" and merge"; 
				bAllowMerge = FALSE;
			}

			if (bNeedDeletePerms && !bDeleteAccess && !bDeleteAccessWithPass){
				bAllowMerge = FALSE;
				strActionType.Replace(" and ", ", ");
				strActionType += (strActionType=="")?"delete":" and delete"; 
			}
			// if merge is fall the show user which permission is missing
			if (bAllowMerge==FALSE ){
				MessageBox(FormatString("You do not have permission to %s %s.\r\nPlease contact your office manager for assistance." ,strActionType,strMsg) 
						,"NexTech Practice", MB_OK | MB_ICONEXCLAMATION);
				return;
			}

			// See if we need a password
			if (bNeedCreatePerms && !bCreateAccess && bCreateAccessWithPass) {
				bNeedPassword = TRUE;
			}
			if (bNeedDeletePerms && !bDeleteAccess && bDeleteAccessWithPass) {
				bNeedPassword = TRUE;
			}
			// If they need a password, then ask for it now. If they do not need a password, then
			// it must mean they have permission
			if (bNeedPassword) {
				if (!CheckCurrentUserPassword()) {
					return;
				}
			}
		}	
		// show confirmation dialog
		CReconcileMergeMedicationsDlg dlg(this);
		dlg.SetBackColor(m_clrBackReconcilation );
		dlg.m_aReconcileValidationItem.Copy(aReconcileValidationItemArray);
		// (b.spivey, October 30, 2015) PLID 67514 - override button text if we're doing something
		//	 special like a faux wizard. 
		dlg.m_strOkButtonOverrideText = m_strPreviewDlgOkButtonTextOverride;
		switch (m_nReconciliationType)
		{
		case erAllergy:
			{
				// (s.dhole 2013-10-30 13:46) - PLID 59232
				dlg.m_ReconciliationType = CReconcileMergeMedicationsDlg::erMergAllergy;
				if(dlg.DoModal() != IDOK) {
					return;
				}
				else{
					m_btnOkReconcilation.EnableWindow(FALSE);
					CWaitCursor cwait;
					SaveAllergyData(aReconciledItemsArray);
				}
				break;
			}
		case erProblem:
			{
				// (s.dhole 2013-10-30 09:51) - PLID 56935
				dlg.m_ReconciliationType = CReconcileMergeMedicationsDlg::erMergProblem;
				if(dlg.DoModal() != IDOK) {
					return;
				}
				else{
					m_btnOkReconcilation.EnableWindow(FALSE);
					CWaitCursor cwait;
					// (s.dhole 2014-02-04 10:11) - PLID 60201
					if (CanAddOrEditProblems(aReconciledItemsArray))
					{
						SaveProblemData (aReconciledItemsArray);
					}
					else
					{
						m_btnOkReconcilation.EnableWindow(TRUE);
						return;
					}
				}
			break;
			}
		case erMedication:
			{
				dlg.m_ReconciliationType = CReconcileMergeMedicationsDlg::erMergMedication;
				if(dlg.DoModal() != IDOK) {
					return;
				}
				else{
					m_btnOkReconcilation.EnableWindow(FALSE);
					CWaitCursor cwait;
					SaveMedicationData(aReconciledItemsArray);
				}
				break;
			}
		default:
			break;
		}

		//TES 11/14/2013 - PLID 59498 - All of these object types can trigger CDS Rules, so check them now
		CDWordArray arNewCDSInterventions;
		UpdateDecisionRules(GetRemoteData(), m_nPatientID, arNewCDSInterventions);
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

		CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}
// (s.dhole 2013-10-30 13:46) - PLID 59232 Set  reconciliation object to save data (From New Item)
CReconciliationDlg::ReconciledItem CReconciliationDlg::LoadReconcileNewItem(IRowSettingsPtr pRow )
{
	ReconciledItem  aItem;
	BOOL bIsChecked = VarBool(pRow->GetValue(erctcCheckbox));
	aItem.strName = VarString(pRow->GetValue(erctcName));
	aItem.bIsActive = VarBool(pRow->GetValue(erctcIsActive));
	aItem.strCode = VarString(pRow->GetValue(erctcCompText));
	// From new list
	aItem.strDescription = VarString(pRow ->GetValue(erctcDescription));
	aItem.dtLastDate  = VarDateTime (pRow->GetValue(erctcDate), g_cvarNull);
	if (bIsChecked != FALSE ){
		IRowSettingsPtr pRowExist= m_dlReconciliationExist->FindByColumn(erctcCompText, _bstr_t(VarString(pRow->GetValue(erctcCompText), "")), NULL, VARIANT_FALSE);
		if (pRowExist  && VarBool(pRowExist->GetValue(erctcCheckbox))!=FALSE ){
			aItem.nInternalID= VarLong(pRowExist->GetValue(erctcInternalID));
			if ((VarBool(pRow->GetValue(erctcIsActive))!= TRUE) || (VarBool(pRowExist->GetValue(erctcIsActive)) != TRUE))
			{
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				COleDateTime dtNewDate = VarDateTime(pRow->GetValue(erctcDate), dtInvalid);
				COleDateTime dtCurrentDate = VarDateTime(pRowExist->GetValue(erctcDate), dtInvalid);

				//if  date is missing in new item then use current item status
				if (!IsDateTimeValid(dtNewDate)){
					aItem.bIsActive = VarBool (pRowExist->GetValue(erctcIsActive));
				}
				//if new item date is greater than current item date than use new item status
				else if(dtNewDate > dtCurrentDate){
					aItem.bIsActive = VarBool (pRow->GetValue(erctcIsActive));
				}
				//if new item date is less than current item date than use current item status
				else if(dtNewDate <= dtCurrentDate){
					aItem.bIsActive = VarBool (pRowExist->GetValue(erctcIsActive));
				}
			}
			aItem.Action = aItem.eMergeCurItem;
		}
		else{
			aItem.nInternalID  = -1;
			aItem.Action = aItem.eAddItem;
		}
	}
	else{
		aItem.Action = aItem.eExcludeCurItem;
		aItem.nInternalID  = -1;
	}
	return aItem;
}

// (s.dhole 2013-10-30 13:46) - PLID 59232 Set  reconciliation object to save data (From current item)
CReconciliationDlg::ReconciledItem CReconciliationDlg::LoadReconcileCurrentItem(IRowSettingsPtr pRow,BOOL bKeepRecord )
{
	ReconciledItem  aItem;
	aItem.nInternalID = VarLong(pRow->GetValue(erctcInternalID));
	aItem.bIsActive  = VarBool(pRow->GetValue(erctcIsActive));
	aItem.dtLastDate = VarDateTime(pRow->GetValue(erctcDate), g_cvarNull);
	aItem.strCode = VarString(pRow->GetValue(erctcCompText));
	if(bKeepRecord != FALSE ){
		aItem.Action = aItem.eKeepCurItem;
	}
	else if(VarBool(pRow->GetValue(erctcCheckbox)) == FALSE){
		aItem.Action = aItem.eDeleteItem;
	}
	return aItem;
}

// (s.dhole 2013-10-30 13:46) - PLID 59232 Set  reconciliation object to validation data(New item)
CReconcileMergeMedicationsDlg::ReconcileValidationItem CReconciliationDlg::LoadReconcileValidationNewItem(IRowSettingsPtr pRow )
{
	CReconcileMergeMedicationsDlg::ReconcileValidationItem  aItem;
	BOOL bIsChecked = VarBool(pRow->GetValue(erctcCheckbox));
	aItem.strName = VarString(pRow->GetValue(erctcName));
	aItem.bIsActive  =VarBool(pRow->GetValue(erctcIsActive));
	aItem.strDescription  = VarString(pRow->GetValue(erctcDescription));
	aItem.dtLastDate = VarDateTime (pRow->GetValue(erctcDate), g_cvarNull);
	if (bIsChecked != FALSE ){
		IRowSettingsPtr pRowExist= m_dlReconciliationExist->FindByColumn(erctcCompText, _bstr_t(VarString(pRow->GetValue(erctcCompText), "")), NULL, VARIANT_FALSE);

		if (pRowExist  && VarBool(pRowExist->GetValue(erctcCheckbox))!=FALSE ){
			aItem.strCurrentName = VarString(pRowExist->GetValue(erctcName));
			aItem.strCurrentDescription = VarString(pRowExist->GetValue(erctcDescription));
			aItem.bCurrentIsActive = VarBool(pRowExist->GetValue(erctcIsActive));
			aItem.dtCurrentLastDate = VarDateTime(pRowExist->GetValue(erctcDate), g_cvarNull);
			aItem.Action =aItem.eMergeCurItem; 
		}
		else{
			aItem.Action = aItem.eAddItem;
		}
	}
	else{
		aItem.Action = aItem.eExcludeCurItem;
	}
	return aItem;
}

// (s.dhole 2013-10-30 13:46) - PLID 59232 Set  reconciliation object to validation data(Current  item)
CReconcileMergeMedicationsDlg::ReconcileValidationItem CReconciliationDlg::LoadReconcileValidationCurrentItem(IRowSettingsPtr pRow,BOOL bKeepRecord )
{
	CReconcileMergeMedicationsDlg::ReconcileValidationItem  aItem;
	if(bKeepRecord != FALSE ){
		aItem.Action = aItem.eKeepCurItem; 
	}
	else if(VarBool(pRow->GetValue(erctcCheckbox)) == FALSE){
		aItem.Action = aItem.eDeleteItem;
	}
	aItem.bCurrentIsActive = VarBool(pRow->GetValue(erctcIsActive));
	aItem.dtCurrentLastDate= VarDateTime(pRow->GetValue(erctcDate), g_cvarNull);
	aItem.strCurrentName  =  VarString(pRow->GetValue(erctcName));
	aItem.strCurrentDescription  = VarString(pRow->GetValue(erctcDescription), "");
	return aItem;
}

// (s.dhole 2013-10-30 13:46) - PLID 59232 Fill array to show validation and save data
// (s.dhole 2014-01-13 17:40) - PLID 59643 load edit flag 
BOOL CReconciliationDlg::GetDataArray(CReconciledItemsArray &aReconciledItemsArray,
											 CReconcileMergeMedicationsDlg::CReconcileValidationItemArray &aReconcileValidationItemArray,
											 OUT BOOL &bNeedCreatePerms,OUT BOOL &bNeedDeletePerms,OUT BOOL &bNeedEditPerms)
{
	BOOL bReturn = FALSE;
	// Scan new item list
	IRowSettingsPtr pRow =  m_dlReconciliationNew->GetFirstRow();
	while (NULL != pRow) {
		BOOL bIsChecked = VarBool(pRow->GetValue(erctcCheckbox));
		// check prescription list , Is there any medication mark for reconciliation? 
		if (bIsChecked != FALSE){
			//there is prescription row need to add in medication list
			IRowSettingsPtr pRowExist = m_dlReconciliationExist->FindByColumn(erctcCompText, _bstr_t(VarString(pRow->GetValue(erctcCompText), "")), NULL, VARIANT_FALSE);
			if(!pRowExist )
			{
			bNeedCreatePerms =TRUE;
			}
		
		}
		bReturn = TRUE;
		aReconcileValidationItemArray.Add(LoadReconcileValidationNewItem(pRow));
		aReconciledItemsArray.Add(LoadReconcileNewItem(pRow)) ;
		pRow = pRow->GetNextRow();
	}

 	// now scan current medication list
	pRow = m_dlReconciliationExist->GetFirstRow();
	while (NULL != pRow) {
		BOOL bIsChecked = VarBool(pRow->GetValue(erctcCheckbox));
		BOOL bKeepRecord= FALSE;
		BOOL bDeleteRecord= FALSE;
		if ((bIsChecked != FALSE) )
		{// If record is not mark to delete
			IRowSettingsPtr pRowExist = m_dlReconciliationNew->FindByColumn(erctcCompText, _bstr_t(VarString(pRow->GetValue(erctcCompText), "")), NULL, VARIANT_FALSE);
			//check if prescription record is exist and it is not unchecked
			 if (pRowExist &&   VarBool(pRowExist->GetValue(erctcCheckbox))== TRUE){ 
				// do nothing since this  is alredy mark to merge 
			 }
			 else{// this record is not not matching with any existing allergy
				 // will keep this record
				 // associated record from prescription list mark as deleted
				bKeepRecord = TRUE ;
			}
			 // (s.dhole 2014-01-13 17:31) - PLID 59643  check merging
			bNeedEditPerms = TRUE;
		}
		else if (bIsChecked == FALSE){
			// uer mark to remove this from current prescription list
			bNeedDeletePerms = TRUE;
			bDeleteRecord= TRUE;
		}
		if (bKeepRecord != FALSE || bDeleteRecord != FALSE ) {
			bReturn = TRUE;
			aReconcileValidationItemArray.Add (LoadReconcileValidationCurrentItem(pRow, bKeepRecord ));
			aReconciledItemsArray.Add(LoadReconcileCurrentItem(pRow, bKeepRecord )) ;
		}
		pRow = pRow->GetNextRow();
	}
			
	return bReturn;
}

// (s.dhole 2013-10-30 13:46) - PLID 59232 save Allergy 
void CReconciliationDlg::SaveAllergyData(CReconciledItemsArray &aReconciledItemsArray)
{	
	m_mapListIDToRxNormCodes.clear(); 
	NexTech_Accessor::_PracticeMethodsPtr pAPI = GetAPI();
	if(pAPI == NULL) {
		ThrowNxException("Failed to connect to the API.");
	}
	// Save data is true
	BOOL bSaveData = FALSE;
	if (aReconciledItemsArray.GetCount()>0){
		//Save Data
		//Step 1  Check if we do have allergy in lookup table
		//	Select 
		//Step 2  If allergy informatio missing from lookup table then import from FDB=> Lookup table
		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		CString strPersonName;
		// we not sure this dialog may call from patient module
		_RecordsetPtr rsPer = CreateParamRecordset("Select FullName  From PersonT WHERE ID = {INT}", m_nPatientID);
		if(!rsPer->eof) {
			strPersonName = AdoFldString(rsPer, "FullName","");
		}
		// (s.dhole 2013-10-30 14:37) - PLID 56930  Audit
		long nAuditTransID = BeginAuditTransaction();

	// (s.dhole 2013-10-30 14:48) - PLID 56625  Import  all missing allergy  based on RXNorm code 
		 //add new allergies
		Nx::SafeArray<BSTR> saRxNormIDs;
		for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
			ReconciledItem  aReconciledItem = aReconciledItemsArray[i] ;
			if (aReconciledItem.Action==aReconciledItem.eAddItem){
				saRxNormIDs.Add(aReconciledItem.strCode);
			}
		}
		NexTech_Accessor::_FDBImportRxNormAllergiesPtr  pResult = pAPI->ImportAllergiesUsingRxNormCode(GetAPISubkey(), GetAPILoginToken(), saRxNormIDs);
		if(pResult != NULL && pResult->Allergies    != NULL) {
			Nx::SafeArray<IUnknown *> saResults = pResult->Allergies;
			for each(NexTech_Accessor::_FDBImportRxNormAllergyPtr pAllergy in saResults) {
				// allergy missing from allergy
				long nAllergyID = atoi(AsString(pAllergy->AllergyID  ));
				CString strRXCUI = AsString(pAllergy->RXCUI);
				m_mapListIDToRxNormCodes.insert(std::pair<CString,long>(strRXCUI, nAllergyID)); 
			}
		}

		 AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"SET NOCOUNT ON \r\n"
			"DECLARE @PatientID AS INT \r\n"
			"DECLARE @RowID AS INT \r\n"
			// (s.dhole 2013-10-30 14:37) - PLID 56930  Audit
			"DECLARE @NewAllergiesTempTbl TABLE ( ID INT ,AllergyID INT ,TrasType INT ,Discontinued BIT,NewDiscontinued BIT,name NVARCHAR(100)); \r\n"
			"DECLARE @HasNoAllergiesChanged AS BIT \r\n"
			"DECLARE @AllergiesChangedStatus AS BIT \r\n"
			"SET @PatientID ={INT}  \r\n"
			"SET @AllergiesChangedStatus =0 \r\n"
			"SET @HasNoAllergiesChanged =0 \r\n",m_nPatientID);

		for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
			ReconciledItem  aReconciledItem = aReconciledItemsArray[i] ;
			int nID= aReconciledItem.nInternalID; 
			CString strRXCUI= aReconciledItem.strCode; 
			switch(aReconciledItem.Action){
			case aReconciledItem.eAddItem:
				{
					std::map<CString,long>::iterator iter;
					if(m_mapListIDToRxNormCodes.end() != (iter = m_mapListIDToRxNormCodes.find(aReconciledItem.strCode))) 
						{
						long ID = iter->second;
						// (j.jones 2015-12-30 10:33) - PLID 67771 - ensured it is not possible to duplicate allergies
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
							"IF NOT EXISTS (SELECT AllergyID FROM PatientAllergyT WHERE AllergyID = {INT} AND PersonID = @PatientID) \r\n"
							"BEGIN \r\n"
							"	SET @RowID = ISNULL((Select MAX(ID) FROM PatientAllergyT),0) + 1 \r\n"
							"	INSERT INTO PatientAllergyT (ID, AllergyID, PersonID, EnteredDate, LastUpdateDate, Description) \r\n"
							"	OUTPUT INSERTED.ID,INSERTED.AllergyID, 1, 0, 0 INTO @NewAllergiesTempTbl (ID, AllergyID, TrasType, Discontinued, NewDiscontinued) "
							"	SELECT TOP 1 @RowID, AllergyT.ID AS AllergyID, @PatientID AS PersonID, GETDATE(), GETDATE(), {STRING} AS Description \r\n"
							"	FROM AllergyT \r\n"
							"	WHERE AllergyT.ID = {INT} \r\n"
							"END \r\n", ID, aReconciledItem.strDescription, ID);
						bSaveData = TRUE;
					}
				}
				break; 

			case aReconciledItem.eMergeCurItem: 
				//Todo change as date STATUS
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
					"IF EXISTS (SELECT TOP 1 1 FROM PatientAllergyT WHERE ID = {INT}) \r\n"
					"BEGIN \r\n"
					"  INSERT INTO @NewAllergiesTempTbl (ID,AllergyID,TrasType,Discontinued,NewDiscontinued)  \r\n"
					"  SELECT ID,AllergyID,2,Discontinued, {INT} AS NewDiscontinued FROM  PatientAllergyT WHERE ID = {INT}\r\n"
					"  UPDATE PatientAllergyT SET Discontinued = {INT} ,LastUpdateDate=GETDATE(),   \r\n"
					"  Description = CASE WHEN ISNULL(Description,'')='' THEN {STRING} ELSE  Description END,  \r\n"
					" DiscontinuedDate=NULL  WHERE ID = {INT}; \r\n"
					"END \r\n",nID,(aReconciledItem.bIsActive?0:1),nID,(aReconciledItem.bIsActive?0:1), aReconciledItem.strDescription,nID );
					bSaveData =TRUE;
				}
				break;
			case aReconciledItem.eDeleteItem:
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
					"IF EXISTS (SELECT TOP 1 1 FROM PatientAllergyT WHERE ID = {INT}) \r\n"
					"BEGIN \r\n"
					"  INSERT INTO @NewAllergiesTempTbl (ID,AllergyID,TrasType,Discontinued,NewDiscontinued)  \r\n"
					"  SELECT ID,AllergyID,3,0,0 FROM  PatientAllergyT WHERE ID = {INT}\r\n"
					" DELETE FROM AllergyIngredientT WHERE PatientAllergyID  = {INT};\r\n"
					" DELETE FROM PatientAllergyT WHERE ID = {INT}; \r\n"
					"END \r\n",nID,nID,nID,nID);
					bSaveData =TRUE;
				}
				break; 
			case aReconciledItem.eExcludeCurItem: 
				//if (nID>0)
				{
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						" INSERT INTO  @NewAllergiesTempTbl(ID,AllergyID,TrasType,name  ) \r\n" 
						" VALUES(-1,-1,3,{STRING})\r\n" ,aReconciledItem.strName);
					/*
					do nothing
					*/
					bSaveData =TRUE;
				}
				break; 
			case aReconciledItem.eKeepCurItem : 
				// do nothing
				// this is unchaged record , but as per document will say "Merge"
				// Will Just update last update date
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
					"IF EXISTS (SELECT TOP 1 1 FROM PatientAllergyT WHERE ID = {INT}) \r\n"
					"BEGIN \r\n"
					"  INSERT INTO @NewAllergiesTempTbl (ID,AllergyID,TrasType,Discontinued,NewDiscontinued)  \r\n"
					"  SELECT ID,AllergyID,1,Discontinued,Discontinued FROM  PatientAllergyT WHERE ID = {INT}\r\n"
					"  UPDATE PatientAllergyT SET LastUpdateDate=GETDATE()  WHERE ID = {INT}; \r\n"
					"END \r\n"
					,nID,nID,nID);
					bSaveData =TRUE;
				}

				break; 
			}
		}
		if (bSaveData != FALSE){ 
			CString strShowResult;
			//Check if any active record in patitn allergy table
			AddParamStatementToSqlBatch(strSqlBatch, aryParams," IF EXISTS (select Top 1 1 from PatientAllergyT WHERE Discontinued =0 AND PersonID =@PatientID) \r\n"
				 " BEGIN \r\n"
				 " SET @HasNoAllergiesChanged = 1 \r\n"
				 " END   \r\n" 
			//Check if any patitn flag need to set, Ad set output flag for audit
			"IF NOT EXISTS (select Top 1 1 from PatientsT WHERE HasNoAllergies  != @HasNoAllergiesChanged AND PersonID = @PatientID )   \r\n"
			"BEGIN  \r\n"
			"  SELECT @AllergiesChangedStatus= CASE WHEN ISNULL(HasNoAllergies ,0) =0 THEN 1 ELSE 0 END    FROM PatientsT   WHERE  PersonID = @PatientID  \r\n"
			"  UPDATE PatientsT SET HasNoAllergies = @AllergiesChangedStatus WHERE PersonID = @PatientID \r\n"
			" 	SET @HasNoAllergiesChanged = 1 \r\n"
			" END \r\n"
			" ELSE\r\n"
			"BEGIN  \r\n"
			" SET @HasNoAllergiesChanged = 0 \r\n"
			" END \r\n"
			"SET NOCOUNT OFF \r\n" 
			"SELECT @HasNoAllergiesChanged AS HasNoAllergiesChanged,@AllergiesChangedStatus AS AllergiesChangedStatus;  \r\n" 
			// (s.dhole 2013-10-30 14:37) - PLID 56930  Audit
			"SELECT TempQ.ID,TempQ.AllergyID,TempQ.TrasType,TempQ.Discontinued, EmrDataT.DATA AS AllergyName ,TempQ.name ,TempQ.NewDiscontinued FROM @NewAllergiesTempTbl  AS TempQ \r\n" 
			"LEFT JOIN AllergyT ON TempQ.AllergyID =AllergyT.ID \r\n" 
			"LEFT JOIN EMRDataT ON EMRDataT.ID = AllergyT.EmrDataID \r\n" 
			);
			_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);
			if(!prs->eof) {
				if ( AdoFldBool(prs,"HasNoAllergiesChanged",FALSE)!=FALSE)
				{
					CString strOldValue, strNewValue;
					if (AdoFldBool(prs,"AllergiesChangedStatus",FALSE)!=FALSE ){
						strOldValue = "'No Known Allergies' Status Cleared";
						strNewValue = "'No Known Allergies' Status Selected";
					}
					else{
						strOldValue = "'No Known Allergies' Status Selected";
						strNewValue = "'No Known Allergies' Status Cleared";
					}
					AuditEvent(m_nPatientID , strPersonName, nAuditTransID, aeiPatientHasNoAllergiesStatus, m_nPatientID, strOldValue, strNewValue, aepMedium, aetChanged);
				}
				prs = prs->NextRecordset(NULL);
				// (s.dhole 2013-10-30 14:37) - PLID 56930  Audit
				while(!prs->eof) {
					long nID =  AdoFldLong(prs,"ID",-1);
					CString strAllergyName= AdoFldString (prs,"AllergyName","");
					if (strAllergyName.IsEmpty()){
						strAllergyName=AdoFldString (prs,"name","");
					}
					// Add
					switch (AdoFldLong(prs,"TrasType",-1)) 
					{
						case 1: //Add
							{
								if (nID >0){
									AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientAllergyReconcileAdded, nID, "", strAllergyName , aepMedium, aetCreated);
								}
								strShowResult += strAllergyName; 
								strShowResult += " was MERGED\r\n";
							}
							break; 
						case 2: //UPDATE
							//UPDATE
							{
								strShowResult += strAllergyName; 
								strShowResult += " was CONSOLIDATED and MERGED\r\n";
								if (nID >0){
									if ( AdoFldBool(prs,"Discontinued")!=AdoFldBool(prs,"NewDiscontinued")){
										
										if ( AdoFldBool(prs,"Discontinued")==FALSE){
											AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientAllergyReconcileMerged, nID,  "<Discontinued>",FormatString("%s: Active ",  strAllergyName) , aepMedium, aetChanged);
										}
										else{
											AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientAllergyReconcileMerged, nID,  FormatString("%s: Active ",  strAllergyName),"<Discontinued>" , aepMedium, aetChanged);
										}
									}
								}
							}
							break; 
						case 3: //Delete
							{
							//DELETE
							if (nID >0){
								AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientAllergyReconcileRemoved, nID, strAllergyName, "<Deleted>", aepMedium, aetDeleted);
							}
							strShowResult += strAllergyName; 
							strShowResult += " was DELETED\r\n";
							}
							break; 
						default:
							break; 
					}
					prs->MoveNext();		
				}
			}
			CommitAuditTransaction(nAuditTransID);
			CClient::RefreshTable(NetUtils::PatientAllergyT,  m_nPatientID);
			m_strResultText =FormatString("Allergy List Reconciliation\r\n\r\n%s" , strShowResult);
		}
		else{
			//nothing to save
			RollbackAuditTransaction(nAuditTransID);
		}
	}

}



// (s.dhole 2013-10-30 13:46) - PLID 56933Add new  Problem code (Snomed and US Extension to SNOMED CT)
void CReconciliationDlg::ImportNewProblemCodes(CReconciledItemsArray &aReconciledItemsArray){	
	//TODO  Move this code to API
	// Save data is true
	CString strSqlBatch;
	CNxParamSqlArray aryParams;
	if (aReconciledItemsArray.GetCount()>0){
		BOOL bRecordExist = FALSE;
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DECLARE @TempCodesT TABLE (ID INT IDENTITY(1,1) NOT NULL, Vocab NVARCHAR(48),Code NVARCHAR(100) ,Name NVARCHAR(510),ConceptID NVARCHAR(64)) \r\n");
		for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
			ReconciledItem  aReconciledItem = aReconciledItemsArray[i] ;
			if (aReconciledItem.Action==aReconciledItem.eAddItem){
				CString  strVocab = "";
				// (a.walling 2014-10-16 16:59) - PLID 62911 - Update for SNOMEDCT_US - combine SNOMEDCT with SCTUSX
				if (-1 != aReconciledItem.strName.Find("SNOMED CT")) {
					strVocab = "SNOMEDCT_US";
				}
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO @TempCodesT (Vocab ,Code ,Name ,ConceptID ) \r\n"
					" VALUES({STRING},{STRING},{STRING},''); "
					,strVocab,aReconciledItem.strCode,	aReconciledItem.strDescription,aReconciledItem.strName);
				bRecordExist = TRUE;
			}
		}
		// Inser missing codes
		if (bRecordExist ){
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			" INSERT INTO CodesT(  Vocab ,Code ,Name ,ConceptID , DESCRIPTION ) \r\n"
			" SELECT Vocab ,Code ,Name ,ConceptID ,'' AS DESCRIPTION  FROM @TempCodesT AS TempCodesQ  \r\n"
			" WHERE TempCodesQ.Code NOT IN (  \r\n"
			" SELECT CodesT.Code FROM CodesT  INNER JOIN  @TempCodesT AS TempCodesQ  \r\n"
			" ON CodesT.Code = TempCodesQ.Code AND CodesT.Vocab = TempCodesQ.Vocab \r\n"
			" ) \r\n");

			ExecuteParamSqlBatch(  GetRemoteData(), strSqlBatch, aryParams);
		}
	}
}




// (s.dhole 2013-10-30 09:51) - PLID 56935 Save Problem reconcile list
void CReconciliationDlg::SaveProblemData(CReconciliationDlg::CReconciledItemsArray &aReconciledItemsArray)
{
	// Save data is true
	long nRegardintType =  9;//unsigned
	long nStatus = 1;//Active
	BOOL bSaveData = FALSE;
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	if (aReconciledItemsArray.GetCount()>0){
		//Save Data
		// (s.dhole 2013-10-30 09:53) - PLID 56933 import all new codes
		ImportNewProblemCodes(aReconciledItemsArray);
		//Step 1  Check if we do have allergy in lookup table
		//	Select 
		//Step 2  If allergy informatio missing from lookup table then import from FDB=> Lookup table
		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		CString strPatientName;
			// we not sure this dialog may call from patient module
		_RecordsetPtr rsPer = CreateParamRecordset("Select FullName  From PersonT WHERE ID = {INT}", m_nPatientID);
		if(!rsPer->eof) {
			strPatientName = AdoFldString(rsPer, "FullName","");
		}
		long nAuditTransID = BeginAuditTransaction();
		// (s.dhole 2013-12-18 16:13) - PLID 59229 Remove onset date 
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
		"SET NOCOUNT ON \r\n"
		"DECLARE @PatientID AS INT \r\n"
		"DECLARE @NewProblemID AS INT \r\n"
		"DECLARE @NewCodeID AS INT \r\n"
		"DECLARE @NewProblemIDTbl TABLE (ProblemID int,CodeID INT,TransType INT,  \r\n"
		" StatusID INT , OldStatusID INT, Name NVARCHAR(MAX), OldName NVARCHAR(MAX)) \r\n"
		"DECLARE @DeleteLinkProblemTbl TABLE (ProblemID int, Description NVARCHAR(MAX),NewVal NVARCHAR(MAX)) \r\n"
		"SET @PatientID ={INT}  \r\n",m_nPatientID);
		for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
			ReconciledItem  aReconciledItem = aReconciledItemsArray[i] ;
			int nID= aReconciledItem.nInternalID; 
			CString strRXCUI= aReconciledItem.strCode; 
			switch(aReconciledItem.Action){
			case aReconciledItem.eAddItem: 
				AddProblemListData( aReconciledItem,strPatientName,nAuditTransID,strSqlBatch,aryParams);
				bSaveData =TRUE;
				break; 
			case aReconciledItem.eDeleteItem:
				if (nID>0){
					// (s.dhole 2013-10-30 11:50) - PLID 59230
					DeleteProblemListData( aReconciledItem,strPatientName,nAuditTransID,strSqlBatch,aryParams);
					bSaveData =TRUE;
				}
				break; 
			case aReconciledItem.eMergeCurItem: 
				//Todo Chaneg as date STATUS
				if (nID>0){
					// (s.dhole 2013-10-30 11:44) - PLID 59229
					UpdateProblemListData ( aReconciledItem,strPatientName,nAuditTransID,strSqlBatch,aryParams);
					bSaveData =TRUE;
				}
				break;
			case aReconciledItem.eExcludeCurItem: 
				{
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
					 "INSERT INTO @NewProblemIDTbl(ProblemID,CodeID,TransType,Name ) \r\n"
					 " VALUES( -1 ,-1,3,{STRING})  \r\n" ,aReconciledItem.strDescription);  
				}
				break; 
			case aReconciledItem.eKeepCurItem : 
				// do nothing
				// this is unchaged record , but as per document will say "Merge"
				// (s.dhole 2014-02-04 10:12) - PLID 60201 make it as keep record(4)
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
					 "INSERT INTO @NewProblemIDTbl(ProblemID,CodeID,TransType ) \r\n"
					 " SELECT ID ,CodeID,4 FROM EMRProblemsT WHERE ID ={INT} \r\n"
					,nID);
				}
				break; 
			}
		}
		if (bSaveData != FALSE){
			CString  strShowResult;
			// (s.dhole 2013-10-30 11:40) - PLID 56935 Load Saved Records
			// (s.dhole 2013-12-18 16:13) - PLID 59229 Removed onset date
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,"SET NOCOUNT OFF \r\n "
				" SELECT TempTBL.ProblemID,TempTBL.CodeID,CodesT.Code,CodesT.Name,TransType,  \r\n"
		  " TempTBL.name AS ProblemName ,TempTBL.Oldname AS ProblemNameOld ,  \r\n" 
		  " EMRProblemStatusT.Name AS NewStatus ,EMRProblemStatusQ.Name AS OLDStatus \r\n"
		  " FROM @NewProblemIDTbl AS TempTBL LEFT JOIN CodesT  ON TempTBL.CodeID = CodesT.ID  \r\n"
		  " LEFT JOIN EMRProblemStatusT ON EMRProblemStatusT.ID=TempTBL.StatusID \r\n"
		  " LEFT JOIN EMRProblemStatusT AS EMRProblemStatusQ  ON EMRProblemStatusQ.ID=TempTBL.OldStatusID;  \r\n"
		  " SELECT ProblemID , Description AS ProblemDescription,NewVal FROM @DeleteLinkProblemTbl;");
			_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);
			while(!prs->eof) {
				long nProblemID =  AdoFldLong(prs,"ProblemID",-1);
				
				CString  strProblemName =  AdoFldString (prs,"Name","");
				if (strProblemName.IsEmpty()){
					strProblemName =AdoFldString (prs,"ProblemName","");
				}
				//AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemChronicity, nProblemID , "",AdoFldString(prs,"Problemdesc",""),aepMedium,aetCreated);
				 switch (AdoFldLong(prs,"TransType",-1)) 
					{
					case 1: //Add
						{	
							if (nProblemID >0){
								// Audit to create Problem
								AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemDescReconcile, nProblemID , "", strProblemName, aepMedium,aetCreated);
								// Audit to create Problem
								AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemStatusReconcile, nProblemID , "","Active",aepMedium,aetCreated);
								CString strNewValue;
								CString strOwnerType = GetProblemTypeDescription((EMRProblemRegardingTypes)nRegardintType );
								strNewValue.Format("Linked with %s: %s", strOwnerType, strPatientName);
								AuditEvent(m_nPatientID, strPatientName, nAuditTransID, aeiEMNProblemLinkCreatedReconcile, -1, strProblemName, strNewValue, aepHigh, aetChanged);
								AuditEvent(m_nPatientID, strPatientName, nAuditTransID, aeiEMNProblemSNOMEDCode, nProblemID, "", strProblemName, aepMedium, aetCreated); 
							}
							strShowResult += strProblemName; 
							strShowResult += " was MERGED\r\n";
						}
						break; 
					case 2: //UPDATE
						//UPDATE
						{
							// (s.dhole 2013-12-18 16:13) - PLID 59229 Removed onset date
							if (nProblemID >0){

								CString  strOldStatus=  AdoFldString (prs,"OLDStatus","");
								CString  strStatus=  AdoFldString (prs,"NewStatus","");
								if (strOldStatus!=strStatus){
									AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemStatusReconcile, nProblemID , strOldStatus,strStatus,aepMedium,aetCreated);
								}
								CString  strName=  AdoFldString (prs,"ProblemName","");
								CString  strOldName=  AdoFldString (prs,"ProblemNameOld","");
								if(strName!=strOldName)
								{
									AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemDesc,nProblemID,strOldName,strName,aepMedium,aetChanged);
								}
							
							}
							strShowResult += strProblemName; 
							strShowResult += " was CONSOLIDATED and MERGED\r\n";
						}
						break; 
					case 3: //Delete
						{
							//DELETE
							if (nProblemID >0){
								AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemLinkDeleted,nProblemID ,"","<Deleted>",aepHigh,aetDeleted);
							}
							strShowResult += strProblemName; 
							strShowResult += " was REMOVED\r\n";
						}
						break; 
					case 4: //Keep // (s.dhole 2014-02-04 10:13) - PLID 60201
						{
							strShowResult += strProblemName; 
							strShowResult += " was MERGED\r\n";
						}
						break; 
					default:
						break; 
				 }
				
				prs->MoveNext();
			}
			prs = prs->NextRecordset(NULL);
			while(!prs->eof) {
				long nProblemID =  AdoFldLong(prs,"ProblemID",-1);
				CString  strNewValue=  AdoFldString (prs,"NewVal","");
				CString  strOldValue =  AdoFldString (prs,"ProblemDescription","");
				AuditEvent(m_nPatientID, strPatientName,nAuditTransID ,aeiEMNProblemLinkDeleted,nProblemID,strOldValue,strNewValue, aepHigh, aetChanged);
				prs->MoveNext();
			}
			
			CommitAuditTransaction(nAuditTransID);
			CClient::RefreshTable(NetUtils::EMRProblemsT, m_nPatientID);
			m_strResultText = FormatString("Problem List Reconciliation\r\n\r\n%s",  strShowResult);
		}
		else{
			//nothing to save
			RollbackAuditTransaction(nAuditTransID);

		}
	}
}


// (s.dhole 2013-10-30 11:27) - PLID 56935 Add Problem
// (a.walling 2014-10-16 16:59) - PLID 62911 - Update for SNOMEDCT_US
void CReconciliationDlg::AddProblemListData(CReconciliationDlg::ReconciledItem &aReconciledItem,CString strPatientName,
											long nAuditTransID, CString &strSqlBatch,CNxParamSqlArray &aryParams){		
	long nRegardintType =  9;//unsigned
	long nStatus = 1;//Active
	// Insert dato to problem table and Set temp table 	 
	// there are changces that it may have same code n number of time, we import only one code
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @NewProblemID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRProblemsT WITH(UPDLOCK, HOLDLOCK)) \r\n");
	// (s.dhole 2013-12-18 16:13) - PLID 59229 Removed onset date
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
	" INSERT INTO EMRProblemsT (ID, PatientID, Description, StatusID,  CodeID)  \r\n"
	" OUTPUT inserted.ID ,inserted.CodeID ,1,inserted.StatusID ,inserted.StatusID \r\n"
	" INTO @NewProblemIDTbl(ProblemID,CodeID,TransType,StatusID,OldStatusID )  \r\n"
	" SELECT TOP 1 @NewProblemID, @PatientID AS PatientID, {STRING} AS Description,  \r\n"
	" {INT} AS StatusID,   ID AS CodeID \r\n"
	" FROM CodesT WHERE Vocab = 'SNOMEDCT_US'  AND Code={STRING} \r\n",
	aReconciledItem.strDescription , nStatus, aReconciledItem.strCode   );
	
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
		"INSERT INTO EMRProblemLinkT (EMRProblemID, EMRRegardingType, EMRRegardingID)  \r\n"
		"VALUES (@NewProblemID, {VT_I4}, {VT_I4}) \r\n",
		_variant_t(nRegardintType, VT_I4),_variant_t(m_nPatientID, VT_I4));
		
	AddParamStatementToSqlBatch(strSqlBatch, aryParams,
		"INSERT INTO EMRProblemHistoryT (ProblemID, Description, StatusID, UserID )  \r\n"
		"VALUES (@NewProblemID, {STRING}, {INT}, {INT}) \r\n",
		 aReconciledItem.strDescription , nStatus, GetCurrentUserID() );


}

// (s.dhole 2013-10-30 11:44) - PLID 59229
// (a.walling 2014-10-16 16:59) - PLID 62911 - Update for SNOMEDCT_US
void CReconciliationDlg::UpdateProblemListData(ReconciledItem &aReconciledItem,const CString strPatientName,
											   long nAuditTransID, CString &strSqlBatch,CNxParamSqlArray &aryParams)
{
	long nRegardintType =  9;//unsigned
	long nStatus = (aReconciledItem.bIsActive?1:2);//Active=1 ; closed =2
	// Load old problem values
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	// (s.tullis 2015-03-11 10:48) - PLID 64723 - Added DoNotshowonCCDA
	// (r.gonet 2015-03-09 19:58) - PLID 65008 - Added DoNotShowOnProblemPrompt.
	_RecordsetPtr prs = CreateParamRecordset("SELECT StatusID, Description, OnsetDate, ChronicityID, DiagCodeID, DiagCodeID_ICD10, CodeID, DoNotShowOnCCDA, \r\n"
		"	DoNotShowOnProblemPrompt \r\n"
		"FROM EMRProblemsT  \r\n"
		"WHERE EMRProblemsT.ID = {INT} \r\n", aReconciledItem.nInternalID);
	if (!prs->eof) {
		
		CString strNewStatus;
		COleDateTime dtInvalid;
		CString strCurrentDesc = AdoFldString(prs, "Description");
		long nCurrentStatusID = AdoFldLong(prs, "StatusID");
		dtInvalid.SetStatus(COleDateTime::invalid);
		long nCurrentChronicityID = AdoFldLong(prs, "ChronicityID", -1);
		// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
		long nCurrentDiagCodeID_ICD9 = AdoFldLong(prs, "DiagCodeID", -1);
		long nCurrentDiagCodeID_ICD10 = AdoFldLong(prs, "DiagCodeID_ICD10", -1);
		// (s.tullis 2015-03-11 10:48) - PLID 64723 - Added DoNotshowonCCDA
		BOOL bDoNotShowOnCCDA = AdoFldBool(prs, "DoNotShowOnCCDA",FALSE);
		// (r.gonet 2015-03-09 19:58) - PLID 65008 - Added DoNotShowOnProblemPrompt.
		BOOL bDoNotShowOnProblemPrompt = AdoFldBool(prs, "DoNotShowOnProblemPrompt", FALSE);
		
		// if new status is active and old status is not complete then keep existing status
		if (nStatus==1 && nCurrentStatusID !=2  ){
			nStatus =nCurrentStatusID ;
		}
		
		// (s.dhole 2013-12-18 16:13) - PLID 59229 Removed onset date
		if (ReturnsRecordsParam("SELECT TOP 1 ID FROM EMRProblemsT WHERE ID = {INT} AND Deleted = 0", aReconciledItem.nInternalID )) {
			_variant_t varChronicID = nCurrentChronicityID == -1 ? g_cvarNull : _variant_t(nCurrentChronicityID, VT_I4);
			_variant_t varDiagCode9 = nCurrentDiagCodeID_ICD9 == -1 ? g_cvarNull : _variant_t(nCurrentDiagCodeID_ICD9, VT_I4);
			_variant_t varDiagCode10 = nCurrentDiagCodeID_ICD10 == -1 ? g_cvarNull : _variant_t(nCurrentDiagCodeID_ICD10, VT_I4);
			// (s.dhole 2013-12-18 16:13) - PLID 59229 Remove onset date
			// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
			// (s.tullis 2015-03-11 10:48) - PLID 64723 - Added DoNotshowonCCDA
			// (r.gonet 2015-03-09 19:58) - PLID 65008 - Added DoNotShowOnProblemPrompt.
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"SELECT @NewCodeID =ID from CodesT  WHERE Vocab = 'SNOMEDCT_US'  AND Code={STRING}   \r\n"
				 "INSERT INTO @NewProblemIDTbl(ProblemID,CodeID,TransType,StatusID,OldStatusID, OldName,Name) \r\n"
				 "SELECT ID ,CodeID,2,StatusID, {INT} AS OldStatusID, {STRING} AS OldName,{STRING} AS Name \r\n"
				 "FROM EMRProblemsT WHERE ID ={INT} \r\n"
				"UPDATE EMRProblemsT  \r\n"
				"SET StatusID = {INT},  ModifiedDate = GETDATE(),  \r\n"
				"ChronicityID = {VT_I4}, Description = {STRING}, \r\n"
				"DiagCodeID = {VT_I4}, DiagCodeID_ICD10 = {VT_I4}, CodeID = @NewCodeID ,  \r\n"
				"DoNotShowOnCCDA = {BIT}, "
				"DoNotShowOnProblemPrompt = {BIT} "
				"WHERE ID = {INT};  \r\n",
				aReconciledItem.strCode,nStatus, strCurrentDesc,((strCurrentDesc.IsEmpty())?aReconciledItem.strDescription: strCurrentDesc  ),aReconciledItem.nInternalID , 
				nStatus,varChronicID, ((strCurrentDesc.IsEmpty())?aReconciledItem.strDescription: strCurrentDesc  ), varDiagCode9, varDiagCode10, bDoNotShowOnCCDA,
				bDoNotShowOnProblemPrompt,
				aReconciledItem.nInternalID );
				// (s.dhole 2013-12-18 16:13) - PLID 59229 Will allways add to history since we change modify date
			//if (strCurrentDesc != aReconciledItem.strDescription || nCurrentStatusID != nStatus ){
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
					"INSERT INTO EMRProblemHistoryT (ProblemID,Description,  StatusID, UserID, ChronicityID, DiagCodeID, DiagCodeID_ICD10)  \r\n"
					"VALUES ({INT}, {STRING}, {INT}, {INT}, {VT_I4}, {VT_I4}, {VT_I4}) \r\n",
					aReconciledItem.nInternalID, (strCurrentDesc.IsEmpty()?aReconciledItem.strDescription: strCurrentDesc  ),
					nStatus, GetCurrentUserID(), 
					varChronicID, varDiagCode9, varDiagCode10);
			//}
		}
	}
}



// (s.dhole 2013-10-30 11:50) - PLID 59230 Delete Problems
void CReconciliationDlg::DeleteProblemListData(CReconciliationDlg::ReconciledItem &aReconciledItem,CString strPatientName,
						   long nAuditTransID, CString &strSqlBatch,CNxParamSqlArray &aryParams)
{
	// Check if proble is exist
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	_RecordsetPtr prs = CreateParamRecordset("SELECT StatusID, Description, OnsetDate, ChronicityID, DiagCodeID, CodeID  \r\n"
		"FROM EMRProblemsT  \r\n"
		"WHERE EMRProblemsT.ID = {INT} \r\n", aReconciledItem.nInternalID);
	if (!prs->eof) {
		AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				" INSERT INTO @NewProblemIDTbl(ProblemID,CodeID,TransType ) \r\n"
				 " SELECT ID ,CodeID,3 FROM EMRProblemsT WHERE ID ={INT} \r\n"
					"UPDATE EMRProblemsT  \r\n"
					" SET Deleted = 1, DeletedBy = {STRING},  \r\n"
					" DeletedDate = GETDATE() WHERE ID = {INT}; \r\n"
					,aReconciledItem.nInternalID , GetCurrentUserName(), aReconciledItem.nInternalID );				
					
		AuditEvent(m_nPatientID, strPatientName,nAuditTransID,aeiEMNProblemLinkDeleted,aReconciledItem.nInternalID ,"","<Deleted>",aepHigh,aetDeleted);
		_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM " + GetEmrProblemListFromClause() + " WHERE ProblemsQ.ID = {INT} AND ProblemsQ.Deleted = 0",
				aReconciledItem.nInternalID); 
		while (!prs->eof) {
			
			CString strNewValue;
			EMRProblemRegardingTypes nType= (EMRProblemRegardingTypes)AdoFldLong(prs, "EMRRegardingType");
			//// (z.manning 2009-05-27 09:37) - PLID 34297 - Moved this code to a utility function
			CString strOwnerType = GetProblemTypeDescription(nType);

			CString strName;
			if (nType ==eprtUnassigned){
				strName = strPatientName;
			}
			strNewValue.Format("Unlinked from %s: %s", strOwnerType,strName );
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EmrProblemLinkT WHERE ID ={INT}",  
				AdoFldLong(prs, "EMRProblemLinkID"));
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
			" INSERT INTO @DeleteLinkProblemTbl(ProblemID,Description,NewVal ) \r\n"
				" VALUES({INT},{STRING},{STRING}) \r\n"
				,aReconciledItem.nInternalID,aReconciledItem.strDescription ,strNewValue);
			
			prs->MoveNext();
		}
	}
}




// (s.dhole 2013-10-30 15:21) - PLID 57861 Save Data
void CReconciliationDlg::SaveMedicationData(CReconciledItemsArray &aReconciledItemsArray)
{
	BOOL bSaveData = FALSE;
	m_mapListIDToRxNormCodes.clear(); 
	NexTech_Accessor::_PracticeMethodsPtr pAPI = GetAPI();
	if(pAPI == NULL) {
		ThrowNxException("Failed to connect to the API.");
	}
	if (aReconciledItemsArray.GetCount()>0){
		//Save Data
		
		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		CString strPersonName;
		// we not sure this dialog may call from patient module
		_RecordsetPtr rsPer = CreateParamRecordset("Select FullName  From PersonT WHERE ID = {INT}", m_nPatientID);
		if(!rsPer->eof) {
			strPersonName = AdoFldString(rsPer, "FullName","");
		}
		long nAuditTransID = BeginAuditTransaction();
		
		// Import new Medication from  CCDA
		//Step 1  Check if we do have allergy in lookup table
		//	Select 
		//Step 2  If allergy informatio missing from lookup table then import from FDB=> Lookup table
		Nx::SafeArray<BSTR> saRxNormIDs;
		for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
			ReconciledItem  aReconciledItem = aReconciledItemsArray[i] ;
			if (aReconciledItem.Action==aReconciledItem.eAddItem){
				saRxNormIDs.Add(aReconciledItem.strCode);
			}
		}
		NexTech_Accessor::_FDBRxNormCodeMedicationsPtr  pResult = pAPI->ImportMedicationUsingRxNormCode(GetAPISubkey(), GetAPILoginToken(), saRxNormIDs);
		if(pResult != NULL && pResult->Medications  != NULL) {
			Nx::SafeArray<IUnknown *> saResults = pResult->Medications;
			for each(NexTech_Accessor::_FDBRxNormCodeMedicationPtr  pDrug in saResults) {
				// Medication missing from druglist
				long nDruglistID = atoi(AsString(pDrug->DrugListID ));
				CString strRXCUI = AsString(pDrug->RXCUI);
				m_mapListIDToRxNormCodes.insert(std::pair<CString,long>(strRXCUI , nDruglistID)); 
			}
		}

		
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"SET NOCOUNT ON \r\n"
			"DECLARE @PatientID AS INT \r\n"
			"DECLARE @NewMedicationIDTbl TABLE ( ID INT,MedicationID INT,TransType INT, Name NVARCHAR(100), Discontinued BIT, NewDiscontinued BIT) \r\n"
			"DECLARE @HasNoMedChanged AS BIT \r\n"
			"DECLARE @MedChangedStatus AS BIT \r\n"
			"SET @PatientID ={INT}  \r\n",m_nPatientID);

		for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
			ReconciledItem  aReconciledItem = aReconciledItemsArray[i] ;
			int nID= aReconciledItem.nInternalID; 
			CString strRxNorm= aReconciledItem.strCode; 
			switch(aReconciledItem.Action){
			case aReconciledItem.eAddItem: 
				{
				std::map<CString,long>::iterator iter;
				if(m_mapListIDToRxNormCodes.end() != (iter = m_mapListIDToRxNormCodes.find(strRxNorm))) 
					{
					long ID = iter->second;

					// (j.jones 2015-12-30 10:33) - PLID 67771 - ensured it is not possible to duplicate medications
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
						"IF NOT EXISTS (SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID = {INT} AND PatientID = @PatientID) \r\n"
						"BEGIN \r\n"
						"	INSERT INTO CurrentPatientMedsT (PatientID, MedicationID, InputByUserID, Sig, LastUpdateDate) \r\n"
						"	OUTPUT INSERTED.ID, INSERTED.MedicationID, 1, 1, 1 INTO @NewMedicationIDTbl (ID, MedicationID, TransType, Discontinued, NewDiscontinued) \r\n"
						"	SELECT TOP 1 @PatientID AS PatientID, DrugList.ID AS MedicationID, {INT} AS InputByUserID, {STRING} AS Sig, GETDATE() AS LastUpdateDate \r\n"
						"	FROM DrugList  \r\n"
						"	WHERE DrugList.ID = {INT}   \r\n"
						"END \r\n"
						, ID, GetCurrentUserID(), aReconciledItem.strDescription, ID);
					bSaveData =TRUE;
					} 
				}
				break; 

			case aReconciledItem.eMergeCurItem: 
				//Todo Chaneg as date STATUS
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						" IF Exists(SELECT TOP 1 1 FROM CurrentPatientMedsT WHERE ID = {INT}) \r\n" 
						" BEGIN \r\n" 
						" INSERT INTO  @NewMedicationIDTbl(ID,MedicationID,TransType,Discontinued,NewDiscontinued ) \r\n" 
						" SELECT ID,MedicationID,2, Discontinued,{INT} AS NewDiscontinued FROM CurrentPatientMedsT WHERE ID = {INT}\r\n" 
						" UPDATE CurrentPatientMedsT SET Discontinued = {INT}, DiscontinuedDate = NULL, "
						"  Sig = {STRING} , LastUpdateDate=GETDATE() "
						" WHERE ID = {INT} \r\n"
						" END \r\n" 
						,nID ,(aReconciledItem.bIsActive?0:1),nID ,(aReconciledItem.bIsActive?0:1), aReconciledItem.strDescription, nID);
					
					bSaveData =TRUE;
				}
				break;
			case aReconciledItem.eDeleteItem:
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						" IF Exists(SELECT TOP 1 1 FROM CurrentPatientMedsT WHERE ID = {INT}) \r\n" 
						" BEGIN \r\n" 
						" INSERT INTO  @NewMedicationIDTbl(ID,MedicationID,TransType,Discontinued,NewDiscontinued) \r\n" 
						" SELECT ID,MedicationID,3, Discontinued,Discontinued AS NewDiscontinued FROM CurrentPatientMedsT WHERE ID = {INT}\r\n" 
						" DELETE FROM CurrentPatientMedsT WHERE ID = {INT} \r\n" 
						" END \r\n" 
						, nID, nID, nID);
					
					bSaveData =TRUE;
				}
				break; 
			case aReconciledItem.eExcludeCurItem: 
				//if (nID>0)
				{
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						" INSERT INTO  @NewMedicationIDTbl(ID,MedicationID,TransType,name  ) \r\n" 
						" VALUES(-1,-1,3,{STRING})\r\n" ,aReconciledItem.strName);
				}
				break; 
			case aReconciledItem.eKeepCurItem : 
				// do nothing
				// this is unchaged record , but as per document will say "Merge"
				if (nID>0){
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						" IF Exists(SELECT TOP 1 1 FROM CurrentPatientMedsT WHERE ID = {INT}) \r\n" 
						" BEGIN \r\n" 
						" INSERT INTO  @NewMedicationIDTbl(ID,MedicationID,TransType,Discontinued) \r\n" 
						" SELECT ID,MedicationID,1, Discontinued FROM CurrentPatientMedsT WHERE ID = {INT} \r\n" 
						" UPDATE CurrentPatientMedsT SET LastUpdateDate=GETDATE() WHERE ID = {INT} \r\n" 
						" END \r\n" 
						, nID, nID, nID);
					bSaveData =TRUE;
				}
				break; 
			}
		}
		if (bSaveData != FALSE){ 
			CString strShowResult;
			//Check if any active record in patitn allergy table
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"SET NOCOUNT ON \r\n" 
			" IF EXISTS (select Top 1 1 from CurrentPatientMedsT WHERE Discontinued =0 AND PatientID =@PatientID) \r\n"
				 " BEGIN \r\n"
				 " SET @HasNoMedChanged = 1 \r\n"
				 " END   \r\n" 
			//Check if any patient flag need to set, Ad set output flag for audit
			"IF NOT EXISTS (select Top 1 1 from PatientsT WHERE HasNoMeds  != @HasNoMedChanged AND PersonID = @PatientID )   \r\n"
			"BEGIN  \r\n"
			"  SELECT @MedChangedStatus= CASE WHEN ISNULL(HasNoMeds ,0) =0 THEN 1 ELSE 0 END    FROM PatientsT   WHERE  PersonID = @PatientID  \r\n"
			"  UPDATE PatientsT SET HasNoMeds = @MedChangedStatus WHERE PersonID = @PatientID \r\n"
			" 	SET @HasNoMedChanged = 1 \r\n"
			" END \r\n"
			"ELSE \r\n"
			"BEGIN  \r\n"
			"  SET @HasNoMedChanged = 0 \r\n"
			" END \r\n"
			// (s.dhole 2013-10-30 15:21) - PLID 62149 save reconciliation action
			"INSERT INTO PatientReconciliationT(PatientID, ReconciliationDesc )  \r\n"
			"SELECT  TOP 1 @PatientID , 'CCDA Reconciliation' FROM @NewMedicationIDTbl WHERE TransType IN (1,2,3) "
			"SET NOCOUNT OFF \r\n" 
			"SELECT @HasNoMedChanged AS HasNoMedChanged,@MedChangedStatus AS MedChangedStatus;  \r\n" 

			"Select TempTbl.ID ,TempTbl.MedicationID ,TempTbl.TransType , TempTbl.Discontinued ,EMRDataT.Data As MedName ,DrugList.EMRDataID ,TempTbl.name, TempTbl.NewDiscontinued  \r\n"
			" FROM @NewMedicationIDTbl AS TempTbl  \r\n"
			" LEFT JOIN DrugList  ON TempTbl.MedicationID = DrugList.ID \r\n"
			" LEFT JOIN  EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n" );
			_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);
			if(!prs->eof) {
				if ( AdoFldBool(prs,"HasNoMedChanged",FALSE)!=FALSE)
				{
					CString strOldValue, strNewValue;
					if (AdoFldBool(prs,"MedChangedStatus",FALSE)!=FALSE ){
						//they are checking the box
						strOldValue = "'No Medications' Status Cleared";
						strNewValue = "'No Medications' Status Selected";
					}
					else {
						//they are unchecking the box
						strOldValue = "'No Medications' Status Selected";
						strNewValue = "'No Medications' Status Cleared";
					}
					AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientHasNoMedicationsStatus, m_nPatientID, strOldValue, strNewValue, aepMedium, aetChanged);
				}
			}
			prs = prs->NextRecordset(NULL);
			while(!prs->eof) {
				// (s.dhole 2013-10-30 16:45) - PLID 56927 Auditing
				long nID =  AdoFldLong(prs,"ID",-1);
				CString strDrugName= AdoFldString (prs,"MedName","");
				if (strDrugName.IsEmpty()){
					strDrugName =AdoFldString (prs,"name","");
				}
				// Add
				switch (AdoFldLong(prs,"TransType",-1)) 
				{
					case 1: //Add
						{
							AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientCurrentMedicationReconcileAdd, nID , "", strDrugName, aepMedium, aetCreated);
							strShowResult += strDrugName; 
							strShowResult += " was MERGED\r\n";
						}
						break; 
					case 2: //UPDATE
						//UPDATE
						{
							strShowResult += strDrugName; 
							strShowResult += " was CONSOLIDATED and MERGED\r\n";
							if (nID >0){
								if ( AdoFldBool(prs,"Discontinued")!=AdoFldBool(prs,"NewDiscontinued")){
									if (AdoFldBool(prs,"NewDiscontinued")!=FALSE ){
										AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientCurrentMedicationReconcileMerged, m_nPatientID,  "<Discontinued>",FormatString("%s: Active ",  strDrugName) , aepMedium, aetChanged);
									}
									else{
										AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientCurrentMedicationReconcileMerged, m_nPatientID,  FormatString("%s: Active ",  strDrugName), "<Discontinued>",  aepMedium, aetChanged);
									}
								}
							}
						}
						break; 
					case 3: //Delete
						{
							//DELETE
							strShowResult += strDrugName; 
							strShowResult += " was REMOVED\r\n";
							AuditEvent(m_nPatientID, strPersonName, nAuditTransID, aeiPatientCurrentMedicationReconcileRemoved, nID , strDrugName , "<Deleted>", aepHigh, aetDeleted);
						}
						break; 
					default:
						break; 
				}

				prs->MoveNext();
			}
			
			CommitAuditTransaction(nAuditTransID);
			CClient::RefreshTable(NetUtils::PatientMedications,  m_nPatientID);
			m_strResultText = FormatString("Medication List Reconciliation\r\n\r\n%s",  strShowResult);
		}
		else{
			//nothing to save
			RollbackAuditTransaction(nAuditTransID);
		}
	}
}

// (s.dhole 2014-02-04 10:11) - PLID 60201 
BOOL CReconciliationDlg::CanAddOrEditProblems(CReconciliationDlg::CReconciledItemsArray &aReconciledItemsArray)
{
	// Check if any item selected to merge or delete is not link to emn and emn is open.
	// Will scan arrey for any issue, upon first issue we wel show warning and skip rest of the scan 
	for (int i=0; i < aReconciledItemsArray.GetSize(); i++) {
		ReconciledItem  aReconciledItem = aReconciledItemsArray[i];
		int nProblemID= aReconciledItem.nInternalID; 
			switch(aReconciledItem.Action){
				case aReconciledItem.eDeleteItem:
				case aReconciledItem.eMergeCurItem: 
					//Todo Chaneg as date STATUS
					if (nProblemID>0 && WarnIfEMNConcurrencyIssuesExist(this,CEmrProblem::GetEMNQueryFromProblemID(nProblemID, FALSE),
							"There is a problem linked with this emn, You can not perform reconciliation at this time",
							TRUE)) {	
								return FALSE;
					
					}
				break;
				case aReconciledItem.eAddItem: 
				case aReconciledItem.eExcludeCurItem: 
				case aReconciledItem.eKeepCurItem : 
				default :
					// do nothing
					break; 
			}
	}
	return TRUE;
}
