// EMRLockManager.cpp : implementation file
//

#include "stdafx.h"
#include "EMRLockManager.h"
#include "EMRLockReminderDlg.h"
#include "AuditTrail.h"
#include "EmrUtils.h"
#include "EMRPreviewPopupDlg.h"
#include "EMN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum LockManagerColumns {
	lmcEMRID = 0,
	lmcEMNID,
	lmcPicID,
	lmcPatientID,
	lmcLock,
	lmcPreview, // (a.walling 2010-01-11 12:10) - PLID 31482
	lmcPatient,
	lmcDescription,
	lmcInputDate,
	lmcModifiedDate,
	lmcProvider,
	lmcSecondaryProvider, // (z.manning 2011-06-24 17:47) - PLID 35566
	lmcLocation,
	lmcStatus,		// (d.thompson 2009-12-30) - PLID 36026
};

/////////////////////////////////////////////////////////////////////////////
// CEMRLockManager dialog


CEMRLockManager::CEMRLockManager(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRLockManager::IDD, pParent, "CEMRLockManager")
{
	//{{AFX_DATA_INIT(CEMRLockManager)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (a.walling 2010-01-11 12:11) - PLID 31482
	m_hIconPreview = NULL;
	m_pEMRPreviewPopupDlg = NULL;
}


void CEMRLockManager::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRLockManager)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EMRLOCK_STATUS, m_nxstaticEmrlockStatus);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_REFRESH_LOCK_MGR, m_btnRefresh);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRLockManager, CNxDialog)
	//{{AFX_MSG_MAP(CEMRLockManager)
	ON_COMMAND(ID_EMRLOCK_CHECK, OnCheckSelected)
	ON_COMMAND(ID_EMRLOCK_UNCHECK, OnUncheckSelected)
	ON_COMMAND(ID_EMRLOCK_GOTOPATIENT, OnGotoPatient)
	ON_COMMAND(ID_EMRLOCK_GOTOEMN, OnGotoEMN)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	// (a.walling 2010-01-11 13:21) - PLID 31482
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LOCK_MANAGER_SHOW_SECONDARY_PROVIDER, OnBnClickedLockManagerShowSecondaryProvider)
	ON_BN_CLICKED(IDC_BTN_REFRESH_LOCK_MGR, OnBtnRefreshLockMgr)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRLockManager message handlers

