// ExportWizardNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportWizardNameDlg.h"
#include "ExportUtils.h"
#include "ExportWizardDlg.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportWizardNameDlg property page

IMPLEMENT_DYNCREATE(CExportWizardNameDlg, CPropertyPage)

CExportWizardNameDlg::CExportWizardNameDlg() : CPropertyPage(CExportWizardNameDlg::IDD)
{
	Construct(IDD_EXPORT_WIZARD_NAME);
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = "Enter a Name";
	
	//{{AFX_DATA_INIT(CExportWizardNameDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CExportWizardNameDlg::~CExportWizardNameDlg()
{
}

void CExportWizardNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportWizardNameDlg)
	DDX_Control(pDX, IDC_ADD_EMN_TEMPLATE, m_nxbAdd);
	DDX_Control(pDX, IDC_REMOVE_EMN_TEMPLATE, m_nxbRemove);
	DDX_Control(pDX, IDC_EXPORT_NAME, m_editName);
	DDX_Control(pDX, IDC_EMN_LABEL, m_nxstaticEmnLabel);
	DDX_Control(pDX, IDC_EXPORT_CATEGORY_NXLABEL, m_nxlabelCategories);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportWizardNameDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CExportWizardNameDlg)
	ON_BN_CLICKED(IDC_EXPORT_PATIENTS, OnExportPatients)
	ON_BN_CLICKED(IDC_EXPORT_CHARGES, OnExportCharges)
	ON_BN_CLICKED(IDC_EXPORT_APPOINTMENTS, OnExportAppointments)
	ON_BN_CLICKED(IDC_EXPORT_PAYMENTS, OnExportPayments)
	ON_EN_CHANGE(IDC_EXPORT_NAME, OnChangeExportName)
	ON_BN_CLICKED(IDC_EXPORT_EMNS, OnExportEmns)
	ON_BN_CLICKED(IDC_ADD_EMN_TEMPLATE, OnAddEmnTemplate)
	ON_BN_CLICKED(IDC_REMOVE_EMN_TEMPLATE, OnRemoveEmnTemplate)
	ON_BN_CLICKED(IDC_ALLOW_OTHER_TEMPLATES, OnAllowOtherTemplates)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EXPORT_HISTORY, &CExportWizardNameDlg::OnBnClickedExportHistory)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportWizardNameDlg message handlers
BOOL CExportWizardNameDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_editName.SetLimitText(255);

	m_nxbAdd.AutoSet(NXB_RIGHT);
	m_nxbRemove.AutoSet(NXB_LEFT);

	m_pAvail = BindNxDataListCtrl(this, IDC_EMN_TEMPLATES_AVAIL, GetRemoteData(), true);
	m_pSelect = BindNxDataListCtrl(this, IDC_EMN_TEMPLATES_SELECT, GetRemoteData(), false);
	m_pdlCategoryFilter = BindNxDataList2Ctrl(this, IDC_EXPORT_CATEGORY_FILTER, GetRemoteData(), true);

	// (z.manning 2009-12-11 09:37) - PLID 36519
	m_nxlabelCategories.SetType(dtsHyperlink);
	m_nxlabelCategories.SetText(m_strInitialCategoryText);

	// (a.walling 2007-11-28 13:02) - PLID 28044 - Check for expired EMR license
	// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
	if(!(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2)) {
		GetDlgItem(IDC_EXPORT_EMNS)->ShowWindow(SW_HIDE);
	}
	
	// (a.walling 2010-10-04 13:26) - PLID 40738 - Preapre the patient restriction label
	GetDlgItem(IDC_EXPORT_PATIENT_RESTRICTION)->ShowWindow(SW_HIDE);
	SetDlgItemText(IDC_EXPORT_PATIENT_RESTRICTION, GetPatientExportRestrictions().Description());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportWizardNameDlg::OnExportPatients() 
{
	HandleRadioButton();
}

void CExportWizardNameDlg::OnExportCharges() 
{
	HandleRadioButton();
}

