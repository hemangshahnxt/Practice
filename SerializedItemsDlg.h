//PLID 28380	r.galicki	6/11/08		-  View Serialized Items dialog

#if !defined(AFX_SERIALIZEDITEMSDLG_H__6E4FCA45_AB3C_4E63_9760_C62858DF3849__INCLUDED_)
#define AFX_SERIALIZEDITEMSDLG_H__6E4FCA45_AB3C_4E63_9760_C62858DF3849__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SerializedItemsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSerializedItemsDlg dialog

class CSerializedItemsDlg : public CNxDialog
{
// Construction
public:
	CSerializedItemsDlg(CWnd* pParent);   // standard constructor
	CSerializedItemsDlg(CWnd* pParent, long nID = -1, BOOL bIsBill = FALSE);

// Dialog Data
	//{{AFX_DATA(CSerializedItemsDlg)
	enum { IDD = IDD_SERIALIZED_ITEMS };
	CNxStatic	m_lbl_Info;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSerializedItemsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pItemList;

	long m_nID;
	BOOL m_bIsBill;

	// Generated message map functions
	//{{AFX_MSG(CSerializedItemsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIALIZEDITEMSDLG_H__6E4FCA45_AB3C_4E63_9760_C62858DF3849__INCLUDED_)
