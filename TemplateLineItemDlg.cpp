// TemplateLineItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "TemplateLineItemDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SchedulerRc.h"
#include "ChooseDateDlg.h"
#include "TemplateItemEntryGraphicalDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2008-05-13 10:37) - PLID 27591 - Removed some COleVariant and VarDateTime usage that is no longer necessary

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;

#define SEND_TO_DIALOG FALSE

enum ResourceListColumns {
	rlcID = 0,
	rlcCheck,
	rlcName,
};

enum ExceptionListColumns {
	elcDate = 0,
};

/////////////////////////////////////////////////////////////////////////////
// CTemplateLineItemDlg dialog

// (z.manning 2014-12-11 09:18) - PLID 64230 - Added editor type to constructor
CTemplateLineItemDlg::CTemplateLineItemDlg(ESchedulerTemplateEditorType eEditorType, CWnd* pParent)
	: CNxDialog(CTemplateLineItemDlg::IDD, pParent),
	m_rectApplyDateLabel(228,14,285,28), // Location of apply date picker and its label are now dynamic
	m_rectApplyDate(290,11,375,33)
{
	//{{AFX_DATA_INIT(CTemplateLineItemDlg)
	m_strResults = _T("");
	//}}AFX_DATA_INIT
	m_bReady = false;
	m_bGotDialogData = false;
	m_pLineItem = NULL;
	m_pOriginalLineItem = NULL;
	m_eEditorType = eEditorType;

	m_esScale = sNone;
	m_embBy = mbNone;
	m_nPatternOrdinal = 1;
	m_nDayNumber = 1;
	m_nInclude = 1;
	m_dtStartDate = g_cdtInvalid;
	m_dtEndDate = g_cdtInvalid;
	m_dtStartTime = g_cdtInvalid;
	m_dtEndTime = g_cdtInvalid;
}

CTemplateLineItemDlg::~CTemplateLineItemDlg()
{
	// (z.manning 2015-01-06 09:31) - PLID 64521 - Clean up the original line item pointer
	if (m_pOriginalLineItem != NULL) {
		delete m_pOriginalLineItem;
		m_pOriginalLineItem = NULL;
	}
}

void CTemplateLineItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateLineItemDlg)
	DDX_Control(pDX, IDC_EVERY_1_RADIO, m_btnEvery1);
	DDX_Control(pDX, IDC_EVERY_2_RADIO, m_btnEvery2);
	DDX_Control(pDX, IDC_EVERY_X_RADIO, m_btnEveryX);
	DDX_Control(pDX, IDC_RADIO_SELECT_RESOURCES, m_btnSelectResources);
	DDX_Control(pDX, IDC_CHECK_BLOCK_APPTS, m_btnShowAsPrecisionTemplate);
	DDX_Control(pDX, IDC_RADIO_ALL_RESOURCES, m_btnAllResources);
	DDX_Control(pDX, IDC_WEDNESDAY_CHECK, m_btnWednesday);
	DDX_Control(pDX, IDC_TUESDAY_CHECK, m_btnTuesday);
	DDX_Control(pDX, IDC_THURSDAY_CHECK, m_btnThursday);
	DDX_Control(pDX, IDC_SUNDAY_CHECK, m_btnSunday);
	DDX_Control(pDX, IDC_SATURDAY_CHECK, m_btnSaturday);
	DDX_Control(pDX, IDC_MONDAY_CHECK, m_btnMonday);
	DDX_Control(pDX, IDC_FRIDAY_CHECK, m_btnFriday);
	DDX_Control(pDX, IDC_CHECK_TO_DATE, m_btnToDate);
	DDX_Text(pDX, IDC_RESULTS_LABEL, m_strResults);
	DDX_Control(pDX, IDC_APPLY_DATE_START, m_dtpStartDate);
	DDX_Control(pDX, IDC_APPLY_DATE_END, m_dtpEndDate);
	DDX_Control(pDX, IDC_EVERY_X_TEXT, m_nxeditEveryXText);
	DDX_Control(pDX, IDC_DAY_NUMBER_EDIT, m_nxeditDayNumberEdit);
	DDX_Control(pDX, IDC_EVERY_X_STATIC, m_nxstaticEveryXStatic);
	DDX_Control(pDX, IDC_RESULTS_LABEL, m_nxstaticResultsLabel);
	DDX_Control(pDX, IDC_BY_LABEL, m_nxstaticByLabel);
	DDX_Control(pDX, IDC_DAY_NUMBER_LABEL, m_nxstaticDayNumberLabel);
	DDX_Control(pDX, IDC_STATIC_FOREVER, m_nxstaticForever);
	DDX_Control(pDX, IDC_BTN_ADD_LINE_ITEM_EXCEPTION, m_btnAddLineItemException);
	DDX_Control(pDX, IDC_BTN_EDIT_LINE_ITEM_EXCEPTION, m_btnEditLineItemException);
	DDX_Control(pDX, IDC_BTN_REMOVE_LINE_ITEM_EXCEPTION, m_btnRemoveLineItemException);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_TIME_GROUPBOX, m_btnTimeGroupbox);
	DDX_Control(pDX, IDC_PERIOD_FRAME, m_btnPeriodFrame);
	DDX_Control(pDX, IDC_BY_DATE_FRAME, m_btnByDateFrame);
	DDX_Control(pDX, IDC_RESULTS_GROUPBOX, m_btnResultsGroupbox);
	DDX_Control(pDX, IDC_APPLIES_GROUPBOX, m_btnAppliesGroupbox);
	DDX_Control(pDX, IDC_INCLUDE_FRAME, m_btnIncludeFrame);
	DDX_Control(pDX, IDC_APPEARANCE_GROUPBOX, m_btnAppearanceGroupbox);
	DDX_Control(pDX, IDC_DATES_GROUPBOX, m_btnDatesGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateLineItemDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplateLineItemDlg)
	ON_CBN_SELCHANGE(IDC_SCALE_COMBO, OnSelchangeScaleCombo)
	ON_BN_CLICKED(IDC_EVERY_X_RADIO, OnEveryXRadio)
	ON_BN_CLICKED(IDC_EVERY_2_RADIO, OnEvery2Radio)
	ON_BN_CLICKED(IDC_EVERY_1_RADIO, OnEvery1Radio)
	ON_CBN_SELCHANGE(IDC_BY_COMBO, OnSelchangeByCombo)
	ON_BN_CLICKED(IDC_MONDAY_CHECK, OnMondayCheck)
	ON_BN_CLICKED(IDC_TUESDAY_CHECK, OnTuesdayCheck)
	ON_BN_CLICKED(IDC_WEDNESDAY_CHECK, OnWednesdayCheck)
	ON_BN_CLICKED(IDC_THURSDAY_CHECK, OnThursdayCheck)
	ON_BN_CLICKED(IDC_SUNDAY_CHECK, OnSundayCheck)
	ON_BN_CLICKED(IDC_FRIDAY_CHECK, OnFridayCheck)
	ON_BN_CLICKED(IDC_SATURDAY_CHECK, OnSaturdayCheck)
	ON_EN_CHANGE(IDC_DAY_NUMBER_EDIT, OnChangeDayNumberEdit)
	ON_CBN_SELCHANGE(IDC_PATTERN_ORDINAL_COMBO, OnSelchangePatternOrdinalCombo)
	ON_EN_CHANGE(IDC_EVERY_X_TEXT, OnChangeEveryXText)
	ON_BN_CLICKED(IDC_CHECK_TO_DATE, OnToDateCheck)
	ON_BN_CLICKED(IDC_RADIO_ALL_RESOURCES, OnAllResourcesBtn)
	ON_BN_CLICKED(IDC_RADIO_SELECT_RESOURCES, OnSelectResourcesBtn)
	ON_BN_CLICKED(IDC_BTN_ADD_LINE_ITEM_EXCEPTION, OnAddLineItemException)
	ON_BN_CLICKED(IDC_BTN_EDIT_LINE_ITEM_EXCEPTION, OnEditLineItemException)
	ON_BN_CLICKED(IDC_BTN_REMOVE_LINE_ITEM_EXCEPTION, OnRemoveLineItemException)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_BLOCK_APPTS, &CTemplateLineItemDlg::OnCheckBlockAppts)
	// (d.singleton 2011-10-13 13:50) - PLID 45945 added these so the OnChangeStartDate and OnChangeEndDate actually fire
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_APPLY_DATE_START, &CTemplateLineItemDlg::OnChangeStartDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_APPLY_DATE_END, &CTemplateLineItemDlg::OnChangeEndDate)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CTemplateLineItemDlg, CNxDialog)
	// (a.walling 2007-11-07 10:33) - PLID 27998 - VS2008 - Errant extra , after dispid 2, does not make any sense. Removed the extra comma.
    //{{AFX_EVENTSINK_MAP(CTemplateLineItemDlg)
	// (d.singleton 2011-10-13 13:50) - PLID 45945 the first two dont even fire, they do nothing
	//ON_EVENT(CTemplateLineItemDlg, IDC_APPLY_DATE_START, 2 /* Change */, OnChangeStartDate, VTS_NONE)
	//ON_EVENT(CTemplateLineItemDlg, IDC_APPLY_DATE_END, 2 /* Change */, OnChangeEndDate, VTS_NONE)
	ON_EVENT(CTemplateLineItemDlg, IDC_START_TIME, 1 /* KillFocus */, OnKillFocusStartTime, VTS_NONE)
	ON_EVENT(CTemplateLineItemDlg, IDC_END_TIME, 1 /* KillFocus */, OnKillFocusEndTime, VTS_NONE)
	ON_EVENT(CTemplateLineItemDlg, IDC_LIST_TEMPLATE_RESOURCES, 18 /* RequeryFinished */, OnRequeryFinishedResourceList, VTS_I2)
	ON_EVENT(CTemplateLineItemDlg, IDC_LIST_TEMPLATE_RESOURCES, 10 /* EditingFinished */, OnEditingFinishedResourceList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CTemplateLineItemDlg, IDC_LIST_EXCEPTION_DATES, 2 /* SelChanged */, OnSelchangedExceptionList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CTemplateLineItemDlg, IDC_LIST_EXCEPTION_DATES, 3 /* DblClickCell */, OnDblClickCellExceptionList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CTemplateLineItemDlg, IDC_RESOURCE_DROPDOWN, 18 /* RequeryFinished */, OnRequeryFinishedResourceDropdown, VTS_I2)
	ON_EVENT(CTemplateLineItemDlg, IDC_RESOURCE_DROPDOWN, 16, OnSelChosenResourceDropdown, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateLineItemDlg message handlers

BOOL CTemplateLineItemDlg::OnInitDialog()
{
	CWaitCursor wc;
	int i;

	CNxDialog::OnInitDialog();

	m_bReady = false;

	// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
	m_btnAddLineItemException.AutoSet(NXB_NEW);
	m_btnEditLineItemException.AutoSet(NXB_MODIFY);
	m_btnRemoveLineItemException.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//TES 6/21/2010 - PLID 39262 - Update the window title to indicate whether these are Resource Availability templates
	if(m_eEditorType == stetLocation) {
		//(e.lally 2010-07-15) PLID 39626 - Renamed to Location Templates
		SetWindowText("Location Template Line Item");
	}
	else if (m_eEditorType == stetCollection) {
		// (z.manning 2014-12-11 10:07) - PLID 64230 - Collection applies
		SetWindowText("Collection Apply");
	}

	// (c.haag 2006-11-02 16:45) - PLID 23336 - Template line items now have their
	// own start and end dates. The start date is now the pivot date.
	//m_applyDate.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
	m_dtpStartDate.SetValue(COleDateTime::GetCurrentTime());
	m_dtpEndDate.SetValue(COleDateTime::GetCurrentTime());

	// (c.haag 2006-12-27 12:13) - PLID 23989 - We now use a datalist for line item exception dates
	m_dlExceptionDates = BindNxDataList2Ctrl(this, IDC_LIST_EXCEPTION_DATES, NULL, false);
		
	m_bReady = false;

	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have the precision templating option.
	if(m_eEditorType == stetLocation) {
		GetDlgItem(IDC_CHECK_BLOCK_APPTS)->EnableWindow(FALSE);
	}

	// (z.manning, 11/14/2006) - We load from the line item no matter what.  If it's new, we use the defaults
	// set in TemplateLineItemInfo's constructor.
	m_nPeriod = m_pLineItem->m_nPeriod;
	m_esScale = m_pLineItem->m_esScale;
	m_embBy = m_pLineItem->m_embBy;
	m_nPatternOrdinal = m_pLineItem->m_nPatternOrdinal;
	m_nDayNumber = m_pLineItem->m_nDayNumber;
	// (c.haag 2006-11-03 10:47) - PLID 23336 - The pivot date field has been superceded by
	// StartDate, and no longer exists.
	//m_dtPivotDate = m_pLineItem->m_dtPivotDate;
	m_nInclude = m_pLineItem->m_nInclude;
	// (c.haag 2006-11-02 16:28) - PLID 23336 - The start and end date fields have moved from
	// the template scope to the template line item scope
	m_dtStartDate = m_pLineItem->m_dtStartDate;
	m_dtEndDate = m_pLineItem->m_dtEndDate;
	
	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-11 10:13) - PLID 64230 - Hide the controls that aren't relevant when editing
		// a collection apply.
		GetDlgItem(IDC_TIME_GROUPBOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_START_TIME_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_START_TIME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_END_TIME_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_END_TIME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_APPLIES_GROUPBOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_ALL_RESOURCES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_SELECT_RESOURCES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LIST_TEMPLATE_RESOURCES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_BLOCK_APPTS)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_RESOURCE_LABEL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_RESOURCE_DROPDOWN)->ShowWindow(SW_SHOW);
		SetDlgItemText(IDC_APPEARANCE_GROUPBOX, "Resource");
		SetDlgItemText(IDOK, "&Apply");

		// (z.manning 2014-12-11 11:01) - PLID 64230 - Rearrange some things to make up for the empty space of the
		// hidden controls;
		CWnd *pwnd = GetDlgItem(IDC_TIME_GROUPBOX);
		CRect rect;
		pwnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		long nRightBorder = rect.right;

		pwnd = GetDlgItem(IDC_DATES_GROUPBOX);
		pwnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right = nRightBorder;
		pwnd->MoveWindow(rect);

		pwnd = GetDlgItem(IDC_PERIOD_FRAME);
		pwnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right = nRightBorder;
		pwnd->MoveWindow(rect);

		m_pdlApplyResource = BindNxDataList2Ctrl(IDC_RESOURCE_DROPDOWN, true);
	}
	else
	{
		m_nxtEnd = BindNxTimeCtrl(this, IDC_END_TIME);
		m_nxtStart = BindNxTimeCtrl(this, IDC_START_TIME);

		// (c.haag 2006-11-06 11:42) - PLID 23336 - Requery the resource list. The list is
		// populated with resource selections later in OnRequeryFinished
		m_dlResources = BindNxDataList2Ctrl(this, IDC_LIST_TEMPLATE_RESOURCES, GetRemoteData(), false);
		// (c.haag 2006-12-06 13:24) - PLID 23336 - Include inactive resources ONLY if they are
		// tied to this item
		CString strWhere;
		if (m_pLineItem && m_pLineItem->m_nID != -1) {
			strWhere.Format("Inactive = 0 OR (Inactive <> 0 AND ID IN (SELECT ResourceID FROM TemplateItemResourceT WHERE TemplateItemID = %d))",
				m_pLineItem->m_nID);
		}
		else {
			strWhere = "Inactive = 0";
		}
		m_dlResources->WhereClause = (LPCTSTR)strWhere;
		m_dlResources->Requery();

		// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
		// converting back and forth to CString, so as to avoid any problems from regional settings.
		m_dtStartTime = m_pLineItem->m_dtStartTime;
		m_dtEndTime = m_pLineItem->m_dtEndTime;
		m_nxtStart->SetDateTime(m_dtStartTime);
		m_nxtEnd->SetDateTime(m_dtEndTime);

		// (c.haag 2006-11-06 11:38) - PLID 23336 - Load in resource information
		if (m_pLineItem->m_bAllResources) {
			CheckDlgButton(IDC_RADIO_ALL_RESOURCES, 1);
			CheckDlgButton(IDC_RADIO_SELECT_RESOURCES, 0);
			GetDlgItem(IDC_LIST_TEMPLATE_RESOURCES)->EnableWindow(FALSE);
		}
		else {
			CheckDlgButton(IDC_RADIO_ALL_RESOURCES, 0);
			CheckDlgButton(IDC_RADIO_SELECT_RESOURCES, 1);
			GetDlgItem(IDC_LIST_TEMPLATE_RESOURCES)->EnableWindow(TRUE);
		}

		// (a.walling 2007-11-07 11:39) - PLID 27998 - VS2008 - for() loops
		NXDATALIST2Lib::IRowSettingsPtr pRow = NULL;
		for (pRow = m_dlResources->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			pRow->PutValue(rlcCheck, COleVariant((short)FALSE, VT_BOOL));
		}
		for (i = 0; i < m_pLineItem->GetResourceCount(); i++) {
			pRow = m_dlResources->FindByColumn(rlcID, (long)m_pLineItem->GetResourceByIndex(i).m_nID, NULL, FALSE);
			if (pRow) {
				pRow->PutValue(rlcCheck, COleVariant((short)TRUE, VT_BOOL));
			}
		}

		// (c.haag 2006-11-20 11:41) - PLID 23605 - Update the appearance of the blocks checkbox
		CheckDlgButton(IDC_CHECK_BLOCK_APPTS, (m_pLineItem->m_bIsBlock) ? 1 : 0 );

		// (z.manning 2015-01-02 14:13) - PLID 64295 - Need to disable most controls if editing a
		// line item that is part of a collection apply.
		if (m_pLineItem->m_nCollectionID != -1)
		{
			GetDlgItem(IDC_APPLY_DATE_START)->EnableWindow(FALSE);
			GetDlgItem(IDC_APPLY_DATE_END)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_TO_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_ALL_RESOURCES)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_SELECT_RESOURCES)->EnableWindow(FALSE);
			m_dlResources->PutReadOnly(VARIANT_TRUE);
			GetDlgItem(IDC_SCALE_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BY_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_PATTERN_ORDINAL_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_DAY_NUMBER_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_DAY_NUMBER_SPIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_EVERY_1_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_EVERY_2_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_EVERY_X_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_EVERY_X_TEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_MONDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_TUESDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_WEDNESDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_THURSDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_FRIDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_SATURDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_SUNDAY_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_BLOCK_APPTS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RESOURCE_DROPDOWN)->EnableWindow(FALSE);
		}
	}

	// (c.haag 2006-11-20 10:09) - PLID 23589 - Load in the exception dates
	for (i=0; i < m_pLineItem->m_arydtExceptionDates.GetSize(); i++) {
		COleDateTime dt = m_pLineItem->m_arydtExceptionDates[i];
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlExceptionDates->GetNewRow();
		pRow->Value[elcDate] = ToDateVariant(dt);
		m_dlExceptionDates->AddRowAtEnd(pRow, NULL);
	}
	m_dlExceptionDates->Sort();

	((CSpinButtonCtrl *)GetDlgItem(IDC_DAY_NUMBER_SPIN))->SetRange(1, 31);

	// Set one-time defaults
	LoadCurrentState();
	
	// (c.haag 2003-09-03 16:38) - Make the day names international compliant
	CString strText;
	strText = GetDayText(1, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_MONDAY_CHECK, strText);
	strText = GetDayText(2, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_TUESDAY_CHECK, strText);
	strText = GetDayText(3, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_WEDNESDAY_CHECK, strText);
	strText = GetDayText(4, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_THURSDAY_CHECK, strText);
	strText = GetDayText(5, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_FRIDAY_CHECK, strText);
	strText = GetDayText(6, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_SATURDAY_CHECK, strText);
	strText = GetDayText(0, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_SUNDAY_CHECK, strText);

	m_bReady = true;
	ApplyChanges(m_pLineItem->IsNew());

	// Update the button appearances
	ReflectCurrentStateOnBtns();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTemplateLineItemDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CTemplateLineItemDlg::LoadCurrentState()
{
	// Set current period
	
	switch (m_nPeriod) {
	case 1:
		CheckRadioButton(IDC_EVERY_1_RADIO, IDC_EVERY_X_RADIO, IDC_EVERY_1_RADIO);
		SetDlgItemText(IDC_EVERY_X_TEXT, "3");
		break;
	case 2:
		CheckRadioButton(IDC_EVERY_1_RADIO, IDC_EVERY_X_RADIO, IDC_EVERY_2_RADIO);
		SetDlgItemText(IDC_EVERY_X_TEXT, "3");
		break;
	case 3:
	default:
		{
			CString strPeriod;
			strPeriod.Format("%i", m_nPeriod);
			CheckRadioButton(IDC_EVERY_1_RADIO, IDC_EVERY_X_RADIO, IDC_EVERY_X_RADIO);
			SetDlgItemText(IDC_EVERY_X_TEXT, strPeriod);
		}
		break;
	}

	// Set current scale combo
	((CComboBox *)GetDlgItem(IDC_SCALE_COMBO))->SetCurSel((int)(m_esScale) - 1);

	// Set current "month by" combo
	((CComboBox *)GetDlgItem(IDC_BY_COMBO))->SetCurSel((int)(m_embBy) - 1);

	// Set current pattern ordinal
	if(m_nPatternOrdinal == -1) 
		((CComboBox *)GetDlgItem(IDC_PATTERN_ORDINAL_COMBO))->SetCurSel(5);
	else 
		((CComboBox *)GetDlgItem(IDC_PATTERN_ORDINAL_COMBO))->SetCurSel(m_nPatternOrdinal - 1);

	if (m_eEditorType != stetCollection)
	{
		// Set start and end times
		// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
		// converting back and forth to CString, so as to avoid any problems from regional settings.
		m_nxtStart->SetDateTime(m_dtStartTime);
		m_nxtEnd->SetDateTime(m_dtEndTime);
	}

	// Set day number
	SetDlgItemInt(IDC_DAY_NUMBER_EDIT, m_nDayNumber);

	// Set pivot date
	// (c.haag 2006-11-03 10:54) - PLID 23336 - The pivot date combo has been
	// superceded by the start date combo
//	((CDateCombo *)GetDlgItem(IDC_PIVOT_DATE_COMBO))->SetDate(m_dtPivotDate.Format("%x"));
	//m_applyDate.SetValue((COleVariant)m_dtPivotDate);
	//
	// (c.haag 2006-11-02 16:45) - PLID 23336 - Template line items now have their
	// own start and end dates. The end date may be null, so we need to be careful.
	//
	m_dtpStartDate.SetValue(m_dtStartDate);
	if (COleDateTime::valid != m_dtEndDate.GetStatus()) {
		m_dtpEndDate.SetValue(COleDateTime::GetCurrentTime());
		GetDlgItem(IDC_APPLY_DATE_END)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_FOREVER)->ShowWindow(SW_SHOW);
		CheckDlgButton(IDC_CHECK_TO_DATE, 0);
	} else {
		m_dtpEndDate.SetValue(m_dtEndDate);
		GetDlgItem(IDC_APPLY_DATE_END)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_FOREVER)->ShowWindow(SW_HIDE);
		CheckDlgButton(IDC_CHECK_TO_DATE, 1);
	}

	// Set include days	
	SetDayCheck(m_nInclude, 1);

