// ReportGroupsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ReportGroupsDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "NxSecurity.h"
#include "GlobalReportUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CReportGroupsDlg dialog


CReportGroupsDlg::CReportGroupsDlg(CWnd* pParent)
	: CNxDialog(CReportGroupsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReportGroupsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReportGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReportGroupsDlg)
	DDX_Control(pDX, IDC_CLOSE_REPORT_GROUPS, m_btnClose);
	DDX_Control(pDX, IDC_RENAME_REPORT_GROUP, m_btnRename);
	DDX_Control(pDX, IDC_DELETE_REPORT_GROUP, m_btnDelete);
	DDX_Control(pDX, IDC_ADD_REPORT_GROUP, m_btnAdd);
	DDX_Control(pDX, IDC_MOVE_RIGHT, m_btnMoveOneRight);
	DDX_Control(pDX, IDC_MOVE_LEFT, m_btnMoveOneLeft);
	DDX_Control(pDX, IDC_MOVE_ALL_RIGHT, m_btnMoveAllRight);
	DDX_Control(pDX, IDC_MOVE_ALL_LEFT, m_btnMoveAllLeft);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CReportGroupsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CReportGroupsDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CLOSE_REPORT_GROUPS, OnCloseReportGroups)
	ON_BN_CLICKED(IDC_ADD_REPORT_GROUP, OnAddReportGroup)
	ON_BN_CLICKED(IDC_DELETE_REPORT_GROUP, OnDeleteReportGroup)
	ON_BN_CLICKED(IDC_DEFAULT_CHECK, OnDefaultCheck)
	ON_BN_CLICKED(IDC_MOVE_RIGHT, OnMoveRight)
	ON_BN_CLICKED(IDC_MOVE_LEFT, OnMoveLeft)
	ON_BN_CLICKED(IDC_MOVE_ALL_RIGHT, OnMoveAllRight)
	ON_BN_CLICKED(IDC_MOVE_ALL_LEFT, OnMoveAllLeft)
	ON_BN_CLICKED(IDC_RENAME_REPORT_GROUP, OnRenameReportGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportGroupsDlg message handlers



void CReportGroupsDlg::ShowNewReportsMessage() {


	try {

		//PLID 20025: pop up a message with the new reports if there are any which have been added since the last time they were in this dialog
		//now all the lastknownIds are the same, so we can just get the first one
		_RecordsetPtr rsLastKnown = CreateRecordset("SELECT Top 1 LastKnownID FROM ReportGroupsT ");
		if (rsLastKnown->eof) {

			//what!?!  perhaps they deleted all their report groups
			ASSERT(FALSE);
		}
		else {

			//see if the last known ID is the last report id for this release
			long nLastKnownID = AdoFldLong(rsLastKnown, "LastKnownId");

			if (nLastKnownID == (CReportInfo::REPORT_NEXT_INFO_ID - 1)) {

				//do nothing, they've already gotten this message
			}
			else {

				//loop through and generate the list of new IDs and what group they are currently in and then tell them
				CString strMessage;
				BOOL bDefaultGroup;
				//first we have to check if they have the preferenence to not put things into groups
				if (! GetRemotePropertyInt("Reports_DisAllow_New_Addition", 0, 0, "<None>", TRUE)) {
					bDefaultGroup = TRUE;
				}
				else {
					bDefaultGroup = FALSE;
				}

				strMessage = "There have been new reports added to the system since the last time you've modified your report groups.  \r\n";
				/*if (bDefaultGroup) {
					strMessage += "They have been put in default groups for you, listed below. \r\n";
				}*/
				
				//now loop 
				for (long i=0; i< CReports::gcs_nKnownReportCount; i++) {
					long nID = CReports::gcs_aryKnownReports[i].nID;
					if (nID > nLastKnownID) {
						CString strTmp;
						strTmp = CReports::gcs_aryKnownReports[i].strPrintName;
						if (bDefaultGroup) {
							//see what groups this report is already in
							_RecordsetPtr rsGroups = CreateRecordset("SELECT Name FROM ReportGroupsT LEFT JOIN "
								" ReportGroupDetailsT ON ReportGroupsT.ID = ReportGroupDetailsT.GroupID "
								" WHERE ReportID = %li", nID);

							CString strGroups = "";
							while (! rsGroups->eof) {
								strGroups += AdoFldString(rsGroups, "Name") + ", ";
								rsGroups->MoveNext();
							}

							//take off the last comma
							strGroups = strGroups.Left(strGroups.GetLength() - 2);

							//see if we found a comma
							if (strGroups.IsEmpty()) {
								strTmp += "\r\n";
							}
							else {

								if (strGroups.Find(",") == -1) {
									//there is only one group
									strTmp += " to the " + strGroups + " group by default. \r\n";
								}
								else {
									strTmp += " added to groups: " + strGroups + " \r\nby default.";
								}
							}
	
						}
						else {
							strMessage += "\r\n";
						}

						strMessage += strTmp;
					}
				}

				strMessage += "\r\n\r\nTo access just these reports, select <New Reports> under the Tabs filter";

				//pop it up!
				MsgBox(strMessage);
			}
		}

	}NxCatchAll("Error showing new reports message");
}


BOOL CReportGroupsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	m_pAvailList = BindNxDataListCtrl(IDC_ALL_REPORTS, GetRemoteData(), FALSE);	
	m_pCurrentList = BindNxDataListCtrl(IDC_REPORTS_FOR_GROUP, GetRemoteData(), FALSE);
	m_pGroupList = BindNxDataListCtrl(IDC_REPORT_GROUP_LIST, GetRemoteData(), TRUE);
	m_pTabList = BindNxDataListCtrl(IDC_TAB_LIST, GetRemoteData(), FALSE);

	m_btnMoveAllLeft.AutoSet(NXB_LLEFT);
	m_btnMoveAllRight.AutoSet(NXB_RRIGHT);
	m_btnMoveOneLeft.AutoSet(NXB_LEFT);
	m_btnMoveOneRight.AutoSet(NXB_RIGHT);
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnRename.AutoSet(NXB_MODIFY);
	m_btnDelete.AutoSet(NXB_DELETE);

	//set up the tab list
	IRowSettingsPtr pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("AllP"));
	pRow->PutValue(1, _variant_t("<All Reports>"));
	m_pTabList->InsertRow(pRow, 0);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("PatientP"));
	pRow->PutValue(1, _variant_t("Patients Tab"));
	m_pTabList->InsertRow(pRow, 1);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("OthrContactP"));
	pRow->PutValue(1, _variant_t("Contacts Tab"));
	m_pTabList->InsertRow(pRow, 2);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("MarketP"));
	pRow->PutValue(1, _variant_t("Marketing Tab"));
	m_pTabList->InsertRow(pRow, 3);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("InventoryP"));
	pRow->PutValue(1, _variant_t("Inventory Tab"));
	m_pTabList->InsertRow(pRow, 4);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("ScheduleP"));
	pRow->PutValue(1, _variant_t("Scheduler Tab"));
	m_pTabList->InsertRow(pRow, 5);
	
	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("ChargesP"));
	pRow->PutValue(1, _variant_t("Charges Tab"));
	m_pTabList->InsertRow(pRow, 6);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("PaymentsP"));
	pRow->PutValue(1, _variant_t("Payments Tab"));
	m_pTabList->InsertRow(pRow, 7);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("FinancialP"));
	pRow->PutValue(1, _variant_t("Financial Tab"));
	m_pTabList->InsertRow(pRow, 8);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("ASCP"));
	pRow->PutValue(1, _variant_t("ASC Tab"));
	m_pTabList->InsertRow(pRow, 9);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("AdminP"));
	pRow->PutValue(1, _variant_t("Administration Tab"));
	m_pTabList->InsertRow(pRow, 10);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("OtherP"));
	pRow->PutValue(1, _variant_t("Other Tab"));
	m_pTabList->InsertRow(pRow, 11);

	// (j.gruber 2008-08-22 09:39) - PLID 28976 - Practice Analysis Tab
	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("PracAnalP"));
	pRow->PutValue(1, _variant_t("Practice Analysis Tab"));
	m_pTabList->InsertRow(pRow, 12);

	// (s.dhole 2012-04-19 16:37) - PLID  49341 Optica tab
	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("OpticalP"));
	pRow->PutValue(1, _variant_t("Optical Tab"));
	m_pTabList->InsertRow(pRow, 13);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t(""));
	pRow->PutValue(1, _variant_t("<Preview Reports>"));
	m_pTabList->InsertRow(pRow, 14);

	pRow = m_pTabList->GetRow(-1);
	pRow->PutValue(0, _variant_t("NewP"));
	pRow->PutValue(1, _variant_t("<New Reports>"));
	m_pTabList->InsertRow(pRow, 15);

	//set the current selection to the all tabs
	m_pTabList->PutCurSel(0);

	//now add all the reports to the available list
	CString strReportName;
	long nReportID;
	for (long i=0; i< CReports::gcs_nKnownReportCount; i++) {
		//(m.thurston 2012-07-24) - PLID 51673 - Adding check to display internal only reports correctly 
		if(IsNexTechInternal() || !IsReportNexTechOnly(&CReports::gcs_aryKnownReports[i])){
			nReportID = CReports::gcs_aryKnownReports[i].nID;
			strReportName = CReports::gcs_aryKnownReports[i].strPrintName;

			pRow = m_pAvailList->GetRow(-1);
			pRow->PutValue(0, nReportID);
			pRow->PutValue(1, _variant_t(strReportName));

			m_pAvailList->AddRow(pRow);
		}
	}	

	ShowNewReportsMessage();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CReportGroupsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CReportGroupsDlg)
	ON_EVENT(CReportGroupsDlg, IDC_REPORT_GROUP_LIST, 18 /* RequeryFinished */, OnRequeryFinishedReportGroupList, VTS_I2)
	ON_EVENT(CReportGroupsDlg, IDC_REPORT_GROUP_LIST, 16 /* SelChosen */, OnSelChosenReportGroupList, VTS_I4)
	ON_EVENT(CReportGroupsDlg, IDC_TAB_LIST, 16 /* SelChosen */, OnSelChosenTabList, VTS_I4)
	ON_EVENT(CReportGroupsDlg, IDC_REPORTS_FOR_GROUP, 3 /* DblClickCell */, OnDblClickCellReportsForGroup, VTS_I4 VTS_I2)
	ON_EVENT(CReportGroupsDlg, IDC_ALL_REPORTS, 3 /* DblClickCell */, OnDblClickCellAllReports, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CReportGroupsDlg::OnRequeryFinishedReportGroupList(short nFlags) 
{
	//set the current selection to the first value
	if (m_pGroupList->GetRowCount() > 0) {
		m_pGroupList->PutCurSel(0);
		OnSelChosenReportGroupList(0);
	}
	
}

void CReportGroupsDlg::OnSelChosenReportGroupList(long nRow) 
{
	try {

		if (m_pGroupList->CurSel == -1) {

			//wait for the requery
			m_pGroupList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}

		//clear the report list
		m_pCurrentList->Clear();

		//it should be set now, bu check again, just to be safe
		if (m_pGroupList->CurSel == -1) {
			return;
		}

		//ok now we can proceed
		long nCurSel = m_pGroupList->CurSel;
		_variant_t var = m_pGroupList->GetValue(nCurSel, 0);
		long nGroupID = VarLong(var);

		_RecordsetPtr rs = CreateRecordset("SELECT ReportID FROM ReportGroupDetailsT WHERE GroupID = %li", nGroupID);
		long nReportID;
		while (!rs->eof) {

			nReportID = AdoFldLong(rs, "ReportID");

			// (j.jones 2008-05-02 12:02) - PLID 28261 - make sure the report still exists
			long nReportIndex = CReportInfo::GetInfoIndex(nReportID);
			if(nReportIndex != -1) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[nReportIndex]);
				CString strReportName;
				strReportName = infReport.strPrintName;
				IRowSettingsPtr pRow = m_pCurrentList->GetRow(-1);
				pRow->PutValue(0, nReportID);
				pRow->PutValue(1, _variant_t(strReportName));
				m_pCurrentList->AddRow(pRow);
			}

			rs->MoveNext();
		}

	}NxCatchAll("Error in CReportGroupsDlg::OnSelChosenReportGroupList");
}

