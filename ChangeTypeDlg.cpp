// ChangeTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChangeTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeTypeDlg dialog


CChangeTypeDlg::CChangeTypeDlg(CWnd* pParent)
	: CNxDialog(CChangeTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	//initialize to -1, nothing selected
	nStatus = -1;
}


void CChangeTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeTypeDlg)
	DDX_Control(pDX, IDC_PROVIDER_BTN, m_btnProvider);
	DDX_Control(pDX, IDC_USER_BTN, m_btnUser);
	DDX_Control(pDX, IDC_SUPPLIER_BTN, m_btnSupplier);
	DDX_Control(pDX, IDC_OTHER_BTN, m_btnOther);
	DDX_Control(pDX, IDC_REFPHYS_BTN, m_btnRefPhys);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeTypeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChangeTypeDlg)
	ON_BN_CLICKED(IDC_OTHER_BTN, OnOtherBtn)
	ON_BN_CLICKED(IDC_PROVIDER_BTN, OnProviderBtn)
	ON_BN_CLICKED(IDC_REFPHYS_BTN, OnRefphysBtn)
	ON_BN_CLICKED(IDC_SUPPLIER_BTN, OnSupplierBtn)
	ON_BN_CLICKED(IDC_USER_BTN, OnUserBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeTypeDlg message handlers

void CChangeTypeDlg::OnOK()
{
	
	if(nStatus == -1){
		AfxMessageBox("Please select a type before you hit OK");
		return;
	}
	
	CDialog::OnOK();

}

void CChangeTypeDlg::OnOtherBtn() 
{
	nStatus = 0;	
}

void CChangeTypeDlg::OnProviderBtn() 
{
	nStatus = 2;	
}

void CChangeTypeDlg::OnRefphysBtn() 
{
	nStatus = 1;
}

void CChangeTypeDlg::OnSupplierBtn() 
{
	nStatus = 8;
}

void CChangeTypeDlg::OnUserBtn() 
{
	nStatus = 4;
}

void CChangeTypeDlg::OnCancel()
{
	CDialog::OnCancel();
}

BOOL CChangeTypeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	switch (nStatus) {
		case 0:
			CheckRadioButton(IDC_REFPHYS_BTN, IDC_USER_BTN, IDC_OTHER_BTN);
			//OnOtherBtn();
		break;
	
		case 1:
			CheckRadioButton(IDC_REFPHYS_BTN, IDC_USER_BTN, IDC_REFPHYS_BTN);
			//OnRefphysBtn();
		break;
		
		case 2:
			CheckRadioButton(IDC_REFPHYS_BTN, IDC_USER_BTN, IDC_PROVIDER_BTN);
			//OnProviderBtn();
		break;

		case 4:
			CheckRadioButton(IDC_REFPHYS_BTN, IDC_USER_BTN, IDC_USER_BTN);
			//OnUserBtn();
		break;

		case 8:
			CheckRadioButton(IDC_REFPHYS_BTN, IDC_USER_BTN, IDC_SUPPLIER_BTN);
			//OnSupplierBtn();
		break;

		default:
			//don't check anything
		break;
	}



	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
