// LockboxManageDlg.cpp : implementation file

// (b.spivey - July 21st, 2014) - PLID 62959 - Created. 

#include "stdafx.h"
#include "Practice.h"
#include "LockboxManageDlg.h"
#include "afxdialogex.h"
#include "LockBoxPaymentImportDlg.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "ReportInfo.h"


// CLockboxManageDlg dialog
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CLockboxManageDlg, CNxDialog)

#define ID_EDIT_LOCKBOX_PAYMENT 65000
#define ID_PRINT_PREVIEW		65001
#define ID_DELETE_PAYMENT		65002 // (b.spivey, August 13th, 2014) - PLID 63361 - 

CLockboxManageDlg::CLockboxManageDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLockboxManageDlg::IDD, pParent)
{

}

CLockboxManageDlg::~CLockboxManageDlg()
{
}

void CLockboxManageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_Close);
	DDX_Control(pDX, IDC_DTP_START_DATE, m_dtpStartDate);
	DDX_Control(pDX, IDC_DTP_END_DATE, m_dtpEndDate);
	DDX_Control(pDX, IDC_LOCKBOX_CHECK_START_DATE, m_checkStartDate);
	DDX_Control(pDX, IDC_LOCKBOX_CHECK_END_DATE, m_checkEndDate);
	DDX_Control(pDX, IDC_PRINT_PREVIEW, m_PrintPreview);
	DDX_Control(pDX, IDC_REFRESH_LOCKBOX, m_Refresh);
	DDX_Control(pDX, IDC_EDIT_LOCKBOX_PAYMENT, m_Edit);
	DDX_Control(pDX, IDC_DELETE_LOCKBOX_PAYMENT, m_Delete);
	DDX_Control(pDX, IDC_RADIO_DEPOSIT_DATE, m_RadioDepostDate); 
	DDX_Control(pDX, IDC_RADIO_PAYMENT_DATE, m_RadioPaymentDate);
}

BEGIN_MESSAGE_MAP(CLockboxManageDlg, CNxDialog)
	ON_BN_CLICKED(IDC_LOCKBOX_CHECK_END_DATE, OnBnClickedCheckEndDate)
	ON_BN_CLICKED(IDC_LOCKBOX_CHECK_START_DATE, OnBnClickedCheckBeginDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DTP_START_DATE, OnChangeStartDateFilter)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DTP_END_DATE, OnChangeEndDateFilter)
	ON_BN_CLICKED(IDC_PRINT_PREVIEW, &CLockboxManageDlg::OnBnClickedPrintPreview)
	ON_BN_CLICKED(IDC_REFRESH_LOCKBOX, &CLockboxManageDlg::OnBnClickedRefreshLockbox)
	ON_BN_CLICKED(IDC_EDIT_LOCKBOX_PAYMENT, &CLockboxManageDlg::OnBnClickedEditLockboxPayment)
	ON_BN_CLICKED(IDC_DELETE_LOCKBOX_PAYMENT, &CLockboxManageDlg::OnBnClickedDeleteLockboxPayment)
	ON_BN_CLICKED(IDC_RADIO_DEPOSIT_DATE, &CLockboxManageDlg::OnBnClickedRadioDepositDate)
	ON_BN_CLICKED(IDC_RADIO_PAYMENT_DATE, &CLockboxManageDlg::OnBnClickedRadioPaymentInputDate)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLockboxManageDlg, CNxDialog)
	ON_EVENT(CLockboxManageDlg, IDC_LOCKBOX_LOCATION_FILTER, 16, OnSelChosenLocationFilter, VTS_DISPATCH)
	ON_EVENT(CLockboxManageDlg, IDC_LOCKBOX_BATCH_LIST, 3, OnDblClickCellLockBoxBatchList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CLockboxManageDlg, IDC_LOCKBOX_BATCH_LIST, 6, OnRButtonDownLockBoxBatchList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLockboxManageDlg, IDC_LOCKBOX_BATCH_LIST, 28, CurSelWasSetLockboxBatchList, VTS_NONE)
	ON_EVENT(CLockboxManageDlg, IDC_LOCKBOX_LOCATION_FILTER, 1, OnSelChangingLocationFilter, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

BOOL CLockboxManageDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		m_dlBatchList = BindNxDataList2Ctrl(IDC_LOCKBOX_BATCH_LIST, true);
		m_dlLocationFilter = BindNxDataList2Ctrl(IDC_LOCKBOX_LOCATION_FILTER, true);

		m_Close.AutoSet(NXB_CLOSE);
		m_PrintPreview.AutoSet(NXB_PRINT_PREV);
		m_Refresh.AutoSet(NXB_REFRESH);
		m_Edit.AutoSet(NXB_MODIFY);
		m_Delete.AutoSet(NXB_DELETE); 

		m_dtpEndDate.EnableWindow(FALSE);
		m_dtpStartDate.EnableWindow(FALSE);

		m_dlLocationFilter->SetSelByColumn(elblcID, -1);

		SetControlState(FALSE); 

		// (b.spivey - October 1, 2014) - PLID 63814 - 
		m_RadioDepostDate.SetCheck(TRUE); 
		m_RadioPaymentDate.SetCheck(FALSE); 
		m_LastRadioChecked = IDC_RADIO_DEPOSIT_DATE;

	} NxCatchAll(__FUNCTION__); 

	return TRUE;
}

