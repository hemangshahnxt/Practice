// ViewWebinarDlg.cpp : implementation file
//
// (b.savon 2011-12-19 17:12) - PLID 47074 - Created

#include "stdafx.h"
#include "Practice.h"
#include "ViewWebinarDlg.h"


// CViewWebinarDlg dialog

IMPLEMENT_DYNAMIC(CViewWebinarDlg, CNxDialog)

CViewWebinarDlg::CViewWebinarDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CViewWebinarDlg::IDD, pParent)
{
	m_strTitle = _T("");
	m_strText = _T("");
	m_strName = _T("");
	m_strURLDisplay = _T("");
	m_bDontRemind = FALSE;
	m_ewaAnchor = ewaNone;
}

CViewWebinarDlg::~CViewWebinarDlg()
{
}

void CViewWebinarDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_MESSAGE, m_nxstaticMessage);
	DDX_Control(pDX, IDC_WEBINAR, m_nxstaticWebinar);
}

BOOL CViewWebinarDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		//	Button
		m_btnOk.AutoSet(NXB_OK);
		
		//	Dialog
		if( !m_strTitle.IsEmpty() ){
			SetWindowText(m_strTitle);
		}

		//	Message
		m_nxstaticMessage.SetWindowText(m_strText);

		//	URL
		m_nxstaticWebinar.SetWindowText(m_strURLDisplay);
		m_nxstaticWebinar.SetColor(RGB(0,0,255));

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

int CViewWebinarDlg::DoModal(const CString &strText, const CString &strName, const CString &strTitle /*= ""*/, const CString &strURLDisplay, const WebinarAnchor ewaAnchor)
{
	//	Cache the Property
	g_propManager.CachePropertiesInBulk("ViewWebinar", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'dontshow %s' "
			")", _Q(GetCurrentUserName()), _Q(strName));

	//	Get the property
	long nDontShow = GetRemotePropertyInt("dontshow " + strName, 0, 0, GetCurrentUserName(), true);
	
	//	If they haven't checked 'Don't Remind Me Again', Set the Message properties and Display it
	if(  nDontShow == 0 )
	{
		m_strText = strText;
		m_strName = strName;
		m_strTitle = strTitle;
		m_strURLDisplay = strURLDisplay;
		m_ewaAnchor = ewaAnchor;
	
		return CNxDialog::DoModal();
	}

	return FALSE;
}

BEGIN_MESSAGE_MAP(CViewWebinarDlg, CNxDialog)
	ON_STN_CLICKED(IDC_WEBINAR, &CViewWebinarDlg::OnStnClickedWebinar)
	ON_BN_CLICKED(IDC_DONT_REMIND, &CViewWebinarDlg::OnBnClickedDontRemind)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDOK, &CViewWebinarDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CViewWebinarDlg message handlers

void CViewWebinarDlg::OnStnClickedWebinar()
{
	try{

		ViewWebinar();
	
	}NxCatchAll(__FUNCTION__);
}

