// SearchNotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SearchNotesDlg.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_GOTO_PATIENT	54798

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSearchNotesDlg dialog


CSearchNotesDlg::CSearchNotesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSearchNotesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSearchNotesDlg)
		nPersonID = -1;
	//}}AFX_DATA_INIT
}


void CSearchNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchNotesDlg)
	DDX_Control(pDX, IDC_SEARCH_DATE_FROM, m_dateFrom);
	DDX_Control(pDX, IDC_SEARCH_DATE_TO, m_dateTo);
	DDX_Control(pDX, IDC_SEARCH_CRITERIA, m_nxeditSearchCriteria);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_RADIO_SEARCH_ALL_DATES, m_radioSearchAllDates);
	DDX_Control(pDX, IDC_RADIO_SEARCH_DATE_RANGE, m_radioSearchDateRange);
	DDX_Control(pDX, IDC_BTN_FIND, m_btnFind);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchNotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSearchNotesDlg)
	ON_BN_CLICKED(IDC_BTN_FIND, OnBtnFind)
	ON_BN_CLICKED(IDC_RADIO_SEARCH_ALL_DATES, OnRadioAllDates)
	ON_BN_CLICKED(IDC_RADIO_SEARCH_DATE_RANGE, OnRadioDateRange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchNotesDlg message handlers

void CSearchNotesDlg::OnBtnFind() 
{
	try {

		CString strSearchString;
		GetDlgItemText(IDC_SEARCH_CRITERIA,strSearchString);
		strSearchString.TrimRight();
		strSearchString.TrimLeft();
		if(strSearchString == "") {
			AfxMessageBox("Please enter in data to search for.");
			return;
		}

		CWaitCursor pWait;

		CString where = ParseCriteriaToSql(strSearchString);

		if(nPersonID >= 0) {
			CString str;
			str.Format(" AND Notes.PersonID = %li",nPersonID);
			where += str;
		}

		if(((CButton*)GetDlgItem(IDC_RADIO_SEARCH_DATE_RANGE))->GetCheck()) {
			COleDateTime dtFrom, dtTo;
			dtFrom = m_dateFrom.GetValue();
			dtTo = m_dateTo.GetValue();

			if(dtTo < dtFrom) {
				AfxMessageBox("Your 'To' date is earlier than your 'From' date. Please correct this before continuing.");
				return;
			}

			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dtTo += dtSpan;			

			CString str;
			str.Format(" AND Notes.Date >= '%s' AND Notes.Date < '%s'",FormatDateTimeForSql(dtFrom,dtoDate),FormatDateTimeForSql(dtTo,dtoDate));
			where += str;
		}

		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
		CIncreaseCommandTimeout ict(600);

		m_ResultsList->WhereClause = _bstr_t(where);
		m_ResultsList->Requery();
		m_ResultsList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		if(m_ResultsList->GetRowCount() == 0) {
			AfxMessageBox("No results matched your search.");
		}
		else {
			AfxMessageBox("Search is complete.");
		}

	}NxCatchAll("Error searching notes.");
}

BOOL CSearchNotesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
	
		// (c.haag 2008-04-25 15:01) - PLID 29793 - NxIconify buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnFind.AutoSet(NXB_INSPECT);
		
		m_ResultsList = BindNxDataListCtrl(this,IDC_RESULTS_LIST,GetRemoteData(),false);

		m_dateFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dateTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		((CButton*)GetDlgItem(IDC_RADIO_SEARCH_ALL_DATES))->SetCheck(TRUE);
		OnChangeRadioDate();

		if(nPersonID >= 0) {
			//hide the patient name column
			m_ResultsList->GetColumn(2)->ColumnStyle = csWidthData;
			m_ResultsList->GetColumn(2)->PutStoredWidth(0);
			SetWindowText("Search Notes For: " + GetExistingPatientName(nPersonID));
		}

		GetDlgItem(IDC_SEARCH_CRITERIA)->SetFocus();
		//((CEdit*)GetDlgItem(IDC_SEARCH_CRITERIA))->SetSel(0, -1);
	}
	NxCatchAll("Error in CSearchNotesDlg::OnInitDialog");
	
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CSearchNotesDlg::ParseCriteriaToSql(CString strSearchString)
{
	CString strSQLwhere, str;

	//first parse each word into an array
	BuildSearchArray(strSearchString);

	int i=0;

	for(i=0;i<m_arySearchParts.GetSize();i++) {
		/*
		if(m_arySearchParts.GetAt(i) == "AND") {
			if((i!=0) && ((i+1)!=m_arySearchParts.GetSize())) {
				str.Format(" Notes.Note LIKE '%%%s%%' AND Notes.Note LIKE '%%%s%%' ",m_arySearchParts.GetAt(i-1),m_arySearchParts.GetAt(i+1));
				if(strSQLwhere == "")
					strSQLwhere = str;
				else
					strSQLwhere = strSQLwhere + " AND (" + str + ") ";
			}
		}
		else if(m_arySearchParts.GetAt(i) == "OR") {
			if((i!=0) && ((i+1)!=m_arySearchParts.GetSize())) {
				str.Format(" Notes.Note LIKE '%%%s%%' OR Notes.Note LIKE '%%%s%%' ",m_arySearchParts.GetAt(i-1),m_arySearchParts.GetAt(i+1));
				if(strSQLwhere == "")
					strSQLwhere = str;
				else
					strSQLwhere = strSQLwhere + " AND (" + str + ") ";
			}
		}
		else {
		*/
			str.Format(" Notes.Note LIKE '%%%s%%' ",_Q(m_arySearchParts.GetAt(i)));
			if(strSQLwhere == "")
				strSQLwhere = str;
			else
				strSQLwhere = strSQLwhere + " AND (" + str + ") ";
		/*}*/
	}

	//now empty the array
	ClearSearchArray();

	return strSQLwhere;
}