// originals
//	CheckRadioButton(IDC_EVERY_1_RADIO, IDC_EVERY_X_RADIO, IDC_EVERY_1_RADIO);
//	((CComboBox *)GetDlgItem(IDC_SCALE_COMBO))->SetCurSel(1);
//	((CComboBox *)GetDlgItem(IDC_BY_COMBO))->SetCurSel(0);
//	((CComboBox *)GetDlgItem(IDC_PATTERN_ORDINAL_COMBO))->SetCurSel(0);
//	((CComboBox *)GetDlgItem(IDC_START_TIME_COMBO))->SelectString(-1, "8:00a");
//	((CComboBox *)GetDlgItem(IDC_END_TIME_COMBO))->SelectString(-1, "5:00p");
//	SetDlgItemText(IDC_EVERY_X_TEXT, "3");
//	SetDlgItemText(IDC_DAY_NUMBER_EDIT, "1");
//	((CSpinButtonCtrl *)GetDlgItem(IDC_DAY_NUMBER_SPIN))->SetRange(1, 31);
//
}

bool CTemplateLineItemDlg::Validate()
{
	//
	// (c.haag 2006-11-09 10:01) - PLID 23336 - This function used to be SaveCurrentState(), and
	// used to pull data from the dialog form into our member variables and do validation. Now, 
	// EnsureDialogData pulls the data, and this function only does validation
	//
	if(m_nPeriod <= 0) {
		AfxMessageBox("Invalid number of days entered, please enter a non-zero, non-negative number.");
		return false;	//failure to save
	}
	if (sMonthly == m_esScale && mbNone == m_embBy) {
		AfxMessageBox("You must specify whether your monthly scale is by pattern or by date");
		return false;
	}
	if (!sMonthly == m_esScale && mbDate == m_embBy && m_nDayNumber <= 0) {
		AfxMessageBox("The day of month field is invalid. Please correct this before saving.");
		return false;
	}

	// (z.manning 2014-12-11 13:47) - PLID 64230 - Times aren't relevant when editing collection applies
	if (m_eEditorType != stetCollection)
	{
		if (m_dtStartTime > m_dtEndTime) {
			AfxMessageBox("The start time is after the end time. Please correct this before saving.");
			return false;
		}
		else if (m_dtStartTime == m_dtEndTime) {
			AfxMessageBox("The start time is the same as the end time.  Please adjust one of these.");
			return false;
		}
	}

	// (c.haag 2006-11-09 11:50) - PLID 23336 - Not sure if this can ever happen, but I'm
	// checking anyway
	if (COleDateTime::valid != m_dtStartDate.GetStatus()) {
		AfxMessageBox("The start date is invalid. Please correct this before saving.");
		return false;
	}

	// If we're doing a range for the once scale, make sure end date >= pivot date
	// (c.haag 2006-11-08 09:57) - PLID 23336 - The previous action has been superceeded by the
	// new design. It is OK for the dates to be the same, but not to have a start date after an
	// end date.
	if (COleDateTime::valid == m_dtEndDate.GetStatus() && m_dtStartDate > m_dtEndDate) {
		MsgBox(MB_ICONWARNING | MB_OK, "The start date is after the end date.\r\n\r\nPlease fix this before saving.");
		return false;
	}

	// (c.haag 2006-11-14 11:43) - PLID 23336 - If the weekday checkboxes are necessary, make
	// sure that at least one is checked
	if (sOnce != m_esScale && sYearly != m_esScale && !m_nInclude) {
		MsgBox(MB_ICONWARNING | MB_OK, "You do not have any weekdays selected.\r\n\r\nPlease fix this before saving.");
		return false;
	}

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-11 12:35) - PLID 64230 - We have a separate resource combo when editing collections
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlApplyResource->GetCurSel();
		if (pRow == NULL) {
			MessageBox("You must select a resource.", NULL, MB_OK | MB_ICONWARNING);
			return false;
		}
	}
	else
	{
		// (c.haag 2006-11-14 11:43) - PLID 23336 - Make sure at least one resource is selected
		if (IDC_RADIO_SELECT_RESOURCES == GetCheckedRadioButton(IDC_RADIO_ALL_RESOURCES, IDC_RADIO_SELECT_RESOURCES)) {
			NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_dlResources->GetFirstRow();
			BOOL bFound = FALSE;
			while (NULL != pCurRow && !bFound) {
				if (VarBool(pCurRow->GetValue(rlcCheck))) {
					bFound = TRUE;
				}
				pCurRow = pCurRow->GetNextRow();
			}
			if (!bFound) {
				MsgBox(MB_ICONWARNING | MB_OK, "You do not have any resources selected.\r\n\r\nPlease fix this before saving.");
				return false;
			}
		}
	}

	// (c.haag 2006-11-20 09:56) - PLID 23589 - Fail if any of the date exceptions fall outside the date range
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlExceptionDates->GetFirstRow();
	while (NULL != pRow) {
		COleDateTime dt = VarDateTime(pRow->GetValue(elcDate));
		if (dt < m_dtStartDate) {
			MsgBox(MB_ICONWARNING | MB_OK, "You are attempting to exclude a date that occurs before the From date.\r\n\r\nPlease fix this before saving.");
			return false;
		} else if (COleDateTime::valid == m_dtEndDate.GetStatus() && dt > m_dtEndDate) {
			MsgBox(MB_ICONWARNING | MB_OK, "You are attempting to exclude a date that occurs after the To date.\r\n\r\nPlease fix this before saving.");
			return false;
		}
		pRow = pRow->GetNextRow();
	}

	// (c.haag 2006-11-28 17:48) - PLID 23589 - Make sure the day of month field is set
	if (sMonthly == m_esScale && mbDate == m_embBy && !(m_nDayNumber > 0 && m_nDayNumber < 32)) {
		MsgBox(MB_ICONWARNING | MB_OK, "Please fill in a valid day of month.");
		return false;
	}

	return true;
}

