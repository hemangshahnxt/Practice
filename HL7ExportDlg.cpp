// HL7ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "HL7ExportDlg.h"
#include "HL7Utils.h"
#include "NxMessageDef.h"
#include "HL7SettingsDlg.h"
#include "NxSocketUtils.h"
#include "RegUtils.h"
#include "InternationalUtils.h"
#include <NxHL7Lib/HL7DataUtils.h>
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include "FinancialRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CHL7ExportDlg dialog

// (z.manning 2008-07-18 12:18) - PLID 30728 - These columns are common to all the datalists on this dialog
// (except the HL7 settings group combo)
enum ListCommonColumns
{
	lccID = 0,
	lccMessageID,
};

CHL7ExportDlg::CHL7ExportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7ExportDlg::IDD, pParent)
{
	m_bExcludeProspects = FALSE;
	m_bRequeriedOnce = FALSE;
	// (r.gonet 12/11/2012) - PLID 54115 - Create a new HL7 client to manage the HL7 sends between
	// us and NxServer.
	// (z.manning 2013-05-20 11:21) - PLID 56777 - Renamed
	m_pHL7Client = new CHL7Client_Practice();
	//{{AFX_DATA_INIT(CHL7ExportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CHL7ExportDlg::~CHL7ExportDlg()
{
	// (r.gonet 12/11/2012) - PLID 54115 - Free up the HL7 Client
	if(m_pHL7Client) {
		delete m_pHL7Client;
		m_pHL7Client = NULL;
	}
}

void CHL7ExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7ExportDlg)
	DDX_Control(pDX, IDC_HL7_SEND, m_btnSend);
	DDX_Control(pDX, IDC_HL7_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_HL7_REMOVE_ALL, m_btnRemAll);
	DDX_Control(pDX, IDC_HL7_REMOVE, m_btnRem);
	DDX_Control(pDX, IDC_HL7_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_APPT_LIST_LABEL, m_nxstaticApptLabel);
	DDX_Control(pDX, IDC_APPT_EXPORT_LABEL, m_nxstaticApptExportLabel);
	DDX_Control(pDX, IDC_PATIENT_LIST_LABEL, m_nxstaticPatientLabel);
	DDX_Control(pDX, IDC_PATIENT_EXPORT_LABEL, m_nxstaticPatientExportLabel);
	DDX_Control(pDX, IDC_EMN_BILL_LIST_LABEL, m_nxstaticEmnBillLabel);
	DDX_Control(pDX, IDC_EMN_BILL_EXPORT_LABEL, m_nxstaticEmnBillExportLabel);
	DDX_Control(pDX, IDC_LOCKED_EMN_LABEL, m_nxstaticLockedEmnLabel);
	DDX_Control(pDX, IDC_LOCKED_EMN_EXPORT_LABEL, m_nxstaticLockedEmnExportLabel);
	DDX_Control(pDX, IDC_SIGNED_LAB_RESULT_LABEL, m_nxstaticLabResultLabel);
	DDX_Control(pDX, IDC_SIGNED_LAB_RESULT_EXPORT_LABEL, m_nxstaticLabResultExportLabel);
	DDX_Control(pDX, IDC_REFPHYS_LIST_LABEL, m_nxstaticRefPhysicianLabel);
	DDX_Control(pDX, IDC_REFPHYS_EXPORT_LABEL, m_nxstaticRefPhysicianExportLabel);
	DDX_Control(pDX, IDC_SEND_PRIMARY_IMAGE, m_nxbSendPrimaryImage);
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7ExportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7ExportDlg)
	ON_BN_CLICKED(IDC_HL7_SEND, OnHl7Send)
	ON_BN_CLICKED(IDC_HL7_ADD, OnHl7Add)
	ON_BN_CLICKED(IDC_HL7_REMOVE, OnHl7Remove)
	ON_BN_CLICKED(IDC_HL7_REMOVE_ALL, OnHl7RemoveAll)
	ON_BN_CLICKED(IDC_HL7_CLOSE, OnHl7Close)
	ON_BN_CLICKED(IDC_EDIT_SETTINGS, OnEditSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7ExportDlg message handlers

BOOL CHL7ExportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();


	// (d.singleton 2012-12-20 13:00) - PLID 53041 cashe properties
	g_propManager.BulkCache("HL7ExportDlg", propbitNumber, " (Username = '<None>' OR Username = '%s') AND ("
		"Name = 'LastHL7LockedEmnExport' OR "
		"Name = 'LastHL7LabResultExport' OR "
		"Name = 'LastHL7Export' OR "
		"Name = 'LastHL7ApptExport' OR "
		"Name = 'LastHL7EmnBillExport' OR "
		"Name = 'LastHL7ReferringPhysicianExport')", GetCurrentUserName());



	//init controls
	m_pSettings = BindNxDataListCtrl(IDC_SETTINGS_LIST);
	m_pPatients = BindNxDataListCtrl(IDC_PRAC_LIST, false);
	m_pAppts = BindNxDataListCtrl(IDC_HL7EXPORT_APPT_LIST, false); // (z.manning 2008-07-18 11:44) - PLID 30782
	m_pEmnBills = BindNxDataListCtrl(IDC_HL7EXPORT_EMN_BILL_LIST, false); //TES 7/10/2009 - PLID 34845
	m_pLockedEmns = BindNxDataListCtrl(IDC_HL7EXPORT_LOCKED_EMN_LIST, false); // (d.singleton 2012-10-04 16:52) - PLID 53041
	m_pLabResults = BindNxDataListCtrl(IDC_HL7EXPORT_LAB_RESULTS, false); // (d.singleton 2012-10-04 16:52) - PLID 53282
	m_pSyndromicList = BindNxDataListCtrl(IDC_SYNDROMIC_PATIENT_LIST, false); 
	m_pReferringPhyList = BindNxDataListCtrl(IDC_HL7EXPORT_REFPHYS_LIST, false); // (r.farnworth 2014-12-22 14:46) - PLID 64473

	//TES 5/27/2009 - PLID 34282 - Check their permission to edit HL7 settings
	if(!(GetCurrentUserPermissions(bioHL7Settings) & SPT_V__________ANDPASS)) {
		GetDlgItem(IDC_EDIT_SETTINGS)->EnableWindow(FALSE);
	}

	//TES 9/21/2010 - PLID 40595 - Moved all the requerying to OnSelChosenSettingsList() (it should really always have been there, and
	// we should be tracking different LastExport times by group, but that is another story, and shall be told another time).
	if(m_eExportType == hprtPatient)
		{
			ShowPatientControls(SW_SHOW);
			ShowApptControls(SW_HIDE);
			ShowEmnBillControls(SW_HIDE);
			ShowLockedEmnControls(SW_HIDE);
			ShowLabResultControls(SW_HIDE);
			ShowSyndromicControls(SW_HIDE);
			ShowReferringPhyControls(SW_HIDE);

			m_pExport = BindNxDataListCtrl(IDC_EXPORT_LIST, false);
		}
		// (z.manning 2008-07-18 14:16) - PLID 30782 - We now also support exporting appts in this dialog.
		else if(m_eExportType == hprtAppt)
		{
			ShowPatientControls(SW_HIDE);
			ShowApptControls(SW_SHOW);
			ShowEmnBillControls(SW_HIDE);
			ShowLockedEmnControls(SW_HIDE);
			ShowLabResultControls(SW_HIDE);
			ShowSyndromicControls(SW_HIDE);
			ShowReferringPhyControls(SW_HIDE);

			m_pExport = BindNxDataListCtrl(IDC_HL7EXPORT_APPT_EXPORT_LIST, false);
		}
		else if(m_eExportType == hprtEmnBill)
		{
			ShowPatientControls(SW_HIDE);
			ShowApptControls(SW_HIDE);
			ShowEmnBillControls(SW_SHOW);
			ShowLockedEmnControls(SW_HIDE);
			ShowLabResultControls(SW_HIDE);
			ShowSyndromicControls(SW_HIDE);
			ShowReferringPhyControls(SW_HIDE);

			m_pExport = BindNxDataListCtrl(IDC_HL7EXPORT_EMN_BILL_EXPORT_LIST, false);
		}
		else if(m_eExportType == hprtLockedEmn)
		{
			ShowPatientControls(SW_HIDE);
			ShowApptControls(SW_HIDE);
			ShowEmnBillControls(SW_HIDE);
			ShowLockedEmnControls(SW_SHOW);
			ShowLabResultControls(SW_HIDE);
			ShowSyndromicControls(SW_HIDE);
			ShowReferringPhyControls(SW_HIDE);

			m_pExport = BindNxDataListCtrl(IDC_HL7EXPORT_LOCKED_EMN_LIST_EXPORT, false);
		}
		else if(m_eExportType == hprtLabResult) {

			ShowPatientControls(SW_HIDE);
			ShowApptControls(SW_HIDE);
			ShowEmnBillControls(SW_HIDE);
			ShowLockedEmnControls(SW_HIDE);
			ShowLabResultControls(SW_SHOW);
			ShowSyndromicControls(SW_HIDE);
			ShowReferringPhyControls(SW_HIDE);

			m_pExport = BindNxDataListCtrl(IDC_HL7EXPORT_LAB_RESULTS_EXPORT, false);
		}
		// (b.spivey, November 1, 2013) - PLID 59267 - 
		else if (m_eExportType == hprtSyndromicSurveillance) {
			ShowPatientControls(SW_HIDE);
			ShowApptControls(SW_HIDE);
			ShowEmnBillControls(SW_HIDE);
			ShowLockedEmnControls(SW_HIDE);
			ShowLabResultControls(SW_HIDE);
			ShowSyndromicControls(SW_SHOW);
			ShowReferringPhyControls(SW_HIDE);

			// (b.spivey, November 1, 2013) - PLID 59267 - unused.
			GetDlgItem(IDC_SETTINGS_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_SETTINGS)->ShowWindow(SW_HIDE); 
			GetDlgItem(IDC_STATIC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SETTINGS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_SETTINGS)->EnableWindow(FALSE); 

			m_pExport = BindNxDataListCtrl(IDC_SYNDROMIC_PATIENT_EXPORT_LIST, false);
		}
		// (r.farnworth 2014-12-22 12:48) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
		else if (m_eExportType == hprtReferringPhysician)
		{
			ShowPatientControls(SW_HIDE);
			ShowApptControls(SW_HIDE);
			ShowEmnBillControls(SW_HIDE);
			ShowLockedEmnControls(SW_HIDE);
			ShowLabResultControls(SW_HIDE);
			ShowSyndromicControls(SW_HIDE);
			ShowReferringPhyControls(SW_SHOW);

			m_pExport = BindNxDataListCtrl(IDC_HL7EXPORT_REFPHYS_EXPORT_LIST, false);
		}
		else {
			// (z.manning 2008-07-18 14:14) - PLID 30782 - That's all we support for now.
			ASSERT(FALSE);
			EndDialog(IDCANCEL);
		}

	// (j.jones 2008-05-08 09:46) - PLID 29953 - added nxiconbuttons for modernization
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnSend.AutoSet(NXB_EXPORT);	//technically this may only batch and not export, based on their settings,
									//but use the export style anyways because that's the action they ultimately want

	m_btnAdd.AutoSet(NXB_DOWN);
	m_btnRem.AutoSet(NXB_UP);
	m_btnRemAll.AutoSet(NXB_UUP);

	//Figure most of the time there will only be the one HL7 group, so just set the selection to the first row.
	//TES 9/21/2010 - PLID 40595 - Make sure we've finished requerying the settings list, then call the SelChosen handler to requery
	// the main lists.
	m_bRequeriedOnce = false;
	m_pSettings->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	m_pSettings->CurSel = 0;
	OnSelChosenSettingsList(m_pSettings->CurSel);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7ExportDlg::OnOK() 
{
}

void CHL7ExportDlg::OnCancel() 
{
}

//returns the name of the temp table created
//Copied and modified from ExternalForm.cpp, CreateTempIDTable
//I went with the 7.0 version, because it should work in all cases, where the 
//8.0 version will not work with SQL 7
CString CreateTempIDTable(_DNxDataListPtr pDataList, short nIDColIndex)
{
	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it

	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#TempHL7Export%lu", GetTickCount());

	//The major reason this is not using a LW version, is that some reports filter on an ID, and some filter on a string!
	//So we need to create the id field appropriately.  NOTE:  This is all setup in the OnInitDialog, so we have no idea
	//what type the field is.  So get the value of the first one out, and try our best to guess from there.
	CString strType;
	_variant_t var = pDataList->GetValue(0, nIDColIndex);
	VARTYPE vt = var.vt;

	if(vt == VT_BSTR) {
		//we are working with a number
		strType.Format("nvarchar(500)");	//should be ample size
	}
	else if(vt == VT_I4) {
		strType.Format("int");
	}
	else {
		//TODO:  if you get this assertion, your type is not setup.  define it
		//		in a style similar to the above.  You also need to set it up
		//		in the while loop below.
		ASSERT(FALSE);
	}

	ExecuteSql("CREATE TABLE %s (ID %s)", strTempT, strType);

	// Loop through the datalist
	LONG nLastIDVal = -1;
	long i = 0;
	long p = pDataList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while (p) {
		pDataList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
		_variant_t var = pRow->GetValue(nIDColIndex);

		if(var.vt == VT_BSTR) {
			// Add the ID
			ExecuteSql("INSERT INTO %s (ID) VALUES ('%s')", strTempT, _Q(VarString(var, "")));
		}
		else if(var.vt == VT_I4) {
			ExecuteSql("INSERT INTO %s (ID) VALUES (%li)", strTempT, VarLong(var));
		}
		else {
			//TODO:  If you get this assertion, you probably got the one above.  You need to 
			//setup the insert for the type
			ASSERT(FALSE);
		}

	}


	// Return the name of the temp table
	return strTempT;

}


void CHL7ExportDlg::OnHl7Send() 
{
	try {
		//TES 11/10/2015 - PLID 67500 - If the type supports it, use the mass-export functionality
		if (m_eExportType == hprtPatient || m_eExportType == hprtAppt || m_eExportType == hprtLabResult || m_eExportType == hprtReferringPhysician || m_eExportType == hprtEmnBill) {
			ExportUsingMultiple();
			return;
		}
		// (a.wilson 2013-06-11 15:37) - PLID 57117 - add successful emnids to an array so we can then insert or update them in data.
		CArray<long> arySuccessfulEMNIDs;
		long nGroupID;
		if(m_pSettings->GetCurSel() == -1) {
			MessageBox("No group is selected.", "Practice", MB_OK | MB_ICONSTOP);
			return;
		}
		if(m_pExport->GetRowCount() == 0) {
			MessageBox(FormatString("No %ss are selected to export.", GetExportDescription()), "Practice", MB_OK | MB_ICONSTOP);
			return;
		}

		nGroupID = VarLong(m_pSettings->GetValue(m_pSettings->GetCurSel(), 0));
		CString strHL7GroupName = VarString(m_pSettings->GetValue(m_pSettings->GetCurSel(), 1));
		//check to see what type of export we are doing
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		long nType = GetHL7SettingInt(nGroupID, "ExportType");
		BOOL bSuccess = TRUE;
		CString strMessage, strTemp;

		CString strRemoteProperty;

		// (r.gonet 12/03/2012) - PLID 54115 - When the HL7 export code was in Practice, we used to
		//  distinguish between file and tcp/ip export methods here because Practice was instant while
		//  NxServer was being communicated with asyncronously. Since we send everything over to
		//  NxServer anyway, we no longer have to distinguish and have separate branches for each type.

		//visibly disable the send
		GetDlgItem(IDC_HL7_SEND)->EnableWindow(FALSE);
		EnableWindow(FALSE);
		BeginWaitCursor();

		CArray<long, long> aryPersonIDs;

		// (r.gonet 12/03/2012) - PLID 54115 - Iterate through the datalist and try to export each record in it
		while(m_pExport->GetRowCount() > 0) {
			long nRecordID = VarLong(m_pExport->GetValue(0, lccID));
			NXDATALISTLib::IRowSettingsPtr pRow = m_pExport->GetRow(0);
			HL7ResponsePtr pResponse;
			NXDATALISTLib::_DNxDataListPtr pTopList = NULL;
			if (m_eExportType == hprtLockedEmn) {
				// (d.singleton 2012-10-04 17:52) - PLID 53041
				// (b.spivey, December 12, 2012) - PLID 53040 - Send the EMN upon request. 
				long nEmnID = VarLong(m_pExport->GetValue(0, lccID));
				pResponse = m_pHL7Client->SendEMNHL7Message(nEmnID, nGroupID, false);
				pTopList = m_pLockedEmns;
			}
			else if(m_eExportType == hprtSyndromicSurveillance) {
				// (b.spivey, November 1, 2013) - PLID 59267 
				long nPersonID = VarLong(m_pExport->GetValue(0, lccID));
				aryPersonIDs.Add(nPersonID); 
				pTopList = m_pSyndromicList; 
			}

			// (r.gonet 12/03/2012) - PLID 54115 - Check the status of the return and terminate upon failure.
			// (b.spivey, November 1, 2013) - PLID 59267 - if syndromic, pResponse will be null. 
			if(m_eExportType == hprtSyndromicSurveillance && pResponse == NULL) {
				//devour this.
			}
			else if(pResponse->hmssSendStatus == hmssFailure_NotBatched) {
				// (r.gonet 12/03/2012) - PLID 54115 - We didn't even get so far as writing the message to the datababase.
				CString strMessage;
				strMessage.Format(
					"Error while exporting %s '%s':\r\n"
					"\r\n"
					"%s\r\n"
					"\r\n"
					"This %s has not been exported.  The export will now terminate.", GetExportDescription(), VarString(m_pExport->GetValue(0, 2)), pResponse->strErrorMessage, GetExportDescription());
				MessageBox(strMessage, "Practice", MB_OK | MB_ICONSTOP);
				break;
			} else if(pResponse->hmssSendStatus == hmssFailure_Batched) {
				// (r.gonet 12/03/2012) - PLID 54115 - We failed to send it, but the message was generated and written to the database.
				CString strMessage;
				strMessage.Format("Error while exporting %s '%s':\r\n"
					"%s\r\n"
					"This %s has not been exported, but instead has been batched for a future export in the HL7 tab of the Links module.", GetExportDescription(), VarString(m_pExport->GetValue(0, 2)), pResponse->strErrorMessage, GetExportDescription());
				MessageBox(strMessage, "Practice", MB_OK | MB_ICONWARNING);
				break;
			} else if(pResponse->hmssSendStatus == hmssBatched || pResponse->hmssSendStatus == hmssSent || pResponse->hmssSendStatus == hmssSent_AckReceived) {
				// (r.gonet 12/03/2012) - PLID 54115 - Mission completed.
				// (a.wilson 2013-06-11 15:40) - PLID 57117 - add to success array.
				if (pResponse->hemtMessageType == hemtNewEmnBill) {
					arySuccessfulEMNIDs.Add(nRecordID);
				}
			} else if(pResponse->hmssSendStatus == hmssSent_AckFailure) {
				// (r.gonet 12/03/2012) - PLID 54115 - If we don't get an ack back for this export and we needed one, then warn.
				CString strMessage;
				strMessage.Format("An error occurred while waiting for an acknowledgement from the HL7 link for the export of %s '%s':\r\n"
					"\r\n"
					"%s\r\n"
					"\r\n"
					"This %s has been exported but may not have been properly received by the third party.", GetExportDescription(), VarString(m_pExport->GetValue(0, 2)), pResponse->strErrorMessage, GetExportDescription());
				MessageBox(strMessage, "Practice", MB_OK | MB_ICONWARNING);
				break;
			} else if(m_eExportType == hprtSyndromicSurveillance) {
				// (b.spivey, November 1, 2013) - PLID 59267 Just eat this. 
			} else {
				// (r.gonet 12/03/2012) - PLID 54115 - Unrecognized status.
				ASSERT(FALSE);
			}

			// (r.gonet 12/03/2012) - PLID 54115 - Put the row back in the top bin for later exports.
			if (pTopList && pRow) {
				pTopList->TakeRow(pRow);
			}
		}

		// (r.gonet 12/03/2012) - PLID 54115 - If we have no rows left in our list, that means we did not terminate the export,
		//  which means we succeeded.
		if(m_pExport->GetRowCount() == 0) {
			// (r.gonet 12/03/2012) - PLID 54115 - That was the last record!
			GetDlgItem(IDC_HL7_SEND)->EnableWindow(TRUE);
			EnableWindow(TRUE);

			// (b.spivey, November 1, 2013) - PLID 59267 - this message doesn't make sense for syndromic
			if (m_eExportType != hprtSyndromicSurveillance) {
				MessageBox(FormatString("All %ss successfully exported!", GetExportDescription()), "Practice", MB_OK | MB_ICONINFORMATION);
			}

			CString strRemoteProperty;
			if(m_eExportType == hprtPatient) {
				strRemoteProperty = "LastHL7Export";
			}
			// (d.singleton 2012-10-04 17:01) - PLID 53041
			else if (m_eExportType == hprtLockedEmn) {
				strRemoteProperty = "LastHL7LockedEmnExport";
			}
			// (b.spivey, November 1, 2013) - PLID 59267 
			else if(m_eExportType == hprtSyndromicSurveillance) {
				m_arySyndromicPersonIDs.Copy(aryPersonIDs);
			}
			else {
				ASSERT(FALSE);
			}
			SetRemotePropertyDateTime(strRemoteProperty, COleDateTime::GetCurrentTime(), 0, "<None>");
			CClient::RefreshTable(NetUtils::HL7MessageLogT);
		}

		// (a.wilson 2013-06-11 15:39) - PLID 57117 - insert records into table to ensure they dont show in emns to be billed.
		if (arySuccessfulEMNIDs.GetCount() > 0) {
			MarkHL7EMNBillsAsSent(arySuccessfulEMNIDs);
		}

		// (r.gonet 12/03/2012) - PLID 54115 - Enable the dialog again.
		GetDlgItem(IDC_HL7_SEND)->EnableWindow(TRUE);
		EnableWindow(TRUE);
		EndWaitCursor();

		// (b.spivey, November 1, 2013) - PLID 59267 - Gotta exit here because the flow of this process wouldn't make sense if we don't.
		if(m_eExportType == hprtSyndromicSurveillance) {
			CDialog::OnCancel(); 
		}

	}NxCatchAll("Error in CHL7ExportDlg::OnHl7Send()");
}

//TES 11/10/2015 - PLID 67500 - Separate export functionality, for message types that use the mass export
void CHL7ExportDlg::ExportUsingMultiple()
{
	// (a.wilson 2013-06-11 15:37) - PLID 57117 - add successful emnids to an array so we can then insert or update them in data.
	CArray<long> arySuccessfulEMNIDs;
	long nGroupID;
	if (m_pSettings->GetCurSel() == -1) {
		MessageBox("No group is selected.", "Practice", MB_OK | MB_ICONSTOP);
		return;
	}
	if (m_pExport->GetRowCount() == 0) {
		MessageBox(FormatString("No %ss are selected to export.", GetExportDescription()), "Practice", MB_OK | MB_ICONSTOP);
		return;
	}

	nGroupID = VarLong(m_pSettings->GetValue(m_pSettings->GetCurSel(), 0));
	CString strHL7GroupName = VarString(m_pSettings->GetValue(m_pSettings->GetCurSel(), 1));
	//check to see what type of export we are doing
	//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
	long nType = GetHL7SettingInt(nGroupID, "ExportType");
	BOOL bSuccess = TRUE;
	CString strMessage, strTemp;

	CString strRemoteProperty;

	// (r.gonet 12/03/2012) - PLID 54115 - When the HL7 export code was in Practice, we used to
	//  distinguish between file and tcp/ip export methods here because Practice was instant while
	//  NxServer was being communicated with asyncronously. Since we send everything over to
	//  NxServer anyway, we no longer have to distinguish and have separate branches for each type.

	//visibly disable the send
	GetDlgItem(IDC_HL7_SEND)->EnableWindow(FALSE);
	EnableWindow(FALSE);
	BeginWaitCursor();

	CArray<long, long> aryPersonIDs;

	NXDATALISTLib::_DNxDataListPtr pTopList = NULL;

	// (r.gonet 12/03/2012) - PLID 54115 - Iterate through the datalist and try to export each record in it
	long nMultipleRecordIDs[HL7_MULTIPLE_RECORD_LIMIT];
	ZeroMemory(nMultipleRecordIDs, HL7_MULTIPLE_RECORD_LIMIT*sizeof(long));
	long nMultipleRecordIndex = 0;
	HL7ResponsePtr pLastResponse = NULL;
	bool bSomeRecordsSucceeded = false;
	for (long nRow = 0; nRow < m_pExport->GetRowCount(); nRow++) {
		long nRecordID = VarLong(m_pExport->GetValue(nRow, lccID));
		NXDATALISTLib::IRowSettingsPtr pRow = m_pExport->GetRow(nRow);
		//TES 11/10/2015 - PLID 67500 - Don't send yet, just add to our array
		nMultipleRecordIDs[nMultipleRecordIndex] = nRecordID;
		nMultipleRecordIndex++;
		//TES 11/10/2015 - PLID 67500 - If our array is full, send this message off
		if (nMultipleRecordIndex == HL7_MULTIPLE_RECORD_LIMIT) {
			if (m_eExportType == hprtPatient) {
				pLastResponse = m_pHL7Client->SendMultipleNewPatientHL7Message(nMultipleRecordIDs, nGroupID, false, "", m_nxbSendPrimaryImage.IsWindowVisible() && IsDlgButtonChecked(IDC_SEND_PRIMARY_IMAGE) ? true : false);
				pTopList = m_pPatients;
			} 
			else if (m_eExportType == hprtAppt) {
				pLastResponse = m_pHL7Client->SendMultipleNewAppointmentHL7Message(nMultipleRecordIDs, nGroupID, false, "");
				pTopList = m_pAppts;
			}
			else if (m_eExportType == hprtLabResult) {
				pLastResponse = m_pHL7Client->SendMultipleNewLabResultHL7Message(nMultipleRecordIDs, nGroupID, false);
				pTopList = m_pLabResults;
			}
			else if (m_eExportType == hprtReferringPhysician) {
				pLastResponse = m_pHL7Client->SendMultipleReferringPhysicianMessage(nMultipleRecordIDs, nGroupID, false);
				pTopList = m_pReferringPhyList;
			}
			else if (m_eExportType == hprtEmnBill) {
				pLastResponse = m_pHL7Client->SendMultipleNewEmnBillHL7Message(nMultipleRecordIDs, nGroupID, false);
				pTopList = m_pEmnBills;
			}
			else {
				ASSERT(FALSE);
				return;
			}
			bool bEndFound = false;
			for (long nRecord = 0; nRecord < HL7_MULTIPLE_RECORD_LIMIT && !bEndFound; nRecord++) {
				if (pLastResponse->nSuccessfulMessageIDs[nRecord] == 0) {
					bEndFound = true;
				}
				else {
					bSomeRecordsSucceeded = true;
					pTopList->TakeRow(m_pExport->GetRow(0));
				}
			}
			//TES 11/10/2015 - PLID 67500 - If the export failed, skip to the end, don't keep trying.
			if (pLastResponse->hmssSendStatus == hmssFailure_NotBatched || pLastResponse->hmssSendStatus == hmssFailure_Batched || pLastResponse->hmssSendStatus == hmssSent_AckFailure) {
				nRow = m_pExport->GetRowCount();
			}
			else {
				//TES 11/10/2015 - PLID 67500 - It succeeded, and we took out all the successful rows, so move back to the top (use -1 because nRow is about to increment)
				nRow = -1;
			}
			ZeroMemory(nMultipleRecordIDs, HL7_MULTIPLE_RECORD_LIMIT*sizeof(long));
			nMultipleRecordIndex = 0;
		}
	}

	//TES 11/10/2015 - PLID 67500 - If we haven't gotten a response yet (meaning we never filled up our array), or if our last response was successful, send off the last batch.
	if (pLastResponse == NULL || 
		(pLastResponse->hmssSendStatus != hmssFailure_NotBatched && pLastResponse->hmssSendStatus != hmssFailure_Batched && pLastResponse->hmssSendStatus != hmssSent_AckFailure) ) {
		if (nMultipleRecordIDs[0] == 0) {
			if (pLastResponse == NULL) {
				MsgBox("No records were found to export.");
				return;
			}
		}
		else {
			if (m_eExportType == hprtPatient) {
				pLastResponse = m_pHL7Client->SendMultipleNewPatientHL7Message(nMultipleRecordIDs, nGroupID, false, "", m_nxbSendPrimaryImage.IsWindowVisible() && IsDlgButtonChecked(IDC_SEND_PRIMARY_IMAGE) ? true : false);
				pTopList = m_pPatients;
			}
			else if (m_eExportType == hprtAppt) {
				pLastResponse = m_pHL7Client->SendMultipleNewAppointmentHL7Message(nMultipleRecordIDs, nGroupID, false, "");
				pTopList = m_pAppts;
			}
			else if (m_eExportType == hprtLabResult) {
				pLastResponse = m_pHL7Client->SendMultipleNewLabResultHL7Message(nMultipleRecordIDs, nGroupID, false);
				pTopList = m_pLabResults;
			}
			else if (m_eExportType == hprtReferringPhysician) {
				pLastResponse = m_pHL7Client->SendMultipleReferringPhysicianMessage(nMultipleRecordIDs, nGroupID, false);
				pTopList = m_pReferringPhyList;
			}
			else if (m_eExportType == hprtEmnBill) {
				pLastResponse = m_pHL7Client->SendMultipleNewEmnBillHL7Message(nMultipleRecordIDs, nGroupID, false);
				pTopList = m_pEmnBills;
			}
			else {
				ASSERT(FALSE);
				return;
			}
			bool bEndFound = false;
			for (long nRecord = 0; nRecord < HL7_MULTIPLE_RECORD_LIMIT && !bEndFound; nRecord++) {
				if (pLastResponse->nSuccessfulMessageIDs[nRecord] == 0) {
					bEndFound = true;
				}
				else {
					pTopList->TakeRow(m_pExport->GetRow(0));
				}
			}
		}
	}

	//TES 11/10/2015 - PLID 67500 - If a row failed, grab the description of the failing record
	long nFailedRow = m_pExport->FindByColumn(lccID, pLastResponse->nFailedRecordID, 0, g_cvarFalse);
	IRowSettingsPtr pFailedRow = NULL;
	if (nFailedRow != -1) {
		pFailedRow = m_pExport->GetRow(nFailedRow);
	}
	else {
		pFailedRow = m_pExport->GetRow(0);
	}
	//TES 11/10/2015 - PLID 67500 - Now report success or failure to the user. Note that it may have been partially successful, in which case we should add a sentence 
	// mentioning that some records succeeded
	CString strFailedDescription = (pFailedRow == NULL) ? "<Unknown>" : VarString(pFailedRow->GetValue(2));
	CString strSucceededDescription = bSomeRecordsSucceeded ? " The previous " + GetExportDescription() + "s were exported successfully.":"";
	if (pLastResponse->hmssSendStatus == hmssFailure_NotBatched) {
		// (r.gonet 12/03/2012) - PLID 54115 - We didn't even get so far as writing the message to the datababase.
		CString strMessage;
		strMessage.Format(
			"Error while exporting %s '%s':\r\n"
			"\r\n"
			"%s\r\n"
			"\r\n"
			"This %s has not been exported.%s  The export will now terminate.", GetExportDescription(), strFailedDescription, pLastResponse->strErrorMessage, GetExportDescription(), strSucceededDescription);
		MessageBox(strMessage, "Practice", MB_OK | MB_ICONSTOP);
	}
	else if (pLastResponse->hmssSendStatus == hmssFailure_Batched) {
		// (r.gonet 12/03/2012) - PLID 54115 - We failed to send it, but the message was generated and written to the database.
		CString strMessage;
		strMessage.Format("Error while exporting %s '%s':\r\n"
			"%s\r\n"
			"This %s has not been exported, but instead has been batched for a future export in the HL7 tab of the Links module.%s", GetExportDescription(), strFailedDescription, pLastResponse->strErrorMessage, GetExportDescription(), strSucceededDescription);
		MessageBox(strMessage, "Practice", MB_OK | MB_ICONWARNING);
	}
	else if (pLastResponse->hmssSendStatus == hmssBatched || pLastResponse->hmssSendStatus == hmssSent || pLastResponse->hmssSendStatus == hmssSent_AckReceived) {
		// (r.gonet 12/03/2012) - PLID 54115 - Mission completed.
		MessageBox(FormatString("All %ss successfully exported!", GetExportDescription()), "Practice", MB_OK | MB_ICONINFORMATION);
		CString strRemoteProperty;
		if (m_eExportType == hprtPatient) {
			strRemoteProperty = "LastHL7Export";
		}
		else if (m_eExportType == hprtAppt) {
			strRemoteProperty = "LastHL7ApptExport";
		}
		else if (m_eExportType == hprtLabResult) {
			strRemoteProperty = "LastHL7LabResultExport";
		}
		else if (m_eExportType == hprtReferringPhysician) {
			strRemoteProperty = "LastHL7ReferringPhysicianExport";
		}
		else if (m_eExportType == hprtEmnBill) {
			strRemoteProperty = "LastHL7EmnBillExport";
		}
		else {
			ASSERT(FALSE);
		}
		SetRemotePropertyDateTime(strRemoteProperty, COleDateTime::GetCurrentTime(), 0, "<None>");
		CClient::RefreshTable(NetUtils::HL7MessageLogT);
	}
	else if (pLastResponse->hmssSendStatus == hmssSent_AckFailure) {
		// (r.gonet 12/03/2012) - PLID 54115 - If we don't get an ack back for this export and we needed one, then warn.
		CString strMessage;
		strMessage.Format("An error occurred while waiting for an acknowledgement from the HL7 link for the export of %s '%s':\r\n"
			"\r\n"
			"%s\r\n"
			"\r\n"
			"This %s has been exported but may not have been properly received by the third party.%s", GetExportDescription(), strFailedDescription, pLastResponse->strErrorMessage, GetExportDescription(), strSucceededDescription);
		MessageBox(strMessage, "Practice", MB_OK | MB_ICONWARNING);
	}
	else {
		// (r.gonet 12/03/2012) - PLID 54115 - Unrecognized status.
		ASSERT(FALSE);
	}
		
	// (r.gonet 12/03/2012) - PLID 54115 - That was the last record!
	GetDlgItem(IDC_HL7_SEND)->EnableWindow(TRUE);
	EnableWindow(TRUE);
	EndWaitCursor();

}
void CHL7ExportDlg::OnHl7Add() 
{
	try {
		if(m_eExportType == hprtPatient) {
			//copy this row from patients to export list
			int nCurSel = m_pPatients->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pPatients);
			}
		}
		else if(m_eExportType == hprtAppt) {
			// (z.manning 2008-07-18 15:57) - PLID 30782 - Also need to handle appts
			int nCurSel = m_pAppts->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pAppts);
			}
		}
		else if(m_eExportType == hprtEmnBill) {
			//TES 7/10/2009 - PLID 34845 - Also need to handle EMNs
			int nCurSel = m_pEmnBills->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pEmnBills);
			}
		}
		else if(m_eExportType == hprtLockedEmn) {
			// (d.singleton 2012-10-04 17:45) - PLID 53041
			int nCurSel = m_pLockedEmns->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pLockedEmns);
			}
		}
		else if(m_eExportType == hprtLabResult) {
			// (d.singleton 2012-10-19 16:47) - PLID 53282
			int nCurSel = m_pLabResults->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pLabResults);
			}
		}
		else if (m_eExportType == hprtSyndromicSurveillance) {
			// (b.spivey, November 1, 2013) - PLID 59267 
			int nCurSel = m_pSyndromicList->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pSyndromicList);
			}
		}
		else if (m_eExportType == hprtReferringPhysician) {
			// (r.farnworth 2014-12-22 15:00) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
			int nCurSel = m_pReferringPhyList->GetCurSel();

			if (nCurSel != -1) {
				m_pExport->TakeCurrentRow(m_pReferringPhyList);
			}
		}
		else {
			ASSERT(FALSE);
		}
	} NxCatchAll("Error in OnHl7Add()");
}

