// EditDischargeStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditDischargeStatusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

// (j.jones 2006-11-22 15:57) - PLID 23371 - created

/////////////////////////////////////////////////////////////////////////////
// CEditDischargeStatusDlg dialog


CEditDischargeStatusDlg::CEditDischargeStatusDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditDischargeStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditDischargeStatusDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditDischargeStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDischargeStatusDlg)
	DDX_Control(pDX, IDC_BTN_ADD_DISCHARGE_STATUS, m_btnAddDischargeStatus);
	DDX_Control(pDX, IDC_BTN_DELETE_DISCHARGE_STATUS, m_btnDeleteDischargeStatus);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditDischargeStatusDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditDischargeStatusDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_DISCHARGE_STATUS, OnBtnAddStatus)
	ON_BN_CLICKED(IDC_BTN_DELETE_DISCHARGE_STATUS, OnBtnDeleteStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDischargeStatusDlg message handlers

BOOL CEditDischargeStatusDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 16:00) - PLID 29871 - NxIconify the buttons
		m_btnAddDischargeStatus.AutoSet(NXB_NEW);
		m_btnDeleteDischargeStatus.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);

		m_DischargeStatusList = BindNxDataList2Ctrl(this, IDC_DISCHARGE_STATUS_LIST, GetRemoteData(), true);

	}
	NxCatchAll("Error in CEditDischargeStatusDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDischargeStatusDlg::OnBtnAddStatus() 
{
	try {

		//add to data
		long nNewID = NewNumber("DischargeStatusT", "ID");
		ExecuteSql("INSERT INTO DischargeStatusT (ID, Code, Description) VALUES (%li, '[New Code]', '[New Description]')", nNewID);
		
		//add to list (no need to enforce duplicates)
		IRowSettingsPtr pRow = m_DischargeStatusList->GetNewRow();
		pRow->PutValue(0, nNewID);
		pRow->PutValue(1, "[New Code]");
		pRow->PutValue(2, "[New Description]");
		m_DischargeStatusList->AddRowAtEnd(pRow, NULL);

		// (j.jones 2007-05-02 15:22) - PLID 25883 - send a tablechecker
		CClient::RefreshTable(NetUtils::DischargeStatusT);

	}NxCatchAll("Error in CEditDischargeStatusDlg::OnBtnAddStatus");
}

void CEditDischargeStatusDlg::OnBtnDeleteStatus() 
{
	try {

		IRowSettingsPtr pRow = m_DischargeStatusList->GetCurSel();

		if(pRow == NULL) {
			AfxMessageBox("You must first select a status before deleting.");
		}
		else {

			long nID = VarLong(pRow->GetValue(0),-1);
			
			// (b.eyers 2016-02-22) - PLID 68321 - don't delete if used on an emn
			//ensure it's not in use
			if(ReturnsRecords("SELECT DischargeStatusID FROM BillsT WHERE DischargeStatusID = %li", nID)
				|| ReturnsRecords("SELECT DischargeStatusID FROM CaseHistoryT WHERE DischargeStatusID = %li", nID)
				|| ReturnsRecords("SELECT DischargeStatusID FROM EMRMasterT WHERE DischargeStatusID = %li", nID)) {
				//this intentionally includes deleted bills
				AfxMessageBox("This status is in use on at least one bill or case history or EMN, and cannot be deleted.");
				return;
			}

			if(IDNO == MessageBox("Are you sure you want to delete this status?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			//now delete!
			ExecuteSql("DELETE FROM DischargeStatusT WHERE ID = %li", nID);

			m_DischargeStatusList->RemoveRow(pRow);

			// (j.jones 2007-05-02 15:22) - PLID 25883 - send a tablechecker
			CClient::RefreshTable(NetUtils::DischargeStatusT);
		}

	}NxCatchAll("Error in CEditDischargeStatusDlg::OnBtnDeleteStatus");
}

BEGIN_EVENTSINK_MAP(CEditDischargeStatusDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditDischargeStatusDlg)
	ON_EVENT(CEditDischargeStatusDlg, IDC_DISCHARGE_STATUS_LIST, 9 /* EditingFinishing */, OnEditingFinishingDischargeStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditDischargeStatusDlg, IDC_DISCHARGE_STATUS_LIST, 10 /* EditingFinished */, OnEditingFinishedDischargeStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditDischargeStatusDlg::OnEditingFinishingDischargeStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		//TES 7/31/2007 - PLID 26864 - No need to do any of this if they're cancelling their edit.
		if(*pbCommit) {
			if(strUserEntered != VarString(varOldValue, "")) {

				IRowSettingsPtr pRow(lpRow);
				if(pRow == NULL) {
					return;
				}
				long nID = VarLong(pRow->GetValue(0),-1);

				//see if it is in use
				if(ReturnsRecords("SELECT DischargeStatusID FROM BillsT WHERE DischargeStatusID = %li", nID)
					|| ReturnsRecords("SELECT DischargeStatusID FROM CaseHistoryT WHERE DischargeStatusID = %li", nID)) {
					//this intentionally includes deleted bills				
					if(IDNO == MessageBox("This status is in use on at least one bill or case history. Are you sure you want to modify it?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						*pbContinue = FALSE;
						return;
					}
				}
			}
		}

	}NxCatchAll("Error in CEditDischargeStatusDlg::OnEditingFinishingDischargeStatusList");
}

void CEditDischargeStatusDlg::OnEditingFinishedDischargeStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		//TES 7/31/2007 - PLID 26864 - Don't save their change if they cancelled!
		if(bCommit) {
			long nID = VarLong(pRow->GetValue(0),-1);
			switch(nCol) {
				case 1:	//Code
					ExecuteSql("UPDATE DischargeStatusT SET Code = '%s' WHERE ID = %li", _Q(VarString(varNewValue,"")), nID);
					// (j.jones 2007-05-02 15:22) - PLID 25883 - send a tablechecker
					CClient::RefreshTable(NetUtils::DischargeStatusT);
					break;

				case 2: //Description
					ExecuteSql("UPDATE DischargeStatusT SET Description = '%s' WHERE ID = %li", _Q(VarString(varNewValue,"")), nID);
					// (j.jones 2007-05-02 15:22) - PLID 25883 - send a tablechecker
					CClient::RefreshTable(NetUtils::DischargeStatusT);
					break;
			}
		}
	}NxCatchAll("Error in CEditDischargeStatusDlg::OnEditingFinishedDischargeStatusList");
}