void CTemplateLineItemDlg::ApplyChanges(BOOL bForceRefresh /* = TRUE */)
{
	if (m_bReady) {
		// Re-ensure data
		EnsureDialogData(bForceRefresh);
		
		// Show and hide areas of the screen appropriately
		ShowScaleChanges();
		ShowPeriodChanges();
		ShowIncludeChanges();
		
		// Show textual description of current selection
		DisplayResults();
	}
}

void CTemplateLineItemDlg::OnSelchangeScaleCombo() 
{
	ApplyChanges();

	// (z.manning, 12/20/05, PLID 6595)
	// If the scale is now once, let's make sure the end date is at least the start date.
	//if(VarDateTime(m_dtpEndDate.GetValue()) < m_dtPivotDate) {
	//	m_dtpEndDate.SetValue((COleVariant)m_dtPivotDate);
	//}

	// (c.haag 2006-11-09 11:52) - PLID 23336 - The pivot date is now the start date
	// (a.walling 2008-05-15 14:17) - PLID 27591 - Using a variant return type again...
	if (VarDateTime(m_dtpEndDate.GetValue()) < m_dtStartDate) {
		m_dtpEndDate.SetValue(m_dtStartDate);
	}
}

void CTemplateLineItemDlg::ShowScaleChanges()
{
	EnsureDialogData();

	//
	// (c.haag 2006-11-02 16:55) - PLID 23336 - The apply date (pivot date)
	// labels have been depreciated. IDC_APPLY_DATE_START and IDC_APPLY_DATE_END
	// are now the start and end dates of the line item. There is also no longer
	// a need to hide or move any date fields. However, here we will enable and
	// disable the start and end date fields depending on the state of their
	// respective "enabled" checkboxes.
	//
	GetDlgItem(IDC_APPLY_DATE_END)->ShowWindow( IsDlgButtonChecked(IDC_CHECK_TO_DATE) ? SW_SHOW : SW_HIDE );
	GetDlgItem(IDC_STATIC_FOREVER)->ShowWindow( IsDlgButtonChecked(IDC_CHECK_TO_DATE) ? SW_HIDE : SW_SHOW );

	//GetDlgItem(IDC_APPLY_DATE)->MoveWindow(m_rectApplyDate);
	//GetDlgItem(IDC_APPLY_DATE_LABEL)->MoveWindow(m_rectApplyDateLabel);
	//SetDlgItemText(IDC_APPLY_DATE_LABEL, "Apply date:");
	//GetDlgItem(IDC_APPLY_DATE_END)->ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_APPLY_DATE_END_LABEL)->ShowWindow(SW_HIDE);
	switch (m_esScale) {
	case sOnce:
		GetDlgItem(IDC_BY_COMBO)->ShowWindow(FALSE);
		GetDlgItem(IDC_BY_LABEL)->ShowWindow(FALSE);
		GetDlgItem(IDC_PATTERN_ORDINAL_COMBO)->ShowWindow(FALSE);
		ShowFrame(IDC_BY_DATE_FRAME, FALSE);
		ShowFrame(IDC_SCOPE_FRAME, FALSE);
		ShowFrame(IDC_PERIOD_FRAME, FALSE);
		ShowFrame(IDC_INCLUDE_FRAME, FALSE);
		//GetDlgItem(IDC_APPLY_DATE)->MoveWindow(m_rectApplyDate.left - 85, m_rectApplyDate.top, m_rectApplyDate.Width(), m_rectApplyDate.Height());
		//GetDlgItem(IDC_APPLY_DATE_LABEL)->MoveWindow(m_rectApplyDateLabel.left - 85, m_rectApplyDateLabel.top, m_rectApplyDateLabel.Width(), m_rectApplyDateLabel.Height());
		//SetDlgItemText(IDC_APPLY_DATE_LABEL, "Start date:");
		//GetDlgItem(IDC_APPLY_DATE_END)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_APPLY_DATE_END_LABEL)->ShowWindow(SW_SHOW);
		break;
	case sMonthly:
		GetDlgItem(IDC_BY_COMBO)->ShowWindow(TRUE);
		GetDlgItem(IDC_BY_LABEL)->ShowWindow(TRUE);
		GetDlgItem(IDC_PATTERN_ORDINAL_COMBO)->ShowWindow(m_embBy == mbPattern);
		ShowFrame(IDC_BY_DATE_FRAME, m_embBy == mbDate);
		ShowFrame(IDC_SCOPE_FRAME, TRUE);
		ShowFrame(IDC_PERIOD_FRAME, TRUE);
		ShowFrame(IDC_INCLUDE_FRAME, m_embBy == mbPattern);
		break;
	case sYearly:
		GetDlgItem(IDC_BY_COMBO)->ShowWindow(FALSE);
		GetDlgItem(IDC_BY_LABEL)->ShowWindow(FALSE);
		GetDlgItem(IDC_PATTERN_ORDINAL_COMBO)->ShowWindow(FALSE);
		ShowFrame(IDC_BY_DATE_FRAME, FALSE);
		ShowFrame(IDC_SCOPE_FRAME, TRUE);
		ShowFrame(IDC_PERIOD_FRAME, TRUE);
		ShowFrame(IDC_INCLUDE_FRAME, FALSE);
		break;
	default:
		GetDlgItem(IDC_BY_COMBO)->ShowWindow(FALSE);
		GetDlgItem(IDC_BY_LABEL)->ShowWindow(FALSE);
		GetDlgItem(IDC_PATTERN_ORDINAL_COMBO)->ShowWindow(FALSE);
		ShowFrame(IDC_BY_DATE_FRAME, FALSE);
		ShowFrame(IDC_SCOPE_FRAME, TRUE);
		ShowFrame(IDC_PERIOD_FRAME, TRUE);
		ShowFrame(IDC_INCLUDE_FRAME, TRUE);
		break;
	}

	// Change the scale text accordingly
	CString strScaleWord;
	strScaleWord = GetScaleWord();
	SetDlgItemText(IDC_EVERY_1_RADIO, "&Every " + strScaleWord);
	SetDlgItemText(IDC_EVERY_2_RADIO, "Every &Other " + strScaleWord);
	SetDlgItemText(IDC_EVERY_X_STATIC, GetScaleWord(true, false));
}

