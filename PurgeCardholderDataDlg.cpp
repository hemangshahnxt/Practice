// PurgeCardholderDataDlg.cpp : implementation file
// (d.thompson 2010-03-15) - PLID 37729 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "PurgeCardholderDataDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"


// CPurgeCardholderDataDlg dialog

IMPLEMENT_DYNAMIC(CPurgeCardholderDataDlg, CNxDialog)

CPurgeCardholderDataDlg::CPurgeCardholderDataDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPurgeCardholderDataDlg::IDD, pParent)
{

}

CPurgeCardholderDataDlg::~CPurgeCardholderDataDlg()
{
}

void CPurgeCardholderDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PURGE_DATE, m_datePurge);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PURGE_PAN, m_btnPAN);
	DDX_Control(pDX, IDC_PURGE_EXP_DATE, m_btnExpDate);
	DDX_Control(pDX, IDC_PURGE_NAMEONCARD, m_btnCardholderName);
}


BEGIN_MESSAGE_MAP(CPurgeCardholderDataDlg, CNxDialog)
END_MESSAGE_MAP()


// CPurgeCardholderDataDlg message handlers
BOOL CPurgeCardholderDataDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Set the datetime picker to today
		m_datePurge.SetValue(COleDateTime::GetCurrentTime());

		//setup interface buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CPurgeCardholderDataDlg::OnOK()
{
	try {
		//Get the date we're purging from
		COleDateTime dtPurge = m_datePurge.GetValue();

		//Figure out just what we're purging
		CString strDataToPurge;
		if(IsDlgButtonChecked(IDC_PURGE_PAN)) {
			//SecurePAN is the encrypted PAN, but CCNumber is the 'last 4'.  If we don't clear both, it would still show up
			//	on receipts and the like despite being "purged".
			// (a.walling 2010-03-15 14:40) - PLID 37751 - Clear out the KeyIndex as well.
			strDataToPurge += "SecurePAN = NULL, KeyIndex = NULL, CCNumber = ''";
		}
		if(IsDlgButtonChecked(IDC_PURGE_EXP_DATE)) {
			if(!strDataToPurge.IsEmpty()) {
				strDataToPurge += ", ";
			}

			strDataToPurge += "CCExpDate = NULL";
		}
		if(IsDlgButtonChecked(IDC_PURGE_NAMEONCARD)) {
			if(!strDataToPurge.IsEmpty()) {
				strDataToPurge += ", ";
			}

			strDataToPurge += "CCHoldersName = ''";
		}

		//Ensure we're doing something
		if(strDataToPurge.IsEmpty()) { 
			AfxMessageBox("You must select a data set to purge before continuing.");
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you absolutely sure you wish to purge this data?  This action is unrecoverable!", MB_YESNO) != IDYES) {
			return;
		}

		//Do the purge
		{
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(-1);
			ExecuteParamSql(
				(LPCTSTR)FormatString("UPDATE PaymentPlansT SET %s WHERE ID IN (SELECT ID FROM LineItemT WHERE Date < {STRING})", strDataToPurge),
				FormatDateTimeForSql(dtPurge, dtoDate));
		}

		//Audit
		long nAuditID = BeginNewAuditEvent();
		if(IsDlgButtonChecked(IDC_PURGE_PAN)) {
			AuditEvent(-1, "", nAuditID, aeiPurgeCardholderPAN, -1, FormatString("Payments before %s", FormatDateTimeForInterface(dtPurge, NULL, dtoDate)), "Purged", aepHigh, aetDeleted);
		}
		if(IsDlgButtonChecked(IDC_PURGE_EXP_DATE)) {
			AuditEvent(-1, "", nAuditID, aeiPurgeCardholderExpDate, -1, FormatString("Payments before %s", FormatDateTimeForInterface(dtPurge, NULL, dtoDate)), "Purged", aepHigh, aetDeleted);
		}
		if(IsDlgButtonChecked(IDC_PURGE_NAMEONCARD)) {
			AuditEvent(-1, "", nAuditID, aeiPurgeCardholderNameOnCard, -1, FormatString("Payments before %s", FormatDateTimeForInterface(dtPurge, NULL, dtoDate)), "Purged", aepHigh, aetDeleted);
		}

		//If we got here, success
		AfxMessageBox("Data has been purged successfully.");
		CDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CPurgeCardholderDataDlg::OnCancel()
{
	try {
		CDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}
