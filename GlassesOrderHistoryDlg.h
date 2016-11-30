#pragma once

//TES 6/20/2011 - PLID 43700 - Created
// CGlassesOrderHistoryDlg dialog

class CGlassesOrderHistoryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CGlassesOrderHistoryDlg)

public:
	CGlassesOrderHistoryDlg(CWnd* pParent);   // standard constructor
	virtual ~CGlassesOrderHistoryDlg();

	//TES 6/20/2011 - PLID 43700 - Caller should set this before showing the dialog
	long m_nPatientID;

protected:
	CNxIconButton m_nxbClose;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	NXCOLORLib::_DNxColorPtr m_pBkg;

// Dialog Data
	enum { IDD = IDD_GLASSES_ORDER_HISTORY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
