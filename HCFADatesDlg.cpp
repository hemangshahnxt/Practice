// HCFADatesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HCFADatesDlg.h"
#include "HCFASetupInfo.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

//In honor of Chris Styduhar - "Do the dates, yo!"

/////////////////////////////////////////////////////////////////////////////
// CHCFADatesDlg dialog


CHCFADatesDlg::CHCFADatesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHCFADatesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHCFADatesDlg)
	m_HCFASetupGroupID = -1;
	//}}AFX_DATA_INIT
}


void CHCFADatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHCFADatesDlg)
	DDX_Control(pDX, IDC_BOX9B_WIDE, m_checkBox9B_Wide);
	DDX_Control(pDX, IDC_BOX9B_4DIGIT, m_radioBox9B_4);
	DDX_Control(pDX, IDC_BOX9B_2DIGIT, m_radioBox9B_2);
	DDX_Control(pDX, IDC_BOX31_WIDE, m_checkBox31_Wide);
	DDX_Control(pDX, IDC_BOX31_4DIGIT, m_radioBox31_4);
	DDX_Control(pDX, IDC_BOX31_2DIGIT, m_radioBox31_2);
	DDX_Control(pDX, IDC_BOX3_WIDE, m_checkBox3_Wide);
	DDX_Control(pDX, IDC_BOX3_4DIGIT, m_radioBox3_4);
	DDX_Control(pDX, IDC_BOX3_2DIGIT, m_radioBox3_2);
	DDX_Control(pDX, IDC_BOX24T_WIDE, m_checkBox24T_Wide);
	DDX_Control(pDX, IDC_BOX24T_4DIGIT, m_radioBox24T_4);
	DDX_Control(pDX, IDC_BOX24T_2DIGIT, m_radioBox24T_2);
	DDX_Control(pDX, IDC_BOX24F_WIDE, m_checkBox24F_Wide);
	DDX_Control(pDX, IDC_BOX24F_4DIGIT, m_radioBox24F_4);
	DDX_Control(pDX, IDC_BOX24F_2DIGIT, m_radioBox24F_2);
	DDX_Control(pDX, IDC_BOX18_WIDE, m_checkBox18_Wide);
	DDX_Control(pDX, IDC_BOX18_4DIGIT, m_radioBox18_4);
	DDX_Control(pDX, IDC_BOX18_2DIGIT, m_radioBox18_2);
	DDX_Control(pDX, IDC_BOX16_WIDE, m_checkBox16_Wide);
	DDX_Control(pDX, IDC_BOX16_4DIGIT, m_radioBox16_4);
	DDX_Control(pDX, IDC_BOX16_2DIGIT, m_radioBox16_2);
	DDX_Control(pDX, IDC_BOX15_WIDE, m_checkBox15_Wide);
	DDX_Control(pDX, IDC_BOX15_4DIGIT, m_radioBox15_4);
	DDX_Control(pDX, IDC_BOX15_2DIGIT, m_radioBox15_2);
	DDX_Control(pDX, IDC_BOX14_WIDE, m_checkBox14_Wide);
	DDX_Control(pDX, IDC_BOX14_4DIGIT, m_radioBox14_4);
	DDX_Control(pDX, IDC_BOX14_2DIGIT, m_radioBox14_2);
	DDX_Control(pDX, IDC_BOX12_WIDE, m_checkBox12_Wide);
	DDX_Control(pDX, IDC_BOX12_4DIGIT, m_radioBox12_4);
	DDX_Control(pDX, IDC_BOX12_2DIGIT, m_radioBox12_2);
	DDX_Control(pDX, IDC_BOX11A_WIDE, m_checkBox11A_Wide);
	DDX_Control(pDX, IDC_BOX11A_4DIGIT, m_radioBox11A_4);
	DDX_Control(pDX, IDC_BOX11A_2DIGIT, m_radioBox11A_2);
	DDX_Control(pDX, IDC_RADIO_BOX_12_PRINT_DATE, m_radioBox12UsePrintDate);
	DDX_Control(pDX, IDC_RADIO_BOX_12_BILL_DATE, m_radioBox12UseBillDate);
	DDX_Control(pDX, IDC_RADIO_BOX_31_PRINT_DATE, m_radioBox31UsePrintDate);
	DDX_Control(pDX, IDC_RADIO_BOX_31_BILL_DATE, m_radioBox31UseBillDate);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}

