// NexERxConfigureLicenses.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexERxConfigureLicenses.h"
#include "AuditTrail.h"

// CNexERxConfigureLicenses dialog
// (b.savon 2013-01-31 14:18) - PLID 54964 - Created

enum ENexERxLicenseColumn{
	nelcID = 0,
	nelcName,
};

IMPLEMENT_DYNAMIC(CNexERxConfigureLicenses, CNxDialog)

void CNexERxConfigureLicenses::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_LP_REQUEST, m_btnLicensedPrescriberRequest);
	DDX_Control(pDX, IDC_LP_DEACTIVATE, m_btnLicensedPrescriberDeactivate);
	DDX_Control(pDX, IDC_ML_REQUEST, m_btnMidlevelPrescriberRequest);
	DDX_Control(pDX, IDC_ML_DEACTIVATE, m_btnMidlevelPrescriberDeactivate);
	DDX_Control(pDX, IDC_NS_REQUEST, m_btnNurseStaffRequest);
	DDX_Control(pDX, IDC_NS_DEACTIVATE, m_btnNurseStaffDeactivate);
	DDX_Control(pDX, IDC_NXC_NEXERX_LICENSE_BACK, m_nxcBack);
}

CNexERxConfigureLicenses::CNexERxConfigureLicenses(CNexERxSetupDlg* pCaller, CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexERxConfigureLicenses::IDD, pParent)
{
	ASSERT(pCaller!=NULL); //You should only be calling this dialog from NexERxSetupDlg

	m_pSetup = pCaller;
}

CNexERxConfigureLicenses::~CNexERxConfigureLicenses()
{
}

BOOL CNexERxConfigureLicenses::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		
		// Dialog Setup
		SetTitleBarIcon(IDI_ERX);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnLicensedPrescriberRequest.AutoSet(NXB_RIGHT);
		m_btnLicensedPrescriberDeactivate.AutoSet(NXB_RIGHT);
		m_btnMidlevelPrescriberRequest.AutoSet(NXB_RIGHT);
		m_btnMidlevelPrescriberDeactivate.AutoSet(NXB_RIGHT);
		m_btnNurseStaffRequest.AutoSet(NXB_RIGHT);
		m_btnNurseStaffDeactivate.AutoSet(NXB_RIGHT);
		m_nxcBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// Datalist Setup
		m_nxdlLicensedPrescriberAvailable = BindNxDataList2Ctrl(IDC_NXDL_AVAILABLE_LP, false);
		m_nxdlLicensedPrescriberLicensed = BindNxDataList2Ctrl(IDC_NXDL_LICENSED_LP, false);
		m_nxdlLicensedPrescriberInactive = BindNxDataList2Ctrl(IDC_NXDL_INACTIVE_LP, false);
		m_nxdlMidlevelPrescriberAvailable = BindNxDataList2Ctrl(IDC_NXDL_AVAILABLE_ML, false);
		m_nxdlMidlevelPrescriberLicensed = BindNxDataList2Ctrl(IDC_NXDL_LICENSED_ML, false);
		m_nxdlMidlevelPrescriberInactive = BindNxDataList2Ctrl(IDC_NXDL_INACTIVE_ML, false);
		m_nxdlNurseStaffPrescriberAvailable = BindNxDataList2Ctrl(IDC_NXDL_AVAILABLE_NS, false);
		m_nxdlNurseStaffPrescriberLicensed = BindNxDataList2Ctrl(IDC_NXDL_LICENSED_NS, false);
		m_nxdlNurseStaffPrescriberInactive = BindNxDataList2Ctrl(IDC_NXDL_INACTIVE_NS, false);

		LoadDatalists();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNexERxConfigureLicenses::LoadDatalists()
{
	LoadLicensedPrescribersNexERx();
	LoadMidlevelPrescribersNexERx();
	LoadNurseStaffNexERx();
}

