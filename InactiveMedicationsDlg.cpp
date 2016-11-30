// InactiveMedicationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "InactiveMedicationsDlg.h"
#include "AuditTrail.h"
#include "EmrUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CInactiveMedicationsDlg dialog


CInactiveMedicationsDlg::CInactiveMedicationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInactiveMedicationsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInactiveMedicationsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInactiveMedicationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInactiveMedicationsDlg)
	DDX_Control(pDX, IDC_ACTIVATE_MEDICATION, m_btnActivate);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInactiveMedicationsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInactiveMedicationsDlg)
	ON_BN_CLICKED(IDC_ACTIVATE_MEDICATION, OnActivateMedication)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInactiveMedicationsDlg message handlers

void CInactiveMedicationsDlg::OnActivateMedication() 
{
	
	try {
		//get the current selection
		if (m_pInactiveList->CurSel == -1) {

			return;
		}
		else {

			if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to re-activate this medication?")) {

				long nMedID = VarLong(m_pInactiveList->GetValue(m_pInactiveList->CurSel, 0));

				// (c.haag 2007-01-31 09:13) - PLID 24422 - We now do all the data work for the
				// DrugList table and EMR in one function
				ExecuteActivation(nMedID);

				m_pInactiveList->Requery();

			}

		}
	}NxCatchAll("Error Activating Medication");

}

BOOL CInactiveMedicationsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 12:36) - PLID 29790 - NxIconify the buttons
		m_btnActivate.AutoSet(NXB_MODIFY);
		m_btnOK.AutoSet(NXB_CLOSE);


		m_pInactiveList = BindNxDataListCtrl(this, IDC_INACTIVE_MEDICATION_LIST, GetRemoteData(), TRUE);
	}
	NxCatchAll("Error in CInactiveMedicationsDlg::OnInitDialog");
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInactiveMedicationsDlg::ExecuteActivation(long nDrugID)
{
	//
	// (c.haag 2007-01-31 11:40) -  PLID 24422 - This function is called to activate a medication
	// in the DrugList table. We must update the EMR data as well.
	//
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteActivation")

		long nEmrInfoID = GetActiveCurrentMedicationsInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchCurrentMedicationsInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		long nDataID = VarLong(GetTableField("DrugList", "EMRDataID", "ID", nDrugID));

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = %d", nDataID);
		strOldValue.Format("Item: 'Current Medications' - Table Row: %s (Inactive)", AdoFldString(prs, "Data"));
		strNewValue.Format("Table Row: %s (Active)", AdoFldString(prs, "Data"));
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemInactive, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetChanged);

		//// Begin legacy delete code /////
		// (c.haag 2007-02-02 19:07) - PLID 24565 - DrugList no longer has an inactive flag
		//ExecuteSql("Update DrugList SET Inactive = 0 WHERE ID = %li", nDrugID);

		//// End legacy delete code /////

		ExecuteSql("UPDATE EMRDataT SET Inactive = 0 WHERE ID IN (SELECT EMRDataID FROM DrugList WHERE ID = %d)", nDrugID);

	END_TRANS("ExecuteActivation")

	// (c.haag 2007-02-07 13:49) - PLID 24422 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in DrugList with a bad EmrDataID, or a record exists
	// in EmrDataT for the active Current Medications item that does not correspond to DrugList
	WarnEmrDataDiscrepanciesWithDrugList();
}