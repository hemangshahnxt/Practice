#pragma once


// CLabConfirmDeleteDlg dialog
// (z.manning 2008-10-09 14:46) - PLID 31628 - Created
class CLabConfirmDeleteDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabConfirmDeleteDlg)

public:
	CLabConfirmDeleteDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabConfirmDeleteDlg();

	void SetLabInfo(IN const CString strLabInfo);
	void SetPatientName(IN const CString strPatientName);

// Dialog Data
	enum { IDD = IDD_LAB_CONFIRM_DELETE };
	CNxStatic m_nxstaticMessageTop;
	CNxStatic m_nxstaticMessageBottom;
	CNxEdit m_nxeditLabDetails;
	CNxIconButton m_btnKeepLab;
	CNxIconButton m_btnDeleteLab;

protected:

	CString m_strLabInfo;
	CString m_strPatientName;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
