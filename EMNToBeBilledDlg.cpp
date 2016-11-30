// EMNToBeBilledDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMNToBeBilledDlg.h"
#include "BillingModuleDlg.h"
#include "GlobalFinancialUtils.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "LinkEMNToBillDlg.h"
#include "AuditTrail.h"
#include "EMRPreviewPopupDlg.h"
#include "ChargeSplitDlg.h"	// (j.dinatale 2012-01-12 11:37) - PLID 47483 - need the charge split dlg here
#include "HL7Utils.h"
#include "HL7Client_Practice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

#define COLUMN_PATIENT_ID	0
#define COLUMN_EMN_ID		1
// (a.walling 2010-01-11 13:17) - PLID 31482
#define COLUMN_PREVIEW		2
#define COLUMN_PATIENT_NAME	3
#define COLUMN_DESCRIPTION	4
#define COLUMN_DATE			5
#define COLUMN_STATUS		6
#define COLUMN_COST			7
#define COLUMN_ASSIGN_CHARGE_RESP	8	// (j.dinatale 2012-01-11 16:15) - PLID 47483 - inserted assign charge resp column 
#define COLUMN_BILL_EMN		9
#define COLUMN_LINK_EXISTING_BILL	10	// (j.jones 2009-06-24 16:30) - PLID 24076
//#define COLUMN_GOTO_PATIENT	9	// (a.walling 2010-04-26 13:01) - PLID 35921 - the Patient Name will be the hyperlink
#define COLUMN_COLOR		11	// (j.jones 2008-06-05 12:36) - PLID 30255
#define COLUMN_MODIFIED_DATE 12	// (z.manning 2012-09-10 15:24) - PLID 52543
#define COLUMN_SEND_TO_HL7	13	// (a.wilson 2013-05-23 11:56) - PLID 56784

/////////////////////////////////////////////////////////////////////////////
// CEMNToBeBilledDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

CEMNToBeBilledDlg::CEMNToBeBilledDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMNToBeBilledDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMNToBeBilledDlg)
		m_BillingDlg = NULL;
	//}}AFX_DATA_INIT

	m_bShowState = false;

	m_bCreated = FALSE;

	// (j.dinatale 2012-02-02 11:20) - PLID 47846
	m_nEMNIDToSelAfterRef = -1;

	// (a.walling 2010-01-11 12:11) - PLID 31482
	m_hIconPreview = NULL;
	m_pEMRPreviewPopupDlg = NULL;

	// (a.wilson 2013-06-11 16:51) - PLID 57117
	m_pHL7Client.reset(new CHL7Client_Practice());
}


void CEMNToBeBilledDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMNToBeBilledDlg)
	DDX_Control(pDX, IDC_BTN_EMN_TO_BE_BILLED_PREVIEW, m_btnPreview);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_EMN_TO_BE_BILLED_REFRESH, m_btnRefresh);
	DDX_Control(pDX, IDC_BTN_SEND_BILL_TO_HL7, m_btnSendToHL7);
	DDX_Control(pDX, IDC_CHECK_ALL_FOR_HL7, m_lblCheckAll);
	DDX_Control(pDX, IDC_UNCHECK_ALL_FOR_HL7, m_lblUncheckAll);
	DDX_Control(pDX, IDC_ONLY_SHOW_INSURANCE_RESP_CHECK, m_chkHidePatientResp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMNToBeBilledDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMNToBeBilledDlg)
	ON_BN_CLICKED(IDC_BTN_EMN_TO_BE_BILLED_PREVIEW, OnBtnEmnToBeBilledPreview)
	ON_BN_CLICKED(IDC_BTN_EMN_TO_BE_BILLED_REFRESH, OnBtnEmnToBeBilledRefresh)
	ON_WM_DESTROY()	
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_SEND_BILL_TO_HL7, &CEMNToBeBilledDlg::OnBnClickedBtnSendBillToHl7)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_HIDE_PATIENT_RESP_CHECK, &CEMNToBeBilledDlg::OnBnClickedHidePatientRespCheck)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMNToBeBilledDlg message handlers

BOOL CEMNToBeBilledDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (a.walling 2010-01-11 12:11) - PLID 31482
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

		// (c.haag 2008-04-28 11:46) - PLID 29806 - NxIconize the close button
		m_btnOK.AutoSet(NXB_CLOSE);
		// (j.jones 2008-07-03 15:48) - PLID 18354 - added m_btnPreview
		m_btnPreview.AutoSet(NXB_PRINT_PREV);
		// (j.jones 2010-07-07 15:22) - PLID 36682 - added a refresh button
		m_btnRefresh.AutoSet(NXB_REFRESH);
		// (a.wilson 2013-05-28 11:30) - PLID 56784 - send to hl7 button icon
		m_btnSendToHL7.AutoSet(NXB_EXPORT);

		// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the dialog in the taskbar if necessary
		//								  PLID 22877 - and respect the preference to not do so
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}
		
		// (j.jones 2008-06-05 12:38) - PLID 30255 - converted to be a datalist2
		m_List = BindNxDataList2Ctrl(this, IDC_UNBILLED_EMN_LIST, GetRemoteData(), false);

		// (a.walling 2010-01-11 14:23) - PLID 31482 - Set up this column to pull in the HICON handle
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_List->GetColumn(COLUMN_PREVIEW);
		if (pCol != NULL && m_hIconPreview != NULL) {
			CString strHICON;
			strHICON.Format("%li", m_hIconPreview);
			pCol->FieldName = (LPCTSTR)strHICON;
		}

		// (a.wilson 2013-05-23 16:48) - PLID 56784 - hide send to hl7 if they dont have specific settings.
		{
			CArray<long, long> aryDefaultGroupIDs;
			//get hl7 group settings that export emn bills.
			GetHL7SettingsGroupsBySetting("ExportEmnBills", TRUE, aryDefaultGroupIDs);
			//if either of these are no good then we need to hide the send to hl7 controls.
			if (!g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent) || aryDefaultGroupIDs.GetCount() <= 0) {
				//hide list column
				NXDATALIST2Lib::IColumnSettingsPtr pCol = m_List->GetColumn(COLUMN_SEND_TO_HL7);
				pCol->PutColumnStyle(csFixedWidth);
				pCol->PutStoredWidth(0);
				//hide button
				m_btnSendToHL7.EnableWindow(FALSE);
				m_btnSendToHL7.ShowWindow(SW_HIDE);
				//hide hyperlinks
				m_lblCheckAll.EnableWindow(FALSE);
				m_lblCheckAll.ShowWindow(SW_HIDE);
				m_lblUncheckAll.EnableWindow(FALSE);
				m_lblUncheckAll.ShowWindow(SW_HIDE);
				//hide checkbox
				m_chkHidePatientResp.EnableWindow(FALSE);
				m_chkHidePatientResp.ShowWindow(SW_HIDE);

			} else {
				//setup hyperlinks to be clickable and assign text.
				m_lblCheckAll.SetType(dtsHyperlink);
				m_lblCheckAll.SetText("Check all for HL7");
				m_lblUncheckAll.SetType(dtsHyperlink);
				m_lblUncheckAll.SetText("Uncheck all for HL7");
			}
		}

		// (a.wilson 2013-06-10 15:37) - PLID 56784 - moved to its own function so we can requery when necessary.
		UpdateEMNBillsListFromClause();

		m_bCreated = TRUE;

		m_List->GetColumn(COLUMN_BILL_EMN)->PutForeColor(RGB(0,0,255));
		// (j.jones 2009-06-24 16:30) - PLID 24076 - added ability to link to existing bill
		m_List->GetColumn(COLUMN_LINK_EXISTING_BILL)->PutForeColor(RGB(0,0,255));

		// (j.dinatale 2012-01-11 16:59) - PLID 47483 - ability to assign charge resps
		m_List->GetColumn(COLUMN_ASSIGN_CHARGE_RESP)->PutForeColor(RGB(0,0,255));

		//Tell the mainframe that we want to know when the database changes.
		// (j.jones 2010-07-07 16:16) - PLID 36682 - not anymore, we do NOT want them
		//GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());
	}
	NxCatchAll("Error in CEMNToBeBilledDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMNToBeBilledDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMNToBeBilledDlg)
	ON_EVENT(CEMNToBeBilledDlg, IDC_UNBILLED_EMN_LIST, 19 /* LeftClick */, OnLeftClickUnbilledEmnList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMNToBeBilledDlg, IDC_UNBILLED_EMN_LIST, 6 /* RButtonDown */, OnRButtonDownUnbilledEmnList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMNToBeBilledDlg, IDC_UNBILLED_EMN_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUnbilledEmnList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

LRESULT CEMNToBeBilledDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if(message == NXM_POST_EDIT_BILL) {
		// (j.dinatale 2012-01-19 10:27) - PLID 47514 - unsubscribe from events, now that a refresh happened.
		GetMainFrame()->m_EMNBillController.UnsubscribeFromEvents(this);
		//requery the list so the just-saved bill gets removed
		Refresh();
	}

	// (j.dinatale 2012-01-20 16:01) - PLID 47514 - we want to unsubscribe if the user cancelled billing
	if(message == NXM_POST_CANCEL_BILL){
		GetMainFrame()->m_EMNBillController.UnsubscribeFromEvents(this);
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

// (j.jones 2008-06-05 12:55) - PLID 30255 - made this function require a row
void CEMNToBeBilledDlg::GoToPatient(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// (j.jones 2010-04-21 08:42) - PLID 38279 - this function will warn
	// if the bill is open already, restore if minimized, and return TRUE
	// if a bill is open
	// (a.walling 2010-04-26 15:59) - PLID 38364 - We are modeless now.
	/*
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return;
	}
	*/

	if(pRow == NULL) {
		return;
	}

	long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));

	if (nPatientID != -1) {
		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {			

			// (a.walling 2010-01-11 14:59) - PLID 31482
			if (m_pEMRPreviewPopupDlg) {
				m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
			}

			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView)
					pView->UpdateView();

				//we're done, so close this screen
				//CDialog::OnOK();
				return;
			}
		}//end if MainFrame
		else {
			MsgBox(MB_ICONSTOP|MB_OK, "ERROR - EMNToBeBilledDlg.cpp: Cannot Open Mainframe");
			return;
		}//end else pMainFrame
	}//end if nPatientID
}

