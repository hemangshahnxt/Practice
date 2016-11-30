// CreatePatientRecall.cpp : implementation file
//

// (b.savon 2012-02-28 17:09) - PLID 48301 - created

#include "stdafx.h"
#include "Practice.h"
#include "CreatePatientRecall.h"
#include "GlobalUtils.h"
#include "RecallUtils.h"
#include "AuditTrail.h"
#include "PatientsRc.h"

using namespace RecallUtils;
using namespace ADODB;

// CCreatePatientRecall dialog

// (j.jones 2016-02-17 16:44) - PLID 68348 - added columns for provider/location combos

enum ProviderComboColumns {
	pccID = 0,
	pccProviderName,
};

enum LocationComboColumns {
	lccID = 0,
	lccLocationName,
};

IMPLEMENT_DYNAMIC(CCreatePatientRecall, CNxDialog)

CCreatePatientRecall::CCreatePatientRecall(CCreatePatientRecall::PatientRecall &prRecall, CWnd* pParent /*=NULL*/)
	: CNxDialog(CCreatePatientRecall::IDD, pParent)
{
	m_prciPatientRecall = prRecall;

	if (m_prciPatientRecall.nPatientID != -1) {
		m_prciPatientRecall.strPatientName = GetExistingPatientName(m_prciPatientRecall.nPatientID);
		m_prciPatientRecall.nPatientDisplayID = GetExistingPatientUserDefinedID(m_prciPatientRecall.nPatientID);
	}
}

