// ApptTypeSelector.cpp : implementation file
//

#include "stdafx.h"
#include "ApptSelector.h"
#include "PracProps.h"
#include "GlobalUtils.h"
#include "practice.h"
#include "NxException.h"
#include "Filter.h"
#include "GlobalDataUtils.h"
#include "FilterDetail.h"
#include "AuditTrail.h"
#include "FilterFieldInfo.h"
#include "Groups.h"
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FILTER_NAME_NEW				"Enter a name for this filter"

/////////////////////////////////////////////////////////////////////////////
// CApptSelector dialog


CApptSelector::CApptSelector(BOOL bNewFilter /* = TRUE*/, long nFilterID /* = -1*/, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*))
	: CNxDialog(CApptSelector::IDD, NULL)
{
	//{{AFX_DATA_INIT(CApptSelector)
	//}}AFX_DATA_INIT

	m_bNewFilter = bNewFilter;
	m_nFilterID = nFilterID;
	m_pfnIsActionSupported = pfnIsActionSupported;
	m_pfnCommitSubfilterAction = pfnCommitSubfilterAction;
	m_strOldFilterName = "";

	for (int i = 0; i < 5; i++) 
		m_bListSet[i] = FALSE;
}


void CApptSelector::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApptSelector)
	DDX_Control(pDX, IDC_SHOW_INACTIVE, m_btnShowInactive);
	DDX_Control(pDX, IDC_USE_DATE_RANGE, m_dateFilter);
	DDX_Control(pDX, IDC_TYPE_REMOVE_ALL, m_typeRemoveAllBtn);
	DDX_Control(pDX, IDC_TYPE_REMOVE, m_typeRemoveBtn);
	DDX_Control(pDX, IDC_TYPE_ADD_ALL, m_typeAddAllBtn);
	DDX_Control(pDX, IDC_TYPE_ADD, m_typeAddBtn);
	DDX_Control(pDX, IDC_STATUS_REMOVE_ALL, m_statusRemoveAllBtn);
	DDX_Control(pDX, IDC_STATUS_REMOVE, m_statusRemoveBtn);
	DDX_Control(pDX, IDC_STATUS_ADD_ALL, m_statusAddAllBtn);
	DDX_Control(pDX, IDC_STATUS_ADD, m_statusAddBtn);
	DDX_Control(pDX, IDC_RES_REMOVE_ALL, m_resRemoveAllBtn);
	DDX_Control(pDX, IDC_RES_REMOVE, m_resRemoveBtn);
	DDX_Control(pDX, IDC_RES_ADD_ALL, m_resAddAllBtn);
	DDX_Control(pDX, IDC_RES_ADD, m_resAddBtn);
	DDX_Control(pDX, IDC_PUR_REMOVE_ALL, m_purRemoveAllBtn);
	DDX_Control(pDX, IDC_PUR_REMOVE, m_purRemoveBtn);
	DDX_Control(pDX, IDC_PUR_ADD_ALL, m_purAddAllBtn);
	DDX_Control(pDX, IDC_PUR_ADD, m_purAddBtn);
	DDX_Control(pDX, IDC_LOC_REMOVE_ALL, m_locRemoveAllBtn);
	DDX_Control(pDX, IDC_LOC_REMOVE, m_locRemoveBtn);
	DDX_Control(pDX, IDC_LOC_ADD_ALL, m_locAddAllBtn);
	DDX_Control(pDX, IDC_LOC_ADD, m_locAddBtn);
	DDX_Control(pDX, IDC_TO, m_dateTo);
	DDX_Control(pDX, IDC_FROM, m_dateFrom);
	DDX_Control(pDX, IDC_EXCLUDE_CANCELLED, m_includeCancelled);
	DDX_Control(pDX, IDC_FILTER_NAME_EDIT, m_nxeditFilterNameEdit);
	DDX_Control(pDX, IDC_FILTER_BKG_LABEL, m_nxstaticFilterBkgLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApptSelector, CNxDialog)
	//{{AFX_MSG_MAP(CApptSelector)
	ON_BN_CLICKED(IDC_SHOW_INACTIVE, OnShowInactive)
	ON_BN_CLICKED(IDC_TYPE_ADD, OnTypeAdd)
	ON_BN_CLICKED(IDC_TYPE_ADD_ALL, OnTypeAddAll)
	ON_BN_CLICKED(IDC_TYPE_REMOVE, OnTypeRemove)
	ON_BN_CLICKED(IDC_TYPE_REMOVE_ALL, OnTypeRemoveAll)
	ON_BN_CLICKED(IDC_STATUS_ADD, OnStatusAdd)
	ON_BN_CLICKED(IDC_STATUS_ADD_ALL, OnStatusAddAll)
	ON_BN_CLICKED(IDC_STATUS_REMOVE, OnStatusRemove)
	ON_BN_CLICKED(IDC_STATUS_REMOVE_ALL, OnStatusRemoveAll)
	ON_BN_CLICKED(IDC_RES_ADD, OnResAdd)
	ON_BN_CLICKED(IDC_RES_ADD_ALL, OnResAddAll)
	ON_BN_CLICKED(IDC_RES_REMOVE, OnResRemove)
	ON_BN_CLICKED(IDC_RES_REMOVE_ALL, OnResRemoveAll)
	ON_BN_CLICKED(IDC_PUR_ADD, OnPurAdd)
	ON_BN_CLICKED(IDC_PUR_ADD_ALL, OnPurAddAll)
	ON_BN_CLICKED(IDC_PUR_REMOVE, OnPurRemove)
	ON_BN_CLICKED(IDC_PUR_REMOVE_ALL, OnPurRemoveAll)
	ON_BN_CLICKED(IDC_LOC_ADD, OnLocAdd)
	ON_BN_CLICKED(IDC_LOC_ADD_ALL, OnLocAddAll)
	ON_BN_CLICKED(IDC_LOC_REMOVE, OnLocRemove)
	ON_BN_CLICKED(IDC_LOC_REMOVE_ALL, OnLocRemoveAll)
	ON_BN_CLICKED(IDC_USE_DATE_RANGE, OnUseDateRange)
	ON_BN_CLICKED(IDC_EXCLUDE_CANCELLED, OnIncludeCancelled)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CApptSelector, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CApptSelector)
	ON_EVENT(CApptSelector, IDC_RESOURCE_SELECTED, 3 /* DblClickCell */, OnDblClickCellResSelected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_RESOURCE_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellResUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_PUR_SELECTED, 3 /* DblClickCell */, OnDblClickCellPurSelected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_PUR_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellPurUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_STATUS_SELECTED, 3 /* DblClickCell */, OnDblClickCellStatusSelected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_STATUS_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellStatusUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_LOCATION_SELECTED, 3 /* DblClickCell */, OnDblClickCellLocSelected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_LOCATION_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellLocUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_TYPE_SELECTED, 3 /* DblClickCell */, OnDblClickCellTypeSelected, VTS_I4 VTS_I2)
	ON_EVENT(CApptSelector, IDC_TYPE_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellTypeUnselected, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApptSelector message handlers

BOOL CApptSelector::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	CWaitCursor wc;

	try {
		m_resUnselected = BindNxDataListCtrl(IDC_RESOURCE_UNSELECTED);
		m_resSelected = BindNxDataListCtrl(IDC_RESOURCE_SELECTED, false);
		m_purUnselected = BindNxDataListCtrl(IDC_PUR_UNSELECTED);
		m_purSelected = BindNxDataListCtrl(IDC_PUR_SELECTED, false);
		m_statusUnselected = BindNxDataListCtrl(IDC_STATUS_UNSELECTED);
		m_statusSelected = BindNxDataListCtrl(IDC_STATUS_SELECTED, false);
		m_locUnselected = BindNxDataListCtrl(IDC_LOCATION_UNSELECTED);
		m_locSelected = BindNxDataListCtrl(IDC_LOCATION_SELECTED, false);
		m_typeUnselected = BindNxDataListCtrl(IDC_TYPE_UNSELECTED);
		m_typeSelected = BindNxDataListCtrl(IDC_TYPE_SELECTED, false);
		
		m_typeRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_typeRemoveBtn.AutoSet(NXB_LEFT);
		m_typeAddAllBtn.AutoSet(NXB_RRIGHT);
		m_typeAddBtn.AutoSet(NXB_RIGHT);
		m_statusRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_statusRemoveBtn.AutoSet(NXB_LEFT);
		m_statusAddAllBtn.AutoSet(NXB_RRIGHT);
		m_statusAddBtn.AutoSet(NXB_RIGHT);
		m_resRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_resRemoveBtn.AutoSet(NXB_LEFT);
		m_resAddAllBtn.AutoSet(NXB_RRIGHT);
		m_resAddBtn.AutoSet(NXB_RIGHT);
		m_purRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_purRemoveBtn.AutoSet(NXB_LEFT);
		m_purAddAllBtn.AutoSet(NXB_RRIGHT);
		m_purAddBtn.AutoSet(NXB_RIGHT);
		m_locRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_locRemoveBtn.AutoSet(NXB_LEFT);
		m_locAddAllBtn.AutoSet(NXB_RRIGHT);
		m_locAddBtn.AutoSet(NXB_RIGHT);
		// (z.manning, 04/25/2008) - PLID 29795 - OK and cancel buttons
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if (m_bNewFilter) {
			InitNewFilter();
		}
		else {
			// Put the inactive items in the unselected lists so that they can be selected if need be
			CheckDlgButton(IDC_SHOW_INACTIVE, TRUE);
			OnShowInactive();

			// Select all of the items on the dialog for the filter
			if (!TranslateFilterToDlg()) {
				// There are some fields on the filter that can't be represented on the dialog, thus the Appointment Selector can't be used
				AfxMessageBox("This filter is too advanced to open using the Appointment Selector.\nUse the Filter Editor to edit this filter.");
				OnCancel();
			}
		}

		// Set the show inactive checkbox appropriately
		if(GetRemotePropertyInt("ApptSelectorShowInactive", 0, 0, GetCurrentUserName(), true) == 1)
			CheckDlgButton(IDC_SHOW_INACTIVE, TRUE);
		else
			CheckDlgButton(IDC_SHOW_INACTIVE, FALSE);

		OnShowInactive();

	}NxCatchAll("Error in CApptSelector::OnInitDialog");
	
	return TRUE;  
}

