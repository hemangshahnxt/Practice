// TimedMessageBoxDlg.cpp : implementation file
// (d.thompson 2009-07-08) - PLID 34809
//

#include "stdafx.h"
#include "Practice.h"
#include "TimedMessageBoxDlg.h"


// CTimedMessageBoxDlg dialog
#define IDT_TIMEOUT_UPDATE	1001


IMPLEMENT_DYNAMIC(CTimedMessageBoxDlg, CNxDialog)

CTimedMessageBoxDlg::CTimedMessageBoxDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTimedMessageBoxDlg::IDD, pParent)
{
	m_dwStartTime = 0;
	m_bAllowTimeout = false;
	m_bShowCancel = true;
}

CTimedMessageBoxDlg::~CTimedMessageBoxDlg()
{
}

void CTimedMessageBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DESTROY_MESSAGE, m_staticDestroyMessage);
	DDX_Control(pDX, IDC_USER_MESSAGE, m_staticUserMessage);
}


BEGIN_MESSAGE_MAP(CTimedMessageBoxDlg, CNxDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CTimedMessageBoxDlg message handlers
BOOL CTimedMessageBoxDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Initialize controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Based on the options, hide/show appropriate buttons
		EnableOptionedButtons();


		//if the timout is enabled, turn it on here
		if(m_bAllowTimeout) {
			EnableTimer();
		}

		//Fill the text appropriately
		SetDlgItemText(IDC_USER_MESSAGE, m_strMessageText);

		//Based on the options, reformat the display appropriately
		ReformatDisplay();

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CTimedMessageBoxDlg::OnTimer(UINT_PTR nIDEvent)
{
	try {
		switch(nIDEvent) {
			case IDT_TIMEOUT_UPDATE:
				{
					//Determine the time remaining.
					DWORD dwTicksExpired = GetTickCount() - m_dwStartTime;
					if(dwTicksExpired >= (m_nTimeoutSeconds * 1000)) {
						//We have expired.  Just quit.
						KillTimer(nIDEvent);
						CDialog::OnOK();
					}
					else {
						//Update the interface - truncate to seconds
						UpdateTimerMessage();
					}
				}
				break;
		}

	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnTimer(nIDEvent);
}

//This function should control enabling/showing all buttons based on the options set by the caller.
void CTimedMessageBoxDlg::EnableOptionedButtons()
{
	if(!m_bShowCancel) {
		CWnd *pWnd = GetDlgItem(IDCANCEL);
		pWnd->EnableWindow(FALSE);
		pWnd->ShowWindow(SW_HIDE);
	}
}

//This function ensures defaults are properly set, then starts a timer and all related
//	timer tracking data.
void CTimedMessageBoxDlg::EnableTimer()
{
	//Ensure the m_nTimeoutSeconds is valid.
	if(m_nTimeoutSeconds < 1) {
		m_nTimeoutSeconds = 1;
	}

	//Just set the update time for half a second.  We'll update the dialog and check for completion
	//	that often.
	SetTimer(IDT_TIMEOUT_UPDATE, 500, NULL);
	m_dwStartTime = GetTickCount();

	//Set the message to start
	UpdateTimerMessage();
}

//This function should contain the functionality to reformat the display appropriately.  For example, moving buttons, 
//	resizing controls, resizing the entire dialog, etc.
void CTimedMessageBoxDlg::ReformatDisplay()
{
	//1)  Buttons
	{
		//Currently we have only OK/Cancel.  If cancel is hidden, center OK
		if(!m_bShowCancel) {
			CWnd *pWnd = GetDlgItem(IDOK);
			CRect rcDialog, rcOK;
			pWnd->GetWindowRect(&rcOK);
			GetWindowRect(&rcDialog);
			ScreenToClient(&rcOK);
			ScreenToClient(&rcDialog);

			//determine new position of OK
			long nMoveUnits = (rcDialog.Width() / 2) - (rcOK.Width() / 2) - rcOK.left;
			rcOK.left += nMoveUnits;
			rcOK.right += nMoveUnits;
			pWnd->MoveWindow(&rcOK);
		}
	}

	//2)  Consider resizing the dialog appropriately
}

void CTimedMessageBoxDlg::UpdateTimerMessage()
{
	DWORD dwTicksExpired = GetTickCount() - m_dwStartTime;
	//Display as the next highest second
	unsigned long nSecondsRemaining = (unsigned long)ceil((((double)m_nTimeoutSeconds * 1000.0) - (double)dwTicksExpired)/1000.0);
	CString strMsg;
	strMsg.Format("This message will automatically close in %li seconds.", nSecondsRemaining);
	SetDlgItemText(IDC_DESTROY_MESSAGE, strMsg);
}
