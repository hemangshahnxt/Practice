// DocBar.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "DocBar.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "MainFrm.h"
#include "NxTabView.h"
#include "dynUXTheme.h"
#include <afxdtctl.h>
#include "DateTimeUtils.h"
#include "internationalUtils.h"
#include "MultiSelectDlg.h"
#include "MarketingRc.h"
#include "MarketFilterPickerDlg.h"
#include "FilterEditDLg.h"
#include "Groups.h"
#include "Filter.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

#define IDW_LOCATION_COMBO IDW_COMBO + 1
#define IDW_PROVIDER_COMBO IDW_COMBO
#define IDW_PAT_FILTER_COMBO IDW_COMBO + 2
#define IDW_DATE_OPTION_COMBO IDW_COMBO + 3
#define IDW_CATEGORY_COMBO IDW_COMBO + 4
#define IDW_RESP_COMBO IDW_COMBO + 5


// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate control ID range (0x0008 -> 0xDFFF / 8 -> 57343)
#define IDC_LOCATION_LABEL_TOOLBAR 54571
#define IDC_PROVIDER_LABEL_TOOLBAR 54572
#define IDC_PAT_FILTER_LABEL_TOOLBAR 54573
#define IDC_DATE_LABEL_TOOLBAR 54574
#define IDC_FROM_LABEL_TOOLBAR 54575
#define IDC_TO_LABEL_TOOLBAR 54576
#define IDC_CATEGORY_LABEL_TOOLBAR 54577
#define IDC_RESP_LABEL_TOOLBAR 54578
#define IDC_PAT_FILTER_BUTTON_TOOLBAR 54582


#define IDC_MULTI_PROV_LIST_TOOLBAR  54579
#define IDC_MULTI_LOC_LIST_TOOLBAR  54580
//#define IDC_MULTI_FILTER_LIST_TOOLBAR  54581

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CDocBar
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

CDocBar::CDocBar()
{
	m_pFont = NULL;
	m_fromChanged = false;
	m_toChanged = false;
	m_nDocCurSel = GetPropertyInt("lastDoc", 0);
	m_nLocCurSel = GetPropertyInt("lastLoc", 0);
	m_nCatCurSel = GetPropertyInt("lastCat", 0);
	m_dwHidden = 0;

	m_nUseMarketing = -2;

	m_strMultiProvString = "";
	m_strMultiLocString = "";

	m_nPatFilterID = -1;
	
}

// (a.walling 2007-05-11 12:33) - PLID 4850 - Reinit the state of this toolbar
void CDocBar::OnUserChanged()
{
	try {
		m_fromChanged = false;
		m_toChanged = false;
		m_nDocCurSel = GetPropertyInt("lastDoc", 0);
		m_nLocCurSel = GetPropertyInt("lastLoc", 0);
		m_nCatCurSel = GetPropertyInt("lastCat", 0);
		m_dwHidden = 0;

		m_nUseMarketing = -2;

		m_strMultiProvString = "";
		m_strMultiLocString = "";

		m_nPatFilterID = -1;

		m_dwAllowedDateFilters.RemoveAll();
		m_dwAllowedLocationFilters.RemoveAll();
		m_dwAllowedProviderFilters.RemoveAll();

		m_dwProvIDs.RemoveAll();
		m_dwLocIDs.RemoveAll();

		m_Loc.SetColor(GetSysColor(COLOR_3DFACE));
		m_Loc.SetText("Patient Location");
		m_Loc.SetType(dtsHyperlink);

		m_nxlLocationLabel.SetColor(GetSysColor(COLOR_3DFACE));
		m_nxlLocationLabel.SetText("");
		m_nxlLocationLabel.SetType(dtsHyperlink);
		m_nxlLocationLabel.ShowWindow(SW_HIDE);
		m_locCombo->CurSel = 0;

		m_wndLocCombo.ShowWindow(SW_SHOW); //rq

		m_Prov.SetColor(GetSysColor(COLOR_3DFACE));
		m_Prov.SetText("Patient Provider");
		m_Prov.SetType(dtsHyperlink);

		m_nxlProviderLabel.SetColor(GetSysColor(COLOR_3DFACE));
		m_nxlProviderLabel.SetText("");
		m_nxlProviderLabel.SetType(dtsHyperlink);
		m_nxlProviderLabel.ShowWindow(SW_HIDE);

		m_wndDocCombo.ShowWindow(SW_SHOW); 
		m_docCombo->CurSel = 0; // text

		m_wndPatFilterCombo.ShowWindow(SW_SHOW);
		m_nPatFilterID = -1;
		m_PatFilterCombo->CurSel = 0; // text

		m_Date.SetColor(GetSysColor(COLOR_3DFACE));
		m_Date.SetText("First Contact Date");
		m_Date.SetType(dtsHyperlink);

		m_wndDateCombo.ShowWindow(SW_SHOW);
		m_DateCombo->PutCurSel(2); // custom

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		dtNow.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);	//remove the time
		m_ToDate.SetValue(_variant_t(dtNow));

		// (a.walling 2012-02-29 09:55) - PLID 48479 - Fails on leap years
		//dtNow.SetDate(dtNow.GetYear() - 1, dtNow.GetMonth(), dtNow.GetDay());
		dtNow -= COleDateTimeSpan(365, 0, 0, 0);
		m_FromDate.SetValue(_variant_t(dtNow));

		m_categoryCombo->CurSel = 0; //text
		m_wndCatCombo.ShowWindow(SW_SHOW);

		m_RespCombo->CurSel = 0;
		m_wndRespCombo.ShowWindow(SW_SHOW);

		ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn(0, (long)2), FALSE);
	} NxCatchAllThrow("Error initializing marketing toolbar");
}

CDocBar::~CDocBar()
{
	SetPropertyInt("lastDoc", GetDoc());
	SetPropertyInt("lastLoc", GetLoc());
	SetPropertyInt("lastCat", GetCategory());

	if (m_pFont)
		delete m_pFont;

	m_searchButtonFont.DeleteObject();
}

//	ON_EVENT(CDocBar, IDC_DOCFROM, 2 /* Change */, OnChangeFilteredFromDate, VTS_NONE)
//	ON_EVENT(CDocBar, IDC_DOCTO, 2 /* Change */, OnChangeFilteredToDate, VTS_NONE)

// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
BEGIN_MESSAGE_MAP(CDocBar, CDialogBar)
	//{{AFX_MSG_MAP(CDocBar)
	ON_WM_ENABLE()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOCFROM, OnChangeFilteredFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOCTO, OnChangeFilteredToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DOCFROM, OnPullupFrom)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DOCTO, OnPullupTo)
	ON_NOTIFY(NM_KILLFOCUS, IDC_DOCFROM, OnKillfocusFrom)
	ON_NOTIFY(NM_KILLFOCUS, IDC_DOCTO, OnKillfocusTo)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_PAT_FILTER_BUTTON_TOOLBAR, OnEditFilter)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_UPDATE_COMMAND_UI(IDC_PAT_FILTER_BUTTON_TOOLBAR, &CDocBar::OnUpdatePatFilterButton) //j.camacho plid 58876
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDocBar, CDialogBar)
	//{{AFX_EVENTSINK_MAP(CDocBar)
	//ON_EVENT(CDocBar, IDW_COMBO_CAT, 2 /* SelectionChange */, OnSelectionChangeCombo, VTS_I4)
	//ON_EVENT(CDocBar, IDW_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedDocCombo, VTS_I2)
	//ON_EVENT(CDocBar, IDW_COMBO2, 18 /* RequeryFinished */, OnRequeryFinishedLocationCombo, VTS_I2)
	//ON_EVENT(CDocBar, IDW_COMBO_CAT, 18 /* RequeryFinished */, OnRequeryFinishedCatCombo, VTS_I2)
	ON_EVENT(CDocBar, IDW_PAT_FILTER_COMBO, 16 /* SelChosen */, OnSelChosenPatFilterList, VTS_I4)
	ON_EVENT(CDocBar, IDW_PROVIDER_COMBO, 16 /* SelChosen */, OnSelChosenFilteredProviderList, VTS_I4)
	ON_EVENT(CDocBar, IDW_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenFilteredLocationList, VTS_I4)
	ON_EVENT(CDocBar, IDW_DATE_OPTION_COMBO, 16 /* SelChosen */, OnSelChosenFilteredDateOptionList, VTS_I4)
	ON_EVENT(CDocBar, IDW_CATEGORY_COMBO, 16 /* SelChosen */, OnSelChosenFilteredCategoryList, VTS_I4)
	ON_EVENT(CDocBar, IDW_RESP_COMBO, 16 /* SelChosen */, OnSelChosenFilteredRespList, VTS_I4)
	ON_EVENT(CDocBar, IDW_LOCATION_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedLocationCombo, VTS_I2)
	ON_EVENT(CDocBar, IDW_PROVIDER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedDocCombo, VTS_I2)
	ON_EVENT(CDocBar, IDW_PAT_FILTER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPatFilterCombo, VTS_I2)
	ON_EVENT(CDocBar, IDW_CATEGORY_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCategoryCombo, VTS_I2)
	ON_EVENT(CDocBar, IDW_DATE_OPTION_COMBO, 1 /* SelChanging */, OnSelChangingFilteredDateOptionList, VTS_PI4)	

	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CDocBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddMultEmrDataItems)
	DDX_Control(pDX, IDC_DOCFROM, m_FromDate);
	DDX_Control(pDX, IDC_DOCTO, m_ToDate);
	//}}AFX_DATA_MAP
}
/////////////////////////////////////////////////////////////////////////////
// CDocBar message handlers


//Ripped from NxDialog since this can't be an nxdialog since its a toolbar
void CDocBar::InvalidateDlgItem(int nID, bool bErase /*=TRUE*/)
{
	CRect rc;
	CWnd *p;

	p = GetDlgItem(nID);
	ASSERT(p && p->GetSafeHwnd());
	p->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc, bErase);
}

int CDocBar::GetDoc()
{
	_variant_t var;

	//TES 3/1/2004: Datalist throws exceptions now.
	if(m_docCombo->GetCurSel() == -1) return -1;

	var = m_docCombo->GetValue(m_docCombo->GetCurSel(),0);
	if (var.vt != VT_EMPTY)
		return var.lVal;
	else return - 1;
}

void CDocBar::SetDoc(long id)
{
	m_docCombo->SetSelByColumn(0, id);
}

void CDocBar::ScrollDoc(int i)
{
	if(m_docCombo->CurSel + i >= 0 && m_docCombo->CurSel + i < m_docCombo->GetRowCount()) 
		m_docCombo->SetSelByColumn(0, m_docCombo->GetValue(m_docCombo->CurSel + i, 0));
}

int CDocBar::GetDocCount()
{
	return m_docCombo->GetRowCount();
}

int CDocBar::GetDocIndex()
{
	return m_docCombo->CurSel;
}

int CDocBar::GetLoc()
{
	_variant_t var;

	//TES 3/1/2004: Datalist throws exceptions now.
	if(m_locCombo->GetCurSel() == -1) return -1;

	var = m_locCombo->GetValue(m_locCombo->GetCurSel(),0);
	if (var.vt != VT_EMPTY)
		return var.lVal;
	else return - 1;
}