void CApptSelector::OnOK() 
{	
	if (m_dateFilter.GetCheck()) {
		if ((COleDateTime)m_dateTo.GetValue() < (COleDateTime)m_dateFrom.GetValue()){
			MsgBox(MB_ICONHAND + MB_OK, "The date range is invalid: The \"From\" date is after the \"To\" date.");
			return;
		}
	}

	//remember if this box is checked
	long nRemember = 0;
	if(IsDlgButtonChecked(IDC_SHOW_INACTIVE))
		nRemember = 1;
	SetRemotePropertyInt("ApptSelectorShowInactive", nRemember, 0, GetCurrentUserName());

	CFilter *pFilter = CreateNewFilterFromDlg();

#ifdef _DEBUG
	// For Debugging show the clauses that were generated
	CString strFrom, strWhere;
	if(!pFilter->GetFromClause(strFrom)) {
		MsgBox("Could not save filter");
		return;
	}
	CMapStringToString mapUsedFilters;
	pFilter->GetWhereClause(strWhere, mapUsedFilters);
	if (MsgBox(MB_OKCANCEL, "SELECT * FROM %s\n\nWHERE %s", strFrom, strWhere) != IDOK) return;
#endif

	if(!SaveFilterToData(pFilter)) {
		// That function should have already displayed a msg box for the problem (e.g. a blank filter).
		return;
	}

	if (pFilter)
		delete pFilter;

	CClient::RefreshTable(NetUtils::FiltersT, m_nFilterID);
	
	CNxDialog::OnOK();
}

