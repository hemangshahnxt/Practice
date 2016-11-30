#if !defined(AFX_ADVPRINTOPTIONS_H__BA19DA97_9BC0_4AC5_B9F0_E2A36706AF88__INCLUDED_)
#define AFX_ADVPRINTOPTIONS_H__BA19DA97_9BC0_4AC5_B9F0_E2A36706AF88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvPrintOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvPrintOptions dialog

class CAdvPrintOptions : public CNxDialog
{
// Construction
public:
	CAdvPrintOptions(CWnd* pParent);   // standard constructor
	~CAdvPrintOptions();

	// combo box options
	CString		m_cSQL,  // NOTE: do not use a semicolon w/ your SQL
				m_cColumnFormats,
				m_cColumnWidths,
				m_cDisplayFormat, // remember to include brackets
				m_cAllCaption,
				m_cSingleCaption;
	int			m_nResult;			//category ID returned
	int			m_cBoundCol;

	NXDATALISTLib::_DNxDataListPtr m_OptionList;

	// other options
	CString		m_btnCaption;
	COleDateTime m_dtInitFrom, m_dtInitTo;

	// (a.walling 2009-11-24 14:31) - PLID 36418
	CString		m_strSpecialCaption;
	NxButton*	m_pwndSpecialButton;

	//return values
	COleDateTime m_dtFromDate, m_dtToDate;

	//visibility options
	bool m_bDetailed, m_bOptionCombo, m_bDateRange, m_bAllOptions;

	CNxStatic m_FromDateCaption, m_ToDateCaption;
	NxButton m_rSingleOptions, m_rAllOptions, m_rDetailOptions, m_rSummaryOptions;
	

// Dialog Data
	//{{AFX_DATA(CAdvPrintOptions)
	enum { IDD = IDD_ADV_PRINT_OPTIONS };
	CDateTimePicker	m_FromDate;
	CDateTimePicker	m_ToDate;
	CNxStatic	m_nxstaticFromdateLabel;
	CNxStatic	m_nxstaticTodateLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvPrintOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvPrintOptions)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnClickAllOption();
	afx_msg void OnClickSingleOption();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVPRINTOPTIONS_H__BA19DA97_9BC0_4AC5_B9F0_E2A36706AF88__INCLUDED_)
