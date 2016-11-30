// EmrCodingGroupEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrCodingGroupEditDlg.h"
#include "EmrCodingGroupManager.h"
#include "EMRSelectServiceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CEmrCodingGroupEditDlg dialog
// (z.manning 2011-07-05 14:50) - PLID 44421 - Created

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CEmrCodingGroupEditDlg, CNxDialog)

CEmrCodingGroupEditDlg::CEmrCodingGroupEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrCodingGroupEditDlg::IDD, pParent)
{

}

CEmrCodingGroupEditDlg::~CEmrCodingGroupEditDlg()
{
}

void CEmrCodingGroupEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_CODING_EDITOR_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDC_EMR_CODING_EDITOR_BACKGROUND2, m_nxcolor2);
	DDX_Control(pDX, IDC_ADD_EMR_CODING_GROUP, m_btnAddGroup);
	DDX_Control(pDX, IDC_RENAME_EMR_CODING_GROUP, m_btnRenameGroup);
	DDX_Control(pDX, IDC_DELETE_EMR_CODING_GROUP, m_btnDeleteGroup);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_NEW_EMR_CODING_QUANTITY_RANGE, m_btnAddRange);
	DDX_Control(pDX, IDC_NEW_EMR_CODING_CPT_CODE, m_btnAddCpt);
	DDX_Control(pDX, IDC_EMR_CODING_DELETE_SELECTED, m_btnDeleteSelected);
}


BEGIN_MESSAGE_MAP(CEmrCodingGroupEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_EMR_CODING_GROUP, &CEmrCodingGroupEditDlg::OnBnClickedAddEmrCodingGroup)
	ON_BN_CLICKED(IDC_RENAME_EMR_CODING_GROUP, &CEmrCodingGroupEditDlg::OnBnClickedRenameEmrCodingGroup)
	ON_BN_CLICKED(IDC_DELETE_EMR_CODING_GROUP, &CEmrCodingGroupEditDlg::OnBnClickedDeleteEmrCodingGroup)
	ON_BN_CLICKED(IDC_NEW_EMR_CODING_QUANTITY_RANGE, &CEmrCodingGroupEditDlg::OnBnClickedNewEmrCodingQuantityRange)
	ON_BN_CLICKED(IDC_NEW_EMR_CODING_CPT_CODE, &CEmrCodingGroupEditDlg::OnBnClickedNewEmrCodingCptCode)
