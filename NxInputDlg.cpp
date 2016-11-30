// NxInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NxInputDlg.h"
#include "NxStandard.h"
#include "NxStandardRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// (a.walling 2008-09-19 09:53) - PLID 28040 - NxStandard has been consolidated into Practice

/////////////////////////////////////////////////////////////////////////////
// CNxInputDlg dialog


CNxInputDlg::CNxInputDlg(CWnd* pParent, const CString &strPrompt, CString &strResult, const CString &strOther, bool bPassword /* = false */, bool bBrowseBtn /* = false */, LPCTSTR strCancelBtnText /* = NULL */, unsigned long nMaxChars /* = -1 */, BOOL bIsNumeric /* = FALSE*/)
	: CNxDialog(IDD_INPUT_DLG, pParent),
	m_strPrompt(strPrompt),
	m_strResult(strResult),
	m_strOther(strOther)
{
	//{{AFX_DATA_INIT(CNxInputDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bPassword = bPassword;
	m_bBrowseBtn = bBrowseBtn;
	m_strCancelText = strCancelBtnText;
	m_nMaxChars = nMaxChars;
	m_bIsNumeric = bIsNumeric;
}

// (a.walling 2008-10-13 10:23) - PLID 28040 - Made this a proper NxDialog
void CNxInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxInputDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_NXINPUT_PROMPT_LABEL, m_lblPrompt);

	DDX_Control(pDX, IDC_RESULT_EDIT, m_editResult);
	DDX_Control(pDX, IDC_RESULT_NUM_EDIT, m_editNumResult);
	DDX_Control(pDX, IDC_INPUT_MASKED_RESULT, m_editMasked);

	DDX_Control(pDX, IDC_BROWSE_BTN, m_nxibBrowse);
	DDX_Control(pDX, IDOK, m_nxibOK);
	DDX_Control(pDX, IDABORT, m_nxibOther);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNxInputDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNxInputDlg)
	ON_BN_CLICKED(IDABORT, OnAbort)
	ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxInputDlg message handlers

