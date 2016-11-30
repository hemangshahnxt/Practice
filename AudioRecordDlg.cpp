// AudioRecordDlg.cpp : implementation file
//
// (c.haag 2006-09-22 17:43) - PLID 21327 - Initial implementation

#include "stdafx.h"
#include "practice.h"
#include "AudioRecordDlg.h"
#include "NxMessageDef.h"
#include "mergeengine.h"
#include "globalutils.h"
#include "FileUtils.h"
#include "EmrTreeWnd.h"
#include "EMN.h"

#define NXT_PLAYBACK			0x3020

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioRecordDlg dialog

// (a.walling 2010-04-13 14:18) - PLID 36821 - Create a modeless audio record dialog, or bring up the existing one if it exists
void CAudioRecordDlg::DoAudioRecord(CWnd* pParent, long nPatientID, bool bIsPatient, long nPicID, CEMN* pEmn)
{
	if (m_pCurrentInstance != NULL) {
		if (m_pCurrentInstance->GetPatientID() != nPatientID || m_pCurrentInstance->GetPicID() != nPicID) {
			pParent->MessageBox("Another recording is already in progress.", NULL, MB_ICONEXCLAMATION);
		} else {
			m_pCurrentInstance->BringToTop();
			return;
		}
	}

	m_pCurrentInstance = new CAudioRecordDlg(pParent);
	m_pCurrentInstance->m_nPatientID = nPatientID;
	m_pCurrentInstance->m_bIsPatient = bIsPatient;
	m_pCurrentInstance->m_nPicID = nPicID;
	m_pCurrentInstance->m_pEmn = pEmn;

	//(e.lally 2012-05-02) PLID 50134 - Pass in the parent
	m_pCurrentInstance->Create(IDD_AUDIO_RECORD_DIALOG, pParent);
	m_pCurrentInstance->ShowWindow(SW_SHOW);
	m_pCurrentInstance->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
}

CAudioRecordDlg* CAudioRecordDlg::GetCurrentInstance()
{
	return m_pCurrentInstance;
}

CAudioRecordDlg* CAudioRecordDlg::m_pCurrentInstance = NULL;

CAudioRecordDlg::CAudioRecordDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAudioRecordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioRecordDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nPatientID = -1;
	m_bIsPatient = false;
	m_nPicID = -1;
	m_pEmn = NULL; // (a.walling 2010-07-27 16:40) - PLID 39433 - Support attaching to an EMN
	m_bIsPlaying = FALSE;
	m_bIsPaused = FALSE;
	m_nTotalTime = 0;
	m_nTotalBytes = 0;
	// Designate the location for the audio capture object to actually record
	// to. We will move the file to the location specified by m_strAudioFileName
	// when the user clicks the OK button
	m_strTempOutputFileName = GetNxTempPath() ^ "tmpRecord.wav";
}

// (a.walling 2010-07-27 16:24) - PLID 39433 -  Helpers
void CAudioRecordDlg::BringToTop()
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(&wp)) {
		if (IsIconic()) {
			if (wp.flags & WPF_RESTORETOMAXIMIZED) {
				wp.showCmd = SW_MAXIMIZE;
			} else {
				wp.showCmd = SW_RESTORE;
			}
			SetWindowPlacement(&wp);
		}
	}
	BringWindowToTop();
	SetForegroundWindow();
}

bool CAudioRecordDlg::IsEmpty()
{
	return m_nTotalTime == 0 && m_nTotalBytes == 0;
}


void CAudioRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	// (a.walling 2008-10-27 08:27) - PLID 31827 - CWnd m_wndWMP for windowless activex control
	// DDX_Control will handle AttachControlSite if the hosted control is windowless.
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioRecordDlg)
	DDX_Control(pDX, IDC_SLIDER, m_Progress);
	DDX_Control(pDX, IDC_BTN_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BTN_REC, m_btnRec);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_STATIC_AUDIO_STATUS, m_nxstaticAudioStatus);
	DDX_Control(pDX, IDC_STATIC_REC_POS, m_nxstaticRecPos);
	DDX_Control(pDX, IDC_STATIC_REC_LENGTH, m_nxstaticRecLength);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_WMP, m_wndWMP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioRecordDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAudioRecordDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_REC, OnBtnRec)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_BN_CLICKED(IDC_BTN_PLAY, OnBtnPlay)
	ON_BN_CLICKED(IDC_BTN_PAUSE, OnBtnPause)
	ON_MESSAGE(NXMEVM_WMP_PLAY_STATE_CHANGE, OnWMPPlayStateChange)
	ON_MESSAGE(NXM_AUDIO_RECORDING_TERMINATED, OnRecordingTerminated)
	ON_MESSAGE(NXM_AUDIO_RECORDING_PROGRESS, OnRecordingProgress)
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