void CApptSelector::OnCancel() 
{	
	CNxDialog::OnCancel();
}

void CApptSelector::OnShowInactive() 
{
	CString strWhereClause;

	if (IsDlgButtonChecked(IDC_SHOW_INACTIVE)) {
		m_locUnselected->WhereClause =  "TypeID = 1 " + _bstr_t(CreateWhereClause("AND LocationsT.ID", m_locSelected));
		m_resUnselected->WhereClause = _bstr_t(CreateWhereClause("ResourceT.ID", m_resSelected));
		m_typeUnselected->WhereClause = _bstr_t(CreateWhereClause("AptTypeT.ID", m_typeSelected));
	}
	else {
		m_locUnselected->WhereClause = "LocationsT.Active <> 0 AND TypeID = 1" + _bstr_t(CreateWhereClause("AND LocationsT.ID", m_locSelected));
		m_resUnselected->WhereClause = "ResourceT.Inactive = 0" + _bstr_t(CreateWhereClause("AND ResourceT.ID", m_resSelected));
		m_typeUnselected->WhereClause = "AptTypeT.Inactive = 0" + _bstr_t(CreateWhereClause("AND AptTypeT.ID", m_typeSelected));
	}

	m_locUnselected->Clear();
	m_resUnselected->Clear();
	m_typeUnselected->Clear();

	m_locUnselected->Requery();
	m_resUnselected->Requery();
	m_typeUnselected->Requery();
}

CString CApptSelector::CreateWhereClause(CString strFieldName, _DNxDataListPtr pSelectedDataList)
{
	CString strIDs, strWhereClause = "";
	strIDs = CreateIDString(pSelectedDataList);
	if (strIDs != "") {
		strWhereClause.Format("%s NOT IN (%s)", strFieldName, strIDs);
	}
	
	return strWhereClause;
}

// Add buttons
void CApptSelector::OnTypeAdd()
{
	m_typeSelected->TakeCurrentRow(m_typeUnselected);
}

void CApptSelector::OnStatusAdd()
{
	m_statusSelected->TakeCurrentRow(m_statusUnselected);
}

void CApptSelector::OnResAdd()
{
	m_resSelected->TakeCurrentRow(m_resUnselected);
}

void CApptSelector::OnPurAdd()
{
	m_purSelected->TakeCurrentRow(m_purUnselected);
}

void CApptSelector::OnLocAdd()
{
	m_locSelected->TakeCurrentRow(m_locUnselected);
}

// Add All buttons
void CApptSelector::OnTypeAddAll()			
{
	m_typeSelected->TakeAllRows(m_typeUnselected); 
}

