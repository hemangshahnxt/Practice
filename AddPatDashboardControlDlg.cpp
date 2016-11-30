// AddPatDashboardControlDlg.cpp : implementation file
//

// (j.gruber 2012-04-13 15:46) - PLID 49700 - created for

#include "stdafx.h"
#include "AddPatDashboardControlDlg.h"
#include "PatientsRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddPatDashboardControlDlg dialog

enum typeListColumn
{
	tlcID = 0,
	tlcName,
};

enum ApptTypeListColumn
{
	atlcID = 0,
	atlcChecked,
	atlcName,
};

enum TimeIntervalColumn
{
	ticID = 0,
	ticName,
};

enum EMNItemListColumn
{
	eilcID = 0,
	eilcName,
};

CAddPatDashboardControlDlg::CAddPatDashboardControlDlg(PDControl *pControl, BOOL bWriteable, CWnd* pParent /*=NULL*/)
	: CNxDialog(CAddPatDashboardControlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddPatDashboardControlDlg)
	m_pControl = pControl;
	m_bWriteable = bWriteable;
	//}}AFX_DATA_INIT
}

BEGIN_EVENTSINK_MAP(CAddPatDashboardControlDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAddPatDashboardControlDlg)
	ON_EVENT(CAddPatDashboardControlDlg, IDC_APD_APPT_TYPE_LIST, 18 /* RequeryFinished */, CAddPatDashboardControlDlg::OnRequeryFinishedApptTypeList, VTS_I2)	
	ON_EVENT(CAddPatDashboardControlDlg, IDC_APD_HISTORY_CAT_LIST, 18 /* RequeryFinished */, CAddPatDashboardControlDlg::OnRequeryFinishedHistoryCatList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CAddPatDashboardControlDlg, IDC_APD_EMN_ITEM_LIST, 18, CAddPatDashboardControlDlg::RequeryFinishedApdEmnItemList, VTS_I2)
	ON_EVENT(CAddPatDashboardControlDlg, IDC_APD_TYPE_LIST, 16, CAddPatDashboardControlDlg::SelChosenApdTypeList, VTS_DISPATCH)
	ON_EVENT(CAddPatDashboardControlDlg, IDC_APD_TIME_INTERVAL, 16, CAddPatDashboardControlDlg::SelChosenApdTimeInterval, VTS_DISPATCH)
	ON_EVENT(CAddPatDashboardControlDlg, IDC_APD_EMN_ITEM_LIST, 16, CAddPatDashboardControlDlg::SelChosenApdEmnItemList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CAddPatDashboardControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_PAT_DASH_BKG, m_bkg);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//{{AFX_DATA_MAP(CAddPatDashboardControlDlg)		
	

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddPatDashboardControlDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAddPatDashboardControlDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddPatDashboardControlDlg message handlers

// (z.manning 2015-05-07 11:18) - NX-100439 - Removed name param as it is not needed
void CAddPatDashboardControlDlg::AddTypeRow(PatientDashboardType type)
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->GetNewRow();
	if (pRow) {
		pRow->PutValue(tlcID, type);
		pRow->PutValue(tlcName, _bstr_t(GetDashboardTypeNameFromID(type)));

		m_pTypeList->AddRowAtEnd(pRow, NULL);
	}

}

void CAddPatDashboardControlDlg::AddTimeIntervalRow(LastTimeFormat ltf, CString strName)
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->GetNewRow();
	if (pRow) {
		pRow->PutValue(ticID, ltf);
		pRow->PutValue(ticName, _variant_t(strName));

		m_pTimeIntervalList->AddRowAtEnd(pRow, NULL);
	}

}

