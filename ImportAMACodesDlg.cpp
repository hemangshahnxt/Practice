// ImportAMACodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImportAMACodesDlg.h"
#include "AMACode.h"
#include "AuditTrail.h"
#include "SelectAMACodeDlg.h"
#include "GlobalFinancialUtils.h"
#include "FtpUtils.h"
#include "FileUtils.h"
#include "RegUtils.h"
#include "ShowProgressFeedbackDlg.h"
#include <io.h>
#include <afxinet.h>
#include "AdministratorRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.
//(e.lally 2010-02-16) PLID 36849 - Use defines for the various views
#define AMA_CPT			0
#define AMA_ICD9		1
#define AMA_MODIFIER	2
#define TOPS_CPT_MAP	3

//(e.lally 2010-02-16) PLID 36849 - Added
enum TopsCptListColumns
{
	tclCptID=0,
	tclCptCode,
	tclCptDesc,
};


//DRT 4/19/2007 - PLID 25598 - Open the data file and retrieve the version.  
//nFile is the choice of which of the 3 files you want to download 
//	(0 = cpt, 1 = diag, 2 = mod)
//Returns 0 (unknown) if the version is unknown, the file format is invalid, or the file 
//	just does not exist, or any other error condition.
//This is a global function, so I'm placing it here as an AMA Utility (in leiu of making an AMAUtils.cpp file)
long GetAMAVersionOfFile(long nFile)
{
	//Ensure valid option
	//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA codes to service codes for TOPS
	if(nFile < 0 || nFile > 3)
		return 0;

	try {
		//We always store our files in the shared path, not the local path.  We will just hardcode this
		//	to always check the CPT version.  It's the first one downloaded and technically the most
		//	important of the three.
		CString strFullPath = GetSharedPath();
		//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
		if(nFile == AMA_CPT || nFile == TOPS_CPT_MAP)
			strFullPath = strFullPath ^ "AMA_CPT.dat";
		else if(nFile == AMA_ICD9)
			strFullPath = strFullPath ^ "AMA_DIAG.dat";
		else if(nFile == AMA_MODIFIER)
			strFullPath = strFullPath ^ "AMA_MOD.dat";

		long nRetVal = 0;

		//First, see if the file exists.  When clients have never run a download, they won't have
		//	these files.
		if(_access(strFullPath, 0) != 0) {
			//File does not yet exist.
			return 0;	//Unknown
		}

		//Open the file for reading only
		CFile fIn(strFullPath, CFile::modeRead|CFile::shareDenyNone);
		CArchive arIn(&fIn, CArchive::load);

		//read the first line
		CString strIn;
		arIn.ReadString(strIn);
		//convert the text we read in back to something understandable
		ConvertText(strIn, ToGood);

		//The first line needs to be "2.0", because that is the version we support at
		//	this time.
		if(strIn.CompareNoCase("2.0") == 0) {
			//All is well so far!  Read the next line
			arIn.ReadString(strIn);
			ConvertText(strIn, ToGood);

			//The next line is the rest of the header, giving us the version of the file and a count of how many 
			//	codes are in it.  It looks like:
			//VERSION_YYYY, XXXXX (Y = year, x = count)
			CString strFoundVersion;
			if(strIn.Find("VERSION_") > -1) {
				//we have found a version
				strFoundVersion = strIn.Mid(8, 4);
				nRetVal = atoi(strFoundVersion);
			}
			else {
				//we should be on a version but we're not... catastropic error!
				MessageBox(NULL, "Failed to correctly read the input file.  Ensure the file is correctly downloaded.", "AMA Update", MB_OK);
				nRetVal = 0;	//Unknown file version
			}

			//All we wanted was the version at this point, so we're happy to quit.
		}
		else {
			//The versions do not match, we cannot support this file
			MessageBox(NULL, "The AMA files that are currently downloaded are corrupt and cannot be read.  Please ensure your download has finished correctly.", "AMA Update", MB_OK);
			nRetVal = 0;	//Unknown file version
		}

		//File cleanup
		arIn.Close();
		fIn.Close();

		return nRetVal;

	} NxCatchAll("Error in GetAMAVersionOfFile");

	//Only get here if an exception occurred, so we must return 0.
	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CImportAMACodesDlg dialog
#define ALL_CATS	"{All Categories}"

CImportAMACodesDlg::CImportAMACodesDlg(CWnd* pParent)
	: CNxDialog(CImportAMACodesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportAMACodesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nPct = -1;
	m_bImporting = FALSE;
	m_nLastCategoryRow = -1;
	m_bAlwaysChoose = FALSE;
	m_dwCurrentAMAVersionAllowed = 0;		//DRT 4/10/2007 - PLID 25556 - Default to 0, which indicates nothing is known.
	m_dwCurrentAMAVersionDownloaded = 0;	//DRT 4/10/2007 - PLID 25556 - Default to 0, which indicates nothing is known.
}

void CImportAMACodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportAMACodesDlg)
	DDX_Control(pDX, IDC_AMA_UNSELECT_ONE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_AMA_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_AMA_SELECT_ONE, m_btnSelectOne);
	DDX_Control(pDX, IDC_AMA_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_AMA_FILTER, m_btnFilter);
	DDX_Control(pDX, IDC_CODE_VERSION, m_nxstaticCodeVersion);
	DDX_Control(pDX, IDC_PROGRESS_LABEL, m_nxstaticProgressLabel);
	DDX_Control(pDX, IDC_PROGRESS_AMT, m_nxstaticProgressAmt);
	DDX_Control(pDX, IDOK, m_btnImport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_AMA_IMPORT_CATEGORY_LABEL, m_nxstaticCategoryLabel);
	DDX_Control(pDX, IDC_ADMIN_TOPS_CPT_NOTICE_LABEL, m_nxstaticTopsNoticeLabel);
	DDX_Control(pDX, IDC_ADMIN_TOPS_CPT_INSTRUCT, m_nxstaticTopsCptInstructions);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportAMACodesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CImportAMACodesDlg)
	ON_BN_CLICKED(IDC_AMA_SELECT_ONE, OnAmaSelectOne)
	ON_BN_CLICKED(IDC_AMA_SELECT_ALL, OnAmaSelectAll)
	ON_BN_CLICKED(IDC_AMA_UNSELECT_ONE, OnAmaUnselectOne)
	ON_BN_CLICKED(IDC_AMA_UNSELECT_ALL, OnAmaUnselectAll)
	ON_BN_CLICKED(IDC_AMA_FILTER, OnAmaFilter)
	ON_BN_CLICKED(IDC_UPDATE_CODES, OnUpdateCodes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportAMACodesDlg message handlers

BOOL CImportAMACodesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//DRT 4/23/2007 - PLID 25598 - I removed the old description here, we are no longer following it
	//	(and it wasn't exactly right anyways).  The *.dat files now are 1 single year's version, nothing
	//	more.  We now provide a way to download those codes from the nextech ftp site, they are no 
	//	longer installed.

	try {
		m_pUnselected = BindNxDataListCtrl(IDC_AMA_UNSELECTED, false);
		m_pSelected = BindNxDataListCtrl(IDC_AMA_SELECTED, false);
		m_pCategories = BindNxDataListCtrl(IDC_AMA_CATEGORY_LIST, false);

		//setup the NxIconButtons
		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnFilter.SetTextColor(0x00000000);
		// (z.manning, 05/01/2008) - PLID 29860 - More button styles
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnImport.AutoSet(NXB_OK);
		m_btnFilter.SetIcon(IDI_FILTER);

		//add a row to the categories and select it
		TryAddToCategory(ALL_CATS);
		m_pCategories->PutCurSel(0);
		m_nLastCategoryRow = 0;

		//MDH 5/12/2005 - PLID 16458 - Add the ability to filter the unselected codes
		// Set the filter status
		m_bFiltered = false;

		//DRT - PLID 25598 - We need to load the m_dwCurrentAMAVersionDownloaded by looking at the files.  Compare 
		//	this against the version allowed, and if they differ, we need to download the new files before loading the rest 
		//	of the dialog.
		//We do this first because it's reading from a physical file, and thus it's a "permanent" part of the algorithm.  If we
		//	do not, then the auto-download below if you lack a version won't know that we already have something, if an odd case
		//	comes up.
		m_dwCurrentAMAVersionDownloaded = GetAMAVersionOfFile(m_nType);

		//DRT 4/12/2007 - PLID 25556 - We need to load the m_dwCurrentAMAVersionAllowed so we know what we are allowed to use.
		_RecordsetPtr prsLoad = CreateRecordset("SELECT MAX(Version) AS MaxVersion FROM AMAVersionsT");
		if(!prsLoad->eof) {
			_variant_t var = prsLoad->Fields->Item["MaxVersion"]->Value;
			if(var.vt == VT_NULL) {
				//This can happen if the user has never been authorized for any codes.
				if(MessageBox("You currently have no AMA code files.  Would you like to download the latest codes now?\r\n  "
					"This requires an active internet connection, and may take a few minutes to download.", "Download?", MB_YESNO) == IDYES) {
					//Of course they want to download them!
					OnUpdateCodes();
				}
				else {
					//DRT 4/25/2007 - PLID 25598 - If they don't want any code files, we may as well just quit, because they can't do anything anyways.
					CDialog::OnOK();
					return TRUE;
				}
			}
			else {
				//Get the version out for use later
				m_dwCurrentAMAVersionAllowed = VarLong(var);
			}
		}
		else {
			//This should not be able to happen, the MAX() command on no records will result in a NULL 1 record return value.
			ASSERT(FALSE);
		}

		if(m_dwCurrentAMAVersionAllowed < m_dwCurrentAMAVersionDownloaded) {
			//Somehow we downloaded a newer version than we are allowed to use.  The simplest way to resolve the 
			//	situation is just to authorize to the newest version and download the newest codes.
			if(MessageBox("Your AMA file versions are mismatched.  Would you like to check for the latest files now?", "Download?", MB_YESNO) == IDYES) {
				UpdateCodes(TRUE);
			}
			else {
				//They don't want to update, we can't let them import codes they are not authorized for
				return TRUE;
			}
		}
		else if(m_dwCurrentAMAVersionAllowed > m_dwCurrentAMAVersionDownloaded) {
			//Authorized for a newer version, but we have not yet downloaded it.
			if(VerifyDownloadedFiles() == 0) {
				//Failure to verify, we cannot load.  User should already have been warned in the Verify function.
				return TRUE;
			}

			//1 last check, just to be safe.  If the Verify function returns success, the allowed and downloaded
			//	member variables should match.
 			if(m_dwCurrentAMAVersionAllowed != m_dwCurrentAMAVersionDownloaded) {
				//This should not happen, as the verify reported success.
				ASSERT(FALSE);
				MessageBox("Current AMA Code version could not be properly read.  Please contact NexTech Technical Support.");
				return TRUE;
			}
		}
		else {
			//Versions match, life is good, we can load the dialog
		}

		LoadCurrentAMACodes();

		//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA codes to service codes for TOPS
		if(m_nType == TOPS_CPT_MAP){
			m_btnImport.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AMA_CATEGORY_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROGRESS_AMT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROGRESS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADMIN_TOPS_CPT_NOTICE_LABEL)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_ADMIN_TOPS_CPT_INSTRUCT)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_ADMIN_TOPS_CPT_LIST)->ShowWindow(SW_SHOW);
			SetDlgItemText(IDCANCEL, "Close");
			SetDlgItemText(IDC_AMA_IMPORT_CATEGORY_LABEL, "Service Code:");
			SetWindowText("Link AMA Codes to Service Codes");
			m_btnCancel.AutoSet(NXB_CLOSE);
			m_pTopsCptList = BindNxDataList2Ctrl(IDC_ADMIN_TOPS_CPT_LIST, true);
			
		}
		//(e.lally 2010-02-16) PLID 36849 - All other views need to hide the TOPS specific controls
		else{
			GetDlgItem(IDC_AMA_CATEGORY_LIST)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_PROGRESS_AMT)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_PROGRESS_LABEL)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_ADMIN_TOPS_CPT_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADMIN_TOPS_CPT_NOTICE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADMIN_TOPS_CPT_INSTRUCT)->ShowWindow(SW_HIDE);
		}


	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CImportAMACodesDlg::SetType(int nType)
{
	m_nType = nType;
}