void CTemplateLineItemDlg::ShowPeriodChanges()
{
	if (IsDlgButtonChecked(IDC_EVERY_X_RADIO) == 1) {
		GetDlgItem(IDC_EVERY_X_TEXT)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_EVERY_X_TEXT)->EnableWindow(FALSE);
	}
}

void CTemplateLineItemDlg::ShowIncludeChanges()
{
}

void CTemplateLineItemDlg::OnEveryXRadio() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnEvery2Radio() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnEvery1Radio() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::ShowFrame(int nFrameID, BOOL bShow)
{
	switch (nFrameID) {
	case IDC_PERIOD_FRAME:
		//GetDlgItem(IDC_PERIOD_FRAME)->ShowWindow(bShow);
		GetDlgItem(IDC_EVERY_1_RADIO)->ShowWindow(bShow);
		GetDlgItem(IDC_EVERY_2_RADIO)->ShowWindow(bShow);
		GetDlgItem(IDC_EVERY_X_RADIO)->ShowWindow(bShow);
		GetDlgItem(IDC_EVERY_X_TEXT)->ShowWindow(bShow);
		GetDlgItem(IDC_EVERY_X_STATIC)->ShowWindow(bShow);
		ShowPeriodChanges();
		break;
	case IDC_BY_DATE_FRAME:
		GetDlgItem(IDC_BY_DATE_FRAME)->ShowWindow(bShow);
		GetDlgItem(IDC_DAY_NUMBER_EDIT)->ShowWindow(bShow);
		GetDlgItem(IDC_DAY_NUMBER_LABEL)->ShowWindow(bShow);
		GetDlgItem(IDC_DAY_NUMBER_SPIN)->ShowWindow(bShow);
		break;
	case IDC_INCLUDE_FRAME:
		GetDlgItem(IDC_INCLUDE_FRAME)->ShowWindow(bShow);
		GetDlgItem(IDC_MONDAY_CHECK)->ShowWindow(bShow);
		GetDlgItem(IDC_TUESDAY_CHECK)->ShowWindow(bShow);
		GetDlgItem(IDC_WEDNESDAY_CHECK)->ShowWindow(bShow);
		GetDlgItem(IDC_THURSDAY_CHECK)->ShowWindow(bShow);
		GetDlgItem(IDC_FRIDAY_CHECK)->ShowWindow(bShow);
		GetDlgItem(IDC_SATURDAY_CHECK)->ShowWindow(bShow);
		GetDlgItem(IDC_SUNDAY_CHECK)->ShowWindow(bShow);
		ShowIncludeChanges();
		break;
	}

}

void CTemplateLineItemDlg::OnSelchangeByCombo() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnMondayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnTuesdayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnWednesdayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnThursdayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnSundayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnFridayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnSaturdayCheck() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::DisplayResults()
{
	//
	// (c.haag 2006-11-08 16:32) - PLID 23336 - Create a temporary line item object, and
	// populate it with content from this dialog
	//
	CTemplateLineItemInfo temp;
	WriteContentToLineItem(&temp);
	m_strResults = temp.GetApparentDescription(m_eEditorType);
	UpdateData(SEND_TO_DIALOG);

/*	bool bNormalCase = true;

	m_strResults = "";
	m_strResults += GetPreTimeRangeText(bNormalCase);
	m_strResults += GetPreIncludeText(bNormalCase);
	m_strResults += GetPeriodText(bNormalCase);
	m_strResults += GetPostIncludeText(bNormalCase);
	m_strResults += GetPostTimeRangeText(bNormalCase);
	m_strResults += GetDateRangeText(bNormalCase);

	//we're going to re-word things now based on various cases
	//remove the "on every day" if we are specifying non-excluded days
	if((m_strResults.Find("on every day") != -1 || m_strResults.Find("on Every day") != -1)
		&& m_strResults.Find("excluding") == -1) {
		m_strResults.Replace(" on every day","");
		m_strResults.Replace(" on Every day","");
	}
	//remove "excluding none" - it's not needed
	if(m_strResults.Find("excluding none") != -1) {
		m_strResults.Replace(" excluding none","");
	}
	
	UpdateData(SEND_TO_DIALOG);*/
}

