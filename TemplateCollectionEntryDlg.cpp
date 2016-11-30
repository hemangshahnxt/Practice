// TemplateCollectionEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TemplateCollectionEntryDlg.h"
#include "SchedulerRc.h"
#include "afxdialogex.h"
#include "NxAPI.h"
#include "TemplateLineItemDlg.h"
#include "TemplateItemEntryGraphicalDlg.h"
#include "NxUILib\DatalistUtils.h"
#include "NxAPIUtils.h"
#include "ChooseDateRangeDlg.h"

using namespace NexTech_Accessor;
using namespace NXDATALIST2Lib;

// CTemplateCollectionEntryDlg dialog
// (z.manning 2014-12-04 13:19) - PLID 64228 - Created

IMPLEMENT_DYNAMIC(CTemplateCollectionEntryDlg, CNxDialog)

CTemplateCollectionEntryDlg::CTemplateCollectionEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTemplateCollectionEntryDlg::IDD, pParent)
{

}

CTemplateCollectionEntryDlg::~CTemplateCollectionEntryDlg()
{
}

void CTemplateCollectionEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_TEMPLATE_COLLECTION_NEW_APPLY, m_btnNewApply);
	DDX_Control(pDX, IDC_TEMPLATE_COLLECTION_EDIT_APPLY, m_btnEditApply);
	DDX_Control(pDX, IDC_TEMPLATE_COLLECTION_DELETE_APPLY, m_btnDeleteApply);
	DDX_Control(pDX, IDC_TEMPLATE_COLLECTION_UPDATE_APPLY, m_btnRemoveAndReapply);
	DDX_Control(pDX, IDC_COLOR_PICKER_CTRL, m_ctrlColorPicker);
}