BOOL CDocBar::CreateComboBox()
{
	UpdateData(FALSE);
	CRect	rect, labelRect, comboRect;
	const int comboWidth = 132;
	const int comboHeight = 10;//not the height of the box, but the toolbar menu around it
	const int dateWidth = 89;
	long nRightCounter = 0;
	long nDateRight = 91;
	const int nComboRight = 135;
	const int nLabelTop = 3;
	const int nLabelHeight = 15;
	const int nComboTop = 20;
	const int nComboBottom = 50;
	const int nLeftMargin = 5;
	

	LOGFONT lf;
	if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
		m_searchButtonFont.CreateFontIndirect(&lf);
	}

	//create the label for it first
	rect = CRect(0,0,comboWidth, TOOLBARHEIGHT);
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	labelRect.left = labelRect.left + nLeftMargin;
	if(m_Loc.Create("Location", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT|SS_NOTIFY, labelRect, this, IDC_LOCATION_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_Loc.SetColor(GetSysColor(COLOR_3DFACE));
	m_Loc.SetText("Patient Location");
	m_Loc.SetType(dtsHyperlink);
	
	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_Loc.ModifyStyleEx(0, WS_EX_TRANSPARENT);

	
	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;
	rect.left = rect.left + nLeftMargin;


	//draw the label underneath the combo
	if(m_nxlLocationLabel.Create("", WS_CHILD|WS_GROUP|SS_LEFT|SS_NOTIFY, rect, this, IDC_MULTI_LOC_LIST_TOOLBAR) == 0)
		return FALSE;
	m_nxlLocationLabel.SetColor(GetSysColor(COLOR_3DFACE));
	m_nxlLocationLabel.SetText("");
	m_nxlLocationLabel.SetType(dtsHyperlink);
	m_nxlLocationLabel.ShowWindow(SW_HIDE);

	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_nxlLocationLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT);



	if (m_wndLocCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "DocBar", WS_CHILD, rect, this, IDW_LOCATION_COMBO)) 
	{	m_locCombo = m_wndLocCombo.GetControlUnknown();
		ASSERT(m_locCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_locCombo) 
		{	try 
			{	// Set up the columns
				IColumnSettingsPtr(m_locCombo->GetColumn(m_locCombo->InsertColumn(0, _T("ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_locCombo->GetColumn(m_locCombo->InsertColumn(1, _T("Name"), _T("Location"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				m_locCombo->FromClause = _T("LocationsT");
				//r.wilson 5/23/2012 - PLID 49383 - We want to be able to filter on unmanaged locations as well

				/* If you change this where clause, be sure to change it in CDocBar::OnMultiSelectLocations() as well!*/
				m_locCombo->WhereClause = _T("Active = 1 AND TypeID = 1");

				// We want it to be a combo, not a datalist
				m_locCombo->IsComboBox = TRUE;
				m_locCombo->DisplayColumn = "[1]";
				m_locCombo->GetColumn(1)->PutSortPriority(0);
				m_locCombo->GetColumn(1)->PutSortAscending(TRUE);
				m_locCombo->DropDownWidth = 400;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
				m_locCombo->AdoConnection = GetRemoteDataSnapshot();
				m_locCombo->Requery();
				
				//m_locCombo->SetSelByColumn(0, _variant_t());
			} NxCatchAll("Error in creating location combo");
			// We have success so display the window
			m_wndLocCombo.ShowWindow(SW_SHOW);
				
			
			
		}
	}
	else 
	{	// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}

	
	//Create the Provider combo box

	//work around because its not sizing properly
	nRightCounter += nComboRight;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += comboWidth;
		
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_Prov.Create("Provider", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT|SS_NOTIFY, labelRect, this, IDC_PROVIDER_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_Prov.SetColor(GetSysColor(COLOR_3DFACE));
	m_Prov.SetText("Patient Provider");
	m_Prov.SetType(dtsHyperlink);
	
	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_Prov.ModifyStyleEx(0, WS_EX_TRANSPARENT);
	

	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;	

	//draw the label underneath the combo
	if(m_nxlProviderLabel.Create("", WS_CHILD|WS_GROUP|SS_LEFT|SS_NOTIFY, rect, this, IDC_MULTI_PROV_LIST_TOOLBAR) == 0)
		return FALSE;
	m_nxlProviderLabel.SetColor(GetSysColor(COLOR_3DFACE));
	m_nxlProviderLabel.SetText("");
	m_nxlProviderLabel.SetType(dtsHyperlink);
	m_nxlProviderLabel.ShowWindow(SW_HIDE);

	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_nxlProviderLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT);


	//Create the doctor combo box
	if (m_wndDocCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "DocBar", WS_CHILD, rect, this, IDW_PROVIDER_COMBO)) 
	{
		m_docCombo = m_wndDocCombo.GetControlUnknown();
		ASSERT(m_docCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_docCombo) 
		{
			try 
			{
				// Set up the columns
				IColumnSettingsPtr(m_docCombo->GetColumn(m_docCombo->InsertColumn(0, _T("ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_docCombo->GetColumn(m_docCombo->InsertColumn(1, _T("[Last] + ', ' + [First] + ' ' + Title"), _T("Provider"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				m_docCombo->FromClause = _T("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID AND PersonT.ID = ProvidersT.PersonID");
				m_docCombo->WhereClause = _T("PersonT.Archived = 0");
				// We want it to be a combo, not a datalist
				m_docCombo->IsComboBox = TRUE;
				m_docCombo->DisplayColumn = "[1]";
				m_docCombo->GetColumn(1)->PutSortPriority(1);
				m_docCombo->GetColumn(1)->PutSortAscending(TRUE);
				m_docCombo->DropDownWidth = 400;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				m_docCombo->AdoConnection = GetRemoteDataSnapshot();
				m_docCombo->Requery();

			
				//m_docCombo->SetSelByColumn(0, _variant_t());
			} NxCatchAll("Error in creating doctor combo");
			// We have success so display the window
			m_wndDocCombo.ShowWindow(SW_SHOW);
			// Done, return success

			//JJ - trying to speed up the loading of the program, the previous SetSel calls were very
			//inefficient so I made this bar mimic the other two
		}
	}
	else 
	{	// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}

	

	//PLID 15261 changing the patient coord box to be a patient filter that pulls from letterwriting
	//Create the Patient Filter combo box
	nRightCounter += nComboRight;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += nComboRight + 1;

	
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_PatFilter.Create("Patient Filter", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, labelRect, this, IDC_PAT_FILTER_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_PatFilter.SetFont (&m_searchButtonFont, FALSE);
	
	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_PatFilter.ModifyStyleEx(0, WS_EX_TRANSPARENT);


	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;


	if (m_wndPatFilterCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "DocBar", WS_CHILD, rect, this, IDW_PAT_FILTER_COMBO)) 
	{	m_PatFilterCombo = m_wndPatFilterCombo.GetControlUnknown();
		ASSERT(m_PatFilterCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_PatFilterCombo) 
		{	try 
			{	// Set up the columns
				IColumnSettingsPtr(m_PatFilterCombo->GetColumn(m_PatFilterCombo->InsertColumn(0, _T("ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_PatFilterCombo->GetColumn(m_PatFilterCombo->InsertColumn(1, _T("Name"), _T("Name"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_PatFilterCombo->GetColumn(m_PatFilterCombo->InsertColumn(2, _T("Filter"), _T("Filter"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				//TES 3/27/2007 - PLID 20528 - Added the filters' Created and Modified dates.
				IColumnSettingsPtr(m_PatFilterCombo->GetColumn(m_PatFilterCombo->InsertColumn(3, _T("CreatedDate"), _T("Created"), 0, csVisible|csWidthAuto)))->FieldType = cftDateAndTime;
				IColumnSettingsPtr(m_PatFilterCombo->GetColumn(m_PatFilterCombo->InsertColumn(4, _T("ModifiedDate"), _T("Modified"), 0, csVisible|csWidthData)))->FieldType = cftDateAndTime;
				m_PatFilterCombo->FromClause = _T("FiltersT");
				// We want it to be a combo, not a datalist
				m_PatFilterCombo->IsComboBox = TRUE;
				m_PatFilterCombo->WhereClause = _T("FiltersT.Type = 1");
				m_PatFilterCombo->DisplayColumn = "[1]";
				m_PatFilterCombo->GetColumn(1)->PutSortPriority(0);
				m_PatFilterCombo->GetColumn(1)->PutSortAscending(TRUE);
				m_PatFilterCombo->DropDownWidth = 400;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				m_PatFilterCombo->AdoConnection = GetRemoteDataSnapshot();
				//MessageBox(m_PatFilterCombo->GetSqlPending());
				m_PatFilterCombo->Requery();
				//m_categoryCombo->SetSelByColumn(0, _variant_t());
			} NxCatchAll("Error in creating Patient Filter combo.");
			// We have success so display the window
			m_wndPatFilterCombo.ShowWindow(SW_SHOW);
			// Done, return success
		
		}
	}
	else 
	{	// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}

	//now we need an elipsis button that will make a new filter
	nRightCounter += nComboRight;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += 20;

	if (m_btnPatFilter.Create("...", WS_CHILD | WS_GROUP | WS_VISIBLE | SS_LEFT | SS_NOTIFY, rect, this, IDC_PAT_FILTER_BUTTON_TOOLBAR) == 0)
		return FALSE;
	//Create the Date combo box
	nRightCounter += 23;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += comboWidth;
	
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_Date.Create("Date", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT|SS_NOTIFY, labelRect, this, IDC_DATE_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_Date.SetColor(GetSysColor(COLOR_3DFACE));
	m_Date.SetText("First Contact Date");
	m_Date.SetType(dtsHyperlink);

	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_Date.ModifyStyleEx(0, WS_EX_TRANSPARENT);


	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;
	if (m_wndDateCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "DocBar", WS_CHILD, rect, this, IDW_DATE_OPTION_COMBO)) 
	{	m_DateCombo = m_wndDateCombo.GetControlUnknown();
		ASSERT(m_DateCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_DateCombo) 
		{	try 
			{	// Set up the columns
				IColumnSettingsPtr(m_DateCombo->GetColumn(m_DateCombo->InsertColumn(0, _T("ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_DateCombo->GetColumn(m_DateCombo->InsertColumn(1, _T("Name"), _T("Category"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				// We want it to be a combo, not a datalist
				m_DateCombo->IsComboBox = TRUE;
				m_DateCombo->DisplayColumn = "[1]";
				//m_DateCombo->GetColumn(0)->PutSortPriority(0);
				//m_DateCombo->GetColumn(0)->PutSortAscending(TRUE);
				m_DateCombo->DropDownWidth = 400;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				m_DateCombo->AdoConnection = GetRemoteDataSnapshot();
				m_DateCombo->AllowSort = FALSE;
				
				// (j.gruber 2010-09-08 10:23) - PLID 37425 - changed to enum and added Yesterday
				IRowSettingsPtr pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvAll);
				pRow->PutValue(1, _variant_t("All"));
				m_DateCombo->InsertRow(pRow, 0);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvOneYear);
				pRow->PutValue(1, _variant_t("One Year"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvCustom);
				pRow->PutValue(1, _variant_t("Custom"));
				m_DateCombo->AddRow(pRow);

				
				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvSeparator);
				pRow->PutValue(1, _variant_t("--------------------------------"));
				pRow->PutBackColorSel(RGB(255,255,255));
				pRow->PutForeColorSel(RGB(0,0,0));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvToday);
				pRow->PutValue(1, _variant_t("Today"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisWeek);
				pRow->PutValue(1, _variant_t("This Week"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisMonth);
				pRow->PutValue(1, _variant_t("This Month"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisQuarter);
				pRow->PutValue(1, _variant_t("This Quarter"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisYear);
				pRow->PutValue(1, _variant_t("This Year"));
				m_DateCombo->AddRow(pRow);
				
				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvSeparator);
				pRow->PutValue(1, _variant_t("--------------------------------"));
				pRow->PutBackColorSel(RGB(255,255,255));
				pRow->PutForeColorSel(RGB(0,0,0));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisMonthToDate);
				pRow->PutValue(1, _variant_t("This Month To Date"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisQuarterToDate);
				pRow->PutValue(1, _variant_t("This Quarter To Date"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvThisYearToDate);
				pRow->PutValue(1, _variant_t("This Year To Date"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvSeparator);
				pRow->PutValue(1, _variant_t("--------------------------------"));
				pRow->PutBackColorSel(RGB(255,255,255));
				pRow->PutForeColorSel(RGB(0,0,0));
				m_DateCombo->AddRow(pRow);

				// (j.gruber 2010-09-08 10:03) - PLID 37425 - added Yesterday
				//Yesterday
				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvYesterday);
				pRow->PutValue(1, _variant_t("Yesterday"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvLastWeek);
				pRow->PutValue(1, _variant_t("Last Week"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvLastMonth);
				pRow->PutValue(1, _variant_t("Last Month"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvLastQuarter);
				pRow->PutValue(1, _variant_t("Last Quarter"));
				m_DateCombo->AddRow(pRow);

				pRow = m_DateCombo->GetRow(-1);
				pRow->PutValue(0, (long)emdvLastYear);
				pRow->PutValue(1, _variant_t("Last Year"));
				m_DateCombo->AddRow(pRow);

				//m_categoryCombo->SetSelByColumn(0, _variant_t());
			} NxCatchAll("Error in creating category combo.");
			// We have success so display the window
			m_wndDateCombo.ShowWindow(SW_SHOW);
			// Done, return success
		
		}
	}
	else 
	{	// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}


	//fromText
	//SetButtonInfo(5, ID_FROM, TBBS_SEPARATOR, dateWidth/2);
	//rect.top += comboHeight / 2;
	//rect.bottom -= comboHeight / 2;
	//m_fromText.Create("From", WS_VISIBLE|WS_CHILD|SS_CENTER, rect, this, IDC_STATIC);
	//m_fromText.SetFont (&m_searchButtonFont, FALSE);

	//DRT 1/12/2005 - PLID 15274 - We have to set the to date!
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	dtNow.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);	//remove the time
	m_ToDate.SetValue(_variant_t(dtNow));
	m_FromDate.SetValue(_variant_t(dtNow));

	//from
	nRightCounter += nComboRight;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += dateWidth;
	
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_From.Create("From", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, labelRect, this, IDC_FROM_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_From.SetFont (&m_searchButtonFont, FALSE);
	
	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_From.ModifyStyleEx(0, WS_EX_TRANSPARENT);

	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;
	//m_FromDate.Create("From", WS_VISIBLE|WS_CHILD|WS_TABSTOP|DTS_SHORTDATEFORMAT, rect, this, IDC_DOCFROM);
	m_FromDate.MoveWindow(rect);
	COleDateTime dt;
	dt = VarDateTime(m_FromDate.GetValue());
	// (a.walling 2012-02-29 09:55) - PLID 48479 - Fails on leap years
	//dt.SetDate(dt.GetYear() - 1, dt.GetMonth(), dt.GetDay());
	dt -= COleDateTimeSpan(365, 0, 0, 0);
	m_FromDate.SetValue(_variant_t(dt));

	COleDateTime dtMin, dtMax;
	dtMin.SetDate(1753,1,1);
	dtMax.SetDate(5000,12,31);
	m_FromDate.SetMinDate(dtMin);
	m_FromDate.SetMaxDate(dtMax);

	//toText
	//SetButtonInfo(7, ID_FROM, TBBS_SEPARATOR, dateWidth/2);
	//GetItemRect(7, &rect);
	//rect.top += comboHeight / 2;
	//rect.bottom -= comboHeight / 2;
	//m_toText.Create("To", WS_VISIBLE|WS_CHILD|SS_CENTER, rect, this, IDC_STATIC);
	//m_toText.SetFont (&m_searchButtonFont, FALSE);

	//to
	nRightCounter += nDateRight;
	rect.right  = nRightCounter;
	rect.left = rect.right;
	rect.right += dateWidth;
	
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_To.Create("To", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, labelRect, this, IDC_TO_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_To.SetFont (&m_searchButtonFont, FALSE);
	
	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_To.ModifyStyleEx(0, WS_EX_TRANSPARENT);

	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;
	//m_ToDate.Create("To", WS_VISIBLE|WS_CHILD|WS_TABSTOP|DTS_SHORTDATEFORMAT, rect, this, IDC_DOCTO);
	m_ToDate.MoveWindow(rect);

	m_ToDate.SetMinDate(dtMin);
	m_ToDate.SetMaxDate(dtMax);

	//Create the category combo box
	nRightCounter += nDateRight;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += comboWidth;
	
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_Category.Create("Category", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, labelRect, this, IDC_CATEGORY_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_Category.SetFont (&m_searchButtonFont, FALSE);

	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_Category.ModifyStyleEx(0, WS_EX_TRANSPARENT);

	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;
	if (m_wndCatCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "DocBar", WS_CHILD, rect, this, IDW_CATEGORY_COMBO)) 
	{	m_categoryCombo = m_wndCatCombo.GetControlUnknown();
		ASSERT(m_categoryCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_categoryCombo) 
		{	try 
			{	// Set up the columns
				IColumnSettingsPtr(m_categoryCombo->GetColumn(m_categoryCombo->InsertColumn(0, _T("ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_categoryCombo->GetColumn(m_categoryCombo->InsertColumn(1, _T("Name"), _T("Category"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				m_categoryCombo->FromClause = _T("CategoriesT");
				// We want it to be a combo, not a datalist
				m_categoryCombo->IsComboBox = TRUE;
				m_categoryCombo->DisplayColumn = "[1]";
				m_categoryCombo->GetColumn(1)->PutSortPriority(0);
				m_categoryCombo->GetColumn(1)->PutSortAscending(TRUE);
				m_categoryCombo->DropDownWidth = 400;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				m_categoryCombo->AdoConnection = GetRemoteDataSnapshot();
				m_categoryCombo->Requery();
				//m_categoryCombo->SetSelByColumn(0, _variant_t());
			} NxCatchAll("Error in creating category combo.");
			// We have success so display the window
			m_wndCatCombo.ShowWindow(SW_SHOW);
			// Done, return success
		
		}
	}
	else 
	{	// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}

	//Create the Resp combo box
	nRightCounter += nComboRight;
	rect.right = nRightCounter;
	rect.left = rect.right;
	rect.right += comboWidth;
	//SetButtonInfo(7, IDW_COMBO, TBBS_SEPARATOR, comboWidth);	
	//create the label for it first
	labelRect = rect;
	labelRect.top = nLabelTop;
	labelRect.bottom = labelRect.top + nLabelHeight;
	if(m_Resp.Create("Revenue Source", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, labelRect, this, IDC_RESP_LABEL_TOOLBAR) == 0)
		return FALSE;
	m_Resp.SetFont (&m_searchButtonFont, FALSE);

	// (a.walling 2008-04-21 15:19) - PLID 29642 - Use WS_EX_TRANSPARENT style
	m_Resp.ModifyStyleEx(0, WS_EX_TRANSPARENT);

	//now the combo
	rect.top = nComboTop;
	rect.bottom = nComboBottom;
	if (m_wndRespCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "DocBar", WS_CHILD, rect, this, IDW_RESP_COMBO)) 
	{	m_RespCombo = m_wndRespCombo.GetControlUnknown();
		ASSERT(m_RespCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_RespCombo) 
		{	try 
			{	// Set up the columns
				IColumnSettingsPtr(m_RespCombo->GetColumn(m_RespCombo->InsertColumn(0, _T("ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_RespCombo->GetColumn(m_RespCombo->InsertColumn(1, _T("Name"), _T("Category"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				// We want it to be a combo, not a datalist
				m_RespCombo->IsComboBox = TRUE;
				m_RespCombo->DisplayColumn = "[1]";
				m_RespCombo->GetColumn(1)->PutSortPriority(0);
				m_RespCombo->GetColumn(1)->PutSortAscending(TRUE);
				m_RespCombo->DropDownWidth = 400;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				m_RespCombo->AdoConnection = GetRemoteDataSnapshot();
				
				IRowSettingsPtr pRow;
				pRow = m_RespCombo->GetRow(-1);
				pRow->PutValue(0, (long)-1);
				pRow->PutValue(1, _variant_t("<All Sources>"));
				m_RespCombo->InsertRow(pRow, 0);

				pRow = m_RespCombo->GetRow(-1);
				pRow->PutValue(0, (long)1);
				pRow->PutValue(1, _variant_t("<Patient Responsibility>"));
				m_RespCombo->InsertRow(pRow, 1);

				pRow = m_RespCombo->GetRow(-1);
				pRow->PutValue(0, (long)2);
				pRow->PutValue(1, _variant_t("<Insurance Responsibility>"));
				m_RespCombo->InsertRow(pRow, 2);

				m_RespCombo->CurSel = 0;

				//m_categoryCombo->SetSelByColumn(0, _variant_t());
			} NxCatchAll("Error in creating category combo.");
			// We have success so display the window
			m_wndRespCombo.ShowWindow(SW_SHOW);
			// Done, return success
		
		}
	}
	else 
	{	// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}

	ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn(0, (long)2), FALSE);



	// Turn off themes for this window
	// (a.walling 2008-04-21 15:19) - PLID 29642 - Why?
	/*
	UXTheme uxtheme;
	uxtheme.SetWindowTheme(GetSafeHwnd(), L" ", L" ");
	*/
	return TRUE;
}


void CDocBar::SetDateOptionCombo(MarketDateOptionType mdotSelection) {

	switch (mdotSelection) {
		case mdotAll:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn(0, (long) -1), FALSE);
		break;

		case mdotOneYear:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)1), FALSE);
		break;

		case mdotCustom:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)2), FALSE);
		break;

		case mdotToday:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0,(long) 3), FALSE);
		break;

		case mdotThisWeek:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)4), FALSE);
		break;

		case mdotThisMonth:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)5), FALSE);
		break;

		case mdotThisQuarter:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)6), FALSE);
		break;

		case mdotYear:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)7), FALSE);
		break;

		case mdotThisMTD:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)8), FALSE);
		break;

		case mdotThisQTD:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)9), FALSE);
		break;

		case mdotThisYTD:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)10), FALSE);
		break;

		case mdotLastWeek:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)11), FALSE);
		break;

		case mdotLastMonth:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)12), FALSE);
		break;

		case mdotLastQuarter:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)13), FALSE);
		break;

		case mdotLastYear:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)14), FALSE);
		break;

		default:
			ChangeSelectionFilteredDateOptionList(m_DateCombo->SetSelByColumn( 0, (long)2), FALSE);
		break;

	}
}
void CDocBar::RequeryDoc(BOOL bKeepSelection)
{
	//Remember which doctor is selected.
	if (bKeepSelection) {
		m_nDocCurSel = GetDoc();
	}
	else {
		m_nDocCurSel = -1;
	}

	if ((m_ProvFilter == mfApptProvider) || (m_ProvFilter == mfNoPatApptProvider)) {
		
		//set the datalist to show resources
		IColumnSettingsPtr pCol;
		pCol = m_docCombo->GetColumn(1);
		pCol->PutFieldName("Item");

		m_docCombo->FromClause = "ResourceT";
		m_docCombo->WhereClause = "";
		
	}
	else {
		//set the datalist to show Providers
		IColumnSettingsPtr pCol;
		pCol = m_docCombo->GetColumn(1);
		pCol->PutFieldName("[Last] + ', ' + [First] + ' ' + Title");
		m_docCombo->FromClause = "PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID AND PersonT.ID = ProvidersT.PersonID";
		m_docCombo->WhereClause = "PersonT.Archived = 0";
	}

	//set the label if its there
	m_strMultiProvString = "";
	ShowDlgItem(this, IDC_MULTI_PROV_LIST_TOOLBAR, SW_HIDE);
	ShowDlgItem(this, IDW_PROVIDER_COMBO, SW_SHOW);

	//requery
	m_docCombo->Requery();
}