long CAudioRecordDlg::GetPatientID() const
{
	return m_nPatientID;
}

//void CAudioRecordDlg::SetPatientID(long nID)
//{
//	m_nPatientID = nID;
//}

long CAudioRecordDlg::GetPicID() const
{
	return m_nPicID;
}

//void CAudioRecordDlg::SetPicID(long nID)
//{
//	m_nPicID = nID;
//}

// (a.walling 2010-07-27 16:40) - PLID 39433 - Support attaching to an EMN
CEMN* CAudioRecordDlg::GetEmn() const
{
	return m_pEmn;
}

void CAudioRecordDlg::ResetEmn()
{
	m_pEmn = NULL;
}

void CAudioRecordDlg::SetStatusText(const CString& strText)
{
	SetDlgItemText(IDC_STATIC_AUDIO_STATUS, strText);
}

int CAudioRecordDlg::ConvertTimeToProgressValue(DWORD nMilliseconds)
{
	// (c.haag 2006-09-25 17:11) - I can't seem to get a straight answer on the limitations
	// of the slider control range, so I'm going with a sentinel precision threshold of 10.
	return (nMilliseconds / 10);
}

DWORD CAudioRecordDlg::ConvertProgressToTimeValue(int nPos)
{
	// (c.haag 2006-09-25 17:11) - I can't seem to get a straight answer on the limitations
	// of the slider control range, so I'm going with a sentinel precision threshold of 10.
	return (nPos) * 10;
}

void CAudioRecordDlg::SetPositionText(DWORD nMilliseconds)
{	
	// (a.walling 2010-04-13 14:18) - PLID 36821 - Smaller display
	CString str;
	if (m_NxAudioCapture.IsRecording()) {
		str.Format("Position: %.02f / %d sec.", (float)nMilliseconds / 1000.0f, m_NxAudioCapture.GetMaxRecordingDuration());
	} else {
		str.Format("Position: %.02f sec.", (float)nMilliseconds / 1000.0f);
	}
	SetDlgItemText(IDC_STATIC_REC_POS, str);
}

void CAudioRecordDlg::SetLengthText(DWORD nMilliseconds, DWORD nBytes)
{
	// (a.walling 2010-04-13 14:18) - PLID 36821 - Smaller display
	CString str;
	str.Format("%.02f sec. / %.02f KiB", (float)nMilliseconds / 1000.0f, (float)nBytes / (float)1024);
	SetDlgItemText(IDC_STATIC_REC_LENGTH, str);
}

/////////////////////////////////////////////////////////////////////////////
// CAudioRecordDlg message handlers

BOOL CAudioRecordDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//
	// Designate icons for the buttons
	//
	m_btnPlay.SetIcon(::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_PLAY)));
	m_btnRec.SetIcon(::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_RECORD)));
	m_btnStop.SetIcon(::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_STOP)));
	m_btnPause.SetIcon(::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_PAUSE)));
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//
	// Bind the Windows Media Player control
	//
	try {
		// (a.walling 2008-10-27 08:27) - PLID 31827 - CWnd m_wndWMP for windowless activex control
		m_WMP = m_wndWMP.GetControlUnknown();
		m_WMPSink.EnsureSink(this, m_WMP);
	}
	NxCatchAllCall("Error initializing the Windows Media Player control", if (NULL != m_WMP) { m_WMP.Release(); });
	
	//
	// Set up the static icons
	//
	extern CPracticeApp theApp;
	GetDlgItem(IDC_STATIC_REC_POS)->SetFont(&theApp.m_boldFont);
	GetDlgItem(IDC_STATIC_REC_LENGTH)->SetFont(&theApp.m_boldFont);
	GetDlgItem(IDC_STATIC_AUDIO_STATUS)->SetFont(&theApp.m_boldFont);

	//
	// Set up the status text
	//
	SetStatusText("Status: Ready");
	SetPositionText(0);
	SetLengthText(0,0);
	m_Progress.SetRange(0,1);
	m_Progress.SetPos(0);
	
	// (a.walling 2010-04-13 14:18) - PLID 36821 - Set the title
	CString strTitle;
	strTitle.Format("NexTech %s Audio Recording for %s", m_nPicID != -1 ? "EMR" : m_bIsPatient ? "Patient" : "Contact", m_bIsPatient ? GetExistingPatientName(m_nPatientID) : GetExistingContactName(m_nPatientID));
	SetWindowText(strTitle);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioRecordDlg::OnDestroy() 
{
	try {
		//
		// Make sure no recording or playing operations are in progress
		//
		if (m_NxAudioCapture.IsRecording()) {
			m_NxAudioCapture.Cancel();
		}
		if (m_bIsPlaying) { // If the playback is paused, m_bIsPlaying is still true
			m_WMP->controls->stop();
			m_WMP->URL = "";
			m_bIsPlaying = FALSE;
		}
		m_WMPSink.CleanUp();
	}
	catch (...)
	{
	}

	//
	// Make sure the temporary file is deleted
	//
	DeleteFile(m_strTempOutputFileName);

	CDialog::OnDestroy();
}

