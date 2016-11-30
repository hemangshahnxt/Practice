#include "stdafx.h"
#include "AnchorSettingsDlg.h"
#include "PracticeRc.h"
#include "NxAPI.h"
#include "NxAPIUtils.h"

// (j.armen 2014-07-09 17:30) - PLID 58034 - Created

using namespace ADODB;
using namespace boost;
using namespace std;
using namespace NXDATALIST2Lib;
using namespace NexTech_Accessor;

IMPLEMENT_DYNAMIC(CAnchorSettingsDlg, CNxDialog)

enum Callers
{
	ID = 0,
	Username,
	Password,
	PracticeUser,
	PracticePassword,
	Subdomain,
	NexWebKeyAlias,
	TransactionMode,
	PracticeUserValue,
	OriginalPracticeUserValue,	// (j.armen 2014-09-23 12:13) - PLID 58034 - Original Practice Username Value
	SubdomainValue,				// (j.armen 2014-09-23 12:13) - PLID 63758 - Current Subdomain Value
	OriginalSubdomainValue,		// (j.armen 2014-09-23 12:13) - PLID 63758 - Original Subdomain Value
};

CAnchorSettingsDlg::CAnchorSettingsDlg(CWnd* pParent)
	: CNxDialog(CAnchorSettingsDlg::IDD, pParent)
{
}

CAnchorSettingsDlg::~CAnchorSettingsDlg()
{
}

BOOL CAnchorSettingsDlg::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	try
	{
		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ANCHOR));
		SetIcon(hIcon, TRUE);		// Set big icon
		SetIcon(hIcon, FALSE);		// Set small icon

		CenterWindow();

		m_nxColor1.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxColor2.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxColor3.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_btnRefresh.AutoSet(NXB_REFRESH);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);

		m_nxlStatus.SetSingleLine();

		m_checkOverride.EnableWindow(IsNextechAdmin());

		m_pdlCallers = BindNxDataList2Ctrl(IDC_ANCHOR_NXDATALISTCTRL, false);

		RefreshSettings();

	}NxCatchAll(__FUNCTION__);

	return ret;
}

void CAnchorSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANCHOR_NXCOLORCTRL1, m_nxColor1);
	DDX_Control(pDX, IDC_ANCHOR_NXCOLORCTRL2, m_nxColor2);
	DDX_Control(pDX, IDC_ANCHOR_NXCOLORCTRL3, m_nxColor3);
	DDX_Control(pDX, IDC_ANCHOR_ENABLED, m_checkEnabled);
	DDX_Control(pDX, IDC_ANCHOR_CONNECTED, m_checkConnected);
	DDX_Control(pDX, IDC_ANCHOR_AFFIRMED, m_checkAffirmed);
	DDX_Control(pDX, IDC_ANCHOR_STATUS, m_nxlStatus);
	DDX_Control(pDX, IDC_ANCHOR_REFRESH, m_btnRefresh);
	DDX_Control(pDX, IDC_ANCHOR_CALLER_COUNT, m_nxlCallerCount);
	DDX_Control(pDX, IDC_ANCHOR_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_ANCHOR_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_ANCHOR_SET_PRAC_PASS, m_btnSetPracticeUserPassword);
	DDX_Control(pDX, IDC_ANCHOR_SHOW_CALLER_PASSWORD, m_checkShowPasswords);
	DDX_Control(pDX, IDC_ANCHOR_OVERRIDE, m_checkOverride);
	DDX_Control(pDX, IDC_ANCHOR_KEY, m_editAnchorKey);
	DDX_Control(pDX, IDC_ANCHOR_PASSWORD, m_editAnchorPassword);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Text(pDX, IDC_ANCHOR_KEY, m_nAnchorKey);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Used to reload the interface