CString CTemplateLineItemDlg::GetScaleWord(bool bNormalCase /* = true */, bool bSingular /* = true */)
{
	CString strAns;

	switch (m_esScale) {
	case sDaily:
		strAns = "Day";
		break;
	case sWeekly:
		strAns = "Week";
		break;
	case sMonthly:
		strAns = "Month";
		break;
	case sYearly:
		strAns = "Year";
		break;
	default:
		strAns = "";
		break;
	}

	if (!strAns.IsEmpty()) {
		if (!bNormalCase) {
			strAns.MakeLower();
		}
		
		if (!bSingular) {
			strAns += 's';
		}
	}

	return strAns;
}

void CTemplateLineItemDlg::GetDays(int &nDays, int &nDayCnt, UINT nCheckVal /* = 1 */)
{
	if ((m_esScale == sOnce) || (m_esScale == sYearly) || ((m_esScale == sMonthly) && (m_embBy == mbDate))) {
		nDays = dowEveryday;
		nDayCnt = 7;
		return;
	}
	nDays = 0;
	nDayCnt = 0;
	if (IsDlgButtonChecked(IDC_MONDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowMonday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_TUESDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowTuesday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_WEDNESDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowWednesday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_THURSDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowThursday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_FRIDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowFriday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_SATURDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowSaturday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_SUNDAY_CHECK) == nCheckVal) {
		nDays = nDays | dowSunday;
		nDayCnt++;
	}
}

void CTemplateLineItemDlg::GetDaysEx(int &nDays, int &nDayCnt, UINT& nIncluded)
{
	if ((m_esScale == sOnce) || (m_esScale == sYearly) || ((m_esScale == sMonthly) && (m_embBy == mbDate))) {
		nDays = dowEveryday;
		nDayCnt = 7;
		nIncluded = 1;
		return;
	}
	nDays = 0;
	nDayCnt = 0;
	if (IsDlgButtonChecked(IDC_MONDAY_CHECK)) {
		nDays = nDays | dowMonday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_TUESDAY_CHECK)) {
		nDays = nDays | dowTuesday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_WEDNESDAY_CHECK)) {
		nDays = nDays | dowWednesday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_THURSDAY_CHECK)) {
		nDays = nDays | dowThursday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_FRIDAY_CHECK)) {
		nDays = nDays | dowFriday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_SATURDAY_CHECK)) {
		nDays = nDays | dowSaturday;
		nDayCnt++;
	}
	if (IsDlgButtonChecked(IDC_SUNDAY_CHECK)) {
		nDays = nDays | dowSunday;
		nDayCnt++;
	}

	if (nDayCnt > 3)
	{
		nDays ^= (UINT)-1;
		nDayCnt = 7 - nDayCnt;
		nIncluded = 0;
	}
	else
		nIncluded = 1;
}

void CTemplateLineItemDlg::OnChangeDayNumberEdit() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::EnsureDialogData(BOOL bFullEnsure /* = FALSE */)
{
	if (!m_bGotDialogData || bFullEnsure) {
		CString strScale;
		GetDlgItemText(IDC_SCALE_COMBO, strScale);
		
		// Categorize period
		// (z.manning, 11/28/2006) - PLID 23675 - I entered 23675 to change this to a datalist.
		if (strScale.CompareNoCase("Once") == 0) {
			m_esScale = sOnce;
			m_embBy = mbNone;
		} else if (strScale.CompareNoCase("Daily") == 0) {
			m_esScale = sDaily;
			m_embBy = mbNone;
		} else if (strScale.CompareNoCase("Weekly") == 0) {
			m_esScale = sWeekly;
			m_embBy = mbNone;
		} else if (strScale.CompareNoCase("Monthly") == 0) {
			m_esScale = sMonthly;
			CString strBy;
			GetDlgItemText(IDC_BY_COMBO, strBy);
			// Categorize "by" field
			if (strBy.CompareNoCase("Date") == 0) {
				m_embBy = mbDate;
			} else if (strBy.CompareNoCase("Pattern") == 0) {
				m_embBy = mbPattern;
			} else {
				// (c.haag 2006-11-14 12:35) - PLID 23544 - If we get here, the "by"
				// combo has not been set. So, we need to assign it a default value.
				m_embBy = mbPattern;
				((CComboBox *)GetDlgItem(IDC_BY_COMBO))->SetCurSel((int)(m_embBy) - 1);
			}
		} else if (strScale.CompareNoCase("Yearly") == 0) {
			m_esScale = sYearly;
			m_embBy = mbNone;
		} else {
			m_esScale = sNone;
			m_embBy = mbNone;
		}

		if (m_eEditorType != stetCollection)
		{
			//
			// (c.haag 2006-11-09 09:51) - PLID 23336 - Update the start and end times
			//
			COleDateTime dt;
			dt = m_nxtStart->GetDateTime();
			m_dtStartTime.SetTime(dt.GetHour(), dt.GetMinute(), 0);
			dt = m_nxtEnd->GetDateTime();
			m_dtEndTime.SetTime(dt.GetHour(), dt.GetMinute(), 0);
		}

		// (c.haag 2006-11-02 16:45) - PLID 23336 - Template line items now have their
		// own start and end dates. The apply date combo has been superceded by the
		// start date combo.
		//m_dtPivotDate = m_applyDate.GetValue();
		m_dtStartDate = m_dtpStartDate.GetValue();
		m_dtEndDate = m_dtpEndDate.GetValue();
		if (IsDlgButtonChecked(IDC_CHECK_TO_DATE)) {
			m_dtEndDate = m_dtpEndDate.GetValue();
		} else {
			m_dtEndDate.SetStatus(COleDateTime::invalid);
		}

		// Get current period
		switch (GetCheckedRadioButton(IDC_EVERY_1_RADIO, IDC_EVERY_X_RADIO)) {
		case IDC_EVERY_X_RADIO: {
			CString strPeriod;
			GetDlgItemText(IDC_EVERY_X_TEXT, strPeriod);
			m_nPeriod = atoi(strPeriod);
			} break;
		case IDC_EVERY_2_RADIO:
			m_nPeriod = 2;
			break;
		case IDC_EVERY_1_RADIO:
		default:
			m_nPeriod = 1;
			break;
		}

		// Get current pattern ordinal
		m_nPatternOrdinal = ((CComboBox *)GetDlgItem(IDC_PATTERN_ORDINAL_COMBO))->GetCurSel() + 1;
		if(m_nPatternOrdinal == 6) m_nPatternOrdinal = -1;

		// Get day number
		m_nDayNumber = GetDlgItemInt(IDC_DAY_NUMBER_EDIT);

		// Get include days	
		int nDays, nDaysCnt;
		GetDays(nDays, nDaysCnt);
		m_nInclude = nDays;


		// Success!
		m_bGotDialogData = true;
	}
}

void CTemplateLineItemDlg::SetDayCheck(int nDay, int nCheckVal /* = 1 */)
{
	if (nDay & dowSunday) {
		CheckDlgButton(IDC_SUNDAY_CHECK, nCheckVal);
	}
	if (nDay & dowMonday) {
		CheckDlgButton(IDC_MONDAY_CHECK, nCheckVal);
	}
	if (nDay & dowTuesday) {
		CheckDlgButton(IDC_TUESDAY_CHECK, nCheckVal);
	}
	if (nDay & dowWednesday) {
		CheckDlgButton(IDC_WEDNESDAY_CHECK, nCheckVal);
	}
	if (nDay & dowThursday) {
		CheckDlgButton(IDC_THURSDAY_CHECK, nCheckVal);
	}
	if (nDay & dowFriday) {
		CheckDlgButton(IDC_FRIDAY_CHECK, nCheckVal);
	}
	if (nDay & dowSaturday) {
		CheckDlgButton(IDC_SATURDAY_CHECK, nCheckVal);
	}
}

void CTemplateLineItemDlg::OnSelchangeStartTimeCombo() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnSelchangeEndTimeCombo() 
{
	ApplyChanges();
}

