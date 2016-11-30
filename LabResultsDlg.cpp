// LabResultsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabResultsDlg.h"

//TES 5/14/2009 - PLID 28559 - Created
// CLabResultsDlg dialog

//TES 11/11/2009 - PLID 36194 - Put in this enum, added columns for Diagnosis and Microscopic Description
enum LabResultsColumns {
	lrcID = 0,
	lrcProcedure = 1,
	lrcFormNumber = 2,
	lrcTest = 3,
	lrcTestDate = 4,
	lrcDateReceived = 5, //TES 10/27/2010 - PLID 41135 - This column was added in 9600, but the enum was never updated
	lrcValue = 6,
	lrcUnits = 7,
	lrcReference = 8,
	lrcFlag = 9,
	lrcStatus = 10,
	lrcDiagnosis = 11,
	lrcMicroscopic = 12,
};
	

IMPLEMENT_DYNAMIC(CLabResultsDlg, CNxDialog)

CLabResultsDlg::CLabResultsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabResultsDlg::IDD, pParent)
{
	m_bFiltered = FALSE;
	m_iFilterColumnID = -1;
	m_bRememberColumnWidths = FALSE;
}

CLabResultsDlg::~CLabResultsDlg()
{
}

void CLabResultsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabResultsDlg)
	DDX_Control(pDX, IDOK, m_nxbClose);
	DDX_Control(pDX, IDC_RESULT_PROCEDURE, m_nxeProcedure);
	DDX_Control(pDX, IDC_RESULT_FORM_NO, m_nxeFormNumber);
	DDX_Control(pDX, IDC_RESULT_STATUS, m_nxeStatus);
	DDX_Control(pDX, IDC_RESULT_FLAG, m_nxeFlag);
	DDX_Control(pDX, IDC_RESULT_DIAGNOSIS, m_nxeDiagnosis);
	DDX_Control(pDX, IDC_RESULT_MICROSCOPIC, m_nxeMicroscopic);
	DDX_Control(pDX, IDC_REMEMBER_COLUMN_WIDTHS, m_nxbRememberColumnWidths);
	DDX_Control(pDX, IDC_RESULT_UNITS, m_nxeUnits);
	DDX_Control(pDX, IDC_RESULT_REFERENCE, m_nxeReference);
}


BEGIN_MESSAGE_MAP(CLabResultsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REMEMBER_COLUMN_WIDTHS, &CLabResultsDlg::OnRememberColumnWidths)
END_MESSAGE_MAP()


// CLabResultsDlg message handlers
BOOL CLabResultsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_nxbClose.AutoSet(NXB_CLOSE);

		//TES 5/15/2009 - PLID 28559 - Set the title text.
		SetWindowText("Lab results for " + m_strPatientName);

		m_pResults = BindNxDataList2Ctrl(IDC_LAB_RESULTS, false);
		//TES 5/14/2009 - PLID 28559 - Filter on the patient we were given.
		CString strWhere;
		strWhere.Format("LabsT.PatientID = %li AND LabsT.Deleted = 0 AND LabResultsT.Deleted = 0", m_nPatientID);
		m_pResults->WhereClause = _bstr_t(strWhere);
		m_pResults->Requery();

		//TES 11/11/2009 - PLID 36277 - Do we need to restore saved column widths?
		m_bRememberColumnWidths = GetRemotePropertyInt("LabResultsRememberColumns", 0, 0, GetCurrentUserName(), true) != 0;
		CheckDlgButton(IDC_REMEMBER_COLUMN_WIDTHS, m_bRememberColumnWidths);
		if (m_bRememberColumnWidths) {
			//TES 11/11/2009 - PLID 36277 - Yes, we do!
			RestoreColumnWidths();
		}
	}NxCatchAll("Error in CLabResultsDlg::OnInitDialog()");

	return TRUE;
}

BEGIN_EVENTSINK_MAP(CLabResultsDlg, CNxDialog)
ON_EVENT(CLabResultsDlg, IDC_LAB_RESULTS, 6, CLabResultsDlg::OnRButtonDownLabResults, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CLabResultsDlg, IDC_LAB_RESULTS, 2, CLabResultsDlg::OnSelChangedLabResults, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CLabResultsDlg, IDC_LAB_RESULTS, 18, CLabResultsDlg::OnRequeryFinishedLabResults, VTS_I2)
END_EVENTSINK_MAP()

using namespace NXDATALIST2Lib;
// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_FILTER_SELECTION	36790
#define ID_FILTER_EXCLUDING	36791
#define ID_REMOVE_FILTER	36792

