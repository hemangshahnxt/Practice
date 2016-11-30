#pragma once

// (j.dinatale 2012-08-02 16:11) - PLID 51911 - created

// COMRReviewDlg dialog
class COMRReviewBrowserCtrl;

class COMRReviewDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COMRReviewDlg)

public:
	COMRReviewDlg(NexTech_COM::INxXmlGeneratorPtr m_pNxNXLtoReview, CWnd* pParent = NULL);   // standard constructor
	virtual ~COMRReviewDlg();

// Dialog Data
	enum { IDD = IDD_OMR_REVIEW_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();

	NexTech_COM::INxXmlGeneratorPtr m_pNxNXLtoReview;
	CArray<CString, CString> m_aryScannedDocs;

	CNxIconButton m_btnCommit, m_btnDoNotCommit, m_btnCancel, m_btnPrevAttach, m_btnNextAttach;
	CNxColor m_bkgrnd;

	// (b.savon 2013-03-05 14:11) - PLID 55456
	NxButton	m_radioExpandAll;
	NxButton	m_radioCollapseAll;
	NxButton	m_radioCollapseValid;

	NXDATALIST2Lib::_DNxDataListPtr m_pDetailList;
	scoped_ptr<COMRReviewBrowserCtrl> m_pCtrl;

	CRect COMRReviewDlg::GetHTMLControlRect();
	void LoadScannedDoc(long nIndex);
	long m_nDocIndex;

	// (b.savon 2013-03-08 12:54) - PLID 55456 - Added silent flag and format tree flag
	// (b.savon 2013-02-28 16:26) - PLID 54714 - Changed return type to BOOL
	// (b.spivey, August 28, 2012) - PLID 52286 - Color rows based on validity. 
	BOOL ValidateSelections(BOOL bSilent = FALSE, BOOL bFormatTree = TRUE);
	bool m_bMultiSelectWarning; 

	// (b.savon 2013-02-28 15:09) - PLID 54714 - Validate items singley
	// (b.savon 2013-03-05 14:12) - PLID 55456 - Added default collapse flag to not collapse on corrections
	void ValidateItemSelections(NXDATALIST2Lib::IRowSettingsPtr pParentRow, BOOL bCollapse = TRUE);

	// (b.savon 2013-03-05 14:12) - PLID 55456 - Collapse/Expand parent row based on radio selection
	void PutParentRowView(NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOmrReviewPrev();
	afx_msg void OnBnClickedOmrReviewNext();
	afx_msg void OnBnClickedOmrReviewCommit();
	afx_msg void OnBnClickedOmrReviewDonotcommit();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedOmrPendCommitDetails(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedRdoCollapsedSuccess();
	afx_msg void OnBnClickedRdoCollapsedAll();
	afx_msg void OnBnClickedRdoExpanded();
	afx_msg void OnDestroy(); //TES 5/15/2014 - PLID 62130
};
