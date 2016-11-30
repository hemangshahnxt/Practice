#if !defined(AFX_HL7SELECTPATIENTDLG_H__4D4195D1_4031_41BE_B02B_30F8848CF8A5__INCLUDED_)
#define AFX_HL7SELECTPATIENTDLG_H__4D4195D1_4031_41BE_B02B_30F8848CF8A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7SelectPatientDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHL7SelectPatientDlg dialog

class CHL7SelectPatientDlg : public CNxDialog
{
// Construction
public:
	CHL7SelectPatientDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-05-08 10:07) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CHL7SelectPatientDlg)
	enum { IDD = IDD_HL7_SELECT_PATIENT };
	CNxIconButton	m_btnCreateNew;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticTopCaption;
	CNxStatic	m_nxstaticHl7First;
	CNxStatic	m_nxstaticHl7Middle;
	CNxStatic	m_nxstaticHl7Last;
	CNxStatic	m_nxstaticHl7Birthdate;
	CNxStatic	m_nxstaticHl7Ssn;
	NxButton	m_btnHl7Group;
	//}}AFX_DATA
	long m_nPersonID;
	CString m_strFirstName, m_strMiddleName, m_strLastName, m_strSSN, m_strGroupName;
	// (s.dhole 2013-08-13 14:33) - PLID 54572  added gender, phone and sex
	long m_nGender;
	CString  m_strZip , m_strPhone ; 
	COleDateTime m_dtBirthDate;
	//TES 2/24/2012 - PLID 48395 - You can now pass in a person ID to preselect in the dropdown list.
	long m_nPreselectedPersonID;

	//TES 7/11/2007 - PLID 26642 - Set this to true if you want "Create New" to be an option. This dialog
	// does not create the new patient itself, but if m_bAllowCreateNew is true, then it is possible for m_nPersonID
	// to be set to -2, which means that "Create New" was selected and the caller must therefore create the new patient.
	bool m_bAllowCreateNew;

	//TES 7/12/2007 - PLID 26642 - If this is set to true, the dialog will give a warning that patient information will
	// be overwritten, if they link to an existing patient.
	bool m_bWillOverwrite;

	//TES 9/23/2008 - PLID 21093 - Some sample lab messages we got had blank patient IDs; in that case we won't be able
	// to link the patients so we want to warn them of that.
	bool m_bWillLink;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7SelectPatientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pSelectedPatient;

	// Generated message map functions
	//{{AFX_MSG(CHL7SelectPatientDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCreateNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7SELECTPATIENTDLG_H__4D4195D1_4031_41BE_B02B_30F8848CF8A5__INCLUDED_)
