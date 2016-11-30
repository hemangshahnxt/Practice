// EditReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "reports.h"
#include "EditReportDlg.h"
#include "GlobalReportUtils.h"
#include "SaveReportDlg.h"
#include "GlobalUtils.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CEditReportDlg dialog


extern bool EnsureDEPWithOldATLThunkEmulation();

// (a.walling 2014-05-13 14:12) - PLID 62098 - VS2013 - Warn once with a message box, and log. Still allow it to continue, but at least it will be obvious what the problem is.
static void CheckOldATLThunkSupport()
{
	static bool bChecked = false;
	if (bChecked) {
		return;
	}
	bChecked = true;

	auto atlThunksOkay = EnsureDEPWithOldATLThunkEmulation();
	if (atlThunksOkay) {
		return;
	}
	
	auto message = 
		"Practice has detected a compatibility issue with your system that may prevent you from being able to edit reports. "
		"Please contact technical support for more information."
	;

	Log(message);
	AfxMessageBox(message);
}

CEditReportDlg::CEditReportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditReportDlg::IDD, pParent)
	, m_InstructDlg(pParent)
{
	//{{AFX_DATA_INIT(CEditReportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (a.walling 2014-05-13 14:12) - PLID 62098 - VS2013 - Check ATL thunk support and warn once with a message box and log.
	CheckOldATLThunkSupport();
}

CEditReportDlg::CEditReportDlg(CWnd* pParent, CReportInfo *CurrReport)
	: CNxDialog(CEditReportDlg::IDD, pParent)
	, m_InstructDlg(pParent)
{

	m_CurrReport = CurrReport;
//	m_Application = NULL;
	//m_Report = NULL;

	// (a.walling 2014-05-13 14:12) - PLID 62098 - VS2013 - Check ATL thunk support and warn once with a message box and log.
	CheckOldATLThunkSupport();
}


CEditReportDlg::CEditReportDlg(CWnd* pParent, CReportInfo *CurrReport, CString strOpenFileName,  CString strSaveFileName, bool bIsCustom, long nNumber, bool bIsNew)
	: CNxDialog(CEditReportDlg::IDD, pParent)
	, m_InstructDlg(pParent)
{

	m_CurrReport = CurrReport;
	m_strOpenFileName = strOpenFileName;
	m_strSaveFileName = strSaveFileName;
	m_bIsCustom = bIsCustom;
	m_nCustomReportID = nNumber;
	m_bIsNew = bIsNew;
	m_bSaved = false;

	// (a.walling 2014-05-13 14:12) - PLID 62098 - VS2013 - Check ATL thunk support and warn once with a message box and log.
	CheckOldATLThunkSupport();
}	

void CEditReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditReportDlg)
	DDX_Control(pDX, IDC_SAVEREPORT, m_btnSave);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditReportDlg, CNxDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SAVEREPORT, OnSavereport)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_VERIFYREPORT, OnVerifyreport)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditReportDlg message handlers