BOOL CCreatePatientRecall::OnInitDialog()
{
	CArray<int, int> aryDiagCodeIDs;
	CArray<CSimpleArray<int>, CSimpleArray<int>> aryRecallTemplateIDs;

	try {

		CNxDialog::OnInitDialog();

		//Control Setup
		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnCreate.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//If we don't have a valid patient, do nothing.
		if (IsValid()) {

			//Setup window
			CString strTitle;
			strTitle.Format( " for %s (%li)",
							 m_prciPatientRecall.strPatientName, 
							 m_prciPatientRecall.nPatientDisplayID);
			CString strCurrentTitle;
			GetWindowText(strCurrentTitle);
			SetWindowText(strCurrentTitle + strTitle);

			CString strDescription;
			// (z.manning 2013-10-29 11:19) - PLID 59212 - Renamed this to make it clear that it refers to EmrGroupsT.ID
			if( m_prciPatientRecall.nEmrGroupID != -1 ){
				strDescription = "The patient's diagnosis codes shown in green have an existing link to a recall template.  "
								 "You may also choose to create a recall without linking a diagnosis code by "
								 "selecting a recall template listed in the blue category.";
			} else {
				strDescription = "You may choose to create a recall by selecting a configured recall template listed below in the blue category.";
			}

			SetDlgItemText(IDC_STATIC_RECALL_DESCRIPTION, strDescription);

			//Setup datalists
			m_pdlRecallTemplates = BindNxDataList2Ctrl(IDC_NXDL_RECALL_TEMPLATE, true);
			m_pdlDiagnosisCodes = BindNxDataList2Ctrl(IDC_NXDL_DIAG_CODE, true);

			// (j.jones 2016-02-17 16:20) - PLID 68348 - added provider & location combos,
			// but do not set the value until the end of OnInit
			m_ProviderCombo = BindNxDataList2Ctrl(IDC_RECALL_PROVIDER_COMBO, true);
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProviderCombo->GetNewRow();
				pRow->PutValue(pccID, (long)-1);
				pRow->PutValue(pccProviderName, _bstr_t(" <No Provider>"));
				m_ProviderCombo->AddRowSorted(pRow, NULL);
			}
			m_LocationCombo = BindNxDataList2Ctrl(IDC_RECALL_LOCATION_COMBO, true);
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_LocationCombo->GetNewRow();
				pRow->PutValue(lccID, (long)-1);
				pRow->PutValue(lccLocationName, _bstr_t(" <No Location>"));
				m_LocationCombo->AddRowSorted(pRow, NULL);
			}

			m_pdlRecallTemplates->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);						
							
			//Configure the dialog based on the recall that is attempted to be created.
			//	(1.) EMR
			//	(2.) Lab
			//	(3.) Appt
			//	(4.) Any
			if( m_prciPatientRecall.nEmrGroupID != -1 &&
				m_prciPatientRecall.nLabID == -1 &&
				m_prciPatientRecall.nApptID == -1 ){

				// (r.gonet 04/13/2014) - PLID 60870 - Combined GetEMNDiagnosisCodeIDs into GetRecallTemplateDiagnosisIDs
				//Step 1.
				//			Get Current EMR DiagCodes, then, Check DiagCode and Template Link table
				//				a. Query
				//				b. Check Link Table
				GetRecallTemplateDiagnosisIDs(aryRecallTemplateIDs, aryDiagCodeIDs);

				//				c. Fill Map of possible 1-1 (DiagCode->RecallTemplate) for Auto-creation
				FillMap(aryRecallTemplateIDs, aryDiagCodeIDs);

				//STEP 1.5
				//			AUTOCREATE? if NO, move to Step 2

				//Step 2
				IntellisenseRecallTemplateList(aryRecallTemplateIDs, aryDiagCodeIDs);

			}/*END if( EMR Recall )*/
			else if( m_prciPatientRecall.nLabID != -1 &&
				m_prciPatientRecall.nEmrGroupID == -1 &&
				m_prciPatientRecall.nApptID == -1 ){

				IntellisenseRecallTemplateList(aryRecallTemplateIDs, aryDiagCodeIDs);

			}/*END if( Lab Recall )*/
			else if( m_prciPatientRecall.nApptID != -1 &&
				m_prciPatientRecall.nEmrGroupID == -1 &&
				m_prciPatientRecall.nLabID == -1 ){
				
				IntellisenseRecallTemplateList(aryRecallTemplateIDs, aryDiagCodeIDs);

			}/*END if( Appt Recall )*/
			else if( m_prciPatientRecall.nApptID == -1 &&
				m_prciPatientRecall.nEmrGroupID == -1 &&
				m_prciPatientRecall.nLabID == -1 ){
			
				IntellisenseRecallTemplateList(aryRecallTemplateIDs, aryDiagCodeIDs);

			}/*END if( Any Recall )*/

		}//END if(IsValid())

		 // (j.jones 2016-02-17 16:41) - PLID 68348 - If the provided recall has a provider or location,
		 // try to set them. If -1, or the provider/location is no longer active, try to find the patient's
		 // last appointment information.
		if (m_prciPatientRecall.nPatientID != -1) {

			bool bNeedsProviderLoaded = true, bNeedsLocationLoaded = true;

			if (m_prciPatientRecall.nProviderID != -1
				&& m_ProviderCombo->SetSelByColumn(pccID, (long)m_prciPatientRecall.nProviderID)) {

				//a record was selected
				bNeedsProviderLoaded = false;
			}

			if (m_prciPatientRecall.nLocationID != -1
				&& m_LocationCombo->SetSelByColumn(lccID, (long)m_prciPatientRecall.nLocationID)) {

				//a record was selected
				bNeedsLocationLoaded = false;
			}

			if (bNeedsProviderLoaded || bNeedsLocationLoaded) {

				//Find the most recent appt before or on today's date.
				//It may be cancelled, that's ok.
				_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ProvidersT.PersonID AS ProviderID, LocationsT.ID AS LocationID "
					"FROM AppointmentsT "
					"INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
					"LEFT JOIN ResourceProviderLinkT ON AppointmentResourceT.ResourceID = ResourceProviderLinkT.ResourceID "
					"LEFT JOIN ProvidersT ON ResourceProviderLinkT.ProviderID = ProvidersT.PersonID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS PersonT ON ProvidersT.PersonID = PersonT.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND TypeID = 1) AS LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
					"WHERE AppointmentsT.PatientID = {INT} AND dbo.AsDateNoTime(AppointmentsT.Date) <= dbo.AsDateNoTime(GetDate()) "
					"ORDER BY AppointmentsT.Date DESC; "
					""
					"SELECT ProviderPersonT.ID AS ProviderID, LocationsT.ID AS LocationID "
					"FROM PatientsT "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"LEFT JOIN (SELECT ID FROM PersonT WHERE Archived = 0) AS ProviderPersonT ON PatientsT.MainPhysician = ProviderPersonT.ID "
					"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND TypeID = 1) AS LocationsT ON PersonT.Location = LocationsT.ID "
					"WHERE PersonT.ID = {INT}",
					m_prciPatientRecall.nPatientID, m_prciPatientRecall.nPatientID);
				if (!rs->eof) {
					if (bNeedsProviderLoaded) {
						m_prciPatientRecall.nProviderID = VarLong(rs->Fields->Item["ProviderID"]->Value, -1);
						if (m_prciPatientRecall.nProviderID != -1
							&& m_ProviderCombo->SetSelByColumn(pccID, (long)m_prciPatientRecall.nProviderID)) {
							//a record was selected
							bNeedsProviderLoaded = false;
						}
					}
					if (bNeedsLocationLoaded) {
						m_prciPatientRecall.nLocationID = VarLong(rs->Fields->Item["LocationID"]->Value, -1);
						if (m_prciPatientRecall.nLocationID != -1
							&& m_LocationCombo->SetSelByColumn(lccID, (long)m_prciPatientRecall.nLocationID)) {
							//a record was selected
							bNeedsLocationLoaded = false;
						}
					}
				}

				rs = rs->NextRecordset(NULL);

				//load the patient default provider/location if we still do not have one
				if (!rs->eof) {
					if (bNeedsProviderLoaded) {
						m_prciPatientRecall.nProviderID = VarLong(rs->Fields->Item["ProviderID"]->Value, -1);
						if (m_prciPatientRecall.nProviderID != -1
							&& m_ProviderCombo->SetSelByColumn(pccID, (long)m_prciPatientRecall.nProviderID)) {
							//a record was selected
							bNeedsProviderLoaded = false;
						}
					}
					if (bNeedsLocationLoaded) {
						m_prciPatientRecall.nLocationID = VarLong(rs->Fields->Item["LocationID"]->Value, -1);
						if (m_prciPatientRecall.nLocationID != -1
							&& m_LocationCombo->SetSelByColumn(lccID, (long)m_prciPatientRecall.nLocationID)) {
							//a record was selected
							bNeedsLocationLoaded = false;
						}
					}
				}
				rs->Close();
			}
		}

		// (j.jones 2016-02-17 16:52) - PLID 68348 - if no provider or location is selected, pick the <none> row
		if (m_ProviderCombo->GetCurSel() == NULL) {
			m_ProviderCombo->SetSelByColumn(pccID, (long)-1);
		}
		if (m_LocationCombo->GetCurSel() == NULL) {
			m_LocationCombo->SetSelByColumn(lccID, (long)-1);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

CCreatePatientRecall::~CCreatePatientRecall()
{
	m_mapRecalls.RemoveAll();
}

void CCreatePatientRecall::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_NXCOLOR_BACKGROUND, m_nxcBackground);
	DDX_Control(pDX, IDOK, m_btnCreate);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CCreatePatientRecall, CNxDialog)
	ON_BN_CLICKED(IDOK, &CCreatePatientRecall::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCreatePatientRecall::OnBnClickedCancel)
