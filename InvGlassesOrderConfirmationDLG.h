// (s.dhole 2010-11-29 17:10) - PLID 41125 Glasses order confirmation

#pragma once
#include "InventoryRc.h"
 
// CInvGlassesOrderConfirmationDLG dialog

class CInvGlassesOrderConfirmationDLG : public CNxDialog
{
	DECLARE_DYNAMIC(CInvGlassesOrderConfirmationDLG)

public:
	CInvGlassesOrderConfirmationDLG(CWnd* pParent);   // standard constructor
	virtual ~CInvGlassesOrderConfirmationDLG();
	long m_nOrderID;

// Dialog Data
	enum { IDD = IDD_GLASSES_ORDER_CONFIRMATION };

protected:
	CNxIconButton m_btnSend;
	CNxIconButton m_btnCancel;

	IWebBrowser2Ptr m_pBrowser;	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	BOOL NavigateToHTML(COleVariant& varHTML);
	CString   GetHTML();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
 