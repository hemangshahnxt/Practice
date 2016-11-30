// RecallLinkDlg.cpp : implementation file
//(a.wilson 2012-3-5) PLID 48420 - created for the recall system.

#include "stdafx.h"
#include "Practice.h"
#include "RecallLinkDlg.h"
#include "RecallUtils.h"

using namespace ADODB;

// CRecallLinkDlg dialog
enum eActiveListColumns 
{
	alcID = 0,
	alcDate,
	alcStatus,
	// (j.jones 2016-02-18 11:20) - PLID 68350 - added provider & location
	alcProviderName,
	alcLocationName,
	alcTemplateName,
};

IMPLEMENT_DYNAMIC(CRecallLinkDlg, CNxDialog)

//default constructor
CRecallLinkDlg::CRecallLinkDlg(const long& nPatientID, const long& nAppointmentID, CWnd* pParent)
	: CNxDialog(CRecallLinkDlg::IDD, pParent), 
	  m_nPatientID(nPatientID), 
	  m_nAppointmentID(nAppointmentID)
{
}

CRecallLinkDlg::~CRecallLinkDlg()
{
}

void CRecallLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RECALL_LINK_BACKGROUND, m_nxcBackground);
	DDX_Control(pDX, IDOK, m_nxbLink);
	DDX_Control(pDX, IDCANCEL, m_nxbClose);
}