void CLabResultsDlg::OnRButtonDownLabResults(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		m_pResults->CurSel = pRow;

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		if(m_pResults->CurSel != NULL) {
			//TES 5/15/2009 - PLID 28559 - Give them options to filter (copied from Auditing).
			IColumnSettingsPtr pCol = m_pResults->GetColumn(nCol);
			CString strClause = AsClauseString(pRow->GetValue(nCol));
			CString strCellString;
			strCellString.Format("%s %s", (LPCTSTR)pCol->ColumnTitle, strClause);
		
			m_iFilterColumnID = pCol->Index;
			m_varFilterColumnData = pRow->GetValue(nCol);

			CString strFilterSelection = "Filter Selection " + strCellString;
			CString strFilterExcluding = "Filter Excluding " + strCellString;

			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_FILTER_SELECTION, strFilterSelection);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_FILTER_EXCLUDING, strFilterExcluding);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		}
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REMOVE_FILTER, "Remove Filter");

		//TES 5/15/2009 - PLID 28559 - disable the filter selection if there is none
		if(!m_bFiltered)
			mnu.EnableMenuItem(ID_REMOVE_FILTER,MF_GRAYED);

		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, this, NULL);
	}NxCatchAll("Error in CLabResultsDlg::RButtonDownLabResults()");
}

BOOL CLabResultsDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		//TES 5/15/2009 - PLID 28559 - Handle filtering (copied from Auditing).
		switch(wParam) {
			case ID_FILTER_SELECTION: {
				CWaitCursor pWait;
				m_pResults->SetRedraw(FALSE);
				IRowSettingsPtr pRow = m_pResults->GetFirstRow();
				while(pRow) {
					//TES 5/15/2009 - PLID 28559 - Get the next row first, in case this one gets removed.
					IRowSettingsPtr pNextRow = pRow->GetNextRow();
					if(pRow->GetValue(m_iFilterColumnID) != m_varFilterColumnData) {
						m_bFiltered = TRUE;
						m_pResults->RemoveRow(pRow);
					}
					pRow = pNextRow;
				}
				m_pResults->SetRedraw(TRUE);
				break;
			}
			case ID_FILTER_EXCLUDING: {
				CWaitCursor pWait;
				m_pResults->SetRedraw(FALSE);
				IRowSettingsPtr pRow = m_pResults->GetFirstRow();
				while(pRow) {
					//TES 5/15/2009 - PLID 28559 - Get the next row first, in case this one gets removed.
					IRowSettingsPtr pNextRow = pRow->GetNextRow();
					if(pRow->GetValue(m_iFilterColumnID) == m_varFilterColumnData) {
						m_bFiltered = TRUE;
						m_pResults->RemoveRow(pRow);
					}
					pRow = pNextRow;
				}
				m_pResults->SetRedraw(TRUE);
				break;
			}
			case ID_REMOVE_FILTER:
				m_bFiltered = FALSE;
				m_pResults->Requery();
				break;
		}
	}NxCatchAll("Error in CLabResultsDlg::OnCommand()");

	return CNxDialog::OnCommand(wParam, lParam);
}

void CLabResultsDlg::OnSelChangedLabResults(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try { 
		//TES 11/11/2009 - PLID 36194 - Fill in the controls on the bottom of the screen.
		FillControls();
	}NxCatchAll("Error in CLabResultsDlg::OnSelChangedLabResults()");
}

void CLabResultsDlg::FillControls()
{
	IRowSettingsPtr pRow = m_pResults->CurSel;
	if(pRow == NULL) {
		//TES 11/11/2009 - PLID 36194 - Just clear everything out.
		SetDlgItemText(IDC_RESULT_PROCEDURE, "");
		SetDlgItemText(IDC_RESULT_FORM_NO, "");
		SetDlgItemText(IDC_RESULT_UNITS, "");
		SetDlgItemText(IDC_RESULT_REFERENCE, "");
		SetDlgItemText(IDC_RESULT_STATUS, "");
		SetDlgItemText(IDC_RESULT_FLAG, "");
		SetDlgItemText(IDC_RESULT_DIAGNOSIS, "");
		SetDlgItemText(IDC_RESULT_MICROSCOPIC, "");
	}
	else {
		//TES 11/11/2009 - PLID 36194 - Pull all the information from the datalist.
		SetDlgItemText(IDC_RESULT_PROCEDURE, AsString(pRow->GetValue(lrcProcedure)));
		SetDlgItemText(IDC_RESULT_FORM_NO, AsString(pRow->GetValue(lrcFormNumber)));
		SetDlgItemText(IDC_RESULT_UNITS, AsString(pRow->GetValue(lrcUnits)));
		SetDlgItemText(IDC_RESULT_REFERENCE, AsString(pRow->GetValue(lrcReference)));
		SetDlgItemText(IDC_RESULT_STATUS, AsString(pRow->GetValue(lrcStatus)));
		SetDlgItemText(IDC_RESULT_FLAG, AsString(pRow->GetValue(lrcFlag)));
		SetDlgItemText(IDC_RESULT_DIAGNOSIS, AsString(pRow->GetValue(lrcDiagnosis)));
		SetDlgItemText(IDC_RESULT_MICROSCOPIC, AsString(pRow->GetValue(lrcMicroscopic)));
	}
}
void CLabResultsDlg::OnRequeryFinishedLabResults(short nFlags)
{
	try {
		//TES 11/11/2009 - PLID 36194 - Go ahead and select the first result.
		if(m_pResults->CurSel == NULL) {
			m_pResults->CurSel = m_pResults->GetFirstRow();
		}
		FillControls();
	}NxCatchAll("Error in CLabResultsDlg::OnRequeryFinishedLabResults()");
}


