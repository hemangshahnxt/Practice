// CEMRLockReminderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRLockReminderDlg.h"
#include "practice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRLockReminderDlg dialog


CEMRLockReminderDlg::CEMRLockReminderDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRLockReminderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRLockReminderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (z.manning 2008-07-30 10:16) - PLID 30883 - No longer show the count of unlocked EMNs on this dialog.
	//m_nUnlockedAgedEMNs = -1;
}


void CEMRLockReminderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRLockReminderDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_STATUS, m_nxstaticStatus);
	DDX_Control(pDX, IDC_AGED, m_nxstaticAged);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRLockReminderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRLockReminderDlg)
	ON_BN_CLICKED(IDOK, OnManage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRLockReminderDlg message handlers

BOOL CEMRLockReminderDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 12:04) - PLID 29866 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		extern CPracticeApp theApp;

		GetDlgItem(IDC_STATUS)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftTitle));
		GetDlgItem(IDC_AGED)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_brush.CreateSolidBrush(PaletteColor(0x00CCC8D5));

		UpdateText();
	}
	NxCatchAll("Error in CEMRLockReminderDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//static
long CEMRLockReminderDlg::NumUnlockedAgedEMNs(OPTIONAL BOOL bOnlyCheckForAtLeastOne /* = FALSE */)
{
	long nDays = GetRemotePropertyInt("EmnRemindLockDays", 30, 0, GetCurrentUserName(), true);

	CString strField = GetRemindField();

	//TES 9/21/2009 - PLID 33965 - Changed from > to >=
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	CString strFromWhere = FormatString(
		"FROM EMRMasterT "
		"LEFT JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
		"LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID "
		"WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Status <> 2 "
		"	AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL) "
		"	AND DATEDIFF(day, EMRMasterT.%s, GetDate()) >= {INT} "
		"	AND PatientCreatedStatus <> 1 " // (a.walling 2013-05-07 15:23) - PLID 56589 - Like the EMRLockManager, ignore non-finalized patient-created EMNs
		, strField);

	long nNum = 0;
	if(bOnlyCheckForAtLeastOne) {
		if(ReturnsRecordsParam(FormatString("SELECT EmrMasterT.ID %s", strFromWhere), nDays)) {
			nNum = 1;
		}
		else {
			nNum = 0;
		}
	}
	else {
		ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT COUNT(EMRMasterT.ID) AS NumAgedUnlocked %s", strFromWhere), nDays);

		if (!prs->eof) {
			nNum = AdoFldLong(prs, "NumAgedUnlocked", 0);
		}
		else {
			nNum = 0;
		}
	}

	return nNum;
}

// (z.manning 2008-07-30 09:53) - PLID 30883 - Took out this function since we no longer display
// the count of unlocked EMNs on this dialog.
/*
void CEMRLockReminderDlg::SetUnlockedAgedEMNs(long nNum)
{
	m_nUnlockedAgedEMNs = nNum;
	UpdateText();
}
*/

void CEMRLockReminderDlg::UpdateText()
{
	long nDays = GetRemotePropertyInt("EmnRemindLockDays", 30, 0, GetCurrentUserName(), true);

	CString str;
	// (z.manning 2008-07-30 09:51) - PLID 30883 - We no longer show the count of unlocked EMNs 
	// on this dialog since it doesn't respond to table checkers.  However, if we ever do add
	// it back it's still maintained in the m_nUnlockedAgedEMNs variable.
	str.Format("You have unlocked EMNs");
	SetDlgItemText(IDC_STATUS, str);
	//TES 9/21/2009 - PLID 33965 - Changed from "over" to "at least"
	str.Format("at least %li days old.", nDays);
	SetDlgItemText(IDC_AGED, str);
}

void CEMRLockReminderDlg::OnManage() 
{
	GetMainFrame()->ShowLockManager();

	CNxDialog::OnOK();	
}

void CEMRLockReminderDlg::OnCancel() 
{	
	CNxDialog::OnCancel();
}

//static
CString CEMRLockReminderDlg::GetRemindField()
{
	long nFieldNum = GetRemotePropertyInt("EmnRemindField", lrfLastModified, 0, GetCurrentUserName(), true);

	switch (nFieldNum) {
	case lrfLastModified:
		return "ModifiedDate";
		break;
	case lrfInput:
		return "InputDate";
		break;
	default:
		return "ModifiedDate";
		ASSERT(FALSE);
		break;
	}
}
