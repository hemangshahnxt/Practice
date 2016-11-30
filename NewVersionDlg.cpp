// NewVersionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewVersionDlg.h"
#include "DateTimeUtils.h"
#include "GlobalDataUtils.h"
#include "versioninfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CNewVersionDlg dialog


CNewVersionDlg::CNewVersionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewVersionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewVersionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNewVersionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewVersionDlg)
	DDX_Control(pDX, IDC_NEW_VERSION, m_btnNewVersion);
	DDX_Control(pDX, IDC_DELETE_VERSION, m_btnDeleteVersion);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_VERSION_NUMBER, m_nxeditVersionNumber);
	DDX_Control(pDX, IDC_AMA_VERSION, m_nxeditAmaVersion);
	DDX_Control(pDX, IDC_MSI_VERSION, m_nxeditMsiVersion);
	DDX_Control(pDX, IDC_LICENSE_VERSION, m_nxeditLicenseVersion);
	DDX_Control(pDX, IDC_SCOPE_VERSION, m_nxeditScopeVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewVersionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNewVersionDlg)
	ON_EN_KILLFOCUS(IDC_VERSION_NUMBER, OnKillfocusVersionNumber)
	ON_EN_KILLFOCUS(IDC_AMA_VERSION, OnKillfocusAmaVersion)
	ON_BN_CLICKED(IDC_NEW_VERSION, OnNewVersion)
	ON_BN_CLICKED(IDC_DELETE_VERSION, OnDeleteVersion)
	ON_EN_KILLFOCUS(IDC_MSI_VERSION, OnKillfocusMsiVersion)
	ON_EN_KILLFOCUS(IDC_LICENSE_VERSION, OnKillfocusLicenseVersion)
	ON_EN_KILLFOCUS(IDC_SCOPE_VERSION, OnKillfocusScopeVersion)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewVersionDlg message handlers

BOOL CNewVersionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnNewVersion.AutoSet(NXB_NEW);
		m_btnDeleteVersion.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_pVersionList = BindNxDataListCtrl(this, IDC_VERSION_LIST, GetRemoteData(), true);
		m_pReleaseDate = BindNxTimeCtrl(this, IDC_RELEASE_DATE);

		m_pVersionList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_pVersionList->PutCurSel(0);
		OnSelChosenVersionList(0);
	} NxCatchAll("Error in OnInitDialog");

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewVersionDlg::OnOK() 
{
	CNxDialog::OnOK();
}

void CNewVersionDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CNewVersionDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewVersionDlg)
	ON_EVENT(CNewVersionDlg, IDC_RELEASE_DATE, 1 /* KillFocus */, OnKillFocusReleaseDate, VTS_NONE)
	ON_EVENT(CNewVersionDlg, IDC_VERSION_LIST, 16 /* SelChosen */, OnSelChosenVersionList, VTS_I4)
	ON_EVENT(CNewVersionDlg, IDC_VERSION_LIST, 1 /* SelChanging */, OnSelChangingVersionList, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNewVersionDlg::OnKillFocusReleaseDate() 
{
	try {
		//Ensure something is selected
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("You must select a version to edit.");
			return;
		}

		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		CString strDate;
		COleDateTime dt;
		dt = m_pReleaseDate->GetDateTime();

		if(dt.GetStatus() != COleDateTime::valid) 
			strDate = "NULL";
		else
			strDate = "'" + FormatDateTimeForSql(dt, dtoDate) + "'";

		//Save to data
		ExecuteSql("UPDATE ReleasedVersionsT SET ReleaseDate = %s WHERE ID = %li", strDate, nID);

		//update the datalist
		_variant_t var;
		var.vt = VT_DATE;
		var.date = dt;
		m_pVersionList->PutValue(nCurSel, 3, var);

	} NxCatchAll("Error in OnKillfocusReleaseDate");
}

void CNewVersionDlg::OnKillfocusVersionNumber() 
{
	try {
		//Ensure something is selected
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("You must select a version to edit.");
			return;
		}

		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		CString strNumber;
		GetDlgItemText(IDC_VERSION_NUMBER, strNumber);
		strNumber.TrimRight();

		//Make sure this is valid
		if(strNumber.IsEmpty()) {
			MsgBox("You must fill in a version.");
			GetDlgItem(IDC_VERSION_NUMBER)->SetFocus();
			return;
		}

		//Ensure it doesn't already exist
		if(ReturnsRecords("SELECT ID FROM ReleasedVersionsT WHERE Version = '%s' AND ID <> %li", strNumber, nID)) {
			AfxMessageBox("This version already exists.  You cannot duplicate versions.");
			return;
		}

		//Save to data
		ExecuteSql("UPDATE ReleasedVersionsT SET Version = '%s' WHERE ID = %li", strNumber, nID);

		//update the datalist
		m_pVersionList->PutValue(nCurSel, 1, _bstr_t(strNumber));

	} NxCatchAll("Error in OnKillfocusVersionNumber");
}

