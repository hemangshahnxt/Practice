// CustomRecordSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "CustomRecordSetupDlg.h"
#include "globaldatautils.h"
#include "nxstandard.h"
#include "CustomRecordActionDlg.h"
#include "EMRCategoriesDlg.h"
#include "EMRHeaderDlg.h"
#include "OfficeVisitConfigDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_DELETE_PROCEDURE		38000
#define ID_DELETE_DIAG			38001

using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CCustomRecordSetupDlg dialog
using namespace ADODB;

CCustomRecordSetupDlg::CCustomRecordSetupDlg(CWnd* pParent)
	: CNxDialog(CCustomRecordSetupDlg::IDD, pParent),
	m_procedureNameChecker(NetUtils::AptPurposeT),
	m_ICD9Checker(NetUtils::DiagCodes)
{
	//{{AFX_DATA_INIT(CCustomRecordSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCustomRecordSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordSetupDlg)
	DDX_Control(pDX, IDC_RADIO_LIST_SELECT, m_btnList);
	DDX_Control(pDX, IDC_RADIO_TEXT_SELECT, m_btnText);
	DDX_Control(pDX, IDC_ITEM_NAME, m_nxeditItemName);
	DDX_Control(pDX, IDC_SENTENCE, m_nxeditSentence);
	DDX_Control(pDX, IDC_DATA_VALUE, m_nxeditDataValue);
	DDX_Control(pDX, IDC_DATA_TYPE_LABEL, m_nxstaticDataTypeLabel);
	DDX_Control(pDX, IDC_DATA_LABEL, m_nxstaticDataLabel);
	DDX_Control(pDX, IDC_DATA_LABEL2, m_nxstaticDataLabel2);
	DDX_Control(pDX, IDC_ADD_INFO_ITEM, m_btnAddInfoItem);
	DDX_Control(pDX, IDC_DELETE_INFO_ITEM, m_btnDeleteInfoItem);
	DDX_Control(pDX, IDC_ADD_DATA_ITEM, m_btnAddDataItem);
	DDX_Control(pDX, IDC_DELETE_DATA_ITEM, m_btnDeleteDataItem);
	DDX_Control(pDX, IDC_ITEM_ACTION, m_btnItemAction);
	DDX_Control(pDX, IDC_DATA_ACTION, m_btnDataAction);
	DDX_Control(pDX, IDC_SETUP_HEADER, m_btnSetupHeader);
	DDX_Control(pDX, IDC_OFFICE_VISIT, m_btnOfficeVisit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomRecordSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustomRecordSetupDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_RADIO_TEXT_SELECT, OnRadioTextSelect)
	ON_BN_CLICKED(IDC_RADIO_LIST_SELECT, OnRadioListSelect)
	ON_BN_CLICKED(IDC_ADD_INFO_ITEM, OnAddInfoItem)
	ON_BN_CLICKED(IDC_DELETE_INFO_ITEM, OnDeleteInfoItem)
	ON_COMMAND(ID_DELETE_PROCEDURE, OnDeleteProcedure)
	ON_COMMAND(ID_DELETE_DIAG, OnDeleteDiag)
	ON_EN_KILLFOCUS(IDC_ITEM_NAME, OnKillfocusItemName)
	ON_EN_KILLFOCUS(IDC_SENTENCE, OnKillfocusSentence)
	ON_BN_CLICKED(IDC_ADD_DATA_ITEM, OnAddDataItem)
	ON_BN_CLICKED(IDC_DELETE_DATA_ITEM, OnDeleteDataItem)
	ON_EN_KILLFOCUS(IDC_DATA_VALUE, OnKillfocusDataValue)
	ON_BN_CLICKED(IDC_ITEM_ACTION, OnItemAction)
	ON_BN_CLICKED(IDC_DATA_ACTION, OnDataAction)
	ON_BN_CLICKED(IDC_EDIT_EMR_CATEGORIES, OnEditEMRCategories)
	ON_BN_CLICKED(IDC_SETUP_HEADER, OnSetupHeader)
	ON_BN_CLICKED(IDC_OFFICE_VISIT, OnOfficeVisit)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordSetupDlg message handlers

HBRUSH CCustomRecordSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// (a.walling 2008-05-12 13:30) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	/*
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() != IDC_DATA_VALUE )
	{
		if (pWnd->GetExStyle() & WS_EX_TRANSPARENT)
		{	pDC->SetBkMode (TRANSPARENT);
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}

		pDC->SetBkColor(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
		return hbr;
	}

	return hbr;
	*/
}

