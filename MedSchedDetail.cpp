// MedSchedDetail.cpp : implementation file
//

#include "stdafx.h"
#include "color.h"
#include "MedSchedDetail.h"
#include "EditMedicationListDlg.h"
#include "EditComboBox.h"
#include "GlobalDrawingUtils.h"
#include "LetterWriting.h"
#include "MergeEngine.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "HistoryDlg.h"
#include "DontShowDlg.h"
#include "ReconcileMedicationsUtils.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "DecisionRuleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXT_STOP_DATE1		"Display until day"
#define TEXT_STOP_DATE2		""
#define TEXT_STOP_RANGE1	"Display for"
#define TEXT_STOP_RANGE2	"days."
#define TEXT_STOP_METHOD_DAYOF		"the last day."
#define TEXT_STOP_METHOD_DAYAFTER	"the day after the last day."
#define TEXT_BEFORE_APPT	"Before appt."
#define TEXT_AFTER_APPT		"After appt."

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (b.savon 2013-03-26 10:09) - PLID 54459
enum EMedSchedDetailDrug{
	msdID = 0,
	msdName = 1,
	msdFDB = 2,
	msdFDBOutOfDate = 3, //TES 5/9/2013 - PLID 56614
};

/////////////////////////////////////////////////////////////////////////////
// CMedSchedDetail dialog


CMedSchedDetail::CMedSchedDetail(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMedSchedDetail::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMedSchedDetail)
		m_DetailID = -1;
		m_MedSchedID = -1;
		iStopMethod = 1;
		iDurationType = 1;
		iApptDurationType = 1;
		m_strStopDayLabel1 = _T("");
		m_strStopDayLabel2 = _T("");
		m_strStopDayMethod = _T("");
	//}}AFX_DATA_INIT

	m_rcStopDayLabel1.left = m_rcStopDayLabel1.top = m_rcStopDayLabel1.right = m_rcStopDayLabel1.bottom = 0;
	m_rcStopDayLabel2.left = m_rcStopDayLabel2.top = m_rcStopDayLabel2.right = m_rcStopDayLabel2.bottom = 0;
	m_rcStopMethod.left = m_rcStopMethod.top = m_rcStopMethod.right = m_rcStopMethod.bottom = 0;

	m_strStopDayLabel1 = TEXT_STOP_DATE1;
	m_strStopDayLabel2 = TEXT_STOP_DATE2;
	m_strStopDayMethod = TEXT_STOP_METHOD_DAYOF;

	// (d.singleton 2012-04-13 17:21) - PLID 50442
	m_bIsPreOpSchedule = FALSE;
}

void CMedSchedDetail::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMedSchedDetail)
	DDX_Control(pDX, IDC_COLOR_PICKER_CTRL, m_ctrlColorPicker);
	DDX_Control(pDX, IDC_TYPE_COLOR, m_color);
	DDX_Text(pDX, IDC_STOP_DAY_LABEL, m_strStopDayLabel1);
	DDX_Text(pDX, IDC_STOP_DAY_LABEL_2, m_strStopDayLabel2);
	DDX_Text(pDX, IDC_STOP_DAY_METHOD, m_strStopDayMethod);
	DDX_Control(pDX, IDC_MEDSCHEDDETAIL_NAME, m_nxeditMedscheddetailName);
	DDX_Control(pDX, IDC_MEDDETAIL_START_DAY, m_nxeditMeddetailStartDay);
	DDX_Control(pDX, IDC_MEDDETAIL_STOP_DAY, m_nxeditMeddetailStopDay);
	DDX_Control(pDX, IDC_MEDDETAIL_START_NOTE, m_nxeditMeddetailStartNote);
	DDX_Control(pDX, IDC_MEDDETAIL_MIDDLE_NOTE, m_nxeditMeddetailMiddleNote);
	DDX_Control(pDX, IDC_MEDDETAIL_STOP_NOTE, m_nxeditMeddetailStopNote);
	DDX_Control(pDX, IDC_STOP_DAY_LABEL, m_nxstaticStopDayLabel);
	DDX_Control(pDX, IDC_STOP_DAY_LABEL_2, m_nxstaticStopDayLabel2);
	DDX_Control(pDX, IDC_STOP_DAY_METHOD, m_nxstaticStopDayMethod);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_DELETE_DETAIL, m_btnDeleteDetail);
	DDX_Control(pDX, IDC_BTN_WRITE_PRESCRIPTION, m_btnWritePrescription);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMedSchedDetail, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_EDIT_DETAIL_LIST, OnBtnEditDetailList)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BTN_DELETE_DETAIL, OnBtnDeleteDetail)
	ON_BN_CLICKED(IDC_BTN_WRITE_PRESCRIPTION, OnBtnWritePrescription)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMedSchedDetail message handlers

BEGIN_EVENTSINK_MAP(CMedSchedDetail, CNxDialog)
	ON_EVENT(CMedSchedDetail, IDC_TYPE_COLOR, -600 /* Click */, OnClickTypeColor, VTS_NONE)
	ON_EVENT(CMedSchedDetail, IDC_MEDSCHEDDETAIL_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenMedscheddetailTypeCombo, VTS_I4)
	ON_EVENT(CMedSchedDetail, IDC_MEDSCHEDDETAIL_MED_COMBO, 16 /* SelChosen */, OnSelChosenMedscheddetailMedCombo, VTS_I4)
	ON_EVENT(CMedSchedDetail, IDC_MEDSCHEDDETAIL_EVENT_COMBO, 16 /* SelChosen */, OnSelChosenMedscheddetailEventCombo, VTS_I4)
	ON_EVENT(CMedSchedDetail, IDC_MEDSCHEDDETAIL_MED_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedMedscheddetailMedCombo, VTS_I4 VTS_I4)
	ON_EVENT(CMedSchedDetail, IDC_MEDSCHEDDETAIL_MED_COMBO, 18, CMedSchedDetail::RequeryFinishedMedscheddetailMedCombo, VTS_I2)
END_EVENTSINK_MAP()

