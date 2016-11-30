// ICCPSetupDlg.cpp : implementation file
// Integrated Credit Card Processing Setup dialog
// (d.lange 2015-07-22 16:35) - PLID 67188 - Initial implementation

#include "stdafx.h"
#include "Practice.h"
#include "ICCPSetupDlg.h"
#include "GlobalFinancialUtils.h"
#include "afxdialogex.h"
#include "NxAPI.h"
#include "NxPracticeSharedLib\ICCPUtils.h"
#include "ICCPAdvSettingsDlg.h"
#include "NxPracticeSharedLib\ICCPDeviceManager.h"
#include "NxSystemUtilitiesLib\RemoteDesktopServices.h"


// ICCPSetupDlg dialog
enum eMerchAcctColumns
{
	macID,
	macMerchID,
	macMerchDescription,
	macInactive,
};

IMPLEMENT_DYNAMIC(CICCPSetupDlg, CNxDialog)

CICCPSetupDlg::CICCPSetupDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CICCPSetupDlg::IDD, pParent)
{
	m_bIsLoadingMerchant = FALSE;
}

CICCPSetupDlg::~CICCPSetupDlg()
{
}

// (c.haag 2015-07-30) - PLID 67187 - Adds a merchant to data
LPUNKNOWN CICCPSetupDlg::AddMerchant(const CString& strMerchantName)
{
	NexTech_Accessor::_ICCPMerchantCommitPtr pCommit(__uuidof(NexTech_Accessor::ICCPMerchantCommit));
	pCommit->description = _bstr_t(strMerchantName);
	pCommit->MerchantID = _bstr_t("");
	Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
	NexTech_Accessor::_ICCPMerchantsPtr pNewMerchants = GetAPI()->SetICCPMerchants(GetAPISubkey(), GetAPILoginToken(), saCommits);
	Nx::SafeArray<IUnknown *> saResults = pNewMerchants->Values;
	return saResults[0];
}

// (c.haag 2015-07-30) - PLID 67187 - Saves an existing merchant
void CICCPSetupDlg::SaveMerchant(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	NexTech_Accessor::_ICCPMerchantCommitPtr pCommit(__uuidof(NexTech_Accessor::ICCPMerchantCommit));
	pCommit->ID = _bstr_t(pRow->Value[macID]);
	pCommit->description = _bstr_t(pRow->Value[macMerchDescription]);
	pCommit->MerchantID = _bstr_t(pRow->Value[macMerchID]);
	pCommit->Inactive = pRow->Value[macInactive];
	Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
	GetAPI()->SetICCPMerchants(GetAPISubkey(), GetAPILoginToken(), saCommits);
}

// Toggles the enabled status of every control that can be used to modify or delete the selected merchant
void CICCPSetupDlg::EnableSelectedMerchantControls(BOOL bEnable)
{
	bEnable = (GetCurrentUserPermissions(bioCCProcessingSetup) & sptView) ? bEnable : FALSE;
	m_btnRenameMerchant.EnableWindow(bEnable);
	m_btnDeleteMerchant.EnableWindow(bEnable);
	m_checkActive.EnableWindow(bEnable);
	GetDlgItem(IDC_MERCH_ID)->EnableWindow(bEnable);
}

// Loads the selected mechant to the form
void CICCPSetupDlg::LoadSelectedMerchAccount()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->GetCurSel();
	m_bIsLoadingMerchant = TRUE;
	if (nullptr == pRow)
	{
		// If we get here, no selection was made or there are no merchants.
		EnableSelectedMerchantControls(FALSE);
		SetDlgItemText(IDC_MERCH_ID, "");
		m_checkActive.SetCheck(FALSE);
	}
	else
	{
		// A valid selection was made
		EnableSelectedMerchantControls(TRUE);
		SetDlgItemText(IDC_MERCH_ID, VarString(pRow->GetValue(macMerchID)));
		m_checkActive.SetCheck(VarBool(pRow->GetValue(macInactive)) ? 0 : 1);
	}
	m_bIsLoadingMerchant = FALSE;
}

// Moves a window from its present position by delta values
void CICCPSetupDlg::PushWindow(CWnd* pWnd, const CPoint& ptDelta)
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	if (nullptr != pWnd)
	{
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);
		pWnd->SetWindowPos(NULL, rc.left + ptDelta.x, rc.top + ptDelta.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
}

