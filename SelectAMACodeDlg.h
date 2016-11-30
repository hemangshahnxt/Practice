#if !defined(AFX_SELECTAMACODEDLG_H__30D37D2D_056B_4CEF_8E94_9693F5079A78__INCLUDED_)
#define AFX_SELECTAMACODEDLG_H__30D37D2D_056B_4CEF_8E94_9693F5079A78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectAMACodeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectAMACodeDlg dialog

class CSelectAMACodeDlg : public CNxDialog
{
// Construction
public:
	// (d.singleton 2011-10-04 16:41) - PLID 45904 add flag for ama vs alberta billing import
	CSelectAMACodeDlg(CWnd* pParent, BOOL bIsHLINK = FALSE);   // standard constructor

	long m_nChosen;
	CString m_strCode;
	CString m_strDesc;
	BOOL m_bAlwaysChoose;

// Dialog Data
	//{{AFX_DATA(CSelectAMACodeDlg)
	enum { IDD = IDD_SELECT_AMA_CODE_DLG };
	NxButton	m_btnAlwaysChoose;
	CNxStatic	m_nxstaticCodeDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectAMACodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;

	// Generated message map functions
	//{{AFX_MSG(CSelectAMACodeDlg)
	virtual BOOL OnInitDialog();
	// (d.singleton 2011-10-04 16:40) - PLID 45904 flag to see if its ama import or alberta billing import, or any generic import really
	BOOL m_bIsHLINK;
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTAMACODEDLG_H__30D37D2D_056B_4CEF_8E94_9693F5079A78__INCLUDED_)
