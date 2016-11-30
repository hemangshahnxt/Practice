// EMRImageStampSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRImageStampSetupDlg.h"
#include "EmrActionDlg.h"
#include "EmrUtils.h"
#include "MultiSelectDlg.h"
#include "EditComboBox.h"
#include "SharedEmrUtils.h"

// CEMRImageStampSetupDlg dialog

// (j.jones 2010-02-10 15:39) - PLID 37224 - created
// (a.walling 2012-06-08 13:38) - PLID 46648 - Dialogs must set a parent -- use MessageBox instead of AfxMessageBox

using namespace ADODB;
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CEMRImageStampSetupDlg, CNxDialog)

// (j.jones 2012-12-28 14:59) - PLID 54377 - the definition for maximum count of stamp buttons
// is now in SharedEmrUtils.h, so that the NxInkPicture and Practice can use the same define

enum StampListColumns {

	slcID = 0,
	slcStampText,
	slcTypeName,
	slcDescription,
	slcSmartStampTableSpawnRule,	// (j.jones 2010-02-16 11:11) - PLID 37365
	slcActions,		// (z.manning 2010-02-15 10:34) - PLID 37226
	slcColor,
	slcShowDot, // (z.manning 2012-01-26 17:21) - PLID 47592
	slcInactive,	// (j.jones 2010-02-16 12:24) - PLID 37377
	slcHasImage,	// (r.gonet 05/02/2012) - PLID 49949 - A editable checkbox column that is true if the stamp has an image and false otherwise. 
					//  Checking this will open an image select. Unchecking will remove the image from the stamp.
	slcImage,		// (r.gonet 05/02/2012) - PLID 49949 - A small preview of the image contained by the stamp. Blank if there is no image. Contains a handle.
	slcImageBytes,	// (r.gonet 05/02/2012) - PLID 49949 - The actual byte array of the loaded image.
	slcCategoryID,	// (b.spivey, August 14, 2012) - PLID 52130 - CategoryID
};

CEMRImageStampSetupDlg::CEMRImageStampSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRImageStampSetupDlg::IDD, pParent)
{

}

CEMRImageStampSetupDlg::~CEMRImageStampSetupDlg()
{
	// (j.dinatale 2012-08-20 17:54) - PLID 52221 - On second thought, let's clear the destructor, it's not necessary to loop
	//		when this will be taken care of for us
}

void CEMRImageStampSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_STAMP_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADD_STAMP, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_DELETE_STAMP, m_btnDelete);
	DDX_Control(pDX, IDC_CHECK_SHOW_INACTIVE_STAMPS, m_checkShowInactive);
	DDX_Control(pDX, IDC_BTN_EDIT_CATEGORIES, m_btnEditCategories);
}


BEGIN_MESSAGE_MAP(CEMRImageStampSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_STAMP, OnBtnAddStamp)
	ON_BN_CLICKED(IDC_BTN_DELETE_STAMP, OnBtnDeleteStamp)
	ON_BN_CLICKED(IDC_BTN_STAMP_CLOSE, OnBtnStampClose)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INACTIVE_STAMPS, OnCheckShowInactiveStamps)
	ON_BN_CLICKED(IDC_BTN_EDIT_CATEGORIES, OnBtnEditCategories)
END_MESSAGE_MAP()

// CEMRImageStampSetupDlg message handlers

