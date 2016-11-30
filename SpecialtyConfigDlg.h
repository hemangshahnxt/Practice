#pragma once


// CSpecialtyConfigDlg dialog

class CSpecialtyConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(CSpecialtyConfigDlg)

public:
	CSpecialtyConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpecialtyConfigDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_SpecialtyList;

// Dialog Data
	enum { IDD = IDD_PATIENT_SPECIALTY_CONFIG_DLG };
	CNxIconButton	m_btnNew;
	CNxIconButton	m_btnRenameSpecialty;
	CNxIconButton	m_btnDeleteSpecialty;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void InsertNew();
	CString m_strSql;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNew();
	afx_msg void OnBnClickedRenameGroup();
	afx_msg void OnBnClickedDeleteSpecialty();
	DECLARE_EVENTSINK_MAP()
	void SelSetSpecialtiesConfigList(LPDISPATCH lpSel);
	void EditingFinishingSpecialtiesConfigList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedSpecialtiesConfigList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedOk();
};
