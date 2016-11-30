// OHIPImportCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "OHIPImportCodesDlg.h"

// (a.walling 2008-12-10 14:45) - PLID 32355 - Merged the OHIP Code Importer into Practice

// COHIPImportCodesDlg dialog

IMPLEMENT_DYNAMIC(COHIPImportCodesDlg, CNxDialog)

COHIPImportCodesDlg::COHIPImportCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COHIPImportCodesDlg::IDD, pParent)
{

}

COHIPImportCodesDlg::~COHIPImportCodesDlg()
{
}

void COHIPImportCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OHIP_LOAD, m_btnLoad);
	DDX_Control(pDX, IDC_BROWSE_AND_LOAD, m_btnBrowseAndLoad);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnImport);
	DDX_Control(pDX, IDC_OHIP_SUFFIX, m_editSuffix);
	DDX_Control(pDX, IDC_OHIP_SUFFIX_LBL, m_lblSuffix);
	DDX_Control(pDX, IDC_OHIP_PATH, m_editPath);
	DDX_Control(pDX, IDC_OHIPCODES_NXCOLORCTRL, m_nxcolor);
	DDX_Control(pDX, IDC_OHIP_PREFER_SPEC, m_nxbPreferSpec);
	DDX_Control(pDX, IDC_OHIP_PREFER_PROV, m_nxbPreferProv);		
	DDX_Control(pDX, IDC_OHIP_PATH_LABEL, m_lblPath);
	DDX_Control(pDX, IDC_OHIP_HELP_LABEL, m_lblHelp);
	DDX_Control(pDX, IDC_OHIP_BLANK_CHECK, m_chkBlank);
}


BEGIN_MESSAGE_MAP(COHIPImportCodesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BROWSE_AND_LOAD, &COHIPImportCodesDlg::OnBnClickedBrowseAndLoad)
	ON_BN_CLICKED(IDC_OHIP_LOAD, &COHIPImportCodesDlg::OnBnClickedOhipLoad)
	ON_BN_CLICKED(IDOK, &COHIPImportCodesDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &COHIPImportCodesDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_OHIP_PATH, &COHIPImportCodesDlg::OnEnChangeOHIPPath)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// COHIPImportCodesDlg message handlers

void COHIPImportCodesDlg::OnBnClickedBrowseAndLoad()
{
	try {
		CFileDialog f(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_EXPLORER, NULL, this);

		if (IDOK == f.DoModal()) {
			m_editPath.SetWindowText(f.GetPathName());
			
			m_editPath.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ERASENOW);

			ImportFile();
		}
	} NxCatchAll("Error in OnBnClickedBrowseAndLoad");
}

void COHIPImportCodesDlg::OnBnClickedOhipLoad()
{
	try {
		ImportFile();
	} NxCatchAll("Error in OnBnClickedOhipLoad");
}

void COHIPImportCodesDlg::OnBnClickedOk()
{
	// Import
	try {
		if (IDYES == MessageBox("Please make sure you have carefully reviewed these codes. New codes will be imported; any existing codes will have their fees updated.\n\nDo you want to continue?", NULL, MB_YESNO | MB_ICONQUESTION)) {
			long nImported = ImportCodes();
			
			// (a.walling 2009-01-20 15:43) - PLID 32355 - If zero, make sure they know
			if (nImported == 0) {
				MessageBox("No codes are selected to be imported.", NULL, MB_ICONINFORMATION);
			} else if (nImported > 0) {  // success
				CString strMessage;
				strMessage.Format("Successfuly imported %li codes!", nImported);
				MessageBox(strMessage, NULL, MB_ICONINFORMATION);

				CDialog::OnOK();
			} // otherwise they should have gotten an exception message already
		}
	} NxCatchAll("Error importing OHIP codes");
}

void COHIPImportCodesDlg::OnBnClickedCancel()
{
	OnCancel();
}