void CNexERxConfigureLicenses::LoadLicensedPrescribersNexERx()
{
	try{

		m_nxdlLicensedPrescriberInactive->PutWhereClause((_bstr_t)m_lNexERxLicense.GetInactiveLicensedPrescriberWhereClause());
			
		m_nxdlLicensedPrescriberLicensed->PutWhereClause((_bstr_t)m_lNexERxLicense.GetLicensedPrescriberWhereClause());

		m_nxdlLicensedPrescriberAvailable->PutWhereClause((_bstr_t)m_lNexERxLicense.GetAvailableLicensedPrescriberWhereClause());

		m_bLicensedPrescriberAllReady = false;
		m_bLicensedPrescriberUsedReady = false;
		m_bLicensedPrescriberInactiveReady = false;

		m_nxdlLicensedPrescriberAvailable->Requery();
		m_nxdlLicensedPrescriberLicensed->Requery();
		m_nxdlLicensedPrescriberInactive->Requery();

	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::LoadMidlevelPrescribersNexERx()
{
	try{

		m_nxdlMidlevelPrescriberInactive->PutWhereClause((_bstr_t)m_lNexERxLicense.GetInactiveMidlevelPrescriberWhereClause());

		m_nxdlMidlevelPrescriberLicensed->PutWhereClause((_bstr_t)m_lNexERxLicense.GetLicensedMidlevelPrescriberWhereClause());

		m_nxdlMidlevelPrescriberAvailable->PutWhereClause((_bstr_t)m_lNexERxLicense.GetAvailableMidlevelPrescriberWhereClause());

		m_bMidlevelPrescriberAllReady = false;
		m_bMidlevelPrescriberUsedReady = false;
		m_bMidlevelPrescriberInactiveReady = false;

		m_nxdlMidlevelPrescriberAvailable->Requery();
		m_nxdlMidlevelPrescriberLicensed->Requery();
		m_nxdlMidlevelPrescriberInactive->Requery();

	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::LoadNurseStaffNexERx()
{
	try{
	
		m_nxdlNurseStaffPrescriberInactive->PutWhereClause((_bstr_t)m_lNexERxLicense.GetInactiveNurseStaffWhereClause());
			
		m_nxdlNurseStaffPrescriberLicensed->PutWhereClause((_bstr_t)m_lNexERxLicense.GetLicensedNurseStaffWhereClause());

		m_nxdlNurseStaffPrescriberAvailable->PutWhereClause((_bstr_t)m_lNexERxLicense.GetAvailableNurseStaffWhereClause());

		m_bNurseStaffAllReady = false;
		m_bNurseStaffUsedReady = false;
		m_bNurseStaffInactiveReady = false;

		m_nxdlNurseStaffPrescriberAvailable->Requery();
		m_nxdlNurseStaffPrescriberLicensed->Requery();
		m_nxdlNurseStaffPrescriberInactive->Requery();

	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::EnableLicensedPrescriberButtons()
{
	try{
		if (m_bLicensedPrescriberAllReady && m_bLicensedPrescriberUsedReady && m_bLicensedPrescriberInactiveReady) {
			UpdateInfoLabel();

			if (m_lNexERxLicense.GetUsedLicensedPrescriberCount() > 0) {
				m_btnLicensedPrescriberDeactivate.EnableWindow(TRUE);
			} else {
				m_btnLicensedPrescriberDeactivate.EnableWindow(FALSE);
			}

			if (m_lNexERxLicense.GetUsedLicensedPrescriberCount() == m_lNexERxLicense.GetAllowedLicensedPrescriberCount() ||
				m_nxdlLicensedPrescriberAvailable->GetRowCount() == 0) {
				m_btnLicensedPrescriberRequest.EnableWindow(FALSE);
			} else {
				m_btnLicensedPrescriberRequest.EnableWindow(TRUE);
			}
		} else {
			m_btnLicensedPrescriberDeactivate.EnableWindow(FALSE);
			m_btnLicensedPrescriberRequest.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::EnableMidlevelPrescriberButtons()
{
	try{
		if (m_bMidlevelPrescriberAllReady && m_bMidlevelPrescriberUsedReady && m_bMidlevelPrescriberInactiveReady) {
			UpdateInfoLabel();

			if (m_lNexERxLicense.GetUsedMidlevelPrescriberCount() > 0) {
				m_btnMidlevelPrescriberDeactivate.EnableWindow(TRUE);
			} else {
				m_btnMidlevelPrescriberDeactivate.EnableWindow(FALSE);
			}

			if (m_lNexERxLicense.GetUsedMidlevelPrescriberCount() == m_lNexERxLicense.GetAllowedMidlevelPrescriberCount() ||
				m_nxdlMidlevelPrescriberAvailable->GetRowCount() == 0) {
				m_btnMidlevelPrescriberRequest.EnableWindow(FALSE);
			} else {
				m_btnMidlevelPrescriberRequest.EnableWindow(TRUE);
			}
		} else {
			m_btnMidlevelPrescriberDeactivate.EnableWindow(FALSE);
			m_btnMidlevelPrescriberRequest.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::EnableNurseStaffButtons()
{
	try{
		if (m_bNurseStaffAllReady && m_bNurseStaffUsedReady && m_bNurseStaffInactiveReady) {
			UpdateInfoLabel();

			if (m_lNexERxLicense.GetUsedNurseStaffCount() > 0) {
				m_btnNurseStaffDeactivate.EnableWindow(TRUE);
			} else {
				m_btnNurseStaffDeactivate.EnableWindow(FALSE);
			}

			if (m_lNexERxLicense.GetUsedNurseStaffCount() == m_lNexERxLicense.GetAllowedNurseStaffCount() ||
				m_nxdlNurseStaffPrescriberAvailable->GetRowCount() == 0) {
				m_btnNurseStaffRequest.EnableWindow(FALSE);
			} else {
				m_btnNurseStaffRequest.EnableWindow(TRUE);
			}
		} else {
			m_btnNurseStaffDeactivate.EnableWindow(FALSE);
			m_btnNurseStaffRequest.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::UpdateInfoLabel()
{
	try{
		SetDlgItemText(IDC_STATIC_INFO_LABEL_NEXERX_LICENSES, m_lNexERxLicense.GetLicenseStatisticsMessage());
	}NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CNexERxConfigureLicenses, CNxDialog)
	ON_BN_CLICKED(IDC_LP_REQUEST, &CNexERxConfigureLicenses::OnBnClickedLpRequest)
	ON_BN_CLICKED(IDC_LP_DEACTIVATE, &CNexERxConfigureLicenses::OnBnClickedLpDeactivate)
	ON_BN_CLICKED(IDC_ML_REQUEST, &CNexERxConfigureLicenses::OnBnClickedMlRequest)
	ON_BN_CLICKED(IDC_ML_DEACTIVATE, &CNexERxConfigureLicenses::OnBnClickedMlDeactivate)
	ON_BN_CLICKED(IDC_NS_REQUEST, &CNexERxConfigureLicenses::OnBnClickedNsRequest)
	ON_BN_CLICKED(IDC_NS_DEACTIVATE, &CNexERxConfigureLicenses::OnBnClickedNsDeactivate)
END_MESSAGE_MAP()


// CNexERxConfigureLicenses message handlers
BEGIN_EVENTSINK_MAP(CNexERxConfigureLicenses, CNxDialog)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_AVAILABLE_LP, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlAvailableLp, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_LICENSED_LP, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlLicensedLp, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_INACTIVE_LP, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlInactiveLp, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_AVAILABLE_ML, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlAvailableMl, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_LICENSED_ML, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlLicensedMl, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_INACTIVE_ML, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlInactiveMl, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_AVAILABLE_NS, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlAvailableNs, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_LICENSED_NS, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlLicensedNs, VTS_I2)
	ON_EVENT(CNexERxConfigureLicenses, IDC_NXDL_INACTIVE_NS, 18, CNexERxConfigureLicenses::RequeryFinishedNxdlInactiveNs, VTS_I2)
END_EVENTSINK_MAP()

void CNexERxConfigureLicenses::RequeryFinishedNxdlAvailableLp(short nFlags)
{
	try{
		m_bLicensedPrescriberAllReady = TRUE;
		EnableLicensedPrescriberButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlLicensedLp(short nFlags)
{
	try{
		m_bLicensedPrescriberUsedReady = TRUE;
		EnableLicensedPrescriberButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlInactiveLp(short nFlags)
{
	try{
		m_bLicensedPrescriberInactiveReady = TRUE;
		EnableLicensedPrescriberButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlAvailableMl(short nFlags)
{
	try{
		m_bMidlevelPrescriberAllReady = TRUE;
		EnableMidlevelPrescriberButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlLicensedMl(short nFlags)
{
	try{
		m_bMidlevelPrescriberUsedReady = TRUE;
		EnableMidlevelPrescriberButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlInactiveMl(short nFlags)
{
	try{
		m_bMidlevelPrescriberInactiveReady = TRUE;
		EnableMidlevelPrescriberButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlAvailableNs(short nFlags)
{
	try{
		m_bNurseStaffAllReady = TRUE;
		EnableNurseStaffButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlLicensedNs(short nFlags)
{
	try{
		m_bNurseStaffUsedReady = TRUE;
		EnableNurseStaffButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::RequeryFinishedNxdlInactiveNs(short nFlags)
{
	try{
		m_bNurseStaffInactiveReady = TRUE;
		EnableNurseStaffButtons();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::OnBnClickedLpRequest()
{
	try{

		CWaitCursor cws;		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlLicensedPrescriberAvailable->GetCurSel();

		if (pRow) {
			if (IDYES == MessageBox("Once a provider acquires a Licensed Prescriber license, the provider will hold that license until they are deactivated.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.\r\n\r\nAre you sure you are ready to allocate this Licensed Prescriber license?", "Practice", MB_YESNO | MB_ICONINFORMATION)) {
				long nProvID = VarLong(pRow->GetValue(nelcID), -1);

				if (nProvID > 0) {
					BOOL bResult = m_lNexERxLicense.RequestLicensedPrescriber(nProvID);

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();

						AuditEvent(-1, GetExistingContactName(nProvID), nAuditID, aeiLicensedPrescriberRequest, nProvID, "", "Licensed", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_nxdlLicensedPrescriberLicensed->GetNewRow();

						pNewRow->PutValue(nelcID, pRow->GetValue(nelcID));
						pNewRow->PutValue(nelcName, pRow->GetValue(nelcName));

						m_nxdlLicensedPrescriberLicensed->AddRowAtEnd(pNewRow, NULL);
						m_nxdlLicensedPrescriberAvailable->RemoveRow(pRow);
						EnableLicensedPrescriberButtons();
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::OnBnClickedLpDeactivate()
{
	try{

		CWaitCursor cws;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlLicensedPrescriberLicensed->GetCurSel();

		if (pRow) {
			if (IDYES == MessageBox("Once a provider's Licensed Prescriber license has been deactivated, it can not be reactivated! Please use this feature only if the provider will no longer be sending electronic prescriptions.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.\r\n\r\nAre you sure you are ready to deactivate this provider's Licensed Prescriber license?", "Practice", MB_YESNO | MB_ICONEXCLAMATION)) {
				long nProvID = VarLong(pRow->GetValue(nelcID), -1);

				if (nProvID > 0) {
					BOOL bResult = m_lNexERxLicense.DeactivateLicensedPrescriber(nProvID);

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();

						AuditEvent(-1, GetExistingContactName(nProvID), nAuditID, aeiLicensedPrescriberDeactivate, nProvID, "Licensed", "Deactivated", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_nxdlLicensedPrescriberInactive->GetNewRow();

						pNewRow->PutValue(nelcID, pRow->GetValue(nelcID));
						pNewRow->PutValue(nelcName, pRow->GetValue(nelcName));

						m_nxdlLicensedPrescriberInactive->AddRowAtEnd(pNewRow, NULL);
						m_nxdlLicensedPrescriberLicensed->RemoveRow(pRow);
						EnableLicensedPrescriberButtons();
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::OnBnClickedMlRequest()
{
	try{

		CWaitCursor cws;		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlMidlevelPrescriberAvailable->GetCurSel();

		if (pRow) {

			if( !DetermineSupervisorsForUser(VarLong(pRow->GetValue(nelcID))) ){
				return; // Get out, they first need to license the dependents.
			}

			if (IDYES == MessageBox("Once a provider acquires a Midlevel Prescriber license, the provider will hold that license until they are deactivated.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.\r\n\r\nAre you sure you are ready to allocate this Midlevel Prescriber license?", "Practice", MB_YESNO | MB_ICONINFORMATION)) {
				long nProvID = VarLong(pRow->GetValue(nelcID), -1);

				if (nProvID > 0) {
					BOOL bResult = m_lNexERxLicense.RequestMidelevelPrescriber(nProvID);

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();

						AuditEvent(-1, GetExistingContactName(nProvID), nAuditID, aeiMidlevelPrescriberRequest, nProvID, "", "Licensed", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_nxdlMidlevelPrescriberLicensed->GetNewRow();

						pNewRow->PutValue(nelcID, pRow->GetValue(nelcID));
						pNewRow->PutValue(nelcName, pRow->GetValue(nelcName));

						m_nxdlMidlevelPrescriberLicensed->AddRowAtEnd(pNewRow, NULL);
						m_nxdlMidlevelPrescriberAvailable->RemoveRow(pRow);
						EnableMidlevelPrescriberButtons();
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::OnBnClickedMlDeactivate()
{
	try{

		CWaitCursor cws;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlMidlevelPrescriberLicensed->GetCurSel();

		if (pRow) {
			if (IDYES == MessageBox("Once a provider's Midlevel Prescriber license has been deactivated, it can not be reactivated! Please use this feature only if the provider will no longer be sending electronic prescriptions.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.\r\n\r\nAre you sure you are ready to deactivate this provider's Midlevel Prescriber license?", "Practice", MB_YESNO  | MB_ICONEXCLAMATION)) {
				long nProvID = VarLong(pRow->GetValue(nelcID), -1);

				if (nProvID > 0) {
					BOOL bResult = m_lNexERxLicense.DeactivateMidlevelPrescriber(nProvID);

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();

						AuditEvent(-1, GetExistingContactName(nProvID), nAuditID, aeiMidlevelPrescriberDeactivate, nProvID, "Licensed", "Deactivated", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_nxdlMidlevelPrescriberInactive->GetNewRow();

						pNewRow->PutValue(nelcID, pRow->GetValue(nelcID));
						pNewRow->PutValue(nelcName, pRow->GetValue(nelcName));

						m_nxdlMidlevelPrescriberInactive->AddRowAtEnd(pNewRow, NULL);
						m_nxdlMidlevelPrescriberLicensed->RemoveRow(pRow);
						EnableMidlevelPrescriberButtons();
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::OnBnClickedNsRequest()
{
	try{

		CWaitCursor cws;		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlNurseStaffPrescriberAvailable->GetCurSel();

		if (pRow) {

			if( !DetermineSupervisorsAndMidlevelsLicensedForUser(VarLong(pRow->GetValue(nelcID))) ){
				return; // Get out, they first need to license the dependents.
			}
			
			if (IDYES == MessageBox("Once a user acquires a Nurse/Staff license, the user will hold that license until they are deactivated.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.\r\n\r\nAre you sure you are ready to allocate this Nurse/Staff license?", "Practice", MB_YESNO | MB_ICONINFORMATION)) {
				long nUserID = VarLong(pRow->GetValue(nelcID), -1);

				if (nUserID > 0) {
					BOOL bResult = m_lNexERxLicense.RequestNurseStaff(nUserID);

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();

						AuditEvent(-1, GetExistingUserName(nUserID), nAuditID, aeiNurseStaffRequest, nUserID, "", "Licensed", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_nxdlNurseStaffPrescriberLicensed->GetNewRow();

						pNewRow->PutValue(nelcID, pRow->GetValue(nelcID));
						pNewRow->PutValue(nelcName, pRow->GetValue(nelcName));

						m_nxdlNurseStaffPrescriberLicensed->AddRowAtEnd(pNewRow, NULL);
						m_nxdlNurseStaffPrescriberAvailable->RemoveRow(pRow);
						EnableNurseStaffButtons();
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxConfigureLicenses::OnBnClickedNsDeactivate()
{
	try{

		CWaitCursor cws;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlNurseStaffPrescriberLicensed->GetCurSel();

		if (pRow) {
			if (IDYES == MessageBox("Once a user's Nurse/Staff license has been deactivated, it can not be reactivated! Please use this feature only if the user will no longer be sending electronic prescriptions.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.\r\n\r\nAre you sure you are ready to deactivate this user's Nurse/Staff license?", "Practice", MB_YESNO | MB_ICONEXCLAMATION)) {
				long nUserID = VarLong(pRow->GetValue(nelcID), -1);

				if (nUserID > 0) {
					BOOL bResult = m_lNexERxLicense.DeactivateNurseStaff(nUserID);

					if (bResult) {
						long nAuditID = BeginNewAuditEvent();

						AuditEvent(-1, GetExistingUserName(nUserID), nAuditID, aeiNurseStaffDeactivate, nUserID, "Licensed", "Deactivated", aepHigh, aetChanged);

						// move the item into the used list
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_nxdlNurseStaffPrescriberInactive->GetNewRow();

						pNewRow->PutValue(nelcID, pRow->GetValue(nelcID));
						pNewRow->PutValue(nelcName, pRow->GetValue(nelcName));

						m_nxdlNurseStaffPrescriberInactive->AddRowAtEnd(pNewRow, NULL);
						m_nxdlNurseStaffPrescriberLicensed->RemoveRow(pRow);
						EnableNurseStaffButtons();
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CNexERxConfigureLicenses::DetermineSupervisorsAndMidlevelsLicensedForUser(long nPersonID)
{
	//Get the user info
	UserCommitInfo uciUserInfo = m_pSetup->GetUserRoleInfo(nPersonID);

	//Remember, this is a nurse staff who have both supervisors and midlevels
	BOOL bMidlevelError = CheckMidlevelForError(uciUserInfo.aryMidlevel, nPersonID);
	BOOL bSupervisorError = CheckSupervisorForError(uciUserInfo.arySupervising, nPersonID);

	// Alert if any issues
	if( bMidlevelError || bSupervisorError ){
		MsgBox("You must license the prescribers that this user will prescribe for before you may license the user.", "NexERx License", MB_ICONINFORMATION);
		return FALSE;
	}

	return TRUE;
}

BOOL CNexERxConfigureLicenses::DetermineSupervisorsForUser(long nPersonID)
{
	//Get the user info
	UserCommitInfo uciUserInfo = m_pSetup->GetUserRoleInfo(nPersonID);

	//Remember, this is a nurse staff who have both supervisors and midlevels
	BOOL bSupervisorError = CheckSupervisorForError(uciUserInfo.arySupervising, nPersonID);

	// Alert if any issues
	if( bSupervisorError ){
		MsgBox("You must license the prescribers that this user will prescribe for before you may license the user.", "NexERx License", MB_ICONINFORMATION);
		return FALSE;
	}

	return TRUE;
}

BOOL CNexERxConfigureLicenses::CheckMidlevelForError(const CSimpleArray<long> &aryMidlevel, long nPersonID)
{
	//First, check midlevels
	for( int idx = 0; idx < aryMidlevel.GetSize(); idx++ ){
		if( !m_lNexERxLicense.IsUserLicensedNexERxMidlevelPrescriber(aryMidlevel[idx]) ){
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNexERxConfigureLicenses::CheckSupervisorForError(const CSimpleArray<long> &arySupervising, long nPersonID)
{
	//Check supervisors (licensed prescribers)
	for( int idx = 0; idx < arySupervising.GetSize(); idx++ ){
		if( !m_lNexERxLicense.IsUserLicensedNexERxLicensedPrescriber(arySupervising[idx]) ){
			return TRUE;
		}
	}

	return FALSE;
}