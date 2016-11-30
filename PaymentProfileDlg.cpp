// PaymentProfileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillingRc.h"
#include "PaymentProfileDlg.h"
#include "NxPracticeSharedLib\ICCPDeviceManager.h"
#include "NxPracticeSharedLib\ICCPUtils.h"
#include "NxAPIUtils.h"
#include "GlobalFinancialUtils.h"


// CICCPPaymentProfileDlg dialog
// (z.manning 2015-07-22 10:02) - PLID 67241 - Created

using namespace NXDATALIST2Lib;
using namespace NexTech_Accessor;

IMPLEMENT_DYNAMIC(CICCPPaymentProfileDlg, CNxDialog)

CICCPPaymentProfileDlg::CICCPPaymentProfileDlg(const long nPatientPersonID, CWnd* pParent)
	: CNxDialog(CICCPPaymentProfileDlg::IDD, pParent)
	, m_nPatientPersonID(nPatientPersonID)
{

}

CICCPPaymentProfileDlg::~CICCPPaymentProfileDlg()
{
}

void CICCPPaymentProfileDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ICCP_PAYMENT_PROFILE_BACKGROUND, m_nxclrBackground);
	DDX_Control(pDX, IDC_ADD_ICCP_PAYMENT_PROFILE, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_ICCP_PAYMENT_PROFILE, m_btnDelete);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
}


BEGIN_MESSAGE_MAP(CICCPPaymentProfileDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PAYMENT_PROFILE_HIDE_EXPIRED, &CICCPPaymentProfileDlg::OnBnClickedPaymentProfileHideExpired)
	ON_BN_CLICKED(IDC_ADD_ICCP_PAYMENT_PROFILE, &CICCPPaymentProfileDlg::OnBnClickedAddICCPPaymentProfile)
	ON_BN_CLICKED(IDC_DELETE_ICCP_PAYMENT_PROFILE, &CICCPPaymentProfileDlg::OnBnClickedDeleteICCPPaymentProfile)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CICCPPaymentProfileDlg, CNxDialog)
ON_EVENT(CICCPPaymentProfileDlg, IDC_ICCP_PAYMENT_PROFILE_LIST, 10, CICCPPaymentProfileDlg::EditingFinishedICCPPaymentProfileList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CICCPPaymentProfileDlg, IDC_ICCP_PAYMENT_PROFILE_LIST, 2, CICCPPaymentProfileDlg::SelChangedICCPPaymentProfileList, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CICCPPaymentProfileDlg, IDC_ICCP_PAYMENT_PROFILE_LIST, 8, CICCPPaymentProfileDlg::EditingStartingIccpPaymentProfileList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CICCPPaymentProfileDlg, IDC_ICCP_PAYMENT_PROFILE_LIST, 6, CICCPPaymentProfileDlg::RButtonDownIccpPaymentProfileList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


// CICCPPaymentProfileDlg message handlers

BOOL CICCPPaymentProfileDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CICCPPaymentProfileDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'PaymentProfilesHideExpired' \r\n" // (z.manning 2015-08-24 09:48) - PLID 67254
			"	) \r\n"
			, _Q(GetCurrentUserName())
			);

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_nxclrBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		SetWindowText(FormatString("Payment Profiles for %s (%li)"
			, GetExistingPatientName(m_nPatientPersonID)
			, GetExistingPatientUserDefinedID(m_nPatientPersonID)
			));

		m_pdlPaymentProfiles = BindNxDataList2Ctrl(IDC_ICCP_PAYMENT_PROFILE_LIST, false);

		// (z.manning 2015-08-24 08:37) - PLID 67251 - Set an input mask on the exp date column
		IColumnSettingsPtr pExpDateCol = m_pdlPaymentProfiles->GetColumn(ppcExpDateText);
		pExpDateCol->InputMask = _bstr_t(ICCP::GetInputMaskForExpirationDate());

		// (z.manning 2015-08-24 09:53) - PLID 67254 - Set the hide expirted option
		CheckDlgButton(IDC_PAYMENT_PROFILE_HIDE_EXPIRED, GetRemotePropertyInt("PaymentProfilesHideExpired", BST_CHECKED, 0, GetCurrentUserName()));

		UpdateView();
	}
	NxCatchAll(__FUNCTION__)

	return TRUE;
}

void CICCPPaymentProfileDlg::UpdateView(bool bForceRefresh /* = true */)
{
	UpdateButtons();

	if (bForceRefresh) {
		LoadProfiles();
	}

	UpdateVisibleRows();
}

