//CFirstDataBankImport.cpp


// (j.gruber 2010-07-14 11:02) - PLID  39049 - created for
#include "stdafx.h"
#include "FirstDataBankImportDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "AddComplexMedicationNameDlg.h"
#include "FirstDataBankUtils.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>
#include "PatientsRc.h"

#define FDB_OBSOLETE RGB(167,166,166)

enum SearchListColumns {
	slcFDBID = 0,
	slcImport,
	slcName,
	slcStrength,
	slcStrengthUnit,
	slcDosageForm,
	slcMedicationStatus,
	slcImportStatus,
};

IMPLEMENT_DYNAMIC(CFirstDataBankImportDlg, CNxDialog)

CFirstDataBankImportDlg::CFirstDataBankImportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFirstDataBankImportDlg::IDD, pParent)
{

}

CFirstDataBankImportDlg::~CFirstDataBankImportDlg()
{
}

void CFirstDataBankImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnImport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_FDB_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_NXC_BACK, m_nxcBackground);
}

BOOL CFirstDataBankImportDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//make sure we are connected to FDB
		if (!FirstDataBank::EnsureDatabase(this, true)) {
			OnCancel();
			return TRUE;
		}

		m_btnImport.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSearch.AutoSet(NXB_INSPECT);

		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_pSearchList = BindNxDataList2Ctrl(IDC_FDB_MED_LIST, false);

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pSearchList->GetColumn(slcImport);
		if (pCol) {
			pCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csEditable);
		}

		GetDlgItem(IDC_EDT_SEARCH)->SetFocus();

	}NxCatchAll(__FUNCTION__);

	return TRUE;

}

BEGIN_MESSAGE_MAP(CFirstDataBankImportDlg, CNxDialog)
	ON_BN_CLICKED(IDC_FDB_SEARCH, &CFirstDataBankImportDlg::OnBnClickedFdbSearch)
	ON_BN_CLICKED(IDOK, &CFirstDataBankImportDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFirstDataBankImportDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHK_SHOW_NON_ACTIVE, &CFirstDataBankImportDlg::OnBnClickedChkShowNonActive)
END_MESSAGE_MAP()

void CFirstDataBankImportDlg::LoadFDBList()
{

	CString strSearch;

	GetDlgItemText(IDC_EDT_SEARCH, strSearch);

	if (strSearch.GetLength() < 3) {
		MsgBox("Please enter a search string greater than 3 characters.");
		return;
	}

	CWaitCursor cwait;

	m_pSearchList->Clear();
		

	// (b.savon 2012-08-09 18:02) - PLID 51703 - Restructure to use the API
	// Call the API to do the Allergy Search.  We must covert the SAFEARRAY to a CArray before we can do anything useful with it.
	NexTech_Accessor::MedicationStatus medStatus = IsDlgButtonChecked(IDC_CHK_SHOW_NON_ACTIVE) == BST_CHECKED ? 
													NexTech_Accessor::MedicationStatus_AllNonActive :
													NexTech_Accessor::MedicationStatus_Active;

	// (j.fouts 2012-09-07 16:30) - PLID 51997 - Added a filter
	NexTech_Accessor::_FDBMedicationFilterPtr pFilter(__uuidof(NexTech_Accessor::FDBMedicationFilter));

	pFilter->NameLike = _bstr_t(strSearch);
	pFilter->_MedicationStatus = medStatus;
	pFilter->IncludeMonograph = FALSE;
	// (j.jones 2016-01-20 15:15) - PLID 68011 - filter out non-FDB drugs
	NexTech_Accessor::_NullableBoolPtr pIncludeFDBMedsOnly(__uuidof(NexTech_Accessor::NullableBool));
	pIncludeFDBMedsOnly->SetBool(VARIANT_TRUE);
	pFilter->IncludeFDBMedsOnly = pIncludeFDBMedsOnly;

	NexTech_Accessor::_FDBMedicationArrayPtr searchResults = GetAPI()->MedicationSearch(GetAPISubkey(), GetAPILoginToken(), pFilter);

	//	If our search produced no results, tell the user and bail
	if (searchResults->FDBMedications == NULL){
		MsgBox("No medications that start with '%s' could be found.  Please check the spelling and try your search again.", strSearch);
		return;
	}	

	Nx::SafeArray<IUnknown *> saryMedications(searchResults->FDBMedications);

	// (b.savon 2013-03-27 09:18) - PLID 54919 - Add a message if non are found
	if( saryMedications.GetCount() == 0 ){
		MsgBox("No medications that start with '%s' could be found.  Please check the spelling and try your search again.", strSearch);
		return;
	}

	// Run through our array and add the meds found to the list
	foreach(NexTech_Accessor::_FDBMedicationPtr med, saryMedications){
		// (b.savon 2013-01-29 18:15) - PLID 54919
		if( med->MedID != -1 ){ // Only do if this is an FDB Medications 
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetNewRow();
			pRow->PutValue(slcFDBID, med->MedID);
			pRow->PutValue(slcImport, g_cvarFalse);
			pRow->PutValue(slcName, med->MedName);
			pRow->PutValue(slcImportStatus, med->AlreadyInData == VARIANT_TRUE ? _bstr_t("Already Imported") : _bstr_t("Available To Import"));
			pRow->PutValue(slcMedicationStatus, (long)med->MedStatus);
			if( med->AlreadyInData == VARIANT_TRUE ){
				// Let's visually let the user know these have been imported already.  We'll piggy back the 'discontinue' color to do this.
				pRow->PutBackColor(RGB(222, 225, 231));
			}

			m_pSearchList->AddRowAtEnd(pRow, NULL);
		}
		else {
			// (j.jones 2016-01-20 15:16) - PLID 68011 - all returned meds ought to be FDB only now,
			// if not, something in the API has failed
			ASSERT(FALSE);
		}
	}

}

