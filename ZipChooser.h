#if !defined(AFX_ZIPCHOOSER_H__AFDCC1A4_6773_4ACA_AD9D_673F9084A772__INCLUDED_)
#define AFX_ZIPCHOOSER_H__AFDCC1A4_6773_4ACA_AD9D_673F9084A772__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZipChooser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZipChooser dialog

class CZipChooser : public CNxDialog
{
// Construction
public:
	CString m_strArea;
	CString m_strState;
	CString m_strCity;
	CString m_strZip;
	// (j.gruber 2009-10-05 16:00) - PLID 35607 - added fields to support city searching
	BOOL m_bSearchZip;
	CString m_strSearchField;
	CZipChooser(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pZipCodes;

// Dialog Data
	//{{AFX_DATA(CZipChooser)
	enum { IDD = IDD_ZIP_CHOOSER_DLG };
	CNxStatic	m_nxstaticZipText;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnMakePrimary;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZipChooser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.gruber 2009-10-06 14:19) - PLID 35607
	BOOL LoadFromZipFile();
	BOOL LoadFromCityFile();

	// Generated message map functions
	//{{AFX_MSG(CZipChooser)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnDblClickCellZipList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZIPCHOOSER_H__AFDCC1A4_6773_4ACA_AD9D_673F9084A772__INCLUDED_)
