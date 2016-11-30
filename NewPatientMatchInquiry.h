#if !defined(AFX_NEWPATIENTMATCHINQUIRY_H__49D8AFAC_3117_404B_9837_7A6D809D1790__INCLUDED_)
#define AFX_NEWPATIENTMATCHINQUIRY_H__49D8AFAC_3117_404B_9837_7A6D809D1790__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewPatientMatchInquiry.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewPatientMatchInquiry dialog

// (d.moore 2007-08-15) - PLID 25455 - This dialog lists inquiries that match the
//  values for m_strFirstName, m_strLastName, or m_strEmail. The user can then
//  select an inquiry from the list and the PersonID for that inquiry will be 
//  stored in m_nPersonID for retrieval from outside the dialog.

class CNewPatientMatchInquiry : public CNxDialog
{
// Construction
public:
	CNewPatientMatchInquiry(CWnd* pParent);   // standard constructor

	// Variables used to pass data into the dialog.
	CString m_strFirstName;  // Each of these three variables should be set before opening
	CString m_strLastName;   //  the dialog. They will be used to search for inquiries
	CString m_strEmail;      //  with matching values.

	// Variable used to retrieve data from the dialog.
	long m_nPersonID;   // If there was a selection made from the inquiry list 
						//  then this will have a value greater than 0;


// Dialog Data
	//{{AFX_DATA(CNewPatientMatchInquiry)
	enum { IDD = IDD_NEW_PATIENT_MATCH_INQUIRY };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewPatientMatchInquiry)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	enum EInquiryListCols {
		eilPersonID, 
		eilUserDefinedID, 
		eilLastName, 
		eilFirstName, 
		eilMiddleName, 
		eilEmail, 
		eilProcedures, 
		eilReferral
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pInquiryList;

	// Generated message map functions
	//{{AFX_MSG(CNewPatientMatchInquiry)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedInquiryList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPATIENTMATCHINQUIRY_H__49D8AFAC_3117_404B_9837_7A6D809D1790__INCLUDED_)
