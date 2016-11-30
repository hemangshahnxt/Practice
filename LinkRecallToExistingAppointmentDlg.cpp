// LinkRecallToExistingAppointmentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LinkRecallToExistingAppointmentDlg.h"
#include "AuditTrail.h"
#include "RecallUtils.h"

// (b.savon 2012-03-01 11:56) - PLID 48486 - Created

// CLinkRecallToExistingAppointmentDlg dialog

IMPLEMENT_DYNAMIC(CLinkRecallToExistingAppointmentDlg, CNxDialog)

using namespace ADODB;

CLinkRecallToExistingAppointmentDlg::CLinkRecallToExistingAppointmentDlg(const long &nPatientID, const CString &strPatientName, const long &nRecallID, const CString &strTemplateName, CWnd* pParent /*=NULL*/)
	: CNxDialog(CLinkRecallToExistingAppointmentDlg::IDD, pParent)
{
	m_nPatientID = nPatientID;
	m_strPatientName = strPatientName;
	m_nRecallID = nRecallID;
	m_pSelectedRow = NULL;
	m_strTemplateName = strTemplateName;
}

CLinkRecallToExistingAppointmentDlg::~CLinkRecallToExistingAppointmentDlg()
{
}

BOOL CLinkRecallToExistingAppointmentDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{ 
		//Prepare controls and window
		m_btnCreate.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_cBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		CString strWindowTitle;
		CString strWindowAppend;
		strWindowAppend.Format(" for %s (%li) ", m_strPatientName, GetExistingPatientUserDefinedID(m_nPatientID) );
		GetWindowText(strWindowTitle);
		SetWindowText(strWindowTitle+strWindowAppend);
		
		CString strHeading;
		strHeading.Format("%s (%li) has the following upcoming appointments.  Select one from the list and click 'Link' to assign the appointment for this recall.", m_strPatientName, GetExistingPatientUserDefinedID(m_nPatientID));

		m_lblDialogHeading.SetWindowTextA( strHeading );

		m_pnxdlExistingAppointments = BindNxDataList2Ctrl(IDC_NXDL_PT_EXISTING_APPTS, false);
		CSqlFragment sqlWhere = CSqlFragment("AppointmentsT.PatientID = {INT} \r\n AND AppointmentsT.Status <> 4 \r\n AND dbo.AsDateNoTime(AppointmentsT.Date) >= dbo.AsDateNoTime(GETDATE()) \r\n", m_nPatientID);
		m_pnxdlExistingAppointments->PutWhereClause(_bstr_t(sqlWhere.Flatten()));
		m_pnxdlExistingAppointments->Requery();

		UpdateButtons();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CLinkRecallToExistingAppointmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnCreate);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_EXISTING_APPT_DESC, m_lblDialogHeading);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_cBackground);
}


BEGIN_MESSAGE_MAP(CLinkRecallToExistingAppointmentDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CLinkRecallToExistingAppointmentDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PREVIOUS, &CLinkRecallToExistingAppointmentDlg::OnBnClickedCheckShowPrevious)
END_MESSAGE_MAP()


// CLinkRecallToExistingAppointmentDlg message handlers

