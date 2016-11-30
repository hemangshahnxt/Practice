// RoomManagerConfigureColumnsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "RoomManagerConfigureColumnsDlg.h"
#include "RoomManagerDlg.h"

// (d.lange 2010-06-28 12:22) - PLID 37317 - created
// CRoomManagerConfigureColumnsDlg dialog

enum RoomManagerConfigureColumns{
	rmcCheck = 0,
	rmcColumnName,
};

IMPLEMENT_DYNAMIC(CRoomManagerConfigureColumnsDlg, CNxDialog)

CRoomManagerConfigureColumnsDlg::CRoomManagerConfigureColumnsDlg(CRoomManagerDlg* pParent /*=NULL*/)
	: CNxDialog(CRoomManagerConfigureColumnsDlg::IDD, pParent)
{
	m_pParentRoomManager = pParent;
}

CRoomManagerConfigureColumnsDlg::~CRoomManagerConfigureColumnsDlg()
{
}

void CRoomManagerConfigureColumnsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
}


BEGIN_MESSAGE_MAP(CRoomManagerConfigureColumnsDlg, CNxDialog)
END_MESSAGE_MAP()


// CRoomManagerConfigureColumnsDlg message handlers
BOOL CRoomManagerConfigureColumnsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//NxIconify the close button
		m_btnOK.AutoSet(NXB_CLOSE);

		m_pWaitingAreaColumns = BindNxDataList2Ctrl(this, IDC_WAITING_AREA_COLUMNS, GetRemoteData(), false);
		m_pRoomsColumns = BindNxDataList2Ctrl(this, IDC_ROOMS_COLUMNS, GetRemoteData(), false);
		m_pCheckoutColumns = BindNxDataList2Ctrl(this, IDC_CHECKOUT_COLUMNS, GetRemoteData(), false);

		//Configure column names for Waiting Area
		m_aryWaitingAreaColumnNames.Add("Room");	// (j.jones 2010-12-01 17:44) - PLID 38597 - added Waiting Room
		m_aryWaitingAreaColumnNames.Add("Patient Name");
		// (j.armen 2012-03-28 09:59) - PLID 48480 - Only show the recall option if they have recall
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
			m_aryWaitingAreaColumnNames.Add("Recall");	// (j.armen 2012-03-05 11:16) - PLID 48555 - Added Recall
		}
		m_aryWaitingAreaColumnNames.Add("Appointment Time");
		m_aryWaitingAreaColumnNames.Add("Purpose");
		m_aryWaitingAreaColumnNames.Add("Checked In");
		m_aryWaitingAreaColumnNames.Add("Last Seen");
		m_aryWaitingAreaColumnNames.Add("Waiting");
		// (d.lange 2010-08-30 17:30) - PLID 39431 - Added Provider column
		m_aryWaitingAreaColumnNames.Add("Provider");
		m_aryWaitingAreaColumnNames.Add("Checked In By");
		m_aryWaitingAreaColumnNames.Add("Resources");// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			m_aryWaitingAreaColumnNames.Add("Preview EMN");		// (d.lange 2010-11-29 17:09) - PLID 40295 - Added Preview EMNs column
		}

		//Configure column names for Rooms
		m_aryRoomsColumnNames.Add("Room");
		m_aryRoomsColumnNames.Add("Patient Name");
		// (j.armen 2012-03-28 09:59) - PLID 48480 - Only show the recall option if they have recall
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
			m_aryRoomsColumnNames.Add("Recall");	// (j.armen 2012-03-05 11:16) - PLID 48555 - Added Recall
		}
		m_aryRoomsColumnNames.Add("Purpose");
		// (j.jones 2010-08-27 09:29) - PLID 39774 - added appt. time
		m_aryRoomsColumnNames.Add("Appointment Time");
		m_aryRoomsColumnNames.Add("Arrival Time");
		m_aryRoomsColumnNames.Add("Checked In");
		m_aryRoomsColumnNames.Add("Last Seen");
		m_aryRoomsColumnNames.Add("Status");
		m_aryRoomsColumnNames.Add("Waiting");
		// (d.lange 2010-08-27 18:02) - PLID 39431 - Added Provider column
		m_aryRoomsColumnNames.Add("Provider");
		m_aryRoomsColumnNames.Add("Last Updated By");
		m_aryRoomsColumnNames.Add("Resources");// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			m_aryRoomsColumnNames.Add("Preview EMN");		// (d.lange 2010-11-29 17:09) - PLID 40295 - Added Preview EMNs column
		}

		//Configure column names for Checkout
		m_aryCheckoutColumnNames.Add("Patient Name");
		// (j.armen 2012-03-28 09:59) - PLID 48480 - Only show the recall option if they have recall
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
			m_aryCheckoutColumnNames.Add("Recall");	// (j.armen 2012-03-05 11:16) - PLID 48555 - Added Recall
		}
		m_aryCheckoutColumnNames.Add("Appointment Time");
		m_aryCheckoutColumnNames.Add("Purpose");
		m_aryCheckoutColumnNames.Add("Checked In");
		m_aryCheckoutColumnNames.Add("Time Left Room");
		m_aryCheckoutColumnNames.Add("Waiting");
		// (d.lange 2010-08-30 17:30) - PLID 39431 - added provider column
		m_aryCheckoutColumnNames.Add("Provider");
		m_aryCheckoutColumnNames.Add("Last Updated By");
		m_aryCheckoutColumnNames.Add("Resources");// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			m_aryCheckoutColumnNames.Add("Preview EMN");		// (d.lange 2010-11-29 17:09) - PLID 40295 - Added Preview EMNs column
		}

		LoadWaitingAreaColumns();
		LoadRoomsColumns();
		LoadCheckoutColumns();

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CRoomManagerConfigureColumnsDlg::LoadWaitingAreaColumns()
{
	try{
		m_pWaitingAreaColumns->Clear();

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		for(int i = 0; i < m_aryWaitingAreaColumnNames.GetSize(); i++) {
			long nDefault = 1;
			pRow = m_pWaitingAreaColumns->GetNewRow();

			VARIANT_BOOL bCheck = VARIANT_FALSE;
				// (d.lange 2010-08-30 17:52) - PLID 39431 - added provider column, hidden by default
			// (j.jones 2010-12-01 17:47) - PLID 38597 - also hide the waiting room name by default
			if(m_aryWaitingAreaColumnNames.GetAt(i).Compare("Last Seen") == 0 || m_aryWaitingAreaColumnNames.GetAt(i).Compare("Provider") == 0
				 || m_aryWaitingAreaColumnNames.GetAt(i).Compare("Room") == 0) {
				//By default lets not show these columns
				nDefault = 0;
			}
			// (d.lange 2010-11-10 12:47) - PLID 43431 - Rewrote so we were only calling GetRemotePropertyInt once
			if(1 == GetRemotePropertyInt(FormatString("ShowRoomManagerWaitingAreaColumn%s", m_aryWaitingAreaColumnNames.GetAt(i)), nDefault, 0, GetCurrentUserName(), true)) {
					bCheck = VARIANT_TRUE;
			}

			pRow->PutValue(rmcCheck, bCheck);
			pRow->PutValue(rmcColumnName, _bstr_t(m_aryWaitingAreaColumnNames.GetAt(i)));
			m_pWaitingAreaColumns->AddRowSorted(pRow, NULL);
		}

	} NxCatchAll(__FUNCTION__);
}

