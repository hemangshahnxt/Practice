// DrugInteractionSeverityConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DrugInteractionSeverityConfigDlg.h"
#include "PrescriptionUtilsAPI.h"
#include "AuditTrail.h"

// (j.jones 2013-05-10 14:05) - PLID 55955 - created

// CDrugInteractionSeverityConfigDlg dialog

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CDrugInteractionSeverityConfigDlg, CNxDialog)

enum SeverityTreeColumns {
	//no ID/ParentID since this is never requeried
	stcEnum = 0,
	stcName,
	stcPriority,
	stcOldDisplayedValue,
	stcDisplayedCheck,
};

CDrugInteractionSeverityConfigDlg::CDrugInteractionSeverityConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDrugInteractionSeverityConfigDlg::IDD, pParent)
{

}

CDrugInteractionSeverityConfigDlg::~CDrugInteractionSeverityConfigDlg()
{
}

void CDrugInteractionSeverityConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_INTERACTIONS_CONFIG_COLOR, m_bkg);
}


BEGIN_MESSAGE_MAP(CDrugInteractionSeverityConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()


// CDrugInteractionSeverityConfigDlg message handlers

BOOL CDrugInteractionSeverityConfigDlg::OnInitDialog() 
{	
	try {

		CNxDialog::OnInitDialog();
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//A note on ConfigRT settings here: Every interaction type has one, but top priority
		//interactions have their settings completely ignored. This is so that we can always
		//tweak what priorities each interaction type has without having to rewrite this
		//dialog when those changes are made.
		// (j.fouts 2013-05-20 10:17) - PLID 56571 - Added new categories to be more accurate
		g_propManager.CachePropertiesInBulk("CDrugInteractionSeverityConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DISeverityFilter_Allergy_DirectIngredient' "
			"OR Name = 'DISeverityFilter_Allergy_RelatedIngredient' "
			"OR Name = 'DISeverityFilter_Allergy_GroupIngredient_Severe' "
			"OR Name = 'DISeverityFilter_Allergy_GroupIngredient_Moderate' "
			"OR Name = 'DISeverityFilter_Allergy_CrossSensitiveIngredient' "
			"OR Name = 'DISeverityFilter_Drug_ContraindicatedDrugCombination' "
			"OR Name = 'DISeverityFilter_Drug_SevereInteraction' "
			"OR Name = 'DISeverityFilter_Drug_ModerateInteraction' "
			"OR Name = 'DISeverityFilter_Drug_UndeterminedSeverity' "
			"OR Name = 'DISeverityFilter_Diag_AbsoluteContradiction' "
			"OR Name = 'DISeverityFilter_Diag_RelativeContraindication' "
			"OR Name = 'DISeverityFilter_Diag_ContraindicationWarning' "
			")",
			_Q(GetCurrentUserName()));

		//this uses the same color as the interactions dialog, which is always
		//patient's module blue, it doesn't change based on patient status
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_SeverityTree = BindNxDataList2Ctrl(IDC_DRUG_INTERACTION_SEVERITY_TREE, false);

		//drug-allergy interactions
		{
			IRowSettingsPtr pDrugAllergyHeader = m_SeverityTree->GetNewRow(); 
			pDrugAllergyHeader->PutValue(stcEnum, (long)ditDrugAllergy);
			pDrugAllergyHeader->PutValue(stcName, "Drug/Allergy Interactions");
			pDrugAllergyHeader->PutValue(stcPriority, (long)-1);
			pDrugAllergyHeader->PutValue(stcOldDisplayedValue, g_cvarNull);
			pDrugAllergyHeader->PutValue(stcDisplayedCheck, g_cvarNull);
			m_SeverityTree->AddRowAtEnd(pDrugAllergyHeader, NULL);

			//FDBAllergyInteractionSource_DirectIngredient
			{
				DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource_DirectIngredient, NexTech_Accessor::DrugAllergySeverityLevel_Severe);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)daflDirectSevere);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Allergy_DirectIngredient", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugAllergyHeader);
			}

			//FDBAllergyInteractionSource_RelatedIngredient
			{
				DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource_RelatedIngredient, NexTech_Accessor::DrugAllergySeverityLevel_Moderate);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)daflRelatedModerate);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Allergy_RelatedIngredient", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugAllergyHeader);
			}

			// (j.fouts 2013-05-20 10:17) - PLID 56571 - Added new categories to be more accurate
			//FDBAllergyInteractionSource_GroupIngredient_Severe
			{
				DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource_GroupIngredient, NexTech_Accessor::DrugAllergySeverityLevel_Severe);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)daflGroupSevere);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Allergy_GroupIngredient_Severe", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugAllergyHeader);
			}

			//FDBAllergyInteractionSource_GroupIngredient_Moderate
			{
				DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource_GroupIngredient, NexTech_Accessor::DrugAllergySeverityLevel_Moderate);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)daflGroupModerate);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Allergy_GroupIngredient_Moderate", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugAllergyHeader);
			}

			//FDBAllergyInteractionSource_CrossSensitiveIngredient
			{
				DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(NexTech_Accessor::FDBAllergyInteractionSource_CrossSensitiveIngredient, NexTech_Accessor::DrugAllergySeverityLevel_Low);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)daflCrossSensitiveLow);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Allergy_CrossSensitiveIngredient", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugAllergyHeader);
			}
			
			pDrugAllergyHeader->PutExpanded(VARIANT_TRUE);
		}

		//drug-drug interactions

		{
			IRowSettingsPtr pDrugDrugHeader = m_SeverityTree->GetNewRow(); 
			pDrugDrugHeader->PutValue(stcEnum, (long)ditDrugDrug);
			pDrugDrugHeader->PutValue(stcName, "Drug/Drug Interactions");
			pDrugDrugHeader->PutValue(stcPriority, (long)-1);
			pDrugDrugHeader->PutValue(stcOldDisplayedValue, g_cvarNull);
			pDrugDrugHeader->PutValue(stcDisplayedCheck, g_cvarNull);
			m_SeverityTree->AddRowAtEnd(pDrugDrugHeader, NULL);
		
			//FDBDrugInteractionSeverity_ContraindicatedDrugCombination
			{
				DrugInteractionDisplayFields eResult = GetDrugDrugInteractionSeverityInfo(NexTech_Accessor::FDBDrugInteractionSeverity_ContraindicatedDrugCombination);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDrugInteractionSeverity_ContraindicatedDrugCombination);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Drug_ContraindicatedDrugCombination", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDrugHeader);
			}

			//FDBDrugInteractionSeverity_SevereInteraction
			{
				DrugInteractionDisplayFields eResult = GetDrugDrugInteractionSeverityInfo(NexTech_Accessor::FDBDrugInteractionSeverity_SevereInteraction);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDrugInteractionSeverity_SevereInteraction);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Drug_SevereInteraction", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDrugHeader);
			}

			//FDBDrugInteractionSeverity_ModerateInteraction
			{
				DrugInteractionDisplayFields eResult = GetDrugDrugInteractionSeverityInfo(NexTech_Accessor::FDBDrugInteractionSeverity_ModerateInteraction);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDrugInteractionSeverity_ModerateInteraction);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Drug_ModerateInteraction", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDrugHeader);
			}

			//FDBDrugInteractionSeverity_UndeterminedSeverity
			{
				DrugInteractionDisplayFields eResult = GetDrugDrugInteractionSeverityInfo(NexTech_Accessor::FDBDrugInteractionSeverity_UndeterminedSeverity);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDrugInteractionSeverity_UndeterminedSeverity);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Drug_UndeterminedSeverity", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDrugHeader);
			}

			pDrugDrugHeader->PutExpanded(VARIANT_TRUE);
		}

		//drug-diagnosis interactions
		{
			IRowSettingsPtr pDrugDiagnosisHeader = m_SeverityTree->GetNewRow(); 
			pDrugDiagnosisHeader->PutValue(stcEnum, (long)ditDrugDiagnosis);
			pDrugDiagnosisHeader->PutValue(stcName, "Drug/Diagnosis Interactions");
			pDrugDiagnosisHeader->PutValue(stcPriority, (long)-1);
			pDrugDiagnosisHeader->PutValue(stcOldDisplayedValue, g_cvarNull);
			pDrugDiagnosisHeader->PutValue(stcDisplayedCheck, g_cvarNull);
			m_SeverityTree->AddRowAtEnd(pDrugDiagnosisHeader, NULL);

			// (r.gonet 02/28/2014) - PLID 60755 - Fixed spelling of contraindication
			//FDBDiagnosisInteractionSeverity_AbsoluteContraindication
			{
				DrugInteractionDisplayFields eResult = GetDrugDiagnosisInteractionSeverityInfo(NexTech_Accessor::FDBDiagnosisInteractionSeverity_AbsoluteContraindication);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDiagnosisInteractionSeverity_AbsoluteContraindication);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Diag_AbsoluteContradiction", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDiagnosisHeader);
			}

			//FDBDiagnosisInteractionSeverity_RelativeContraindication
			{
				DrugInteractionDisplayFields eResult = GetDrugDiagnosisInteractionSeverityInfo(NexTech_Accessor::FDBDiagnosisInteractionSeverity_RelativeContraindication);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDiagnosisInteractionSeverity_RelativeContraindication);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Diag_RelativeContraindication", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDiagnosisHeader);
			}

			//FDBDiagnosisInteractionSeverity_ContraindicationWarning
			{
				DrugInteractionDisplayFields eResult = GetDrugDiagnosisInteractionSeverityInfo(NexTech_Accessor::FDBDiagnosisInteractionSeverity_ContraindicationWarning);
				IRowSettingsPtr pRow = m_SeverityTree->GetNewRow(); 
				pRow->PutValue(stcEnum, (long)NexTech_Accessor::FDBDiagnosisInteractionSeverity_ContraindicationWarning);
				FillSeverityRow(pRow, eResult, GetRemotePropertyInt("DISeverityFilter_Diag_ContraindicationWarning", 1, 0, GetCurrentUserName(), true) == 1 ? true : false);
				m_SeverityTree->AddRowSorted(pRow, pDrugDiagnosisHeader);
			}

			pDrugDiagnosisHeader->PutExpanded(VARIANT_TRUE);
		}
		
	}NxCatchAll(__FUNCTION__);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//every severity level has columns filled with the contents of a DrugInteractionDisplayFields struct,
