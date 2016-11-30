// ChooseDateRangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "ChooseDateRangeDlg.h"


// CChooseDateRangeDlg dialog
// (j.dinatale 2011-11-14 13:04) - PLID 45658 - created

IMPLEMENT_DYNAMIC(CChooseDateRangeDlg, CNxDialog)

CChooseDateRangeDlg::CChooseDateRangeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChooseDateRangeDlg::IDD, pParent)
{
	m_dtTo = COleDateTime::GetCurrentTime();
	m_dtFrom = m_dtTo;
	m_dtMin = g_cdtMin;
	m_dtMax = g_cdtMax;
	m_bCancelButtonVisible = FALSE; // (c.haag 2014-12-17) - PLID 64253
	m_bChooseTimeRange = FALSE; // (c.haag 2014-12-17) - PLID 64253
	m_bToDateOptional = FALSE;
}

CChooseDateRangeDlg::~CChooseDateRangeDlg()
{
}

void CChooseDateRangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DATE_TO, m_dtpTo);
	DDX_Control(pDX, IDC_DATE_FROM, m_dtpFrom);
}


BEGIN_MESSAGE_MAP(CChooseDateRangeDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CChooseDateRangeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CChooseDateRangeDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_DATE_TO_CHECK, &CChooseDateRangeDlg::OnBnClickedDateToCheck)
END_MESSAGE_MAP()


// CChooseDateRangeDlg message handlers
BOOL CChooseDateRangeDlg::OnInitDialog() 
{
	try{
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL); // (c.haag 2014-12-17) - PLID 64253

		// (c.haag 2014-12-17) - PLID 64253 - Configure the date time choosers
		if (m_bChooseTimeRange)
		{
			m_dtpTo.ShowWindow(SW_HIDE);
			m_dtpFrom.ShowWindow(SW_HIDE);
			m_pTimeTo = BindNxTimeCtrl(this, IDC_TOTIME);
			m_pTimeFrom = BindNxTimeCtrl(this, IDC_FROMTIME);
			GetDlgItem(IDC_FROMTIME)->SetFocus();
			SetWindowText("Choose Time Range");
		}
		else
		{
			GetDlgItem(IDC_FROMTIME)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TOTIME)->ShowWindow(SW_HIDE);
			m_dtpTo.SetValue(m_dtTo);
			m_dtpFrom.SetValue(m_dtFrom);
		}

		if (m_bCancelButtonVisible)
		{
			// (c.haag 2014-12-17) - PLID 64253 - Display the cancel button
			CRect rOK, rNewOKRect, rNewCancelRect;
			GetDlgItem(IDOK)->GetWindowRect(rOK);
			ScreenToClient(&rOK);
			GetDlgItem(IDC_STATIC_OKALIGNMENT)->GetWindowRect(rNewOKRect);
			ScreenToClient(&rNewOKRect);
			GetDlgItem(IDC_STATIC_CANCELALIGNMENT)->GetWindowRect(rNewCancelRect);
			ScreenToClient(&rNewCancelRect);

			GetDlgItem(IDOK)->SetWindowPos(NULL, rNewOKRect.left, rOK.top, rOK.Width(), rOK.Height(), SWP_NOZORDER);
			GetDlgItem(IDCANCEL)->SetWindowPos(NULL, rNewCancelRect.left, rOK.top, rOK.Width(), rOK.Height(), SWP_NOZORDER);
			GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
		}
		else
		{
			// disable the X at the top of the form so they cant close attempt to close the window
			CMenu* pSM = GetSystemMenu(FALSE);
			if (pSM){
				pSM->EnableMenuItem(SC_CLOSE, (MF_BYCOMMAND | MF_GRAYED | MF_DISABLED));
			}
		}

		// (z.manning 2016-03-11 15:44) - PLID 68584 - Show/hide the relevant controls depending on
		// whether or not the end date is optional.
		GetDlgItem(IDC_DATE_TO_LABEL)->ShowWindow(m_bToDateOptional ? SW_HIDE : SW_SHOWNA);
		GetDlgItem(IDC_DATE_TO_CHECK)->ShowWindow(m_bToDateOptional ? SW_SHOWNA : SW_HIDE);
		GetDlgItem(IDC_DATE_TO_CHECK)->EnableWindow(m_bToDateOptional);

		UpdateControls();

	}NxCatchAll(__FUNCTION__);

	return m_bChooseTimeRange;
}

void CChooseDateRangeDlg::SetMinDate(COleDateTime dtMin)
{
	// if the date passed in is bad, we assume the min possible date as the new minimum
	if(dtMin == g_cdtNull || dtMin.GetStatus() == COleDateTime::invalid){
		dtMin = g_cdtMin;
	}

	m_dtMin = dtMin;
}

void CChooseDateRangeDlg::SetMaxDate(COleDateTime dtMax)
{
	// if the date passed in is bad, we assume the max possible date
	if(dtMax == g_cdtNull || dtMax.GetStatus() == COleDateTime::invalid){
		dtMax = g_cdtMax;
	}

	m_dtMax = dtMax;
}

// (c.haag 2014-12-17) - PLID 64253 - Lets the caller choose whether to show the cancel button
// when the dialog is open.
void CChooseDateRangeDlg::SetCancelButtonVisible(BOOL bVisible)
{
	m_bCancelButtonVisible = bVisible;
}