// (a.walling 2010-04-26 13:01) - PLID 35921
void CEMNToBeBilledDlg::GoToEMN(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		long nID = VarLong(pRow->GetValue(COLUMN_EMN_ID));

		// need to get the PICID!
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT PicT.ID, EmnTabChartsLinkT.EmnTabChartID "
			"FROM PicT "
			"INNER JOIN EMRGroupsT ON PicT.EmrGroupID = EMRGroupsT.ID "
			"INNER JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"WHERE EMRMasterT.ID = {INT} "
			, nID);

		// (z.manning 2011-05-24 10:27) - PLID 33114 - We do not filter out EMNs in this dialog based on chart permissions.
		// However, we still must prevent users from opening EMNs they do not have access to.
		long nChartID = AdoFldLong(prs, "EmnTabChartID", -1);
		if(nChartID != -1) {
			if(!CheckCurrentUserPermissions(bioEmrCharts, sptView, TRUE, nChartID)) {
				return;
			}
		}
		
		long nPicID = -1;

		if (!prs->eof) {
			nPicID = AdoFldLong(prs, "ID", -1);
		}

		// (c.haag 2010-08-04 12:20) - PLID 39980 - This is acceptable now, albeit a very rare occurrence
		//if (nPicID == -1) {
		//	ThrowNxException("Could not find PIC record for the current EMN");
		//}
		prs->Close(); // No sense in leaving this open while the EMR is opened for editing.

		GetMainFrame()->EditEmrRecord(nPicID, nID);
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2008-06-05 12:55) - PLID 30255 - made this function require a row
void CEMNToBeBilledDlg::BillThisEMN(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		// (j.jones 2010-04-21 08:42) - PLID 38279 - do not check for the open bill twice,
		// as IsBillingModuleOpen already does this
		// (a.walling 2010-04-26 16:03) - PLID 38364 - Might as well check here before we
		// switch patients and all that.

		// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		if(pRow == NULL) {
			return;
		}

		long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));

		if (nPatientID != -1) {	

			if(!CheckCurrentUserPermissions(bioBill,sptCreate))
				return;			

			// (a.walling 2010-01-11 14:59) - PLID 31482
			if (m_pEMRPreviewPopupDlg) {
				m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
			}

			bool bBillPartials = false;

			// (j.dinatale 2012-01-17 08:53) - PLID 47514 - check if we need to bill partials. If we do, skip the context menu
			long nEMNID = (long)VarLong(pRow->GetValue(COLUMN_EMN_ID));
			if(GetMainFrame()) {
				bBillPartials = GetMainFrame()->m_EMNBillController.NeedToBillPartials(nEMNID);
			}

			// (j.jones 2006-02-03 11:14) - PLID 18735 - determined which responsibility to generate a bill for
			long nBillInsuredPartyID = -1;

			if(!bBillPartials){
				long nBillEMNTo = GetRemotePropertyInt("BillEMNTo", 0, 0, GetCurrentUserName(), true);
				//1 - patient responsibility
				//2 - primary responsibility if it exists
				//0 - pop-out a menu of options of available responsibilities

				// (j.jones 2011-12-16 15:39) - PLID 46289 - we now pop out the menu if they
				// select primary, but have more than one primary (medical and vision)
				BOOL bPopOutMenu = FALSE;

				if(nBillEMNTo == 1) {
					//patient responsibility
					nBillInsuredPartyID = -1;
				}
				else if(nBillEMNTo == 2) {
					//primary responsibility if it exists
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					// (j.jones 2011-12-16 16:43) - PLID 46289 - find all Primary resps, there could be Primary Medical or Vision, or both
					_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID FROM InsuredPartyT "
						"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
						"WHERE PatientID = {INT} AND RespTypeT.CategoryPlacement = 1", nPatientID);
					if(!rs->eof) {
						// (j.jones 2011-12-16 16:41) - PLID 46289 - if multiple records,
						// we will pop out the menu
						if(rs->GetRecordCount() > 1) {
							nBillInsuredPartyID = -1;
							bPopOutMenu = TRUE;
						}
						else {
							nBillInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
						}
					}
					else {
						nBillInsuredPartyID = -1;
					}
					rs->Close();
				}
				else {
					bPopOutMenu = TRUE;
				}

				if(bPopOutMenu) {		
					//pop-out a menu of options of available responsibilities

					//if there is no insurance, use patient

					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					// (j.jones 2011-12-16 16:43) - PLID 46289 - find all active resps
					_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID "
						"FROM InsuredPartyT "
						"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
						"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
						"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
						"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority", nPatientID);
					if(!rs->eof) {

						CMenu mnu;
						mnu.m_hMenu = CreatePopupMenu();
						long nIndex = 0;
						//add a line for patient
						mnu.InsertMenu(nIndex++, MF_BYPOSITION, -1, "For Patient Responsibility");

						//add a line for each responsibility
						while(!rs->eof) {

							long nInsPartyID = AdoFldLong(rs, "PersonID",-1);
							CString strInsCoName = AdoFldString(rs, "Name","");
							CString strRespTypeName = AdoFldString(rs, "TypeName","");
							CString strLabel;
							strLabel.Format("For %s (%s)", strInsCoName, strRespTypeName);

							mnu.InsertMenu(nIndex++, MF_BYPOSITION, nInsPartyID, strLabel);
							rs->MoveNext();
						}

						CPoint pt;
						GetCursorPos(&pt);
						nBillInsuredPartyID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , pt.x, pt.y, this, NULL);

						if(nBillInsuredPartyID == 0)
							return;
					}
					else {
						nBillInsuredPartyID = -1;
					}
					rs->Close();
				}
			}else{
				// (j.dinatale 2012-01-17 09:00) - PLID 47514 - if we need to bill partials... then dont do anything with the context menu
			}

			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView)
						pView->UpdateView();
				}
				else {
					return;
				}

			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - EMNToBeBilledDlg.cpp: Cannot Open Mainframe");
				return;
			}//end else pMainFrame

			//checks for any active global period, and warns accordingly
			// (a.walling 2008-07-07 18:04) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			if(!CheckWarnGlobalPeriod(nPatientID))
				return;

			//bill this EMR

			// (j.dinatale 2012-01-17 09:08) - PLID 47514 - Need to now go to our controller and ask it to bill accordingly
			if(GetMainFrame()) {
				// check if we are billing partials or not
				if(bBillPartials){
					// (j.dinatale 2012-01-27 14:50) - PLID 47514 - it is possible that an EMN is billed by someone else, and since this
					//	dialog doesnt refresh by table checker, we need to check.
					if(GetMainFrame()->m_EMNBillController.HasBeenFullyBilled(nEMNID)){
						MessageBox("This EMN has been fully billed already. It cannot be billed again.");
						return;
					}

					bool bHasUnassigned = false;
					
					// if we need to, check and see if we have to display the charge split dialog and if we have any unassigned charges
					bool bAlwaysShowChrgSplitDlg = (!!GetRemotePropertyInt("DisplayChargeSplitDlgAlways", 0, 0, "<None>", true));

					// (j.dinatale 2012-01-20 09:44) - PLID 47514 - need to ensure that our insurance is assigned correctly for charges that have yet to be billed
					if(GetMainFrame()->m_EMNBillController.HasInactiveInsuranceAssigned(nEMNID)){
						MessageBox("At least one unbilled EMN charge is assigned to a responsibility that is marked inactive. Please reassign those charges and then bill this EMN again.");
						bHasUnassigned = true;
					}

					if(!bHasUnassigned && !bAlwaysShowChrgSplitDlg){
						bHasUnassigned = GetMainFrame()->m_EMNBillController.HasUnassignedCharges(nEMNID);
					}

					if(bAlwaysShowChrgSplitDlg || bHasUnassigned){
						// (j.dinatale 2012-01-27 09:51) - PLID 47514 - if they canceled, just return
						if(IDCANCEL ==  ShowChargeRespDialog(nPatientID, nEMNID)){
							return;
						}

						// (j.dinatale 2012-01-20 09:44) - PLID 47514 - need to ensure that our insurance is assigned correctly for charges that have yet to be billed
						if(GetMainFrame()->m_EMNBillController.HasInactiveInsuranceAssigned(nEMNID)){
							MessageBox("At least one unbilled EMN charge is assigned to a responsibility that is marked inactive. Please reassign those charges and then bill this EMN again.");
							return;
						}

						// if the user hit cancel, or we still have charges that are not assigned, fail
						if(GetMainFrame()->m_EMNBillController.HasUnassignedCharges(nEMNID)){
							MessageBox("EMN could not be billed because all EMN charges must be assigned to a responsibility before this EMN can be billed.");
							return;
						}
					}

					// at this point we can try and bill partials, because all EMR charges are assigned
					if(!GetMainFrame()->m_EMNBillController.BillEMNToAssignedResps(nEMNID, nPatientID)){
						return;
					}

					// (j.dinatale 2012-01-19 10:27) - PLID 47514 - subscribe to events, since we are now the ones billing
					GetMainFrame()->m_EMNBillController.SubscribeToEvents(this);
				}else{
					// if not, bill everything under the insured party
					if(!GetMainFrame()->m_EMNBillController.BillEntireEMN(nEMNID, nPatientID, nBillInsuredPartyID)){
						return;
					}

					// (j.dinatale 2012-01-19 10:27) - PLID 47514 - subscribe to events, since we are now the ones billing
					GetMainFrame()->m_EMNBillController.SubscribeToEvents(this);
				}
			}

			/*// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
			if (GetMainFrame()->IsBillingModuleOpen(true)) {
				return;
			}

			CPatientView* pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
			if(!pView || !m_BillingDlg || !m_BillingDlg->GetSafeHwnd() || !IsWindow(m_BillingDlg->GetSafeHwnd())) {
				//if the patients module is closed, our billing pointer is definitely invalid,
				//so we must get it back!
				if(GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
					pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
					m_BillingDlg = pView->GetBillingDlg();
				}
			}
			if(m_BillingDlg) {
				m_BillingDlg->m_pFinancialDlg = this;
				m_BillingDlg->m_nPatientID = nPatientID;

				// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
				m_BillingDlg->OpenWithBillID(-1, 1, 1, TRUE);

				m_BillingDlg->PostMessage(NXM_BILL_EMR,
					(long)VarLong(pRow->GetValue(COLUMN_EMN_ID)),
					(long)nBillInsuredPartyID);
			}*/

			//do not close the screen, let it persist so they can bill again

		}//end if nPatientID

	}NxCatchAll("Error in CEMNToBeBilledDlg::BillThisEMN");
}

