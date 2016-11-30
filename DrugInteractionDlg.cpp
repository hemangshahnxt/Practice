// DrugInteractionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DrugInteractionDlg.h"
#include "DrugInteractionSeverityConfigDlg.h"
#include "PrescriptionUtilsAPI.h"
#include "MonographDlg.h"

// (j.fouts 2012-08-15 13:07) - PLID 52145 - Added

//Columns in the datalist
enum EDrugInteractionList{ 
	dilMedID = 0, 
	dilInteractID, 
	dilTypeID, 
	dilSeverityOrderEnum, 
	dilFDBSeverityEnum, 
	dilMed, 
	dilInteract, 
	dilType,
	dilSeverityLevelName,
	dilDetail, 
	dilMsg,
	dilMonograph, //TES 11/10/2013 - PLID 59399
	dilMonographText, //TES 11/10/2013 - PLID 59399
};

// CDrugInteractionDlg dialog

IMPLEMENT_DYNAMIC(CDrugInteractionDlg, CNxDialog)

// (j.fouts 2012-09-06 08:34) - PLID 52482 - Added the string paramter to save and restore location.
// (b.savon 2012-11-30 10:30) - PLID 53773 - Pass in the parent enum
CDrugInteractionDlg::CDrugInteractionDlg(CWnd* pParent /*=NULL*/, EInteractionParent eipParent /*=eipMedications*/)
	: CNxDialog(CDrugInteractionDlg::IDD, pParent, "DrugInteractionDlg")
{
	m_nCurrentPatientID = -1;
	m_bInteractionsChanged = true;

	// (b.savon 2012-11-30 08:30) - PLID 53773 - Save the parent.  Since this dialog is held in memory and can be
	// accessed from either EMR, Medications tab, or Queue (both different copies of the dialog) we will need to store 3
	// different size keys.  If we dont, we get in a situation where whichever dialog is destroyed last will have
	// its size saved and upon opening Practice next, both dialogs will take on that same size.
	m_eipParent = eipParent;

	// (j.jones 2013-05-14 17:06) - PLID 56634 - Added ability to toggle filtering on and off.
	m_bShowFilteredInteractions = true;
}

CDrugInteractionDlg::~CDrugInteractionDlg()
{
}

void CDrugInteractionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INTERACTIONS_COLOR, m_bkg);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_SEVERITY_FILTERS, m_btnConfigSeverityFilters);
	DDX_Control(pDX, IDC_BTN_TOGGLE_INTERACTION_FILTER, m_btnToggleFilter);
	DDX_Control(pDX, IDC_LABEL_INTERACTION_COUNT, m_nxstaticLabelInteractionCount);
}


BEGIN_MESSAGE_MAP(CDrugInteractionDlg, CNxDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_SEVERITY_FILTERS, OnBtnConfigureSeverityFilters)
	ON_BN_CLICKED(IDC_BTN_TOGGLE_INTERACTION_FILTER, OnBtnToggleInteractionFilter)
END_MESSAGE_MAP()


BOOL CDrugInteractionDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try
	{
		// (b.savon 2012-11-29 16:50) - PLID 53773 - Setup
		EnableDragHandle();
		SetMinSize(630, 360);
		SetMaxSize(1024, 768);
		LoadWindowSize();

		//Set our icons and colors
		SetTitleBarIcon(IDI_DRUG_INTERACTIONS);

		// (j.jones 2013-05-13 17:29) - PLID 56634 - Supported hiding less-severe drug interactions.
		//Be aware that while every interaction type can potentially have a ConfigRT setting,
		//top priority interactions have their settings completely ignored.
		//This is so that the setup can be modular and we can change priorities in the future.
		g_propManager.CachePropertiesInBulk("CDrugInteractionDlg", propNumber,
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
			"OR Name = 'DISeverityFilters_FilterByDefault' "
			")",
			_Q(GetCurrentUserName()));

		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_pInteractionList = BindNxDataList2Ctrl(IDC_INTERACTION_LIST, false);
		m_btnClose.AutoSet(NXB_CLOSE);

		// (j.jones 2013-05-10 14:37) - PLID 55955 - added ability to configure severity filters
		m_btnConfigSeverityFilters.AutoSet(NXB_MODIFY);
		//check permissions
		if(!(GetCurrentUserPermissions(bioDrugInteractionSeverityFilter) & (sptWrite|sptWriteWithPass))) {
			m_btnConfigSeverityFilters.EnableWindow(FALSE);
		}

		// (j.jones 2013-05-14 17:14) - PLID 56634 - Added ability to toggle filtering on and off.
		// We default to applying the filters, if any are configured, but if the user toggles the
		// filtering off then that will persist between sessions, such that they can choose to
		// "show all" by default.
		m_bShowFilteredInteractions = (GetRemotePropertyInt("DISeverityFilters_FilterByDefault", 1, 0, GetCurrentUserName(), true) == 1);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.jones 2012-11-28 16:24) - PLID 53194 - assigns our tracked interaction arrays
