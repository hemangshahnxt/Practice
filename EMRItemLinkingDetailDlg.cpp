// EMRItemLinkingDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRItemLinkingDetailDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRItemLinkingDetailDlg dialog


CEMRItemLinkingDetailDlg::CEMRItemLinkingDetailDlg(CWnd* pParent)
	: CNxDialog(CEMRItemLinkingDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRItemLinkingDetailDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nInfoID1 = -1;
	m_nInfoID2 = -1;

	m_bIsNew = true;
}

void CEMRItemLinkingDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRItemLinkingDetailDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEMRItemLinkingDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRItemLinkingDetailDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRItemLinkingDetailDlg message handlers

BOOL CEMRItemLinkingDetailDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//DRT 1/24/2007 - PLID 24401 - This interface controls creating data for the EMR Item Linking feature.

	try {
		// (c.haag 2008-04-29 17:20) - PLID 29840 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pList = BindNxDataList2Ctrl(IDC_LINKING_LIST, false);
		m_pDetailOne = BindNxDataList2Ctrl(IDC_DETAIL_LIST_ONE, true);
		m_pDetailTwo = BindNxDataList2Ctrl(IDC_DETAIL_LIST_TWO, true);

		if(m_nInfoID1 != -1 && m_nInfoID2 != -1) {
			//We are loading an existing detail
			m_bIsNew = false;
			BeginWaitCursor();
			LoadExistingDetail();
		}

		// TODO: Add extra initialization here
	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRItemLinkingDetailDlg::LoadExistingDetail()
{
	//
	//Load the info IDs into the 2 combo box lists
	//	We don't really save any specific order of details here.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDetailOne->SetSelByColumn(0, (long)m_nInfoID1);
	if(pRow == NULL) {
		//This should not be possible
		ASSERT(FALSE);
	}

	//Set the next one
	pRow = m_pDetailTwo->SetSelByColumn(0, (long)m_nInfoID2);
	if(pRow == NULL) {
		//This should not be possible
		ASSERT(FALSE);
	}

	//Both lists are set, do the generation of the main list from them.
	RefreshLinkList();

	//Must wait on refresh
	m_pList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	//
	//At this point we've loaded all the info data and requeried the main list.  Now we need to go through all
	//	the list items and join them together.  I have been poring over these trying to figure how to 
	//	get a query that gives me all the data I need, but I cannot find it.  We'll just loop through every
	//	EMRDataT record that is here and look for a link.
	CString strSql;
	NXDATALIST2Lib::IRowSettingsPtr pSearchRow = m_pList->FindAbsoluteFirstRow(VARIANT_FALSE);
	while(pSearchRow != NULL) {
		long nDataID = VarLong(pSearchRow->GetValue(0));

		//Find another DataID for m_nInfoID2 that shares our LinkID
		CString str;
		str.Format("UNION SELECT %li AS Src, EMRDataT.ID, EMRItemLinkedDataT.EmrLinkID "
			"FROM EMRItemLinkedDataT INNER JOIN EMRDataT ON EMRItemLinkedDataT.EMRDataID = EMRDataT.ID "
			"WHERE EMRDataT.EMRInfoID = %li AND EMRItemLinkedDataT.EMRLinkID IN "
			"	(SELECT EMRLinkID FROM EMRItemLinkedDataT WHERE EMRDataID = %li) ", 
			nDataID, m_nInfoID2, nDataID);
		strSql += str;

		pSearchRow = m_pList->FindAbsoluteNextRow(pSearchRow, VARIANT_FALSE);
	}

	//remove pre-UNION
	if(strSql.GetLength() > 6)
		strSql = strSql.Mid(6);

	//Execute our query
	if(!strSql.IsEmpty()) {
		ADODB::_RecordsetPtr prs = CreateRecordsetStd(strSql);
		while(!prs->eof) {
			long nDataID1 = AdoFldLong(prs, "Src");

			_variant_t var = prs->Fields->Item["ID"]->Value;
			if(var.vt == VT_I4) {
				long nDataID2 = VarLong(var);
				long nLinkID = AdoFldLong(prs, "EmrLinkID");

				NXDATALIST2Lib::IRowSettingsPtr pUpdateRow = m_pList->FindByColumn(0, (long)nDataID1, NULL, VARIANT_FALSE);
				if(pUpdateRow != NULL) {
					pUpdateRow->PutValue(2, (long)nDataID2);
					pUpdateRow->PutValue(3, (long)nLinkID);
				}
			}
			else {
				//No link to this ID
			}

			prs->MoveNext();
		}
		prs->Close();
	}
}

void CEMRItemLinkingDetailDlg::OnOK() 
{
	try {
		CString strInsert;

		//Loop through all records in the main list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);
		while(pRow != NULL) {
			//Source and destination have no actual bearing on the final saved data, they are
			//	only for source code clarification of which side we're looking at.
			long nSourceDataID = VarLong(pRow->GetValue(0));

			_variant_t varDest = pRow->GetValue(2);
			if(varDest.vt == VT_I4) {
				//There is a selection here
				long nDestDataID = VarLong(varDest);
				_variant_t varLinkID = pRow->GetValue(3);

				//Ensure that this changed by looking for a NULL link ID, or just if we're on a new Link
				if(m_bIsNew || varLinkID.vt == VT_NULL) {

					//We will name the link as just a collection of the data records.  Note that the name has
					//	no bearing (and should never) on the program itself usage of these links.  It is only
					//	for the exporter to be able to update existing links when they are put into another
					//	database that may already have them.
					CString strLeft, strRight;
					strLeft = VarString(pRow->GetValue(1));
					strRight = VarString(pRow->GetOutputValue(2));

					CString strLinkName;
					strLinkName.Format("%s || %s", strLeft, strRight);

					//insert!
					//DRT 1/24/2007 - PLID 24401 - Note!  We have designed the data structure to be quite flexible and allow multiple links in a 
					//	EMRItemLinksT group.  However, the interface that allows entry into such a structure has been deemed too
					//	complex for what is needed.  So while the interface pretends a 1-1 bridge, it is actually a collective
					//	group that is being put into data.
					CString str;
					str.Format("SET @nLinkID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRItemLinksT);\r\n");
					strInsert += str;
					str.Format("INSERT INTO EmrItemLinksT (ID, Name) values (@nLinkID, '%s');\r\n", _Q(strLinkName));
					strInsert += str;

					//EMRItemLinkedDataT.ID is an identity
					str.Format("INSERT INTO EmrItemLinkedDataT (EmrDataID, EmrLinkID) values (%li, @nLinkID);\r\n", nSourceDataID);
					strInsert += str;
					str.Format("INSERT INTO EmrItemLinkedDataT (EmrDataID, EmrLinkID) values (%li, @nLinkID);\r\n", nDestDataID);
					strInsert += str;
				}
			}

			pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}


		//Generate any deletions that need to happen.
		CString strDelete;
		for(int i = 0; i < m_dwaryDeleteLinks.GetSize(); i++) {
			long nID = m_dwaryDeleteLinks.GetAt(i);

			CString str;
			str.Format(
				"SET @nDeleteID = %li;\r\n"
				"DELETE FROM EmrItemLinkedDataT WHERE EmrLinkID = @nDeleteID;\r\n"
				"DELETE FROM EmrItemLinksT WHERE ID = @nDeleteID;\r\n", nID);
			strDelete += str;
		}

		if(strInsert.IsEmpty() && strDelete.IsEmpty()) {
			//There are no actual records to insert, warn the user
			AfxMessageBox("You have made no changes to the links between details.  Please make some selections before attempting to save.  If you do not wish to make changes, please press Cancel.");
			return;
		}

		//pre-prend some info
		CString str;
		str.Format("DECLARE @nLinkID int;\r\n");
		strInsert = str + strInsert;

		//If anything to delete, pre-pend
		if(!strDelete.IsEmpty()) {
			CString str;
			str.Format("DECLARE @nDeleteID int;\r\n");
			strDelete = str + strDelete;
		}

		//Execute all insertions in a transaction
		BEGIN_TRANS("EMRItemLinkingDetails") {
			ExecuteSqlStd(strInsert);

			//We can delete afterwards
			if(!strDelete.IsEmpty())
				ExecuteSqlStd(strDelete);
		} END_TRANS("EMRItemLinkingDetails");

		//Only close the dialog if no exception
		CDialog::OnOK();

	} NxCatchAll("Error in OnOK");
}

void CEMRItemLinkingDetailDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CEMRItemLinkingDetailDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRItemLinkingDetailDlg)
	ON_EVENT(CEMRItemLinkingDetailDlg, IDC_DETAIL_LIST_ONE, 16 /* SelChosen */, OnSelChosenDetailListOne, VTS_DISPATCH)
	ON_EVENT(CEMRItemLinkingDetailDlg, IDC_DETAIL_LIST_TWO, 16 /* SelChosen */, OnSelChosenDetailListTwo, VTS_DISPATCH)
	ON_EVENT(CEMRItemLinkingDetailDlg, IDC_LINKING_LIST, 10 /* EditingFinished */, OnEditingFinishedLinkingList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRItemLinkingDetailDlg::OnSelChosenDetailListOne(LPDISPATCH lpRow) 
{
	try {
		//Selection has changed, refresh the list at the bottom
		//TES 2/9/2007 - PLID 24671 - Make sure they don't select the same item in both lists.
		NXDATALIST2Lib::IRowSettingsPtr pRowOne(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr pRowTwo = m_pDetailTwo->CurSel;
		if(pRowOne != NULL) {
			if(pRowTwo != NULL) {
				long nInfo1 = VarLong(pRowOne->GetValue(0));
				long nInfo2 = VarLong(pRowTwo->GetValue(0));
				if(nInfo1 == nInfo2) {
					MsgBox("You cannot link a detail to itself");
					m_pDetailOne->CurSel = NULL;
				}
			}
		}

		//If we are editing an existing list, we want to delete all references to all links, the whole setup is now invalid.
		//	Loop through all rows in the main list and find the links.  Only need to do this on existing edits.
		if(m_bIsNew == false) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_FALSE);
			while(pRow != NULL) {
				_variant_t var = pRow->GetValue(3);	//LinkID
				if(var.vt == VT_I4) {
					m_dwaryDeleteLinks.Add(VarLong(var));
				}

				pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
			}
		}

		RefreshLinkList();
	} NxCatchAll("Error in OnSelChosenDetailListOne");
}

