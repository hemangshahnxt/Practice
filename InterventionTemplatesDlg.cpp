// InterventionTemplatesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InterventionTemplatesDlg.h"
#include "WellnessManager.h"
#include "DecisionSupportManager.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
using namespace Intervention;

//TES 5/19/2009 - PLID 34302 - Created
// CInterventionTemplatesDlg dialog

IMPLEMENT_DYNAMIC(CInterventionTemplatesDlg, CNxDialog)

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
CInterventionTemplatesDlg::CInterventionTemplatesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInterventionTemplatesDlg::IDD, pParent)
{
	m_bRequeryingCriteria = false;
}

CInterventionTemplatesDlg::~CInterventionTemplatesDlg()
{
}

void CInterventionTemplatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_WELLNESS_TEMPLATE, m_nxbAddTemplate);
	DDX_Control(pDX, IDC_DELETE_WELLNESS_TEMPLATE, m_nxbDeleteTemplate);
	DDX_Control(pDX, IDOK, m_nxbClose);
	DDX_Control(pDX, IDC_WELLNESS_TEMPLATE_NAME, m_nxeTemplateName);

	DDX_Control(pDX, IDC_EDIT_DEVELOPER, m_nxeDeveloper);
	DDX_Control(pDX, IDC_EDIT_FUNDING, m_nxeFundingSource);
	DDX_Control(pDX, IDC_STATIC_GUIDELINE, m_nxlGuidline);
	DDX_Control(pDX, IDC_STATIC_REFERENCE_NIFO, m_nxlReference);
	DDX_Control(pDX, IDC_EDIT_CITIATION, m_nxeCitiation);
	DDX_Control(pDX, IDC_EDIT_REFERENCE_INFO, m_nxeRefrenceInfo);
	DDX_Control(pDX, IDC_INACTIVATE_CDS_RULE, m_nxbMarkInactive);
	DDX_Control(pDX, IDC_VIEW_INACTIVE_CDS_RULES, m_nxbViewInactive);
}
	


BEGIN_MESSAGE_MAP(CInterventionTemplatesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_WELLNESS_TEMPLATE, &CInterventionTemplatesDlg::OnAddInterventionTemplate)
	ON_BN_CLICKED(IDC_DELETE_WELLNESS_TEMPLATE, &CInterventionTemplatesDlg::OnDeleteInterventionTemplate)
	ON_EN_KILLFOCUS(IDC_WELLNESS_TEMPLATE_NAME, &CInterventionTemplatesDlg::OnKillfocusInterventionTemplateName)
	ON_BN_CLICKED(IDOK, &CInterventionTemplatesDlg::OnOK)
	ON_EN_KILLFOCUS(IDC_EDIT_FUNDING, &CInterventionTemplatesDlg::OnEnKillfocusEditFunding)
	ON_EN_KILLFOCUS(IDC_EDIT_DEVELOPER, &CInterventionTemplatesDlg::OnEnKillfocusEditDeveloper)
	ON_EN_KILLFOCUS(IDC_EDIT_REFERENCE_INFO, &CInterventionTemplatesDlg::OnEnKillfocusEditReferenceInfo)
	ON_EN_KILLFOCUS(IDC_EDIT_CITIATION, &CInterventionTemplatesDlg::OnEnKillfocusEditCitiation)
	ON_BN_CLICKED(IDC_INACTIVATE_CDS_RULE, &CInterventionTemplatesDlg::OnBnClickedInactivateCdsRule)
	ON_BN_CLICKED(IDC_VIEW_INACTIVE_CDS_RULES, &CInterventionTemplatesDlg::OnBnClickedViewInactiveCdsRules)
END_MESSAGE_MAP()

enum EmrItemListColumns
{
	eilcID = 0,
	eilcType = 1, //TES 6/8/2009 - PLID 34504
	eilcName = 2,
};

enum CompletionItemListColumns
{
	cilcID = 0,
	cilcRecordID = 1,
	cilcRecordType = 2, //TES 6/8/2009 - PLID 34504
	cilcName = 3,
};

enum AvailableCriteriaListColumns
{
	aclcName = 0,
	aclcTypeID = 1,
	aclcType = 2,
	aclcRecordID = 3, //TES 6/8/2009 - PLID 34509 - Renamed
	aclcEmrInfoType = 4,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CInterventionTemplatesDlg message handlers
BOOL CInterventionTemplatesDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbAddTemplate.AutoSet(NXB_NEW);
		m_nxbDeleteTemplate.AutoSet(NXB_DELETE);
		m_nxbMarkInactive.AutoSet(NXB_MODIFY); //TES 11/27/2013 - PLID 59848
		m_nxeDeveloper.SetLimitText(255);
		m_nxeFundingSource.SetLimitText(255);
		m_nxeCitiation.SetLimitText(500);
		m_nxeRefrenceInfo.SetLimitText(4000);
		m_nxeTemplateName.SetLimitText(m_pConfigurationManager->GetNameConstraint());

