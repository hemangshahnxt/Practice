#pragma once


// CDictationEditInfoDlg dialog
// (z.manning 2015-12-18 10:06) - PLID 67738 - Created

class CDictationEditInfoDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDictationEditInfoDlg)

public:
	CDictationEditInfoDlg(long nLicenseKey, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDictationEditInfoDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_DICTATION_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	long m_nLicenseKey;

	DECLARE_MESSAGE_MAP()
};