void CTemplateLineItemDlg::OnOK() 
{
	// Validate dialog
//	// If we are on a weekly scale, we need to make sure 
//	// that the day of the week of the pivot date is checked
//	EnsureDialogData();
//	if (m_esScale == sWeekly) {
//		SetDayCheck(m_dtPivotDate.GetDayOfWeek() - 1, 1);
//	}
	if (m_pLineItem) {
		// (c.haag 2006-11-06 13:57) - PLID 23336 - Don't do anything with resources here
		// because we have no member variables for resource values; they are strictly in
		// controls. They are handled in OnOK.
		ApplyChanges();

		// (c.haag 2006-11-09 10:01) - PLID 23336 - Write the dialog form states to our
		// member variables.
		EnsureDialogData();

		if(!Validate())
			return;	//quit if the save failed

		//
		// (c.haag 2006-11-08 16:23) - PLID 23336 - We now write to m_pLineItem in this function
		//
		WriteContentToLineItem(m_pLineItem);

		if (!((CompareTimes(m_pLineItem->m_dtStartTime, m_pOriginalLineItem->m_dtStartTime) & CT_EQUAL)
			&& (CompareTimes(m_pLineItem->m_dtEndTime, m_pOriginalLineItem->m_dtEndTime) & CT_EQUAL)))
		{
			// (z.manning 2015-01-06 09:52) - PLID 64521 - If they changed the time then make sure that
			// we clear out the collection ID.
			m_pLineItem->m_nCollectionID = -1;
		}
	}
	
	CDialog::OnOK();
}

void CTemplateLineItemDlg::WriteContentToLineItem(CTemplateLineItemInfo *pLineItem)
{
	//
	// (c.haag 2006-11-08 16:22) - PLID 23336 - This function basically does what
	// OnOK used to do; write all of the dialog content to a line item object. The
	// reason we are self-contained is so we can write the data to a temporary
	// line item info object to calculate the results text at the bottom of the dialog.
	//
	if (!pLineItem) {
		ASSERT(FALSE);
		return;
	}

	pLineItem->m_nPeriod = m_nPeriod;
	pLineItem->m_esScale = m_esScale;
	pLineItem->m_embBy = m_embBy;
	pLineItem->m_nPatternOrdinal = m_nPatternOrdinal;
	pLineItem->m_nDayNumber = m_nDayNumber;
	// (c.haag 2006-11-03 10:47) - PLID 23336 - The pivot date field has been superceded by
	// StartDate, and no longer exists.
	//pLineItem->m_dtPivotDate = m_dtPivotDate;
	// (c.haag 2006-11-02 17:34) - PLID 23336 - Template line items now have start dates
	m_dtStartDate.SetDate(m_dtStartDate.GetYear(), m_dtStartDate.GetMonth(), m_dtStartDate.GetDay());
	pLineItem->m_dtStartDate = m_dtStartDate;
	m_dtEndDate.SetDate(m_dtEndDate.GetYear(), m_dtEndDate.GetMonth(), m_dtEndDate.GetDay());
	pLineItem->m_dtEndDate = m_dtEndDate;
	pLineItem->m_nInclude = m_nInclude;
	// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
	// converting back and forth to CString, so as to avoid any problems from regional settings.
	m_dtStartTime.SetTime(m_dtStartTime.GetHour(), m_dtStartTime.GetMinute(), m_dtStartTime.GetSecond());
	pLineItem->m_dtStartTime = m_dtStartTime;
	m_dtEndTime.SetTime(m_dtEndTime.GetHour(), m_dtEndTime.GetMinute(), m_dtEndTime.GetSecond());
	pLineItem->m_dtEndTime = m_dtEndTime;
	//
	// (c.haag 2006-11-20 11:43) - PLID 23605 - Save the block appearance toggle
	//
	pLineItem->m_bIsBlock = IsDlgButtonChecked(IDC_CHECK_BLOCK_APPTS) ? TRUE : FALSE;

	if (m_eEditorType == stetCollection)
	{
		if (m_pdlApplyResource->IsRequerying()) {
			// (z.manning 2014-12-16 15:09) - PLID 64232 - Must be the intial load in which case the line item
			// resource is already correct
			pLineItem->m_bAllResources = m_pLineItem->m_bAllResources;
			pLineItem->RemoveAllResources();
			for (int nResourceIndex = 0; nResourceIndex < m_pLineItem->GetResourceCount(); nResourceIndex++) {
				Resource resource = m_pLineItem->GetResourceByIndex(nResourceIndex);
				pLineItem->AddResource(resource.m_nID, resource.m_strName);
			}
		}
		else
		{
			// (z.manning 2014-12-11 12:36) - PLID 64230 - Handle collection resource separately
			pLineItem->m_bAllResources = FALSE;
			pLineItem->RemoveAllResources();
			m_pdlApplyResource->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlApplyResource->GetCurSel();
			if (pRow != NULL) {
				pLineItem->AddResource(VarLong(pRow->GetValue(arcID)), VarString(pRow->GetValue(arcName), ""));
			}
		}
	}
	else
	{
		//
		// (c.haag 2006-11-06 13:50) - PLID 23336 - Save the resources
		//
		switch (GetCheckedRadioButton(IDC_RADIO_ALL_RESOURCES, IDC_RADIO_SELECT_RESOURCES)) {
		case IDC_RADIO_ALL_RESOURCES:
			pLineItem->m_bAllResources = TRUE;
			break;
		case IDC_RADIO_SELECT_RESOURCES:
			pLineItem->m_bAllResources = FALSE;
		}
		m_dlResources->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_dlResources->GetFirstRow();
		pLineItem->RemoveAllResources();
		while (NULL != pCurRow) {
			if (VarBool(pCurRow->GetValue(rlcCheck))) {
				// (z.manning 2011-12-07 11:40) - PLID 46910 - Pass in the resource name too
				pLineItem->AddResource(VarLong(pCurRow->GetValue(rlcID)), VarString(pCurRow->GetValue(rlcName), ""));
			}
			pCurRow = pCurRow->GetNextRow();
		}
	}

	//
	// (c.haag 2006-11-20 09:48) - PLID 23589 - Save the exception dates
	//
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlExceptionDates->GetFirstRow();
	pLineItem->m_arydtExceptionDates.RemoveAll();
	while (NULL != pRow) {
		pLineItem->m_arydtExceptionDates.Add( VarDateTime(pRow->GetValue(elcDate)) );
		pRow = pRow->GetNextRow();
	}
}

// Shows dialog for the given line item
long CTemplateLineItemDlg::ZoomLineItem(CTemplateLineItemInfo *pLineItem)
{
	// Initialize the dialog
	m_bReady = false;
	m_bGotDialogData = false;
	m_pLineItem = pLineItem;

	// We need to recycle the memory of the previous lineitem 
	if (m_pOriginalLineItem != nullptr )
	{
		delete m_pOriginalLineItem;
	}
	// (z.manning 2015-01-06 09:45) - PLID 64521 - Set the original line item
	m_pOriginalLineItem = new CTemplateLineItemInfo;
	*m_pOriginalLineItem = *pLineItem;

	// Run the dialog
	return DoModal();
}

// (z.manning 2014-12-11 12:28) - PLID 64230
int CTemplateLineItemDlg::ZoomCollectionApply(CTemplateLineItemInfo *pLineItem)
{
	return ZoomLineItem(pLineItem);
}

