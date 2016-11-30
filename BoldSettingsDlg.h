#pragma once
#include "FinancialRc.h"

// CBoldSettingsDlg dialog
// (j.gruber 2010-06-01 12:25) - PLID 38817 - created for
class CBoldSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBoldSettingsDlg)

public:
	CBoldSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CBoldSettingsDlg();
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_rdPreProduction;
	NxButton m_rdProduction;
	CNxStatic m_stMessage;
	// (j.gruber 2010-06-01 12:44) - PLID 38932 - added Practice COEID
	CNxEdit m_edtPracCOEID;

// Dialog Data
	enum { IDD = IDD_BOLD_SETUP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:	
	afx_msg void OnBnClickedOk();
};