BOOL CEMRLockManager::OnInitDialog() 
{

	try {
		
		CNxDialog::SetMinSize(500, 350); // (r.goldschmidt 2014-10-06 10:03) - PLID 62649 - Enforce a minimum opening size (default is 993x502)
		CNxDialog::OnInitDialog();
		CNxDialog::SetMinSize(0, 0); // PLID 62649 - Allow resizing down to nothing

		if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return TRUE;
		}

		// (z.manning 2011-06-24 17:51) - PLID 35566 - Cache properties
		g_propManager.CachePropertiesInBulk("CEMRLockManager-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'EmrLockManagerShowSecondaryProviderColumn' \r\n"
			"	, 'EmnRemindLockDays' \r\n"
			"	, 'DisplayTaskbarIcons' \r\n"
			"	, 'RequireCPTCodeEMNLocking' \r\n"	// (d.singleton 2013-07-24 14:49) - PLID 44840
			"	, 'RequireDiagCodeEMNLocking' \r\n" // (d.singleton 2013-07-24 14:49) - PLID 44840
			") \r\n"
			, _Q(GetCurrentUserName()));
		
		// (a.walling 2010-01-11 12:11) - PLID 31482
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

		// (c.haag 2008-05-01 12:08) - PLID 29866 - NxIconify buttons
		m_btnApply.AutoSet(NXB_OK);
		m_btnOK.AutoSet(NXB_CLOSE); // Ironically, IDOK prevents changes from being made
		// (j.jones 2011-07-08 12:02) - PLID 42878 - added a refresh button
		m_btnRefresh.AutoSet(NXB_REFRESH);

		// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the dialog in the taskbar if necessary
		//								  PLID 22877 - and respect the preference to not do so
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		m_nDays = GetRemotePropertyInt("EmnRemindLockDays", 30, 0, GetCurrentUserName(), true);

		extern CPracticeApp theApp;

		GetDlgItem(IDC_EMRLOCK_STATUS)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftTitle));

		m_pList = BindNxDataList2Ctrl(this, IDC_EMR_LOCK_LIST, GetRemoteData(), false);

		// (z.manning 2011-06-24 17:52) - PLID 35566 - Added secondary provider column
		long nShowSecProv = GetRemotePropertyInt("EmrLockManagerShowSecondaryProviderColumn", 1, 0, GetCurrentUserName());
		if(nShowSecProv == 1) {
			CheckDlgButton(IDC_LOCK_MANAGER_SHOW_SECONDARY_PROVIDER, BST_CHECKED);
		}
		ToggleSecondaryProviderColum();
		
		// (a.walling 2010-01-11 14:23) - PLID 31482 - Set up this column to pull in the HICON handle
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(lmcPreview);
		if (pCol != NULL && m_hIconPreview != NULL) {
			CString strHICON;
			strHICON.Format("%li", m_hIconPreview);
			pCol->FieldName = (LPCTSTR)strHICON;
		}

		AppendWhereClause();

		m_pList->Requery();

	} NxCatchAll("Error in CEMRLockManager::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRLockManager::AppendWhereClause()
{
	CString strWhere = "EMRMasterT.Status <> 2 AND EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL) AND PatientCreatedStatus <> 1";
	CString strAge;
	//TES 9/21/2009 - PLID 33965 - Changed from > to >=
	strAge.Format(" AND DATEDIFF(day, EMRMasterT.%s, GetDate()) >= %li", CEMRLockReminderDlg::GetRemindField(), m_nDays);
	// (z.manning 2011-05-20 11:16) - PLID 33114 - Added filter to chart permissions
	CString strChartFilter = GetEmrChartPermissionFilter().Flatten();

	m_pList->PutWhereClause(_bstr_t(strWhere + strAge + strChartFilter));
}

BEGIN_EVENTSINK_MAP(CEMRLockManager, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRLockManager)
	ON_EVENT(CEMRLockManager, IDC_EMR_LOCK_LIST, 6 /* RButtonDown */, OnRButtonDownEmrLockList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRLockManager, IDC_EMR_LOCK_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmrLockList, VTS_I2)
	ON_EVENT(CEMRLockManager, IDC_EMR_LOCK_LIST, 19 /* LeftClick */, OnLeftClickEmrLockList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRLockManager::OnRButtonDownEmrLockList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL)
			return;

		if(pRow->GetSelected() == 0) {
			m_pList->PutCurSel(pRow); // set only this row to current selection if it was not originally selected
		}
		//pRow->PutSelected(VARIANT_TRUE);

		CMenu mnu;
		CMenu *pSubMenu;
		mnu.LoadMenu(IDR_EMR_POPUP);
		pSubMenu = mnu.GetSubMenu(1);

		bool bMultiSelected = (NumSelected() > 1);

		if (bMultiSelected) {
			pSubMenu->EnableMenuItem(ID_EMRLOCK_GOTOPATIENT, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			pSubMenu->EnableMenuItem(ID_EMRLOCK_GOTOEMN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	} NxCatchAll("CEMRLockManager: Error creating popup menu");
	
}

void CEMRLockManager::OnOK() 
{

	CNxDialog::OnCancel();

	DestroyWindow();
}

// (r.goldschmidt 2014-07-23 10:46) - PLID 62649
void CEMRLockManager::OnCancel()
{
	CNxDialog::OnCancel();
	DestroyWindow();
}

void CEMRLockManager::OnCheckSelected()
{
	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstSelRow(); pRow != NULL; pRow = pRow->GetNextSelRow()) {
		pRow->PutValue(lmcLock, _variant_t(VARIANT_TRUE, VT_BOOL));
	}
}

void CEMRLockManager::OnUncheckSelected()
{
	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstSelRow(); pRow != NULL; pRow = pRow->GetNextSelRow()) {
		pRow->PutValue(lmcLock, _variant_t(VARIANT_FALSE, VT_BOOL));
	}
}


