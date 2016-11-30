#if !defined(AFX_EDITMEDICATIONLISTDLG_H__8D5C119B_2D0F_4704_B6A7_9242E7E3EC0E__INCLUDED_)
#define AFX_EDITMEDICATIONLISTDLG_H__8D5C119B_2D0F_4704_B6A7_9242E7E3EC0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditMedicationListDlg.h : header file
//

// (j.jones 2014-07-29 10:09) - PLID 63085 - forward declarations
// in the Accessor namespaces allow us to remove .h includes
namespace NexTech_Accessor
{
	enum DrugType;
}

/////////////////////////////////////////////////////////////////////////////
// CEditMedicationListDlg dialog

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

class CEditMedicationListDlg : public CNxDialog
{
private:
	// (b.savon 2013-07-31 17:28) - PLID 57799 - Flag that holds if there are any FDB meds
	long m_nDrugListHasOutOfDateFDBMed;

// Construction
public:
	CEditMedicationListDlg(CWnd* pParent);   // standard constructor
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

// Dialog Data
	//{{AFX_DATA(CEditMedicationListDlg)
	enum { IDD = IDD_EDIT_MEDICATION_LIST };
	CNxIconButton	m_btnLatinNotation;
	CNxIconButton	m_Inactivate;
	CNxIconButton	m_Show_Inactive;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnEdit;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnHelp;
	NxButton	m_btnShowAdvDrugNames;
	NxButton	m_btnShowDrugNotes;
	// (b.savon 2013-01-21 12:37) - PLID 54722
	CNxColor	m_nxcBack;
	//TES 5/9/2013 - PLID 56631
	CNxIconButton m_btnUpdateAllMedications;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMedicationListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2007-01-30 16:26) - PLID 24422 - This function is called to change the name
	// of a medication. We must update the EMR data as well as the Druglist table
	void ExecuteNameChange(long nDrugID, const CString& strNewName);

	// (c.haag 2007-01-30 16:32) - PLID 24422 - This function is called to add a medication to the DrugList
	// table. We must update the EMR data as well.
	// (d.thompson 2008-12-01) - PLID 32175 - Now requires the drug description fields as well.
	// (d.thompson 2009-03-17) - PLID 33477 - Return value is now the QuantityUnitID
	// (s.dhole 2012-11-16 11:00) - PLID 53698 Added varQuntityUnit and no retunr value
	// (j.fouts 2012-11-27 16:46) - PLID 51889 - Added Notes and Drug Type
	// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
	// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added Dosage Unit
	void ExecuteAddition(long nDrugID, CString strFullDrugDescription, CString strDrugName, CString strStrength, 
			_variant_t varStrengthUnitID, _variant_t varDosageFormID,_variant_t  varQuntityUnit, CString strNotes, NexTech_Accessor::DrugType drugType,
			_variant_t varDosageUnitID, _variant_t varDosageRouteID);

	// (c.haag 2007-01-30 16:32) - PLID 24422 - This function is called to remove a medication from the DrugList
	// table. We must update the EMR data as well.
	void ExecuteDeletion(long nDrugID);

	// (c.haag 2007-01-30 17:09) - PLID 24422 - This function is called to inactivate a medication
	// in the DrugList table. We must update the EMR data as well.
	void ExecuteInactivation(long nDrugID);


	// (j.gruber 2007-08-09 13:20) - PLID 26973 - added for deleting, renaming meds
	BOOL CEditMedicationListDlg::IsMedicationInUse(long nMedicationID, CString &strCounts);

	// (j.gruber 2010-10-27 13:10) - PLID 39049
	void OnImportMedications();
	void OnAddFreeTextMed();

protected:
	// (c.haag 2007-03-05 09:31) - PLID 25056 - This function determines if there is exactly one
	// active medication in Practice, and returns TRUE if there is not. If necessary, the medication
	// list will also be refreshed.
	BOOL HasMultipleActiveMedications();

	//DRT 11/25/2008 - PLID 32175 - Ensures controls are properly enabled/disabled
	void EnsureControls();

	// (d.thompson 2008-12-03) - PLID 32175 - Helper to apply link style to combo columns
	void ApplyLinkStyleToCell(NXDATALIST2Lib::IRowSettingsPtr pRow, short nColumn);

	// (a.walling 2009-11-16 13:24) - PLID 36239 - Prevent prompting all the time if the database is inaccessible
	bool m_bHasEnsuredFirstDataBankOnce; 

protected:
	// Generated message map functions
	//{{AFX_MSG(CEditMedicationListDlg)
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnMarkMedicationInactive();
	afx_msg void OnShowInactiveMeds();
	afx_msg void OnBtnEditLatinNotation();
	afx_msg void OnBnClickedShowDrugNotes();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnEditingFinishedEditList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnEditingFinishingEditList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnSelChangedEditList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnLButtonUpEditList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnRequeryFinishedEditList(short nFlags);
	afx_msg void OnBnClickedMedShowAdvDrugNames();
	void OnColumnClickingMedicationList(short nCol, BOOL* bAllowSort);
	LRESULT OnDelayedDontShow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedAdvMedHelp();
public:
	void EditingStartingEditList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateAllMedications();
	void RButtonUpEditList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITMEDICATIONLISTDLG_H__8D5C119B_2D0F_4704_B6A7_9242E7E3EC0E__INCLUDED_)
