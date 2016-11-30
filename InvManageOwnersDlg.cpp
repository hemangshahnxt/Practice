// InvManageOwnersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvManageOwnersDlg.h"

// (j.fouts 2012-08-10 09:12) - PLID 50934 - Created.

enum TrackableInventoryList { tilProdID = 0, tilProdName, tilCategory, tilUnitDesc };
enum AllOwnersList { aolID = 0 , aolLast , aolFirst };
enum SelectedOwnersList { solID = 0 , solLast , solFirst, solColor };

enum SelectOwnerColor{ socBlackColor = 0x0, socRedColor = 0xFF, };

// CInvManageOwnersDlg dialog

IMPLEMENT_DYNAMIC(CInvManageOwnersDlg, CNxDialog)

CInvManageOwnersDlg::CInvManageOwnersDlg(CWnd* pParent /*=NULL*/, long nPreselectedID /*=-1*/)
	: CNxDialog(CInvManageOwnersDlg::IDD, pParent)
{
	//Set the selected equipment to the id they pass in
	m_nPreselectedID = nPreselectedID;
}

CInvManageOwnersDlg::~CInvManageOwnersDlg()
{
}

void CInvManageOwnersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_ADD_OWNER, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_OWNER, m_btnRemove);
}


BEGIN_MESSAGE_MAP(CInvManageOwnersDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_OWNER, &CInvManageOwnersDlg::OnBnClickedAddOwner)
	ON_BN_CLICKED(IDC_REMOVE_OWNER, &CInvManageOwnersDlg::OnBnClickedRemoveOwner)
END_MESSAGE_MAP()

BOOL CInvManageOwnersDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		//Add our icons
		m_btnOk.AutoSet(NXB_OK);
		m_btnAdd.AutoSet(NXB_RIGHT);
		m_btnRemove.AutoSet(NXB_LEFT);

		//Bind the datalists
		m_pItems = BindNxDataList2Ctrl(IDC_OWNABLE_ITEMS);
		m_pAllOwners = BindNxDataList2Ctrl(IDC_ALL_OWNERS);
		m_pItemOwners = BindNxDataList2Ctrl(IDC_ITEM_OWNERS, false);

		//If they passed in a preselected item, select it
		if(m_nPreselectedID >= 0)
		{
			m_pItems->FindByColumn(tilProdID,_variant_t(m_nPreselectedID), 0, true);
		}

		RequeryOwnersList();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

// CInvManageOwnersDlg message handlers
BEGIN_EVENTSINK_MAP(CInvManageOwnersDlg, CNxDialog)
	ON_EVENT(CInvManageOwnersDlg, IDC_ALL_OWNERS, 3, CInvManageOwnersDlg::DblClickCellAllOwners, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvManageOwnersDlg, IDC_ITEM_OWNERS, 3, CInvManageOwnersDlg::DblClickCellItemOwners, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvManageOwnersDlg, IDC_OWNABLE_ITEMS, 2, CInvManageOwnersDlg::SelChangedOwnableItems, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CInvManageOwnersDlg, IDC_OWNABLE_ITEMS, 19, CInvManageOwnersDlg::LeftClickOwnableItems, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CInvManageOwnersDlg::OnBnClickedAddOwner()
{
	try
	{
		AddOwner();
	}NxCatchAll(__FUNCTION__);
}

void CInvManageOwnersDlg::OnBnClickedRemoveOwner()
{
	try
	{
		RemoveOwner();
	}NxCatchAll(__FUNCTION__);
}

void CInvManageOwnersDlg::AddOwner()
{
	if(!m_pItems->CurSel)
	{
		MessageBox("Please select at least one inventory item first.");
	}
	else
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pAllOwners->GetFirstSelRow();

		if(!pRow)
		{
			MessageBox("Please select at least one owner first.");
			return;
		}

		CParamSqlBatch sqlBatch;

		{
			//We need to check if the Product is in ProductRequestableT and add it if its not
			NXDATALIST2Lib::IRowSettingsPtr pProductRow;
			for(pProductRow = m_pItems->GetFirstSelRow(); pProductRow; pProductRow = pProductRow->GetNextSelRow())
			{
				long nProdID = VarLong(pProductRow->GetValue(tilProdID));

				sqlBatch.Add("IF NOT EXISTS(SELECT * FROM ProductRequestableT WHERE ProductID = {INT}) " 
					"INSERT INTO ProductRequestableT ( ProductID, IsRequestable, CurrentStatus) VALUES ({INT}, 1, 1)",
					nProdID, nProdID);
			}
		}

		while(pRow)
		{
			//Get the id of the current selected owner
			long nSelID = VarLong(pRow->GetValue(aolID), -1);
			if(nSelID >= 0)
			{
				NXDATALIST2Lib::IRowSettingsPtr pExistingOwnerRow;
				pExistingOwnerRow = m_pItemOwners->FindByColumn(solID, nSelID, m_pItemOwners->GetFirstRow() ,FALSE);
				if(pExistingOwnerRow)
				{
					//The owner was already in our datalist, let show them as black if they were red before
					pExistingOwnerRow->PutValue(solColor, _variant_t(socBlackColor));
					pExistingOwnerRow->ForeColor = socBlackColor;
				} 
				else 
				{
					//They are not in the datalist yet so lets add them
					NXDATALIST2Lib::IRowSettingsPtr pNewOwnerRow = m_pItemOwners->GetNewRow();
					pNewOwnerRow->PutValue(solID, pRow->GetValue(aolID));
					pNewOwnerRow->PutValue(solLast, pRow->GetValue(aolLast));
					pNewOwnerRow->PutValue(solFirst, pRow->GetValue(aolFirst));
					pNewOwnerRow->PutValue(solColor, _variant_t(socBlackColor));

					m_pItemOwners->AddRowSorted(pNewOwnerRow, NULL);
				}
				
				NXDATALIST2Lib::IRowSettingsPtr pProductRow;
				for(pProductRow = m_pItems->GetFirstSelRow(); pProductRow; pProductRow = pProductRow->GetNextSelRow())
				{
					long nOwnerID = VarLong(pRow->GetValue(aolID));
					long nProdID = VarLong(pProductRow->GetValue(tilProdID));

					sqlBatch.Add(
						"IF NOT EXISTS( "
						"	SELECT * FROM ProductRequestableOwnersT "
						"	INNER JOIN ProductRequestableT ON ProductRequestableOwnersT.ProductRequestableID = ProductRequestableT.ID"
						"	WHERE ProductID = {INT} AND OwnerID = {INT}"
						") INSERT INTO ProductRequestableOwnersT (ProductRequestableID , OwnerID) VALUES ((SELECT ID FROM ProductRequestableT WHERE ProductID = {INT}), {INT})",
						nProdID, nOwnerID, nProdID, nOwnerID);
				}		
			}
			pRow = pRow->GetNextSelRow();
		}

		if(!sqlBatch.IsEmpty())
		{
			sqlBatch.Execute(GetRemoteData());
		}
	}
}

