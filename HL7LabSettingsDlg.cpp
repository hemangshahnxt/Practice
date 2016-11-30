// HL7LabSettingsDlg.cpp : implementation file
//
//TES 5/20/2010 - PLID 38810 - Created, moved some lab-related HL7 settings to this dialog.

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7LabSettingsDlg.h"
#include "HL7ParseUtils.h"
#include "AuditTrail.h"
#include "HL7Utils.h"
#include "HL7ConfigCodeLinksDlg.h"
#include "ConfigObr24ValuesDlg.h"
#include "HL7CustomSegmentsDlg.h"

// CHL7LabSettingsDlg dialog

IMPLEMENT_DYNAMIC(CHL7LabSettingsDlg, CNxDialog)

CHL7LabSettingsDlg::CHL7LabSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7LabSettingsDlg::IDD, pParent)
{

}

CHL7LabSettingsDlg::~CHL7LabSettingsDlg()
{
}

void CHL7LabSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_OBR_15_COMPONENT, m_nxeOBR15Component);
	DDX_Control(pDX, IDC_USE_OBR_15, m_nxbUseOBR15);
	DDX_Control(pDX, IDC_USE_OBX_13, m_nxbUseOBX13);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_PATIENT_ID, m_nxbPatientID);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_PATIENT_NAME, m_nxbPatientName);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_FORM_NUMBER, m_nxbFormNumber);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_SSN, m_nxbSSN);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_BIRTH_DATE, m_nxbBirthDate);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_SPECIMEN, m_nxbSpecimen);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCH_TEST, m_nxbTestCode);
	DDX_Control(pDX, IDC_LAB_IMPORT_MATCHING_FIELDS_GROUP_BOX, m_nxstaticLabImportMatchingFieldGroupBox);
	DDX_Control(pDX, IDC_CONFIGURE_RESULT_FLAGS, m_nxbConfigureResultFlags);
	DDX_Control(pDX, IDC_CONFIGURE_QUALIFIERS, m_nxbConfigureQualifiers);
	DDX_Control(pDX, IDC_CONFIGURE_RESULT_FIELDS, m_nxbConfigureResultFields);
	DDX_Control(pDX, IDC_ALWAYS_COMBINE, m_nxbAlwaysCombine);
	DDX_Control(pDX, IDC_NEVER_COMBINE, m_nxbNeverCombine);
	DDX_Control(pDX, IDC_COMBINE_SUB_GROUPS, m_nxbCombineSubGroups);
	DDX_Control(pDX, IDC_COMBINE_BASED_ON_OBR_24, m_nxbCombineBasedOnObr24);
	DDX_Control(pDX, IDC_CONFIG_OBR_24_VALUES, m_nxbConfigObr24Values);
	DDX_Control(pDX, IDC_OBX_PARSE_SPECIMEN, m_nxbParseSpecimen);
	DDX_Control(pDX, IDC_BTN_HELP_PARSE_SPECIMENS, m_nxbHelpParseSpecimens);
	DDX_Control(pDX, IDC_COPY_RESULT_CHECK, m_nxbCopyResult);
	DDX_Control(pDX, IDC_HELP_COPY_RESULT_BUTTON, m_nxbHelpCopyResult);
	DDX_Control(pDX, IDC_USE_OBX_3_5, m_nxbUseObx3_5);
	DDX_Control(pDX, IDC_MSH4, m_nxbMSH4);
	DDX_Control(pDX, IDC_OBX23, m_nxbOBX23);
	DDX_Control(pDX, IDC_OBR21, m_nxbOBR21);
	DDX_Control(pDX, IDC_APPEND_OBR_20, m_nxbAppendObr20);
	DDX_Control(pDX, IDC_CONFIGURE_CUSTOM_ORDER_SEGMENTS, m_nxbEditCustomSegments);
	DDX_Control(pDX, IDC_OBR_4_LOINC, m_nxbObr4Loinc);
	DDX_Control(pDX, IDC_OBR_4_USE_TEXT, m_nxbObr4UseText);
	DDX_Control(pDX, IDC_OBR_4_TEXT, m_nxeObr4Text);
	DDX_Control(pDX, IDC_HL7_LABS_CONFIG_INS_CO_BTN, m_nxbConfigureInsCos);
	DDX_Control(pDX, IDC_BIOPSY_TYPE_OBR_13, m_nxbBiopsyTypeOBR13);
    DDX_Control(pDX, IDC_SEND_IN1_BEFORE_GT1, m_nxbSendIN1BeforeGT1);
	DDX_Control(pDX, IDC_SEND_NTE_BEFORE_DG1, m_nxbSendNTEBeforeDG1);
	DDX_Control(pDX, IDC_ONLY_SEND_FIRST_REP_PID_3, m_nxbOnlySendFirstRepetitionPID_3);
	DDX_Control(pDX, IDC_ONLY_SEND_FIRST_REP_PID_13, m_nxbOnlySendFirstRepetitionPID_13);
	DDX_Control(pDX, IDC_HL7LS_UNMATCHED_PROMPT_RADIO, m_nxbUnmatchedPromptRadio);
	DDX_Control(pDX, IDC_HL7LS_UNMATCHED_AUTOCREATE_RADIO, m_nxbUnmatchedAutocreateRadio);
	DDX_Control(pDX, IDC_FILL_PID_18_4, m_nxbFillPID_18_4);
	DDX_Control(pDX, IDC_OUTPUT_DG1_BEFORE_ORC_SEGMENTS, m_nxbOutputDG1BeforeORCSegments);
	DDX_Control(pDX, IDC_FILL_GT1_16, m_nxbFillGT1_16);
	DDX_Control(pDX, IDC_PATIENT_HISTORY_IMPORT_CHECK, m_chkPatientHistoryDocumentImportCheck);
	DDX_Control(pDX, IDC_REQUIRE_PERFORMING_LAB, m_nxbRequirePerformingLab);
	DDX_Control(pDX, IDC_MATCH_FORM_NUMBER_AND_SPECIMEN_RADIO, m_radioMatchFormNumberAndSpecimen);
	DDX_Control(pDX, IDC_MATCH_FORM_NUMBER_AND_TEST_CODE_RADIO, m_radioMatchFormNumberAndTestCode);
	DDX_Control(pDX, IDC_ENABLE_REFLEX_TESTING_CHECK, m_checkEnableReflexTesting);
	DDX_Control(pDX, IDC_PERFORMING_LAB_OBR_21, m_radioOBR21);
	DDX_Control(pDX, IDC_PERFORMING_LAB_OBX_15, m_radioOBX16);
	DDX_Control(pDX, IDC_ORDER_NUMBER_FORMAT_SPACE, m_nxbOrderNumberFormatSpace);
	DDX_Control(pDX, IDC_ORDER_NUMBER_FORMAT_NO_SPACE, m_nxbOrderNumberFormatNoSpace);
	DDX_Control(pDX, IDC_ORDER_NUMBER_FORMAT_NONE, m_nxbOrderNumberFormatNone);
	DDX_Control(pDX, IDC_FILL_OBR_18, m_nxbFillOBR18);
	DDX_Control(pDX, IDC_ALWAYS_SEND_GT1, m_nxbAlwaysSendGT1);
	DDX_Control(pDX, IDC_INCLUDE_ALL_INSURANCE, m_nxbIncludeAllInsurance);
	DDX_Control(pDX, IDC_PERFORMING_LAB_OBX_23, m_radioOBX23);
}


