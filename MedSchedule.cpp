// MedSchedule.cpp : implementation file
//

#include "stdafx.h"
#include "MedSchedule.h"
#include "MedSchedDetail.h"
#include "Color.h"
#include "GetNewIDName.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DETAIL_COLUMN_ID			0
#define DETAIL_COLUMN_NAME			1
#define DETAIL_COLUMN_DURATION_DESC	2
#define DETAIL_COLUMN_PRIORITY		3
#define DETAIL_COLUMN_COLOR			4
#define DETAIL_COLUMN_NOTE			5

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMedSchedule dialog


CMedSchedule::CMedSchedule(CWnd* pParent /*=NULL*/, BOOL bIsPreOpSched /*=FALSE*/)
	: CNxDialog(CMedSchedule::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMedSchedule)
		m_MedSchedID = -1;
		m_bIsNew = TRUE;
		m_bPrintOnClose = FALSE;
		// (d.singleton 2012-04-05 10:49) - PLID 50436
		m_bIsPreOpSchedule = bIsPreOpSched;
		m_nProcedureID = -1;
	//}}AFX_DATA_INIT
}


void CMedSchedule::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMedSchedule)
	DDX_Control(pDX, IDC_APPLY_ON_DATE, m_btnApplyStartDate);
	DDX_Control(pDX, IDC_MOVE_DETAIL_DOWN, m_btnMoveDetailDown);
	DDX_Control(pDX, IDC_MOVE_DETAIL_UP, m_btnMoveDetailUp);
	DDX_Control(pDX, IDC_SCHED_START_DATE, m_dtApplyDate);
	DDX_Control(pDX, IDC_MEDSCHED_NAME, m_nxeditMedschedName);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_DELETE_SCHED, m_btnDeleteSched);
	DDX_Control(pDX, IDC_PRINT_MEDSCHED, m_btnPrintMedSched);
	DDX_Control(pDX, IDC_ADD_SCHED_DETAIL, m_btnAddSchedDetail);
	DDX_Control(pDX, IDC_EDIT_SCHED_DETAIL, m_btnEditSchedDetail);
	DDX_Control(pDX, IDC_REMOVE_SCHED_DETAIL, m_btnRemoveSchedDetail);
	DDX_Control(pDX, IDC_SAVE_TO_TEMPLATE, m_btnSaveToTemplate);
	DDX_Control(pDX, IDC_SAVE_NEW_TEMPLATE, m_btnSaveNewTemplate);
	DDX_Control(pDX, IDC_DELETE_TEMPLATE, m_btnDeleteTemplate);
	DDX_Control(pDX, IDC_TEMPLATES_GROUPBOX, m_btnTemplatesGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMedSchedule, CNxDialog)
	//{{AFX_MSG_MAP(CMedSchedule)
	ON_BN_CLICKED(IDC_ADD_SCHED_DETAIL, OnAddSchedDetail)
	ON_BN_CLICKED(IDC_EDIT_SCHED_DETAIL, OnEditSchedDetail)
	ON_BN_CLICKED(IDC_REMOVE_SCHED_DETAIL, OnRemoveSchedDetail)
	ON_BN_CLICKED(IDC_APPLY_ON_DATE, OnApplyOnDate)
	ON_BN_CLICKED(IDC_MOVE_DETAIL_UP, OnMoveDetailUp)
	ON_BN_CLICKED(IDC_MOVE_DETAIL_DOWN, OnMoveDetailDown)
	ON_BN_CLICKED(IDC_BTN_DELETE_SCHED, OnBtnDeleteSched)
	ON_BN_CLICKED(IDC_PRINT_MEDSCHED, OnPrintMedsched)
	ON_BN_CLICKED(IDC_LOAD_FROM_TEMPLATE, OnLoadFromTemplate)
	ON_BN_CLICKED(IDC_SAVE_TO_TEMPLATE, OnSaveToTemplate)
	ON_BN_CLICKED(IDC_SAVE_NEW_TEMPLATE, OnSaveNewTemplate)
	ON_BN_CLICKED(IDC_DELETE_TEMPLATE, OnDeleteTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMedSchedule message handlers

BOOL CMedSchedule::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (d.singleton 2012-05-16 17:20) - PLID 50436 reset the dlg text
		if(m_bIsPreOpSchedule) {
			this->SetWindowText("PreOp Calendar Setup");	
		}

		m_btnMoveDetailUp.SetIcon(IDI_UARROW);
		m_btnMoveDetailDown.SetIcon(IDI_DARROW);
		// (c.haag 2008-04-30 15:17) - PLID 29847 - NxIconify more buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnDeleteSched.AutoSet(NXB_DELETE);
		m_btnPrintMedSched.AutoSet(NXB_PRINT_PREV);
		m_btnAddSchedDetail.AutoSet(NXB_NEW);
		m_btnEditSchedDetail.AutoSet(NXB_MODIFY);
		m_btnRemoveSchedDetail.AutoSet(NXB_DELETE);
		m_btnSaveToTemplate.AutoSet(NXB_MODIFY);
		m_btnSaveNewTemplate.AutoSet(NXB_NEW);
		m_btnDeleteTemplate.AutoSet(NXB_DELETE);
		
		m_DetailList = BindNxDataListCtrl(this,IDC_MEDSCHEDDETAIL_LIST,GetRemoteData(),false);
		m_ProcedureList = BindNxDataListCtrl(this,IDC_MEDSCHED_PROCEDURE_COMBO,GetRemoteData(),true);
		m_TemplateList = BindNxDataListCtrl(this,IDC_MEDSCHED_TEMPLATE_LIST,GetRemoteData(),true);

		IRowSettingsPtr pRow;
		_variant_t var;
		pRow = m_ProcedureList->GetRow(-1);
		var = (long)-1;
		pRow->PutValue(0,var);
		var = _bstr_t("<No Procedure Selected>");
		pRow->PutValue(1,var);
		m_ProcedureList->AddRow(pRow);	

		//JJ - I'd rather this be in code rather than the resources, so it is easier to edit
		m_DetailList->GetColumn(DETAIL_COLUMN_DURATION_DESC)->PutFieldName(
			"(CASE WHEN StopDay Is NULL THEN ('Day ' + Convert(varchar,StartDay)) ELSE ( "
			"(CASE WHEN DurationType = 1 THEN ('From Day ' + Convert(varchar,StartDay) + ' to Day ' + Convert(varchar,StopDay)) "
			"ELSE ('From Day ' + Convert(varchar,StartDay) + ' for ' + Convert(varchar,StopDay) + ' days') END)) END)");

		var = COleDateTime::GetCurrentTime();
		m_dtApplyDate.SetValue(var);

		GetDlgItem(IDC_SCHED_START_DATE)->EnableWindow(FALSE);

		// (d.singleton 2012-04-05 11:24) - PLID 50436 we are running from the PIC so auto select the procedure and set the date;
		if(m_bIsPreOpSchedule)
		{
			m_ProcedureList->FindByColumn(0, m_nProcedureID, m_ProcedureList->GetFirstRowEnum(), TRUE);
			GetDlgItem(IDC_SCHED_START_DATE)->EnableWindow(TRUE);
			m_btnApplyStartDate.SetCheck(1); 
			COleDateTime dt = COleDateTime::GetCurrentTime();
			for(int i = 0; i < m_arAppointmentInfo.GetCount(); i++) {
				if(dt > m_arAppointmentInfo.GetAt(i).dtStartDate) {
					dt = m_arAppointmentInfo.GetAt(i).dtStartDate;
				}
			}
			m_dtApplyDate.SetValue(dt);
			//disable so user cannot change start date
			m_dtApplyDate.EnableWindow(FALSE);
			m_btnApplyStartDate.EnableWindow(FALSE);
		}

		if(m_MedSchedID != -1) {
			m_bIsNew = FALSE;
			Load();
		}
		else{
			GetDlgItem(IDC_BTN_DELETE_SCHED)->EnableWindow(FALSE);
		}

	}
	NxCatchAll("Error in CMedSchedule::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMedSchedule::OnAddSchedDetail() 
{
	if(!Save(FALSE))
		return;

	CMedSchedDetail dlg(this);
	dlg.m_MedSchedID = m_MedSchedID;
	// (d.singleton 2012-04-13 16:47) - PLID 50436 get data for pre op calander
	if(m_bIsPreOpSchedule) {
		dlg.m_bIsPreOpSchedule = TRUE;
	}
	dlg.DoModal();
	
	m_DetailList->Requery();

	// (d.singleton 2012-05-16 11:10) - PLID 50436 change the start date if the new detail note would be before it
	if(m_bIsPreOpSchedule) {
		_RecordsetPtr prs = CreateParamRecordset("SELECT Type, DurationType, StartDay FROM MedSchedDetailsT WHERE MedSchedID = {INT}", m_MedSchedID);
		while(!prs->eof) {
			long nType = (long)AdoFldByte(prs, "Type");
			long nDurationType = (long)AdoFldByte(prs, "DurationType", -1);
			COleDateTimeSpan dts(AdoFldLong(prs, "StartDay"));
			COleDateTime dt = AsDateTime(m_dtApplyDate.GetValue());

			if(nType >= 4 && nDurationType == 1) {
				for(int i = 0; i < m_arAppointmentInfo.GetCount(); i++) {
					if((m_arAppointmentInfo.GetAt(i).dtStartDate - dts) < dt) {
						m_dtApplyDate.SetValue(dt - dts);
					}
				}
			}
			prs->MoveNext();
		}
	}
}

void CMedSchedule::OnEditSchedDetail() 
{
	if(m_DetailList->CurSel == -1) {
		AfxMessageBox("Please select an item to edit.");
		return;
	}

	CMedSchedDetail dlg(this);
	dlg.m_DetailID = m_DetailList->GetValue(m_DetailList->CurSel,0).lVal;
	dlg.m_MedSchedID = m_MedSchedID;
	// (d.singleton 2012-04-13 16:47) - PLID 50436 make sure we open the pre op calender details dlg
	if(m_bIsPreOpSchedule) {
		dlg.m_bIsPreOpSchedule = TRUE;
	}
	dlg.DoModal();
	
	m_DetailList->Requery();
}

void CMedSchedule::OnRemoveSchedDetail() 
{
	if(m_DetailList->CurSel == -1) {
		AfxMessageBox("Please select an item to delete.");
		return;
	}

	if(IDYES==MessageBox("This will permanently delete this detail! Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
		long DetailID = m_DetailList->GetValue(m_DetailList->CurSel,0).lVal;
		long Priority = m_DetailList->GetValue(m_DetailList->CurSel,DETAIL_COLUMN_PRIORITY).lVal;

		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("DeleteSchedDetail");
		trans.Begin();

		try {

			ExecuteSql("UPDATE MedSchedDetailsT SET Priority = Priority - 1 WHERE MedSchedID = %li AND Priority > %li",m_MedSchedID,Priority);				

			ExecuteSql("DELETE FROM MedSchedDetailsT WHERE ID = %li",DetailID);

			m_DetailList->Requery();

			trans.Commit();

			return;

		}NxCatchAll("Error deleting medication schedule detail.");
	}
}

void CMedSchedule::OnApplyOnDate() 
{
	if(((CButton*)GetDlgItem(IDC_APPLY_ON_DATE))->GetCheck()) {
		GetDlgItem(IDC_SCHED_START_DATE)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_SCHED_START_DATE)->EnableWindow(FALSE);
	}
}

void CMedSchedule::OnMoveDetailUp() 
{
	if(m_DetailList->CurSel == -1)
		return;

	long DetailID = m_DetailList->GetValue(m_DetailList->CurSel,0).lVal;
	long Priority = m_DetailList->GetValue(m_DetailList->CurSel,DETAIL_COLUMN_PRIORITY).lVal;
		
	if(Priority == 1)
		return;

	try
	{
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("MoveDetailUp");
		trans.Begin();

		//increment the prior detail
		ExecuteSql("UPDATE MedSchedDetailsT SET Priority = Priority + 1 WHERE MedSchedID = %li AND Priority = %li",
			m_MedSchedID, Priority - 1);

		Priority--;	//decrement the priority

		//move up this detail
		ExecuteSql("UPDATE MedSchedDetailsT SET Priority = %li WHERE ID = %li",
			Priority, DetailID);

		trans.Commit();

		m_DetailList->Requery();

		m_DetailList->FindByColumn(0,(long)DetailID,0,TRUE);

		return;
	}
	NxCatchAll("Could not move detail up.");
}

void CMedSchedule::OnMoveDetailDown() 
{
	if(m_DetailList->CurSel == -1)
		return;

	long DetailID = m_DetailList->GetValue(m_DetailList->CurSel,0).lVal;
	long Priority = m_DetailList->GetValue(m_DetailList->CurSel,DETAIL_COLUMN_PRIORITY).lVal;
		
	if(Priority == m_DetailList->GetRowCount())
		return;

	try
	{
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("MoveDetailDown");
		trans.Begin();

		//decrement the next detail
		ExecuteSql("UPDATE MedSchedDetailsT SET Priority = Priority - 1 WHERE MedSchedID = %li AND Priority = %li",
			m_MedSchedID, Priority + 1);

		Priority++;	//increment the priority

		//move this detail down
		ExecuteSql("UPDATE MedSchedDetailsT SET Priority = %li WHERE ID = %li",
			Priority, DetailID);

		trans.Commit();

		m_DetailList->Requery();

		m_DetailList->FindByColumn(0,(long)DetailID,0,TRUE);

		return;
	}
	NxCatchAll("Could not move detail down.");
}

BEGIN_EVENTSINK_MAP(CMedSchedule, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMedSchedule)
	ON_EVENT(CMedSchedule, IDC_MEDSCHEDDETAIL_LIST, 3 /* DblClickCell */, OnDblClickCellMedscheddetailList, VTS_I4 VTS_I2)
	ON_EVENT(CMedSchedule, IDC_MEDSCHEDDETAIL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedMedscheddetailList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMedSchedule::OnDblClickCellMedscheddetailList(long nRowIndex, short nColIndex) 
{
	OnEditSchedDetail();	
}

void CMedSchedule::Load()
{
	try {

		CString str;
		str.Format("MedSchedID = %li",m_MedSchedID);

		m_DetailList->PutWhereClause(_bstr_t(str));

		m_DetailList->Requery();

		_variant_t var;

		_RecordsetPtr rs = CreateRecordset("SELECT * FROM MedScheduleT WHERE ID = %li",m_MedSchedID);

		if(rs->eof) {
			return;
		}

		//Name		
		var = rs->Fields->Item["Name"]->Value;
		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);
			SetDlgItemText(IDC_MEDSCHED_NAME,str);
		}

		//Procedure
		var = rs->Fields->Item["ProcedureID"]->Value;
		if(var.vt == VT_I4) {
			// (c.haag 2009-01-08 12:03) - PLID 32539 - The procedure may be inactive, so we need
			// to know right away if it will appear. If not, we need to add it to the list.
			if (sriNoRow == m_ProcedureList->SetSelByColumn(0,var) && VarLong(var,-1) > 0) {
				_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", VarLong(var));
				if (!prs->eof) {
					IRowSettingsPtr pRow = m_ProcedureList->GetRow(-1);
					pRow->PutValue(0,var);
					pRow->PutValue(1,prs->Fields->Item["Name"]->Value);
					m_ProcedureList->AddRow(pRow);
					m_ProcedureList->Sort();
					m_ProcedureList->SetSelByColumn(0, var);
				} else {
					// It was deleted. Nothing we can do.
				}
			}
		}

		//Apply On Date Checkbox & DateTime
		var = rs->Fields->Item["StartDate"]->Value;
		if(var.vt == VT_DATE) {
			((CButton*)GetDlgItem(IDC_APPLY_ON_DATE))->SetCheck(TRUE);
			GetDlgItem(IDC_SCHED_START_DATE)->EnableWindow(TRUE);
			m_dtApplyDate.SetValue(var);
		}

		rs->Close();

		//change the start date if the new detail note would be before it
		if(m_bIsPreOpSchedule) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT Type, DurationType, StartDay FROM MedSchedDetailsT WHERE MedSchedID = {INT}", m_MedSchedID);
			while(!prs->eof) {
				long nType = (long)AdoFldByte(prs, "Type");
				long nDurationType = (long)AdoFldByte(prs, "DurationType", -1);
				COleDateTimeSpan dts(AdoFldLong(prs, "StartDay"));
				COleDateTime dt = AsDateTime(m_dtApplyDate.GetValue());

				if(nType >= 4 && nDurationType == 1) {
					for(int i = 0; i < m_arAppointmentInfo.GetCount(); i++) {
						if((m_arAppointmentInfo.GetAt(i).dtStartDate - dts) < dt) {
							COleDateTime dtNew = (dt - dts);
							m_dtApplyDate.SetValue(dtNew);
						}
					}
				}
				prs->MoveNext();
			}
			m_dtApplyDate.EnableWindow(FALSE);
		}

	}NxCatchAll("Error loading medication schedule.");

}

