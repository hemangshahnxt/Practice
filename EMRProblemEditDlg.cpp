// EMRProblemEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRProblemEditDlg.h"
#include "EMRProblemStatusDlg.h"
#include "PatientsRc.h"
#include "AuditTrail.h"
#include "globalutils.h"
#include "emrutils.h"
#include "internationalutils.h"
#include "EditComboBox.h"
#include "EMNDetail.h"
#include "WellnessDataUtils.h"
#include "DecisionRuleUtils.h"
#include "EmrColors.h"
#include "UTSSearchDlg.h"
#include "MedlinePlusUtils.h"
#include "DiagSearchUtils.h"
#include "EmrRc.h"

using namespace ADODB;
using namespace NXTIMELib;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2009-05-21 17:52) - PLID 34250 - added enum for linked object columns
enum LinkedObjectListColumns
{
	lolcEMRProblemLinkObject = 0,
	lolcEMRRegardingType,
	lolcEMRRegardingID,
	lolcEMRDataID,
	lolcLinkedItemType,
	lolcDetailName,
	lolcDetailValue,
};

// (b.spivey September 24, 2013) - PLID 58677 - enum for new datalist
enum SNOWMEDCodeListColumns 
{
	sclcID = 0,
	sclcCode,
	sclcName, 
	sclcDescription, 
};

// (j.jones 2014-02-24 12:26) - PLID 60781 - added a diag code list
enum DiagCodeListColumns {
	dclcDiagICD9CodeID = 0,
	dclcDiagICD9Code,
	dclcDiagICD9Desc,
	dclcDiagICD10CodeID,
	dclcDiagICD10Code,
	dclcDiagICD10Desc,
};

// (j.jones 2014-03-06 15:41) - PLID 60781 - enumerated the history columns
enum HistoryListColumns {
	hlcID = 0,
	hlcDate,
	hlcDescription,
	hlcUser,
	hlcStatus,
	hlcDiagCodeICD9,
	hlcDiagCodeICD10,
	hlcChronicity,
};

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemEditDlg dialog


CEMRProblemEditDlg::CEMRProblemEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRProblemEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRProblemEditDlg)
	m_strCtrlDesc = _T("");
	//}}AFX_DATA_INIT
	m_nProblemID = -1;
	m_nStatusID = -1;
	m_nPatientID = -25;
	m_bWriteToData = FALSE;
	m_bProblemWasDeleted = FALSE;
	m_dtOnsetDate.SetStatus(COleDateTime::invalid); // (c.haag 2008-07-25 12:35) - PLID 30727
	// (a.walling 2008-06-12 10:22) - PLID 23138
	m_bReadOnly = FALSE;
	// (j.jones 2009-05-21 16:53) - PLID 34250 - removed these fields
	//m_nOwnerID = -1;
	//m_nEMRDataID = -1;
	
	// (a.walling 2009-05-01 17:56) - PLID 33751
	m_nChronicityID = -1;
	// (b.spivey September 24, 2013) - PLID 58677 - 
	m_nCodeID = -1;
	// (j.jones 2014-02-24 12:50) - PLID 60781
	m_nDiagICD9CodeID = -1;
	m_nDiagICD10CodeID = -1;
	// (s.tullis 2015-03-11 10:14) - PLID 64723 
	m_bDoNotShowOnCCDA = FALSE;
	// (r.gonet 2015-03-09 19:24) - PLID 64723 - Init DoNotShowOnProblemPrompt.
	m_bDoNotShowOnProblemPrompt = FALSE;
}

// (j.jones 2009-05-22 10:05) - PLID 34250 - for memory cleanup
CEMRProblemEditDlg::~CEMRProblemEditDlg()
{
	for(int i=m_arypLinkedObjects.GetSize()-1; i>=0; i--) {
		LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
		delete pInfo;
	}
	m_arypLinkedObjects.RemoveAll();
}

void CEMRProblemEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRProblemEditDlg)
	DDX_Control(pDX, IDC_NXC_PROBLEM_EDIT, m_nxcTop);
	DDX_Text(pDX, IDC_EDIT_PROBLEM_DESCRIPTION, m_strCtrlDesc);
	DDV_MaxChars(pDX, m_strCtrlDesc, 255);
	DDX_Control(pDX, IDC_EDIT_PROBLEM_DESCRIPTION, m_nxeditEditProblemDescription);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_btnSave);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PT_EDUCATION_LABEL, m_nxlabelPatientEducation);
	DDX_Control(pDX, IDC_BTN_PT_EDUCATION, m_btnPatientEducation);
	DDX_Control(pDX, IDC_CHECK_NOSHOWONCCDA, m_checkDoNotShowOnCCDA); // (s.tullis 2015-02-23 15:20) - PLID 64723 - Checkbox
	DDX_Control(pDX, IDC_CHECK_DO_NOT_SHOW_ON_PROBLEM_PROMPT, m_checkDoNotShowOnProblemPrompt); // (r.gonet 2015-03-09 18:21) - PLID 65008
	//}}AFX_DATA_MAP
}

BOOL CEMRProblemEditDlg::IsDateTimeValid(const COleDateTime& dt) const
{
	return (dt.GetStatus() == COleDateTime::valid && dt.m_dt > 0) ? TRUE : FALSE;
}

// (c.haag 2008-07-23 10:13) - PLID 30727 - Assigns the type and description of the source
// object that owns this problem
// (j.jones 2008-07-25 10:18) - PLID 30841 - nOwnerID and nEMRDataID are required for saving new problems
// (j.jones 2009-05-21 16:42) - PLID 34250 - removed this function in lieu of two new ones
/*
void CEMRProblemEditDlg::SetSourceInfo(EMRProblemRegardingTypes type, const CString& strDescription, long nOwnerID, long nEMRDataID /*= -1*//*)
{
	m_OwnerType = type;
	m_strOwnerDescription = strDescription;
	m_nOwnerID = nOwnerID;
	m_nEMRDataID = nEMRDataID;
}
*/

// (j.jones 2009-05-21 16:42) - PLID 34250 - AddLinkedObjectInfo adds direct info. regarding a problem link,
// can be called multiple times
// pEmrProblemLink is NULL if a memory object is not available
void CEMRProblemEditDlg::AddLinkedObjectInfo(long nEMRProblemLinkID, EMRProblemRegardingTypes eType, const CString& strName, const CString& strValue, long nRegardingID, CEmrProblemLink *pEmrProblemLink /*= NULL*/, long nEMRDataID /*= -1*/)
{
	LinkedObjectInfo *pInfo = new LinkedObjectInfo;
	pInfo->nEMRProblemLinkID = nEMRProblemLinkID;	
	pInfo->eType = eType;
	pInfo->strName = strName;
	pInfo->strValue = strValue;
	pInfo->nRegardingID = nRegardingID;
	pInfo->pEmrProblemLink = pEmrProblemLink;
	pInfo->nEMRDataID = nEMRDataID;
	pInfo->bDeleted = FALSE;	//deleted objects are never initially provided to us
	m_arypLinkedObjects.Add(pInfo);
}

// (j.jones 2009-05-21 16:42) - PLID 34250 - GenerateLinkedObjectInfo will crunch data from a passed-in array of problem links,
// and call AddLinkedObjectInfo for each problem link
// Note: not currently used, it just so happens that all places that call this dialog can more efficiently use AddLinkedObjectInfo
/*
void CEMRProblemEditDlg::GenerateLinkedObjectInfo(CArray<CEmrProblemLink*, CEmrProblemLink*> &apEmrProblemLinks)
{
	for(int i = 0; i < apEmrProblemLinks.GetSize(); i++) {		

		//add to our linked object info

		CEmrProblemLink *pProblemLink = apEmrProblemLinks.GetAt(i);
		EMRProblemRegardingTypes eType = pProblemLink->GetType();
		
		CString strDetailName = "";
		CString strDetailValue = "";
		
		pProblemLink->GetDetailNameAndValue(strDetailName, strDetailValue);

		AddLinkedObjectInfo(pProblemLink->GetID(), eType, strDetailName, strDetailValue, pProblemLink->GetRegardingID(), pProblemLink, pProblemLink->GetDataID());
	}
}
*/

void CEMRProblemEditDlg::SetProblemID(long nProblemID)
{
	m_nProblemID = nProblemID;
}

// (j.jones 2008-07-21 09:48) - PLID 30779 - added GetProblemID
long CEMRProblemEditDlg::GetProblemID()
{
	return m_nProblemID;
}

void CEMRProblemEditDlg::SetProblemDesc(const CString& strDescription)
{
	m_strProblemDesc = strDescription;
}

CString CEMRProblemEditDlg::GetProblemDesc()
{
	return m_strProblemDesc;
}

void CEMRProblemEditDlg::SetProblemStatusID(long nStatusID)
{
	m_nStatusID = nStatusID;
}

long CEMRProblemEditDlg::GetProblemStatusID()
{
	return m_nStatusID;
}

void CEMRProblemEditDlg::SetWriteToData(BOOL bWriteToData)
{
	m_bWriteToData = bWriteToData;
}

void CEMRProblemEditDlg::SetPatientID(long nPatientID)
{
	// (c.haag 2006-11-13 13:53) - PLID 22052 - We now track the patient ID for
	// proper auditing when saving
	m_nPatientID = nPatientID;
}

// (c.haag 2008-07-23 09:45) - PLID 30727 - Get the onset date
COleDateTime CEMRProblemEditDlg::GetOnsetDate()
{
	return m_dtOnsetDate;
}

// (c.haag 2008-07-23 10:44) - PLID 30727 - Set the onset date
void CEMRProblemEditDlg::SetOnsetDate(const COleDateTime& dtOnsetDate)
{
	m_dtOnsetDate = dtOnsetDate;
}

// (r.gonet 2015-03-06 10:05) - PLID 65008 - Sets the EMR Problem's Do Not Show On Problem Prompt flag.
void CEMRProblemEditDlg::SetProblemDoNotShowOnProblemPrompt(BOOL bDoNotShowOnProblemPrompt)
{
	m_bDoNotShowOnProblemPrompt = bDoNotShowOnProblemPrompt;
}

