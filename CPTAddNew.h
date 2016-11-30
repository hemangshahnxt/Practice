#if !defined(AFX_CPTADDNEW_H__F85BE533_3614_11D3_A36A_00C04F42E33B__INCLUDED_)
#define AFX_CPTADDNEW_H__F85BE533_3614_11D3_A36A_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CPTAddNew.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCPTAddNew dialog

class CCPTAddNew : public CNxDialog
{
// Construction
public:
	CCPTAddNew(CWnd* pParent);   // standard constructor
	CString strName, strCode, strSubCode;
	// (j.jones 2010-01-11 09:08) - PLID 24054 - exposed the price as a currency, not a string
	COleCurrency m_cyPrice;
	long m_nID;

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CCPTAddNew)
	enum { IDD = IDD_CPT_ADDNEW };
	NxButton	m_Taxable2;
	NxButton	m_Taxable1;
	CNxEdit	m_nxeditCode;
	CNxEdit	m_nxeditSubcode;
	CNxEdit	m_nxeditDesc;
	CNxEdit	m_nxeditFee;
	// (s.dhole 2012-03-02 17:57) - PLID 47399
	CNxEdit	m_nxeditCost;
	CNxEdit	m_nxeditTypeOfService;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCPTAddNew)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-11-12 16:10) - PLID 30702 - added procedure list
	NXDATALIST2Lib::_DNxDataListPtr m_ProcedureList;

	// Generated message map functions
	//{{AFX_MSG(CCPTAddNew)
	virtual void OnOK();
	afx_msg void OnOkBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusFee();
	afx_msg void OnKillfocusCost();
	afx_msg void OnTaxable1();
	afx_msg void OnTaxable2();
	virtual void OnCancel();
	afx_msg void OnKillfocusTypeOfService();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CWnd* m_pParent;
	COleCurrency cy;
	CString str;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPTADDNEW_H__F85BE533_3614_11D3_A36A_00C04F42E33B__INCLUDED_)