BOOL CEditReportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try { 

		// (z.manning, 04/28/2008) - PLID 29807 - Set button styles
		m_btnSave.SetIcon(IDI_SAVE);

		//before you do anything, make sure that the file we are trying to open exists
		if (m_bIsCustom) {
			if (!DoesExist(GetCustomReportsPath() ^ m_strOpenFileName)) {

				MsgBox("There is a error with the report you are trying to open.  Please select another report to edit");
				CNxDialog::OnOK();
				return FALSE;
			}
		}
		else {

			//check and make sure that we have the system report as well, we always should, but just in case
			if (!DoesExist(GetReportsPath() ^ m_strOpenFileName)) {

				MsgBox("There is a error with the report you are trying to open.  Please select another report to edit");
				CNxDialog::OnOK();
				return FALSE;
			}
		}


		//set the current directory
		SetCurrentDirectory(GetCustomReportsPath());
		
		//Initialize the size stuff
		CRect rcDialog;
		GetWindowRect(&rcDialog);
		ScreenToClient(&rcDialog);


		
		CWnd *pWnd  = NULL;
		CWnd *pWnd2 = NULL;
		pWnd = GetDlgItem(IDC_SAVEREPORT);
		pWnd2 = GetDlgItem(IDC_VERIFYREPORT);
		if (pWnd && pWnd2) {
			CRect rc;
			CRect rc2;
			pWnd->GetWindowRect(&rc);
			pWnd2->GetWindowRect(&rc2);
			ScreenToClient(&rc);
			ScreenToClient(&rc2);
			m_ptSaveBtn.x = ((rcDialog.Width()/2) - (((rc.Width() * 2) + 10) / 2));
			m_ptSaveBtn.y = rcDialog.bottom - rc.top;

			m_ptVerifyBtn.x = ((rcDialog.Width()/2) + (((rc.Width() * 2) + 10) / 2));
			m_ptVerifyBtn.y = rcDialog.bottom - rc.top;
		}


		pWnd = GetDlgItem(IDC_EMBEDDESIGN);
		
		if (pWnd) {
			// Remember where the top of the datalist is
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_nTop = rc.top;
			m_nBottom = rcDialog.bottom - rc.bottom;
		}

		// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
		this->SetWindowPos(NULL, 0, 0, 994, 738, 0);
		this->ShowWindow(SW_MAXIMIZE);	

		
		m_designer = pWnd->GetControlUnknown();
		
		// (a.walling 2011-07-28 14:22) - PLID 44787 - Newer crystal runtimes set themselves as the CurVer but provide a non-backwards compatible interface.
		// So, request the 8.5 version, and fallback to version independent if that fails.
		m_Application.CreateInstance("CrystalDesignRuntime.Application.8.5");
		if (!m_Application) {
			m_Application.CreateInstance("CrystalDesignRuntime.Application");
		}

		if (!m_Application) {
			AfxThrowNxException("CEditReportDlg - Could not load instance of Crystal app");
		}

		CString strOpenFileName;
		if (m_bIsCustom) {

			strOpenFileName = GetCustomReportsPath() ^ m_strOpenFileName;
		}
		else {
			strOpenFileName = GetReportsPath() ^ m_strOpenFileName;
		}
		
		m_Report = m_Application->OpenReport((LPCTSTR)strOpenFileName);
		m_designer->PutReportObject(m_Report);


		//if it is a custom report, then we need to print its name, otherwise, print the report name
		CString strTitle;
		if ((m_bIsCustom && !m_bIsNew) || m_bSaved) {
			//We know that they have a customreportid by now.
			_RecordsetPtr rsTitle = CreateRecordset("SELECT Title FROM CustomReportsT WHERE ID = %li AND Number = %li", m_CurrReport->nID, m_nCustomReportID);

			//this should never happen, but just in case
			if (rsTitle->eof) {
				strTitle = m_CurrReport->strPrintName;
			}
			else {
				strTitle = AdoFldString(rsTitle, "Title");
			}
		}
		else {

			strTitle = m_CurrReport->strPrintName;
		}
		
		SetWindowText(strTitle);

	}NxCatchAll("Error Opening Report");
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditReportDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