// Hides the Integrated Credit Card Processing window
void CICCPSetupDlg::HideEnableICCPWindow()
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	CRect rcEnableICCP, rcTestEMV, rcThis;
	m_btnEnableICCP.GetWindowRect(&rcEnableICCP);
	ScreenToClient(&rcEnableICCP);
	m_btnTestEMV.GetWindowRect(&rcTestEMV);
	ScreenToClient(&rcTestEMV);

	// Figure out how many pels we're moving the controls below the Enable ICCP button up
	int verticalScoot = rcTestEMV.top - rcEnableICCP.top;

	// Hide the Enable ICCP window and the color control under it
	m_btnEnableICCP.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_ICCP_SETUP_BKG3)->ShowWindow(SW_HIDE);
	// Move the test EMV button
	m_btnTestEMV.SetWindowPos(NULL, rcEnableICCP.left, rcEnableICCP.top, rcEnableICCP.Width(), rcEnableICCP.Height(), SWP_NOZORDER);
	// Move the close button up
	PushWindow(&m_btnClose, CPoint(0, -verticalScoot));
	// Reduce the full window height
	GetWindowRect(&rcThis);
	SetWindowPos(NULL, 0, 0, rcThis.Width(), rcThis.Height() - verticalScoot, SWP_NOZORDER | SWP_NOMOVE);
	// Ensure the window is still centered
	CenterWindow();
}

void CICCPSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADDMERCHANT, m_btnAddMerchant);
	DDX_Control(pDX, IDC_BTN_RENAMEMERCHANT, m_btnRenameMerchant);
	DDX_Control(pDX, IDC_BTN_DELETEMERCHANT, m_btnDeleteMerchant);
	DDX_Control(pDX, IDC_MERCH_ID, m_nxeditMerchantID);
	DDX_Control(pDX, IDC_CHECK_ACTIVE, m_checkActive);
	DDX_Control(pDX, IDC_BTN_TEST_EMV, m_btnTestEMV);
	DDX_Control(pDX, IDC_BTN_ENABLE_ICCP, m_btnEnableICCP);
}

BEGIN_MESSAGE_MAP(CICCPSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADDMERCHANT, &CICCPSetupDlg::OnBnClickedBtnAddmerchant)
	ON_BN_CLICKED(IDOK, &CICCPSetupDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_BTN_RENAMEMERCHANT, &CICCPSetupDlg::OnBnClickedBtnRenamemerchant)
	ON_BN_CLICKED(IDC_BTN_DELETEMERCHANT, &CICCPSetupDlg::OnBnClickedBtnDeletemerchant)
	ON_BN_CLICKED(IDC_CHECK_ACTIVE, &CICCPSetupDlg::OnBnClickedCheckActive)
	ON_BN_CLICKED(IDC_BTN_ENABLE_ICCP, &CICCPSetupDlg::OnBnClickedBtnEnableICCP)
	ON_BN_CLICKED(IDC_BTN_TEST_EMV, &CICCPSetupDlg::OnBnClickedBtnTestEmv)
	ON_BN_CLICKED(IDC_BTN_INSTALL_INGENICO_DRIVER, &CICCPSetupDlg::OnBnClickedBtnInstallIngenicoDriver)
	ON_BN_CLICKED(IDCANCEL, &CICCPSetupDlg::OnBnClickedCancel)
	ON_EN_KILLFOCUS(IDC_MERCH_ID, &CICCPSetupDlg::OnEnKillfocusMerchId)
	ON_BN_CLICKED(IDC_BTN_ICCP_ADV_SETTINGS, &CICCPSetupDlg::OnBnClickedBtnIccpAdvSettings)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CICCPSetupDlg, CNxDialog)
	ON_EVENT(CICCPSetupDlg, IDC_MERCH_ACCT_LIST, 1, CICCPSetupDlg::SelChangingMerchAccountList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CICCPSetupDlg, IDC_MERCH_ACCT_LIST, 16, CICCPSetupDlg::SelChosenMerchAccountList, VTS_DISPATCH)
END_EVENTSINK_MAP()


