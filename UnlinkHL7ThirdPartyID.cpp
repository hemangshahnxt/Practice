// UnlinkHL7ThirdPartyID.cpp : implementation file
//

// (b.savon 2011-10-04 11:22) - PLID 39890 - We need a way to unlink patients/providers/ref. phys 
//											 that have their NexTech 3rd party ID remembered

#include "stdafx.h"
#include "Practice.h"
#include "PracticeRc.h"
#include "UnlinkHL7ThirdPartyID.h"

using namespace NXDATALIST2Lib;

// (b.savon 2011-10-04 11:22) - PLID 39890 - Setup DDL/DL column enums
enum HL7GroupFilter{
	hgfID = 0,
	hgfName
};

enum HL7LinkFilter{
	hlfLink = 0,
	hlfName
};

enum HL7UnlinkList{
	hulGroupID = 0,
	hulName,
	hulThirdParty,
	hulPracticeID,
	hulPracticeLink,
	hulRecordType
};

// CUnlinkHL7ThirdPartyID dialog

IMPLEMENT_DYNAMIC(CUnlinkHL7ThirdPartyID, CNxDialog)

// (b.savon 2011-10-04 11:22) - PLID 39890 - Constructor Initialize
CUnlinkHL7ThirdPartyID::CUnlinkHL7ThirdPartyID(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUnlinkHL7ThirdPartyID::IDD, pParent)
{
	m_strGroupFilter = "< All Groups >";
	m_nGroupFilterID = -1;
	m_strLinkFilter = "< All Links >";
	m_nLinkFilterID = -1;

	m_strBothFilterWhere = " UnLinkQ.GroupID = %li AND UnLinkQ.RecordType = %li ";
	m_strGroupFilterWhere = " UnLinkQ.GroupID = %li ";
	m_strLinkFilterWhere = " UnLinkQ.RecordType = %li ";
}

CUnlinkHL7ThirdPartyID::~CUnlinkHL7ThirdPartyID()
{
}

// (b.savon 2011-10-04 11:22) - PLID 39890 - Setup Dialog and DDL/DL
BOOL CUnlinkHL7ThirdPartyID::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		//	Do the buttons
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnUnlink.AutoSet(NXB_DELETE);

		//	Do the datalists
		m_pHL7GroupFilter = BindNxDataList2Ctrl(IDC_NX_DL_HL7_GROUP_FILTER, false);
		m_pHL7LinkFilter = BindNxDataList2Ctrl(IDC_NX_DL_HL7_LINK_FILTER, false);
		m_pHL7UnlinkList = BindNxDataList2Ctrl(IDC_NX_DL_UNLINK_LIST, false);
		BindGroupList();
		BindLinkList();
		BindUnlinkList();

	}NxCatchAll("CUnlinkHL7ThirdPartyID::OnInitDialog - Unable to build dialog.");

	return TRUE;
}//END BOOL CUnlinkHL7ThirdPartyID::OnInitDialog()

void CUnlinkHL7ThirdPartyID::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDB_UNLINK_SELECTED, m_btnUnlink);
}


BEGIN_MESSAGE_MAP(CUnlinkHL7ThirdPartyID, CNxDialog)
	ON_BN_CLICKED(IDOK, &CUnlinkHL7ThirdPartyID::OnBnClickedOk)
	ON_BN_CLICKED(IDB_UNLINK_SELECTED, &CUnlinkHL7ThirdPartyID::OnBnClickedUnlinkSelected)
END_MESSAGE_MAP()


// CUnlinkHL7ThirdPartyID message handlers

void CUnlinkHL7ThirdPartyID::OnBnClickedOk()
{
	CNxDialog::OnOK();
}

// (b.savon 2011-10-04 11:22) - PLID 39890 - Bind the Data to the Group Drop Down Filter
void CUnlinkHL7ThirdPartyID::BindGroupList()
{
	try{
		//	Bind the HL7 Group Filter Combo
		m_pHL7GroupFilter->Requery();
		m_pHL7GroupFilter->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			//	Add an 'All' row
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pHL7GroupFilter->GetNewRow();
		pRow->PutValue(hgfID, -1);
		pRow->PutValue(hgfName, _bstr_t(m_strGroupFilter));
		m_pHL7GroupFilter->AddRowBefore(pRow, m_pHL7GroupFilter->GetFirstRow());
		m_pHL7GroupFilter->PutComboBoxText(_bstr_t(m_strGroupFilter));
	}NxCatchAll("CUnlinkHL7ThirdPartyID::BindGroupList - Unable to bind group list.");
}//END void CUnlinkHL7ThirdPartyID::BindGroupList()