void CAudioRecordDlg::OnBtnRec() 
{
	try {
		CWaitCursor wc;

		//
		// This should never happen; but if we are in the middle of playback, terminate it.
		// If playback is paused, then m_bIsPlaying is still true
		//
		if (m_bIsPlaying) {
			m_WMP->controls->stop();
			m_WMP->URL = "";
			m_bIsPlaying = FALSE;
		}

		// (c.haag 2006-09-29 15:49) - If there is a recording, then we need to warn the user
		// before we overwrite it
		if (m_nTotalBytes > 0) {
			if (IDNO == MsgBox(MB_YESNO, "Would you like to overwrite the existing recording?")) {
				return;
			}
		}

		m_NxAudioCapture.SetAudioFileName(GetNxTempPath() ^ "tmpRecord.wav");
		m_NxAudioCapture.SetNotificationWnd(this);
		m_NxAudioCapture.Start();

		SetStatusText("Status: Recording");
		m_Progress.SetRange(0, ConvertTimeToProgressValue(m_NxAudioCapture.GetMaxRecordingDuration() * 1000));
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_REC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
	}
	NxCatchAll("Error in CAudioRecordDlg::OnBtnRec");
}

void CAudioRecordDlg::OnBtnStop() 
{
	try {
		CWaitCursor wc;
		//
		// The behavior of the stop button depends on the current action. If
		// we are recording, we stop the recording and wait for the recording
		// termination event to later update the window enabled states. If we
		// are playing back, kill the playback right away and update the window
		// enabled states
		//
		if (m_NxAudioCapture.IsRecording()) {
			m_NxAudioCapture.Stop();
			SetPositionText(0);
			m_Progress.SetPos(0);
			m_Progress.SetRange(0, ConvertTimeToProgressValue(m_nTotalTime));
		}
		if (m_bIsPlaying) {
			m_WMP->controls->stop();
			m_WMP->URL = "";
			m_bIsPaused = FALSE;
			m_bIsPlaying = FALSE;
			KillTimer(NXT_PLAYBACK);
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_REC)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_SLIDER)->EnableWindow(FALSE);
		}
	}
	NxCatchAll("Error in CAudioRecordDlg::OnBtnStop");
}

void CAudioRecordDlg::OnBtnPause()
{
	try {
		CWaitCursor wc;
		//
		// When we pause playback, m_bIsPlaying is still set to true, and
		// we enable the slider control so that the user can change the
		// playback position
		//
		if (m_bIsPlaying) {
			m_WMP->controls->pause();
			m_bIsPaused = TRUE;
			KillTimer(NXT_PLAYBACK);
			GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_SLIDER)->EnableWindow(TRUE);
			SetStatusText("Status: Paused");
		}
	}
	NxCatchAll("Error in CAudioRecordDlg::OnBtnPause");
}

void CAudioRecordDlg::OnBtnPlay() 
{
	try {
		CWaitCursor wc;
		//
		// If playback is paused, simply resume it. Otherwise, assign the
		// temporary recording file to the Windows Media Player and begin
		// playback. In both cases, we also update the window enabled states
		//
		if (NULL != m_WMP) {
			if (m_bIsPaused) {
				m_WMP->controls->play();
				SetStatusText("Status: Playing");
				m_bIsPaused = FALSE;
			} else {
				m_WMP->URL = _bstr_t(m_strTempOutputFileName);
				m_WMP->controls->play();				
				m_bIsPlaying = TRUE;
			}
			m_Progress.SetRange(0, ConvertTimeToProgressValue(m_nTotalTime));
			SetTimer(NXT_PLAYBACK, 500, NULL);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_REC)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_SLIDER)->EnableWindow(FALSE);
		} else {
			ThrowNxException("Could not access Windows Media Player");
		}
	}
	NxCatchAll("Error in CAudioRecordDlg::OnBtnPlay");
}