void CHL7ExportDlg::OnHl7Remove() 
{
	try {
		if(m_eExportType == hprtPatient) {
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				m_pPatients->TakeCurrentRow(m_pExport);
			}
		}
		else if(m_eExportType == hprtAppt) {
			// (z.manning 2008-07-18 15:57) - PLID 30782 - Also need to handle appts
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				m_pAppts->TakeCurrentRow(m_pExport);
			}
		}
		else if(m_eExportType == hprtEmnBill) {
			//TES 7/10/2009 - PLID 34845 - Also need to handle EMNs
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				m_pEmnBills->TakeCurrentRow(m_pExport);
			}
		}
		else if(m_eExportType == hprtLockedEmn) {
			// (d.singleton 2012-10-04 17:44) - PLID 53041
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				m_pLockedEmns->TakeCurrentRow(m_pExport);
			}
		}
		else if(m_eExportType == hprtLabResult) {
			// (d.singleton 2012-10-19 16:48) - PLID 53282
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				m_pLabResults->TakeCurrentRow(m_pExport);
			}
		}
		else if (m_eExportType == hprtSyndromicSurveillance) {
			// (b.spivey, November 1, 2013) - PLID 59267 
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				// (b.spivey, December 12, 2013) - PLID 59267 - Was taking a row and giving it to the datalist I 
				//	 was taking it away from. 
				m_pSyndromicList->TakeCurrentRow(m_pExport);
			}
		}
		else if (m_eExportType == hprtReferringPhysician) {
			// (r.farnworth 2014-12-22 15:01) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
			int nCurSel = m_pExport->GetCurSel();

			if (nCurSel != -1) {
				m_pReferringPhyList->TakeCurrentRow(m_pExport);
			}
		}
		else {
			ASSERT(FALSE);
		}

	} NxCatchAll("Error in OnHl7Remove()");
}

