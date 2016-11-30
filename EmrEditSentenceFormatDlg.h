#pragma once
#include "afxwin.h"

// (z.manning 2010-07-29 10:12) - PLID 36150 - Created
// CEmrEditSentenceFormatDlg dialog

class CEmrEditSentenceFormatDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrEditSentenceFormatDlg)

public:
	// (r.gonet 04/29/2013) - PLID 44897 - Added two parameters to tell us whether this is a table and its orientation
	CEmrEditSentenceFormatDlg(CWnd* pParent, bool bRowNameFieldAvailable = false, bool bIsTableFlipped = false);   // standard constructor
	virtual ~CEmrEditSentenceFormatDlg();

	void SetInitialSentenceFormat(const CString &strSentenceFormat);
	CString GetSentenceFormat();

	// (z.manning 2011-11-07 10:36) - PLID 46309
	void SetInitialSpawnedItemsSeparator(const CString &strSpawnedItemsSeparator);
	CString GetSpawnedItemsSeparator();

// Dialog Data
	enum { IDD = IDD_EMR_EDIT_SENTENCE_FORMAT };
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
	CString m_strSentenceFormat;
	CString m_strSpawnedItemsSeparator; // (z.manning 2011-11-07 10:36) - PLID 46309
	bool m_bRowNameFieldAvailable; // (r.gonet 04/29/2013) - PLID 44897 - Should we show the table specific row/column name fields?
	bool m_bIsTableFlipped; // (r.gonet 04/29/2013) - PLID 44897 - Which orientation is the table? Assuming it is a table.
	afx_msg void OnBnClickedEditSentenceInsertField();
	CNxColor m_nxcolor;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
};