void CDocBar::RequeryLoc()
{
	//Remember which location is selected.
	m_nLocCurSel = GetLoc();
	m_locCombo->Requery();
}

BOOL CDocBar::PreTranslateMessage(MSG* pMsg) 
{
	/*TES 5/20/03: Brad originally commented this out, with a comment indicating that he didn't know why it was
	//here in the first place, but phrased in such an arrogant manner that I decided to remove it just to keep
	//my blood pressure down.

	// Note: there is only one control in the toolbar that accepts keyboard input,
	// this is the combobox.
	
	// user hit ENTER
	if (pMsg->wParam == VK_RETURN && GetKeyState(VK_RETURN) < 0)
	{
		// extract the text, update combobox lists, and do the search
		CString s1;
		s1 = CString(m_docCombo->GetValue(m_docCombo->GetCurSel(),5).bstrVal);

		if(s1.GetLength()) {
			return TRUE;  // key processed
		} else {
			MessageBeep(MB_ICONEXCLAMATION);
			return TRUE;  // key processed
		}
	}

	// user hit ESC
	if (pMsg->wParam == VK_ESCAPE && GetKeyState(VK_ESCAPE) < 0)
	{
		return TRUE; // key processed
	}

	// user hit DOWN-ARROW
	if (pMsg->wParam == VK_DOWN && GetKeyState(VK_DOWN) < 0)
	{
		return TRUE; // key processed
	}

	return 	CToolBar::PreTranslateMessage(pMsg);	*/
	return CDialogBar::PreTranslateMessage(pMsg);
}

void CDocBar::OnEnable(BOOL bEnable) 
{
	CDialogBar::OnEnable(bEnable);
}

//These refresh the modules
void CDocBar::OnSelectionChangeCombo(long iNewRow)
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
			CWaitCursor cWait;
	//		GetMainFrame()->GetActiveView()->UpdateView();

			// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
			// (c.haag 2007-03-15 17:06) - PLID 24253 - Post it to the parent view, and it will percolate it to the
			// active sheet
			GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, -1, -1);
		}
	}NxCatchAll(__FUNCTION__);
}

/*void CDocBar::OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult) 
{	m_toChanged = true;//only refresh if date is different
	GetMainFrame()->GetActiveView()->UpdateView();
}
*/

/*void CDocBar::OnChange From(NMHDR* pNMHDR, LRESULT* pResult) 
{	m_fromChanged = true;
	GetMainFrame()->GetActiveView()->UpdateView();
	
}*/

void CDocBar::OnPullupFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		if (m_fromChanged)
			m_fromChanged = false;
		else return;
			
		COleDateTime from, to;
		from = VarDateTime(m_FromDate.GetValue());
		to = VarDateTime(m_ToDate.GetValue());
		if (from > to)
			m_ToDate.SetValue(_variant_t(from));
		OnSelectionChangeCombo(0);

		m_DateCombo->SetSelByColumn(0, (long)2);
	}NxCatchAll(__FUNCTION__);
}

void CDocBar::OnPullupTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		if (m_toChanged)
			m_toChanged = false;
		else return;

		COleDateTime from, to;
		from = VarDateTime(m_FromDate.GetValue());
		to = VarDateTime(m_ToDate.GetValue());
		if (from > to)
			m_FromDate.SetValue(_variant_t(to));
		OnSelectionChangeCombo(0);

		m_DateCombo->SetSelByColumn(0, (long)2);
	}NxCatchAll(__FUNCTION__);
}

void CDocBar::OnKillfocusFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnPullupFrom(pNMHDR, pResult);
}

void CDocBar::OnKillfocusTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnPullupTo(pNMHDR, pResult);
}

void CDocBar::SetCategory(long id) {

	m_categoryCombo->SetSelByColumn(0, (long)id);
}

void CDocBar::RequeryCategory() {

	m_nCatCurSel = m_categoryCombo->CurSel;
	m_categoryCombo->Requery();
}


BOOL CDocBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	GetDlgItem(IDC_LOCATION_LABEL_TOOLBAR)->GetWindowRect(rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	GetDlgItem(IDC_DATE_LABEL_TOOLBAR)->GetWindowRect(rc);
	ScreenToClient(&rc);

	if (rc.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	GetDlgItem(IDC_PROVIDER_LABEL_TOOLBAR)->GetWindowRect(rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	if (GetDlgItem(IDC_MULTI_PROV_LIST_TOOLBAR)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_PROV_LIST_TOOLBAR)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (GetDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	/*if (GetDlgItem(IDC_MULTI_COORD_LIST_TOOLBAR)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_COORD_LIST_TOOLBAR)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}*/


		
	return CDialogBar::OnSetCursor(pWnd, nHitTest, message);
}


LRESULT CDocBar::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
	case IDC_DATE_LABEL_TOOLBAR: {
		CMarketFilterPickerDlg *pDlg;
		if(m_Type == -1) {
			pDlg = new CMarketFilterPickerDlg(m_dwAllowedDateFilters, mftDate, this);
		}
		else {
			pDlg = new CMarketFilterPickerDlg(m_Type, mftDate, this);
		}
		pDlg->m_Filter = m_DateFilter;
		if (IDOK == pDlg->DoModal()) {
			m_DateFilter = pDlg->m_Filter;
			
			//change the date field
			m_Date.SetText(GetDisplayNameFromEnum(m_DateFilter));
			Invalidate();
			if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
				CWaitCursor cWait;
//				GetMainFrame()->GetActiveView()->UpdateView();
				// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
				GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, (WPARAM)m_DateFilter, (LPARAM)mftDate);
			}

		}
		delete pDlg;
	}
	break;
	case IDC_LOCATION_LABEL_TOOLBAR: {
		CMarketFilterPickerDlg *pDlg;
		if(m_Type == -1) {
			pDlg = new CMarketFilterPickerDlg(m_dwAllowedLocationFilters, mftLocation, this);
		}
		else {
			pDlg = new CMarketFilterPickerDlg(m_Type, mftLocation, this);
		}
		pDlg->m_Filter = m_LocFilter;
		if (IDOK == pDlg->DoModal()) {
			m_LocFilter = pDlg->m_Filter;
			
			//change the date field
			m_Loc.SetText(GetDisplayNameFromEnum(m_LocFilter));
			Invalidate();
			if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
				CWaitCursor cWait;
//				GetMainFrame()->GetActiveView()->UpdateView();
				// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
				GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, (WPARAM)m_LocFilter, (LPARAM)mftLocation);
			}

		}
		delete pDlg;
	}
	break;
	case IDC_PROVIDER_LABEL_TOOLBAR: {
		CMarketFilterPickerDlg *pDlg;
		if(m_Type == -1) {
			pDlg = new CMarketFilterPickerDlg(m_dwAllowedProviderFilters, mftProvider, this);
		}
		else {
			pDlg = new CMarketFilterPickerDlg(m_Type, mftProvider, this);
		}
		pDlg->m_Filter = m_ProvFilter;

		if (IDOK == pDlg->DoModal()) {
	
			BOOL bNeedRequery = CheckProvRequery(m_ProvFilter, pDlg->m_Filter);
			m_ProvFilter = pDlg->m_Filter;

			if (bNeedRequery) {
				RequeryDoc(FALSE);
			}			

			//change the Provider field
			m_Prov.SetText(GetDisplayNameFromEnum(m_ProvFilter));
			Invalidate();
			if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
				CWaitCursor cWait;
//				GetMainFrame()->GetActiveView()->UpdateView();
				// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
				GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, (WPARAM)m_ProvFilter, (LPARAM)mftProvider);
			}

		}
		delete pDlg;
		}
	break;

	case IDC_MULTI_PROV_LIST_TOOLBAR:
		if (GetDlgItem(IDC_MULTI_PROV_LIST_TOOLBAR)->IsWindowVisible()) {
			//it visible so we can handle it
			if (OnMultiSelectProviders()) {
				//refresh the screen
				if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
					CWaitCursor cWait;
//					GetMainFrame()->GetActiveView()->UpdateView();
					// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
					GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, -1, -1);
				}
			}
		}

	break;

	case IDC_MULTI_LOC_LIST_TOOLBAR:
		if (GetDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR)->IsWindowVisible()) {
			//it visible so we can handle it
			if (OnMultiSelectLocations()) {
				//refresh the screen
				if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
					CWaitCursor cWait;
//					GetMainFrame()->GetActiveView()->UpdateView();
					// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
					GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, -1, -1);
				}
			}
		}

	break;

	/*case IDC_MULTI_COORD_LIST_TOOLBAR:
		if (GetDlgItem(IDC_MULTI_COORD_LIST_TOOLBAR)->IsWindowVisible()) {
			//it visible so we can handle it
			if (OnMultiSelectPatCoords()) {
				//refresh the screen
				if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
					CWaitCursor cWait;
					GetMainFrame()->GetActiveView()->UpdateView();
				}
			}
		}

	break;*/


	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}


