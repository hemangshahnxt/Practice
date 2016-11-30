// EditReportPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReportsRc.h"
#include "reports.h"
#include "EditReportPickerDlg.h"
#include "EditReportDlg.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "CustomReportsByLocationDlg.h"
#include "LabReqCustomFieldsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_EDIT_CUSTOM_REPORT_FIELDS 39000

//defined in reportInfo.h
/*
#define INPUT -13
#define SERVICE -12
#define ONEREPORT -11
#define DETAILED  -10
#define SUMMARY  -9
#define DETAILEDSERVICE -8
#define DETAILEDINPUT  -7
#define SUMMARYSERVICE -6
#define SUMMARYINPUT -5
#define STATEMENTDTLDAVERY1 -4
#define STATEMENTDTLDAVERY2 -3
#define STATEMENTSMRYAVERY1 -2
#define STATEMENTSMRYAVERY2 -1
#define NODEFAULT
*/

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditReportPickerDlg dialog


CEditReportPickerDlg::CEditReportPickerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditReportPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditReportPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CEditReportPickerDlg::CEditReportPickerDlg(CWnd* pParent, CReportInfo * CurrReport)
	: CNxDialog(CEditReportPickerDlg::IDD, pParent)
{
	m_CurrReport = CurrReport;
}


void CEditReportPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditReportPickerDlg)
	DDX_Control(pDX, IDC_NEWREPORT, m_btnNewReport);
	DDX_Control(pDX, IDC_EDITREPORT, m_btnEditReport);
	DDX_Control(pDX, IDC_CUSTOM_REPORT_DELETE, m_btnDeleteReport);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_btnMakeDefault);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_REVERT, m_btnRevert);
	DDX_Control(pDX, IDC_EXTRA_BTN, m_btnExtra);
	DDX_Control(pDX, IDC_EXTRA_CHECKBOX, m_checkExtra);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditReportPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditReportPickerDlg)
	ON_BN_CLICKED(IDC_EDITREPORT, OnEditreport)
	ON_BN_CLICKED(IDC_MAKEDEFAULT, OnMakedefault)
	ON_BN_CLICKED(IDC_NEWREPORT, OnNewreport)
	ON_BN_CLICKED(IDC_REVERT, OnRevert)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CUSTOM_REPORT_DELETE, OnDelete)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EXTRA_BTN, &CEditReportPickerDlg::OnExtraBtn)
	ON_BN_CLICKED(IDC_EXTRA_CHECKBOX, &CEditReportPickerDlg::OnExtraCheckbox)
	ON_COMMAND(ID_EDIT_CUSTOM_REPORT_FIELDS, &CEditReportPickerDlg::OnEditCustomReportFields)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditReportPickerDlg message handlers