void CEMNToBeBilledDlg::OnOK() 
{
	// (j.jones 2010-04-21 08:42) - PLID 38279 - this function will warn
	// if the bill is open already, restore if minimized, and return TRUE
	// if a bill is open
	// (a.walling 2010-04-26 13:34) - PLID 38364 - Modeless now
	/*
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return;
	}
	
	CDialog::OnOK();
	*/
	CNxDialog::OnOK();
	DestroyWindow();
}

void CEMNToBeBilledDlg::OnCancel() 
{
	// (j.jones 2010-04-21 08:42) - PLID 38279 - this function will warn
	// if the bill is open already, restore if minimized, and return TRUE
	// if a bill is open
	// (a.walling 2010-04-26 13:34) - PLID 38364 - Modeless now
	/*
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return;
	}
	
	CDialog::OnCancel();
	*/
	CNxDialog::OnOK();
	DestroyWindow();
}

void CEMNToBeBilledDlg::Refresh()
{
	try {
		// (j.dinatale 2012-02-02 13:36) - PLID 47846 - if we dont have a value for the emn to be selected after a refresh,
		//		it is safe to attempt to get the current selection
		if(m_nEMNIDToSelAfterRef == -1){
			// Get the currently selected row
			long lEMNID = -1;
			IRowSettingsPtr pRow = m_List->GetCurSel();
			if(pRow) {
				lEMNID = VarLong(pRow->GetValue(COLUMN_EMN_ID), -1);
			}

			m_nEMNIDToSelAfterRef = lEMNID;
		}

		// Requery the list
		m_List->Requery();
	} NxCatchAllThrow(__FUNCTION__);
}

// (j.jones 2008-06-05 12:37) - PLID 30255 - converted to use a datalist2
void CEMNToBeBilledDlg::OnLeftClickUnbilledEmnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_List->PutCurSel(pRow);

		// (a.walling 2010-04-26 13:01) - PLID 35921
		if(nCol == COLUMN_PATIENT_NAME) {
			GoToPatient(pRow);
		} else if (nCol == COLUMN_DESCRIPTION) {
			// (a.walling 2010-04-26 13:01) - PLID 35921
			GoToEMN(pRow);
		} else if(nCol == COLUMN_BILL_EMN) {
			BillThisEMN(pRow);		
		} else if(nCol == COLUMN_LINK_EXISTING_BILL) {
			// (j.jones 2009-06-24 16:30) - PLID 24076 - added ability to link to existing bill
			LinkWithExistingBill(pRow);		
		} else if (nCol == COLUMN_PREVIEW) {
			// (a.walling 2010-01-11 12:53) - PLID 31482 - Open up the EMN preview

			long nPatID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));
			long nEMNID = VarLong(pRow->GetValue(COLUMN_EMN_ID));
			COleDateTime dtEmnModifiedDate = VarDateTime(pRow->GetValue(COLUMN_MODIFIED_DATE));

			ShowPreview(nPatID, nEMNID, dtEmnModifiedDate);
		} else if(nCol == COLUMN_ASSIGN_CHARGE_RESP){
			// (j.dinatale 2012-01-11 17:12) - PLID 47483 - open up the charge split dialog
			long nPatID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID), -1);
			long nEMNID = VarLong(pRow->GetValue(COLUMN_EMN_ID), -1);

			if(nEMNID != -1 && nPatID != -1){
				ShowChargeRespDialog(nPatID, nEMNID);
			}
		}

	}NxCatchAll("Error in CEMNToBeBilledDlg::OnLeftClickUnbilledEmnList");
}

