// EditInsuranceRespTypesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditInsuranceRespTypesDlg.h"
#include "SharedInsuranceUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//type columns
// (j.jones 2010-06-16 12:01) - PLID 39190 - converted to an enum,
// added "old" columns, and added the HasBillingColumn column
// (j.jones 2010-08-17 09:33) - PLID 40134 - added CategoryType
enum TypeListColumns {

	tlcID = 0,
	tlcOldName,
	tlcName,
	tlcOldPriority,
	tlcPriority,
	tlcOldCategoryType,
	tlcCategoryType,	
	tlcOldCategoryPlacement,
	tlcCategoryPlacement,
	tlcPlacement,
	tlcOldHasBillingColumn,
	tlcHasBillingColumn,
	tlcOldBillDescription,
	tlcBillDescription, // (j.gruber 2012-01-04 08:51) - PLID 47322 - bill description
};

// (j.jones 2011-03-31 09:13) - PLID 43044 - added color for primary & secondary placements
#define PRIMARY_PLACEMENT_COLOR RGB(192,0,0)
#define SECONDARY_PLACEMENT_COLOR RGB(0,0,128)

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALIST2Lib;
/////////////////////////////////////////////////////////////////////////////
// CEditInsuranceRespTypesDlg dialog


CEditInsuranceRespTypesDlg::CEditInsuranceRespTypesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditInsuranceRespTypesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditInsuranceRespTypesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditInsuranceRespTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditInsuranceRespTypesDlg)
	DDX_Control(pDX, IDC_MOVE_DOWN_BTN, m_btnMoveDown);
	DDX_Control(pDX, IDC_MOVE_UP_BTN, m_btnMoveUp);
	DDX_Control(pDX, IDC_NEW_RESP_BTN, m_btnNewResp);
	DDX_Control(pDX, IDC_DELETE_RESP_BTN, m_btnDeleteResp);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditInsuranceRespTypesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditInsuranceRespTypesDlg)
	ON_BN_CLICKED(IDC_NEW_RESP_BTN, OnNewBtn)
	ON_BN_CLICKED(IDC_DELETE_RESP_BTN, OnDeleteBtn)
	ON_BN_CLICKED(IDC_MOVE_UP_BTN, OnMoveUpBtn)
	ON_BN_CLICKED(IDC_MOVE_DOWN_BTN, OnMoveDownBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditInsuranceRespTypesDlg message handlers

void CEditInsuranceRespTypesDlg::OnOK() 
{
	try {

		BOOL bChanged = FALSE;

		//all saving is done when they hit OK
		CArray<long, long> aryIDs;	//array of IDs in the data
		CString str, str2, str3, str4, str5, strExecute, strDeleteSql;

		long nNextID = NewNumber("RespTypeT", "ID");	//this will be the next number if we need it

		//1)  Get a list of everything in data
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM RespTypeT WHERE ID > 0");
		while(!rs->eof) {
			aryIDs.Add(AdoFldLong(rs, "ID"));

			rs->MoveNext();
		}
		rs->Close();

		// (m.cable 7/7/2004 17:19) - Need these update statments with case statments because otherwise we will have 
		// issues changing priorities due to the unique key contraint
		CString strTemp = "UPDATE RespTypeT SET TypeName = CASE ";
		CString strTemp2 = "UPDATE RespTypeT SET Priority = CASE ";
		CString strTemp3 = "UPDATE RespTypeT SET HasBillingColumn = CASE ";
		// (j.jones 2010-08-17 09:33) - PLID 40134 - added CategoryType
		CString strTemp4 = "UPDATE RespTypeT SET CategoryType = CASE ";
		// (j.gruber 2012-01-04 10:51) - PLID 47322 - bill description
		CString strTemp5 = "UPDATE RespTypeT SET BillDescription = CASE ";
		CString strIDs = "";
		bool bAddToStatement = false;
		
		//now loop through all of these IDs and compare against our datalist
		for(int i = aryIDs.GetSize() - 1; i >= 0; i--) {

			//find it in the datalist
			long nID = aryIDs.GetAt(i);
			// (j.jones 2011-03-30 11:10) - PLID 43044 - changed to a datalist 2
			IRowSettingsPtr pFound = m_List->FindByColumn(tlcID, long(nID), m_List->GetFirstRow(), VARIANT_FALSE);			
			if(pFound) {

				aryIDs.RemoveAt(i);	//get it out of the list

				// (j.jones 2010-06-17 15:33) - PLID 39190 - only update if something changed
				CString strOldName = VarString(pFound->GetValue(tlcOldName), "");
				CString strName = VarString(pFound->GetValue(tlcName));
				long nOldPriority = VarLong(pFound->GetValue(tlcOldPriority), -2);
				long nPriority = VarLong(pFound->GetValue(tlcPriority));
				// (j.jones 2010-08-17 09:33) - PLID 40134 - added CategoryType
				RespCategoryType eOldCategoryType = (RespCategoryType)VarLong(pFound->GetValue(tlcOldCategoryType), (long)rctInvalidRespCategory);
				RespCategoryType eCategoryType = (RespCategoryType)VarLong(pFound->GetValue(tlcCategoryType));
				BOOL bOldHasBillingColumn = VarBool(pFound->GetValue(tlcOldHasBillingColumn), FALSE);
				BOOL bHasBillingColumn = VarBool(pFound->GetValue(tlcHasBillingColumn));
				// (j.jones 2011-03-31 09:31) - PLID 43044 - added CategoryPlacement
				long nOldCategoryPlacement = VarLong(pFound->GetValue(tlcOldCategoryPlacement), -1);
				long nCategoryPlacement = VarLong(pFound->GetValue(tlcCategoryPlacement), -1);				
				// (j.gruber 2012-01-04 10:53) - PLID 47322
				CString strOldBillDescription = VarString(pFound->GetValue(tlcOldBillDescription), "");
				CString strBillDescription = VarString(pFound->GetValue(tlcBillDescription), "");


				if(strOldName != strName || nOldPriority != nPriority || bOldHasBillingColumn != bHasBillingColumn
					|| eOldCategoryType != eCategoryType 
					|| strOldBillDescription != strBillDescription
					) {

					bChanged = TRUE;

					//Update the data with whatever is in the datalist
					str.Format("WHEN ID = %li THEN '%s' ", nID, _Q(strName)); 
					str2.Format("WHEN ID = %li THEN %li ", nID, nPriority);
					str3.Format("WHEN ID = %li THEN %li ", nID, bHasBillingColumn ? 1 : 0);
					str4.Format("WHEN ID = %li THEN %li ", nID, (long)eCategoryType);
					// (j.gruber 2012-01-04 10:55) - PLID 47322
					str5.Format("WHEN ID = %li THEN '%s' ", nID, _Q(strBillDescription));

					CString strID;
					strID.Format("%li, ", nID);
					strIDs += strID;
					strTemp += str;
					strTemp2 += str2;
					strTemp3 += str3;		
					strTemp4 += str4;
					// (j.gruber 2012-01-04 10:55) - PLID 47322
					strTemp5 += str5;
					bAddToStatement = true;
				}
				
				// (j.jones 2011-03-31 09:31) - PLID 43044 - added CategoryPlacement
				if(nOldCategoryPlacement != nCategoryPlacement) {

					bChanged = TRUE;
					CString strCategoryPlacement;
					strCategoryPlacement.Format("UPDATE RespTypeT SET CategoryPlacement = %s WHERE ID = %li\n",
						nCategoryPlacement > 0 ? AsString(nCategoryPlacement) : "NULL", nID);
					strExecute += strCategoryPlacement;
				}
			}
		}
		if(bAddToStatement){
			strIDs.TrimRight(", '");
			strExecute += strTemp + " END WHERE ID IN (" + strIDs + ")\n" +  strTemp2 + " END WHERE ID IN (" + strIDs + ")\n" +  strTemp3 + " END WHERE ID IN (" + strIDs + ")\n" +  strTemp4 + " END WHERE ID IN (" + strIDs + ")\n"+  strTemp5 + " END WHERE ID IN (" + strIDs + ")\n";
		}

		for(int j = 0; j < aryIDs.GetSize(); j++) {
			//exist in the data but are not found in our datalist.  Queueing for deletion...
			//1)  Check to see if anyone is using this.  If so we can't delete.
			//DRT 6/10/03 - Moved this to the delete function, so if we get to this point, we've already done the check

			//2)  deleteme!
			CString strDeleteTemp;
			strDeleteTemp.Format("DELETE FROM RespTypeT WHERE ID = %li;", aryIDs.GetAt(j));
			strDeleteSql += strDeleteTemp;

			bChanged = TRUE;
		}

		//all new items have an ID of -2
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			if(VarLong(pRow->GetValue(tlcID)) == -2) {
				// (j.jones 2010-06-16 12:02) - PLID 39190 - added HasBillingColumn
				// (j.jones 2010-08-17 14:57) - PLID 40134 - added CategoryType
				// (j.jones 2011-03-31 09:31) - PLID 43044 - added CategoryPlacement
				str.Format("INSERT INTO RespTypeT (ID, TypeName, Priority, HasBillingColumn, CategoryType, CategoryPlacement) "
					"VALUES (%li, '%s', %li, %li, %li, %s)", nNextID, 
					_Q(VarString(pRow->GetValue(tlcName))),
					VarLong(pRow->GetValue(tlcPriority)),
					VarBool(pRow->GetValue(tlcHasBillingColumn)) ? 1 : 0,
					VarLong(pRow->GetValue(tlcCategoryType)),
					VarLong(pRow->GetValue(tlcCategoryPlacement), -1) > 0 ? AsString(VarLong(pRow->GetValue(tlcCategoryPlacement))) : "NULL");
				strExecute += str + "\n";
				nNextID++;

				bChanged = TRUE;
			}

			pRow = pRow->GetNextRow();
		}
		
		strExecute = strDeleteSql + strExecute;

		// (j.dinatale 2010-10-28) - PLID 28773 - Need to add entries and/or remove entries from RespTypeColumnWidthsT when RespType get modified
		// (j.jones 2014-11-13 17:13) - PLID 64116 - if they delete the last resp type for a category, and that category is in use in the
		// AutoFillApptInsurance_DefaultCategory, we need to reset the preference to use the Medical category.
		strExecute += FormatString(
			"DECLARE @UserID int; \r\n"

			"DECLARE AllUserBillTabColInfo CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n"
			"SELECT DISTINCT UserID FROM BillingColumnsT; \r\n"

			"OPEN AllUserBillTabColInfo; \r\n"

			"FETCH FROM AllUserBillTabColInfo INTO @UserID; \r\n"

			"WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
			"INSERT INTO RespTypeColumnWidthsT(RespTypeID, UserID, StoredWidth) \r\n"
			"SELECT ID, @UserID, -1 FROM RespTypeT WHERE ID <> -1 AND ID NOT IN (SELECT RespTypeID FROM RespTypeColumnWidthsT WHERE UserID = @UserID) \r\n"
			"FETCH FROM AllUserBillTabColInfo INTO @UserID; \r\n"
			"END \r\n"

			"DELETE FROM RespTypeColumnWidthsT WHERE RespTypeID NOT IN (SELECT ID FROM RespTypeT WHERE ID <> -1) AND RespTypeID > -1 \r\n"

			"UPDATE ConfigRT SET IntParam = %li WHERE Name = 'AutoFillApptInsurance_DefaultCategory' AND IntParam NOT IN (SELECT CategoryType FROM RespTypeT) \r\n"

			"CLOSE AllUserBillTabColInfo \r\n"
			"DEALLOCATE AllUserBillTabColInfo \r\n",
			RespCategoryType::rctMedical);

		if(!strExecute.IsEmpty()) {
			BEGIN_TRANS("RespUpdate") {
				ExecuteSql("%s", strExecute);
			} END_TRANS_CATCH_ALL("RespUpdate");
		}

		// (j.jones 2010-06-17 15:15) - PLID 39190 - if something changed, refresh the tablechecker
		if(bChanged) {
			CClient::RefreshTable(NetUtils::RespTypeT);
		}

		CNxDialog::OnOK();

	} NxCatchAll("Error in OnOK()");
}