END_MESSAGE_MAP()


// CCreatePatientRecall message handlers

void CCreatePatientRecall::OnBnClickedOk()
{
	try {

		CreateRecalls();

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CCreatePatientRecall::OnBnClickedCancel()
{
	try{
		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

BOOL CCreatePatientRecall::IsValid()
{
	if( m_prciPatientRecall.nPatientID == NULL || 
		m_prciPatientRecall.nPatientID == -1 )
		return FALSE;

	return TRUE;
}

void CCreatePatientRecall::GetCurrentRecallDiagCodes(CArray<long, long>& aryCurrDiagCodes)
{
	try{
		_RecordsetPtr prsDiag = CreateParamRecordset("SELECT DiagCodeID \r\n"
															"FROM	RecallT \r\n"
															"WHERE	PatientID = {INT} \r\n"
															"AND	EMRID = {INT} \r\n"
															"AND	Discontinued = 0 \r\n"
															,
															m_prciPatientRecall.nPatientID,
															m_prciPatientRecall.nEmrGroupID
															);

		while(!prsDiag->eof ){
			aryCurrDiagCodes.Add( AdoFldLong(prsDiag->Fields, "DiagCodeID", -1) );

			prsDiag->MoveNext();
		}
		prsDiag->Close();
	}NxCatchAll(__FUNCTION__);
}


// (r.gonet 04/13/2014) - PLID 60870 - In addition to retrieving the map of associated recall templates, this also fills aryDiagCodeIDs now
void CCreatePatientRecall::GetRecallTemplateDiagnosisIDs(CArray<CSimpleArray<int>, CSimpleArray<int>>& aryRecallTemplateDiagIDs, CArray<int, int>& aryDiagCodeIDs)
{
	try{
		// (r.gonet 04/13/2014) - PLID 60870 - Get the diagnosis codes that are linked to templates and the templates they are linked to.
		CArray<RecallListMap, RecallListMap> aryRecallListMap;
		// (j.jones 2016-02-18 08:41) - PLID 68390 - this now also loads the providerID and locationID, but don't try to do 
		// any of this if we don't have an EMR ID
		// (j.jones 2016-02-19 11:03) - PLID 68378 - renamed and added a bool to control auto-creating recalls,
		// which we do not want to happen here
		if (m_prciPatientRecall.nEmrGroupID != -1) {
			RecallUtils::CalculateRecallsForEMR(false, m_prciPatientRecall.nEmrGroupID, m_prciPatientRecall.nProviderID, m_prciPatientRecall.nLocationID, aryRecallListMap);
		}

		// (r.gonet 04/13/2014) - PLID 60870 - We need to massage the data returned to get it into two arrays. 
		// One an array of arrays containing recall template IDs. The other containing diagnosis codes.
		// aryDiagCodeIDs[i] is linked to the recall template IDs in aryRecallTemplateDiagIDs[i].
		long nCurrentDiagCodeID = -1;
		CSimpleArray<int> aryCurrentDiagCodeTemplateIDs;
		for(int i = 0; i < aryRecallListMap.GetSize(); i++) {
			long nDiagCodeID = aryRecallListMap[i].nDiagCodeID;
			if(nCurrentDiagCodeID == -1) {
				nCurrentDiagCodeID = nDiagCodeID;
			}
			if(nCurrentDiagCodeID != nDiagCodeID) {
				aryRecallTemplateDiagIDs.Add(aryCurrentDiagCodeTemplateIDs);
				aryDiagCodeIDs.Add(nCurrentDiagCodeID);
				aryCurrentDiagCodeTemplateIDs.RemoveAll();
				nCurrentDiagCodeID = nDiagCodeID;
			}
			aryCurrentDiagCodeTemplateIDs.Add((int)aryRecallListMap[i].nRecallTemplateID);
		}
		if(aryRecallListMap.GetSize() > 0) {
			aryRecallTemplateDiagIDs.Add(aryCurrentDiagCodeTemplateIDs);
			aryDiagCodeIDs.Add(nCurrentDiagCodeID);
		}
	}NxCatchAll(__FUNCTION__);
}

void CCreatePatientRecall::FillMap(CArray<CSimpleArray<int>, CSimpleArray<int>>& aryRecallTemplateDiagIDs, CArray<int, int>& aryDiagCodeIDs)
{
	try{
		for( int i = 0; i < aryDiagCodeIDs.GetSize(); i++ ){
			if( aryRecallTemplateDiagIDs.GetAt(i).GetSize() == 1 ){
					m_mapRecalls[aryDiagCodeIDs.GetAt(i)] = aryRecallTemplateDiagIDs.GetAt(i)[0];
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CCreatePatientRecall::IntellisenseRecallTemplateList(CArray<CSimpleArray<int>, CSimpleArray<int>>& aryRecallTemplateDiagIDs, CArray<int, int>& aryDiagCodeIDs)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pBlankRow;

		//For each diag code in our list, display it if and any associated recall templates (if any)
		for( int iCode = 0; iCode < aryDiagCodeIDs.GetSize(); iCode++ ){
			//Only add Diag Codes to the list if they have any recall template tied to them.
			if( aryRecallTemplateDiagIDs.GetAt(iCode).GetSize() > 0 ){

				//make sure the list has loaded
				m_pdlDiagnosisCodes->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

				//Step 0:
				//	Get DiagCode description for group header
				NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_pdlDiagnosisCodes->FindByColumn(dccID, aryDiagCodeIDs.GetAt(iCode), 0, VARIANT_FALSE);
				CString strDiagCode = "";
				if( pDiagRow ){
					strDiagCode = AsString(pDiagRow->GetValue(dccCode)) + " - " + AsString(pDiagRow->GetValue(dccDescription));
				}

				//Step 1:
				//	PARENT:: Add Intellisense header - assign the diag code ID as the parent header row ID so for manual creation
				NXDATALIST2Lib::IRowSettingsPtr  pIntellisenseRow = m_pdlRecallTemplates->GetNewRow();
				pIntellisenseRow->PutValue(rtcID, aryDiagCodeIDs.GetAt(iCode));/*Not -> INTELLISENSE_RECALL_HEADING_ID*/
				pIntellisenseRow->PutValue(rtcName, _bstr_t(INTELLISENSE_RECALL_HEADING_NAME + strDiagCode + "  ***"));
				pIntellisenseRow->PutBackColor(RGB(128,255,128));//Green
				m_pdlRecallTemplates->AddRowBefore(pIntellisenseRow, m_pdlRecallTemplates->GetFirstRow());

				//Step 2:
				//	CHILDREN:: Add all recall templates for diag code [iCode] at the top
				NXDATALIST2Lib::IRowSettingsPtr pRow;
				for(int iRecallTemplate = 0; iRecallTemplate < aryRecallTemplateDiagIDs.GetAt(iCode).GetSize(); iRecallTemplate++){
					pRow = m_pdlRecallTemplates->FindByColumn(rtcID, aryRecallTemplateDiagIDs.GetAt(iCode)[iRecallTemplate], 0, VARIANT_FALSE);
					//If we found a row (and the ID is valid), let's place it at the top.
					if( pRow ){
						m_pdlRecallTemplates->AddRowAtEnd(pRow, pIntellisenseRow);
					}
				}
				pIntellisenseRow->PutExpanded(VARIANT_TRUE);

				//Step 3:
				//	Add blank row
				pBlankRow = m_pdlRecallTemplates->GetNewRow();
				pBlankRow->PutValue(rtcID, BLANK_HEADING_ID);
				pBlankRow->PutValue(rtcName, _bstr_t(BLANK_HEADING_CODE));
				m_pdlRecallTemplates->AddRowBefore(pBlankRow, pIntellisenseRow->GetNextRow());
			}//END if( diag has recall template )
		}

		//Now, group all the recall templates
		//	Save all the templates in an array and remove them from the DL as they are added.
		//	Start from the bottom up because we intellisensed our diag->recall groups
		CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> aryAllTemplates;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallTemplates->GetLastRow();
		while( pRow ){
			//If we start from the bottom and reach a blank row, we've gotten to the intellisense.
			//We're done now.
			if( AsLong(pRow->GetValue(rtcID)) == BLANK_HEADING_ID ){
				break;
			}else{
				//Otherwise, add our row to the array
				aryAllTemplates.Add(pRow);
				pRow = pRow->GetPreviousRow();
				//Remove the correct row logic
				if( pRow == NULL ){
					m_pdlRecallTemplates->RemoveRow(m_pdlRecallTemplates->GetFirstRow());
					break;
				} else{
					m_pdlRecallTemplates->RemoveRow(pRow->GetNextRow());
				}
			}
		}

		// Then, add an All Recall section as a Parent, and add all the rows as Children.
		//	PARENT::
		NXDATALIST2Lib::IRowSettingsPtr  pAllRow = m_pdlRecallTemplates->GetNewRow();
		pAllRow->PutValue(rtcID, INTELLISENSE_RECALL_HEADING_ID);
		pAllRow->PutValue(rtcName, aryAllTemplates.GetSize() == 0 ? _bstr_t("***  No Recall Templates Configured  ***") : _bstr_t("***  All Recall Templates  ***"));
		pAllRow->PutBackColor(RGB(153, 204, 255));//Blue
		m_pdlRecallTemplates->AddRowBefore(pAllRow, NULL);
		
		//	CHILDREN::
		for( int i = 0; i < aryAllTemplates.GetSize(); i++ ){
			m_pdlRecallTemplates->AddRowAtEnd(aryAllTemplates.GetAt(i), pAllRow);
		}
		pAllRow->PutExpanded(VARIANT_TRUE);

		SetMapChecks();
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

void CCreatePatientRecall::FillListStruct(CArray<RecallUtils::RecallListMap, RecallUtils::RecallListMap>& aryRecallListMap)
{
	try{
		//Run through the data list and add mappings to our struct map array
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallTemplates->GetFirstRow();
		while(pRow){
			//Get all the rows that are checked.
			if( pRow->GetParentRow() != NULL && AsBool(pRow->GetValue(rtcCreate)) ){
				long nRecallTemplateID = AsLong(pRow->GetValue(rtcID));
				if( pRow->GetParentRow() != NULL ){
					long nDiagCodeID = AsLong(pRow->GetParentRow()->GetValue(rtcID));
					RecallUtils::RecallListMap rlmPair(nDiagCodeID, nRecallTemplateID);
					aryRecallListMap.Add(rlmPair);
				}else{
					TRACE("Unable to set recall for RecallTemplateID: %li\n", nRecallTemplateID);
				}
			}//END if( child )
			pRow = m_pdlRecallTemplates->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CCreatePatientRecall::CreateRecalls()
{
	//throw exceptions to the caller

	// (j.armen 2012-03-28 10:45) - PLID 48480 - The user is trying to create a recall.  Use the license now!
	if(!g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrUse)) {
		return;
	}
	//Declare and fill map of recalls to be created from checked datalist.
	CArray<RecallListMap, RecallListMap> aryRecallListMap;
	CCreatePatientRecall::FillListStruct(aryRecallListMap);

	// (j.jones 2016-02-17 17:03) - PLID 68348 - get the provider ID and location ID from the interface
	long nProviderID = -1;
	if (m_ProviderCombo->GetCurSel()) {
		nProviderID = VarLong(m_ProviderCombo->GetCurSel()->GetValue(pccID), -1);
	}
	long nLocationID = -1;
	if (m_LocationCombo->GetCurSel()) {
		nLocationID = VarLong(m_LocationCombo->GetCurSel()->GetValue(lccID), -1);
	}

	// (a.walling 2013-12-13 10:20) - PLID 60010 - Creating recalls now in the shared RecallUtils namespace
	RecallUtils::CreateRecalls(m_prciPatientRecall.nPatientID, m_prciPatientRecall.nEmrGroupID, 
		m_prciPatientRecall.nApptID, m_prciPatientRecall.nLabID,
		nProviderID, nLocationID,
		aryRecallListMap);
}

void CCreatePatientRecall::SetMapChecks()
{
	try{
		//Run through the data list and check off the Create checkbox if they are in the map.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallTemplates->GetFirstRow();
		while(pRow){
			//Set all the rows that are checked.
			if( pRow->GetParentRow() != NULL /*&& VarBool(pRow->GetValue(rtcCreate))*/ ){
				int nRecallTemplateID = AsLong(pRow->GetValue(rtcID));
				int nDiagCodeID = AsLong(pRow->GetParentRow()->GetValue(rtcID));
				if( m_mapRecalls.Lookup(nDiagCodeID, nRecallTemplateID) ){
					pRow->PutValue(rtcCreate, VARIANT_TRUE);
				}
			}//END if( child )
			pRow = m_pdlRecallTemplates->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CCreatePatientRecall::UpdateButtons()
{
	if( GetCheckedRecallCount() <= 0 ){
		m_btnCreate.EnableWindow(FALSE);
	}else{
		m_btnCreate.EnableWindow(TRUE);
	}
}

long CCreatePatientRecall::GetCheckedRecallCount()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallTemplates->GetFirstRow();
	long nCount = 0;
	while(pRow){
		//Get all the rows that are checked.
		if( pRow->GetParentRow() != NULL && AsBool(pRow->GetValue(rtcCreate)) ){
			++nCount;
		}//END if( checked child )
		pRow = m_pdlRecallTemplates->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
	}//while( valid row )

	return nCount;
}

BEGIN_EVENTSINK_MAP(CCreatePatientRecall, CNxDialog)
	ON_EVENT(CCreatePatientRecall, IDC_NXDL_RECALL_TEMPLATE, 10, CCreatePatientRecall::EditingFinishedNxdlRecallTemplate, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


void CCreatePatientRecall::EditingFinishedNxdlRecallTemplate(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			switch( nCol ){
				case rtcCreate:
					{	
						UpdateButtons();
					}
					break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}