BOOL CEMRImageStampSetupDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		// (b.spivey, August 20, 2012) - PLID 52130 - new button for categories. 
		m_btnEditCategories.AutoSet(NXB_MODIFY); 

		m_StampList = BindNxDataList2Ctrl(IDC_STAMP_LIST, false);

		// (j.jones 2010-02-16 11:11) - PLID 37365 - build the combo for table spawning
		// (j.jones 2010-04-07 12:37) - PLID 38069 - added 'do not add to table' as an option
		CString strTableSpawnCombo;
		strTableSpawnCombo.Format("%li;Add New Row;%li;Increase Quantity;%li;Do Not Add To Table;",
			esstsrAddNewRow, esstsrIncreaseQuantity, esstsrDoNotAddToTable);
		IColumnSettingsPtr pcolTableSpawnRule = m_StampList->GetColumn(slcSmartStampTableSpawnRule);
		pcolTableSpawnRule->PutComboSource((LPCTSTR)strTableSpawnCombo);

		IColumnSettingsPtr pcolActions = m_StampList->GetColumn(slcActions);
		// (z.manning 2010-02-15 10:57) - PLID 37226 - Added a column for actions
		pcolActions->PutFieldName(_bstr_t(FormatString(
			"'<' + CONVERT(nvarchar(20), (SELECT COUNT(*) FROM EmrActionsT WHERE Deleted = 0 AND SourceType = %d AND SourceID = EMRImageStampsT.ID)) + ' Action(s)>'"
			, eaoSmartStamp)));
		
		// (j.jones 2010-02-16 12:24) - PLID 37377 - default to showing only active stamps		
		m_StampList->PutWhereClause("Inactive = 0");
		m_StampList->Requery();

		// (j.jones 2010-09-27 09:40) - PLID 39403 - enable/disable controls based on permissions
		if(!(GetCurrentUserPermissions(bioEMRImageStamps) & (sptCreate|sptCreateWithPass))) {
			GetDlgItem(IDC_BTN_ADD_STAMP)->EnableWindow(FALSE);
		}
		if(!(GetCurrentUserPermissions(bioEMRImageStamps) & (sptDelete|sptDeleteWithPass))) {
			GetDlgItem(IDC_BTN_DELETE_STAMP)->EnableWindow(FALSE);
		}
		if(!(GetCurrentUserPermissions(bioEMRImageStamps) & (sptWrite|sptWriteWithPass))) {
			m_StampList->PutReadOnly(g_cvarTrue);
		}

	}NxCatchAll("Error in CEMRImageStampSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRImageStampSetupDlg::HandleStampChange()
{
	m_bChanged = TRUE;

	//clear our existing stamps and send a tablechecker,
	//the next time they are needed they will be reloaded
	GetMainFrame()->ClearCachedEMRImageStamps();
	GetMainFrame()->m_bEMRImageStampsLoaded = FALSE;

	CClient::RefreshTable(NetUtils::EMRImageStampsT);
}

void CEMRImageStampSetupDlg::OnBtnAddStamp()
{
	try {

		// (j.jones 2010-09-27 09:40) - PLID 39403 - check create permission
		if(!CheckCurrentUserPermissions(bioEMRImageStamps, sptCreate)) {
			return;
		}

		GetMainFrame()->LoadEMRImageStamps();
		
		// (j.jones 2012-12-28 15:04) - PLID 54377 - We no longer have an upper limit to the amount
		// of stamps that can be entered in the system. The MAX_NUM_STAMP_BUTTONS define does however
		// still restrict how many stamps can be associated with a given image.
		/*
		if(GetMainFrame()->m_aryEMRImageStamps.GetSize() >= MAX_NUM_STAMP_BUTTONS) {
			// They have already reached the maximum number of stamps
			MessageBox("You have already added the maximum number of stamps and cannot add any more.");
		}
		*/

		CString strStampName = "";
		if(InputBox(this, "Enter a new stamp name:", strStampName, "") == IDOK) {

			strStampName.TrimLeft();
			strStampName.TrimRight();

			if(strStampName.IsEmpty()) {
				MessageBox("The stamp text cannot be blank.");
				return;
			}

			//validate the length
			if(strStampName.GetLength() > 50) {
				MessageBox("The stamp text cannot be longer than 50 characters.");
				return;
			}

			//make sure the stamp text does not already exist
			//(don't bother checking if they just deleted a stamp, they should know better)
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM EMRImageStampsT "
				"WHERE StampText = {STRING}", strStampName);
			if(!rs->eof) {
				MessageBox("The stamp text you entered is already in use by another stamp.");
				return;
			}

			long nColor = 0;
			// (j.jones 2010-02-16 11:11) - PLID 37365 - added spawn rule, defaults to 1, Add New Row
			_RecordsetPtr prsInsert = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO EMRImageStampsT (StampText, TypeName, Description, Color, SmartStampTableSpawnRule, Inactive) VALUES ({STRING}, {STRING}, '', {INT}, {INT}, 0) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, strStampName, strStampName, nColor, esstsrAddNewRow);
			HandleStampChange();

			//add the row
			const long nNewStampID = AdoFldLong(prsInsert->GetFields(), "NewID");
			IRowSettingsPtr pRow = m_StampList->GetNewRow();
			pRow->PutValue(slcID, nNewStampID);
			pRow->PutValue(slcStampText, (LPCTSTR)strStampName);
			pRow->PutValue(slcTypeName, (LPCTSTR)strStampName);
			pRow->PutValue(slcDescription, (LPCTSTR)"");
			pRow->PutValue(slcColor, nColor);
			pRow->PutCellBackColor(slcColor, nColor);
			// (j.jones 2010-02-16 11:11) - PLID 37365 - added spawn rule, defaults to 1, Add New Row
			pRow->PutValue(slcSmartStampTableSpawnRule, (long)esstsrAddNewRow);
			pRow->PutValue(slcActions, "<0 Action(s)>");
			// (z.manning 2012-01-26 17:22) - PLID 47592 - Show dot option, default to on
			pRow->PutValue(slcShowDot, g_cvarTrue);
			// (j.jones 2010-02-16 12:24) - PLID 37377 - added inactive ability
			pRow->PutValue(slcInactive, g_cvarFalse);
			// (r.gonet 05/02/2012) - PLID 49949 - Added default - no image - for the stamp
			pRow->PutValue(slcHasImage, g_cvarFalse);
			pRow->PutValue(slcImage, GetVariantNull());
			pRow = m_StampList->AddRowSorted(pRow, NULL);
			m_StampList->PutCurSel(pRow);
			m_StampList->EnsureRowInView(pRow);

			// (z.manning 2011-10-25 14:45) - PLID 39401 - Now let's prompt for what EMR images this stamp should be included with.
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlgSelectImages(this, "EmrInfoT");
			dlgSelectImages.m_bPreSelectAll = TRUE;
			const CString strFrom = "EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID";
			const CString strWhere = FormatString("EmrInfoMasterT.Inactive = 0 AND EmrInfoT.DataType = %d AND EmrInfoT.ID > 0", eitImage);
			const CString strDescription = "Select the EMR images that should include this stamp";
			if(IDOK == dlgSelectImages.Open(strFrom, strWhere, "EmrInfoMasterT.ID", "EmrInfoT.Name", strDescription))
			{
				CArray<long,long> arynEmrInfoMasterIDsToExclude;
				dlgSelectImages.FillArrayWithUnselectedIDs(&arynEmrInfoMasterIDsToExclude);
				if(arynEmrInfoMasterIDsToExclude.GetCount() > 0)
				{
					// (z.manning 2011-10-25 14:46) - PLID 39401 - If they unchecked any images then store those exclusions in data.
					CParamSqlBatch sqlBatch;
					for(int nInfoMasterIndex = 0; nInfoMasterIndex < arynEmrInfoMasterIDsToExclude.GetCount(); nInfoMasterIndex++) {
						const long nEmrInfoMasterID = arynEmrInfoMasterIDsToExclude.GetAt(nInfoMasterIndex);
						sqlBatch.Add("INSERT EmrInfoStampExclusionsT (EmrInfoMasterID, StampID) VALUES ({INT}, {INT})", nEmrInfoMasterID, nNewStampID);
					}
					sqlBatch.Execute(GetRemoteData());
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMRImageStampSetupDlg::OnBtnDeleteStamp()
{
	try {

		// (j.jones 2010-09-27 09:40) - PLID 39403 - check delete permission
		if(!CheckCurrentUserPermissions(bioEMRImageStamps, sptDelete)) {
			return;
		}

		IRowSettingsPtr pRow = m_StampList->CurSel;
		if(pRow == NULL) {
			MessageBox("You must first select a stamp before deleting.");
			return;
		}

		long nID = VarLong(pRow->GetValue(slcID));

		if(nID != -1) {
			// (z.manning 2010-02-25 11:48) - PLID 37404 - Can't delete if this is tied to data
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			// (z.manning 2011-09-28 12:15) - PLID 45729 - Also check for dropdown stamp filters here.
			// (j.jones 2012-11-27 10:38) - PLID 53144 - default values have been moved to EMRTableDropdownStampDefaultsT
			// (j.jones 2013-07-22 16:15) - PLID 57277 - rewrote this query to more easily handle checking lots of tables at once,
			// and added the ability to disallow deleting if the actions have been used (even if the actions are deleted)
			_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON; "
				"DECLARE @ImageStampID INT, @ImageStampSourceType INT; "
				"SET @ImageStampID = {INT}; "
				"SET @ImageStampSourceType = {CONST_INT}; "
				""
				"SET NOCOUNT OFF; "
				""
				"SELECT TOP 1 StampID FROM ("
				"	SELECT ID AS StampID FROM EmrDetailImageStampsT WHERE EmrImageStampID = @ImageStampID "
				"	UNION SELECT StampID FROM EmrTableDropdownStampFilterT WHERE StampID = @ImageStampID "
				"	UNION SELECT StampID FROM EMRTableDropdownStampDefaultsT WHERE StampID = @ImageStampID "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRDetailsT ON EMRActionsT.ID = EMRDetailsT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRChargesT ON EMRActionsT.ID = EMRChargesT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRDiagCodesT ON EMRActionsT.ID = EMRDiagCodesT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EmrProcedureT ON EMRActionsT.ID = EmrProcedureT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EmrMedicationsT ON EMRActionsT.ID = EmrMedicationsT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRMasterT ON EMRActionsT.ID = EMRMasterT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRTemplateDetailsT ON EMRActionsT.ID = EMRTemplateDetailsT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRTemplateTopicsT ON EMRActionsT.ID = EMRTemplateTopicsT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRTodosT ON EMRActionsT.ID = EMRTodosT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN EMRTopicsT ON EMRActionsT.ID = EMRTopicsT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				"	UNION SELECT SourceID AS StampID FROM EMRActionsT "
				"		INNER JOIN LabsT ON EMRActionsT.ID = LabsT.SourceActionID "
				"		WHERE EMRActionsT.SourceID = @ImageStampID AND EMRActionsT.SourceType = @ImageStampSourceType "
				") AS StampsInUseQ", nID, eaoSmartStamp);
			if(!rs->eof) {
				MessageBox("You may not delete this stamp because it is in use in EMR. You may still inactivate it though.", NULL, MB_ICONWARNING|MB_OK);
				return;
			}
			rs->Close();

			if(IDNO == MessageBox("Are you sure you wish to delete this stamp?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
			// (z.manning 2010-03-02 15:56) - PLID 37571 - Also delete any actions
			// (j.jones 2013-07-22 16:15) - PLID 57277 - we now really delete actions, instead of marking them deleted
			CString strSqlBatch;
			CNxParamSqlArray aryParams;
			_RecordsetPtr rsActions = CreateParamRecordset("SELECT ID FROM EMRActionsT WHERE SourceID = {INT} AND SourceType = {CONST_INT}", nID, eaoSmartStamp);
			while(!rsActions->eof) {
				//DeleteEMRAction returns a SQL fragment
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "{SQL}", DeleteEMRAction(AdoFldLong(rsActions, "ID")));
				rsActions->MoveNext();
			}
			rsActions->Close();

			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nID INT");
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @nID = {INT}", nID);
			// (z.manning 2011-10-24 12:45) - PLID 46082 - Delete from EmrInfoStampExclusionsT
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EmrInfoStampExclusionsT WHERE StampID = @nID");
			// (z.manning 2010-02-15 12:44) - PLID 37226 - We now delete immediately
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EMRImageStampsT WHERE ID = @nID");
			
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			HandleStampChange();
		}

		//remove the row
		m_StampList->RemoveRow(pRow);

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 20, 2012) - PLID 52130 - editing stamp categories. 
void CEMRImageStampSetupDlg::OnBtnEditCategories()
{
	try {
		CEditComboBox dlg(this, 79, "Edit Smart Stamp Categories"); 
		if (dlg.DoModal() == IDOK) {
			long nStampID = -1; 
			//If not null we need to reset the selection. 
			if (m_StampList->GetCurSel()){
				nStampID = VarLong(m_StampList->GetCurSel()->GetValue(slcID), -1);
			}

			// (a.walling 2013-03-20 13:57) - PLID 55787 - Clear out old icons
			m_StampList->Clear();
			m_imageIcons.clear();
			m_StampList->Requery();
			m_StampList->FindByColumn(slcID, nStampID, m_StampList->GetFirstRow(), TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEMRImageStampSetupDlg, CNxDialog)
	ON_EVENT(CEMRImageStampSetupDlg, IDC_STAMP_LIST, 6, OnRButtonDownStampList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRImageStampSetupDlg, IDC_STAMP_LIST, 9, OnEditingFinishingStampList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMRImageStampSetupDlg, IDC_STAMP_LIST, 10, OnEditingFinishedStampList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMRImageStampSetupDlg, IDC_STAMP_LIST, 19, CEMRImageStampSetupDlg::LeftClickStampList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRImageStampSetupDlg, IDC_STAMP_LIST, 18, CEMRImageStampSetupDlg::RequeryFinishedStampList, VTS_I2)
	ON_EVENT(CEMRImageStampSetupDlg, IDC_STAMP_LIST, 8, CEMRImageStampSetupDlg::OnEditingStartingStampList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

void CEMRImageStampSetupDlg::OnRButtonDownStampList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_StampList->CurSel = pRow;

		enum {
			eChangeColor = 1,
			eDeleteStamp = 2,
			eEditActions = 3,
		};

		// (j.jones 2010-09-27 09:40) - PLID 39403 - show/hide menu items based on permissions
		BOOL bDisabledEdit = FALSE;
		BOOL bDisabledDelete = FALSE;

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();

		if(!(GetCurrentUserPermissions(bioEMRImageStamps) & (sptWrite|sptWriteWithPass))) {
			bDisabledEdit = TRUE;
		}
		else {
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eChangeColor, "&Change Stamp Color");
			// (z.manning 2010-02-15 13:32) - PLID 37226 - Right click option to edit actions
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eEditActions, "&Edit Actions");
		}

		if(!(GetCurrentUserPermissions(bioEMRImageStamps) & (sptDelete|sptDeleteWithPass))) {
			bDisabledDelete = TRUE;
		}
		else {
			if(!bDisabledEdit) {
				mnu.AppendMenu(MF_SEPARATOR);
			}
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eDeleteStamp, "&Delete This Stamp");
		}

		if(bDisabledEdit && bDisabledDelete) {
			//we removed all options, so there is no menu
			return;
		}

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eChangeColor) {
			PromptForStampColor(pRow);
		}
		else if(nRet == eDeleteStamp) {

			OnBtnDeleteStamp();
		}
		else if(nRet == eEditActions) {
			LeftClickStampList(lpRow, slcActions, x, y, 0);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMRImageStampSetupDlg::OnEditingFinishingStampList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == slcStampText) {

			CString strStampName = CString(strUserEntered);

			strStampName.TrimLeft();
			strStampName.TrimRight();

			if(strStampName.IsEmpty()) {
				MessageBox("The stamp text cannot be blank.");
				*pbCommit = FALSE;
				return;
			}

			if(strStampName.GetLength() > 50) {
				MessageBox("The stamp text cannot be longer than 50 characters.");
				*pbCommit = FALSE;
				return;
			}
			else {
				//make sure the stamp text does not already exist
				//(don't bother checking if they just deleted a stamp, they should know better)
				long nID = VarLong(pRow->GetValue(slcID));
				_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM EMRImageStampsT "
					"WHERE ID <> {INT} AND StampText = {STRING}", nID, strStampName);
				if(!rs->eof) {
					MessageBox("The stamp text you entered is already in use by another stamp.");
					*pbCommit = FALSE;
					return;
				}
				rs->Close();
			}

			_variant_t varNew(strStampName);
			*pvarNewValue = varNew.Detach();
		}
		else if(nCol == slcTypeName) {

			CString strTypeName = CString(strUserEntered);

			strTypeName.TrimLeft();
			strTypeName.TrimRight();

			if(strTypeName.IsEmpty()) {
				MessageBox("The type name cannot be blank.");
				*pbCommit = FALSE;
				return;
			}

			if(strTypeName.GetLength() > 255) {
				MessageBox("The type name cannot be longer than 255 characters.");
				*pbCommit = FALSE;
				return;
			}

			_variant_t varNew(strTypeName);
			*pvarNewValue = varNew.Detach();
		}
		else if(nCol == slcDescription) {

			CString strDescription = CString(strUserEntered);

			strDescription.TrimLeft();
			strDescription.TrimRight();

			//a blank description is allowed
			/*
			if(strDescription.IsEmpty()) {
				MessageBox("The description cannot be blank.");
				*pbCommit = FALSE;
				return;
			}
			*/

			if(strDescription.GetLength() > 2000) {
				MessageBox("The description cannot be longer than 2000 characters.");
				*pbCommit = FALSE;
				return;
			}

			_variant_t varNew(strDescription);
			*pvarNewValue = varNew.Detach();
		}
		else if(nCol == slcSmartStampTableSpawnRule) {
			// (j.jones 2010-02-16 11:11) - PLID 37365 - ensure the spawn rule is valid
			if(pvarNewValue->vt != VT_I4 || pvarNewValue->lVal < (long)esstsrAddNewRow) {
				_variant_t varNew((long)esstsrAddNewRow);
				*pvarNewValue = varNew.Detach();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMRImageStampSetupDlg::OnEditingFinishedStampList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		// (z.manning 2010-02-15 12:51) - PLID 37226 - We now save this data immediately
		long nID = VarLong(pRow->GetValue(slcID));
		switch(nCol)
		{
			case slcStampText:
				ExecuteParamSql("UPDATE EMRImageStampsT SET StampText = {STRING} WHERE ID = {INT}", VarString(varNewValue,""), nID);
				break;

			case slcTypeName:
				ExecuteParamSql("UPDATE EMRImageStampsT SET TypeName = {STRING} WHERE ID = {INT}", VarString(varNewValue,""), nID);
				break;

			case slcDescription:
				ExecuteParamSql("UPDATE EMRImageStampsT SET Description = {STRING} WHERE ID = {INT}", VarString(varNewValue,""), nID);
				break;

			case slcSmartStampTableSpawnRule: {
				// (j.jones 2010-02-16 11:11) - PLID 37365 - save the spawn rule
				long nSpawnRule = VarLong(varNewValue, (long)esstsrAddNewRow);
				ExecuteParamSql("UPDATE EMRImageStampsT SET SmartStampTableSpawnRule = {INT} WHERE ID = {INT}", nSpawnRule, nID);
				break;
			}

			case slcShowDot: { // (z.manning 2012-01-26 17:23) - PLID 47592
				BOOL bShowDot = VarBool(varNewValue, TRUE);
				ExecuteParamSql("UPDATE EMRImageStampsT SET ShowDot = {BIT} WHERE ID = {INT}", bShowDot, nID);
				break;
			}

			case slcInactive:
			{
				// (j.jones 2010-02-16 12:24) - PLID 37377 - save the Inactive status
				BOOL bInactive = VarBool(varNewValue, FALSE);
				ExecuteParamSql("UPDATE EMRImageStampsT SET Inactive = {INT} WHERE ID = {INT}", bInactive ? 1 : 0, nID);

				if(bInactive && !m_checkShowInactive.GetCheck()) {
					//remove the row
					m_StampList->RemoveRow(pRow);
				}
				break;
			}

			// (b.spivey, August 14, 2012) - PLID 52130 - for updating category ID
			case slcCategoryID:
			{
				long nCategoryID = VarLong(pRow->GetValue(slcCategoryID), -1);
				//This is adding. -- Temporary fix. 
				// (b.spivey, August 20, 2012) - PLID 52130 - We don't add in this datalist anymore. 
				if (nCategoryID == -1) { //they selected none. 
					ExecuteParamSql("UPDATE EMRImageStampsT SET CategoryID = NULL WHERE ID = {INT}", nID); 
					pRow->PutValue(slcCategoryID, VT_NULL); 
				}
				else { //this has a value. 
					long nCatID = VarLong(pRow->GetValue(slcCategoryID)); 
					ExecuteParamSql("UPDATE EMRImageStampsT SET CategoryID = {INT} WHERE ID = {INT}", nCatID, nID); 
				}
				break;
			}
		}

		// (j.jones 2012-12-14 09:09) - PLID 54204 - All changes to stamps need to call HandleStampChange().
		// Before, we called it in each case statement above, which led to the possibility of adding new
		// cases without remembering to call this function.
		// Just call it at all times. In the event that we didn't really change stamp content, the worst
		// case is we refreshed the master stamp list unnecessarily.
		HandleStampChange();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-02-15 10:38) - PLID 37226
void CEMRImageStampSetupDlg::LeftClickStampList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case slcActions:
			{
				// (j.jones 2010-09-27 09:40) - PLID 39403 - check write permission
				if(!CheckCurrentUserPermissions(bioEMRImageStamps, sptWrite)) {
					return;
				}

				CEmrActionDlg dlg(this);
				dlg.m_SourceType = eaoSmartStamp;
				dlg.m_nSourceID = VarLong(pRow->GetValue(slcID));
				if(dlg.DoModal() == IDOK) {

					// (j.jones 2012-07-05 09:55) - PLID 48616 - always refresh the stamp info.
					// even if m_arActions is empty, because we could have just deleted all actions
					HandleStampChange();

					long nActionCount = 0;
					for(int nActionIndex = 0; nActionIndex < dlg.m_arActions.GetSize(); nActionIndex++) {
						EmrAction ea = dlg.m_arActions.GetAt(nActionIndex);
						if(!ea.bDeleted) {
							nActionCount++;
						}
					}
					pRow->PutValue(slcActions, _bstr_t("<" + AsString(nActionCount) + " Action(s)>"));
				}
			}
			break;

			case slcColor:
				// (z.manning 2010-09-08 10:04) - PLID 39490 - Prompt for a color
				PromptForStampColor(pRow);
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/02/2012) - PLID 49949 - Removes the associated image from the stamp, given a row representing that stamp.
void CEMRImageStampSetupDlg::UnselectImage(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(!pRow) {
		return;
	}

	// Free up the memory used by the safearrays stored in the datalist
	// (r.gonet 05/02/2012) - PLID 49949 - slcImageBytes is a safearray of VT_UI1s
	_variant_t varBinary = pRow->GetValue(slcImageBytes);
	if (varBinary.vt == (VT_ARRAY | VT_UI1)) {
		pRow->PutValue(slcImageBytes, _variant_t((long)0, VT_I4));
	}

	long nID = VarLong(pRow->GetValue(slcID));
	pRow->PutValue(slcHasImage, g_cvarFalse);
	// (r.gonet 05/02/2012) - PLID 49949 - Put a NULL bitmap handle.
	pRow->PutValue(slcImage, _variant_t((long)0, VT_I4));
	pRow->PutValue(slcImageBytes, g_cvarNull);
	// (r.gonet 05/02/2012) - PLID 49949 - NULL out the image in the database for this stamp.
	ExecuteParamSql("UPDATE EMRImageStampsT SET [Image] = NULL WHERE ID = {INT}", nID);
	// (r.gonet 05/02/2012) - PLID 49949 - Other places in Practice must know about our change.
	HandleStampChange();

	// (r.gonet 05/02/2012) - PLID 49949 - Requery in order to show the stamp image is gone
	// (a.walling 2013-03-20 13:57) - PLID 55787 - Clear out old icons
	m_StampList->Clear();
	m_imageIcons.clear();
	m_StampList->Requery();
	m_StampList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	// (r.gonet 05/02/2012) - PLID 49949 - Set back the row we were on
	pRow = m_StampList->SetSelByColumn(slcID, _variant_t(nID, VT_I4));
	if(pRow) {
		m_StampList->EnsureRowInView(pRow);
	} else {
		// (r.gonet 05/02/2012) - PLID 49949 - The stamp was deleted while we were editing it?
	}
}

// (r.gonet 05/02/2012) - PLID 49949 - Loads an image given a file path. Handles bmps, jpegs, gifs, tifs, and pngs.
//  Returns true if loading was successful and false otherwise
bool CEMRImageStampSetupDlg::LoadImg(IN CString strFilePath, OUT CxImage &xImage)
{
	CString strFileExt = GetFileExtension(strFilePath).MakeLower();
	if(strFileExt == "bmp") {
		return xImage.Load(strFilePath, CXIMAGE_FORMAT_BMP);
	} else if(strFileExt == "jpg" || strFileExt == "jpeg") {
		return xImage.Load(strFilePath, CXIMAGE_FORMAT_JPG);
	} else if(strFileExt == "gif") {
		return xImage.Load(strFilePath, CXIMAGE_FORMAT_GIF);
	} else if(strFileExt == "tif" || strFileExt == "tiff") {
		return xImage.Load(strFilePath, CXIMAGE_FORMAT_TIF);
	} else if(strFileExt == "png") {
		return xImage.Load(strFilePath, CXIMAGE_FORMAT_PNG);
	} else {
		return false;
	}
}

// (r.gonet 05/02/2012) - PLID 49949 - Selects an image from a file and assigns it to the stamp at pRow
//  Saves to the data.
bool CEMRImageStampSetupDlg::SelectImage(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(!pRow) {
		// (r.gonet 05/02/2012) - PLID 49949 - The caller has made a mistake, I assume.
		return false;
	}

	// (r.gonet 05/02/2012) - PLID 49949 - Get from the user a file of type that we support as the image stamp image.
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_ENABLESIZING, 
		"All Picture Files|*.bmp;*.jpg;*.jpeg;*.gif;*.tif;*.tiff;*.png|"
		"Bitmap Files (*.bmp)|*.bmp|" 
		"JPEG (*.jpg;*.jpeg)|*.jpg;*.jpeg|"
		"GIF (*.gif)|*.gif|"
		"TIFF (*.tif;*.tiff)|*.tif;*.tiff|"
		"PNG (*.png)|*.png|", this);
	// (r.gonet 05/02/2012) - PLID 49949 - Start off browsing in a shared folder.
	CString strImagePath = GetSharedPath() ^ "Images";
	dlgFile.m_ofn.lpstrInitialDir = strImagePath;
	if(dlgFile.DoModal() == IDOK)
	{
		// (r.gonet 05/02/2012) - PLID 49949 - They chose an image, so set it up as the stamp's image.
		CString strImageFilePath = dlgFile.GetPathName();

		// (r.gonet 06/07/2012) - PLID 50725 - Cap the file size at 50 KB. We do store everything as PNGs though, so this may be limiting to other uncompressed file types,
		//  however it would be strange to say "Please choose an image that when converted to a PNG, is less than 50 KB" or the useless "File size too large"
		CFile f;
		if (!f.Open(strImageFilePath, CFile::modeRead | CFile::shareDenyNone)) {
			MessageBox(FormatString("Could not open the file '%s'.\r\n\r\nPlease make sure this file exists and that you have access to it.", strImageFilePath), "Could not open file", MB_ICONERROR|MB_OK);
			return false;
		}
		DWORD dwSize = ::GetFileSize(f.m_hFile, NULL);
		if(dwSize > 50 * 1024) {
			MessageBox("Image file size is too large. The Image file must be 50 KB or less in size.", "File size too large", MB_ICONERROR|MB_OK);
			return false;
		}
		f.Close();

		CxImage xImage;
		if(LoadImg(strImageFilePath, xImage))
		{
			// (r.gonet 05/02/2012) - PLID 49949 - We successfully loaded the image file the user gave us
			// (a.walling 2013-05-08 16:56) - PLID 56610 - CxImage standardizing with cstdint
			int nNewImageNumBytes = 0;
			BYTE *arNewImageBytes = NULL;
			// (r.gonet 05/02/2012) - PLID 49949 - Take whatever they gave us and encode it as a PNG, since that is lossless
			bool bEncodeResult = xImage.Encode(arNewImageBytes, nNewImageNumBytes, CXIMAGE_FORMAT_PNG);
			
			// (r.gonet 05/02/2012) - PLID 49949 - Our goal below is to take the image and make it a variant
			//  so that we can pass it as a parameter to the SQL update statement.
			VARIANT vOut;
			VariantInit(&vOut);

			SAFEARRAY *pSA;
			SAFEARRAYBOUND aDim[1];
			aDim[0].lLbound = 0;
			aDim[0].cElements = nNewImageNumBytes;
			BYTE *pActualData = NULL;
			HRESULT hres = S_OK;

			// Create the safearray
			pSA = SafeArrayCreate(VT_UI1, 1, aDim);

			VariantInit(&vOut);
			vOut.vt = (VT_ARRAY|VT_UI1);
			vOut.parray = pSA;
			SafeArrayAccessData(pSA, (void**)&pActualData);
			memcpy(pActualData, arNewImageBytes, nNewImageNumBytes);
			SafeArrayUnaccessData(pSA);

			pRow->PutValue(slcHasImage, g_cvarTrue);
			UpdateImage(pRow);
			pRow->PutValue(slcImageBytes, vOut);
			long nID = VarLong(pRow->GetValue(slcID));
			// (r.gonet 05/02/2012) - PLID 49949 - Save the image to the data and let users of the stamp know it has changed.
			ExecuteParamSql("UPDATE EMRImageStampsT SET [Image] = {VARBINARY} WHERE ID = {INT}", vOut, nID);
			HandleStampChange();
			
			// (r.gonet 05/02/2012) - PLID 49949 - Clean up memory or we leak.
			xImage.FreeMemory(arNewImageBytes);
			VariantClear(&vOut);

			// (r.gonet 05/02/2012) - PLID 49949 - Requery in order to show the stamp image
			// (a.walling 2013-03-20 13:57) - PLID 55787 - Clear out old icons
			m_StampList->Clear();
			m_imageIcons.clear();
			m_StampList->Requery();
			m_StampList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			// (r.gonet 05/02/2012) - PLID 49949 - Select the row we were editing.
			pRow = m_StampList->SetSelByColumn(slcID, _variant_t(nID, VT_I4));
			if(pRow) {
				m_StampList->EnsureRowInView(pRow);
			} else {
				// (r.gonet 05/02/2012) - PLID 49949 - Stamp was deleted while we were editing?
			}
			return true;
		} else {
			MsgBox("Practice could not load the specified image.");
			return false;
		}
	} else {
		// (r.gonet 05/02/2012) - PLID 49949 -  Nothing was selected, don't commit the change.
		return false;
	}
}

void CEMRImageStampSetupDlg::OnBtnStampClose()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-16 12:22) - PLID 37377 - added ability to show inactive stamps
void CEMRImageStampSetupDlg::OnCheckShowInactiveStamps()
{
	try {

		if(m_checkShowInactive.GetCheck()) {
			m_StampList->PutWhereClause("");
		}
		else {
			m_StampList->PutWhereClause("Inactive = 0");
		}

		// (a.walling 2013-03-20 13:57) - PLID 55787 - Clear out old icons
		m_StampList->Clear();
		m_imageIcons.clear();
		m_StampList->Requery();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/02/2012) - PLID 49949 - Update the preview in the slcImage column from the bytes stored in the ImageBytes column
void CEMRImageStampSetupDlg::UpdateImage(IRowSettingsPtr pRow)
{
	// (r.gonet 05/02/2012) - PLID 49949 - slcImageBytes is a safearray of VT_UI1s
	_variant_t varBinary = pRow->GetValue(slcImageBytes);
	BYTE *tmpBytes = NULL;
	long nImageNumBytes = 0;
	BYTE *arImageBytes = NULL;

	if (varBinary.vt == (VT_ARRAY | VT_UI1)) {
		// (r.gonet 05/02/2012) - PLID 49949 - Get the data as a byte array
		SafeArrayAccessData(varBinary.parray,(void **) &arImageBytes);
		nImageNumBytes = varBinary.parray->rgsabound[0].cElements;
		// (r.gonet 05/02/2012) - PLID 49949 - Make a copy of the data so we can manipulate it.
		tmpBytes = new BYTE[nImageNumBytes];
		memcpy(tmpBytes, arImageBytes, nImageNumBytes);
		CxImage xImage;
		// (r.gonet 05/02/2012) - PLID 49949 - Load the bytes into a CxImage object. Remember this is PNG data.
		xImage.Decode(tmpBytes, nImageNumBytes, CXIMAGE_FORMAT_PNG);
		
		long nIcoNumBytes = 0;
		BYTE *arIcoBytes = NULL;
		// (r.gonet 05/02/2012) - PLID 49949 - We want a smallish size so it can fit in the row.
		xImage.QIShrink(32, 32);
		HBITMAP hBitmap = xImage.MakeBitmap();

		BITMAP b;
		GetObject(hBitmap, sizeof(BITMAP), &b);

		int width = xImage.GetWidth();
		int height = xImage.GetHeight();

		// (r.gonet 05/02/2012) - PLID 49949 - We need to create an icon in order to place it
		//  in the row. Icons have two bitmaps associated with them in order to draw transparently.
		//  The first bitmap is just the image itself. The second is a mask that will select all transparent parts after a bitwise AND.
		//  We must create both manually. We don't care about transparency here, so a mask of all black pixels will be sufficient.
		ICONINFO iconInfo;
		HICON hIcon;

		// Create the mask
		CDC *pDC = GetDC();
		CDC memDCSrc, memDCAndMask, memDCBlackness;
		memDCSrc.CreateCompatibleDC(pDC);
		CBitmap bmp;
		bmp.Attach(hBitmap);
		CBitmap bmpAndMask, bmpXorMask;
		memDCSrc.SelectObject(bmp);
		memDCBlackness.CreateCompatibleDC(pDC);
		memDCBlackness.SelectObject(bmp);
		memDCAndMask.SelectObject(bmp);
		// (r.gonet 05/02/2012) - PLID 49949 - Make the memDC completely black.
		memDCBlackness.BitBlt(0, 0, width, height, &memDCSrc, 0, 0, BLACKNESS);
		CBitmap bmpMask;
		bmpMask.CreateCompatibleBitmap(&memDCBlackness, width, height);
		iconInfo.hbmColor = bmp; // Just the image
		iconInfo.hbmMask = bmpMask; // A black mask so everything will be retained.
		hIcon = CreateIconIndirect(&iconInfo);
		// (r.gonet 05/02/2012) - PLID 49949 - Finally, put the icon in the row
		pRow->PutValue(slcImage, _variant_t((long)hIcon, VT_I4));
		// (r.gonet 05/02/2012) - PLID 49949 - Cleanup or we leak.
		delete [] tmpBytes;
		xImage.Destroy();
		SafeArrayUnaccessData(varBinary.parray);

		// (a.walling 2013-03-20 13:57) - PLID 55787 - Keep track of icons to delete
		m_imageIcons.push_back(shared_ptr<HICON__>(hIcon, &::DestroyIcon));
	} else {
		// (r.gonet 05/02/2012) - PLID 49949 - There is no image associated with this stamp
	}
}

// (z.manning 2010-09-08 09:41) - PLID 39490
void CEMRImageStampSetupDlg::RequeryFinishedStampList(short nFlags)
{
	try
	{
		for(IRowSettingsPtr pRow = m_StampList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			pRow->PutCellBackColor(slcColor, pRow->GetValue(slcColor));
			// (r.gonet 05/02/2012) - PLID 49949 - Also generate the preview of the image
			UpdateImage(pRow);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-09-08 10:00) - PLID 39490 - Move this logic to its own function
void CEMRImageStampSetupDlg::PromptForStampColor(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	// (j.jones 2010-09-27 09:40) - PLID 39403 - check write permission
	if(!CheckCurrentUserPermissions(bioEMRImageStamps, sptWrite)) {
		return;
	}

	long nCurColor = VarLong(pRow->GetValue(slcColor), 0);
	// (a.walling 2012-06-08 13:37) - PLID 46648 - Dialogs must set a parent!
	CColorDialog dlg(nCurColor, CC_ANYCOLOR|CC_RGBINIT, this);
	if(dlg.DoModal() == IDOK) {
		pRow->PutValue(slcColor, (long)dlg.m_cc.rgbResult);
		pRow->PutForeColor((long)dlg.m_cc.rgbResult);
		pRow->PutCellBackColor(slcColor, (long)dlg.m_cc.rgbResult);

		// (z.manning 2010-02-15 13:08) - PLID 37226 - We now update this right away
		long nID = VarLong(pRow->GetValue(slcID));
		ExecuteParamSql("UPDATE EMRImageStampsT SET Color = {INT} WHERE ID = {INT}", dlg.m_cc.rgbResult, nID);
		HandleStampChange();
	}
}

// (j.jones 2010-09-27 09:49) - PLID 39403 - added to check permissions
void CEMRImageStampSetupDlg::OnEditingStartingStampList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			// (j.jones 2010-09-27 09:52) - PLID 39403 - if it is any of our editable columns,
			// check permissions first

			case slcStampText:
			case slcTypeName:
			case slcDescription:
			case slcSmartStampTableSpawnRule:
			case slcInactive:
			case slcShowDot: // (z.manning 2012-01-26 17:24) - PLID 47592

				if(!CheckCurrentUserPermissions(bioEMRImageStamps, sptWrite)) {
					*pbContinue = FALSE;
					return;
				}
				break;
			case slcHasImage:
				// (r.gonet 05/02/2012) - PLID 49949 - This checkbox column controls whether we have an image or not.
				if(pvarValue == NULL || VarBool(*pvarValue, FALSE) == FALSE) {
					// (r.gonet 05/02/2012) - PLID 49949 - The checkbox column is not filled somehow or it is unchecked, meaning we don't have an image.
					if(!SelectImage(pRow)) {
						// (r.gonet 05/02/2012) - PLID 49949 - The user cancelled out of selecting an image or something went wrong during its loading. Cancel.
						*pbContinue = FALSE;
						return;
					} else {
						// (r.gonet 05/02/2012) - PLID 49949 - The user selected an image, so proceed with the edit.
					}
				} else {
					// (r.gonet 05/02/2012) - PLID 49949 - Remove the image from the stamp.
					UnselectImage(pRow);
				}
				break;
		}
		
	}NxCatchAll(__FUNCTION__);
}
