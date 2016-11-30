// CustomRecordsTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PatientView.h"
#include "CustomRecordsTabDlg.h"
#include "GlobalUtils.h"
#include "CustomRecordDlg.h"
#include "NxSecurity.h"
#include "PatientsRc.h"
#include "AuditTrail.h"
#include "DeleteEMNConfirmDlg.h"
#include "BillingModuleDlg.h"
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_NEW_EMR		50201
#define ID_EDIT_EMR		50202
#define ID_DELETE_EMR	50203

/////////////////////////////////////////////////////////////////////////////
// CustomRecordsTabDlg dialog


CCustomRecordsTabDlg::CCustomRecordsTabDlg(CWnd* pParent)
	: CPatientDialog(CCustomRecordsTabDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomRecordsTabDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Nextech_Installation/Things_to_Know_Before_you_Install.htm";
	// (c.haag 2004-06-04 10:18) - Always reset member pointers to null!
	m_pEMRDlg = NULL;
}


void CCustomRecordsTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordsTabDlg)
	DDX_Control(pDX, IDC_NEW_EMR, m_btnNewEMR);
	DDX_Control(pDX, IDC_BKG, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomRecordsTabDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustomRecordsTabDlg)
	ON_BN_CLICKED(IDC_NEW_EMR, OnNewEmr)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CustomRecordsTabDlg message handlers

BOOL CCustomRecordsTabDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_EMRList = BindNxDataListCtrl(IDC_CUSTOM_RECORDS_LIST,false);
		SecureControls();

		m_BillingDlg = m_pParent->GetBillingDlg();

	} NxCatchAll("Error initializing Patient Custom Record Dialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomRecordsTabDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
	m_btnNewEMR.AutoSet(NXB_NEW);

	CPatientDialog::SetColor(nNewColor);
}

void CCustomRecordsTabDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{

	m_ID = GetActivePatientID();

	CString str;

	str.Format("EMRMasterT.Deleted = 0 AND PatientID = %li",m_ID);

	m_EMRList->PutWhereClause(_bstr_t(str));
	m_EMRList->Requery();
}

void CCustomRecordsTabDlg::OnNewEmr() {

	try {
		// (j.jones 2007-05-15 17:00) - PLID 25431 - you can't create an EMR
		// without Create and Write permissions, this function cleanly checks for both
		// with only one password prompt or denial message
		if(CheckHasEMRCreateAndWritePermissions()) {
		//if(CheckCurrentUserPermissions(bioPatientEMR, sptCreate)) {
			// (j.armen 2013-05-08 13:04) - PLID 56603 - Added params to the constructor
			CCustomRecordDlg dlg(this, GetActivePatientID(), -1, m_BillingDlg);
			dlg.DoModal();
			UpdateView();
		}
	}NxCatchAll("Error in OnNewEmr");
}

BEGIN_EVENTSINK_MAP(CCustomRecordsTabDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCustomRecordsTabDlg)
	ON_EVENT(CCustomRecordsTabDlg, IDC_CUSTOM_RECORDS_LIST, 6 /* RButtonDown */, OnRButtonDownEmrList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordsTabDlg, IDC_CUSTOM_RECORDS_LIST, 3 /* DblClickCell */, OnDblClickCellEmrList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCustomRecordsTabDlg::OnRButtonDownEmrList(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu pMenu;
	m_EMRList->CurSel = nRow;

	pMenu.CreatePopupMenu();
	if(nRow == -1) {
		// (j.jones 2007-05-15 17:00) - PLID 25431 - you can't create an EMR
		// without Create and Write permissions
		if((GetCurrentUserPermissions(bioPatientEMR) & SPT____C_______ANDPASS) &&
			(GetCurrentUserPermissions(bioPatientEMR) & SPT___W________ANDPASS)) {
			pMenu.InsertMenu(0, MF_BYPOSITION, ID_NEW_EMR, "Create New Record");
		}
	}
	else {
		if(GetCurrentUserPermissions(bioPatientEMR) & SPT___W________ANDPASS) {
			pMenu.InsertMenu(0, MF_BYPOSITION, ID_EDIT_EMR, "Edit Record");
			
			// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
			// it is now administrator-only
			if(IsCurrentUserAdministrator()) {
				pMenu.InsertMenu(1, MF_BYPOSITION, ID_DELETE_EMR, "Delete Record");
			}
		}
	}

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);	
}

