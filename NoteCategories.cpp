// NoteCategories.cpp : implementation file
//

#include "stdafx.h"
#include "NoteCategories.h"
#include "Globalutils.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "EMRMergePrecedenceDlg.h"
#include "ListMergeDlg.h"
#include "DontShowDlg.h"
#include "AuditTrail.h"
#include "PatientDashboardDlg.h"
#include "ConfigurePatDashboardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

#define  IDM_ADD 123337
#define	 IDM_DELETE 123983
#define	 IDM_SET_DEFAULT 123984
#define	 IDM_REMOVE_DEFAULT 123985

// (j.jones 2012-04-17 16:49) - PLID 13109 - added enum for columns
enum NoteCatColumns {

	nccID = 0,
	nccDescription,
	nccPatientTab,
	nccContactTab,
	nccSortOrder,
};

/////////////////////////////////////////////////////////////////////////////
// CNoteCategories dialog

CNoteCategories::CNoteCategories(CWnd* pParent)
	: CNxDialog(CNoteCategories::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNoteCategories)
	m_bEditingNoteCat = false;
	m_bEditingFollowUpCat = false;
	m_bEditingHistoryCat = false;
	//}}AFX_DATA_INIT
}


void CNoteCategories::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNoteCategories)
	DDX_Control(pDX, IDC_ADV_EMR_MERGE_SETUP, m_advEmrMergeSetupButton);
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_NEW, m_newButton);
	DDX_Control(pDX, IDC_COMBINE_CATEGORIES, m_btnCombineCategories);
	DDX_Control(pDX, IDC_CHECK_SORT_NOTECATS_ALPHABETICALLY, m_checkSortAlphabetically);
	DDX_Control(pDX, IDC_BTN_MOVE_NOTECAT_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BTN_MOVE_NOTECAT_DOWN, m_btnMoveDown);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNoteCategories, CNxDialog)
	//{{AFX_MSG_MAP(CNoteCategories)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADV_EMR_MERGE_SETUP, OnAdvEmrMergeSetup)
	ON_BN_CLICKED(IDC_COMBINE_CATEGORIES, OnCombineCategories)
	ON_BN_CLICKED(IDC_CHECK_SORT_NOTECATS_ALPHABETICALLY, OnCheckSortNotecatsAlphabetically)
	ON_BN_CLICKED(IDC_BTN_MOVE_NOTECAT_UP, OnBtnMoveNotecatUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_NOTECAT_DOWN, OnBtnMoveNotecatDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNoteCategories, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNoteCategories)
	ON_EVENT(CNoteCategories, IDC_NOTECATS, 17 /* RepeatedLeftClick */, OnRepeatedLeftClickNotecats, VTS_VARIANT VTS_I4 VTS_I4)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 5 /* LButtonUp */, OnLButtonUpNxdlnotecats, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 10 /* EditingFinished */, OnEditingFinishedNxdlnotecats, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 7 /* RButtonUp */, OnRButtonUpNxdlnotecats, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 18 /* RequeryFinished */, OnRequeryFinishedNxdlnotecats, VTS_I2)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 2 /* SelChanged */, OnSelChangedNxdlnotecats, VTS_I4)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 9 /* EditingFinishing */, OnEditingFinishingNxdlnotecats, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 14, OnDragEndNxdlnotecats, VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CNoteCategories, IDC_NXDLNOTECATS, 12, OnDragBeginNxdlnotecats, VTS_PBOOL VTS_I4 VTS_I2 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoteCategories message handlers

