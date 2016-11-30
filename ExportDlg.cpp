// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportDlg.h"
#include "ExportWizardDlg.h"
#include "filter.h"
#include "groups.h"
#include "DateTimeUtils.h"
#include "FileUtils.h"
#include "VAASCExportDlg.h"
#include "MsgBox.h"
#include "TodoUtils.h"
#include "HistoryUtils.h"
#include "NxCompressUtils.h"
#include "LabCorpInsCoExportDlg.h"
#include "HL7IDLinkExportDlg.h"	// (j.dinatale 2013-01-14 12:12) - PLID 54602
#include <NxPracticeSharedLib\ASCUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


CExportDlg::CExportDlg(CWnd* pParent)
	: CNxDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
	//}}AFX_DATA_INIT
}


void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Control(pDX, IDC_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_BTN_SYSTEM_EXPORT, m_btnASCExport);
	DDX_Control(pDX, IDC_BTN_THIRD_PARTY_EXPORTS, m_btnThirdPartyExports);
	DDX_Control(pDX, IDC_DELETE_EXPORT, m_btnDelete);
	DDX_Control(pDX, IDC_EDIT_EXPORT, m_btnEdit);
	DDX_Control(pDX, IDC_NEW_EXPORT, m_btnNew);
	DDX_Control(pDX, IDC_MANUAL_SORT_DOWN, m_nxbDown);
	DDX_Control(pDX, IDC_MANUAL_SORT_UP, m_nxbUp);
	DDX_Control(pDX, IDC_REMOVE_ONE_RECORD, m_btnRemoveOne);
	DDX_Control(pDX, IDC_REMOVE_ALL_RECORDS, m_btnRemoveAll);
	DDX_Control(pDX, IDC_ADD_ALL_RECORDS, m_btnAddAll);
	DDX_Control(pDX, IDC_ADD_ONE_RECORD, m_btnAddOne);
	DDX_Control(pDX, IDC_EXPORT_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_EXPORT_DATE_TO, m_dtTo);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CExportDlg, IDC_EXPORT_DATE_FROM, 2 /* Change */, OnChangeExportDateFrom, VTS_NONE)
//	ON_EVENT(CExportDlg, IDC_EXPORT_DATE_TO, 2 /* Change */, OnChangeExportDateTo, VTS_NONE)

BEGIN_MESSAGE_MAP(CExportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPORT_DATE_FROM, OnChangeExportDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPORT_DATE_TO, OnChangeExportDateTo)
	ON_BN_CLICKED(IDC_NEW_EXPORT, OnNewExport)
	ON_BN_CLICKED(IDC_EDIT_EXPORT, OnEditExport)
	ON_BN_CLICKED(IDC_DELETE_EXPORT, OnDeleteExport)
	ON_BN_CLICKED(IDC_ADD_ALL_RECORDS, OnAddAllRecords)
	ON_BN_CLICKED(IDC_ADD_ONE_RECORD, OnAddOneRecord)
	ON_BN_CLICKED(IDC_REMOVE_ALL_RECORDS, OnRemoveAllRecords)
	ON_BN_CLICKED(IDC_REMOVE_ONE_RECORD, OnRemoveOneRecord)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_MANUAL_SORT_UP, OnManualSortUp)
	ON_BN_CLICKED(IDC_MANUAL_SORT_DOWN, OnManualSortDown)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_BTN_SYSTEM_EXPORT, OnBtnSystemExport)
	ON_BN_CLICKED(IDC_AHCA_EXPORT, OnAHCAExport)
	ON_BN_CLICKED(IDC_KY_IPOP, OnKYIPOPExport)
	// (j.politis 2015-02-11 15:21) - PLID 64871 this line of code was accidentally overridden in revision 177332 for plid 63612
	ON_BN_CLICKED(IDC_VHI_EXPORT, OnVHIExport)
	ON_BN_CLICKED(IDC_IL_IDPH, OnILIDPHExport)
	ON_BN_CLICKED(IDC_OK_OKDH_EXPORT,OnOKDHExport)
	ON_BN_CLICKED(IDC_LABCORP_INSURANCE_EXPORT, OnLabCorpInsuranceExport)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EXPORT_LIFT_PATIENT_RESTRICTIONS, &CExportDlg::OnBnClickedExportLiftPatientRestrictions)
	ON_BN_CLICKED(IDC_BTN_THIRD_PARTY_EXPORTS, &CExportDlg::OnBnClickedBtnThirdPartyExports)
	ON_COMMAND(IDC_EXPORT_REF_PHYS_ID_LINK, &CExportDlg::OnExportRefPhys)
	ON_BN_CLICKED(IDC_PASRA_EXPORT, OnPASRAExport)
	ON_BN_CLICKED(IDC_UT_UTDH_EXPORT, OnUTDHExport)
	ON_BN_CLICKED(IDC_PA_PHC4_EXPORT, OnPHC4Export)
	ON_BN_CLICKED(IDC_OR_OHPR_EXPORT, OnOHPRExport)
	ON_BN_CLICKED(IDC_NY_ASC_EXPORT, OnNYASCExport)
	ON_BN_CLICKED(IDC_TN_ASTC, OnTNASTCExport) // (a.walling 2016-01-20 16:43) - PLID 68013 - Tennessee / TNASTC exporter
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

using namespace ADODB;

void CExportDlg::OnNewExport() 
{
	CExportWizardDlg dlg;
	dlg.m_nExportID = -1;
	try {
		int nReturn = dlg.DoModal();
		if(nReturn == IDOK || nReturn == ID_WIZFINISH) {
			IRowSettingsPtr pRow = m_pStoredExportList->GetRow(-1);
			pRow->PutValue(secID, dlg.m_nExportID);
			pRow->PutValue(secName, _bstr_t(dlg.m_strName));
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->PutValue(secLastExportDate, varNull);
			pRow->PutValue(secBasedOn, (long)dlg.m_ertRecordType);
			pRow->PutValue(secFilterType, (long)dlg.m_efoFilterOption);
			pRow->PutValue(secIncludeFieldNames, dlg.m_bIncludeFieldNames);
			pRow->PutValue(secManualSort, dlg.m_bManualSort);
			pRow->PutValue(secAllowOtherTemplates, dlg.m_bAllowOtherTemplates);
			pRow->PutValue(secExtraEmnFilter, varNull);
			pRow->PutValue(secFilterFlags, (long)dlg.m_nFilterFlags); // (z.manning 2009-12-14 09:31) - PLID 36576
			m_pStoredExportList->AddRow(pRow);
			m_pStoredExportList->SetSelByColumn(secID,dlg.m_nExportID);
			OnSelChangedStoredExportList(m_pStoredExportList->CurSel);

			//Do we need to create a todo alarm?
			if(dlg.m_bCreateTodo) {
				COleDateTime dtActive;
				switch(dlg.m_diTodoIntervalUnit) {
				case diDays:
						//OK, the date is x days from now.
						dtActive = COleDateTime::GetCurrentTime() + COleDateTimeSpan(dlg.m_nTodoIntervalAmount,0,0,0);
					break;
				case diWeeks:
						//OK, the date is x weeks from now.
						dtActive = COleDateTime::GetCurrentTime() + COleDateTimeSpan(dlg.m_nTodoIntervalAmount*7,0,0,0);
					break;
				case diMonths:
					{
						//OK, the date is x months from now.
						int nMonths = dlg.m_nTodoIntervalAmount;
						int nActiveMonth = COleDateTime::GetCurrentTime().GetMonth() + nMonths;
						int nActiveYear = COleDateTime::GetCurrentTime().GetYear();
						while(nActiveMonth > 12) {
							nActiveYear++;
							nActiveMonth -= 12;
						}
						//This may be an invalid month/day combination.
						int nActiveDay = COleDateTime::GetCurrentTime().GetDay();
						while( (nActiveMonth == 2 && (nActiveYear % 4 != 0 || (nActiveYear % 100 == 0 && nActiveYear % 400 != 0)) && nActiveDay > 28) ||
							(nActiveMonth == 2 && !(nActiveYear % 4 != 0 || (nActiveYear % 100 == 0 && nActiveYear % 400 != 0)) && nActiveDay > 29) ||
							(nActiveMonth == 4 && nActiveDay > 30) ||
							(nActiveMonth == 6 && nActiveDay > 30) ||
							(nActiveMonth == 9 && nActiveDay > 30) ||
							(nActiveMonth == 11 && nActiveDay > 30)) {
							nActiveDay--;
						}
						dtActive.SetDate(nActiveYear, nActiveMonth, nActiveDay);
					}
					break;
				}

				// (c.haag 2008-06-09 10:40) - PLID 30321 - Use a utility function to create the todo
				TodoCreate(dtActive, dtActive, dlg.m_nTodoUser, CString("Periodic Export '") + dlg.m_strName + "'", "", dlg.m_nExportID, ttExport, -1, -1, ttpMedium);
			}

			m_lExpectedTableCheckers.AddTail(dlg.m_nExportID);
			CClient::RefreshTable(NetUtils::ExportT, dlg.m_nExportID);
		}
	}catch(CException* e) {
		// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - This is an error to begin with; nothing throws CExceptions, just CException*.
		e->ReportError();
		e->Delete();
	}
}

BOOL CExportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pStoredExportList = BindNxDataListCtrl(IDC_STORED_EXPORT_LIST);
	m_pAvail = BindNxDataListCtrl(IDC_AVAILABLE_RECORDS, false);
	m_pSelect = BindNxDataListCtrl(IDC_RECORDS_TO_BE_EXPORTED, false);
	m_pDateFilters = BindNxDataListCtrl(IDC_EXPORT_DATE_FILTER_OPTIONS, false);

	// (j.jones 2008-05-08 09:15) - PLID 29953 - added nxiconbuttons for modernization
	m_btnExport.AutoSet(NXB_EXPORT);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnEdit.AutoSet(NXB_MODIFY);
	m_btnNew.AutoSet(NXB_NEW);

	m_btnAddOne.AutoSet(NXB_DOWN);
	m_btnAddAll.AutoSet(NXB_DDOWN);
	m_btnRemoveOne.AutoSet(NXB_UP);
	m_btnRemoveAll.AutoSet(NXB_UUP);
	m_nxbUp.AutoSet(NXB_UP);
	m_nxbDown.AutoSet(NXB_DOWN);

	// (a.walling 2010-10-07 09:20) - PLID 40738 - Bulk cache my props!
	g_propManager.BulkCache("CExportDlg", propbitNumber | propbitDateTime, 
		"Username = '<None>' AND Name IN ("
		"'PatientExportRestrictions_Sunrise' "
		", 'PatientExportRestrictions_Limit' "
		", 'PatientExportRestrictions_Enabled' " // (z.manning 2011-02-14 15:31) - PLID 42443
		", 'GlobalPeriod_OnlySurgicalCodes' "	// (j.jones 2012-07-24 10:45) - PLID 51737
		", 'GlobalPeriod_IgnoreModifier78' "	// (j.jones 2012-07-26 14:26) - PLID 51827
		")"
	);


	OnSelChangedStoredExportList(-1);

	// (j.jones 2006-12-27 12:19) - PLID 23281 - right now, the "system export"
	// button is labeled "ASC Export" because it only shows the AHCA export,
	// and its future uses are currently believed to be only ASC exports.
	// This may of course change later, in which case we may make it always show.
	// But for now, only show when the client has ASC.
	// (j.jones 2007-07-03 09:09) - PLID 25493 - we now also show the VHI export,
	// but it too is ASC only
	// (z.manning 2015-05-13 10:38) - PLID 65960 - We now have a license specifically for State ASC reports
	BOOL bHasStateASCReportsLicense = g_pLicense->CheckForLicense(CLicense::lcStateASCReports, CLicense::cflrSilent);
	GetDlgItem(IDC_BTN_SYSTEM_EXPORT)->ShowWindow(bHasStateASCReportsLicense ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_BTN_SYSTEM_EXPORT)->EnableWindow(bHasStateASCReportsLicense);

	// (a.walling 2010-10-05 13:12) - PLID 40822 - Check for NexTech 'superuser'
	if (-26 == GetCurrentUserID()) {
		CNxIconButton* pButton = (CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_EXPORT_LIFT_PATIENT_RESTRICTIONS);
		pButton->AutoSet(NXB_MODIFY);
		pButton->ShowWindow(SW_SHOWNA);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportDlg::OnEditExport() 
{
	if(m_pStoredExportList->CurSel == -1) {
		MsgBox("Please select an export to edit.");
		return;
	}

	CExportWizardDlg dlg;
	dlg.m_nExportID = VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secID));
	try {
		int nReturn = dlg.DoModal();
		if(nReturn == IDOK || nReturn == ID_WIZFINISH) {
			//Regenerate the todo alarm.
			if(dlg.m_bCreateTodo) {
				COleDateTime dtActive;
				switch((DateInterval)dlg.m_diTodoIntervalUnit) {
					case diDays:
						//OK, the date is x days from now.
						dtActive = COleDateTime::GetCurrentTime() + COleDateTimeSpan(dlg.m_nTodoIntervalAmount,0,0,0);
					break;
				case diWeeks:
						//OK, the date is x weeks from now.
						dtActive = COleDateTime::GetCurrentTime() + COleDateTimeSpan(dlg.m_nTodoIntervalAmount*7,0,0,0);
					break;
				case diMonths:
					{
						//OK, the date is x months from now.
						int nMonths = dlg.m_nTodoIntervalAmount;
						int nActiveMonth = COleDateTime::GetCurrentTime().GetMonth() + nMonths;
						int nActiveYear = COleDateTime::GetCurrentTime().GetYear();
						while(nActiveMonth > 12) {
							nActiveYear++;
							nActiveMonth -= 12;
						}
						//This may be an invalid month/day combination.
						int nActiveDay = COleDateTime::GetCurrentTime().GetDay();
						while( (nActiveMonth == 2 && (nActiveYear % 4 != 0 || (nActiveYear % 100 == 0 && nActiveYear % 400 != 0)) && nActiveDay > 28) ||
							(nActiveMonth == 2 && !(nActiveYear % 4 != 0 || (nActiveYear % 100 == 0 && nActiveYear % 400 != 0)) && nActiveDay > 29) ||
							(nActiveMonth == 4 && nActiveDay > 30) ||
							(nActiveMonth == 6 && nActiveDay > 30) ||
							(nActiveMonth == 9 && nActiveDay > 30) ||
							(nActiveMonth == 11 && nActiveDay > 30)) {
							nActiveDay--;
						}
						dtActive.SetDate(nActiveYear, nActiveMonth, nActiveDay);
					}
					break;
				}
				// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
				TodoDelete(FormatString("Done Is Null AND RegardingID = %li AND RegardingType = %i", dlg.m_nExportID, ttExport));
				// (c.haag 2008-06-09 10:53) - PLID 30321 - Use a utility function to create the todo
				TodoCreate(dtActive, dtActive, dlg.m_nTodoUser, CString("Periodic Export '") + VarString(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secName)) + "'",
					"", dlg.m_nExportID, ttExport, -1, -1, ttpMedium);
			}
			else {
				// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
				TodoDelete(FormatString("Done Is Null AND RegardingID = %li AND RegardingType = %i", dlg.m_nExportID, ttExport));
			}

			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secName, _bstr_t(dlg.m_strName));
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secBasedOn, (long)dlg.m_ertRecordType);
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secFilterType, (long)dlg.m_efoFilterOption);
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secIncludeFieldNames, dlg.m_bIncludeFieldNames);
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secManualSort, dlg.m_bManualSort);
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secAllowOtherTemplates, dlg.m_bAllowOtherTemplates);
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secFilterFlags, (long)dlg.m_nFilterFlags); // (z.manning 2009-12-14 09:32) - PLID 36576
			_variant_t varNull;
			varNull.vt = VT_NULL;
			m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secExtraEmnFilter, varNull);

			OnSelChangedStoredExportList(m_pStoredExportList->CurSel);

			m_lExpectedTableCheckers.AddTail(dlg.m_nExportID);
			CClient::RefreshTable(NetUtils::ExportT, dlg.m_nExportID);
		}
	}catch(CException* e) {
		// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - This is an error to begin with; nothing throws CExceptions, just CException*.
		e->ReportError();
		e->Delete();
	}
}