// (c.haag 2014-12-17) - PLID 64253 - Lets the caller choose between two different times of day
// when the dialog is open.
void CChooseDateRangeDlg::SetChooseTimeRange(BOOL bChooseTimeRange)
{
	m_bChooseTimeRange = bChooseTimeRange;
}

// (z.manning 2016-03-11 15:41) - PLID 68584
void CChooseDateRangeDlg::SetToDateOptional(BOOL bToDateOptional)
{
	m_bToDateOptional = bToDateOptional;
}

int CChooseDateRangeDlg::DoModal()
{
	// call base DoModal
	return CNxDialog::DoModal();
}

COleDateTime CChooseDateRangeDlg::GetToDate()
{
	return m_dtTo;
}

COleDateTime CChooseDateRangeDlg::GetFromDate()
{
	return m_dtFrom;
}


void CChooseDateRangeDlg::OnBnClickedOk()
{
	try{
		CString strMessage;

		// update our member variables
		// (c.haag 2014-12-17) - PLID 64253 - What we do here depends on what kind of range we're modifying
		if (m_bChooseTimeRange)
		{
			m_dtFrom = m_pTimeFrom->GetDateTime();
			m_dtTo = m_pTimeTo->GetDateTime();

			// verify that the from time is valid. We can't set it to null or invalid; so we have to settle for 0.
			if (m_dtFrom.m_dt == 0){
				strMessage.Format("Please select a valid From time.");
			}
			// verify that the to time is valid
			else if (m_dtTo.m_dt == 0){
				strMessage.Format("Please select a valid To time.");
			}
			// verify that the from time comes before the to time
			else if (CompareTimes(m_dtTo, m_dtFrom) & CT_LESS_THAN_OR_EQUAL){
				strMessage.Format("Please select a To time that is after the From time.");
			}
		}
		else
		{
			m_dtFrom = m_dtpFrom.GetDateTime();

			// (z.manning 2016-03-11 15:45) - PLID 68584 - Handle the end date being optional
			if (IsEndDateVisible()) {
				m_dtTo = m_dtpTo.GetDateTime();
			}
			else {
				m_dtTo.SetStatus(COleDateTime::null);
			}

			// verify that the from date is valid
			if (m_dtFrom == g_cdtNull || m_dtFrom.GetStatus() == COleDateTime::invalid){
				strMessage.Format("Please select a valid From date.");
			}

			if (CompareDatesNoTime(m_dtFrom, m_dtMin) < 0){
				strMessage.Format("Please select a From date that is equal to or later than %s.", FormatDateTimeForInterface(m_dtMin, dtoDate));
			}

			if (CompareDatesNoTime(m_dtFrom, m_dtMax) > 0){
				strMessage.Format("Please select a From date that is before or equal to %s.", FormatDateTimeForInterface(m_dtMax, dtoDate));
			}

			// (z.manning 2016-03-11 15:50) - PLID 68584 - We can skip these validations if the user chose not to select an end date
			if (IsEndDateVisible())
			{
				// verify that the to date is valid
				if (m_dtTo == g_cdtNull || m_dtTo.GetStatus() == COleDateTime::invalid) {
					strMessage.Format("Please select a valid To date.");
				}

				if (CompareDatesNoTime(m_dtTo, m_dtMin) < 0) {
					strMessage.Format("Please select a To date that is equal to or later than %s.", FormatDateTimeForInterface(m_dtMin, dtoDate));
				}

				if (CompareDatesNoTime(m_dtTo, m_dtMax) > 0) {
					strMessage.Format("Please select a To date that is before or equal to %s.", FormatDateTimeForInterface(m_dtMax, dtoDate));
				}

				// verify that the from date comes before the to date
				if (CompareDatesNoTime(m_dtTo, m_dtFrom) < 0) {
					strMessage.Format("Please select a To date that is the same or after the From date.");
				}
			}
		}

		if(!strMessage.IsEmpty()){
			AfxMessageBox(strMessage);
			return;
		}
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}

void CChooseDateRangeDlg::OnBnClickedCancel()
{
	if (m_bCancelButtonVisible)
	{
		// (c.haag 2014-12-17) - PLID 64253 - Cancel out of the window
		__super::OnCancel();
	}
	else
	{
		// do nothing here
	}
}

// (z.manning 2016-03-11 16:04) - PLID 68584
BOOL CChooseDateRangeDlg::IsEndDateVisible()
{
	return (!m_bChooseTimeRange && (!m_bToDateOptional || IsDlgButtonChecked(IDC_DATE_TO_CHECK) == BST_CHECKED));
}

// (z.manning 2016-03-11 16:04) - PLID 68584
void CChooseDateRangeDlg::UpdateControls()
{
	GetDlgItem(IDC_DATE_TO)->ShowWindow(IsEndDateVisible() ? SW_SHOWNA : SW_HIDE);
	GetDlgItem(IDC_DATE_TO)->EnableWindow(IsEndDateVisible());
}

// (z.manning 2016-03-11 15:34) - PLID 68584
void CChooseDateRangeDlg::OnBnClickedDateToCheck()
{
	try
	{
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}