BOOL CMedSchedDetail::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-30 13:54) - PLID 29847 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnDeleteDetail.AutoSet(NXB_DELETE);
		m_btnWritePrescription.AutoSet(NXB_NEW);
		
		m_DetailType = BindNxDataListCtrl(this,IDC_MEDSCHEDDETAIL_TYPE_COMBO,GetRemoteData(),false);
		m_Med_Combo = BindNxDataListCtrl(this,IDC_MEDSCHEDDETAIL_MED_COMBO,GetRemoteData(),true);
		m_Event_Combo = BindNxDataListCtrl(this,IDC_MEDSCHEDDETAIL_EVENT_COMBO,GetRemoteData(),true);

		IRowSettingsPtr pRow;
		_variant_t var;
		pRow = m_DetailType->GetRow(-1);
		var = (long)1;
		var.vt = VT_UI1;
		pRow->PutValue(0,var);
		var = _bstr_t("Medication");
		pRow->PutValue(1,var);
		m_DetailType->AddRow(pRow);

		pRow = m_DetailType->GetRow(-1);
		var = (long)2;
		var.vt = VT_UI1;
		pRow->PutValue(0,var);
		var = _bstr_t("Event");
		pRow->PutValue(1,var);
		m_DetailType->AddRow(pRow);

		pRow = m_DetailType->GetRow(-1);
		var = (long)3;
		var.vt = VT_UI1;
		pRow->PutValue(0,var);
		var = _bstr_t("Note");
		pRow->PutValue(1,var);
		m_DetailType->AddRow(pRow);

		// (d.singleton 2012-04-13 16:26) - PLID 50442 if pre op calender add more options
		if(m_bIsPreOpSchedule) {
			this->SetWindowText("PreOp Calendar Detail");

			pRow = m_DetailType->GetRow(-1);
			var = (long)4;
			var.vt = VT_UI1;
			pRow->PutValue(0,var);
			var = _bstr_t("PreOp Note");
			pRow->PutValue(1,var);
			m_DetailType->AddRow(pRow);

			pRow = m_DetailType->GetRow(-1);
			var = (long)5;
			var.vt = VT_UI1;
			pRow->PutValue(0,var);
			var = _bstr_t("Surgery Note");
			pRow->PutValue(1,var);
			m_DetailType->AddRow(pRow);

			pRow = m_DetailType->GetRow(-1);
			var = (long)6;
			var.vt = VT_UI1;
			pRow->PutValue(0,var);
			var = _bstr_t("PostOp Note");
			pRow->PutValue(1,var);
			m_DetailType->AddRow(pRow);
		}

		m_DetailType->CurSel = 0;

		OnSelChosenMedscheddetailTypeCombo(0);

		m_ctrlColorPicker.SetColor(0);
		m_color.SetColor(0);

			// Calculate hyperlink rectangles
		{
			CWnd *pWnd;

			pWnd = GetDlgItem(IDC_STOP_DAY_LABEL);
			if (pWnd->GetSafeHwnd()) {
				// Get the position of the hotlinks
				pWnd->GetWindowRect(m_rcStopDayLabel1);
				ScreenToClient(&m_rcStopDayLabel1);

				//save this size
				m_rcStdStopDayLabel1 = m_rcStopDayLabel1;

				// Hide the static text that was there
				pWnd->ShowWindow(SW_HIDE);
			}
			
			pWnd = GetDlgItem(IDC_STOP_DAY_LABEL_2);
			if (pWnd->GetSafeHwnd()) {
				// Get the position of the hotlinks
				pWnd->GetWindowRect(m_rcStopDayLabel2);
				ScreenToClient(&m_rcStopDayLabel2);

				//save this size
				m_rcStdStopDayLabel2 = m_rcStopDayLabel2;

				// Hide the static text that was there
				pWnd->ShowWindow(SW_HIDE);
			}
			
			pWnd = GetDlgItem(IDC_STOP_DAY_METHOD);
			if (pWnd->GetSafeHwnd()) {
				// Get the position of the hotlinks
				pWnd->GetWindowRect(m_rcStopMethod);
				ScreenToClient(&m_rcStopMethod);

				//save this size
				m_rcStdStopMethod = m_rcStopMethod;

				// Hide the static text that was there
				pWnd->ShowWindow(SW_HIDE);
			}
		}

		if(m_DetailID != -1)
			Load();
	}
	NxCatchAll("Error in CMedSchedDetail::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMedSchedDetail::OnSelChosenMedscheddetailTypeCombo(long nRow) 
{
	try {
		// (d.singleton 2012-05-08 12:53) - PLID 50231 need to actually check for no row selected and force select a row if that is the case
		if(nRow == -1) {
			//no row selected force select first row
			m_DetailType->PutCurSel(0);
			//now that we forced a selection update nRow's value
			nRow = m_DetailType->GetCurSel();
		}

		switch(nRow) {
		case 0:
			//Medications

			//Show medications combo, hide events combo
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_WRITE_PRESCRIPTION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->EnableWindow(FALSE);

			//Show the button that will let you edit medications
			GetDlgItem(IDC_BTN_EDIT_DETAIL_LIST)->ShowWindow(SW_SHOW);		

			//Enable Stop Day, Stop Note, and Middle Note
			GetDlgItem(IDC_MEDDETAIL_STOP_DAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_MEDDETAIL_MIDDLE_NOTE)->EnableWindow(TRUE);
			GetDlgItem(IDC_MEDDETAIL_STOP_NOTE)->EnableWindow(TRUE);

			m_strStopDayLabel1 = TEXT_STOP_DATE1;
			iDurationType = 1;

			break;

		case 1:
			//Events

			//Show events combo, hide medications combo
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_WRITE_PRESCRIPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->ShowWindow(SW_SHOW);

			//Show the button that will let you edit events
			GetDlgItem(IDC_BTN_EDIT_DETAIL_LIST)->ShowWindow(SW_SHOW);

			//Disable Stop Day, Stop Note, and Middle Note
			GetDlgItem(IDC_MEDDETAIL_STOP_DAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_MIDDLE_NOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_STOP_NOTE)->EnableWindow(FALSE);

			SetDlgItemText(IDC_MEDDETAIL_STOP_DAY,"");
			SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,"");
			SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,"");

			break;

		case 2:
			//Note

			//Hide both the events and medications combos
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_WRITE_PRESCRIPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->EnableWindow(FALSE);

			//Hide the button that will let you edit events and medications
			GetDlgItem(IDC_BTN_EDIT_DETAIL_LIST)->ShowWindow(SW_HIDE);

			//Disable Stop Day, Stop Note, and Middle Note
			GetDlgItem(IDC_MEDDETAIL_STOP_DAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_MIDDLE_NOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_STOP_NOTE)->EnableWindow(FALSE);

			SetDlgItemText(IDC_MEDDETAIL_STOP_DAY,"");
			SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,"");
			SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,"");		

			break;

		case 3:
			// (d.singleton 2012-05-16 17:41) - PLID 50442 PreOp Notes (PreOp Calander Only)

			//Hide both the events and medications combos
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_WRITE_PRESCRIPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->EnableWindow(FALSE);

			//Hide the button that will let you edit events and medications
			GetDlgItem(IDC_BTN_EDIT_DETAIL_LIST)->ShowWindow(SW_HIDE);

			//Disable Stop Day, Stop Note, and Middle Note
			GetDlgItem(IDC_MEDDETAIL_STOP_DAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_MIDDLE_NOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_STOP_NOTE)->EnableWindow(FALSE);

			SetDlgItemText(IDC_MEDDETAIL_STOP_DAY,"");
			SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,"");
			SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,"");	

			m_strStopDayLabel1 = TEXT_BEFORE_APPT;
			iApptDurationType = 1;

			break;

		case 4:
			// (d.singleton 2012-05-16 17:41) - PLID 50442 Surgery Notes (PreOp Calander Only)

			//Hide both the events and medications combos
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_WRITE_PRESCRIPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->EnableWindow(FALSE);

			//Hide the button that will let you edit events and medications
			GetDlgItem(IDC_BTN_EDIT_DETAIL_LIST)->ShowWindow(SW_HIDE);

			//Disable Stop Day, Stop Note, and Middle Note
			GetDlgItem(IDC_MEDDETAIL_STOP_DAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_MIDDLE_NOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_STOP_NOTE)->EnableWindow(FALSE);

			SetDlgItemText(IDC_MEDDETAIL_STOP_DAY,"");
			SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,"");
			SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,"");	

			m_strStopDayLabel1 = TEXT_BEFORE_APPT;
			iApptDurationType = 1;

			break;

		case 5:
			// (d.singleton 2012-05-16 17:41) - PLID 50442 PostOp Notes (PreOp Calander Only)

			//Hide both the events and medications combos
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_MED_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_WRITE_PRESCRIPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->ShowWindow(SW_HIDE);
			// (d.singleton 2012-05-17 10:03) - PLID 50446 need to disable control so you cant get it to appear by hitting tab
			GetDlgItem(IDC_MEDSCHEDDETAIL_EVENT_COMBO)->EnableWindow(FALSE);

			//Hide the button that will let you edit events and medications
			GetDlgItem(IDC_BTN_EDIT_DETAIL_LIST)->ShowWindow(SW_HIDE);

			//Disable Stop Day, Stop Note, and Middle Note
			GetDlgItem(IDC_MEDDETAIL_STOP_DAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_MIDDLE_NOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_MEDDETAIL_STOP_NOTE)->EnableWindow(FALSE);

			SetDlgItemText(IDC_MEDDETAIL_STOP_DAY,"");
			SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,"");
			SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,"");	

			m_strStopDayLabel1 = TEXT_BEFORE_APPT;
			iApptDurationType = 1;

			break;
		}

		//in all cases, redraw the entirety of the hyperlink labels
		InvalidateRect(m_rcStdStopDayLabel1);
		InvalidateRect(m_rcStdStopDayLabel2);
		InvalidateRect(m_rcStdStopMethod);
	}NxCatchAll(__FUNCTION__);
}