void CApptSelector::OnStatusAddAll()			
{
	m_statusSelected->TakeAllRows(m_statusUnselected); 
}

void CApptSelector::OnResAddAll()			
{
	m_resSelected->TakeAllRows(m_resUnselected); 
}

void CApptSelector::OnPurAddAll()			
{
	m_purSelected->TakeAllRows(m_purUnselected); 
}

void CApptSelector::OnLocAddAll()			
{
	m_locSelected->TakeAllRows(m_locUnselected); 
}

// Remove buttons
void CApptSelector::OnTypeRemove()			
{
	m_typeUnselected->TakeCurrentRow(m_typeSelected); 
}

void CApptSelector::OnStatusRemove()			
{
	m_statusUnselected->TakeCurrentRow(m_statusSelected); 
}

void CApptSelector::OnResRemove()			
{
	m_resUnselected->TakeCurrentRow(m_resSelected); 
}

void CApptSelector::OnPurRemove()			
{
	m_purUnselected->TakeCurrentRow(m_purSelected); 
}

void CApptSelector::OnLocRemove()			
{
	m_locUnselected->TakeCurrentRow(m_locSelected); 
}

// Remove All buttons
void CApptSelector::OnTypeRemoveAll()		
{ 
	m_typeUnselected->TakeAllRows(m_typeSelected); 
}

void CApptSelector::OnStatusRemoveAll()		
{ 
	m_statusUnselected->TakeAllRows(m_statusSelected); 
}

void CApptSelector::OnResRemoveAll()		
{ 
	m_resUnselected->TakeAllRows(m_resSelected); 
}

void CApptSelector::OnPurRemoveAll()		
{ 
	m_purUnselected->TakeAllRows(m_purSelected); 
}

void CApptSelector::OnLocRemoveAll()		
{ 
	m_locUnselected->TakeAllRows(m_locSelected); 
}

// On double clicks
void CApptSelector::OnDblClickCellTypeSelected(long nRowIndex, short nColIndex) 
{
	OnTypeRemove();
}

void CApptSelector::OnDblClickCellTypeUnselected(long nRowIndex, short nColIndex) 
{
	OnTypeAdd();
}

void CApptSelector::OnDblClickCellStatusSelected(long nRowIndex, short nColIndex) 
{
	OnStatusRemove();
}

void CApptSelector::OnDblClickCellStatusUnselected(long nRowIndex, short nColIndex) 
{
	OnStatusAdd();
}

void CApptSelector::OnDblClickCellResSelected(long nRowIndex, short nColIndex) 
{
	OnResRemove();
}

void CApptSelector::OnDblClickCellResUnselected(long nRowIndex, short nColIndex) 
{
	OnResAdd();
}

void CApptSelector::OnDblClickCellPurSelected(long nRowIndex, short nColIndex) 
{
	OnPurRemove();
}

void CApptSelector::OnDblClickCellPurUnselected(long nRowIndex, short nColIndex) 
{
	OnPurAdd();
}

void CApptSelector::OnDblClickCellLocSelected(long nRowIndex, short nColIndex) 
{
	OnLocRemove();
}

void CApptSelector::OnDblClickCellLocUnselected(long nRowIndex, short nColIndex) 
{
	OnLocAdd();
}


void CApptSelector::OnUseDateRange() 
{
	if(IsDlgButtonChecked(IDC_USE_DATE_RANGE)) {
		GetDlgItem(IDC_FROM)->EnableWindow(TRUE);
		GetDlgItem(IDC_TO)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_TO)->EnableWindow(FALSE);
	}
}

// Creates a string of all of the IDs in column 0 in the datalist
CString CApptSelector::CreateIDString(_DNxDataListPtr pDataList)
{
	CString strIDList = "";

	try {
		CString strTemp;
		IRowSettingsPtr pRow;

		// Go through all items in list and create ID string
		for (int i = 0; i < pDataList->GetRowCount(); i++) {
			pRow = pDataList->GetRow(i);
			strTemp.Format("%li, ", AsLong(pRow->GetValue(0)));
			strIDList += strTemp;
		}

		strIDList.TrimRight(", ");
	}NxCatchAll("Error in CApptSelector::CreateIDString");

	return strIDList;
}

// Puts the items with IDs in the IDList from the unselected datalist into the selected datalist
void CApptSelector::SelectItemsFromIDString(_DNxDataListPtr pUnselectedDataList, _DNxDataListPtr pSelectedDataList, CString strIDList) 
{
	try {
		long nNumLength = 0, nID, nResult;
		strIDList.TrimLeft("\"");
		strIDList.TrimRight("\"");
		while (strIDList.GetLength() != 0) {
			nNumLength = strIDList.Find(",");
			if (nNumLength == -1)
				nNumLength = strIDList.GetLength();
			nID = atoi(strIDList.Left(nNumLength));
			strIDList = strIDList.Right((strIDList.GetLength() - nNumLength - 1) <= strIDList.GetLength() ? (strIDList.GetLength() - nNumLength - 1) : (strIDList.GetLength() - nNumLength));

			// Bring item with that ID over to selected list
			nResult = pUnselectedDataList->FindByColumn(0, nID, 0, true);
			if (nResult != -1)
				pSelectedDataList->TakeCurrentRow(pUnselectedDataList);
		}
	}NxCatchAll("Error in CApptSelector::SelectItemsFromIDString");
}