HBRUSH CReportGroupsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*

	static CBrush br(PaletteColor(COLORREF(0x00DEB05C)));


	switch(pWnd->GetDlgCtrlID()) {
		
	case IDC_DEFAULT_CHECK: 
	case IDC_STATIC: {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00DEB05C));
			return (HBRUSH)br;
							}
		break;
	}
	
	
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CReportGroupsDlg::OnCloseReportGroups() 
{
	try {

		//check to make sure that there are no groups with out any reports in them because that would be dumb
		_RecordsetPtr rsCheck = CreateRecordset("SELECT Name FROM ReportGroupsT WHERE ID NOT IN (SELECT GroupID FROM ReportGroupDetailsT GROUP BY GroupID)");
		CString strReportList = "";
		while (! rsCheck->eof) {
			strReportList += AdoFldString(rsCheck, "Name") + ", ";

			rsCheck->MoveNext();
		}

		if (! strReportList.IsEmpty()) {

			//take off the last comma
			strReportList = strReportList.Left(strReportList.GetLength() - 2);

			//output the message
			CString strMsg;
			strMsg.Format("The following report groups have no reports in them. Either add reports or delete the group before exiting the dialog.\n%s", strReportList);

			MsgBox(strMsg);
			return;
		}

		//update the lastknownIDs of the groups
		ExecuteSql("UPDATE ReportGroupsT SET LastKnownID = %li ", CReportInfo::REPORT_NEXT_INFO_ID - 1);

		CWaitCursor wait;
		extern BOOL g_bIsAvailReportsLoaded;
		g_bIsAvailReportsLoaded = FALSE;
		LoadAvailableReportsMap();

		CDialog::OnOK();
			
	}NxCatchAll("Error Closing report groups");	
	
}