void CICCPPaymentProfileDlg::LoadProfiles()
{
	CWaitCursor wc;
	m_pdlPaymentProfiles->Clear();

	_ICCPPaymentProfilesPtr pProfiles = GetAPI()->GetICCPPaymentProfiles(GetAPISubkey(), GetAPILoginToken()
		, _bstr_t(m_nPatientPersonID), VARIANT_TRUE);
	if (pProfiles == NULL || pProfiles->Profiles == NULL) {
		return;
	}

	Nx::SafeArray<IUnknown*> saProfiles(pProfiles->Profiles);
	for each (_ICCPPaymentProfilePtr pProfile in saProfiles)
	{
		m_pdlPaymentProfiles->AddRowAtEnd(GetNewRowFromProfile(pProfile), NULL);
	}
}

void CICCPPaymentProfileDlg::EditingFinishedICCPPaymentProfileList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		if (!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		switch (nCol)
		{
		case ppcExpDateText:
			if (VarString(varOldValue, "") == VarString(varNewValue, "")) {
				return;
			}
			break;

		case ppcDefault:
			if (VarBool(varOldValue) == VarBool(varNewValue)) {
				return;
			}
			break;
		}

		const CString strProfileID = VarString(pRow->GetValue(ppcProfileID));

		// (z.manning 2015-08-18 08:48) - PLID 67251 - Handle changed expiration dates
		if (nCol == ppcExpDateText)
		{
			COleDateTime dtNewExpiration;
			if (ICCP::ParseExpirationDate(VarString(varNewValue), dtNewExpiration))
			{
				// (z.manning 2015-08-18 10:18) - PLID 67251 - They entered a valid date so update the hidden
				// date column.
				pRow->PutValue(ppcExpDate, COleVariant(dtNewExpiration));
				// (z.manning 2015-08-18 10:21) - PLID 67251 - Make sure the expiration date is formatted exactly
				// as we want it.
				pRow->PutValue(ppcExpDateText, _bstr_t(ICCP::GetExpirationDateText(dtNewExpiration)));
			}
			else
			{
				pRow->PutValue(nCol, varOldValue);
				return;
			}
		}

		try
		{
			if (!UpdatePaymentProfileFromRow(pRow)) {
				pRow->PutValue(nCol, varOldValue);
				return;
			}
		}
		NxCatchAllSilentCallThrow({
			pRow->PutValue(nCol, varOldValue);
		})

		if (nCol == ppcDefault)
		{
			// (z.manning 2015-07-22 11:44) - PLID 67251 - We only allow one as the default so ensure the
			// rest of the rows do not have it selected.
			if (VarBool(varNewValue))
			{
				IRowSettingsPtr pLoopRow = m_pdlPaymentProfiles->FindAbsoluteFirstRow(VARIANT_FALSE);
				for (; pLoopRow != NULL; pLoopRow = m_pdlPaymentProfiles->FindAbsoluteNextRow(pLoopRow, VARIANT_FALSE))
				{
					if (VarString(pLoopRow->GetValue(ppcProfileID)) != strProfileID) {
						pLoopRow->PutValue(ppcDefault, g_cvarFalse);
					}
				}
			}
		}

		UpdateVisibleRows();
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPPaymentProfileDlg::SelChangedICCPPaymentProfileList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		UpdateButtons();
	}
	NxCatchAll(__FUNCTION__)
}

// (z.manning 2015-07-23 08:47) - PLID 67237
void CICCPPaymentProfileDlg::UpdateButtons()
{
	// (z.manning 2015-07-23 08:55) - PLID 67237 - Disable the buttons if they do not have the necessary permission for each one.
	// Note: There aren't w/ pw perms for these.
	BOOL bHasWritePerm = CheckCurrentUserPermissions(bioPaymentProfile, sptWrite, FALSE, 0, TRUE);
	BOOL bHasDeletePerm = CheckCurrentUserPermissions(bioPaymentProfile, sptDelete, FALSE, 0, TRUE);
	BOOL bHasSelection = (m_pdlPaymentProfiles->CurSel != NULL);

	GetDlgItem(IDC_ADD_ICCP_PAYMENT_PROFILE)->EnableWindow(bHasWritePerm);
	GetDlgItem(IDC_DELETE_ICCP_PAYMENT_PROFILE)->EnableWindow(bHasDeletePerm && bHasSelection);
}