		m_pReleaseDate =BindNxTimeCtrl(this, IDC_DT_RELEASE_DATE);
		m_pRevisionDate =BindNxTimeCtrl(this, IDC_DT_REVISION_DATE);

		m_pTemplateList = BindNxDataList2Ctrl(IDC_WELLNESS_TEMPLATE_LIST, false);
		m_pEmrItemList = BindNxDataList2Ctrl(IDC_EMR_ITEM_LIST, m_pConfigurationManager->ShowCompletionList() ? true : false);
		m_pAvailableCriteriaList = BindNxDataList2Ctrl(IDC_AVAILABLE_CRITERIA_LIST, false);
		m_pCompletionItemList = BindNxDataList2Ctrl(IDC_COMPLETION_ITEMS, false);
		m_pCriteriaList = BindNxDataList2Ctrl(IDC_TEMPLATE_CRITERIA, false);

		m_reGuidelines = GetDlgItem(IDC_GUIDELINES)->GetControlUnknown();
		m_reReferenceMaterials = GetDlgItem(IDC_REFERENCE_MATERIALS)->GetControlUnknown();

		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		m_pTemplateList->FromClause = _bstr_t(m_pConfigurationManager->GetTemplateListFromClause());
		m_pTemplateList->WhereClause = _bstr_t(m_pConfigurationManager->GetTemplateListWhereClause());
		m_pAvailableCriteriaList->FromClause = _bstr_t(m_pConfigurationManager->GetCriteriaListFromClause());
		m_pAvailableCriteriaList->WhereClause = _bstr_t(m_pConfigurationManager->GetCriteriaListWhereClause());
		m_pCriteriaList->FromClause = _bstr_t(m_pConfigurationManager->GetTemplateCriteriaFromClause());

		m_pTemplateList->Requery();
		m_pAvailableCriteriaList->Requery();
		
		SetWindowText(m_pConfigurationManager->GetDlgName());

		if(!m_pConfigurationManager->ShowCompletionList()){
			HideCompletionList();
			m_nxlGuidline.SetText("Citation"); 
			m_nxlReference.SetText("Reference Information"); 
		}
		else{
			// (s.dhole 2013-10-31 16:12) - PLID 
			HideDesisionExtraFields();
			m_nxlGuidline.SetText("Guidelines"); 
		}
		if(!m_pConfigurationManager->ShowLastXDays())
		{
			HideLastXDays();
		}

		//TES 11/10/2013 - PLID 59400 - Disable buttons based on permissions (if they don't have write permission, they can't get to this dialog)
		m_nxbAddTemplate.EnableWindow(GetCurrentUserPermissions(bioCDSRules) & (sptCreate|sptCreateWithPass));
		m_nxbDeleteTemplate.EnableWindow(GetCurrentUserPermissions(bioCDSRules) & (sptDelete|sptDeleteWithPass));
		//TES 11/27/2013 - PLID 59848 - Added Mark Inactive and View Inactive buttons
		m_nxbViewInactive.EnableWindow(GetCurrentUserPermissions(bioCDSRules) & (sptCreate|sptCreateWithPass));
		m_nxbMarkInactive.EnableWindow(GetCurrentUserPermissions(bioCDSRules) & (sptDelete|sptDeleteWithPass));

		Refresh();

	}NxCatchAll("Error in CInterventionTemplateDlg::OnInitDialog()");

	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CInterventionTemplatesDlg::HideCompletionList()
{
	GetDlgItem(IDC_COMPLETION_LIST_LABEL)->ShowWindow(SW_HIDE);
	m_pEmrItemList->Enabled = VARIANT_FALSE;
	GetDlgItem(IDC_EMR_ITEM_LIST)->ShowWindow(SW_HIDE);
	m_pCompletionItemList->Enabled = VARIANT_FALSE;
	GetDlgItem(IDC_COMPLETION_ITEMS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GUIDELINES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_REFERENCE_MATERIALS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_REFERENCE)->ShowWindow(SW_HIDE);
	
}
// (s.dhole 2013-10-31 16:12) - PLID 
void CInterventionTemplatesDlg::HideDesisionExtraFields()
{
	GetDlgItem(IDC_STATIC_DEVELOPER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_DEVELOPER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_FUNDING)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_FUNDING)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_RELEASE_DATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DT_RELEASE_DATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_REVISION_DATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DT_REVISION_DATE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_CITIATION)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_REFERENCE_INFO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_REFERENCE_NIFO)->ShowWindow(SW_HIDE);

	//TES 11/27/2013 - PLID 59848 - Only CDS Rules can be marked Inactive
	GetDlgItem(IDC_INACTIVATE_CDS_RULE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_VIEW_INACTIVE_CDS_RULES)->ShowWindow(SW_HIDE);
		

}


