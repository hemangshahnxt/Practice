// AdvEbillingSetup.cpp : implementation file
//

#include "stdafx.h"
#include "AdvEbillingSetup.h"
#include "GlobalDrawingUtils.h"
#include "SecondaryANSIClaimConfigDlg.h"
#include "AuditTrail.h"
#include "EligibilitySetupDlg.h"
#include "GlobalFinancialUtils.h"
#include "Adv2010ABPayToOverrideDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CAdvEbillingSetup dialog

// (j.jones 2008-02-06 16:36) - PLID 28843 - enums for the secondary code list combo
enum SecondaryCodeListColumns {
	sclcCode = 0,
	sclcDesc,
};

// (j.jones 2008-05-22 15:58) - PLID 29886 - added enums for the provider and location combos
enum ProviderListColumns {
	pccID = 0,
	pccName,
};

enum LocationListColumns {
	lccID = 0,
	lccName,
};

// (j.jones 2010-03-02 12:52) - PLID 37584 - added enum for Prior. Auth. Qualifier override
enum PriorAuthQualColumns {

	paqcID = 0,
	paqcQualifier = 1,
};

enum PriorAuthQualOptions {

	paqoNone = -1,
	paqoG1 = 1,
	paqo9F = 2,
};

// (j.jones 2012-03-23 15:41) - PLID 49176 - added 2000A taxonomy dropdown
enum TaxonomyComboColumns {
	tccID = 0,
	tccName = 1,
};

enum TaxonomyCodeOptions {
	tcoProvider = 0,
	tcoLocation = 1,
};

// (j.jones 2014-04-23 13:53) - PLID 61840 - added Hide 2310A dropdown
enum Hide2310AColumns {
	h2310AcID = 0,
	hh2310AcDescription = 1,
};

CAdvEbillingSetup::CAdvEbillingSetup(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvEbillingSetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvEbillingSetup)
		m_ProviderID = -1;
		m_LocationID = -1;
		m_bIsUB92 = FALSE;
		m_b2010AAChanged = FALSE;
		m_b2010ABChanged = FALSE;
		m_b2310BChanged = FALSE;		
		m_b2420AChanged = FALSE;

		m_bOldUse2010AA = FALSE;
		m_strOld2010AAQual = "";
		m_strOld2010AA = "";
		m_bOldUse2010AB = FALSE;
		m_strOld2010ABQual = "";
		m_strOld2010AB = "";
		m_bOldUse2310B = FALSE;
		m_strOld2310BQual = "";
		m_strOld2310B = "";
		m_bOldUse2420A = FALSE;
		m_strOld2420AQual = "";
		m_strOld2420A = "";
		m_nOldNM109 = -1;
		m_nOldExtraREF = -1;

		// (j.jones 2008-12-11 14:55) - PLID 32413 - added 2310E fields
		m_b2310EChanged = FALSE;
		m_bOldUse2310E = FALSE;
		m_strOld2310EQual = "";
		m_strOld2310E = "";

		// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
		m_strOld2000ATaxonomyCode = "";
		m_strOld2310BTaxonomyCode = "";
		m_strOld2420ATaxonomyCode = "";

		m_bOldExport2310BRecord = TRUE;

		m_bIs5010Enabled = FALSE;

	//}}AFX_DATA_INIT
}

void CAdvEbillingSetup::DoDataExchange(CDataExchange* pDX)
{
	// (j.jones 2008-05-12 17:09) - PLID 29986 - removed all NSF controls
	// (j.jones 2008-12-11 14:55) - PLID 32413 - added 2310E controls
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvEbillingSetup)	
	DDX_Control(pDX, IDC_2310E_TEXT, m_nxstatic2310ELabel);
	DDX_Control(pDX, IDC_2310E, m_checkUse2310E);
	DDX_Control(pDX, IDC_2310E_QUALIFIER, m_nxedit2310EQualifier);
	DDX_Control(pDX, IDC_2310E_NUMBER, m_nxedit2310ENumber);
	DDX_Control(pDX, IDC_RADIO_SEND_BOX19_2400_NTE, m_radioSendBox19_2400NTE);
	DDX_Control(pDX, IDC_RADIO_SEND_BOX19_2300_NTE, m_radioSendBox19_2300NTE);
	DDX_Control(pDX, IDC_RADIO_SEND_CORRESP_2400_NTE, m_radioSendCorresp2400NTE);
	DDX_Control(pDX, IDC_RADIO_SEND_CORRESP_2300_NTE, m_radioSendCorresp2300NTE);
	DDX_Control(pDX, IDC_BTN_ELIGIBILITY_SETUP, m_btnEligibilitySetup);
	DDX_Control(pDX, IDC_2420A, m_btn2420A);
	DDX_Control(pDX, IDC_2310B, m_btn2310B);
	DDX_Control(pDX, IDC_2010AB, m_btn2010AB);
	DDX_Control(pDX, IDC_2010AA, m_btn2010AA);
	DDX_Control(pDX, IDC_CHECK_HIDE_2330A_REF, m_checkHide2330AREF);
	DDX_Control(pDX, IDC_CHECK_SECONDARY_INS_TYPE_CODE, m_checkUseSecondaryInsCode);
	DDX_Control(pDX, IDC_RADIO_EXTRA_REF_USE_EIN, m_radioExtraRefUseEIN);
	DDX_Control(pDX, IDC_RADIO_EXTRA_REF_USE_NPI, m_radioExtraRefUseNPI);
	DDX_Control(pDX, IDC_RADIO_EXTRA_REF_USE_NONE, m_radioExtraRefUseNone);
	DDX_Control(pDX, IDC_RADIO_NM109_USE_NPI, m_radioNM109UseNPI);
	DDX_Control(pDX, IDC_RADIO_NM109_USE_EIN, m_radioNM109UseEIN);
	DDX_Control(pDX, IDC_RADIO_2000A_PRV_WHEN_NOT_GROUP, m_radio2000APRVWhenNotGroup);
	DDX_Control(pDX, IDC_RADIO_2000A_PRV_ALWAYS, m_radio2000APRVAlways);
	DDX_Control(pDX, IDC_RADIO_2000A_PRV_NEVER, m_radio2000APRVNever);
	DDX_Control(pDX, IDC_CHECK_USE_ANESTHESIA_TIME_AS_QUANTITY, m_checkUseAnesthMinutes);
	DDX_Control(pDX, IDC_CHECK_SEND_BOX_19, m_checkSendBox19);
	DDX_Control(pDX, IDC_CHECK_SEND_REF_PHY_IN_2300, m_checkSendRefPhyIn2300);
	DDX_Control(pDX, IDC_USE_2310B, m_checkUse2310B);
	DDX_Control(pDX, IDC_CHECK_EXPORT_2310B_PRV_SEGMENT, m_checkUse2310BPRVSegment);
	DDX_Control(pDX, IDC_2010AA_QUALIFIER, m_nxedit2010AaQualifier);
	DDX_Control(pDX, IDC_2010AA_NUMBER, m_nxedit2010AaNumber);
	DDX_Control(pDX, IDC_2010AB_QUALIFIER, m_nxedit2010AbQualifier);
	DDX_Control(pDX, IDC_2010AB_NUMBER, m_nxedit2010AbNumber);
	DDX_Control(pDX, IDC_2310B_QUALIFIER, m_nxedit2310BQualifier);
	DDX_Control(pDX, IDC_2310B_NUMBER, m_nxedit2310BNumber);
	DDX_Control(pDX, IDC_2420A_QUALIFIER, m_nxedit2420AQualifier);
	DDX_Control(pDX, IDC_2420A_NUMBER, m_nxedit2420ANumber);
	DDX_Control(pDX, IDC_EDIT_CORRESPONDENCE_DATE, m_nxeditEditCorrespondenceDate);
	DDX_Control(pDX, IDC_GROUP_NAME, m_nxstaticGroupName);
	DDX_Control(pDX, IDC_2010AA_TEXT, m_nxstatic2010AaText);
	DDX_Control(pDX, IDC_2010AB_TEXT, m_nxstatic2010AbText);
	DDX_Control(pDX, IDC_2310B_TEXT, m_nxstatic2310BText);
	DDX_Control(pDX, IDC_2420A_TEXT, m_nxstatic2420AText);
	DDX_Control(pDX, IDC_ANSI_NM109_LABEL, m_nxstaticAnsiNm109Label);
	DDX_Control(pDX, IDC_ANSI_EXTRA_REF_LABEL, m_nxstaticAnsiExtraRefLabel);
	DDX_Control(pDX, IDC_RADIO_2000A_PRV_LABEL, m_nxstaticRadio2000APrvLabel);
	DDX_Control(pDX, IDC_CORRESPONDENCE_LABEL, m_nxstaticCorrespondenceLabel);
	DDX_Control(pDX, IDC_CORRESPONDENCE_LABEL2, m_nxstaticCorrespondenceLabel2);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_UB_ALWAYS_SEND_STATEMENT_DATE_RANGE, m_checkUBAlwaysSendStatementDateRange);
	DDX_Control(pDX, IDC_RADIO_SEND_TPL_IN_2330B, m_radioSendTPLIn2330B);
	DDX_Control(pDX, IDC_RADIO_SEND_TPL_IN_2430, m_radioSendTPLIn2430);
	DDX_Control(pDX, IDC_RADIO_SEND_TPL_IN_BOTH, m_radioSendTPLInBoth);
	DDX_Control(pDX, IDC_RADIO_DO_NOT_SEND_TPL, m_radioDoNotSendTPL);
	DDX_Control(pDX, IDC_BTN_EDIT_2010AB_ADDRESS, m_btnEdit2010AB_Address);
	DDX_Control(pDX, IDC_BTN_EDIT_2010AA_ADDRESS, m_btnEdit2010AA_Address);
	DDX_Control(pDX, IDC_CHECK_HIDE_2000B_SBR03, m_checkHide2000BSBR03);
	DDX_Control(pDX, IDC_RADIO_SEND_2000B_SBR04_ALWAYS, m_radioSend2000BSBR04Always);
	DDX_Control(pDX, IDC_RADIO_SEND_2000B_SBR04_WHEN_GROUP_BLANK, m_radioSend2000BSBR04WhenGroupBlank);
	DDX_Control(pDX, IDC_RADIO_SEND_2000B_SBR04_NEVER, m_radioSend2000BSBR04Never);
	DDX_Control(pDX, IDC_RADIO_SEND_2320_SBR04_ALWAYS, m_radioSend2320SBR04Always);
	DDX_Control(pDX, IDC_RADIO_SEND_2320_SBR04_WHEN_GROUP_BLANK, m_radioSend2320SBR04WhenGroupBlank);
	DDX_Control(pDX, IDC_RADIO_SEND_2320_SBR04_NEVER, m_radioSend2320SBR04Never);
	DDX_Control(pDX, IDC_CHECK_HIDE_REF_PHY_FIELDS, m_checkHideRefPhyFields);	
	DDX_Control(pDX, IDC_CHECK_ORIG_REF_NO_2300_HIDE_WHEN_ORIGINAL_CLAIM, m_checkOrigRefNo_2300);
	DDX_Control(pDX, IDC_CHECK_ORIG_REF_NO_2330B_SEND_WHEN_SECONDARY, m_checkOrigRefNo_2330B);
	DDX_Control(pDX, IDC_2310B_TAXONOMY_LABEL, m_nxstatic2310BTaxonomyLabel);
	DDX_Control(pDX, IDC_2420A_TAXONOMY_LABEL, m_nxstatic2420ATaxonomyLabel);
	DDX_Control(pDX, IDC_2000A_TAXONOMY, m_nxedit2000ATaxonomy);
	DDX_Control(pDX, IDC_2310B_TAXONOMY, m_nxedit2310BTaxonomy);
	DDX_Control(pDX, IDC_2420A_TAXONOMY, m_nxedit2420ATaxonomy);
	DDX_Control(pDX, IDC_ORDERING_PROVIDER_2420E_LABEL, m_nxstaticOrderingProviderLabel);
	DDX_Control(pDX, IDC_RADIO_SEND_2420E_SERVICE_CODE, m_radioOrderingProviderServiceCode);
	DDX_Control(pDX, IDC_RADIO_SEND_2420E_ALWAYS, m_radioOrderingProviderAlways);
	DDX_Control(pDX, IDC_RADIO_SEND_2420E_NEVER, m_radioOrderingProviderNever);
	DDX_Control(pDX, IDC_LABEL_HIDE_2310A, m_nxstaticHide2310ALabel);
	DDX_Control(pDX, IDC_SENDN3N4PERSEGMENTS, m_checkSendN3N4PERSegment);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvEbillingSetup, CNxDialog)
	//{{AFX_MSG_MAP(CAdvEbillingSetup)
	ON_BN_CLICKED(IDC_BTN_SECONDARY_CLAIM_SETUP, OnBtnSecondaryClaimSetup)
	ON_BN_CLICKED(IDC_CHECK_SECONDARY_INS_TYPE_CODE, OnCheckSecondaryInsTypeCode)
	ON_BN_CLICKED(IDC_BTN_ELIGIBILITY_SETUP, OnBtnEligibilitySetup)	
	ON_BN_CLICKED(IDC_CHECK_SEND_BOX_19, OnBnClickedCheckSendBox19)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_EDIT_2010AB_ADDRESS, OnBtnEdit2010ABAddress)
	ON_BN_CLICKED(IDC_BTN_EDIT_2010AA_ADDRESS, OnBtnEdit2010AAAddress)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvEbillingSetup message handlers

void CAdvEbillingSetup::OnOK() 
{
	if(!Save())
		return;
	
	CDialog::OnOK();
}