BOOL COHIPImportCodesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnImport.AutoSet(NXB_NEW);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));

		m_dl = BindNxDataList2Ctrl(IDC_OHIP_CODES_DL, false);

		m_btnLoad.EnableWindow(FALSE);
		m_btnImport.EnableWindow(FALSE);

		// (a.walling 2009-01-20 15:32) - PLID 32355 - Cache int properties
		g_propManager.CachePropertiesInBulk("OHIPImportCodes_num", propNumber,
			"(Username = '<None>') AND ("
			"Name = 'OHIPImportCodes_PreferProvider' OR "
			"Name = 'OHIPImportCodes_UseBlank' "
			")");

		// (a.walling 2009-01-20 15:33) - PLID 32355 - We only have one text property, but might as well cache it anyway		
		g_propManager.CachePropertiesInBulk("OHIPImportCodes_text", propText,
			"(Username = '<None>') AND ("
			"Name = 'OHIPImportCodes_Suffix'"
			")");

		BOOL bPreferProv = GetRemotePropertyInt("OHIPImportCodes_PreferProvider", TRUE, 0, "<None>", true);
		m_nxbPreferProv.SetCheck(bPreferProv ? TRUE : FALSE);
		m_nxbPreferSpec.SetCheck(bPreferProv ? FALSE : TRUE);

		m_chkBlank.SetCheck(GetRemotePropertyInt("OHIPImportCodes_UseBlank", TRUE, 0, "<None>", true) ? TRUE : FALSE);

		CString strSuffix = GetRemotePropertyText("OHIPImportCodes_Suffix", "A", 0, "<None>", true);
		m_editSuffix.SetWindowText(strSuffix);
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void COHIPImportCodesDlg::OnEnChangeOHIPPath()
{
	try {
		CString strPath;
		m_editPath.GetWindowText(strPath);

		if (strPath.IsEmpty()) {
			if (m_btnLoad.IsWindowEnabled()) {
				m_btnLoad.EnableWindow(FALSE);
			}
		} else {
			if (!m_btnLoad.IsWindowEnabled()) {
				m_btnLoad.EnableWindow(TRUE);
			}
		}
	} NxCatchAll(__FUNCTION__);
}


/*
OHIP Code File Field Layout

First four characters of Fee Schedule code
	1	4	X	ANNN Refers to Provider
Effective Date
	5	8	D	Date on which schedule of fees becomes effective.
Termination Date
	13	8	D	Date after which the schedule fees is no longer in force. 99999999 in this field signifies that the period of validity continues indefinitely
Provider Fee
	21	11	N	Dollars (4 decimal places)
Assistant's fee
	32	11	N	Zero filled
Specialist fee
	43	11	N	Dollars (4 decimal places)
Anaesthetist's fee 
	54	11	N	Zero filled
Non-Anaesthetist's fee
	65	11	N	Zero filled
*/
#define GET_FIELD(str, start, length) str.Mid(start - 1, length)

