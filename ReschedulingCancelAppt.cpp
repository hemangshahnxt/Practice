// (r.goldschmidt 2014-12-23 18:14) - PLID 64383 - Created
// ReschedulingCancelAppt.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ReschedulingCancelAppt.h"
#include "afxdialogex.h"


// CReschedulingCancelAppt dialog
//

enum EReschedulingCancelAppointmentListColumns {
	rcalcID,
	rcalcSelected,
	rcalcPatientName,
	rcalcDate,
	rcalcPurpose,
	rcalcResources,
	rcalcLocation,
	rcalcNotes,
	rcalcHasActiveResources
};

IMPLEMENT_DYNAMIC(CReschedulingCancelAppt, CNxDialog)

CReschedulingCancelAppt::CReschedulingCancelAppt(CWnd* pParent /*= NULL*/)
	: CNxDialog(CReschedulingCancelAppt::IDD, pParent)
{
}

CReschedulingCancelAppt::~CReschedulingCancelAppt()
{
}

void CReschedulingCancelAppt::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_NXCOLOR_CANCEL_APPT, m_background);
	DDX_Control(pDX, IDC_RADIO_SHOW_ALL_APPTS, m_radioShowAllAppts);
	DDX_Control(pDX, IDC_RADIO_SHOW_APPTS_WITH_MULTIPLE_RESOURCES, m_radioShowApptsWithOtherResources);
}

BOOL CReschedulingCancelAppt::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{		
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDOK))->AutoSet(NXB_DELETE);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);

		m_background.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_pListAppointments = BindNxDataList2Ctrl(IDC_LISTAPPOINTMENTS, false);
		m_pListAppointments->FromClause = _bstr_t(m_strAppointmentListFromClause);
		m_pListAppointments->WhereClause = "";
		m_pListAppointments->Requery();

		SecureControls();
		ToggleRows();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (r.goldschmidt 2014-12-31 11:15) - PLID 64383 - Take rescheduling info to build appointment list for review
// (r.goldschmidt 2015-01-28 13:20) - PLID 64384 - Past dates are now allowed; account for NULL strings
void CReschedulingCancelAppt::SetAppointmentListFromClause(const Nx::Scheduler::RescheduleAppointmentsInfo& info)
{
	CSqlFragment sqlHasActiveResources, sqlResources;

	// if resources vector is empty, then the all resources radio button was selected.
	//  therefore, all appointments that are filtered can't possibly have unselected resources (sqlHasActiveResources)
	//  and all appointments filtered don't need additional filtering on the resource list (sqlResources)
	// if resources vector isn't empty, then we need to differentiate between appointments that have unselected resources
	//  and appointments with all the resources selected (sqlHasActiveResources)
	//  and we need to have an additional filter to only collect appointments that contain selected resources (sqlResources)
	if (info.resources.empty()) {
		sqlHasActiveResources.Create("CONVERT(BIT, 0)");
	} else {
		sqlHasActiveResources.Create(
			R"(CASE WHEN AppointmentsT.ID IN
			(SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID NOT IN ({INTVECTOR}))
		THEN Convert(BIT, 1) 
		ELSE Convert(BIT, 0)
		END)", info.resources);
		sqlResources.Create("AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN ({INTVECTOR}))", info.resources);
	}

	// the where clause filtering is borrowed from Nx::Scheduler::MakeFromClause(info)
	CSqlFragment sqlFrom;
	sqlFrom.Create(
		R"(
(SELECT 
	AppointmentsT.ID,
	Convert(BIT, 1) AS Checked,
	PersonT.FullName AS PatientName,
	AppointmentsT.StartTime,
	AptTypeT.Name + COALESCE(' - ' + dbo.GetPurposeString(AppointmentsT.ID), '') AS Purpose,
	dbo.GetResourceString(AppointmentsT.ID) AS Resources,
	LocationsT.Name AS Location,
	AppointmentsT.Notes,
	{SQL} AS HasActiveResources
FROM 
	AppointmentsT
	LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID
	LEFT JOIN LocationsT ON LocationsT.ID = AppointmentsT.LocationID
	LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID
WHERE 
	AppointmentsT.STATUS <> 4
	AND StartTime >= {OLEDATETIME}
	AND StartTime < {OLEDATETIME}
	AND 
	(
		dbo.AsTimeNoDate(EndTime) >= {OLEDATETIME}
		AND
		dbo.AsTimeNoDate(StartTime) < {OLEDATETIME}
	)
	{SQL}
) AS SubQ
		)"
		, sqlHasActiveResources
		, info.from
		, info.to + COleDateTimeSpan(1, 0, 0, 0) // since the range is exclusive at to, [from, to), add a day
		, g_cdtSqlZero + info.offsetFrom
		, g_cdtSqlZero + info.offsetTo
		, sqlResources
	);

	m_strAppointmentListFromClause = sqlFrom.Flatten();
}

