// InactiveServiceItems.cpp : implementation file
//

#include "stdafx.h"
#include "InactiveServiceItems.h"
#include "Client.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "InvUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInactiveServiceItems dialog


CInactiveServiceItems::CInactiveServiceItems(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInactiveServiceItems::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInactiveServiceItems)
		m_ServiceType = 1;
		m_Changed = FALSE;
	//}}AFX_DATA_INIT
}


void CInactiveServiceItems::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInactiveServiceItems)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_INACTIVE_TITLE_LABEL, m_nxstaticInactiveTitleLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInactiveServiceItems, CNxDialog)
	//{{AFX_MSG_MAP(CInactiveServiceItems)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInactiveServiceItems message handlers

BOOL CInactiveServiceItems::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);

	m_List = BindNxDataListCtrl(this,IDC_INACTIVE_ITEM_LIST,GetRemoteData(),false);
	
	if(m_ServiceType==1) {

		//CPT Codes

		SetWindowText("Inactive Service Codes");

		m_List->FromClause = "ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID";
		
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(0, _T("ServiceT.ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(1, _T("Code"), _T("Service Code"), 70, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(2, _T("Subcode"), _T("Sub"), 35, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(3, _T("Name"), _T("Description"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(4, _T("Price"), _T("Std. Fee"), 100, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;

		m_List->GetColumn(1)->SortPriority = 0;
		m_List->GetColumn(1)->PutSortAscending(TRUE);
		m_List->GetColumn(2)->SortPriority = 1;
		m_List->GetColumn(2)->PutSortAscending(TRUE);

		m_List->Requery();
	}
	else {

		//Inventory Items

		SetWindowText("Inactive Inventory Items");

		m_List->FromClause = "ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID";
		
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(0, _T("ServiceT.ID"), _T("ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(1, _T("ServiceT.Name"), _T("Description"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(2, _T("Price"), _T("Price"), 100, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(3, _T("CategoriesT.Name"), _T("Category"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;

		m_List->GetColumn(1)->SortPriority = 0;
		m_List->GetColumn(1)->PutSortAscending(TRUE);

		m_List->Requery();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInactiveServiceItems::OnOK() 
{
	
	CNxDialog::OnOK();
}

void CInactiveServiceItems::OnCancel() 
{
	
	CNxDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CInactiveServiceItems, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInactiveServiceItems)
	ON_EVENT(CInactiveServiceItems, IDC_INACTIVE_ITEM_LIST, 6 /* RButtonDown */, OnRButtonDownInactiveItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInactiveServiceItems, IDC_INACTIVE_ITEM_LIST, 19 /* LeftClick */, OnLeftClickInactiveItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInactiveServiceItems, IDC_INACTIVE_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellInactiveItemList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInactiveServiceItems::OnRButtonDownInactiveItemList(long nRow, short nCol, long x, long y, long nFlags) 
{
	
}

void CInactiveServiceItems::OnLeftClickInactiveItemList(long nRow, short nCol, long x, long y, long nFlags) 
{
}

void CInactiveServiceItems::RestoreItem(long ID)
{
	try {

		CString str, strDesc, strModule, strServType;

		if(m_ServiceType==1) {
			strModule = "Administrator";
			strServType = "service code";

			strDesc = VarString(m_List->GetValue(m_List->CurSel,1),"") + " " + VarString(m_List->GetValue(m_List->CurSel,2),"")
				+ " - " + VarString(m_List->GetValue(m_List->CurSel,3),"");
		}
		else {
			strModule = "Inventory";
			strServType = "inventory item";

			strDesc = VarString(m_List->GetValue(m_List->CurSel,1),"");
		}

		str.Format("You are about to restore the %s '%s'.\n"
			"This item will become available for editing in the %s module, in billing, and in quotes.\n\n"
			"Are you sure you wish to restore this item to active use?",strServType,strDesc,strModule);

		if(IDYES==MessageBox(str,"Practice",MB_YESNO|MB_ICONQUESTION)) {

			ExecuteSql("UPDATE ServiceT SET Active = 1 WHERE ID = %li",ID);

			m_Changed = TRUE;

			m_List->RemoveRow(m_List->CurSel);

			long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();

			if(m_ServiceType==1) {
				// (a.walling 2007-08-06 12:20) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
				CClient::RefreshTable(NetUtils::CPTCodeT, ID);				
				if(nAuditID != -1)
					AuditEvent(-1, strDesc, nAuditID, aeiCPTActive, ID, "", "<Marked Active>",aepMedium, aetCreated);
			}
			else {
				// (c.haag 2008-04-24 10:06) - PLID 29770 - This product may have been inactivated at a time when
				// its on-hand levels were below the reorder point or consignment levels. We need to ensure that
				// inventory todo alarms exist for this item if that were the case.
				ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT LocationID FROM ProductLocationInfoT WHERE ProductID = {INT}",
					ID);
				while (!prs->eof) {

					// (j.jones 2008-09-16 09:27) - PLID 31380 - EnsureInventoryTodoAlarms now supports multiple products,
					// though in this particular case, it really is only one product, but multiple locations, which the
					// function does not support
					CArray<long, long> aryProductIDs;
					aryProductIDs.Add(ID);
					//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
					InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, AdoFldLong(prs, "LocationID"), false);
					prs->MoveNext();
				}

				// (a.walling 2007-08-06 12:38) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
				CClient::RefreshTable(NetUtils::Products, ID);
				if(nAuditID != -1)
					AuditEvent(-1, strDesc, nAuditID, aeiProductServiceActive, ID, "", "<Marked Active>",aepMedium, aetCreated);
			}
		}
		else
			return;		

	}NxCatchAll("Error restoring item.");
}

void CInactiveServiceItems::OnDblClickCellInactiveItemList(long nRowIndex, short nColIndex) 
{
	//DRT 7/25/03 - Moved here from OnLeftClick
	if(nRowIndex == -1)
		return;

	try {
		long ServiceID = m_List->GetValue(nRowIndex, 0).lVal;
		RestoreItem(ServiceID);
	}NxCatchAll("Error selecting item.");

}
