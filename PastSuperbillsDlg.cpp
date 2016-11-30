// PastSuperbillsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practicerc.h"
#include "PastSuperbillsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 9/3/2008 - PLID 31222 - Created
/////////////////////////////////////////////////////////////////////////////
// CPastSuperbillsDlg dialog

enum PastSuperbillsColumns {
	pscPrintedDate = 0,
	pscNumOfPatients = 1,
	pscSuperbillName = 2,
	pscPathName = 3,
	pscColor = 4,
	pscPatientID = 5,
};

CPastSuperbillsDlg::CPastSuperbillsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPastSuperbillsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPastSuperbillsDlg)
	//}}AFX_DATA_INIT
}


void CPastSuperbillsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPastSuperbillsDlg)
	DDX_Control(pDX, IDC_PRINT_SELECTED, m_nxbPrint);
	DDX_Control(pDX, IDC_OPEN_SELECTED, m_nxbOpen);
	DDX_Control(pDX, IDC_CLOSE_PAST_SUPERBILLS, m_nxbClose);
	DDX_Control(pDX, IDC_PRINTED_DATE_TO, m_dtpPrintedDateTo);
	DDX_Control(pDX, IDC_PRINTED_DATE_FROM, m_dtpPrintedDateFrom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPastSuperbillsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPastSuperbillsDlg)
	ON_BN_CLICKED(IDC_CLOSE_PAST_SUPERBILLS, OnClosePastSuperbills)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PRINTED_DATE_FROM, OnDatetimechangePrintedDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PRINTED_DATE_TO, OnDatetimechangePrintedDateTo)
	ON_BN_CLICKED(IDC_OPEN_SELECTED, OnOpenSelected)
	ON_BN_CLICKED(IDC_PRINT_SELECTED, OnPrintSelected)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPastSuperbillsDlg message handlers

using namespace NXDATALIST2Lib;
BOOL CPastSuperbillsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//TES 9/3/2008 - PLID 31222 - Set up our NxIconButtons.
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbOpen.AutoSet(NXB_MERGE);
		m_nxbPrint.AutoSet(NXB_PRINT);

		//TES 9/3/2008 - PLID 31222 - Prevent them from selecting out-of-range dates.
		COleDateTime dtMin;
		dtMin.SetDate(1800,01,01);
		m_dtpPrintedDateTo.SetMinDate(dtMin);
		m_dtpPrintedDateFrom.SetMinDate(dtMin);

		//TES 9/3/2008 - PLID 31222 - Default the date range to the last week.
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		dtToday.SetDateTime(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay(), 0, 0, 0);
		m_dtpPrintedDateTo.SetValue(dtToday);
		m_dtpPrintedDateFrom.SetValue(dtToday - COleDateTimeSpan(7,0,0,0));

		m_pPastSuperbillsList = BindNxDataList2Ctrl(IDC_PAST_SUPERBILLS_LIST, false);

		//TES 9/3/2008 - PLID 31222 - Requery our list, with the current date filters.
		RefreshSuperbillsList();
	
	}NxCatchAll("Error in CPastSuperbillsDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPastSuperbillsDlg::RefreshSuperbillsList()
{
	//TES 9/3/2008 - PLID 31222 - Get the current date range.
	COleDateTime dtFrom = m_dtpPrintedDateFrom.GetDateTime();
	COleDateTime dtTo = m_dtpPrintedDateTo.GetDateTime();
	CString strWhere; 
	//TES 9/3/2008 - PLID 31222 - Now filter the list based on PrintedSuperbillsT.PrintedOn
	strWhere.Format("PrintedOn >= '%s' AND PrintedOn < '%s'", FormatDateTimeForSql(dtFrom), FormatDateTimeForSql(dtTo+COleDateTimeSpan(1,0,0,0)));
	m_pPastSuperbillsList->WhereClause = _bstr_t(strWhere);
	m_pPastSuperbillsList->Requery();
}

void CPastSuperbillsDlg::OnClosePastSuperbills() 
{
	CNxDialog::OnOK();
}

void CPastSuperbillsDlg::OnDatetimechangePrintedDateFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		//TES 9/3/2008 - PLID 31222 - Update the list with the new date range.
		RefreshSuperbillsList();
	}NxCatchAll("Error in CPastSuperbillsDlg::OnDatetimechangePrintedDateFrom()");

	*pResult = 0;
}