BOOL CCustomRecordsTabDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
	case ID_NEW_EMR:
		OnNewEmr();
		break;
	case ID_EDIT_EMR: {
		try {
			// (j.armen 2013-05-08 13:04) - PLID 56603 - Added params to the constructor
			CCustomRecordDlg dlg(this, GetActivePatientID(), VarLong(m_EMRList->GetValue(m_EMRList->GetCurSel(),elcID)), m_BillingDlg);
			dlg.DoModal();
			UpdateView();
		}NxCatchAll("Error editing custom record.");
		break;
	}
	case ID_DELETE_EMR:
		try {
			// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
			// it is now administrator-only
			if(IsCurrentUserAdministrator()) {

				//TES 2/4/2005 - If this is the only record for this EmrGroupsT record, delete that as well.
				long nEmrMasterID = VarLong(m_EMRList->GetValue(m_EMRList->GetCurSel(),elcID));
				long nEmrGroupID = VarLong(m_EMRList->GetValue(m_EMRList->GetCurSel(),elcEmrGroupID));
				_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(*) AS NumEmns, "
					"EMRGroupsT.Description "
					"FROM EMRMasterT "
					"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
					"WHERE EMRMasterT.Deleted = 0 AND EMRGroupsT.ID = {INT} "
					"GROUP BY EMRGroupsT.ID, EMRGroupsT.Description", nEmrGroupID);
				if(rsCount->eof) {
					break;
				}

				// (j.jones 2009-10-01 12:22) - PLID 30479 - added the EMR's deletion
				// confirmation dialog to custom records
				CDeleteEMNConfirmDlg dlg(ecdtCustomRecord, AdoFldString(rsCount, "Description", ""), this);
				if(dlg.DoModal() != DELETE_EMN_RETURN_DELETE) {
					//unless the return value specifically says to delete, leave now
					break;
				}

				if(AdoFldLong(rsCount, "NumEmns") == 1) {
					DeleteCustomRecord(nEmrGroupID);
				}
				else {
					DeleteEMN(nEmrMasterID);
				}
				
				CString strAuditName = "Deleted '";
				strAuditName += VarString(m_EMRList->GetValue(m_EMRList->CurSel, elcProcedureName),"") + "'";
				AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiPatientEMR, VarLong(m_EMRList->GetValue(m_EMRList->CurSel,elcID)), "", strAuditName, aepHigh, aetDeleted);

				m_EMRList->Requery();
			}
		}NxCatchAll("Error deleting custom record.");
		break;

	}
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CCustomRecordsTabDlg::OnDblClickCellEmrList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex==-1)
		return;

	try {
		// (j.armen 2013-05-08 13:04) - PLID 56603 - Added params to the constructor
		CCustomRecordDlg dlg(this, GetActivePatientID(), VarLong(m_EMRList->GetValue(m_EMRList->GetCurSel(),elcID)), m_BillingDlg);
		dlg.DoModal();
		UpdateView();
	} NxCatchAll("Error editing custom record.");
}

void CCustomRecordsTabDlg::SecureControls()
{
	// Return if we have write access
	if (!(GetCurrentUserPermissions(bioPatientEMR) & SPT___W________ANDPASS)) {
		// Make the controls read-only
		GetDlgItem(IDC_CUSTOM_RECORDS_LIST)->EnableWindow(FALSE);
	}
	if (!(GetCurrentUserPermissions(bioPatientEMR) & SPT____C_______ANDPASS)) {
		GetDlgItem(IDC_CUSTOM_RECORDS_LIST)->EnableWindow(FALSE);
	}
}

CBillingModuleDlg * CCustomRecordsTabDlg::GetBillingDlg()
{
	return m_BillingDlg;
}

LRESULT CCustomRecordsTabDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {
	try {
		switch(wParam) {
		case NetUtils::PatientBDate:
			try {
				if((long)lParam == GetActivePatientID()) {
					//this patients bdate has changed.  Update the list
					UpdateView();
				}
			} NxCatchAll("Error in CCustomRecordsTabDlg::OnTableChanged:PatientBDate");
			break;
		
		case NetUtils::EMRMasterT:
			try {
				// (j.jones 2006-04-06 12:22) - only refresh if the tablechecker is for this patient,
				// or all patients
				if(lParam == GetActivePatientID() || lParam == -1) {
					UpdateView();
				}
			} NxCatchAll("Error in CCustomRecordsTabDlg::OnTableChanged:EMRMasterT");
			break;
		}

		// (c.haag 2004-06-01 13:54) - We used to do this if it was a
		// birthdate message; now we do it every time.

		//we also need to update the dialog if it's open
		if(m_pEMRDlg)
			m_pEMRDlg->PostMessage(WM_TABLE_CHANGED, wParam, lParam);

	} NxCatchAll("Error in CCustomRecordsTabDlg::OnTableChanged");

	return 0;
}

//TES 12/29/2006 - PLID 23400 - This function appears to never be called.
/*void CCustomRecordsTabDlg::OpenEMR(int nEmrID)
{
	long nRowIndex = m_EMRList->FindByColumn(elcID, (long)nEmrID, 0, true);
	if(nRowIndex==-1)
		return;

	try {
		CCustomRecordDlg dlg;
		dlg.m_ID = VarLong(m_EMRList->GetValue(nRowIndex,elcID));
		dlg.m_PatID = GetActivePatientID();
		dlg.m_BillingDlg = m_BillingDlg;
		dlg.DoModal();
		UpdateView();
	} NxCatchAll("Error editing custom record.");
}*/