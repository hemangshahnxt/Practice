// EMRChargePromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "EMRChargePromptDlg.h"
#include "ChargeSplitDlg.h"
#include "EmrColors.h"
#include "EmrCodingGroupManager.h"
#include "EMNChargeArray.h"
#include "EmnCodingGroupInfo.h"
#include "EmrCodesDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRChargePromptDlg dialog
using namespace NXDATALIST2Lib;

// (z.manning 2011-07-11 14:11) - PLID 44469 - Added optional param for coding group
// (j.jones 2012-01-26 11:19) - PLID 47700 - added patient ID, and an optional EMNID
// (j.jones 2013-05-16 15:13) - PLID 56596 - m_arypWorkingCharges and m_arypAllLoadedCharges are now references, need to be created here
CEMRChargePromptDlg::CEMRChargePromptDlg(CWnd* pParent, long nPatientID, CEMNChargeArray *parypCharges, long nEMNID /*= -1*/, 
										 CEmrCodingGroup *pCodingGroup /*= NULL*/, CEmnCodingGroupInfo *pEmnCodingInfo /*= NULL*/)
	: CNxDialog(CEMRChargePromptDlg::IDD, pParent)
	, m_nPatientID(nPatientID)
	, m_parypOriginalCharges(parypCharges)
	, m_nEMNID(nEMNID)
	, m_pCodingGroup(pCodingGroup)
	, m_pEmnCodingInfo(pEmnCodingInfo)
	, m_arypWorkingCharges(*(new CEMNChargeArray()))
	, m_arypAllLoadedCharges(*(new CEMNChargeArray()))
{
	//{{AFX_DATA_INIT(CEMRChargePromptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (z.manning 2011-07-12 11:06) - PLID 44469 - Keep track of the original charges in case we need to revert
	if(parypCharges->GetCount() > 0)
	{
		CEMN *pEmnOverride = parypCharges->GetAt(0)->pEmnOverride;
		m_arypWorkingCharges.CopyNew(parypCharges, pEmnOverride, &m_mapWorkingChargeToOriginal);
		m_arypAllLoadedCharges.Append(m_arypWorkingCharges);
	}
}

CEMRChargePromptDlg::~CEMRChargePromptDlg()
{
	try {

		// (z.manning 2011-07-12 17:30) - PLID 44469 - Clean up any charge object we may have loaded that
		// are not being returned in the original charge array and are not a part of the working array.
		for(int nAllChargeIndex = m_arypAllLoadedCharges.GetCount() - 1; nAllChargeIndex >= 0; nAllChargeIndex--) {
			EMNCharge *pCharge = m_arypAllLoadedCharges.GetAt(nAllChargeIndex);
			if(!m_arypWorkingCharges.HasCharge(pCharge) && !m_parypOriginalCharges->HasCharge(pCharge)) {
				delete pCharge;
				m_arypAllLoadedCharges.RemoveAt(nAllChargeIndex);
			}
		}

		// (z.manning 2011-07-12 17:31) - PLID 44469 - Now do the same thing for the working array
		for(int nWorkingChargeIndex = m_arypWorkingCharges.GetCount() - 1; nWorkingChargeIndex >= 0; nWorkingChargeIndex--) {
			EMNCharge *pCharge = m_arypWorkingCharges.GetAt(nWorkingChargeIndex);
			if(!m_parypOriginalCharges->HasCharge(pCharge)) {
				delete pCharge;
				m_arypWorkingCharges.RemoveAt(nWorkingChargeIndex);
			}
		}

		// (j.jones 2013-05-16 15:13) - PLID 56596 - m_arypWorkingCharges and m_arypAllLoadedCharges are now a references,
		// they are never null, are always filled in the constructor, and must be cleared here
		delete &m_arypWorkingCharges;
		delete &m_arypAllLoadedCharges;

	}NxCatchAll(__FUNCTION__);
}


void CEMRChargePromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRChargePromptDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EMR_CHARGE_PROMPT_BACKGROUND, m_nxcolorBackground);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRChargePromptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRChargePromptDlg)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EMR_CHARGE_PROMPT_QUANTITY, &CEMRChargePromptDlg::OnEnChangeEmrChargePromptQuantity)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRChargePromptDlg message handlers

