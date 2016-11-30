// AppointmentReminderAdvancedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "AppointmentReminderAdvancedDlg.h"


// CAppointmentReminderAdvancedDlg dialog
// (z.manning 2011-11-02 09:58) - PLID 46220 - Created

IMPLEMENT_DYNAMIC(CAppointmentReminderAdvancedDlg, CNxDialog)

CAppointmentReminderAdvancedDlg::CAppointmentReminderAdvancedDlg(CAppointmentReminderSetupDlg::AdvancedSettings *pSettings, CWnd* pParent /*=NULL*/)
	: CNxDialog(CAppointmentReminderAdvancedDlg::IDD, pParent)
	, m_pSettings(pSettings)
{

}

CAppointmentReminderAdvancedDlg::~CAppointmentReminderAdvancedDlg()
{
}

void CAppointmentReminderAdvancedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEXREMINDER_ADVANCED_BACKGROUND, m_nxcolor);
}


BEGIN_MESSAGE_MAP(CAppointmentReminderAdvancedDlg, CNxDialog)
END_MESSAGE_MAP()


// CAppointmentReminderAdvancedDlg message handlers


BOOL CAppointmentReminderAdvancedDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		((CEdit*)GetDlgItem(IDC_NEXREMINDER_LEADING_DIGIT_TO_TRIM))->SetLimitText(10);

		SetDlgItemText(IDC_NEXREMINDER_LEADING_DIGIT_TO_TRIM, m_pSettings->strLeadingDigitToTrim);
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CAppointmentReminderAdvancedDlg::OnOK()
{
	try
	{
		// (z.manning 2011-11-02 10:14) - PLID 46220 - Store the leading digit to trim option.
		// No need to validate this as nonsense data will just result in this being ignored.
		GetDlgItemText(IDC_NEXREMINDER_LEADING_DIGIT_TO_TRIM, m_pSettings->strLeadingDigitToTrim);

		CNxDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}