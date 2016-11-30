// ERemitImportFilteringDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ERemitImportFilteringDlg.h"

// CERemitImportFilteringDlg dialog

// (j.jones 2010-02-08 15:00) - PLID 37174 - created

enum ERemitIDListColumns {

	elcID = 0,
	elcFilteredID,
};

using namespace ADODB;
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CERemitImportFilteringDlg, CNxDialog)

CERemitImportFilteringDlg::CERemitImportFilteringDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CERemitImportFilteringDlg::IDD, pParent)
{

}

CERemitImportFilteringDlg::~CERemitImportFilteringDlg()
{
}

void CERemitImportFilteringDlg::DoDataExchange(CDataExchange* pDX)
{	
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_EREMIT_IMPORT_FILTER_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADD_EOB_ID, m_btnAddEOBID);
	DDX_Control(pDX, IDC_BTN_DELETE_EOB_ID, m_btnDeleteEOBID);
	DDX_Control(pDX, IDC_BTN_ADD_CLAIM_ID, m_btnAddClaimID);
	DDX_Control(pDX, IDC_BTN_DELETE_CLAIM_ID, m_btnDeleteClaimID);
}

BEGIN_MESSAGE_MAP(CERemitImportFilteringDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_EREMIT_IMPORT_FILTER_CLOSE, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_ADD_EOB_ID, OnBtnAddEobID)
	ON_BN_CLICKED(IDC_BTN_DELETE_EOB_ID, OnBtnDeleteEobID)
	ON_BN_CLICKED(IDC_BTN_ADD_CLAIM_ID, OnBtnAddClaimID)
	ON_BN_CLICKED(IDC_BTN_DELETE_CLAIM_ID, OnBtnDeleteClaimID)
END_MESSAGE_MAP()