// (z.manning 2015-07-23 09:57) - PLID 67254
void CICCPPaymentProfileDlg::OnBnClickedPaymentProfileHideExpired()
{
	try
	{
		UpdateVisibleRows();

		SetRemotePropertyInt("PaymentProfilesHideExpired", IsDlgButtonChecked(IDC_PAYMENT_PROFILE_HIDE_EXPIRED), 0, GetCurrentUserName());
	}
	NxCatchAll(__FUNCTION__)
}

// (z.manning 2015-07-23 10:00) - PLID 67254
void CICCPPaymentProfileDlg::UpdateVisibleRows()
{
	BOOL bHideExpired = (IsDlgButtonChecked(IDC_PAYMENT_PROFILE_HIDE_EXPIRED) == BST_CHECKED);

	IRowSettingsPtr pRow = m_pdlPaymentProfiles->FindAbsoluteFirstRow(VARIANT_FALSE);
	for (; pRow != NULL; pRow = m_pdlPaymentProfiles->FindAbsoluteNextRow(pRow, VARIANT_FALSE))
	{
		COleDateTime dtRowExpiration = g_cdtInvalid;
		_variant_t varRowExpiration = pRow->GetValue(ppcExpDate);
		if (varRowExpiration.vt == VT_DATE) {
			dtRowExpiration = VarDateTime(varRowExpiration);
		}

		if (!bHideExpired || dtRowExpiration.GetStatus() != COleDateTime::valid || dtRowExpiration > AsDateNoTime(COleDateTime::GetCurrentTime())) {
			pRow->Visible = VARIANT_TRUE;
		}
		else {
			pRow->Visible = VARIANT_FALSE;
		}
	}
}