BOOL CAdvEbillingSetup::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_ProvList = BindNxDataListCtrl(this,IDC_ADV_EBILL_PROVIDERS,GetRemoteData(),TRUE);
		// (j.jones 2007-01-29 14:19) - PLID 24411 - added location dropdown
		m_LocList = BindNxDataListCtrl(this,IDC_ADV_EBILL_LOCATIONS,GetRemoteData(),TRUE);

		// (j.jones 2008-06-23 12:32) - PLID 30434 - hide the eligibility button if no license
		if(!g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)) {
			GetDlgItem(IDC_BTN_ELIGIBILITY_SETUP)->ShowWindow(SW_HIDE);
		}

		// (j.jones 2010-10-19 10:02) - PLID 40931 - cache the value for 5010
		m_bIs5010Enabled = Is5010Enabled();

		// (j.jones 2010-10-19 10:10) - PLID 40931 - in 5010 these settings are obsolete, 
		// one way or another
		if(m_bIs5010Enabled) {

			//NM109 is always NPI, in all cases, disable it
			m_radioNM109UseNPI.EnableWindow(FALSE);
			m_radioNM109UseEIN.EnableWindow(FALSE);

			//the additional REF is only used in 2010AA
			if(m_bIsUB92) {
				// (j.jones 2012-01-06 09:33) - PLID 47336 - for UB claims, there is only one REF in 2010AA,
				// so don't use the word "additional" (for UB 5010 claims only, the 2010AA override in this
				// dialog would replace the selection chosen here)
				GetDlgItem(IDC_ANSI_EXTRA_REF_LABEL)->SetWindowText("Add 2010AA REF segment:");
			}
			else {
				GetDlgItem(IDC_ANSI_EXTRA_REF_LABEL)->SetWindowText("Add additional 2010AA REF segment:");
			}

			//2010AB has no IDs in 5010, just hide it completely
			GetDlgItem(IDC_2010AB)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2010AB_TEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2010AB_QUALIFIER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2010AB_NUMBER)->ShowWindow(SW_HIDE);

			//UB statement date range is always used in 5010, disable it
			m_checkUBAlwaysSendStatementDateRange.EnableWindow(FALSE);

			//supervising provider is 2310D, not 2310E
			GetDlgItem(IDC_2310E_TEXT)->SetWindowText("2310D - Supervising Provider");

			// (j.jones 2011-11-01 09:04) - PLID 46218 - this option applies to 2330A and 2010BA in 5010
			// (j.jones 2012-01-18 14:50) - PLID 47627 - tweaked the wording, as this now applies when THIS company
			// is sent in 2010BA or THIS company is sent in 2330A, unlike many other HCFA Group settings which apply
			// only when sending directly to that company
			GetDlgItem(IDC_CHECK_HIDE_2330A_REF)->SetWindowText("Do not send the Subscriber SSN in 2010BA REF (when Primary), or 2330A REF (when Secondary)");
		}
		else {
			//4010

			// (j.jones 2010-11-01 09:32) - PLID 40919 - hide the 2010AB button
			GetDlgItem(IDC_BTN_EDIT_2010AB_ADDRESS)->ShowWindow(SW_HIDE);
			// (j.jones 2011-11-16 16:18) - PLID 46489 - and 2010AA
			GetDlgItem(IDC_BTN_EDIT_2010AA_ADDRESS)->ShowWindow(SW_HIDE);

			// (j.jones 2013-09-05 10:36) - PLID 58252 - disable, rather than hide, the taxonomy code fields,
			// as no new development is being done in 4010, and thus these aren't supported
			m_nxedit2000ATaxonomy.EnableWindow(FALSE);
			m_nxedit2310BTaxonomy.EnableWindow(FALSE);
			m_nxedit2420ATaxonomy.EnableWindow(FALSE);

			// (j.jones 2014-04-23 13:47) - PLID 61840 - the Hide 2310A settings don't apply to 4010 claims
			m_nxstaticHide2310ALabel.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_HIDE_2310A_COMBO)->ShowWindow(SW_HIDE);
		}
		
		// (j.jones 2008-02-06 16:16) - PLID 28843 - added secondary ins. code setting
		m_SecondaryCodeList = BindNxDataList2Ctrl(this,IDC_SECONDARY_INS_CODE_DROPDOWN,GetRemoteData(),false);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "");
		pRow->PutValue(sclcDesc, "<No Selection>");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "AP");
		pRow->PutValue(sclcDesc, "Auto Insurance Policy");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "C1");
		pRow->PutValue(sclcDesc, "Commercial");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "CP");
		pRow->PutValue(sclcDesc, "Medicare Conditionally Primary");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "GP");
		pRow->PutValue(sclcDesc, "Group Policy");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "HM");
		pRow->PutValue(sclcDesc, "Health Maintenance Organization (HMO)");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "IP");
		pRow->PutValue(sclcDesc, "Individual Policy");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "LD");
		pRow->PutValue(sclcDesc, "Long Term Policy");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "LT");
		pRow->PutValue(sclcDesc, "Litigation");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "MB");
		pRow->PutValue(sclcDesc, "Medicare Part B");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "MC");
		pRow->PutValue(sclcDesc, "Medicaid");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "MI");
		pRow->PutValue(sclcDesc, "Medigap Part B");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "MP");
		pRow->PutValue(sclcDesc, "Medicare Primary");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "OT");
		pRow->PutValue(sclcDesc, "Other");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "PP");
		pRow->PutValue(sclcDesc, "Personal Payment (Cash - No Insurance)");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);
		pRow = m_SecondaryCodeList->GetNewRow();
		pRow->PutValue(sclcCode, "SP");
		pRow->PutValue(sclcDesc, "Supplemental Policy");
		m_SecondaryCodeList->AddRowAtEnd(pRow, NULL);

		// (j.jones 2009-11-24 15:49) - PLID 36411 - added Prior Auth. Qualifier list
		// (j.jones 2010-03-02 12:54) - PLID 37584 - added ability for "no override"
		m_PriorAuthQualifierCombo = BindNxDataList2Ctrl(IDC_PRIOR_AUTH_QUAL_COMBO, false);
		pRow = m_PriorAuthQualifierCombo->GetNewRow();
		pRow->PutValue(paqcID, (long)paqoNone);
		pRow->PutValue(paqcQualifier, " <No Override>");
		m_PriorAuthQualifierCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_PriorAuthQualifierCombo->GetNewRow();
		pRow->PutValue(paqcID, (long)paqoG1);
		pRow->PutValue(paqcQualifier, "G1");
		m_PriorAuthQualifierCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_PriorAuthQualifierCombo->GetNewRow();
		pRow->PutValue(paqcID, (long)paqo9F);
		pRow->PutValue(paqcQualifier, "9F");
		m_PriorAuthQualifierCombo->AddRowAtEnd(pRow, NULL);

		// (j.jones 2012-03-23 15:41) - PLID 49176 - added 2000A taxonomy dropdown
		m_2000ATaxonomyCombo = BindNxDataList2Ctrl(IDC_2000A_TAXONOMY_COMBO, false);
		pRow = m_2000ATaxonomyCombo->GetNewRow();
		pRow->PutValue(tccID, (long)tcoProvider);
		pRow->PutValue(tccName, "Provider Taxonomy Code");
		m_2000ATaxonomyCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_2000ATaxonomyCombo->GetNewRow();
		pRow->PutValue(tccID, (long)tcoLocation);
		pRow->PutValue(tccName, "Location Taxonomy Code");
		m_2000ATaxonomyCombo->AddRowAtEnd(pRow, NULL);

		// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A settings
		m_Hide2310ACombo = BindNxDataList2Ctrl(IDC_HIDE_2310A_COMBO, false);
		pRow = m_Hide2310ACombo->GetNewRow();
		pRow->PutValue(h2310AcID, (long)hide2310A_Never);
		pRow->PutValue(hh2310AcDescription, "Never Hide 2310A");
		m_Hide2310ACombo->AddRowAtEnd(pRow, NULL);
		pRow = m_Hide2310ACombo->GetNewRow();
		pRow->PutValue(h2310AcID, (long)hide2310A_When2420E);
		pRow->PutValue(hh2310AcDescription, "When 2420E Is Sent");
		m_Hide2310ACombo->AddRowAtEnd(pRow, NULL);
		pRow = m_Hide2310ACombo->GetNewRow();
		pRow->PutValue(h2310AcID, (long)hide2310A_When2420F);
		pRow->PutValue(hh2310AcDescription, "When 2420F Is Sent");
		m_Hide2310ACombo->AddRowAtEnd(pRow, NULL);
		pRow = m_Hide2310ACombo->GetNewRow();
		pRow->PutValue(h2310AcID, (long)hide2310A_When2420EorF);
		pRow->PutValue(hh2310AcDescription, "When 2420E or 2420F Are Sent");
		m_Hide2310ACombo->AddRowAtEnd(pRow, NULL);

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

		_RecordsetPtr rs;
		
		if(m_bIsUB92) {
			// (j.jones 2008-04-01 17:41) - PLID 29486 - added ANSI_Hide2330AREF
			// (j.jones 2009-11-24 15:51) - PLID 36411 - added PriorAuthQualifier
			// (j.jones 2010-04-19 12:13) - PLID 38265 - added ANSI_StatementDateRange
			// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
			// (j.jones 2012-03-26 14:44) - PLID 49175 - added PRV2000ACode and Use2000APRVSegment
			// (j.jones 2012-05-14 11:16) - PLID 50338 - added ANSI_SBR04
			// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
			// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
			// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
			// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
			rs = CreateParamRecordset("SELECT Name, NM109_IDType, ExtraREF_IDType, "
				"ANSI_Hide2330AREF, ANSI_SendRefPhyIn2300, PriorAuthQualifier, "
				"ANSI_StatementDateRange, ANSI_SendTPLNumber, PRV2000ACode, Use2000APRVSegment, "
				"ANSI_2000B_SBR03, ANSI_2000B_SBR04, ANSI_2320_SBR04, ANSI_HideRefPhyFields, "
				"OrigRefNo_2300, OrigRefNo_2330B "
				"FROM UB92SetupT WHERE ID = {INT}",m_GroupID);
			if(!rs->eof) {
				m_strGroupName = AdoFldString(rs, "Name","");
				SetDlgItemText(IDC_GROUP_NAME, m_strGroupName);

				// (j.jones 2006-11-13 15:01) - PLID 23446 - added settings for the ID
				// used in NM109 of all UB92 provider loops, and whether to use an extra REF in those loops

				// (j.jones 2008-05-22 14:49) - PLID 29886 - cache the NM109 and ExtraREF values for auditing

				m_nOldNM109 = AdoFldLong(rs, "NM109_IDType",0);
				if(m_nOldNM109 == 0) {	//0 = NPI
					m_radioNM109UseNPI.SetCheck(TRUE);
				}
				else {	//1 = EIN/SSN
					m_radioNM109UseEIN.SetCheck(TRUE);
				}

				// (j.jones 2008-09-09 13:12) - PLID 27450 - changed the default to 2
				// (j.jones 2010-04-16 08:33) - PLID 38149 - hide the NPI option unless it is already selected
				m_nOldExtraREF = AdoFldLong(rs, "ExtraREF_IDType", 2);
				if(m_nOldExtraREF == 0) {	//0 = EIN
					m_radioExtraRefUseEIN.SetCheck(TRUE);
					GetDlgItem(IDC_RADIO_EXTRA_REF_USE_NPI)->ShowWindow(SW_HIDE);
				}
				else if(m_nOldExtraREF == 1) {	//1 = NPI
					m_radioExtraRefUseNPI.SetCheck(TRUE);
					GetDlgItem(IDC_RADIO_EXTRA_REF_USE_NPI)->ShowWindow(SW_SHOW);
				}
				else {	//2 = None
					m_radioExtraRefUseNone.SetCheck(TRUE);
					GetDlgItem(IDC_RADIO_EXTRA_REF_USE_NPI)->ShowWindow(SW_HIDE);
				}

				m_checkHide2330AREF.SetCheck(AdoFldLong(rs, "ANSI_Hide2330AREF",0) == 1);

				// (j.jones 2008-05-23 09:09) - PLID 29939 - supported ANSI_SendRefPhyIn2300 for UBs
				m_checkSendRefPhyIn2300.SetCheck(AdoFldLong(rs, "ANSI_SendRefPhyIn2300",0));

				// (j.jones 2009-11-24 15:49) - PLID 36411 - added Prior Auth. Qualifier list
				// (j.jones 2010-03-02 12:54) - PLID 37584 - added ability for "no override"
				CString strPriorAuthQual = AdoFldString(rs, "PriorAuthQualifier", "");
				PriorAuthQualOptions epaqoID = paqoNone;
				if(strPriorAuthQual == "G1") {
					epaqoID = paqoG1;
				}
				else if(strPriorAuthQual == "9F") {
					epaqoID = paqo9F;
				}
				else if(strPriorAuthQual != "") {
					//we do not allow free text at this time,
					//don't throw an exception though
					ASSERT(FALSE);
				}
				m_PriorAuthQualifierCombo->SetSelByColumn(paqcID, (long)epaqoID);

				// (j.jones 2010-04-19 12:13) - PLID 38265 - added ability to send the UB Statement Date as a range
				m_checkUBAlwaysSendStatementDateRange.SetCheck(AdoFldLong(rs, "ANSI_StatementDateRange", 1) == 1);

				// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
				long nSendTPLNumber = AdoFldLong(rs, "ANSI_SendTPLNumber", 1);
				//1 - do not send
				//2 - send both
				//3 - send 2330B only
				//4 - send 2430 only
				if(nSendTPLNumber == 2) {
					m_radioSendTPLInBoth.SetCheck(TRUE);
				}
				else if(nSendTPLNumber == 3) {
					m_radioSendTPLIn2330B.SetCheck(TRUE);
				}
				else if(nSendTPLNumber == 4) {
					m_radioSendTPLIn2430.SetCheck(TRUE);
				}
				else {
					m_radioDoNotSendTPL.SetCheck(TRUE);
				}

				// (j.jones 2012-03-26 14:44) - PLID 49175 - added PRV2000ACode
				TaxonomyCodeOptions tco2000A = (TaxonomyCodeOptions)AdoFldLong(rs, "PRV2000ACode", (long)tcoProvider);
				m_2000ATaxonomyCombo->SetSelByColumn(tccID, (long)tco2000A);

				// (j.jones 2012-03-26 14:44) - PLID 49175 - added Use2000APRVSegment
				long n2000APRV = AdoFldLong(rs, "Use2000APRVSegment",1);

				if(n2000APRV == 1)
					m_radio2000APRVWhenNotGroup.SetCheck(TRUE);
				else if(n2000APRV == 2)
					m_radio2000APRVAlways.SetCheck(TRUE);
				else
					m_radio2000APRVNever.SetCheck(TRUE);

				// (j.jones 2012-05-14 11:16) - PLID 50338 - added ANSI_SBR04
				// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
				// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
				long nANSI_2000B_SBR03 = AdoFldLong(rs, "ANSI_2000B_SBR03", 0);
				long nANSI_2000B_SBR04 = AdoFldLong(rs, "ANSI_2000B_SBR04", 1);
				long nANSI_2320_SBR04 = AdoFldLong(rs, "ANSI_2320_SBR04", 1);

				m_checkHide2000BSBR03.SetCheck(nANSI_2000B_SBR03 == 1);

				//0 - send only when no group number exists, 1 - send always, 2 - never send
				if(nANSI_2000B_SBR04 == 0) {
					m_radioSend2000BSBR04WhenGroupBlank.SetCheck(TRUE);
				}
				else if(nANSI_2000B_SBR04 == 2) {
					m_radioSend2000BSBR04Never.SetCheck(TRUE);
				}
				else {
					//1 is the default
					m_radioSend2000BSBR04Always.SetCheck(TRUE);
				}
				if(nANSI_2320_SBR04 == 0) {
					m_radioSend2320SBR04WhenGroupBlank.SetCheck(TRUE);
				}
				else if(nANSI_2320_SBR04 == 2) {
					m_radioSend2320SBR04Never.SetCheck(TRUE);
				}
				else {
					//1 is the default
					m_radioSend2320SBR04Always.SetCheck(TRUE);
				}

				// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
				m_checkHideRefPhyFields.SetCheck(AdoFldLong(rs, "ANSI_HideRefPhyFields",1) == 1);

				// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
				m_checkOrigRefNo_2300.SetCheck(AdoFldLong(rs, "OrigRefNo_2300", 1) == 1);
				m_checkOrigRefNo_2330B.SetCheck(AdoFldLong(rs, "OrigRefNo_2330B", 1) == 1);
			}
			rs->Close();
		}
		else {

			// (j.jones 2006-11-28 15:59) - PLID 23651 - removed Use_2310A and ANSIRefPhyQual
			// (j.jones 2008-02-06 16:42) - PLID 28843 - added SecondaryInsCodeUsage and SecondaryInsCode
			// (j.jones 2008-04-01 17:43) - PLID 29486 - added ANSI_Hide2330AREF
			// (j.jones 2008-10-06 10:29) - PLID 31580 - added ANSI_SendCorrespSegment
			// (j.jones 2008-10-06 13:04) - PLID 31578 - added ANSI_SendBox19Segment
			// (j.jones 2009-08-03 14:26) - PLID 33827 - moved Use2310B from HCFASetupT to EbillingSetupT
			// (j.jones 2009-11-24 15:51) - PLID 36411 - added PriorAuthQualifier
			// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
			// (j.jones 2012-03-23 15:45) - PLID 49176 - added PRV2000ACode
			// (j.jones 2012-05-14 11:16) - PLID 50338 - added ANSI_SBR04
			// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
			// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
			// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
			// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
			// (j.jones 2014-01-22 09:49) - PLID 60034 - added ordering provider settings
			// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
			// (b.spivey January 26, 2015) - PLID 64452 - added SendN3N4PERsegment
			rs = CreateParamRecordset("SELECT Name, Box17a, "
				"ANSI_SendBox19, ANSI_SendRefPhyIn2300, ANSI_UseAnesthMinutesAsQty, "
				"ANSI_Correspondence_Note, Use2000APRVSegment, Use2310BPRVSegment, "
				"NM109_IDType, ExtraREF_IDType, SecondaryInsCodeUsage, SecondaryInsCode, "
				"ANSI_Hide2330AREF, ANSI_SendCorrespSegment, ANSI_SendBox19Segment, "
				"PriorAuthQualifier, ANSI_SendTPLNumber, PRV2000ACode, "
				"ANSI_2000B_SBR03, ANSI_2000B_SBR04, ANSI_2320_SBR04, ANSI_HideRefPhyFields, "
				"OrigRefNo_2300, OrigRefNo_2330B, OrderingProvider, Hide2310A, SendN3N4PERSegment "
				"FROM HCFASetupT WHERE ID = {INT}",m_GroupID);

			if(!rs->eof) {
				m_strGroupName = AdoFldString(rs, "Name","");
				SetDlgItemText(IDC_GROUP_NAME, m_strGroupName);

				// (j.jones 2006-11-13 15:01) - PLID 23413 - added settings for the ID
				// used in NM109 of all HCFA provider loops, and whether to use an extra REF in those loops

				// (j.jones 2008-05-22 14:49) - PLID 29886 - cache the NM109 and ExtraREF values for auditing

				m_nOldNM109 = AdoFldLong(rs, "NM109_IDType",0);
				if(m_nOldNM109 == 0) {	//0 = NPI
					m_radioNM109UseNPI.SetCheck(TRUE);
				}
				else {	//1 = EIN/SSN
					m_radioNM109UseEIN.SetCheck(TRUE);
				}

				// (j.jones 2010-04-16 08:33) - PLID 38149 - hide the NPI option unless it is already selected
				m_nOldExtraREF = AdoFldLong(rs, "ExtraREF_IDType", 2);
				if(m_nOldExtraREF == 0) {	//0 = EIN
					m_radioExtraRefUseEIN.SetCheck(TRUE);
					GetDlgItem(IDC_RADIO_EXTRA_REF_USE_NPI)->ShowWindow(SW_HIDE);
				}
				else if(m_nOldExtraREF == 1) {	//1 = NPI
					m_radioExtraRefUseNPI.SetCheck(TRUE);
					GetDlgItem(IDC_RADIO_EXTRA_REF_USE_NPI)->ShowWindow(SW_SHOW);
				}
				else {	//2 = None
					m_radioExtraRefUseNone.SetCheck(TRUE);
					GetDlgItem(IDC_RADIO_EXTRA_REF_USE_NPI)->ShowWindow(SW_HIDE);
				}

				long n2000APRV = AdoFldLong(rs, "Use2000APRVSegment",1);

				if(n2000APRV == 1)
					m_radio2000APRVWhenNotGroup.SetCheck(TRUE);
				else if(n2000APRV == 2)
					m_radio2000APRVAlways.SetCheck(TRUE);
				else
					m_radio2000APRVNever.SetCheck(TRUE);

				m_checkUse2310BPRVSegment.SetCheck(AdoFldBool(rs, "Use2310BPRVSegment",FALSE));

				m_checkSendBox19.SetCheck(AdoFldLong(rs, "ANSI_SendBox19",0));
				m_checkSendRefPhyIn2300.SetCheck(AdoFldLong(rs, "ANSI_SendRefPhyIn2300",0));
				m_checkUseAnesthMinutes.SetCheck(AdoFldLong(rs, "ANSI_UseAnesthMinutesAsQty",0));
				SetDlgItemText(IDC_EDIT_CORRESPONDENCE_DATE, AdoFldString(rs, "ANSI_Correspondence_Note",""));

				// (j.jones 2008-02-06 16:52) - PLID 28843 - load the secondary ins. code settings
				BOOL bSecondaryInsCodeUsage = AdoFldBool(rs, "SecondaryInsCodeUsage", 0);
				m_checkUseSecondaryInsCode.SetCheck(bSecondaryInsCodeUsage);
				m_SecondaryCodeList->Enabled = bSecondaryInsCodeUsage;
				m_SecondaryCodeList->SetSelByColumn(sclcCode, rs->Fields->Item["SecondaryInsCode"]->Value);

				m_checkHide2330AREF.SetCheck(AdoFldLong(rs, "ANSI_Hide2330AREF",0) == 1);

				// (j.jones 2008-10-06 10:29) - PLID 31580 - added ANSI_SendCorrespSegment
				long nANSI_SendCorrespSegment = AdoFldLong(rs, "ANSI_SendCorrespSegment", 1);
				//0 - 2300 NTE, 1 - 2400 NTE
				if(nANSI_SendCorrespSegment == 0) {
					m_radioSendCorresp2300NTE.SetCheck(TRUE);
				}
				else {
					m_radioSendCorresp2400NTE.SetCheck(TRUE);
				}

				// (j.jones 2008-10-06 13:04) - PLID 31578 - added ANSI_SendBox19Segment
				long nANSI_SendBox19Segment = AdoFldLong(rs, "ANSI_SendBox19Segment", 1);
				//0 - 2300 NTE, 1 - 2400 NTE
				if(nANSI_SendBox19Segment == 0) {
					m_radioSendBox19_2300NTE.SetCheck(TRUE);
				}
				else {
					m_radioSendBox19_2400NTE.SetCheck(TRUE);
				}
				OnBnClickedCheckSendBox19();

				// (j.jones 2009-11-24 15:49) - PLID 36411 - added Prior Auth. Qualifier list
				// (j.jones 2010-03-02 12:54) - PLID 37584 - added ability for "no override"
				CString strPriorAuthQual = AdoFldString(rs, "PriorAuthQualifier", "");
				PriorAuthQualOptions epaqoID = paqoNone;
				if(strPriorAuthQual == "G1") {
					epaqoID = paqoG1;
				}
				else if(strPriorAuthQual == "9F") {
					epaqoID = paqo9F;
				}
				else if(strPriorAuthQual != "") {
					//we do not allow free text at this time,
					//don't throw an exception though
					ASSERT(FALSE);
				}
				m_PriorAuthQualifierCombo->SetSelByColumn(paqcID, (long)epaqoID);

				// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
				long nSendTPLNumber = AdoFldLong(rs, "ANSI_SendTPLNumber", 1);
				//1 - do not send
				//2 - send both
				//3 - send 2330B only
				//4 - send 2430 only
				if(nSendTPLNumber == 2) {
					m_radioSendTPLInBoth.SetCheck(TRUE);
				}
				else if(nSendTPLNumber == 3) {
					m_radioSendTPLIn2330B.SetCheck(TRUE);
				}
				else if(nSendTPLNumber == 4) {
					m_radioSendTPLIn2430.SetCheck(TRUE);
				}
				else {
					m_radioDoNotSendTPL.SetCheck(TRUE);
				}

				// (j.jones 2012-03-23 15:41) - PLID 49176 - added 2000A taxonomy dropdown
				TaxonomyCodeOptions tco2000A = (TaxonomyCodeOptions)AdoFldLong(rs, "PRV2000ACode", (long)tcoProvider);
				m_2000ATaxonomyCombo->SetSelByColumn(tccID, (long)tco2000A);

				// (j.jones 2012-05-14 11:16) - PLID 50338 - added ANSI_SBR04
				// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
				// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
				long nANSI_2000B_SBR03 = AdoFldLong(rs, "ANSI_2000B_SBR03", 0);
				long nANSI_2000B_SBR04 = AdoFldLong(rs, "ANSI_2000B_SBR04", 1);
				long nANSI_2320_SBR04 = AdoFldLong(rs, "ANSI_2320_SBR04", 1);

				m_checkHide2000BSBR03.SetCheck(nANSI_2000B_SBR03 == 1);

				//0 - send only when no group number exists, 1 - send always, 2 - never send
				if(nANSI_2000B_SBR04 == 0) {
					m_radioSend2000BSBR04WhenGroupBlank.SetCheck(TRUE);
				}
				else if(nANSI_2000B_SBR04 == 2) {
					m_radioSend2000BSBR04Never.SetCheck(TRUE);
				}
				else {
					//1 is the default
					m_radioSend2000BSBR04Always.SetCheck(TRUE);
				}
				if(nANSI_2320_SBR04 == 0) {
					m_radioSend2320SBR04WhenGroupBlank.SetCheck(TRUE);
				}
				else if(nANSI_2320_SBR04 == 2) {
					m_radioSend2320SBR04Never.SetCheck(TRUE);
				}
				else {
					//1 is the default
					m_radioSend2320SBR04Always.SetCheck(TRUE);
				}

				// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
				m_checkHideRefPhyFields.SetCheck(AdoFldLong(rs, "ANSI_HideRefPhyFields",1) == 1);

				// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
				m_checkOrigRefNo_2300.SetCheck(AdoFldLong(rs, "OrigRefNo_2300", 1) == 1);
				m_checkOrigRefNo_2330B.SetCheck(AdoFldLong(rs, "OrigRefNo_2330B", 1) == 1);

				// (j.jones 2014-01-22 09:49) - PLID 60034 - added ordering provider settings
				long nOrderingProvider = AdoFldLong(rs, "OrderingProvider");
				m_radioOrderingProviderServiceCode.SetCheck(nOrderingProvider == 0);
				m_radioOrderingProviderAlways.SetCheck(nOrderingProvider == 1);
				m_radioOrderingProviderNever.SetCheck(nOrderingProvider == 2);

				// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
				ANSI_Hide2310AOptions eHide2310A = (ANSI_Hide2310AOptions)AdoFldLong(rs, "Hide2310A", 0);
				m_Hide2310ACombo->SetSelByColumn(h2310AcID, (long)eHide2310A);

				// (b.spivey January 26, 2015) - PLID 64452 - added SendN3N4PERsegment
				m_checkSendN3N4PERSegment.SetCheck(AdoFldBool(rs, "SendN3N4PERSegment", FALSE));

			}
			rs->Close();
		}

		// (j.jones 2008-05-12 17:09) - PLID 29986 - removed all NSF controls

		if(m_bIsUB92) {

			// (j.jones 2013-04-11 09:49) - PLID 56166 - Change the hide ref. phy. checkbox
			// to say 2310B, not 2310A. Technically we also hide it in 2310C if they use a
			// ref. phy. there, but we don't need to state that here.
			m_checkHideRefPhyFields.SetWindowText("Do not send Referring Physician middle name or suffix in 2310B");

			// (j.jones 2013-06-20 13:00) - PLID 57245 - the OrigRefNo_2300 checkbox doesn't apply to the
			// Original/Corrected setting for UBs, it is simply an on/off switch
			// (j.jones 2016-05-24 15:43) - NX-100706 - UB claims now have corrected types, so the regular text now applies
			//m_checkOrigRefNo_2300.SetWindowText("Do not send the Original Reference Number in Loop 2300");

			SetDlgItemText(IDC_2310B_TEXT, "2310A - Attending Physician");
			GetDlgItem(IDC_2420A)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2420A_TEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2420A_QUALIFIER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2420A_NUMBER)->ShowWindow(SW_HIDE);

			// (j.jones 2012-03-26 14:53) - PLID 49175 - the 2000A controls are now on both screens
			/*
			GetDlgItem(IDC_RADIO_2000A_PRV_LABEL)->ShowWindow(SW_HIDE);
			// (j.jones 2012-03-23 17:40) - PLID 49176 - this now has two labels
			GetDlgItem(IDC_RADIO_2000A_PRV_LABEL2)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2000A_TAXONOMY_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_2000A_PRV_WHEN_NOT_GROUP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_2000A_PRV_ALWAYS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_2000A_PRV_NEVER)->ShowWindow(SW_HIDE);
			*/
			GetDlgItem(IDC_USE_2310B)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_EXPORT_2310B_PRV_SEGMENT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_SEND_BOX_19)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_USE_ANESTHESIA_TIME_AS_QUANTITY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CORRESPONDENCE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_CORRESPONDENCE_DATE)->ShowWindow(SW_HIDE);
			// (j.jones 2008-02-06 16:40) - PLID 28843 - the secondary ins. code settings are HCFA-only for now
			GetDlgItem(IDC_CHECK_SECONDARY_INS_TYPE_CODE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SECONDARY_INS_CODE_DROPDOWN)->ShowWindow(SW_HIDE);

			// (j.jones 2008-11-12 12:23) - PLID 32010 - hide these controls as well
			GetDlgItem(IDC_RADIO_SEND_BOX19_2400_NTE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_SEND_BOX19_2300_NTE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_SEND_CORRESP_2400_NTE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_SEND_CORRESP_2300_NTE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CORRESPONDENCE_LABEL2)->ShowWindow(SW_HIDE);

			// (j.jones 2008-12-11 14:55) - PLID 32413 - hide 2310E fields
			GetDlgItem(IDC_2310E)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2310E_TEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2310E_QUALIFIER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_2310E_NUMBER)->ShowWindow(SW_HIDE);

			// (j.jones 2013-09-05 10:36) - PLID 58252 - rename the 2310B taxonomy code to 2310A,
			// and hide the 2420A one (there is no 2420A in UB)
			m_nxstatic2310BTaxonomyLabel.SetWindowText("2310A");
			m_nxstatic2420ATaxonomyLabel.ShowWindow(SW_HIDE);
			m_nxedit2420ATaxonomy.ShowWindow(SW_HIDE);

			// (j.jones 2014-01-22 09:49) - PLID 60034 - ordering provider settings are not used on the UB
			m_nxstaticOrderingProviderLabel.ShowWindow(SW_HIDE);
			m_radioOrderingProviderServiceCode.ShowWindow(SW_HIDE);
			m_radioOrderingProviderAlways.ShowWindow(SW_HIDE);
			m_radioOrderingProviderNever.ShowWindow(SW_HIDE);

			// (b.spivey January 26, 2015) - PLID 64452 - not used on UB 
			m_checkSendN3N4PERSegment.ShowWindow(SW_HIDE);

			// (j.jones 2014-04-23 13:47) - PLID 61840 - the Hide 2310A settings don't apply to UB claims
			m_nxstaticHide2310ALabel.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_HIDE_2310A_COMBO)->ShowWindow(SW_HIDE);
		}
		else { //HCFA
			// (j.jones 2010-04-19 12:18) - PLID 38265 - the UB Statement Date option is
			// currently the only setting that is UB-only, and hidden on the HCFA setup
			GetDlgItem(IDC_CHECK_UB_ALWAYS_SEND_STATEMENT_DATE_RANGE)->ShowWindow(SW_HIDE);
		}

		GetDlgItem(IDC_2010AA_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2010AB_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2310B_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2420A_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2010AA_QUALIFIER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2010AB_QUALIFIER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2310B_QUALIFIER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2420A_QUALIFIER)->EnableWindow(FALSE);

		// (j.jones 2008-12-11 14:55) - PLID 32413 - added 2310E fields
		GetDlgItem(IDC_2310E_QUALIFIER)->EnableWindow(FALSE);
		GetDlgItem(IDC_2310E_NUMBER)->EnableWindow(FALSE);

		m_ProvList->CurSel = 0;
		m_LocList->CurSel = 0;

		((CNxEdit*)GetDlgItem(IDC_2010AA_QUALIFIER))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_2010AA_NUMBER))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_2010AB_QUALIFIER))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_2010AB_NUMBER))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_2310B_QUALIFIER))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_2310B_NUMBER))->SetLimitText(50);
		// (j.jones 2008-12-11 14:55) - PLID 32413 - added 2310E fields
		((CNxEdit*)GetDlgItem(IDC_2310E_QUALIFIER))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_2310E_NUMBER))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_2420A_QUALIFIER))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_2420A_NUMBER))->SetLimitText(50);		
		((CNxEdit*)GetDlgItem(IDC_EDIT_CORRESPONDENCE_DATE))->SetLimitText(50);

		// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
		m_nxedit2000ATaxonomy.SetLimitText(20);
		m_nxedit2310BTaxonomy.SetLimitText(20);
		m_nxedit2420ATaxonomy.SetLimitText(20);

		if(m_ProvList->CurSel != -1 && m_LocList->CurSel != -1) {
			m_ProviderID = VarLong(m_ProvList->GetValue(m_ProvList->CurSel, pccID),-1);
			m_LocationID = VarLong(m_LocList->GetValue(m_LocList->CurSel, lccID),-1);

			Load();
		}

	} NxCatchAll("Error loading AdvEbillingSetup");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CAdvEbillingSetup::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD nID;

	switch (HIWORD(wParam))
	{	case BN_CLICKED:
		{			
			nID = LOWORD(wParam);

			// (j.jones 2008-04-29 16:18) - PLID 29619 - this sets m_bHasChanged
			// even if they click a button that didn't actually change anything

			if (!m_bIsLoading
				&& nID != IDOK
				&& nID != IDCANCEL
				&& nID != IDC_BTN_SECONDARY_CLAIM_SETUP
				&& nID != IDC_BTN_EDIT_2010AB_ADDRESS
				&& nID != IDC_BTN_EDIT_2010AA_ADDRESS) {
				m_bHasChanged = TRUE;
			}
			switch (nID)
			{
				case IDC_2010AA:
					GetDlgItem(IDC_2010AA_NUMBER)->EnableWindow(((CButton*)GetDlgItem(IDC_2010AA))->GetCheck());
					GetDlgItem(IDC_2010AA_QUALIFIER)->EnableWindow(((CButton*)GetDlgItem(IDC_2010AA))->GetCheck());
					// (j.jones 2008-04-29 16:23) - PLID 29619 - we now track whether any individual override was modified
					m_b2010AAChanged = TRUE;
					break;
				case IDC_2010AB:
					GetDlgItem(IDC_2010AB_NUMBER)->EnableWindow(((CButton*)GetDlgItem(IDC_2010AB))->GetCheck());
					GetDlgItem(IDC_2010AB_QUALIFIER)->EnableWindow(((CButton*)GetDlgItem(IDC_2010AB))->GetCheck());
					// (j.jones 2008-04-29 16:23) - PLID 29619 - we now track whether any individual override was modified
					m_b2010ABChanged = TRUE;
					break;
				case IDC_2310B:
					GetDlgItem(IDC_2310B_NUMBER)->EnableWindow(((CButton*)GetDlgItem(IDC_2310B))->GetCheck());
					GetDlgItem(IDC_2310B_QUALIFIER)->EnableWindow(((CButton*)GetDlgItem(IDC_2310B))->GetCheck());
					// (j.jones 2008-04-29 16:23) - PLID 29619 - we now track whether any individual override was modified
					m_b2310BChanged = TRUE;
					break;
				case IDC_2420A:
					GetDlgItem(IDC_2420A_NUMBER)->EnableWindow(((CButton*)GetDlgItem(IDC_2420A))->GetCheck());
					GetDlgItem(IDC_2420A_QUALIFIER)->EnableWindow(((CButton*)GetDlgItem(IDC_2420A))->GetCheck());
					// (j.jones 2008-04-29 16:23) - PLID 29619 - we now track whether any individual override was modified
					m_b2420AChanged = TRUE;
					break;
				// (j.jones 2008-12-11 15:04) - PLID 32413 - added 2310E fields
				case IDC_2310E:
					GetDlgItem(IDC_2310E_NUMBER)->EnableWindow(m_checkUse2310E.GetCheck());
					GetDlgItem(IDC_2310E_QUALIFIER)->EnableWindow(m_checkUse2310E.GetCheck());
					m_b2310EChanged = TRUE;
					break;

				default:
					return CDialog::OnCommand(wParam, lParam);
			}
			break;
		}
		case EN_CHANGE: {

			nID = LOWORD(wParam);

			if (!m_bIsLoading) {
				m_bHasChanged = TRUE;

				// (j.jones 2008-04-29 16:23) - PLID 29619 - we now track whether any individual override was modified
				switch(nID) {
					case IDC_2010AA_NUMBER:
					case IDC_2010AA_QUALIFIER:
						m_b2010AAChanged = TRUE;
						break;
					case IDC_2010AB_NUMBER:
					case IDC_2010AB_QUALIFIER:
						m_b2010ABChanged = TRUE;
						break;
					case IDC_2310B_NUMBER:
					case IDC_2310B_QUALIFIER:
						m_b2310BChanged = TRUE;
						break;
					case IDC_2420A_NUMBER:
					case IDC_2420A_QUALIFIER:
						m_b2420AChanged = TRUE;
						break;
					// (j.jones 2008-12-11 15:04) - PLID 32413 - added 2310E fields
					case IDC_2310E_NUMBER:
					case IDC_2310E_QUALIFIER:
						m_b2310EChanged = TRUE;
						break;
				}			
				break;
			}
		}			
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

// (j.jones 2007-02-20 10:42) - PLID 23953 - converted the Save function to return
// a boolean to determine success or failure
BOOL CAdvEbillingSetup::Save()
{
	if(m_ProvList->CurSel == -1)
		return FALSE;

	if(m_LocList->CurSel == -1)
		return FALSE;

	long nAuditTransactionID = -1;

	try {

		//check each field, then save new or update as needed

		BOOL	bUse_2010AA, bUse_2010AB, bUse_2310B, bUse_2420A, bUse_2310E,
				bExport2310B, bUse2310BPRVSegment, 
				bSendBox19, bSendRefPhyIn2300, bUseAnesthMinutes,
				bANSI_Hide2330AREF, bANSI_StatementDateRange,
				bANSI_HideRefPhyFields = TRUE,
				bOrigRefNo_2300 = TRUE, bOrigRefNo_2330B = TRUE;

		long	nUse2000APRVSegment;

		CString ANSI_2010AA, ANSI_2010AB, ANSI_2310B, ANSI_2420A,
				ANSI_2010AA_Qual, ANSI_2010AB_Qual, ANSI_2310B_Qual, ANSI_2420A_Qual,
				ANSI_2310E, ANSI_2310E_Qual;

		//ANSI

		//IDC_2010AA:
		bUse_2010AA = ((CButton*)GetDlgItem(IDC_2010AA))->GetCheck();
		GetDlgItemText(IDC_2010AA_NUMBER,ANSI_2010AA);
		GetDlgItemText(IDC_2010AA_QUALIFIER,ANSI_2010AA_Qual);
		ANSI_2010AA_Qual.TrimLeft();
		ANSI_2010AA_Qual.TrimRight();
		ANSI_2010AA.TrimLeft();
		ANSI_2010AA.TrimRight();

		//IDC_2010AB:
		bUse_2010AB = ((CButton*)GetDlgItem(IDC_2010AB))->GetCheck();
		GetDlgItemText(IDC_2010AB_NUMBER,ANSI_2010AB);
		GetDlgItemText(IDC_2010AB_QUALIFIER,ANSI_2010AB_Qual);
		ANSI_2010AB_Qual.TrimLeft();
		ANSI_2010AB_Qual.TrimRight();
		ANSI_2010AB.TrimLeft();
		ANSI_2010AB.TrimRight();

		//IDC_2310B:
		bUse_2310B = ((CButton*)GetDlgItem(IDC_2310B))->GetCheck();
		GetDlgItemText(IDC_2310B_NUMBER,ANSI_2310B);
		GetDlgItemText(IDC_2310B_QUALIFIER,ANSI_2310B_Qual);
		ANSI_2310B_Qual.TrimLeft();
		ANSI_2310B_Qual.TrimRight();
		ANSI_2310B.TrimLeft();
		ANSI_2310B.TrimRight();

		//IDC_2420A:
		bUse_2420A = ((CButton*)GetDlgItem(IDC_2420A))->GetCheck();
		GetDlgItemText(IDC_2420A_NUMBER,ANSI_2420A);
		GetDlgItemText(IDC_2420A_QUALIFIER,ANSI_2420A_Qual);
		ANSI_2420A_Qual.TrimLeft();
		ANSI_2420A_Qual.TrimRight();
		ANSI_2420A.TrimLeft();
		ANSI_2420A.TrimRight();

		// (j.jones 2008-12-11 15:07) - PLID 32413 - added 2310E fields
		bUse_2310E = m_checkUse2310E.GetCheck();
		GetDlgItemText(IDC_2310E_NUMBER,ANSI_2310E);
		GetDlgItemText(IDC_2310E_QUALIFIER,ANSI_2310E_Qual);
		ANSI_2310E_Qual.TrimLeft();
		ANSI_2310E_Qual.TrimRight();
		ANSI_2310E.TrimLeft();
		ANSI_2310E.TrimRight();

		// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
		CString str2000ATaxonomy;
		m_nxedit2000ATaxonomy.GetWindowText(str2000ATaxonomy);
		str2000ATaxonomy.TrimLeft();
		str2000ATaxonomy.TrimRight();

		CString str2310BTaxonomy;
		m_nxedit2310BTaxonomy.GetWindowText(str2310BTaxonomy);
		str2310BTaxonomy.TrimLeft();
		str2310BTaxonomy.TrimRight();

		CString str2420ATaxonomy;
		m_nxedit2420ATaxonomy.GetWindowText(str2420ATaxonomy);
		str2420ATaxonomy.TrimLeft();
		str2420ATaxonomy.TrimRight();

		nUse2000APRVSegment = 1;

		if(m_radio2000APRVWhenNotGroup.GetCheck())
			nUse2000APRVSegment = 1;
		else if(m_radio2000APRVAlways.GetCheck())
			nUse2000APRVSegment = 2;
		else
			nUse2000APRVSegment = 3;

		bExport2310B = m_checkUse2310B.GetCheck();
		bUse2310BPRVSegment = m_checkUse2310BPRVSegment.GetCheck();

		bSendBox19 = m_checkSendBox19.GetCheck();
		bSendRefPhyIn2300 = m_checkSendRefPhyIn2300.GetCheck();
		bUseAnesthMinutes = m_checkUseAnesthMinutes.GetCheck();

		CString strCorrespondenceNote;
		GetDlgItemText(IDC_EDIT_CORRESPONDENCE_DATE, strCorrespondenceNote);

		long nRecordsAffected = 0;

		long nNM109_IDType = 0;
		if(m_radioNM109UseEIN.GetCheck())
			nNM109_IDType = 1;

		long nExtraREF_IDType = 0;
		if(m_radioExtraRefUseNPI.GetCheck()) {
			nExtraREF_IDType = 1;
		}
		else if(m_radioExtraRefUseNone.GetCheck()) {
			nExtraREF_IDType = 2;
		}

		// (j.jones 2008-04-01 17:44) - PLID 29486 - added ANSI_Hide2330AREF
		bANSI_Hide2330AREF = m_checkHide2330AREF.GetCheck();

		// (j.jones 2010-04-19 12:13) - PLID 38265 - added ability to send the UB Statement Date as a range
		bANSI_StatementDateRange = m_checkUBAlwaysSendStatementDateRange.GetCheck();

		// (j.jones 2008-10-06 10:29) - PLID 31580 - added ANSI_SendCorrespSegment
		//0 - 2300 NTE, 1 - 2400 NTE
		long nANSI_SendCorrespSegment = 1;
		if(m_radioSendCorresp2300NTE.GetCheck()) {
			nANSI_SendCorrespSegment = 0;
		}

		// (j.jones 2008-10-06 13:04) - PLID 31578 - added ANSI_SendBox19Segment
		//0 - 2300 NTE, 1 - 2400 NTE
		long nANSI_SendBox19Segment = 1;		
		if(m_radioSendBox19_2300NTE.GetCheck()) {
			nANSI_SendBox19Segment = 0;
		}

		// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
		bANSI_HideRefPhyFields = m_checkHideRefPhyFields.GetCheck();

		// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
		bOrigRefNo_2300 = m_checkOrigRefNo_2300.GetCheck();
		bOrigRefNo_2330B = m_checkOrigRefNo_2330B.GetCheck();

		// (j.jones 2007-02-20 10:32) - PLID 23953 - disallow saving changes if
		// there is any override enabled without a qualifier or ID.
		// (j.jones 2008-04-29 15:13) - PLID 29619 - now we do allow a blank override,
		// but we need to warn in that case, but only allow both to be blank, not
		// one or the other, so I re-wrote this code to handle the new rules
		// (j.jones 2010-04-15 17:28) - PLID 38149 - warn when the qualifier is XX
		
		CString strInvalidOverrides = "";
		CString strBlankOverrides = "";
		CString strXXOverrideNames = "";

		if(bUse_2010AA) {
			ValidateOneANSIOverride("2010AA - Billing Provider", m_b2010AAChanged, ANSI_2010AA_Qual, ANSI_2010AA, strInvalidOverrides, strBlankOverrides, strXXOverrideNames);
		}
		if(bUse_2010AB) {
			ValidateOneANSIOverride("2010AB - Pay-To Provider", m_b2010ABChanged, ANSI_2010AB_Qual, ANSI_2010AB, strInvalidOverrides, strBlankOverrides, strXXOverrideNames);
		}
		if(bUse_2310B) {
			ValidateOneANSIOverride(m_bIsUB92 ? "2310A - Attending Physician" : "2310B - Rendering Provider", m_b2310BChanged, ANSI_2310B_Qual, ANSI_2310B, strInvalidOverrides, strBlankOverrides, strXXOverrideNames);
		}
		// (j.jones 2008-12-11 15:08) - PLID 32413 - added 2310E fields, HCFA only
		if(!m_bIsUB92 && bUse_2310E) {
			ValidateOneANSIOverride("2310E - Supervising Provider", m_b2310EChanged, ANSI_2310E_Qual, ANSI_2310E, strInvalidOverrides, strBlankOverrides, strXXOverrideNames);
		}
		if(!m_bIsUB92 && bUse_2420A) {
			ValidateOneANSIOverride("2420A - Rendering Provider", m_b2420AChanged, ANSI_2420A_Qual, ANSI_2420A, strInvalidOverrides, strBlankOverrides, strXXOverrideNames);
		}
		
		//now stop the save if any are invalid
		if(!strInvalidOverrides.IsEmpty()) {
			CString strWarn;
			strWarn.Format("The following override values have only an ID number or qualifier filled in, but not both:\n\n"
				"%s\n\n"
				"You cannot have any overrides enabled with just an ID number or a qualifier. "
				"You must have either an ID number and a qualifier filled in, or both left blank, or else the override must be disabled.", strInvalidOverrides);
			AfxMessageBox(strWarn);
			return FALSE;
		}
		//warn if any are blank, and give the option to cancel saving		
		if(!strBlankOverrides.IsEmpty()) {
			CString strWarn;
			strWarn.Format("The following override values have the ID number and qualifier both left blank:\n\n"
				"%s\n\n"
				"This configuration will cause no provider ID to be submitted for the associated provider loop. This is allowed, but not typical.\n\n"
				"Are you sure you do not want to send any ID number for the listed overrides?", strBlankOverrides);
			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//make it clear that nothing was saved
				AfxMessageBox("The save has been cancelled, please correct the blank overrides before continuing.");
				return FALSE;
			}
		}

		// (j.jones 2010-04-15 17:31) - PLID 38149 - warn if any have XX qualifiers, and give the option to cancel saving		
		if(!strXXOverrideNames.IsEmpty()) {
			CString strWarn;
			strWarn.Format("The following override values have an XX qualifier, which is not a valid qualifier to use:\n\n"
				"%s\n\n"
				"This configuration may cause your claims to be rejected.\n\n"
				"Are you sure you want to send XX as a qualifier for the listed overrides?", strXXOverrideNames);
			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//make it clear that nothing was saved
				AfxMessageBox("The save has been cancelled, please correct the XX qualifiers before continuing.");
				return FALSE;
			}
		}

		// (j.jones 2008-02-06 16:56) - PLID 28843 - save the secondary ins. code settings
		BOOL bSecondaryInsCodeUsage = m_checkUseSecondaryInsCode.GetCheck();
		CString strSecondaryCode = "";
		NXDATALIST2Lib::IRowSettingsPtr pSecondaryCodeRow = m_SecondaryCodeList->GetCurSel();
		if(pSecondaryCodeRow) {
			strSecondaryCode = VarString(pSecondaryCodeRow->GetValue(sclcCode), "");
		}
		if(bSecondaryInsCodeUsage && strSecondaryCode.IsEmpty()) {
			AfxMessageBox("You cannot enable the secondary insurance type code without selecting a code from the list.\n"
				"Please select an insurance code from the list, or disable the secondary insurance type code option.");
			return FALSE;
		}

		// (j.jones 2009-11-24 15:51) - PLID 36411 - added PriorAuthQualifier
		CString strPriorAuthQual = "";
		NXDATALIST2Lib::IRowSettingsPtr pPriorAuthRow = m_PriorAuthQualifierCombo->GetCurSel();
		if(pPriorAuthRow) {
			PriorAuthQualOptions epaqoID = (PriorAuthQualOptions)VarLong(pPriorAuthRow->GetValue(paqcID), paqoNone);
			if(epaqoID == paqoG1) {
				strPriorAuthQual = "G1";
			}
			else if(epaqoID == paqo9F) {
				strPriorAuthQual = "9F";
			}
		}

		// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
		long nSendTPLNumber = 1;
		//1 - do not send
		//2 - send both
		//3 - send 2330B only
		//4 - send 2430 only
		if(m_radioSendTPLInBoth.GetCheck()) {
			nSendTPLNumber = 2;
		}
		else if(m_radioSendTPLIn2330B.GetCheck()) {
			nSendTPLNumber = 3;
		}
		else if(m_radioSendTPLIn2430.GetCheck()) {
			nSendTPLNumber = 4;
		}

		// (j.jones 2012-03-23 15:41) - PLID 49176 - added 2000A taxonomy dropdown
		TaxonomyCodeOptions tco2000A = tcoProvider;
		NXDATALIST2Lib::IRowSettingsPtr pTaxonomyRow = m_2000ATaxonomyCombo->GetCurSel();
		if(pTaxonomyRow) {
			tco2000A = (TaxonomyCodeOptions)VarLong(pTaxonomyRow->GetValue(tccID), (long)tcoProvider);
		}

		// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
		// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
		long ANSI_2000B_SBR03 = 0;
		if(m_checkHide2000BSBR03.GetCheck()) {
			ANSI_2000B_SBR03 = 1;
		}
		
		//0 - send only when no group number exists, 1 - send always, 2 - never send
		long ANSI_2000B_SBR04 = 1;
		if(m_radioSend2000BSBR04WhenGroupBlank.GetCheck()) {
			ANSI_2000B_SBR04 = 0;
		}
		else if(m_radioSend2000BSBR04Never.GetCheck()) {
			ANSI_2000B_SBR04 = 2;
		}
		long ANSI_2320_SBR04 = 1;
		if(m_radioSend2320SBR04WhenGroupBlank.GetCheck()) {
			ANSI_2320_SBR04 = 0;
		}
		else if(m_radioSend2320SBR04Never.GetCheck()) {
			ANSI_2320_SBR04 = 2;
		}

		// (j.jones 2014-01-22 09:49) - PLID 60034 - added ordering provider settings
		long nOrderingProvider = 0; //0 is the service code setting (default)
		if(m_radioOrderingProviderAlways.GetCheck()) {
			nOrderingProvider = 1;
		}
		else if(m_radioOrderingProviderNever.GetCheck()) {
			nOrderingProvider = 2;
		}

		// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
		ANSI_Hide2310AOptions eHide2310A = hide2310A_Never;
		NXDATALIST2Lib::IRowSettingsPtr pHide2310ARow = m_Hide2310ACombo->GetCurSel();
		if(pHide2310ARow) {
			eHide2310A = (ANSI_Hide2310AOptions)VarLong(pHide2310ARow->GetValue(h2310AcID), (long)hide2310A_Never);
		}

		if(!m_bIsUB92) {

			// (j.jones 2006-11-13 15:18) - PLID 23413 - added settings for the ID
			// used in NM109 of all HCFA provider loops, and whether to use an extra REF in those loops
			// (j.jones 2006-11-28 15:59) - PLID 23651 - removed Use_2310A and ANSIRefPhyQual
			// (j.jones 2008-02-06 17:01) - PLID 28843 - added SecondaryInsCodeUsage and SecondaryInsCode
			// (j.jones 2008-04-01 17:44) - PLID 29486 - added ANSI_Hide2330AREF
			// (j.jones 2008-10-06 10:29) - PLID 31580 - added ANSI_SendCorrespSegment
			// (j.jones 2008-10-06 13:04) - PLID 31578 - added ANSI_SendBox19Segment
			// (j.jones 2009-08-03 14:26) - PLID 33827 - removed Use2310B
			// (j.jones 2009-11-24 15:51) - PLID 36411 - added PriorAuthQualifier
			// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
			// (j.jones 2012-03-23 15:49) - PLID 49176 - added PRV2000ACode, and parameterized
			// (j.jones 2012-05-14 11:16) - PLID 50338 - added ANSI_SBR04
			// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
			// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
			// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
			// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
			// (j.jones 2014-01-22 09:49) - PLID 60034 - added ordering provider settings
			// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
			// (b.spivey January 26, 2015) - PLID 64452 - added SendN3N4PERsegment
			ExecuteParamSql("UPDATE HCFASetupT SET ANSI_SendBox19 = {INT}, "
				"ANSI_SendRefPhyIn2300 = {INT}, ANSI_UseAnesthMinutesAsQty = {INT}, "
				"ANSI_Correspondence_Note = {STRING}, Use2000APRVSegment = {INT}, Use2310BPRVSegment = {INT}, "
				"NM109_IDType = {INT}, ExtraREF_IDType = {INT}, SecondaryInsCodeUsage = {INT}, SecondaryInsCode = {STRING}, "
				"ANSI_Hide2330AREF = {INT}, ANSI_SendCorrespSegment = {INT}, ANSI_SendBox19Segment = {INT}, "
				"PriorAuthQualifier = {STRING}, ANSI_SendTPLNumber = {INT}, PRV2000ACode = {INT}, "
				"ANSI_2000B_SBR03 = {INT}, ANSI_2000B_SBR04 = {INT}, ANSI_2320_SBR04 = {INT}, ANSI_HideRefPhyFields = {INT}, "
				"OrigRefNo_2300 = {INT}, OrigRefNo_2330B = {INT}, OrderingProvider = {INT}, Hide2310A = {INT}, SendN3N4PERSegment = {INT} "
				"WHERE ID = {INT}", (bSendBox19 ? 1 : 0),
				(bSendRefPhyIn2300 ? 1 : 0), (bUseAnesthMinutes ? 1 : 0), strCorrespondenceNote, 
				nUse2000APRVSegment, (bUse2310BPRVSegment ? 1 : 0),
				nNM109_IDType, nExtraREF_IDType, bSecondaryInsCodeUsage, strSecondaryCode, bANSI_Hide2330AREF ? 1 : 0,
				nANSI_SendCorrespSegment, nANSI_SendBox19Segment, strPriorAuthQual, nSendTPLNumber, (long)tco2000A,
				ANSI_2000B_SBR03, ANSI_2000B_SBR04, ANSI_2320_SBR04, (bANSI_HideRefPhyFields ? 1 : 0),
				(bOrigRefNo_2300 ? 1 : 0), (bOrigRefNo_2330B ? 1 : 0), nOrderingProvider, (long)eHide2310A, m_checkSendN3N4PERSegment.GetCheck(),
				m_GroupID);

			// (j.jones 2008-05-22 14:52) - PLID 29886 - audit the NM109 and ExtraREF fields
			if(m_nOldNM109 != nNM109_IDType) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//this field does not need provider and location names
				CString strOld, strNew;

				//0 = NPI, 1 = EIN/SSN

				if(m_nOldNM109 == 0) {
					strOld = "NPI";
				}
				else {
					strOld = "EIN/SSN";
				}

				if(nNM109_IDType == 0) {
					strNew = "NPI";
				}
				else {
					strNew = "EIN/SSN";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_NM109, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_nOldNM109 = nNM109_IDType;

			if(m_nOldExtraREF != nExtraREF_IDType) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//this field does not need provider and location names
				CString strOld, strNew;

				//0 = EIN/SSN, 1 - NPI, 2 - None

				if(m_nOldExtraREF == 0) {
					strOld = "EIN/SSN";
				}
				else if(m_nOldExtraREF == 1) {
					strOld = "NPI";
				}
				else {
					strOld = "Do Not Add";
				}

				if(nExtraREF_IDType == 0) {
					strNew = "EIN/SSN";
				}
				else if(nExtraREF_IDType == 1) {
					strNew = "NPI";
				}
				else {
					strNew = "Do Not Add";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_ExtraREF, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_nOldExtraREF = nExtraREF_IDType;

			ExecuteParamSql(GetRemoteData(), &nRecordsAffected, 
				// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here. If we
				// do, then nRecordsAffected may never be updated.
				// (j.jones 2008-12-11 15:09) - PLID 32413 - added support for 2310E fields
				// (j.jones 2009-08-03 14:26) - PLID 33827 - added Export2310BRecord
				// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
				"SET NOCOUNT OFF "
				"UPDATE EbillingSetupT SET "
				"Use_2010AA = {INT}, "
				"ANSI_2010AA = {STRING}, "
				"ANSI_2010AA_Qual = {STRING}, "
				"Use_2010AB = {INT}, "
				"ANSI_2010AB = {STRING}, "
				"ANSI_2010AB_Qual = {STRING}, "
				"Use_2310B = {INT}, "
				"ANSI_2310B = {STRING}, "
				"ANSI_2310B_Qual = {STRING}, "
				"Use_2420A = {INT}, "
				"ANSI_2420A = {STRING}, "
				"ANSI_2420A_Qual = {STRING}, "
				"Use_2310E = {INT}, "
				"ANSI_2310E = {STRING}, "
				"ANSI_2310E_Qual = {STRING}, "
				"Export2310BRecord = {INT}, "
				"ANSI_2000A_Taxonomy = {STRING}, "
				"ANSI_2310B_Taxonomy = {STRING}, "
				"ANSI_2420A_Taxonomy = {STRING} "
				""
				"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
				(bUse_2010AA ? 1 : 0), ANSI_2010AA, ANSI_2010AA_Qual,
				(bUse_2010AB ? 1 : 0), ANSI_2010AB, ANSI_2010AB_Qual,
				(bUse_2310B ? 1 : 0), ANSI_2310B, ANSI_2310B_Qual,
				(bUse_2420A ? 1 : 0), ANSI_2420A, ANSI_2420A_Qual,
				(bUse_2310E ? 1 : 0), ANSI_2310E, ANSI_2310E_Qual,
				(bExport2310B ? 1 : 0),
				str2000ATaxonomy, str2310BTaxonomy, str2420ATaxonomy,
				m_GroupID, m_ProviderID, m_LocationID);

			if(nRecordsAffected == 0) {
				ExecuteParamSql(GetRemoteData(), &nRecordsAffected, 
					// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here. If we
					// do, then nRecordsAffected may never be updated.
					// (j.jones 2008-12-11 15:09) - PLID 32413 - added support for 2310E fields
					// (j.jones 2009-08-03 14:26) - PLID 33827 - added Export2310BRecord
					// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
					"SET NOCOUNT OFF "
					"INSERT INTO EbillingSetupT "
					"(SetupGroupID, ProviderID, LocationID, "
					"Use_2010AA, ANSI_2010AA, ANSI_2010AA_Qual, "
					"Use_2010AB, ANSI_2010AB, ANSI_2010AB_Qual, "
					"Use_2310B, ANSI_2310B, ANSI_2310B_Qual, "
					"Use_2420A, ANSI_2420A, ANSI_2420A_Qual, "
					"Use_2310E, ANSI_2310E, ANSI_2310E_Qual, "
					"Export2310BRecord, "
					"ANSI_2000A_Taxonomy, ANSI_2310B_Taxonomy, ANSI_2420A_Taxonomy) "
					"VALUES ({INT}, {INT}, {INT}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, "
					"{STRING}, {STRING}, {STRING}) ",
					m_GroupID, m_ProviderID, m_LocationID, 
					(bUse_2010AA ? 1 : 0), ANSI_2010AA, ANSI_2010AA_Qual,
					(bUse_2010AB ? 1 : 0), ANSI_2010AB, ANSI_2010AB_Qual,
					(bUse_2310B ? 1 : 0), ANSI_2310B, ANSI_2310B_Qual,
					(bUse_2420A ? 1 : 0), ANSI_2420A, ANSI_2420A_Qual,
					(bUse_2310E ? 1 : 0), ANSI_2310E, ANSI_2310E_Qual,
					(bExport2310B ? 1 : 0),
					str2000ATaxonomy, str2310BTaxonomy, str2420ATaxonomy);
			}

			// (j.jones 2008-05-22 15:40) - PLID 29886 - audit the overrides

			if(m_bOldUse2010AA != bUse_2010AA) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2010AA) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2010AA) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_Use2010AA, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2010AA = bUse_2010AA;

			if(m_strOld2010AAQual != ANSI_2010AA_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010AAQual.IsEmpty() ? "<None>" : m_strOld2010AAQual, strInfo);

				strNew = ANSI_2010AA_Qual.IsEmpty() ? "<None>" : ANSI_2010AA_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2010AA_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010AAQual = ANSI_2010AA_Qual;

			if(m_strOld2010AA != ANSI_2010AA) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010AA.IsEmpty() ? "<None>" : m_strOld2010AA, strInfo);

				strNew = ANSI_2010AA.IsEmpty() ? "<None>" : ANSI_2010AA;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2010AA_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010AA = ANSI_2010AA;

			if(m_bOldUse2010AB != bUse_2010AB) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2010AB) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2010AB) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_Use2010AB, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2010AB = bUse_2010AB;

			if(m_strOld2010ABQual != ANSI_2010AB_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010ABQual.IsEmpty() ? "<None>" : m_strOld2010ABQual, strInfo);

				strNew = ANSI_2010AB_Qual.IsEmpty() ? "<None>" : ANSI_2010AB_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2010AB_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010ABQual = ANSI_2010AB_Qual;

			if(m_strOld2010AB != ANSI_2010AB) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010AB.IsEmpty() ? "<None>" : m_strOld2010AB, strInfo);

				strNew = ANSI_2010AB.IsEmpty() ? "<None>" : ANSI_2010AB;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2010AB_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010AB = ANSI_2010AB;

			if(m_bOldUse2310B != bUse_2310B) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2310B) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2310B) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_Use2310B, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2310B = bUse_2310B;

			if(m_strOld2310BQual != ANSI_2310B_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310BQual.IsEmpty() ? "<None>" : m_strOld2310BQual, strInfo);

				strNew = ANSI_2310B_Qual.IsEmpty() ? "<None>" : ANSI_2310B_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2310B_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310BQual = ANSI_2310B_Qual;

			if(m_strOld2310B != ANSI_2310B) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310B.IsEmpty() ? "<None>" : m_strOld2310B, strInfo);

				strNew = ANSI_2310B.IsEmpty() ? "<None>" : ANSI_2310B;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2310B_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310B = ANSI_2310B;

			if(m_bOldUse2420A != bUse_2420A) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2420A) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2420A) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_Use2420A, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2420A = bUse_2420A;

			if(m_strOld2420AQual != ANSI_2420A_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2420AQual.IsEmpty() ? "<None>" : m_strOld2420AQual, strInfo);

				strNew = ANSI_2420A_Qual.IsEmpty() ? "<None>" : ANSI_2420A_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2420A_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2420AQual = ANSI_2420A_Qual;

			if(m_strOld2420A != ANSI_2420A) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2420A.IsEmpty() ? "<None>" : m_strOld2420A, strInfo);

				strNew = ANSI_2420A.IsEmpty() ? "<None>" : ANSI_2420A;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2420A_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2420A = ANSI_2420A;

			// (j.jones 2008-12-11 15:09) - PLID 32413 - added support for 2310E fields
			if(m_bOldUse2310E != bUse_2310E) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2310E) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2310E) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_Use2310E, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2310E = bUse_2310E;

			if(m_strOld2310EQual != ANSI_2310E_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310EQual.IsEmpty() ? "<None>" : m_strOld2310EQual, strInfo);

				strNew = ANSI_2310E_Qual.IsEmpty() ? "<None>" : ANSI_2310E_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2310E_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310EQual = ANSI_2310E_Qual;

			if(m_strOld2310E != ANSI_2310E) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310E.IsEmpty() ? "<None>" : m_strOld2310E, strInfo);

				strNew = ANSI_2310E.IsEmpty() ? "<None>" : ANSI_2310E;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2310E_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310E = ANSI_2310E;

			// (j.jones 2009-08-03 14:26) - PLID 33827 - audit Export2310BRecord
			if(m_bOldExport2310BRecord != bExport2310B) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldExport2310BRecord) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bExport2310B) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_Export2310BRecord, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldExport2310BRecord = bExport2310B;

			// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
			if(m_strOld2000ATaxonomyCode != str2000ATaxonomy) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2000ATaxonomyCode.IsEmpty() ? "<None>" : m_strOld2000ATaxonomyCode, strInfo);

				strNew = str2000ATaxonomy.IsEmpty() ? "<None>" : str2000ATaxonomy;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2000A_Taxonomy, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2000ATaxonomyCode = str2000ATaxonomy;

			if(m_strOld2310BTaxonomyCode != str2310BTaxonomy) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310BTaxonomyCode.IsEmpty() ? "<None>" : m_strOld2310BTaxonomyCode, strInfo);

				strNew = str2310BTaxonomy.IsEmpty() ? "<None>" : str2310BTaxonomy;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2310B_Taxonomy, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310BTaxonomyCode = str2310BTaxonomy;

			if(m_strOld2420ATaxonomyCode != str2420ATaxonomy) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2420ATaxonomyCode.IsEmpty() ? "<None>" : m_strOld2420ATaxonomyCode, strInfo);

				strNew = str2420ATaxonomy.IsEmpty() ? "<None>" : str2420ATaxonomy;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiHCFA_2420A_Taxonomy, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2420ATaxonomyCode = str2420ATaxonomy;
		}
		else {

			// (j.jones 2006-11-13 15:18) - PLID 23446 - added settings for the ID
			// used in NM109 of all UB92 provider loops, and whether to use an extra REF in those loops
			// (j.jones 2008-04-01 17:46) - PLID 29486 - added ANSI_Hide2330AREF
			// (j.jones 2008-05-23 09:10) - PLID 29939 - added ANSI_SendRefPhyIn2300
			// (j.jones 2009-11-24 15:51) - PLID 36411 - added PriorAuthQualifier
			// (j.jones 2010-04-19 12:13) - PLID 38265 - added ANSI_StatementDateRange
			// (j.jones 2010-08-30 16:55) - PLID 15025 - added ANSI_SendTPLNumber
			// (j.jones 2012-03-26 14:44) - PLID 49175 - added PRV2000ACode and Use2000APRVSegment, and parameterized
			// (j.jones 2012-05-14 11:16) - PLID 50338 - added ANSI_SBR04
			// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
			// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
			// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
			// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
			ExecuteParamSql("UPDATE UB92SetupT SET NM109_IDType = {INT}, ExtraREF_IDType = {INT}, ANSI_Hide2330AREF = {INT}, "
				"ANSI_SendRefPhyIn2300 = {INT}, PriorAuthQualifier = {STRING}, ANSI_StatementDateRange = {INT}, ANSI_SendTPLNumber = {INT}, "
				"PRV2000ACode = {INT}, Use2000APRVSegment = {INT}, "
				"ANSI_2000B_SBR03 = {INT}, ANSI_2000B_SBR04 = {INT}, ANSI_2320_SBR04 = {INT}, ANSI_HideRefPhyFields = {INT}, "
				"OrigRefNo_2300 = {INT}, OrigRefNo_2330B = {INT} "
				"WHERE ID = {INT}", nNM109_IDType, nExtraREF_IDType, bANSI_Hide2330AREF ? 1 : 0,
				bSendRefPhyIn2300 ? 1 : 0, strPriorAuthQual, bANSI_StatementDateRange ? 1 : 0, nSendTPLNumber,
				(long)tco2000A, nUse2000APRVSegment,
				ANSI_2000B_SBR03, ANSI_2000B_SBR04, ANSI_2320_SBR04, (bANSI_HideRefPhyFields ? 1 : 0),
				(bOrigRefNo_2300 ? 1 : 0), (bOrigRefNo_2330B ? 1 : 0),
				m_GroupID);

			// (j.jones 2008-05-22 14:52) - PLID 29886 - audit the NM109 and ExtraREF fields
			if(m_nOldNM109 != nNM109_IDType) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//this field does not need provider and location names
				CString strOld, strNew;

				//0 = NPI, 1 = EIN/SSN

				if(m_nOldNM109 == 0) {
					strOld = "NPI";
				}
				else {
					strOld = "EIN/SSN";
				}

				if(nNM109_IDType == 0) {
					strNew = "NPI";
				}
				else {
					strNew = "EIN/SSN";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_NM109, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_nOldNM109 = nNM109_IDType;

			if(m_nOldExtraREF != nExtraREF_IDType) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//this field does not need provider and location names
				CString strOld, strNew;

				//0 = EIN/SSN, 1 - NPI, 2 - None

				if(m_nOldExtraREF == 0) {
					strOld = "EIN/SSN";
				}
				else if(m_nOldExtraREF == 1) {
					strOld = "NPI";
				}
				else {
					strOld = "Do Not Add";
				}

				if(nExtraREF_IDType == 0) {
					strNew = "EIN/SSN";
				}
				else if(nExtraREF_IDType == 1) {
					strNew = "NPI";
				}
				else {
					strNew = "Do Not Add";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_ExtraREF, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_nOldExtraREF = nExtraREF_IDType;

			ExecuteParamSql(GetRemoteData(), &nRecordsAffected, 
				// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here. If we
				// do, then nRecordsAffected may never be updated.
				// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
				"SET NOCOUNT OFF "
				"UPDATE UB92EbillingSetupT SET "
				"Use_2010AA = {INT}, "
				"ANSI_2010AA = {STRING}, "
				"ANSI_2010AA_Qual = {STRING}, "
				"Use_2010AB = {INT}, "
				"ANSI_2010AB = {STRING}, "
				"ANSI_2010AB_Qual = {STRING}, "
				"Use_2310A = {INT}, "
				"ANSI_2310A = {STRING}, "
				"ANSI_2310A_Qual = {STRING}, "
				"ANSI_2000A_Taxonomy = {STRING}, "
				"ANSI_2310A_Taxonomy = {STRING} "
				""
				"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
				(bUse_2010AA ? 1 : 0), ANSI_2010AA, ANSI_2010AA_Qual,
				(bUse_2010AB ? 1 : 0), ANSI_2010AB, ANSI_2010AB_Qual,
				(bUse_2310B ? 1 : 0), ANSI_2310B, ANSI_2310B_Qual,
				str2000ATaxonomy, str2310BTaxonomy,
				m_GroupID, m_ProviderID, m_LocationID);

			if(nRecordsAffected == 0) {
				ExecuteParamSql(GetRemoteData(), &nRecordsAffected, 
					// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here. If we
					// do, then nRecordsAffected may never be updated.
					// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
					"SET NOCOUNT OFF "
					"INSERT INTO UB92EbillingSetupT "
					"(SetupGroupID, ProviderID, LocationID, "
					"Use_2010AA, ANSI_2010AA, ANSI_2010AA_Qual, "
					"Use_2010AB, ANSI_2010AB, ANSI_2010AB_Qual, "
					"Use_2310A, ANSI_2310A, ANSI_2310A_Qual, "			
					"ANSI_2000A_Taxonomy, ANSI_2310A_Taxonomy)"
					"VALUES ({INT}, {INT}, {INT}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, {STRING}, {STRING}, "
					"{INT}, {STRING}, {STRING}, "
					"{STRING}, {STRING}) ",
					m_GroupID, m_ProviderID, m_LocationID, 
					(bUse_2010AA ? 1 : 0), ANSI_2010AA, ANSI_2010AA_Qual,
					(bUse_2010AB ? 1 : 0), ANSI_2010AB, ANSI_2010AB_Qual,
					(bUse_2310B ? 1 : 0), ANSI_2310B, ANSI_2310B_Qual,
					str2000ATaxonomy, str2310BTaxonomy);
			}

			// (j.jones 2008-05-22 15:40) - PLID 29886 - audit the overrides

			if(m_bOldUse2010AA != bUse_2010AA) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2010AA) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2010AA) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_Use2010AA, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2010AA = bUse_2010AA;

			if(m_strOld2010AAQual != ANSI_2010AA_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010AAQual.IsEmpty() ? "<None>" : m_strOld2010AAQual, strInfo);

				strNew = ANSI_2010AA_Qual.IsEmpty() ? "<None>" : ANSI_2010AA_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2010AA_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010AAQual = ANSI_2010AA_Qual;

			if(m_strOld2010AA != ANSI_2010AA) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010AA.IsEmpty() ? "<None>" : m_strOld2010AA, strInfo);

				strNew = ANSI_2010AA.IsEmpty() ? "<None>" : ANSI_2010AA;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2010AA_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010AA = ANSI_2010AA;

			if(m_bOldUse2010AB != bUse_2010AB) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2010AB) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2010AB) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_Use2010AB, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2010AB = bUse_2010AB;

			if(m_strOld2010ABQual != ANSI_2010AB_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010ABQual.IsEmpty() ? "<None>" : m_strOld2010ABQual, strInfo);

				strNew = ANSI_2010AB_Qual.IsEmpty() ? "<None>" : ANSI_2010AB_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2010AB_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010ABQual = ANSI_2010AB_Qual;

			if(m_strOld2010AB != ANSI_2010AB) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2010AB.IsEmpty() ? "<None>" : m_strOld2010AB, strInfo);

				strNew = ANSI_2010AB.IsEmpty() ? "<None>" : ANSI_2010AB;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2010AB_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2010AB = ANSI_2010AB;

			if(m_bOldUse2310B != bUse_2310B) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				if(!m_bOldUse2310B) {
					strOld.Format("Disabled (%s)", strInfo);
				}
				else {
					strOld.Format("Enabled (%s)", strInfo);
				}

				if(!bUse_2310B) {
					strNew = "Disabled";
				}
				else {
					strNew = "Enabled";
				}

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_Use2310A, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_bOldUse2310B = bUse_2310B;

			if(m_strOld2310BQual != ANSI_2310B_Qual) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310BQual.IsEmpty() ? "<None>" : m_strOld2310BQual, strInfo);

				strNew = ANSI_2310B_Qual.IsEmpty() ? "<None>" : ANSI_2310B_Qual;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2310A_Qual, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310BQual = ANSI_2310B_Qual;

			if(m_strOld2310B != ANSI_2310B) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310B.IsEmpty() ? "<None>" : m_strOld2310B, strInfo);

				strNew = ANSI_2310B.IsEmpty() ? "<None>" : ANSI_2310B;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2310A_Value, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310B = ANSI_2310B;

			// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
			if(m_strOld2000ATaxonomyCode != str2000ATaxonomy) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2000ATaxonomyCode.IsEmpty() ? "<None>" : m_strOld2000ATaxonomyCode, strInfo);

				strNew = str2000ATaxonomy.IsEmpty() ? "<None>" : str2000ATaxonomy;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2000A_Taxonomy, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2000ATaxonomyCode = str2000ATaxonomy;

			if(m_strOld2310BTaxonomyCode != str2310BTaxonomy) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strOld, strNew, strInfo = GetFullAuditInformation();

				strOld.Format("%s (%s)", m_strOld2310BTaxonomyCode.IsEmpty() ? "<None>" : m_strOld2310BTaxonomyCode, strInfo);

				strNew = str2310BTaxonomy.IsEmpty() ? "<None>" : str2310BTaxonomy;

				AuditEvent(-1, m_strGroupName, nAuditTransactionID, aeiUB_2310A_Taxonomy, m_GroupID, strOld, strNew, aepMedium, aetChanged); 
			}
			m_strOld2310BTaxonomyCode = str2310BTaxonomy;
		}

		// (j.jones 2008-05-22 14:55) - PLID 29886 - added auditing
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		return TRUE;

	}NxCatchAllCall("Error saving ebilling information.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)

	return FALSE;
}

