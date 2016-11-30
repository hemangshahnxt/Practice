#pragma once

// (d.singleton 2013-02-15 09:12) - PLID 55199 new dialog for the text content setting values

#include "AdministratorRc.h"

// CNexwebTextContentOverrideDlg dialog

class CNexwebTextContentOverrideDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexwebTextContentOverrideDlg)

public:
	CNexwebTextContentOverrideDlg(long nSubdomainID, CString strMasterUID, CString strSettingName, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexwebTextContentOverrideDlg();

	CNxIconButton	m_btnNexWebContentSettingUp;
	CNxIconButton	m_btnNexWebContentSettingDown;
	CNxIconButton	m_btnNexWebContentSettingAdd;
	CNxIconButton	m_btnNexWebContentSettingRemove;
	CNxIconButton	m_btnClose;

// Dialog Data
	enum { IDD = IDD_TEXT_CONTENT_SETTING_OVERRIDE };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CString m_strMasterUID;
	long m_nSubdomainID;
	CString m_strSettingName;

	NXDATALIST2Lib::_DNxDataListPtr m_pTextContentSettingDetails;

	afx_msg void OnBnClickedAddNewLine();
	afx_msg void OnBnClickedRemoveLine();
	afx_msg void OnBnClickedMoveLineUp();
	afx_msg void OnBnClickedMoveLineDown();
	afx_msg void OnBnClickedContentSettingClose();
	void EditingFinishedTextContentSettings(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()	
};