void CLabResultsDlg::RestoreColumnWidths()
{
	//TES 11/11/2009 - PLID 36277 - Code based off of CEMRSearch::RestoreColumns()
	CString strColumnWidths = GetRemotePropertyText("LabResultsColumnWidths", "", 0, GetCurrentUserName(), false);

	CArray<int, int> arWidths;
	//TES 7/23/2010 - PLID 38243 - We now start the column widths with the version (prefixed with V).
	// If they don't have that, then it must be version 1 (the original widths).
	int nVersion = 1;
	int tokIndex = strColumnWidths.Find(',');
	if(strColumnWidths.GetAt(0) == 'V') {
		nVersion = atol(strColumnWidths.Mid(1,tokIndex-1));
		strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
		tokIndex = strColumnWidths.Find(',');
	}

	if (tokIndex == -1) {
		// (a.walling 2006-07-11 16:33) - PLID 21073 - It is empty or invalid, so rebuild
		SaveColumnWidths();
		return;
	}

	int nColIndex = 0;
	while(tokIndex != -1) {
		if(nColIndex == 5 && nVersion < 2) {
			//TES 7/23/2010 - PLID 38243 - They're on a version from before the Received Date column
			// existed, so add in that column's default width (which is zero).
			arWidths.Add(0);
		}
		else {
			CString str = strColumnWidths.Left(tokIndex);
			arWidths.Add(atoi(str));
			strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
			tokIndex = strColumnWidths.Find(',');
		}
		nColIndex++;
	}
	arWidths.Add(atoi(strColumnWidths));

	if (arWidths.GetSize() != m_pResults->ColumnCount) {
		//TES 11/11/2009 - PLID 36277 - We have invalid data, so save the current widths.
		SaveColumnWidths();
		return;
	}

	for (int i = 0; i < m_pResults->ColumnCount; i++)
	{
		//TES 11/11/2009 - PLID 36277 - Now go through the columns, clear any that are set as Auto or Data, and set their saved width.
		IColumnSettingsPtr pCol = m_pResults->GetColumn(i);
		long nStyle = pCol->ColumnStyle;
		nStyle = (nStyle&(~csWidthAuto)&(~csWidthData));
		pCol->ColumnStyle = nStyle;
		pCol->StoredWidth = arWidths[i];
	}
}

void CLabResultsDlg::SaveColumnWidths()
{
	//TES 11/11/2009 - PLID 36277 - Code based off of CEMRSearch::SaveColumns()
	CString strColumnWidths;
	if (!m_bRememberColumnWidths) 
		return;

	//TES 7/23/2010 - PLID 38243 - Start with the version of the column widths; we're on Version 2, 
	// which is the original columns plus a Received Date column at position 5.
	strColumnWidths += "V2,";

	// Store the columns in a xx,xx,xx,xx format
	for (int i = 0; i < m_pResults->ColumnCount; i++)
	{
		IColumnSettingsPtr pCol = m_pResults->GetColumn(i);
		CString str;		
		
		str.Format("%d", pCol->StoredWidth);
		
		if (i > 0)
			strColumnWidths += ",";

		strColumnWidths += str;
	}

	SetRemotePropertyText("LabResultsColumnWidths", strColumnWidths, 0, GetCurrentUserName());
}

void CLabResultsDlg::OnRememberColumnWidths()
{
	try {
		//TES 11/11/2009 - PLID 36277 - Set the property.
		m_bRememberColumnWidths = IsDlgButtonChecked(IDC_REMEMBER_COLUMN_WIDTHS) != 0;
		SetRemotePropertyInt("LabResultsRememberColumns", m_bRememberColumnWidths ? 1 : 0, 0, GetCurrentUserName());

		if (m_bRememberColumnWidths) {
			//TES 11/11/2009 - PLID 36277 - They want to remember, so go ahead and save the widths to data.
			SaveColumnWidths();
		}
	} NxCatchAll("Error in CLabResultsDlg::OnRememberColumnWidths()");
}


void CLabResultsDlg::OnOK()
{
	try {
		//TES 11/11/2009 - PLID 36277 - Before we go, make sure our column widths are stored, if necessary.
		SaveColumnWidths();
	}NxCatchAll("Error in CLabResultsDlg::OnOK()");

	CNxDialog::OnOK();
}

void CLabResultsDlg::OnCancel()
{
	try {
		//TES 11/11/2009 - PLID 36277 - Before we go, make sure our column widths are stored, if necessary.
		SaveColumnWidths();
	}NxCatchAll("Error in CLabResultsDlg::OnCancel()");

	CNxDialog::OnCancel();
}