void CHL7ExportDlg::OnHl7RemoveAll() 
{
	try {
		if(m_eExportType == hprtPatient) {
			m_pPatients->TakeAllRows(m_pExport);
		}
		else if(m_eExportType == hprtAppt) {
			// (z.manning 2008-07-18 15:57) - PLID 30782 - Also need to handle appts
			m_pAppts->TakeAllRows(m_pExport);
		}
		else if(m_eExportType == hprtEmnBill) {
			//TES 7/10/2009 - PLID 34845 - Also need to handle EMNs
			m_pEmnBills->TakeAllRows(m_pExport);
		}
		else if(m_eExportType == hprtLockedEmn) {
			// (d.singleton 2012-10-04 17:47) - PLID 53041
			m_pLockedEmns->TakeAllRows(m_pExport);
		}
		else if(m_eExportType == hprtLabResult) {
			// (d.singleton 2012-10-19 16:50) - PLID 
			m_pLabResults->TakeAllRows(m_pExport);
		}
		else if(m_eExportType == hprtSyndromicSurveillance) {
			// (b.spivey, November 1, 2013) - PLID 59267 
			m_pSyndromicList->TakeAllRows(m_pExport); 
		}
		else if (m_eExportType == hprtReferringPhysician) {
			// (r.farnworth 2014-12-22 15:01) - PLID 64473
			m_pReferringPhyList->TakeAllRows(m_pExport);
		}
		else {
			ASSERT(FALSE);
		}
	} NxCatchAll("Error in OnHl7RemoveAll()");
}