ON_BN_CLICKED(IDC_EMR_CODING_DELETE_SELECTED, &CEmrCodingGroupEditDlg::OnBnClickedEmrCodingDeleteSelected)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CEmrCodingGroupEditDlg, CNxDialog)
ON_EVENT(CEmrCodingGroupEditDlg, IDC_EMR_CODING_GROUP_LIST, 16, CEmrCodingGroupEditDlg::SelChosenEmrCodingGroupList, VTS_DISPATCH)
ON_EVENT(CEmrCodingGroupEditDlg, IDC_EMR_CODING_DETAIL_LIST, 2, CEmrCodingGroupEditDlg::SelChangedEmrCodingDetailList, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CEmrCodingGroupEditDlg, IDC_EMR_CODING_DETAIL_LIST, 8, CEmrCodingGroupEditDlg::EditingStartingEmrCodingDetailList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CEmrCodingGroupEditDlg, IDC_EMR_CODING_DETAIL_LIST, 10, CEmrCodingGroupEditDlg::EditingFinishedEmrCodingDetailList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


// CEmrCodingGroupEditDlg message handlers

BOOL CEmrCodingGroupEditDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pdlGroups = BindNxDataList2Ctrl(IDC_EMR_CODING_GROUP_LIST, false);
		m_pdlGroupDetails = BindNxDataList2Ctrl(IDC_EMR_CODING_DETAIL_LIST, false);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxcolor2.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddGroup.AutoSet(NXB_NEW);
		m_btnRenameGroup.AutoSet(NXB_MODIFY);
		m_btnDeleteGroup.AutoSet(NXB_DELETE);
		m_btnAddRange.AutoSet(NXB_NEW);
		m_btnAddCpt.AutoSet(NXB_NEW);
		m_btnDeleteSelected.AutoSet(NXB_DELETE);

		CEmrCodingGroupArray *pCodingManager = GetMainFrame()->GetEmrCodingGroupManager();

		m_pdlGroupDetails->GetColumn(gldcCptQuantity)->PutComboSource(_bstr_t(FormatString(
			"%d;1;%d;Total Quantity;%d;Total Quantity - 1"
			, cqsvOne, cqsvTotal, cqsvTotalMinus1)));

		for(int nGroupIndex = 0; nGroupIndex < pCodingManager->GetCount(); nGroupIndex++)
		{
			CEmrCodingGroup *pCodingGroup = pCodingManager->GetAt(nGroupIndex);
			IRowSettingsPtr pNewGroupRow = m_pdlGroups->GetNewRow();
			pNewGroupRow->PutValue(glcPointer, (long)pCodingGroup);
			pNewGroupRow->PutValue(glcName, _bstr_t(pCodingGroup->GetName()));
			m_pdlGroups->AddRowSorted(pNewGroupRow, NULL);
		}

		if(m_pdlGroups->GetRowCount() > 0) {
			m_pdlGroups->PutCurSel(m_pdlGroups->GetFirstRow());
		}
		UpdateView();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEmrCodingGroupEditDlg::OnBnClickedAddEmrCodingGroup()
{
	try
	{
		CString strNewName;
		int nResult = InputBoxLimitedWithParent(this, "Enter a new name for the coding group", strNewName, "", 50, false, false, NULL);
		if(nResult == IDOK)
		{
			if(!GetMainFrame()->GetEmrCodingGroupManager()->IsNameValid(strNewName, this)) {
				return;
			}

			CEmrCodingGroup* pNewCodingGroup = GetMainFrame()->GetEmrCodingGroupManager()->CreateNewGroup(strNewName);
			HandleCodingGroupChange();

			IRowSettingsPtr pNewRow = m_pdlGroups->GetNewRow();
			pNewRow->PutValue(glcPointer, (long)pNewCodingGroup);
			pNewRow->PutValue(glcName, _bstr_t(strNewName));
			m_pdlGroups->PutCurSel(m_pdlGroups->AddRowSorted(pNewRow, NULL));
			UpdateView();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::OnBnClickedRenameEmrCodingGroup()
{
	try
	{
		CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
		if(pCodingGroup == NULL) {
			return;
		}

		CString strOldName = pCodingGroup->GetName();
		CString strNewName;
		int nResult = InputBoxLimitedWithParent(this, "Enter a new name for the coding group", strNewName, "", 50, false, false, NULL);
		if(nResult == IDOK)
		{
			if(strOldName.CompareNoCase(strNewName) != 0) {
				if(!GetMainFrame()->GetEmrCodingGroupManager()->IsNameValid(strNewName, this)) {
					return;
				}
			}

			CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
			pCodingGroup->RenameGroup(strNewName);
			HandleCodingGroupChange();
			m_pdlGroups->GetCurSel()->PutValue(glcName, _bstr_t(strNewName));
			m_pdlGroups->Sort();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::OnBnClickedDeleteEmrCodingGroup()
{
	try
	{
		CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
		if(pCodingGroup == NULL) {
			return;
		}

		if(ReturnsRecordsParam("SELECT EmrCodingGroupID FROM EmnCodingGroupLinkT WHERE EmrCodingGroupID = {INT}", pCodingGroup->GetID())) {
			MessageBox("You cannot delete this coding group because it is used on at least one patient EMN.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		int nResult = MessageBox("Are you sure you want to delete this coding group?\r\n\r\nThis cannot be undone.", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult == IDYES)
		{
			GetMainFrame()->GetEmrCodingGroupManager()->DeleteGroup(pCodingGroup->GetID());
			HandleCodingGroupChange();
			m_pdlGroups->RemoveRow(m_pdlGroups->GetCurSel());
			if(m_pdlGroups->GetRowCount() > 0) {
				m_pdlGroups->PutCurSel(m_pdlGroups->GetFirstRow());
			}
			UpdateView();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::UpdateView(bool bForceRefresh /* = true */)
{
	if(bForceRefresh) {
		LoadCurrentGroup();
	}

	BOOL bEnableGroupControls;
	if(GetCurrentCodingGroupPointer() == NULL) {
		bEnableGroupControls = FALSE;
	}
	else {
		bEnableGroupControls = TRUE;
	}
	m_btnRenameGroup.EnableWindow(bEnableGroupControls);
	m_btnDeleteGroup.EnableWindow(bEnableGroupControls);
	m_btnAddRange.EnableWindow(bEnableGroupControls);

	BOOL bEnableDetailControls;
	if(m_pdlGroupDetails->GetCurSel() == NULL) {
		bEnableDetailControls = FALSE;
	}
	else {
		bEnableDetailControls = TRUE;
	}
	m_btnAddCpt.EnableWindow(bEnableDetailControls);
	m_btnDeleteSelected.EnableWindow(bEnableDetailControls);
}

void CEmrCodingGroupEditDlg::LoadCurrentGroup()
{
	m_pdlGroupDetails->SetRedraw(VARIANT_FALSE);
	m_pdlGroupDetails->Clear();
	
	CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
	if(pCodingGroup != NULL)
	{
		// (z.manning 2011-07-06 09:44) - Before we fill the details let's load info about all CPT codes for this
		// group at once so we don't have to query each time through the loop.
		CArray<long,long> arynCptCodeIDs;
		pCodingGroup->GetAllCptCodeIDs(&arynCptCodeIDs);
		CMap<long,long,CString,LPCTSTR> mapCptIDToCode, mapCptIDToDescription;
		if(arynCptCodeIDs.GetCount() > 0)
		{
			ADODB::_RecordsetPtr prsCpt = CreateParamRecordset(
				"SELECT CptCodeT.ID, Code, Name \r\n"
				"FROM CptCodeT \r\n"
				"INNER JOIN ServiceT ON CptCodeT.ID = ServiceT.ID \r\n"
				"WHERE CptCodeT.ID IN ({INTARRAY}) \r\n"
				, arynCptCodeIDs);
			for(; !prsCpt->eof; prsCpt->MoveNext()) {
				const long nID = AdoFldLong(prsCpt, "ID");
				const CString strCode = AdoFldString(prsCpt, "Code", "");
				const CString strDescription = AdoFldString(prsCpt, "Name", "");
				mapCptIDToCode.SetAt(nID, strCode);
				mapCptIDToDescription.SetAt(nID, strDescription);
			}
		}

		for(int nRangeIndex = 0; nRangeIndex < pCodingGroup->GetRangeCount(); nRangeIndex++)
		{
			// (z.manning 2011-07-06 09:47) - First add the parent row for this coding group range
			CEmrCodingRange *pCodingRange = pCodingGroup->GetRangeByIndex(nRangeIndex);
			IRowSettingsPtr pRangeRow = GetNewDetailRow();
			pRangeRow->PutValue(gldcRangePointer, (long)pCodingRange);
			pRangeRow->PutValue(gldcQuantity, pCodingRange->GetMinQuantity());
			m_pdlGroupDetails->AddRowAtEnd(pRangeRow, NULL);

			// (z.manning 2011-07-06 09:47) - Now go through and add all the detail rows for this range as children.
			for(int nDetailIndex = 0; nDetailIndex < pCodingRange->GetDetailCount(); nDetailIndex++)
			{
				CEmrCodingGroupDetail *pCodingDetail = pCodingRange->GetDetailByIndex(nDetailIndex);
				IRowSettingsPtr pDetailRow = GetNewDetailRow();
				pDetailRow->PutValue(gldcDetailPointer, (long)pCodingDetail);
				ECptQuantitySentinelValues eQuantityType = GetQuantitySentinelFromCodingDetail(pCodingDetail);
				pDetailRow->PutValue(gldcCptQuantity, (short)eQuantityType);
				CString strCptCode, strCptDescription;
				mapCptIDToCode.Lookup(pCodingDetail->GetCptCodeID(), strCptCode);
				mapCptIDToDescription.Lookup(pCodingDetail->GetCptCodeID(), strCptDescription);
				pDetailRow->PutValue(gldcCptCode, _bstr_t(strCptCode));
				pDetailRow->PutValue(gldcCptName, _bstr_t(strCptDescription));
				m_pdlGroupDetails->AddRowAtEnd(pDetailRow, pRangeRow);
			}

			pRangeRow->PutExpanded(VARIANT_TRUE);
		}
	}

	m_pdlGroupDetails->SetRedraw(VARIANT_TRUE);
}

void CEmrCodingGroupEditDlg::OnBnClickedNewEmrCodingQuantityRange()
{
	try
	{
		AddQuantityRange();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::OnBnClickedNewEmrCodingCptCode()
{
	try
	{
		AddCptCode();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::HandleCodingGroupChange()
{
	// (z.manning 2011-07-07 10:30) - PLID 44421 - No need to send this table checker locally because we update
	// local memory every time a change occurs.
	CClient::RefreshTable(NetUtils::EmrCodingGroupsT, -1, NULL, FALSE);
}

void CEmrCodingGroupEditDlg::AddQuantityRange()
{
	CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
	if(pCodingGroup == NULL) {
		return;
	}

	CString strMinQuantity;
	int nResult = InputBox(this, "Enter the quantity value that triggers this range to begin", strMinQuantity, "", false, false, NULL, TRUE);
	if(nResult == IDOK)
	{
		long nMinQuantity = atol(strMinQuantity);
		if(nMinQuantity <= 0) {
			MessageBox("Please enter a number greater than zero.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		if(pCodingGroup->FindByMinQuantity(nMinQuantity) != NULL) {
			MessageBox("There is already a range for that quantity.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		pCodingGroup->CreateNewRange(nMinQuantity);
		HandleCodingGroupChange();
		UpdateView();
	}
}

void CEmrCodingGroupEditDlg::AddCptCode()
{
	CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
	if(pCodingGroup == NULL) {
		return;
	}

	IRowSettingsPtr pRangeRow = GetCurrentRangeRow();
	if(pRangeRow == NULL) {
		return;
	}

	CEmrCodingRange *pCodingRange = (CEmrCodingRange*)((long)(pRangeRow->GetValue(gldcRangePointer)));

	CEMRSelectServiceDlg dlg(this);
	dlg.m_bCptCodesOnly = TRUE;
	if(dlg.DoModal() == IDOK)
	{
		if(dlg.m_ServiceID != -1)
		{
			// (z.manning 2011-07-07 10:03) - We do not allow a code in more than one group. Check data for this to be safe.
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT EmrCodingGroupsT.Name \r\n"
				"FROM EmrCodingGroupsT \r\n"
				"INNER JOIN EmrCodingGroupRangesT ON EmrCodingGroupsT.ID = EmrCodingGroupRangesT.EmrCodingGroupID \r\n"
				"INNER JOIN EmrCodingGroupDetailsT ON EmrCodingGroupRangesT.ID = EmrCodingGroupDetailsT.EmrCodingGroupRangeID \r\n"
				"WHERE EmrCodingGroupsT.ID <> {INT} AND EmrCodingGroupDetailsT.CptCodeID = {INT} \r\n"
				, pCodingGroup->GetID(), dlg.m_ServiceID);
			if(!prs->eof) {
				CString strExistingGroupName = AdoFldString(prs, "Name");
				MessageBox(FormatString("The selected code is already in coding group '%s' so it cannot be part of this group.", strExistingGroupName), NULL, MB_OK|MB_ICONERROR);
				return;
			}

			// (j.jones 2012-02-22 17:41) - PLID 48330 - we disallow selecting the same service code twice in a range,
			// but similarly we should warn if they pick the same CPT code (different subcode), because that makes no
			// sense and they are probably picking the wrong code
			BOOL bFoundDuplicateCPTCode = FALSE;
			for(int nDetailIndex = 0; nDetailIndex < pCodingRange->GetDetailCount(); nDetailIndex++)
			{
				CEmrCodingGroupDetail *pCodingDetail = pCodingRange->GetDetailByIndex(nDetailIndex);
				long nDetailCptID = pCodingDetail->GetCptCodeID();
				if(nDetailCptID == dlg.m_ServiceID) {
					MessageBox("The selected code is already in this range group.", NULL, MB_OK|MB_ICONERROR);
					return;
				}
				else if(ReturnsRecordsParam("SELECT ID FROM CPTCodeT WHERE ID = {INT} AND Code IN (SELECT Code FROM CPTCodeT WHERE ID = {INT})", nDetailCptID, dlg.m_ServiceID)) {
					//track that we found a dupe, but don't prompt just yet, incase multiple dupes intentionally exist
					bFoundDuplicateCPTCode = TRUE;
				}
			}

			if(bFoundDuplicateCPTCode) {
				if(IDNO == MessageBox("A different service code with the same code number is already in this range group.\n"
					"Are you sure you wish to add this code again with a different subcode?", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
					return;
				}
			}

			pCodingRange->CreateNewDetail(dlg.m_ServiceID);
			HandleCodingGroupChange();
			UpdateView();
		}
	}
}

void CEmrCodingGroupEditDlg::DeleteSelected()
{
	CEmrCodingGroup *pCodingGroup = GetCurrentCodingGroupPointer();
	if(pCodingGroup == NULL) {
		return;
	}

	IRowSettingsPtr pCurSel = m_pdlGroupDetails->GetCurSel();
	if(pCurSel == NULL) {
		return;
	}

	_variant_t varRangePointer = pCurSel->GetValue(gldcRangePointer);
	_variant_t varDetailPointer = pCurSel->GetValue(gldcDetailPointer);
	CString strMessage;
	if(varRangePointer.vt != VT_NULL)
	{
		CEmrCodingRange *pCodingRange = (CEmrCodingRange*)((long)varRangePointer);
		// (z.manning 2011-07-06 15:15) - PLID 44421 - We do not allow deleting of the range with min quantity of one.
		// This ensures that a coding group never has an undefined state (i.e. it has a lower quantity than the lowest
		// quantity in the group).
		if(pCodingRange->GetMinQuantity() == 1) {
			MessageBox("You cannot delete the quantity one range of a coding group.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		strMessage = "Are you sure you want to delete this quantity range and all codes associated with it?";
	}
	else if(varDetailPointer.vt != VT_NULL) {
		strMessage = "Are you sure you want to delete this code?";
	}
	else {
		// (z.manning 2011-07-06 12:33) - Every row should be a range row or a detail row
		ASSERT(FALSE);
		return;
	}

	if(MessageBox(strMessage, NULL, MB_YESNO|MB_ICONQUESTION) == IDYES)
	{
		if(varRangePointer.vt != VT_NULL) {
			CEmrCodingRange *pCodingRange = (CEmrCodingRange*)((long)varRangePointer);
			pCodingGroup->DeleteRange(pCodingRange->GetID());
		}
		else if(varDetailPointer.vt != VT_NULL) {
			CEmrCodingGroupDetail *pCodingDetail = (CEmrCodingGroupDetail*)((long)varDetailPointer);
			IRowSettingsPtr pRangeRow = pCurSel->GetParentRow();
			CEmrCodingRange *pCodingRange = (CEmrCodingRange*)((long)(pRangeRow->GetValue(gldcRangePointer)));
			pCodingRange->DeleteDetail(pCodingDetail);
		}

		HandleCodingGroupChange();
		UpdateView();
	}
}

IRowSettingsPtr CEmrCodingGroupEditDlg::GetNewDetailRow()
{
	IRowSettingsPtr pNewRow = m_pdlGroupDetails->GetNewRow();
	for(short nCol = 0; nCol < m_pdlGroupDetails->GetColumnCount(); nCol++) {
		pNewRow->PutValue(nCol, g_cvarNull);
	}
	return pNewRow;
}

void CEmrCodingGroupEditDlg::SelChosenEmrCodingGroupList(LPDISPATCH lpRow)
{
	try
	{
		UpdateView();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::OnBnClickedEmrCodingDeleteSelected()
{
	try
	{
		DeleteSelected();
	}
	NxCatchAll(__FUNCTION__);
}

IRowSettingsPtr CEmrCodingGroupEditDlg::GetCurrentRangeRow()
{
	IRowSettingsPtr pCurSel = m_pdlGroupDetails->GetCurSel();
	if(pCurSel == NULL || pCurSel->GetValue(gldcRangePointer).vt != VT_NULL) {
		return pCurSel;
	}
	else {
		return pCurSel->GetParentRow();
	}
}

void CEmrCodingGroupEditDlg::SelChangedEmrCodingDetailList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		UpdateView(false);
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::EditingStartingEmrCodingDetailList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		_variant_t varRangePointer = pRow->GetValue(gldcRangePointer);

		if(varRangePointer.vt != VT_NULL) {
			*pbContinue = FALSE;
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCodingGroupEditDlg::EditingFinishedEmrCodingDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(!bCommit || _variant_t(varOldValue) == _variant_t(varNewValue)) {
			return;
		}

		switch(nCol)
		{
			case gldcCptQuantity:
				UpdateCurrentRowQuantity();
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
}

CEmrCodingGroupEditDlg::ECptQuantitySentinelValues CEmrCodingGroupEditDlg::GetQuantitySentinelFromCodingDetail(CEmrCodingGroupDetail *pCodingDetail)
{
	if(pCodingDetail->GetType() == ecqtQuantity)
	{
		if(pCodingDetail->GetQuantityValue() == 1) {
			return cqsvOne;
		}
		else {
			return cqsvInvalid;
		}
	}
	else if(pCodingDetail->GetType() == ecqtSubtractFromTotal)
	{
		if(pCodingDetail->GetQuantityValue() == 0) {
			return cqsvTotal;
		}
		else if(pCodingDetail->GetQuantityValue() == 1) {
			return cqsvTotalMinus1;
		}
		else {
			return cqsvInvalid;
		}
	}
	else {
		// (z.manning 2011-07-06 16:32) - Those are the only valid types
		ASSERT(FALSE);
		ThrowNxException("Invalid coding detail type: %li", pCodingDetail->GetType());
	}

	return cqsvInvalid;
}

void CEmrCodingGroupEditDlg::UpdateCurrentRowQuantity()
{
	IRowSettingsPtr pCurSel = m_pdlGroupDetails->GetCurSel();
	if(pCurSel == NULL) {
		return;
	}

	ECptQuantitySentinelValues eQuantitySentinel = (ECptQuantitySentinelValues)VarShort(pCurSel->GetValue(gldcCptQuantity));
	if(eQuantitySentinel == cqsvInvalid) {
		return;
	}
	
	_variant_t varDetailPointer = pCurSel->GetValue(gldcDetailPointer);
	if(varDetailPointer.vt != VT_NULL)
	{
		CEmrCodingGroupDetail *pCodingDetail = (CEmrCodingGroupDetail*)((long)varDetailPointer);
		EEmrCodingQuantityTypes eNewQuantityType;
		long nNewQuantityValue;
		switch(eQuantitySentinel)
		{
			case cqsvOne:
				eNewQuantityType = ecqtQuantity;
				nNewQuantityValue = 1;
				break;

			case cqsvTotal:
				eNewQuantityType = ecqtSubtractFromTotal;
				nNewQuantityValue = 0;
				break;

			case cqsvTotalMinus1:
				eNewQuantityType = ecqtSubtractFromTotal;
				nNewQuantityValue = 1;
				break;
		}

		pCodingDetail->UpdateQuantity(eNewQuantityType, nNewQuantityValue);
		HandleCodingGroupChange();
	}
}

CEmrCodingGroup* CEmrCodingGroupEditDlg::GetCurrentCodingGroupPointer()
{
	IRowSettingsPtr pRow = m_pdlGroups->GetCurSel();
	if(pRow == NULL) {
		return NULL;
	}

	CEmrCodingGroup *pCodingGroup = (CEmrCodingGroup*)((long)pRow->GetValue(glcPointer));
	return pCodingGroup;
}