// (b.spivey - July 21st, 2014) - PLID 62958 - Activate the end date filter
void CLockboxManageDlg::OnBnClickedCheckEndDate()
{
	try {

		if (m_checkEndDate.GetCheck()) {
			m_dtpEndDate.EnableWindow(TRUE); 
		}
		else {
			m_dtpEndDate.EnableWindow(FALSE); 
		}

		m_Refresh.SetTextColor(RGB(255, 0, 0)); 
	} NxCatchAll(__FUNCTION__); 
}

// (b.spivey - July 21st, 2014) - PLID 62958 - Activate the begin date filter
void CLockboxManageDlg::OnBnClickedCheckBeginDate()
{
	try {
		if (m_checkStartDate.GetCheck()) {
			m_dtpStartDate.EnableWindow(TRUE);
		}
		else {
			m_dtpStartDate.EnableWindow(FALSE);
		}

		m_Refresh.SetTextColor(RGB(255, 0, 0));
	} NxCatchAll(__FUNCTION__)
}

// (b.spivey - July 21st, 2014) - PLID 62958 - start filtering on locations
void CLockboxManageDlg::OnSelChosenLocationFilter(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_Refresh.SetTextColor(RGB(255, 0, 0));
		}

	} NxCatchAll(__FUNCTION__)
}

void CLockboxManageDlg::OnSelChangingLocationFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		//selecting nothing is meaningless, so disable that ability
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey - July 21st, 2014) - PLID 62958 - when the date changes, update the list. 
void CLockboxManageDlg::OnChangeStartDateFilter(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		COleDateTime dtStartDate = VarDateTime(m_dtpStartDate.GetValue(), GetDateTimeNull());
		COleDateTime dtEndDate = VarDateTime(m_dtpEndDate.GetValue(), GetDateTimeNull());

		//Don't allow invalid dates. 
		if (dtStartDate > dtEndDate) {
			m_dtpEndDate.SetValue(dtStartDate);
		}
		m_Refresh.SetTextColor(RGB(255, 0, 0));
	} NxCatchAll(__FUNCTION__)
}

// (b.spivey - July 21st, 2014) - PLID 62958 - when the date changes, update the list. 
void CLockboxManageDlg::OnChangeEndDateFilter(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		COleDateTime dtStartDate = VarDateTime(m_dtpStartDate.GetValue(), GetDateTimeNull());
		COleDateTime dtEndDate = VarDateTime(m_dtpEndDate.GetValue(), GetDateTimeNull());

		//Don't allow invalid dates. 
		if (dtEndDate < dtStartDate) {
			m_dtpStartDate.SetValue(dtEndDate);
		}
		m_Refresh.SetTextColor(RGB(255, 0, 0));
	} NxCatchAll(__FUNCTION__)
}

