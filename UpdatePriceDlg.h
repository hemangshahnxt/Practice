#if !defined(AFX_UPDATEPRICEDLG_H__D4AA3BEC_00DF_4151_B1C5_BEAFF22B955C__INCLUDED_)
#define AFX_UPDATEPRICEDLG_H__D4AA3BEC_00DF_4151_B1C5_BEAFF22B955C__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdatePriceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdatePriceDlg dialog

class CUpdatePriceDlg : public CNxDialog
{
// Construction
public:
	long nType;
	CUpdatePriceDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUpdatePriceDlg)
	enum { IDD = IDD_UPDATE_PRICE_DLG };
	NxButton	m_btnCategory;
	NxButton	m_btnRound;
	NxButton	m_btnSurgery;
	CNxEdit	m_nxeditUpdatePercentage;
	CNxStatic	m_nxstaticDescText;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdatePriceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_listCategory;

	// Generated message map functions
	//{{AFX_MSG(CUpdatePriceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCategoryCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEPRICEDLG_H__D4AA3BEC_00DF_4151_B1C5_BEAFF22B955C__INCLUDED_)
