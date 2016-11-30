// Auditing.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "administratorRc.h"
#include "Auditing.h"
#include "AuditTrail.h"
#include "nxsecurity.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "RegUtils.h"
#include "AdvancedAuditConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

 

#define COLUMN_AUDIT_ID			0
#define COLUMN_AUDIT_DETAIL_ID	1
#define COLUMN_CHANGED_DATE		2
#define COLUMN_LOCATION_NAME	3
#define COLUMN_CHANGED_BY_NAME	4
// (a.walling 2010-01-22 16:15) - PLID 37018
#define COLUMN_INTERNAL_ID		5
#define COLUMN_PATIENT_ID		6
#define COLUMN_PERSON_NAME		7
// (c.haag 2010-09-07 17:22) - PLID 40198 - We now have separate item (description) and item ID columns
#define COLUMN_ITEM				8
#define COLUMN_ITEM_ID			9
#define COLUMN_RECORD_ID		10
#define COLUMN_OLD_VALUE		11
#define COLUMN_NEW_VALUE		12
#define COLUMN_PRIORITY			13
#define COLUMN_TYPE				14
#define COLUMN_CHANGERS_IP      15
#define COLUMN_SYSTEM_COMPONENT 16

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_PURGE_ALL	36785
#define ID_PURGE_CURRENT	36787

#define ID_FILTER_SELECTION	36790
#define ID_FILTER_EXCLUDING	36791
#define ID_REMOVE_FILTER	36792

// (c.haag 2010-09-07 17:29) - PLID 40198 - Timer for updating the "Loading Records..." label
#define IDT_CALL_TIMER 200

using namespace ADODB;
// (c.haag 2010-09-07 16:48) - PLID 40439 - We now use a datalist 2
using namespace NXDATALIST2Lib;
/////////////////////////////////////////////////////////////////////////////
// CAuditing dialog

// (c.haag 2010-09-07 17:29) - PLID 40198 - This event is fired when the datalist is visibly populating rows.
// It is in this function that we replace the sentinel {item} text with the actual audit description based
// on the value of the itemID column.
HRESULT __stdcall CAuditing_JITCellValue::raw_GetValue(VARIANT * pvValue, NXDATALIST2Lib::EJITGetValueReason egvrReason, NXDATALIST2Lib::IRowSettings * pRow, NXDATALIST2Lib::IColumnSettings * pCol)
{
	NXDATALIST2Lib::IRowSettingsPtr pLocRow(pRow);

	if (NULL != pLocRow && 
		(NXDATALIST2Lib::jitgvrDraw == egvrReason 
		|| NXDATALIST2Lib::jitgvrCalcExportText == egvrReason
		|| NXDATALIST2Lib::jitgvrCompareRows == egvrReason
		|| NXDATALIST2Lib::jitgvrCelltip == egvrReason
		) &&
		COLUMN_ITEM == pCol->Index) 
	{
		long nItemID = pLocRow->GetValue(COLUMN_ITEM_ID);
		_variant_t var = m_pDlgOwner->GetAuditItemDescription(nItemID);
		VariantCopy(pvValue, &var);
		return S_OK; // Tell the caller we did something
	}

	return E_NOTIMPL; // Tell the caller we're not doing anything
}

// (c.haag 2010-09-07 17:41) - PLID 40198 - This is the dialog-level override for getting the description
// of the audit item. For efficiency purposes, we do lookups in a variant map before deferring to the global
// version.
_variant_t CAuditing::GetAuditItemDescription(long nItemID)
{
	_variant_t vResult;
	if (!m_mapCachedAuditDescriptions.Lookup(nItemID, vResult)) 
	{
		vResult = _bstr_t(::GetAuditItemDescription(nItemID));
		m_mapCachedAuditDescriptions.SetAt(nItemID, vResult);
	}
	return vResult;
}

// (c.haag 2010-09-07 17:41) - PLID 40198 - Clears the map of cached audit descriptions used during painting
void CAuditing::ClearAuditItemDescriptionMap()
{
	m_mapCachedAuditDescriptions.RemoveAll();
}

CAuditing::CAuditing(CWnd* pParent)
	: CNxDialog(CAuditing::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAuditing)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	//PLID 21512: per Don, if we don't have anything to put here, default to the earliest thing we can which is new patient
	//m_strManualLocation = "NexTech_Practice_Manual.chm";
	//m_strManualBookmark = "System_Setup/Auditing/auditing.htm";
	m_hSocket = NULL;

	m_bPurgeDBEnsured = FALSE;

	m_pJITItemListValue = NULL; // (c.haag 2010-09-07 17:29) - PLID 40198
	m_nRequeryDurationInSeconds = 0; // (c.haag 2010-09-09 10:59) - PLID 40198
}

CAuditing::~CAuditing()
{
	try {
		if (m_hSocket) 
			NxSocketUtils::Disconnect(m_hSocket);

	} NxCatchAll("Error destroying socket information");
}


void CAuditing::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAuditing)
	DDX_Control(pDX, IDC_ADVANCED_AUDIT_OPTIONS, m_btnAdvancedOptions); // (a.walling 2009-12-18 10:46) - PLID 36626
	DDX_Control(pDX, IDC_COPY_OUTPUT_BTN, m_btnCopyOutput); // (b.cardillo 2010-01-07 13:26) - PLID 35780
	DDX_Control(pDX, IDC_PURGE, m_btnPurge);
	DDX_Control(pDX, IDC_RADIO_ALL_DATES, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_DATE_RANGE, m_radioDateRange);
	DDX_Control(pDX, IDC_FINANCIAL, m_financialButton);
	DDX_Control(pDX, IDC_AUDIT_INSURANCE_RADIO, m_insuranceButton);
	DDX_Control(pDX, IDC_SCHEDULE, m_scheduleButton);
	DDX_Control(pDX, IDC_AUDIT_PATIENT_RADIO, m_patientButton);
	DDX_Control(pDX, IDC_AUDIT_INV_RADIO, m_inventoryButton);
	DDX_Control(pDX, IDC_AUDIT_CONTACTS_RADIO, m_contactsButton);
	DDX_Control(pDX, IDC_AUDIT_MISC_RADIO, m_miscButton);
	DDX_Control(pDX, IDC_AUDIT_EMR_RADIO, m_emrButton);
	DDX_Control(pDX, IDC_AUDIT_PALM_RADIO, m_palmButton);
	DDX_Control(pDX, IDC_AUDIT_BACKUP_RADIO, m_backupButton);
	DDX_Control(pDX, IDC_ALL, m_allButton);
	DDX_Control(pDX, IDC_HIGH, m_highButton);
	DDX_Control(pDX, IDC_TOP, m_topButton);
	DDX_Control(pDX, IDC_AUDIT_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_AUDIT_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_BTN_AUDIT_VALIDATION, m_btnAuditValidation);
	
	//}}AFX_DATA_MAP
}


//	ON_EVENT(CAuditing, IDC_AUDIT_DATE_FROM, 2 /* Change */, OnChangeAuditDateFrom, VTS_NONE)
//	ON_EVENT(CAuditing, IDC_AUDIT_DATE_TO, 2 /* Change */, OnChangeAuditDateTo, VTS_NONE)
// (a.walling 2008-05-28 11:26) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CAuditing, CNxDialog)
	//{{AFX_MSG_MAP(CAuditing)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_AUDIT_DATE_FROM, OnChangeAuditDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_AUDIT_DATE_TO, OnChangeAuditDateTo)
	ON_BN_CLICKED(IDC_FINANCIAL, OnFinancial)
	ON_BN_CLICKED(IDC_SCHEDULE, OnSchedule)
	ON_BN_CLICKED(IDC_AUDIT_PATIENT_RADIO, OnPatient)
	ON_BN_CLICKED(IDC_ALL, OnAll)
	ON_BN_CLICKED(IDC_HIGH, OnHigh)
	ON_BN_CLICKED(IDC_TOP, OnTop)
	ON_BN_CLICKED(IDC_RADIO_ALL_DATES, OnRadioAllDates)
	ON_BN_CLICKED(IDC_RADIO_DATE_RANGE, OnRadioDateRange)
	ON_BN_CLICKED(IDC_PURGE, OnPurge)
	ON_BN_CLICKED(IDC_ADVANCED_AUDIT_OPTIONS, OnAdvancedOptions) // (a.walling 2009-12-18 10:46) - PLID 36626
	ON_BN_CLICKED(IDC_COPY_OUTPUT_BTN, OnCopyOutputBtn) // (b.cardillo 2010-01-07 13:26) - PLID 35780
	ON_BN_CLICKED(IDC_AUDIT_MISC_RADIO, OnAuditMiscRadio)
	ON_BN_CLICKED(IDC_AUDIT_PALM_RADIO, OnAuditPalmRadio)
	ON_BN_CLICKED(IDC_AUDIT_INSURANCE_RADIO, OnAuditInsuranceRadio)
	ON_BN_CLICKED(IDC_AUDIT_INV_RADIO, OnAuditInvRadio)
	ON_BN_CLICKED(IDC_AUDIT_CONTACTS_RADIO, OnAuditContactsRadio)
	ON_BN_CLICKED(IDC_AUDITING_REFRESH, OnAuditingRefresh)
	ON_BN_CLICKED(IDC_AUDITING_FILTER_TODAY, OnAuditingFilterToday)
	ON_BN_CLICKED(IDC_AUDIT_EMR_RADIO, OnAuditEmrRadio)
	ON_BN_CLICKED(IDC_AUDIT_BACKUP_RADIO, OnAuditBackupRadio)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_AUDIT_VALIDATION, &CAuditing::OnBnClickedBtnAuditValidation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuditing message handlers

