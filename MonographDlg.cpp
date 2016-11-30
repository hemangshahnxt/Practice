// MonographDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "MonographDlg.h"

// (j.fouts 2012-08-10 09:57) - PLID 52090 - Created.

// CMonographDlg dialog

IMPLEMENT_DYNAMIC(CMonographDlg, CNxDialog)

CMonographDlg::CMonographDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMonographDlg::IDD, pParent, "MonographDlg")
{
	// (j.fouts 2012-08-10 09:58) - PLID 52090 - Default the HTML to blank
	m_strHTML = "";
	m_pBrowser = NULL;
	m_bShowDeveloper = false;
}

CMonographDlg::~CMonographDlg()
{
}

void CMonographDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRINT_MONOGRAPH, m_btnPrint);
	DDX_Control(pDX, IDC_PRINT_PREVIEW_MONOGRAPH, m_btnPrintPreview);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_MONOGRAPH_COLOR, m_bkg);
}

BOOL CMonographDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		SetTitleBarIcon(IDI_MONOGRAPH);

		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);
		m_btnOk.AutoSet(NXB_CLOSE);
		// (j.fouts 2013-02-13 11:32) - PLID 55146 - Set the color
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// (j.fouts 2012-08-10 09:58) - PLID 52090 - Display the HTML monograph
		CWnd* pBrowserWnd = GetDlgItem(IDC_MONOGRAPH_BROWSER);
		if(pBrowserWnd)
		{
			m_pBrowser = pBrowserWnd->GetControlUnknown();
			if(!m_pBrowser)
			{
				return FALSE;
			}

			//Start with a blank page
			m_pBrowser->Navigate(_bstr_t("about:blank"), NULL, NULL, NULL, NULL);
		}

		//TES 11/21/2013 - PLID 59399 - Added a label that's only shown for drug-drug interactions
		if(m_bShowDeveloper) {
			extern CPracticeApp theApp;
			GetDlgItem(IDC_DEVELOPER_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DEVELOPER_LABEL)->SetFont(&theApp.m_boldFont);
		}
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.fouts 2012-08-29 09:59) - PLID 51719 - Load the html into the browser
void CMonographDlg::LoadCustomHtml()
{
	if(!m_pBrowser)
	{
		return;
	}

	IDispatch* pDoc;
	if(!SUCCEEDED(m_pBrowser->get_Document(&pDoc)))
	{
		//Failed to get the doc
		return;
	}

	CComPtr<IHTMLDocument2> pDoc2;
	pDoc2.Attach((IHTMLDocument2*)pDoc);

	if (!pDoc2)
	{
		//Failed to attach the doc
		return;
	}

	SAFEARRAY* saHTML = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if(!saHTML)
	{
		//Failed to create the safearray
		return;
	}

	VARIANT* v;
	if(SUCCEEDED(SafeArrayAccessData(saHTML, (LPVOID*)&v)))
	{
		//Create a variant to give to the HtmlDoc, the safe array should handle the clean up of the BSTR
		v->vt = VT_BSTR;
		v->bstrVal = m_strHTML.AllocSysString();

		if(SUCCEEDED(SafeArrayUnaccessData(saHTML)))
		{
			//Write our html to the doc so it can be displayed
			pDoc2->write(saHTML);
			pDoc2->close();
		}
	}

	if(saHTML)
	{
		//free our memory
		SafeArrayDestroy(saHTML);
	}
}

BEGIN_MESSAGE_MAP(CMonographDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PRINT_MONOGRAPH, &CMonographDlg::OnBnClickedPrintMonograph)
	ON_BN_CLICKED(IDC_PRINT_PREVIEW_MONOGRAPH, &CMonographDlg::OnBnClickedPrintPreviewMonograph)
END_MESSAGE_MAP()


// CMonographDlg message handlers

// (j.fouts 2012-08-16 17:55) - PLID 52194 - Added buttons for print/preview
void CMonographDlg::OnBnClickedPrintMonograph()
{
	try
	{
		if(m_pBrowser)
		{
			m_pBrowser->ExecWB(OLECMDID_PRINT, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-08-16 17:55) - PLID 52194 - Added buttons for print/preview
void CMonographDlg::OnBnClickedPrintPreviewMonograph()
{
	try
	{
		if(m_pBrowser)
		{
			m_pBrowser->ExecWB(OLECMDID_PRINTPREVIEW, OLECMDEXECOPT_PROMPTUSER, NULL, NULL);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-08-29 09:51) - PLID 51719 - When the page loads we can set the custom HTML
void CMonographDlg::DocumentComplete(LPDISPATCH pDisp, VARIANT* URL)
{
	try
	{
		LoadCustomHtml();
	}
	NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CMonographDlg, CNxDialog)
	ON_EVENT(CMonographDlg, IDC_MONOGRAPH_BROWSER, 259, DocumentComplete, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()