void CDocBar::SetType(int mktType) {

	m_Type = mktType;
	EnsureFilter(mftDate);
	EnsureFilter(mftLocation);
	EnsureFilter(mftProvider);

}

BOOL CDocBar::CheckProvRequery(MarketFilter mfFilterOld, MarketFilter mfFilterNew) {

	//check if we are now using resources or not
	if ((mfFilterNew == mfApptProvider) || (mfFilterNew == mfNoPatApptProvider)) {
		//were we using them before?
		if ((mfFilterOld == mfApptProvider) || (mfFilterOld == mfNoPatApptProvider)) {
			//we were using them before, we don't need to requery
			return FALSE;
		}
		else {
			//we need to requery
			return TRUE;
		}
	}
	else {
		//check to see if we weren't using resources before
		if ((mfFilterOld == mfApptProvider) || (mfFilterOld == mfNoPatApptProvider)) {
			//were using them before, but now we aren't, so requery
			return TRUE;
		}
		else {
			//we are ok
			return FALSE;
		}
	}
}


void CDocBar::SetFilter(MarketFilter mktFilter, MarketFilterType mfType /*= mftDate*/) {
	switch (mfType) {
		case mftDate:
			m_DateFilter = mktFilter;
			m_Date.SetText(GetDisplayNameFromEnum(mktFilter));
			m_Date.Invalidate();
		break;

		case mftLocation:
			m_LocFilter = mktFilter;
			m_Loc.SetText(GetDisplayNameFromEnum(mktFilter));
			m_Loc.Invalidate();	
		break;

		case mftProvider: {

			//check to see if we need to requery
			BOOL bNeedRequery = CheckProvRequery(m_ProvFilter, mktFilter);

			m_ProvFilter = mktFilter;
			m_Prov.SetText(GetDisplayNameFromEnum(mktFilter));
			m_Prov.Invalidate();	

			if (bNeedRequery) {
				RequeryDoc(FALSE);
			}
						  }
		break;

		default:
			ASSERT(FALSE);
		break;

	}


}

CString CDocBar::GetFromDate() {

	return FormatDateTimeForSql(VarDateTime(m_FromDate.GetValue()), dtoDate);
}


CString CDocBar::GetToDate() {

	return FormatDateTimeForSql(VarDateTime(m_ToDate.GetValue()), dtoDate);
}

CString CDocBar::GetProviderString() {

	CString strProv;
	//generate provider "IN" clause
	if(m_dwProvIDs.GetSize() == 0) {
		strProv = "";
	}
	else {
		strProv = "(";
		for(int i=0; i<m_dwProvIDs.GetSize(); i++) {
			CString str;
			str.Format("%li,",(long)m_dwProvIDs.GetAt(i));
			strProv += str;
		}
		strProv.SetAt(strProv.GetLength()-1,')');	
	}

	return strProv;
}

CString CDocBar::GetLocationString() {

	CString strLoc;
	//generate location "IN" clause
	if(m_dwLocIDs.GetSize() == 0)
		strLoc = "";
	else {
		strLoc = "(";
		for(int i=0; i<m_dwLocIDs.GetSize(); i++) {
			CString str;
			str.Format("%li,",(long)m_dwLocIDs.GetAt(i));
			strLoc += str;
		}
		strLoc.SetAt(strLoc.GetLength()-1,')');	
	}
	return strLoc;

}

// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
CString CDocBar::GetPatientFilterString(ADODB::_ConnectionPtr pCon, CString &strPatientTempTable) {

	try {
		
		if ((m_nPatFilterID == -1) || (m_nPatFilterID == -2)) {
			return "";
		}
		else {
			CString strPatFilter, strFilter, strFilterWhere, strFilterFrom;
			//generate the filter string

			// (c.haag 2007-02-20 13:18) - PLID 24749 - If there is no valid selection,
			// just return an empty filter
			long nCurSel = m_PatFilterCombo->CurSel;
			if (-1 == nCurSel)
				return "";

			strFilter = VarString(m_PatFilterCombo->GetValue(nCurSel, 2));
		

			//Decrypt it
			if(!CFilter::ConvertFilterStringToClause(m_nPatFilterID, strFilter, 1, &strFilterWhere, &strFilterFrom)) {
				MsgBox("Report could not be generated because it uses an invalid filter.");
				return "";
			}

			// (j.jones 2010-07-19 15:24) - PLID 39053 - create a temp table with these patient IDs,
			// and be sure we use the passed-in connection (if strPatientTempTable is not empty,
			// it means that the caller is re-using this filter multiple times, so don't change it)
			if(strPatientTempTable.IsEmpty()) {
				if(pCon == GetRemoteData()) {
					//This has to be a global temp table available to all connections or else the
					//Effectiveness tab won't work since datalists are almost always different connections.
					//It is dropped when all connections that reference it have closed.				
					strPatientTempTable.Format("##TempLWPatients%lu", GetTickCount());
				}
				else {
					//if a snapshot connection, use a normal temp table
					strPatientTempTable.Format("#TempLWPatients%lu", GetTickCount());
				}

				CString strSql;
				strSql.Format("CREATE TABLE %s (PatientID INT NOT NULL) \r\n"
					"INSERT INTO %s (PatientID) SELECT PersonT.ID FROM %s WHERE %s", strPatientTempTable, strPatientTempTable, strFilterFrom, strFilterWhere);

				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				NxAdo::PushMaxRecordsWarningLimit pmr(5000);
				ExecuteSqlStd(pCon, strSql, NULL, adCmdText);
			}

			strPatFilter.Format(" (SELECT DISTINCT PatientID FROM %s) ", strPatientTempTable);

			return strPatFilter;
		}
	
	}NxCatchAll("Error Generating Filter String");
	return "";
	
}



MarketFilter CDocBar::GetFilterType(MarketFilterType mft) {

	switch (mft) {

		case mftDate:
			return m_DateFilter;
		break;

		case mftLocation:
			return m_LocFilter;
		break;

		case mftProvider:
			return m_ProvFilter;
		break;

		default:
			ASSERT(FALSE);
			return mfUnknown;
		break;
	}

}

CString CDocBar::GetFilterField(MarketFilterType mft) {

	switch (mft) {

		case mftDate:

			if (m_DateCombo->CurSel != -1 && VarLong(m_DateCombo->GetValue(m_DateCombo->CurSel,0)) != -1) {
				return GetFieldFromEnum(m_DateFilter);
			}
			else {
				return "";
			}
		break;

		case mftLocation:
			if (m_locCombo->CurSel != -1) {
				return GetFieldFromEnum(m_LocFilter);
			}
			else {
				return "";
			}
		break;

		case mftProvider:
			if (m_docCombo->CurSel != -1) {
				return GetFieldFromEnum(m_ProvFilter);
			}
			else {
				return "";
			}
		break;


		default:
			return "";
		break;
	}
}


long CDocBar::GetCategory() {

	long nCategory;
	if(m_categoryCombo->CurSel == -1)
		nCategory = -1;
	else {
		nCategory = VarLong(m_categoryCombo->GetValue(m_categoryCombo->CurSel, 0));
	}

	return nCategory;

}

long CDocBar::GetResp() {

	long nResp;

	if(m_RespCombo->CurSel == -1)
		nResp = -1;
	else {
		nResp = VarLong(m_RespCombo->GetValue(m_RespCombo->CurSel, 0));
	}

	return nResp;

}


CString CDocBar::GetDisplayNameFromEnum(MarketFilter mf)
{
	switch(mf) {
		case mfFirstContactDate:
			return "First Contact Date";
		break;
		case mfChargeDate:
			return "Charge Date";
		break;
		case mfPaymentDate:
			return "Payment Date";
		break;
		case mfApptInputDate:
			return "Appt. Input Date";
		break;
		case mfApptDate:
			return "Appointment Date";
		break;
		case mfEffectivenessDate:
			return "Effectiveness Date";
		break;

		case mfConsultDate:
			return "Consult Date";
		break;

		case mfConsultInputDate:
			return "Consult Input Date";
		break;

		case mfCostDatePaid:
			return "Date Paid";
		break;

		case mfPatientLocation:
			return "Patient Location";
		break;

		case mfTransLocation:
			return "Transaction Location";
		break;

		case mfApptLocation:
			return "Appointment Location";
		break;

		case mfCostLocation:
			return "Cost Location";
		break;

		case mfPatCostLocation:
			return "Patient\\Cost Locations";
		break;

		case mfTransCostLocation:
			return "Transaction\\Cost Locations";
		break;

		case mfPatNoCostLocation:
			return "Patient\\<No Cost> Locations";
		break;

		case mfTransNoCostLocation:
			return "Charge\\<No Cost> Locations";
		break;

		case mfPatApptLocation:
			return "Patient and Appointment Locations";
		break;

		case mfNoPatApptLocation:
			return "Only Appointment Location";
		break;

		case mfPatNoApptLocation:
			return "Only Patient Location";
		break;

		case mfChargeLocation:
			return "Charge Location";
		break;

		case mfPayLocation:
			return "Payment Location";
		break;

		case mfGraphDependant:
			return "Graph Location";
		break;

		case mfPatientProvider:
			return "Patient Provider";
		break;

		case mfTransProvider:
			return "Transaction Provider";
		break;

		case mfApptProvider:
			return "Appointment Resource";
		break;

/*		case mfPatApptProvider:
			return "Patient Provider and Appt. Resource";
		break;*/

		case mfPatNoApptProvider:
			return "Patient Provider";
		break;

		case mfNoPatApptProvider:
			return "Appointment Resource";
		break;

		case mfChargeProvider:
			return "Charge Provider";
		break;

		case mfPayProvider:
			return "Payment Provider";
		break;

		case mfDependantProvider:
			return "Provider";
		break;

		//DRT 5/8/2008 - PLID 29966 - Added referral date
		case mfReferralDate:
			return "Referral Date";
		break;

	}
	ASSERT(FALSE);
	return "";
}




void CDocBar::OnSelChosenPatFilterList(long nRow) 
{
	try {
		BOOL bNeedRefresh = FALSE;

		if(nRow == -1) {
			//select "No Patient Filters"
			m_PatFilterCombo->SetSelByColumn(0,(long)-1);
			nRow = m_PatFilterCombo->CurSel;
			m_nPatFilterID = -1;
			bNeedRefresh = TRUE;
		}

		long nPatFilterID = VarLong(m_PatFilterCombo->GetValue(nRow,0),-1);

		if(nPatFilterID == -1) {
			//no patient filter
			m_nPatFilterID = -1;
			bNeedRefresh = TRUE;
		}
		else if(nPatFilterID == -2) {
			//new filter selected
			if (AddNewFilter()) {
				bNeedRefresh = TRUE;
			}
			else {
				m_PatFilterCombo->SetSelByColumn(0, m_nPatFilterID);
			}
		}
		else {
			//they chose a specific filter
			m_nPatFilterID = nPatFilterID;
			bNeedRefresh = TRUE;
		}

		if (bNeedRefresh)
		{
			if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
				CWaitCursor cWait;
//				GetMainFrame()->GetActiveView()->UpdateView();
				// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
				// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
				GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_FILTER, -1);
			}
		}

	}NxCatchAll("Error selecting patient Filter.");
}

