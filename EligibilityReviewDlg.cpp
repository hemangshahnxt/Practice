// EligibilityReviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilityReviewDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "EligibilityRequestDetailDlg.h"
#include "FinancialView.h"
#include "EEligibilityTabDlg.h"

// (j.jones 2007-06-18 10:09) - PLID 26369 - created the EligibilityReviewDlg

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

enum EligibilityReviewListColumn {

	erlcID = 0,
	erlcPatientID = 1,
	erlcUserDefinedID = 2,
	erlcPatientName = 3,
	erlcInsCoID = 4,
	erlcInsCoName = 5,
	erlcRespType = 6,
	erlcProviderID = 7,
	erlcProvName = 8,
	erlcLastSentDate = 9,
	erlcBatched = 10,
	erlcRowColor = 11,
	erlcResponse = 12,
};

#define ID_REVIEW_SELECTED_ELIG	42001
#define ID_REVIEW_ELIG_GOTO_PATIENT	42002

/////////////////////////////////////////////////////////////////////////////
// CEligibilityReviewDlg dialog


CEligibilityReviewDlg::CEligibilityReviewDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilityReviewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEligibilityReviewDlg)

	//}}AFX_DATA_INIT
}


void CEligibilityReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEligibilityReviewDlg)
	DDX_Control(pDX, IDC_RADIO_ELIG_CREATE_DATE, m_btnCreateDate);
	DDX_Control(pDX, IDC_RADIO_ELIG_SENT_DATE, m_btnSentDate);
	DDX_Control(pDX, IDC_RADIO_ELIG_ALL_DATES, m_btnAllDates);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_REBATCH_SELECTED_ELIG, m_btnRebatch);
	DDX_Control(pDX, IDC_ELIG_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_ELIG_TO_DATE, m_dtTo);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CEligibilityReviewDlg, IDC_ELIG_FROM_DATE, 2 /* Change */, OnChangeEligFromDate, VTS_NONE)
//	ON_EVENT(CEligibilityReviewDlg, IDC_ELIG_FROM_DATE, 3 /* CloseUp */, OnCloseUpEligFromDate, VTS_NONE)
//	ON_EVENT(CEligibilityReviewDlg, IDC_ELIG_TO_DATE, 2 /* Change */, OnChangeEligToDate, VTS_NONE)
//	ON_EVENT(CEligibilityReviewDlg, IDC_ELIG_TO_DATE, 3 /* CloseUp */, OnCloseUpEligToDate, VTS_NONE)

BEGIN_MESSAGE_MAP(CEligibilityReviewDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEligibilityReviewDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ELIG_FROM_DATE, OnChangeEligFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ELIG_TO_DATE, OnChangeEligToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_ELIG_FROM_DATE, OnCloseUpEligFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_ELIG_TO_DATE, OnCloseUpEligToDate)
	ON_BN_CLICKED(IDC_RADIO_ELIG_CREATE_DATE, OnRadioEligCreateDate)
	ON_BN_CLICKED(IDC_RADIO_ELIG_SENT_DATE, OnRadioEligSentDate)
	ON_BN_CLICKED(IDC_RADIO_ELIG_ALL_DATES, OnRadioEligAllDates)
	ON_BN_CLICKED(IDC_BTN_REBATCH_SELECTED_ELIG, OnBtnRebatchSelectedElig)
	ON_BN_CLICKED(ID_REVIEW_SELECTED_ELIG, OnReviewSelectedElig)
	ON_BN_CLICKED(ID_REVIEW_ELIG_GOTO_PATIENT, OnGoToPatient)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEligibilityReviewDlg message handlers