const int BACKGROUND_COLOR  = GetNxColor(GNC_ADMIN, 0);

BEGIN_MESSAGE_MAP(CHCFADatesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHCFADatesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHCFADatesDlg message handlers

BOOL CHCFADatesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29680 - Set button styles
	m_btnClose.AutoSet(NXB_CLOSE);
	
	m_Box11a = BindNxDataListCtrl(this,IDC_BOX11A,GetRemoteData(),false);
	m_Box9b = BindNxDataListCtrl(this,IDC_BOX9B,GetRemoteData(),false);
	m_Box3 = BindNxDataListCtrl(this,IDC_BOX3,GetRemoteData(),false);

	BuildDateCombos();

	Load();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHCFADatesDlg::SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue)
{//Eliminates a few lines
	if (isTrue)
	{	radioTrue.SetCheck(true);
		radioFalse.SetCheck(false);
	}
	else 
	{	radioTrue.SetCheck(false);
		radioFalse.SetCheck(true);
	}
}

void CHCFADatesDlg::Load()
{
	try
	{
		if(m_HCFASetupGroupID==-1)
			return;

		CHCFASetupInfo HCFAInfo(m_HCFASetupGroupID);

		//Box3
		SetRadio (m_radioBox3_4, m_radioBox3_2, HCFAInfo.Box3 == 4);

		//Box9b
		SetRadio (m_radioBox9B_4, m_radioBox9B_2, HCFAInfo.Box9b == 4);

		//Box11a
		SetRadio (m_radioBox11A_4, m_radioBox11A_2, HCFAInfo.Box11a == 4);

		//Box31
		SetRadio (m_radioBox31_4, m_radioBox31_2, HCFAInfo.Box31 == 4);

		//Box24To
		SetRadio (m_radioBox24T_4, m_radioBox24T_2, HCFAInfo.Box24To == 4);

		//Box24From
		SetRadio (m_radioBox24F_4, m_radioBox24F_2, HCFAInfo.Box24From == 4);
	
		//Box18
		SetRadio (m_radioBox18_4, m_radioBox18_2, HCFAInfo.Box18 == 4);

		//Box16
		SetRadio (m_radioBox16_4, m_radioBox16_2, HCFAInfo.Box16 == 4);

		//Box14
		SetRadio (m_radioBox14_4, m_radioBox14_2, HCFAInfo.Box14 == 4);

		//Box15
		SetRadio (m_radioBox15_4, m_radioBox15_2, HCFAInfo.Box15 == 4);

		//Box12
		SetRadio (m_radioBox12_4, m_radioBox12_2, HCFAInfo.Box12 == 4);

		//Box3Wide
		m_checkBox3_Wide.SetCheck(HCFAInfo.Box3Wide);

		//Box9bWide
		m_checkBox9B_Wide.SetCheck(HCFAInfo.Box9bWide);

		//Box11aWide
		m_checkBox11A_Wide.SetCheck(HCFAInfo.Box11aWide);

		//Box31Wide
		m_checkBox31_Wide.SetCheck(HCFAInfo.Box31Wide);

		//Box24FWide
		m_checkBox24F_Wide.SetCheck(HCFAInfo.Box24FWide);

		//Box24TWide
		m_checkBox24T_Wide.SetCheck(HCFAInfo.Box24TWide);

		//Box16Wide
		m_checkBox16_Wide.SetCheck(HCFAInfo.Box16Wide);

		//Box18Wide
		m_checkBox18_Wide.SetCheck(HCFAInfo.Box18Wide);

		//Box14Wide
		m_checkBox14_Wide.SetCheck(HCFAInfo.Box14Wide);

		//Box15Wide
		m_checkBox15_Wide.SetCheck(HCFAInfo.Box15Wide);

		//Box12Wide
		m_checkBox12_Wide.SetCheck(HCFAInfo.Box12Wide);
	
		//Box3E
		m_Box3->SetSelByColumn(0, HCFAInfo.Box3E);

		//Box11aE
		m_Box11a->SetSelByColumn(0, HCFAInfo.Box11aE);

		//Box9bE
		m_Box9b->SetSelByColumn(0, HCFAInfo.Box9bE);

		//Box12UseDate
		SetRadio (m_radioBox12UsePrintDate, m_radioBox12UseBillDate, HCFAInfo.Box12UseDate == 0);

		//Box31UseDate
		SetRadio (m_radioBox31UsePrintDate, m_radioBox31UseBillDate, HCFAInfo.Box31UseDate == 0);

	}NxCatchAll("Error in Load()");
}

