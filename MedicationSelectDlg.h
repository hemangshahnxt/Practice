#if !defined(AFX_MEDICATIONSELECTDLG_H__ADCA5FD9_4A24_4938_9DAC_3C7C72F7568F__INCLUDED_)
#define AFX_MEDICATIONSELECTDLG_H__ADCA5FD9_4A24_4938_9DAC_3C7C72F7568F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MedicationSelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMedicationSelectDlg dialog

class CMedicationSelectDlg : public CNxDialog
{
// Construction
public:
	CMedicationSelectDlg(CWnd* pParent);   // standard constructor
	long m_nMedicationID;
	// (b.savon 2012-09-05 11:34) - PLID 52454 - Propagate the FDBID for meds from import to current list.
	long m_nFDBID;

	NXDATALISTLib::_DNxDataListPtr  m_pMedList;
	void RequeryDialog();
	// (c.haag 2007-03-08 12:49) - PLID 25110 - Functions to get/set the patient
	// we are choosing the medication for
	void SetPatientID(long nPatID);
	long GetPatientID() const;
	// (a.wilson 2013-02-07 15:13) - PLID 55014 - use this to override the search for last med 
	//and set the selected medication on open.
	void SetInitialMedicationID(long nMedicationID);
	
	// (j.jones 2008-05-20 15:39) - PLID 30079 - removed the todo creation code

// Dialog Data
	//{{AFX_DATA(CMedicationSelectDlg)
	enum { IDD = IDD_MEDICATION_SELECT };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnEditMedList;
	CNxColor	m_bkg;
	CNxEdit	m_nxeditFirstdate;
	CNxStatic	m_nxstaticDatedesc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMedicationSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nPatientID; // (c.haag 2007-03-08 12:49) - PLID 25110 - The patient we are
					// choosing the medication for

	// Generated message map functions
	//{{AFX_MSG(CMedicationSelectDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEditMedList();
	virtual void OnCancel();
	afx_msg void OnSelChangedMedicationList(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void RequeryFinishedMedicationList(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDICATIONSELECTDLG_H__ADCA5FD9_4A24_4938_9DAC_3C7C72F7568F__INCLUDED_)