BOOL CNoteCategories::OnInitDialog() 
{
	//(e.lally 2011-11-18) PLID 46539 - Added try/catch
	try {
		CNxDialog::OnInitDialog();
		
		m_pNxDlNoteCats = BindNxDataListCtrl(IDC_NXDLNOTECATS);
		
		m_newButton.AutoSet(NXB_NEW);
		m_deleteButton.AutoSet(NXB_DELETE);
		m_okButton.AutoSet(NXB_CLOSE); // (c.haag 2008-05-20 13:52) - PLID 29790 - The OK button should be Close
		m_advEmrMergeSetupButton.AutoSet(NXB_MODIFY);
		m_btnCombineCategories.AutoSet(NXB_MODIFY); //(e.lally 2011-07-07) PLID 31585 
		// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);

		if(m_pNxDlNoteCats->GetRowCount() == 0){
			GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		}
		else{
			m_pNxDlNoteCats->PutCurSel(0);
		}

		//(e.lally 2011-11-18) PLID 46539 - Disable buttons the user doesn't have permissions for
		CPermissions curUserPermissions = GetCurrentUserPermissions(bioHistoryCategories);
		if(!(curUserPermissions & sptCreate)){
			GetDlgItem(IDC_NEW)->EnableWindow(FALSE);
		}
		if(!(curUserPermissions & sptDelete)){
			GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		}
		if(!(curUserPermissions & sptDynamic0)){
			GetDlgItem(IDC_COMBINE_CATEGORIES)->EnableWindow(FALSE);
		}

		//TES 8/31/2010 - PLID 39740 - Cached properties, added LabFollowUpDefaultCategory
		g_propManager.CachePropertiesInBulk("NoteCategories", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefaultNoteCatID' OR "
			"Name = 'DefaultFollowUpCatID' OR "
			"Name = 'DefaultHistoryCatID' OR "
			"Name = 'LabFollowUpDefaultCategory' "
			"OR Name = 'SortHistoryTabsAlphabetically' " // (j.jones 2012-04-17 16:44) - PLID 13109
			")", _Q(GetCurrentUserName()));

		// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
		BOOL bSortAlpha = (GetRemotePropertyInt("SortHistoryTabsAlphabetically", 1, 0, "<None>", true) == 1);
		m_checkSortAlphabetically.SetCheck(bSortAlpha);
		ReflectAlphabeticSorting(bSortAlpha);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNoteCategories::OnLeftClickNotecats(const VARIANT FAR& varBoundValue, long iColumn) 
{
	return;
}

void CNoteCategories::OnRightClickNotecats(const VARIANT FAR& varBoundValue, long iColumn) 
{
	
}

void CNoteCategories::OnPopupSelectionNotecats(long iItemID) 
{

}

void CNoteCategories::OnRepeatedLeftClickNotecats(const VARIANT FAR& varBoundValue, long iColumn, long nClicks) 
{
}

void CNoteCategories::OnOK() 
{
	CDialog::OnOK();
}

void CNoteCategories::OnLButtonUpNxdlnotecats(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (m.cable 06/14/2004 10:21) - this is now handled in OnSelChangedNxdlnotecats
	/*if(m_pNxDlNoteCats->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
	}*/


}

//(e.lally 2011-11-18) PLID 46539 - Added
void CNoteCategories::OnEditingFinishingNxdlnotecats(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	try {
		if (!(*pbCommit)) {
			return;
		}

		if(nCol == nccDescription){ //Category name
			CString strOld, strNew;
			if (varOldValue.vt != VT_EMPTY){
				strOld = VarString(varOldValue, "");
			}
			if(pvarNewValue != NULL && pvarNewValue->vt != VT_EMPTY){
				strNew = VarString(*pvarNewValue, "");
			}
			if(strOld == strNew){
				//We don't need to check permissions if the name isn't changing
				*pbCommit = FALSE;
				return;
			}
		}

		//(e.lally 2011-11-18) PLID 46539 - Check write permissions
		if(!CheckCurrentUserPermissions(bioHistoryCategories, sptWrite)){
			*pbCommit = FALSE;
			return;
		}
	}NxCatchAll(__FUNCTION__);

}
void CNoteCategories::OnEditingFinishedNxdlnotecats(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (!bCommit)
			return;

		// (c.haag 2005-01-03 17:31) - PLID 15173 - Check the data types before checking for
		// matching old and new values
		if (varOldValue.vt == VT_BSTR && varNewValue.vt == VT_BSTR)
		{
			if (VarString(varOldValue) == VarString(varNewValue))
				return;
		}

		long nID = -1;
		if(nCol == nccDescription) {
			_variant_t var = m_pNxDlNoteCats->GetValue(nRow, nccID);
			CString strValue;
			strValue = VarString(varNewValue);
			strValue.TrimRight();
			if(var.vt == VT_NULL || var.vt == VT_EMPTY || strValue == "") {
				//We used to just ask people about this, saying that it was "inadvisable" to have blank category names.
				//But it turns out it's more than just "inadvisable", it's really bad, so we won't give them the option.
				MsgBox("You cannot enter a category name which is a blank string.");
				//don't save and set the value of the column back to what it was
				bCommit = false;
				m_pNxDlNoteCats->PutValue(nRow, nccDescription, varOldValue);
			}
			else {
				//m.carlson 7/7/2004 1:04 pm PL 13346
				//don't want to let them enter duplicate names
				_RecordsetPtr rs = CreateRecordset("Select ID FROM NoteCatsF WHERE Description = '%s'", _Q(strValue));
				if(!rs->eof){
					rs->Close();
					AfxMessageBox("You must enter a unique category name - this category name already exists.");
					bCommit = false;
					m_pNxDlNoteCats->PutValue(nRow, nccDescription, varOldValue);
					
					return;	//you already exist
				}
				rs->Close();

				//things are good, lets save it!
				nID = VarLong(var);

				if(strValue.GetLength() > 50){
					AfxMessageBox("This field may only contain 50 characters.  The string will be truncated.");
					strValue = strValue.Left(50);
					m_pNxDlNoteCats->PutValue(nRow, nCol, _variant_t(strValue));
				}

				// (z.manning, 08/07/2007) - PLID 26960 - Warn if renaming categories that are in use somewhere.
				_RecordsetPtr prsCounts = CreateParamRecordset(
					"SELECT \r\n"
					"	(SELECT COUNT(*) FROM ToDoList WHERE CategoryID = {INT}) AS ToDoCount, \r\n"
					"	(SELECT COUNT(*) FROM Notes WHERE Category = {INT}) AS NoteCount, \r\n"
					"	(SELECT COUNT(*) FROM MailSent WHERE CategoryID = {INT}) AS MailCount, \r\n"
					"	(SELECT COUNT(*) FROM PacketsT WHERE PacketCategoryID = {INT}) AS PacketCount \r\n"
					, nID, nID, nID, nID);
				long nToDoCount = AdoFldLong(prsCounts, "ToDoCount", 0);
				long nNoteCount = AdoFldLong(prsCounts, "NoteCount", 0);
				long nMailCount = AdoFldLong(prsCounts, "MailCount", 0);
				long nPacketCount = AdoFldLong(prsCounts, "PacketCount", 0);
				if(nToDoCount != 0 || nNoteCount != 0 || nMailCount != 0 || nPacketCount != 0)
				{
					int nResult = MessageBox(FormatString(
						"Changing this category will change it in all of the following places:\r\n"
						" -- %li to-do tasks\r\n"
						" -- %li patient notes\r\n" // (z.manning, 08/07/2007) - Yes, I realize these aren't necessarily patient notes, but users may not understand if we just say notes.
						" -- %li history documents\r\n"
						" -- %li packets\r\n"
						"\r\nAre you sure you want to rename this category?", nToDoCount, nNoteCount, nMailCount, nPacketCount)
						, NULL, MB_YESNO);
					if(nResult != IDYES) {
						bCommit = FALSE;
						m_pNxDlNoteCats->PutValue(nRow, nccDescription, varOldValue);
						return;
					}
				}
				
				//TES 8/8/2011 - PLID 44716 - We need to update the user-defined permission as well
				CSqlTransaction sqlTran;
				sqlTran.Begin();
				ExecuteSql("Update NoteCatsF Set Description = '%s' WHERE ID = %li", _Q(strValue), nID);
				long nSecurityObjectID = UpdateUserDefinedPermissionNameInData(bioHistoryCategories, nID, strValue);
				sqlTran.Commit();
				UpdateUserDefinedPermissionNameInMemory(nSecurityObjectID, strValue);

				//(e.lally 2011-07-08) PLID 44495 - Audit the change
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiHistoryCategoryChanged, nID, VarString(varOldValue), strValue, aepMedium, aetChanged);

				CClient::RefreshTable(NetUtils::NoteCatsF, nID);

				// (j.jones 2012-04-17 17:21) - PLID 13109 - re-sort the list
				m_pNxDlNoteCats->Sort();

				UpdateArrowButtons();
			}
		}
		else if(nCol == nccPatientTab) {
			ExecuteSql("UPDATE NoteCatsF SET IsPatientTab = %li WHERE ID = %li", VarBool(varNewValue) ? 1 : 0, VarLong(m_pNxDlNoteCats->GetValue(nRow, nccID)));
		}
		else if(nCol == nccContactTab) {
			ExecuteSql("UPDATE NoteCatsF SET IsContactTab = %li WHERE ID = %li", VarBool(varNewValue) ? 1 : 0, VarLong(m_pNxDlNoteCats->GetValue(nRow, nccID)));
		}


	}NxCatchAll("Error in CNoteCategories::OnEditingFinishedNxdlnotecats in NoteCategories.cpp");
}

