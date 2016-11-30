#pragma once
#include "NxInputMask.h"

// (z.manning 2009-01-13 14:31) - PLID 32719 - Created
// CEmrTableInputMaskDlg dialog

class CEmrTableInputMaskDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrTableInputMaskDlg)

public:
	CEmrTableInputMaskDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrTableInputMaskDlg();

	void SetInitialInputMask(const CString strInputMask);
	CString GetInputMask();

// Dialog Data
	enum { IDD = IDD_EMR_TABLE_INPUT_MASK };

protected:

	CString m_strInputMask;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	CNxEdit m_nxeditHelp;
	CNxEdit m_nxeditInputMask;
	CMaskedEdit m_editInputMaskTest;

	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	afx_msg void OnEnChangeInputMask();
	afx_msg void OnBnClickedOk();
};
