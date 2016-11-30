// AttendanceOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ContactsRc.h"
#include "AttendanceOptionsDlg.h"
#include "AttendanceUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace NXDATALIST2Lib;

enum ETodoCategoryColumns
{
	tccID = 0,
	tccDescription,
};

// (z.manning, 02/15/2008) - PLID 28909 - Shared by all 3 type datalists.
enum EAttendaneTypeColumns
{
	atcID = 0,
	atcName,
};

/////////////////////////////////////////////////////////////////////////////
// CAttendanceOptionsDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29139 - Created

CAttendanceOptionsDlg::CAttendanceOptionsDlg(CWnd* pParent)
	: CNxDialog(CAttendanceOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttendanceOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAttendanceOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttendanceOptionsDlg)
	DDX_Control(pDX, IDC_SHOW_ATTENDANCE_GRID, m_btnShowGridlines);
	DDX_Control(pDX, IDC_EMAIL_ON_DENY, m_btnEmailOnDeny);
	DDX_Control(pDX, IDC_EMAIL_ON_APPROVE, m_btnEmailOnApprove);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_DEFAULT_VACATION_ALLOWANCE, m_nxeditDefaultVacationAllowance);
	DDX_Control(pDX, IDC_DEFAULT_SICK_ALLOWANCE, m_nxeditDefaultSickAllowance);
	DDX_Control(pDX, IDC_ATTENDANCE_OPTION_COLOR_CHOOSER_CTRL, m_ctrlColorPicker);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAttendanceOptionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAttendanceOptionsDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ACCRUED_ROW_COLOR, &CAttendanceOptionsDlg::OnBnClickedAccruedRowColor)
	ON_BN_CLICKED(IDC_AVAILABLE_ROW_COLOR, &CAttendanceOptionsDlg::OnBnClickedAvailableRowColor)
	ON_BN_CLICKED(IDC_TOTAL_USED_ROW_COLOR, &CAttendanceOptionsDlg::OnBnClickedTotalUsedRowColor)
	ON_BN_CLICKED(IDC_BALANCE_ROW_COLOR, &CAttendanceOptionsDlg::OnBnClickedBalanceRowColor)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CAttendanceOptionsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAttendanceOptionsDlg)
	ON_EVENT(CAttendanceOptionsDlg, IDC_ATTENDANCE_TODO_CATEGORY, 18 /* RequeryFinished */, OnRequeryFinishedAttendanceTodoCategory, VTS_I2)
	ON_EVENT(CAttendanceOptionsDlg, IDC_ATTENDANCE_VACATION_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedAttendanceVacationType, VTS_I2)
	ON_EVENT(CAttendanceOptionsDlg, IDC_ATTENDANCE_SICK_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedAttendanceSickType, VTS_I2)
	ON_EVENT(CAttendanceOptionsDlg, IDC_ATTENDANCE_OTHER_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedAttendanceOtherType, VTS_I2)
	ON_EVENT(CAttendanceOptionsDlg, IDC_ATTENDANCE_PAID_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedAttendancePaidType, VTS_I2)
	ON_EVENT(CAttendanceOptionsDlg, IDC_ATTENDANCE_UNPAID_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedAttendanceUnpaidType, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAttendanceOptionsDlg message handlers

BOOL CAttendanceOptionsDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CAttendanceOptionsDlg", propNumber,
			"(Username = '%s' OR Username = '<None>') AND Name IN ( \r\n"
			"	'DefaultVacationAllowance' \r\n"
			"	, 'DefaultSickAllowance' \r\n"
			"	, 'AttendanceListShowGrid' \r\n"
			"	, 'EmailOnTimeOffApproved' \r\n"
			"	, 'EmailOnTimeOffDenied' \r\n"
			"	, 'AttendanceToDoCategory' \r\n"
			"	, 'AttendanceVacationTypeID' \r\n"
			"	, 'AttendanceSickTypeID' \r\n"
			"	, 'AttendanceOtherTypeID' \r\n"
			"	, 'AttendancePaidTypeID' \r\n"
			"	, 'AttendanceUnpaidTypeID' \r\n"
			// (z.manning 2012-03-27 11:42) - PLID 49227 - Added row color prefs
			"	, 'AttendanceAccruedRowColor' \r\n"
			"	, 'AttendanceAvailableRowColor' \r\n"
			"	, 'AttendanceTotalUsedRowColor' \r\n"
			"	, 'AttendanceBalanceRowColor' \r\n"
			")",
			_Q(GetCurrentUserName()));

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (z.manning, 01/11/2008) - PLID 28600 - Added to-do category option
		m_pdlTodoCategory = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_TODO_CATEGORY, GetRemoteData(), true);
		m_pdlVacationType = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_VACATION_TYPE, GetRemoteData(), true);
		m_pdlSickType = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_SICK_TYPE, GetRemoteData(), true);
		m_pdlOtherType = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_OTHER_TYPE, GetRemoteData(), true);
		// (z.manning 2008-11-13 13:43) - PLID 31831 - Paid/unpaid
		m_pdlPaidType = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_PAID_TYPE, GetRemoteData(), true);
		m_pdlUnpaidType = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_UNPAID_TYPE, GetRemoteData(), true);

		SetDlgItemInt(IDC_DEFAULT_VACATION_ALLOWANCE, GetRemotePropertyInt("DefaultVacationAllowance", 0, 0, "<None>", true));
		SetDlgItemInt(IDC_DEFAULT_SICK_ALLOWANCE, GetRemotePropertyInt("DefaultSickAllowance", 0, 0, "<None>", true));

		if(GetRemotePropertyInt("AttendanceListShowGrid", 0, 0, GetCurrentUserName(), true) != 0) {
			CheckDlgButton(IDC_SHOW_ATTENDANCE_GRID, BST_CHECKED);
		}

		// (z.manning, 01/02/2008) - PLID 28461 - Options for auto emailing.
		CheckDlgButton(IDC_EMAIL_ON_APPROVE, GetRemotePropertyInt("EmailOnTimeOffApproved", 1, 0, GetCurrentUserName(), true) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_EMAIL_ON_DENY, GetRemotePropertyInt("EmailOnTimeOffDenied", 1, 0, GetCurrentUserName(), true) == 1 ? BST_CHECKED : BST_UNCHECKED);

		// (z.manning 2012-03-27 11:45) - PLID 49227 - Row color options
		m_nAccruedRowColor = GetAccruedRowColor();
		m_nAvailableRowColor = GetAvailableRowColor();
		m_nTotalUsedRowColor = GetTotalUsedRowColor();
		m_nBalanceRowColor = GetBalanceRowColor();

		// (z.manning, 12/11/2007) - Only admins may change the default allowances.
		if(!CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE))
		{
			GetDlgItem(IDC_DEFAULT_VACATION_ALLOWANCE)->EnableWindow(FALSE);
			GetDlgItem(IDC_DEFAULT_SICK_ALLOWANCE)->EnableWindow(FALSE);

			m_pdlTodoCategory->PutReadOnly(VARIANT_TRUE);

			m_pdlVacationType->PutReadOnly(VARIANT_TRUE);
			m_pdlSickType->PutReadOnly(VARIANT_TRUE);
			// (z.manning 2008-11-13 13:44) - PLID 31831 - Paid/Unpaid
			m_pdlPaidType->PutReadOnly(VARIANT_TRUE);
			m_pdlUnpaidType->PutReadOnly(VARIANT_TRUE);
			m_pdlOtherType->PutReadOnly(VARIANT_TRUE);
		}

	}NxCatchAll("CAttendanceOptionsDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttendanceOptionsDlg::Save()
{
	SetRemotePropertyInt("DefaultVacationAllowance", GetDlgItemInt(IDC_DEFAULT_VACATION_ALLOWANCE), 0, "<None>");
	SetRemotePropertyInt("DefaultSickAllowance", GetDlgItemInt(IDC_DEFAULT_SICK_ALLOWANCE), 0, "<None>");

	// (z.manning, 01/02/2008) - PLID 28461 - Options for auto-emailing.
	SetRemotePropertyInt("EmailOnTimeOffApproved", IsDlgButtonChecked(IDC_EMAIL_ON_APPROVE) == BST_CHECKED ? 1 : 0, 0, GetCurrentUserName());
	SetRemotePropertyInt("EmailOnTimeOffDenied", IsDlgButtonChecked(IDC_EMAIL_ON_DENY) == BST_CHECKED ? 1 : 0, 0, GetCurrentUserName());
	
	long nShowGrid;
	if(IsDlgButtonChecked(IDC_SHOW_ATTENDANCE_GRID) == BST_CHECKED) {
		nShowGrid = 1;
	}
	else {
		nShowGrid = 0;
	}
	SetRemotePropertyInt("AttendanceListShowGrid", nShowGrid, 0, GetCurrentUserName());

	long nTodoCategory = VarLong(m_pdlTodoCategory->GetCurSel()->GetValue(tccID));
	SetRemotePropertyInt("AttendanceToDoCategory", nTodoCategory, 0, "<None>");

	// (z.manning, 02/15/2008) - PLID 28909 - Save the type options.
	SetRemotePropertyInt("AttendanceVacationTypeID", VarLong(m_pdlVacationType->GetCurSel()->GetValue(atcID),-1), 0, "<None>");
	SetRemotePropertyInt("AttendanceSickTypeID", VarLong(m_pdlSickType->GetCurSel()->GetValue(atcID),-1), 0, "<None>");
	SetRemotePropertyInt("AttendanceOtherTypeID", VarLong(m_pdlOtherType->GetCurSel()->GetValue(atcID),-1), 0, "<None>");
	// (z.manning 2008-11-13 13:45) - PLID 31831 - Paid/Unpaid
	SetRemotePropertyInt("AttendancePaidTypeID", VarLong(m_pdlPaidType->GetCurSel()->GetValue(atcID),-1), 0, "<None>");
	SetRemotePropertyInt("AttendanceUnpaidTypeID", VarLong(m_pdlUnpaidType->GetCurSel()->GetValue(atcID),-1), 0, "<None>");

	// (z.manning 2012-03-27 12:12) - PLID 49227 - Row colors
	SetRemotePropertyInt("AttendanceAccruedRowColor", m_nAccruedRowColor, 0, GetCurrentUserName());
	SetRemotePropertyInt("AttendanceAvailableRowColor", m_nAvailableRowColor, 0, GetCurrentUserName());
	SetRemotePropertyInt("AttendanceTotalUsedRowColor", m_nTotalUsedRowColor, 0, GetCurrentUserName());
	SetRemotePropertyInt("AttendanceBalanceRowColor", m_nBalanceRowColor, 0, GetCurrentUserName());
}

void CAttendanceOptionsDlg::OnOK() 
{
	try
	{
		Save();

		CDialog::OnOK();

	}NxCatchAll("CAttendanceOptionsDlg::OnOK");
}
void CAttendanceOptionsDlg::OnRequeryFinishedAttendanceTodoCategory(short nFlags) 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlTodoCategory->GetNewRow();
		pRow->PutValue(tccID, (long)-1);
		pRow->PutValue(tccDescription, "{ No Category }");
		m_pdlTodoCategory->AddRowBefore(pRow, m_pdlTodoCategory->GetFirstRow());

		// (z.manning, 01/11/2008) - PLID 28600 - Set the selection here based on the configrt value.
		if(m_pdlTodoCategory->SetSelByColumn(tccID, GetRemotePropertyInt("AttendanceToDoCategory", -1, 0, "<None>", true)) == NULL) {
			// (z.manning, 01/11/2008) - If we couldn't find the category this was set, so it must have been deleted.
			// Reset the value to no category.
			m_pdlTodoCategory->SetSelByColumn(tccID, (long)-1);
			SetRemotePropertyInt("AttendanceToDoCategory", -1, 0, "<None>");
		}

	}NxCatchAll("CAttendanceOptionsDlg::OnRequeryFinishedAttendanceTodoCategory");
}

