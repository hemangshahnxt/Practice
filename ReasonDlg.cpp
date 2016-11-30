// CancelApptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReasonDlg.h"
#include "NxStandard.h"
#include "EditComboBox.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "CommonSchedUtils.h"
#include "GlobalSchedUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CReasonDlg dialog

using namespace ADODB;

CReasonDlg::CReasonDlg(CWnd* pParent)
	: CNxDialog(CReasonDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReasonDlg)
	m_strText = _T("");
	m_strReason = _T("");
	//}}AFX_DATA_INIT

	//DRT 4/12/2004 - Sometimes this dialog is not opened (cut/paste an appt, say yes to cancel).  In
	//	that case, this needs to be set!
	m_nIsChecked = 1;

	m_bNoShow = false;
}


void CReasonDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReasonDlg)
	DDX_Control(pDX, IDC_CUSTOM_REASON, m_customReason);
	DDX_Text(pDX, IDC_REASON_TEXT, m_strText);
	DDX_Text(pDX, IDC_CUSTOM_REASON_EDIT, m_strReason);
	DDX_Control(pDX, IDC_CUSTOM_REASON_EDIT, m_nxeditCustomReasonEdit);
	DDX_Control(pDX, IDC_REASON_PATIENT, m_nxeditReasonPatient);
	DDX_Control(pDX, IDC_REASON_START_TIME, m_nxeditReasonStartTime);
	DDX_Control(pDX, IDC_REASON_END_TIME, m_nxeditReasonEndTime);
	DDX_Control(pDX, IDC_REASON_DATE, m_nxeditReasonDate);
	DDX_Control(pDX, IDC_REASON_TEXT, m_nxeditReasonText);
	DDX_Control(pDX, IDC_REASON_TYPE, m_nxeditReasonType);
	DDX_Control(pDX, IDC_REASON_PURPOSE, m_nxeditReasonPurpose);
	DDX_Control(pDX, IDC_REASON_CAPTION, m_nxstaticReasonCaption);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_REASON_COMBO, m_btnReasonCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReasonDlg, CNxDialog)
	//{{AFX_MSG_MAP(CReasonDlg)
	ON_BN_CLICKED(IDC_CUSTOM_REASON, OnCustomReason)
	ON_BN_CLICKED(IDC_EDIT_REASON_COMBO, OnEditReasonCombo)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReasonDlg message handlers

BOOL CReasonDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// (d.singleton 2011-10-12 12:10) - PLID cashe properties
	g_propManager.CachePropertiesInBulk("CReasonDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
		" 'ApptCancellationReason' \r\n" // (d.singleton 2011-10-12 12:12) - PLID 25956 add pref for canacellation reason
		")"
		, _Q(GetCurrentUserName()));

	// (d.singleton 2011-12-28 10:34) - PLID 47110 store prop value in mem variable
	m_bReqReason = GetRemotePropertyInt("ApptCancellationReason", 0, 0, "<None>", true);
	
	// (c.haag 2008-04-24 15:17) - PLID 29776 - NxIconized the buttons
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_ReasonCombo = BindNxDataListCtrl(IDC_REASON_COMBO,false);

	// (a.walling 2014-12-22 10:05) - PLID 64366
	m_bReschedule = false;

	// (c.haag 2008-06-04 10:37) - PLID 29882 - Create the back brush

	if(m_bNoShow) {
		m_ReasonCombo->FromClause = _bstr_t("AptNoShowReasonT");
		SetDlgItemText(IDC_REASON_CAPTION, "Reason for marking as No Show");
		SetWindowText("No Show Appointment");
	}
	else {
		m_ReasonCombo->FromClause = _bstr_t("AptCancelReasonT");
		SetDlgItemText(IDC_REASON_CAPTION, "Reason for cancelling appointment");
		SetWindowText("Cancel Appointment");
	}
	m_ReasonCombo->Requery();
	m_ReasonCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	IRowSettingsPtr pRow;
	pRow = m_ReasonCombo->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,"<No Item Selected>");
	m_ReasonCombo->InsertRow(pRow, 0);

	//Set the text on screen.
	_RecordsetPtr rsAppointment = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
		"AptTypeT.Name AS Type, dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime "
		"FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID LEFT JOIN PersonT ON "
		"AppointmentsT.PatientID = PersonT.ID WHERE AppointmentsT.ID = %li", m_nApptID);

	if(rsAppointment->eof) {
		//Yikes!
		ASSERT(FALSE);
		CNxDialog::OnCancel();
	}

	SetDlgItemText(IDC_REASON_PATIENT, AdoFldString(rsAppointment, "Name", ""));
	SetDlgItemText(IDC_REASON_TYPE, AdoFldString(rsAppointment, "Type", "<None>"));
	SetDlgItemText(IDC_REASON_PURPOSE, AdoFldString(rsAppointment, "Purpose", "<None>"));
	SetDlgItemText(IDC_REASON_DATE, FormatDateTimeForInterface(AdoFldDateTime(rsAppointment, "Date"), 0, dtoDate, true));
	SetDlgItemText(IDC_REASON_START_TIME, FormatDateTimeForInterface(AdoFldDateTime(rsAppointment, "StartTime"), DTF_STRIP_SECONDS, dtoTime, true));
	SetDlgItemText(IDC_REASON_END_TIME, FormatDateTimeForInterface(AdoFldDateTime(rsAppointment, "EndTime"), DTF_STRIP_SECONDS, dtoTime, true));

	m_nIsChecked = m_customReason.GetCheck();

	// (a.walling 2014-12-22 10:05) - PLID 64366 - reschedule checkbox should be red
	SafeGetDlgItem<NxButton>(IDC_CHECK_RESCHEDULE)->SetTextColor(HEXRGB(0x800000));

	// (a.walling 2015-02-04 09:12) - PLID 64412 - hide checkbox to add to rescheduling queue
	if (m_bDisableReschedulingQueue) {
		auto pCheck = GetDlgItem(IDC_CHECK_RESCHEDULE);
		pCheck->ShowWindow(SW_HIDE);
		pCheck->EnableWindow(FALSE);
	}

	return FALSE;
}