long CEMRLockManager::NumChecked()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
	long nCount = 0;

	while (pRow) {
		_variant_t var = pRow->GetValue(lmcLock);
		if (var.vt != VT_BOOL) {
			ASSERT(FALSE);
		}
		else {
			if (VarBool(var)) {
				nCount++;
			}
		}
		pRow = pRow->GetNextRow();
	}

	return nCount;
}

long CEMRLockManager::NumSelected()
{
	long nCount = 0;

	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstSelRow(); pRow != NULL; pRow = pRow->GetNextSelRow()) {
		nCount++;
	}
	return nCount;
}

void CEMRLockManager::OnRequeryFinishedEmrLockList(short nFlags) 
{
	CString strMsg;
	//TES 9/21/2009 - PLID 33965 - Changed from "over" to "at least"
	strMsg.Format("%li unlocked EMNs at least %li days old.", m_pList->GetRowCount(), m_nDays);
	SetDlgItemText(IDC_EMRLOCK_STATUS, strMsg);
}

//(e.lally 2007-01-29) PLID 24442 - Removing the ability to quickly lock a ton of EMNs, as a precaution
// against user error.
/*
void CEMRLockManager::OnCheckall() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();

	m_pList->SetRedraw(FALSE);
	while (pRow) {
		pRow->PutValue(lmcLock, _variant_t(VARIANT_TRUE, VT_BOOL));

		pRow = pRow->GetNextRow();
	}

	m_pList->SetRedraw(TRUE);
}
*/

void CEMRLockManager::OnLeftClickEmrLockList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if (nCol == lmcPatient) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow == NULL)
				return;

			long nPatID = VarLong(pRow->GetValue(lmcPatientID));

			GotoPatient(nPatID);
		} else if (nCol == lmcPreview) {
			// (a.walling 2010-01-11 12:53) - PLID 31482 - Open up the EMN preview

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow == NULL)
				return;

			long nPatID = VarLong(pRow->GetValue(lmcPatientID));
			long nEMNID = VarLong(pRow->GetValue(lmcEMNID));
			COleDateTime dtModified = VarDateTime(pRow->GetValue(lmcModifiedDate));

			ShowPreview(nPatID, nEMNID, dtModified);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEMRLockManager::GotoPatient(long nPatID)
{
	try {
		if (nPatID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {				

				// (a.walling 2010-01-11 14:59) - PLID 31482
				if (m_pEMRPreviewPopupDlg) {
					m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
				}

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatID)) {

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView)
						pView->UpdateView();
					OnOK(); // exit this dialog!
				}
			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - Cannot Open Mainframe");
			}//end else pMainFrame
		}//end if nPatientID
	}NxCatchAll("Error in CEMRLockManager::GotoPatient()");	
}

void CEMRLockManager::OnGotoPatient()
{
	if (NumSelected() > 1)
		return;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetCurSel();

	if (pRow != NULL) {
		long nPatID = VarLong(pRow->GetValue(lmcPatientID));

		GotoPatient(nPatID);
	}
}

void CEMRLockManager::OnGotoEMN()
{
	try {
		if (NumSelected() > 1)
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetCurSel();

		if (pRow != NULL) {
			long nEMNID = VarLong(pRow->GetValue(lmcEMNID));
			long nPicID = VarLong(pRow->GetValue(lmcPicID), -1);

			if (nPicID == -1) {
				AfxMessageBox("This EMN record does not have an associated PIC entry. Data is intact, however the record must be opened manually from the Patients module.");
				return;
			}			

			// (a.walling 2010-01-11 14:59) - PLID 31482
			if (m_pEMRPreviewPopupDlg) {
				m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
			}

			GetMainFrame()->EditEmrRecord(nPicID, nEMNID);
		}
	} NxCatchAll("Error in CEMRLockManager::OnGotoEMN()");
}