void CEditInsuranceRespTypesDlg::OnCancel() 
{
	//cancel anything ... i.e., don't save

	CDialog::OnCancel();
}

void CEditInsuranceRespTypesDlg::OnNewBtn() 
{
	try {

		//add a new resp type to the list
		CString strResult, strOther;

		int nRes;
		do {
			nRes = InputBoxLimited(this, "Enter a name for the new responsibility type:", strResult, strOther, 255, false, false, "Cancel");
			strResult.TrimRight();
			strResult.TrimLeft();
		} while(nRes == IDOK && strResult.IsEmpty());

		if(nRes == IDOK) {
			//make sure it doesn't already exist
			{
				IRowSettingsPtr pRow = m_List->GetFirstRow();
				while(pRow) {
					if(stricmp(strResult, VarString(pRow->GetValue(tlcName))) == 0) {
						MsgBox("You cannot have 2 resp types of the same name.");
						return;
					}

					pRow = pRow->GetNextRow();
				}
			}

			//find the last row and get it's priority
			IRowSettingsPtr pLastRow = m_List->GetLastRow();
			long nNewPriority = VarLong(pLastRow->GetValue(tlcPriority)) + 1;

			IRowSettingsPtr pNewRow = m_List->GetNewRow();
			pNewRow->PutValue(tlcID, long(-2));	//-2 means it needs a new number generated during the save operation
			pNewRow->PutValue(tlcOldName, g_cvarNull);
			pNewRow->PutValue(tlcName, _bstr_t(strResult));
			pNewRow->PutValue(tlcOldPriority, g_cvarNull);
			pNewRow->PutValue(tlcPriority, long(nNewPriority));
			// (j.jones 2010-08-17 09:33) - PLID 40134 - added CategoryType
			pNewRow->PutValue(tlcOldCategoryType, g_cvarNull);
			pNewRow->PutValue(tlcCategoryType, long(rctMedical));
			pNewRow->PutValue(tlcOldHasBillingColumn, g_cvarNull);
			// (j.jones 2010-06-16 12:02) - PLID 39190 - new types default HasBillingColumn to off
			pNewRow->PutValue(tlcHasBillingColumn, g_cvarFalse);
			// (j.gruber 2012-01-04 10:57) - PLID 47322 - new values are null
			pNewRow->PutValue(tlcOldBillDescription, g_cvarNull);
			pNewRow->PutValue(tlcBillDescription, g_cvarNull);

			m_List->AddRowSorted(pNewRow, NULL);
		}

		// (j.jones 2011-03-31 09:24) - PLID 43044 - update CategoryPlacement
		EnsureCategoryPlacement();

	} NxCatchAll("Error in OnNewBtn()");
}

