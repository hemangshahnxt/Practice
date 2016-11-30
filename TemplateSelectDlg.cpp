// TemplateNewLineItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TemplateSelectDlg.h"
#include "TemplateLineItemInfo.h"
#include "SchedulerRc.h"
#include "TemplateItemEntryGraphicalDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateSelectDlg dialog


// (z.manning 2014-12-05 13:41) - PLID 64219 - Added editor type param
CTemplateSelectDlg::CTemplateSelectDlg(ESchedulerTemplateEditorType eType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CTemplateSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTemplateSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nSelectedTemplateID = -1;
	m_nColor = 255;
	m_eEditorType = eType;
	m_bIsBlock = FALSE;
	m_dtStartTime.SetStatus(COleDateTime::invalid);
	m_dtEndTime.SetStatus(COleDateTime::invalid);
}


void CTemplateSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateSelectDlg)
	DDX_Control(pDX, IDC_NEW_TEMPLATE_LINE_ITEM, m_btnNewTemplate);
	DDX_Control(pDX, IDC_SELECT_EXISTING_TEMPLATE, m_btnSelectExisting);
	DDX_Control(pDX, IDC_COLOR_CHOOSER_CTRL, m_ctrlColorPicker);
	DDX_Control(pDX, IDC_NEW_TEMPLATE_NAME, m_nxeditNewTemplateName);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplateSelectDlg)
	ON_BN_CLICKED(IDC_SELECT_EXISTING_TEMPLATE, OnSelectExistingTemplate)
	ON_BN_CLICKED(IDC_NEW_TEMPLATE_LINE_ITEM, OnNewTemplate)
	ON_BN_CLICKED(IDC_NEW_TEMPLATE_CHOOSE_COLOR, OnNewTemplateChooseColor)
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateSelectDlg message handlers

BEGIN_EVENTSINK_MAP(CTemplateSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTemplateSelectDlg)
	ON_EVENT(CTemplateSelectDlg, IDC_TEMPLATE_DROPDOWN, 18 /* RequeryFinished */, OnRequeryFinishedSelectExistingTemplate, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CTemplateSelectDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (z.manning, 04/29/2008) - PLID 29814 - Set buttons styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//TES 6/19/2010 - PLID 5888 - Tell the list to pull from the appropriate table.
		m_pdlTemplates = BindNxDataList2Ctrl(this, IDC_TEMPLATE_DROPDOWN, GetRemoteData(), false);
		if (m_eEditorType == stetLocation) {
			m_pdlTemplates->FromClause = _bstr_t("ResourceAvailTemplateT");
		}
		else {
			m_pdlTemplates->FromClause = _bstr_t("TemplateT");
		}
		m_pdlTemplates->Requery();

		//TES 6/19/2010 - PLID 5888 - If we're on a resource availability template, bind the locations list,
		// otherwise, hide it
		if (m_eEditorType == stetLocation) {
			m_pLocationCombo = BindNxDataList2Ctrl(IDC_SELECT_TEMPLATE_LOCATION_COMBO);
		}
		else
		{
			GetDlgItem(IDC_SELECT_TEMPLATE_LOCATION_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SELECT_TEMPLATE_LOCATION_COMBO)->ShowWindow(SW_HIDE);
		}

		if (m_eEditorType == stetCollection)
		{
			SetWindowText("Add Template to Collection");

			// (z.manning 2014-12-05 15:16) - PLID 64219 - Show the collection related fields
			GetDlgItem(IDC_TIME_RANGE_BOX)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_START_TIME_LABEL)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_START_TIME)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_END_TIME_LABEL)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_END_TIME)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_APPEARANCE_BOX)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_PRECISION_TEMPLATE_CHECK)->ShowWindow(SW_SHOWNA);

			m_nxtStart = BindNxTimeCtrl(this, IDC_START_TIME);
			m_nxtEnd = BindNxTimeCtrl(this, IDC_END_TIME);

			if (m_dtStartTime.GetStatus() == COleDateTime::valid) {
				m_nxtStart->SetDateTime(m_dtStartTime);
			}
			if (m_dtEndTime.GetStatus() == COleDateTime::valid) {
				m_nxtEnd->SetDateTime(m_dtEndTime);
			}
			if (m_bIsBlock) {
				CheckDlgButton(IDC_PRECISION_TEMPLATE_CHECK, BST_CHECKED);
			}
		}
		else
		{
			// (z.manning 2014-12-08 16:09) - PLID 64219 - Shrink the dialog a bit so we don't have a giant
			// blank space where the hidden controls are.
			const short nYOffset = 130;
			CRect rect;
			CWnd *pwnd = GetDlgItem(IDOK);
			pwnd->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.top -= nYOffset;
			rect.bottom -= nYOffset;
			pwnd->MoveWindow(&rect);

			pwnd = GetDlgItem(IDCANCEL);
			pwnd->GetWindowRect(&rect);
			ScreenToClient(&rect);
			rect.top -= nYOffset;
			rect.bottom -= nYOffset;
			pwnd->MoveWindow(&rect);

			GetWindowRect(&rect);
			rect.bottom -= nYOffset;
			MoveWindow(&rect);
			CenterWindow();
		}

		// (j.jones 2007-04-20 10:21) - PLID 25689 - limit the template name
		((CNxEdit*)GetDlgItem(IDC_NEW_TEMPLATE_NAME))->SetLimitText(50);
	
		CheckDlgButton(IDC_SELECT_EXISTING_TEMPLATE, BST_CHECKED);
		OnSelectExistingTemplate();

	}NxCatchAll("CTemplateSelectDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateSelectDlg::OnSelectExistingTemplate() 
{
	try {

		GetDlgItem(IDC_TEMPLATE_DROPDOWN)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEW_TEMPLATE_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEW_TEMPLATE_CHOOSE_COLOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEW_TEMPLATE_CHOOSE_COLOR)->RedrawWindow();
		if (m_eEditorType == stetLocation) {
			//TES 6/19/2010 - PLID 5888 - This control is hidden if we're not using Resource Availability templates
			GetDlgItem(IDC_SELECT_TEMPLATE_LOCATION_COMBO)->EnableWindow(FALSE);
		}

	}NxCatchAll("CTemplateSelectDlg::OnSelectExistingTemplate");
}

