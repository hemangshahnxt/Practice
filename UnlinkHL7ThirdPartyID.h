#pragma once

// CUnlinkHL7ThirdPartyID dialog

// (b.savon 2011-10-04 11:22) - PLID 39890 - We need a way to unlink patients/providers/ref. phys 
//											 that have their NexTech 3rd party ID remembered

class CUnlinkHL7ThirdPartyID : public CNxDialog
{
	DECLARE_DYNAMIC(CUnlinkHL7ThirdPartyID)

public:
	CUnlinkHL7ThirdPartyID(CWnd* pParent);   // standard constructor
	virtual ~CUnlinkHL7ThirdPartyID();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_UNLINK_HL7_3RDPARTYIDS };

protected:

	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnUnlink;

	NXDATALIST2Lib::_DNxDataListPtr m_pHL7GroupFilter;
	NXDATALIST2Lib::_DNxDataListPtr m_pHL7LinkFilter;
	NXDATALIST2Lib::_DNxDataListPtr m_pHL7UnlinkList;

	void BindGroupList();
	void BindLinkList();
	void BindUnlinkList();

	CString m_strGroupFilter;
	long m_nGroupFilterID;

	CString m_strLinkFilter;
	long m_nLinkFilterID;

	CString GetBindUnlinkListWhereClause();
	CString m_strBothFilterWhere;
	CString m_strGroupFilterWhere;
	CString m_strLinkFilterWhere;

	void PrepareSelectedRows( CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arySelectedRows );

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedUnlinkSelected();
	DECLARE_EVENTSINK_MAP()
	void SelChosenNxDlHl7GroupFilter(LPDISPATCH lpRow);
	void SelChosenNxDlHl7LinkFilter(LPDISPATCH lpRow);
};