// (b.spivey - September26, 2014) - PLID 62958 - Build me a where clause.
CString CLockboxManageDlg::BuildWhereClause()
{

	//Get the locationID 
	long nLocationID = -2;
	IRowSettingsPtr pRow = m_dlLocationFilter->GetCurSel();

	if (pRow) {
		nLocationID = VarLong(pRow->GetValue(elblcID), -2);
	}

	COleDateTime dtStart = AsDateNoTime(m_dtpStartDate.GetValue());
	COleDateTime dtEnd = AsDateNoTime(m_dtpEndDate.GetValue());

	CSqlFragment sqlWhere("(Deleted = 0) "); 
	if (nLocationID > 0) {
		CSqlFragment sql(" AND (LocationID = {INT} ) ", nLocationID);
		sqlWhere += sql;
	}
	else {
		CSqlFragment sql(" AND (1 = 1) ");
		sqlWhere += sql;
	}

	if (m_RadioDepostDate.GetCheck()) {
		if (m_checkEndDate.GetCheck()) {
			CSqlFragment sql(" AND (PaymentDate < DateAdd(d, 1, {OLEDATETIME})) ", dtEnd);
			sqlWhere += sql;
		}
		else {
			CSqlFragment sql(" AND (1 = 1) ");
			sqlWhere += sql;
		}

		if (m_checkStartDate.GetCheck()) {
			CSqlFragment sql(" AND (PaymentDate >= {OLEDATETIME}) ", dtStart);
			sqlWhere += sql;
		}
		else {
			CSqlFragment sql(" AND (1 = 1) ");
			sqlWhere += sql;
		}
	}
	// (b.spivey - October 1, 2014) - PLID 63814 - input date check. 
	else if (m_RadioPaymentDate.GetCheck()) {

		CSqlFragment sqlStartDate, sqlEndDate;
		// (b.spivey - October 3, 2014) - PLID 63814 -Cleaned up variable names
		if (m_checkStartDate.GetCheck()) {
			sqlStartDate.Create(" (LIT.InputDate >= {OLEDATETIME}) ", dtStart);
		}
		else {
			sqlStartDate.Create(" (1 = 1) ");
		}

		if (m_checkEndDate.GetCheck()) {
			sqlEndDate.Create(" AND (LIT.InputDate < DateAdd(d, 1, {OLEDATETIME})) ", dtEnd);
		}
		else {
			sqlEndDate.Create(" AND (1 = 1) ");
		}

		// (b.spivey - October 3, 2014) - PLID 63814 - filter out deleted payments. 
		CSqlFragment sql(R"( AND ID IN (
			SELECT DISTINCT Batch.ID 
			FROM LineItemT LIT 
			INNER JOIN PaymentsT PT ON LIT.ID = PT.ID
			INNER JOIN LockboxPaymentMapT Map ON PT.ID = Map.PaymentID 
			INNER JOIN LockboxPaymentT LockboxPay ON Map.LockboxPaymentID = LockboxPay.ID 
			INNER JOIN LockboxBatchT Batch ON LockboxPay.LockboxBatchID = Batch.ID
			WHERE {SQL} {SQL} AND LIT.Deleted = 0)
			)", sqlStartDate, sqlEndDate);

		sqlWhere += sql; 		
	}
	else {
		//Why did we hit this? it's a radio select that has a default. 
		ASSERT(TRUE);
	}

	return sqlWhere.Flatten();
}

// (b.spivey - July 21st, 2014) - PLID 62960 - When double clicking, open the edit payment dialog. 
void CLockboxManageDlg::OnDblClickCellLockBoxBatchList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		GetMainFrame()->ShowLockboxPaymentImportDlg(VarLong(pRow->GetValue(elbbcID)));
	} NxCatchAll(__FUNCTION__); 
}

