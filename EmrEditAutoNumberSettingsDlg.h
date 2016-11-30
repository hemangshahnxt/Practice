#pragma once


// CEmrEditAutoNumberSettingsDlg dialog
// (z.manning 2010-08-11 13:17) - PLID 40074 - Created

class CEmrEditAutoNumberSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrEditAutoNumberSettingsDlg)

public:
	CEmrEditAutoNumberSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrEditAutoNumberSettingsDlg();

	CString m_strPrefix;
	EEmrTableAutoNumberType m_eType;

// Dialog Data
	enum { IDD = IDD_EMR_EDIT_AUTO_NUMBER_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	CNxColor m_nxcolor;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