void CAdvEbillingSetup::Load()
{
	if(m_ProvList->CurSel == -1)
		return;

	if(m_LocList->CurSel == -1)
		return;

	m_bHasChanged = FALSE;
	m_bIsLoading = TRUE;

	// (j.jones 2008-04-29 16:23) - PLID 29619 - track whether any individual override was modified
	m_b2010AAChanged = FALSE;
	m_b2010ABChanged = FALSE;
	m_b2310BChanged = FALSE;
	m_b2310EChanged = FALSE;
	m_b2420AChanged = FALSE;
	m_bOldExport2310BRecord = TRUE;

	try {

		_RecordsetPtr rs;
		if(!m_bIsUB92) {
			rs = CreateParamRecordset("SELECT * FROM EbillingSetupT WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",m_GroupID,m_ProviderID,m_LocationID);
		}
		else {
			rs = CreateParamRecordset("SELECT * FROM UB92EbillingSetupT WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",m_GroupID,m_ProviderID,m_LocationID);
		}

		if(!rs->eof) {

			//ANSI

			// (j.jones 2008-05-22 15:17) - PLID 29886 - cache these values for auditing

			//IDC_2010AA:
			m_bOldUse2010AA = AdoFldBool(rs, "Use_2010AA",FALSE);
			((CButton*)GetDlgItem(IDC_2010AA))->SetCheck(m_bOldUse2010AA);
			GetDlgItem(IDC_2010AA_NUMBER)->EnableWindow(m_bOldUse2010AA);
			GetDlgItem(IDC_2010AA_QUALIFIER)->EnableWindow(m_bOldUse2010AA);
			m_strOld2010AA = AdoFldString(rs, "ANSI_2010AA","");
			SetDlgItemText(IDC_2010AA_NUMBER, m_strOld2010AA);
			m_strOld2010AAQual = AdoFldString(rs, "ANSI_2010AA_Qual","");
			SetDlgItemText(IDC_2010AA_QUALIFIER, m_strOld2010AAQual);

			//IDC_2010AB:
			m_bOldUse2010AB = AdoFldBool(rs, "Use_2010AB",FALSE);
			((CButton*)GetDlgItem(IDC_2010AB))->SetCheck(m_bOldUse2010AB);
			GetDlgItem(IDC_2010AB_NUMBER)->EnableWindow(m_bOldUse2010AB);
			GetDlgItem(IDC_2010AB_QUALIFIER)->EnableWindow(m_bOldUse2010AB);
			m_strOld2010AB = AdoFldString(rs, "ANSI_2010AB","");
			SetDlgItemText(IDC_2010AB_NUMBER, m_strOld2010AB);
			m_strOld2010ABQual = AdoFldString(rs, "ANSI_2010AB_Qual","");
			SetDlgItemText(IDC_2010AB_QUALIFIER, m_strOld2010ABQual);

			// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
			m_strOld2000ATaxonomyCode = AdoFldString(rs, "ANSI_2000A_Taxonomy","");
			m_nxedit2000ATaxonomy.SetWindowText(m_strOld2000ATaxonomyCode);

			if(!m_bIsUB92) {
				//IDC_2310B:
				m_bOldUse2310B = AdoFldBool(rs, "Use_2310B",FALSE);
				((CButton*)GetDlgItem(IDC_2310B))->SetCheck(m_bOldUse2310B);
				GetDlgItem(IDC_2310B_NUMBER)->EnableWindow(m_bOldUse2310B);
				GetDlgItem(IDC_2310B_QUALIFIER)->EnableWindow(m_bOldUse2310B);
				m_strOld2310B = AdoFldString(rs, "ANSI_2310B","");
				SetDlgItemText(IDC_2310B_NUMBER, m_strOld2310B);
				m_strOld2310BQual = AdoFldString(rs, "ANSI_2310B_Qual","");
				SetDlgItemText(IDC_2310B_QUALIFIER, m_strOld2310BQual);

				//IDC_2420A:
				m_bOldUse2420A = AdoFldBool(rs, "Use_2420A",FALSE);
				((CButton*)GetDlgItem(IDC_2420A))->SetCheck(m_bOldUse2420A);
				GetDlgItem(IDC_2420A_NUMBER)->EnableWindow(m_bOldUse2420A);
				GetDlgItem(IDC_2420A_QUALIFIER)->EnableWindow(m_bOldUse2420A);
				m_strOld2420A = AdoFldString(rs, "ANSI_2420A","");
				SetDlgItemText(IDC_2420A_NUMBER, m_strOld2420A);
				m_strOld2420AQual = AdoFldString(rs, "ANSI_2420A_Qual","");
				SetDlgItemText(IDC_2420A_QUALIFIER, m_strOld2420AQual);

				// (j.jones 2008-12-11 15:04) - PLID 32413 - added 2310E fields
				m_bOldUse2310E = AdoFldBool(rs, "Use_2310E",FALSE);
				m_checkUse2310E.SetCheck(m_bOldUse2310E);
				GetDlgItem(IDC_2310E_NUMBER)->EnableWindow(m_bOldUse2310E);
				GetDlgItem(IDC_2310E_QUALIFIER)->EnableWindow(m_bOldUse2310E);
				m_strOld2310E = AdoFldString(rs, "ANSI_2310E","");
				SetDlgItemText(IDC_2310E_NUMBER, m_strOld2310E);
				m_strOld2310EQual = AdoFldString(rs, "ANSI_2310E_Qual","");
				SetDlgItemText(IDC_2310E_QUALIFIER, m_strOld2310EQual);

				// (j.jones 2009-08-03 14:26) - PLID 33827 - added Export2310BRecord
				m_bOldExport2310BRecord = AdoFldBool(rs, "Export2310BRecord", TRUE);
				m_checkUse2310B.SetCheck(m_bOldExport2310BRecord);

				// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
				m_strOld2310BTaxonomyCode = AdoFldString(rs, "ANSI_2310B_Taxonomy","");
				m_nxedit2310BTaxonomy.SetWindowText(m_strOld2310BTaxonomyCode);
				m_strOld2420ATaxonomyCode = AdoFldString(rs, "ANSI_2420A_Taxonomy","");
				m_nxedit2420ATaxonomy.SetWindowText(m_strOld2420ATaxonomyCode);
			}
			else {
				//IDC_2310A:
				//re-use the 2310B member variables
				m_bOldUse2310B = AdoFldBool(rs, "Use_2310A",FALSE);
				((CButton*)GetDlgItem(IDC_2310B))->SetCheck(m_bOldUse2310B);
				GetDlgItem(IDC_2310B_NUMBER)->EnableWindow(m_bOldUse2310B);
				GetDlgItem(IDC_2310B_QUALIFIER)->EnableWindow(m_bOldUse2310B);
				m_strOld2310B = AdoFldString(rs, "ANSI_2310A","");
				SetDlgItemText(IDC_2310B_NUMBER, m_strOld2310B);
				m_strOld2310BQual = AdoFldString(rs, "ANSI_2310A_Qual","");
				SetDlgItemText(IDC_2310B_QUALIFIER, m_strOld2310BQual);

				// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
				m_strOld2310BTaxonomyCode = AdoFldString(rs, "ANSI_2310A_Taxonomy","");
				m_nxedit2310BTaxonomy.SetWindowText(m_strOld2310BTaxonomyCode);
			}
		}
		else {

			//set all the boxes to be grayed and unchecked

			//ANSI

			//IDC_2010AA:
			((CButton*)GetDlgItem(IDC_2010AA))->SetCheck(FALSE);
			GetDlgItem(IDC_2010AA_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_2010AA_QUALIFIER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_2010AA_NUMBER,"");
			SetDlgItemText(IDC_2010AA_QUALIFIER,"");

			//IDC_2010AB:
			((CButton*)GetDlgItem(IDC_2010AB))->SetCheck(FALSE);
			GetDlgItem(IDC_2010AB_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_2010AB_QUALIFIER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_2010AB_NUMBER,"");
			SetDlgItemText(IDC_2010AB_QUALIFIER,"");

			//IDC_2310B:
			((CButton*)GetDlgItem(IDC_2310B))->SetCheck(FALSE);
			GetDlgItem(IDC_2310B_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_2310B_QUALIFIER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_2310B_NUMBER,"");
			SetDlgItemText(IDC_2310B_QUALIFIER,"");

			//IDC_2420A:
			((CButton*)GetDlgItem(IDC_2420A))->SetCheck(FALSE);
			GetDlgItem(IDC_2420A_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_2420A_QUALIFIER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_2420A_NUMBER,"");
			SetDlgItemText(IDC_2420A_QUALIFIER,"");

			// (j.jones 2008-12-11 15:04) - PLID 32413 - added 2310E fields
			m_checkUse2310E.SetCheck(FALSE);
			GetDlgItem(IDC_2310E_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_2310E_QUALIFIER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_2310E_NUMBER,"");
			SetDlgItemText(IDC_2310E_QUALIFIER,"");

			// (j.jones 2009-08-03 14:26) - PLID 33827 - added Export2310BRecord
			m_checkUse2310B.SetCheck(TRUE);
			m_bOldExport2310BRecord = TRUE;

			// (z.manning 2008-08-28 09:23) - PLID 31188 - If provider/location combination doesn't have
			// any data then we need to make sure we clear out our audit cache variables.
			m_bOldUse2010AA = FALSE;
			m_strOld2010AAQual.Empty();
			m_strOld2010AA.Empty();
			m_bOldUse2010AB = FALSE;
			m_strOld2010ABQual.Empty();
			m_strOld2010AB.Empty();
			m_bOldUse2310B = FALSE;
			m_strOld2310BQual.Empty();
			m_strOld2310B.Empty();
			m_bOldUse2310E = FALSE;
			m_strOld2310EQual.Empty();
			m_strOld2310E.Empty();
			m_bOldUse2420A = FALSE;
			m_strOld2420AQual.Empty();
			m_strOld2420A.Empty();

			// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
			m_nxedit2000ATaxonomy.SetWindowText("");
			m_nxedit2310BTaxonomy.SetWindowText("");
			m_nxedit2420ATaxonomy.SetWindowText("");
			m_strOld2000ATaxonomyCode = "";
			m_strOld2310BTaxonomyCode = "";
			m_strOld2420ATaxonomyCode = "";
		}

		// (j.jones 2010-11-01 09:53) - PLID 40919 - update the in-use label on the 2010AB override button		
		Update2010ABInUseLabel();
		Update2010AAInUseLabel();

		m_bIsLoading = FALSE;

	}NxCatchAll("Error loading ebilling information.");

}

