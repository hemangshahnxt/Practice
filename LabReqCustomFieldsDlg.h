// (r.gonet 10/16/2011) - PLID 45968 - Dialog that allows the configuration of which lab custom fields will go on a given custom lab request report.

#pragma once

#include "ReportsRc.h"

// CLabReqCustomFieldsDlg dialog

class CLabReqCustomFieldsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabReqCustomFieldsDlg)

	enum ELabReqCustomFieldsColumns
	{
		lrcfcID = 0,
		lrcfcLabCustomFieldID,
		lrcfcReportFieldName,
		lrcfcModified,
	};

	// Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pFieldsList;
	CNxIconButton m_nxbAdd;
	CNxIconButton m_nxbAddMultiple;
	CNxIconButton m_nxbRemove;
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;

	// Data
	CArray<long, long> m_aryDeletedFields; // (r.gonet 10/16/2011) - PLID 45968 - The report field IDs that will be deleted when this dialog is saved.
	bool m_bChainingAdds; // (r.gonet 10/16/2011) - PLID 45968 - While this is true, after the user completes a new row, another new row is created and begins editing.
	NXDATALIST2Lib::IRowSettingsPtr m_pLastAddedRow; // (r.gonet 10/16/2011) - PLID 45968 - The last row that was added by the user. May be deleted when chaining is stopped.
	long m_nCustomReportNumber; // (r.gonet 10/16/2011) - PLID 45968 - The custom report number we are configuring

	// Methods
	NXDATALIST2Lib::IRowSettingsPtr AddField();
	void StartChaining();
	void StopChaining();

public:
	CLabReqCustomFieldsDlg(long nCustomReportNumber, CWnd* pParent);   // standard constructor
	virtual ~CLabReqCustomFieldsDlg();

// Dialog Data
	enum { IDD = IDD_REQ_CUSTOM_FIELDS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedReqCustomFieldsAddBtn();
	afx_msg void OnBnClickedReqCustomFieldsAddMultipleBtn();
	afx_msg void OnBnClickedReqCustomFieldsRemoveBtn();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedReqCustomFieldsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingReqCustomFieldsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
