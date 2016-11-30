// PrepareRefundChecksDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PrepareRefundChecksDlg.h"
#include "GlobalDrawingUtils.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_ID	0
#define COLUMN_PAT_ID	1
#define COLUMN_PAT_NAME	2
#define COLUMN_PAT_USER_DEFINED_ID	3
#define COLUMN_AMOUNT	4
#define COLUMN_DATE	5
#define COLUMN_INPUT_DATE	6
#define COLUMN_LOC_NAME	7
#define COLUMN_PROVIDER_NAME	8
#define COLUMN_CHECK_NO	9

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPrepareRefundChecksDlg dialog


CPrepareRefundChecksDlg::CPrepareRefundChecksDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPrepareRefundChecksDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrepareRefundChecksDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPrepareRefundChecksDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrepareRefundChecksDlg)
	DDX_Control(pDX, IDC_BTN_SAVE_AND_PREVIEW, m_btnSaveAndPreview);
	DDX_Control(pDX, IDC_BTN_SAVE_AND_CLOSE, m_btnSaveAndClose);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_CLEAR, m_btnClear);
	DDX_Control(pDX, IDC_BTN_AUTONUMBER, m_btnAutoNumber);
	DDX_Control(pDX, IDC_UNSELECT_ALL_REFUNDS, m_btnUnselectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_REFUND, m_btnUnselectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_REFUNDS, m_btnSelectAll);
	DDX_Control(pDX, IDC_SELECT_ONE_REFUND, m_btnSelectOne);
	DDX_Control(pDX, IDC_STARTING_CHECK_NO, m_nxeditStartingCheckNo);
	DDX_Control(pDX, IDC_UNSELECTED_TOTAL, m_nxstaticUnselectedTotal);
	DDX_Control(pDX, IDC_SELECTED_TOTAL, m_nxstaticSelectedTotal);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrepareRefundChecksDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPrepareRefundChecksDlg)
	ON_BN_CLICKED(IDC_SELECT_ONE_REFUND, OnSelectOneRefund)
	ON_BN_CLICKED(IDC_SELECT_ALL_REFUNDS, OnSelectAllRefunds)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_REFUND, OnUnselectOneRefund)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_REFUNDS, OnUnselectAllRefunds)
	ON_BN_CLICKED(IDC_BTN_AUTONUMBER, OnBtnAutonumber)
	ON_BN_CLICKED(IDC_BTN_CLEAR, OnBtnClear)
	ON_BN_CLICKED(IDC_BTN_SAVE_AND_CLOSE, OnBtnSaveAndClose)
	ON_BN_CLICKED(IDC_BTN_SAVE_AND_PREVIEW, OnBtnSaveAndPreview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrepareRefundChecksDlg message handlers

BEGIN_EVENTSINK_MAP(CPrepareRefundChecksDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPrepareRefundChecksDlg)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_UNSELECTED_REFUND_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedRefundList, VTS_I4 VTS_I2)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_UNSELECTED_REFUND_LIST, 9 /* EditingFinishing */, OnEditingFinishingUnselectedRefundList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_UNSELECTED_REFUND_LIST, 10 /* EditingFinished */, OnEditingFinishedUnselectedRefundList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_UNSELECTED_REFUND_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUnselectedRefundList, VTS_I2)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_SELECTED_REFUND_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedRefundList, VTS_I4 VTS_I2)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_SELECTED_REFUND_LIST, 9 /* EditingFinishing */, OnEditingFinishingSelectedRefundList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPrepareRefundChecksDlg, IDC_SELECTED_REFUND_LIST, 10 /* EditingFinished */, OnEditingFinishedSelectedRefundList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CPrepareRefundChecksDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 10:35) - PLID 29953 - set button styles for modernization
	m_btnSaveAndPreview.AutoSet(NXB_PRINT_PREV);
	m_btnSaveAndClose.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_btnSelectOne.AutoSet(NXB_DOWN);
	m_btnSelectAll.AutoSet(NXB_DDOWN);
	m_btnUnselectOne.AutoSet(NXB_UP);
	m_btnUnselectAll.AutoSet(NXB_UUP);

	m_btnAutoNumber.AutoSet(NXB_MODIFY);
	m_btnClear.AutoSet(NXB_MODIFY);

	m_UnselectedList = BindNxDataListCtrl(this,IDC_UNSELECTED_REFUND_LIST,GetRemoteData(),true);
	m_SelectedList = BindNxDataListCtrl(this,IDC_SELECTED_REFUND_LIST,GetRemoteData(),false);

	try {

		// (j.jones 2005-10-06 17:50) - PLID 17831 - I removed this functionality because realistically
		// the check number will rarely be the next sequential number from the last refund made, and we
		// don't want to cause them to accidentally apply the wrong numbers.

		/*
		//get last check number
		_RecordsetPtr rs = CreateRecordset("SELECT Max(CheckNo) AS NewCheckNo FROM "
			"LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID INNER JOIN PaymentPlansT ON LineItemT.ID = PaymentPlansT.ID "
			"WHERE Deleted = 0 AND LineItemT.Type = 3 AND PaymentsT.PayMethod = 8 AND (CheckNo Is Not Null OR CheckNo <> '') ");

		if(!rs->eof) {
			CString strCheckNo = AdoFldString(rs, "NewCheckNo","");
			//it could have characters in it, if so, we'll ignore it
			int nCheckNo = atoi(strCheckNo);
			if(nCheckNo > 0) {
				nCheckNo++;
				SetDlgItemInt(IDC_STARTING_CHECK_NO,nCheckNo);
			}
		}
		rs->Close();
		*/
	
	}NxCatchAll("Error loading last check number.");

	UpdateTotals();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrepareRefundChecksDlg::OnDblClickCellUnselectedRefundList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1 || nColIndex == COLUMN_CHECK_NO)
		return;

	m_UnselectedList->CurSel = nRowIndex;
	OnSelectOneRefund();
}

