#if !defined(AFX_NEXWEBIMPORTDLG_H__E1D1724C_0A84_4680_8F4C_8A81130A93E9__INCLUDED_)
#define AFX_NEXWEBIMPORTDLG_H__E1D1724C_0A84_4680_8F4C_8A81130A93E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebImportDlg.h : header file
//

#include "AuditTrail.h"
/////////////////////////////////////////////////////////////////////////////
// CNexWebImportDlg dialog

	struct AuditEventStruct {
		AuditEventItems aeiItem;
		CString strOldValue;
		CString strNewValue;
		long nPriority;
		long nType;
		CString strUserName;
	};

	struct APPT {
		long nPatientID;
		long nAptTypeID;
		CDWordArray *pdwResourceList;
		CDWordArray *pdwPurposeList;
		long nStatus;
		long nMoveUp;
		COleDateTime dtDate;
		COleDateTime dtStartTime;
		COleDateTime dtEndTime;
	};


	// (j.gruber 2008-06-03 10:15) - PLID 30235 - added NickName, also added Username for consistency, since it was in the dll but not here
	enum TransactionType {

		
		//Person Transaction Types
		transTypeFirstName = 1001,
		transTypeLastName = 1002,
		transTypeEmailAddress = 1003,
		transTypePassword = 1004,
		transTypeAddress1 = 1005,
		transTypeAddress2 = 1006,
		transTypeCity = 1007,
		transTypeState = 1008,
		transTypeZipCode = 109,
		transTypeNotes = 1010,
		transTypeHomePhone = 1011,
		transTypeWorkPhone = 1012,
		transTypeFax = 1013,
		transTypeSocialSecurity = 1014,
		transTypeBirthDate = 1015,
		transTypeGender = 1016,
		transTypeCompany= 1017,
		transTypeExtension = 1018,
		transTypeEmergencyContactFirst = 1019,
		transTypeEmergencyContactLast = 1020,
		transTypeEmergencyContactRelation = 1021,
		transTypeEmergencyContactHomePhone = 1022,
		transTypeEmergencyContactWorkPhone = 1023,
		transTypeMiddleName = 1024,
		transTypeCellPhone = 1025,
		transTypeLocation = 1026,
		transTypeInactive = 1027,
		transTypePrefix = 1028,
		transTypeUserName = 1029,
		transTypeNickName = 1030,


		transTypePatient = 2000,
		//Patient Transaction Types
		transTypeID = 2001,
		transTypePatientProcedures = 2002,
		transTypePatientCustomText1 = 2003,
		transTypePatientCustomText2 = 2004,
		transTypePatientCustomText3 = 2005,
		transTypePatientCustomText4 = 2006,
		transTypeCustomText1 = 2007,
		transTypeCustomText2 = 2008,
		transTypeCustomText3 = 2009,
		transTypeCustomText4 = 2010,
		transTypeCustomText5 = 2011,
		transTypeCustomText6 = 2012,
		transTypeCustomCheck1 = 2013,
		transTypeCustomCheck2 = 2014,
		transTypeCustomCheck3 = 2015,
		transTypeCustomCheck4 = 2016,
		transTypeCustomCheck5 = 2017,
		transTypeCustomCheck6 = 2018,
		transTypeCustomDate1 = 2019,
		transTypeCustomDate2 = 2020,
		transTypeCustomDate3 = 2021,
		transTypeCustomDate4 = 2022,
		transTypeMaritalStatus = 2023,
		transTypeSpouseName = 2024,
		transTypeComapny = 2025,
		transTypeOccupation = 2026,
		transTypeEmployerAddress1 = 2027,
		transTypeEmployerAddress2 = 2028,
		transTypeEmployerCity = 2029,
		transTypeEmployerState = 2030,
		transTypeEmployerZipCode = 2031,
		transTypePatientReferralSource= 2032,
		transTypeAddPrimaryReferralSource = 2033,
		transTypePatientReferralSources = 2034,
		transTypeCurrentIllnessDate = 2035,
		transTypePrimaryReferralSourceName = 2036,
		transTypeManagerFirst = 2037,
		transTypeManagerMiddle = 2038,
		transTypeManagerLast = 2039,
		transTypePatientType = 2040,
		transTypePatientReferralSourceDate = 2041,
		transTypePersonNoteAdded = 2042,
		transTypePersonNoteDate = 2043,
		transTypePersonNoteChanged = 2044,
		// (j.gruber 2006-11-27 13:01) - PLID 20758 - added Patient Coordinator to patient
		transTypePatientCoord = 2045,

		//Appointment Transaction Types
		transTypeAppointment = 3001,
		transTypeApptPatient = 3002,
		transTypeApptType = 3003,
		transTypeApptPurposeAdd = 3004,
		transTypeApptPurposeRemove = 3005,
		transTypeApptResourceAdd = 3006,
		transTypeApptResourceRemove = 3007,
		transTypeApptNote = 3008,
		transTypeApptStartTime = 3009,
		transTypeApptEndTime = 3010,
		transTypeApptConfirmed = 3011,
		transTypeApptDate = 3012,
		transTypeApptMoveUp = 3013,
		transTypeApptCancel = 3014,
		transTypeApptConfirmedDate = 3015,
		transTypeApptLocation = 3016,


		//Insured Party Transaction Types
		transTypeInsuredParty = 4001,
		transTypeInsPartyFirst = 4002,
		transTypeInsPartyLast = 4003,
		transTypeInsPartyAddress1 = 4004,
		transTypeInsParty = 4005,
		transTypeInsPartyCity = 4006,
		transTypeInsPartyState = 4007,
		transTypeInsPartyZip = 4008,
		transTypeInsPartyNotes = 4009,
		transTypeInsPartyHomePhone = 4010,
		transTypeInsPartySSN = 4011,
		transTypeInsPartyBirthDate = 4012,
		transTypeInsPartyInsuranceCompany = 4013,
		transTypeInsPartyInsContact = 4014,
		transTypeInsPartyPlanType = 4015,
		transTypeInsPartyRespType = 4016,
		transTypeInsPartyMiddle = 4017,
		transTypeInsPartyEmployer = 4018,
		transTypeInsPartyIDForInsurance = 4019,
		transTypeInsPartyGroupNumber = 4020,
		transTypeInsPartyRelationToPatient = 4021,
		transTypeInsPartyAddress2 = 4022,
		transTypeInsPartyGender = 4023,

		//(e.lally 2007-05-15) PLID 26014 - Add ability for ToDo alarms to be written to a transaction file.
		//ToDo task Transaction Types
		transTypeTodoTask		= 4101,
		transTypeTodoMethod		= 4102,
		transTypeTodoPriority	= 4103,
		transTypeTodoAssignTo	= 4104,
		transTypeTodoDeadline	= 4105,
		transTypeTodoRemindBy	= 4106,
		transTypeTodoNotes		= 4107,
			

	};


