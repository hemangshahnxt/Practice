// SelectPacketDlg.cpp : implementation file
//

#include "stdafx.h"
#include "letterwriting.h"
#include "letterwritingrc.h"
#include "SelectPacketDlg.h"
#include "ConfigPacketsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectPacketDlg dialog


CSelectPacketDlg::CSelectPacketDlg(CWnd* pParent)
	: CNxDialog(CSelectPacketDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectPacketDlg)
	m_bReverseMerge = FALSE;
	m_bChooseOnly = FALSE;
	m_bSeparateDocuments = FALSE;
	//}}AFX_DATA_INIT
}


void CSelectPacketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectPacketDlg)
	DDX_Check(pDX, IDC_CHECK_REVERSE_MERGE, m_bReverseMerge);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnMerge);
	DDX_Control(pDX, IDC_CHECK_REVERSE_MERGE, m_btnReverseMerge);
	DDX_Control(pDX, IDC_SEPARATE_DOCUMENTS, m_btnSeparateDocuments);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CSelectPacketDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectPacketDlg)
	ON_BN_CLICKED(IDC_CONFIG_PACKETS, OnConfigPackets)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectPacketDlg message handlers

BOOL CSelectPacketDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	// (z.manning, 04/25/2008) - PLID 29795 - Set button styles
	m_btnMerge.AutoSet(NXB_MERGE);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_pPacketList = BindNxDataListCtrl(IDC_PACKET_LIST);
	m_nPacketID = -1;

	if (m_bChooseOnly)
	{
		SetDlgItemText(IDOK, "OK");
		GetDlgItem(IDC_CHECK_REVERSE_MERGE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SEPARATE_DOCUMENTS)->ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectPacketDlg::OnOK() 
{
	try {
		if(m_pPacketList->CurSel == -1) {
			AfxMessageBox("Please select a packet from the list.");
			return;
		}

		UpdateData(TRUE);
		m_nPacketID = VarLong(m_pPacketList->GetValue(m_pPacketList->CurSel, 0), -1);

		if (!m_bChooseOnly)
		{
			if(!ReturnsRecords("SELECT PacketID FROM PacketComponentsT WHERE PacketID = %li", m_nPacketID)) {
				MsgBox("The selected packet does not contain any templates.\nNo documents will be created.");
			}
		}

		m_bSeparateDocuments = IsDlgButtonChecked(IDC_SEPARATE_DOCUMENTS);
		
		CDialog::OnOK();
	}NxCatchAll("Error in CSelectPacketDlg::OnOK()");
}

void CSelectPacketDlg::OnCancel() 
{
	m_nPacketID = -1;

	CDialog::OnCancel();
}

void CSelectPacketDlg::OnConfigPackets() 
{
	CConfigPacketsDlg dlg(this);
	dlg.DoModal();
	m_pPacketList->Requery();
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CSelectPacketDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectPacketDlg)
	ON_EVENT(CSelectPacketDlg, IDC_PACKET_LIST, 16 /* SelChosen */, OnSelChosenPacketList, VTS_I4)
	ON_EVENT(CSelectPacketDlg, IDC_PACKET_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPacketList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectPacketDlg::OnSelChosenPacketList(long nRow) 
{
	if(nRow == -1) {
		m_nPacketID = -1;
	}
	else {
		m_nPacketID = VarLong(m_pPacketList->GetValue(m_pPacketList->CurSel, 0), -1);
	}

}

void CSelectPacketDlg::OnRequeryFinishedPacketList(short nFlags) 
{
	if(m_nPacketID == -1) {
		//Let's try to select the first one
		if(m_pPacketList->GetRowCount() != 0) {
			m_pPacketList->CurSel = 0;
			m_nPacketID = VarLong(m_pPacketList->GetValue(m_pPacketList->CurSel, 0), -1);
		}
	}
	else {
		m_pPacketList->SetSelByColumn(0, (long)m_nPacketID);
	}
}
