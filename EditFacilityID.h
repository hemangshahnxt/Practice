#if !defined(AFX_EDITFACILITYID_H__F25FE866_5CFE_448C_8886_89CB62C9E492__INCLUDED_)
#define AFX_EDITFACILITYID_H__F25FE866_5CFE_448C_8886_89CB62C9E492__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditFacilityID.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditFacilityID dialog

class CEditFacilityID : public CNxDialog
{
// Construction
public:
	CString m_strInsCo;
	NXDATALISTLib::_DNxDataListPtr m_ComboLocations;
	long m_iInsuranceCoID;
	long m_iLocationID;
	void LoadFacilityID();
	void SaveFacilityID();
	CEditFacilityID(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditFacilityID)
	enum { IDD = IDD_EDIT_FACILITY_ID };
	CString	m_FacilityID;
	CString m_Qualifier;
	CNxEdit	m_nxeditEditFacilityId;
	CNxEdit	m_nxeditEditFacilityIdQual;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdvFacilityIDEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditFacilityID)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditFacilityID)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenComboLocations(long nRow);
	afx_msg void OnRequeryFinishedComboLocations(short nFlags);
	afx_msg void OnKillfocusEditFacilityId();
	afx_msg void OnKillfocusEditFacilityIdQual();
	virtual void OnOK();
	afx_msg void OnAdvFacilityIdEdit();
	afx_msg void OnSelChangingComboLocations(long FAR* nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITFACILITYID_H__F25FE866_5CFE_448C_8886_89CB62C9E492__INCLUDED_)
