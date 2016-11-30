// InactiveProcedureWarningDlg.cpp : implementation file
// (c.haag 2009-01-06 09:54) - PLID 10776 - Initial implementation. This dialog
// lists reasons why a user cannot inactivate a procedure, and warnings that make
// them think hard before doing so.
//

#include "stdafx.h"
#include "Practice.h"
#include "InactiveProcedureWarningDlg.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

CInactiveProcedureWarnings::CInactiveProcedureWarnings(long nProcedureID, const CString& strProcedureName)
{
	CWaitCursor wc;
	FieldsPtr f;
	m_strProcedureName = strProcedureName;
	_RecordsetPtr prs = CreateParamRecordset(
		/////////////// Get mandatory changes ///////////////

		// (c.haag 2009-01-06 11:10) - PLID 10776 - Populate active child procedures
		"SELECT Name FROM ProcedureT WHERE Inactive = 0 AND MasterProcedureID = {INT} "
		"ORDER BY Name;\r\n"

		// (c.haag 2009-01-06 11:10) - PLID 32571 - Populate ladder steps
		"SELECT 'Ladder ''' + Name + ''' - Step ' + convert(nvarchar, StepOrder) + ' - ''' + StepName + '''' AS Name FROM StepTemplatesT "
		"INNER JOIN LadderTemplatesT ON LadderTemplatesT.ID = StepTemplatesT.LadderTemplateID "
		"WHERE Inactive = 0 AND Action IN (18,19,20) " // PA_Bill, PA_Quote, PA_Ladder
		"AND StepTemplatesT.ID IN (SELECT StepTemplateID FROM StepCriteriaT WHERE ActionID = {INT}) "
		"ORDER BY Name, StepOrder;\r\n"

		// (c.haag 2009-01-06 11:11) - PLID 32521 - Populate EMR items that spawn this procedure. Procedures can only be spawned by
		// list selection actions. Here, we're only interested in active EMR Info items.
		"SELECT 'EMR Item ''' + EmrInfoT.Name + ''' - Option ''' + EmrDataT.Data + '''' AS Name FROM EmrActionsT "
		"INNER JOIN EmrDataT ON EmrDataT.ID = EmrActionsT.SourceID "
		"INNER JOIN EmrInfoT ON EmrInfoT.ID = EmrDataT.EmrInfoID "
		"INNER JOIN EMRInfoMasterT ON EMRInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
		"WHERE EmrInfoMasterT.Inactive = 0 AND EmrActionsT.SourceType = 4 AND EmrActionsT.DestType = 5 AND EmrActionsT.DestID = {INT} AND EmrActionsT.Deleted = 0 AND EmrDataT.Inactive = 0 "
		"ORDER BY EmrInfoT.Name, EmrDataT.Data;\r\n"

		// (c.haag 2009-01-08 13:46) - PLID 32521 - Populate EMR image items that spawn this procedure via hotspots
		"SELECT 'EMR Item ''' + EmrInfoT.Name + '''' AS Name FROM EMRActionsT "
		"INNER JOIN EmrImageHotSpotsT ON EmrImageHotSpotsT.ID = EmrActionsT.SourceID "
		"INNER JOIN EmrInfoT ON EmrInfoT.ID = EmrImageHotSpotsT.EmrInfoID "
		"INNER JOIN EMRInfoMasterT ON EMRInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
		"WHERE EmrInfoMasterT.Inactive = 0 AND EmrActionsT.SourceType = 10 AND EmrActionsT.DestType = 5 AND EmrActionsT.DestID = {INT} AND EmrActionsT.Deleted = 0 "
		"ORDER BY EmrInfoT.Name;\r\n"

		// (c.haag 2009-01-06 11:11) - PLID 32521 - Populate active EMR templates that use this procedure
		"SELECT Name FROM EMRTemplateT WHERE Deleted = 0 AND ID IN (SELECT EMRTemplateID FROM EMRTemplateProceduresT WHERE ProcedureID = {INT}) "
		"AND CollectionID NOT IN (SELECT ID FROM EMRCollectionT WHERE Inactive = 1) "
		"ORDER BY Name;\r\n"

		// (c.haag 2009-01-06 11:12) - PLID 32579 - Populate custom records that use this procedure
		"SELECT Name FROM EMRInfoT INNER JOIN ProcedureToEMRInfoT ON ProcedureToEMRInfoT.EmrInfoID = EMRInfoT.ID WHERE ProcedureID = {INT} "
		"ORDER BY Name;\r\n"

		/////////////// Get warnings ///////////////

		// (c.haag 2009-01-06 11:12) - PLID 10776 - Populate CPT codes that use this procedure (Note: You
		// cannot inactivate a service code that is tied to a procedure)
		"SELECT Code + ' - ' + Name AS ServCode FROM ServiceT INNER JOIN CPTCodeT ON CPTCodeT.ID = ServiceT.ID WHERE ProcedureID = {INT} "
		"ORDER BY Code, Name;\r\n"

		// (c.haag 2009-01-06 11:12) - PLID 10776 - Populate inventory items that use this procedure (Note:
		// You CAN inactivate an inventory item that is tied to a procedure. Maybe it's a legacy bug; we should
		// revisit some day)
		"SELECT Name FROM ServiceT INNER JOIN ProductT ON ProductT.ID = ServiceT.ID WHERE ProcedureID = {INT} "
		"ORDER BY Name;\r\n"

		// (c.haag 2009-01-06 11:12) - PLID 10776 - Populate the parent procedure (Note: You cannot inactivate
		// a procedure with children)
		"SELECT Name FROM ProcedureT WHERE ID IN (SELECT MasterProcedureID FROM ProcedureT WHERE ID = {INT}) "
		"ORDER BY Name;\r\n"

		// (c.haag 2009-01-06 11:13) - PLID 32571 - Populate tracking ladders
		"SELECT Name FROM LadderTemplatesT WHERE ID IN (SELECT LadderTemplateID FROM ProcedureLadderTemplateT WHERE ProcedureID = {INT}) "
		"ORDER BY Name;\r\n"

		,nProcedureID, nProcedureID, nProcedureID, nProcedureID, nProcedureID,
		nProcedureID, nProcedureID, nProcedureID, nProcedureID, nProcedureID);

	// (c.haag 2009-01-06 11:36) - PLID 10776 - Active child procedures
	f = prs->Fields;
	while (!prs->eof) {
		m_astrChildProcedures.Add(AdoFldString(f, "Name"));
		prs->MoveNext();
	}

	// (c.haag 2009-01-06 11:10) - PLID 32571 - Populate ladder steps
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
		while (!prs->eof) {
			m_astrLadderSteps.Add(AdoFldString(f, "Name"));
			prs->MoveNext();
		}
	}

	// (c.haag 2009-01-07 10:20) - PLID 32521 - Populate EMR item option actions that spawn the procedure
	// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
		while (!prs->eof) {
			m_astrEmrItemSpawners.Add(AdoFldString(f, "Name"));
			prs->MoveNext();
		}
	}

	// (c.haag 2009-01-07 10:20) - PLID 32521 - Populate EMR hot spot actions that spawn the procedure
	// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
		while (!prs->eof) {
			m_astrEmrHotSpotSpawners.Add(AdoFldString(f, "Name"));
			prs->MoveNext();
		}
	}

	// (c.haag 2009-01-07 10:24) - PLID 32521 - Populate procedures tied to EMR templates
	// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
		while (!prs->eof) {
			m_astrEMRTemplates.Add(AdoFldString(f, "Name"));
			prs->MoveNext();
		}
	}

	// (c.haag 2009-01-06 17:32) - PLID 32579 - Custom record items
	// (d.thompson 2009-01-26) - PLID 32147 - EMR Standard too
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (g_pLicense->CheckForLicense(CLicense::lcCustomRecords, CLicense::cflrSilent) || 
		g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent))
	{
		while (!prs->eof) {
			m_astrCustomRecordTemplates.Add(AdoFldString(f, "Name"));
			prs->MoveNext();
		}
	}

	// (c.haag 2009-01-06 11:37) - PLID 10776 - CPT codes
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	while (!prs->eof) {
		m_astrLinkedServiceCodes.Add(AdoFldString(f, "ServCode"));
		prs->MoveNext();
	}

	// (c.haag 2009-01-06 11:37) - PLID 10776 - Inventory items
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	while (!prs->eof) {
		m_astrLinkedInvItems.Add(AdoFldString(f, "Name"));
		prs->MoveNext();
	}

	// (c.haag 2009-01-06 11:38) - PLID 10776 - Parent procedure
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (!prs->eof) {
		m_strParentProcedure = AdoFldString(f, "Name");
		m_bHasParentProcedure = TRUE;
	} else {
		m_bHasParentProcedure = FALSE;
	}

	// (c.haag 2009-01-07 09:00) - PLID 32571 - Ladder templates
	prs = prs->NextRecordset(NULL); f = prs->Fields;
	if (g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
		while (!prs->eof) {
			m_astrLadders.Add(AdoFldString(f, "Name"));
			prs->MoveNext();
		}
	}
}

