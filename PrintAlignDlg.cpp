// PrintAlignDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PrintAlignDlg.h"
#include "GetNewIDName.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

#define COLUMN_ID	0
#define COLUMN_FORMID	1
#define COLUMN_DEFAULT	2
#define COLUMN_NAME	3

/////////////////////////////////////////////////////////////////////////////
// CPrintAlignDlg dialog


CPrintAlignDlg::CPrintAlignDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPrintAlignDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrintAlignDlg)
	m_FormID = 0;
	m_nCurrentFormAlignID = -1;
	//}}AFX_DATA_INIT
}


void CPrintAlignDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrintAlignDlg)
	// (j.jones 2007-04-25 14:16) - PLID 4758 - added ability to set a default per workstation
	DDX_Control(pDX, IDC_CHECK_DEFAULT_FOR_ME, m_CheckDefaultForMe);
	DDX_Control(pDX, IDC_EDIT_YSCALE, m_EditYScale);
	DDX_Control(pDX, IDC_EDIT_XSCALE, m_EditXScale);
	DDX_Control(pDX, IDC_DOWN, m_down);
	DDX_Control(pDX, IDC_UP, m_up);
	DDX_Control(pDX, IDC_RIGHT, m_right);
	DDX_Control(pDX, IDC_LEFT, m_left);
	DDX_Control(pDX, IDC_EDIT_MINI, m_EditMini);
	DDX_Control(pDX, IDC_CHECK_DEFAULT, m_CheckDefault);
	DDX_Control(pDX, IDC_EDIT_YSHIFT, m_EditYShift);
	DDX_Control(pDX, IDC_EDIT_XSHIFT, m_EditXShift);
	DDX_Control(pDX, IDC_EDIT_FONT, m_EditFont);
	DDX_Control(pDX, IDC_MINI_FONT_LABEL, m_nxstaticMiniFontLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_ADVANCED_SETTINGS_GROUPBOX, m_btnAdvancedSettingsGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrintAlignDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPrintAlignDlg)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT, OnCheckDefault)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_FOR_ME, OnCheckDefaultForMe)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrintAlignDlg message handlers

BOOL CPrintAlignDlg::OnInitDialog() 
{
	try {
		m_ComboPrinters = BindNxDataListCtrl(this,IDC_COMBO_PRINTERS,GetRemoteData(),FALSE);

		CString str;

		// (c.haag 2008-05-01 17:53) - PLID 29876 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		CNxDialog::OnInitDialog();

		if(m_FormID != 0) {
			GetDlgItem(IDC_MINI_FONT_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_MINI)->ShowWindow(SW_HIDE);
		}

		m_nCurrentFormAlignID = -1;
		str.Format("FormID = %d", m_FormID);
		m_ComboPrinters->PutWhereClause(_bstr_t(str));
		m_ComboPrinters->Requery();

		m_left.AutoSet(NXB_LEFT);
		m_right.AutoSet(NXB_RIGHT);
		m_down.AutoSet(NXB_DOWN);
		m_up.AutoSet(NXB_UP);

	}
	NxCatchAll("Error in CPrintAlignDlg::OnInitDialog");
		
	return TRUE;  
}

