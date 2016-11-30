// OHIPSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OHIPSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

// (j.jones 2006-11-09 12:53) - PLID 21570 - created

/////////////////////////////////////////////////////////////////////////////
// COHIPSetupDlg dialog


COHIPSetupDlg::COHIPSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COHIPSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COHIPSetupDlg)
		m_nHealthNumberCustomField = 1;
		m_bHealthNumberCustomFieldChanged = FALSE;
	//}}AFX_DATA_INIT
}


void COHIPSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COHIPSetupDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_MOH_OFFICE_CODE, m_nxeditEditMohOfficeCode);
	DDX_Control(pDX, IDC_EDIT_GROUP_NUMBER, m_nxeditEditGroupNumber);
	//DDX_Control(pDX, IDC_EDIT_SPECIALTY, m_nxeditEditSpecialty);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COHIPSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(COHIPSetupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COHIPSetupDlg message handlers

BOOL COHIPSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 10:17) - PLID 29953 - added nxiconbuttons for modernization
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_HealthNumberCombo = BindNxDataList2Ctrl(this, IDC_OHIP_HEALTH_NUM_COMBO, GetRemoteData(), true);
	m_VersionCodeCombo = BindNxDataList2Ctrl(this, IDC_OHIP_VERSION_CODE_COMBO, GetRemoteData(), true);
	m_RegistrationNumberCombo = BindNxDataList2Ctrl(this, IDC_OHIP_REGISTRATION_NUMBER_COMBO, GetRemoteData(), true);
	
	CString strMOHOfficeCode = GetRemotePropertyText("OHIP_MOHOfficeCode", "N", 0, "<None>", true);
	SetDlgItemText(IDC_EDIT_MOH_OFFICE_CODE, strMOHOfficeCode);

	CString strGroupNumber = GetRemotePropertyText("OHIP_GroupNumber", "0000", 0, "<None>", true);
	SetDlgItemText(IDC_EDIT_GROUP_NUMBER, strGroupNumber);
	
	// (j.jones 2009-06-26 09:13) - PLID 34292 - moved Specialty to Contacts module
	//CString strSpecialty = GetRemotePropertyText("OHIP_Specialty", "08", 0, "<None>", true);
	//SetDlgItemText(IDC_EDIT_SPECIALTY, strSpecialty);

	// (j.jones 2010-05-04 11:41) - PLID 32325 - we now cache this field
	m_nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
	m_HealthNumberCombo->SetSelByColumn(0, m_nHealthNumberCustomField);

	long nVersionCodeCustomField = GetRemotePropertyInt("OHIP_VersionCodeCustomField", 2, 0, "<None>", true);
	m_VersionCodeCombo->SetSelByColumn(0, nVersionCodeCustomField);

	long nRegistrationNumberCustomField = GetRemotePropertyInt("OHIP_RegistrationNumberCustomField", 3, 0, "<None>", true);
	m_RegistrationNumberCombo->SetSelByColumn(0, nRegistrationNumberCustomField);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COHIPSetupDlg::OnOK() 
{
	IRowSettingsPtr pHealthNumberRow = m_HealthNumberCombo->GetCurSel();
	if(pHealthNumberRow == NULL) {
		AfxMessageBox("A value must be selected for the Health Number.");
		return;
	}

	IRowSettingsPtr pVersionCodeRow = m_VersionCodeCombo->GetCurSel();
	if(pVersionCodeRow == NULL) {
		AfxMessageBox("A value must be selected for the Version Code.");
		return;
	}

	IRowSettingsPtr pRegistrationNumberRow = m_RegistrationNumberCombo->GetCurSel();
	if(pRegistrationNumberRow == NULL) {
		AfxMessageBox("A value must be selected for the Registration Number.");
		return;
	}

	CString strMOHOfficeCode;
	GetDlgItemText(IDC_EDIT_MOH_OFFICE_CODE, strMOHOfficeCode);
	SetRemotePropertyText("OHIP_MOHOfficeCode", strMOHOfficeCode, 0, "<None>");

	CString strGroupNumber;
	GetDlgItemText(IDC_EDIT_GROUP_NUMBER, strGroupNumber);
	SetRemotePropertyText("OHIP_GroupNumber", strGroupNumber, 0, "<None>");
	
	// (j.jones 2009-06-26 09:13) - PLID 34292 - moved Specialty to Contacts module
	//CString strSpecialty;
	//GetDlgItemText(IDC_EDIT_SPECIALTY, strSpecialty);
	//SetRemotePropertyText("OHIP_Specialty", strSpecialty, 0, "<None>");
	
	long nHealthNumberCustomField = VarLong(pHealthNumberRow->GetValue(0), -1);
	SetRemotePropertyInt("OHIP_HealthNumberCustomField", nHealthNumberCustomField, 0, "<None>");

	// (j.jones 2010-05-04 11:42) - PLID 32325 - did this field change?
	if(m_nHealthNumberCustomField != nHealthNumberCustomField) {
		m_bHealthNumberCustomFieldChanged = TRUE;
		m_nHealthNumberCustomField = nHealthNumberCustomField;
	}

	long nVersionCodeCustomField = VarLong(pVersionCodeRow->GetValue(0), -1);
	SetRemotePropertyInt("OHIP_VersionCodeCustomField", nVersionCodeCustomField, 0, "<None>");

	long nRegistrationNumberCustomField = VarLong(pRegistrationNumberRow->GetValue(0), -1);
	SetRemotePropertyInt("OHIP_RegistrationNumberCustomField", nRegistrationNumberCustomField, 0, "<None>");

	CDialog::OnOK();
}