BOOL CAddPatDashboardControlDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_pTypeList = BindNxDataList2Ctrl(IDC_APD_TYPE_LIST, false);
		m_pEMNItemList = BindNxDataList2Ctrl(IDC_APD_EMN_ITEM_LIST, false);
		m_pTimeIntervalList = BindNxDataList2Ctrl(IDC_APD_TIME_INTERVAL, false);
		m_pApptTypeList = BindNxDataList2Ctrl(IDC_APD_APPT_TYPE_LIST, true);
		m_pHistoryCategoryList = BindNxDataList2Ctrl(IDC_APD_HISTORY_CAT_LIST, true); // (c.haag 2015-04-29) - NX-100441

		((CEdit*)GetDlgItem(IDC_APD_TITLE))->SetLimitText(200);

		m_pEMNItemList->WhereClause =  " ActiveEMRInfoID > 0  AND " 
			//we don't support images and narratives
			"DataType NOT IN (-1, 4, 6)";
		m_pEMNItemList->Requery();

		// (z.manning 2015-04-15 10:56) - NX-100388 - Set a limit to the time increment field
		((CEdit*)GetDlgItem(IDC_APD_TIME_INCREMENT))->SetLimitText(4);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);	

		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		AddTypeRow(pdtAllergies);
		AddTypeRow(pdtCurrentMeds);
		AddTypeRow(pdtPrescriptions);
		AddTypeRow(pdtHistoryImages);
		AddTypeRow(pdtEMNSummary);
		AddTypeRow(pdtProcedures);
		AddTypeRow(pdtDiagCodes);
		AddTypeRow(pdtCreateEMNs);
		AddTypeRow(pdtProblems);
		AddTypeRow(pdtLabs);
		AddTypeRow(pdtEMNItems);
		AddTypeRow(pdtAppointments);
		AddTypeRow(pdtBills);
		AddTypeRow(pdtHistoryAttachments); // (c.haag 2015-04-29) - NX-100441
		m_pTypeList->GetColumn(tlcName)->SortPriority = 0;
		m_pTypeList->Sort();

		AddTimeIntervalRow(ltfDay, "Day");
		AddTimeIntervalRow(ltfMonth, "Month");
		AddTimeIntervalRow(ltfYear, "Year");
		// (z.manning 2015-04-15 10:10) - NX-100388
		AddTimeIntervalRow(lftRecord, "Record");

		Load();
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddPatDashboardControlDlg::OnOK() 
{
	try {
		//shows messages
		if (Validate()) {
			Save();
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

void CAddPatDashboardControlDlg::MoveAndShowWindow(int IDToMove, int IDToMoveBelow, CString strCaption /*= ""*/) 
{
	CRect rctMoveBelow;
	GetDlgItem(IDToMoveBelow)->GetWindowRect(rctMoveBelow);
	ScreenToClient(rctMoveBelow);

	long nSpacer = GetSpacer();

	CRect rctMove;
	GetDlgItem(IDToMove)->GetWindowRect(rctMove);
	ScreenToClient(rctMove);		

	long nDiff = rctMove.top - rctMove.top;

	long nHeight = rctMove.Height();

	rctMove.top = rctMoveBelow.bottom + nSpacer;
	rctMove.bottom = rctMove.top + nHeight;

	GetDlgItem(IDToMove)->MoveWindow(rctMove);

	if (strCaption != "") {
		GetDlgItem(IDToMove)->SetWindowTextA(strCaption);
	}

	GetDlgItem(IDToMove)->ShowWindow(SW_SHOW);
	GetDlgItem(IDToMove)->EnableWindow(TRUE);

}


	
void CAddPatDashboardControlDlg::ShowIncludeOnly(CString strCaption)
{
	//we need to move the include 
	CRect rctType;
	GetDlgItem(IDC_APD_TYPE_LIST)->GetWindowRect(rctType);
	ScreenToClient(rctType);

	long nSpacer = GetSpacer();

	CRect rctInclude;
	GetDlgItem(IDC_APD_INCLUDE)->GetWindowRect(rctInclude);
	ScreenToClient(rctInclude);

	long nHeight = rctInclude.bottom - rctInclude.top;

	rctInclude.top = rctType.top + nSpacer;
	rctInclude.bottom = rctInclude.top + nHeight;

	GetDlgItem(IDC_APD_INCLUDE)->MoveWindow(rctInclude);
	SetDlgItemText(IDC_APD_INCLUDE, strCaption);

	GetDlgItem(IDC_APD_INCLUDE)->ShowWindow(TRUE);
	GetDlgItem(IDC_APD_INCLUDE)->EnableWindow(TRUE);
}

void CAddPatDashboardControlDlg::ShowInclude(int idcToShowBelow, long nPxBelow, CString strCaption)
{
	CRect rctBelow;
	GetDlgItem(idcToShowBelow)->GetWindowRect(rctBelow);
	ScreenToClient(rctBelow);

	CRect rctInclude;
	GetDlgItem(IDC_APD_INCLUDE)->GetWindowRect(rctInclude);
	ScreenToClient(rctInclude);	

	long nHeight = rctInclude.bottom - rctInclude.top;

	//now start our box nPxBelow px below the bottom
	rctInclude.top = rctBelow.bottom + nPxBelow;
	rctInclude.bottom = rctInclude.top + nHeight;

	GetDlgItem(IDC_APD_INCLUDE)->MoveWindow(rctInclude);
	SetDlgItemText(IDC_APD_INCLUDE, strCaption);

	GetDlgItem(IDC_APD_INCLUDE)->ShowWindow(TRUE);
	GetDlgItem(IDC_APD_INCLUDE)->EnableWindow(TRUE);
}

// (z.manning 2015-03-26 13:36) - NX-100399 - Function to show the EMN item list
void CAddPatDashboardControlDlg::ShowEmnItemList(UINT nIdcToShowBelow)
{
	CRect rctBelow;
	GetDlgItem(nIdcToShowBelow)->GetWindowRect(rctBelow);
	ScreenToClient(rctBelow);

	CRect rctDropdown, rctLabel;
	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->GetWindowRect(rctDropdown);
	ScreenToClient(rctDropdown);

	GetDlgItem(IDC_APD_EMN_ITEM_LIST_LABEL)->GetWindowRect(rctLabel);
	ScreenToClient(rctLabel);

	long nNewX = rctBelow.bottom + GetSpacer();
	long nDiff = nNewX - rctDropdown.top;

	rctDropdown.top = rctDropdown.top + nDiff;
	rctDropdown.bottom = rctDropdown.bottom + nDiff;

	rctLabel.top = rctLabel.top + nDiff;
	rctLabel.bottom = rctLabel.bottom + nDiff;

	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->MoveWindow(rctDropdown);
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->MoveWindow(rctLabel);

	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->EnableWindow(TRUE);
}

void CAddPatDashboardControlDlg::ShowApptType(int idcToShowBelow, long nPxBelow)
{
	CRect rctBelow;
	GetDlgItem(idcToShowBelow)->GetWindowRect(rctBelow);
	ScreenToClient(rctBelow);

	CRect rctApptDesc, rctApptBox;
	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->GetWindowRect(rctApptDesc);
	ScreenToClient(rctApptDesc);	
	
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->GetWindowRect(rctApptBox);
	ScreenToClient(rctApptBox);	

	//find the different between where they are now
	long nSpacer = rctApptBox.top - rctApptDesc.bottom;

	long nDescHeight = rctApptDesc.bottom - rctApptDesc.top;
	long nBoxHeight = rctApptBox.bottom - rctApptBox.top;

	//now start our box nPxBelow px below the bottom
	rctApptDesc.top = rctBelow.bottom + nPxBelow;
	rctApptDesc.bottom = rctApptDesc.top + nDescHeight;

	rctApptBox.top = rctApptDesc.bottom + nSpacer;
	rctApptBox.bottom = rctApptBox.top + nBoxHeight;

	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->MoveWindow(rctApptDesc);
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->MoveWindow(rctApptBox);	

	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->ShowWindow(SW_SHOW);	

	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->EnableWindow(TRUE);
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->EnableWindow(TRUE);	
}

// (c.haag 2015-04-29) - NX-100441 - Show the History Categories list
void CAddPatDashboardControlDlg::ShowHistoryCategories(int idcToShowBelow, long nPxBelow)
{
	CRect rctBelow;
	GetDlgItem(idcToShowBelow)->GetWindowRect(rctBelow);
	ScreenToClient(rctBelow);

	CRect rctApptDesc, rctApptBox;
	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->GetWindowRect(rctApptDesc);
	ScreenToClient(rctApptDesc);

	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->GetWindowRect(rctApptBox);
	ScreenToClient(rctApptBox);

	//find the different between where they are now
	long nSpacer = rctApptBox.top - rctApptDesc.bottom;

	long nDescHeight = rctApptDesc.bottom - rctApptDesc.top;
	long nBoxHeight = rctApptBox.bottom - rctApptBox.top;

	//now start our box nPxBelow px below the bottom
	rctApptDesc.top = rctBelow.bottom + nPxBelow;
	rctApptDesc.bottom = rctApptDesc.top + nDescHeight;

	rctApptBox.top = rctApptDesc.bottom + nSpacer;
	rctApptBox.bottom = rctApptBox.top + nBoxHeight;

	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->MoveWindow(rctApptDesc);
	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->MoveWindow(rctApptBox);

	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->EnableWindow(TRUE);
	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->EnableWindow(TRUE);
}

// (z.manning 2015-03-26 14:49) - NX-100399 - Added IDC to show below param
void CAddPatDashboardControlDlg::ShowTimeInterval(UINT nIdcToShowBelow)
{
	MoveAndShowWindow(IDC_CHK_TIME, nIdcToShowBelow, "");

	CRect rctTime;
	GetDlgItem(IDC_CHK_TIME)->GetWindowRect(rctTime);
	ScreenToClient(rctTime);

	CRect rctTypeInc;
	GetDlgItem(IDC_APD_TIME_INCREMENT)->GetWindowRect(rctTypeInc);
	ScreenToClient(rctTypeInc);

	CRect rctTypeInt;
	GetDlgItem(IDC_APD_TIME_INTERVAL)->GetWindowRect(rctTypeInt);
	ScreenToClient(rctTypeInt);

	CRect rctTypeIntSt;
	GetDlgItem(IDC_APD_TIME_STATIC)->GetWindowRect(rctTypeIntSt);
	ScreenToClient(rctTypeIntSt);

	long nDiff = rctTypeIntSt.top - rctTime.top;
	
	rctTypeInc.top = rctTypeInc.top - nDiff;
	rctTypeInc.bottom = rctTypeInc.bottom - nDiff;

	rctTypeInt.top = rctTypeInt.top - nDiff;
	rctTypeInt.bottom = rctTypeInt.bottom - nDiff;

	rctTypeIntSt.top = rctTypeIntSt.top - nDiff;
	rctTypeIntSt.bottom = rctTypeIntSt.bottom - nDiff;

	GetDlgItem(IDC_APD_TIME_INCREMENT)->MoveWindow(rctTypeInc);
	GetDlgItem(IDC_APD_TIME_INTERVAL)->MoveWindow(rctTypeInt);
	GetDlgItem(IDC_APD_TIME_STATIC)->MoveWindow(rctTypeIntSt);

	GetDlgItem(IDC_APD_TIME_INCREMENT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_APD_TIME_INCREMENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_APD_TIME_INTERVAL)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_APD_TIME_INTERVAL)->EnableWindow(TRUE);
	GetDlgItem(IDC_APD_TIME_STATIC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_APD_TIME_STATIC)->EnableWindow(TRUE);
}

void CAddPatDashboardControlDlg::ResetPositions()
{
	MoveWindow(m_rctWindow);
	m_bkg.MoveWindow(m_rctBG);
	GetDlgItem(IDOK)->MoveWindow(m_rctOK);
	GetDlgItem(IDCANCEL)->MoveWindow(m_rctCancel);
	GetDlgItem(IDC_APD_INCLUDE)->MoveWindow(m_rctInclude);
	GetDlgItem(IDC_CHK_TIME)->MoveWindow(m_rctChkTime);
	GetDlgItem(IDC_APD_TIME_INCREMENT)->MoveWindow(m_rctTimeIncrement);
	GetDlgItem(IDC_APD_TIME_INTERVAL)->MoveWindow(m_rctTimeInterval);
	GetDlgItem(IDC_APD_TIME_STATIC)->MoveWindow(m_rctTimeStatic);
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->MoveWindow(m_rctInfoStatic);
	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->MoveWindow(m_rctItem);
	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->MoveWindow(m_rctApptDesc);
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->MoveWindow(m_rctApptBox);	
	GetDlgItem(IDC_DASH_NOSHOWCCDA)->MoveWindow(m_rctDoNotShowOnCCDA);// (s.tullis 2015-02-25 09:53) - PLID 64740 
	// (r.gonet 2015-03-17 10:23) - PLID 65020 - Reset the position of the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox.
	GetDlgItem(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT)->MoveWindow(m_rctExcludeDoNotShowOnProblemPrompt);
	// (c.haag 2015-04-29) - NX-100441 - History categories
	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->MoveWindow(m_rctHistCatDesc);
	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->MoveWindow(m_rctHistCatBox);
}

void CAddPatDashboardControlDlg::SetRects()
{
	GetWindowRect(m_rctWindow);
	ScreenToClient(m_rctWindow);

	m_bkg.GetWindowRect(m_rctBG);
	ScreenToClient(m_rctBG);

	GetDlgItem(IDOK)->GetWindowRect(m_rctOK);
	ScreenToClient(m_rctOK);

	GetDlgItem(IDCANCEL)->GetWindowRect(m_rctCancel);
	ScreenToClient(m_rctCancel);

	GetDlgItem(IDC_APD_INCLUDE)->GetWindowRect(m_rctInclude);
	ScreenToClient(m_rctInclude);

	GetDlgItem(IDC_CHK_TIME)->GetWindowRect(m_rctChkTime);
	ScreenToClient(m_rctChkTime);

	GetDlgItem(IDC_APD_TIME_INCREMENT)->GetWindowRect(m_rctTimeIncrement);
	ScreenToClient(m_rctTimeIncrement);

	GetDlgItem(IDC_APD_TIME_INTERVAL)->GetWindowRect(m_rctTimeInterval);
	ScreenToClient(m_rctTimeInterval);	

	GetDlgItem(IDC_APD_TIME_STATIC)->GetWindowRect(m_rctTimeStatic);
	ScreenToClient(m_rctTimeStatic);
	
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->GetWindowRect(m_rctInfoStatic);
	ScreenToClient(m_rctInfoStatic);

	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->GetWindowRect(m_rctItem);
	ScreenToClient(m_rctItem);

	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->GetWindowRect(m_rctApptDesc);
	ScreenToClient(m_rctApptDesc);

	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->GetWindowRect(m_rctApptBox);
	ScreenToClient(m_rctApptBox);

	// (s.tullis 2015-02-25 09:53) - PLID 64740 
	GetDlgItem(IDC_DASH_NOSHOWCCDA)->GetWindowRect(m_rctDoNotShowOnCCDA);
	ScreenToClient(m_rctDoNotShowOnCCDA);

	// (r.gonet 2015-03-17 10:23) - PLID 65020 - Obtain the position and size of the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox.
	GetDlgItem(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT)->GetWindowRect(m_rctExcludeDoNotShowOnProblemPrompt);
	ScreenToClient(m_rctExcludeDoNotShowOnProblemPrompt);

	// (c.haag 2015-04-29) - NX-100441 - History categories
	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->GetWindowRect(m_rctHistCatDesc);
	ScreenToClient(m_rctHistCatDesc);

	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->GetWindowRect(m_rctHistCatBox);
	ScreenToClient(m_rctHistCatBox);
}

void CAddPatDashboardControlDlg::ShowControls(PatientDashboardType type)
{	
	long nSpacer = GetSpacer();

	switch(type) {
		case pdtAllergies:
		case pdtCurrentMeds:
			//discontinued
			ResetPositions();			
			MoveAndShowWindow(IDC_APD_INCLUDE, IDC_APD_TYPE_LIST, "Include Discontinued");			
			MoveBottom(IDC_APD_INCLUDE);
		break;

		case pdtPrescriptions:
			//time, discontinued
			ResetPositions();			
			ShowTimeInterval(IDC_APD_TYPE_LIST);
			MoveAndShowWindow(IDC_APD_INCLUDE, IDC_CHK_TIME, "Include Discontinued");									
			MoveBottom(IDC_APD_INCLUDE);
		break;

		case pdtProblems:
			//include resolved
			ResetPositions();		
			MoveAndShowWindow(IDC_APD_INCLUDE, IDC_APD_TYPE_LIST, "Include Resolved");	
			MoveAndShowWindow(IDC_DASH_NOSHOWCCDA,IDC_APD_INCLUDE, "Exclude problems flagged to 'Do not show on CCDA'."); // (s.tullis 2015-02-25 09:53) - PLID 64740
			// (r.gonet 2015-03-17 10:23) - PLID 65020 - Add the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox below the other two checkboxes.
			MoveAndShowWindow(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT, IDC_DASH_NOSHOWCCDA, "Exclude problems flagged to 'Do not show on problem prompt'.");
			MoveBottom(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT);
		break;

		case pdtAppointments:			
			//time, type
			ResetPositions();		
			ShowTimeInterval(IDC_APD_TYPE_LIST);
			ShowApptType(IDC_CHK_TIME, nSpacer);
			MoveBottom(IDC_APD_APPT_TYPE_LIST);			
		break;

		case pdtBills:
		case pdtProcedures:
		// (z.manning 2015-03-24 17:28) - NX-100399 - Diags and labs now show time
		case pdtDiagCodes:
		case pdtLabs:
			//time
			ResetPositions();		
			ShowTimeInterval(IDC_APD_TYPE_LIST);
			MoveBottom(IDC_CHK_TIME);
		break;

		case pdtEMNItems:
			ResetPositions();
			//just need the item list here
			ShowEmnItemList(IDC_APD_TYPE_LIST);

			// (z.manning 2015-03-24 17:27) - NX-100399 - Added time here
			ShowTimeInterval(IDC_APD_EMN_ITEM_LIST);
			MoveBottom(IDC_CHK_TIME);
		break;

		case pdtHistoryImages:
		case pdtEMNSummary:
		case pdtCreateEMNs:
			ResetPositions();
			MoveBottom(IDC_APD_TYPE_LIST);
		break;

		case pdtHistoryAttachments: // (c.haag 2015-04-29) - NX-100441
			ResetPositions();
			ShowTimeInterval(IDC_APD_TYPE_LIST); // Show the time interval below the type list
			ShowHistoryCategories(IDC_CHK_TIME, nSpacer); // Show the history category list below the time interval checkbox
			MoveBottom(IDC_APD_HISTORY_CAT_LIST); // Move the history category list to the bottom
			break;

		default:
		break;
	}

	// (z.manning 2015-04-15 10:33) - NX-100388
	UpdateVisibleRows(type);
	UpdateTimeIntervalLabel();
}

// (z.manning 2015-04-15 10:31) - NX-100388
void CAddPatDashboardControlDlg::UpdateVisibleRows(PatientDashboardType type)
{
	NXDATALIST2Lib::IRowSettingsPtr pRecordsRow = m_pTimeIntervalList->FindByColumn(ticID, lftRecord, NULL, VARIANT_FALSE);
	if (pRecordsRow != NULL) {
		switch (type)
		{
		case pdtEMNItems:
			pRecordsRow->Visible = g_cvarTrue;
			break;
		default:
			pRecordsRow->Visible = g_cvarFalse;
			break;
		}
	}
}

int CAddPatDashboardControlDlg::GetSpacer()
{
	CRect rctType, rctTitle;
	GetDlgItem(IDC_APD_TYPE_LIST)->GetWindowRect(rctType);
	ScreenToClient(rctType);	
	
	GetDlgItem(IDC_APD_TITLE)->GetWindowRect(rctTitle);
	ScreenToClient(rctTitle);	

	long nSpacer = rctType.top - rctTitle.bottom;

	return nSpacer;

}

void CAddPatDashboardControlDlg::MoveBottom(int idcToShowBelow)
{
	long nSpacer = GetSpacer();

	CRect rctBelow;
	GetDlgItem(idcToShowBelow)->GetWindowRect(rctBelow);
	ScreenToClient(rctBelow);

	//set the ok and cancel boxes
	CRect rctOK, rctCancel;
	GetDlgItem(IDOK)->GetWindowRect(rctOK);
	ScreenToClient(rctOK);	
	
	GetDlgItem(IDCANCEL)->GetWindowRect(rctCancel);
	ScreenToClient(rctCancel);	

	long nHeight = rctOK.Height();

	rctOK.top = rctBelow.bottom + nSpacer;
	rctOK.bottom = rctOK.top + nHeight;

	rctCancel.top = rctBelow.bottom + nSpacer;
	rctCancel.bottom = rctCancel.top + nHeight;

	GetDlgItem(IDOK)->MoveWindow(rctOK);
	GetDlgItem(IDCANCEL)->MoveWindow(rctCancel);

	//move the color control
	CRect rctBG;
	GetDlgItem(IDC_ADD_PAT_DASH_BKG)->GetWindowRect(rctBG);
	ScreenToClient(rctBG);

	rctBG.bottom = rctOK.bottom + nSpacer;
	GetDlgItem(IDC_ADD_PAT_DASH_BKG)->MoveWindow(rctBG);

	//now move the bottom of the window
	CRect rct;
	GetWindowRect(rct);
	ScreenToClient(rct);

	rct.bottom = rctBG.bottom + nSpacer;
	MoveWindow(rct);
	CenterWindow();
	
}

void CAddPatDashboardControlDlg::HideControls()
{
	//first off, let's hide all our filters
	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_EMN_ITEM_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_APPT_TYPE_DESC)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_APPT_TYPE_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_INCLUDE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_INCLUDE)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHK_TIME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CHK_TIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_TIME_INCREMENT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_TIME_INCREMENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_TIME_INTERVAL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_TIME_INTERVAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_TIME_STATIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_TIME_STATIC)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->ShowWindow(SW_HIDE);		
	GetDlgItem(IDC_APD_EMN_INFO_STATIC)->EnableWindow(FALSE);
	// (s.tullis 2015-03-13 10:30) - PLID 64740 
	GetDlgItem(IDC_DASH_NOSHOWCCDA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DASH_NOSHOWCCDA)->EnableWindow(FALSE);
	// (r.gonet 2015-03-17 10:23) - PLID 65020 - Disable and hide the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox.
	GetDlgItem(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT)->EnableWindow(FALSE);
	// (c.haag 2015-04-29) - NX-100441 - History categories label "Include these history categories:"
	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_HISTORY_CAT_DESC)->EnableWindow(FALSE);
	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APD_HISTORY_CAT_LIST)->EnableWindow(FALSE);
}

