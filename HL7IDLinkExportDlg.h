#pragma once

// (j.dinatale 2013-01-14 11:51) - PLID 54602 - created

#include "FinancialRC.h"
#include <NxHL7Lib\HL7CommonTypes.h>

// CHL7IDLinkExportDlg dialog

enum HL7IDLink_RecordType;

class CHL7IDLinkExportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7IDLinkExportDlg)

public:
	CHL7IDLinkExportDlg(HL7IDLink_RecordType hl7IDLinkType, CWnd* pParent = NULL);   // standard constructor
	virtual ~CHL7IDLinkExportDlg();

	CString GetFilePath();

// Dialog Data
	enum { IDD = IDD_HL7_ID_LINK_EXPORT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	// controls
	CNxIconButton m_nxbExport;
	CNxIconButton m_nxbCancel;

	//store the file path
	CString m_strFilePath;
	
	// utils
	CString GetHL7IDLinkRecordName(HL7IDLink_RecordType hilrt);
	CString GetTableName(HL7IDLink_RecordType hilrt);
	CString EscapeCSV(CString strValue);

	// what HL7 ID record type are we configuring?
	HL7IDLink_RecordType m_IDLinkRecordType;

	// HL7 Group List
	NXDATALIST2Lib::_DNxDataListPtr m_pGroupList;

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChosenHl7idExportHl7groupsList(LPDISPATCH lpRow);
	void SelChangingHl7idExportHl7groupsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RequeryFinishedHl7idExportHl7groupsList(short nFlags);
};