void CExportWizardNameDlg::OnExportAppointments() 
{
	HandleRadioButton();
}

void CExportWizardNameDlg::OnExportPayments() 
{
	HandleRadioButton();
}

void CExportWizardNameDlg::HandleRadioButton()
{
	bool bChanged = false;
	if(IsDlgButtonChecked(IDC_EXPORT_PATIENTS)) {
		bChanged = (((CExportWizardDlg*)GetParent())->m_ertRecordType != ertPatients);
		((CExportWizardDlg*)GetParent())->m_ertRecordType = ertPatients;
	}
	else if(IsDlgButtonChecked(IDC_EXPORT_APPOINTMENTS)) {
		bChanged = (((CExportWizardDlg*)GetParent())->m_ertRecordType != ertAppointments);
		((CExportWizardDlg*)GetParent())->m_ertRecordType = ertAppointments;
	}
	else if(IsDlgButtonChecked(IDC_EXPORT_CHARGES)) {
		bChanged = (((CExportWizardDlg*)GetParent())->m_ertRecordType != ertCharges);
		((CExportWizardDlg*)GetParent())->m_ertRecordType = ertCharges;
	}
	else if(IsDlgButtonChecked(IDC_EXPORT_PAYMENTS)) {
		bChanged = (((CExportWizardDlg*)GetParent())->m_ertRecordType != ertPayments);
		((CExportWizardDlg*)GetParent())->m_ertRecordType = ertPayments;
	}
	else if(IsDlgButtonChecked(IDC_EXPORT_EMNS)) {
		bChanged = (((CExportWizardDlg*)GetParent())->m_ertRecordType != ertEMNs);
		((CExportWizardDlg*)GetParent())->m_ertRecordType = ertEMNs;
	}
	// (z.manning 2009-12-10 13:47) - PLID 36519 - Added history export
	else if(IsDlgButtonChecked(IDC_EXPORT_HISTORY)) {
		bChanged = (((CExportWizardDlg*)GetParent())->m_ertRecordType != ertHistory);
		((CExportWizardDlg*)GetParent())->m_ertRecordType = ertHistory;
	}
	GetDlgItem(IDC_ALLOW_OTHER_TEMPLATES)->ShowWindow(IsDlgButtonChecked(IDC_EXPORT_EMNS)?SW_SHOWNA:SW_HIDE);
	GetDlgItem(IDC_EMN_LABEL)->ShowWindow(IsDlgButtonChecked(IDC_EXPORT_EMNS)?SW_SHOWNA:SW_HIDE);
	GetDlgItem(IDC_EMN_TEMPLATES_AVAIL)->ShowWindow(IsDlgButtonChecked(IDC_EXPORT_EMNS)?SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_EMN_TEMPLATES_SELECT)->ShowWindow(IsDlgButtonChecked(IDC_EXPORT_EMNS)?SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_ADD_EMN_TEMPLATE)->ShowWindow(IsDlgButtonChecked(IDC_EXPORT_EMNS)?SW_SHOWNA:SW_HIDE);
	GetDlgItem(IDC_REMOVE_EMN_TEMPLATE)->ShowWindow(IsDlgButtonChecked(IDC_EXPORT_EMNS)?SW_SHOWNA:SW_HIDE);

	UpdateCategoryFilterDisplay();

	if(bChanged) {
		((CExportWizardDlg*)GetParent())->m_arExportFields.RemoveAll();
		// (z.manning, 09/24/2007) - PLID 27498 - Also remove sort fields if we changed export type.
		((CExportWizardDlg*)GetParent())->m_arSortFields.RemoveAll();
	}
	
	UpdateWizardButtons();

	// (a.walling 2010-10-04 13:26) - PLID 40738 - Show/hide the info label
	// (a.walling 2010-10-05 15:01) - PLID 40822
	GetDlgItem(IDC_EXPORT_PATIENT_RESTRICTION)->ShowWindow( (IsDlgButtonChecked(IDC_EXPORT_PATIENTS) && GetPatientExportRestrictions().Enabled(false)) ? SW_SHOWNA : SW_HIDE);
}