// (b.savon 2011-10-04 11:22) - PLID 39890 - Bind the Data to the Link Drop Down Filter
void CUnlinkHL7ThirdPartyID::BindLinkList()
{
	try{
		//	Bind the HL7 Field list taken from HL7IDLink_RecordType enums
		m_pHL7LinkFilter->FromClause = _bstr_t("(SELECT	Name, Link "
												"FROM	( "
												"			SELECT	DISTINCT "
												"					CASE RecordType "
												"						WHEN( 1 ) THEN 'Patient' "
												"						WHEN( 2 ) THEN 'Provider' "
												"						WHEN( 3 ) THEN 'Referring Physician' "
												"					ELSE "
												"						'ID Unknown'		 "				
												"					END AS Name, "
												"					RecordType AS Link"	
												"			FROM	HL7IDLinkT	"	
												"		) AS lQ )  AS LinkedQ");
		m_pHL7LinkFilter->Requery();
		m_pHL7LinkFilter->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			//	Add an 'All' row
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pHL7LinkFilter->GetNewRow();
		pRow->PutValue(hlfLink, -1);
		pRow->PutValue(hlfName, _bstr_t(m_strLinkFilter));
		m_pHL7LinkFilter->AddRowBefore(pRow, m_pHL7LinkFilter->GetFirstRow());
		m_pHL7LinkFilter->PutComboBoxText(_bstr_t(m_strLinkFilter));
	}NxCatchAll("CUnlinkHL7ThirdPartyID::BindGroupList - Unable to bind link list.");
}//END void CUnlinkHL7ThirdPartyID::BindLinkList()

// (b.savon 2011-10-04 11:22) - PLID 39890 - Bind the Data to the Unlink items List
void CUnlinkHL7ThirdPartyID::BindUnlinkList()
{
	try{ 
		m_pHL7UnlinkList->Clear();
		//	Bind the HL7 Field list taken from HL7IDLink_RecordType enums
		m_pHL7UnlinkList->FromClause = _bstr_t("(SELECT	GroupID, Name, ThirdParty, PracticeID, PracticeLink, RecordType "
												"FROM	( "
												"			SELECT	GroupID,  "
												"					CASE RecordType "
												"						WHEN( 1 ) THEN 'Patient' "
												"						WHEN( 2 ) THEN 'Provider' "
												"						WHEN( 3 ) THEN 'Referring Physician' "
												"					ELSE "
												"						'Unknown' "
												"					END AS Name, "
												"					ThirdPartyID AS ThirdParty, "
												"					PersonID AS PracticeID, "
												"					LastName + ', ' + FirstName AS PracticeLink, "
												"					RecordType AS RecordType "
												"			FROM	HL7IDLinkT	"
												"		) AS uQ )AS UnLinkQ ");
		//	Dependent on the status of the filters, prepare the WHERE clause
		if( !GetBindUnlinkListWhereClause().IsEmpty() ){
			m_pHL7UnlinkList->WhereClause = _bstr_t(GetBindUnlinkListWhereClause());
		} else{ 
			m_pHL7UnlinkList->WhereClause = _bstr_t("");
		}
		m_pHL7UnlinkList->Requery();
		m_pHL7UnlinkList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	}NxCatchAll("CUnlinkHL7ThirdPartyID::BindGroupList - Unable to bind unlinked list.");
}//END void CUnlinkHL7ThirdPartyID::BindUnlinkList()

// (b.savon 2011-10-04 11:22) - PLID 39890 - Grab the row pointers for all the selected rows in the
//											 unlink list.
void CUnlinkHL7ThirdPartyID::PrepareSelectedRows( CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arySelectedRows )
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pHL7UnlinkList->GetFirstRow();
	while( pRow ){
		//	Add the selected rows to the array.
		if( pRow->Selected == VARIANT_TRUE ){
			arySelectedRows.Add(pRow);
		}
		pRow = pRow->GetNextRow();
	}
}//END void CUnlinkHL7ThirdPartyID::PrepareSelectedRows( CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arySelectedRows )

// (b.savon 2011-10-04 11:22) - PLID 39890 - Unlink the selected items in the list
void CUnlinkHL7ThirdPartyID::OnBnClickedUnlinkSelected()
{
	// (b.savon 2011-10-04 11:22) - PLID 39890 - Populate the list of selected row pointers
	CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> arySelectedRows;
	PrepareSelectedRows( arySelectedRows );

	// (b.savon 2011-10-04 11:22) - PLID 39890 - Alert the user this action cannot be undone.
	int nNumberSelected = arySelectedRows.GetSize();
	if( nNumberSelected == 0 ){
		MessageBox("Please select items to unlink.", "No Items Selected", MB_OK | MB_ICONWARNING);
		return;
	}//END if( none selected )
	
	CString strMessage;
	strMessage.Format("There are (%li) selected HL7 third party links.\n\nThis action is cannot be undone.  Are you sure you would like to remove the link(s)?", nNumberSelected);
	if( IDYES == MessageBox(strMessage, "Unlink Selected", MB_YESNO | MB_ICONWARNING ) ){
		CString strUnlinkSelectedQuery = _T("");
		CString strSingleDeleteQuery = _T("");
		// (b.savon 2011-10-04 11:22) - PLID 39890 - If they agree, prepare the DELETE query
		for( int idx = 0; idx < nNumberSelected; idx++ ){
			strSingleDeleteQuery.Format("DELETE FROM HL7IDLinkT WHERE GroupID = %li AND ThirdPartyID = '%s' AND PersonID = %li AND RecordType = %li \r\n", 
										VarLong(arySelectedRows.GetAt(idx)->GetValue(hulGroupID)),
										VarString(arySelectedRows.GetAt(idx)->GetValue(hulThirdParty)),
										VarLong(arySelectedRows.GetAt(idx)->GetValue(hulPracticeID)),
										VarLong(arySelectedRows.GetAt(idx)->GetValue(hulRecordType)) );
			strUnlinkSelectedQuery += strSingleDeleteQuery;
			strSingleDeleteQuery.Empty();
		}//END for( format query )

		// (b.savon 2011-10-04 11:22) - PLID 39890 - Execute the query
		try{
			ExecuteSql(strUnlinkSelectedQuery);
		}NxCatchAll("void CUnlinkHL7ThirdPartyID::OnBnClickedUnlinkSelected() - Unable to delete HL7 links.");

		// (b.savon 2011-10-04 11:22) - PLID 39890 - Remove the rows from the list, without requerying the data
		//											 One less trip to the DB
		for( int idx = 0; idx < nNumberSelected; idx++ ){
			m_pHL7UnlinkList->RemoveRow(arySelectedRows.GetAt(idx));
		}
	}//END if( MessageBox->Yes )
}//END void CUnlinkHL7ThirdPartyID::OnBnClickedUnlinkSelected()