void CImportAMACodesDlg::OnOK()
{
	if(m_bImporting)
		return;

	try {
		//(e.lally 2010-02-16) PLID 36849 - Just close if this is the TOPS mode.
		if(m_nType == TOPS_CPT_MAP){
			CDialog::OnOK();
			return;
		}

		if(m_pSelected->GetRowCount() == 0) {
			MsgBox("You must select at least one code before importing.");
			return;
		}

		//Do some preparation for the import - disable controls and display a waiting cursor
		BeginWaitCursor();
		PrepareImport();

		//keep a % count of how far done we are
		long nTotal = m_pSelected->GetRowCount();

		//loop through all of the selected codes, and import each one
		BEGIN_TRANS("AMAImport") {
			long nAuditID = BeginNewAuditEvent();	//we'll use 1 audit event for all code updates
			for(int i = 0; i < nTotal; i++) {
				CString strCode, strDesc;
				strCode = VarString(m_pSelected->GetValue(i, 0));
				strDesc = VarString(m_pSelected->GetValue(i, 1));
				ImportSingleCode(strCode, strDesc, nAuditID);
				RestoreWaitCursor();	//if we popped up a dialog, this will restore the wait cursor

				UpdatePercentage((double)i / (double)(nTotal-1));

				//to ensure it doesn't look like Practice locks up
				PeekAndPump();
			}
		} END_TRANS_CATCH_ALL("AMAImport");

		//we're finished import, undo everything the import
		//preparation did.
		UnprepareImport();
		EndWaitCursor();

		MsgBox("AMA code import successful!");

		CDialog::OnOK();		
	} NxCatchAll("Error import ama codes.");

	//if an error occurred, we might not have called Unprepare
	UnprepareImport();
}

void CImportAMACodesDlg::UpdatePercentage(double pct)
{
	int nPct = (int)(pct * 100.0);

	//only update if the number is going to change
	if(nPct != m_nPct) {
		m_nPct = nPct;

		CString str;
		str.Format("%d%% Completed", nPct);

		SetDlgItemText(IDC_PROGRESS_AMT, str);
	}
}

//We are about to import - to correctly handle the updating of the progress
//	all buttons need disabled.
void CImportAMACodesDlg::PrepareImport()
{
	m_bImporting = TRUE;

	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_AMA_SELECT_ALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_AMA_SELECT_ONE)->EnableWindow(FALSE);
	GetDlgItem(IDC_AMA_UNSELECT_ALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_AMA_UNSELECT_ONE)->EnableWindow(FALSE);
	GetDlgItem(IDC_AMA_CATEGORY_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_AMA_FILTER)->EnableWindow(FALSE);

	m_pSelected->ReadOnly = TRUE;
	m_pUnselected->ReadOnly = TRUE;
}