BOOL CCustomRecordSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (z.manning, 04/25/2008) - PLID 29566 - Set button styles		
		m_btnAddInfoItem.AutoSet(NXB_NEW);
		m_btnDeleteInfoItem.AutoSet(NXB_DELETE);
		m_btnAddDataItem.AutoSet(NXB_NEW);
		m_btnDeleteDataItem.AutoSet(NXB_DELETE);
		m_btnItemAction.AutoSet(NXB_MODIFY);
		m_btnDataAction.AutoSet(NXB_MODIFY);
		m_btnSetupHeader.AutoSet(NXB_MODIFY);
		m_btnOfficeVisit.AutoSet(NXB_MODIFY);

		//Bind all our datalists.
		m_pItemList = BindNxDataListCtrl(IDC_EMR_INFO_LIST);
		m_pProcedureDropdown = BindNxDataListCtrl(IDC_PROCEDURE);
		m_pProcedureList = BindNxDataListCtrl(IDC_EMR_PROCEDURE_LIST, false);
		m_pDiagDropdown = BindNxDataListCtrl(IDC_DIAG_DROPDOWN);
		m_pDiagList = BindNxDataListCtrl(IDC_DIAG_LIST, false);
		m_pCategoryList = BindNxDataListCtrl(IDC_EMR_CATEGORIES);
		m_pDataList = BindNxDataListCtrl(IDC_EMR_DATA_LIST, false);

		((CNxEdit*)GetDlgItem(IDC_ITEM_NAME))->LimitText(255);
		((CNxEdit*)GetDlgItem(IDC_SENTENCE))->LimitText(2000);
		((CNxEdit*)GetDlgItem(IDC_DATA_VALUE))->LimitText(2000);

		m_pItemList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		if(m_pItemList->GetRowCount() > 0) {
			m_pItemList->CurSel = 0;
			Load(VarLong(m_pItemList->GetValue(0,0)));
		}
		else{
			// the list is empty so disable the delete button
			GetDlgItem(IDC_DELETE_INFO_ITEM)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CCustomRecordSetupDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomRecordSetupDlg::Load(long nItemID)
{
	_RecordsetPtr rsItem = CreateRecordset("SELECT EmrInfoT.Name, EmrInfoT.DataType, EmrInfoT.LongForm, EmrInfoCategoryT.EmrCategoryID "
		"FROM EMRInfoT LEFT JOIN EmrInfoCategoryT ON EmrInfoT.ID = EmrInfoCategoryT.EmrInfoID "
		"WHERE EmrInfoT.ID = %li", nItemID);
	if(rsItem->eof) {
		AfxThrowNxException("Attempted to load invalid EMR Item");
		return;
	}
	SetDlgItemText(IDC_ITEM_NAME, AdoFldString(rsItem, "Name"));
	SetDlgItemText(IDC_SENTENCE, AdoFldString(rsItem, "LongForm", ""));
	m_pCategoryList->SetSelByColumn(0, AdoFldLong(rsItem, "EmrCategoryID", -1));

	if(AdoFldByte(rsItem, "DataType") == 2) {
		CheckDlgButton(IDC_RADIO_TEXT_SELECT, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_LIST_SELECT, BST_CHECKED);
		OnRadioListSelect();
	}
	else {
		CheckDlgButton(IDC_RADIO_LIST_SELECT, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_TEXT_SELECT, BST_CHECKED);
		OnRadioTextSelect();
	}

	CString strWhere;
	strWhere.Format("ProcedureToEMRInfoT.EMRInfoID = %li", nItemID);
	m_pProcedureList->WhereClause = _bstr_t(strWhere);
	m_pProcedureList->Requery();
	// (r.gonet 03/02/2014) - PLID 61131 - Removed support for ICD-10
	strWhere.Format("DiagCodeToEMRInfoT.EMRInfoID = %li AND DiagCodes.ICD10 = 0", nItemID);
	m_pDiagList->WhereClause = _bstr_t(strWhere);
	m_pDiagList->Requery();

}

void CCustomRecordSetupDlg::OnRadioTextSelect()
{
	try {
		if(m_pItemList->CurSel != -1) {
			ExecuteSql("UPDATE EMRInfoT SET DataType = 1 WHERE ID = %li", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)));
		}
		GetDlgItem(IDC_DATA_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATA_LABEL2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATA_ACTION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADD_DATA_ITEM)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DELETE_DATA_ITEM)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EMR_DATA_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATA_VALUE)->ShowWindow(SW_HIDE);
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnRadioTextSelect()");
}