// (j.jones 2008-06-05 12:37) - PLID 30255 - converted to use a datalist2
void CEMNToBeBilledDlg::OnRButtonDownUnbilledEmnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_List->PutCurSel(pRow);

		enum {

			miBillThisEMN = -1,
			miGoToPatient = -2,
			miGoToEMN = -3, // (a.walling 2010-04-26 13:01) - PLID 35921
			miLinkWithExisting = -4,
			miRemoveFromList = -5,	// (j.jones 2009-09-15 10:02) - PLID 34717
			miAssignChargeResp = -6, // (j.dinatale 2012-01-11 17:17) - PLID 47483
		};
	
		CMenu pMenu;
		pMenu.CreatePopupMenu();
		// (j.dinatale 2012-01-11 17:20) - PLID 47483 - need to be able to assign charge resps
		pMenu.InsertMenu(0, MF_BYPOSITION, miAssignChargeResp, "&Assign Charge Resps.");
		pMenu.InsertMenu(1, MF_BYPOSITION, miBillThisEMN, "&Bill this EMN");
		// (j.jones 2009-06-24 15:04) - PLID 24076 - added ability to link to existing bill
		pMenu.InsertMenu(2, MF_BYPOSITION, miLinkWithExisting, "&Link with an Existing Bill");
		// (j.jones 2009-09-15 10:02) - PLID 34717 - added ability to remove from the list
		pMenu.InsertMenu(3, MF_BYPOSITION, miRemoveFromList, "&Remove from List");
		pMenu.InsertMenu(4, MF_BYPOSITION|MF_SEPARATOR);
		pMenu.InsertMenu(5, MF_BYPOSITION, miGoToPatient, "&Go to Patient");
		pMenu.InsertMenu(6, MF_BYPOSITION, miGoToEMN, "Go to &EMN");
		CPoint pt;
		GetCursorPos(&pt);
		// (j.jones 2008-08-01 09:09) - PLID 30255 - force the menu to return its selection now,
		// so we can call the resulting function with the current row
		int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		switch(nCmdId) {

			case miGoToPatient:

				GoToPatient(pRow);
				break;

			// (a.walling 2010-04-26 13:01) - PLID 35921 - Go to EMN
			case miGoToEMN:

				GoToEMN(pRow);
				break;

			case miBillThisEMN:

				BillThisEMN(pRow);
				break;

			// (j.jones 2009-06-24 15:04) - PLID 24076 - added ability to link to existing bill
			case miLinkWithExisting:

				LinkWithExistingBill(pRow);			
				break;

			// (j.jones 2009-09-15 10:02) - PLID 34717 - added ability to remove from the list
			case miRemoveFromList:

				RemoveEMNFromList(pRow);
				break;

			// (j.dinatale 2012-01-11 17:21) - PLID 47483 - be able to assign charge resps
			case miAssignChargeResp:
				long nPatID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID), -1);
				long nEMNID = VarLong(pRow->GetValue(COLUMN_EMN_ID), -1);

				if(nEMNID != -1 && nPatID != -1){
					ShowChargeRespDialog(nPatID, nEMNID);
				}
				break;
		}
		pMenu.DestroyMenu();

	}NxCatchAll("Error in CEMNToBeBilledDlg::OnRButtonDownUnbilledEmnList");
}

// (j.jones 2008-06-05 12:37) - PLID 30255 - converted to use a datalist2
void CEMNToBeBilledDlg::OnRequeryFinishedUnbilledEmnList(short nFlags) 
{
	try {
		// (j.dinatale 2012-02-02 13:36) - PLID 47846 - if we canceled, dont bother doing anything
		if((nFlags & dlRequeryFinishedCanceled) != 0){
			return;
		}
		//since requerying can change the colors of the rows, ensure
		//these columns remain blue

		IColumnSettingsPtr pCol = m_List->GetColumn(COLUMN_BILL_EMN);
		pCol->PutForeColor(RGB(0,0,255));
		// (j.jones 2009-06-24 16:30) - PLID 24076 - added ability to link to existing bill
		m_List->GetColumn(COLUMN_LINK_EXISTING_BILL)->PutForeColor(RGB(0,0,255));

		// (j.dinatale 2012-02-02 10:19) - PLID 47846 - find the row, then ensure it is in view, then reset our internal variable
		// Reselect the previously selected row
		if (m_nEMNIDToSelAfterRef != -1){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_List->FindByColumn(COLUMN_EMN_ID, m_nEMNIDToSelAfterRef, NULL, TRUE);
			if(pRow){
				m_List->EnsureRowInView(pRow);
			}
		}

		m_nEMNIDToSelAfterRef = -1;

	}NxCatchAll("Error in CEMNToBeBilledDlg::OnRequeryFinishedUnbilledEmnList");
}

// (j.jones 2008-07-03 15:48) - PLID 18354 - added ability to preview this screen
void CEMNToBeBilledDlg::OnBtnEmnToBeBilledPreview() 
{
	try {	

		// (a.walling 2010-01-11 14:59) - PLID 31482
		if (m_pEMRPreviewPopupDlg) {
			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(631)]);

		CPtrArray params;
		CRParameterInfo *tmpParam;

		tmpParam = new CRParameterInfo;
		tmpParam->m_Data = GetCurrentUserName();
		tmpParam->m_Name = "CurrentUserName";
		params.Add((void *)tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateTo";
		tmpParam->m_Data = "12/31/5000";
		params.Add((void *)tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateFrom";
		tmpParam->m_Data = "01/01/1000";
		params.Add((void *)tmpParam);

		// (j.jones 2009-09-15 10:58) - PLID 34717 - the regular version of this report
		//only filters on whether the EMN has or has not been billed, but this preview
		//needs to also filter out which EMNs have been cleared
		//send -2 to indicate this specific filter
		// (a.wilson 2013-6-12) PLID 56761 - notify that the query should 
		//filter out all patient resp. emn bills when -3 is passed.
		if (!(m_chkHidePatientResp.GetCheck() == BST_CHECKED)) {
			infReport.nExtraID = -2;
		} else {
			infReport.nExtraID = -3;
		}

		RunReport(&infReport, &params, TRUE, this, "EMNs With Charges");
		ClearRPIParameterList(&params);

		//close the screen

		// (j.jones 2011-07-29 12:14) - PLID 39574 - because this is modeless,
		// and the report is also modeless, the user could potentially close
		// this dialog before the report finishes, which would cause this 
		// ShowWindow call to throw assertions
		if(GetSafeHwnd() && IsWindowVisible()) {
			ShowWindow(SW_HIDE);
		}

	}NxCatchAll("Error in CEMNToBeBilledDlg::OnBtnEmnToBeBilledPreview");
}

// (j.jones 2009-06-24 15:04) - PLID 24076 - added ability to link to existing bill
void CEMNToBeBilledDlg::LinkWithExistingBill(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		// (j.jones 2010-04-21 08:42) - PLID 38279 - this function will warn
		// if the bill is open already, restore if minimized, and return TRUE
		// if a bill is open
		// (a.walling 2010-04-26 14:20) - PLID 38364 - Now modeless
		/*
		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}
		*/

		if(pRow == NULL) {
			return;
		}

		long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));

		if (nPatientID != -1) {	

			//technically this isn't writing to the bill, but we should make sure
			//that only someone with the ability to edit a bill can do this
			if(!CheckCurrentUserPermissions(bioBill,sptWrite)) {
				return;
			}

			// (j.dinatale 2012-01-18 12:00) - PLID 47621 - Prevent linking to a bill if for some reason we need to do partial billing
			if(GetMainFrame()) {
				long nEMNID = (long)VarLong(pRow->GetValue(COLUMN_EMN_ID));
				BOOL bRequireEMRChargeResps = GetRemotePropertyInt("RequireEMRChargeResps", 0, 0, "<None>", true);

				// if charge assignment is required, state such
				if(bRequireEMRChargeResps){
					MessageBox("EMR charges must be linked to a responsibility which may require multiple bills to be created. This EMN cannot be linked to a bill. If this EMN has been billed previously, right click and remove it from the list.",
						"Nextech Practice", MB_ICONEXCLAMATION);
					return;
				}else{
					// otherwise, check and see if we need partial bills because charges can still be assigned to resps.
					if(GetMainFrame()->m_EMNBillController.NeedToBillPartials(nEMNID)){
						MessageBox("Some EMR charges are assigned to a responsibility which may require multiple bills to be created. If you wish to link this EMN to a bill, unassign responsibilities from this EMN's charges otherwise "
							"this EMN cannot be linked to a bill. If this EMN has been billed previously, right click and remove it from the list.",
							"Nextech Practice", MB_ICONEXCLAMATION);
						return;
					}
				}
			}

			//do not open the dialog if they have no bills at all
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillsT.ID FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
				"AND BillsT.PatientID = {INT} ", nPatientID);
			if(rs->eof) {
				AfxMessageBox("This patient has no bills on their account.");
				return;
			}
			rs->Close();
			
			// (a.walling 2010-01-11 14:59) - PLID 31482
			if (m_pEMRPreviewPopupDlg) {
				m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
			}

			CLinkEMNToBillDlg dlg(this);
			dlg.m_nPatientID = nPatientID;
			dlg.m_nEMNID = VarLong(pRow->GetValue(COLUMN_EMN_ID));
			dlg.m_strEMNDesc = VarString(pRow->GetValue(COLUMN_DESCRIPTION));
			dlg.m_dtEMNDate = VarDateTime(pRow->GetValue(COLUMN_DATE));
			dlg.m_strPatientName = VarString(pRow->GetValue(COLUMN_PATIENT_NAME));
			if(IDOK == dlg.DoModal()) {
				//if the user was able to click OK on this dialog,
				//then we linked the EMN to a bill, and can now
				//remove it from the list
				m_List->RemoveRow(pRow);
			}
		}

	}NxCatchAll("Error in CEMNToBeBilledDlg::LinkWithExistingBill");
}