void CNewVersionDlg::OnKillfocusAmaVersion() 
{
	try {
		//Ensure something is selected
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("You must select a version to edit.");
			return;
		}

		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		CString strNumber;
		GetDlgItemText(IDC_AMA_VERSION, strNumber);
		strNumber.TrimRight();

		//this can be blank or already be in use

		//Save to data
		ExecuteSql("UPDATE ReleasedVersionsT SET AMAVersion = '%s' WHERE ID = %li", strNumber, nID);

		//update the datalist
		m_pVersionList->PutValue(nCurSel, 2, _bstr_t(strNumber));

	} NxCatchAll("Error in OnKillfocusAmaVersion");
}

void CNewVersionDlg::OnSelChosenVersionList(long nRow) 
{
	try {
		//Won't handle no row
		if(nRow == sriNoRow) {
			m_pVersionList->PutCurSel(0);
			nRow = 0;
		}

		//get the ID
		long nID = VarLong(m_pVersionList->GetValue(nRow, 0));

		//Load the values
		_RecordsetPtr prs = CreateRecordset("SELECT Version, AMAVersion, ReleaseDate, MsiVersion, LicenseVersion, ScopeVersion "
			"FROM ReleasedVersionsT WHERE ID = %li", nID);
		if(!prs->eof) {
			COleDateTime dtInvalid;	dtInvalid.SetStatus(COleDateTime::invalid);
			CString strVer = AdoFldString(prs, "Version", "");
			CString strAma = AdoFldString(prs, "AMAVersion", "");
			COleDateTime dt = AdoFldDateTime(prs, "ReleaseDate", dtInvalid);
			CString strMsiVer = AdoFldString(prs, "MsiVersion", "");
			long nLicenseVersion = AdoFldLong(prs, "LicenseVersion", -1);
			long nScopeVersion = AdoFldLong(prs, "ScopeVersion", -1);

			SetDlgItemText(IDC_VERSION_NUMBER, strVer);
			SetDlgItemText(IDC_AMA_VERSION, strAma);
			if(dt.GetStatus() == COleDateTime::valid)
				m_pReleaseDate->SetDateTime(dt);
			else
				m_pReleaseDate->Clear();
			SetDlgItemText(IDC_MSI_VERSION, strMsiVer);

			if (nLicenseVersion != -1) {
				SetDlgItemInt(IDC_LICENSE_VERSION, nLicenseVersion);
			}
			else {
				SetDlgItemText(IDC_LICENSE_VERSION, "");
			}

			if(nScopeVersion >= 0) {
				SetDlgItemInt(IDC_SCOPE_VERSION, nScopeVersion);
			}
			else {
				SetDlgItemText(IDC_SCOPE_VERSION, "");
			}
		}

	} NxCatchAll("Error in OnSelChosenVersionList");
}

void CNewVersionDlg::OnSelChangingVersionList(long FAR* nNewSel) 
{
	try {
		//If they try to select nothing, set it to the first item
		if(*nNewSel == sriNoRow)
			m_pVersionList->PutCurSel(0);

	} NxCatchAll("Error in OnSelChangingVersionList");
}

void CNewVersionDlg::OnNewVersion() 
{
	// (d.thompson 2009-08-18) - PLID 35258 - Disabled this until the upgrade center is live.  I don't think this is going to be
	//	necessary anymore.
	AfxMessageBox("This has been disabled for now.  Please see a release generator.");
	return;

	try {
		CString strResult;
		int nRes = IDCANCEL;
		do {
			nRes = InputBoxLimited(this, "Enter release version", strResult, "",50,false,false,NULL);
			if(nRes == IDCANCEL)
				return;

			strResult.TrimRight();

		} while(!(nRes == IDOK && !strResult.IsEmpty()));

		//Insert into the data
		//DRT 11/12/2008 - PLID 32012 - Fixed so this actually works by adding the fields we are inserting to.
		long nNewID = NewNumber("ReleasedVersionsT", "ID");
		ExecuteSql("INSERT INTO ReleasedVersionsT (ID, Version, AMAVersion, ReleaseDate, Notes, MSIVersion, LicenseVersion, ScopeVersion) "
			"values (%li, '%s', '', GetDate(), '', '', %li, NULL)", nNewID, strResult, LICENSE_VERSION);

		//Create a new row for the datalist
		IRowSettingsPtr pRow = m_pVersionList->GetRow(sriNoRow);
		pRow->PutValue(0, (long)nNewID);
		pRow->PutValue(1, _bstr_t(strResult));
		pRow->PutValue(2, _bstr_t(""));
		pRow->PutValue(3, _variant_t(COleDateTime::GetCurrentTime()));
		pRow->PutValue(5, _variant_t((long)LICENSE_VERSION));
		pRow->PutValue(6, (long)0);
		long nRow = m_pVersionList->AddRow(pRow);

		//now select that row
		m_pVersionList->PutCurSel(nRow);
		OnSelChosenVersionList(nRow);

	} NxCatchAll("Error in OnNewVersion");
}