void CExportWizardNameDlg::OnChangeExportName() 
{
	//They're allowed to move on now if they have a name.
	CString strName;
	GetDlgItemText(IDC_EXPORT_NAME, strName);
	strName.TrimLeft();
	strName.TrimRight();
	((CExportWizardDlg*)GetParent())->SetWizardButtons(strName.IsEmpty()?0:PSWIZB_NEXT);
}

BOOL CExportWizardNameDlg::OnSetActive()
{
	//Fill things in from our parent.
	
	UINT idcToCheck;
	switch(((CExportWizardDlg*)GetParent())->m_ertRecordType) {
	case ertPatients:
	default:
		idcToCheck = IDC_EXPORT_PATIENTS;
		break;
	case ertAppointments:
		idcToCheck = IDC_EXPORT_APPOINTMENTS;
		break;
	case ertCharges:
		idcToCheck = IDC_EXPORT_CHARGES;
		break;
	case ertPayments:
		idcToCheck = IDC_EXPORT_PAYMENTS;
		break;
	case ertEMNs:
		idcToCheck = IDC_EXPORT_EMNS;
		break;
	case ertHistory: // (z.manning 2009-12-10 13:49) - PLID 36519
		idcToCheck = IDC_EXPORT_HISTORY;
		break;
	}
	CheckRadioButton(IDC_EXPORT_PATIENTS, IDC_EXPORT_HISTORY, idcToCheck);
	HandleRadioButton();

	CString strName = ((CExportWizardDlg*)GetParent())->m_strName;
	strName.TrimLeft();
	strName.TrimRight();
	SetDlgItemText(IDC_EXPORT_NAME, strName);

	m_pAvail->TakeAllRows(m_pSelect);

	//PLID 20029 - sometimes the wrong row was being selected, wait for the requery
	m_pAvail->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);

	for(int i = 0; i < ((CExportWizardDlg*)GetParent())->m_arEmnTemplateIDs.GetSize(); i++) {
		long nRow = m_pAvail->FindByColumn(0, ((CExportWizardDlg*)GetParent())->m_arEmnTemplateIDs[i], 0, FALSE);
		if(nRow != -1) {
			m_pSelect->TakeRow(m_pAvail->GetRow(nRow));
			
		}
	}

	// (z.manning 2009-12-11 09:30) - PLID 36519 - Ensure the history filter is correct
	m_pdlCategoryFilter->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	UpdateCategoryFilterDisplay();

	CheckDlgButton(IDC_ALLOW_OTHER_TEMPLATES, ((CExportWizardDlg*)GetParent())->m_bAllowOtherTemplates?BST_CHECKED:BST_UNCHECKED);

	UpdateWizardButtons();

	return CPropertyPage::OnSetActive();
}

BOOL CExportWizardNameDlg::OnKillActive()
{
	try {
		//Check for duplicate name.
		CString strName;
		GetDlgItemText(IDC_EXPORT_NAME, strName);
		strName.TrimRight();
		strName.TrimLeft();
		if(strName.IsEmpty()) {
			MsgBox("You must enter a name for this export");
			return FALSE;
		}
		if(strName.GetLength() > 255) {
			MsgBox("The name you have entered is too long.  Please enter a name with less than 256 characters.");
			return FALSE;
		}
		if(ReturnsRecords("SELECT ID FROM ExportT WHERE Name = '%s' AND ID <> %li", _Q(strName), ((CExportWizardDlg*)GetParent())->m_nExportID)) {
			MsgBox("A stored export with the name '" + strName + "' already exists.  Please enter a different name.");
			return FALSE;
		}

		((CExportWizardDlg*)GetParent())->m_strName = strName;

		((CExportWizardDlg*)GetParent())->m_arEmnTemplateIDs.RemoveAll();
		for(int i = 0; i < m_pSelect->GetRowCount(); i++) {
			((CExportWizardDlg*)GetParent())->m_arEmnTemplateIDs.Add(VarLong(m_pSelect->GetValue(i,0)));
		}
		((CExportWizardDlg*)GetParent())->m_bAllowOtherTemplates = IsDlgButtonChecked(IDC_ALLOW_OTHER_TEMPLATES)?true:false;
		
		return CPropertyPage::OnKillActive();
	}NxCatchAll("Error in CExportWizardNameDlg::OnKillActive()");
	return FALSE;
}