void CPrepareRefundChecksDlg::OnDblClickCellSelectedRefundList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1 || nColIndex == COLUMN_CHECK_NO)
		return;

	m_SelectedList->CurSel = nRowIndex;
	OnUnselectOneRefund();
}

void CPrepareRefundChecksDlg::OnEditingFinishingUnselectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	
}

void CPrepareRefundChecksDlg::OnEditingFinishingSelectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	
}

void CPrepareRefundChecksDlg::OnEditingFinishedUnselectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	
}

void CPrepareRefundChecksDlg::OnEditingFinishedSelectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	
}

void CPrepareRefundChecksDlg::OnSelectOneRefund() 
{
	if (m_UnselectedList->GetCurSel()!=-1)
		m_SelectedList->TakeCurrentRow(m_UnselectedList);
	
	UpdateTotals();
}

void CPrepareRefundChecksDlg::OnSelectAllRefunds() 
{
	m_SelectedList->TakeAllRows(m_UnselectedList);
	
	UpdateTotals();
}

void CPrepareRefundChecksDlg::OnUnselectOneRefund() 
{
	if (m_SelectedList->GetCurSel()!=-1)
		m_UnselectedList->TakeCurrentRow(m_SelectedList);
	
	UpdateTotals();
}

void CPrepareRefundChecksDlg::OnUnselectAllRefunds() 
{
	m_UnselectedList->TakeAllRows(m_SelectedList);
	
	UpdateTotals();
}

BOOL CPrepareRefundChecksDlg::Save(BOOL bPreviewReport) 
{
	try {

		//first check and see if any refunds are blank

		BOOL bBlank = FALSE;

		for(int i=0; i<m_UnselectedList->GetRowCount() && !bBlank; i++) {
			CString strCheckNo = VarString(m_UnselectedList->GetValue(i,COLUMN_CHECK_NO),"");
			strCheckNo.TrimRight();
			if(strCheckNo.IsEmpty()) {
				bBlank = TRUE;
			}
		}

		for(i=0; i<m_SelectedList->GetRowCount() && !bBlank; i++) {
			CString strCheckNo = VarString(m_SelectedList->GetValue(i,COLUMN_CHECK_NO),"");
			strCheckNo.TrimRight();
			if(strCheckNo.IsEmpty()) {
				bBlank = TRUE;
			}
		}

		if(bBlank) {
			if(IDNO == MessageBox("There are still some refunds without check numbers. Do you still wish to save?\n"
				"(If 'No', you can still update these numbers later.)","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return FALSE;
			}
		}

		CString strIDs;

		//save
		for(i=0; i<m_UnselectedList->GetRowCount(); i++) {
			CString strCheckNo = VarString(m_UnselectedList->GetValue(i,COLUMN_CHECK_NO),"");
			strCheckNo.TrimLeft();
			strCheckNo.TrimRight();
			if(!strCheckNo.IsEmpty()) {
				long nID = VarLong(m_UnselectedList->GetValue(i,COLUMN_ID),-1);
				ExecuteSql("UPDATE PaymentPlansT SET CheckNo = '%s' WHERE ID = %li",_Q(strCheckNo),nID);
				
				//add to the ID list
				CString str;
				str.Format("%li,",nID);
				strIDs += str;
			}
		}

		for(i=0; i<m_SelectedList->GetRowCount(); i++) {
			CString strCheckNo = VarString(m_SelectedList->GetValue(i,COLUMN_CHECK_NO),"");
			strCheckNo.TrimLeft();
			strCheckNo.TrimRight();
			if(!strCheckNo.IsEmpty()) {
				long nID = VarLong(m_SelectedList->GetValue(i,COLUMN_ID),-1);
				ExecuteSql("UPDATE PaymentPlansT SET CheckNo = '%s' WHERE ID = %li",_Q(strCheckNo),nID);

				//add to the ID list
				CString str;
				str.Format("%li,",nID);
				strIDs += str;
			}
		}

		if(bPreviewReport) {
			if(!strIDs.IsEmpty()) {
				strIDs.TrimRight(",");

				//preview the report, filtered by these ID numbers

				CWaitCursor pWait;
				
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(171)]);
				CPrintDialog* dlg;
				dlg = new CPrintDialog(FALSE);
				CPrintInfo prInfo;
				prInfo.m_bPreview = false;
				prInfo.m_bDirect = false;
				prInfo.m_bDocObject = false;
				if (prInfo.m_pPD) delete prInfo.m_pPD;
				prInfo.m_pPD = dlg;

				//Set up the parameters.
				CPtrArray paParams;
				CRParameterInfo *paramInfo;
				paramInfo = new CRParameterInfo;
				paramInfo->m_Data = "01/01/1000";
				paramInfo->m_Name = "DateFrom";
				paParams.Add((void *)paramInfo);

				paramInfo = new CRParameterInfo;
				paramInfo->m_Data = "12/31/5000";
				paramInfo->m_Name = "DateTo";
				paParams.Add((void *)paramInfo);

				if(infReport.strFilterString.IsEmpty()) {
					infReport.strFilterString.Format(" {PaymentsByProviderSubQ.ID} IN (%s) ", strIDs);
				}
				else {
					infReport.strFilterString.Format(" OR {PaymentsByProviderSubQ.ID} IN (%s) ", strIDs);
				}

				infReport.strReportFile += "ServiceDtld";

				RunReport(&infReport, &paParams, true, this, "Updated Refund Checks", &prInfo);
				ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
			}
			else {
				AfxMessageBox("No check numbers were generated. The report will not be run.");
			}
		}

		return TRUE;

	}NxCatchAll("Error saving check numbers.");

	return FALSE;
}