void CMedSchedDetail::OnClickTypeColor() 
{
	CColor color = 0;
	
	m_ctrlColorPicker.ShowColor();
	color = m_ctrlColorPicker.GetColor();
	m_color.SetColor(color);
}


void CMedSchedDetail::Load()
{
	try {

		if(m_DetailID == -1)
			return;

		_RecordsetPtr rs = CreateRecordset("SELECT * FROM MedSchedDetailsT WHERE ID = %li",m_DetailID);

		if(rs->eof)
			return;

		CString str;
		_variant_t var;

		//Name
		var = rs->Fields->Item["Name"]->Value;
		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);
			SetDlgItemText(IDC_MEDSCHEDDETAIL_NAME, str);
		}

		//DetailType
		var = rs->Fields->Item["Type"]->Value;
		if(var.vt == VT_UI1) {
			m_DetailType->TrySetSelByColumn(0,var);
			OnSelChosenMedscheddetailTypeCombo(var.iVal - 1);
		}

		//Medication
		var = rs->Fields->Item["MedicationID"]->Value;
		if(var.vt == VT_I4) {
			if( m_Med_Combo->TrySetSelByColumn(0,var) == -1 ) {
				//it could be inactive
				// (c.haag 2007-02-02 17:53) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
				_RecordsetPtr rsMeds = CreateRecordset("SELECT EMRDataT.Data AS Name FROM DrugList "
					"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
					"WHERE DrugList.ID = %li", AdoFldLong(rs, "MedicationID"));
				if (! rsMeds->eof) {
					m_Med_Combo->PutComboBoxText(_bstr_t(AdoFldString(rsMeds, "Name")));
				}
			}
		}

		//Event
		var = rs->Fields->Item["EventID"]->Value;
		if(var.vt == VT_I4) {
			m_Event_Combo->TrySetSelByColumn(0,var);
		}

		//Start Day
		var = rs->Fields->Item["StartDay"]->Value;
		if(var.vt == VT_I4) {
			SetDlgItemInt(IDC_MEDDETAIL_START_DAY,var.lVal);
		}

		//Stop Day
		var = rs->Fields->Item["StopDay"]->Value;
		if(var.vt == VT_I4) {
			SetDlgItemInt(IDC_MEDDETAIL_STOP_DAY,var.lVal);
		}

		//Duration Type
		var = rs->Fields->Item["DurationType"]->Value;
		if(var.vt == VT_UI1) {
			//since the change only toggles back and forth,
			//and defaults to 1, we only need to do this if
			//the type is 2. This function will assign the
			//right value to iDurationType
			if(var.iVal == 2)
				ChangeDurationLabels();			
		}

		//Start Note
		var = rs->Fields->Item["StartNote"]->Value;
		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);
			SetDlgItemText(IDC_MEDDETAIL_START_NOTE, str);
		}

		//Middle Note
		var = rs->Fields->Item["MiddleNote"]->Value;
		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);
			SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE, str);
		}

		//Stop Note
		var = rs->Fields->Item["StopNote"]->Value;
		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);
			SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE, str);
		}

		//Color
		var = rs->Fields->Item["Color"]->Value;
		if(var.vt == VT_I4) {
			CColor color = var.lVal;
			m_ctrlColorPicker.SetColor(color);
			m_color.SetColor(color);
		}

		//Stop Method
		var = rs->Fields->Item["StopMethod"]->Value;
		if(var.vt == VT_UI1) {
			//since the change only toggles back and forth,
			//and defaults to 1, we only need to do this if
			//the type is 2. This function will assign the
			//right value to iStopMethod
			if(var.iVal == 2)
				ChangeMethodLabels();			
		}

		rs->Close();

	}NxCatchAll("Error loading medication schedule detail.");
}