void CApptSelector::OnIncludeCancelled() 
{
	if (IsDlgButtonChecked(IDC_EXCLUDE_CANCELLED))
		m_bIncludeCancelled = TRUE;
	else
		m_bIncludeCancelled = FALSE;

}

BOOL CApptSelector::TranslateFilterToDlg()
{
	try {
		_RecordsetPtr prs = CreateRecordset("SELECT Name, Filter FROM FiltersT WHERE ID = %li", m_nFilterID);
		if (!prs->eof) {
			// Create the filter object
			CFilter filter(m_nFilterID, fboAppointment, NULL, NULL, NULL);

			CArray<CString, CString> arystrDates;
			CArray<FieldOperatorEnum, FieldOperatorEnum> aryfoeOperator;

			if (filter.SetFilterString(AdoFldString(prs, "Filter")) >= 0) {
				CFilterDetail *pFilterDetail;
				long nCount = filter.GetDetailAryItemCount();
				
				// Go through all of the details of the filter and make the appropriate seletions on the dialog
				for (int i = 0; i < nCount; i++) {
					pFilterDetail = filter.GetFilterDetail(i);
					if (!SetDataItemsOnDlg(pFilterDetail->GetDetailInfoId(), pFilterDetail->GetDetailOperator(), pFilterDetail->GetDetailValue(), pFilterDetail->GetDetailUseOr(), arystrDates, aryfoeOperator, i == 0 ? true : false))
						// This means that the filter is too advanced and can't be represented on the Appointment Selector dialog
						return FALSE;
				}
			}
			
			// Set the name of the filter
			m_strOldFilterName = AdoFldString(prs, "Name");
			SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strOldFilterName);

			if (!SetDateRange(arystrDates, aryfoeOperator))
				// The date cannot be represented on the Appointment Selector dialog
				return FALSE;
		}

	}NxCatchAll("Error in CApptSelector::TranslateFilterToDlg");

	return TRUE;
}

BOOL CApptSelector::SetDateRange(CArray<CString, CString> &arystrDates, CArray<FieldOperatorEnum, FieldOperatorEnum> &aryfoeOperator)
{
	// The ideal situation is that there are two dates in the array where one is >= and the other is <=, or there is only one date
	// with the operator =.
	// If there is something else besides this ideal situation, then the Appointment Selector dialog can't handle it, i.e. return FALSE.
	// It is assumed that both arrays passed to this function are the same size.  One array has the date and the other has its 
	// corresponding operator.

	if (arystrDates.GetSize() == 0) {
		// There are no dates to worry about
		// Set dates to current date then disable
		COleDateTime now = COleDateTime::GetCurrentTime();
		COleDateTime dt(now.GetYear(), 
						now.GetMonth(),
						now.GetDay(),
						0,0,0);
		// (a.walling 2008-05-13 15:42) - PLID 27591 - COleVariant no longer necessary
		m_dateFrom.SetValue((dt));
		m_dateTo.SetValue((dt));
		m_dateFilter.SetCheck(FALSE);
	}
	else if (arystrDates.GetSize() == 1){
		if (aryfoeOperator.GetAt(0) == foEqual) {
			COleDateTime tempDate;
			tempDate.ParseDateTime(arystrDates.GetAt(0));
			// (a.walling 2008-05-13 15:42) - PLID 27591 - COleVariant no longer necessary
			m_dateFrom.SetValue((tempDate));
			m_dateTo.SetValue((tempDate));

			m_dateFilter.SetCheck(TRUE);
		}
		else
			// The one date had a different operator than equal to
			return FALSE;
	}
	else if (arystrDates.GetSize() == 2) {
		FieldOperatorEnum foeDate1 = aryfoeOperator.GetAt(0), foeDate2 = aryfoeOperator.GetAt(1);
		COleDateTime dtDate1, dtDate2;	
		dtDate1.ParseDateTime(arystrDates.GetAt(0));
		dtDate2.ParseDateTime(arystrDates.GetAt(1));
		BOOL bFoundGreater = FALSE, bFoundLess = FALSE;

		// Determine type of operator for Date1: must be either >, <, >= or <=
		if (foeDate1 == foGreater || foeDate1 == foGreaterEqual)
			bFoundGreater = TRUE;
		else if (foeDate1 == foLess || foeDate1 == foLessEqual)
			bFoundLess = TRUE;
		else
			return FALSE;

		// Determine type of operator for Date2: must be either >, <, >= or <= and it can't be the same as Date1
		if ((foeDate2 == foGreater || foeDate2 == foGreaterEqual) && bFoundGreater == FALSE) {
			// Date2 should be greater than Date1
			if (dtDate2 >= dtDate1)
				return FALSE;
		}
		else if ((foeDate2 == foLess || foeDate2 == foLessEqual) && bFoundLess == FALSE) {
			// Date2 should be less than Date1
			if (dtDate2 <= dtDate1)
				return FALSE;
		}
		else
			return FALSE;

		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(1,0,0,0);

		// Make the operators be only "greater than or equal" or "less than or equal" fixing the dates where appropriate
		if (foeDate1 == foGreater) {
			foeDate1 = foGreaterEqual;
			dtDate1 += dtSpan;

		}
		else if (foeDate1 == foLess) {
			foeDate1 = foLessEqual;
			dtDate1 -= dtSpan;
		}

		if (foeDate2 == foGreater) {
			foeDate2 = foGreaterEqual;
			dtDate2 += dtSpan;

		}
		else if (foeDate2 == foLess) {
			foeDate2 = foLessEqual;
			dtDate2 -= dtSpan;
		}

		// Ok, now we're ready to set the dates on the dialog
		// (a.walling 2008-05-13 15:42) - PLID 27591 - COleVariant no longer necessary
		if (foeDate1 == foGreaterEqual) {
			m_dateFrom.SetValue((dtDate1));
			m_dateTo.SetValue((dtDate2));
		}
		else {
			m_dateFrom.SetValue((dtDate2));
			m_dateTo.SetValue((dtDate1));
		}

		m_dateFilter.SetCheck(TRUE);
	}
	else
		// Not one of the handled situations
		return FALSE;

	OnUseDateRange();
	
	return TRUE;
}