//We are done importing, re-enable everything that the 
//	prepare function disabled.
void CImportAMACodesDlg::UnprepareImport()
{
	m_bImporting = FALSE;

	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_AMA_SELECT_ALL)->EnableWindow(TRUE);
	GetDlgItem(IDC_AMA_UNSELECT_ONE)->EnableWindow(TRUE);
	GetDlgItem(IDC_AMA_SELECT_ALL)->EnableWindow(TRUE);
	GetDlgItem(IDC_AMA_UNSELECT_ONE)->EnableWindow(TRUE);
	GetDlgItem(IDC_AMA_CATEGORY_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_AMA_FILTER)->EnableWindow(TRUE);
	
	m_pSelected->ReadOnly = FALSE;
	m_pUnselected->ReadOnly = FALSE;
}

//DRT 4/10/2007 - PLID 25564 - Removed PeekAndPump in favor of a global version.

//write a single code/desc pair into the database.  Handles auditing the code as well
//throws _com_error exceptions
void CImportAMACodesDlg::ImportSingleCode(CString strCode, CString strDesc, long nAuditID)
{
	if(m_nType == AMA_CPT)
		//CPT code
		ImportSingleCPT(strCode, strDesc, nAuditID);
	else if(m_nType == AMA_ICD9)
		ImportSingleDiag(strCode, strDesc, nAuditID);
	else if(m_nType == AMA_MODIFIER)
		ImportSingleMod(strCode, strDesc, nAuditID);
	//(e.lally 2010-02-16) PLID 36849 - This should never be called for TOPS mode.
	else
		ASSERT(FALSE);
}

void CImportAMACodesDlg::ImportSingleCPT(CString strCode, CString strDesc, long nAuditID)
{
	//first check to see if it already exists.
	
	// respect the checkbox to "Always choose the first code", by sorting in a
	// predictable manner: AMA codes first, then order by subcode second

	ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT ServiceT.ID, ServiceT.Name, "
		"(SELECT COUNT(*) AS Cnt FROM CPTCodeT WHERE Code = {STRING}) AS CodeCnt "
		"FROM CPTCodeT "
		"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
		"WHERE Code = {STRING} "
		"ORDER BY IsAMA DESC, SubCode ASC",
		strCode, strCode);

	if(prs->eof) {
		//no codes at all
		//this is a new code
		long nNewID = NewNumber("ServiceT", "ID");
		ExecuteSql("INSERT INTO ServiceT (ID, Name, Active) values (%li, '%s', 1)", nNewID, _Q(strDesc));
		ExecuteSql("INSERT INTO CPTCodeT (ID, Code, SubCode, IsAMA) values (%li, '%s', '0', 1)", nNewID, _Q(strCode));
		// (j.gruber 2012-12-04 11:39) - PLID 48566 - servicelocationInfoT
		ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT {INT}, ID FROM LocationsT WHERE Managed = 1 "
						, nNewID);	

		//audit this update
		AuditEvent(-1, strCode, nAuditID, aeiAMAAddNew, nNewID, "", _Q(strCode), aepMedium, aetCreated);
	}
	else {
		long nCodeCnt = AdoFldLong(prs, "CodeCnt", -1);
		long nID = AdoFldLong(prs, "ID");

		// do not show the dialog if "Always choose the first code" was checked
		if(nCodeCnt > 1 && !m_bAlwaysChoose)
		{
			//there are multiple codes here - we need to prompt them
			CSelectAMACodeDlg dlg(this);
			dlg.m_strCode = strCode;
			dlg.m_strDesc = strDesc;
			dlg.m_bAlwaysChoose = m_bAlwaysChoose;
			if(dlg.DoModal() == IDOK) {
				//get the selected ID that was chosen
				long nChosen = dlg.m_nChosen;
				m_bAlwaysChoose = dlg.m_bAlwaysChoose;

				//and now update that item
				ExecuteParamSql("UPDATE ServiceT SET Name = {STRING} WHERE ID = {INT}", strDesc, nChosen);
				ExecuteParamSql("UPDATE CPTCodeT SET IsAMA = 1 WHERE ID = {INT}", nChosen);

				//audit this update
				AuditEvent(-1, strCode, nAuditID, aeiAMAUpdateDesc, nID, _Q(AdoFldString(prs, "Name", "")), _Q(strDesc), aepMedium, aetChanged);
			}
		}
		else {
			//this code already exists.  Update it's description and flag it
			//as AMA.
			ExecuteParamSql("UPDATE ServiceT SET Name = {STRING} WHERE ID = {INT}", strDesc, nID);
			ExecuteParamSql("UPDATE CPTCodeT SET IsAMA = 1 WHERE ID = {INT}", nID);

			//audit this update
			AuditEvent(-1, strCode, nAuditID, aeiAMAUpdateDesc, nID, _Q(AdoFldString(prs, "Name", "")), _Q(strDesc), aepMedium, aetChanged);
		}
	}
}

void CImportAMACodesDlg::ImportSingleDiag(CString strCode, CString strDesc, long nAuditID)
{
	//first check to see if it already exists.
	// (j.armen 2014-03-24 10:47) - PLID 61516 - Code must be ICD9
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, CodeNumber, CodeDesc\r\n"
		"FROM DiagCodes\r\n"
		"WHERE CodeNumber = {STRING} AND ICD10 = 0", strCode);
	if(prs->eof) {
		//no codes at all
		//this is a new code
		//DRT 1/16/2007 - PLID 24177 - No | allowed!
		if(strCode.Find("|") > -1) {
			AfxThrowNxException("Invalid character '|' found importing diagnosis code.");
		}

		// (j.armen 2014-03-12 09:18) - PLID 60410 - DiagCodes.ID is identity seeded
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"INSERT INTO DiagCodes (CodeNumber, CodeDesc, Active, IsAMA)\r\n"
			"	VALUES ({STRING}, {STRING}, 1, 1)\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS DiagID",
			strCode, strDesc.Left(255));

		//audit this update
		AuditEvent(-1, strCode, nAuditID, aeiAMADiagAddNew, AdoFldLong(prs, "DiagID"), "", _Q(strCode), aepMedium, aetCreated);
	}
	else {
		long nID = AdoFldLong(prs, "ID");
		//this code already exists.  Update it's description and flag it
		//as AMA.
		ExecuteParamSql("UPDATE DiagCodes SET CodeDesc = {STRING}, IsAMA = 1 WHERE ID = {INT}", strDesc.Left(255), nID);

		//audit this update
		AuditEvent(-1, strCode, nAuditID, aeiAMADiagUpdateDesc, nID, _Q(AdoFldString(prs, "CodeDesc", "")), _Q(strDesc), aepMedium, aetChanged);
	}
}

//Note that the note field here is limited to 50 characters.
void CImportAMACodesDlg::ImportSingleMod(CString strCode, CString strDesc, long nAuditID)
{
	// (j.dinatale 2010-08-30) - PLID 38121 - Need to trim strCode before we use it
	strCode.TrimLeft();
	strCode.TrimRight();

	//first check to see if it already exists.
	_RecordsetPtr prs = CreateRecordset("SELECT Number, Note FROM CPTModifierT WHERE Number = '%s'", _Q(strCode));
	if(prs->eof) {
		//no codes at all
		//this is a new code
		ExecuteSql("INSERT INTO CPTModifierT (Number, Note, Multiplier, IsAMA) values ('%s', '%s', '1', 1)", _Q(strCode), _Q(strDesc.Left(255)));

		//audit this update
		AuditEvent(-1, strCode, nAuditID, aeiAMAModAddNew, -1, "", _Q(strCode), aepMedium, aetCreated);
	}
	else {
		//this code already exists.  Update it's description and flag it
		//as AMA.
		// (z.manning, 05/01/2007) - PLID 16623 - Ensure the imported code is active.
		ExecuteSql("UPDATE CPTModifierT SET Note = '%s', IsAMA = 1, Active = 1 WHERE Number = '%s'", _Q(strDesc.Left(255)), _Q(strCode));

		//audit this update
		AuditEvent(-1, strCode, nAuditID, aeiAMAModUpdateDesc, -1, _Q(AdoFldString(prs, "Note", "")), _Q(strDesc), aepMedium, aetChanged);
	}
}

