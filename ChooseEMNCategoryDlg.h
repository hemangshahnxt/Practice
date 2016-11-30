#if !defined(AFX_CHOOSEEMNCATEGORYDLG_H__8E2C50BF_0DB9_4470_9765_F5883724D5E3__INCLUDED_)
#define AFX_CHOOSEEMNCATEGORYDLG_H__8E2C50BF_0DB9_4470_9765_F5883724D5E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseEMNCategoryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseEMNCategoryDlg dialog

class CChooseEMNCategoryDlg : public CNxDialog
{
// Construction
public:
	CChooseEMNCategoryDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseEMNCategoryDlg)
	enum { IDD = IDD_CHOOSE_EMN_CATEGORY_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	long m_nEMNCategoryID;
	CString m_strEMNCategoryName; // (z.manning, 05/07/2007) - PLID 25731 - Get the name as well.

	// (z.manning, 06/04/2007) - PLID 26214 - Chart ID to filter on.
	long m_nChartID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseEMNCategoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_EMNCategoriesList;

	// Generated message map functions
	//{{AFX_MSG(CChooseEMNCategoryDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEEMNCATEGORYDLG_H__8E2C50BF_0DB9_4470_9765_F5883724D5E3__INCLUDED_)
