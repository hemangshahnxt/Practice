// SelectRecipientsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectRecipientsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectRecipientsDlg dialog


CSelectRecipientsDlg::CSelectRecipientsDlg(CWnd* pParent)
	: CNxDialog(CSelectRecipientsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectRecipientsDlg)
	//}}AFX_DATA_INIT
}


void CSelectRecipientsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectRecipientsDlg)
	DDX_Control(pDX, IDC_LLEFT, m_buLLeft);
	DDX_Control(pDX, IDC_RRIGHT, m_buRRight);
	DDX_Control(pDX, IDC_RIGHT, m_buRight);
	DDX_Control(pDX, IDC_LEFT, m_buLeft);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectRecipientsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectRecipientsDlg)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_RRIGHT, OnRright)
	ON_BN_CLICKED(IDC_LLEFT, OnLleft)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectRecipientsDlg message handlers

BOOL CSelectRecipientsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try{
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pAvail = BindNxDataListCtrl(IDC_AVAIL);
		m_pSelect = BindNxDataListCtrl(IDC_SELECT, false);

		long nRow;
		for(int i = 0; i < m_dwRecipientsAry.GetSize(); i++) {
			nRow = m_pAvail->FindByColumn(0, (long)m_dwRecipientsAry.GetAt(i), 0, FALSE);
			if(nRow >= 0) {
				m_pSelect->TakeRow(m_pAvail->GetRow(nRow));
			}
		}

		m_buLeft.AutoSet(NXB_LEFT);
		m_buRight.AutoSet(NXB_RIGHT);
		m_buLLeft.AutoSet(NXB_LLEFT);
		m_buRRight.AutoSet(NXB_RRIGHT);

		// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
		((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));


	}NxCatchAll("Error in CSelectRecipientsDlg::OnInitDialog()");

	return TRUE;
}

void CSelectRecipientsDlg::OnRight() 
{
	if(m_pAvail->CurSel != -1){
		IRowSettingsPtr pRow = m_pSelect->Row[-1];
		pRow->Value[0] = m_pAvail->GetValue(m_pAvail->CurSel,0);
		pRow->Value[1] = m_pAvail->GetValue(m_pAvail->CurSel,1);
		m_pSelect->AddRow(pRow);
		m_pAvail->RemoveRow(m_pAvail->CurSel);
	}
}

void CSelectRecipientsDlg::OnLeft() 
{
	if(m_pSelect->CurSel != -1){
		IRowSettingsPtr pRow = m_pAvail->Row[-1];
		pRow->Value[0] = m_pSelect->GetValue(m_pSelect->CurSel,0);
		pRow->Value[1] = m_pSelect->GetValue(m_pSelect->CurSel,1);
		m_pAvail->AddRow(pRow);
		m_pSelect->RemoveRow(m_pSelect->CurSel);
	}
}

void CSelectRecipientsDlg::OnOK() 
{
	//Fill in the arrays.
	m_dwRecipientsAry.RemoveAll();
	m_strRecipientsAry.RemoveAll();
	for(int i = 0; i < m_pSelect->GetRowCount(); i++) {
		m_dwRecipientsAry.Add((DWORD)VarLong(m_pSelect->GetValue(i, 0)));
		m_strRecipientsAry.Add(VarString(m_pSelect->GetValue(i, 1)));
	}

	CDialog::OnOK();
}

void CSelectRecipientsDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CSelectRecipientsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectRecipientsDlg)
	ON_EVENT(CSelectRecipientsDlg, IDC_AVAIL, 3 /* DblClickCell */, OnDblClickCellAvail, VTS_I4 VTS_I2)
	ON_EVENT(CSelectRecipientsDlg, IDC_SELECT, 3 /* DblClickCell */, OnDblClickCellSelect, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectRecipientsDlg::OnDblClickCellAvail(long nRowIndex, short nColIndex) 
{
	OnRight();
}

void CSelectRecipientsDlg::OnDblClickCellSelect(long nRowIndex, short nColIndex) 
{
	OnLeft();
}

void CSelectRecipientsDlg::OnRright() 
{
	m_pSelect->TakeAllRows(m_pAvail);
}

void CSelectRecipientsDlg::OnLleft() 
{
	m_pAvail->TakeAllRows(m_pSelect);
}