BEGIN_EVENTSINK_MAP(CAuditing, CNxDialog)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CAuditing, IDC_AUDIT_LIST, 6, CAuditing::RButtonDownAuditList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CAuditing, IDC_AUDIT_LIST, 18, CAuditing::RequeryFinishedAuditList, VTS_I2)
END_EVENTSINK_MAP()

BOOL CAuditing::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
		// (c.haag 2010-09-08 13:01) - PLID 40198 - Use our own connection object. We will never open it, but
		// we will assign it all of our standard connection information (except for an inflated command timeout).
		// The datalist will grab all that information and use it to create its own connection in a worker thread.
		m_pConAudit.CreateInstance(__uuidof(Connection));
		m_pConAudit->ConnectionString = _bstr_t(GetStandardConnectionString());
		// Use the same connection timeout
		m_pConAudit->PutConnectionTimeout(GetConnectionTimeout());
		// Use a larger command timeout
		m_pConAudit->PutCommandTimeout(600);
		m_AuditList = BindNxDataList2Ctrl(IDC_AUDIT_LIST,m_pConAudit,false);
		m_DecoyList = BindNxDataList2Ctrl(IDC_AUDIT_DECOY_LIST,NULL,false);

		// (c.haag 2010-09-07 17:30) - PLID 40198 - Create the JIT object to make it so as the list is visibily
		// populated, the datalist will search for any cells with the value "{item}" in them, and if found, fire an
		// event that we will use to replace that text with the row's audit item description.
		m_pJITItemListValue = new CAuditing_JITCellValue(this);
		m_AuditList->PutBuiltInValue("item", _variant_t(m_pJITItemListValue, true));
		
		m_btnPurge.AutoSet(NXB_DELETE);
		m_btnAdvancedOptions.AutoSet(NXB_MODIFY); // (a.walling 2009-12-18 10:46) - PLID 36626	
		m_btnCopyOutput.AutoSet(NXB_EXPORT); // (b.cardillo 2010-01-07 13:26) - PLID 35780
		//m_btnAuditValidation.AutoSet(NXB_NOTUSED); 

		//as opposed to setting both to today's date, and selecting all dates by default,
		//we choose to show the past 60 days worth of audits by default, thus making the default list
		//shorter and more pertinent
		COleDateTime dt = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(60,0,0,0);
		dt -= dtSpan;
		// (a.walling 2008-05-28 17:47) - PLID 27591 - Explicitly use a COleDateTime to set the value
		m_dtFrom.SetValue(dt);
		m_dtTo.SetValue(COleDateTime::GetCurrentTime());
		m_dtFrom.EnableWindow(TRUE);
		m_dtTo.EnableWindow(TRUE);
		m_radioDateRange.SetCheck(TRUE);

		//show all items by default
		m_allButton.SetCheck(TRUE);
		m_highButton.SetCheck(FALSE);
		m_topButton.SetCheck(FALSE);

		//this function will enable/disable each category based on available permissions
		EnableControls(FALSE);

		//begin by showing financial items, as they are the most commonly viewed audits
		//m_financialButton.SetCheck(TRUE);

		//since we can't just blindly choose the Financial category, 
		ChooseFirstCategory();


		//PLID 15074 - rename the emr button based on the license
		// (a.walling 2007-11-28 13:01) - PLID 28044 - Check for expired EMR license
		if(g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent)) {
			// (z.manning 2009-01-21 10:44) - PLID 32640 - Rename to EMR Standard if they have an
			// EMR std license.
			m_emrButton.SetWindowText("EMR Standard");
		}
		else if (g_pLicense->HasEMR(CLicense::cflrSilent) == 1) {
			m_emrButton.SetWindowText("Custom Records");
		}
		else {
			m_emrButton.SetWindowText("NexEMR");
		}

		// (z.manning 2009-05-22 16:37) - PLID 34330 - We now have a separate permission for the
		// old/new value columns.
		if(!CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
			IColumnSettingsPtr pCol;
			pCol = m_AuditList->GetColumn(COLUMN_OLD_VALUE);
			pCol->PutColumnStyle(pCol->GetColumnStyle() & (~csVisible));
			pCol = m_AuditList->GetColumn(COLUMN_NEW_VALUE);
			pCol->PutColumnStyle(pCol->GetColumnStyle() & (~csVisible));
		}

		// (e.lally 2009-06-02) PLID 34396
		g_propManager.CachePropertiesInBulk("AuditingDlg", propNumber,
				"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
				"	'AuditingShowSystemComponent' \r\n"
				// (s.dhole 2014-01-20 14:53) - PLID 60363  remove preference
				//"  ,'AuditingPurgeButtonHidden' \r\n" 
				")"
				, _Q(GetCurrentUserName()));

		//Initialize
		m_bShowSysComponent = FALSE;

		// (c.haag 2010-09-09 10:42) - PLID 40198 - Reset the progress text
		m_staticProgress.SetWindowText("");

		EnsureSystemComponentVisibility();

		// (s.dhole 2013-11-15 11:32) - PLID 59522 allow only admin read permission
		if (!CheckCurrentUserPermissions(bioAdminAuditTrail,sptRead))
		{
			m_btnAuditValidation.ShowWindow(SW_HIDE);
		}
		

	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

//TES 5/15/2009 - PLID 28559 - Moved AsClauseString() to GlobalUtils

BOOL CAuditing::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch(wParam) {
			case ID_PURGE_ALL:
				PurgeAll();
				break;
			case ID_PURGE_CURRENT:
				PurgeCurrent();
				break;
			case ID_FILTER_SELECTION: {
				CWaitCursor pWait;
				m_AuditList->SetRedraw(FALSE);

				// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
				IRowSettingsPtr pRow = m_AuditList->GetLastRow();
				while (NULL != pRow) 
				{
					IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
					if (GetRowValue(pRow, m_iFilterColumnID) != m_varFilterColumnData) 
					{
						m_bFiltered = TRUE;
						m_AuditList->RemoveRow(pRow);
					}
					pRow = pPrevRow;
				}
				/*
				for(int i=m_AuditList->GetRowCount()-1;i>=0;i--) {
					if(m_AuditList->GetValue(i,m_iFilterColumnID) != m_varFilterColumnData) {
						m_bFiltered = TRUE;
						m_AuditList->RemoveRow(i);
					}
				}*/
				m_AuditList->SetRedraw(TRUE);
				break;
			}
			case ID_FILTER_EXCLUDING: {
				CWaitCursor pWait;
				m_AuditList->SetRedraw(FALSE);

				// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
				IRowSettingsPtr pRow = m_AuditList->GetLastRow();
				while (NULL != pRow) 
				{
					IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
					if (GetRowValue(pRow, m_iFilterColumnID) == m_varFilterColumnData) 
					{
						m_bFiltered = TRUE;
						m_AuditList->RemoveRow(pRow);
					}
					pRow = pPrevRow;
				}
				/*
				for(int i=m_AuditList->GetRowCount()-1;i>=0;i--) {
					if(m_AuditList->GetValue(i,m_iFilterColumnID) == m_varFilterColumnData) {
						m_bFiltered = TRUE;
						m_AuditList->RemoveRow(i);
					}
				}*/
				m_AuditList->SetRedraw(TRUE);
				break;
			}
			case ID_REMOVE_FILTER:
				OnAuditingRefresh();
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CAuditing::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try
	{
		// (s.dhole 2014-01-20 14:53) - PLID 60363 Hide purge button
		m_btnPurge.ShowWindow(SW_HIDE);
		//OnAuditingRefresh();
		//(r.wilson 11/13/2013) PLID 59468 - Shows/Hides the Purge button based on the preference
		/*m_bPurgeBtnHidden = GetRemotePropertyInt("AuditingPurgeButtonHidden",1,0, "<None>",true);
		if(m_bPurgeBtnHidden){
			m_btnPurge.ShowWindow(SW_HIDE);
		}
		else{
			m_btnPurge.ShowWindow(SW_SHOW);
		}*/
	}NxCatchAll(__FUNCTION__);

}

