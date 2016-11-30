// TransferTodosDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "TransferTodosDlg.h"
#include "audittrail.h"
#include "TodoUtils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CTransferTodosDlg dialog


CTransferTodosDlg::CTransferTodosDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTransferTodosDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTransferTodosDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTransferTodosDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTransferTodosDlg)
	DDX_Control(pDX, IDC_ALL_PATIENTS_RADIO, m_btnAllPatients);
	DDX_Control(pDX, IDC_SINGLE_PATIENT_RADIO, m_btnSinglePatient);
	DDX_Control(pDX, IDC_ALL, m_btnAll);
	DDX_Control(pDX, IDC_UNFINISHED, m_btnUnfinished);
	DDX_Control(pDX, IDC_FROM_GROUPBOX, m_btnFromGroupbox);
	DDX_Control(pDX, IDC_TO_GROUPBOX, m_btnToGroupbox);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTransferTodosDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTransferTodosDlg)
	ON_BN_CLICKED(IDC_TRANSFER, OnTransfer)
	ON_BN_CLICKED(IDC_SINGLE_PATIENT_RADIO, OnSinglePatientRadio)
	ON_BN_CLICKED(IDC_ALL_PATIENTS_RADIO, OnAllPatientsRadio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransferTodosDlg message handlers

BOOL CTransferTodosDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);
	
	// bind the data lists
	m_dlTransferFromUserCombo = BindNxDataListCtrl(this, IDC_TRANSFER_FROM_COMBO, GetRemoteData(), true);
	m_dlTransferToUserCombo = BindNxDataListCtrl(this, IDC_TRANSFER_TO_USER_COMBO, GetRemoteData(), true);
	m_dlForPatientCombo= BindNxDataListCtrl(this, IDC_FOR_PATIENT_COMBO, GetRemoteData(), true);
	
	// set the defaults - all patients and all status
	CheckDlgButton(IDC_ALL_PATIENTS_RADIO,TRUE);
	GetDlgItem(IDC_FOR_PATIENT_COMBO)->EnableWindow(FALSE);

	CheckDlgButton(IDC_ALL,TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTransferTodosDlg::OnTransfer() 
{

	try{
		// make sure there is a user to tranfer to and from
		if(m_dlTransferToUserCombo->GetCurSel() == -1 || m_dlTransferFromUserCombo->GetCurSel() == -1){
			AfxMessageBox("Please select a user to transfer from and a user to transfer to.");
			return;
		}

		if(IsDlgButtonChecked(IDC_SINGLE_PATIENT_RADIO) && m_dlForPatientCombo->GetCurSel() == -1){
			// they are transfering for a single patient but they don't have one selected
			AfxMessageBox("Please choose a patient from the drop down list or select \'All Patients\'.", MB_OK);
			return;
		}

		// find the ID of the transfer to
		long nTransferFromID = m_dlTransferFromUserCombo->GetValue(m_dlTransferFromUserCombo->GetCurSel(), 0);
		long nTransferToID = m_dlTransferToUserCombo->GetValue(m_dlTransferToUserCombo->GetCurSel(), 0);
		CString strWhereClause;

		if (nTransferFromID == nTransferToID) {
			AfxMessageBox("To and From user cannot be the same. Please change your selection and try again.");
			return;
		}

		if (!ConfirmUserPermissions()) {
			return;
		}
		
		// make sure they want to transfer them
		if(IDNO == MessageBox("Are you sure you wish to transfer these to-do items?", "To-Do Transfer", MB_YESNO)) {
			return;
		}

		BuildWhereClause(strWhereClause);

		// (c.haag 2008-06-11 12:03) - PLID 30321 - Use new TodoAssignToT structure
		TodoTransferAssignTo(nTransferFromID, nTransferToID, strWhereClause);

		// network message that the todolist changed
		CClient::RefreshTable(NetUtils::TodoList, -1);

		// audit the events
		CString strOldUser = VarString(m_dlTransferFromUserCombo->GetValue(m_dlTransferFromUserCombo->GetCurSel(), 1));
		CString strNewUser = VarString(m_dlTransferToUserCombo->GetValue(m_dlTransferToUserCombo->GetCurSel(), 1));
		CString strName;
		long nPatientID = -1;
		if(IsDlgButtonChecked(IDC_SINGLE_PATIENT_RADIO)){
			long nCurSel = m_dlForPatientCombo->GetCurSel();
			CString strLast = VarString(m_dlForPatientCombo->GetValue(nCurSel, 2));
			CString strFirst = VarString(m_dlForPatientCombo->GetValue(nCurSel, 3));
			strName.Format("%s, %s", strLast, strFirst);

			nPatientID = VarLong(m_dlForPatientCombo->GetValue(nCurSel, 0), -1);
		}
		else{
			strName = "For All Patients";
		}
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(nPatientID, strName, nAuditID, aeiTodosTransferred,-1, strOldUser, strNewUser, aepMedium, aetChanged);

		MsgBox("To-Do Transfer successful!");
		
	}NxCatchAll("Error transfering todo's");
}

void CTransferTodosDlg::OnSinglePatientRadio() 
{
	GetDlgItem(IDC_FOR_PATIENT_COMBO)->EnableWindow(TRUE);
}

void CTransferTodosDlg::OnAllPatientsRadio() 
{
	GetDlgItem(IDC_FOR_PATIENT_COMBO)->EnableWindow(FALSE);
}

void CTransferTodosDlg::BuildWhereClause(CString &strWhere)
{
	// (c.haag 2008-06-10 13:36) - PLID 30321 - We don't need to filter on the Transfer From user; that
	// is now done by the caller
	if(IsDlgButtonChecked(IDC_SINGLE_PATIENT_RADIO)){
		long nPatientID = m_dlForPatientCombo->GetValue(m_dlForPatientCombo->GetCurSel(), 0);
		CString strToAdd;
		strToAdd.Format("PersonID = %li", nPatientID);
		strWhere += strToAdd;
	}
	
	if(IsDlgButtonChecked(IDC_UNFINISHED)){
		if (strWhere.IsEmpty()) {
			strWhere += "Done IS NULL";
		} else {
			strWhere += " AND Done IS NULL";
		}
	}

	if (strWhere.IsEmpty()) {
		strWhere = "(1=1)";
	}
}

BOOL CTransferTodosDlg::ConfirmUserPermissions()
{
	//check the permissions
	//1)  If this user is selected in either dropdown, they must have
	//	permissions for "Self" and "Write"
	//2)  If any other user is selected in either dropdown, they must have 
	//	permissions for "Other" and "Write"
	bool bNeedSelf = false, bNeedOther = false;
	CPermissions permsSelf = GetCurrentUserPermissions(bioSelfFollowUps);
	CPermissions permsOther = GetCurrentUserPermissions(bioNonSelfFollowUps);

	//get the id's we're using
	long nToID = VarLong(m_dlTransferToUserCombo->GetValue(m_dlTransferToUserCombo->GetCurSel(), 0));
	long nFromID = VarLong(m_dlTransferFromUserCombo->GetValue(m_dlTransferFromUserCombo->GetCurSel(), 0));

	//1)  check for self
	if(nToID == GetCurrentUserID() || nFromID == GetCurrentUserID()) {
		bNeedSelf = true;
	}

	//2)  check for other
	if(nToID != GetCurrentUserID() || nFromID != GetCurrentUserID()) {
		//at least one of the users is not us
		bNeedOther = true;
	}

	//for password checking
	bool bBeenPrompted = false;
	bool bNeedSelfPrompt = false, bNeedOtherPrompt = false;

	if(bNeedSelf) {
		if(permsSelf.nPermissions & sptWriteWithPass) {
			//we need self, and they require a password, so ask them
			bNeedSelfPrompt = true;
		}
		else if(permsSelf.nPermissions & sptWrite) {
			//they have write permission, we're a-ok
		}
		else {
			//they do not have write or write with pass permission, quit out
			//this is guaranteed to fail, but it handles giving the nice message 
			//box and everything, so we'll use it
			CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite);
			return FALSE;
		}
	}

	if(bNeedOther) {
		//this is pretty much guaranteed unless they're moving items from themself to themself...
		if(permsOther.nPermissions & sptWriteWithPass) {
			bNeedOtherPrompt = true;
		}
		else if(permsOther.nPermissions & sptWrite) {
			//they have write permission, hooray
		}
		else {
			//they do not have write or write w/ pass permission, so quit,
			//but do it nicely!
			CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite);
			return FALSE;
		}
	}

	//after all this, we have 3 possible states
	//1)  They have full permissions, and can proceed.  bNeedSelfPrompt and bNeedOtherPrompt are both false
	//2)  They have no permission for one of the required items.  In this case the code will not even reach 
	//		this point, and will have already returned.
	//3)  They need to be prompted for at least 1, but maybe both, of the permissions.  The bNeed... flags 
	//		will be set accordingly

	if(bNeedSelfPrompt) {
		if(!CheckCurrentUserPassword()) {
			//failure to enter the correct password will result in explusion!
			return FALSE;
		}

		bBeenPrompted = true;	//they passed the password check
	}

	if(!bBeenPrompted && bNeedOtherPrompt) {
		//they need to be prompted for others, but they haven't yet
		if(!CheckCurrentUserPassword()) {
			return FALSE;
		}

		bBeenPrompted = true;	//they passed the password check
	}


	//If we are still here, then we've successfully passed all permission checks, and can proceed
	//merrily on the way to grandmother's house
	return TRUE;
}
