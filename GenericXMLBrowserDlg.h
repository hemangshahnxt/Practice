#pragma once

#include "GenericBrowserDlg.h"

// (j.jones 2010-06-30 11:40) - PLID 38031 - moved from the CCDInterface namespace

// CGenericXMLBrowserDlg dialog

class CGenericXMLBrowserDlg : public CGenericBrowserDlg
{
	DECLARE_DYNAMIC(CGenericXMLBrowserDlg)

public:
	CGenericXMLBrowserDlg(CWnd* pParent) : CGenericBrowserDlg(pParent) {
		m_bViewXMLSource = FALSE;
		m_bAllowToggle = TRUE;
	};

	void NavigateToXMLDocument(const CString& strXMLFilePath, MSXML2::IXMLDOMDocument2Ptr pXslDocument) throw(...);
	void NavigateToXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, MSXML2::IXMLDOMDocument2Ptr pXslDocument) throw(...);
	void NavigateToXMLDocument(const CString& strXMLFilePath, const CString& strStylesheetPath = "") throw(...);
	void NavigateToXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strStylesheetPath = "") throw(...);

	BOOL m_bAllowToggle;

protected:	
	void LoadInternalXMLDocument() throw(...);
	MSXML2::IXMLDOMDocument2Ptr m_pDocument;

	BOOL m_bViewXMLSource;

	CString m_strStylesheet;
	MSXML2::IXMLDOMDocument2Ptr m_pXslDocument;

	afx_msg virtual LRESULT OnLabelClicked(WPARAM wParam, LPARAM lParam);
	virtual void InitializeControls();
	virtual void UpdateControls();

	CString GetCaption();
};