void CAuditing::OnFinancial() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailFinancial, sptRead)) {
		m_financialButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnSchedule() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailScheduling, sptRead)) {
		m_scheduleButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditInvRadio() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailInventory, sptRead)) {
		m_inventoryButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnPatient() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailPatients, sptRead)) {
		m_patientButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditContactsRadio() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailContacts, sptRead)) {
		m_contactsButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditPalmRadio()
{
	if(!CheckCurrentUserPermissions(bioAuditTrailPDAs, sptRead)) {
		m_palmButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditEmrRadio() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailEMR, sptRead)) {
		m_emrButton.SetCheck(FALSE);
		ChooseFirstCategory();
		
		//don't return, we should still clear the list
		//return;
	}

	//- Updating is now done by hitting the button
	//- Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditInsuranceRadio()
{
	if(!CheckCurrentUserPermissions(bioAuditTrailInsurance, sptRead)) {
		m_insuranceButton.SetCheck(FALSE);
		ChooseFirstCategory();

		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditMiscRadio() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailMisc, sptRead)) {
		m_miscButton.SetCheck(FALSE);
		ChooseFirstCategory();

		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAuditBackupRadio() 
{
	if(!CheckCurrentUserPermissions(bioAuditTrailBackup, sptRead)) {
		m_backupButton.SetCheck(FALSE);
		ChooseFirstCategory();

		//don't return, we should still clear the list
		//return;
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnAll() 
{
	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnHigh() 
{
	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();	

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnTop() 
{
	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnRadioAllDates() 
{
	//DRT 7/14/03 - Updating is now done by hitting the button
	//OnDateRange();

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnRadioDateRange() 
{
	//DRT 7/14/03 - Updating is now done by hitting the button
	//OnDateRange();	

	//DRT 9/22/03 - PLID 9512 - Clear the list when you change what you are looking at.
	m_AuditList->Clear();
}

void CAuditing::OnDateRange() {

	if(m_radioAllDates.GetCheck()) {
		m_dtFrom.EnableWindow(FALSE);
		m_dtTo.EnableWindow(FALSE);
	}
	else {
		m_dtFrom.EnableWindow(TRUE);
		m_dtTo.EnableWindow(TRUE);
	}

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
void CAuditing::OnChangeAuditDateFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	COleDateTime dt = m_dtFrom.GetValue();
	COleDateTime dtMin;
	dtMin.ParseDateTime("12/31/1899");
	if(dt.m_status == COleDateTime::invalid || dt < dtMin) {
		m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		AfxMessageBox("The date you entered was invalid. The 'from' date will be set to today's date.");
	}

	*pResult = 0;

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();
}

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
void CAuditing::OnChangeAuditDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	COleDateTime dt = m_dtTo.GetValue();
	COleDateTime dtMin;
	dtMin.ParseDateTime("12/31/1899");
	if(dt.m_status == COleDateTime::invalid || dt < dtMin) {
		m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		AfxMessageBox("The date you entered was invalid. The 'to' date will be set to today's date.");
	}

	*pResult = 0;

	//DRT 7/14/03 - Updating is now done by hitting the button
	//UpdateView();
}

void CAuditing::OnPurge() 
{
	// (j.jones 2005-05-12 11:14) - PLID 11656 - this has been broken up a little:
	// to purge all events, you need the AdminAudit write permission
	// to purge individual events, you need the category write permission
	/*
	if (!CheckCurrentUserPermissions(bioAdminAuditTrail,sptWrite))
	{
		return;
	}
	*/

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();
	long nIndex = 0;
	mnu.InsertMenu(nIndex++, MF_BYPOSITION | ((GetCurrentUserPermissions(bioAdminAuditTrail) & SPT___W________ANDPASS) ? MF_ENABLED : MF_GRAYED), ID_PURGE_ALL, "Purge &All Events Over 6 Years Old");
	if(m_AuditList->GetRowCount() > 0){
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | (CanPurgeCurrentCategory(FALSE) ? MF_ENABLED : MF_GRAYED), ID_PURGE_CURRENT, "Purge &Currently Displayed Events Over 6 Years Old");
	}
		
	CRect rc;
	CWnd *pWnd = GetDlgItem(IDC_PURGE);
	if (pWnd) {
		pWnd->GetWindowRect(&rc);
		mnu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_LEFTBUTTON, rc.left, rc.top, this, NULL);
	} else {
		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, this, NULL);
	}

	//}	
}

void CAuditing::PurgeAll()
{
	try	{

		// (j.jones 2005-05-12 11:14) - PLID 11656 - to purge all events, you need
		// the admin audit write permission
		if (!CheckCurrentUserPermissions(bioAdminAuditTrail,sptWrite))
		{
			return;
		}

		m_bPurgeDBEnsured = FALSE;

		//TES 7/19/2004 - PLID 13527 - In all cases now, we only purge things over 6 years old

		if(IDYES == MessageBox("This will clear ALL auditing information that is more than 6 years old!\n"
			"This is unrecoverable. Are you sure you wish to do this?","NexTech",MB_ICONEXCLAMATION|MB_YESNO)) {

			// (j.jones 2007-01-16 09:05) - PLID 22190 - ReturnsRecords failes with ntext columns,
			// we must use !IsRecordsetEmpty instead
			// (z.manning 2008-12-10 15:18) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors

			// (a.walling 2012-02-17 15:43) - PLID 32916 - Just check for the existence of an ID and use only the date (no time)
			long nAffected = PurgeToDisk();
			if(0 == nAffected) {
				MsgBox("All auditing records are less than three years old; no action has been taken.");
				m_bPurgeDBEnsured = FALSE;
				return;	//quit if the purge to disk failed, we don't want to delete anything that isn't kept track of
			}
			// (a.walling 2012-02-17 15:43) - PLID 32916 - Moved a lot of this within PurgeToDisk

			// CAH: This causes the list not to refresh after a purge; I don't
			// understand its purpose
//			if(m_AuditList->FromClause == _bstr_t(""))
//				return;			
			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			if(AuditID!=-1)
				AuditEvent(-1, "",AuditID,aeiAuditPurge,-1,"","Purged All Events > 6 Years Old",aepHigh);
			
			MsgBox("%li records have been succesfully moved to the purge database.", nAffected);

			RebuildList();
		}

	}NxCatchAll("Error purging audit information.");

	m_bPurgeDBEnsured = FALSE;
}

void CAuditing::PurgeCurrent()
{
	try	{

		// (j.jones 2005-05-12 11:14) - PLID 11656 - to purge currently displayed events, you need
		// the write permission for the current category
		if (!CanPurgeCurrentCategory(TRUE))
		{
			return;
		}

		m_bPurgeDBEnsured = FALSE;

		CIncreaseCommandTimeout ict(GetRemoteData(),0); // (a.vengrofski 2010-01-28 12:47) - PLID <33698> - Change the timeout to 0, then when this goes
														// out of scope it will be changed back.

		if(IDYES == MessageBox("This will clear all the auditing information currently displayed on the screen that is more than 6 years old.\n"
			"This is unrecoverable. Are you sure you wish to do this?","NexTech",MB_ICONEXCLAMATION|MB_YESNO)) {

			CWaitCursor pWait;

			long nTotalAffected = 0;

			CString strAuditDetails;

			// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
			IRowSettingsPtr pRow = m_AuditList->GetFirstRow();
			while (NULL != pRow)
			{
				long AuditDetailID = VarLong(GetRowValue(pRow, COLUMN_AUDIT_DETAIL_ID));

				strAuditDetails.AppendFormat("%li,", AuditDetailID);

				//TES 7/19/2004 - PLID 13527 - In all cases now, we only purge things over 6 years old

				// (a.walling 2012-02-17 15:43) - PLID 32916 - Moved a lot of this within PurgeToDisk

				pRow = pRow->GetNextRow();
			}

			strAuditDetails.TrimRight(",");

			if (!strAuditDetails.IsEmpty()) {		
				// (a.walling 2012-02-17 15:43) - PLID 32916 - Just pass the IDs
				nTotalAffected = PurgeToDisk(strAuditDetails);
			}

			// CAH: This causes the list not to refresh after a purge; I don't
			// understand its purpose
			//if(m_AuditList->FromClause == _bstr_t(""))
			//	return;
			if(nTotalAffected) {
				long AuditID = -1;
				AuditID = BeginNewAuditEvent();
				if(AuditID!=-1)
					AuditEvent(-1, "",AuditID,aeiAuditPurge,-1,"","Purged Selected Events > 6 Years Old",aepHigh);

				MsgBox("%li records have been succesfully moved to the purge database.", nTotalAffected);
			}
			else {
				MsgBox("No currently displayed events were more than 6 years old.  No action was taken.");
			}
			RebuildList();
		}

	}NxCatchAll("Error purging audit information.");

	m_bPurgeDBEnsured = FALSE;
}

// (c.haag 2010-09-07 17:21) - PLID 40198 - This should be used in place of GetValue whenever we want to
// fetch a value from a row
_variant_t CAuditing::GetRowValue(IRowSettingsPtr pRow, short nCol)
{
	if (COLUMN_ITEM == nCol) {
		// Special handling for the JIT column. Don't use the variant map; that is only intended for the requery
		long nItemID = pRow->GetValue(COLUMN_ITEM_ID);
		return _bstr_t(::GetAuditItemDescription(nItemID));
	} else {
		return pRow->GetValue(nCol); // (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
	}
}

// (c.haag 2010-09-07 17:21) - PLID 40198 - This function is used to build the query components that will be
// used in the audit list requery. This function will also update column titles appropriately. Most of the code in
// this function is pre-existing; I just wanted to split it out of RebuildList.
void CAuditing::PrepareListForRebuild(OUT CString& strFrom, OUT CString& strWhere, OUT CString& strSection,
										OUT AuditEventItems& aeiWhatWasViewed)
{
	//NOTE - if you change any of these queries, go to AuditTrail.cpp and change it there as well, for the report.

	//TODO - obviously, try to make these queries modular some day.

    // v.arth 05/19/09 - PLID 28569, Updated all queries to get the AuditT.IPOctet1, AuditT.IPOctet2, AuditT.IPOctet3, and AuditT.IPOctet4
	if(m_financialButton.GetCheck()) {
		//financial SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = /*"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
                 "       AuditDetailsT.*, AuditDetailsT.ID AS AuditDetailID, "
				 "       AuditDetailsT.InternalPatientID, "
				 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
				 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
				 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
				 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
				 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress "
			     "FROM"*/ "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
			     "    LEFT JOIN LineItemT WITH(NOLOCK) "
                 "        ON AuditDetailsT.RecordID = LineItemT.ID "
			     "    LEFT JOIN BillsT WITH(NOLOCK) "
                 "        ON AuditDetailsT.RecordID = BillsT.ID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 2001 AND AuditDetailsT.ItemID <= 3000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Patient Name");
		strSection = "Billing area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditFinancial;
	}
	else if(m_scheduleButton.GetCheck()) {
		//scheduling SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
			     "    LEFT JOIN AppointmentsT WITH(NOLOCK) "
                 "        ON AuditDetailsT.RecordID = AppointmentsT.ID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 3001 AND AuditDetailsT.ItemID <= 4000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Person Name");
		strSection = "Appointments area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditScheduling;
	}		
	else if(m_inventoryButton.GetCheck()) {
		//inventory SQL
		//(e.lally 2009-02-16) PLID 33078 - Added the inclusion of the ItemIDs for adding and removing allocation products to a bill (they will also display
		//	under the financial audit). I am using the enum names for searchability and clarity.
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                      "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                      "        ON AuditT.ID = AuditDetailsT.AuditID "
			          "    LEFT JOIN ServiceT WITH(NOLOCK) "
                      "        ON AuditDetailsT.RecordID = ServiceT.ID "
			          "    LEFT JOIN OrderT WITH(NOLOCK) "
                      "        ON AuditDetailsT.RecordID = OrderT.ID "
					  "    LEFT JOIN PersonT WITH(NOLOCK) "
					  "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
					  "    LEFT JOIN PatientsT WITH(NOLOCK) "
					  "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere.Format("((AuditDetailsT.ItemID >= 4001 AND AuditDetailsT.ItemID <= 5000) OR AuditDetailsT.ItemID IN(%li, %li))",
			aeiAddChargedAllocationDetail, aeiDeleteChargedAllocationDetail);

        m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Product Name");
		strSection = "Inventory area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditInventory;
	}
	else if(m_patientButton.GetCheck()) {
		//patient SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 1 AND AuditDetailsT.ItemID <= 1000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Patient Name");
		strSection = "Patients module";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditPatients;
	}
	else if(m_contactsButton.GetCheck()) {
		//contacts SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 1001 AND AuditDetailsT.ItemID <= 2000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Contact Name");
		strSection = "Contacts module";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditContacts;
	}
	else if(m_insuranceButton.GetCheck()) {
		//insurance SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 5001 AND AuditDetailsT.ItemID <= 6000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Company Name");
		strSection = "Insurance area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditInsurance;
	}
	else if(m_palmButton.GetCheck()) {
		//palm SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 6001 AND AuditDetailsT.ItemID <= 7000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Person Name");
		strSection = "PDA area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditPDA;
	}
	else if(m_emrButton.GetCheck()) {
		//EMR SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 7001 AND AuditDetailsT.ItemID <= 8000)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Person Name");
		strSection = "EMR area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditNexEMR;
	}
	else if(m_miscButton.GetCheck()) {
		//miscellaneous SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "((AuditDetailsT.ItemID >= 9001 AND AuditDetailsT.ItemID <= 9600) "
			     "    OR (AuditDetailsT.ItemID >= 9801 AND AuditDetailsT.ItemID <= 10000))";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Person Name");
		strSection = "System area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditMisc;
	}
	else if(m_backupButton.GetCheck()) {
		//miscellaneous SQL
		// (a.walling 2010-01-22 16:17) - PLID 37018 - Get the patient's userdefinedID
		// (c.haag 2010-09-07 17:22) - PLID 40198 - Drop the fields list and split the clause components
		strFrom = "AuditT "
                 "    INNER JOIN AuditDetailsT WITH(NOLOCK) "
                 "        ON AuditT.ID = AuditDetailsT.AuditID "
				 "    LEFT JOIN PersonT WITH(NOLOCK) "
				 "        ON AuditDetailsT.InternalPatientID = PersonT.ID "
				 "    LEFT JOIN PatientsT WITH(NOLOCK) "
				 "        ON PersonT.ID = PatientsT.PersonID ";
		strWhere = "(AuditDetailsT.ItemID >= 9601 AND AuditDetailsT.ItemID <= 9800)";

		m_AuditList->GetColumn(COLUMN_PERSON_NAME)->PutColumnTitle("Person Name");

		strSection = "Data Backup area";
		// (d.thompson 2010-03-16) - PLID 37721 - Auditing the view of auditing.
		aeiWhatWasViewed = aeiViewAuditBackup;
	}
}

void CAuditing::RebuildList()
{

	try {
		// (d.thompson 2010-03-16) - PLID 37721 - Used for auditing later
		AuditEventItems aeiWhatWasViewed = (AuditEventItems)-1;
		CString strAuditDateText;

		CWaitCursor pWait;
	
		m_AuditList->Clear();

		// (e.lally 2009-06-02) PLID 34396
		EnsureSystemComponentVisibility();

		// (c.haag 2010-09-07 17:21) - PLID 40198 - We no longer build a full strSQL string. We instead
		// build from and where clauses.
		CString strFrom = "";
		CString strWhere = "";
		CString strSection = "";

		// Calculate the query we're going to use
		PrepareListForRebuild(strFrom, strWhere, strSection, aeiWhatWasViewed);

		if(strFrom == "") {
			AfxMessageBox("Please select a category of audit events.");
			return;
		}

		// (c.haag 2010-09-07 17:22) - PLID 40198 - Update the where string based on priority
		if(m_allButton.GetCheck()) {
			//All Priorities
		}
		else if(m_highButton.GetCheck()) {
			//High Priorities
			strWhere += " AND (Priority = 1 OR Priority = 2)";
		}
		else if(m_topButton.GetCheck()) {
			//Top Priorities
			strWhere += " AND (Priority = 1)";
		}

		//we have a user-specified filter, so add it on
		/*
		if(m_strFilter != "") {
			if(strWhere != "")
				strWhere += " AND ";
			strWhere += m_strFilter;
		}
		*/

		if(m_radioDateRange.GetCheck()) {
			//Date Range
			CString dateFrom, dateTo, dateClause;
			COleDateTime dtTo;
			COleDateTime dtFrom = COleDateTime(m_dtFrom.GetValue());
			dateFrom = FormatDateTimeForSql(dtFrom, dtoDate);
			dtTo = m_dtTo.GetValue().date;

			// (d.thompson 2010-03-16) - PLID 37721 - Auditing date text.  Get our text before we add 1 to the dtTo
			strAuditDateText = FormatString("From %s to %s", FormatDateTimeForInterface(dtFrom, NULL, dtoDate), 
				FormatDateTimeForInterface(dtTo, NULL, dtoDate));

			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			//advance by one day, so we include all of the "to" day
			dtTo += dtSpan;
			dateTo = FormatDateTimeForSql(dtTo, dtoDate);
			dateClause.Format("(ChangedDate >= Convert(datetime,'%s') AND ChangedDate < Convert(datetime,'%s'))",dateFrom,dateTo);
			// (c.haag 2010-09-07 17:22) - PLID 40198 - Now store it in the where string
			if (!strWhere.IsEmpty()) {
				strWhere += " AND " + dateClause;
			} else {
				strWhere = dateClause;
			}
		}
		else {
			// (d.thompson 2010-03-16) - PLID 37721 - Auditing date text
			strAuditDateText = "For all dates";
		}

		// (a.walling 2010-01-25 09:35) - PLID 37056 - Check to see if they have permission to view patient names
		bool bCanViewPatientNames = CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE) ? true : false;

		//now fill the list

		// (c.haag 2010-09-07 15:28) - PLID 40198 - Do an asynchronous requery rather than manually
		// pulling the data and manually adding the rows. Before we get started, we must tune two particular
		// columns for output based on preferences and permissions
		if (m_bShowSysComponent) {
			IColumnSettingsPtr pCol = m_AuditList->GetColumn(COLUMN_SYSTEM_COMPONENT);
			CString strFieldName = FormatString("'%s'", strSection);
			pCol->FieldName = _bstr_t( FormatString("'%s'", strSection) );
		}
		else {
			IColumnSettingsPtr pCol = m_AuditList->GetColumn(COLUMN_SYSTEM_COMPONENT);
			pCol->FieldName = "NULL";
		}

		if (bCanViewPatientNames) {
			// If the user has permissions to see patient names, then we always show PersonName.
			IColumnSettingsPtr pCol = m_AuditList->GetColumn(COLUMN_PERSON_NAME);
			// (j.jones 2015-03-05 09:11) - PLID 61160 - trim this name
			pCol->FieldName = "LTRIM(RTRIM(PersonName))";
		}
		else {
			// If the user does not have permission to see patient names, suppress only patient names;
			// contact names however are still fine to see.
			IColumnSettingsPtr pCol = m_AuditList->GetColumn(COLUMN_PERSON_NAME);
			// (j.jones 2015-03-05 09:11) - PLID 61160 - trim this name
			pCol->FieldName = "(CASE WHEN InternalPatientID IS NULL THEN LTRIM(RTRIM(PersonName)) ELSE '' END) AS PersonName";
		}

		// (c.haag 2010-09-07 15:28) - PLID 40198 - Clear the map of audit descriptions if we changed
		// the From clause. It's unlikely that we will be reusing most anything in the map if the From clause
		// changed, and we want to be conscious about memory usage.
		if (m_AuditList->FromClause != _bstr_t(strFrom))
		{
			ClearAuditItemDescriptionMap();
		}

		// (c.haag 2010-09-09 10:42) - PLID 40198 - Replace the real list with a read-only empty decoy; make sure the decoy column
		// widths are as identical as possible. Even without the decoy, there are still known quirks with auto-sizing column behavior where
		// they can suddenly collapse to very narrow widths.
		for (int iCol=0; iCol < m_AuditList->GetColumnCount(); iCol++) {
			IColumnSettingsPtr(m_DecoyList->GetColumn(iCol))->StoredWidth = IColumnSettingsPtr(m_AuditList->GetColumn(iCol))->StoredWidth;
		}
		GetDlgItem(IDC_AUDIT_DECOY_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_AUDIT_LIST)->ShowWindow(SW_HIDE);

		m_AuditList->FromClause = _bstr_t(strFrom);
		m_AuditList->WhereClause = _bstr_t(strWhere);
		m_bFiltered = FALSE;
		m_AuditList->Requery();
		// (c.haag 2010-09-07 15:28) - PLID 40198 - Update the controls based on the requery state
		EnableControls();
		// (c.haag 2010-09-07 15:28) - PLID 40198 - Set a timer to update the count of loaded records
		m_nRequeryDurationInSeconds = 0;
		m_staticProgress.SetWindowText("Loading records...please wait...");
		SetTimer(IDT_CALL_TIMER, 1000, NULL);

		//_RecordsetPtr rs = CreateRecordset(strSQL);

		// (d.thompson 2010-03-16) - PLID 37721 - We need to audit when the logs are viewed, and what they viewed.  I
		//	put the code here because I want this to audit even if something failed, if we got this far.  Technically if
		//	the first iteration of the below loop failed
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiWhatWasViewed, -1, "", strAuditDateText, aepMedium, aetOpened);
		
		// (a.walling 2010-06-08 10:02) - PLID 38558 - Audit flags
		EnsureAuditFlags();

	}NxCatchAll("Error rebuilding list.");
}

