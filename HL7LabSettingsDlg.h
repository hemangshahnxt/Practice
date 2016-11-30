#pragma once

//TES 5/20/2010 - PLID 38810 - Created, moved some lab-related HL7 settings to this dialog.
// CHL7LabSettingsDlg dialog

class CHL7LabSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7LabSettingsDlg)

	enum EProviderIDTypeComboColumns {
		epidtID = 0,
		epidtName,
	};

public:
	CHL7LabSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CHL7LabSettingsDlg();

	long m_nHL7GroupID;

// Dialog Data
	enum { IDD = IDD_HL7_LAB_SETTINGS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (r.gonet 09/27/2011) - PLID 45719 - Added ins cos button
	CNxIconButton m_nxbOK, m_nxbCancel, m_nxbConfigureResultFlags, m_nxbConfigureQualifiers, m_nxbConfigureResultFields, m_nxbConfigureInsCos;
	CNxEdit m_nxeOBR15Component;
	NxButton m_nxbUseOBR15, m_nxbUseOBX13;
	// (z.manning 2010-05-21 10:08) - PLID 38638 - Added checkboxes for the required matching field when auto importing labs
	// (r.gonet 03/07/2013) - PLID 55527 - Added checkboxes for specimen and test code.
	NxButton m_nxbPatientID, m_nxbPatientName, m_nxbFormNumber, m_nxbSSN, m_nxbBirthDate, m_nxbSpecimen, m_nxbTestCode;
	NxButton m_nxstaticLabImportMatchingFieldGroupBox;
	NxButton m_nxbAlwaysCombine, m_nxbNeverCombine, m_nxbCombineSubGroups, m_nxbCombineBasedOnObr24;
	CNxIconButton m_nxbConfigObr24Values;
	// (c.haag 2010-09-16 12:01) - PLID 40176 - Specimen parsing
	NxButton m_nxbParseSpecimen;
	CNxIconButton m_nxbHelpParseSpecimens;
	// (r.gonet 2011-03-09 10:30) - PLID 42655 - Textbox for option to a single result to all specimens
	NxButton m_nxbCopyResult;
	// (r.gonet 2011-03-09 10:30) - PLID 42655 - Contextual help for option to a single result to all specimens
	CNxIconButton m_nxbHelpCopyResult;
	//TES 4/25/2011 - PLID 43423
	NxButton m_nxbUseObx3_5;
	//TES 4/29/2011 - PLID 43424
	NxButton m_nxbMSH4, m_nxbOBX23, m_nxbOBR21;
	//TES 5/11/2011 - PLID 43634
	NxButton m_nxbAppendObr20;
	CNxIconButton m_nxbEditCustomSegments;
	//TES 9/19/2011 - PLID 45542
	NxButton m_nxbObr4Loinc, m_nxbObr4UseText;
	CNxEdit m_nxeObr4Text;
	NxButton m_nxbBiopsyTypeOBR13; // (z.manning 2011-10-03 11:35) - PLID 45724
    // (j.kuziel 2011-10-14 16:38) - PLID 41419
    NxButton m_nxbSendIN1BeforeGT1;
	// (r.gonet 02/28/2012) - PLID 48044 - Added a button to switch around the order of NTE segments and DG1 segments in the ORDER_DETAIL group.
	NxButton m_nxbSendNTEBeforeDG1;
	// (r.gonet 02/28/2012) - PLID 48042 - Added a button to disable the sending of more than one repetition of PID-3
	NxButton m_nxbOnlySendFirstRepetitionPID_3;
	// (r.gonet 02/28/2012) - PLID 48454 - Added a button to disable the sending of more than one repetition of PID-13
	NxButton m_nxbOnlySendFirstRepetitionPID_13;
	// (r.gonet 02/28/2012) - PLID 48606 - Added a setting not to send spaces around the dash in the order number....
	//NxButton m_nxbDontSendSpacesInOrderNumberSeparator;
	//TES 6/24/2013 - PLID 57288 - Replaced the checkbox with radio buttons, including an option to not send the dash
	NxButton m_nxbOrderNumberFormatSpace, m_nxbOrderNumberFormatNoSpace, m_nxbOrderNumberFormatNone;
	// (r.gonet 06/05/2012) - PLID 50629 - Prompt when a lab result is unmatched to either create a new lab or match an existing one
	NxButton m_nxbUnmatchedPromptRadio;
	// (r.gonet 06/05/2012) - PLID 50629 - Always create a new lab if the order number is unmatched
	NxButton m_nxbUnmatchedAutocreateRadio;
	//TES 7/26/2012 - PLID 51110 - Setting to fill in the 4th component of PID-18 with a LabCorp-specific "Bill Type" code.
	NxButton m_nxbFillPID_18_4;
	//TES 7/26/2012 - PLID 51111 - Setting to output the DG1 segments before the specimen-specific statements.
	NxButton m_nxbOutputDG1BeforeORCSegments;
	// (r.gonet 08/24/2012) - PLID 52013 - Setting to control what type of ID is sent and received for providers.
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderIDTypes;
	//TES 11/27/2012 - PLID 53914 - Setting to send the employer in GT1-16
	NxButton m_nxbFillGT1_16;
	// (b.spivey, January 22, 2013) - PLID 54751 - Controls for inputting patimport flag
	NxButton m_chkPatientHistoryDocumentImportCheck;
	//TES 2/25/2013 - PLID 54876 - Preference to force the PerformingLabID field to be mapped based on the information in OBR-21
	NxButton m_nxbRequirePerformingLab;
	// (r.gonet 03/07/2013) - PLID 55527 - Add visible radio buttons for the different matching modes.
	NxButton m_radioMatchFormNumberAndSpecimen;
	NxButton m_radioMatchFormNumberAndTestCode;
	// (r.gonet 03/07/2013) - PLID 55489 - Added
	NxButton m_checkEnableReflexTesting;
	//TES 5/17/2013 - PLID 56286 - Added
	NxButton m_radioOBR21;
	NxButton m_radioOBX16;	//why is this called obx16 it makes no sense!!!!!
	// (d.singleton 2013-10-25 10:22) - PLID 59181 - need new option to pull performing lab from obx23
	NxButton m_radioOBX23;
	//TES 6/24/2013 - PLID 57293 - Added
	NxButton m_nxbFillOBR18;
	//TES 6/24/2013 - PLID 57294 - Added
	NxButton m_nxbAlwaysSendGT1;
	//TES 6/24/2013 - PLID 57295 - Added
	NxButton m_nxbIncludeAllInsurance;

    virtual BOOL OnInitDialog();
	virtual void OnOK();

	//TES 7/30/2010 - PLID 39908 - Called when one of the Combine Text Segments radio buttons is selected.
	void HandleCombineButton();

	//TES 7/30/2010 - PLID 39908 - This setting isn't stored on screen, so we track it in this member variable.
	CStringArray m_saObr24Values;

	//TES 9/19/2011 - PLID 45542 - Disable the OBR-4 Text box if we're using LOINC for that field
	void ReflectObr4();

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnUseObr15();
public:
	afx_msg void OnConfigureResultFlags();
	afx_msg void OnConfigureQualifiers();
	afx_msg void OnConfigureResultFields();
	afx_msg void OnCombineTextSegments();
	afx_msg void OnNeverCombineTextSegments();
	afx_msg void OnAlwaysCombine();
	afx_msg void OnNeverCombine();
	afx_msg void OnCombineSubGroups();
	afx_msg void OnCombineBasedOnObr24();
	afx_msg void OnConfigObr24Values();
	afx_msg void OnHelpParseSpecimens();
	// (r.gonet 2011-03-09 10:53) - PLID 42655 - 
	afx_msg void OnBnClickedHelpCopyResultButton();
	afx_msg void OnConfigureCustomOrderSegments();
	afx_msg void OnObr4Loinc();
	afx_msg void OnObr4UseText();
	// (r.gonet 09/27/2011) - PLID 45719
	afx_msg void OnBnClickedHl7LabsConfigInsCoBtn();
	// (r.gonet 08/24/2012) - PLID 52013 - Handle the case where they select nothing
	afx_msg void OnSelChangingProviderIDTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (r.gonet 03/07/2013) - PLID 55527 - Added handlers for the radio buttons that will set the hidden matching field checkboxes.
	afx_msg void OnBnClickedMatchFormNumberAndSpecimenRadio();
	afx_msg void OnBnClickedMatchFormNumberAndTestRadio();
};
