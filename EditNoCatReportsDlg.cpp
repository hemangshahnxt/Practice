// EditNoCatReportsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "reports.h"
#include "EditNoCatReportsDlg.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "EditREportPickerDlg.h"
#include "globalreportutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditNoCatReportsDlg dialog


CEditNoCatReportsDlg::CEditNoCatReportsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditNoCatReportsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditNoCatReportsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditNoCatReportsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditNoCatReportsDlg)
	DDX_Control(pDX, IDC_EDITNOCATREPORT, m_btnEditReport);
	DDX_Control(pDX, IDC_CLOSEEDITREPORTDLG, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditNoCatReportsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditNoCatReportsDlg)
	ON_BN_CLICKED(IDC_EDITNOCATREPORT, OnEditnocatreport)
	ON_BN_CLICKED(IDC_CLOSEEDITREPORTDLG, OnCloseeditreportdlg)
	ON_BN_CLICKED(IDCANCEL, OnCloseeditreportdlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditNoCatReportsDlg message handlers

void CEditNoCatReportsDlg::OnEditnocatreport() 
{
	
	//get the ID of the report that is higlighted
	long nCurSel = m_ReportList->GetCurSel();

	if (nCurSel == -1) {

		MsgBox("Please select a report to edit");
		return;
	}
	
	long nID = m_ReportList->GetValue(nCurSel, 0);

	//open the edit reports dialog with the ID
	CEditReportPickerDlg dlg(this);
	dlg.DoModal(nID);


	
}

BOOL CEditNoCatReportsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		GetMainFrame()->DisableHotKeys();

		// (z.manning, 04/28/2008) - PLID 29807 - Set button styles
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnEditReport.AutoSet(NXB_MODIFY);

		// (r.gonet 2015-11-12 03:01) - PLID 67466 - Bulk cache our configrt calls.
		g_propManager.CachePropertiesInBulk("CEditNoCatReportsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'BatchPayments_EnableCapitation' "
			")",
			_Q(GetCurrentUserName()));

		//Initialize the datalist
		m_ReportList = BindNxDataListCtrl(this, IDC_NOCATREPORTLIST, GetRemoteData(), false);

		//loop through all the reports in the list and add the ones that have no category and are editable
		//also leave out the statement reports
		for (long i = 0; i < CReports::gcs_nKnownReportCount; i++) {

			CReportInfo pReport(CReports::gcs_aryKnownReports[i]);

			//JMM 5-11-04 made all reports editble
			if (pReport.strCategory.IsEmpty() /*&& CReports::gcs_aryKnownReports[i].bEditable*/ && !IsStatement(pReport.nID)) {

				long nID = pReport.nID;
				CString strName = pReport.strPrintName;

				//DRT TODO - PLID 12076 - We need to revise the way we handle license-restricted reports so
				//	this code can read it.  For now, i need to get the gift certificate one blocked for
				//	non-spa folk, so I'm doing it this (shady) way.
				if (CheckLicenseForReport(&pReport, true)) {
					//add it to the list
					IRowSettingsPtr pRow = m_ReportList->GetRow(-1);
					pRow->PutValue(0, (long)nID);
					pRow->PutValue(1, (_variant_t)strName);
					m_ReportList->AddRow(pRow);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//TS 12/23/02: I moved this to globalreportutils
/*BOOL CEditNoCatReportsDlg::IsStatement(long nID) {

	if (nID == 234 || nID == 338 || nID == 169 || nID == 337 || nID == 353 || nID == 354 || nID == 355 || nID == 356 ) {

		return true;
	}
	else {
		return false;
	}

}*/


void CEditNoCatReportsDlg::OnCloseeditreportdlg() 
{
	CNxDialog::OnOK();
	
	GetMainFrame()->EnableHotKeys();
}