// and toggles the m_bInteractionsChanged boolean
// (j.fouts 2013-01-03 12:27) - PLID 54429 - This now returns true if interactions have changed
bool CDrugInteractionDlg::UpdateInteractionArrays(Nx::SafeArray<IUnknown *> aryAllNewDrugDrugInteractions,
												  Nx::SafeArray<IUnknown *> aryAllNewDrugAllergyInteractions,
												  Nx::SafeArray<IUnknown *> aryAllNewDrugDiagnosisInteractions)
{
	// Track whether the content of any of the three interactions lists actually changed.
	// Only do this if the boolean is false, to handle the possibility of the arrays being
	// re-assigned more than once between Refresh() calls.

	// (j.jones 2013-06-04 17:27) - PLID 56634 - apply filters, if any, to the new interaction lists
	Nx::SafeArray<IUnknown *>  aryFilteredNewDrugDrugInteractions = FilterDrugDrugInteractionsBySeverity(aryAllNewDrugDrugInteractions);
	Nx::SafeArray<IUnknown *>  aryFilteredNewDrugAllergyInteractions = FilterDrugAllergyInteractionsBySeverity(aryAllNewDrugAllergyInteractions);
	Nx::SafeArray<IUnknown *>  aryFilteredNewDrugDiagnosisInteractions = FilterDrugDiagnosisInteractionsBySeverity(aryAllNewDrugDiagnosisInteractions);

	// (j.jones 2013-06-04 17:27) - PLID 56634 - We will notify the user based on the subset of
	// interactions they are currently configured to be seeing.
	// If the "show all" toggle is on, then this would be all interactions even if filters are in place.
	// This means that automatic popup notifications can still occur when filters are in place, if the
	// "show all" toggle is currently on. That toggle persists between sessions.
	Nx::SafeArray<IUnknown *>  aryCurDrugDrugInteractions;
	Nx::SafeArray<IUnknown *>  aryCurDrugAllergyInteractions;
	Nx::SafeArray<IUnknown *>  aryCurDrugDiagnosisInteractions;
	Nx::SafeArray<IUnknown *>  aryNewDrugDrugInteractions;
	Nx::SafeArray<IUnknown *>  aryNewDrugAllergyInteractions;
	Nx::SafeArray<IUnknown *>  aryNewDrugDiagnosisInteractions;
	if(m_bShowFilteredInteractions) {
		//we will be comparing the filtered interactions
		aryCurDrugDrugInteractions = m_aryFilteredDrugDrugInteractions;
		aryCurDrugAllergyInteractions = m_aryFilteredDrugAllergyInteractions;
		aryCurDrugDiagnosisInteractions = m_aryFilteredDrugDiagnosisInteractions;
		aryNewDrugDrugInteractions = aryFilteredNewDrugDrugInteractions;
		aryNewDrugAllergyInteractions = aryFilteredNewDrugAllergyInteractions;
		aryNewDrugDiagnosisInteractions = aryFilteredNewDrugDiagnosisInteractions;
	}
	else {
		//we will be comparing all interactions
		aryCurDrugDrugInteractions = m_aryAllDrugDrugInteractions;
		aryCurDrugAllergyInteractions = m_aryAllDrugAllergyInteractions;
		aryCurDrugDiagnosisInteractions = m_aryAllDrugDiagnosisInteractions;
		aryNewDrugDrugInteractions = aryAllNewDrugDrugInteractions;
		aryNewDrugAllergyInteractions = aryAllNewDrugAllergyInteractions;
		aryNewDrugDiagnosisInteractions = aryAllNewDrugDiagnosisInteractions;
	}

	if(!m_bInteractionsChanged) {

		//first compare sizes, since that's a quick check
		if(!m_bInteractionsChanged && aryCurDrugDrugInteractions.GetSize() != aryNewDrugDrugInteractions.GetSize()) {
			m_bInteractionsChanged = true;
		}
		if(!m_bInteractionsChanged && aryCurDrugAllergyInteractions.GetSize() != aryNewDrugAllergyInteractions.GetSize()) {
			m_bInteractionsChanged = true;	
		}
		if(!m_bInteractionsChanged && aryCurDrugDiagnosisInteractions.GetSize() != aryNewDrugDiagnosisInteractions.GetSize()) {
			m_bInteractionsChanged = true;
		}

		//Next compare contents, which is a more intensive check.
		//These comparisons *will* think something changed if the lists happen
		//to have the same contents in a different order.
		if(!m_bInteractionsChanged && aryNewDrugDrugInteractions.GetSize() > 0
			&& aryNewDrugDrugInteractions.GetSize() == aryCurDrugDrugInteractions.GetSize()) {

			long nDrugCount = aryCurDrugDrugInteractions.GetCount();
			for(int i=0; i<nDrugCount && !m_bInteractionsChanged; i++)
			{
				NexTech_Accessor::_FDBDrugDrugInteractionPtr ptrCurrent = aryCurDrugDrugInteractions.GetAt(i);
				NexTech_Accessor::_FDBDrugDrugInteractionPtr ptrNew = aryNewDrugDrugInteractions.GetAt(i);

				if(ptrCurrent->MedID != ptrNew->MedID
					|| ptrCurrent->InteractMedID != ptrNew->InteractMedID
					|| ptrCurrent->DrugName != ptrNew->DrugName
					|| ptrCurrent->InteractDrugName != ptrNew->InteractDrugName
					|| ptrCurrent->DetailedMessage != ptrNew->DetailedMessage
					|| ptrCurrent->Severity != ptrNew->Severity) {
					//something changed
					m_bInteractionsChanged = true;
				}
			}
		}
		if(!m_bInteractionsChanged && aryNewDrugAllergyInteractions.GetSize() > 0
			&& aryNewDrugAllergyInteractions.GetSize() == aryCurDrugAllergyInteractions.GetSize()) {

			long nAllergyCount = aryCurDrugAllergyInteractions.GetCount();

			for(int i=0; i<nAllergyCount && !m_bInteractionsChanged; i++)
			{
				NexTech_Accessor::_FDBDrugAllergyInteractionPtr ptrCurrent = aryCurDrugAllergyInteractions.GetAt(i);
				NexTech_Accessor::_FDBDrugAllergyInteractionPtr ptrNew = aryNewDrugAllergyInteractions.GetAt(i);

				if(ptrCurrent->MedID != ptrNew->MedID
					|| ptrCurrent->AllergyConceptID != ptrNew->AllergyConceptID
					|| ptrCurrent->AllergyConceptType != ptrNew->AllergyConceptType
					|| ptrCurrent->DrugName != ptrNew->DrugName
					|| ptrCurrent->AllergyName != ptrNew->AllergyName
					|| ptrCurrent->DetailedMessage != ptrNew->DetailedMessage
					|| ptrCurrent->source != ptrNew->source
					|| ptrCurrent->SeverityLevel != ptrNew->SeverityLevel) {						
					//something changed
					m_bInteractionsChanged = true;
				}
			}
		}
		if(!m_bInteractionsChanged && aryNewDrugDiagnosisInteractions.GetSize() > 0
			&& aryNewDrugDiagnosisInteractions.GetSize() == aryCurDrugDiagnosisInteractions.GetSize()) {

			long nDiagnosisCount = aryCurDrugDiagnosisInteractions.GetCount();

			for(int i=0; i<nDiagnosisCount && !m_bInteractionsChanged; i++)
			{
				NexTech_Accessor::_FDBDrugDiagnosisInteractionPtr ptrCurrent = aryCurDrugDiagnosisInteractions.GetAt(i);
				NexTech_Accessor::_FDBDrugDiagnosisInteractionPtr ptrNew = aryNewDrugDiagnosisInteractions.GetAt(i);

				// (r.gonet 02/28/2014) - PLID 60755 - Renamed some ICD-9 referencing member variables and added DiagCodeSystem
				// so that we account for the ICD-10 interactions as well.
				if(ptrCurrent->MedID != ptrNew->MedID
					|| ptrCurrent->DiagCode != ptrNew->DiagCode
					|| ptrCurrent->DrugName != ptrNew->DrugName
					|| ptrCurrent->DiagDesc != ptrNew->DiagDesc
					|| ptrCurrent->DiagCodeSystem != ptrNew->DiagCodeSystem
					|| ptrCurrent->DetailedMessage != ptrNew->DetailedMessage
					|| ptrCurrent->NavCode != ptrNew->NavCode
					|| ptrCurrent->Severity != ptrNew->Severity) {
					//something changed
					m_bInteractionsChanged = true;
				}
			}
		}
	}

	//now assign the new arrays, both "all" and "filtered"
	m_aryAllDrugDrugInteractions = aryAllNewDrugDrugInteractions;
	m_aryAllDrugAllergyInteractions = aryAllNewDrugAllergyInteractions;
	m_aryAllDrugDiagnosisInteractions = aryAllNewDrugDiagnosisInteractions;

	m_aryFilteredDrugDrugInteractions = aryFilteredNewDrugDrugInteractions;
	m_aryFilteredDrugAllergyInteractions = aryFilteredNewDrugAllergyInteractions;
	m_aryFilteredDrugDiagnosisInteractions = aryFilteredNewDrugDiagnosisInteractions;

	return m_bInteractionsChanged;
}