void CEMRItemLinkingDetailDlg::OnSelChosenDetailListTwo(LPDISPATCH lpRow) 
{
	try {
		//Selection has changed, refresh the list at the bottom
		//TES 2/9/2007 - PLID 24671 - Make sure they don't select the same item in both lists.
		NXDATALIST2Lib::IRowSettingsPtr pRowOne = m_pDetailOne->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pRowTwo(lpRow);
		if(pRowOne != NULL) {
			if(pRowTwo != NULL) {
				long nInfo1 = VarLong(pRowOne->GetValue(0));
				long nInfo2 = VarLong(pRowTwo->GetValue(0));
				if(nInfo1 == nInfo2) {
					MsgBox("You cannot link a detail to itself");
					m_pDetailTwo->CurSel = NULL;
				}
			}
		}

		//If we are editing an existing list, we want to delete all references to all links, the whole setup is now invalid.
		//	Loop through all rows in the main list and find the links.  Only need to do this on existing edits.
		if(m_bIsNew == false) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_FALSE);
			while(pRow != NULL) {
				_variant_t var = pRow->GetValue(3);	//LinkID
				if(var.vt == VT_I4) {
					m_dwaryDeleteLinks.Add(VarLong(var));
				}

				pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
			}
		}

		RefreshLinkList();
	} NxCatchAll("Error in OnSelChosenDetailListOne");
}