BEGIN_EVENTSINK_MAP(CAdvEbillingSetup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvEbillingSetup)
	ON_EVENT(CAdvEbillingSetup, IDC_ADV_EBILL_PROVIDERS, 16 /* SelChosen */, OnSelChosenAdvEbillProviders, VTS_I4)
	ON_EVENT(CAdvEbillingSetup, IDC_ADV_EBILL_LOCATIONS, 16 /* SelChosen */, OnSelChosenAdvEbillLocations, VTS_I4)	
	ON_EVENT(CAdvEbillingSetup, IDC_PRIOR_AUTH_QUAL_COMBO, 1, OnSelChangingPriorAuthQualCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvEbillingSetup, IDC_SECONDARY_INS_CODE_DROPDOWN, 1, OnSelChangingSecondaryInsCodeDropdown, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvEbillingSetup, IDC_2000A_TAXONOMY_COMBO, 1, OnSelChanging2000aTaxonomyCombo, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CAdvEbillingSetup, IDC_HIDE_2310A_COMBO, 1, CAdvEbillingSetup::OnSelChangingHide2310aCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CAdvEbillingSetup::OnSelChosenAdvEbillProviders(long nRow) 
{
	if(nRow == -1)
		return;

	if(m_ProviderID != -1) {
		//PLID 21711 - check to see if they are still on the same provider
		long nProvID = VarLong(m_ProvList->GetValue(nRow, pccID));
		if (nProvID != m_ProviderID) {
			if (m_bHasChanged) {
				if(IDYES == MessageBox("Any changes made to the previous provider / location combination will be saved.\n"
					"Do you still wish to switch providers?","Practice",MB_ICONQUESTION|MB_YESNO)) {
	
					if(!Save()) {
						//the save failed, don't load and overwrite their changes yet
						m_ProvList->SetSelByColumn(pccID,(long)m_ProviderID);
						return;
					}
				}
				else {
					//don't load and overwrite their changes yet
					m_ProvList->SetSelByColumn(pccID,(long)m_ProviderID);
					return;
				}
			}
		}
		else {
			//don't load and overwrite the changes
			return;
		}
	}

	m_ProviderID = m_ProvList->GetValue(nRow, pccID).lVal;

	Load();
}

void CAdvEbillingSetup::OnBtnSecondaryClaimSetup() 
{
	// (j.jones 2006-11-24 12:43) - PLID 23415 - added Secondary ANSI Claim config.
	CSecondaryANSIClaimConfigDlg dlg(this);
	dlg.m_GroupID = m_GroupID;
	// (j.jones 2006-11-27 17:30) - PLID 23652 - supported UB92
	dlg.m_bIsUB92 = m_bIsUB92;
	// (j.jones 2010-10-19 13:32) - PLID 40931 - pass in the value for 5010
	dlg.m_bIs5010Enabled = m_bIs5010Enabled;
	dlg.DoModal();
}

void CAdvEbillingSetup::OnSelChosenAdvEbillLocations(long nRow) 
{
	if(nRow == -1)
		return;

	if(m_LocationID != -1) {
		//check to see if they are still on the same location
		long nLocID = VarLong(m_LocList->GetValue(nRow, lccID));
		if (nLocID != m_LocationID) {
			if (m_bHasChanged) {
				if(IDYES == MessageBox("Any changes made to the previous provider / location combination will be saved.\n"
					"Do you still wish to switch locations?","Practice",MB_ICONQUESTION|MB_YESNO)) {
	
					if(!Save()) {
						//the save failed, don't load and overwrite their changes yet
						m_LocList->SetSelByColumn(lccID,(long)m_LocationID);
						return;
					}
				}
				else {
					//don't load and overwrite their changes yet
					m_LocList->SetSelByColumn(lccID,(long)m_LocationID);
					return;
				}
			}
		}
		else {
			//don't load and overwrite the changes
			return;
		}
	}

	m_LocationID = VarLong(m_LocList->GetValue(nRow,lccID),-1);

	Load();
}

// (j.jones 2008-02-06 17:07) - PLID 28843 - added handler for the secondary ins. code setting
void CAdvEbillingSetup::OnCheckSecondaryInsTypeCode() 
{
	try {

		m_SecondaryCodeList->Enabled = m_checkUseSecondaryInsCode.GetCheck();

	}NxCatchAll("Error in CAdvEbillingSetup::OnCheckSecondaryInsTypeCode");
}

// (j.jones 2008-04-29 15:18) - PLID 29619 - validates an individual ANSI ID and qualifier,
// if both are blank then the strLabel is added to strBlankOverrideNames,
// if only one or the other is blank then the strLabel is added to strInvalidOverrideNames
// (j.jones 2010-04-15 17:28) - PLID 38149 - we now warn when the qualifier is XX, using strXXOverrideNames
void CAdvEbillingSetup::ValidateOneANSIOverride(const CString strLabel, BOOL bFieldChanged, const CString strQual, const CString strID, CString &strInvalidOverrideNames, CString &strBlankOverrideNames, CString &strXXOverrideNames)
{
	//for the blank warning
	if(strQual.IsEmpty() && strID.IsEmpty()) {
		//both are empty, which is allowed but we will later warn about this,
		//so track the strLabel in strBlankOverrideNames

		//only track it if the field in question changed
		if(bFieldChanged) {
			if(!strBlankOverrideNames.IsEmpty()) {
				strBlankOverrideNames += "\n";
			}
			strBlankOverrideNames += strLabel;
		}
	}
	else if(strQual.IsEmpty() || strID.IsEmpty()) {
		//one is empty, one is not, which is not allowed
		//the caller will handle this, so track the strLabel in strInvalidOverrideNames

		if(!strInvalidOverrideNames.IsEmpty()) {
			strInvalidOverrideNames += "\n";
		}
		strInvalidOverrideNames += strLabel;
	}

	// (j.jones 2010-04-15 17:28) - PLID 38149 - we now warn when the qualifier is XX, using strXXOverrides
	if(strQual.CompareNoCase("XX") == 0) {
		//only track it if the field in question changed
		if(bFieldChanged) {
			if(!strXXOverrideNames.IsEmpty()) {
				strXXOverrideNames += "\n";
			}
			strXXOverrideNames += strLabel;
		}
	}
}

// (j.jones 2008-05-22 14:55) - PLID 29886 - returns provider and location names for consistent auditing
//DRT 8/26/2008 - PLID 31174 - This cannot load from the list, as this function is called from the Save(), 
//	and the list may have already changed.  We can only trust m_ProviderID and m_LocationID.
CString CAdvEbillingSetup::GetFullAuditInformation()
{
	try {

		CString strProviderName = "";
		CString strLocationName = "";

		if(m_ProviderID == -1) {
			//saving shouldn't have occurred, thus this function should not have been called
			ASSERT(FALSE);
		}
		else {
			//For efficiency, just try to pull the name out of the datalist
			long nIndex = m_ProvList->FindByColumn(pccID, (long)(m_ProviderID), 0, VARIANT_FALSE);
			if(nIndex == -1) {
				//Couldn't be found in the datalist.  Look it up in data.
				_RecordsetPtr prsTest = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_ProviderID);
				if(!prsTest->eof) {
					strProviderName = AdoFldString(prsTest, "Name");
				}
			}
			else {
				strProviderName = VarString(m_ProvList->GetValue(nIndex, pccName), "");
			}
		}

		if(m_LocationID == -1) {
			//saving shouldn't have occurred, thus this function should not have been called
			ASSERT(FALSE);
		}
		else {
			//For efficiency, just try to pull the name out of the datalist
			long nIndex = m_LocList->FindByColumn(lccID, (long)(m_LocationID), 0, VARIANT_FALSE);
			if(nIndex == -1) {
				//Couldn't be found in the datalist.  Look it up in data.
				_RecordsetPtr prsTest = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_LocationID);
				if(!prsTest->eof) {
					strLocationName = AdoFldString(prsTest, "Name");
				}
			}
			else {
				strLocationName = VarString(m_LocList->GetValue(nIndex, lccName), "");
			}
		}

		return strProviderName + " / "  + strLocationName;		

	}NxCatchAll("Error in CAdvEbillingSetup::GetFullAuditInformation");

	return "";
}