BOOL CMedSchedDetail::Save()
{

	BOOL bIsNew = FALSE;

	if(m_DetailID == -1)
		bIsNew = TRUE;

	try {

		CString strName, strMedicationID = "NULL", strEventID = "NULL", strDurationType = "NULL", strStopDay = "NULL",
			strStartNote, strMiddleNote = "NULL", strStopNote = "NULL", strStopMethod = "NULL";

		long DetailType, MedicationID, EventID, StartDay, StopDay, Priority;


		//Name
		GetDlgItemText(IDC_MEDSCHEDDETAIL_NAME,strName);

		//DetailType
		DetailType = m_DetailType->GetValue(m_DetailType->CurSel,0).lVal;

		//error checking
		if(DetailType==1 && m_Med_Combo->CurSel == -1 && ((CString)(LPCTSTR)m_Med_Combo->GetComboBoxText()).IsEmpty()) {
			AfxMessageBox("You must select a Medication.");
			return FALSE;
		}
		if(DetailType==2 && m_Event_Combo->CurSel == -1) {
			AfxMessageBox("You must select an Event.");
			return FALSE;
		}

		//Medication
		if(DetailType==1) {
			if (m_Med_Combo->CurSel != -1) {
				//its not inactive
				MedicationID = m_Med_Combo->GetValue(m_Med_Combo->CurSel,0).lVal;
			}
			else if (!((CString)(LPCTSTR)m_Med_Combo->GetComboBoxText()).IsEmpty()) {
				//its an inactive medication
				if (bIsNew) {
					//its new and inactive, we don't allow that
					MsgBox("You cannot save a new medication schedule detail with an inactive medication.");
					return FALSE;
				}
				else {
					// (c.haag 2007-02-02 16:21) - PLID 24561 - There is no longer a name field in DrugList
					_RecordsetPtr rsMed = CreateRecordset("SELECT DrugList.ID FROM DrugList "
						"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
						"WHERE EMRDataT.Data = '%s'", _Q((CString)(LPCTSTR)m_Med_Combo->GetComboBoxText()));
					MedicationID = AdoFldLong(rsMed, "ID");
				}
			}
			strMedicationID.Format("%li",MedicationID);
		}

		//Event
		else if(DetailType==2 && m_Event_Combo->CurSel != -1) {
			EventID = m_Event_Combo->GetValue(m_Event_Combo->CurSel,0).lVal;
			strEventID.Format("%li",EventID);
		}		

		//Start Day
		// (d.singleton 2012-11-27 17:42) - PLID 53371 added success param so we can check for null values
		BOOL bSuccess;
		StartDay = GetDlgItemInt(IDC_MEDDETAIL_START_DAY, &bSuccess);

		//do error checking for medication schedule
		// (d.singleton 2012-11-27 17:48) - PLID 53371 do not allow value of zero if its not a pre op calender
		if(StartDay==0 && !m_bIsPreOpSchedule) {
			AfxMessageBox("You must enter a Start Day.");
			return FALSE;
		}
		// (d.singleton 2012-11-27 17:45) - PLID 53371 do error checking for pre-op calendar allow for value of zero but not nulls
		if(m_bIsPreOpSchedule && !bSuccess) {
			AfxMessageBox("You must enter a Start Day.");
			return FALSE;
		}

		// (d.singleton 2012-04-18 16:23) - PLID 50442 if this is a pre op calender we need to grab the appt duration type
		if(m_bIsPreOpSchedule && (DetailType == 4 || DetailType == 5 || DetailType == 6)) {						
			strDurationType.Format("%li", iApptDurationType);
		}

		//Duration Type
		if(DetailType==1 && m_Med_Combo->CurSel != -1) {
			strDurationType.Format("%li",iDurationType);
		}

		//Stop Day
		StopDay = GetDlgItemInt(IDC_MEDDETAIL_STOP_DAY);
		if(StopDay > 0) {
			strStopDay.Format("%li",StopDay);
		}

		//Start Note
		GetDlgItemText(IDC_MEDDETAIL_START_NOTE,strStartNote);

		//Middle Note		
		if(DetailType==1 && m_Med_Combo->CurSel != -1) {
			GetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,strMiddleNote);
			strMiddleNote = "'" + _Q(strMiddleNote) + "'";
		}

		//Stop Note
		if(DetailType==1 && m_Med_Combo->CurSel != -1) {
			GetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,strStopNote);
			strStopNote = "'" + _Q(strStopNote) + "'";
		}

		//Color
		CColor color = m_ctrlColorPicker.GetColor();

		//Stop Method
		if(DetailType==1 && m_Med_Combo->CurSel != -1) {
			strStopMethod.Format("%li",iStopMethod);
		}

		//do error checking
		if(DetailType==1 && m_Med_Combo->CurSel != -1) {
			if(StopDay == 0) {
				AfxMessageBox("You must enter a Stop Day.");
				return FALSE;
			}
			if(StopDay < StartDay && iDurationType == 1) {
				AfxMessageBox("The Stop Day you entered was before the Start Day.\nPlease make the Stop Day be greater than or equal to the Start Day.");
				return FALSE;
			}
		}

		if(bIsNew) {
			//save new record
			if(IsRecordsetEmpty("SELECT TOP 1 ID FROM MedSchedDetailsT WHERE MedSchedID = %li",m_MedSchedID)) {
				Priority = 1;
			}
			else {
				_RecordsetPtr rs = CreateRecordset("SELECT Max(Priority) AS TopPriority FROM MedSchedDetailsT WHERE MedSchedID = %li",m_MedSchedID);
				if(!rs->eof) {
					Priority = rs->Fields->Item["TopPriority"]->Value.lVal + 1;
				}
				else
					Priority = 1;
			}

			m_DetailID = NewNumber("MedSchedDetailsT","ID");

			ExecuteSql("INSERT INTO MedSchedDetailsT (ID, MedSchedID, Name, Color, Priority, Type, MedicationID, "
				"EventID, StartDay, StartNote, MiddleNote, DurationType, StopDay, StopNote, StopMethod) "
				"VALUES (%li, %li, '%s', %li, %li, %li, %s, %s, %li, '%s', %s, %s, %s, %s, %s)",m_DetailID,m_MedSchedID,
				_Q(strName), color, Priority, DetailType, strMedicationID, strEventID, StartDay, _Q(strStartNote), strMiddleNote,
				strDurationType, strStopDay, strStopNote, strStopMethod);	
		}
		else {
			//update existing record
			ExecuteSql("UPDATE MedSchedDetailsT SET Name = '%s', Color = %li, Type = %li, MedicationID = %s, EventID = %s, "
				"StartDay = %li, StartNote = '%s', MiddleNote = %s, DurationType = %s, StopDay = %s, StopNote = %s, StopMethod = %s "
				"WHERE ID = %li", _Q(strName), color, DetailType, strMedicationID, strEventID,
 				StartDay, _Q(strStartNote), strMiddleNote, strDurationType, strStopDay, strStopNote, strStopMethod, m_DetailID);
		}

		return TRUE;
	
	}NxCatchAll("Error saving medication schedule detail.");

	return FALSE;
}