long CEditReportDlg::SaveReport() {

	try {
		//popup the save dialog set to the userreport directory as the default

		//get the title so that they won't have to put it in again
		CString strTitle;
		if ((m_bIsCustom && !m_bIsNew) || m_bSaved) {
			//We know that they have a customreportid by now.
			_RecordsetPtr rsTitle = CreateRecordset("SELECT Title FROM CustomReportsT WHERE ID = %li AND Number = %li", m_CurrReport->nID, m_nCustomReportID);

			//this should never happen, but just in case
			if (rsTitle->eof) {
				strTitle = "Enter Report Name";
			}
			else {
				strTitle = AdoFldString(rsTitle, "Title");
			}
		}
		else {

			strTitle = "Enter Report Name";
		}


		CSaveReportDlg dlg(this, strTitle, m_CurrReport->nID, m_bIsCustom, m_bIsNew);
		long nResult = dlg.DoModal();

		if (nResult == 1)  {
			CString strSavePathFile;

			strSavePathFile = GetCustomReportsPath() ^ m_strSaveFileName;
			
			//ok, because Crystal is stupid and won't let us save anything we have open, 
			//if it is a custom report, we have to save it as a differnet name and then rename it after the dialog is closed

			if (m_bIsCustom && !m_bIsNew) {

				//append a "tmp to the end of the name
				strSavePathFile += "tmp";
			}

			
			HRESULT hr;
			hr = m_designer->SaveReport((LPCTSTR)strSavePathFile);
			

			if (m_bIsNew) {


				//PLID 17807 - create the custom report records for all the reports with this report file
				CDWordArray dwAry;

							
				//check to see whether they want to change the other reports with the same report name to this report as well

				CString strList = "(";
				CString strTemp;				
				//loop through to get the list of reports to change
				//at the moment, the special case is just for statements, but once its implemented for all reports
				// we'll be able to take out the loop and just have like an if (IsGroupedReport) where it'll return
				// true if the report has a preview report associated with it. ie: daily schedule, receipts
				if (IsStatement(m_CurrReport->nID)) {

					//set up the array with reports that have the same filename
					SetCommonFileArray(&dwAry, m_CurrReport->nID);

				}
				else {

					for (long i=0; i < CReports::gcs_nKnownReportCount; i++) {

						if (CReports::gcs_aryKnownReports[i].strReportFile.CompareNoCase(m_CurrReport->strReportFile) == 0 && CReports::gcs_aryKnownReports[i].nID != m_CurrReport->nID) {
							strTemp.Format("%li,", CReports::gcs_aryKnownReports[i].nID);
							strList += strTemp;
						}
					}

					//add this ID
					strTemp.Format("%li", m_CurrReport->nID);
					strList += strTemp;
					strList += ")";
				}

		


				CString strTableName;
				long nNumber;
				if (IsStatement(m_CurrReport->nID)) {
					//just in case, initialize them
					strTemp = "";
					strList = "(";
					for (int i = 0; i < dwAry.GetSize(); i++) {
						strTemp.Format("%li,", ((long)dwAry.GetAt(i)));
						strList += strTemp;
					}
					//remove the last comma
					strList = strList.Left(strList.GetLength() - 1);
					//we already included the current id, so just close it
					strList += ")";

					
				}
				
				strTableName.Format("CustomReportsT WHERE ID IN %s", strList);
				nNumber = NewNumber(strTableName, "Number");
				

				//for right now, we'll just change it for the statement, since that is the only thing that needs it atm,
				// but I'll write it to work for any report, once we decide to implement it
				if (IsStatement(m_CurrReport->nID)) {

					for (int j = 0; j < dwAry.GetSize(); j++) {
						//check to make sure it doesn't already exist, although it never should
						if (ReturnsRecords("SELECT ID, Number FROM CustomReportsT WHERE ID = %li AND Number = %li", ((long)dwAry.GetAt(j)), nNumber)) {
							ASSERT(FALSE);
						}
						else {
							//now that we have the file saved to the hard drive, let's save the data
							ExecuteSql("INSERT INTO CustomReportsT (ID, Number, Title, FileName, Version) VALUES "
								" (%li, %li, '%s', '%s', %li)", ((long)dwAry.GetAt(j)), nNumber, _Q(dlg.m_strTitle), _Q(m_strSaveFileName), m_CurrReport->nVersion);
						}
					}
				}
				else {
					// (r.gonet 12/05/2011) - PLID 46437 - Get whether to generate the barcode or not so we can copy that.
					BOOL bGenerateBarcode = FALSE;
					if(m_CurrReport->nID == 658 && m_bIsCustom) {
						_RecordsetPtr prsGenerateBarcode = CreateParamRecordset(
							"SELECT GenerateBarcode "
							"FROM CustomReportsT "
							"WHERE ID = {INT} AND Number = {INT}; ",
							m_CurrReport->nID, m_nCustomReportID);
						if(!prsGenerateBarcode->eof) {
							bGenerateBarcode = VarBool(prsGenerateBarcode->Fields->Item["GenerateBarcode"]->Value, FALSE);
						}
						prsGenerateBarcode->Close();
					}

					//now that we have the file saved to the hard drive, let's save the data
					ExecuteSql("INSERT INTO CustomReportsT (ID, Number, Title, FileName, Version, GenerateBarcode) VALUES "
						" (%li, %li, '%s', '%s', %li, %li)", m_CurrReport->nID, nNumber, _Q(dlg.m_strTitle), _Q(m_strSaveFileName), m_CurrReport->nVersion, bGenerateBarcode ? TRUE : FALSE);
					if(m_CurrReport->nID == 658) {
						// (r.gonet 10/16/2011) - PLID 45968 - Lab requests may have custom fields attached to them, which we would also like to copy.
						ExecuteParamSql("INSERT INTO LabReqCustomFieldsT (CustomReportNumber, LabCustomFieldID, ReportFieldName) "
							"SELECT {INT}, LabCustomFieldID, ReportFieldName "
						"FROM LabReqCustomFieldsT "
						"WHERE CustomReportNumber = {INT}; ",
						nNumber, m_nCustomReportID);
					}
				}

				//its not new anymore since we saved it
				m_bIsNew = false;

				//it has its own number now, so save that
				m_nCustomReportID = nNumber;
			}
			else if (dlg.m_bUpdateTitle) {
				//We know it has a customreportid by now.		
				if (IsStatement(m_CurrReport->nID)) {
					//this should really just update anything with the same filename since the filenames are unique
					ExecuteSql("UPDATE CustomReportsT SET Title = '%s' WHERE FileName = '%s'", _Q(dlg.m_strTitle), _Q(m_strSaveFileName));
				}
				else {
					ExecuteSql("UPDATE CustomReportsT SET Title = '%s' WHERE ID = %li AND Number = %li", _Q(dlg.m_strTitle), m_CurrReport->nID, m_nCustomReportID);
				}
			}


			m_bSaved = true;
			
		}
		return nResult;

	}NxCatchAll("Error Saving Report");

	return -1;	

}


