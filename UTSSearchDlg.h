#pragma once


// CUTSSearchDlg dialog

// (d.singleton 2013-10-09 13:49) - PLID 58882 - added dialog to access codes on the utc. 

class CUTSSearchDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CUTSSearchDlg)

public:
	CUTSSearchDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUTSSearchDlg();

// Dialog Data
	enum { IDD = IDD_UTS_SEARCH };

	virtual int DoModal() override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	// (a.walling 2013-10-18 10:51) - PLID 59096 - UTS search should support other vocabularies beyond SNOMEDCT
	CString GetSelectedVocab();

	NXDATALIST2Lib::_DNxDataListPtr m_dlCodes;
	NXDATALIST2Lib::_DNxDataListPtr m_dlVocab;

	bool m_bManuallyEdited;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	afx_msg void OnBnClickedSearch();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAdd();

	void SelChangingUTSCodes(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingVocabList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
};