// (r.gonet 2015-03-06 10:05) - PLID 65008 - Gets the EMR Problem's Do Not Show On Problem Prompt flag.
BOOL CEMRProblemEditDlg::GetProblemDoNotShowOnProblemPrompt() const
{
	return m_bDoNotShowOnProblemPrompt;
}

BOOL CEMRProblemEditDlg::ProblemWasDeleted() const
{
	return m_bProblemWasDeleted;
}

BOOL CEMRProblemEditDlg::IsModified() const
{
	return m_bModified;
}

BEGIN_MESSAGE_MAP(CEMRProblemEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRProblemEditDlg)
	ON_BN_CLICKED(IDC_BTN_EDIT_STATUS_LIST, OnBtnEditStatusList)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_BTN_EDIT_SNOMED_LIST, OnBnClickedOpenUmls)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_EDIT_CHRONICITY_LIST, OnBnClickedBtnEditChronicityList)
	ON_BN_CLICKED(IDC_BTN_PT_EDUCATION, OnBtnPtEducation)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_CHECK_NOSHOWONCCDA, &CEMRProblemEditDlg::OnBnClickedCheckNoshowonccda)
	ON_BN_CLICKED(IDC_CHECK_DO_NOT_SHOW_ON_PROBLEM_PROMPT, &CEMRProblemEditDlg::OnBnClickedCheckDoNotShowOnProblemPrompt)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemEditDlg message handlers

BOOL CEMRProblemEditDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (r.gonet 01/27/2014) - PLID 59339 - Cache the properties in CEMRProblemEditDlg::OnInitDialog() 
		g_propManager.CachePropertiesInBulk("CEMRProblemEditDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"	'ShowPatientEducationLinks', " // (r.gonet 01/27/2014) - PLID 59339
			"	'ShowPatientEducationButtons' " // (r.gonet 01/27/2014) - PLID 59339
			")"
			, _Q(GetCurrentUserName()));

		// (c.haag 2008-04-28 12:17) - PLID 29806 - NxIconify the buttons
		m_btnSave.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (r.gonet 2014-01-27 15:29) - PLID 59339 - Init the preferences to show or hide the patient education related controls.
		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);

		// (j.jones 2013-11-05 14:49) - PLID 58982 - added patient education
		m_btnPatientEducation.SetIcon(IDI_INFO_ICON);
		m_nxlabelPatientEducation.SetText("Pat. Education");

		if(!bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Hide the patient education infobutton.
			m_btnPatientEducation.ShowWindow(SW_HIDE);
		}

		if(bShowPatientEducationLink) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Show the patient education link 
			m_nxlabelPatientEducation.SetType(dtsHyperlink);
		} else if(!bShowPatientEducationLink && bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We just need the label as a description of the info button.
			m_nxlabelPatientEducation.SetType(dtsText);
		} else if(!bShowPatientEducationLink && !bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We don't need the label as either a link or a description label, so hide it.
			m_nxlabelPatientEducation.ShowWindow(SW_HIDE);
		}

		m_dlStatus = BindNxDataList2Ctrl(this, IDC_LIST_PROBLEM_STATUS, GetRemoteData(), true);
		m_dlHistory = BindNxDataList2Ctrl(this, IDC_LIST_EMR_PROBLEM_HISTORY, GetRemoteData(), false);
		UpdateHistoryListDiagnosisColumns();
		
		// (j.jones 2014-02-24 11:14) - PLID 60781 - diagnosis codes now have
		// a search list, and then a list of the selected codes, maximum of one row
		m_DiagSearchList = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_PROBLEM_DIAG_SEARCH_LIST, GetRemoteData());
		m_DiagCodeList = BindNxDataList2Ctrl(this, IDC_PROBLEM_DIAG_CODE_LIST, GetRemoteData(), false);

		// (a.walling 2009-05-01 17:56) - PLID 33751
		m_dlChronicity = BindNxDataList2Ctrl(this, IDC_LIST_PROBLEM_CHRONICITY, GetRemoteData(), true);

		// (b.spivey September 24, 2013) - PLID 58677 - Bind this list. 
		m_dlSNOMEDList = BindNxDataList2Ctrl(this, IDC_LIST_PROBLEM_SNOMED, GetRemoteData(), true); 

		// (j.jones 2009-05-21 17:53) - PLID 34250 - added list of linked objects
		m_dlLinkedObjectsList = BindNxDataList2Ctrl(this, IDC_EMR_PROBLEM_LINKED_OBJECTS_LIST, GetRemoteData(), false);

		//
		// Assign the dialog color
		//
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_nxcTop.SetColor(EmrColors::Topic::PatientBackground());
		//
		// Populate the history list
		//
		if (m_nProblemID > 0) {
			CString strWhere;
			strWhere.Format("ProblemID = %d", m_nProblemID);
			m_dlHistory->WhereClause = (LPCTSTR)strWhere;
			m_dlHistory->Requery();
			m_bModified = FALSE;
		} else {
			m_bModified = TRUE;
		}
		//
		// (c.haag 2008-07-23 09:42) - PLID 30727 - Populate the onset date
		//
		m_nxtOnsetDate = GetDlgItemUnknown(IDC_DATE_ONSET);
		if(IsDateTimeValid(m_dtOnsetDate)) {
			m_nxtOnsetDate->SetDateTime(m_dtOnsetDate);
		}
		else {
			m_nxtOnsetDate->Clear();
		}
		//
		// Populate the default problem description
		//
		SetDlgItemText(IDC_EDIT_PROBLEM_DESCRIPTION, m_strProblemDesc);
		//
		// Set the dialog title
		//
		// (c.haag 2008-07-23 10:18) - PLID 30727 - Format to support different types
		// of owners (not just details)
		// (c.haag 2008-12-05 09:24) - PLID 28496 - Added eprtUnassigned for problems not tied to any
		// specific kinds of patient records
		// (j.jones 2009-05-21 17:20) - PLID 34250 - problems can now be linked to multiple objects,
		// they are now stored in a list
		for(int i = 0; i < m_arypLinkedObjects.GetSize(); i++) {
			
			LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
			
			CString strOwnerType;
			switch (pInfo->eType) {
				case eprtEmrItem:
					strOwnerType = "EMR Item";
					break;
				case eprtEmrDataItem:
					strOwnerType = "EMR List Option";
					break;
				case eprtEmrTopic:
					strOwnerType = "EMR Topic";
					break;
				case eprtEmrEMN:
					strOwnerType = "EMN";
					break;
				case eprtEmrEMR:
					strOwnerType = "EMR";			
					break;
				case eprtEmrDiag:
					// (r.gonet 03/07/2014) - PLID 61191 - Reworded to remove reference to ICD-9
					strOwnerType = "Diagnosis Code";
					break;
				case eprtEmrCharge:
					strOwnerType = "Service Code";
					break;
				case eprtEmrMedication:
					strOwnerType = "Medication";					
					break;
				case eprtUnassigned:
					strOwnerType = "Patient";
					break;
				case eprtLab:
					// (z.manning 2009-05-26 14:50) - PLID 34345
					strOwnerType = "Lab";
					break;
				default:
					ASSERT(FALSE); // This type is not supported
					break;
			}

			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlLinkedObjectsList->GetNewRow();
			pNewRow->PutValue(lolcEMRProblemLinkObject, (long)pInfo);
			pNewRow->PutValue(lolcEMRRegardingType, (long)pInfo->eType);
			pNewRow->PutValue(lolcEMRRegardingID, pInfo->nRegardingID);
			pNewRow->PutValue(lolcEMRDataID, pInfo->nEMRDataID);
			pNewRow->PutValue(lolcLinkedItemType, _bstr_t(strOwnerType));
			pNewRow->PutValue(lolcDetailName, _bstr_t(pInfo->strName));
			pNewRow->PutValue(lolcDetailValue, _bstr_t(pInfo->strValue));
			m_dlLinkedObjectsList->AddRowSorted(pNewRow, NULL);
			if(pInfo->eType == eprtLab && pInfo->pEmrProblemLink != NULL) {
				// (z.manning 2009-07-17 19:40) - PLID 34345
				pInfo->pEmrProblemLink->SetLabText(pInfo->strName);
			}
		}

		{
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlChronicity->GetNewRow();
			pNewRow->PutValue(0, (long)-1);
			pNewRow->PutValue(1, " <None>");
			m_dlChronicity->AddRowSorted(pNewRow, NULL);
		}

		{
			// (b.spivey September 24, 2013) - PLID 58677 - Add the "no-row" row. 
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlSNOMEDList->GetNewRow();
			pNewRow->PutValue(sclcID, (long)-1);
			pNewRow->PutValue(sclcCode, " <None>");
			pNewRow->PutValue(sclcName, _bstr_t(""));
			pNewRow->PutValue(sclcDescription, _bstr_t("")); 
			m_dlSNOMEDList->AddRowSorted(pNewRow, NULL);
		}

		m_dlSNOMEDList->SetSelByColumn(sclcID, m_nCodeID); 
		m_dlChronicity->TrySetSelByColumn_Deprecated(0, m_nChronicityID);

		// (j.jones 2014-02-24 11:20) - PLID 60781 - add the diag codes, if any,
		// to the datalist
		ReflectCurrentDiagnosisCodes();		

		// (s.tullis 2015-02-23 15:44) - PLID 64723
			m_checkDoNotShowOnCCDA.SetCheck(m_bDoNotShowOnCCDA);


		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Initialize the DoNotShowOnProblemPrompt checkbox with the loaded value from the saved problem, if there is one.
		m_checkDoNotShowOnProblemPrompt.SetCheck(m_bDoNotShowOnProblemPrompt);

		//
		// (c.haag 2006-11-01 17:02) - PLID 21453 - Disable controls if we do not
		// have any kind of write access
		//
		// (a.walling 2007-11-28 10:33) - PLID 28044 - Also disable if HasEMR is 0
		// (a.walling 2008-06-12 10:24) - PLID 23138 - Disable if readonly
		if (m_bReadOnly || !(GetCurrentUserPermissions(bioEMRProblems) & SPT___W________ANDPASS)
			|| (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
			GetDlgItem(IDC_BTN_EDIT_STATUS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_PROBLEM_DESCRIPTION)->EnableWindow(FALSE);
			GetDlgItem(IDC_LIST_PROBLEM_STATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_LIST_EMR_PROBLEM_HISTORY)->EnableWindow(FALSE);
			GetDlgItem(IDC_DATE_ONSET)->EnableWindow(FALSE);
			// (j.jones 2009-05-29 17:18) - PLID 34250 - disable the problem links
			GetDlgItem(IDC_EMR_PROBLEM_LINKED_OBJECTS_LIST)->EnableWindow(FALSE);
			// (j.jones 2014-02-24 12:50) - PLID 60781 - new diag controls
			GetDlgItem(IDC_PROBLEM_DIAG_SEARCH_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_PROBLEM_DIAG_CODE_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_LIST_PROBLEM_CHRONICITY)->EnableWindow(FALSE);
			//(b.spivey - November 18th, 2013) - PLID 58677 - read only like the rest of the controls. 
			GetDlgItem(IDC_LIST_PROBLEM_SNOMED)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_EDIT_SNOMED_LIST)->EnableWindow(FALSE); 
			GetDlgItem(IDC_BTN_EDIT_CHRONICITY_LIST)->EnableWindow(FALSE); 
			// (s.tullis 2015-02-23 15:23) - PLID 64723 - Read Only
			GetDlgItem(IDC_CHECK_NOSHOWONCCDA)->EnableWindow(FALSE);
			// (r.gonet 2015-03-09 18:21) - PLID 65008 - Disable the DoNotShowOnProblemPrompt checkbox if the user doesn't have permission to edit the problem.
			GetDlgItem(IDC_CHECK_DO_NOT_SHOW_ON_PROBLEM_PROMPT)->EnableWindow(FALSE);
		}
		else {
			// (c.haag 2008-07-23 09:41) - PLID 30727 - Set the default focus to the problem
			// description
			GetDlgItem(IDC_EDIT_PROBLEM_DESCRIPTION)->SetFocus();
			return FALSE;
		}

	}
	NxCatchAll("Error initializing the EMR problem edit dialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRProblemEditDlg::OnOK()
{
	//do nothing here
}

BEGIN_EVENTSINK_MAP(CEMRProblemEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRProblemEditDlg)
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_STATUS, 18 /* RequeryFinished */, OnRequeryFinishedListProblemStatus, VTS_I2)
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_STATUS, 2 /* SelChanged */, OnSelChangedListProblemStatus, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEMRProblemEditDlg, IDC_DATE_ONSET, 1 /* KillFocus */, OnKillFocusDateOnset, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_CHRONICITY, 2, CEMRProblemEditDlg::SelChangedListProblemChronicity, VTS_DISPATCH VTS_DISPATCH)	
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_CHRONICITY, 18, CEMRProblemEditDlg::RequeryFinishedListProblemChronicity, VTS_I2)
	ON_EVENT(CEMRProblemEditDlg, IDC_EMR_PROBLEM_LINKED_OBJECTS_LIST, 6, CEMRProblemEditDlg::OnRButtonDownEmrProblemLinkedObjectsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_SNOMED, 1, CEMRProblemEditDlg::SelChangingListProblemSNOMED, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_SNOMED, 2, CEMRProblemEditDlg::SelChangedListProblemSNOMED, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_PROBLEM_SNOMED, 18, CEMRProblemEditDlg::RequeryFinishedListProblemSNOMEDCode, VTS_I2)
	ON_EVENT(CEMRProblemEditDlg, IDC_PROBLEM_DIAG_SEARCH_LIST, 16, CEMRProblemEditDlg::OnSelChosenProblemDiagSearchList, VTS_DISPATCH)
	ON_EVENT(CEMRProblemEditDlg, IDC_PROBLEM_DIAG_CODE_LIST, 6, CEMRProblemEditDlg::OnRButtonDownProblemDiagCodeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRProblemEditDlg, IDC_LIST_EMR_PROBLEM_HISTORY, 18, CEMRProblemEditDlg::OnRequeryFinishedListEmrProblemHistory, VTS_I2)
