// OfficeVisitConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "OfficeVisitConfigDlg.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// COfficeVisitConfigDlg dialog


COfficeVisitConfigDlg::COfficeVisitConfigDlg(CWnd* pParent)
	: CNxDialog(COfficeVisitConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COfficeVisitConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COfficeVisitConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COfficeVisitConfigDlg)
	DDX_Control(pDX, IDC_CHECK_ENABLE_OFFICE_VISIT_INCREMENTS, m_checkEnableOfficeVisits);
	DDX_Control(pDX, IDC_5_CATEGORY_TEXT, m_nxlCategory5);
	DDX_Control(pDX, IDC_4_CATEGORY_TEXT, m_nxlCategory4);
	DDX_Control(pDX, IDC_3_CATEGORY_TEXT, m_nxlCategory3);
	DDX_Control(pDX, IDC_2_CATEGORY_TEXT, m_nxlCategory2);
	DDX_Control(pDX, IDC_DEFAULT_LEVEL, m_nxeditDefaultLevel);
	DDX_Control(pDX, IDC_2_NUMBER_ITEMS, m_nxedit2NumberItems);
	DDX_Control(pDX, IDC_3_NUMBER_ITEMS, m_nxedit3NumberItems);
	DDX_Control(pDX, IDC_4_NUMBER_ITEMS, m_nxedit4NumberItems);
	DDX_Control(pDX, IDC_5_NUMBER_ITEMS, m_nxedit5NumberItems);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COfficeVisitConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(COfficeVisitConfigDlg)
	ON_EN_KILLFOCUS(IDC_DEFAULT_LEVEL, OnKillfocusDefaultLevel)
	ON_WM_CLOSE()
	ON_EN_KILLFOCUS(IDC_2_NUMBER_ITEMS, OnKillfocus2NumberItems)
	ON_EN_KILLFOCUS(IDC_3_NUMBER_ITEMS, OnKillfocus3NumberItems)
	ON_EN_KILLFOCUS(IDC_4_NUMBER_ITEMS, OnKillfocus4NumberItems)
	ON_EN_KILLFOCUS(IDC_5_NUMBER_ITEMS, OnKillfocus5NumberItems)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_OFFICE_VISIT_INCREMENTS, OnCheckEnableOfficeVisitIncrements)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// COfficeVisitConfigDlg message handlers