LRESULT CAudioRecordDlg::OnWMPPlayStateChange(WPARAM wParam, LPARAM lParam)
{
	try {
		switch ((ENXWMPPlayState)wParam)
		{
		case eWMP_PlayState_Stopped:
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_REC)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_SLIDER)->EnableWindow(FALSE);
			m_Progress.SetPos(0);
			SetPositionText(0);
			m_WMP->URL = "";
			break;

		case eWMP_PlayState_Buffering:
			SetStatusText("Status: Buffering...");
			break;

		case eWMP_PlayState_Transitioning:
			SetStatusText("Status: Preparing...");
			break;

		case eWMP_PlayState_Playing:
			SetStatusText("Status: Playing");
			break;

		case eWMP_PlayState_Ready:
			SetStatusText("Status: Ready");
			break;
		}
	}
	NxCatchAll("Error in CAudioRecordDlg::OnWMPPlayStateChange");
	return 0;
}

LRESULT CAudioRecordDlg::OnRecordingTerminated(WPARAM wParam, LPARAM lParam)
{
	BOOL bRecordingSaved = (BOOL)wParam;
	SetStatusText("Status: Ready");
	SetPositionText(0);
	m_Progress.SetPos(0);
	GetDlgItem(IDC_BTN_REC)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_PAUSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_SLIDER)->EnableWindow(FALSE);

	// Enable the Save button if the recording was saved to the NxTemp folder. This dialog
	// is responsible for moving and attaching it to the patient's history folder.
	if (bRecordingSaved) GetDlgItem(IDOK)->EnableWindow(TRUE);
	return 0;
}

LRESULT CAudioRecordDlg::OnRecordingProgress(WPARAM wParam, LPARAM lParam)
{
	m_nTotalTime = wParam;
	m_nTotalBytes = lParam;
	SetPositionText(wParam);
	SetLengthText(wParam, lParam);
	m_Progress.SetPos(ConvertTimeToProgressValue(wParam));	
	return 0;
}