// (a.walling 2012-02-20 09:25) - PLID 48238 - Seems to be the fastest way to remove any number of rows from the 
// auditing tables without making the log file explode or sql churn too much.
static void TruncateAuditTablesBatch(CSqlBatch& batch, const CString& strAuditWhere, const CString& strAuditDetailsWhere)
{	
	batch.Declare("BEGIN TRANSACTION");

	batch.Declare(
		"IF OBJECT_ID('FK_AuditDetailsT_AuditT', 'F') IS NOT NULL ALTER TABLE AuditDetailsT DROP CONSTRAINT FK_AuditDetailsT_AuditT"
	);

	batch.Add(
		"SELECT * INTO #PreserveAuditDetailsT FROM AuditDetailsT WITH (TABLOCK) %s"
		, strAuditDetailsWhere
	);

	batch.Declare("TRUNCATE TABLE AuditDetailsT");

	batch.Declare(
		"SET IDENTITY_INSERT AuditDetailsT ON "
		"INSERT INTO AuditDetailsT(ID, AuditID, RecordID, OldValue, NewValue, Priority, Type, PersonName, ItemID, InternalPatientID) SELECT * FROM #PreserveAuditDetailsT "
		"SET IDENTITY_INSERT AuditDetailsT OFF "
		"DROP TABLE #PreserveAuditDetailsT "
	);

	batch.Add(
		"SELECT * INTO #PreserveAuditT FROM AuditT WITH (TABLOCK) %s"
		, strAuditWhere
	);

	batch.Declare("TRUNCATE TABLE AuditT");

	batch.Declare(
		"SET IDENTITY_INSERT AuditT ON "
		"INSERT INTO AuditT(ID, ChangedDate, ChangedByUserName, ChangedAtLocationName, IPOctet1, IPOctet2, IPOctet3, IPOctet4) SELECT * FROM #PreserveAuditT "
		"SET IDENTITY_INSERT AuditT OFF "
		"DROP TABLE #PreserveAuditT "
	);

	batch.Declare(
		"ALTER TABLE AuditDetailsT WITH NOCHECK ADD CONSTRAINT FK_AuditDetailsT_AuditT FOREIGN KEY(AuditID) REFERENCES AuditT(ID)"
	);
	
	batch.Declare("COMMIT TRANSACTION");
}