void CImportAMACodesDlg::OnCancel() 
{
	if(m_bImporting)
		return;

	CDialog::OnCancel();
}

//this will load the codes with the currently selected category
//DRT 4/19/2007 - PLID 25598 - Cleaned up to use the new versioning from the file
//	instead of hardcoded in the .exe.
void CImportAMACodesDlg::LoadCurrentAMACodes()
{
	//clear out the list
	m_pUnselected->Clear();

	//find out what version we should be importing
	long nVersion = GetAMAVersionOfFile(m_nType);

	//set this version on the dialog
	if(nVersion == 0) {
		//There is no version downloaded.  We will set the current version to <None>, 
		//	and just quit.  We do not want to load anything, because there is nothing
		//	to read.
		SetDlgItemText(IDC_CODE_VERSION, "<None>");
		CRect rcUpdate;
		GetDlgItem(IDC_CODE_VERSION)->GetWindowRect(rcUpdate);
		ScreenToClient(rcUpdate);
		InvalidateRect(rcUpdate);
		return;
	}

	//Otherwise, a legit version
	SetDlgItemInt(IDC_CODE_VERSION, nVersion);
	CRect rcUpdate;
	GetDlgItem(IDC_CODE_VERSION)->GetWindowRect(rcUpdate);
	ScreenToClient(rcUpdate);
	InvalidateRect(rcUpdate);

	//now load all the codes for this version
	LoadAMACodes();
}

//only load codes with the currently selected category, or all
//if ALL_CATS is chosen
//DRT 4/19/2007 - PLID 25598 - Changed to handle the new downloadable files, which
//	have moved on to version 2.0 of the file format.
void CImportAMACodesDlg::LoadAMACodes()
{
	//open the file
	CFile* f;
	f = OpenAMACodeFile();

	if(!f)
		//If this failed, the OpenAMACodeFile() function should have prompted the user
		//	with a message.  We can safely return.
		return;

	CArchive ar(f, CArchive::load);
	BOOL bDone = FALSE;

	while(!bDone) {
		CString strIn;
		ar.ReadString(strIn);
		//convert the text we read in back to something understandable
		ConvertText(strIn, ToGood);

		//The first line is now a version.  This should be "2.0", which is what we 
		//	now support.
		if(strIn.CompareNoCase("2.0") != 0) {
			//Not valid.
			MsgBox("Your AMA code file is not of the correct version.  Ensure that the file is properly downloaded and try again.");
			CleanupFileVars(f, &ar);
			return;
		}

		//Read the next line
		ar.ReadString(strIn);
		ConvertText(strIn, ToGood);

		//this second line should be a version, that looks like:
		//VERSION_YYYY, XXXXX (Y = year, x = count)
		if(strIn.Find("VERSION_") > -1) {
			//we have found a version, we're OK.
			//	We no longer do anything here, because the act of opening the import dialog
			//	did all the version authorization that was required.
		}
		else {
			//we should be on a version but we're not... catastropic error!
			MsgBox("Failed to correctly read the input file.  Ensure the file is correctly downloaded.");
			CleanupFileVars(f, &ar);
			return;
		}

		//now get the # of codes off this list
		long nCnt = 0;
		nCnt = atoi(strIn.Mid(14, strIn.GetLength() - 14));
		if(nCnt == 0) {	//failed to read, or bad anyways
			MsgBox("Failed to parse the AMA Codes input file.  Ensure the file is correctly downloaded.");
			CleanupFileVars(f, &ar);
			return;
		}

		ReadCodesFromArchive(&ar, nCnt);
		bDone = TRUE;
	}

	//We've now read through the list until we found the correct version, and loaded that
	//	version into the datalist.  Our job here is done!  Cleanup after ourselves and go home.
	CleanupFileVars(f, &ar);
}

//Arguments:
//	ar - Pointer to an opened CArchive that is positioned at the first code
//	nCntToRead - The number of codes we should read before stopping
//DRT 4/19/2007 - PLID 25598 - Removed the bAddToList parameter.  It was only there so that
//	we could read through an old version of the codes, back when the files supported multiple
//	versions in a single physical file.  As we no longer support that, there is no reason to 
//	keep a parameter that does nothing.
void CImportAMACodesDlg::ReadCodesFromArchive(CArchive* ar, long nCntToRead)
{
	try {
		//see if there's a category we should filter on
		CString strFilter = "";
		if(m_pCategories->GetCurSel() != sriNoRow) {
			CString str = VarString(m_pCategories->GetValue(m_pCategories->GetCurSel(), 0), "");
			if(str != ALL_CATS)
				strFilter = str;
		}

		for(int i = 0; i < nCntToRead; i++) {
			//read from the data file
			CAMACode pCode;
			pCode.Serialize(*ar);	//this will fill the structure appropriately, we don't have to worry about the conversion from bad to good

			//pull data out of the structure
			CString strCode = pCode.GetCode();
			CString strDesc = pCode.GetDesc();

			//Add to the list here
			//see if we need to filter
			if(strFilter.IsEmpty() || pCode.InCategory(strFilter)) {
				//add rows
				IRowSettingsPtr pRow = m_pUnselected->GetRow(sriNoRow);
				pRow->PutValue(0, _bstr_t(strCode));
				pRow->PutValue(1, _bstr_t(strDesc));
				pRow->PutValue(2, _bstr_t(pCode.GetCatList()));
				m_pUnselected->AddRow(pRow);
			}

			//now we need to add this category to the filter, if necessary
			CString strCatList = pCode.GetCatList();
			//this is a comma separated list of categories, parse it out and
			//	add each one we need
			long nComma = strCatList.Find(",");
			while(nComma > -1) {
				CString str = strCatList.Left(nComma);
				strCatList = strCatList.Right(strCatList.GetLength() - str.GetLength() - 2);
				nComma = strCatList.Find(",");

				if(!str.IsEmpty())
					TryAddToCategory(str);
			}

			//we should now have 1 item (or less) in the list
			if(!strCatList.IsEmpty())
				TryAddToCategory(strCatList);
		}
	} NxCatchAll("Error reading codes from data file.");
}

//returns a pointer to an opened CFile, or NULL
//if the file could not be opened.
//The file opened is created dynamically, the calling function
//must deallocate that memory when finished.
//If we fail to open the file, the error reason is reported
//to the user.
//DRT 4/19/2007 - PLID 25598 - Updated to handle the new filenames.
CFile* CImportAMACodesDlg::OpenAMACodeFile()
{
	//attempt to open the ama data file on the server
	CFile* f = NULL;

	try {
		//(e.lally 2010-02-16) PLID 36849 - Use the CPT code file for TOPS.
		if(m_nType == AMA_CPT || m_nType == TOPS_CPT_MAP) 
			f = new CFile(GetSharedPath() ^ "AMA_CPT.dat", CFile::modeRead | CFile::shareDenyNone);
		else if(m_nType == AMA_ICD9)
			f = new CFile(GetSharedPath() ^ "AMA_DIAG.dat", CFile::modeRead | CFile::shareDenyNone);
		else if(m_nType == AMA_MODIFIER)
			f = new CFile(GetSharedPath() ^ "AMA_MOD.dat", CFile::modeRead | CFile::shareDenyNone);
	} catch(CFileException *e) {
		if (e) {
			e->ReportError(MB_OK);
			e->Delete();
		}
	}

	return f;
}