void CAnchorSettingsDlg::RefreshSettings()
{
	m_bHasChanges = false;

	// (j.armen 2014-09-09 11:34) - PLID 58034 - If we get an API Exception, set the status labels
	_AnchorStatusPtr pAnchorStatus = NULL;
	try
	{
		CWaitCursor c;
		pAnchorStatus = GetAPI()->GetAnchorStatus(GetAPISubkey(), GetAPILoginToken());
	}
	NxCatchAllAPIPreCall(__FUNCTION__,
	{
		m_nxlStatus.SetText(strAPIWarningMessage);
		m_nxlStatus.SetToolTip(strAPIWarningMessage);
	}, {}, {});

	m_pStatus = pAnchorStatus;

	m_checkEnabled.EnableWindow(!!pAnchorStatus);
	m_checkEnabled.SetCheck((pAnchorStatus && pAnchorStatus->enabled) ? BST_CHECKED : BST_UNCHECKED);
	m_checkConnected.SetCheck((pAnchorStatus && pAnchorStatus->Connected) ? BST_CHECKED : BST_UNCHECKED);
	m_checkAffirmed.SetCheck((pAnchorStatus && pAnchorStatus->Affirmed) ? BST_CHECKED : BST_UNCHECKED);

	m_pdlCallers->Enabled = !!pAnchorStatus;
	m_pdlCallers->Clear();
	LoadCallers();

	m_nMaxCallers = pAnchorStatus ? pAnchorStatus->AllowedCallerCount : -1;

	if (pAnchorStatus) {
		m_nxlStatus.SetText(VarString(pAnchorStatus->ErrorStatus));
		m_nxlStatus.SetToolTip(VarString(pAnchorStatus->ErrorStatus));
	}

	m_nxlCallerCount.SetText(m_nMaxCallers > 0 ? FormatString("Number of Allowed Callers: %li", m_nMaxCallers) : "");

	m_checkOverride.SetCheck((pAnchorStatus && pAnchorStatus->AnchorOverrideSettings->enabled) ? BST_CHECKED : BST_UNCHECKED);

	RefreshOverrideUI();

	EnsureButtonState();
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - loads anchor callers from an anchor status
void CAnchorSettingsDlg::LoadCallers()
{
	_AnchorStatusPtr pAnchorStatus(m_pStatus);
	if (!pAnchorStatus)
		return;

	_RecordsetPtr prs = CreateParamRecordset(GetRemoteConnectionSnapshot(), R"(
SELECT
	P.ID, U.Username
FROM UsersT U
INNER JOIN PersonT P ON U.PersonID = P.ID
WHERE P.Archived = 0 AND P.ID > 0
ORDER BY U.Username

SELECT
	ID, Name
FROM NexWebSubdomainT
ORDER BY Name
)");

	m_mapUsers.clear();
	CString strUsers = "-1;<No User>;";
	for (; !prs->eof; prs->MoveNext())
	{
		CString strUsername = AdoFldString(prs, "Username");
		long nID = AdoFldLong(prs, "ID");
		m_mapUsers.insert(bimap<long, CiString>::value_type(nID, strUsername));
		strUsername.Replace(';', ' ');
		strUsers += FormatString("%li;%s;", nID, strUsername);
	}
	prs = prs->NextRecordset(NULL);

	m_pdlCallers->GetColumn(Callers::PracticeUser)->ComboSource = _bstr_t(strUsers);

	// (j.armen 2014-09-23 12:14) - PLID 63758 - Gather a map of possible subdomain choices
	m_mapSubdomains.clear();
	CString strSubdomains = "-1;<Default>;";

	// (j.armen 2014-09-25 13:12) - PLID 63758 - If more than one subdomains exist, populate the list
	if (prs->RecordCount > 1)
	{
		for (; !prs->eof; prs->MoveNext())
		{
			CString strSubdomain = AdoFldString(prs, "Name");
			long nID = AdoFldLong(prs, "ID");
			m_mapSubdomains.insert(bimap<long, CiString>::value_type(nID, strSubdomain));
			strSubdomain.Replace(';', ' ');
			strSubdomains += FormatString("%li;%s;", nID, strSubdomain);
		}
		m_pdlCallers->GetColumn(Callers::Subdomain)->Editable = VARIANT_TRUE;
	}
	else
	{
		// (j.armen 2014-09-25 13:12) - PLID 63758 - Only one subdomain option available, users cannot edit
		m_pdlCallers->GetColumn(Callers::Subdomain)->Editable = VARIANT_FALSE;
	}
	prs = prs->NextRecordset(NULL);

	m_pdlCallers->GetColumn(Callers::Subdomain)->ComboSource = _bstr_t(strSubdomains);

	Nx::SafeArray<IUnknown *> saCallers(pAnchorStatus->callers);
	for (unsigned long i = 0; i < saCallers.GetCount(); i++)
	{
		_AnchorCallerPtr pCaller(saCallers.GetAt(i));
		IRowSettingsPtr pRow = m_pdlCallers->GetNewRow();

		pRow->Value[Callers::ID] = pCaller->ID;
		pRow->Value[Callers::Username] = pCaller->username;
		pRow->Value[Callers::Password] = pCaller->password;

		// (j.armen 2014-09-23 12:14) - PLID 63758 - Select the subdomain.  Add a new option to our list if necessary
		pRow->Value[Callers::OriginalSubdomainValue] = pCaller->Subdomain;
		if (pCaller->Subdomain != g_cbstrNull)
		{
			pRow->Value[Callers::SubdomainValue] = pCaller->Subdomain;
			CString strSubdomain = VarString(pCaller->Subdomain);
			if (m_mapSubdomains.right.count(strSubdomain))
			{
				pRow->Value[Callers::Subdomain] = m_mapSubdomains.right.at(strSubdomain);
			}
			else
			{
				strSubdomain.Replace(';', ' ');
				IFormatSettingsPtr pFormat(__uuidof(FormatSettings));
				pFormat->PutDataType(VT_I4);
				pFormat->PutFieldType(cftComboSimple);
				pFormat->ComboSource = FormatBstr("0;%s;%s", strSubdomain, strSubdomains);
				// (j.armen 2014-09-25 13:12) - PLID 63758 - We'll have at least two options, since the selected subdomain isn't in our list.
				// Make sure the user can edit this row.
				pFormat->Editable = VARIANT_TRUE;
				pRow->CellFormatOverride[Callers::Subdomain] = pFormat;
				pRow->Value[Callers::Subdomain] = 0;
			}
		}
		else
		{
			pRow->Value[Callers::Subdomain] = -1;
		}

		// (j.armen 2014-09-23 12:15) - PLID 58034 - Select practice user.  Add additional item to our list if necessary
		pRow->Value[Callers::OriginalPracticeUserValue] = pCaller->PracUsername;
		if (pCaller->PracUsername != g_cbstrNull)
		{
			pRow->Value[Callers::PracticeUserValue] = pCaller->PracUsername;
			CString strUsername = VarString(pCaller->PracUsername);
			if (m_mapUsers.right.count(strUsername))
			{
				pRow->Value[Callers::PracticeUser] = m_mapUsers.right.at(strUsername);
			}
			else
			{
				strUsername.Replace(';', ' ');
				IFormatSettingsPtr pFormat(__uuidof(FormatSettings));
				pFormat->PutDataType(VT_I4);
				pFormat->PutFieldType(cftComboSimple);
				pFormat->ComboSource = FormatBstr("0;%s;%s", strUsername, strUsers);
				pFormat->Editable = VARIANT_TRUE;
				pRow->CellFormatOverride[Callers::PracticeUser] = pFormat;
				pRow->Value[Callers::PracticeUser] = 0;
			}
		}
		else
		{
			pRow->Value[Callers::PracticeUser] = -1;
		}

		pRow->Value[Callers::TransactionMode] = pCaller->TransactionMode ? g_cvarTrue : g_cvarFalse;
		pRow->Value[Callers::NexWebKeyAlias] = pCaller->Alias;

		m_pdlCallers->AddRowAtEnd(pRow, NULL);
	}
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Refreshes the override ui
void CAnchorSettingsDlg::RefreshOverrideUI()
{
	_AnchorStatusPtr pAnchorStatus(m_pStatus);

	if (pAnchorStatus)
	{
		if (m_checkOverride.GetCheck() == BST_CHECKED)
		{
			m_nAnchorKey = pAnchorStatus->AnchorOverrideSettings->AnchorKey;
			m_editAnchorPassword.SetWindowText(VarString(pAnchorStatus->AnchorOverrideSettings->AnchorPassword, ""));
		}
		else
		{
			m_nAnchorKey = g_pLicense->GetLicenseKey();
			m_editAnchorPassword.SetWindowText("");
		}

		m_editAnchorKey.EnableWindow(IsNextechAdmin() && m_checkOverride.GetCheck() == BST_CHECKED);
		m_editAnchorPassword.EnableWindow(IsNextechAdmin() && m_checkOverride.GetCheck() == BST_CHECKED);
		UpdateData(FALSE);
	}
	else
	{
		m_editAnchorKey.SetWindowText("");
		m_nAnchorKey = 0;
		m_editAnchorPassword.SetWindowText("");
		m_editAnchorKey.EnableWindow(FALSE);
		m_editAnchorPassword.EnableWindow(FALSE);
	}
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Utility to make sure button states are correct
void CAnchorSettingsDlg::EnsureButtonState()
{
	m_btnAdd.EnableWindow(m_nMaxCallers > m_pdlCallers->GetRowCount());
	m_btnRemove.EnableWindow(!!m_pdlCallers->CurSel);
	m_btnSetPracticeUserPassword.EnableWindow(!!m_pdlCallers->CurSel && VarLong(m_pdlCallers->CurSel->Value[Callers::PracticeUser]) != -1);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Saves callers
bool CAnchorSettingsDlg::SaveSettings()
{
	Nx::SafeArray<IUnknown *> saAnchorCaller = Nx::SafeArray<IUnknown *>();

	vector<CiString> aryCallers;

	for (auto pRow = m_pdlCallers->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		CiString strCallerKey = FormatString("%s-%s", VarString(pRow->Value[Callers::Username], ""), VarString(pRow->Value[Callers::NexWebKeyAlias], ""));
		if (find(aryCallers.begin(), aryCallers.end(), strCallerKey) != aryCallers.end())
		{
			AfxMessageBox("The same username has been specified more than once for the same alias.\nYour settings cannot be saved.");
			return false;
		}
		else
		{
			aryCallers.push_back(strCallerKey);
		}
		_AnchorCallerPtr pCaller(__uuidof(AnchorCaller));

		pCaller->ID = _bstr_t(pRow->Value[Callers::ID]);
		pCaller->username = _bstr_t(pRow->Value[Callers::Username]);
		pCaller->password = _bstr_t(pRow->Value[Callers::Password]);

		// (j.armen 2014-09-23 12:16) - PLID 58034 - We may not have a practice user value, if the entry was changed to <No User>
		if (pRow->Value[Callers::PracticeUserValue] != g_cvarEmpty)
			pCaller->PracUsername = _bstr_t(pRow->Value[Callers::PracticeUserValue]);
		if (pRow->Value[Callers::PracticePassword] != g_cvarEmpty)
			pCaller->PracPassword = _bstr_t(pRow->Value[Callers::PracticePassword]);

		pCaller->TransactionMode = VarBool(pRow->Value[Callers::TransactionMode]) ? VARIANT_TRUE : VARIANT_FALSE;
		pCaller->Alias = _bstr_t(pRow->Value[Callers::NexWebKeyAlias]);

		// (j.armen 2014-09-23 12:17) - PLID 63758 - If we have a subdomain, save it
		if (pRow->Value[Callers::SubdomainValue] != g_cvarEmpty)
			pCaller->Subdomain = _bstr_t(pRow->Value[Callers::SubdomainValue]);
		saAnchorCaller.push_back(pCaller);
	}

	CWaitCursor c;
	GetAPI()->EditAnchorCallers(GetAPISubkey(), GetAPILoginToken(), saAnchorCaller);

	m_bHasChanges = false;

	return true;
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Saves overrides
void CAnchorSettingsDlg::SaveOverrides()
{
	UpdateData(TRUE);
	_AnchorOverridePtr pOverride(__uuidof(AnchorOverride));
	pOverride->enabled = m_checkOverride.GetCheck() == BST_CHECKED ? VARIANT_TRUE : VARIANT_FALSE;
	pOverride->AnchorKey = m_nAnchorKey;
	pOverride->AnchorPassword = _bstr_t(m_editAnchorPassword.GetText());

	CWaitCursor c;
	GetAPI()->EditAnchorOverrides(GetAPISubkey(), GetAPILoginToken(), pOverride);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Verifies a practice password
bool CAnchorSettingsDlg::VerifyPassword(IRowSettingsPtr pRow, long nUserID)
{
	CString strPass;
	if (AskPassword(strPass))
	{
		// (b.savon 2015-12-21 10:12) - PLID 67707
		if (IsUserPasswordValid(strPass, GetCurrentLocationID(), nUserID, m_mapUsers.left.at(nUserID)) == FALSE)
		{
			auto ret = AfxMessageBox("Password could not be verified. Do you want to save this password?", MB_YESNOCANCEL);
			switch (ret)
			{
				case IDNO:		return true;
				case IDCANCEL:	return false;
				case IDYES:		break;
			}
		}

		pRow->Value[Callers::PracticePassword] = _bstr_t(strPass);
		m_bHasChanges = true;
		return true;
	}

	return false;
}


BEGIN_MESSAGE_MAP(CAnchorSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ANCHOR_REFRESH, &CAnchorSettingsDlg::OnBnClickedAnchorSettingsRefresh)
	ON_BN_CLICKED(IDOK, &CAnchorSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CAnchorSettingsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ANCHOR_ENABLED, &CAnchorSettingsDlg::OnBnClickedAnchorSettingsEnabled)
	ON_BN_CLICKED(IDC_ANCHOR_ADD, &CAnchorSettingsDlg::OnBnClickedAddCaller)
	ON_BN_CLICKED(IDC_ANCHOR_REMOVE, &CAnchorSettingsDlg::OnBnClickedRemoveCaller)
	ON_BN_CLICKED(IDC_ANCHOR_SET_PRAC_PASS, &CAnchorSettingsDlg::OnBnClickedAnchorSetPracticeUserPassword)
	ON_BN_CLICKED(IDC_ANCHOR_SHOW_CALLER_PASSWORD, &CAnchorSettingsDlg::OnBnClickedShowCallerPasswords)
	ON_BN_CLICKED(IDC_ANCHOR_OVERRIDE, &CAnchorSettingsDlg::OnBnClickedAnchorOverride)
	ON_EN_KILLFOCUS(IDC_ANCHOR_KEY, &CAnchorSettingsDlg::OnEnKillfocusAnchorKey)
	ON_EN_KILLFOCUS(IDC_ANCHOR_PASSWORD, &CAnchorSettingsDlg::OnEnKillfocusAnchorPassword)
END_MESSAGE_MAP()

// (j.armen 2014-07-09 17:30) - PLID 58034 - Button to refresh the interface
void CAnchorSettingsDlg::OnBnClickedAnchorSettingsRefresh()
{
	try
	{
		if (m_bHasChanges)
		{
			auto ret = AfxMessageBox("You have pending changes.  Refreshing will discard these changes. Do you want to save these changes now?", MB_YESNOCANCEL | MB_ICONQUESTION);
			switch (ret)
			{
				case IDYES:	if (SaveSettings()) break;
				case IDCANCEL:	return;
			}
		}

		RefreshSettings();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - OK
void CAnchorSettingsDlg::OnBnClickedOk()
{
	try
	{
		if (!SaveSettings())
			return;
		__super::OnOK();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Cancel
void CAnchorSettingsDlg::OnBnClickedCancel()
{
	try
	{
		if (m_bHasChanges)
			if (IDNO == AfxMessageBox("You have pending changes.  Are you sure you want to cancel?", MB_YESNO | MB_ICONQUESTION))
				return;

		__super::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Enable the Anchor
void CAnchorSettingsDlg::OnBnClickedAnchorSettingsEnabled()
{
	try
	{
		try
		{
			if (m_bHasChanges)
			{
				auto ret = AfxMessageBox("You have pending changes.  Disabling the Anchor will discard these changes.  Do you want to save these changes now?", MB_YESNOCANCEL | MB_ICONQUESTION);
				switch (ret)
				{
					case IDYES:	if (SaveSettings()) break;
					case IDCANCEL:	m_checkEnabled.SetCheck(m_checkEnabled.GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED);  return;
				}
			}

			CWaitCursor c;
			GetAPI()->EnableAnchor(GetAPISubkey(), GetAPILoginToken(), m_checkEnabled.GetCheck() == BST_CHECKED ? VARIANT_TRUE : VARIANT_FALSE);
		}NxCatchAllSilentCallThrow(
		{
			m_checkEnabled.SetCheck(m_checkEnabled.GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED);
		});
		RefreshSettings();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Ability to add callers
void CAnchorSettingsDlg::OnBnClickedAddCaller()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlCallers->GetNewRow();

		srand((UINT)time(NULL));
		CString strPassword;
		for (int i = 0; i < 10; i++)
		{
			strPassword += (char)((rand() % 26) + 97);
		}
		pRow->Value[Callers::Password] = _bstr_t(strPassword);
		pRow->Value[Callers::PracticeUser] = -1;
		pRow->Value[Callers::Subdomain] = -1;	// (j.armen 2014-09-24 11:30) - PLID 63758 - Set the subdomain to default
		pRow->Value[Callers::TransactionMode] = g_cvarFalse;
		pRow->Value[Callers::NexWebKeyAlias] = g_cbstrNull;

		m_pdlCallers->AddRowAtEnd(pRow, NULL);
		m_pdlCallers->CurSel = pRow;

		m_pdlCallers->StartEditing(pRow, Callers::Username);

		m_bAddingNew = true;

		EnsureButtonState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Ability to remove callers
void CAnchorSettingsDlg::OnBnClickedRemoveCaller()
{
	try
	{
		m_pdlCallers->RemoveRow(m_pdlCallers->CurSel);
		m_bHasChanges = true;
		EnsureButtonState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Set the practice password for a caller
void CAnchorSettingsDlg::OnBnClickedAnchorSetPracticeUserPassword()
{
	try
	{
		VerifyPassword(m_pdlCallers->CurSel, VarLong(m_pdlCallers->CurSel->Value[Callers::PracticeUser]));
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Toggle showing caller passwords
void CAnchorSettingsDlg::OnBnClickedShowCallerPasswords()
{
	try
	{
		if (m_checkShowPasswords.GetCheck() == BST_CHECKED)
		{
			m_pdlCallers->GetColumn(Callers::Password)->StoredWidth = m_pdlCallers->GetColumn(Callers::Password)->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
			m_pdlCallers->GetColumn(Callers::Password)->ColumnStyle = EColumnStyle::csEditable | EColumnStyle::csVisible;

		}
		else
		{
			m_pdlCallers->GetColumn(Callers::Password)->StoredWidth = 0;
			m_pdlCallers->GetColumn(Callers::Password)->ColumnStyle = EColumnStyle::csFixedWidth | EColumnStyle::csVisible;
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handle enabling anchor overrides
void CAnchorSettingsDlg::OnBnClickedAnchorOverride()
{
	try
	{
		if (m_bHasChanges)
		{
			auto ret = AfxMessageBox(FormatString(
				"You have pending changes.  "
				"%s the Overrides will discard these changes.  "
				"Do you want to save these changes now?",
				m_checkOverride.GetCheck() == BST_CHECKED ? "Disabling" : "Enabling"),
				MB_YESNOCANCEL | MB_ICONQUESTION);
			switch (ret)
			{
				case IDYES:	if (SaveSettings()) break;
				case IDCANCEL:	m_checkOverride.SetCheck(
					m_checkOverride.GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED);
					return;
			}
		}

		try
		{
			SaveOverrides();
		}NxCatchAllSilentCallThrow(
		{
			try
			{
				RefreshSettings();
			}NxCatchAllIgnore();
		});

		RefreshSettings();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handle saving when anchor key looses focus
void CAnchorSettingsDlg::OnEnKillfocusAnchorKey()
{
	try
	{
		if (!UpdateData(TRUE))
			return;

		if (AsString(m_nAnchorKey) != m_editAnchorKey.GetText())
		{
			AfxMessageBox("Enter an integer.", MB_ICONEXCLAMATION);
			m_editAnchorKey.SetFocus();
			return;
		}

		long nOldAnchorKey = _AnchorStatusPtr(m_pStatus)->AnchorOverrideSettings->AnchorKey;
		if (nOldAnchorKey != m_nAnchorKey)
		{
			if (m_bHasChanges)
			{
				auto ret = AfxMessageBox(
					"You have pending changes.  "
					"Modifying the Overrides will discard these changes.  "
					"Do you want to save these changes now?",
					MB_YESNOCANCEL | MB_ICONQUESTION);
				switch (ret)
				{
					case IDYES:	if (SaveSettings()) break;
					case IDCANCEL:	m_editAnchorKey.SetWindowText(AsString(nOldAnchorKey)); return;
				}
			}

			try
			{
				SaveOverrides();
			}NxCatchAllSilentCallThrow(
			{
				try
				{
					RefreshSettings();
				}NxCatchAllIgnore();
			});

			RefreshSettings();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handle saving when anchor password looses focus
void CAnchorSettingsDlg::OnEnKillfocusAnchorPassword()
{
	try
	{
		CString strOldAnchorPassword = VarString(_AnchorStatusPtr(m_pStatus)->AnchorOverrideSettings->AnchorPassword);
		if (strOldAnchorPassword != m_editAnchorPassword.GetText())
		{
			if (m_bHasChanges)
			{
				auto ret = AfxMessageBox(
					"You have pending changes.  "
					"Modifying the Overrides will discard these changes.  "
					"Do you want to save these changes now?",
					MB_YESNOCANCEL | MB_ICONQUESTION);
				switch (ret)
				{
					case IDYES:	if (SaveSettings()) break;
					case IDCANCEL:	m_editAnchorPassword.SetWindowText(strOldAnchorPassword); return;
				}
			}

			try
			{
				SaveOverrides();
			}NxCatchAllSilentCallThrow(
			{
				try
				{
					RefreshSettings();
				}NxCatchAllIgnore();
			});

			RefreshSettings();
		}
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CAnchorSettingsDlg, CNxDialog)
	ON_EVENT(CAnchorSettingsDlg, IDC_ANCHOR_NXDATALISTCTRL, 32, CAnchorSettingsDlg::ShowContextMenuAnchorCallers, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CAnchorSettingsDlg, IDC_ANCHOR_NXDATALISTCTRL, 2, CAnchorSettingsDlg::SelChangedAnchorCallers, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAnchorSettingsDlg, IDC_ANCHOR_NXDATALISTCTRL, 10, CAnchorSettingsDlg::EditingFinishedAnchorCallers, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAnchorSettingsDlg, IDC_ANCHOR_NXDATALISTCTRL, 9, CAnchorSettingsDlg::EditingFinishingAnchorCallers, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handle right click context on datalist
void CAnchorSettingsDlg::ShowContextMenuAnchorCallers(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue)
{
	try
	{
		GetDlgItem(IDC_ANCHOR_NXDATALISTCTRL)->SetFocus();

		IRowSettingsPtr pRow(lpRow);
		m_pdlCallers->CurSel = pRow;

		if (!pRow)
			return;

		CNxMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		mnu.InsertMenu(0, MF_BYPOSITION, WM_USER + 1, "&Remove");

		long nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, x, y, this, NULL);
		mnu.DestroyMenu();

		if (nRet == WM_USER + 1)
		{
			m_pdlCallers->RemoveRow(m_pdlCallers->CurSel);
			m_bHasChanges = true;
			EnsureButtonState();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handle changing caller selection
void CAnchorSettingsDlg::SelChangedAnchorCallers(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		EnsureButtonState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handle editing caller finished
void CAnchorSettingsDlg::EditingFinishedAnchorCallers(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		if (bCommit)
		{
			IRowSettingsPtr pRow(lpRow);
			if (m_bAddingNew && VarString(varNewValue).IsEmpty())
			{
				m_pdlCallers->RemoveRow(pRow);
			}
			else
			{
				m_bHasChanges |= _variant_t(varOldValue) != _variant_t(varNewValue);
			}


			if (nCol == Callers::PracticeUser || nCol == Callers::Subdomain)
			{
				long nValue = VarLong(varNewValue);

				switch (nValue)
				{
					case -1:
						if (nCol == Callers::PracticeUser)		// (j.armen 2014-09-23 12:19) - PLID 58034 - <No User>
							pRow->Value[Callers::PracticeUserValue] = g_cvarEmpty;
						else if (nCol == Callers::Subdomain)	// (j.armen 2014-09-23 12:17) - PLID 63758 - <Default Subdomain>
							pRow->Value[Callers::SubdomainValue] = g_cvarEmpty;
						break;
					case 0:
						if (nCol == Callers::PracticeUser)		// (j.armen 2014-09-23 12:19) - PLID 58034 - Original User - In case user changed selection
							pRow->Value[Callers::PracticeUserValue] = pRow->Value[Callers::OriginalPracticeUserValue];
						else if (nCol == Callers::Subdomain)	// (j.armen 2014-09-23 12:17) - PLID 63758 - Original Subdomain - In case user changed selection, and are now going back.
							pRow->Value[Callers::SubdomainValue] = pRow->Value[Callers::OriginalSubdomainValue];
						break;
					default:
						if (nCol == Callers::PracticeUser)		// (j.armen 2014-09-23 12:19) - PLID 58034 - Select user from map
							pRow->Value[Callers::PracticeUserValue] = _bstr_t(m_mapUsers.left.at(VarLong(varNewValue)));
						else if (nCol == Callers::Subdomain)	// (j.armen 2014-09-23 12:17) - PLID 63758 - Selecte subdomain from map
							pRow->Value[Callers::SubdomainValue] = _bstr_t(m_mapSubdomains.left.at(VarLong(varNewValue)));
				}
			}
		}

		m_bAddingNew = false;
		EnsureButtonState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-09 17:30) - PLID 58034 - Handling editing caller finishing
void CAnchorSettingsDlg::EditingFinishingAnchorCallers(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try
	{
		if (nCol == Callers::PracticeUser)
		{
			if (VarLong(varOldValue) != VarLong(*pvarNewValue))
			{
				IRowSettingsPtr pRow(lpRow);

				// (j.armen 2014-09-23 12:19) - PLID 58034 - Always clear the password when changing users
				pRow->Value[Callers::PracticePassword] = g_cvarEmpty;
				if (VarLong(*pvarNewValue) != -1)
				{
					VerifyPassword(pRow, VarLong(*pvarNewValue));
				}
			}
			else
			{
				*pbCommit = FALSE;
			}
		}
	}NxCatchAll(__FUNCTION__);
}