long CAuditing::PurgeToDisk()
{
	return PurgeToDisk("");
}

//DRT 7/20/2004 - PLID 13487 - Changed the audit purging to create & write to a new 
//	purge-only database, instead of a to a file that is very easily deletable.
// (a.walling 2012-02-17 15:43) - PLID 32916 - The query param is now only for filtering on an auditdetailst.id
long CAuditing::PurgeToDisk(const CString& strAuditDetailsFilter)
{
	// (a.walling 2012-02-17 14:14) - PLID 48224 - Disable performance warnings, max records warnings, timeouts
	NxAdo::PushMaxRecordsWarningLimit pmr(-1);
	NxAdo::PushPerformanceWarningLimit ppw(-1);

	CIncreaseCommandTimeout ict(0);

	//Get the name of our purged database (creates one if it doesn't exist)
	bool bCreated = false;
	CString strPurgeTo = GetPurgeDatabaseName(bCreated);

	if(strPurgeTo.IsEmpty()) {
		//We can't continue like this!
		MsgBox("Errors were detected creating the output for the purged data.  Your data was not purged.");
		return 0;
	}

	// Make sure we have an entry for this in NxServer
	if(!m_bPurgeDBEnsured) {
		EnsurePurgeDatabaseInNxServer(strPurgeTo);
		m_bPurgeDBEnsured = TRUE;
	}

	//Open a connection to our purged database
	
	// (a.walling 2011-09-07 18:01) - PLID 45448 - NxAdo unification
	/*CNxAdoConnection purgeConn = GetRemoteConnection();
	purgeConn.SetDatabase(strPurgeTo);*/

	//{
	//	
	//	// (a.walling 2010-06-02 07:49) - PLID 31316 - Standard database connection string
	//	CString strConn = GetStandardConnectionString(true, strPurgeTo);
	//	pPurgeConn->Open(_bstr_t(strConn), "", "", 0);

	//	if(pPurgeConn == NULL || pPurgeConn->GetState() == adStateClosed)
	//		//If we couldn't open the purge database, we can't continue
	//		return false;
	//}

	// (a.walling 2012-02-18 23:41) - PLID 48230 - Now we just create new tables each time; this simplifies things,
	// esp if / when the auditing structure changes

	//See if our required tables are there, if they are not, we're going to have to create them
	/*
	{
		_RecordsetPtr prs(__uuidof(Recordset));
		CString strSql;
		strSql.Format("SELECT name FROM sysobjects WHERE name = 'AuditDetailsT' AND xtype = 'U'");
		HR(prs->Open(_variant_t(strSql), _variant_t((IDispatch *)pPurgeConn), adOpenForwardOnly, adLockReadOnly, adCmdText));

		if(prs->eof) {
			//Does not exist, we must create the tables

			//AuditT - Statement generated by Enterprise Manager from Practice as of 
			//	07/20/2004 structure. (Had to manually make the ID field primary).  I removed
			//	the identity from the ID fields, we will copy them from the main practice data.
			strSql.Format("CREATE TABLE AuditT ( "
				"	[ID] [int] NOT NULL PRIMARY KEY, "
				"	[ChangedDate] [datetime] NOT NULL , "
				"	[ChangedByUserName] [nvarchar] (50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , "
				"	[ChangedAtLocationName] [nvarchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL  "
				") ON [PRIMARY] ");
			pPurgeConn->Execute(_bstr_t(strSql), NULL, 0);

			//AuditDetailsT - Statement generated by Enterprise Manager from Practice as of 
			//	07/20/2004 structure. (Had to manually add the foreign key to AuditT).  I removed
			//	the identity from the ID fields, we will copy them from the main practice data.
			strSql.Format("CREATE TABLE AuditDetailsT ( "
				"	[ID] [int] NOT NULL PRIMARY KEY, "
				"	[AuditID] [int] NOT NULL FOREIGN KEY REFERENCES AuditT(ID), "
				"	[RecordID] [int] NOT NULL , "
				"	[OldValue] ntext COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , "
				"	[NewValue] ntext COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL , "
				"	[Priority] [int] NOT NULL , "
				"	[Type] [int] NULL , "
				"	[PersonName] [nvarchar] (255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL , "
				"	[ItemID] [int] NULL  "
				") ON [PRIMARY]");
			pPurgeConn->Execute(_bstr_t(strSql), NULL, 0);
		}

		//At this point, we have a purge database connection, and we have auditing tables correctly setup and existing
	}
	*/

	// (a.walling 2012-02-17 14:14) - PLID 48224 - We do our executions against the main connection, not pPurgeConn
	//CIncreaseCommandTimeout ict(pPurgeConn,0);	// (a.vengrofski 2010-01-28 13:07) - PLID <33698> - If deleting the rows might take longer 
												// than the timeout defualt then making the backup copy will take at least the same amout of
												// time; so change the timeout to 0, then when this goes out of scope it will be changed back.

	//Now do the full copying of files over from our main database into the purge database
	//Move all AuditT records
	//BEGIN_TRANS("AuditPurge") {
	//CString strSql;
	//strSql.Format("INSERT INTO %s.dbo.AuditT (ID, ChangedDate, ChangedByUserName, ChangedAtLocationName) "
	//	"SELECT ID, ChangedDate, ChangedByUserName, ChangedAtLocationName FROM AuditT WHERE ID IN (SELECT ID FROM (%s) SubQ) "
	//	"AND ID NOT IN (SELECT ID FROM %s.dbo.AuditT)",
	//	strPurgeTo, strQuery, strPurgeTo);
	//ExecuteSql(strSql);	//Execute this on the main database, it does the insert on the purge database

	//// (a.walling 2012-02-17 14:14) - PLID 48224 - Auditing purge will copy the AuditT records, but not the AuditDetailsT records, yet still deletes!
	//// The problem is that we have AND AuditID NOT IN (SELECT ID FROM %s.dbo.AuditT) as part of the query, and since we just inserted those, it 
	//// should always return 0 records (see PLID 32916 for an example of why this might not always be true)
	//strSql.Format("INSERT INTO %s.dbo.AuditDetailsT (ID, AuditID, RecordID, OldValue, NewValue, Priority, Type, PersonName, ItemID) "
	//	"SELECT ID, AuditID, RecordID, OldValue, NewValue, Priority, Type, PersonName, ItemID "
	//	"FROM AuditDetailsT WHERE AuditID IN (SELECT ID FROM (%s) SubQ) AND AuditID NOT IN (SELECT ID FROM %s.dbo.AuditT)",
	//	strPurgeTo, strQuery, strPurgeTo);

	//ExecuteSql(strSql);	//Execute this on the main database, it does the insert on the purge database			

	// (a.walling 2012-02-17 14:14) - PLID 48224 - Fixing this by using a declared table to hold the IDs we want

	// (a.walling 2012-02-18 23:41) - PLID 48230 - Generate our new table names
	CString strUUID = NewPlainUUID();
	CString strTimestamp = GetRemoteServerTime().Format("%Y_%m_%d@%H_%M_%S");

	CString strAuditTable = FormatString("%s..AuditT_%s$%s", strPurgeTo, strTimestamp, strUUID);
	CString strAuditDetailsTable = FormatString("%s..AuditDetailsT_%s$%s", strPurgeTo, strTimestamp, strUUID);

	CSqlBatch batch;

	batch.Declare("SET XACT_ABORT ON");
	batch.Declare("SET NOCOUNT ON");
	
	// (a.walling 2012-02-17 15:43) - PLID 32916 - Ensure we always use the same date, and use today's date without the time value
	batch.Declare("DECLARE @AuditRowCount INT");
	batch.Declare("DECLARE @AuditDetailsRowCount INT");
	batch.Declare("DECLARE @PurgeDate DATETIME");
	
	batch.Declare("SET @AuditRowCount = 0");
	batch.Declare("SET @AuditDetailsRowCount = 0");
	// (s.dhole 2013-11-15 16:57) - PLID 58925 change purge duration to six year
	batch.Add("SET @PurgeDate = DATEADD(yy, -6, dbo.AsDateNoTime(GETDATE()))");
	
	batch.Declare("DECLARE @PurgeAuditIDs TABLE (ID INT PRIMARY KEY)");

	if (strAuditDetailsFilter.IsEmpty()) {
		batch.Declare("INSERT INTO @PurgeAuditIDs SELECT ID FROM AuditT WHERE ChangedDate < @PurgeDate");
	} else {				
		batch.Declare("DECLARE @PurgeAuditDetailIDs TABLE (ID INT PRIMARY KEY)");
		batch.Add(
			"INSERT INTO @PurgeAuditDetailIDs SELECT AuditDetailsT.ID FROM AuditDetailsT "
			"INNER JOIN AuditT ON AuditDetailsT.AuditID = AuditT.ID "
			"WHERE AuditT.ChangedDate < @PurgeDate "
			"AND AuditDetailsT.ID IN (%s)", strAuditDetailsFilter);

		batch.Declare(
			"INSERT INTO @PurgeAuditIDs SELECT DISTINCT AuditT.ID FROM AuditT "
			"WHERE ID IN (SELECT ID FROM @PurgeAuditDetailIDs)");
	}

	// (a.walling 2012-02-18 23:41) - PLID 48230 - Note we no longer need to worry about checking for inserting duplicate entries
	// and etc into the new tables since we are always creating new tables; surprisingly, this was the slowest part of the whole
	// purge process, except for the bulk deletes for a full purge

	// (a.walling 2012-02-18 23:41) - PLID 48230 - Create the new table and copy the data
	batch.Add("SELECT * INTO %s FROM ("
		"SELECT * FROM AuditT "
		"WHERE ID IN (SELECT ID FROM @PurgeAuditIDs) "
		") SubQ \r\n"
		"SET @AuditRowCount = @@ROWCOUNT"
		, strAuditTable
	);

	// (a.walling 2012-02-17 14:14) - PLID 48224 - And check the AuditID against the @PurgeAuditIDs table
	// (a.walling 2012-02-17 14:14) - PLID 48224 - Also added tablock to somewhat minimize log file explosion
	if (strAuditDetailsFilter.IsEmpty()) {

		// (a.walling 2012-02-18 23:41) - PLID 48230 - Create the new table and copy the data
		batch.Add("SELECT * INTO %s FROM ("
			"SELECT * FROM AuditDetailsT "
			"WHERE AuditID IN (SELECT ID FROM @PurgeAuditIDs) "
			") SubQ \r\n"
			"SET @AuditDetailsRowCount = @@ROWCOUNT"
			, strAuditDetailsTable
		);

		// (a.walling 2012-02-17 14:14) - PLID 48224 - Also added tablock to somewhat minimize log file explosion
		// (a.walling 2012-02-20 09:25) - PLID 48238 - this was taking a half hour or more on several of my test dbs; so if we are not
		// purging a limited set (eg when strAuditDetailsFilter is empty) then we know we might be clearing out thousands upon thousands
		// of rows. So we'll just truncate the table to prevent the log file from exploding and the constant reorganization of the clustered
		// indexes
		// (a.walling 2012-02-20 09:25) - PLID 48238 - Ugh, even just deleting 20k records took over 30 mins before I killed it. Always use this approach then.
		//batch.Declare("IF (@AuditDetailsRowCount + @AuditRowCount) < 1000 BEGIN ");
		//batch.Declare(
		//	"DELETE FROM AuditDetailsT WITH (TABLOCK) "
		//	"WHERE AuditID IN (SELECT ID FROM @PurgeAuditIDs)");
		//batch.Declare(
		//	"DELETE FROM AuditT WITH (TABLOCK) "
		//	"WHERE ID IN (SELECT ID FROM @PurgeAuditIDs)");
		//batch.Declare("END ELSE BEGIN");

		// (a.walling 2012-02-20 09:25) - PLID 48238 - This approach can be slower for smaller number of removed records, but this is a rare operation, and most of the time
		// we are going to be purging a large amount of records, and waiting a half hour for that is troublesome

		TruncateAuditTablesBatch(batch, 
			//"WHERE ID NOT IN (SELECT ID FROM @PurgeAuditIDs)", 
			"WHERE ID IN (SELECT AuditID FROM AuditDetailsT WITH (TABLOCK))", // auditdetailst is cleared before auditt, so we can just use that to test whether to save the auditt record or not
			"WHERE AuditID NOT IN (SELECT ID FROM @PurgeAuditIDs)"
		);
	} else {

		// (a.walling 2012-02-18 23:41) - PLID 48230 - Create the new table and copy the data
		batch.Add("SELECT * INTO %s FROM ("
			"SELECT * FROM AuditDetailsT "
			"WHERE AuditID IN (SELECT ID FROM @PurgeAuditIDs) "
			"AND ID IN (SELECT ID FROM @PurgeAuditDetailIDs) "
			") SubQ \r\n"
			"SET @AuditDetailsRowCount = @@ROWCOUNT"
			, strAuditDetailsTable
		);

		// (a.walling 2012-02-17 14:14) - PLID 48224 - Also added tablock to somewhat minimize log file explosion
		//batch.Declare(
		//	"DELETE FROM AuditDetailsT WITH (TABLOCK) "
		//	"WHERE AuditID IN (SELECT ID FROM @PurgeAuditIDs) "
		//	"AND ID IN (SELECT ID FROM @PurgeAuditDetailIDs)");
		//batch.Declare(
		//	"DELETE FROM AuditT WITH (TABLOCK) "
		//	"WHERE ID IN (SELECT ID FROM @PurgeAuditIDs) "
		//	"AND NOT EXISTS (SELECT AuditID FROM AuditDetailsT WITH (TABLOCK) WHERE AuditID IN (SELECT ID FROM @PurgeAuditIDs))");
		
		TruncateAuditTablesBatch(batch, 
			"WHERE ID IN (SELECT AuditID FROM AuditDetailsT WITH (TABLOCK))", // auditdetailst is cleared before auditt, so we can just use that to test whether to save the auditt record or not
			"WHERE ID NOT IN (SELECT ID FROM @PurgeAuditDetailIDs)"
		);
	}

	batch.Declare("SET NOCOUNT OFF");
	batch.Declare("SELECT @AuditRowCount + @AuditDetailsRowCount AS TotalCount");
	
	long nAffected = 0;
	try {
		CWaitCursor pWait;
		ADODB::_RecordsetPtr prs = CreateRecordsetStd(GetRemoteData(), batch.m_strSqlBatch);
		if (prs->eof) {
			ASSERT(FALSE);
		} else {
			nAffected = AdoFldLong(prs, "TotalCount", 0);
		}
	} NxCatchAllThread("AuditPurge - Executing purge");

	try {
		// (a.walling 2012-02-20 09:25) - PLID 48238 - Finally shrink the log file for the purge db
		CNxAdoConnection(GetRemoteConnection()).SetDatabase(strPurgeTo).ExecuteSql("DBCC SHRINKFILE ('%s_log', 0, TRUNCATEONLY)", strPurgeTo);
	} NxCatchAllThread("AuditPurge - Truncating log");

	try {
		// (a.walling 2012-02-20 09:25) - PLID 48238 - Ensure this still exists
		ExecuteSqlStd("IF OBJECT_ID('FK_AuditDetailsT_AuditT', 'F') IS NULL ALTER TABLE AuditDetailsT WITH NOCHECK ADD CONSTRAINT FK_AuditDetailsT_AuditT FOREIGN KEY(AuditID) REFERENCES AuditT(ID);");
	} NxCatchAllThread("AuditPurge - Recreating constraint");

	try {
		// (a.walling 2012-02-20 09:25) - PLID 48238 - And ensure any options are back at default
		ExecuteSqlStd("SET XACT_ABORT ON; SET NOCOUNT OFF;");
	} NxCatchAllThread("AuditPurge - Restoring db options");

	return nAffected;

/*	DRT 7/20/2004 - PLID 13487 - Removed the old "write to file" feature of the purging.  We now create (if it doesn't exist)
		a separate Purge sql database, and write all the data into there.  This is also a bit faster, we can just do a bulk
		copy of the data.
	Below is the code that used to write to a file, preserved for nostalgia?

	try {

		CWaitCursor pWait;

		//DRT - 8/26/02 - Every record that is being deleted will first be written to a text file
		_RecordsetPtr rs = CreateRecordset(strQuery);

		//open up a text file in the PracStation directory
		CString strFilename;
		strFilename = GetSharedPath();
		strFilename += "\\AuditPurge.dat";
		CFile fOut(strFilename, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone);
		fOut.SeekToEnd();
		FieldsPtr fields = rs->Fields;

		//now loop through everything and write it out to a text file
		while(!rs->eof) {
			CString strOut, strOld, strNew;
			strOld = VarString(fields->Item["OldValue"]->Value, "");
			strOld.Remove('\n');	strOld.Remove('\r');	//get rid of newlines
			strNew = VarString(fields->Item["NewValue"]->Value, "");
			strNew.Remove('\n');	strNew.Remove('\r');	//get rid of newlines

			strOut.Format("\"%li\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%li\",\"%li\",\"%s\",\"%s\"\r\n", 
				VarLong(fields->Item["ID"]->Value,0), 
				FormatDateTimeForInterface(COleDateTime(fields->Item["ChangedDate"]->Value.date)), 
				VarString(fields->Item["ChangedByUserName"]->Value,""), 
				VarString(fields->Item["ChangedAtLocationName"]->Value,""), 
				strOld, 
				strNew, 
				VarLong(fields->Item["Priority"]->Value,0), 
				VarLong(fields->Item["Type"]->Value,0), 
				VarString(fields->Item["PersonName"]->Value,""), 
				FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), DTF_STRIP_SECONDS, dtoDateTime)	//the date we're purging
			);
			char buffer[5000];	//max length if all nvarchar fields are full is 1070, plus a little leeway for added fields + numbers/dates
			strcpy(buffer, strOut);
			fOut.Write(buffer, strlen(buffer));
			rs->MoveNext();
		}
		//cleanup
		rs->Close();
		fOut.Close();
		//
		return true;
	} NxCatchAll("Error purging to disk");

	return false;	//if we got an exception
*/
}

