#include "client.h"
#if !defined(AFX_NEWCONTACT_H__E7654303_69DF_11D3_AD6A_00104B318376__INCLUDED_)
#define AFX_NEWCONTACT_H__E7654303_69DF_11D3_AD6A_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewContact.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewContact dialog

class CNewContact : public CNxDialog
{
// Construction
public:
	CNewContact(CWnd* pParent);   // standard constructor

	BOOL m_bSaveEditEnable;
// Dialog Data
	//{{AFX_DATA(CNewContact)
	enum { IDD = IDD_NEW_CONTACT };
	NxButton m_male; 
	NxButton m_female;
	NxButton m_btnMain;
	NxButton m_btnRef;
	NxButton m_btnSup;
	NxButton m_btnOther;
	NxButton m_btnEmp;
	CNxIconButton m_btnSaveEdit;
	CNxIconButton m_btnSaveResume;
	CNxIconButton m_btnCancel;
	CNxEdit	m_nxeditEmployerBox;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditAddress1Box;
	CNxEdit	m_nxeditAddress2Box;
	CNxEdit	m_nxeditZipBox;
	CNxEdit	m_nxeditCityBox;
	CNxEdit	m_nxeditStateBox;
	CNxEdit	m_nxeditHomePhoneBox;
	CNxEdit	m_nxeditWorkPhoneBox;
	CNxEdit	m_nxeditExtPhoneBox;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticIdLabel12;
	CNxEdit m_nxeditNpi;
	CNxStatic m_nxstaticNpiLabel;
	//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
	CNxEdit m_nxeditFaxBox;
	CNxEdit m_nxeditEmailBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides	
	//{{AFX_VIRTUAL(CNewContact)
	public:
	virtual int DoModal(long *i, long nDefType = 0 /* = 0 */);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool Save();
	long *m_pID;
	long m_nDefaultType;
	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat, m_strAreaCode;

	// (z.manning 2008-12-04 17:51) - PLID 28277 - Shows/hides fields based on the current contact type
	void UpdateTypeSpecificFields();

	// (j.gruber 2009-10-08 09:56) - PLID 35826 - Override the tab order if they have city lookup	
	BOOL m_bLookupByCity;

	// (j.gruber 2009-10-07 17:10) - PLID 35826 - added kill focus city function
	// Generated message map functions
	//{{AFX_MSG(CNewContact)
	afx_msg void OnKillfocusZipBox();
	afx_msg void OnKillfocusCityBox();
	virtual BOOL OnInitDialog();
	afx_msg void OnSaveAndEdit();
	afx_msg void OnSaveAndResume();
	afx_msg void OnCancel();
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (z.manning 2008-12-05 08:44) - PLID 28277
	afx_msg void OnBnClickedMainBtn();
	afx_msg void OnBnClickedEmployeeBtn();
	afx_msg void OnBnClickedRefphysBtn();
	afx_msg void OnBnClickedSupplierBtn();
	afx_msg void OnBnClickedOtherBtn();
	//(c.copits 2011-09-22) PLID 45626 - Validate Email Addresses
	afx_msg void OnKillfocusEmailBox();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWCONTACT_H__E7654303_69DF_11D3_AD6A_00104B318376__INCLUDED_)