BEGIN_EVENTSINK_MAP(CEditReportPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditReportPickerDlg)
	ON_EVENT(CEditReportPickerDlg, IDC_EDITREPORTLIST, 2 /* SelChanged */, OnSelChangedEditreportlist, VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEditReportPickerDlg, IDC_EDITREPORTLIST, 7, CEditReportPickerDlg::RButtonUpEditreportlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CEditReportPickerDlg::OnSelChangedEditreportlist(long nNewSel) 
{
	try {
		//check what they have permissions for 
		//we don't have to check the read permission because we already checked that before 
		//they got into this dialog because if they couldn't read, then they wouldn't be here
		if (nNewSel != -1) {
			// check to see what the number of the report is
			long nCustomReportID = VarLong(m_ReportPicker->GetValue(nNewSel, erpclNumber), -1);

			if (nCustomReportID < 0) {
				
				//check to see if this report is a statement report because if so, we are graying out the make default button.
				GetDlgItem(IDC_MAKEDEFAULT)->EnableWindow(!IsStatement(m_CurrReport->nID));


				//if they are on a non-custom report, grey out the edit button
				GetDlgItem(IDC_EDITREPORT)->EnableWindow(FALSE);
				// (r.gonet 10/11/2011) - PLID 46437 - Hide the extra checkbox for the system standard report.
				m_checkExtra.ShowWindow(SW_HIDE);
			}
			else {

				GetDlgItem(IDC_EDITREPORT)->EnableWindow(TRUE);
				GetDlgItem(IDC_NEWREPORT)->EnableWindow(TRUE);
				GetDlgItem(IDC_MAKEDEFAULT)->EnableWindow(TRUE);
				// (r.gonet 10/11/2011) - PLID 46437 - Update the custom checkbox, it is per custom report
				if(UseExtraCheckbox()) {
					m_checkExtra.ShowWindow(SW_SHOW);
					m_checkExtra.SetCheck(VarBool(m_ReportPicker->GetValue(nNewSel, erpclGenerateBarcode), FALSE));
				}
			}
				
					
		}
		else {
			//They have nothing selected.
			GetDlgItem(IDC_EDITREPORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAKEDEFAULT)->EnableWindow(FALSE);
			// (r.gonet 10/11/2011) - PLID 46437 - Naturally then, don't let them check anything.
			m_checkExtra.ShowWindow(SW_HIDE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEditReportPickerDlg::OnEditreport() 
{

	try {

		//check to see if they have permission to do this
		if (CheckCurrentUserPermissions(bioReportDesigner, sptCreate)) {
			

			bool bIsSaved, bIsCustom;
			CString strSaveFileName;

			//open the editor
			if(!OpenReportEditor(bIsSaved, bIsCustom, strSaveFileName)) {
				//It failed, and also told the user why (we trust).
				return;
			}

			if (bIsCustom && bIsSaved) {

				//we have to kill the dialog in order to save the report
				
				CString strName, strTemp, strTemp2;

				strName = GetCustomReportsPath() ^ strSaveFileName;
				strTemp = GetCustomReportsPath() ^ strSaveFileName + "tmp";
				strTemp2 = GetCustomReportsPath() ^ strSaveFileName + "t";


				//rename the first file
				/*the file we want to save is called fileName.rpttmp
				so we need to take the open file name and rename it to something else*/
				if (MoveFile(strName, strTemp2))  {
					
					//rename the report back to what it was before
					/*Now, we have to take the filename.rpttmp and rename it to 
					the filename of what we opened*/
					if (MoveFile(strTemp, strName)) {

						//delete the Temporary FileName.rptT  report
						CFile::Remove(strTemp2);
						
					}
					else {
				
						//output the error
						CString strError;
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
						strError.ReleaseBuffer();
						MessageBox("Could not Save Report.\nError: " + strError);
					}
				}
				else {

					//output the error
					CString strError;
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
					strError.ReleaseBuffer();
					MessageBox("Could not Save Report.\nError: " + strError);
			

				}
			}

			long nCurSel = m_ReportPicker->CurSel;
			Refresh();
			m_ReportPicker->CurSel = nCurSel;
			OnSelChangedEditreportlist(m_ReportPicker->CurSel);

		}
	}NxCatchAll("Error Opening Edit Dialog");

	
}


bool CEditReportPickerDlg::OpenReportEditor(OUT bool &bIsSaved, OUT bool &bIsCustom, OUT CString &strSaveFileName) {

	//get the number out of the datalist
	long nCurSel = m_ReportPicker->GetCurSel();
	if (nCurSel == -1) {
			MsgBox("Please Select a report to Edit");
			return false;
	}

	
	//DRT 12/30/2004 - PLID 15150 - Everywhere else checks this with an overload for -1 if NULL.  It (was) possible
	//	to get into a state where you could click 'Edit' on a default report, and this would get grumpy.
	long nNumber = VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1);
	CString strFileName;

	bIsCustom = GetReportFileName(nCurSel, strFileName);

	if(!bIsCustom) {
		MsgBox("You cannot edit a default report.  Please use the 'New' button to create a new report.");
		return false;
	}

	//Let's see if we can access this file.
	CFile fReport;
	CFileException *e = new CFileException;
	if(!fReport.Open(GetCustomReportsPath() ^ strFileName, CFile::modeReadWrite|CFile::shareExclusive, e)) {
		if(e->m_cause == CFileException::sharingViolation) {
			//Nope.
			MsgBox("The report file %s could not be opened for editing.\n"
				"This is most likely because it is already being viewed on this or another computer.\n"
				"Please ensure that this report is not open on any computer (use the Window menu to see all open windows),\n"
				"and try editing the report again.", GetCustomReportsPath() ^ strFileName);
			e->Delete();
			return false;
		}
		else if(e->m_cause == CFileException::fileNotFound) {
			MsgBox("Practice could not find the report file %s.  Please ensure that your NxDock settings are correct, or contact Nextech for assistance.", GetCustomReportsPath() ^ strFileName);
			e->Delete();
			return false;
		}
		else {
			throw e;
		}
	}
	e->Delete();
	//Don't forget to close it!
	fReport.Close();

	//CNxDialog::OnOK();
	CEditReportDlg  dlg(this, m_CurrReport, strFileName, strFileName, bIsCustom, nNumber, 0);
	dlg.DoModal();


	//set the out variables
	bIsSaved = dlg.m_bSaved;
	bIsCustom = dlg.m_bIsCustom;
	strSaveFileName = dlg.m_strSaveFileName;
	return true;

}



bool CEditReportPickerDlg::GetReportFileName(long nRow, CString &strFileName) {

	strFileName = m_CurrReport->strReportFile;
	bool bIsCustom = false;
	
	long nCustomReportID = VarLong(m_ReportPicker->GetValue(nRow, erpclNumber), -1);
	if(nCustomReportID == -1) {

		//if its a statement, we have to get it back to what it was before
		if (IsStatement(m_CurrReport->nID)) {
			
			strFileName.MakeLower();
			long nPos;

			if (strFileName.IsEmpty() || (strFileName.Find("ebillstatement") != -1)) {
				//this can only happen if they have chosen to run a default report, and
				//there is no default statement report for this report, so we need to find the name ourselves
				//or if it is an e-statement

				//check to see which report we are running
				strFileName = GetStatementFileName(m_CurrReport->nID);	

			}

			//the report file might be a custom report, but the report they clicked on isn't.  Take that into account
			if (strFileName.Find("custom") >= 0) {
				//take off the custom
				strFileName = strFileName.Right(strFileName.GetLength() - 6);
			}
			
			//see if it has the .rpt
			if (strFileName.Find(".rpt") > 0) {
				strFileName = strFileName.Left(strFileName.GetLength() - 4);
			}

			//take off the avery
			nPos = strFileName.Find("avery");
			if (nPos > 0) {
				strFileName = strFileName.Left(nPos);
			}

			//now take off the dtld or summary
			if (strFileName.Find("dtld") > 0 || strFileName.Find("smry") > 0) {
				strFileName = strFileName.Left(strFileName.GetLength() - 4);
			}

			//we should be good to go now
		}
		
		long nDate = VarLong(m_ReportPicker->GetValue(nRow, erpclDateOption), -1);
		if(nDate != -1) {
			strFileName += m_CurrReport->GetDateSuffix(nDate);
		}
		long nDetail = VarLong(m_ReportPicker->GetValue(nRow, erpclDetailOption), -1);
		if(nDetail != -1) {
			if(nDetail == 1) {
				strFileName += "dtld";
			}
			else if(nDetail == 2) {
				strFileName += "smry";
			}
		}
	
		long nAvery = VarLong(m_ReportPicker->GetValue(nRow, erpclAveryStyle), -1);
		if(nAvery != -1) {
			if(nAvery == 1) {
				strFileName += "Avery1";
			}
			else if(nAvery == 2) {
				strFileName += "Avery2";
			}
		}
	
		strFileName += ".rpt";
	}
	else {
		//it's a custom report that they picked

		_RecordsetPtr rs = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = %li and Number = %li", m_CurrReport->nID, nCustomReportID);
		strFileName = AdoFldString(rs, "FileName");
		bIsCustom = true;

	}

	return bIsCustom;


}

BOOL CEditReportPickerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (z.manning, 04/28/2008) - PLID 29807 - Set button styles
		m_btnNewReport.AutoSet(NXB_NEW);
		m_btnEditReport.AutoSet(NXB_MODIFY);
		m_btnDeleteReport.AutoSet(NXB_DELETE);
		m_btnMakeDefault.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnRevert.AutoSet(NXB_MODIFY);
		//TES 2/1/2010 - PLID 37143 - Show or hide the "extra" button (a customizable button that can be used for different purposes
		// per-report).
		if(UseExtraButton()) {
			//TES 2/1/2010 - PLID 37143 - Don't need to resize, but we do need to set the text on the button.
			switch(m_CurrReport->nID) {
				case 658:
				case 567: //TES 7/27/2012 - PLID 51849 - Lab Results Form
					m_btnExtra.SetWindowText("Assign Defaults By Lab...");
					break;
				default:
					//TES 2/1/2010 - PLID 37143 - We don't support any other reports!
					ASSERT(FALSE);
					break;
			}
		}
		else {
			//TES 2/1/2010 - PLID 37143 - Hide the Close button, shrink the dialog, and rename the extra button as Close
			m_btnClose.ShowWindow(SW_HIDE);
			CRect rcExtra;
			GetDlgItem(IDC_REPORT_PICKER_PLACEHOLDER)->GetWindowRect(&rcExtra);
			CRect rcDialog;
			GetWindowRect(&rcDialog);
			rcDialog.bottom = rcExtra.bottom;
			MoveWindow(&rcDialog);
			m_btnExtra.AutoSet(NXB_CLOSE);
			m_btnExtra.SetWindowText("Close");
		}

		// (r.gonet 10/11/2011) - PLID 46437 - Show or hide the extra checkbox per report.
		if(UseExtraCheckbox()) {
			m_checkExtra.ShowWindow(SW_SHOW);
			switch(m_CurrReport->nID) {
				case 658:
					m_checkExtra.SetWindowText("Generate Barcode for Selected Custom Report.");
					break;
				default:
					// (r.gonet 10/11/2011) - PLID 46437 - We don't support any other reports!
					ASSERT(FALSE);
					break;
			}
		} else {
			m_checkExtra.ShowWindow(SW_HIDE);
		}

		m_ReportPicker = BindNxDataListCtrl(this, IDC_EDITREPORTLIST, GetRemoteData(), false);

		Refresh();

	}NxCatchAll("Error Initializing Report List");

	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEditReportPickerDlg::SetDefaultColor() {

	try {

		//check to see if there is a default report set already
		_RecordsetPtr rs = CreateRecordset("SELECT CustomReportID, DetailOption, DateOption FROM DefaultReportsT WHERE ID = %li", m_CurrReport->nID);
		IRowSettingsPtr pRow;

		if (rs->eof) {

			//there is no default report set yet, so we will set the report ourselves

			//We need to find the row that a.) is not custom, b.) has the same detail option, 
			//and c.) has the same date filter OR a null date filter.  If the date filter is NULL on
			//any non-custom report, it will be NULL on ALL non-custom reports.
			for(int i = 0; i < m_ReportPicker->GetRowCount(); i++) {
				if(VarLong(m_ReportPicker->GetValue(i, erpclNumber), -1) == -1) {
					if(VarLong(m_ReportPicker->GetValue(i, erpclDetailOption), 0) == m_CurrReport->nDetail) {
						if(VarLong(m_ReportPicker->GetValue(i, erpclDateOption), -1) == m_CurrReport->nDateFilter || VarLong(m_ReportPicker->GetValue(i, erpclDateOption), -1) == -1) {
							//OK, this is it.
							pRow = m_ReportPicker->GetRow(i);
							pRow->PutForeColor(RGB(255,0,0));
							//We know there's only one default, so let's skip to the end.
							i = m_ReportPicker->GetRowCount();
						}
					}
				}
			}
		}else {
			//they have already selected a default report

			//We need to find the datalist row that has the same value as the DefaultReports record for each field.
			for(int i = 0; i < m_ReportPicker->GetRowCount(); i++) {
				if(VarLong(m_ReportPicker->GetValue(i, erpclNumber), -1) == AdoFldLong(rs, "CustomReportID", -1)) {
					if(VarLong(m_ReportPicker->GetValue(i, erpclDetailOption), -1) == AdoFldLong(rs, "DetailOption", -1)) {
						if(VarLong(m_ReportPicker->GetValue(i, erpclDateOption), -1) == AdoFldLong(rs, "DateOption", -1)) {
							//All right, this is it.
							pRow = m_ReportPicker->GetRow(i);
							pRow->PutForeColor(RGB(255,0,0));
							//We know there's only one default, so let's skip to the end.
							i = m_ReportPicker->GetRowCount();
						}
					}
				}
			}
		}
	}NxCatchAll("Error Setting Color");
				

}

void CEditReportPickerDlg::OnCancel() 
{

	CNxDialog::OnOK();
	
}

void CEditReportPickerDlg::OnMakedefault() 
{

	try {

		//check to see if they have permission
		if (CheckCurrentUserPermissions(bioReportDesigner, sptWrite)) {
	
			//get the currently selected report
			long nCurSel = m_ReportPicker->GetCurSel();
			long nID = m_CurrReport->nID;
			CString strCustomReportID, strDetailOption, strDateOption, strAveryOption;
			strCustomReportID.Format("%li", VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1));
			strDetailOption.Format("%li", VarLong(m_ReportPicker->GetValue(nCurSel, erpclDetailOption), -1));
			strDateOption.Format("%li", VarLong(m_ReportPicker->GetValue(nCurSel, erpclDateOption), -1));
			strAveryOption.Format("%li", VarLong(m_ReportPicker->GetValue(nCurSel, erpclAveryStyle), -1));

			//DRT 12/30/2004 - PLID 15151 - If all of these are -1, they must be reverting to the default item... 
			//	so why not just do that instead of putting a row in the data with all NULL values (which causes problems later on)
			if(strCustomReportID == "-1" && strDetailOption == "-1" && strDateOption == "-1" && strAveryOption == "-1")
				OnRevert();

			if(strCustomReportID == "-1") strCustomReportID = "NULL";
			if(strDetailOption == "-1") strDetailOption = "NULL";
			if(strDateOption == "-1") strDateOption = "NULL";
			if(strAveryOption == "-1") strAveryOption = "NULL";

			//check to see if there is already a default report for this report
			if(!ReturnsRecords("SELECT ID FROM DefaultReportsT WHERE ID = %li", nID)) {
				//they don't have one yet, so insert one
				ExecuteSql("INSERT INTO DefaultReportsT (ID, CustomReportID, DetailOption, DateOption, AveryOption) VALUES "
				" (%li, %s, %s, %s, %s) ", nID, strCustomReportID, strDetailOption, strDateOption, strAveryOption);
			}
			else {
				//just update 
				ExecuteSql("Update DefaultReportsT SET CustomReportID = %s, DetailOption = %s, DateOption = %s, AveryOption = %s "
					"WHERE ID = %li", strCustomReportID, strDetailOption, strDateOption, strAveryOption, nID);
			}

			//PLID 17807 // check the statement in the new way because we can't rely on the filename anymore
			if (IsStatement(nID)) {
				CDWordArray dwAry;
				SetCommonFileArray(&dwAry, nID);

				for (int i = 0; i < dwAry.GetSize(); i++) {

					//don't include this ID because we already did it
					if (((long)dwAry.GetAt(i)) != nID) {

						//this part is just copied from below with minor changes
						//check to see if they want to make this report the default for this report as well
						CString str, strReportTitle;
						CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(((long)dwAry.GetAt(i)))]);
						str.Format("Would you like to make this report file the same for the %s report as well?", infReport.strPrintName);
						if (MsgBox(MB_YESNO, str) == IDYES) {
							CString strCustomNumber;
							if(VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1) != -1) {
								//Make sure there is an appropriate CustomReportsT record.
								long nCustomNumber;
								_RecordsetPtr rsCustomID = CreateRecordset("SELECT CustomReportsT.Number FROM CustomReportsT WHERE ID = %li AND FileName = (SELECT FileName FROM CustomReportsT CurrReport WHERE CurrReport.ID = %li AND CurrReport.Number = %li)", ((long)dwAry.GetAt(i)), m_CurrReport->nID, VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1));
								if(rsCustomID->eof) {
									CString strTableName;
									nCustomNumber = VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber));
									//strTableName.Format("CustomReportsT WHERE ID = %li", CReports::gcs_aryKnownReports[i].nID);
									//nCustomNumber = NewNumber(strTableName, "Number");
									ExecuteSql("INSERT INTO CustomReportsT (ID, Number, Title, FileName, Version) "
										"SELECT %li, %li, Title, FileName, Version FROM CustomReportsT CurrReport WHERE CurrReport.ID = %li AND CurrReport.Number = %li",
										((long)dwAry.GetAt(i)), nCustomNumber, m_CurrReport->nID, VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1));
								}
								else {
									nCustomNumber = AdoFldLong(rsCustomID, "Number");
								}
								strCustomNumber.Format("%li", nCustomNumber);
							}
							else {
								strCustomNumber = "NULL";
							}
								

							//check to see if there is already a default report for this report
							if(!ReturnsRecords("SELECT ID FROM DefaultReportsT WHERE ID = %li", ((long)dwAry.GetAt(i)))) {
								
								//they don't have one yet, so insert one
								ExecuteSql("INSERT INTO DefaultReportsT (ID, CustomReportID, DetailOption, DateOption, AveryOption) VALUES "
								" (%li, %s, %s, %s, %s) ", ((long)dwAry.GetAt(i)), strCustomNumber, strDetailOption, strDateOption, strAveryOption);

							}
							else {

								//just update 
								ExecuteSql("Update DefaultReportsT SET CustomReportID = %s, DetailOption = %s, DateOption = %s, AveryOption = %s "
									"WHERE ID = %li", strCustomNumber, strDetailOption, strDateOption, strAveryOption, ((long)dwAry.GetAt(i)));
							}
						}
					}
				}

			}
			else {
				//loop through the reports to see if there are any other reports with that report name
				for (long i=0; i< CReports::gcs_nKnownReportCount; i++) {
					
					if (CReports::gcs_aryKnownReports[i].strReportFile.CompareNoCase(m_CurrReport->strReportFile) == 0 && CReports::gcs_aryKnownReports[i].nID != nID) {

						//check to see if they want to make this report the default for this report as well
						CString str;
						str.Format("Would you like to make this report file the same for the %s report as well?", CReports::gcs_aryKnownReports[i].strPrintName);
						if (MsgBox(MB_YESNO, str, "NexTech") == IDYES) {
							CString strCustomNumber;
							if(VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1) != -1) {
								//Make sure there is an appropriate CustomReportsT record.
								long nCustomNumber;
								_RecordsetPtr rsCustomID = CreateRecordset("SELECT CustomReportsT.Number FROM CustomReportsT WHERE ID = %li AND FileName = (SELECT FileName FROM CustomReportsT CurrReport WHERE CurrReport.ID = %li AND CurrReport.Number = %li)", CReports::gcs_aryKnownReports[i].nID, m_CurrReport->nID, VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1));
								if(rsCustomID->eof) {
									CString strTableName;
									nCustomNumber = VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber));
									//strTableName.Format("CustomReportsT WHERE ID = %li", CReports::gcs_aryKnownReports[i].nID);
									//nCustomNumber = NewNumber(strTableName, "Number");
									ExecuteSql("INSERT INTO CustomReportsT (ID, Number, Title, FileName, Version) "
										"SELECT %li, %li, Title, FileName, Version FROM CustomReportsT CurrReport WHERE CurrReport.ID = %li AND CurrReport.Number = %li",
										CReports::gcs_aryKnownReports[i].nID, nCustomNumber, m_CurrReport->nID, VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1));
								}
								else {
									nCustomNumber = AdoFldLong(rsCustomID, "Number");
								}
								strCustomNumber.Format("%li", nCustomNumber);
							}
							else {
								strCustomNumber = "NULL";
							}
								

							//check to see if there is already a default report for this report
							if(!ReturnsRecords("SELECT ID FROM DefaultReportsT WHERE ID = %li", CReports::gcs_aryKnownReports[i].nID)) {
								
								//they don't have one yet, so insert one
								ExecuteSql("INSERT INTO DefaultReportsT (ID, CustomReportID, DetailOption, DateOption, AveryOption) VALUES "
								" (%li, %s, %s, %s, %s) ", CReports::gcs_aryKnownReports[i].nID, strCustomNumber, strDetailOption, strDateOption, strAveryOption);

							}
							else {

								//just update 
								ExecuteSql("Update DefaultReportsT SET CustomReportID = %s, DetailOption = %s, DateOption = %s, AveryOption = %s "
									"WHERE ID = %li", strCustomNumber, strDetailOption, strDateOption, strAveryOption, CReports::gcs_aryKnownReports[i].nID);
							}
						}
					}
				}
			}


			//PLID 15823 - If it is a statement, and they don't already have it set to default,
			//ask if they want to change the format to default
			//if it is a statement, undo the default report setting as well
			long nStyle = GetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");
			if (nStyle != 2) {
				if (IsStatement(m_CurrReport->nID)) {

					long nResult = MsgBox(MB_YESNO, "Would you like to make your statement format the custom report? Click Yes to set the statement format to be the default custom report, No to not change the setting. You can always change the setting yourself in the Patient Statement Configuration dialog ");

					if (nResult == IDYES) {

						SetRemotePropertyInt("SttmntEnvelope", 2, 0, "<None>");
					}
					else {
						//don't change it
					}
				}
			}

			

			//change the colors
			IRowSettingsPtr pRow;
			for(int i = 0; i < m_ReportPicker->GetRowCount(); i++) {
				pRow = m_ReportPicker->GetRow(i);
				if(i == nCurSel) pRow->PutForeColor(RGB(255,0,0));
				else pRow->PutForeColor(dlColorNotSet);
			}

			//set the default in the report
			if(strCustomReportID != "NULL") {
				m_CurrReport->nDefaultCustomReport = atoi(strCustomReportID);
			}
			else {
				//set it to -1 so that the filters we be set correctly
				m_CurrReport->nDefaultCustomReport = -1;
			}			
			if(strDetailOption != "NULL")
				m_CurrReport->nDetail = atoi(strDetailOption);
			if(strDateOption != "NULL")
				m_CurrReport->nDateFilter = atoi(strDateOption);

			//NOTE:  You'll notice I'm not doing anything here about the Avery option.  This is because,
			//for statements, the Make Default button should be disabled on non-custom reports, so it should never be an issue.

		}
			

	}NxCatchAll("Error in Making Default");
}