void CExportWizardNameDlg::OnExportEmns() 
{
	HandleRadioButton();
}

BEGIN_EVENTSINK_MAP(CExportWizardNameDlg, CPropertyPage)
    //{{AFX_EVENTSINK_MAP(CExportWizardNameDlg)
	ON_EVENT(CExportWizardNameDlg, IDC_EMN_TEMPLATES_AVAIL, 2 /* SelChanged */, OnSelChangedEmnTemplatesAvail, VTS_I4)
	ON_EVENT(CExportWizardNameDlg, IDC_EMN_TEMPLATES_AVAIL, 3 /* DblClickCell */, OnDblClickCellEmnTemplatesAvail, VTS_I4 VTS_I2)
	ON_EVENT(CExportWizardNameDlg, IDC_EMN_TEMPLATES_SELECT, 2 /* SelChanged */, OnSelChangedEmnTemplatesSelect, VTS_I4)
	ON_EVENT(CExportWizardNameDlg, IDC_EMN_TEMPLATES_SELECT, 3 /* DblClickCell */, OnDblClickCellEmnTemplatesSelect, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CExportWizardNameDlg, IDC_EXPORT_CATEGORY_FILTER, 16, CExportWizardNameDlg::SelChosenExportCategoryFilter, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CExportWizardNameDlg::HandleSel()
{
	m_nxbAdd.EnableWindow(m_pAvail->CurSel != -1);
	m_nxbRemove.EnableWindow(m_pSelect->CurSel != -1);
	UpdateWizardButtons();
}

void CExportWizardNameDlg::UpdateWizardButtons()
{
	//They can't go Back (we're the first page), and they can only go Next if they have filled in a name.
	CString strName;
	GetDlgItemText(IDC_EXPORT_NAME, strName);
	if(strName.IsEmpty()) {
		((CExportWizardDlg*)GetParent())->SetWizardButtons(0);
	}
	else {
		//TES 9/9/2005: They also can't go next if they have EMNs, don't have any templates selected, and don't allow other templates.
		if(IsDlgButtonChecked(IDC_EXPORT_EMNS) && !IsDlgButtonChecked(IDC_ALLOW_OTHER_TEMPLATES) && !m_pSelect->GetRowCount()) {
			((CExportWizardDlg*)GetParent())->SetWizardButtons(0);
		}
		else {
			((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_NEXT);
		}
	}
}

void CExportWizardNameDlg::OnAddEmnTemplate() 
{
	m_pSelect->TakeCurrentRow(m_pAvail);
	HandleSel();
}

void CExportWizardNameDlg::OnSelChangedEmnTemplatesAvail(long nNewSel) 
{
	HandleSel();
}

void CExportWizardNameDlg::OnDblClickCellEmnTemplatesAvail(long nRowIndex, short nColIndex) 
{
	OnAddEmnTemplate();
}

void CExportWizardNameDlg::OnSelChangedEmnTemplatesSelect(long nNewSel) 
{
	HandleSel();
}

void CExportWizardNameDlg::OnDblClickCellEmnTemplatesSelect(long nRowIndex, short nColIndex) 
{
	OnRemoveEmnTemplate();
}

void CExportWizardNameDlg::OnRemoveEmnTemplate() 
{
	m_pAvail->TakeCurrentRow(m_pSelect);
	HandleSel();
}

void CExportWizardNameDlg::OnAllowOtherTemplates() 
{
	((CExportWizardDlg*)GetParent())->m_bAllowOtherTemplates = IsDlgButtonChecked(IDC_ALLOW_OTHER_TEMPLATES)?true:false;
	UpdateWizardButtons();
}

// (z.manning 2009-12-10 14:03) - PLID 36519
void CExportWizardNameDlg::OnBnClickedExportHistory()
{
	try
	{
		HandleRadioButton();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-12-10 15:38) - PLID 36519
LRESULT CExportWizardNameDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch(nIdc)
		{
			case IDC_EXPORT_CATEGORY_NXLABEL:
				OpenCategoryMultiSelectDialog();
				break;
		}

	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (z.manning 2009-12-10 16:16) - PLID 36519
void CExportWizardNameDlg::OpenCategoryMultiSelectDialog()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "NoteCatsF");
	dlg.PreSelect(((CExportWizardDlg*)GetParent())->m_arynCategoryIDs);
	int nResult = dlg.Open("NoteCatsF", "", "ID", "Description", "Select the categories for the export");
	if(nResult == IDOK)
	{
		((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.RemoveAll();
		dlg.FillArrayWithIDs(((CExportWizardDlg*)GetParent())->m_arynCategoryIDs);
		CVariantArray aryvarCategories;
		dlg.FillArrayWithNames(aryvarCategories);
		CString strCategories;
		for(int nCatIndex = 0; nCatIndex < aryvarCategories.GetSize(); nCatIndex++) {
			strCategories += VarString(aryvarCategories.GetAt(nCatIndex), "") + ", ";
		}
		strCategories.TrimRight(", ");
		m_nxlabelCategories.SetText(strCategories);
		UpdateCategoryFilterDisplay();
	}
}

// (z.manning 2009-12-10 16:16) - PLID 36519
BOOL CExportWizardNameDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try
	{
		if(GetDlgItem(IDC_EXPORT_CATEGORY_NXLABEL)->IsWindowVisible())
		{
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			GetDlgItem(IDC_EXPORT_CATEGORY_NXLABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

	}NxCatchAll(__FUNCTION__);

	return CPropertyPage::OnSetCursor(pWnd, nHitTest, message);
}

void CExportWizardNameDlg::UpdateCategoryFilterDisplay()
{
	// (z.manning 2009-12-10 14:59) - PLID 36519 - Show/hide the history category filter as needed
	GetDlgItem(IDC_EXPORT_CATEGORY_LABEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EXPORT_CATEGORY_FILTER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EXPORT_CATEGORY_NXLABEL)->ShowWindow(SW_HIDE);

	if(IsDlgButtonChecked(IDC_EXPORT_HISTORY)) {
		if(((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.GetSize() == 0) {
			m_pdlCategoryFilter->SetSelByColumn(cfcID, (long)-1);
		}
		else if(((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.GetSize() == 1) {
			m_pdlCategoryFilter->SetSelByColumn(cfcID, ((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.GetAt(0));
		}
		GetDlgItem(IDC_EXPORT_CATEGORY_LABEL)->ShowWindow(SW_SHOW);
		if(((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.GetSize() > 1) {
			GetDlgItem(IDC_EXPORT_CATEGORY_NXLABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EXPORT_CATEGORY_NXLABEL)->Invalidate();
		}
		else {
			GetDlgItem(IDC_EXPORT_CATEGORY_FILTER)->ShowWindow(SW_SHOW);
		}
	}
}

// (z.manning 2009-12-10 16:37) - PLID 36519
void CExportWizardNameDlg::SelChosenExportCategoryFilter(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.RemoveAll();
			return;
		}

		long nCategoryID = VarLong(pRow->GetValue(cfcID));
		if(nCategoryID == -1) {
			((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.RemoveAll();
		}
		// (j.jones 2011-03-29 16:34) - PLID 43038 - changed an if to else if
		else if(nCategoryID == -2) {
			OpenCategoryMultiSelectDialog();
		}
		else {
			((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.RemoveAll();
			((CExportWizardDlg*)GetParent())->m_arynCategoryIDs.Add(nCategoryID);
		}

		UpdateCategoryFilterDisplay();

	}NxCatchAll(__FUNCTION__);
}
