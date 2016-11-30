// (r.gonet 05/19/2014) - PLID 61832 - Created (took over from r.wilson)
// ConfigureChargeLevelProvider.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ConfigureChargeLevelProviderDlg.h"
#include "AdministratorRc.h"
using namespace ADODB;
using namespace NXDATALIST2Lib;

#define NXM_ENSURE_CONTROLS	WM_APP+0x100 

// CConfigureChargeLevelProviderDlg dialog

IMPLEMENT_DYNAMIC(CConfigureChargeLevelProviderDlg, CNxDialog)

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Standard constructor.</summary>
/// <param name="pParent">Parent window</param>
/// <param name="nServiceID">The ID of the initial ServiceT record to select in the Service Codes/Products dropdown. May not be -1.</param>
/// <param name="eServicetype">The type of ServiceT records to show in the dialog.</param>
CConfigureChargeLevelProviderDlg::CConfigureChargeLevelProviderDlg(CWnd* pParent, long nServiceID, EServiceType eServiceType)
	: CNxDialog(CConfigureChargeLevelProviderDlg::IDD, pParent)
{
	if (nServiceID != -1) {
		// (r.gonet 05/19/2014) - PLID 62248 - Make this our initial selection for the service codes combo.
		m_arySelectedServiceCodes.Add(nServiceID);
	} else {
		// (r.gonet 05/19/2014) - PLID 62248 - Since we don't have a row for None or All with the Service Codes and Products, we must require the service code ID
		ThrowNxException("%s : No initial service code ID was supplied.", __FUNCTION__);
	}
	m_eServiceType = eServiceType;
}

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Standard destructor.</summary>
CConfigureChargeLevelProviderDlg::~CConfigureChargeLevelProviderDlg()
{
}

void CConfigureChargeLevelProviderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_CCLP_SERV_CODES, m_nxstaticServiceCodes);
	DDX_Control(pDX, IDC_CCLP_MULTI_SERVICE_CODES_STATIC, m_nxlMultiServiceCodes);
	DDX_Control(pDX, IDC_STATIC_CCLP_PRODUCTS, m_nxstaticProducts);
	DDX_Control(pDX, IDC_CCLP_MULTI_PRODUCTS_STATIC, m_nxlMultiProducts);
	DDX_Control(pDX, IDC_STATIC_CCLP_PROV, m_nxstaticProviders);
	DDX_Control(pDX, IDC_CCLP_MULTI_PROVIDERS_STATIC, m_nxlMultiProviders);
	DDX_Control(pDX, IDC_STATIC_CCLP_LOC, m_nxstaticLocations);
	DDX_Control(pDX, IDC_CCLP_MULTI_LOCATIONS_STATIC, m_nxlMultiLocations);
	DDX_Control(pDX, IDC_STATIC_CCLP_HCFA_GROUP, m_nxstaticHCFAGroups);
	DDX_Control(pDX, IDC_CCLP_MULTI_HCFA_GROUPS_STATIC, m_nxlMultiHCFAGroups);
	DDX_Control(pDX, IDC_CCLP_REFERRING_PROVIDER_CHECK,	m_checkReferringProvider);
	DDX_Control(pDX, IDC_CCLP_ORDERING_PROVIDER_CHECK, m_checkOrderingProvider);
	DDX_Control(pDX, IDC_CCLP_SUPERVISING_PROVIDER_CHECK, m_checkSupervisingProvider);
	DDX_Control(pDX, IDC_CCLP_TOOLTIP_CRITERIA_SECTION, m_icoToolTipCriteriaSection);
	DDX_Control(pDX, IDC_CCLP_TOOLTIP_SETTINGS_SECTION, m_icoToolTipSettingsSection);
	DDX_Control(pDX, IDC_CCLP_NXCOLOR, m_nxcolorBackground);
	DDX_Control(pDX, IDC_CCLP_APPLY_BTN, m_btnApply);
}

BEGIN_MESSAGE_MAP(CConfigureChargeLevelProviderDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CCLP_APPLY_BTN, &CConfigureChargeLevelProviderDlg::OnBnClickedCclpApplyBtn)
	ON_BN_CLICKED(IDC_CCLP_REFERRING_PROVIDER_CHECK, &CConfigureChargeLevelProviderDlg::OnBnClickedCclpReferringProviderCheck)
	ON_BN_CLICKED(IDC_CCLP_ORDERING_PROVIDER_CHECK, &CConfigureChargeLevelProviderDlg::OnBnClickedCclpOrderingProviderCheck)
	ON_BN_CLICKED(IDC_CCLP_SUPERVISING_PROVIDER_CHECK, &CConfigureChargeLevelProviderDlg::OnBnClickedCclpSupervisingProviderCheck)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CConfigureChargeLevelProviderDlg::OnLabelClick)
	ON_MESSAGE(NXM_ENSURE_CONTROLS, &CConfigureChargeLevelProviderDlg::OnEnsureControls)
END_MESSAGE_MAP()

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Initializes the dialog.</summary>
BOOL CConfigureChargeLevelProviderDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (r.gonet 05/19/2014) - PLID 62248 - Depending on where we are called from, set the background color appropriately.
		if (m_eServiceType == EServiceType::Product) {
			m_nxcolorBackground.SetColor(GetNxColor(GNC_INVENTORY, 0));
		} else {
			m_nxcolorBackground.SetColor(GetNxColor(GNC_ADMIN, 0));
		}

		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);
		
		// (r.gonet 05/19/2014) - PLID 62248 - Multi labels are hidden by default and datalist dropdowns are shown instead.
		m_nxlMultiProviders.SetType(dtsDisabledHyperlink);
		m_nxlMultiProviders.SetSingleLine(true);
		
		m_nxlMultiLocations.SetType(dtsDisabledHyperlink);
		m_nxlMultiLocations.SetSingleLine(true);
		
		m_nxlMultiHCFAGroups.SetType(dtsDisabledHyperlink);
		m_nxlMultiHCFAGroups.SetSingleLine(true);

		m_nxlMultiServiceCodes.SetType(dtsDisabledHyperlink);
		m_nxlMultiServiceCodes.SetSingleLine(true);

		m_nxlMultiProducts.SetType(dtsDisabledHyperlink);
		m_nxlMultiProducts.SetSingleLine(true);

		// (r.gonet 05/19/2014) - PLID 61833 - We have some icons that when hovered over, give help text. Set them up.
		InitializeToolTipText();

		// (r.gonet 05/19/2014) - PLID 62248 - Criteria combo boxes
		m_pProviderCombo = BindNxDataList2Ctrl(IDC_CCLP_PROVIDER_COMBO, true);
		m_pLocationCombo = BindNxDataList2Ctrl(IDC_CCLP_LOCATION_COMBO, true);
		m_pHCFAGroupCombo = BindNxDataList2Ctrl(IDC_CCLP_HCFA_GROUP_COMBO, true);
		// (r.gonet 05/19/2014) - PLID 62248 - Initialize the appropriate datalist for displaying ServiceT records.
		if (m_eServiceType == EServiceType::CPTCode) {
			m_pServiceCodeCombo = BindNxDataList2Ctrl(IDC_CCLP_SERVICE_CODE_COMBO, true);
			m_pProductCombo = NULL;
			GetDlgItem(IDC_STATIC_CCLP_SERV_CODES)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_CCLP_SERVICE_CODE_COMBO)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_STATIC_CCLP_PRODUCTS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CCLP_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
		} else if(m_eServiceType == EServiceType::Product) {
			m_pServiceCodeCombo = NULL;
			m_pProductCombo = BindNxDataList2Ctrl(IDC_CCLP_PRODUCT_COMBO, true);
			GetDlgItem(IDC_STATIC_CCLP_SERV_CODES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CCLP_SERVICE_CODE_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_CCLP_PRODUCTS)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_CCLP_PRODUCT_COMBO)->ShowWindow(SW_SHOWNA);
		}

		// (r.gonet 05/19/2014) - PLID 62249 - Provider Settings combo boxes
		m_pReferringProviderCombo = BindNxDataList2Ctrl(IDC_CCLP_REFERRING_PROVIDER_COMBO, false);
		m_pReferringProviderCombo->FromClause = _bstr_t(GetLowerProviderComboFromSql());
		m_pReferringProviderCombo->WhereClause = _bstr_t(GetReferringProviderComboWhereSql());
		m_pReferringProviderCombo->Requery();
		// (r.gonet 05/19/2014) - PLID 62249
		m_pOrderingProviderCombo = BindNxDataList2Ctrl(IDC_CCLP_ORDERING_PROVIDER_COMBO, false);
		m_pOrderingProviderCombo->FromClause = _bstr_t(GetLowerProviderComboFromSql());
		m_pOrderingProviderCombo->WhereClause = _bstr_t(GetOrderingProviderComboWhereSql());
		m_pOrderingProviderCombo->Requery();
		// (r.gonet 05/19/2014) - PLID 62249 
		m_pSupervisingProviderCombo = BindNxDataList2Ctrl(IDC_CCLP_SUPERVISING_PROVIDER_COMBO, false);
		m_pSupervisingProviderCombo->FromClause = _bstr_t(GetLowerProviderComboFromSql());
		m_pSupervisingProviderCombo->WhereClause = _bstr_t(GetSupervisingProviderComboWhereSql());
		m_pSupervisingProviderCombo->Requery();

		// (r.gonet 05/19/2014) - PLID 62249 - We need the dialog to be fully initialized before we ensure the controls are in valid states. Thus post a message to do that later and let the dialog initialization continue.
		this->PostMessage(NXM_ENSURE_CONTROLS);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handles asynchronous calls to EnsureControls.</summary>