void CEditInsuranceRespTypesDlg::OnDeleteBtn() 
{
	try {
		//delete the current resp from the list
		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			MsgBox("Please highlight an item before pressing delete.");
			return;
		}

		long nCurPriority = VarLong(pRow->GetValue(tlcPriority));

		if(nCurPriority == 1 || nCurPriority == 2) {
			//can't delete pri/sec
			// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
			MsgBox("You cannot delete primary or secondary medical responsibilities.");
			return;
		}

		_RecordsetPtr rsDelete = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = {INT};", VarLong(pRow->GetValue(tlcID)));
		if(!rsDelete->eof) {
			MsgBox("There are insured parties related to this type.  Please remove those insured parties first.");
			return;
		}

		m_List->RemoveRow(pRow);

		EnsurePriorities();		//we need to make sure we update the priorities so they don't get out of sync

	} NxCatchAll("Error in OnDeleteBtn()");
}

void CEditInsuranceRespTypesDlg::OnMoveUpBtn() 
{
	try {
		//up the priority
		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		long nCurPriority = VarLong(pRow->GetValue(tlcPriority));
		if(nCurPriority <= 3) {
			MsgBox("You cannot move an item any higher than the third priority.");
			return;
		}

		//take the row above us and move it's priority down 1 (add 1 to it)
		IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
		long nToMovePriority = VarLong(pPrevRow->GetValue(tlcPriority));
		pPrevRow->PutValue(tlcPriority, long(nToMovePriority+1));

		//now take our priority and move it up 1 (subtract 1 from it)
		pRow->PutValue(tlcPriority, long(nCurPriority-1));

		//make sure everything is in order
		EnsurePriorities();

	} NxCatchAll("Error in OnMoveUpBtn()");
}