void CHL7ExportDlg::OnHl7Close() 
{
	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CHL7ExportDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHL7ExportDlg)
	ON_EVENT(CHL7ExportDlg, IDC_PRAC_LIST, 3 /* DblClickCell */, OnDblClickCellPracList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_EXPORT_LIST, 3 /* DblClickCell */, OnDblClickCellExportList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_APPT_LIST, 3 /* DblClickCell */, OnDblClickCellApptList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_APPT_EXPORT_LIST, 3 /* DblClickCell */, OnDblClickCellApptExportList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_EMN_BILL_LIST, 3, CHL7ExportDlg::OnDblClickCellHl7exportEmnBillList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_EMN_BILL_EXPORT_LIST, 3, CHL7ExportDlg::OnDblClickCellHl7exportEmnBillExportList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_LOCKED_EMN_LIST, 3, CHL7ExportDlg::OnDblClickCellLockedEmnList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_LOCKED_EMN_LIST_EXPORT, 3, CHL7ExportDlg::OnDblClickCellLockedEmnExportList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_LAB_RESULTS, 3, CHL7ExportDlg::OnDblClickCellLabResultList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_LAB_RESULTS_EXPORT, 3, CHL7ExportDlg::OnDblClickCellLabResultExportList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_SETTINGS_LIST, 16, CHL7ExportDlg::OnSelChosenSettingsList, VTS_I4)
	ON_EVENT(CHL7ExportDlg, IDC_SYNDROMIC_PATIENT_LIST, 3, CHL7ExportDlg::OnDblClickCellSyndromicSurveillanceList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_SYNDROMIC_PATIENT_EXPORT_LIST, 3, CHL7ExportDlg::OnDblClickCellSyndromicSurveillanceExportList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_REFPHYS_LIST, 3 /* DblClickCell */, OnDblClickCellRefPhysList, VTS_I4 VTS_I2)
	ON_EVENT(CHL7ExportDlg, IDC_HL7EXPORT_REFPHYS_EXPORT_LIST, 3 /* DblClickCell */, OnDblClickCellRefPhysExport, VTS_I4 VTS_I2)
