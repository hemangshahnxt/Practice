// UB92DatesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UB92DatesDlg.h"
#include "UB92SetupInfo.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUB92DatesDlg dialog

using namespace ADODB;

CUB92DatesDlg::CUB92DatesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUB92DatesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUB92DatesDlg)
	m_UB92SetupGroupID = -1;
	//}}AFX_DATA_INIT
}


void CUB92DatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUB92DatesDlg)
	DDX_Control(pDX, IDC_BOX80_4DIGIT, m_radioBox80_4);
	DDX_Control(pDX, IDC_BOX80_2DIGIT, m_radioBox80_2);
	DDX_Control(pDX, IDC_BOX86_4DIGIT, m_radioBox86_4);
	DDX_Control(pDX, IDC_BOX86_2DIGIT, m_radioBox86_2);
	DDX_Control(pDX, IDC_BOX6_4DIGIT, m_radioBox6_4);
	DDX_Control(pDX, IDC_BOX6_2DIGIT, m_radioBox6_2);
	DDX_Control(pDX, IDC_BOX45_4DIGIT, m_radioBox45_4);
	DDX_Control(pDX, IDC_BOX45_2DIGIT, m_radioBox45_2);
	DDX_Control(pDX, IDC_BOX32_4DIGIT, m_radioBox32_4);
	DDX_Control(pDX, IDC_BOX32_2DIGIT, m_radioBox32_2);
	DDX_Control(pDX, IDC_BOX17_4DIGIT, m_radioBox17_4);
	DDX_Control(pDX, IDC_BOX17_2DIGIT, m_radioBox17_2);
	DDX_Control(pDX, IDC_BOX14_4DIGIT, m_radioBox14_4);
	DDX_Control(pDX, IDC_BOX14_2DIGIT, m_radioBox14_2);
	DDX_Control(pDX, IDC_FORMAT_LABEL, m_nxstaticFormatLabel);
	DDX_Control(pDX, IDC_BOX14_LABEL, m_nxstaticBox14Label);
	DDX_Control(pDX, IDC_BOX17_LABEL, m_nxstaticBox17Label);
	DDX_Control(pDX, IDC_BOX32_36_LABEL, m_nxstaticBox3236Label);
	DDX_Control(pDX, IDC_BOX80_LABEL, m_nxstaticBox80Label);
	DDX_Control(pDX, IDC_BOX86_LABEL, m_nxstaticBox86Label);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BOX31_USE_LABEL, m_nxstaticBox31UseLabel);
	DDX_Control(pDX, IDC_BOX31_USE_CHARGE_DATE, m_radioBox31UseChargeDate);
	DDX_Control(pDX, IDC_BOX31_USE_ACC_DATE, m_radioBox31UseDateOfCurAcc);
	//}}AFX_DATA_MAP
}

const int BACKGROUND_COLOR  = GetNxColor(GNC_ADMIN, 0);

BEGIN_MESSAGE_MAP(CUB92DatesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUB92DatesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUB92DatesDlg message handlers

BOOL CUB92DatesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_brush.CreateSolidBrush(PaletteColor(BACKGROUND_COLOR));

	m_btnClose.AutoSet(NXB_CLOSE);

	//TES 3/12/2007 - PLID 24993 - Reflect whether we're on the UB92 or the UB04
	if(GetUBFormType() == eUB04) {
		SetWindowText("UB04 Date Configuration");
		SetDlgItemText(IDC_FORMAT_LABEL, "Formats for paper UB04");
		SetDlgItemText(IDC_BOX14_LABEL, "Box 10 (Patient Birthdate)");
		SetDlgItemText(IDC_BOX17_LABEL, "Box 12 (Admission Date)");
		SetDlgItemText(IDC_BOX32_36_LABEL, "Boxes 31 - 36 (Occurrence Dates)");
		SetDlgItemText(IDC_BOX80_LABEL, "Box 74 (Procedure Codes)");
		SetDlgItemText(IDC_BOX86_LABEL, "Creation Date");
	}
	else {
		SetWindowText("UB92 Date Configuration");
		SetDlgItemText(IDC_FORMAT_LABEL, "Formats for paper UB92");
		SetDlgItemText(IDC_BOX14_LABEL, "Box 14 (Patient Birthdate)");
		SetDlgItemText(IDC_BOX17_LABEL, "Box 17 (Admission Date)");
		SetDlgItemText(IDC_BOX80_LABEL, "Boxes 80 - 81 (Procedure Codes)");
		SetDlgItemText(IDC_BOX32_36_LABEL, "Boxes 32 - 36 (Occurrence Dates)");

		// (j.jones 2009-12-22 10:06) - PLID 27131 - hide the controls for UB04 Box 31 usage
		GetDlgItem(IDC_BOX31_USE_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX31_USE_CHARGE_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX31_USE_ACC_DATE)->ShowWindow(SW_HIDE);
	}

	Load();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUB92DatesDlg::SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue)
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

