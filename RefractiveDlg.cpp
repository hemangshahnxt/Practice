// RefractiveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "RefractiveDlg.h"
#include "VisitDlg.h"
#include "DateTimeUtils.h"

using namespace ADODB;

//temp for Tom
#include "EyeGraphDlg.h"
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CRefractiveDlg dialog


CRefractiveDlg::CRefractiveDlg(CWnd* pParent)
	: CPatientDialog(CRefractiveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRefractiveDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRefractiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRefractiveDlg)
	DDX_Control(pDX, IDC_EYE_BG, m_EyeBG);
	DDX_Control(pDX, IDC_NEW_RECORD, m_btnNewRecord);
	DDX_Control(pDX, IDC_DELETE_RECORD, m_btnDeleteRecord);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRefractiveDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRefractiveDlg)
	ON_BN_CLICKED(IDC_NEW_RECORD, OnNewRecord)
	ON_BN_CLICKED(IDC_EYE_GRAPH, OnEyeGraph)
	ON_BN_CLICKED(IDC_DELETE_RECORD, OnDeleteRecord)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRefractiveDlg message handlers

void CRefractiveDlg::SetColor(OLE_COLOR nNewColor)
{
	m_EyeBG.SetColor(nNewColor);
		
	CPatientDialog::SetColor(nNewColor);
}

void CRefractiveDlg::OnNewRecord() 
{
	long nNewID;

	try {

		//we're going to save the information as blank ahead of time, and just let it fill in from there on the dilog
		nNewID = NewNumber("EyeProceduresT", "ID");

		//get the patients age
		_RecordsetPtr rs = CreateRecordset("SELECT Birthdate FROM PersonT WHERE ID = %li", GetActivePatientID());
		COleDateTime dtBirthDate;	//patient bday
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		long nAge = -1;
		
		if(!rs->eof) {
			_variant_t var = rs->Fields->Item["BirthDate"]->Value;
			if(var.vt == VT_DATE) {
				dtBirthDate = var.date;
				// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
				//  validation should only be done when bdays are entered/changed
				// (z.manning 2010-01-13 16:36) - PLID 22672 - Age is now a string
				nAge = atol(GetPatientAgeOnDate(dtBirthDate, dtToday, FALSE));
			}
			else {
				if(MessageBox("This patient does not currently have a birthdate.  If you create a procedure without a birthdate, the patients age will be left blank.  Do you still wish to continue?", "Continue?", MB_YESNO) == IDNO) {
					return;
				}
			}
		}
		CString strAge;
		if(nAge == -1) 
			strAge = "NULL";
		else
			strAge.Format("%li", nAge);

		CString sql;
		sql.Format("INSERT INTO EyeProceduresT (ID, PatientID, ProcedureDate, ProcedureID, LocationID, Complaint, Age, Monovision, Contacts, ProviderID) "
			"(SELECT %li, %li, '%s', %li, %li, '', %s, 0, 0, MainPhysician FROM PatientsT WHERE PersonID = %li)", nNewID, GetActivePatientID(), FormatDateTimeForSql(dtToday, dtoDate), -1, GetCurrentLocationID(), strAge, GetActivePatientID());
		ExecuteSql("%s", sql);

	
		CVisitDlg dlg(TRUE, this);
		dlg.m_nCurrentID = nNewID;	//the dialog loads based off of the id

		int nReturn = dlg.DoModal();
		if(nReturn != IDOK) {
			//They cancelled, so ditch this new visit we made
			//m.hancock - 10/20/05 - PLID 16756 - Need to delete the tests associated with the visit
			_RecordsetPtr rs = CreateRecordset("SELECT ID FROM EyeVisitsT WHERE EyeProcedureID = %li", nNewID);
			while(!rs->eof)
			{
				long nVisitID = rs->Fields->Item["ID"]->Value.lVal;
				ExecuteSql("DELETE FROM EyeTestsT WHERE VisitID = %li", nVisitID);
				//Also need to delete the EyeVisitsListData
				ExecuteSql("DELETE FROM EyeVisitsListDataT WHERE VisitID = %li", nVisitID);
				rs->MoveNext();
			}

			//delete all the visits, then the procedure record
			ExecuteSql("DELETE FROM EyeVisitsT WHERE EyeProcedureID = %li", nNewID);

			//delete the OutcomesListData
			ExecuteSql("DELETE FROM OutcomesListDataT WHERE OutcomeID = %li", nNewID);

			//Finally, delete the procedure
			ExecuteSql("DELETE FROM EyeProceduresT WHERE ID = %li", nNewID);
			//done deleting
		}
		m_SurgeryCombo->Requery();
	} NxCatchAll ("Error creating new Eye Procedure");
	
}

