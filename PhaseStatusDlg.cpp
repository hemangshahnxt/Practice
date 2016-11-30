// PhaseStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PhaseStatusDlg.h"
#include "GlobalDataUtils.h"
#include "GetNewIDName.h"
#include "SelectNewStatusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPhaseStatusDlg dialog

typedef enum
{
	PS_COLUMN_ID = 0,
	PS_COLUMN_SYSTEM,
	PS_COLUMN_STATUS,
	PS_COLUMN_NOTES,
	PS_COLUMN_ISACTIVE
} PS_COLUMN;

CPhaseStatusDlg::CPhaseStatusDlg(CWnd* pParent)
	: CNxDialog(CPhaseStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhaseStatusDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPhaseStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhaseStatusDlg)
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPhaseStatusDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPhaseStatusDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPhaseStatusDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPhaseStatusDlg)
	ON_EVENT(CPhaseStatusDlg, IDC_STATUS_LIST, 10 /* EditingFinished */, OnEditingFinishedStatusList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPhaseStatusDlg, IDC_STATUS_LIST, 9 /* EditingFinishing */, OnEditingFinishingStatusList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPhaseStatusDlg, IDC_STATUS_LIST, 8 /* EditingStarting */, OnEditingStartingStatusList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPhaseStatusDlg message handlers

void CPhaseStatusDlg::OnOK() 
{
	CDialog::OnOK();
}

BOOL CPhaseStatusDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_statusList = BindNxDataListCtrl(IDC_STATUS_LIST);

	// (z.manning, 05/01/2008) - PLID 29864 - Set button styles	
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);
	
	return TRUE;
}

// (j.armen 2012-01-26 11:45) - PLID 47810 - Parameratized Queries
void CPhaseStatusDlg::OnEditingFinishedStatusList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (bCommit)
	try
	{
		if (nCol == PS_COLUMN_STATUS) {
			ExecuteParamSql("UPDATE LadderStatusT SET Name = {STRING} WHERE ID = {INT}", VarString(m_statusList->Value[nRow][nCol]), VarLong(m_statusList->Value[nRow][PS_COLUMN_ID]));	
		}
		else if (nCol == PS_COLUMN_NOTES) {
			ExecuteParamSql("UPDATE LadderStatusT SET Notes = {STRING} WHERE ID = {INT}", VarString(m_statusList->Value[nRow][nCol]), VarLong(m_statusList->Value[nRow][PS_COLUMN_ID]));	
		}
		else if (nCol == PS_COLUMN_ISACTIVE) {
			ExecuteParamSql("UPDATE LadderStatusT SET IsActive = {BOOL} WHERE ID = {INT}", VarBool(varNewValue), VarLong(m_statusList->Value[nRow][PS_COLUMN_ID]));
			//Now go through all ladders with this status, make sure they're synchronized
			_RecordsetPtr rsLadders = CreateParamRecordset("SELECT ID FROM LaddersT WHERE Status = {INT}", VarLong(m_statusList->Value[nRow][PS_COLUMN_ID]));
			while(!rsLadders->eof) {
				PhaseTracking::SyncTodoWithLadder(AdoFldLong(rsLadders, "ID"));
				rsLadders->MoveNext();
			}
		}
		else
		{	ASSERT(FALSE);
			return;
		}
	}
	NxCatchAll("Could not save phase status");
}

// (j.armen 2012-01-26 11:45) - PLID 47810 - Parameratized Queries
void CPhaseStatusDlg::OnEditingFinishingStatusList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if (*pbCommit)
	try
	{
		if (nCol == PS_COLUMN_STATUS) {
			CString strTemp = strUserEntered;
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if(strTemp == "") {
				MsgBox("You cannot enter a blank status");
				*pbContinue = FALSE;
			}
			else if(ReturnsRecordsParam("SELECT Name FROM LadderStatusT WHERE Name = {STRING} AND ID <> {INT}", strTemp, VarLong(m_statusList->GetValue(nRow, PS_COLUMN_ID)))) {
				MsgBox("This status already exists.");
				*pbContinue = FALSE;
			}
		}
	}NxCatchAll("Error in PhaseTracking::OnEditingFinishingStatusList()");

}

void CPhaseStatusDlg::OnEditingStartingStatusList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	if (VarBool(m_statusList->Value[nRow][PS_COLUMN_SYSTEM]))
		*pbContinue	= FALSE;
}

// (j.armen 2012-01-26 11:45) - PLID 47810 - Parameratized Queries
void CPhaseStatusDlg::OnAdd() 
{
	CGetNewIDName dlg(this);
	CString newName;
	
	dlg.m_pNewName = &newName;
	dlg.m_nMaxLength = 50;

	if (dlg.DoModal() == IDOK) 
	{	try
		{
		newName.TrimLeft();
		newName.TrimRight();
		if(newName == "") {
			MsgBox("You cannot enter a blank status");
			return;
		}
		if(ReturnsRecordsParam("SELECT Name FROM LadderStatusT WHERE Name = {STRING}", newName)) {
			MsgBox("This status already exists.");
			return;
		}
		ExecuteParamSql("INSERT INTO LadderStatusT (ID, Name) SELECT {INT}, {STRING}", 
				NewNumber("LadderStatusT", "ID"), newName);
		}
		NxCatchAll("Could not add new name");
	}

	m_statusList->Requery();
}

// (j.armen 2012-01-26 11:43) - PLID 47810 - Parameratized and Refactored
void CPhaseStatusDlg::OnDelete() 
{
	long curSel = m_statusList->CurSel;

	if (curSel != -1) {
		if(VarBool(m_statusList->Value[curSel][PS_COLUMN_SYSTEM])) {
			MsgBox("This is a system-defined status and may not be deleted.");
		}
		else {
			try
			{
				if(MsgBox(MB_YESNO, "Are you sure you want to delete this status?") == IDNO) return;

				CArray<long, long> aryLadderIDs;
				_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM LaddersT WHERE Status = {INT}", VarLong(m_statusList->Value[curSel][PS_COLUMN_ID]));
				while(!prs->eof) {
					aryLadderIDs.Add(AdoFldLong(prs, "ID"));
					prs->MoveNext();
				}
				if(!aryLadderIDs.IsEmpty())	{
					CSelectNewStatusDlg dlg(this);
					dlg.m_nDoomedID = VarLong(m_statusList->Value[curSel][PS_COLUMN_ID]);
					if(dlg.DoModal() == IDCANCEL) {
						return;
					}
					else {
						ExecuteParamSql("UPDATE LaddersT SET Status = {INT} WHERE ID IN ({INTARRAY})", dlg.m_nNewID, aryLadderIDs);
						for(int i = 0; i < aryLadderIDs.GetSize(); i++) {
							PhaseTracking::SyncTodoWithLadder(aryLadderIDs[i]);
						}
					}
				}
				ExecuteParamSql("DELETE FROM LadderStatusT WHERE ID = {INT}", VarLong(m_statusList->Value[curSel][PS_COLUMN_ID]));
			}
			NxCatchAll("Could not delete phase status");
			m_statusList->Requery();
		}
	}
}

void CPhaseStatusDlg::OnCancel()
{
	CDialog::OnCancel();
}