void CDocBar::OnSelChosenFilteredProviderList(long nRow) 
{
	try {
		BOOL bNeedRefresh = FALSE;

		if(nRow == -1) {
			//select "all providers"
			m_docCombo->SetSelByColumn(0,(long)-1);
			nRow = m_docCombo->CurSel;
			bNeedRefresh = TRUE;
		}

		long nProviderID = VarLong(m_docCombo->GetValue(nRow,0),-1);

		if(nProviderID == -1) {
			//all providers
			m_dwProvIDs.RemoveAll();
			bNeedRefresh = TRUE;
		}
		else if(nProviderID == -2) {
			//multiple providers
			if (OnMultiSelectProviders())
				bNeedRefresh = TRUE;
		}
		// (j.gruber 2007-03-21 11:09) - PLID 25293 - let them pick what to show
		else if (nProviderID == -3) {
			//they want to show the opposite of what they are now showing
			//see if they are showing resources or providers
			CString strFrom = (LPCTSTR)m_docCombo->FromClause;
			CString strWhere = (LPCTSTR)m_docCombo->WhereClause;
			BOOL bShowingInactive;
			if (strWhere.IsEmpty() ) {
				bShowingInactive = TRUE;
			}
			else {
				bShowingInactive = FALSE;
			}
			if (strFrom.CompareNoCase("ResourceT") == 0) {
				strWhere = bShowingInactive ? " Inactive = 0 " : "";
			}
			else {
				strWhere = bShowingInactive ? " Archived = 0 " : "";
			}
			
			m_docCombo->WhereClause = _bstr_t(strWhere);
			bNeedRefresh = TRUE;
			m_docCombo->Requery();
			
			
		}
		else {
			//they chose a specific provider
			m_dwProvIDs.RemoveAll();
			m_dwProvIDs.Add((DWORD)nProviderID);
			bNeedRefresh = TRUE;
		}

		if (bNeedRefresh)
		{

			if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
				//PLID 16769 - show a wait cursor
				m_docCombo->DropDownState = FALSE;
				CWaitCursor cWait;
//				GetMainFrame()->GetActiveView()->UpdateView();
				// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
				// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
				GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_PROVIDER, -1);
			}
		}

	}NxCatchAll("Error selecting provider.");
}

void CDocBar::OnSelChosenFilteredLocationList(long nRow) 
{
	try {
		BOOL bNeedRefresh = FALSE;

		if(nRow == -1) {
			//select "all locations"
			m_locCombo->SetSelByColumn(0,(long)-1);
			nRow = m_locCombo->CurSel;
			bNeedRefresh = TRUE;
		}

		long nLocationID = VarLong(m_locCombo->GetValue(nRow,0),-1);

		if(nLocationID == -1) {
			//all locations
			m_dwLocIDs.RemoveAll();
			bNeedRefresh = TRUE;
		}
		else if(nLocationID == -2) {
			//multiple locations
			if (OnMultiSelectLocations())
				bNeedRefresh = TRUE;
		}
		else {
			//they chose a specific location
			m_dwLocIDs.RemoveAll();
			m_dwLocIDs.Add((DWORD)nLocationID);
			bNeedRefresh = TRUE;
		}

		if (bNeedRefresh)
		{
			if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
				CWaitCursor cWait;
//				GetMainFrame()->GetActiveView()->UpdateView();
				// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
				// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
				GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_LOCATION, -1);
			}
		}

	}NxCatchAll("Error selecting location.");
}


BOOL CDocBar::OnMultiSelectProviders() 
{
	try {
		//are we using resources?
		CString strFrom, strID, strName, strDesc, strOrder;
		strFrom = (LPCTSTR) m_docCombo->FromClause;
		strFrom.TrimRight();
		strFrom.TrimLeft();
		
		// (j.gruber 2007-03-21 11:11) - PLID 25293 - see if they are showing inactives
		CString strWhere = (LPCTSTR)m_docCombo->WhereClause;
		BOOL bShowingInactive;
		if (strWhere.IsEmpty() ) {
			bShowingInactive = TRUE;
		}
		else {
			bShowingInactive = FALSE;
		}
		
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "");

		if (strFrom.CompareNoCase("ResourceT") == 0) {
			strID = "ResourceT.ID";
			strName = "ResourceT.Item";
			strDesc = "Select one or more resources to filter on:";
			strOrder = "Item";
			strWhere = bShowingInactive ? "" : " Inactive = 0 ";
			dlg.SetSizingConfigRT("ResourceT");
		}
		else {
			strID = "PersonT.ID";
			strName = "Last + ', ' + First + ' ' + Middle";
			strDesc = "Select one or more providers to filter on:";
			strOrder = "Last, First, Middle";
			strWhere = bShowingInactive ? "" : " Archived = 0 ";
			dlg.SetSizingConfigRT("PersonT");
		}
			
		dlg.PreSelect(m_dwProvIDs);
		if(IDOK == dlg.Open(strFrom, strWhere, strID, strName, strDesc, 1)) {
			dlg.FillArrayWithIDs(m_dwProvIDs);
			if(m_dwProvIDs.GetSize() > 1) {
				ShowDlgItem(this, IDW_PROVIDER_COMBO, SW_HIDE);
				m_strMultiProvString = dlg.GetMultiSelectString();
				m_nxlProviderLabel.SetText(m_strMultiProvString);
				m_nxlProviderLabel.SetType(dtsHyperlink);
				ShowDlgItem(this, IDC_MULTI_PROV_LIST_TOOLBAR, SW_SHOW);
				InvalidateDlgItem(IDW_PROVIDER_COMBO);
			}
			else if(m_dwProvIDs.GetSize() == 1) {
				//They selected exactly one.
				// (j.gruber 2007-03-21 10:45) - PLID 25270 - this needs to reset the member variable storing multiple selections
				m_strMultiProvString = "";
				ShowDlgItem(this, IDC_MULTI_PROV_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_PROVIDER_COMBO, SW_SHOW);
				m_docCombo->SetSelByColumn(0, (long)m_dwProvIDs.GetAt(0));
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
			return TRUE;
		}
		else {
			//Check if they have "multiple" selected
			if(m_dwProvIDs.GetSize() > 1) {
				ShowDlgItem(this,IDW_PROVIDER_COMBO, SW_HIDE);
				m_nxlProviderLabel.SetText(m_strMultiProvString);
				m_nxlProviderLabel.SetType(dtsHyperlink);
				
				ShowDlgItem(this, IDC_MULTI_PROV_LIST_TOOLBAR, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_PROV_LIST_TOOLBAR);
			}
			else if(m_dwProvIDs.GetSize() == 1) {
				//They selected exactly one
				m_strMultiProvString = "";
				ShowDlgItem(this, IDC_MULTI_PROV_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_PROVIDER_COMBO, SW_SHOW);
				m_docCombo->SetSelByColumn(0, (long)m_dwProvIDs.GetAt(0));
			}
			else if(m_dwProvIDs.GetSize() == 0) {
				//They selected "<All Providers>"
				ShowDlgItem(this, IDC_MULTI_PROV_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_PROVIDER_COMBO, SW_SHOW);
				m_strMultiProvString = "";
				m_docCombo->SetSelByColumn(0, (long)-1);
			}
		}
	}NxCatchAll("Error in OnMultiSelectProviders");
	return FALSE;
}

BOOL CDocBar::OnMultiSelectLocations() 
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "LocationsT");
		dlg.PreSelect(m_dwLocIDs);
		// (c.haag 2004-05-28 13:50) - PLID 12257 - Filter on managed, active locations
		// (j.jones 2012-08-27 16:25) - PLID 52309 - We now allow filtering on unmanaged locations
		
		/* If you change this where clause, be sure to change the m_locCombo->WhereClause as well!*/
		if(IDOK == dlg.Open("LocationsT", "Active = 1 AND TypeID = 1", "LocationsT.ID", "Name", "Select one or more locations to filter on:", 1)) {
			dlg.FillArrayWithIDs(m_dwLocIDs);
		
			if(m_dwLocIDs.GetSize() > 1) {
				ShowDlgItem(this, IDW_LOCATION_COMBO, SW_HIDE);
				m_strMultiLocString = dlg.GetMultiSelectString();
				m_nxlLocationLabel.SetText(m_strMultiLocString);
				m_nxlLocationLabel.SetType(dtsHyperlink);
				ShowDlgItem(this, IDC_MULTI_LOC_LIST_TOOLBAR, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR);
			}
			else if(m_dwLocIDs.GetSize() == 1) {
				//They selected exactly one.
				m_strMultiLocString = "";
				ShowDlgItem(this, IDC_MULTI_LOC_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_LOCATION_COMBO, SW_SHOW);
				m_locCombo->SetSelByColumn(0, (long)m_dwLocIDs.GetAt(0));
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
			return TRUE;
		}
		else {
			//Check if they have "multiple" selected
			if(m_dwLocIDs.GetSize() > 1) {
				ShowDlgItem(this, IDW_LOCATION_COMBO, SW_HIDE);
				m_nxlLocationLabel.SetText(m_strMultiLocString); // (c.haag 2008-06-04 16:33) - PLID 30211 - Get the multi-location string, not multi-provider string
				m_nxlLocationLabel.SetType(dtsHyperlink);
				ShowDlgItem(this, IDC_MULTI_LOC_LIST_TOOLBAR, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR);
			}
			else if(m_dwLocIDs.GetSize() == 1) {
				//They selected exactly one
				ShowDlgItem(this, IDC_MULTI_LOC_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_LOCATION_COMBO, SW_SHOW);
				m_locCombo->SetSelByColumn(0, (long)m_dwLocIDs.GetAt(0));
			}
			else if(m_dwLocIDs.GetSize() == 0) {
				//They selected "<All Locations>"
				ShowDlgItem(this, IDC_MULTI_LOC_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_LOCATION_COMBO, SW_SHOW);
				m_locCombo->SetSelByColumn(0, (long)-1);
			}
		}
	}NxCatchAll("Error in OnMultiSelectLocations");
	return FALSE;
}

/*BOOL CDocBar::OnMultiSelectPatCoords() 
{
	try {
		CMultiSelectDlg dlg;
		dlg.PreSelect(m_dwPatCoordIDs);
		if(IDOK == dlg.Open("PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID", "PatientCoordinator = 1", "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select one or more patient coordinators to filter on:", 1)) {
			dlg.FillArrayWithIDs(m_dwPatCoordIDs);
		
			if(m_dwPatCoordIDs.GetSize() > 1) {
				ShowDlgItem(this, IDW_PAT_COORD_COMBO, SW_HIDE);
				m_strMultiCoordString = dlg.GetMultiSelectString();
				m_nxlPatCoordLabel.SetText(m_strMultiCoordString);
				m_nxlPatCoordLabel.SetType(dtsHyperlink);
				ShowDlgItem(this, IDC_MULTI_COORD_LIST_TOOLBAR, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_COORD_LIST_TOOLBAR);
			}
			else if(m_dwPatCoordIDs.GetSize() == 1) {
				//They selected exactly one.
				m_strMultiCoordString = "";
				ShowDlgItem(this, IDC_MULTI_COORD_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_PAT_COORD_COMBO, SW_SHOW);
				m_PatCoordCombo->SetSelByColumn(0, (long)m_dwPatCoordIDs.GetAt(0));
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
			return TRUE;
		}
		else {
			//Check if they have "multiple" selected
			if(m_dwPatCoordIDs.GetSize() > 1) {
				ShowDlgItem(this, IDW_PAT_COORD_COMBO, SW_HIDE);
				m_nxlPatCoordLabel.SetText(m_strMultiCoordString);
				m_nxlPatCoordLabel.SetType(dtsHyperlink);
				ShowDlgItem(this, IDC_MULTI_COORD_LIST_TOOLBAR, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_COORD_LIST_TOOLBAR);
			}
			else if(m_dwPatCoordIDs.GetSize() == 1) {
				//They selected exactly one
				ShowDlgItem(this, IDC_MULTI_COORD_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_PAT_COORD_COMBO, SW_SHOW);
				m_PatCoordCombo->SetSelByColumn(0, (long)m_dwPatCoordIDs.GetAt(0));
			}
			else if(m_dwPatCoordIDs.GetSize() == 0) {
				//They selected "<All Patient Coordinators>"
				ShowDlgItem(this, IDC_MULTI_COORD_LIST_TOOLBAR, SW_HIDE);
				ShowDlgItem(this, IDW_PAT_COORD_COMBO, SW_SHOW);
				m_PatCoordCombo->SetSelByColumn(0, (long)-1);
			}
		}
	}NxCatchAll("Error in OnMultiSelectPatCoords");
	return FALSE;
}*/