void CEMRItemLinkingDetailDlg::RefreshLinkList()
{
	//Clear anything currently in the list
	m_pList->Clear();

	//Get ptrs to both selections.
	NXDATALIST2Lib::IRowSettingsPtr pRowOne = m_pDetailOne->CurSel;
	NXDATALIST2Lib::IRowSettingsPtr pRowTwo = m_pDetailTwo->CurSel;

	//If detail one is empty, just quit
	if(pRowOne == NULL)
		return;

	//
	//The right-hand side will be an embedded combo of everything in detail 2.  Generate that clause first.
	CString strComboSrc;
	if(pRowTwo != NULL) {
		long nTwoInfoID = VarLong(pRowTwo->GetValue(0));
		strComboSrc.Format("SELECT ID, Data, CASE WHEN Inactive = 1 THEN 0 ELSE 1 END AS Visible, SortOrder FROM EMRDataT WHERE EMRInfoID = %li UNION SELECT NULL, '<No Selection>', 1, -1 ORDER BY SortOrder", nTwoInfoID);
	}

	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(2);
	pCol->ComboSource = _bstr_t(strComboSrc);

	//end detail two
	//

	//
	//The left-hand side needs to be a list of all list items in the first detail
	long nInfoID = VarLong(pRowOne->GetValue(0));

	//Generate the query
	CString strFrom, strWhere;
	strFrom.Format("EMRDataT");
	strWhere.Format("EMRDataT.EMRInfoID = %li AND EMRDataT.Inactive = 0", nInfoID);

	m_pList->FromClause = _bstr_t(strFrom);
	m_pList->WhereClause = _bstr_t(strWhere);

	//end detail one
	//

	//Now requery!
	m_pList->Requery();
}

