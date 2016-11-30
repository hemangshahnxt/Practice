#if !defined(AFX_EDITDEFAULTPOSCODESDLG_H__ABBDB66B_2326_48A7_9BF6_206314D8654C__INCLUDED_)
#define AFX_EDITDEFAULTPOSCODESDLG_H__ABBDB66B_2326_48A7_9BF6_206314D8654C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDefaultPOSCodesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditDefaultPOSCodesDlg dialog

class CEditDefaultPOSCodesDlg : public CNxDialog
{
// Construction
public:
	CEditDefaultPOSCodesDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_POSCombo, m_LocationCombo;

	long m_HCFASetupGroupID;

	void SavePOSLink();
	void LoadPOSLink();

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CEditDefaultPOSCodesDlg)
	enum { IDD = IDD_EDIT_DEFAULT_POS_CODES_DLG };
	CNxIconButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDefaultPOSCodesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditDefaultPOSCodesDlg)
	afx_msg void OnSelChosenPlaceofserviceCombo(long nRow);
	virtual void OnOK();
	afx_msg void OnEditPosCodes();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenComboLocations(long nRow);
	afx_msg void OnRequeryFinishedPlaceofserviceCombo(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITDEFAULTPOSCODESDLG_H__ABBDB66B_2326_48A7_9BF6_206314D8654C__INCLUDED_)