BOOL CMedSchedule::Save(BOOL bTestName /* = TRUE */)
{
	BOOL bIsNew = FALSE;

	if(m_MedSchedID == -1)
		bIsNew = TRUE;

	try {

		CString strName, strStartDate = "NULL", strProcedureID = "NULL";
		long ProcedureID;
		COleDateTime dtStartDate;

		//Name		
		GetDlgItemText(IDC_MEDSCHED_NAME,strName);

		strName.TrimRight();
		if(bTestName && strName == "") {
			AfxMessageBox("Please enter a name for this schedule.");
			return FALSE;
		}

		//Procedure
		if(m_ProcedureList->CurSel != -1) {
			ProcedureID = m_ProcedureList->GetValue(m_ProcedureList->CurSel,0);
			if(ProcedureID != -1) {
				strProcedureID.Format("%li",ProcedureID);
			}
		}

		//Apply On Date Checkbox
		if(((CButton*)GetDlgItem(IDC_APPLY_ON_DATE))->GetCheck()) {
			//Apply On Date DateTime
			_variant_t var;
			var = m_dtApplyDate.GetValue();
			dtStartDate = var.date;

			COleDateTime dtBad;
			dtBad.ParseDateTime("12/31/1800");
			if(dtStartDate.m_status == COleCurrency::invalid ||
				dtStartDate < dtBad) {
				AfxMessageBox("Please enter a valid start date.");
				return FALSE;
			}
			strStartDate = dtStartDate.Format("'%m/%d/%Y'");
		}

		if(bIsNew) {
			//add new record
			m_MedSchedID = NewNumber("MedScheduleT","ID");
			ExecuteSql("INSERT INTO MedScheduleT (ID, PatientID, ProcedureID, Name, InputDate, StartDate) "
				"VALUES (%li, %li, %s, '%s', GetDate(), %s)",m_MedSchedID, GetActivePatientID(), strProcedureID, _Q(strName), strStartDate);

			CString str;
			str.Format("MedSchedID = %li",m_MedSchedID);

			m_DetailList->PutWhereClause(_bstr_t(str));

			m_DetailList->Requery();
		}
		else {
			//update existing record
			ExecuteSql("UPDATE MedScheduleT SET ProcedureID = %s, Name = '%s', StartDate = %s WHERE ID = %li",
				strProcedureID, _Q(strName), strStartDate, m_MedSchedID);
		}

		return TRUE;
	
	} NxCatchAll("Error saving medication schedule.");

	return FALSE;
}