BOOL CRefractiveDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnNewRecord.AutoSet(NXB_NEW);
	m_btnDeleteRecord.AutoSet(NXB_DELETE);
	
	m_SurgeryCombo = BindNxDataListCtrl(IDC_SURGERIES, false);

	UpdateView();

	EnableAppropriateButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRefractiveDlg::OnEyeGraph() 
{		//DEBUG button for Tom
	CEyeGraphDlg dlg(this);
	dlg.DoModal();
}

BEGIN_EVENTSINK_MAP(CRefractiveDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRefractiveDlg)
	ON_EVENT(CRefractiveDlg, IDC_SURGERIES, 3 /* DblClickCell */, OnDblClickCellSurgeries, VTS_I4 VTS_I2)
	ON_EVENT(CRefractiveDlg, IDC_SURGERIES, 2 /* SelChanged */, OnSelChangedSurgeries, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRefractiveDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
	CString strWhere;
	strWhere.Format("PatientID = %li", GetActivePatientID());
	
	m_SurgeryCombo->WhereClause = _bstr_t(strWhere);

	m_SurgeryCombo->Requery();

	EnableAppropriateButtons();
}

void CRefractiveDlg::OnDeleteRecord() 
{
	if(m_SurgeryCombo->GetCurSel() == -1)
		return;

	if( MessageBox("Are you sure you wish to delete this procedure?", "Delete?", MB_YESNO) == IDNO)
		return;

	long delID = VarLong(m_SurgeryCombo->GetValue(m_SurgeryCombo->GetCurSel(), 0));
	
	try {
		//m.hancock - 8/22/05 - PLID 16756 - Need to delete the tests associated with the visit first
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM EyeVisitsT WHERE EyeProcedureID = %li", delID);
		while(!rs->eof)
		{
			long nVisitID = rs->Fields->Item["ID"]->Value.lVal;
			ExecuteSql("DELETE FROM EyeTestsT WHERE VisitID = %li", nVisitID);
			//Also need to delete the EyeVisitsListData
			ExecuteSql("DELETE FROM EyeVisitsListDataT WHERE VisitID = %li", nVisitID);
			rs->MoveNext();
		}

		//delete all the visits, then the procedure record
		ExecuteSql("DELETE FROM EyeVisitsT WHERE EyeProcedureID = %li", delID);

		//delete the OutcomesListData
		ExecuteSql("DELETE FROM OutcomesListDataT WHERE OutcomeID = %li", delID);

		//Finally, delete the procedure
		ExecuteSql("DELETE FROM EyeProceduresT WHERE ID = %li", delID);
		//done deleting
	} NxCatchAll("Error deleting record in CRefractiveDlg");

	m_SurgeryCombo->Requery();

	EnableAppropriateButtons();
}

void CRefractiveDlg::OnDblClickCellSurgeries(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;
	
	//open the visits dialog with the id from the datalist
	CVisitDlg dlg(FALSE, this);

	dlg.m_nCurrentID = VarLong(m_SurgeryCombo->GetValue(nRowIndex, 0));

	if(dlg.DoModal() != 0)
		m_SurgeryCombo->Requery();
}

void CRefractiveDlg::EnableAppropriateButtons()
{
	if(m_SurgeryCombo->GetCurSel() == sriNoRow) {
		GetDlgItem(IDC_DELETE_RECORD)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_DELETE_RECORD)->EnableWindow(TRUE);
	}
}

void CRefractiveDlg::OnSelChangedSurgeries(long nNewSel) 
{
	EnableAppropriateButtons();	
}
