#include "stdafx.h"

// cost.cpp : implementation file
//
#include "MarketCost.h"
#include "pracprops.h"
#include "MarketCostEntry.h"
#include "marketutils.h"
#include "NxStandard.h"
#include "DocBar.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDADD         42345
#define IDMODIFY      42346
#define IDDELETE      42347

// (c.haag 2009-02-16 11:53) - PLID 33109 - Column enumeration
enum EMarketCostColumns
{
	mccID = 0,
	mccDatePaid,
	mccPaidTo,
	mccDescription,
	mccRefNumber,
	mccAmount,
	mccEffectiveFrom,
	mccEffectiveTo,
	mccReferralSource,
	mccLocation
};

/////////////////////////////////////////////////////////////////////////////
// CMarketCostDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketCostDlg::CMarketCostDlg(CWnd* pParent)
	: CMarketingDlg(CMarketCostDlg::IDD, pParent)
	, mceDlg(this)
{
	//{{AFX_DATA_INIT(CMarketCostDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/cost.htm";
}


void CMarketCostDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketCostDlg)
	DDX_Control(pDX, IDC_ADD, m_addButton);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMarketCostDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketCostDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketCostDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketCostDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketCostDlg)
	ON_EVENT(CMarketCostDlg, IDC_NXDATALISTCTRL, 7 /* RButtonUp */, OnRButtonUp, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMarketCostDlg, IDC_NXDATALISTCTRL, 19 /* LeftClick */, OnLeftClickNxdatalistctrl, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMarketCostDlg, IDC_NXDATALISTCTRL, 3 /* DblClickCell */, OnDblClickCellNxdatalistctrl, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CMarketCostDlg::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();
	SetFilter(mfEffectivenessDate, mftDate);

	m_addButton.AutoSet(NXB_NEW);
	try 
	{	m_list = BindNxDataListCtrl(IDC_NXDATALISTCTRL, false);
		if (m_list == NULL)
			HandleException(NULL, "Error in CMarketCostDlg::OnInitDialog \n DataList is Null, Cannot Continue");
	}NxCatchAll("Error in CMarketCostDlg::OnInitDialog \n Could Not Bind DataList");

	m_bActive = false;
	// (s.dhole 2010-10-19 17:05) - PLID 35879  Set Filters 
	UpdateFilters();
	UpdateView();
	//a.walling 5/22/06 PLID 20695 updateview is immediately called by the NxTabView
	//	let's refresh instead
	Refresh();
	return TRUE;
}

void CMarketCostDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	//(e.lally 2009-08-28) PLID 35308 - Added try/catch
	try {
		//UpdateFilters();
		// (s.dhole 2010-10-19 17:04) - PLID 35879  Set filter type
		SetType(COSTS);
		// (s.dhole 2010-10-19 17:05) - PLID 35879  Hide unwanted filters 
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_PROVIDER|DBF_FILTER|DBF_CATEGORY|DBF_RESP);
		
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);

		if (m_bActive) {
			
			// (j.gruber 2006-12-05 11:38) - PLID 22889 - don't update if we are in a report window
			CNxTabView *pView = GetMainFrame()->GetActiveView();
			if (pView) {
				pView->RedrawWindow();
				Refresh();
			}
			
		}
		else {
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
		}
	}NxCatchAll(__FUNCTION__);
}

void CMarketCostDlg::UpdateFilters()
{
	//set up the date filters
	/*SetType(COSTS);*/
	SetFilter(mfEffectivenessDate, mftDate);
	SetFilter(mfCostLocation, mftLocation);
	SetFilter(mfDependantProvider, mftProvider);
	/*GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_PROVIDER|DBF_FILTER|DBF_CATEGORY|DBF_RESP);*/
}

