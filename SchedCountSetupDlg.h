// (c.haag 2009-12-22 16:00) - PLID 28977 - Initial implementation

#pragma once

// (c.haag 2009-12-22 16:00) - PLID 28977 - This class will load and save
// scheduler count settings
class CSchedulerCountSettings
{
private:
	// TRUE if we include non-patient appointments in counts
	BOOL m_bIncludeNonPtAppts;
	// This is a map of appointment types we want to HIDE from the counts. The key is the appointment
	// type ID, and the value is TRUE if we want to HIDE the type from the counts.
	CMap<long,long,BOOL,BOOL> m_mapAptTypesToHide;

public:
	CSchedulerCountSettings();
	CSchedulerCountSettings(BOOL bIncludeNonPtAppts, const CArray<long,long>& anIDsOfHiddenTypes);
public:
	inline BOOL GetIncludeNonPtAppts() { return m_bIncludeNonPtAppts; }
	inline BOOL IsPatientAllowed(long nPersonID) { 
		if (m_bIncludeNonPtAppts) {
			return TRUE; // If we include non-patient appts, then we include everybody
		} else if (nPersonID > 0) {
			return TRUE; // Otherwise, only return TRUE for positive person ID's
		} else {
			return FALSE;
		}
	}
	BOOL IsAptTypeAllowed(long nID) {
		BOOL bHidden = FALSE;
		m_mapAptTypesToHide.Lookup(nID, bHidden);
		return (bHidden) ? FALSE : TRUE;
	}
public:
	void Save();
};


// CSchedCountSetupDlg dialog

class CSchedCountSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSchedCountSetupDlg)

public:
	CSchedCountSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CSchedCountSetupDlg();

// Dialog Data
	enum { IDD = IDD_SCHED_COUNT_SETUP };

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dlAptType;
private:
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnUnselectAll;
	CNxStatic		m_nxStatic;
	NxButton		m_checkIncludeNonPtAppts;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnApptctSelectall();
	afx_msg void OnBnClickedBtnApptctUnselectall();
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedCountTypeList(short nFlags);
};
