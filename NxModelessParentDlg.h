// (a.wilson 2013-01-09 14:00) - PLID 54535 - created

#pragma once

// (a.wilson 2013-01-10 11:09) - PLID 54535 - this will control which buttons the creator wants on the dialog.
enum ParentDialogButtonOptions
{
	bsNone = 0,
	bsOk = 1,
	bsOkCancel = 2,
};

// CNxModelessParentDlg dialog

class CNxModelessParentDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNxModelessParentDlg)

public:
	//constructor/deconstructor
	// (a.walling 2015-01-12 10:28) - PLID 64558 - Rescheduling Queue - handle size and pos memory, custom styles
	CNxModelessParentDlg(CWnd* pParent, const CString& strSizeAndPositionConfigRT, ParentDialogButtonOptions pdbs = bsNone, DWORD styleRemove = 0, DWORD styleAdd = 0);

	virtual ~CNxModelessParentDlg();

	enum { IDD = IDD_NX_MODELESS_PARENT_DLG };

	//variables
	std::unique_ptr<CNxDialog> m_pChild;

	//functions
	void CreateWithChildDialog(std::unique_ptr<CNxDialog> pChild, int IDD_CHILD, int IDR_ICON, const CString & strTitle, CWnd* pParent);

protected:
	//variables
	long m_nButtonBuffer;
	int m_nChildIDD;
	ParentDialogButtonOptions m_pdbs;
	
	DWORD m_styleRemove = 0;
	DWORD m_styleAdd = 0;

	CNxIconButton m_btnOK, m_btnCancel;
	
	//functions
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void ResizeContents();

	//message functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (j.fouts 2013-04-22 16:01) - PLID 54719 - Send Hide/Show messages to child
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	// (j.fouts 2013-04-22 16:01) - PLID 54719 - Send Hide/Show messages to child
	afx_msg void OnClose();
};