void CReasonDlg::OnCustomReason() 
{
	// (d.singleton 2011-09-28 16:40) - PLID 39952 - add permission to add regular and custom cancellation reasons
	if(CheckCurrentUserPermissions(bioCancellationReasons, sptWrite, FALSE, 0, FALSE, TRUE))
	{
		// (a.walling 2014-12-22 10:05) - PLID 64366 - Move the reschedule checkbox when toggling custom reason edit box
		CRect rcEdit;
		GetDlgItem(IDC_CUSTOM_REASON_EDIT)->GetWindowRect(&rcEdit);
		ScreenToClient(&rcEdit);
		
		CRect rcCheck;
		auto* pCheckWnd = GetDlgItem(IDC_CHECK_RESCHEDULE);
		pCheckWnd->GetWindowRect(&rcCheck);
		ScreenToClient(&rcCheck); 
		auto checkHeight = rcCheck.Height();

		if (m_customReason.GetCheck())	{
			rcCheck.top = rcEdit.bottom + 1;
			rcCheck.bottom = rcCheck.top + checkHeight;
		}
		else {			
			rcCheck.bottom = rcEdit.bottom - 1;
			rcCheck.top = rcCheck.bottom - checkHeight;
		}

		pCheckWnd->MoveWindow(rcCheck);

		//Check Show Window functions
		if (m_customReason.GetCheck())	{
			GetDlgItem(IDC_REASON_COMBO)->ShowWindow(false);
			GetDlgItem(IDC_EDIT_REASON_COMBO)->ShowWindow(false);
			GetDlgItem(IDC_CUSTOM_REASON_EDIT)->ShowWindow(true);
		}
		else {
			GetDlgItem(IDC_REASON_COMBO)->ShowWindow(true);
			GetDlgItem(IDC_EDIT_REASON_COMBO)->ShowWindow(true);
			GetDlgItem(IDC_CUSTOM_REASON_EDIT)->ShowWindow(false);
		}

		m_nIsChecked = m_customReason.GetCheck();
	}
	else
	{
		m_customReason.SetCheck(0);
	}
}

void CReasonDlg::EditCustomList(_DNxDataListPtr &list, long listID) 
{
	//I copied this function from custom1dlg.cpp and then made some changes to it

	//BVB - this crashed without a selection, 
	//also duplicating this code 6 times is not acceptable

	_bstr_t			value;
	long			curSel;

	
	//save the current value
	curSel = list->CurSel;
	if (curSel != -1)
		value = list->Value[curSel][1];

	// CH 3/1: Even though it doesnt LOOK like you added
	// it to the list with this conditional, it still doesnt work!!
	// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
	if (IDOK == CEditComboBox(this, listID, list, "Edit Combo Box").DoModal())
	{
		IRowSettingsPtr pRow;
		pRow = list->GetRow(-1);
		_variant_t var;
		var.vt = VT_NULL;
		// (d.singleton 2011-11-08 17:57) - PLID 25956 - had to change the below line to assign value of -1, this is the value assigned OnInit and im checking for this value OnOK
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<No Item Selected>");
		list->InsertRow(pRow,0);

		//try and set the combo to the old value
		if (curSel != -1)
			list->SetSelByColumn(1, value);
	}
	else {
		//DRT 7/25/02
		//if we cancel the dialog, it requeries the list (because changes are made whether you hit ok or cancel)
		//so the list will have no selection.
		list->SetSelByColumn(1, value);

	}
}