void CCustomRecordSetupDlg::OnRadioListSelect()
{
	try {
		GetDlgItem(IDC_DATA_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DATA_LABEL2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DATA_ACTION)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ADD_DATA_ITEM)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DELETE_DATA_ITEM)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EMR_DATA_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DATA_VALUE)->ShowWindow(SW_SHOW);

		if(m_pItemList->CurSel != -1) {
			long nItemID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
			ExecuteSql("UPDATE EMRInfoT SET DataType = 2 WHERE ID = %li", nItemID);

			CString strWhere;
			strWhere.Format("EmrDataT.EMRInfoID = %li", nItemID, nItemID);
			m_pDataList->WhereClause = _bstr_t(strWhere);
			IColumnSettingsPtr pCol = m_pDataList->GetColumn(2);
			CString strFieldName;
			strFieldName.Format("CASE WHEN EMRInfoT.ID = %li THEN 1 ELSE 0 END", nItemID);
			//pCol->PutFieldName(_bstr_t(strFieldName));
			m_pDataList->Requery();
			m_pDataList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			if(m_pDataList->GetRowCount() > 0) {
				m_pDataList->CurSel = 0;
			}
			else {
				m_pDataList->CurSel = 1;
			}
			OnSelChangedEMRDataList(m_pDataList->CurSel);
		}

	}NxCatchAll("Error in CCustomRecordSetupDlg::OnRadioListSelect()");
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CCustomRecordSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCustomRecordSetupDlg)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_INFO_LIST, 16 /* SelChosen */, OnSelChosenEMRInfoList, VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_INFO_LIST, 1 /* SelChanging */, OnSelChangingEMRInfoList, VTS_PI4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_DATA_LIST, 16 /* SelChosen */, OnSelChosenEMRDataList, VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_DATA_LIST, 2 /* SelChanged */, OnSelChangedEMRDataList, VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_PROCEDURE, 16 /* SelChosen */, OnSelChosenProcedure, VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_PROCEDURE_LIST, 7 /* RButtonUp */, OnRButtonUpEMRProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_PROCEDURE_LIST, 10 /* EditingFinished */, OnEditingFinishedEMRProcedureList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCustomRecordSetupDlg, IDC_DIAG_DROPDOWN, 16 /* SelChosen */, OnSelChosenDiagDropdown, VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_DIAG_LIST, 7 /* RButtonUp */, OnRButtonUpDiagList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_DIAG_LIST, 10 /* EditingFinished */, OnEditingFinishedDiagList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_CATEGORIES, 16 /* SelChosen */, OnSelChosenEMRCategories, VTS_I4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_CATEGORIES, 1 /* SelChanging */, OnSelChangingEMRCategories, VTS_PI4)
	ON_EVENT(CCustomRecordSetupDlg, IDC_EMR_DATA_LIST, 10 /* EditingFinished */, OnEditingFinishedEMRDataList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCustomRecordSetupDlg::OnSelChosenEMRInfoList(long nRow)
{
	if(nRow == -1) {
		m_pItemList->CurSel = 0;
		Load(VarLong(m_pItemList->GetValue(0, 0)));
	}
	else {
		Load(VarLong(m_pItemList->GetValue(nRow, 0)));
	}
}

void CCustomRecordSetupDlg::OnSelChangingEMRInfoList(long FAR* nNewSel)
{
	// TODO: Add your control notification handler code here

}

void CCustomRecordSetupDlg::OnSelChosenEMRDataList(long nRow)
{

}

void CCustomRecordSetupDlg::OnSelChangedEMRDataList(long nNewSel)
{
	try {
		if(nNewSel == -1) {
			//Disable everything.
			SetDlgItemText(IDC_DATA_VALUE, "");
			GetDlgItem(IDC_DATA_VALUE)->EnableWindow(FALSE);
			GetDlgItem(IDC_DELETE_DATA_ITEM)->EnableWindow(FALSE);
			GetDlgItem(IDC_DATA_ACTION)->EnableWindow(FALSE);
		}
		else {
			//Enable everything.
			GetDlgItem(IDC_DATA_VALUE)->EnableWindow(TRUE);
			GetDlgItem(IDC_DELETE_DATA_ITEM)->EnableWindow(TRUE);
			GetDlgItem(IDC_DATA_ACTION)->EnableWindow(TRUE);

			//Load.
			_RecordsetPtr rsDataItem = CreateRecordset("SELECT Data FROM EMRDataT WHERE ID = %li", VarLong(m_pDataList->GetValue(nNewSel, 0)));
			if(rsDataItem->eof) {
				AfxThrowNxException("Attempted to load invalid data item");
			}
			else {
				SetDlgItemText(IDC_DATA_VALUE, AdoFldString(rsDataItem, "Data"));
			}

		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnSelChangedEMRDataList()");
}

void CCustomRecordSetupDlg::OnAddInfoItem()
{
	try {

		CString str;
		if(IDOK == InputBoxLimited(this, "Enter a name for the new item:", str, "",255,false,false,NULL)) {

			str.TrimLeft(" ");
			str.TrimRight(" ");

			if(str == "") {
				AfxMessageBox("You cannot add an item with a blank name.");
				return;
			}

			if(str == "<Remove Item>") {
				AfxMessageBox("'<Remove Item>' is an invalid item name. Please change the name of this item.");
				return;
			}

			if(!IsRecordsetEmpty("SELECT Name FROM EMRInfoT WHERE Name = '%s'",_Q(str))) {
				if(IDNO == MessageBox("There is already a custom record item with this name.\n"
					"It is recommended that you enter unique names for custom record items to minimize confusion.\n\n"
					"Do you still wish to use this name?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}

			//Now, add it.

			//TES 12/15/2006 - PLID 23724 - Updated for new structure
			// (j.armen 2014-01-29 12:06) - PLID 60523 - EMRInfoT.ID is now an identity
			_RecordsetPtr prs = CreateParamRecordset("SET NOCOUNT ON\r\n"
				"DECLARE @nNewMasterID INT\r\n"
				"DECLARE @EMRInfoID INT\r\n\r\n"

				"INSERT INTO EmrInfoMasterT (ActiveEmrInfoID) VALUES (NULL); "
				"SET @nNewMasterID = convert(int, SCOPE_IDENTITY())\r\n\r\n"

				"INSERT INTO EMRInfoT (Name, DataType, LongForm, OnePerEmn, EmrInfoMasterID) "
				"	VALUES ({STRING}, 1, '<Data>', 0, @nNewMasterID)\r\n"
				"SET @EMRInfoID = convert(int, SCOPE_IDENTITY())\r\n\r\n"
				
				"UPDATE EmrInfoMasterT SET ActiveEmrInfoID = @EMRInfoID WHERE ID = @nNewMasterID\r\n"
				
				"SET NOCOUNT OFF\r\n"
				"SELECT @EMRInfoID AS EMRInfoID\r\n"
				, str);

			long nID = AdoFldLong(prs, "EMRInfoID");

			ExecuteParamSql("INSERT INTO EmrInfoCategoryT(EmrInfoID, EmrCategoryID) VALUES ({INT},{INT})", nID,VarLong(m_pCategoryList->GetValue(0,0)));

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiEMRItemCreated, nID, "", str, aepMedium, aetCreated);

			IRowSettingsPtr pRow = m_pItemList->GetRow(-1);
			pRow->PutValue(0, nID);
			pRow->PutValue(1, _bstr_t(str));
			m_pItemList->AddRow(pRow);
			m_pItemList->SetSelByColumn(0, nID);
			Load(nID);
			GetDlgItem(IDC_DELETE_INFO_ITEM)->EnableWindow();
			m_pCategoryList->PutDropDownState(TRUE);
			
		}

	} NxCatchAll("Error in adding new custom record item.");
}

void CCustomRecordSetupDlg::OnDeleteInfoItem()
{
	try {
		if(m_pItemList->CurSel == -1) {
			return;
		}

		long nID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		if(ReturnsRecords("SELECT ID FROM EMRDetailsT WHERE EMRInfoID = %li", nID)) {
			MsgBox("This item is already in use by patients.  It cannot be deleted.");
			return;
		}

		//TES 12/5/2006 - PLID 23724 - Copies are no longer dependent in the new structure.
		/*if(ReturnsRecords("SELECT ID FROM EMRInfoT WHERE CopiedFromInfoID = %li", nID)) {
			MsgBox("This item is referenced by a newer version of this item.  It cannot be deleted.");
			return;
		}*/

		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently delete this item?")) {
			return;
		}

		//Shouldn't be in any of these tables, but whatever.
		CString strSql = BeginSqlBatch();
		
		// (z.manning 2013-03-11 13:37) - PLID 55554 - Moved the code to deleting EMR data records to its own function
		CSqlFragment sqlDeleteData;
		GetDeleteEmrDataSql(CSqlFragment("SELECT ID FROM EmrDataT WHERE EmrInfoID = {INT}", nID), sqlDeleteData);
		strSql += sqlDeleteData.Flatten();
		AddStatementToSqlBatch(strSql, "DELETE FROM EmrInfoCategoryT WHERE EmrInfoID = %li", nID);
		AddStatementToSqlBatch(strSql, "DELETE FROM ProcedureToEMRInfoT WHERE EMRInfoID = %li", nID);
		AddStatementToSqlBatch(strSql, "DELETE FROM DiagCodeToEMRInfoT WHERE EMRInfoID = %li", nID);
		//TES 5/6/2008 - PLID 22843 - We also need to clear out EmrInfoDefaultsT
		AddStatementToSqlBatch(strSql, "DELETE FROM EmrInfoDefaultsT WHERE EmrInfoID = %li", nID);
		//TES 9/8/2005 - They may have stored exports that reference this item.
		AddStatementToSqlBatch(strSql, "DELETE FROM ExportFieldsT WHERE FieldID = 480 AND DynamicID = %li", nID);
		// (a.walling 2008-02-20 10:53) - PLID 29021 - Clear out EMRInfoMasterT references when deleting a custom record item
		AddStatementToSqlBatch(strSql, 
			"UPDATE EMRInfoMasterT SET ActiveEMRInfoID = "
				"(SELECT TOP 1 ID FROM EMRInfoT WHERE ID <> %li AND EMRInfoMasterID IN "
					"(SELECT EMRInfoMasterT.ID FROM EMRInfoMasterT WHERE ActiveEMRInfoID = %li) "
				"ORDER BY ID DESC) "
			"WHERE ActiveEMRInfoID = %li", nID, nID, nID);
		// (z.manning 2011-10-24 12:50) - PLID 46082 - Handle stamp exclusions
		AddStatementToSqlBatch(strSql, "DELETE FROM EmrInfoStampExclusionsT WHERE EmrInfoMasterID IN (SELECT ID FROM EMRInfoMasterT WHERE ActiveEMRInfoID IS NULL)", nID);

		AddStatementToSqlBatch(strSql, "DELETE FROM EMRInfoT WHERE ID = %li", nID);
		// (j.jones 2008-10-22 17:32) - PLID 31692 - delete any EMR Analysis details that reference the master item,
		// but do not delete their master records, even if there are no details left in them
		AddStatementToSqlBatch(strSql, "DELETE FROM EMRAnalysisConfigDetailsT WHERE EMRInfoMasterID IN (SELECT ID FROM EMRInfoMasterT WHERE ActiveEMRInfoID IS NULL)");
		AddStatementToSqlBatch(strSql, "DELETE FROM EMRInfoMasterT WHERE ActiveEMRInfoID IS NULL");

		ExecuteSqlBatch(strSql);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRItemDeleted, nID, "", VarString(m_pItemList->GetValue(m_pItemList->CurSel, 1)), aepHigh, aetDeleted);

		m_pItemList->RemoveRow(m_pItemList->CurSel);
		m_pItemList->CurSel = 0;
		if(m_pItemList->GetRowCount() <= 0) {
			// there are no items left, disable the delete button
			GetDlgItem(IDC_DELETE_INFO_ITEM)->EnableWindow(FALSE);
		}
		else{
			Load(VarLong(m_pItemList->GetValue(0, 0)));
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnDeleteInfoItem()");
}

void CCustomRecordSetupDlg::OnSelChosenProcedure(long nRow)
{
	try {
		if(nRow == -1) return;

		long nProcedureID = VarLong(m_pProcedureDropdown->GetValue(nRow, 0));
		long nEMRID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		if(!ReturnsRecords("SELECT ProcedureID FROM ProcedureToEMRInfoT WHERE ProcedureID = %li ANd EMRInfoID = %li", nProcedureID, nEMRID)) {
			ExecuteSql("INSERT INTO ProcedureToEMRInfoT (ProcedureID, EMRInfoID, Required) "
				"VALUES (%li, %li, 0)", nProcedureID, nEMRID);
			IRowSettingsPtr pRow = m_pProcedureList->GetRow(-1);
			pRow->PutValue(0, nProcedureID);
			pRow->PutValue(1, m_pProcedureDropdown->GetValue(nRow, 1));
			pRow->PutValue(2, false);
			m_pProcedureList->AddRow(pRow);
		}

		//now clear out the selection
		m_pProcedureDropdown->CurSel = -1;

	}NxCatchAll("Error in CCustomRecordSetupDlg::OnSelChosenProcedure()");
}

void CCustomRecordSetupDlg::OnRButtonUpEMRProcedureList(long nRow, short nCol, long x, long y, long nFlags)
{
	if(nRow != -1) {
		m_pProcedureList->CurSel = nRow;
		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_DELETE_PROCEDURE, "Delete");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}

void CCustomRecordSetupDlg::OnDeleteProcedure()
{
	try {
		if(m_pProcedureList->CurSel != -1 && m_pItemList->CurSel != -1) {
			ExecuteSql("DELETE FROM ProcedureToEMRInfoT WHERE EMRInfoID = %li AND ProcedureID = %li", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)), VarLong(m_pProcedureList->GetValue(m_pProcedureList->CurSel, 0)));
			m_pProcedureList->RemoveRow(m_pProcedureList->CurSel);
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnDeleteProcedure()");
}

void CCustomRecordSetupDlg::OnEditingFinishedEMRProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		if(nRow == -1) return;

		if(bCommit) {
			switch(nCol) {
			case 2:
				ExecuteSql("UPDATE ProcedureToEMRInfoT SET Required = %li WHERE ProcedureID = %li AND EMRInfoID = %li", VarBool(varNewValue) ? 1 : 0, VarLong(m_pProcedureList->GetValue(nRow, 0)), VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)));
				break;
			}
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnEditingFinishedEMRProcedureList()");

}

void CCustomRecordSetupDlg::OnSelChosenDiagDropdown(long nRow)
{
	try {
		if(nRow == -1) return;

		long nDiagID = VarLong(m_pDiagDropdown->GetValue(nRow, 0));
		long nEMRID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		if(!ReturnsRecords("SELECT DiagCodeID FROM DiagCodeToEMRInfoT WHERE DiagCodeID = %li ANd EMRInfoID = %li", nDiagID, nEMRID)) {
			ExecuteSql("INSERT INTO DiagCodeToEMRInfoT (DiagCodeID, EMRInfoID, Required) "
				"VALUES (%li, %li, 0)", nDiagID, nEMRID);
			IRowSettingsPtr pRow = m_pDiagList->GetRow(-1);
			pRow->PutValue(0, nDiagID);
			pRow->PutValue(1, m_pDiagDropdown->GetValue(nRow, 1));
			pRow->PutValue(2, m_pDiagDropdown->GetValue(nRow, 2));
			pRow->PutValue(3, false);
			m_pDiagList->AddRow(pRow);
		}

		//now clear out the selection
		m_pDiagDropdown->CurSel = -1;

	}NxCatchAll("Error in CCustomRecordSetupDlg::OnSelChosenDiag()");
}

void CCustomRecordSetupDlg::OnRButtonUpDiagList(long nRow, short nCol, long x, long y, long nFlags)
{
	if(nRow != -1) {
		m_pDiagList->CurSel = nRow;
		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_DELETE_DIAG, "Delete");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}

void CCustomRecordSetupDlg::OnDeleteDiag()
{
	try {
		if(m_pDiagList->CurSel != -1 && m_pItemList->CurSel != -1) {
			ExecuteSql("DELETE FROM DiagCodeToEMRInfoT WHERE EMRInfoID = %li AND DiagCodeID = %li", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)), VarLong(m_pDiagList->GetValue(m_pDiagList->CurSel, 0)));
			m_pDiagList->RemoveRow(m_pDiagList->CurSel);
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnDeleteDiag()");
}

void CCustomRecordSetupDlg::OnEditingFinishedDiagList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		if(nRow == -1) return;

		if(bCommit) {
			switch(nCol) {
			case 3:
				ExecuteSql("UPDATE DiagCodeToEMRInfoT SET Required = %li WHERE DiagCodeID = %li AND EMRInfoID = %li", VarBool(varNewValue) ? 1 : 0, VarLong(m_pDiagList->GetValue(nRow, 0)), VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)));
				break;
			}
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnEditingFinishedDiagList()");
}

void CCustomRecordSetupDlg::OnKillfocusItemName()
{
	try {
		if(m_pItemList->CurSel == -1) {
			return;
		}

		long nID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		CString strOld = VarString(m_pItemList->GetValue(m_pItemList->CurSel, 1));
		CString strNew;
		GetDlgItemText(IDC_ITEM_NAME, strNew);
		strNew.TrimLeft();
		strNew.TrimRight();
		if(strOld != strNew) {
			if(strNew == "") {
				MsgBox("You cannot enter a blank name for an custom record item.");
				SetDlgItemText(IDC_ITEM_NAME, strOld);
				return;
			}

			//TES 8/7/2007 - PLID 26987 - Don't let them rename it if it's been used.
			if(ReturnsRecords("SELECT EmrInfoID FROM EmrDetailsT WHERE EmrInfoID = %li", nID)) {
				MsgBox("You cannot rename an item that has been used on a patient's record.");
				SetDlgItemText(IDC_ITEM_NAME, strOld);
				return;
			}

			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to change the name of this item from %s to %s?", strOld, strNew)) {
				SetDlgItemText(IDC_ITEM_NAME, strOld);
				return;
			}

			//TES 2003-12-29: This is weird looking where clause, but it is because our CString comparison is case-sensitive,
			//but SQL's comparison is not case sensitive.  So if they're renaming something only by case, we don't
			//want to warn them if there are no other items with that name.
			if(ReturnsRecords("SELECT ID FROM EMRInfoT WHERE Name = '%s' AND Name <> '%s'", _Q(strNew), _Q(strOld))) {
				if(IDNO == MessageBox("There is already a custom record item with this name.\n"
					"It is recommended that you enter unique names for EMR items to minimize confusion.\n\n"
					"Do you still wish to use this name?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}

			ExecuteSql("UPDATE EMRInfoT SET Name = '%s' WHERE ID = %li", _Q(strNew), nID);
			IRowSettingsPtr pRow = m_pItemList->GetRow(m_pItemList->CurSel);
			pRow->PutValue(1, _bstr_t(strNew));
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnKillFocusItemName()");
}

void CCustomRecordSetupDlg::OnSelChosenEMRCategories(long nRow)
{
	try {
		if(m_pItemList->CurSel != -1) {
			ExecuteSql("DELETE FROM EmrInfoCategoryT WHERE EmrInfoID = %li", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)));
			ExecuteSql("INSERT INTO EmrInfoCategoryT (EmrInfoID, EmrCategoryID) VALUES (%li, %li)", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)), VarLong(m_pCategoryList->GetValue(nRow, 0)));
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnSelChosenEMRCategories()");
}

void CCustomRecordSetupDlg::OnSelChangingEMRCategories(long FAR* nNewSel)
{
	try {
		if(*nNewSel == -1) {
			if(m_pItemList->CurSel != -1) {
				_RecordsetPtr rsCategory = CreateRecordset("SELECT EmrCategoryID FROM EMRInfoCategoryT WHERE EmrInfoID = %li", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)));
				*nNewSel = m_pCategoryList->FindByColumn(0, AdoFldLong(rsCategory, "EmrCategoryID"), 0, false);
			}
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnSelChangingEMRCategories()");
}

void CCustomRecordSetupDlg::OnKillfocusSentence()
{
	try {
		if(m_pItemList->CurSel == -1) {
			return;
		}

		long nID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		_RecordsetPtr rsCurrentSentence = CreateRecordset("SELECT LongForm FROM EMRInfoT WHERE ID = %li", nID);
		CString strOld = AdoFldString(rsCurrentSentence, "LongForm");
		CString strNew;
		GetDlgItemText(IDC_SENTENCE, strNew);
		strNew.TrimLeft();
		strNew.TrimRight();
		if(strOld != strNew) {
			if(strNew == "") {
				if(IDNO == MsgBox(MB_YESNO, "Are you sure you wish to make this field blank?\n"
					"If you do, it will not be included when Chart Notes and other documents are generated from custom records which include this item.")) {
					SetDlgItemText(IDC_SENTENCE, strOld);
					return;
				}
			}

			ExecuteSql("UPDATE EMRInfoT SET LongForm = '%s' WHERE ID = %li", _Q(strNew), nID);
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnKillFocusSentence()");
}

void CCustomRecordSetupDlg::OnAddDataItem()
{
	try {
		if(m_pItemList->CurSel == -1) return;

		long nID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));

		CString str;
		if(IDOK == InputBoxLimited(this, "Enter a name for the new item:", str, "",2000,false,false,NULL)) {

			str.TrimLeft(" ");
			str.TrimRight(" ");

			if(str == "") {
				AfxMessageBox("You cannot add an item with a blank name.");
				return;
			}

			if(ReturnsRecords("SELECT ID FROM EMRDataT WHERE Data = '%s' AND EMRInfoID = %li",_Q(str), nID)) {
				MsgBox("This custom record item already has a list option with this name.");
				return;
			}

			//Now, add it.			

			// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
			long nEMRDataGroupID = NewNumber("EMRDataGroupsT", "ID");
			ExecuteSql("INSERT INTO EMRDataGroupsT (ID) VALUES (%li)", nEMRDataGroupID);

			long nDataID = NewNumber("EMRDataT", "ID");

			//TES 12/15/2006 - PLID 23766 - Updated for new structure.
			ExecuteSql("INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, EmrDataGroupID) "
				"SELECT %li, %li, '%s', COALESCE(MAX(EmrDataT.ID),0)+1, %li "
				"FROM EmrDataT WHERE EmrInfoID = %li", 
				nDataID, nID, _Q(str), nEMRDataGroupID, nID);

			IRowSettingsPtr pRow = m_pDataList->GetRow(-1);
			pRow->PutValue(0, nDataID);
			pRow->PutValue(1, _bstr_t(str));
			pRow->PutValue(2, false);
			m_pDataList->AddRow(pRow);
			m_pDataList->SetSelByColumn(0, nDataID);
			OnSelChangedEMRDataList(m_pDataList->FindByColumn(0, nDataID, 0, false));
		}

	} NxCatchAll("Error in CCustomRecordSetupDlg::OnAddDataItem()");
}

void CCustomRecordSetupDlg::OnDeleteDataItem()
{
	try {
		if(m_pItemList->CurSel == -1 || m_pDataList->CurSel == -1) return;

		long nDataID = VarLong(m_pDataList->GetValue(m_pDataList->CurSel, 0));
		if(ReturnsRecords("SELECT EmrSelectT.ID FROM EMRSelectT WHERE EMRDataID = %li", nDataID)) {
			MsgBox("This data item has been selected on existing custom records.  It cannot be deleted.");
			return;
		}

		if(ReturnsRecords("SELECT ID FROM EMRDataT WHERE CopiedFromDataID = %li", nDataID)) {
			MsgBox("This data item is referenced by a newer version of this item.  It cannot be deleted.");
			return;
		}

		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently delete this item?")) {
			return;
		}

		// (z.manning 2013-03-11 12:51) - PLID 55554 - Use the shared EMR data deletion code.
		CSqlFragment sqlDelete;
		GetDeleteEmrDataSql(CSqlFragment("{INT}", nDataID), sqlDelete);
		ExecuteParamSql(sqlDelete);
		
		m_pDataList->RemoveRow(m_pDataList->CurSel);
		OnSelChangedEMRDataList(m_pDataList->CurSel);		
	}
	NxCatchAll("Error in CCustomRecordSetupDlg::OnDeleteDataItem()");
}

void CCustomRecordSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (z.manning, 5/19/2006, PLID 20726) - We may still have focus on a field that has been changed,
	// so let's kill it's focus to ensure that no changes are lost.
	// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
	CheckFocus();

	if(m_procedureNameChecker.Changed()) {
		m_pProcedureDropdown->Requery();
	}

	if(m_ICD9Checker.Changed()) {
		m_pDiagDropdown->Requery();
	}

	if(m_pItemList->CurSel != -1) {
		Load(m_pItemList->GetValue(m_pItemList->CurSel, 0));
	}
}

void CCustomRecordSetupDlg::OnEditingFinishedEMRDataList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	if(nRow == -1 || !bCommit || m_pItemList->CurSel == -1) return;

	try {
		switch(nCol) {
		case 2: //Default
			if(VarBool(varNewValue)) {
				//Uncheck all other rows.
				for(int i = 0; i < m_pDataList->GetRowCount(); i++) {
					if(i != nRow) {
						IRowSettingsPtr pRow = m_pDataList->GetRow(i);
						pRow->PutValue(2, false);
					}
				}
				//Commit change.
				long nInfoID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel,0));
				ExecuteSql("DELETE FROM EmrInfoDefaultsT WHERE EmrInfoID = %li", nInfoID);
				ExecuteSql("INSERT INTO EmrInfoDefaultsT (EmrInfoID, EmrDataID) VALUES (%li, %li)", nInfoID, VarLong(m_pDataList->GetValue(nRow, 0)));
			}
			else {
				//Commit change.
				ExecuteSql("DELETE FROM EmrInfoDefaultsT WHERE EmrInfoID = %li", VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0)));
			}
			break;
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnEditingFinishedEMRDataList()");

}