void CAuditing::OnAuditingRefresh() 
{

	// (c.haag 2010-09-08 09:38) - PLID 40198 - This button doubles as a Stop button in mid-requery
	if (m_AuditList->IsRequerying()) {
		m_AuditList->CancelRequery();
		m_AuditList->Clear();
		return;
	}

	if(m_radioDateRange.GetCheck() && COleDateTime(m_dtFrom.GetValue()) > COleDateTime(m_dtTo.GetValue())) {
		AfxMessageBox("Your 'from' date is after your 'to' date.\n"
			"Please correct your date range.");
		return;
	}

	RebuildList();
}

BOOL CAuditing::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == NXM_AUDIT_REFRESH) {
		OnAuditingRefresh();
	}
	
	return CNxDialog::PreTranslateMessage(pMsg);
}

void CAuditing::OnAuditingFilterToday() 
{
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	m_dtFrom.SetValue(_variant_t(dtNow));
	m_dtTo.SetValue(_variant_t(dtNow));
}

LRESULT CAuditing::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	try {
		NxSocketUtils::HandleMessage(GetSafeHwnd(), message, wParam, lParam);
	}
	NxCatchAll("Error processing NxSocketUtils message");
	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CAuditing::EnsurePurgeDatabaseInNxServer(const CString& strPurgeTo)
{
	// (c.haag 2005-10-31 12:33) - PLID 16716 - We no longer catch exceptions thrown
	// from NxServer here. We also allow the user to try again if the retry count
	// has expired.
	//
	// It doesn't appear that this function is meant to be called in a silent environment,
	// so the message box there should be fine.
	//
	BOOL bRetry = TRUE;
	while (bRetry) {
		//We only need to connect to NxServer if this happened
		m_hSocket = NxSocketUtils::Connect(GetSafeHwnd(), NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));
		// Get the backup database list
		NxSocketUtils::Send(m_hSocket, PACKET_TYPE_DBLIST_REQUEST, NULL, 0);

		long nTries = 30;
		void* pData = NULL;
	//	try {
			PacketType type;
			unsigned long dwSize;

			do {
				if (pData) delete pData;	//we don't care about any data if it was the wrong type, so delete it if we got it
				NxSocketUtils::GetPacket(GetSafeHwnd(), m_hSocket, type, pData, dwSize, 5000, FALSE);
			} while (type != PACKET_TYPE_DBLIST && --nTries > 0);
	//	} catch(...) {
	//		nTries = 0;	//just make the below code throw the error
	//	}

		//We're finished with the loop
		if (nTries == 0)
		{
			//We failed to find the packet...  We need to warn the user that they should add it manually.
			if (IDNO == MsgBox(MB_YESNO, "A timeout occurred while adding the purge database to the backup list. This may be caused by slowness on your network, or your server may be busy.\n\nWould you like to try again?")) {
				AfxThrowNxException("The purge database was setup correctly, however it could not be added to the backup list. The purge will not continue.");
			}
		}
		else {
			//The loop finished successfully, so go ahead and add the new database to the list and send it back.
			AddDatabaseToNxServer((char*)pData, strPurgeTo);
			bRetry = FALSE;
		}
		NxSocketUtils::Disconnect(m_hSocket);
		m_hSocket = NULL;
	}
}

