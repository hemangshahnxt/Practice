// EmrItemLinkingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrItemLinkingDlg.h"
#include "EMRItemLinkingDetailDlg.h"
#include "GlobalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmrItemLinkingDlg dialog


CEmrItemLinkingDlg::CEmrItemLinkingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrItemLinkingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrItemLinkingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEmrItemLinkingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrItemLinkingDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_NEW_ITEM_LINK, m_btnNewItemLink);
	DDX_Control(pDX, IDC_DELETE_ITEM_LINK, m_btnDeleteItemLink);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrItemLinkingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrItemLinkingDlg)
	ON_BN_CLICKED(IDC_NEW_ITEM_LINK, OnNewItemLink)
	ON_BN_CLICKED(IDC_DELETE_ITEM_LINK, OnDeleteItemLink)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemLinkingDlg message handlers

BOOL CEmrItemLinkingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//DRT 1/24/2007 - PLID 24401 - This interface controls creating data for the EMR Item Linking feature.

	try {
		// (c.haag 2008-04-29 17:22) - PLID 29840 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnNewItemLink.AutoSet(NXB_NEW);
		m_btnDeleteItemLink.AutoSet(NXB_DELETE);

		m_pList = BindNxDataList2Ctrl(this, IDC_LIST_ITEM_LINKS, GetRemoteData(), false);

		LoadLinks();

		// TODO: Add extra initialization here
	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

struct EMRLink {
	long nInfoID1;
	long nInfoID2;

	CString strInfo1;
	CString strInfo2;

	EMRLink() {
		nInfoID1 = -1;
		nInfoID2 = -1;
	}
};

void CEmrItemLinkingDlg::LoadLinks()
{
	//empty list to start
	m_pList->Clear();

	//We need to find all the links and put them in a map
	CMap<long, long, EMRLink, EMRLink> mapLinks;

	{
		//DRT 2/8/2007 - We will only allow active EMR Info Items to be displayed.
		ADODB::_RecordsetPtr prsLoad = CreateRecordset("SELECT EMRLinkID, EMRInfoID, EMRInfoT.Name "
			"FROM EMRItemLinkedDataT INNER JOIN EMRDataT ON EMRItemLinkedDataT.EMRDataID = EMRDataT.ID "
			"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
			"WHERE EMRInfoT.ID IN (SELECT ActiveEMRInfoID FROM EMRInfoMasterT) "
			"GROUP BY EMRLinkID, EMRInfoID, EMRInfoT.Name "
			"ORDER BY EMRLinkID");
		while(!prsLoad->eof) {
			long nLinkID = AdoFldLong(prsLoad, "EMRLinkID");
			long nInfoID = AdoFldLong(prsLoad, "EMRInfoID");
			CString strInfoName = AdoFldString(prsLoad, "Name");

			//Does this link already exist in our map?
			EMRLink elLink;
			if(mapLinks.Lookup(nLinkID, elLink)) {
				//Already exists in our map, that means this must be #2
				elLink.nInfoID2 = nInfoID;
				elLink.strInfo2 = strInfoName;

				//Put back in map
				mapLinks.SetAt(nLinkID, elLink);
			}
			else {
				//Not found, we need a new map.  That means we fill the first InfoID only
				elLink.nInfoID1 = nInfoID;
				elLink.strInfo1 = strInfoName;

				//Put in map
				mapLinks.SetAt(nLinkID, elLink);
			}

			prsLoad->MoveNext();
		}
		prsLoad->Close();
	}

	//We now have a map of all links.  We need to "group" this together into just the EMRLink objects
	//	that are unique.  Note that 1 -> 2 is the same as 2 -> 1, there is no order.
	POSITION pos = mapLinks.GetStartPosition();
	while(pos != NULL) {
		long nKey;
		EMRLink elLink;

		mapLinks.GetNextAssoc(pos, nKey, elLink);

		//Now again loop through all items in the map
		BOOL bAnyRemoved = FALSE;
		POSITION loop = mapLinks.GetStartPosition();
		while(loop != NULL) {
			long nLoopKey;
			EMRLink elLoopLink;

			mapLinks.GetNextAssoc(loop, nLoopKey, elLoopLink);

			//don't look at ourself!
			if(nLoopKey != nKey) {
				//Is this the same?
				if(elLink.nInfoID1 == elLoopLink.nInfoID1 && elLink.nInfoID2 == elLoopLink.nInfoID2 ||
					elLink.nInfoID1 == elLoopLink.nInfoID2 && elLink.nInfoID2 == elLoopLink.nInfoID1)
				{
					//Match!  This means the loop version is a duplicate of the main one.  Go ahead and
					//	remove it from the map.
					mapLinks.RemoveKey(nLoopKey);
					bAnyRemoved = TRUE;
				}
			}
		}

		//If anything was removed, our current "pos" iterator is no longer valid, we have to restart from scratch
		if(bAnyRemoved)
			pos = mapLinks.GetStartPosition();
	}

	//At this point, we have a map of unique links.  Put 'em in the datalist
	pos = mapLinks.GetStartPosition();
	while(pos != NULL) {
		long nKey;
		EMRLink elLink;

		mapLinks.GetNextAssoc(pos, nKey, elLink);

		//TES 2/9/2007 - PLID 24671 - We ignore links that only have one side.
		if(elLink.nInfoID1 != -1 && elLink.nInfoID2 != -1) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
			pRow->PutValue(0, (long)elLink.nInfoID1);
			pRow->PutValue(1, _bstr_t(elLink.strInfo1));
			pRow->PutValue(2, (long)elLink.nInfoID2);
			pRow->PutValue(3, _bstr_t(elLink.strInfo2));
			m_pList->AddRowAtEnd(pRow, NULL);
		}
	}
}

