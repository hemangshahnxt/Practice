#if !defined(AFX_HL7DUPLICATEREFPHYSDLG_H__D3359D85_DE8F_471A_A9CF_026D20B75715__INCLUDED_)
#define AFX_HL7DUPLICATEREFPHYSDLG_H__D3359D85_DE8F_471A_A9CF_026D20B75715__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7DuplicateRefPhysDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHL7DuplicateRefPhysDlg dialog

class CHL7DuplicateRefPhysDlg : public CNxDialog
{
// Construction
public:
	CHL7DuplicateRefPhysDlg(CWnd* pParent);   // standard constructor
	//TES 10/8/2008 - PLID 31414 - Added strRecord, for the user-readable type of the
	// record the physician will be assigned to, and bIsRefPhys, which can be set to
	// false to have this dialog search for Providers, rather than Referring Physicians.
	long Open(long nHL7GroupID, CString strThirdPartyID, CString strFirst, CString strLast, CString strTitle, bool bUseTitle, CString strRecord = "Patient", bool bIsRefPhys = true); 

	// (j.jones 2008-05-08 09:42) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CHL7DuplicateRefPhysDlg)
	enum { IDD = IDD_HL7_DUPLICATE_REFPHYS_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnNoRefPhy;
	CNxStatic	m_nxstaticPhysText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7DuplicateRefPhysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nHL7GroupID;
	CString m_strThirdPartyID;
	CString m_strFirst, m_strLast, m_strTitle;
	//TES 10/8/2008 - PLID 31414 - What type of record will this physician be assigned to?
	CString m_strRecord;
	bool m_bUseTitle;
	//TES 10/8/2008 - PLID 31414 - This dialog can now be used for duplicate providers
	// as well as duplicate referring physicians.
	bool m_bIsRefPhys;
	long m_nID;

	NXDATALISTLib::_DNxDataListPtr m_pList;

	// Generated message map functions
	//{{AFX_MSG(CHL7DuplicateRefPhysDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7DUPLICATEREFPHYSDLG_H__D3359D85_DE8F_471A_A9CF_026D20B75715__INCLUDED_)