void CTemplateSelectDlg::OnNewTemplate() 
{
	try {

		GetDlgItem(IDC_TEMPLATE_DROPDOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEW_TEMPLATE_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEW_TEMPLATE_CHOOSE_COLOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_NEW_TEMPLATE_CHOOSE_COLOR)->RedrawWindow();
		if (m_eEditorType == stetLocation) {
			//TES 6/19/2010 - PLID 5888 - This control is hidden if we're not using Resource Availability templates
			GetDlgItem(IDC_SELECT_TEMPLATE_LOCATION_COMBO)->EnableWindow(TRUE);
		}

	}NxCatchAll("CTemplateSelectDlg::OnNewTemplate");
}

void CTemplateSelectDlg::OnOK() 
{
	CTemplateLineItemInfo *pLineItem = NULL;
	try {

		CString strTemplateName;

		// (z.manning, 11/13/2006) - If we're creating a new template, add it to data.
		if(IsDlgButtonChecked(IDC_NEW_TEMPLATE_LINE_ITEM)) {
			GetDlgItemText(IDC_NEW_TEMPLATE_NAME, strTemplateName);
			if(strTemplateName.IsEmpty()) {
				MessageBox("Please enter a name for the new template.");
				return;
			}

			//TES 6/19/2010 - PLID 5888 - Resource Availability templates need to have a location specified.
			long nLocationID = -1;
			if (m_eEditorType == stetLocation) {
				NXDATALIST2Lib::IRowSettingsPtr pLocationRow = m_pLocationCombo->CurSel;
				if(pLocationRow) {
					nLocationID = VarLong(pLocationRow->GetValue(0),-1);
				}
				if(nLocationID == -1) {
					MsgBox("Please select a location to associate with the specified resources and times for this template.");
					GetDlgItem(IDC_SELECT_TEMPLATE_LOCATION_COMBO)->SetFocus();
					m_pLocationCombo->DropDownState = g_cvarTrue;
					return;
				}
			}

			//TES 6/19/2010 - PLID 5888 - Use the correct table
			m_nSelectedTemplateID = NewNumber((m_eEditorType == stetLocation) ? "ResourceAvailTemplateT" : "TemplateT", "ID");
			//(e.lally 2008-03-28) PLID 29464 - Added _Q to template name.
			if (m_eEditorType == stetLocation) {
				ExecuteParamSql(
					"INSERT INTO ResourceAvailTemplateT (ID, Name, Color, Priority, LocationID) "
					"SELECT {INT}, {STRING}, {INT}, (SELECT ISNULL(MAX(Priority), 0) + 1 FROM TemplateT), {INT} ",
					m_nSelectedTemplateID, strTemplateName, m_nColor, nLocationID);
			}
			else {
				ExecuteParamSql(
					"INSERT INTO TemplateT (ID, Name, Color, Priority) "
					"SELECT {INT}, {STRING}, {INT}, (SELECT ISNULL(MAX(Priority), 0) + 1 FROM TemplateT) ",
					m_nSelectedTemplateID, strTemplateName, m_nColor);
			}
		}
		else { // We're using an existing template.
			ASSERT(IsDlgButtonChecked(IDC_SELECT_EXISTING_TEMPLATE));
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlTemplates->GetCurSel();
			if(pRow) {
				m_nSelectedTemplateID = VarLong(pRow->GetValue(tlcID));
			}
			else {
				MessageBox("Please select a template before continuing.");
				return;
			}
		}

		// (z.manning 2014-12-08 17:19) - PLID 64219 - Extra handling when adding a collection template
		if (m_eEditorType == stetCollection)
		{
			if (m_nxtStart->GetStatus() != 1) {
				MessageBox("Please enter a valid start time.", NULL, MB_OK | MB_ICONERROR);
				return;
			}
			if (m_nxtEnd->GetStatus() != 1) {
				MessageBox("Please enter a valid end time.", NULL, MB_OK | MB_ICONERROR);
				return;
			}
			if (m_nxtStart->GetDateTime() >= m_nxtEnd->GetDateTime()) {
				MessageBox("Please enter an end time later than the start time.", NULL, MB_OK | MB_ICONERROR);
				return;
			}

			m_dtStartTime = m_nxtStart->GetDateTime();
			m_dtEndTime = m_nxtEnd->GetDateTime();
			m_bIsBlock = (IsDlgButtonChecked(IDC_PRECISION_TEMPLATE_CHECK) == BST_CHECKED);
		}
		
		CDialog::OnOK();

	}NxCatchAll("CTemplateSelectDlg::OnOK");
}