//this modular function cuts down on duplicated code
void CDrugInteractionSeverityConfigDlg::FillSeverityRow(NXDATALIST2Lib::IRowSettingsPtr pChildRow, DrugInteractionDisplayFields eResult, bool bChecked)
{
	pChildRow->PutValue(stcName, (LPCTSTR)eResult.strSeverityName);
	pChildRow->PutValue(stcPriority, (long)eResult.ePriority);

	//top priority levels cannot be unchecked,
	//if top priority the bChecked value is ignored
	_variant_t varDisplayed = g_cvarNull;
	if(eResult.ePriority != soTopPriority) {
		if(bChecked) {
			varDisplayed = g_cvarTrue;
		}
		else {
			varDisplayed = g_cvarFalse;
		}
	}
	pChildRow->PutValue(stcOldDisplayedValue, varDisplayed);
	pChildRow->PutValue(stcDisplayedCheck, varDisplayed);

	pChildRow->PutBackColor((long)eResult.eColor);
}

void CDrugInteractionSeverityConfigDlg::OnOK()
{
	try {

		//this code will audit immediately, instead of in a transaction,
		//since changes to ConfigRT will take place immediately, and not in a batch
		long nAuditID = -1;

		IRowSettingsPtr pParentRow = m_SeverityTree->GetFirstRow();
		while (pParentRow) {
			//get the interaction type
			EDrugInteractionType eInterationType = (EDrugInteractionType)VarLong(pParentRow->GetValue(stcEnum));

			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while(pChildRow) {
				//If top priority, do nothing. This means that top priority interactions
				//will always have their ConfigRT settings be ignored.
				long nPriority = VarLong(pChildRow->GetValue(stcPriority), -1);
				if(nPriority == -1 || nPriority == (long)soTopPriority) {
					pChildRow = pChildRow->GetNextRow();
					continue;
				}

				BOOL bOldDisplayed = VarBool(pChildRow->GetValue(stcOldDisplayedValue), TRUE);
				BOOL bNewDisplayed = VarBool(pChildRow->GetValue(stcDisplayedCheck), TRUE);

				//if nothing changed, skip this row
				if(bOldDisplayed == bNewDisplayed) {
					pChildRow = pChildRow->GetNextRow();
					continue;
				}

				//something changed, get the interaction type, save the change, and audit it immediately

				if(eInterationType == ditDrugAllergy) {
					
					// (j.fouts 2013-05-20 10:17) - PLID 56571 - Added new categories to be more accurate
					DrugAllergyFilerLevels eAllergyFilterLevel = (DrugAllergyFilerLevels)VarLong(pChildRow->GetValue(stcEnum));

					switch(eAllergyFilterLevel) {
						case daflDirectSevere:
							SetRemotePropertyInt("DISeverityFilter_Allergy_DirectIngredient", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case daflRelatedModerate:
							SetRemotePropertyInt("DISeverityFilter_Allergy_RelatedIngredient", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case daflGroupSevere:
							SetRemotePropertyInt("DISeverityFilter_Allergy_GroupIngredient_Severe", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case daflGroupModerate:
							SetRemotePropertyInt("DISeverityFilter_Allergy_GroupIngredient_Moderate", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case daflCrossSensitiveLow:
							SetRemotePropertyInt("DISeverityFilter_Allergy_CrossSensitiveIngredient", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
					}
				}
				else if(eInterationType == ditDrugDrug) {

					NexTech_Accessor::FDBDrugInteractionSeverity eDrugInteraction = (NexTech_Accessor::FDBDrugInteractionSeverity)VarLong(pChildRow->GetValue(stcEnum));

					switch (eDrugInteraction) {
						case NexTech_Accessor::FDBDrugInteractionSeverity_ContraindicatedDrugCombination:
							SetRemotePropertyInt("DISeverityFilter_Drug_ContraindicatedDrugCombination", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case NexTech_Accessor::FDBDrugInteractionSeverity_SevereInteraction:
							SetRemotePropertyInt("DISeverityFilter_Drug_SevereInteraction", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case NexTech_Accessor::FDBDrugInteractionSeverity_ModerateInteraction:
							SetRemotePropertyInt("DISeverityFilter_Drug_ModerateInteraction", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case NexTech_Accessor::FDBDrugInteractionSeverity_UndeterminedSeverity:
							SetRemotePropertyInt("DISeverityFilter_Drug_UndeterminedSeverity", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						default:
							//since we built the rows in code, it should be impossible to get this
							ASSERT(FALSE);
							ThrowNxException("Invalid drug child row found!");
							break;
					}
				}
				else if(eInterationType == ditDrugDiagnosis) {

					NexTech_Accessor::FDBDiagnosisInteractionSeverity eDiagInteraction = (NexTech_Accessor::FDBDiagnosisInteractionSeverity)VarLong(pChildRow->GetValue(stcEnum));

					switch (eDiagInteraction) {
						// (r.gonet 02/28/2014) - PLID 60755 - Fixed spelling of contraindication
						case NexTech_Accessor::FDBDiagnosisInteractionSeverity_AbsoluteContraindication:
							SetRemotePropertyInt("DISeverityFilter_Diag_AbsoluteContradiction", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case NexTech_Accessor::FDBDiagnosisInteractionSeverity_RelativeContraindication:
							SetRemotePropertyInt("DISeverityFilter_Diag_RelativeContraindication", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						case NexTech_Accessor::FDBDiagnosisInteractionSeverity_ContraindicationWarning:
							SetRemotePropertyInt("DISeverityFilter_Diag_ContraindicationWarning", bNewDisplayed ? 1 : 0, 0, GetCurrentUserName());
							break;
						default:
							//since we built the rows in code, it should be impossible to get this
							ASSERT(FALSE);
							ThrowNxException("Invalid diagnosis child row found!");
							break;
					}
				}

				//now audit our change
				if(nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}

				CString strTypeName = VarString(pParentRow->GetValue(stcName));
				CString strSeverityName = VarString(pChildRow->GetValue(stcName));
				CString strOldValue, strNewValue;
				strOldValue.Format("%s - %s: %s", strTypeName, strSeverityName, bOldDisplayed ? "Displayed" : "Not Displayed");
				strNewValue.Format("%s - %s: %s", strTypeName, strSeverityName, bNewDisplayed ? "Displayed" : "Not Displayed");
				AuditEvent(-1, GetCurrentUserName(), nAuditID, aeiDrugInteractionSeverityFilter, GetCurrentUserID(), strOldValue, strNewValue, aepHigh, aetChanged);

				pChildRow = pChildRow->GetNextRow();
			}

			pParentRow = pParentRow->GetNextRow();
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}