#if !defined(AFX_MARKETFILTERPICKERDLG_H__FF713095_393E_4B1B_8790_6B5127182B8C__INCLUDED_)
#define AFX_MARKETFILTERPICKERDLG_H__FF713095_393E_4B1B_8790_6B5127182B8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketFilterPickerDlg.h : header file
//

#include "MarketUtils.h"
/////////////////////////////////////////////////////////////////////////////
// CMarketFilterPickerDlg dialog

class CMarketFilterPickerDlg : public CNxDialog
{
// Construction
public:
	CMarketFilterPickerDlg(int mktType, MarketFilterType mfType, CWnd* pParent);   // standard constructor
	CMarketFilterPickerDlg(CDWordArray &dwAllowedFilters, MarketFilterType mfType, CWnd* pParent);

	int m_mktType;
	NXDATALISTLib::_DNxDataListPtr  m_pMarketFilter;
	MarketFilter m_Filter;
	MarketFilterType m_mfType;

// Dialog Data
	//{{AFX_DATA(CMarketFilterPickerDlg)
	enum { IDD = IDD_MARKET_FILTER_PICKER_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketFilterPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CDWordArray m_dwAllowedFilters;

	// Generated message map functions
	//{{AFX_MSG(CMarketFilterPickerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenMarketFilterList(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETFILTERPICKERDLG_H__FF713095_393E_4B1B_8790_6B5127182B8C__INCLUDED_)
