// ReconcileMedicationsDlg.cpp : implementation file
//
// (c.haag 2010-02-17 10:05) - PLID 37403 - Initial implementation. This dialog brings up a list
// of patient Current Medications and patient Prescriptions; and gives the user the opportunity
// to select patient Prescriptions to put into the patient Current Medications list.
//

#include "stdafx.h"
#include "Practice.h"
#include "ReconcileMedicationsDlg.h"
#include "AuditTrail.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CReconcileMedicationsDlg dialog
// (s.dhole 2013-06-21 16:24) - PLID 55964 color constant
#define COLOR_PRESCRIPTION_MED RGB(239,228,176)
#define COLOR_CURRENT_MED_BLUE RGB(153,217,234)
#define COLOR_CURRENT_MED_DISCONTINUED RGB(222, 225, 231)
typedef enum {
	eclInternalID = 0L,
	eclMedicationID,
	eclEMRDataID,
	// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
	eclNewCropGUID,
	eclCheckbox,
	eclName,
	eclSig,	// (j.jones 2011-05-02 16:17) - PLID 43450 - added Sig
	eclStatus,	// (s.dhole 2013-06-18 15:23) - PLID 55964
	eclIsDiscontinued,// (s.dhole 2013-06-18 15:23) - PLID 55964
	eclUpdateDate,	 // (s.dhole 2013-06-18 15:23) - PLID 55964
	eclMedDescAndSig,// (s.dhole 2013-06-18 15:23) - PLID 55964
	eclStartDate, // (b.eyers 2015-12-18) - PLID 67749
} Columns;

IMPLEMENT_DYNAMIC(CReconcileMedicationsDlg, CNxDialog)

// If a user does not have access to create or delete patient current medications, then returns FALSE. Otherwise, returns TRUE.
/* static */ BOOL CReconcileMedicationsDlg::CanCurrentUserAccess()
{
	BOOL bCreateAccess = GetCurrentUserPermissions(bioPatientCurrentMeds) & sptCreate;
	BOOL bCreateAccessWithPass = GetCurrentUserPermissions(bioPatientCurrentMeds) & sptCreateWithPass;
	BOOL bDeleteAccess = GetCurrentUserPermissions(bioPatientCurrentMeds) & sptDelete;
	BOOL bDeleteAccessWithPass = GetCurrentUserPermissions(bioPatientCurrentMeds) & sptDeleteWithPass;
	if (!bCreateAccess && !bCreateAccessWithPass && !bDeleteAccess && !bDeleteAccessWithPass) {
		return FALSE;
	} else {
		return TRUE;
	}
}

CReconcileMedicationsDlg::CReconcileMedicationsDlg(long nPatientID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CReconcileMedicationsDlg::IDD, pParent)
{
	m_clrBack = 0;
	m_nPatientID = nPatientID;
}

CReconcileMedicationsDlg::~CReconcileMedicationsDlg()
{
}

// Adds all current medications of the member patient to the input current meds list
void CReconcileMedicationsDlg::AddCurrentMedicationsFromData()
{
	// (s.dhole 2013-06-18 15:24) - PLID  55964 Added  CurrentPatientMedsT.LastUpdateDate
	// Load discontinued medication
	// if last LastUpdateDate date is missing then use current date since we are merging medication to todayas date
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT CurrentPatientMedsT.ID AS CurrentPatientMedsID, \r\n"
		"CurrentPatientMedsT.MedicationID, \r\n"
		"EmrDataT.Data AS Name, EMRDataID, NewCropGUID, \r\n"
		"Discontinued, CurrentPatientMedsT.Sig,\r\n"
		"DrugList.NDCNumber,  ISNULL(CurrentPatientMedsT.LastUpdateDate,GETDATE()) AS LastUpdateDate \r\n"
		"FROM CurrentPatientMedsT \r\n"
		"LEFT JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID \r\n"
		"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		"WHERE PatientID = {INT} "
		,m_nPatientID);
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		Medication rm;
		rm.nCurrentPatientMedsID = AdoFldLong(f, "CurrentPatientMedsID");
		rm.nMedicationID = AdoFldLong(f, "MedicationID");
		//rm.strName = AdoFldString(f, "Name", "");
		rm.nEMRDataID = AdoFldLong(f, "EMRDataID");
		// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
		rm.strNewCropGUID = AdoFldString(f, "NewCropGUID", "");
		// (j.jones 2011-05-02 16:20) - PLID 43450 - added Sig
		rm.strSig = AdoFldString(f, "Sig", "");
		// (s.dhole 2013-06-18 15:28) - PLID 55964
		if (AdoFldBool(f, "Discontinued",FALSE) == FALSE){
			rm.bActive = VARIANT_TRUE;
		}
		else{
			rm.bActive = VARIANT_FALSE;
		}
		
		rm.strName =  AdoFldString(f, "Name", "");
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		rm.dtLastDate = AdoFldDateTime (f, "LastUpdateDate" ,dtInvalid);
		m_aOldCurMeds.Add(rm);
		prs->MoveNext();
	}
}

