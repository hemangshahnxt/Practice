#if !defined(AFX_SELECTPREPAYDLG_H__5720A02F_60A0_482C_B21B_4441CA960FCF__INCLUDED_)
#define AFX_SELECTPREPAYDLG_H__5720A02F_60A0_482C_B21B_4441CA960FCF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectPrepayDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectPrepayDlg dialog

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

class CSelectPrepayDlg : public CNxDialog
{
// Construction
public:
	CSelectPrepayDlg(CWnd* pParent);   // standard constructor

	ADODB::_RecordsetPtr m_rsPrepays;
	CArray<int, int> m_arSelectedIds;

// Dialog Data
	//{{AFX_DATA(CSelectPrepayDlg)
	enum { IDD = IDD_SELECT_PREPAY };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectPrepayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pPrepayList;

	// Generated message map functions
	//{{AFX_MSG(CSelectPrepayDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPREPAYDLG_H__5720A02F_60A0_482C_B21B_4441CA960FCF__INCLUDED_)