int CEditReportPickerDlg::DoModal() {

	return CNxDialog::DoModal();

}
int CEditReportPickerDlg::DoModal(long nID) 
{
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nID)]);
	m_CurrReport = &infReport;	
	
	return CNxDialog::DoModal();
}

void CEditReportPickerDlg::OnNewreport() 
{

	try {

		//check to see if they have permission
		if (CheckCurrentUserPermissions(bioReportDesigner, sptCreate)) {

			//get the number out of the datalist
			if (m_ReportPicker->CurSel == -1) {
				//check to see if there is only one thing in the list
				if (m_ReportPicker->GetRowCount() == 1) {
					//highlight that report and keep going
					m_ReportPicker->PutCurSel(0);
				}
				else {
					MsgBox("Please select a report to base the new report on");
					return;
				}
			}
			long nCustomReportID = VarLong(m_ReportPicker->GetValue(m_ReportPicker->CurSel, erpclNumber), -1);
			bool bIsCustom = false;
			CString strOpenName;


			bIsCustom = nCustomReportID != -1;
			GetReportFileName(m_ReportPicker->CurSel, strOpenName);

			//we aren't allowing them to save using the same filename as the original report, so we have to 
			//change it
			long nResult;
			CString strSaveName;
			nResult = strOpenName.Find("Custom");
			if (nResult == -1) {
				strSaveName = "Custom"+ strOpenName;
			}
			else {
				strSaveName = strOpenName;
			}

			//make sure that is doesn't exist
			//this should always not exist
			long nCount = m_ReportPicker->GetRowCount() + 1;
			while (DoesExist(GetCustomReportsPath() ^ strSaveName)) {

				//append a number to the end
				strSaveName = strSaveName.Left(strSaveName.GetLength() - 4) + AsStringForSql(nCount) + ".rpt";
				nCount++;
			}

			//ok, we got here, so we have a valid report path
			
			//CNxDialog::OnOK();
			CEditReportDlg  dlg(this, m_CurrReport, strOpenName, strSaveName, bIsCustom, nCustomReportID, 1);
			dlg.DoModal();
			if(dlg.m_bSaved) {
				Refresh();
				m_ReportPicker->SetSelByColumn(erpclNumber, dlg.m_nCustomReportID);
				OnSelChangedEditreportlist(m_ReportPicker->CurSel);
				if(m_ReportPicker->CurSel == -1) {
					//???
					ASSERT(FALSE);
				}
				else {
					if(IDYES == MsgBox(MB_YESNO, "Would you like to make this new report the Default?")) {
						OnMakedefault();
					}
				}
			}
			else {
				long nCurSel = m_ReportPicker->CurSel;
				Refresh();
				m_ReportPicker->CurSel = nCurSel;
				OnSelChangedEditreportlist(m_ReportPicker->CurSel);
			}

		}
	}NxCatchAll("Error Opening Edit Dialog");

	
	
}