void CInvManageOwnersDlg::RemoveOwner()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pItemOwners->GetFirstSelRow();

	CParamSqlBatch sqlBatch;

	while(pRow)
	{
		NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextSelRow();

		NXDATALIST2Lib::IRowSettingsPtr pProductRow;
		for(pProductRow = m_pItems->GetFirstSelRow(); pProductRow; pProductRow = pProductRow->GetNextSelRow())
		{
			long nOwnerID = VarLong(pRow->GetValue(aolID));
			long nProdID = VarLong(pProductRow->GetValue(tilProdID));

			sqlBatch.Add("DELETE FROM ProductRequestableOwnersT WHERE "
				"ProductRequestableID = (SELECT ID FROM ProductRequestableT WHERE ProductID = {INT}) AND OwnerID = {INT}",
				nProdID, nOwnerID);		
		}	

		m_pItemOwners->RemoveRow(pRow);
		pRow = pNextRow;
	}

	if(!sqlBatch.IsEmpty())
	{
		sqlBatch.Execute(GetRemoteData());
	}
}

// (j.fouts 2012-08-23 17:34) - PLID 50934 - Add owner on a double click
void CInvManageOwnersDlg::DblClickCellAllOwners(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		AddOwner();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-08-23 17:34) - PLID 50934 - Remove owner on a double click
void CInvManageOwnersDlg::DblClickCellItemOwners(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		RemoveOwner();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-08-23 17:35) - PLID 50934 - Requery the Owners list based on current selections
void CInvManageOwnersDlg::RequeryOwnersList()
{
	CString strINClause = "";
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pItems->GetFirstSelRow();
	long nNumSelected = 0;

	//Build an IN clause of all selected items
	while(pRow)
	{
		long nSelID = VarLong(pRow->GetValue(tilProdID), -1);
		if(nSelID >= 0)
		{
			strINClause += FormatString("%li, ", nSelID);
			++nNumSelected;
		}
		pRow = pRow->GetNextSelRow();
	}

	strINClause.TrimRight(", ");

	if(strINClause.IsEmpty())
	{
		m_pItemOwners->Clear();
	}
	else
	{
		m_pItemOwners->PutFromClause(_bstr_t(FormatString(
			"UsersT INNER JOIN PersonT ON PersonT.ID = UsersT.PersonID "
			"INNER JOIN (SELECT OwnerID, CASE WHEN Count(OwnerID) = %li THEN %li ELSE %li END AS Color "
			"FROM (SELECT DISTINCT ProductRequestableID, OwnerID FROM ProductRequestableOwnersT) SlimOwnersT "
			"INNER JOIN ProductRequestableT ON ProductRequestableT.ID = SlimOwnersT.ProductRequestableID "
			"WHERE ProductID IN ( %s ) "
			"GROUP BY OwnerID) AllQ ON AllQ.OwnerID = PersonT.ID"
			, nNumSelected, (long)socBlackColor, (long)socRedColor, strINClause)));
		m_pItemOwners->PutWhereClause(_bstr_t(FormatString(
			"PersonT.ID IN (SELECT OwnerID FROM ProductRequestableOwnersT "
			"INNER JOIN ProductRequestableT ON ProductRequestableT.ID = ProductRequestableOwnersT.ProductRequestableID "
			"WHERE ProductID IN (%s))", strINClause)));
			
		m_pItemOwners->Requery();
	}
}

// (j.fouts 2012-08-23 17:35) - PLID 50934 - Selection Changed so requery owners
void CInvManageOwnersDlg::SelChangedOwnableItems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		RequeryOwnersList();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-08-23 17:36) - PLID 50934 - SelChanged does not account for having multiple items selected and 
// then clicking the last selecting item so that CurSel does not change but but all other selected items are
// no longer selected. Added LeftClick to handle this case as well.
void CInvManageOwnersDlg::LeftClickOwnableItems(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		RequeryOwnersList();
	}
	NxCatchAll(__FUNCTION__);
}
