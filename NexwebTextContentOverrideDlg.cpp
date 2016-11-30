// NexwebTextContentOverrideDlg.cpp : implementation file
//

// (d.singleton 2013-02-15 09:12) - PLID 55199 new dialog for the text content setting values

#include "stdafx.h"
#include "Practice.h"
#include "NexwebTextContentOverrideDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CNexwebTextContentOverrideDlg dialog

IMPLEMENT_DYNAMIC(CNexwebTextContentOverrideDlg, CNxDialog)

enum NexWebContentDetailColumns
{
	cdcID = 0,
	cdcText,
	cdcOrder,
};

CNexwebTextContentOverrideDlg::CNexwebTextContentOverrideDlg(long nSubdomainID, CString strMasterUID, CString strSettingName, CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexwebTextContentOverrideDlg::IDD, pParent)
{
	m_nSubdomainID = nSubdomainID;
	m_strMasterUID = strMasterUID;
	m_strSettingName = strSettingName;
}

CNexwebTextContentOverrideDlg::~CNexwebTextContentOverrideDlg()
{
}

void CNexwebTextContentOverrideDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_NEW_LINE, m_btnNexWebContentSettingAdd);
	DDX_Control(pDX, IDC_REMOVE_LINE, m_btnNexWebContentSettingRemove);
	DDX_Control(pDX, IDC_MOVE_LINE_UP, m_btnNexWebContentSettingUp);
	DDX_Control(pDX, IDC_MOVE_LINE_DOWN, m_btnNexWebContentSettingDown);
	DDX_Control(pDX, IDC_CONTENT_SETTING_CLOSE, m_btnClose);
}

BOOL CNexwebTextContentOverrideDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//set buttons
		m_btnNexWebContentSettingAdd.AutoSet(NXB_NEW);		
		m_btnNexWebContentSettingRemove.AutoSet(NXB_DELETE);
		m_btnNexWebContentSettingUp.AutoSet(NXB_UP);
		m_btnNexWebContentSettingDown.AutoSet(NXB_DOWN);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_pTextContentSettingDetails = BindNxDataList2Ctrl(IDC_TEXT_CONTENT_SETTINGS, false);

		//set where clause
		CString strWhere;
		strWhere.Format("ContentMasterUID = '%s' AND SubdomainID = %li", m_strMasterUID, m_nSubdomainID);
		m_pTextContentSettingDetails->PutWhereClause(_bstr_t(strWhere));
		m_pTextContentSettingDetails->Requery();

		this->SetWindowTextA("NexWeb Patient Portal Setting - " + m_strSettingName);

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CNexwebTextContentOverrideDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_NEW_LINE, &CNexwebTextContentOverrideDlg::OnBnClickedAddNewLine)
	ON_BN_CLICKED(IDC_REMOVE_LINE, &CNexwebTextContentOverrideDlg::OnBnClickedRemoveLine)
	ON_BN_CLICKED(IDC_MOVE_LINE_UP, &CNexwebTextContentOverrideDlg::OnBnClickedMoveLineUp)
	ON_BN_CLICKED(IDC_MOVE_LINE_DOWN, &CNexwebTextContentOverrideDlg::OnBnClickedMoveLineDown)
	ON_BN_CLICKED(IDC_CONTENT_SETTING_CLOSE, &CNexwebTextContentOverrideDlg::OnBnClickedContentSettingClose)
END_MESSAGE_MAP()


// CNexwebTextContentOverrideDlg message handlers

