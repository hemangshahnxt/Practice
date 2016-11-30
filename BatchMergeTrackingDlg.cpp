// BatchMergeTrackingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "letterwritingRc.h"
#include "BatchMergeTrackingDlg.h"
#include "SelectDlg.h"
#include "MergeEngine.h"
#include "LetterWriting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBatchMergeTrackingDlg dialog


CBatchMergeTrackingDlg::CBatchMergeTrackingDlg(CWnd* pParent)
	: CNxDialog(CBatchMergeTrackingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchMergeTrackingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBatchMergeTrackingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchMergeTrackingDlg)
	DDX_Control(pDX, IDC_MERGE_BATCH_TO_PRINTER, m_btnMergeToPrinter);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_MERGE_ALL, m_btnMergeAll);
	DDX_Control(pDX, IDC_SHOW_ON_HOLD_LADDERS, m_btnShowOnHoldLadders);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatchMergeTrackingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBatchMergeTrackingDlg)
	ON_BN_CLICKED(IDC_MERGE_ALL, OnMergeAll)
	ON_BN_CLICKED(IDC_SELECT_ALL_STEPS, OnSelectAllSteps)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_STEPS, OnUnselectAllSteps)
	ON_BN_CLICKED(IDC_SHOW_ON_HOLD_LADDERS, OnShowOnHoldLadders)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchMergeTrackingDlg message handlers

using namespace NXDATALIST2Lib;
using namespace ADODB;