END_EVENTSINK_MAP()

void CEMRProblemEditDlg::OnRequeryFinishedListProblemStatus(short nFlags) 
{	
	try {
		if (-1 == m_nStatusID) {
			// (z.manning 2009-06-03 11:05) - PLID 34460 - "Open" was renamed to "Active", but the safer
			// thing to do here is to look this up by ID.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->FindByColumn(0, (long)1, NULL, VARIANT_TRUE);
			if(pRow != NULL) {
				m_nStatusID = VarLong(pRow->GetValue(0));
			}
		}
		else {
			if (NULL == m_dlStatus->FindByColumn(0, m_nStatusID, NULL, VARIANT_TRUE)) {
				//
				// (c.haag 2006-07-10 09:16) - If we get here, it means that this problem has a status
				// ID that does not exist in the status dropdown. This can happen if the status in inactive.
				// We will have to pull it from data, add it manually, and then select it.
				//
				ADODB::_RecordsetPtr prs = CreateRecordset("SELECT Name FROM EMRProblemStatusT WHERE ID = %d",
					m_nStatusID);
				if (!prs->eof) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->GetNewRow();
					pRow->Value[0] = m_nStatusID;
					pRow->Value[1] = (LPCTSTR)AdoFldString(prs, "Name");
					m_dlStatus->CurSel = m_dlStatus->AddRowAtEnd(pRow, NULL);
				}

			}
		}
	}
	NxCatchAll("Error in OnRequeryFinishedListProblemStatus");
}

void CEMRProblemEditDlg::OnSelChangedListProblemStatus(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlStatus->CurSel;
		// (a.walling 2006-10-05 14:24) - PLID 22839 - Fix the no selection exception
		if (pRow) {
			if (m_nStatusID != VarLong(pRow->GetValue(0))) {
				m_nStatusID = VarLong(pRow->GetValue(0));
				m_bModified = TRUE;
			}
		}
		else {
			pRow = lpOldSel;
			if (pRow) {
				m_dlStatus->PutCurSel(pRow);
			}
			else {
				ASSERT(FALSE);
			}
		}
	}
	NxCatchAll("Error in OnSelChangedListProblemStatus");
}

void CEMRProblemEditDlg::OnBtnEditStatusList() 
{
	CEMRProblemStatusDlg dlg(this);
	dlg.SetParentStatusID(m_nStatusID);
	if (IDOK == dlg.DoModal()) {
		m_dlStatus->Requery();
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_dlStatus->TrySetSelByColumn_Deprecated(0, m_nStatusID);
	}
}

void CEMRProblemEditDlg::OnButtonSave() 
{
	CString strText;
	GetDlgItemText(IDC_EDIT_PROBLEM_DESCRIPTION, strText);
	if (m_strProblemDesc != strText) {
		m_strProblemDesc = strText;
		m_bModified = TRUE;
	}


	if(m_nxtOnsetDate->GetStatus() == 1) { // Form control is Valid
		if (!IsDateTimeValid(m_dtOnsetDate) || m_dtOnsetDate != COleDateTime(m_nxtOnsetDate->GetDateTime()))
		{
			m_dtOnsetDate = m_nxtOnsetDate->GetDateTime();
			m_bModified = TRUE;
		}
	}
	else { // Form control is Invalid / Empty
		if (IsDateTimeValid(m_dtOnsetDate)) {
			m_dtOnsetDate.SetStatus(COleDateTime::invalid);
			m_bModified = TRUE;
		}
	}


	// (c.haag 2008-07-24 17:53) - PLID 30727 - Whether or not the problem was modified, warn if the
	// onset date is in the future
	if (!m_bReadOnly && IsDateTimeValid(m_dtOnsetDate)) {
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
		if (m_dtOnsetDate > dtToday) {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The onset date is in the future. Are you sure you wish to continue?")) {
				return;
			}
		}
	}

	//
	// (c.haag 2006-11-01 17:10) - PLID 21453 - If the problem was modified, check if the user
	// has permission to save their changes, starting with creating the item if necessary
	//
	if (m_bModified) {
		BOOL bKnowsPassword = FALSE; // Make sure the user only has to enter their password once
		//
		// Begin by checking for create permissions. The reason for going through all this
		// hassle is so that we have complete control over the prompts and message boxes. It
		// would seem awkward to click OK and suddenly see "Enter Password" and then "You
		// do not have permission to access this feature."
		//
		// (c.haag 2006-12-13 09:30) - PLID 21453 - After some testing, I found that this code 
		// is actually unnecessary because the user could not have created this dialog with a
		// -1 problem ID unless they had create permissions
		/*if (-1 == m_nProblemID) {
			if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate, FALSE, 0, TRUE, TRUE)) {
				// The user does not have write permissions outright
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "You do not have permission to save this problem. Please contact your office manager for assistance.");
				return;
			} else if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate, FALSE, 0, TRUE, FALSE)) {
				// If the first statement is true, and this is false, then the user must enter their password
				if (!bKnowsPassword && !CheckCurrentUserPassword("Confirm Problem Changes")) {
					return;
				} else {
					bKnowsPassword = TRUE;
				}
			}
		}*/

		//
		// Now confirm write permissions
		//
		// (a.walling 2008-06-12 10:25) - PLID 23138 - if readonly, just exit
		if (m_bReadOnly) {
			CNxDialog::OnOK();
			return;
		} if (!CheckCurrentUserPermissions(bioEMRProblems, sptWrite, FALSE, 0, TRUE, TRUE)) {
			// The user does not have write permissions outright
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "You do not have permission to save your changes. Please contact your office manager for assistance.");
			return;
		} else if (!CheckCurrentUserPermissions(bioEMRProblems, sptWrite, FALSE, 0, TRUE, FALSE)) {
			// If the first statement is true, and this is false, then the user must enter their password
			if (!bKnowsPassword && !CheckCurrentUserPassword("Confirm Problem Changes")) {
				return;
			} else {
				bKnowsPassword = TRUE;
			}
		}

		// (c.haag 2006-11-13 13:39) - PLID 22052 - If we are responsible for saving changes
		// to data, do so now
		if (m_bWriteToData) {
			if (!WriteToData())
				return;
		}
	} // if (m_bModified) {

	CNxDialog::OnOK();
}