void CTemplateSelectDlg::OnNewTemplateChooseColor() 
{
	try {

		m_ctrlColorPicker.SetColor(m_nColor);
		m_ctrlColorPicker.ShowColor();
		m_nColor = m_ctrlColorPicker.GetColor();
		GetDlgItem(IDC_NEW_TEMPLATE_CHOOSE_COLOR)->RedrawWindow();

	}NxCatchAll("CTemplateSelectDlg::OnNewTemplateChooseColor");
}

void CTemplateSelectDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (nIDCtl == IDC_NEW_TEMPLATE_CHOOSE_COLOR && IsDlgButtonChecked(IDC_NEW_TEMPLATE_LINE_ITEM)) {
		// (z.manning 2012-03-27 12:09) - PLID 49227 - Moved this code to global drawing utils
		DrawColorOnButton(lpDrawItemStruct, m_nColor);
	} 
	else {	
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
	}
}

void CTemplateSelectDlg::OnRequeryFinishedSelectExistingTemplate(short nFlags)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	for(pRow = m_pdlTemplates->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
		long nColor = VarLong(pRow->GetValue(tlcColor), RGB(255,255,255));
		pRow->PutCellBackColor(tlcColorPreview, nColor);
		pRow->PutCellBackColorSel(tlcColorPreview, nColor);
	}

	// (z.manning 2014-12-17 14:49) - PLID 64427 - Select the template if there's one set already
	if (m_nSelectedTemplateID != -1) {
		m_pdlTemplates->SetSelByColumn(tlcID, m_nSelectedTemplateID);
	}
}
