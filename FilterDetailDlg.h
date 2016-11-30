#if !defined(AFX_FILTERDETAILDLG_H__3F331A21_46B0_11D4_957A_00C04F4C8415__INCLUDED_)
#define AFX_FILTERDETAILDLG_H__3F331A21_46B0_11D4_957A_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterDetailDlg.h : header file
//

#include "FilterDetail.h"

// Specific messages

/////////////////////////////////////////////////////////////////////////////
// CFilterDetailDlg dialog
class CFilterFieldInfo;
enum FieldOperatorEnum;
class CFilterDetail;

class CFilterDetailDlg : public CNxDialog, public CFilterDetail
{
// Construction
public:
	CFilterDetailDlg(CWnd* pParent, WPARAM nDetailData, long nFilterType, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&));   // standard constructor

// Handy members
public:
	virtual void CFilterDetailDlg::SetUseOr(bool bUseOr);

// More implementation
public:
	void RefreshOperator();
	void RefreshValue();
	void RefreshFieldType();
	void RefreshUseOr();
	WPARAM m_nDetailData;
	bool m_bUseOrAfter;

	// Used when multiple items are chosen for the detail
	CString m_strMultiItemValues;

	// (j.armen 2012-06-20 15:23) - PLID 49607 - Corrected Spelling
	void DoMultipleSelectDlg();

	long m_lCurSelIndex;
	
	CString StripNonPhoneNumber(CString strIn);

	NXDATALISTLib::_DNxDataListPtr m_dlValueList;
// Dialog Data
	//{{AFX_DATA(CFilterDetailDlg)
	enum { IDD = IDD_FILTER_DETAIL_DLG };
	CComboBox	m_cboOperators;
	CNxLabel	m_nxlMultiValueListLabel;
	CNxEdit	m_nxeditValueEdit;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_dlFields;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Interface
public:
	virtual void SetPosition(long nIndexPosition); // Stores the new position and moves the windows around
	virtual void Refresh(); // Puts the underlying values on the screen
	virtual bool Store(); // Stores the values on screen to the base variables
	
// Implementation
protected:
	CStringArray m_nextechFilter;

	BOOL (WINAPI* m_pfnIsActionSupported)(SupportedActionsEnum, long);
	BOOL (WINAPI* m_pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*);

	CString CreateStringListFromIDs(CString strIDList);

// Implementation
protected:
	bool AllowFilter(CString name);
	void OnDrawUseOrBtn(CDC *pdc, UINT nAction, UINT nState, HWND hWnd);
	// Generated message map functions
	//{{AFX_MSG(CFilterDetailDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeOperatorCombo();
	afx_msg void OnClickOrDoubleClickUseOrBtn();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnFieldBtn();
	afx_msg void OnSelChosenField(long nRow);
	afx_msg void OnEditSubfilter();
	afx_msg void OnAdd();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnKillfocusValueEdit();
	afx_msg void OnSelChosenValueList(long nRow);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILTERDETAILDLG_H__3F331A21_46B0_11D4_957A_00C04F4C8415__INCLUDED_)
