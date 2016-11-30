// EditCreditCardsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillingRc.h"
#include "EditCreditCardsDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

// (d.thompson 2009-06-29) - PLID 34742 - Removed IGS entirely from the system.
enum CreditCardSetupColumns{
	cscID =0,
	cscCardName,
	cscPAYMENTECHCardType,		// (d.thompson 2009-04-09) - PLID 33935 - Renamed to PaymenTech
	cscInactive,
};



/////////////////////////////////////////////////////////////////////////////
// CEditCreditCardsDlg dialog

//(e.lally 2007-07-11) PLID 26590 - Created a new edit dialog for credit/charge cards

CEditCreditCardsDlg::CEditCreditCardsDlg(CWnd* pParent)
	: CNxDialog(CEditCreditCardsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditCreditCardsDlg)
	//}}AFX_DATA_INIT
}


void CEditCreditCardsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditCreditCardsDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_REMOVE_CARD, m_btnRemove);
	DDX_Control(pDX, IDC_ADD_CARD, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditCreditCardsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditCreditCardsDlg)
	ON_BN_CLICKED(IDC_ADD_CARD, OnAddCard)
	ON_BN_CLICKED(IDC_REMOVE_CARD, OnRemoveCard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditCreditCardsDlg message handlers

BOOL CEditCreditCardsDlg::OnInitDialog() 
{
	try{
		// (c.haag 2008-04-23 14:51) - PLID 29761 - Added icons to CNxIconButtons
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnAdd.AutoSet(NXB_NEW);

		//(e.lally 2007-07-10) PLID 26590 - Created edit dlg for credit/charge cards
		CNxDialog::OnInitDialog();
		
		m_pChargeCards = BindNxDataList2Ctrl(IDC_CREDIT_CARD_SETUP, false);

		// (d.thompson 2009-04-09) - PLID 33935 - The PAYMENTECH versions are obsolete at this point, this column
		//	can never be displayed.  I leave this code so it still operates in the background since there may be
		//	some saved data from the past.
		IColumnSettingsPtr pCol;
		pCol = m_pChargeCards->GetColumn(cscPAYMENTECHCardType);
		if(pCol){
			CString strComboSource;
			strComboSource.Format("%li;Credit;%li;Debit (US);%li;Interac (CA);%li;American Express;%li;Visa",
				pctCredit, pctUSDebit, pctInterac, pctAmex, pctVisa);
			pCol->ComboSource = _bstr_t(strComboSource);
		}

		m_pChargeCards->Requery();
		
		return TRUE;  
	}NxCatchAll("Error opening Credit Card editor")
	return FALSE;
}

BEGIN_EVENTSINK_MAP(CEditCreditCardsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditCreditCardsDlg)
	ON_EVENT(CEditCreditCardsDlg, IDC_CREDIT_CARD_SETUP, 9 /* EditingFinishing */, OnEditingFinishingCreditCardSetup, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditCreditCardsDlg::OnEditingFinishingCreditCardSetup(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try{
		IRowSettingsPtr pRow(lpRow);
		long nCardID = VarLong(pRow->GetValue(cscID), -1);

		//Make sure if they change the card name, that it isn't blank
		if(nCol == (short)cscCardName){
			CString strNewValue = VarString(*pvarNewValue, "");
			CString strOldValue = VarString(varOldValue, "");
			//Check if the user is committing a blank new value, or cancelling to a blank old value
			if((strNewValue.IsEmpty() && *pbCommit != FALSE) ||
				(strOldValue.IsEmpty() && *pbCommit == FALSE)){

				MessageBox("The Card Name cannot be blank.", NULL, MB_OK|MB_ICONINFORMATION);
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
			//Only allow 50 characters for the new name
			const long MAX_CARD_NAME_LENGTH = 50;
			if(*pbCommit != FALSE && strNewValue.GetLength() > MAX_CARD_NAME_LENGTH){
				CString strMessage;
				strMessage.Format("The Card Name has exceded the maximum length (%li) by %li characters."
					" Please shorten the Card Name before continuing.",
					MAX_CARD_NAME_LENGTH, strNewValue.GetLength() - MAX_CARD_NAME_LENGTH);
				MessageBox(strMessage, NULL, MB_OK|MB_ICONINFORMATION);
				//Commit is already true
				//Don't let them continue
				*pbContinue = FALSE;
				return;
			}
			// (s.tullis 2016-02-12 15:07) - PLID 68130 - Don't allow duplicate card type names
			if (*pbCommit != FALSE)
			{
				ADODB::_RecordsetPtr rs = CreateParamRecordset("Select ID FROM CreditCardNamesT WHERE CardName = {STRING} AND ID <> {INT} ", strNewValue, nCardID);
				if (!rs->eof)
				{
					MessageBox("The Card Name already exists. Please enter another Card Name to continue. ", NULL, MB_OK | MB_ICONINFORMATION);
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
			}
		}

		//if they cancelled the commit, return
		if(*pbCommit == FALSE)
			return;

		//Otherwise, we don't care if they edited a new card
		if(nCardID == -1)
			return;

		//Check if it was in our updated array
		for(int i=0, nSize = m_aryChangedCards.GetSize(); i<nSize; i++){
			if(m_aryChangedCards.GetAt(i) == nCardID){
				//It is already here, return
				return;
			}
		}
		m_aryChangedCards.Add(nCardID);


	}NxCatchAll("Error in OnEditingFinishingCreditCardSetup.")	
}

void CEditCreditCardsDlg::OnOK() 
{	
	try{
		Save();
		CDialog::OnOK();
	}NxCatchAll("Error saving credit card setup.")
}

void CEditCreditCardsDlg::OnCancel() 
{	
	try{
		CDialog::OnCancel();
	}NxCatchAll("Error closing credit card editor.")
}

void CEditCreditCardsDlg::OnAddCard() 
{
	try{
		//Add a blank row to the list, giving it the proper default values
		IRowSettingsPtr pRow = m_pChargeCards->GetNewRow();
		_variant_t varDefaultInactive (VARIANT_FALSE, VT_BOOL);

		pRow->PutValue(cscID, (long) -1);
		pRow->PutValue(cscCardName, _bstr_t(""));
		pRow->PutValue(cscPAYMENTECHCardType, (long) pctCredit);
		pRow->PutValue(cscInactive, varDefaultInactive);

		m_pChargeCards->AddRowBefore(pRow, m_pChargeCards->GetFirstRow());
		m_pChargeCards->CurSel = pRow;

		//Put the user in edit mode on the new card name.
		m_pChargeCards->StartEditing(pRow, cscCardName);
		
	}NxCatchAll("Error adding new card to the list.")
	
}

void CEditCreditCardsDlg::OnRemoveCard() 
{
	try{
		IRowSettingsPtr pRow = m_pChargeCards->GetCurSel();
		//If no row is selected, return
		if(pRow == NULL)
			return;

		long nCardID = VarLong(pRow->GetValue(cscID), -1);

		//Check if this card can be removed
		// (j.gruber 2008-02-27 16:22) - PLID 29103 - made it check for orders
		if(ReturnsRecords("SELECT ID FROM PaymentPlansT WHERE CreditCardID = %li", nCardID)){
			MessageBox("This card exists on at least one payment and cannot be deleted. "
				"You may, however, inactivate it.");
			return;
		}

		if(ReturnsRecords("SELECT ID FROM OrderT WHERE CCTypeID = %li", nCardID)) {
			MessageBox("This card exists on at least one inventory order and cannot be deleted. "
				"You may, however, inactivate it.");
			return;
		}

		if(nCardID != -1){
			m_aryDeletedCards.Add(nCardID);
			//Check if it was in our updated array
			for(int i=0, nSize = m_aryChangedCards.GetSize(); i<nSize; i++){
				if(m_aryChangedCards.GetAt(i) == nCardID){
					//it was, it only needs to be in the delete array
					m_aryChangedCards.RemoveAt(i);
				}
			}
		}

		m_pChargeCards->RemoveRow(pRow);
		
		
	}NxCatchAll("Error removing selected card from the list.")
	
}

void CEditCreditCardsDlg::GenerateDeletedCardSql(CString& strSqlBatch)
{
	long nCardID = -1;
	for(int i=0, nSize = m_aryDeletedCards.GetSize(); i<nSize; i++){
		nCardID = m_aryDeletedCards.GetAt(i);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CreditCardNamesT WHERE ID = %li ", nCardID);
	}

}

void CEditCreditCardsDlg::GenerateNewCardSql(CString& strSqlBatch)
{
	long nCardID = -1, nPAYMENTECHCardType;
	CString strCardName;
	BOOL bInactive;

	IRowSettingsPtr pRow = m_pChargeCards->GetFirstRow();

	while(pRow){
		//Only insert the ones with our sentinel ID value
		if(VarLong(pRow->GetValue(cscID), -1) == -1){
			strCardName = VarString(pRow->GetValue(cscCardName), "");
			nPAYMENTECHCardType = VarLong(pRow->GetValue(cscPAYMENTECHCardType), 1);
			bInactive = VarBool(pRow->GetValue(cscInactive), FALSE);

			AddStatementToSqlBatch(strSqlBatch, 
				"INSERT INTO CreditCardNamesT (CardName, CardType, Inactive) "
				"VALUES ('%s', %li, %li) ",
				_Q(strCardName), nPAYMENTECHCardType, bInactive == FALSE? 0: 1);
		}
		pRow = pRow->GetNextRow();
	}

}

void CEditCreditCardsDlg::GenerateUpdateSql(CString& strSqlBatch)
{
	long nCardID = -1, nCardType;
	CString strCardName;
	BOOL bInactive;

	IRowSettingsPtr pRow;

	for(int i=0, nSize = m_aryChangedCards.GetSize(); i<nSize; i++){
		nCardID = m_aryChangedCards.GetAt(i);
		pRow = m_pChargeCards->FindByColumn(cscID, (long)nCardID, m_pChargeCards->GetFirstRow(), VARIANT_FALSE);
		if(pRow){
			strCardName = VarString(pRow->GetValue(cscCardName), "");
			nCardType = VarLong(pRow->GetValue(cscPAYMENTECHCardType), 1);
			bInactive = VarBool(pRow->GetValue(cscInactive), FALSE);

			AddStatementToSqlBatch(strSqlBatch, "UPDATE CreditCardNamesT "
				"SET CardName = '%s', CardType = %li, Inactive = %li "
				"WHERE ID = %li ",
				_Q(strCardName), nCardType, 
				bInactive == FALSE? 0: 1, 
				nCardID);
		}
		else{
			//Why didn't we find the row by ID?
			ASSERT(FALSE);
		}
		
	}

}

void CEditCreditCardsDlg::Save() 
{
	CString strSqlBatch = BeginSqlBatch();

	//In case our generated sql statements get doubled up (i.e. we delete and try to update the same record),
		//we should do them in insert, update, delete order to avoid referencial errors.
	GenerateNewCardSql(strSqlBatch);
	GenerateUpdateSql(strSqlBatch);
	GenerateDeletedCardSql(strSqlBatch);

	if(!strSqlBatch.IsEmpty())
		ExecuteSqlBatch(strSqlBatch);
	
}