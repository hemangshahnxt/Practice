#if !defined(AFX_ZIPENTRY_H__E081876C_6AC6_11D3_AD6A_00104B318376__INCLUDED_)
#define AFX_ZIPENTRY_H__E081876C_6AC6_11D3_AD6A_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZipEntry.h : header file

/////////////////////////////////////////////////////////////////////////////
// CZipEntry dialog

class CZipEntry : public CNxDialog
{
// Construction
public:
	CZipEntry();   // standard constructor
	CString m_strZip, m_strCity, m_strState, m_strAreaCode;
	long m_nNewID;

// Dialog Data
	//{{AFX_DATA(CZipEntry)
	enum { IDD = IDD_ZIP_ENTRY };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditZip;
	CNxEdit	m_nxeditCity;
	CNxEdit	m_nxeditState;
	CNxEdit	m_nxeditAreaCode;
	CNxStatic	m_nxstaticText2;
	CNxStatic	m_nxstaticText1;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZipEntry)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
	
	

protected:

	// Generated message map functions
	//{{AFX_MSG(CZipEntry)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZIPENTRY_H__E081876C_6AC6_11D3_AD6A_00104B318376__INCLUDED_)