// (b.savon 2011-10-04 11:22) - PLID 39890 - Return the WHERE clause dependent on the status of the filters.
CString CUnlinkHL7ThirdPartyID::GetBindUnlinkListWhereClause()
{
	CString strWhere = _T("");
	//	Both filtered
	if( m_nGroupFilterID != -1 && m_nLinkFilterID != -1 ){
		strWhere.Format(m_strBothFilterWhere, m_nGroupFilterID, m_nLinkFilterID);
		return strWhere;
	} else if ( m_nGroupFilterID != -1 ){ // Group filtered
		strWhere.Format(m_strGroupFilterWhere, m_nGroupFilterID);
		return strWhere;
	} else if ( m_nLinkFilterID != -1 ){ //	Link Filtered
		strWhere.Format(m_strLinkFilterWhere, m_nLinkFilterID);
		return strWhere;
	} else{ // No Filter
		return strWhere;
	}
}//END CString CUnlinkHL7ThirdPartyID::GetBindUnlinkListWhereClause()

BEGIN_EVENTSINK_MAP(CUnlinkHL7ThirdPartyID, CNxDialog)
	ON_EVENT(CUnlinkHL7ThirdPartyID, IDC_NX_DL_HL7_GROUP_FILTER, 16, CUnlinkHL7ThirdPartyID::SelChosenNxDlHl7GroupFilter, VTS_DISPATCH)
	ON_EVENT(CUnlinkHL7ThirdPartyID, IDC_NX_DL_HL7_LINK_FILTER, 16, CUnlinkHL7ThirdPartyID::SelChosenNxDlHl7LinkFilter, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (b.savon 2011-10-04 11:22) - PLID 39890 - Group Filtered
void CUnlinkHL7ThirdPartyID::SelChosenNxDlHl7GroupFilter(LPDISPATCH lpRow)
{
	//	Make sure the user doesn't do anything silly.
	if( lpRow != NULL ){
		// (b.savon 2011-10-04 11:22) - PLID 39890 - Set the Group filter members
		m_strGroupFilter = VarString(m_pHL7GroupFilter->CurSel->GetValue(hgfName), "< All Groups >"); 
		m_nGroupFilterID = VarLong(m_pHL7GroupFilter->CurSel->GetValue(hgfID), -1);
		// (b.savon 2011-10-04 11:22) - PLID 39890 - Update the Unlink List with our new filtered results
		BindUnlinkList();
	} else{
		m_pHL7GroupFilter->PutComboBoxText(_bstr_t(m_strGroupFilter));
	}
}//END void CUnlinkHL7ThirdPartyID::SelChosenNxDlHl7GroupFilter(LPDISPATCH lpRow)

// (b.savon 2011-10-04 11:22) - PLID 39890 - Link Filtered
void CUnlinkHL7ThirdPartyID::SelChosenNxDlHl7LinkFilter(LPDISPATCH lpRow)
{
	//	Make sure the user doesn't do anything silly.
	if( lpRow != NULL ){
		// (b.savon 2011-10-04 11:22) - PLID 39890 - Set the Link filter members
		m_strLinkFilter = VarString(m_pHL7LinkFilter->CurSel->GetValue(hlfName), "< All Links >"); 
		m_nLinkFilterID = VarLong(m_pHL7LinkFilter->CurSel->GetValue(hlfLink), -1);
		// (b.savon 2011-10-04 11:22) - PLID 39890 - Update the Unlink List with our new filtered results.
		BindUnlinkList();
	} else{
		m_pHL7LinkFilter->PutComboBoxText(_bstr_t(m_strLinkFilter));
	}
}//END void CUnlinkHL7ThirdPartyID::SelChosenNxDlHl7LinkFilter(LPDISPATCH lpRow)