// Adds medications defined by EmrDataT ID's to the input current meds list
// (j.jones 2011-05-04 14:37) - PLID 43527 - added mapDataIDsToSig, which tracks the Sig for each current medication
void CReconcileMedicationsDlg::AddCurrentMedicationsFromEmrDataIDs(const CArray<long,long>& anEmrDataIDs,
																   CMap<long, long, CString, LPCTSTR> &mapDataIDsToSig)
{
	// (s.dhole 2013-06-18 15:28) - PLID 55964 Add last date
	if (anEmrDataIDs.GetSize() > 0) {
		CString strSql;
		strSql.Format(
			"SELECT DrugList.ID AS MedicationID, EmrDataT.Data AS Name, EmrDataT.ID AS EMRDataID, "
			" DrugList.NDCNumber , GETDATE() AS  lastDate "
			"FROM DrugList INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID "
			"WHERE EmrDataT.ID IN (%s)"
			,ArrayAsString(anEmrDataIDs));
		_RecordsetPtr prs = CreateRecordsetStd(strSql);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			Medication rm;
			rm.nCurrentPatientMedsID = -1;
			rm.nMedicationID = AdoFldLong(f, "MedicationID");
			//rm.strName = AdoFldString(f, "Name", "");
			long nDataID = AdoFldLong(f, "EMRDataID");
			rm.nEMRDataID = nDataID;
			// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID, which is blank here
			rm.strNewCropGUID = "";
			// (j.jones 2011-05-04 14:37) - PLID 43527 - load the Sig if it is in our map
			CString strSig = "";
			// (s.dhole 2013-06-18 15:28) - PLID 55964
			CString strNDC =  AdoFldString(f, "NDCNumber", "");
			rm.strName =  AdoFldString(f, "Name", "");
			
			rm.bActive= VARIANT_TRUE;
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			rm.dtLastDate =AdoFldDateTime (f, "lastDate",dtInvalid);
			mapDataIDsToSig.Lookup(nDataID, strSig);
			rm.strSig = strSig;
			m_aOldCurMeds.Add(rm);
			prs->MoveNext();
		}
	}
}

// Adds a single prescription, by patient medication ID, to the input Rx list
void CReconcileMedicationsDlg::AddPrescriptionByPMID(long nPatientMedicationsID)
{
	// (j.jones 2013-01-09 11:55) - PLID 54530 - just add to an array and call the
	// same function as for multiple prescriptions
	CArray<long, long> aryPrescriptionIDs;
	aryPrescriptionIDs.Add(nPatientMedicationsID);
	AddPrescriptionByPMIDs(aryPrescriptionIDs);
}

// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to add multiple prescriptions at once
void CReconcileMedicationsDlg::AddPrescriptionByPMIDs(CArray<long, long> &aryPrescriptionIDs)
{
	// (s.dhole 2013-06-18 15:28) - PLID 55964 Added  PatientMedications.PrescriptionDate 
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT PatientMedications.ID, MedicationID, EmrDataT.Data AS Name, EMRDataID, NewCropGUID, "
		"PatientMedications.PatientExplanation, DrugList.NDCNumber, PatientMedications.PrescriptionDate "
		"FROM PatientMedications "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID \r\n"
		"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		"WHERE PatientMedications.ID IN ({INTARRAY}) AND Deleted = 0"
		,aryPrescriptionIDs);
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		Prescription rx;
		rx.nPatientMedicationsID = AdoFldLong(f, "ID");
		rx.nMedicationID = AdoFldLong(f, "MedicationID");
		//rx.strName = AdoFldString(f, "Name", "");
		rx.nEMRDataID = AdoFldLong(f, "EMRDataID");
		// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
		rx.strNewCropGUID = AdoFldString(f, "NewCropGUID", "");
		// (j.jones 2011-05-02 16:20) - PLID 43450 - added Sig
		rx.strPatientExplanation = AdoFldString(f, "PatientExplanation", "");
		// (s.dhole 2013-06-18 15:28) - PLID 55964
		rx.strName = AdoFldString(f, "Name", "");
		
		rx.bActive = VARIANT_TRUE;
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		rx.dtLastDate = AdoFldDateTime (f, "PrescriptionDate",dtInvalid);

		// (b.eyers 2015-12-18) - PLID 67749
		rx.dtPrescriptionDate = AdoFldDateTime(f, "PrescriptionDate", dtInvalid);

		m_aPrescriptions.Add(rx);

		prs->MoveNext();
	}
	prs->Close();
}