//(e.lally 2011-05-19) PLID 43333 - Adds a new line entry at the bottom
// (d.singleton 2013-02-14 17:01) - PLID 55199 moved from LinksSetupDlg.cpp
void CNexwebTextContentOverrideDlg::OnBnClickedAddNewLine()
{
	try {
		if(m_strMasterUID.IsEmpty() || m_nSubdomainID == -1){
			return;
		}

		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
			"DECLARE @NewSortOrder INT "
			"SET @NewSortOrder = (SELECT COALESCE(Max(SortOrder), -1) + 1 "
				"FROM NexWebSubdomainContentT "
				"WHERE ContentMasterUID = {STRING} AND SubdomainID = {INT} )\r\n "
			"INSERT INTO NexWebSubdomainContentT(ContentMasterUID, SortOrder, Text, SubdomainID) "
			"VALUES({STRING}, @NewSortOrder, '', {INT} ) "
			"SET NOCOUNT OFF \r\n"
			"SELECT Convert(int, SCOPE_IDENTITY()) AS ID, @NewSortOrder AS SortOrder \r\n" 
			, m_strMasterUID, m_nSubdomainID, m_strMasterUID, m_nSubdomainID);
		if(!rs->eof){
			long nID = AdoFldLong(rs, "ID");
			long nSortOrder = AdoFldLong(rs, "SortOrder");
			
			IRowSettingsPtr pNewRow = m_pTextContentSettingDetails->GetNewRow();
			if(pNewRow != NULL){
				pNewRow->PutValue(cdcID, (long)nID);
				pNewRow->PutValue(cdcText, "");
				pNewRow->PutValue(cdcOrder, (long)nSortOrder);

				m_pTextContentSettingDetails->AddRowAtEnd(pNewRow, NULL);
				m_pTextContentSettingDetails->CurSel = pNewRow;
				m_pTextContentSettingDetails->StartEditing(pNewRow, cdcText);
			}
		}
		else {
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-05-19) PLID 43333 - Removes the current selected line
// (d.singleton 2013-02-14 17:01) - PLID 55199 moved from LinksSetupDlg.cpp
void CNexwebTextContentOverrideDlg::OnBnClickedRemoveLine()
{
	try {
		if(m_strMasterUID.IsEmpty() || m_nSubdomainID == -1){
			return;
		}

		IRowSettingsPtr pCurSel = m_pTextContentSettingDetails->GetCurSel();
		if(pCurSel == NULL){
			AfxMessageBox("You must first select a line to remove from the list.");
			return;
		}

		long nID = VarLong(pCurSel->GetValue(cdcID));
		long nSortOrder = VarLong(pCurSel->GetValue(cdcOrder));
		ExecuteParamSql("SET XACT_ABORT ON \r\n"
			"BEGIN TRAN \r\n"
			"UPDATE NexWebSubdomainContentT SET SortOrder = (SortOrder - 1) "
			"WHERE ContentMasterUID = {STRING} AND SubdomainID = {INT} AND SortOrder > {INT} \r\n"
			""
			"DELETE FROM NexWebSubdomainContentT WHERE ID = {INT} \r\n"
			"COMMIT TRAN \r\n", 
			m_strMasterUID, m_nSubdomainID, nSortOrder,
			nID);
		IRowSettingsPtr pNextRow = pCurSel->GetNextRow();
		while(pNextRow != NULL){
			long nNewSortOrder = VarLong(pNextRow->GetValue(cdcOrder)) - 1;
			pNextRow->PutValue(cdcOrder, (long)nNewSortOrder);
			pNextRow = pNextRow->GetNextRow();
		}
		m_pTextContentSettingDetails->RemoveRow(pCurSel);
		m_pTextContentSettingDetails->CurSel = NULL;
		
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-05-19) PLID 43333 - Moves the current selected line up one
// (d.singleton 2013-02-14 17:01) - PLID 55199 moved from LinksSetupDlg.cpp
void CNexwebTextContentOverrideDlg::OnBnClickedMoveLineUp()
{
	try {
	IRowSettingsPtr pSelRow = m_pTextContentSettingDetails->GetCurSel();
		if(pSelRow == NULL){
			return;
		}

		IRowSettingsPtr pPrevRow = pSelRow->GetPreviousRow();
		if(pPrevRow == NULL){
			//we're already the first row
			return;
		}
		//update the sort order
		long nTopID = VarLong(pPrevRow->GetValue(cdcID));
		long nTopSortOrder = VarLong(pPrevRow->GetValue(cdcOrder));

		long nBottomID = VarLong(pSelRow->GetValue(cdcID));
		long nBottomSortOrder = VarLong(pSelRow->GetValue(cdcOrder));

		ASSERT(nTopSortOrder < nBottomSortOrder);

		ExecuteParamSql("SET XACT_ABORT ON\r\n"
			"BEGIN TRAN\r\n"
			"UPDATE NexWebSubdomainContentT SET SortOrder = {INT} WHERE ID = {INT} \r\n"
			"UPDATE NexWebSubdomainContentT SET SortOrder = {INT} WHERE ID = {INT} \r\n"
			"COMMIT TRAN ",
			nTopSortOrder, nBottomID,
			nBottomSortOrder, nTopID);
		//Update the interface
		pPrevRow->PutValue(cdcOrder, (long)nBottomSortOrder);
		pSelRow->PutValue(cdcOrder, (long)nTopSortOrder);

		IRowSettingsPtr pNewSelRow = m_pTextContentSettingDetails->AddRowBefore(pSelRow, pPrevRow);
		m_pTextContentSettingDetails->CurSel = pNewSelRow;
		m_pTextContentSettingDetails->RemoveRow(pSelRow);

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-05-19) PLID 43333 - Moves the current selected line down one
// (d.singleton 2013-02-14 17:01) - PLID 55199 moved from LinksSetupDlg.cpp
void CNexwebTextContentOverrideDlg::OnBnClickedMoveLineDown()
{
	try {
		IRowSettingsPtr pSelRow = m_pTextContentSettingDetails->GetCurSel();
		if(pSelRow == NULL){
			return;
		}
		IRowSettingsPtr pNextRow = pSelRow->GetNextRow();
		if(pNextRow == NULL){
			return;
		}
		//update the sort order
		long nTopID = VarLong(pSelRow->GetValue(cdcID));
		long nTopSortOrder = VarLong(pSelRow->GetValue(cdcOrder));

		long nBottomID = VarLong(pNextRow->GetValue(cdcID));
		long nBottomSortOrder = VarLong(pNextRow->GetValue(cdcOrder));

		ASSERT(nTopSortOrder < nBottomSortOrder);

		ExecuteParamSql("SET XACT_ABORT ON\r\n"
			"BEGIN TRAN\r\n"
			"UPDATE NexWebSubdomainContentT SET SortOrder = {INT} WHERE ID = {INT} \r\n"
			"UPDATE NexWebSubdomainContentT SET SortOrder = {INT} WHERE ID = {INT} \r\n"
			"COMMIT TRAN ",
			nTopSortOrder, nBottomID,
			nBottomSortOrder, nTopID);
		//Update the interface
		pSelRow->PutValue(cdcOrder, (long)nBottomSortOrder);
		pNextRow->PutValue(cdcOrder, (long)nTopSortOrder);

		m_pTextContentSettingDetails->AddRowBefore(pNextRow, pSelRow);
		m_pTextContentSettingDetails->RemoveRow(pNextRow);

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CNexwebTextContentOverrideDlg, CNxDialog)
	ON_EVENT(CNexwebTextContentOverrideDlg, IDC_TEXT_CONTENT_SETTINGS, 10, CNexwebTextContentOverrideDlg::EditingFinishedTextContentSettings, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CNexwebTextContentOverrideDlg::EditingFinishedTextContentSettings(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if(bCommit == FALSE || lpRow == NULL){
			return;
		}
		switch(nCol) {
			case cdcText:
			{
				//(e.lally 2011-05-19) PLID 43333 - Save the Text if it changed
				CString strOld = VarString(varOldValue, "");
				CString strNew = VarString(varNewValue, "");
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				long nID = VarLong(pRow->GetValue(cdcID));
				if(strOld != strNew){
					ExecuteParamSql("UPDATE NexWebSubdomainContentT SET [Text] = {STRING} WHERE ID = {INT} ", strNew, nID);
				}
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexwebTextContentOverrideDlg::OnBnClickedContentSettingClose()
{
	try {
		CDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