void CAddPatDashboardControlDlg::Load()
{
	try {

		SetRects();
		HideControls();
		MoveBottom(IDC_APD_TYPE_LIST);

		if (m_pControl) {			

			//now show and position them depending on the type
			ShowControls(m_pControl->pdtType);			

			//set the disabled if necessary
			if (!m_bWriteable) {
				GetDlgItem(IDC_APD_TITLE)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHK_TIME)->EnableWindow(FALSE);
				GetDlgItem(IDC_APD_TIME_INCREMENT)->EnableWindow(FALSE);
				GetDlgItem(IDC_APD_INCLUDE)->EnableWindow(FALSE);
				GetDlgItem(IDOK)->EnableWindow(FALSE);
				
				m_pTypeList->PutReadOnly(TRUE);
				m_pEMNItemList->PutReadOnly(TRUE);
				m_pTimeIntervalList->PutReadOnly(TRUE);
				m_pApptTypeList->PutReadOnly(TRUE);
				m_pHistoryCategoryList->PutReadOnly(TRUE); // (c.haag 2015-04-29) - NX-100441
			}

			//load the information
			SetDlgItemText(IDC_APD_TITLE, m_pControl->strTitle);
			m_pTypeList->SetSelByColumn(tlcID, (long)m_pControl->pdtType);

			LoadFilters();
		
		}
	}NxCatchAll(__FUNCTION__);
}

