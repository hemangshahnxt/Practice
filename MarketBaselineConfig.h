#if !defined(AFX_MARKETBASELINECONFIG_H__2D74024B_FA33_42CC_B80C_BFDBD9018489__INCLUDED_)
#define AFX_MARKETBASELINECONFIG_H__2D74024B_FA33_42CC_B80C_BFDBD9018489__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketBaselineConfig.h : header file
//
#include "marketingrc.h"

/////////////////////////////////////////////////////////////////////////////
// CMarketBaselineConfig dialog

class CMarketBaselineConfig : public CNxDialog
{
// Construction
public:
	CMarketBaselineConfig(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMarketBaselineConfig)
	enum { IDD = IDD_MARKET_BASELINE_CONFIG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditEditConsultspermonth;
	CNxEdit	m_nxeditEditSurgeriespermonth;
	CNxEdit	m_nxeditEditClosureratio;
	CNxEdit	m_nxeditEditClosureduration;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketBaselineConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Load();
	BOOL Save();

	// Generated message map functions
	//{{AFX_MSG(CMarketBaselineConfig)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETBASELINECONFIG_H__2D74024B_FA33_42CC_B80C_BFDBD9018489__INCLUDED_)