void CPastSuperbillsDlg::OnDatetimechangePrintedDateTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		//TES 9/3/2008 - PLID 31222 - Update the list with the new date range.
		RefreshSuperbillsList();
	}NxCatchAll("Error in CPastSuperbillsDlg::OnDatetimechangePrintedDateTo()");
	*pResult = 0;
}

BEGIN_EVENTSINK_MAP(CPastSuperbillsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPastSuperbillsDlg)
	ON_EVENT(CPastSuperbillsDlg, IDC_PAST_SUPERBILLS_LIST, 1 /* SelChanging */, OnSelChangingPastSuperbillsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPastSuperbillsDlg, IDC_PAST_SUPERBILLS_LIST, 2 /* SelChanged */, OnSelChangedPastSuperbillsList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPastSuperbillsDlg::OnSelChangingPastSuperbillsList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if(*lppNewSel) {
			IRowSettingsPtr pRow(*lppNewSel);
			//TES 9/3/2008 - PLID 31222 - If this row doesn't have a path name, then there's no document for them to open/print,
			// so don't even let them select this row (it's also gray, and there's a note explaining why they can't select it).
			if(VarString(pRow->GetValue(pscPathName),"") == "") {
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
		}

	}NxCatchAll("Error in CPastSuperbillsDlg::OnSelChangingPastSuperbillsList()");
}

void CPastSuperbillsDlg::OnSelChangedPastSuperbillsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//TES 9/3/2008 - PLID 31222 - If they have a row selected, enable the Open and Print buttons.
		if(m_pPastSuperbillsList->CurSel == NULL) {
			m_nxbOpen.EnableWindow(FALSE);
			m_nxbPrint.EnableWindow(FALSE);
		}
		else {
			m_nxbOpen.EnableWindow(TRUE);
			m_nxbPrint.EnableWindow(TRUE);
		}
	}NxCatchAll("CPastSuperbillsDlg::OnSelChangedPastSuperbillsList()");
}

void CPastSuperbillsDlg::OnOpenSelected() 
{
	try {
		IRowSettingsPtr pRow = m_pPastSuperbillsList->CurSel;
		if(pRow == NULL) {
			//TES 9/3/2008 - PLID 31222 - This button should be disabled if nothing is selected.
			ASSERT(FALSE);
			return;
		}
		CString strPathName = VarString(pRow->GetValue(pscPathName),"");
		if(strPathName == "") {
			//TES 9/3/2008 - PLID 31222 - We shouldn't let them select rows that don't have a valid path name.
			ASSERT(FALSE);
			return;
		}
		//TES 9/3/2008 - PLID 31222 - Open the document.  The PatientID is the Min(PatientID) for this batch; if there was
		// only one patient, that will be the folder it uses, otherwise, it will look in the -25 folder.
		if(!OpenDocument(strPathName, VarLong(pRow->GetValue(pscPatientID)))) {
			MsgBox(RCS(IDS_NO_FILE_OPEN));
		}
	}NxCatchAll("Error in CPastSuperbillsDlg::OnOpenSelected()");

}

void CPastSuperbillsDlg::OnPrintSelected() 
{
	try {
		IRowSettingsPtr pRow = m_pPastSuperbillsList->CurSel;
		if(pRow == NULL) {
			//TES 9/3/2008 - PLID 31222 - This button should be disabled if nothing is selected.
			ASSERT(FALSE);
			return;
		}
		CString strFileName = VarString(pRow->GetValue(pscPathName),"");
		if(strFileName == "") {
			//TES 9/3/2008 - PLID 31222 - We shouldn't let them select rows that don't have a valid path name.
			ASSERT(FALSE);
			return;
		}
		//TES 9/3/2008 - PLID 31222 - Get this patient's document path (the patient is an arbitrary one in the case of batch
		// merges).
		CString strFilePath = GetPatientDocumentPath(VarLong(pRow->GetValue(pscPatientID)));
	
		//see if it exists in the patient file
		if (!DoesExist(strFilePath ^ strFileName)) {

			//see if it is a multipatdoc
			if (strFileName.Find("MultiPatDoc") != -1) {
				//change the path to be the --25 path
				strFilePath = GetSharedPath() ^ "Documents\\---25\\";
			}
		}
		//TES 9/3/2008 - PLID 31222 - We've got a full path, now print it.
		PrintFile(strFilePath ^ strFileName);
	}NxCatchAll("Error in CPastSuperbillsDlg::OnPrintSelected()");
}
