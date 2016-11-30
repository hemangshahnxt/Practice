// EMRAlertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "EMRAlertDlg.h"
#include "SendMessageDlg.h"
#include "EmrTreeWnd.h"
#include "EmrColors.h"
#include "EMR.h"

// (a.walling 2008-06-11 10:15) - PLID 22049 - Modeless alert dialog for EMR messages (created for multi-user)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRAlertDlg dialog


// (j.jones 2013-05-16 14:39) - PLID 56596 - m_wtInfo is now a reference, needs to be created here
CEMRAlertDlg::CEMRAlertDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRAlertDlg::IDD, pParent),
	m_wtInfo(*(new CWriteTokenInfo()))
{
	//{{AFX_DATA_INIT(CEMRAlertDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nEmnID = -1;
	m_bIsTemplate = FALSE;
	m_bTryAcquireWriteAccess = FALSE;
}

CEMRAlertDlg::~CEMRAlertDlg()
{
	try {

		// (j.jones 2013-05-16 14:39) - PLID 56596 - m_wtInfo is now a reference, it is never null,
		// and always is filled in the constructor, and must be cleared here
		delete &m_wtInfo;

	}NxCatchAll(__FUNCTION__);
}


void CEMRAlertDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRAlertDlg)
	DDX_Control(pDX, IDC_CHECK_EXPROPRIATE, m_nxbForce);
	DDX_Control(pDX, IDC_RETRY_WRITE_ACCESS, m_nxibRetry);
	DDX_Control(pDX, IDC_EMR_ALERT_TEXT, m_nxsAlertText);
	DDX_Control(pDX, IDC_SEND_YAK, m_nxibSendYak);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	DDX_Control(pDX, IDC_NXCOLOR_EMRALERT, m_background);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRAlertDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRAlertDlg)
	ON_BN_CLICKED(IDC_SEND_YAK, OnSendYak)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RETRY_WRITE_ACCESS, OnRetryWriteAccess)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRAlertDlg message handlers

void CEMRAlertDlg::SetText(const CString& strText)
{
	m_strText = strText;
	if (m_nxsAlertText.GetSafeHwnd()) {
		m_nxsAlertText.SetWindowText(ConvertToControlText(m_strText));
	}
}

void CEMRAlertDlg::SetInfo(const CWriteTokenInfo& wtInfo, long nEmnID, BOOL bIsTemplate, const CString& strTitle, BOOL bTryAcquireWriteAccessButton)
{
	m_wtInfo = wtInfo;
	m_nEmnID = nEmnID;
	m_strEmnTitle = strTitle;
	m_bIsTemplate = bIsTemplate;
	m_bTryAcquireWriteAccess = bTryAcquireWriteAccessButton;
}