// Returns TRUE if any condition would prevent the procedure from being inactivated
BOOL CInactiveProcedureWarnings::HasErrors()
{
	if (m_astrChildProcedures.GetSize() > 0 ||
		m_astrLadderSteps.GetSize() > 0 ||
		m_astrEmrItemSpawners.GetSize() > 0 ||
		m_astrEmrHotSpotSpawners.GetSize() > 0 ||
		m_astrEMRTemplates.GetSize() > 0 ||
		m_astrCustomRecordTemplates.GetSize() > 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// Returns TRUE if the procedure can be inactivated, but it will have side effects
// (which would be no different from a user manually removing such data relationships
// one by one)
BOOL CInactiveProcedureWarnings::HasWarnings()
{
	if (m_astrLinkedServiceCodes.GetSize() > 0 ||
		m_astrLinkedInvItems.GetSize() > 0 ||
		m_bHasParentProcedure ||
		m_astrLadders.GetSize() > 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// CInactiveProcedureWarningDlg dialog

IMPLEMENT_DYNAMIC(CInactiveProcedureWarningDlg, CNxDialog)

CInactiveProcedureWarningDlg::CInactiveProcedureWarningDlg(CInactiveProcedureWarnings& ipw, CWnd* pParent /*=NULL*/)
	: CNxDialog(CInactiveProcedureWarningDlg::IDD, pParent), m_ipw(ipw)
{
}

CInactiveProcedureWarningDlg::~CInactiveProcedureWarningDlg()
{
}

void CInactiveProcedureWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_stHeader);
	DDX_Control(pDX, IDC_STATIC_FOOTER, m_stFooter);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_BTN_YES, m_btnYes);
	DDX_Control(pDX, IDC_BTN_NO, m_btnNo);
}


BEGIN_MESSAGE_MAP(CInactiveProcedureWarningDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_YES, &CInactiveProcedureWarningDlg::OnBnClickedYes)
	ON_BN_CLICKED(IDC_BTN_NO, &CInactiveProcedureWarningDlg::OnBnClickedNo)
	ON_BN_CLICKED(IDC_BTN_SEND_TO_NOTEPAD, &CInactiveProcedureWarningDlg::OnBnClickedSendToNotepad)
END_MESSAGE_MAP()

void CInactiveProcedureWarningDlg::AddRow(const CString& str, COLORREF clr)
{
	IRowSettingsPtr pRow = m_dlList->GetNewRow();
	pRow->Value[0L] = _bstr_t(str);
	pRow->ForeColor = clr;
	m_dlList->AddRowAtEnd(pRow, NULL);
}

// CInactiveProcedureWarningDlg message handlers

BOOL CInactiveProcedureWarningDlg::OnInitDialog()
{
	try {
		// Do standard initializations
		COLORREF clrRed = RGB(192,0,0);
		CNxDialog::OnInitDialog();
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_INACTIVEPROCWARNINGS, false);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnYes.AutoSet(NXB_OK);
		m_btnNo.AutoSet(NXB_CANCEL);
		int i;

		// Present only errors if there are any
		if (m_ipw.HasErrors()) {
			// (c.haag 2009-01-06 11:36) - PLID 10776 - Active child procedures
			if (m_ipw.m_astrChildProcedures.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is the master procedure for one or more detail procedures. Please unassign the following detail procedures from this procedure before inactivating it:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrChildProcedures.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrChildProcedures[i]);
				}
				AddRow(""); AddRow("");
			}
			// (c.haag 2009-01-07 08:42) - PLID 32571 - Ladder step templates
			if (m_ipw.m_astrLadderSteps.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is associated with one or more ladder template steps. Please unassign this procedure from the following steps before inactivating it:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrLadderSteps.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrLadderSteps[i]);
				}
				AddRow(""); AddRow("");
			}
			// (c.haag 2009-01-07 10:21) - PLID 32521 - EMR list items that spawn the procedure
			if (m_ipw.m_astrEmrItemSpawners.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is spawned by one or more EMR list item options. Please unassign this procedure from the following options before inactivating it:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrEmrItemSpawners.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrEmrItemSpawners[i]);
				}
				AddRow(""); AddRow("");
			}
			// (c.haag 2009-01-08 13:53) - PLID 32521 - EMR image items that spawn the procedure
			if (m_ipw.m_astrEmrHotSpotSpawners.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is spawned by one or more EMR image item hot spots. Please unassign this procedure from the following EMR items before inactivating it:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrEmrHotSpotSpawners.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrEmrHotSpotSpawners[i]);
				}
				AddRow(""); AddRow("");
			}
			// (c.haag 2009-01-07 10:24) - PLID 32521 - EMR templates
			if (m_ipw.m_astrEMRTemplates.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is assigned to one or more EMR templates. Please unassign this procedure from the following templates before inactivating it:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrEMRTemplates.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrEMRTemplates[i]);
				}
				AddRow(""); AddRow("");
			}
			// (c.haag 2009-01-06 17:34) - PLID 32579 - Custom record items
			if (m_ipw.m_astrCustomRecordTemplates.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is in use by one or more custom record items. Please unassign this procedure from the following items before inactivating it:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrCustomRecordTemplates.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrCustomRecordTemplates[i]);
				}
				AddRow(""); AddRow("");		
			}
		}
		// If there are no errors, present only warnings
		else if (m_ipw.HasWarnings()) {
			// Hide the Close button and show the Yes/No buttons
			m_btnClose.ShowWindow(SW_HIDE);
			m_btnYes.ShowWindow(SW_SHOW);
			m_btnNo.ShowWindow(SW_SHOW);
			// Now change the static captions
			m_stHeader.SetWindowText("Please review the following warnings before continuing:");
			m_stFooter.SetWindowText("Are you SURE you wish to inactivate this procedure?");

			// (c.haag 2009-01-06 11:37) - PLID 10776 - CPT codes
			if (m_ipw.m_astrLinkedServiceCodes.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is linked with the following service codes:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrLinkedServiceCodes.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrLinkedServiceCodes[i]);
				}
				AddRow(""); AddRow("");
			}

			// (c.haag 2009-01-06 11:37) - PLID 10776 - Inventory items
			if (m_ipw.m_astrLinkedInvItems.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is linked with the following inventory items:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrLinkedInvItems.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrLinkedInvItems[i]);
				}
				AddRow(""); AddRow("");
			}

			// (c.haag 2009-01-06 11:38) - PLID 10776 - Parent procedure
			if (m_ipw.m_bHasParentProcedure) {
				AddRow(FormatString("The procedure '%s' is a detail for the '%s' procedure.", m_ipw.m_strProcedureName, m_ipw.m_strParentProcedure), clrRed);
				AddRow(""); AddRow("");
			}

			// (c.haag 2009-01-07 09:00) - PLID 32571 - Ladder templates
			if (m_ipw.m_astrLadders.GetSize() > 0) {
				AddRow(FormatString("The procedure '%s' is linked with the following ladder templates. **ANY TRACKING ACTIONS WHICH REFERENCE LADDER PROCEDURES WILL STILL TREAT THIS PROCEDURE AS ACTIVE**:", m_ipw.m_strProcedureName), clrRed);
				for (i=0; i < m_ipw.m_astrLadders.GetSize(); i++) {
					AddRow("     " + m_ipw.m_astrLadders[i]);
				}
				AddRow(""); AddRow("");
			}
		}
	}
	NxCatchAll("Error in CInactiveProcedureWarningDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInactiveProcedureWarningDlg::OnBnClickedYes()
{
	try {
		EndDialog(IDYES);
	}
	NxCatchAll("Error in CInactiveProcedureWarningDlg::OnBnClickedYes");
}

void CInactiveProcedureWarningDlg::OnBnClickedNo()
{
	try {
		EndDialog(IDNO);
	}
	NxCatchAll("Error in CInactiveProcedureWarningDlg::OnBnClickedNo");
}

// (c.haag 2009-01-07 09:24) - PLID 10776 - Copy the header and content text to notepad
void CInactiveProcedureWarningDlg::OnBnClickedSendToNotepad()
{
	try {
		IRowSettingsPtr pRow = m_dlList->GetFirstRow();
		CString strText, strHeader;
		m_stHeader.GetWindowText(strHeader);

		// Build the text output
		strText = strHeader + "\r\n\r\n";
		while (NULL != pRow) {
			strText += VarString(pRow->Value[0L],"") + "\r\n";
			pRow = pRow->GetNextRow();
		}

		// Now write it to a file
		CString strOutputFile = GetNxTempPath() ^ "Inactive Procedure Warnings.txt";
		CStdioFile fOut;
		if (!fOut.Open(strOutputFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat)) {
			MessageBox(CString("Error opening output file for writing - ") + FormatLastError(), "Practice", MB_OK | MB_ICONSTOP);
			return;
		}
		fOut.WriteString(strText);
		fOut.Close();

		// Now open notepad
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		ShellExecute(::GetDesktopWindow(), NULL, "notepad.exe", ("'" + strOutputFile + "'"), NULL, SW_SHOW);
	}
	NxCatchAll("Error in CInactiveProcedureWarningDlg::OnBnClickedSendToNotepad");
}