void CInterventionTemplatesDlg::OnAddInterventionTemplate()
{
	try {
		//TES 11/10/2013 - PLID 59400 - Check permissions
		if(!CheckCurrentUserPermissions(bioCDSRules, sptCreate)) {
			return;
		}
		//TES 5/22/2009 - PLID 34302 - Make sure the current template is valid before letting them add a new one.
		if(!ValidateCurrentTemplate()) {
			return;
		}

		IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetNewTemplate();

		if(pTemplate != NULL)
		{
			//TES 5/22/2009 - PLID 34302 - Add to interface
			IRowSettingsPtr pRow;
			pRow = m_pTemplateList->GetNewRow();
			pTemplate->LoadIntoRow(pRow);
			m_pTemplateList->AddRowSorted(pRow, NULL);
			SelectTemplateRow(pRow);

			//TES 5/22/2009 - PLID 34302 - Reflect the new template in the display
			Refresh();
		}

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnAddInterventionTemplate()");
}

void CInterventionTemplatesDlg::Refresh()
{
	IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();

	IRowSettingsPtr pRow = m_pTemplateList->CurSel;
	if(pRow == NULL || pTemplate == NULL) {
		//TES 5/22/2009 - PLID 34302 - Try selecting the first row.
		m_pTemplateList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		SelectTemplateRow(m_pTemplateList->GetFirstRow());
		pTemplate = m_pConfigurationManager->GetSelectedTemplate();
		pRow = m_pTemplateList->CurSel;
		if(pRow == NULL || pTemplate == NULL) {
			//TES 5/22/2009 - PLID 34302 - No rows, clear the display
			m_nxeTemplateName.SetWindowText("");
			m_nxeTemplateName.EnableWindow(FALSE);
			GetDlgItem(IDC_AVAILABLE_CRITERIA_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_EMR_ITEM_LIST)->EnableWindow(FALSE);
			m_pCriteriaList->Clear();
			GetDlgItem(IDC_TEMPLATE_CRITERIA)->EnableWindow(FALSE);
			m_pCompletionItemList->Clear();
			GetDlgItem(IDC_COMPLETION_ITEMS)->EnableWindow(FALSE);
			m_reGuidelines->RichText = _bstr_t("");
			GetDlgItem(IDC_GUIDELINES)->EnableWindow(FALSE);
			m_reReferenceMaterials->RichText = _bstr_t("");
			GetDlgItem(IDC_REFERENCE_MATERIALS)->EnableWindow(FALSE);
			return;
		}
	}

	//TES 5/22/2009 - PLID 34302 - Enable everything that might be disabled.
	m_nxeTemplateName.EnableWindow(TRUE);
	GetDlgItem(IDC_AVAILABLE_CRITERIA_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_EMR_ITEM_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_TEMPLATE_CRITERIA)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMPLETION_ITEMS)->EnableWindow(TRUE);
	GetDlgItem(IDC_GUIDELINES)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFERENCE_MATERIALS)->EnableWindow(TRUE);

	m_nxeTemplateName.SetWindowText(pTemplate->GetName());
	m_reReferenceMaterials->RichText = _bstr_t(pTemplate->GetMaterials());
	m_reGuidelines->RichText = _bstr_t(pTemplate->GetGuidelines());

	if(m_pConfigurationManager->ShowCompletionList())
	{
		//TES 5/22/2009 - PLID 34302 - Requery the completion items.
		m_pCompletionItemList->WhereClause = _bstr_t(m_pConfigurationManager->GetCompletionListWhereClause());
		m_pCompletionItemList->Requery();
	}
	else{
		SetDlgItemText(IDC_EDIT_CITIATION, pTemplate->GetGuidelines());
		SetDlgItemText(IDC_EDIT_REFERENCE_INFO, pTemplate->GetMaterials());
		SetDlgItemText(IDC_EDIT_DEVELOPER, pTemplate->GetDeveloper());
		SetDlgItemText(IDC_EDIT_FUNDING, pTemplate->GetFundingInfo());
		_variant_t  varDate=GetVariantDate(pTemplate->GetReleaseDate());
		if(varDate.vt == VT_DATE) {
			m_pReleaseDate->SetDateTime(varDate );
		}
		else{
			m_pReleaseDate->Clear(); 
		}
		varDate=GetVariantDate(pTemplate->GetRevisionDate());
		if(varDate.vt == VT_DATE) {
			m_pRevisionDate->SetDateTime(varDate );
		}
		else{
			m_pRevisionDate->Clear(); 
		}

		
	}

	//TES 5/22/2009 - PLID 34302 - Requery the criteria
	m_pCriteriaList->WhereClause = _bstr_t(m_pConfigurationManager->GetTemplateCriteriaWhereClause());
	m_pCriteriaList->Requery();

	//TES 5/26/2009 - PLID 34302 - Make sure nobody edits until we're done requerying and updating the DisplayValue
	m_bRequeryingCriteria = true;
}