// (z.manning 2016-01-13 15:33) - PLID 67778 - Changed return type to boolean
BOOL CEMRLockManager::LockSelected()
// precondition: there is at least one selection.
{
	//TES 5/1/2008 - PLID 27586 - Declare the audit transaction id out here, in case we need to roll it back.
	long nAuditID = -1;
	CArray<CPendingAuditInfo> aryAuditInfo;
	try
	{
		CSqlFragment sqlLock;
		nAuditID = BeginAuditTransaction();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
	
		//TES 6/3/2008 - PLID 30237 - Keep an array of the patient IDs that changed, so we can send table checkers.
		// (a.walling 2008-06-25 16:42) - PLID 30515 - Whether to skip the current EMN or not.
		// (b.cardillo 2012-07-05 17:26) - PLID 42207 - Track the skipped due to required details separately from the 
		// rest (looks like the existing skips could be for reasons other than strictly "in use" but it had been well 
		// established in the code history that the message would just say in use, so I'm not changing it for anything 
		// other than the skip reason that this pl item is responsible for: required details).
		// Track the reason for skipping any that we skip
		long nSkipped_InUse = 0;
		long nSkipped_RequiredDetails = 0;
		// (d.singleton 2013-07-24 14:40) - PLID 44840 - Having a pop up to warn doctor a diagnosis code and/or CPT needs to be selected before locking an EMN
		long nSkipped_NoCpts = 0;
		long nSkipped_NoDiags = 0;
		long nSkipped_UnsettledPrescriptions = 0; // (b.savon 2015-12-29 13:56) - PLID 58470
		CArray<long, long> arChangedPatIDs, arynEmnIDsToLock;
		while (pRow) {
			BOOL bSkip = FALSE;
			_variant_t var = pRow->GetValue(lmcLock);
			if (var.vt != VT_BOOL) {
				ASSERT(FALSE);
			}
			else {
				if (VarBool(var)) {
					long nEMNID = VarLong(pRow->GetValue(lmcEMNID));
					long nPatID = VarLong(pRow->GetValue(lmcPatientID));

					//TES 2/13/2007 - PLID 23401 - Make sure the providers for this EMN are licensed.
					CString strDescription;
					strDescription.Format("%s - %s", VarString(pRow->GetValue(lmcPatient)), VarString(pRow->GetValue(lmcDescription)));
					if(AreEmnProvidersLicensed(nEMNID, strDescription)) {
						// (a.walling 2008-06-25 16:40) - PLID 30515 - This is inefficient, but it already existed here, so ensure that this EMN is not currently in use
						// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
						// (j.jones 2011-07-05 17:02) - PLID 44432 - supported custom statuses
						// (j.armen 2013-05-14 11:49) - PLID 56680 - Refactor EMN Access
						// (d.singleton 2013-07-24 14:30) - PLID 44840 - added count of cpt codes and diag codes so we can fail if an emn doesnt have any
						ADODB::_RecordsetPtr rs = CreateParamRecordset(
							"SELECT M.Status AS StatusID, SL.Name AS StatusName, A.UserLoginTokenID,\r\n"
							"COALESCE(EmnCpt.Count, 0) AS CptCount, COALESCE(EmnDiag.Count, 0) AS DiagCount\r\n"
							"FROM EMRMasterT M\r\n"
							"LEFT JOIN EMNAccessT A WITH(UPDLOCK, HOLDLOCK) ON M.ID = A.EmnID\r\n"
							"LEFT JOIN EMRStatusListT SL ON M.Status = SL.ID\r\n"
							"LEFT JOIN (SELECT COUNT(EMRID) AS Count, EMRID FROM EMRChargesT GROUP BY EMRID, Deleted HAVING Deleted = 0) EmnCpt ON M.ID = EmnCpt.EMRID\r\n"
							"LEFT JOIN (SELECT COUNT(EMRID) AS Count, EMRID FROM EMRDiagCodesT GROUP BY EMRID, Deleted HAVING Deleted = 0) EmnDiag ON M.ID = EmnDiag.EMRID\r\n"
							"WHERE M.ID = {INT}", nEMNID);
						if(!rs->eof) {
							_variant_t varUserToken = rs->Fields->Item["UserLoginTokenID"]->Value;

							if (varUserToken.vt != VT_NULL) {
								// someone has this EMN modified!
								bSkip = TRUE;
								// (b.cardillo 2012-07-05 17:26) - PLID 42207 - Track the skip reason
								nSkipped_InUse++;
							} else {
								long nOldStatusID = AdoFldLong(rs, "StatusID",-1);
								// (d.singleton 2013-07-24 14:30) - PLID 44840 - if preference enabled need to fail if now cpt or diag codes exist
								long nCptCount = AdoFldLong(rs, "CptCount", 0);
								long nDiagCount = AdoFldLong(rs, "DiagCount", 0);

								// (j.jones 2011-07-22 14:31) - PLID 44494 - if the status is already locked,
								// follow the same logic as if it is in use, because clearly somebody has locked it
								if(nOldStatusID == 2) {
									bSkip = TRUE;
									// (b.cardillo 2012-07-05 17:26) - PLID 42207 - Track the skip reason
									nSkipped_InUse++;
								}
								// (d.singleton 2013-07-24 14:30) - PLID 44840 - check preference and then cpt + diag codes.  fail if there are none and preference is enabled
								else if(GetRemotePropertyInt("RequireCPTCodeEMNLocking", 0, 0, "<None>", true) && nCptCount == 0) {
									bSkip = TRUE;
									nSkipped_NoCpts++;
								}
								else if(GetRemotePropertyInt("RequireDiagCodeEMNLocking", 0, 0, "<None>", true) && nDiagCount == 0) {
									bSkip = TRUE;
									nSkipped_NoDiags++;
								}
								else {
									CString strOldStatusName = AdoFldString(rs, "StatusName", "Unidentified");

									// (a.walling 2008-06-25 16:56) - PLID 30515 - Should not audit when failed
									aryAuditInfo.Add(CPendingAuditInfo(nPatID, GetExistingPatientName(nPatID), aeiEMNStatus, nEMNID, strOldStatusName, "Locked", aepHigh, aetChanged));
								}
							}
						}
						else {
							ASSERT(FALSE); // the EMN doesn't exist?
						}

						// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
						if (!bSkip) {
							// Make sure there are no unfilled required details
							if (CEMN::HasVisibleUnfilledRequiredDetails(nEMNID)) {
								// Can't lock an EMN that has unfilled required details
								bSkip = TRUE;
								// (b.cardillo 2012-07-05 17:26) - PLID 42207 - Track the skip reason
								nSkipped_RequiredDetails++;
							}

							// (b.savon 2015-12-29 10:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it.
							if (HasUnsettledPrescriptions(nEMNID, false)) {
								bSkip = TRUE;
								++nSkipped_UnsettledPrescriptions;
							}
							else {
								// No unsettled prescriptions
							}
						}

						if (!bSkip) {
							// (a.walling 2008-06-25 16:43) - PLID 30515 - As another failsafe, ensure that no one is modifying this EMN
							// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
							// (j.armen 2013-05-14 11:51) - PLID 56680 - Refactor EMN AccessT
							// (z.manning 2016-01-13 15:22) - PLID 67778 - Need to handle EMN to-dos when locking
							sqlLock += CSqlFragment(R"(
UPDATE EMRMasterT SET Status = 2
WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK))
IF @@ROWCOUNT = 0 BEGIN
	INSERT INTO @Skipped (ID) VALUES ({INT})
END ELSE BEGIN
	{CONST_STR}
END
)"
, nEMNID, nEMNID, GetEMRLockTodoAlarmSql(nEMNID));

							arynEmnIDsToLock.Add(nEMNID);

							//TES 6/3/2008 - PLID 30237 - Remember that we want to send a tablechecker for this EMN.  Since, for 
							// whatever reason, NetUtils::EMRMasterT uses the patient ID, we only want one entry per patient.
							int i = 0;
							int nCount = arChangedPatIDs.GetSize();
							bool bFound = false;
							for(i = 0; i < nCount && !bFound; i++) {
								if(arChangedPatIDs[i] == nPatID) bFound = true;
							}
							if(!bFound) {
								arChangedPatIDs.Add(nPatID);
							}
						}
					}
				}
			}

			pRow = pRow->GetNextRow();
		}

		// (z.manning 2016-01-13 13:52) - PLID 67778 - Check for incomplete to-dos
		if (!PromptIfAnyOutstandingTodos(GetRemoteData(), arynEmnIDsToLock, GetSafeHwnd())) {
			return FALSE;
		}

		// (a.walling 2008-06-25 17:14) - PLID 30515 - Run the recordset and check all those that failed to lock
		ADODB::_RecordsetPtr prsFinal = CreateParamRecordset(R"(
SET NOCOUNT ON
DECLARE @Skipped TABLE(ID INT)
BEGIN TRAN
{SQL}
COMMIT TRAN
SET NOCOUNT OFF
SELECT ID FROM @Skipped
)"
, sqlLock);

		while (!prsFinal->eof) {
			long nFailedID = AdoFldLong(prsFinal, "ID", -1);

			if (nFailedID != -1) {
				for (int i = aryAuditInfo.GetSize() - 1; i >= 0; i--) {
					if (aryAuditInfo[i].m_nRecordID == nFailedID) {
						aryAuditInfo.RemoveAt(i);
						// (b.cardillo 2012-07-05 17:26) - PLID 42207 - Track the skip reason
						nSkipped_InUse++;
						break;
					}
				}
			}

			prsFinal->MoveNext();
		}

		for (int i = 0; i < aryAuditInfo.GetSize(); i++) {
			AuditPendingEvent(nAuditID, &aryAuditInfo[i]);
		}

		CommitAuditTransaction(nAuditID);

		//TES 6/3/2008 - PLID 30237 - Now go ahead and actually send the tablecheckers for the patients that changed.
		int nSize = arChangedPatIDs.GetSize();
		for(int i = 0; i < nSize; i++) {
			CClient::RefreshTable(NetUtils::EMRMasterT, arChangedPatIDs[i]);
		}

		// (b.cardillo 2012-07-05 17:26) - PLID 42207 - Track the skip reason
		// Give an appropriate message depending on the reason for the skips, if any are skipped
		// (d.singleton 2013-07-24 14:44) - PLID 44840 - track failures for no cpt codes and no diagnosis codes
		// (b.savon 2015-12-29 14:00) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it. -- Practice side
		if (nSkipped_InUse > 0 || nSkipped_RequiredDetails > 0 || nSkipped_NoCpts > 0 || nSkipped_NoDiags > 0 || nSkipped_UnsettledPrescriptions > 0) {
			CString strMsg;
			if (nSkipped_InUse > 0) {
				strMsg += FormatString("%li EMNs are currently being modified, and therefore were not locked.\r\n\r\n", nSkipped_InUse);
			}
			if (nSkipped_RequiredDetails > 0) {
				strMsg += FormatString("%li EMNs each contain at least one required detail that has not been filled in, and therefore were not locked.\r\n\r\n", nSkipped_RequiredDetails);
			}
			if (nSkipped_NoCpts > 0) {
				strMsg += FormatString("%li EMNs have no CPT codes selected, and therefore were not locked.\r\n\r\n", nSkipped_NoCpts);
			}
			if (nSkipped_NoDiags > 0) {
				strMsg += FormatString("%li EMNs have no Diagnosis codes selected, and therefore were not locked.\r\n\r\n", nSkipped_NoDiags);
			}
			if (nSkipped_UnsettledPrescriptions > 0) {
				strMsg += FormatString("%li EMNs have unsettled prescriptions, and therefore were not locked.\r\n\r\n", nSkipped_UnsettledPrescriptions);
			}
			MessageBox(strMsg, NULL, MB_OK);
		}

		return TRUE;

	} NxCatchAllCall("Error in CEMRLockManager::LockSelected() (Failsafe: No EMNs have been locked)",
		if(nAuditID != -1) {
			//TES 5/1/2008 - PLID 27586 - Need to rollback our audit transaction
			RollbackAuditTransaction(nAuditID);
		}
	);

	return FALSE;
}

