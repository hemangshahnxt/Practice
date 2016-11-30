// ConfigureLicensesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigureLicensesDlg.h"
#include "GlobalDrawingUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CConfigureLicensesDlg dialog


CConfigureLicensesDlg::CConfigureLicensesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureLicensesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureLicensesDlg)
		m_nPersonID = -1;
	//}}AFX_DATA_INIT
}


void CConfigureLicensesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureLicensesDlg)
	DDX_Control(pDX, IDC_EXPIRE_WARN_DAY_EDIT, m_nxeditExpireWarnDayEdit);
	DDX_Control(pDX, IDC_BTN_ADD_LICENSE, m_btnAddLicense);
	DDX_Control(pDX, IDC_BTN_DELETE_LICENSE, m_btnDeleteLicense);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureLicensesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureLicensesDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_LICENSE, OnBtnAddLicense)
	ON_BN_CLICKED(IDC_BTN_DELETE_LICENSE, OnBtnDeleteLicense)
	ON_EN_KILLFOCUS(IDC_EXPIRE_WARN_DAY_EDIT, OnKillfocusExpireWarnDayEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureLicensesDlg message handlers

BOOL CConfigureLicensesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - pLID 29852 - Set button styles
	m_btnAddLicense.AutoSet(NXB_NEW);
	m_btnDeleteLicense.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);

	m_PersonCombo = BindNxDataListCtrl(this,IDC_PROVIDER_LICENSE_COMBO,GetRemoteData(),true);
	m_LicenseList = BindNxDataListCtrl(this,IDC_LICENSE_LIST,GetRemoteData(),false);
	m_RespUserCombo = BindNxDataListCtrl(this,IDC_USER_RESP_LIST,GetRemoteData(),true);

	IRowSettingsPtr pRow = m_RespUserCombo->GetRow(-1);
	pRow->PutValue(0,long(-1));
	pRow->PutValue(1," <No User>");
	m_RespUserCombo->AddRow(pRow);

	//load the default user
	m_RespUserCombo->SetSelByColumn(0,(long)GetRemotePropertyInt("DefaultASCLicenseWarnUser",-1,0,"<None>",true));

	//load the warning day period
	SetDlgItemInt(IDC_EXPIRE_WARN_DAY_EDIT,GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true));
	
	long nRow = -1;
	if(m_nPersonID != -1) {
		//if we were passed in a PersonID, select that person and load their licenses
		nRow = m_PersonCombo->SetSelByColumn(0,m_nPersonID);
	}
	else {
		//otherwise, just load the first entry
		m_PersonCombo->CurSel = 0;
		nRow = 0;
	}
		
	OnSelChosenProviderLicenseCombo(nRow);

	EnableControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CConfigureLicensesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureLicensesDlg)
	ON_EVENT(CConfigureLicensesDlg, IDC_PROVIDER_LICENSE_COMBO, 16 /* SelChosen */, OnSelChosenProviderLicenseCombo, VTS_I4)
	ON_EVENT(CConfigureLicensesDlg, IDC_LICENSE_LIST, 9 /* EditingFinishing */, OnEditingFinishingLicenseList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CConfigureLicensesDlg, IDC_LICENSE_LIST, 10 /* EditingFinished */, OnEditingFinishedLicenseList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConfigureLicensesDlg, IDC_USER_RESP_LIST, 16 /* SelChosen */, OnSelChosenUserRespList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigureLicensesDlg::OnSelChosenProviderLicenseCombo(long nRow) 
{
	try {

		if(nRow == -1) {
			m_nPersonID = -1;
			m_LicenseList->Clear();
			return;
		}

		m_nPersonID = VarLong(m_PersonCombo->GetValue(nRow,0));

		CString str;
		str.Format("PersonID = %li",m_nPersonID);
		m_LicenseList->WhereClause = _bstr_t(str);
		m_LicenseList->Requery();

	}NxCatchAll("Error loading licenses.");
}

void CConfigureLicensesDlg::OnEditingFinishingLicenseList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		if(!CheckCurrentUserPermissions(bioASCLicensing,sptWrite)) {
			*pbCommit = FALSE;
			return;
		}

		if(nCol == 1) { //name

			if (pvarNewValue->vt != VT_BSTR) {
				MsgBox("The text you entered is not valid.");
				*pbCommit = FALSE;
				return;
			}

			CString str = strUserEntered;
			str.TrimRight();
			if(str.IsEmpty()) {
				MsgBox("You cannot save a blank name.");
				*pbCommit = FALSE;
				return;
			}
			else if(!IsRecordsetEmpty("SELECT Name FROM PersonCertificationsT WHERE PersonID = %li AND Name = '%s' AND ID <> %li",m_nPersonID,_Q(str),VarLong(m_LicenseList->GetValue(m_LicenseList->CurSel,0)))) {
				MsgBox("This license name already exists in the list. Please enter a different license name.");
				*pbCommit = FALSE;
				return;
			}
		}
		else if(nCol == 2) { //date

			if (pvarNewValue->vt != VT_DATE) {
				MsgBox("The date you entered is not valid.");
				*pbCommit = FALSE;
				return;
			}

			COleDateTime dtNew = pvarNewValue->date;
			COleDateTime dtBad;
			dtBad.SetDateTime(1800,1,1,0,0,0);

			if (dtNew.m_status == COleDateTime::invalid ||
				dtNew.m_dt == 0.0 || dtNew < dtBad) {
				MsgBox("The date you entered is not valid.");
				*pbCommit = FALSE;
				return;
			}

			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtToday.SetDateTime(dtToday.GetYear(),dtToday.GetMonth(),dtToday.GetDay(),0,0,0);

			if(dtNew < dtToday) {
				if(IDNO == MessageBox("The date you entered is in the past. Are you sure you wish to save this date?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					*pbCommit = FALSE;
					return;
				}
			}
		}

	}NxCatchAll("Error editing license information.");
}