void CEditInsuranceRespTypesDlg::OnMoveDownBtn() 
{
	try {
		//lower the priority
		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		long nCurPriority = VarLong(pRow->GetValue(tlcPriority));
		if(nCurPriority < 3) {
			//pri/sec MUST always be 1/2
			// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
			MsgBox("You cannot change the priority of the primary or secondary medical insurance responsibilities.");
			return;
		}

		IRowSettingsPtr pNextRow = pRow->GetNextRow();
		if(pNextRow == NULL) {
			//we're moving the last row.  just return, it's already moved down!
			return;
		}

		//take the row below us and move it up (subtract 1 from it)
		long nToMovePriority = VarLong(pNextRow->GetValue(tlcPriority));
		pNextRow->PutValue(tlcPriority, long(nToMovePriority-1));

		//take our row and move it down 1 (add 1 to it)
		pRow->PutValue(tlcPriority, long(nCurPriority+1));

		EnsurePriorities();

	} NxCatchAll("Error in OnMoveDownBtn()");
}

void CEditInsuranceRespTypesDlg::EnsurePriorities() {

	try {

		m_List->Sort();

		long nCur = -1;
		long nLastFound = 0;
		long nShift = 0;

		//loop through all items and make sure there are no gaps - if we find
		//any, just shift the rest down into it
		// (j.jones 2011-03-31 08:59) - PLID 43044 - switched to a datalist 2
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			//get current priority
			nCur = VarLong(pRow->GetValue(tlcPriority));

			//see if we need to shift at all
			if(nShift > 0) {
				pRow->PutValue(tlcPriority, long(nCur + nShift));
				nCur += nShift;
			}

			//compare it against the last.  If it's 1 higher, we're good.  If not we need to shift
			if(nCur - nLastFound == 1) {
				//good to go
			}
			else {
				//we need to shift all items down by the difference
				nShift += nCur - nLastFound - 1;
			}

			nLastFound = nCur;

			//DRT 6/16/03 - Apparently this code never actually did anything, it just figured some 
			//		stuff out and went on it's merry way.
			if(nShift > 0) {
				pRow->PutValue(tlcPriority, long(nCur - nShift));
			}

			pRow = pRow->GetNextRow();
		}

		// (j.jones 2011-03-31 09:24) - PLID 43044 - update CategoryPlacement
		EnsureCategoryPlacement();

	} NxCatchAll("Error in EnsurePriorities()");
}

BOOL CEditInsuranceRespTypesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:14) - PLID 29817 - NxIconified buttons
		m_btnNewResp.AutoSet(NXB_NEW);
		m_btnDeleteResp.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_List = BindNxDataList2Ctrl(IDC_LIST_RESP_TYPES, true);
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);

		// (j.gruber 2012-07-26 09:52) - PLID 51777 - add types
		IColumnSettingsPtr pCol = m_List->GetColumn(tlcCategoryType);
		if (pCol) {
			pCol->PutComboSource("1;Medical;2;Vision;3;Auto;4;Workers' Comp.;5;Dental;6;Study;7;Letter of Protection;8;Letter of Agreement");
		}


		if(m_List->GetCurSel() == NULL){
			GetDlgItem(IDC_DELETE_RESP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(FALSE);
		}
		else{
			GetDlgItem(IDC_DELETE_RESP_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(TRUE);
		}
	}
	NxCatchAll("Error in CEditInsuranceRespTypesDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditInsuranceRespTypesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditInsuranceRespTypesDlg)	
	ON_EVENT(CEditInsuranceRespTypesDlg, IDC_LIST_RESP_TYPES, 10, OnEditingFinishedListRespTypes, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditInsuranceRespTypesDlg, IDC_LIST_RESP_TYPES, 18, OnRequeryFinishedListRespTypes, VTS_I2)
	ON_EVENT(CEditInsuranceRespTypesDlg, IDC_LIST_RESP_TYPES, 2, OnSelChangedListRespTypes, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEditInsuranceRespTypesDlg, IDC_LIST_RESP_TYPES, 8, OnEditingStartingListRespTypes, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditInsuranceRespTypesDlg::OnSelChangedListRespTypes(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
	
		IRowSettingsPtr pRow(lpNewSel);

		if(pRow == NULL){
			GetDlgItem(IDC_DELETE_RESP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(FALSE);
		}
		else{
			GetDlgItem(IDC_DELETE_RESP_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-16 13:22) - PLID 39190 - ensured you can't change HasBillingColumn for Primary/Secondary
void CEditInsuranceRespTypesDlg::OnEditingStartingListRespTypes(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//The category and 'has billing column' settings are disabled for inactive as well, but we do not
		//currently show the inactive resp. type in this setup. If we ever do, we should change these
		//messageboxes to reflect that you can't edit the inactive settings.

		long nCurPriority = VarLong(pRow->GetValue(tlcPriority));
		if(nCurPriority < 3) {

			// (j.jones 2010-08-17 09:41) - PLID 40134 - you can't change the category for Primary/Secondary/Inactive
			if(nCol == tlcCategoryType) {
				MsgBox("You cannot edit the category for primary or secondary medical responsibilities.");
				*pbContinue = FALSE;
				return;
			}
			else if(nCol == tlcHasBillingColumn) {
				MsgBox("You cannot edit the 'Show On Billing Tab' setting for primary or secondary medical responsibilities.");
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-31 09:06) - PLID 43044 - added OnEditingFinished
void CEditInsuranceRespTypesDlg::OnEditingFinishedListRespTypes(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if(!bCommit) {
			return;
		}

		// (j.jones 2011-03-31 09:08) - PLID 43044 - if they changed the category,
		// we need to refresh the category placement
		if(nCol == tlcCategoryType) {
			EnsureCategoryPlacement();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-31 09:07) - PLID 43044 - this function will
// keep RespTypeT.CategoryPlacement up to date
void CEditInsuranceRespTypesDlg::EnsureCategoryPlacement()
{
	try {


		// (j.gruber 2012-07-25 14:47) - PLID 51777 - add other categories

		BOOL bFoundFirstPrimaryMedical = FALSE;
		BOOL bFoundFirstSecondaryMedical = FALSE;
		BOOL bFoundFirstPrimaryVision = FALSE;
		BOOL bFoundFirstSecondaryVision = FALSE;
		BOOL bFoundFirstPrimaryAuto = FALSE;
		BOOL bFoundFirstSecondaryAuto = FALSE;
		BOOL bFoundFirstPrimaryWorkersComp = FALSE;
		BOOL bFoundFirstSecondaryWorkersComp = FALSE;
		BOOL bFoundFirstPrimaryDental = FALSE;
		BOOL bFoundFirstSecondaryDental = FALSE;
		BOOL bFoundFirstPrimaryStudy = FALSE;
		BOOL bFoundFirstSecondaryStudy = FALSE;
		BOOL bFoundFirstPrimaryLOP = FALSE;
		BOOL bFoundFirstSecondaryLOP = FALSE;
		BOOL bFoundFirstPrimaryLOA = FALSE;
		BOOL bFoundFirstSecondaryLOA = FALSE;

		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			long nID = VarLong(pRow->GetValue(tlcID), -1);
			long nPriority = VarLong(pRow->GetValue(tlcPriority), -1);
			RespCategoryType eCategoryType = (RespCategoryType)VarLong(pRow->GetValue(tlcCategoryType), rctInvalidRespCategory);

			if(eCategoryType == rctMedical && !bFoundFirstPrimaryMedical && nPriority > 0) {
				if(nPriority != 1 || nID != 1) {
					//this is bad data, we expect these to be 1
					ThrowNxException("Invalid data exists for Primary Medical Resp (ID: %li, Priority: %li)", nID, nPriority);
				}

				//this is the first medical insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Medical");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryMedical = TRUE;
			}
			else if(eCategoryType == rctMedical && bFoundFirstPrimaryMedical && !bFoundFirstSecondaryMedical && nPriority > 0) {

				if(nPriority != 2 || nID != 2) {
					//this is bad data, we expect these to be 2
					ThrowNxException("Invalid data exists for Secondary Medical Resp (ID: %li, Priority: %li)", nID, nPriority);
				}

				//this is the second medical insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Medical");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryMedical = TRUE;
			}
			else if(eCategoryType == rctVision && !bFoundFirstPrimaryVision && nPriority > 0) {
				//this is the first vision insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Vision");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryVision = TRUE;
			}
			else if(eCategoryType == rctVision && bFoundFirstPrimaryVision && !bFoundFirstSecondaryVision && nPriority > 0) {
				//this is the second vision insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Vision");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryVision = TRUE;
			}
			// (j.gruber 2012-07-25 14:50) - PLID 51777 - more categories
			else if(eCategoryType == rctAuto && !bFoundFirstPrimaryAuto && nPriority > 0) {
				//this is the first Auto insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Auto");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryAuto = TRUE;
			}
			else if(eCategoryType == rctAuto && bFoundFirstPrimaryAuto && !bFoundFirstSecondaryAuto && nPriority > 0) {
				//this is the second auto insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Auto");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryAuto = TRUE;
			}
			else if(eCategoryType == rctWorkersComp && !bFoundFirstPrimaryWorkersComp && nPriority > 0) {
				//this is the first workers comp insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Workers' Comp.");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryWorkersComp = TRUE;
			}
			else if(eCategoryType == rctWorkersComp&& bFoundFirstPrimaryWorkersComp && !bFoundFirstSecondaryWorkersComp && nPriority > 0) {
				//this is the second workers comp insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Workers' Comp.");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryWorkersComp = TRUE;
			}
			else if(eCategoryType == rctDental && !bFoundFirstPrimaryDental && nPriority > 0) {
				//this is the first dental insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Dental");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryDental = TRUE;
			}
			else if(eCategoryType == rctDental && bFoundFirstPrimaryDental && !bFoundFirstSecondaryDental && nPriority > 0) {
				//this is the second Dental insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Dental");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryDental = TRUE;
			}
			else if(eCategoryType == rctStudy && !bFoundFirstPrimaryStudy && nPriority > 0) {
				//this is the first Study insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Study");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryStudy = TRUE;
			}
			else if(eCategoryType == rctStudy && bFoundFirstPrimaryStudy && !bFoundFirstSecondaryStudy && nPriority > 0) {
				//this is the second Study insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Study");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryStudy = TRUE;
			}
			else if(eCategoryType == rctLOP && !bFoundFirstPrimaryLOP && nPriority > 0) {
				//this is the first LOP insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Letter of Protection");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryLOP = TRUE;
			}
			else if(eCategoryType == rctLOP && bFoundFirstPrimaryLOP && !bFoundFirstSecondaryLOP && nPriority > 0) {
				//this is the second LOP insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Letter of Protection");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryLOP = TRUE;
			}
			else if(eCategoryType == rctLOA && !bFoundFirstPrimaryLOA && nPriority > 0) {
				//this is the first LOA insurance
				pRow->PutValue(tlcCategoryPlacement, (long)1);
				pRow->PutValue(tlcPlacement, "Primary Letter of Agreement");
				pRow->PutCellForeColor(tlcPlacement, PRIMARY_PLACEMENT_COLOR);

				bFoundFirstPrimaryLOA = TRUE;
			}
			else if(eCategoryType == rctLOA && bFoundFirstPrimaryLOA && !bFoundFirstSecondaryLOA && nPriority > 0) {
				//this is the second LOA insurance
				pRow->PutValue(tlcCategoryPlacement, (long)2);
				pRow->PutValue(tlcPlacement, "Secondary Letter of Agreement");
				pRow->PutCellForeColor(tlcPlacement, SECONDARY_PLACEMENT_COLOR);

				bFoundFirstSecondaryLOA = TRUE;
			}
			else {
				//set this to no placement
				pRow->PutValue(tlcCategoryPlacement, g_cvarNull);
				pRow->PutValue(tlcPlacement, "");
				pRow->PutCellForeColor(tlcPlacement, RGB(0,0,0));
			}

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-31 09:15) - PLID 43044 - added coloring to the placement column
void CEditInsuranceRespTypesDlg::OnRequeryFinishedListRespTypes(short nFlags)
{
	try {

		EnsureCategoryPlacement();

	}NxCatchAll(__FUNCTION__);
}