void CEMRLockManager::OnApply() 
{
	// (c.haag 2009-03-16 15:57) - PLID 28968 - We now have a user-level preference for auto-dismissing this window
	BOOL bAutoDismiss = (GetRemotePropertyInt("DismissEMRLockManagerAfterLocking", 1, 0, GetCurrentUserName(), true) > 0) ? TRUE : FALSE;
	long nChecked = NumChecked();
	if (nChecked <= 0) {
		if (bAutoDismiss) { // (c.haag 2009-03-16 15:58) - PLID 28968
			CNxDialog::OnOK();
			DestroyWindow();
		} else {
			// (c.haag 2009-03-16 16:09) - PLID 28968 - Since this button has no effect if nothing is selected
			// with auto-dismiss turned off, prompt the user. This is better than doing nothing and appearing 
			// to be broken.
			AfxMessageBox("Please select one or more EMN's to lock first.", MB_OK | MB_ICONERROR);
		}
	}
	else {
		// (a.walling 2007-11-28 11:23) - PLID 28044 - Check for expired
		// (z.manning 2009-08-11 15:21) - PLID 24277 - Use lock permission instead of write
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptDynamic3) || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
			return;
		}

		CString strMsg;
		strMsg.Format("You have chosen to lock %li EMN(s).  Once an EMN is locked, it will not be possible for any user to make any changes to it, under any circumstances.\nAre you SURE you wish to do this?", nChecked);

		if(IDYES != MsgBox(MB_YESNO, strMsg)) {
			return;
		}

		if (!LockSelected()) {
			return;
		}

		if (bAutoDismiss) { // (c.haag 2009-03-16 15:58) - PLID 28968
			CNxDialog::OnOK();
			DestroyWindow();
		} else {
			// (c.haag 2009-03-16 16:01) - PLID 28968 - Need to reload the content
			m_pList->Requery();
		}
	}	
}

