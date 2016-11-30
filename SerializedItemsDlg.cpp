//PLID 28380	r.galicki	6/11/08		-  View Serialized Items dialog

// SerializedItemsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SerializedItemsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSerializedItemsDlg dialog


CSerializedItemsDlg::CSerializedItemsDlg(CWnd* pParent /*=NULL*/, long nID, BOOL bIsBill)
	: CNxDialog(CSerializedItemsDlg::IDD, pParent)
{
	m_nID = nID;
	m_bIsBill = bIsBill;
	//{{AFX_DATA_INIT(CSerializedItemsDlg)
	//}}AFX_DATA_INIT
}


void CSerializedItemsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSerializedItemsDlg)
	DDX_Control(pDX, IDC_LBL_INFO, m_lbl_Info);
	DDX_Control(pDX, ID_BTN_CLOSE, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSerializedItemsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSerializedItemsDlg)
	ON_BN_CLICKED(ID_BTN_CLOSE, OnCloseButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSerializedItemsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

		m_pItemList = BindNxDataList2Ctrl(IDC_NXDL_ITEM_LIST, GetRemoteData(), FALSE);

		CString strWhere;

		if(m_bIsBill) //get serialized items for entire bill
		{
			m_lbl_Info.SetWindowText("Below is a list of items with serial numbers and expiration dates attached to this bill...");
			strWhere.Format("ChargesT.BillID = %li", m_nID);
		}
		else  //get serialized items for specific charge
		{
			m_lbl_Info.SetWindowText("Below is a list of items with serial numbers and expiration dates attached to this charge...");
			strWhere.Format("ChargesT.ID = %li", m_nID);
		}

		m_pItemList->WhereClause = _bstr_t(strWhere);

		m_pItemList->Requery();
	}
	NxCatchAll("Error in CSearchChecksDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CSerializedItemsDlg message handlers

void CSerializedItemsDlg::OnCloseButton() 
{
	CDialog::OnOK();	
}