BOOL COHIPImportCodesDlg::ImportFile(void)
{	
	CString strPath;
	m_editPath.GetWindowText(strPath);
	if (strPath.IsEmpty()) {
		MessageBox("You must choose a file!", NULL, MB_ICONSTOP);
		return FALSE;
	}

	CStdioFile fIn;
	if (fIn.Open(strPath, CFile::modeRead | CFile::shareCompat)) {
		CWaitCursor cws;

		// (a.walling 2009-01-28 15:46) - PLID 32355 - I am a fool; did not clear the list before loading.
		m_dl->Clear();

		CString strLine;
		while (fIn.ReadString(strLine)) {
			if (strLine.GetLength() != 75) {
				m_dl->Clear();
				MessageBox("The OHIP code file is in an unexpected format and cannot be read. Please ensure you are using the correct file.");
				return FALSE; // unreachable code
			}
			CString	strCode = 
				GET_FIELD(strLine, 1, 4);
			CString	strEffectiveDate = 
				GET_FIELD(strLine, 5, 8);
			CString	strTerminationDate = 
				GET_FIELD(strLine, 13, 8);
			CString	strProviderFee = 
				GET_FIELD(strLine, 21, 11);
			CString	strAssistantFee = 
				GET_FIELD(strLine, 32, 11);
			CString	strSpecialistFee = 
				GET_FIELD(strLine, 43, 11);
			CString	strAnaesthetistFee = 
				GET_FIELD(strLine, 54, 11);
			CString	strNonAnaesthetistFee = 
				GET_FIELD(strLine, 65, 11);

			long nProvFee = atoi(strProviderFee);
			long nSpecFee = atoi(strSpecialistFee);

			COleCurrency cyProvFee, cySpecFee;
			// (a.walling 2008-12-03 11:26) - PLID 32284 - Apparently the least significant two decimal places
			// are unused (or for microcents, who knows).
			ASSERT(nProvFee % 100 == 0);
			ASSERT(nSpecFee % 100 == 0);
			cyProvFee.SetCurrency(nProvFee / 10000, nProvFee % 10000);
			cySpecFee.SetCurrency(nSpecFee / 10000, nSpecFee % 10000);

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dl->GetNewRow();
			pRow->PutValue(elcCheck, _variant_t(VARIANT_TRUE, VT_BOOL));
			pRow->PutValue(elcCode, (LPCTSTR)strCode);
			pRow->PutValue(elcProvFee, _variant_t(cyProvFee));
			pRow->PutValue(elcSpecFee, _variant_t(cySpecFee));

			m_dl->AddRowAtEnd(pRow, NULL);
		}

		m_btnImport.EnableWindow(TRUE);
		return TRUE;
	} else {
		MessageBox(FormatString("Could not open the file '%s'.\r\n\r\nPlease make sure this file exists and that you have access to it.", strPath));
	}

	m_btnImport.EnableWindow(FALSE);
	return FALSE;
}