BEGIN_MESSAGE_MAP(CHL7LabSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_USE_OBR_15, &CHL7LabSettingsDlg::OnUseObr15)
	ON_BN_CLICKED(IDC_CONFIGURE_RESULT_FLAGS, &CHL7LabSettingsDlg::OnConfigureResultFlags)
	ON_BN_CLICKED(IDC_CONFIGURE_QUALIFIERS, &CHL7LabSettingsDlg::OnConfigureQualifiers)
	ON_BN_CLICKED(IDC_CONFIGURE_RESULT_FIELDS, &CHL7LabSettingsDlg::OnConfigureResultFields)
	ON_BN_CLICKED(IDC_ALWAYS_COMBINE, &CHL7LabSettingsDlg::OnAlwaysCombine)
	ON_BN_CLICKED(IDC_NEVER_COMBINE, &CHL7LabSettingsDlg::OnNeverCombine)
	ON_BN_CLICKED(IDC_COMBINE_SUB_GROUPS, &CHL7LabSettingsDlg::OnCombineSubGroups)
	ON_BN_CLICKED(IDC_COMBINE_BASED_ON_OBR_24, &CHL7LabSettingsDlg::OnCombineBasedOnObr24)
	ON_BN_CLICKED(IDC_CONFIG_OBR_24_VALUES, &CHL7LabSettingsDlg::OnConfigObr24Values)
	ON_BN_CLICKED(IDC_BTN_HELP_PARSE_SPECIMENS, &CHL7LabSettingsDlg::OnHelpParseSpecimens)
	ON_BN_CLICKED(IDC_HELP_COPY_RESULT_BUTTON, &CHL7LabSettingsDlg::OnBnClickedHelpCopyResultButton)
	ON_BN_CLICKED(IDC_CONFIGURE_CUSTOM_ORDER_SEGMENTS, &CHL7LabSettingsDlg::OnConfigureCustomOrderSegments)
	ON_BN_CLICKED(IDC_OBR_4_LOINC, &CHL7LabSettingsDlg::OnObr4Loinc)
	ON_BN_CLICKED(IDC_OBR_4_USE_TEXT, &CHL7LabSettingsDlg::OnObr4UseText)
	ON_BN_CLICKED(IDC_HL7_LABS_CONFIG_INS_CO_BTN, &CHL7LabSettingsDlg::OnBnClickedHl7LabsConfigInsCoBtn)
	ON_BN_CLICKED(IDC_MATCH_FORM_NUMBER_AND_SPECIMEN_RADIO, &CHL7LabSettingsDlg::OnBnClickedMatchFormNumberAndSpecimenRadio)
	ON_BN_CLICKED(IDC_MATCH_FORM_NUMBER_AND_TEST_CODE_RADIO, &CHL7LabSettingsDlg::OnBnClickedMatchFormNumberAndTestRadio)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHL7LabSettingsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHL7LabSettingsDlg)
	ON_EVENT(CHL7LabSettingsDlg, IDC_PROVIDER_ID_TYPE_COMBO, 1 /* SelChanging */, OnSelChangingProviderIDTypeCombo, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


// CHL7LabSettingsDlg message handlers
BOOL CHL7LabSettingsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try { 
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 5/20/2010 - PLID 38810 - Load the settings that are on this dialog.

		//TES 4/6/2010 - PLID 38040 - Reflect the setting for which OBR-15 component to pull the Anatomic Location from.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		long nUseObr15Component = GetHL7SettingInt(m_nHL7GroupID, "AnatomicLocationComponent");
		if(nUseObr15Component == -1) {
			//TES 4/6/2010 - PLID 38040 - They don't want to pull it.
			m_nxbUseOBR15.SetCheck(BST_UNCHECKED);
			m_nxeOBR15Component.SetWindowText("");
			m_nxeOBR15Component.EnableWindow(FALSE);
		}
		else {
			m_nxbUseOBR15.SetCheck(BST_CHECKED);
			m_nxeOBR15Component.SetWindowText(AsString(nUseObr15Component));
			m_nxeOBR15Component.EnableWindow(TRUE);
		}

		//TES 2/9/2010 - PLID 37268 - Use OBX-13 option
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		m_nxbUseOBX13.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "UseOBX13") ? BST_CHECKED : BST_UNCHECKED);

		//TES 2/23/2010 - PLID 37503 - Added setting to pull the Receiving Lab from OBX
		//m_nxbUseOBXLocation.SetCheck(CHL7Settings::GetUseOBXLocation(m_nHL7GroupID) ? BST_CHECKED : BST_UNCHECKED);
		//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		ReceivingLabLocation rll = (ReceivingLabLocation)GetHL7SettingInt(m_nHL7GroupID, "ReceivingLabLocation");
		switch(rll) {
			case rllMSH4:
				CheckDlgButton(IDC_MSH4, BST_CHECKED);
				break;
			case rllOBX23:
				CheckDlgButton(IDC_OBX23, BST_CHECKED);
				break;
			case rllOBR21:
				CheckDlgButton(IDC_OBR21, BST_CHECKED);
				break;
		}


		// (c.haag 2010-09-16 12:02) - PLID 40176 - Lab specimen parsing method
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		m_nxbParseSpecimen.SetCheck(GetHL7SettingInt(m_nHL7GroupID, "LabSpecimenParseMethod") ? BST_CHECKED : BST_UNCHECKED);

		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		m_nxbCopyResult.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "CopyResult") ? BST_CHECKED : BST_UNCHECKED);

		//TES 7/30/2010 - PLID 39908 - The settings for whether to combine text segments are now radio buttons, 
		// with a new "Check OBR-24" option.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		CombineTextSegmentsOptions ctso = (CombineTextSegmentsOptions)GetHL7SettingInt(m_nHL7GroupID, "CombineTextSegments");
		//TES 8/4/2010 - PLID 39908 - Don't use CheckRadioButton(), ever, it uses the numerical values that the IDCs are #defined as to 
		// determine what's in the group, instead of the tab order and Group settings, so it can wind up unchecking random other buttons.
		switch(ctso) {
			case ctsoCheckSubGroupID:
				CheckDlgButton(IDC_COMBINE_SUB_GROUPS, BST_CHECKED);
				break;
			case ctsoAlwaysCombine:
				CheckDlgButton(IDC_ALWAYS_COMBINE, BST_CHECKED);
				break;
			case ctsoNeverCombine:
				CheckDlgButton(IDC_NEVER_COMBINE, BST_CHECKED);
				break;
			case ctsoCheckObr24:
				CheckDlgButton(IDC_COMBINE_BASED_ON_OBR_24, BST_CHECKED);
				break;
		}
		HandleCombineButton();
		//TES 7/30/2010 - PLID 39908 - Load the OBR-24 values from data, they're not stored on screen so we track them separately.
		GetOBR24Values(m_nHL7GroupID, m_saObr24Values);

		//TES 4/25/2011 - PLID 43423 - Load the "Use OBX-3.5" setting
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		m_nxbUseObx3_5.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "UseOBX3_5")?BST_CHECKED:BST_UNCHECKED);

		//TES 5/11/2011 - PLID 43634 - Load the Append OBR-20 setting
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		m_nxbAppendObr20.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "AppendOBR20")?BST_CHECKED:BST_UNCHECKED);

		// (z.manning 2011-10-03 10:17) - PLID 45724
		m_nxbBiopsyTypeOBR13.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "BiopsyTypeOBR13") ? BST_CHECKED : BST_UNCHECKED);

		//TES 9/19/2011 - PLID 45542 - Settings for how to fill OBR-4
		if(GetHL7SettingBit(m_nHL7GroupID, "OBR_4_LOINC")) {
			CheckRadioButton(IDC_OBR_4_LOINC, IDC_OBR_4_USE_TEXT, IDC_OBR_4_LOINC);
		}
		else {
			CheckRadioButton(IDC_OBR_4_LOINC, IDC_OBR_4_USE_TEXT, IDC_OBR_4_USE_TEXT);
		}
		//TES 10/30/2011 - PLID 45542 - Yes, somebody tested putting > 3000 characters in this box.
		m_nxeObr4Text.SetLimitText(3000);
		SetDlgItemText(IDC_OBR_4_TEXT, GetHL7SettingText(m_nHL7GroupID, "OBR_4_Text"));
		ReflectObr4();
		
		// (z.manning 2010-05-21 10:32) - PLID 38638 - Load the lab import matching field settings
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		DWORD dwLabMatchingFieldFlags = (DWORD)GetHL7SettingInt(m_nHL7GroupID, "LabImportMatchingFieldFlags");
		if((dwLabMatchingFieldFlags & limfPatientID) == limfPatientID) { m_nxbPatientID.SetCheck(BST_CHECKED); }
		if((dwLabMatchingFieldFlags & limfPatientName) == limfPatientName) { m_nxbPatientName.SetCheck(BST_CHECKED); }
		if((dwLabMatchingFieldFlags & limfFormNumber) == limfFormNumber) { m_nxbFormNumber.SetCheck(BST_CHECKED); }
		if((dwLabMatchingFieldFlags & limfSSN) == limfSSN) { m_nxbSSN.SetCheck(BST_CHECKED); }
		if((dwLabMatchingFieldFlags & limfBirthDate) == limfBirthDate) { m_nxbBirthDate.SetCheck(BST_CHECKED); }
		// (r.gonet 03/07/2013) - PLID 55527 - We consider the specimen a separate entitiy from the form number now.
		if((dwLabMatchingFieldFlags & limfSpecimen) == limfSpecimen) { m_nxbSpecimen.SetCheck(BST_CHECKED); }
		// (r.gonet 03/07/2013) - PLID 55527 - We can now match on the test code 
		if((dwLabMatchingFieldFlags & limfTestCode) == limfTestCode) { m_nxbTestCode.SetCheck(BST_CHECKED); }

        // (j.kuziel 2011-10-14 16:44) - PLID 41419
        m_nxbSendIN1BeforeGT1.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "SendIN1BeforeGT1OnORM") ? BST_CHECKED : BST_UNCHECKED);

		// (r.gonet 02/28/2012) - PLID 48044 - Send the NTE segments before the DG1 segment. This is correct by the HL7 standard,
		//  but we retain the old way for legacy interfaces.
		m_nxbSendNTEBeforeDG1.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "SendNTEBeforeDG1") ? BST_CHECKED : BST_UNCHECKED); 

		// (r.gonet 02/28/2012) - PLID 48042 - Only send the first repetition of PID-3
		m_nxbOnlySendFirstRepetitionPID_3.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "OnlySendFirstRepetition_PID_3") ? BST_CHECKED : BST_UNCHECKED);

		// (r.gonet 02/28/2012) - PLID 48454 - Only send the first repetition of PID-13
		m_nxbOnlySendFirstRepetitionPID_13.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "OnlySendFirstRepetition_PID_13") ? BST_CHECKED : BST_UNCHECKED);

		// (r.gonet 05/07/2012) - PLID 48606 - Don't send the spaces around the dash separator between the form number and the specimen label
		//m_nxbDontSendSpacesInOrderNumberSeparator.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "DontSendSpacesInOrderNumberSeparator") ? BST_CHECKED : BST_UNCHECKED);
		//TES 6/24/2013 - PLID 57288 - The checkbox to send spaces has been replaced by an enum with different formatting options
		long nOrderNumberFormat = GetHL7SettingInt(m_nHL7GroupID, "OrderNumberSeparatorFormat");
		switch(nOrderNumberFormat) {
			case onfLegacySetting:
				//TES 6/24/2013 - PLID 57288 - This is the default, and it signifies that we should check the old setting
				if(GetHL7SettingBit(m_nHL7GroupID, "DontSendSpacesInOrderNumberSeparator")) {
					m_nxbOrderNumberFormatNoSpace.SetCheck(BST_CHECKED);
				}
				else {
					m_nxbOrderNumberFormatSpace.SetCheck(BST_CHECKED);
				}
				break;
			case onfDashAndSpace:
				m_nxbOrderNumberFormatSpace.SetCheck(BST_CHECKED);
				break;
			case onfDashNoSpace:
				m_nxbOrderNumberFormatNoSpace.SetCheck(BST_CHECKED);
				break;
			case onfNoDash:
				m_nxbOrderNumberFormatNone.SetCheck(BST_CHECKED);
				break;
			default:
				ASSERT(FALSE);
				m_nxbOrderNumberFormatSpace.SetCheck(BST_CHECKED);
				break;
		}

		// (r.gonet 06/05/2012) - PLID 50629 - Set the radios for the unmatched lab result behavior
		switch(GetHL7SettingInt(m_nHL7GroupID, "UnmatchedLabBehavior")) {
			case 0:
				m_nxbUnmatchedPromptRadio.SetCheck(BST_CHECKED);
				break;
			case 1:
				m_nxbUnmatchedAutocreateRadio.SetCheck(BST_CHECKED);
				break;
			default:
				ASSERT(FALSE); // Missing a handler
				m_nxbUnmatchedPromptRadio.SetCheck(BST_CHECKED);
				break;
		}
		
		//TES 7/26/2012 - PLID 51110 - Setting to fill in the 4th component of PID-18 with a LabCorp-specific "Bill Type" code.
		m_nxbFillPID_18_4.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "FillPID_18_4") ? BST_CHECKED : BST_UNCHECKED);

		//TES 7/26/2012 - PLID 51111 - Setting to output the DG1 segments before the specimen-specific statements.
		m_nxbOutputDG1BeforeORCSegments.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "OutputDG1BeforeORCSegments") ? BST_CHECKED : BST_UNCHECKED);

		// (r.gonet 08/24/2012) - PLID 52013 - Setting to control what IDs are sent and received for providers (in OBR-16.1 and ORC-12.1)
		m_pProviderIDTypes = BindNxDataList2Ctrl(IDC_PROVIDER_ID_TYPE_COMBO, false);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderIDTypes->GetNewRow();
		pRow->PutValue(epidtID, epridInternalID);
		pRow->PutValue(epidtName, "Practice Internal ID");
		m_pProviderIDTypes->AddRowAtEnd(pRow, NULL);
		pRow = m_pProviderIDTypes->GetNewRow();
		pRow->PutValue(epidtID, epridNPI);
		pRow->PutValue(epidtName, "NPI");
		m_pProviderIDTypes->AddRowAtEnd(pRow, NULL);
		pRow = m_pProviderIDTypes->GetNewRow();
		pRow->PutValue(epidtID, epridUPIN);
		pRow->PutValue(epidtName, "UPIN");
		m_pProviderIDTypes->AddRowAtEnd(pRow, NULL);
		pRow = m_pProviderIDTypes->GetNewRow();
		pRow->PutValue(epidtID, epridThirdPartyID);
		// This one is a catch all and is useful if they have some special values they want us to map to the provider.
		pRow->PutValue(epidtName, "Third Party ID");
		m_pProviderIDTypes->AddRowAtEnd(pRow, NULL);

		EProviderIDType epridType = (EProviderIDType)GetHL7SettingInt(m_nHL7GroupID, "ProviderIDType");
		m_pProviderIDTypes->FindByColumn(epidtID, _variant_t((long)epridType, VT_I4), m_pProviderIDTypes->GetFirstRow(), VARIANT_TRUE);

		//TES 11/27/2012 - PLID 53914 - Setting to send the employer in GT1-16
		m_nxbFillGT1_16.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "FillGT1_16") ? BST_CHECKED : BST_UNCHECKED);

		// (b.spivey, January 22, 2013) - PLID 54751 - Load saved data
		m_chkPatientHistoryDocumentImportCheck.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "UsePatientHistoryImportMSH_3") ? BST_CHECKED : BST_UNCHECKED);

		//TES 2/25/2013 - PLID 54876 - Preference to force the PerformingLabID field to be mapped based on the information in OBR-21
		m_nxbRequirePerformingLab.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "RequirePerformingLab") ? BST_CHECKED : BST_UNCHECKED);

		//TES 5/17/2013 - PLID 56286 - We can now read Performing Lab information out of OBX-15 instead of OBR-21
		// (d.singleton 2013-10-25 10:45) - PLID 59181 - need new option to pull performing lab from obx23
		long nPerformingLabObr21 = GetHL7SettingInt(m_nHL7GroupID, "PerformingLab_OBR_21");		
		if(nPerformingLabObr21 == (long)plcOBR21) {
			CheckDlgButton(IDC_PERFORMING_LAB_OBR_21, BST_CHECKED);
		}
		else if(nPerformingLabObr21 == (long)plcOBX15) {
			CheckDlgButton(IDC_PERFORMING_LAB_OBX_15, BST_CHECKED);
		}
		else if(nPerformingLabObr21 == (long)plcOBX23) {
			CheckDlgButton(IDC_PERFORMING_LAB_OBX_23, BST_CHECKED);
		}

		// (r.gonet 03/07/2013) - PLID 55527 - The user has access to two radio buttons which set 
		//  a number of hidden checkboxes. We don't want the user to have access to those. (I'm not
		//  sure why they keep them around actually since they could be replaced by member variables).
		if((dwLabMatchingFieldFlags & limfFormNumber) == limfFormNumber &&
			(dwLabMatchingFieldFlags & limfSpecimen) == limfSpecimen &&
			(dwLabMatchingFieldFlags & limfTestCode) != limfTestCode) 
		{ 
			// (r.gonet 03/07/2013) - PLID 55527 - We're matching on form number and specimen and not on the test code.
			m_radioMatchFormNumberAndSpecimen.SetCheck(BST_CHECKED); 
		} else if((dwLabMatchingFieldFlags & limfFormNumber) == limfFormNumber &&
			(dwLabMatchingFieldFlags & limfSpecimen) != limfSpecimen &&
			(dwLabMatchingFieldFlags & limfTestCode) == limfTestCode)
		{
			// (r.gonet 03/07/2013) - PLID 55527 - We're matching on form number and test code and not the specimen.
			m_radioMatchFormNumberAndTestCode.SetCheck(BST_CHECKED);
		} else {
			// (r.gonet 03/07/2013) - PLID 55527 - Something custom is going on. Don't set either radio button in this case...
		}

		// (r.gonet 03/07/2013) - PLID 55489 - Show the current state of reflex testing support.
		m_checkEnableReflexTesting.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "EnableReflexTesting") ? BST_CHECKED : BST_UNCHECKED);

		//TES 6/24/2013 - PLID 57293 - OBR-18 is now optional
		m_nxbFillOBR18.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "FillObr18") ? BST_CHECKED : BST_UNCHECKED);

		//TES 6/24/2013 - PLID 57294 - Added option to send GT1 segment even for labs with no insurance
		m_nxbAlwaysSendGT1.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "AlwaysSendGT1") ? BST_CHECKED : BST_UNCHECKED);

		//TES 6/24/2013 - PLID 57295 - Added option to send information about all of a patient's insurance companies
		m_nxbIncludeAllInsurance.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "IncludeAllInsuranceOnLabs") ? BST_CHECKED : BST_UNCHECKED);
		
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7LabSettingsDlg::OnUseObr15()
{
	try {
		//TES 5/20/2010 - PLID 38810 - Enable the edit box for the component appropriately.
		if(m_nxbUseOBR15.GetCheck() == BST_CHECKED) {
			m_nxeOBR15Component.EnableWindow(TRUE);
			m_nxeOBR15Component.SetFocus();
		}
		else {
			m_nxeOBR15Component.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);

}

void CHL7LabSettingsDlg::OnOK()
{
	try {

		CAuditTransaction auditTran;

		// (z.manning 2010-05-21 10:30) - PLID 38638 - Save the fields that need to match when auto importing lab results
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		DWORD dwOldLabImportMatchingField = (DWORD)GetHL7SettingInt(m_nHL7GroupID, "LabImportMatchingFieldFlags");
		DWORD dwLabImportMatchingField = 0;
		if(m_nxbPatientID.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfPatientID; }
		if(m_nxbPatientName.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfPatientName; }
		if(m_nxbFormNumber.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfFormNumber; }
		if(m_nxbSSN.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfSSN; }
		if(m_nxbBirthDate.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfBirthDate; }
		// (r.gonet 03/07/2013) - PLID 55527 - Save whether we want to match on the specimen label.
		if(m_nxbSpecimen.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfSpecimen; }
		// (r.gonet 03/07/2013) - PLID 55527 - Save whether we want to match on the test code.
		if(m_nxbTestCode.GetCheck() == BST_CHECKED) { dwLabImportMatchingField |= limfTestCode; }

		if(dwLabImportMatchingField != dwOldLabImportMatchingField) {
			// (z.manning 2010-05-21 13:45) - PLID 38638 - Audit changes to the lab import required matching fields
			CString strOld = GetLabImportMatchingFieldAuditText(dwOldLabImportMatchingField);
			CString strNew = GetLabImportMatchingFieldAuditText(dwLabImportMatchingField);
			AuditEvent(-1, "", auditTran, aeiHL7LabImportMatchingFields, m_nHL7GroupID, strOld, strNew, aepHigh, aetChanged);
		}

		//TES 5/20/2010 - PLID 38810 - Get the value for our OBR-15 component (NULL unless the box is checked
		// and a valid number is entered).
		_variant_t varObr15Component = g_cvarNull;
		if(IsDlgButtonChecked(IDC_USE_OBR_15)) {
			CString strComponent;
			GetDlgItemText(IDC_OBR_15_COMPONENT, strComponent);
			if(!strComponent.IsEmpty()) {
				varObr15Component = atol(strComponent);
			}
		}

		//TES 7/30/2010 - PLID 39908 - Pull the CombineTextSegments option from the radio buttons.
		CombineTextSegmentsOptions ctso;
		if(m_nxbCombineSubGroups.GetCheck()) {
			ctso = ctsoCheckSubGroupID;
		}
		else if(m_nxbAlwaysCombine.GetCheck()) {
			ctso = ctsoAlwaysCombine;
		}
		else if(m_nxbNeverCombine.GetCheck()) {
			ctso = ctsoNeverCombine;
		}
		else if(m_nxbCombineBasedOnObr24.GetCheck()) {
			ctso = ctsoCheckObr24;
		}
		else {
			ASSERT(FALSE);
			ctso = ctsoCheckSubGroupID;
		}
		
		//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
		ReceivingLabLocation rll;
		if(m_nxbMSH4.GetCheck()) {
			rll = rllMSH4;
		}
		else if(m_nxbOBX23.GetCheck()) {
			rll = rllOBX23;
		}
		else if(m_nxbOBR21.GetCheck()) {
			rll = rllOBR21;
		}
		else {
			ASSERT(FALSE);
			rll = rllMSH4;
		}

		// (r.gonet 06/05/2012) - PLID 50629
		UnmatchedLabBehavior ulb;
		if(m_nxbUnmatchedPromptRadio.GetCheck()) {
			ulb = ulbPrompt;
		}
		else if(m_nxbUnmatchedAutocreateRadio.GetCheck()) {
			ulb = ulbAutocreateOrder;
		}
		else {
			ASSERT(FALSE);
			ulb = ulbPrompt;
		}

		//TES 7/30/2010 - PLID 39908 - Store the OBR-24 values they've selected; convert the CStringArray to a |-delimited string.
		CString strObr24Values;
		for(int i = 0; i < m_saObr24Values.GetSize(); i++) {
			strObr24Values += m_saObr24Values[i] + "|";
		}

		// (b.spivey, January 23, 2013) - PLID 54751 - Save the right values, mimics existing functionality
		//Big BOOL. 
		BOOL bPatientHistoryFlag = FALSE;
		if (IsDlgButtonChecked(IDC_PATIENT_HISTORY_IMPORT_CHECK) == BST_CHECKED) {
			bPatientHistoryFlag = TRUE;
		}
		else {
			bPatientHistoryFlag = FALSE;
		}

		// (d.singleton 2013-10-25 10:45) - PLID 59181 - need new option to pull performing lab from obx23
		PerformingLabComponent plc;
		if(IsDlgButtonChecked(IDC_PERFORMING_LAB_OBR_21)) {
			plc = plcOBR21;
		}
		else if(IsDlgButtonChecked(IDC_PERFORMING_LAB_OBX_15)) {
			plc = plcOBX15;
		}
		else if(IsDlgButtonChecked(IDC_PERFORMING_LAB_OBX_23)) {
			plc = plcOBX23;
		}
		

		//TES 5/20/2010 - PLID 38810 - Update all values in data.
		//TES 6/11/2010 - PLID 38541 - Added LabFacilityID
		//TES 7/30/2010 - PLID 39908 - Added Obr24Values
		// (c.haag 2010-09-16 12:02) - PLID 40176 - Added lab specimen parsing method
		// (z.manning 2010-10-04 09:19) - PLID 40777 - Removed LabFacilityID
		// (r.gonet 2011-03-09 10:53) - PLID 42655 - Added saving of Copy Result
		//TES 4/25/2011 - PLID 43423 - Added UseOBX3_5
		//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
		//TES 5/11/2011 - PLID 43634 - Added AppendObr20
		//TES 6/23/2011 - PLID 44261 - We've got new functions for accessing the HL7 Settings, use them to build a batch query, then execute.
		CString strSql;
		CNxParamSqlArray aryParams;
		SetHL7SettingInt(m_nHL7GroupID, "AnatomicLocationComponent", VarLong(varObr15Component, -1), &strSql, &aryParams);
		SetHL7SettingBit(m_nHL7GroupID, "UseOBX13", m_nxbUseOBX13.GetCheck() == BST_CHECKED, &strSql, &aryParams);
		SetHL7SettingInt(m_nHL7GroupID, "ReceivingLabLocation", (long)rll, &strSql, &aryParams);
		SetHL7SettingInt(m_nHL7GroupID, "LabImportMatchingFieldFlags", (long)dwLabImportMatchingField, &strSql, &aryParams);
		SetHL7SettingInt(m_nHL7GroupID, "CombineTextSegments", (long)ctso, &strSql, &aryParams);
		SetHL7SettingText(m_nHL7GroupID, "Obr24Values", strObr24Values, &strSql, &aryParams);
		SetHL7SettingInt(m_nHL7GroupID, "LabSpecimenParseMethod", m_nxbParseSpecimen.GetCheck() == BST_CHECKED ? 1 : 0, &strSql, &aryParams);
		SetHL7SettingBit(m_nHL7GroupID, "CopyResult", m_nxbCopyResult.GetCheck() == BST_CHECKED, &strSql, &aryParams);
		SetHL7SettingBit(m_nHL7GroupID, "UseOBX3_5", m_nxbUseObx3_5.GetCheck() == BST_CHECKED, &strSql, &aryParams);
		SetHL7SettingBit(m_nHL7GroupID, "AppendObr20", m_nxbAppendObr20.GetCheck() == BST_CHECKED, &strSql, &aryParams);
		//TES 9/19/2011 - PLID 45542 - Added options for how to fill OBR-4
		SetHL7SettingBit(m_nHL7GroupID, "OBR_4_LOINC", m_nxbObr4Loinc.GetCheck() == BST_CHECKED, &strSql, &aryParams);
		CString strObr4Text;
		GetDlgItemText(IDC_OBR_4_TEXT, strObr4Text);
		SetHL7SettingText(m_nHL7GroupID, "OBR_4_Text", strObr4Text);
		// (z.manning 2011-10-03 10:19) - PLID 45724 - BiopsyTypeOBR13
		SetHL7SettingBit(m_nHL7GroupID, "BiopsyTypeOBR13", m_nxbBiopsyTypeOBR13.GetCheck() == BST_CHECKED, &strSql, &aryParams);
        // (j.kuziel 2011-10-14 16:48) - PLID 41419
        SetHL7SettingBit(m_nHL7GroupID, "SendIN1BeforeGT1OnORM", m_nxbSendIN1BeforeGT1.GetCheck() == BST_CHECKED, &strSql, &aryParams);
		// (r.gonet 02/28/2012) - PLID 48044 - Added an option to change the ordering of the ORDER_DETAILS segments, DG1 and NTE
		SetHL7SettingBit(m_nHL7GroupID, "SendNTEBeforeDG1", IsDlgButtonChecked(IDC_SEND_NTE_BEFORE_DG1));
		// (r.gonet 02/28/2012) - PLID 48042 - Added an option to disable sending of multiple repetitions of PID-3.
		SetHL7SettingBit(m_nHL7GroupID, "OnlySendFirstRepetition_PID_3", IsDlgButtonChecked(IDC_ONLY_SEND_FIRST_REP_PID_3));
		// (r.gonet 02/28/2012) - PLID 48454 - Added an option to disable sending of multiple repetitions of PID-13.
		SetHL7SettingBit(m_nHL7GroupID, "OnlySendFirstRepetition_PID_13", IsDlgButtonChecked(IDC_ONLY_SEND_FIRST_REP_PID_13));
		// (r.gonet 05/07/2012) - PLID 48606 - Don't send the spaces around the dash separator between the form number and the specimen label
		//SetHL7SettingBit(m_nHL7GroupID, "DontSendSpacesInOrderNumberSeparator", IsDlgButtonChecked(IDC_HL7LS_DONT_SEND_SPACES_AROUND_DASH_CHECK));
		//TES 6/24/2013 - PLID 57288 - The checkbox to send spaces has been replaced by an enum with different formatting options
		if(IsDlgButtonChecked(IDC_ORDER_NUMBER_FORMAT_SPACE)) {
			SetHL7SettingInt(m_nHL7GroupID, "OrderNumberSeparatorFormat", onfDashAndSpace, &strSql, &aryParams);
		}
		else if(IsDlgButtonChecked(IDC_ORDER_NUMBER_FORMAT_NO_SPACE)) {
			SetHL7SettingInt(m_nHL7GroupID, "OrderNumberSeparatorFormat", onfDashNoSpace, &strSql, &aryParams);
		}
		else if(IsDlgButtonChecked(IDC_ORDER_NUMBER_FORMAT_NONE)) {
			SetHL7SettingInt(m_nHL7GroupID, "OrderNumberSeparatorFormat", onfNoDash, &strSql, &aryParams);
		}
		else {
			ASSERT(FALSE);
		}
		// (r.gonet 06/05/2012) - PLID 50629 - Save the behavior to use when getting results that don't match anything in our data.
		SetHL7SettingInt(m_nHL7GroupID, "UnmatchedLabBehavior", (long)ulb, &strSql, &aryParams);
		//TES 7/26/2012 - PLID 51110 - Setting to fill in the 4th component of PID-18 with a LabCorp-specific "Bill Type" code.
		SetHL7SettingBit(m_nHL7GroupID, "FillPID_18_4", IsDlgButtonChecked(IDC_FILL_PID_18_4), &strSql, &aryParams);
		//TES 7/26/2012 - PLID 51111 - Setting to output the DG1 segments before the specimen-specific statements.
		SetHL7SettingBit(m_nHL7GroupID, "OutputDG1BeforeORCSegments", IsDlgButtonChecked(IDC_OUTPUT_DG1_BEFORE_ORC_SEGMENTS), &strSql, &aryParams);
		// (r.gonet 08/24/2012) - PLID 52013 - Setting to control what IDs are sent and received for providers (in OBR-16.1 and ORC-12.1)
		SetHL7SettingInt(m_nHL7GroupID, "ProviderIDType", m_pProviderIDTypes->CurSel ? (EProviderIDType)VarLong(m_pProviderIDTypes->CurSel->GetValue(epidtID)) : epridInternalID);
		//TES 11/27/2012 - PLID 53914 - Setting to send the employer in GT1-16
		SetHL7SettingBit(m_nHL7GroupID, "FillGT1_16", IsDlgButtonChecked(IDC_FILL_GT1_16), &strSql, &aryParams);
		// (b.spivey, January 22, 2013) - PLID 54751 - Save changes
		SetHL7SettingBit(m_nHL7GroupID, "UsePatientHistoryImportMSH_3", bPatientHistoryFlag, &strSql, &aryParams);
		//TES 2/25/2013 - PLID 54876 - Preference to force the PerformingLabID field to be mapped based on the information in OBR-21
		SetHL7SettingBit(m_nHL7GroupID, "RequirePerformingLab", IsDlgButtonChecked(IDC_REQUIRE_PERFORMING_LAB), &strSql, &aryParams);
		// (r.gonet 03/07/2013) - PLID 55489 - Save the preference to enable support for reflex testing.
		SetHL7SettingBit(m_nHL7GroupID, "EnableReflexTesting", IsDlgButtonChecked(IDC_ENABLE_REFLEX_TESTING_CHECK));
		//TES 5/17/2013 - PLID 56286 - We can now read Performing Lab information out of OBX-15 instead of OBR-21
		// (d.singleton 2013-10-25 10:45) - PLID 59181 - need new option to pull performing lab from obx23
		SetHL7SettingInt(m_nHL7GroupID, "PerformingLab_OBR_21", (long)plc);		
		//TES 6/24/2013 - PLID 57293 - OBR-18 is now optional
		SetHL7SettingBit(m_nHL7GroupID, "FillObr18", IsDlgButtonChecked(IDC_FILL_OBR_18), &strSql, &aryParams);
		//TES 6/24/2013 - PLID 57294 - Added option to send GT1 even for labs with no insurance
		SetHL7SettingBit(m_nHL7GroupID, "AlwaysSendGT1", IsDlgButtonChecked(IDC_ALWAYS_SEND_GT1), &strSql, &aryParams);
		//TES 6/24/2013 - PLID 57295 - Added option to include all of the patient's insurance companies
		SetHL7SettingBit(m_nHL7GroupID, "IncludeAllInsuranceOnLabs", IsDlgButtonChecked(IDC_INCLUDE_ALL_INSURANCE), &strSql, &aryParams);

		ExecuteParamSqlBatch(GetRemoteData(), strSql, aryParams);

		//TES 5/20/2010 - PLID 38810 - Let NxServer and the global HL7Settings object know that we've changed.
		CClient::RefreshTable(NetUtils::HL7SettingsT, m_nHL7GroupID);
		RefreshHL7Group(m_nHL7GroupID);

		auditTran.Commit();

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}
void CHL7LabSettingsDlg::OnConfigureResultFlags()
{
	try {
		//TES 5/26/2010 - PLID 38660 - Pop up the CHL7ConfigCodeLinksDlg, telling it that we're interested
		// in result flags.
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtLabResultFlag;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnConfigureQualifiers()
{
	try {
		//TES 6/1/2010 - PLID 38066 - Pop up the CHL7ConfigCodeLinksDlg, telling it that we're interested
		// in anatomic sides and qualifiers.
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtLabSideAndQual;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnConfigureResultFields()
{
	try {
		//TES 6/3/2010 - PLID 38776 - Pop up the CHL7ConfigCodeLinksDlg, telling it that we're interested
		// in result fields
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtLabResultField;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnCombineTextSegments()
{
	try {
		//TES 6/14/2010 - PLID 39135 - If this is checked, uncheck the "Never combine" checkbox.
		if(IsDlgButtonChecked(IDC_COMBINE_TEXT_SEGMENTS)) {
			CheckDlgButton(IDC_NEVER_COMBINE_TEXT_SEGMENTS, BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnNeverCombineTextSegments()
{
	try {
		//TES 6/14/2010 - PLID 39135 - If this is checked, uncheck the "Always combine" checkbox.
		if(IsDlgButtonChecked(IDC_NEVER_COMBINE_TEXT_SEGMENTS)) {
			CheckDlgButton(IDC_COMBINE_TEXT_SEGMENTS, BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnAlwaysCombine()
{
	try {
		HandleCombineButton();
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnNeverCombine()
{
	try {
		HandleCombineButton();
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnCombineSubGroups()
{
	try {
		HandleCombineButton();
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnCombineBasedOnObr24()
{
	try {
		HandleCombineButton();
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::HandleCombineButton()
{
	//TES 7/30/2010 - PLID 39908 - Enable the button to configure OBR-24 values, iff that radio button is checked.
	if(m_nxbCombineBasedOnObr24.GetCheck()) {
		m_nxbConfigObr24Values.EnableWindow(TRUE);
	}
	else {
		m_nxbConfigObr24Values.EnableWindow(FALSE);
	}
}
void CHL7LabSettingsDlg::OnConfigObr24Values()
{
	try {
		//TES 7/30/2010 - PLID 39908 - Load the values we have into the dialog.
		CConfigObr24ValuesDlg dlg(this);
		dlg.m_saValues.Copy(m_saObr24Values);
		if(IDOK == dlg.DoModal()) {
			//TES 7/30/2010 - PLID 39908 - Remember the values they selected.
			m_saObr24Values.RemoveAll();
			m_saObr24Values.Copy(dlg.m_saValues);
		}
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-09-16 11:58) - PLID 40176 - Explains how specimens are parsed
// (j.kuziel 2014-05-02) - PLID 60396 - Changed to include X number patterns.
void CHL7LabSettingsDlg::OnHelpParseSpecimens()
{
	try {
		AfxMessageBox(
			"When this setting is enabled, Practice will look for order numbers that end with "
			"a \" - X\" pattern where X is any letter of the alphabet or a 1 to 2 digit number. "
			"If the pattern is found, then the value of X is considered to be a specimen."
			"\n\nWhen the lab is created in Practice, the specimen will be removed from "
			"the form number and placed into the specimen field of the lab.",
			MB_ICONINFORMATION | MB_OK);
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 2011-03-09 10:53) - PLID 42655 - Displays contextual help message for the Copy Result option
void CHL7LabSettingsDlg::OnBnClickedHelpCopyResultButton()
{
	try {
		AfxMessageBox(
			"When this setting is enabled, Practice will copy the result OBR from messages with a single "
			"OBR segment to all of the specimens in the lab it gets attached to. This setting is intended "
			"as a workaround for labs that cannot send per-specimen results and instead send results for all "
			"specimens inside a single OBR. Note that any embedded PDF(s) will not be copied. ",
			MB_ICONINFORMATION | MB_OK);
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnConfigureCustomOrderSegments()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Added a dialog to configure custom segments for Lab Orders
		CHL7CustomSegmentsDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_MessageType = hmtLabOrder;
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

//TES 9/19/2011 - PLID 45542 - Disable the OBR-4 Text box if we're using LOINC for that field
void CHL7LabSettingsDlg::ReflectObr4()
{
	GetDlgItem(IDC_OBR_4_TEXT)->EnableWindow(IsDlgButtonChecked(IDC_OBR_4_USE_TEXT));
}

void CHL7LabSettingsDlg::OnObr4Loinc()
{
	try {
		ReflectObr4();
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnObr4UseText()
{
	try {
		ReflectObr4();
	}NxCatchAll(__FUNCTION__);
}

void CHL7LabSettingsDlg::OnBnClickedHl7LabsConfigInsCoBtn()
{
	try {
		// (r.gonet 09/27/2011) - PLID 45719 - Let them configure ins cos from here since labcorp forces us to do so.
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtInsCo;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/24/2012) - PLID 52013 - Handle the case where they select nothing
void CHL7LabSettingsDlg::OnSelChangingProviderIDTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		//selecting nothing is meaningless, so disable that ability
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/07/2013) - PLID 55527 - Set the appropriate hidden checkboxes.
void CHL7LabSettingsDlg::OnBnClickedMatchFormNumberAndSpecimenRadio()
{
	try {
		m_nxbFormNumber.SetCheck(BST_CHECKED);
		m_nxbSpecimen.SetCheck(BST_CHECKED);
		m_nxbTestCode.SetCheck(BST_UNCHECKED);
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/07/2013) - PLID 55527 - Set the appropriate hidden checkboxes.
void CHL7LabSettingsDlg::OnBnClickedMatchFormNumberAndTestRadio()
{
	try {
		m_nxbFormNumber.SetCheck(BST_CHECKED);
		m_nxbSpecimen.SetCheck(BST_UNCHECKED);
		m_nxbTestCode.SetCheck(BST_CHECKED);
	}NxCatchAll(__FUNCTION__);
}