// (j.jones 2008-06-23 09:44) - PLID 30434 - added OnBtnEligibilitySetup
void CAdvEbillingSetup::OnBtnEligibilitySetup()
{
	try {

		CEligibilitySetupDlg dlg(this);
		dlg.m_nGroupID = m_GroupID;
		dlg.m_strGroupName = m_strGroupName;
		dlg.DoModal();			

	}NxCatchAll("Error in CAdvEbillingSetup::OnBtnEligibilitySetup");
}

// (j.jones 2008-10-06 13:19) - PLID 31578 - added OnBnClickedCheckSendBox19
void CAdvEbillingSetup::OnBnClickedCheckSendBox19()
{
	try {

		//disable/enable the 2300/2400 options
		BOOL bEnabled = m_checkSendBox19.GetCheck();
		m_radioSendBox19_2300NTE.EnableWindow(bEnabled);
		m_radioSendBox19_2400NTE.EnableWindow(bEnabled);

	}NxCatchAll("Error in CAdvEbillingSetup::OnBnClickedCheckSendBox19");
}

// (j.jones 2009-12-10 17:36) - PLID 36411 - flag that this field changed
void CAdvEbillingSetup::OnSelChangingPriorAuthQualCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}

	}NxCatchAll("Error in CAdvEbillingSetup::OnSelChangingPriorAuthQualCombo");
}