void CAddPatDashboardControlDlg::LoadFilters() 
{
	for (int i = 0; i < m_pControl->aryFilters.GetSize(); i++) {
		PDFilter *filter = m_pControl->aryFilters[i];
		if (filter->filterType == includeDiscontinued
			|| filter->filterType == includeResolved) {
			if (filter->nValue != 0) {
				CheckDlgButton(IDC_APD_INCLUDE, TRUE);
			}
			else {
				CheckDlgButton(IDC_APD_INCLUDE, FALSE);
			}
		}
		else if (filter->filterType == LastTypeFormat) {
			if (filter->nValue == ltfAll) {
				CheckDlgButton(IDC_CHK_TIME, FALSE);
			}
			else {
				CheckDlgButton(IDC_CHK_TIME, TRUE);
				m_pTimeIntervalList->SetSelByColumn(ticID, filter->nValue);
			}
		}
		else if (filter->filterType == timeIncrement) {
			SetDlgItemInt(IDC_APD_TIME_INCREMENT, filter->nValue);
		}// (s.tullis 2015-02-25 09:53) - PLID 64740 
		else if (filter->filterType == DoNotShowOnCCDA){
			if (filter->nValue == TRUE){
				CheckDlgButton(IDC_DASH_NOSHOWCCDA, BST_CHECKED);
			}
			else
			{
				CheckDlgButton(IDC_DASH_NOSHOWCCDA, BST_UNCHECKED);
			}
		}
		// (r.gonet 2015-03-17 10:23) - PLID 65020 - Load the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox from the in memory filter.
		else if (filter->filterType == excludeDoNotShowOnProblemPrompt) {
			CheckDlgButton(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT, (BOOL)filter->nValue ? BST_CHECKED : BST_UNCHECKED);
		}
		//the others are taken care of in onRequeryfinished messages
	}

	UpdateTimeIntervalLabel();
}
void CAddPatDashboardControlDlg::SetFilters() 
{
	//first, we have to remove all the existing filters so we can reset them
	for (int i = m_pControl->aryFilters.GetSize()-1; i >= 0; i--) {
		PDFilter *filter = m_pControl->aryFilters[i];
		m_pControl->aryFilters.RemoveAt(i);
		if (filter) {
			delete filter;
		}
	}
	m_pControl->aryFilters.RemoveAll();

	switch(m_pControl->pdtType) {
		case pdtAllergies:
		case pdtCurrentMeds:		
			if (IsDlgButtonChecked(IDC_APD_INCLUDE)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = includeDiscontinued;
				filter->nValue = 1;
				m_pControl->aryFilters.Add(filter);
			}		
		break;

		case pdtPrescriptions:
			if (IsDlgButtonChecked(IDC_APD_INCLUDE)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = includeDiscontinued;
				filter->nValue = 1;
				m_pControl->aryFilters.Add(filter);
			}	
			if (IsDlgButtonChecked(IDC_CHK_TIME)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->CurSel;
				filter->nValue = VarLong(pRow->GetValue(ticID));
				m_pControl->aryFilters.Add(filter);

				filter = new PDFilter();
				filter->filterType = timeIncrement;
				filter->nValue = GetDlgItemInt(IDC_APD_TIME_INCREMENT);
				m_pControl->aryFilters.Add(filter);
			}
			else {
				//all
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				filter->nValue = ltfAll;				
				m_pControl->aryFilters.Add(filter);
			}
			
		break;

		case pdtProblems:
			if (IsDlgButtonChecked(IDC_APD_INCLUDE)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = includeResolved;
				filter->nValue = 1;
				m_pControl->aryFilters.Add(filter);
			}
			// (s.tullis 2015-02-25 09:53) - PLID 64740
			if (IsDlgButtonChecked(IDC_DASH_NOSHOWCCDA))
			{
				PDFilter *filter = new PDFilter();
				filter->filterType = DoNotShowOnCCDA;
				filter->nValue = 1;
				m_pControl->aryFilters.Add(filter);
			}
			// (r.gonet 2015-03-17 10:23) - PLID 65020 - If the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox
			// is checked, then create a new filter for it.
			if (IsDlgButtonChecked(IDC_DASH_EXCLUDE_DONOTSHOWONPROBLEMPROMPT) == BST_CHECKED)
			{
				PDFilter *filter = new PDFilter();
				filter->filterType = excludeDoNotShowOnProblemPrompt;
				filter->nValue = (long)TRUE;
				m_pControl->aryFilters.Add(filter);
			}
		break;

		case pdtAppointments:			
			if (IsDlgButtonChecked(IDC_CHK_TIME)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->CurSel;
				filter->nValue = VarLong(pRow->GetValue(ticID));
				m_pControl->aryFilters.Add(filter);

				filter = new PDFilter();
				filter->filterType = timeIncrement;
				filter->nValue = GetDlgItemInt(IDC_APD_TIME_INCREMENT);
				m_pControl->aryFilters.Add(filter);
			}
			else {
				//all
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				filter->nValue = ltfAll;				
				m_pControl->aryFilters.Add(filter);
			}

			//appt type
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptTypeList->GetFirstRow();
				CString strIDs, strNames;
				while (pRow) {
					if (VarBool(pRow->GetValue(atlcChecked))) {
						strIDs += AsString(VarLong(pRow->GetValue(atlcID))) + ", ";
						strNames += VarString(pRow->GetValue(atlcName)) + ", ";
					}
					pRow = pRow->GetNextRow();
				}
				if (!strIDs.IsEmpty()) {
					PDFilter *filter = new PDFilter();
					filter->filterType = ApptType;
					//trim off the last comma
					strIDs = strIDs.Left(strIDs.GetLength() - 2);
					strNames = strNames.Left(strNames.GetLength() - 2);
					filter->strValue = "(" + strIDs + ")";
					filter->strDisplay = strNames;
					m_pControl->aryFilters.Add(filter);
				}

			}

		break;

		case pdtBills:
		case pdtProcedures:
		// (z.manning 2015-03-24 17:28) - NX-100399 - EMN items, Diags, and labs now show time
		case pdtDiagCodes:
		case pdtLabs:
		case pdtEMNItems:
			if (IsDlgButtonChecked(IDC_CHK_TIME)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->CurSel;
				filter->nValue = VarLong(pRow->GetValue(ticID));
				m_pControl->aryFilters.Add(filter);

				filter = new PDFilter();
				filter->filterType = timeIncrement;
				filter->nValue = GetDlgItemInt(IDC_APD_TIME_INCREMENT);
				m_pControl->aryFilters.Add(filter);
			}
			else {
				//all
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				filter->nValue = ltfAll;				
				m_pControl->aryFilters.Add(filter);
			}
		break;

		case pdtHistoryImages:
		case pdtEMNSummary:
		case pdtCreateEMNs:
		//nothing
		break;

		case pdtHistoryAttachments: // (c.haag 2015-04-29) - NX-100441
			if (IsDlgButtonChecked(IDC_CHK_TIME)) {
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->CurSel;
				filter->nValue = VarLong(pRow->GetValue(ticID));
				m_pControl->aryFilters.Add(filter);

				filter = new PDFilter();
				filter->filterType = timeIncrement;
				filter->nValue = GetDlgItemInt(IDC_APD_TIME_INCREMENT);
				m_pControl->aryFilters.Add(filter);
			}
			else {
				//all
				PDFilter *filter = new PDFilter();
				filter->filterType = LastTypeFormat;
				filter->nValue = ltfAll;
				m_pControl->aryFilters.Add(filter);
			}

			// History category
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pHistoryCategoryList->GetFirstRow();
				CString strIDs, strNames;
				while (pRow) {
					if (VarBool(pRow->GetValue(atlcChecked))) {
						strIDs += AsString(VarLong(pRow->GetValue(atlcID))) + ", ";
						strNames += VarString(pRow->GetValue(atlcName)) + ", ";
					}
					pRow = pRow->GetNextRow();
				}
				if (!strIDs.IsEmpty()) {
					PDFilter *filter = new PDFilter();
					filter->filterType = HistoryCategory;
					//trim off the last comma
					strIDs = strIDs.Left(strIDs.GetLength() - 2);
					strNames = strNames.Left(strNames.GetLength() - 2);
					filter->strValue = "(" + strIDs + ")";
					filter->strDisplay = strNames;
					m_pControl->aryFilters.Add(filter);
				}

			}
			break;

		default:
			break;
	}
}