/// <param name="wParam">Unused</param>
/// <param name="lParam">Unused</param>
LRESULT CConfigureChargeLevelProviderDlg::OnEnsureControls(WPARAM wParam, LPARAM lParam)
{
	try {
		// First ensure that all of the datalists have finished requerying, because we are about to make selections on them.
		m_pProviderCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pLocationCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pHCFAGroupCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		if (m_pServiceCodeCombo) {
			m_pServiceCodeCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		} else if (m_pProductCombo) {
			m_pProductCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}
		m_pReferringProviderCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pOrderingProviderCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pSupervisingProviderCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		// Handle any remaining messages from the datalists requerying.
		PeekAndPump();
		// Now ensure the controls are in valid states.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Ensures the controls on the dialog are in valid states.</summary>
void CConfigureChargeLevelProviderDlg::EnsureControls()
{
	// (r.gonet 05/19/2014) - PLID 62249 - The setting combo boxes are limited to the NoSelection row when the Required checkbox is unchecked. Enforce this rule.
	if (m_checkReferringProvider.GetCheck() == BST_UNCHECKED) {
		// Ensure we have no selection set
		m_pReferringProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::NoSelection), m_pReferringProviderCombo->GetFirstRow(), VARIANT_TRUE);
	}
	// (r.gonet 05/19/2014) - PLID 62249 
	if (m_checkOrderingProvider.GetCheck() == BST_UNCHECKED) {
		// Ensure we have no selection set
		m_pOrderingProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::NoSelection), m_pOrderingProviderCombo->GetFirstRow(), VARIANT_TRUE);
	}
	// (r.gonet 05/19/2014) - PLID 62249
	if (m_checkSupervisingProvider.GetCheck() == BST_UNCHECKED) {
		// Ensure we have no selection set
		m_pSupervisingProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::NoSelection), m_pSupervisingProviderCombo->GetFirstRow(), VARIANT_TRUE);
	}

	// Get what is currently selected for the criteria combo boxes.
	long nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID;
	GetCriteriaSelections(nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID);

	// (r.gonet 05/19/2014) - PLID 62249
	BOOL bEnableSettings = TRUE;
	if (nProviderID == (long)ECriteriaSpecialIds::None || nLocationID == (long)ECriteriaSpecialIds::None || nHCFAGroupID == (long)ECriteriaSpecialIds::None || nServiceCodeID == (long)ECriteriaSpecialIds::None) {
		// (r.gonet 05/19/2014) - PLID 62248 - One of the criteria combo boxes is set with NULL. Don't let them save or modify the settings.
		bEnableSettings = FALSE;
	}

	// (r.gonet 05/19/2014) - PLID 62248
	m_checkReferringProvider.EnableWindow(bEnableSettings);
	m_checkOrderingProvider.EnableWindow(bEnableSettings);
	m_checkSupervisingProvider.EnableWindow(bEnableSettings);
	GetDlgItem(IDC_CCLP_REFERRING_PROVIDER_COMBO)->EnableWindow(bEnableSettings && (m_checkReferringProvider.GetCheck() == BST_CHECKED));
	GetDlgItem(IDC_CCLP_ORDERING_PROVIDER_COMBO)->EnableWindow(bEnableSettings && (m_checkOrderingProvider.GetCheck() == BST_CHECKED));
	GetDlgItem(IDC_CCLP_SUPERVISING_PROVIDER_COMBO)->EnableWindow(bEnableSettings && (m_checkSupervisingProvider.GetCheck() == BST_CHECKED));
	m_btnApply.EnableWindow(bEnableSettings);
}

// (r.gonet 05/19/2014) - PLID 61833 - Added.
/// <summary>Initializes the tooltip text for the tooltip help buttons.</summary>
void CConfigureChargeLevelProviderDlg::InitializeToolTipText()
{
	CString strToolTipCriteriaSection = FormatString(
		"Configure Charge Level Providers \r\n"
		"\r\n"
		"To configure Charge Level Providers, select a %s from the drop down along with a provider, location, and HCFA group. "
		"If you are making changes to the default settings, be "
		"sure to press Apply to save your changes."
		, (m_eServiceType == EServiceType::Product ? "product" : "service code"));
	m_icoToolTipCriteriaSection.LoadToolTipIcon(
		AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK),
		strToolTipCriteriaSection,
		true, true, false);

	CString strToolTipSettingsSection = 
		"Check the box next to the provider type(s) required. You can set a default provider using the drop down menu. If you do not select a default provider, you will be prompted to select one from the bill. \r\n"
		"\r\n"
		"On the HCFA form, these providers will print in the following priority: Referring Provider, Ordering Provider, and Supervising Provider – per National Uniform Claim Committee standards.";

	m_icoToolTipSettingsSection.LoadToolTipIcon(
		AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK),
		strToolTipSettingsSection,
		true, true, false);
}