//attempts to add a category to the list.  If it already exists, it is not added.
void CImportAMACodesDlg::TryAddToCategory(CString strCat)
{
	try {
		long nRow = m_pCategories->FindByColumn(0, _bstr_t(strCat), 0, FALSE);
		if(nRow == sriNoRow) {
			//not found
			IRowSettingsPtr pRow = m_pCategories->GetRow(sriNoRow);
			pRow->PutValue(0, _bstr_t(strCat));
			m_pCategories->AddRow(pRow);
		}
	} NxCatchAll("Error adding to category.");
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CImportAMACodesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CImportAMACodesDlg)
	ON_EVENT(CImportAMACodesDlg, IDC_AMA_CATEGORY_LIST, 1 /* SelChanging */, OnSelChangingAmaCategoryList, VTS_PI4)
	ON_EVENT(CImportAMACodesDlg, IDC_AMA_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenAmaCategoryList, VTS_I4)
	ON_EVENT(CImportAMACodesDlg, IDC_AMA_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellAmaUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CImportAMACodesDlg, IDC_AMA_SELECTED, 3 /* DblClickCell */, OnDblClickCellAmaSelected, VTS_I4 VTS_I2)
	ON_EVENT(CImportAMACodesDlg, IDC_ADMIN_TOPS_CPT_LIST, 16, OnSelChosenTopsCptList, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CImportAMACodesDlg::OnSelChangingAmaCategoryList(long FAR* nNewSel) 
{
	if(*nNewSel == sriNoRow)
		*nNewSel = 0;
}

void CImportAMACodesDlg::OnSelChosenAmaCategoryList(long nRow) 
{
	if(m_bImporting)
		return;

	//MDH 5/12/2005 - PLID 16458 - Add the ability to filter the unselected codes
	// If the unselected codes are currently filtered, remove the filter
	if(m_bFiltered)
		RemoveFilter();

	try {
		//DRT 7/22/2004 - PLID 13502 - We should just blank out the selected side too, but warn 
		//	them first.
		if(nRow == m_nLastCategoryRow)
			return;	//we clicked the same row!

		//need to reload the list - this function does all the filtering on its own
		LoadCurrentAMACodes();

		//MDH - Remove currently selected codes from the unselected codes list
		// Get number of rows in unselected codes datalist
		long nRows = m_pSelected->GetRowCount();
		long nCurrRow = nRows - 1;
		long nUnselectedRow;
		while( (nCurrRow > -1) && (nRows > 0) )
		{ 
			// Get a selected record's code
			_variant_t varCode = m_pSelected->GetValue(nCurrRow,0);

			// Find the code in the unselected codes list
			nUnselectedRow = m_pUnselected->FindByColumn(0, varCode, 0, FALSE);

			// If nUnselectedRow is -1, the record does not exist in the unselected codes list;
			// otherwise, remove the record from the unselected codes list
			if(nUnselectedRow != -1)
				m_pUnselected->RemoveRow(nUnselectedRow);

			// Decrement current row and loop control
			nCurrRow--;
			nRows = m_pSelected->GetRowCount();
		}

		m_nLastCategoryRow = nRow;
	} NxCatchAll("Error loading category.");
}

//(e.lally 2010-02-16) PLID 36849 - Adds the array of AMA codes/descriptions to the given cpt code ID.
//Assumes array positions for code and description match each other.
void CImportAMACodesDlg::AddTopsCptCodeLink(long nCptCodeID, CStringArray* paryAmaCodes, CStringArray* paryAmaDescriptions)
{
	long nAmaCodeSize = paryAmaCodes->GetSize();
	long nAmaDescSize = paryAmaDescriptions->GetSize();
	//Make sure the sizes match
	if(nAmaCodeSize == nAmaDescSize){
		CString strSqlBatch="";
		for(long i = 0; i < nAmaCodeSize; i++){
			CString strCode = paryAmaCodes->GetAt(i);
			CString strDesc = paryAmaDescriptions->GetAt(i);
			//Use a regular batch since this is a dynamic size, plus we can only parametized up to 2100 places
			AddStatementToSqlBatch(strSqlBatch,  
				//Don't insert this CptCodeID, AmaCode combo if it already exists
				"IF NOT EXISTS(SELECT ID FROM TopsCptCodeLinkT WHERE CptCodeID = %li AND AmaCode = '%s')\r\n"
				"BEGIN\r\n"
					"INSERT INTO TopsCptCodeLinkT (CptCodeID, AmaCode, AmaDescription) "
					"VALUES (%li, '%s', '%s')\r\n"
				"END\r\n", nCptCodeID, _Q(strCode), nCptCodeID, _Q(strCode), _Q(strDesc));
		}
		if(!strSqlBatch.IsEmpty()){
			ExecuteSqlBatch(GetRemoteData(), strSqlBatch);
		}
	}
	else{
		ASSERT(FALSE);
		//this shouldn't be possible
	}
}

void CImportAMACodesDlg::OnAmaSelectOne() 
{
	if(m_bImporting)
		return;

	try {
		if(m_pUnselected->GetCurSel() == sriNoRow)
			return;

		//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
		if(m_nType == TOPS_CPT_MAP){
			NXDATALIST2Lib::IRowSettingsPtr pTopsCptRow = m_pTopsCptList->GetCurSel();
			if(pTopsCptRow == NULL){
				return;
			}
			long nCptCodeID = VarLong(pTopsCptRow->GetValue(tclCptID));
			CStringArray saryAmaCode;
			CStringArray saryAmaDesc;
			long p = m_pUnselected->GetFirstSelEnum();
			LPDISPATCH pDisp = NULL;
			bool bWarn = false;
			while(p){
				m_pUnselected->GetNextSelEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);
				pDisp->Release();
				
				CString strAmaCode = VarString(pRow->GetValue(0));
				long nResult = m_pSelected->FindByColumn(0, _bstr_t(strAmaCode), 0, VARIANT_FALSE);
				if(nResult == sriNoRow){
					saryAmaCode.Add(strAmaCode);
					saryAmaDesc.Add(VarString(pRow->GetValue(1)));
				}
			}
			AddTopsCptCodeLink(nCptCodeID, &saryAmaCode, &saryAmaDesc);
		}

		m_pSelected->TakeCurrentRow(m_pUnselected);
	} NxCatchAll("Error selecting one ama code.");
}

void CImportAMACodesDlg::OnAmaSelectAll() 
{
	if(m_bImporting)
		return;

	try {
		//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
		if(m_nType == TOPS_CPT_MAP){
			NXDATALIST2Lib::IRowSettingsPtr pTopsCptRow = m_pTopsCptList->GetCurSel();
			if(pTopsCptRow == NULL){
				return;
			}
			//(e.lally 2010-02-16) PLID 36849 - Selecting All is generally a bad idea unless they have filtered down to a very small number
				//We'll warn for anything over 10 codes.
			if(m_pUnselected->GetRowCount() > 10){
				if(IDOK != MessageBox("This will associate all the listed AMA codes with this service code when only a few are usually needed.\r\n"
					"Are you sure you wish to continue?", "Practice", MB_OKCANCEL)){
						//They cancelled.
						return;
				}
			}
			//This is likely going to take a while
			CWaitCursor wc;
			long nCptCodeID = VarLong(pTopsCptRow->GetValue(tclCptID));
			CStringArray saryAmaCode;
			CStringArray saryAmaDesc;
			long p = m_pUnselected->GetFirstRowEnum();
			LPDISPATCH pDisp = NULL;
			while(p){
				m_pUnselected->GetNextRowEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);
				pDisp->Release();
				
				saryAmaCode.Add(VarString(pRow->GetValue(0)));
				saryAmaDesc.Add(VarString(pRow->GetValue(1)));
			}
			AddTopsCptCodeLink(nCptCodeID, &saryAmaCode, &saryAmaDesc);
		}
		m_pSelected->TakeAllRows(m_pUnselected);
	} NxCatchAll("Error selecting all ama codes");
}


