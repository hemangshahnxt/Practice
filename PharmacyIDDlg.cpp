// PharmacyIDDlg.cpp : implementation file
//

//DRT 11/19/2008 - PLID 32092 - Created

#include "stdafx.h"
#include "Practice.h"
#include "PharmacyIDDlg.h"

using namespace ADODB;


// CPharmacyIDDlg dialog

IMPLEMENT_DYNAMIC(CPharmacyIDDlg, CNxDialog)

CPharmacyIDDlg::CPharmacyIDDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPharmacyIDDlg::IDD, pParent)
{
	m_nPharmacyID = -1;
}

CPharmacyIDDlg::~CPharmacyIDDlg()
{
}

void CPharmacyIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PHARMACY_NAME, m_nxstaticPharmName);
	DDX_Control(pDX, IDC_NCPDPID, m_nxeditNCPDPID);
	DDX_Control(pDX, IDC_FILEID, m_nxeditFileID);
	DDX_Control(pDX, IDC_STATE_LIC_NUM, m_nxeditStateLicNum);
	DDX_Control(pDX, IDC_MEDICARE_NUM, m_nxeditMedicareNum);
	DDX_Control(pDX, IDC_MEDICAID_NUM, m_nxeditMedicaidNum);
	DDX_Control(pDX, IDC_PPO_NUM, m_nxeditPPONum);
	DDX_Control(pDX, IDC_PAYER_ID, m_nxeditPayerID);
	DDX_Control(pDX, IDC_BIN_LOCATION_NUM, m_nxeditBINLocNum);
	DDX_Control(pDX, IDC_DEA_NUM, m_nxeditDEANum);
	DDX_Control(pDX, IDC_HIN, m_nxeditHIN);
	DDX_Control(pDX, IDC_NAIC_CODE, m_nxeditNAICCode);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
}


BEGIN_MESSAGE_MAP(CPharmacyIDDlg, CNxDialog)
END_MESSAGE_MAP()


// CPharmacyIDDlg message handlers

BOOL CPharmacyIDDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//First, setup the label for the name
		m_nxstaticPharmName.SetWindowText(m_strPharmacyName);

		//Configure our buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Then load everything out of data
		LoadFromData();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CPharmacyIDDlg::OnOK()
{
	try {
		//Attempt to save everything back to data, failing if there was an error
		if(!SaveToData()) {
			return;
		}

		//success!
		CDialog::OnOK();

	} NxCatchAll("Error in OnOK");
}

void CPharmacyIDDlg::OnCancel()
{
	//Throw it all away
	CDialog::OnCancel();
}

void CPharmacyIDDlg::LoadFromData()
{
	// (a.walling 2009-03-30 10:19) - PLID 33729 - Get the linked status and save the original NCPDPID
	_RecordsetPtr prs = CreateParamRecordset("SELECT LocationsT.LinkToDirectory, PharmacyIDT.* FROM LocationsT INNER JOIN PharmacyIDT ON LocationsT.ID = PharmacyIDT.LocationID WHERE LocationID = {INT};", m_nPharmacyID);
	if(prs->eof) {
		//This data is only created on save, so it's entirely possible that nothing will exist here.  If so, we don't need
		//	to load anything.
		return;
	}

	//Otherwise we do have data to load.
	// (a.walling 2009-03-30 10:19) - PLID 33729 - Get the linked status and save the original NCPDPID
	m_strOriginalNCPDPID = AdoFldString(prs, "NCPDPID");
	m_varLinkToDirectory = prs->Fields->Item["LinkToDirectory"]->Value;

	m_nxeditNCPDPID.SetWindowText(m_strOriginalNCPDPID);
	m_nxeditFileID.SetWindowText( AdoFldString(prs, "FileID") );
	m_nxeditStateLicNum.SetWindowText( AdoFldString(prs, "StateLicNum") );
	m_nxeditMedicareNum.SetWindowText( AdoFldString(prs, "MedicareNum") );
	m_nxeditMedicaidNum.SetWindowText( AdoFldString(prs, "MedicaidNum") );
	m_nxeditPPONum.SetWindowText( AdoFldString(prs, "PPONum") );
	m_nxeditPayerID.SetWindowText( AdoFldString(prs, "PayerID") );
	m_nxeditBINLocNum.SetWindowText( AdoFldString(prs, "BINLocNum") );
	m_nxeditDEANum.SetWindowText( AdoFldString(prs, "DEANum") );
	m_nxeditHIN.SetWindowText( AdoFldString(prs, "HIN") );
	m_nxeditNAICCode.SetWindowText( AdoFldString(prs, "NAICCode") );

}

//Used to ensure a control is not over the max allowed size.  If it is, a message box will prompt the user, the
//	failing control will get focus, and the IDs will be highlighted
#define ENSURE_MAX_SIZE(ctrl, nSize) { if(ctrl.GetWindowTextLength() > nSize) {  AfxMessageBox( FormatString("IDs may not be longer than %li characters.  Please shorten the ID and try again.", nSize) );  ctrl.SetFocus(); ctrl.SetSel(0, -1); return false; } }

