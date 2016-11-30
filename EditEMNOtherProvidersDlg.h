#pragma once

// (j.jones 2013-05-16 15:18) - PLID 56596 - replaced EMN.h with a forward declare of EMNProvider
class EMNProvider;

// CEditEMNOtherProvidersDlg dialog
// (j.gruber 2009-06-16 16:01) - PLID 33688 - created for
class CEditEMNOtherProvidersDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditEMNOtherProvidersDlg)

public:
	CEditEMNOtherProvidersDlg(CArray<EMNProvider*, EMNProvider*> *paryProviders, CWnd* pParent);   // standard constructor
	virtual ~CEditEMNOtherProvidersDlg();

// Dialog Data
	enum { IDD = IDD_EDIT_EMN_OTHER_PROVS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnRemove;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxColor	m_color;

	afx_msg void OnBnClickedAddEmrOtherProv();
	afx_msg void OnBnClickedRemoveEmrOtherProv();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	CArray<EMNProvider*, EMNProvider*> *m_paryProviders;
	CMap<long, long, CString, CString> m_mapProviderTypes;

	void LoadListFromArray();	

	BOOL m_bWarnIfCancel;

	NXDATALIST2Lib::_DNxDataListPtr m_pProviderList;
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishingEmnOtherProvList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