BOOL CNxInputDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_nxibOK.AutoSet(NXB_OK);
		m_nxibCancel.AutoSet(NXB_CANCEL);

		//Resize everything.
		int nVertOffset;
		{
			CDC *pDC = GetDC();
			CRect rOldLabel, rNewLabel;
			CWnd *pWndStatic = GetDlgItem(IDC_NXINPUT_PROMPT_LABEL);
			if (pWndStatic->GetSafeHwnd()) {
				pWndStatic->GetWindowRect(rOldLabel);
				rNewLabel = CRect(0, 0, rOldLabel.Width(), 0);
				CFont *pStaticFont = pWndStatic->GetFont();
				CFont *pOldFont = pDC->SelectObject(pStaticFont);
				pDC->DrawText(m_strPrompt, &rNewLabel, DT_LEFT | DT_CALCRECT | DT_WORDBREAK);
				pDC->SelectObject(pOldFont);
				ReleaseDC(pDC);
				nVertOffset = rNewLabel.Height() - rOldLabel.Height();
			}
			else {
				nVertOffset = 0;
			}
		}
		if (nVertOffset > 0) {
			//Resize the window.
			CRect rect;
			GetWindowRect(rect);
			SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height() + nVertOffset, SWP_NOMOVE | SWP_NOZORDER);

			//Move everything else down.
			GetDlgItem(IDC_RESULT_EDIT)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDC_RESULT_EDIT)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			GetDlgItem(IDC_RESULT_NUM_EDIT)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDC_RESULT_NUM_EDIT)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			// (z.manning 2015-08-14 09:28) - PLID 67248
			GetDlgItem(IDC_INPUT_MASKED_RESULT)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDC_INPUT_MASKED_RESULT)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			// (z.manning 2015-08-14 09:28) - PLID 67248
			GetDlgItem(IDC_NXINPUT_PROMPT_LOWER_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDC_NXINPUT_PROMPT_LOWER_LABEL)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			GetDlgItem(IDABORT)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDABORT)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			GetDlgItem(IDCANCEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDCANCEL)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			GetDlgItem(IDC_BROWSE_BTN)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDC_BROWSE_BTN)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			GetDlgItem(IDOK)->GetWindowRect(rect);
			ScreenToClient(rect);
			GetDlgItem(IDOK)->SetWindowPos(NULL, rect.left, rect.top + nVertOffset, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOZORDER);

			//We need to set the prompt label to be left justified.
			DWORD dwLabelStyle = GetDlgItem(IDC_NXINPUT_PROMPT_LABEL)->GetStyle();
			GetDlgItem(IDC_NXINPUT_PROMPT_LABEL)->ModifyStyle(SS_CENTER | SS_CENTERIMAGE, SS_LEFT, 0);
			//Finally, resize the prompt label.
			GetDlgItem(IDC_NXINPUT_PROMPT_LABEL)->GetWindowRect(rect);
			GetDlgItem(IDC_NXINPUT_PROMPT_LABEL)->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height() + nVertOffset, SWP_NOMOVE | SWP_NOZORDER);
		}

		SetDlgItemText(IDC_NXINPUT_PROMPT_LABEL, m_strPrompt);
		SetDlgItemText(IDC_RESULT_EDIT, m_strResult);
		// (j.jones 2012-02-17 09:06) - PLID 48137 - numeric prompts with no default would default to zero, and should not
		if (!m_strResult.IsEmpty()) {
			SetDlgItemInt(IDC_RESULT_NUM_EDIT, atol(m_strResult));
		}
		if (m_strOther.GetLength()) {
			SetDlgItemText(IDABORT, m_strOther);
		}
		else {
			GetDlgItem(IDABORT)->ShowWindow(SW_HIDE);
		}
		if (m_strCancelText) {
			SetDlgItemText(IDCANCEL, m_strCancelText);
		}

		if (m_bPassword) {
			((CEdit *)GetDlgItem(IDC_RESULT_EDIT))->SetPasswordChar('*');
		}
		if (m_bBrowseBtn) {
			GetDlgItem(IDC_BROWSE_BTN)->ShowWindow(SW_SHOW);
		}
		else {
			GetDlgItem(IDC_BROWSE_BTN)->ShowWindow(SW_HIDE);
		}
		if (m_nMaxChars != -1) {
			((CEdit *)GetDlgItem(IDC_RESULT_EDIT))->SetLimitText(m_nMaxChars);
			((CEdit *)GetDlgItem(IDC_RESULT_NUM_EDIT))->SetLimitText(m_nMaxChars);
			((CEdit *)GetDlgItem(IDC_INPUT_MASKED_RESULT))->SetLimitText(m_nMaxChars);
		}

		// (z.manning 2015-08-14 09:39) - PLID 67248 - Set the input mask of the masked field
		m_editMasked.SetInputMask(m_strInputMask);

		// (z.manning 2015-08-14 11:48) - PLID 67248 - Set the cue banner if we have one
		if (!m_strCueBanner.IsEmpty())
		{
			CStringW wstrCueBanner = m_strCueBanner;
			m_editResult.SendMessage(EM_SETCUEBANNER, 0, (LPARAM)(LPCWSTR)wstrCueBanner);
			m_editNumResult.SendMessage(EM_SETCUEBANNER, 0, (LPARAM)(LPCWSTR)wstrCueBanner);
			m_editMasked.SendMessage(EM_SETCUEBANNER, 0, (LPARAM)(LPCWSTR)wstrCueBanner);

			// (z.manning 2015-08-24 08:58) - PLID 67248 - Also set the lower label to the cue
			// banner so it's visible when focus is in the edit box.
			SetDlgItemText(IDC_NXINPUT_PROMPT_LOWER_LABEL, m_strCueBanner);
		}

		BOOL bMasked = !m_strInputMask.IsEmpty();
		GetDlgItem(IDC_RESULT_EDIT)->ShowWindow(!m_bIsNumeric && !bMasked ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_RESULT_NUM_EDIT)->ShowWindow(m_bIsNumeric && !bMasked ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_INPUT_MASKED_RESULT)->ShowWindow(bMasked ? SW_SHOW : SW_HIDE);
	}
	NxCatchAll(__FUNCTION__)

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNxInputDlg::OnOK() 
{
	try
	{
		if (m_strInputMask.IsEmpty())
		{
			if (!m_bIsNumeric) {
				GetDlgItemText(IDC_RESULT_EDIT, m_strResult);
			}
			else {
				m_strResult.Format("%li", GetDlgItemInt(IDC_RESULT_NUM_EDIT));
			}
		}
		else
		{
			// (z.manning 2015-08-14 09:35) - PLID 67248 - Handled input mask edit
			GetDlgItemText(IDC_INPUT_MASKED_RESULT, m_strResult);
		}

		CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__)
}

void CNxInputDlg::OnAbort() 
{
	try
	{
		CDialog::EndDialog(IDABORT);
	}
	NxCatchAll(__FUNCTION__)
}

void CNxInputDlg::OnBrowseBtn() 
{
	try
	{
		CString strCurPath;
		GetDlgItemText(IDC_RESULT_EDIT, strCurPath);
		if (Browse(GetSafeHwnd(), strCurPath, strCurPath, false)) {
			SetDlgItemText(IDC_RESULT_EDIT, strCurPath);
		}
	}
	NxCatchAll(__FUNCTION__)
}