void CReasonDlg::OnEditReasonCombo() 
{
	// (d.singleton 2011-09-28 16:40) - PLID 39952 - add permission to add regular and custom cancellation reasons
	if(CheckCurrentUserPermissions(bioCancellationReasons, sptWrite, FALSE, 0, FALSE, TRUE)){
		if(m_bNoShow) {
			EditCustomList(m_ReasonCombo, 13); //NO_SHOW_REASON_COMBO
		}
		else {
			EditCustomList(m_ReasonCombo, 12); //CANCEL_REASON_COMBO
		}
	}
}

void CReasonDlg::OnOK() 
{	
	try {
		// (d.singleton 2011-12-27 08:57) - PLID 47110 - changed to check mem variable for preference state since i now fill that in OnInit
		if (m_bReqReason)
		{
			// (d.singleton 2011-11-14 17:42) - PLID 25956 added this check to make sure im not calling GetRow() on a row that doesnt exist and returns -1,  if its a custom reason this check doesnt matter
			if(m_ReasonCombo->GetCurSel() != -1 || IsCustomReason())
			{
				IRowSettingsPtr pReasonRow = m_ReasonCombo->GetRow(m_ReasonCombo->GetCurSel());

				// (d.singleton 2011-10-12 12:12) - PLID 25956 add pref to require canacellation reason
				if(IsCustomReason()) 
				{
					CString strDlgText;
					GetDlgItemText(IDC_CUSTOM_REASON_EDIT, strDlgText);
					if(strDlgText.Trim().CompareNoCase("") == 0) 
					{
						AfxMessageBox("Please enter a reason before saving");
						return;
					}
				}	
				// (d.singleton 2011-11-08 17:56) - PLID 25956 - check for empty or null rows
				else if(pReasonRow->GetValue(0).vt != VT_EMPTY && pReasonRow->GetValue(0).vt != VT_NULL) 
				{
					// (d.singleton 2011-11-08 17:58) - PLID check for value of <no item selected>
					if(VarLong(pReasonRow->GetValue(0)) == -1)
					{
						AfxMessageBox("Please enter a reason before saving");
						return;
					}
				}
				else
				{
					AfxMessageBox("Please enter a reason before saving");
					return;
				}
			}
			else
			{
				AfxMessageBox("Please enter a reason before saving");
				return;	
			}
		}

		
		// (a.walling 2014-12-22 10:05) - PLID 64366 - update m_bReschedule flag
		// (a.walling 2015-02-04 09:12) - PLID 64412 - hide checkbox to add to rescheduling queue
		m_bReschedule = !m_bDisableReschedulingQueue && !!IsDlgButtonChecked(IDC_CHECK_RESCHEDULE);
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
	
}

void CReasonDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CReasonDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CReasonDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

CString CReasonDlg::GetReason(long &ReasonID)
{
	if (m_ReasonCombo->CurSel == -1)
	{
		ReasonID = -1;
		return "";
	}
	//Gets reason ID
	ReasonID = VarLong(m_ReasonCombo->GetValue(m_ReasonCombo->CurSel, 0));
	//Returns reason
	return VarString(m_ReasonCombo->GetValue(m_ReasonCombo->CurSel, 1));
}

Nx::Scheduler::Reason CReasonDlg::GetReason()
{
	if (IsCustomReason()) {
		return{ -1, m_strReason };
	}
	else if (m_ReasonCombo->CurSel != -1) {
		return{ VarLong(m_ReasonCombo->GetValue(m_ReasonCombo->CurSel, 0)), VarString(m_ReasonCombo->GetValue(m_ReasonCombo->CurSel, 1)) };
	}
	else {
		return{};
	}
}

int CReasonDlg::IsCustomReason()
{
	return m_nIsChecked;
}

HBRUSH CReasonDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (c.haag 2008-05-09 15:15) - PLID 29882 - This code will force
	// the read-only edit boxes to share the solid background color
	// instead of an ugly gray.
	switch (pWnd->GetDlgCtrlID()) {
	case IDC_REASON_TEXT:
	case IDC_REASON_PATIENT:
	case IDC_REASON_DATE:
	case IDC_REASON_TYPE:
	case IDC_REASON_START_TIME:
	case IDC_REASON_PURPOSE:
	case IDC_REASON_END_TIME:
		{
		pDC->SetBkColor(GetSolidBackgroundColor());
		return (HBRUSH)m_brBackground;
		}
	default:
		HANDLE_GENERIC_TRANSPARENT_CTL_COLOR();
		break;
	}	
}