void CExportDlg::OnDeleteExport() 
{
	if(m_pStoredExportList->CurSel == -1) {
		MsgBox("Please select an export to edit.");
		return;
	}

	try {
		if(IDYES != MsgBox(MB_YESNO, "Are you sure you want to delete the stored export '%s'?  This action can NOT be undone!",
			VarString(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secName)))) {
			return;
		}

		long nExportID = VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secID));
		ExecuteSql("DELETE FROM ExportHistoryT WHERE ExportID = %li", nExportID);
		ExecuteSql("DELETE FROM ExportSortFieldsT WHERE ExportID = %li", nExportID);
		ExecuteSql("DELETE FROM ExportSpecialCharsT WHERE ExportID = %li", nExportID);
		ExecuteSql("DELETE FROM ExportFieldsT WHERE ExportID = %li", nExportID);
		ExecuteSql("DELETE FROM ExportEmnTemplatesT WHERE ExportID = %li", nExportID);
		// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
		TodoDelete(FormatString("RegardingID = %li AND RegardingType = %li", nExportID, ttExport));
		// (z.manning 2009-12-11 09:41) - PLID 36519 - Handle export history categories
		ExecuteParamSql("DELETE FROM ExportHistoryCategoriesT WHERE ExportID = {INT}", nExportID);
		ExecuteSql("DELETE FROM ExportT WHERE ID = %li", nExportID);

		m_pStoredExportList->RemoveRow(m_pStoredExportList->CurSel);
		OnSelChangedStoredExportList(m_pStoredExportList->CurSel);

		m_lExpectedTableCheckers.AddTail(nExportID);
		CClient::RefreshTable(NetUtils::ExportT, nExportID);

	}NxCatchAll("Error in CExportDlg::OnDeleteExport()");
}