END_EVENTSINK_MAP()

void CHL7ExportDlg::OnDblClickCellPracList(long nRowIndex, short nColIndex) 
{
	OnHl7Add();
}

void CHL7ExportDlg::OnDblClickCellExportList(long nRowIndex, short nColIndex) 
{
	OnHl7Remove();
}

// (r.farnworth 2014-12-22 16:02) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
void CHL7ExportDlg::OnDblClickCellRefPhysList(long nRowIndex, short nColIndex)
{
	OnHl7Add();
}

// (r.farnworth 2014-12-22 16:02) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
void CHL7ExportDlg::OnDblClickCellRefPhysExport(long nRowIndex, short nColIndex)
{
	OnHl7Remove();
}

// (z.manning 2008-07-18 16:02) - PLID 30782 - Double click handlers for appt lists
void CHL7ExportDlg::OnDblClickCellApptList(long nRowIndex, short nColIndex)
{
	try
	{
		OnHl7Add();

	}NxCatchAll("CHL7ExportDlg::OnDblClickCellApptList");
}
void CHL7ExportDlg::OnDblClickCellApptExportList(long nRowIndex, short nColIndex)
{
	try
	{
		OnHl7Remove();

	}NxCatchAll("CHL7ExportDlg::OnDblClickCellApptExportList");
}