void CMedSchedule::DeleteSchedule(long ID)
{
	try {

		ExecuteSql("DELETE FROM MedSchedDetailsT WHERE MedSchedID = %li",ID);
		ExecuteSql("DELETE FROM MedScheduleT WHERE ID = %li",ID);

	} NxCatchAll("Error deleting medication schedule.");
}

void CMedSchedule::OnOK() 
{
	if(!Save())
		return;
	
	CDialog::OnOK();
}

void CMedSchedule::OnCancel() 
{
	if(m_bIsNew && m_MedSchedID != -1) {
		if(IDNO == MessageBox("You have made changes to this Medication Schedule. Are you sure you wish to cancel?",
			"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}
		else {
			DeleteSchedule(m_MedSchedID);
		}
	}

	CDialog::OnCancel();
}

void CMedSchedule::OnBtnDeleteSched() 
{
	if(m_MedSchedID == -1)
		return;

	if(IDNO == MessageBox("This will permanently delete this schedule! Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO))
		return;
	
	DeleteSchedule(m_MedSchedID);
	CDialog::OnCancel();
}

void CMedSchedule::OnRequeryFinishedMedscheddetailList(short nFlags) 
{
	CColor color;
	IRowSettingsPtr pRow;

	for(int i=0; i<m_DetailList->GetRowCount(); i++) {
		color = m_DetailList->GetValue(i,DETAIL_COLUMN_COLOR).lVal;
		pRow = m_DetailList->GetRow(i);
		pRow->PutCellForeColor(DETAIL_COLUMN_NAME,color);
	}
}

void CMedSchedule::OnPrintMedsched() 
{
	AfxMessageBox("This schedule will be saved and closed.");

	if(!Save())
		return;

	CDialog::OnCancel();

	m_bPrintOnClose = TRUE;	
}

void CMedSchedule::OnLoadFromTemplate() 
{
	long CurSel = m_TemplateList->GetCurSel();
	if(CurSel == -1) {
		AfxMessageBox("Please select a template from the list.");
		return;
	}

	//if we've added details, then this will be true, otherwise we've only
	//added a name, which is no big deal
	if(m_MedSchedID != -1 && IDNO == MessageBox("This will overwrite the current data. Are you sure you wish to load a template?","Practice",MB_YESNO|MB_ICONQUESTION))
		return;

	long TemplateID = m_TemplateList->GetValue(CurSel,0).lVal;

	try {
		_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID, Name FROM MedScheduleT WHERE ID = %li",TemplateID);
		
		if(!rs->eof) {

			if(m_MedSchedID == -1)
				if(!Save(FALSE))
					return;
			else {
				ExecuteSql("DELETE FROM MedSchedDetailsT WHERE MedSchedID = %li",m_MedSchedID);
			}

			CString name = "NULL", strProcID = "NULL";
			long ProcID;
			_variant_t var;

			var = rs->Fields->Item["Name"]->Value;
			if(var.vt == VT_BSTR) {
				name = CString(var.bstrVal);
				name = "'" + name + "'";
			}

			var = rs->Fields->Item["ProcedureID"]->Value;
			if(var.vt == VT_I4) {
				ProcID = var.lVal;
				strProcID.Format("%li",ProcID);
			}

			ExecuteSql("UPDATE MedScheduleT SET Name = '%s', ProcedureID = %s WHERE ID = %li",_Q(name), strProcID, m_MedSchedID);
		}
		rs->Close();

		//save new medscheddetails

		rs = CreateRecordset("SELECT ID FROM MedSchedDetailsT WHERE MedSchedID = %li",TemplateID);
		while(!rs->eof) {

			long TemplateDetailID = rs->Fields->Item["ID"]->Value.lVal;

			long NewDetailID;

			NewDetailID = NewNumber("MedSchedDetailsT","ID");

			ExecuteSql("INSERT INTO MedSchedDetailsT (ID, MedSchedID, Name, Color, Priority, Type, MedicationID, "
				"EventID, StartDay, StartNote, MiddleNote, DurationType, StopDay, StopNote, StopMethod) "
				"SELECT %li, %li, Name, Color, Priority, Type, MedicationID, EventID, StartDay, StartNote, "
				"MiddleNote, DurationType, StopDay, StopNote, StopMethod FROM MedSchedDetailsT WHERE ID = %li",NewDetailID,m_MedSchedID,TemplateDetailID);
			
			rs->MoveNext();
		}
		rs->Close();

		Load();

	}NxCatchAll("Error loading template.");
}

void CMedSchedule::OnSaveToTemplate() 
{
	long CurSel = m_TemplateList->GetCurSel();
	if(CurSel == -1) {
		AfxMessageBox("Please select a template from the list.");
		return;
	}

	if(IDNO == MessageBox("This will overwrite the currently selected template. Are you sure you wish to save as a template?","Practice",MB_YESNO|MB_ICONQUESTION))
		return;

	long TemplateID = m_TemplateList->GetValue(CurSel,0).lVal;
	CString strName = CString(m_TemplateList->GetValue(CurSel,1).bstrVal);
	if(TemplateID != -1) {
		DeleteSchedule(TemplateID);
		SaveTemplate(strName);
	}
}

void CMedSchedule::OnSaveNewTemplate() 
{
	CString name;
	CGetNewIDName dlg(this);
	dlg.m_pNewName = &name;
	dlg.m_nMaxLength = 255;
	dlg.m_strCaption = "Enter a new template name";

	if (IDOK != dlg.DoModal())
		return;

	name.TrimRight();

	if(name == "") {
		AfxMessageBox("Please enter a template name.");
		return;
	}

	SaveTemplate(name);
}

void CMedSchedule::OnDeleteTemplate() 
{
	long CurSel = m_TemplateList->GetCurSel();
	if(CurSel == -1) {
		AfxMessageBox("Please select a template from the list.");
		return;
	}
	
	if(IDNO == MessageBox("This will delete the currently selected template. Are you sure you wish to delete this template?","Practice",MB_YESNO|MB_ICONQUESTION))
		return;

	long TemplateID = m_TemplateList->GetValue(CurSel,0).lVal;

	if(TemplateID != -1) {
		DeleteSchedule(TemplateID);
	}

	m_TemplateList->Requery();
}

void CMedSchedule::SaveTemplate(CString strName)
{
	try {
		if(m_MedSchedID == -1)
			if(!Save())
				return;

		CString strStartDate = "NULL", strProcedureID = "NULL";
		long ProcedureID;
		COleDateTime dtStartDate;

		//Procedure
		if(m_ProcedureList->CurSel != -1) {
			ProcedureID = m_ProcedureList->GetValue(m_ProcedureList->CurSel,0);
			if(ProcedureID != -1) {
				strProcedureID.Format("%li",ProcedureID);
			}
		}

		//Apply On Date Checkbox
		if(((CButton*)GetDlgItem(IDC_APPLY_ON_DATE))->GetCheck()) {
			//Apply On Date DateTime
			_variant_t var;
			var = m_dtApplyDate.GetValue();
			dtStartDate = var.date;
			strStartDate = "'" + FormatDateTimeForSql(dtStartDate, dtoDate) + "'";
		}

		//add new record
		long TemplateID = NewNumber("MedScheduleT","ID");
		ExecuteSql("INSERT INTO MedScheduleT (ID, PatientID, ProcedureID, Name, InputDate) "
			"VALUES (%li, -25, %s, '%s', GetDate())",TemplateID, strProcedureID, _Q(strName));
		
		//save new medscheddetails
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM MedSchedDetailsT WHERE MedSchedID = %li",m_MedSchedID);
		while(!rs->eof) {

			long DetailID = rs->Fields->Item["ID"]->Value.lVal;

			long NewDetailID;

			NewDetailID = NewNumber("MedSchedDetailsT","ID");

			ExecuteSql("INSERT INTO MedSchedDetailsT (ID, MedSchedID, Name, Color, Priority, Type, MedicationID, "
				"EventID, StartDay, StartNote, MiddleNote, DurationType, StopDay, StopNote, StopMethod) "
				"SELECT %li, %li, Name, Color, Priority, Type, MedicationID, EventID, StartDay, StartNote, "
				"MiddleNote, DurationType, StopDay, StopNote, StopMethod FROM MedSchedDetailsT WHERE ID = %li",NewDetailID,TemplateID,DetailID);
			
			rs->MoveNext();
		}
		rs->Close();

		m_TemplateList->Requery();

	}NxCatchAll("Error saving template.");
}

void CMedSchedule::CalculateApptDays(ATL::COleDateTime dt)
{
	


}