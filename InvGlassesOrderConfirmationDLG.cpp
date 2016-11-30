// InvGlassesOrderConfirmationDLG.cpp : implementation file
//
// (s.dhole 2010-11-29 17:10) - PLID 41125 Glasses order confirmation
#include "stdafx.h"
#include "Practice.h"
#include "Practice.h"
#include "InvGlassesOrderConfirmationDLG.h"
#include "InventoryRc.h"
#include "InvVisionWebUtils.h"

// CInvGlassesOrderConfirmationDLG dialog
using namespace ADODB;

IMPLEMENT_DYNAMIC(CInvGlassesOrderConfirmationDLG, CNxDialog)

CInvGlassesOrderConfirmationDLG::CInvGlassesOrderConfirmationDLG(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvGlassesOrderConfirmationDLG::IDD, pParent)
{

}

CInvGlassesOrderConfirmationDLG::~CInvGlassesOrderConfirmationDLG()
{
}

void CInvGlassesOrderConfirmationDLG::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnSend);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
} 


BEGIN_MESSAGE_MAP(CInvGlassesOrderConfirmationDLG, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvGlassesOrderConfirmationDLG::OnBnClickedOk)
END_MESSAGE_MAP()


// CSureScriptsConfirmationDlg message handlers
BOOL CInvGlassesOrderConfirmationDLG::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();
		m_btnSend.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pBrowser = GetDlgItem(IDC_GLASSES_ORDER_PREVIEW)->GetControlUnknown();
		//TES 11/23/2009 - PLID 36192 - Now set it to blank, and disable it.
		COleVariant varUrl("about:blank");
		if (m_pBrowser) {
			m_pBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
			m_pBrowser->Navigate2(varUrl, COleVariant((long)navNoHistory), NULL, NULL, NULL);
			GetDlgItem(IDC_GLASSES_ORDER_PREVIEW)->EnableWindow(FALSE);
		}
		CInvVisionWebUtils *VisionWebUtils= new CInvVisionWebUtils;
		// (s.dhole 2011-04-12 16:02) - PLID 43237 Check for error if error flag set than disable send button
		BOOL ISError=FALSE; 
		GetDlgItem(IDC_GLASSES_ORDER_PREVIEW)->EnableWindow(NavigateToHTML(COleVariant(  VisionWebUtils->GetHTML(m_nOrderID,ISError))));
		// Release object
		delete VisionWebUtils;
		if (ISError==TRUE)
			GetDlgItem(IDOK)->EnableWindow(FALSE);

	}NxCatchAll("Error in CInvGlassesOrderConfirmationDLG::OnInitDialog()");

	return FALSE;
}


BOOL CInvGlassesOrderConfirmationDLG::NavigateToHTML(COleVariant& varHTML)
{
	CString  strHTML; 
	HGLOBAL hHTMLText = NULL;
	IStreamPtr pStream;
	IPersistStreamInitPtr pPersistStreamPtr;

	try {
		CString str = VarString(varHTML);
		hHTMLText = GlobalAlloc(GPTR, str.GetLength() + 1);
		if (hHTMLText) {
			// copy the text into the global memory
			strcpy((TCHAR*)hHTMLText, str);
			// by sending TRUE for the second parameter, the underlying handle (hHTMLText
			// will be freed when the stream is released
			HRESULT hr = CreateStreamOnHGlobal(hHTMLText, TRUE, &pStream);
			if (SUCCEEDED(hr)) {
				// load the stream into the browser
				IDispatchPtr pDoc;
				hr = m_pBrowser->get_Document(&pDoc);
				if (SUCCEEDED(hr)) {
					hr = pDoc->QueryInterface(IID_IPersistStreamInit, (void**)&pPersistStreamPtr);
					if (SUCCEEDED(hr)) {
						hr = pPersistStreamPtr->InitNew();
						if (SUCCEEDED(hr)) {
							hr = pPersistStreamPtr->Load(pStream);
						}
					}
				}
			}

			hHTMLText = NULL;
			return SUCCEEDED(hr);
        }

	} NxCatchAll("Error in CInvGlassesOrderConfirmationDLG::NavigateToHTML");

	// if we have not created a stream, release the HGLOBAL memory
	if (pStream == NULL) {
		if (hHTMLText) {
			GlobalFree(hHTMLText);
		}
	}

	return FALSE;
}


// CInvGlassesOrderConfirmationDLG message handlers

void CInvGlassesOrderConfirmationDLG::OnBnClickedOk()
{
	try {
		CInvVisionWebUtils *VisionWebUtils= new CInvVisionWebUtils;
		if (!VisionWebUtils->UploadOrderToVisionWeb(m_nOrderID)) {
			// releasing object
			delete VisionWebUtils;
			CNxDialog::OnCancel();
			return;
		}
		// releasing object
		delete VisionWebUtils;
		CNxDialog::OnOK();

	
	}NxCatchAll(__FUNCTION__);
}