BEGIN_MESSAGE_MAP(CReschedulingCancelAppt, CNxDialog)
	ON_BN_CLICKED(IDC_UNCHECKALL, &CReschedulingCancelAppt::OnBnClickedUncheckall)
	ON_BN_CLICKED(IDC_CHECKALL, &CReschedulingCancelAppt::OnBnClickedCheckall)
	ON_BN_CLICKED(IDOK, &CReschedulingCancelAppt::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_SHOW_APPTS_WITH_MULTIPLE_RESOURCES, &CReschedulingCancelAppt::OnBnClickedRadioShowApptsWithMultipleResources)
	ON_BN_CLICKED(IDC_RADIO_SHOW_ALL_APPTS, &CReschedulingCancelAppt::OnBnClickedRadioShowAllAppts)
END_MESSAGE_MAP()


// CReschedulingCancelAppt message handlers

// (r.goldschmidt 2014-12-30 10:25) - PLID 64385 - uncheck all showing appointments
void CReschedulingCancelAppt::OnBnClickedUncheckall()
{
	try{
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListAppointments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (pRow->GetVisible()){
				pRow->PutValue(rcalcSelected, g_cvarFalse);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-12-30 10:25) - PLID 64385 - check all showing appointments
void CReschedulingCancelAppt::OnBnClickedCheckall()
{
	try{
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListAppointments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (pRow->GetVisible()){
				pRow->PutValue(rcalcSelected, g_cvarTrue);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-12-30 16:23) - PLID 64383 - after this, the parent will need to get the vector of excluded appts before performing the mass cancel/reschedule
void CReschedulingCancelAppt::OnBnClickedOk()
{
	try{
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListAppointments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (!pRow->GetValue(rcalcSelected)){
				m_vecAppointmentIDsNotCancelled.push_back(pRow->GetValue(rcalcID));
			}
		}
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CReschedulingCancelAppt::OnCancel()
{
	try{
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

void CReschedulingCancelAppt::SecureControls()
{
	m_radioShowAllAppts.SetCheck(m_bShowAll);
	m_radioShowApptsWithOtherResources.SetCheck(!m_bShowAll);
	m_radioShowApptsWithOtherResources.EnableWindow(!m_bShowAll);

}

// (r.goldschmidt 2015-01-12 15:54) - checks status of checkboxes to decide which appointments to display
void CReschedulingCancelAppt::ToggleRows()
{
	// make sure we finish the requery before showing/hiding rows (otherwise all rows get shown no matter what)
	m_pListAppointments->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pListAppointments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()){
		if (m_radioShowApptsWithOtherResources.GetCheck() && !pRow->GetValue(rcalcHasActiveResources)){
			pRow->PutVisible(VARIANT_FALSE);
		}
		else {
			pRow->PutVisible(VARIANT_TRUE);
		}
	}
}

// (r.goldschmidt 2015-01-12 15:54) - radio button to show appointments only with resources not requested to be cancelled
void CReschedulingCancelAppt::OnBnClickedRadioShowApptsWithMultipleResources()
{
	try{
		m_radioShowAllAppts.SetCheck(FALSE);
		ToggleRows();
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2015-01-12 15:54) - radio button to show all appointments with a resource requested to be cancelled
void CReschedulingCancelAppt::OnBnClickedRadioShowAllAppts()
{
	try{
		m_radioShowApptsWithOtherResources.SetCheck(FALSE);
		ToggleRows();
	}NxCatchAll(__FUNCTION__);
}