// (j.jones 2009-09-15 10:02) - PLID 34717 - added ability to remove from the list
void CEMNToBeBilledDlg::RemoveEMNFromList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		if(pRow == NULL) {
			return;
		}

		long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));

		if (nPatientID != -1) {	

			//this isn't actually writing to either an EMR or a bill, but as this is
			//a task for the biller, we should make sure that only someone with the
			//ability to edit bills can make this change
			if(!CheckCurrentUserPermissions(bioBill, sptWrite)) {
				return;
			}

			// (r.gonet 04/16/2014) - PLID 40220 - Updated the text from More Info topic to Codes topic
			if(IDYES == MessageBox("Removing this EMN will remove it from this list permanently, without linking to any bill.\n"
				"You may still bill this EMN from the <Codes> topic of the EMN, or when creating a new bill, but the EMN will no longer be displayed in this list.\n\n"
				"Are you sure you wish to remove this EMN?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

				//ok, mark it as cleared
				long nEMNID = VarLong(pRow->GetValue(COLUMN_EMN_ID));

				// (j.jones 2011-07-06 14:46) - PLID 44464 - if someone else is clearing the list at the same time,
				// and already removed this record, don't insert twice, just remove the row
				if(!ReturnsRecordsParam("SELECT EMNID FROM EMNToBeBilled_ClearedRecordsT WHERE EMNID = {INT}", nEMNID)) {

					ExecuteParamSql("INSERT INTO EMNToBeBilled_ClearedRecordsT (EMNID) VALUES ({INT})", nEMNID);
					
					//audit the removal
					long nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) {
						CString strOldValue;
						strOldValue.Format("EMN: %s", VarString(pRow->GetValue(COLUMN_DESCRIPTION)));
						AuditEvent(VarLong(pRow->GetValue(COLUMN_PATIENT_ID)), VarString(pRow->GetValue(COLUMN_PATIENT_NAME)), nAuditID, aeiEMNToBeBilled_Cleared, nEMNID, strOldValue, "<Removed From 'EMNs To Be Billed' List>", aepMedium, aetDeleted);
					}
				}

				//remove it from the list
				m_List->RemoveRow(pRow);
			}
		}

	}NxCatchAll("Error in CEMNToBeBilledDlg::RemoveEMNFromList");
}

void CEMNToBeBilledDlg::OnDestroy()
{
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	if (m_pEMRPreviewPopupDlg) {
		m_pEMRPreviewPopupDlg->DestroyWindow();
		delete m_pEMRPreviewPopupDlg;
		m_pEMRPreviewPopupDlg = NULL;
	}

	if (m_hIconPreview) {
		DestroyIcon(m_hIconPreview);
	}

	m_BillingDlg = NULL;

	m_bShowState = false;

	// (j.dinatale 2012-01-19 10:30) - PLID 47514 - we need to ensure that we have unsubscribed from billing events
	if(GetMainFrame()){
		GetMainFrame()->m_EMNBillController.UnsubscribeFromEvents(this);
	}

	CNxDialog::OnDestroy();
}

// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
// (z.manning 2012-09-10 15:22) - PLID 52543 - Added modified date
void CEMNToBeBilledDlg::ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate)
{
	if (nPatID == -1 || nEMNID == -1) {
		return;
	}

	if (m_pEMRPreviewPopupDlg == NULL) {
		// create the dialog!

		// (a.walling 2007-04-13 09:49) - PLID 25648 - Load and initialize our preview popup
		m_pEMRPreviewPopupDlg = new CEMRPreviewPopupDlg(this);
		m_pEMRPreviewPopupDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

		// (a.walling 2010-01-11 12:37) - PLID 31482
		m_pEMRPreviewPopupDlg->RestoreSize("EMNToBeBilled");
	}
	
	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, but since we haven't opened the dialog yet,
	// we can pass in an empty array.
	// (z.manning 2012-09-10 15:23) - PLID 52543 - Use the new EmnPreviewPopup struct
	EmnPreviewPopup emn(nEMNID, dtEmnModifiedDate);
	m_pEMRPreviewPopupDlg->SetPatientID(nPatID, emn);
	m_pEMRPreviewPopupDlg->PreviewEMN(emn, 0);

	// (a.walling 2010-01-11 16:20) - PLID 27733 - Only show if it is not already
	if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
		m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
	}
}

void CEMNToBeBilledDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {

		if (bShow) {
			bool bPreviousState = m_bShowState;
			m_bShowState = true;

			// (j.jones 2010-07-07 16:46) - PLID 36682 - always refresh if not previously shown
			if (!bPreviousState) {
				try {
					Refresh();	
				} NxCatchAll("Error refreshing CEMNToBeBilledDlg");
			}
		} else {		
			m_bShowState = false;

			if (m_pEMRPreviewPopupDlg) {
				m_pEMRPreviewPopupDlg->DestroyWindow();
				delete m_pEMRPreviewPopupDlg;
				m_pEMRPreviewPopupDlg = NULL;
			}
		}
	} NxCatchAll(__FUNCTION__);
	CNxDialog::OnShowWindow(bShow, nStatus);
}