BEGIN_EVENTSINK_MAP(CPrintAlignDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPrintAlignDlg)
	ON_EVENT(CPrintAlignDlg, IDC_COMBO_PRINTERS, 16 /* SelChosen */, OnSelChosenComboPrinters, VTS_I4)
	ON_EVENT(CPrintAlignDlg, IDC_COMBO_PRINTERS, 18 /* RequeryFinished */, OnRequeryFinishedComboPrinters, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPrintAlignDlg::UpdateControls()
{
	CString str, strXOff, strYOff, strFont, strMini, strDefault, strXScale, strYScale;
	_variant_t var;
	int pos;

	pos = m_ComboPrinters->GetCurSel();

	GetDlgItem(IDC_EDIT_XSHIFT)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_EDIT_YSHIFT)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_EDIT_XSCALE)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_EDIT_YSCALE)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_UP)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_RIGHT)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_DOWN)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_LEFT)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_DELETE)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_CHECK_DEFAULT)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_EDIT_FONT)->EnableWindow(pos!=-1);
	GetDlgItem(IDC_EDIT_MINI)->EnableWindow(pos!=-1);

	if(pos==-1) {		
		return;
	}

	m_nCurrentFormAlignID = VarLong(m_ComboPrinters->GetValue(pos,COLUMN_ID));

	try {

		//ID|PrinterName|XOffset|YOffset|FontSize|Default
		_RecordsetPtr rs = CreateRecordset("SELECT * FROM FormAlignT WHERE ID = %li",m_nCurrentFormAlignID);
		if(!rs->eof) {
			var = rs->Fields->Item["XOffset"]->Value;
			str.Format("%d", var.lVal);
			m_EditXShift.SetWindowText(str);
			var = rs->Fields->Item["YOffset"]->Value;
			str.Format("%d", var.lVal);
			m_EditYShift.SetWindowText(str);
			var = rs->Fields->Item["FontSize"]->Value;
			str.Format("%d", var.lVal);
			m_EditFont.SetWindowText(str);
			var = rs->Fields->Item["MiniFontSize"]->Value;
			str.Format("%d", var.lVal);
			m_EditMini.SetWindowText(str);
			var = rs->Fields->Item["XScale"]->Value;
			str.Format("%d", var.lVal);
			m_EditXScale.SetWindowText(str);
			var = rs->Fields->Item["YScale"]->Value;
			str.Format("%d", var.lVal);
			m_EditYScale.SetWindowText(str);

			var = rs->Fields->Item["Default"]->Value;;
			if (var.boolVal != 0)
				m_CheckDefault.SetCheck(TRUE);
			else
				m_CheckDefault.SetCheck(FALSE);

			// (j.jones 2007-04-25 14:35) - PLID 4758 - load the workstation's default
			long nDefault = GetPropertyInt("DefaultFormAlignID", -1, m_FormID, FALSE);
			long nID = AdoFldLong(rs, "ID");
			if(nDefault != -1) {
				//if there is a default, set the check if we're on that default
				m_CheckDefaultForMe.SetCheck(nDefault == nID);
			}
			else {
				//if there is no default, set the check if we're on the system default
				m_CheckDefaultForMe.SetCheck(m_CheckDefault.GetCheck());
			}
		}
		rs->Close();
	}NxCatchAll("Error in Updating controls.");
}

void CPrintAlignDlg::OnOK()
{
	if(m_ComboPrinters->CurSel == -1) {
		CDialog::OnOK();
		return;
	}

	if(Save())
		CDialog::OnOK();
}