void CViewWebinarDlg::ViewWebinar()
{
	// Lifted - Uses Webinar security to auto-login client to webinar session
	CWaitCursor wc;

	const LPCTSTR caryLetters[] = {"00", "ba", "be", "bi", "bo", "bu", "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du", "fa", "fe", "fi", "fo", "fu", "ga", 
			"ge", "gi", "go", "gu", "ha", "he", "hi", "ho", "hu", "ja", "je", "ji", "jo", "ju", "ka", "ke", "ki", "ko", "ku", "la", "le", 
			"li", "lo", "lu", "ma", "me", "mi", "mo", "mu", "na", "ne", "ni", "no", "nu", "pa", "pe", "pi", "po", "pu", "qa", "qe", 
			"qi", "qo", "ra", "re", "ri", "ro", "ru", "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu", "va", "ve", "vi", "vo", 
			"vu", "wa", "we", "wi", "wo", "wu", "xa", "xe", "xi", "xo", "xu", "ya", "ye", "yi", "yo", "yu", "za", "ze", "zi", "zo", "zu",
			"ab", "ac", "ad", "ae", "af", "ag", "ah", "aj", "ak", "al", "am", "an", "ap", "aq", "ar", "as", "at", "av", "aw", "ax", "ay", 
			"az", "eb", "ec", "ed", "ef", "eg", "eh", "ej", "ek", "el", "em", "en", "ep", "eq", "er", "es", "et", "ev", "ew", "ex", "ey", 
			"ez", "ib", "ic", "id", "if", "ig", "ih", "ij", "ik", "il", "im", "in", "ip", "iq", "ir", "is", "it", "iv", "iw", "ix", "iy",
			"iz", "ob", "oc", "od", "of", "og", "oh", "oj", "ok", "ol", "om", "on", "op", "oq", "or", "os", "ot", "ov", "ow", "ox", "oy", 
			"oz", "ub", "uc", "ud", "uf", "ug", "uh", "uj", "uk", "ul", "um", "un", "up", "uq", "ur", "us", "ut", "uv", "uw", "ux", "uy", "uz",
			"1a", "1b", "1c", "1d", "1e", "1f", "1g", "1h", "1i", "1j", "1k", "1l", "1m", "1n", "1o", "1p", "1q", "1r", "1s", "1t", "1u", 
			"1v", "1x", "1y", "1z", "2a", "2b", "2c", "2d", "2e", "2f", "2g", "2h", "2i", "2j", "2k", "2l", "2m", "2n", "2o", "2p", "2q", 
			"2r", "2s", "2t", "2u", "2v", "2w", "2x", "2y", "2z", "3a", "3b", "3c", "3d", "3e", "3f", "3g", "3h", "3i", "3j", "3k", "3l",
			"3m", "3n", "3o", "3p", "3q", "3r", "3s", "3t", "3u", "3v", "3w", "3x", "3y", "3z", "4a", "4b", "4c", "4d", "4e", "4f", "4g", 
			"4h", "4i", "4j", "4k", "4l", "4m", "4n", "4o", "4p", "4q", "4r", "4s", "4t", "4u", "4v", "4w", "4x", "4y", "4z"};

	//get their license key
	long nLicenseKey = g_pLicense->GetLicenseKey();
	CString strLicenseKey;
	strLicenseKey.Format("LoginWithLicenseKey:%li", nLicenseKey);

	CString strEncrypt = "";
	for (int i = 0; i < strLicenseKey.GetLength(); i++) {
		
		long nChar  = ((long)((strLicenseKey.GetAt(i) + 125) ^ 77));
		
		strEncrypt += caryLetters[nChar];
	}
	CString strLink;
	strLink.Format("http://www.nextech.com/index.asp?webinars.asp?%s", strEncrypt);

	//Add the anchor to navigate to (if any)
	CString strAnchor = GetWebinarAnchor();
	if( !strAnchor.IsEmpty() ){
		strLink += strAnchor;
	}

	ShellExecute(NULL, NULL, strLink, NULL, NULL, SW_SHOW);
}

void CViewWebinarDlg::OnBnClickedDontRemind()
{
	try{

		//	Get the value of the check and then save it
		m_bDontRemind = ((CButton*)GetDlgItem(IDC_DONT_REMIND))->GetCheck();

	}NxCatchAll(__FUNCTION__);
}

BOOL CViewWebinarDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try
	{
		//	Get the mouse points
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		//	Get the region of the hyperlink label
		CRect rcWebinar;
		m_nxstaticWebinar.GetWindowRect(rcWebinar);
		ScreenToClient(&rcWebinar);

		//	If our mouse is in the hyperlink region, change our cursor to a HAND
		if(rcWebinar.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CViewWebinarDlg::OnBnClickedOk()
{
	try{

		// Save the property
		SetRemotePropertyInt("dontshow " + m_strName, m_bDontRemind, 0, GetCurrentUserName());
	
		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

CString CViewWebinarDlg::GetWebinarAnchor()
{
	CString strAnchor = "";

	switch( m_ewaAnchor )
	{
		case ewaBasics:
			strAnchor += "#WebinarsTableBasics";
			break;
		case ewaBilling:
			strAnchor += "#WebinarsTableBilling";		
			break;
		case ewaModules:
			strAnchor += "#WebinarsTableModules";		
			break;
		case ewaCommonTasks:
			strAnchor += "#WebinarsTableCommonTasks";
			break;
		case ewaUncommonTasks:
			strAnchor += "#WebinarsTableUncommonTasks";
			break;
		case ewaWhatsNew:
			strAnchor += "#WebinarsTableWhatsNew";
			break;
		case ewaEMR:
			strAnchor += "#WebinarsTableEMR";
			break;
		case ewaMeaningfulUse:
			strAnchor += "#WebinarsTableMeaningfulUse";
			break;
		case ewaNone:
		default:
			break;
	}

	return strAnchor;
}