void CLinkRecallToExistingAppointmentDlg::OnBnClickedOk()
{
	try{
		// (j.armen 2012-03-28 11:03) - PLID 48480 - Use our license now
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrUse))
		{
			if( m_pSelectedRow ){

				long nAppointmentID = AsLong(m_pSelectedRow->GetValue(eacID));

				// (j.jones 2016-02-18 09:30) - PLID 68350 - warn if the recall already has a provider or location
				// that is different from the appointment's provider/location
				bool bWasWarned = false;
				if (!RecallUtils::CanLinkRecallAndApptByProviderLocation(this, m_nRecallID, nAppointmentID, bWasWarned)) {
					return;
				}
				
				if(bWasWarned || IDYES == MessageBox("Are you sure you would like to link this appointment with the patient's recall?", "Practice", MB_ICONQUESTION | MB_YESNO ) ){
					// (b.savon 2012-03-20 11:38) - PLID 48471 - Audit Linked Appointments
					CSqlFragment sqlLinked = CSqlFragment(	"SET NOCOUNT OFF\r\n"
															"/* Old Linked Appointment */ \r\n"
															"SELECT RecallT.RecallAppointmentID, \r\n"
															"		RecallTemplateT.Name, \r\n"
															"		AppointmentsT.StartTime, \r\n"
															"		dbo.GetPurposeString(AppointmentsT.ID) AS Purpose \r\n"
															"FROM	RecallT \r\n"
															"		INNER JOIN AppointmentsT ON RecallT.RecallAppointmentID = AppointmentsT.ID \r\n"
															"		INNER JOIN RecallTemplateT ON RecallT.RecallTemplateID = RecallTemplateT.ID \r\n"
															"WHERE RecallT.ID = {INT} \r\n\r\n"
															,
															m_nRecallID
														 );

					CSqlFragment sqlUpdate = CSqlFragment("SET NOCOUNT ON UPDATE RecallT SET RecallAppointmentID = {INT} WHERE ID = {INT}\r\n", nAppointmentID, m_nRecallID);
					_RecordsetPtr prs = CreateParamRecordset( sqlLinked + sqlUpdate );

					// (b.savon 2012-03-20 10:19) - PLID 48471 - Auditing Appt Linking
					CString strFormattedDate = "", strPurpose = "", strRecallTemplate = "";
					CString strOldValue = "", strNewValue = "";
					while(!prs->eof){
						strFormattedDate = FormatDateTimeForInterface(AdoFldDateTime(prs->Fields, "StartTime", COleDateTime(0, 0, 0, 0, 0, 0)));
						strPurpose = AdoFldString(prs->Fields, "Purpose", "");
						strRecallTemplate = AdoFldString(prs->Fields, "Name", "");

						prs->MoveNext();
					}
					prs->Close();

					strOldValue = (strPurpose == "" && strRecallTemplate == "") ? "" : "Recall: " + strRecallTemplate + " - Appointment: " + strFormattedDate + " - Purpose: " + strPurpose;
					strNewValue =   "Recall: " + m_strTemplateName + 
									" - Appointment: " + FormatDateTimeForInterface(AsDateTime(m_pSelectedRow->GetValue(eacDate))) + " " + FormatDateTimeForInterface(AsDateTime(m_pSelectedRow->GetValue(eacTime))) +
									" - Purpose: " + AsString(m_pSelectedRow->GetValue(eacPurpose));

					long nAuditID = BeginNewAuditEvent();
					AuditEvent(	m_nPatientID, 
								m_strPatientName, 
								nAuditID, 
								aeiPatientRecallLinkedAppointment, 
								m_nRecallID, 
								strOldValue, 
								strNewValue,
								aepMedium, 
								aetChanged);

					// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
					GetMainFrame()->HandleRecallChanged();

					CNxDialog::OnOK();
				}

			}else{
				MessageBox("Please select an appointment to link and try again.", "Practice", MB_ICONINFORMATION);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CLinkRecallToExistingAppointmentDlg, CNxDialog)
	ON_EVENT(CLinkRecallToExistingAppointmentDlg, IDC_NXDL_PT_EXISTING_APPTS, 29, CLinkRecallToExistingAppointmentDlg::SelSetNxdlPtExistingAppts, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CLinkRecallToExistingAppointmentDlg::UpdateButtons()
{
	if( m_pSelectedRow ){
		m_btnCreate.EnableWindow(TRUE);
	}else{
		m_btnCreate.EnableWindow(FALSE);
	}
}

void CLinkRecallToExistingAppointmentDlg::SelSetNxdlPtExistingAppts(LPDISPATCH lpSel)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpSel);

		if( pRow ){
			m_pSelectedRow = pRow;
		}else{
			m_pSelectedRow = NULL;
		}

		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

void CLinkRecallToExistingAppointmentDlg::OnBnClickedCheckShowPrevious()
{
	try{
		CSqlFragment sqlWhere;

		//Adjust our query, and requery the DL
		if( IsDlgButtonChecked(IDC_CHECK_SHOW_PREVIOUS) ){
			sqlWhere = CSqlFragment("AppointmentsT.PatientID = {INT} \r\n AND AppointmentsT.Status <> 4 \r\n \r\n", m_nPatientID);
		}else{
			sqlWhere = CSqlFragment("AppointmentsT.PatientID = {INT} \r\n AND AppointmentsT.Status <> 4 \r\n AND dbo.AsDateNoTime(AppointmentsT.Date) >= dbo.AsDateNoTime(GETDATE()) \r\n", m_nPatientID);
		}

		m_pnxdlExistingAppointments->PutWhereClause(_bstr_t(sqlWhere.Flatten()));
		m_pnxdlExistingAppointments->Requery();

		m_pSelectedRow = NULL;

		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}