// CConfigureChargeLevelProvider message handlers
BEGIN_EVENTSINK_MAP(CConfigureChargeLevelProviderDlg, CNxDialog)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_PROVIDER_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpProviderCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_LOCATION_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpLocationCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_HCFA_GROUP_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpHcfaGroupCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_SERVICE_CODE_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpServiceCodeCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_PRODUCT_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpProductCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_REFERRING_PROVIDER_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpReferringProviderCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_ORDERING_PROVIDER_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpOrderingProviderCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_SUPERVISING_PROVIDER_COMBO, 18, CConfigureChargeLevelProviderDlg::RequeryFinishedCclpSupervisingProviderCombo, VTS_I2)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_PROVIDER_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_LOCATION_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_HCFA_GROUP_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_SERVICE_CODE_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_PRODUCT_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_REFERRING_PROVIDER_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_ORDERING_PROVIDER_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_SUPERVISING_PROVIDER_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_PROVIDER_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpProviderCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_LOCATION_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpLocationCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_HCFA_GROUP_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpHcfaGroupCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_SERVICE_CODE_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpServiceCodeCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_PRODUCT_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpProductCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_REFERRING_PROVIDER_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpReferringProviderCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_ORDERING_PROVIDER_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpOrderingProviderCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureChargeLevelProviderDlg, IDC_CCLP_SUPERVISING_PROVIDER_COMBO, 16, CConfigureChargeLevelProviderDlg::SelChosenCclpSupervisingProviderCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Callback for the multi-select dialog enabling the user to select all and unselect all.</summary>
/// <param name="pwndMultSelDlg">Pointer to the CMultiSelectDlg that called this callback.</param>
/// <param name="lpRow">Pointer to the datalist2 row that the user right-clicked on.</param>
/// <returns>TRUE if the context menu was shown. FALSE if not.</returns>
BOOL CALLBACK CMultiSelectDlg_ContextMenuProc(IN CMultiSelectDlg *pwndMultSelDlg, IN LPARAM pParam, IN NXDATALIST2Lib::IRowSettings *lpRow, IN CWnd* pContextWnd, IN const CPoint &point, IN OUT CArray<long, long> &m_aryOtherChangedMasterIDs)
{
	// The context menu for the data element list is based on the current selection
	if (lpRow) {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// Build the menu for the current row
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 1, "&Select All");
		mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 2, "&Unselect All");

		// Pop up the menu and gather the immediate response
		CPoint pt = CalcContextMenuPos(pContextWnd, point);
		long nMenuResult = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, pwndMultSelDlg);
		switch (nMenuResult) {
		case 1: // select all
		{
			// Check all the rows in the multi-select list that are unchecked
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pwndMultSelDlg->GetDataList2()->GetFirstRow();
			while (pRowIter) {
				if (!VarBool(pRowIter->GetValue(CMultiSelectDlg::mslcSelected), FALSE)) {
					pRowIter->PutValue(CMultiSelectDlg::mslcSelected, g_cvarTrue);
				}
				pRowIter = pRowIter->GetNextRow();
			}
			break;
		}
		case 2: // unselect all
		{
			// Uncheck all the rows in the multi-select list that are checked
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pwndMultSelDlg->GetDataList2()->GetFirstRow();
			while (pRowIter) {
				if (VarBool(pRowIter->GetValue(CMultiSelectDlg::mslcSelected), TRUE)) {
					pRowIter->PutValue(CMultiSelectDlg::mslcSelected, g_cvarFalse);
				}
				pRowIter = pRowIter->GetNextRow();
			}
			break;
		}
		case 0:
			// The user canceled, do nothing
			break;
		default:
			// Unexpected response!
			ASSERT(FALSE);
			ThrowNxException("%s : Unexpected return value %li from context menu!", __FUNCTION__, nMenuResult);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Shows a multi-select dialog for a given combo box/multi-label pair and reflects the selections in those controls.</summary>
/// <param name="nxlMultiLabel">The CNxLabel control that shows multiple selections for <paramref name="pCombo" />. Selections reflect back to this label.</param>
/// <param name="pCombo">The datalist2 combo box to draw the multiple selection items from. Selections reflect back to this combo.</param>
/// <param name="nComboControlID">The control identifier of the datalist2 combo box. Needed to show and hide it.</param>
/// <param name="aryCurrentSelections">An array containing the IDs of the currently selected items in pCombo. Will be modified to reflect the user's selections.</param>
/// <param name="pbAllSelected">A pointer to a boolean variable stating whether the combo box's previous selection was all the codes. Will be set based on the user's selections.
/// May be NULL if the All row doesn't exist.</param>
/// <param name="strConfigRTName">An identifier to store the column widths of the CMultiSelectDlg. Should be unique per combo box.</param>
/// <param name="strDescription">Text that will go at the top of the multiple selection dialog describing to the user what to do.</param>
/// <param name="nIDColumnIndex">The datalist column index of the column in <paramref name="pCombo" /> that stores the database ID. Defaults to 0.</param>
/// <param name="nDescriptionColumnIndex">The datalist column index of the column in <paramref name="pCombo" /> that stores the description/name of the record. Defaults to 1.</param>
/// <param name="paryExtraColumnIndices">Pointer to an array of datalist column indices of the columns in <paramref name="pCombo" /> that store additional values to display as separate columns
/// in the multiple selection dialog. May be NULL if no extra columns are desired. Defaults to NULL.</param>
void CConfigureChargeLevelProviderDlg::HandleMultipleSelection(
	CNxLabel &nxlMultiLabel, NXDATALIST2Lib::_DNxDataListPtr pCombo, UINT nComboControlID, CArray<long, long> &aryCurrentSelectionIDs,
	bool *pbAllSelected, CString strConfigRTName, CString strDescription,
	short nIDColumnIndex/* = 0*/, short nDescriptionColumnIndex/* = 1*/, CArray<short, short> *paryExtraColumnIndices/* = NULL*/)
{
	if (pCombo == NULL) {
		ThrowNxException("%s : pCombo is NULL (strConfigRTName = %s).", __FUNCTION__, strConfigRTName);
	}

	CMultiSelectDlg dlg(this, strConfigRTName);
	dlg.m_pfnContextMenuProc = CMultiSelectDlg_ContextMenuProc;
	dlg.m_nContextMenuProcParam = (LPARAM)this;
	dlg.m_bPutSelectionsAtTop = TRUE;

	if (aryCurrentSelectionIDs.GetSize() > 1) {
		// Preselect whatever we had selected before.
		dlg.PreSelect(aryCurrentSelectionIDs);
	}
	
	// Ensure that we don't end up showing the special rows.
	CVariantArray vaIDsToSkip;
	vaIDsToSkip.Add(_variant_t((long)ECriteriaSpecialIds::All, VT_I4));
	vaIDsToSkip.Add(_variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
	vaIDsToSkip.Add(_variant_t((long)ECriteriaSpecialIds::None, VT_I4));

	// Now show the user the multi-select dialog.
	if (IDOK == dlg.OpenWithDataList2(pCombo, vaIDsToSkip, strDescription, 1, 0xFFFFFFFF, nIDColumnIndex, nDescriptionColumnIndex, paryExtraColumnIndices)) {
		// OK, they selected some records. Reflect their selections in the combo box and multi-label.

		// Get rid of the old selections. We have some new ones.
		aryCurrentSelectionIDs.RemoveAll();
		dlg.FillArrayWithIDs(aryCurrentSelectionIDs);
		// We need to tell if the user selected all the rows or just some of them.
		CArray<long, long> aryUnselectedIDs;
		dlg.FillArrayWithUnselectedIDs(&aryUnselectedIDs);

		// Did they select everything?
		if (pbAllSelected != NULL) {
			if (aryCurrentSelectionIDs.GetSize() > 1 && aryUnselectedIDs.GetSize() == 0) {
				// Yep. Note that if there was only 1 record available to select, we don't count that as all because:
				// If only one check box is checked, only display that X in the drop down.
				*pbAllSelected = true;
			} else {
				// Nope. Either there are some left over rows they didn't select or there was only 1 thing to select and they selected it.
				*pbAllSelected = false;
			}
		} else {
			// We don't care about selecting the All Row.
		}
		
		// Grab the all row. We need to make sure we have it available and maybe select it.
		NXDATALIST2Lib::IRowSettingsPtr pAllRow = pCombo->FindByColumn(nIDColumnIndex, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), pCombo->GetFirstRow(), VARIANT_FALSE);
		// If the all row is available in the combo box and the user selected all the rows, then select the All row in the combo box.
		if (pbAllSelected != NULL && *pbAllSelected == true && pAllRow != NULL) {
			// Hide any hyperlink we might have showing and show the combo box instead.
			nxlMultiLabel.SetText("");
			nxlMultiLabel.SetType(dtsDisabledHyperlink);
			nxlMultiLabel.ShowWindow(SW_HIDE);
			GetDlgItem(nComboControlID)->ShowWindow(SW_SHOW);

			pCombo->CurSel = pAllRow;
			// All means all. Not some subset. We don't want to record the Ids.
			aryCurrentSelectionIDs.RemoveAll();
		} else if (aryCurrentSelectionIDs.GetSize() == 1) {
			// Show single selections in the combo box.
			nxlMultiLabel.SetText("");
			nxlMultiLabel.SetType(dtsDisabledHyperlink);
			nxlMultiLabel.ShowWindow(SW_HIDE);
			GetDlgItem(nComboControlID)->ShowWindow(SW_SHOW);

			long nSelectedId = aryCurrentSelectionIDs.GetAt(0);
			NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = pCombo->FindByColumn(nIDColumnIndex, _variant_t(nSelectedId, VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE);
			if (pSelectedRow == NULL) {
				ThrowNxException("%s : Could not find selected row (strConfigRTName = %s).", __FUNCTION__, strConfigRTName);
			}
		} else if (aryCurrentSelectionIDs.GetSize() > 1) {
			// Show multiple selections (other than All selections) in the multi select label
			CString strMultiSelectString = dlg.GetMultiSelectString();
			if (strMultiSelectString.GetLength() > 255) {
				strMultiSelectString = strMultiSelectString.Left(255);
			}

			GetDlgItem(nComboControlID)->ShowWindow(SW_HIDE);
			nxlMultiLabel.ShowWindow(SW_SHOW);
			nxlMultiLabel.SetText(strMultiSelectString);
			nxlMultiLabel.SetType(dtsHyperlink);
		} else {
			// OK was pressed but no row was selected. This should be impossible.
			ThrowNxException("%s : No row was selected in the multi-select dialog but OK was pressed (strConfigRTName = %s).", __FUNCTION__, strConfigRTName);
		}

		nxlMultiLabel.AskParentToRedrawWindow();
		ReloadSettings();
	} else {
		// They cancelled. Revert to the old selections
		if (pbAllSelected && *pbAllSelected == true) {
			// All records were selected before. Revert to the All row if we have it.
			NXDATALIST2Lib::IRowSettingsPtr pOldSelection = pCombo->FindByColumn(nIDColumnIndex, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE);
		} else if((pbAllSelected == NULL || *pbAllSelected == false) && aryCurrentSelectionIDs.GetSize() == 1) {
			// Only one record was selected before. Revert to it in the combo box.
			NXDATALIST2Lib::IRowSettingsPtr pOldSelection = pCombo->FindByColumn(nIDColumnIndex, _variant_t(aryCurrentSelectionIDs.GetAt(0), VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE);
		} else if(aryCurrentSelectionIDs.GetSize() > 1) {
			// The multiple label is shown. We don't need to revert anything.
		} else if (aryCurrentSelectionIDs.GetSize() == 0) {
			// Highly unusual situation here. Don't think it is possible under normal circumstances. Try to select the All row and if unsuccessful, select NULL.
			if (!pCombo->FindByColumn(nIDColumnIndex, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE)) {
				// Woah. Um.
				pCombo->CurSel = NULL;
				if (pbAllSelected) {
					*pbAllSelected = false;
				}
			} else if(pbAllSelected) {
				*pbAllSelected = true;
			}
		}
	}
	EnsureControls();
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handles the WM_SETCURSOR message. Sets the mouse cursor to a hand when hovering over multi-select label hyperlinks.</summary>
/// <param name="pWnd">Specifies a pointer to the window that contains the cursor. The pointer may be temporary and should not be used for later use.</param>
/// <param name="nHitTest">Specifies the hit-test area code. The hit test determines the cursor's location.</param>
/// <param name="message">Specifies the mouse message number.</param>
/// <returns>Nonzero to halt further processing, or 0 to continue.</return>
BOOL CConfigureChargeLevelProviderDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiServiceCodes.IsWindowVisible() && m_nxlMultiServiceCodes.IsWindowEnabled()) {
			m_nxlMultiServiceCodes.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiProducts.IsWindowVisible() && m_nxlMultiProducts.IsWindowEnabled()) {
			m_nxlMultiProducts.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiProviders.IsWindowVisible() && m_nxlMultiProviders.IsWindowEnabled()) {
			m_nxlMultiProviders.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiLocations.IsWindowVisible() && m_nxlMultiLocations.IsWindowEnabled()) {
			m_nxlMultiLocations.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiHCFAGroups.IsWindowVisible() && m_nxlMultiHCFAGroups.IsWindowEnabled()) {
			m_nxlMultiHCFAGroups.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAllCallIgnore({
		if (m_bNotifyOnce) {
			m_bNotifyOnce = false;
			try { throw; }NxCatchAll(__FUNCTION__);
		}
	});
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handles when the user left clicks a CNxLabel control.</summary>
/// <param name="wParam">The nFlags parameter of the WM_LBUTTONDOWN message.</param>
/// <param name="lParam">IDC number of the CNxLabel control.</param>
/// <returns>Nonzero to halt further processing, or 0 to continue.</return>
LRESULT CConfigureChargeLevelProviderDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		case IDC_CCLP_MULTI_SERVICE_CODES_STATIC:
			SelectMultiServiceCodes();
			break;
		case IDC_CCLP_MULTI_PRODUCTS_STATIC:
			SelectMultiProducts();
			break;
		case IDC_CCLP_MULTI_PROVIDERS_STATIC:
			SelectMultiProviders();
			break;
		case IDC_CCLP_MULTI_LOCATIONS_STATIC:
			SelectMultiLocations();
			break;
		case IDC_CCLP_MULTI_HCFA_GROUPS_STATIC:
			SelectMultiHCFAGroups();
			break;

		default:
			//Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Shows the Multiple Selection Dialog for the Providers combo.</summary>
void CConfigureChargeLevelProviderDlg::SelectMultiProviders()
{
	// If < Multiple Providers > is selected, present a pop up window with a list of all active Providers. The current provider 
	// will be selected by default, and display at the top.The rest of the list should display in alphabetical order.
	// There is no such thing as a current provider in the place this dialog is called from.
	HandleMultipleSelection(m_nxlMultiProviders, m_pProviderCombo, IDC_CCLP_PROVIDER_COMBO, m_arySelectedProviders, &m_bAllProvidersSelected,
		"ProvidersT", "Select one or more Providers:",
		(short)EProviderComboColumns::ID, (short)EProviderComboColumns::Name);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Shows the Multiple Selection Dialog for the Locations combo.</summary>
void CConfigureChargeLevelProviderDlg::SelectMultiLocations()
{
	// If < Multiple Locations > is selected, present a pop up window with a list of all managed locations. The current logged-in location 
	// will be selected by default, and display at the top. The rest of the list should display in alphabetical order.
	HandleMultipleSelection(m_nxlMultiLocations, m_pLocationCombo, IDC_CCLP_LOCATION_COMBO, m_arySelectedLocations, &m_bAllLocationsSelected,
		"LocationsT", "Select one or more Locations:",
		(short)ELocationComboColumns::ID, (short)ELocationComboColumns::Name);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Shows the Multiple Selection Dialog for the HCFA Groups combo.</summary>
void CConfigureChargeLevelProviderDlg::SelectMultiHCFAGroups()
{
	// If < Multiple HCFA Groups > is selected, present a pop up window with a list of all HCFA Groups. Nothing will be checked by default. 
	// The rest of the list should display in alphabetical order.
	HandleMultipleSelection(m_nxlMultiHCFAGroups, m_pHCFAGroupCombo, IDC_CCLP_HCFA_GROUP_COMBO, m_arySelectedHCFAGroups, &m_bAllHCFAGroupsSelected,
		"HCFASetupT", "Select one or more HCFA Setup Groups:",
		(short)EHCFAGroupComboColumns::ID, (short)EHCFAGroupComboColumns::Name);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Shows the Multiple Selection Dialog for the Service Codes combo.</summary>
void CConfigureChargeLevelProviderDlg::SelectMultiServiceCodes()
{
	// If < Multiple Service Codes > is selected, present a pop up window with a list of all Service Codes. There should be three columns – Checkbox, 
	// Code, and Description. Nothing will be checked by default. The default sort should be by code.
	CArray<short, short> aryExtraColumnIndices; 
	aryExtraColumnIndices.Add((short)EServiceCodeComboColumns::Description);
	HandleMultipleSelection(m_nxlMultiServiceCodes, m_pServiceCodeCombo, IDC_CCLP_SERVICE_CODE_COMBO, m_arySelectedServiceCodes, NULL,
		"CPTCodeT", "Select one or more Service Codes:",
		(short)EServiceCodeComboColumns::ID, (short)EServiceCodeComboColumns::Code, &aryExtraColumnIndices);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Shows the Multiple Selection Dialog for the Products combo.</summary>
void CConfigureChargeLevelProviderDlg::SelectMultiProducts()
{
	// If < Multiple Products > is selected, present a pop up window with a list of all Products. There should be three columns – Checkbox, 
	// Code, and Description. Nothing will be checked by default. The rest of the list should display in alphabetical order.
	HandleMultipleSelection(m_nxlMultiProducts, m_pProductCombo, IDC_CCLP_PRODUCT_COMBO, m_arySelectedServiceCodes, NULL,
		"ProductT", "Select one or more Products:",
		(short)EProductComboColumns::ID, (short)EProductComboColumns::Name);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the user selects a row from the Providers combo.</summary>
/// <param name="lpRow">A pointer to the row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpProviderCombo(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		long nSelId = VarLong(pRow->GetValue((short)EProviderComboColumns::ID));

		if (nSelId == (long)ECriteriaSpecialIds::Multiple) {
			SelectMultiProviders();
			return;
		} else {
			if (nSelId == (long)ECriteriaSpecialIds::All) {
				// The all row's ids are implicitly known.
				m_arySelectedProviders.RemoveAll();
				m_bAllProvidersSelected = true;
			} else {
				m_arySelectedProviders.RemoveAll();
				m_arySelectedProviders.Add(nSelId);
				m_bAllProvidersSelected = false;
			}

			// If there was some previously saved settings for this particular combination of criteria, then load it.
			ReloadSettings();
			// Ensure our controls
			EnsureControls();
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the user selects a row from the Locations combo.</summary>
/// <param name="lpRow">A pointer to the row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpLocationCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		long nSelId = VarLong(pRow->GetValue((short)ELocationComboColumns::ID));

		if (nSelId == (long)ECriteriaSpecialIds::Multiple) {
			SelectMultiLocations();
		} else {
			if (nSelId == (long)ECriteriaSpecialIds::All) {
				// The all row's ids are implicitly known.
				m_arySelectedLocations.RemoveAll();
				m_bAllLocationsSelected = true;
			} else {
				m_arySelectedLocations.RemoveAll();
				m_arySelectedLocations.Add(nSelId);
				m_bAllLocationsSelected = false;
			}

			// If there was some previously saved settings for this particular combination of criteria, then load it.
			ReloadSettings();
			// Ensure our controls
			EnsureControls();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the user selects a row from the HCFA Groups combo.</summary>
/// <param name="lpRow">A pointer to the row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpHcfaGroupCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		long nSelId = VarLong(pRow->GetValue((short)EHCFAGroupComboColumns::ID), (long)ECriteriaSpecialIds::All);

		if (nSelId == (long)ECriteriaSpecialIds::Multiple) {
			SelectMultiHCFAGroups();
		} else {
			if (nSelId == (long)ECriteriaSpecialIds::All) {
				// The all row's ids are implicitly known.
				m_arySelectedHCFAGroups.RemoveAll();
				m_bAllHCFAGroupsSelected = true;
			} else {
				m_arySelectedHCFAGroups.RemoveAll();
				m_arySelectedHCFAGroups.Add(nSelId);
				m_bAllHCFAGroupsSelected = false;
			}

			// If there was some previously saved settings for this particular combination of criteria, then load it.
			ReloadSettings();
			// Ensure our controls
			EnsureControls();
		}

		
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the user selects a row from the Service Codes combo.</summary>
/// <param name="lpRow">A pointer to the row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpServiceCodeCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		long nSelId = VarLong(pRow->GetValue((short)EServiceCodeComboColumns::ID));

		if (nSelId == (long)ECriteriaSpecialIds::Multiple) {
			SelectMultiServiceCodes();
		} else {
			m_arySelectedServiceCodes.RemoveAll();
			m_arySelectedServiceCodes.Add(nSelId);

			// If there was some previously saved settings for this particular combination of criteria, then load it.
			ReloadSettings();
			// Ensure our controls
			EnsureControls();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the user selects a row from the Products combo.</summary>
/// <param name="lpRow">A pointer to the row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpProductCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		long nSelId = VarLong(pRow->GetValue((short)EProductComboColumns::ID));

		if (nSelId == (long)ECriteriaSpecialIds::Multiple) {
			SelectMultiProducts();
		} else {
			m_arySelectedServiceCodes.RemoveAll();
			m_arySelectedServiceCodes.Add(nSelId);

			// If there was some previously saved settings for this particular combination of criteria, then load it.
			ReloadSettings();
			// Ensure our controls
			EnsureControls();
		}	
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Returns the SQL to be used in all of the Provider settings combos' FROM clauses.</summary>
/// <returns>A string of a SQL fragment to be put into a combo box's FROM clause.</returns>
CString CConfigureChargeLevelProviderDlg::GetLowerProviderComboFromSql()
{
	// The following is a combination of all providers and physicians, plus what they are.
	return
		"( \r\n"
		"	SELECT PersonT.ID, PersonT.FullName, PersonT.Archived, ProvidersT.ReferringProvider, ProvidersT.OrderingProvider, ProvidersT.SupervisingProvider \r\n"
		"	FROM ProvidersT \r\n"
		"	INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID \r\n"
		"	UNION ALL \r\n"
		"	SELECT PersonT.ID, PersonT.FullName, PersonT.Archived, ReferringPhysT.ReferringProvider, ReferringPhysT.OrderingProvider, ReferringPhysT.SupervisingProvider \r\n"
		"	FROM ReferringPhysT \r\n"
		"	INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID \r\n"
		") AllProvidersQ";
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Returns the SQL to be used in the Referring Provider setting combo's WHERE clauses.</summary>
/// <returns>A string of a SQL fragment to be put into a combo box's WHERE clause.</returns>
CString CConfigureChargeLevelProviderDlg::GetReferringProviderComboWhereSql()
{
	return "AllProvidersQ.Archived = 0 AND AllProvidersQ.ReferringProvider = 1";
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Returns the SQL to be used in the Ordering Provider setting combo's WHERE clauses.</summary>
/// <returns>A string of a SQL fragment to be put into a combo box's WHERE clause.</returns>
CString CConfigureChargeLevelProviderDlg::GetOrderingProviderComboWhereSql()
{
	return "AllProvidersQ.Archived = 0 AND AllProvidersQ.OrderingProvider = 1";
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Returns the SQL to be used in the Supervising Provider setting combo's WHERE clauses.</summary>
/// <returns>A string of a SQL fragment to be put into a combo box's WHERE clause.</returns>
CString CConfigureChargeLevelProviderDlg::GetSupervisingProviderComboWhereSql()
{
	return "AllProvidersQ.Archived = 0 AND AllProvidersQ.SupervisingProvider = 1";
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the Provider combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpProviderCombo(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pProviderCombo->GetNewRow();
		pNewRow->PutValue((short)EProviderComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
		pNewRow->PutValue((short)EProviderComboColumns::Name, _bstr_t(" < Multiple Providers >"));
		m_pProviderCombo->AddRowBefore(pNewRow, m_pProviderCombo->GetFirstRow());
		
		pNewRow = m_pProviderCombo->GetNewRow();
		pNewRow->PutValue((short)EProviderComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));
		pNewRow->PutValue((short)EProviderComboColumns::Name, _bstr_t(" < All Providers >"));
		m_pProviderCombo->AddRowBefore(pNewRow, m_pProviderCombo->GetFirstRow());

		m_pProviderCombo->FindByColumn((short)EProviderComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), m_pProviderCombo->GetFirstRow(), VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the Location combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpLocationCombo(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pLocationCombo->GetNewRow();
		pNewRow->PutValue((short)ELocationComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
		pNewRow->PutValue((short)ELocationComboColumns::Name, _bstr_t(" < Multiple Locations >"));
		m_pLocationCombo->AddRowBefore(pNewRow, m_pLocationCombo->GetFirstRow());
		
		pNewRow = m_pLocationCombo->GetNewRow();
		pNewRow->PutValue((short)ELocationComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));
		pNewRow->PutValue((short)ELocationComboColumns::Name, _bstr_t(" < All Locations >"));
		m_pLocationCombo->AddRowBefore(pNewRow, m_pLocationCombo->GetFirstRow());

		m_pLocationCombo->FindByColumn((short)ELocationComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), m_pLocationCombo->GetFirstRow(), VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the HCFA Group combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpHcfaGroupCombo(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pHCFAGroupCombo->GetNewRow();
		pNewRow->PutValue((short)EHCFAGroupComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
		pNewRow->PutValue((short)EHCFAGroupComboColumns::Name, _bstr_t(" < Multiple HCFA Groups >"));
		m_pHCFAGroupCombo->AddRowBefore(pNewRow, m_pHCFAGroupCombo->GetFirstRow());
		
		pNewRow = m_pHCFAGroupCombo->GetNewRow();
		pNewRow->PutValue((short)EHCFAGroupComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));
		pNewRow->PutValue((short)EHCFAGroupComboColumns::Name, _bstr_t(" < All HCFA Groups >"));
		m_pHCFAGroupCombo->AddRowBefore(pNewRow, m_pHCFAGroupCombo->GetFirstRow());

		m_pHCFAGroupCombo->FindByColumn((short)EHCFAGroupComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), m_pHCFAGroupCombo->GetFirstRow(), VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the Service Code combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpServiceCodeCombo(short nFlags)
{
	try {
		// This one only has the Multiple row. It does not have an All row. Just following the breakdown....
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pServiceCodeCombo->GetNewRow();
		pNewRow->PutValue((short)EServiceCodeComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
		pNewRow->PutValue((short)EServiceCodeComboColumns::Description, _bstr_t(" < Multiple Service Codes >"));
		pNewRow->PutValue((short)EServiceCodeComboColumns::DisplayColumn, _bstr_t(" < Multiple Service Codes >"));
		m_pServiceCodeCombo->AddRowBefore(pNewRow, m_pServiceCodeCombo->GetFirstRow());

		// We may have an initial service code to select.
		if (m_arySelectedServiceCodes.GetSize() > 0) {
			long nInitialServiceID = m_arySelectedServiceCodes.GetAt(0);
			if (nInitialServiceID != -1) {
				NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_pServiceCodeCombo->FindByColumn((short)EServiceCodeComboColumns::ID, _variant_t(nInitialServiceID, VT_I4), m_pServiceCodeCombo->GetFirstRow(), VARIANT_TRUE);
				if (!pSelectedRow) {
					ThrowNxException("%s : Could not select the initially selected service.", __FUNCTION__);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62248 - Added.
/// <summary>Handler for when the Product combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpProductCombo(short nFlags)
{
	try {
		// This one only has the Multiple row. It does not have an All row. Just following the breakdown....
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pProductCombo->GetNewRow();
		pNewRow->PutValue((short)EProductComboColumns::ID, _variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
		pNewRow->PutValue((short)EProductComboColumns::Name, _bstr_t(" < Multiple Products >"));
		m_pProductCombo->AddRowBefore(pNewRow, m_pProductCombo->GetFirstRow());

		// We may have an initial product to select.
		if (m_arySelectedServiceCodes.GetSize() > 0) {
			long nInitialServiceID = m_arySelectedServiceCodes.GetAt(0);
			if (nInitialServiceID != -1) {
				NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_pProductCombo->FindByColumn((short)EProductComboColumns::ID, _variant_t(nInitialServiceID, VT_I4), m_pProductCombo->GetFirstRow(), VARIANT_TRUE);
				if (!pSelectedRow) {
					ThrowNxException("%s : Could not select the initially selected service.", __FUNCTION__);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Adds special ID rows to a provider setting combo box and selects one as the default.</summary>
/// <param name="pCombo">The datalist to add the special ID rows to.</param>
void CConfigureChargeLevelProviderDlg::AddSpecialColummnsToProviderCombos(NXDATALIST2Lib::_DNxDataListPtr pCombo)
{
	// All of these provider settings combos have the same special rows. None, Claim, and Bill.
	// Notice that we negate the value of the option so that it won't conflict with the real record IDs.
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = pCombo->GetNewRow();
	pNewRow->PutValue((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::ClaimProvider, VT_I4));
	pNewRow->PutValue((short)ECommonProviderComboColumns::Name, _bstr_t(" < Claim Provider >"));
	pCombo->AddRowBefore(pNewRow, pCombo->GetFirstRow());

	pNewRow = pCombo->GetNewRow();
	pNewRow->PutValue((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::ChargeProvider, VT_I4));
	pNewRow->PutValue((short)ECommonProviderComboColumns::Name, _bstr_t(" < Charge Provider >"));
	pCombo->AddRowBefore(pNewRow, pCombo->GetFirstRow());

	pNewRow = pCombo->GetNewRow();
	pNewRow->PutValue((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::NoSelection, VT_I4));
	pNewRow->PutValue((short)ECommonProviderComboColumns::Name, _bstr_t(" < No Default Provider >"));
	pCombo->AddRowBefore(pNewRow, pCombo->GetFirstRow());

	pCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)ChargeLevelProviderConfigOption::NoSelection), pCombo->GetFirstRow(), VARIANT_TRUE);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the Referring Provider combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpReferringProviderCombo(short nFlags)
{
	try {
		AddSpecialColummnsToProviderCombos(m_pReferringProviderCombo);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the Ordering Provider combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpOrderingProviderCombo(short nFlags)
{
	try {
		AddSpecialColummnsToProviderCombos(m_pOrderingProviderCombo);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the Supervising Provider combo finishes requerying. Adds the special ID rows and selects a row.</summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CConfigureChargeLevelProviderDlg::RequeryFinishedCclpSupervisingProviderCombo(short nFlags)
{
	try {
		AddSpecialColummnsToProviderCombos(m_pSupervisingProviderCombo);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Gets the IDs of the upper criteria combo boxes.</summary>
/// <param name="nProviderID">A reference to a long that will receive the ID of the provider record selected. May be -1 if no row is selected.</param>
/// <param name="nLocationID">A reference to a long that will receive the ID of the location record selected. May be -1 if no row is selected.</param>
/// <param name="nHCFAGroupID">A reference to a long that will receive the ID of the HCFA group record selected. May be -1 if no row is selected.</param>
/// <param name="nServiceCodeID">A reference to a long that will receive the ID of the service code/product record selected. May be -1 if no row is selected.</param>
void CConfigureChargeLevelProviderDlg::GetCriteriaSelections(long &nProviderID, long &nLocationID, long &nHCFAGroupID, long &nServiceCodeID)
{
	nProviderID = (m_pProviderCombo->CurSel != NULL ? VarLong(m_pProviderCombo->CurSel->GetValue((short)EProviderComboColumns::ID), (long)ECriteriaSpecialIds::None) : (long)ECriteriaSpecialIds::None);
	nLocationID = (m_pLocationCombo->CurSel != NULL ? VarLong(m_pLocationCombo->CurSel->GetValue((short)ELocationComboColumns::ID), (long)ECriteriaSpecialIds::None) : (long)ECriteriaSpecialIds::None);
	nHCFAGroupID = (m_pHCFAGroupCombo->CurSel != NULL ? VarLong(m_pHCFAGroupCombo->CurSel->GetValue((short)EHCFAGroupComboColumns::ID), (long)ECriteriaSpecialIds::None) : (long)ECriteriaSpecialIds::None);
	nServiceCodeID = (long)ECriteriaSpecialIds::None;
	// Where we draw the Service ID from depends on what we are showing, CPT codes or products.
	if (m_eServiceType == EServiceType::CPTCode) {
		nServiceCodeID = (m_pServiceCodeCombo->CurSel != NULL ? VarLong(m_pServiceCodeCombo->CurSel->GetValue((short)EServiceCodeComboColumns::ID), (long)ECriteriaSpecialIds::None) : (long)ECriteriaSpecialIds::None);
	} else if (m_eServiceType == EServiceType::Product) {
		nServiceCodeID = (m_pProductCombo->CurSel != NULL ? VarLong(m_pProductCombo->CurSel->GetValue((short)EProductComboColumns::ID), (long)ECriteriaSpecialIds::None) : (long)ECriteriaSpecialIds::None);
	} else {
		ThrowNxException("%s : Invalid service type.", __FUNCTION__);
	}
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Gets the IDs of the lower provider settings combo boxes. Throws an exception if no row is selected for one of the settings combos.</summary>
/// <param name="nReferringProviderID">A reference to a long that will receive the ID of the person record selected.</param>
/// <param name="nOrderingProviderID">A reference to a long that will receive the ID of the person record selected.</param>
/// <param name="nSupervisingProviderID">A reference to a long that will receive the ID of the person record selected.</param>
void CConfigureChargeLevelProviderDlg::GetSettingSelections(long &nReferringProviderID, long &nOrderingProviderID, long &nSupervisingProviderID)
{
	// Referring Provider
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReferringProviderCombo->CurSel;
	if (!pRow) {
		ThrowNxException("%s : The current selection for the Referring Provider combo is NULL.", __FUNCTION__);
	}
	nReferringProviderID = VarLong(pRow->GetValue((short)ECommonProviderComboColumns::ID));

	// Ordering Provider
	pRow = m_pOrderingProviderCombo->CurSel;
	if (!pRow) {
		ThrowNxException("%s : The current selection for the Ordering Provider combo is NULL.", __FUNCTION__);
	}
	nOrderingProviderID = VarLong(pRow->GetValue((short)ECommonProviderComboColumns::ID));

	// Supervising Provider
	pRow = m_pSupervisingProviderCombo->CurSel;
	if (!pRow) {
		ThrowNxException("%s : The current selection for the Supervising Provider combo is NULL.", __FUNCTION__);
	}
	nSupervisingProviderID = VarLong(pRow->GetValue((short)ECommonProviderComboColumns::ID));
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Fills the controls with the saved settings of a particular criteria configuration, if one exists.</summary>
void CConfigureChargeLevelProviderDlg::ReloadSettings()
{
	// Load up the existing settings, if there were any. If there weren't then we'll get back the default set of settings.
	ChargeClaimProviderSettings settings = GetSettingsFromData();

	// Referring Provider
	if (settings.nReferringProviderID != -1) {
		// We have an actual provider/referring physician, try to select it.
		m_pReferringProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(settings.nReferringProviderID, VT_I4), m_pReferringProviderCombo->GetFirstRow(), VARIANT_TRUE);
	} else {
		// There is no real provider/referring physician. We may have no selection or we may have one of the special provider options, Claim or Charge. Anyway, those are special options and
		// are stored in the combo box as their negations.
		m_pReferringProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)settings.eReferringProviderOption, VT_I4), m_pReferringProviderCombo->GetFirstRow(), VARIANT_TRUE);
	}
	if (settings.eReferringProviderOption != ChargeLevelProviderConfigOption::NoSelection) {
		// Everything but the not required < No Default Provider > row makes the provider Required.
		m_checkReferringProvider.SetCheck(BST_CHECKED);
	} else {
		// When there is < No Default Provider > and it is not required, uncheck the required box.
		m_checkReferringProvider.SetCheck(BST_UNCHECKED);
	}

	// Ordering Provider
	if (settings.nOrderingProviderID != -1) {
		// We have an actual provider/referring physician, try to select it.
		m_pOrderingProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(settings.nOrderingProviderID, VT_I4), m_pOrderingProviderCombo->GetFirstRow(), VARIANT_TRUE);
	} else {
		// There is no real provider/referring physician. We may have no selection or we may have one of the special provider options, Claim or Charge. Anyway, those are special options and
		// are stored in the combo box as their negations.
		m_pOrderingProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)settings.eOrderingProviderOption, VT_I4), m_pOrderingProviderCombo->GetFirstRow(), VARIANT_TRUE);
	}
	if (settings.eOrderingProviderOption != ChargeLevelProviderConfigOption::NoSelection) {
		// Everything but the not required < No Default Provider > row makes the provider Required.
		m_checkOrderingProvider.SetCheck(BST_CHECKED);
	} else {
		// When there is < No Default Provider > and it is not required, uncheck the required box.
		m_checkOrderingProvider.SetCheck(BST_UNCHECKED);
	}

	// Supervising Provider.
	if (settings.nSupervisingProviderID != -1) {
		// We have an actual provider/referring physician, try to select it.
		m_pSupervisingProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(settings.nSupervisingProviderID, VT_I4), m_pSupervisingProviderCombo->GetFirstRow(), VARIANT_TRUE);
	} else {
		// There is no real provider/referring physician. We may have no selection or we may have one of the special provider options, Claim or Charge. Anyway, those are special options and
		// are stored in the combo box as their negations.
		m_pSupervisingProviderCombo->FindByColumn((short)ECommonProviderComboColumns::ID, _variant_t(-(long)settings.eSupervisingProviderOption, VT_I4), m_pSupervisingProviderCombo->GetFirstRow(), VARIANT_TRUE);
	}
	if (settings.eSupervisingProviderOption != ChargeLevelProviderConfigOption::NoSelection) {
		// Everything but the not required < No Default Provider > row makes the provider Required.
		m_checkSupervisingProvider.SetCheck(BST_CHECKED);
	} else {
		// When there is < No Default Provider > and it is not required, uncheck the required box.
		m_checkSupervisingProvider.SetCheck(BST_UNCHECKED);
	}

	// Enable/disable the settings controls according to the new settings.
	EnsureControls();
}

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Returns the charge level claim provider settings saved in the database for the currently selected criteria.</summary>
/// <returns>Charge level claim provider settings for the currently selected criteria.</return>
ChargeClaimProviderSettings CConfigureChargeLevelProviderDlg::GetSettingsFromData()
{
	ChargeClaimProviderSettings settings;

	// Grab the criteria combo box selections.
	long nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID;
	GetCriteriaSelections(nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID);
	
	if (nProviderID == (long)ECriteriaSpecialIds::None || nLocationID == (long)ECriteriaSpecialIds::None || nHCFAGroupID == (long)ECriteriaSpecialIds::None || nServiceCodeID == (long)ECriteriaSpecialIds::None) {
		// One of the combo boxes is null. We can't have that...
		return settings;
	}

	if (nProviderID == (long)ECriteriaSpecialIds::Multiple || nLocationID == (long)ECriteriaSpecialIds::Multiple || nHCFAGroupID == (long)ECriteriaSpecialIds::Multiple || nServiceCodeID == (long)ECriteriaSpecialIds::Multiple) {
		// Multiple are no good either. 
		return settings;
	}

	if (nProviderID == (long)ECriteriaSpecialIds::All || nLocationID == (long)ECriteriaSpecialIds::All || nHCFAGroupID == (long)ECriteriaSpecialIds::All) {
		// If any combo box is all, then return the default settings
		return settings;
	}

	// Get the settings from the database.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT TOP 1 \r\n"
		"	ChargeLevelProviderConfigT.ReferringProviderOption, ChargeLevelProviderConfigT.ReferringProviderID, \r\n"
		"	ChargeLevelProviderConfigT.OrderingProviderOption, ChargeLevelProviderConfigT.OrderingProviderID, \r\n"
		"	ChargeLevelProviderConfigT.SupervisingProviderOption, ChargeLevelProviderConfigT.SupervisingProviderID \r\n"
		"FROM ChargeLevelProviderConfigT \r\n"
		"WHERE ChargeLevelProviderConfigT.ProviderID = {INT} \r\n"
		"	AND ChargeLevelProviderConfigT.LocationID = {INT} \r\n"
		"	AND ChargeLevelProviderConfigT.HCFAGroupID = {INT} \r\n"
		"	AND ChargeLevelProviderConfigT.ServiceID = {INT}; "
		, nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID);
	if (!prs->eof) {
		settings.eReferringProviderOption = (ChargeLevelProviderConfigOption)AdoFldLong(prs->Fields, "ReferringProviderOption");
		settings.nReferringProviderID = AdoFldLong(prs->Fields, "ReferringProviderID", -1);
		settings.eOrderingProviderOption = (ChargeLevelProviderConfigOption)AdoFldLong(prs->Fields, "OrderingProviderOption");
		settings.nOrderingProviderID = AdoFldLong(prs->Fields, "OrderingProviderID", -1);
		settings.eSupervisingProviderOption = (ChargeLevelProviderConfigOption)AdoFldLong(prs->Fields, "SupervisingProviderOption");
		settings.nSupervisingProviderID = AdoFldLong(prs->Fields, "SupervisingProviderID", -1);
		return settings;
	}
	prs->Close();

	return settings;
}

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Handler for when the user clicks the Apply button. Saves the selected provider settings for the selected criteria.</summary>
void CConfigureChargeLevelProviderDlg::OnBnClickedCclpApplyBtn()
{
	try {
		Save();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Saves the selected provider settings for the selected criteria.</summary>
void CConfigureChargeLevelProviderDlg::Save()
{
	// Grab the selected criteria.
	long nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID;
	GetCriteriaSelections(nProviderID, nLocationID, nHCFAGroupID, nServiceCodeID);

	// Honestly, the following checks should never occur because we prevent the user from selecting NULL. But just in case.
	if (nProviderID == (long)ECriteriaSpecialIds::None) {
		MessageBox("Please select a Provider before applying.");
	}
	if (nLocationID == (long)ECriteriaSpecialIds::None) {
		MessageBox("Please select a Location before applying.");
	}
	if (nHCFAGroupID == (long)ECriteriaSpecialIds::None) {
		MessageBox("Please select a HCFA Setup Group before applying.");
	}
	if (nServiceCodeID == (long)ECriteriaSpecialIds::None) {
		MessageBox(FormatString("Please select a %s before applying.", m_eServiceType == EServiceType::Product ? "Product" : "Service Code"));
	}

	// Prompt the user to make sure they want to apply the changes as it will overwrite the current settings
	if (IDYES != MessageBox("Your changes will overwrite the existing settings. Do you want to continue?", NULL, MB_ICONQUESTION | MB_YESNO)) {
		return;
	}

	// <All> means it will save a record for each provider, or location, or HCFA group. We show a don't show me again warning the first
	// time they apply to <All> to make it clear that if they add a new provider, or location, or group, that the changes won't carry over.
	if (nProviderID == (long)ECriteriaSpecialIds::All || nLocationID == (long)ECriteriaSpecialIds::All || nHCFAGroupID == (long)ECriteriaSpecialIds::All) {
		DontShowMeAgain(this, "You have selected < All > for one or more of the criteria drop-down lists. Please be aware that this only applies to all existing records of that type. "
			"If more records of that type are subsequently added to Nextech, then they will need configured here as well.", "ConfigureChargeLevelProviderDlg_AllWarning", "Warning", FALSE);
	}

	// Generate some dynamic SQL to clear out the existing records that this criteria matches.
	// Also construct the join clauses here for the insert statement.
	CParamSqlBatch sqlSaveBatch;
	CArray<CSqlFragment> aryDeleteConditions;
	CSqlFragment sqlProvidersJoinWhereCondition("WHERE PersonT.Archived = 0");
	if (nProviderID != (long)ECriteriaSpecialIds::All) {
		aryDeleteConditions.Add(CSqlFragment("ChargeLevelProviderConfigT.ProviderID IN ({INTARRAY})", m_arySelectedProviders));
		sqlProvidersJoinWhereCondition += CSqlFragment(" AND PersonT.ID IN ({INTARRAY})", m_arySelectedProviders);
	}

	CSqlFragment sqlLocationsJoinWhereCondition("WHERE LocationsT.Managed = 1 AND LocationsT.TypeID = 1 AND LocationsT.Active = 1");
	if (nLocationID != (long)ECriteriaSpecialIds::All) {
		aryDeleteConditions.Add(CSqlFragment("ChargeLevelProviderConfigT.LocationID IN ({INTARRAY})", m_arySelectedLocations));
		sqlLocationsJoinWhereCondition += CSqlFragment(" AND LocationsT.ID IN ({INTARRAY})", m_arySelectedLocations);
	}

	CSqlFragment sqlHCFAGroupsJoinWhereCondition;
	if (nHCFAGroupID != (long)ECriteriaSpecialIds::All) {
		aryDeleteConditions.Add(CSqlFragment("ChargeLevelProviderConfigT.HCFAGroupID IN ({INTARRAY})", m_arySelectedHCFAGroups));
		sqlHCFAGroupsJoinWhereCondition += CSqlFragment("WHERE HCFASetupT.ID IN ({INTARRAY})", m_arySelectedHCFAGroups);
	}

	CSqlFragment sqlServiceCodesJoinWhereCondition("WHERE ServiceT.Active = 1");
	aryDeleteConditions.Add(CSqlFragment("ChargeLevelProviderConfigT.ServiceID IN ({INTARRAY})", m_arySelectedServiceCodes));
	sqlServiceCodesJoinWhereCondition += CSqlFragment(" AND ServiceT.ID IN ({INTARRAY})", m_arySelectedServiceCodes);

	// Put the delete statement together from the parts we just made.
	CSqlFragment sqlDeleteExisting("DELETE FROM ChargeLevelProviderConfigT");
	if (aryDeleteConditions.GetSize() > 0) {
		sqlDeleteExisting += CSqlFragment(" WHERE ");
	}
	for (int i = 0; i < aryDeleteConditions.GetSize(); i++) {
		if (i > 0) {
			sqlDeleteExisting += CSqlFragment(" AND ");
		}
		sqlDeleteExisting += aryDeleteConditions.GetAt(i);
	}
	sqlDeleteExisting += CSqlFragment(";");
	sqlSaveBatch.Add(sqlDeleteExisting);

	// Oh yeah, grab the currently selected settings so we can save them to the database.
	ChargeClaimProviderSettings settings = GetSettingsFromUI();

	// We need every combination, which means cross joining.
	sqlSaveBatch.Add(
		"INSERT INTO ChargeLevelProviderConfigT (ProviderID, LocationID, HCFAGroupID, ServiceID, ReferringProviderOption, ReferringProviderID, OrderingProviderOption, OrderingProviderID, SupervisingProviderOption, SupervisingProviderID) \r\n"
		"SELECT ProvidersQ.ID, LocationsQ.ID, HCFAGroupsQ.ID, ServiceQ.ID, {INT}, {VT_I4}, {INT}, {VT_I4}, {INT}, {VT_I4} \r\n"
		"FROM (SELECT PersonT.ID FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = Providerst.PersonID {SQL}) ProvidersQ \r\n"
		"CROSS JOIN (SELECT LocationsT.ID FROM LocationsT {SQL}) LocationsQ \r\n"
		"CROSS JOIN (SELECT HCFASetupT.ID FROM HCFASetupT {SQL}) HCFAGroupsQ \r\n"
		"CROSS JOIN (SELECT ServiceT.ID FROM ServiceT {SQL}) ServiceQ;"
		, (long)settings.eReferringProviderOption, (settings.nReferringProviderID != -1 ? _variant_t(settings.nReferringProviderID, VT_I4) : g_cvarNull)
		, (long)settings.eOrderingProviderOption, (settings.nOrderingProviderID != -1 ? _variant_t(settings.nOrderingProviderID, VT_I4) : g_cvarNull)
		, (long)settings.eSupervisingProviderOption, (settings.nSupervisingProviderID != -1 ? _variant_t(settings.nSupervisingProviderID, VT_I4) : g_cvarNull)
		, sqlProvidersJoinWhereCondition
		, sqlLocationsJoinWhereCondition
		, sqlHCFAGroupsJoinWhereCondition
		, sqlServiceCodesJoinWhereCondition
		);
	// This could be a lot of records affected
	NxAdo::PushMaxRecordsWarningLimit pmr(10000);
	// Use an infinite timeout since this can take a long time if there are many records.
	CIncreaseCommandTimeout ict(0);
	CWaitCursor waitCursor;
	sqlSaveBatch.Execute(GetRemoteData());

	// We need to inform others of our misdeeds.
	CClient::RefreshTable(NetUtils::ChargeLevelProviderConfigT);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Gets the charge level claim provider settings represented by the control values of the settinsg combos and required checkboxes.</summary>
/// <returns>Charge level claim provider settings representation of the settings combo boxes and required checkboxes.</return>
ChargeClaimProviderSettings CConfigureChargeLevelProviderDlg::GetSettingsFromUI()
{
	long nReferringProviderID, nOrderingProviderID, nSupervisingProviderID;
	GetSettingSelections(nReferringProviderID, nOrderingProviderID, nSupervisingProviderID);

	ChargeClaimProviderSettings settings;

	// Referring Provider
	if (m_checkReferringProvider.GetCheck() == BST_UNCHECKED) {
		// This will always be NoSelection. Can't be anything else.
		settings.eReferringProviderOption = ChargeLevelProviderConfigOption::NoSelection;
	} else {
		// Let's start this off as just Required and then constrain in more maybe.
		settings.eReferringProviderOption = ChargeLevelProviderConfigOption::Required;
	}
	if (nReferringProviderID <= 0) {
		// Its a special option rather than an actual person
		ChargeLevelProviderConfigOption eConstrainingOption = (ChargeLevelProviderConfigOption)(-nReferringProviderID);
		if (!(settings.eReferringProviderOption == ChargeLevelProviderConfigOption::Required && eConstrainingOption == ChargeLevelProviderConfigOption::NoSelection)) {
			// OK. Now we are constrained to Required, Required Claim Provider, or Required Charge Provider.
			settings.eReferringProviderOption = eConstrainingOption;
		} else {
			// Business logic dictates that we allow the state of Required but No Default Provider
		}
	} else {
		// We have an actual person ID
		settings.nReferringProviderID = _variant_t(nReferringProviderID, VT_I4);
	}

	// Ordering Provider
	if (m_checkOrderingProvider.GetCheck() == BST_UNCHECKED) {
		// This will always be NoSelection. Can't be anything else.
		settings.eOrderingProviderOption = ChargeLevelProviderConfigOption::NoSelection;
	} else {
		// Let's start this off as just Required and then constrain in more maybe.
		settings.eOrderingProviderOption = ChargeLevelProviderConfigOption::Required;
	}
	if (nOrderingProviderID <= 0) {
		// Its a special option rather than an actual person
		ChargeLevelProviderConfigOption eConstrainingOption = (ChargeLevelProviderConfigOption)(-nOrderingProviderID);
		if (!(settings.eOrderingProviderOption == ChargeLevelProviderConfigOption::Required && eConstrainingOption == ChargeLevelProviderConfigOption::NoSelection)) {
			// OK. Now we are constrained to Required, Required Claim Provider, or Required Charge Provider.
			settings.eOrderingProviderOption = eConstrainingOption;
		} else {
			// Business logic dictates that we allow the state of Required but No Default Provider
		}
	} else {
		// We have an actual person ID
		settings.nOrderingProviderID = _variant_t(nOrderingProviderID, VT_I4);
	}

	// Supervising Provider
	if (m_checkSupervisingProvider.GetCheck() == BST_UNCHECKED) {
		// This will always be NoSelection. Can't be anything else.
		settings.eSupervisingProviderOption = ChargeLevelProviderConfigOption::NoSelection;
	} else {
		// Let's start this off as just Required and then constrain in more maybe.
		settings.eSupervisingProviderOption = ChargeLevelProviderConfigOption::Required;
	}
	if (nSupervisingProviderID <= 0) {
		// Its a special option rather than an actual person
		ChargeLevelProviderConfigOption eConstrainingOption = (ChargeLevelProviderConfigOption)(-nSupervisingProviderID);
		if (!(settings.eSupervisingProviderOption == ChargeLevelProviderConfigOption::Required && eConstrainingOption == ChargeLevelProviderConfigOption::NoSelection)) {
			// OK. Now we are constrained to Required, Required Claim Provider, or Required Charge Provider.
			settings.eSupervisingProviderOption = eConstrainingOption;
		} else {
			// Business logic dictates that we allow the state of Required but No Default Provider
		}
	} else {
		// We have an actual person ID
		settings.nSupervisingProviderID = _variant_t(nSupervisingProviderID, VT_I4);
	}

	return settings;
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the user checks the Referring Provider checkbox. Ensures that settings are valid.</summary>
void CConfigureChargeLevelProviderDlg::OnBnClickedCclpReferringProviderCheck()
{
	try {
		// Make sure we don't have selected an invalid row in the combo box next to us.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the user checks the Ordering Provider checkbox. Ensures that settings are valid.</summary>
void CConfigureChargeLevelProviderDlg::OnBnClickedCclpOrderingProviderCheck()
{
	try {
		// Make sure we don't have selected an invalid row in the combo box next to us.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the user checks the Supervising Provider checkbox. Ensures that settings are valid.</summary>
void CConfigureChargeLevelProviderDlg::OnBnClickedCclpSupervisingProviderCheck()
{
	try {
		// Make sure we don't have selected an invalid row in the combo box next to us.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the user selects a row from the Referring Provider combo box. Ensures that settings are valid.</summary>
/// <param name="lpRow">The combo box row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpReferringProviderCombo(LPDISPATCH lpRow)
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the user selects a row from the Ordering Provider combo box. Ensures that settings are valid.</summary>
/// <param name="lpRow">The combo box row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpOrderingProviderCombo(LPDISPATCH lpRow)
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 62249 - Added.
/// <summary>Handler for when the user selects a row from the Supervising Provider combo box. Ensures that settings are valid.</summary>
/// <param name="lpRow">The combo box row that was selected.</param>
void CConfigureChargeLevelProviderDlg::SelChosenCclpSupervisingProviderCombo(LPDISPATCH lpRow)
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/19/2014) - PLID 61832 - Added.
/// <summary>Callback for messages sent to this dialog.</summary>
/// <param name="message">The message.</param>
/// <param name="wParam">Additional message information.</param>
/// <param name="lParam">Additional message information.</param>
LRESULT CConfigureChargeLevelProviderDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == NXM_ENSURE_CONTROLS) {
		return OnEnsureControls(wParam, lParam);
	}

	return CNxDialog::WindowProc(message, wParam, lParam);
}