BOOL CEMRChargePromptDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//DRT 1/11/2007 - PLID 24182 - This dialog allows the EMR to pop up a to-be-spawned charge
		//	to the user and let them make modifications to that.  This saves the user time because
		//	they do not need to spawn their charge, then make changes to it later.

		// (c.haag 2008-04-28 11:49) - PLID 29806 - NxIconize the buttons
		// (c.haag 2008-08-15 09:24) - PLID 29806 - Don requested we not have any icons in this dialog
		//m_btnOK.AutoSet(NXB_OK);
		//m_btnCancel.AutoSet(NXB_CANCEL);
		// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
		// -1 EMNID is passed in the contructor if a template called this dialog
		m_bIsTemplate = m_nEMNID == -1 ? TRUE : FALSE;
		//Setup datalist
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pList = BindNxDataList2Ctrl(this, IDC_ADD_CHARGE_LIST, GetRemoteData(), false);

		// (z.manning 2011-07-12 09:03) - PLID 44469 - Added NxColor, use same color as EMR topic wnd background
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_nxcolorBackground.SetColor(EmrColors::Topic::PatientBackground());

		((CEdit*)GetDlgItem(IDC_EMR_CHARGE_PROMPT_QUANTITY))->SetLimitText(8);

		//We need to set the embedded combo boxes here so they will requery
		{
			// (z.manning, 05/01/2007) - PLID 16623 - Don't show inactive modifiers.
			CString strSource = "SELECT Number AS ID, (Number + '   ' + Note) AS Text, Active FROM CPTModifierT UNION SELECT '' AS ID, '     (None)' AS Text, 1 ORDER BY Text ASC";
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(clcMod1);
			pCol->ComboSource = _bstr_t(strSource);
			pCol = m_pList->GetColumn(clcMod2);
			pCol->ComboSource = _bstr_t(strSource);
			pCol = m_pList->GetColumn(clcMod3);
			pCol->ComboSource = _bstr_t(strSource);
			pCol = m_pList->GetColumn(clcMod4);
			pCol->ComboSource = _bstr_t(strSource);
		}

		{
			// (j.jones 2012-01-26 11:03) - PLID 47700 - fill the resp. dropdown with the patient's insured parties
			CString strComboSource = GetEMRChargeRespAssignComboBoxSource(m_nPatientID, m_nEMNID);
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(clcInsuredPartyID);
			pCol->PutComboSource(_bstr_t(strComboSource));
		}

		RefreshChargeList();

		UpdateControls();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRChargePromptDlg::RefreshChargeList()
{
	// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
	BOOL bShowChargeCategory = FALSE;
	//Load the m_pCharge member into the datalist
	// (z.manning 2011-07-07 16:44) - PLID 44469 - We now support multiple charges here
	for(int nChargeIndex = 0; nChargeIndex < m_arypWorkingCharges.GetCount(); nChargeIndex++)
	{
		EMNCharge *pCharge = m_arypWorkingCharges.GetAt(nChargeIndex);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
		// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
		if (pCharge->nCategoryCount > 1 && m_bIsTemplate == FALSE)
		{
			bShowChargeCategory = TRUE;
		}
		SetCPTCategoryCombo(pRow, pCharge);
		pRow->PutValue(clcCode, _bstr_t(pCharge->strCode));
		pRow->PutValue(clcSubCode, _bstr_t(pCharge->strSubCode));
		pRow->PutValue(clcCategory, pCharge->nCategoryID);
		pRow->PutValue(clcDescription, _bstr_t(pCharge->strDescription));
		pRow->PutValue(clcQuantity, (double)(pCharge->dblQuantity));
		pRow->PutValue(clcMod1, _bstr_t(pCharge->strMod1));
		pRow->PutValue(clcMod2, _bstr_t(pCharge->strMod2));
		pRow->PutValue(clcMod3, _bstr_t(pCharge->strMod3));
		pRow->PutValue(clcMod4, _bstr_t(pCharge->strMod4));
		pRow->PutValue(clcUnitCost, _variant_t(pCharge->cyUnitCost));
		// (j.jones 2012-01-26 10:59) - PLID 47700 - added Resp. dropdown
		pRow->PutValue(clcInsuredPartyID, _variant_t(pCharge->nInsuredPartyID));
		m_pList->AddRowAtEnd(pRow, NULL);
	}
	// (s.tullis 2015-04-14 11:47) - PLID 64978 - added charge category
	if (bShowChargeCategory)
	{
		ForceShowColumn(clcCategory, 10, 85);
	}
}

