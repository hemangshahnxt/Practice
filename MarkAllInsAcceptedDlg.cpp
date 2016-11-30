// MarkAllInsAcceptedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MarkAllInsAcceptedDlg.h"
#include "AuditTrail.h"

// CMarkAllInsAcceptedDlg dialog

// (j.jones 2010-07-30 14:09) - PLID 39917 - created

IMPLEMENT_DYNAMIC(CMarkAllInsAcceptedDlg, CNxDialog)

using namespace ADODB;

CMarkAllInsAcceptedDlg::CMarkAllInsAcceptedDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMarkAllInsAcceptedDlg::IDD, pParent)
{

}

CMarkAllInsAcceptedDlg::~CMarkAllInsAcceptedDlg()
{
}

void CMarkAllInsAcceptedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ACCEPT_ALL, m_btnAcceptAll);
	DDX_Control(pDX, IDC_BTN_ACCEPT_NONE, m_btnAcceptNone);
}


BEGIN_MESSAGE_MAP(CMarkAllInsAcceptedDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ACCEPT_ALL, OnBtnAcceptAll)
	ON_BN_CLICKED(IDC_BTN_ACCEPT_NONE, OnBtnAcceptNone)
END_MESSAGE_MAP()

// CMarkAllInsAcceptedDlg message handlers
BOOL CMarkAllInsAcceptedDlg::OnInitDialog() 
{	
	try {

		CNxDialog::OnInitDialog();

		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAcceptAll.AutoSet(NXB_OK);	//we want the green checkbox
		m_btnAcceptNone.AutoSet(NXB_DELETE);
		//force the color to be red
		m_btnAcceptNone.SetTextColor(RGB(255,0,0));

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarkAllInsAcceptedDlg::OnBtnAcceptAll()
{
	try {

		if(IDNO == MessageBox("This will mark all insurance companies as being accepted by all providers.\n"
			"HCFA Box 27, Accept Assignment, will send 'Yes'.\n\n"
			"Are you absolutely SURE you wish to do this?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		CString strSqlBatch;

		_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM ProvidersT");
		while(!rs->eof) {
			long nProviderID = rs->Fields->Item["PersonID"]->Value.lVal;
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceAcceptedT SET Accepted = 1 WHERE ProviderID = %li", nProviderID);
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) "
				"SELECT PersonID, %li, 1 FROM InsuranceCoT WHERE PersonID NOT IN ("
				"SELECT InsuranceCoID FROM InsuranceAcceptedT WHERE ProviderID = %li"
				")", nProviderID, nProviderID);
			rs->MoveNext();
		}
		rs->Close();

		if(!strSqlBatch.IsEmpty()) {
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(1000);
			ExecuteSqlBatch(strSqlBatch);

			// (j.jones 2010-07-23 10:06) - PLID 25217 - audit that this button was clicked
			long AuditID = BeginNewAuditEvent();
			AuditEvent(-1, "<All Companies>", AuditID, aeiInsCoAccepted, -1, "", "Added Accept Assignment to all Insurance Companies & Providers", aepHigh, aetChanged);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CMarkAllInsAcceptedDlg::OnBtnAcceptNone()
{
	try {

		if(IDNO == MessageBox("This will mark all insurance companies as not being accepted by any providers.\n"
			"HCFA Box 27, Accept Assignment, will send 'No'.\n\n"
			"Are you absolutely SURE you wish to do this?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		CString strSqlBatch;

		// (j.jones 2010-07-23 15:18) - PLID 39783 - changed this code to ensure
		// we always try to fill InsuranceAcceptedT
		_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM ProvidersT");
		while(!rs->eof) {
			long nProviderID = rs->Fields->Item["PersonID"]->Value.lVal;
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceAcceptedT SET Accepted = 0 WHERE ProviderID = %li", nProviderID);
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) "
				"SELECT PersonID, %li, 0 FROM InsuranceCoT WHERE PersonID NOT IN ("
				"SELECT InsuranceCoID FROM InsuranceAcceptedT WHERE ProviderID = %li"
				")", nProviderID, nProviderID);
			rs->MoveNext();
		}
		rs->Close();

		if(!strSqlBatch.IsEmpty()) {
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(1000);
			ExecuteSqlBatch(strSqlBatch);

			// (j.jones 2010-07-23 10:06) - PLID 25217 - audit that this button was clicked
			long AuditID = BeginNewAuditEvent();
			AuditEvent(-1, "<All Companies>", AuditID, aeiInsCoAccepted, -1, "", "Removed Accept Assignment from all Insurance Companies & Providers", aepHigh, aetChanged);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}