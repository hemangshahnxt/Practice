#if !defined(AFX_CUSTOMRECORDDETAILDLG_H__BF1441D0_0189_46FE_9E21_54491234AA18__INCLUDED_)
#define AFX_CUSTOMRECORDDETAILDLG_H__BF1441D0_0189_46FE_9E21_54491234AA18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomRecordDetailDlg.h : header file
//

// These must be related in that you must make sure the dialog 
// height fits ITEM_COUNT_PER_PAGE items each of ITEM_SIZE_VERT closely
#define EMR_ITEM_SIZE_VERT			26
#define EMR_ITEM_COUNT_PER_PAGE	9

// Logically this should always be 5 because it should take 5 clicks 
// on the up or down scroll to move the distance of a single item
#define EMR_SCROLL_POS_PER_ITEM		6

// Handy Automated Calculations
#define EMR_SCROLL_POS_PER_PAGE		((long)(EMR_ITEM_COUNT_PER_PAGE * EMR_SCROLL_POS_PER_ITEM))
#define EMR_SCROLL_POS_HEIGHT		((long)(EMR_ITEM_SIZE_VERT / EMR_SCROLL_POS_PER_ITEM))
#define EMR_SCROLL_TOP_POS			((long)0)
#define CUSTOM_RECORD_SCROLL_BOTTOM_POS		((long)(m_aryEMRItemDlgs.GetSize() * EMR_SCROLL_POS_PER_ITEM - EMR_SCROLL_POS_PER_PAGE + 1))

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordDetailDlg dialog

class CCustomRecordDetailDlg : public CDialog
{
// Construction
public:
	void RefreshAllItems();
	void ClearDetails();
	void DeleteEMRItem(long index);
	CCustomRecordDetailDlg(CWnd* pParent);   // standard constructor
	~CCustomRecordDetailDlg();

	CString m_strInfoWhereClause;
	void LoadInfoWhereClause();

	void AddDetail(long InfoID = -1, long DetailID = -1, BOOL bIsNew = TRUE);

	void TryAddBlankDetail();
	void TryRemoveBlankDetail();
	BOOL IsInfoInList(long InfoID);

	void AddDiagCodeID(long DiagID);
	void RemoveDiagCodeID(long DiagID);

	//stores the procedure ID of this EMR
	long m_ProcedureID;

	//stores the DiagCodeID(s) of this EMR
	CDWordArray m_dwDiagCodeIDs;

	long m_EMRID;

	CPtrArray m_aryEMRItemDlgs;

	CDWordArray m_aryDeletedDetails;

	long DoScrollTo(long nNewTopPos);

	long m_nScrolledHeight;

	void RefreshScrollBar();

// Dialog Data
	//{{AFX_DATA(CCustomRecordDetailDlg)
	enum { IDD = IDD_CUSTOM_RECORD_DETAIL_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomRecordDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCustomRecordDetailDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMRECORDDETAILDLG_H__BF1441D0_0189_46FE_9E21_54491234AA18__INCLUDED_)