BEGIN_MESSAGE_MAP(CTemplateCollectionEntryDlg, CNxDialog)
	ON_BN_CLICKED(IDC_TEMPLATE_COLLECTION_CHOOSE_COLOR, &CTemplateCollectionEntryDlg::OnBnClickedNameColorEntryChooseColor)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_TEMPLATE_COLLECTION_NEW_APPLY, &CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionNewApply)
	ON_BN_CLICKED(IDC_TEMPLATE_COLLECTION_DELETE_APPLY, OnBnClickedTemplateCollectionDeleteApply)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_TEMPLATE_COLLECTION_EDIT_APPLY, &CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionEditApply)
	ON_BN_CLICKED(IDC_TEMPLATE_COLLECTION_HIDE_OUTDATED, &CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionHideOutdated)
	ON_BN_CLICKED(IDC_TEMPLATE_COLLECTION_UPDATE_APPLY, &CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionUpdateApply)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CTemplateCollectionEntryDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CTemplateCollectionEntryDlg)
	ON_EVENT(CTemplateCollectionEntryDlg, IDC_TEMPLATE_COLLECTION_APPLY_LIST, 2 /* SelChanged */, OnSelChangedCollectionApplyList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CTemplateCollectionEntryDlg, IDC_TEMPLATE_COLLECTION_APPLY_LIST, 3, DblClickCellApplyList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CTemplateCollectionEntryDlg, IDC_TEMPLATE_COLLECTION_APPLY_LIST, 7 /* RButtonUp */, CTemplateCollectionEntryDlg::OnRButtonUpApplyList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CTemplateCollectionEntryDlg, IDC_TEMPLATE_COLLECTION_APPLY_LIST, 19, CTemplateCollectionEntryDlg::LeftClickTemplateCollectionApplyList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CTemplateCollectionEntryDlg message handlers

BOOL CTemplateCollectionEntryDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		CEdit *peditName = (CEdit*)GetDlgItem(IDC_TEMPLATE_COLLECTIOIN_NAME);
		peditName->SetLimitText(50);

		// (z.manning 2015-01-02 10:33) - PLID 64508 - Bulk caching
		g_propManager.CachePropertiesInBulk("CTemplateCollectionEntryDlg", propNumber,
			"(Username IN ('<None>', '%s')) AND Name IN ( \r\n"
			"	'HideCollectionAppliesWithPastEndDates' \r\n"
			"	) \r\n",
			_Q(GetCurrentUserName()));

		// (z.manning 2015-01-02 10:55) - PLID 64508
		SetDlgItemCheck(IDC_TEMPLATE_COLLECTION_HIDE_OUTDATED, GetRemotePropertyInt("HideCollectionAppliesWithPastEndDates", BST_UNCHECKED, 0, GetCurrentUserName()));

		if (m_pCollection == NULL)
		{
			m_pCollection.CreateInstance(__uuidof(SchedulerTemplateCollection));
			m_pCollection->Color = _bstr_t(ConvertCOLORREFToHexString(RGB(255, 0, 0)));
		}
		else
		{
			SetDlgItemText(IDC_TEMPLATE_COLLECTIOIN_NAME, (LPCTSTR)m_pCollection->Name);
		}

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnNewApply.AutoSet(NXB_NEW);
		m_btnEditApply.AutoSet(NXB_MODIFY);
		m_btnDeleteApply.AutoSet(NXB_DELETE);
		m_btnRemoveAndReapply.AutoSet(NXB_MODIFY);

		m_pdlApplies = BindNxDataList2Ctrl(IDC_TEMPLATE_COLLECTION_APPLY_LIST, false);

		LoadApplies();

		// (c.haag 2014-12-16) - PLID 64240 - EnableControls() should already have been called
		// somewhere in LoadApplies, so no need to call it here.
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (c.haag 2014-12-16) - PLID 64240 - Enables or disables controls based on the current form state
void CTemplateCollectionEntryDlg::EnableControls()
{
	// (z.manning 2016-03-07 10:51) - PLID 68443 - Support multiple rows
	std::vector<IRowSettingsPtr> vecSelectedRows = GetSelectedRows(m_pdlApplies);
	size_t nSelCount = vecSelectedRows.size();
	m_btnDeleteApply.EnableWindow(nSelCount > 0);
	m_btnEditApply.EnableWindow(nSelCount == 1);
	m_btnRemoveAndReapply.EnableWindow(nSelCount > 0);

	// (z.manning 2016-03-07 10:59) - PLID 68443 - We allow deletion and updating of multiple rows so 
	// let's keep the button text updated.
	CString strApplyWord = (nSelCount > 1 ? "Applies" : "Apply");
	m_btnDeleteApply.SetWindowText("Delete " + strApplyWord + "...");
}

void CTemplateCollectionEntryDlg::LoadApplies()
{
	_SchedulerTemplateCollectionFilterPtr pCollectionFilter(__uuidof(SchedulerTemplateCollectionFilter));
	pCollectionFilter->CollectionIDs = Nx::SafeArray<BSTR>::FromValue(m_pCollection->ID);
	_SchedulerTemplateCollectionAppliesPtr pApplies =
		GetAPI()->GetSchedulerTemplateCollectionApplies(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown*>::FromValue(pCollectionFilter));
	LoadApplies(pApplies);
}

// (z.manning 2016-03-11 11:25) - PLID 68443 - Added overload if you already have the apply objects
void CTemplateCollectionEntryDlg::LoadApplies(_SchedulerTemplateCollectionAppliesPtr pApplies)
{
	m_pdlApplies->Clear();
	m_mapApplies.clear();

	Nx::SafeArray<IUnknown*> saApplies(pApplies->Applies);
	for each (_SchedulerTemplateCollectionApplyPtr pApply in saApplies)
	{
		EnsureRowForApply(pApply);
	}
	m_pdlApplies->Sort();

	// (z.manning 2015-01-02 10:59) - PLID 64508 - May need to hide past applies
	RefreshVisibleApplies();
}

// (z.manning 2014-12-16 17:07) - PLID 64232
void CTemplateCollectionEntryDlg::EnsureRowForApply(_SchedulerTemplateCollectionApplyPtr pApply)
{
	long nApplyID = AsLong(pApply->ID);
	IRowSettingsPtr pRow = m_pdlApplies->FindByColumn(alcID, nApplyID, NULL, VARIANT_FALSE);
	if (pRow == NULL) {
		// (z.manning 2014-12-16 17:07) - PLID 64232 - The row isn't in the list yet so add it
		pRow = m_pdlApplies->GetNewRow();
		pRow = m_pdlApplies->AddRowAtEnd(pRow, NULL);
		pRow->PutValue(alcID, nApplyID);
	}
	CString strResources;
	if (pApply->Resources != NULL)
	{
		Nx::SafeArray<IUnknown*> saResources(pApply->Resources);
		for each (_SchedulerResourcePtr pResource in saResources)
		{
			if (!strResources.IsEmpty()) {
				strResources += ", ";
			}
			strResources += (LPCTSTR)pResource->Name;
		}
	}
	pRow->PutValue(alcResourceName, AsBstr(strResources));
	pRow->PutValue(alcStartDate, COleVariant(COleDateTime(pApply->StartDate)));
	if (pApply->EndDate->IsNull()) {
		pRow->PutValue(alcEndDate, "(Forever)");
	}
	else {
		pRow->PutValue(alcEndDate, pApply->EndDate->GetValue());
	}
	pRow->PutValue(alcPeriod, AsBstr(GetApparentDescriptionForCollectionApply(pApply, true)));

	// (z.manning 2014-12-16 10:59) - PLID 64232 - Store the apply in our map
	m_mapApplies[nApplyID] = pApply;
}

int CTemplateCollectionEntryDlg::EditCollection(_SchedulerTemplateCollectionPtr pCollection)
{
	m_pCollection = pCollection;

	return DoModal();
}

void CTemplateCollectionEntryDlg::OnBnClickedNameColorEntryChooseColor()
{
	try
	{
		if (!CheckCurrentUserPermissions(bioSchedTemplating, sptWrite)) {
			return;
		}

		m_ctrlColorPicker.SetColor(ConvertHexStringToCOLORREF(m_pCollection->Color));
		m_ctrlColorPicker.ShowColor();
		m_pCollection->Color = _bstr_t(ConvertCOLORREFToHexString(m_ctrlColorPicker.GetColor()));
		GetDlgItem(IDC_TEMPLATE_COLLECTION_CHOOSE_COLOR)->RedrawWindow();
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateCollectionEntryDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl == IDC_TEMPLATE_COLLECTION_CHOOSE_COLOR) {
		DrawColorOnButton(lpDrawItemStruct, ConvertHexStringToCOLORREF(m_pCollection->Color));
	}
	else {
		CNxDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
	}
}

BOOL CTemplateCollectionEntryDlg::Validate()
{
	CString strName;
	GetDlgItemText(IDC_TEMPLATE_COLLECTIOIN_NAME, strName);

	strName.TrimRight();
	if (strName.IsEmpty()) {
		MessageBox("Please enter a name.", NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	CSqlFragment sqlAdditionalWhere;
	CString strCollectionID = VarString(m_pCollection->ID, "");
	if (!strCollectionID.IsEmpty()) {
		sqlAdditionalWhere = CSqlFragment(" AND TC.ID <> {INT}", AsLong(_bstr_t(strCollectionID)));
	}

	if (ReturnsRecordsParam("SELECT TOP 1 1 FROM TemplateCollectionT TC WHERE TC.Name = {STRING}{SQL}"
		, strName, sqlAdditionalWhere))
	{
		MessageBox(FormatString("The name '%s' is already in use. Please enter a different name.", strName)
			, NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	m_pCollection->Name = _bstr_t(strName);

	return TRUE;
}

void CTemplateCollectionEntryDlg::OnOK()
{
	try
	{
		if (Validate())
		{
			// (z.manning 2014-12-12 09:07) - PLID 64228 - Save the collection
			_SchedulerTemplateCollectionCommitPtr pCommit(__uuidof(SchedulerTemplateCollectionCommit));
			pCommit->ID = m_pCollection->ID;
			pCommit->Name = m_pCollection->Name;
			pCommit->Color = m_pCollection->Color;
			GetAPI()->EditSchedulerTemplateCollection(GetAPISubkey(), GetAPILoginToken(), pCommit);

			CNxDialog::OnOK();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CTemplateCollectionEntryDlg::OnClose()
{
	try
	{
		OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-12-11 10:27) - PLID 64230
void CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionNewApply()
{
	CTemplateLineItemInfo* pLineItem = NULL;
	try
	{
		if (!CheckCurrentUserPermissions(bioSchedTemplating, sptCreate)) {
			return;
		}

		if (m_pCollection->Templates == NULL || Nx::SafeArray<IUnknown*>(m_pCollection->Templates).GetCount() == 0) {
			MessageBox("You cannot apply a collection that does not contain any templates."
				, NULL, MB_OK | MB_ICONWARNING);
			return;
		}

		pLineItem = new CTemplateLineItemInfo;
		pLineItem->m_bUseResourceAvailTemplates = false;
		pLineItem->m_bAllResources = FALSE;
		COleDateTime dtStartDate = COleDateTime::GetCurrentTime();
		dtStartDate.SetDate(dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay());
		pLineItem->m_esScale = sWeekly;
		pLineItem->m_dtStartDate = dtStartDate;
		pLineItem->m_dtEndDate = g_cdtInvalid;
		pLineItem->m_nInclude = DAYVAL(dtStartDate.GetDayOfWeek() - 1);
		pLineItem->m_nDayNumber = pLineItem->m_dtStartDate.GetDay();

		EditAndSaveApply(pLineItem);
	}
	NxCatchAll(__FUNCTION__);

	if (pLineItem != NULL) {
		delete pLineItem;
		pLineItem = NULL;
	}
}

// (z.manning 2014-12-16 11:35) - PLID 64232
void CTemplateCollectionEntryDlg::EditAndSaveApply(CTemplateLineItemInfo *pLineItem)
{
	CTemplateLineItemDlg dlgApply(stetCollection, this);
	if (dlgApply.ZoomCollectionApply(pLineItem) == IDOK)
	{
		// (z.manning 2014-12-11 15:39) - PLID 64230 - Commit the apply
		_SchedulerTemplateCollectionApplyCommitPtr pCommit(__uuidof(SchedulerTemplateCollectionApplyCommit));
		pCommit->CollectionID = m_pCollection->ID;
		pCommit->Scale = GetAPISchedulerTemplateItemScale(pLineItem->m_esScale);
		pCommit->Period = pLineItem->m_nPeriod;
		_NullableSchedulerTemplateItemMonthByPtr pMonthBy(__uuidof(NullableSchedulerTemplateItemMonthBy));
		pMonthBy->SetNullableSchedulerTemplateItemMonthBy(GetAPISchedulerTemplateItemMonthBy(pLineItem->m_embBy));
		pCommit->MonthBy = pMonthBy;
		_NullableIntPtr pPatternOrdinal(__uuidof(NullableInt));
		pPatternOrdinal->SetInt(pLineItem->m_nPatternOrdinal);
		pCommit->PatternOrdinal = pPatternOrdinal;
		pCommit->DayNumber = pLineItem->m_nDayNumber;
		Nx::SafeArray<long> saryDaysOfWeek;
		for (long i = 0; i < 7; i++)
		{
			if (pLineItem->m_nInclude & DAYVAL(i))
			{
				saryDaysOfWeek.Add(i);
			}
		}
		pCommit->DaysOfWeek = saryDaysOfWeek;
		pCommit->StartDate = pLineItem->m_dtStartDate;
		_NullableDatePtr pEndDate(__uuidof(NullableDate));
		if (pLineItem->m_dtEndDate.GetStatus() == COleDateTime::valid) {
			pEndDate->SetDate(pLineItem->m_dtEndDate);
		}
		pCommit->EndDate = pEndDate;
		pCommit->AllResources = pLineItem->m_bAllResources;

		CArray<_bstr_t, _bstr_t> arybstrResourceIDs;
		for (int nResourceIndex = 0; nResourceIndex < pLineItem->GetResourceCount(); nResourceIndex++) {
			long nResourceID = pLineItem->GetResourceByIndex(nResourceIndex).m_nID;
			arybstrResourceIDs.Add(_bstr_t(nResourceID));
		}
		pCommit->ResourceIDs = Nx::SafeArray<BSTR>::From(arybstrResourceIDs);

		CArray<_SchedulerTemplateItemExceptionPtr, _SchedulerTemplateItemExceptionPtr> arypExceptions;
		for (int nExceptionIndex = 0; nExceptionIndex < pLineItem->m_arydtExceptionDates.GetCount(); nExceptionIndex++) {
			_SchedulerTemplateItemExceptionPtr pException(__uuidof(SchedulerTemplateItemException));
			pException->ExceptionDate = pLineItem->m_arydtExceptionDates.GetAt(nExceptionIndex);
			arypExceptions.Add(pException);
		}
		pCommit->Exceptions = Nx::SafeArray<IUnknown*>::From(arypExceptions);

		_SchedulerTemplateCollectionAppliesPtr pNewApplies = NULL;
		bool bAddedApply = false;
		if (pLineItem->m_nID == -1) {
			pNewApplies = GetAPI()->CreateSchedulerTemplateCollectionApplies(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown*>::FromValue(pCommit));
			bAddedApply = true;
		}
		else {
			// (z.manning 2014-12-16 11:38) - PLID 64232 - Save existing applies
			pCommit->ID = _bstr_t(pLineItem->m_nID);
			pNewApplies = GetAPI()->EditSchedulerTemplateCollectionApplies(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown*>::FromValue(pCommit));
		}

		if (pNewApplies != NULL && pNewApplies->Applies != NULL)
		{
			Nx::SafeArray<IUnknown*> saNewApplies(pNewApplies->Applies);
			for each (_SchedulerTemplateCollectionApplyPtr pApply in saNewApplies)
			{
				EnsureRowForApply(pApply);
				if (bAddedApply) {
					m_pdlApplies->SetSelByColumn(alcID, AsLong(pApply->ID));
				}
			}

			m_pdlApplies->Sort();

			// (z.manning 2015-01-02 11:06) - PLID 64508 - Row may need to be hidden
			RefreshVisibleApplies();
			EnableControls();
		}
	}
}

// (z.manning 2014-12-16 11:01) - PLID 64232
// (z.manning 2016-03-04 08:59) - PLID 68443 - Plura now that we support multi-selection
std::vector<_SchedulerTemplateCollectionApplyPtr> CTemplateCollectionEntryDlg::GetSelectedApplies()
{
	std::vector<_SchedulerTemplateCollectionApplyPtr> vecApplies;
	for (IRowSettingsPtr pRow = m_pdlApplies->GetFirstSelRow(); pRow != nullptr; pRow = pRow->GetNextSelRow())
	{
		long nApplyID = VarLong(pRow->GetValue(alcID));
		_SchedulerTemplateCollectionApplyPtr pApply = m_mapApplies[nApplyID];
		if (pApply != nullptr) {
			vecApplies.push_back(pApply);
		}
	}

	return vecApplies;
}

// (z.manning 2014-12-16 11:29) - PLID 64232
void CTemplateCollectionEntryDlg::DblClickCellApplyList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		OnBnClickedTemplateCollectionEditApply();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-12-16 10:47) - PLID 64232
void CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionEditApply()
{
	CTemplateLineItemInfo* pLineItem = NULL;
	try
	{
		if (!CheckCurrentUserPermissions(bioSchedTemplating, sptWrite)) {
			return;
		}

		// (z.manning 2016-03-04 09:25) - PLID 68443 - Handle multi-selection support
		std::vector<_SchedulerTemplateCollectionApplyPtr> vecApplies = GetSelectedApplies();
		if (vecApplies.size() != 1) {
			return;
		}

		CTemplateLineItemDlg dlgApply(stetCollection, this);
		pLineItem = GetLineItemFromCollectionApply(vecApplies[0]);

		EditAndSaveApply(pLineItem);
	}
	NxCatchAll(__FUNCTION__);

	if (pLineItem != NULL) {
		delete pLineItem;
		pLineItem = NULL;
	}
}

// (c.haag 2014-12-16) - PLID 64240 - Called when the user elects to delete the selected apply
void CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionDeleteApply()
{
	try
	{
		if (!CheckCurrentUserPermissions(bioSchedTemplating, sptDelete)) {
			return;
		}

		// (z.manning 2016-03-03 14:53) - PLID 68443 - We now support multi-selection in the apply list
		Nx::SafeArray<BSTR> saSelectedIDs = GetSelectedValuesAsSafeArray(m_pdlApplies, alcID);
		if (saSelectedIDs.GetCount() == 0)
		{
			// We should never get here
		}
		else
		{
			CString strApplyWord = (saSelectedIDs.GetCount() == 1 ? "apply" : "applies");
			if (IDYES == MessageBox(FormatString("Are you sure you wish to remove the selected %s from the schedule?"
				, strApplyWord)
				, NULL, MB_YESNO | MB_ICONQUESTION))
			{
				// Delete from data
				GetAPI()->DeleteSchedulerTemplateCollectionApplies(GetAPISubkey(), GetAPILoginToken(), saSelectedIDs);

				// Remove from the list and update the buttons
				for (int nSelRowIndex = 0; nSelRowIndex < (int)saSelectedIDs.GetCount(); nSelRowIndex++) {
					IRowSettingsPtr pSelRow = m_pdlApplies->FindByColumn(alcID, AsLong(saSelectedIDs[nSelRowIndex]), nullptr, VARIANT_FALSE);
					if (pSelRow != nullptr) {
						m_pdlApplies->RemoveRow(pSelRow);
					}
				}
				EnableControls();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2014-12-16) - PLID 64240 - Called when the user changes the current list selection
void CTemplateCollectionEntryDlg::OnSelChangedCollectionApplyList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		EnableControls();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-12-18 17:09) - PLID 64239
void CTemplateCollectionEntryDlg::OnRButtonUpApplyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		// (z.manning 2016-03-04 08:55) - PLID 68443 - We now support multi-selection so clear out the selection 
		// before selecting the row on which they right clicked.
		m_pdlApplies->PutCurSel(nullptr);
		m_pdlApplies->PutCurSel(pRow);
		EnableControls();
		const long nApplyID = VarLong(pRow->GetValue(alcID));

		enum EMenuOptions {
			miEdit = 1,
			miCopy,
			miDelete,
		};

		CMenu mnuPopup;
		mnuPopup.CreatePopupMenu();
		mnuPopup.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, miEdit, "&Edit Apply...");
		mnuPopup.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, miCopy, "&Copy Apply...");
		mnuPopup.AppendMenu(MF_SEPARATOR | MF_BYPOSITION);
		mnuPopup.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, miDelete, "&Remove Apply...");
		mnuPopup.SetDefaultItem(miEdit);

		CPoint pt;
		GetCursorPos(&pt);

		int nResult = mnuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
		switch (nResult)
		{
			case miEdit:
				OnBnClickedTemplateCollectionEditApply();
				break;

			case miCopy:
				CopyAndEditNewApply(nApplyID);
				break;

			case miDelete:
				OnBnClickedTemplateCollectionDeleteApply();
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-12-19 11:40) - PLID 64239
void CTemplateCollectionEntryDlg::CopyAndEditNewApply(const long nApplyIDToCopy)
{
	_SchedulerTemplateCollectionApplyPtr pApplyToCopy = m_mapApplies[nApplyIDToCopy];
	if (pApplyToCopy != NULL) {
		CopyAndEditNewApply(pApplyToCopy);
	}
}

// (z.manning 2014-12-19 11:40) - PLID 64239
void CTemplateCollectionEntryDlg::CopyAndEditNewApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApplyToCopy)
{
	CTemplateLineItemInfo* pLineItem = NULL;
	try
	{
		CTemplateLineItemDlg dlgApply(stetCollection, this);
		pLineItem = GetLineItemFromCollectionApply(pApplyToCopy);
		// (z.manning 2014-12-19 11:47) - PLID 64239 - We want this to be a new line item so set that
		pLineItem->SetNew();
		EditAndSaveApply(pLineItem);
	}
	NxCatchAll(__FUNCTION__);

	if (pLineItem != NULL) {
		delete pLineItem;
		pLineItem = NULL;
	}
}

// (z.manning 2015-01-02 10:41) - PLID 64508
void CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionHideOutdated()
{
	try
	{
		UINT nChecked = IsDlgButtonChecked(IDC_TEMPLATE_COLLECTION_HIDE_OUTDATED);
		SetRemotePropertyInt("HideCollectionAppliesWithPastEndDates", nChecked, 0, GetCurrentUserName());
		RefreshVisibleApplies();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-01-02 10:42) - PLID 64508
void CTemplateCollectionEntryDlg::RefreshVisibleApplies()
{
	UINT nHidePastApplies = IsDlgButtonChecked(IDC_TEMPLATE_COLLECTION_HIDE_OUTDATED);
	for (IRowSettingsPtr pRow = m_pdlApplies->FindAbsoluteFirstRow(VARIANT_FALSE); pRow != NULL; pRow = m_pdlApplies->FindAbsoluteNextRow(pRow, VARIANT_FALSE))
	{
		VARIANT_BOOL bRowVisible = VARIANT_TRUE;
		_variant_t varEndDate = pRow->GetValue(alcEndDate);
		if (nHidePastApplies == BST_CHECKED && varEndDate.vt == VT_DATE)
		{
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			if (VarDateTime(varEndDate) < dtToday) {
				bRowVisible = VARIANT_FALSE;
			}
		}

		pRow->PutVisible(bRowVisible);
	}

	EnableControls();
}

void CTemplateCollectionEntryDlg::LeftClickTemplateCollectionApplyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		// (z.manning 2016-03-04 08:57) - PLID 68443 - Update the controls any time a row is selected now
		// that we support multi-selection.
		EnableControls();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2016-03-11 11:36) - PLID 68443 - Created
void CTemplateCollectionEntryDlg::OnBnClickedTemplateCollectionUpdateApply()
{
	try
	{
		// (z.manning 2016-03-14 14:02) - PLID 68443 - This operation may involve, modifying, creating,
		// and deleting collection applies to ensure all three permissions.
		if (!CheckCurrentUserPermissions(bioSchedTemplating, sptWrite|sptCreate|sptDelete)) {
			return;
		}

		Nx::SafeArray<BSTR> saSelectedApplyIDs = GetSelectedValuesAsSafeArray(m_pdlApplies, alcID);
		if (saSelectedApplyIDs.GetCount() == 0) {
			return;
		}

		CChooseDateRangeDlg dlgChooseDates(this);
		dlgChooseDates.SetCancelButtonVisible(TRUE);
		dlgChooseDates.SetToDateOptional(TRUE);
		if (dlgChooseDates.DoModal() == IDOK)
		{
			_NullableDateTimePtr pEndDate = GetNullableDateTime(dlgChooseDates.GetToDate());
			_SchedulerTemplateCollectionAppliesPtr pApplies =
				GetAPI()->ReApplyTemplateCollectionAppliesForDateRange(GetAPISubkey(), GetAPILoginToken()
					, m_pCollection->ID, saSelectedApplyIDs, dlgChooseDates.GetFromDate(), pEndDate);

			LoadApplies(pApplies);
		}
	}
	NxCatchAll(__FUNCTION__);
}
