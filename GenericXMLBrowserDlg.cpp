// GenericXMLBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GenericXMLBrowserDlg.h"
#include "NxXMLUtils.h"
#include "GlobalUtils.h"

// (j.jones 2010-06-30 11:40) - PLID 38031 - moved from the CCDInterface namespace

// CGenericXMLBrowserDlg dialog

IMPLEMENT_DYNAMIC(CGenericXMLBrowserDlg, CGenericBrowserDlg)

void CGenericXMLBrowserDlg::NavigateToXMLDocument(const CString& strXMLFilePath, MSXML2::IXMLDOMDocument2Ptr pXslDocument)
{
	// (j.jones 2010-06-30 10:58) - PLID 38031 - LoadXMLDocument has been moved to the NxXMLUtils namespace
	NavigateToXMLDocument(NxXMLUtils::LoadXMLDocument(strXMLFilePath), pXslDocument);
}

void CGenericXMLBrowserDlg::NavigateToXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, MSXML2::IXMLDOMDocument2Ptr pXslDocument)
{
	m_pDocument = pDocument;
	m_strStylesheet.Empty();
	m_pXslDocument = pXslDocument;

	if (m_pXslDocument == NULL) {
		m_bAllowToggle = FALSE;
		m_bViewXMLSource = TRUE;
		m_strStylesheet = GetGenericXMLXSLResourcePath();
	}

	LoadInternalXMLDocument();
}

void CGenericXMLBrowserDlg::NavigateToXMLDocument(const CString& strXMLFilePath, const CString& strStylesheetPath)
{
	// (j.jones 2010-06-30 10:58) - PLID 38031 - LoadXMLDocument has been moved to the NxXMLUtils namespace
	NavigateToXMLDocument(NxXMLUtils::LoadXMLDocument(strXMLFilePath), strStylesheetPath);
}

void CGenericXMLBrowserDlg::NavigateToXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strStylesheetPath)
{
	m_pDocument = pDocument;
	m_strStylesheet = strStylesheetPath;

	if (m_strStylesheet.IsEmpty()) {
		m_bAllowToggle = FALSE;
		m_bViewXMLSource = TRUE;
		m_strStylesheet = GetGenericXMLXSLResourcePath();
	}

	LoadInternalXMLDocument();
}

void CGenericXMLBrowserDlg::LoadInternalXMLDocument()
{	
	if (m_pDocument == NULL) {
		_com_issue_error(DISP_E_BADVARTYPE);
	}

	if (m_pXslDocument != NULL) {
		UpdateControls();

		if (m_bViewXMLSource) {
			NavigateToStream(ApplyXSLT(m_pDocument, GetGenericXMLXSLResourcePath()));
		} else {
			NavigateToStream(ApplyXSLT(m_pDocument, m_pXslDocument));
		}
	} else {
		CString strStylesheet = m_bViewXMLSource ? GetGenericXMLXSLResourcePath() : m_strStylesheet;
		if (strStylesheet.IsEmpty()) {
			strStylesheet = GetGenericXMLXSLResourcePath();
			m_bAllowToggle = FALSE;
			m_bViewXMLSource = TRUE;
		}

		UpdateControls();

		NavigateToStream(ApplyXSLT(m_pDocument, strStylesheet));
	}
}

void CGenericXMLBrowserDlg::InitializeControls()
{
	m_nxibCancel.AutoSet(NXB_CLOSE);
	m_nxibCancel.SetFocus();

	m_nxlabelCaption.SetType(dtsHyperlink);
	UpdateControls();
}

void CGenericXMLBrowserDlg::UpdateControls()
{
	// no OK button by default
	m_nxibOK.EnableWindow(FALSE);
	m_nxibOK.ShowWindow(SW_HIDE);

	if (!m_bAllowToggle) {
		m_nxlabelCaption.ShowWindow(SW_HIDE);
		m_nxlabelCaption.EnableWindow(FALSE);
		m_nxlabelCaption.SetType(dtsDisabledHyperlink);			
	} else {
		m_nxlabelCaption.ShowWindow(SW_SHOWNA);
		m_nxlabelCaption.EnableWindow(TRUE);
		m_nxlabelCaption.SetType(dtsHyperlink);
	}

	m_nxlabelCaption.SetText(GetCaption());
}

CString CGenericXMLBrowserDlg::GetCaption()
{
	if (m_bViewXMLSource) {
		return "View styled document";
	} else {
		return "View XML source";
	}
}

LRESULT CGenericXMLBrowserDlg::OnLabelClicked(WPARAM wParam, LPARAM lParam)
{
	try {
		m_bViewXMLSource = m_bViewXMLSource ? FALSE : TRUE;

		LoadInternalXMLDocument();
		
		UpdateControls();

	} NxCatchAll("Error toggling view state");
	return 0;
}