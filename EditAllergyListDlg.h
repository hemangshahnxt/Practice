#if !defined(AFX_EDITALLERGYLISTDLG_H__8ED1B0E5_3C50_4148_B0D0_7FC8CF855B2B__INCLUDED_)
#define AFX_EDITALLERGYLISTDLG_H__8ED1B0E5_3C50_4148_B0D0_7FC8CF855B2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditAllergyListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditAllergyListDlg dialog

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

class CEditAllergyListDlg : public CNxDialog
{
// Construction
public:
	CEditAllergyListDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr  m_pAllergyList;

// Dialog Data
	//{{AFX_DATA(CEditAllergyListDlg)
	enum { IDD = IDD_EDIT_ALLERGY_LIST };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnEdit;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	// (b.savon 2012-09-26 16:58) - PLID 52874
	CNxColor m_nxcBack;
	//TES 5/9/2013 - PLID 56631
	CNxIconButton m_btnUpdateAllAllergies;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditAllergyListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2007-04-04 16:07) - PLID 25498 - This function is called to change the name
	// of an allergy. We must update the EMR data only.
	void ExecuteNameChange(long nAllergyID, const CString& strNewName);

	// (c.haag 2007-04-04 16:16) - PLID 25498 - This function is called to add an allergy to the
	// AllergyT table. We must update the EMR data as well.
	void ExecuteAddition(long nAllergyID);

	// (c.haag 2007-04-04 16:21) - PLID 25498 - This function is called to remove an allergy from
	// the allergy table. We must update the EMR data as well.
	void ExecuteDeletion(long nAllergyID);

	// (c.haag 2007-04-09 14:53) - PLID 25504 - This function is called to activate or inactivate an 
	// allergy in the AllergyT table. We must update the EMR data as well.
	void ExecuteChangeActiveFlag(long nAllergyID, BOOL bActivate);

	// (b.savon 2012-07-24 09:56) - PLID 51734 - Allow free text allergies and imported allergies from FDB
	void OnAddFreeTextAllergy();
	void OnAddImportedFDBAllergy();

protected:
	// (c.haag 2007-04-04 17:23) - PLID 25498 - This function determines if there is exactly one
	// active allergy in Practice, and returns TRUE if there is not. If necessary, the allergy
	// list will also be refreshed.
	BOOL HasMultipleActiveAllergies();

	// Generated message map functions
	//{{AFX_MSG(CEditAllergyListDlg)
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnEditingFinishedEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, long bCommit);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishingEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnLButtonUpEditList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedEditList(short nFlags);
	afx_msg void OnUpdateAllAllergies();
	afx_msg void OnRButtonDownEditList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITALLERGYLISTDLG_H__8ED1B0E5_3C50_4148_B0D0_7FC8CF855B2B__INCLUDED_)
