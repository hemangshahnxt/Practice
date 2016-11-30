#if !defined(AFX_NEXWEBEXTRAPATIENTINFODLG_H__EABA2864_7FE6_4E05_99E2_531AB6D2AF68__INCLUDED_)
#define AFX_NEXWEBEXTRAPATIENTINFODLG_H__EABA2864_7FE6_4E05_99E2_531AB6D2AF68__INCLUDED_

#include "NexWebImportDlg.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebExtraPatientInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNexWebExtraPatientInfoDlg dialog

class CNexWebExtraPatientInfoDlg : public CNxDialog
{
// Construction
public:
	CNexWebExtraPatientInfoDlg(CWnd* pParent);   // standard constructor
	void SetPersonID(long nPersonID);
	void SaveInfo(long nPersonID = -1, BOOL bSkipOverwrites = FALSE);
	NXTIMELib::_DNxTimePtr m_dtIllness;
	void FillArrayWithFields(CPtrArray &paryFields);
	CString GetParameterName(DWORD nWindowID);
	CString GetParameterFieldValue(DWORD nWindowID);
	BOOL ValidateData();
	CString m_strError;

// Dialog Data
	//{{AFX_DATA(CNexWebExtraPatientInfoDlg)
	enum { IDD = IDD_NEXWEB_EXTRA_PATIENT_INFO_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditNexwebOccupation;
	CNxEdit	m_nxeditNexwebCompany;
	CNxEdit	m_nxeditNexwebManagerFname;
	CNxEdit	m_nxeditNexwebManagerMname;
	CNxEdit	m_nxeditNexwebManagerLname;
	CNxEdit	m_nxeditNexwebEmpAddress1;
	CNxEdit	m_nxeditNexwebEmpAddress2;
	CNxEdit	m_nxeditNexwebEmpCity;
	CNxEdit	m_nxeditNexwebEmpState;
	CNxEdit	m_nxeditNexwebEmpZip;
	CNxEdit	m_nxeditNexwebEmergencyFirstName;
	CNxEdit	m_nxeditNexwebEmergencyLastName;
	CNxEdit	m_nxeditNexwebEmergencyRelate;
	CNxEdit	m_nxeditNexwebEmergencyHome;
	CNxEdit	m_nxeditNexwebEmergencyWork;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebExtraPatientInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	void SaveWindowData(long nWindowID, CString &strPatientUpdate, CString &strPersonUpdate, BOOL bSkipOverwrites = FALSE);
	_variant_t GetField(CString strFieldName, CString strTableName, CString strIDField);
	void AddToAuditArray(UINT nWindowID, CString strUserName, CString strEntry);
	// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
	void AddToSaveMap(long nID);


	long m_nPersonID;
	CPtrArray m_paryAuditEvents;
	void InitializeControls(BOOL bEnabled /*=FALSE*/);
	void Load();
	void ProcessField(TransactionType transField, CString strEntry);
	//CArray<long, long>  m_IDsToSave;
	// (j.gruber 2006-11-03 09:19) - PLID 22978 - changed from an array to a map
	CMap<long, long, long, long> m_mapIDsToSave;

	// Generated message map functions
	//{{AFX_MSG(CNexWebExtraPatientInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocusNexwebCurrentIllnessDate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBEXTRAPATIENTINFODLG_H__EABA2864_7FE6_4E05_99E2_531AB6D2AF68__INCLUDED_)