BOOL CApptSelector::SetDataItemsOnDlg(long nInfoId, FieldOperatorEnum foOperator, CString strValue, bool bUseOr, CArray<CString, CString> &arystrDates, CArray<FieldOperatorEnum, FieldOperatorEnum> &aryfoeOperator, bool bFirstDetail)
{ 
	try {
		// Only the AND operator is supported by the Appointment Selector
		if (bUseOr == true && bFirstDetail != true)
			return FALSE;

		// Only the equals comparison operator is supported by the Appointment Selector
		if (foOperator != foEqual && nInfoId != FILTER_FIELD_DATE)
			return FALSE;

		// Items can only be set for a data list once. If there are multiple details in the filter for this field, then the filter is
		// most likely to advanced for the Appointment Selector
		if (HaveItemsForListBeenSelected(nInfoId))
			return FALSE;

		switch (nInfoId)
		{
		case FILTER_FIELD_STATUS: // "Show State" (Status)
			SelectItemsFromIDString(m_statusUnselected, m_statusSelected, strValue);
			break;
	
		case FILTER_FIELD_DATE: // "Date"
			arystrDates.Add(strValue);
			aryfoeOperator.Add(foOperator);
			break;

		case FILTER_FIELD_PURPOSE: // "Has Purpose"
			SelectItemsFromIDString(m_purUnselected, m_purSelected, strValue);
			break;

		case FILTER_FIELD_LOCATION: // "Location"
			SelectItemsFromIDString(m_locUnselected, m_locSelected, strValue);
			break;

		case FILTER_FIELD_RESOURCE: // "Has Resource"
			SelectItemsFromIDString(m_resUnselected, m_resSelected, strValue);
			break;

		case FILTER_FIELD_TYPE: // "Type"
			SelectItemsFromIDString(m_typeUnselected, m_typeSelected, strValue);
			break;

		case FILTER_FIELD_CANCELLED: // "Cancelled"
			if (strValue == "0")
				CheckDlgButton(IDC_EXCLUDE_CANCELLED, true);
			else
				return FALSE;
			break;

		default:
			// This detail can't be represented on the Appointment Selector, so return FALSE
			return FALSE;
		}

		SetItemsForListHaveBeenSelected(nInfoId);

	}NxCatchAll("Error in CApptSelector::SelectDataListItems");

	return TRUE;
}

void CApptSelector::SetItemsForListHaveBeenSelected(long nInfoId)
{
	switch (nInfoId)
	{
	case FILTER_FIELD_STATUS: // "Show State" (Status)
		m_bListSet[0] = TRUE;
		break;

	case FILTER_FIELD_PURPOSE: // "Has Purpose"
		m_bListSet[1] = TRUE;
		break;

	case FILTER_FIELD_LOCATION: // "Location"
		m_bListSet[2] = TRUE;
		break;

	case FILTER_FIELD_RESOURCE: // "Has Resource"
		m_bListSet[3] = TRUE;
		break;

	case FILTER_FIELD_TYPE: // "Type"
		m_bListSet[4] = TRUE;
		break;

	default:
		// This item is not a datalist on this dialog, this function should not be called if it isn't, but it won't hurt anything
		break;
	}
}

