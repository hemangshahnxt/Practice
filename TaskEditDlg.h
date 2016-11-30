#if !defined(AFX_TASKEDITDLG_H__853B35E1_9D8F_11D2_AB76_544307C10000__INCLUDED_)
#define AFX_TASKEDITDLG_H__853B35E1_9D8F_11D2_AB76_544307C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TaskEditDlg.h : header file
//

#include "client.h"

/////////////////////////////////////////////////////////////////////////////
// CTaskEditDlg dialog

#include "TodoUtils.h"

class CTaskEditDlg : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_user;
	NXDATALISTLib::_DNxDataListPtr m_description;
	NXDATALISTLib::_DNxDataListPtr m_priority;
	NXDATALISTLib::_DNxDataListPtr m_method;

	NXTIMELib::_DNxTimePtr m_nxtRemind;

	// (c.haag 2008-06-10 11:37) - PLID 11599 - Multiple assign to label
	CRect m_rcAssignToMultiple;

	bool m_isPatient;
	long m_iTaskID;
	// (a.walling 2008-07-07 17:46) - PLID 29900 - Stored person (patient/contact) id
	long m_nPersonID;
	//long m_nPatientID;	//this is only used for patients on new tasks to determine the color scheme
	CTaskEditDlg(CWnd* pParent);   // standard constructor
	CTableChecker m_tblCheckTask;

	// (j.gruber 2010-01-12 12:44) - PLID 20916 - patient coordinator
	long m_nPatientCoordID;

	// (c.haag 2010-05-21 13:48) - PLID 38827 - Be able to override the regarding type and ID
	long m_nRegardingIDOverride;
	TodoType m_RegardingTypeOverride;

	CString m_originalInfo;  //for auditing purposes

	// (c.haag 2008-06-10 11:42) - PLID 11599 - This array stores the current Assign To selections
	CArray<long,long> m_anAssignTo;

	// (c.haag 2008-07-03 12:29) - PLID 30615 - This array stores the Assign To selections as they were
	// when an existing todo alarm is open
	CArray<long,long> m_anExistingAssignTo;

	// (m.hancock 2006-07-12 11:16) - PLID 21353
	CString m_strDefaultNote;	//If set, this string will be placed into the note field on the task dialog.
	long m_nDefaultCategoryID;	//If set, the category on the new task will be set to this ID and will override the default follow up category preference.

public:
	BOOL m_bInvokedFromEMN; // (c.haag 2008-07-11 09:56) - PLID 30550
	CString m_strEMNDescription;
	long m_nEMNID;

public:
	// (c.haag 2008-08-18 16:47) - PLID 30607 - Set to TRUE if the todo was deleted
	BOOL m_bWasDeleted;

public:
	BOOL m_bWasLinkedToEMNWhenSaved; // (c.haag 2008-07-11 09:56) - PLID 30550

protected:
	TodoType m_RegardingType; // (c.haag 2008-07-11 09:55) - PLID 30550
	

protected:
	// (c.haag 2008-07-08 09:06) - PLID 30641 - If this is set to TRUE, then the dialog will be read-only
	BOOL m_bReadOnly;

public:
	// (m.hancock 2006-10-30 09:58) - PLID 21730 - Allow a default deadline and reminder date to be set.
	void SetDeadlineDate(COleDateTime dtDeadline);
	void SetReminderDate(COleDateTime dtReminder);

	// (c.haag 2010-05-21 13:48) - PLID 38827 - Be able to override the regarding type and ID
	void SetNewTaskRegardingOverrides(long nID, TodoType type);

	//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
	BOOL m_bIsNew; 	// Set this to TRUE if the alarm is being created for the first time (when a new m_iTaskID value is not created in CTaskEditDlg).

// Dialog Data
	//{{AFX_DATA(CTaskEditDlg)
	enum { IDD = IDD_TASK_EDIT_DLG };
	CNxIconButton	m_cancelButton;
	CNxIconButton	m_okButton;
	CNxIconButton	m_deleteButton;
	CNxEdit	m_editNotes;
	CDateTimePicker	m_completed;
	CDateTimePicker	m_deadline;
	CDateTimePicker	m_remind;
	CNxColor	m_bkg1;
	CNxLabel	m_nxlAssignToMultiple;
	NxButton	m_checkLinkTaskWithEMN;
	// (j.gruber 2010-01-12 13:27) - PLID 20916 - added pat coord color
	CNxStatic	m_stUserDescription;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskEditDlg)
	public:
	virtual int DoModal(bool setIsPatient = true);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2008-06-02 14:56) - PLID 11599 - Based on the current content of m_anAssignTo,
	// update the visibility of the Assign To combo
	void RefreshAssignToCombo();

	// (c.haag 2008-07-03 12:25) - PLID 30615 - This function encapsulates security checking, and saves
	// us from having to repeat code.
	BOOL CheckAssignToPermissions(ESecurityPermissionType type);

	// (c.haag 2008-06-10 12:30) - PLID 11599 - Returns the "Assign To" names
	CString GetAssignToNames();

	//(c.copits 2010-12-02) PLID 40794 - Permissions for individual todo alarm fields
	unsigned short BytePriorityString(long nPriorityIn);
	void ApplyIndividualPermissions();

protected:
	// Generated message map functions
	//{{AFX_MSG(CTaskEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDeadline(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChangingPriority(long FAR* nNewSel);
	afx_msg void OnSelChangingAssignto(long FAR* nNewSel);
	afx_msg void OnSelChangingMethod(long FAR* nNewSel);
	afx_msg void OnRequeryFinishedAssignto(short nFlags);
	afx_msg void OnSelChosenAssignto(long nRow);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelLButtonDown(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

	// (m.hancock 2006-10-30 09:58) - PLID 21730 - Allow a default deadline and reminder date to be set.
	BOOL m_bUseDefaultDeadline;
	COleDateTime m_dtDefaultDeadline;
	BOOL m_bUseDefaultRemindingDate;
	COleDateTime m_dtDefaultRemindingDate;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKEDITDLG_H__853B35E1_9D8F_11D2_AB76_544307C10000__INCLUDED_)