void CMarketCostDlg::Refresh()
{
	try
	{
		CString from, to, sql;
		from = GetMainFrame()->m_pDocToolBar->GetFromDate();
		to = GetMainFrame()->m_pDocToolBar->GetToDate();
		CString strLocation = GetMainFrame()->m_pDocToolBar->GetLocationString();
		CString strDateField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftDate);
		if(GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			if (strDateField.CompareNoCase("MarketingCostsT.DatePaid") == 0) {
				if(strLocation.IsEmpty()) {
					sql.Format("%s >= '%s' AND %s < DATEADD(day,1,'%s')", strDateField, from, strDateField, to);
				}
				else {
					sql.Format("%s >= '%s' AND %s < DATEADD(day,1,'%s') AND MarketingCostsT.LocationID IN %s", strDateField, from, strDateField, to, strLocation);
				}
			}
			else {
				//its effectiveness date so we need to set the filter differently
				if(strLocation.IsEmpty()) {
					sql.Format(" (EffectiveFrom <= '%s' AND EffectiveTo >= '%s')", to, from);
				}
				else {
					sql.Format("(EffectiveFrom <= '%s' AND EffectiveTo >= '%s') AND MarketingCostsT.LocationID IN %s", to, from, strLocation);
				}
			}

		}
		else {
			if(!strLocation.IsEmpty())
				sql.Format("MarketingCostsT.LocationID IN %s", strLocation);
		}

		m_list->WhereClause = _bstr_t(sql);

		m_list->Requery();

	}NxCatchAll("Error in Refresh: ");
}

void CMarketCostDlg::OnAdd() 
{
	CMarketCostEntry dlg(this);
	dlg.m_bIsNew = true;
	if (IDOK == dlg.DoModal())
		Refresh();	
}

void CMarketCostDlg::OnRButtonUp(long nRow, short nCol, long x, long y, long nFlags) 
{
	//select the row they clicked
	m_list->PutCurSel(nRow);

	CMenu Popup;
	CPoint pt;
	CWnd* pwnd;

	pt.x = x;
	pt.y = y;
	pwnd = GetDlgItem(IDC_NXDATALISTCTRL);
	if (pwnd != NULL) 
	{	m_rightClicked = nRow;//remember where they clicked
		pwnd->ClientToScreen(&pt);
		Popup.m_hMenu = CreatePopupMenu();
		Popup.InsertMenu(1, MF_BYPOSITION, IDADD,	"Add");
		if (nRow != -1)
		{	Popup.InsertMenu(2, MF_BYPOSITION, IDMODIFY,"Edit");
			Popup.InsertMenu(3, MF_BYPOSITION, IDDELETE,"Delete");
		}
		Popup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
}

void CMarketCostDlg::OnModify(int id)
{
	//DRT - 11/8/01 - Made the mceDlg a member variable so only one can be open at a time
	if (IDOK == mceDlg.DoModal(id))
		Refresh();
}

void CMarketCostDlg::OnDelete(int id)
{
	try
	{
		// (c.haag 2009-02-16 11:51) - PLID 33109 - For auditing
		long nRow = m_list->FindByColumn(mccID, (long)id, 0, VARIANT_FALSE);
		CString strDesc, strReferralName;
		if (nRow > -1) {
			strDesc = VarString(m_list->GetValue(nRow, mccDescription), "");
			strReferralName = VarString(m_list->GetValue(nRow, mccReferralSource), "");
		} else {
			// This should never happen; just leave the descriptions blank
		}

		// (c.haag 2009-02-16 11:31) - PLID 33100 - Delete from related tables, too
		ExecuteSql(
			"DELETE FROM MarketingCostProcedureT WHERE MarketingCostID = %d;\r\n"
			"DELETE FROM MarketingCostsT WHERE ID = %i", id, id);

		// (c.haag 2009-02-16 11:51) - PLID 33109 - Audit the deletion
		CString desc;
		GetDlgItemText(IDC_NOTES,	desc);
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			AuditEvent(-1, strReferralName, nAuditID, aeiMarketCostDelete, -1, strDesc, "<Deleted>", aepMedium, aetDeleted);
		}

	}NxCatchAll("Could not delete cost");
	m_list->Requery();
}

BOOL CMarketCostDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) 
	{	case IDADD:
			OnAdd();
		break;
		case IDMODIFY:
			OnModify(m_list->GetValue(m_rightClicked, 0).lVal);
		break;
		case IDDELETE:
			OnDelete(m_list->GetValue(m_rightClicked, 0).lVal);
		break;
	}	
	return CMarketingDlg::OnCommand(wParam, lParam);
}

void CMarketCostDlg::OnLeftClickNxdatalistctrl(long nRow, short nCol, long x, long y, long nFlags) 
{
	//DRT 7/14/03 - Changed editing from a single click to a double click, like the rest of the program
}

void CMarketCostDlg::OnDblClickCellNxdatalistctrl(long nRowIndex, short nColIndex) 
{
	if (nRowIndex != -1)
		OnModify(VarLong(m_list->GetValue(nRowIndex, 0)));
}

void CMarketCostDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

// a.walling PLID 20695 6/05/06 DocBar Filters changed, so refresh
LRESULT CMarketCostDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	Refresh();

	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}