void CHL7ExportDlg::OnEditSettings() 
{
	//TES 5/27/2009 - PLID 34282 - Check their permission
	if(!CheckCurrentUserPermissions(bioHL7Settings, sptView)) {
		return;
	}
	CHL7SettingsDlg dlg(this);
	dlg.DoModal();	
	m_pSettings->Requery();
	//TES 9/21/2010 - PLID 40595 - Make sure the requery finishes, then call the OnSelChosen handler to requery the lists if needed
	// (we may now be filtering out prospects when we weren't before).
	m_pSettings->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	m_pSettings->CurSel = 0;
	OnSelChosenSettingsList(m_pSettings->CurSel);
}

// (z.manning 2008-07-18 12:26) - PLID 30782 - Functions to show/hide patient or appointment controls
void CHL7ExportDlg::ShowApptControls(UINT nShow)
{
	GetDlgItem(IDC_HL7EXPORT_APPT_LIST)->ShowWindow(nShow);
	GetDlgItem(IDC_HL7EXPORT_APPT_EXPORT_LIST)->ShowWindow(nShow);
	m_nxstaticApptLabel.ShowWindow(nShow);
	m_nxstaticApptExportLabel.ShowWindow(nShow);
}
void CHL7ExportDlg::ShowPatientControls(UINT nShow)
{
	GetDlgItem(IDC_EXPORT_LIST)->ShowWindow(nShow);
	GetDlgItem(IDC_PRAC_LIST)->ShowWindow(nShow);
	m_nxstaticPatientLabel.ShowWindow(nShow);
	m_nxstaticPatientExportLabel.ShowWindow(nShow);
	//TES 9/29/2015 - PLID 66193 - The Send Primary Image checkbox is only for patients, and only if the MDI integration is enabled
	if (m_pSettings->CurSel == -1) {
		m_nxbSendPrimaryImage.ShowWindow(SW_HIDE);
	}
	else {
		long nGroupID = VarLong(m_pSettings->GetValue(m_pSettings->GetCurSel(), 0));
		if (!GetHL7SettingBit(nGroupID, "EnableIntelleChart")) {
			m_nxbSendPrimaryImage.ShowWindow(SW_HIDE);
		}
		else {
			m_nxbSendPrimaryImage.ShowWindow(nShow);
		}
	}
}
void CHL7ExportDlg::ShowReferringPhyControls(UINT nShow)
{
	// (r.farnworth 2014-12-22 14:23) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
	GetDlgItem(IDC_HL7EXPORT_REFPHYS_LIST)->ShowWindow(nShow);
	GetDlgItem(IDC_HL7EXPORT_REFPHYS_EXPORT_LIST)->ShowWindow(nShow);
	m_nxstaticRefPhysicianLabel.ShowWindow(nShow);
	m_nxstaticRefPhysicianExportLabel.ShowWindow(nShow);
}
void CHL7ExportDlg::ShowEmnBillControls(UINT nShow)
{
	//TES 7/10/2009 - PLID 34845 - Added option for EMN billing information
	GetDlgItem(IDC_HL7EXPORT_EMN_BILL_LIST)->ShowWindow(nShow);
	GetDlgItem(IDC_HL7EXPORT_EMN_BILL_EXPORT_LIST)->ShowWindow(nShow);
	m_nxstaticEmnBillLabel.ShowWindow(nShow);
	m_nxstaticEmnBillExportLabel.ShowWindow(nShow);
}
void CHL7ExportDlg::ShowLockedEmnControls(UINT nShow)
{
	// (d.singleton 2012-10-04 17:00) - PLID 53041 add options for locked emns
	GetDlgItem(IDC_HL7EXPORT_LOCKED_EMN_LIST)->ShowWindow(nShow);
	GetDlgItem(IDC_HL7EXPORT_LOCKED_EMN_LIST_EXPORT)->ShowWindow(nShow);
	m_nxstaticLockedEmnLabel.ShowWindow(nShow);
	m_nxstaticLockedEmnExportLabel.ShowWindow(nShow);
}
void CHL7ExportDlg::ShowLabResultControls(UINT nShow)
{
	// (d.singleton 2012-10-19 16:53) - PLID 53282
	GetDlgItem(IDC_HL7EXPORT_LAB_RESULTS)->ShowWindow(nShow);
	GetDlgItem(IDC_HL7EXPORT_LAB_RESULTS_EXPORT)->ShowWindow(nShow);
	m_nxstaticLabResultLabel.ShowWindow(nShow);
	m_nxstaticLabResultExportLabel.ShowWindow(nShow);
}