BOOL CBatchMergeTrackingDlg::OnInitDialog() 
{
	
	CNxDialog::OnInitDialog();
	
	try {
		//TES 12/18/2006 - PLID 19278 - OK, we need to find all the active tracking steps that are for either templates or packets,
		// and add them to our list.

		// (z.manning 2010-06-25 14:40) - PLID 39369 - Added bulk caching
		g_propManager.CachePropertiesInBulk("BatchPaymentEditDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ( \r\n"
			"	Name = 'BatchMergeTrackingShowOnHoldLadders' \r\n"
			")",
			_Q(GetCurrentUserName()));

		// (z.manning, 04/25/2008) - PLID 29795 - Button styles
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnMergeAll.AutoSet(NXB_MERGE);
		
		//Bind our controls.
		m_pList = BindNxDataList2Ctrl(IDC_BATCH_MERGE_LIST, false);
		//TES 7/16/2010 - PLID 39400 - Set the scope column to default to our global preference.
		IColumnSettingsPtr pScopeCol = m_pList->GetColumn(mlcScope);
		pScopeCol->PutFieldName(_bstr_t(FormatString("COALESCE(StepTemplatesT.DefaultScope,%li)", GetRemotePropertyInt("Tracking_DefaultMergeScope", PhaseTracking::mtsPic, 0, "<None>"))));

		m_pUserCombo = BindNxDataList2Ctrl(IDC_TRACKING_USERS);

		//Add in the {All Users} row.
		m_pUserCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		IRowSettingsPtr pRow = m_pUserCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{All Users}"));
		m_pUserCombo->AddRowBefore(pRow, m_pUserCombo->GetFirstRow());

		//Now, try and set the selection to the current user, and if we can't, just go with {All Users}
		pRow = m_pUserCombo->FindByColumn(0, GetCurrentUserID(), NULL, FALSE);
		if(pRow == NULL) {
			pRow = m_pUserCombo->GetFirstRow();
		}
		m_pUserCombo->CurSel = pRow;

		// (z.manning 2010-06-25 14:16) - PLID 39369
		m_btnShowOnHoldLadders.SetCheck(GetRemotePropertyInt("BatchMergeTrackingShowOnHoldLadders", 0, 0, GetCurrentUserName()));

		//Finally, call the event handler, which will in turn call RefreshList().
		OnSelChosenTrackingUsers(m_pUserCombo->CurSel);
	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchMergeTrackingDlg::OnCancel() 
{	
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CBatchMergeTrackingDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBatchMergeTrackingDlg)
	ON_EVENT(CBatchMergeTrackingDlg, IDC_BATCH_MERGE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedBatchMergeList, VTS_I2)
	ON_EVENT(CBatchMergeTrackingDlg, IDC_BATCH_MERGE_LIST, 19 /* LeftClick */, OnLeftClickBatchMergeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBatchMergeTrackingDlg, IDC_BATCH_MERGE_LIST, 8 /* EditingStarting */, OnEditingStartingBatchMergeList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CBatchMergeTrackingDlg, IDC_BATCH_MERGE_LIST, 10 /* EditingFinished */, OnEditingFinishedBatchMergeList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CBatchMergeTrackingDlg, IDC_TRACKING_USERS, 1 /* SelChanging */, OnSelChangingTrackingUsers, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBatchMergeTrackingDlg, IDC_TRACKING_USERS, 16 /* SelChosen */, OnSelChosenTrackingUsers, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CBatchMergeTrackingDlg::OnRequeryFinishedBatchMergeList(short nFlags) 
{
	try {
		//Go through all the rows; if they don't have exactly one thing to merge, then flag them as yellow, and make
		// the last column a hyperlink so they can select which one to merge.
		IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow) {
			_variant_t varActionID = pRow->GetValue(mlcActionID);
			if(varActionID.vt == VT_NULL) {
				//No action selected.
				pRow->PutCellLinkStyle(mlcActionName, dlLinkStyleTrue);
				pRow->PutBackColor(RGB(255,255,127));
			}
			//Is this a packet?
			if(VarLong(pRow->GetValue(mlcAction)) == PhaseTracking::PA_WritePacket) {
				//Yes, so the "Scope" field is n/a
				pRow->PutValue(mlcScope, g_cvarNull);
				pRow->PutCellBackColor(mlcScope, RGB(127,127,127));
			}

			if(varActionID.vt == VT_I4) {
				//If they have an action (meaning the "Selected" checkbox is an option), see if the step isn't actually active
				// yet; and if it isn't, unselect this row.
				COleDateTime dtActive = VarDateTime(pRow->GetValue(mlcActiveDate), COleDateTime(1899,12,30,0,0,0));
				dtActive.SetDateTime(dtActive.GetYear(), dtActive.GetMonth(), dtActive.GetDay(), 0, 0, 0);
				if(dtActive > COleDateTime::GetCurrentTime()) {
					//This step isn't actually active yet, so leave it unchecked (but in the list).
					pRow->PutValue(mlcSelected, g_cvarFalse);
				}
			}

			pRow = pRow->GetNextRow();
		}

		//OK, we're all set, so enable the controls.
		GetDlgItem(IDC_BATCH_MERGE_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_SELECT_ALL_STEPS)->EnableWindow(TRUE);
		GetDlgItem(IDC_UNSELECT_ALL_STEPS)->EnableWindow(TRUE);
		SetDlgItemText(IDC_MERGE_ALL, "Merge All Selected");
		GetDlgItem(IDC_MERGE_ALL)->EnableWindow(TRUE);
	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnRequeryFinishedBatchMergeList()");		
}

void CBatchMergeTrackingDlg::OnLeftClickBatchMergeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	if(!lpRow) return;

	// (a.walling 2007-12-03 10:15) - PLID 28255 - Adding try/catch handling to this event handler

	try {

		switch(nCol) {
		case mlcActionName:
			{
				IRowSettingsPtr pRow(lpRow);
				if(pRow->GetCellLinkStyle(mlcActionName) == dlLinkStyleTrue) {
					//They want to select which templates/packets to merge.
					CSelectDlg dlg(this);
					long nAction = VarLong(pRow->GetValue(mlcAction));
					long nStepTemplateID = VarLong(pRow->GetValue(mlcStepTemplateID));
					long nSelectedID = -1;
					CString strSelectedName;
					if(nAction == PhaseTracking::PA_WriteTemplate) {
						//Do they have some specified templates?
						// (a.walling 2007-12-03 10:05) - PLID 28255 - Extra ) was throwing an exception
						if(ReturnsRecords("SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = %li", nStepTemplateID)) {
							//They do, so make them select out of that list.
							CString strWhere;
							strWhere.Format("ID IN (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = %li)", nStepTemplateID);
							dlg.m_strWhereClause = strWhere;
							dlg.m_strFromClause = "MergeTemplatesT";
							dlg.m_strCaption = "Please select a template to merge";
							dlg.m_strTitle = "Choose template";
							dlg.AddColumn("ID", "ID", FALSE, FALSE);
							dlg.AddColumn("Path", "Template", TRUE, TRUE);
							if(IDOK != dlg.DoModal()) {
								return;
							}
							ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
							nSelectedID = VarLong(dlg.m_arSelectedValues[0]);
							strSelectedName = VarString(dlg.m_arSelectedValues[1]);
						}
						else {
							//Any template can satisfy this, so let them browse.
							char path[MAX_PATH];
							path[0] = 0;
							OPENFILENAME ofn;
							CString strInitDir = GetTemplatePath("");
							ZeroMemory(&ofn, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hwndOwner =	GetSafeHwnd();

							// (a.walling 2007-06-14 13:22) - PLID 26342 - Should we support word 2007?
							// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
							static char Filter2007[] = "Microsoft Word Templates (*.dot, *.dotx, *.dotm)\0*.DOT;*.DOTX;*.DOTM\0";
							// Always support Word 2007 templates
							ofn.lpstrFilter = Filter2007;

							ofn.lpstrCustomFilter = NULL;
							ofn.nFilterIndex = 1;
							ofn.lpstrFile = path;
							ofn.nMaxFile = MAX_PATH;
							ofn.lpstrInitialDir = strInitDir.GetBuffer(MAX_PATH);
							strInitDir.ReleaseBuffer();
							ofn.lpstrTitle = "Select a merge template";
							ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
							ofn.lpstrDefExt = "dot";

							if (::GetOpenFileName(&ofn)) {	
								//DRT 7/7/03 - Make sure the path we were given is valid
								if(!DoesExist(path)) {
									MsgBox("The document you have selected does not exist.  Please choose a valid template and try again.");
									return;
								}

								//Normalize the path name.
								CString strPath(path);
								if(strPath.Left(GetSharedPath().GetLength()).CompareNoCase(GetSharedPath()) == 0) {
									strPath = strPath.Mid(GetSharedPath().GetLength());
									//Since this is presumed to be relative to the shared path, make sure it starts with exactly one '\'.
									if(strPath.Left(1) != "\\") {
										strPath = "\\" + strPath;
									}
								}

								// (a.walling 2007-12-03 13:36) - PLID 28259 - This is creating multiple entries in MergeTemplatesT for the same template!
								_RecordsetPtr rsTemplateID = CreateRecordset("SELECT ID FROM MergeTemplatesT WHERE Path = '%s'", _Q(strPath));
								if(rsTemplateID->eof) {									
									// (a.walling 2007-12-03 13:35) - PLID 28259 - Mind your P's and _Q's.
									//OK, let's put this in our MergeTemplatesT table.
									nSelectedID = NewNumber("MergeTemplatesT", "ID");
									ExecuteSql("INSERT INTO MergeTemplatesT (ID, Path) VALUES (%li, '%s')", nSelectedID, _Q(strPath));
								}
								else {
									//It already has been stored.
									nSelectedID = AdoFldLong(rsTemplateID, "ID");
								}

								strSelectedName = strPath.Mid(strPath.ReverseFind('\\'));
							}
							else {
								return;
							}
						}
					}
					else {
						//Make them select from a list of packets; either the packets specified for this step template, or, if this
						// step template doesn't have any specified, then all packets.
						CString strWhere;
						strWhere.Format("NOT EXISTS (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = %li) OR ID IN (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = %li)", nStepTemplateID, nStepTemplateID);
						dlg.m_strWhereClause = strWhere;
						dlg.m_strFromClause = "PacketsT";
						dlg.m_strCaption = "Please select a packet to merge";
						dlg.m_strTitle = "Choose packet";
						dlg.AddColumn("ID", "ID", FALSE, FALSE);
						dlg.AddColumn("Name", "Packet", TRUE, TRUE);
						if(IDOK != dlg.DoModal()) {
							return;
						}
						ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
						nSelectedID = VarLong(dlg.m_arSelectedValues[0]);
						strSelectedName = VarString(dlg.m_arSelectedValues[1]);
					}
					
					//OK, at this point they've selected a template or packet, so update the row to reflect that, and select it.
					pRow->PutValue(mlcActionID, nSelectedID);
					pRow->PutValue(mlcActionName, _bstr_t(strSelectedName));
					pRow->PutBackColor(m_pList->GetNewRow()->GetBackColor());
					pRow->PutValue(mlcSelected, g_cvarTrue);

					BOOL bScope = FALSE;
					if(nAction == PhaseTracking::PA_WriteTemplate) {
						//TES 7/16/2010 - PLID 39400 - We now have a default scope for each step, and a global default.  It was already
						// loaded when we requeried the list, so no need to do anything here.
						//What they selected was a template, so use its default scope (if it doesn't have a default, use 0, which is one-per-PIC).
						/*// (a.walling 2007-12-03 13:46) - PLID 28259 - Use the selected ID since we might not always be using the dialog
						long nDefaultScope = VarLong(GetTableField("MergeTemplatesT", "DefaultScope", "ID", nSelectedID), 0);
						//If it's any of the per-procedure scopes, then check "Per Proc", otherwise don't.
						bScope = (nDefaultScope == 1 || nDefaultScope == 2 || nDefaultScope == 4) ? TRUE : FALSE;
						pRow->PutValue(mlcScope, bScope?g_cvarTrue:g_cvarFalse);*/
					}

					//If there is at least one other row that is for this template and doesn't have an Action ID, ask if they want to
					// update all of them.
					bool bPromptedUser = false;
					bool bChangeAll = true;
					IRowSettingsPtr p = m_pList->GetFirstRow();
					while(p != NULL && bChangeAll) {
						if(p != pRow) {
							if(VarLong(p->GetValue(mlcStepTemplateID),-1) == VarLong(pRow->GetValue(mlcStepTemplateID),-2)) {
								if(p->GetValue(mlcActionID).vt == VT_NULL) {
									if(!bPromptedUser) {
										if(IDYES != MsgBox(MB_YESNO, "There are other active ladders at this step, would you like to use this item for them as well?")) {
											bChangeAll = false;
										}
										bPromptedUser = true;
									}
									if(bChangeAll) {
										p->PutValue(mlcActionID, nSelectedID);
										p->PutValue(mlcActionName, _bstr_t(strSelectedName));
										p->PutBackColor(m_pList->GetNewRow()->GetBackColor());
										p->PutValue(mlcSelected, g_cvarTrue);
										//TES 7/16/2010 - PLID 39400 - Leave this alone, we don't modify the scope when changing
										// the template any more.
										/*if(nAction == PhaseTracking::PA_WriteTemplate) {
											p->PutValue(mlcScope, bScope?g_cvarTrue:g_cvarFalse);
										}*/

									}
								}
							}
						}
						p = p->GetNextRow();
					}
				}
			}
			break;
		}
	} NxCatchAll("Error in CBatchMergeTrackingDlg::OnLeftClickBatchMergeList()");
}

bool CompareArrays(const CArray<long,long> &ar1, const CArray<long,long> &ar2)
{
	if(ar1.GetSize() != ar2.GetSize()) return false;
	
	for(int i = 0; i < ar1.GetSize(); i++) {
		bool bMatched = false;
		for(int j = 0; j < ar2.GetSize() && !bMatched; j++) {
			if(ar1[i] == ar2[j]) bMatched = true;
		}
		if(!bMatched) return false;
	}
	return true;
}

void CBatchMergeTrackingDlg::OnMergeAll() 
{
	try {
		//This has three main parts:
		// 1.)	Go through and build a list of things to merge, trying to group as many rows together as possible so that we
		//		create the fewest number of documents.
		// 2.)	If we were not able to combine everything into a single document (or packet), warn the user that they may 
		//		be about to have a ton of Word windows open.
		// 3.)	Assuming the user said it was all right, go through and merge everything, based on the info we compiled in 1.)

		CArray<MergeDocumentInfo*,MergeDocumentInfo*> arMergeDocuments;
		{
			// 1.)	Go through and build a list of things to merge, trying to group as many rows together as possible so that we
			//		create the fewest number of documents.		
			CWaitCursor cuWait;
			IRowSettingsPtr pRow = m_pList->GetFirstRow();

			while(pRow) {
				if(VarBool(pRow->GetValue(mlcSelected),FALSE)) {
					//We've got a selected row. First, figure out what its action is.
					long nAction = VarLong(pRow->GetValue(mlcAction));
					bool bIsPacket = (nAction == PhaseTracking::PA_WritePacket)?true:false;
					long nActionID = VarLong(pRow->GetValue(mlcActionID));

					//Now, fill in our list of templates.
					CArray<MergeTemplate,MergeTemplate&> arTemplates;
					if(bIsPacket) {
						// (a.walling 2007-12-04 14:34) - PLID 28273 - Parameterized
						//DRT 6/11/2008 - PLID 28919 - We need to ensure that at least 1 of the templates in the packet exist before 
						//	attempting to merge.  This mirrors the tracking tab behavior
						_RecordsetPtr rsComponents = CreateParamRecordset("SELECT MergeTemplateID, Scope, MergeTemplatesT.Path "
							"FROM PacketComponentsT INNER JOIN MergeTemplatesT ON PacketComponentsT.MergeTemplateID = MergeTemplatesT.ID "
							"WHERE PacketID = {INT} ORDER BY ComponentOrder", nActionID);
						long nTemplatesMergeable = 0;
						while(!rsComponents->eof) {
							MergeTemplate mt;
							mt.nMergeTemplateID = AdoFldLong(rsComponents, "MergeTemplateID");
							mt.nScope = AdoFldLong(rsComponents, "Scope");
							mt.nPicID = VarLong(pRow->GetValue(mlcPicID), -1); // (a.walling 2007-12-04 13:44) - PLID 28272
							//DRT 6/11/2008 - PLID 28919 - Ensure it exists
							CString strTemplateName = AdoFldString(rsComponents, "Path", "");
							//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
							if(strTemplateName.GetLength() >= 2 && strTemplateName.GetAt(0) == '\\' && strTemplateName.GetAt(1) != '\\') {
								strTemplateName = GetSharedPath() ^ strTemplateName;
							}

							if(DoesExist(strTemplateName)) {
								arTemplates.Add(mt);
								nTemplatesMergeable++;
							}
							else {
								CString strPacketName = VarString(pRow->GetValue(mlcActionName), "");
								MsgBox("The template '%s' in the packet '%s' could not be found.  This template will be skipped.", strTemplateName, strPacketName);
							}
							rsComponents->MoveNext();
						}
						//DRT 6/11/2008 - PLID 28919 - We must first check that there is anything to merge in the packet selected.  If not, 
						//	this entire attempt to merge should be skipped (this is consistent with behavior in the tracking tab).
						if(nTemplatesMergeable == 0) {
							//There are no templates in this packet!
							CString strPtName = VarString(pRow->GetValue(mlcPatientName), "");
							CString strPacketName = VarString(pRow->GetValue(mlcActionName), "");
							AfxMessageBox( FormatString("The packet '%s' for patient '%s' does not contain any templates.\r\n"
								"No documents will be created.", strPacketName, strPtName) );
							//Also, uncheck this row, just to be safe.
							pRow->PutValue(mlcSelected, _variant_t(false));
							//Just skip the rest of the work, we cannot mark this step as complete if we didn't do anything!
							pRow = pRow->GetNextRow();
							continue;
						}
						rsComponents->Close();						
					}
					else {
						MergeTemplate mt;
						mt.nMergeTemplateID = nActionID;
						//TES 7/1/2010 - PLID 39400 - They now have all the scopes available.
						mt.nScope = VarLong(pRow->GetValue(mlcScope));
						mt.nPicID = VarLong(pRow->GetValue(mlcPicID), -1); // (a.walling 2007-12-04 13:44) - PLID 28272
						arTemplates.Add(mt);
					}

					// (a.walling 2007-12-04 13:03) - PLID 28271 - We need to calculate the procedures for single templates too,
					// not just packets.
					//Now go through and calculate the procedures for each (this is a separate loop so that we don't have
					// two recordsets open at once).
					for(int nTemplate = 0; nTemplate < arTemplates.GetSize(); nTemplate++) {
						MergeTemplate mt = arTemplates[nTemplate];
						switch(mt.nScope) {
						case PhaseTracking::mtsMasterProcedure: //One-per-Master
							{
								// (a.walling 2007-12-04 14:34) - PLID 28273 - Parameterized
								// (a.walling 2007-12-03 13:24) - PLID 28256 - Should be passing ProcInfoDetailsT.ID, not ProcedureT.ID
								_RecordsetPtr rsProcs = CreateParamRecordset("SELECT ID FROM ProcInfoDetailsT "
									"WHERE ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID Is Null) "
									"AND ProcInfoID = {INT}", VarLong(pRow->GetValue(mlcProcInfoID)));
								while(!rsProcs->eof) {
									mt.arProcIDs.Add(AdoFldLong(rsProcs, "ID"));
									rsProcs->MoveNext();
								}
								rsProcs->Close();
							}
							break;
						//TES 7/1/2010 - PLID 39400 - Added a -1 scope, which we treat as "1-per-patient", so don't add any procedures.
						//TES 7/16/2010 - PLID 39400 - This is now official, and in the PhaseTracking::MergeTemplateScope enum
						case PhaseTracking::mtsPatient:
							break;
						case PhaseTracking::mtsPic: //One-per-PIC
							// (a.walling 2007-12-04 13:14) - PLID 28271 - Actually, we should choose the top, arbitrary
							// ProcInfoDetailsT record, same way the PIC does.
						case PhaseTracking::mtsProcedure: //One-per-procedure
							{
								// (a.walling 2007-12-03 10:33) - PLID 28256 - This was always failing due to a bad
								// join (ON ProcInfoDetailsT.ProcedureID = ProcInfoDetailsT.ID), and then returning
								// too many records due to a misplaced OR.
								// (a.walling 2007-12-04 14:34) - PLID 28273 - Parameterized
								_RecordsetPtr rsProcs = CreateParamRecordset(
									"SELECT ProcInfoDetailsT.ID, "
										"ProcedureID, "
										"ProcInfoDetailsT. *, "
										"ProcedureT.Name "
									"FROM ProcInfoDetailsT "
									"INNER JOIN ProcedureT "
									"ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
									"WHERE ProcInfoID                = {INT} "
										"AND ( Chosen = 1 OR"
												"("
													"ProcedureT.MasterProcedureID Is Null "
													"AND NOT EXISTS "
													"(SELECT ID "
													"FROM ProcInfoDetailsT OtherDetails "
													"WHERE OtherDetails.ProcInfoID = ProcInfoDetailsT.ProcInfoID "
														"AND Chosen                   = 1 "
														"AND ProcedureID IN "
														"(SELECT ID "
														"FROM ProcedureT DetailProcs "
														"WHERE DetailProcs.MasterProcedureID = ProcedureT.ID "
														") "
													")"
												")"
											")",
										VarLong(pRow->GetValue(mlcProcInfoID)));

								// (a.walling 2007-12-04 13:16) - PLID 28271 - If we are per-PIC, choose an arbitary ProcInfoDetailsT.ID
								if (mt.nScope == PhaseTracking::mtsPic) {
									// this is done the same way in PicContainerDlg's OnGenTemplate
									if (!rsProcs->eof) {
										mt.arProcIDs.Add(AdoFldLong(rsProcs, "ID"));
									}
								} else {
									// (a.walling 2007-12-03 13:24) - PLID 28256 - Should be passing ProcInfoDetailsT.ID, not ProcedureT.ID
									while(!rsProcs->eof) {
										mt.arProcIDs.Add(AdoFldLong(rsProcs, "ID"));
										rsProcs->MoveNext();
									}
								}

								rsProcs->Close();
							}
							break;
						case PhaseTracking::mtsPrescription: //One-per-prescription
							{
								// (a.walling 2007-12-04 14:34) - PLID 28273 - Parameterized
								_RecordsetPtr rsMeds = CreateParamRecordset("SELECT PatientMedications.ID "
									"FROM PatientMedications INNER JOIN EmrMedicationsT ON PatientMedications.ID = "
									"EmrMedicationsT.MedicationID INNER JOIN EmrMasterT ON EmrMedicationsT.EmrID = EmrMasterT.ID "
									"INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID INNER JOIN ProcInfoT ON PicT.ProcInfoID "
									"= ProcInfoT.ID "
									"WHERE EMRMasterT.Deleted = 0 AND EMRMedicationsT.Deleted = 0 AND ProcInfoT.ID = {INT}", VarLong(pRow->GetValue(mlcProcInfoID)));
								while(!rsMeds->eof) {
									mt.arMedicationIDs.Add(AdoFldLong(rsMeds, "ID"));
									rsMeds->MoveNext();
								}
								rsMeds->Close();
							}
							break;
						case PhaseTracking::mtsDetailProcedure: //One-per-Detail
							{
								// (a.walling 2007-12-03 13:24) - PLID 28256 - Should be passing ProcInfoDetailsT.ID, not ProcedureT.ID
								// (a.walling 2007-12-04 14:32) - PLID 28273 - This was joined incorrectly, and not filtered on anything.
								// (a.walling 2007-12-04 14:34) - PLID 28273 - Parameterized
								_RecordsetPtr rsProcs = CreateParamRecordset("SELECT ProcInfoDetailsT.ID FROM ProcInfoDetailsT INNER JOIN "
									"ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
									"WHERE Chosen = 1 AND ProcedureT.MasterProcedureID Is Not Null AND ProcInfoID = {INT}", VarLong(pRow->GetValue(mlcProcInfoID)));
								while(!rsProcs->eof) {
									mt.arProcIDs.Add(AdoFldLong(rsProcs, "ID"));
									rsProcs->MoveNext();
								}
								rsProcs->Close();
							}
							break;
						}
						//If they are merging per-detail, and don't have any details, skip this template.
						if(mt.nScope == PhaseTracking::mtsDetailProcedure && mt.arProcIDs.GetSize() == 0) {
							arTemplates.RemoveAt(nTemplate);
							nTemplate--;
						}
						else {
							arTemplates.SetAt(nTemplate, mt);
						}
					}										

					//Now, have we already got a merge document to add this to?
					bool bMatchFound = false;
					for(int nDoc = 0; nDoc < arMergeDocuments.GetSize() && !bMatchFound; nDoc++) {
						MergeDocumentInfo *pMdi = arMergeDocuments[nDoc];
						if(pMdi->bIsPacket == bIsPacket && pMdi->nActionID == nActionID) {
							//The action is the same, are the templates?
							
							if(pMdi->arTemplates.GetSize() == arTemplates.GetSize()) {
								bool bMismatchFound = false;
								for(int j = 0; j < pMdi->arTemplates.GetSize() && !bMismatchFound; j++) {
									bool bTemplateMatchFound = false;
									for(int k = 0; k < arTemplates.GetSize() && !bTemplateMatchFound; k++) {
										if(pMdi->arTemplates[j] == arTemplates[k]) bTemplateMatchFound = true;
									}
									if(!bTemplateMatchFound) bMismatchFound = true;
								}
								if(!bMismatchFound) {
									//These seem to match.  However, we can't have the same patient in the list more than once.
									// Why is this?  Because that means that patient has two different steps needing the same document,
									// and if we merge the document with the same patient in there twice, it will only create one
									// document, and therefore only complete one of the steps.
									long nThisPatientID = VarLong(pRow->GetValue(mlcPatientID));
									bool bPatientIDMatched = false;
									for(int nRow = 0; nRow < pMdi->arRows.GetSize() && !bPatientIDMatched; nRow++) {
										IRowSettingsPtr pThisRow(pMdi->arRows[nRow]);
										if(VarLong(pThisRow->GetValue(mlcPatientID)) == nThisPatientID) {
											bPatientIDMatched = true;
										}
									}
									if(!bPatientIDMatched) {
										//We've got a match!  Just add this row to the list.
										pMdi->arRows.Add(pRow);
										arMergeDocuments.SetAt(nDoc, pMdi);
										bMatchFound = true;
									}
								}
							}
						}
					}
					
					
					if(!bMatchFound) {
						//We couldn't match it to an existing record, so we need to make a new record.
						MergeDocumentInfo *pMdi = new MergeDocumentInfo;
						pMdi->bIsPacket = bIsPacket;
						pMdi->nActionID = nActionID;
						for(int nTemplate = 0; nTemplate < arTemplates.GetSize(); nTemplate++) {
							pMdi->arTemplates.Add(arTemplates[nTemplate]);
						}
						pMdi->arRows.Add(pRow);

						//And, add it to our list.
						arMergeDocuments.Add(pMdi);
					}
				}

				pRow = pRow->GetNextRow();
			}
		}

		// 2.)	If we were not able to combine everything into a single document (or packet), warn the user that they may 
		//		be about to have a ton of Word windows open.
		int nDocTotal = 0;
		if(arMergeDocuments.GetSize() > 0) {
			for(int nDoc = 0; nDoc < arMergeDocuments.GetSize(); nDoc++) {
				nDocTotal += arMergeDocuments[nDoc]->arTemplates.GetSize();
			}
			if(nDocTotal > 1 && arMergeDocuments.GetSize() > 1) {//Don't notify them if they're just merging one packet.
				CString strMessage;
				if(IsDlgButtonChecked(IDC_MERGE_BATCH_TO_PRINTER)) {
					strMessage.Format("This merge will create %li documents, and output them all directly to the printer.  Are you sure you wish to continue?", nDocTotal);
				}
				else {
					strMessage.Format("This merge will create %li documents, each in a separate Word window.  Are you sure you wish to continue?", nDocTotal);
				}
				if(IDYES != MsgBox(MB_YESNO, "%s", strMessage)) {
					for(int nDoc = 0; nDoc < arMergeDocuments.GetSize(); nDoc++) {
						delete arMergeDocuments[nDoc];
					}
					arMergeDocuments.RemoveAll();
					return;
				}
			}
		}
		else {
			ASSERT(arMergeDocuments.GetSize() == 0); //There should be any MergeDocumentInfos with no MergeTemplates
			MsgBox("You have not selected anything to merge. Please select one or more rows before continuing.");
			return;
		}


		// 3.)	Assuming the user said it was all right, go through and merge everything, based on the info we compiled in 1.)

		//If we can't find a template, we'll skip it, we want to remember which ones we skip so we only notify the user once
		// about a given template.
		CStringArray arSkippedTemplates;
		//Track how many templates we've merged, so we can properly update our progress.
		int nDocumentsMerged = 0;
		BOOL bStopMerging = FALSE;
		for(int nDocument = 0; nDocument < arMergeDocuments.GetSize() && !bStopMerging; nDocument++) {
			MergeDocumentInfo *pMdi = arMergeDocuments[nDocument];
			long nPacketID = -1;
			long nMergedPacketID = -1;
			if(pMdi->bIsPacket) nPacketID = pMdi->nActionID;
			BOOL bAtLeastOneTemplateMerged = FALSE;
			//Go through all the templates in this merge
			for(int nTemplate = 0; nTemplate < pMdi->arTemplates.GetSize() && !bStopMerging; nTemplate++) {
				//Increment our counter.
				nDocumentsMerged++;

				MergeTemplate mt = pMdi->arTemplates[nTemplate];
				CString strTemplateName = VarString(GetTableField("MergeTemplatesT", "Path", "ID", mt.nMergeTemplateID));
							
				//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
				if(strTemplateName.GetAt(0) == '\\' && strTemplateName.GetAt(1) != '\\') {
					strTemplateName = GetSharedPath() ^ strTemplateName;
				}
					
				if(!DoesExist(strTemplateName) ) {
					//Have we already told the user that this will be skipped?
					bool bWarnedAlready = false;
					for(int nSkipped = 0; nSkipped < arSkippedTemplates.GetSize() && !bWarnedAlready; nSkipped++) {
						if(arSkippedTemplates[nSkipped] == strTemplateName) bWarnedAlready = true;
					}
					if(!bWarnedAlready) MsgBox("The template '%s' could not be found.  This template will be skipped.", strTemplateName);
				}
				else {			
					/// Generate the temp table
					CString strPatientIDs;
					for(int nRow = 0; nRow < pMdi->arRows.GetSize(); nRow++) {
						IRowSettingsPtr pRow(pMdi->arRows[nRow]);
						strPatientIDs += FormatString("%li,", VarLong(pRow->GetValue(mlcPatientID)));
					}
					ASSERT(!strPatientIDs.IsEmpty());
					strPatientIDs.TrimRight(",");
					CString strSql;
					strSql.Format("SELECT ID FROM PersonT WHERE ID IN (%s)", strPatientIDs);
					CString strMergeT = CreateTempIDTable(strSql, "ID");
						
					// Merge
					CMergeEngine mi;

					// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
					mi.LoadSenderInfo(FALSE);

					mi.m_arydwProcInfoIDs.RemoveAll();
					for(int nProc = 0; nProc < mt.arProcIDs.GetSize(); nProc++) {
						mi.m_arydwProcInfoIDs.Add(mt.arProcIDs[nProc]);
					}
					for(int nMed = 0; nMed < mt.arMedicationIDs.GetSize(); nMed++) {
						mi.m_arydwPrescriptionIDs.Add(mt.arMedicationIDs[nMed]);
					}

					// (a.walling 2007-12-04 13:45) - PLID 28272 - Give the PicID to the merge engine
					mi.m_nPicID = mt.nPicID;

					if(mi.m_arydwProcInfoIDs.GetSize()) {
						// (b.cardillo 2005-05-18 14:35) - PLID 16557 - Even if this is a multi-procedure 
						// template, we are still generating ONE document, so we want the page numbers to 
						// be considered continuous, rather than resetting to 1 on each record.
						mi.m_nFlags |= BMS_CONTINUOUS_PAGE_NUMBERING;
					}
					
					//We've gathered our info, go ahead and merge.
					if (g_bMergeAllFields)
						mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

					//mi.m_nFlags |= BMS_SAVE_FILE; //save the file, do not save in history
					
					if(nPacketID != -1 && nMergedPacketID == -1) {
						nMergedPacketID = NewNumber("MergedPacketsT", "ID");
						ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID) "
							"VALUES (%li, %li)", nMergedPacketID, nPacketID);
					}

					// (b.cardillo 2005-05-20 12:46) - PLID 15553 - Include the article number and count 
					// as merge fields, and then increment the number.
					mi.m_lstExtraFields.RemoveAll();
					mi.m_lstExtraFields.Add("Packet_Article_Number", AsString((long)nTemplate));
					mi.m_lstExtraFields.Add("Packet_Article_Count", AsString((long)pMdi->arTemplates.GetSize()));

					// Do the merge
					if(IsDlgButtonChecked(IDC_MERGE_BATCH_TO_PRINTER)) {
						mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
					}
					mi.m_nFlags = (mi.m_nFlags | BMS_SAVE_FILE_AND_HISTORY);
					CString strExtraProgress = nDocTotal > 1 ? FormatString("Template %i of %i", nDocumentsMerged, nDocTotal) : "";
					// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
					if (mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT, "", nMergedPacketID, -1, false, strExtraProgress)) {
						bAtLeastOneTemplateMerged = TRUE;
					}
					else {
						bStopMerging = TRUE;
					}
				}
			}

			//If this was a packet, we need to update the tracking ourselves (the MergeEngine code will handle the templates).
			if(pMdi->bIsPacket && bAtLeastOneTemplateMerged) {
				for(int nRow = 0; nRow < pMdi->arRows.GetSize(); nRow++) {
					IRowSettingsPtr pRow(pMdi->arRows[nRow]);
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PacketSent, VarLong(pRow->GetValue(mlcPatientID)), COleDateTime::GetCurrentTime(), nMergedPacketID, false, VarLong(pRow->GetValue(mlcStepID)));
				}
			}

		}

		//Cleanup
		for(int i = 0; i < arMergeDocuments.GetSize(); i++) {
			delete arMergeDocuments[i];
		}
		arMergeDocuments.RemoveAll();

		//Now all those things we just merged aren't in the list any more, but new steps may have become active.  Update the list
		// to reflect that.
		RefreshList();

	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnMergeAll()");

}