// (z.manning, 02/15/2008) - PLID 28909 - Wrote this macro to load the type options since it's the same
// thing just on 3 different datalists.
#define HANDLE_TYPE_REQUERY_FINISHED(attendanceType) \
	IRowSettingsPtr pRow = m_pdl##attendanceType##Type->GetNewRow(); \
	pRow->PutValue(atcID, (long)-1); \
	pRow->PutValue(atcName, "{ No Type }"); \
	m_pdl##attendanceType##Type->AddRowBefore(pRow, m_pdl##attendanceType##Type->GetFirstRow()); \
	long nTypeID = GetRemotePropertyInt(FormatString("Attendance%sTypeID",#attendanceType), -1, 0, "<None>", true); \
	if(NULL == m_pdl##attendanceType##Type->SetSelByColumn(atcID, nTypeID)) { \
		m_pdl##attendanceType##Type->SetSelByColumn(atcID, (long)-1); \
	}

void CAttendanceOptionsDlg::OnRequeryFinishedAttendanceVacationType(short nFlags) 
{
	try
	{
		HANDLE_TYPE_REQUERY_FINISHED(Vacation);

	}NxCatchAll("CAttendanceOptionsDlg::OnRequeryFinishedAttendanceVacationType");
}

void CAttendanceOptionsDlg::OnRequeryFinishedAttendanceSickType(short nFlags) 
{
	try
	{
		HANDLE_TYPE_REQUERY_FINISHED(Sick);

	}NxCatchAll("CAttendanceOptionsDlg::OnRequeryFinishedAttendanceSickType");
}

void CAttendanceOptionsDlg::OnRequeryFinishedAttendanceOtherType(short nFlags) 
{
	try
	{
		HANDLE_TYPE_REQUERY_FINISHED(Other);

	}NxCatchAll("CAttendanceOptionsDlg::OnRequeryFinishedAttendanceOtherType");
}

// (z.manning 2008-11-13 13:46) - PLID 31831
void CAttendanceOptionsDlg::OnRequeryFinishedAttendancePaidType(short nFlags)
{
	try
	{
		HANDLE_TYPE_REQUERY_FINISHED(Paid);

	}NxCatchAll("CAttendanceOptionsDlg::OnRequeryFinishedAttendancePaidType");
}

// (z.manning 2008-11-13 13:46) - PLID 31831
void CAttendanceOptionsDlg::OnRequeryFinishedAttendanceUnpaidType(short nFlags)
{
	try
	{
		HANDLE_TYPE_REQUERY_FINISHED(Unpaid);

	}NxCatchAll("CAttendanceOptionsDlg::OnRequeryFinishedAttendanceUnpaidType");
}

// (z.manning 2012-03-27 12:07) - PLID 49227
void CAttendanceOptionsDlg::SelectRowColorOption(UINT nCtrlIDButton, IN OUT OLE_COLOR &nColor)
{
	m_ctrlColorPicker.SetColor(nColor);
	m_ctrlColorPicker.ShowColor();
	nColor = m_ctrlColorPicker.GetColor();
	GetDlgItem(nCtrlIDButton)->RedrawWindow();
}

void CAttendanceOptionsDlg::OnBnClickedAccruedRowColor()
{
	try
	{
		SelectRowColorOption(IDC_ACCRUED_ROW_COLOR, m_nAccruedRowColor);
	}
	NxCatchAll(__FUNCTION__);
}

void CAttendanceOptionsDlg::OnBnClickedAvailableRowColor()
{
	try
	{
		SelectRowColorOption(IDC_AVAILABLE_ROW_COLOR, m_nAvailableRowColor);
	}
	NxCatchAll(__FUNCTION__);
}

void CAttendanceOptionsDlg::OnBnClickedTotalUsedRowColor()
{
	try
	{
		SelectRowColorOption(IDC_TOTAL_USED_ROW_COLOR, m_nTotalUsedRowColor);
	}
	NxCatchAll(__FUNCTION__);
}

void CAttendanceOptionsDlg::OnBnClickedBalanceRowColor()
{
	try
	{
		SelectRowColorOption(IDC_BALANCE_ROW_COLOR, m_nBalanceRowColor);
	}
	NxCatchAll(__FUNCTION__);
}

void CAttendanceOptionsDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// (z.manning 2012-03-27 11:55) - PLID 49227 - Special drawing for the row color option buttons
	switch(nIDCtl)
	{
		case IDC_ACCRUED_ROW_COLOR:
			DrawColorOnButton(lpDrawItemStruct, m_nAccruedRowColor);
			break;
		case IDC_AVAILABLE_ROW_COLOR:
			DrawColorOnButton(lpDrawItemStruct, m_nAvailableRowColor);
			break;
		case IDC_TOTAL_USED_ROW_COLOR:
			DrawColorOnButton(lpDrawItemStruct, m_nTotalUsedRowColor);
			break;
		case IDC_BALANCE_ROW_COLOR:
			DrawColorOnButton(lpDrawItemStruct, m_nBalanceRowColor);
			break;
		default:
			CNxDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
			break;
	}
}