void CEditReportPickerDlg::OnRevert() 
{

	try {

		long nCurSel = m_ReportPicker->GetCurSel();

		//long nNumber = VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber));

		//figure out which one is currently the default
		
		if (!(ReturnsRecords("SELECT ID FROM DefaultReportsT WHERE ID = %li", m_CurrReport->nID))) {
			//there is currently no default report
			//TS 12/23/02:  Why do we need to notify them?  If there's no default report, then it 
			//is by definition using the system default, so they've got what they want.
			//MsgBox("There is no default report for this report");
		}
		else {

			ExecuteSql("DELETE FROM DefaultReportsT WHERE ID = %li", m_CurrReport->nID);

			//take off the color

			//TES 11/5/2007 - PLID 27978 - VS 2008 - for() loops
			int i = 0;
			for(i = 0; i < m_ReportPicker->GetRowCount(); i++) {
				IRowSettingsPtr pRow = m_ReportPicker->GetRow(i);
				pRow->PutForeColor(dlColorNotSet);
			}

			m_CurrReport->nDefaultCustomReport = -1;
			//Now we need to actually modify m_CurrReport.  We're reverting it to it's default, meaning
			//that we have to find what we instantiated it from in the array of reports.
			for (i = 0; i < CReports::gcs_nKnownReportCount; i++) {
				if(CReports::gcs_aryKnownReports[i].nID == m_CurrReport->nID) {
					//OK, this is the one.
					m_CurrReport->nDetail = CReports::gcs_aryKnownReports[i].nDetail;
					m_CurrReport->nDateFilter = CReports::gcs_aryKnownReports[i].nDateFilter;
					//We're done.
					i = CReports::gcs_nKnownReportCount;
				}
			}

			//set the system default 
			SetDefaultColor();
			
			//check to see whether they want to change the other reports with the same report name to this report as well
			
			//loop through the reports to see if there are any other reports with that report name
			//PLID 17807
			if (IsStatement(m_CurrReport->nID)) {
				CDWordArray dwAry;
				SetCommonFileArray(&dwAry, m_CurrReport->nID);

				for (int i = 0; i < dwAry.GetSize(); i++) {
					if (((long)dwAry.GetAt(i)) != m_CurrReport->nID) {

						//copied from below
						//check to see if they want to make this report the default for this report as well
						CString str;
						CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(((long)dwAry.GetAt(i)))]);
						str.Format("Would you like to revert the %s report as well?", infReport.strPrintName);
						if (MsgBox(MB_YESNO, str, "NexTech") == IDYES) {

							//check to see if there is already a default report for this report
							ExecuteSql("DELETE FROM DefaultReportsT WHERE ID = %li", ((long)dwAry.GetAt(i)));
				
							
						}
					}
				}

			}
			else {

				for (i=0; i< CReports::gcs_nKnownReportCount; i++) {
					
					if (CReports::gcs_aryKnownReports[i].strReportFile.CompareNoCase(m_CurrReport->strReportFile) == 0 && CReports::gcs_aryKnownReports[i].nID != m_CurrReport->nID) {

						//check to see if they want to make this report the default for this report as well
						CString str;
						str.Format("Would you like to revert the %s report as well?", CReports::gcs_aryKnownReports[i].strPrintName);
						if (MsgBox(MB_YESNO, str, "NexTech") == IDYES) {

							//check to see if there is already a default report for this report
							ExecuteSql("DELETE FROM DefaultReportsT WHERE ID = %li", CReports::gcs_aryKnownReports[i].nID);
				
							
						}
					}
				}
			}


			//if it is a statement, undo the default report setting as well
			if (IsStatement(m_CurrReport->nID)) {

				long nResult = MsgBox(MB_YESNOCANCEL, "Would you like to make your default statement the CO165?  Click Yes to default to CO165 format, No to default to CO158 format or cancel to set the setting yourself in the Patient Statement Configuration dialog");

				if (nResult == IDYES) {

					SetRemotePropertyInt("SttmntEnvelope", 0, 0, "<None>");
				}
				else if (nResult == IDNO) {

					SetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");
				}
			}
				
		}


	}NxCatchAll("Error reverting to system");	
}