// (j.jones 2009-12-10 17:36) - PLID 36411 - flag that this field changed
void CAdvEbillingSetup::OnSelChangingSecondaryInsCodeDropdown(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}

	}NxCatchAll("Error in CAdvEbillingSetup::OnSelChangingSecondaryInsCodeDropdown");
}

// (j.jones 2010-11-01 09:28) - PLID 40919 - added ability to override the 2010AB address
void CAdvEbillingSetup::OnBtnEdit2010ABAddress()
{
	try {

		if(m_ProviderID == -1 || m_LocationID == -1) {
			return;
		}

		CAdv2010ABPayToOverrideDlg dlg(this);
		// (j.jones 2011-11-16 16:01) - PLID 46489 - we need to tell the dialog to load 2010AB
		if(dlg.DoModal(m_ProviderID, m_LocationID, m_bIsUB92, m_GroupID, FALSE) == IDOK) {
			//update the in-use label on the button
			Update2010ABInUseLabel();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-11-01 09:47) - PLID 40919 - updates the 2010AB button to reflect if it is in use
void CAdvEbillingSetup::Update2010ABInUseLabel()
{
	try {

		if(m_ProviderID == -1 || m_LocationID == -1) {
			return;
		}

		if(!m_bIs5010Enabled) {
			//this button is not displayed in 4010, so do nothing
			return;
		}
		
		_RecordsetPtr rs;
		if(m_bIsUB92) {
			//UB
			rs = CreateParamRecordset("SELECT SetupGroupID FROM UB92EbillingSetupT "
				"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT} "
				"AND "
				"("
				"(PayTo2010AB_Address1 Is Not Null AND PayTo2010AB_Address1 <> '') "
				"OR (PayTo2010AB_Address2 Is Not Null AND PayTo2010AB_Address2 <> '') "
				"OR (PayTo2010AB_City Is Not Null AND PayTo2010AB_City <> '') "
				"OR (PayTo2010AB_State Is Not Null AND PayTo2010AB_State <> '') "
				"OR (PayTo2010AB_Zip Is Not Null AND PayTo2010AB_Zip <> '')"
				")",
				m_GroupID, m_ProviderID, m_LocationID);
		}
		else {
			//HCFA
			rs = CreateParamRecordset("SELECT SetupGroupID FROM EbillingSetupT "
				"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT} "
				"AND "
				"("
				"(PayTo2010AB_Address1 Is Not Null AND PayTo2010AB_Address1 <> '') "
				"OR (PayTo2010AB_Address2 Is Not Null AND PayTo2010AB_Address2 <> '') "
				"OR (PayTo2010AB_City Is Not Null AND PayTo2010AB_City <> '') "
				"OR (PayTo2010AB_State Is Not Null AND PayTo2010AB_State <> '') "
				"OR (PayTo2010AB_Zip Is Not Null AND PayTo2010AB_Zip <> '')"
				")",
				m_GroupID, m_ProviderID, m_LocationID);
		}

		if(!rs->eof) {
			SetDlgItemText(IDC_BTN_EDIT_2010AB_ADDRESS, "Override 2010AB Pay-To Address (In Use)");
			m_btnEdit2010AB_Address.SetTextColor(RGB(255,0,0));
			m_btnEdit2010AB_Address.Invalidate();
		}
		else {
			SetDlgItemText(IDC_BTN_EDIT_2010AB_ADDRESS, "Override 2010AB Pay-To Address");
			m_btnEdit2010AB_Address.SetTextColor(RGB(0,0,0));
			m_btnEdit2010AB_Address.Invalidate();
		}
		rs->Close();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-16 16:11) - PLID 46489 - added ability to override the 2010AA address
void CAdvEbillingSetup::OnBtnEdit2010AAAddress()
{
	try {

		if(m_ProviderID == -1 || m_LocationID == -1) {
			return;
		}

		CAdv2010ABPayToOverrideDlg dlg(this);
		if(dlg.DoModal(m_ProviderID, m_LocationID, m_bIsUB92, m_GroupID, TRUE) == IDOK) {
			//update the in-use label on the button
			Update2010AAInUseLabel();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-16 16:11) - PLID 46489 - updates the 2010AA button to reflect if it is in use
void CAdvEbillingSetup::Update2010AAInUseLabel()
{
	try {

		if(m_ProviderID == -1 || m_LocationID == -1) {
			return;
		}

		if(!m_bIs5010Enabled) {
			//this button is not displayed in 4010, so do nothing
			return;
		}
		
		_RecordsetPtr rs;
		if(m_bIsUB92) {
			//UB
			rs = CreateParamRecordset("SELECT SetupGroupID FROM UB92EbillingSetupT "
				"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT} "
				"AND "
				"("
				"(PayTo2010AA_Address1 Is Not Null AND PayTo2010AA_Address1 <> '') "
				"OR (PayTo2010AA_Address2 Is Not Null AND PayTo2010AA_Address2 <> '') "
				"OR (PayTo2010AA_City Is Not Null AND PayTo2010AA_City <> '') "
				"OR (PayTo2010AA_State Is Not Null AND PayTo2010AA_State <> '') "
				"OR (PayTo2010AA_Zip Is Not Null AND PayTo2010AA_Zip <> '')"
				")",
				m_GroupID, m_ProviderID, m_LocationID);
		}
		else {
			//HCFA
			rs = CreateParamRecordset("SELECT SetupGroupID FROM EbillingSetupT "
				"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT} "
				"AND "
				"("
				"(PayTo2010AA_Address1 Is Not Null AND PayTo2010AA_Address1 <> '') "
				"OR (PayTo2010AA_Address2 Is Not Null AND PayTo2010AA_Address2 <> '') "
				"OR (PayTo2010AA_City Is Not Null AND PayTo2010AA_City <> '') "
				"OR (PayTo2010AA_State Is Not Null AND PayTo2010AA_State <> '') "
				"OR (PayTo2010AA_Zip Is Not Null AND PayTo2010AA_Zip <> '')"
				")",
				m_GroupID, m_ProviderID, m_LocationID);
		}

		if(!rs->eof) {
			SetDlgItemText(IDC_BTN_EDIT_2010AA_ADDRESS, "Override 2010AA Billing Provider Address (In Use)");
			m_btnEdit2010AA_Address.SetTextColor(RGB(255,0,0));
			m_btnEdit2010AA_Address.Invalidate();
		}
		else {
			SetDlgItemText(IDC_BTN_EDIT_2010AA_ADDRESS, "Override 2010AA Billing Provider Address");
			m_btnEdit2010AA_Address.SetTextColor(RGB(0,0,0));
			m_btnEdit2010AA_Address.Invalidate();
		}
		rs->Close();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-03-23 15:51) - PLID 49176 - added 2000A taxonomy dropdown
void CAdvEbillingSetup::OnSelChanging2000aTaxonomyCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//if they picked a valid selection, flag that something changed
		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A settings
void CAdvEbillingSetup::OnSelChangingHide2310aCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//if they picked a valid selection, flag that something changed
		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}

	}NxCatchAll(__FUNCTION__);
}
