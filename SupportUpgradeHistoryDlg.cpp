// SupportUpgradeHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportUpgradeHistoryDlg.h"

// (f.dinatale 2010-05-24) - PLID 38842 - Added the Version History Dialog
// CSupportUpgradeHistoryDlg dialog

IMPLEMENT_DYNAMIC(CSupportUpgradeHistoryDlg, CNxDialog)

CSupportUpgradeHistoryDlg::CSupportUpgradeHistoryDlg(CWnd* pParent /*=NULL*/, long nClientID /*= -1*/)
	: CNxDialog(CSupportUpgradeHistoryDlg::IDD, pParent)
{
	m_nClientID = nClientID;
}

CSupportUpgradeHistoryDlg::~CSupportUpgradeHistoryDlg()
{

}

void CSupportUpgradeHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	// (f.dinatale 2010-06-16) - PLID 38842 - Close button
	DDX_Control(pDX, IDC_SUPPORT_UPGRADE_HISTORY_CLOSE, m_nxbClose);
}


BEGIN_MESSAGE_MAP(CSupportUpgradeHistoryDlg, CNxDialog) // (a.walling 2011-01-14 13:50) - no PLID - fixed bad base class
	ON_BN_CLICKED(IDC_SUPPORT_UPGRADE_HISTORY_CLOSE, OnClose)
END_MESSAGE_MAP()


// CSupportUpgradeHistoryDlg message handlers

BOOL CSupportUpgradeHistoryDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();
		if(m_nClientID != -1){
			// Store the query
			CString strFromClause;
			strFromClause.Format("(SELECT ReleasedVersionsT.Version AS Version, VersionHistoryT.IncidentID AS IncidentID, UsersT.Username AS Username, "
				"VersionHistoryT.Date AS UpgradeDate FROM VersionHistoryT "
				"LEFT JOIN UsersT ON VersionHistoryT.UserID = UsersT.PersonID "
				"LEFT JOIN ReleasedVersionsT ON VersionHistoryT.VerID = ReleasedVersionsT.ID "
				"WHERE VersionHistoryT.ClientID = %li) AS VersionRecords", m_nClientID);

			// Retrieve a pointer to the Upgrade History datalist.
			CWnd* pWnd = GetDlgItem(IDC_SUPPORT_UPGRADE_HISTORY_LIST);
			m_pUpgradehist = pWnd->GetControlUnknown();

			// Set the query and connection for the datalist
			// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
			// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
			m_pUpgradehist->PutAdoConnection(GetRemoteDataSnapshot());
			m_pUpgradehist->PutFromClause(_bstr_t(strFromClause));

			// Perfect, now let's requery.
			m_pUpgradehist->Requery();
		}
		// Set up the graphical look to match the Patients module
		((CNxColor*) GetDlgItem(IDC_UPGRADEHISTORY_COLOR))->SetColor((GetNxColor(GNC_PATIENT_STATUS, 1)));
		m_nxbClose.AutoSet(NXB_CLOSE);
	}NxCatchAll("Error in CSupportUpgradeHistoryDlg::OnInitDialog()");
	return FALSE;
}

void CSupportUpgradeHistoryDlg::OnClose()
{
	OnCancel();
}
BEGIN_EVENTSINK_MAP(CSupportUpgradeHistoryDlg, CNxDialog) // (a.walling 2011-01-14 13:50) - no PLID - fixed bad base class
	ON_EVENT(CSupportUpgradeHistoryDlg, IDC_SUPPORT_UPGRADE_HISTORY_LIST, 18, CSupportUpgradeHistoryDlg::OnRequeryFinished, VTS_I2)
END_EVENTSINK_MAP()

void CSupportUpgradeHistoryDlg::OnRequeryFinished(short nFlags)
{
	// Highlight the first row because it's guaranteed that it is the current version.
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUpgradehist->GetFirstRow();

		if(pRow){
			pRow->PutBackColor(GetNxColor(GNC_FINANCIAL, 0));
		}
		//No else, if NULL don't do anything.
		else{}
	}NxCatchAll("Error in CSupportUpgradeHistoryDlg::OnRequeryFinished()");
}