BEGIN_EVENTSINK_MAP(CInterventionTemplatesDlg, CNxDialog)
ON_EVENT(CInterventionTemplatesDlg, IDC_WELLNESS_TEMPLATE_LIST, 16, CInterventionTemplatesDlg::OnSelChosenInterventionTemplateList, VTS_DISPATCH)
ON_EVENT(CInterventionTemplatesDlg, IDC_EMR_ITEM_LIST, 16, CInterventionTemplatesDlg::OnSelChosenEmrItemList, VTS_DISPATCH)
ON_EVENT(CInterventionTemplatesDlg, IDC_COMPLETION_ITEMS, 6, CInterventionTemplatesDlg::OnRButtonDownCompletionItems, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CInterventionTemplatesDlg, IDC_AVAILABLE_CRITERIA_LIST, 16, CInterventionTemplatesDlg::OnSelChosenAvailableCriteriaList, VTS_DISPATCH)
ON_EVENT(CInterventionTemplatesDlg, IDC_TEMPLATE_CRITERIA, 18, CInterventionTemplatesDlg::OnRequeryFinishedTemplateCriteria, VTS_I2)
ON_EVENT(CInterventionTemplatesDlg, IDC_TEMPLATE_CRITERIA, 9, CInterventionTemplatesDlg::OnEditingFinishingTemplateCriteria, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
ON_EVENT(CInterventionTemplatesDlg, IDC_TEMPLATE_CRITERIA, 10, CInterventionTemplatesDlg::OnEditingFinishedTemplateCriteria, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CInterventionTemplatesDlg, IDC_TEMPLATE_CRITERIA, 6, CInterventionTemplatesDlg::OnRButtonDownTemplateCriteria, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CInterventionTemplatesDlg, IDC_WELLNESS_TEMPLATE_LIST, 1, CInterventionTemplatesDlg::OnSelChangingInterventionTemplateList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CInterventionTemplatesDlg, IDC_TEMPLATE_CRITERIA, 8, CInterventionTemplatesDlg::OnEditingStartingTemplateCriteria, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CInterventionTemplatesDlg, IDC_GUIDELINES, 3, CInterventionTemplatesDlg::OnKillFocusGuidelines, VTS_NONE)
ON_EVENT(CInterventionTemplatesDlg, IDC_REFERENCE_MATERIALS, 3, CInterventionTemplatesDlg::OnKillFocusReferenceMaterials, VTS_NONE)
ON_EVENT(CInterventionTemplatesDlg, IDC_TEMPLATE_CRITERIA, 19, CInterventionTemplatesDlg::OnLeftClickTemplateCriteria, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CInterventionTemplatesDlg, IDC_DT_RELEASE_DATE, 1, CInterventionTemplatesDlg::KillFocusDtReleaseDate, VTS_NONE)
ON_EVENT(CInterventionTemplatesDlg, IDC_DT_REVISION_DATE, 1, CInterventionTemplatesDlg::KillFocusDtRevisionDate, VTS_NONE)
END_EVENTSINK_MAP()

void CInterventionTemplatesDlg::OnSelChosenInterventionTemplateList(LPDISPATCH lpRow)
{
	try {
		SelectTemplateRow(IRowSettingsPtr(lpRow));

		//TES 5/22/2009 - PLID 34302 - Update the screen.
		Refresh();
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnSelChosenInterventionTemplateList()");
}

void CInterventionTemplatesDlg::OnDeleteInterventionTemplate()
{
	try {
		//TES 11/10/2013 - PLID 59400 - Check permissions
		if(!CheckCurrentUserPermissions(bioCDSRules, sptDelete)) {
			return;
		}
		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		if(m_pConfigurationManager->DeleteSelectedTemplate())
		{
			//TES 5/22/2009 - PLID 34302 - Remove from the interface
			m_pTemplateList->RemoveRow(m_pTemplateList->CurSel);
			SelectTemplateRow(NULL);
			
			//TES 5/22/2009 - PLID 34302 - Display whatever template is selected now.
			Refresh();
		}
		
	}NxCatchAll("CInterventionTemplatesDlg::OnDeleteInterventionTemplate()");
}

void CInterventionTemplatesDlg::OnKillfocusInterventionTemplateName()
{
	try {

		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		IRowSettingsPtr pRow = m_pTemplateList->CurSel;
		IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();

		if(pRow == NULL || pTemplate == NULL) {
			return;
		}

		CString strNewName;
		GetDlgItemText(IDC_WELLNESS_TEMPLATE_NAME, strNewName);
		if(pTemplate->SetName(strNewName))
		{
			//TES 5/22/2009 - PLID 34302 - Update the interface.
			pRow->PutValue(ilcName, _bstr_t(strNewName));
		}
		else
		{
			SetDlgItemText(IDC_WELLNESS_TEMPLATE_NAME, VarString(pRow->GetValue(ilcName)));
		}

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnKillfocusInterventionTemplateName()");
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
void CInterventionTemplatesDlg::OnSelChosenEmrItemList(LPDISPATCH lpRow)
{
	try {
		if(!m_pConfigurationManager->ShowCompletionList())
		{
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		IRowSettingsPtr pTemplateRow = m_pTemplateList->CurSel;
		IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
		if(pTemplateRow == NULL || pTemplate == NULL) {
			return;
		}

		//TES 5/22/2009 - PLID 34302 - Make sure this item isn't in our list already.
		long nNewRecordID = VarLong(pRow->GetValue(eilcID));
		//TES 6/8/2009 - PLID 34504 - Check the type as well
		BYTE nNewRecordType = VarByte(pRow->GetValue(eilcType));
		IRowSettingsPtr p = m_pCompletionItemList->GetFirstRow();
		bool bFound = false;
		while(p && !bFound) {
			if(VarLong(p->GetValue(cilcRecordID)) == nNewRecordID
				&& VarByte(p->GetValue(cilcRecordType)) == nNewRecordType) {
				bFound = true;
			}
			p = p->GetNextRow();
		}

		if(bFound) {
			MsgBox("This item is already in the list of items to be completed for this template.");
			return;
		}

		long nNewID = pTemplate->AddCompletionItem(nNewRecordID, nNewRecordType);
		if(nNewID > 0)
		{
			//TES 5/22/2009 - PLID 34302 - Add to our list
			IRowSettingsPtr pNewRow = m_pCompletionItemList->GetNewRow();
			pNewRow->PutValue(cilcID, nNewID);
			pNewRow->PutValue(cilcRecordID, nNewRecordID);
			pNewRow->PutValue(cilcRecordType, nNewRecordType);
			pNewRow->PutValue(cilcName, pRow->GetValue(eilcName));
			m_pCompletionItemList->AddRowSorted(pNewRow, NULL);
		}

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnSelChosenEmrItemList()");
}

void CInterventionTemplatesDlg::OnRButtonDownCompletionItems(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if(!m_pConfigurationManager->ShowCompletionList())
		{
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		m_pCompletionItemList->CurSel = pRow;
		if(pRow == NULL) {
			return;
		}

		//TES 5/22/2009 - PLID 34302 - Give them the option to delete this item
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED, 1, "Remove");
		CPoint ptClicked(x, y);
		GetDlgItem(IDC_COMPLETION_ITEMS)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
		if(nResult == 1) {
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently remove this completion item?  This action cannot be undone!")) {
				return;
			}

			if(m_pConfigurationManager->RemoveCompletionItem(VarLong(pRow->GetValue(cilcID))))
			{
				//TES 5/22/2009 - PLID 34302 - Remove from interface
				m_pCompletionItemList->RemoveRow(pRow);
			}	
		}
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnRButtonDownCompletionItems()");
}

void CInterventionTemplatesDlg::OnSelChosenAvailableCriteriaList(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		IRowSettingsPtr pTemplateRow = m_pTemplateList->CurSel;
		IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
		if(pTemplateRow == NULL || pTemplate == NULL) {
			return;
		}

		IInterventionCriterionPtr pCriterion = pTemplate->AddCriterionFromRow(lpRow);
		
		if(pCriterion != NULL)
		{
			//TES 5/22/2009 - PLID 34302 - Add to our list
			IRowSettingsPtr pNewRow = m_pCriteriaList->GetNewRow();
			pCriterion->LoadIntoRow(pNewRow);
			m_pCriteriaList->AddRowSorted(pNewRow, NULL);
			m_pCriteriaList->CurSel = pNewRow;

			GetDlgItem(IDC_TEMPLATE_CRITERIA)->SetFocus();
			//TES 5/22/2009 - PLID 34302 - Start them editing the row.
			m_pCriteriaList->StartEditing(pNewRow, clcOperator);
		}

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnSelChosenAvailableCriteriaList()");
}

void CInterventionTemplatesDlg::OnRequeryFinishedTemplateCriteria(short nFlags)
{
	try {
		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		//TES 5/22/2009 - PLID 34302 - Need to go through and set the appropriate operators to be available for each row.
		IRowSettingsPtr pRow = m_pCriteriaList->GetFirstRow();
		while(pRow) {

			IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);
			pCriterion->LoadIntoRow(pRow);
				
			pRow = pRow->GetNextRow();
		}
		//TES 5/26/2009 - PLID 34302 - They can edit the list now.
		m_bRequeryingCriteria = false;
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnRequeryFinishedTemplateCriteria()");
}

void CInterventionTemplatesDlg::OnEditingFinishingTemplateCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		switch(nCol) {
			case clcLastXDays:
				{
					// (s.tullis 2015-05-19 14:53) - PLID 61879 - Disallow anything less less than 1
					//TES 5/22/2009 - PLID 34302 - Force a positive integer.
					long nValue = atol(strUserEntered);
					if(nValue <= 0) {
						*pvarNewValue = g_cvarNull;
						*pbCommit = FALSE;
					}
					else if(nValue > 36500) {
						MsgBox("You cannot enter a value in the 'Last X Days' column greater than 100 years");
						*pbCommit = FALSE;
					}
					else {
						*pvarNewValue = _variant_t(nValue);
					}
				}
				break;
			case clcDisplayValue:
				{
					// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(lpRow);
					if(!pCriterion->ValidateValue(CString(strUserEntered), pvarNewValue))
					{
						*pbCommit = FALSE;
					}
				}
				break;
		}
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnEditingFinishingTemplateCriteria()");
}

void CInterventionTemplatesDlg::OnEditingFinishedTemplateCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		if(!bCommit) {
			return;
		}
		
		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		CString strField, strValue;
		switch(nCol) 
		{
			case clcOperator:
				{
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(lpRow);
					pCriterion->Operator = (InterventionCriteriaOperator)VarByte(varNewValue);

					pRow->PutValue(clcDisplayValue, _bstr_t(pCriterion->DisplayValue));
					pRow->PutValue(clcValue, _bstr_t(pCriterion->Value));

					//TES 6/2/2009 - PLID 34302 - For some reason the datalist wasn't redrawing itself here, so make
					// sure that it does.
					m_pCriteriaList->SetRedraw(TRUE);
				}
				break;

			case clcDisplayValue:
				{
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(lpRow);
					pCriterion->DisplayValue = AsString(varNewValue);

					//TES 5/22/2009 - PLID 34302 - Copy the value to the "real" value field.
					//TES 7/14/2009 - PLID 34302 - Need to make sure it's a string; the PatientWellnessAlertDlg has
					// always done this, I'm not quite sure how this code wasn't.
					pRow->PutValue(clcValue, _bstr_t(pCriterion->Value));
				}
				break;

			case clcLastXDays:
				{
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(lpRow);
					pCriterion->LastXDays = AsLong(varNewValue);
				}
				break;
		}
				
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnEditingFinishedTemplateCriteria()");
}

void CInterventionTemplatesDlg::OnRButtonDownTemplateCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		m_pCriteriaList->CurSel = pRow;
		if(pRow == NULL) {
			return;
		}

		//TES 5/22/2009 - PLID 34302 - Give them the option to remove this criterion.
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED, 1, "Remove");
		CPoint ptClicked(x, y);
		GetDlgItem(IDC_TEMPLATE_CRITERIA)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
		if(nResult == 1) {
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently remove this criterion?  This action cannot be undone!")) {
				return;
			}
			
			// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
			IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);
			if(pCriterion->Delete())
			{
				//TES 5/22/2009 - PLID 34302 - Remove from the interface
				m_pCriteriaList->RemoveRow(pRow);
			}
		}
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnRButtonDownTemplateCriteria()");
}