void CEMRLockManager::OnDestroy()
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

	CNxDialog::OnDestroy();
}


// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
// (z.manning 2012-09-10 15:42) - PLID 52543 - Added modified date
void CEMRLockManager::ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate)
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
		m_pEMRPreviewPopupDlg->RestoreSize("EMRLockManager");
	}
	
	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, but since we haven't opened the dialog yet,
	// we can pass in an empty array.
	EmnPreviewPopup emn(nEMNID, dtEmnModifiedDate);
	m_pEMRPreviewPopupDlg->SetPatientID(nPatID, emn);
	m_pEMRPreviewPopupDlg->PreviewEMN(emn, 0);

	// (a.walling 2010-01-11 16:20) - PLID 27733 - Only show if it is not already
	if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
		m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
	}
}

// (z.manning 2011-06-24 17:37) - PLID 35566
void CEMRLockManager::OnBnClickedLockManagerShowSecondaryProvider()
{
	try
	{
		long nPropValue = (IsDlgButtonChecked(IDC_LOCK_MANAGER_SHOW_SECONDARY_PROVIDER) == BST_CHECKED ? 1 : 0);
		SetRemotePropertyInt("EmrLockManagerShowSecondaryProviderColumn", nPropValue, 0, GetCurrentUserName());
		ToggleSecondaryProviderColum();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-06-24 17:49) - PLID 35566
void CEMRLockManager::ToggleSecondaryProviderColum()
{
	NXDATALIST2Lib::IColumnSettingsPtr pSecProvCol = m_pList->GetColumn(lmcSecondaryProvider);
	if(IsDlgButtonChecked(IDC_LOCK_MANAGER_SHOW_SECONDARY_PROVIDER) == BST_CHECKED) {
		pSecProvCol->PutStoredWidth(125);
	}
	else {
		pSecProvCol->PutStoredWidth(0);
	}
}

// (j.jones 2011-07-08 12:02) - PLID 42878 - added a refresh button
void CEMRLockManager::OnBtnRefreshLockMgr()
{
	try {

		//this will lose checked selections, of course
		m_pList->Requery();

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-15 16:12) - PLID 62649 - Make the EMR Lock Manager resizable and enable the maximize button
void CEMRLockManager::OnSize(UINT nType, int cx, int cy)
{
	try{
		CNxDialog::OnSize(nType, cx, cy);
	}NxCatchAll(__FUNCTION__);
}