bool CPharmacyIDDlg::SaveToData()
{
	//All of these have a max of 35 characters, fail if we go over
	long nMaxChars = 35;

	//Per control
	ENSURE_MAX_SIZE(m_nxeditNCPDPID, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditFileID, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditStateLicNum, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditMedicareNum, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditMedicaidNum, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditPPONum, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditPayerID, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditBINLocNum, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditDEANum, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditHIN, nMaxChars);
	ENSURE_MAX_SIZE(m_nxeditNAICCode, nMaxChars);


	//First, pull all the interface data to variables
	CString strNCPDPID, strFileID, strStateLicNum, strMedicareNum, strMedicaidNum, strPPONum, strPayerID, strBINLocNum, 
		strDEANum, strHIN, strNAICCode;

	m_nxeditNCPDPID.GetWindowText(strNCPDPID);
	m_nxeditFileID.GetWindowText(strFileID);
	m_nxeditStateLicNum.GetWindowText(strStateLicNum);
	m_nxeditMedicareNum.GetWindowText(strMedicareNum);
	m_nxeditMedicaidNum.GetWindowText(strMedicaidNum);
	m_nxeditPPONum.GetWindowText(strPPONum);
	m_nxeditPayerID.GetWindowText(strPayerID);
	m_nxeditBINLocNum.GetWindowText(strBINLocNum);
	m_nxeditDEANum.GetWindowText(strDEANum);
	m_nxeditHIN.GetWindowText(strHIN);
	m_nxeditNAICCode.GetWindowText(strNAICCode);

	BOOL bUnlinkDirectory = FALSE;
	
	if (g_pLicense->CheckForLicense(CLicense::lcePrescribe, CLicense::cflrSilent)) {
		// (a.walling 2009-03-30 10:19) - PLID 33729 - If they were set to link, and the NCPDPID has changed (and used to be a valid, 7 digit NCPDPID), warn them
		if (m_strOriginalNCPDPID.GetLength() == 7 && m_strOriginalNCPDPID != strNCPDPID && m_varLinkToDirectory.vt == VT_BOOL && VarBool(m_varLinkToDirectory)) {
			if (IDOK != MessageBox(FormatString("This pharmacy has been linked with the official SureScripts pharmacy directory, and the NCPDP number has changed.\r\n\r\n"
				"The NCPDP number is used to identify pharmacies to automatically update and add pharmacy information from the directory. If the new NCPDP number exists in the directory, this pharmacy's information will be updated during the next directory update.\r\n\r\n"
				"Please ensure you want to change the NCPDP number for this pharmacy to '%s' from '%s'.", strNCPDPID, m_strOriginalNCPDPID), NULL, MB_ICONEXCLAMATION|MB_OKCANCEL)) {
				return false;
			}

			bUnlinkDirectory = TRUE;
		}
	}

	//Then, Generate your save string appropriately
	CString strSqlBatch = BeginSqlBatch();
	CNxParamSqlArray args;

	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @LocationID int;\r\n");
	AddParamStatementToSqlBatch(strSqlBatch, args, "SET @LocationID = {INT};\r\n", m_nPharmacyID);
	AddDeclarationToSqlBatch(strSqlBatch, "IF EXISTS (SELECT LocationID FROM PharmacyIDT WHERE LocationID = @LocationID) BEGIN\r\n");

	//Code for an UPDATE statement here
	AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PharmacyIDT SET NCPDPID = {STRING}, FileID = {STRING}, StateLicNum = {STRING}, "
		"MedicareNum = {STRING}, MedicaidNum = {STRING}, PPONum = {STRING}, PayerID = {STRING}, BINLocNum = {STRING}, "
		"DEANum = {STRING}, HIN = {STRING}, NAICCode = {STRING} WHERE LocationID = @LocationID;\r\n", strNCPDPID, strFileID, 
		strStateLicNum, strMedicareNum, strMedicaidNum, strPPONum, strPayerID, strBINLocNum, strDEANum, strHIN, strNAICCode);

	AddDeclarationToSqlBatch(strSqlBatch, "END ELSE BEGIN\r\n");

	//Code for an INSERT statement here
	AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PharmacyIDT (LocationID, NCPDPID, FileID, StateLicNum, MedicareNum, MedicaidNum, "
		"PPONum, PayerID, BINLocNum, DEANum, HIN, NAICCode) values (@LocationID, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, "
		"{STRING}, {STRING}, {STRING}, {STRING}, {STRING});\r\n", strNCPDPID, strFileID, 
		strStateLicNum, strMedicareNum, strMedicaidNum, strPPONum, strPayerID, strBINLocNum, strDEANum, strHIN, strNAICCode);

	AddDeclarationToSqlBatch(strSqlBatch, "END\r\n");

	// (a.walling 2009-03-30 10:19) - PLID 33729 - Set the link status to NULL. Will collapse to true/false next directory update.
	if (bUnlinkDirectory) {
		AddDeclarationToSqlBatch(strSqlBatch, "UPDATE LocationsT SET LinkToDirectory = NULL WHERE ID = @LocationID;\r\n");
	}


	//Now attempt to execute it
	// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
	ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

	//Success!
	return true;
}