void CBatchMergeTrackingDlg::OnEditingStartingBatchMergeList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) return;

		switch(nCol) {
		case mlcScope:
			//If this is a packet, the "Scope" field is n/a.
			if(VarLong(pRow->GetValue(mlcAction)) == PhaseTracking::PA_WritePacket) {
				*pbContinue = FALSE;
			} else {
				//TES 7/16/2010 - PLID 39400 - This is now always valid for template steps, so just make sure they selected something.
				/*// (a.walling 2007-12-04 13:35) - PLID 28272 - If the checkbox is invalid (hidden) then don't process anything.
				_variant_t varScope = pRow->GetValue(mlcScope);
				if (varScope.vt == VT_EMPTY || varScope.vt == VT_NULL) {
					*pbContinue = FALSE;
				}*/
			}
			break;
		case mlcSelected:
			//If they haven't selected an action for this row, they can't check it off.
			if(pRow->GetValue(mlcActionID).vt == VT_NULL) {
				*pbContinue = FALSE;
			}
			break;
		}
	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnEditingStartingBatchMergeList()");
}

void CBatchMergeTrackingDlg::OnEditingFinishedBatchMergeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		IRowSettingsPtr pEditedRow(lpRow);
		if(pEditedRow == NULL) return;

		switch(nCol) {
		case mlcScope:
			if(bCommit) {
				
				//TES 7/16/2010 - PLID 39400 - This is now a dropdown, not a checkbox.
				long nNewVal = VarLong(varNewValue);
				//If there is at least one other row that is for this template and whose value is different, ask if they want to
				// update all of them.
				bool bPromptedUser = false;
				bool bChangeAll = true;
				IRowSettingsPtr pRow = m_pList->GetFirstRow();
				while(pRow != NULL && bChangeAll) {
					if(pRow != pEditedRow) {
						if(VarLong(pRow->GetValue(mlcActionID),-1) == VarLong(pEditedRow->GetValue(mlcActionID),-2)) {
							if(VarLong(pRow->GetValue(mlcScope)) != nNewVal) {
								if(!bPromptedUser) {
									if(IDYES != MsgBox(MB_YESNO, "Would you like to apply this change to all steps that use this template?")) {
										bChangeAll = false;
									}
									bPromptedUser = true;
								}
								if(bChangeAll) {
									pRow->PutValue(mlcScope, nNewVal);
								}
							}
						}
					}
					pRow = pRow->GetNextRow();
				}
			}
			break;
		}

	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnEditingFinishedBatchMergeList()");
						
}

