// ContactBiographyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ContactBiographyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace RICHTEXTEDITORLib;

/////////////////////////////////////////////////////////////////////////////
// CContactBiographyDlg dialog


CContactBiographyDlg::CContactBiographyDlg(long nProviderID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CContactBiographyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CContactBiographyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nProviderID = nProviderID;
}


void CContactBiographyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactBiographyDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CContactBiographyDlg, CNxDialog)
	//{{AFX_MSG_MAP(CContactBiographyDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactBiographyDlg message handlers

BOOL CContactBiographyDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_RichEditCtrl = GetDlgItem(IDC_RICH_TEXT_CTRL)->GetControlUnknown();

		UpdateView();
	
	}NxCatchAll("CContactBiographyDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CContactBiographyDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	CString strBiography = VarString(GetTableField("ProvidersT", "Biography", "PersonID", m_nProviderID), "");
	if(strBiography.IsEmpty()) {
		// (z.manning, 06/07/2007) - Ok, there's no text, let put in some rich text such that the default 
		// font is Tahoma 10 (which is the font that is being used for NexForms in OfficalContent).
		m_RichEditCtrl->RichText = _bstr_t("[##NXRTFHEADER=<VERSION=2>ENDNXRTFHEADER##]{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\froman\\fprq2\\fcharset0 Tahoma;}}\r\n\\viewkind4\\uc1\\pard\\lang1033\\f0\\fs20\r\n\\par }");
	}
	else {
		m_RichEditCtrl->RichText = _bstr_t(strBiography);
	}
}

void CContactBiographyDlg::OnOK() 
{
	try
	{
		if(!Save()) {
			return;
		}
	
		CNxDialog::OnOK();

	}NxCatchAll("CContactBiographyDlg::OnOK");
}

BOOL CContactBiographyDlg::Save()
{
	CString strBiography = (LPCTSTR)m_RichEditCtrl->RichText;
	
	// (z.manning, 06/12/2007) - PLID 26255 - Even though the biography field is ntext (meaning no character limit),
	// we are limiting this field to 8000 characters (including rich text stuff) for merging purposed.
	if(strBiography.GetLength() >= 8000) {
		MessageBox("The text you entered is too long. Please shorten it before saving.");
		return FALSE;
	}

	long nRecordsAffected;
	ExecuteParamSql(GetRemoteData(), &nRecordsAffected, 
		"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
		"UPDATE ProvidersT SET Biography = {STRING} "
		"WHERE PersonID = {INT}", strBiography, m_nProviderID);
	
	// (z.manning, 06/07/2007) - PLID 23862 - The only way I could see this happening is if someone else deletes
	// the provider while thid dialog is open, but better safe than sorry.
	if(nRecordsAffected == 0) {
		MessageBox(FormatString("Failed to save biography because system could not find provider with ID of %li",m_nProviderID));
		return FALSE;
	}

	return TRUE;
}