void CMedSchedDetail::OnBtnEditDetailList() 
{
	try {
		int Type = m_DetailType->CurSel;

		switch(Type) {
		case 0: {
				_variant_t var;
				long nCurSel = m_Med_Combo->GetCurSel();
				CString strCurText = (LPCTSTR)m_Med_Combo->GetComboBoxText();
				if(nCurSel !=-1)
					var = m_Med_Combo->GetValue(m_Med_Combo->GetCurSel(),0);

				// (a.walling 2007-04-04 15:19) - PLID 25459 - Chris put a check here for EMR permissions. We never really
				// checked for any permission other than the EMR one. But now let's check for the new Edit Medication List.

				// (will prompt for passwords)
				if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
					return;
				}

				CEditMedicationListDlg dlg(this);
				dlg.DoModal();
				m_Med_Combo->Requery();
				if((!strCurText.IsEmpty()) && m_Med_Combo->GetCurSel() == -1 && m_Med_Combo->TrySetSelByColumn(0,var) == -1) {
					m_Med_Combo->PutComboBoxText(_bstr_t(strCurText));
					
				}
				else {
					m_Med_Combo->SetSelByColumn(0,var);
				}
			}
			break;
		case 1: {
				_variant_t var;
				if(m_Event_Combo->GetCurSel()!=-1)
					var = m_Event_Combo->GetValue(m_Event_Combo->GetCurSel(),0);

				// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
				CEditComboBox(this, 8, m_Event_Combo, "Edit Combo Box").DoModal();

				m_Event_Combo->SetSelByColumn(0,var);
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

void CMedSchedDetail::OnLButtonDown(UINT nFlags, CPoint point) 
{
	try {
		DoClickHyperlink(nFlags, point);
	}NxCatchAll(__FUNCTION__);
	CNxDialog::OnLButtonDown(nFlags, point);
}

void CMedSchedDetail::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	try {
		DoClickHyperlink(nFlags, point);	
	}NxCatchAll(__FUNCTION__);
	CNxDialog::OnLButtonDblClk(nFlags, point);
}

void CMedSchedDetail::DoClickHyperlink(UINT nFlags, CPoint point)
{
	//if not medications, we don't want to be able to change these labels
	if(m_DetailType->CurSel == 0) {

		if (m_rcStopDayLabel1.PtInRect(point)) {
			ChangeDurationLabels();
		}

		if (m_rcStopMethod.PtInRect(point)) {
			ChangeMethodLabels();			
		}
	}
	else if(m_bIsPreOpSchedule && m_DetailType->CurSel > 2) {
		if(m_rcStopDayLabel1.PtInRect(point)) {
			ChangeDurationLabels();
		}
	}

}

void CMedSchedDetail::OnPaint() 
{
	try {
		CPaintDC dc(this); // device context for painting
		
		DrawStopDayLabel(&dc);
		DrawStopMethodLabel(&dc);
	}NxCatchAll(__FUNCTION__);
}

void CMedSchedDetail::DrawStopDayLabel(CDC *pdc) {

	BOOL bSelectionEnabled = TRUE;

	if(m_DetailType->CurSel != 0 && m_DetailType->CurSel < 3)
		bSelectionEnabled = FALSE;

	// (d.singleton 2012-04-17 16:39) - PLID 50442 need this to be different if we are printing the pre op calander
	if(m_bIsPreOpSchedule && m_DetailType->CurSel > 2) {
		//make sure we get the original rect size just in case our previous draw resized the rect
		m_rcStopDayLabel1 = m_rcStdStopDayLabel1;
		DrawTextOnDialog(this, pdc, m_rcStopDayLabel1, m_strStopDayLabel1, dtsHyperlink, true, DT_LEFT, true, false, 0);
	}
	else {
		// Draw the "Display until day" or "Display for" text, right justified
		// (j.jones 2008-05-01 16:14) - PLID 29874 - Set background color to transparent
		//make sure we get the original rect size just in case our previous draw resized the rect
		m_rcStopDayLabel1 = m_rcStdStopDayLabel1;
		DrawTextOnDialog(this, pdc, m_rcStopDayLabel1, m_strStopDayLabel1, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_RIGHT, true, false, 0);
	}

	// Draw the "" or "days." text, left justified
	// (j.jones 2008-05-01 16:06) - PLID 29874 - Set background color to transparent
	DrawTextOnDialog(this, pdc, m_rcStopDayLabel2, m_strStopDayLabel2, dtsText, false, DT_LEFT, true, false, 0);
}

void CMedSchedDetail::DrawStopMethodLabel(CDC *pdc) {

	BOOL bSelectionEnabled = TRUE;

	if(m_DetailType->CurSel != 0)
		bSelectionEnabled = FALSE;

	// Draw the "the last day." or "the day after the last day." text, left justified
	// (j.jones 2008-05-01 16:06) - PLID 29874 - Set background color to transparent
	DrawTextOnDialog(this, pdc, m_rcStopMethod, m_strStopDayMethod, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
}

BOOL CMedSchedDetail::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (d.singleton 2012-04-19 10:12) - PLID 50442 also needs hyperlink if any of the new appt day types
		if(m_DetailType->CurSel == 0 || m_DetailType->CurSel > 2) {

			if (m_rcStopDayLabel1.PtInRect(pt) || m_rcStopMethod.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
	
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CMedSchedDetail::OnOK() 
{
	if(!Save())
		return;
	
	CNxDialog::OnOK();
}

void CMedSchedDetail::OnCancel() 
{	
	CNxDialog::OnCancel();
}

void CMedSchedDetail::DeleteDetail()
{
	try {

		if(m_DetailID == -1) {
			CNxDialog::OnCancel();
			return;
		}

		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("DeleteSchedDetail");
		trans.Begin();

		try {

			long Priority;
			_RecordsetPtr rs = CreateRecordset("SELECT Priority FROM MedSchedDetailsT WHERE ID = %li",m_DetailID);
			if(!rs->eof) {
				Priority = rs->Fields->Item["Priority"]->Value.lVal;
			}
			else {
				trans.Rollback();
				AfxMessageBox("This detail has already been deleted.");
				rs->Close();
				CNxDialog::OnCancel();
				return;
			}
			rs->Close();

			ExecuteSql("UPDATE MedSchedDetailsT SET Priority = Priority - 1 WHERE MedSchedID = %li AND Priority > %li",m_MedSchedID,Priority);

			ExecuteSql("DELETE FROM MedSchedDetailsT WHERE ID = %li",m_DetailID);

			trans.Commit();

			CNxDialog::OnCancel();
							
			return;

		}NxCatchAll("Error deleting medication schedule detail.");

	}NxCatchAll("Error in CMedSchedDetail::DeleteDetail");
}

void CMedSchedDetail::OnBtnDeleteDetail() 
{
	DeleteDetail();	
}

void CMedSchedDetail::ChangeDurationLabels()
{
	// (d.singleton 2012-05-16 17:52) - PLID 50442 if the new appt day types need a new duration label "before appt" "after appt"
	if(m_bIsPreOpSchedule && m_DetailType->CurSel > 2) {
		m_rcStopDayLabel1 = m_rcStdStopDayLabel1;
		if(iApptDurationType == 1) {
			m_strStopDayLabel1 = TEXT_AFTER_APPT;
			iApptDurationType = 2;
		} else {
			m_strStopDayLabel1 = TEXT_BEFORE_APPT;
			iApptDurationType = 1;
		}
		InvalidateRect(m_rcStopDayLabel1);
	} else {
		m_rcStopDayLabel1 = m_rcStdStopDayLabel1;
		m_rcStopDayLabel2 = m_rcStdStopDayLabel2;
		if (iDurationType == 1) {
			m_strStopDayLabel1 = TEXT_STOP_RANGE1;
			m_strStopDayLabel2 = TEXT_STOP_RANGE2;			
			iDurationType = 2;
		} else {
			m_strStopDayLabel1 = TEXT_STOP_DATE1;
			m_strStopDayLabel2 = TEXT_STOP_DATE2;
			iDurationType = 1;
		}
		InvalidateRect(m_rcStopDayLabel1);
		InvalidateRect(m_rcStopDayLabel2);
	}
}

void CMedSchedDetail::ChangeMethodLabels()
{
	m_rcStopMethod = m_rcStdStopMethod;
	if (iStopMethod == 1) {
		m_strStopDayMethod = TEXT_STOP_METHOD_DAYAFTER;
		iStopMethod = 2;
	} else {
		m_strStopDayMethod = TEXT_STOP_METHOD_DAYOF;
		iStopMethod = 1;
	}
	InvalidateRect(m_rcStopMethod);
}

void CMedSchedDetail::OnSelChosenMedscheddetailMedCombo(long nRow) 
{
	try {
		if(m_Med_Combo->GetCurSel()==-1)
			return;

		_variant_t var = m_Med_Combo->GetValue(m_Med_Combo->GetCurSel(),1);
		if(var.vt == VT_BSTR) {
			UpdateNotes(CString(var.bstrVal),1);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMedSchedDetail::OnSelChosenMedscheddetailEventCombo(long nRow) 
{
	try {
		if(m_Event_Combo->GetCurSel()==-1)
			return;

		_variant_t var = m_Event_Combo->GetValue(m_Event_Combo->GetCurSel(),1);
		if(var.vt == VT_BSTR) {
			UpdateNotes(CString(var.bstrVal),2);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMedSchedDetail::UpdateNotes(CString strName, long Type)
{
	CString str, strNewName;

	GetDlgItemText(IDC_MEDSCHEDDETAIL_NAME,str);
	if(str.GetLength()==0) {
		SetDlgItemText(IDC_MEDSCHEDDETAIL_NAME,strName);
	}

	if(Type == 2)
		SetDlgItemText(IDC_MEDDETAIL_START_NOTE,strName);
	else {
		strNewName = "Start " + strName;
		SetDlgItemText(IDC_MEDDETAIL_START_NOTE,strNewName);
		strNewName = "Continue " + strName;
		SetDlgItemText(IDC_MEDDETAIL_MIDDLE_NOTE,strNewName);
		strNewName = "Stop " + strName;
		SetDlgItemText(IDC_MEDDETAIL_STOP_NOTE,strNewName);
	}
}

void CMedSchedDetail::OnBtnWritePrescription() 
{
	try {

		if(m_Med_Combo->CurSel == -1 && ((CString)(LPCTSTR)m_Med_Combo->GetComboBoxText()).IsEmpty()) {
			AfxMessageBox("Please select a medication first.");
			return;
		}

		//check to see that they have permissions - this will prompt a password
		if(!CheckCurrentUserPermissions(bioPatientMedication,sptCreate))
			return;

		long nMedicationID;
		//see if this is an inactive medication
		if(m_Med_Combo->CurSel == -1 && !((CString)(LPCTSTR)m_Med_Combo->GetComboBoxText()).IsEmpty()) {
			//its inactive, we have to get the ID from the database
			//warn them before we do it though
			if(MsgBox(MB_YESNO, "This medication is inactive, are you sure you want to prescribe it?") == IDYES) {
				_RecordsetPtr rs = CreateRecordset("SELECT MedicationID FROM MedSchedDetailsT WHERE ID = %li",m_DetailID);
				nMedicationID = AdoFldLong(rs, "MedicationID");
			}
			else  {
				return;
			}
		}
		else {
			nMedicationID = m_Med_Combo->GetValue(m_Med_Combo->CurSel,0).lVal;		
		}

		//Check that the patient isn't allergic to it.
		if(!CheckAllergies(GetActivePatientID(), nMedicationID)) return;
			
		//insert the medication into PatientMedications			
		CString strTimeToDisplay = FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoDate);
		CString strTimeToInsert = _Q(FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate));
		
		_RecordsetPtr rs;
		// (c.haag 2007-02-02 17:53) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		// (d.thompson 2008-12-01) - PLID 32174 - DefaultPills is now DefaultQuantity, Description is now PatientInstructions (and parameterized)
		// (d.thompson 2009-01-15) - PLID 32176 - DrugList.Unit is now DrugStrengthUnitsT.Name, joined from DrugList.StrengthUnitID
		// (d.thompson 2009-03-11) - PLID 33481 - Actually this should use QuantityUnitID, not StrengthUnitID
		rs = CreateParamRecordset("SELECT EMRDataT.Data AS Name, PatientInstructions, DefaultRefills, DefaultQuantity, "
			"COALESCE(DrugStrengthUnitsT.Name, '') AS Unit FROM DrugList "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"LEFT JOIN DrugStrengthUnitsT ON DrugList.QuantityUnitID = DrugStrengthUnitsT.ID "
			"WHERE DrugList.ID = {INT}", nMedicationID);

		
		FieldsPtr fields;
		fields = rs->Fields;

		//Put the variables from the recordset into local variables
		CString strName, strPatientExplanation, strUnit, strRefills, strPills;
		long nLocID = -1; //nProvID = -1;
		_variant_t varProvID = g_cvarNull; //// (a.vengrofski 2009-12-18 17:49) - PLID <34890> - changed from a long to a variant for the parameterization

		strName = VarString(fields->Item["Name"]->Value);
		// (d.thompson 2008-12-01) - PLID 32174 - Description is now PatientInstructions
		strPatientExplanation = VarString(fields->Item["PatientInstructions"]->Value);
		strRefills = VarString(fields->Item["DefaultRefills"]->Value);
		// (d.thompson 2008-12-01) - PLID 32174 - DefaultPills is now DefaultQuantity
		strPills = VarString(fields->Item["DefaultQuantity"]->Value);
		strUnit = AdoFldString(fields, "Unit");
		

		rs->Close();

		rs = CreateRecordset("SELECT MainPhysician, Location FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PatientsT.PersonID = %li",GetActivePatientID());
		_variant_t var;
		if(!rs->eof) {
			varProvID = rs->Fields->Item["MainPhysician"]->Value;// (a.vengrofski 2009-12-18 18:23) - PLID <34890> - changed from long to var

			var = rs->Fields->Item["Location"]->Value;
			if(var.vt == VT_I4)
				nLocID = var.lVal;
		}
		rs->Close();

		if(nLocID == -1)
			nLocID = GetCurrentLocationID();

		if(varProvID.vt == VT_NULL) {
			rs = CreateRecordset("SELECT DefaultProviderID FROM LocationsT WHERE ID = %li",GetCurrentLocationID());
			if(!rs->eof) {
				varProvID = rs->Fields->Item["DefaultProviderID"]->Value;// (a.vengrofski 2009-12-18 18:23) - PLID <34890> - changed from long to var
			}
			rs->Close();
		}
		
		//Insert all the defaults into the PatientMedications Table
		//TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation, PillsPerBottle to Quantity
		// (a.vengrofski 2009-12-18 18:21) - PLID <34890> - changed to a param query for fun.
		// (j.jones 2010-01-22 11:31) - PLID 37016 - supported InputByUserID
		// (j.armen 2013-05-24 15:22) - PLID 56863 - Patient Medications has an identity.
		rs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @PatientMedication_ID TABLE(ID INT NOT NULL)\r\n"
			"INSERT INTO PatientMedications (\r\n"
			"	PatientID, MedicationID, PatientExplanation, EnglishDescription, PrescriptionDate,\r\n"
			"	RefillsAllowed, Quantity, ProviderID, LocationID, Unit,\r\n"
			"	InputByUserID)\r\n"
			"OUTPUT inserted.ID INTO @PatientMedication_ID\r\n"
			"SELECT\r\n"
			"	{INT}, {INT}, {STRING}, {STRING}, {STRING},\r\n"
			"	{STRING}, {STRING}, {VT_I4}, {INT}, {STRING},\r\n"
			"	{INT}\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT ID FROM @PatientMedication_ID",
			GetActivePatientID(), nMedicationID, strPatientExplanation, GetLatinToEnglishConversion(strPatientExplanation), strTimeToInsert,
			strRefills, strPills, varProvID, nLocID, strUnit,
			GetCurrentUserID());

		long nID = AdoFldLong(rs, "ID");

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientPrescriptionCreated, GetActivePatientID(), "", strName, aepMedium, aetCreated);

		// (c.haag 2010-02-18 12:13) - PLID 37384 - Let the user apply the prescriptions to the current medications list.
		// (j.jones 2010-08-23 09:23) - PLID 40178 - this is user-created so the NewCropGUID is empty
		// (j.jones 2011-05-02 15:39) - PLID 43450 - pass in the patient explanation as the sig
		// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also no longer needs the Sig nor NewCropGUID
		CDWordArray arNewCDSInterventions;
		//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
		ReconcileCurrentMedicationsWithOneNewPrescription(GetActivePatientID(), nID, GetSysColor(COLOR_BTNFACE), this, arNewCDSInterventions);
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

		if (GetMainFrame()->GetActiveView())
			GetMainFrame()->GetActiveView()->UpdateView();

		///////////////////////////////////////
		//now merge it

		if(IDNO == MessageBox("The prescription was generated and added to the patient's medication list.\n"
			"Would you like to merge the prescription to Word now?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		CWaitCursor pWait;

		// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
		// based on how many prescriptions are printed
		BOOL bExactCountFound = FALSE;
		BOOL bOtherCountFound = FALSE;
		CString strDefTemplate = GetDefaultPrescriptionTemplateByCount(1, bExactCountFound, bOtherCountFound);

		if(strDefTemplate.IsEmpty()) {
			AfxMessageBox("There is no default template for prescriptions.\n"
				"The prescription was created in the patient's medications tab, but could not be merged to Word.");
			return;
		}
		//if no template was found for the exact count, and there are some for other counts,
		//ask if they wish to continue or not (will use the standard default otherwise)
		else if(!bExactCountFound && bOtherCountFound) {
			if(IDNO == MessageBox("There is no default template configured for use with one prescription, "
				"but there are templates configured for other counts of prescriptions.\n\n"
				"Would you like to continue merging using the standard prescription template?",
				"Practice", MB_ICONQUESTION|MB_YESNO)) {

				AfxMessageBox("The prescription was created in the patient's medications tab, but will not be be printed.");
				return;
			}
		}

		// At this point, we know we want to do it by word merge, so make sure word exists
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}
		
		/// Generate the temp table
		CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", GetActivePatientID());
		CString strMergeT = CreateTempIDTable(strSql, "ID");
		
		// Merge
		CMergeEngine mi;

		// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
		if(!mi.LoadSenderInfo(TRUE)) {
			return;
		}
		
		//add this prescriptions to the merge
		mi.m_arydwPrescriptionIDs.Add(nID);

		if (g_bMergeAllFields)
			mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

		mi.m_nFlags |= BMS_SAVE_FILE_NO_HISTORY; //save the file, do not save in history

		try {
			// Do the merge
			// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
			if (mi.MergeToWord(GetTemplatePath("Forms", strDefTemplate), std::vector<CString>(), strMergeT))
			{
				//Update patient history here, because multiple merges per patient ID
				//will screw up the merge engine's method of doing it. But hey,
				//we get to make the description a lot better as a result!

				CString strDescription = "Prescription printed for ";
				// (c.haag 2007-02-02 17:53) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
				_RecordsetPtr rs = CreateRecordset("SELECT PatientMedications.*, EMRDataT.Data AS Name FROM PatientMedications LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
					"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
					"WHERE PatientMedications.ID = %li", (long)nID);
				if (!rs->eof) {
					CString MedicationName = CString(rs->Fields->Item["Name"]->Value.bstrVal);
					CString strRefills = CString(rs->Fields->Item["RefillsAllowed"]->Value.bstrVal);
					//TES 2/10/2009 - PLID 33002 - Renamed PillsPerBottle to Quantity
					CString strQuantity = CString(rs->Fields->Item["Quantity"]->Value.bstrVal);
					CString strUnit = AdoFldString(rs, "Unit");
					CString strExtra;
					strExtra.Format("%s, Quantity: %s %s, Refills: %s", MedicationName, strQuantity, strUnit, strRefills);
					strDescription += strExtra;
				}
				rs->Close();

				// (j.jones 2008-09-04 16:56) - PLID 30288 - converted to use CreateNewMailSentEntry,
				// which creates the data in one batch and sends a tablechecker
				// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
				CreateNewMailSentEntry(GetActivePatientID(), strDescription, SELECTION_WORDDOC, mi.m_strSavedAs, GetCurrentUserName(), "", GetCurrentLocationID());
			}

		} NxCatchAll("CMedSchedDetail::PrintPrescription");
	
	}NxCatchAll("Error in CMedSchedDetail::OnBtnWritePrescription()");
}

void CMedSchedDetail::OnTrySetSelFinishedMedscheddetailMedCombo(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//it could be inactive
			// (c.haag 2007-02-02 17:53) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			_RecordsetPtr rsMeds = CreateRecordset("SELECT EMRDataT.Data AS Name FROM DrugList "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"WHERE DrugList.ID IN (SELECT MedicationID FROM MedSchedDetailsT WHERE ID = %li)", m_DetailID);
			if (! rsMeds->eof) {
				m_Med_Combo->PutComboBoxText(_bstr_t(AdoFldString(rsMeds, "Name")));
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CMedSchedDetail::RequeryFinishedMedscheddetailMedCombo(short nFlags)
{
	try{
		//we don't need to check for the license because maybe they had newcrop and ditched it, they'll still need to see the prescriptions
		long p = m_Med_Combo->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;	
		while (p) {
			m_Med_Combo->GetNextRowEnum(&p, &lpDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

			long nFromFDB = VarLong(pRow->GetValue(msdFDB), -1);
			if( nFromFDB > 0 ){ // (b.savon 2013-01-07 13:18) - PLID 54459 - Color imported meds
				//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
				if(VarBool(pRow->GetValue(msdFDBOutOfDate), FALSE)) {
					pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
				}
				else {
					pRow->PutBackColor(ERX_IMPORTED_COLOR);
				}
			}
		}	
	}NxCatchAll(__FUNCTION__);
}