void CEditReportPickerDlg::AddStandardRecords(long nDateOption, CString strDateName)
{
	_variant_t varNull;
	varNull.vt = VT_NULL;
	IRowSettingsPtr pRow;

	//OK, first, is there a detail option?
	if(m_CurrReport->nDetail) {
		//Next, is there an avery option?  (Is this a statement?)
		if(IsStatement(m_CurrReport->nID)) {
			
			pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, varNull);
			pRow->PutValue(erpclDetailOption, (long)1);
			pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
			pRow->PutValue(erpclAveryStyle, (long)1);
			pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + " Detailed CO165 Format" + strDateName));
			m_ReportPicker->AddRow(pRow);

			pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, varNull);
			pRow->PutValue(erpclDetailOption, (long)1);
			pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
			pRow->PutValue(erpclAveryStyle, (long)2);
			pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + " Detailed CO158 Format" + strDateName));
			m_ReportPicker->AddRow(pRow);

			pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, varNull);
			pRow->PutValue(erpclDetailOption, (long)2);
			pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
			pRow->PutValue(erpclAveryStyle, (long)1);
			pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + " Summary CO165 Format" + strDateName));
			m_ReportPicker->AddRow(pRow);

			pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, varNull);
			pRow->PutValue(erpclDetailOption, (long)2);
			pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
			pRow->PutValue(erpclAveryStyle, (long)2);
			pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + " Summary CO158 Format" + strDateName));
			m_ReportPicker->AddRow(pRow);	

		}

		else {
			//OK, just detail and summary.
			pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, varNull);
			pRow->PutValue(erpclDetailOption, (long)1);
			pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
			pRow->PutValue(erpclAveryStyle, varNull);
			pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + " Detailed" + strDateName));
			m_ReportPicker->AddRow(pRow);

			pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, varNull);
			pRow->PutValue(erpclDetailOption, (long)2);
			pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
			pRow->PutValue(erpclAveryStyle, varNull);
			pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + " Summary" + strDateName));
			m_ReportPicker->AddRow(pRow);
		}
	}
	else {
		//TODO:  Allow for Avery options w/o detail options.  This doesn't exist now, but it's possible.

		//OK, there's only one option, so...
		pRow = m_ReportPicker->GetRow(-1);
		pRow->PutValue(erpclNumber, varNull);
		pRow->PutValue(erpclDetailOption, varNull);
		pRow->PutValue(erpclDateOption, nDateOption == -1 ? varNull : nDateOption);
		pRow->PutValue(erpclAveryStyle, varNull);
		pRow->PutValue(erpclName, (_variant_t) (m_CurrReport->strPrintName + strDateName));
		m_ReportPicker->AddRow(pRow);
	}

	//OK, we've added all the options for this date option.
}

