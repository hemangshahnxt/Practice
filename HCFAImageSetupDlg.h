#if !defined(AFX_HCFAIMAGESETUPDLG_H__B0F2FE42_8ED6_4A02_BB53_5D4E0689A8EA__INCLUDED_)
#define AFX_HCFAIMAGESETUPDLG_H__B0F2FE42_8ED6_4A02_BB53_5D4E0689A8EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HCFAImageSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHCFAImageSetupDlg dialog

class CHCFAImageSetupDlg : public CNxDialog
{
// Construction
public:
	CHCFAImageSetupDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-05-08 09:29) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CHCFAImageSetupDlg)
	enum { IDD = IDD_HCFA_IMAGE_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton	m_radioUseNPIHCFAImage;
	NxButton	m_radioUseIntermediateHCFAImage;
	NxButton	m_radioUsePreNPIHCFAImage;
	NxButton	m_checkSendBox32NPI;
	NxButton	m_checkSwapDiagPlacement;
	NxButton	m_checkShowFacilityCode;
	NxButton	m_checkUseDecimals;
	// (j.jones 2009-08-04 12:26) - PLID 14573 - renamed to be regular payer ID
	NxButton	m_checkShowPayerID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHCFAImageSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2007-05-14 10:53) - PLID 25953 - added options for all three image types
	void OnImageTypeChanged();

	// (j.jones 2008-05-06 09:39) - PLID 29128 - this will hide all irrelevant controls
	// when on the NPI form type, even the type selections themselves
	void HideLegacyControls();

	// Generated message map functions
	//{{AFX_MSG(CHCFAImageSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckShowFacilityCode();
	afx_msg void OnRadioUseIntermediateHcfaImage();
	afx_msg void OnRadioUseNpiHcfaImage();
	afx_msg void OnRadioUsePreNpiHcfaImage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCFAIMAGESETUPDLG_H__B0F2FE42_8ED6_4A02_BB53_5D4E0689A8EA__INCLUDED_)
