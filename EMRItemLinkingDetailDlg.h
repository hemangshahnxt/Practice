#if !defined(AFX_EMRITEMLINKINGDETAILDLG_H__33BC51F4_D503_4435_8148_68C4B057313E__INCLUDED_)
#define AFX_EMRITEMLINKINGDETAILDLG_H__33BC51F4_D503_4435_8148_68C4B057313E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRItemLinkingDetailDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRItemLinkingDetailDlg dialog

class CEMRItemLinkingDetailDlg : public CNxDialog
{
// Construction
public:
	CEMRItemLinkingDetailDlg(CWnd* pParent);   // standard constructor

	long m_nInfoID1;
	long m_nInfoID2;

// Dialog Data
	//{{AFX_DATA(CEMRItemLinkingDetailDlg)
	enum { IDD = IDD_EMR_ITEM_LINKING_DETAIL_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRItemLinkingDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void RefreshLinkList();
	void LoadExistingDetail();

	bool m_bIsNew;
	CDWordArray m_dwaryDeleteLinks;

	NXDATALIST2Lib::_DNxDataListPtr m_pList;	//main list
	NXDATALIST2Lib::_DNxDataListPtr m_pDetailOne;	//First detail
	NXDATALIST2Lib::_DNxDataListPtr m_pDetailTwo;	//Second detail

	// Generated message map functions
	//{{AFX_MSG(CEMRItemLinkingDetailDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenDetailListOne(LPDISPATCH lpRow);
	afx_msg void OnSelChosenDetailListTwo(LPDISPATCH lpRow);
	afx_msg void OnEditingFinishedLinkingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMLINKINGDETAILDLG_H__33BC51F4_D503_4435_8148_68C4B057313E__INCLUDED_)