// (j.fouts 2012-11-14 11:42) - PLID 53573 - Created a ShowOnInteractions to show without calling the API, this will use the arrays
// of interactions that are passed
// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
// will show the dialog even if its current interactions have not changed since the last popup.
void CDrugInteractionDlg::ShowOnInteraction(Nx::SafeArray<IUnknown*> saryDrugDrugInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts,
									  long nPatientID,
									  bool bForceShowEvenIfBlank /*=false*/, bool bForceShowEvenIfUnchanged /*= false*/)
{
	// (j.jones 2012-11-29 10:18) - PLID 53194 - if the patient changed,
	// then the interactions definitely changed
	if(m_nCurrentPatientID != nPatientID) {
		m_bInteractionsChanged = true;
	}

	m_nCurrentPatientID = nPatientID;

	// (j.jones 2012-11-28 16:24) - PLID 53194 - assigns our tracked interaction arrays
	// and toggles the m_bInteractionsChanged boolean
	UpdateInteractionArrays(saryDrugDrugInteracts, saryDrugAllergyInteracts, saryDrugDiagnosisInteracts);

	// (j.jones 2013-06-04 17:27) - PLID 56634 - We will notify the user based on the subset of
	// interactions they are currently configured to be seeing.
	// If the "show all" toggle is on, then this would be all interactions even if filters are in place.
	// This means that automatic popup notifications can still occur when filters are in place, if the
	// "show all" toggle is currently on. That toggle persists between sessions.
	long nDrugCount = 0, nAllergyCount = 0, nDiagnosisCount = 0;
	if(m_bShowFilteredInteractions) {
		//we will be comparing the filtered interactions
		nDrugCount = m_aryFilteredDrugDrugInteractions.GetCount();
		nAllergyCount = m_aryFilteredDrugAllergyInteractions.GetCount();
		nDiagnosisCount = m_aryFilteredDrugDiagnosisInteractions.GetCount();
	}
	else {
		//we will be comparing all interactions
		nDrugCount = m_aryAllDrugDrugInteractions.GetCount();
		nAllergyCount = m_aryAllDrugAllergyInteractions.GetCount();
		nDiagnosisCount = m_aryAllDrugDiagnosisInteractions.GetCount();
	}	

	//If all our counts are 0 then there were no interactions found that would currently be shown,
	//so only show if we are forced
	if(!nDrugCount && !nAllergyCount && !nDiagnosisCount)
	{
		ShowIfForced(bForceShowEvenIfBlank);
		return;
	}
		
	//We have interactions so lets show the dialog
	// (j.jones 2012-11-28 16:20) - PLID 53194 - This was TRUE, but now we
	// optionally only show if interactions changed since the last time we
	// showed the dialog, or of course if bForceShowEvenIfBlank is true.
	ShowIfForced(bForceShowEvenIfBlank || bForceShowEvenIfUnchanged || m_bInteractionsChanged);
}

// (j.jones 2012-09-26 14:09) - PLID 52872 - added patient ID
// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
// will show the dialog even if its current interactions have not changed since the last popup.
void CDrugInteractionDlg::ShowOnInteraction(long nPatientID, bool bRequery /*= true*/,
											bool bForceShowEvenIfBlank /*= false*/, bool bForceShowEvenIfUnchanged /*= false*/)
{
	try
	{
		// (j.fouts 2012-09-07 13:18) - PLID 52482 - We are not going to show if they are not on the meds tab
		// (j.jones 2012-09-26 11:32) - PLID 52872 - removed the meds. tab check, it's now the calling code's
		// responsibility to determine whether to show the dialog on a certain tab

		//If we haven't changed patients and we are not forced to requery then show the dialog
		if(!bRequery && (m_nCurrentPatientID == nPatientID))
		{
			ShowIfForced(bForceShowEvenIfBlank);
			return;
		}

		// (j.jones 2012-11-29 10:18) - PLID 53194 - if the patient changed,
		// then the interactions definitely changed
		if(m_nCurrentPatientID != nPatientID) {
			m_bInteractionsChanged = true;
		}

		m_nCurrentPatientID = nPatientID;

		// (j.fouts 2012-08-15 13:28) - PLID 52089 - Get the interaction data from the API
		NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpect(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));
		//We want to check all interactions
		pExpect->DrugDrugInteracts = TRUE;
		// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
		pExpect->ExcludeMonographInformation = VARIANT_TRUE;
		pExpect->DrugAllergyInteracts = TRUE;
		// (j.fouts 2012-09-05 10:26) - PLID 51710 - We are currently not using Diagnosis interactions until this is fully implemented in Practice.
		pExpect->DrugDiagnosisInteracts = TRUE;
		// (b.savon 2013-03-11 12:18) - PLID 55518 - remove the last parameter
		//Call the API and have it generated the interaction results
		// (j.fouts 2013-04-24 16:54) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
		NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(), _bstr_t(FormatString("%li", m_nCurrentPatientID)), pExpect);

		if(!pResults)
		{
			//No interactions
			ShowIfForced(bForceShowEvenIfBlank);
			return;
		}

		// (j.jones 2012-11-28 16:24) - PLID 53194 - assigns our tracked interaction arrays
		// and toggles the m_bInteractionsChanged boolean
		UpdateInteractionArrays(Nx::SafeArray<IUnknown *>(pResults->DrugDrugInteracts),
			Nx::SafeArray<IUnknown *>(pResults->DrugAllergyInteracts),
			Nx::SafeArray<IUnknown *>(pResults->DrugDiagnosisInteracts));

		// (j.jones 2013-06-04 17:27) - PLID 56634 - We will notify the user based on the subset of
		// interactions they are currently configured to be seeing.
		// If the "show all" toggle is on, then this would be all interactions even if filters are in place.
		// This means that automatic popup notifications can still occur when filters are in place, if the
		// "show all" toggle is currently on. That toggle persists between sessions.
		long nCurrentInteractionCount = 0;
		if(m_bShowFilteredInteractions) {
			//we will be comparing the filtered interactions
			nCurrentInteractionCount = GetFilteredInteractionCount();
		}
		else {
			//we will be comparing all interactions
			nCurrentInteractionCount = InteractionCount();
		}

		//If all our counts are 0 then there were no interactions found that would currently be shown,
		//so only show if we are forced
		if(nCurrentInteractionCount == 0)
		{
			ShowIfForced(bForceShowEvenIfBlank);
			return;
		}
		
		//We have interactions so lets show the dialog
		// (j.jones 2012-11-28 16:20) - PLID 53194 - This was TRUE, but now we
		// optionally only show if interactions changed since the last time we
		// showed the dialog, or of course if bForceShowEvenIfBlank is true.
		ShowIfForced(bForceShowEvenIfBlank || bForceShowEvenIfUnchanged || m_bInteractionsChanged);
		return;
	}
	NxCatchAll("Error Generating Drug Interactions. CDrugInteractionDlg::ShowOnInteraction()");

	//If we got here we caught an exception so we may not be initialized yet
	m_nCurrentPatientID = -1;
	m_bInteractionsChanged = true;
	ShowIfForced(bForceShowEvenIfBlank || bForceShowEvenIfUnchanged);
	return;
}