void CAudioRecordDlg::OnOK() 
{
	try {
		CWaitCursor wc;
		if (DoesExist(m_strTempOutputFileName)) {
			CString strFileName = GetPatientDocumentName(m_nPatientID, "wav");
			CString strFilePath = GetPatientDocumentPath(m_nPatientID);
			CString strFinalOutputLocation = strFilePath ^ strFileName;
			//
			// Move the temporary recording file to its final location
			//
			//TES 9/18/2008 - PLID 31413 - EnsureDirectory() moved to FileUtils
			if (!FileUtils::EnsureDirectory(strFilePath)) {
				CString strErr;
				FormatLastError(strErr, "Could not create or access the folder '%s'", strFilePath);
				ThrowNxException(strErr);
			}
			if (!CopyFile(m_strTempOutputFileName, strFinalOutputLocation, TRUE)) {
				CString strErr;
				FormatLastError(strErr, "Could not move '%s' to '%s'", m_strTempOutputFileName,
					strFinalOutputLocation);
				ThrowNxException(strErr);
			} else {
				DeleteFile(m_strTempOutputFileName);
			}
			//
			// Now attach the file to the patients history tab
			//
			long nMailID = AttachFileToHistory(strFinalOutputLocation, GetPatientID(), GetSafeHwnd(),
				-1, SELECTION_AUDIO, NULL, m_nPicID);


			// (a.walling 2010-04-14 15:20) - PLID 36821 - Show a message to the user
			CString strInfoMessage;
			
			if (m_nPicID != -1) {
				if (m_pEmn) {			
				
					// (a.walling 2010-07-27 16:37) - PLID 39433 - Check for unsaved EMN
					if (m_pEmn->GetID() == -1) {
						if (m_pEmn->GetInterface()) {
							CEmrTreeWnd* pTreeWnd = m_pEmn->GetInterface();

							if(IDYES != MessageBox("The EMN must be saved initially before attaching an audio recording.  Would you like to continue?", NULL, MB_YESNO | MB_ICONQUESTION)) {
								return;
							}
							if(FAILED(pTreeWnd->SaveEMR(esotEMN, (long)m_pEmn, TRUE))) {
								if (IDOK == MessageBox("The EMN was not saved; audio may not be attached to an unsaved EMN. However, the audio may be attached to the EMR instead.\r\n\r\nChoose OK to attach this audio file to the EMR instead of the EMN, or Cancel to try again later.", NULL, MB_ICONERROR | MB_OKCANCEL)) {
									m_pEmn = NULL;
									PostMessage(WM_COMMAND, (WPARAM)MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(IDOK)->GetSafeHwnd());
								}
								return;
							}
						}
					}

					// (a.walling 2010-07-27 16:42) - PLID 39433 - Associate this recording with the EMN.
					// (a.walling 2010-07-28 09:29) - PLID 39433 - Check records affected
					long nRecordsAffected = 0;

					// (a.walling 2010-07-28 17:46) - PLID 39433 - Disassociate from pic/emr if associated with an EMN.
					// (a.walling 2010-08-09 14:16) - PLID 39433 - Do not unset the PicID!
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, 
						"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
						"IF (SELECT Deleted FROM EmrMasterT WHERE ID = {INT}) = 0 BEGIN "
							"UPDATE MailSent SET EmnID = {INT} WHERE MailID = {INT} "
						"END", m_pEmn->GetID(), m_pEmn->GetID(), nMailID);

					if (nRecordsAffected != 1) {
						strInfoMessage.Format("The original EMN selected when recording has been deleted. Regardless, the recording has been saved and attached to this EMR's history tab for the patient %s.", GetExistingPatientName(m_nPatientID));
					} else {
						strInfoMessage.Format("The recording has been saved and attached to the EMN and the EMR's history tab for the patient %s.", GetExistingPatientName(m_nPatientID));
					}
				} else {
					strInfoMessage.Format("The recording has been saved and attached to this EMR's history tab for the patient %s.", GetExistingPatientName(m_nPatientID));
				}
			} else if (m_bIsPatient) {
				strInfoMessage.Format("The recording has been saved and attached to the history tab for the patient %s.", GetExistingPatientName(m_nPatientID));
			} else {
				strInfoMessage.Format("The recording has been saved and attached to the history tab for the contact %s.", GetExistingContactName(m_nPatientID));
			}

			MessageBox(strInfoMessage, NULL, MB_ICONINFORMATION);
		}
		CDialog::OnOK();
		
		// (a.walling 2010-04-13 14:18) - PLID 36821 - Destroy ourselves
		::DestroyWindow(this->GetSafeHwnd());
	}
	NxCatchAll("Error in CAudioRecordDlg::OnOK()");
	
	// (c.haag 2006-10-06 10:16) - PLID 21327 - Support note: If an exception is thrown, the
	// recording may still be found in the NxTemp folder until this dialog is destroyed
}

void CAudioRecordDlg::OnCancel()
{
	try {
		// (a.walling 2010-04-13 14:18) - PLID 36821 - Destroy ourselves
		::DestroyWindow(this->GetSafeHwnd());
	} NxCatchAll(__FUNCTION__);
}

void CAudioRecordDlg::OnTimer(UINT nIDEvent) 
{
	//
	// This timer is fired while audio is being played back. This will
	// update the progress text and slider to the current position of
	// the playback
	//
	if (NXT_PLAYBACK == nIDEvent) {
		if (m_bIsPlaying) {
			double d = m_WMP->controls->currentPosition;
			SetPositionText((DWORD)(d * 1000.0));
			m_Progress.SetPos(ConvertTimeToProgressValue((DWORD)(d * 1000.0)));
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CAudioRecordDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	//
	// The only time that a user should be able to move the slider themselves is
	// when playback is paused. When that happens, we reposition the current playback
	// to match the slider position
	//
	if (&m_Progress == (CSliderCtrl*)pScrollBar) {
		if (m_bIsPaused) {
			int nPos = m_Progress.GetPos();
			DWORD nNewTime = ConvertProgressToTimeValue(nPos);
			SetPositionText(nNewTime);
			m_WMP->controls->currentPosition = (double)nNewTime / 1000.0;
		}
	}
}

// (a.walling 2010-04-13 14:18) - PLID 36821 - Delete ourselves
void CAudioRecordDlg::PostNcDestroy()
{
	try {
		m_pCurrentInstance = NULL;
		delete this;
	} NxCatchAll(__FUNCTION__);
}