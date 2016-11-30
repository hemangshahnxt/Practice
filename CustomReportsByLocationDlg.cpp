// CustomReportsByLocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReportsRc.h"
#include "CustomReportsByLocationDlg.h"
#include "Reports.h"
#include "ReportInfo.h"

//TES 2/1/2010 - PLID 37143 - Created.  At the moment this is only used for the Lab Request Form, but it was designed to be extendable.
// CCustomReportsByLocationDlg dialog

IMPLEMENT_DYNAMIC(CCustomReportsByLocationDlg, CNxDialog)

CCustomReportsByLocationDlg::CCustomReportsByLocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCustomReportsByLocationDlg::IDD, pParent)
{

}

CCustomReportsByLocationDlg::~CCustomReportsByLocationDlg()
{
}

void CCustomReportsByLocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCATION_REPORTS_CAPTION, m_nxsCaption);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CCustomReportsByLocationDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CCustomReportsByLocationDlg::OnOK)
END_MESSAGE_MAP()

enum ReportListColumns {
	rlcLocationID = 0,
	rlcLocationName = 1,
	rlcReport = 2,
};

using namespace NXDATALIST2Lib;
// CCustomReportsByLocationDlg message handlers
BOOL CCustomReportsByLocationDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 2/1/2010 - PLID 37143 - Set up our datalist.
		m_pList = BindNxDataList2Ctrl(IDC_LOCATION_REPORTS_LIST, false);
		CString strFrom;
		strFrom.Format("LocationsT LEFT JOIN (SELECT * FROM LocationCustomReportsT WHERE ReportID = %li) AS LocationCustomReportsT "
			"ON LocationsT.ID = LocationCustomReportsT.LocationID", m_nReportID);
		m_pList->FromClause = _bstr_t(strFrom);

		switch(m_nReportID) {
			case 658:
			case 567: //TES 7/27/2012 - PLID 51849 - Lab Results Form
				m_pList->WhereClause = "LocationsT.TypeID = 2";
				break;
			default:
				//TES 2/1/2010 - PLID 37143 - The Request Form is the only report we support!
				ASSERT(FALSE);
				break;
		}

		//TES 2/1/2010 - PLID 37143 - Set up the embedded combo.
		IColumnSettingsPtr pCol = m_pList->GetColumn(rlcReport);
		CString strComboSource;
		strComboSource.Format("SELECT -2, '' "
			"UNION SELECT -1, '%s' "
			"UNION SELECT Number, Title FROM CustomReportsT WHERE ID = %li", 
			_Q(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(m_nReportID)].strPrintName), m_nReportID);
		pCol->ComboSource = _bstr_t(strComboSource);

		m_pList->Requery();

		switch(m_nReportID) {
			case 658:
				//TES 2/1/2010 - PLID 37143 - Explain the dialog to the user.
				m_nxsCaption.SetWindowText("For each lab location listed below, you may select a custom Lab Request Form to use by default.  "
					"When a lab record is assigned to a lab location, the custom report from this screen, if any, will be used as the default.  "
					"If there is no report selected on this screen, then whichever Lab Request Form you have designated as the default in the "
					"Edit Reports screen will be used.");
				break;
			case 567:
				//TES 7/27/2012 - PLID 51849 - Explain the dialog to the user.
				m_nxsCaption.SetWindowText("For each lab location listed below, you may select a custom Lab Results Form to use by default.  "
					"When a lab record is assigned to a lab location, the custom report from this screen, if any, will be used as the default.  "
					"If there is no report selected on this screen, then whichever Lab Results Form you have designated as the default in the "
					"Edit Reports screen will be used.");
				break;
			default:
				//TES 2/1/2010 - PLID 37143 - The Request Form is the only report we support!
				ASSERT(FALSE);
				break;
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CCustomReportsByLocationDlg::OnOK()
{
	try {
		
		//TES 2/1/2010 - PLID 37143 - Clear out the data.
		CString strSql = BeginSqlBatch();
		AddStatementToSqlBatch(strSql, "DELETE FROM LocationCustomReportsT WHERE ReportID = %li", m_nReportID);

		//TES 2/1/2010 - PLID 37143 - Now, add whatever defaults they've selected.
		IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow) {
			long nReportNumber = VarLong(pRow->GetValue(rlcReport),-2);
			if(nReportNumber != -2) {
				AddStatementToSqlBatch(strSql, "INSERT INTO LocationCustomReportsT (LocationID, ReportID, ReportNumber) "
					"VALUES (%li, %li, %li)", VarLong(pRow->GetValue(rlcLocationID)), m_nReportID, nReportNumber);
			}
			pRow = pRow->GetNextRow();
		}

		//TES 2/1/2010 - PLID 37143 - Commit
		ExecuteSqlBatch(strSql);

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}
