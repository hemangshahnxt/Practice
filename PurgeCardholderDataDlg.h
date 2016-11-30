#pragma once
#include "AdministratorRc.h"

// CPurgeCardholderDataDlg dialog
// (d.thompson 2010-03-15) - PLID 37729 - Created

class CPurgeCardholderDataDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPurgeCardholderDataDlg)

public:
	CPurgeCardholderDataDlg(CWnd* pParent);   // standard constructor
	virtual ~CPurgeCardholderDataDlg();

// Dialog Data
	enum { IDD = IDD_PURGE_CARDHOLDER_DATA_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();


	CDateTimePicker m_datePurge;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_btnPAN;
	NxButton m_btnExpDate;
	NxButton m_btnCardholderName;

	DECLARE_MESSAGE_MAP()
};
