// EmrTableDropdownStampSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrTableDropdownStampSetupDlg.h"


// CEmrTableDropdownStampSetupDlg dialog
// (z.manning 2011-09-28 12:29) - PLID 45729 - Created

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CEmrTableDropdownStampSetupDlg, CNxDialog)

CEmrTableDropdownStampSetupDlg::CEmrTableDropdownStampSetupDlg(CEmrTableDropDownItem *pDrowpdownItem, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrTableDropdownStampSetupDlg::IDD, pParent)
	, m_pDropdownItem(pDrowpdownItem)
	, m_bChangedFilter(FALSE)
	, m_bChangedDefaults(FALSE)
{
	m_arypEMRDropDownList = NULL;
}

CEmrTableDropdownStampSetupDlg::~CEmrTableDropdownStampSetupDlg()
{
}

void CEmrTableDropdownStampSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_TABLE_DROPDOWN_STAMP_SETUP_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEmrTableDropdownStampSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_DROPDOWN_STAMP_SELECT_ALL, &CEmrTableDropdownStampSetupDlg::OnBnClickedDropdownStampSelectAll)
	ON_BN_CLICKED(IDC_DROPDOWN_STAMP_SELECT_NONE, &CEmrTableDropdownStampSetupDlg::OnBnClickedDropdownStampSelectNone)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrTableDropdownStampSetupDlg, CNxDialog)
