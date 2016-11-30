// ZipEntry.cpp : implementation file
//

#include "stdafx.h"
#include "PracProps.h"
#include "ZipEntry.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CZipEntry dialog

CZipEntry::CZipEntry()
	: CNxDialog(CZipEntry::IDD, NULL)
{
	//{{AFX_DATA_INIT(CZipEntry)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CZipEntry::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZipEntry)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_ZIP, m_nxeditZip);
	DDX_Control(pDX, IDC_CITY, m_nxeditCity);
	DDX_Control(pDX, IDC_STATE, m_nxeditState);
	DDX_Control(pDX, IDC_AREA_CODE, m_nxeditAreaCode);
	DDX_Control(pDX, IDC_TEXT2, m_nxstaticText2);
	DDX_Control(pDX, IDC_TEXT1, m_nxstaticText1);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CZipEntry, CNxDialog)
	//{{AFX_MSG_MAP(CZipEntry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZipEntry message handlers

BOOL CZipEntry::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	static CString str;

	switch (HIWORD(wParam))
	{	case EN_CHANGE:
			switch (nID = LOWORD(wParam))
			{	case IDC_ZIP:
					// (d.moore 2007-04-25 09:13) - PLID 23118 - Capitalize alpha-numeric zip codes
					//  this is required for Canadian postal codes.
					CapitalizeAll(IDC_ZIP);					
					//FormatItem (nID, "#####-nnnn");
					break;
				case IDC_CITY:
					Capitalize(nID);
					break;
				case IDC_STATE:
				{
					CString		value;
					int			x1, 
								x2;
					static bool IsRunning = false;
					
					if (!IsRunning)
					{	IsRunning = true;
						CNxEdit *tmpEdit = (CNxEdit *)GetDlgItem(nID);
						tmpEdit->GetSel (x1, x2);
						GetDlgItemText (nID, value);
						value.MakeUpper();

						SetDlgItemText (nID, value);
						tmpEdit->SetSel (x1, x2);
						IsRunning = false;
					}
					break;
				}
				case IDC_AREA_CODE:
					FormatItem (nID, "nnn");
					break;
			}
		break;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

void CZipEntry::OnOK() 
{
	try{
		_RecordsetPtr rs;

		GetDlgItemText (IDC_ZIP,		m_strZip);
		GetDlgItemText (IDC_CITY,		m_strCity);
		GetDlgItemText (IDC_STATE,		m_strState);
		GetDlgItemText (IDC_AREA_CODE,	m_strAreaCode);

		m_strZip.Replace ("#", "");
		m_strZip.TrimRight();
		if(m_strZip.GetLength()==10)
			m_strZip.TrimRight("-");

		if (m_strZip == "")
		{	MsgBox(RCS(IDS_ZIP_BLANK));
			return;
		}
		
		m_nNewID = NewNumber("ZipCodesT", "UniqueID");
		ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode) SELECT %li, NULL, '%s', '%s', '%s', '%s'",
		m_nNewID, _Q(m_strZip), _Q(m_strCity), _Q(m_strState), m_strAreaCode);

	}NxCatchAll("Error saving Zip Code");

	CDialog::OnOK();
}

void CZipEntry::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL CZipEntry::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	((CNxEdit*)GetDlgItem(IDC_ZIP))->LimitText(10);
	((CNxEdit*)GetDlgItem(IDC_CITY))->LimitText(50);
	((CNxEdit*)GetDlgItem(IDC_STATE))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_AREA_CODE))->LimitText(3);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
