#pragma once

#include "GenericXMLBrowserDlg.h"

// (d.singleton 2014-05-28 09:10) - PLID 61927 created to have a custom context menu options for just ccda
// CCCDAViewerDlg

class CCCDAViewerDlg : public CGenericXMLBrowserDlg
{
	DECLARE_DYNAMIC(CCCDAViewerDlg)

public:
	CCCDAViewerDlg(CWnd* pParent, long nMailSentID);

protected:

	virtual BOOL OnInitDialog();

private:
	long m_nMailSentID;	

	class ICCDAInterface* m_piCCDAClientSite;

};

class ICCDAInterface : public IGenericBrowserInterface
{
public:
	ICCDAInterface(long nMailSentID);

	STDMETHOD(GetExternal)(
		/* [out] */ IDispatch **ppDispatch);

private:
	long m_nMailSentID;
	// return S_FALSE to perform web browser's default behavior
	// (d.singleton 2014-05-28 15:01) - PLID 61927  custom menu
	virtual HRESULT CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);

};