#define ShowMultiString(nLevel)	GetDlgItem(IDC_##nLevel##_CATEGORY_DROPDOWN)->ShowWindow(SW_HIDE);\
	m_nxlCategory##nLevel##.ShowWindow(SW_SHOW);\
	InvalidateDlgItem(IDC_##nLevel##_CATEGORY_TEXT);\
	CString strSql = "SELECT Name FROM EmrCategoriesT WHERE ID IN (";\
	for(int i = 0; i < m_arCategories##nLevel##.GetSize(); i++) {\
		CString strPart;\
		strPart.Format("%li, ", m_arCategories##nLevel##.GetAt(i));\
		strSql += strPart;\
	}\
	strSql = strSql.Left(strSql.GetLength()-2) + ")";\
	_RecordsetPtr rsNames = CreateRecordset("%s", strSql);\
	CString strNames;\
	while(!rsNames->eof) {\
		strNames += AdoFldString(rsNames, "Name") + ", ";\
		rsNames->MoveNext();\
	}\
	m_nxlCategory##nLevel##.SetText(strNames.Left(strNames.GetLength()-2));\
	m_nxlCategory##nLevel##.SetType(m_nxlCategory##nLevel##.IsWindowEnabled() ? dtsHyperlink : dtsDisabledHyperlink);


BOOL COfficeVisitConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
	m_nxlCategory2.SetColor(GetNxColor(GNC_ADMIN, 0));
	m_nxlCategory3.SetColor(GetNxColor(GNC_ADMIN, 0));
	m_nxlCategory4.SetColor(GetNxColor(GNC_ADMIN, 0));
	m_nxlCategory5.SetColor(GetNxColor(GNC_ADMIN, 0));

	// (z.manning, 05/01/2008) - PLID 29864 - Set button style
	m_btnClose.AutoSet(NXB_CLOSE);

	//Let's set up all our datalists.
	m_pDefault2 = BindNxDataListCtrl(IDC_2_DEFAULT, false);
	IRowSettingsPtr pRow = m_pDefault2->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Non-Default"));
	m_pDefault2->AddRow(pRow);
	pRow = m_pDefault2->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Any"));
	m_pDefault2->AddRow(pRow);

	m_pDefault3 = BindNxDataListCtrl(IDC_3_DEFAULT, false);
	pRow = m_pDefault3->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Non-Default"));
	m_pDefault3->AddRow(pRow);
	pRow = m_pDefault3->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Any"));
	m_pDefault3->AddRow(pRow);

	m_pDefault4 = BindNxDataListCtrl(IDC_4_DEFAULT, false);
	pRow = m_pDefault4->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Non-Default"));
	m_pDefault4->AddRow(pRow);
	pRow = m_pDefault4->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Any"));
	m_pDefault4->AddRow(pRow);

	m_pDefault5 = BindNxDataListCtrl(IDC_5_DEFAULT, false);
	pRow = m_pDefault5->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Non-Default"));
	m_pDefault5->AddRow(pRow);
	pRow = m_pDefault5->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Any"));
	m_pDefault5->AddRow(pRow);

	m_pCategories2 = BindNxDataListCtrl(IDC_2_CATEGORY_DROPDOWN);
	m_pCategories3 = BindNxDataListCtrl(IDC_3_CATEGORY_DROPDOWN);
	m_pCategories4 = BindNxDataListCtrl(IDC_4_CATEGORY_DROPDOWN);
	m_pCategories5 = BindNxDataListCtrl(IDC_5_CATEGORY_DROPDOWN);

	pRow = m_pCategories2->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("{Multiple}"));
	m_pCategories2->InsertRow(pRow, 0);
	pRow = m_pCategories2->GetRow(-1);
	pRow->PutValue(0, (long)-2);
	pRow->PutValue(1, _bstr_t("{All}"));
	m_pCategories2->InsertRow(pRow, 0);

	pRow = m_pCategories3->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("{Multiple}"));
	m_pCategories3->InsertRow(pRow, 0);
	pRow = m_pCategories2->GetRow(-1);
	pRow->PutValue(0, (long)-2);
	pRow->PutValue(1, _bstr_t("{All}"));
	m_pCategories3->InsertRow(pRow, 0);

	pRow = m_pCategories4->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("{Multiple}"));
	m_pCategories4->InsertRow(pRow, 0);
	pRow = m_pCategories4->GetRow(-1);
	pRow->PutValue(0, (long)-2);
	pRow->PutValue(1, _bstr_t("{All}"));
	m_pCategories4->InsertRow(pRow, 0);

	pRow = m_pCategories5->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("{Multiple}"));
	m_pCategories5->InsertRow(pRow, 0);
	pRow = m_pCategories5->GetRow(-1);
	pRow->PutValue(0, (long)-2);
	pRow->PutValue(1, _bstr_t("{All}"));
	m_pCategories5->InsertRow(pRow, 0);


	//OK, now let's actually load the data.
	int nEnableOfficeVisits = GetRemotePropertyInt("EnableEMROfficeVisitIncrementing", 0, 0, "<None>", true);
	if(nEnableOfficeVisits == 1)
		m_checkEnableOfficeVisits.SetCheck(TRUE);

	int nDefaultLevel = GetRemotePropertyInt("DefaultOfficeVisitLevel", 2, 0, "<None>", true);
	if(nDefaultLevel < 1 || nDefaultLevel > 5) {
		//Reset the default.
		SetRemotePropertyInt("DefaultOfficeVisitLevel", 2, 0, "<None>");
		nDefaultLevel = 2;
	}
	SetDlgItemInt(IDC_DEFAULT_LEVEL, nDefaultLevel);
	
	int nNumber2 = GetRemotePropertyInt("NumberOfItems", 2, 2, "<None>", true);
	if(nNumber2 < 1) {
		//Reset the default.
		SetRemotePropertyInt("NumberOfItems", 2, 2, "<None>");
		nNumber2 = 2;
	}
	SetDlgItemInt(IDC_2_NUMBER_ITEMS, nNumber2);
	int n2Default = GetRemotePropertyInt("ItemType", 0, 2, "<None>", true);
	if(n2Default != 0 && n2Default != 1) {
		//Reset the default.
		SetRemotePropertyInt("ItemType", 0, 2, "<None>");
		n2Default = 0;
	}
	m_pDefault2->CurSel = n2Default;
	GetRemotePropertyArray("EmrCategories", m_arCategories2, 2, "<None>");
	if(m_arCategories2.GetSize() == 0) {
		//All categories are allowed.
		GetDlgItem(IDC_2_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_pCategories2->SetSelByColumn(0, (long)-2);
		m_nxlCategory2.ShowWindow(SW_HIDE);
	}
	else if(m_arCategories2.GetSize() == 1) {
		//Exactly one category is allowed.
		m_pCategories2->SetSelByColumn(0, (long)m_arCategories2.GetAt(0));
		GetDlgItem(IDC_2_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory2.ShowWindow(SW_HIDE);
	}
	else {
		//Multiple.
		ShowMultiString(2);
	}

	int nNumber3 = GetRemotePropertyInt("NumberOfItems", 4, 3, "<None>", true);
	if(nNumber3 < 1) {
		//Reset the default.
		SetRemotePropertyInt("NumberOfItems", 4, 3, "<None>");
		nNumber3 = 4;
	}
	SetDlgItemInt(IDC_3_NUMBER_ITEMS, nNumber3);
	int n3Default = GetRemotePropertyInt("ItemType", 0, 3, "<None>", true);
	if(n3Default != 0 && n3Default != 1) {
		//Reset the default.
		SetRemotePropertyInt("ItemType", 0, 3, "<None>");
		n3Default = 0;
	}
	m_pDefault3->CurSel = n3Default;
	GetRemotePropertyArray("EmrCategories", m_arCategories3, 3, "<None>");
	if(m_arCategories3.GetSize() == 0) {
		//All categories are allowed.
		m_pCategories3->SetSelByColumn(0, (long)-2);
		GetDlgItem(IDC_3_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory3.ShowWindow(SW_HIDE);
	}
	else if(m_arCategories3.GetSize() == 1) {
		//Exactly one category is allowed.
		m_pCategories3->SetSelByColumn(0, (long)m_arCategories3.GetAt(0));
		GetDlgItem(IDC_3_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory3.ShowWindow(SW_HIDE);
	}
	else {
		//Multiple.
		ShowMultiString(3);
	}

	int nNumber4 = GetRemotePropertyInt("NumberOfItems", 6, 4, "<None>", true);
	if(nNumber4 < 1) {
		//Reset the default.
		SetRemotePropertyInt("NumberOfItems", 6, 4, "<None>");
		nNumber4 = 6;
	}
	SetDlgItemInt(IDC_4_NUMBER_ITEMS, nNumber4);
	int n4Default = GetRemotePropertyInt("ItemType", 0, 4, "<None>", true);
	if(n4Default != 0 && n4Default != 1) {
		//Reset the default.
		SetRemotePropertyInt("ItemType", 0, 4, "<None>");
		n4Default = 0;
	}
	m_pDefault4->CurSel = n4Default;
	GetRemotePropertyArray("EmrCategories", m_arCategories4, 4, "<None>");
	if(m_arCategories4.GetSize() == 0) {
		//All categories are allowed.
		m_pCategories4->SetSelByColumn(0, (long)-2);
		GetDlgItem(IDC_4_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory4.ShowWindow(SW_HIDE);
	}
	else if(m_arCategories4.GetSize() == 1) {
		//Exactly one category is allowed.
		m_pCategories4->SetSelByColumn(0, (long)m_arCategories4.GetAt(0));
		GetDlgItem(IDC_4_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory4.ShowWindow(SW_HIDE);
	}
	else {
		//Multiple.
		ShowMultiString(4);
	}

	int nNumber5 = GetRemotePropertyInt("NumberOfItems", 8, 5, "<None>", true);
	if(nNumber5 < 1) {
		//Reset the default.
		SetRemotePropertyInt("NumberOfItems", 8, 5, "<None>");
		nNumber5 = 8;
	}
	SetDlgItemInt(IDC_5_NUMBER_ITEMS, nNumber5);
	int n5Default = GetRemotePropertyInt("ItemType", 0, 5, "<None>", true);
	if(n5Default != 0 && n5Default != 1) {
		//Reset the default.
		SetRemotePropertyInt("ItemType", 0, 5, "<None>");
		n5Default = 0;
	}
	m_pDefault5->CurSel = n5Default;
	GetRemotePropertyArray("EmrCategories", m_arCategories5, 5, "<None>");
	if(m_arCategories5.GetSize() == 0) {
		//All categories are allowed.
		m_pCategories5->SetSelByColumn(0, (long)-2);
		GetDlgItem(IDC_5_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory5.ShowWindow(SW_HIDE);
	}
	else if(m_arCategories5.GetSize() == 1) {
		//Exactly one category is allowed.
		m_pCategories5->SetSelByColumn(0, (long)m_arCategories5.GetAt(0));
		GetDlgItem(IDC_5_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory5.ShowWindow(SW_HIDE);
	}
	else {
		//Multiple.
		ShowMultiString(5);
	}

	OnKillfocusDefaultLevel();

	EnableControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COfficeVisitConfigDlg::OnKillfocusDefaultLevel() 
{
	if(!m_checkEnableOfficeVisits.GetCheck())
		return;

	CString strLevel;
	GetDlgItemText(IDC_DEFAULT_LEVEL, strLevel);
	strLevel.TrimLeft();
	strLevel.TrimRight();
	if(strLevel.IsEmpty() || strLevel.SpanIncluding("1234567890").GetLength() != strLevel.GetLength() || atoi(strLevel) < 1 || atoi(strLevel) > 5) {
		MsgBox("The level must default to a number from 1 to 5.");
		GetDlgItem(IDC_DEFAULT_LEVEL)->SetFocus();
		return;
	}
	int nLevel = atoi(strLevel);
	SetRemotePropertyInt("DefaultOfficeVisitLevel", nLevel, 0, "<None>");
	
	//Enable everything appropriately.
	GetDlgItem(IDC_2_NUMBER_ITEMS)->EnableWindow(nLevel < 2);
	GetDlgItem(IDC_2_DEFAULT)->EnableWindow(nLevel < 2);
	GetDlgItem(IDC_2_CATEGORY_DROPDOWN)->EnableWindow(nLevel < 2);
	GetDlgItem(IDC_2_CATEGORY_TEXT)->EnableWindow(nLevel < 2);
	m_nxlCategory2.SetType(nLevel < 2 ? dtsHyperlink : dtsDisabledHyperlink);
	InvalidateDlgItem(IDC_2_CATEGORY_DROPDOWN);
	InvalidateDlgItem(IDC_2_CATEGORY_TEXT);

	GetDlgItem(IDC_3_NUMBER_ITEMS)->EnableWindow(nLevel < 3);
	GetDlgItem(IDC_3_DEFAULT)->EnableWindow(nLevel < 3);
	GetDlgItem(IDC_3_CATEGORY_DROPDOWN)->EnableWindow(nLevel < 3);
	GetDlgItem(IDC_3_CATEGORY_TEXT)->EnableWindow(nLevel < 3);
	m_nxlCategory3.SetType(nLevel < 3 ? dtsHyperlink : dtsDisabledHyperlink);
	InvalidateDlgItem(IDC_3_CATEGORY_DROPDOWN);
	InvalidateDlgItem(IDC_3_CATEGORY_TEXT);

	GetDlgItem(IDC_4_NUMBER_ITEMS)->EnableWindow(nLevel < 4);
	GetDlgItem(IDC_4_DEFAULT)->EnableWindow(nLevel < 4);
	GetDlgItem(IDC_4_CATEGORY_DROPDOWN)->EnableWindow(nLevel < 4);
	GetDlgItem(IDC_4_CATEGORY_TEXT)->EnableWindow(nLevel < 4);
	m_nxlCategory4.SetType(nLevel < 4 ? dtsHyperlink : dtsDisabledHyperlink);
	InvalidateDlgItem(IDC_4_CATEGORY_DROPDOWN);
	InvalidateDlgItem(IDC_4_CATEGORY_TEXT);

	GetDlgItem(IDC_5_NUMBER_ITEMS)->EnableWindow(nLevel < 5);
	GetDlgItem(IDC_5_DEFAULT)->EnableWindow(nLevel < 5);
	GetDlgItem(IDC_5_CATEGORY_DROPDOWN)->EnableWindow(nLevel < 5);
	GetDlgItem(IDC_5_CATEGORY_TEXT)->EnableWindow(nLevel < 5);
	m_nxlCategory5.SetType(nLevel < 5 ? dtsHyperlink : dtsDisabledHyperlink);
	InvalidateDlgItem(IDC_5_CATEGORY_DROPDOWN);
	InvalidateDlgItem(IDC_5_CATEGORY_TEXT);
}

void COfficeVisitConfigDlg::OnOK() 
{
	//Save everything.

	CDialog::OnOK();
}

void COfficeVisitConfigDlg::OnClose() 
{
	OnOK();
}

void COfficeVisitConfigDlg::OnKillfocus2NumberItems() 
{
	CString strEntered;
	GetDlgItemText(IDC_2_NUMBER_ITEMS, strEntered);
	strEntered.TrimLeft();
	strEntered.TrimRight();
	if(strEntered.IsEmpty() || strEntered.SpanIncluding("1234567890").GetLength() != strEntered.GetLength() || atoi(strEntered) < 1) {
		MsgBox("This value must be a number greater than 0");
		GetDlgItem(IDC_2_NUMBER_ITEMS)->SetFocus();
		return;
	}
	SetRemotePropertyInt("NumberOfItems", atoi(strEntered), 2, "<None>");
}

void COfficeVisitConfigDlg::OnKillfocus3NumberItems() 
{
	CString strEntered;
	GetDlgItemText(IDC_3_NUMBER_ITEMS, strEntered);
	strEntered.TrimLeft();
	strEntered.TrimRight();
	if(strEntered.IsEmpty() || strEntered.SpanIncluding("1234567890").GetLength() != strEntered.GetLength() || atoi(strEntered) < 1) {
		MsgBox("This value must be a number greater than 0");
		GetDlgItem(IDC_3_NUMBER_ITEMS)->SetFocus();
	return;
	}
	SetRemotePropertyInt("NumberOfItems", atoi(strEntered), 3, "<None>");
}

void COfficeVisitConfigDlg::OnKillfocus4NumberItems() 
{
	CString strEntered;
	GetDlgItemText(IDC_4_NUMBER_ITEMS, strEntered);
	strEntered.TrimLeft();
	strEntered.TrimRight();
	if(strEntered.IsEmpty() || strEntered.SpanIncluding("1234567890").GetLength() != strEntered.GetLength() || atoi(strEntered) < 1) {
		MsgBox("This value must be a number greater than 0");
		GetDlgItem(IDC_4_NUMBER_ITEMS)->SetFocus();
	return;
	}
	SetRemotePropertyInt("NumberOfItems", atoi(strEntered), 4, "<None>");
}

void COfficeVisitConfigDlg::OnKillfocus5NumberItems() 
{
	CString strEntered;
	GetDlgItemText(IDC_5_NUMBER_ITEMS, strEntered);
	strEntered.TrimLeft();
	strEntered.TrimRight();
	if(strEntered.IsEmpty() || strEntered.SpanIncluding("1234567890").GetLength() != strEntered.GetLength() || atoi(strEntered) < 1) {
		MsgBox("This value must be a number greater than 0");
		GetDlgItem(IDC_5_NUMBER_ITEMS)->SetFocus();
	return;
	}
	SetRemotePropertyInt("NumberOfItems", atoi(strEntered), 5, "<None>");
}

BEGIN_EVENTSINK_MAP(COfficeVisitConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(COfficeVisitConfigDlg)
	ON_EVENT(COfficeVisitConfigDlg, IDC_2_DEFAULT, 1 /* SelChanging */, OnSelChanging2Default, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_2_DEFAULT, 16 /* SelChosen */, OnSelChosen2Default, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_3_DEFAULT, 1 /* SelChanging */, OnSelChanging3Default, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_3_DEFAULT, 16 /* SelChosen */, OnSelChosen3Default, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_4_DEFAULT, 1 /* SelChanging */, OnSelChanging4Default, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_4_DEFAULT, 16 /* SelChosen */, OnSelChosen4Default, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_5_DEFAULT, 1 /* SelChanging */, OnSelChanging5Default, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_5_DEFAULT, 16 /* SelChosen */, OnSelChosen5Default, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_2_CATEGORY_DROPDOWN, 1 /* SelChanging */, OnSelChanging2CategoryDropdown, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_3_CATEGORY_DROPDOWN, 1 /* SelChanging */, OnSelChanging3CategoryDropdown, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_4_CATEGORY_DROPDOWN, 1 /* SelChanging */, OnSelChanging4CategoryDropdown, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_5_CATEGORY_DROPDOWN, 1 /* SelChanging */, OnSelChanging5CategoryDropdown, VTS_PI4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_2_CATEGORY_DROPDOWN, 16 /* SelChosen */, OnSelChosen2CategoryDropdown, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_3_CATEGORY_DROPDOWN, 16 /* SelChosen */, OnSelChosen3CategoryDropdown, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_4_CATEGORY_DROPDOWN, 16 /* SelChosen */, OnSelChosen4CategoryDropdown, VTS_I4)
	ON_EVENT(COfficeVisitConfigDlg, IDC_5_CATEGORY_DROPDOWN, 16 /* SelChosen */, OnSelChosen5CategoryDropdown, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void COfficeVisitConfigDlg::OnSelChanging2Default(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault2->CurSel;
}

void COfficeVisitConfigDlg::OnSelChosen2Default(long nRow) 
{
	if(nRow != 0 && nRow != 1) {
		ASSERT(FALSE);
		return;
	}
	SetRemotePropertyInt("ItemType", nRow, 2, "<None>");
}

void COfficeVisitConfigDlg::OnSelChanging3Default(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault3->CurSel;
}

void COfficeVisitConfigDlg::OnSelChosen3Default(long nRow) 
{
	if(nRow != 0 && nRow != 1) {
		ASSERT(FALSE);
		return;
	}
	SetRemotePropertyInt("ItemType", nRow, 3, "<None>");
}

void COfficeVisitConfigDlg::OnSelChanging4Default(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault4->CurSel;
}

void COfficeVisitConfigDlg::OnSelChosen4Default(long nRow) 
{
	if(nRow != 0 && nRow != 1) {
		ASSERT(FALSE);
		return;
	}
	SetRemotePropertyInt("ItemType", nRow, 4, "<None>");
}

void COfficeVisitConfigDlg::OnSelChanging5Default(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault5->CurSel;
}

void COfficeVisitConfigDlg::OnSelChosen5Default(long nRow) 
{
	if(nRow != 0 && nRow != 1) {
		ASSERT(FALSE);
		return;
	}
	SetRemotePropertyInt("ItemType", nRow, 5, "<None>");
}

void COfficeVisitConfigDlg::OnSelChanging2CategoryDropdown(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault2->CurSel;
}

void COfficeVisitConfigDlg::OnSelChanging3CategoryDropdown(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault3->CurSel;
}

void COfficeVisitConfigDlg::OnSelChanging4CategoryDropdown(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault4->CurSel;
}

void COfficeVisitConfigDlg::OnSelChanging5CategoryDropdown(long FAR* nNewSel) 
{
	if(*nNewSel == -1) *nNewSel = m_pDefault5->CurSel;
}

void COfficeVisitConfigDlg::OnSelChosen2CategoryDropdown(long nRow) 
{
	if(nRow == -1) {
		ASSERT(FALSE);
		return;
	}
	long nNewCatID = VarLong(m_pCategories2->GetValue(nRow, 0));
	if(nNewCatID != -1) {
		m_arCategories2.RemoveAll();
		if(nNewCatID != -2) {
			m_arCategories2.Add(nNewCatID);
		}
		SetRemotePropertyArray("EmrCategories", m_arCategories2, 2, "<None>");
	}
	else {
		On2CategoryText();		
	}
	
}

void COfficeVisitConfigDlg::OnSelChosen3CategoryDropdown(long nRow) 
{
	if(nRow == -1) {
		ASSERT(FALSE);
		return;
	}
	long nNewCatID = VarLong(m_pCategories2->GetValue(nRow, 0));
	if(nNewCatID != -1) {
		m_arCategories3.RemoveAll();
		if(nNewCatID != -2) {
			m_arCategories3.Add(nNewCatID);
		}
		SetRemotePropertyArray("EmrCategories", m_arCategories3, 3, "<None>");
	}
	else {
		On3CategoryText();		
	}
}

void COfficeVisitConfigDlg::OnSelChosen4CategoryDropdown(long nRow) 
{
	if(nRow == -1) {
		ASSERT(FALSE);
		return;
	}
	long nNewCatID = VarLong(m_pCategories4->GetValue(nRow, 0));
	if(nNewCatID != -1) {
		m_arCategories4.RemoveAll();
		if(nNewCatID != -2) {
			m_arCategories4.Add(nNewCatID);
		}
		SetRemotePropertyArray("EmrCategories", m_arCategories4, 4, "<None>");
	}
	else {
		On4CategoryText();		
	}
}

void COfficeVisitConfigDlg::OnSelChosen5CategoryDropdown(long nRow) 
{
	if(nRow == -1) {
		ASSERT(FALSE);
		return;
	}
	long nNewCatID = VarLong(m_pCategories5->GetValue(nRow, 0));
	if(nNewCatID != -1) {
		m_arCategories5.RemoveAll();
		if(nNewCatID != -2) {
			m_arCategories5.Add(nNewCatID);
		}
		SetRemotePropertyArray("EmrCategories", m_arCategories5, 5, "<None>");
	}
	else {
		On5CategoryText();		
	}
}

HBRUSH COfficeVisitConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-05-12 13:30) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	/*
	//to avoid drawing problems with transparent text and disabled ites
	//override the NxDialog way of doing text with a non-grey background
	//NxDialog relies on the NxColor to draw the background, then draws text transparently
	//instead, we actually color the background of the STATIC text
	if (nCtlColor == CTLCOLOR_STATIC
		&& pWnd->GetDlgCtrlID() != IDC_2_CATEGORY_TEXT
		&& pWnd->GetDlgCtrlID() != IDC_3_CATEGORY_TEXT
		&& pWnd->GetDlgCtrlID() != IDC_4_CATEGORY_TEXT
		&& pWnd->GetDlgCtrlID() != IDC_5_CATEGORY_TEXT
		&& pWnd->GetDlgCtrlID() != IDC_2_NUMBER_ITEMS
		&& pWnd->GetDlgCtrlID() != IDC_3_NUMBER_ITEMS
		&& pWnd->GetDlgCtrlID() != IDC_4_NUMBER_ITEMS
		&& pWnd->GetDlgCtrlID() != IDC_5_NUMBER_ITEMS
		&& pWnd->GetDlgCtrlID() != IDC_DEFAULT_LEVEL) {
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
		return m_brush;
	} else {
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}*/
}

void COfficeVisitConfigDlg::On2CategoryText() 
{
	if(!m_checkEnableOfficeVisits.GetCheck())
		return;

	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "EmrCategoriesT");
	CDWordArray dwa;
	for(int i = 0; i < m_arCategories2.GetSize(); i++) {
		dwa.Add((DWORD)m_arCategories2.GetAt(i));
	}
	dlg.PreSelect(dwa);
	int nReturn = dlg.Open("EmrCategoriesT", "", "ID", "Name", "EMR Categories");
	if(nReturn == IDOK) {
		m_arCategories2.RemoveAll();
		dlg.FillArrayWithIDs(dwa);
		for(int i = 0; i < dwa.GetSize(); i++) {
			m_arCategories2.Add(dwa.GetAt(i));
		}
	}
	//Now, either they said OK, in which case m_arCategories2 is what they selected, or they 
	//cancelled, in which case m_arCategories2 is whatever it was before they started all this
	//nonsense.  In either case, m_arCategories2 is what we want to reflect on screen.
	if(m_arCategories2.GetSize() == 0) {
		GetDlgItem(IDC_2_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory2.ShowWindow(SW_HIDE);
		m_pCategories2->SetSelByColumn(0, (long)-2);
	}
	else if(m_arCategories2.GetSize() == 1) {
		GetDlgItem(IDC_2_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory2.ShowWindow(SW_HIDE);
		m_pCategories2->SetSelByColumn(0, (long)m_arCategories2.GetAt(0));
	}
	else {
		ShowMultiString(2);
	}
	SetRemotePropertyArray("EmrCategories", m_arCategories2, 2, "<None>");
}

void COfficeVisitConfigDlg::On3CategoryText() 
{
	if(!m_checkEnableOfficeVisits.GetCheck())
		return;

	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "EmrCategoriesT");
	CDWordArray dwa;
	for(int i = 0; i < m_arCategories3.GetSize(); i++) {
		dwa.Add((DWORD)m_arCategories3.GetAt(i));
	}
	dlg.PreSelect(dwa);
	int nReturn = dlg.Open("EmrCategoriesT", "", "ID", "Name", "EMR Categories");
	if(nReturn == IDOK) {
		m_arCategories3.RemoveAll();
		dlg.FillArrayWithIDs(dwa);
		for(int i = 0; i < dwa.GetSize(); i++) {
			m_arCategories3.Add(dwa.GetAt(i));
		}
	}
	//Now, either they said OK, in which case m_arCategories3 is what they selected, or they 
	//cancelled, in which case m_arCategories3 is whatever it was before they started all this
	//nonsense.  In either case, m_arCategories3 is what we want to reflect on screen.
	if(m_arCategories3.GetSize() == 0) {
		GetDlgItem(IDC_3_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory3.ShowWindow(SW_HIDE);
		m_pCategories3->SetSelByColumn(0, (long)-2);
	}
	else if(m_arCategories3.GetSize() == 1) {
		GetDlgItem(IDC_3_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory3.ShowWindow(SW_HIDE);
		m_pCategories3->SetSelByColumn(0, (long)m_arCategories3.GetAt(0));
	}
	else {
		ShowMultiString(3);
	}
	SetRemotePropertyArray("EmrCategories", m_arCategories3, 3, "<None>");
}

void COfficeVisitConfigDlg::On4CategoryText() 
{
	if(!m_checkEnableOfficeVisits.GetCheck())
		return;

	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "EmrCategoriesT");
	CDWordArray dwa;
	for(int i = 0; i < m_arCategories4.GetSize(); i++) {
		dwa.Add((DWORD)m_arCategories4.GetAt(i));
	}
	dlg.PreSelect(dwa);
	int nReturn = dlg.Open("EmrCategoriesT", "", "ID", "Name", "EMR Categories");
	if(nReturn == IDOK) {
		m_arCategories4.RemoveAll();
		dlg.FillArrayWithIDs(dwa);
		for(int i = 0; i < dwa.GetSize(); i++) {
			m_arCategories4.Add(dwa.GetAt(i));
		}
	}
	//Now, either they said OK, in which case m_arCategories4 is what they selected, or they 
	//cancelled, in which case m_arCategories4 is whatever it was before they started all this
	//nonsense.  In either case, m_arCategories4 is what we want to reflect on screen.
	if(m_arCategories4.GetSize() == 0) {
		GetDlgItem(IDC_4_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory4.ShowWindow(SW_HIDE);
		m_pCategories4->SetSelByColumn(0, (long)-2);
	}
	else if(m_arCategories4.GetSize() == 1) {
		GetDlgItem(IDC_4_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory4.ShowWindow(SW_HIDE);
		m_pCategories4->SetSelByColumn(0, (long)m_arCategories4.GetAt(0));
	}
	else {
		ShowMultiString(4);
	}
	SetRemotePropertyArray("EmrCategories", m_arCategories4, 4, "<None>");
}

void COfficeVisitConfigDlg::On5CategoryText() 
{
	if(!m_checkEnableOfficeVisits.GetCheck())
		return;

	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "EmrCategoriesT");
	CDWordArray dwa;
	for(int i = 0; i < m_arCategories5.GetSize(); i++) {
		dwa.Add((DWORD)m_arCategories5.GetAt(i));
	}
	dlg.PreSelect(dwa);
	int nReturn = dlg.Open("EmrCategoriesT", "", "ID", "Name", "EMR Categories");
	if(nReturn == IDOK) {
		m_arCategories5.RemoveAll();
		dlg.FillArrayWithIDs(dwa);
		for(int i = 0; i < dwa.GetSize(); i++) {
			m_arCategories5.Add(dwa.GetAt(i));
		}
	}
	//Now, either they said OK, in which case m_arCategories5 is what they selected, or they 
	//cancelled, in which case m_arCategories5 is whatever it was before they started all this
	//nonsense.  In either case, m_arCategories5 is what we want to reflect on screen.
	if(m_arCategories5.GetSize() == 0) {
		GetDlgItem(IDC_5_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory5.ShowWindow(SW_HIDE);
		m_pCategories5->SetSelByColumn(0, (long)-2);
	}
	else if(m_arCategories5.GetSize() == 1) {
		GetDlgItem(IDC_5_CATEGORY_DROPDOWN)->ShowWindow(SW_SHOW);
		m_nxlCategory5.ShowWindow(SW_HIDE);
		m_pCategories5->SetSelByColumn(0, (long)m_arCategories5.GetAt(0));
	}
	else {
		ShowMultiString(5);
	}
	SetRemotePropertyArray("EmrCategories", m_arCategories5, 5, "<None>");
}

BOOL COfficeVisitConfigDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if(!m_checkEnableOfficeVisits.GetCheck())
		return CNxDialog::OnSetCursor(pWnd, nHitTest, message);

	if (m_nxlCategory2.IsWindowVisible() && m_nxlCategory2.GetType() == dtsHyperlink)
	{
		CRect rc;
		m_nxlCategory2.GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (m_nxlCategory3.IsWindowVisible() && m_nxlCategory3.GetType() == dtsHyperlink)
	{
		CRect rc;
		m_nxlCategory3.GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (m_nxlCategory4.IsWindowVisible() && m_nxlCategory4.GetType() == dtsHyperlink)
	{
		CRect rc;
		m_nxlCategory4.GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (m_nxlCategory5.IsWindowVisible() && m_nxlCategory5.GetType() == dtsHyperlink)
	{
		CRect rc;
		m_nxlCategory5.GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT COfficeVisitConfigDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
	case IDC_2_CATEGORY_TEXT:
		On2CategoryText();
		break;
	case IDC_3_CATEGORY_TEXT:
		On3CategoryText();
		break;
	case IDC_4_CATEGORY_TEXT:
		On4CategoryText();
		break;
	case IDC_5_CATEGORY_TEXT:
		On5CategoryText();
		break;
	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}

void COfficeVisitConfigDlg::OnCheckEnableOfficeVisitIncrements() 
{
	EnableControls();
}

void COfficeVisitConfigDlg::EnableControls()
{
	if(m_checkEnableOfficeVisits.GetCheck()) {
		GetDlgItem(IDC_DEFAULT_LEVEL)->EnableWindow(TRUE);
		OnKillfocusDefaultLevel();

		SetRemotePropertyInt("EnableEMROfficeVisitIncrementing", 1, 0, "<None>");
	}
	else {
		GetDlgItem(IDC_DEFAULT_LEVEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_2_NUMBER_ITEMS)->EnableWindow(FALSE);
		GetDlgItem(IDC_3_NUMBER_ITEMS)->EnableWindow(FALSE);
		GetDlgItem(IDC_4_NUMBER_ITEMS)->EnableWindow(FALSE);
		GetDlgItem(IDC_5_NUMBER_ITEMS)->EnableWindow(FALSE);	
		GetDlgItem(IDC_2_DEFAULT)->EnableWindow(FALSE);
		GetDlgItem(IDC_3_DEFAULT)->EnableWindow(FALSE);
		GetDlgItem(IDC_4_DEFAULT)->EnableWindow(FALSE);
		GetDlgItem(IDC_5_DEFAULT)->EnableWindow(FALSE);
		GetDlgItem(IDC_2_CATEGORY_DROPDOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_3_CATEGORY_DROPDOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_4_CATEGORY_DROPDOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_5_CATEGORY_DROPDOWN)->EnableWindow(FALSE);

		SetRemotePropertyInt("EnableEMROfficeVisitIncrementing", 0, 0, "<None>");
	}
}
