//(a.wilson 2012-3-5) PLID 48420 - created for the recall system.

#pragma once

// CRecallLinkDlg dialog

class CRecallLinkDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRecallLinkDlg)

public:
	CRecallLinkDlg(const long& nPatientID, const long& nAppointmentID, CWnd* pParent);
	virtual ~CRecallLinkDlg();

	enum { IDD = IDD_RECALL_LINK_DLG };

	bool HasRecalls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	struct RecallInfo
	{
		long nID;
		long nStatusColor;
		COleDateTime dtDate;
		CString strStatus;
		CString strTemplateName;
		// (j.jones 2016-02-18 11:19) - PLID 68350 - added provider & location
		CString strProviderName;
		CString strLocationName;
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	const long m_nPatientID;
	const long m_nAppointmentID;
	CArray<RecallInfo> m_arRecallInfo;
	CNxColor m_nxcBackground;
	CNxIconButton m_nxbLink, m_nxbClose;

private:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();

private:
	DECLARE_EVENTSINK_MAP()
	void SelChangingActiveList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);	// (j.armen 2012-07-23 10:52) - PLID 51600
};