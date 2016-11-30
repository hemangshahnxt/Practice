// RescheduleAppointmentsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "RescheduleAppointmentsDlg.h"
#include "SchedulerRc.h"

#include "EditComboBox.h"

#include <NxUILib/DatalistUtils.h>

// (a.walling 2015-01-05 16:24) - PLID 64373 - Reschedule appointments dialog

// CRescheduleAppointmentsDlg dialog

namespace {
	enum GenericListColumns
	{
		glcID = 0,
		glcName = 1,
	};

	enum ResourceListColumns
	{
		rlcID = 0,
		rlcChecked = 1,
		rlcName = 2,
	};
}

IMPLEMENT_DYNAMIC(CRescheduleAppointmentsDlg, CNxDialog)

CRescheduleAppointmentsDlg::CRescheduleAppointmentsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRescheduleAppointmentsDlg::IDD, pParent)
{

}

CRescheduleAppointmentsDlg::~CRescheduleAppointmentsDlg()
{
}

void CRescheduleAppointmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATE_START, m_dtpFrom);
	DDX_Control(pDX, IDC_DATE_END, m_dtpTo);
}


BEGIN_MESSAGE_MAP(CRescheduleAppointmentsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ALL_RESOURCES, &CRescheduleAppointmentsDlg::OnBnClickedAllResources)
	ON_BN_CLICKED(IDC_SELECT_RESOURCES, &CRescheduleAppointmentsDlg::OnBnClickedSelectResources)
	ON_BN_CLICKED(IDC_EDIT_CANCELLATION_REASONS, &CRescheduleAppointmentsDlg::OnBnClickedEditCancellationReasons)
	ON_BN_CLICKED(IDOK, &CRescheduleAppointmentsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RESCHEDULE_ALL, &CRescheduleAppointmentsDlg::OnBnClickedRescheduleAll)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_START, &CRescheduleAppointmentsDlg::OnDtnDatetimechangeDateStart)
END_MESSAGE_MAP()


// CRescheduleAppointmentsDlg message handlers

BOOL CRescheduleAppointmentsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDOK))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CLOSE);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_EDIT_CANCELLATION_REASONS))->AutoSet(NXB_MODIFY);

		m_pResources = BindNxDataList2Ctrl(IDC_RESOURCES, false);
		m_pReason = BindNxDataList2Ctrl(IDC_CANCELLATION_REASON, false);
		m_pTemplate = BindNxDataList2Ctrl(IDC_CANCELLATION_TEMPLATE, false);

		m_pTimeFrom = BindNxTimeCtrl(this, IDC_TIME_START);
		m_pTimeTo = BindNxTimeCtrl(this, IDC_TIME_END);

		// (a.walling 2015-01-06 14:28) - PLID 64376 - Start and End time
		m_pTimeFrom->ReturnType = 1;
		m_pTimeTo->ReturnType = 1;

		// (a.walling 2015-01-06 14:37) - PLID 64375 - Start and End date
		auto dtNow = AsDateNoTime(COleDateTime::GetCurrentTime());
		m_dtpFrom.SetTime(dtNow);
		m_dtpTo.SetTime(dtNow);

		// past dates are now allowed
		//m_dtpFrom.SetRange(nullptr, nullptr);
		m_dtpTo.SetRange(&dtNow, nullptr);

		// (a.walling 2015-01-06 13:16) - PLID 64374 - Resources list
		CheckRadioButton(IDC_ALL_RESOURCES, IDC_SELECT_RESOURCES, IDC_SELECT_RESOURCES);
		m_pResources->Enabled = VARIANT_TRUE;

		{
			using namespace NXDATALIST2Lib;

			m_pResources->AllowMultiSelect = VARIANT_TRUE;

			AppendColumns(m_pResources, {
					{ "ID", "ID", 0, csFixedWidth },
					{ "CONVERT(BIT, 0)", "", 24, csFixedWidth | csEditable, cftBoolCheckbox },
					{ "Item", "Name", 0, csWidthAuto, cftTextSingleLine, 0 }
			});
			m_pResources->FromClause = "ResourceT";
			m_pResources->WhereClause = "ResourceT.Inactive = 0";

			m_pResources->Requery();
		}

		// (a.walling 2015-01-06 15:06) - PLID 64378 - Template list
		{
			using namespace NXDATALIST2Lib;

			AppendColumns(m_pTemplate, {
					{ "ID", "ID", 0, csFixedWidth },
					{ "Name", "Name", 0, csWidthAuto, cftTextSingleLine, 0 }
			});
			m_pTemplate->FromClause = "(SELECT ID, Name FROM TemplateT UNION ALL SELECT -1, '< No Template >') SubQ";

			m_pTemplate->DisplayColumn = "[1]";

			m_pTemplate->Requery();

			m_pTemplate->SetSelByColumn(glcID, -1L);
		}

			// (a.walling 2015-01-06 15:12) - PLID 64379 - Reason list
		{
			using namespace NXDATALIST2Lib;

			AppendColumns(m_pReason, {
					{ "ID", "ID", 0, csFixedWidth },
					{ "Description", "Name", 0, csWidthAuto, cftTextSingleLine, 0 }
			});
			m_pReason->FromClause = "(SELECT ID, Description FROM AptCancelReasonT UNION ALL SELECT -1, '< No Reason >') SubQ";

			m_pReason->DisplayColumn = "[1]";

			m_pReason->Requery();

			m_pReason->SetSelByColumn(glcID, -1L);
		}
	}NxCatchAll(__FUNCTION__); 
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

