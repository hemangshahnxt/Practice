// MarketBaselineConfig.cpp : implementation file
//

#include "stdafx.h"
#include "MarketBaselineConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketBaselineConfig dialog


CMarketBaselineConfig::CMarketBaselineConfig(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMarketBaselineConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketBaselineConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CMarketBaselineConfig::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketBaselineConfig)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EDIT_CONSULTSPERMONTH, m_nxeditEditConsultspermonth);
	DDX_Control(pDX, IDC_EDIT_SURGERIESPERMONTH, m_nxeditEditSurgeriespermonth);
	DDX_Control(pDX, IDC_EDIT_CLOSURERATIO, m_nxeditEditClosureratio);
	DDX_Control(pDX, IDC_EDIT_CLOSUREDURATION, m_nxeditEditClosureduration);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketBaselineConfig, CNxDialog)
	//{{AFX_MSG_MAP(CMarketBaselineConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketBaselineConfig message handlers

BOOL CMarketBaselineConfig::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 12:04) - PLID 29824 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		Load();
	}
	NxCatchAll("Error in CMarketBaselineConfig::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarketBaselineConfig::OnOK()
{
	// (c.haag 2004-04-16 10:22) - Don't save closure rates > 100%
	int iRate = GetDlgItemInt(IDC_EDIT_CLOSURERATIO);
	if (iRate > 100)
	{
		MsgBox("You must enter a Consultations Closure Ratio at or under 100%% before saving your goals.");
		GetDlgItem(IDC_EDIT_CLOSURERATIO)->SetFocus();
		return;
	}

	if (!Save()) return;
	CNxDialog::OnOK();
}

void CMarketBaselineConfig::OnCancel()
{
	CNxDialog::OnCancel();
}

void CMarketBaselineConfig::Load()
{
	SetDlgItemInt(IDC_EDIT_CONSULTSPERMONTH, GetRemotePropertyInt("MarketConsultsGoal", 27));
	SetDlgItemInt(IDC_EDIT_SURGERIESPERMONTH, GetRemotePropertyInt("MarketSurgeriesGoal", 18));
	SetDlgItemInt(IDC_EDIT_CLOSURERATIO, GetRemotePropertyInt("MarketClosureRateGoal", 55));
	SetDlgItemInt(IDC_EDIT_CLOSUREDURATION, GetRemotePropertyInt("MarketClosureTimeSpanGoal", 35));
}

BOOL CMarketBaselineConfig::Save()
{
	CString str;
	GetDlgItemText(IDC_EDIT_CONSULTSPERMONTH, str);
	if (str.IsEmpty())
	{
		MsgBox("Please enter a goal for the number of consults per month.");
		GetDlgItem(IDC_EDIT_CONSULTSPERMONTH)->SetFocus();
		return FALSE;
	}
	GetDlgItemText(IDC_EDIT_SURGERIESPERMONTH, str);
	if (str.IsEmpty())
	{
		MsgBox("Please enter a goal for the number of surgeries per month.");
		GetDlgItem(IDC_EDIT_SURGERIESPERMONTH)->SetFocus();
		return FALSE;
	}
	GetDlgItemText(IDC_EDIT_CLOSURERATIO, str);
	if (str.IsEmpty())
	{
		MsgBox("Please enter a goal for the consultations closure ratio.");
		GetDlgItem(IDC_EDIT_CLOSURERATIO)->SetFocus();
		return FALSE;
	}
	GetDlgItemText(IDC_EDIT_CLOSUREDURATION, str);
	if (str.IsEmpty())
	{
		MsgBox("Please enter a goal for the number of days from closure to surgery.");
		GetDlgItem(IDC_EDIT_CLOSUREDURATION)->SetFocus();
		return FALSE;
	}

	try {
		SetRemotePropertyInt("MarketConsultsGoal", GetDlgItemInt(IDC_EDIT_CONSULTSPERMONTH));
		SetRemotePropertyInt("MarketSurgeriesGoal", GetDlgItemInt(IDC_EDIT_SURGERIESPERMONTH));
		SetRemotePropertyInt("MarketClosureRateGoal", GetDlgItemInt(IDC_EDIT_CLOSURERATIO));
		SetRemotePropertyInt("MarketClosureTimeSpanGoal", GetDlgItemInt(IDC_EDIT_CLOSUREDURATION));
		return TRUE;
	}
	NxCatchAll("Error saving baseline goals");
	return FALSE;
}