void CDocBar::EnsureFilter(MarketFilterType mft)
{
	if(m_Type == -1) {
		switch (mft) {
			case mftDate:
				if(m_dwAllowedDateFilters.GetSize() > 0) {
					m_DateFilter = (MarketFilter)m_dwAllowedDateFilters.GetAt(0);
					m_Date.SetText(GetDisplayNameFromEnum(m_DateFilter));
					m_Date.Invalidate();
				}
			break;

			case mftLocation:
				if(m_dwAllowedLocationFilters.GetSize() > 0) {
					m_LocFilter = (MarketFilter)m_dwAllowedLocationFilters.GetAt(0);
					m_Loc.SetText(GetDisplayNameFromEnum(m_LocFilter));
					m_Loc.Invalidate();
				}
			break;
				
			case mftProvider:
				if(m_dwAllowedProviderFilters.GetSize() > 0) {
					m_ProvFilter = (MarketFilter)m_dwAllowedProviderFilters.GetAt(0);
					m_Prov.SetText(GetDisplayNameFromEnum(m_ProvFilter));
					m_Prov.Invalidate();
				}
			break;
			

			default:
				ASSERT(FALSE);
			break;
		}

		
	}
	else {
		switch (mft) {

			case mftDate:
				if(!DoesGraphSupportFilter((MarketGraphType)m_Type, m_DateFilter)) {
					//Get the first filter that is supported, use it.
					if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfChargeDate)) {
						m_DateFilter = mfChargeDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfPaymentDate)) {
						m_DateFilter = mfPaymentDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfApptInputDate)) {
						m_DateFilter = mfApptInputDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfApptDate)) {
						m_DateFilter = mfApptDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfEffectivenessDate)) {
						m_DateFilter = mfEffectivenessDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfCostDatePaid)) {
						m_DateFilter = mfCostDatePaid;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfConsultDate)) {
						m_DateFilter = mfConsultDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfConsultInputDate)) {
						m_DateFilter = mfConsultInputDate;
					}
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfFirstContactDate)) {
						m_DateFilter = mfFirstContactDate;
					}
					//DRT 5/8/2008 - PLID 29966 - Added referral date
					else if(DoesGraphSupportFilter((MarketGraphType)m_Type, mfReferralDate)) {
						m_DateFilter = mfReferralDate;
					}
					else {
						ASSERT(FALSE);
					}
				}

				m_Date.SetText(GetDisplayNameFromEnum(m_DateFilter));
				m_Date.Invalidate();

			break;

			case mftLocation:
				if (! DoesGraphSupportFilter((MarketGraphType)m_Type, m_LocFilter)) {
					if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatientLocation)) {
						m_LocFilter = mfPatientLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfTransLocation)) {
						m_LocFilter = mfTransLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfApptLocation)) {
						m_LocFilter = mfApptLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfCostLocation)) {
						m_LocFilter = mfCostLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatCostLocation)) {
						m_LocFilter = mfPatCostLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfTransCostLocation)) {
						m_LocFilter = mfTransCostLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatNoCostLocation)) {
						m_LocFilter = mfPatNoCostLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfTransNoCostLocation)) {
						m_LocFilter = mfTransNoCostLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatApptLocation)) {
						m_LocFilter = mfPatApptLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfNoPatApptLocation)) {
						m_LocFilter = mfNoPatApptLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatNoApptLocation)) {
						m_LocFilter = mfPatNoApptLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfChargeLocation)) {
						m_LocFilter = mfChargeLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPayLocation)) {
						m_LocFilter = mfPayLocation;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfGraphDependant)) {
						m_LocFilter = mfGraphDependant;
					}

					else {
						ASSERT(FALSE);
					}
				}

				m_Loc.SetText(GetDisplayNameFromEnum(m_LocFilter));
				m_Loc.Invalidate();
			break;		

			case mftProvider: {

				BOOL bNeedRequery;
				MarketFilter mfTemp;
				if (! DoesGraphSupportFilter((MarketGraphType)m_Type, m_ProvFilter)) {
					if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatientProvider)) {
						mfTemp = mfPatientProvider;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfTransProvider)) {
						mfTemp = mfTransProvider;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfApptProvider)) {
						mfTemp = mfApptProvider;
					}
/*					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatApptProvider)) {
						mfTemp = mfPatApptProvider;
					}*/
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPatNoApptProvider)) {
						mfTemp = mfPatNoApptProvider;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfNoPatApptProvider)) {
						mfTemp = mfNoPatApptProvider;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfChargeProvider)) {
						mfTemp = mfChargeProvider;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfPayProvider)) {
						mfTemp = mfPayProvider;
					}
					else if (DoesGraphSupportFilter((MarketGraphType)m_Type, mfDependantProvider)) {
						mfTemp = mfDependantProvider;
					}
					else {
						ASSERT(FALSE);
					}
				}
				else {
					mfTemp = m_ProvFilter;
				}

				bNeedRequery = CheckProvRequery(m_ProvFilter, mfTemp);
				m_ProvFilter = mfTemp;

				if (bNeedRequery) {
					RequeryDoc(FALSE);
				}
								

				m_Prov.SetText(GetDisplayNameFromEnum(m_ProvFilter));
				m_Prov.Invalidate();
							  }
			break;

			default:
			break;
		}
	}
	
}


void CDocBar::OnRequeryFinishedDocCombo(short nFlags)
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		CString strFrom = (LPCTSTR)m_docCombo->FromClause;
		strFrom.TrimRight();
		strFrom.TrimLeft();

		// (j.gruber 2007-03-21 11:02) - PLID 25293 - add a row to show inactive providers
		CString strWhere = (LPCTSTR)m_docCombo->WhereClause;
		BOOL bShowingInactive;
		if (strWhere.IsEmpty() ) {
			bShowingInactive = TRUE;
		}
		else {
			bShowingInactive = FALSE;
		}

		CString strAll, strMultiple, strInactive;
		if (strFrom.CompareNoCase("ResourceT") == 0) {
			strAll = "<All Resources>";
			strMultiple = "<Multiple Resources>";
			strInactive = bShowingInactive ? "<Show Only Active Resources>" : "<Show Inactive Resources Also>";
		}
		else {
			strAll = "<All Providers>";
			strMultiple = "<Multiple Providers>";
			strInactive = "<Show Inactive Providers>";
			strInactive = bShowingInactive ? "<Show Only Active Providers>" : "<Show Inactive Providers Also>";
		}		

		IRowSettingsPtr pRow;
		pRow = m_docCombo->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t(strAll));
		m_docCombo->InsertRow(pRow, 0);

		pRow = m_docCombo->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _variant_t(strMultiple));
		m_docCombo->InsertRow(pRow, 1);

		pRow = m_docCombo->GetRow(-1);
		pRow->PutValue(0, (long)-3);
		pRow->PutValue(1, _variant_t(strInactive));
		m_docCombo->AddRow(pRow);

		m_docCombo->CurSel = 0;
	}NxCatchAll(__FUNCTION__);
}

void CDocBar::OnRequeryFinishedLocationCombo(short nFlags) 
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try{
		IRowSettingsPtr pRow;
		pRow = m_locCombo->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<All Locations>"));
		m_locCombo->InsertRow(pRow, 0);

		pRow = m_locCombo->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _variant_t("<Multiple Locations>"));
		m_locCombo->InsertRow(pRow, 1);

		m_locCombo->CurSel = 0;
	}NxCatchAll(__FUNCTION__);
}

void CDocBar::OnRequeryFinishedCategoryCombo(short nFlags) {
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		IRowSettingsPtr pRow;
		pRow = m_categoryCombo->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<All Categories>"));
		m_categoryCombo->InsertRow(pRow, 0);

		m_categoryCombo->CurSel = 0;
	}NxCatchAll(__FUNCTION__);
}

void CDocBar::OnRequeryFinishedPatFilterCombo(short nFlags) {
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		IRowSettingsPtr pRow;
		pRow = m_PatFilterCombo->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<No Filter Selected>"));
		m_PatFilterCombo->InsertRow(pRow, 0);

		//j.camacho 6/24/2014  PLID 58876 - LW licensing should be checked to prevent users from creating new filters.
		if(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent))
		{
			pRow = m_PatFilterCombo->GetRow(-1);
			pRow->PutValue(0, (long)-2);
			pRow->PutValue(1, _variant_t("<New Filter>"));
			m_PatFilterCombo->InsertRow(pRow, 1);
		}
				
		if (m_nPatFilterID == -1) {
			m_PatFilterCombo->CurSel = 0;
		}	
	}NxCatchAll(__FUNCTION__);
}


void CDocBar::ChangeSelectionFilteredDateOptionList(long nRow, BOOL bIsEvent) {

	long nValue;
	if(nRow == -1) nValue = -2;
	else nValue  = VarLong(m_DateCombo->GetValue(nRow, 0));

	long nYear, nMonth, nDay, nDayOfWeek;

	nYear = COleDateTime::GetCurrentTime().GetYear();
	nMonth = COleDateTime::GetCurrentTime().GetMonth();
	nDay = COleDateTime::GetCurrentTime().GetDay();
	nDayOfWeek = COleDateTime::GetCurrentTime().GetDayOfWeek();

	COleDateTimeSpan dtSpan;	
	COleDateTime dtFrom, dtTo, dtTemp;

	switch(nValue) {
		case emdvAll:
			//All Dates
			break;		
		case emdvOneYear: 
			//Year
			dtFrom.SetDate(nYear - 1, nMonth, nDay);
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));			
		break;

		case emdvCustom:
			//Custom
		break;

		case emdvToday:
			//Today
			m_FromDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
			m_ToDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		break;

		case emdvThisWeek: 
			//This week:
			dtFrom.SetDate(nYear,nMonth,nDay);			
			//if this is Sunday, start from today
			if (nDayOfWeek != 0) {
				dtSpan.SetDateTimeSpan(nDayOfWeek - 1,0,0,0);
				dtFrom -= dtSpan;
			}	

			dtTo.SetDate(nYear,nMonth,nDay);
			dtSpan.SetDateTimeSpan(6,0,0,0);
			dtTo = dtFrom + dtSpan;
			
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
		break;

		case emdvThisMonth: 
			//This Month
			dtFrom.SetDate(nYear, nMonth, 1);
			if (nMonth == 12) {
				dtTo.SetDate(nYear, 12, 31);
			}
			else {
				dtTemp.SetDate(nYear, nMonth + 1, 1);
				COleDateTimeSpan dtSpan(1,0,0,0);
				dtTo = dtTemp - dtSpan;
			}
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));			
				
		break;

		case emdvThisQuarter:
			//This Quarter
			//a quarter is 1/1 - 3/31, 4/1 - 6/30, 7/1 - 9/30, 10/1 - 12/31
			if(nMonth >= 1 && nMonth <= 3) {
				//quarter 1
				dtFrom.SetDate(nYear,1,1);
				dtTo.SetDate(nYear,3,31);
			}
			else if(nMonth >= 4 && nMonth <= 6) {
				//quarter 2
				dtFrom.SetDate(nYear,4,1);
				dtTo.SetDate(nYear,6,30);
			}
			else if(nMonth >= 7 && nMonth <= 9) {
				//quarter 3
				dtFrom.SetDate(nYear,7,1);
				dtTo.SetDate(nYear,9,30);
			}
			else if(nMonth >= 10 && nMonth <= 12) {
				//quarter 4
				dtFrom.SetDate(nYear,10,1);
				dtTo.SetDate(nYear,12,31);
			}
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
			break;

		case emdvThisYear: 
			//This year
			dtFrom.SetDate(nYear, 1, 1);
			m_FromDate.SetValue(_variant_t(dtFrom));
			dtTo.SetDate(nYear, 12,31);
			m_ToDate.SetValue(_variant_t(dtTo));				
			break;

		case emdvThisMonthToDate: 
			//This Month to date
			dtFrom.SetDate(nYear, nMonth, 1);
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
			break;

		case emdvThisQuarterToDate:	 
			//This quarter to date
			//a quarter is 1/1 - 3/31, 4/1 - 6/30, 7/1 - 9/30, 10/1 - 12/31
			if(nMonth >= 1 && nMonth <= 3) {
				//quarter 1
				dtFrom.SetDate(nYear,1,1);
			}
			else if(nMonth >= 4 && nMonth <= 6) {
				//quarter 2
				dtFrom.SetDate(nYear,4,1);
			}
			else if(nMonth >= 7 && nMonth <= 9) {
				//quarter 3
				dtFrom.SetDate(nYear,7,1);
			}
			else if(nMonth >= 10 && nMonth <= 12) {
				//quarter 4
				dtFrom.SetDate(nYear,10,1);
			}
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
			break;

		case emdvThisYearToDate:
			//this year to date
			dtFrom.SetDate(nYear, 1, 1);
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
			break;

		// (j.gruber 2010-09-08 10:43) - PLID 37425
		case emdvYesterday:
			//Yesterday
			dtFrom.SetDate(nYear,nMonth,nDay);		
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dtFrom -= dtSpan;
			dtTo = dtFrom;				
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
		break;

		case emdvLastWeek:
			//last week
			dtFrom.SetDate(nYear,nMonth,nDay);		
			dtSpan.SetDateTimeSpan(nDayOfWeek,0,0,0);
			dtFrom -= dtSpan;
			dtTo = dtFrom;
			dtSpan.SetDateTimeSpan(6,0,0,0);
			dtFrom -= dtSpan;

			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
			break;

		case emdvLastMonth:
			//Last month
			dtFrom.SetDate(nYear, nMonth, 1);
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dtFrom -= dtSpan;
			dtTo = dtFrom;
			dtFrom.SetDate(dtFrom.GetYear(),dtFrom.GetMonth(),1);

			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
			break;

		case emdvLastQuarter:
			//last quarter
			//a quarter is 1/1 - 3/31, 4/1 - 6/30, 7/1 - 9/30, 10/1 - 12/31
			if(nMonth >= 1 && nMonth <= 3) {
				//Q1, so last quarter was Q4 of previous year
				dtFrom.SetDate(nYear-1,10,1);
				dtTo.SetDate(nYear-1,12,31);		
			}
			else if(nMonth >= 4 && nMonth <= 6) {
				//Q2, so last quarter was quarter 1
				dtFrom.SetDate(nYear,1,1);
				dtTo.SetDate(nYear,3,31);				
			}
			else if(nMonth >= 7 && nMonth <= 9) {
				//Q3, so last quarter was quarter 2
				dtFrom.SetDate(nYear,4,1);
				dtTo.SetDate(nYear,6,30);
			}
			else if(nMonth >= 10 && nMonth <= 12) {
				//Q4, so last quarter was quarter 3
				//(e.lally 2010-10-18) PLID 40970 - Fixed to be July-Sept, the real Q3
				dtFrom.SetDate(nYear,7,1);
				dtTo.SetDate(nYear,9,30);
			}
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
			break;

		case emdvLastYear: 
			//last year
			dtFrom.SetDate(nYear - 1, 1, 1);
			dtTo.SetDate(nYear - 1, 12, 31);
			m_FromDate.SetValue(_variant_t(dtFrom));
			m_ToDate.SetValue(_variant_t(dtTo));
		break;

		default :
			//Separator or -1.
			//Set it to custom, leave the dates how they are.
			m_DateCombo->SetSelByColumn(0, (long)2);
		break;
	}

	m_FromDate.EnableWindow(nValue != -1);
	m_ToDate.EnableWindow(nValue != -1);

	if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
		CWaitCursor cWait;
//		GetMainFrame()->GetActiveView()->UpdateView();
		// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
		// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
		GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_DATES, -1);
	}




}

