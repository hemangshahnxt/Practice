// MedicationSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientDialog.h"
#include "MedicationSelectDlg.h"
#include "GlobalDataUtils.h"
#include "EditMedicationListDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "DontShowDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CMedicationSelectDlg dialog
// (c.haag 2007-03-08 12:51) - PLID 25110 - This dialog no longer uses GetActivePatientID
// except for the constructor

CMedicationSelectDlg::CMedicationSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMedicationSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMedicationSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// (c.haag 2007-03-08 12:48) - PLID 25110 - We now retain the patient ID
	SetPatientID(GetActivePatientID());
	m_nMedicationID = -1;
}


void CMedicationSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMedicationSelectDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_MED_LIST, m_btnEditMedList);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_bkg);
	DDX_Control(pDX, IDC_FIRSTDATE, m_nxeditFirstdate);
	DDX_Control(pDX, IDC_DATEDESC, m_nxstaticDatedesc);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMedicationSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMedicationSelectDlg)
	ON_BN_CLICKED(IDC_EDIT_MED_LIST, OnEditMedList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMedicationSelectDlg message handlers

BOOL CMedicationSelectDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-04-25 12:28) - PLID 29790 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2008-05-20 15:39) - PLID 30079 - removed the todo creation code

		//Initialize the datalist
		m_pMedList = BindNxDataListCtrl(this, IDC_MEDICATION_LIST, GetRemoteData(), true);

		// (b.savon 2013-01-07 13:38) - PLID 54459 - Unify the UI
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		RequeryDialog();
	
	}NxCatchAll("Error in InitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMedicationSelectDlg::OnOK() 
{	
	//get the current Selection
	long nCurSel = m_pMedList->GetCurSel();

	if (nCurSel != -1) {
		//set the medicationID
		m_nMedicationID = VarLong(m_pMedList->GetValue(nCurSel, mlcMedicationID));
		// (b.savon 2012-09-05 11:32) - PLID 52454 - Propagate the FDBID for meds from import to current list.
		m_nFDBID = VarLong(m_pMedList->GetValue(nCurSel, mlcFirstDataBankID), -1);

		// (j.jones 2008-05-20 15:39) - PLID 30079 - removed the todo creation code

		CDialog::EndDialog(IDOK);
	}
	else {
		//tell them to pick something or click cancel
		MessageBox("Please select a medication or click Cancel");
	}
}

void CMedicationSelectDlg::OnEditMedList() 
{
	// (a.walling 2007-04-04 15:19) - PLID 25459 - Chris put a check here for EMR permissions. We never really
	// checked for any permission other than the EMR one. But now let's check for the new Edit Medication List.

	// (will prompt for passwords)
	if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
		return;
	}

	//open the edit medication dialog

	CEditMedicationListDlg dlg(this);
	dlg.DoModal();

	//refresh the list just in case something has changed
	m_pMedList->Requery();
}

void CMedicationSelectDlg::OnCancel() 
{
	CDialog::EndDialog(IDCANCEL);
}

// (a.wilson 2013-02-07 15:18) - PLID 55014 - rewrote this to improve efficiency and add a passable default.
void CMedicationSelectDlg::RequeryDialog()
{
	try {
		//Set the text for the first of date
		// (j.jones 2008-04-25 09:03) - PLID 28933 - no need for a recordset, use GetExistingPatientName()
		CString str;
		str.Format("Date of First Prescription for %s", GetExistingPatientName(GetPatientID()));
		SetDlgItemText(IDC_DATEDESC, str);

		bool bAssignedMedication = false;
		//check to see if a initial medication was passed.
		if (m_nMedicationID != -1)
		{
			if (m_pMedList->FindByColumn(0, m_nMedicationID, NULL, VARIANT_TRUE) != -1)
				bAssignedMedication = true;
		}
		//if no initial medication was defined or if the assignment was a failure,
		//try setting to patient's last prescription.
		if (!bAssignedMedication)
		{
			_RecordsetPtr rsLastMed;
			// (j.jones 2008-04-25 08:59) - PLID 28933 - only select medications that are going to exist in our list
			// (also parameterized the recordsets in this function)
			rsLastMed = CreateParamRecordset("SELECT TOP 1 PatientMedications.ID, PatientMedications.MedicationID As LastMedID "
				"FROM PatientMedications "
				"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				"INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"WHERE EMRDataT.Inactive = 0 "
				"AND PatientMedications.Deleted = 0 "
				"AND PatientID = {INT} "
				"ORDER BY PatientMedications.ID Desc", GetPatientID());	

			if (!rsLastMed->eof)
			{
				_variant_t varLastID = rsLastMed->Fields->Item["LastMedID"]->Value;

				if (varLastID.vt == VT_I4 || varLastID.vt == VT_INT)
				{
					m_nMedicationID = VarLong(varLastID);
					if (m_pMedList->FindByColumn(0, m_nMedicationID, NULL, VARIANT_TRUE) != -1)
						bAssignedMedication = true;
				}
			}
			rsLastMed->Close();
		}
		//if assignment still failed then set to first record.
		if (!bAssignedMedication)
		{
			m_pMedList->PutCurSel(0);
			//also set first prescription date to an empty string since we couldn't find a valid record.
			SetDlgItemText(IDC_FIRSTDATE, " ");
		}
		//if we succeeded then determine if they have a prescription date for it.
		else
		{
			//Set the Date
			_RecordsetPtr rsLastDate = CreateParamRecordset("SELECT Min(PatientMedications.PrescriptionDate) AS FirstOfPrescriptionDate "
					"FROM PatientMedications GROUP BY PatientMedications.PatientID, "
					"PatientMedications.MedicationID, PatientMedications.Deleted "
					"HAVING PatientMedications.PatientID = {INT} AND "
					"PatientMedications.MedicationID = {INT} "
					"AND PatientMedications.Deleted = 0 "
					"ORDER BY Min(PatientMedications.PrescriptionDate)", GetPatientID(), m_nMedicationID);
			if (!rsLastDate->eof) {
				str = FormatDateTimeForInterface(AdoFldDateTime(rsLastDate, "FirstOfPrescriptionDate"), NULL, dtoDate);
				SetDlgItemText(IDC_FIRSTDATE, str);
			}
			else {
				//they don't have a prescription for this medicine yet
				SetDlgItemText(IDC_FIRSTDATE, " ");
			}
			rsLastDate->Close();
		}

	}NxCatchAll("Error in Requery Dialog");
}
	