void CPrintAlignDlg::OnSelChosenComboPrinters(long nRow) 
{
	if(nRow == -1) {
		m_ComboPrinters->SetSelByColumn(COLUMN_ID, m_nCurrentFormAlignID);
	}
	else {

		long nNewID = VarLong(m_ComboPrinters->GetValue(nRow, COLUMN_ID));
		if(m_nCurrentFormAlignID == nNewID)
			return;

		if(IDNO == MessageBox("Your changes to the current printer will be saved. Are you sure you wish to save these changes?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
			return;
		}
	}

	if(!Save()) {
		m_ComboPrinters->SetSelByColumn(COLUMN_ID, m_nCurrentFormAlignID);
	}

	UpdateControls();
}

void CPrintAlignDlg::OnRequeryFinishedComboPrinters(short nFlags) 
{
	// (j.jones 2007-04-25 14:28) - PLID 4758 - auto-select the workstation's default
	long nDefault = GetPropertyInt("DefaultFormAlignID", -1, m_FormID, FALSE);
	if(nDefault != -1)
		m_ComboPrinters->SetSelByColumn(COLUMN_ID, nDefault);

	//if the cursel is still -1, set the system default
	if(m_ComboPrinters->GetCurSel() == -1)
		m_ComboPrinters->SetSelByColumn(COLUMN_DEFAULT, (bool)1);

	//if the cursel is still -1, just use the first row
	if(m_ComboPrinters->GetCurSel() == -1)
		m_ComboPrinters->PutCurSel(0);

	UpdateControls();
}

void CPrintAlignDlg::OnAdd() 
{
	if(m_ComboPrinters->CurSel != -1) {
		if(IDNO == MessageBox("Your changes to the current printer will be saved. Are you sure you wish to save these changes?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
			return;
		}
		if(!Save())
			return;
	}

	CGetNewIDName dlg(this);
	CString newname = "New Printer";
	dlg.m_pNewName = &newname;
	dlg.m_nMaxLength = 50;
	
	//DRT - 10/5/01 - We should probably not keep going if they said to cancel
	if(dlg.DoModal() == IDCANCEL)
		return;

	BOOL bFirst = FALSE;
	long nNewID = -1;

	try {
		if(IsRecordsetEmpty("SELECT ID FROM FormAlignT WHERE FormID = %li",m_FormID))
			bFirst = TRUE;
		nNewID = NewNumber("FormAlignT","ID");
		ExecuteSql("INSERT INTO FormAlignT (ID, FormID, [Default], PrinterName, XOffset, YOffset, XScale, YScale, FontSize, MiniFontSize) VALUES (%li,%li,%li,'%s',0,0,0,0,%li,10)",nNewID,m_FormID,(bFirst ? 1:0),_Q(newname),m_FormID == 1 ? 10 : 12);
	}NxCatchAll("Error in OnAdd()");

	//add to the list
	NXDATALISTLib::IRowSettingsPtr pRow = m_ComboPrinters->GetRow(-1);
	pRow->PutValue(COLUMN_ID, nNewID);
	pRow->PutValue(COLUMN_FORMID, (long)m_FormID);
	_variant_t varFalse;
	varFalse.vt = VT_BOOL;
	varFalse.boolVal = false;
	pRow->PutValue(COLUMN_DEFAULT, varFalse);
	pRow->PutValue(COLUMN_NAME, _bstr_t(newname));
	m_ComboPrinters->AddRow(pRow);
	m_ComboPrinters->SetSelByColumn(COLUMN_ID, nNewID);
	UpdateControls();
	
	if(bFirst)
		m_CheckDefault.SetCheck(TRUE);
}

void CPrintAlignDlg::OnDelete() 
{
	int iRow = m_ComboPrinters->GetCurSel();
	if(iRow==-1)
		return;
	CString str;
	str.Format("DELETE FROM FormAlignT WHERE ID = %d", VarLong(m_ComboPrinters->GetValue(iRow,COLUMN_ID)));
	ExecuteSql(str);

	// (j.jones 2007-04-25 11:44) - PLID 4758 - If this was the default per workstation,
	// remove that setting. Continue with the pre-existing logic of not assigning a new
	// system default.

	//clear out our workstation default (granted, they may have checked this printer,
	//not saved, and deleted, but in that case they had intended to make this the default,
	//and we're removing the printer, so despite it not being saved we would still clear
	//it out)
	if(m_CheckDefaultForMe.GetCheck()) {
		SetPropertyInt("DefaultFormAlignID", -1, m_FormID);
	}

	m_ComboPrinters->Requery();
}

BOOL CPrintAlignDlg::Save()
{
	try {

		if(m_nCurrentFormAlignID == -1)
			return FALSE;

		long nXOff, nYOff, nFont, nMini, nDefault, nXScale, nYScale;

		nXOff = GetDlgItemInt(IDC_EDIT_XSHIFT);
		nYOff = GetDlgItemInt(IDC_EDIT_YSHIFT);
		nFont = GetDlgItemInt(IDC_EDIT_FONT);
		nMini = GetDlgItemInt(IDC_EDIT_MINI);
		nXScale = GetDlgItemInt(IDC_EDIT_XSCALE);
		nYScale = GetDlgItemInt(IDC_EDIT_YSCALE);

		//if UB92, upper limit is 12
		if(m_FormID == 1 && nFont > 12) {
			//TES 3/13/2007 - PLID 24993 - Changed from "UB92" to "UB"
			AfxMessageBox("The 'Font Size' for a UB form cannot be greater than 12. Please reduce your font size.");
			return FALSE;
		}

		//any other form, limit is 14
		if(m_FormID != 1 && nFont > 14) {
			AfxMessageBox("The 'Font Size' cannot be greater than 14. Please reduce your font size.");
			return FALSE;
		}

		//only HCFA has the Mini-Font
		if(m_FormID == 0 && nMini > 12) {
			AfxMessageBox("The 'Box 31-33 Font Size' for a HCFA form cannot be greater than 12. Please reduce this font size.");
			return FALSE;
		}

		if (m_CheckDefault.GetCheck()) {
			nDefault = 1;
			//make sure no other entries are default for this form
			ExecuteSql("UPDATE FormAlignT SET [Default] = 0 WHERE FormID = %li",m_FormID);
		}
		else
			nDefault = 0;

		ExecuteSql("UPDATE FormAlignT SET XOffset = %li, YOffset = %li, FontSize = %li, MiniFontSize = %li, "
			"[Default] = %li, XScale = %li, YScale = %li WHERE ID = %li",
			nXOff, nYOff, nFont, nMini, nDefault, nXScale, nYScale, m_nCurrentFormAlignID);

		// (j.jones 2007-04-25 14:39) - PLID 4758 - store the default for this workstation, if checked
		if(m_CheckDefaultForMe.GetCheck()) {
			SetPropertyInt("DefaultFormAlignID", m_nCurrentFormAlignID, m_FormID);
		}
		else {
			//was this the default before? If so, set to -1
			long nDefault = GetPropertyInt("DefaultFormAlignID", -1, m_FormID, FALSE);
			if(nDefault == m_nCurrentFormAlignID) {
				//we unchecked our default, so clear it out (we'll end up using the system default)
				SetPropertyInt("DefaultFormAlignID", -1, m_FormID);
			}
		}

		return TRUE;

	}NxCatchAll("Error saving printer settings.");

	return FALSE;
}

void CPrintAlignDlg::OnUp() 
{
	CString str;
	int yShift;
	m_EditYShift.GetWindowText(str);
	yShift = atoi(str);
	yShift++;
	str.Format("%d", yShift);
	m_EditYShift.SetWindowText(str);	
}

void CPrintAlignDlg::OnRight() 
{
	CString str;
	int xShift;
	m_EditXShift.GetWindowText(str);
	xShift = atoi(str);
	xShift++;
	str.Format("%d", xShift);
	m_EditXShift.SetWindowText(str);	
}

void CPrintAlignDlg::OnDown() 
{
	CString str;
	int yShift;
	m_EditYShift.GetWindowText(str);
	yShift = atoi(str);
	yShift--;
	str.Format("%d", yShift);
	m_EditYShift.SetWindowText(str);	
}

void CPrintAlignDlg::OnLeft() 
{
	CString str;
	int xShift;
	m_EditXShift.GetWindowText(str);
	xShift = atoi(str);
	xShift--;
	str.Format("%d", xShift);
	m_EditXShift.SetWindowText(str);	
}

void CPrintAlignDlg::OnCancel() 
{
	if(m_ComboPrinters->CurSel != -1) {
		if(IDNO == MessageBox("Are you sure you wish to cancel without saving your changes?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
			return;
		}
	}
	
	CDialog::OnCancel();
}

void CPrintAlignDlg::OnCheckDefault() 
{
	// (j.jones 2007-04-25 14:16) - PLID 4758 - added a don't show warning
	if (m_CheckDefault.GetCheck()) {
		DontShowMeAgain(this, "Setting this alignment as the 'System Default' will ensure these are the default settings "
					    "for printing the current claim type on a computer that has not checked the "
						"'Default For My Workstation' option in the past. This will not change any other computers' "
						"default printer.","PrintAlignDlgOnCheckDefault", "Practice");
	}
}

// (j.jones 2007-04-25 14:16) - PLID 4758 - added ability to set a default per workstation
void CPrintAlignDlg::OnCheckDefaultForMe() 
{
	if (m_CheckDefaultForMe.GetCheck()) {
		DontShowMeAgain(this, "Setting this alignment as the 'Default For My Workstation' will ensure that your station "
					    "will always use these settings for the current printer for the current claim type.\n"
						"This will not change any other computers' default printer.","PrintAlignDlgOnCheckDefaultForMe", "Practice");
	}	
}
