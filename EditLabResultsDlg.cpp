// EditLabResultsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditLabResultsDlg.h"
#include "NxStandard.h"
#include "GlobalLabUtils.h"
#include "HL7ParseUtils.h"
#include "PatientsRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CEditLabResultsDlg dialog

enum ResultListColumns{
	rlcID =0,
	rlcName,
	rlcValue,
	rlcInactive, //TES 2/2/2010 - PLID 34336
	rlcPriority, //TES 7/31/2013 - PLID 54573
	rlcToDoPriority, //TES 8/5/2013 - PLID 51147
};


CEditLabResultsDlg::CEditLabResultsDlg(CWnd* pParent)
	: CNxDialog(CEditLabResultsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditLabResultsDlg)
		m_nCurResultIDInUse = -1;
	//}}AFX_DATA_INIT
}


void CEditLabResultsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditLabResultsDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EDIT_RESULT, m_btnEdit);
	DDX_Control(pDX, IDC_DELETE_RESULT, m_btnDelete);
	DDX_Control(pDX, IDC_ADD_RESULT, m_btnAdd);
	DDX_Control(pDX, IDC_MOVE_FLAG_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_MOVE_FLAG_DOWN, m_btnMoveDown);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditLabResultsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditLabResultsDlg)
	ON_BN_CLICKED(IDC_ADD_RESULT, OnAddResult)
	ON_BN_CLICKED(IDC_DELETE_RESULT, OnDeleteResult)
	ON_BN_CLICKED(IDC_EDIT_RESULT, OnEditResult)
	ON_BN_CLICKED(IDC_MOVE_FLAG_UP, OnMoveFlagUp)
	ON_BN_CLICKED(IDC_MOVE_FLAG_DOWN, OnMoveFlagDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditLabResultsDlg message handlers

BOOL CEditLabResultsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try{
		// (c.haag 2008-04-29 10:16) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnAdd.AutoSet(NXB_NEW);

		//TES 7/31/2013 - PLID 54573
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);
	
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pResultList = BindNxDataList2Ctrl(IDC_LAB_RESULT_LIST);

		//We will manually set the "value" column's combo box values
		IColumnSettingsPtr pCol = m_pResultList->GetColumn(rlcValue);
		CString strComboSource;
		strComboSource.Format("%li;Positive;%li;Negative;", rvPositive, rvNegative);

		pCol->ComboSource = (LPCTSTR)strComboSource;

		//TES 7/31/2013 - PLID 54573 - Enable/disable buttons
		UpdateControls();

		return TRUE; 
	}NxCatchAll("Error in CEditLabResultsDlg::OnInitDialog()");
	return FALSE;
}

void CEditLabResultsDlg::OnOK() 
{
	CDialog::OnOK();
}

void CEditLabResultsDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CEditLabResultsDlg::OnAddResult() 
{
	try{
		//Add a new row into the list and set it to be editable
		IRowSettingsPtr pRow;
		pRow = m_pResultList->GetNewRow();

		//TES 12/16/2008 - PLID 32360 - The new value will be written to data immediately, so we need to make sure
		// that it's not a duplicate.
		CString strNewName = "Enter New Flag";
		BOOL bFound = (m_pResultList->FindByColumn(rlcName, _bstr_t(strNewName), NULL, VARIANT_FALSE) != NULL);
		for(int i = 1; i <= 100 && bFound; i++) {
			strNewName.Format("Enter New Flag%i", i);
			bFound = (m_pResultList->FindByColumn(rlcName, _bstr_t(strNewName), NULL, VARIANT_FALSE) != NULL);
		}
		if(bFound) {
			//TES 12/16/2008 - PLID 32360 - They have more than 100 "Enter New Flag"s already, there's no valid reason
			// for them to do that.
			AfxThrowNxException("Could not generate new Result Flag name!");
		}


		// (j.gruber 2008-10-06 08:36) - PLID 31432 - change to use LabResultFlagsT
		//TES 7/31/2013 - PLID 54573 - Fill the Priority column with the next highest number
		_RecordsetPtr prsID = CreateRecordset("SET NOCOUNT OFF \r\n "
			"INSERT INTO LabResultFlagsT (Name, Value, Priority) SELECT '%s', %li, (SELECT COALESCE(Max(Priority),0)+1 FROM LabResultFlagsT);" 
			"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum;",
			_Q(strNewName), (long) rvPositive /*Positive*/);

		_variant_t v;
		prsID = prsID->NextRecordset(&v);
		if (NULL == prsID) {
			//If this is NULL, something bad happened
			AfxThrowNxException("The lab flag record could not be made. Please try again or restart NexTech Practice if the problem persists.");
		}
		long nNewID = AdoFldLong(prsID, "NewNum");

		pRow->PutValue(rlcID, nNewID);
		pRow->PutValue(rlcName, _variant_t(strNewName));
		pRow->PutValue(rlcValue, (long) rvPositive /*Positive*/);
		//TES 2/2/2010 - PLID 34336 - Added an Inactive column.
		pRow->PutValue(rlcInactive, g_cvarFalse);
		//TES 8/5/2013 - PLID 51147 - Added TodoPriority, it defaults to 1 (High)
		pRow->PutValue(rlcToDoPriority, (long)1);

		m_pResultList->AddRowBefore(pRow, NULL);
		m_pResultList->SetSelByColumn(0, nNewID);
		m_pResultList->StartEditing(m_pResultList->GetCurSel(), 1);
	}NxCatchAll("Error in CEditLabResultsDlg::OnAddResult");
}

void CEditLabResultsDlg::OnDeleteResult() 
{
	try{
		//check to see that something is selected
		IRowSettingsPtr pCurSel = m_pResultList->GetCurSel();
	
		if (pCurSel != NULL) {
			if (IDYES == MessageBox("Are you sure you want to delete this lab flag?", "NexTech Practice", MB_YESNO|MB_ICONQUESTION)) {
	
				long nDelID = VarLong(pCurSel->GetValue(rlcID));

				//check to see that there are no patients with that allergy

				_RecordsetPtr rs;
				rs = CreateRecordset("SELECT Count(ResultID) as nCount FROM LabResultsT WHERE FlagID = %li", nDelID);
				long nCount  = AdoFldLong(rs, "nCount");
				
				if (nCount > 0 ) {
					//we can't delete
					CString str;
					str.Format("There are %li patients with this as a lab flag, you cannot delete this flag",  nCount);
					MessageBox(str);
				}
				else {
					//TES 5/26/2010 - PLID 38660 - We need to delete any code mappings for this result.
					ExecuteParamSql("DELETE FROM HL7CodeLinkT WHERE Type = {INT} AND PracticeID = {INT}", hclrtLabResultFlag, nDelID);
					//we can delete it!
					ExecuteSql("DELETE FROM LabResultFlagsT WHERE ID = %li", nDelID);
					m_pResultList->RemoveRow(pCurSel);
				}
			}
			else {
				//they don't want to delete, so do nothing
			}
		}
		else {
			//they haven't selected anything
			MessageBox("Please select a lab flag to delete");
		}
	}NxCatchAll("Error in CEditLabResultsDlg::OnDeleteResult");
	
}

void CEditLabResultsDlg::OnEditResult() 
{
	try{
		//check to see that something is selected

		IRowSettingsPtr pCurSel = m_pResultList->GetCurSel();

		if (pCurSel != NULL) {

			//make the datalist for that sel editable
			m_pResultList->StartEditing(pCurSel,1);
		}
		else {
			MessageBox("Please select a lab flag to edit");
		}
	}NxCatchAll("Error in CEditLabResultsDlg::OnEditResult");
	
}