//(e.lally 2010-02-16) PLID 36849 - Removes the link between the given CPT Code ID and the list of AMA CPT codes
void CImportAMACodesDlg::RemoveTopsCptCodeLink(long nCptCodeID, CStringArray* paryAmaCodes)
{
	long nAmaCodeSize = paryAmaCodes->GetSize();
	CString strSqlBatch="";
	for(long i = 0; i < nAmaCodeSize; i++){
		CString strCode = paryAmaCodes->GetAt(i);
		//Just use a regular batch since we have a dynamic size
		AddStatementToSqlBatch(strSqlBatch, 
			"DELETE FROM TopsCptCodeLinkT "
			"WHERE CptCodeID = %li AND AmaCode = '%s' \r\n" , nCptCodeID, _Q(strCode));
	}
	if(!strSqlBatch.IsEmpty()){
		ExecuteSqlBatch(GetRemoteData(), strSqlBatch);
	}
}
void CImportAMACodesDlg::OnAmaUnselectOne() 
{
	if(m_bImporting)
		return;

	try {
		if(m_pSelected->GetCurSel() == sriNoRow)
			return;

		//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
		if(m_nType == TOPS_CPT_MAP){
			NXDATALIST2Lib::IRowSettingsPtr pTopsCptRow = m_pTopsCptList->GetCurSel();
			if(pTopsCptRow == NULL){
				//This code should be unreachable, but just in case.
				return;
			}
			long nCptCodeID = VarLong(pTopsCptRow->GetValue(tclCptID));
			CStringArray saryAmaCode;
			long p = m_pSelected->GetFirstSelEnum();
			LPDISPATCH pDisp = NULL;
			while(p){
				m_pSelected->GetNextSelEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);
				pDisp->Release();
				
				saryAmaCode.Add(VarString(pRow->GetValue(0)));
			}
			RemoveTopsCptCodeLink(nCptCodeID, &saryAmaCode);
		}

		m_pUnselected->TakeCurrentRow(m_pSelected);
	} NxCatchAll("Error removing one ama code.");
}


//(e.lally 2010-02-16) PLID 36849 - Removes all linked AMA codes with the given CPT Code ID
void CImportAMACodesDlg::RemoveAllTopsCptCodeLinks(long nCptCodeID)
{
	//Here we can parameterize
	ExecuteParamSql(GetRemoteData(),
		"DELETE FROM TopsCptCodeLinkT "
		"WHERE CptCodeID = {INT} \r\n" , nCptCodeID);
}
void CImportAMACodesDlg::OnAmaUnselectAll() 
{
	if(m_bImporting)
		return;

	try {
		//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
		if(m_nType == TOPS_CPT_MAP){
			NXDATALIST2Lib::IRowSettingsPtr pTopsCptRow = m_pTopsCptList->GetCurSel();
			if(pTopsCptRow == NULL){
				return;
			}
			long nCptCodeID = VarLong(pTopsCptRow->GetValue(tclCptID));
			RemoveAllTopsCptCodeLinks(nCptCodeID);
		}

		m_pUnselected->TakeAllRows(m_pSelected);
	} NxCatchAll("Error removing all ama codes.");
}

void CImportAMACodesDlg::OnDblClickCellAmaUnselected(long nRowIndex, short nColIndex) 
{
	if(m_bImporting)
		return;

	if(nRowIndex == sriNoRow)
		return;

	OnAmaSelectOne();
}

void CImportAMACodesDlg::OnDblClickCellAmaSelected(long nRowIndex, short nColIndex) 
{
	if(m_bImporting)
		return;

	if(nRowIndex == sriNoRow)
		return;

	OnAmaUnselectOne();
}

//closes both files, and attempts to delete the cfile pointer
void CImportAMACodesDlg::CleanupFileVars(CFile* f, CArchive* ar)
{
	if(ar)
		ar->Close();

	if(f) {
		f->Close();
		delete f;
	}
}

//MDH 5/18/2005 - PLID 16458 - Add the ability to filter the unselected codes
void CImportAMACodesDlg::OnAmaFilter()
{
	//DRT 12/30/2005 - PLID 18663 - Don't allow this to work while importing
	if(m_bImporting)
		return;

	try 
	{
		// Define the number of allowable keywords
		const int nKeywordListSize = 30;

		// Check the filter status.
		// If a filter is currently applied, it needs to be removed.
		if(m_bFiltered)
			RemoveFilter();

		// Else the filter is not applied, so create and apply a filter.
		else
		{
			// Get the search string
			CString strSearch;
			if (IDCANCEL == InputBox(this, "Enter a list of required keywords", strSearch, ""))
				return;
			if (!strSearch.GetLength())
				return;

			// Format strSearch for manipulation
			CString strTemp = strSearch;
			strTemp.TrimLeft();
			strTemp.TrimRight();
			
			// Break strSearch into tokens
			CString strKeywords[nKeywordListSize];
			int nKeywordLocation = 0, nStart = 0, nEnd = 0, nSearchSize = strTemp.GetLength();
			bool bTokenize = true;
			while(bTokenize && (nKeywordLocation < nKeywordListSize))
			{
				strTemp = strTemp.Right(strTemp.GetLength() - nStart);
				strTemp.TrimLeft();
				nStart = 0;
				int nEnd = strTemp.Find(" ", nStart);

				if (nEnd != -1)
				{
					// Add the keyword to the keyword list
					strKeywords[nKeywordLocation] = strTemp.Mid(nStart, nEnd - nStart);
					strKeywords[nKeywordLocation].MakeUpper();
					nKeywordLocation++;
					nStart = nEnd + 1;
				}
				else
				{
					// Last keyword in list, so exit loop
					strKeywords[nKeywordLocation] = strTemp;
					strKeywords[nKeywordLocation].MakeUpper();
					nKeywordLocation++;
					bTokenize = false;
				}
			}

			// Get number of rows in unselected codes datalist
			long nRows = m_pUnselected->GetRowCount();

			// Loop through all records in unselected codes datalist starting at bottom of list
			CString strDesc;
			long nCurrRow = nRows - 1;
			while( (nCurrRow > -1) && (nRows > 0) )
			{ 
				// Get a record's description
				_variant_t desc = m_pUnselected->GetValue(nCurrRow,1);
				if(desc.vt == VT_BSTR)
					strDesc = VarString(desc);

				// Convert the description to upper case (useful for searching)
				CString strDescUpper = strDesc;
				strDescUpper.MakeUpper();

				// Search each row for each token.  If the token is found, mark the token as found.
				int nKeywords = nKeywordLocation;
				bool bFoundKeywords[nKeywordListSize];
				for(nKeywordLocation=0; nKeywordLocation < nKeywords; nKeywordLocation++)
				{
					if(strDescUpper.Find(strKeywords[nKeywordLocation]) != -1)
						bFoundKeywords[nKeywordLocation] = true;
					else
						bFoundKeywords[nKeywordLocation] = false;
				}

				// Determine if all tokens were found or not.  If all tokens were found, then the search was
				//   successfull for this row.  If any one token is not found for the row, then the search
				//   fails for the row.
				bool bFound;
				for(nKeywordLocation=0; nKeywordLocation < nKeywords; nKeywordLocation++)
				{
					if(nKeywordLocation == 0)
						bFound = true;

					if((bFoundKeywords[nKeywordLocation] && bFound) == false)
						bFound = false;
				}

				// If all tokens are not found in the description for the row, the row must be removed
				if(!bFound)
					m_pUnselected->RemoveRow(nCurrRow);

				// Decrement current row and loop control
				nCurrRow--;
				nRows = m_pUnselected->GetRowCount();

			}

			// If no results are returned, restore the current filter
			if (m_pUnselected->GetRowCount() == 0)
			{
				MsgBox("Your filter returned no records. The filter will now be removed");
				RemoveFilter();
			}
			// Else apply the filter
			else
				ApplyFilter();
		}
	} NxCatchAll("CImportAMACodesDlg::OnAmaFilter()");
}