void CEditReportPickerDlg::OnDelete() 
{
	try {
		long nCurSel = m_ReportPicker->CurSel;
		
		//make sure that an item was picked
		if (nCurSel == -1) {

			MsgBox("Please select a custom report to delete");
			return;
		}

		long nID, nNumber;
		nID = m_CurrReport->nID;
		nNumber = VarLong(m_ReportPicker->GetValue(nCurSel, erpclNumber), -1);

		if (nNumber == -1) {

			MsgBox("You may only delete custom reports, not system reports");
			return;
		}
		

		//warn them and make sure that they want to do this
		if (IDYES == MsgBox(MB_YESNO, "This action is unrecoverable!! Are you sure you want to delete this custom report?")) {

			//check to see if it is the default report
			_RecordsetPtr rsDuplicate = CreateRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = %li", nID);

			if (! rsDuplicate->eof) {

				//check to see if the number is the one we are trying to delete
				long nDefaultNumber = AdoFldLong(rsDuplicate, "CustomReportID", -1);

				if(nDefaultNumber == -1) {
					//DRT 12/30/2004 - PLID 15151 - There is no default custom report.  This could happen previously (due to a bug)
					//	if you selected the original report and clicked 'Make Default'.  To handle this possible bad data, we will
					//	just call revert to default now, and then this code will work fine.
					OnRevert();
				}
				else if (nDefaultNumber == nNumber) {

						//revert it to the system default
						OnRevert();
				}
			}
			

			//get the filename
			CString strFileName;
			GetReportFileName(nCurSel, strFileName);

			//its not the default report so we are A-OK to proceed
			//TES 2/1/2010 - PLID 37143 - Need to clear out any per-location defaults that use this custom report.
			ExecuteSql("DELETE FROM LocationCustomReportsT WHERE EXISTS (SELECT ID FROM CustomReportsT WHERE FileName = '%s' AND "
				"CustomReportsT.ID = LocationCustomReportsT.ReportID AND CustomReportsT.Number = LocationCustomReportsT.ReportNumber)",
				_Q(strFileName));
			// (r.gonet 10/16/2011) - PLID 45968 - If this is a lab request form, then we may have some custom fields that reference this custom report
			if(m_CurrReport->nID == 658) {
				ExecuteParamSql("DELETE FROM LabReqCustomFieldsT WHERE CustomReportNumber = {INT}; ", nNumber);
			}
			//TES 12/10/2003: We want to delete all references to this filename.
			ExecuteSql("DELETE FROM CustomReportsT WHERE FileName = '%s'", _Q(strFileName));
			
			//here goes nothing!!
			DeleteFile(GetCustomReportsPath() ^ strFileName);

			// (c.haag 2007-02-20 11:00) - PLID 24779 - There was a bug where if you had three custom reports,
			// and you deleted the first one, then tried to delete the last one, an EOF error would occur. This
			// is because the report numbers are not updated in the datalist. While we could just do that right
			// here, refreshing the list outright will allow us to make neighboring code more maintainable.
			Refresh();

			/*
			//remove the row
			m_ReportPicker->RemoveRow(nCurSel);

			//DRT 12/30/2004 - PLID 15150 - We need to set the selection to something ourself, otherwise we
			//	can get in an invalid state.
			m_ReportPicker->PutCurSel(-1);
			OnSelChangedEditreportlist(-1);*/
		}
		
	}NxCatchAll("Error deleting custom report");
	
}

