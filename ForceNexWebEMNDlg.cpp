// ForceNexWebEMNDlg.cpp : implementation file
// (d.thompson 2009-11-04) - PLID 35811 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "ForceNexWebEMNDlg.h"
#include "AuditTrail.h"

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum eForceColumns {
	eEMNID = 0,
	eUserDefinedID,
	ePersonID,
	ePatName,
	eInputDate,
	eDescription,
};

// CForceNexWebEMNDlg dialog

IMPLEMENT_DYNAMIC(CForceNexWebEMNDlg, CNxDialog)

CForceNexWebEMNDlg::CForceNexWebEMNDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CForceNexWebEMNDlg::IDD, pParent)
{

}

CForceNexWebEMNDlg::~CForceNexWebEMNDlg()
{
}

void CForceNexWebEMNDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_FORCE_FINAL, m_btnForce);
}


BEGIN_MESSAGE_MAP(CForceNexWebEMNDlg, CNxDialog)
	ON_BN_CLICKED(IDC_FORCE_FINAL, &CForceNexWebEMNDlg::OnBnClickedForceFinal)
END_MESSAGE_MAP()


// CForceNexWebEMNDlg message handlers
BOOL CForceNexWebEMNDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Init controls
		m_pList = BindNxDataList2Ctrl(IDC_FORCE_EMN_LIST, false);
		// (z.manning 2011-05-23 10:23) - PLID 33114 - Filter on charting permissions
		m_pList->PutWhereClause(_bstr_t("PatientCreatedStatus = 1 AND Deleted = 0 " + GetEmrChartPermissionFilter().Flatten()));
		m_pList->Requery();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnForce.AutoSet(NXB_MODIFY);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CForceNexWebEMNDlg::OnOK()
{
	//Nothing to save
	CDialog::OnOK();
}

void CForceNexWebEMNDlg::OnCancel()
{
	//Just call OK
	OnOK();
}

void CForceNexWebEMNDlg::OnBnClickedForceFinal()
{
	try {
		if(!CheckCurrentUserPermissions(bioFinalizeNexWebEMNs, sptWrite)) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow == NULL) {
			AfxMessageBox("Please select a row to finalize.");
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you sure you want to finalize this EMN?  This means that the patient will no longer be able to "
			"edit this record through the NexWeb module.\r\n"
			"This action cannot be undone.", MB_YESNO) != IDYES) {
			return;
		}

		//Get the EMN ID
		long nID = VarLong(pRow->GetValue(eEMNID));
		//And Patient Name / ID
		long nPersonID = VarLong(pRow->GetValue(ePersonID));
		CString strPatName = VarString(pRow->GetValue(ePatName));

		//Update it to '2' (finalized)
		ExecuteParamSql("UPDATE EMRMasterT SET PatientCreatedStatus = 2 WHERE EMRMasterT.ID = {INT};", nID);
		AuditEvent(nPersonID, strPatName, BeginNewAuditEvent(), aeiFinalizeNexWebEMN, nID, "Patient-Entered EMN", "Forcibly Finalized EMN", aepMedium, aetChanged);

		//Remove the row from the list
		m_pList->RemoveRow(pRow);

	} NxCatchAll(__FUNCTION__);
}
