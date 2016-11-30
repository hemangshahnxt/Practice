#if !defined(AFX_HL7LINKMULTIPLEPATIENTSDLG_H__B6D0A8A2_9453_48D5_9A99_3F7DAD494CB0__INCLUDED_)
#define AFX_HL7LINKMULTIPLEPATIENTSDLG_H__B6D0A8A2_9453_48D5_9A99_3F7DAD494CB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7LinkMultiplePatientsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHL7LinkMultiplePatientsDlg dialog

// (r.gonet 12/11/2012) - PLID 53798 - Use forward declaration instead of an include
//  to reduce rebuilds.
struct HL7_PIDFields;

//TES 8/9/2007 - PLID 26892 - This dialog allows the user to select corresponding Nextech patients for a set of multiple
// incoming HL7 patients which couldn't be automatically linked.
class CHL7LinkMultiplePatientsDlg : public CNxDialog
{
// Construction
public:
	CHL7LinkMultiplePatientsDlg(CWnd* pParent);   // standard constructor

	//Input
	//Adds a PID segment that needs linked.
	void AddPID(HL7_PIDFields &PID);
	//Used for formatting the labels.
	CString m_strHL7GroupName;

	//Output, maps all the HL7 IDs included in any PIDs that were added to valid PersonT.IDs in Nextech.  Will be filled
	// if the dialog returns IDOK, empty otherwise.
	CMapStringToPtr m_mapHL7IDToNextechID;

	//Accessor
	int GetPatientCount();
	

// Dialog Data
	//{{AFX_DATA(CHL7LinkMultiplePatientsDlg)
	enum { IDD = IDD_HL7_LINK_MULTIPLE_PATIENTS_DLG };
	CNxIconButton	m_nxbLink;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnHelp;
	CNxStatic	m_nxstaticMassLinkCaption;
	CNxStatic	m_nxstaticHl7PatientsCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7LinkMultiplePatientsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Stores all the PID fields we've been asked to link.
	CArray<HL7_PIDFields, HL7_PIDFields&> m_arPIDsToLink;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pPracticePatientList, m_pHL7PatientList;

	//TES 10/2/2007 - PLID 26892 - Makes sure the Link button is appropriately enabled.
	void EnableButtons();

	//Are we currently filtering the Nextech Patients list, or are we showing all?
	bool m_bPatientListFiltered;
	// Generated message map functions
	//{{AFX_MSG(CHL7LinkMultiplePatientsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedHl7PatientList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	virtual void OnCancel();
	afx_msg void OnLink();
	afx_msg void OnOk();
	afx_msg void OnRequeryFinishedPracticePatientList(short nFlags);
	afx_msg void OnLeftClickPracticePatientList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnMatchingInfo();
	afx_msg void OnSelChangedPracticePatientList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7LINKMULTIPLEPATIENTSDLG_H__B6D0A8A2_9453_48D5_9A99_3F7DAD494CB0__INCLUDED_)
