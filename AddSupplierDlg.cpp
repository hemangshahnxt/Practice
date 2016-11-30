// AddSupplierDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "AddSupplierDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CAddSupplierDlg dialog


CAddSupplierDlg::CAddSupplierDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAddSupplierDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddSupplierDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddSupplierDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddSupplierDlg)
	DDX_Control(pDX, IDC_BTN_ADD_NEW_SUPPLIER, m_btnAddNewSupplier);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddSupplierDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAddSupplierDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_SUPPLIER, OnBtnAddNewSupplier)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddSupplierDlg message handlers

BOOL CAddSupplierDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:35) - PLID 29820 - NxIconified buttons
		m_btnAddNewSupplier.AutoSet(NXB_NEW);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_SupplierList = BindNxDataListCtrl(this,IDC_NEW_SUPPLIER_LIST,GetRemoteData(),true);
	}
	NxCatchAll("Error in CAddSupplierDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddSupplierDlg::OnOK() 
{
	if(m_SupplierList->CurSel != -1) {

		//get the ID
		long SupplierID = m_SupplierList->GetValue(m_SupplierList->CurSel,0).lVal;

		//check to make sure it doesn't exist already for this product
		if(IsRecordsetEmpty("SELECT * FROM MultiSupplierT WHERE SupplierID = %li AND ProductID = %li", SupplierID, m_ProductID)) {
			
			//add the record
			ExecuteSql("INSERT INTO MultiSupplierT (SupplierID,ProductID) VALUES (%li,%li)", SupplierID, m_ProductID);

			//if the product has no supplier already, make this one default
			_RecordsetPtr rs = CreateRecordset("SELECT DefaultMultiSupplierID FROM ProductT WHERE ID = %li",m_ProductID);
			if(!rs->eof) {
				if(AdoFldLong(rs, "DefaultMultiSupplierID", -1) == -1) {

					_RecordsetPtr rs2 = CreateRecordset("SELECT ID FROM MultiSupplierT WHERE SupplierID = %li AND ProductID = %li", SupplierID, m_ProductID);
					if(!rs2->eof) {
						long nNewMultiSupplierID = AdoFldLong(rs2, "ID");
						ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = %li WHERE ID = %li", nNewMultiSupplierID, m_ProductID);
					}
					rs2->Close();
				}
			}
			rs->Close();

			//auditing
			//look up the product
			rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li", m_ProductID);
			CString strItem;
			if(!rs->eof && rs->Fields->Item["Name"]->Value.vt != VT_NULL)
				strItem = CString(rs->Fields->Item["Name"]->Value.bstrVal);

			CString strNew = "Added: ";
			strNew += CString(m_SupplierList->GetValue(m_SupplierList->CurSel, 1).bstrVal);
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, strItem, nAuditID, aeiProductSupplier, m_ProductID, "", strNew, aepMedium, aetChanged);

		}
		else {
			AfxMessageBox("This supplier is already selected for this item, please choose another supplier.");
			return;
		}
	}		
	
	CDialog::OnOK();
}

void CAddSupplierDlg::OnBtnAddNewSupplier() 
{	
	// this will create a contact and return the ID of the contact that was added
	long nNewSupplierID = GetMainFrame()->AddContact(GetMainFrame()->dctSupplier);
	if(nNewSupplierID != -1){
		m_SupplierList->Requery();
	}

	CChildFrame *p = GetMainFrame()->GetActiveViewFrame();
	if (p) {
		CNxTabView *pView = (CNxTabView *)p->GetActiveView();
		CChildFrame *pFrame = GetMainFrame()->GetActiveViewFrame();
		if (pView && !pFrame->IsOfType(INVENTORY_MODULE_NAME)) {
			CDialog::OnOK();
		}
	}
}