// ICCPSetupDlg message handlers
BOOL CICCPSetupDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddMerchant.AutoSet(NXB_NEW);
		m_btnRenameMerchant.AutoSet(NXB_MODIFY);
		m_btnDeleteMerchant.AutoSet(NXB_DELETE);
		m_pMerchAccountList = BindNxDataList2Ctrl(IDC_MERCH_ACCT_LIST, false);

		// (c.haag 2015-07-30) - PLID 67187 - If Integrated Credit Card Processing is enabled then hide the Enable ICCP button and move
		// the test button over it.
		if (IsICCPEnabled())
		{
			HideEnableICCPWindow();
		}

		BOOL bHasSetupPermission = GetCurrentUserPermissions(bioCCProcessingSetup) & sptView;
		// (c.haag 2015-07-30) - PLID 67187 - Disable the Add button if the user doesn't have permissions
		m_btnAddMerchant.EnableWindow(bHasSetupPermission);
		// (c.haag 2015-07-30) - PLID 67190 - Disable the Enable ICCP button if the user doesn't have permissions
		m_btnEnableICCP.EnableWindow(bHasSetupPermission);
		// (z.manning 2015-10-07 15:38) - PLID 67255 - Disable adv settings button too
		GetDlgItem(IDC_BTN_ICCP_ADV_SETTINGS)->EnableWindow(bHasSetupPermission);

		// Populate the merchant list in memory
		NexTech_Accessor::_ICCPMerchantsPtr pICCPMerchants = GetAPI()->GetICCPMerchants(GetAPISubkey(), GetAPILoginToken(), nullptr);
		Nx::SafeArray<IUnknown *> saResults = pICCPMerchants->Values;
		for each(NexTech_Accessor::_ICCPMerchantPtr pMerchant in saResults)
		{
			// Add the merchant to the datalist
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->GetNewRow();

			pRow->Value[macID] = pMerchant->ID;
			pRow->Value[macMerchDescription] = pMerchant->description;
			pRow->Value[macMerchID] = pMerchant->MerchantID;
			pRow->Value[macInactive] = (VARIANT_FALSE == pMerchant->Inactive) ? g_cvarFalse : g_cvarTrue;

			m_pMerchAccountList->AddRowSorted(pRow, NULL);
		}

		// Load the first merchant in the list
		m_pMerchAccountList->CurSel = m_pMerchAccountList->GetFirstRow();
		LoadSelectedMerchAccount();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CICCPSetupDlg::SelChangingMerchAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		// Don't let them select nothing
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

void CICCPSetupDlg::SelChosenMerchAccountList(LPDISPATCH lpRow)
{
	try {
		// Load the selected merchant account info
		LoadSelectedMerchAccount();
	} NxCatchAll(__FUNCTION__);
}