void CNoteCategories::OnRButtonUpNxdlnotecats(long nRow, short nCol, long x, long y, long nFlags) 
{
	//(e.lally 2011-11-18) PLID 46539 - Added try/catch
	try {
		//set the selection when you right click so that you can delete the item
		m_pNxDlNoteCats->PutCurSel(nRow);
		
		CMenu menPopup;
		//(e.lally 2011-11-18) PLID 46539 - Get permissions and disable menu options accordingly
		CPermissions curUserPermissions = GetCurrentUserPermissions(bioHistoryCategories);
		//Initialize all flags to the same thing.
		DWORD dwCreatePermitted, dwWritePermitted, dwDeletePermitted;
		dwCreatePermitted = dwWritePermitted = dwDeletePermitted = (MF_DISABLED | MF_GRAYED);
		if((curUserPermissions & sptCreate)){
			dwCreatePermitted = MF_ENABLED;
		}
		if((curUserPermissions & sptWrite)){
			dwWritePermitted = MF_ENABLED;
		}
		if((curUserPermissions & sptDelete)){
			dwDeletePermitted = MF_ENABLED;
		}

		if (nRow != -1) {
			menPopup.m_hMenu = CreatePopupMenu();
			//(e.lally 2011-11-18) PLID 46539 - Get permissions and disable accordingly
			menPopup.InsertMenu(0, MF_BYPOSITION|dwDeletePermitted, IDM_DELETE, "Delete");
			menPopup.InsertMenu(1, MF_BYPOSITION|dwCreatePermitted, IDM_ADD, "Add");

			if (!m_bEditingHistoryCat)
			{
				//(e.lally 2011-11-18) PLID 46539 - Get permissions and disable accordingly
				if(IsDefaultSelected()){
					menPopup.InsertMenu(2, MF_BYPOSITION|dwWritePermitted, IDM_REMOVE_DEFAULT, "Remove Default");
				}
				else{
					menPopup.InsertMenu(2, MF_BYPOSITION|dwWritePermitted, IDM_SET_DEFAULT, "Set As Default");
				}
			}
		}
		else
		{
			menPopup.m_hMenu = CreatePopupMenu();
			//(e.lally 2011-11-18) PLID 46539 - Get permissions and disable accordingly
			menPopup.InsertMenu(1, MF_BYPOSITION|dwCreatePermitted, IDM_ADD, "Add");
		}

		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* dlNotes = GetDlgItem(IDC_NXDLNOTECATS);
		if (dlNotes != NULL) {
			dlNotes->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else {
			HandleException(NULL, "An error ocurred while creating menu");
		}
	} NxCatchAll(__FUNCTION__);
}



BOOL CNoteCategories::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {

		case IDM_ADD:
			OnNew();
		break;

		case IDM_DELETE:
			OnDelete();
		break;

		case IDM_SET_DEFAULT:
			SetAsDefault();
		break;
		
		case IDM_REMOVE_DEFAULT:
			RemoveDefault();
		break;

		return true;
	}

	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CNoteCategories::OnCancel()
{
	CDialog::OnCancel();
}

void CNoteCategories::OnNew() 
{
	//(e.lally 2011-11-18) PLID 46539 - Added try/catch
	try {
		//(e.lally 2011-11-18) PLID 46539 - Check create permissions
		if(!CheckCurrentUserPermissions(bioHistoryCategories, sptCreate)){
			return;
		}

		CString strResult;
		int nResult = InputBoxLimited(this, "Enter Category Name", strResult, "",50,false,false,NULL);

		strResult.TrimRight();

		if (nResult == IDOK)
		{
			while(strResult == "") {
				MessageBox("You cannot have a category name which is a blank string.");
				if(InputBoxLimited(this, "Enter Category Name", strResult, "",50,false,false,NULL) == IDCANCEL)
					return;
				strResult.TrimRight();
			}
			EnsureRemoteData();

			//check to see if it's too long
			if(strResult.GetLength() > 50)
			{
				AfxMessageBox("This category is longer than the allowed size (50 characters).  It will be truncated to fit accordingly");
				strResult = strResult.Left(50);
			}
			
			//don't want to enter this twice
			_RecordsetPtr rs = CreateRecordset("Select ID FROM NoteCatsF WHERE Description = '%s'", _Q(strResult));
			if(!rs->eof){
				AfxMessageBox("You cannot enter 2 items with the same name.  Please enter a new item.");
				return;	//you already exist
			}

			//TES 8/1/2011 - PLID 44716 - Moved to GlobalUtils function
			// (j.jones 2012-04-17 17:05) - PLID 13109 - this now generates a new sort order,
			// it would be the next highest unique SortOrder value
			long nSortOrder = -1;
			long nID = CreateNoteCategory(strResult, false, &nSortOrder);
			IRowSettingsPtr pRow;
			pRow = m_pNxDlNoteCats->GetRow(-1);
			pRow->PutValue(nccID, nID);
			pRow->PutValue(nccDescription, _variant_t(strResult));
			pRow->PutValue(nccPatientTab, _variant_t(false));
			pRow->PutValue(nccContactTab, _variant_t(false));
			pRow->PutValue(nccSortOrder, nSortOrder);
			m_pNxDlNoteCats->AddRow(pRow);
			m_pNxDlNoteCats->Sort();

			UpdateArrowButtons();

			CClient::RefreshTable(NetUtils::NoteCatsF, nID);
		}	
	}NxCatchAll("Error is CNoteCategories::OnClickAdd() in NoteCategories.cpp \n Could not add note category");
}

void CNoteCategories::OnDelete() 
{
	long nID;
	try {
		//(e.lally 2011-11-18) PLID 46539 - Check delete permissions
		if(!CheckCurrentUserPermissions(bioHistoryCategories, sptDelete)){
			return;
		}

		long nCurSel  = m_pNxDlNoteCats->GetCurSel();

		if(nCurSel==-1)
			return;
		nID = VarLong(m_pNxDlNoteCats->GetValue(nCurSel, nccID));
		CString strCat = VarString(m_pNxDlNoteCats->GetValue(nCurSel, nccDescription));

		//warn them to make sure that they really want to delete this stuff
		// (c.haag 2008-06-23 16:00) - PLID 30471 - Cleaned up the warning construction, and we now also include EMRActionsTodoDataT
		// (b.savon 2013-05-29 13:44) - PLID 42902 - Added DefaultCategoryID for plugins
		// (r.goldschmidt 2015-03-18 12:32) - PLID 64643 - bug fix in query
		CWaitCursor wc;
		CString strWarning = "The following items are using this category:\r\n\r\n";
		BOOL bShowWarning = FALSE;
		_RecordsetPtr prs = CreateParamRecordset(
			"DECLARE @CategoryID INT;\r\n"
			"SET @CategoryID = {INT};\r\n"
			"SELECT Count(ID) as ItemCount FROM Notes Where Category = @CategoryID;\r\n"
			"SELECT Count(TaskID) as ItemCount FROM ToDoList Where CategoryID = @CategoryID;\r\n"
			"SELECT Count(MailID) AS ItemCount FROM MailSent WHERE CategoryID = @CategoryID;\r\n"
			"SELECT COUNT(ActionID) AS ItemCount FROM EMRActionsTodoDataT INNER JOIN EmrActionsT ON EmrActionsT.ID = EMRActionsTodoDataT.ActionID WHERE CategoryID = @CategoryID AND EmrActionsT.Deleted = 0;\r\n"
			"SELECT COUNT(ID) AS ItemCount FROM DevicePluginConfigT WHERE DefaultCategoryID = @CategoryID; \r\n"
			,nID);

		// Notes
		long nCount = AdoFldLong(prs, "ItemCount");
		if (nCount > 0) {
			strWarning += FormatString("%li note(s)\r\n", nCount);
			bShowWarning = TRUE;
		}

		// Tasks
		prs = prs->NextRecordset(NULL);
		nCount = AdoFldLong(prs, "ItemCount");
		if (nCount > 0) {
			strWarning += FormatString("%li task(s)\r\n", nCount);
			bShowWarning = TRUE;
		}

		// History items
		prs = prs->NextRecordset(NULL);
		nCount = AdoFldLong(prs, "ItemCount");
		if (nCount > 0) {
			strWarning += FormatString("%li history item(s)\r\n", nCount);
			bShowWarning = TRUE;
		}

		// Active EMR todo spawning actions
		prs = prs->NextRecordset(NULL);
		nCount = AdoFldLong(prs, "ItemCount");
		if (nCount > 0) {
			strWarning += FormatString("%li EMR Todo spawning action(s)\r\n", nCount);
			bShowWarning = TRUE;
		}

		// (b.savon 2013-05-29 13:44) - PLID 42902
		// DevicePlugin default category IDs
		prs = prs->NextRecordset(NULL);
		nCount = AdoFldLong(prs, "ItemCount");
		if (nCount > 0) {
			strWarning += FormatString("%li device plugin(s)\r\n", nCount);
			bShowWarning = TRUE;
		}

		//TES 8/31/2010 - PLID 39740 - Don't let them delete the default category for lab followups.
		long nLabDefault = GetRemotePropertyInt("LabFollowUpDefaultCategory", -1, 0, "<None>");
		if(nLabDefault == nID) {
			strWarning += "The default category for Lab to-do tasks\r\n";
			bShowWarning = TRUE;
		}

		if (bShowWarning) {
			//finish up the warning
			strWarning += "\r\nPlease change these before attempting to delete this category.";
			MsgBox(strWarning);
		}
		else {
			//There aren't any so we can warn them and delete it
			if (IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO, "Are you sure you want to delete this category?")) {
				//TES 8/2/2011 - PLID 44716 - We'll be removing a permission, track its ID
				long nSecurityObjectID = -1;
				BEGIN_TRANS("Delete Category");
				{
					// (z.manning 2008-10-13 09:57) - PLID 21108 - Lab to-do cateogry ID
					ExecuteParamSql("UPDATE LabProcedureStepsT SET TodoCategoryID = NULL WHERE TodoCategoryID = {INT}", nID);
					// (z.manning 2008-07-03 10:53) - PLID 25574 - Delete from EmrHistoryCategoryLinkT
					ExecuteParamSql("DELETE FROM EmrHistoryCategoryLinkT WHERE NoteCategoryID = {INT}", nID);
					// a.walling 4/28/06 PLID 20322 Remove the category from PacketsT to avoid Foreign Key errors
					ExecuteSql("UPDATE PacketsT SET PacketCategoryID = NULL WHERE PacketCategoryID = %li", nID);
					ExecuteSql("DELETE FROM EMRWordTemplateCategoryT WHERE NoteCatID = %li", nID);
					ExecuteSql("UPDATE NoteMacroT SET CategoryID = NULL WHERE CategoryID = %i", nID);
					ExecuteSql("UPDATE ToDoList SET CategoryID = NULL WHERE CategoryID = %li", nID);
					ExecuteSql("UPDATE Notes SET Category = NULL WHERE Category = %li", nID);
					// (c.haag 2008-06-23 16:29) - PLID 30471 - Clear out EMRActionsTodoDataT
					ExecuteParamSql("UPDATE EMRActionsTodoDataT SET CategoryID = NULL WHERE CategoryID = {INT}", nID);
					// (a.walling 2008-06-23 15:02) - PLID 30475 - Clear out references from StepTemplatesT
					ExecuteSql("UPDATE StepTemplatesT SET TodoCategory = NULL WHERE TodoCategory = %li", nID);
					// (z.manning 2009-12-11 17:26) - PLID 36519 - Handle ExportHistoryCategoriesT
					ExecuteParamSql("DELETE FROM ExportHistoryCategoriesT WHERE CategoryID = {INT}", nID);
					//TES 8/2/2011 - PLID 44716 - Remove the permission object for this category
					nSecurityObjectID = DeleteUserDefinedPermissionFromData(bioHistoryCategories, nID);
					// (j.jones 2012-04-18 09:51) - PLID 13109 - update remaining sortorders
					ExecuteParamSql("UPDATE NoteCatsF SET SortOrder = SortOrder - 1 WHERE SortOrder > (SELECT SortOrder FROM NoteCatsF WHERE ID = {INT})", nID);
					ExecuteParamSql("DELETE FROM NoteCatsF WHERE ID = {INT}", nID);
					// (z.manning, 01/11/2008) - PLID 28600 - Make sure we clear out any remote properties that use this category.
					ExecuteSql("UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = %li AND Name IN ('AttendanceToDoCategory')", nID);
					// (r.goldschmidt 2014-08-05 13:12) - PLID 62717 - Make sure we clear out any remote properties that use this category and default is zero
					ExecuteSql("UPDATE ConfigRT SET IntParam = 0 WHERE IntParam = %li AND Name IN ('SttmntSendToHistoryCategory')", nID);
					// (j.gruber 2015-02-10 08:40) - PLID 64392 - Rescheduling Queue - new category preference
					ExecuteSql("UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = %li AND Name IN ('ApptNotesDefaultCategory')", nID);
					// (r.farnworth 2016-03-15 09:46) - PLID 68453 - We need to clear the Online Visits default category if it matches the one we are deleting
					if (GetRemotePropertyInt("OnlineVisitsDocumentCategory", -1, 0, "<None>") == nID) {
						SetRemotePropertyInt("OnlineVisitsDocumentCategory", -1, 0, "<None>");
					}
					// (r.goldschmidt 2015-04-17 13:31) - PLID 64755 - For many preferences/settings that set a category for notes, the preferences are not set up to handle the deletion of that category
					ExecuteParamSql(R"(
DECLARE @CategoryID INT;
SET @CategoryID = {INT};
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('LineItemPosting_DedBillingNote_DefCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('ERemitPostAdjustmentNoteWhenZeroPays_DefaultNoteCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('ERemit_AddCASPR_BillingNote_DefCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('ERemit_AddDetailedInfo_DefCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('OMRDefaultImportCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('SentReminderNoteDefaultCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('PatientHistoryDefaultImageCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('PatientHistoryDefaultCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('LabAttachmentsDefaultCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('LabNotesDefaultCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('RecallNotesDefaultCategory');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('ParseOptionsDefaultCategoryID');
UPDATE ConfigRT SET IntParam = -1 WHERE IntParam = @CategoryID AND Name IN ('ScanMultiDocCategory');
						)", nID);
					
					if(IsDefaultSelected()){
						RemoveDefault();
					}

					// (c.haag 2016-05-05 14:59) - NX-100441 - Update the dashboard controls to no longer have the doomed filters
					CConfigurePatDashboardDlg::RemoveMissingFilterValues(pdtHistoryAttachments, HistoryCategory, "NoteCatsF", "ID");

					//(e.lally 2011-07-08) PLID 44495 - Audit the deletion
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiHistoryCategoryDeleted, nID, strCat, "<Deleted>", aepHigh, aetDeleted);
					
				}END_TRANS_CATCH_ALL("Delete Category");

				// (j.jones 2012-04-18 09:52) - PLID 13109 - requery so any affected sortorders reload properly
				m_pNxDlNoteCats->Requery();
				UpdateArrowButtons();

				//TES 8/2/2011 - PLID 44716 - Update our memory based on the permission object we just deleted
				DeleteUserDefinedPermissionFromMemory(nSecurityObjectID);

				//send the table checker message
				CClient::RefreshTable(NetUtils::NoteCatsF, nID);
			}
		}
		
	}NxCatchAll ("Please make sure that you have an item selected before clicking the delete button");	
}

//NOTE:  Technically, a person could set all of these.  They are just checked in a random (to the user) order, so 
//make sure you don't ever set more than one.
bool CNoteCategories::GetCurrentPropertyName(CString& strName)
{
	//TODO:  Ideally, this should really just be an enumerated value that we can do a switch on, instead of defining 
	//	a new boolean every time.  It would also avoid the possible problem of multiple bools being set.
	if(m_bEditingNoteCat){
		strName = "DefaultNoteCatID";
	}
	else if(m_bEditingFollowUpCat){
		strName = "DefaultFollowUpCatID";
	}
	else if(m_bEditingHistoryCat) {
		strName = "DefaultHistoryCatID";
	}
	else {
		//unknown - quit before highlighting anything
		return false;
	}

	return true;
}

void CNoteCategories::SetAsDefault()
{
	if(m_pNxDlNoteCats->CurSel == -1)
		return;

	CString strProperty;

	if(!GetCurrentPropertyName(strProperty))
		return;
	if (m_bEditingHistoryCat)
		return;

	//(e.lally 2011-11-18) PLID 46539 - Check write permissions
	if(!CheckCurrentUserPermissions(bioHistoryCategories, sptWrite)){
		return;
	}

	//DRT 11/11/2003 - Made this code not be ridiculously repeated.
	//first remove old color
	long nOldDefaultID = GetRemotePropertyInt(strProperty, NULL,0,"<None>",TRUE);
	int nRow = m_pNxDlNoteCats->FindByColumn(0,nOldDefaultID,0,FALSE);
	if(nRow >= 0)
		IRowSettingsPtr(m_pNxDlNoteCats->GetRow(nRow))->PutForeColor(RGB(0,0,0));

	//save the default
	long nNewDefaultID = VarLong(m_pNxDlNoteCats->GetValue(m_pNxDlNoteCats->CurSel, nccID));
	SetRemotePropertyInt(strProperty, nNewDefaultID ,0,"<None>");

	//now set colors
	IRowSettingsPtr(m_pNxDlNoteCats->GetRow(m_pNxDlNoteCats->CurSel))->PutForeColor(RGB(255,0,0));

}

void CNoteCategories::RemoveDefault()
{
	if(m_pNxDlNoteCats->CurSel == -1)
		return;

	CString strProperty;

	if(!GetCurrentPropertyName(strProperty))
		return;
	if (m_bEditingHistoryCat)
		return;

	//(e.lally 2011-11-18) PLID 46539 - Check write permissions
	if(!CheckCurrentUserPermissions(bioHistoryCategories, sptWrite)){
		return;
	}

	//DRT 11/11/2003 - Removed silly code duplication
	//first remove colors
	long nOldDefaultID = GetRemotePropertyInt(strProperty, NULL,0,"<None>",TRUE);
	int nRow = m_pNxDlNoteCats->FindByColumn(nccID, nOldDefaultID,0,FALSE);
	if(nRow >= 0)
		IRowSettingsPtr(m_pNxDlNoteCats->GetRow(nRow))->PutForeColor(RGB(0,0,0));

	//now remove the default
	SetRemotePropertyInt(strProperty, NULL,0,"<None>");
}

BOOL CNoteCategories::IsDefaultSelected()
{
	if(m_pNxDlNoteCats->CurSel == -1 )
		return FALSE;

	CString strProperty;

	if(!GetCurrentPropertyName(strProperty))
		return FALSE;
	if (m_bEditingHistoryCat)
		return FALSE;

	//DRT 11/11/2003 - Rewrote silly code duplication
	long nDefaultID = GetRemotePropertyInt(strProperty,NULL,0,"<None>",TRUE);
	int nRow = m_pNxDlNoteCats->FindByColumn(nccID,nDefaultID,0,FALSE);
	return (nRow == m_pNxDlNoteCats->CurSel);

}

void CNoteCategories::OnRequeryFinishedNxdlnotecats(short nFlags) 
{
	CString strProperty;

	if(!GetCurrentPropertyName(strProperty))
		return;
	if (m_bEditingHistoryCat)
		return;

	//DRT 11/11/2003 - Rewrote silly code duplication
	long nDefaultID = GetRemotePropertyInt(strProperty, NULL,0,"<None>",TRUE);
	int nRow = m_pNxDlNoteCats->FindByColumn(nccID,nDefaultID,0,FALSE);
	if(nRow >= 0)
		IRowSettingsPtr(m_pNxDlNoteCats->GetRow(nRow))->PutForeColor(RGB(255,0,0));

}

void CNoteCategories::OnSelChangedNxdlnotecats(long nNewSel) 
{
	//(e.lally 2011-11-18) PLID 46539 - Added try/catch
	try {

		// (j.jones 2012-04-17 18:01) - PLID 13109 - enable the arrows, if in use
		if(!m_checkSortAlphabetically.GetCheck()) {
			UpdateArrowButtons();
		}

		if(m_pNxDlNoteCats->GetCurSel() == sriNoRow){
			GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		}
		else{
			//(e.lally 2011-11-18) PLID 46539 - Get permissions and disable accordingly
			if(!(GetCurrentUserPermissions(bioHistoryCategories) & sptDelete)){
				GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNoteCategories::OnAdvEmrMergeSetup() 
{
	try {
		CEMRMergePrecedenceDlg dlg(this);
		dlg.b_OutOfEMR = true;
		dlg.DoModal();
	} NxCatchAll("Error in OnMergecat()");
}

//(e.lally 2011-07-07) PLID 31585 - Added support for combining categories
void CNoteCategories::OnCombineCategories()
{
	try {
		//(e.lally 2011-11-18) PLID 46539 - Check combine permissions
		if(!CheckCurrentUserPermissions(bioHistoryCategories, sptDynamic0)){
			return;
		}

		DontShowMeAgain(this, "Be careful when using this utility.\n"
			"You can change large amounts of data at once,\n"
			"and your changes cannot be undone", 
			"MergeNoteCategories");

		CListMergeDlg dlg(this);
		dlg.m_eListType = mltNoteCategories;
		dlg.DoModal();

		if(dlg.m_bCombined){
			m_pNxDlNoteCats->Requery();
			UpdateArrowButtons();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::OnCheckSortNotecatsAlphabetically()
{
	try {

		BOOL bSortAlpha = m_checkSortAlphabetically.GetCheck();

		SetRemotePropertyInt("SortHistoryTabsAlphabetically", bSortAlpha ? 1 : 0, 0, "<None>");

		ReflectAlphabeticSorting(bSortAlpha);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::ReflectAlphabeticSorting(BOOL bSortAlphabetically)
{
	try {

		//change the datalist sort, and show/hide the arrow buttons

		IColumnSettingsPtr pDescCol = m_pNxDlNoteCats->GetColumn(nccDescription);
		IColumnSettingsPtr pSortCol = m_pNxDlNoteCats->GetColumn(nccSortOrder);

		if(bSortAlphabetically) {
			pSortCol->PutSortPriority(-1);
			pDescCol->PutSortPriority(0);
			pDescCol->PutSortAscending(TRUE);
		}
		else {
			pDescCol->PutSortPriority(-1);
			pSortCol->PutSortPriority(0);
			pSortCol->PutSortAscending(TRUE);
		}
		m_pNxDlNoteCats->Sort();

		UpdateArrowButtons();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::OnBtnMoveNotecatUp()
{
	try {

		long nCurSel = m_pNxDlNoteCats->CurSel;

		if(m_checkSortAlphabetically.GetCheck() || nCurSel == sriNoRow || nCurSel == 0) {
			UpdateArrowButtons();
			return;
		}

		long nID1 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel, nccID));
		long nSort1 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel, nccSortOrder));
		long nID2 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel-1, nccID));
		long nSort2 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel-1, nccSortOrder));

		//swap the sort orders in data
		ExecuteParamSql("UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}; "
			"UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}; ", nSort2, nID1, nSort1, nID2);

		//swap the sort orders in the datalist
		m_pNxDlNoteCats->PutValue(nCurSel, nccSortOrder, nSort2);
		m_pNxDlNoteCats->PutValue(nCurSel-1, nccSortOrder, nSort1);

		m_pNxDlNoteCats->Sort();

		UpdateArrowButtons();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::OnBtnMoveNotecatDown()
{
	try {

		long nCurSel = m_pNxDlNoteCats->CurSel;

		if(m_checkSortAlphabetically.GetCheck() || nCurSel == sriNoRow || nCurSel == m_pNxDlNoteCats->GetRowCount() - 1) {
			UpdateArrowButtons();
			return;
		}

		long nID1 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel, nccID));
		long nSort1 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel, nccSortOrder));
		long nID2 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel+1, nccID));
		long nSort2 = VarLong(m_pNxDlNoteCats->GetValue(nCurSel+1, nccSortOrder));

		//swap the sort orders in data
		ExecuteParamSql("UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}; "
			"UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}; ", nSort2, nID1, nSort1, nID2);

		//swap the sort orders in the datalist
		m_pNxDlNoteCats->PutValue(nCurSel, nccSortOrder, nSort2);
		m_pNxDlNoteCats->PutValue(nCurSel+1, nccSortOrder, nSort1);

		m_pNxDlNoteCats->Sort();

		UpdateArrowButtons();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::UpdateArrowButtons()
{
	try {

		//disable the buttons if sorting alphabetically
		if(m_checkSortAlphabetically.GetCheck()) {		
			m_btnMoveUp.EnableWindow(FALSE);
			m_btnMoveDown.EnableWindow(FALSE);
			m_btnMoveUp.ShowWindow(SW_HIDE);
			m_btnMoveDown.ShowWindow(SW_HIDE);
			return;
		}
		else {
			m_btnMoveUp.ShowWindow(SW_SHOW);
			m_btnMoveDown.ShowWindow(SW_SHOW);
		}

		long nCurSel = m_pNxDlNoteCats->CurSel;
		if(nCurSel == sriNoRow) {
			m_btnMoveUp.EnableWindow(FALSE);
			m_btnMoveDown.EnableWindow(FALSE);
		}
		else if(nCurSel == 0) {
			m_btnMoveUp.EnableWindow(FALSE);
			m_btnMoveDown.EnableWindow(TRUE);
		}
		else if(nCurSel == m_pNxDlNoteCats->GetRowCount() - 1) {
			m_btnMoveUp.EnableWindow(TRUE);
			m_btnMoveDown.EnableWindow(FALSE);
		}
		else {
			m_btnMoveUp.EnableWindow(TRUE);
			m_btnMoveDown.EnableWindow(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::OnDragEndNxdlnotecats(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags)
{
	try {

		if(m_checkSortAlphabetically.GetCheck()) {
			//can't drag if sorting alphabetically
			return;
		}

		if(nFromRow == -1 || nFromRow >= m_pNxDlNoteCats->GetRowCount()) {
			return;
		}

		if(nRow >= m_pNxDlNoteCats->GetRowCount()) {
			return;
		}

		if(nRow == -1) {
			//they dragged to the bottom of the list

			long nID = VarLong(m_pNxDlNoteCats->GetValue(nFromRow, nccID));
			long nSort = VarLong(m_pNxDlNoteCats->GetValue(nFromRow, nccSortOrder));
			
			//move up every following row
			for(int i = nFromRow + 1; i < m_pNxDlNoteCats->GetRowCount(); i++) {
				long nOtherSort = VarLong(m_pNxDlNoteCats->GetValue(i, nccSortOrder));
				nOtherSort--;
				m_pNxDlNoteCats->PutValue(i, nccSortOrder, nOtherSort);
			}

			//save
			ExecuteParamSql("UPDATE NoteCatsF SET SortOrder = SortOrder - 1 WHERE SortOrder > {INT} "
				"UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}", nSort, m_pNxDlNoteCats->GetRowCount(), nID);

			//move this row to the end
			nSort = m_pNxDlNoteCats->GetRowCount();
			m_pNxDlNoteCats->PutValue(nFromRow, nccSortOrder, nSort);

			//we will re-sort at the end of this function
		}
		else {
			
			//may be moving up or down

			long nFromID = VarLong(m_pNxDlNoteCats->GetValue(nFromRow, nccID));
			long nFromSort = VarLong(m_pNxDlNoteCats->GetValue(nFromRow, nccSortOrder));

			long nToID = VarLong(m_pNxDlNoteCats->GetValue(nRow, nccID));
			long nToSort = VarLong(m_pNxDlNoteCats->GetValue(nRow, nccSortOrder));

			//if dragging a row up
			if(nFromSort > nToSort) {

				//more down every row in between, counting the destination row
				for(int i = nFromRow - 1; i >= nRow; i--) {
					long nOtherSort = VarLong(m_pNxDlNoteCats->GetValue(i, nccSortOrder));
					nOtherSort++;
					m_pNxDlNoteCats->PutValue(i, nccSortOrder, nOtherSort);
				}

				//now set the index for the moved row
				m_pNxDlNoteCats->PutValue(nFromRow, nccSortOrder, nToSort);

				//save
				ExecuteParamSql("UPDATE NoteCatsF SET SortOrder = SortOrder + 1 WHERE SortOrder >= {INT} AND SortOrder < {INT}  "
					"UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}", nToSort, nFromSort, nToSort, nFromID);

				//we will re-sort at the end of this function
			}
			//if dragging a row down
			else if(nFromSort < nToSort) {

				int i = 0;
				//move up every row in between, except the destination row
				for(i=nFromRow+1;i<nRow;i++) {
					long nOtherSort = VarLong(m_pNxDlNoteCats->GetValue(i, nccSortOrder));
					nOtherSort--;
					m_pNxDlNoteCats->PutValue(i, nccSortOrder, nOtherSort);
				}

				//now set the index for the moved row
				m_pNxDlNoteCats->PutValue(nFromRow, nccSortOrder, nToSort-1);

				//save
				ExecuteParamSql("UPDATE NoteCatsF SET SortOrder = SortOrder - 1 WHERE SortOrder > {INT} AND SortOrder <= {INT} "
					"UPDATE NoteCatsF SET SortOrder = {INT} WHERE ID = {INT}", nFromSort, nToSort, nToSort, nFromID);

				//we will re-sort at the end of this function
			}
		}

		m_pNxDlNoteCats->Sort();

		UpdateArrowButtons();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
void CNoteCategories::OnDragBeginNxdlnotecats(BOOL* pbShowDrag, long nRow, short nCol, long nFlags)
{
	try {

		if(m_checkSortAlphabetically.GetCheck()) {
			//can't drag if sorting alphabetically
			*pbShowDrag = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}
