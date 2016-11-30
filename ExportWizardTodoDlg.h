#if !defined(AFX_EXPORTWIZARDTODODLG_H__D1412FE6_066A_4AC5_B716_0D9EAC29B270__INCLUDED_)
#define AFX_EXPORTWIZARDTODODLG_H__D1412FE6_066A_4AC5_B716_0D9EAC29B270__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportWizardTodoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportWizardTodoDlg dialog

class CExportWizardTodoDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CExportWizardTodoDlg)

// Construction
public:
	CExportWizardTodoDlg();
	~CExportWizardTodoDlg();

// Dialog Data
	//{{AFX_DATA(CExportWizardTodoDlg)
	enum { IDD = IDD_EXPORT_WIZARD_TODO };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	CNxEdit	m_nxeditExportIntervalNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExportWizardTodoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pDateUnits, m_pUserList;

	//TES 6/27/2007 - PLID 26435 - Enables/disables fields appropriately based on which radio button is checked.
	void EnableFields();

	// Generated message map functions
	//{{AFX_MSG(CExportWizardTodoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnExportRunManual();
	afx_msg void OnExportUseInterval();
	afx_msg void OnSelChosenExportUserList(long nRow);
	afx_msg void OnChangeExportIntervalNumber();
	afx_msg void OnSelChosenExportIntervalUnit(long nRow);
	virtual BOOL OnSetActive();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTWIZARDTODODLG_H__D1412FE6_066A_4AC5_B716_0D9EAC29B270__INCLUDED_)