void CPrepareRefundChecksDlg::OnCancel() 
{
	try {

		BOOL bFound = FALSE;

		for(int i=0; i<m_SelectedList->GetRowCount(); i++) {
			CString strCheckNo = VarString(m_SelectedList->GetValue(i,COLUMN_CHECK_NO),"");
			strCheckNo.TrimRight();
			if(!strCheckNo.IsEmpty()) {
				bFound = TRUE;
			}
		}

		if(bFound) {
			//the user entered at least one check number
			if(IDNO == MessageBox("You have entered in check numbers. These will not be saved if you Cancel.\n"
				"Are you sure you wish to cancel and leave these refunds without any check numbers?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		
		CNxDialog::OnCancel();

	}NxCatchAll("Error verifying check number status,");
}

void CPrepareRefundChecksDlg::OnRequeryFinishedUnselectedRefundList(short nFlags) 
{
	if(m_SelectedList)
		UpdateTotals();
}

void CPrepareRefundChecksDlg::UpdateTotals()
{
	long nUnselectedCount = m_UnselectedList->GetRowCount();
	long nSelectedCount = m_SelectedList->GetRowCount();

	SetDlgItemInt(IDC_UNSELECTED_TOTAL,nUnselectedCount);
	SetDlgItemInt(IDC_SELECTED_TOTAL,nSelectedCount);
}

void CPrepareRefundChecksDlg::OnBtnAutonumber() 
{
	try {

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("Please place at least one refund into the Selected List.");
			return;
		}

		long nDefaultCheckNumber = GetDlgItemInt(IDC_STARTING_CHECK_NO);

		if(nDefaultCheckNumber <= 0) {
			AfxMessageBox("Please enter a starting check number greater than zero.");
			return;
		}

		for(int i=0; i<m_SelectedList->GetRowCount(); i++) {
			CString strCheckNo = VarString(m_SelectedList->GetValue(i,COLUMN_CHECK_NO),"");
			strCheckNo.TrimRight();
			if(strCheckNo.IsEmpty()) {
				strCheckNo.Format("%li",nDefaultCheckNumber);
				m_SelectedList->PutValue(i,COLUMN_CHECK_NO,_bstr_t(strCheckNo));
				nDefaultCheckNumber++;
			}
		}

		//incase they were sorting by check number
		m_SelectedList->Sort();

		//set the next check number
		SetDlgItemInt(IDC_STARTING_CHECK_NO,nDefaultCheckNumber);

	}NxCatchAll("Error generating check numbers.");	
}

void CPrepareRefundChecksDlg::OnBtnClear() 
{
	try {

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("Please place at least one refund into the Selected List.");
			return;
		}

		if(IDYES == MessageBox("This action will clear out the check numbers for all refunds in the Selected List.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {

			for(int i=0; i<m_SelectedList->GetRowCount(); i++) {
				m_SelectedList->PutValue(i,COLUMN_CHECK_NO,_bstr_t(""));
			}

			// (j.jones 2005-10-06 17:45) - PLID 17831 - revert the starting number to be blank
			SetDlgItemText(IDC_STARTING_CHECK_NO, "");
		}

	}NxCatchAll("Error clearing check numbers.");	
}

void CPrepareRefundChecksDlg::OnBtnSaveAndClose() 
{
	if(!Save(FALSE))
		return;
	
	CDialog::OnOK();
}

void CPrepareRefundChecksDlg::OnBtnSaveAndPreview() 
{
	if(!Save(TRUE))
		return;
	
	CDialog::OnOK();
}
