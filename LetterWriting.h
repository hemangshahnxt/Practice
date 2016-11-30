
#include "letterview.h"
#include "wfxctl32.h"

#if !defined(AFX_LETTERWRITING_H__9BF5CC8F_F976_11D2_936C_00104BD3573F__INCLUDED_)
#define AFX_LETTERWRITING_H__9BF5CC8F_F976_11D2_936C_00104BD3573F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LetterWriting.h : header file
//

// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
#ifdef _DEBUG
#define DEBUGCODE(code)		code
#else
#define DEBUGCODE(code)	
#endif

class CGroups;

extern bool g_bMergeAllFields;
CString CreateTempIDTable(const CString &strSqlSource, const CStringArray &aryFieldNames, const CStringArray &aryFieldTypes, BOOL bIncludeRecordNumber, BOOL bSkipDups, OPTIONAL OUT long *pnRecordCount);
CString CreateTempIDTable(NXDATALISTLib::_DNxDataListPtr pDataList, short nIDColIndex, BOOL bIncludeRecordNumber = TRUE, BOOL bSkipDups = TRUE, OUT long *pnRecordCount = NULL, BOOL bAppointmentBasedMerge = FALSE);
CString CreateTempIDTable(const CString &strSqlSource, const CString &strIDFieldName, BOOL bIncludeRecordNumber = TRUE, BOOL bSkipDups = TRUE, OUT long *pnRecordCount = NULL);
// (a.walling 2010-08-27 17:18) - PLID 39965 - We require at least SQL 2000 now.
//BOOL DoesSqlSupportXml();


class CMergeEngine;

//m.hancock - 2/20/2006 - PLID 17239 - Add capability to create word templates from EMR
//Moved some declarations from LetterWriting.cpp

BOOL BrowseForTemplate(HWND hwndDlgOwner, OUT CString &strFullPathToTemplate);

class CLetterWriting_ExtraMergeFields
{
public:
	CString m_strEMRItemFilter;
};

CString CALLBACK CLetterWriting__ExtraMergeFields(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam);

/////////////////////////////////////////////////////////////////////////////
// CLetterWriting dialog

class CLetterWriting : public CNxDialog
{
	DECLARE_DYNAMIC(CLetterWriting);

// Construction
public:
	CLetterWriting(CWnd* pParent);   // standard constructor
	// (j.jones 2016-04-15 09:09) - NX-100214 - added destructor
	~CLetterWriting();
	long CreateMergeFlags(BOOL bShowAll = FALSE);
	long CheckAllowClose();
	void PreClose();
	CGroups &m_groupEditor;

	NXDATALISTLib::_DNxDataListPtr m_SubjectCombo;
	NXDATALISTLib::_DNxDataListPtr m_SortOrderCombo;

	CTableChecker m_coordChecker;

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (e.lally 2009-06-19) PLID 29908 - Checkbox to protect PHI fields

// Dialog Data
	//{{AFX_DATA(CLetterWriting)
	enum { IDD = IDD_LETTERS };
	NxButton	m_btnIncludeBillInfo;
	NxButton	m_btnIncludeCustomInfo;
	NxButton	m_btnIncludeDateInfo;
	NxButton	m_btnIncludeDoctorInfo;
	NxButton	m_btnIncludeEMRInfo;
	NxButton	m_btnIncludeInsuranceInfo;
	NxButton	m_btnIncludePersonInfo;
	NxButton	m_btnIncludeRespInfo;
	NxButton	m_btnIncludeProcedureInfo;
	NxButton	m_btnIncludePrescriptionInfo;
	NxButton	m_btnIncludePracticeInfo;
	NxButton	m_btnDoNotIncludePrivateEmail;
	NxButton	m_btnExportCSV;
	NxButton	m_btnSortForMassMail;
	NxButton	m_btnHTMLFormat;
	NxButton	m_btnSendAsEmail;
	NxButton	m_btnDoNotAttachToHistory;
	NxButton	m_btnExcludeInactivePatients;
	NxButton	m_btnDoNotExcludePatientsFromMailings;
	NxButton	m_btnMergeDirectToPrinter;
	// (b.savon 2014-09-02 11:34) - PLID 62791 - Add Patient Reminder
	NxButton	m_btnAddPatientReminder;
	CNxIconButton	m_editTemplateButton;
	CNxIconButton	m_newTemplateButton;
	CNxIconButton	m_envelopeButton;
	CNxIconButton	m_formButton;
	CNxIconButton	m_labelButton;
	CNxIconButton	m_otherButton;
	CNxIconButton	m_letterButton;
	CNxIconButton	m_packetButton;
	CNxIconButton	m_EMRButton;
	CNxEdit	m_nxeditMerge;
	CNxStatic	m_nxstaticGroupLabel2;
	CNxStatic	m_nxstaticGroupLabel3;
	NxButton	m_btnGroupeditor;
	NxButton	m_btnOptionsGroupbox;
	NxButton	m_btnIncludeFieldsGroupbox;
	NxButton	m_btnHidePHI;
	CNxIconButton	m_btnPHIHelp;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_dlEmployees;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLetterWriting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OpenDocument(LPCTSTR strSubFolder = NULL);
	void MergeDocument(const CString &strPath, bool bDirectToPrinter = false);
	void MergeDocumentBatch(const CStringArray &arystrPaths, bool bDirectToPrinter = false, long nPacketID = -1, bool bSeparatePacket = false);
	CString GetNexID();	//for faxing
	void AddUsersToWinFaxFolder(CString strFolderID);	//for faxing
	ISDKPhoneBook m_pBookObj;
	bool m_bWFXActive;
	
protected:
	BOOL PrepareMerge(OUT CMergeEngine &mi, OUT CString &strMergeTo, OUT BOOL &bAllowSave);

public:
	// (m.hancock 2006-12-04 10:52) - PLID 21965 - Changes the sort option datalist to show options applicable 
	// to the specific merge type.
	void DisplayDefaultSortOptions();
	void DisplayAppointmentBasedSortOptions();

protected:
	// Generated message map functions
	//{{AFX_MSG(CLetterWriting)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWriteForm();
	afx_msg void OnWriteLabel();
	afx_msg void OnWriteLetter();
	afx_msg void OnWriteOther();
	afx_msg void OnWriteEnvelope();
	afx_msg void OnNoinactiveCheck();
	afx_msg void OnNewTemplateBtn();
	afx_msg void OnEditTemplateBtn();
	afx_msg void OnCustomInfoCheck();
	afx_msg void OnDateInfoCheck();
	afx_msg void OnInsuranceInfoCheck();
	afx_msg void OnPersonInfoCheck();
	afx_msg void OnPracticeInfoCheck();
	afx_msg void OnPrescriptionInfoCheck();
	afx_msg void OnEmailCheck();
	afx_msg void OnExportOnlyCheck();
	afx_msg void OnEMRCheck();
	afx_msg void OnWritePacket();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNextechFax();
	afx_msg void OnBillInfoCheck();
	afx_msg void OnProcedureInfoCheck();
	afx_msg void OnDoctorInfo();
	afx_msg void OnRespInfoCheck();
	afx_msg void OnSelChosenSubjectCombo(long nRow);
	afx_msg void OnEditSubject();
	afx_msg void OnMassMail();
	afx_msg void OnHidePHICheck();
	afx_msg void OnPHIHelpBtn();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAddReminderForPtMerged();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LETTERWRITING_H__9BF5CC8F_F976_11D2_936C_00104BD3573F__INCLUDED_)