void CDocBar::OnSelChosenFilteredDateOptionList(long nRow) {
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try{
		ChangeSelectionFilteredDateOptionList(nRow, TRUE);
	}NxCatchAll(__FUNCTION__);

}

void CDocBar::OnSelChosenFilteredCategoryList(long nRow)  {
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try{
		if(nRow == -1) 
			m_categoryCombo->CurSel = 0;

		if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
			CWaitCursor cWait;
	//		GetMainFrame()->GetActiveView()->UpdateView();
			// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
			// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
			GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_CATEGORY, -1);
		}
	}NxCatchAll(__FUNCTION__);

}

void CDocBar::OnSelChosenFilteredRespList(long nRow)  {
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try{
		if(nRow == -1)
			m_RespCombo->CurSel = 0;

		if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
			CWaitCursor CWait;
	//		GetMainFrame()->GetActiveView()->UpdateView();
			// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
			// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
			GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_RESP, -1);
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CDocBar::UseFilter(MarketFilterType mft)
{
	switch (mft) {
		
		case mftDate:
			return m_DateCombo->CurSel != -1 && m_DateCombo->GetValue(m_DateCombo->CurSel,0).lVal != -1;
		break;

		case mftLocation:
			return m_locCombo->CurSel != -1 && m_locCombo->GetValue(m_locCombo->CurSel, 0).lVal != -1;
		break;

		case mftProvider: 
			return m_docCombo->CurSel != -1 && m_docCombo->GetValue(m_docCombo->CurSel, 0).lVal != -1;
		break;

		default:
			return false;
		break;
	}
}


void CDocBar::SetAllowedFilters(CDWordArray &dwFilters, MarketFilterType mft)
{
	switch (mft) {
	case mftDate: {
			m_dwAllowedDateFilters.RemoveAll();
			for(int i = 0; i < dwFilters.GetSize(); i++) m_dwAllowedDateFilters.Add(dwFilters.GetAt(i));

			EnsureFilter(mftDate);
				  }
		break;
	case mftLocation: {
			m_dwAllowedLocationFilters.RemoveAll();
			for(int i = 0; i < dwFilters.GetSize(); i++) m_dwAllowedLocationFilters.Add(dwFilters.GetAt(i));

			EnsureFilter(mftLocation);
				  }
	case mftProvider: {
			m_dwAllowedProviderFilters.RemoveAll();
			for(int i = 0; i < dwFilters.GetSize(); i++) m_dwAllowedProviderFilters.Add(dwFilters.GetAt(i));

			EnsureFilter(mftProvider);
				  }
		default:
		break;
	}
}

void CDocBar::SetHiddenFilters(DWORD dwHidden)
{
	// (a.walling 2006-10-11 16:13) - PLID 22764 - reliable way to check for hidden filters
	m_dwHidden = dwHidden;


	//TES 5/21/2004: Note that we're using SW_SHOWNA not SW_SHOW.  That way the focus doesn't jump around.
	//
	// (a.walling 2008-08-27 10:38) - PLID 30099 - For some reason, if we use SW_SHOWNOACTIVATE (which 
	// _is_ different than SW_SHOWNA, even though the NA stands for NO ACTIVATE...) then the window may
	// appear later via a RedrawWindow call with RDW_ALLCHILDREN. Bizarre. I won't pretend to understand
	// it, but I do know that only a handful of places in Practice code (and code searching through the web)
	// use SW_SHOWNOACTIVATE, and if I change it to SW_SHOWNA, everything works fine. According to docs, 
	// SW_SHOWNA shows the window with the current size and pos, while SW_SHOWNOACTIVATE shows the window
	// with the 'most recent' size and pos.
	//
	// (z.manning 2008-12-11 17:19) - PLID 32384 - That's all well and good, but the thing is, we want
	// to activate these controls!  Otherwise they may not be visible to the user. I changed all
	// instances of SW_SHOWNA to SW_SHOW. Regardaring Tom's comment from 2004, even with
	// SW_SHOWNA, focus was jumping around every time this function was called. And even so, I see 
	// no significant benefit of maintaining focus on this toolbar.

	// (z.manning 2008-12-11 17:25) - PLID 32384 - Make sure we aren't showing both the location combo
	// and the provider label at the same time.
	if(m_strMultiLocString.IsEmpty()) {
		GetDlgItem(IDW_LOCATION_COMBO)->ShowWindow(dwHidden & DBF_LOCATION ? SW_HIDE : SW_SHOW);
		GetDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDW_LOCATION_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MULTI_LOC_LIST_TOOLBAR)->ShowWindow(dwHidden & DBF_LOCATION ? SW_HIDE : SW_SHOW);
	}
	GetDlgItem(IDC_LOCATION_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_LOCATION ? SW_HIDE : SW_SHOW);

	// (z.manning 2008-12-11 17:25) - PLID 32384 - Make sure we aren't showing both the provider combo
	// and the provider label at the same time.
	if(m_strMultiProvString.IsEmpty()) {
		GetDlgItem(IDW_PROVIDER_COMBO)->ShowWindow(dwHidden & DBF_PROVIDER ? SW_HIDE : SW_SHOW);
		GetDlgItem(IDC_MULTI_PROV_LIST_TOOLBAR)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDW_PROVIDER_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MULTI_PROV_LIST_TOOLBAR)->ShowWindow(dwHidden & DBF_PROVIDER ? SW_HIDE : SW_SHOW);
	}
	GetDlgItem(IDC_PROVIDER_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_PROVIDER ? SW_HIDE : SW_SHOW);
	

	GetDlgItem(IDC_PAT_FILTER_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_FILTER ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDW_PAT_FILTER_COMBO)->ShowWindow(dwHidden & DBF_FILTER ? SW_HIDE : SW_SHOW);
	//j.camacho 6/24/2014  PLID 58876 - LW licensing should be checked to prevent users from creating new filters. use silent to test for toolbar changes
	GetDlgItem(IDC_PAT_FILTER_BUTTON_TOOLBAR)->ShowWindow(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent) || (dwHidden & DBF_FILTER) ? SW_HIDE : SW_SHOW);

	GetDlgItem(IDC_DATE_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_DATES ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDW_DATE_OPTION_COMBO)->ShowWindow(dwHidden & DBF_DATES ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_FROM_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_DATES ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DOCFROM)->ShowWindow(dwHidden & DBF_DATES ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_TO_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_DATES ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DOCTO)->ShowWindow(dwHidden & DBF_DATES ? SW_HIDE : SW_SHOW);

	GetDlgItem(IDC_CATEGORY_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_CATEGORY ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDW_CATEGORY_COMBO)->ShowWindow(dwHidden & DBF_CATEGORY ? SW_HIDE : SW_SHOW);

/*	if (dwHidden & DBF_CATEGORY) {
		CRect rect, rect2;
		GetDlgItem(IDC_DOCTO)->GetWindowRect(&rect);
		rect.left = rect.right + 2;
		GetItemRect(6, &rect2);
		rect.right = rect2.right;
		m_blank3.Create("", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, rect, this, IDC_CATEGORY_LABEL_TOOLBAR);
	}*/

	GetDlgItem(IDC_RESP_LABEL_TOOLBAR)->ShowWindow(dwHidden & DBF_RESP ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDW_RESP_COMBO)->ShowWindow(dwHidden & DBF_RESP ? SW_HIDE : SW_SHOW);

	/*if (dwHidden & DBF_RESP) {
		CRect rect, rect2;
		GetItemRect(6, &rect);
		rect2 = rect;
		rect.left += rect.right + 2;
		rect.right = rect.left + rect2.Width();
		m_blank4.Create("", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, rect, this, IDC_CATEGORY_LABEL_TOOLBAR);
	}*/
}
//j.camacho 7/8/2014 - plid 58876
void CDocBar::OnUpdatePatFilterButton(CCmdUI* pCmdUI)
{
	static BOOL hasLetterLicense = g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent);
	pCmdUI->Enable(hasLetterLicense ? TRUE : FALSE);
}

int CDocBar::GetType()
{
	return m_Type;
}

BOOL CDocBar::IsFilteringAllDates()
{
	return (m_DateCombo->GetCurSel() == 0);
}

void CDocBar::SetFromDate(COleDateTime dt)
{
	m_FromDate.SetValue(_variant_t(dt));
}

void CDocBar::SetToDate(COleDateTime dt)
{
	m_ToDate.SetValue(_variant_t(dt));
}

void CDocBar::OnChangeFilteredToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		COleDateTime from, to;
		from = VarDateTime(m_FromDate.GetValue());
		to = VarDateTime(m_ToDate.GetValue());
		if (from > to)
			m_FromDate.SetValue(_variant_t(to));
		
		
		//(e.lally 2009-09-02) PLID 35446 - Need to check for the mainFrame and ActiveView first. That's what all the other posts do.
		if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
			CWaitCursor cWait;
			// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
			// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
			GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_TO, -1);
		}

		m_DateCombo->SetSelByColumn(0, (long)2);

		*pResult = 0;
	}NxCatchAll("CDocBar::OnChangeFilteredToDate - Error trying to change Marketing filter To Date")
}

void CDocBar::OnChangeFilteredFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try{
		COleDateTime from, to;
		from = VarDateTime(m_FromDate.GetValue());
		to = VarDateTime(m_ToDate.GetValue());
		if (from > to)
			m_ToDate.SetValue(_variant_t(from));
		
		//(e.lally 2009-09-02) PLID 35446 - Send this to the marketing view, since we might be viewing a report or something
		// to where there is no active view.
		if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
			CWaitCursor cWait;
			// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
			// (a.wilson 2011-11-18) PLID 38789 - altered the WPARAM for checking whether a filter has changed.
			GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, DBF_FROM, -1);
		}

		m_DateCombo->SetSelByColumn(0, (long)2);

		*pResult = 0;
	}NxCatchAll("CDocBar::OnChangeFilteredFromDate - Error trying to change Marketing filter From Date");
}

void CDocBar::OnSelChangingFilteredDateOptionList(long FAR* nNewSel) 
{
	//(e.lally 2009-09-02) PLID 35446 - Added try/catch
	try {
		long nValue;
		if(*nNewSel == -1) nValue = -2;
		else nValue  = VarLong(m_DateCombo->GetValue(*nNewSel, 0));

		if(nValue == -2) {
			*nNewSel = m_DateCombo->CurSel;
		}
	}NxCatchAll("CDocBar::OnSelChangingFilteredDateOptionList - Error changing date type selection");
}