void CSearchNotesDlg::BuildSearchArray(CString strSearchString)
{
	while(strSearchString.GetLength() > 0) {

		int nextSpace = strSearchString.Find(" ");
		
		if(nextSpace != -1) {
			//if empty, parse the last section
			CString strPart = strSearchString.Left(nextSpace);
			strPart.TrimRight();
			strPart.TrimLeft();
			if(strPart != "")
				m_arySearchParts.Add(strPart);
			strSearchString = strSearchString.Right(strSearchString.GetLength() - nextSpace - 1);
		}
		else {
			//if empty, parse the last section
			m_arySearchParts.Add(strSearchString);
			strSearchString = "";
		}
	}
}

void CSearchNotesDlg::ClearSearchArray()
{
	for(int i = m_arySearchParts.GetSize() - 1; i >= 0; i--) {
		m_arySearchParts.RemoveAt(i);
	}
	m_arySearchParts.RemoveAll();
}

BEGIN_EVENTSINK_MAP(CSearchNotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSearchNotesDlg)
	ON_EVENT(CSearchNotesDlg, IDC_RESULTS_LIST, 4 /* LButtonDown */, OnLButtonDownResultsList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSearchNotesDlg, IDC_RESULTS_LIST, 6 /* RButtonDown */, OnRButtonDownResultsList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSearchNotesDlg::OnLButtonDownResultsList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nCol == 2 && nRow != -1 && nPersonID == -1) {

		long nID = m_ResultsList->GetValue(nRow,1).lVal;

		GoToPatient(nID);
	}
}

void CSearchNotesDlg::OnRButtonDownResultsList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_ResultsList->PutCurSel(nRow);

	if(nRow == -1 || nPersonID >= 0)
		return;

	CMenu pMenu;
	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, ID_GOTO_PATIENT, "&Go To Patient");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);	
}

BOOL CSearchNotesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {
	case ID_GOTO_PATIENT:

		try {
			long nPatientID;
			nPatientID = m_ResultsList->GetValue(m_ResultsList->GetCurSel(),1).lVal;
			if (nPatientID != -1) {
				GoToPatient(nPatientID);				
			}	
		}NxCatchAll("Error in OnGoToPatient");

		break;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CSearchNotesDlg::GoToPatient(long PatientID) {

	//Set the active patient

	CMainFrame *pMainFrame;
	pMainFrame = GetMainFrame();
	if (pMainFrame != NULL) {

		if (PatientID != GetActivePatientID()) {
			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(PatientID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(!pMainFrame->m_patToolBar.TrySetActivePatientID(PatientID)) {
				return;
			}
		}

		//Now just flip to the patient's module and set the active Patient
		if(!pMainFrame->FlipToModule(PATIENT_MODULE_NAME)) {
			//FlipToModule will have explained why it failed.
			return;
		}

		CNxTabView *pView = pMainFrame->GetActiveView();
		if(pView)
			pView->UpdateView();
	}

	CDialog::OnOK();
}

void CSearchNotesDlg::OnRadioAllDates() 
{
	OnChangeRadioDate();
}

void CSearchNotesDlg::OnRadioDateRange() 
{
	OnChangeRadioDate();
}

void CSearchNotesDlg::OnChangeRadioDate()
{
	if(((CButton*)GetDlgItem(IDC_RADIO_SEARCH_ALL_DATES))->GetCheck()) {
		GetDlgItem(IDC_SEARCH_DATE_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_SEARCH_DATE_TO)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_SEARCH_DATE_FROM)->EnableWindow(TRUE);
		GetDlgItem(IDC_SEARCH_DATE_TO)->EnableWindow(TRUE);
	}
}
