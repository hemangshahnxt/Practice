#if !defined(AFX_ANESTHESIAINSCOSETUPDLG_H__13ADC8D7_4E96_4A4E_BCD0_7B8042CA5EF4__INCLUDED_)
#define AFX_ANESTHESIAINSCOSETUPDLG_H__13ADC8D7_4E96_4A4E_BCD0_7B8042CA5EF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnesthesiaInsCoSetupDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaInsCoSetupDlg dialog

class CAnesthesiaInsCoSetupDlg : public CNxDialog
{
// Construction
public:
	CAnesthesiaInsCoSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UnselectedList, m_SelectedList;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons for OK and Cancel
// Dialog Data
	//{{AFX_DATA(CAnesthesiaInsCoSetupDlg)
	enum { IDD = IDD_ANESTHESIA_INS_CO_SETUP_DLG };
	CNxIconButton	m_btnUnselectOneInsCo;
	CNxIconButton	m_btnUnselectAllInsCo;
	CNxIconButton	m_btnSelectAllInsCo;
	CNxIconButton	m_btnSelectOneInsCo;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnesthesiaInsCoSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnesthesiaInsCoSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSelectOneAnesthInsco();
	afx_msg void OnBtnSelectAllAnesthInsco();
	afx_msg void OnBtnUnselectOneAnesthInsco();
	afx_msg void OnBtnUnselectAllAnesthInsco();
	afx_msg void OnDblClickCellUnselectedAnesthInscoList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedAnesthInscoList(long nRowIndex, short nColIndex);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANESTHESIAINSCOSETUPDLG_H__13ADC8D7_4E96_4A4E_BCD0_7B8042CA5EF4__INCLUDED_)
