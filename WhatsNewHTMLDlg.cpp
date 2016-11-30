// WhatsNewHTMLDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "practicerc.h"
#include "WhatsNewHTMLDlg.h"
#include "RegUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
//DRT 4/26/2006 - PLID 20301 - Changed to read from the client path instead of the server path
#define FILE_CHANGES_HTML		(NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "changes.html")
//Macro taken from SizeableTextDlg.cpp
// if a > b+m, return 1; if a < b-m, return -1; else return 0
#define COMPARE_DATETIME_INEXACT(a, b, marginseconds) \
	(((a) > ((b) + COleDateTimeSpan(0, 0, 0, marginseconds))) ? 1 : \
	 ((a) < ((b) - COleDateTimeSpan(0, 0, 0, marginseconds))) ? -1 : 0)

/////////////////////////////////////////////////////////////////////////////
// CWhatsNewHTMLDlg dialog

CWhatsNewHTMLDlg::CWhatsNewHTMLDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CWhatsNewHTMLDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWhatsNewHTMLDlg)
	//}}AFX_DATA_INIT

	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Check for existence of the file
	m_bFileExists = false;  //We set this in NeedToDisplay()
	//Set the override time to invalid
	m_dtOverrideTime.SetStatus(COleDateTime::invalid);
}

CWhatsNewHTMLDlg::~CWhatsNewHTMLDlg()
{
}

void CWhatsNewHTMLDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWhatsNewHTMLDlg)
	DDX_Control(pDX, IDC_DONTSHOW_CHECK, m_btnDontShow);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_PRINT_BTN, m_btnPrint);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWhatsNewHTMLDlg, CNxDialog)
	//{{AFX_MSG_MAP(CWhatsNewHTMLDlg)
	ON_BN_CLICKED(IDC_PRINT_BTN, OnPrintBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhatsNewHTMLDlg message handlers

BOOL CWhatsNewHTMLDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (c.haag 2008-04-25 12:32) - PLID 29793 - NxIconified buttons
	m_btnOK.AutoSet(NXB_CLOSE);
	m_btnPrint.AutoSet(NXB_PRINT);
	
	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Initialize COM objects.
	CoInitialize(NULL);
	//Set the pointer to the browser control
	m_pBrowser = GetDlgItem(IDC_BROWSER)->GetControlUnknown();
	//Determine if we know the file exists
	if(!m_bFileExists)
		ThrowNxException("Could not access file %s!", FILE_CHANGES_HTML);
	else {
		//Tell the WebBrowser object which file it should display.
		m_pBrowser->Navigate(_bstr_t(FILE_CHANGES_HTML), NULL, NULL, NULL, NULL);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWhatsNewHTMLDlg::DoModeless(BOOL bForceWhatsNew /*= FALSE*/)
{
	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Determine if the user has chosen to hide the changes.
	// (a.vengrofski 2009-11-11 18:02) - PLID <36104> - Added override for NeedToDisplay.
	if (NeedToDisplay() || bForceWhatsNew) {
		//Display the changes
		//Create(m_lpszTemplateName, m_pParentWnd) //From SizeableTxtDlg.cpp
		//if (Create(IDD_WHATS_NEW_HTML_DLG, NULL)) {  //Keeps the Whats New box in front
		//if (Create(IDD_WHATS_NEW_HTML_DLG, m_pParentWnd)) { //MSDN suggestion; Whats New box hides behind Practice
		if (Create(m_lpszTemplateName, m_pParentWnd)) { 
			if (bForceWhatsNew){
				GetDlgItem(IDC_DONTSHOW_CHECK)->EnableWindow(FALSE);
			}
			ShowWindow(SW_SHOW);
			return TRUE;
		} else {
			//Failure creating dialog
			ThrowNxException("Could not create WhatsNewHTML dialog!");
			return FALSE;
		}
	} else {  
		//We don't need to show the changes, so just return
		return FALSE;
	}
	return TRUE;
}

void CWhatsNewHTMLDlg::OnCancel() 
{
	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Taken from CSizeableTextDlg::OnCancel()
	// Users tend to think that hitting escape when there's only an OK button is the same as clicking OK
	if (GetDlgItem(IDCANCEL)->GetSafeHwnd() == NULL) {
		OnOK();
	} else {
		CDialog::OnCancel();
	}

	// (a.walling 2011-08-10 14:30) - PLID 44972 - Destroy ourselves since we are modeless; this frees the memory used by the MSHTML object
	DestroyWindow();
}

void CWhatsNewHTMLDlg::OnOK() 
{
	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Determine if the "Don't Show again" checkbox is checked
	if(IsDlgButtonChecked(IDC_DONTSHOW_CHECK)) {
		//The override date/time should have been set by NeedToDisplay(), but we need to make sure it is valid
		if(m_dtOverrideTime.GetStatus() == COleDateTime::invalid)
			ThrowNxException("Could not set override date and time!");
		//Store the settings
		SetRemotePropertyDateTime("timestamp ShowChanges", m_dtOverrideTime, 0, GetCurrentUserName());
	}
	CDialog::OnOK();

	// (a.walling 2011-08-10 14:30) - PLID 44972 - Destroy ourselves since we are modeless; this frees the memory used by the MSHTML object
	DestroyWindow();
}

void CWhatsNewHTMLDlg::OnPrintBtn() 
{
	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Fortunately, the WebBrowser2 object includes a method for printing its contents.
	//The code below prints the file displayed in the object.
	m_pBrowser->ExecWB(OLECMDID_PRINT, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
}

bool CWhatsNewHTMLDlg::NeedToDisplay()
{
	//m.hancock - 2/15/2006 - PLID 19312 - Present the list of changes in an HTML file.
	//Determine if the changes should be shown.

	//Check for existence of the file
	m_bFileExists = DoesExist(FILE_CHANGES_HTML);

	//Determine the last time we showed the file.  If the user has never said "Don't show me", a date/time for 1901 is returned.
	COleDateTime dtLast = GetRemotePropertyDateTime("timestamp ShowChanges", &COleDateTime(1901, 1, 1, 1, 1, 1), 0, GetCurrentUserName(), false);

	//If the file does not exist and the user has stated they do not wish to see changes, then just return.
	COleDateTime dtDefault = COleDateTime(1901, 1, 1, 1, 1, 1);
	if(!m_bFileExists && (dtLast != dtDefault))
		return false;
	else if(!m_bFileExists) {
		//The file does not exist and we should be showing it, so throw an exception.
		ThrowNxException("Could not access file %s!", FILE_CHANGES_HTML);
	}

	//Get the file's modification date and time and display an error if we can't access it
	COleDateTime dtFileTime;
	if (!GetFileSystemTime(FILE_CHANGES_HTML, dtFileTime)) {
		ThrowNxException("Could not get date attributes for %s!", FILE_CHANGES_HTML);
		return false;
	}

	//Store the override time for use in OnOK().  For this, we use the date of the file.
	m_dtOverrideTime = dtFileTime;

	//If "last time" is >= the file's time, then we've shown it before and the user decided not to see it again
	if (COMPARE_DATETIME_INEXACT(dtLast, dtFileTime, 5) >= 0) {
		//User doesn't want to see the file
		return false;
	} else {
		//"Last time" was before the file's time which means we have yet to show it, let's show it now
		return true;
	}
}