void CEmrItemLinkingDlg::OnOK() 
{
	//Just a close button, everything saves immediately
	CNxDialog::OnOK();
}

void CEmrItemLinkingDlg::OnCancel() 
{
	//Just a close button
	OnOK();
}

void CEmrItemLinkingDlg::OnNewItemLink() 
{
	try {
		//Let them enter a new item, requery the list when done.
		CEMRItemLinkingDetailDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			LoadLinks();
		}
	} NxCatchAll("Error in OnNewItemLink");
}

void CEmrItemLinkingDlg::OnDeleteItemLink() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if(pRow == NULL)
			return;

		//Confirm
		if(AfxMessageBox("Are you sure you wish to permanently remove this link?  This action cannot be undone.", MB_YESNO) == IDNO)
			return;

		//Remove this row!
		long nID1 = VarLong(pRow->GetValue(0));
		long nID2 = VarLong(pRow->GetValue(2));

		CDWordArray aryIDsFound;
		CDWordArray aryIDsToDelete;

		//We have to find all link IDs that have these 2 info IDs
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT EMRItemLinkedDataT.EMRLinkID "
			"FROM EMRDataT INNER JOIN EMRItemLinkedDataT ON EMRDataT.ID = EMRItemLinkedDataT.EMRDataID "
			"WHERE EMRDataT.EMRInfoID = %li OR EMRDataT.EMRInfoID = %li", nID1, nID2);
		while(!prs->eof) {
			long nLinkID = AdoFldLong(prs, "EMRLinkID");

			//We need 2 records to delete
			if(IsIDInArray(nLinkID, &aryIDsFound)) {
				//2nd record found
				aryIDsToDelete.Add(nLinkID);
			}
			else {
				//1st time finding, add to found list
				aryIDsFound.Add(nLinkID);
			}

			prs->MoveNext();
		}
		prs->Close();

		//Generate SQL to delete all
		if(aryIDsToDelete.GetSize() > 0) {
			CString strSql;
			for(int i = 0; i < aryIDsToDelete.GetSize(); i++) {
				long nLinkID = aryIDsToDelete.GetAt(i);
				CString str;
				str.Format("DELETE FROM EMRItemLinkedDataT WHERE EmrLinkID = %li;\r\n"
					"DELETE FROM EMRItemLinksT WHERE ID = %li;\r\n", nLinkID, nLinkID);
				strSql += str;
			}

			BEGIN_TRANS("DeleteEMRLinkedDetails") {
				ExecuteSqlStd(strSql);
			} END_TRANS("DeleteEMRLinkedDetails");

			//Requery the list
			LoadLinks();
		}


	} NxCatchAll("Error in OnDeleteItemLink");
}

BEGIN_EVENTSINK_MAP(CEmrItemLinkingDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrItemLinkingDlg)
	ON_EVENT(CEmrItemLinkingDlg, IDC_LIST_ITEM_LINKS, 3 /* DblClickCell */, OnDblClickCellListItemLinks, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrItemLinkingDlg::OnDblClickCellListItemLinks(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		//Quit if no row
		if(lpRow == NULL)
			return;

		//Get the IDs and open for editing
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nInfoID1 = VarLong(pRow->GetValue(0));
		long nInfoID2 = VarLong(pRow->GetValue(2));

		CEMRItemLinkingDetailDlg dlg(this);
		dlg.m_nInfoID1 = nInfoID1;
		dlg.m_nInfoID2 = nInfoID2;
		if(dlg.DoModal() == IDOK) {
			//Changes made, requery
			LoadLinks();
		}

	} NxCatchAll("Error in OnDblClickCellListItemLinks");
}