void CAuditing::AddDatabaseToNxServer(char* szList, const CString& strNewDatabase)
{
	int nSize = 0;
	char* szOriginalList = szList;
	while (szList && *szList)
	{
		if (!stricmp(szList, strNewDatabase))
			return; // It already exists
		nSize += strlen(szList) + 1;
		szList += strlen(szList) + 1;
	}
	//add the new database to it
	char* pNew = new char[nSize + strNewDatabase.GetLength() + 2];
	memcpy(pNew, szOriginalList, nSize);
	strcpy(pNew + nSize, strNewDatabase);
	pNew[nSize + strNewDatabase.GetLength() + 1] = '\0';
	//once done, just resend it right back out
	NxSocketUtils::Send(m_hSocket, PACKET_TYPE_DBLIST, pNew, nSize + strNewDatabase.GetLength() + 2);
	delete pNew;
}

void CAuditing::EnableControls(BOOL bIsRequerying)
{
	try {

		m_financialButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailFinancial) & SPT__R_________ANDPASS);
		m_scheduleButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailScheduling) & SPT__R_________ANDPASS);
		m_inventoryButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailInventory) & SPT__R_________ANDPASS);
		m_insuranceButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailInsurance) & SPT__R_________ANDPASS);
		m_patientButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailPatients) & SPT__R_________ANDPASS);
		m_contactsButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailContacts) & SPT__R_________ANDPASS);
		m_palmButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailPDAs) & SPT__R_________ANDPASS);
		m_emrButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailEMR) & SPT__R_________ANDPASS);
		m_miscButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailMisc) & SPT__R_________ANDPASS);
		m_backupButton.EnableWindow(GetCurrentUserPermissions(bioAuditTrailBackup) & SPT__R_________ANDPASS);
		// (j.jones 2010-01-08 10:01) - PLID 35778 - this needs to be a real permission
		m_btnAdvancedOptions.EnableWindow(GetCurrentUserPermissions(bioAdvAuditOptions) & SPT___W________ANDPASS);

		// (c.haag 2010-09-08 09:38) - PLID 40198 - Disable all controls if we're requerying
		if (NULL != m_AuditList) 
		{
			// (s.dhole 2011-07-15 15:58) - PLID 44593 I have to remove this call, Because this dose not return proper valu ion Dr Levi's machine (From RequeryFinishedAuditList)
			//BOOL bIsRequerying = m_AuditList->IsRequerying();
			BOOL bEnableFilters = (bIsRequerying) ? FALSE : TRUE;

			if (bIsRequerying) {
				m_financialButton.EnableWindow(FALSE);
				m_scheduleButton.EnableWindow(FALSE);
				m_inventoryButton.EnableWindow(FALSE);
				m_insuranceButton.EnableWindow(FALSE);
				m_patientButton.EnableWindow(FALSE);
				m_contactsButton.EnableWindow(FALSE);
				m_palmButton.EnableWindow(FALSE);
				m_emrButton.EnableWindow(FALSE);
				m_miscButton.EnableWindow(FALSE);
				m_backupButton.EnableWindow(FALSE);
				m_btnAdvancedOptions.EnableWindow(FALSE);				
			}
			else {
				// These controls are already disabled by pre-existing code per permissions; nothing
				// to do here.
			}
			
			GetDlgItem(IDC_RADIO_ALL_DATES)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_RADIO_DATE_RANGE)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_AUDIT_DATE_FROM)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_AUDIT_DATE_TO)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_AUDITING_FILTER_TODAY)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_AUDITING_FILTER_TODAY)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_ALL)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_HIGH)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_TOP)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_COPY_OUTPUT_BTN)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_ADVANCED_AUDIT_OPTIONS)->EnableWindow(bEnableFilters);
			GetDlgItem(IDC_PURGE)->EnableWindow(bEnableFilters);
			// (s.dhole 2013-12-11 11:16) - PLID 59522
			GetDlgItem(IDC_BTN_AUDIT_VALIDATION)->EnableWindow(bEnableFilters);


			GetDlgItem(IDC_AUDITING_REFRESH)->EnableWindow(TRUE);
			if (bIsRequerying) {
				GetDlgItem(IDC_AUDITING_REFRESH)->SetWindowText("Cancel");
			}
			else {
				GetDlgItem(IDC_AUDITING_REFRESH)->SetWindowText("Refresh");
			}
		}

	}NxCatchAll("Error enabling controls.");
}

void CAuditing::ChooseFirstCategory()
{
	try {

		//essentially we want to check off the first category that the user has full read permission to
		if(GetCurrentUserPermissions(bioAuditTrailFinancial) & sptRead) {
			m_financialButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailScheduling) & sptRead) {
			m_scheduleButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailInventory) & sptRead) {
			m_inventoryButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailInsurance) & sptRead) {
			m_insuranceButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailPatients) & sptRead) {
			m_patientButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailContacts) & sptRead) {
			m_contactsButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailPDAs) & sptRead) {
			m_palmButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailEMR) & sptRead) {
			m_emrButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailMisc) & sptRead) {
			m_miscButton.SetCheck(TRUE);
			return;
		}

		if(GetCurrentUserPermissions(bioAuditTrailBackup) & sptRead) {
			m_backupButton.SetCheck(TRUE);
			return;
		}

	}NxCatchAll("Error initializing categories.");
}