// (b.spivey - July 21st, 2014) - PLID 62960 - Right click option for editing the payment. 
void CLockboxManageDlg::OnRButtonDownLockBoxBatchList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		else m_dlBatchList->PutCurSel(pRow);

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_STRING, ID_EDIT_LOCKBOX_PAYMENT, "Edit Deposit");
		mnu.AppendMenu(MF_STRING, ID_DELETE_PAYMENT, "Delete Deposit");
		mnu.AppendMenu(MF_STRING, ID_PRINT_PREVIEW, "Print Preview");
		CPoint pt;
		GetCursorPos(&pt);

		switch (mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this))
		{
		case ID_EDIT_LOCKBOX_PAYMENT:

			GetMainFrame()->ShowLockboxPaymentImportDlg(VarLong(pRow->GetValue(elbbcID)));
			break;

			// (b.spivey, August 11th, 2014) - PLID 62965 - Print Preview. 
		case ID_PRINT_PREVIEW:
		{
			long nID = VarLong(pRow->GetValue(elbbcID));
			COleDateTime dt = VarDateTime(pRow->GetValue(elbbcPaymentDate));

			PrintPreviewReport(nID, dt); 
			break;
		}
			// (b.spivey, August 18, 2014) - PLID 63361 - Delete a batch payment. 
		case ID_DELETE_PAYMENT:
		{
			long nBatchID = VarLong(pRow->GetValue(elbbcID));
			if (DeleteLockboxBatch(nBatchID)) {
				m_dlBatchList->RemoveRow(pRow);
			}
			break;
		}
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__); 
}

// (b.spivey, August 13th, 2014) - PLID 63361 - Delete a lockbox batch if it's not linked to anything. 
bool CLockboxManageDlg::DeleteLockboxBatch(long nBatchID)
{
	if (!(nBatchID > 0)) {
		return false;
	}

	if (!ReturnsRecordsParam(
		R"( 
			SELECT TOP 1 LBT.ID 
			FROM LockboxBatchT LBT 
			LEFT JOIN LockboxPaymentT LBPT ON LBT.ID = LBPT.LockboxBatchID 
			LEFT JOIN LockboxPaymentMapT LBPMT ON LBPT.ID = LBPMT.LockboxPaymentID 
			LEFT JOIN PaymentsT PT ON LBPMT.PaymentID = PT.ID  
			LEFT JOIN LineItemT LIT ON PT.ID = LIT.ID 
			WHERE LIT.Deleted = 0 AND LBPT.LockboxBatchID = {INT}
			)", nBatchID)) {


		ExecuteParamSql(
			R"( 
				UPDATE LockboxBatchT SET DeletedDate = GetDate(), Deleted = 1 WHERE ID = {INT} 
			)", nBatchID);
	}
	else {
		// (b.spivey, October 14th, 2014) - PLID 63361 - Prompt them about if they want to see the report preview.
		if (MessageBox("The lockbox deposit you have selected to delete has payments applied to patient accounts. "
			"You must delete the payments before you can delete the lockbox deposit. \r\n\r\n"
			"Would you like to preview the report that indicates which patients these payments are applied to?",
			"Unable to delete Lockbox Payment", MB_YESNO|MB_ICONWARNING) == IDYES) {

			IRowSettingsPtr pRow = m_dlBatchList->GetCurSel();
			if (pRow) {
				PrintPreviewReport(VarLong(pRow->GetValue(elbbcID)), VarDateTime(pRow->GetValue(elbbcPaymentDate)));
			}
		}


		return false; 
	}

	return true; 
}