// CDrugInteractionDlg message handlers
BEGIN_EVENTSINK_MAP(CDrugInteractionDlg, CNxDialog)
	ON_EVENT(CDrugInteractionDlg, IDC_INTERACTION_LIST, 19, CDrugInteractionDlg::LeftClickInteractionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CDrugInteractionDlg::LeftClickInteractionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		//Get the row that the clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
		{
			return;
		}

		//Did the click the detatil column?
		if(nCol == dilDetail)
		{
			//Show the details for that interaction
			MessageBox(VarString(pRow->GetValue(dilMsg)));
		}
		else if(nCol == dilMonograph)
		{
			//TES 11/10/2013 - PLID 59399 - If they clicked on the Monograph cell, and this row has a monograph, pop up the dialog
			if(pRow->GetValue(dilMonograph).vt != VT_NULL) {
				// (b.savon 2014-01-30 09:31) - PLID 60517 - Make a public exposed FDB DrugDrug interation monograph method
				// Make the API request to get the monograph data.
				CWaitCursor cwait;
				NexTech_Accessor::_FDBDrugDrugInteractionPtr pResults = 
					GetAPI()->GetDrugDrugMonograph(
								GetAPISubkey(), 
								GetAPILoginToken(),
								_bstr_t(FormatString("%li",VarLong(pRow->GetValue(dilMedID)))),
								_bstr_t(FormatString("%li",VarLong(pRow->GetValue(dilInteractID))))
					);

				CMonographDlg dlg(this);
				//Give the dlg the HTML string that it should display
				// (b.savon 2014-01-30 09:31) - PLID 60517 - Use our API result HtmlString
				dlg.m_strHTML = CString((LPCTSTR)pResults->MonographData->HtmlString);
				//TES 11/21/2013 - PLID 59399 - Tell it to show the label indicating that the monograph was developed by FDB
				dlg.m_bShowDeveloper = true;
				dlg.DoModal();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

//TES 11/10/2013 - PLID 59399 - Function so that rows with no monograph don't show the link cursor on the monograph cell
NXDATALIST2Lib::IFormatSettingsPtr CDrugInteractionDlg::GetMonographCellFormat(BOOL bHasMonograph)
{
	NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));	
	pfs->PutDataType(VT_BSTR);
	pfs->PutFieldType(bHasMonograph?NXDATALIST2Lib::cftTextSingleLineLink:NXDATALIST2Lib::cftTextSingleLine);
	pfs->PutEditable(VARIANT_FALSE);
	return pfs;	
}

void CDrugInteractionDlg::PopulateDrugDrugInteractions()
{
	Nx::SafeArray<IUnknown *>  aryDrugDrugInteractions;
	if(m_bShowFilteredInteractions) {
		aryDrugDrugInteractions = m_aryFilteredDrugDrugInteractions;
	}
	else {
		//show all interactions
		aryDrugDrugInteractions = m_aryAllDrugDrugInteractions;
	}

	long nDrugCount = aryDrugDrugInteractions.GetCount();

	for(int i=0;i<nDrugCount;i++)
	{
		NexTech_Accessor::_FDBDrugDrugInteractionPtr ptrCurrent = aryDrugDrugInteractions.GetAt(i);

		//We have a new drug/drug pair so lets create a new row and fill it with what we can
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInteractionList->GetNewRow();
		pRow->PutValue(dilMedID,_variant_t(ptrCurrent->MedID));
		pRow->PutValue(dilInteractID,_variant_t(ptrCurrent->InteractMedID));
		pRow->PutValue(dilTypeID,_variant_t(ditDrugDrug));
		pRow->PutValue(dilMed,_variant_t(ptrCurrent->DrugName));
		pRow->PutValue(dilInteract,_variant_t(ptrCurrent->InteractDrugName));
		pRow->PutValue(dilDetail,_variant_t("Show Details"));
		pRow->PutValue(dilMsg,ptrCurrent->DetailedMessage);

		// (j.jones 2012-11-13 12:20) - PLID 53724 - This is now the FDB severity enum, we now use the accessor's FDB enums.
		// The severity level name will match those enums. This enum column, however, currently serves no purpose.
		pRow->PutValue(dilFDBSeverityEnum,_variant_t(ptrCurrent->Severity));
		
		// (j.jones 2012-11-13 14:36) - PLID 53724 - the SeverityOrder column is used to determine the sort
		// order, such that the most severe interactions show up first (this is defined by us, not FDB)

		// (j.jones 2013-05-10 15:35) - PLID 55955 - we now get this information from a utils function
		DrugInteractionDisplayFields eResult = GetDrugDrugInteractionSeverityInfo(ptrCurrent->Severity);
		pRow->PutValue(dilSeverityOrderEnum, _variant_t(eResult.ePriority));
		pRow->PutValue(dilSeverityLevelName, _variant_t(eResult.strSeverityName));
		//TES 11/10/2013 - PLID 59399 - If they have a monograph, let the user pop it up
		if(ptrCurrent->MonographData->MonographExists) {
			pRow->PutValue(dilMonograph, _bstr_t("Show Monograph"));
			pRow->PutValue(dilMonographText, ptrCurrent->MonographData->HtmlString);
		}
		else {
			pRow->PutValue(dilMonograph, g_cvarNull);
			pRow->PutValue(dilMonographText, g_cvarNull);
		}
		//TES 11/10/2013 - PLID 59399 - Turn off the hyperlink for rows without a monograph
		pRow->PutRefCellFormatOverride(dilMonograph, GetMonographCellFormat(ptrCurrent->MonographData->MonographExists));
		pRow->BackColor = eResult.eColor;

		if(ptrCurrent->DrugInactive)
		{
			pRow->PutValue(dilType,_variant_t(L"Drug (Inactive)"));
		} else {
			pRow->PutValue(dilType,_variant_t(L"Drug"));
		}

		//Add our new row
		m_pInteractionList->AddRowSorted(pRow,NULL);
	}
}

void CDrugInteractionDlg::ShowIfForced(BOOL bForceShow)
{
	// (j.fouts 2012-09-06 08:35) - PLID 52482 - This is no longer modal, so refresh if it is open and show if its not
	if(this->IsWindowVisible())
	{
		Refresh();
	}
	else
	{
		if(bForceShow)
		{
			// (j.jones 2012-10-11 12:50) - PLID 53091 - Get the parent, the currently active window,
			// and the enabled state of the parent. If the parent is not enabled, it must mean that
			// the active window is modal.
			CWnd *pParentWnd = this->GetParent();
			CWnd *pActiveWnd = GetActiveWindow();
			BOOL bParentEnabled = FALSE;
			if(pParentWnd) {
				bParentEnabled = pParentWnd->IsWindowEnabled();
			}

			Refresh();

			ShowWindow(SW_RESTORE);
			BringWindowToTop();

			// (j.jones 2012-10-11 12:51) - PLID 53091 - if the active window is modal (parent is disabled),
			// bring it to top, otherwise our window activation would be on top, but unclickable
			if(!bParentEnabled && pActiveWnd) {
				pActiveWnd->BringWindowToTop();
			}
		}
	}
}

void CDrugInteractionDlg::PopulateDrugDiagnosisInteractions()
{
	Nx::SafeArray<IUnknown *>  aryDrugDiagnosisInteractions;
	if(m_bShowFilteredInteractions) {
		aryDrugDiagnosisInteractions = m_aryFilteredDrugDiagnosisInteractions;
	}
	else {
		//show all interactions
		aryDrugDiagnosisInteractions = m_aryAllDrugDiagnosisInteractions;
	}

	long nDiagnosisCount = aryDrugDiagnosisInteractions.GetCount();

	for(int i=0;i<nDiagnosisCount;i++)
	{
		NexTech_Accessor::_FDBDrugDiagnosisInteractionPtr ptrCurrent = aryDrugDiagnosisInteractions.GetAt(i);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInteractionList->GetNewRow();
		pRow->PutValue(dilMedID,_variant_t(ptrCurrent->MedID));
		// (r.gonet 02/28/2014) - PLID 60755 - Renamed to remove reference to ICD-9
		pRow->PutValue(dilInteractID,_variant_t(ptrCurrent->DiagCode));
		pRow->PutValue(dilTypeID,_variant_t(ditDrugDiagnosis));
		pRow->PutValue(dilMed,_variant_t(ptrCurrent->DrugName));
		// (r.gonet 02/28/2014) - PLID 60755 - Renamed to remove reference to ICD-9
		pRow->PutValue(dilInteract,_variant_t(ptrCurrent->DiagDesc));
		pRow->PutValue(dilDetail,_variant_t("Show Details"));
		pRow->PutValue(dilMsg,_variant_t(ptrCurrent->DetailedMessage));
		if(ptrCurrent->NavCode == NexTech_Accessor::FDBDiagnosisInteractionNavCode_Equal)
		{
			pRow->PutValue(dilType,_variant_t(L"Diagnosis"));
		} else {
			pRow->PutValue(dilType,_variant_t(L"Diagnosis (Related)"));
		}
		
		// (j.jones 2012-11-13 12:20) - PLID 53724 - This is now the FDB severity enum, we now use the accessor's FDB enums.
		// The severity level name will match those enums. This enum column, however, currently serves no purpose.
		pRow->PutValue(dilFDBSeverityEnum,_variant_t(ptrCurrent->Severity));

		// (j.jones 2012-11-13 14:36) - PLID 53724 - the SeverityOrder column is used to determine the sort
		// order, such that the most severe interactions show up first (this is defined by us, not FDB)

		// (j.jones 2013-05-10 15:35) - PLID 55955 - we now get this information from a utils function
		DrugInteractionDisplayFields eResult = GetDrugDiagnosisInteractionSeverityInfo(ptrCurrent->Severity);
		pRow->PutValue(dilSeverityOrderEnum, _variant_t(eResult.ePriority));
		pRow->PutValue(dilSeverityLevelName, _variant_t(eResult.strSeverityName));
		//TES 11/10/2013 - PLID 59399 - Only drug-drug interactions have monographs
		pRow->PutValue(dilMonograph, g_cvarNull);
		pRow->PutValue(dilMonographText, g_cvarNull);
		pRow->BackColor = eResult.eColor;

		m_pInteractionList->AddRowSorted(pRow,NULL);
	}
}

void CDrugInteractionDlg::PopulateDrugAllergyInteractions()
{
	Nx::SafeArray<IUnknown *>  aryDrugAllergyInteractions;
	if(m_bShowFilteredInteractions) {
		aryDrugAllergyInteractions = m_aryFilteredDrugAllergyInteractions;
	}
	else {
		//show all interactions
		aryDrugAllergyInteractions = m_aryAllDrugAllergyInteractions;
	}

	long nAllergyCount = aryDrugAllergyInteractions.GetCount();

	for(int i=0;i<nAllergyCount;i++)
	{
		NexTech_Accessor::_FDBDrugAllergyInteractionPtr ptrCurrent = aryDrugAllergyInteractions.GetAt(i);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInteractionList->GetNewRow();
		pRow->PutValue(dilMedID,_variant_t(ptrCurrent->MedID));
		pRow->PutValue(dilInteractID,_variant_t(ptrCurrent->AllergyConceptID));
		pRow->PutValue(dilTypeID,_variant_t(ditDrugAllergy));
		pRow->PutValue(dilMed,_variant_t(ptrCurrent->DrugName));
		pRow->PutValue(dilInteract,_variant_t(ptrCurrent->AllergyName));	
		pRow->PutValue(dilDetail,_variant_t("Show Details"));
		pRow->PutValue(dilMsg,_variant_t(ptrCurrent->DetailedMessage));

		// (j.jones 2012-11-13 12:20) - PLID 53724 - This is now the FDB severity enum, we now use the accessor's FDB enums.
		// The severity level name will match those enums. This enum column, however, currently serves no purpose.
		// In the case of allergies, the name isn't so much a severity level, instead it's the type of allergy
		// interaction (inactive, related, direct).
		pRow->PutValue(dilFDBSeverityEnum,_variant_t(ptrCurrent->source));

		// (j.jones 2012-11-13 14:36) - PLID 53724 - the SeverityOrder column is used to determine the sort
		// order, such that the most severe interactions show up first (this is defined by us, not FDB)

		// (j.jones 2013-05-10 15:35) - PLID 55955 - we now get this information from a utils function
		// (j.fouts 2013-05-20 10:17) - PLID 56571 - The API now returns the severity level
		DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(ptrCurrent->source, ptrCurrent->SeverityLevel);
		pRow->PutValue(dilSeverityOrderEnum, _variant_t(eResult.ePriority));
		pRow->PutValue(dilSeverityLevelName, _variant_t(eResult.strSeverityName));
		pRow->BackColor = eResult.eColor;

		pRow->PutValue(dilType,_variant_t(L"Allergy"));

		//TES 11/10/2013 - PLID 59399 - Only drug-drug interactions have monographs
		pRow->PutValue(dilMonograph, g_cvarNull);
		pRow->PutValue(dilMonographText, g_cvarNull);

		m_pInteractionList->AddRowSorted(pRow,NULL);
	}
}

// (j.fouts 2012-09-06 08:36) - PLID 52482 - Need to be able to refresh the datalist to reflect interaction changes
void CDrugInteractionDlg::Refresh()
{
	try
	{
		CWaitCursor pWait;

		//Populate the datalist
		m_pInteractionList->Clear();

		// (j.jones 2012-10-15 16:07) - PLID 53185 - show the current patient's name and ID
		CString strWindowText;
		strWindowText.Format("Drug Interactions for %s (%li)", GetExistingPatientName(m_nCurrentPatientID), GetExistingPatientUserDefinedID(m_nCurrentPatientID));
		SetWindowText(strWindowText);

		// (j.jones 2013-05-14 09:25) - PLID 56634 - now these functions can either show
		// "all" interactions, or just the filtered interactions, which hides
		// lower-level severity interactions the user has chosen to ignore
		PopulateDrugDrugInteractions();
		PopulateDrugDiagnosisInteractions();
		PopulateDrugAllergyInteractions();

		// (j.jones 2013-05-14 16:50) - PLID 56634 - update the count label, and show/hide the
		// button to toggle the filter
		long nTotalCount = InteractionCount();
		long nFilteredCount = GetFilteredInteractionCount();

		//update the label to show both counts only if the counts are different and
		//if we are only displaying the filtered list
		CString strLabel;
		if(nTotalCount != nFilteredCount && m_bShowFilteredInteractions) {
			//update the label with both counts
			strLabel.Format("Total Interactions: %li		Displayed Interactions: %li", nTotalCount, nFilteredCount);
		}
		else {
			//update the label with just the total count
			strLabel.Format("Total Interactions: %li", nTotalCount);
		}
		m_nxstaticLabelInteractionCount.SetWindowText(strLabel);

		//update the filter button to use the "filter in use" icon if any are suppressed,
		//normal filter icon if we're showing everything
		if(m_bShowFilteredInteractions) {
			//we're showing filtered interactions, so make the button say "show all"
			//and make the icon indicate that a filter is in use
			m_btnToggleFilter.SetWindowText("Show All Interactions");
			m_btnToggleFilter.SetIcon(IDI_FILTERDN);
		}
		else {
			//we're showing all interactions, so make the button say "filter"
			//and make the icon indicate that a filter can be applied
			m_btnToggleFilter.SetWindowText("Filter Interactions");
			m_btnToggleFilter.SetIcon(IDI_FILTER);
		}

		//if the total count and filtered count are different, show the toggle button
		if(nTotalCount != nFilteredCount) {
			m_btnToggleFilter.ShowWindow(SW_SHOW);
			//force the button to redraw, otherwise the icon doesn't change
			//until you move your mouse away
			m_btnToggleFilter.Invalidate();
		}
		else {
			//counts are the same, so we don't need the filter button at all
			m_btnToggleFilter.ShowWindow(SW_HIDE);
		}

		// (j.jones 2012-11-28 16:14) - PLID 53194 - reset the interactions changed flag,
		// all changes to the arrays between now and the next Refresh() call will set it to true
		m_bInteractionsChanged = false;
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-09-07 13:18) - PLID 52482 - Need to handle the patient changing
void CDrugInteractionDlg::HandleActivePatientChange()
{
	// (j.jones 2012-09-26 11:33) - PLID 52872 - We now do nothing if the window is not visible.
	// Also we now pass in the patient ID.
	if(IsWindowVisible()) {
		this->ShowOnInteraction(GetActivePatientID(), true, false, false);
	}
}

// (b.savon 2012-11-29 16:49) - PLID 53773 - Save Window Size
void CDrugInteractionDlg::OnDestroy()
{
	CNxDialog::OnDestroy();

	try{
		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);
		CString strBuffer;
		strBuffer.Format("%d,%d,%d,%d", wp.rcNormalPosition.left, wp.rcNormalPosition.top, wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
		AfxGetApp()->WriteProfileString("Settings", "DrugInteractionWindowSize-" + GetInteractionParent(), strBuffer);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-11-29 16:49) - PLID 53773 - Resotre Window Size
void CDrugInteractionDlg::LoadWindowSize()
{
	// Size the window to the last size it was
	{
		// Get the work area to make sure that wherever we put it, it's accessible
		CRect rcWork;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
		// Get the last size and position of the window
		CRect rcDialog;
		CString strBuffer = AfxGetApp()->GetProfileString("Settings", "DrugInteractionWindowSize-" + GetInteractionParent());
		if (strBuffer.IsEmpty() || _stscanf(strBuffer, "%d,%d,%d,%d", &rcDialog.left, &rcDialog.top, &rcDialog.right, &rcDialog.bottom) != 4) {
			// We couldn't get the registry setting for some reason
			CSize ptDlgHalf(450, 250);
			CPoint ptScreenCenter(rcWork.CenterPoint());
			rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
		} else {
			// (b.cardillo 2010-03-26 16:31) - PLID 37583
			// Translate from workspace coordinates (which are what we save in the registry) to screen coordinates
			rcDialog.OffsetRect(rcWork.TopLeft());
		}
		// Make sure if we put the dialog at rcDialog it's accessible (we consider 'accessible' 
		// to mean that the dialog title bar is visible vertically, and 1/3 visible horizontally)
		if (rcDialog.top+rcDialog.Height()/8<rcWork.bottom && rcDialog.top>rcWork.top &&
			rcDialog.left<rcWork.right-rcDialog.Width()/3 && rcDialog.right>rcWork.left+rcDialog.Width()/3) {
			// It's accessible so leave it
		} else {
			// It's not accessible so center it
			CSize ptDlgHalf(rcDialog.Width()/2, rcDialog.Height()/2);
			CPoint ptScreenCenter(rcWork.CenterPoint());
			rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
		}
		// Move the window to its new position
		MoveWindow(rcDialog);
	}
}

// (b.savon 2012-11-29 16:49) - PLID 53773 - Resotre Window Size
CString CDrugInteractionDlg::GetInteractionParent()
{
	switch(m_eipParent){
		case eipMedications:
			return "Medication";
		case eipEMR:
			return "EMR";
		case eipQueue:
			return "Queue";
		// (j.jones 2013-11-25 13:47) - PLID 59772 - added enum for CPrescriptionEditDlg
		case epiPrescriptionEdit:
			return "Prescription";
		default:
			return "";
	}
}

// (j.fouts 2013-01-03 12:27) - PLID 54429 - Gets the count of interactions
long CDrugInteractionDlg::InteractionCount()
{
	try
	{
		// (j.jones 2013-05-14 08:58) - PLID 56634 - return the count of "all" interactions,
		// NOT the "displayed" ones, to make it clear what the grand total is
		long nDrugCount = m_aryAllDrugDrugInteractions.GetCount();
		long nAllergyCount = m_aryAllDrugAllergyInteractions.GetCount();
		long nDiagnosisCount = m_aryAllDrugDiagnosisInteractions.GetCount();

		return nDrugCount + nAllergyCount + nDiagnosisCount;
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2013-05-14 16:57) - PLID 56634 - returns the count of interactions that
// remain displayed after low-severity interactions are hidden (if any)
long CDrugInteractionDlg::GetFilteredInteractionCount()
{
	try
	{
		long nDrugCount = m_aryFilteredDrugDrugInteractions.GetCount();
		long nAllergyCount = m_aryFilteredDrugAllergyInteractions.GetCount();
		long nDiagnosisCount = m_aryFilteredDrugDiagnosisInteractions.GetCount();

		return nDrugCount + nAllergyCount + nDiagnosisCount;
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2013-05-10 14:37) - PLID 55955 - added ability to configure severity filters
void CDrugInteractionDlg::OnBtnConfigureSeverityFilters()
{
	try {

		//check permissions
		if(!CheckCurrentUserPermissions(bioDrugInteractionSeverityFilter, sptWrite)) {
			return;
		}

		CDrugInteractionSeverityConfigDlg dlg;
		if(dlg.DoModal() == IDOK) {
			// (j.jones 2013-05-13 17:30) - PLID 56634 - re-filter the displayed interactions,
			// then refresh the screen to reflect any changes
			m_aryFilteredDrugDrugInteractions = FilterDrugDrugInteractionsBySeverity(m_aryAllDrugDrugInteractions);
			m_aryFilteredDrugAllergyInteractions = FilterDrugAllergyInteractionsBySeverity(m_aryAllDrugAllergyInteractions);
			m_aryFilteredDrugDiagnosisInteractions = FilterDrugDiagnosisInteractionsBySeverity(m_aryAllDrugDiagnosisInteractions);
			Refresh();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-05-14 09:06) - PLID 56634 - added functions to filter a list of "all" interactions
// down to a list of interactions that will be displayed, ignoring low-severity interactions that
// the user has chosen to suppress
Nx::SafeArray<IUnknown *> CDrugInteractionDlg::FilterDrugDrugInteractionsBySeverity(Nx::SafeArray<IUnknown *> aryAllDrugDrugInteractions)
{
	bool bShowContraindicatedDrugCombinations = GetRemotePropertyInt("DISeverityFilter_Drug_ContraindicatedDrugCombination", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowSevereInteractions = GetRemotePropertyInt("DISeverityFilter_Drug_SevereInteraction", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowModerateInteractions = GetRemotePropertyInt("DISeverityFilter_Drug_ModerateInteraction", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowUndeterminedSeverity = GetRemotePropertyInt("DISeverityFilter_Drug_UndeterminedSeverity", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;

	Nx::SafeArray<IUnknown *> aryFilteredDrugDrugInteractions;

	long nDrugCount = aryAllDrugDrugInteractions.GetCount();
	for(int i=0; i<nDrugCount; i++)
	{
		NexTech_Accessor::_FDBDrugDrugInteractionPtr ptrDrugInt = aryAllDrugDrugInteractions.GetAt(i);
		DrugInteractionDisplayFields eResult = GetDrugDrugInteractionSeverityInfo(ptrDrugInt->Severity);
		bool bAddInteraction = true;
		//if the severity is top-priority, always show, no matter what the preference says
		if(eResult.ePriority != soTopPriority) {
			if(ptrDrugInt->Severity == NexTech_Accessor::FDBDrugInteractionSeverity_ContraindicatedDrugCombination) {
				bAddInteraction = bShowContraindicatedDrugCombinations;
			}
			else if(ptrDrugInt->Severity == NexTech_Accessor::FDBDrugInteractionSeverity_SevereInteraction) {
				bAddInteraction = bShowSevereInteractions;
			}
			else if(ptrDrugInt->Severity == NexTech_Accessor::FDBDrugInteractionSeverity_ModerateInteraction) {
				bAddInteraction = bShowModerateInteractions;
			}
			else if(ptrDrugInt->Severity == NexTech_Accessor::FDBDrugInteractionSeverity_UndeterminedSeverity) {
				bAddInteraction = bShowUndeterminedSeverity;
			}
			else {
				//unknown severity, should be impossible, but if it does happen
				//don't throw an exception, just always show it
				ASSERT(FALSE);
			}
		}

		if(bAddInteraction) {
			aryFilteredDrugDrugInteractions.Add(ptrDrugInt);
		}
	}

	return aryFilteredDrugDrugInteractions;
}

// (j.jones 2013-05-14 09:06) - PLID 56634 - added functions to filter a list of "all" interactions
// down to a list of interactions that will be displayed, ignoring low-severity interactions that
// the user has chosen to suppress
Nx::SafeArray<IUnknown *> CDrugInteractionDlg::FilterDrugAllergyInteractionsBySeverity(Nx::SafeArray<IUnknown *> aryAllDrugAllergyInteractions)
{
	// (j.fouts 2013-05-20 10:17) - PLID 56571 - Added to categories to be more accurate
	bool bShowDirectIngredients = GetRemotePropertyInt("DISeverityFilter_Allergy_DirectIngredient", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowRelatedIngredients = GetRemotePropertyInt("DISeverityFilter_Allergy_RelatedIngredient", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowSevereGroupIngredients = GetRemotePropertyInt("DISeverityFilter_Allergy_GroupIngredient_Severe", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowModerateGroupIngredients = GetRemotePropertyInt("DISeverityFilter_Allergy_GroupIngredient_Moderate", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowCrossSensitiveIngredients = GetRemotePropertyInt("DISeverityFilter_Allergy_CrossSensitiveIngredient", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;

	Nx::SafeArray<IUnknown *> aryFilteredDrugAllergyInteractions;

	long nAllergyCount = aryAllDrugAllergyInteractions.GetCount();
	for(int i=0; i<nAllergyCount; i++)
	{
		NexTech_Accessor::_FDBDrugAllergyInteractionPtr ptrAllergyInt = aryAllDrugAllergyInteractions.GetAt(i);
		DrugInteractionDisplayFields eResult = GetDrugAllergyInteractionSeverityInfo(ptrAllergyInt->source, ptrAllergyInt->SeverityLevel);
		bool bAddInteraction = true;
		//if the severity is top-priority, always show, no matter what the preference says
		if(eResult.ePriority != soTopPriority) {
			switch(ptrAllergyInt->source)
			{
			case NexTech_Accessor::FDBAllergyInteractionSource_DirectIngredient:
				bAddInteraction = bShowDirectIngredients;
				break;
			case NexTech_Accessor::FDBAllergyInteractionSource_RelatedIngredient:
				bAddInteraction = bShowRelatedIngredients;
				break;
			case NexTech_Accessor::FDBAllergyInteractionSource_GroupIngredient:
				//Group has two possible severities, so we allow filtering on both
				if(ptrAllergyInt->SeverityLevel == NexTech_Accessor::DrugAllergySeverityLevel_Severe)
				{
					bAddInteraction = bShowSevereGroupIngredients;
				}
				else
				{
					bAddInteraction = bShowModerateGroupIngredients;
				}
				break;
			case NexTech_Accessor::FDBAllergyInteractionSource_CrossSensitiveIngredient:
				bAddInteraction = bShowCrossSensitiveIngredients;
				break;
			default:
				//unknown severity, should be impossible, but if it does happen
				//don't throw an exception, just always show it
				ASSERT(FALSE);
			}
		}

		if(bAddInteraction) {
			aryFilteredDrugAllergyInteractions.Add(ptrAllergyInt);
		}
	}

	return aryFilteredDrugAllergyInteractions;
}

// (j.jones 2013-05-14 09:06) - PLID 56634 - added functions to filter a list of "all" interactions
// down to a list of interactions that will be displayed, ignoring low-severity interactions that
// the user has chosen to suppress
Nx::SafeArray<IUnknown *> CDrugInteractionDlg::FilterDrugDiagnosisInteractionsBySeverity(Nx::SafeArray<IUnknown *> aryAllDrugDiagnosisInteractions)
{
	bool bShowAbsoluteContraindications = GetRemotePropertyInt("DISeverityFilter_Diag_AbsoluteContradiction", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowRelativeContraindications = GetRemotePropertyInt("DISeverityFilter_Diag_RelativeContraindication", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;
	bool bShowContraindicationWarnings = GetRemotePropertyInt("DISeverityFilter_Diag_ContraindicationWarning", 1, 0, GetCurrentUserName(), true) == 1 ? true : false;

	Nx::SafeArray<IUnknown *> aryFilteredDrugDiagnosisInteractions;

	long nDiagCount = aryAllDrugDiagnosisInteractions.GetCount();
	for(int i=0; i<nDiagCount; i++)
	{
		NexTech_Accessor::_FDBDrugDiagnosisInteractionPtr ptrDiagInt = aryAllDrugDiagnosisInteractions.GetAt(i);
		DrugInteractionDisplayFields eResult = GetDrugDiagnosisInteractionSeverityInfo(ptrDiagInt->Severity);
		bool bAddInteraction = true;
		//if the severity is top-priority, always show, no matter what the preference says
		if(eResult.ePriority != soTopPriority) {
			// (r.gonet 02/28/2014) - PLID 60755 - Fixed spelling.
			if(ptrDiagInt->Severity == NexTech_Accessor::FDBDiagnosisInteractionSeverity_AbsoluteContraindication) {
				bAddInteraction = bShowAbsoluteContraindications;
			}
			else if(ptrDiagInt->Severity == NexTech_Accessor::FDBDiagnosisInteractionSeverity_RelativeContraindication) {
				bAddInteraction = bShowRelativeContraindications;
			}
			else if(ptrDiagInt->Severity == NexTech_Accessor::FDBDiagnosisInteractionSeverity_ContraindicationWarning) {
				bAddInteraction = bShowContraindicationWarnings;
			}
			else {
				//unknown severity, should be impossible, but if it does happen
				//don't throw an exception, just always show it
				ASSERT(FALSE);
			}
		}

		if(bAddInteraction) {
			aryFilteredDrugDiagnosisInteractions.Add(ptrDiagInt);
		}
	}

	return aryFilteredDrugDiagnosisInteractions;
}

// (j.jones 2013-05-14 16:50) - PLID 56634 - added ability to toggle filtering on and off
void CDrugInteractionDlg::OnBtnToggleInteractionFilter()
{
	try {

		//just toggle the boolean, Refresh() will update the button text and icon
		//toggle the boolean, the button text, and the icon
		m_bShowFilteredInteractions = !m_bShowFilteredInteractions;

		//save this setting per user, so that it persists between sessions
		SetRemotePropertyInt("DISeverityFilters_FilterByDefault", m_bShowFilteredInteractions ? 1 : 0, 0, GetCurrentUserName());

		Refresh();

	}NxCatchAll(__FUNCTION__);
}