// (a.walling 2008-07-28 15:21) - PLID 30855 - This can fail gracefully if write access is not available
BOOL CEMRProblemEditDlg::WriteToData()
{
	//
	// (c.haag 2006-11-13 13:41) - PLID 22052 - Save the problem to data
	//
	//TES 5/1/2008 - PLID 27587 - Declare the audit transaction id out here, in case we need to roll it back.
	// (z.manning 2010-01-06 11:26) - PLID 36768 - Use a CAuditTransaction to clean this up
	CAuditTransaction audit;
	try {
		// (c.haag 2006-11-13 13:41) - PLID 22052 - Get old problem information for proper auditing
		// and checks for changes
		if (-25 == m_nPatientID) {
			ASSERT(FALSE);
			ThrowNxException("The patient ID is invalid");
		}

		// (j.jones 2008-07-25 10:14) - PLID 30841 - handle saving new problems
		if(m_nProblemID == -1) {

			// (j.jones 2009-05-22 15:54) - PLID 34324 - quickly make sure we have at least one valid link
			BOOL bHasValidLinkedObject = FALSE;
			int i = 0;
			for(i=0; i < m_arypLinkedObjects.GetSize() && !bHasValidLinkedObject; i++) {
			
				LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
				if(!pInfo->bDeleted) {
					//we have at least one non-deleted object, we're ok
					bHasValidLinkedObject = TRUE;
				}
			}

			if(!bHasValidLinkedObject) {
				ThrowNxException("CEMRProblemEditDlg::WriteToData - Attempted to save a new problem with no linked objects!");
			}

			if(m_nPatientID == -25) {
				ThrowNxException("CEMRProblemEditDlg::WriteToData - Attempted to save a new problem for the -25 patient!");
			}

			
			// (a.walling 2009-05-04 09:30) - PLID 33751 - Chronicity
			// (a.walling 2009-05-04 09:30) - PLID 28495 - Diag codes
			_variant_t varChronicID; 
			if(GetProblemChronicityID() != -1) {
				varChronicID = _variant_t(GetProblemChronicityID(), VT_I4); 
			}
			else {
				varChronicID = g_cvarNull;
			}
			
			
			// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
			// an ICD-9 code and/or an ICD-10 code, no more than one pair.
			_variant_t varDiagCode9, varDiagCode10;
			if(m_nDiagICD9CodeID != -1) {
				varDiagCode9 = _variant_t(m_nDiagICD9CodeID, VT_I4);
			}
			else {
				varDiagCode9 = g_cvarNull;
			}
			if(m_nDiagICD10CodeID != -1) {
				varDiagCode10 = _variant_t(m_nDiagICD10CodeID, VT_I4); 
			}
			else {
				varDiagCode10 = g_cvarNull;
			}

			// (j.jones 2009-05-22 08:52) - PLID 34324 - changed saving to support the new structure
			CString strSqlBatch;
			CNxParamSqlArray aryParams;
			for(i=0; i < m_arypLinkedObjects.GetSize(); i++) {
			
				LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
				EMRProblemRegardingTypes eType = pInfo->eType;

				// (z.manning 2009-05-26 14:51) - PLID 34345 - Also skip this check for labs.
				if(eType != eprtEmrEMR && eType != eprtInvalid && eType != eprtUnassigned && eType != eprtLab) {

					// (a.walling 2008-07-28 15:26) - PLID 30855 - Fail if someone has access
					// (c.haag 2008-12-05 09:47) - PLID 28496 - Doesn't apply to eprtUnassigned
					// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
					// (j.armen 2013-05-14 11:56) - PLID 56680 - Use the LoginTokenID for user verification
					// (b.spivey September 24, 2013) - PLID 58677 - instead of perpetuating the same "use strings 
					//		to handle nulls" precedent, I've refactored this batch to be paramaterized. 
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						"IF EXISTS(\r\n"
						"	SELECT 1\r\n"
						"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
						"	WHERE EmnID IN ({SQL}) AND UserLoginTokenID <> {INT})\r\n"
						"BEGIN\r\n"
						"	RAISERROR('Problem cannot be saved; the EMN is being modified by another user.', 16, 43)\r\n"
						"	ROLLBACK TRAN\r\n"
						"	RETURN\r\n"
						"END\r\n",
						CEmrProblem::GetRegardingEMNFormatQueryString(eType, pInfo->nRegardingID),
						GetAPIUserLoginTokenID());
					
					// (a.walling 2008-08-27 13:29) - PLID 30855 - Use the common function to detect and warn if concurrency issues exist
					if (WarnIfEMNConcurrencyIssuesExist(this, 
						CEmrProblem::GetRegardingEMNFormatQueryString(eType, pInfo->nRegardingID),
						"This problem may not be created."))
					{
						return FALSE;
					}
				}
			}

			
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @nNewProblemID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRProblemsT WITH(UPDLOCK, HOLDLOCK))");
			// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
			// an ICD-9 code and/or an ICD-10 code, no more than one pair.
			// (s.tullis 2015-02-23 15:15) - PLID 64723 - Problem Checkbox: Add a checkbox at the top left of the EMR problem below onset date for “Do not show on CCDA”. This box is defaulted unchecked.
			// (r.gonet 2015-03-06 09:58) - PLID 65008 - Save the value of the Do Not Show On Problem Prompt checkbox as well.
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
				"INSERT INTO EMRProblemsT (ID, PatientID, Description, StatusID, OnsetDate, ChronicityID, DiagCodeID, DiagCodeID_ICD10, CodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) "
				"VALUES (@nNewProblemID, {INT}, {STRING}, {INT}, {VT_DATE}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {BIT}, {BIT})",
				m_nPatientID, GetProblemDesc(), GetProblemStatusID(), 
				((IsDateTimeValid(m_dtOnsetDate) ? _variant_t(m_dtOnsetDate, VT_DATE) : g_cvarNull)), 
				varChronicID,
				varDiagCode9, varDiagCode10,
				(m_nCodeID > 0 ? _variant_t(m_nCodeID, VT_I4) : g_cvarNull),m_bDoNotShowOnCCDA, GetProblemDoNotShowOnProblemPrompt());

			// (j.jones 2009-05-22 08:52) - PLID 34324 - changed saving to support the new structure
			for(i=0; i < m_arypLinkedObjects.GetSize(); i++) {
			
				LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
				EMRProblemRegardingTypes eType = pInfo->eType;

				long nDataID = pInfo->nEMRDataID; 
				
				// (c.haag 2009-05-11 17:21) - PLID 28494 - Insert records into the problem link table
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
					"INSERT INTO EMRProblemLinkT (EMRProblemID, EMRRegardingType, EMRRegardingID, EMRDataID) "
					"VALUES (@nNewProblemID, {VT_I4}, {VT_I4}, {VT_I4})",
					_variant_t((long)eType, VT_I4), 
					_variant_t(pInfo->nRegardingID, VT_I4), 
					(nDataID != -1 ? _variant_t(nDataID, VT_I4) : g_cvarNull));
					

				CString strNewValue;
				// (z.manning 2009-05-27 09:37) - PLID 34297 - Moved this code to a utility function
				CString strOwnerType = GetProblemTypeDescription(pInfo->eType);

				strNewValue.Format("Linked with %s: %s%s%s", strOwnerType, pInfo->strName, pInfo->strValue.IsEmpty() ? "" : " - ", pInfo->strValue);
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), audit, aeiEMNProblemLinkCreated, m_nProblemID, GetProblemDesc(), strNewValue, aepHigh, aetChanged);
			}

			// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
			// an ICD-9 code and/or an ICD-10 code, no more than one pair.
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"INSERT INTO EMRProblemHistoryT (ProblemID, Description, StatusID, UserID, ChronicityID, DiagCodeID, DiagCodeID_ICD10) "
				"VALUES (@nNewProblemID, {STRING}, {INT}, {INT}, {VT_I4}, {VT_I4}, {VT_I4})",
				GetProblemDesc(), GetProblemStatusID(), GetCurrentUserID(), varChronicID, varDiagCode9, varDiagCode10);
						

			_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteData(), ("SET NOCOUNT ON\r\n"
				"DECLARE @nNewProblemID INT\r\n"
				"BEGIN TRAN \r\n"
				+ strSqlBatch +
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nNewProblemID AS NewID\r\n"), aryParams);
			if(!rs->eof) {
				m_nProblemID = AdoFldLong(rs, "NewID");
			}
			rs->Close();

			// (z.manning, 09/06/2006) - PLID 22400 - Need to audit this here so we have a problem ID
			// rather than auditing it with a -1 record ID which would make it impossible to track.
			CString strPatientName = GetExistingPatientName(m_nPatientID);
			AuditEvent(m_nPatientID, strPatientName,audit,aeiEMNProblemDesc, m_nProblemID, "", GetProblemDesc(), aepMedium,aetCreated);
			AuditEvent(m_nPatientID, strPatientName,audit,aeiEMNProblemStatus, m_nProblemID, "",VarString(GetTableField("EMRProblemStatusT", "Name", "ID", GetProblemStatusID()), ""),aepMedium,aetCreated);
			
			// (a.walling 2009-05-04 09:30) - PLID 33751 - Chronicity
			AuditEvent(m_nPatientID, strPatientName,audit,aeiEMNProblemChronicity, m_nProblemID, "",VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", GetProblemChronicityID()), ""),aepMedium,aetCreated);
			
			// (j.jones 2014-02-24 13:15) - PLID 60781 - handle ICD-9 & 10 in the same audit
			{
				CString strICD9Code, strICD10Code;
				if(m_nDiagICD9CodeID != -1 || m_nDiagICD10CodeID != -1) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow();
					if(pRow == NULL) {
						//should not be possible
						ThrowNxException("Could not save problem - diagnosis codes are not displayed properly.");
					}
					strICD9Code = VarString(pRow->GetValue(dclcDiagICD9Code), "");
					strICD10Code = VarString(pRow->GetValue(dclcDiagICD10Code), "");
				}
				CString strOldDiagCodeAudit, strNewDiagCodeAudit;
				GetProblemICD910AuditDescriptions("", "", strICD9Code, strICD10Code, strOldDiagCodeAudit, strNewDiagCodeAudit);					
				//don't fill the old value here, even though the function gave us one
				AuditEvent(m_nPatientID, strPatientName, audit, aeiEMNProblemDiagCode, m_nProblemID, "", strNewDiagCodeAudit, aepMedium, aetCreated);
			}

			// (b.spivey, October 22, 2013) - PLID 58677 - refactored to work in EMR. 
			AuditEvent(m_nPatientID, strPatientName, audit, aeiEMNProblemSNOMEDCode, m_nProblemID, "", VarString(GetTableField("CodesT", "Code", "ID", GetProblemCodeID()), ""), aepMedium, aetCreated); 

			// (s.tullis 2015-02-23 15:44) - PLID 64723
			AuditEvent(m_nPatientID, strPatientName, audit, aeiEMNProblemDoNotShowOnCCDA, m_nProblemID, "",GetProblemDoNotShowOnCCDA()?"Checked":"Unchecked" , aepMedium, aetCreated);
			
			// (r.gonet 2015-03-06 10:16) - PLID 65008 - Audit the EMR Problem's Do Not Show On Problem Prompt flag. Always audit the value, even if not checked.
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), audit, aeiEMNProblemDoNotShowOnProblemPrompt, m_nProblemID, 
				"", GetProblemDoNotShowOnProblemPrompt() ? "Checked" : "Unchecked", aepMedium, aetCreated);


			CClient::RefreshTable(NetUtils::EMRProblemsT, m_nPatientID);
		}
		else {

			// (j.jones 2009-05-22 15:54) - PLID 34324 - quickly make sure we have at least one valid link
			BOOL bHasValidLinkedObject = FALSE;
			int i = 0;
			for(i=0; i < m_arypLinkedObjects.GetSize() && !bHasValidLinkedObject; i++) {
			
				LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
				if(!pInfo->bDeleted) {
					//we have at least one non-deleted object, we're ok
					bHasValidLinkedObject = TRUE;
				}
			}

			if(!bHasValidLinkedObject) {
				ThrowNxException("CEMRProblemEditDlg::WriteToData - Attempted to save an existing problem (%li) with no linked objects!", m_nProblemID);
			}

			// (c.haag 2008-06-16 12:44) - PLID 30319 - We now factor the names of text macro details
			// (j.jones 2008-07-15 17:21) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
			// (c.haag 2008-07-23 09:55) - PLID 30727 - Added onset date
			// (a.walling 2009-05-04 09:30) - PLID 33751 - Chronicity
			// (a.walling 2009-05-04 09:30) - PLID 28495 - Diag codes
			// (b.spivey September 24, 2013) - PLID 58677 - SNOMED ID
			// (j.jones 2014-02-24 13:24) - PLID 60781 - we have two diag codes now
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			// (r.gonet 2015-03-06 10:14) - PLID 65008 - Get the old value for the Do Not Show On Problem Prompt
			_RecordsetPtr prs = CreateParamRecordset("SELECT StatusID, Description, OnsetDate, ChronicityID, DiagCodeID, DiagCodeID_ICD10, CodeID, "
				"EmrProblemsT.DoNotShowOnCCDA, EMRProblemsT.DoNotShowOnProblemPrompt "
				"FROM EMRProblemsT "
				"WHERE EMRProblemsT.ID = {INT}", m_nProblemID);
			if (prs->eof) {
				ASSERT(FALSE); // This should never happen!
				ThrowNxException("The problem could not be found in the database");
			} else {
				CString strCurrentDesc = AdoFldString(prs, "Description");
				long nCurrentStatusID = AdoFldLong(prs, "StatusID");
				CString strOldStatus;
				CString strNewStatus;
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				COleDateTime dtOldOnsetDate = AdoFldDateTime(prs, "OnsetDate", dtInvalid);

				long nCurrentChronicityID = AdoFldLong(prs, "ChronicityID", -1);
				CString strOldChronicity;
				CString strNewChronicity;

				// (a.walling 2009-05-04 09:38) - PLID 33751 - Only necessary if we are auditing
				if (nCurrentStatusID != GetProblemStatusID()) {
					if (nCurrentStatusID != -1) {
						strOldStatus = VarString(GetTableField("EMRProblemStatusT", "Name", "ID", nCurrentStatusID), "");
					}
					if (GetProblemStatusID() != -1) {
						strNewStatus = VarString(GetTableField("EMRProblemStatusT", "Name", "ID", GetProblemStatusID()), "");
					}
				}

				// (a.walling 2009-05-04 09:39) - PLID 33751 - Chronicity statuses for auditing
				if (nCurrentChronicityID != GetProblemChronicityID()) {
					if (nCurrentChronicityID != -1) {
						strOldChronicity = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", nCurrentChronicityID), "");
					}
					if (GetProblemChronicityID() != -1) {
						strNewChronicity = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", GetProblemChronicityID()), "");
					}
				}

				// (a.walling 2009-05-04 09:40) - PLID 28495 - Diag code numbers for auditing
				// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
				// an ICD-9 code and/or an ICD-10 code, no more than one pair.
				long nCurrentDiagCodeID9 = AdoFldLong(prs, "DiagCodeID", -1);
				long nCurrentDiagCodeID10 = AdoFldLong(prs, "DiagCodeID_ICD10", -1);
				CString strOldDiagCodeAudit;
				CString strNewDiagCodeAudit;
				if (nCurrentDiagCodeID9 != m_nDiagICD9CodeID || nCurrentDiagCodeID10 != m_nDiagICD10CodeID) {
					CString strICD9CodeOld, strICD9CodeNew;
					CString strICD10CodeOld, strICD10CodeNew;
					if(nCurrentDiagCodeID9 != -1 || nCurrentDiagCodeID10 != -1) {
						GetICD9And10Codes(nCurrentDiagCodeID9, nCurrentDiagCodeID10, strICD9CodeOld, strICD10CodeOld);
					}
					if(m_nDiagICD9CodeID != -1 || m_nDiagICD10CodeID != -1) {
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow();
						if(pRow == NULL) {
							//should not be possible
							ThrowNxException("Could not save problem - diagnosis codes are not displayed properly.");
						}
						strICD9CodeNew = VarString(pRow->GetValue(dclcDiagICD9Code), "");
						strICD10CodeNew = VarString(pRow->GetValue(dclcDiagICD10Code), "");
					}
					GetProblemICD910AuditDescriptions(strICD9CodeOld, strICD10CodeOld, strICD9CodeNew, strICD10CodeNew, strOldDiagCodeAudit, strNewDiagCodeAudit);
				}

				// (b.spivey, October 22, 2013) - PLID 58677 - auditing
				long nCurrentCodeID = AdoFldLong(prs, "CodeID", -1); 
				CString strOldProblemCode;
				CString strNewProblemCode;
				if (nCurrentCodeID != GetProblemCodeID()) {
					if (nCurrentCodeID != -1) {
						strOldProblemCode = VarString(GetTableField("CodesT", "Code", "ID", nCurrentCodeID), "");
					}
					if (GetProblemCodeID() != -1) {
						strNewProblemCode = VarString(GetTableField("CodesT", "Code", "ID", GetProblemCodeID()), "");
					}
				}
				// (s.tullis 2015-02-23 15:44) - PLID 64723
				BOOL bOldDoNotShowOnCCDA = AdoFldBool(prs, "DoNotShowOnCCDA", FALSE);
				BOOL bDoNotShowOnCCDA = GetProblemDoNotShowOnCCDA();

				// (r.gonet 2015-03-06 10:16) - PLID 65008 - Get the old and new values of the Do Not Show On Problem Prompt flag so we can check and see if we need to audit them later.
				BOOL bOldDoNotShowOnProblemPrompt = AdoFldBool(prs->Fields, "DoNotShowOnProblemPrompt", FALSE);
				BOOL bNewDoNotShowOnProblemPrompt = GetProblemDoNotShowOnProblemPrompt();

				// Don't do anything if nothing changed
				// (c.haag 2008-07-23 09:53) - PLID 30727 - This makes no sense because the caller has already determined
				// that something has changed
				//if (strCurrentDesc != GetProblemDesc() ||
				//	nCurrentStatusID != GetProblemStatusID())
				//{
					// (c.haag 2006-12-27 10:33) - PLID 23158 - If the problem does not exist in data anymore, then warn
					// the user and abort the save
					// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
				if (ReturnsRecordsParam("SELECT TOP 1 ID FROM EMRProblemsT WHERE ID = {INT} AND Deleted = 0", m_nProblemID)) {

						// Save the problem details
						// (c.haag 2008-07-23 10:02) - PLID 30727 - Save the onset date
						
						// (a.walling 2008-07-28 15:26) - PLID 30855 - Fail if someone has access
						CString strSqlBatch = BeginSqlBatch();
						CNxParamSqlArray aryParams;

						// (j.jones 2009-05-22 08:52) - PLID 34324 - changed saving to support the new structure
						int i = 0;
						for(i=0; i < m_arypLinkedObjects.GetSize(); i++) {
			
							LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
							EMRProblemRegardingTypes eType = pInfo->eType;

							// (z.manning 2009-05-26 14:53) - PLID 34345 - Also skip this check for labs
							if(eType != eprtEmrEMR && eType != eprtInvalid && eType != eprtUnassigned && eType != eprtLab) {

								// (a.walling 2008-07-28 15:26) - PLID 30855 - Fail if someone has access
								// (c.haag 2008-12-05 09:47) - PLID 28496 - Doesn't apply to eprtUnassigned
								// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
								// (j.armen 2013-05-14 11:56) - PLID 56680 - Use the LoginTokenID for user verification
								// (a.walling 2013-08-08 11:41) - PLID 57925 - Flatten the query string
								AddParamStatementToSqlBatch(strSqlBatch, aryParams,
									"IF EXISTS(\r\n"
									"	SELECT 1\r\n"
									"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
									"	WHERE EmnID IN ({SQL}) AND UserLoginTokenID <> {INT})\r\n"
									"BEGIN\r\n"
									"	RAISERROR('Problem cannot be saved; the EMN is being modified by another user.', 16, 43)\r\n"
									"	ROLLBACK TRAN\r\n"
									"	RETURN\r\n"
									"END\r\n",
									CEmrProblem::GetRegardingEMNFormatQueryString(eType, pInfo->nRegardingID),
									GetAPIUserLoginTokenID());

								// (a.walling 2008-08-27 13:29) - PLID 30855 - Use the common function to detect and warn if concurrency issues exist
								if (WarnIfEMNConcurrencyIssuesExist(this, 
									CEmrProblem::GetRegardingEMNFormatQueryString(eType, pInfo->nRegardingID), 
									"This problem may not be created."))
								{
									return FALSE;
								}
							}
						}

						_variant_t varDate = !IsDateTimeValid(m_dtOnsetDate) ? g_cvarNull : _variant_t(m_dtOnsetDate, VT_DATE); 
						_variant_t varChronicID = GetProblemChronicityID() == -1 ? g_cvarNull : _variant_t(GetProblemChronicityID(), VT_I4);
						
						// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
						// an ICD-9 code and/or an ICD-10 code, no more than one pair.
						_variant_t varDiagCode9, varDiagCode10;
						if(m_nDiagICD9CodeID != -1) {
							varDiagCode9 = _variant_t(m_nDiagICD9CodeID, VT_I4);
						}
						else {
							varDiagCode9 = g_cvarNull;
						}
						if(m_nDiagICD10CodeID != -1) {
							varDiagCode10 = _variant_t(m_nDiagICD10CodeID, VT_I4); 
						}
						else {
							varDiagCode10 = g_cvarNull;
						}

						// (a.walling 2009-05-04 09:42) - PLID 33751, 28495 - Add diag code and chronicity
						// (b.spivey September 24, 2013) - PLID 58677 - Added SNOMED ID
						// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
						// an ICD-9 code and/or an ICD-10 code, no more than one pair.
						// (s.tullis 2015-02-23 15:44) - PLID 64723
						// (r.gonet 2015-03-06 10:16) - PLID 65008 - Update the EMR Problem's Do Not Show On Problem Prompt flag.
						AddParamStatementToSqlBatch(strSqlBatch, aryParams,
							"UPDATE EMRProblemsT "
							"SET Description = {STRING}, "
							"StatusID = {INT}, OnsetDate = {VT_DATE}, ModifiedDate = GetDate(), "
							"ChronicityID = {VT_I4}, "
							"DiagCodeID = {VT_I4}, DiagCodeID_ICD10 = {VT_I4}, CodeID = {VT_I4}, "
							"DoNotShowOnCCDA = {BIT}, DoNotShowOnProblemPrompt = {BIT} "
							"WHERE ID = {INT};",
							GetProblemDesc(), 
							GetProblemStatusID(), 
							varDate, 
							varChronicID, varDiagCode9, varDiagCode10, 
							(m_nCodeID > 0 ? _variant_t(m_nCodeID, VT_I4) : g_cvarNull), 
							GetProblemDoNotShowOnCCDA(), GetProblemDoNotShowOnProblemPrompt(),
							m_nProblemID);

						// (c.haag 2008-07-23 10:37) - PLID 30727 - Now write to the EMR problem history table if either the
						// status or description have changed. We don't need to log the onset date because that ideally should
						// never change once it's been entered -- it doesn't progress like the rest of the problem fields do.
						if (strCurrentDesc != GetProblemDesc() || nCurrentStatusID != GetProblemStatusID()
							|| nCurrentChronicityID != GetProblemChronicityID()
							|| nCurrentDiagCodeID9 != m_nDiagICD9CodeID
							|| nCurrentDiagCodeID10 != m_nDiagICD10CodeID)
						{
							// (a.walling 2009-05-04 09:42) - PLID 33751, 28495 - Add diag code and chronicity
							// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
							// an ICD-9 code and/or an ICD-10 code, no more than one pair.
							AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
								"INSERT INTO EMRProblemHistoryT (ProblemID, Description, StatusID, UserID, ChronicityID, DiagCodeID, DiagCodeID_ICD10) "
								"VALUES ({INT}, {STRING}, {INT}, {INT}, {VT_I4}, {VT_I4}, {VT_I4})",
								m_nProblemID, GetProblemDesc(), 
								GetProblemStatusID(), GetCurrentUserID(), 
								varChronicID, varDiagCode9, varDiagCode10);
						}

						// (j.jones 2009-05-22 16:03) - PLID 34324 - see if we need to delete problem links
						for(i=0; i < m_arypLinkedObjects.GetSize() && bHasValidLinkedObject; i++) {
						
							LinkedObjectInfo *pInfo = m_arypLinkedObjects.GetAt(i);
							if(pInfo->bDeleted) {

								//calculate the link ID
								long nEMRProblemLinkID = -1;
								if(pInfo->pEmrProblemLink != NULL) {
									nEMRProblemLinkID = pInfo->pEmrProblemLink->GetID();
								}

								if(nEMRProblemLinkID == -1) {
									nEMRProblemLinkID = pInfo->nEMRProblemLinkID;
								}

								if(nEMRProblemLinkID != -1) {
									//delete this link
									AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EmrProblemLinkT WHERE ID = {INT}", nEMRProblemLinkID);

									CString strNewValue;
									// (z.manning 2009-05-27 09:37) - PLID 34297 - Moved this code to a utility function
									CString strOwnerType = GetProblemTypeDescription(pInfo->eType);

									strNewValue.Format("Unlinked from %s: %s%s%s", strOwnerType, pInfo->strName, pInfo->strValue.IsEmpty() ? "" : " - ", pInfo->strValue);
									AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), audit, aeiEMNProblemLinkDeleted, m_nProblemID, GetProblemDesc(), strNewValue, aepHigh, aetChanged);
								}
							}
						}

						// (a.walling 2008-07-28 16:12) - PLID 30855 - Execute the batch
						ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

						// Audit the changes
						if (strCurrentDesc != GetProblemDesc()) {
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),audit,aeiEMNProblemDesc,m_nProblemID,strCurrentDesc,GetProblemDesc(),aepMedium,aetChanged);
						}
						if (nCurrentStatusID != GetProblemStatusID()) {
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),audit,aeiEMNProblemStatus,m_nProblemID,strOldStatus,strNewStatus,aepMedium,aetChanged);
						}
						// (z.manning 2011-06-17 13:25) - PLID 34932 - Don't audit if both dates are invalid.
						if (m_dtOnsetDate != dtOldOnsetDate && (IsDateTimeValid(dtOldOnsetDate) || IsDateTimeValid(m_dtOnsetDate))) {
							CString strOld = (!IsDateTimeValid(dtOldOnsetDate)) ? "" : FormatDateTimeForInterface(dtOldOnsetDate);
							CString strNew = (!IsDateTimeValid(m_dtOnsetDate)) ? "" : FormatDateTimeForInterface(m_dtOnsetDate);
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),audit,aeiEMNProblemOnsetDate,m_nProblemID,strOld,strNew,aepMedium,aetChanged);
						}
						// (a.walling 2009-05-04 09:44) - PLID 33751 - Audit the chronicity
						if (nCurrentChronicityID != GetProblemChronicityID()) {
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),audit,aeiEMNProblemChronicity,m_nProblemID,strOldChronicity,strNewChronicity,aepMedium,aetChanged);
						}

						// (a.walling 2009-05-04 09:44) - PLID 28495 - Audit the diag code
						// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
						// an ICD-9 code and/or an ICD-10 code, no more than one pair.
						if (nCurrentDiagCodeID9 != m_nDiagICD9CodeID || nCurrentDiagCodeID10 != m_nDiagICD10CodeID) {
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),audit,aeiEMNProblemDiagCode,m_nProblemID,strOldDiagCodeAudit,strNewDiagCodeAudit,aepMedium,aetChanged);
						}

						// (b.spivey, October 22, 2013) - PLID 58677 - auditing
						if (nCurrentCodeID != GetProblemCodeID()) {
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID),audit,aeiEMNProblemSNOMEDCode,m_nProblemID, strOldProblemCode, strNewProblemCode,aepMedium,aetChanged);
						}
						// (s.tullis 2015-02-23 15:44) - PLID 64723 
						if (bOldDoNotShowOnCCDA != bDoNotShowOnCCDA){
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), audit, aeiEMNProblemDoNotShowOnCCDA, m_nProblemID, bOldDoNotShowOnCCDA ? "Checked" : "Unchecked", bDoNotShowOnCCDA ? "Checked" : "Unchecked", aepMedium, aetChanged);
						}
						// (r.gonet 2015-03-06 10:16) - PLID 65008 - Audit any changes in the EMR Problem's Do Not Show On Problem Prompt flag.
						if (bOldDoNotShowOnProblemPrompt != bNewDoNotShowOnProblemPrompt) {
							AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), audit, aeiEMNProblemDoNotShowOnProblemPrompt, m_nProblemID,
								bOldDoNotShowOnProblemPrompt ? "Checked" : "Unchecked", bNewDoNotShowOnProblemPrompt ? "Checked" : "Unchecked", aepMedium, aetChanged);
						}
						// (c.haag 2006-11-13 15:11) - PLID 23158 - Now that everything is done,
						// we must send a table checker to let everyone know the problem was edited
						// (j.jones 2008-07-23 16:22) - PLID 30823 - send the patient ID, not the problem ID
						CClient::RefreshTable(NetUtils::EMRProblemsT, m_nPatientID);
					} else {
						MsgBox("This EMR problem has been deleted since opening this window. Your changes will not be saved.");
						m_bProblemWasDeleted = TRUE;
					}
				//}
			}
		}

		// (z.manning 2010-01-06 13:00) - PLID 36768 - Commit the audit transaction
		audit.Commit();

		//TES 6/3/2009 - PLID 34371 - Update the Patient Wellness qualifications for this patient.
		UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), m_nPatientID);
		// (c.haag 2010-09-21 11:35) - PLID 40612 - Create todo alarms for decisions
		if (!m_bProblemWasDeleted) {
			//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, display them to the user
			CDWordArray arNewCDSInterventions;
			UpdateDecisionRules(GetRemoteData(), m_nPatientID, arNewCDSInterventions);
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}

		return TRUE;
	} NxCatchAll("Error saving EMR problem");

	return FALSE;
}