// (b.spivey, August 18, 2014) - PLID 62965 - Print preview a lockbox batch 
void CLockboxManageDlg::OnBnClickedPrintPreview()
{
	try {
		
		IRowSettingsPtr pRow = m_dlBatchList->GetCurSel();
		if (pRow) {

			PrintPreviewReport(VarLong(pRow->GetValue(elbbcID)), VarDateTime(pRow->GetValue(elbbcPaymentDate)));
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 18, 2014) - PLID 62959 - Refresh list. 
void CLockboxManageDlg::OnBnClickedRefreshLockbox()
{
	try {

		// (b.spivey - September26, 2014) - PLID 62958
		CString str = BuildWhereClause();
		m_Refresh.SetTextColor(RGB(0, 0, 0));
		m_dlBatchList->WhereClause = _bstr_t(str); 
		m_dlBatchList->Requery(); 
		m_dlBatchList->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely); 
		// (b.spivey - October 1, 2014) - PLID 63814 - track which is checked when we refresh.
		if (m_RadioDepostDate.GetCheck()) {
			m_LastRadioChecked = IDC_RADIO_DEPOSIT_DATE;
		}
		else if (m_RadioPaymentDate.GetCheck()) {
			m_LastRadioChecked = IDC_RADIO_PAYMENT_DATE; 
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 18, 2014) - PLID 62960 - Edit lockbox batch
void CLockboxManageDlg::OnBnClickedEditLockboxPayment()
{
	try {
		IRowSettingsPtr pRow = m_dlBatchList->GetCurSel();
		if (pRow) {
			GetMainFrame()->ShowLockboxPaymentImportDlg(VarLong(pRow->GetValue(elbbcID)));
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 18, 2014) - PLID 63361 - Delete a lockbox batch
void CLockboxManageDlg::OnBnClickedDeleteLockboxPayment()
{
	try {
		IRowSettingsPtr pRow = m_dlBatchList->GetCurSel();
		if (pRow) {
			if (DeleteLockboxBatch(VarLong(pRow->GetValue(elbbcID)))) {
				m_dlBatchList->RemoveRow(pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 18, 2014) - PLID 62959 - Set control state
void CLockboxManageDlg::SetControlState(BOOL bControlState)
{
	m_Delete.EnableWindow(bControlState);
	m_PrintPreview.EnableWindow(bControlState);
	m_Edit.EnableWindow(bControlState); 
}

// (b.spivey, August 18, 2014) - PLID 62959 - Set the control state every time this selection changes. 
void CLockboxManageDlg::CurSelWasSetLockboxBatchList()
{
	try {

		if (!m_dlBatchList)
		{
			SetControlState(FALSE);
			return;
		}

		IRowSettingsPtr pRow = m_dlBatchList->GetCurSel();

		if (!pRow) {
			SetControlState(FALSE);
		}
		else {
			SetControlState(TRUE);
		}
	} NxCatchAll(__FUNCTION__); 
}

	// (b.spivey, September 30, 2014) - PLID 62965 - Print preview the report. 
void CLockboxManageDlg::PrintPreviewReport(long nBatchID, COleDateTime dtBatchDate)
{
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(757)]);
	infReport.bExternal = TRUE;
	infReport.strExternalFilter = FormatString("LBBatch.ID = %li", nBatchID);
	infReport.nDateRange = -1; //All dates
	infReport.nDateFilter = 1;

	CRParameterInfo *paramInfo;
	CPtrArray paParams;

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = FormatDateTimeForInterface(dtBatchDate, DTF_STRIP_SECONDS);
	paramInfo->m_Name = "DateFrom";
	paParams.Add((void *)paramInfo);

	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = FormatDateTimeForInterface(dtBatchDate, DTF_STRIP_SECONDS);
	paramInfo->m_Name = "DateTo";
	paParams.Add((void *)paramInfo);

	RunReport(&infReport, &paParams, TRUE, this, "Lockbox Payment Report");
	ClearRPIParameterList(&paParams);
	OnCancel();
}

// (b.spivey - October 1, 2014) - PLID 63814 - if it's different, show the refresh button as red
void CLockboxManageDlg::OnBnClickedRadioDepositDate()
{
	try {
		if (m_LastRadioChecked != IDC_RADIO_DEPOSIT_DATE) {
			m_Refresh.SetTextColor(RGB(255, 0, 0));
		}
	}NxCatchAll(__FUNCTION__);
}

void CLockboxManageDlg::OnBnClickedRadioPaymentInputDate()
{
	try {
		if (m_LastRadioChecked != IDC_RADIO_PAYMENT_DATE) {
			m_Refresh.SetTextColor(RGB(255, 0, 0));
		}
	}NxCatchAll(__FUNCTION__); 
}