void CEMRChargePromptDlg::OnOK() 
{
	try {
		//Commit our changes back to the EMNCharge structure

		SaveChargeListToArray();

		if(IsCodingGroup())
		{
			UINT nQuantity = GetDlgItemInt(IDC_EMR_CHARGE_PROMPT_QUANTITY);
			if(nQuantity <= 0) {
				// (z.manning 2011-07-11 14:44) - PLID 44469 - Ensure we have a valid quantity
				MessageBox("You must enter a valid quantity.", NULL, MB_OK|MB_ICONERROR);
				return;
			}

			m_pEmnCodingInfo->m_nGroupQuantity = nQuantity;
		}

		// (z.manning 2011-07-12 11:40) - PLID 44469 - We now need to "save" any charge changes back to the original
		// charge array.
		for(int nChargeIndex = 0; nChargeIndex < m_arypWorkingCharges.GetCount(); nChargeIndex++)
		{
			EMNCharge *pWorkingCharge = m_arypWorkingCharges.GetAt(nChargeIndex);
			EMNCharge *pOriginalCharge = NULL;
			// (z.manning 2011-07-12 11:41) - PLID 44469 - If this charge corresponds to a charge in the original
			// charge array then simply update the original charge. Otherwise add the new charge to the original array.
			if(m_mapWorkingChargeToOriginal.Lookup(pWorkingCharge, pOriginalCharge)) {
				*pOriginalCharge = *pWorkingCharge;
				m_mapWorkingChargeToOriginal.RemoveKey(pWorkingCharge);
			}
			else {
				m_parypOriginalCharges->Add(pWorkingCharge);
				// (z.manning 2011-07-12 11:44) - PLID 44469 - Since we're using this charge we must remove it
				// from the working charge array so we don't deallocate it later.
				m_arypWorkingCharges.RemoveAt(nChargeIndex);
				nChargeIndex--;
			}
		}

		// (z.manning 2011-07-12 17:34) - PLID 44469 - If there are any charges left in the map that means
		// we have charges that were removed in this dialog, so we need to ensure they're not in the main
		// array. Note: the caller is responsible for freeing their memory.
		POSITION pos = m_mapWorkingChargeToOriginal.GetStartPosition();
		while(pos != NULL)
		{
			EMNCharge *pWorkingCharge = NULL, *pOriginalCharge = NULL;
			m_mapWorkingChargeToOriginal.GetNextAssoc(pos, pWorkingCharge, pOriginalCharge);
			m_parypOriginalCharges->RemoveCharge(pOriginalCharge);
		}

		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

void CEMRChargePromptDlg::SaveChargeListToArray()
{
	//We can only have 1 row, but for future-compatibility (just in case), we'll loop
	//	over all rows in the list.
	// (z.manning 2011-07-07 16:46) - PLID 44469 - Look at that, someone planning ahead (sort of).
	// We do now support multiple charges here.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);
	for(int nChargeIndex = 0; nChargeIndex < m_arypWorkingCharges.GetCount(); nChargeIndex++)
	{
		if(pRow == NULL) {
			ASSERT(FALSE);
			ThrowNxException(CString(__FUNCTION__) + " - Mismatch between charge list and charge array");
		}
		EMNCharge *pCharge = m_arypWorkingCharges.GetAt(nChargeIndex);
		pCharge->dblQuantity = VarDouble(pRow->GetValue(clcQuantity));
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		pCharge->nCategoryID = VarLong(pRow->GetValue(clcCategory),-1);
		pCharge->strMod1 = VarString(pRow->GetValue(clcMod1), "");
		pCharge->strMod2 = VarString(pRow->GetValue(clcMod2), "");
		pCharge->strMod3 = VarString(pRow->GetValue(clcMod3), "");
		pCharge->strMod4 = VarString(pRow->GetValue(clcMod4), "");
		pCharge->cyUnitCost = VarCurrency(pRow->GetValue(clcUnitCost));
		// (j.jones 2012-01-26 10:59) - PLID 47700 - added Resp. dropdown
		pCharge->nInsuredPartyID = VarLong(pRow->GetValue(clcInsuredPartyID), -2);

		pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
	}
}

void CEMRChargePromptDlg::OnCancel() 
{
	//Discard any changes made (just leave the m_pCharge structure alone)

	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CEMRChargePromptDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRChargePromptDlg)
	ON_EVENT(CEMRChargePromptDlg, IDC_ADD_CHARGE_LIST, 9 /* EditingFinishing */, OnEditingFinishingAddChargeList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRChargePromptDlg::OnEditingFinishingAddChargeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		//If already cancelling, skip
		if(*pbCommit == FALSE)
			return;

		switch(nCol) {
		case clcQuantity:	//Quantity
			{
				//Ensure that the quantity is > 0
				double dbl = VarDouble(pvarNewValue, 0.0);
				if(LooseCompareDouble(dbl, 0.0, 0.001) <= 0) {
					//0 or negative
					MessageBox("You must enter a positive quantity.", "NexTech", MB_OK);
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
			}
			break;
		}


	} NxCatchAll("Error in OnEditingFinishingAddChargeList");
}

// (z.manning 2011-07-11 14:14) - PLID 44469
void CEMRChargePromptDlg::UpdateControls()
{
	BOOL bShowCodingGroup;
	if(IsCodingGroup()) {
		bShowCodingGroup = TRUE;
		SetDlgItemText(IDC_EMR_CHARGE_PROMPT_CAPTION, "Current charge(s) for coding group " + m_pCodingGroup->GetName());
		SetDlgItemInt(IDC_EMR_CHARGE_PROMPT_QUANTITY, m_pEmnCodingInfo->m_nGroupQuantity);
		// (z.manning 2011-07-12 08:44) - PLID 44469 - No editing quantity if this is part of a coding group.
		m_pList->GetColumn(clcQuantity)->PutEditable(VARIANT_FALSE);
	}
	else {
		bShowCodingGroup = FALSE;
		SetDlgItemText(IDC_EMR_CHARGE_PROMPT_CAPTION, "");
		m_pList->GetColumn(clcQuantity)->PutEditable(VARIANT_TRUE);
	}

	UINT nShowCmd = bShowCodingGroup ? SW_SHOWNA : SW_HIDE;
	GetDlgItem(IDC_EMR_CHARGE_PROMPT_CAPTION)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_EMR_CHARGE_PROMPT_QUANTITY_LABEL)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_EMR_CHARGE_PROMPT_QUANTITY)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_EMR_CHARGE_PROMPT_QUANTITY)->EnableWindow(bShowCodingGroup);
}

