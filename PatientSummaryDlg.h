#if !defined(AFX_PATIENTSUMMARYDLG_H__9BE04A1E_3152_4CBD_B777_C9AA92614FB9__INCLUDED_)
#define AFX_PATIENTSUMMARYDLG_H__9BE04A1E_3152_4CBD_B777_C9AA92614FB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientSummaryDlg.h : header file
//

// (j.jones 2008-07-08 09:14) - PLID 24624 - created

/////////////////////////////////////////////////////////////////////////////
// CPatientSummaryDlg dialog

#include "PatientsRc.h"

// (j.gruber 2010-06-10 10:28) - PLID 26363
class CPatSummaryListInfoObject
{
public:
	CString m_strLabel;
	COLORREF m_cColor;
	CString m_strTabLink;
	
	CPatSummaryListInfoObject(CString strLabel = "", COLORREF cColor = RGB(0,0,0), CString strTabLink = "")
	{
		m_strLabel = strLabel;
		m_cColor = cColor;
		m_strTabLink = strTabLink;		
	}
};
// (j.gruber 2010-06-10 10:28) - PLID 26363
class CPatSummaryConfigType
{
public:
	CString m_strConfigName;
	long m_nSortOrder;
	long m_nList;
	BOOL m_bCheck;

	CPatSummaryConfigType(BOOL bCheck = FALSE, CString strConfigName= "", long nSortOrder = -1, long nList = 1)
	{
		m_bCheck = bCheck;
		m_strConfigName = strConfigName;
		m_nList = nList;
		m_nSortOrder = nSortOrder;
	}
};
// (j.gruber 2010-06-10 10:28) - PLID 26363
class CPatSummaryList
{
public:
	long m_nSortOrder;	
	CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *m_paryObjects;

	CPatSummaryList(long nSortOrder = -1, CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryObjects = NULL) 
	{
		m_nSortOrder = nSortOrder;		

		if (paryObjects) {
			m_paryObjects = paryObjects;
			/*for (int i = 0; i < paryObjects->GetSize(); i++) {
				m_paryObjects.Add(paryObjects->GetAt(i));
			}*/
		}
	}
};

class CPatientSummaryDlg : public CNxDialog
{
// Construction
public:
	CPatientSummaryDlg(CWnd* pParent);   // standard constructor

	long m_nPatientID;
	CString m_strPatientName;

// Dialog Data
	//{{AFX_DATA(CPatientSummaryDlg)
	enum { IDD = IDD_PATIENT_SUMMARY_DLG };
	CNxColor		m_bkg;
	CNxStatic		m_nxstaticStatusLabel;
	CNxIconButton	m_btnHelp;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnConfigure; // (j.gruber 2010-06-15 14:18) - PLID 26363
	// (j.jones 2010-05-26 08:57) - PLID 38508 - added more controls
	// (j.gruber 2010-06-15 14:17) - PLID 26363 - taken out
	//CNxEdit			m_editLastApptInfo;
	//CNxEdit			m_editLastDiagCodes;
	//CNxEdit			m_editPatientBalance;
	//CNxEdit			m_editInsuranceBalance;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientSummaryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_SummaryList;

	// (j.gruber 2010-06-15 11:48) - PLID 26363 - split into 2 functions
	void LoadTopList();
	void LoadBottomList();

	// (j.jones 2010-05-26 09:51) - PLID 38508 - loads the info. at the top of the screen
	// (j.gruber 2010-06-15 14:11) - PLID 26363 - not used anymore
	//void FillDialogFields();

	void DisplayAppointments();
	void DisplayBills();
	void DisplayQuotes();
	void DisplayPayments();
	void DisplayEMNs();
	void DisplayPrescriptions();
	void DisplayFollowUps();
	void DisplayNotes();
	void DisplayHistory();
	void DisplayTracking();
	void DisplayLabs();

	// (j.gruber 2010-06-15 14:18) - PLID 26363
	void DisplayCustomField(ADODB::_RecordsetPtr rsAllRecs, long nCustomField, CString strBase);
	void DisplayPatientInformation(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayEmergencyContactInfo(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayGeneralField(ADODB::_RecordsetPtr rsAllRecs, CString strTitle, CString strTab, CString strBase);
	void DisplayGeneralDate(ADODB::_RecordsetPtr rsAllRecs, CString strTitle, CString strTab, CString strBase, BOOL bDisplayTime = FALSE);
	void DisplayEmploymentInfo(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayReferralInformation(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayWarningInformation(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayInsPartyInfo(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayCurrentMeds(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayAllergies(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayLastAppt(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayLastDiagCodesBilled(ADODB::_RecordsetPtr rsAllRecs);
	void DisplayGeneralCurrency(ADODB::_RecordsetPtr rsAllRecs, CString strTitle, CString strTab, CString strBase);
	void DisplayLanguageField(ADODB::_RecordsetPtr rsAllRecs);
	void LoadConfigValues();
	void DeleteArrays();

	// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
	CString GetTotalSql();
	CPatSummaryConfigType GetConfigInfo(CString strBase, BOOL bDefaultCheck = FALSE, long nDefaultSort = -1, long nDefaultList = 1);
	long GetCheckedValue(CString strBase);
	
	// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
	CArray<CPatSummaryList*, CPatSummaryList*> m_aryLeftList;
	CArray<CPatSummaryList*, CPatSummaryList*> m_aryRightList;
	CArray<CPatSummaryConfigType, CPatSummaryConfigType> m_pConfigList;
	CMap<long, long, CString, LPCTSTR> m_mapCustomFields;

	// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
	void LoadCustomFieldData();
	void MoveToTab(CString strTabLink);

	void AddListDataToArray(CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *aryObjects, CString strLabel = "", COLORREF cColor = RGB(0,0,0), CString strTabLink ="");

	void AddArraysToList();	
	void SortArray(CArray<CPatSummaryList*, CPatSummaryList*> *pList);

	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	//copied from PatientLabsDlg, used in DisplayLabs()
	//TES 11/10/2009 - PLID 36260 - This is no longer an enum, but a configurable table, so the description should be loaded along with the ID.
	//TES 12/8/2009 - PLID 36512 - AnatomySide is back!
	CString GetAnatomySideString(long nSide);

	//builds the text to be used for the help and do not show me again messages
	CString GetHelpText();

	// Generated message map functions
	//{{AFX_MSG(CPatientSummaryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSummaryHelp();
	afx_msg void OnBtnSummaryClose();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnSummaryConfigure();
	DECLARE_EVENTSINK_MAP()
	void LeftClickPatInfoList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTSUMMARYDLG_H__9BE04A1E_3152_4CBD_B777_C9AA92614FB9__INCLUDED_)
