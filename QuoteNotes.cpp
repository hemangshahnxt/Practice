// QuoteNotes.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "QuoteNotes.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "QuoteAdminDlg.h"
#include "EditComboBox.h"
#include "GlobalDrawingUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CQuoteNotes dialog


CQuoteNotes::CQuoteNotes(CWnd* pParent /*=NULL*/)
	: CNxDialog(CQuoteNotes::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuoteNotes)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CQuoteNotes::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuoteNotes)
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDC_ADMINISTRATOR, m_adminButton);
	DDX_Control(pDX, IDC_SAVE, m_saveButton);
	DDX_Control(pDX, IDC_EDIT_SPECIFIC, m_editNotes);
	DDX_Control(pDX, IDC_LABEL142, m_nxstaticLabel142);
	DDX_Control(pDX, IDC_LABEL, m_nxstaticLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuoteNotes, CNxDialog)
	ON_BN_CLICKED(IDC_ADMINISTRATOR, OnAdministrator)
	ON_BN_CLICKED(IDC_EDIT_QUOTE_NOTES, OnEditQuoteNotes)	
	ON_BN_CLICKED(IDC_SAVE, OnSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuoteNotes message handlers

BEGIN_EVENTSINK_MAP(CQuoteNotes, CNxDialog)
	ON_EVENT(CQuoteNotes, IDC_QUOTE_NOTES_COMBO, 16 /* SelChosen */, OnSelChosenQuoteNotesCombo, VTS_I4)
END_EVENTSINK_MAP()

BOOL CQuoteNotes::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_saveButton.AutoSet(NXB_OK);
	m_adminButton.AutoSet(NXB_MODIFY);
	m_cancelButton.AutoSet(NXB_CANCEL);

	m_editNotes.SetLimitText(255);

	m_NoteCombo = BindNxDataListCtrl(this,IDC_QUOTE_NOTES_COMBO,GetRemoteData(),TRUE);

	m_strSpecificNotes = m_strSpecificNotes.Left(255);
	SetDlgItemText (IDC_EDIT_SPECIFIC, m_strSpecificNotes);

	return TRUE;
}

void CQuoteNotes::OnAdministrator() 
{
	try {
		CQuoteAdminDlg  dlg(this);
		
		//make sure they have access to the administrator tab since these changes will affect all quote
		
		if (!UserPermission(AdministratorModuleItem))
			return;
		else {
			dlg.m_nColor = RGB(255,179,128);
			dlg.DoModal();
		}
	}NxCatchAll(__FUNCTION__);
}

void CQuoteNotes::OnSelChosenQuoteNotesCombo(long nRow) 
{
	//DRT 9/24/2004 - PLID 14211 - Added try / catch, checked for no selection
	try {
		CString str;
		GetDlgItem(IDC_EDIT_SPECIFIC)->GetWindowText(m_strSpecificNotes);
		if(m_strSpecificNotes!="" &&
			IDNO == MessageBox("This will overwrite the current text. Are you sure?","Practice",MB_YESNO|MB_ICONINFORMATION)) {
			return;
		}
		if(nRow > -1) 
			m_strSpecificNotes = CString(m_NoteCombo->GetValue(nRow,0).bstrVal);
		else
			m_strSpecificNotes = "";
		m_NoteCombo->PutComboBoxText("");
		m_strSpecificNotes = m_strSpecificNotes.Left(255);
		SetDlgItemText(IDC_EDIT_SPECIFIC,m_strSpecificNotes);
	} NxCatchAll("Error in OnSelChosenQuoteNotesCombo");
}

void CQuoteNotes::OnEditQuoteNotes() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 1, m_NoteCombo, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CQuoteNotes::OnSave() 
{
	CString str;

	try {

		GetDlgItemText(IDC_EDIT_SPECIFIC,m_strSpecificNotes);

		if(m_strSpecificNotes.GetLength()>255) {
			AfxMessageBox("The text you entered is too long. Your note has been truncated to 255 characters.");
			m_strSpecificNotes = m_strSpecificNotes.Left(255);
			SetDlgItemText(IDC_EDIT_SPECIFIC,m_strSpecificNotes);
		}

		// Find out of this note already exists in QuoteNotesT
		_RecordsetPtr rs = CreateRecordset("SELECT Note FROM QuoteNotesT WHERE Note = '%s'", _Q(m_strSpecificNotes));

		if(m_strSpecificNotes!="" && rs->eof && (!(m_NoteCombo->GetCurSel() != -1 &&			 
		  (CString(m_NoteCombo->GetValue(m_NoteCombo->GetCurSel(),0).bstrVal)==m_strSpecificNotes)))) {
			if(IDYES == MessageBox("Would you like to add this note for future use?","Practice",MB_YESNO|MB_ICONINFORMATION)) {
				str.Format("INSERT INTO QuoteNotesT (Note) VALUES ('%s')",_Q(m_strSpecificNotes));
				ExecuteSql("%s",str);
			}
		}
	}NxCatchAll("Error saving note.");

	CDialog::OnOK();	
}
