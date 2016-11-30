#if !defined(AFX_SELECTLOCATIONDLG_H__0B1CC3B5_956E_4C05_8CA2_93E39E8AC4DB__INCLUDED_)
#define AFX_SELECTLOCATIONDLG_H__0B1CC3B5_956E_4C05_8CA2_93E39E8AC4DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectLocationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectLocationDlg dialog

class CSelectLocationDlg : public CNxDialog
{
// Construction
public:
	CSelectLocationDlg(CWnd* pParent);   // standard constructor

	long m_nCurrentLocationID; //The location being inactivated.
	long m_nNewLocationID; //The location they selected.
	NXDATALISTLib::_DNxDataListPtr m_pNewLocation;
	CString m_strCaption;

// Dialog Data
	//{{AFX_DATA(CSelectLocationDlg)
	enum { IDD = IDD_SELECT_LOCATION_DLG };
	CNxStatic	m_nxstaticSelectLocationCaption;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectLocationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectLocationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTLOCATIONDLG_H__0B1CC3B5_956E_4C05_8CA2_93E39E8AC4DB__INCLUDED_)