// (j.jones 2010-07-07 15:22) - PLID 36682 - added a refresh button
void CEMNToBeBilledDlg::OnBtnEmnToBeBilledRefresh()
{
	try {

		Refresh();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-07 15:50) - PLID 36682 - track when the window is sized
void CEMNToBeBilledDlg::OnSize(UINT nType, int cx, int cy)
{
	try {

		//if this window is ever made resizeable, this code will have
		//to be changed, it is intended to refresh the list only when it
		//is restored from being minimized

		// (j.jones 2010-07-07 16:06) - PLID 36682 - reload on resize, which
		// currently can only occur on minimize or restore, so only do it on restore
		// (but check m_bCreated first, as this code is hit before OnInitDialog) 
		if(m_bShowState && nType != SIZE_MINIMIZED && m_bCreated && cx > 0 && cy > 0) {
			Refresh();
		}

		CNxDialog::OnSize(nType, cx, cy);

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-01-11 17:36) - PLID 47483 - show the charge split dialog
// returns the result from the dialog it creates
long CEMNToBeBilledDlg::ShowChargeRespDialog(long nPatID, long nEMNID)
{
	CArray<long, long> aryCharges;
	try{
		CMap<long, long, long, long> mapChargeToOldInsResp;

		// get the basic charge information to fill our dialog
		// (j.dinatale 2012-01-18 10:01) - PLID 47483 - need to allow for products to be assigned resps
		_RecordsetPtr rsCharges = CreateParamRecordset(
			"SELECT EMRChargesT.ID, EMRChargesT.Description, "
			"Quantity, UnitCost, CPTCodeT.Code, CPTCodeT.SubCode, CAST(COALESCE(CPTCodeT.Billable, 1) AS BIT) AS Billable, "
			"EMRChargeRespT.EMRChargeID AS RespChargeID, EMRChargeRespT.InsuredPartyID AS InsuredPartyID "
			"FROM EMRChargesT "
			"LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
			"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
			"WHERE EMRChargesT.Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND EMRID = {INT}; ", nEMNID);

		while(!rsCharges->eof){
			EMNChargeSummary *pCharge = new EMNChargeSummary();

			pCharge->nID = AdoFldLong(rsCharges, "ID");
			pCharge->strDescription = AdoFldString(rsCharges, "Description","");
			pCharge->dblQuantity = AdoFldDouble(rsCharges, "Quantity",0.0);
			pCharge->cyUnitCost = AdoFldCurrency(rsCharges, "UnitCost",COleCurrency(0,0));
			pCharge->strSubCode = AdoFldString(rsCharges, "SubCode","");
			pCharge->strCode = AdoFldString(rsCharges, "Code","");
			pCharge->bBillable = AdoFldBool(rsCharges, "Billable", TRUE);

			if(AdoFldLong(rsCharges, "RespChargeID", -1) != -1){
				// patient resp (if InsuredPartyID is null) or other resp
				pCharge->nInsuredPartyID = AdoFldLong(rsCharges, "InsuredPartyID", -1);
			}else{
				// unassigned
				pCharge->nInsuredPartyID = -2;
			}

			aryCharges.Add((long)pCharge);
			mapChargeToOldInsResp.SetAt((long)pCharge, pCharge->nInsuredPartyID); 
			rsCharges->MoveNext();
		}

		// create the dialog and show it
		long nSize = aryCharges.GetSize();
		CChargeSplitDlg dlg(ChrgSptDlgEnums::EMNChargeSummary, aryCharges, nPatID, nEMNID, FALSE);
		long nResult = dlg.DoModal();

		// if changes were actually made
		if(nResult == IDOK && dlg.ChargesChanged()){
			CSqlFragment sqlFrag;
			bool bChangesMade = false;
			CMap<long, long, CString, CString> mapInsPartyIDtoName;

			// (j.dinatale 2012-01-27 10:19) - PLID 47483 - switched to the AuditTransaction object
			CAuditTransaction auditTrans;

			mapInsPartyIDtoName.SetAt(-2, "< Unassigned >");
			mapInsPartyIDtoName.SetAt(-1, "Patient Resp.");
			
			// cache insurance name info so that way we dont hit data multiple times and at this time we know that something changed, so may as well
			_RecordsetPtr rsInsInfo = CreateParamRecordset(GetRemoteData(),
				"SELECT InsuredPartyT.PersonID AS InsuredPartyID, InsuranceCoT.Name AS Name, RespTypeT.TypeName AS TypeName "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE InsuredPartyT.PatientID = {INT} ", nPatID);

			while(!rsInsInfo->eof){
				long nInsPartyID = AdoFldLong(rsInsInfo, "InsuredPartyID", -1);

				if(nInsPartyID != -1){
					CString strName;
					strName.Format("%s (%s)", AdoFldString(rsInsInfo, "Name", ""), AdoFldString(rsInsInfo, "TypeName", ""));

					mapInsPartyIDtoName.SetAt(nInsPartyID, strName);
				}

				rsInsInfo->MoveNext();
			}

			for(int i = 0; i < nSize; i++){
				EMNChargeSummary *pCharge = (EMNChargeSummary *)aryCharges.GetAt(i);

				if(pCharge){
					// save code here
					long nOldInsPartyID;

					// check our map of old insured party IDs
					if(mapChargeToOldInsResp.Lookup((long)pCharge, nOldInsPartyID)){
						// if the insured party did change, add sql to our fragment
						if(pCharge->nInsuredPartyID != nOldInsPartyID){
							bChangesMade = true;
							if(pCharge->nInsuredPartyID == -2){
								sqlFrag += CSqlFragment("DELETE FROM EMRChargeRespT WHERE EMRChargeID = {INT} \r\n", pCharge->nID);
							}else{
								_variant_t vtValue = (pCharge->nInsuredPartyID == -1) ? g_cvarNull : _variant_t(pCharge->nInsuredPartyID);
								sqlFrag += CSqlFragment(
									"IF EXISTS(SELECT TOP 1 1 FROM EMRChargeRespT WHERE EMRChargeID = {INT}) BEGIN "
									"	UPDATE EMRChargeRespT SET InsuredPartyID = {VT_I4} WHERE EMRChargeID = {INT} "
									"END "
									"ELSE BEGIN "
									"	INSERT INTO EMRChargeRespT (EMRChargeID, InsuredPartyID) VALUES ({INT}, {VT_I4}) "
									"END ", pCharge->nID, vtValue, pCharge->nID, pCharge->nID, vtValue);
							}

							// add auditing
							// (j.dinatale 2012-01-27 10:19) - PLID 47483 - switched to the AuditTransaction object
							CString strOldID, strNewID;
							if(!mapInsPartyIDtoName.Lookup(nOldInsPartyID, strOldID)){
								strOldID = "< Unknown >";
							}

							if(!mapInsPartyIDtoName.Lookup(pCharge->nInsuredPartyID, strNewID)){
								strNewID = "< Unknown >";
							}

							AuditEvent(nPatID, GetExistingPatientName(nPatID), auditTrans, 
								aeiEMNServiceCodeAssignedRespChanged, nEMNID, strOldID, strNewID, aepMedium, aetChanged);
						}
					}
				}
			}

			if(bChangesMade){
				// if we found changes, fire away captain!
				ExecuteParamSql(GetRemoteData(), "BEGIN TRAN \r\n {SQL} \r\n COMMIT TRAN", sqlFrag);

				// (j.dinatale 2012-01-27 10:19) - PLID 47483 - switched to the AuditTransaction object
				auditTrans.Commit();

				// (a.wilson 2013-06-10 16:52) - PLID 57117- requery the list if hide patient is checked in case they set all to patient resp.
				if (m_chkHidePatientResp.GetCheck() == BST_CHECKED) {
					m_List->Requery();
				}
			}
		}

		// clean up, clean up, everybody do your share...
		mapChargeToOldInsResp.RemoveAll();
		for(int i = 0; i < nSize; i++){
			EMNChargeSummary *pCharge = (EMNChargeSummary *)aryCharges.GetAt(0);
			delete pCharge;
			aryCharges.RemoveAt(0);
		}
		aryCharges.RemoveAll();

		return nResult;
	}NxCatchAll(__FUNCTION__);

	// (j.dinatale 2012-01-27 10:59) - PLID 47483 - attempt to clean up memory
	try{
		int nSize = aryCharges.GetSize();
		for(int i = 0; i < nSize; i++){
			EMNChargeSummary *pCharge = (EMNChargeSummary *)aryCharges.GetAt(0);
			delete pCharge;
			aryCharges.RemoveAt(0);
		}
		aryCharges.RemoveAll();
	}NxCatchAll("Error in CEMNToBeBilledDlg::ShowChargeRespDialog. Failed to clean-up memory.");

	return 0;
}

// (a.wilson 2013-05-23 15:27) - PLID 56784 - send all checked off emn charges to hl7.
void CEMNToBeBilledDlg::OnBnClickedBtnSendBillToHl7()
{
	try {
		CArray<HL7EMNCharges> aryEMNCharges;

		//get each emn id for each row marked to send to hl7
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_List->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (AsBool(pRow->GetValue(COLUMN_SEND_TO_HL7))) {
				aryEMNCharges.Add(HL7EMNCharges(
					AsLong(pRow->GetValue(COLUMN_EMN_ID)), 
					AsString(pRow->GetValue(COLUMN_DESCRIPTION)),
					AsString(pRow->GetValue(COLUMN_PATIENT_NAME)),
					AsDateNoTime(pRow->GetValue(COLUMN_DATE))));
				//uncheck all checked rows.
				pRow->PutValue(COLUMN_SEND_TO_HL7, g_cvarFalse);
			}
		}
		//warn about nothing being selected.
		if (aryEMNCharges.GetCount() <= 0) {
			MessageBox("You must select at least one EMN Bill to Send to HL7.", "Select an EMN Bill", MB_OK);
			return;
		}

		CArray<HL7SettingsGroup,HL7SettingsGroup&> aryDefaultGroups;
		CArray<long, long> aryDefaultGroupIDs;
		//get hl7 group settings that export emn bills.
		GetHL7SettingsGroupsBySetting("ExportEmnBills", TRUE, aryDefaultGroupIDs);

		//check to make sure we have at least one hl7 group. this should never happen but just in case someone change the setup after we opened.
		if (aryDefaultGroupIDs.GetCount() <= 0) {
			MessageBox("There are no HL7 group settings that export EMN bills to HL7.  Setup an HL7 group to send bills to HL7.", 
				"No HL7 Groups Export EMN Bills to HL7", MB_OK);
			return;
		}

		for (int i = 0; i < aryDefaultGroupIDs.GetSize(); i++) {
			long nGroupID = aryDefaultGroupIDs[i];
			HL7SettingsGroup hsg;
			hsg.nID = nGroupID;
			hsg.nExportType = GetHL7SettingInt(nGroupID, "ExportType");
			hsg.bExpectAck = GetHL7SettingBit(nGroupID, "ExpectAck");
			hsg.strName = GetHL7GroupName(nGroupID);
			aryDefaultGroups.Add(hsg);
		}
		//attempt to send emn charges to hl7. catalog any failures to address to the user.
		CString strSendingErrorMessage;
		CArray<long> arySuccessfulEMNIDs;
		//go through each emn charge first so we can check if it succeeded for each group.
		for (long i = 0; i < aryEMNCharges.GetSize(); i++) {
			HL7EMNCharges emnCharge = aryEMNCharges.GetAt(i);
			long nFailed = 0;
			bool bAckFailure = false;
			//iterate through each group and send the emn charge off.
			for (long j = 0; j < aryDefaultGroups.GetSize(); j++) {
				HL7SettingsGroup hsg = aryDefaultGroups[j];
				BOOL bExpectAck = (hsg.nExportType == 1 && hsg.bExpectAck);
				
				HL7ResponsePtr pResponse = m_pHL7Client->SendNewEmnBillHL7Message(emnCharge.nEMNID, hsg.nID, false);
				//check the response to see if the message failed.
				if (pResponse->hmssSendStatus == hmssFailure_Batched || 
					pResponse->hmssSendStatus == hmssFailure_NotBatched) {
					nFailed++;
				//if the failure was an ack failure then we need to handle that special case.
				} else if (pResponse->hmssSendStatus == hmssSent_AckFailure) {
					bAckFailure = true;
					nFailed++;
				}
			}
			//if this message failed atleast once then we need to alert the user of the occcurence.
			if (nFailed > 0) {
				if (strSendingErrorMessage.IsEmpty()) {
					strSendingErrorMessage = ("The following EMN Bills had issues sending to HL7:");
				}
				strSendingErrorMessage += FormatString("\r\n- Patient %s for EMN %s on %s, ", 
					emnCharge.strPatientName, emnCharge.strEMNDescription, emnCharge.dtEMNDate.Format("%x"));
				//check to see if the message failed for all possible groups or only a few. 
				//if all failed then warn the user and prevent saving as success.
				if (nFailed == aryDefaultGroups.GetSize()) {
					strSendingErrorMessage += FormatString("all HL7 exports failed. ");
				} else {
					strSendingErrorMessage += FormatString("atleast one HL7 export failed. ");
					//only mark successful if atleast one export succeeded.
					arySuccessfulEMNIDs.Add(emnCharge.nEMNID);
				}
				//if the failure was because we received no ack then alert the user of this.
				if (bAckFailure) {
					strSendingErrorMessage += "One or more failed because Practice did not get an acknowledgement back from the receiver.";
				} else {
					strSendingErrorMessage += "One or more failed because the HL7 group settings may be incorrect.";
				}
			} else {
				//only mark successful if atleast one export succeeded.
				arySuccessfulEMNIDs.Add(emnCharge.nEMNID);
			}
		}
		// (a.wilson 2013-06-11 14:14) - PLID 57117 - save the emns as sent to hl7 that had atleast one success at sending to hl7.
		if (arySuccessfulEMNIDs.GetCount() > 0) {
			MarkHL7EMNBillsAsSent(arySuccessfulEMNIDs);
		}

		//if there were any issues sending messages then show them. we already saved to data to prevent holding up commits.
		if (!strSendingErrorMessage.IsEmpty()) {
			//final explanation of what to expect from the export.
			// (r.gonet 04/16/2014) - PLID 40220 - Updated the text from More Info topic to Codes topic
			strSendingErrorMessage += ("\r\n\r\nAny EMN Bills that did not send successfully at least once will remain in the list. "
				"The ones that did send to HL7 at least once were removed from the list.  In order to send them again you can either "
				"go to the EMN's <Codes> topic or go to Links Module->HL7 and use the Export Bills Screen.");
			MessageBox(strSendingErrorMessage, "Errors While Sending to HL7", MB_OK);
		} else {
			MessageBox("All selections sent to HL7 successfully.", "Sending to HL7 Successful", MB_OK);
		}

		//finally remove any rows that were sent to hl7.
		for (int i = 0; i < arySuccessfulEMNIDs.GetCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRemoveRow = m_List->FindByColumn(COLUMN_EMN_ID, arySuccessfulEMNIDs.GetAt(i), NULL, VARIANT_FALSE);
			if (pRemoveRow) {
				m_List->RemoveRow(pRemoveRow);
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-06-11 12:10) - PLID 56784 - turns cursor to hand when over hyperlink labels.
BOOL CEMNToBeBilledDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcCheckAllLink;
		GetDlgItem(IDC_CHECK_ALL_FOR_HL7)->GetWindowRect(rcCheckAllLink);
		ScreenToClient(&rcCheckAllLink);
		
		CRect rcUncheckAllLink;
		GetDlgItem(IDC_UNCHECK_ALL_FOR_HL7)->GetWindowRect(rcUncheckAllLink);
		ScreenToClient(&rcUncheckAllLink);

		if ((rcCheckAllLink.PtInRect(pt) && m_lblCheckAll.GetType() == dtsHyperlink) 
			|| (rcUncheckAllLink.PtInRect(pt) && m_lblUncheckAll.GetType() == dtsHyperlink) ) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
// (a.wilson 2013-06-11 12:10) - PLID 56784 - when clicked either checks or unchecks all for sending to hl7.
LRESULT CEMNToBeBilledDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		bool bCheckAll = false;

		//run based on what hyperlink was clicked.
		switch ((UINT)wParam) {
			case IDC_CHECK_ALL_FOR_HL7:
					bCheckAll = true;

			case IDC_UNCHECK_ALL_FOR_HL7:
				{
					for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_List->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) 
					{
						pRow->PutValue(COLUMN_SEND_TO_HL7, (bCheckAll ? g_cvarTrue : g_cvarFalse));
					}
				}
				break;
			default:
				//this should not happen.
				ASSERT(FALSE);
				break;
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (a.wilson 2013-06-11 12:11) - PLID 56761 - moved code here so that we can filter query if they have checkbox checked to hide patient resp.
void CEMNToBeBilledDlg::OnBnClickedHidePatientRespCheck()
{
	try {
		//need to requery the list based on show insurance check.
		UpdateEMNBillsListFromClause(true);
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 2013-06-11 14:20) - PLID 56761 - requery the list of EMN Bills on certain occasions.
void CEMNToBeBilledDlg::UpdateEMNBillsListFromClause(bool bForceRequery /* = false */)
{
	bool bHidePatientBills = (m_chkHidePatientResp.GetCheck() == BST_CHECKED ? true : false);

	// (j.jones 2008-06-05 12:38) - PLID 30255 - added Color, to determine if the EMN has been quoted
	// (j.jones 2009-09-15 10:32) - PLID 34717 - now we ignore EMNs tracked in EMNToBeBilled_ClearedRecordsT
	// (a.walling 2010-04-26 14:12) - PLID 35921 - Include status
	// (j.jones 2011-03-28 15:03) - PLID 42575 - exclude unbillable CPT codes
	// (j.jones 2011-07-06 09:14) - PLID 44432- added StatusName
	// (j.jones 2011-07-06 14:44) - PLID 44464 - we should not have grouped by CPT code
	// (j.dinatale 2012-01-23 17:37) - PLID 47699 - we now need to handle partial billing, so if the EMN ID is in subquery, it needs to be billed
	// (j.dinatale 2012-01-25 10:31) - PLID 47699 - need to ignore EMRChargesT.Deleted, just because they were deleted, doesnt mean the EMN wasnt billed
	// (j.dinatale 2012-01-27 09:43) - PLID 47699 - Fixed this query, there were some weird ways of still getting EMNs to show up when they shouldnt
	// (j.jones 2012-01-27 16:25) - PLID 47699 - the BilledEMNsQ join in the first union has to return records if the EMN has been billed and has unassigned
	// charges, and return records if the EMN has been billed to a resp and has assigned charges
	// (z.manning 2012-09-10 15:26) - PLID 52543 - Added modified date
	// (a.wilson 2013-05-28 17:03) - PLID 56761 - change the from clause based on the checkbox to hide patient resp. bills.
	// (a.wilson 2013-06-11 14:17) - PLID 57117 - do not display bills that have already been sent to hl7 from more info, export bills dialog, or emns to be billed.
	CString strFrom;
	strFrom.Format("(SELECT EMRMasterT.PatientID, EMRMasterT.ID AS EMNID, Date, EMRMasterT.Description, "
		"Last + ', ' + First + ' ' + Middle AS PatName, Sum(Round(Convert(money, Quantity * UnitCost), 2)) AS EstimatedCost, "
		"CASE WHEN Sum(CASE WHEN QuotedChargesQ.ChargeID Is Null THEN 0 ELSE 1 END) = Count(EMRChargesT.ID)	THEN %li "
		"WHEN Sum(CASE WHEN QuotedChargesQ.ChargeID Is Null THEN 0 ELSE 1 END) > 0 THEN %li "
		"ELSE %li END AS Color, "
		"EMRStatusListT.Name AS StatusName, EmrMasterT.ModifiedDate "
		"FROM EMRMasterT INNER JOIN PersonT ON EMRMasterT.PatientID = PersonT.ID "
		"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
		"INNER JOIN EMRChargesT ON EMRMasterT.ID = EMRChargesT.EMRID "
		"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"LEFT JOIN EMNBillsSentToHL7T ON EMRMasterT.ID = EMNBillsSentToHL7T.EMNID "
		"LEFT JOIN (SELECT Min(EMRQuotedChargesT.ChargeID) AS ChargeID, EMRQuotedChargesT.EMRChargeID "
		"	FROM EMRQuotedChargesT "
		"	INNER JOIN LineItemT ON EMRQuotedChargesT.ChargeID = LineItemT.ID "
		"	WHERE LineItemT.Deleted = 0 "
		"	GROUP BY EMRQuotedChargesT.EMRChargeID) AS QuotedChargesQ ON EMRChargesT.ID = QuotedChargesQ.EMRChargeID "
		"WHERE EMRChargesT.Deleted = 0 AND EMRMasterT.Deleted = 0 "
		"AND EMRMasterT.ID NOT IN (SELECT EMNID FROM EMNToBeBilled_ClearedRecordsT) "
		/*"AND EMRMasterT.ID NOT IN (SELECT EMNID FROM BilledEMNsT "
		"	WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) "*/
		"AND EMRMasterT.ID IN ( "
		"	SELECT DISTINCT EMRChargesT.EMRID FROM "
		"	EMRChargesT "
		"	LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"	LEFT JOIN "
		"	( "
		"		SELECT BillsT.Deleted, BilledEMNsT.* FROM BilledEMNsT "
		"		LEFT JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
		"		LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
		"		WHERE BillCorrectionsT.OriginalBillID IS NULL AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
		"	) BilledEMNsQ ON BilledEMNsQ.EMNID = EMRChargesT.EMRID "
		"		AND (EMRChargeRespT.EMRChargeID Is Null OR COALESCE(EMRChargeRespT.InsuredPartyID, -1) = COALESCE(BilledEMNsQ.InsuredPartyID, -1)) "
		"	WHERE "
		"	(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
		"	AND EMRChargesT.Deleted = 0 "
		"	AND BilledEMNsQ.BillID IS NULL "
		"	AND EMRChargesT.EMRID IN (SELECT EMRID FROM EMRChargesT INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID) "
		"	UNION "
		"	SELECT DISTINCT EMRChargesT.EMRID FROM "
		"	EMRChargesT "
		"	LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"	LEFT JOIN "
		"	( "
		"		SELECT BillsT.Deleted, BilledEMNsT.* FROM BilledEMNsT "
		"		LEFT JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
		"		LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
		"		WHERE BillCorrectionsT.OriginalBillID IS NULL AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
		"	) BilledEMNsT ON BilledEMNsT.EMNID = EMRChargesT.EMRID "
		"	WHERE "
		"	(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
		"	AND EMRChargesT.Deleted = 0 "
		"	AND EMRChargesT.EMRID NOT IN (SELECT EMRID FROM EMRChargesT INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID) "
		"	AND BilledEMNsT.BillID IS NULL "
		") ", (long)EMR_ALL_CHARGES_ON_QUOTES, (long)EMR_SOME_CHARGES_ON_QUOTES, (long)RGB(255,255,255));
	// (a.wilson 2013-06-11 14:21) - PLID 56761 - hide any bills that all charges are assigned patient resp. when checkbox is checked. otherwise show all.
	if (bHidePatientBills) {
		strFrom += (" AND EMRMasterT.ID NOT IN (SELECT EMRID FROM EMRChargesT "
			"LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
			"GROUP BY EMRID "
			"HAVING Count(*) = Sum(CASE WHEN EMRChargeRespT.EMRChargeID Is Not Null AND EMRChargeRespT.InsuredPartyID Is Null THEN 1 ELSE 0 END) "
			")");
	}

	strFrom += (" AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND EMNBillsSentToHL7T.EMNID IS NULL "
		"GROUP BY EMRMasterT.PatientID, EMRMasterT.ID, EMRStatusListT.Name, EMRMasterT.Date, EMRMasterT.Description, EmrMasterT.ModifiedDate, "
		"Last + ', ' + First + ' ' + Middle) AS EMNsToBeBilledQ ");

	m_List->FromClause = _bstr_t(strFrom);
	
	// (j.jones 2010-07-07 16:58) - PLID 36682 - not needed because we requery in OnShowWindow
	// (a.wilson 2013-06-10 15:40) - PLID 56761 - added option to requery.
	if (bForceRequery) {
		m_List->Requery();
	}
}