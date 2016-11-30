// CSearchReportDescDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SearchReportDescDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSearchReportDescDlg dialog


CSearchReportDescDlg::CSearchReportDescDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSearchReportDescDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSearchReportDescDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSearchReportDescDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchReportDescDlg)
	DDX_Control(pDX, IDC_SEARCH_TEXT, m_nxeditSearchText);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchReportDescDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSearchReportDescDlg)
	ON_BN_CLICKED(IDC_SEARCH_BUTTON, OnSearchButton)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchReportDescDlg message handlers

void CSearchReportDescDlg::OnSearchButton() 
{
	CString strWhere;
	GetDlgItemText(IDC_SEARCH_TEXT, strWhere);

	strWhere.TrimRight();

	if(strWhere.IsEmpty()) {
		MsgBox("You must enter a string before searching.");
		return;
	}

	CWaitCursor pWait;

	//clear it out to start
	m_pReportList->Clear();

	strWhere.MakeUpper();
	BuildSearchArray(strWhere);

	//otherwise it's OK to go
	for(int i = 0; i < CReports::gcs_nKnownReportCount; i++) {
		const CReportInfo* pRep = &CReports::gcs_aryKnownReports[i];

		// (c.haag 2009-12-18 11:48) - PLID 29264 - Check licensing
		// (j.dinatale 2012-07-30 17:31) - PLID 51384 - check the current user permissions
		if(!CheckLicenseForReport(pRep, true) || !CheckCurrentUserReportAccess(pRep->nID, TRUE)) {
			continue;
		}

		CString strDescription = pRep->GetDescription();
		CString strDescToSearch = strDescription;
		strDescToSearch.MakeUpper();

		BOOL bFound = TRUE;

		//see if our search words are in the description somewhere
		for(int j = 0; j < m_arySearchParts.GetSize(); j++) {
			if(strDescToSearch.Find(m_arySearchParts.GetAt(j)) == -1) {
				bFound = FALSE;				
			}
		}

		if(!bFound) {
			//now see if our search words are in the name somewhere
			bFound = TRUE;			
			strDescToSearch = pRep->strPrintName;
			strDescToSearch.MakeUpper();
			for(j = 0; j < m_arySearchParts.GetSize(); j++) {
				if(strDescToSearch.Find(m_arySearchParts.GetAt(j)) == -1) {
					bFound = FALSE;	
				}
			}
		}

		if(bFound) {
			_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
			_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

			//add the report to the list
			IRowSettingsPtr pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long)pRep->nID);
			pRow->PutValue(1, _bstr_t(pRep->strPrintName));
			pRow->PutValue(2, _bstr_t(GetTabNameFromCategory(pRep->strCategory))); //z.manning, PLID 17373
			pRow->PutValue(3, _bstr_t(strDescription));
			pRow->PutValue(4, pRep->strCategory == "" ? varTrue : varFalse);

			m_pReportList->AddRow(pRow);
		}
	}

	ClearSearchArray();

	if(m_pReportList->GetRowCount()>0)
		AfxMessageBox("Search complete.");
	else
		AfxMessageBox("There were no records that matched your search.");
}

void CSearchReportDescDlg::BuildSearchArray(CString strSearchString)
{
	while(strSearchString.GetLength() > 0) {

		int nextSpace = strSearchString.Find(" ");
		
		if(nextSpace != -1) {
			//if empty, parse the last section
			CString strPart = strSearchString.Left(nextSpace);
			strPart.TrimRight();
			strPart.TrimLeft();
			if(strPart != "")
				m_arySearchParts.Add(strPart);
			strSearchString = strSearchString.Right(strSearchString.GetLength() - nextSpace - 1);
		}
		else {
			//if empty, parse the last section
			m_arySearchParts.Add(strSearchString);
			strSearchString = "";
		}
	}
}

void CSearchReportDescDlg::ClearSearchArray()
{
	for(int i = m_arySearchParts.GetSize() - 1; i >= 0; i--) {
		m_arySearchParts.RemoveAt(i);
	}
	m_arySearchParts.RemoveAll();
}

void CSearchReportDescDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

void CSearchReportDescDlg::OnOK() 
{
	CNxDialog::OnOK();
}

BOOL CSearchReportDescDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (z.manning, 04/28/2008) - PLID 29807 - Set button styles
		m_btnCancel.AutoSet(NXB_CLOSE);

		// (r.gonet 2015-11-12 03:01) - PLID 67466 - Bulk cache our configrt calls.
		g_propManager.CachePropertiesInBulk("CSearchReportDescDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'BatchPayments_EnableCapitation' "
			")",
			_Q(GetCurrentUserName()));

		m_pReportList = BindNxDataListCtrl(this, IDC_REPORT_DESC_LIST, GetRemoteData(), false);

		GetDlgItem(IDC_SEARCH_TEXT)->SetFocus();
	}NxCatchAll(__FUNCTION__);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CSearchReportDescDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSearchReportDescDlg)
	ON_EVENT(CSearchReportDescDlg, IDC_REPORT_DESC_LIST, 3 /* DblClickCell */, OnDblClickCellReportDescList, VTS_I4 VTS_I2)
	ON_EVENT(CSearchReportDescDlg, IDC_REPORT_DESC_LIST, 6 /* RButtonDown */, OnRButtonDownReportDescList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSearchReportDescDlg::OnDblClickCellReportDescList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	long nID = VarLong(m_pReportList->GetValue(nRowIndex, 0));	//get the report id

	BOOL bIsPreview = VarBool(m_pReportList->GetValue(nRowIndex, 4), FALSE);

	if(!bIsPreview) {
		// (z.manning 2009-06-11 17:44) - PLID 34589 - Check permission
		if(CheckCurrentUserReportAccess(nID, FALSE)) {
			AddReport(nID);
		}
	}
	else {
		AfxMessageBox("The selected report is not available in the Reports module, meaning that it is only able to be run from elsewhere in Practice.\n"
			"As such, this report cannot be batched in the reports module.");
	}
}

void CSearchReportDescDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if(pWnd->GetSafeHwnd() == GetDlgItem(IDC_REPORT_DESC_LIST)->GetSafeHwnd()) {
		long nCurSel = m_pReportList->GetCurSel();
		if(nCurSel == -1)
			return;

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		BOOL bIsPreview = VarBool(m_pReportList->GetValue(nCurSel, 4), FALSE);

		long nIndex = 0;
		// (z.manning 2009-06-11 17:45) - PLID 34589
		if(CheckCurrentUserReportAccess(VarLong(m_pReportList->GetValue(nCurSel, 0)), TRUE)) {
			mnu.InsertMenu(nIndex++, bIsPreview ? MF_BYPOSITION|MF_GRAYED : MF_BYPOSITION, ID_SEARCHREPORTDESC_ADDTOCURRENTBATCH, "&Add to Current Batch");
		}
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SEARCHREPORTDESC_VIEWDESCRIPTION, "&View Description");

		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
	}
}

BOOL CSearchReportDescDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(LOWORD(wParam)) {
	case ID_SEARCHREPORTDESC_ADDTOCURRENTBATCH:
		{
			try {
				long nCurSel = m_pReportList->GetCurSel();
				if(nCurSel == -1)
					break;

				long nID = VarLong(m_pReportList->GetValue(nCurSel, 0));
				BOOL bIsPreview = VarBool(m_pReportList->GetValue(nCurSel, 4), FALSE);
				if(!bIsPreview) {
					AddReport(nID);
				}
				else {
					AfxMessageBox("The selected report is not available in the Reports module, meaning that it is only able to be run from elsewhere in Practice.\n"
						"As such, this report cannot be batched in the reports module.");
				}

				return TRUE;
			} NxCatchAll("Error 1 in OnCommand()");
		}
		break;
	case ID_SEARCHREPORTDESC_VIEWDESCRIPTION:
		{
			try {
				long nCurSel = m_pReportList->GetCurSel();
				if(nCurSel == -1)
					break;

				long nRepID = VarLong(m_pReportList->GetValue(nCurSel, 0));

				const CReportInfo* pRep = NULL;
				for (long i = 0; i < CReports::gcs_nKnownReportCount; i++) {
					if (CReports::gcs_aryKnownReports[i].nID == nRepID) {
						pRep = &CReports::gcs_aryKnownReports[i];
					}
				}

				CString strName, strDesc, strTab;
				CString str;

				strName = pRep->strPrintName;
				strDesc = pRep->GetDescription();
				strTab = GetTabNameFromCategory(pRep->strCategory);
				

				str.Format( "Report Name:  %s\r\n"
							"Report Tab:   %s\r\n"
							"Report Description:  %s", strName, strTab, strDesc);

				MsgBox(str);

				return TRUE;
			} NxCatchAll("Error 2 in OnCommand()");
		}
		break;
	default:
		break;
	}
	return CNxDialog::OnCommand(wParam, lParam);
}

void CSearchReportDescDlg::OnRButtonDownReportDescList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1)
		return;

	//set the selection to this row
	m_pReportList->PutCurSel(nRow);
}

void CSearchReportDescDlg::AddReport(long nRepID)
{
	//we need to post a message to the reports dialog to add this report
	if(m_pReportView)	//this may be null
		m_pReportView->PostMessage(NXM_ADD_REPORT, (WPARAM)nRepID, 0);

}

//returns the name of a tab given it's category (like 'PatientP',etc)
CString CSearchReportDescDlg::GetTabNameFromCategory(CString strCat)
{
	// (j.gruber 2008-07-21 10:33) - PLID 28976 - add the new Practice Analysis tab
	if(strCat == "PatientP")
		return "Patients";
	else if(strCat == "OthrContactP")
		return "Contacts";
	else if(strCat == "MarketP")
		return "Marketing";
	else if(strCat == "InventoryP")
		return "Inventory";
	else if(strCat == "ScheduleP")
		return "Scheduler";
	else if(strCat == "ChargesP")
		return "Charges";
	else if(strCat == "PaymentsP")
		return "Payments";
	else if(strCat == "FinancialP")
		return "Financial";
	else if(strCat == "ASCP")
		return "ASC";
	else if(strCat == "AdminP")
		return "Administration";
	else if(strCat == "OtherP")
		return "Other";
	else if(strCat == "PracAnalP")
		return "Prac. Analysis";
	// (s.dhole 2012-04-19 17:11) - PLID 49341  added report category - optical
	else if(strCat == "OpticalP")
		return "Optical";
	
	else {
		//is a preview report
		return "<Not In Reports Module>";
	}
}