void CUB92DatesDlg::Load()
{
	try
	{
		if(m_UB92SetupGroupID==-1)
			return;

		CUB92SetupInfo UB92Info(m_UB92SetupGroupID);

		//Box 6
		SetRadio (m_radioBox6_4, m_radioBox6_2, UB92Info.Box6 == 4);

		//Box 14
		SetRadio (m_radioBox14_4, m_radioBox14_2, UB92Info.Box14 == 4);

		//Box 17
		SetRadio (m_radioBox17_4, m_radioBox17_2, UB92Info.Box17 == 4);

		//Box 32 - 36
		SetRadio (m_radioBox32_4, m_radioBox32_2, UB92Info.Box32_36 == 4);

		//Box 45
		SetRadio (m_radioBox45_4, m_radioBox45_2, UB92Info.Box45 == 4);

		//Box 80 - 81
		SetRadio (m_radioBox80_4, m_radioBox80_2, UB92Info.Box80_81 == 4);

		//Box 86
		SetRadio (m_radioBox86_4, m_radioBox86_2, UB92Info.Box86 == 4);

		// (j.jones 2009-12-22 10:21) - PLID 27131 - added UB04UseBox31Date
		//0 - use charge date, 1 - use acc. date
		SetRadio (m_radioBox31UseChargeDate, m_radioBox31UseDateOfCurAcc, UB92Info.UB04UseBox31Date == 0);
		
	}NxCatchAll("Error in Load()");
}

BOOL CUB92DatesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD	nID;
	CString box;
	long	value;
	_RecordsetPtr rs;

	switch (HIWORD(wParam))
	{	case BN_CLICKED:
		{	switch (nID = LOWORD(wParam))
			{
				case IDC_BOX6_2DIGIT:
				case IDC_BOX14_2DIGIT:
				case IDC_BOX17_2DIGIT:
				case IDC_BOX32_2DIGIT:
				case IDC_BOX45_2DIGIT:
				case IDC_BOX80_2DIGIT:
				case IDC_BOX86_2DIGIT:
					value = 2;
					break;				
				case IDC_BOX6_4DIGIT:
				case IDC_BOX14_4DIGIT:
				case IDC_BOX17_4DIGIT:
				case IDC_BOX32_4DIGIT:
				case IDC_BOX45_4DIGIT:
				case IDC_BOX80_4DIGIT:
				case IDC_BOX86_4DIGIT:
					value = 4;
					break;
				// (j.jones 2009-12-22 10:06) - PLID 27131 - added controls for UB04 Box 31 usage
				case IDC_BOX31_USE_CHARGE_DATE:
					value = 0;
					break;
				case IDC_BOX31_USE_ACC_DATE:
					value = 1;
					break;
				default: return CDialog::OnCommand(wParam, lParam);//handles anything else here
			}
			switch (nID = LOWORD(wParam))
			{
				case IDC_BOX6_2DIGIT:
				case IDC_BOX6_4DIGIT:
					box = "Box6";
					break;

				case IDC_BOX14_2DIGIT:
				case IDC_BOX14_4DIGIT:
					box = "Box14";
					break;

				case IDC_BOX17_2DIGIT:
				case IDC_BOX17_4DIGIT:
					box = "Box17";
					break;

				case IDC_BOX32_2DIGIT:
				case IDC_BOX32_4DIGIT:
					box = "Box32_36";
					break;

				case IDC_BOX45_2DIGIT:
				case IDC_BOX45_4DIGIT:
					box = "Box45";
					break;

				case IDC_BOX80_2DIGIT:
				case IDC_BOX80_4DIGIT:
					box = "Box80_81";
					break;

				case IDC_BOX86_2DIGIT:
				case IDC_BOX86_4DIGIT:
					box = "Box86";
					break;

				// (j.jones 2009-12-22 10:06) - PLID 27131 - added controls for UB04 Box 31 usage
				case IDC_BOX31_USE_CHARGE_DATE:
				case IDC_BOX31_USE_ACC_DATE:
					box = "UB04UseBox31Date";
					break;
			}
			try
			{
				if(m_UB92SetupGroupID==-1)
					return CDialog::OnCommand(wParam, lParam);

				UpdateTable(box,value);

			}NxCatchAll("Error saving date configuration.");
			break;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CUB92DatesDlg::UpdateTable(CString BoxName, long data)
{
	if(m_UB92SetupGroupID == -1)
		return;

	ExecuteSql("UPDATE UB92SetupT SET %s = %li WHERE ID = %li", BoxName, data, m_UB92SetupGroupID);
}