// Adds a single prescription, by medication ID, to the input Rx list
// (j.jones 2011-05-04 16:51) - PLID 43527 - added strPatientExplanation
void CReconcileMedicationsDlg::AddPrescriptionByMedicationID(long nMedicationID, CString strPatientExplanation)
{
	// (s.dhole 2013-06-18 15:28) - PLID 55964  lastDate 
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT EmrDataT.Data AS Name, EMRDataID, DrugList.NDCNumber,GETDATE() AS lastDate "
		"FROM DrugList "
		"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		"WHERE DrugList.ID = {INT}"
		,nMedicationID);
	FieldsPtr f = prs->Fields;
	if (!prs->eof) {
		Prescription rx;
		rx.nPatientMedicationsID = -1;
		rx.nMedicationID = nMedicationID;
		//rx.strName = AdoFldString(f, "Name", "");
		rx.nEMRDataID = AdoFldLong(f, "EMRDataID");
		// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID, which is blank here
		rx.strNewCropGUID = "";
		rx.strPatientExplanation = strPatientExplanation;
		// (s.dhole 2013-06-18 15:28) - PLID 55964
		rx.strName =  AdoFldString(f, "Name", "");
		
		rx.bActive =  VARIANT_TRUE;
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		rx.dtLastDate = AdoFldDateTime (f, "lastDate",dtInvalid);

		// (b.eyers 2015-12-18) - PLID 67749 - the 'lastdate' is the prescription date
		rx.dtPrescriptionDate = AdoFldDateTime(f, "lastDate", dtInvalid);

		m_aPrescriptions.Add(rx);
	}
}

// Adds multiple prescriptions from NewCrop. Returns the number of prescriptions added.
int CReconcileMedicationsDlg::AddPrescriptionsByNewCropRxGUIDs(const CStringArray& astrNewCropRxGUIDs)
{
	CString strSqlFilter;
	for (int i=0; i < astrNewCropRxGUIDs.GetSize(); i++) {
		strSqlFilter += "'" + _Q(astrNewCropRxGUIDs[i]) + "',";
	}
	int nRxAdded = 0;
	if (strSqlFilter.GetLength() > 0) {
		strSqlFilter.TrimRight(",");
		// (s.dhole 2013-06-18 15:28) - PLID 55964 Added PatientMedications.PrescriptionDate
		_RecordsetPtr prs = CreateRecordset(
			"SELECT PatientMedications.ID AS PatientMedicationID, MedicationID, EmrDataT.Data AS Name, EMRDataID, NewCropGUID, "
			"PatientMedications.PatientExplanation, DrugList.NDCNumber ,PatientMedications.PrescriptionDate "
			"FROM PatientMedications "
			"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID \r\n"
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
			"WHERE PatientID = %d AND Deleted = 0 AND NewCropGUID IN (%s)", m_nPatientID, strSqlFilter);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			Prescription rx;
			rx.nPatientMedicationsID = AdoFldLong(f, "PatientMedicationID");
			rx.nMedicationID = AdoFldLong(f, "MedicationID");
			//rx.strName = AdoFldString(f, "Name", "");
			rx.nEMRDataID = AdoFldLong(f, "EMRDataID");
			// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
			rx.strNewCropGUID = AdoFldString(f, "NewCropGUID", "");
			// (j.jones 2011-05-02 16:20) - PLID 43450 - added Sig
			rx.strPatientExplanation = AdoFldString(f, "PatientExplanation", "");
			// (s.dhole 2013-06-18 15:28) - PLID 55964
			rx.strName = AdoFldString(f, "Name", "");
			
			rx.dtLastDate=  AdoFldDateTime (f, "PrescriptionDate");

			// (b.eyers 2015-12-18) - PLID 67749 - save the prescription date
			rx.dtPrescriptionDate = AdoFldDateTime(f, "PrescriptionDate");

			m_aPrescriptions.Add(rx);
			nRxAdded++;
			prs->MoveNext();
		}
	}
	return nRxAdded;
}

void CReconcileMedicationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MEDICATIONS_BKG, m_nxcBack);	
}

BEGIN_MESSAGE_MAP(CReconcileMedicationsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CReconcileMedicationsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CReconcileMedicationsDlg message handlers

BOOL CReconcileMedicationsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// Set control styles
		m_btnOk.AutoSet(NXB_OK); // (s.dhole 2013-06-21 16:27) - PLID 55964 change view
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_nxcBack.SetColor(m_clrBack);
		extern CPracticeApp theApp;
		GetDlgItem(IDC_STATIC_MED_HEADER)->SetFont(&theApp.m_boldFont);// (s.dhole 2013-06-21 16:27) - PLID 55964 

		// Populate the list with existing items
		m_dlMedRx = BindNxDataList2Ctrl(IDC_LIST_MED_NEW_RX, false);
		// (s.dhole 2013-06-18 15:43) - PLID 55964
		m_dlOldMedRx = BindNxDataList2Ctrl(IDC_LIST_MEDRX2, false);

		//		Current medications
		
		
		int i;
		_variant_t vTrue((bool)true);
		_variant_t vFalse((bool)false);
		for (i=0; i < m_aOldCurMeds.GetSize(); i++) {
			IRowSettingsPtr pRow = m_dlOldMedRx->GetNewRow();
			pRow->PutValue(eclInternalID, m_aOldCurMeds[i].nCurrentPatientMedsID);
			pRow->PutValue(eclMedicationID, m_aOldCurMeds[i].nMedicationID);
			pRow->PutValue(eclEMRDataID, m_aOldCurMeds[i].nEMRDataID);
			// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
			pRow->PutValue(eclNewCropGUID, _bstr_t(m_aOldCurMeds[i].strNewCropGUID));
			pRow->PutValue(eclCheckbox, vTrue);
			pRow->PutValue(eclName, _bstr_t(m_aOldCurMeds[i].strName));
			// (j.jones 2011-05-02 16:19) - PLID 43450 - supported Sig
			pRow->PutValue(eclSig, _bstr_t(m_aOldCurMeds[i].strSig));
			// (s.dhole 2013-06-18 15:23) - PLID 55964
			pRow->PutValue(eclIsDiscontinued , _variant_t(m_aOldCurMeds[i].bActive ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
			pRow->PutValue(eclStatus, _variant_t(m_aOldCurMeds[i].bActive? "Active": "Discontinued"));
			if (m_aOldCurMeds[i].bActive == VARIANT_FALSE) {
					pRow->PutCellBackColor(eclStatus , COLOR_CURRENT_MED_DISCONTINUED );
				}
			else{
				pRow->PutCellBackColor(eclStatus , COLOR_CURRENT_MED_BLUE);
			}
			CString strTemp =  m_aOldCurMeds[i].strSig;
			strTemp.Remove(' '); 
			_variant_t vtTemp= (m_aOldCurMeds[i].dtLastDate.GetStatus()==COleDateTime::valid)?_variant_t(m_aOldCurMeds[i].dtLastDate,VT_DATE): g_cvarNull ;
			pRow->PutValue(eclUpdateDate, (m_aOldCurMeds[i].dtLastDate.GetStatus()==COleDateTime::valid)?_variant_t(m_aOldCurMeds[i].dtLastDate,VT_DATE): g_cvarNull);
			pRow->PutValue(eclMedDescAndSig,  m_aOldCurMeds[i].nMedicationID);
			// (b.eyers 2015-12-18) - PLID 67749 - add start date
			pRow->PutValue(eclStartDate, (m_aOldCurMeds[i].dtStartDate.GetStatus() == COleDateTime::valid) ? _variant_t(m_aOldCurMeds[i].dtStartDate, VT_DATE) : g_cvarNull);
			pRow->PutBackColor( COLOR_CURRENT_MED_BLUE);
			
			m_dlOldMedRx->AddRowSorted(pRow, NULL);
		}

		//	// (s.dhole 2013-06-18 15:23) - PLID 55964	New prescriptions
		BOOL bOnePrescriptionAdded = TRUE;
		for (i=0; i < m_aPrescriptions.GetSize(); i++) {
			IRowSettingsPtr pRow = m_dlMedRx->GetNewRow();
			pRow->PutBackColor( COLOR_PRESCRIPTION_MED );
			pRow->PutValue(eclInternalID, m_aPrescriptions[i].nPatientMedicationsID);
			pRow->PutValue(eclMedicationID, m_aPrescriptions[i].nMedicationID);
			pRow->PutValue(eclEMRDataID, m_aPrescriptions[i].nEMRDataID);
			// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
			pRow->PutValue(eclNewCropGUID, _bstr_t(m_aPrescriptions[i].strNewCropGUID));
			pRow->PutValue(eclCheckbox, vTrue);
			pRow->PutValue(eclName, _bstr_t(m_aPrescriptions[i].strName));
			// (j.jones 2011-05-02 16:19) - PLID 43450 - supported Sig
			pRow->PutValue(eclSig, _bstr_t(m_aPrescriptions[i].strPatientExplanation));
			// (s.dhole 2013-06-18 15:23) - PLID 55964
			pRow->PutValue(eclUpdateDate, (m_aPrescriptions[i].dtLastDate.GetStatus()==COleDateTime::valid)?_variant_t(m_aPrescriptions[i].dtLastDate,VT_DATE): g_cvarNull );
			pRow->PutValue(eclIsDiscontinued ,_variant_t(m_aPrescriptions[i].bActive ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL)) ;
			pRow->PutValue(eclStatus, _variant_t(m_aPrescriptions[i].bActive? "Active": "Discontinued"));
			CString strTemp =  m_aPrescriptions[i].strPatientExplanation;
			strTemp.Remove(' '); 
			pRow->PutValue(eclMedDescAndSig, m_aPrescriptions[i].nMedicationID);
			// (b.eyers 2015-12-18) - PLID 67749 - add start date
			pRow->PutValue(eclStartDate, (m_aPrescriptions[i].dtPrescriptionDate.GetStatus() == COleDateTime::valid) ? _variant_t(m_aPrescriptions[i].dtPrescriptionDate, VT_DATE) : g_cvarNull);
			SetExistingItemFlag(pRow,FALSE);
			m_dlMedRx->AddRowSorted(pRow, NULL);
			bOnePrescriptionAdded = TRUE;
		}

		// If no prescriptions were added, don't even show the dialog. There's nothing to do since all prescriptions
		// already exist as current medications.
		if (!bOnePrescriptionAdded) {
			EndDialog(IDCANCEL);
		}
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


 //(s.dhole 2013-06-18 15:23) - PLID 55964 change function so we can open review window
void CReconcileMedicationsDlg::OnBnClickedOk()
{
	try {
		// check if there is any medication is checked
		
		CMergeMedicationArray aMergeMedicationArray;
		BOOL bNeedCreatePerms = FALSE; // Create and merge is same action
		BOOL bNeedDeletePerms = FALSE;
		BOOL bIsPrescriptiomMedicationChecked = FALSE;
		BOOL bIsCurrentMedicationChecked = FALSE;
		{// Scan Prescription list
			
			IRowSettingsPtr pRow =  m_dlMedRx->GetFirstRow();
			while (NULL != pRow) {
				BOOL bIsChecked = VarBool(pRow->GetValue(eclCheckbox));
				// check prescription list , Is there any medication mark for reconciliation? 
				if (bIsChecked != FALSE)
				{
					//there is prescription row need to add in medication list
					bNeedCreatePerms =TRUE;
				}
				MergeMedication  aMergeMedication;
				
				aMergeMedication.nPatientMedicationsID = VarLong(pRow->GetValue(eclInternalID));
				aMergeMedication.nMedicationID   = VarLong(pRow->GetValue(eclMedicationID)); 
				aMergeMedication.nEMRDataID  = VarLong(pRow->GetValue(eclEMRDataID));
				aMergeMedication.strNewCropGUID  = VarString(pRow->GetValue(eclNewCropGUID), "");
				aMergeMedication.strName = VarString(pRow->GetValue(eclName), "");
				aMergeMedication.bIsActive =VarBool(pRow->GetValue(eclIsDiscontinued)) ;
				aMergeMedication.strPatientExplanation  = VarString(pRow->GetValue(eclSig), "");
				aMergeMedication.dtLastDate  = VarDateTime (pRow->GetValue(eclUpdateDate), g_cvarNull);
				// (b.eyers 2015-12-18) - PLID 67749 - add start date
				aMergeMedication.dtStartDate = VarDateTime(pRow->GetValue(eclStartDate), g_cvarNull);
				
				if (bIsChecked != FALSE){
					// all checked row
					// If row is checkd then scan for any record Current medication record which can be matched based on name and sig , And mark as merge 
					IRowSettingsPtr pRowExist = m_dlOldMedRx->FindByColumn(eclMedDescAndSig, aMergeMedication.nMedicationID, NULL, VARIANT_FALSE);
					if (pRowExist && VarLong(pRowExist->GetValue(eclMedicationID),-1) !=-1 &&  VarBool(pRowExist->GetValue(eclCheckbox))!=FALSE ){
						aMergeMedication.nCurrentPatientMedsID= VarLong(pRowExist->GetValue(eclInternalID));
						aMergeMedication.nCurrentMedicationID  = VarLong(pRowExist->GetValue(eclMedicationID)); 
						aMergeMedication.nCurrentEMRDataID = VarLong(pRowExist->GetValue(eclEMRDataID));
						aMergeMedication.strCurrentNewCropGUID = VarString(pRowExist->GetValue(eclNewCropGUID), "");
						aMergeMedication.strCurrentName= VarString(pRowExist->GetValue(eclName), "");
						aMergeMedication.strCurrentSig = VarString(pRowExist->GetValue(eclSig), "");
						aMergeMedication.bCurrentIsActive  = VarBool(pRowExist->GetValue(eclIsDiscontinued));
						aMergeMedication.dtCurrentLastDate = VarDateTime(pRowExist->GetValue(eclUpdateDate), g_cvarNull);
						// (b.eyers 2015-12-18) - PLID 67749 - add start date
						aMergeMedication.dtCurrentStartDate = VarDateTime(pRowExist->GetValue(eclStartDate), g_cvarNull);
						
						aMergeMedication.Action = aMergeMedication.eMergeCurMed;
					}
					else{
						// found matching current medication row , but mark as remove, so we will add p this record as new record
						aMergeMedication.Action = aMergeMedication.eAddMed;
					}
				}
				else{
					// since curren record mark as not to add, then we skip this record
					aMergeMedication.Action = aMergeMedication.eExcludeCurMed;
				}
	
				aMergeMedicationArray.Add (aMergeMedication) ;
				pRow = pRow->GetNextRow();
			}

			// now scan current medication list
			pRow = m_dlOldMedRx->GetFirstRow();
			while (NULL != pRow) {
				BOOL bIsChecked = VarBool(pRow->GetValue(eclCheckbox));
				BOOL bAddRow= FALSE;
				BOOL bKeepRecord= FALSE;
				BOOL bDeleteRecord= FALSE;
				if ((bIsChecked != FALSE) )
				{
					// If record is not mark to delete
					IRowSettingsPtr pRowExist = m_dlMedRx->FindByColumn(eclMedDescAndSig, pRow->GetValue(eclMedicationID), NULL, VARIANT_FALSE);
					//check if prescription record is exist and it is not unchecked
					 if (pRowExist &&    VarBool(pRow->GetValue(eclCheckbox))== TRUE)
					 { // do nothing since this  is alredy mark to merge 
					 }
					 else{
						 // this record is not not matcjhing with any prescription
						 // will keep this record
						 // associated record from prescription list mark as deleted
						bKeepRecord = TRUE ;
						bAddRow = TRUE;
					}
				}
				else if (bIsChecked == FALSE){
					// uer mark to remove this from current prescription list
					bNeedDeletePerms = TRUE;
					bDeleteRecord=TRUE;
					bAddRow = TRUE;
					
				}
				if (bAddRow != FALSE){
					// (s.dhole 2013-09-13 10:26) - PLID 55964
					MergeMedication  aMergeMedication;
					aMergeMedication.nCurrentPatientMedsID= VarLong(pRow->GetValue(eclInternalID));
					aMergeMedication.nCurrentMedicationID  = VarLong(pRow->GetValue(eclMedicationID)); 
					aMergeMedication.nCurrentEMRDataID = VarLong(pRow->GetValue(eclEMRDataID));
					aMergeMedication.strCurrentNewCropGUID = VarString(pRow->GetValue(eclNewCropGUID), "");
					aMergeMedication.strCurrentName= VarString(pRow->GetValue(eclName), "");
					aMergeMedication.strCurrentSig = VarString(pRow->GetValue(eclSig), "");
					aMergeMedication.bCurrentIsActive =VarBool(pRow->GetValue(eclIsDiscontinued)) ;
					aMergeMedication.dtCurrentLastDate  = VarDateTime(pRow->GetValue(eclUpdateDate), g_cvarNull);
					// (b.eyers 2015-12-18) - PLID 67749 - add start date
					aMergeMedication.dtCurrentStartDate = VarDateTime(pRow->GetValue(eclStartDate), g_cvarNull);

					if (bKeepRecord){
						aMergeMedication.Action = aMergeMedication.eKeepCurMed;
					}
					else if(bDeleteRecord) {
						aMergeMedication.Action = aMergeMedication.eDeleteMed;
					}
					aMergeMedicationArray.Add (aMergeMedication);
				}
				pRow = pRow->GetNextRow();
			}

			// Now that we have the change list, check any necessary permissions. Don't
			// let the user proceed if they don't have access.
			BOOL bCreateAccess = (GetCurrentUserPermissions(bioPatientCurrentMeds) & sptCreate) ? TRUE : FALSE;
			BOOL bCreateAccessWithPass = (GetCurrentUserPermissions(bioPatientCurrentMeds) & sptCreateWithPass) ? TRUE : FALSE;
			BOOL bDeleteAccess = (GetCurrentUserPermissions(bioPatientCurrentMeds) & sptDelete) ? TRUE : FALSE;
			BOOL bDeleteAccessWithPass = (GetCurrentUserPermissions(bioPatientCurrentMeds) & sptDeleteWithPass) ? TRUE : FALSE;
			BOOL bNeedPassword = FALSE;

			// Quit if they simply don't have permission of any kind
			if (bNeedCreatePerms && !bCreateAccess && !bCreateAccessWithPass) {
				MessageBox("You do not have permission to create or merge patient medications. Please contact your office manager for assistance.", "NexTech Practice", MB_OK | MB_ICONHAND);
				return;
			}
			else if (bNeedDeletePerms && !bDeleteAccess && !bDeleteAccessWithPass) {
				MessageBox("You do not have permission to delete patient medications. Please contact your office manager for assistance.", "NexTech Practice", MB_OK | MB_ICONHAND);
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
		m_aRequestedChanges.Copy(aMergeMedicationArray);
		CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CReconcileMedicationsDlg, CNxDialog)
	
	ON_EVENT(CReconcileMedicationsDlg, IDC_LIST_MED_NEW_RX, 9, CReconcileMedicationsDlg::EditingFinishingListMedNewRx, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	
END_EVENTSINK_MAP()



void CReconcileMedicationsDlg::EditingFinishingListMedNewRx(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
				return;
		}
		SetExistingItemFlag(pRow,VarBool(pRow->GetValue(eclCheckbox)));
	}NxCatchAll(__FUNCTION__);
}

void CReconcileMedicationsDlg::SetExistingItemFlag(NXDATALIST2Lib::IRowSettingsPtr pRow,BOOL bShoCheckBox)
{
	IRowSettingsPtr pRowExist = m_dlOldMedRx->FindByColumn(eclMedDescAndSig, pRow->GetValue(eclMedicationID), NULL, VARIANT_FALSE);
	if (pRowExist){
		IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
		pfs->PutDataType(VT_BOOL);
		pfs->PutAlignV(vaVCenter);  
		if (bShoCheckBox==FALSE){
			pfs->PutFieldType(cftBoolYesNo);
			pfs->PutEditable(VARIANT_FALSE);
			pRowExist->PutValue(eclCheckbox,g_cvarTrue);
		}
		else{
			pfs->PutFieldType(cftBoolCheckbox);
			pfs->PutEditable(VARIANT_TRUE);
		}
		pRowExist->PutRefCellFormatOverride(eclCheckbox,pfs) ;
	}
}