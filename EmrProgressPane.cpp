#include "stdafx.h"
#include "EmrProgressPane.h"
#include "EmrFrameWnd.h"
#include "EmrRc.h"
#include "dynuxtheme.h"

extern CPracticeApp theApp;
 //(e.lally 2012-02-23) PLID 48016 - Created
IMPLEMENT_DYNCREATE(CEMRProgressPane, CEmrPane)

BEGIN_MESSAGE_MAP(CEMRProgressPane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()

	////
	/// UI State overrides
	ON_UPDATE_COMMAND_UI(ID_EMR_PROGRESS_PANE_STATUS_LABEL, &CEMRProgressPane::OnUpdateStatusLabel)
	ON_UPDATE_COMMAND_UI(ID_EMR_PROGRESS_PANE_BAR, &CEMRProgressPane::OnUpdateProgressBar)
	ON_UPDATE_COMMAND_UI(ID_EMR_PROGRESS_PANE_SHOW_DETAILS, &CEMRProgressPane::OnUpdateShowDetails) //(e.lally 2012-03-26) PLID 48016
	ON_UPDATE_COMMAND_UI(ID_EMR_PROGRESS_PANE_CONFIGURE, &CEMRProgressPane::OnUpdateConfigure) //(e.lally 2012-03-26) PLID 48264


	////
	/// UI Command overrides

END_MESSAGE_MAP()


void CEMRProgressPane::OnUpdateStatusLabel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CEMRProgressPane::OnUpdateProgressBar(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

//(e.lally 2012-03-26) PLID 48016
void CEMRProgressPane::OnUpdateShowDetails(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

//(e.lally 2012-03-26) PLID 48264
void CEMRProgressPane::OnUpdateConfigure(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

CEMRProgressPane::CEMRProgressPane()
	: m_pButtonFont(NULL) // (a.walling 2014-04-29 16:55) - PLID 61966 - Init m_pButtonFont to NULL in constructor
{}

//(e.lally 2012-03-26) PLID 48264 - Moved destructor to .cpp since we're using it now
CEMRProgressPane::~CEMRProgressPane()
{
	if(m_pButtonFont) {
		delete m_pButtonFont;
		m_pButtonFont = NULL;
	}
}

HBRUSH CEMRProgressPane::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	try {
		if(pWnd->GetDlgCtrlID() == ID_EMR_PROGRESS_PANE_STATUS_LABEL)
		{
			//(e.lally 2012-02-23) PLID 48016 - Color the background with the same light blue as the rest
			// (a.walling 2012-02-24 09:38) - PLID 48386 - Get the color from CNexTechDialog
			static CBrush br(CNexTechDialog::GetSolidBackgroundRGBColor());
			pDC->SetBkColor(CNexTechDialog::GetSolidBackgroundRGBColor());
			return br;
		}
	} NxCatchAll(__FUNCTION__);

	HBRUSH hbr = CEmrPane::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

int CEMRProgressPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		CRect rcClient;
		GetClientRect(&rcClient);

		CreateControls();

		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}


//These are not available in windows 2000 compatibility, so if they aren't defined, do it ourselves
#ifndef PBM_SETSTATE
	#define PBM_SETSTATE (WM_USER+16)
#endif
#ifndef PBST_NORMAL
	#define PBST_NORMAL             0x0001
#endif
#ifndef PBST_ERROR
	#define PBST_ERROR              0x0002
#endif
#ifndef PBST_PAUSED
	#define PBST_PAUSED             0x0003
#endif

void CEMRProgressPane::CreateControls()
{
	try {
		CEmrFrameWnd* pEmrFrameWnd = GetEmrFrameWnd();
		if(pEmrFrameWnd == NULL){
			ThrowNxException("Could not get EMR Frame Window");
		}
		m_bIsEmrTemplate = pEmrFrameWnd->IsTemplate();

		//(e.lally 2012-03-26) PLID 48264 - Get button font
		if(!m_pButtonFont) {
			LOGFONT lf;
			if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
				m_pButtonFont = new CFont;
				m_pButtonFont->CreateFontIndirect(&lf);
			}
		}

		//create our status label
		if(!m_bIsEmrTemplate && !m_progressStatusLabel.GetSafeHwnd()) {
			CRect rCtrl = GetControlRect(ID_EMR_PROGRESS_PANE_STATUS_LABEL);
			//(e.lally 2012-02-28) PLID 48265 - Add SS_NOTIFY to get mouse clicks
			m_progressStatusLabel.Create("", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_NOTIFY|SS_CENTER|SS_CENTERIMAGE|SS_ENDELLIPSIS, rCtrl, this, ID_EMR_PROGRESS_PANE_STATUS_LABEL);
			//(e.lally 2012-04-02) PLID 48886 - turns out we don't need this as we are setting the background ourself and this causes weird label erasing during modal actions.
			//m_progressStatusLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT);
			m_progressStatusLabel.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		}

		//Create the progress bar
		if(!m_bIsEmrTemplate && !m_progressBar.GetSafeHwnd()) {
			CRect rCtrl = GetControlRect(ID_EMR_PROGRESS_PANE_BAR);
			m_progressBar.Create(WS_CHILD|WS_VISIBLE|PBS_SMOOTH, rCtrl, this, ID_EMR_PROGRESS_PANE_BAR);
			//(e.lally 2012-02-23) PLID 48016 - I found the animation that shows a progress bar is not frozen to be distracting since we know it's not going to progress.
			//	Since it is theme driven, we have a limited number of options.
			//(e.lally 2012-04-24) PLID 48016 - It was unanimous to come up with alternatives. These are the quickest to implement
			int nThemeOption = GetRemotePropertyInt("EmrMUProgressTheme", 2, 0, GetCurrentUserName(), true);
			switch(nThemeOption){
				case 1:
					//A. Leave the Windows Theme to control it. Windows Vista/7 are the only ones with animation so previous versions can use this freely.
					break;
				case 2: 
					//	B. We can set the state to normal, paused and error. The downside here is that it is an ugly yellow color on at least Win 7 when paused.
					//We'll set the state in the progress bar updates
					break;
				case 3:
				{
					//	C. We can disable the theme and use the old blue rectangle from Windows Classic (2000)
					static UXTheme uxthemeNone;
					uxthemeNone.SetWindowTheme(m_progressBar.GetSafeHwnd(), L"", L"");
					break;
				}
				//case 4: //TODO: draw it ourselves
			}
			
			//Range will be a percentage 0% - 100%
			m_progressBar.SetRange32(0, 100);
		}

		//(e.lally 2012-03-26) PLID 48016 - create our show details button
		if(!m_bIsEmrTemplate && !m_btnShowDetails.GetSafeHwnd()) {
			CRect rCtrl = GetControlRect(ID_EMR_PROGRESS_PANE_SHOW_DETAILS);
			//(e.lally 2012-02-28) PLID 48265 - Shows the details of the progress pane
			m_btnShowDetails.Create("", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rCtrl, this, ID_EMR_PROGRESS_PANE_SHOW_DETAILS);
			m_btnShowDetails.AutoSet(NXB_INSPECT);
			m_btnShowDetails.SetFont(m_pButtonFont);
		}

		//(e.lally 2012-03-26) PLID 48264 - create our configure button
		if(!m_bIsEmrTemplate && !m_btnConfigure.GetSafeHwnd()) {
			CRect rCtrl = GetControlRect(ID_EMR_PROGRESS_PANE_CONFIGURE);
			m_btnConfigure.Create("...", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rCtrl, this, ID_EMR_PROGRESS_PANE_CONFIGURE);
			m_btnConfigure.SetFont(m_pButtonFont);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMRProgressPane::RepositionControls()
{
	try {
		ASSERT(IsWindow(GetSafeHwnd()));

		CRect rc;

		if (IsWindow(m_progressStatusLabel.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_PROGRESS_PANE_STATUS_LABEL);
			m_progressStatusLabel.MoveWindow(&rc);
			// (e.lally 2012-04-23) PLID 48016
			m_progressStatusLabel.Invalidate();
		}

		if (IsWindow(m_progressBar.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_PROGRESS_PANE_BAR);
			m_progressBar.MoveWindow(&rc);
		}

		//(e.lally 2012-03-26) PLID 48016
		if (IsWindow(m_btnShowDetails.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_PROGRESS_PANE_SHOW_DETAILS);
			m_btnShowDetails.MoveWindow(&rc);
		}

		//(e.lally 2012-03-26) PLID 48264
		if (IsWindow(m_btnConfigure.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_PROGRESS_PANE_CONFIGURE);
			m_btnConfigure.MoveWindow(&rc);
		}
	} NxCatchAll(__FUNCTION__);
}


//(e.lally 2012-03-26) PLID 48264 - Reconfigured calculations to account for a new layout
CRect CEMRProgressPane::GetControlRect(UINT nID)
{
	CRect rThis, rControl;
	GetClientRect(rThis);
	//Limit how wide we are going to allow the controls to extend since it looks weird when extended the full screen width
	if(rThis.Width() > 360){
		rThis.right = rThis.left + 360;
	}
	//Limit how tall we are going to allow the controls to extend since it looks weird when extended the full screen height
	if(rThis.Height() > 117){
		rThis.bottom = rThis.top + 117;
	}
	//Flush out the border buffer ahead of time
	const int cnTopBuffer = 4;
	const int cnHBuffer = 4;

	rThis.left += cnHBuffer;
	rThis.right -= cnHBuffer;
	//rThis.top += cnTopBuffer;
	rThis.bottom -= cnTopBuffer;

	rControl = rThis;
	
	const int cnVControlRowCount = 3;
	//Subtract out the other row count - 1 buffers
	const int cnVControlSplits = (int)((rThis.Height() - (cnTopBuffer * (cnVControlRowCount-1))) / cnVControlRowCount);
	int nConfigureBtnWidth = 35;
	//Make sure the configure button is not wider than our pane.
	if(rThis.Width() <= 40){
		nConfigureBtnWidth = (int)rThis.Width() / 2;
	}
	int nShowDetailsBtnWidth = 70;
	if(rThis.Width() < nShowDetailsBtnWidth){
		nShowDetailsBtnWidth = rThis.Width();
	}

	switch(nID) {
		case ID_EMR_PROGRESS_PANE_STATUS_LABEL:
			rControl.SetRect(rThis.left, rThis.top, rThis.right, rThis.top + (1 * cnVControlSplits));
			break;
		case ID_EMR_PROGRESS_PANE_BAR:
			rControl.SetRect(rThis.left, rThis.top + (1 * cnVControlSplits) + (1 * cnTopBuffer), rThis.Width() - nConfigureBtnWidth - cnHBuffer, rThis.top + (2 * cnVControlSplits)+ cnTopBuffer);
			break;
		case ID_EMR_PROGRESS_PANE_CONFIGURE:
			rControl.SetRect(rThis.Width() - nConfigureBtnWidth, rThis.top + (1 * cnVControlSplits) + (1 * cnTopBuffer), rThis.right, rThis.top + (2 * cnVControlSplits)+ cnTopBuffer);
			break;
		case ID_EMR_PROGRESS_PANE_SHOW_DETAILS:
			{
			int nHCenter = rThis.left + (rThis.Width()/2);
			//To center, start at the left plus half the width (to reach the center point) and then subtract half the button width
			//	Then go from
			rControl.SetRect(nHCenter - (nShowDetailsBtnWidth / 2), rThis.top + (2 * cnVControlSplits) + (2 * cnTopBuffer), nHCenter + (nShowDetailsBtnWidth / 2), rThis.bottom);
			break;
			}
		default:
			break;
	}

	return rControl;
}

//(e.lally 2012-02-23) PLID 48016 - Updates the status label
void CEMRProgressPane::SetStatusText(const CString& strNewText)
{
	if(!IsWindow(m_progressStatusLabel.GetSafeHwnd()) ){
		return;
	}
	CString strCurrentText;
	m_progressStatusLabel.GetWindowText(strCurrentText);
	if(strNewText != strCurrentText){
		m_progressStatusLabel.SetWindowText(strNewText);
		m_progressStatusLabel.Invalidate();
	}
}

//(e.lally 2012-02-23) PLID 48016 - Updates the progress bar. Pass in the current completed and total amounts
void CEMRProgressPane::SetProgressBar(const long& nCompleted, const long& nTotal)
{
	//We've set the range to a percentage, so just calculate the percentage here.
	double dblPercent = 0.0;
	if(nTotal > 0){
		dblPercent = (100*(nCompleted/(double)nTotal));
	}
	if(dblPercent > 100){
		dblPercent = 100;
	}
	if(m_progressBar.GetSafeHwnd()){
		m_progressBar.SetPos((int)dblPercent);
	}

	//(e.lally 2012-04-24) PLID 48016
	int nThemeOption = GetRemotePropertyInt("EmrMUProgressTheme", 2, 0, GetCurrentUserName(), true);
	if(nThemeOption == 2){
		//	We can set the state to normal, paused and error. 
		//	The Normal will be animated in Windows Vista / 7 until the progress bar is at 100%. We are only going to satisfy it at 100% because the animation can be distracting.
		if(dblPercent <= 50.0){
			//Red
			::SendMessage(m_progressBar.GetSafeHwnd(),(UINT)PBM_SETSTATE, (WPARAM)PBST_ERROR,(LPARAM)0L); 
		}
		else if(dblPercent < 100.0){
			//Yellow
			::SendMessage(m_progressBar.GetSafeHwnd(),(UINT)PBM_SETSTATE, (WPARAM)PBST_PAUSED,(LPARAM)0L);
		}
		else {
			//Green!
			::SendMessage(m_progressBar.GetSafeHwnd(),(UINT)PBM_SETSTATE, (WPARAM)PBST_NORMAL,(LPARAM)0L);
		}
	}
}