BOOL CEMRChargePromptDlg::IsCodingGroup()
{
	return (m_pCodingGroup != NULL && m_pEmnCodingInfo != NULL);
}

// (z.manning 2011-07-12 09:25) - PLID 44469
void CEMRChargePromptDlg::OnEnChangeEmrChargePromptQuantity()
{
	try
	{
		if(!IsCodingGroup()) {
			// (z.manning 2011-07-12 09:36) - Shoudn't be here then;
			ASSERT(FALSE);
			return;
		}

		SaveChargeListToArray();

		m_pList->Clear();
		m_arypWorkingCharges.RemoveAll();
		UINT nQuantity = GetDlgItemInt(IDC_EMR_CHARGE_PROMPT_QUANTITY);
		if(nQuantity > 0)
		{
			CEmrCodingRange *pCodingRange = m_pCodingGroup->FindByCurQuantity(nQuantity);
			if(pCodingRange != NULL) {
				GetChargesByCodingGroupQuantity(pCodingRange, nQuantity, &m_arypAllLoadedCharges, &m_arypWorkingCharges);
				RefreshChargeList();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (s.tullis 2015-04-01 14:09) - PLID 64978 - Support Charge Category
void CEMRChargePromptDlg::SetCPTCategoryCombo(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNCharge* pCharge)
{
	try{
		NXDATALIST2Lib::IFormatSettingsPtr pfsLookup(__uuidof(NXDATALIST2Lib::FormatSettings));
		if (pRow){
			pfsLookup = GetCPTMultiCategoryCombo(pCharge);

			if (pfsLookup != NULL){
				pRow->PutRefCellFormatOverride(clcCategory, pfsLookup);
			}
			else{
				ASSERT(FALSE);
			}
		}

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category  Gets the Format Setting pointer 
IFormatSettingsPtr CEMRChargePromptDlg::GetCPTMultiCategoryCombo(EMNCharge* pCharge)
{
	IFormatSettingsPtr pfs(__uuidof(FormatSettings));
	// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
	//templates do not support saving charge categories.. disable the drop down
	long nCategoryCount = m_bIsTemplate ? 0 : pCharge->nCategoryCount;
	// (s.tullis 2015-04-14 12:01) - PLID 65538 - 
	//If an EMN charge has no categories, say (None) for the category. 
	//If an EMN charge has multiple categories, the ‘no selection’ row should say <Select>, 
	//but the column should never color red, because a selection is not required on EMNs.
	CString strComboSource = FormatString("Select CategoriesT.ID AS ID , CategoriesT.Name AS Text "
		"FROM CategoriesT  "
		"LEFT JOIN ServiceMultiCategoryT ON CategoriesT.ID = ServiceMultiCategoryT.CategoryID "
		"Where ServiceMultiCategoryT.ServiceID = %li OR CategoriesT.ID = %li "
		"UNION "
		"SELECT '-1' AS ID, CASE WHEN %li = 1 THEN  '     <Select>' ELSE  '     (None)' END AS Text "
		"ORDER BY Text ASC", pCharge->nServiceID, pCharge->nCategoryID, long(nCategoryCount > 1 ? TRUE: FALSE));

	pfs->PutDataType(VT_I4);
	pfs->PutFieldType(cftComboSimple);
	pfs->PutEditable(nCategoryCount > 1 ? VARIANT_TRUE : VARIANT_FALSE);
	pfs->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
	pfs->EmbeddedComboDropDownMaxHeight = 300;
	pfs->EmbeddedComboDropDownWidth = 200;
	pfs->PutComboSource(_bstr_t(strComboSource));

	return pfs;
}

// (s.tullis 2015-04-01 14:09) - PLID 64978 - Force Show the column if they have multiple categories configured
void CEMRChargePromptDlg::ForceShowColumn(short iColumnIndex, long nPercentWidth, long nPixelWidth)
{
	if (nPercentWidth <= 0 || nPixelWidth <= 0 || nPercentWidth >= 100) {
		//You clearly have missed the point of this function.
		//All callers must provide nonzero numbers in both fields,
		//and percent width cannot be > 100.
		ASSERT(FALSE);
		ThrowNxException("CEmrCodesDlg::ForceShowColumn could not size column %li appropriately.", iColumnIndex);
	}

	if (nPercentWidth >= nPixelWidth) {
		//are you sure you didn't enter these numbers backwards?
		ASSERT(FALSE);
	}

	IColumnSettingsPtr pCol = m_pList->GetColumn(iColumnIndex);
	if (pCol == NULL) {
		//should be impossible
		ThrowNxException("CEmrCodesDlg::ForceShowColumn could not find column index %li.", iColumnIndex);
	}

	if (pCol->GetStoredWidth() == 0) {
		//the column is not shown, so show it

		long nStyle = pCol->ColumnStyle;
		if (nStyle & csWidthPercent) {
			//percent width
			pCol->PutStoredWidth(nPercentWidth);
		}
		else {
			//pixel width
			pCol->PutStoredWidth(nPixelWidth);
		}
	}
}