void CEMRItemLinkingDetailDlg::OnEditingFinishedLinkingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	_variant_t varNull;
	varNull.vt = VT_NULL;

	try {
		switch(nCol) {
		case 2:	//Linked To
			{
				if(lpRow == NULL)
					return;	//shouldn't be possible

				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

				//The user is changing the selection of Linked To value.  If we are editing an existing list, 
				//	then we need to delete the link that used to be here (if any), so that a new one can be 
				//	created.  There are a few scenarios here:
				if(m_bIsNew == false) {
					//this is the newly selected value in the embedded combo
					_variant_t varNew = pRow->GetValue(nCol);
					//LinkID value
					_variant_t varLink = pRow->GetValue(3);

					if(varNew.vt == VT_I4) {
						if(varLink.vt == VT_I4) {
							long nLinkID = VarLong(varLink);
							long nNewDataID = VarLong(varNew);

							//A)  Remove an individual row that used to be linked (new selection is NULL)
							if(nNewDataID == 0) {
								//The datalist translates NULL into 0 when the user selects <No Selection>
								m_dwaryDeleteLinks.Add(nLinkID);
								pRow->PutValue(3, varNull);
								pRow->PutValue(2, varNull);
							}
							//B) A new selection is made 
							else {
								//This is an existing link, flag it for deletion so the new data will be created clean
								m_dwaryDeleteLinks.Add(nLinkID);

								//reset the value in the link column so we don't get confused later
								pRow->PutValue(3, varNull);
							}
						}
						else {
							//NULL values need to manually get reset to <No Selection>
							if(VarLong(varNew) == 0)
								pRow->PutValue(2, varNull);
						}
					}
					else {
						//Bad data -- no reason to mess with the link
					}
				}
				else {
					//On a new link, we don't care, there can never be anything to delete.
					//The datalist translates NULL into 0 when the user selects <No Selection>
					_variant_t varNew = pRow->GetValue(nCol);
					if(varNew.vt == VT_I4 && VarLong(varNew) == 0)
						pRow->PutValue(2, varNull);
				}
			}
			break;
		}


	} NxCatchAll("Error in OnEditingFinishedLinkingList");
}