BOOL CEligibilityReviewDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (j.jones 2008-05-07 15:40) - PLID 29854 - added nxiconbuttons for modernization
		m_btnRebatch.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_EligibilityList = BindNxDataList2Ctrl(this, IDC_ELIGIBILITY_REQUESTS, GetRemoteData(), false);

		//default the date filter to requests created in the past 60 days
		CheckDlgButton(IDC_RADIO_ELIG_CREATE_DATE, TRUE);

		COleDateTime dtFrom, dtTo;
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(60,0,0,0);
		dtFrom = dtNow - dtSpan;
		m_dtFrom.SetValue(_variant_t(dtFrom));
		m_dtTo.SetValue(_variant_t(dtNow));

		//load the list based on the default filter
		Refilter();

	}NxCatchAll("Error in CEligibilityReviewDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEligibilityReviewDlg::OnRadioEligCreateDate() 
{
	if(IsDlgButtonChecked(IDC_RADIO_ELIG_CREATE_DATE)) {
		GetDlgItem(IDC_ELIG_FROM_DATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ELIG_TO_DATE)->EnableWindow(TRUE);
	}

	Refilter();
}

void CEligibilityReviewDlg::OnRadioEligSentDate() 
{
	if(IsDlgButtonChecked(IDC_RADIO_ELIG_SENT_DATE)) {
		GetDlgItem(IDC_ELIG_FROM_DATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ELIG_TO_DATE)->EnableWindow(TRUE);
	}

	Refilter();
}

void CEligibilityReviewDlg::OnRadioEligAllDates() 
{
	if(IsDlgButtonChecked(IDC_RADIO_ELIG_ALL_DATES)) {
		GetDlgItem(IDC_ELIG_FROM_DATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ELIG_TO_DATE)->EnableWindow(FALSE);
	}

	Refilter();
}

BEGIN_EVENTSINK_MAP(CEligibilityReviewDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEligibilityReviewDlg)
	ON_EVENT(CEligibilityReviewDlg, IDC_ELIGIBILITY_REQUESTS, 3 /* DblClickCell */, OnDblClickCellEligibilityRequests, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEligibilityReviewDlg, IDC_ELIGIBILITY_REQUESTS, 6 /* RButtonDown */, OnRButtonDownEligibilityRequests, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityReviewDlg, IDC_ELIGIBILITY_REQUESTS, 18 /* RequeryFinished */, OnRequeryFinishedEligibilityRequests, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEligibilityReviewDlg::OnChangeEligFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpEligFromDate(pNMHDR, pResult);

	*pResult = 0;
}

void CEligibilityReviewDlg::OnCloseUpEligFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {

		COleDateTime dtFrom, dtTo;
		dtFrom = m_dtFrom.GetValue();
		dtTo = m_dtTo.GetValue();

		if(dtFrom > dtTo) {

			m_EligibilityList->Clear();

			AfxMessageBox("Your 'from' date is after your 'to' date.\n"
				"Please correct the date range.");

			*pResult = 0;
			return;
		}

		Refilter();

	}NxCatchAll("Error changing date filters.");

	*pResult = 0;
}

void CEligibilityReviewDlg::OnChangeEligToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpEligFromDate(pNMHDR, pResult);

	*pResult = 0;
}

void CEligibilityReviewDlg::OnCloseUpEligToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpEligFromDate(pNMHDR, pResult);

	*pResult = 0;
}

void CEligibilityReviewDlg::Refilter() 
{
	try {

		//requery the list based on our new filter setup

		CString strWhere = "";

		if(!IsDlgButtonChecked(IDC_RADIO_ELIG_ALL_DATES)) {

			COleDateTime dtFrom, dtTo;
			dtFrom = m_dtFrom.GetValue();
			dtTo = m_dtTo.GetValue();

			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dtTo += dtSpan;

			CString strDate = "EligibilityRequestsT.CreateDate";
			
			if(IsDlgButtonChecked(IDC_RADIO_ELIG_SENT_DATE))
				strDate = "EligibilityRequestsT.LastSentDate";

			strWhere.Format("%s >= '%s' AND %s < '%s'", strDate, FormatDateTimeForSql(dtFrom, dtoDate), strDate, FormatDateTimeForSql(dtTo, dtoDate));
		}
		
		m_EligibilityList->WhereClause = _bstr_t(strWhere);
		m_EligibilityList->Requery();

	}NxCatchAll("Error in CEligibilityReviewDlg::Refilter");
}

void CEligibilityReviewDlg::OnDblClickCellEligibilityRequests(LPDISPATCH lpRow, short nColIndex) 
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL)
		return;

	m_EligibilityList->PutCurSel(pRow);

	OnReviewSelectedElig();	
}

void CEligibilityReviewDlg::OnRButtonDownEligibilityRequests(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	//generate a menu with the ability to re-batch the request or review the request

	CMenu pMenu;

	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL)
		return;

	m_EligibilityList->PutCurSel(pRow);

	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, ID_REVIEW_SELECTED_ELIG, "&Review Selected Request");
	pMenu.InsertMenu(1, MF_BYPOSITION, IDC_BTN_REBATCH_SELECTED_ELIG, "Res&tore Selected Request");	
	pMenu.InsertMenu(2, MF_BYPOSITION | MF_SEPARATOR);
	// (j.jones 2010-03-26 08:52) - PLID 37712 - added ability to go to patient
	pMenu.InsertMenu(3, MF_BYPOSITION, ID_REVIEW_ELIG_GOTO_PATIENT, "Close && &Go To Patient");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
}