BOOL CAddPatDashboardControlDlg::Validate()
{

	//did they enter a title?
	CString strTitle;
	GetDlgItemText(IDC_APD_TITLE, strTitle);
	strTitle.TrimLeft();
	strTitle.TrimRight();
	if (strTitle == "") {
		MsgBox("Please enter a valid title");
		return FALSE;
	}

	
	//check that a type is selected
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->CurSel;
	if (pRow == NULL)  {
		MsgBox("Please select a type");
		return FALSE;
	}

	//we have a type, the only thing we need to be concerned with is them picking a EMN Item
	PatientDashboardType type = (PatientDashboardType) VarLong(pRow->GetValue(tlcID));
	if (type == pdtEMNItems) {
		//make sure we have an EMN Item
		pRow = m_pEMNItemList->CurSel;
		if (pRow == NULL) {
			MsgBox("Please select an EMN Item");
			return FALSE;
		}
	}

	//if they checked the last time box, make sure they checked the rest
	if (IsDlgButtonChecked(IDC_CHK_TIME)) {
		long nTimeInterval = -99;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->CurSel;
		if (!pRow) {			
			MsgBox("Please choose a time interval");
			return FALSE;
		}
		else {
			nTimeInterval  = VarLong(pRow->GetValue(ticID), -1);
		}
		
		long nTimeIncrement = GetDlgItemInt(IDC_APD_TIME_INCREMENT);
		if (nTimeIncrement <= 0) {
			if (nTimeInterval == lftRecord) {
				MsgBox("Please enter the number of records");
			}
			else {
				MsgBox("Please enter a time increment");
			}
			return FALSE;
		}	

		//check for only going back 100 years
		if (nTimeInterval == 0) { //day
			//don't go over 3650 days
			if (nTimeIncrement > 3650) {
				MsgBox("You may only specifically include up to 100 years in the past, to see more than this, please choose all dates.");
				return FALSE;
			}
		}
		else if (nTimeInterval == 1) { //month
			if (nTimeIncrement > 1200) {
				MsgBox("You may only specifically include up to 100 years in the past, to see more than this, please choose all dates.");
				return FALSE;
			}
		}
		else if (nTimeInterval == 2) { //year
			if (nTimeIncrement > 100) {
				MsgBox("You may only specifically include up to 100 years in the past, to see more than this, please choose all dates.");
				return FALSE;
			}
		}

	


	}

		
	
	return TRUE;
}

