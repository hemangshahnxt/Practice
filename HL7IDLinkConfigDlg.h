#pragma once

// CHL7IDLinkConfigDlg dialog

// (j.dinatale 2013-01-08 10:48) - PLID 54491 - created

enum HL7IDLink_RecordType;
struct HL7IDMap {
	CString strCode;
	long nPersonID;
};

class CHL7IDLinkConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7IDLinkConfigDlg)

public:
	CHL7IDLinkConfigDlg(HL7IDLink_RecordType hl7IDLinkType, long nHL7GroupID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CHL7IDLinkConfigDlg();

// Dialog Data
	enum { IDD = IDD_HL7_ID_LINK_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	// what HL7 ID record type are we configuring?
	HL7IDLink_RecordType m_IDLinkRecordType;

	// what hl7 group?
	long m_nHL7GroupID;

	// HL7 ID map list
	NXDATALIST2Lib::_DNxDataListPtr m_pIDMapList;

	// store deleted codes
	CArray<HL7IDMap, HL7IDMap&> m_aryDeletedIDs;

	// UI Buttons
	CNxIconButton	m_btnImport;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

	// Utils
	CString GetHL7IDLinkRecordName(HL7IDLink_RecordType hilrt);
	CString GetPersonName(long nPersonID);
	CString GetTableName(HL7IDLink_RecordType hilrt);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedAddHl7IdMap();
	afx_msg void OnBnClickedRemoveHl7IdMap();
	DECLARE_EVENTSINK_MAP()
	void SelChangedHl7IdMap(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void EditingFinishingHl7IdMap(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedImportHl7IdMap();	// (j.dinatale 2013-01-14 16:48) - PLID 54602
};