//DRT 6/22/2006 - PLID 20944 - Added exception handline
LRESULT CDocBar::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		if (wParam == NetUtils::LocationsT) {
			try {
				long nLocationID = lParam;
				if (nLocationID == -1) {

					//just requery because they didn't send the value
					m_locCombo->Requery();
				}
				else {

					_RecordsetPtr rsLocation = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Active = 1 AND Managed = 1 AND ID = %li", nLocationID);

					if (rsLocation->eof) {

						//remove the row from the datalist if it is there
						m_locCombo->RemoveRow(m_locCombo->FindByColumn(0, nLocationID, 0, FALSE));
					}
					else {

						//update or insert the row
						long nRowLoc = m_locCombo->FindByColumn(0, nLocationID, 0, FALSE);
						
						//get the row, 
						IRowSettingsPtr pRow = m_locCombo->GetRow(nRowLoc);

						pRow->PutValue(0, nLocationID);
						pRow->PutValue(1, _variant_t(AdoFldString(rsLocation, "Name")));

						if (nRowLoc == -1) {
							//insert
							m_locCombo->AddRow(pRow);
						}
					}
				}
			} NxCatchAll("Error in CDocBar::OnTablechanged:LocationsT");
		}
		else if (wParam == NetUtils::Providers) {
			try {
				CString strFrom = (LPCTSTR)m_docCombo->FromClause;
				strFrom.TrimRight();
				strFrom.TrimLeft();
				//check to see that we are using providers
				if ( strFrom.CompareNoCase("ResourceT") == 0) {
					return 0;
				}

				long nProviderID = lParam;
				if (nProviderID == -1) {

					//just requery because they didn't send the value
					m_docCombo->Requery();
				}
				else {

					_RecordsetPtr rsProvider = CreateRecordset("SELECT ID, [Last] + ', ' + [First] + ' ' + Title As Name FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PErsonID = PersonT.ID WHERE ID = %li", nProviderID);

					if (rsProvider->eof) {

						//remove the row from the datalist if it is there
						m_docCombo->RemoveRow(m_docCombo->FindByColumn(0, nProviderID, 0, FALSE));
					}
					else {

						//update or insert the row
						long nRowDoc = m_docCombo->FindByColumn(0, nProviderID, 0, FALSE);
						
						//get the row, 
						IRowSettingsPtr pRow = m_docCombo->GetRow(nRowDoc);

						pRow->PutValue(0, nProviderID);
						pRow->PutValue(1, _variant_t(AdoFldString(rsProvider, "Name")));

						if (nRowDoc == -1) {
							//insert
							m_docCombo->AddRow(pRow);
						}
					}
				}
			} NxCatchAll("Error in CDocBar::OnTableChanged:Providers");
		}
		/*else if (wParam == NetUtils::Coordinators) {

			long nCoordID = lParam;
			if (nCoordID == -1) {

				//just requery because they didn't send the value
				m_PatCoordCombo->Requery();
			}
			else {

				_RecordsetPtr rsCoord = CreateRecordset("SELECT ID, PersonT.Last + ', ' + PersonT.First As Name FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID WHERE ID = %li AND UsersT.PatientCoordinator <> 0 ", nCoordID);

				if (rsCoord->eof) {

					//remove the row from the datalist if it is there
					m_PatCoordCombo->RemoveRow(m_PatCoordCombo->FindByColumn(0, nCoordID, 0, FALSE));
				}
				else {

					//update or insert the row
					long nRowCoord = m_PatCoordCombo->FindByColumn(0, nCoordID, 0, FALSE);
					
					//get the row, 
					IRowSettingsPtr pRow = m_PatCoordCombo->GetRow(nRowCoord);

					pRow->PutValue(0, nCoordID);
					pRow->PutValue(1, _variant_t(AdoFldString(rsCoord, "Name")));

					if (nRowCoord == -1) {
						//insert
						m_PatCoordCombo->AddRow(pRow);
					}
				}
			}
		}*/
		else if (wParam == NetUtils::CPTCategories) {
			try {
				long nCatID = lParam;
				if (nCatID == -1) {

					//just requery because they didn't send the value
					m_categoryCombo->Requery();
				}
				else {

					_RecordsetPtr rsCat= CreateRecordset("SELECT ID, Name FROM CategoriesT WHERE ID = %li  ", nCatID);

					if (rsCat->eof) {

						//remove the row from the datalist if it is there
						m_categoryCombo->RemoveRow(m_categoryCombo->FindByColumn(0, nCatID, 0, FALSE));
					}
					else {

						//update or insert the row
						long nRowCat = m_categoryCombo->FindByColumn(0, nCatID, 0, FALSE);
						
						//get the row, 
						IRowSettingsPtr pRow = m_categoryCombo->GetRow(nRowCat);

						pRow->PutValue(0, nCatID);
						pRow->PutValue(1, _variant_t(AdoFldString(rsCat, "Name")));

						if (nRowCat == -1) {
							//insert
							m_categoryCombo->AddRow(pRow);
						}
					}
				}
			} NxCatchAll("Error in CDocBar::OnTableChanged:CPTCategories");
		}
		else if (wParam == NetUtils::Resources) {
			try {
				CString strFrom = (LPCTSTR)m_docCombo->FromClause;
				strFrom.TrimLeft();
				strFrom.TrimRight();
				//check to see that we are using providers
				if (strFrom.CompareNoCase("ResourceT") == 0) {
					return 0;
				}

				long nResourceID = lParam;
				if (nResourceID == -1) {

					//just requery because they didn't send the value
					m_docCombo->Requery();
				}
				else {

					_RecordsetPtr rsResource = CreateRecordset("SELECT ID, Item As Name FROM ResourceT WHERE ID = %li", nResourceID);

					if (rsResource->eof) {

						//remove the row from the datalist if it is there
						m_docCombo->RemoveRow(m_docCombo->FindByColumn(0, nResourceID, 0, FALSE));
					}
					else {

						//update or insert the row
						long nRowDoc = m_docCombo->FindByColumn(0, nResourceID, 0, FALSE);
						
						//get the row, 
						IRowSettingsPtr pRow = m_docCombo->GetRow(nRowDoc);

						pRow->PutValue(0, nResourceID);
						pRow->PutValue(1, _variant_t(AdoFldString(rsResource, "Name")));

						if (nRowDoc == -1) {
							//insert
							m_docCombo->AddRow(pRow);
						}
					}
				}
			} NxCatchAll("Error in CDocBar::OnTableChanged:Resources");
		}
		else if (wParam == NetUtils::FiltersT) {
			try {
				long nFilterID = lParam;
				if (nFilterID == -1) {

					//just requery because they didn't send the value
					m_PatFilterCombo->Requery();
				}
				else {

					// (j.gruber 2011-06-17 14:55) - PLID 29557 - add create and modified dates
					_RecordsetPtr rsFilter = CreateRecordset("SELECT ID, Name, Filter, CreatedDate, ModifiedDate FROM FiltersT WHERE ID = %li AND Type = 1 ", nFilterID);

					if (rsFilter->eof) {

						//remove the row from the datalist if it is there
						m_PatFilterCombo->RemoveRow(m_PatFilterCombo->FindByColumn(0, nFilterID, 0, FALSE));
					}
					else {

						//update or insert the row
						long nRowFilter = m_PatFilterCombo->FindByColumn(0, nFilterID, 0, FALSE);
						
						//get the row, 
						IRowSettingsPtr pRow = m_PatFilterCombo->GetRow(nRowFilter);

						// (j.gruber 2011-06-17 15:06) - PLID 29557 - added created and modified dates
						pRow->PutValue(0, nFilterID);
						pRow->PutValue(1, _variant_t(AdoFldString(rsFilter, "Name")));
						pRow->PutValue(2, _variant_t(AdoFldString(rsFilter, "Filter")));
						_variant_t var = rsFilter->Fields->Item["CreatedDate"]->Value;
						CString strDate;
						if (var.vt == VT_DATE) {
							strDate = FormatDateTimeForInterface(AdoFldDateTime(rsFilter, "CreatedDate"), DTF_STRIP_SECONDS);
						}						
						pRow->PutValue(3, _variant_t(strDate));

						strDate = "";
						var = rsFilter->Fields->Item["ModifiedDate"]->Value;
						if (var.vt == VT_DATE) {
							strDate = FormatDateTimeForInterface(AdoFldDateTime(rsFilter, "ModifiedDate"), DTF_STRIP_SECONDS);
						}						
						pRow->PutValue(4, _variant_t(strDate));

						if (nRowFilter == -1) {
							//insert
							m_PatFilterCombo->AddRow(pRow);
						}
					}
				}
			} NxCatchAll("Error in CDocBar::OnTableChanged:FiltersT");
		}

	} NxCatchAll("Error in CDocBar::OnTableChanged");

	return 0;
}


CString CDocBar::GetLocationFilterString() {

	CString strTitle, strValue, strReturn;

	strTitle = m_Loc.GetText();

	long nCurSel = m_locCombo->GetCurSel();

	if (m_strMultiLocString.IsEmpty()) {

		// (c.haag 2007-02-20 13:20) - PLID 24749 - Also account for the selection being -1
		if (nCurSel <= 0) {
			//all Locations
			strValue = "All";

			strTitle.TrimRight("s");
			strReturn = strValue + " " + strTitle + "s";
		}
		else {
			strValue = VarString(m_locCombo->GetValue(nCurSel, 1));

			strReturn = strTitle + ": " + strValue;
		}
	}
	else {

		strReturn = strTitle + ": " + m_strMultiLocString;
	}

	return strReturn;




}

CString CDocBar::GetProviderFilterString() {

	CString strTitle, strValue, strReturn;

	strTitle = m_Prov.GetText();

	long nCurSel = m_docCombo->GetCurSel();

	if (m_strMultiProvString.IsEmpty()) {

		// (c.haag 2007-02-20 13:20) - PLID 24749 - Also account for the selection being -1
		if (nCurSel <= 0) {
			//all Providers
			strValue = "All";

			strReturn = strValue + " " + strTitle + "s";
		}
		else {
			strValue = VarString(m_docCombo->GetValue(nCurSel, 1));

			strReturn = strTitle + ": " + strValue;
		}
	}
	else {

		strReturn = strTitle + ": " + m_strMultiProvString;
	}

	return strReturn;


}


CString CDocBar::GetPatFilterFilterString() {

	CString strTitle, strValue, strReturn;

	strTitle = "Patient Filter";

	long nCurSel = m_PatFilterCombo->GetCurSel();

	// (c.haag 2007-02-20 13:20) - PLID 24749 - Also account for the selection being -1
	if (m_nPatFilterID == -1 || nCurSel == -1) {

		//all Providers
		strReturn = "No Patient Filter Selected";

	}
	else {
		strValue = VarString(m_PatFilterCombo->GetValue(nCurSel, 1));
		strReturn = strTitle + ": " + strValue;
	}
	
	return strReturn;



}

CString CDocBar::GetDateFilterString() {

	CString strTitle, strValue, strFrom, strTo, strReturn;

	strTitle = m_Date.GetText();

	long nCurSel = m_DateCombo->GetCurSel();

	// (c.haag 2007-02-20 13:20) - PLID 24749 - Also account for the selection being -1
	if (nCurSel <= 0) {
		//all Providers
		strValue = "All";
		strReturn = strValue + " " + strTitle + "s";
	}
	else {
		strFrom = FormatDateTimeForInterface(VarDateTime(m_FromDate.GetValue()), NULL, dtoDate);
		strTo = FormatDateTimeForInterface(VarDateTime(m_ToDate.GetValue()), NULL, dtoDate);

		strReturn = strTitle + ": "  + strFrom + " to " + strTo;
		
	}

	return strReturn;

}

// (a.walling 2006-10-11 09:57) - PLID 22764 - Return the responsibility / revenue source string.
CString CDocBar::GetRespFilterString()
{
	long nCurSel = m_RespCombo->GetCurSel();

	CString str = VarString(m_RespCombo->GetValue(nCurSel, 1), "");

	// (c.haag 2007-02-20 13:20) - PLID 24749 - Also account for the selection being -1
	if (nCurSel <= 0) {
		return "All Revenue Sources";
	}
	else
	{
		return "Revenue source: " + str;
	}
}

// (a.walling 2006-10-11 09:57) - PLID 22764 - Return the category string.
CString CDocBar::GetCategoryFilterString()
{
	long nCurSel = m_categoryCombo->GetCurSel();

	CString str = VarString(m_categoryCombo->GetValue(nCurSel, 1), "");

	// (c.haag 2007-02-20 13:20) - PLID 24749 - Also account for the selection being -1
	if (nCurSel <= 0) {
		return "All Categories";
	}
	else
	{
		return "Category: " + str;
	}
}

// (a.walling 2006-10-11 09:41) - PLID 22764 - Show the filter criteria in marketing reports
CString CDocBar::GetDescription()
{
	CString strDescription;

	///////////////

	if (!(m_dwHidden & DBF_LOCATION)) // && (m_locCombo->GetCurSel() != 0) )
	{
		strDescription += GetLocationFilterString() + "\r\n";
	}
	if (!(m_dwHidden & DBF_PROVIDER)) // && (m_docCombo->GetCurSel() != 0) )
	{
		strDescription += GetProviderFilterString() + "\r\n";
	}
	if (!(m_dwHidden & DBF_FILTER)) // && (m_PatFilterCombo->GetCurSel() != 0) )
	{
		strDescription += GetPatFilterFilterString() + "\r\n";
	}
	if (!(m_dwHidden & DBF_DATES)) // && (m_DateCombo->GetCurSel() != 0) )
	{
		strDescription += GetDateFilterString() + "\r\n";
	}
	if (!(m_dwHidden & DBF_CATEGORY)) // && (m_categoryCombo->GetCurSel() != 0) )
	{
		strDescription += GetCategoryFilterString() + "\r\n";
	}
	if (!(m_dwHidden & DBF_RESP)) // && (m_RespCombo->GetCurSel() != 0) )
	{
		strDescription += GetRespFilterString() + "\r\n";
	}

	strDescription.TrimRight("\r\n");

	return strDescription;
}


void CDocBar::OnEditFilter() {

	if (m_nPatFilterID == -1 || m_nPatFilterID == -2) {
		return;
	}
	//check their permissions and their license
	if (!CheckCurrentUserPermissions(bioLWFilter, sptWrite)) {
		return;
	}

	//Check license. they should not be able to edit filter if Letter writing is disabled.
	if (!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse)) {
			return;
	}

	//open up the filter dialog
	//we only support patient filters for this
	CFilterEditDlg dlg(NULL, 1, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
	long nResult;
	nResult = dlg.EditFilter(VarLong(m_PatFilterCombo->GetValue(m_PatFilterCombo->CurSel, 0)));

	if (nResult == 1) {
		//they clicked OK
		long nID = dlg.GetFilterId();
	
		m_PatFilterCombo->Requery();

		m_PatFilterCombo->SetSelByColumn(0, nID);
		m_nPatFilterID = nID;
		

		//refresh
		CClient::RefreshTable(NetUtils::Groups);

		//update the view
		if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
			CWaitCursor cWait;
//			GetMainFrame()->GetActiveView()->UpdateView();
			// (a.walling 2006-06-07 10:25) - PLID 20695, 20928 Post the market ready message with filter changes if any
			GetMainFrame()->GetActiveView()->PostMessage(NXM_MARKET_READY, -1, -1);
		}
	}
		

}

BOOL CDocBar::AddNewFilter() {
	if (!CheckCurrentUserPermissions(bioLWFilter, sptWrite))
		return FALSE;
		
	CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
	long nResult;
	nResult = dlg.DoModal();

	if (nResult == 1) {
		//they clicked OK
		long nID = dlg.GetFilterId();
	
		m_PatFilterCombo->Requery();

		m_PatFilterCombo->SetSelByColumn(0, nID);
		m_nPatFilterID = nID;
		
		//refresh
		CClient::RefreshTable(NetUtils::Groups);

		return TRUE;
	}

	return FALSE;	

}

BOOL CDocBar::OnEraseBkgnd(CDC* pDC) 
{
	// (a.walling 2008-04-21 14:04) - PLID 29724 - Erase the background!

	// (a.walling 2009-01-14 13:33) - PLID 32734 - Delegate background erasing to the parent
	CWnd* pWndParent = GetParent();

	CRect rcScreen;
	GetWindowRect(rcScreen);
	CPoint pt(rcScreen.left, rcScreen.top);
	CPoint ptNew(pt);

	// We need to offset the origin so the background is erased correctly. This was never an issue
	// with solid color fills, but now the background may be different depending on it's Y value.

	MapWindowPoints(pWndParent, &ptNew, 1);
	int nOffsetY = pt.y - ptNew.y;

	CPoint ptOld = pDC->SetBrushOrg(0, nOffsetY);
	pWndParent->SendMessage(WM_ERASEBKGND, (WPARAM)pDC->GetSafeHdc(), 0);
	pDC->SetBrushOrg(ptOld.x, ptOld.y);

	return TRUE;
}

// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
void CDocBar::OnNcPaint()
{
	// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
	/*
	if (g_bClassicToolbarBorders) {
		CDialogBar::OnNcPaint();
	}
	*/
	return;
}

// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing
void CDocBar::OnPaint()
{
	// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
	/*
	if (g_bClassicToolbarBorders) {
		CDialogBar::OnPaint();
	}
	*/
	CPaintDC(this);
}