BEGIN_EVENTSINK_MAP(CEditLabResultsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditLabResultsDlg)
	ON_EVENT(CEditLabResultsDlg, IDC_LAB_RESULT_LIST, 9 /* EditingFinishing */, OnEditingFinishingLabResultList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditLabResultsDlg, IDC_LAB_RESULT_LIST, 10 /* EditingFinished */, OnEditingFinishedLabResultList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditLabResultsDlg, IDC_LAB_RESULT_LIST, 8 /* EditingStarting */, OnEditingStartingLabResultList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEditLabResultsDlg, IDC_LAB_RESULT_LIST, 2 /* SelChanged */, OnSelChangedLabResultList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditLabResultsDlg::OnEditingFinishingLabResultList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try{
		//TES 12/8/2008 - PLID 32360 - If they're not committing, no need to validate.
		if(!(*pbCommit)) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		switch (nCol) {
			case rlcName:
			{
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (strTemp.IsEmpty()) {
					MessageBox("Please enter a valid flag name.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
				if(!IsRecordsetEmpty("SELECT Name FROM LabResultFlagsT WHERE ID <> %li AND Name = '%s'",VarLong(pRow->GetValue(rlcID)), _Q(strTemp))) {
					MessageBox("The flag you entered already exists in the list. Please enter a unique name.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
			}
			break;
			case rlcValue:
			{
				long nNewValue = VarLong(pRow->GetValue(rlcValue), -1);
				if(nNewValue != rvPositive /*positive*/ && nNewValue != rvNegative  /*Negative*/){
					MessageBox("Please enter a valid flag value.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
			}
			break;

			default:
			break;
		}
		
	}NxCatchAll("Error in CEditLabResultsDlg::OnEditingFinishingLabResultList");
	
}

void CEditLabResultsDlg::OnEditingFinishedLabResultList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try{
		//TES 12/8/2008 - PLID 32360 - Don't save if they're not committing!
		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if (pRow != NULL) {
			long nID;
			nID = VarLong(pRow->GetValue(rlcID));

			switch (nCol){
			case rlcName:
			{
				CString strValue = VarString(varNewValue);
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					pRow->PutValue(nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE LabResultFlagsT SET Name = '%s' WHERE ID = %li", _Q(strValue), nID);
			}
			break;
			case rlcValue:
			{
				ExecuteSql("UPDATE LabResultFlagsT SET Value = %li WHERE ID = %li", VarLong(varNewValue), nID);
			}
			break;
			case rlcInactive:
			{
				//TES 2/2/2010 - PLID 34336 - Support the Inactive flag
				ExecuteParamSql("UPDATE LabResultFlagsT SET Inactive = {BIT} WHERE ID = {INT}", VarBool(varNewValue), nID);
			}
			break;
			case rlcToDoPriority:
			{
				//TES 8/5/2013 - PLID 51147 - Support for the ToDoPriority column.
				ExecuteParamSql("UPDATE LabResultFlagsT SET ToDoPriority = {INT} WHERE ID = {INT}", VarLong(varNewValue), nID);
			}
			break;
			default:
				break; //do nothing
			}
		}
		
	}NxCatchAll("Error in CEditLabResultsDlg::OnEditingFinishedLabResultList");
	
}

// (j.jones 2007-07-20 12:53) - PLID 26749 - added ability to disallow changes to data
void CEditLabResultsDlg::OnEditingStartingLabResultList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		//TES 2/2/2010 - PLID 34336 - They can edit the Inactive flag even if it's in use.
		//TES 8/5/2013 - PLID 51147 - Also the Todo Priority field
		if (pRow != NULL && nCol != rlcInactive && nCol != rlcToDoPriority) {

			CString strSQL;

			long nResultID = VarLong(pRow->GetValue(rlcID), -1);
			strSQL.Format("SELECT ResultID FROM LabResultsT WHERE FlagID = %li", nResultID);

			long nCount = GetRecordCount(strSQL);
			if (nCount > 0)
			{
				MsgBox("You may not edit this flag because there are %d saved lab result(s) associated with it.", nCount);
				*pbContinue = FALSE;
				return;
			}
			//if m_nCurIDInUse is set, it means the calling Lab is using that ResultID, which might not be saved
			else if(nCount == 0 && m_nCurResultIDInUse == nResultID) {
				if(IDNO == MessageBox("The current lab is associated with this flag. Are you sure you wish to edit it?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					*pbContinue = FALSE;
					return;
				}
			}
		}
		
	}NxCatchAll("Error in CEditLabResultsDlg::OnEditingStartingLabResultList");
	
}

void CEditLabResultsDlg::OnSelChangedLabResultList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		UpdateControls();

	}NxCatchAll(__FUNCTION__);
}

//TES 7/31/2013 - PLID 54573 - Enable/disable buttons based on the current selection.
void CEditLabResultsDlg::UpdateControls()
{
	IRowSettingsPtr pRow = m_pResultList->CurSel;
	if(pRow == NULL) {
		//TES 7/31/2013 - PLID 54573 - Can't edit, delete, or move
		m_btnEdit.EnableWindow(FALSE);
		m_btnDelete.EnableWindow(FALSE);
		m_btnMoveUp.EnableWindow(FALSE);
		m_btnMoveDown.EnableWindow(FALSE);
	}
	else {
		//TES 7/31/2013 - PLID 54573 - Can edit and delete, can move if we're not at the end of the list
		m_btnEdit.EnableWindow(TRUE);
		m_btnDelete.EnableWindow(TRUE);
		m_btnMoveUp.EnableWindow(pRow->GetPreviousRow() != NULL);
		m_btnMoveDown.EnableWindow(pRow->GetNextRow() != NULL);
	}
}

void CEditLabResultsDlg::OnMoveFlagUp()
{
	try {
		//TES 7/31/2013 - PLID 54573 - Find this row and the previous one.
		IRowSettingsPtr pRow = m_pResultList->CurSel;
		if(pRow) {
			IRowSettingsPtr pRow2 = pRow->GetPreviousRow();
			if(pRow2) {
				long nPriority = VarLong(pRow->GetValue(rlcPriority));
				long nID = VarLong(pRow->GetValue(rlcID));
				long nPriority2 = VarLong(pRow2->GetValue(rlcPriority));
				long nID2 = VarLong(pRow2->GetValue(rlcID));
				//TES 7/31/2013 - PLID 54573 - Swap their priorities in data
				ExecuteParamSql("UPDATE LabResultFlagsT SET Priority = -1 WHERE ID = {INT} \r\n"
					"UPDATE LabResultFlagsT SET Priority = {INT} WHERE ID = {INT} \r\n"
					"UPDATE LabResultFlagsT SET Priority = {INT} WHERE ID = {INT} \r\n",
					nID2,
					nPriority2, nID,
					nPriority, nID2);
				//TES 7/31/2013 - PLID 54573 - Now swap them in the datalist, and resort.
				pRow->PutValue(rlcPriority, nPriority2);
				pRow2->PutValue(rlcPriority, nPriority);
				m_pResultList->Sort();
				UpdateControls();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditLabResultsDlg::OnMoveFlagDown()
{
	try {
		//TES 7/31/2013 - PLID 54573 - Find this row and the next one.
		IRowSettingsPtr pRow = m_pResultList->CurSel;
		if(pRow) {
			IRowSettingsPtr pRow2 = pRow->GetNextRow();
			if(pRow2) {
				long nPriority = VarLong(pRow->GetValue(rlcPriority));
				long nID = VarLong(pRow->GetValue(rlcID));
				long nPriority2 = VarLong(pRow2->GetValue(rlcPriority));
				long nID2 = VarLong(pRow2->GetValue(rlcID));
				//TES 7/31/2013 - PLID 54573 - Swap their priorities in data
				ExecuteParamSql("UPDATE LabResultFlagsT SET Priority = -1 WHERE ID = {INT} \r\n"
					"UPDATE LabResultFlagsT SET Priority = {INT} WHERE ID = {INT} \r\n"
					"UPDATE LabResultFlagsT SET Priority = {INT} WHERE ID = {INT} \r\n",
					nID2,
					nPriority2, nID,
					nPriority, nID2);
				//TES 7/31/2013 - PLID 54573 - Now swap them in the datalist, and resort.
				pRow->PutValue(rlcPriority, nPriority2);
				pRow2->PutValue(rlcPriority, nPriority);
				m_pResultList->Sort();
				UpdateControls();
			}
		}
	}NxCatchAll(__FUNCTION__);
}