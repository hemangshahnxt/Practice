#if !defined(AFX_NEXWEBPATIENTINFODLG_H__0CEEF9DB_3D4E_4B48_8C57_DF90E21081BB__INCLUDED_)
#define AFX_NEXWEBPATIENTINFODLG_H__0CEEF9DB_3D4E_4B48_8C57_DF90E21081BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebPatientInfoDlg.h : header file
//

#include "NexWebImportDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CNexWebPatientInfoDlg dialog

class CNexWebPatientInfoDlg : public CNxDialog
{
// Construction
public:
	CNexWebPatientInfoDlg(CWnd* pParent);   // standard constructor
	void SetPersonID(long nPersonID);
	NXDATALISTLib::_DNxDataListPtr  m_pLocationList;
	NXDATALISTLib::_DNxDataListPtr  m_pPrefixList;
	NXDATALISTLib::_DNxDataListPtr  m_pPatientTypeList;
	NXDATALISTLib::_DNxDataListPtr  m_pReferralList;
	NXDATALISTLib::_DNxDataListPtr  m_pGenderList;
	NXDATALISTLib::_DNxDataListPtr  m_pMaritalStatusList;
	NXDATALIST2Lib::_DNxDataListPtr  m_pPatCoordList;
	NXTIMELib::_DNxTimePtr m_dtBDay;

	//(e.lally 2008-02-28) PLID 27379 - Added flag for overwriting existing data vs. updating if the data is blank
	void SaveInfo(long nPersonID = -1, BOOL bSkipOverwrites = FALSE);
	//(e.lally 2008-02-28) PLID 27379 - Added flag for overwriting existing data vs. updating if the data is blank
	void SaveWindowData(long nWindowID, CString &strPatientUpdate, CString &strPersonUpdate, long nPersonID, BOOL bSkipOverwrites);
	CString GetFieldString(CString strFieldName, CString strTableName, CString strIDField);
	_variant_t GetField(CString strFieldName, CString strTableName, CString strIDField);
	void AddToAuditArray(UINT nWindowID, CString strUserName, CString strEntry);
	void FillArrayWithFields(CPtrArray &paryFields);
	CString GetParameterName(DWORD nWindowID);
	CString GetParameterFieldValue(DWORD nWindowID);
	BOOL ValidateData();
	void SetRefListColors();
	//(e.lally 2008-02-28) PLID 27379 - Added flag for overwriting existing data vs. updating if the data is blank
	void ProcessReferral(long nRow, long nPersonID, BOOL bSkipOverwrites);
	void AddToSaveMap(long nID);

	CPtrArray m_paryAuditEvents;
	//CArray<long, long>  m_IDsToSave;
	// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
	//changed from array to map
	CMap<long, long, long, long> m_mapIDsToSave;

	CString m_strError;

// Dialog Data
	// (j.gruber 2008-06-03 11:02) - PLID 30235 - added NickName
	//{{AFX_DATA(CNexWebPatientInfoDlg)
	enum { IDD = IDD_NEXWEB_PATIENT_INFO_DLG };
	CNxEdit	m_nxeditNexwebNickName;
	CNxIconButton	m_btnAddRefSource;
	CNxEdit	m_nxeditNexwebId;
	CNxEdit	m_nxeditNexwebFirstName;
	CNxEdit	m_nxeditNexwebMiddleName;
	CNxEdit	m_nxeditNexwebLastName;
	CNxEdit	m_nxeditNexwebAddress1;
	CNxEdit	m_nxeditNexwebAddress2;
	CNxEdit	m_nxeditNexwebCity;
	CNxEdit	m_nxeditNexwebState;
	CNxEdit	m_nxeditNexwebZip;
	CNxEdit	m_nxeditNexwebSsn;
	CNxEdit	m_nxeditNexwebSpouseName;
	CNxEdit	m_nxeditNexwebHomePhone;
	CNxEdit	m_nxeditNexwebWorkPhone;
	CNxEdit	m_nxeditNexwebExtPhone;
	CNxEdit	m_nxeditNexwebCellPhone;
	CNxEdit	m_nxeditNexwebEmail;
	CNxEdit	m_nxeditNexwebNotes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebPatientInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	long m_nPersonID;
	void InitializeControls(BOOL bEnable = FALSE);
	void Load();
	void ProcessField(TransactionType transField, CString strEntry, long nObjectID);

	// Generated message map functions
	//{{AFX_MSG(CNexWebPatientInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChangeAddRefSour();
	afx_msg void OnRButtonUpNexwebReferralList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingNexwebReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedNexwebReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRemoveReferral();
	afx_msg void OnMakePrimaryReferral();
	afx_msg void OnEditingStartingNexwebReferralList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnSelChosenNexwebPtPrefixList(long nRow);
	afx_msg void OnSelChosenNexwebPatientTypeList(long nRow);
	afx_msg void OnSelChosenNexwebLocationList(long nRow);
	afx_msg void OnSelChosenNexwebMaritalStatus(long nRow);
	afx_msg void OnSelChosenNexwebGenderlist(long nRow);
	afx_msg void OnKillFocusNexwebDob();
	afx_msg void OnRequeryFinishedNexwebPatCoord(short nFlags);
	afx_msg void OnSelChosenNexwebPatCoord(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBPATIENTINFODLG_H__0CEEF9DB_3D4E_4B48_8C57_DF90E21081BB__INCLUDED_)
