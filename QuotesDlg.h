#include "PatientDialog.h"
#if !defined(AFX_QUOTESDLG_H__BF36CEF3_4CCB_11D2_9844_00104B318376__INCLUDED_)
#define AFX_QUOTESDLG_H__BF36CEF3_4CCB_11D2_9844_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuotesDlg.h : header file
//

class CBillingModuleDlg;

/////////////////////////////////////////////////////////////////////////////
// CQuotesDlg dialog

class CQuotesDlg : public CPatientDialog
{
// Construction
public:
	void ResizeColumns();
	void SaveColumnWidths();
	//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 control
	NXDATALIST2Lib::_DNxDataListPtr m_QuoteList;
	//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 row pointer
	NXDATALIST2Lib::IRowSettingsPtr m_pRow;
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);	
	// (a.walling 2010-10-14 09:08) - PLID 40977
	virtual void Refresh();

	CQuotesDlg(CWnd* pParent);   // standard constructor
	virtual void SetColor(OLE_COLOR nNewColor);
// Dialog Data
	//{{AFX_DATA(CQuotesDlg)
	enum { IDD = IDD_QUOTES_DLG };
	NxButton	m_ShowPackageInfo;
	CNxIconButton	m_newQuoteButton;
	CNxColor	m_BackGround;
	NxButton	m_RememberColWidthsCheck;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_id; // (a.walling 2010-10-14 09:08) - PLID 40977

	CDWordArray m_aDefColumnStyles; // Tracks the default column styles of the quote list
	CDWordArray m_aDefColumnWidths; // Tracks the default column widths of the quote list

	// (a.walling 2008-06-03 10:54) - PLID 27686 - Hold the bitmap handle to be destroyed
	HBITMAP m_hBitmap;

	//(e.lally 2008-06-17) PLID 27303 - Removed headers for the datalist 1 version of the quote list events,
		//added new headers for datalist 2 version events, removed locally depreciated OnLButtonDown event

	//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	void GetBestUPCProduct(CString strCode);

	//updates the package totals in the currently displayed quote list
	void CalculateAllPackageTotals();

	// Generated message map functions
	//{{AFX_MSG(CQuotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLeftClickQuoteList(const VARIANT FAR& varBoundValue, long iColumn);	
	afx_msg void OnPopupSelectionQuoteList(long iItemID);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNewQuote();
	afx_msg void OnShowPackageInfo();
	afx_msg void OnRememberColumns();
	afx_msg long GetColumnWidth(const CString& strColWidths, short nColumn);
	afx_msg void OnOpenCareCredit();
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDownQuoteList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellQuoteList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRequeryFinishedQuoteList(short nFlags);
	afx_msg void OnColumnSizingFinished(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUOTESDLG_H__BF36CEF3_4CCB_11D2_9844_00104B318376__INCLUDED_)
