// DictationEditInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "DictationEditInfoDlg.h"


// CDictationEditInfoDlg dialog
// (z.manning 2015-12-18 10:06) - PLID 67738 - Created

IMPLEMENT_DYNAMIC(CDictationEditInfoDlg, CNxDialog)

CDictationEditInfoDlg::CDictationEditInfoDlg(long nLicenseKey, CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_EDIT_DICTATION_INFO, pParent)
{
	m_nLicenseKey = nLicenseKey;
}

CDictationEditInfoDlg::~CDictationEditInfoDlg()
{
}

void CDictationEditInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CDictationEditInfoDlg, CNxDialog)
END_MESSAGE_MAP()


// CDictationEditInfoDlg message handlers

BOOL CDictationEditInfoDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		ADODB::_RecordsetPtr prsLoad = CreateParamRecordset(R"(
SELECT C.NuanceOrganizationGuid, C.NuanceLicenseGuid
FROM NxClientsT C
WHERE C.LicenseKey = {INT}
)"
, m_nLicenseKey);
		if (prsLoad->eof) {
			MessageBox(FormatString("Could not find data for license key %li", m_nLicenseKey), "Error", MB_ICONERROR);
			EndDialog(IDCANCEL);
		}

		CString strOrganizationID = AdoFldString(prsLoad, "NuanceOrganizationGuid", "");
		CString strOrganizationToken = AdoFldString(prsLoad, "NuanceLicenseGuid", "");
		strOrganizationID.Trim("{}");
		strOrganizationToken.Trim("{}");
		SetDlgItemText(IDC_NUANCE_ORGANIZATION_ID, strOrganizationID);
		SetDlgItemText(IDC_NUANCE_ORGANIZATION_TOKEN, strOrganizationToken);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CDictationEditInfoDlg::OnOK()
{
	try
	{
		CString strOrganizationID, strOrganizationToken;
		GetDlgItemText(IDC_NUANCE_ORGANIZATION_ID, strOrganizationID);
		GetDlgItemText(IDC_NUANCE_ORGANIZATION_TOKEN, strOrganizationToken);

		GUID guidOrgID, guidOrgToken;
		if (!ParseGUID(strOrganizationID, guidOrgID)) {
			MessageBox("Please enter a valid GUID for organization ID.", "Invalid GUID", MB_ICONERROR);
			return;
		}
		if (!ParseGUID(strOrganizationToken, guidOrgToken)) {
			MessageBox("Please enter a valid GUID for organization token.", "Invalid GUID", MB_ICONERROR);
			return;
		}

		ExecuteParamSql(R"(
UPDATE NxClientsT SET NuanceOrganizationGuid = {STRING}
	, NuanceLicenseGuid = {STRING}
WHERE LicenseKey = {INT}
)"
, AsGUIDString(guidOrgID), AsGUIDString(guidOrgToken), m_nLicenseKey);

		__super::OnOK(); 
	}
	NxCatchAll(__FUNCTION__);
}
