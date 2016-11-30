// NYMedicaidSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NYMedicaidSetupDlg.h"

// (k.messina 2010-07-16 10:00) - PLID 39685 - created

/////////////////////////////////////////////////////////////////////////////
// CNYMedicaidSetupDlg dialog

IMPLEMENT_DYNAMIC(CNYMedicaidSetupDlg, CNxDialog)

CNYMedicaidSetupDlg::CNYMedicaidSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNYMedicaidSetupDlg::IDD, pParent)
{

}

CNYMedicaidSetupDlg::~CNYMedicaidSetupDlg()
{
}

void CNYMedicaidSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCNYMedicaidSetupDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_BOX25, m_checkBox25);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNYMedicaidSetupDlg, CNxDialog)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCNYMedicaidSetupDlg message handlers

BOOL CNYMedicaidSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		//set the ok and cancel buttons with nx theme
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2013-04-04 17:19) - PLID 56038 - added bulk caching
		g_propManager.CachePropertiesInBulk("CNYMedicaidSetupDlg-1", propNumber,
			"(Username = '%s') AND ("
			"Name = 'NYMedicaidBox23' OR "
			"Name = 'NYMedicaidBox24M' OR "
			"Name = 'NYMedicaidDoNotFillBox25' OR "
			"Name = 'NYMedicaidBox24H' "
			")",
			_Q(GetCurrentUserName()));

		//BOX 23 (check)
		CheckDlgButton(IDC_CHECK_BOX23, GetRemotePropertyInt("NYMedicaidBox23",0,0,"<None>",true));
		
		//BOX 24 M (check)
		CheckDlgButton(IDC_CHECK_BOX24M, GetRemotePropertyInt("NYMedicaidBox24M",0,0,"<None>",true));

		// (j.jones 2013-04-04 17:25) - PLID 56038 - added ability to disable filling Box 25
		m_checkBox25.SetCheck(GetRemotePropertyInt("NYMedicaidDoNotFillBox25",1,0,"<None>",true) == 1);

		//BOX 24 H (radio)
		switch(GetRemotePropertyInt("NYMedicaidBox24H",0,0,"<None>",true))
		{
		case 0 : CheckDlgButton(IDC_RADIO_ALWAYS_LINE_ITEM, TRUE); break;
		case 1 : CheckDlgButton(IDC_RADIO_ALWAYS_BILL, TRUE); break;
		case 2 : CheckDlgButton(IDC_RADIO_SHOW_POINTERS, TRUE); break;
		case 3 : CheckDlgButton(IDC_RADIO_SHOW_DIAG_OR_POINTERS, TRUE); break;
		}
	
	}NxCatchAll("Error in CNYMedicaidSetupDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNYMedicaidSetupDlg::OnOK() 
{
	try {
		//BOX 23 (check)
		SetRemotePropertyInt("NYMedicaidBox23",IsDlgButtonChecked(IDC_CHECK_BOX23),0,"<None>");

		//BOX 24 M (check)
		SetRemotePropertyInt("NYMedicaidBox24M",IsDlgButtonChecked(IDC_CHECK_BOX24M),0,"<None>");

		// (j.jones 2013-04-04 17:25) - PLID 56038 - added ability to disable filling Box 25
		SetRemotePropertyInt("NYMedicaidDoNotFillBox25", m_checkBox25.GetCheck() ? 1 : 0, 0, "<None>");

		//BOX 24 H (radio)
		long nBox24H = 0;
		if(IsDlgButtonChecked(IDC_RADIO_ALWAYS_BILL))
			nBox24H = 1;
		else if(IsDlgButtonChecked(IDC_RADIO_SHOW_POINTERS))
			nBox24H = 2;
		else if(IsDlgButtonChecked(IDC_RADIO_SHOW_DIAG_OR_POINTERS))
			nBox24H = 3;
		SetRemotePropertyInt("NYMedicaidBox24H",nBox24H,0,"<None>");
		
		CDialog::OnOK();

	}NxCatchAll("Error in CNYMedicaidSetupDlg::OnOK");
}