BEGIN_EVENTSINK_MAP(CMedicationSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMedicationSelectDlg)
	ON_EVENT(CMedicationSelectDlg, IDC_MEDICATION_LIST, 2 /* SelChanged */, OnSelChangedMedicationList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CMedicationSelectDlg, IDC_MEDICATION_LIST, 18, CMedicationSelectDlg::RequeryFinishedMedicationList, VTS_I2)
END_EVENTSINK_MAP()

void CMedicationSelectDlg::OnSelChangedMedicationList(long nNewSel) 
{		
	try {
		//Get the value of the currently selected medication Id
		long nCurSel = m_pMedList->GetCurSel();
		if (nCurSel == -1) return;

		long nCurID = VarLong(m_pMedList->GetValue(nCurSel, 0));

		//Set the Date
		// (j.jones 2008-04-25 08:56) - PLID 28933 - parameterized the recordset
		_RecordsetPtr rs3 = CreateParamRecordset("SELECT Min(PatientMedications.PrescriptionDate) AS FirstOfPrescriptionDate "
				 "FROM PatientMedications GROUP BY PatientMedications.PatientID, "
				 "PatientMedications.MedicationID, PatientMedications.Deleted "
				"HAVING PatientMedications.PatientID = {INT} AND "
				"PatientMedications.MedicationID = {INT} "
				" AND PatientMedications.Deleted = 0 "
				"ORDER BY Min(PatientMedications.PrescriptionDate)", GetPatientID(), nCurID);
		if (!rs3->eof) {
			CString str;
			str = FormatDateTimeForInterface(AdoFldDateTime(rs3, "FirstOfPrescriptionDate"), NULL, dtoDate);
			SetDlgItemText(IDC_FIRSTDATE, str);
		}
		else {
			//they don't have a prescription for this medicine yet
			SetDlgItemText(IDC_FIRSTDATE, " ");
		}
		rs3->Close();
	}NxCatchAll("Error in OnSelChangedMedicationList");	
}

void CMedicationSelectDlg::SetPatientID(long nPatID)
{
	// (c.haag 2007-03-08 12:48) - PLID 25110 - Sets the patient ID
	m_nPatientID = nPatID;
}

long CMedicationSelectDlg::GetPatientID() const
{
	// (c.haag 2007-03-08 12:48) - PLID 25110 - Returns the patient ID
	return m_nPatientID;
}
// (a.wilson 2013-02-07 15:13) - PLID 55014 - use this to override the search for last med 
//and set the selected medication on open.
void CMedicationSelectDlg::SetInitialMedicationID(long nMedicationID)
{
	m_nMedicationID = nMedicationID;
}

// (b.savon 2013-01-07 13:09) - PLID 54459 - Color the Imported meds
void CMedicationSelectDlg::RequeryFinishedMedicationList(short nFlags)
{
	try{
		for( int i = 0; i < m_pMedList->GetRowCount(); i++ ){
			long nFDBID = VarLong(m_pMedList->GetValue(i, mlcFirstDataBankID), -1);
			if (nFDBID != -1) {
				//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
				// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
				if (nFDBID > 0 && VarBool(m_pMedList->GetValue(i, mlcFDBOutOfDate), FALSE)) {
					((NXDATALISTLib::IRowSettingsPtr)m_pMedList->GetRow(i))->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
				}
				else {
					((NXDATALISTLib::IRowSettingsPtr)m_pMedList->GetRow(i))->PutBackColor(ERX_IMPORTED_COLOR);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
	// (a.wilson 2013-02-07 15:39) - PLID 55014 - moved function call to here to ensure datalist is fully loaded before we use it.
	RequeryDialog();	
}