ON_EVENT(CEmrTableDropdownStampSetupDlg, IDC_EMR_DROPDOWN_STAMP_LIST, 10, CEmrTableDropdownStampSetupDlg::EditingFinishedEmrDropdownStampList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


// CEmrTableDropdownStampSetupDlg message handlers

BOOL CEmrTableDropdownStampSetupDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pdlStamps = BindNxDataList2Ctrl(IDC_EMR_DROPDOWN_STAMP_LIST, false);
		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2012-11-28 09:29) - PLID 53144 - tweaked this text to reflect that using defaults does not require
		// filtering on stamps
		SetDlgItemText(IDC_EMR_DROPDOWN_STAMP_SETUP_LABEL, FormatString(
			"When using this table with a smart stamp image you can configure only certain dropdown elements to display "
			"for a stamp. For any stamp selected here the dropdown item '%s' will display in any table row or column that "
			"gets created by that stamp. Note: If a stamp is not checked in any dropdown items then all dropdown items will "
			"display for that stamp."
			"\r\n\r\n"
			"If you check the default column the dropdown item '%s' will be automatically selected when stamping that "
			"stamp in an image linked to this table. You do not need to use stamp filters in order to use defaults."
			, m_pDropdownItem->strData, m_pDropdownItem->strData));

		GetMainFrame()->LoadEMRImageStamps();
		for(int nStampIndex = 0; nStampIndex < GetMainFrame()->m_aryEMRImageStamps.GetCount(); nStampIndex++)
		{
			// (a.walling 2014-06-30 10:21) - PLID 62497 - use boost::exists with new collection types

			EMRImageStamp *pStamp = GetMainFrame()->m_aryEMRImageStamps.GetAt(nStampIndex);
			bool isFilterStamp = boost::exists(m_pDropdownItem->aryStampFilter, pStamp->nID);
			// (j.jones 2012-11-26 17:39) - PLID 53144 - default stamps are now in a separate array
			bool isDefaultStamp = boost::exists(m_pDropdownItem->aryStampDefaults, pStamp->nID);
			if (!pStamp->bInactive || isFilterStamp || isDefaultStamp)
			{
				IRowSettingsPtr pRow = m_pdlStamps->GetNewRow();
				pRow->PutValue(slcCheck, isFilterStamp ? g_cvarTrue : g_cvarFalse);
				pRow->PutValue(slcID, pStamp->nID);
				pRow->PutValue(slcStamp, _bstr_t(pStamp->strTypeName));
				// (z.manning 2011-10-12 14:46) - PLID 45728 - Added default column
				// (j.jones 2012-11-26 17:39) - PLID 53144 - this now comes from a different array
				pRow->PutValue(slcDefault, isDefaultStamp ? g_cvarTrue : g_cvarFalse);
				m_pdlStamps->AddRowSorted(pRow, NULL);
			}
		}

	}
	NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CEmrTableDropdownStampSetupDlg::OnOK()
{
	try
	{
		// (j.jones 2012-11-26 17:46) - PLID 53144 - default values are no longer direclty tied to the filter,
		// but if they do have a filter in place then they cannot have a default value that is not included
		// in the filter

		// (j.jones 2013-03-07 09:00) - PLID 55500 - reworked this logic to give more accurate warnings

		// (j.jones 2013-03-07 12:37) - PLID 55511 - only show filter warnings if filtering is not disabled
		// (this ConfigRT value is cached in EmrItemEntryDlg, which is the only place from which you can get to this dialog)
		BOOL bFiltersDisabled = (GetRemotePropertyInt("DisableSmartStampTableDropdownStampFilters", 0, 0, "<None>", true) != 0);
		if(!bFiltersDisabled) {
			//first iterate through the list and find out if any stamp is filtered
			BOOL bCurrentDropdownHasFilters = FALSE;
			for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL && !bCurrentDropdownHasFilters; pRow = pRow->GetNextRow())
			{
				BOOL bFiltered = VarBool(pRow->GetValue(slcCheck), FALSE);
				if(bFiltered) {
					//this dropdown has at least one stamp filter enabled
					bCurrentDropdownHasFilters = TRUE;
				}
			}

			for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
			{
				// (j.jones 2013-03-07 09:00) - PLID 55500 - reworked this logic such that all of our tracking
				// is done one stamp at a time, all we care to warn about is if this dropdown item would
				// not show up for the stamp that they want to set a default on
				BOOL bDefault = VarBool(pRow->GetValue(slcDefault), FALSE);
				BOOL bFiltered = VarBool(pRow->GetValue(slcCheck), FALSE);
				if(bDefault && !bFiltered) {
					
					//this stamp has a default, and we're not filtering on it, so first simply check if
					//this dropdown has any filters, if so then we can warn right now

					bool bAlreadyWarned = false;

					long nStampID = VarLong(pRow->GetValue(slcID));
					CString strStampName = VarString(pRow->GetValue(slcStamp), "");

					if(bCurrentDropdownHasFilters) {
						CString strWarn;
						strWarn.Format("The stamp '%s' is selected as a default, but is not selected in your stamp filter. "
							"You must either include this stamp in your filter, or remove the default.\n\n"
							"Are you sure you wish to save this invalid default value?", strStampName);
						if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
							return;
						}
						
						bAlreadyWarned = true;
					}
					
					//next, we need to make sure no other dropdown is filtering on it either, or else it will never show up
					if(!bAlreadyWarned && m_arypEMRDropDownList != NULL) {
						// (a.walling 2014-06-30 10:21) - PLID 62497
						for(const auto& pddiCheck : *m_arypEMRDropDownList) {
							if (bAlreadyWarned) {
								break;
							}
							if(pddiCheck.get() != m_pDropdownItem) {
								if (!bAlreadyWarned && boost::exists(pddiCheck->aryStampFilter, nStampID)) {
									//If we get here, it means:
									//- this stamp is selected as a default
									//- this dropdown entry is not being filtered on this stamp
									//- other dropdown entries are being filtered on this stamp
									//This means that using this stamp will show other dropdown entries, but not this one.
									//Warn the user so they know this default value is for a dropdown entry that will
									//never be shown in the current configuration, unless they go remove the filters
									//from the other dropdowns.
									CString strWarn;
									strWarn.Format("The stamp '%s' is selected as a default, but is not selected in your stamp filter. "
										"However, the stamp is in other dropdown item filters, and will not show up for this dropdown item "
										"unless it is also included in its filter.\n\n"
										"You must either include this stamp in your filter, remove the default value, "
										"or edit other dropdown items to not use this stamp in their filters.\n\n"
										"Are you sure you wish to save this invalid default value?", strStampName);
									if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
										return;
									}

									bAlreadyWarned = true;
								}
							}
						}
					}
				}
			}
		}

		if(m_bChangedFilter || m_bChangedDefaults)
		{
			// (j.jones 2012-11-26 17:39) - PLID 53144 - stamp filters and default stamps are now in separate arrays
			if(m_bChangedFilter) {
				m_pDropdownItem->aryStampFilter.clear();
				m_pDropdownItem->bStampFilterChanged = TRUE;
			}
			if(m_bChangedDefaults) {
				m_pDropdownItem->aryStampDefaults.clear();
				m_pDropdownItem->bStampDefaultsChanged = TRUE;
			}
			for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
			{
				long nStampID = VarLong(pRow->GetValue(slcID));

				if(m_bChangedFilter && VarBool(pRow->GetValue(slcCheck))) {
					m_pDropdownItem->aryStampFilter.push_back(nStampID);
				}

				if(m_bChangedDefaults && VarBool(pRow->GetValue(slcDefault))) {
					m_pDropdownItem->aryStampDefaults.push_back(nStampID);
				}
			}
		}

		// (j.jones 2013-03-07 13:08) - PLID 55511 - if filters are disabled, but they changed the filters,
		// warn that their changes are meaningless
		if (m_pDropdownItem->bStampFilterChanged && !m_pDropdownItem->aryStampFilter.empty()
			&& bFiltersDisabled) {
			MessageBox("The stamp filter changes you have made will have no effect because the "
				"'Disable dropdown list stamp filtering on SmartStamp tables' preference is turned on.\n\n"
				"Your filter changes will be saved, however this preference will need to be turned off for the changes to take effect.",
				"Practice", MB_ICONINFORMATION|MB_OK);
		}

		CNxDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrTableDropdownStampSetupDlg::OnBnClickedDropdownStampSelectAll()
{
	try
	{
		SelectAll(TRUE);
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrTableDropdownStampSetupDlg::OnBnClickedDropdownStampSelectNone()
{
	try
	{
		SelectAll(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrTableDropdownStampSetupDlg::SelectAll(BOOL bSelect)
{
	for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		pRow->PutValue(slcCheck, bSelect ? g_cvarTrue : g_cvarFalse);
		// (j.jones 2012-11-26 17:46) - PLID 53144 - default values are no longer
		// direclty tied to the filter
	}
	m_bChangedFilter = TRUE;
}

void CEmrTableDropdownStampSetupDlg::EditingFinishedEmrDropdownStampList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case slcCheck:
				// (z.manning 2011-10-12 14:34) - PLID 45728 - If they unchecked a stamp row make sure default is unchecked too.
				// (j.jones 2012-11-26 17:46) - PLID 53144 - default values are no longer direclty tied to the filter
				m_bChangedFilter = TRUE;
				break;

			case slcDefault:
				// (z.manning 2011-10-12 14:37) - PLID 45728 - If they checked default then make sure this stamp is checked too.
				// (j.jones 2012-11-26 17:46) - PLID 53144 - default values are no longer direclty tied to the filter
				m_bChangedDefaults = TRUE;
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
}