void CEditReportDlg::OnSavereport() 
{

	SaveReport();
					
}

void CEditReportDlg::OnClose() 
{

	try {

		//check to see if they want to save the report
		long nResult = SaveReport();

		if (nResult == 0 || nResult == 1) {
			//this is a work around so that the designer will not set focus to any of the controls inside of it.
			//this is to fix a bug in the designer that occurs when you close the designer when a text
			//box has focus.
			m_designer->PutReportObject(m_designer->GetReportObject());

			
			m_Report.Release();
			m_designer.Release();

			CNxDialog::OnClose();
		}

	}NxCatchAll("Error closing reports");

	
}

void CEditReportDlg::OnVerifyreport() 
{
	CWaitCursor pWait;
	//long nOldTimeout;

	try {

		//lets change the CommandTimeout so that we aren't getting any timeout messages here
		
		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);
		//nOldTimeout = g_ptrRemoteData->CommandTimeout;

		//g_ptrRemoteData->CommandTimeout = 600;

		long nCustomReportID = -1;
		if(m_CurrReport->nID == 658) {
			// (r.gonet 10/16/2011) - PLID 45968 - For lab requests, the ttx generated can vary based on the custom report
			nCustomReportID = m_nCustomReportID;
		}
		//first, Create the ttx file for the report in the custom reports folder
		if (CreateAllTtxFiles(m_CurrReport->nID, GetCustomReportsPath(), nCustomReportID)) {

			//we made the ttx file successfully, so lets proceed
			
			//check to see that the ttx file name, is what we think it is
			CString strReportServerName = (LPCTSTR)m_Report->Database->Tables->Item[1]->GetLogOnServerName();
			CString strTtxFileName;
			if (IsStatement(m_CurrReport->nID)) {
				strTtxFileName = GetStatementFileName(m_CurrReport->nID) + ".ttx";
			}
			else {
				strTtxFileName = m_CurrReport->strReportFile + ".ttx";
			}

			if (strReportServerName.CompareNoCase(strTtxFileName) != 0) {

				//this should never really happen, but just in case, we need to handle it

				//the names do not match, so we are going to have to log on to the server and then tell them what to do
				m_Report->Database->LogOnServerEx("pdsmon.dll", (LPCTSTR)strTtxFileName, vtMissing, vtMissing, vtMissing, "Active Data (Field Definitions Only)");

				//End the wait cursor if we are here
				EndWaitCursor();

				//now we are logged on, so give them instructions on what to do
				m_InstructDlg.m_strTtxFileName  = strTtxFileName;
				m_InstructDlg.Create(IDD_VERIFY_INS, this);
				m_InstructDlg.SetWindowPos(&CWnd::wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_NOZORDER);
				m_InstructDlg.ShowWindow(SW_SHOW);


				/*
				// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
				int nResult = (int)ShellExecute(GetSafeHwnd(), NULL, "notepad.exe", GetSharedPath() ^ "VerifyingInstructions.txt", NULL, SW_SHOW);
				if (nResult < 32) {

					//show the message box, I guess...
					CString strInstructions;
					strInstructions.Format(" Please do the following: "
						"\n1. Put your mouse over the left side of the report, in the white space underneath \"Unbound Fields\""
						"\n2. Right click and choose Database and then SetLocation "
						"\n3. Click the \"Set Location\" Button "
						"\n4. Click on the + next to Current Connections to expand that item "
						"\n5. Expand the item again by clicking the + next to %s "
						"\n6. Double click on the %s underneath the %s you just expanded "
						"\n7. Click the Done button "
						"\n8. Click the Verify button again", strTtxFileName, strTtxFileName, strTtxFileName);

					//show them the instructions
					AfxMessageBox(strInstructions);
				}*/				
				

			}
			else {

				//we are good to go, so all we need to do is just verify
				HRESULT hr;

				hr = m_Report->Database->Verify();

				if (hr == 0) {
					
					//if we are here then everything worked and we need to update the version in the CustomReports Table
					ExecuteSql("UPDATE CustomReportsT SET Version = %li WHERE ID = %li AND Number = %li", m_CurrReport->nVersion, m_CurrReport->nID, m_CurrReport->nDefaultCustomReport);
					
					//we also have to update all the records in CustomReportsT that use this report
					_RecordsetPtr rsCustom = CreateRecordset("SELECT Id, number From CustomReportsT WHERE FileName = '%s'", m_strSaveFileName);
					
					while (! rsCustom->eof) {

						long nID, nNumber;
						nID = AdoFldLong(rsCustom, "ID");
						nNumber = AdoFldLong(rsCustom, "Number");

						ExecuteSql("UPDATE CustomReportsT SET Version = %li WHERE ID = %li AND Number = %li", m_CurrReport->nVersion, nID, nNumber);

						rsCustom->MoveNext();

					}

					// (j.gruber 2007-02-22 16:48) - PLID 24832 - Call a reportinfo function so we can handle subreports
					m_CurrReport->DeleteTtxFiles();
					//finally, just get rid of the ttx file becuse we don't want to keep it on their hard drive
					// (j.gruber 2007-02-20 14:44) - PLID 23786 - fix statements not deleting their ttx files

					//CString strFileName = GetCustomReportsPath() ^ strTtxFileName;
					//DeleteFile(strFileName);					

				}
			}

		}

		//reset the timeout
		//g_ptrRemoteData->CommandTimeout = nOldTimeout;

	}NxCatchAll("Error when attempting to Verify Report");/*, //reset the timeout
		g_ptrRemoteData->CommandTimeout = nOldTimeout;);*/
		
}