void CHCFADatesDlg::OnOK() 
{	
	CDialog::OnOK();
}

BOOL CHCFADatesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD	nID;
	CString box;
	long	value;
	_RecordsetPtr rs;

	switch (HIWORD(wParam))
	{	case BN_CLICKED:
		{	switch (nID = LOWORD(wParam))
			{
				case IDC_RADIO_BOX_12_PRINT_DATE:
				case IDC_RADIO_BOX_31_PRINT_DATE:
					value = 0;
					break;
				case IDC_RADIO_BOX_12_BILL_DATE:
				case IDC_RADIO_BOX_31_BILL_DATE:
					value = 1;
					break;
				case IDC_BOX9B_2DIGIT:
				case IDC_BOX31_2DIGIT:
				case IDC_BOX3_2DIGIT:
				case IDC_BOX24T_2DIGIT:
				case IDC_BOX24F_2DIGIT:
				case IDC_BOX18_2DIGIT:
				case IDC_BOX11A_2DIGIT:
				case IDC_BOX16_2DIGIT:
				case IDC_BOX15_2DIGIT:
				case IDC_BOX14_2DIGIT:
				case IDC_BOX12_2DIGIT:
					value = 2;
					break;
				case IDC_BOX9B_4DIGIT:
				case IDC_BOX31_4DIGIT:
				case IDC_BOX3_4DIGIT:
				case IDC_BOX24T_4DIGIT:
				case IDC_BOX24F_4DIGIT:
				case IDC_BOX18_4DIGIT:
				case IDC_BOX11A_4DIGIT:
				case IDC_BOX16_4DIGIT:
				case IDC_BOX15_4DIGIT:
				case IDC_BOX14_4DIGIT:
				case IDC_BOX12_4DIGIT:
					value = 4;
					break;
				case IDC_BOX9B_WIDE:
				case IDC_BOX31_WIDE:
				case IDC_BOX3_WIDE:
				case IDC_BOX24T_WIDE:
				case IDC_BOX24F_WIDE:
				case IDC_BOX18_WIDE:
				case IDC_BOX11A_WIDE:
				case IDC_BOX16_WIDE:
				case IDC_BOX15_WIDE:
				case IDC_BOX14_WIDE:
				case IDC_BOX12_WIDE:
					if(((NxButton*)GetDlgItem(nID))->GetCheck())
						value = 1;
					else
						value = 0;
					break;
				default: return CDialog::OnCommand(wParam, lParam);//handles anything else here
			}
			switch (nID = LOWORD(wParam))
			{
				case IDC_RADIO_BOX_12_PRINT_DATE:
				case IDC_RADIO_BOX_12_BILL_DATE:
					box = "Box12UseDate";
					break;
				case IDC_RADIO_BOX_31_PRINT_DATE:
				case IDC_RADIO_BOX_31_BILL_DATE:
					box = "Box31UseDate";
					break;

				case IDC_BOX9B_2DIGIT:
				case IDC_BOX9B_4DIGIT:
					box = "Box9b";
					break;

				case IDC_BOX31_2DIGIT:
				case IDC_BOX31_4DIGIT:
					box = "Box31";
					break;

				case IDC_BOX3_2DIGIT:
				case IDC_BOX3_4DIGIT:
					box = "Box3";
					break;

				case IDC_BOX24T_2DIGIT:
				case IDC_BOX24T_4DIGIT:
					box = "Box24To";
					break;

				case IDC_BOX24F_2DIGIT:
				case IDC_BOX24F_4DIGIT:
					box = "Box24From";
					break;

				case IDC_BOX18_2DIGIT:
				case IDC_BOX18_4DIGIT:
					box = "Box18";
					break;

				case IDC_BOX11A_2DIGIT:
				case IDC_BOX11A_4DIGIT:
					box = "Box11a";
					break;

				case IDC_BOX16_2DIGIT:
				case IDC_BOX16_4DIGIT:
					box = "Box16";
					break;

				case IDC_BOX15_2DIGIT:
				case IDC_BOX15_4DIGIT:
					box = "Box15";
					break;

				case IDC_BOX14_2DIGIT:
				case IDC_BOX14_4DIGIT:
					box = "Box14";
					break;

				case IDC_BOX12_2DIGIT:
				case IDC_BOX12_4DIGIT:
					box = "Box12";
					break;

				case IDC_BOX9B_WIDE:
					box = "Box9bWide";
					break;

				case IDC_BOX31_WIDE:
					box = "Box31Wide";
					break;

				case IDC_BOX3_WIDE:
					box = "Box3Wide";
					break;

				case IDC_BOX24T_WIDE:
					box = "Box24TWide";
					break;

				case IDC_BOX24F_WIDE:
					box = "Box24FWide";
					break;

				case IDC_BOX18_WIDE:
					box = "Box18Wide";
					break;

				case IDC_BOX11A_WIDE:
					box = "Box11aWide";
					break;

				case IDC_BOX16_WIDE:
					box = "Box16Wide";
					break;

				case IDC_BOX15_WIDE:
					box = "Box15Wide";
					break;

				case IDC_BOX14_WIDE:
					box = "Box14Wide";
					break;

				case IDC_BOX12_WIDE:
					box = "Box12Wide";
					break;
			}
			try
			{
				if(m_HCFASetupGroupID==-1)
					return CDialog::OnCommand(wParam, lParam);

				UpdateTable(box,value);

			}NxCatchAll("Error saving date configuration.");
			break;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

BEGIN_EVENTSINK_MAP(CHCFADatesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHCFADatesDlg)
	ON_EVENT(CHCFADatesDlg, IDC_BOX3, 16 /* SelChosen */, OnSelChosenBox3, VTS_I4)
	ON_EVENT(CHCFADatesDlg, IDC_BOX9B, 16 /* SelChosen */, OnSelChosenBox9b, VTS_I4)
	ON_EVENT(CHCFADatesDlg, IDC_BOX11A, 16 /* SelChosen */, OnSelChosenBox11a, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CHCFADatesDlg::BuildDateCombos()
{
	IRowSettingsPtr pRow11a, pRow9b, pRow3;
	_variant_t var;

	pRow11a = m_Box11a->GetRow(-1);
	pRow9b = m_Box9b->GetRow(-1);
	pRow3 = m_Box3->GetRow(-1);
	var = (long)1;
	pRow11a->PutValue(0,var);
	pRow9b->PutValue(0,var);
	pRow3->PutValue(0,var);
	var = _bstr_t("yyyy/mm/dd");
	pRow11a->PutValue(1,var);
	pRow9b->PutValue(1,var);
	pRow3->PutValue(1,var);
	m_Box11a->AddRow(pRow11a);
	m_Box9b->AddRow(pRow9b);
	m_Box3->AddRow(pRow3);

	pRow11a = m_Box11a->GetRow(-1);
	pRow9b = m_Box9b->GetRow(-1);
	pRow3 = m_Box3->GetRow(-1);
	var = (long)2;
	pRow11a->PutValue(0,var);
	pRow9b->PutValue(0,var);
	pRow3->PutValue(0,var);
	var = _bstr_t("mm/dd/yyyy");
	pRow11a->PutValue(1,var);
	pRow9b->PutValue(1,var);
	pRow3->PutValue(1,var);
	m_Box11a->AddRow(pRow11a);
	m_Box9b->AddRow(pRow9b);
	m_Box3->AddRow(pRow3);
}

void CHCFADatesDlg::OnSelChosenBox3(long nRow) 
{
	if(m_HCFASetupGroupID==-1 || nRow == -1)
		return;

	try {

		UpdateTable("Box3E",m_Box3->GetValue(nRow,0).lVal);

	} NxCatchAll("Error in OnSelChosenBox3");
}

void CHCFADatesDlg::OnSelChosenBox9b(long nRow) 
{
	if(m_HCFASetupGroupID == -1 || nRow == -1)
		return;

	try {

		UpdateTable("Box9bE",m_Box9b->GetValue(nRow,0).lVal);

	} NxCatchAll("Error in OnSelChosenBox9b");
}

void CHCFADatesDlg::OnSelChosenBox11a(long nRow) 
{
	if(m_HCFASetupGroupID == -1 || nRow == -1)
		return;

	try {

		UpdateTable("Box11aE",m_Box11a->GetValue(nRow,0).lVal);

	} NxCatchAll("Error in OnSelChosenBox11a");
}

void CHCFADatesDlg::UpdateTable(CString BoxName, long data)
{
	if(m_HCFASetupGroupID == -1)
		return;

	ExecuteSql("UPDATE HCFASetupT SET %s = %li WHERE ID = %li", BoxName, data, m_HCFASetupGroupID);
}