void CEditReportPickerDlg::Refresh()
{
	try {
		m_ReportPicker->Clear();

		IRowSettingsPtr pRow;
		_variant_t varNull;
		varNull.vt = VT_NULL;

		//TS 12/23/02:  OK, here's the logic.  We go through each date option, 
		//and for each one we also go through each detail and avery option as appropriate.
		if(m_CurrReport->strDateOptions != "") {
			long nIndex = 0;
			long nRecordNum = 0;
			long nOptionID = -1;
			CString strDisplaySuffix;
			while(nIndex < m_CurrReport->strDateOptions.GetLength() ) {
				nRecordNum++;
				if(nRecordNum == 2) {//We have two options, let's output the first one, see comment below
					AddStandardRecords(nOptionID, strDisplaySuffix);
				}
				//Load a record into the datalist.
				//First, the id.
				long nSemicolon = m_CurrReport->strDateOptions.Find(";", nIndex);
				nOptionID = (long)atoi(m_CurrReport->strDateOptions.Mid(nIndex, nSemicolon-nIndex));
				nIndex = nSemicolon+1;

				//Now, the display name
				nSemicolon = m_CurrReport->strDateOptions.Find(";", nIndex);
				strDisplaySuffix = " by " + m_CurrReport->strDateOptions.Mid(nIndex, nSemicolon-nIndex);
				nIndex = nSemicolon+1;

				//Now, the field name
				nSemicolon = m_CurrReport->strDateOptions.Find(";", nIndex);
				nIndex = nSemicolon+1;

				//Finally the report suffix
				nSemicolon = m_CurrReport->strDateOptions.Find(";", nIndex);
				nIndex = nSemicolon+1;

				//All right, let's go through and add each option for this name.
				//However, if we're on the first record, we might be on the only record, so we will add 
				//it at the beginning of the second iteration of this loop (see above).
				if(nRecordNum > 1) {
					AddStandardRecords(nOptionID, strDisplaySuffix);
				}
			}
			if(nRecordNum == 1) {
				//There was only one, so there's only the one option, and it doesn't need a name.
				AddStandardRecords(-1, "");
			}
		}
		else {
			//There's no options, so don't worry about it.
			AddStandardRecords(-1, "");
		}
		

		//now put all the Custom Reports that they already have for this report in there
		_RecordsetPtr rs = CreateParamRecordset("SELECT Title, Number, GenerateBarcode FROM CustomReportsT WHERE ID = {INT}", m_CurrReport->nID);
		CString strName;
		long nNumber;
		BOOL bGenerateBarcode;
		while (! rs->eof) {
			strName = AdoFldString(rs, "Title");
			nNumber = AdoFldLong(rs, "Number");
			bGenerateBarcode = AdoFldBool(rs, "GenerateBarcode", FALSE);

			IRowSettingsPtr  pRow = m_ReportPicker->GetRow(-1);
			pRow->PutValue(erpclNumber, nNumber);
			pRow->PutValue(erpclDetailOption, varNull);
			pRow->PutValue(erpclDateOption, varNull);
			pRow->PutValue(erpclAveryStyle, varNull);
			pRow->PutValue(erpclName, (_variant_t) strName);
			pRow->PutValue(erpclGenerateBarcode, _variant_t(bGenerateBarcode ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
			m_ReportPicker->AddRow(pRow);

			rs->MoveNext();

		}

		OnSelChangedEditreportlist(-1);
		SetDefaultColor();
	}NxCatchAll("Error in CEditReportPickerDlg::Refresh()");
}

bool CEditReportPickerDlg::UseExtraButton()
{
	//TES 2/1/2010 - PLID 37143 - Put any reports that we want to have a special button for in this function.
	switch(m_CurrReport->nID) {
		case 658: //Lab Request Form
		case 567: //TES 7/27/2012 - PLID 51849 - Lab Results Form
			return true;
		default:
			return false;
	}
}
void CEditReportPickerDlg::OnExtraBtn()
{
	try {
		if(!UseExtraButton()) {
			//TES 2/1/2010 - PLID 37143 - We're not using the "Extra" button, so the user thinks this is the "Close" button, treat accordingly.
			OnCancel();
		}
		else {
			//TES 2/1/2010 - PLID 37143 - Call the appropriate handling based on the report.
			switch(m_CurrReport->nID) {
			case 658:
				{
					CCustomReportsByLocationDlg dlg(this);
					dlg.m_nReportID = 658;
					dlg.DoModal();
				}
				break;
			case 567: 
				{
					//TES 7/27/2012 - PLID 51849 - Added support for the Lab Results Form
					CCustomReportsByLocationDlg dlg(this);
					dlg.m_nReportID = 567;
					dlg.DoModal();
				}
				break;
			default:
				//We shouldn't have gotten here if UseExtraButton() was true.
				ASSERT(FALSE);
				break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

bool CEditReportPickerDlg::UseExtraCheckbox()
{
	// (r.gonet 10/11/2011) - PLID 46437 - Put any reports that we want to have a special checkbox for in this function.
	switch(m_CurrReport->nID) {
		case 658: //Lab Request Form
			return true;
		default:
			return false;
	}
}

void CEditReportPickerDlg::OnExtraCheckbox()
{
	try {
		if(UseExtraCheckbox()) {
			// (r.gonet 10/11/2011) - PLID 46437 - Call the appropriate handling based on the report.
			switch(m_CurrReport->nID) {
			case 658:
				{
					long nRow = m_ReportPicker->CurSel;
					if(nRow >= 0) {
						long nCustomReportID = VarLong(m_ReportPicker->GetValue(nRow, erpclNumber), -1);
						BOOL bGenerateBarcode = m_checkExtra.GetCheck() ? TRUE : FALSE;
						if(nCustomReportID >= 0) {
							ExecuteParamSql(
								"UPDATE CustomReportsT SET GenerateBarcode = {BIT} WHERE ID = 658 AND Number = {INT}; ",
								bGenerateBarcode, nCustomReportID);
						}
						m_ReportPicker->PutValue(nRow, erpclGenerateBarcode, _variant_t(bGenerateBarcode ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
					}
				}
				break;
			default:
				//We shouldn't have gotten here if UseExtraCheckbox() was true.
				ASSERT(FALSE);
				break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/16/2011) - PLID 45968 - Added the ability to have custom menu commands per report type.
void CEditReportPickerDlg::RButtonUpEditreportlist(long nRow, short nCol, long x, long y, long nFlags)
{
	try {
		bool bShowPopup = false;
		CMenu mPopup;
		mPopup.CreatePopupMenu();

		switch(m_CurrReport->nID) {
			case 658:
				{
					// (r.gonet 10/16/2011) - PLID 45968 - Only allow custom lab requests
					if(nRow != -1 && VarLong(m_ReportPicker->GetValue(nRow, erpclNumber), -1) > 0) {
						m_ReportPicker->CurSel = nRow;
						// (r.gonet 10/16/2011) - PLID 45968 - Give the user the ability to configure which lab custom fields are available on the current custom report.
						mPopup.InsertMenu(0, MF_BYPOSITION, ID_EDIT_CUSTOM_REPORT_FIELDS, "&Edit Custom Report Fields");
						bShowPopup = true;
					}
				}
				break;
			default:
				break;
		}
		if(bShowPopup) {
			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/16/2011) - PLID 45968 - Open up the lab request custom field editor dialog for the currently selected custom report.
void CEditReportPickerDlg::OnEditCustomReportFields()
{
	try {
		if(m_CurrReport->nID != 658) {
			// (r.gonet 10/16/2011) - PLID 45968 - Only for lab requests
			return;
		}
	
		long nRow = m_ReportPicker->CurSel;
		if(nRow >= 0) {
			long nCustomReportID = VarLong(m_ReportPicker->GetValue(nRow, erpclNumber), -1);
			if(nCustomReportID > 0) {
				// (r.gonet 10/16/2011) - PLID 45968 - Only for custom lab requests
				CLabReqCustomFieldsDlg dlg(nCustomReportID, this);
				dlg.DoModal();
			}
		}
	} NxCatchAll(__FUNCTION__);
}