void CAddPatDashboardControlDlg::Save()
{		
				
	GetDlgItemText(IDC_APD_TITLE, m_pControl->strTitle);
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeList->CurSel;
	if (pRow) {
		m_pControl->pdtType = (PatientDashboardType)VarLong(pRow->GetValue(ticID));
	}
	if (m_pControl->pdtType == pdtEMNItems) {
		pRow = m_pEMNItemList->CurSel;
		if (pRow) {
			m_pControl->nEMRInfoMasterID = VarLong(pRow->GetValue(eilcID));
		}
	}

	SetFilters();
		

	//now save to data
	CSqlFragment sql(" SET NOCOUNT ON; \r\n "
	" SET XACT_ABORT ON \r\n"
	" DECLARE @ControlID INT; " 
	" BEGIN TRAN \r\n" );

	if (m_pControl->nID < 0) {

		//new record
		sql += CSqlFragment("INSERT INTO PatientDashboardControlsT (Title, TypeID, EMRInfoMasterID) "
			" VALUES({STRING}, {INT}, {VT_I4}); \r\n "
			" SET @ControlID = SCOPE_IDENTITY(); \r\n", 
			m_pControl->strTitle, m_pControl->pdtType, 
			m_pControl->pdtType == pdtEMNItems ? _variant_t(m_pControl->nEMRInfoMasterID) : g_cvarNull);			
		
	}
	else {
		sql += CSqlFragment(" SET @ControlID = {INT}; "
			" UPDATE PatientDashboardControlsT SET Title = {STRING}, "
			" TypeID = {INT}, "
			" EMRInfoMasterID = {VT_I4} "
			" WHERE ID = @ControlID ",
			m_pControl->nID,
			m_pControl->strTitle, m_pControl->pdtType,
			m_pControl->pdtType == pdtEMNItems ? _variant_t(m_pControl->nEMRInfoMasterID) : g_cvarNull
			);	

		//remove all existing filters
		sql += CSqlFragment(" DELETE FROM PatientDashboardControlFiltersT WHERE ControlID = @ControlID; \r\n\r\n");
	}

	if (m_pControl->aryFilters.GetSize() >= 1) {		

		for (int i = 0; i < m_pControl->aryFilters.GetSize(); i++) {
			PDFilter *pFilter = m_pControl->aryFilters[i];
			_variant_t varInt = (long)pFilter->nValue;
			_variant_t varText = _variant_t(pFilter->strValue);

			sql += CSqlFragment(" INSERT INTO PatientDashboardControlFiltersT (ControlID, FilterTypeID, FilterValueInt, FilterValueText) "
				" VALUES (@ControlID, {INT}, {VT_I4}, {VT_BSTR}); \r\n",
				pFilter->filterType,
				// (c.haag 2015-04-29) - NX-100441 - This applies to both appointment types and history categories
				(pFilter->filterType == ApptType || pFilter->filterType == HistoryCategory) ? g_cvarNull : varInt,
				(pFilter->filterType == ApptType || pFilter->filterType == HistoryCategory) ? varText : g_cvarNull);
		}
	}
				
	//now get the controlID back out
	sql += CSqlFragment( "COMMIT TRAN; \r\n "
		" SET NOCOUNT OFF; \r\n "
		" SELECT @ControlID as ControlID; ");

	ADODB::_RecordsetPtr rsNew = CreateParamRecordset(sql);
	if (!rsNew->eof) {
		m_pControl->nID = AdoFldLong(rsNew->Fields, "ControlID");
	}
	
}