//MDH 5/12/2005 - PLID 16458 - Add the ability to filter the unselected codes
void CImportAMACodesDlg::ApplyFilter()
{
	try {
		// Change the filter status
		m_bFiltered = true;
		SetDlgItemText(IDC_AMA_FILTER, "Unfilter");

		// Set the background in a tinted red so the user knows a filter is in place.
		for (short i=0; i < m_pUnselected->ColumnCount; i++)
			m_pUnselected->GetColumn(i)->BackColor = RGB(255,210,210);

		// Place the selection at the first row of the unselected codes list
		m_pUnselected->PutCurSel(0);

	} NxCatchAll("CImportAMACodesDlg::ApplyFilter()");
}

//MDH 5/12/2005 - PLID 16458 - Add the ability to filter the unselected codes
void CImportAMACodesDlg::RemoveFilter()
{
	try {
		// Change the filter status
		m_bFiltered = false;
		SetDlgItemText(IDC_AMA_FILTER, "Filter");

		// Set the background back to white so the user knows a filter is not in place.
		for (short i=0; i < m_pUnselected->ColumnCount; i++)
			m_pUnselected->GetColumn(i)->BackColor = RGB(255,255,255);

		// Restore previously selected category
		long nLastCategoryRow = m_nLastCategoryRow; //temporarily store last category row
		m_nLastCategoryRow = -1;
		OnSelChosenAmaCategoryList(nLastCategoryRow);

	} NxCatchAll("CImportAMACodesDlg::RemoveFilter()");
}

//DRT 4/10/2007 - PLID 25556 - This button will allow the user to connect to the License Activation
//	Server, request authorization to update their AMA codes, and save that authorization in the local
//	database.  We then fire off the request to perform the download.
void CImportAMACodesDlg::OnUpdateCodes() 
{
	UpdateCodes();
}

//Set bIgnoreNoChange if you do not want the user to be prompted that the codes are already up to date.
void CImportAMACodesDlg::UpdateCodes(BOOL bIgnoreNoChange /*= FALSE*/)
{
	NxSocketUtils::HSERVER hLicenseActivationServer = NULL;

	try {
		//Open a connection to the license activation server.
		hLicenseActivationServer = ConnectToLicenseActivationServer(GetSubRegistryKey());

		if(!hLicenseActivationServer) {
			//Failed to connect
			MessageBox("Failed to connect to the NexTech Activation Server.  Please ensure that you have internet access and try again later.");
			return;
		}

		//Format a request packet with our license key.
		_PACKET_TYPE_AMA_VERSION_REQUEST pktRequest;
		pktRequest.dwKey = g_pLicense->GetLicenseKey();

		//Data for the response
		void* pResponse = NULL;
		DWORD dwSizeOut = 0;

		//Send a request for the AMA version.  If successful, we will have our pResponse filled.
		BOOL bSync = NxSocketUtils::SyncPacketWait(hLicenseActivationServer, PACKET_TYPE_AMA_VERSION_REQUEST, PACKET_TYPE_AMA_VERSION_REQUEST_RESPONSE, (void*)&pktRequest, 
			sizeof(_PACKET_TYPE_AMA_VERSION_REQUEST), pResponse, dwSizeOut);

		if(bSync == FALSE)
		{
			//Failed to send the packet.  We either timed out waiting for information, or there was some network failure on either end that did
			//	not allow the send / response to happen.
			MessageBox("Failed to get authorization from the server.  This may indicate a problem with your internet connection, or may be "
				"caused by interference with a firewall or other 3rd party application.");
		}
		else {
			//We got our response back, huzzah!
			_PACKET_TYPE_AMA_VERSION_REQUEST_RESPONSE* pPktOut = (_PACKET_TYPE_AMA_VERSION_REQUEST_RESPONSE*)pResponse;

			//This is the version we've been approved to use.
			DWORD dwVersion = pPktOut->dwVersion;

			if(dwVersion == 0) {
				//0 means there was some kind of failure on the server end and we are not allowed to update our codes.  We don't
				//	get any more info than that.
				MessageBox("NexTech Activation Server did not approve an update of the AMA codes.  Please contact NexTech Technical Support.");
			}
			else {
				//Any legit version.  We may download the codes and do as we wish with them.

				//See if we're already on the version we were just notified is the latest.  If so, there is no work to be done.
				if(m_dwCurrentAMAVersionAllowed == dwVersion) {
					//Follow parameter to allow suppressing this message.  This situation may occur if the files & recorded version are
					//	out of sync and we are trying to straighten things up without confusing the user with a bunch of error messages.
					if(!bIgnoreNoChange) {
						MessageBox("No newer AMA code data sets were found.  You are up to date with the latest codes.");
					}
				}
				else {
					//The versions differ!  We will need to download the new codes.  But first, we have to set a flag
					//	in the database that tracks our update to this new version.
					ExecuteSql("INSERT INTO AMAVersionsT (Version) values (%li)", dwVersion);

					//Update our member variable, we're now authorized.
					m_dwCurrentAMAVersionAllowed = dwVersion;
				}
			}
		}

	} NxCatchAll("Error in OnUpdateCodes : Update");

	//We must clean up after ourselves and disconnect from the activation server.  This must be done after we
	//	finish with our packet, otherwise the packet will be destroyed.
	//	This function can throw exceptions
	try {
		if(hLicenseActivationServer != NULL)
			NxSocketUtils::Disconnect(hLicenseActivationServer);
	} NxCatchAll("Error in OnUpdateCodes : Disconnect");


	//Fire off the verification, which should detect that our versions no longer match, and will 
	//	do the job of downloading the files.
	if(VerifyDownloadedFiles() == 2) {
		//The verify actually did something, we need to refresh what is currently loaded.
		m_pSelected->Clear();
		LoadCurrentAMACodes();
	}
}