// (b.spivey, November 1, 2013) - PLID 59267 
void CHL7ExportDlg::ShowSyndromicControls(UINT nShow)
{

	GetDlgItem(IDC_SYNDROMIC_PATIENT_LIST)->ShowWindow(nShow);
	GetDlgItem(IDC_SYNDROMIC_PATIENT_EXPORT_LIST)->ShowWindow(nShow);
	m_nxstaticPatientLabel.ShowWindow(nShow);
	m_nxstaticPatientExportLabel.ShowWindow(nShow);
}


// (z.manning 2008-07-21 10:19) - PLID 30782 - It's now required to pass in the export type when
// opening this dialog.
int CHL7ExportDlg::DoModal(HL7PracticeRecordType eExportType)
{
	int nResult = IDCANCEL;
	try
	{
		m_eExportType = eExportType;
		nResult = CNxDialog::DoModal();

	}NxCatchAll("CHL7ExportDlg::DoModal");

	return nResult;
}

int CHL7ExportDlg::DoModal()
{
	// (z.manning 2008-07-21 10:37) - PLID 30782 - This is a protected function and should never
	// actually be called.
	ASSERT(FALSE);
	m_eExportType = hprtPatient;
	return CNxDialog::DoModal();
}

// (r.gonet 12/12/2012) - PLID 54114 - Made descriptions singular
CString CHL7ExportDlg::GetExportDescription()
{
	switch(m_eExportType)
	{
		case hprtPatient:
			return "Patient";
			break;

		case hprtAppt:
			return "Appointment";
			break;

		//TES 7/10/2009 - PLID 34845 - Added option for EMN billing information
		case hprtEmnBill:
			return "EMN";
			break;

		// (d.singleton 2012-10-04 17:16) - PLID 53041
		case hprtLockedEmn:
			return "Locked EMN";
			break;

		// (d.singleton 2012-10-19 16:57) - PLID 53282
		case hprtLabResult:
			return "Lab Result";
			break;

		// (b.spivey, November 1, 2013) - PLID 59267 
		case hprtSyndromicSurveillance:
			return "Syndromic Message";
			break; 

		// (r.farnworth 2014-12-22 15:02) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
		case hprtReferringPhysician:
			return "Referring Physician";
			break;

		default:
			ASSERT(FALSE);
			return "";
			break;			
	}
}

//TES 7/10/2009 - PLID 34845 - Added option for EMN billing information
void CHL7ExportDlg::OnDblClickCellHl7exportEmnBillList(long nRowIndex, short nColIndex)
{
	try
	{
		OnHl7Add();

	}NxCatchAll("Error in CHL7ExportDlg::OnDblClickCellHl7exportEmnBillList()");
}

//TES 7/10/2009 - PLID 34845 - Added option for EMN billing information
void CHL7ExportDlg::OnDblClickCellHl7exportEmnBillExportList(long nRowIndex, short nColIndex)
{
	try
	{
		OnHl7Remove();
	}NxCatchAll("Error in OnDblClickCellHl7exportEmnBillExportList()");
}

// (d.singleton 2012-10-04 17:17) - PLID 53041
void CHL7ExportDlg::OnDblClickCellLockedEmnList(long nRowIndex, short nColIndex)
{
	try {
		OnHl7Add();
	}NxCatchAll("Error in OnDblClickCellLockedEmnList()");
}

// (d.singleton 2012-10-04 17:19) - PLID 53041
void CHL7ExportDlg::OnDblClickCellLockedEmnExportList(long nRowIndex, short nColIndex)
{
	try {
		OnHl7Remove();
	}NxCatchAll("Error on OnDblClickCellLockedEmnExportList()");
}
// (d.singleton 2012-10-04 17:17) - PLID 53282
void CHL7ExportDlg::OnDblClickCellLabResultList(long nRowIndex, short nColIndex)
{
	try {
		OnHl7Add();
	}NxCatchAll("Error in OnDblClickCellLockedEmnList()");
}

// (d.singleton 2012-10-04 17:19) - PLID 53282
void CHL7ExportDlg::OnDblClickCellLabResultExportList(long nRowIndex, short nColIndex)
{
	try {
		OnHl7Remove();
	}NxCatchAll("Error on OnDblClickCellLockedEmnExportList()");
}

// (b.spivey, November 1, 2013) - PLID 59267 - event handlers for the new datalists.
void CHL7ExportDlg::OnDblClickCellSyndromicSurveillanceList(long nRowIndex, short nColIndex)
{
	try {
		OnHl7Add();
	}NxCatchAll(__FUNCTION__);
}


