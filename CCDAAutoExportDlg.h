#pragma once


// CCCDAAutoExportDlg dialog

// (a.walling 2015-10-28 09:49) - PLID 67425 - Configuration for CCDA export locations

class CCCDAAutoExportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCCDAAutoExportDlg)

public:
	CCCDAAutoExportDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCCDAAutoExportDlg();
	
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CCDA_AUTO_EXPORT_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	
	void Reload();
	void Save();
	// (v.maida 2016-06-14 10:35) - NX-100833 - Created a function for enabling/disabling all controls.
	void EnableControls(BOOL bEnable);

	virtual void OnOK();
	afx_msg void OnBnClickedVerify();
	afx_msg void OnBnClickedAdd();
};
