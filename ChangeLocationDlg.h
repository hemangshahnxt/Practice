#if !defined(AFX_CHANGELOCATIONDLG_H__D7DFF7B5_4AF7_4703_914D_EE37E811444F__INCLUDED_)
#define AFX_CHANGELOCATIONDLG_H__D7DFF7B5_4AF7_4703_914D_EE37E811444F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeLocationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeLocationDlg dialog

class CChangeLocationDlg : public CNxDialog
{
// Construction
public:
	CChangeLocationDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_pLocList;

// Dialog Data
	//{{AFX_DATA(CChangeLocationDlg)
	enum { IDD = IDD_CHANGE_LOCATION_DIALOG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticCurrLocationName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeLocationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeLocationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGELOCATIONDLG_H__D7DFF7B5_4AF7_4703_914D_EE37E811444F__INCLUDED_)
