#if !defined(AFX_PACKAGES_H__8D82C52D_4439_463A_B37C_1410D98D61C1__INCLUDED_)
#define AFX_PACKAGES_H__8D82C52D_4439_463A_B37C_1410D98D61C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Packages.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPackages dialog

class CPackages : public CNxDialog
{
// Construction
public:
	void InsertSeparatorLine(long QuoteID);
	BOOL m_bShowAll;
	void Requery();
	void BillPackage(long QuoteID);
	void MakePrePayment(long QuoteID);
	void InsertBlankRow(long QuoteID);
	void FillList();

	NXDATALISTLib::_DNxDataListPtr m_List;

	BOOL Create();	

	CPackages(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPackages)
	enum { IDD = IDD_PACKAGE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPackages)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CWnd* m_pParent;
	int m_nID;

	//used to determine if the datalist is drawing
	BOOL m_bIsScreenEnabled;

	//reference count for the functions below
	long m_DisableRefCount;

	//will disable/enable drawing of the datalist
	void DisablePackageScreen();
	void EnablePackageScreen();

	// Generated message map functions
	//{{AFX_MSG(CPackages)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnShowAll();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PACKAGES_H__8D82C52D_4439_463A_B37C_1410D98D61C1__INCLUDED_)