void CCustomRecordSetupDlg::OnKillfocusDataValue()
{
	try {
		if(m_pItemList->CurSel == -1 || m_pDataList->CurSel == -1) {
			return;
		}

		long nEMRID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		CString strOld = VarString(m_pDataList->GetValue(m_pDataList->CurSel, 1));
		CString strNew;
		GetDlgItemText(IDC_DATA_VALUE, strNew);
		strNew.TrimLeft();
		strNew.TrimRight();
		if(strOld != strNew) {
			if(strNew == "") {
				MsgBox("You cannot enter a blank name for an custom record list option.");
				SetDlgItemText(IDC_DATA_VALUE, strOld);
				return;
			}

			//TES 8/7/2007 - PLID 26987 - Don't let them rename it if it's in use.
			long nDataID = VarLong(m_pDataList->GetValue(m_pDataList->CurSel, 0));
			if(ReturnsRecords("SELECT EmrDataID FROM EmrSelectT WHERE EmrDataID = %li", nDataID)) {
				MsgBox("You cannot rename a list option that has been selected on a patient's record.");
				SetDlgItemText(IDC_DATA_VALUE, strOld);
				return;
			}

			if(ReturnsRecords("SELECT ID FROM EMRDataT WHERE Data = '%s' AND EMRInfoID = %li AND ID <> %li",_Q(strNew), nEMRID, nDataID)) {
				MsgBox("This EMR item already has a list option with this name.");
				SetDlgItemText(IDC_DATA_VALUE, strOld);
				return;
			}

			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to change the name of this item from %s to %s?", strOld, strNew)) {
				SetDlgItemText(IDC_DATA_VALUE, strOld);
				return;
			}

			ExecuteSql("UPDATE EMRDataT SET Data = '%s' WHERE ID = %li", _Q(strNew), nDataID);
			IRowSettingsPtr pRow = m_pDataList->GetRow(m_pDataList->CurSel);
			pRow->PutValue(1, _bstr_t(strNew));
		}
	}NxCatchAll("Error in CCustomRecordSetupDlg::OnKillFocusDataValue()");
}

