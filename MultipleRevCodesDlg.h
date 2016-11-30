#if !defined(AFX_MULTIPLEREVCODESDLG_H__A9BE3AFF_F89A_49FD_9D2A_248F84A40122__INCLUDED_)
#define AFX_MULTIPLEREVCODESDLG_H__A9BE3AFF_F89A_49FD_9D2A_248F84A40122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultipleRevCodesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultipleRevCodesDlg dialog

class CMultipleRevCodesDlg : public CNxDialog
{
// Construction
public:
	CMultipleRevCodesDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_InsCoList, m_RevCodeList;

	long m_nServiceID;

	void Save();
	void Load();

	BOOL m_bIsInv; //used to pass into the advanced revenue code setup

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CMultipleRevCodesDlg)
	enum { IDD = IDD_MULTIPLE_REV_CODES_DLG };
	CNxIconButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultipleRevCodesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultipleRevCodesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenComboInscos(long nRow);
	afx_msg void OnSelChosenUb92Revcodes(long nRow);
	virtual void OnOK();
	afx_msg void OnRequeryFinishedComboInscos(short nFlags);
	afx_msg void OnBtnAdvRevcodeSetup();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIPLEREVCODESDLG_H__A9BE3AFF_F89A_49FD_9D2A_248F84A40122__INCLUDED_)