void CInterventionTemplatesDlg::OnSelChangingInterventionTemplateList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 5/22/2009 - PLID 34302 - Don't let them change if the current template is invalid.
		if(!ValidateCurrentTemplate()) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnSelChangingInterventionTemplateList()");
}

void CInterventionTemplatesDlg::OnOK()
{
	try {
		//TES 5/22/2009 - PLID 34302 - Don't let them close if the current template is invalid.
		if(!ValidateCurrentTemplate()) {
			return;
		}
		CNxDialog::OnOK();
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnOK()");
}

bool CInterventionTemplatesDlg::ValidateCurrentTemplate()
{
	IRowSettingsPtr pTemplateRow = m_pTemplateList->CurSel;
	if(pTemplateRow == NULL) {
		return true;
	}

	//TES 5/22/2009 - PLID 34302 - Are there any criteria?
	if(m_pCriteriaList->GetRowCount() == 0) {
		MsgBox("The currently selected template does not have any criteria selected.  Please add one or more criteria to this template.");
		return false;
	}

	//TES 5/22/2009 - PLID 34302 - Validate that each criteria has a value, unless the operator is Filled In/Not Filled In.
	IRowSettingsPtr pRow = m_pCriteriaList->GetFirstRow();
	while(pRow) {

		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);
		if(!pCriterion->IsValid())
		{
			MsgBox("The criterion for %s does not have a value entered. Please enter a value for this criterion to be compared to.", 
				pCriterion->Name);
			return false;
		}

		pRow = pRow->GetNextRow();
	}

	return true;
}