void CHL7ExportDlg::OnDblClickCellSyndromicSurveillanceExportList(long nRowIndex, short nColIndex)
{
	try {
		OnHl7Remove();
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, November 1, 2013) - PLID 59267 - specific to syndromic, shouldn't be used with other export types.
// (r.gonet 03/18/2014) - PLID 60782 - Changed the array to diagnosis code IDs rather than the code numbers due to ICD-9, ICD-10 overlap
void CHL7ExportDlg::SetDiagnosisCodeArray(const CArray<long, long>& aryDiagCodeIDs)
{
	m_aryDiagCodeIDs.Copy(aryDiagCodeIDs);
}

void CHL7ExportDlg::GetSyndromicPersonIDs(CArray<long, long>& aryPersonIDs)
{
	aryPersonIDs.Copy(m_arySyndromicPersonIDs);
}

void CHL7ExportDlg::OnSelChosenSettingsList(long nRow)
{
	try {
		//TES 9/21/2010 - PLID 40595 - Moved all the requerying here from OnInitDialog().
		if(m_pSettings->GetCurSel() == -1) {
			return;
		}
		long nGroupID = VarLong(m_pSettings->GetValue(m_pSettings->GetCurSel(), 0));

		if(m_eExportType == hprtPatient)
		{
			//TES 9/21/2010 - PLID 40595 - If this group is set to exclude prospects, then filter them out of the lists.
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			BOOL bExcludeProspects = GetHL7SettingBit(nGroupID, "ExcludeProspects");
			CString strProspectsClause = bExcludeProspects?"CurrentStatus <> 2":"1=1";
			//TES 9/21/2010 - PLID 40595 - We only need to requery if we haven't yet, or if we've switched between two groups that have
			// a different Exclude Prospects setting.
			if(!m_bRequeriedOnce || bExcludeProspects != m_bExcludeProspects) {
				//TES 9/21/2010 - PLID 40595 - Remember our current ExcludeProspects setting.
				m_bExcludeProspects = bExcludeProspects;
				//TES 10/5/2005 - Auto-fill the select list with those modified since the last export.
				COleDateTime dtLastExport = GetRemotePropertyDateTime("LastHL7Export", NULL, 0, "<None>", false);
				if(dtLastExport.GetStatus() == COleDateTime::valid) {
					CString strPatWhere;
					strPatWhere.Format("Archived = 0 AND ID <> -25 AND CurrentStatus <> 4 AND "
						"PatientsT.ModifiedDate < '%s' AND %s", FormatDateTimeForSql(dtLastExport), strProspectsClause);
					m_pPatients->WhereClause = _bstr_t(strPatWhere);
					CString strExportWhere;
					strExportWhere.Format("Archived = 0 AND ID <> -25 AND CurrentStatus <> 4 AND "
						"PatientsT.ModifiedDate >= '%s' AND %s", FormatDateTimeForSql(dtLastExport), strProspectsClause);
					m_pExport->WhereClause = _bstr_t(strExportWhere);
					m_pPatients->Requery();
					m_pExport->Requery();
				}
				else {
					//They don't have a "last export" logged, don't auto-select anybody.
					m_pPatients->WhereClause = _bstr_t("Archived = 0 AND ID <> -25 AND CurrentStatus <> 4 AND " + strProspectsClause);
					m_pPatients->Requery();
				}
			}
			if (GetHL7SettingBit(nGroupID, "EnableIntelleChart")) {
				m_nxbSendPrimaryImage.ShowWindow(SW_SHOWNA);
			}
			else {
				m_nxbSendPrimaryImage.ShowWindow(SW_HIDE);
			}
		}
		// (z.manning 2008-07-18 14:16) - PLID 30782 - We now also support exporting appts in this dialog.
		else if(m_eExportType == hprtAppt)
		{
			//TES 9/21/2010 - PLID 40595 - We only need to requery if we haven't done so yet.
			if(!m_bRequeriedOnce) {
				// (z.manning 2008-07-18 14:16) - PLID 30782 - Same as patients, let's default to selecting
				// all patients since last export.
				COleDateTime dtLastApptExport = GetRemotePropertyDateTime("LastHL7ApptExport", NULL, 0, "<None>", false);
				if(dtLastApptExport.GetStatus() == COleDateTime::valid)
				{
					m_pAppts->PutWhereClause(_bstr_t(FormatString(
						"AppointmentsT.Status <> 4 AND AppointmentsT.ModifiedDate < '%s'"
						, FormatDateTimeForSql(dtLastApptExport))));
					m_pAppts->Requery();

					m_pExport->PutWhereClause(_bstr_t(FormatString(
						"AppointmentsT.Status <> 4 AND AppointmentsT.ModifiedDate >= '%s'"
						, FormatDateTimeForSql(dtLastApptExport))));
					m_pExport->Requery();
				}
				else {
					m_pAppts->Requery();
				}
			}

		}
		else if(m_eExportType == hprtEmnBill)
		{
			//TES 9/21/2010 - PLID 40595 - We only need to requery if we haven't done so yet.
			if(!m_bRequeriedOnce) {
				//TES 7/10/2009 - PLID 34845 - Same as the others, let's default to selecting
				// all EMNs since last export.
				COleDateTime dtLastEmnBillExport = GetRemotePropertyDateTime("LastHL7EmnBillExport", NULL, 0, "<None>", false);
				if(dtLastEmnBillExport.GetStatus() == COleDateTime::valid)
				{
					// (r.gonet 2015-07-07) - PLID 62466 - Removed the IN clause that ensured that the EMN had non-deleted charges. That bit is now part of an inner join
					// in the from clause.
					m_pEmnBills->PutWhereClause(_bstr_t(FormatString(
						"EmrMasterT.Deleted = 0 AND EmrMasterT.ModifiedDate < '%s'"
						, FormatDateTimeForSql(dtLastEmnBillExport))));
					m_pEmnBills->Requery();

					m_pExport->PutWhereClause(_bstr_t(FormatString(
						"EmrMasterT.Deleted = 0 AND EmrMasterT.ID IN (SELECT EmrID FROM EmrChargesT WHERE EmrChargesT.Deleted = 0) AND EmrMasterT.ModifiedDate >= '%s'"
						, FormatDateTimeForSql(dtLastEmnBillExport))));
					m_pExport->Requery();
				}
				else {
					m_pEmnBills->Requery();
				}
			}

		}
		else if(m_eExportType == hprtLockedEmn)
		{
			// (d.singleton 2012-10-04 17:24) - PLID 53041 - We only need to requery if we haven't done so yet.
			if(!m_bRequeriedOnce) {
				// (d.singleton 2012-10-04 17:24) - PLID 53041 - Same as the others, let's default to selecting
				// all EMNs since last export.
				COleDateTime dtLastLockedEmnExport = GetRemotePropertyDateTime("LastHL7LockedEmnExport", NULL, 0, "<None>", false);
				if(dtLastLockedEmnExport.GetStatus() == COleDateTime::valid)
				{
					m_pLockedEmns->PutWhereClause(_bstr_t(FormatString(
						"EmrMasterT.Deleted = 0 AND EmrMasterT.Status = 2 AND EmrMasterT.ModifiedDate < '%s' "
						, FormatDateTimeForSql(dtLastLockedEmnExport))));
					m_pLockedEmns->Requery();

					m_pExport->PutWhereClause(_bstr_t(FormatString(
						"EmrMasterT.Deleted = 0 AND EmrMasterT.Status = 2 AND EmrMasterT.ModifiedDate >= '%s'"
						, FormatDateTimeForSql(dtLastLockedEmnExport))));
					m_pExport->Requery();
				}
				else {
					m_pLockedEmns->Requery();
				}
			}
		}
		else if(m_eExportType == hprtLabResult) {
			// (d.singleton 2012-10-22 16:33) - PLID 53282 need to be able to export lab results as well
			if(!m_bRequeriedOnce) {
				// (d.singleton 2012-10-04 17:24) - PLID 53282 Same as the others, let's default to selecting
				// all results since last export.
				COleDateTime dtLastLabResultExport = GetRemotePropertyDateTime("LastHL7LabResultExport", NULL, 0, "<None>", false);
				if(dtLastLabResultExport.GetStatus() == COleDateTime::valid)
				{
					m_pLabResults->PutWhereClause(_bstr_t(FormatString(
						"LabResultsT.Deleted = 0 and ResultSignedDate IS NOT NULL AND LabResultsT.ResultSignedDate < '%s'"
						, FormatDateTimeForSql(dtLastLabResultExport))));
					m_pLabResults->Requery();

					m_pExport->PutWhereClause(_bstr_t(FormatString(
						"LabResultsT.Deleted = 0 and ResultSignedDate IS NOT NULL AND LabResultsT.ResultSignedDate >= '%s'"
						, FormatDateTimeForSql(dtLastLabResultExport))));
					m_pExport->Requery();
				}
				else {
					m_pLabResults->Requery();
				}
			}
		}
		else if(m_eExportType == hprtSyndromicSurveillance) {
			// (b.spivey, November 1, 2013) - PLID 59267 - fill datalist with syndromic data.
			if(!m_bRequeriedOnce) {
				//this query returns a ton of records and multiple per patient. All we want is the patient's UID, last, First, and middle. 
				// (r.gonet 03/18/2014) - PLID 60782 - Changed the array to diagnosis code IDs rather than the code numbers due to ICD-9, ICD-10 overlap
				m_pSyndromicList->FromClause = _bstr_t(" ( SELECT DISTINCT UserDefinedID, PersonID, Last, First, Middle "
					"FROM (" + CreateSyndromicBatchQuery(m_aryDiagCodeIDs).Flatten() + ") InnerSubQ"
					" ) OuterSubQ "); 
				m_pSyndromicList->Requery();
			}
		}
		// (r.farnworth 2014-12-22 15:09) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
		else if (m_eExportType == hprtReferringPhysician){
			if (!m_bRequeriedOnce) {
				COleDateTime dtLastReferringPhyExport = GetRemotePropertyDateTime("LastHL7ReferringPhysicianExport", NULL, 0, "<None>", false);
				if (dtLastReferringPhyExport.GetStatus() == COleDateTime::valid){
					m_pReferringPhyList->PutWhereClause(_bstr_t(FormatString(
						// (r.farnworth 2015-01-22 10:23) - PLID 64473 - We do not support a LastModifiedDate for Referring Physicians so we cannot include it in the where clause
						"PersonID > 0 and Archived = 0 "
						, FormatDateTimeForSql(dtLastReferringPhyExport))));
					m_pReferringPhyList->Requery();

					m_pExport->PutWhereClause(_bstr_t(FormatString(
						"PersonID > 0 and Archived = 0 AND ModifiedDate >= '%s'"
						, FormatDateTimeForSql(dtLastReferringPhyExport))));
					m_pExport->Requery();
				}
				else {
					m_pReferringPhyList->Requery();
				}
			}
		}
		else {
			// (z.manning 2008-07-18 14:14) - PLID 30782 - That's all we support for now.
			ASSERT(FALSE);
		}

		//TES 9/21/2010 - PLID 40595 - We've now requeried at least once.
		m_bRequeriedOnce = true;
	}NxCatchAll(__FUNCTION__);
}