BEGIN_MESSAGE_MAP(CRecallLinkDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CRecallLinkDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CRecallLinkDlg, CNxDialog)
	ON_EVENT(CRecallLinkDlg, IDC_ACTIVE_LIST, 1, CRecallLinkDlg::SelChangingActiveList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

//(a.wilson 2012-3-20) PLID 48420 - take the carray of recalls and enter them into the datalist.
BOOL CRecallLinkDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		CString strPatient = ("Active Recalls for " + GetExistingPatientName(m_nPatientID));
		SetWindowText(strPatient);

		m_nxbLink.AutoSet(NXB_OK);
		m_nxbClose.AutoSet(NXB_CLOSE);

		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_pList = BindNxDataList2Ctrl(IDC_ACTIVE_LIST, false);

		// (j.armen 2012-07-23 10:48) - PLID 51600 - Don't select a default recall
		for (int i = 0; i < m_arRecallInfo.GetCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
			const RecallInfo &rInfo = m_arRecallInfo[i];
			pRow->PutValue(alcID, rInfo.nID);									//ID
			pRow->PutValue(alcDate, _variant_t(rInfo.dtDate, VT_DATE));			//Date
			pRow->PutValue(alcStatus, _bstr_t(rInfo.strStatus));				//Status
			pRow->PutBackColor(rInfo.nStatusColor);								//StatusColor
			// (j.jones 2016-02-18 11:20) - PLID 68350 - added provider & location
			pRow->PutValue(alcProviderName, _bstr_t(rInfo.strProviderName));	//Provider Name
			pRow->PutValue(alcLocationName, _bstr_t(rInfo.strLocationName));	//Location Name
			pRow->PutValue(alcTemplateName, _bstr_t(rInfo.strTemplateName));	//Template Name
			m_pList->AddRowSorted(pRow, NULL);
		}
	} NxCatchAll(__FUNCTION__);
	
	return TRUE;
}
//(a.wilson 2012-3-5) PLID 48420 - we need to check to see if the current patient has any active recalls before we 
//create the dialog.
bool CRecallLinkDlg::HasRecalls()
{
	try {
		if (m_nPatientID > 0 && m_nAppointmentID > 0) {
			
			using namespace RecallUtils;
			{
				// (a.walling 2013-12-12 16:51) - PLID 60007 - Update recalls now
				RecallUtils::UpdateRecalls(m_nPatientID);

				//this query will generate a list of recalls that do not have a recallappointmentid and are not discontinued 
				//for the current patient as long as this appointment isn't already scheduled.
				// (j.armen 2012-03-20 13:38) - PLID 49057 - Avoid ambiguity - Reference the table
				// (j.luckoski 2012-10-09 09:02) - PLID 51693 - Allows recalls linked to a no-show appt to display in the list for relinking
				// (b.savon 2014-12-01 08:29) - PLID 63481 - The recall system needs to prompt if the user schedules an appointment and a cancelled appointment is linked to an outstanding recall.
				_RecordsetPtr rsRecalls = CreateParamRecordset("{SQL} WHERE (RecallT.RecallAppointmentID IS NULL OR RecallT.RecallAppointmentID IN "
					"(Select ID from AppointmentsT WHERE (ShowState = 3 OR AppointmentsT.Status = 4) AND PatientID = {INT})) "
					"AND RecallT.Discontinued = 0 AND RecallT.PatientID = {INT}", RecallUtils::SelectRecalls(), m_nPatientID, m_nPatientID);

				if (!rsRecalls->eof) {

					for (; !rsRecalls->eof; rsRecalls->MoveNext()) {
						RecallInfo rInfo;
						//ID
						rInfo.nID = AdoFldLong(rsRecalls, "RecallID");
						//RecallDate
						rInfo.dtDate = AdoFldDateTime(rsRecalls, "RecallDate");
						//Status
						rInfo.strStatus = AdoFldString(rsRecalls, "RecallStatus");
						//StatusColor
						rInfo.nStatusColor = AdoFldLong(rsRecalls, "RecallStatusColor");
						//Template Name
						rInfo.strTemplateName = AdoFldString(rsRecalls, "RecallTemplate");
						// (j.jones 2016-02-18 11:19) - PLID 68350 - added provider & location
						rInfo.strProviderName = AdoFldString(rsRecalls, "ProviderName", "");
						rInfo.strLocationName = AdoFldString(rsRecalls, "LocationName", "");
						m_arRecallInfo.Add(rInfo);
					}
					return true;
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return false;
}

//(a.wilson 2012-3-5) PLID 48420 - if a valid recall was selected then we need to update the recall table for that row.
//otherwise, we need to let them know they didnt select a valid recall.
void CRecallLinkDlg::OnBnClickedOk()
{
	try {
		// (j.armen 2012-03-28 11:19) - PLID 48480 - Use our license
		if(!g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrUse)) {
			return;
		}
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetCurSel();

		if (pRow) {
			long nRecallID;
			nRecallID = VarLong(pRow->GetValue(alcID), -1);

			if (nRecallID > 0 && m_nAppointmentID > 0 && m_nPatientID > 0) {

				// (j.jones 2016-02-18 09:30) - PLID 68350 - warn if the recall already has a provider or location
				// that is different from the appointment's provider/location
				bool bWasWarned = false;	//this isn't used
				if (!RecallUtils::CanLinkRecallAndApptByProviderLocation(this, nRecallID, m_nAppointmentID, bWasWarned)) {
					return;
				}

				// (j.luckoski 2012-10-09 08:45) - PLID 51693 - Allow recalls linked to no-show appts to be re-linked
				// (b.savon 2014-12-01 08:29) - PLID 63481 - The recall system needs to prompt if the user schedules an appointment and a cancelled appointment is linked to an outstanding recall.
				ExecuteParamSql("UPDATE RecallT SET RecallAppointmentID = {INT} WHERE ID = {INT} AND PatientID = {INT} "
					"AND (RecallAppointmentID IS NULL OR RecallAppointmentID IN (SELECT ID FROM AppointmentsT WHERE "
					" (ShowState = 3 OR AppointmentsT.Status = 4) AND PatientID = {INT}))"
					, m_nAppointmentID, nRecallID, m_nPatientID, m_nPatientID);

				// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
				GetMainFrame()->HandleRecallChanged();
			}
		} else {
			MsgBox("Not a valid selection.  Please select an active recall entry you would "
				"like to link to this appointment or press cancel.");
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-07-23 10:51) - PLID 51600 - Enable/Disable the Link button based on selection
void CRecallLinkDlg::SelChangingActiveList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		m_nxbLink.EnableWindow((*lppNewSel) ? TRUE : FALSE);
	}NxCatchAll(__FUNCTION__);
}