void CICCPSetupDlg::OnBnClickedBtnAddmerchant()
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	try
	{
		// We disable the button if the user doesn't have permissions; but check again anyway to be sure
		if (CheckCurrentUserPermissions(bioCCProcessingSetup, sptView))
		{
			CString strMerchantName;
			if (IDOK == InputBoxNonEmpty(this, "Please enter a name for the new merchant", strMerchantName, 100))
			{
				// Add the merchant to data
				NexTech_Accessor::_ICCPMerchantPtr pNewMerchant = AddMerchant(strMerchantName);

				// Add the merchant to the datalist and select it
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->GetNewRow();
				pRow->Value[macID] = pNewMerchant->ID;
				pRow->Value[macMerchDescription] = pNewMerchant->description;
				pRow->Value[macMerchID] = pNewMerchant->MerchantID;
				pRow->Value[macInactive] = (VARIANT_FALSE == pNewMerchant->Inactive) ? g_cvarFalse : g_cvarTrue;
				m_pMerchAccountList->CurSel = m_pMerchAccountList->AddRowSorted(pRow, NULL);

				// Now update the form fields
				LoadSelectedMerchAccount();
			}
			else
			{
				// User changed their mind
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CICCPSetupDlg::OnBnClickedBtnRenamemerchant()
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	try
	{
		// We disable the button if the user doesn't have permissions; but check again anyway to be sure
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->CurSel;
		if (nullptr != pRow && CheckCurrentUserPermissions(bioCCProcessingSetup, sptView))
		{
			CString strMerchantName = VarString(pRow->Value[macMerchDescription]);
			if (IDOK == InputBoxNonEmpty(this, "Please enter a new name for the merchant", strMerchantName, 100))
			{
				// Update the datalist first
				pRow->Value[macMerchDescription] = _bstr_t(strMerchantName);

				// Update the data second
				SaveMerchant(pRow);
			}
			else
			{
				// User changed their mind
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CICCPSetupDlg::OnBnClickedBtnDeletemerchant()
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	try
	{
		try
		{
			// We disable the button if the user doesn't have permissions; but check again anyway to be sure
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->CurSel;
			if (nullptr != pRow && CheckCurrentUserPermissions(bioCCProcessingSetup, sptView))
			{
				CString strMerchantAccountID = VarString(pRow->Value[macID]);
				CString strMerchantName = VarString(pRow->Value[macMerchDescription]);
				NxTaskDialog dlgDelete;
				dlgDelete.Config()
					.WarningIcon()
					.ZeroButtons()
					.MainInstructionText(FormatString("Are you sure you wish to delete the %s merchant account?", strMerchantName))
					.ContentText("Deleting this merchant account will remove it from your list of available merchant accounts to process credit card payments to.")
					.AddCommand(1000, "Delete Merchant Account")
					.AddCommand(1001, "Cancel")
					.DefaultButton(1001);

				if (1000 == dlgDelete.DoModal())
				{
					// Delete the merchant
					GetAPI()->DeleteICCPMerchant(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strMerchantAccountID));

					// Remove the row from the list and select the next item
					NXDATALIST2Lib::IRowSettingsPtr pDoomedRow = m_pMerchAccountList->CurSel;
					m_pMerchAccountList->CurSel = m_pMerchAccountList->CurSel->GetNextRow();
					if (nullptr == m_pMerchAccountList->CurSel)
					{
						// We deleted the last merchant in the list, so set the current list selection to the new "last" merchant
						m_pMerchAccountList->CurSel = m_pMerchAccountList->GetLastRow();
					}
					m_pMerchAccountList->RemoveRow(pDoomedRow);

					// Now update the form fields
					LoadSelectedMerchAccount();
				}
			}
		}
		catch (_com_error &e)
		{
			CString strError((LPCTSTR)e.Description());
			if (-1 == strError.Find("Attempted to delete a merchant with processed transactions"))
			{
				throw;
			}
			else
			{
				MessageBox(R"(You may not delete a merchant with processed transactions.

If you wish for it to no longer appear in the merchant list of the payment window, please inactivate it.)"
					, NULL, MB_OK | MB_ICONERROR);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CICCPSetupDlg::OnEnKillfocusMerchId()
{
	try
	{
		// We disable the button if the user doesn't have permissions; but check again anyway to be sure
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->CurSel;
		if (!m_bIsLoadingMerchant && nullptr != pRow && CheckCurrentUserPermissions(bioCCProcessingSetup, sptView))
		{
			CString strOldMerchID = VarString(pRow->Value[macMerchID]);
			CString strNewMerchID;
			GetDlgItemText(IDC_MERCH_ID, strNewMerchID);
		
			if (strOldMerchID != strNewMerchID)
			{
				// Update the datalist first
				pRow->Value[macMerchID] = _bstr_t(strNewMerchID);

				// Update the data second
				SaveMerchant(m_pMerchAccountList->CurSel);
			}
		}
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPSetupDlg::OnBnClickedCheckActive()
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	try
	{
		// We disable the button if the user doesn't have permissions; but check again anyway to be sure
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->CurSel;
		if (!m_bIsLoadingMerchant && nullptr != pRow && CheckCurrentUserPermissions(bioCCProcessingSetup, sptView))
		{
			// Update the active status first
			pRow->Value[macInactive] = (m_checkActive.GetCheck() == 0) ? g_cvarTrue : g_cvarFalse;

			// Update the data second
			SaveMerchant(m_pMerchAccountList->CurSel);
		}
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPSetupDlg::OnBnClickedBtnTestEmv()
{
	// (c.haag 2015-07-30) - PLID 67191 - Initial implementation
	try
	{
		// (z.manning 2015-09-04 08:51) - PLID 67236 - Moved logic to this function
		TestEmv();
	}
	NxCatchAll(__FUNCTION__)
}

BOOL CICCPSetupDlg::CheckForDeviceConnection()
{
	return CheckForDeviceConnection("");
}

BOOL CICCPSetupDlg::CheckForDeviceConnection(const CString &strNoDeviceMessageOverride)
{
	NexTech_COM::IICCPSessionManager* pDevice;

	// This can take a few seconds to have a wait cursor
	{
		CWaitCursor wc;
		pDevice = GetICCPDevice();
	}

	if (nullptr == pDevice)
	{
		// This should never happen. It's not a matter of the device not being connected; this could
		// mean there's an issue with the Nextech installation
		ThrowNxException("GetICCPDevice() returned NULL!");
	}
	else if (!pDevice->IsDeviceConnected())
	{
		BOOL bConnected = FALSE;

		// We couldn't find the device. Before we give up, let's check and see if we're using
		// RDS as in that case, a COM port setting is required that may not be set up. Let's
		// see if the device can auto-detect its port and connect.
		if (RdsApi::Instance().IsRemoteConnected())
		{
			try
			{
				CString strComPort = (LPCTSTR)pDevice->TryToFindComPortAndConnect();
				if (!strComPort.IsEmpty())
				{
					// We found a port so update the setting so we don't need to do this again.
					SetRemotePropertyText("IngenicoComPortPerMachine", strComPort, 0, g_propManager.GetSystemName());
					bConnected = TRUE;
				}
			}
			// This was an extra effort on the software's part to connect, but we don't want
			// this to result in an exception displayed to the user.
			NxCatchAllIgnore();
		}

		if (bConnected)
		{
			return TRUE;
		}
		else
		{
			CString strMessage = strNoDeviceMessageOverride;
			if (strMessage.IsEmpty()) {
				strMessage = "A supported card reader / PIN pad is not detected on this workstation. Please check your configuration and try again.\r\n\r\n"
					"If a supported card reader / PIN pad is already attached and the device drivers have been installed on this workstation, please call Nextech Product Support for assistance.";
			}
			// Handle detection failure
			MessageBox(strMessage, NULL, MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	else
	{
		return TRUE;
	}
}

// (z.manning 2015-09-04 08:50) - PLID 67236
BOOL CICCPSetupDlg::TestEmv()
{
	CWaitCursor wc;

	// (c.haag 2015-07-30) - PLID 67191 - Ensure the device manager is valid so that users may test
	// the swiper without having ICCP enabled
	GetMainFrame()->EnsureICCPDeviceManager(TRUE);

	if (!CheckForDeviceConnection()) {
		return FALSE;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMerchAccountList->CurSel;
	if (nullptr == pRow)
	{
		MessageBox(R"(A supported card reader / PIN pad was detected.

To perform a test authorization for a merchant, please select it from the merchant account dropdown.)"
, NULL, MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	// Let the user know they will be opted to do a $0 swipe and give them the chance to back out.
	NxTaskDialog dlgAuth;
	dlgAuth.Config()
		.WarningIcon()
		.ZeroButtons()
		.MainInstructionText(FormatString("A test authorization will now be performed for the %s merchant.", VarString(pRow->Value[macMerchDescription])))
		.ContentText("Please follow the prompts on the credit card device. The card used for testing this device will NOT be charged.")
		.AddCommand(1000, "Proceed")
		.AddCommand(1001, "Cancel")
		.DefaultButton(1000);
	if (1001 == dlgAuth.DoModal())
	{
		// User opted to cancel the authorization
		return FALSE;
	}

	BOOL bTestResult = FALSE;
	try
	{
		bTestResult = DoCreditCardTestSwipe(this, atoi(VarString(pRow->Value[macID])));
	}
	catch (_com_error e)
	{
		// (z.manning 2015-10-06 08:45) - PLID 67236 - Since the test button is likely to be pressed at initial set up,
		// it's more likely for something to go wrong. Instead of displaying a giant exception box, let's attempt to
		// caputure the description of the error and just display that in a message box.
		CString strException = (LPCTSTR)e.Description();
		CString strDescription;
		int nColon = strException.Find(':');
		if (nColon >= 0) {
			strException = strException.Mid(nColon + 1);
			strException.TrimLeft();
			int nEol = strException.Find('\n');
			if (nEol >= 0) {
				strDescription = strException.Left(nEol);
				strDescription.Trim();
			}
		}
		if (strDescription.IsEmpty()) {
			throw;
		}
		else {
			CString strMessage = FormatString("The test has failed. Make sure you have a device attached to this computer "
				"and that you have entered the correct merchant ID.\r\n\r\nError details...\r\n%s"
				, strDescription);
			MessageBox(strMessage, "Test Failed", MB_ICONERROR);
		}
	}

	// Perform a $0 test charge
	if (!bTestResult)
	{
		// The user should have gotten error messages from inside the function
		return FALSE;
	}
	else
	{
		MessageBox(R"(The device attached to this workstation has been tested successfully!)"
			, NULL, MB_OK | MB_ICONINFORMATION);
		return TRUE;
	}
}

void CICCPSetupDlg::OnBnClickedBtnEnableICCP()
{
	// (c.haag 2015-07-30) - PLID 67190 - Initial implementation
	try
	{
		// We disable the add button if the user doesn't have permissions; but check again anyway to be sure
		if (CheckCurrentUserPermissions(bioCCProcessingSetup, sptView))
		{
			NxTaskDialog dlgEnable;
			dlgEnable.Config()
				.WarningIcon()
				.ZeroButtons()
				.MainInstructionText("Are you SURE you wish to enable Integrated Credit Card Processing?")
				.ContentText("To enable Integrated Credit Card Processing, all existing card holder data must be purged from the database for security purposes. "
				"Once Integrated Credit Card Processing is enabled, credit card transactions must be processed using a secure EMV-ready device.\r\n\r\n"
				"An EMV-ready device must be connected to this workstation in order to enable Integrated Credit Card Processing."
				)
				.AddCommand(1000, "Purge all existing card holder data and enable Integrated Credit Card Processing.")
				.AddCommand(1001, "Cancel")
				.DefaultButton(1001);

			if (1000 == dlgEnable.DoModal())
			{
				CWaitCursor wc;

				// (z.manning 2015-08-27 10:46) - PLID 67221 - Attempt to connect to a device. Ignore the preference
				// because we know it's not enabled yet.
				GetMainFrame()->EnsureICCPDeviceManager(TRUE);

				// (z.manning 2015-09-04 08:55) - PLID 67236 - We now do a test before allowing them to enable it
				// because once it's enable there's no going back. This way we ensure they have a valid device.
				CString strNotConnectedMessage = "Integrated Credit Card Processing can only be enabled from a workstation that has an EMV terminal device attached and installed. "
					"Please attach and install an EMV terminal device on this machine and try again.";
				if (!CheckForDeviceConnection(strNotConnectedMessage)) {
					return;
				}

				// Enable Integrated Credit Card Processing
				GetAPI()->EnableICCP(GetAPISubkey(), GetAPILoginToken());
				// Ensure the property manager cache is updated
				SetRemotePropertyInt("ICCPEnabled", 1, 0, "<None>");
				// Hide the ICCP window
				HideEnableICCPWindow();
			}
		}
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPSetupDlg::OnBnClickedClose()
{
	// (c.haag 2015-07-30) - PLID 67187 - Initial implementation
	try
	{
		__super::OnOK();
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPSetupDlg::OnBnClickedCancel()
{
	// (c.haag 2015-07-30) - PLID 67187 - Do nothing
}

// (z.manning 2015-08-11 12:38) - PLID 67235
void CICCPSetupDlg::OnBnClickedBtnInstallIngenicoDriver()
{
	try
	{
		// (z.manning 2015-08-12 09:42) - PLID 67235 - All we're doing here (for now at least) is running
		// the driver install and that's it.
		ICCP::RunIngenicoDriverInstall(GetSafeHwnd());
	}
	NxCatchAll(__FUNCTION__)
}

// (z.manning 2015-09-30 11:10) - PLID 67255
void CICCPSetupDlg::OnBnClickedBtnIccpAdvSettings()
{
	try
	{
		if (!CheckCurrentUserPermissions(bioCCProcessingSetup, sptView)) {
			return;
		}

		CICCPAdvSettingsDlg dlg(this);
		dlg.DoModal();
		if (dlg.m_bChanged)
		{
			if (dlg.m_bRestartNeeded) {
				MessageBox("Please restart Practice for your changes to take effect.", NULL, MB_ICONWARNING);
			}
			else {
				// (z.manning 2015-09-30 15:48) - PLID 67255 - Need to refresh the device if settings changed
				GetMainFrame()->RefreshICCPDeviceManager();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}