void CFirstDataBankImportDlg::OnBnClickedFdbSearch()
{
	try {		

		LoadFDBList();

	}NxCatchAll(__FUNCTION__);
}

void CFirstDataBankImportDlg::ImportMedications(CMap<long, long, CString, LPCTSTR> *pmapFDBs) 
{
	try {

		// (b.savon 2012-08-09 18:01) - PLID 51703 - Restructure to use the API
		if( pmapFDBs->GetSize() <= 0 ){
			MsgBox("There were no medications chosen to be imported.  Please select medications to import.");
			return;
		}

		POSITION pos = pmapFDBs->GetStartPosition();
		CArray<NexTech_Accessor::_FDBMedicationImportInputPtr, NexTech_Accessor::_FDBMedicationImportInputPtr> aryMedications;
		long nFDBID;
		CString strName;
		while (pos != NULL) {
		
			NexTech_Accessor::_FDBMedicationImportInputPtr med(__uuidof(NexTech_Accessor::FDBMedicationImportInput));
			
			pmapFDBs->GetNextAssoc(pos, nFDBID, strName);
			
			med->FirstDataBankID = nFDBID;
			med->MedicationName = _bstr_t(strName);

			aryMedications.Add(med);
		}

		//	Create our SAFEARRAY to be passed to the AllergyImport function in the API
		Nx::SafeArray<IUnknown *> saryMedications = Nx::SafeArray<IUnknown *>::From(aryMedications);

		//	Call the API to import the allergies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		NexTech_Accessor::_FDBMedicationImportResultsArrayPtr importResults = GetAPI()->MedicationImport(GetAPISubkey(), GetAPILoginToken(), saryMedications);

		//	If for some reason we get nothing back (although, this should never happen), tell the user and bail.
		if( importResults->FDBMedicationImportResults == NULL ){
			MsgBox("There were no medications to import.");
			return;
		}

		Nx::SafeArray<IUnknown *> saryMedicationResults(importResults->FDBMedicationImportResults);

		//	Prepare our results message so the user knows which imported succesfully (or not)
		CString strSuccess = "The following medications have imported successfully:\r\n";
		CString strFailure = "\r\nThe following medications are already imported:\r\n";
		foreach(NexTech_Accessor::_FDBMedicationImportOutputPtr pMedicationResult, saryMedicationResults)
		{
			if( pMedicationResult->Success == VARIANT_TRUE ){
				strSuccess += CString((LPCTSTR)pMedicationResult->MedName) + "\r\n";
			}else{
				strFailure += CString((LPCTSTR)pMedicationResult->MedName) + "\r\n";
			}
		}

		CString strResults;
		if( strSuccess != "The following medications have imported successfully:\r\n" ){
			strResults += strSuccess;
		}

		if( strFailure != "\r\nThe following medications are already imported:\r\n" ){
			strResults += strFailure;
		}

		if( !strResults.IsEmpty() ){
			MsgBox("%s", strResults);
		}


	}NxCatchAll(__FUNCTION__);
}

void CFirstDataBankImportDlg::OnBnClickedOk()
{
	try {

		//loop through the whole list and see if any are checked
		CMap<long, long, CString, LPCTSTR>  mapFDBs;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetFirstRow();
			
		CWaitCursor cwait;

		while (pRow) {

			if (VarBool(pRow->GetValue(slcImport), FALSE)) {
				// (b.savon 2012-08-09 18:01) - PLID 51703 - Update to use API calls
				CString strName = VarString(pRow->GetValue(slcName));
				mapFDBs.SetAt(VarLong(pRow->GetValue(slcFDBID)), strName);
			}
			pRow = pRow->GetNextRow();
		}

		if (mapFDBs.GetSize() > 0) {

			ImportMedications(&mapFDBs);
		}

		OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CFirstDataBankImportDlg::OnBnClickedCancel()
{
	try {
		OnCancel();
	}NxCatchAll(__FUNCTION__);
}

void CFirstDataBankImportDlg::OnBnClickedChkShowNonActive()
{
	try {
		LoadFDBList();
	}NxCatchAll(__FUNCTION__) 
}