void CRoomManagerConfigureColumnsDlg::LoadRoomsColumns()
{
	try{
		m_pRoomsColumns->Clear();

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		for(int i = 0; i < m_aryRoomsColumnNames.GetSize(); i++) {
			long nDefault = 1;
			pRow = m_pRoomsColumns->GetNewRow();

			VARIANT_BOOL bCheck = VARIANT_FALSE;
			//We want the 'Last Seen' column to be hidden by default
			// (j.jones 2010-08-27 09:29) - PLID 39774 - added appt. time, also hidden by default
			// (d.lange 2010-08-30 17:51) - PLID 39431 - added Provider column, also hidden by default
			if(m_aryRoomsColumnNames.GetAt(i).Compare("Last Seen") == 0 || m_aryRoomsColumnNames.GetAt(i).Compare("Appointment Time") == 0 || 
				m_aryRoomsColumnNames.GetAt(i).Compare("Provider") == 0) {				
				nDefault = 0;
			}
			// (d.lange 2010-11-10 12:47) - PLID 43431 - Rewrote so we were only calling GetRemotePropertyInt once
			if(1 == GetRemotePropertyInt(FormatString("ShowRoomManagerRoomsColumn%s", m_aryRoomsColumnNames.GetAt(i)), nDefault, 0, GetCurrentUserName(), true)) {
					bCheck = VARIANT_TRUE;
			}

			pRow->PutValue(rmcCheck, bCheck);
			pRow->PutValue(rmcColumnName, _bstr_t(m_aryRoomsColumnNames.GetAt(i)));
			m_pRoomsColumns->AddRowSorted(pRow, NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

void CRoomManagerConfigureColumnsDlg::LoadCheckoutColumns()
{
	try{
		m_pCheckoutColumns->Clear();

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		for(int i = 0; i < m_aryCheckoutColumnNames.GetSize(); i++) {
			long nDefault = 1;
			pRow = m_pCheckoutColumns->GetNewRow();

			VARIANT_BOOL bCheck = VARIANT_FALSE;
			// (d.lange 2010-08-31 08:53) - PLID 39431 - added provider column, hidden by default
			if(m_aryCheckoutColumnNames.GetAt(i).Compare("Checked In") == 0 || m_aryCheckoutColumnNames.GetAt(i).Compare("Provider") == 0) {
				nDefault = 0;
			}
			// (d.lange 2010-11-10 12:47) - PLID 43431 - Rewrote so we were only calling GetRemotePropertyInt once
			if(1 == GetRemotePropertyInt(FormatString("ShowRoomManagerCheckoutColumn%s", m_aryCheckoutColumnNames.GetAt(i)), nDefault, 0, GetCurrentUserName(), true)) {
				bCheck = VARIANT_TRUE;
			}

			pRow->PutValue(rmcCheck, bCheck);
			pRow->PutValue(rmcColumnName, _bstr_t(m_aryCheckoutColumnNames.GetAt(i)));
			m_pCheckoutColumns->AddRowSorted(pRow, NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CRoomManagerConfigureColumnsDlg, CNxDialog)
	ON_EVENT(CRoomManagerConfigureColumnsDlg, IDC_WAITING_AREA_COLUMNS, 10, CRoomManagerConfigureColumnsDlg::EditingFinishedWaitingAreaColumns, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CRoomManagerConfigureColumnsDlg, IDC_ROOMS_COLUMNS, 10, CRoomManagerConfigureColumnsDlg::EditingFinishedRoomsColumns, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CRoomManagerConfigureColumnsDlg, IDC_CHECKOUT_COLUMNS, 10, CRoomManagerConfigureColumnsDlg::EditingFinishedCheckoutColumns, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CRoomManagerConfigureColumnsDlg::EditingFinishedWaitingAreaColumns(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		switch(nCol) {
			case rmcCheck:
				long nIntParam = VarBool(varNewValue);
				SetRemotePropertyInt(FormatString("ShowRoomManagerWaitingAreaColumn%s", VarString(pRow->GetValue(rmcColumnName))), nIntParam, 0, GetCurrentUserName());

				if(m_pParentRoomManager) {
					m_pParentRoomManager->EnsureWaitingAreaColumns();
				}
				break;
		}

	} NxCatchAll(__FUNCTION__);
}

void CRoomManagerConfigureColumnsDlg::EditingFinishedRoomsColumns(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		switch(nCol) {
			case rmcCheck:
				long nIntParam = VarBool(varNewValue);
				SetRemotePropertyInt(FormatString("ShowRoomManagerRoomsColumn%s", VarString(pRow->GetValue(rmcColumnName))), nIntParam, 0, GetCurrentUserName());

				if(m_pParentRoomManager) {
					m_pParentRoomManager->EnsureRoomsColumns();
				}
				break;
		}

	} NxCatchAll(__FUNCTION__);
}

void CRoomManagerConfigureColumnsDlg::EditingFinishedCheckoutColumns(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		switch(nCol) {
			case rmcCheck:
				long nIntParam = VarBool(varNewValue);
				SetRemotePropertyInt(FormatString("ShowRoomManagerCheckoutColumn%s", VarString(pRow->GetValue(rmcColumnName))), nIntParam, 0, GetCurrentUserName());

				if(m_pParentRoomManager) {
					m_pParentRoomManager->EnsureCheckoutColumns();
				}
				break;
		}

	} NxCatchAll(__FUNCTION__);
}
