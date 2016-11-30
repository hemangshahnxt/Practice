// EMRSelectProductDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRSelectProductDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEMRSelectProductDlg dialog

#define COLUMN_SERVICE_ID		0
#define COLUMN_CATEGORY_ID		1
#define COLUMN_CATEGORY_NAME	2
#define COLUMN_SUPPLIER_NAME	3
#define COLUMN_PRODUCT_NAME		4
#define COLUMN_PRICE			5
#define COLUMN_LAST_COST		6

CEMRSelectProductDlg::CEMRSelectProductDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRSelectProductDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRSelectProductDlg)
		m_strProductName = "";
		m_nSelectedProductID = -1;
		m_nSelectedCategoryID = -1;
		m_bAutoAddAction = FALSE;
		m_bDisableInvAction = FALSE;
	//}}AFX_DATA_INIT
}


void CEMRSelectProductDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRSelectProductDlg)
	DDX_Control(pDX, IDC_CHECK_AUTO_ASSIGN_INV_ACTION, m_btnAutoSelectAction);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_ADD_CATEGORY, m_btnAddCategory);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRSelectProductDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRSelectProductDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_CATEGORY, OnBtnAddCategory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRSelectProductDlg message handlers

BOOL CEMRSelectProductDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 17:27) - PLID 29840 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_NEW);
		m_btnAddCategory.AutoSet(NXB_NEW);
		m_btnCancel.AutoSet(NXB_CLOSE);
		
		m_ProductList = BindNxDataListCtrl(this,IDC_ACTION_PRODUCT_COMBO,GetRemoteData(),true);

		if(!m_bDisableInvAction) {

			m_bAutoAddAction = GetRemotePropertyInt("AutoAddEMRInventoryActions",1,0,"<None>",true) == 1 ? TRUE : FALSE;

			CheckDlgButton(IDC_CHECK_AUTO_ASSIGN_INV_ACTION, m_bAutoAddAction);
		}
		else {
			GetDlgItem(IDC_CHECK_AUTO_ASSIGN_INV_ACTION)->EnableWindow(FALSE);
		}

		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ADD_CATEGORY)->EnableWindow(FALSE);
	}
	NxCatchAll("Error in CEMRSelectProductDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRSelectProductDlg::OnOK() 
{
	try {

		if(m_ProductList->CurSel == -1) {
			AfxMessageBox("Please select an inventory item before continuing.");
			return;
		}

		m_nSelectedProductID = VarLong(m_ProductList->GetValue(m_ProductList->CurSel, COLUMN_SERVICE_ID), -1);
		m_strProductName = VarString(m_ProductList->GetValue(m_ProductList->CurSel, COLUMN_PRODUCT_NAME), "");
	
		//save the checkbox selection for future use
		m_bAutoAddAction = IsDlgButtonChecked(IDC_CHECK_AUTO_ASSIGN_INV_ACTION);
		SetRemotePropertyInt("AutoAddEMRInventoryActions",m_bAutoAddAction ? 1 : 0, 0, "<None>");
		
		CDialog::OnOK();
	
	}NxCatchAll("Error selecting product.");
}

void CEMRSelectProductDlg::OnBtnAddCategory() 
{
	try {

		if(m_ProductList->CurSel == -1) {
			AfxMessageBox("Please select an inventory item before continuing.");
			return;
		}

		long nCategoryID = VarLong(m_ProductList->GetValue(m_ProductList->CurSel, COLUMN_CATEGORY_ID), -1);

		if(nCategoryID == -1) {
			AfxMessageBox("The selected inventory item does not have a category associated with it.");
			return;
		}

		CString strWarning, strCategory;
		long nCount = 0;

		strCategory = VarString(m_ProductList->GetValue(m_ProductList->CurSel, COLUMN_CATEGORY_NAME), "");
		_RecordsetPtr rs = CreateRecordset("SELECT Count(ServiceT.ID) AS CountOfProducts FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"WHERE Category = %li AND Active = 1",nCategoryID);
		if(!rs->eof) {
			nCount = AdoFldLong(rs, "CountOfProducts",0);
		}
		rs->Close();

		strWarning.Format("You are about to add %li inventory items from the '%s' category to your EMR data list.",nCount,strCategory);

		if(IsDlgButtonChecked(IDC_CHECK_AUTO_ASSIGN_INV_ACTION))
			strWarning += "\nEach item will also be auto-assigned to select the product as the default action.";

		strWarning += "\n\nAre you sure you wish to do this?";

		if(IDNO == MessageBox(strWarning,"Practice",MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		m_nSelectedCategoryID = nCategoryID;
	
		if(!m_bDisableInvAction) {
			//save the checkbox selection for future use
			m_bAutoAddAction = IsDlgButtonChecked(IDC_CHECK_AUTO_ASSIGN_INV_ACTION);
			SetRemotePropertyInt("AutoAddEMRInventoryActions",m_bAutoAddAction ? 1 : 0, 0, "<None>");
		}
		else {
			m_bAutoAddAction = FALSE;
		}

		CDialog::OnOK();

	}NxCatchAll("Error selecting category.");
}

BEGIN_EVENTSINK_MAP(CEMRSelectProductDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRSelectProductDlg)
	ON_EVENT(CEMRSelectProductDlg, IDC_ACTION_PRODUCT_COMBO, 16 /* SelChosen */, OnSelChosenActionProductCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRSelectProductDlg::OnSelChosenActionProductCombo(long nRow) 
{
	if(nRow == -1) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ADD_CATEGORY)->EnableWindow(FALSE);
		return;
	}

	GetDlgItem(IDOK)->EnableWindow(TRUE);
	
	if(VarLong(m_ProductList->GetValue(nRow, COLUMN_CATEGORY_ID), -1) == -1) {
		GetDlgItem(IDC_BTN_ADD_CATEGORY)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_ADD_CATEGORY)->EnableWindow(TRUE);
	}
}