BOOL CApptSelector::HaveItemsForListBeenSelected(long nInfoId)
{
	switch (nInfoId)
	{
	case FILTER_FIELD_STATUS: // "Show State" (Status)
		return m_bListSet[0];

	case FILTER_FIELD_PURPOSE: // "Has Purpose"
		return m_bListSet[1];

	case FILTER_FIELD_LOCATION: // "Location"
		return m_bListSet[2];

	case FILTER_FIELD_RESOURCE: // "Has Resource"
		return m_bListSet[3];

	case FILTER_FIELD_TYPE: // "Type"
		return m_bListSet[4];

	default:
		// This item is not a datalist on this dialog, this function should not be called if it isn't
		return FALSE;
	}
}

/*void CApptSelector::FillDlgValues()
{
	if (m_strDateTo == "" || m_strDateFrom == "") {
		COleDateTime now = COleDateTime::GetCurrentTime();
		COleDateTime dt(now.GetYear(), 
						now.GetMonth(),
						now.GetDay(),
						0,0,0);
		m_dateFrom.SetValue(COleVariant(dt));
		m_dateTo.SetValue(COleVariant(dt));
		m_dateFilter.SetCheck(FALSE);
	}
	else {
		COleDateTime tempDate;
		tempDate.ParseDateTime(m_strDateFrom);
		m_dateFrom.SetValue(COleVariant(tempDate));
		tempDate.ParseDateTime(m_strDateTo);
		m_dateTo.SetValue(COleVariant(tempDate));
		m_dateFilter.SetCheck(TRUE);
	}

	OnUseDateRange();

	CheckDlgButton(IDC_SHOW_INACTIVE, TRUE);
	OnShowInactive();

	SelectItemsFromIDString(m_resUnselected, m_resSelected, m_resSelectedIDs);
	SelectItemsFromIDString(m_purUnselected, m_purSelected, m_purSelectedIDs);
	SelectItemsFromIDString(m_statusUnselected, m_statusSelected, m_statusSelectedIDs);
	SelectItemsFromIDString(m_locUnselected, m_locSelected, m_locSelectedIDs);
	SelectItemsFromIDString(m_typeUnselected, m_typeSelected, m_typeSelectedIDs);

	CheckDlgButton(IDC_EXCLUDE_CANCELLED, m_bIncludeCancelled);
	OnIncludeCancelled();


}*/

void CApptSelector::InitNewFilter()
{
	// Set default filter name text
	SetDlgItemText(IDC_FILTER_NAME_EDIT, FILTER_NAME_NEW);

	// Set dates to current date then disable
	COleDateTime now = COleDateTime::GetCurrentTime();
	COleDateTime dt(now.GetYear(), 
					now.GetMonth(),
					now.GetDay(),
					0,0,0);
	// (a.walling 2008-05-13 15:42) - PLID 27591 - COleVariant no longer necessary
	m_dateFrom.SetValue((dt));
	m_dateTo.SetValue((dt));
	m_dateFilter.SetCheck(FALSE);
	OnUseDateRange();
}

/*void CApptSelector::FillFilterCombo()
{
	CWaitCursor wc;

	// First empty it out
	m_cboFilterList.ResetContent();	

	// Prepare variables
	int nIndex = 0;
	try {
		// Add all the user-defined filters (only patient filters exist right now)
		// Loop through the filter name records adding an entry for each one
		_RecordsetPtr prs = CreateRecordset("SELECT ID, Name FROM FiltersT WHERE Type = %li ORDER BY Name ASC", fboPerson);
		FieldsPtr flds = prs->Fields;
		FieldPtr fldName = flds->Item["Name"];
		FieldPtr fldID = flds->Item["ID"];
		CString strName;
		long nID;
		while (!prs->eof) {
			strName = AdoFldString(fldName);
			nID = AdoFldLong(fldID);
			nIndex = m_cboFilterList.InsertString(-1, strName);
			if (nIndex >= 0) {
				m_cboFilterList.SetItemData(nIndex, nID);
			} else {
				AfxThrowNxException("Could not add filter ID %li, Name '%s' to the list", nID, strName);
			}
			HR(prs->MoveNext());
		}

		nIndex = m_cboFilterList.InsertString(-1, "{All Patients}");
		if (nIndex >= 0) {
			m_cboFilterList.SetItemData(nIndex, FILTER_ID_ALL);
		}
		else { 
			AfxThrowUserException(); 
		}

	} NxCatchAll("Error in CApptSelector::FillFilterCombo");
}*/