void CReportGroupsDlg::OnAddReportGroup() 
{

	try {
		//prompt for a name
		CString strNewName;
		if (IDOK == (InputBoxLimited(this, "Please enter a name for the new report group.", strNewName, "",50,false,false,NULL))) {

			//check to see if it already exists
			if (ReturnsRecords("SELECT ID FROM ReportGroupsT WHERE Name = '%s'", _Q(strNewName))) {

				MessageBox("There is already a report group with this name, please choose another.");
				return;
			}

			long nNewGroupID = NewNumber("ReportGroupsT", "ID");
			
			//create the new group
			ExecuteSql("INSERT INTO ReportGroupsT (ID, Name, LastKnownID) VALUES "
				" (%li, '%s', %li)", nNewGroupID, _Q(strNewName), (CReportInfo::REPORT_NEXT_INFO_ID - 1));

			CString strDesc;
			strDesc.Format("Controls access to preview or print reports in this group. \r\n"
				"Note: Access is granted on a \"Access First\" basis, this means that if "
				"a report is in two groups and a user has permissions for only one of those groups, "
				"the user will be able to run the report");

			AddUserDefinedPermission(nNewGroupID, sptView, strNewName, strDesc, bioReportGroup, NULL);

			//add the group to the list
			IRowSettingsPtr pRow = m_pGroupList->GetRow(-1);
			pRow->PutValue(0, nNewGroupID);
			pRow->PutValue(1, _variant_t(strNewName));
			m_pGroupList->AddRow(pRow);
			long nRow = m_pGroupList->SetSelByColumn(0, nNewGroupID);
			OnSelChosenReportGroupList(nRow);
		}
	}NxCatchAll("Error adding new report group");
	
}

