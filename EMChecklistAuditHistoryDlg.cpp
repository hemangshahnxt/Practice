// EMChecklistAuditHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMChecklistAuditHistoryDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-08-24 10:51) - PLID 27152 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum AuditListColumns {

	alcAuditID = 0,
	alcAuditDetailID,
	alcChangedDate,
	alcLocationName,
	alcChangedByName,
	alcPersonName,
	alcItem,
	alcRecordID,
	alcOldValue,
	alcNewValue,
	alcPriority,
	alcType,
};

/////////////////////////////////////////////////////////////////////////////
// CEMChecklistAuditHistoryDlg dialog


CEMChecklistAuditHistoryDlg::CEMChecklistAuditHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMChecklistAuditHistoryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMChecklistAuditHistoryDlg)
		m_nChecklistID = -1;
	//}}AFX_DATA_INIT
}


void CEMChecklistAuditHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMChecklistAuditHistoryDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_COPY_OUTPUT_BTN, m_btnCopyOutput); // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMChecklistAuditHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMChecklistAuditHistoryDlg)
	ON_BN_CLICKED(IDC_COPY_OUTPUT_BTN, OnCopyOutputBtn) // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMChecklistAuditHistoryDlg message handlers

BOOL CEMChecklistAuditHistoryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		
		// (a.walling 2010-06-08 10:02) - PLID 38558 - Audit flags
		EnsureAuditFlags();

		// (c.haag 2008-04-29 17:15) - PLID 29837 - NxIconize the Close button
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnCopyOutput.AutoSet(NXB_EXPORT); // (b.cardillo 2010-01-07 13:26) - PLID 35780

		m_AuditList = BindNxDataList2Ctrl(this, IDC_EM_CHECKLIST_AUDIT_HISTORY_LIST, GetRemoteData(), false);

		// (z.manning 2009-05-22 16:37) - PLID 34330 - We now have a separate permission for the
		// old/new value columns.
		if(!CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
			IColumnSettingsPtr pCol;
			pCol = m_AuditList->GetColumn(alcOldValue);
			pCol->PutColumnStyle(pCol->GetColumnStyle() & (~csVisible));
			pCol = m_AuditList->GetColumn(alcNewValue);
			pCol->PutColumnStyle(pCol->GetColumnStyle() & (~csVisible));
		}
		
		_variant_t var;

		//get all checklist audits for this checklist ID,
		//order by date/ID desc for some manipulation we need to do later

		// (j.jones 2007-09-17 10:11) - PLID 27396 - increased range to 9192 to support new audits
		// (j.jones 2007-09-18 14:24) - PLID 27397 - increased range to 9193 to support new audits
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, AuditDetailsT.*, AuditDetailsT.ID AS AuditDetailID "
						"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
						"WHERE (AuditDetailsT.ItemID >= 9172 AND AuditDetailsT.ItemID <= 9193) "
						"AND RecordID = {INT} ORDER BY ChangedDate DESC, AuditDetailsT.ID DESC", m_nChecklistID);

		if(rs->eof) {
			CString str = "No audit history was found for this E/M Checklist.";
			// (c.haag 2007-09-11 15:55) - PLID 27353 - Changed the message box
			// to use this dialog as the parent rather than the main window
			MessageBox(str, "Practice", MB_OK | MB_ICONERROR);
		}

		while(!rs->eof) {

			var = rs->Fields->Item["ItemID"]->Value;
			long nItemID = VarLong(var);

			//check for if the checklist was deleted
			if(nItemID == 9173) {
				//we are loading in order of most recent items first, and once we
				//hit a "checklist deleted" audit then we know we hit the case
				//where they've previously had a checklist with this ID and deleted it,
				//in which case the remaining records in the recordset are invalid,
				//as they are not really referencing the current checklist
				break;
			}
			
			IRowSettingsPtr pRow = m_AuditList->GetNewRow();

			var = rs->Fields->Item["AuditID"]->Value;
			pRow->PutValue(alcAuditID,var);

			var = rs->Fields->Item["AuditDetailID"]->Value;
			pRow->PutValue(alcAuditDetailID,var);

			var = rs->Fields->Item["ChangedDate"]->Value;
			pRow->PutValue(alcChangedDate,var);

			var = rs->Fields->Item["ChangedAtLocationName"]->Value;
			pRow->PutValue(alcLocationName,var);

			var = rs->Fields->Item["ChangedByUserName"]->Value;
			pRow->PutValue(alcChangedByName,var);

			var = rs->Fields->Item["PersonName"]->Value;
			pRow->PutValue(alcPersonName,var);

			pRow->PutValue(alcItem,_bstr_t(GetAuditItemDescription(nItemID)));

			var = rs->Fields->Item["RecordID"]->Value;
			pRow->PutValue(alcRecordID,var);

			var = rs->Fields->Item["OldValue"]->Value;
			pRow->PutValue(alcOldValue,var);

			var = rs->Fields->Item["NewValue"]->Value;
			pRow->PutValue(alcNewValue,var);

			var = rs->Fields->Item["Priority"]->Value;
			pRow->PutValue(alcPriority,var);

			var = rs->Fields->Item["Type"]->Value;
			pRow->PutValue(alcType,var);

			rs->MoveNext();

			m_AuditList->AddRowSorted(pRow, NULL);
		}
		rs->Close();
		
	}NxCatchAll("Error in CEMChecklistAuditHistoryDlg::OnInitDialog");	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (b.cardillo 2010-01-07 13:26) - PLID 35780 - Added ability to save the current audit results to .csv
void CEMChecklistAuditHistoryDlg::OnCopyOutputBtn()
{
	try {
		PromptSaveFile_CsvFromDLor2(m_AuditList, this, "AuditExport", TRUE);
	} NxCatchAll(__FUNCTION__);
}