BOOL CEMRAlertDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (a.walling 2008-08-25 14:15) - PLID 
		m_nxsAlertText.SetWindowText(ConvertToControlText(m_strText));

		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_background.SetColor(EmrColors::Topic::PatientBackground());

		// (j.jones 2011-11-11 16:59) - PLID 45956 - if the EMN is opened by a device, you cannot pracyak them
		// (c.haag 2012-06-11) - PLID 50806 - Prevent yakking, but allow for forced overrides. I removed the code
		// that hid the force override checkbox.
		// (j.armen 2013-05-14 11:40) - PLID 56680 - We now just track the external status
		if(m_wtInfo.bIsExternal) {
			m_nxibSendYak.EnableWindow(FALSE);
			m_nxibSendYak.ShowWindow(SW_HIDE);
		}
		
		// (a.walling 2008-06-11 11:11) - PLID 22049 - I was going to disable if the yakker is unavailable, but it is possible
		// they enable it while the window is up. So rather than deal with handling messages to know if the yakker was enabled
		// while we are around, we'll just change the default button, and give a message box when they actually click.
		CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
		if (dlg == NULL || m_wtInfo.bIsExternal) {
			m_nxibCancel.SetFocus();
		} else {
			m_nxibSendYak.SetFocus();
		}

		m_nxbForce.EnableWindow(CanForce());

		if (!m_bTryAcquireWriteAccess) {
			m_nxibRetry.ShowWindow(SW_HIDE);
			m_nxibRetry.EnableWindow(FALSE);
			m_nxbForce.ShowWindow(SW_HIDE);
			m_nxbForce.EnableWindow(FALSE);
		}
	} NxCatchAll("Error in CEMRAlertDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRAlertDlg::OnCancel() 
{
	try {
		CNxDialog::OnCancel();

		DestroyWindow();
	} NxCatchAll("Error in CEMRAlertDlg::OnCancel");
}

void CEMRAlertDlg::OnSendYak() 
{
	try {

		//TES 1/12/2009 - PLID 32525 - Make sure they can access the PracYakker before bringing it up.
		// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
		if (!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm") || !CheckCurrentUserPermissions(bioPracYakker, sptView)) {
			return;
		}
		CWnd* pOwner = GetOwner();
		ASSERT(pOwner);
		ASSERT(pOwner->IsKindOf(RUNTIME_CLASS(CEmrTreeWnd)));
		
		CEmrTreeWnd* pTreeWnd = (CEmrTreeWnd*)pOwner;

		long nPatientID = pTreeWnd->GetPatientID();

		CDWordArray dwRecipients;
		dwRecipients.Add(m_wtInfo.nHeldByUserID);

		CStringArray saRecipients;
		saRecipients.Add(m_wtInfo.strHeldByUserName);

		CString strMessage;
		if (m_bTryAcquireWriteAccess) {
			strMessage.Format("Please release write access for the %s '%s' as soon as possible.", m_bIsTemplate ? "template" : "EMN", m_strEmnTitle);
		} else {
			strMessage.Format("I am working in the %s '%s'! Please exit without saving so I can continue.", m_bIsTemplate ? "template" : "EMN", m_strEmnTitle);
		}

		CSendMessageDlg* dlg = GetMainFrame()->m_pdlgSendYakMessage;
		if (dlg)
			// (j.gruber 2010-07-16 14:04) - PLID 39463 - added threadID
			dlg->PopUpMessage(-1, nPatientID, &dwRecipients, &saRecipients, strMessage, -1, "Urgent");
		
		DestroyWindow();
	} NxCatchAll("Error in CEMRAlertDlg::OnSendYak");
}

void CEMRAlertDlg::OnDestroy() 
{
	try {
		CWnd* pOwner = GetOwner();
		ASSERT(pOwner);
		ASSERT(pOwner->IsKindOf(RUNTIME_CLASS(CEmrTreeWnd)));
		if (pOwner) {
			pOwner->SendMessage(NXM_WINDOW_CLOSING, (WPARAM)this, (LPARAM)m_nEmnID);
		}
		CNxDialog::OnDestroy();

		delete this;
	} NxCatchAll("Error in CEMRAlertDlg::OnDestroy");
}

void CEMRAlertDlg::OnRetryWriteAccess() 
{	
	try {
		if (!m_bTryAcquireWriteAccess)
			return;

		BOOL bForce = m_nxbForce.GetCheck() == BST_CHECKED;

		if (bForce && !CanForce()) {
			try {
				ThrowNxException("An attempt to compromise security has been detected");
			} NxCatchAll("Error in CEMRAlertDlg::OnRetryWriteAccess()");

			// and yet, allow them to continue, just without forcing.
			bForce = FALSE;
		}

		if (bForce) {			
			if (!CheckCurrentUserPermissions(bioPatientEMR, sptDynamic1)) {
				//|| (IDNO == MessageBox("By forcing another user to release write access, they will be unable to save their changes. This should only be used as a last resort! Are you certain you wish to continue?", NULL, MB_YESNO | MB_ICONSTOP))) {
				return;
			}
		}

		CWnd* pOwner = GetOwner();
		ASSERT(pOwner);
		ASSERT(pOwner->IsKindOf(RUNTIME_CLASS(CEmrTreeWnd)));
		if (pOwner) {

			// (c.haag 2012-06-11) - PLID 50806 - If we cannot interact with the user, there's no point in waiting for them
			// to release the EMN because they can't receive yaks. So, take the chart immediately in those cases.
			if (bForce & m_wtInfo.bIsExternal) {
				// We still want them to confirm that they want to forceably take the EMN from the user.
				if (IDOK != DoForcedWriteAcquisitionPrompt(this, m_wtInfo)) {
					return;
				} else {
					// Force the EMN to reload because it has almost certainly changed. If we fail, just defer to the old logic
					// because there's nothing else we can do.
					CEmrTreeWnd* pTreeWnd = (CEmrTreeWnd*)pOwner;
					CEMR* pEMR = pTreeWnd->GetEMR();
					if (NULL != pEMR) {
						CEMN* pEMN = pEMR->GetEMNByID(m_nEmnID);
						if (NULL != pEMN) {
							pTreeWnd->ReloadEMN(pEMN, TRUE /* If FALSE then the EMR window shows nothing */);
						}
					}
				}
			}
			LPARAM lParam = (m_wtInfo.bIsExternal) ? MAKELPARAM(bForce ? 1 : 0, bForce ? 1 : 0) : MAKELPARAM(0, bForce ? 1 : 0);
			pOwner->PostMessage(NXM_TRY_ACQUIRE_WRITE_ACCESS, (WPARAM)m_nEmnID, lParam);
		}

		OnCancel();
	} NxCatchAll("Error in CEMRAlertDlg::OnRetryWriteAccess");
}

BOOL CEMRAlertDlg::CanForce()
{
	// for now we'll just use whether we are an admin or not
	// (a.walling 2008-06-17 17:33) - PLID 30356
	if (IsCurrentUserAdministrator() || CheckCurrentUserPermissions(bioPatientEMR, sptDynamic1, FALSE, 0, TRUE, TRUE) ) {
		return TRUE;
	} else {
		return FALSE;
	}
}