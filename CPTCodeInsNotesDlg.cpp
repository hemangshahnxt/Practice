// CPTCodeInsNotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CPTCodeInsNotesDlg.h"
#include "InsCodeNotesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCPTCodeInsNotesDlg dialog

using namespace ADODB;

CCPTCodeInsNotesDlg::CCPTCodeInsNotesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCPTCodeInsNotesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCPTCodeInsNotesDlg)
		m_nInsCoID = -1;
		m_nCPTCodeID = -1;
		m_nDiagCodeID = -1;
	//}}AFX_DATA_INIT
}


void CCPTCodeInsNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCPTCodeInsNotesDlg)
	DDX_Control(pDX, IDC_SERVICE_CODE_NOTE, m_btnSvcCode);
	DDX_Control(pDX, IDC_DIAG_CODE_NOTE, m_btnDiagCode);
	DDX_Control(pDX, IDC_EDIT_CPT_NOTES, m_nxeditEditCptNotes);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCPTCodeInsNotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCPTCodeInsNotesDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_CPT_NOTES, OnKillfocusEditCptNotes)
	ON_BN_CLICKED(IDC_SERVICE_CODE_NOTE, OnServiceCodeNote)
	ON_BN_CLICKED(IDC_DIAG_CODE_NOTE, OnDiagCodeNote)
	ON_BN_CLICKED(IDC_ADVANCED_CODE_NOTES, OnAdvancedCodeNotes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCPTCodeInsNotesDlg message handlers

BOOL CCPTCodeInsNotesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29852 - Set button styles
	m_btnClose.AutoSet(NXB_CLOSE);

	m_InsCoList = BindNxDataListCtrl(this,IDC_INSCOS_LIST,GetRemoteData(),true);
	m_CPTList = BindNxDataListCtrl(this,IDC_CPT_LIST,GetRemoteData(),true);
	m_pDiagList = BindNxDataListCtrl(this, IDC_DIAG_LIST, GetRemoteData(), true);
	
	if(m_nInsCoID != -1)
		m_InsCoList->SetSelByColumn(0,m_nInsCoID);

	if(m_nCPTCodeID != -1)
		m_CPTList->SetSelByColumn(0,m_nCPTCodeID);

	((CNxEdit*)GetDlgItem(IDC_EDIT_CPT_NOTES))->LimitText(4000);

	CheckRadioButton(IDC_SERVICE_CODE_NOTE, IDC_DIAG_CODE_NOTE, IDC_SERVICE_CODE_NOTE);
	OnServiceCodeNote();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCPTCodeInsNotesDlg::OnKillfocusEditCptNotes() 
{
	Save();
}

BEGIN_EVENTSINK_MAP(CCPTCodeInsNotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCPTCodeInsNotesDlg)
	ON_EVENT(CCPTCodeInsNotesDlg, IDC_CPT_LIST, 16 /* SelChosen */, OnSelChosenCptList, VTS_I4)
	ON_EVENT(CCPTCodeInsNotesDlg, IDC_INSCOS_LIST, 16 /* SelChosen */, OnSelChosenInscosList, VTS_I4)
	ON_EVENT(CCPTCodeInsNotesDlg, IDC_DIAG_LIST, 16 /* SelChosen */, OnSelChosenDiagList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCPTCodeInsNotesDlg::OnSelChosenCptList(long nRow) 
{
	try {

		Save();

		if(nRow == -1)
			return;

		m_nCPTCodeID = VarLong(m_CPTList->GetValue(m_CPTList->GetCurSel(),0));

		Load();

	}NxCatchAll("Error loading service code data.");
}

void CCPTCodeInsNotesDlg::OnSelChosenInscosList(long nRow) 
{
	try {

		Save();

		if(nRow == -1)
			return;

		m_nInsCoID = VarLong(m_InsCoList->GetValue(m_InsCoList->GetCurSel(),0));

		Load();

	}NxCatchAll("Error loading insurance company data.");
}

void CCPTCodeInsNotesDlg::Load()
{
	try {
		if(IsDlgButtonChecked(IDC_SERVICE_CODE_NOTE)) {

			if(m_nInsCoID == -1 || m_nCPTCodeID == -1) {
				SetDlgItemText(IDC_EDIT_CPT_NOTES,"");
				return;
			}

			_RecordsetPtr rs = CreateRecordset("SELECT Notes FROM CPTInsNotesT WHERE CPTCodeID = %li AND InsuranceCoID = %li",m_nCPTCodeID,m_nInsCoID);
			if(!rs->eof) {
				CString strNotes = AdoFldString(rs, "Notes","");
				SetDlgItemText(IDC_EDIT_CPT_NOTES,strNotes);
			}
			else {
				SetDlgItemText(IDC_EDIT_CPT_NOTES,"");
			}
			rs->Close();
		}
		else {
			if(m_nInsCoID == -1 || m_nDiagCodeID == -1) {
				SetDlgItemText(IDC_EDIT_CPT_NOTES,"");
				return;
			}

			_RecordsetPtr rs = CreateRecordset("SELECT Notes FROM DiagInsNotesT WHERE DiagCodeID = %li AND InsuranceCoID = %li", m_nDiagCodeID, m_nInsCoID);
			if(!rs->eof) {
				CString strNotes = AdoFldString(rs, "Notes","");
				SetDlgItemText(IDC_EDIT_CPT_NOTES, strNotes);
			}
			else {
				SetDlgItemText(IDC_EDIT_CPT_NOTES,"");
			}
			rs->Close();
		}

	}NxCatchAll("Error loading service code note.");
}

void CCPTCodeInsNotesDlg::Save()
{
	try {

		if(IsDlgButtonChecked(IDC_SERVICE_CODE_NOTE)) {
			
			if(m_nInsCoID == -1 || m_nCPTCodeID == -1) {
				return;
			}

			CString strNotes;
			GetDlgItemText(IDC_EDIT_CPT_NOTES,strNotes);

			if(IsRecordsetEmpty("SELECT Notes FROM CPTInsNotesT WHERE CPTCodeID = %li AND InsuranceCoID = %li",m_nCPTCodeID,m_nInsCoID)) {
				ExecuteSql("INSERT INTO CPTInsNotesT (CPTCodeID, InsuranceCoID, Notes) VALUES (%li, %li, '%s')",m_nCPTCodeID,m_nInsCoID,_Q(strNotes));
			}
			else {
				ExecuteSql("UPDATE CPTInsNotesT SET Notes = '%s' WHERE CPTCodeID = %li AND InsuranceCoID = %li",_Q(strNotes),m_nCPTCodeID,m_nInsCoID);
			}
		}
		else {
			if(m_nInsCoID == -1 || m_nDiagCodeID == -1) {
				return;
			}

			CString strNotes;
			GetDlgItemText(IDC_EDIT_CPT_NOTES,strNotes);

			if(IsRecordsetEmpty("SELECT Notes FROM DiagInsNotesT WHERE DiagCodeID = %li AND InsuranceCoID = %li", m_nDiagCodeID, m_nInsCoID)) {
				ExecuteSql("INSERT INTO DiagInsNotesT (DiagCodeId, InsuranceCoID, Notes) VALUES (%li, %li, '%s')", m_nDiagCodeID, m_nInsCoID,_Q(strNotes));
			}
			else {
				ExecuteSql("UPDATE DiagInsNotesT SET Notes = '%s' WHERE DiagCodeID = %li AND InsuranceCoID = %li",_Q(strNotes), m_nDiagCodeID, m_nInsCoID);
			}
		}

	}NxCatchAll("Error saving service code note.");
}

void CCPTCodeInsNotesDlg::OnServiceCodeNote() 
{
	GetDlgItem(IDC_DIAG_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CPT_LIST)->ShowWindow(SW_SHOW);
	Load();
}

void CCPTCodeInsNotesDlg::OnDiagCodeNote() 
{
	GetDlgItem(IDC_CPT_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DIAG_LIST)->ShowWindow(SW_SHOW);
	Load();
}

void CCPTCodeInsNotesDlg::OnSelChosenDiagList(long nRow) 
{
	try {

		Save();

		if(nRow == -1)
			return;

		m_nDiagCodeID = VarLong(m_pDiagList->GetValue(m_pDiagList->GetCurSel(),0));

		Load();

	}NxCatchAll("Error loading diagnosis code data.");
}

void CCPTCodeInsNotesDlg::OnAdvancedCodeNotes() 
{
	try {
		// (c.haag 2009-08-25 13:25) - PLID 29129 - Save the current text content now.
		// The dialog about to open provides another avenue of modifying data, so we 
		// want to commit the current text first.
		Save();

		CInsCodeNotesDlg dlg(this);
		dlg.DoModal();

		// (c.haag 2009-08-25 13:26) - PLID 29129 - Now reload the text for the current
		// filter in case it was changed within the dialog.
		Load();
	}
	NxCatchAll(__FUNCTION__);
}