//Return values:
//	0 - Some error condition happened, the files are not up to date and we could not update them.
//	1 - Files are already up to date, no work to be done.
//	2 - Files were out of date, we have now updated and all is well.
//DRT 4/23/2007 - PLID 25598 - Added a progress window.
long CImportAMACodesDlg::VerifyDownloadedFiles()
{
	//FTP data
	CInternetSession* pisNetSession = NULL;
	CFtpConnection *pFTP = NULL;

	//Destination paths.  See comments around the actual downloading.
	const CString strCPTTemp = GetSharedPath() ^ "AMA_CPT_TMP.dat";
	const CString strCPTFinal = GetSharedPath() ^ "AMA_CPT.dat";
	const CString strDiagTemp = GetSharedPath() ^ "AMA_DIAG_TMP.dat";
	const CString strDiagFinal = GetSharedPath() ^ "AMA_DIAG.dat";
	const CString strModTemp = GetSharedPath() ^ "AMA_MOD_TMP.dat";
	const CString strModFinal = GetSharedPath() ^ "AMA_MOD.dat";


	try {
		//First, if the allowed version if still 0, we must quit, because we aren't authorized yet.
		if(m_dwCurrentAMAVersionAllowed == 0)
			return 0;

		//Next, verify the current authorized version vs the current downloaded version.
		if(m_dwCurrentAMAVersionAllowed == m_dwCurrentAMAVersionDownloaded) {
			//They match.  No work needs done, we're up to date.
			return 1;
		}


		//At this point, the versions do not match.  We need to download the latest files.

		CShowProgressFeedbackDlg dlgProgress(0);	//No Delay in displaying the window
		dlgProgress.SetCaption("Downloading AMA Code files.  This may take a few minutes.");
		dlgProgress.SetProgress(0, 4, 0);		//1 step for connecting, 1 step for each file download


		//
		//Open a connection to our FTP site.
		//

		//Open a connection using our standard username and password
		CString strLogin = NxRegUtils::ReadString(GetRegistryBase() + "AMAUpdateLogin", "AMAUpdate");
		CString strPass = NxRegUtils::ReadString(GetRegistryBase() + "AMAUpdatePassword", "AMAUpdate.07");
		//DRT 11/12/2008 - PLID 31970 - Updated to specific subdomain
		CString strSite = NxRegUtils::ReadString(GetRegistryBase() + "AMAUpdateSite", "amaupdate.nextech.com");

		//Create the InternetSession
		pisNetSession = new CInternetSession;
		// (a.walling 2010-06-25 11:48) - PLID 39358 - Use a passive FTP connection. Active FTP connections are fraught with peril!
		pFTP = pisNetSession->GetFtpConnection(strSite, strLogin, strPass, INTERNET_INVALID_PORT_NUMBER, TRUE);

		if (!pFTP) {
			MessageBox("A connection with the FTP site could not be made.  Please check your internet connection and try again.");
			//DRT 9/21/2007 - PLID 25598 - If this fails, we need to cleanup the memory for our session
			if(pisNetSession) {
				pisNetSession->Close();
				delete pisNetSession;
				pisNetSession = NULL;
			}
			return 0;
		}

		//Make sure we're in the base directory
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "AMAUpdatePath", "/ftpUsers/AMACodes");
		if(!pFTP->SetCurrentDirectory(strPath)) {
			//Failed to go to that directory, so our file download will not work.  Give an error here.
			MessageBox("Failed to properly find the AMA update data.  Please check your internet connection and try again.  If you continue to "
				"have problems, please contact NexTech Technical Support.");
			//DRT 9/21/2007 - PLID 25598 - If this fails, we need to cleanup the memory for our session & ftp
			if(pFTP) {
				pFTP->Close();
				delete pFTP;
				pFTP = NULL;
			}

			if(pisNetSession) {
				pisNetSession->Close();
				delete pisNetSession;
				pisNetSession = NULL;
			}

			return 0;
		}

		//Bump progress by 1 since we connected
		dlgProgress.SetProgress(0, 4, 1);

		//
		//At this point we are connected to the FTP site, and we can begin to download our files.  Despite this dialog being setup
		//	to only show 1 of the 3 import types at a time, we'll download all 3.  For error handling purposes, we download all
		//	3 files to a temporary location.  Only if all 3 successfully download do we then move them to their "true" positions.
		//	We do not want to get into a position where 1 file is the new version and the rest are not.
		//Note:  These files should be generated and uploaded using the MakeChecksum program, and uploaded to the proper directory
		//	for the AMAUpdate login.
		FtpUtils::DownloadFileNxCompressedIncremental(pFTP, "AMA_CPT.dat", strCPTTemp, TRUE);
		dlgProgress.SetProgress(0, 4, 2);	//Bump progress by 1 since we downloaded a file

		FtpUtils::DownloadFileNxCompressedIncremental(pFTP, "AMA_DIAG.dat", strDiagTemp, TRUE);
		dlgProgress.SetProgress(0, 4, 3);	//Bump progress by 1 since we downloaded a file

		FtpUtils::DownloadFileNxCompressedIncremental(pFTP, "AMA_MOD.dat", strModTemp, TRUE);
		dlgProgress.SetProgress(0, 4, 4);	//Bump progress by 1 since we downloaded a file

		//If we got here, great success!  Let's get rid of the real ones then move ours over.  If anything fails
		//	while attempting to do this... we're pretty much screwed, and will have to let the user contact technical
		//	support to fix the files.
		FileUtils::MoveFileDiscardSecurity(strCPTTemp, strCPTFinal, FALSE);
		FileUtils::MoveFileDiscardSecurity(strDiagTemp, strDiagFinal, FALSE);
		FileUtils::MoveFileDiscardSecurity(strModTemp, strModFinal, FALSE);

		//And we're done.  Cleanup after ourselves with the FTP connection.
		if(pFTP) {
			pFTP->Close();
			delete pFTP;
			pFTP = NULL;
		}

		if(pisNetSession) {
			pisNetSession->Close();
			delete pisNetSession;
			pisNetSession = NULL;
		}


		//
		//DRT 4/19/2007 - PLID 25598 - Now all files are downloaded.  We need to confirm that these 
		//	files are actually the right version.  Get the version of the file we are currently loading.
		//
		m_dwCurrentAMAVersionDownloaded = GetAMAVersionOfFile(m_nType);

		//Confirm that the version downloaded matches what we expected.
		if(m_dwCurrentAMAVersionAllowed != m_dwCurrentAMAVersionDownloaded) {
			//Despite our best efforts, we still do not match up with the authorized and downloaded
			//	versions.  We're out of options at this point, either the files on the NexTech
			//	servers are just wrong, or there is some failure in the FTP download that 
			//	is not properly throwing errors on failure.
			MessageBox("The downloaded version of your AMA codes does not match the version you are licensed to use.  Please contact "
				"NexTech Technical Support to get the correct codes.");
			return 0;
		}

		//Success!  The versions now match.
		return 2;

	} NxCatchAll("Error in VerifyDownloadedFiles");

	//Just in case our ftp was initialized, but we got an exception, we need to clean these up
	if(pFTP) {
		pFTP->Close();
		delete pFTP;
		pFTP = NULL;
	}

	if(pisNetSession) {
		pisNetSession->Close();
		delete pisNetSession;
		pisNetSession = NULL;
	}

	//It is possible that we partially downloaded some TEMP files for the AMA codes.  If that is the case, 
	//	we need to remove them there.  We don't care if these functions return TRUE or FALSE, we're just
	//	trying to cleanup.
	DeleteFile(strCPTTemp);
	DeleteFile(strDiagTemp);
	DeleteFile(strModTemp);

	//We need to cleanup the interface, if there was a download failure.  Clear all codes from the lists, 
	//	and clear the version text.
	m_pUnselected->Clear();
	m_pSelected->Clear();

	SetDlgItemText(IDC_CODE_VERSION, "");
	CRect rcUpdate;
	GetDlgItem(IDC_CODE_VERSION)->GetWindowRect(rcUpdate);
	ScreenToClient(rcUpdate);
	InvalidateRect(rcUpdate);

	//generic error condition
	return 0;
}

//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
void CImportAMACodesDlg::LoadTopsLinkedAmaCodes(long nCptCodeID)
{
	m_pUnselected->TakeAllRows(m_pSelected);
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteData(), 
		"SELECT * FROM TopsCptCodeLinkT WHERE CptCodeID = {INT} ", nCptCodeID);
	IRowSettingsPtr pNewRow;
	CString strAmaCode, strAmaDescription;
	while(!rs->eof){
		pNewRow = m_pSelected->GetRow(sriGetNewRow);
		strAmaCode = AdoFldString(rs, "AmaCode");
		strAmaDescription = AdoFldString(rs, "AmaDescription");
		pNewRow->PutValue(0, _bstr_t(strAmaCode));
		pNewRow->PutValue(1, _bstr_t(strAmaDescription));
		m_pSelected->AddRow(pNewRow);

		long nResult = m_pUnselected->FindByColumn(0, _bstr_t(strAmaCode), 0, VARIANT_FALSE);
		if(nResult != sriNoRow){
			m_pUnselected->RemoveRow(nResult);
		}
		rs->MoveNext();
	}
}

//(e.lally 2010-02-16) PLID 36849 - Support new mode for mapping AMA CPT codes to service codes for TOPS
void CImportAMACodesDlg::OnSelChosenTopsCptList(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL){
			return;
		}
		long nCptCodeID = VarLong(pRow->GetValue(tclCptID));
		LoadTopsLinkedAmaCodes(nCptCodeID);
	}NxCatchAll(__FUNCTION__)
}