long COHIPImportCodesDlg::ImportCodes(void)
{
	IProgressDialog *pProgressDialog;
	CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (void **)&pProgressDialog);

	if (pProgressDialog) {
		pProgressDialog->SetAnimation(LoadLibrary("Shell32.dll"), 165);               // Set the animation to play.
		pProgressDialog->SetTitle(_bstr_t("NexTech Practice"));
		pProgressDialog->SetCancelMsg(_bstr_t("This operation cannot be canceled."), NULL);   // Will only be displayed if Cancel button is pressed.
		pProgressDialog->SetLine(1, _bstr_t("Importing OHIP codes"), FALSE, NULL);
		pProgressDialog->SetLine(2, _bstr_t("Please wait a few seconds..."), FALSE, NULL);

		DWORD dwFlags;

		if (IsVistaOrGreater()) {
			dwFlags = PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | 0x00000020 | 0x00000040;
		} else {
			dwFlags = PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR;
		}

		pProgressDialog->StartProgressDialog(NULL, NULL, dwFlags, NULL); // Display and enable automatic estimated time remaining.
	}

	try {
		// (a.walling 2009-01-20 15:40) - PLID 32355 - Different method to import the codes. Insert them all into a temp table,
		// then update those that exist and clear them out of the temp table. Finally import the remaining codes as new codes.

		long nImported = 0;

		BOOL bPreferProvFee = m_nxbPreferProv.GetCheck() ? TRUE : FALSE;

		CString strSql = BeginSqlBatch();

		AddStatementToSqlBatch(strSql, "DECLARE @OHIP_NewCodes TABLE(ID INT IDENTITY(1,1) PRIMARY KEY, Code NVARCHAR(50) NOT NULL UNIQUE NONCLUSTERED, Price MONEY NOT NULL)");

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dl->GetFirstRow();

		CString strSuffix;
		m_editSuffix.GetWindowTextA(strSuffix);

		CWaitCursor cws;

		const COleCurrency cyZero(0, 0);
		while (pRow) {
			BOOL bChecked = VarBool(pRow->GetValue(elcCheck));
			if (bChecked) {
				CString strCode = VarString(pRow->GetValue(elcCode));
				strCode += strSuffix;

				COleCurrency cyProvFee = VarCurrency(pRow->GetValue(elcProvFee));
				COleCurrency cySpecFee = VarCurrency(pRow->GetValue(elcSpecFee));

				COleCurrency cyFee;

				// (a.walling 2008-12-03 15:18) - PLID 32284 - Apparently we will import both the provider and specialist
				// fees, using whichever is not zero. Some of the T codes have both a provider and specialist fee, so in
				// that case we use the radio button to determine which fee we 'prefer'.
				if (cySpecFee == cyZero) {
					cyFee = cyProvFee;
				} else if (cyProvFee == cyZero) {
					cyFee = cySpecFee;
				} else {
					// two conflicting fees!
					cyFee = bPreferProvFee ? cyProvFee : cySpecFee;
				}
				
				AddStatementToSqlBatch(strSql, "INSERT INTO @OHIP_NewCodes(Code, Price) VALUES('%s', CONVERT(Money, '%s'))",
					_Q(strCode), _Q(FormatCurrencyForSql(cyFee)));

				nImported++;
			}

			pRow = pRow->GetNextRow();
		};

		if (nImported > 0) {
			//update existing
			AddStatementToSqlBatch(strSql, 
				"UPDATE ServiceT SET Price = NewCodesT.Price "
				"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				"INNER JOIN @OHIP_NewCodes NewCodesT ON CPTCodeT.Code = NewCodesT.Code AND CPTCodeT.Subcode = ''");

			//clear these out of the database for simplicity in the next step
			AddStatementToSqlBatch(strSql, "DELETE FROM @OHIP_NewCodes WHERE Code IN (SELECT Code FROM CPTCodeT WHERE Subcode = '')");

			//insert new
			AddStatementToSqlBatch(strSql, "DECLARE @MaxID INT");
			AddStatementToSqlBatch(strSql, "SET @MaxID = (SELECT COALESCE(MAX(ID), 0) FROM ServiceT);");

			AddStatementToSqlBatch(strSql, 
				"INSERT INTO ServiceT (ID, Name, Price, Taxable1, Taxable2) SELECT @MaxID + ID, %s, Price, 0, 0 FROM @OHIP_NewCodes"
				"", m_chkBlank.GetCheck() ? "' '" : "Code" // use blank (actually a single space for some reason) if desired, otherwise use the code itself
			);
			
			AddStatementToSqlBatch(strSql, 
				"INSERT INTO CPTCodeT (ID, Code, SubCode, TypeOfService, RVU) SELECT @MaxID + ID, Code, '', 0, 0 FROM @OHIP_NewCodes"
			);

			// (j.gruber 2012-12-04 11:44) - PLID 48566
			AddStatementToSqlBatch(strSql, "INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT @MaxID + OHIPCodes.ID, LocationsT.ID FROM LocationsT CROSS JOIN @OHIP_NewCodes OHIPCodes WHERE LocationsT.Managed = 1 ");	

			// expect to get a warning about affecting lots of records here. I'm not sure how to prevent executesqlbatch from doing this,
			// but i'm not about to make an overload for it right now.
			ExecuteSqlBatch(strSql);

			// (a.walling 2009-01-20 15:38) - PLID 32355 - Notify of changes to CPT code table
			CClient::RefreshTable(NetUtils::CPTCodeT);
		}

		if (pProgressDialog) {
			pProgressDialog->StopProgressDialog();
			pProgressDialog->Release();
			pProgressDialog = NULL;
		}

		return nImported;
	} NxCatchAll("Error importing OHIP codes");

	if (pProgressDialog) {
		pProgressDialog->StopProgressDialog();
		pProgressDialog->Release();
		pProgressDialog = NULL;
	}

	return -1;
}

void COHIPImportCodesDlg::OnDestroy()
{
	try {
		SetRemotePropertyInt("OHIPImportCodes_PreferProvider", m_nxbPreferProv.GetCheck() ? TRUE : FALSE, 0, "<None>");

		SetRemotePropertyInt("OHIPImportCodes_UseBlank", m_chkBlank.GetCheck() ? TRUE : FALSE, 0, "<None>");

		CString strSuffix;
		m_editSuffix.GetWindowText(strSuffix);
		SetRemotePropertyText("OHIPImportCodes_Suffix", strSuffix, 0, "<None>");
	} NxCatchAll("COHIPImportCodesDlg::OnDestroy()");

	CNxDialog::OnDestroy();
}