void CCustomRecordSetupDlg::OnItemAction()
{
	if(m_pItemList->CurSel != -1) {
		CCustomRecordActionDlg dlg(this);
		dlg.m_SourceType = eaoEmrItem;
		dlg.m_nSourceID = VarLong(m_pItemList->GetValue(m_pItemList->CurSel, 0));
		dlg.DoModal();
	}
}

void CCustomRecordSetupDlg::OnDataAction()
{
	if(m_pDataList->CurSel != -1) {
		CCustomRecordActionDlg dlg(this);
		dlg.m_SourceType = eaoEmrDataItem;
		dlg.m_nSourceID = VarLong(m_pDataList->GetValue(m_pDataList->CurSel, 0));
		dlg.DoModal();
	}
}

void CCustomRecordSetupDlg::OnEditEMRCategories()
{
	try {
		CEmrCategoriesDlg dlg(this);
		dlg.DoModal();
		//PLID 15072 - fixed problem where you got error if you didn't have a category selected
		if (m_pCategoryList->CurSel != -1) {
			long nCurrentID = VarLong(m_pCategoryList->GetValue(m_pCategoryList->CurSel, 0));
			m_pCategoryList->Requery();
			m_pCategoryList->SetSelByColumn(0, nCurrentID);
		}
		else {
			m_pCategoryList->Requery();
		}

	}NxCatchAll("Error in CCustomRecordSetupDlg::OnEditEMRCategories()");
}

void CCustomRecordSetupDlg::OnSetupHeader()
{
	CEmrHeaderDlg dlg(this);
	dlg.DoModal();
}

void CCustomRecordSetupDlg::OnOfficeVisit()
{
	COfficeVisitConfigDlg dlg(this);
	dlg.DoModal();
}

LRESULT CCustomRecordSetupDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::AptPurposeT:
			case NetUtils::DiagCodes: {
				try {
					UpdateView();
				} NxCatchAll("Error in CCustomRecordSetupDlg::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CCustomRecordSetupDlg::OnTableChanged");

	return 0;
}