void CEMRProblemEditDlg::OnKillFocusDateOnset()
{
	// (c.haag 2008-07-23 10:31) - PLID 30727 - Revert the onset date to its stored value if it becomes invalid.
	// Clearing it would be my second best option, but if someone accidentally types over the date and subsequently
	// makes it invalid, they'd have to cancel out of the problem and go back in if they don't remember the onset date.
	try {
		if(m_nxtOnsetDate->GetStatus() != 1) {
			// The onset date is clearly invalid
			if (IsDateTimeValid(m_dtOnsetDate)) {
				m_nxtOnsetDate->SetDateTime(m_dtOnsetDate);
			}
			else {
				m_nxtOnsetDate->Clear();
			}
		} else {
			// The onset date is valid as far as the control is concerned; but that doesn't mean we can save it to data
			// (seems shady to me that the control would allow for that). Do additional validation.
			COleDateTime dt = COleDateTime(m_nxtOnsetDate->GetDateTime());
			if (!IsDateTimeValid(dt)) {
				m_nxtOnsetDate->Clear();
			}
		}
	}
	NxCatchAll("Error in CEMRProblemEditDlg::OnKillFocusDateOnset");
}

// (a.walling 2009-05-01 17:40) - PLID 33751 
void CEMRProblemEditDlg::OnBnClickedBtnEditChronicityList()
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox dlg(this, 65, "Edit Combo Box");
		// (a.walling 2009-07-20 09:17) - PLID 33751 - Pass the in-use id to the edit function
		dlg.m_nCurIDInUse = m_nChronicityID;
		dlg.DoModal();

		m_dlChronicity->Requery();
		{
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlChronicity->GetNewRow();
			pNewRow->PutValue(0, (long)-1);
			pNewRow->PutValue(1, " <None>");
			m_dlChronicity->AddRowSorted(pNewRow, NULL);
			m_dlChronicity->TrySetSelByColumn_Deprecated(0, m_nChronicityID);

			// ensure that RequeryFinished is handled
			// before any other input event
			m_dlChronicity->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely); 
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-05-01 17:40) - PLID 33751 
void CEMRProblemEditDlg::SelChangedListProblemChronicity(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlChronicity->CurSel;
		if (pRow) {
			if (m_nChronicityID != VarLong(pRow->GetValue(0))) {
				m_nChronicityID = VarLong(pRow->GetValue(0));
				m_bModified = TRUE;
			}
		}
		else {
			pRow = lpOldSel;
			if (pRow) {
				m_dlChronicity->PutCurSel(pRow);
			}
			else {
				ASSERT(FALSE);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CEMRProblemEditDlg::RequeryFinishedListProblemChronicity(short nFlags)
{
	try {
		if (-1 != m_nChronicityID) {
			if (NULL == m_dlChronicity->FindByColumn(0, m_nChronicityID, NULL, VARIANT_TRUE)) {
				// (a.walling 2009-05-04 09:10) - PLID 33751 - Inactive chronicity
				ADODB::_RecordsetPtr prs = CreateRecordset("SELECT Name FROM EMRProblemChronicityT WHERE ID = %d",
					m_nChronicityID);
				if (!prs->eof) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlChronicity->GetNewRow();
					pRow->Value[0] = m_nChronicityID;
					pRow->Value[1] = (LPCTSTR)AdoFldString(prs, "Name");
					m_dlChronicity->CurSel = m_dlChronicity->AddRowAtEnd(pRow, NULL);
				} else {
					// (a.walling 2009-07-20 09:33) - PLID 33751 - Otherwise reset this to none
					m_nChronicityID = -1;
					m_dlChronicity->SetSelByColumn(0, m_nChronicityID);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-05-22 13:30) - PLID 34250 - supported a right click menu in the list of linked objects
void CEMRProblemEditDlg::OnRButtonDownEmrProblemLinkedObjectsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_dlLinkedObjectsList->PutCurSel(pRow);

		//if the dialog is read only, disallow even bringing up the right click menu
		if(m_bReadOnly) {
			return;
		}

		//add an ability to remove the row
		enum {
			eUnlinkFromProblem = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eUnlinkFromProblem, "&Unlink From Problem");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eUnlinkFromProblem) {

			//disallow if there is only one row
			if(m_dlLinkedObjectsList->GetRowCount() == 1) {
				AfxMessageBox("All problems must be linked to at least one item. You may not unlink this item from this problem.");
				return;
			}

			//warn before unlinking
			if(IDNO == MessageBox("Are you sure you wish to unlink this item from this problem?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}

			//grab the object
			_variant_t varObj = pRow->GetValue(lolcEMRProblemLinkObject);

			//this should always be non-null and numeric (and we're gonna
			//get an exception real fast if it's not)
			ASSERT(varObj.vt == VT_I4);

			LinkedObjectInfo *pInfo = (LinkedObjectInfo*)VarLong(varObj);

			//set the object as deleted
			pInfo->bDeleted = TRUE;

			//now remove the row from the list
			m_dlLinkedObjectsList->RemoveRow(pRow);

			m_bModified = TRUE;

			//if the parent is responsible for saving, the parent will be responsible for deletions
		}

	}NxCatchAll("Error in CEMRProblemEditDlg::OnRButtonDownEmrProblemLinkedObjectsList");
}

// (b.spivey September 24, 2013) - PLID 58677 - saving happens else where, 
//		we need to tell 'elsewhere' that we were modified
void CEMRProblemEditDlg::SelChangedListProblemSNOMED(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSNOMEDList->CurSel;
		if (pRow) {
			if (m_nCodeID != VarLong(pRow->GetValue(sclcID))) {
				m_nCodeID = VarLong(pRow->GetValue(sclcID));
				m_bModified = TRUE;
			}
		}
		else {
			pRow = lpOldSel;
			if (pRow) {
				m_dlSNOMEDList->PutCurSel(pRow);
			}
			else {
				ASSERT(FALSE);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey September 24, 2013) - PLID 58677 - null row handling
void CEMRProblemEditDlg::SelChangingListProblemSNOMED(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

// (b.spivey September 24, 2013) - PLID 58677 - Handle case if the selection has been removed.
void CEMRProblemEditDlg::RequeryFinishedListProblemSNOMEDCode(short nFlags)
{
	try {
		if (m_nCodeID > 0) {
			if (NULL == m_dlSNOMEDList->FindByColumn(0, m_nCodeID, NULL, VARIANT_TRUE)) {
				//Get Snomed code info. 
				ADODB::_RecordsetPtr prs = CreateParamRecordset(
					"SELECT ID, Code, Name, Description "
					"FROM CodesT "
					"WHERE ID = {INT} ", m_nCodeID
					);

				if (!prs->eof) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSNOMEDList->GetNewRow();
					pRow->PutValue(sclcID, m_nCodeID);
					pRow->PutValue(sclcCode, _variant_t(AdoFldString(prs, "Code", "")));
					pRow->PutValue(sclcName, _variant_t(AdoFldString(prs, "Name", "")));
					pRow->PutValue(sclcDescription, _variant_t(AdoFldString(prs, "Description", ""))); 
					m_dlSNOMEDList->CurSel = m_dlSNOMEDList->AddRowAtEnd(pRow, NULL);
				} else {
					//Can't find it, just put nothin. 
					m_nCodeID = -1;
					m_dlSNOMEDList->SetSelByColumn(0, m_nCodeID);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, October 21, 2013) - PLID 58677 - Allow opening of the UMLS dialog. 
void CEMRProblemEditDlg::OnBnClickedOpenUmls()
{
	try{
		CUTSSearchDlg dlg;
		int nResult = dlg.DoModal();
		long nRowID = VarLong(m_dlSNOMEDList->GetCurSel()->GetValue(sclcID), -1); 

		if (nResult == IDOK)
		{
			//requery our code list
			m_dlSNOMEDList->Requery();
		
			// (b.spivey November 25, 2013) - PLID 58677 - Add the "no-row" row. 
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlSNOMEDList->GetNewRow();
			pNewRow->PutValue(sclcID, (long)-1);
			pNewRow->PutValue(sclcCode, " <None>");
			pNewRow->PutValue(sclcName, _bstr_t(""));
			pNewRow->PutValue(sclcDescription, _bstr_t("")); 
			m_dlSNOMEDList->AddRowSorted(pNewRow, NULL);
			m_dlSNOMEDList->SetSelByColumn(sclcID, nRowID); 
		
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-11-05 14:49) - PLID 58982 - added patient education
void CEMRProblemEditDlg::OnBtnPtEducation()
{
	try {

		// (j.jones 2014-02-24 11:49) - PLID 60781 - added ICD-10 to EMR problems,
		// but at the moment Patient Education only supports ICD-9
		// (r.gonet 03/07/2014) - PLID 60756 - The ICD-10 code can now be used in pt edu lookup.
		if(m_nDiagICD9CodeID == -1 && m_nDiagICD10CodeID == -1) {
			AfxMessageBox("Please assign a Diagnosis Code to this Problem before accessing Patient Education.");
			return;
		}

		// (r.gonet 03/04/2014) - PLID 60756 - Pt education buttons should be based on ICD-10. If an ICD-10 is not available, 
		// then base it on an ICD-9.
		long nPrimaryRecordID = -1;
		CArray<long, long> aryAlternateRecordIDs;
		if(m_nDiagICD10CodeID != -1) {
			nPrimaryRecordID = m_nDiagICD10CodeID;
			if(m_nDiagICD9CodeID != -1) {
				aryAlternateRecordIDs.Add(m_nDiagICD9CodeID);
			}
		} else {
			nPrimaryRecordID = m_nDiagICD9CodeID;
		}

		//the info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
		// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which may be empty.
		LookupMedlinePlusInformationViaURL(this, mlpDiagCodeID, nPrimaryRecordID, aryAlternateRecordIDs);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-11-05 14:49) - PLID 58982 - added patient education
LRESULT CEMRProblemEditDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		// (j.jones 2013-10-18 09:46) - PLID 58983 - added infobutton abilities
		case IDC_PT_EDUCATION_LABEL: {
			// (j.jones 2014-02-24 11:49) - PLID 60781 - added ICD-10 to EMR problems,
			// but at the moment Patient Education only supports ICD-9
			// (r.gonet 03/04/2014) - PLID 60756 - Support sending ICD-9 and 10 codes to MedlinePlus. Prefer ICD-10s,
			// but if we don't have one or it doesn't return any results, fall back on the ICD-9.
			long nPrimaryRecordID = -1;
			CArray<long, long> aryAlternateRecordIDs;
			if(m_nDiagICD10CodeID != -1) {
				nPrimaryRecordID = m_nDiagICD10CodeID;
				if(m_nDiagICD9CodeID != -1) {
					aryAlternateRecordIDs.Add(m_nDiagICD9CodeID);
				}
			} else if(m_nDiagICD9CodeID != -1) {
				nPrimaryRecordID = m_nDiagICD9CodeID;
			} else {
				AfxMessageBox("Please assign a Diagnosis Code to this Problem before accessing Patient Education.");
				return 0;
			}
			
			//The patient education hyperlink goes to the Medline Plus website
			// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which may be empty.
			LookupMedlinePlusInformationViaSearch(this, mlpDiagCodeID, nPrimaryRecordID, aryAlternateRecordIDs);
			break;
			}
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2013-11-05 14:49) - PLID 58982 - added for patient education
BOOL CEMRProblemEditDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (j.jones 2013-11-05 14:49) - PLID 58982 - added patient education
		CRect rcPatientEducation;
		GetDlgItem(IDC_PT_EDUCATION_LABEL)->GetWindowRect(rcPatientEducation);
		ScreenToClient(&rcPatientEducation);

		// (r.gonet 2014-01-27 15:29) - PLID 59339 - Only turn the cursor into a link cursor if we are showing the
		// patient education link.
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		if ((bShowPatientEducationLink && rcPatientEducation.PtInRect(pt) && m_nxlabelPatientEducation.GetType() == dtsHyperlink)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

		return CNxDialog::OnSetCursor(pWnd, nHitTest, message);

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.jones 2014-02-24 12:32) - PLID 60781 - refreshes the datalist display
// to show the codes for the current member variables
void CEMRProblemEditDlg::ReflectCurrentDiagnosisCodes()
{
	if(m_nDiagICD9CodeID != -1 || m_nDiagICD10CodeID != -1) {
		CString strFromClause;
		//we will show codes even if inactive
		strFromClause.Format("(SELECT "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber AS DiagICD9Code, DiagCodes9.CodeDesc AS DiagICD9Desc, "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber AS DiagICD10Code, DiagCodes10.CodeDesc AS DiagICD10Desc "
			"FROM (SELECT %li AS DiagCode9ID, %li AS DiagCode10ID) AS BaseQ "
			"LEFT JOIN DiagCodes DiagCodes9 ON BaseQ.DiagCode9ID = DiagCodes9.ID "
			"LEFT JOIN DiagCodes DiagCodes10 ON BaseQ.DiagCode10ID = DiagCodes10.ID "
			") Q", m_nDiagICD9CodeID, m_nDiagICD10CodeID);
		m_DiagCodeList->PutFromClause(_bstr_t(strFromClause));
		m_DiagCodeList->Requery();
		m_DiagCodeList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}
	else {
		//if both codes are -1, just clear the list
		m_DiagCodeList->Clear();
	}

	UpdateDiagnosisListColumnSizes();
}

// (j.jones 2014-02-24 12:27) - PLID 60781 - added diagnosis search
void CEMRProblemEditDlg::OnSelChosenProblemDiagSearchList(LPDISPATCH lpRow)
{
	try {

		if(lpRow) {
			CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);

			if(results.m_ICD9.m_nDiagCodesID == -1 && results.m_ICD10.m_nDiagCodesID == -1) {
				//no code selected
				return;
			}
			
			//if we have a row, ask if they want to replace it
			if(m_DiagCodeList->GetRowCount() > 0) {
				if(IDNO == MessageBox("Only one diagnosis selection can be made at a time on an EMR Problem.\n"
					"Would you like to replace the existing diagnosis with the new diagnosis?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return;
				}

				//clear the list
				m_DiagCodeList->Clear();
				m_nDiagICD9CodeID = -1;
				m_nDiagICD10CodeID = -1;
				m_bModified = TRUE;
			}

			//apply the diagnosis IDs
			m_nDiagICD9CodeID = results.m_ICD9.m_nDiagCodesID;
			m_nDiagICD10CodeID = results.m_ICD10.m_nDiagCodesID;

			//don't call ReflectCurrentDiagnosisCodes to requery,
			//because our search provides us with all the necessary data
			m_DiagCodeList->Clear();

			_variant_t varICD9 = g_cvarNull;
			_variant_t varICD10 = g_cvarNull;
			if(results.m_ICD9.m_nDiagCodesID != -1) {
				varICD9 = results.m_ICD9.m_nDiagCodesID;
			}
			if(results.m_ICD10.m_nDiagCodesID != -1) {
				varICD10 = results.m_ICD10.m_nDiagCodesID;
			}

			//add the row
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetNewRow();
			pRow->PutValue(dclcDiagICD9CodeID, varICD9);
			pRow->PutValue(dclcDiagICD9Code, _bstr_t(results.m_ICD9.m_strCode));
			pRow->PutValue(dclcDiagICD9Desc, _bstr_t(results.m_ICD9.m_strDescription));
			pRow->PutValue(dclcDiagICD10CodeID, varICD10);
			pRow->PutValue(dclcDiagICD10Code, _bstr_t(results.m_ICD10.m_strCode));
			pRow->PutValue(dclcDiagICD10Desc, _bstr_t(results.m_ICD10.m_strDescription));
			m_DiagCodeList->AddRowAtEnd(pRow, NULL);

			m_bModified = TRUE;

			UpdateDiagnosisListColumnSizes();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-24 12:32) - PLID 60781 - resizes ICD-9 or 10 columns to
// show/hide based on the search preference and current content
void CEMRProblemEditDlg::UpdateDiagnosisListColumnSizes()
{
	try {

		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_DiagCodeList, dclcDiagICD9Code, dclcDiagICD10Code,
			50, 50, "", "", dclcDiagICD9Desc, dclcDiagICD10Desc, false, true, true);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-24 12:27) - PLID 60781 - added diagnosis search
void CEMRProblemEditDlg::OnRButtonDownProblemDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_DiagCodeList->CurSel = pRow;

		enum {
			eRemoveDiag = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveDiag, "&Remove Diagnosis Code");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveDiag) {

			//check permissions, a prompt occurs only when saving
			if (m_bReadOnly || !(GetCurrentUserPermissions(bioEMRProblems) & SPT___W________ANDPASS)
				|| (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
				return;
			}

			//remove this row
			m_DiagCodeList->RemoveRow(pRow);

			//apply the diagnosis IDs
			m_nDiagICD9CodeID = -1;
			m_nDiagICD10CodeID = -1;

			m_bModified = TRUE;			

			//we may have removed an obsolete code type (like ICD-9),
			//so we may need to resize the columns to hide old codesets
			UpdateDiagnosisListColumnSizes();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-03-06 15:46) - PLID 60781 - resize the diag columns
void CEMRProblemEditDlg::OnRequeryFinishedListEmrProblemHistory(short nFlags)
{
	try {

		UpdateHistoryListDiagnosisColumns();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-03-06 15:46) - PLID 60781 - standalone function for updating diagnosis columns in the history
void CEMRProblemEditDlg::UpdateHistoryListDiagnosisColumns()
{
	try {

		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_dlHistory, hlcDiagCodeICD9, hlcDiagCodeICD10,
			65, 65, "<None>", "<None>", -1, -1, false, false);

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2015-02-23 15:44) - PLID 64723
void CEMRProblemEditDlg::OnBnClickedCheckNoshowonccda()
{
	try{
		m_bDoNotShowOnCCDA = m_checkDoNotShowOnCCDA.GetCheck();
		m_bModified = TRUE;
	}NxCatchAll(__FUNCTION__)
}

// (r.gonet 2015-03-09 18:21) - PLID 65008 - Handles the event when the Do Not Show On Problem Prompt checkbox is checked.
void CEMRProblemEditDlg::OnBnClickedCheckDoNotShowOnProblemPrompt()
{
	try {
		m_bDoNotShowOnProblemPrompt = m_checkDoNotShowOnProblemPrompt.GetCheck();
		m_bModified = TRUE;
	} NxCatchAll(__FUNCTION__);
}