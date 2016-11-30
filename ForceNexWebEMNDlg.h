#pragma once


// CForceNexWebEMNDlg dialog
// (d.thompson 2009-11-04) - PLID 35811 - Created
#include "EMRRc.h"

class CForceNexWebEMNDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CForceNexWebEMNDlg)

public:
	CForceNexWebEMNDlg(CWnd* pParent);   // standard constructor
	virtual ~CForceNexWebEMNDlg();

// Dialog Data
	enum { IDD = IDD_FORCE_NEXWEB_EMN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnForce;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedForceFinal();
};