void CReportGroupsDlg::OnDeleteReportGroup() 
{
	try {

		long nCurSel = m_pGroupList->CurSel;
		if (nCurSel == -1) {
			return;
		}

		long nValue = VarLong(m_pGroupList->GetValue(nCurSel, 0));


		if (IDYES == MsgBox(MB_YESNO, "This will remove this group and any permissions associated with it, are you sure you wish to continue?")) {

			//remove the permissions first
			DeleteUserDefinedPermission(bioReportGroup, nValue, TRUE);

			//now delete the report group
			ExecuteSql("DELETE FROM ReportGroupDetailsT WHERE GroupID = %li", nValue);
			ExecuteSql("DELETE FROM ReportGroupsT WHERE ID = %li", nValue);

			m_pGroupList->RemoveRow(nCurSel);
			OnSelChosenReportGroupList(0);

		}
	}NxCatchAll("Error Deleting Report Group");
	
}

void CReportGroupsDlg::OnDefaultCheck() 
{

	
}

void CReportGroupsDlg::OnCancel(){

	OnCloseReportGroups();
}


void CReportGroupsDlg::OnMoveRight() 
{
	MoveOneRight();
	
}

void CReportGroupsDlg::OnMoveLeft() 
{
	MoveOneLeft();
	
}

void CReportGroupsDlg::OnMoveAllRight() 
{
	try {
		
		long nGroupSel = m_pGroupList->CurSel;
		if (nGroupSel != -1) {

			long nGroupID = VarLong(m_pGroupList->GetValue(nGroupSel, 0));
		
			//take all the reports in the available list and move them to the selected list
			long p = m_pAvailList->GetFirstRowEnum();
		
			LPDISPATCH lpDisp = NULL;
			while (p) {
				
				//insert them into this group
				m_pAvailList->GetNextRowEnum(&p, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
				long nReportID = VarLong(pRow->GetValue(0));
				if (nReportID != -1 && IsRecordsetEmpty("SELECT GroupID FROM ReportGroupDetailsT WHERE GroupID = %li AND ReportID = %li", nGroupID, nReportID)) {
					ExecuteSql("INSERT INTO ReportGroupDetailsT (GroupID, ReportID) VALUES (%li, %li)",
						nGroupID, nReportID);

					//Update the last known ID
					ExecuteSql("UPDATE ReportGroupsT SET LastKnownID = %li WHERE ID = %li", (CReportInfo::REPORT_NEXT_INFO_ID - 1), nGroupID);

					m_pCurrentList->AddRow(pRow);
				}
						
			}
		}	
	}NxCatchAll("Error Adding all reports");

}

void CReportGroupsDlg::MoveOneRight() {
	
	try {

		//first, check to make sure we have a valid group
		long nGroupSel = m_pGroupList->CurSel;
		if (nGroupSel != -1) {
			long nCurSel = m_pAvailList->GetCurSel();
			
			if (nCurSel != -1) {
			
				IRowSettingsPtr pRow;
				pRow = m_pAvailList->GetRow(nCurSel);
				long nGroupID = VarLong(m_pGroupList->GetValue(nGroupSel, 0));
				long nReportID = VarLong(pRow->GetValue(0));
			
				if (nReportID != -1) {

					//make sure its not already in this group
					// (c.haag 2010-04-27 09:13) - PLID 21688 - Make sure it's selected in the other list if
					// it already exists. It would help the user as a visual queue if it's already on the screen,
					// and we can say that all paths in this function lead to the item being selected in the
					// "current" list.
					if (m_pCurrentList->FindByColumn(0, nReportID, 0, VARIANT_TRUE) == -1) {
										
						//add it to the data
						ExecuteSql("INSERT INTO ReportGroupDetailsT (GroupID, ReportID) VALUES "
							" (%li, %li)", nGroupID, nReportID);		

						//Update the last known ID
						ExecuteSql("UPDATE ReportGroupsT SET LastKnownID = %li WHERE ID = %li", (CReportInfo::REPORT_NEXT_INFO_ID - 1), nGroupID);
				
						//add the row to the other datalist
						// (c.haag 2010-04-27 09:13) - PLID 21688 - Also select it
						m_pCurrentList->CurSel = m_pCurrentList->AddRow(pRow);												
					}
				}
			}
				
		}
	}NxCatchAll("Error in CReportGroupsDlg::MoveOneRight()");
	

}

void CReportGroupsDlg::MoveOneLeft() {

	try {
		long nGroupSel = m_pGroupList->CurSel;
		if (nGroupSel != -1) {
			
			long nCurSel = m_pCurrentList->CurSel;
			if (nCurSel != -1) {
		
				//get the currently selected row
				long nGroupID = VarLong(m_pGroupList->GetValue(nGroupSel, 0));
				long nReportID = VarLong(m_pCurrentList->GetValue(nCurSel, 0));

				//take the report out of the group
				ExecuteSql("DELETE FROM ReportGroupDetailsT WHERE GroupID = %li AND ReportID = %li",
					nGroupID, nReportID);

				//Update the last known ID
				ExecuteSql("UPDATE ReportGroupsT SET LastKnownID = %li WHERE ID = %li", (CReportInfo::REPORT_NEXT_INFO_ID - 1), nGroupID);
	
				//take the row out of the currently selected list
				m_pCurrentList->RemoveRow(nCurSel);
			}
			
		}
	}NxCatchAll("Error in CReportGroupsDlg::MoveOneLeft()");


}

void CReportGroupsDlg::OnMoveAllLeft() 
{
	try {
		
		long nGroupSel = m_pGroupList->CurSel;
		if (nGroupSel != -1) {
			//clear the list, warn them first though
			if (IDYES == MsgBox(MB_YESNO, "This will take all the reports out of this group.\nAre you sure you want to do this?")) {
				//loop through the list and take all the rows out of the group in data
				//take all the reports in the available list and move them to the selected list
				long p = m_pCurrentList->GetFirstRowEnum();
				long nGroupID = VarLong(m_pGroupList->GetValue(nGroupSel, 0));
				LPDISPATCH lpDisp = NULL;
				while (p) {
					m_pCurrentList->GetNextRowEnum(&p, &lpDisp);
					IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
					long nReportID = VarLong(pRow->GetValue(0));

					//delete them from the group
					ExecuteSql("DELETE FROM ReportGroupDetailsT WHERE GroupID = %li and ReportID = %li",
						nGroupID, nReportID);

					//Update the last known ID
					ExecuteSql("UPDATE ReportGroupsT SET LastKnownID = %li WHERE ID = %li", (CReportInfo::REPORT_NEXT_INFO_ID - 1), nGroupID);
				}

				//now that we are done with that, clear the list
				m_pCurrentList->Clear();
							
			}
		}
	
	}NxCatchAll("Error Adding all reports");	
}

void CReportGroupsDlg::OnSelChosenTabList(long nRow) 
{

	//first, clear the list
	m_pAvailList->Clear();
	long nCurSel = m_pTabList->CurSel;
	IRowSettingsPtr pRow;

	//get what the current ID is
	CString strID = VarString(m_pTabList->GetValue(nCurSel, 0));
	if (strID == "AllP") {
		//special case, show all reports
		CString strReportName;
		long nReportID;
		for (long i=0; i< CReports::gcs_nKnownReportCount; i++) {

			nReportID = CReports::gcs_aryKnownReports[i].nID;
			strReportName = CReports::gcs_aryKnownReports[i].strPrintName;

			pRow = m_pAvailList->GetRow(-1);
			pRow->PutValue(0, nReportID);
			pRow->PutValue(1, _variant_t(strReportName));
			
			m_pAvailList->AddRow(pRow);
		}	
	}
	else if (strID == "NewP") {

		if (m_pGroupList->CurSel == -1) {
			pRow = m_pAvailList->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _variant_t("<No New Reports>"));
			pRow->ForeColor =  RGB(196,191,189);
			pRow->ForeColorSel =  RGB(196,191,189);
			m_pAvailList->AddRow(pRow);
			return;
		}


		//get the currently selected group
		long nGroupID = VarLong(m_pGroupList->GetValue(m_pGroupList->CurSel, 0));
		//get what the greatest last known ID is for all the groups
		_RecordsetPtr rsMax = CreateRecordset("SELECT LastKnownID AS MaxID FROM ReportGroupsT WHERE ID = %li", nGroupID);
		if (! rsMax->eof) {

			long nMaxID = AdoFldLong(rsMax, "MaxID");
			long nReportID;
			CString strReportName;

			//loop through and add any report that is greater than the report ID
			long nCount = 0;
			for (int i = 0; i < CReports::gcs_nKnownReportCount; i++) {

				if (CReports::gcs_aryKnownReports[i].nID > nMaxID) {
	
					nReportID = CReports::gcs_aryKnownReports[i].nID;
					strReportName = CReports::gcs_aryKnownReports[i].strPrintName;


					pRow = m_pAvailList->GetRow(-1);
					pRow->PutValue(0, nReportID);
					pRow->PutValue(1, _variant_t(strReportName));
					nCount++;
					m_pAvailList->AddRow(pRow);
				}
			}	

			//make sure that if there is nothing in the list then we put something there
			if (nCount == 0) {
			
				pRow = m_pAvailList->GetRow(-1);
				pRow->PutValue(0, (long)-1);
				pRow->PutValue(1, _variant_t("<No New Reports>"));
				pRow->ForeColor =  RGB(196,191,189);
				pRow->ForeColorSel =  RGB(196,191,189);
			
				m_pAvailList->AddRow(pRow);
			}
		}
		else {
			pRow = m_pAvailList->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _variant_t("<No New Reports>"));
			pRow->ForeColor =  RGB(196,191,189);
			pRow->ForeColorSel =  RGB(196,191,189);
		
			m_pAvailList->AddRow(pRow);
		}
	}
	else {
		//special case, show all reports
		CString strReportName;
		long nReportID;
		for (long i=0; i< CReports::gcs_nKnownReportCount; i++) {

			nReportID = CReports::gcs_aryKnownReports[i].nID;
			strReportName = CReports::gcs_aryKnownReports[i].strPrintName;

			if (CReports::gcs_aryKnownReports[i].strCategory.CompareNoCase(strID) == 0) {
				pRow = m_pAvailList->GetRow(-1);
				pRow->PutValue(0, nReportID);
				pRow->PutValue(1, _variant_t(strReportName));
			
				m_pAvailList->AddRow(pRow);
			}
		}	
	}
	
	
}

