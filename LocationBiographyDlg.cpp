// LocationBiographyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "LocationBiographyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace RICHTEXTEDITORLib;

/////////////////////////////////////////////////////////////////////////////
// CLocationBiographyDlg dialog

// (d.moore 2007-07-18 17:35) - PLID 14799 - Created a dialog to add biography text to locations.

CLocationBiographyDlg::CLocationBiographyDlg(long nLocationID, CWnd* pParent)
	: CNxDialog(CLocationBiographyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLocationBiographyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nLocationID = nLocationID;
}


void CLocationBiographyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocationBiographyDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocationBiographyDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLocationBiographyDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocationBiographyDlg message handlers

BOOL CLocationBiographyDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);	
		
		m_RichEditCtrl = GetDlgItem(IDC_RICH_TEXT_CTRL)->GetControlUnknown();
		UpdateView();
	
	} NxCatchAll("Error In: CLocationBiographyDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLocationBiographyDlg::OnOK() 
{
	try
	{
		if(!Save()) {
			return;
		}
	
		CNxDialog::OnOK();

	} NxCatchAll("CLocationBiographyDlg::OnOK");
}

void CLocationBiographyDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

void CLocationBiographyDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	CString strBiography = VarString(GetTableField("LocationsT", "Biography", "ID", m_nLocationID), "");
	if(strBiography.IsEmpty()) {
		// Ok, there's no text, let put in some rich text such that the default 
		//  font is Tahoma 10 (which is the font that is being used for NexForms in OfficalContent).
		m_RichEditCtrl->RichText = _bstr_t("[##NXRTFHEADER=<VERSION=2>ENDNXRTFHEADER##]{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\froman\\fprq2\\fcharset0 Tahoma;}}\r\n\\viewkind4\\uc1\\pard\\lang1033\\f0\\fs20\r\n\\par }");
	}
	else {
		m_RichEditCtrl->RichText = _bstr_t(strBiography);
	}
}

BOOL CLocationBiographyDlg::Save()
{
	CString strBiography = (LPCTSTR)m_RichEditCtrl->RichText;
	
	// Even though the biography field is ntext (meaning no character limit),
	//  we are limiting this field to 8000 characters (including rich text stuff) for merging purposed.
	if(strBiography.GetLength() >= 8000) {
		MessageBox("The text you entered is too long. "
			"Please shorten it before saving. "
			"Text is limited to 8000 characters in length.");
		return FALSE;
	}

	long nRecordsAffected;
	ExecuteSqlStd(
		FormatString(
			"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
			"UPDATE LocationsT "
			"SET Biography = '%s' "
			"WHERE ID = %li", 
			_Q(strBiography), 
			m_nLocationID), 
		&nRecordsAffected);
	
	// The only way I could see this happening is if someone else deletes
	//  the provider while this dialog is open, but better safe than sorry.
	if(nRecordsAffected == 0) {
		MessageBox(FormatString("Failed to save biography because the system could not find a location with ID of %li",m_nLocationID));
		return FALSE;
	}

	return TRUE;
}