void CICCPPaymentProfileDlg::OnBnClickedAddICCPPaymentProfile()
{
	try
	{
		CMenu mnu;
		if (mnu.CreatePopupMenu())
		{
			enum EMenuOptions {
				moCardPresent = 1,
				moCardNotPresent,
			};

			mnu.AppendMenu(MF_STRING, moCardPresent, "Card &Present");
			mnu.AppendMenu(MF_STRING, moCardNotPresent, "Card &Not Present");

			CPoint pt;

			CWnd *pwndAddProfile = GetDlgItem(IDC_ADD_ICCP_PAYMENT_PROFILE);
			if (pwndAddProfile != NULL) {
				CRect rc;
				pwndAddProfile->GetWindowRect(&rc);
				pt.x = rc.right;
				pt.y = rc.top;
			}
			else {
				GetMessagePos(pt);
			}

			DWORD dwReturn = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
			switch (dwReturn)
			{
			case moCardPresent:
				AddNewPaymentProfile(TRUE);
				break;

			case moCardNotPresent:
				AddNewPaymentProfile(FALSE);
				break;
			}
		}
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPPaymentProfileDlg::AddNewPaymentProfile(BOOL bCardPresent)
{
	CWaitCursor wc;

	// (z.manning 2015-08-26 16:52) - PLID 67241 - A device is required in order to create a payment profile
	if (GetICCPDevice() == NULL || !GetICCPDevice()->IsDeviceConnected())
	{
		MessageBox("There is not a supported card reader attached to this workstation. A card reader device is required in order to create payment profiles."
			, "No Device", MB_ICONERROR);
		return;
	}

	// (z.manning 2015-08-04 11:34) - PLID 67247 - Need to make sure we are able to create profiles
	_CanCreateICCPPaymentProfilesResultPtr pCanCreateResult = GetAPI()->CanCreateICCPPaymentProfiles(GetAPISubkey(), GetAPILoginToken());
	if (!pCanCreateResult->CanCreate)
	{
		MessageBox(FormatString("Unable to create payment profile.\r\n\r\nReason:\r\n%s", (LPCTSTR)pCanCreateResult->FailureReason)
			, NULL, MB_ICONERROR);
		return;
	}

	BSTR bstrSwipeToken, bstrEncryptedTrack;
	VARIANT_BOOL bWasSigned;
	_bstr_t bstrPrompt = "Please follow the prompts on the credit card device.\r\n\r\nThe patient's credit card will not be charged.";
	// (z.manning 2015-08-31 09:25) - PLID 67230 - Pass false for the get signature param
	// (z.manning 2015-09-14 14:28) - PLID 67221 - Pass in zero for the value
	if (ICCP::GetCardSecureTokenFromDeviceWithPrompt(
		GetICCPDevice()
		, GetSafeHwnd()
		, 0.0
		, bCardPresent ? VARIANT_TRUE : VARIANT_FALSE
		, VARIANT_FALSE
		, bstrPrompt
		, &bstrSwipeToken
		, &bstrEncryptedTrack
		, &bWasSigned
		))
	{
		CString strToken = CString(static_cast<const char*>(_bstr_t(bstrSwipeToken)));
		if (!strToken.IsEmpty())
		{
			// (z.manning 2015-08-14 09:40) - PLID 67248 - Need to prompt for the expiration date, at least for the time being,
			// as the track data of the card is encrypted so we cannot access it.
			COleDateTime dtExpiration;
			if (PromptForCreditCardExpirationDate(this, dtExpiration))
			{
				_CreateICCPPaymentProfileResultPtr pResult = GetAPI()->CreateICCPPaymentProfile(GetAPISubkey(), GetAPILoginToken()
					, _bstr_t(m_nPatientPersonID), _bstr_t(strToken), GetNullableDateTime(dtExpiration));
				if (pResult != NULL)
				{
					if (pResult->NewProfile != NULL) {
						IRowSettingsPtr pNewRow = GetNewRowFromProfile(pResult->NewProfile);
						pNewRow = m_pdlPaymentProfiles->AddRowBefore(pNewRow, m_pdlPaymentProfiles->GetFirstRow());
						m_pdlPaymentProfiles->EnsureRowInView(pNewRow);
						m_pdlPaymentProfiles->CurSel = pNewRow;
						UpdateButtons();
					}
					else {
						MessageBox(FormatString("Failed to create payment profile\r\n\r\nReason:\r\n%s", (LPCTSTR)pResult->FailureReason)
							, NULL, MB_ICONERROR);
					}
				}
			}
			else {
				MessageBox("You must provide an expiration date to save a payment profile. ", NULL, MB_ICONINFORMATION);
			}
		}
	}
}

// (z.manning 2015-08-17 15:16) - PLID 67251
BOOL CICCPPaymentProfileDlg::UpdatePaymentProfileFromRow(IRowSettingsPtr pRow)
{
	if (pRow == NULL) {
		return FALSE;
	}

	CWaitCursor wc;

	_ICCPPaymentProfileCommitPtr pCommit(__uuidof(ICCPPaymentProfileCommit));
	_variant_t varProfileID = pRow->GetValue(ppcProfileID);
	pCommit->ProfileID = _bstr_t(pRow->GetValue(ppcProfileID));
	pCommit->IsDefault = VarBool(pRow->GetValue(ppcDefault)) ? VARIANT_TRUE : VARIANT_FALSE;
	pCommit->ExpirationDate = GetNullableDateTime(VarDateTime(pRow->GetValue(ppcExpDate), g_cdtNull));

	_UpdateICCPPaymentProfileResultPtr pResult = GetAPI()->UpdateICCPPaymentProfile(GetAPISubkey(), GetAPILoginToken()
		, _bstr_t(m_nPatientPersonID), pCommit);
	if (pResult->Success) {
		return TRUE;
	}
	else {
		MessageBox(FormatString("Failed to update payment profile\r\n\r\nReason:\r\n%s", (LPCTSTR)pResult->FailureReason)
			, NULL, MB_ICONERROR);
		return FALSE;
	}
}

IRowSettingsPtr CICCPPaymentProfileDlg::GetNewRowFromProfile(_ICCPPaymentProfilePtr pProfile)
{
	IRowSettingsPtr pNewRow = m_pdlPaymentProfiles->GetNewRow();
	pNewRow->PutValue(ppcProfileID, pProfile->ProfileAccountID);
	pNewRow->PutValue(ppcCardType, pProfile->AccountType);
	pNewRow->PutValue(ppcLastFour, pProfile->Last4Digits);
	pNewRow->PutValue(ppcDefault, pProfile->IsDefault ? g_cvarTrue : g_cvarFalse);

	if (pProfile->ExpirationDate->IsNull()) {
		pNewRow->PutValue(ppcExpDate, g_cvarNull);
		pNewRow->PutValue(ppcExpDateText, g_cvarNull);
	}
	else {
		COleDateTime dtExpiration = pProfile->ExpirationDate->GetValue();
		pNewRow->PutValue(ppcExpDate, COleVariant(dtExpiration));
		pNewRow->PutValue(ppcExpDateText, _bstr_t(ICCP::GetExpirationDateText(dtExpiration)));
	}

	return pNewRow;
}

void CICCPPaymentProfileDlg::OnBnClickedDeleteICCPPaymentProfile()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlPaymentProfiles->CurSel;
		if (pRow == NULL) {
			return;
		}

		CString strCardType = VarString(pRow->GetValue(ppcCardType), "");
		CString strLast4 = VarString(pRow->GetValue(ppcLastFour), "");
		BOOL bIsDefault = VarBool(pRow->GetValue(ppcDefault));

		// (z.manning 2015-08-24 12:08) - PLID 67241 - CardConnect does not let you delete a default profile
		// if there are other non-defaults.
		if (bIsDefault && m_pdlPaymentProfiles->GetRowCount() > 1) {
			MessageBox("You cannot delete the default profile if there are other profiles. Please change the default if you would like to delete this profile."
				, NULL, MB_ICONINFORMATION);
			return;
		}

		enum PromptOptions {
			poDelete = 1000,
			poCancel,
		};
		CString strPrompt = FormatString("You have selected to delete the payment profile for '%s' for the %s ending in %s. This action is unrecoverable!"
			, GetExistingPatientName(m_nPatientPersonID)
			, strCardType
			, strLast4
			);
		NxTaskDialog dlgDelete;
		dlgDelete.Config()
			.WarningIcon()
			.ZeroButtons()
			.MainInstructionText(strPrompt)
			.ContentText("")
			.AddCommand(poDelete, "Delete Payment Profile")
			.AddCommand(poCancel, "Cancel")
			.DefaultButton(poDelete);
		if (dlgDelete.DoModal() != poDelete) {
			return;
		}

		CWaitCursor wc;
		CString strProfileAccountID = VarString(pRow->GetValue(ppcProfileID));
		_DeleteICCPPaymentProfileResultPtr pResult = GetAPI()->DeleteICCPPaymentProfile(
			GetAPISubkey(), GetAPILoginToken(), _bstr_t(strProfileAccountID), _bstr_t(m_nPatientPersonID));
		if (pResult->Success)
		{
			if (bIsDefault && m_pdlPaymentProfiles->GetRowCount() > 1) {
				// (z.manning 2015-08-06 15:27) - PLID 67241 - If we deleted the default account then we
				// need to reload everything as CardConnect may have assigned a new default behind the scenes.
				UpdateView(true);
			}
			else {
				m_pdlPaymentProfiles->RemoveRow(pRow);
				UpdateButtons();
			}
		}
		else
		{
			MessageBox(FormatString("Failed to delete payment profile\r\n\r\nReason:\r\n%s", (LPCTSTR)pResult->FailureReason));
		}
	}
	NxCatchAll(__FUNCTION__)
}