void CTemplateLineItemDlg::OnSelchangePatternOrdinalCombo() 
{
	try
	{
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnChangeEveryXText() 
{
	try
	{
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

// (d.singleton 2011-11-02 14:04) - PLID removed since they are never used.
//void CTemplateLineItemDlg::OnChangeStartDate() 
//{
//	COleDateTime dtStartTime;
//	m_dtpStartDate.GetTime(dtStartTime);
//	m_dtpEndDate.SetTime(dtStartTime);
//	ApplyChanges();	
//}

//void CTemplateLineItemDlg::OnChangeEndDate() 
//{
//	ApplyChanges();	
//}

void CTemplateLineItemDlg::OnKillFocusStartTime() 
{
	try
	{
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnKillFocusEndTime() 
{
	try
	{
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnToDateCheck()
{
	try
	{
		// (c.haag 2006-11-03 11:34) - PLID 23336 - Users may toggle the use
		// of an end date. If the checkbox is unchecked, then the end date
		// field is ignored, and the actual date is treated as "forever in the future"
		if (IsDlgButtonChecked(IDC_CHECK_TO_DATE)) {
			m_dtEndDate.SetStatus(COleDateTime::valid);
		}
		else {
			m_dtEndDate.SetStatus(COleDateTime::invalid);
		}
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnRequeryFinishedResourceList(short nFlags)
{
	try
	{
		//
		// (c.haag 2006-11-06 12:08) - PLID 23336 - If we are loading an existing line item,
		// we need to populate the checkbox column. Make sure all checkbox values are of
		// type VT_BOOL, whether or not they are checked.
		//
		if (m_pLineItem) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlResources->GetFirstRow();
			COleVariant varTrue((short)TRUE, VT_BOOL);
			COleVariant varFalse((short)FALSE, VT_BOOL);
			while (NULL != pRow) {
				pRow->Value[rlcCheck] = varFalse;
				pRow = pRow->GetNextRow();
			}
			for (int i = 0; i < m_pLineItem->GetResourceCount(); i++) {
				long nResID = m_pLineItem->GetResourceByIndex(i).m_nID;
				pRow = m_dlResources->FindByColumn(rlcID, nResID, NULL, FALSE);
				if (NULL != pRow) {
					pRow->Value[rlcCheck] = varTrue;
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-12-11 11:47) - PLID 64230
void CTemplateLineItemDlg::OnRequeryFinishedResourceDropdown(short nFlags)
{
	try
	{
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlApplyResource->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (VarBool(pRow->GetValue(arcInactive))) {
				pRow->PutVisible(VARIANT_FALSE);
			}
		}

		if (m_pLineItem->GetResourceCount() > 0) {
			m_pdlApplyResource->SetSelByColumn(arcID, m_pLineItem->GetResourceByIndex(0).m_nID);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnEditingFinishedResourceList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try
	{
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnSelChosenResourceDropdown(LPDISPATCH lpRow)
{
	try
	{
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnAllResourcesBtn()
{
	try
	{
		// (c.haag 2006-11-09 10:31) - PLID 23336 - Change the resource list state and
		// apply changes to our member variables
		GetDlgItem(IDC_LIST_TEMPLATE_RESOURCES)->EnableWindow(FALSE);
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnSelectResourcesBtn()
{
	try
	{
		// (c.haag 2006-11-09 10:31) - PLID 23336 - Change the resource list state and
		// apply changes to our member variables
		GetDlgItem(IDC_LIST_TEMPLATE_RESOURCES)->EnableWindow(TRUE);
		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnAddLineItemException()
{
	try
	{
		// (c.haag 2006-11-17 12:40) - PLID 23589 - Add a new line item exception
		CChooseDateDlg dlg(this);

		// Have the user pick the date. If they cancel out of the dialog, then quit
		COleDateTime dt = COleDateTime::GetCurrentTime();
		BOOL bRetry;
		do {
			dt = dlg.Open(dt);
			if (COleDateTime::invalid == dt.GetStatus())
				return;
			dt.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());

			// Make sure the date doesn't already exist in the list
			bRetry = FALSE;
			if (NULL != m_dlExceptionDates->FindByColumn(elcDate, ToDateVariant(dt), 0, false)) {
				MsgBox(MB_ICONWARNING | MB_OK, "This date already exists in the list. Please choose another date.");
				bRetry = TRUE;
			}
		} while (bRetry);

		// If we get here, the user picked a valid date; so add it to the list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlExceptionDates->GetNewRow();
		pRow->Value[elcDate] = ToDateVariant(dt);
		m_dlExceptionDates->AddRowAtEnd(pRow, NULL);
		m_dlExceptionDates->Sort();

		// Update the button appearances
		ReflectCurrentStateOnBtns();

		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnEditLineItemException()
{
	try
	{
		// (c.haag 2006-11-17 12:40) - PLID 23589 - Edit an existing line item exception
		CChooseDateDlg dlg(this);

		// Get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlExceptionDates->CurSel;
		if (NULL == pRow)
			return;

		COleDateTime dt = VarDateTime(pRow->GetValue(elcDate));
		BOOL bRetry;
		do {
			dt = dlg.Open(dt);
			if (COleDateTime::invalid == dt.GetStatus())
				return;
			dt.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());

			// Make sure the date doesn't already exist in the list
			bRetry = FALSE;
			if (NULL != m_dlExceptionDates->FindByColumn(elcDate, ToDateVariant(dt), 0, false)) {
				MsgBox(MB_ICONWARNING | MB_OK, "This date already exists in the list. Please choose another date.");
				bRetry = TRUE;
			}
		} while (bRetry);

		// If we get here, the user picked a valid date; so update the list
		pRow->Value[elcDate] = ToDateVariant(dt);
		m_dlExceptionDates->Sort();

		// Update the button appearances
		ReflectCurrentStateOnBtns();

		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnRemoveLineItemException()
{
	try
	{
		// (z.manning 2008-06-12 11:50) - PLID 29273 - Do not allow any template related
		// deleting without this permission.
		//TES 2/20/2015 - PLID 64336 - This was just checking bioSchedTemplating, regardless of m_eEditoryType
		if (!CheckCurrentUserPermissions(m_eEditorType == stetLocation?bioResourceAvailTemplating:bioSchedTemplating, sptDelete)) {
			return;
		}

		// (c.haag 2006-11-17 12:40) - PLID 23589 - Remove an existing line item exception

		// Get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlExceptionDates->CurSel;
		if (NULL == pRow)
			return;

		// Remove the list item
		m_dlExceptionDates->RemoveRow(pRow);

		// Update the button appearances
		ReflectCurrentStateOnBtns();

		ApplyChanges();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnSelchangedExceptionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		// (c.haag 2006-11-17 12:47) - PLID 23589 - Update the button appearances
		ReflectCurrentStateOnBtns();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::OnDblClickCellExceptionList(LPDISPATCH lpRow, short nColIndex) 
{
	try
	{
		OnEditLineItemException();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateLineItemDlg::ReflectCurrentStateOnBtns()
{
	// (c.haag 2006-11-17 12:42) - PLID 23589 - Make sure that the template
	// exception add, edit, and remove buttons are properly enabled or disabled
	if (NULL == m_dlExceptionDates->CurSel) {
		GetDlgItem(IDC_BTN_EDIT_LINE_ITEM_EXCEPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_REMOVE_LINE_ITEM_EXCEPTION)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_BTN_EDIT_LINE_ITEM_EXCEPTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_REMOVE_LINE_ITEM_EXCEPTION)->EnableWindow(TRUE);
	}
}

_variant_t CTemplateLineItemDlg::ToDateVariant(const COleDateTime& dt)
{
	_variant_t v((DATE)dt);
	v.vt = VT_DATE;
	return v;
}

void CTemplateLineItemDlg::OnCheckBlockAppts()
{
	try {
		if(IsDlgButtonChecked(IDC_CHECK_BLOCK_APPTS)) {
			//TES 12/19/2008 - PLID 32537 - Scheduler Standard users aren't allowed to create precision templates.
			if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Precision Scheduler templating", "System_Setup/Scheduler_Setup/Precision_Templating.htm")) {
				CheckDlgButton(IDC_CHECK_BLOCK_APPTS, BST_UNCHECKED);
			}
		}
	}NxCatchAll("Error in CTemplateLineItemDlg::OnCheckBlockAppts()");
}

void CTemplateLineItemDlg::OnChangeStartDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	// (d.singleton 2011-10-13 14:11) - PLID 29351 - Scheduler Template Editor - Date range - auto change the end date to the same as the start date
	COleDateTime dtStartTime;
	m_dtpStartDate.GetTime(dtStartTime);
	m_dtpEndDate.SetTime(dtStartTime);
	ApplyChanges();	
	*pResult = 0;
}

void CTemplateLineItemDlg::OnChangeEndDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	ApplyChanges();
	*pResult = 0;
}