void CInterventionTemplatesDlg::OnEditingStartingTemplateCriteria(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//TES 5/26/2009 - PLID 34302 - First off, if we're still requerying this list, don't let them do anything.
		if(m_bRequeryingCriteria) {
			*pbContinue = false;
			return;
		}

		switch(nCol) {
			case clcDisplayValue:
				{
					// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);
					if(!pCriterion->IsValueEnabled())
					{
						*pbContinue = false;
					}
				}
				break;
			case clcLastXDays:
				{
					// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);
					if(!pCriterion->IsLastXDaysEnabled())
					{
						*pbContinue = false;
					}
				}
				break;
		}

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnEditingStartingTemplateCriteria()");
}

void CInterventionTemplatesDlg::OnKillFocusGuidelines()
{
	try {
		//TES 5/21/2009 - PLID 34302 - Sometimes this function gets called before OnInitDialog()
		if(m_pTemplateList == NULL) {
			return;
		}

		IRowSettingsPtr pRow = m_pTemplateList->CurSel;
		IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
		if(pRow == NULL || pTemplate == NULL) {
			return;
		}

		//TES 5/22/2009 - PLID 34302 - Put the new text in data.
		long nID = VarLong(pRow->GetValue(ilcID));
		CString strNew = (LPCTSTR)m_reGuidelines->RichText;

		pTemplate->SetGuidelines(strNew);

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnKillFocusGuidelines()");
}

void CInterventionTemplatesDlg::OnKillFocusReferenceMaterials()
{
	try {
		//TES 5/21/2009 - PLID 34302 - Sometimes this function gets called before OnInitDialog()
		if(m_pTemplateList == NULL) {
			return;
		}

		IRowSettingsPtr pRow = m_pTemplateList->CurSel;
		IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
		if(pRow == NULL || pTemplate == NULL) {
			return;
		}

		//TES 5/22/2009 - PLID 34302 - Put the new text in data.
		CString strNew = (LPCTSTR)m_reReferenceMaterials->RichText;

		pTemplate->SetMaterials(strNew);

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnKillFocusReferenceMaterials()");
}

void CInterventionTemplatesDlg::OnLeftClickTemplateCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
			case clcDisplayValue:
				{
					// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
					IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);

					if(pCriterion->Operator == icoFilledIn || 
						pCriterion->Operator == icoNotFilledIn || 
						pCriterion->Operator == icoExists || 
						pCriterion->Operator == icoDoesNotExist) {
						//TES 6/2/2009 - PLID 34302 - They can't change the value for this operator.
						return;
					}

					pCriterion->OnClickValue();
					pCriterion->LoadIntoRow(pRow);
				}
				break;
		}

	}NxCatchAll("Error in CInterventionTemplatesDlg::OnLeftClickTemplateCriteria()");
}