// (z.manning 2015-08-18 08:27) - PLID 67251
void CICCPPaymentProfileDlg::EditingStartingIccpPaymentProfileList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try
	{
		BOOL bHasWritePerm = CheckCurrentUserPermissions(bioPaymentProfile, sptWrite, FALSE, 0, TRUE);
		// (z.manning 2015-10-09 13:13) - PLID 67251 - Don't let them edit anything if no permission
		if (!bHasWritePerm) {
			*pbContinue = FALSE;
			return;
		}

		// (z.manning 2015-08-18 08:31) - PLID 67251 - CardConnet requires a default so do not let them
		// unselect one.
		switch (nCol)
		{
		case ppcDefault:
			if (VarBool(*pvarValue)) {
				*pbContinue = FALSE;
			}
			break;
		}
	}
	NxCatchAll(__FUNCTION__)
}

void CICCPPaymentProfileDlg::RButtonDownIccpPaymentProfileList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		m_pdlPaymentProfiles->CurSel = pRow;
		UpdateButtons();
		
		if (pRow == NULL) {
			return;
		}

		// (j.jones 2015-09-16 14:54) - PLID 67237 - if they don't have delete permission,
		// don't show the right click menu
		BOOL bHasDeletePerm = CheckCurrentUserPermissions(bioPaymentProfile, sptDelete, FALSE, 0, TRUE);
		if (!bHasDeletePerm) {
			return;
		}

		enum MenuOptions {
			moDelete = 1,
		};

		CMenu menu;
		menu.CreatePopupMenu();

		menu.AppendMenu(MF_BYCOMMAND, moDelete, "&Delete");

		CPoint pt;
		GetCursorPos(&pt);
		switch (menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, this))
		{
		case moDelete:
			OnBnClickedDeleteICCPPaymentProfile();
			break;
		}
	}
	NxCatchAll(__FUNCTION__)
}
