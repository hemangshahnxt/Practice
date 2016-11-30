#pragma once


// CEditLabStatusDlg dialog

//TES 12/4/2008 - PLID 32191 - Created, based off of CEditLabResultsDlg.
class CEditLabStatusDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditLabStatusDlg)

public:
	CEditLabStatusDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditLabStatusDlg();

	//TES 5/2/2011 - PLID 43428 - Added option to use this dialog for editing LabOrderStatusT instead of LabResultStatusT, since both have
	// the exact same fields.
	bool m_bEditOrderStatus;

	NXDATALIST2Lib::_DNxDataListPtr m_pStatusList;
// Dialog Data
	enum { IDD = IDD_EDIT_LAB_STATUS };
	CNxIconButton	m_nxbClose;
	CNxIconButton	m_nxbEdit;
	CNxIconButton	m_nxbDelete;
	CNxIconButton	m_nxbAdd;
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAddStatus();
	afx_msg void OnDeleteStatus();
	afx_msg void OnEditStatus();
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishingResultStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedResultStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