CFilter *CApptSelector::CreateNewFilterFromDlg()
{
	CWaitCursor wc;

	CFilter *pFilter = new CFilter(m_nFilterID, fboAppointment, NULL, NULL, NULL);		

	AddListItemsToFilter(m_resSelected, pFilter, FILTER_FIELD_RESOURCE);
	AddListItemsToFilter(m_purSelected, pFilter, FILTER_FIELD_PURPOSE);
	AddListItemsToFilter(m_statusSelected, pFilter, FILTER_FIELD_STATUS);
	AddListItemsToFilter(m_locSelected, pFilter, FILTER_FIELD_LOCATION);
	AddListItemsToFilter(m_typeSelected, pFilter, FILTER_FIELD_TYPE);

	long nIndex;

	// Add dates to filter
	if (GetDlgItem(IDC_FROM)->IsWindowEnabled()) {
		nIndex = CFilterFieldInfo::GetInfoIndex(FILTER_FIELD_DATE, (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);

		// (a.walling 2008-05-13 15:45) - PLID 27591 - Need to pass a variant to AsString
		if (AsString((_variant_t)m_dateFrom.GetValue()) == AsString((_variant_t)m_dateTo.GetValue()))
			pFilter->AddDetail(nIndex, foEqual, AsString((_variant_t)m_dateFrom.GetValue()), false, INVALID_DYNAMIC_ID);
		else {
			pFilter->AddDetail(nIndex, foGreaterEqual, AsString((_variant_t)m_dateFrom.GetValue()), false, INVALID_DYNAMIC_ID);
			pFilter->AddDetail(nIndex, foLessEqual, AsString((_variant_t)m_dateTo.GetValue()), false, INVALID_DYNAMIC_ID);
		}		
	}

	if (IsDlgButtonChecked(IDC_EXCLUDE_CANCELLED)) {
		nIndex = CFilterFieldInfo::GetInfoIndex(FILTER_FIELD_CANCELLED, (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);

		pFilter->AddDetail(nIndex, foEqual, "0", false, INVALID_DYNAMIC_ID);
	}

	return pFilter;
}

void CApptSelector::AddListItemsToFilter(_DNxDataListPtr pDataList, CFilter *pFilter, long nFieldId)
{
	CString strValue;
	IRowSettingsPtr pRow;
	long nIndex;
	if (pDataList->GetRowCount() > 0) {
		nIndex = CFilterFieldInfo::GetInfoIndex(nFieldId, (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);

		pFilter->AddDetail(nIndex, foEqual, CreateIDString(pDataList), false, INVALID_DYNAMIC_ID);
	}
}

bool CApptSelector::SaveFilterToData(CFilter *pFilter)
{
	try {
		CString strFilterName, strFilterString;
		// Get the filter string from the filter
		if (pFilter->ValidateFilter()) {
			// Get the name for the filter
			GetDlgItemText(IDC_FILTER_NAME_EDIT, strFilterName);
			strFilterName.TrimLeft();
			strFilterName.TrimRight();
			//First, make sure that it is not "Enter a name for this filter" (We, the framework, put that string in there,
			//so it's up to us to deal with it.)
			int nReturn = IDOK;
			while(nReturn == IDOK && (strFilterName == FILTER_NAME_NEW || strFilterName.IsEmpty()) ){
				nReturn = InputBoxLimited(this, FILTER_NAME_NEW, strFilterName,"",50,false,false,NULL);
			}
			if(nReturn != IDOK) {
				//They canceled.
				return false;
			}

			// See if this is a new filter
			if (m_nFilterID == FILTER_ID_NEW) {
				// It is a new filter
				if(m_pfnIsActionSupported(saAdd, fboAppointment) && pFilter->GetFilterString(strFilterString)) {
					if(m_pfnCommitSubfilterAction(saAdd, fboAppointment, m_nFilterID, strFilterName, strFilterString, this)) {
						//auditing
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "",nAuditID, aeiFilterCreated, -1, "", strFilterName, aepMedium, aetCreated);
						return true;
					}
					else {
						//Failed to save.
						return false;
					}
				}
				else {
					//Adding filters not supported.
					return false;
				}
			} else {
				if(m_pfnIsActionSupported(saEdit, fboAppointment) && pFilter->GetFilterString(strFilterString)) {
					if(m_pfnCommitSubfilterAction(saEdit, fboAppointment, m_nFilterID, strFilterName, strFilterString, this)) {
						//auditing
						if(m_strOldFilterName != strFilterName) {
							long nAuditID = BeginNewAuditEvent();
							AuditEvent(-1, "", nAuditID, aeiFilter, -1, m_strOldFilterName, strFilterName, aepMedium, aetChanged);
						}
						// If we made it to here we have succeeded
						return true;
					}
					else {
						//Failed to commit edit.
						return false;
					}
				}
				else {
					//Editing not supported.
					return false;
				}				
			}
			
		} else {
			// ValidateFilter failed
			return false;
		}
	} NxCatchAll("Error in CApptSelector::SaveFilterToData");
	return false;
}