void CNewVersionDlg::OnDeleteVersion() 
{
	// (d.thompson 2009-08-18) - PLID 35258 - Disabled this until the upgrade center is live.  I don't think this is going to be
	//	necessary anymore.
	AfxMessageBox("This has been disabled for now.  Please see a release generator.");
	return;

	try {
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("You must select a release to delete.");
			return;
		}

		//get the id
		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		//If it's in use, they can't delete it
		if(ReturnsRecords("SELECT TOP 1 PersonID FROM NxClientsT WHERE VersionCurrent = %li", nID)) {
			AfxMessageBox("You may not delete a version that is in use.");
			return;
		}

		//Safe to delete
		if(AfxMessageBox("Are you SURE you wish to delete this version?", MB_YESNO) != IDYES)
			return;

		//Delete it
		ExecuteSql("DELETE FROM ReleasedVersionsT WHERE ID = %li", nID);

		//get rid of the row
		m_pVersionList->RemoveRow(nCurSel);

		//set the sel to first item
		m_pVersionList->PutCurSel(0);
		OnSelChosenVersionList(0);

	} NxCatchAll("Error in OnDeleteVersion");
}

void CNewVersionDlg::OnKillfocusMsiVersion() 
{
	try {
		//Ensure something is selected
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("You must select a version to edit.");
			return;
		}

		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		CString strVersion;
		GetDlgItemText(IDC_MSI_VERSION, strVersion);

		//Save to data
		ExecuteSql("UPDATE ReleasedVersionsT SET MsiVersion = '%s' WHERE ID = %li", _Q(strVersion), nID);

		//update the datalist
		m_pVersionList->PutValue(nCurSel, 4, _bstr_t(strVersion));
	} NxCatchAll("Error in OnKillFocusMsiVersion()");
}

void CNewVersionDlg::OnKillfocusLicenseVersion() 
{
	try {
		//Ensure something is selected
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("You must select a version to edit.");
			return;
		}

		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		long nVersion;
		nVersion = GetDlgItemInt(IDC_LICENSE_VERSION);

		//Save to data
		ExecuteSql("UPDATE ReleasedVersionsT SET LicenseVersion = '%li' WHERE ID = %li", nVersion, nID);

		//update the datalist
		m_pVersionList->PutValue(nCurSel, 5, nVersion);
		
	} NxCatchAll("Error in OnKillFocusLicenseVersion()");


	
}

void CNewVersionDlg::OnKillfocusScopeVersion() 
{
	try {
		//ensure selected
		long nCurSel = m_pVersionList->GetCurSel();
		if(nCurSel == sriNoRow) {
			AfxMessageBox("you must select a version to edit.");
			SetDlgItemText(IDC_SCOPE_VERSION, "");
			return;
		}

		long nID = VarLong(m_pVersionList->GetValue(nCurSel, 0));

		//numeric only box
		CString strScope;
		GetDlgItemText(IDC_SCOPE_VERSION, strScope);
		long nScope = atoi(strScope);

		//ensure numeric
		CString strTest;
		strTest.Format("%li", nScope);

		if(strScope != strTest) {
			strScope = "";
		}

		if(strScope.IsEmpty()) {
			strScope = "NULL";
		}

		//save it
		ExecuteSql("UPDATE ReleasedVersionsT SET ScopeVersion = %s WHERE ID = %li", strScope, nID);

		//update datalist
		if(strScope != "NULL") {
			m_pVersionList->PutValue(nCurSel, 6, nScope);
		}
		else {
			_variant_t varNull;
			varNull.vt = VT_NULL;
			m_pVersionList->PutValue(nCurSel, 6, varNull);
		}

	} NxCatchAll("Error in OnKillfocusScopeVersion");
}