void CEligibilityReviewDlg::OnBtnRebatchSelectedElig() 
{
	try {

		CWaitCursor pWait;

		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_EligibilityList->GetFirstSelRow();
		CString strSqlBatch = BeginSqlBatch();

		while (pCurSelRow != NULL) {
			IRowSettingsPtr pRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();

			long nID = VarLong(pRow->GetValue(erlcID));

			//ensure the request is batched, and unselected, but don't change the selection
			//if it is already in the batch
			AddStatementToSqlBatch(strSqlBatch, "UPDATE EligibilityRequestsT SET Selected = (CASE WHEN Batched = 0 THEN 0 ELSE Selected END), Batched = 1 WHERE ID = %li", nID);

			_variant_t varTrue(VARIANT_TRUE, VT_BOOL);

			pRow->PutValue(erlcBatched, varTrue);
			pRow->PutForeColor(RGB(127,127,127));

			// (j.jones 2007-06-27 10:18) - PLID 25868 - added "Has Response?" column, and colored it blue,
			// need to reset the color if we colored the row gray
			BOOL bHasResponse = VarLong(pRow->GetValue(erlcResponse), 0) == 1;
			if(bHasResponse) {
				pRow->PutCellForeColor(erlcResponse, RGB(0,0,192));
			}
		}

		if(!strSqlBatch.IsEmpty())
			ExecuteSqlBatch(strSqlBatch);

		// (r.goldschmidt 2014-10-10 15:54) - PLID 62644 - update tab if it is active. needed to adjust because of conversion to modeless
		RefreshEEligibilityTab();

		AfxMessageBox("The selected requests have been re-batched.");

	}NxCatchAll("Error restoring selected requests.");
}

void CEligibilityReviewDlg::OnRequeryFinishedEligibilityRequests(short nFlags) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_EligibilityList->GetFirstRow();
		while (pRow) {

			//don't need to gray out the row when batched, the datalist requery handles that

			// (j.jones 2007-06-27 10:18) - PLID 25868 - added "Has Response?" column, and colored it blue
			BOOL bHasResponse = VarLong(pRow->GetValue(erlcResponse), 0) == 1;
			if(bHasResponse) {
				pRow->PutCellForeColor(erlcResponse, RGB(0,0,192));
			}

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error colorizing list.");
}

void CEligibilityReviewDlg::OnReviewSelectedElig()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_EligibilityList->GetCurSel();
		if(pRow == NULL)
			return;

		// (j.jones 2007-06-20 10:48) - PLID 26387 - added ability to review individual requests/responses
		// (j.jones 2010-07-07 09:52) - PLID 39534 - this dialog now takes in an array of request IDs,
		// though right now we are only viewing one request
		// (r.goldschmidt 2014-10-10 12:51) - PLID 62644 - convert eligibility request detail dialog to modeless
		GetMainFrame()->ShowEligibilityRequestDetailDlg(VarLong(pRow->GetValue(erlcID)));

	}NxCatchAll("Error colorizing list.");
}

// (j.jones 2010-03-26 08:52) - PLID 37712 - added ability to go to patient
void CEligibilityReviewDlg::OnGoToPatient()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_EligibilityList->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		//grab the ID
		long nPatientID = VarLong(pRow->GetValue(erlcPatientID), -1);

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//close this dialog
					CNxDialog::OnOK();

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CEligibilityReviewDlg: Cannot Open Mainframe");
			}
		}
		
	}NxCatchAll("Error in CEligibilityReviewDlg::OnGoToPatient");
}

// (r.goldschmidt 2014-10-10 15:54) - PLID 62644 - Refresh EEligibility Tab if it is the active sheet
void CEligibilityReviewDlg::RefreshEEligibilityTab()
{
	try{

		if (GetMainFrame()) {
			if (GetMainFrame()->IsActiveView(FINANCIAL_MODULE_NAME)) {
				CFinView* pView = (CFinView *)GetMainFrame()->GetOpenView(FINANCIAL_MODULE_NAME);
				if (pView) {
					if (pView->GetActiveTab() == FinancialModule::EEligibilityTab && pView->GetActiveSheet() != NULL) {
						((CEEligibilityTabDlg*)pView->GetActiveSheet())->UpdateView(); // if EEligibility tab is active, update view
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}