void CReportGroupsDlg::OnDblClickCellReportsForGroup(long nRowIndex, short nColIndex) 
{
	//make sure that this is the currently selected row
	// (c.haag 2010-04-27 09:13) - PLID 21688 - This used to be the wrong list!
	if (nRowIndex != m_pCurrentList->GetCurSel()) {
		m_pCurrentList->CurSel = nRowIndex;
	}

	MoveOneLeft();
	
}

void CReportGroupsDlg::OnDblClickCellAllReports(long nRowIndex, short nColIndex) 
{
	//make sure that this is the currently selected row
	// (c.haag 2010-04-27 09:13) - PLID 21688 - This used to be the wrong list!
	if (nRowIndex != m_pAvailList->GetCurSel()) {
		m_pAvailList->CurSel = nRowIndex;
	}
	

	MoveOneRight();
	
}

void CReportGroupsDlg::OnRenameReportGroup() 
{
	try {
		long nCurSel = m_pGroupList->CurSel;

		if (nCurSel == -1) {
			return;
		}

		//get the current report group ID
		long nGroupID = VarLong(m_pGroupList->GetValue(nCurSel, 0));

		//prompt for new name
		CString strNewName;
		InputBoxLimited(this, "New Report Group Name:", strNewName, "",50,false,false,NULL);

		//make sure it doesn't already exist
		if (ReturnsRecords("SELECT ID FROM ReportGroupsT WHERE Name = '%s'", strNewName)) {

			//output a messagebox
			MsgBox("There is already a report group with that name, please choose a different name");
			return;
		}

		//we are good to go
		ExecuteSql("Update ReportGroupsT SET Name = '%s' WHERE ID = %li", _Q(strNewName), nGroupID);

		//Now, update the list on screen.
		IRowSettingsPtr pRow = m_pGroupList->GetRow(nCurSel);
		pRow->PutValue(1, _variant_t(strNewName));

		//now update the permission name
		UpdateUserDefinedPermissionName(bioReportGroup, nGroupID, strNewName);
	
	}NxCatchAll("Error renaming group");

	
	
}