Nx::Scheduler::RescheduleAppointmentsInfo CRescheduleAppointmentsDlg::GenerateInfo() const
{
	Nx::Scheduler::RescheduleAppointmentsInfo info;

	if (auto row = m_pReason->CurSel) {
		info.cancelReasonID = row->Value[glcID];
	}

	if (auto row = m_pTemplate->CurSel) {
		info.templateID = row->Value[glcID];
	}

	COleDateTime dt;
	if (m_dtpFrom.GetTime(dt)) {
		info.from = AsDateNoTime(dt);
	}
	if (m_dtpTo.GetTime(dt)) {
		info.to = AsDateNoTime(dt);
	}

	if (!IsDlgButtonChecked(IDC_RESCHEDULE_ALL)) {
		if (m_pTimeFrom->GetStatus() == 1)  {
			COleDateTime dt = m_pTimeFrom->GetDateTime();
			info.offsetFrom = dt - AsDateNoTime(dt);
		}
		if (m_pTimeTo->GetStatus() == 1)  {
			COleDateTime dt = m_pTimeTo->GetDateTime();
			info.offsetTo = dt - AsDateNoTime(dt);
		}
	}

	if (!IsDlgButtonChecked(IDC_ALL_RESOURCES)) {
		for (auto row = m_pResources->GetFirstRow(); !!row; row = row->GetNextRow()) {
			if (row->Value[rlcChecked]) {
				info.resources.push_back(row->Value[rlcID]);
			}
		}
	}

	return info;
}

Nx::Scheduler::RescheduleAppointmentsInfo CRescheduleAppointmentsDlg::ValidateInfo()
{

	// (a.walling 2015-01-06 16:46) - PLID 64377 - Ensure a time frame is selected
	if (!IsDlgButtonChecked(IDC_RESCHEDULE_ALL))
	{
		if ((m_pTimeFrom->GetStatus() != 1) || (m_pTimeTo->GetStatus() != 1)) {
			MessageBox("You have not selected a time frame.");
			return{};
		}
	}

	auto info = GenerateInfo();

	// (a.walling 2015-01-22 15:39) - PLID 64374 - Ensure some resources are chosen if 'Select Resources' is checked
	if (info.resources.empty() && IsDlgButtonChecked(IDC_SELECT_RESOURCES)) {
		MessageBox("You have not checked any resources. Please check one or more resources, or select 'All Resources' instead.");
		return{};
	}

	if ((info.to < info.from) || (info.offsetTo < info.offsetFrom)) {
		MessageBox("The end time is earlier than the start time!");
		return{};
	}

	return info;
}

void CRescheduleAppointmentsDlg::OnBnClickedAllResources()
{
	try {
		// (a.walling 2015-01-06 13:16) - PLID 64374 - Toggle resources list enabled
		m_pResources->Enabled = VARIANT_FALSE;
	} NxCatchAll(__FUNCTION__);
}

void CRescheduleAppointmentsDlg::OnBnClickedSelectResources()
{
	try {
		// (a.walling 2015-01-06 13:16) - PLID 64374 - Toggle resources list enabled
		m_pResources->Enabled = VARIANT_TRUE;
	} NxCatchAll(__FUNCTION__);
}

void CRescheduleAppointmentsDlg::OnBnClickedEditCancellationReasons()
{
	// (a.walling 2015-01-06 15:12) - PLID 64379 - Customize reason list
	try {
		long curID = -1;
		if (auto curSel = m_pReason->CurSel) {
			curID = curSel->Value[glcID];
		}

		int nRet = CEditComboBox(this, 12 /*CANCEL_REASON_COMBO*/, m_pReason, "Edit Cancellation Reasons")
			.OverrideWhere("ID <> -1")
			.DoModal();
		
		// restore selection
		auto newSel = m_pReason->SetSelByColumn(glcID, curID);
		if (!newSel) {
			m_pReason->SetSelByColumn(glcID, -1L);
		}

	} NxCatchAll(__FUNCTION__);
}

void CRescheduleAppointmentsDlg::OnBnClickedOk()
{
	try {
		auto info = ValidateInfo();

		if (!info) {
			return;
		}

		// may add exclusions to returned info
		info = ConfirmRescheduleAppointments(this, info);
		if (!info) {
			return;
		}

		if (!RescheduleAppointments(this, info)) {
			return;
		}

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

void CRescheduleAppointmentsDlg::OnBnClickedRescheduleAll()
{
	// (a.walling 2015-01-06 14:28) - PLID 64376 - Reschedule All Appointments checkbox disables time fields as appropriate
	try {
		VARIANT_BOOL enable = IsDlgButtonChecked(IDC_RESCHEDULE_ALL) ? VARIANT_FALSE : VARIANT_TRUE;

		m_pTimeFrom->Enabled = enable;
		m_pTimeTo->Enabled = enable;

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2015-01-06 14:37) - PLID 64375 - Start and End date
void CRescheduleAppointmentsDlg::OnDtnDatetimechangeDateStart(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	
	// (a.walling 2015-01-06 14:37) - PLID 64375 - Set new range for end date
	try {
		COleDateTime dtStart = AsDateNoTime(COleDateTime(pDTChange->st));
		COleDateTime dtEnd;
		m_dtpTo.GetTime(dtEnd);
		
		// update range
		m_dtpTo.SetRange(&dtStart, nullptr);

		// reset End to Start time if invalid or <
		if ((dtEnd.GetStatus() != COleDateTime::valid) || (dtEnd < dtStart) ) {
			m_dtpTo.SetTime(dtStart);
		}
	} NxCatchAll(__FUNCTION__);


	*pResult = 0;
}