class CNexWebImportDlg : public CNxDialog
{
// Construction

public:
	CNexWebImportDlg(CWnd* pParent);   // standard constructor
	void RemoveTransactionFromList(long nPersonID);
	BOOL AppointmentInfoExists();
	BOOL PatientNotesExists();


// Dialog Data
	//{{AFX_DATA(CNexWebImportDlg)
	enum { IDD = IDD_NEXWEB_IMPORT_DLG };
	CNxIconButton	m_Exit;
	CNxIconButton	m_Save;
	CNxIconButton	m_ImportAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebImportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:


	void LoadTransactionRecords();
	void SaveField(TransactionType transField, CString strEntry, CString &strPersonSave, CString &strPatientSave, CString &strReferralSources, long nRow, BOOL bSkipOverwrites = FALSE);
	void AddToAuditArray(CString strValue, CString strField, CString strTable, long nPersonID, TransactionType transType);
	BOOL FieldChanged(CString strField, CString strTable, long nPersonID, CString strNewValue, CString &strOldValue);
	//(e.lally 2008-02-28) PLID 29142 - Function for opening the passed in row for editing/individual importing
	long EditImportListObject(long nRow);
	NXDATALISTLib::_DNxDataListPtr m_pImportList;
	CPtrArray m_paryAuditEvents;
	CString m_strError;
	BOOL m_bContinueMassImport;
	BOOL m_bSkipOverwrites;

	BOOL ToDoTaskExists();
	BOOL ValidateData();
	// Generated message map functions
	//{{AFX_MSG(CNexWebImportDlg)
	afx_msg void OnImport();
	afx_msg void OnCancel();	
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUpNexwebImportList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSave();
	afx_msg void OnDblClickCellNexwebImportList(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonUpNexwebImportList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBIMPORTDLG_H__E1D1724C_0A84_4680_8F4C_8A81130A93E9__INCLUDED_)