void CBatchMergeTrackingDlg::RefreshList()
{
	//Disable the interface while we load.
	GetDlgItem(IDC_MERGE_ALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_SELECT_ALL_STEPS)->EnableWindow(FALSE);
	GetDlgItem(IDC_UNSELECT_ALL_STEPS)->EnableWindow(FALSE);
	SetDlgItemText(IDC_MERGE_ALL, "Loading...");
	GetDlgItem(IDC_BATCH_MERGE_LIST)->EnableWindow(FALSE);

	//Calculate the WHERE clause based on their user filter.
	long nFilteredUser = -1;
	//This is the "base" filter
	CString strWhere = 
		"LadderStatusT.IsActive = 1 AND StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) \r\n"
		"	AND NOT EXISTS (SELECT ID FROM StepsT LowerStepsT WHERE StepsT.LadderID = LowerStepsT.LadderID \r\n"
		"		AND LowerStepsT.StepOrder < StepsT.StepOrder AND LowerStepsT.ID NOT IN \r\n"
		"		(SELECT StepID FROM EventAppliesT)) AND StepTemplatesT.Action IN (9,10) \r\n";

	if(m_pUserCombo->CurSel != NULL) {
		nFilteredUser = VarLong(m_pUserCombo->CurSel->GetValue(0));
	}
	if(nFilteredUser != -1) {
		// (j.jones 2008-11-26 15:57) - PLID 30830 - supported multiple users per step
		strWhere += FormatString(" AND (StepsT.ID IN (SELECT StepID FROM StepsAssignToT WHERE UserID = %li) OR (StepsT.ID NOT IN (SELECT StepID FROM StepsAssignToT) AND LaddersT.UserID = %li)) \r\n", nFilteredUser, nFilteredUser);
	}

	// (z.manning 2010-06-25 14:18) - PLID 39369
	BOOL bShowOnHoldLadders = (m_btnShowOnHoldLadders.GetCheck() == BST_CHECKED);
	if(!bShowOnHoldLadders) {
		strWhere += " AND (StepsT.ActiveDate IS NULL OR StepsT.ActiveDate <= GetDate()) \r\n";
	}

	m_pList->WhereClause = _bstr_t(strWhere);
	m_pList->Requery();
}