BOOL CERemitImportFilteringDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddEOBID.AutoSet(NXB_NEW);
		m_btnDeleteEOBID.AutoSet(NXB_DELETE);
		// (j.jones 2010-02-09 13:44) - PLID 37254 - added claim filtering
		m_btnAddClaimID.AutoSet(NXB_NEW);
		m_btnDeleteClaimID.AutoSet(NXB_DELETE);

		m_EOBIDList = BindNxDataList2Ctrl(IDC_EOB_IDS_LIST, true);
		// (j.jones 2010-02-09 13:44) - PLID 37254 - added claim filtering
		m_ClaimIDList = BindNxDataList2Ctrl(IDC_CLAIM_IDS_LIST, true);

	}NxCatchAll("Error in CERemitImportFilteringDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CERemitImportFilteringDlg::OnBtnClose()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CERemitImportFilteringDlg::OnBtnAddEobID()
{
	try {

		CString strNewID;

		if(InputBox(this, "Enter an EOB Payee ID to filter on:", strNewID, "") == IDOK) {

			if(strNewID.GetLength() > 255) {
				AfxMessageBox("The ID you entered is too long. Please enter an ID no longer than 255 characters.");
				return;
			}

			strNewID.TrimLeft();
			strNewID.TrimRight();

			if(strNewID.IsEmpty()) {
				AfxMessageBox("You may not enter a blank ID.");
				return;
			}

			//verify it does not already exist (case insensitive)
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM ERemitEOBFilteredIDsT WHERE FilteredID = {STRING}", strNewID);
			if(!rs->eof) {
				AfxMessageBox("This ID already exists in the list.");
				return;
			}
			rs->Close();

			//save immediately
			rs = CreateParamRecordset("SET NOCOUNT ON "
				"INSERT INTO ERemitEOBFilteredIDsT (FilteredID) VALUES ({STRING}) "
				"SET NOCOUNT OFF "
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID", strNewID);

			if(!rs->eof) {
				IRowSettingsPtr pRow = m_EOBIDList->GetNewRow();
				pRow->PutValue(elcID, (long)AdoFldLong(rs, "NewID"));
				pRow->PutValue(elcFilteredID, (LPCTSTR)strNewID);
				m_EOBIDList->AddRowSorted(pRow, NULL);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CERemitImportFilteringDlg::OnBtnDeleteEobID()
{
	try {

		IRowSettingsPtr pRow = m_EOBIDList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("Please select an ID before deleting.");
			return;
		}

		if(IDNO == MessageBox("Are you sure you wish to remove this ID from the EOB filter list?",
			"Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		long nID = VarLong(pRow->GetValue(elcID));
		
		ExecuteParamSql("DELETE FROM ERemitEOBFilteredIDsT WHERE ID = {INT}", nID);
		
		m_EOBIDList->RemoveRow(pRow);

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CERemitImportFilteringDlg, CNxDialog)
	ON_EVENT(CERemitImportFilteringDlg, IDC_EOB_IDS_LIST, 6, OnRButtonDownEobIDsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CERemitImportFilteringDlg, IDC_EOB_IDS_LIST, 9, OnEditingFinishingEobIDsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CERemitImportFilteringDlg, IDC_EOB_IDS_LIST, 10, OnEditingFinishedEobIDsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CERemitImportFilteringDlg, IDC_CLAIM_IDS_LIST, 6, OnRButtonDownClaimIDsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CERemitImportFilteringDlg, IDC_CLAIM_IDS_LIST, 9, OnEditingFinishingClaimIDsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CERemitImportFilteringDlg, IDC_CLAIM_IDS_LIST, 10, OnEditingFinishedClaimIDsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CERemitImportFilteringDlg::OnRButtonDownEobIDsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_EOBIDList->PutCurSel(pRow);

		//add an ability to remove the row
		enum {
			eRemoveFromList = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveFromList, "&Remove From List");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveFromList) {

			OnBtnDeleteEobID();
		}

	}NxCatchAll(__FUNCTION__);
}

void CERemitImportFilteringDlg::OnEditingFinishingEobIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == elcFilteredID) {

			CString strNewID = strUserEntered;

			if(strNewID.GetLength() > 255) {
				AfxMessageBox("The ID you entered is too long. Please enter an ID no longer than 255 characters.");
				*pbCommit = FALSE;
				return;
			}

			strNewID.TrimLeft();
			strNewID.TrimRight();

			if(strNewID.IsEmpty()) {
				AfxMessageBox("You may not enter a blank ID.");
				*pbCommit = FALSE;
				return;
			}

			long nID = VarLong(pRow->GetValue(elcID));

			//verify it does not already exist (case insensitive)
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM ERemitEOBFilteredIDsT WHERE ID <> {INT} AND FilteredID = {STRING}", nID, strNewID);
			if(!rs->eof) {
				AfxMessageBox("This ID already exists in the list.");
				*pbCommit = FALSE;
				return;
			}
			rs->Close();

			_variant_t varNew(strNewID);
			*pvarNewValue = varNew.Detach();
		}

	}NxCatchAll(__FUNCTION__);
}

void CERemitImportFilteringDlg::OnEditingFinishedEobIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		if(nCol == elcFilteredID) {

			long nID = VarLong(pRow->GetValue(elcID));
			ExecuteParamSql("UPDATE ERemitEOBFilteredIDsT SET FilteredID = {STRING} WHERE ID = {INT}", VarString(varNewValue, ""), nID);	
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 14:00) - PLID 37254 - added claim filtering
void CERemitImportFilteringDlg::OnBtnAddClaimID()
{
	try {

		CString strNewID;

		if(InputBox(this, "Enter an Claim Rendering Provider ID to filter on:", strNewID, "") == IDOK) {

			if(strNewID.GetLength() > 255) {
				AfxMessageBox("The ID you entered is too long. Please enter an ID no longer than 255 characters.");
				return;
			}

			strNewID.TrimLeft();
			strNewID.TrimRight();

			if(strNewID.IsEmpty()) {
				AfxMessageBox("You may not enter a blank ID.");
				return;
			}

			//verify it does not already exist (case insensitive)
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM ERemitClaimFilteredIDsT WHERE FilteredID = {STRING}", strNewID);
			if(!rs->eof) {
				AfxMessageBox("This ID already exists in the list.");
				return;
			}
			rs->Close();

			//save immediately
			rs = CreateParamRecordset("SET NOCOUNT ON "
				"INSERT INTO ERemitClaimFilteredIDsT (FilteredID) VALUES ({STRING}) "
				"SET NOCOUNT OFF "
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID", strNewID);

			if(!rs->eof) {
				IRowSettingsPtr pRow = m_ClaimIDList->GetNewRow();
				pRow->PutValue(elcID, (long)AdoFldLong(rs, "NewID"));
				pRow->PutValue(elcFilteredID, (LPCTSTR)strNewID);
				m_ClaimIDList->AddRowSorted(pRow, NULL);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 14:00) - PLID 37254 - added claim filtering
void CERemitImportFilteringDlg::OnBtnDeleteClaimID()
{
	try {

		IRowSettingsPtr pRow = m_ClaimIDList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("Please select an ID before deleting.");
			return;
		}

		if(IDNO == MessageBox("Are you sure you wish to remove this ID from the Claim filter list?",
			"Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		long nID = VarLong(pRow->GetValue(elcID));
		
		ExecuteParamSql("DELETE FROM ERemitClaimFilteredIDsT WHERE ID = {INT}", nID);
		
		m_ClaimIDList->RemoveRow(pRow);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 14:00) - PLID 37254 - added claim filtering
void CERemitImportFilteringDlg::OnRButtonDownClaimIDsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_ClaimIDList->PutCurSel(pRow);

		//add an ability to remove the row
		enum {
			eRemoveFromList = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveFromList, "&Remove From List");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveFromList) {

			OnBtnDeleteClaimID();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 14:00) - PLID 37254 - added claim filtering
void CERemitImportFilteringDlg::OnEditingFinishingClaimIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == elcFilteredID) {

			CString strNewID = strUserEntered;

			if(strNewID.GetLength() > 255) {
				AfxMessageBox("The ID you entered is too long. Please enter an ID no longer than 255 characters.");
				*pbCommit = FALSE;
				return;
			}

			strNewID.TrimLeft();
			strNewID.TrimRight();

			if(strNewID.IsEmpty()) {
				AfxMessageBox("You may not enter a blank ID.");
				*pbCommit = FALSE;
				return;
			}

			long nID = VarLong(pRow->GetValue(elcID));

			//verify it does not already exist (case insensitive)
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM ERemitClaimFilteredIDsT WHERE ID <> {INT} AND FilteredID = {STRING}", nID, strNewID);
			if(!rs->eof) {
				AfxMessageBox("This ID already exists in the list.");
				*pbCommit = FALSE;
				return;
			}
			rs->Close();

			_variant_t varNew(strNewID);
			*pvarNewValue = varNew.Detach();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-02-09 14:00) - PLID 37254 - added claim filtering
void CERemitImportFilteringDlg::OnEditingFinishedClaimIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		if(nCol == elcFilteredID) {

			long nID = VarLong(pRow->GetValue(elcID));
			ExecuteParamSql("UPDATE ERemitClaimFilteredIDsT SET FilteredID = {STRING} WHERE ID = {INT}", VarString(varNewValue, ""), nID);	
		}

	}NxCatchAll(__FUNCTION__);
}
