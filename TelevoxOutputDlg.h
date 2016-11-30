#if !defined(AFX_TELEVOXOUTPUTDLG_H__1477B1DB_4B87_46EF_AC5C_3A886A6E590F__INCLUDED_)
#define AFX_TELEVOXOUTPUTDLG_H__1477B1DB_4B87_46EF_AC5C_3A886A6E590F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TelevoxOutputDlg.h : header file
//

#define DEFAULT_TELEVOX_FIELD_ORDER

// (z.manning 2008-07-10 16:40) - PLID 20543 - Simple struct to track TeleVox field info
struct TelevoxField
{
	long nID;
	CString strDisplay;
	BOOL bExport;

	TelevoxField()
	{
		bExport = TRUE;
	}
};

// (z.manning 2008-07-10 16:41) - PLID 20543 - This function loads all televox fields in
// their proper sort order.
void LoadTelevoxFieldsInOrder(OUT CArray<TelevoxField,TelevoxField&> &aryTelevoxFields);

/////////////////////////////////////////////////////////////////////////////
// CTelevoxOutputDlg dialog

class CTelevoxOutputDlg : public CNxDialog
{
// Construction
public:
	CTelevoxOutputDlg(CWnd* pParent);   // standard constructor

	void SetOutputSql(CString strSql);
	void IncludeEmail(BOOL bInclude);

	// (z.manning 2008-07-10 17:14) - PLID 20543 - Option from the TelevoxExportDlg
	BOOL m_bFailIfNot10Digits;
	BOOL m_bNoCellNoExport;
	BOOL m_bNoCellIfNoTextMsg; // (z.manning 2008-07-11 13:35) - PLID 30678

// Dialog Data
	//{{AFX_DATA(CTelevoxOutputDlg)
	enum { IDD = IDD_TELEVOX_OUTPUT_DLG };
	CNxLabel	m_lblWhyFailed;
	CNxEdit	m_nxeditTelevoxOutputPath;
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTelevoxOutputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pOutput;
	NXDATALISTLib::_DNxDataListPtr m_pFailed;
	CString m_strSql;
	CString m_strOutput;
	BOOL m_bUseEmail;

	void GeneratePatientInfo();

	// Generated message map functions
	//{{AFX_MSG(CTelevoxOutputDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBrowse();
	afx_msg LRESULT OnWhyFailed(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TELEVOXOUTPUTDLG_H__1477B1DB_4B87_46EF_AC5C_3A886A6E590F__INCLUDED_)