void CAddPatDashboardControlDlg::OnRequeryFinishedApptTypeList(short nFlags)
{
	try {

		if ((m_pControl) && m_pControl->aryFilters.GetSize() > 0) {

			for(int i = 0; i < m_pControl->aryFilters.GetSize(); i++ ){
				if (m_pControl->aryFilters[i]->filterType == ApptType) {

					CString strFilter = m_pControl->aryFilters[i]->strValue;
					strFilter.TrimLeft("(");
					strFilter.TrimRight(")");
					long nResult = strFilter.Find(",");
					while (nResult != -1) {
						CString strTypeID = strFilter.Left(nResult);
						long nTypeID = atoi(strTypeID);

						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptTypeList->FindByColumn(atlcID, nTypeID,0, FALSE);
						if (pRow) {
							pRow->PutValue(atlcChecked, g_cvarTrue);
						}

						strFilter = strFilter.Right(strFilter.GetLength() - (nResult + 1));
						nResult = strFilter.Find(",");
					}

					//do the last one
					CString strTypeID = strFilter;
					long nTypeID = atoi(strTypeID);

					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptTypeList->FindByColumn(atlcID, nTypeID, 0, FALSE);
					if (pRow) {
						pRow->PutValue(atlcChecked, g_cvarTrue);
					}

					strFilter = strFilter.Right(strFilter.GetLength() - (nResult + 1));
					nResult = strFilter.Find(",");

				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2015-04-29) - NX-100441 - Handler for when the history category list has finished
void CAddPatDashboardControlDlg::OnRequeryFinishedHistoryCatList(short nFlags)
{
	try {

		if ((m_pControl) && m_pControl->aryFilters.GetSize() > 0) {

			for (int i = 0; i < m_pControl->aryFilters.GetSize(); i++){
				if (m_pControl->aryFilters[i]->filterType == HistoryCategory) {

					CString strFilter = m_pControl->aryFilters[i]->strValue;
					strFilter.TrimLeft("(");
					strFilter.TrimRight(")");
					long nResult = strFilter.Find(",");
					while (nResult != -1) {
						CString strCatID = strFilter.Left(nResult);
						long nCatID = atoi(strCatID);

						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pHistoryCategoryList->FindByColumn(atlcID, nCatID, 0, FALSE);
						if (pRow) {
							pRow->PutValue(atlcChecked, g_cvarTrue);
						}

						strFilter = strFilter.Right(strFilter.GetLength() - (nResult + 1));
						nResult = strFilter.Find(",");
					}

					//do the last one
					CString strCatID = strFilter;
					long nCatID = atoi(strCatID);

					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pHistoryCategoryList->FindByColumn(atlcID, nCatID, 0, FALSE);
					if (pRow) {
						pRow->PutValue(atlcChecked, g_cvarTrue);
					}

					strFilter = strFilter.Right(strFilter.GetLength() - (nResult + 1));
					nResult = strFilter.Find(",");

				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CAddPatDashboardControlDlg::RequeryFinishedApdEmnItemList(short nFlags)
{
	try {

		if (m_pControl && m_pControl->pdtType == pdtEMNItems) {			
			m_pEMNItemList->SetSelByColumn(eilcID, m_pControl->nEMRInfoMasterID);
		}	
		
	}NxCatchAll(__FUNCTION__);
}

void CAddPatDashboardControlDlg::SelChosenApdTypeList(LPDISPATCH lpRow)
{	
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow){
			ShowWindow(SW_HIDE);
			SetActiveWindow();
			PatientDashboardType type = (PatientDashboardType) VarLong(pRow->GetValue(tlcID));
			HideControls();
			ShowControls(type);
			ShowWindow(SW_SHOW);			
			GetDlgItem(IDC_APD_TYPE_LIST)->SetFocus();

			// (z.manning 2015-05-05 09:51) - NX-100447 - If they selected EMN item history and there
			// is not currently an item selected, then drop down the list for them if this is a new control.
			if (type == pdtEMNItems && m_pEMNItemList->CurSel == NULL && m_pControl != NULL && m_pControl->nID < 0)
			{
				m_pEMNItemList->DropDownState = VARIANT_TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-04-02 12:00) - NX-100388
void CAddPatDashboardControlDlg::SelChosenApdTimeInterval(LPDISPATCH lpRow)
{
	try
	{
		UpdateTimeIntervalLabel();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-04-15 10:48) - NX-100388
void CAddPatDashboardControlDlg::UpdateTimeIntervalLabel()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTimeIntervalList->GetCurSel();
	CString strText = "(s)";
	if (pRow == NULL || VarLong(pRow->GetValue(ticID)) != lftRecord) {
		strText += " of records";
	}

	SetDlgItemText(IDC_APD_TIME_STATIC, strText);
}

// (z.manning 2015-05-04 16:54) - NX-100447
void CAddPatDashboardControlDlg::SelChosenApdEmnItemList(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		CString strTitle;
		GetDlgItemText(IDC_APD_TITLE, strTitle);
		// (z.manning 2015-05-04 17:09) - NX-100447 - If this is a new control and the user did not enter a title
		// already then set the title to the name of the item.
		// Note: We check for an ID of less than zero instead of -1 because if you add more than one control
		// at the same time then it will keep decrementing the ID value.
		if (strTitle.IsEmpty() && m_pControl != NULL && m_pControl->nID < 0)
		{
			SetDlgItemText(IDC_APD_TITLE, VarString(pRow->GetValue(eilcName), ""));
		}
	}
	NxCatchAll(__FUNCTION__);
}