void CBatchMergeTrackingDlg::OnSelChangingTrackingUsers(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if(lppNewSel == NULL) {
		//Don't let them select no row.
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
}

void CBatchMergeTrackingDlg::OnSelChosenTrackingUsers(LPDISPATCH lpRow) 
{
	try {
		//They've changed users, so, just refresh the list.
		RefreshList();
	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnSelChosenTrackingUsers()");
}

void CBatchMergeTrackingDlg::OnSelectAllSteps() 
{
	try {
		IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow) {
			if(!VarBool(pRow->GetValue(mlcSelected),TRUE)) {
				pRow->PutValue(mlcSelected, g_cvarTrue);
			}
			pRow = pRow->GetNextRow();
		}
	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnSelectAllSteps()");
}

void CBatchMergeTrackingDlg::OnUnselectAllSteps() 
{
	try {
		IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow) {
			if(VarBool(pRow->GetValue(mlcSelected),FALSE)) {
				pRow->PutValue(mlcSelected, g_cvarFalse);
			}
			pRow = pRow->GetNextRow();
		}
	}NxCatchAll("Error in CBatchMergeTrackingDlg::OnUnselectAllSteps()");
}

// (z.manning 2010-06-25 14:03) - PLID 39369
void CBatchMergeTrackingDlg::OnShowOnHoldLadders()
{
	try
	{
		RefreshList();
		SetRemotePropertyInt("BatchMergeTrackingShowOnHoldLadders", m_btnShowOnHoldLadders.GetCheck(), 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}