void CConfigureLicensesDlg::OnEditingFinishedLicenseList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if (bCommit) {
			if(nCol == 1) { //name
				ExecuteSql("UPDATE PersonCertificationsT SET Name = '%s' WHERE ID = %li",_Q(VarString(varNewValue)),VarLong(m_LicenseList->GetValue(m_LicenseList->CurSel,0)));
			}
			else if(nCol == 2) { //date
				ExecuteSql("UPDATE PersonCertificationsT SET ExpDate = '%s' WHERE ID = %li",FormatDateTimeForSql(VarDateTime(varNewValue),dtoDate),VarLong(m_LicenseList->GetValue(m_LicenseList->CurSel,0)));
			}

			UpdateASCLicenseToDos();
		}

	}NxCatchAll("Error editing license information.");
}

void CConfigureLicensesDlg::OnBtnAddLicense() 
{
	try {

		if(m_nPersonID == -1) {
			AfxMessageBox("Please select a person to add a license for.");
			return;
		}

		if(!CheckCurrentUserPermissions(bioASCLicensing,sptCreate)) {
			return;
		}

		int nResult;
		CString strNewName, strDesc;

		nResult = InputBoxLimited(this, "Enter New License Name", strNewName, "",255,false,false,NULL);
		strNewName.TrimRight();
		if (nResult != IDOK || strNewName == "")
			return;

		if(!IsRecordsetEmpty("SELECT Name FROM PersonCertificationsT WHERE PersonID = %li AND Name = '%s'",m_nPersonID,_Q(strNewName))) {
			AfxMessageBox("This license already exists in the list. Please enter a different license name.");
			return;
		}

		//add the license, default the date to be one year from today, and start editing the date
		long nID = NewNumber("PersonCertificationsT","ID");
		ExecuteSql("INSERT INTO PersonCertificationsT (ID, PersonID, Name, ExpDate) VALUES (%li, %li, '%s', DateAdd(year,1,CONVERT(datetime,CONVERT(nvarchar, GetDate(), 101))))",
			nID,m_nPersonID,_Q(strNewName));

		m_LicenseList->Requery();
		
		int nRow = m_LicenseList->SetSelByColumn(0,nID);

		//start editing the exp date
		m_LicenseList->StartEditing(nRow,2);

	}NxCatchAll("Error adding new license.");	
}

void CConfigureLicensesDlg::OnBtnDeleteLicense() 
{
	try {

		if(m_LicenseList->CurSel == -1) {
			AfxMessageBox("Please select a license from the list before deleting.");
			return;
		}

		if(!CheckCurrentUserPermissions(bioASCLicensing,sptDelete)) {
			return;
		}

		if(IDNO == MessageBox("Are you sure you wish to delete the selected license?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		ExecuteSql("DELETE FROM PersonCertificationsT WHERE ID = %li",VarLong(m_LicenseList->GetValue(m_LicenseList->CurSel,0)));

		m_LicenseList->Requery();

		UpdateASCLicenseToDos();

	}NxCatchAll("Error deleting license.");
}

void CConfigureLicensesDlg::OnKillfocusExpireWarnDayEdit() 
{
	if(!CheckCurrentUserPermissions(bioASCLicensing,sptWrite)) {
		//load the default warning day period
		SetDlgItemInt(IDC_EXPIRE_WARN_DAY_EDIT,GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true));
		return;
	}

	//save the warning day period
	long nDayRange = 0;
	nDayRange = GetDlgItemInt(IDC_EXPIRE_WARN_DAY_EDIT);
	
	SetRemotePropertyInt("DefaultASCLicenseWarnDayRange",nDayRange,0,"<None>");
}

void CConfigureLicensesDlg::OnSelChosenUserRespList(long nRow) 
{
	if(!CheckCurrentUserPermissions(bioASCLicensing,sptWrite)) {
		//load the default user
		m_RespUserCombo->SetSelByColumn(0,(long)GetRemotePropertyInt("DefaultASCLicenseWarnUser",-1,0,"<None>",true));
		return;
	}

	//save the default user
	if(nRow == -1)
		m_RespUserCombo->SetSelByColumn(0,(long)-1);

	long nUser = VarLong(m_RespUserCombo->GetValue(m_RespUserCombo->CurSel,0),-1);

	SetRemotePropertyInt("DefaultASCLicenseWarnUser",nUser,0,"<None>");
}

void CConfigureLicensesDlg::EnableControls()
{
	BOOL bEnable = (GetCurrentUserPermissions(bioASCLicensing) & (SPT___W________ANDPASS));

	GetDlgItem(IDC_BTN_ADD_LICENSE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_DELETE_LICENSE)->EnableWindow(bEnable);
	((CNxEdit*)GetDlgItem(IDC_EXPIRE_WARN_DAY_EDIT))->SetReadOnly(!bEnable);
	GetDlgItem(IDC_USER_RESP_LIST)->EnableWindow(bEnable);

	m_LicenseList->PutReadOnly(!bEnable);
}
