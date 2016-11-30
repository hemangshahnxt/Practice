#pragma once

// (j.dinatale 2010-11-05) - PLID 39226 - Created

// CBillTabSettingsDlg dialog

class CBillTabSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillTabSettingsDlg)

public:
	CBillTabSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CBillTabSettingsDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_dlColumnList;

	CString GetColumnName(long nColType);
	void Save();
	void EnsureBillColSQLStructure();

	virtual int DoModal();

// Dialog Data
	enum { IDD = IDD_BILLTAB_COL_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnMoveColUp;
	CNxIconButton m_btnMoveColDown;

	void OnEditFinished(LPDISPATCH lpRow, short nCol, const VARIANT &varOldValue, const VARIANT &varNewValue, BOOL bCommit);

	DECLARE_EVENTSINK_MAP()

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCancel();
	afx_msg void OnOK();
	afx_msg void OnBnClickedMovecolup();
	afx_msg void OnBnClickedMovecoldown();
};
