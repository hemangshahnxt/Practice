// Address.cpp : implementation file
//

#include "stdafx.h"
//#include "Palmexe.h"
#include "Address.h"

#include "globalutils.h"

using namespace ADODB;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddress dialog


CAddress::CAddress(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAddress::IDD, pParent)
{
	

	//{{AFX_DATA_INIT(CAddress)
	m_nPhone1 = -1;
	m_nPhone2 = -1;
	m_nPhone5 = -1;
	m_nPhone4 = -1;
	m_nPhone3 = -1;
	m_bDoctors = FALSE;
	m_bOther = FALSE;
	m_bReferring = FALSE;
	m_bEmployees = FALSE;
	m_bSuppliers = FALSE;
	m_bPrimary = FALSE;
	m_bAllowRestoreSyncImports = FALSE;
	m_nPUserID = 0;
	m_iDuplicateHandling = -1;
	//}}AFX_DATA_INIT
}


void CAddress::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddress)
	DDX_Control(pDX, IDC_RADIO_PALMDUP_ADDCONTACTS, m_btnRadAddContacts);
	DDX_Control(pDX, IDC_RADIO_PALMDUP_MERGEMATCHING, m_btnRadDupMatch);
	DDX_Control(pDX, IDC_RADIO_PALMDUP_MERGE, m_btnRadDupMerge);
	DDX_Control(pDX, IDC_CHECK_RESTOREIMPORT, m_chkAllowRestoreSyncImports);
	DDX_Control(pDX, IDC_CHECK_SUPPLIERS, m_chkSuppliers);
	DDX_Control(pDX, IDC_PALM_CHECK_EMPLOYEE, m_chkEmployees);
	DDX_Control(pDX, IDC_PALM_CHECK_OTHER, m_chkOther);
	DDX_Control(pDX, IDC_CHECK_REFERRING, m_chkReferring);
	DDX_Control(pDX, IDC_CHECK_DOCTORS, m_chkDoctors);
	DDX_Control(pDX, IDC_COMBO_PHONE5, m_cmbPhone5);
	DDX_Control(pDX, IDC_COMBO_PHONE4, m_cmbPhone4);
	DDX_Control(pDX, IDC_COMBO_PHONE3, m_cmbPhone3);
	DDX_Control(pDX, IDC_COMBO_PHONE2, m_cmbPhone2);
	DDX_Control(pDX, IDC_COMBO_PHONE1, m_cmbPhone1);
	DDX_CBIndex(pDX, IDC_COMBO_PHONE1, m_nPhone1);
	DDX_CBIndex(pDX, IDC_COMBO_PHONE2, m_nPhone2);
	DDX_CBIndex(pDX, IDC_COMBO_PHONE5, m_nPhone5);
	DDX_CBIndex(pDX, IDC_COMBO_PHONE4, m_nPhone4);
	DDX_CBIndex(pDX, IDC_COMBO_PHONE3, m_nPhone3);
	DDX_Check(pDX, IDC_CHECK_DOCTORS, m_bDoctors);
	DDX_Check(pDX, IDC_PALM_CHECK_OTHER, m_bOther);
	DDX_Check(pDX, IDC_CHECK_REFERRING, m_bReferring);
	DDX_Check(pDX, IDC_PALM_CHECK_EMPLOYEE, m_bEmployees);
	DDX_Check(pDX, IDC_CHECK_SUPPLIERS, m_bSuppliers);
	DDX_Check(pDX, IDC_CHECK_RESTOREIMPORT, m_bAllowRestoreSyncImports);
	DDX_Radio(pDX, IDC_RADIO_PALMDUP_ADDCONTACTS, m_iDuplicateHandling);
	DDX_Control(pDX, IDC_CATEGORIES_GROUPBOX, m_btnCategoriesGroupbox);
	DDX_Control(pDX, IDC_FIELDS_GROUPBOX, m_btnFieldsGroupbox);
	DDX_Control(pDX, IDC_DUPLICATE_GROUPBOX, m_btnDuplicateGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddress, CNxDialog)
	//{{AFX_MSG_MAP(CAddress)
	ON_CBN_SELCHANGE(IDC_COMBO_PHONE1, OnSelchangeComboPhone1)
	ON_CBN_SELCHANGE(IDC_COMBO_PHONE2, OnSelchangeComboPhone2)
	ON_CBN_SELCHANGE(IDC_COMBO_PHONE3, OnSelchangeComboPhone3)
	ON_CBN_SELCHANGE(IDC_COMBO_PHONE4, OnSelchangeComboPhone4)
	ON_CBN_SELCHANGE(IDC_COMBO_PHONE5, OnSelchangeComboPhone5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddress message handlers



BOOL CAddress::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	InitDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddress::InitDlg()
{
	UpdateData(FALSE);	
	
	// DO THE DATETIME PICKERS
	UpdateData(TRUE);	
}

void CAddress::OnSelchangeComboPhone1() 
{
	// Updatedata
	UpdateData(TRUE);

	CString temp;

	// get the value for error checking
	m_cmbPhone1.GetLBText(m_nPhone1, temp);

	
	if ((m_nPhone1 != 0) && ((m_nPhone1 == m_nPhone2) || (m_nPhone1 == m_nPhone3) || (m_nPhone1 == m_nPhone4) || 
		(m_nPhone1 == m_nPhone5))) 
	{
		// message box, baby
		m_cmbPhone1.MessageBox("Invalid choice, try again", "NexTech Error");

		// set it back to none
		m_cmbPhone1.SetCurSel(0);
		
		// byebye
		return;
	}

	// update the data
	UpdateData(FALSE);
}

void CAddress::OnSelchangeComboPhone2() 
{
	// Updatedata
	UpdateData(TRUE);

	CString temp;

	// get the value for error checking
	m_cmbPhone2.GetLBText(m_nPhone2, temp);

	
	if ((m_nPhone2 != 0) && ((m_nPhone2 == m_nPhone1) || (m_nPhone2 == m_nPhone3) || (m_nPhone2 == m_nPhone4) || 
		(m_nPhone2 == m_nPhone5))) 
	{
		// message box, baby
		m_cmbPhone2.MessageBox("Invalid choice, try again", "NexTech Error");

		// set it back to none
		m_cmbPhone2.SetCurSel(0);
		
		// byebye
		return;
	}

	// update the data
	UpdateData(FALSE);
}

void CAddress::OnSelchangeComboPhone3() 
{
	// Updatedata
	UpdateData(TRUE);

	CString temp;

	// get the value for error checking
	m_cmbPhone3.GetLBText(m_nPhone3, temp);

	
	if ((m_nPhone3 != 0) && ((m_nPhone3 == m_nPhone2) || (m_nPhone3 == m_nPhone1) || (m_nPhone3 == m_nPhone4) || 
		(m_nPhone3 == m_nPhone5))) 
	{
		// message box, baby
		m_cmbPhone3.MessageBox("Invalid choice, try again", "NexTech Error");

		// set it back to none
		m_cmbPhone3.SetCurSel(0);
		
		// byebye
		return;
	}

	// update the data
	UpdateData(FALSE);
}

void CAddress::OnSelchangeComboPhone4() 
{
	// Updatedata
	UpdateData(TRUE);

	CString temp;

	// get the value for error checking
	m_cmbPhone4.GetLBText(m_nPhone4, temp);

	
	if ((m_nPhone4 != 0) && ((m_nPhone4 == m_nPhone2) || (m_nPhone4 == m_nPhone3) || (m_nPhone4 == m_nPhone1) || 
		(m_nPhone4 == m_nPhone5))) 
	{
		// message box, baby
		m_cmbPhone4.MessageBox("Invalid choice, try again", "NexTech Error");

		// set it back to none
		m_cmbPhone4.SetCurSel(0);
		
		// byebye
		return;
	}

	// update the data
	UpdateData(FALSE);
}

void CAddress::OnSelchangeComboPhone5() 
{
	// Updatedata
	UpdateData(TRUE);

	CString temp;

	// get the value for error checking
	m_cmbPhone5.GetLBText(m_nPhone5, temp);

	
	if ((m_nPhone5 != 0) && ((m_nPhone5 == m_nPhone2) || (m_nPhone5 == m_nPhone3) || (m_nPhone5 == m_nPhone4) || 
		(m_nPhone5 == m_nPhone1))) 
	{
		// message box, baby
		m_cmbPhone5.MessageBox("Invalid choice, try again", "NexTech Error");

		// set it back to none
		m_cmbPhone5.SetCurSel(0);
		
		// byebye
		return;
	}

	// update the data
	UpdateData(FALSE);
}


void CAddress::ItemChange()
{
	UpdateData(TRUE);

	char strSQL[4096];
	CString strTemp;
	_variant_t Temp;
	

	sprintf(strSQL, "SELECT PalmSettingsT.Phone1, PalmSettingsT.Phone2, PalmSettingsT.Phone3, "
		"PalmSettingsT.Phone4, PalmSettingsT.Phone5, PalmSettingsT.Doctors, PalmSettingsT.Referring, "
		"PalmSettingsT.Employees, PalmSettingsT.Other, PalmSettingsT.Suppliers, PalmSettingsT.SyncContacts, PalmSettingsT.AllowRestoreSyncContactImports, "
		"PalmSettingsT.DupContactBehavior "
		"FROM PalmSettingsT WHERE ID = %d", GetUserID());

	try {
		m_rs = CreateRecordset(strSQL);



		// do the phone ID boxes first
		Temp = m_rs->Fields->GetItem("Phone1")->Value;
		m_cmbPhone1.SetCurSel(Temp.lVal);

		Temp = m_rs->Fields->GetItem("Phone2")->Value;
		m_cmbPhone2.SetCurSel(Temp.lVal);

		Temp = m_rs->Fields->GetItem("Phone3")->Value;
		m_cmbPhone3.SetCurSel(Temp.lVal);
		
		Temp = m_rs->Fields->GetItem("Phone4")->Value;
		m_cmbPhone4.SetCurSel(Temp.lVal);
		
		Temp = m_rs->Fields->GetItem("Phone5")->Value;
		m_cmbPhone5.SetCurSel(Temp.lVal);

		// now do the checkboxes
		Temp = m_rs->Fields->GetItem("Doctors")->Value;
		if (Temp.boolVal)
			m_chkDoctors.SetCheck(1);
		else
			m_chkDoctors.SetCheck(0);
		
		Temp = m_rs->Fields->GetItem("Referring")->Value;
		if (Temp.boolVal)
			m_chkReferring.SetCheck(1);
		else
			m_chkReferring.SetCheck(0);
		
		Temp = m_rs->Fields->GetItem("Employees")->Value;
		if (Temp.boolVal)
			m_chkEmployees.SetCheck(1);
		else
			m_chkEmployees.SetCheck(0);

		Temp = m_rs->Fields->GetItem("Other")->Value;
		if (Temp.boolVal)
			m_chkOther.SetCheck(1);
		else
			m_chkOther.SetCheck(0);

		Temp = m_rs->Fields->GetItem("Suppliers")->Value;
		if (Temp.boolVal)
			m_chkSuppliers.SetCheck(1);
		else
			m_chkSuppliers.SetCheck(0);

		Temp = m_rs->Fields->GetItem("AllowRestoreSyncContactImports")->Value;
		if (Temp.boolVal)
			m_chkAllowRestoreSyncImports.SetCheck(1);
		else
			m_chkAllowRestoreSyncImports.SetCheck(0);

		UpdateData(TRUE);

		m_iDuplicateHandling = VarLong(m_rs->Fields->GetItem("DupContactBehavior")->Value);
		UpdateData(FALSE);

		m_rs->Close();
	}
	NxCatchAll("General SQL Error");

	
	UpdateData(FALSE);

}


unsigned long GetLicense(const unsigned long nNumberOfLicenses) {

	// TODO: this should actually work

	return 5;

}


void CAddress::OK()
{
	UpdateData(TRUE);
	//_variant_t Value;
	//bool bZip = false, bCity = false, bState = false;
	//CString strCity, strState, strZip;
	char strSQL[4096];
	try {
		sprintf(strSQL, "UPDATE PalmSettingsT SET Phone1 = %li, Phone2 = %li, Phone3 = %li, Phone4 = %li, "
				"Phone5 = %li, Doctors = %li, Referring = %li, Employees = %li, Other = %li, Suppliers = %li, "
				"SyncContacts = %li, AllowRestoreSyncContactImports = %li, CategoriesChanged = 1, DupContactBehavior = %d WHERE ID = %li", 
				m_nPhone1, m_nPhone2, m_nPhone3, m_nPhone4, m_nPhone5, (m_bDoctors?1:0), (m_bReferring?1:0),
				(m_bEmployees?1:0), (m_bOther?1:0), (m_bSuppliers?1:0), (m_bPrimary?1:0), (m_bAllowRestoreSyncImports?1:0), m_iDuplicateHandling,
				GetUserID());
#ifdef _DEBUG
		MessageBox(strSQL);
#endif

		ExecuteSql(strSQL);
	}
	NxCatchAll("General SQL Error");	
}

long CAddress::GetUserID()
{
	return m_nPUserID;
}

void CAddress::SetUserID(long ID)
{
	m_nPUserID = ID;
}

void CAddress::OnOK()
{
	//We should pass this on up to our parent.
	CWnd *pParent = GetParent();
	if(pParent && pParent->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)GetParent())->OK();
	}
	else if(pParent->GetParent() && pParent->GetParent()->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)pParent->GetParent())->OK();
	}
}

void CAddress::OnCancel()
{
	//We should pass this on up to our parent.
	CWnd *pParent = GetParent();
	if(pParent && pParent->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)GetParent())->Cancel();
	}
	else if(pParent->GetParent() && pParent->GetParent()->IsKindOf(RUNTIME_CLASS(CPalmDialogDlg))) {
		((CPalmDialogDlg*)pParent->GetParent())->Cancel();
	}
}