BOOL CAuditing::CanPurgeCurrentCategory(BOOL bCheckPermission)
{
	try {

		EBuiltInObjectIDs bioID = bioInvalidID;

		if(m_financialButton.GetCheck()) {
			
			bioID = bioAuditTrailFinancial;
		}
		else if(m_scheduleButton.GetCheck()) {
			
			bioID = bioAuditTrailScheduling;
		}		
		else if(m_inventoryButton.GetCheck()) {
			
			bioID = bioAuditTrailInventory;
		}
		else if(m_patientButton.GetCheck()) {
			
			bioID = bioAuditTrailPatients;
		}
		else if(m_contactsButton.GetCheck()) {
			
			bioID = bioAuditTrailContacts;
		}
		else if(m_insuranceButton.GetCheck()) {
			
			bioID = bioAuditTrailInsurance;
		}
		else if(m_palmButton.GetCheck()) {
			
			bioID = bioAuditTrailPDAs;
		}
		else if(m_emrButton.GetCheck()) {
			
			bioID = bioAuditTrailEMR;
		}
		else if(m_miscButton.GetCheck()) {
			
			bioID = bioAuditTrailMisc;
		}
		else if(m_backupButton.GetCheck()) {
			
			bioID = bioAuditTrailBackup;
		}

		if(bioID == bioInvalidID)
			return FALSE;

		if(bCheckPermission)
			return CheckCurrentUserPermissions(bioID, sptWrite);
		else
			return (GetCurrentUserPermissions(bioID) & SPT___W________ANDPASS);

	}NxCatchAll("Error checking category permissions for purge.");

	return FALSE;
}

LRESULT CAuditing::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	return 0;
}

void CAuditing::EnsureSystemComponentVisibility()
{
	// (c.haag 2010-09-07 16:48) - PLID 40439 - Removing unnecessary explicit references to the DL1 library
	IColumnSettingsPtr pCol = m_AuditList->GetColumn(COLUMN_SYSTEM_COMPONENT);
	if(pCol != NULL){
		// (e.lally 2009-06-02) PLID 34396 - Default our preference to hide.
		if (GetRemotePropertyInt("AuditingShowSystemComponent", 0, 0, "<None>", true) == 0) {
			//Hide it
			m_bShowSysComponent = FALSE;
			pCol->ColumnStyle = csVisible;
			pCol->StoredWidth = 0;
		}
		else{
			//Show it
			m_bShowSysComponent = TRUE;
			pCol->ColumnStyle = csVisible;
			pCol->StoredWidth = 130;
		}
	}
	else{
		//We couldn't get our column!
		ASSERT(FALSE);
	}
}

// (a.walling 2009-12-18 10:46) - PLID 36626 - Open the advanced options dialog
void CAuditing::OnAdvancedOptions() 
{
	try {

		// (j.jones 2010-01-08 10:01) - PLID 35778 - this needs to be a real permission
		if(!CheckCurrentUserPermissions(bioAdvAuditOptions, sptWrite)) {
			return;
		}

		CAdvancedAuditConfigDlg dlg(this);

		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (b.cardillo 2010-01-07 13:26) - PLID 35780 - Added ability to save the current audit results to .csv
void CAuditing::OnCopyOutputBtn() 
{
	try {
		PromptSaveFile_CsvFromDLor2(m_AuditList, this, "AuditExport", TRUE);
	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-09-07 16:48) - PLID 40439 - Now a datalist 2 event (as opposed to dl1)
void CAuditing::RButtonDownAuditList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try 
	{
		// (c.haag 2010-09-08 10:04) - PLID 40198 - No filtering allowed during a requery
		if (m_AuditList->IsRequerying()) {
			return;
		}

		// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
		IRowSettingsPtr pRow(lpRow);
		m_AuditList->CurSel = pRow;

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		if(NULL != pRow) { // (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
			IColumnSettingsPtr pCol = m_AuditList->GetColumn(nCol);
			CString strClause;
			
			if (COLUMN_ITEM == nCol) {
				// (c.haag 2010-09-08 09:20) - PLID 40198 - Special handling for the JIT column. Don't use the variant
				// map; that is only intended for the requery
				long nItemID = GetRowValue(pRow,COLUMN_ITEM_ID);
				strClause = AsClauseString(_bstr_t(::GetAuditItemDescription(nItemID)));
			} else {
				strClause = AsClauseString(GetRowValue(pRow,nCol)); // (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
			}
			m_strCellString.Format("%s %s", (LPCTSTR)pCol->ColumnTitle, strClause);
			
			m_iFilterColumnID = pCol->Index;
			m_varFilterColumnData = GetRowValue(pRow,nCol); // (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2

			CString strFilterSelection = "Filter Selection " + m_strCellString;
			CString strFilterExcluding = "Filter Excluding " + m_strCellString;

			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_FILTER_SELECTION, strFilterSelection);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_FILTER_EXCLUDING, strFilterExcluding);
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		}
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REMOVE_FILTER, "Remove Filter");

		//disable the filter selection if there is none
		if(!m_bFiltered)
			mnu.EnableMenuItem(ID_REMOVE_FILTER,MF_GRAYED);

		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, this, NULL);
	}
	NxCatchAll(__FUNCTION__);
}

void CAuditing::RequeryFinishedAuditList(short nFlags)
{
	// (c.haag 2010-09-07 17:23) - PLID 40198 - Back when the audit trail was synchronously
	// populated, the row colors were updated at the end of RebuildList. Now it needs to be done
	// when the requery is finished. m_bFiltered is already FALSE here.
	try 
	{
		// CAH: Color each set of Palm audit rows based on the start and completion
		// of a synchronization. Josh, I will smack you if you take this out.
		// (j.jones 2006-10-12 16:02) - don't think I wasn't tempted
		if(m_palmButton.GetCheck())
		{
			COLORREF clrGlow = RGB(172,172,255);
			COLORREF clrWhite = RGB(220,220,255);
			COLORREF clr = clrGlow;
			long lAuditID = -1;

			// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
			IRowSettingsPtr pRow = m_AuditList->GetFirstRow();
			while (NULL != pRow)
			{
				if (VarLong(GetRowValue(pRow,COLUMN_AUDIT_ID)) != lAuditID)
				{
					lAuditID = VarLong(GetRowValue(pRow,COLUMN_AUDIT_ID));
					clr = (clr == clrWhite) ? clrGlow : clrWhite;
				}
				pRow->BackColor = clr;
				pRow = pRow->GetNextRow();
			}
		}

		// (c.haag 2010-09-07 15:28) - PLID 40198 - Update the controls based on the requery state
		if (NXDATALIST2Lib::dlRequeryFinishedCanceled == nFlags)
		{
			// Clear the list before redrawing is enabled; when a requery is cancelled the rows that did come over
			// are still in the list.
			m_AuditList->Clear();
		}

		// (c.haag 2010-09-09 10:42) - PLID 40439 - Replace the decoy with the real list
		GetDlgItem(IDC_AUDIT_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_AUDIT_DECOY_LIST)->ShowWindow(SW_HIDE);

		// (c.haag 2010-09-09 10:42) - PLID 40198 - Reset the progress text and enable controls
		KillTimer(IDT_CALL_TIMER);
		m_staticProgress.SetWindowText("");
		// (s.dhole 2011-07-15 15:55) - PLID 44593 sET bIsRequerying=fALSE
		EnableControls(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}

void CAuditing::OnDestroy()
{
	try 
	{
		CNxDialog::OnDestroy();

		// (c.haag 2010-09-07 17:31) - PLID 40198 - Because we created m_pJITItemListValue
		// in OnInitDialog, we should release it here. Do NOT delete it because the datalist still
		// references it.
		m_pJITItemListValue->Release();
		m_pJITItemListValue = NULL;
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-09-09 10:51) - PLID 40198 - Called to update the requery progress text
void CAuditing::OnTimer(UINT nIDEvent) 
{
	try {
		if (++m_nRequeryDurationInSeconds == 1) {
			m_staticProgress.SetWindowText(FormatString("Loading records...please wait...(%d second elapsed...)", m_nRequeryDurationInSeconds));
		} else {
			m_staticProgress.SetWindowText(FormatString("Loading records...please wait...(%d seconds elapsed...)", m_nRequeryDurationInSeconds));
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-11-15 11:32) - PLID 59522 Check any issue
void CAuditing::OnBnClickedBtnAuditValidation()
{
CWaitCursor pWait;
try {
		EnableControls();
		GetDlgItem(IDC_AUDITING_REFRESH)->EnableWindow(FALSE);

		_RecordsetPtr rsAudit = CreateRecordset("Select TOP 1 1 FROM AuditDetailsT inner join AuditT ON AuditDetailsT.AuditID =  AuditT.ID");
		if (rsAudit->eof){
			MsgBox(MB_ICONERROR|MB_OK,"We have detected that the Audit Log has been modified outside of NexTech software.\n\nPlease contact your System Admin or NexTech Technical Support.");
		}
		else{
			if (IDYES == MsgBox(MB_YESNO,"This may take a long time. Please be patient...\n\nDo you want to continue?")){
				// Check tampering
				if (IsAuditRecordValid(GetRemoteConnection())!=FALSE){
					MsgBox(MB_ICONINFORMATION|MB_OK,"No issues were found in Audit Log.");
				}
				else{
					MsgBox(MB_ICONERROR|MB_OK,"We have detected that the Audit Log has been modified outside of NexTech software."
							"\n\nPlease contact your System Admin or NexTech Technical Support.");
				}
			}
		}
		GetDlgItem(IDC_AUDITING_REFRESH)->EnableWindow(TRUE);
		EnableControls(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}
