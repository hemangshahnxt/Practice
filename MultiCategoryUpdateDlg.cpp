// MultiCategoryUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiCategoryUpdateDlg.h"
#include "CategorySelectDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiCategoryUpdateDlg dialog


CMultiCategoryUpdateDlg::CMultiCategoryUpdateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultiCategoryUpdateDlg::IDD, pParent)
{
	m_nDefaultCategoryID = -1;
}


void CMultiCategoryUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiCategoryUpdateDlg)
	DDX_Control(pDX, IDC_REMOVE_ALL, m_btnRemoveAll);
	DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_SELECT_CATEGORY, m_btnSelectCategory);
	DDX_Control(pDX, IDC_BTN_REMOVE_CATEGORY, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_CATEGORY_BOX, m_editCategory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiCategoryUpdateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiCategoryUpdateDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SELECT_CATEGORY, OnBtnSelectCategory)
	ON_BN_CLICKED(IDC_BTN_REMOVE_CATEGORY, OnBtnRemoveCategory)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiCategoryUpdateDlg message handlers

BEGIN_EVENTSINK_MAP(CMultiCategoryUpdateDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMultiCategoryUpdateDlg)
	ON_EVENT(CMultiCategoryUpdateDlg, IDC_SELECTED_CPT_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedList, VTS_I4 VTS_I2)
	ON_EVENT(CMultiCategoryUpdateDlg, IDC_UNSELECTED_CPT_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CMultiCategoryUpdateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//setup the datalists
	m_pUnselected = BindNxDataListCtrl(this, IDC_UNSELECTED_CPT_LIST, GetRemoteData(), true);
	m_pSelected = BindNxDataListCtrl(this, IDC_SELECTED_CPT_LIST, GetRemoteData(), false);

	//setup the NxIconButtons
	m_btnAdd.AutoSet(NXB_DOWN);
	m_btnRemove.AutoSet(NXB_UP);
	m_btnRemoveAll.SetIcon(IDI_UUARROW);
	// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultiCategoryUpdateDlg::OnAdd() 
{
	long nCurSel = m_pUnselected->GetCurSel();

	if(nCurSel == -1)
		return;

	m_pSelected->TakeCurrentRow(m_pUnselected);
	
}

void CMultiCategoryUpdateDlg::OnRemove() 
{
	long nCurSel = m_pSelected->GetCurSel();

	if(nCurSel == -1)
		return;

	m_pUnselected->TakeCurrentRow(m_pSelected);
}

void CMultiCategoryUpdateDlg::OnDblClickCellSelectedList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	//call the remove function
	OnRemove();

}

void CMultiCategoryUpdateDlg::OnDblClickCellUnselectedList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1) 
		return;

	//call the add function
	OnAdd();
}

void CMultiCategoryUpdateDlg::OnOK() 
{
	try {
		//make sure we have a valid category
		// (j.jones 2015-03-10 09:37) - PLID 64971 - supported multiple categories
		if (m_aryCategoryIDs.size() == 0) {
			if (IDNO == MessageBox("Are you sure you want to configure all the selected services to have no category?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//make sure something is selected
		long nSelRows = m_pSelected->GetRowCount();
		if(nSelRows == 0) {
			MessageBox("You must have at least one service code selected to continue.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		// (j.jones 2015-03-10 09:37) - PLID 64971 - get the category name
		CString strCategoryNames;
		GetDlgItemText(IDC_CATEGORY_BOX, strCategoryNames);
		CString strWarn;
		if (strCategoryNames.IsEmpty()) {
			strWarn.Format("You are about to update %li codes to have no category. Are you absolutely sure you wish to do this?", nSelRows);
		}
		else {
			strWarn.Format("You are about to update %li codes to have the category: %s. Are you absolutely sure you wish to do this?", nSelRows, strCategoryNames);
		}

		//one last warning to make sure
		if (IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		// (j.jones 2015-03-10 10:40) - PLID 64971 - update en masse using the API,
		// this will also audit
		std::vector<long> aryServiceIDs;
		for(int i = 0; i < nSelRows; i++) {
			long nID = m_pSelected->GetValue(i, 0);
			aryServiceIDs.push_back(nID);
		}
		UpdateServiceCategories(aryServiceIDs, m_aryCategoryIDs, m_nDefaultCategoryID, false);

	} NxCatchAll("Error in CMultiCategoryUpdateDlg::OnOK()");

	//and close the dialog
	CDialog::OnOK();
}

void CMultiCategoryUpdateDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CMultiCategoryUpdateDlg::OnRemoveAll() 
{
	//move all the rows from selected to unselected
	if(m_pSelected->GetRowCount() > 0) {
		m_pUnselected->TakeAllRows(m_pSelected);
	}
}

// (j.jones 2015-03-10 09:37) - PLID 64971 - added ability to select multiple categories
void CMultiCategoryUpdateDlg::OnBtnSelectCategory()
{
	try {

		CCategorySelectDlg dlg(this, true, "service code");

		if (m_aryCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), m_aryCategoryIDs.begin(), m_aryCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = m_nDefaultCategoryID;
		}


		if (IDOK == dlg.DoModal()) {

			//this isn't saving anything, just setting our member variables
			m_aryCategoryIDs.clear();
			m_aryCategoryIDs.insert(m_aryCategoryIDs.end(), dlg.m_arySelectedCategoryIDs.begin(), dlg.m_arySelectedCategoryIDs.end());
			m_nDefaultCategoryID = dlg.m_nSelectedDefaultCategoryID;

			//now load the proper display string
			CString strNewCategoryNames;
			LoadServiceCategories(m_aryCategoryIDs, m_nDefaultCategoryID, strNewCategoryNames);

			SetDlgItemText(IDC_CATEGORY_BOX, strNewCategoryNames);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-10 09:37) - PLID 64971 - added ability to select multiple categories
void CMultiCategoryUpdateDlg::OnBtnRemoveCategory()
{
	try {

		//no need to warn, just clear the categories, they have not been applied yet
		m_aryCategoryIDs.clear();
		m_nDefaultCategoryID = -1;

		SetDlgItemText(IDC_CATEGORY_BOX, "");

	}NxCatchAll(__FUNCTION__);
}