// LeafletDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LeafletDlg.h"

// (j.luckoski 2012-09-25 04:14 PM) - PLID 53042 - Created.

// CLeafletDlg dialog

IMPLEMENT_DYNAMIC(CLeafletDlg, CNxDialog)

CLeafletDlg::CLeafletDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLeafletDlg::IDD, pParent, "LeafletDlg")
{
	// (j.luckoski 2013-01-15 14:58) - PLID 53042 - Empty html
	m_strHTML = "";
	m_pBrowser = NULL;
}

CLeafletDlg::~CLeafletDlg()
{
}

void CLeafletDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LEAFLET_COLOR, m_bkg);
	DDX_Control(pDX, IDC_PRINT_LEAFLET, m_btnPrint);
	DDX_Control(pDX, IDC_PRINT_PREVIEW_LEAFLET, m_btnPrintPreview);
	DDX_Control(pDX, IDOK, m_btnOk);
}

BOOL CLeafletDlg::OnInitDialog()
{
	try
	{

		CNxDialog::OnInitDialog();

		SetTitleBarIcon(IDI_LEAFLET);

		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);
		m_btnOk.AutoSet(NXB_CLOSE);
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));


		// (j.luckoski 2013-01-15 14:59) - PLID 53042 - Display the HTML leaflet
		CWnd* pBrowserWnd = GetDlgItem(IDC_LEAFLET_BROWSER);
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
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.luckoski 2013-01-15 15:00) - PLID 53042 - Load the html into the browser
void CLeafletDlg::LoadCustomHtml()
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

BEGIN_MESSAGE_MAP(CLeafletDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PRINT_LEAFLET, &CLeafletDlg::OnBnClickedPrintLeaflet)
	ON_BN_CLICKED(IDC_PRINT_PREVIEW_LEAFLET, &CLeafletDlg::OnBnClickedPrintPreviewLeaflet)
END_MESSAGE_MAP()


// CLeafletDlg message handlers

// (j.luckoski 2013-01-15 15:00) - PLID 54638 - Added buttons for print/preview
void CLeafletDlg::OnBnClickedPrintLeaflet()
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

/// (j.luckoski 2013-01-15 15:02) - PLID 54638 - Added buttons for print/preview
void CLeafletDlg::OnBnClickedPrintPreviewLeaflet()
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

// (j.luckoski 2013-01-15 15:02) - PLID 53042 - When the page loads we can set the custom HTML
void CLeafletDlg::DocumentComplete(LPDISPATCH pDisp, VARIANT* URL)
{
	try
	{
		LoadCustomHtml();
	}
	NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CLeafletDlg, CNxDialog)
	ON_EVENT(CLeafletDlg, IDC_LEAFLET_BROWSER, 259, DocumentComplete, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()