void CInterventionTemplatesDlg::OnCancel()
{
	try {
		//TES 6/2/2009 - PLID 34302 - Don't let them close if the current template is invalid.
		if(!ValidateCurrentTemplate()) {
			return;
		}

		CNxDialog::OnCancel();
	}NxCatchAll("Error in CInterventionTemplatesDlg::OnCancel()");
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
int CInterventionTemplatesDlg::DoModal()
{
	return CNxDialog::DoModal();
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
int CInterventionTemplatesDlg::OpenWellnessConfiguration()
{
	m_pConfigurationManager.reset(new WellnessManager());
	return this->DoModal();
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
int CInterventionTemplatesDlg::OpenDecisionSupportConfiguration()
{
	m_pConfigurationManager.reset(new DecisionSupportManager());
	return this->DoModal();
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
void CInterventionTemplatesDlg::SelectTemplateRow(IRowSettingsPtr pRow)
{
	IInterventionTemplatePtr pTemplate;
	pTemplate.reset();

	if(pRow != NULL)
	{
		long nTemplateID = VarLong(pRow->GetValue(ilcID));
		CString strTemplateName = VarString(pRow->GetValue(ilcName));

		pTemplate = m_pConfigurationManager->GetTemplatePtr(strTemplateName, nTemplateID);
		m_pConfigurationManager->SetSelectedTemplate(pTemplate);
	}

	m_pTemplateList->CurSel = pRow;
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
void CInterventionTemplatesDlg::HideLastXDays()
{
	m_pCriteriaList->GetColumn(clcLastXDays)->PutColumnStyle(csVisible|csFixedWidth);
	m_pCriteriaList->GetColumn(clcLastXDays)->PutStoredWidth(0);
}
void CInterventionTemplatesDlg::OnEnKillfocusEditFunding()
{
try {
		long nID = GetTempletID();
		if (nID>-1){
			CString strNew ;
			GetDlgItemText(IDC_EDIT_FUNDING, strNew);
			IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
			pTemplate->SetFundingInfo(strNew);
		}

	}NxCatchAll(__FUNCTION__);
}

void CInterventionTemplatesDlg::KillFocusDtReleaseDate()
{
try {

		long nID = GetTempletID();
		if (nID>-1){
			COleDateTime dtDate =m_pReleaseDate->GetDateTime();
			IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
			pTemplate->SetReleaseDate(dtDate);
			_variant_t  varDate=GetVariantDate(pTemplate->GetReleaseDate());
			if(varDate.vt == VT_DATE) {
				m_pReleaseDate->SetDateTime(varDate );
			}
			else{
				m_pReleaseDate->Clear(); 
			}
			
		}

	}NxCatchAll(__FUNCTION__);
}

void CInterventionTemplatesDlg::KillFocusDtRevisionDate()
{
try {
		long nID = GetTempletID();
		if (nID>-1){
			COleDateTime dtDate =m_pRevisionDate->GetDateTime();
			IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
			pTemplate->SetRevisionDate(dtDate);
			_variant_t  varDate=GetVariantDate(pTemplate->GetRevisionDate());
			if(varDate.vt == VT_DATE) {
				m_pRevisionDate->SetDateTime(varDate );
			}
			else{
				m_pRevisionDate->Clear(); 
			}
		}


	}NxCatchAll(__FUNCTION__);
}

void CInterventionTemplatesDlg::OnEnKillfocusEditDeveloper()
{
try {
		long nID = GetTempletID();
		if (nID>-1){
			CString strNew ;
			GetDlgItemText(IDC_EDIT_DEVELOPER, strNew);
			IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
			pTemplate->SetDeveloper(strNew);
		}

	}NxCatchAll(__FUNCTION__);

}

void CInterventionTemplatesDlg::OnEnKillfocusEditReferenceInfo()
{
try {
		long nID = GetTempletID();
		if (nID>-1){
			CString strNew ;
			GetDlgItemText(IDC_EDIT_REFERENCE_INFO, strNew);
			IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
			pTemplate->SetMaterials(strNew);
		}

	}NxCatchAll(__FUNCTION__);
}

void CInterventionTemplatesDlg::OnEnKillfocusEditCitiation()
{
try {
		long nID = GetTempletID();
		if (nID>-1){
			CString strNew ;
			GetDlgItemText(IDC_EDIT_CITIATION, strNew);
			IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
			pTemplate->SetGuidelines(strNew);
		}
	}NxCatchAll(__FUNCTION__);
}

long CInterventionTemplatesDlg::GetTempletID()
{
	long nID=-1;
	if(m_pTemplateList == NULL) {
		return nID;
	}
	IRowSettingsPtr pRow = m_pTemplateList->CurSel;
	IInterventionTemplatePtr pTemplate = m_pConfigurationManager->GetSelectedTemplate();
	if(pRow == NULL || pTemplate == NULL) {
		return nID;
	}
	nID = VarLong(pRow->GetValue(ilcID));
	return nID ;
}

_variant_t CInterventionTemplatesDlg::GetVariantDate(COleDateTime Dt)
{
	_variant_t  varDate=g_cvarNull ;
	COleDateTime  dttemp;
	dttemp.ParseDateTime("01/01/1800");
	if (Dt.GetStatus() == COleDateTime::valid && Dt.GetStatus() != COleDateTime::null && Dt!=NULL  &&  Dt.m_dt > dttemp.m_dt ){
			varDate =COleVariant(Dt);
	}
	return varDate;
}

void CInterventionTemplatesDlg::OnBnClickedInactivateCdsRule()
{
	try {
		//TES 11/27/2013 - PLID 59848 - Added the ability to mark rules Inactive
		if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to mark this rule as Inactive? Inactive rules will not create any new interventions, though existing interventions will still appear.")) {
			ExecuteParamSql("UPDATE DecisionRulesT SET Inactive = 1 WHERE ID = {INT}", GetTempletID());
			
			m_pTemplateList->RemoveRow(m_pTemplateList->CurSel);
			SelectTemplateRow(NULL);
			Refresh();
		}
	}NxCatchAll(__FUNCTION__);
}

void CInterventionTemplatesDlg::OnBnClickedViewInactiveCdsRules()
{
	try {
		//TES 11/27/2013 - PLID 59848 - Open up a dialog with inactive rules, let them reactivate them if they wish.
		CMultiSelectDlg dlg(this, "InactiveDecisionRulesT");
		if(IDOK == dlg.Open("DecisionRulesT", "Inactive = 1", "ID", "Name", "This dialog shows all Clinical Decision Support Rules which have been marked as inactive. You may select one or more to re-activate; those rules will then be checked when triggering interventions for patients.")) {
			CArray<long,long> arIDs;
			dlg.FillArrayWithIDs(arIDs);
			ExecuteParamSql("UPDATE DecisionRulesT SET Inactive = 0 WHERE ID IN ({INTARRAY})", arIDs);
			m_pTemplateList->Requery();
			SelectTemplateRow(NULL);
			Refresh();
		}
	}NxCatchAll(__FUNCTION__);
}
