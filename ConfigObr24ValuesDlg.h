#pragma once

//TES 7/30/2010 - PLID 39908 - Created
// CConfigObr24ValuesDlg dialog

class CConfigObr24ValuesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigObr24ValuesDlg)

public:
	CConfigObr24ValuesDlg(CWnd* pParent);   // standard constructor
	virtual ~CConfigObr24ValuesDlg();

	//TES 7/30/2010 - PLID 39908 - Input/output variable, tracks which values the user has set up.
	CStringArray m_saValues;

protected:
	CNxIconButton m_nxbAdd, m_nxbRemove, m_nxbOK, m_nxbCancel;
	NXDATALIST2Lib::_DNxDataListPtr m_pValuesList;

// Dialog Data
	enum { IDD = IDD_CONFIG_OBR_24_VALUES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAddObr24Value();
	afx_msg void OnRemoveObr24Value();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangedObr24Values(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnEditingFinishingObr24Values(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