BEGIN_EVENTSINK_MAP(CExportDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CExportDlg)
	ON_EVENT(CExportDlg, IDC_STORED_EXPORT_LIST, 2 /* SelChanged */, OnSelChangedStoredExportList, VTS_I4)
	ON_EVENT(CExportDlg, IDC_EXPORT_DATE_FILTER_OPTIONS, 16 /* SelChosen */, OnSelChosenExportDateFilterOptions, VTS_I4)
	ON_EVENT(CExportDlg, IDC_AVAILABLE_RECORDS, 2 /* SelChanged */, OnSelChangedAvailableRecords, VTS_I4)
	ON_EVENT(CExportDlg, IDC_AVAILABLE_RECORDS, 3 /* DblClickCell */, OnDblClickCellAvailableRecords, VTS_I4 VTS_I2)
	// (b.cardillo 2007-02-16 14:56) - PLID 24791 - Added RequeryFinished event handler.  See implementation comments for more info.
	ON_EVENT(CExportDlg, IDC_AVAILABLE_RECORDS, 18 /* RequeryFinished */, OnRequeryFinishedAvailableRecords, VTS_I2)
	ON_EVENT(CExportDlg, IDC_RECORDS_TO_BE_EXPORTED, 2 /* SelChanged */, OnSelChangedRecordsToBeExported, VTS_I4)
	ON_EVENT(CExportDlg, IDC_RECORDS_TO_BE_EXPORTED, 3 /* DblClickCell */, OnDblClickCellRecordsToBeExported, VTS_I4 VTS_I2)
	ON_EVENT(CExportDlg, IDC_STORED_EXPORT_LIST, 3 /* DblClickCell */, OnDblClickCellStoredExportList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

COleDateTime GetSpecialDate(DateFilterOption dfo)
{
	COleDateTime dt;
	COleDateTime dtToday = COleDateTime::GetCurrentTime();
	dtToday.SetDateTime(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay(), 0, 0, 0);
	ASSERT(dfo != dfoCustom);
	switch(dfo) {
	case dfoToday:
		dt = dtToday;
		break;

	case dfoYesterday:
		dt = dtToday - COleDateTimeSpan(1,0,0,0);
		break;

	case dfoTomorrow:
		dt = dtToday + COleDateTimeSpan(1,0,0,0);
		break;

	case dfoFirstOfWeek:
		dt = dtToday;
		while(dt.GetDayOfWeek() != 1) {
			dt -= COleDateTimeSpan(1,0,0,0);
		}
		break;

	case dfoFirstOfMonth:
		dt = COleDateTime(dtToday.GetYear(), dtToday.GetMonth(), 1, 0, 0, 0);
		break;
		
	case dfoFirstOfYear:
		dt = COleDateTime(dtToday.GetYear(), 1, 1, 0, 0, 0);
		break;

	case dfoLastOfWeek:
		dt = dtToday;
		while(dt.GetDayOfWeek() != 7) {
			dt += COleDateTimeSpan(1,0,0,0);
		}
		break;

	case dfoLastOfMonth:
		if(dtToday.GetMonth() == 12) {
			dt = COleDateTime(dtToday.GetYear()+1,1,1,0,0,0);
		}
		else {
			dt = COleDateTime(dtToday.GetYear(),dtToday.GetMonth()+1,1,0,0,0);
		}
		dt -= COleDateTimeSpan(1,0,0,0);
		break;

	case dfoLastOfYear:
		dt = COleDateTime(dtToday.GetYear(),12,31,0,0,0);
		break;

	case dfoFirstOfLastMonth:
		if(dtToday.GetMonth() == 1) {
			dt = COleDateTime(dtToday.GetYear()-1,12,1,0,0,0);
		}
		else {
			dt = COleDateTime(dtToday.GetYear(),dtToday.GetMonth()-1,1,0,0,0);
		}
		break;

	case dfoOneMonthAgo:
		if(dtToday.GetMonth() == 1) {
			dt = COleDateTime(dtToday.GetYear()-1, 12, dtToday.GetDay(),0,0,0);
		}
		else if(dtToday.GetMonth() == 3) {
			//Is this a leap year?
			if(dtToday.GetYear() % 4 != 0 || (dtToday.GetYear() % 100 == 0 && dtToday.GetYear() % 400 != 0)) {
				//No.
				if(dtToday.GetDay() > 28) {
					dt = COleDateTime(dtToday.GetYear(), 2, 28, 0, 0, 0);
				}
				else {
					dt = COleDateTime(dtToday.GetYear(), 2, dtToday.GetDay(), 0, 0, 0);
				}
			}
			else {
				//Yes.
				if(dtToday.GetDay() > 29) {
					dt = COleDateTime(dtToday.GetYear(), 2, 29, 0, 0, 0);
				}
				else {
					dt = COleDateTime(dtToday.GetYear(), 2, dtToday.GetDay(), 0, 0, 0);
				}
			}
		}
		else if(dtToday.GetDay() == 31 && (dtToday.GetMonth() == 5 || dtToday.GetMonth() == 7 || dtToday.GetMonth() == 10 || dtToday.GetMonth() == 12)) {
			dt = COleDateTime(dtToday.GetYear(), dtToday.GetMonth()-1, 30, 0, 0, 0);
		}
		else {
			dt = COleDateTime(dtToday.GetYear(), dtToday.GetMonth()-1, dtToday.GetDay(), 0, 0, 0);
		}
		break;

	case dfoLastOfLastMonth:
		dt = COleDateTime(dtToday.GetYear(), dtToday.GetMonth(), 1, 0, 0, 0);
		dt -= COleDateTimeSpan(1,0,0,0);
		break;

	case dfoFirstOfLastYear:
		dt = COleDateTime(dtToday.GetYear()-1,1,1,0,0,0);
		break;

	case dfoOneYearAgo:
		if(dtToday.GetMonth() == 2 && dtToday.GetDay() == 29) {
			dt = COleDateTime(dtToday.GetYear()-1,2,28,0,0,0);
		}
		else {
			dt = COleDateTime(dtToday.GetYear()-1,dtToday.GetMonth(),dtToday.GetDay(),0,0,0);
		}
		break;

	case dfoLastOfLastYear:
		dt = COleDateTime(dtToday.GetYear()-1,12,31,0,0,0);
		break;
	}

	return dt;
}

void CExportDlg::OnSelChangedStoredExportList(long nNewSel) 
{
	try
	{
		HandleSelChangedStoredExportList(nNewSel, TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CExportDlg::HandleSelChangedStoredExportList(long nNewSel, BOOL bReloadDateFilters)
{
	CWaitCursor pWait;

	try {

		GetDlgItem(IDC_EDIT_EXPORT)->EnableWindow(nNewSel != -1);
		GetDlgItem(IDC_DELETE_EXPORT)->EnableWindow(nNewSel != -1);

		//Now, based on the record and filter types of the selected export, show and hide windows.
		if(nNewSel == -1) {
			//Just disable everything.
			GetDlgItem(IDC_AVAILABLE_RECORDS)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADD_ONE_RECORD)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADD_ALL_RECORDS)->EnableWindow(FALSE);
			GetDlgItem(IDC_REMOVE_ONE_RECORD)->EnableWindow(FALSE);
			GetDlgItem(IDC_REMOVE_ALL_RECORDS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECORDS_TO_BE_EXPORTED)->EnableWindow(FALSE);
			GetDlgItem(IDC_EXPORT_DATE_FILTER_OPTIONS)->EnableWindow(FALSE);
			GetDlgItem(IDC_EXPORT_DATE_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_EXPORT_DATE_TO)->EnableWindow(FALSE);
			GetDlgItem(IDC_EXPORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_MANUAL_SORT_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_MANUAL_SORT_DOWN)->EnableWindow(FALSE);
			m_pAvail->Clear();
			m_pSelect->Clear();
		}
		else {
			//Enable everything.
			GetDlgItem(IDC_AVAILABLE_RECORDS)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADD_ONE_RECORD)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADD_ALL_RECORDS)->EnableWindow(TRUE);
			GetDlgItem(IDC_REMOVE_ONE_RECORD)->EnableWindow(TRUE);
			GetDlgItem(IDC_REMOVE_ALL_RECORDS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECORDS_TO_BE_EXPORTED)->EnableWindow(TRUE);
			GetDlgItem(IDC_EXPORT_DATE_FILTER_OPTIONS)->EnableWindow(TRUE);
			GetDlgItem(IDC_EXPORT_DATE_FROM)->EnableWindow(TRUE);
			GetDlgItem(IDC_EXPORT_DATE_TO)->EnableWindow(TRUE);

			ExportRecordType ert = (ExportRecordType)VarLong(m_pStoredExportList->GetValue(nNewSel, secBasedOn));

			switch(ert) {
			case ertPatients:
				{
					m_pAvail->GetColumn(0)->FieldName = _bstr_t("PersonT.ID");
					m_pSelect->GetColumn(0)->FieldName = _bstr_t("PersonT.ID");
					m_pAvail->GetColumn(2)->FieldName = _bstr_t("PatientsT.UserDefinedID");
					m_pSelect->GetColumn(2)->FieldName = _bstr_t("PatientsT.UserDefinedID");
					m_pAvail->GetColumn(2)->ColumnTitle = _bstr_t("ID");
					m_pSelect->GetColumn(2)->ColumnTitle = _bstr_t("ID");
					m_pAvail->GetColumn(2)->FieldType = cftTextSingleLine;
					m_pSelect->GetColumn(2)->FieldType = cftTextSingleLine;
					//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
					//If the filter option is to filter on new patients, we need to determine if we're looking at the input date or the modified date
					ExportFilterOption efo = (ExportFilterOption)VarLong(m_pStoredExportList->GetValue(nNewSel,secFilterType));
					if(efo == efoAllNewModified)
					{
						//we're dealing with the patient modified date
						m_pAvail->GetColumn(3)->FieldName = _bstr_t("PatientsT.ModifiedDate");
						m_pSelect->GetColumn(3)->FieldName = _bstr_t("PatientsT.ModifiedDate");
						//change the column title for modified patients
						m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Modified");
					}
					else
					{
						//we're dealing with the patient input date
						m_pAvail->GetColumn(3)->FieldName = _bstr_t("PersonT.InputDate");
						m_pSelect->GetColumn(3)->FieldName = _bstr_t("PersonT.InputDate");
						//change the column title for created patients
						m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Created");
					}

					m_pAvail->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID");
					m_pSelect->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID");
				}
				break;
			case ertAppointments:
				{
					m_pAvail->GetColumn(0)->FieldName = _bstr_t("AppointmentsT.ID");
					m_pSelect->GetColumn(0)->FieldName = _bstr_t("AppointmentsT.ID");
					m_pAvail->GetColumn(2)->FieldName = _bstr_t("AppointmentsT.StartTime");
					m_pSelect->GetColumn(2)->FieldName = _bstr_t("AppointmentsT.StartTime");
					m_pAvail->GetColumn(2)->ColumnTitle = _bstr_t("Start Time");
					m_pSelect->GetColumn(2)->ColumnTitle = _bstr_t("Start Time");
					m_pAvail->GetColumn(2)->FieldType = cftDateAndTime;
					m_pSelect->GetColumn(2)->FieldType = cftDateAndTime;
					//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
					//If the filter option is to filter on new appointments, we need to determine if we're looking at the created date or the modified date
					ExportFilterOption efo = (ExportFilterOption)VarLong(m_pStoredExportList->GetValue(nNewSel,secFilterType));
					if(efo == efoAllNewModified)
					{
						//we're dealing with the appointment modified date
						m_pAvail->GetColumn(3)->FieldName = _bstr_t("AppointmentsT.ModifiedDate");
						m_pSelect->GetColumn(3)->FieldName = _bstr_t("AppointmentsT.ModifiedDate");
						//change the column title for modified appointments
						m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Modified");
					}
					else
					{
						//we're dealing with the appointment created date
						m_pAvail->GetColumn(3)->FieldName = _bstr_t("AppointmentsT.CreatedDate");
						m_pSelect->GetColumn(3)->FieldName = _bstr_t("AppointmentsT.CreatedDate");
						//change the column title for created appointments
						m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Created");
					}
					
					m_pAvail->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN AppointmentsT WITH(NOLOCK) ON PersonT.ID = AppointmentsT.PatientID");
					m_pSelect->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN AppointmentsT WITH(NOLOCK) ON PersonT.ID = AppointmentsT.PatientID");
				}
				break;
			case ertCharges:
				{
					m_pAvail->GetColumn(0)->FieldName = _bstr_t("LineChargeT.ID");
					m_pSelect->GetColumn(0)->FieldName = _bstr_t("LineChargeT.ID");
					m_pAvail->GetColumn(2)->FieldName = _bstr_t("LineChargeT.Description");
					m_pSelect->GetColumn(2)->FieldName = _bstr_t("LineChargeT.Description");
					m_pAvail->GetColumn(2)->ColumnTitle = _bstr_t("Description");
					m_pSelect->GetColumn(2)->ColumnTitle = _bstr_t("Description");
					m_pAvail->GetColumn(2)->FieldType = cftTextSingleLine;
					m_pSelect->GetColumn(2)->FieldType = cftTextSingleLine;
					m_pAvail->GetColumn(3)->FieldName = _bstr_t("LineChargeT.InputDate");
					m_pSelect->GetColumn(3)->FieldName = _bstr_t("LineChargeT.InputDate");
					//m.hancock PLID 17422 9/2/2005 - change the column title for created payments
					m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Created");
					m_pAvail->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN LineItemT LineChargeT WITH(NOLOCK) ON PersonT.ID = LineChargeT.PatientID");
					m_pSelect->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN LineItemT LineChargeT WITH(NOLOCK) ON PersonT.ID = LineChargeT.PatientID");
				}
				break;
			case ertPayments:
				{
					m_pAvail->GetColumn(0)->FieldName = _bstr_t("LinePayT.ID");
					m_pSelect->GetColumn(0)->FieldName = _bstr_t("LinePayT.ID");
					m_pAvail->GetColumn(2)->FieldName = _bstr_t("LinePayT.Description");
					m_pSelect->GetColumn(2)->FieldName = _bstr_t("LinePayT.Description");
					m_pAvail->GetColumn(2)->ColumnTitle = _bstr_t("Description");
					m_pSelect->GetColumn(2)->ColumnTitle = _bstr_t("Description");
					m_pAvail->GetColumn(2)->FieldType = cftTextSingleLine;
					m_pSelect->GetColumn(2)->FieldType = cftTextSingleLine;
					m_pAvail->GetColumn(3)->FieldName = _bstr_t("LinePayT.InputDate");
					m_pSelect->GetColumn(3)->FieldName = _bstr_t("LinePayT.InputDate");
					//m.hancock PLID 17422 9/2/2005 - change the column title for created payments
					m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Created");
					m_pAvail->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN LineItemT LinePayT WITH(NOLOCK) ON PersonT.ID = LinePayT.PatientID");
					m_pSelect->FromClause = _bstr_t("PersonT WITH(NOLOCK) INNER JOIN LineItemT LinePayT WITH(NOLOCK) ON PersonT.ID = LinePayT.PatientID");
				}
				break;
			case ertEMNs:
				{
					m_pAvail->GetColumn(0)->FieldName = _bstr_t("EmrMasterT.ID");
					m_pSelect->GetColumn(0)->FieldName = _bstr_t("EmrMasterT.ID");
					m_pAvail->GetColumn(2)->FieldName = _bstr_t("EmrMasterT.Description");
					m_pSelect->GetColumn(2)->FieldName = _bstr_t("EmrMasterT.Description");
					m_pAvail->GetColumn(2)->ColumnTitle = _bstr_t("Description");
					m_pSelect->GetColumn(2)->ColumnTitle = _bstr_t("Description");
					//m.hancock - 5/5/2006 - PLID 20409 - EMN names should wrap text when displayed
					//Changed from cftTextSingleLine to cftTextWordWrap
					m_pAvail->GetColumn(2)->FieldType = cftTextWordWrap;
					m_pSelect->GetColumn(2)->FieldType = cftTextWordWrap;
					m_pAvail->GetColumn(3)->FieldName = _bstr_t("EmrMasterT.InputDate");
					m_pSelect->GetColumn(3)->FieldName = _bstr_t("EmrMasterT.InputDate");
					//m.hancock PLID 17422 9/2/2005 - change the column title for created payments
					m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Created");
					//TES 6/5/2007 - PLID 26226 - If the export uses an EMR-based letter writing filter, the WHERE clause
					// will reference EmrGroupsT, so make sure it's in the FROM clause as well.
					// (z.manning 2011-05-24 09:28) - PLID 33114 - Added join of EmnTabChartsLinkT
					_bstr_t bstrEmnFrom = 
						"PersonT WITH(NOLOCK) \r\n"
						"INNER JOIN (SELECT * FROM EmrMasterT WITH(NOLOCK) WHERE Deleted = 0) AS EmrMasterT ON PersonT.ID = EmrMasterT.PatientID \r\n"
						"INNER JOIN EmrGroupsT WITH(NOLOCK) ON EmrMasterT.EmrGroupID = EmrGroupsT.ID \r\n"
						"LEFT JOIN EmnTabChartsLinkT WITH(NOLOCK) ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n";
					m_pAvail->FromClause = bstrEmnFrom;
					m_pSelect->FromClause = bstrEmnFrom;
				}
				break;

			case ertHistory: // (z.manning 2009-12-11 10:01) - PLID 36519 - Added history export
				{
					// (z.manning 2009-12-11 10:46) - PLID 36519 - Manual sorting is not an option for the
					// history export so no need to worry about the available list.
					m_pSelect->GetColumn(0)->FieldName = _bstr_t("MailSent.MailID");
					m_pSelect->GetColumn(2)->FieldName = _bstr_t("MailSentNotesT.Note");
					m_pSelect->GetColumn(2)->ColumnTitle = _bstr_t("Document");
					m_pSelect->GetColumn(2)->FieldType = cftTextSingleLine;
					m_pSelect->GetColumn(3)->FieldName = _bstr_t("MailSent.Date");
					m_pSelect->GetColumn(3)->ColumnTitle = _bstr_t("Attach Date");
					m_pSelect->FromClause = _bstr_t(
						"MailSent WITH(NOLOCK) \r\n"
						"LEFT JOIN MailSentNotesT WITH(NOLOCK) ON MailSent.MailID = MailSentNotesT.MailID \r\n"
						"LEFT JOIN PersonT WITH(NOLOCK) ON MailSent.PersonID = PersonT.ID \r\n"
						);
				}
				break;

			default:
				ASSERT(FALSE);
				break;
			}

			ExportFilterOption efo = (ExportFilterOption)VarLong(m_pStoredExportList->GetValue(nNewSel,secFilterType));
			long nFilterFlags = VarLong(m_pStoredExportList->GetValue(nNewSel, secFilterFlags));
			//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
			//I changed this selection structure because I did not want to duplicate the code for case efoAllNew
			if(efo == efoAllNew || efo == efoAllNewModified)
			{
				GetDlgItem(IDC_AVAILABLE_RECORDS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADD_ONE_RECORD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADD_ALL_RECORDS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REMOVE_ONE_RECORD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REMOVE_ALL_RECORDS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EXPORT_DATE_FILTER_OPTIONS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EXPORT_DATE_FROM)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EXPORT_DATE_TO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MANUAL_SORT_UP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MANUAL_SORT_DOWN)->ShowWindow(SW_HIDE);
				m_pSelect->ReadOnly = VARIANT_TRUE;

				CString strLastExport = FormatDateTimeForSql(VarDateTime(m_pStoredExportList->GetValue(nNewSel,secLastExportDate),COleDateTime(1900,1,1,0,0,0)));
				//TES 6/5/2007 - PLID 26125 - Make sure to include the base WHERE clause in any filtering.
				CString strBaseWhere = GetBaseWhereClause(ert);
				switch(ert) {
				case ertPatients:
					{
						CString strWhere;
						// (a.walling 2008-03-06 15:46) - PLID 29226 - We used to say InputDate AND ModifiedDate, which resulted in selecting only
						// records that were BOTH input AND modified after the last export.						
						
						//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
						if(efo == efoAllNewModified) {
							strWhere.Format("%s AND (PersonT.InputDate > '%s' OR PatientsT.ModifiedDate > '%s')", strBaseWhere, strLastExport, strLastExport);
						} else {
							strWhere.Format("%s AND PersonT.InputDate > '%s'", strBaseWhere, strLastExport);
						}

						m_pSelect->WhereClause = _bstr_t(strWhere);
						m_pSelect->Requery();
						//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
						//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
						//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
						//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
					}
					break;
				case ertAppointments:
					{
						CString strWhere;
						// (a.walling 2008-03-06 15:46) - PLID 29226 - We used to say InputDate AND ModifiedDate, which resulted in selecting only
						// records that were BOTH input AND modified after the last export.	

						//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
						if(efo == efoAllNewModified) {
							strWhere.Format("%s AND (AppointmentsT.CreatedDate > '%s' OR AppointmentsT.ModifiedDate > '%s')", strBaseWhere, strLastExport, strLastExport);
						} else {
							strWhere.Format("%s AND AppointmentsT.CreatedDate > '%s'", strBaseWhere, strLastExport);
						}

						m_pSelect->WhereClause = _bstr_t(strWhere);
						m_pSelect->Requery();
						//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
						//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
						//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
						//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
					}
					break;
				case ertCharges:
					{
						CString strWhere;
						strWhere.Format("%s AND LineChargeT.InputDate > '%s'", strBaseWhere, strLastExport);
						m_pSelect->WhereClause = _bstr_t(strWhere);
						m_pSelect->Requery();
						//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
						//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
						//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
						//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
					}
					break;
				case ertPayments:
					{
						CString strWhere;
						strWhere.Format("%s AND LinePayT.InputDate > '%s'", strBaseWhere, strLastExport);
						m_pSelect->WhereClause = _bstr_t(strWhere);
						m_pSelect->Requery();
						//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
						//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
						//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
						//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
					}
					break;
				case ertEMNs:
					{
						CString strWhere;
						strWhere.Format("%s AND %s AND EMRMasterT.InputDate > '%s'", GetCurrentExtraEmnFilter(), strBaseWhere, strLastExport);
						m_pSelect->WhereClause = _bstr_t(strWhere);
						m_pSelect->Requery();
						//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
						//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
						//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
						//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
					}
					break;
				case ertHistory: // (z.manning 2009-12-11 10:19) - PLID 36519 - History export
					{
						CString strLocalBaseWhere = strBaseWhere;
						if(!strLocalBaseWhere.IsEmpty()) {
							strLocalBaseWhere += " AND ";
						}
						m_pSelect->PutWhereClause(_bstr_t(FormatString(
							"%s MailSent.Date > '%s' AND %s"
							, strLocalBaseWhere, strLastExport, GetCurrentExtraHistoryFilter())));
						m_pSelect->Requery();
					}
					break;
				default:
					ASSERT(FALSE);
					break;
				}

				GetDlgItem(IDC_EXPORT)->EnableWindow(TRUE);
			}

			else
			{
				CString strDateLwWhere;
				switch(efo)
				{
					case efoDateOrLwFilter:
						{
							GetDlgItem(IDC_AVAILABLE_RECORDS)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_ADD_ONE_RECORD)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_ADD_ALL_RECORDS)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_REMOVE_ONE_RECORD)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_REMOVE_ALL_RECORDS)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_MANUAL_SORT_UP)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_MANUAL_SORT_DOWN)->ShowWindow(SW_HIDE);
							m_pSelect->ReadOnly = VARIANT_TRUE;
							if(nFilterFlags & effDate)
							{
								GetDlgItem(IDC_EXPORT_DATE_FILTER_OPTIONS)->ShowWindow(SW_SHOW);
								GetDlgItem(IDC_EXPORT_DATE_FROM)->ShowWindow(SW_SHOW);
								GetDlgItem(IDC_EXPORT_DATE_TO)->ShowWindow(SW_SHOW);

								// (z.manning 2009-12-15 14:59) - PLID 36576 - Only do this sometimes
								if(bReloadDateFilters)
								{
									//Fill in the date filters drop down.
									m_pDateFilters->Clear();
									switch(ert)
									{
									case ertPatients:
										{
											IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdFirstContactDate);
											pRow->PutValue(1, _bstr_t("First Contact Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdHasAppointmentDate);
											pRow->PutValue(1, _bstr_t("Has Appointment With Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdNextAppointmentDate);
											pRow->PutValue(1, _bstr_t("Next Appointment Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdLastAppointmentDate);
											pRow->PutValue(1, _bstr_t("Last Appointment Date"));
											m_pDateFilters->AddRow(pRow);
										}
										break;
										
									case ertAppointments:
										{
											IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdAppointmentDate);
											pRow->PutValue(1, _bstr_t("Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdAppointmentInputDate);
											pRow->PutValue(1, _bstr_t("Input Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdFirstContactDate);
											pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
											m_pDateFilters->AddRow(pRow);
										}
										break;

									case ertCharges:
										{
											IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdServiceDate);
											pRow->PutValue(1, _bstr_t("Service Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdInputDate);
											pRow->PutValue(1, _bstr_t("Input Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdBillDate);
											pRow->PutValue(1, _bstr_t("Bill Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdFirstContactDate);
											pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdHasAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient Has Appointment With Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdNextAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient's Next Appointment Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdLastAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient's Last Appointment Date"));
											m_pDateFilters->AddRow(pRow);
										}
										break;

									case ertPayments:
										{
											IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdServiceDate);
											pRow->PutValue(1, _bstr_t("Service Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdInputDate);
											pRow->PutValue(1, _bstr_t("Input Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdFirstContactDate);
											pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdHasAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient Has Appointment With Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdNextAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient's Next Appointment Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdLastAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient's Last Appointment Date"));
											m_pDateFilters->AddRow(pRow);
										}
										break;

									case ertEMNs:
										{
											IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdServiceDate);
											pRow->PutValue(1, _bstr_t("Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdInputDate);
											pRow->PutValue(1, _bstr_t("Input Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdFirstContactDate);
											pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdHasAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient Has Appointment With Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdNextAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient's Next Appointment Date"));
											m_pDateFilters->AddRow(pRow);
											pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdLastAppointmentDate);
											pRow->PutValue(1, _bstr_t("Patient's Last Appointment Date"));
											m_pDateFilters->AddRow(pRow);
										}
										break;

									case ertHistory: // (z.manning 2009-12-11 11:13) - PLID 36519
										{
											IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
											pRow->PutValue(0, (long)fdAttachDate);
											pRow->PutValue(1, _bstr_t("Attach Date"));
											m_pDateFilters->AddRow(pRow);
										}
										break;
										
									default:
										{
											ASSERT(FALSE);
										}
										break;
									
									}

									{
										//Gather more information about the date filter.
										_RecordsetPtr rsExport = CreateRecordset("SELECT FilterDateType, FromDateType, FromDateOffset, FromDate, "
											"ToDateType, ToDateOffset, ToDate FROM ExportT WHERE ID = %li", VarLong(m_pStoredExportList->GetValue(nNewSel,secID)));
										ASSERT(!rsExport->eof);
										FilterableDate fd = (FilterableDate)AdoFldLong(rsExport, "FilterDateType");
										m_pDateFilters->SetSelByColumn(0, (long)fd);

										DateFilterOption dfoFrom = (DateFilterOption)AdoFldLong(rsExport, "FromDateType");
										COleDateTime dtFrom;
										if(dfoFrom == dfoCustom) {
											dtFrom = AdoFldDateTime(rsExport, "FromDate");
										}
										else {
											dtFrom = GetSpecialDate(dfoFrom) + COleDateTimeSpan(AdoFldLong(rsExport, "FromDateOffset"),0,0,0);
										}
										DateFilterOption dfoTo = (DateFilterOption)AdoFldLong(rsExport, "ToDateType");
										COleDateTime dtTo;
										if(dfoTo == dfoCustom) {
											dtTo = AdoFldDateTime(rsExport, "ToDate");
										}
										else {
											dtTo = GetSpecialDate(dfoTo) + COleDateTimeSpan(AdoFldLong(rsExport, "ToDateOffset"),0,0,0);
										}

										if(dtFrom.m_status == COleDateTime::invalid)
											dtFrom = COleDateTime::GetCurrentTime();
										m_dtFrom.SetValue(_variant_t(dtFrom));

										if(dtTo.m_status == COleDateTime::invalid)
											dtTo = COleDateTime::GetCurrentTime();
										m_dtTo.SetValue(_variant_t(dtTo));
									}
								}

								strDateLwWhere = GetDateFilterWhereClause();

								GetDlgItem(IDC_EXPORT)->EnableWindow(TRUE);
							}
							else {
								GetDlgItem(IDC_EXPORT_DATE_FILTER_OPTIONS)->ShowWindow(SW_HIDE);
								GetDlgItem(IDC_EXPORT_DATE_FROM)->ShowWindow(SW_HIDE);
								GetDlgItem(IDC_EXPORT_DATE_TO)->ShowWindow(SW_HIDE);

								strDateLwWhere = GetBaseWhereClause(ert);
							}

							if(nFilterFlags & effLetterWriting)
							{

								//Get the filter string
								long nFilterID = VarLong(GetTableField("ExportT", "LetterWritingFilterID", "ID", VarLong(m_pStoredExportList->GetValue(nNewSel,secID))));
								_RecordsetPtr rsFilter = CreateRecordset("SELECT Type, Filter FROM FiltersT WHERE ID = %li", nFilterID);
								long nFilterBasedOn = AdoFldLong(rsFilter, "Type");
								CString strFilterString = AdoFldString(rsFilter, "Filter");
								//Convert the string we have to an SQL
								CString strFilterWhere, strFilterFrom;
								if(!CFilter::ConvertFilterStringToClause(nFilterID, strFilterString, nFilterBasedOn, &strFilterWhere, &strFilterFrom)) {
									MsgBox("Record list could not be generated because the export uses an invalid filter.");
									GetDlgItem(IDC_EXPORT)->EnableWindow(FALSE);
									return;
								}
								switch(ert)
								{
								case ertPatients:
									{
										ASSERT(nFilterBasedOn == fboPerson);
										strDateLwWhere = FormatString("%s AND "
											"PersonT.ID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
											strDateLwWhere, strFilterFrom, strFilterWhere);
										//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
										//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
										//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
										//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
									}
									break;
								case ertAppointments:
									{
										ASSERT(nFilterBasedOn == fboPerson || nFilterBasedOn == fboAppointment);
										if(nFilterBasedOn == fboPerson) {
											strDateLwWhere = FormatString("%s AND AppointmentsT.PatientID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
												strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										else if(nFilterBasedOn == fboAppointment) {
											strDateLwWhere = FormatString("%s AND AppointmentsT.ID IN (SELECT AppointmentsT.ID AS AppointmentID FROM %s WHERE %s GROUP BY AppointmentsT.ID)",
												strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
										//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
										//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
										//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
									}
									break;
								case ertCharges:
									{
										ASSERT(nFilterBasedOn == fboPerson);
										strDateLwWhere = FormatString("%s AND "
											"PersonT.ID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
											strDateLwWhere, strFilterFrom, strFilterWhere);
										//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
										//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
										//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
										//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
									}
									break;
								case ertPayments:
									{
										ASSERT(nFilterBasedOn == fboPerson || nFilterBasedOn == fboPayment);
										if(nFilterBasedOn == fboPerson) {
											strDateLwWhere = FormatString("%s AND "
												"PersonT.ID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
											strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										else {
											strDateLwWhere = FormatString("%s AND LinePayT.ID IN (SELECT PaymentsT.ID AS PayID FROM %s WHERE %s)",
												strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
										//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
										//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
										//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
									}
									break;
								case ertEMNs:
										{
										ASSERT(nFilterBasedOn == fboPerson || nFilterBasedOn == fboEMN || nFilterBasedOn == fboEMR);
										if(nFilterBasedOn == fboPerson) {
											strDateLwWhere = FormatString("%s AND %s AND PersonT.ID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
												GetCurrentExtraEmnFilter(), strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										else if(nFilterBasedOn == fboEMN) {
											strDateLwWhere = FormatString("%s AND %s AND EMRMasterT.ID IN (SELECT EMRMasterT.ID AS EMNID FROM %s WHERE %s)",
												GetCurrentExtraEmnFilter(), strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										else {
											strDateLwWhere = FormatString("%s AND %s AND EMRGroupsT.Deleted = 0 AND EMRGroupsT.ID IN (SELECT EmrGroupsT.ID AS EMRID FROM %s WHERE %s)",
												GetCurrentExtraEmnFilter(), strDateLwWhere, strFilterFrom, strFilterWhere);
										}
										//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
										//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
										//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
										//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);
									}
									break;

								case ertHistory: // (z.manning 2009-12-11 11:15) - PLID 36519
									{
										ASSERT(nFilterBasedOn == fboPerson);
										strDateLwWhere = FormatString("%s AND %s AND PersonT.ID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)"
											, GetCurrentExtraHistoryFilter(), strDateLwWhere, strFilterFrom, strFilterWhere);
									}
									break;

								default:
									{
										ASSERT(FALSE);
									}
									break;
								}
							}
							m_pSelect->WhereClause = _bstr_t(strDateLwWhere);
							m_pSelect->Requery();
							GetDlgItem(IDC_EXPORT)->EnableWindow(TRUE);
						}
						break;

					case efoManual:
						{
							BOOL bManualSort = VarBool(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secManualSort));
							GetDlgItem(IDC_AVAILABLE_RECORDS)->ShowWindow(SW_SHOW);
							GetDlgItem(IDC_ADD_ONE_RECORD)->ShowWindow(SW_SHOW);
							GetDlgItem(IDC_ADD_ALL_RECORDS)->ShowWindow(SW_SHOW);
							GetDlgItem(IDC_REMOVE_ONE_RECORD)->ShowWindow(SW_SHOW);
							GetDlgItem(IDC_REMOVE_ALL_RECORDS)->ShowWindow(SW_SHOW);
							GetDlgItem(IDC_EXPORT_DATE_FILTER_OPTIONS)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_EXPORT_DATE_FROM)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_EXPORT_DATE_TO)->ShowWindow(SW_HIDE);
							GetDlgItem(IDC_MANUAL_SORT_UP)->ShowWindow(bManualSort?SW_SHOW:SW_HIDE);
							GetDlgItem(IDC_MANUAL_SORT_DOWN)->ShowWindow(bManualSort?SW_SHOW:SW_HIDE);
							m_pSelect->ReadOnly = VARIANT_FALSE;

							m_pSelect->Clear();
							m_pAvail->Clear();

							//TES 6/5/2007 - PLID 26125 - Make sure to include the base WHERE clause in any filtering.
							CString strBaseWhere = GetBaseWhereClause(ert);
							switch(ert) {
							case ertPatients:
								{
									// (b.cardillo 2007-02-16 14:48) - PLID 24791 - Just like letter writing has done for ages, we now also 
									// disable the "add all" button while the requery is going on.  When it's done (in our RequeryFinished event 
									// handler) we enable it again.  We can't allow the user to add all rows while the requery is still loading 
									// them because only the ones that have loaded so far would be added, which makes no sense to the user.
									EnableDlgItem(IDC_ADD_ALL_RECORDS, FALSE);

									m_pAvail->WhereClause = _bstr_t(strBaseWhere);
									m_pAvail->Requery();
									//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
									//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
									//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
									//m_pAvail->WaitForRequery(dlPatienceLevelWaitIndefinitely);
								}
								break;
							case ertAppointments:
								{
									// (b.cardillo 2007-02-16 14:48) - PLID 24791 - Just like letter writing has done for ages, we now also 
									// disable the "add all" button while the requery is going on.  When it's done (in our RequeryFinished event 
									// handler) we enable it again.  We can't allow the user to add all rows while the requery is still loading 
									// them because only the ones that have loaded so far would be added, which makes no sense to the user.
									EnableDlgItem(IDC_ADD_ALL_RECORDS, FALSE);

									m_pAvail->WhereClause = _bstr_t(strBaseWhere);
									m_pAvail->Requery();
									//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
									//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
									//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
									//m_pAvail->WaitForRequery(dlPatienceLevelWaitIndefinitely);
								}
								break;
							case ertCharges:
								{
									// (b.cardillo 2007-02-16 14:48) - PLID 24791 - Just like letter writing has done for ages, we now also 
									// disable the "add all" button while the requery is going on.  When it's done (in our RequeryFinished event 
									// handler) we enable it again.  We can't allow the user to add all rows while the requery is still loading 
									// them because only the ones that have loaded so far would be added, which makes no sense to the user.
									EnableDlgItem(IDC_ADD_ALL_RECORDS, FALSE);

									m_pAvail->WhereClause = _bstr_t(strBaseWhere);
									m_pAvail->Requery();
									//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
									//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
									//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
									//m_pAvail->WaitForRequery(dlPatienceLevelWaitIndefinitely);
								}
								break;
							case ertPayments:
								{
									// (b.cardillo 2007-02-16 14:48) - PLID 24791 - Just like letter writing has done for ages, we now also 
									// disable the "add all" button while the requery is going on.  When it's done (in our RequeryFinished event 
									// handler) we enable it again.  We can't allow the user to add all rows while the requery is still loading 
									// them because only the ones that have loaded so far would be added, which makes no sense to the user.
									EnableDlgItem(IDC_ADD_ALL_RECORDS, FALSE);

									m_pAvail->WhereClause = _bstr_t(strBaseWhere);
									m_pAvail->Requery();
									//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
									//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
									//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
									//m_pAvail->WaitForRequery(dlPatienceLevelWaitIndefinitely);
								}
								break;
							case ertEMNs:
								{
									// (b.cardillo 2007-02-16 14:48) - PLID 24791 - Just like letter writing has done for ages, we now also 
									// disable the "add all" button while the requery is going on.  When it's done (in our RequeryFinished event 
									// handler) we enable it again.  We can't allow the user to add all rows while the requery is still loading 
									// them because only the ones that have loaded so far would be added, which makes no sense to the user.
									EnableDlgItem(IDC_ADD_ALL_RECORDS, FALSE);

									m_pAvail->WhereClause = _bstr_t(GetCurrentExtraEmnFilter() + " AND " + strBaseWhere);
									m_pAvail->Requery();
									//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
									//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
									//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
									//m_pAvail->WaitForRequery(dlPatienceLevelWaitIndefinitely);
								}
								break;
							case ertHistory:
								// (z.manning 2009-12-11 11:38) - PLID 36519 - History does not support manual exports
								ASSERT(FALSE);
								break;
							default:
								ASSERT(FALSE);
								break;
							}
							GetDlgItem(IDC_EXPORT)->EnableWindow(TRUE);
						}
						break;

					default:
						ASSERT(FALSE);
						break;
					}
				}
		}
	}NxCatchAll(__FUNCTION__);
}

void CExportDlg::OnSelChosenExportDateFilterOptions(long nRow) 
{
	if(nRow == -1) {
		m_pDateFilters->CurSel = 0;
		OnSelChosenExportDateFilterOptions(0);
	}
	else {
		HandleSelChangedStoredExportList(m_pStoredExportList->GetCurSel(), FALSE);
	}
}

void CExportDlg::OnChangeExportDateFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	HandleSelChangedStoredExportList(m_pStoredExportList->GetCurSel(), FALSE);

	*pResult = 0;
}

void CExportDlg::OnChangeExportDateTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	HandleSelChangedStoredExportList(m_pStoredExportList->GetCurSel(), FALSE);

	*pResult = 0;
}

CString CExportDlg::GetDateFilterWhereClause()
{
	try {
		if(m_pStoredExportList->CurSel == -1) {
			return "";
		}
		COleDateTime dtFrom = VarDateTime(m_dtFrom.GetValue());
		COleDateTime dtTo = VarDateTime(m_dtTo.GetValue()) + COleDateTimeSpan(1,0,0,0);

		ExportRecordType ert = (ExportRecordType)VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secBasedOn));
		FilterableDate fd = (FilterableDate)VarLong(m_pDateFilters->GetValue(m_pDateFilters->CurSel,0));
		CString strWhere;
		//TES 6/5/2007 - PLID 26125 - Make sure to include the base WHERE clause in any filtering.
		CString strBaseWhere = GetBaseWhereClause(ert);
		switch(ert) {
		case ertPatients:
			{
				switch(fd) {
				case fdFirstContactDate:
					strWhere.Format("%s AND PersonT.FirstContactDate >= '%s' AND PersonT.FirstContactDate < '%s'", 
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdHasAppointmentDate:
					strWhere.Format("%s AND PersonT.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
						"AND Date < '%s')", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdNextAppointmentDate:
					strWhere.Format("%s AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdLastAppointmentDate:
					strWhere.Format("%s AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
			break;
		
		case ertAppointments:
			{
				//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
				//ExportFilterOption efo = (ExportFilterOption)VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secFilterType));

				switch(fd) {
				case fdAppointmentDate:
					strWhere.Format("%s AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date < '%s'", 
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdAppointmentInputDate:
						strWhere.Format("%s AND AppointmentsT.CreatedDate >= '%s' AND AppointmentsT.CreatedDate < '%s'", 
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdFirstContactDate:
					strWhere.Format("%s AND PatientID <> -25 AND PersonT.FirstContactDate >= '%s' AND PersonT.FirstContactDate < '%s'", 
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
			break;
	
		case ertCharges:
			{
				switch(fd) {
				case fdServiceDate:
					strWhere.Format("%s AND "
						"LineChargeT.Date >= '%s' AND LineChargeT.Date < '%s'",
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdInputDate:
					strWhere.Format("%s AND "
						"LineChargeT.InputDate >= '%s' AND LineChargeT.InputDate < '%s'",
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdBillDate:
					strWhere.Format("%s AND "
						"LineChargeT.ID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"WHERE BillsT.date >= '%s' AND BillsT.Date < '%s')",
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdFirstContactDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND PersonT.FirstContactDate >= '%s' AND PersonT.FirstContactDate < '%s'", 
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdHasAppointmentDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND PersonT.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
						"AND Date < '%s')", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdNextAppointmentDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdLastAppointmentDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
			break;
		
		case ertPayments:
			{
				switch(fd) {
				case fdServiceDate:
					strWhere.Format("%s AND "
						"LinePayT.Date >= '%s' AND LinePayT.Date < '%s'",
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdInputDate:
					strWhere.Format("%s AND "
						"LinePayT.InputDate >= '%s' AND LinePayT.InputDate < '%s'",
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdFirstContactDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND PersonT.FirstContactDate >= '%s' AND PersonT.FirstContactDate < '%s'", 
						strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdHasAppointmentDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND PersonT.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
						"AND Date < '%s')", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdNextAppointmentDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdLastAppointmentDate:
					strWhere.Format("%s AND "
						"PatientID <> -25 AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
			break;
		case ertEMNs:
			{
				//TES 7/9/2007 - PLID 26226 - Went through and made sure that PatientIDs specified which table they should be pulled from.
				switch(fd) {
				case fdServiceDate:
					strWhere.Format("%s AND %s AND EmrMasterT.Date >= '%s' AND EmrMasterT.Date < '%s'",
						GetCurrentExtraEmnFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdInputDate:
					strWhere.Format("%s AND %s AND EmrMasterT.InputDate >= '%s' AND EmrMasterT.InputDate < '%s'",
						GetCurrentExtraEmnFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdFirstContactDate:
					strWhere.Format("%s AND %s AND EmrMasterT.PatientID <> -25 AND PersonT.FirstContactDate >= '%s' AND PersonT.FirstContactDate < '%s'", 
						GetCurrentExtraEmnFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdHasAppointmentDate:
					strWhere.Format("%s AND %s AND EmrMasterT.PatientID <> -25 AND PersonT.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
						"AND Date < '%s')", GetCurrentExtraEmnFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdNextAppointmentDate:
					strWhere.Format("%s AND %s AND EmrMasterT.PatientID <> -25 AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", GetCurrentExtraEmnFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				case fdLastAppointmentDate:
					strWhere.Format("%s AND %s AND EmrMasterT.PatientID <> -25 AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonT.ID AND "
						"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
						"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
						"OthrAppts.PatientID = PersonT.ID))", GetCurrentExtraEmnFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
			break;

		case ertHistory: // (z.manning 2009-12-11 11:40) - PLID 36519
			{
				switch(fd)
				{
				case fdAttachDate:
					strWhere.Format("%s AND %s AND MailSent.Date >= '%s' AND MailSent.Date < '%s'",
						GetCurrentExtraHistoryFilter(), strBaseWhere, FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
			break;
		}

		//DRT 6/2/2006 - PLID 20222 - t.schneider and I have discussed, and we are pretty certain through his memory
		//	and my testing that the only purpose of this WaitForRequery is to make sure they do not export before
		//	the list finishes.  I moved the WaitForRequery to the export button handling instead.
		//m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		return strWhere;

	}NxCatchAll("Error in CExportDlg::HandleNewDateFilter()");
	return "";
}

void CExportDlg::OnAddAllRecords() 
{
	m_pSelect->TakeAllRows(m_pAvail);
	GetDlgItem(IDC_ADD_ALL_RECORDS)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMOVE_ALL_RECORDS)->EnableWindow(m_pSelect->GetRowCount() > 0);
}

void CExportDlg::OnAddOneRecord() 
{
	m_pSelect->TakeCurrentRow(m_pAvail);
	// (b.cardillo 2007-02-19 11:29) - PLID 24791 - We also can't allow the "add all" button 
	// to be enabled if the available list is still requerying, so we require !m_pAvail->IsRequerying().
	GetDlgItem(IDC_ADD_ALL_RECORDS)->EnableWindow(m_pAvail->GetRowCount() > 0 && !m_pAvail->IsRequerying());
	GetDlgItem(IDC_REMOVE_ALL_RECORDS)->EnableWindow(m_pSelect->GetRowCount() > 0);
}

void CExportDlg::OnRemoveAllRecords() 
{
	m_pAvail->TakeAllRows(m_pSelect);
	// (b.cardillo 2007-02-19 11:29) - PLID 24791 - We also can't allow the "add all" button 
	// to be enabled if the available list is still requerying, so we require !m_pAvail->IsRequerying().
	GetDlgItem(IDC_ADD_ALL_RECORDS)->EnableWindow(m_pAvail->GetRowCount() > 0 && !m_pAvail->IsRequerying());
	GetDlgItem(IDC_REMOVE_ALL_RECORDS)->EnableWindow(FALSE);
}

void CExportDlg::OnRemoveOneRecord() 
{
	m_pAvail->TakeCurrentRow(m_pSelect);
	// (b.cardillo 2007-02-19 11:29) - PLID 24791 - We also can't allow the "add all" button 
	// to be enabled if the available list is still requerying, so we require !m_pAvail->IsRequerying().
	GetDlgItem(IDC_ADD_ALL_RECORDS)->EnableWindow(m_pAvail->GetRowCount() > 0 && !m_pAvail->IsRequerying());
	GetDlgItem(IDC_REMOVE_ALL_RECORDS)->EnableWindow(m_pSelect->GetRowCount() > 0);
}

void CExportDlg::OnSelChangedAvailableRecords(long nNewSel) 
{
	GetDlgItem(IDC_ADD_ONE_RECORD)->EnableWindow(nNewSel != -1);
}

void CExportDlg::OnDblClickCellAvailableRecords(long nRowIndex, short nColIndex) 
{
	OnAddOneRecord();
}

void CExportDlg::OnSelChangedRecordsToBeExported(long nNewSel) 
{
	GetDlgItem(IDC_REMOVE_ONE_RECORD)->EnableWindow(nNewSel != -1);
	GetDlgItem(IDC_MANUAL_SORT_UP)->EnableWindow(nNewSel != -1 && nNewSel != 0);
	GetDlgItem(IDC_MANUAL_SORT_DOWN)->EnableWindow(nNewSel != -1 && nNewSel != m_pSelect->GetRowCount()-1);
}

void CExportDlg::OnDblClickCellRecordsToBeExported(long nRowIndex, short nColIndex) 
{
	// (b.cardillo 2005-09-02 14:18) - PLID 17411 - Can't remove the record if m_pSelect is read-only
	if (m_pSelect->ReadOnly) {
		return;
	}

	OnRemoveOneRecord();
}

void CExportDlg::OnExport() 
{
	//OK, here goes.
	try {
		if(m_pStoredExportList->CurSel == -1) {
			MsgBox("Please select a stored export from the list.");
			return;
		}

		//DRT 6/2/2006 - PLID 20222 - Moved the WaitForRequery to here instead of when loading.
		if(m_pSelect->IsRequerying())
			m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		ExportRecordType ert = (ExportRecordType)VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secBasedOn));
		
		// (a.walling 2010-10-04 13:45) - PLID 40738 - Are patient exports restricted?
		bool bRestricted = false;
		if (ert == ertPatients && GetPatientExportRestrictions().Enabled(true)) {
			bRestricted = true;
			if (IDNO == MessageBox(FormatString("%s\r\n\r\nDo you want to continue?", GetPatientExportRestrictions().Description()), NULL, MB_ICONWARNING | MB_YESNO)) {
				return;
			}
		}
		
		long nExportID = VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secID));
		//Set up our recordset.

		CString strSelect, strFrom, strWhere;

		CArray<SelectedExportField,SelectedExportField> arFields;
		// (z.manning 2009-12-11 12:35) - PLID 36519 - We hard code the fields we need for history
		// since it's just a file export.
		if(ert != ertHistory)
		{
			//Let's load our array of fields from data once.
			//(e.lally 2008-07-28) PLID 30861 - Need to force the ORDER BY here so we are sure to be creating columns in the Export Order.
				//Inexplicably, this pulled the correct order *most* of the time and went 3 years without being noticed. I believe changing
				//the saved ordering of fields could cause this to be apparent.
			_RecordsetPtr rsFields = CreateRecordset("SELECT FieldID, DynamicID, Format FROM ExportFieldsT WHERE ExportID = %li ORDER BY ExportOrder ASC ", nExportID);
			while(!rsFields->eof) {				

				SelectedExportField sef;			
				sef.nID = AdoFldLong(rsFields, "FieldID");				

				// (a.walling 2010-10-04 13:45) - PLID 40738 - If restricted, allow only certain fields.
				if (bRestricted && GetPatientExportRestrictions().IsFieldRestricted(sef.nID)) {
					rsFields->MoveNext();
					continue;
				}

				CExportField ef = GetFieldByID(sef.nID);
				sef.nDynamicID = AdoFldLong(rsFields, "DynamicID", -1);
				sef.strFormat = AdoFldString(rsFields, "Format");
				sef.bHasAJoinOrFromClause = ef.bHasAJoinOrFromClause;

				if(sef.nDynamicID != -1 && ef.eftType == eftEmnItem) {
					if(IsRecordsetEmpty("SELECT Name FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID WHERE EmrInfoMasterT.ID = %li", sef.nDynamicID)) {
						//it has been deleted!
						rsFields->MoveNext();
						continue;
					}
				}

				arFields.Add(sef);
				rsFields->MoveNext();
			}
			rsFields->Close();
		}

		// (a.walling 2010-10-07 17:33) - PLID 40738 - Stop them if there are no fields (if they are restricted and only have black fields selected)
		//(e.lally 2011-03-29) PLID 42834 - Exclude History exports since the array will always be empty
		if (arFields.IsEmpty()) {
			if (bRestricted) {
				MessageBox("All exported fields are unavailable for a restricted patient export! Please contact NexTech Technical Support for assistance.", NULL, MB_ICONSTOP);
				return;
			} else if(ert != ertHistory){
				MessageBox("There are no fields available to export! Please contact NexTech Technical Support for assistance.", NULL, MB_ICONSTOP);
				return;
			}
		}

		// (j.jones 2006-03-22 12:06) - PLID 19384 - the * fields are complex and add more JOIN/FROM clauses
		// to the export, of which there is a maximum of 256. So warn if we have more thn 200 such fields.
		long nJoinFieldCount = 0;
		if(arFields.GetSize() > 200) {
			for(int i=0; i < arFields.GetSize(); i++)  {
				if(arFields.GetAt(i).bHasAJoinOrFromClause) {
					nJoinFieldCount++;
				}
			}

			if(nJoinFieldCount > 200) {
				CString str;
				str.Format("There is a limit of 200 export fields that have the * indicator.\n"
					"Please remove at least %li of these * fields before continuing.", nJoinFieldCount - 200);
				AfxMessageBox(str);
			}
		}

		// (a.walling 2010-10-04 13:45) - PLID 40738 - If restricted, limit the number of records
		if (bRestricted) {
			strSelect = FormatString("SELECT TOP %li ", GetPatientExportRestrictions().Limit());
		} else {
			strSelect = "SELECT ";
		}
		if(ert == ertHistory)
		{
			// (z.manning 2009-12-11 12:40) - PLID 36519 - The history export is just a file export so
			// just hard code the columns as having them select fields isn't relevant.
			strSelect += "MailSent.PathName, PersonT.ID AS PersonID, PersonT.First, PersonT.Last, PatientsT.UserDefinedID "
				", CASE WHEN PatientsT.PersonID IS NULL THEN CONVERT(bit, 0) ELSE CONVERT(bit, 1) END AS IsPatient ";
		}
		else
		{
			for(int i = 0; i < arFields.GetSize(); i++) {
				CString str;
				//Give it a unique alias.
				CString strField;
				CExportField ef = GetFieldByID(arFields.GetAt(i).nID);
				if(ef.eftType == eftAdvanced) {
					strField = GetNthField(arFields.GetAt(i).strFormat,3,"");
				}
				else if(ef.eftType == eftEmnItem) {
					strField = ef.GetField(); // (a.walling 2011-03-11 15:24) - PLID 42786 - GetField may format the strField further for SSNs etc
					CString strDynamicID;
					strDynamicID.Format("%li", arFields.GetAt(i).nDynamicID);
					strField.Replace("DYNAMIC_ID", strDynamicID);
				}
				else {
					strField = ef.GetField(); // (a.walling 2011-03-11 15:24) - PLID 42786 - GetField may format the strField further for SSNs etc
				}
				str.Format("%s AS Field%i, ", strField, i);
				strSelect += str;
			}
			ASSERT(strSelect.Right(2) == ", ");//We shouldn't have stored exports with 0 fields.
			strSelect = strSelect.Left(strSelect.GetLength() - 2);
		}

		// (j.jones 2006-03-22 11:32) - PLID 19384 - we need to track the number of tables in use,
		// so we don't exceed SQL's limit of 256
		long nTableCount = 0;
		CString strTest = strSelect;
		while(!strTest.IsEmpty() && (strTest.Find("JOIN") != -1 || strTest.Find("FROM") != -1)) {
			long nStartJoin = strTest.Find("JOIN");
			long nStartFrom = strTest.Find("FROM");

			if((nStartFrom != -1 && nStartJoin < nStartFrom && nStartJoin != -1) || (nStartFrom == -1 && nStartJoin != -1)) {
				nTableCount++;
				strTest = strTest.Right(strTest.GetLength() - 4 - nStartJoin);
				continue;
			}

			if((nStartJoin != -1 && nStartFrom < nStartJoin && nStartFrom != -1) || (nStartJoin == -1 && nStartFrom != -1)) {
				nTableCount++;
				strTest = strTest.Right(strTest.GetLength() - 4 - nStartFrom);
				continue;
			}
		}

		strFrom = GetExportFromClause(ert);

		// (j.jones 2006-03-22 11:32) - PLID 19384 - we need to track the number of tables in use,
		// so we don't exceed SQL's limit of 256
		strTest = strFrom;
		while(!strTest.IsEmpty() && (strTest.Find("JOIN") != -1 || strTest.Find("FROM") != -1)) {
			long nStartJoin = strTest.Find("JOIN");
			long nStartFrom = strTest.Find("FROM");

			if((nStartFrom != -1 && nStartJoin < nStartFrom && nStartJoin != -1) || (nStartFrom == -1 && nStartJoin != -1)) {
				nTableCount++;
				strTest = strTest.Right(strTest.GetLength() - 4 - nStartJoin);
				continue;
			}

			if((nStartJoin != -1 && nStartFrom < nStartJoin && nStartFrom != -1) || (nStartJoin == -1 && nStartFrom != -1)) {
				nTableCount++;
				strTest = strTest.Right(strTest.GetLength() - 4 - nStartFrom);
				continue;
			}
		}

		//TES 6/5/2007 - PLID 26125 - Start the WHERE clause off with the base WHERE clause.
		strWhere.Format("WHERE %s ", GetBaseWhereClause(ert));
		ExportFilterOption efo = (ExportFilterOption)VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secFilterType));
		long nFilterFlags = VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secFilterFlags));
		//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
		//I changed this selection structure because I did not want to duplicate the code for case efoAllNew
		if(efo == efoAllNew || efo == efoAllNewModified)
		{
					CString strLastExport = FormatDateTimeForSql(VarDateTime(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secLastExportDate),COleDateTime(1900,1,1,0,0,0)));
					switch(ert) {
					case ertPatients:
						{
							// (a.walling 2008-03-06 15:46) - PLID 29226 - We used to say InputDate AND ModifiedDate, which resulted in selecting only
							// records that were BOTH input AND modified after the last export.
							if(efo == efoAllNewModified) {
								//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
								strWhere += FormatString ("AND (PersonPat.InputDate > '%s' OR PatientsT.ModifiedDate > '%s')", strLastExport, strLastExport);
							} else {
								strWhere += FormatString(" AND PersonPat.InputDate > '%s'", strLastExport);
							}								
						}
						break;
					case ertAppointments:
						{
							// (a.walling 2008-03-06 15:46) - PLID 29226 - We used to say InputDate AND ModifiedDate, which resulted in selecting only
							// records that were BOTH input AND modified after the last export.
							if(efo == efoAllNewModified) {
								//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
								strWhere += FormatString("AND (AppointmentsT.CreatedDate > '%s' OR AppointmentsT.ModifiedDate > '%s')", strLastExport, strLastExport);

							} else {
								strWhere += FormatString(" AND AppointmentsT.CreatedDate > '%s'", strLastExport);
							}
						}
						break;
					case ertCharges:
						{
							strWhere += FormatString(" AND LineChargeT.InputDate > '%s'", strLastExport);
						}
						break;
					case ertPayments:
						{
							strWhere += FormatString(" AND LinePayT.InputDate > '%s'", strLastExport);
						}
						break;
					case ertEMNs:
						{
							strWhere += FormatString(" AND %s AND EmrMasterT.InputDate > '%s'", GetCurrentExtraEmnFilter(), strLastExport);
						}
						break;
					case ertHistory: // (z.manning 2009-12-11 12:03) - PLID 36519
						{
							strWhere += FormatString(" AND %s AND MailSent.Date > '%s'", GetCurrentExtraHistoryFilter(), strLastExport);
						}
						break;
					default:
						ASSERT(FALSE);
						return;
						break;
					}
		}
		else
		{
			switch(efo) {
				// (z.manning 2009-12-14 10:27) - PLID 36576 - Letter writing and date filter are now combined
				// into one option.
				case efoDateOrLwFilter:
					{
						if(nFilterFlags & effDate)
						{
							FilterableDate fd = (FilterableDate)VarLong(m_pDateFilters->GetValue(m_pDateFilters->CurSel,0));
							COleDateTime dtFrom = VarDateTime(m_dtFrom.GetValue());
							COleDateTime dtTo = VarDateTime(m_dtTo.GetValue()) + COleDateTimeSpan(1,0,0,0);
							switch(ert) {
							case ertPatients:
								{
									switch(fd) {
									case fdFirstContactDate:
										strWhere += FormatString(" AND PersonPat.FirstContactDate >= '%s' AND PersonPat.FirstContactDate < '%s'", 
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdHasAppointmentDate:
										strWhere += FormatString(" AND PersonPat.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
											"AND Date < '%s')", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdNextAppointmentDate:
										strWhere += FormatString(" AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdLastAppointmentDate:
										strWhere += FormatString(" AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									default:
										ASSERT(FALSE);
										break;
									}
								}
								break;
							
							case ertAppointments:
								{
									switch(fd) {
									case fdAppointmentDate:
										strWhere += FormatString(" AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date < '%s'", 
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdAppointmentInputDate:
										strWhere += FormatString(" AND AppointmentsT.CreatedDate >= '%s' AND AppointmentsT.CreatedDate < '%s'", 
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdFirstContactDate:
										strWhere += FormatString(" AND AppointmentsT.PatientID <> -25 AND PersonPat.FirstContactDate >= '%s' AND PersonPat.FirstContactDate < '%s'",
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									default:
										ASSERT(FALSE);
										break;
									}
								}
								break;
						
							case ertCharges:
								{
									switch(fd) {
									case fdServiceDate:
										strWhere += FormatString(" AND LineChargeT.Date >= '%s' AND LineChargeT.Date < '%s'",
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdInputDate:
										strWhere += FormatString(" AND LineChargeT.InputDate >= '%s' AND LineChargeT.InputDate < '%s'",
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdBillDate:
										strWhere += FormatString(" AND BillsT.Date >= '%s' AND BillsT.Date < '%s'",
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdFirstContactDate:
										strWhere += FormatString(" AND PersonPat.FirstContactDate >= '%s' AND PersonPat.FirstContactDate < '%s'", 
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdHasAppointmentDate:
										strWhere += FormatString(" AND PersonPat.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
											"AND Date < '%s')", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdNextAppointmentDate:
										strWhere += FormatString(" AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdLastAppointmentDate:
										strWhere += FormatString(" AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									default:
										ASSERT(FALSE);
										break;
									}
								}
								break;
							
							case ertPayments:
								{
									switch(fd) {
									case fdServiceDate:
										strWhere += FormatString(" AND LinePayT.Date >= '%s' AND LinePayT.Date < '%s'",
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdInputDate:
										strWhere += FormatString(" AND LinePayT.InputDate >= '%s' AND LinePayT.InputDate < '%s'",
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdFirstContactDate:
										strWhere += FormatString(" AND PersonPat.FirstContactDate >= '%s' AND PersonPat.FirstContactDate < '%s'", 
											FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdHasAppointmentDate:
										strWhere += FormatString(" AND PersonPat.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
											"AND Date < '%s')", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdNextAppointmentDate:
										strWhere += FormatString(" AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdLastAppointmentDate:
										strWhere += FormatString(" AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									default:
										ASSERT(FALSE);
										break;
									}
								}
								break;

							case ertEMNs:
								{
									switch(fd) {
									case fdServiceDate:
										strWhere += FormatString(" AND %s AND EmrMasterT.Date >= '%s' AND EmrMasterT.Date < '%s'",
											GetCurrentExtraEmnFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdInputDate:
										strWhere += FormatString(" AND %s AND EmrMasterT.InputDate >= '%s' AND EmrMasterT.InputDate < '%s'",
											GetCurrentExtraEmnFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdFirstContactDate:
										strWhere += FormatString(" AND %s AND PersonPat.FirstContactDate >= '%s' AND PersonPat.FirstContactDate < '%s'", 
											GetCurrentExtraEmnFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdHasAppointmentDate:
										strWhere += FormatString("AND %s AND PersonPat.ID IN (SELECT PatientID FROM AppointmentsT WITH(NOLOCK) WHERE Date >= '%s' "
											"AND Date < '%s')", GetCurrentExtraEmnFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdNextAppointmentDate:
										strWhere += FormatString(" AND %s AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime > getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime < AppointmentsT.StartTime AND OthrAppts.StartTime > getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", GetCurrentExtraEmnFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									case fdLastAppointmentDate:
										strWhere += FormatString(" AND %s AND EXISTS (SELECT ID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PersonPat.ID AND "
											"Date >= '%s' AND Date < '%s' AND StartTime < getdate() AND NOT EXISTS (SELECT ID FROM AppointmentsT AS OthrAppts WITH(NOLOCK) "
											"WHERE OthrAppts.StartTime > AppointmentsT.StartTime AND OthrAppts.StartTime < getdate() AND "
											"OthrAppts.PatientID = PersonPat.ID))", GetCurrentExtraEmnFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									default:
										ASSERT(FALSE);
										break;
									}
								}
								break;

							case ertHistory: // (z.manning 2009-12-11 11:40) - PLID 36519
								{
									switch(fd)
									{
									case fdAttachDate:
										strWhere += FormatString(" AND %s AND MailSent.Date >= '%s' AND MailSent.Date < '%s'",
											GetCurrentExtraHistoryFilter(), FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
										break;
									default:
										ASSERT(FALSE);
										break;
									}
								}
								break;

							default:
								ASSERT(FALSE);
								break;
							}
						}

						if(nFilterFlags & effLetterWriting)
						{
							//Get the filter string
							long nFilterID = VarLong(GetTableField("ExportT", "LetterWritingFilterID", "ID", nExportID));
							_RecordsetPtr rsFilter = CreateRecordset("SELECT Type, Filter FROM FiltersT WHERE ID = %li", nFilterID);
							long nFilterBasedOn = AdoFldLong(rsFilter, "Type");
							CString strFilterString = AdoFldString(rsFilter, "Filter");
							//Convert the string we have to an SQL
							CString strFilterWhere, strFilterFrom;
							if(!CFilter::ConvertFilterStringToClause(nFilterID, strFilterString, nFilterBasedOn, &strFilterWhere, &strFilterFrom)) {
								MsgBox("Record list could not be generated because the export uses an invalid filter.");
								GetDlgItem(IDC_EXPORT)->EnableWindow(FALSE);
								return;
							}
							
							switch(ert) {
							case ertPatients:
								{
									ASSERT(nFilterBasedOn == fboPerson);
									strWhere += FormatString(" AND PersonPat.ID IN (SELECT PersonT.ID AS PersonID FROM %s "
										"WHERE %s GROUP BY PersonT.ID)", strFilterFrom, strFilterWhere);
								}
								break;
							case ertAppointments:
								{
									ASSERT(nFilterBasedOn == fboPerson || nFilterBasedOn == fboAppointment);
									if(nFilterBasedOn == fboPerson) {
										strWhere += FormatString(" AND AppointmentsT.PatientID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
											strFilterFrom, strFilterWhere);
									}
									else if(nFilterBasedOn == fboAppointment) {
										strWhere += FormatString(" AND AppointmentsT.ID IN (SELECT AppointmentsT.ID AS AppointmentID FROM %s WHERE %s GROUP BY AppointmentsT.ID)",
											strFilterFrom, strFilterWhere);
									}
								}
								break;
							case ertCharges:
								{
									ASSERT(nFilterBasedOn == fboPerson);
									strWhere += FormatString(" AND PersonPat.ID IN (SELECT PersonT.ID AS PersonID FROM %s "
										"WHERE %s GROUP BY PersonT.ID)", strFilterFrom, strFilterWhere);
								}
								break;
							case ertPayments:
								{
									ASSERT(nFilterBasedOn == fboPerson || nFilterBasedOn == fboPayment);
									if(nFilterBasedOn == fboPerson) {
										strWhere += FormatString(" AND LinePayT.PatientID IN (SELECT PersonT.ID AS PersonID "
											"FROM %s WHERE %s GROUP BY PersonT.ID)", strFilterFrom, strFilterWhere);
									}
									else {
										strWhere += FormatString(" AND LinePayT.ID IN (SELECT PaymentsT.ID AS PayID FROM %s WHERE %s)",
											strFilterFrom, strFilterWhere);
									}
								}
								break;
							case ertEMNs:
								{
									ASSERT(nFilterBasedOn == fboPerson || nFilterBasedOn == fboEMR || nFilterBasedOn == fboEMN);
									if(nFilterBasedOn == fboPerson) {
										strWhere += FormatString(" AND %s AND EMRMasterT.PatientID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", 
											GetCurrentExtraEmnFilter(), strFilterFrom, strFilterWhere);
									}
									else if(nFilterBasedOn == fboEMN) {
										strWhere += FormatString(" AND %s AND EMRMasterT.ID IN (SELECT EMRMasterT.ID AS EMNID FROM %s WHERE %s)",
											GetCurrentExtraEmnFilter(), strFilterFrom, strFilterWhere);
									}
									else {
										strWhere += FormatString(" AND %s AND EMRGroupsT.Deleted = 0 AND EMRGroupsT.ID IN (SELECT EmrGroupsT.ID AS EMRID FROM %s WHERE %s)",
											GetCurrentExtraEmnFilter(), strFilterFrom, strFilterWhere);
									}
								}
								break;

							case ertHistory: // (z.manning 2009-12-11 11:15) - PLID 36519
								{
									ASSERT(nFilterBasedOn == fboPerson);
									strWhere += FormatString(" AND %s AND PersonT.ID IN (SELECT PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)"
										, GetCurrentExtraHistoryFilter(), strFilterFrom, strFilterWhere);
								}
								break;

							default:
								ASSERT(FALSE);
								break;
							}
						}
					}
					break;

				case efoManual:
					{
						//PLID 17014
						//if the in clause is too large, SQL gives errors, so lets insert the IDs into a temp table
						// use them and then delete the temp table
						CStringArray aryFieldNames;
						CStringArray aryFieldTypes;
						CString strIn;
										

						//this isn't the correct way to do this, it'll be a lot slower especially if there are a lot of values in the list
						/*for(int i = 0; i < m_pSelect->GetRowCount(); i++) {
							CString str;
							str.Format("%li,",VarLong(m_pSelect->GetValue(i,0)));
							strIn += str;
						}*/

						//Limit the size of a single XML statement, otherwise it can cause errors.
						CStringArray saXMLStatements;

						if(m_pSelect->GetRowCount() == 0) {
							AfxMessageBox("You have not selected any records to export.");
							return;
						}
						
						long p = m_pSelect->GetFirstRowEnum();
						LPDISPATCH lpDisp = NULL;
						if (p) {
							//start the XML
							strIn = "<ROOT>";
						}
						while (p) {
							CString str;
							m_pSelect->GetNextRowEnum(&p, &lpDisp);
							IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
							str.Format("<P ID=\"%li\"/>", VarLong(pRow->GetValue(0)));			
							strIn += str;
							if(strIn.GetLength() > 2000) {
								//End this statement.
								strIn += "</ROOT>";
								saXMLStatements.Add(strIn);
								//Start a new one.
								strIn = "<ROOT>";
							}
						}

						if (!strIn.IsEmpty()) {
							//now add the trailer
							strIn += "</ROOT>";
							saXMLStatements.Add(strIn);
						}

						aryFieldNames.Add("ID");
						aryFieldTypes.Add("int");

						//now create our temp table
						CString strTempT = CreateTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(0));
						for(int i = 1; i < saXMLStatements.GetSize(); i++) {
							AppendToTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(i), strTempT);
						}
						
						//TES 6/5/2007 - PLID 26125 - I'm not including the base WHERE clause here, records that are 
						// filtered out by that clause shouldn't be availabe for the user to select, if they did somehow
						// manage to select them, I think it's marginally better to include them (since that's what the user
						// expects) than to not include them.
						switch(ert) {
						case ertPatients:
							strWhere.Format("WHERE PersonPat.ID IN (SELECT ID FROM %s)", strTempT);
							break;
						case ertAppointments:
							strWhere.Format("WHERE AppointmentsT.ID IN (SELECT ID FROM %s)", strTempT);
							break;
						case ertCharges:
							strWhere.Format("WHERE LineChargeT.ID IN (SELECT ID FROM %s)", strTempT);
							break;
						case ertPayments:
							strWhere.Format("WHERE LinePayT.ID IN (SELECT ID FROM %s)", strTempT);
							break;
						case ertEMNs:
							strWhere.Format("WHERE EMRMasterT.Deleted = 0 AND EmrMasterT.ID IN (SELECT ID FROM %s)", strTempT);
							break;
						case ertHistory:
							// (z.manning 2009-12-11 12:07) - PLID 36519 - We don't currently support manual history exporting
							ASSERT(FALSE);
							break;
						default:
							ASSERT(FALSE);
							break;
						}
					}
					break;

				default:
					ASSERT(FALSE);
					return;
					break;
			}
		}

		// (j.jones 2006-03-22 11:32) - PLID 19384 - we need to track the number of tables in use,
		// so we don't exceed SQL's limit of 256
		strTest = strWhere;
		while(!strTest.IsEmpty() && (strTest.Find("JOIN") != -1 || strTest.Find("FROM") != -1)) {
			long nStartJoin = strTest.Find("JOIN");
			long nStartFrom = strTest.Find("FROM");

			if((nStartFrom != -1 && nStartJoin < nStartFrom && nStartJoin != -1) || (nStartFrom == -1 && nStartJoin != -1)) {
				nTableCount++;
				strTest = strTest.Right(strTest.GetLength() - 4 - nStartJoin);
				continue;
			}

			if((nStartJoin != -1 && nStartFrom < nStartJoin && nStartFrom != -1) || (nStartJoin == -1 && nStartFrom != -1)) {
				nTableCount++;
				strTest = strTest.Right(strTest.GetLength() - 4 - nStartFrom);
				continue;
			}
		}

		// (j.jones 2006-03-22 12:20) - per d.thompson's request, we will actually limit them to 240 tables
		if(nTableCount > 240) {
			CString str;
			str.Format("The current export has specified too many fields to process.\n"
				"Please edit your export configuration and remove at least %li field(s) that have the * indicator.\n\n"
				"(Note: depending on the complexity of the export fields and filters in use,\n"
				"it is possible you may get this warning again and will need to remove more than %li field(s).)", nTableCount - 240, nTableCount - 240);
			AfxMessageBox(str);
			return;
		}

		//Load the info we'll need (we'd prefer not to open two recordsets at once.
		_RecordsetPtr rsExportInfo = CreateRecordset("SELECT PromptForFile, Filename, OutputType, FieldSeparator, FieldEscape, TextDelimiter, TextEscape, RecordSeparator, ManualSort, CreateTodo, TodoIntervalAmount, TodoIntervalUnit, TodoUser, RecordEscape FROM ExportT WHERE ID = %li", nExportID);
		BOOL bManualSort = AdoFldBool(rsExportInfo, "ManualSort");
		BOOL bPromptForFile = AdoFldBool(rsExportInfo, "PromptForFile");
		CString strFileName = AdoFldString(rsExportInfo, "Filename", "");
		ExportOutputType eot = (ExportOutputType)AdoFldLong(rsExportInfo, "OutputType");
		CArray<SpecialChar,SpecialChar> arSpecialChars;
		CString strFieldSeparator, strTextDelimiter;
		if(eot == eotCharacterSeparated) {
			strFieldSeparator = AdoFldString(rsExportInfo, "FieldSeparator");
			SpecialChar sc;
			sc.strSourceChar = strFieldSeparator;
			sc.strReplaceChar = AdoFldString(rsExportInfo, "FieldEscape");
			arSpecialChars.Add(sc);
			strTextDelimiter = AdoFldString(rsExportInfo, "TextDelimiter");
			sc.strSourceChar = strTextDelimiter;
			sc.strReplaceChar = AdoFldString(rsExportInfo, "TextEscape");
			arSpecialChars.Add(sc);
		}
		CString strRecordSeparator = AdoFldString(rsExportInfo, "RecordSeparator");
		SpecialChar sc;
		sc.strSourceChar = strRecordSeparator;
		sc.strReplaceChar = AdoFldString(rsExportInfo, "RecordEscape");
		arSpecialChars.Add(sc);

		bool bCreateTodo = AdoFldBool(rsExportInfo, "CreateTodo")?true:false;
		long nTodoAmount = AdoFldLong(rsExportInfo, "TodoIntervalAmount",-1);
		long nTodoUnit = AdoFldLong(rsExportInfo, "TodoIntervalUnit",-1);
		long nTodoUser = AdoFldLong(rsExportInfo, "TodoUser", -1);
		rsExportInfo->Close();

		CString strOrder;
		CString strSql;
		if(efo == efoManual && bManualSort) {
			//OK, we need to set up our temp table.
			//Limit the size of a single XML statement, otherwise it can cause errors.
			CStringArray saXMLStatements;
			CString strIn;
			long p = m_pSelect->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			int i = 0;
			if (p) {
				//start the XML
				strIn = "<ROOT>";
			}
			while (p) {
				CString str;
				m_pSelect->GetNextRowEnum(&p, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
				str.Format("<P RecordID=\"%li\" SortOrder=\"%i\"/>", VarLong(pRow->GetValue(0)), i);			
				strIn += str;
				if(strIn.GetLength() > 2000) {
					//End this statement.
					strIn += "</ROOT>";
					saXMLStatements.Add(strIn);
					//Start a new one.
					strIn = "<ROOT>";
				}
				i++;
			}

			if (!strIn.IsEmpty()) {
				//now add the trailer
				strIn += "</ROOT>";
				saXMLStatements.Add(strIn);
			}

			CStringArray aryFieldNames, aryFieldTypes;
			aryFieldNames.Add("RecordID");
			aryFieldNames.Add("SortOrder");
			aryFieldTypes.Add("int");
			aryFieldTypes.Add("int");


			//now create our temp table
			CString strTempT = CreateTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(0));

			for(i = 1; i < saXMLStatements.GetSize(); i++) {
				AppendToTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(i), strTempT);
			}

			//Now, modify the FROM clause.
			switch(ert) {
			case ertPatients:
				strFrom += " INNER JOIN " + strTempT + " AS OrderQ ON PersonPat.ID = OrderQ.RecordID";
				break;
			case ertAppointments:
				strFrom += " INNER JOIN " + strTempT + " AS OrderQ ON AppointmentsT.ID = OrderQ.RecordID";
				break;
			case ertCharges:
				strFrom += " INNER JOIN " + strTempT + " AS OrderQ ON LineChargeT.ID = OrderQ.RecordID";
				break;
			case ertPayments:
				strFrom += " INNER JOIN " + strTempT + " AS OrderQ ON LinePayT.ID = OrderQ.RecordID";
				break;
			case ertEMNs:
				strFrom += " INNER JOIN " + strTempT + " AS OrderQ ON EmrMasterT.ID = OrderQ.RecordID";
				break;
			case ertHistory:
				// (z.manning 2009-12-11 12:10) - PLID 36519 - We do not support manual sorting for history exports
				ASSERT(FALSE);
				break;
			default:
				ASSERT(FALSE);
				break;
			}

			strOrder = "ORDER BY OrderQ.SortOrder ASC";
		}
		// (z.manning 2009-12-11 12:33) - PLID 36519 - History doesn't support ordering
		else if(ert != ertHistory) {
			//Now, generate our ORDER BY clause.
			_RecordsetPtr rsSortFields = CreateRecordset("SELECT FieldID, SortDescending, ExtraInfo FROM ExportSortFieldsT WHERE ExportID = %li ORDER BY SortOrder ASC", nExportID);
			CArray<SortField,SortField> arSortFields;
			while(!rsSortFields->eof) {
				SortField sf;
				sf.nID = AdoFldLong(rsSortFields, "FieldID");
				sf.bSortDescending = AdoFldBool(rsSortFields, "SortDescending")?true:false;
				sf.strExtraInfo = AdoFldString(rsSortFields, "ExtraInfo");
				arSortFields.Add(sf);
				rsSortFields->MoveNext();
			}
			rsSortFields->Close();

			if(arSortFields.GetSize()) {
				strOrder = "ORDER BY ";
				for(int i = 0; i < arSortFields.GetSize(); i++) {
					CString str;
					CString strField;
					CExportField ef = GetFieldByID(arSortFields.GetAt(i).nID);
					if(ef.eftType == eftAdvanced) {
						strField = arSortFields.GetAt(i).strExtraInfo;
					}
					else {
						strField = ef.GetField(); // (a.walling 2011-03-11 15:24) - PLID 42786 - GetField may format the strField further for SSNs etc
					}
					str.Format("%s %s, ", strField, arSortFields.GetAt(i).bSortDescending?"DESC":"ASC");
					strOrder += str;
				}
			
				strOrder = strOrder.Left(strOrder.GetLength() - 2);
			}
		}
		strSql += strSelect + " " + strFrom + " " + strWhere + " " + strOrder;

		_RecordsetPtr rsExportChars = CreateRecordset("SELECT Replace, ReplaceWith FROM ExportSpecialCharsT WHERE ExportID = %li", nExportID);
		while(!rsExportChars->eof) {
			SpecialChar sc;
			sc.strSourceChar = AdoFldString(rsExportChars, "Replace");
			sc.strReplaceChar = AdoFldString(rsExportChars, "ReplaceWith");
			arSpecialChars.Add(sc);
			rsExportChars->MoveNext();
		}
		rsExportChars->Close();

		int nLength = strSql.GetLength();
#ifdef _DEBUG
		//(e.lally 2008-04-10)- Switched to our CMsgBox dialog
		CMsgBox dlg(this);
		dlg.msg = strSql;
		dlg.DoModal();
#endif
		//TES 6/23/2005 - Since this can be a very slow query, let's keep it from timing out.
		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
		CIncreaseCommandTimeout icr(600);

		_RecordsetPtr rsExportData = CreateRecordsetStd(strSql);
		
		if(rsExportData->eof) {
			MsgBox("There were no records selected to export!");
			return;
		}
		//Set up our file.
		CString strHistoryExportDirectory;
		if(bPromptForFile) {
			if(ert == ertHistory) {
				// (z.manning 2009-12-11 13:59) - PLID 36519 - For history exports, we want a directory rather
				// than a filename.
				if(!BrowseToFolder(&strHistoryExportDirectory, "Select Folder for History Export", GetSafeHwnd(), NULL, NULL)) {
					return;
				}
			}
			else {
				CFileDialog SaveAs(FALSE,NULL,"Export.txt");
				// (j.armen 2011-10-25 14:04) - PLID 46132 - GetPracPath is prompting us to save, so it is safe to use the practice path
				// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
				CString dir = GetEnvironmentDirectory();
				SaveAs.m_ofn.lpstrInitialDir = (LPCSTR)dir;
				SaveAs.m_ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
				if (SaveAs.DoModal() == IDCANCEL) {
					return;
				}
				strFileName = SaveAs.GetPathName();
			}
		}
		else if(ert != ertHistory) {
			if(strFileName.Find("%n") != -1) {
				int n = 1;
				CString strTmp = strFileName;
				CString strN;
				strN.Format("%i", n);
				strTmp.Replace("%n", strN);
				while(FileUtils::DoesFileOrDirExist(strTmp)) {
					n++;
					strTmp = strFileName;
					CString strN;
					strN.Format("%i", n);				
					strTmp.Replace("%n", strN);
				}
				strFileName = strTmp;
			}
		}
		else {
			strHistoryExportDirectory = strFileName;
		}

		CString strFolder;
		if(ert == ertHistory) {
			strHistoryExportDirectory.TrimRight('\\');
			strFolder = strHistoryExportDirectory;
		}
		else {
			strFolder = strFileName;
			int nBackslash = strFileName.ReverseFind('\\');
			if(nBackslash != -1) strFolder = strFileName.Left(nBackslash);
		}
		if(strFolder.IsEmpty() || !FileUtils::DoesFileOrDirExist(strFolder)) {
			MsgBox("This export is configured to write to a directory (%s) which does not exist.  Please review the settings for this stored export.", strFolder);
			return;
		}

		// (z.manning 2009-12-14 13:57) - PLID 36519 - For history exports, ensure we have a directory
		if(ert == ertHistory && !FileUtils::IsDirectoryAndNotDots(strFolder)) {
			MessageBox(strFolder + " is not a valid directory for exporting.", "Export", MB_ICONERROR);
			return;
		}

		// (z.manning 2009-12-11 15:09) - PLID 36519 - History exports are handled differently since
		// it's a file export.
		if(ert != ertHistory)
		{
			//DRT 9/22/2005 - PLID 17573 - Check to ensure the filename given is valid.  The bPrompt method will check for that, but if
			//	they hardcoded one, it will not.  This also prevents possible problems of file permissions.
			CFile fExport;
			CFileException pError;
			if(fExport.Open(strFileName, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive, &pError) == FALSE) {
				//We failed to load this filename, it's invalid, we don't have permission, something of that variety
				CString strMsg;

				switch(pError.m_cause) {
				case CFileException::badPath:	//All or part of the path is invalid.
					strMsg.Format("The export data was unable to be created, the filename '%s' is invalid.", strFileName);
					break;
				case CFileException::accessDenied:	//The file could not be accessed.
					strMsg.Format("Access was denied to the file '%s', the export will be halted.", strFileName);
					break;

				/*Generic errors that shouldn't happen in this case
				CFileException::none   No error occurred.
				CFileException::generic   An unspecified error occurred.
				CFileException::fileNotFound   The file could not be located.
				CFileException::tooManyOpenFiles   The permitted number of open files was exceeded.
				CFileException::invalidFile   There was an attempt to use an invalid file handle.
				CFileException::removeCurrentDir   The current working directory cannot be removed.
				CFileException::directoryFull   There are no more directory entries.
				CFileException::badSeek   There was an error trying to set the file pointer.
				CFileException::hardIO   There was a hardware error.
				CFileException::sharingViolation   SHARE.EXE was not loaded, or a shared region was locked.
				CFileException::lockViolation   There was an attempt to lock a region that was already locked.
				CFileException::diskFull   The disk is full.
				CFileException::endOfFile   The end of file was reached. 
				*/
				default:
					strMsg.Format("An unknown error occurred while attempting to create the file '%s'.  Please ensure the file path and name are valid, "
						"and the location you are saving to exists.", strFileName);
				}

				AfxMessageBox(strMsg, MB_OK);
				return;
			}

			CArchive arExport(&fExport, CArchive::store);

			if(VarBool(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secIncludeFieldNames))) {
				//They want the first row to be the display names.
				CString strFieldNames;
				for(long i = 0; i < arFields.GetSize(); i++) {
					//First we have to get the first three format fields, but the defaults are different based on the type, grr.
					ExportFieldType eft = GetFieldByID(arFields.GetAt(i).nID).eftType;
					CString strFormat = arFields.GetAt(i).strFormat;
					CString strFirstThree;
					switch(eft) {
					case eftPlaceholder:
					case eftAdvanced:
						strFirstThree = GetNthField(strFormat,0,"5") + "|" + GetNthField(strFormat,1,"0") + "|" + GetNthField(strFormat,2," ");
						break;
					case eftGenericText:
					case eftGenericNtext:
					case eftDateTime:
					case eftEmnItem:
						strFirstThree = GetNthField(strFormat,0,"10") + "|" + GetNthField(strFormat,1,"0") + "|" + GetNthField(strFormat,2," ");
						break;
					case eftGenericNumber:
						strFirstThree = GetNthField(strFormat,0,"10") + "|" + GetNthField(strFormat,1,"1") + "|" + GetNthField(strFormat,2,"0");
						break;
					case eftCurrency:
						strFirstThree = GetNthField(strFormat,0,"10") + "|" + GetNthField(strFormat,1,"1") + "|" + GetNthField(strFormat,2," ");
						break;
					case eftPhoneNumber:
						strFirstThree = GetNthField(strFormat,0,"14") + "|" + GetNthField(strFormat,1,"0") + "|" + GetNthField(strFormat,2," ");
						break;
					case eftSSN:
						strFirstThree = GetNthField(strFormat,0,"11") + "|" + GetNthField(strFormat,1,"0") + "|" + GetNthField(strFormat,2," ");
						break;
					case eftBool:
					case eftGender:
					case eftMarital:
						strFirstThree = GetNthField(strFormat,0,"1") + "|" + GetNthField(strFormat,1,"0") + "|" + GetNthField(strFormat,2," ");
						break;
					case eftDiag:
						strFirstThree = GetNthField(strFormat,0,"6") + "|" + GetNthField(strFormat,1,"0") + "|" + GetNthField(strFormat,2," ");
						break;
					}

					CString strFieldName;
					if(eft == eftEmnItem) {
						_RecordsetPtr rs = CreateRecordset("SELECT Name FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID WHERE EmrInfoMasterT.ID = %li", arFields.GetAt(i).nDynamicID);
						if(!rs->eof) {
							strFieldName = AdoFldString(rs, "Name","");
						}
						else {
							//the item was deleted!
							continue;
						}
						rs->Close();
					}
					else {
						strFieldName = GetFieldByID(arFields.GetAt(i).nID).GetDisplayName();
					}
					strFieldNames += strTextDelimiter + FormatExportField(_variant_t(), eftPlaceholder, strFirstThree + "|" + strFieldName, eot == eotFixedWidth, arSpecialChars) + strTextDelimiter + strFieldSeparator;
				}
				strFieldNames += strRecordSeparator;
				arExport.WriteString(strFieldNames);
			}

			//Now, go through all our records, and export them.
			while(!rsExportData->eof) {
				CString strRecord;
				for(long i = 0; i < arFields.GetSize(); i++) {
					strRecord += strTextDelimiter + FormatExportField(rsExportData->Fields->GetItem(i)->Value, GetFieldByID(arFields.GetAt(i).nID).eftType,  arFields.GetAt(i).strFormat, eot == eotFixedWidth, arSpecialChars) + strTextDelimiter + strFieldSeparator;
				}
				strRecord += strRecordSeparator;
				arExport.WriteString(strRecord);
				rsExportData->MoveNext();
			}

			arExport.Close();
			fExport.Close();
		}
		else
		{
			// (z.manning 2009-12-11 15:09) - PLID 36519 - Handle history file exporting

			// (z.manning 2009-12-11 15:20) - PLID 36519 - Create a filename for the export based on today's date.
			CString strZipFile = strHistoryExportDirectory ^ FormatString("HistoryExport_%s.zip", COleDateTime::GetCurrentTime().Format("%Y-%m-%d"));
			if(FileUtils::DoesFileOrDirExist(strZipFile)) {
				// (z.manning 2009-12-11 16:07) - PLID 36519 - The file already exists so prompt to overwrite.
				int nResult = MessageBox("The file '" + strZipFile + "' already exists.\r\n\r\nWould you like to overwrite it?", "Export History", MB_YESNO|MB_ICONQUESTION);
				if(nResult != IDYES) {
					return;
				}

				if(!DeleteFile(strZipFile)) {
					MessageBox("Failed to overwrite existing file", "History Export", MB_ICONERROR);
					return;
				}
			}

			// (z.manning 2009-12-11 16:09) - PLID 36519 - Loop through and store all the files to be
			// exported in a string array.
			CWaitCursor wc;
			CStringArray arystrExportFiles;
			CString strFailedFiles;
			for(; !rsExportData->eof; rsExportData->MoveNext())
			{
				FieldsPtr pflds = rsExportData->GetFields();
				CString strPathName = AdoFldString(pflds, "PathName");
				if(strPathName.Find('\\') == -1) {
					// (z.manning 2009-12-11 16:10) - PLID 36519 - There's no backslag in the path name so
					// we assume this to be just a file name stored in the patient's documents folder.
					if(AdoFldBool(pflds,"IsPatient")) {
						// (a.walling 2010-04-28 17:37) - PLID 38410 - Pass in a connection to GetPatientDocumentPath
						strPathName = GetPatientDocumentPath(GetRemoteData(), AdoFldLong(pflds,"PersonID"), AdoFldString(pflds,"First",""), AdoFldString(pflds,"Last",""), AdoFldLong(pflds,"UserDefinedID"), FALSE)
							^ strPathName;
					}
					else {
						// (z.manning 2009-12-11 16:10) - PLID 36519 - We have just a filename but it's not
						// a patient so skip importing this file.
						strFailedFiles += strPathName + "\r\n";
						continue;
					}
				}

				// (z.manning 2009-12-11 16:20) - PLID 36519 - As long as the file exists then add it to
				// the list of files to export.
				if(FileUtils::DoesFileOrDirExist(strPathName)) {
					arystrExportFiles.Add(strPathName);
				}
				else {
					strFailedFiles += strPathName + "\r\n";
				}
			}

			// (z.manning 2009-12-11 16:21) - PLID 36519 - Go ahead and create the zip file.
			NxCompressUtils::NxCompressFilesToFile(arystrExportFiles, strZipFile);

			if(!strFailedFiles.IsEmpty()) {
				CMsgBox dlgFailedFiles(this);
				dlgFailedFiles.msg = "The following files failed to export...\r\n\r\n" + strFailedFiles;
				dlgFailedFiles.DoModal();
			}

			ShellExecute(NULL, "open", strHistoryExportDirectory, NULL, NULL, SW_SHOWNORMAL);
		}
		
		MsgBox("Export completed!");

		COleDateTime dtExport = COleDateTime::GetCurrentTime();

		ExecuteSql("INSERT INTO ExportHistoryT (ExportID, Date, ExportedBy) "
			"VALUES (%li, '%s', %li)", nExportID, FormatDateTimeForSql(dtExport, dtoDateTime), GetCurrentUserID());

		m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secLastExportDate, _variant_t(dtExport, VT_DATE));

		if(bCreateTodo) {
			COleDateTime dtActive;
			switch((DateInterval)nTodoUnit) {
			case diDays:
					//OK, the date is x days from now.
					dtActive = COleDateTime::GetCurrentTime() + COleDateTimeSpan(nTodoAmount,0,0,0);
				break;
			case diWeeks:
					//OK, the date is x weeks from now.
					dtActive = COleDateTime::GetCurrentTime() + COleDateTimeSpan(nTodoAmount*7,0,0,0);
				break;
			case diMonths:
				{
					//OK, the date is x months from now.
					int nMonths = nTodoAmount;
					int nActiveMonth = COleDateTime::GetCurrentTime().GetMonth() + nMonths;
					int nActiveYear = COleDateTime::GetCurrentTime().GetYear();
					while(nActiveMonth > 12) {
						nActiveYear++;
						nActiveMonth -= 12;
					}
					//This may be an invalid month/day combination.
					int nActiveDay = COleDateTime::GetCurrentTime().GetDay();
					while( (nActiveMonth == 2 && (nActiveYear % 4 != 0 || (nActiveYear % 100 == 0 && nActiveYear % 400 != 0)) && nActiveDay > 28) ||
						(nActiveMonth == 2 && !(nActiveYear % 4 != 0 || (nActiveYear % 100 == 0 && nActiveYear % 400 != 0)) && nActiveDay > 29) ||
						(nActiveMonth == 4 && nActiveDay > 30) ||
						(nActiveMonth == 6 && nActiveDay > 30) ||
						(nActiveMonth == 9 && nActiveDay > 30) ||
						(nActiveMonth == 11 && nActiveDay > 30)) {
						nActiveDay--;
					}
					dtActive.SetDate(nActiveYear, nActiveMonth, nActiveDay);
				}
				break;
			}
			ExecuteSql("UPDATE TodoList Set Done = getdate() WHERE RegardingID = %li AND RegardingType = %i", nExportID, ttExport);
			// (c.haag 2008-06-09 10:53) - PLID 30321 - Use a utility function to create the todo
			TodoCreate(dtActive, dtActive, nTodoUser, CString("Periodic Export '") + VarString(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secName)) + "'",
				"", nExportID, ttExport, -1, -1, ttpMedium);
		}
		OnSelChangedStoredExportList(m_pStoredExportList->CurSel);
	}NxCatchAll("Error in CExportDlg::OnExport()");
}

void CExportDlg::OnManualSortUp() 
{
	if(m_pSelect->CurSel == -1 || m_pSelect->CurSel == 0) return;

	long nInitialCurSel = m_pSelect->CurSel;
	m_pSelect->TakeRowInsert(m_pSelect->GetRow(m_pSelect->CurSel),m_pSelect->CurSel-1);
	m_pSelect->CurSel = nInitialCurSel-1;

	GetDlgItem(IDC_MANUAL_SORT_UP)->EnableWindow(m_pSelect->CurSel != 0);
	GetDlgItem(IDC_MANUAL_SORT_DOWN)->EnableWindow(m_pSelect->CurSel != m_pSelect->GetRowCount()-1);
}

void CExportDlg::OnManualSortDown() 
{
	if(m_pSelect->CurSel == -1 || m_pSelect->CurSel == m_pSelect->GetRowCount()-1) return;

	long nInitialCurSel = m_pSelect->CurSel;
	m_pSelect->TakeRowInsert(m_pSelect->GetRow(m_pSelect->CurSel),m_pSelect->CurSel+2);
	m_pSelect->CurSel = nInitialCurSel+1;
	
	GetDlgItem(IDC_MANUAL_SORT_UP)->EnableWindow(m_pSelect->CurSel != 0);
	GetDlgItem(IDC_MANUAL_SORT_DOWN)->EnableWindow(m_pSelect->CurSel != m_pSelect->GetRowCount()-1);
}

void CExportDlg::LoadExport(long nExportID)
{
	m_pStoredExportList->SetSelByColumn(secID,nExportID);
	OnSelChangedStoredExportList(m_pStoredExportList->CurSel);
}

void CExportDlg::OnDblClickCellStoredExportList(long nRowIndex, short nColIndex) 
{
	OnEditExport();
}

CString CExportDlg::GetCurrentExtraHistoryFilter()
{
	_variant_t varFilter = m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secExtraEmnFilter);
	if(varFilter.vt == VT_BSTR) {
		return VarString(varFilter);
	}
	else {
		CString strExtraHistoryFilter = "1=1";
		if((ExportRecordType)VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secBasedOn)) == ertHistory) {
			_RecordsetPtr prsCategories = CreateParamRecordset(
				"SELECT CategoryID FROM ExportHistoryCategoriesT WHERE ExportID = {INT}"
				, VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secID)));
			CString strCategoryIDList;
			for(; !prsCategories->eof; prsCategories->MoveNext()) {
				strCategoryIDList += AsString(AdoFldLong(prsCategories, "CategoryID")) + ',';
			}
			prsCategories->Close();
			strCategoryIDList.TrimRight(',');
			if(!strCategoryIDList.IsEmpty()) {
				strExtraHistoryFilter = FormatString("MailSent.CategoryID IN (%s)", strCategoryIDList);
			}
			else {
				// (z.manning 2009-12-15 14:39) - This means we want all categories
			}
		}
		m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secExtraEmnFilter, _bstr_t(strExtraHistoryFilter));
		return strExtraHistoryFilter;
	}
}

CString CExportDlg::GetCurrentExtraEmnFilter()
{
	_variant_t varFilter = m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secExtraEmnFilter);
	if(varFilter.vt == VT_BSTR) {
		return VarString(varFilter);
	}
	else {
		CString strExtraEmnFilter = "1=1";
		if((ExportRecordType)VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel,secBasedOn)) == ertEMNs && !VarBool(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secAllowOtherTemplates))) {
			_RecordsetPtr rsTemplates = CreateRecordset("SELECT EmnTemplateID FROM ExportEmnTemplatesT WHERE ExportID = %li", VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secID)));
			CString strTemplateIDList;
			while(!rsTemplates->eof) {
				CString strID;
				strID.Format("%li,", AdoFldLong(rsTemplates, "EmnTemplateID"));
				strTemplateIDList += strID;
				rsTemplates->MoveNext();
			}
			strTemplateIDList.TrimRight(",");
			if(strTemplateIDList.IsEmpty()) strTemplateIDList = "-1";
			strExtraEmnFilter = "(EmrMasterT.TemplateID Is Not NULL AND EMRMasterT.Deleted = 0 AND EmrMasterT.TemplateID IN (" + strTemplateIDList + "))";
		}
		m_pStoredExportList->PutValue(m_pStoredExportList->CurSel, secExtraEmnFilter, _bstr_t(strExtraEmnFilter));
		return strExtraEmnFilter;
	}
}

void CExportDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	CWaitCursor pWait;

	long nCurSel = -1;
	if(m_pStoredExportList->CurSel != -1) {
		nCurSel = VarLong(m_pStoredExportList->GetValue(m_pStoredExportList->CurSel, secID));
	}
	m_pStoredExportList->Requery();
	m_pStoredExportList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	if(nCurSel != -1) m_pStoredExportList->SetSelByColumn(secID, nCurSel);
	OnSelChangedStoredExportList(m_pStoredExportList->CurSel);

	/*if(!m_pAvail->IsRequerying() && m_pAvail->FromClause != _bstr_t("")) {
		m_pAvail->Requery();		
	}

	if(!m_pSelect->IsRequerying() && m_pSelect->FromClause != _bstr_t("")) {
		m_pSelect->Requery();
	}

	if(m_pAvail->IsRequerying())
		m_pAvail->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	if(m_pSelect->IsRequerying())
		m_pSelect->WaitForRequery(dlPatienceLevelWaitIndefinitely);*/
}

LRESULT CExportDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
		case NetUtils::ExportT:
			{
				try {
					if(lParam == -1) {
						UpdateView();
					}
					else {
						//Did we send this ourselves?
						if(m_lExpectedTableCheckers.Find(lParam)) {
							//Yes.  Discard this message, and stop expecting it.
							m_lExpectedTableCheckers.RemoveAt(m_lExpectedTableCheckers.Find(lParam));
							return 0;
						}
						long nExportID = lParam;
						long nRow = m_pStoredExportList->FindByColumn(secID, nExportID, 0, VARIANT_FALSE);
						if(nRow == -1) {
							//This is a new row, add it.
							_RecordsetPtr rsExportInfo = CreateRecordset("SELECT ID, Name, ExportHistoryQ.Date, BasedOn, FilterType, "
								"IncludeFieldNames, ManualSort, AllowOtherTemplates "
								"FROM ExportT LEFT JOIN (SELECT ExportID, Max(Date) AS Date FROM ExportHistoryT GROUP BY ExportID) AS ExportHistoryQ ON ExportT.ID = ExportHistoryQ.ExportID "
								"WHERE ExportT.ID = %li", nExportID);
							if(rsExportInfo->eof) {
								//It was deleted, but we didn't have it anyway, so we don't need to do anything.
							}
							else {
								IRowSettingsPtr pRow = m_pStoredExportList->GetRow(-1);
								FieldsPtr fExport = rsExportInfo->Fields;
								pRow->PutValue(secID, fExport->GetItem("ID")->Value);
								pRow->PutValue(secName, fExport->GetItem("Name")->Value);
								pRow->PutValue(secLastExportDate, fExport->GetItem("Date")->Value);
								pRow->PutValue(secBasedOn, fExport->GetItem("BasedOn")->Value);
								pRow->PutValue(secFilterType, fExport->GetItem("FilterType")->Value);
								pRow->PutValue(secIncludeFieldNames, fExport->GetItem("IncludeFieldNames")->Value);
								pRow->PutValue(secManualSort, fExport->GetItem("ManualSort")->Value);
								pRow->PutValue(secAllowOtherTemplates, fExport->GetItem("AllowOtherTemplates")->Value);
								_variant_t varNull;
								varNull.vt = VT_NULL;
								pRow->PutValue(secExtraEmnFilter, varNull);
								m_pStoredExportList->AddRow(pRow);
							}
						}
						else {
							//Update this row.
							_RecordsetPtr rsExportInfo = CreateRecordset("SELECT ID, Name, ExportHistoryQ.Date, BasedOn, FilterType, "
								"IncludeFieldNames, ManualSort, AllowOtherTemplates "
								"FROM ExportT LEFT JOIN (SELECT ExportID, Max(Date) AS Date FROM ExportHistoryT GROUP BY ExportID) AS ExportHistoryQ ON ExportT.ID = ExportHistoryQ.ExportID "
								"WHERE ExportT.ID = %li", nExportID);
							if(rsExportInfo->eof) {
								//This item has been deleted.  Remove it from our list.
								bool bSelChanged = false;
								if(nRow == m_pStoredExportList->CurSel) {
									//We're removing the export they have selected, so our selection will change.
									bSelChanged = true;
									//If they're looking at the screen, they will be taken aback to see their export disappear.
									if(IsWindowVisible()) {
										MsgBox("The export you have selected has been deleted by another user.");
									}
								}
								m_pStoredExportList->RemoveRow(nRow);
								if(bSelChanged) {
									OnSelChangedStoredExportList(m_pStoredExportList->CurSel);
								}
							}
							else {
								//Update the row with the new settings.
								IRowSettingsPtr pRow = m_pStoredExportList->GetRow(nRow);
								FieldsPtr fExport = rsExportInfo->Fields;
								pRow->PutValue(secID, fExport->GetItem("ID")->Value);
								pRow->PutValue(secName, fExport->GetItem("Name")->Value);
								pRow->PutValue(secLastExportDate, fExport->GetItem("Date")->Value);
								pRow->PutValue(secBasedOn, fExport->GetItem("BasedOn")->Value);
								pRow->PutValue(secFilterType, fExport->GetItem("FilterType")->Value);
pRow->PutValue(secIncludeFieldNames, fExport->GetItem("IncludeFieldNames")->Value);
pRow->PutValue(secManualSort, fExport->GetItem("ManualSort")->Value);
pRow->PutValue(secAllowOtherTemplates, fExport->GetItem("AllowOtherTemplates")->Value);
_variant_t varNull;
varNull.vt = VT_NULL;
pRow->PutValue(secExtraEmnFilter, varNull);

if (nRow == m_pStoredExportList->CurSel) {
	if (IsWindowVisible()) {
		MsgBox("The export you have selected has been modified by another user. Your screen will be refreshed to reflect the new settings.");
		OnSelChangedStoredExportList(m_pStoredExportList->CurSel);
	}
}
							}
						}
					}
				} NxCatchAll("Error in CExportDlg::OnTableChanged:ExportT");
			}
			break;
		}
	}NxCatchAll("Error in CExportDlg::OnTableChanged()");
	return 0;
}

// (j.jones 2006-12-27 12:19) - PLID 23281 - created to support the ASC exports
void CExportDlg::OnBtnSystemExport()
{
	try
	{
		//pop up a menu of options

		// (j.jones 2006-12-27 12:19) - PLID 23281 - added AHCA
		// (j.jones 2007-07-03 09:09) - PLID 25493 - added VHI
		// (z.manning 2015-05-13 11:57) - PLID 66048 - Added UTDH, OHPR, AND PHC4

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_AHCA_EXPORT, "Florida &AHCA Export");
		// (b.spivey, December 18th, 2014) - PLID 64158 - Alphabetical
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_IL_IDPH, "Illinois I&DPH Export");
		// (c.haag 2014-09-10) - PLID 63612 - Added KY IPOP. Lets keep things alphabetical.
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_KY_IPOP, "Kentucky &IPOP Export");
		// (b.spivey, January 07, 2013) - PLID 54538 - added PASRA
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_PASRA_EXPORT, "Missouri &PASRA Export");
		// (z.manning 2015-11-23 09:51) - PLID 67569 - NY ASC export
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_NY_ASC_EXPORT, "&New York SPARCS Export");
		// (s.tullis 2015-05-15 14:23) - PLID 65996 - added OKDH exporter 
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_OK_OKDH_EXPORT, "Oklahoma O&KDH Export");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_OR_OHPR_EXPORT, "Oregon &OHPR Export");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_PA_PHC4_EXPORT, "Pennsylvania P&HC4 Export");
		// (a.walling 2016-01-20 16:43) - PLID 68013 - Tennessee / TNASTC exporter
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_TN_ASTC, "&Tennessee ASTC Export");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_UT_UTDH_EXPORT, "Utah &UTDH Export");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_VHI_EXPORT, "Virginia &VHI Export");
		

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_SYSTEM_EXPORT);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		}
		else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}
	NxCatchAll(__FUNCTION__)
}

// (j.jones 2006-12-27 12:19) - PLID 23281 - created to support the AHCA export
void CExportDlg::OnAHCAExport()
{
	try
	{
		// (z.manning 2016-01-08 09:27) - PLID 67835 - Now a standalone exe
		ASCUtils::OpenFloridaAHCAExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}

// (j.jones 2007-07-03 09:08) - PLID 25493 - created to support the VA ASC export
void CExportDlg::OnVHIExport()
{
	try {

		CVAASCExportDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll("Error in CExportDlg::OnVHIExport");
}

// (c.haag 2014-09-10) - PLID 63612 - Perform a KY IPOP export
void CExportDlg::OnKYIPOPExport()
{
	try
	{
		// (z.manning 2015-05-13 17:44) - PLID 66048 - Moved this code
		ASCUtils::OpenKentuckyIPOPExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__)
}

// (b.spivey, December 18th, 2014) - PLID 64158 - Find the Illinois Exporter if it's open, and open it if it's not. 
void CExportDlg::OnILIDPHExport()
{
	try
	{
		// (z.manning 2015-05-13 17:44) - PLID 66048 - Moved this code
		ASCUtils::OpenIllinoisIDPHExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-05-13 17:47) - PLID 66048
void CExportDlg::OnUTDHExport()
{
	try
	{
		ASCUtils::OpenUtahUTDHExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-05-13 17:47) - PLID 66048
void CExportDlg::OnOHPRExport()
{
	try
	{
		ASCUtils::OpenOregonOHPRExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-05-13 17:47) - PLID 66048
void CExportDlg::OnPHC4Export()
{
	try
	{
		ASCUtils::OpenPennsylvaniaExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}

// (s.tullis 2015-05-15 14:23) - PLID 65996 - added OKDH exporter
void CExportDlg::OnOKDHExport()
{
	try
	{
		ASCUtils::OpenOklahomaExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-11-23 09:54) - PLID 67569
void CExportDlg::OnNYASCExport()
{
	try
	{
		ASCUtils::OpenNewYorkExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}
// (a.walling 2016-01-20 16:43) - PLID 68013 - Tennessee / TNASTC exporter
void CExportDlg::OnTNASTCExport()
{
	try
	{
		ASCUtils::OpenTennesseeExporter(GetSubRegistryKey());
	}
	NxCatchAll(__FUNCTION__);
}


void CExportDlg::OnRequeryFinishedAvailableRecords(short nFlags)
{
	// (b.cardillo 2007-02-16 14:48) - PLID 24791 - Just like letter writing has done for ages, we now also 
	// disable the "add all" button while the requery is going on.  When it's done (in our RequeryFinished event 
	// handler) we enable it again.  We can't allow the user to add all rows while the requery is still loading 
	// them because only the ones that have loaded so far would be added, which makes no sense to the user.
	EnableDlgItem(IDC_ADD_ALL_RECORDS, TRUE);
}

// (a.walling 2010-10-05 13:12) - PLID 40822 - Temporarily lift patient export restrictions
void CExportDlg::OnBnClickedExportLiftPatientRestrictions()
{
	try {
		if (IDNO == MessageBox("NexTech licensing servers will be contacted to authorize lifting restrictions on patient exports for 72 hours.\r\n\r\nDo you want to continue?", NULL, MB_YESNO|MB_ICONINFORMATION)) {
			return;
		}
		GetPatientExportRestrictions().LiftRestrictions();
		MessageBox("Restrictions have been lifted!");
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/27/2011) - PLID 45717 - User clicked on third party exports, give them a choice of 
//  narrow third party specific exports to perform.
void CExportDlg::OnBnClickedBtnThirdPartyExports()
{
	try {

		// (r.gonet 09/27/2011) - PLID 45717 - Added LabCorp Ins Cos		
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_LABCORP_INSURANCE_EXPORT, "&LabCorp Insurance");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_EXPORT_REF_PHYS_ID_LINK, "&Referring Physicians");	// (j.dinatale 2013-01-14 17:23) - PLID 54602
		
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_THIRD_PARTY_EXPORTS);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/27/2011) - PLID 45717 - Added the ability to export insurance companies for LabCorp 
void CExportDlg::OnLabCorpInsuranceExport()
{
	try {
		// (r.gonet 09/27/2011) - PLID 45717 - Launch the export dialog
		CLabCorpInsCoExportDlg dlg(this);
		if(IDOK == dlg.DoModal()) {
			// (r.gonet 09/27/2011) - PLID 45717 - We've exported the insurance companies, so now show the user the file.
			ShellExecute(NULL, "open", dlg.GetExportFilePath(), NULL, NULL, SW_SHOWNORMAL);
			MessageBox("Export Successful!", "Success", MB_ICONINFORMATION);
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-01-14 12:11) - PLID 54602 - export ref phys for hl7
void CExportDlg::OnExportRefPhys()
{
	try{
		CHL7IDLinkExportDlg dlg(hilrtReferringPhysician, this);
		if(IDOK == dlg.DoModal()) {
			ShellExecute(NULL, "open", dlg.GetFilePath(), NULL, NULL, SW_SHOWNORMAL);
			MessageBox("Referring Physician Export Successful!", "Success", MB_ICONINFORMATION);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 07, 2013) - PLID 54538 - handling the launch of PASRA. 
void CExportDlg::OnPASRAExport()
{
	try {
		// (d.lange 2016-01-11 10:27) - PLID 67829 - Split out the PASRA exporter into a standalone exe
		ASCUtils::OpenMissouriPASRAExporter(GetSubRegistryKey());
	}NxCatchAll(__FUNCTION__); 
}
