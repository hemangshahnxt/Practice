// General1Dlg.cpp : implementation file

#include "stdafx.h"
#include "General1Dlg.h"
#include "MainFrm.h"
#include "GlobalUtils.h"
//#include "LabelControl.h"
//#include "SelectStatusDlg.h"
#include "NxTabView.h"
#include "PracProps.h"
#include "SuperBill.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"
#include "Client.h"
#include "Mirror.h"
//#include "NxErrorDialog.h"
#include "InformLink.h"
#include "UnitedLink.h"
#include "MirrorLink.h"
#include "ImageDlg.h"
#include "Graphics.h"
#include "PatientGroupsDlg.h"
#include "PatientsRc.h"
#include "AuditTrail.h"
#include "QuickbooksUtils.h"
#include "QuickbooksLink.h"
#include "PatientToolbar.h"
#include "FileUtils.h"
#include "CareCreditUtils.h"
#include "NxCanfieldLink.h"
#include "PhoneNumberEntryDlg.h"
#include "OHIPUtils.h"
#include "RSIMMSLink.h"
#include "SecurityGroupsDlg.h"
#include "AlbertaHLINKUtils.h"
#include "RecallsNeedingAttentionDlg.h"	// (j.armen 2012-02-24 16:29) - PLID 48303
#include "RecallUtils.h"	//(a.wilson 2012-3-5) PLID 48485
#include "DecisionRuleUtils.h"

// Practice image stuff
// (a.walling 2008-04-10 12:52) - PuntoEXE image processing no longer used
/*
#include "imagergbbuffer.h"
#include "imagecodecprocessor.h"
#include "imagedrawprocessor.h"
#include "imageDibCodec.h"          // DIB codec
#include "imageJpegCodec.h"         // Jpeg codec
*/

#include "EditPrefixesDlg.h"

#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "globalschedutils.h"
#include "ppclink.h"
#include "OPOSMSRDevice.h"
#include "dontshowdlg.h"
#include "HL7Utils.h"
#include "PatientSummaryDlg.h"
#include "WellnessDataUtils.h"

#include "Reports.h"
#include "GlobalReportUtils.h"
#include "PatientView.h"
#include "DevicePlugin.h"			// (d.lange 2010-06-22 16:52) - PLID 38687 - Implementation for sending patient demographics to devices
#include "DevicePluginUtils.h"
#include "AlbertaHLINKUtils.h"
#include "OPOSBarcodeScanner.h"	//(a.wilson 2012-1-13) PLID 47485
#include "GlassesEMNPrescriptionList.h"

#include "DeviceLaunchUtils.h" // (j.gruber 2013-04-02 12:57) - PLID 56012
#include "PatientReminderSenthistoryDlg.h"	//(s.dhole 8/29/2014 4:04 PM ) - PLID 62751

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace


// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#define IDT_REFRESH 187
//#define AUTOREFRESH

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2012-11-05 11:58) - PLID 53588 - Resolve conflict with mshtmcid.h
#define IDM_COPY_IMAGE 40000

#define IDM_VIEW_IMAGE 40001
//#define IDM_RELOAD 40002

#define ID_UPDATE_INFORM 40003
#define ID_UPDATE_MIRROR 40004
#define ID_UPDATE_UNITED 40005
#define ID_UPDATE_QBOOKS 40006

#define ID_LINK_TO_INFORM 40007
#define ID_LINK_TO_MIRROR 40008
#define ID_LINK_TO_UNITED 40009
#define ID_LINK_TO_QUICKBOOKS 40010
#define ID_LINK_TO_CARECREDIT 40011

//'T'hird 'P'arty 'L'inks
#define TPL_SEND_TO_INFORM		0x000001
#define TPL_UPDATE_INFORM		0x000002
#define TPL_DISABLE_INFORM		0x000004
#define TPL_SEND_TO_MIRROR		0x000008
#define TPL_UPDATE_MIRROR		0x000010
#define TPL_DISABLE_MIRROR		0x000020
#define TPL_SEND_TO_UNITED		0x000040
#define TPL_UPDATE_UNITED		0x000080
#define TPL_DISABLE_UNITED		0x000100
#define TPL_SEND_TO_QBOOKS		0x000200
#define TPL_UPDATE_QBOOKS		0x000400
#define TPL_DISABLE_QBOOKS		0x000800
#define TPL_SEND_TO_CARECREDIT	0x001000
#define TPL_HAS_LAUNCH_DEVICE	0x002000 // (j.gruber 2013-04-22 11:47) - PLID 56361

#define HAS_NO_LINKS		-1
#define HAS_MULTIPLE_LINKS	0
#define HAS_ONLY_INFORM		1
#define HAS_ONLY_MIRROR		2
#define HAS_ONLY_UNITED		3
#define HAS_ONLY_QBOOKS		4
#define HAS_ONLY_CARECREDIT	5
#define HAS_ONLY_DEVICE		6			// (d.lange 2010-06-30 15:31) - PLID 38687 - Added to show only devices if everything else is disabled


// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-03-09 14:17) - PLID 37640 - Moved to cpp
CGeneral1ImageLoad::CGeneral1ImageLoad(CGeneral1Dlg* pMsgWnd, CMirrorPatientImageMgr* pMirrorImageMgr, CString strRemoteID, long nImageIndex, long nImageCount, long nPatientID, CMirrorImageButton *pButton)
{
	m_pMsgWnd = pMsgWnd;
	m_pMirrorImageMgr = new CMirrorPatientImageMgr(pMirrorImageMgr);
	m_strRemoteID = strRemoteID;
	m_nImageIndex = nImageIndex;
	m_nImageCount = nImageCount;
	m_nPatientID = nPatientID;
	m_pButton = pButton;
}

CGeneral1ImageLoad::~CGeneral1ImageLoad()
{
	if (NULL != m_pMirrorImageMgr) {
		delete m_pMirrorImageMgr;
	}
}

//Preference types are hardcoded, so I moved them all up here to be easier to read
CString GetPreferredContactTypeStringFromID(long nID)
{
	switch(nID) {
		//These are all hardcoded
	case -1:
	case 0:
		return "<No Preference>";
		break;
	case 1:
		return "Home Phone";
		break;
	case 2:
		return "Work Phone";
		break;
	case 3:
		return "Mobile Phone";
		break;
	case 4:
		return "Pager";
		break;
	case 5:
		return "Other Phone";
		break;
	case 6:
		return "Email";
		break;
	case 7:// (a.vengrofski 2010-02-01 13:10) - PLID <34208> - Added for the "text messaging"
		return "Text Messaging";// (a.vengrofski 2010-02-01 13:10) - PLID <34208> 
		break;// (a.vengrofski 2010-02-01 13:10) - PLID <34208> 
	default:
		return "";
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CGeneral1Dlg dialog

// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
CGeneral1Dlg::CGeneral1Dlg(CWnd* pParent)
	: CPatientDialog(CGeneral1Dlg::IDD, pParent)
,
m_doctorChecker(NetUtils::Providers),
m_coordChecker(NetUtils::Coordinators),
m_partnerChecker(NetUtils::PatCombo)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/select_a_patient.htm";

	// (a.walling 2010-10-12 14:40) - PLID 40908
	m_id = -1;
	m_strFirstName = "";
	m_strMiddleName = "";
	m_strLastName = "";
	m_dwSendToStatus = 0;
	m_pLoadImageThread = NULL;
	m_bSavingContactDate = false;
	m_bBirthDateSet = false;
	m_RightClicked = NULL;
	m_bFormattingField = false;
	m_bFormattingAreaCode = false;
	m_bProcessingCard = false;
	m_nMMSPatientID = -1;
	m_pMirrorImageMgr = NULL;
	//{{AFX_DATA_INIT(CGeneral1Dlg)
	//}}AFX_DATA_INIT
}

void CGeneral1Dlg::DoDataExchange(CDataExchange* pDX)
{
	//TES 4/4/2008 - PLID 29550 - Converted to CNxEdits.
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneral1Dlg)
	DDX_Control(pDX, IDC_BTN_PATIENT_SUMMARY, m_btnPatientSummary);
	DDX_Control(pDX, IDC_TITLE_BOX, m_nxeTitle);
	DDX_Control(pDX, IDC_WORK_PHONE_BOX, m_nxeWorkPhone);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeZip);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeState);
	DDX_Control(pDX, IDC_SOC_SEC_NO_BOX, m_nxeSSN);
	DDX_Control(pDX, IDC_REFERRAL, m_nxeReferral);
	DDX_Control(pDX, IDC_PAGER_PHONE_BOX, m_nxePager);
	DDX_Control(pDX, IDC_OTHER_PHONE_BOX, m_nxeOtherPhone);
	DDX_Control(pDX, IDC_NOTES, m_nxeNotes);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeLastName);
	DDX_Control(pDX, IDC_ID_BOX, m_nxeID);
	DDX_Control(pDX, IDC_HOME_PHONE_BOX, m_nxeHomePhone);
	DDX_Control(pDX, IDC_FAX_PHONE_BOX, m_nxeFax);
	DDX_Control(pDX, IDC_EXT_PHONE_BOX, m_nxeExtension);
	DDX_Control(pDX, IDC_ENTERED_BY, m_nxeEnteredBy);
	DDX_Control(pDX, IDC_EMERGENCY_WORK, m_nxeEmergWorkPhone);
	DDX_Control(pDX, IDC_EMERGENCY_RELATE, m_nxeEmergRelation);
	DDX_Control(pDX, IDC_EMERGENCY_LAST_NAME, m_nxeEmergLast);
	DDX_Control(pDX, IDC_EMERGENCY_HOME, m_nxeEmergHomePhone);
	DDX_Control(pDX, IDC_EMERGENCY_FIRST_NAME, m_nxeEmergFirst);
	DDX_Control(pDX, IDC_EMAIL_BOX, m_nxeEmail);
	DDX_Control(pDX, IDC_DEAR_BOX, m_nxeDear);
	DDX_Control(pDX, IDC_CUSTOM4_BOX, m_nxeCustom4);
	DDX_Control(pDX, IDC_CUSTOM3_BOX, m_nxeCustom3);
	DDX_Control(pDX, IDC_CUSTOM2_BOX, m_nxeCustom2);
	DDX_Control(pDX, IDC_CUSTOM1_BOX, m_nxeCustom1);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeCity);
	DDX_Control(pDX, IDC_CELL_PHONE_BOX, m_nxeCellPhone);
	DDX_Control(pDX, IDC_AGE_BOX, m_nxeAge);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeAddress2);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeAddress1);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeMiddleName);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeFirstName);
	DDX_Control(pDX, IDC_EMAIL_PRIV_CHECK, m_btnEmailPriv);
	DDX_Control(pDX, IDC_OTHER_PRIV_CHECK, m_btnOtherPriv);
	DDX_Control(pDX, IDC_FAX_PRIV_CHECK, m_btnFaxPriv);
	DDX_Control(pDX, IDC_PAGER_PRIV_CHECK, m_btnPagerPriv);
	DDX_Control(pDX, IDC_CELL_PRIV_CHECK, m_CellPrivCheck);
	DDX_Control(pDX, IDC_EXCLUDE_MAILINGS, m_ExcludeMailingsCheck);
	DDX_Control(pDX, IDC_SEND_TO_THIRD_PARTY, m_btnSendToThirdParty);
	DDX_Control(pDX, IDC_IMAGE_NEXT, m_imageNext);
	DDX_Control(pDX, IDC_IMAGE_LAST, m_imageLast);
	DDX_Control(pDX, IDC_IMAGE, m_imageButton);
	DDX_Control(pDX, IDC_IMAGE_LEFT, m_imageButtonLeft);
	DDX_Control(pDX, IDC_IMAGE_RIGHT, m_imageButtonRight);
	DDX_Control(pDX, IDC_IMAGE_UPPER_LEFT, m_imageButtonUpperLeft);
	DDX_Control(pDX, IDC_IMAGE_UPPER_RIGHT, m_imageButtonUpperRight);
	DDX_Control(pDX, IDC_IMAGE_LOWER_LEFT, m_imageButtonLowerLeft);
	DDX_Control(pDX, IDC_IMAGE_LOWER_RIGHT, m_imageButtonLowerRight);
	DDX_Control(pDX, IDC_MARRIAGE_OTHER_BOX, m_MarriageOther);
	DDX_Control(pDX, IDC_HOME_PRIV_CHECK, m_HomePrivCheck);
	DDX_Control(pDX, IDC_INACTIVE_CHECK, m_InactiveCheck);
	DDX_Control(pDX, IDC_FOREIGN_CHECK, m_ForeignCheck);
	DDX_Control(pDX, IDC_WORK_PRIV_CHECK, m_WorkPrivCheck);
	DDX_Control(pDX, IDC_SINGLE_RAD, m_singleRad);
	DDX_Control(pDX, IDC_MARRIED_RAD, m_marriedRad);
	DDX_Control(pDX, IDC_OTHER_RAD, m_otherRad);
	DDX_Control(pDX, IDC_IS_PATIENT, m_isPatient);
	DDX_Control(pDX, IDC_IS_PATIENT_PROSPECT, m_isPatientProspect);
	DDX_Control(pDX, IDC_IS_PROSPECT, m_isProspect);
	DDX_Control(pDX, IDC_CUSTOM_BKG, m_customBkg);
	DDX_Control(pDX, IDC_EMERG_BKG, m_emergBkg);
	DDX_Control(pDX, IDC_PHONE_BKG, m_phoneBkg);
	DDX_Control(pDX, IDC_REFERRAL_BKG, m_referralBkg);
	DDX_Control(pDX, IDC_DEMOGRAPHICS_BKG, m_demBkg);
	DDX_Control(pDX, IDC_PARTNER_LABEL, m_nxstaticPartnerLabel);
	DDX_Control(pDX, IDC_CUSTOM1_LABEL, m_nxstaticCustom1Label);
	DDX_Control(pDX, IDC_CUSTOM4_LABEL, m_nxstaticCustom4Label);
	DDX_Control(pDX, IDC_CUSTOM3_LABEL, m_nxstaticCustom3Label);
	DDX_Control(pDX, IDC_CUSTOM2_LABEL, m_nxstaticCustom2Label);
	DDX_Control(pDX, IDC_CELL_TEXT_MESSAGE, m_btnTextMessage);
	DDX_Control(pDX, IDC_EMAIL_DECLINED_LABEL, m_nxlabelDeclinedEmail);
	DDX_Control(pDX, IDC_EMAIL_SET_DECLINED_LABEL, m_nxlabelSetEmailDeclined);
	DDX_Control(pDX, IDC_EDIT_SECURITY_GROUPS, m_btnEditSecurityGroups);
	DDX_Control(pDX, IDC_COMPANY_LINK_LABEL, m_nxlabelSetCompanyLink);// (s.dhole 2010-03-26 16:50) - PLID 37796 For internal only, make the company show up in G1.
	DDX_Control(pDX, IDC_SHOW_VISION_PRESCRIPTION, m_nxbtnVisonPriscription);
	
	DDX_Control(pDX, IDC_RECALL_PATIENT, m_btnRecall);	// (j.armen 2012-02-24 16:30) - PLID 48303
	DDX_Control(pDX, IDC_BTN_SENT_REMINDER, m_btnReminder);	//(s.dhole 8/28/2014 1:36 PM ) - PLID 62747 
	DDX_Control(pDX, IDC_STATIC_REMINDER, m_nxstaticReminder);	//(s.dhole 8/28/2014 1:36 PM ) - PLID 62747 
	

	
	//}}AFX_DATA_MAP
}

// (a.walling 2008-04-10 12:52) - PuntoEXE image processing no longer used
BEGIN_MESSAGE_MAP(CGeneral1Dlg, CNxDialog)
	//{{AFX_MSG_MAP(CGeneral1Dlg)
	ON_BN_CLICKED(IDC_FOREIGN_CHECK, OnForeignCheck)
	ON_BN_CLICKED(IDC_INACTIVE_CHECK, OnInactiveCheck)
	ON_BN_CLICKED(IDC_HOME_PRIV_CHECK, OnHomePrivCheck)
	ON_BN_CLICKED(IDC_WORK_PRIV_CHECK, OnWorkPrivCheck)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_IS_PATIENT, OnStatusChanged)
	ON_BN_CLICKED(IDC_IMAGE_LAST, OnImageLast)
	ON_BN_CLICKED(IDC_IMAGE_NEXT, OnImageNext)
	ON_BN_CLICKED(IDC_IMAGE, OnImage)
	ON_BN_CLICKED(IDC_IMAGE_LEFT, OnImageLeft)
	ON_BN_CLICKED(IDC_IMAGE_RIGHT, OnImageRight)
	ON_BN_CLICKED(IDC_IMAGE_UPPER_LEFT, OnImageUpperLeft)
	ON_BN_CLICKED(IDC_IMAGE_UPPER_RIGHT, OnImageUpperRight)
	ON_BN_CLICKED(IDC_IMAGE_LOWER_LEFT, OnImageLowerLeft)
	ON_BN_CLICKED(IDC_IMAGE_LOWER_RIGHT, OnImageLowerRight)
	ON_BN_CLICKED(IDC_EMAIL, OnEmail)	
	ON_BN_CLICKED(ID_UPDATE_INFORM, OnUpdateInform)
	ON_BN_CLICKED(IDC_SINGLE_RAD, OnSingleRad)
	ON_BN_CLICKED(IDC_MARRIED_RAD, OnMarriedRad)
	ON_BN_CLICKED(IDC_OTHER_RAD, OnOtherRad)
	ON_BN_CLICKED(ID_UPDATE_MIRROR, OnUpdateMirror)
	ON_BN_CLICKED(ID_UPDATE_UNITED, OnUpdateUnited)
	ON_BN_CLICKED(ID_UPDATE_QBOOKS, OnUpdateQBooks)
	ON_EN_KILLFOCUS(IDC_NOTES, OnKillfocusNotes)
	ON_COMMAND(IDM_COPY_IMAGE, OnCopyImage)
	ON_COMMAND(IDM_VIEW_IMAGE, OnViewImage)
	ON_BN_CLICKED(IDC_PARTNER_BTN, OnPartnerBtn)
	ON_BN_CLICKED(IDC_GROUPS, OnGroups)
	ON_CBN_SELCHANGE(IDC_FAX_CHOICE, OnSelChangeFaxChoice)
	ON_MESSAGE(NXM_G1THUMB_IMAGELOADED, OnImageLoaded)
	ON_BN_CLICKED(IDC_SEND_TO_THIRD_PARTY, OnSendToThirdParty)
	ON_WM_DESTROY()
	ON_MESSAGE(NXM_G1_LOSTFOCUS, OnLostFocus)
	ON_BN_CLICKED(IDC_EXCLUDE_MAILINGS, OnExcludeMailings)
	ON_BN_CLICKED(IDC_EDIT_PREFIXES, OnEditPrefixes)
	ON_BN_CLICKED(IDC_CELL_PRIV_CHECK, OnCellPrivCheck)
	ON_EN_KILLFOCUS(IDC_EMAIL_BOX, OnKillfocusEmailBox)
	ON_BN_CLICKED(IDC_PAGER_PRIV_CHECK, OnPagerPrivCheck)
	ON_BN_CLICKED(IDC_FAX_PRIV_CHECK, OnFaxPrivCheck)
	ON_BN_CLICKED(IDC_OTHER_PRIV_CHECK, OnOtherPrivCheck)
	ON_BN_CLICKED(IDC_EMAIL_PRIV_CHECK, OnEmailPrivCheck)
	ON_BN_CLICKED(ID_LINK_TO_INFORM, OnLinkToInform)
	ON_BN_CLICKED(ID_LINK_TO_MIRROR, OnLinkToMirror)
	ON_BN_CLICKED(ID_LINK_TO_UNITED, OnLinkToUnited)
	ON_BN_CLICKED(ID_LINK_TO_QUICKBOOKS, OnLinkToQuickbooks)
	ON_BN_CLICKED(IDC_IS_PATIENT_PROSPECT, OnStatusChanged)
	ON_BN_CLICKED(IDC_IS_PROSPECT, OnStatusChanged)
	ON_WM_KILLFOCUS()
	ON_BN_CLICKED(ID_LINK_TO_CARECREDIT, OnLinkToCareCredit)
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_BN_CLICKED(IDC_BTN_PATIENT_SUMMARY, OnBtnPatientSummary)
	ON_BN_CLICKED(IDC_CELL_TEXT_MESSAGE, OnCellTextMessage)
	ON_MESSAGE(NXM_MIRROR_CANFIELD_SDK_INIT_COMPLETE, OnCanfieldSDKInitComplete)
	ON_MESSAGE(NXM_BARSCAN_DATA_EVENT, OnBarcodeScannerDataEvent)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_EDIT_SECURITY_GROUPS, OnEditSecurityGroups)
	ON_BN_CLICKED(IDC_RECALL_PATIENT, OnBtnRecallClicked)	// (j.armen 2012-02-24 16:30) - PLID 48303
	ON_BN_CLICKED(IDC_SHOW_VISION_PRESCRIPTION, &CGeneral1Dlg::OnBnClickedShowVisionPrescription)
	
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SENT_REMINDER, &CGeneral1Dlg::OnBnClickedBtnSentReminder)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneral1Dlg message handlers


BEGIN_EVENTSINK_MAP(CGeneral1Dlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGeneral1Dlg)
	ON_EVENT(CGeneral1Dlg, IDC_PREFIX_COMBO, 2 /* SelChanged */, OnPullUpPrefixCombo, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_DOCTOR_COMBO, 16 /* SelChosen */, OnPullUpDoctorCombo, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_COORD_COMBO, 16 /* SelChosen */, OnPullUpCoordCombo, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_PARTNER_LIST, 16 /* SelChosen */, OnSelChosenPartnerList, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_GENDER_LIST, 11 /* ClosedUp */, OnClosedUpGenderList, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_PREFERRED_CONTACT_LIST, 16 /* SelChosen */, OnSelChosenPreferredContactList, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_PREFIX_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPrefixCombo, VTS_I2)
	ON_EVENT(CGeneral1Dlg, IDC_PREFIX_COMBO, 16 /* SelChosen */, OnSelChosenPrefixCombo, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_GENDER_LIST, 16 /* SelChosen */, OnSelChosenGenderList, VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_BIRTH_DATE_BOX, 1 /* KillFocus */, OnKillFocusBirthDateBox, VTS_NONE)
	ON_EVENT(CGeneral1Dlg, IDC_CONTACT_DATE, 1 /* KillFocus */, OnKillFocusContactDate, VTS_NONE)
	ON_EVENT(CGeneral1Dlg, IDC_DOCTOR_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedDoctorCombo, VTS_I4 VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_COORD_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCoordCombo, VTS_I4 VTS_I4)
	ON_EVENT(CGeneral1Dlg, IDC_DOCTOR_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedDoctorCombo, VTS_I2)
	ON_EVENT(CGeneral1Dlg, IDC_COORD_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCoordCombo, VTS_I2)
	ON_EVENT(CGeneral1Dlg, IDC_COUNTRY_LIST, 16 /* SelChosen */, CGeneral1Dlg::SelChosenCountryList, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CGeneral1Dlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (d.singleton 2012-06-18 11:21) - PLID 51029 set text limit on PersonT.Address(1, 2) fields
		m_nxeAddress1.SetLimitText(150);
		m_nxeAddress2.SetLimitText(150);

		// (b.eyers 2015-04-14) - PLID 25917 - set text limit on first, middle, last, city, state, and zip
		m_nxeFirstName.SetLimitText(50);
		m_nxeMiddleName.SetLimitText(50);
		m_nxeLastName.SetLimitText(50);
		m_nxeCity.SetLimitText(50);
		m_nxeState.SetLimitText(20);
		m_nxeZip.SetLimitText(20);

		//(e.lally 2007-08-07) PLID 20765 - Bulk cache our preferences
		//(e.lally 2011-08-26) PLID 44950 - Moved up above any uses
		// NxPropManager cache
			g_propManager.CachePropertiesInBulk("PatientGeneral1", propNumber,
				"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
				"Name = 'FormatPhoneNums' OR "
				"Name = 'LadderAssignment' OR "
				"Name = 'DisableQuickBooks' OR "
				"Name = 'AutoCapitalizeMiddleInitials' OR "
				"Name = 'General1ImageCount' OR "
				"Name = 'Gen1SaveEmails' OR "
				"Name = 'MirrorExportOverwrite' OR "
				"Name = 'GenderPrefixLink' OR "
				"Name = 'DefaultMalePrefix' OR "
				"Name = 'DefaultFemalePrefix' OR "
				"Name = 'MirrorAllowAsyncOperations' OR "
				// (j.jones 2009-06-24 10:41) - PLID 33650 - added the OHIP fields
				"Name = 'UseOHIP' OR "
				"Name = 'OHIP_HealthNumberCustomField' OR "
				"Name = 'OHIP_VersionCodeCustomField' OR "
				// (c.haag 2009-07-07 13:17) - PLID 34379 - RSI MMS properties
				//(e.lally 2011-08-25) PLID 44950 - This is a current user computer name pref.
				"Name = 'ShowRSIMMSThumbs' OR "
				// (j.gruber 2009-10-06 16:43) - PLID 35825 - CityCodeLookup
				"Name = 'LookupZipStateByCity' OR "
				// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
				"Name = 'UseAlbertaHLINK' OR "
				"Name = 'Alberta_PatientULICustomField' OR "
				//(c.copits 2011-01-18) PLID 27344 - Truncate trailing space on names and addresses
				"Name = 'TruncateTrailingNameAddressSpace' OR "
				// (c.haag 2011-01-20) - PLID 42166 - MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp
				"Name = 'MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp' OR "
				//(c.copits 2011-09-15) PLID 28544 - Reject invalid email addresses
				"Name = 'RejectInvalidEmailAddresses' "
				")",
				_Q(GetCurrentUserName()), _Q(GetCurrentUserComputerName()) );

			g_propManager.CachePropertiesInBulk("PaymentDlg-TextParam", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'PhoneFormatString' OR "
				"Name = 'InformDataPath' OR "
				// (c.haag 2009-07-07 13:16) - PLID 34379 - RSI MMS properties
				"Name = 'RSIMMSConnString' OR "
				"Name = 'RSIMMSSharedPath' "
				")",
				_Q(GetCurrentUserName()));
		
		m_imageNext.SetIcon(IDI_RARROW);
		m_imageLast.SetIcon(IDI_LARROW);

		
		// (j.jones 2008-07-09 10:30) - PLID 24624 - added m_btnPatientSummary
		m_btnPatientSummary.AutoSet(NXB_INSPECT);

		m_nxtBirthDate = GetDlgItemUnknown(IDC_BIRTH_DATE_BOX);
		m_nxtFirstContact = GetDlgItemUnknown(IDC_CONTACT_DATE);
	// (s.dhole 2012-02-27 16:40) - PLID 48354 Vision Rx
		m_nxbtnVisonPriscription.AutoSet(NXB_INSPECT );
		//JJ - don't requery the doctor or coordinator because the load does that
		m_DoctorCombo = BindNxDataListCtrl(IDC_DOCTOR_COMBO);
		m_CoordCombo = BindNxDataListCtrl(IDC_COORD_COMBO);
		m_PrefixCombo = BindNxDataListCtrl(IDC_PREFIX_COMBO);
		m_GenderCombo = BindNxDataListCtrl(IDC_GENDER_LIST, false);
		m_PreferredContactCombo = BindNxDataListCtrl(IDC_PREFERRED_CONTACT_LIST, false);
		// (s.dhole 2012-02-27 16:55) - PLID  48354 check License
		if(!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)) {
			m_nxbtnVisonPriscription.ShowWindow(SW_HIDE);
		}
		else
		{
			m_nxbtnVisonPriscription.ShowWindow(SW_SHOW );
		}

		// (j.armen 2012-03-28 09:12) - PLID 48480 - Only display the button if the user has the license
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
		{
			m_btnRecall.AutoSet(NXB_RECALL);	// (j.armen 2012-02-24 16:30) - PLID 48303
			m_btnRecall.ShowWindow(SW_SHOWNA);
		}
		else
		{
			m_btnRecall.ShowWindow(SW_HIDE);
		}

		m_btnEditSecurityGroups.AutoSet(NXB_LOCK); // (b.savon 2012-03-09 17:07) - PLID 48792 - Lock icon button
		m_btnSendToThirdParty.AutoSet(NXB_SENDTO); // (b.savon 2012-03-09 17:16) - PLID 48794 - SendTo icon button

		IRowSettingsPtr pRow;
		_variant_t var;

		if(IsReproductive()) {
			GetDlgItem(IDC_MARRIAGE_OTHER_BOX)->ShowWindow(SW_HIDE);		
			m_PartnerCombo = BindNxDataListCtrl(IDC_PARTNER_LIST,true);

			pRow = m_PartnerCombo->GetRow(-1);
			var = (long)0;
			pRow->PutValue(0,var);
			pRow->PutValue(1,"<No Partner>");
			pRow->PutValue(4,"<No Partner>");
			m_PartnerCombo->InsertRow(pRow,0);

			SetDlgItemText(IDC_PARTNER_LABEL,"Partner");
			CRect rc;		
			GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);

			SetDlgItemText(IDC_IS_PATIENT_PROSPECT,"Partner");
			GetDlgItem(IDC_IS_PATIENT_PROSPECT)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);
		}
		else {
			GetDlgItem(IDC_PARTNER_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PARTNER_BTN)->ShowWindow(SW_HIDE);
		}

		//DRT 6/18/2004 - PLID 13079 - Moved the code to add the
		//	empty row to the requery finished.

		//DRT 8/11/2004 - PLID 13858 - Moved adding empty row to 
		//	OnRequeryFinishedCoordCombo() function

		//populate the Preferred Contact combo
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)0;
		pRow->PutValue(0,var);
		pRow->PutValue(1, _bstr_t(GetPreferredContactTypeStringFromID(0)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)1;
		pRow->PutValue(0,var);
		pRow->PutValue(1, _bstr_t(GetPreferredContactTypeStringFromID(1)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)2;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(2)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)3;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(3)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)4;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(4)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)5;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(5)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);
		var = (long)6;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(6)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(-1);// (a.vengrofski 2010-02-01 13:09) - PLID <34208> - Added for the "text messaging"
		var = (long)7;// (a.vengrofski 2010-02-01 13:09) - PLID <34208> 
		pRow->PutValue(0,var);// (a.vengrofski 2010-02-01 13:09) - PLID <34208> 
		pRow->PutValue(1, _bstr_t(GetPreferredContactTypeStringFromID(7)));// (a.vengrofski 2010-02-01 13:09) - PLID <34208> 
		m_PreferredContactCombo->AddRow(pRow);// (a.vengrofski 2010-02-01 13:09) - PLID <34208>

		m_InformPath = GetRemotePropertyText ("InformDataPath", "", 0, "<None>");

		//setup gender combo
		IColumnSettingsPtr(m_GenderCombo->GetColumn(m_GenderCombo->InsertColumn(0, _T("Gender"), _T("Gender"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("Male");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("Female");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);

		//TES 2/9/2004: The Note is now ntext, therefore it supports an unlimited length.  CEdits, however, do not support an 
		//unlimited length, so we will set it to the maximum possible.
		((CNxEdit*)GetDlgItem(IDC_NOTES))->SetLimitText(0x7FFFFFFE);

		

		// (z.manning 2008-07-11 10:10) - PLID 30678 - Added TextMessage
		// (z.manning 2009-07-09 10:39) - PLID 27251 - Added DeclinedEmail
		// (a.walling 2013-12-12 16:51) - PLID 59998 - Added country
		m_sql = "SELECT [Last] + ', ' + [First] AS FullName, PatientsT.PersonID, PatientsT.UserDefinedID AS ID, PatientsT.EmployeeID, "
				"PatientsT.MainPhysician, PersonT.PrivWork, PersonT.PrivHome, PersonT.PrivCell, PersonT.Fax AS FaxNumber, PersonT.Gender, "
				"PatientsT.MaritalStatus, '' AS [Foreign], PersonT.Archived, PersonT.PrefixID AS [Patient Prefix], "
				"PatientsT.CurrentStatus, PersonT.FirstContactDate AS [First Contact Date], "
				"CASE WHEN (ReferralSourceT.Name Is Null) Then '' ELSE ReferralSourceT.Name END AS ReferralName, "
				"PersonT.EmergWPhone AS EmergCnctWPhone, PersonT.EmergHPhone AS EmergCnctHPhone, "
				"PersonT.EmergRelation AS EmergContactRel, PersonT.Note AS [Memo], PersonT.Company, PersonT.Title, "
				"PersonT.First AS [First Name], PersonT.Middle AS [Middle Name], PersonT.Last AS [Last Name], "
				"PersonT.Address1 AS [Address 1], PersonT.Address2 AS [Address 2], PersonT.City, PersonT.State AS StateProv, "
				"PersonT.Zip AS PostalCode, PatientsT.SpouseName, PersonT.EmergFirst AS EmergCnctFName, "
				"PersonT.EmergLast AS EmergCnctLName, PersonT.HomePhone AS [Home Phone], PersonT.WorkPhone AS [Work Phone], "
				"PersonT.Extension, PersonT.CellPhone AS [Cell Phone], PersonT.Pager, PersonT.OtherPhone AS [Other Phone], "
				"PersonT.Email AS EmailAddress, PatientsT.Nickname, PersonT.SocialSecurity AS [SS #], "
				"PersonT.BirthDate AS [Birth Date], PatientsT.ImageIndex, PatientsT.InformID, PatientsT.MirrorID, "
				"PatientsT.UnitedID, PatientsT.SentToQuickbooks, PatientsT.PreferredContact, PersonT.ExcludeFromMailings, PersonT.PrefixID, "
				"PersonT.PrivFax, PersonT.PrivOther, PersonT.PrivPager, PersonT.PrivEmail, UsersT.Username, PersonT.TextMessage, "
				"PatientsT.DeclinedEmail, PersonT.Country "
				"FROM (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) "
				"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				"LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID ";

		// (a.walling 2010-10-12 14:54) - PLID 40908 - Don't load the id until UpdateView is called
		/*
		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);
		*/
		m_loading = false;
		try 
		{
			//DRT 8/11/2004 - PLID 13858 - This was looping 4 times and doing 4 database reads.  I changed it to 1 database read (to get the same
			//	4 lines), then loop through the 4 possible records.
			_RecordsetPtr rsCustom;
			CString sql;
			sql.Format("SELECT ID, Name FROM CustomFieldsT Where ID = 1 OR ID = 2 OR ID = 3 OR ID = 4");
			rsCustom = CreateRecordsetStd(sql);
			while(!rsCustom->eof) {
				long nID = AdoFldLong(rsCustom, "ID", -1);
				switch(nID) {
					case 1:	SetDlgItemText(IDC_CUSTOM1_LABEL, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
					case 2: SetDlgItemText(IDC_CUSTOM2_LABEL, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
					case 3: SetDlgItemText(IDC_CUSTOM3_LABEL, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
					case 4: SetDlgItemText(IDC_CUSTOM4_LABEL, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
					default:	ASSERT(FALSE);	//shouldn't happen
				}
				rsCustom->MoveNext();
			}
		} NxCatchAll("Error in opening custom fields: General1Dlg::OnInitDialog.");
		m_changed = false;

		//DRT 7/23/02 - for fax combo
		if(!IsNexTechInternal()) {
			GetDlgItem(IDC_FAX_CHOICE)->ShowWindow(SW_HIDE);
		}

		// Disable controls if we don't have security access
		SecureControls();


		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		((CNxEdit*)GetDlgItem(IDC_EXT_PHONE_BOX))->SetLimitText(10);

		// (a.wetta 2007-03-19 16:36) - PLID 24983 - Determine if a card swiper is on this computer and if the user has permission to change 
		// patient records, if so tell them about the card swiper
		// (a.wetta 2007-07-05 09:08) - PLID 26547 - Check for the existence of the swiper using the DoesOPOSMSRDeviceExist function
		if (GetMainFrame()->DoesOPOSMSRDeviceExist() && CheckCurrentUserPermissions(bioPatient, SPT___W_______, FALSE, 0, TRUE)) {
			// A MSR device is on and working, let's let the user know they can use it to input patient information
			CString strMsg = "Practice has detected a magnetic strip reader (card swiper) attached to this computer.  You can swipe a driver's license at\n"
							"anytime and the information from that license will fill the patient demographic fields in the Patient's General 1 tab.\n\n"
							"NOTE: The information is read off of a driver's license using the AAMVA DL/ID Card Design Specifications Ver 2.0, the national\n"
							"standard for storing information on a driver's license in the USA.  Some states and other countries may not follow these\n"
							"specifications, so not all driver's licenses will work.  Additionally, older driver's licenses may use an older standard which\n"
							"will also not work.";
			DontShowMeAgain(this, strMsg, "MSR_Patient_General1", "Magnetic Strip Reader Detected");
		}


		//
		// v.arth 2009-05-29 PLID 34386
		//
		// Bind the countries drop down list to the database
		m_pCountryList = BindNxDataList2Ctrl(IDC_COUNTRY_LIST, true);

		// (j.gruber 2009-10-06 17:02) - PLID 35825 - set the new member variable
		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");	

		//(c.copits 2011-09-15) PLID 28544 - Reject invalid email addresses
		m_nxeEmail.SetLimitText(50);

		//(c.copits 2011-09-22) PLID 45632 - General 1 email validation lets through invalid addresses.
		m_bDeclinedEmail = false;
		m_bDeclinedEmailWasBlank = false;

		//If we're in Azure, hide the 'E-mail' button, and instead use a label.
		if (g_pLicense->GetAzureRemoteApp()) {
			GetDlgItem(IDC_EMAIL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_G1_EMAIL_LABEL)->ShowWindow(SW_SHOW);
		}
		else {
			//No change, the 'E-mail' label is set to visible = false by default.
		}

	}NxCatchAll("Error in CGeneral1Dlg::OnInitDialog");

	return TRUE;
}

void CGeneral1Dlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages
		CheckFocus();

		StoreDetails();

		// (j.gruber 2009-10-06 17:02) - PLID 35825 - reset here in case they cahnged the preference
		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
		} else {
			ChangeZOrder(IDC_ZIP_BOX, IDC_ADDRESS2_BOX);
		}

		// (a.walling 2010-10-12 14:43) - PLID 40908
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);
		
		//(s.dhole 8/28/2014 1:44 PM ) - PLID 62747
		SetReminderLabel();
		// (a.walling 2010-10-12 15:00) - PLID 40908 - Tablechecker-based refreshes moved to their own function
		HandleTableCheckers();

		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			// (a.walling 2013-12-12 16:51) - PLID 59999 - Recalls being reloaded / queries constantly for recall button color
			Load();			
			m_btnRecall.SetTextColor(RecallUtils::GeneratePatientRecallStatusTextColor(m_id));
		}
		m_ForceRefresh = false;
		m_loading = false;

		//(a.wilson 2012-3-5) PLID 48485 - check to see what the current patients 
		//recall status is, then update the recall buttons text color.
		UpdateWindow();

	} NxCatchAll(__FUNCTION__);
}

void CGeneral1Dlg::StoreDetails ()
{
	try
	{
		// (a.walling 2010-10-12 15:03) - PLID 40908 - Skip if -1 ID
		if (m_id == -1) {
			return;
		}

		//TES 6/18/03: We need to compare in date-only fashion.
		//DRT 9/29/03 - 1)  If it was set, and is now clear, save.  2)  If new date is <> old date, save.  3)  If was clear, and now not, save.
		if((m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 3) || ((long)m_nxtBirthDate->GetDateTime() != (long)m_dtBirthDate.m_dt) || (!m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 1)) {
			m_changed = true;
			Save(IDC_BIRTH_DATE_BOX);
		}
		if((long)m_nxtFirstContact->GetDateTime() != (long)m_dtFirstContact.m_dt) {
			m_changed = true;
			Save(IDC_CONTACT_DATE);
		}
		else if (m_changed)
		{	
			CWnd* pWnd = GetFocus();
			if(pWnd) {
				Save (pWnd->GetDlgCtrlID());
			}
			m_changed = false;
		}
	}NxCatchAll("Error in StoreDetails");
}

void CGeneral1Dlg::SetColor(OLE_COLOR nNewColor)
{
	m_customBkg.SetColor(nNewColor);
	m_emergBkg.SetColor(nNewColor);
	m_phoneBkg.SetColor(nNewColor);
	m_referralBkg.SetColor(nNewColor);
	m_demBkg.SetColor(nNewColor);
	
	m_nxlabelDeclinedEmail.SetColor(nNewColor);
	m_nxlabelSetEmailDeclined.SetColor(nNewColor);
	m_nxlabelSetCompanyLink.SetColor(nNewColor); // (s.dhole 2010-03-26 16:52) - PLID 37796 For internal only, make the company show up in G1.

	CPatientDialog::SetColor(nNewColor);
}

// (r.gonet 2010-08-30 16:36) - PLID 39939 - Set the color of the Groups button based on patient's group membership
void CGeneral1Dlg::UpdateGroupsButton()
{
	// Get the current patient
	long nPatientID = GetActivePatientID();
	if(nPatientID <= 0) {
		// There is no active patient
		return;
	}

	// Check if the current patient is a member of any groups
	// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
	BOOL bHasMembership = ReturnsRecordsParam(GetRemoteDataSnapshot(),
		"SELECT * FROM GroupDetailsT WHERE PersonID = {INT}",
		nPatientID);

	if(bHasMembership) {
		// In the case where the patient is a member in at least one group, change the Groups button color to red.
		(SafeGetDlgItem<CNexTechIconButton>(IDC_GROUPS))->SetTextColor(RGB(255, 0, 0));
	} else {
		// In the case where the patient is not a member in any group, leave the Groups button its default color of black.
		(SafeGetDlgItem<CNexTechIconButton>(IDC_GROUPS))->SetTextColor(RGB(0, 0, 0));
	}
}

void CGeneral1Dlg::OnForeignCheck() 
{
	if (m_loading) return;
	//TODO: Bring back once it is in the PatientsT table
	//EnsureRecordset();
	//m_rs->Fields->Item["Foreign"]->Value = (_variant_t)(long)m_ForeignCheck.GetCheck();
	// UPDATE THE PALM RECORD
	UpdatePalm();			
}

/* DRT - 10/22/01 - Why are there 2 on click female radio buttons?  
void CGeneral1Dlg::OnClickFemaleRad() 
{
	if (m_loading) return;
	try {
		ExecuteSql("UPDATE PersonT SET Gender = 2 WHERE ID = %li",m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in Updating Gender");
}
*/
void CGeneral1Dlg::OnInactiveCheck() 
{
	if (m_loading) return;
	try {
		//DRT 1/11/2007 - PLID 24209 - For internal only, it has been requested that this follow the "Bought" permission
		if(IsNexTechInternal()) {
			if(!CheckCurrentUserPermissions(bioSupportBought, sptWrite)) {
				//reset the check
				m_InactiveCheck.SetCheck( m_InactiveCheck.GetCheck() == 0 ? 1 : 0 );
				return;
			}
		}
		
		//here's the logic here, we have a smart message box that knows whether they will be able to see this patient after this goes into effect or not
		if (m_InactiveCheck.GetCheck()) {

			//they are checking the box, not unchecking it
			if (!(GetMainFrame()->m_patToolBar.m_butAll.GetCheck())) {

				//they have the Active checkbox checked
				//if they have the box checked and the active filter is checked then tell them they won't be able to see the thing
				if (IDYES == MessageBox("You are marking this patient inactive, are you sure you want to do this?\nYou will not be able to see this patient again until you select All in the View filter, but no data will be lost.", "NexTech", MB_YESNO)) {
						SetInactiveCheck();
				}
				else {
					//set the checkbox back
					m_InactiveCheck.SetCheck(0);
				}
			}
			else {

				//the patient is not going to disappear because they have all checked
				if (IDYES == MessageBox("You are marking this patient inactive, are you sure you want to do this?", "NexTech", MB_YESNO)) {
						SetInactiveCheck();
				}
				else {
					//set the checkbox back
					m_InactiveCheck.SetCheck(0);
				}
			}
		}
		else {

			//they are unchecking the box, so go ahead and just let them do it without a message box
			SetInactiveCheck();
		}
	} NxCatchAll("Error in Updating Archived Status");
}

void CGeneral1Dlg::SetInactiveCheck()  {

	try {

		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET Archived = {INT} WHERE ID = {INT}",(long)m_InactiveCheck.GetCheck(),m_id);
		//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
		CClient::RefreshPatCombo(m_id, false, m_InactiveCheck.GetCheck()?CClient::pcatInactive:CClient::pcatActive, CClient::pcstUnchanged);
		// UPDATE THE PALM RECORD
		UpdatePalm();
	
		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientStatus, m_id, m_InactiveCheck.GetCheck() == 1 ? "Active" : "Inactive", m_InactiveCheck.GetCheck() == 1 ? "Inactive" : "Active", aepMedium, aetChanged);
		if(!(GetMainFrame()->m_patToolBar.m_butAll.GetCheck()) && m_InactiveCheck.GetCheck())
		{
			//TES 1/6/2010 - PLID 36761 - The datalist is protected now, we just need to call this function.
			GetMainFrame()->m_patToolBar.RemoveCurrentRow();
			//this will pull up any patient warnings
			// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
			if(GetMainFrame()->GetActiveView()) {
				GetMainFrame()->GetActiveView()->UpdateView();
			}
		}

	}NxCatchAll("Error in SetInActiveCheck()");
}

void CGeneral1Dlg::OnHomePrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivHome = {INT} WHERE ID = {INT}",(long)m_HomePrivCheck.GetCheck(),m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivHome, m_id, m_HomePrivCheck.GetCheck() ? "No" : "Yes", m_HomePrivCheck.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnWorkPrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivWork = {INT} WHERE ID = {INT}",(long)m_WorkPrivCheck.GetCheck(),m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivWork, m_id, m_WorkPrivCheck.GetCheck() ? "No" : "Yes", m_WorkPrivCheck.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnCellPrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivCell = {INT} WHERE ID = {INT}",(long)m_CellPrivCheck.GetCheck(),m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivCell, m_id, m_CellPrivCheck.GetCheck() ? "No" : "Yes", m_CellPrivCheck.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnPullUpPrefixCombo(long iNewRow) 
{
	//DRT 5/29/03 - Moved to OnSelChosenPrefix
}

// v.arth 2009-06-01 PLID 34386 - If the user has permission, save the
//                                selected country to the PersonT table.
void CGeneral1Dlg::SelChosenCountryList(LPDISPATCH lpRow)
{
	if (m_loading)
	{
		return;
	}

	try
	{
		// If the person is trying to save, but they do not have the permission
		// to do so, change the value of the field to be what it was before the
		// user changed it.
		if (!CheckCurrentUserPermissions(bioPatientAddress, SPT___W_______))
		{
			// Get the previously stored value from the database
			_RecordsetPtr recordsetPtr = CreateParamRecordset("SELECT Country FROM PersonT WHERE ID = {INT}", m_id);
			// If nothing was found
			if (recordsetPtr->eof)
			{
				// Default to an empty selection
				m_pCountryList->CurSel = m_pCountryList->SetSelByColumn(0, -1);
			}
			// Something was found
			else
			{
				//  Change the selection to be the previously saved selection
				_variant_t varCountryName = recordsetPtr->Fields->Item["Country"]->Value;

				m_pCountryList->SetSelByColumn(1, varCountryName);
			}
			return;
		}

		// (j.armen 2012-06-07 13:08) - PLID 50825 - Refactored. Update HL7
		// Get the currently selected country
		// (j.gruber 2010-04-14 16:13) - PLID 37067 - need to check that the row exists
		// (a.walling 2009-06-05 11:04) - PLID 34464 - Save NULL rather than a blank string
		// (d.thompson 2012-11-20) - PLID 53255 - Despite the note above say to save NULL instead of a blank string...
		//	if you choose the blank row, it still tries to save a blank string, then fails with an exception.  If the country
		//	is blank, we must save NULL.
		NXDATALIST2Lib::IRowSettingsPtr pCurSel(lpRow);
		_variant_t vtNewValue = g_cvarNull;
		if(pCurSel != NULL && VarString(pCurSel->GetValue(1), "") != "") {
			vtNewValue = pCurSel->GetValue(1);
		}

		// Get the previously saved country and update
		_RecordsetPtr prs = CreateParamRecordset(
			"UPDATE PersonT SET Country = {VT_BSTR} OUTPUT deleted.Country WHERE ID = {INT}",
			vtNewValue, m_id);

		if(!prs->eof) {
			_variant_t vtOldValue = prs->Fields->Item["Country"]->Value;
			if(vtOldValue != vtNewValue) {
				// Audit the country being changed
				long auditId = BeginNewAuditEvent();
				AuditEvent(m_id, m_strPatientName, BeginNewAuditEvent(), aeiPatientCountry, m_id, 
					VarString(vtOldValue, ""), VarString(vtNewValue, ""), aepMedium, aetChanged);
				UpdateHL7Data();
			}
		}
	}
	NxCatchAll("Error occured in CGeneral1Dlg::SelChosenCountryList");
}

void CGeneral1Dlg::OnPullUpDoctorCombo(long nRowIndex) 
{
	if (m_loading) return;
	
	try {
	
		if(!CheckCurrentUserPermissions(bioPatientProvider, sptWrite)) {
			//we have to set it back
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rsProv = CreateParamRecordset("SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT}", m_id);
			if (rsProv->eof) {
				m_DoctorCombo->CurSel = 0;
			}
			else {
				_variant_t varMainPhysician = rsProv->Fields->Item["MainPhysician"]->Value;
				if(varMainPhysician.vt == VT_I4)
					m_DoctorCombo->SetSelByColumn(0, VarLong(varMainPhysician));
				else
					m_DoctorCombo->CurSel = -1;
			}
			return;
		}

		if(nRowIndex != -1) {
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT})", m_id);
			CString strOld;
			if(!rs->eof && rs->Fields->Item["Name"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Name"]->Value.bstrVal);
			//

			long nNewID = -1;
			if(m_DoctorCombo->GetValue(nRowIndex, 0).vt == VT_I4)
				nNewID = VarLong(m_DoctorCombo->GetValue(nRowIndex,0));
			if(nNewID > 0) {
				//DRT 11/14/2008 - PLID 32036 - Parameterized
				ExecuteParamSql("UPDATE PatientsT SET MainPhysician = {INT} WHERE PersonID = {INT}",nNewID,m_id);
			}
			else {
				//DRT 11/14/2008 - PLID 32036 - Parameterized
				ExecuteParamSql("UPDATE PatientsT SET MainPhysician = NULL WHERE PersonID = {INT}",m_id);
			}

			//audit it
			CString strNew;
			if(m_DoctorCombo->GetValue(m_DoctorCombo->CurSel, 4).vt == VT_BSTR)
				strNew = VarString(m_DoctorCombo->GetValue(m_DoctorCombo->CurSel, 4));

			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientProvider, m_id, strOld, strNew, aepMedium, aetChanged);
			//

			// UPDATE THE PALM RECORD
			UpdatePalm();

			//TES 9/20/2010 - PLID 40588 - We include the provider in HL7 messages now.
			UpdateHL7Data();
		}
	} NxCatchAll("Error in changing doctor. General1Dlg::OnPullUpDoctorCombo");
}

void CGeneral1Dlg::OnPullUpCoordCombo(long nRowIndex) 
{
	if (m_loading) return;
	long nAuditTransactionID =-1;
	try {

		//we have to check here to see if they have the write password permission
		if (! CheckCurrentUserPermissions(bioPatientCoordinatorGen1, SPT___W_______)) {

			//we have to set it back
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rsCoordID = CreateParamRecordset("SELECT ID FROM PersonT WHERE ID = (SELECT EmployeeID FROM PatientsT WHERE PersonID = {INT})", m_id);
			if (rsCoordID->eof) {
				m_CoordCombo->CurSel = 0;
			}
			else {
				long nCoordID = AdoFldLong(rsCoordID, "ID");
				m_CoordCombo->SetSelByColumn(0, (long) nCoordID);
			}
			return;
		}
		
		if(nRowIndex != -1) {
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.ID, Last + ', ' + First + ' ' + Middle AS Name, UsersT.Username " 
				"FROM PersonT "
				"INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
				"WHERE ID = (SELECT EmployeeID FROM PatientsT WHERE PersonID = {INT})", m_id);
			CString strOld, strOldUsername;
			long nPrevCoordID = -1;
			if(!rs->eof) {
				nPrevCoordID = AdoFldLong(rs, "ID", -1);
				strOld = AdoFldString(rs, "Name", "");
				strOldUsername = AdoFldString(rs, "Username", "");
			}

			CString PatCoord = "NULL";
			long nPatCoordID = VarLong(m_CoordCombo->GetValue(nRowIndex,0), -1);
			if(nPatCoordID > 0) {
				PatCoord.Format("%li", nPatCoordID);
			}
			ExecuteSql("UPDATE PatientsT SET EmployeeID = %s WHERE PersonID = %li",PatCoord,m_id);

			//audit it
			CString strNew;
			strNew = VarString(m_CoordCombo->GetValue(m_CoordCombo->CurSel, 3), "");

			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientCoord, m_id, strOld, strNew, aepMedium, aetChanged);
			//

			//(e.lally 2007-08-07) PLID 20765 - Re-assign tracking steps to the new patient coordinator if our
				//global preference dictates us to as the user
			if(nPatCoordID > 0){
				//(e.lally 2007-09-19) PLID 20765 - See if the user has write permissions for tracking before attempting this feature
				CPermissions permsTracking = GetCurrentUserPermissions(bioPatientTracking);
				//Check the preference for ladder assignment given to current user or patient coordinator.
				//Check if they even have permission to write to tracking
				if(permsTracking & (sptWrite | sptWriteWithPass)) {
					// (z.manning 2008-07-14 16:23) - PLID 14214 - We used to only even try to do this
					// if the preference to assign steps with no default users was set to patient 
					// coordinator. However, it's now possible to assign individual steps to the patient
					// coordinator so we always check that and then additionally check for unassigned 
					// steps if the preference is set.
					// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step
					CString strUserWhere = FormatString("StepTemplatesT.ID IN (SELECT StepTemplateID FROM StepTemplatesAssignToT WHERE UserID = %li) ", PhaseTracking::tssaPatientCoordinator);
					if(GetRemotePropertyInt("LadderAssignment", 0, 0, "<None>", true) != 0) {
						strUserWhere += " OR StepTemplatesT.ID NOT IN (SELECT StepTemplateID FROM StepTemplatesAssignToT) ";
					}
					//Query if and what user this will change
					//DRT 11/13/2008 - PLID 32036 - Parameterized - The string being added with %s is OK because it
					//	does not use a variable, the enumeration is a constant.
					// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step,
					// to do so means that we should only look at steps assigned to the old patient coordinator userID,
					// that are set up to be the Patient Coordinator by default, and only change those assignments
					// to the new userID for this patient coordinator
					if(nPrevCoordID != -1) {
						_RecordsetPtr rs = CreateParamRecordset(
							FormatString("SELECT Count(*) AS NumSteps "
							"FROM StepsT "
							"	INNER JOIN LaddersT ON StepsT.LadderID = LaddersT.ID "
							"	INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID"							
							"	LEFT JOIN EventAppliesT ON StepsT.ID = EventAppliesT.StepID "
							"WHERE EventAppliesT.ID IS NULL "
							"	AND (%s) "
							"	AND LaddersT.PersonID = {INT} "
							"	AND StepsT.ID IN (SELECT StepID FROM StepsAssignToT WHERE UserID = {INT}) "
							, strUserWhere), m_id, nPrevCoordID);
						if(!rs->eof){
							long nStepCount = AdoFldLong(rs, "NumSteps", 0);
							if(nStepCount > 0) {
								CString strReplacedUser, strMessage;
								strMessage.Format("There are %li unfinished tracking steps that were assigned to '%s' by "
									"default.\n"
									"Would you like to update those steps to be assigned to the new patient coordinator?",
									nStepCount, strOldUsername);			
								if(IDYES == MessageBox(strMessage, NULL, MB_YESNO)){
									//(e.lally 2007-09-19) PLID 20765 - Check permissions before executing
									if(CheckCurrentUserPermissions(bioPatientTracking, sptWrite)){
										//change this patient's ladders to use this new patient coord 
											//when it's not specified in the template and not already complete
										CString strSqlBatch = BeginSqlBatch();
										AddStatementToSqlBatch(strSqlBatch,"DECLARE @nPatCoordID INT \r\n"
											"SET @nPatCoordID = %li \r\n"
											"DECLARE @nPatientID INT \r\n"
											"SET @nPatientID = %li \r\n", nPatCoordID, m_id);

										// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step
										AddStatementToSqlBatch(strSqlBatch, "UPDATE StepsAssignToT SET UserID = @nPatCoordID "
											"FROM StepsAssignToT "
											"	INNER JOIN StepsT ON StepsAssignToT.StepID = StepsT.ID "
											"	LEFT JOIN EventAppliesT ON StepsT.ID = EventAppliesT.StepID "
											"	INNER JOIN LaddersT ON StepsT.LadderID = LaddersT.ID "
											"	INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
											"WHERE EventAppliesT.ID IS NULL "
											"	AND (%s) "
											"	AND LaddersT.PersonID = @nPatientID "
											"	AND StepsAssignToT.UserID = %li", strUserWhere, nPrevCoordID);
											
											//Update the overall owner to be the new patient coord. for ladders that
												//aren't done
										AddStatementToSqlBatch(strSqlBatch,
											"UPDATE LaddersT SET UserID = @nPatCoordID "
											"FROM LaddersT "
											"	INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID "
											"WHERE LaddersT.PersonID = @nPatientID "
											"	AND EXISTS (SELECT UnfinishedStepsT.ID "
											"		FROM StepsT UnfinishedStepsT"
											"		LEFT JOIN EventAppliesT ON UnfinishedStepsT.ID = EventAppliesT.StepID "
											"		WHERE EventAppliesT.ID IS NULL "
											"			AND UnfinishedStepsT.LadderID = StepsT.LadderID); ");

										//(e.lally 2007-09-19) PLID 20765 - Update any active ToDo alarms from these ladders that were created for the default user
										// (c.haag 2008-06-13 11:53) - PLID 30321 - Use the new todo structure. The old logic indiscriminantly assigned all the existing
										// ladder todo's to the new coordinator. We will be consistent, and do the same.
										AddStatementToSqlBatch(strSqlBatch, "DECLARE @tblTask TABLE (ID INT NOT NULL) \r\n "
											"INSERT INTO @tblTask (ID) \r\n"
											"SELECT TaskID FROM ToDoList \r\n"
											"	INNER JOIN StepsT ON ToDoList.RegardingID = StepsT.ID AND ToDoList.RegardingType = 3 \r\n"
											"	LEFT JOIN EventAppliesT ON StepsT.ID = EventAppliesT.StepID \r\n"
											"	INNER JOIN LaddersT ON StepsT.LadderID = LaddersT.ID \r\n"
											"	INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID \r\n"
											"WHERE EventAppliesT.ID IS NULL \r\n"
											"	AND (%s) \r\n"
											"	AND ToDoList.Done IS NULL \r\n"
											"	AND ToDoList.PersonID = @nPatientID; ", strUserWhere);
										AddStatementToSqlBatch(strSqlBatch, "DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT ID FROM @tblTask)");
										AddStatementToSqlBatch(strSqlBatch, "INSERT INTO TodoAssignToT (TaskID, AssignTo) SELECT ID, @nPatCoordID FROM @tblTask");

										if(nAuditTransactionID == -1)
											nAuditTransactionID = BeginAuditTransaction();
										//Prep the audit for the re-assigned ToDos before we commit
										//DRT 11/13/2008 - PLID 32036 - Parameterized - The %s string is OK because there are
										//	no variables in it, only a constant.
										_RecordsetPtr rsToDo = CreateParamRecordset(
											FormatString("SELECT dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS OldUsername, NewUser.Username AS NewUsername, "
											"ToDoList.TaskID "
											"FROM ToDoList "
											"	INNER JOIN StepsT ON ToDoList.RegardingID = StepsT.ID AND ToDoList.RegardingType = 3 "
											"	LEFT JOIN EventAppliesT ON StepsT.ID = EventAppliesT.StepID "
											"	INNER JOIN LaddersT ON StepsT.LadderID = LaddersT.ID "
											"	INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
											"	LEFT JOIN UsersT NewUser ON NewUser.PersonID = {INT} "
											"WHERE EventAppliesT.ID IS NULL "
											"	AND (%s) "
											"	AND ToDoList.Done IS NULL "
											"	AND ToDoList.PersonID = {INT}; ", strUserWhere), nPatCoordID, m_id);
										CString strOld, strNew;
										long nTaskID = -1;
										while(!rsToDo->eof){									
											strOld = "Assigned To: " + AdoFldString(rsToDo, "OldUsername", "");
											strNew = "Assigned To: " + AdoFldString(rsToDo, "NewUsername", "");
											nTaskID = AdoFldLong(rsToDo, "TaskID", -1);
											AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditTransactionID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
											rsToDo->MoveNext();
										}
										rsToDo->Close();

										//(e.lally 2007-09-19) PLID 20765 - Now commit our changes
										ExecuteSqlBatch(strSqlBatch);
										if(nAuditTransactionID != -1)
											CommitAuditTransaction(nAuditTransactionID);

									}
									else{
										//Tell the user we didn't make this change
										MessageBox("No changes to the tracking ladder(s) were made.", NULL, MB_OK);
									}
								}
							}
						}
					}
				}
			}

			// UPDATE THE PALM RECORD
			UpdatePalm();
		}
		return;
	} NxCatchAll("Error in changing patient coordinator. General1Dlg::OnPullUpCoordCombo");
	if(nAuditTransactionID != -1)
		RollbackAuditTransaction(nAuditTransactionID);
}

void CGeneral1Dlg::Save(int nID)
{
	try{
	if (m_loading) return;//don't need this, but better be safe
	CString value, field;
	if (!m_changed)
		return;

	m_changed = false;

	CString strOld, strNew;
	long item = -1;

	// (j.gruber 2007-08-27 10:13) - PLID 24628 - Call the updateHL7Data when needed
	BOOL bUpdateHL7 = FALSE;
	
	switch (nID)
	{	//These are special
		case IDC_ID_BOX:
		{	//long focus = GetFocus()->GetDlgCtrlID();
			long id = GetDlgItemInt(nID);
			CString sql;

			if (!CheckCurrentUserPermissions(bioPatientID, SPT___W_______))
			{
				//if no permission, don't change
				//TES 1/6/2010 - PLID 36761 - New accessor function.
				SetDlgItemInt(nID, GetMainFrame()->m_patToolBar.GetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetPatientIDColumn()));
				m_changed = false;
				return;
			}
			if (id <= 0)
			{
				//if negative or 0, don't change
				//TES 1/6/2010 - PLID 36761 - New accessor function.
				SetDlgItemInt(nID, VarLong(GetMainFrame()->m_patToolBar.GetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetPatientIDColumn())));
				m_changed = false;
				AfxMessageBox("Patient ID must be greater than 0");
				return;
			}
			if (id >= 2147483646){
				//We can't handle 2^31 or higher, so don't change
				//TES 1/6/2010 - PLID 36761 - New accessor function.
				SetDlgItemInt(nID, VarLong(GetMainFrame()->m_patToolBar.GetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetPatientIDColumn())));
				m_changed = false;
				AfxMessageBox("Practice cannot store Patient IDs greater than 2,147,483,645.\nPlease enter a smaller Patient ID number.");
				GetDlgItem(IDC_ID_BOX)->SetFocus();
				return;
			}
			try
			{
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT UserDefinedID FROM PatientsT WHERE UserDefinedID = {INT} AND PersonID <> {INT}",id, m_id);
				if(!rs->eof) {
					//if duplicate id, don't change
					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs2 = CreateParamRecordset("SELECT UserDefinedID AS ID FROM PatientsT WHERE PersonID = {INT}",m_id);
					SetDlgItemInt (IDC_ID_BOX, rs2->Fields->GetItem("ID")->Value.lVal);
					m_changed = false;
					rs2->Close();
					//m_changed = false;
					AfxMessageBox("Another patient has this ID.\nTwo patients cannot have the same ID.");
				}
				else {
					//get the old ID, for auditing
					long nOldID;
					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT UserDefinedID FROM PatientsT WHERE PersonID = {INT}", m_id);
					nOldID = AdoFldLong(rs, "UserDefinedID");

					//(e.lally 2006-04-12) PLID 20074 - Users should be asked if they are sure they want to change the ID of the patient.
					if(IDNO == MessageBox("Are you absolutely sure you want to change the ID number for this patient?",NULL, MB_YESNO)){
						//Set it back to the old ID. We don't have to run a query since we just got it for auditing.
						SetDlgItemInt (IDC_ID_BOX, nOldID);
						m_changed = false;
						return;
					}

					// (a.walling 2010-05-17 13:08) - PLID 34056 - Check the history folder first

					CString strNew;
					strNew.Format("%li", id);	//new id

					bool bHistoryOK = EnsureCorrectHistoryFolder(this, eChangedHFID, strNew, m_id, true);

					if (!bHistoryOK) {
						//Undo by setting back to the "old" text
						SetDlgItemInt (IDC_ID_BOX, nOldID);
						m_changed = false;
						return;
					}

					//NOW we can save it
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					ExecuteParamSql("UPDATE PatientsT SET UserDefinedID = {INT} WHERE PersonID = {INT}",id,m_id);
					
					// (z.manning 2008-11-12 11:00) - PLID 31129 - No need to update the patient toolbar here
					// as the table checker will handle that.

					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();

					//for auditing
					if(nOldID != id) {
						CString strOld;
						strOld.Format("%li", nOldID);

						long nAuditID = -1;
						nAuditID = BeginNewAuditEvent();
						if(nAuditID != -1) 
							AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientUserID, m_id, strOld, strNew, aepMedium, aetChanged);
					}

					// UPDATE THE PALM RECORD
					UpdatePalm();
				}
				rs->Close();
				CClient::RefreshTable(NetUtils::PatCombo, m_id);
			}
			catch (_com_error)
			{
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT UserDefinedID AS ID FROM PatientsT WHERE PersonID = {INT}",m_id);
				SetDlgItemInt (IDC_ID_BOX, rs->Fields->GetItem("ID")->Value.lVal);
				m_changed = false;
				rs->Close();
				//m_changed = false;
				AfxMessageBox("Another patient has this ID.\nTwo patients cannot have the same ID.");
//				if (GetDlgItem(focus)->GetSafeHwnd())
//					GetDlgItem(focus)->SetFocus();
				return;
			}
			//_DNxDataListPtr pCombo = GetMainFrame()->m_patToolBar.m_toolBarCombo;
			//pCombo->Requery();
			//pCombo->SetSelByColumn(0,(COleVariant)id);
//			if (GetDlgItem(focus)->GetSafeHwnd())
//				GetDlgItem(focus)->SetFocus();
			//m_changed = false;
			return;
		}
		case IDC_ZIP_BOX://May need further optimization, as it can save the city and state when they haven't changed
		{

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientAddress, SPT___W_______)) {
				SetDlgItemText(IDC_ZIP_BOX, m_strZip);
				m_changed = false;
				return;
			}
			// (j.gruber 2009-10-06 16:38) - PLID 35825 - check if they are looking up by city
			if (!m_bLookupByCity) {
				//for auditing
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				//(e.lally 2009-09-15) PLID 21585 - Audit changes to city and state
				_RecordsetPtr rs = CreateParamRecordset("SELECT City, State, Zip FROM PersonT WHERE ID = {INT}", m_id);
				CString strOldCity, strOldState, strOldZip;
				if(!rs->eof){
					strOldCity = AdoFldString(rs, "City", "");
					strOldState = AdoFldString(rs, "State", "");
					strOldZip = AdoFldString(rs, "Zip", "");
				}
				rs->Close();
				//

				CString strNewCity,
						strNewState,
						strNewZip,
						strTempZip,
						tempCity,
						tempState,
						//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
						strOrigZip;
				GetDlgItemText(IDC_ZIP_BOX, strNewZip);
				GetDlgItemText(IDC_CITY_BOX, tempCity);
				GetDlgItemText(IDC_STATE_BOX, tempState);
				//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
				GetDlgItemText(IDC_ZIP_BOX, strOrigZip);
				tempCity.TrimRight();
				tempState.TrimRight();
				strNewZip.TrimRight();
				// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
				if(!tempCity.IsEmpty() || !tempState.IsEmpty()) {
					//(e.lally 2009-09-15) PLID 21585 - Check if nothing actually changed first
					//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
					// Must allow trailing spaces per preference now.
					//if(strNewZip == strOldZip){
					if(strOrigZip == strOldZip){
						return;
					}
					MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
					//(a.wilson 2012-1-19) PLID 47485 - added to avoid messagebox when scanning or swiping card.
					if (!m_bProcessingCard){
						if(AfxMessageBox("You have changed the postal code but the city or state already have data in them.  Would you like to overwrite "
							"this data with that of the new postal code?", MB_YESNO) == IDYES)
						{
							//Just treat them as empty and the code below will fill them.
							tempCity.Empty();
							tempState.Empty();
						}
					}
				}
				if(tempCity == "" || tempState == "") {
					GetZipInfo(strNewZip, &strNewCity, &strNewState);

					// (s.tullis 2013-10-07 16:42) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
					if(strNewCity == "" && strNewState == ""){
						strTempZip = strNewZip.Left(5);// Get the 5 digit zip code
						GetZipInfo(strTempZip, &strNewCity, &strNewState);
						// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
						//digit zipcode in the locations tab of Administrator, it looks
						//up the city and state based off the 5 digit code, and then 
						//changes the zip code to 5 digits. It should not change the zip code.								
					}
					if(tempCity == "") 
						SetDlgItemText(IDC_CITY_BOX, strNewCity);
					else strNewCity = tempCity;
					if(tempState == "")
						SetDlgItemText(IDC_STATE_BOX, strNewState);
					else strNewState = tempState;

					//(e.lally 2009-09-15) PLID 21585 - Check if nothing actually changed
					//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
					// Must allow trailing spaces per preference now.
					//if(strNewZip == strOldZip && strNewCity == strOldCity && strNewState == strOldState){	
					if(strOrigZip == strOldZip && strNewCity == strOldCity && strNewState == strOldState){	
						return;
					}
					//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
					if (bTruncateTrailingNameAddressSpace) {
						strOrigZip.TrimRight();
						SetDlgItemText(IDC_ZIP_BOX, strOrigZip);
						strNewZip = strOrigZip;
					}
					else {
						strNewZip = strOrigZip;
					}
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					ExecuteParamSql("UPDATE PersonT SET Zip = {STRING}, City = {STRING}, State = {STRING} WHERE ID = {INT}", strNewZip, strNewCity, strNewState, m_id);
					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();
				}
				else {
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					//(e.lally 2009-09-15) PLID 21585 - Check if nothing actually changed
					//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
					// Must allow trailing spaces per preference now.
					//if(strNewZip == strOldZip){
					if (strOrigZip == strOldZip) {
						return;
					}
					//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
					if (bTruncateTrailingNameAddressSpace) {
						strOrigZip.TrimRight();
						SetDlgItemText(IDC_ZIP_BOX, strOrigZip);
						strNewZip = strOrigZip;
					}
					else {
						strNewZip = strOrigZip;
					}
					ExecuteParamSql("UPDATE PersonT SET Zip = {STRING} WHERE ID = {INT}", strNewZip, m_id);
					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();
				}

				// UPDATE THE PALM RECORD
				UpdatePalm();

				//auditing
				//(e.lally 2009-09-15) PLID 21585 - Audit city and state if they changed
				CAuditTransaction audit;

				if(strOldZip != strNewZip) {
					AuditEvent(m_id, m_strPatientName, audit, aeiPatientZip, m_id, strOldZip, strNewZip, aepMedium, aetChanged);
				}
				if(strOldCity != strNewCity) {
					AuditEvent(m_id, m_strPatientName, audit, aeiPatientCity, m_id, strOldCity, strNewCity, aepMedium, aetChanged);
				}
				if(strOldState != strNewState) {
					AuditEvent(m_id, m_strPatientName, audit, aeiPatientState, m_id, strOldState, strNewState, aepMedium, aetChanged);
				}

				audit.Commit();
				//
				//m_changed = false;
				return;
			}
			else {
				field = "Zip";
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT Zip FROM PersonT WHERE ID = {INT}", m_id);
				item = aeiPatientZip;
				if(!rs->eof && rs->Fields->Item["Zip"]->Value.vt == VT_BSTR) {
					strOld = CString(rs->Fields->Item["Zip"]->Value.bstrVal);
				}

				// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
				bUpdateHL7 = TRUE;
			}
		
		}
		break;
		case IDC_BIRTH_DATE_BOX:
		{	//long focus = GetFocus()->GetDlgCtrlID();
			EnsureCorrectAge();

			CString strOldBirthDate = "", strNewBirthDate = "";
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT BirthDate FROM PersonT WHERE ID = {INT}",m_id);
			//for auditing
			if(!rs->eof) {
				_variant_t var = rs->Fields->Item["BirthDate"]->Value;
				if(var.vt == VT_DATE)
					strOldBirthDate = FormatDateTimeForInterface(COleDateTime(var), dtoDate);
			}
			rs->Close();

			// (j.jones 2007-08-07 14:15) - PLID 27002 - reworked the way we update EMRMasterT.PatientAge
			BOOL bUpdatedBirthDate = FALSE;

			value = FormatDateTimeForSql(m_nxtBirthDate->GetDateTime());
			if (m_nxtBirthDate->GetStatus() == 3) {
				//DRT 11/14/2008 - PLID 32036 - Parameterized
				ExecuteParamSql("UPDATE PersonT SET BirthDate = NULL WHERE ID = {INT}",m_id);
				
				// (b.cardillo 2009-05-28 14:55) - PLID 34369 - This patient's birthdate just changed, so update the qualification records.
				UpdatePatientWellnessQualification_Age(GetRemoteData(), m_id);

				//TES 11/15/2013 - PLID 59533 - Check CDS rules
				CDWordArray arNewCDSInterventions;
				UpdateDecisionRules(GetRemoteData(), m_id, arNewCDSInterventions);
				GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

				strNewBirthDate = "";

				m_dtBirthDate.SetDateTime(0,0,0,0,0,0);
				m_dtBirthDate.m_dt = 0;
				m_dtBirthDate.SetStatus(COleDateTime::invalid);
				m_nxtBirthDate->Clear();
				m_bBirthDateSet = false;

				// (j.jones 2007-08-07 14:15) - PLID 27002 - deal with the EMR records later in this function
				bUpdatedBirthDate = TRUE;

				// JMJ 7-23-2003 - The EMR stores the patient age, which would change if this changes. Ideally 
				//we should have reports accurately calculate the age, and not store it. We will have to decide this at some point.
				// (a.walling 2006-08-17 09:56) - PLID 22071 - Do not update locked EMNs!
				//ExecuteSql("UPDATE EMRMasterT SET PatientAge = NULL WHERE PatientID = %li AND Status <> 2",m_id);

				// (j.jones 2007-08-10 15:24) - PLID 27047 - fixed so we actually update the toolbar
				_variant_t varNull;
				varNull.vt = VT_NULL;
				//TES 1/6/2010 - PLID 36761 - New function to update the datalist.
				GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetBirthDateColumn(), varNull);
				
				// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
				UpdateHL7Data();
			}
			else {
				COleDateTime dt, dttemp;
				dttemp.ParseDateTime("01/01/1800");
				dt = m_nxtBirthDate->GetDateTime();
				if(m_nxtBirthDate->GetStatus() == 1 && dt.m_dt >= dttemp.m_dt) {
					strNewBirthDate = FormatDateTimeForInterface(dt, dtoDate);
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					ExecuteParamSql("UPDATE PersonT SET BirthDate = {STRING} WHERE ID = {INT}",FormatDateTimeForSql(dt, dtoDate),m_id);
					
					// (b.cardillo 2009-05-28 14:55) - PLID 34369 - This patient's birthdate just changed, so update the qualification records.
					UpdatePatientWellnessQualification_Age(GetRemoteData(), m_id);

					//TES 11/15/2013 - PLID 59533 - Check CDS rules
					CDWordArray arNewCDSInterventions;
					UpdateDecisionRules(GetRemoteData(), m_id, arNewCDSInterventions);
					GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

					_variant_t varDate;
					varDate.vt = VT_DATE;
					varDate.date = dt;
					//TES 1/6/2010 - PLID 36761 - New function to update the datalist.
					GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetBirthDateColumn(), varDate);
					
					m_dtBirthDate = dt;
					m_bBirthDateSet = true;

					// (j.jones 2007-08-07 14:15) - PLID 27002 - deal with the EMR records later in this function
					bUpdatedBirthDate = TRUE;

					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();

					/*
					// JMJ 7-23-2003 - The EMR stores the patient age, which would change if this changes. Ideally 
					//we should have reports accurately calculate the age, and not store it. We will have to decide this at some point.
					_RecordsetPtr rsEMR = CreateRecordset("SELECT ID, Date FROM EMRMasterT WHERE Deleted = 0 AND PatientID = %li",m_id);
					while(!rsEMR->eof) {
						COleDateTime dtEMR = AdoFldDateTime(rsEMR, "Date");
						long nEMRAge = GetPatientAge(dt, dtEMR);
						//(e.lally 2006-02-13) PLID 17601 - Do not update locked EMNs.
						ExecuteSql("UPDATE EMRMasterT SET PatientAge = %li WHERE ID = %li AND Deleted = 0 AND Status <> 2",nEMRAge,AdoFldLong(rsEMR, "ID"));
						rsEMR->MoveNext();
					}
					rsEMR->Close();
					*/
				}
				else {
					//It is important that we reset the box before notifying them, because the messagebox will fire
					//OnKillFocus()
					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT BirthDate FROM PersonT WHERE ID = {INT}",m_id);
					if(!rs->eof) {
						_variant_t varBirthDate = rs->Fields->Item["BirthDate"]->Value;
						if(varBirthDate.vt == VT_DATE) {
							m_nxtBirthDate->SetDateTime(VarDateTime(varBirthDate));
						}
						else {
							m_nxtBirthDate->Clear();
						}
						m_changed = false;
					}
					rs->Close();
					EnsureCorrectAge();
					AfxMessageBox("You have entered an invalid date. The date has been reset.");
				}
			}

			// (j.jones 2007-08-07 14:16) - PLID 27002 - if we actually changed the birthdate, see if
			// they have any EMRs, and if so, prompt the user, but only if they have write permissions (don't prompt yet)
			
			BOOL bUpdatedEMN = FALSE;

			// (a.walling 2007-11-28 11:29) - PLID 28044 - Check for expired license too
			if(bUpdatedBirthDate && (CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) ) {

				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rsEMR = CreateParamRecordset("SELECT ID, Date, PatientAge, Description FROM EMRMasterT WHERE Deleted = 0 AND Status <> 2 AND PatientID = {INT}",m_id);

				if(!rsEMR->eof) {

					//the wording of this message is such that we aren't saying we have a new age, just that we will try to recalculate it,
					//because of course not all date changes mean their age changed
					if(IDYES == MessageBox("This patient has unlocked EMN records that calculated the patient's Age using the previous birthdate data.\n\n"
						"Would you like to update all of these EMNs with a recalculated new age for each record?\n"
						"(If not, you can update these records individually by clicking the 'Update Demographics' button in the EMN's 'More Info' topic.)",
						"Practice", MB_ICONQUESTION|MB_YESNO)
						//if they click yes, then really check permissions, meaning only now will they get a password
						&& CheckCurrentUserPermissions(bioPatientEMR, sptWrite)) {

						while(!rsEMR->eof) {
							
							long nEMNID = AdoFldLong(rsEMR, "ID");
							COleDateTime dtEMR = AdoFldDateTime(rsEMR, "Date");							
							CString strOldAge = AdoFldString(rsEMR, "PatientAge", "");
							CString strDescription = AdoFldString(rsEMR, "Description", "");

							CString strNewAge;
							// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation,
							// validation should only be done when bdays are entered/changed
							if(m_dtBirthDate.GetStatus() != COleDateTime::invalid)
								strNewAge = GetPatientAgeOnDate(m_dtBirthDate, dtEMR, TRUE);
							
							if(strOldAge != strNewAge) {

								// (a.walling 2008-06-26 09:26) - PLID 30515 - Ensure we don't touch any EMNs that are currently
								// being modified
								// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
								// (j.armen 2013-05-14 12:29) - PLID 56680 - EMN Access restructuring
								long nAffected = 0;
								ExecuteParamSql(GetRemoteData(), &nAffected, 
									"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
									"UPDATE EMRMasterT\r\n"
									"	SET PatientAge = {STRING}\r\n"
									"WHERE ID = {INT}\r\n"
									"	AND ID NOT IN (SELECT EmnID FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK))",
									strNewAge, nEMNID);

								if (nAffected <= 0) {
									MessageBox(FormatString("Could not update the age on the EMN '%s' on %s: the EMN is currently being modified by another user.", strDescription, FormatDateTimeForInterface(dtEMR)));
								} else {
									bUpdatedEMN = TRUE;

									CString strOld = strOldAge;
									CString strNew = strNewAge;

									//audit the change
									long nAuditID = -1;
									nAuditID = BeginNewAuditEvent();
									if(nAuditID != -1) 
										AuditEvent(m_id, m_strPatientName, nAuditID, aeiEMNPatientAge, nEMNID, strOld, strNew, aepHigh, aetChanged);
								}
							}

							rsEMR->MoveNext();
						}
					}
				}
				rsEMR->Close();
			}

			//DRT 10/20/2003 - PLID 8628 - Send a table checker after changing the bdate
			CClient::RefreshTable(NetUtils::PatientBDate, m_id);

			// (j.jones 2007-08-07 14:43) - PLID 27002 - if we actually changed any EMNs, send a tablechecker for this patient
			if(bUpdatedEMN)
				CClient::RefreshTable(NetUtils::EMRMasterT, m_id);

			if(strOldBirthDate != strNewBirthDate) {
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1) 
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientBirthDate, m_id, strOldBirthDate, strNewBirthDate, aepMedium, aetChanged);
			}

			// UPDATE THE PALM RECORD
			UpdatePalm();
			//GetMainFrame()->m_patToolBar.m_toolBarCombo.RefreshContents();
//			if (GetDlgItem(focus)->GetSafeHwnd())
//				GetDlgItem(focus)->SetFocus();
			//m_changed = false;
			return;
		}
		case IDC_CONTACT_DATE:
		{
			if(!m_bSavingContactDate) {
				m_bSavingContactDate = true;
				COleDateTime dtOld = m_dtFirstContact;
				//(e.lally 2010-05-10) PLID 36631 - As a safety, check the permission, even though we should never be able to get here without it.
				if(!CheckCurrentUserPermissions(bioPatientFirstContactDate, sptWrite)){
					m_nxtFirstContact->SetDateTime(m_dtFirstContact);
					m_bSavingContactDate = false;
					return;
				}
				COleDateTime dt, dttemp;
				dttemp.ParseDateTime("01/01/1800");
				dt = m_nxtFirstContact->GetDateTime();
				value = FormatDateTimeForSql(dt);
				if(m_nxtFirstContact->GetStatus() == 1 && dt.m_dt >= dttemp.m_dt) {
					//check to see if the date is in the future and if it is, then we should warn them about it
					COleDateTime dtNow;
					dtNow = COleDateTime::GetCurrentTime();
					if (dtNow.m_dt < dt.m_dt) {
						if (IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO, "The date you entered is in the future, are you sure you want this date as this patient's First Contact Date? ")) {
							//DRT 11/14/2008 - PLID 32036 - Parameterized
							ExecuteParamSql("UPDATE PersonT SET FirstContactDate = {STRING} WHERE ID = {INT}",FormatDateTimeForSql(dt),m_id);
							m_dtFirstContact = dt;
						}
						else { 
						//they don't want to enter that as the first Contact Date, so don't save it
							//DRT 11/13/2008 - PLID 32036 - Parameterized
							_RecordsetPtr rs = CreateParamRecordset("SELECT FirstContactDate FROM PersonT WHERE ID = {INT}",m_id);
							if(!rs->eof) {
								_variant_t varFirstContact = rs->Fields->Item["FirstContactDate"]->Value;
								if(varFirstContact.vt == VT_DATE) {
									m_dtFirstContact = VarDateTime(varFirstContact);
									m_nxtFirstContact->SetDateTime(m_dtFirstContact);									
								}
								else {
									m_nxtFirstContact->Clear();
								}
		 						m_changed = false;
							}
							rs->Close();
						}
					}
					else {
						//save the date
						//DRT 11/14/2008 - PLID 32036 - Parameterized
						ExecuteParamSql("UPDATE PersonT SET FirstContactDate = {STRING} WHERE ID = {INT}", FormatDateTimeForSql(dt), m_id);
						m_dtFirstContact = dt;						
					}
				}			
				else {
					//It is important that we reset the box before notifying them, because the messagebox will fire
					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT FirstContactDate FROM PersonT WHERE ID = {INT}",m_id);
					if(!rs->eof) {
						_variant_t varFirstContact = rs->Fields->Item["FirstContactDate"]->Value;
						if(varFirstContact.vt == VT_DATE) {
							m_dtFirstContact = VarDateTime(varFirstContact);
							m_nxtFirstContact->SetDateTime(m_dtFirstContact);							
						}
						else {
							m_nxtFirstContact->Clear();
						}
						m_changed = false;
					}
					rs->Close();
					AfxMessageBox("You have entered an invalid date. The date has been reset.");
				}
				// UPDATE THE PALM RECORD
				UpdatePalm();

				//DRT 6/30/2005 - PLID 16654 - Auditing
				if(dtOld != m_dtFirstContact) {
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientFirstContact, m_id, FormatDateTimeForInterface(dtOld, NULL, dtoDate), 
						FormatDateTimeForInterface(m_dtFirstContact, NULL, dtoDate), aepLow, aetChanged);
				}

				//m_changed = false;
				m_bSavingContactDate = false;
			}
			return;
		}
		case IDC_FIRST_NAME_BOX:
		{	//long focus = GetFocus()->GetDlgCtrlID();
			CString last, middle;

			GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);
			GetDlgItemText(IDC_LAST_NAME_BOX, last);
			GetDlgItemText(IDC_FIRST_NAME_BOX, value);
			
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on first name
			// Must now allow trailing spaces per preference
			//check to make sure the name has changed
			/*
			if (value == m_strFirstName){
				//this is incase the user only added spaces
				SetDlgItemText(IDC_FIRST_NAME_BOX, m_strFirstName);
				return;
			}
			*/
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on first name
			CString strFirstName;
			GetDlgItemText(IDC_FIRST_NAME_BOX, strFirstName);
			// If not edited (user just clicked on it), then don't change anything.
			if (m_strFirstName == strFirstName) {
				return;
			}

			if (!UserCanChangeName())
			{
				Load(); // Revert to old name
				GetDlgItem(nID)->SetFocus();
				return;
			}

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on first name
			if (bTruncateTrailingNameAddressSpace) {
				// Just grab the string, trim it, and put it back.
				strFirstName.TrimRight();
				SetDlgItemText(IDC_FIRST_NAME_BOX, strFirstName);
				value = strFirstName;
			}
			else {
				value = strFirstName;
			}

			// (a.walling 2010-05-17 13:08) - PLID 34056 - Check the history folder first

			bool bHistoryOK = EnsureCorrectHistoryFolder(this, eChangedHFFirst, value, m_id, true);

			if (!bHistoryOK) {
				Load(); // Revert to old name
				GetDlgItem(nID)->SetFocus();
				return;
			}

			//get old value for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT [First] FROM PersonT WHERE ID = {INT}", m_id);
			strOld = AdoFldString(rs, "First","");
			
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PersonT SET First = {STRING} WHERE ID = {INT}", value, m_id);

			//m.cable - 3-10-03 - Save the current name for this patient in the member variables
			//for something to compare to
			m_strFirstName = value;
			m_strMiddleName = middle;
			m_strLastName = last;

			//for auditing			
			strNew = value;
			item = aeiPatientPersonFirst;

			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) 
				AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, strNew, aepMedium, aetChanged);

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			UpdateHL7Data();

			
			// UPDATE THE PALM RECORD
			UpdatePalm();

			// (z.manning 2008-11-12 10:29) - PLID 31129 - Updating the toolbar is handled by the table checker
			CClient::RefreshTable(NetUtils::PatCombo, m_id);
//			if (GetDlgItem(focus)->GetSafeHwnd())
//				GetDlgItem(focus)->SetFocus();
			//m_changed = false;
			return;
		}
		case IDC_LAST_NAME_BOX:
		{	
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on last name
			CString middle, first, strOrigLastName;
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);
	
			GetDlgItemText(IDC_FIRST_NAME_BOX, first);
			GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);
			GetDlgItemText(IDC_LAST_NAME_BOX, value);
			value.TrimRight();
			if (value == "")
			{	
				AfxMessageBox ("Patients require a last name.");
				// (a.walling 2010-10-12 14:45) - PLID 40908 - Call Load() like the rest of these places
				//UpdateView();
				Load();
				//m_changed = false;
				return;
			}
			
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on last name
			// Must now allow trailing spaces per preference
			//check to make sure the name has changed
			/*
			if (value == m_strLastName){
				//this is incase the user only added spaces
				SetDlgItemText(IDC_LAST_NAME_BOX, m_strLastName);
				return;
			}
			*/

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on last name
			CString strLastName;
			GetDlgItemText(IDC_LAST_NAME_BOX, strLastName);
			// If not edited (user just clicked on it), then don't change anything.
			if (m_strLastName == strLastName) {
				return;
			}

			if (!UserCanChangeName())
			{
				Load(); // Revert to old name
				GetDlgItem(nID)->SetFocus();
				return;
			}
			//long focus = GetFocus()->GetDlgCtrlID();
			
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on last name
			if (bTruncateTrailingNameAddressSpace) {
				// Just grab the string, trim it, and put it back.
				strLastName.TrimRight();
				SetDlgItemText(IDC_LAST_NAME_BOX, strLastName);
				value = strLastName;
			}
			else {
				value = strLastName;
			}

			// (a.walling 2010-05-17 13:08) - PLID 34056 - Check the history folder first

			bool bHistoryOK = EnsureCorrectHistoryFolder(this, eChangedHFLast, value, m_id, true);

			if (!bHistoryOK) {
				Load(); // Revert to old name
				GetDlgItem(nID)->SetFocus();
				return;
			}

			//get old value for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT [Last] FROM PersonT WHERE ID = {INT}", m_id);
			strOld = AdoFldString(rs, "Last","");
			
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PersonT SET Last = {STRING} WHERE ID = {INT}", value, m_id);

			//m.cable - 3-10-03 - Save the current name for this patient in the member variables
			//for something to compare to
			m_strFirstName = first;
			m_strMiddleName = middle;
			m_strLastName = value;

			//for auditing
			strNew = value;
			item = aeiPatientPersonLast;

			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) 
				AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, strNew, aepMedium, aetChanged);

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			UpdateHL7Data();

			UpdatePalm();

			// (z.manning 2008-11-12 10:29) - PLID 31129 - Updating the toolbar is handled by the table checker
			CClient::RefreshTable(NetUtils::PatCombo, m_id);
//			if (GetDlgItem(focus)->GetSafeHwnd())
//				GetDlgItem(focus)->SetFocus();
			//m_changed = false;
			return;
		}
		//The rest are really just data
		case IDC_SOC_SEC_NO_BOX:
			// (f.dinatale 2010-10-07) - PLID 33753 - They shouldn't be able to save if they can't see the full SSN, but just as a precaution.
			if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0)) {
				GetDlgItemText(IDC_SOC_SEC_NO_BOX,value);
				//(e.lally 2005-06-06) PLID 16365 - Make sure ssn's with value '###-##-####' get reset
				//The OnKillFocus event was getting called after the save so the code to handle this
				//needs to be here.
				if(value == "###-##-####") {
					value="";
					m_bFormattingField = true;
					FormatItemText(GetDlgItem(IDC_SOC_SEC_NO_BOX),value,"");
					m_bFormattingField = false;
				}
				value.Replace("#", " "); //take care of bad characters
				value.TrimRight();
				//if they didn't change anything, don't save it
				if (value != m_strSocialSecurity) {
					CString strTemp = value;
					strTemp.Replace("-", "");
					//check to see that this is the only SSN in the database
					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rsDuplicate = CreateParamRecordset("SELECT ID, First, Last FROM PersonT WHERE (REPLACE(SocialSecurity, '-', '') = {STRING}) AND (ID IN (SELECT PersonID FROM PatientsT)) AND (ID <> {INT}) ", strTemp, m_id);
					if (!rsDuplicate->eof && value != "   -  -" && value != "") {
						//there is a duplicate!! Check to see if they want to do this
						CString strMessage;
						if(rsDuplicate->RecordCount < 5){
							// there is only a handful of patiens with the same SSN so give the name(s)
							CString strFirst, strLast, strNameToAdd;
							strMessage.Format("The following patients have this Social Security Number: \r\n");
							while(!rsDuplicate->eof){
								strFirst = AdoFldString(rsDuplicate->GetFields(), "First", "");
								strLast	 = AdoFldString(rsDuplicate->GetFields(), "Last", "");
								strNameToAdd.Format("%s %s \r\n", strFirst, strLast);
								strMessage += strNameToAdd;
								rsDuplicate->MoveNext();
							}
						}
						else{
							// there are potentially a lot of duplicates so just let the user know how many there are
							strMessage.Format("There are %li other patients that have this Social Security Number.  ", rsDuplicate->RecordCount);
						}

						if (IDNO == MsgBox(MB_YESNO, strMessage + "\r\nAre you sure you wish to save this?", "NexTech")) {
							//set it back
							SetDlgItemText(IDC_SOC_SEC_NO_BOX, m_strSocialSecurity);
							return;
						}	
					}

					//if we got here, we are good to go

					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT SocialSecurity FROM PersonT WHERE ID = {INT}", m_id);
					strOld = AdoFldString(rs, "SocialSecurity","");

					//DRT 11/14/2008 - PLID 32036 - Parameterized
					ExecuteParamSql("UPDATE PersonT SET SocialSecurity = {STRING} WHERE ID = {INT}", value, m_id);
					//TES 1/6/2010 - PLID 36761 - New function to update the datalist.
					GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetSSNColumn(),_bstr_t(value));
					m_strSocialSecurity = value;

					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();

					//for auditing
					strNew = value;
					item = aeiPatientSSN;

					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) 
						AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, strNew, aepMedium, aetChanged);
				}
			}
			return;
			break;
		case IDC_EMERGENCY_RELATE: {
				field = "EmergRelation";
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT EmergRelation FROM PersonT WHERE ID = {INT}", m_id);
				item = aeiPatientEmergRelation;
				if(!rs->eof && rs->Fields->Item["EmergRelation"]->Value.vt == VT_BSTR) {
					strOld = CString(rs->Fields->Item["EmergRelation"]->Value.bstrVal);
				}
			}
			break;
		case IDC_MIDDLE_NAME_BOX: 
		{	CString first, last;

			GetDlgItemText(IDC_LAST_NAME_BOX, last);
			GetDlgItemText(IDC_FIRST_NAME_BOX, first);
			GetDlgItemText(IDC_MIDDLE_NAME_BOX, value);
			
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on middle name
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on middle name
			// Must now allow trailing spaces per preference
			//check to make sure the name has changed
			/*
			if (value == m_strMiddleName){
				//this is incase the user only added spaces
				SetDlgItemText(IDC_MIDDLE_NAME_BOX, m_strMiddleName);
				return;
			}
			*/

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on middle name
			CString strMiddleName;
			GetDlgItemText(IDC_MIDDLE_NAME_BOX, strMiddleName);
			// If not edited (user just clicked on it), then don't change anything.
			if (m_strMiddleName == strMiddleName) {
				return;
			}

			if (!UserCanChangeName())
			{
				Load(); // Revert to old name
				GetDlgItem(nID)->SetFocus();
				return;
			}
			//long focus = GetFocus()->GetDlgCtrlID();
			
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on middle name
			if (bTruncateTrailingNameAddressSpace) {
				strMiddleName.TrimRight();
				SetDlgItemText(IDC_MIDDLE_NAME_BOX, strMiddleName);
				value = strMiddleName;
			}
			else {
				value = strMiddleName;
			}

			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Middle FROM PersonT WHERE ID = {INT}", m_id);
			if(rs->Fields->Item["Middle"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Middle"]->Value.bstrVal);
			strNew = value;
			item = aeiPatientPersonMiddle;

			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) 
				AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, strNew, aepMedium, aetChanged);


			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PersonT SET Middle = {STRING} WHERE ID = {INT}", value, m_id);
			
			//m.cable - 3-10-03 - Save the current name for this patient in the member variables
			//for something to compare to
			m_strFirstName = first;
			m_strMiddleName = value;
			m_strLastName = last;

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			UpdateHL7Data();

			UpdatePalm();

			// (z.manning 2008-11-12 10:29) - PLID 31129 - Updating the toolbar is handled by the table checker

			CClient::RefreshTable(NetUtils::PatCombo, m_id);
//			if (GetDlgItem(focus)->GetSafeHwnd())
//				GetDlgItem(focus)->SetFocus();
			//m_changed = false;
			return;
		}
		case IDC_EMAIL_BOX: {
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientEmail, SPT___W_______)) {
				SetDlgItemText(IDC_EMAIL_BOX, m_strEmail);
				m_changed = FALSE;
				return;
			}
			field = "Email";
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			// (z.manning 2009-07-09 11:28) - PLID 27251 - Declined email
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT Email, DeclinedEmail \r\n"
				"FROM PersonT \r\n"
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"
				"WHERE ID = {INT} \r\n"
				, m_id);
			item = aeiPatientEmail;
			if(AdoFldBool(rs->GetFields(), "DeclinedEmail", FALSE)) {
				strOld = "< Declined >";
			}
			else if(!rs->eof && rs->Fields->Item["Email"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Email"]->Value.bstrVal);
			//(c.copits 2011-09-22) PLID 45632 - General 1 email validation lets through invalid addresses.
			// Save old email address if restoring from validation
			m_strOldEmail = strOld;
			// If we're rejecting invalid email addresses, and the email is invalid, don't save.
			CString strEmail;
			GetDlgItemText(IDC_EMAIL_BOX, strEmail);

			// (b.eyers 2015-02-20) - PLID 64083 - If RejectInvalidEmailAddresses is on and there is whitespace after the email, it wasn't saving
			strEmail.TrimLeft();
			strEmail.TrimRight();
			SetDlgItemText(IDC_EMAIL_BOX, strEmail);

			// However, we must allow blank email addresses to be saved (if we switch to declined)
			if (!IsValidEmailAddress(strEmail) && GetRemotePropertyInt("RejectInvalidEmailAddresses", 0, 0, "<None>", true)
				&& strEmail != "") {
				return;
			}
		}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;
			
			break;
		case IDC_TITLE_BOX:
			{
				field = "Title";
				//for auditing
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT Title FROM PersonT WHERE ID = {INT}", m_id);
				item = aeiPatientTitle;
				if(!rs->eof) {
					strOld = AdoFldString(rs, "Title", "");
				}

				// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
				bUpdateHL7 = TRUE;
			}
			break;
		case IDC_ADDRESS1_BOX: 
			{
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			CString strAddress;
			GetDlgItemText(IDC_ADDRESS1_BOX, strAddress);
			// If not edited (user just clicked on it), then don't change anything.
			if ( m_strAddress1 == strAddress) {
				return;
			}

			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientAddress, SPT___W_______)) {
				SetDlgItemText(IDC_ADDRESS1_BOX, m_strAddress1);
				m_changed = false;
				return;
			}

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			if (bTruncateTrailingNameAddressSpace) {
				strAddress.TrimRight();
				SetDlgItemText(IDC_ADDRESS1_BOX, strAddress);
			}

			field = "Address1";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Address1 FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientAddress;
			if(!rs->eof && rs->Fields->Item["Address1"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Address1"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_ADDRESS2_BOX: 
			{
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			CString strAddress;
			GetDlgItemText(IDC_ADDRESS2_BOX, strAddress);
			if (m_strAddress2 == strAddress) {
				return;
			}

			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientAddress, SPT___W_______)) {
				SetDlgItemText(IDC_ADDRESS2_BOX, m_strAddress2);
				m_changed = false;
				return;
			}

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			if (bTruncateTrailingNameAddressSpace) {
				strAddress.TrimRight();
				SetDlgItemText(IDC_ADDRESS2_BOX, strAddress);
			}

			field = "Address2";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Address2 FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientAddress2;
			if(!rs->eof && rs->Fields->Item["Address2"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Address2"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;
			break;
		case IDC_CITY_BOX: 
			{	
			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			CString strCity;
			GetDlgItemText(IDC_CITY_BOX, strCity);
			if (m_strCity == strCity) {
				return;
			}

			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientAddress, SPT___W_______)) {
				SetDlgItemText(IDC_CITY_BOX, m_strCity);
				m_changed = false;
				return;
			}

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			if (bTruncateTrailingNameAddressSpace) {
				strCity.TrimRight();
				SetDlgItemText(IDC_CITY_BOX, strCity);
			}

			// (j.gruber 2009-10-06 11:57) - PLID 35825 - update based preference
			if (m_bLookupByCity) {
					
				//for auditing
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				//(e.lally 2009-09-15) PLID 21585 - Audit changes to city and state
				_RecordsetPtr rs = CreateParamRecordset("SELECT City, State, Zip FROM PersonT WHERE ID = {INT}", m_id);
				CString strOldCity, strOldState, strOldZip;
				if(!rs->eof){
					strOldCity = AdoFldString(rs, "City", "");
					strOldState = AdoFldString(rs, "State", "");
					strOldZip = AdoFldString(rs, "Zip", "");
				}
				rs->Close();
				//

				CString strNewCity,
						strNewState,
						strNewZip,						
						tempState,
						tempZip;
				GetDlgItemText(IDC_CITY_BOX, strNewCity);
				GetDlgItemText(IDC_ZIP_BOX, tempZip);
				GetDlgItemText(IDC_STATE_BOX, tempState);
				tempZip.TrimRight();
				tempState.TrimRight();
				strNewCity.TrimRight();
				//Prompt to see if they wish to overwrite state/zip				
				if(!tempZip.IsEmpty() || !tempState.IsEmpty()) {
					//(e.lally 2009-09-15) PLID 21585 - Check if nothing actually changed first
					if(strNewCity == strOldCity){
						return;
					}
					if(AfxMessageBox("You have changed the city but the state or postal code already have data in them.  Would you like to overwrite "
						"this data with that of the new city?", MB_YESNO) == IDYES)
					{
						//Just treat them as empty and the code below will fill them.
						tempZip.Empty();
						tempState.Empty();
					}
				}
				if(tempZip == "" || tempState == "") {
					GetCityInfo(strNewCity, &strNewZip, &strNewState);
					if(tempZip == "") 
						SetDlgItemText(IDC_ZIP_BOX, strNewZip);
					else strNewZip = tempZip;
					if(tempState == "")
						SetDlgItemText(IDC_STATE_BOX, strNewState);
					else strNewState = tempState;

					//(e.lally 2009-09-15) PLID 21585 - Check if nothing actually changed
					if(strNewCity == strOldCity && strNewZip == strOldZip && strNewState == strOldState){	
						return;
					}
				
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					ExecuteParamSql("UPDATE PersonT SET Zip = {STRING}, City = {STRING}, State = {STRING} WHERE ID = {INT}", strNewZip, strNewCity, strNewState, m_id);
					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();
				}
				else {
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					// Check if nothing actually changed
					if(strNewCity == strOldCity){
						return;
					}
					ExecuteParamSql("UPDATE PersonT SET City = {STRING} WHERE ID = {INT}", strNewCity, m_id);
					// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
					UpdateHL7Data();
				}

				// UPDATE THE PALM RECORD
				UpdatePalm();

				//auditing
				//(e.lally 2009-09-15) PLID 21585 - Audit city and state if they changed
				CAuditTransaction audit;
				if(strOldCity != strNewCity) {
					AuditEvent(m_id, m_strPatientName, audit, aeiPatientCity, m_id, strOldCity, strNewCity, aepMedium, aetChanged);
				}
				if(strOldZip != strNewZip) {
					AuditEvent(m_id, m_strPatientName, audit, aeiPatientZip, m_id, strOldZip, strNewZip, aepMedium, aetChanged);
				}
				if(strOldState != strNewState) {
					AuditEvent(m_id, m_strPatientName, audit, aeiPatientState, m_id, strOldState, strNewState, aepMedium, aetChanged);
				}

				audit.Commit();
				//
				//m_changed = false;
				return;
			}
			else {
			
				field = "City";
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT City FROM PersonT WHERE ID = {INT}", m_id);
				item = aeiPatientCity;
				if(!rs->eof && rs->Fields->Item["City"]->Value.vt == VT_BSTR) {
					strOld = CString(rs->Fields->Item["City"]->Value.bstrVal);
				}

				// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
				bUpdateHL7 = TRUE;
			}
		}
			break;
		case IDC_STATE_BOX: 
			{

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			BOOL bTruncateTrailingNameAddressSpace = GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true);

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			CString strState;
			GetDlgItemText(IDC_STATE_BOX, strState);
			if (m_strState == strState) {
				return;
			}

			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientAddress, SPT___W_______)) {
				SetDlgItemText(IDC_STATE_BOX, m_strState);
				m_changed = false;
				return;
			}

			//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
			if (bTruncateTrailingNameAddressSpace) {
				strState.TrimRight();
				SetDlgItemText(IDC_STATE_BOX, strState);
			}

			field = "State";
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT State FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientState;
			if(!rs->eof && rs->Fields->Item["State"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["State"]->Value.bstrVal);
			}
			
			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_MARRIAGE_OTHER_BOX: {
			GetDlgItemText(nID, value);
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT SpouseName FROM PatientsT WHERE PersonID = {INT}", m_id);
			item = aeiPatientSpouseStatus;
			if(!rs->eof && rs->Fields->Item["SpouseName"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["SpouseName"]->Value.bstrVal);

			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PatientsT SET SpouseName = {STRING} WHERE PersonID = {INT}", value, m_id);
			//for auditing
			strNew = value;

			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) 
				AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, strNew, aepMedium, aetChanged);

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			UpdateHL7Data();

			return;
			}
			break;
		case IDC_EMERGENCY_FIRST_NAME: {
			field = "EmergFirst";
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT EmergFirst FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientEmergFirst;
			if(!rs->eof && rs->Fields->Item["EmergFirst"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmergFirst"]->Value.bstrVal);
			}
			break;
		case IDC_EMERGENCY_LAST_NAME: {
			field = "EmergLast";
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT EmergLast FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientEmergLast;
			if(!rs->eof && rs->Fields->Item["EmergLast"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmergLast"]->Value.bstrVal);
			}
			break;
		case IDC_DEAR_BOX:
			{
			//for auditing
			CString strOld;
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr prsAudit = CreateParamRecordset("SELECT NickName FROM PatientsT WHERE PersonID = {INT}", m_id);
			if(!prsAudit->eof) 
				strOld = AdoFldString(prsAudit, "NickName", "");
			GetDlgItemText(nID, value);
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PatientsT SET NickName = {STRING} WHERE PersonID = {INT}", value, m_id);

			//DRT 6/30/2005 - PLID 16654 - Auditing
			if(strOld != value) {
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientNickname, m_id, strOld, value, aepLow, aetChanged);
			}

			return;
			}
			break;
		case IDC_HOME_PHONE_BOX: 
			{
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientPhoneNumbers, SPT___W_______)) {
				SetDlgItemText(IDC_HOME_PHONE_BOX, m_strHomePhone);
				m_changed = false;
				return;
			}
			field = "HomePhone";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT HomePhone FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientHPhone;
			if(!rs->eof && rs->Fields->Item["HomePhone"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["HomePhone"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_WORK_PHONE_BOX: 
			{
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientPhoneNumbers, SPT___W_______)) {
				SetDlgItemText(IDC_WORK_PHONE_BOX, m_strWorkPhone);
				m_changed = false;
				return;
			}
			field = "WorkPhone";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT WorkPhone FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientWPhone;
			if(!rs->eof && rs->Fields->Item["WorkPhone"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["WorkPhone"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_CELL_PHONE_BOX: {
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientPhoneNumbers, SPT___W_______)) {
				SetDlgItemText(IDC_CELL_PHONE_BOX, m_strCellPhone);
				m_changed = false;
				return;
			}
			field = "CellPhone";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT CellPhone FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientMobilePhone;
			if(!rs->eof && rs->Fields->Item["CellPhone"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["CellPhone"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_PAGER_PHONE_BOX: {
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientPhoneNumbers, SPT___W_______)) {
				SetDlgItemText(IDC_PAGER_PHONE_BOX, m_strPager);
				m_changed = false;
				return;
			}
			field = "Pager";
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Pager FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientPagerNumber;
			if(!rs->eof && rs->Fields->Item["Pager"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Pager"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_OTHER_PHONE_BOX: {
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientPhoneNumbers, SPT___W_______)) {
				SetDlgItemText(IDC_OTHER_PHONE_BOX, m_strOtherPhone);
				m_changed = false;
				return;
			}
			field = "OtherPhone";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT OtherPhone FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientOtherPhone;
			if(!rs->eof && rs->Fields->Item["OtherPhone"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["OtherPhone"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_FAX_PHONE_BOX: {
			// (j.gruber 2007-08-08 15:21) - PLID 25045 - don't save if they don't have permission
			if (!CheckCurrentUserPermissions(bioPatientPhoneNumbers, SPT___W_______)) {
				SetDlgItemText(IDC_FAX_PHONE_BOX, m_strFax);
				m_changed = false;
				return;
			}
			field = "Fax";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Fax FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientFaxNumber;
			if(!rs->eof && rs->Fields->Item["Fax"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Fax"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_EMERGENCY_HOME: {
			field = "EmergHPhone";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT EmergHPhone FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientEmergHPhone;
			if(!rs->eof && rs->Fields->Item["EmergHPhone"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmergHPhone"]->Value.bstrVal);
			}
			break;
		case IDC_EMERGENCY_WORK: {
			field = "EmergWPhone";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT EmergWPhone FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientEmergWPhone;
			if(!rs->eof && rs->Fields->Item["EmergWPhone"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmergWPhone"]->Value.bstrVal);
			}
			break;
		case IDC_EXT_PHONE_BOX: {
			field = "Extension";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Extension FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientExtension;
			if(!rs->eof && rs->Fields->Item["Extension"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Extension"]->Value.bstrVal);
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			bUpdateHL7 = TRUE;

			break;
		case IDC_NOTES: {
			field = "Note";
			//for auditing
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Note FROM PersonT WHERE ID = {INT}", m_id);
			item = aeiPatientG1Note;
			if(!rs->eof && rs->Fields->Item["Note"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Note"]->Value.bstrVal);
			}
			break;
		case IDC_CUSTOM1_BOX:
			GetDlgItemText(nID, value);
			SaveCustomInfo(1,value);
			return;
			break;
		case IDC_CUSTOM2_BOX:
			GetDlgItemText(nID, value);
			SaveCustomInfo(2,value);
			return;
			break;
		case IDC_CUSTOM3_BOX:
			GetDlgItemText(nID, value);
			SaveCustomInfo(3,value);
			return;
			break;
		case IDC_CUSTOM4_BOX:
			GetDlgItemText(nID, value);
			SaveCustomInfo(4,value);
			return;
			break;
		default:
			return;
	}
	GetDlgItemText(nID, value);
	switch (nID)
	{		case IDC_ZIP_BOX:	
			case IDC_SOC_SEC_NO_BOX: 
			case IDC_EXT_PHONE_BOX:
				UnformatText(value);
				break;
			case IDC_HOME_PHONE_BOX: 
			case IDC_WORK_PHONE_BOX: 
			case IDC_CELL_PHONE_BOX: 
			case IDC_PAGER_PHONE_BOX: 
			case IDC_FAX_PHONE_BOX: 
			case IDC_OTHER_PHONE_BOX: 
			case IDC_EMERGENCY_HOME: 
			case IDC_EMERGENCY_WORK: {
				CString str = value;
				UnformatText(str);
				str.TrimRight();
				if(str=="")
					value="";
				value.Replace("#"," ");
				break;
			}
	}
	//(c.copits 2010-10-19) PLID 27344 - Truncate trailing space on names and addresses
	// Must now allow blank spaces at the end of certain values per preference.
	switch (nID) {
			// Don't trim these values.
		case IDC_FIRST_NAME_BOX:
		case IDC_MIDDLE_NAME_BOX:
		case IDC_LAST_NAME_BOX:
		case IDC_ADDRESS1_BOX:
		case IDC_ADDRESS2_BOX:
		case IDC_CITY_BOX:
		case IDC_STATE_BOX:
		case IDC_ZIP_BOX:
			break;
			// Preserve old behavior -- any other value, trim.
		default:
			value.TrimRight();
	}
	//DRT 11/14/2008 - PLID 32036 - Parameterized.  Also removed nonsense if/else, the same code was
	//	begin run either way.
	ExecuteParamSql(FormatString("UPDATE PersonT SET %s = {STRING} WHERE ID = {INT}", field), value, m_id);
	//m_changed = false;

	// (j.gruber 2007-08-08 15:21) - PLID 25045 - update the new values
	switch (nID) {
		case IDC_ADDRESS1_BOX:
			m_strAddress1 = value;
		break;
		
		case IDC_ADDRESS2_BOX:
			m_strAddress2 = value;
		break;

		case IDC_CITY_BOX:
			m_strCity = value;
		break;

		case IDC_STATE_BOX:
			m_strState = value;
		break;

		case IDC_ZIP_BOX:
			m_strZip = value;
		break;

		case IDC_HOME_PHONE_BOX:
			m_strHomePhone = value;
		break;

		case IDC_WORK_PHONE_BOX:
			m_strWorkPhone = value;
		break;

		case IDC_CELL_PHONE_BOX:
			m_strCellPhone = value;
		break;

		case IDC_OTHER_PHONE_BOX:
			m_strOtherPhone = value;
		break;

		case IDC_PAGER_PHONE_BOX:
			m_strPager = value;
		break;

		case IDC_FAX_PHONE_BOX:
			m_strFax = value;
		break;

		case IDC_EMAIL_BOX:
			m_strEmail = value;
		break;
	}


		

			
	//for auditing
	if(item != -1 && strOld != value) {
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, value, aepMedium, aetChanged);
	}
	CClient::RefreshTable(NetUtils::PatG1, m_id);
	// UPDATE THE PALM RECORD
	UpdatePalm();

	// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
	// (j.jones 2008-04-10 14:45) - PLID 29620 - do not update HL7 if nothing changed,
	// which we can only determine based on audit information, so if we don't have audit
	// information, err on the side of not knowing, and update HL7 anyways
	if (bUpdateHL7 && (item == -1 || strOld != value)) {
		UpdateHL7Data();
	}
	
	// (j.jones 2008-04-10 14:47) - why are we saving if nothing is audited, or, if nothing changed?
	// if you hit this assert, find out why, because item should really never be -1
	ASSERT(item != -1);

	}	NxCatchAll("Error saving General1 data ");
}

void CGeneral1Dlg::LoadPatientImagingInfo(_RecordsetPtr prsCurPatient)
{
	// (j.jones 2008-06-13 11:12) - aside from the "send to" button code, if you change this function,
	// check and see if the same-named function in CEMRSummaryDlg also needs changed

	//Note - this function controls the "Send To Third Party" button's appearance and text as well

	// This will be used later to change the interface depending on if this patient in attached to united or mirror
	bool bHasImaging = false;
	bool bShowImage = true;

	//status of the 'send to' button
	m_dwSendToStatus = 0;

	// Reset the error status of the image button
	m_imageButton.m_nError = eNoError;

	// Do they have inform?
	m_InformPath = GetRemotePropertyText ("InformDataPath", "", 0, "<None>");
	// (j.jones 2011-07-15 12:00) - PLID 42073 - changed to silent
	if (g_pLicense->CheckForLicense(CLicense::lcInform, CLicense::cflrSilent) && m_InformPath != "" /* && DoesExist(m_InformPath)*/) 
	{	
		long InformID = -1;

		if(prsCurPatient->Fields->Item["InformID"]->Value.vt == VT_I4)
			InformID = prsCurPatient->Fields->Item["InformID"]->Value.lVal;

		if(InformID > 0)
			m_dwSendToStatus |= TPL_UPDATE_INFORM;
		else
			m_dwSendToStatus |= TPL_SEND_TO_INFORM;
	}

	// (c.haag 2009-07-08 09:44) - PLID 34379 - Should we try to see images in RSI MMS for this patient?
	if (m_nMMSPatientID != -1) {
		bHasImaging = true;
	}

	// Do they have United?
	m_nUnitedID = -1;
	// (j.jones 2011-07-15 12:00) - PLID 42073 - changed to silent
	if (g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrSilent) && IsUnitedEnabled()) {
		
		// Yes, they have United

		// Figure out if this patient has a United record
		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame) {
			CUnitedLink *pUnitedLink = pMainFrame->GetUnitedLink();
			if (pUnitedLink && pUnitedLink->GetRemotePath() != "") {
				// Is this patient attached to United?
				m_nUnitedID = AdoFldLong(prsCurPatient, "UnitedID", -1);
				if (m_nUnitedID != -1) {
					// Yes, this patient is already attached to United
					m_dwSendToStatus |= TPL_UPDATE_UNITED;
					bHasImaging = true;
				} else {
					// No, this patient is not yet attached to United
					m_dwSendToStatus |= TPL_SEND_TO_UNITED;
				}
			}
		}
	}

	//do they have Quickbooks?
	if (g_pLicense->GetHasQuickBooks() && GetRemotePropertyInt("DisableQuickBooks",0,0,"<None>",TRUE) == 0) {

		if (AdoFldBool(prsCurPatient, "SentToQuickbooks"))
			// Yes, this patient is already attached to Quickbooks
			m_dwSendToStatus |= TPL_UPDATE_QBOOKS;
		else
			// No, this patient is not yet attached to Quickbooks
			m_dwSendToStatus |= TPL_SEND_TO_QBOOKS;
	}

	//Do they have the CareCredit link?
	if(NxCareCredit::GetCareCreditLicenseStatus() != cclsExpired) {
		m_dwSendToStatus |= TPL_SEND_TO_CARECREDIT;
	}

	// (j.gruber 2013-04-22 11:47) - PLID 56361 - see about devices
	if (DeviceLaunchUtils::HasLaunchableDevice()) {
		m_dwSendToStatus |= TPL_HAS_LAUNCH_DEVICE;
	}

	//display the button properly
	UpdateSendToThirdPartyButton();

	// (c.haag 2009-04-01 17:02) - PLID 33630 - Before we do anything else, try to establish a connection
	// with Mirror. If the result is that the link is being established, put the "Initializing Mirror"
	// image on the thumbnail region, and quit immediately. Otherwise, proceed as before.
	if (DoesCanfieldSDKNeedToInitialize()) {
		return;
	}

	// Do they have Mirror?
	if (Mirror::HasMirrorLinkLicense() && Mirror::IsMirrorEnabled())
	{
		// Yes, they have Mirror
		// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
		const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
		if (!bUsingSDKFunctionality) {
			// Does the mirror database even exist?
			if (GetFileAttributes(Mirror::GetMirrorDataPath()) == -1)
			{
				// Uh-oh. Lets tell the user in the form of a
				// thumbnail of text.
				m_imageButton.m_nError = eErrorUnspecified;
				GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
				ShowImagingButtons(1);
				m_dwSendToStatus |= TPL_DISABLE_MIRROR;
			}
			// Is the mirror database read only?
			else if (GetFileAttributes(Mirror::GetMirrorDataPath()) & FILE_ATTRIBUTE_READONLY)
			{
				// Darn tootin'
				m_dwSendToStatus |= TPL_DISABLE_MIRROR;
			}
			else
			{
				if (!(GetCurrentUserPermissions(bioPatient) & SPT___W_______) ||
					!(GetCurrentUserPermissions(bioMirrorIntegration) & SPT_______1___))
					m_dwSendToStatus |= TPL_DISABLE_MIRROR;
			}
		}

		// Is this patient attached to Mirror?
		if (m_strMirrorID != "") {
			// Yes, this patient is already attached to Mirror
			m_dwSendToStatus |= TPL_UPDATE_MIRROR;
			bHasImaging = true;
		} else {
			// No, this patient is not yet attached to Mirror
			m_dwSendToStatus |= TPL_SEND_TO_MIRROR;
		}
	}


	//display the button properly
	UpdateSendToThirdPartyButton();

	// Do they have pictures in the history tab?
	if (GetPatientAttachedImageCount(m_id) > 0)
		bHasImaging = true;

	// What is the default image for this patient?
	m_nImageIndex = AdoFldLong(prsCurPatient, "ImageIndex", 0);
	
	if (bHasImaging) {
		// Show the default image (LoadImage() also sets the enabled states of the prev/next buttons)
		LoadPatientImage();
	} else {
		// Change interface depending on if the patient is attached to imaging software
		GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
		ShowImagingButtons(0);
	}
}

void CGeneral1Dlg::Load(bool overwrite /*=true*/)
{
	CWaitCursor pWait;

	if (m_loading) return;//may need if timer hits in this function
	try
	{
		m_loading = true;
		_variant_t var;

		// (c.haag 2010-02-23 09:55) - PLID 37364 - Delete the Mirror image manager if it exists
		EnsureNotMirrorImageMgr();

		// (a.walling 2010-10-12 15:00) - PLID 40908 - Tablechecker-based refreshes moved to their own function
		HandleTableCheckers();
		m_ForceRefresh = false;

		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);

		//DRT 11/13/2008 - PLID 32036 - Parameterized - This is OK because the m_sql is a static string, no
		//	variables included.
		// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), FormatString("%s WHERE ID = {INT};", m_sql), m_id);
		_RecordsetPtr rsCustom;

		// (a.walling 2013-12-12 16:51) - PLID 59998 - General1 loads Country in a separate recordset
		CString strPersonCountry = AdoFldString(rs, "Country", "");

		//these CStrings are used to compare the names with when one is changed
		m_strFirstName = CString(rs->Fields->Item["First Name"]->Value.bstrVal);
		m_strMiddleName = CString(rs->Fields->Item["Middle Name"]->Value.bstrVal);
		m_strLastName = CString(rs->Fields->Item["Last Name"]->Value.bstrVal);
		
		m_strSocialSecurity = AdoFldString(rs, "SS #");
		// (f.dinatale 2010-10-20) - PLID 33753 - Mask the SSN appropriately to permissions.
		if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
			m_strSocialSecurity = FormatSSNText(m_strSocialSecurity, eSSNNoMask, "###-##-####");
		} else {
			if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				m_strSocialSecurity = FormatSSNText(m_strSocialSecurity, eSSNPartialMask, "###-##-####");
			} else {
				m_strSocialSecurity = FormatSSNText(m_strSocialSecurity, eSSNFullMask, "###-##-####");
			}
		}

		SetDlgItemText(IDC_SOC_SEC_NO_BOX, m_strSocialSecurity);

		// (c.haag 2009-07-07 13:13) - PLID 34379 - Load the internal RSI MMS patient ID
		m_nMMSPatientID = RSIMMSLink::GetInternalPatientID(m_strFirstName, m_strLastName);

		// (c.haag 2004-11-23 11:37) - PLID 14727 - Get the Mirror ID
		m_strMirrorID = AdoFldString(rs, "MirrorID", "");

		m_strAddress1 = AdoFldString(rs, "Address 1", "");
		m_strAddress2 = AdoFldString(rs, "Address 2", "");
		m_strCity = AdoFldString(rs, "City", "");
		m_strState = AdoFldString(rs, "StateProv", "");
		m_strZip = AdoFldString(rs, "PostalCode", "");


		// (j.gruber 2007-08-08 12:09) - PLID 25045 - save the address boxes here
		if (!(GetCurrentUserPermissions(bioPatientAddress) & (SPT__R________))) {
			SetDlgItemText(IDC_ADDRESS1_BOX, "<Hidden>");
			SetDlgItemText(IDC_ADDRESS2_BOX, "<Hidden>");
			SetDlgItemText(IDC_CITY_BOX, "<Hidden>");
			SetDlgItemText(IDC_STATE_BOX, "<Hidden>");
			SetDlgItemText(IDC_ZIP_BOX, "<Hidden>");
		}
		else {
			SetDlgItemText (IDC_ADDRESS1_BOX,			m_strAddress1);//				overwrite);
			SetDlgItemText (IDC_ADDRESS2_BOX,			m_strAddress2);//				overwrite);
			SetDlgItemText (IDC_CITY_BOX,				m_strCity);//,					overwrite);
			SetDlgItemText (IDC_STATE_BOX,				m_strState);//,				overwrite);
			SetDlgItemText (IDC_ZIP_BOX,				m_strZip);//,			overwrite);
		}
		// (s.dhole 2010-03-26 17:56) - PLID 37796 For internal only, make the company show up in G1.
		CString strCompany = AdoFldString(rs, "Company",""); 
		if (IsNexTechInternal() && strCompany!="" )
		{
			m_nxlabelSetCompanyLink.ShowWindow(SW_SHOW);
			m_nxlabelSetCompanyLink.SetType(dtsHyperlink);
			m_nxlabelSetCompanyLink.SetText("Company: " + strCompany) ;
		}
		else {
			m_nxlabelSetCompanyLink.ShowWindow(SW_HIDE);
		}

		SetDlgItemText (IDC_EMPLOYER_BOX,			strCompany); //(s.dhole 2010-03-26 17:56) - PLID 37796 For internal only, make the company show up in G1.//				overwrite);
		SetDlgItemText (IDC_TITLE_BOX,				CString(rs->Fields->Item["Title"]->Value.bstrVal));//				overwrite);
		SetDlgItemText (IDC_FIRST_NAME_BOX,			m_strFirstName);											//			overwrite);
		SetDlgItemText (IDC_MIDDLE_NAME_BOX,		m_strMiddleName);										//			overwrite);
		SetDlgItemText (IDC_LAST_NAME_BOX,			m_strLastName);											//			overwrite);
		SetDlgItemText (IDC_MARRIAGE_OTHER_BOX,		CString(rs->Fields->Item["SpouseName"]->Value.bstrVal));//,			overwrite);
		SetDlgItemText (IDC_EMERGENCY_FIRST_NAME,	CString(rs->Fields->Item["EmergCnctFName"]->Value.bstrVal));//,		overwrite);
		SetDlgItemText (IDC_EMERGENCY_LAST_NAME,	CString(rs->Fields->Item["EmergCnctLName"]->Value.bstrVal));//,		overwrite);
		m_ExcludeMailingsCheck.SetCheck(AdoFldBool(rs, "ExcludeFromMailings"));
		
		CString sql;

		//important to clear out existing data, because CustomFieldDataT won't have an entry if the field is blank
		SetDlgItemText(IDC_CUSTOM1_BOX,CString(""));
		SetDlgItemText(IDC_CUSTOM2_BOX,CString(""));
		SetDlgItemText(IDC_CUSTOM3_BOX,CString(""));
		SetDlgItemText(IDC_CUSTOM4_BOX,CString(""));

		// j.anspach 4/15/04 PLID 16185: Removed the ConvertToControlTest function call from each of the SetDlgItemTexts.
		//	It was causing all ampersands to duplicate upon set and that would cause them to save as doubles.
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
		rsCustom = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TextParam, FieldID FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID >=1 AND FieldID <=4",m_id);
		while(!rsCustom->eof) {
			long i = AdoFldLong(rsCustom, "FieldID");
			switch (i) {
			case 1:
				SetDlgItemText (IDC_CUSTOM1_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			case 2:
				SetDlgItemText (IDC_CUSTOM2_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			case 3:
				SetDlgItemText (IDC_CUSTOM3_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			case 4:
				SetDlgItemText (IDC_CUSTOM4_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			}
			rsCustom->MoveNext();
		}
		rsCustom->Close();

		// (j.gruber 2007-08-08 12:12) - PLID 25045 - now the phone numbers and emails
		m_strHomePhone = AdoFldString(rs, "Home Phone","");
		m_strWorkPhone = AdoFldString(rs, "Work Phone","");
		m_strCellPhone = AdoFldString(rs, "Cell Phone","");
		m_strPager = AdoFldString(rs, "Pager","");
		m_strFax = AdoFldString(rs, "FaxNumber","");
		m_strOtherPhone = AdoFldString(rs, "Other Phone","");
		if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			SetDlgItemText(IDC_HOME_PHONE_BOX, "<Hidden>");
			SetDlgItemText(IDC_WORK_PHONE_BOX, "<Hidden>");
			SetDlgItemText(IDC_CELL_PHONE_BOX, "<Hidden>");
			SetDlgItemText(IDC_OTHER_PHONE_BOX, "<Hidden>");
			SetDlgItemText(IDC_PAGER_PHONE_BOX, "<Hidden>");
			SetDlgItemText(IDC_FAX_PHONE_BOX, "<Hidden>");
		}
		else {
			SetDlgItemText (IDC_HOME_PHONE_BOX,			m_strHomePhone);//,			overwrite);
			SetDlgItemText (IDC_WORK_PHONE_BOX,			m_strWorkPhone);//,			overwrite);
			SetDlgItemText (IDC_CELL_PHONE_BOX,			m_strCellPhone);//,			overwrite);
			SetDlgItemText (IDC_PAGER_PHONE_BOX,		m_strPager);//,					overwrite);
			SetDlgItemText (IDC_FAX_PHONE_BOX,			m_strFax);//,				overwrite);
			SetDlgItemText (IDC_OTHER_PHONE_BOX,		m_strOtherPhone);//,			overwrite);
		}
		
		m_strEmail = AdoFldString(rs, "EmailAddress","");
		if (!(GetCurrentUserPermissions(bioPatientEmail) & (SPT__R________))) {
			SetDlgItemText(IDC_EMAIL_BOX, "<Hidden>");
		}
		else {
			// (z.manning 2009-07-09 11:06) - PLID 27251 - The patient may have declined email when
			// they were created so let's display that as such here.
			BOOL bDeclinedEmail = AdoFldBool(rs->GetFields(), "DeclinedEmail");
			UpdateEmail(bDeclinedEmail, m_strEmail);
		}

		


		SetDlgItemText (IDC_EXT_PHONE_BOX,			AdoFldString(rs, "Extension",""));//,				overwrite);
		SetDlgItemVar (IDC_ID_BOX,						rs->Fields->Item["ID"]->Value,						overwrite);
		SetDlgItemText (IDC_DEAR_BOX,					AdoFldString(rs, "Nickname",""));//,				overwrite);
		SetDlgItemText (IDC_NOTES,						AdoFldString(rs, "Memo",""));//,					overwrite);
		SetDlgItemText (IDC_EMERGENCY_RELATE,		AdoFldString(rs, "EmergContactRel",""));//,		overwrite);
		SetDlgItemText (IDC_EMERGENCY_HOME,			AdoFldString(rs, "EmergCnctHPhone",""));//,		overwrite);
		SetDlgItemText (IDC_EMERGENCY_WORK,			AdoFldString(rs, "EmergCnctWPhone",""));//,		overwrite);
		SetDlgItemText (IDC_REFERRAL,					AdoFldString(rs, "ReferralName",""));//,			overwrite);
		
		_variant_t varDate = rs->Fields->Item["Birth Date"]->Value;
		if(varDate.vt == VT_DATE) {
			m_dtBirthDate = VarDateTime(varDate);
			m_nxtBirthDate->SetDateTime(VarDateTime(varDate));
			m_bBirthDateSet = true;
		}
		else {
			m_dtBirthDate.SetDateTime(0,0,0,0,0,0);
			m_dtBirthDate.m_dt = 0;
			m_dtBirthDate.SetStatus(COleDateTime::invalid);
			m_nxtBirthDate->Clear();
			m_bBirthDateSet = false;
		}
		varDate = rs->Fields->Item["First Contact Date"]->Value;
		if(varDate.vt == VT_DATE) {
			m_dtFirstContact = VarDateTime(varDate);
			m_nxtFirstContact->SetDateTime(m_dtFirstContact);
		}
		else {
			m_dtFirstContact.SetDateTime(0,0,0,0,0,0);
			m_dtFirstContact.SetStatus(COleDateTime::invalid);
			m_nxtFirstContact->Clear();
		}
		
		//TS:  Enable or disable the "E-mail" button 
		UpdateEmailButton();
		// Handle the age field specially
		if(EnsureCorrectAge()) {
			//if it changed the date, save
			m_loading = false;
			m_changed = true;
			Save(IDC_BIRTH_DATE_BOX);
			m_loading = true;
		}

		// Set status to what the rs tells us and if that fails default to unknown so it can be fixed (0)

		long nStatus;
		if(rs->Fields->Item["CurrentStatus"]->Value.vt == VT_NULL)
			nStatus = 0;
		else {
			//TS 4/11/2002: This just set nStatus = Value, without the VarShort!!!!  You all are lucky I came along.
			nStatus = AdoFldShort(rs, "CurrentStatus");
		}

		m_isPatient.SetCheck(nStatus == 1);
		m_isProspect.SetCheck((nStatus != 1) && (nStatus != 3)); // default
		m_isPatientProspect.SetCheck(nStatus == 3);
		//TES 12/18/2009 - PLID 35055 - PatientView already handles the colors just fine, there's no need for this code, which
		// was screwing up the special status-based colors in Internal.
		/*if(m_demBkg.GetColor() != GetNxColor(GNC_PATIENT_STATUS, nStatus))
			SetColor(GetNxColor(GNC_PATIENT_STATUS, nStatus));*/

		//since Brad decided to delete the Status dialog for these problem patients,
		//we couldn't handle bad data. Thanks Brad! This should fix it.
		//DRT 1/11/2007 - PLID 24209 - We added a permission when clicking the buttons, but
		//	in this bad-data case, we launch the button handler.  I've decided to leave this
		//	in, since the permission only affects internal data, and we definitely don't have
		//	any of these.
		if(nStatus == 0)
			OnStatusChanged();
		
		long PrefixID = AdoFldLong(rs, "PrefixID", -1);
		if(PrefixID > -1)
			m_PrefixCombo->SetSelByColumn(0, (long)PrefixID);
		else
			m_PrefixCombo->CurSel = -1;

		m_PreferredContactCombo->SetSelByColumn(0,AdoFldLong(rs, "PreferredContact",0));

		// Fill the rest
		if (rs->Fields->Item["Archived"]->Value.vt != VT_NULL && rs->Fields->Item["Archived"]->Value.boolVal)
			m_InactiveCheck.SetCheck(true);
		else m_InactiveCheck.SetCheck(false);
		if (rs->Fields->Item["Foreign"]->Value.vt != VT_NULL && rs->Fields->Item["Foreign"]->Value.boolVal)
			m_ForeignCheck.SetCheck(true);
		else m_ForeignCheck.SetCheck(false);
		if (rs->Fields->Item["MaritalStatus"]->Value.vt != VT_NULL)
		{	if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "1"))
			{	m_singleRad.SetCheck(TRUE);
				m_marriedRad.SetCheck(FALSE);
				m_otherRad.SetCheck(FALSE);
				if(!IsReproductive()) {	
					CString str;
					GetDlgItemText(IDC_PARTNER_LABEL,str);
					if(str != "") {
						SetDlgItemText(IDC_PARTNER_LABEL,"");
						CRect rc;
						GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
						ScreenToClient(rc);
						InvalidateRect(rc);
					}
					GetDlgItem(IDC_MARRIAGE_OTHER_BOX)->ShowWindow(SW_HIDE);
				}
			}
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "2"))
			{	m_marriedRad.SetCheck(TRUE);
				m_singleRad.SetCheck(FALSE);
				m_otherRad.SetCheck(FALSE);
				if(!IsReproductive()) {
					CString str;
					GetDlgItemText(IDC_PARTNER_LABEL,str);
					if(str != "Spouse") {
						SetDlgItemText(IDC_PARTNER_LABEL,"Spouse");
						CRect rc;
						GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
						ScreenToClient(rc);
						InvalidateRect(rc);						
					}
					GetDlgItem(IDC_MARRIAGE_OTHER_BOX)->ShowWindow(SW_SHOW);
				}
			}
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "3"))
			{	m_otherRad.SetCheck(TRUE);
				m_marriedRad.SetCheck(FALSE);
				m_singleRad.SetCheck(FALSE);
				if(!IsReproductive()) {
					CString str;
					GetDlgItemText(IDC_PARTNER_LABEL,str);
					if(str != "Status") {
						SetDlgItemText(IDC_PARTNER_LABEL,"Status");
						CRect rc;
						GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
						ScreenToClient(rc);
						InvalidateRect(rc);
					}
					GetDlgItem(IDC_MARRIAGE_OTHER_BOX)->ShowWindow(SW_SHOW);
				}
			}
			else
			{	m_singleRad.SetCheck(FALSE);
				m_marriedRad.SetCheck(FALSE);
				m_otherRad.SetCheck(FALSE);
				if(!IsReproductive()) {	
					CString str;
					GetDlgItemText(IDC_PARTNER_LABEL,str);
					if(str != "") {
						SetDlgItemText(IDC_PARTNER_LABEL,"");
						CRect rc;
						GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
						ScreenToClient(rc);
						InvalidateRect(rc);
					}
					GetDlgItem(IDC_MARRIAGE_OTHER_BOX)->ShowWindow(SW_HIDE);
				}
			}
		}
		else
		{	m_marriedRad.SetCheck(FALSE);
			m_singleRad.SetCheck(FALSE);
			m_otherRad.SetCheck(FALSE);
			if(!IsReproductive()) {
				CString str;
				GetDlgItemText(IDC_PARTNER_LABEL,str);
				if(str != "") {
					SetDlgItemText(IDC_PARTNER_LABEL,"");
					CRect rc;
					GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
					ScreenToClient(rc);
					InvalidateRect(rc);
				}
				GetDlgItem(IDC_MARRIAGE_OTHER_BOX)->ShowWindow(SW_HIDE);
			}
		}

		if (rs->Fields->Item["Gender"]->Value.vt != VT_NULL && rs->Fields->Item["Gender"]->Value.vt != VT_EMPTY)
		{	if (AdoFldByte(rs, "Gender") == 1) {
				m_GenderCombo->CurSel = 1;
			}
			else if (AdoFldByte(rs, "Gender") == 2) {
				m_GenderCombo->CurSel = 2;
			}
			else //patient gender exists, but isn't male or female!
			{	
				m_GenderCombo->CurSel = 0;
			}
		}
		else //patient has no gender!
		{	
			m_GenderCombo->CurSel = 0;
		}
		if (rs->Fields->Item["PrivHome"]->Value.vt != VT_NULL && rs->Fields->Item["PrivHome"]->Value.boolVal)
			m_HomePrivCheck.SetCheck(true);
		else m_HomePrivCheck.SetCheck(false);
		if (rs->Fields->Item["PrivWork"]->Value.vt != VT_NULL && rs->Fields->Item["PrivWork"]->Value.boolVal)
			m_WorkPrivCheck.SetCheck(true);
		else m_WorkPrivCheck.SetCheck(false);
		if (rs->Fields->Item["PrivCell"]->Value.vt != VT_NULL && rs->Fields->Item["PrivCell"]->Value.boolVal)
			m_CellPrivCheck.SetCheck(true);
		else m_CellPrivCheck.SetCheck(false);

		//DRT 7/30/03 - Added privacy fields for other, pager, fax, email
		if (rs->Fields->Item["PrivEmail"]->Value.vt != VT_NULL && rs->Fields->Item["PrivEmail"]->Value.boolVal)
			m_btnEmailPriv.SetCheck(true);
		else m_btnEmailPriv.SetCheck(false);

		if (rs->Fields->Item["PrivPager"]->Value.vt != VT_NULL && rs->Fields->Item["PrivPager"]->Value.boolVal)
			m_btnPagerPriv.SetCheck(true);
		else m_btnPagerPriv.SetCheck(false);

		if (rs->Fields->Item["PrivOther"]->Value.vt != VT_NULL && rs->Fields->Item["PrivOther"]->Value.boolVal)
			m_btnOtherPriv.SetCheck(true);
		else m_btnOtherPriv.SetCheck(false);

		if (rs->Fields->Item["PrivFax"]->Value.vt != VT_NULL && rs->Fields->Item["PrivFax"]->Value.boolVal)
			m_btnFaxPriv.SetCheck(true);
		else m_btnFaxPriv.SetCheck(false);

		// (z.manning 2008-07-11 10:11) - PLID 30678 - TextMessage option
		if(AdoFldBool(rs, "TextMessage")) {
			m_btnTextMessage.SetCheck(BST_CHECKED);
		}
		else {
			m_btnTextMessage.SetCheck(BST_UNCHECKED);
		}

		// (c.haag 2006-04-13 09:14) - PLID 20018 - We now include the entered by user in the m_sql statement
		/*CString strSql;
		strSql.Format("Select usersT.Username AS UserName FROM PersonT Left Join UsersT ON PersonT.UserID = UsersT.PersonID WHERE PersonT.ID = %li", m_id);
		_RecordsetPtr rsCreator = CreateRecordset(strSql);
		CString strCreator = AdoFldString(rsCreator->GetFields(), "UserName", "");
		SetDlgItemText(IDC_ENTERED_BY, strCreator);*/
		SetDlgItemText(IDC_ENTERED_BY, AdoFldString(rs, "Username", ""));
		

		//to be safe, set m_loading to be false prior to the SetSel calls, but still put it outside the catch clause too
		m_loading = false;
		if(m_CoordCombo->TrySetSelByColumn(0,rs->Fields->Item["EmployeeID"]->Value) == -1) {
			//maybe it's inactive?
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
			_RecordsetPtr rsCoord = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT EmployeeID FROM PatientsT WHERE PersonID = {INT})", GetActivePatientID());
			if(!rsCoord->eof) {
				m_CoordCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCoord, "Name", "")));
			}
			else 
				m_CoordCombo->PutCurSel(-1);
		}

		
		if(IsReproductive()) {
			//see if they have a partner
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
			_RecordsetPtr rsTemp = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PartnerID FROM PartnerLinkT WHERE InactiveDate Is Null AND PatientID = {INT}",m_id);
			if(!rsTemp->eof) {
				//select the partner in the list
				m_PartnerCombo->TrySetSelByColumn(0,rsTemp->Fields->Item["PartnerID"]->Value);
			}
			else {
				//not found, try looking for the patient
				rsTemp->Close();				
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
				rsTemp = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PatientID FROM PartnerLinkT WHERE InactiveDate Is Null AND PartnerID = {INT}",m_id);
				if(!rsTemp->eof) {
					//select the patient in the list
					m_PartnerCombo->TrySetSelByColumn(0,rsTemp->Fields->Item["PatientID"]->Value);
				}
				else {
					m_PartnerCombo->PutCurSel(-1);
				}
				rsTemp->Close();
			}
		}

		if(m_DoctorCombo->TrySetSelByColumn(0,rs->Fields->Item["MainPhysician"]->Value) == -1) {
			//they may have an inactive provider
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot isolation loading General1
			_RecordsetPtr rsProv = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT})", GetActivePatientID());
			if(!rsProv->eof) {
				m_DoctorCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
			else 
				m_DoctorCombo->PutCurSel(-1);
		}

		// I had to make changes to this code and I just could not do it
		// without moving all the imaging code to a modular function.  So 
		// this line is an important one (as compared to each other line 
		// in this function) because it handles the loading and display 
		// of all the imaging info for this patient
		LoadPatientImagingInfo(rs);

		//DRT 7/23/02 - Load the fax combo for internal
		if(IsNexTechInternal()) {
			try {
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				rs = CreateParamRecordset("SELECT FaxChoice FROM PatientsT WHERE PersonID = {INT}", m_id);

				long nChoice;
				_variant_t var;

				var = rs->Fields->Item["FaxChoice"]->Value;
				if(var.vt == VT_I4)
					nChoice = VarLong(var);
				else
					nChoice = 0;

				((CComboBox*)GetDlgItem(IDC_FAX_CHOICE))->SetCurSel(nChoice);

			} NxCatchAll("Error loading fax choice");
		}

		m_changed = false;


		//
		// v.arth 2009-06-01 PLID 34386 Display the current patient's country
		// (a.walling 2013-12-12 16:51) - PLID 59998 - General1 loads Country in a separate recordset
		{
			// Get the value from the field
			if (strPersonCountry.IsEmpty())
			{
				// Default to an empty selection
				m_pCountryList->SetSelByColumn(0, -1);
			}
			else
			{
				// Show the country name if there was one
				m_pCountryList->SetSelByColumn(1, (const char*)strPersonCountry);
			}
		}

		// (r.gonet 2010-08-30 16:36) - PLID 39939 - Set the color of the Groups button based on the patient's
		//  inclusion in any letter writing group.
#pragma TODO("Groups button color is queried in a separate recordset")
		UpdateGroupsButton();
		m_ForceRefresh = false;
	}
	NxCatchAll("Error in Load: ");
	m_loading = false;
}

// (a.walling 2010-10-12 15:00) - PLID 40908 - Tablechecker-based refreshes moved to their own function
void CGeneral1Dlg::HandleTableCheckers()
{
	try {
		// (a.walling 2010-10-12 17:33) - PLID 40908 - Set m_ForceRefresh if something changed with the table checkers
		if (m_doctorChecker.Changed()) {
			m_DoctorCombo->Requery();
			m_ForceRefresh = true;
			//DRT 6/18/2004 - PLID 13079 - Moved the code to add the empty
			//	row to the requery finished function, where it belongs.
		}
		if (m_coordChecker.Changed()) {
			m_CoordCombo->Requery();
			m_ForceRefresh = true;
			//DRT 8/11/2004 - PLID 13858 - Moved the extra <no coord> row to requery finished
		}
		
		// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker,
		// these properties are cached, so just update the member variables from the cache,
		// and do not force a refresh
		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		if(IsReproductive()) {
			if (m_partnerChecker.Changed()) {
				m_PartnerCombo->Requery();
				IRowSettingsPtr pRow = m_PartnerCombo->GetRow(-1);				
				_variant_t var = (long)0;
				pRow->PutValue(0,var);
				pRow->PutValue(4,"<No Partner>");
				m_PartnerCombo->InsertRow(pRow,0);
				m_ForceRefresh = true;
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// Sets the birth date field to use 4-digit years and sets the age field to show the correct age
BOOL CGeneral1Dlg::EnsureCorrectAge()
{
	BOOL bChanged = FALSE;

	// Make sure we can only run this function once at a time
	static bool IsRunning = false;
	if (IsRunning)
		return FALSE;
	IsRunning = true;

	// Remember the selection if the focus is still on the birth date field
	int x1, x2;
	CNxEdit *tmpEdit = (CNxEdit *)GetDlgItem(IDC_BIRTH_DATE_BOX);
	tmpEdit->GetSel (x1, x2);

	if (m_nxtBirthDate->GetStatus() == 3) {
		// No birth date given
		IsRunning = false;
		SetDlgItemText(IDC_AGE_BOX, "");
		return FALSE;
	}
	
	// Convert the birth date to a datetime and make sure it's valid
	COleDateTime dt;
	dt = m_nxtBirthDate->GetDateTime();
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	if (m_nxtBirthDate->GetStatus() == 1) {
		
		// Make sure the birthdate is in the past
		if(dt > dtNow) {
			//JJ - 1/16/03 - if they enter a 2-digit year less than 30, it converts to 2000 than 1900, but if it is in the future,
			//then they get this message and have no idea why, because they only see 2 digits on the screen.
			//So, if the year is a two-digit year and invalid, fix the date, but don't warn them about it.
			if(dt.GetYear() > dtNow.GetYear())
				AfxMessageBox("You have entered a birthdate in the future. This will be adjusted to a valid date.");
			while (dt > dtNow) {
				dt.SetDate(dt.GetYear() - 100, dt.GetMonth(), dt.GetDay());
			}
			m_nxtBirthDate->SetDateTime(dt);

			bChanged = TRUE;
		}

		// Now that we have our guaranteed valid birth date, set 
		// the text box back (using 4-digit year; this is VITAL 
		// because the caller uses the contents of the text box 
		// to save to the database and we want to be sure we save 
		// with 4-digit year)
		tmpEdit->SetSel (x1, x2);

		// Then set the age box
		// (z.manning 2010-01-13 11:20) - PLID 22672 - Age is now a string		
		// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation,
		// validation should only be done when bdays are entered/changed
		SetDlgItemText(IDC_AGE_BOX, GetPatientAgeOnDate(dt, dtNow, TRUE));		
		// Then set the age box
		// (z.manning 2010-01-13 11:20) - PLID 22672 - Age is now a string		
		// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation,
		// validation should only be done when bdays are entered/changed
		SetDlgItemText(IDC_AGE_BOX, GetPatientAgeOnDate(dt, dtNow, TRUE));		
	}
	IsRunning = false;

	return bChanged;
}

void CGeneral1Dlg::OnStatusChanged() 
{
		// (a.levy 2012-10-10) - PLID 49474 (Initialize) Flag if changed from prospect to patient, DefaultVer Flag AND Set parameterize DefaultVersion ID
		// If default version is not set  and Prospect to patient abort writing to Version table.
	    bool bPatToProspect = false;
		bool bIsDefaultVersSet = false;
		bool bIsDefaultVerOffline = true;
	    int nGetVersionID = NULL;

	//JJ - commented this out so if we load a bad status, we can repair it

	//JJ - commented this out so if we load a bad status, we can repair it

	// If we're loading don't do anything
	//if (m_loading) return;
	if(IsNexTechInternal()) {

	
		try { 
		// (a.levy 2012-10-10) - PLID 49474 - For clients only
		_RecordsetPtr pdrs = CreateParamRecordset("SELECT ID, COALESCE(DefaultVerFlag,-1) AS DefaultVerFlag FROM ReleasedVersionsT WHERE forClients = 1");
		while(!pdrs->eof) {

			long nVersionStat = AdoFldLong(pdrs,"DefaultVerFlag");
			switch(nVersionStat) {
				case 0: //Previous DefaultVersion -This may possibly be used later.
					
					break;
				case 1: //Current DefaultVersion Set
                    nGetVersionID = AdoFldLong(pdrs,"ID");
					bIsDefaultVersSet = true;
					 //Default Version System is online - 
					bIsDefaultVerOffline = false;
					break;
					
				            						 
			                    }

			pdrs->MoveNext();
			

		         }
		    pdrs->Close();
		    }NxCatchAll("Failed to recieve default version ID Check your network or sql instance");
	
		if(!CheckCurrentUserPermissions(bioSupportBought, sptWrite)) {
			//Need to reset to their previous status
			try {
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr prs = CreateParamRecordset("SELECT CurrentStatus FROM PatientsT WHERE PersonID = {INT}", m_id);

				if(!prs->eof) {
					short nOldStatus = AdoFldShort(prs, "CurrentStatus");
					switch(nOldStatus) {
					case 1:	//Patient
						m_isPatient.SetCheck(1);
						m_isProspect.SetCheck(0);
						m_isPatientProspect.SetCheck(0);
						break;
					case 2:	//Prospect
						m_isPatient.SetCheck(0);
						m_isProspect.SetCheck(1);
						m_isPatientProspect.SetCheck(0);
						break;
					case 3:	//Patient-Prospect
						m_isPatient.SetCheck(0);
						m_isProspect.SetCheck(0);
						m_isPatientProspect.SetCheck(1);
						break;
					}
				}
				prs->Close();
			} NxCatchAll("Error resetting patient status -- please refresh your screen.");
			return;
		}
	}


	// Make sure we have a legitimate status
	long nStatus;
	CString strStatus;
	//TES 8/13/2014 - PLID 63194 - Remember the status to pass to the EX tablechecker
	CClient::PatCombo_StatusType pcstStatusForTC = CClient::pcstUnchanged;
	if (m_isPatient.GetCheck()) {
		// Patient status = 1
		nStatus = 1;
		strStatus = "Patient";
		pcstStatusForTC = CClient::pcstPatient;
	} else if (m_isProspect.GetCheck()) {
		// Prospect status = 2
		nStatus = 2;
		strStatus = "Prospect";
		pcstStatusForTC = CClient::pcstProspect;
	} else if (m_isPatientProspect.GetCheck()) {
		// Patient/Prospect status = 3
		nStatus = 3;
		strStatus = "Patient/Prospect";
		pcstStatusForTC = CClient::pcstPatientProspect;
	} else {
		// None selected
		nStatus = -1;
	}


	// Now commit this info
	if (nStatus != -1) {
		try {


			long nPrvStatus = GetMainFrame()->m_patToolBar.GetExistingPatientStatus(m_id);
		
			CString strPrvStatus;
			if(nPrvStatus == 1){
				strPrvStatus = "Patient";
			} else if (nPrvStatus == 2) {

				strPrvStatus = "Prospect";
				// (a.levy 2012-10-10 11:08) - PLID 49474 -Looks like we have a prospect to patient here ready aim set flag.
                bPatToProspect = true;

			} else if (nPrvStatus == 3) {
				strPrvStatus = "Patient/Prospect";
			} else {
				strPrvStatus = "";
			}					

			//DRT 5/17/2006 - PLID 20680 - Combined these 2 sql statements.
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			CString strSqlBatch = BeginSqlBatch();
			CNxParamSqlArray args;
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @MaxStatusHistoryID int;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "SET @MaxStatusHistoryID = (SELECT COALESCE(Max(ID), 0) FROM PatientStatusHistoryT);\r\n");
			
           
			AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = {INT} WHERE PersonID = {INT};\r\n", nStatus, m_id);
			// add a record of the status change
			AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
				"VALUES (@MaxStatusHistoryID + 1, {INT}, {INT}, {INT}, {STRING}, {STRING});\r\n", m_id, nPrvStatus, nStatus, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
            
			// (a.levy 2012-10-10) - PLID -49474 - Set Client to Default Authorized Version if All Flags fired off. Otherwise abort and continue as normal
			if(bPatToProspect && bIsDefaultVersSet) {

				AddParamStatementToSqlBatch(strSqlBatch, args,"UPDATE NxClientsT SET AuthVersion = {INT} WHERE PersonID = {INT}",nGetVersionID, m_id);

			}
			// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

			//TES 1/6/2010 - PLID 36761 - New function to update the datalist.
			GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(CPatientToolBar::ptbcCurrentStatus,(short)nStatus);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(m_id, false, CClient::pcatUnchanged, pcstStatusForTC);

			// UPDATE THE PALM RECORD
			UpdatePalm();
			UpdateHL7Data();	// (j.armen 2012-06-07 13:10) - PLID 50825 - Update HL7 data as well

			// Only change the color if the above was successful
			//TES 3/22/2010 - PLID 35055 - Don't just set the color on this tab, set it for the whole view.  That way, the patientview's
			// m_nCurrentStatus member will be updated.
			//SetColor(GetNxColor(GNC_PATIENT_STATUS, nStatus));
			// (a.walling 2012-02-16 15:54) - PLID 48210 - If we can't get the view, this means we are initially loading, so ignore it
			// Note that SetColor will get called again anyway after this on the initial load
			if (CPatientView* pView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME)) {
				pView->SetColor();
			}

			//Audit the change
			long nID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nID, aeiCurrentStatus, m_id, strPrvStatus, strStatus, aepMedium, aetChanged);

			//TES 9/21/2010 - PLID 40595 - If they are changing from prospect to non-prospect, then we need to send them over to HL7
			if(nPrvStatus == 2 && nStatus != 2) {
				UpdateHL7Data();
			}

		} NxCatchAll("Could not update patient status!");
	}
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CGeneral1Dlg::Hotkey(int key)
{
	switch(key)
	{	case VK_ESCAPE:
			m_id = GetActivePatientID();
			m_strPatientName = GetExistingPatientName(m_id);
			Load();
			return 0;
	}

	//unhandled
	return 1;
}

BOOL CGeneral1Dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	static CString str;

	switch (HIWORD(wParam))
	{	case EN_CHANGE:
			// (c.haag 2006-04-13 11:04) - PLID 20123 - If we're formatting a field, ignore the change
			if (m_bFormattingField || m_bFormattingAreaCode)
				break;
			switch (nID = LOWORD(wParam))
			{	
				case IDC_MIDDLE_NAME_BOX: 
					// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
					// for middle name boxes
					if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
						//TODO: eventually this will only Capitalize (and simultaneously remove leading spaces)
						//ONLY if the previous value was blank. But for now, let's just do it if it is not loading.
						if(!m_loading)
							Capitalize(nID);
					}
					break;
				case IDC_TITLE_BOX: 
				case IDC_FIRST_NAME_BOX: 
				case IDC_LAST_NAME_BOX: 
				case IDC_ADDRESS1_BOX: 
				case IDC_ADDRESS2_BOX: 
				case IDC_CITY_BOX: 
				case IDC_STATE_BOX: 
				case IDC_MARRIAGE_OTHER_BOX: 
				case IDC_EMERGENCY_FIRST_NAME: 
				case IDC_EMERGENCY_LAST_NAME:
				case IDC_DEAR_BOX:
					//TODO: eventually this will only Capitalize (and simultaneously remove leading spaces)
					//ONLY if the previous value was blank. But for now, let's just do it if it is not loading.
					if(!m_loading)
						Capitalize(nID);
					break;
				case IDC_ZIP_BOX:
					// (d.moore 2007-04-23 12:11) - PLID 23118 - 
					//  Capitalize letters in the zip code as they are typed in. Canadian postal
					//    codes need to be formatted this way.
					CapitalizeAll(IDC_ZIP_BOX);
					GetDlgItemText(nID, str);
					str.TrimRight();
					//if (str != "")
						//FormatItem (nID, "#####-nnnn");
					break;
				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				case IDC_CELL_PHONE_BOX: 
				case IDC_PAGER_PHONE_BOX: 
				case IDC_FAX_PHONE_BOX: 
				case IDC_OTHER_PHONE_BOX: 
				case IDC_EMERGENCY_HOME: 
				case IDC_EMERGENCY_WORK: 
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "" && (GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
						if(m_bFormatPhoneNums) {
							m_bFormattingField = true;
							FormatItem (nID, m_strPhoneFormat);
							m_bFormattingField = false;
						}
					}
					break;	
				case IDC_EXT_PHONE_BOX: 
					//I'm taking this out as part of a broader allow-letters-in-extensions campaign.
					//Also, why didn't you just mark this as "Number" in the resources?  Sigh.
					/*GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "")
						FormatItem (nID, "nnnnnnn");*/
					break;			
				case IDC_SOC_SEC_NO_BOX: 
					// (f.dinatale 2010-10-20) - PLID 33753 - Refactored some code to fix an issue when typing in a SSN that would result in it being entered backwards.
					// Check for permissions to make sure that the OnCommand event handler isn't redundantly formating.
					if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
						m_bFormattingField = true;	
						FormatSSN(SafeGetDlgItem<CEdit>(IDC_SOC_SEC_NO_BOX), "###-##-####");
						m_bFormattingField = false;
					}
					break;
				case IDC_EMAIL_BOX:
					UpdateEmailButton();
			}
			if(!m_loading)
				m_changed = true;
			break;
		case EN_KILLFOCUS:

			/*

			switch ((nID = LOWORD(wParam))) {
				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				case IDC_CELL_PHONE_BOX: 
				case IDC_PAGER_PHONE_BOX: 
				case IDC_FAX_PHONE_BOX: 
				case IDC_OTHER_PHONE_BOX: 
				case IDC_EMERGENCY_HOME: 
				case IDC_EMERGENCY_WORK: 
					if (SaveAreaCode(nID)) {
						Save(LOWORD(wParam));
					}
				break;
				default:
					Save(LOWORD(wParam));
				break;
			}
			*/
			SendMessage(NXM_G1_LOSTFOCUS, LOWORD(wParam), 0);

		break;

		case EN_SETFOCUS:
			
			switch ((nID = LOWORD(wParam))) {
				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				case IDC_CELL_PHONE_BOX: 
				case IDC_PAGER_PHONE_BOX: 
				case IDC_FAX_PHONE_BOX: 
				case IDC_OTHER_PHONE_BOX: 
				case IDC_EMERGENCY_HOME: 
				case IDC_EMERGENCY_WORK: 
					if (ShowAreaCode()) {
						FillAreaCode(nID);
					}
				break;
			}
		break;

	}		
	return CNxDialog::OnCommand(wParam, lParam);
}

void CGeneral1Dlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == IDT_REFRESH)
		Load(false);
	CNxDialog::OnTimer(nIDEvent);
}

// TODO: Support all sources (United, Practice)!
UINT LoadImageAsyncThread(LPVOID p)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// EMRSummaryLoadImageAsyncThread in CEMRSummaryDlg also needs changed

	try {
		// (a.walling 2007-07-20 10:55) - PLID 26762 - We were throwing an exception within this thread, which had no
		// exception handling!
		CGeneral1ImageLoad* pLoadInfo = (CGeneral1ImageLoad*)p;
		HBITMAP hBitmap = NULL;

		if (!p)
			ThrowNxException("Error in LoadImageAsyncThread - The image information is empty");

		// Only load the image if we actually have information to work with
		// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
		// (c.haag 2010-02-22 17:49) - PLID 37364 - We now do this from the Mirror Image Manager object.
		if (NULL != pLoadInfo && NULL != pLoadInfo->m_pMirrorImageMgr) {
			hBitmap = pLoadInfo->m_pMirrorImageMgr->LoadMirrorImage(pLoadInfo->m_nImageIndex, pLoadInfo->m_nImageCount, -1);
			delete pLoadInfo->m_pMirrorImageMgr;
			pLoadInfo->m_pMirrorImageMgr = NULL;
		} else {
			ThrowNxException("Attempted to load a Mirror thumbnail from a null Mirror Image Manager object!");
		}

		// Now tell the window that we are done loading the image
		if (pLoadInfo->m_pMsgWnd->GetSafeHwnd())
			pLoadInfo->m_pMsgWnd->PostMessage(NXM_G1THUMB_IMAGELOADED, (WPARAM)pLoadInfo, (LPARAM)hBitmap);
	} NxCatchAllThread("Error in LoadImageAsyncThread");
	return 0;
}

void CGeneral1Dlg::LoadImageAsync(CGeneral1ImageLoad* pLoadInfo)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	// (c.haag 2009-04-01 17:02) - PLID 33630 - Before we do anything else, try to establish a connection
	// with Mirror. If the result is that the link is being established, put the "Initializing Mirror"
	// image on the thumbnail region, and quit immediately. Otherwise, proceed as before.
	if (DoesCanfieldSDKNeedToInitialize()) {
		return;
	}

	//If we're already loading, add to our list.
	if (m_pLoadImageThread) {
		m_WaitingImages.AddTail(pLoadInfo);
		return;
	}
	
	// Create the thread
	m_pLoadImageThread = AfxBeginThread(LoadImageAsyncThread, pLoadInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!m_pLoadImageThread)
		return;

	// Disable the image navigation windows
	GetDlgItem(IDC_IMAGE_LAST)->EnableWindow(FALSE);
	GetDlgItem(IDC_IMAGE_NEXT)->EnableWindow(FALSE);

	// Execute the thread
	m_pLoadImageThread->m_bAutoDelete = FALSE;
	m_pLoadImageThread->ResumeThread();
}

void CGeneral1Dlg::LoadPatientImage(EImageTraversal dir /*= eImageFwd*/)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	// (c.haag 2009-04-01 17:02) - PLID 33630 - Before we do anything else, try to establish a connection
	// with Mirror. If the result is that the link is being established, put the "Initializing Mirror"
	// image on the thumbnail region, and quit immediately. Otherwise, proceed as before.
	if (DoesCanfieldSDKNeedToInitialize()) {
		return;
	}

	try
	{
		//First of all:
		ShowImagingButtons(0);

		//Here's the plan: there's a bunch of different reasons we might want to hide the arrows.
		//If any of them happen, we'll hide them.  If none of them happen, we'll show them.
		BOOL bShowPrimaryThumbnailOnly = GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0) ? TRUE : FALSE;
		bool bShowArrows = true;
		long nImageCountTotal = 0;
	
		// CAH 6/25/03 - We need to wait for Mirror to open.
		// (c.haag 2009-03-31 14:48) - PLID 33630 - We now use InitCanfieldSDK
		Mirror::InitCanfieldSDK(TRUE);//Mirror::IsMirror61(TRUE);

		// Clear whatever images are there
		/*if (m_imageButton.m_image)
		{	DeleteObject(m_imageButton.m_image);
			m_imageButton.m_image = NULL;
		}*/
		
		// Get the counts of images in our two imaging providers		
		// (c.haag 2006-10-23 12:06) - PLID 23181 - If we are only showing the primary thumbnail, don't bother to run
		// unnecessary Mirror calculations
		// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter from GetImageCount
		// (c.haag 2009-07-07 13:01) - PLID 34379 - Integration with RSI MMS
		long nImageCountMMS = (m_nMMSPatientID == -1) ? 0 : RSIMMSLink::GetImageCount(m_nMMSPatientID);
		long nMMSFirst = 0;
		long nMMSLast = nImageCountMMS - 1;
		//TES 11/17/2009 - PLID 35709 - Check whether they're showing Mirror images in G1
		long nImageCountMirror = (GetPropertyInt("MirrorImageDisplay", 1, 0) && GetPropertyInt("ShowMirrorImagesInG1", GetPropertyInt("PracticeShowImages",1,0),0) && !m_strMirrorID.IsEmpty()) ? GetMirrorImageCount() : 0;
		//TES 2/25/2004: GetImageCount() may return -1 if the patient isn't linked.
		if(nImageCountMirror == -1) nImageCountMirror = 0;
		long nImageCountUnited = (GetPropertyInt("UnitedShowImages", 1, 0) && m_nUnitedID >= 0) ? GetMainFrame()->GetUnitedLink()->GetImageCount(m_nUnitedID) : 0;
		long nImageCountPractice = (GetPropertyInt("PracticeShowImages", 1, 0) ? GetPatientAttachedImageCount(m_id) : 0);
		nImageCountTotal = nImageCountMirror + nImageCountUnited + nImageCountPractice + nImageCountMMS;
		// (c.haag 2010-02-24 10:38) - PLID 37364 - Get the image index limits from the Mirror image manager object
		long nMirrorFirst = (nImageCountMirror > 0 ? GetFirstValidMirrorImageIndex(m_strMirrorID) : 0);
		long nMirrorLast = (nImageCountMirror > 0 ? GetLastValidMirrorImageIndex(m_strMirrorID) : -1);
		long nPracticeFirst = 0;
		long nPracticeLast = nImageCountPractice - 1;


		//////////////////////////////////////////////////////////////
		// If we're just using the primary thumbnail, load it
		if (bShowPrimaryThumbnailOnly)
		{
			//
			// (c.haag 2006-11-07 13:18) - PLID 23181 - Use this flag to determine whether to show a Practice image
			// for the primary thumb. The legacy behavior was to show Mirror images if PracticeShowImages was zero,
			// so we will have to carry on with it.
			//
			// The general logic is:
			//
			// 1. Try to load the Practice primary thumbnail, if it exists
			// 2. Try to load the Mirror primary thumbnail, even if PracticeShowImages is false, if it exists
			//
			BOOL bUsePracticeImages = GetPropertyInt("PracticeShowImages", 1, 0);

			//TES 2/25/2004: Just show the big button.
			ShowImagingButtons(1);

			// (c.haag 2006-11-07 13:20) - PLID 23181 - Try to load the Practice image now
			if (bUsePracticeImages) {
				//JMM 11/04/04 - need to set the source to be Practice 
				m_imageButton.m_source = eImageSrcPractice;  				
				m_imageButton.m_image = LoadPatientAttachedImage(m_id, 0, &m_imageButton.m_progress, m_imageButton.m_strPracticeFileName);
				if (!m_imageButton.m_image)
				{
					// (c.haag 2003-10-01 11:24) - Check to see if we even have a primary image.
					// (j.jones 2008-06-13 11:00) - PLID 30388 - filtered only on photos (or unknown), and parameterized
					//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
					_RecordsetPtr prs = CreateParamRecordset("SELECT PathName "
						"FROM MailSent "
						"INNER JOIN PatientsT ON PatientsT.PatPrimaryHistImage = MailSent.MailID "
						"WHERE PatientsT.PersonID = {INT} "
						"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
						"AND {SQLFRAGMENT}", m_id, GetAllowedCategoryClause_Param("CategoryID"));
					if (!prs->eof)
					{
						// Don't try to load
						// (c.haag 2006-11-07 13:21) - PLID 23181 - Keep bUsePracticeImages TRUE so that
						// the user can see the error. There should have been a Practice image!
						//GetDlgItem(IDC_IMAGE)->ShowWindow(SW_HIDE);
						m_imageButton.m_nError = eErrorUnspecified;
					}
					else
					{
						bUsePracticeImages = FALSE;
					}
				} else {
					// (c.haag 2006-11-07 13:21) - PLID 23181 - We successfully loaded the Practice image.
					// No need to load the Mirror image.
					m_imageButton.m_nError = eNoError;
				}
			}

			if (!bUsePracticeImages) {
				//
				// (c.haag 2006-10-23 11:29) - PLID 23181 - If we get here, try to load the primary Mirror thumbnail
				//
				//TES 11/17/2009 - PLID 35709 - Check whether they want to see Mirror images in G1
				if (nImageCountMirror > 0 && (GetPropertyInt("MirrorImageDisplay", 1, 0) && GetPropertyInt("ShowMirrorImagesInG1", GetPropertyInt("PracticeShowImages",1,0),0) && !m_strMirrorID.IsEmpty())) {
					GetDlgItem(IDC_IMAGE)->ShowWindow(SW_SHOW);
					if(m_imageButton.m_image) {
						DeleteObject(m_imageButton.m_image);
						m_imageButton.m_image = NULL;
					}
					m_imageButton.m_nError = eNoError;
					LoadSingleImage(MIRROR_INDEX_PRIMARY_THUMBNAIL, nImageCountMirror, &m_imageButton, eImageSrcMirror);
					m_imageButton.Invalidate();
				} else {
					// (c.haag 2006-11-07 13:24) - PLID 23181 - This wasn't here before, but we really should hide
					// the imaging button if there is nothing to show.
					ShowImagingButtons(0);
					m_imageButton.m_nError = eNoError;
				}
			}

			GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
			
			// Display whatever new image we have, if any
			bShowArrows = false;
			m_imageButton.Invalidate();
			return;
		} // if (bShowPrimaryThumbnailOnly)

		//////////////////////////////////////////////////////////////
		// Make sure the index is in range
		if (m_nImageIndex < 0) m_nImageIndex = nImageCountTotal-1;
		else if (m_nImageIndex > nImageCountTotal-1) m_nImageIndex = 0;

		if (!nImageCountTotal)
		{
			// This means no pictures were found
			bShowArrows = false;
			ShowImagingButtons(0);
			m_nImageIndex = -1;
		}
		else
		{
		
			long nLastValidIndex = m_nImageIndex;

			////////////////////////////////////////////////////////
			// Load the image
			GetDlgItem(IDC_IMAGE)->ShowWindow(SW_SHOW);

			//TES 2/25/2004: We may be loading multiple images at a time.
			int nImagesToShow = GetRemotePropertyInt("General1ImageCount", 1, 0, GetCurrentUserName(), true);
			if(nImageCountTotal <= nImagesToShow) {
				bShowArrows = false;
				nImagesToShow = nImageCountTotal;
			}
			
			CMirrorImageButton * pFirstImage = NULL;
			CMirrorImageButton * pSecondImage = NULL;
			if(nImagesToShow == 1) {
				pFirstImage = &m_imageButton;
			}
			else if(nImagesToShow ==2) {
				pFirstImage = &m_imageButtonLeft;
				pSecondImage = &m_imageButtonRight;
			}
			else {
				pFirstImage = &m_imageButtonUpperLeft;
				pSecondImage = &m_imageButtonUpperRight;
			}
			ShowImagingButtons(nImagesToShow);


			if (m_nImageIndex >= 0 && m_nImageIndex < nImageCountTotal)
			{
				for(int i = 0; i < nImagesToShow; i++) {
					int nIndex = m_nImageIndex+i >= nImageCountTotal ? m_nImageIndex+i-nImageCountTotal : m_nImageIndex+i;
					CMirrorImageButton *pButton;
					if(i == 0) pButton = pFirstImage;
					else if(i == 1) pButton = pSecondImage;
					else if(i == 2) pButton = &m_imageButtonLowerLeft;
					else if(i == 3) pButton = &m_imageButtonLowerRight;

					//Initialize this button.
					if(pButton->m_image) {
						DeleteObject(pButton->m_image);
						pButton->m_image = NULL;
					}
					pButton->m_nError = eNoError;

					//Now, load this sucker.
					// (c.haag 2009-07-07 13:06) - PLID 34379 - Added support for RSI MMS
					if (nIndex >= 0 && nIndex < nImageCountMirror) {
						// Load from Mirror
						LoadSingleImage(nIndex, nImageCountMirror, pButton, eImageSrcMirror);
					} 
					else if (nIndex >= nImageCountMirror && nIndex < nImageCountMirror + nImageCountUnited) {
						// Load from United
						LoadSingleImage(nIndex - nImageCountMirror, nImageCountUnited, pButton, eImageSrcUnited);
					} 
					else if (nIndex >= nImageCountMirror + nImageCountUnited && nIndex < nImageCountMirror + nImageCountUnited + nImageCountMMS) {
						// Load from MMS
						LoadSingleImage(nIndex - nImageCountMirror - nImageCountUnited, nImageCountMMS, pButton, eImageSrcRSIMMS);
					} 
					else if (nIndex >= nImageCountMirror + nImageCountUnited + nImageCountMMS && nIndex < nImageCountTotal) {
						// Load from NexTech
						LoadSingleImage(nIndex - nImageCountMirror - nImageCountUnited - nImageCountMMS, nImageCountPractice, pButton, eImageSrcPractice);
					}

					// Display whatever new image we have, if any
					pButton->Invalidate();
				}			
			}
		}
			

		// Decide what the prev/next buttons should look like
		if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0))
		{
			bShowArrows = false;
		}
		
		//OK, let's show the arrows as appropriate
		if(bShowArrows) {
			GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_SHOW);
		}
		else {
			GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
		}

/*		if (nIndex == -1 || nImageCountTotal == 0) {
			// No valid images exists so hide all the image-related controls
			GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_IMAGE)->ShowWindow(SW_HIDE);
		}
		// TEMP: Take this out after we allow browsing mirror and united thumbs together
		else if (!nImageCountMirror && !nImageCountPractice)
		{
			GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
		}
		else {
			// Valid images exist, so show controls appropriately
			GetDlgItem(IDC_IMAGE)->ShowWindow(SW_SHOW);
			if (nIndex == nMirrorLast) {
				// We're on our last image so disable the next button
				GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
			} else {
				// We're not on our last image so enable the next button
				GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_SHOW);
			}

			if (nIndex == nMirrorFirst) {
				// We're on our first image so disable the previous button
				GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
			} else {
				// We're not on our first image so disable the previous button
				GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_SHOW);
			}
		}*/
	}
	NxCatchAll("Could not load patient image");
}

void CGeneral1Dlg::OnImageLast() 
{
	CWaitCursor wait;
	try 
	{
		m_nImageIndex--;
		LoadPatientImage(eImageBack);
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PatientsT SET ImageIndex = {INT} WHERE PersonID = {INT}", 
			m_nImageIndex, m_id);
	} NxCatchAll("Error in OnImageLast");
}

void CGeneral1Dlg::OnImageNext() 
{
	CWaitCursor wait;

	try 
	{
		m_nImageIndex++;
		LoadPatientImage();
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PatientsT SET ImageIndex = {INT} WHERE PersonID = {INT}", 
			m_nImageIndex, m_id);
	} NxCatchAll("Error in OnImageNext");
}

// This is declared in history.cpp. Too bad; it seems like
// a nice global function.
CString GetPatientDocumentPath(long nPatientID);

void CGeneral1Dlg::HandleImageClick(CMirrorImageButton *pClicked)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	try {
		if (pClicked->m_nError != eNoError)
		{
			CWnd* pWnd = GetFocus();
			switch(pClicked->m_nError) {
			case eErrorNoPermission:
				MsgBox("You do not have permission to view patient images.  Contact your office manager for assistance.");
				break;
			case eErrorUnspecified:
				switch (pClicked->m_source)
				{
				case eImageSrcMirror:
					Mirror::Troubleshoot();
					break;
				case eImageSrcUnited:
					MsgBox("Practice could not load the patient's United Imaging thumbnail. Please ensure you are properly connected to your United server and that the pictures are accessible over your network.");
					break;
				case eImageSrcPractice:
					MsgBox("Practice could not load the image at %s. Please ensure that this patient's pictures are accessible over your network. You may also go to the History tab and click on 'Open Default Folder' to browse for the picture.",
						pClicked->m_strPracticeFileName);					
					break;		
				case eImageSrcRSIMMS:
					// (c.haag 2009-07-07 13:09) - PLID 34379 - Thumbnail load failure. We store MMS' filename in
					// m_strPracticeFileName out of convenience.
					MsgBox("Practice could not load the RSI Medical Media System patient image at %s. Please ensure you are properly connected to your RSI server and that the pictures are accessible over your network.",
						pClicked->m_strPracticeFileName);
					break;
				default:
					MsgBox("Practice could not load the patient's picture. Please ensure that this patient's pictures are accessible over your network.");
					break;
				}
				break;
			// (c.haag 2009-04-01 17:24) - PLID 33630 - Nothing to do. This is not an interactive mode.
			case eImageBtnInitializingMirror:
				break;
			}
			if (pWnd)
				pWnd->SetFocus();
			return;
		}
		else if (pClicked->m_image == NULL)
		{

			// JMM - 11-04-2004- PLID 14630 - Chris and I decided to take this out because the
			//image isn't showing, so you shouldn't be able to click on it

			/*// (c.haag 2003-07-16 16:13) - If they have mirror permissions and
			// a mirror license, and the patient is linked to mirror, open the
			// chart.
			if (g_userPermission[MirrorIntegration] != 0 && g_pLicense->CheckForLicense(CLicense::lcMirror, CLicense::cflrUse) &&
				!m_strMirrorID.IsEmpty())
			{
				Mirror::Run();
			}*/
			return;
		}
		
		if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0) && pClicked->m_source == eImageSrcPractice) {
			CFile file;
			CString strFilename = pClicked->m_strPracticeFileName;
			
			// Open the image with explorer
			if (!file.Open(strFilename, CFile::modeRead | CFile::shareCompat))
			{
				MsgBox("Could not open image");
				return;
			}
			file.Close();
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32)
			{
				MsgBox("Could not open image");
				return;
			}
			
		}
		else if (pClicked->m_source == eImageSrcMirror) {
			Mirror::Run();
		}
		else if (pClicked->m_source == eImageSrcRSIMMS) {
			// (c.haag 2009-07-07 13:09) - PLID 34379 - Open the image with explorer
			CWaitCursor wc;
			CFile file;
			CString strFilename = pClicked->m_strPracticeFileName; // This is the full path name to the image for MMS
			if (!file.Open(strFilename, CFile::modeRead | CFile::shareCompat))
			{
				MsgBox("Could not open image");
				return;
			}
			file.Close();
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32)
			{
				MsgBox("Could not open image");
				return;
			}
		}
		else if (pClicked->m_source == eImageSrcUnited) {

			// (j.armen 2011-10-25 08:56) - PLID 46136 - Added more exception handling to ensure that the working directory is set correctly
			try
			{
				///////////////////////////////////////////////////////
				// Open united (TODO: Put this in the generic link)
				_ConnectionPtr pConRemote(__uuidof(Connection));
				CUnitedLink *pUnitedLink = GetMainFrame()->GetUnitedLink();
				const CString strExecutePath = GetUnitedExecutePath();
				CString strParams, strID;
				CString strCon, strSQL;
				//char szCurrentDirectory[512];

				// Get the correct path
				// (j.armen 2011-10-25 08:50) - PLID 46136 - We already know what the current directory is supposed to be by using GetPracPath(PracPath:SessionPath), so no need to get it
				//GetCurrentDirectory(512, szCurrentDirectory);

				// Set the path because United needs it that way
				// (c.haag 2006-06-30 12:22) - PLID 21263 - We now use the path
				// to the local United install. I'm leaving the current directory
				// logic as is.
				SetCurrentDirectory(FileUtils::GetFilePath(strExecutePath));
				
				/*
				// Set the external ID to the current time
				strID.Format("%d", time(NULL));
				strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
					(pUnitedLink->GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + pUnitedLink->GetRemotePassword() + ";") : "") +
					"Data Source=" + pUnitedLink->GetRemotePath() + ";";
				strSQL.Format("UPDATE tblPatient SET uExternalID = '%s' WHERE ID = %d",
					strID, m_nUnitedID);

				pConRemote->Open(_bstr_t((LPCTSTR)strCon), "","",NULL);
				pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);
				pConRemote->Close();
				pConRemote.Detach();*/

				
				// Get the active patient ID
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT UserDefinedID FROM PatientsT WHERE PersonID = {INT}",
					m_id);
				strID.Format("%d", AdoFldLong(rs->Fields->Item["UserDefinedID"]));
				rs->Close();

				// Update the ID in United
				strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
					(pUnitedLink->GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + pUnitedLink->GetRemotePassword() + ";") : "") +
					"Data Source=" + pUnitedLink->GetRemotePath() + ";";
				strSQL.Format("UPDATE tblPatient SET uExternalID = '%s' WHERE ID = %d",
					strID, m_nUnitedID);
				pConRemote->Open(_bstr_t((LPCTSTR)strCon), "","",NULL);
				pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);
				pConRemote->Close();
				pConRemote.Detach();

				// Build the actual parameters
				strParams.Format("%s %s", strExecutePath, strID);
				int nReturnCode = WinExec (strParams, SW_SHOW);
				if(nReturnCode != 0 && nReturnCode != 33) {//TES 2/25/2004: It consistently returns 33 for me even after successfully
															//opening.  It's a mystery to me.
					if(nReturnCode == ERROR_FILE_NOT_FOUND || nReturnCode == ERROR_PATH_NOT_FOUND) {
						MsgBox("Failed to open United.  The specified file '%s' could not be found.", strExecutePath);
					}
					else {
						MsgBox("Failed to open United.  Unspecified error.");
					}
				}

			}NxCatchAll("CGeneral1Dlg::HandleImageClick()::United");

			// Set our current directory back to what it was
			// (j.armen 2011-10-25 08:52) - PLID 46136 - Force the path back to the session path
			// moved it outside of exception handling to ensure that if we changed the path and United fails, 
			// we still set the current directory back
			SetCurrentDirectory(GetPracPath(PracPath::SessionPath));
		}
		else if (pClicked->m_source == eImageSrcPractice) {
			CFile file;
			CString strFilename = pClicked->m_strPracticeFileName;
			
			// Open the image with explorer
			if (!file.Open(strFilename, CFile::modeRead | CFile::shareCompat))
			{
				MsgBox("Could not open image");
				return;
			}
			file.Close();

			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32)
			{
				MsgBox("Could not open image");
				return;
			}
		}
	} NxCatchAll("CGeneral1Dlg::HandleImageClick()");
}

void CGeneral1Dlg::OnImage() 
{
	HandleImageClick(&m_imageButton);
}

void CGeneral1Dlg::OnImageLeft()
{
	HandleImageClick(&m_imageButtonLeft);
}

void CGeneral1Dlg::OnImageRight()
{
	HandleImageClick(&m_imageButtonRight);
}

void CGeneral1Dlg::OnImageUpperLeft()
{
	HandleImageClick(&m_imageButtonUpperLeft);
}

void CGeneral1Dlg::OnImageUpperRight()
{
	HandleImageClick(&m_imageButtonUpperRight);
}

void CGeneral1Dlg::OnImageLowerLeft()
{
	HandleImageClick(&m_imageButtonLowerLeft);
}

void CGeneral1Dlg::OnImageLowerRight()
{
	HandleImageClick(&m_imageButtonLowerRight);
}

BOOL CGeneral1Dlg::ChangeCustomLabel (const int nID)
{
	int field = GetLabelFieldID(nID);

	if (field == 0)	//didn't click on a changable label
		return false;
	
	if (!UserPermission(CustomLabel))
		return false;

	CString strResult, strPrompt;
	GetDlgItemText(nID, strPrompt);
	strResult = ConvertFromControlText(strPrompt);

	_variant_t var;
	int nResult;
	
	do {
		nResult = InputBoxLimited(this, "Enter new name for \"" + strPrompt + "\"", strResult, "",50,false,false,NULL);
		
		if (nResult == IDCANCEL) // if they hit cancel, immediately return
			return false;

		strResult.TrimRight();
		strResult.TrimLeft();

		if (strResult == "")
			AfxMessageBox("Please type in a new name for this custom label or press cancel");

		else if(strResult.GetLength() > 50)
			AfxMessageBox("The label name you entered is too long.");
	
	} while ((strResult.GetLength() > 50) | (strResult == ""));

	try {
		// Find the record for that label
		// m.carlson:Feb. 2, 2004	PLID #10737
		if(!IsRecordsetEmpty("SELECT * FROM CustomFieldsT WHERE ID = %d", field)) {
			// Make the data change
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE CustomFieldsT SET Name = {STRING} WHERE ID = {INT}", strResult, field);
		} else {
			// The record wasn't found so insert it into the database
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES ({INT},{STRING},1)",field, strResult);
		}
		// (b.cardillo 2006-05-19 17:28) - PLID 20735 - We know the new name for this label, so 
		// update the global cache so it won't have to be reloaded in its entirety.
		SetCustomFieldNameCachedValue(field, strResult);
		// (b.cardillo 2006-07-06 16:27) - PLID 20737 - Notify all the other users as well.
		CClient::RefreshTable_CustomFieldName(field, strResult);

		SetDlgItemText(nID, ConvertToControlText(strResult));
		// (v.maida 2015-02-20 15:38) - PLID 62118 - Just invalidate the changed control/label, not the entire dialog, so as to not erase other labels.
		GetDlgItem(nID)->Invalidate();

		// (j.jones 2007-08-02 08:38) - PLID 26866 - removed this functionality, deemed too dangerous
		/*
		if (!IsRecordsetEmpty("SELECT FieldID FROM CustomFieldDataT WHERE FieldID = %li",field) &&
			IDYES == AfxMessageBox("Would you like to clear the existing data for ALL patients? Selecting \"No\" will only rename the label and continue to use the existing data.", MB_YESNO))
		{	
			CString str, strSQL;
			strSQL.Format("SELECT PersonID FROM CustomFieldDataT WHERE FieldID = '%d'", field);
			long nRecordCount = GetRecordCount(strSQL);
			if (nRecordCount > 1)
			{
				str.Format("There are %d records of legacy data you will be removing information from by doing this!!! ARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?",
					nRecordCount);
			}
			else
			{
				str = "There is 1 record of legacy data you will be removing information from by doing this!!! ARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?";
			}
			if (IDYES == AfxMessageBox(str, MB_YESNO))
			{	
				ExecuteSql("DELETE FROM CustomFieldDataT WHERE FieldID = %li",field);
			}
			UpdateView();
			// update the palm record
			UpdatePalm();
		}
		*/

	}NxCatchAll("Error in changing custom field. CGeneral1Dlg::ChangeCustomLabel");

	//success!
	return true;
}

void CGeneral1Dlg::OnEmail() 
{
	CString str;
	GetDlgItemText(IDC_EMAIL_BOX, str);
	str.TrimRight();
	str.TrimLeft();
	if (str != "") {
		//DRT 7/30/03 - Added a check for the privacy, they'll be warned if sending when it's up
		if(m_btnEmailPriv.GetCheck()) {
			if(MsgBox(MB_YESNO, "This patient is set for privacy on their email.  Are you sure you wish to send this message?") == IDNO)
				return;
		}

		//DRT 7/14/03 - Added the prompt for a subject.  Once that is entered, we can put a record
		//		in the history that an email was sent. Note that we have no control over any of this, 
		//		so we're only opening the default email client with the given subject.  If they choose
		//		to cancel later, we don't know.
		CString strSubject, strHistory;
		// (j.jones 2006-09-18 12:47) - PLID 22545 - limit the subject text to 255 characters, which is Outlook's maximum
		// and thus, by the commonality of said number, we can presume is the maximum in other email clients
		if(InputBoxLimited(this, "Enter your email subject:", strSubject, "", 255, false, false, "Cancel") == IDOK) {
			//(j.anspach 05-31-2006 13:19 PLID 20272) - Entering an ampersand into the subject line causes
			//  everything beyond it to get cut off.  We need to change any ampersands in the subject into
			//  their hex code %26.... while we're at it, let's make it so they can't accidentally (or
			//  otherwise) put in their own hex codes ...
			//  I also saved strSubject to strHistory before making the changes so that we save it in the history
			//  tab correctly.
			// (a.walling 2006-10-05 13:03) - PLID 22869 - Replaced the ampersand replace with EncodeURL function.
			strHistory = strSubject;
			strSubject = EncodeURL(strSubject);

			//DRT 7/16/03 - More improvements!  There is a preference for what they want to do now.
			//1 = always save in history, 2 = save if strSubject is not blank, 0 = never save
			long nSave = GetRemotePropertyInt("Gen1SaveEmails", 2, 0, "<None>", false);

			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, "mailto:" + str + "?Subject=" + strSubject, 
				NULL, "", SW_MAXIMIZE) < 32) {
				AfxMessageBox("Could not e-mail patient");
			}
			else {
				if(nSave == 1 || (nSave == 2 && !strSubject.IsEmpty())) {
					try {
						//email started successfully
						//enter a note in the history tab
						CString strNote;
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strNote.Format("Email created for '%s' on %s.  Subject:  '%s'", str, FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime, false), strHistory);

						// (j.jones 2008-09-04 15:28) - PLID 30288 - converted to use CreateNewMailSentEntry,
						// which creates the data in one batch and sends a tablechecker
						// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
						// Obviously this will contradict strNote if the server/workstation times are different, but if a new MailSent date is to be "now", then it
						// needs to be the server's "now". If the note becomes an issue, we can address it in a future item.
						CreateNewMailSentEntry(GetActivePatientID(), strNote, "", "", GetCurrentUserName(), "", GetCurrentLocationID());

					} NxCatchAll("Error saving email to history");
				}
				else {
					//nSave is 0, we do not want to save this in the history
				}
			}
		}
	}
	else
		AfxMessageBox("Please enter an e-mail address.");
}

void CGeneral1Dlg::UpdatePalm()
{
	try {
		UpdatePalmSyncTByPerson(m_id);
		PPCRefreshAppts(m_id);
	}NxCatchAll("Error in UpdatingPalm");
}

void CGeneral1Dlg::SaveCustomInfo(int id, CString value)
{
	_RecordsetPtr rsCustom;
	CString sql;
	try {
		value.Replace("#", " "); //take care of bad characters
		value.TrimRight();
		EnsureRemoteData();

		//for auditing
		CString strOld;
		//

		//DRT 11/13/2008 - PLID 32036 - Parameterized
		rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,id);
		if(!rsCustom->eof) {
			strOld = AdoFldString(rsCustom, "TextParam", "");
			rsCustom->Close();
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE CustomFieldDataT SET TextParam = {STRING} WHERE PersonID = {INT} AND FieldID = {INT}", value, m_id, id);
		}
		else {
			rsCustom->Close();
			//check CustomFieldsT
			if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = %li", id))
				//DRT 11/14/2008 - PLID 32036 - Parameterized
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES ({INT}, {STRING}, 1)",id, FormatString("Custom %li", id));

			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("INSERT INTO CustomFieldDataT (PersonID,FieldID,TextParam) VALUES ({INT},{INT},{STRING})",m_id,id,value);
		}

		//DRT 6/30/2005 - PLID 16654 - Auditing
		if(strOld != value) {
			AuditEventItems aei = aeiPatientG1Custom1;	//default in case of bad data will be custom1
			switch(id) {
			case 2:
				aei = aeiPatientG1Custom2;
				break;
			case 3:
				aei = aeiPatientG1Custom3;
				break;
			case 4:
				aei = aeiPatientG1Custom4;
				break;
			}
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aei, m_id, strOld, value, aepLow, aetChanged);
		}

		// (j.jones 2010-05-04 13:47) - PLID 32325 - if OHIP is enabled, see if we changed the health card number,
		// and if so, update the patient toolbar
		if(UseOHIP()) {
			long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
			if(nHealthNumberCustomField == id) {
				GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetOHIPHealthCardColumn(), _bstr_t(value));
			}
		}
		// (j.jones 2010-11-04 16:25) - PLID 39620 - supported this for Alberta as well
		else if(UseAlbertaHLINK()) {
			long nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
			if(nHealthNumberCustomField == id) {
				GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetOHIPHealthCardColumn(), _bstr_t(value));
			}
		}
	}
	NxCatchAll("Error in saving custom data. General1Dlg::SaveCustomInfo");
}
void CGeneral1Dlg::OnUpdateInform() 
{
	if (!CheckCurrentUserPermissions(bioInformIntegration, SPT___W_______))
		return;

	// (j.jones 2011-07-15 12:02) - PLID 42073 - added a usage license call
	if(!g_pLicense->CheckForLicense(CLicense::lcInform, CLicense::cflrUse)) {
		MsgBox("The Inform link is not available without a license.\n"
			"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	if(UpdateInform(m_id)) {
		if(m_dwSendToStatus & TPL_SEND_TO_INFORM) {
			m_dwSendToStatus &= ~TPL_SEND_TO_INFORM;
			m_dwSendToStatus |= TPL_UPDATE_INFORM;
			UpdateSendToThirdPartyButton();
		}
	}
	//if it failed, we won't change the button text from the last status
}


BOOL CGeneral1Dlg::PreTranslateMessage(MSG* pMsg) 
{

	switch(pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		ChangeCustomLabel(::GetDlgCtrlID(pMsg->hwnd));
		return TRUE;
		break;
	case WM_LBUTTONDOWN:
		{
			//TES 2/25/2004: This seems unnecessary to me.  Just use the BN_CLICKED handlers
			/*CPoint pt;
			CRect rc;
			GetCursorPos(&pt);
			GetDlgItem(IDC_IMAGE)->ScreenToClient(&pt);
			GetDlgItem(IDC_IMAGE)->GetClientRect(&rc);
			if (rc.PtInRect(pt))
			{
				OnImage();
				return TRUE;
			}*/
		}
		break;
	case WM_RBUTTONDOWN:
		{
			OnRightClickImage(pMsg->hwnd, pMsg->wParam, pMsg->lParam);
		}
	break;	

	default:
		break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

int CGeneral1Dlg::GetLabelFieldID(int nID)
{
	int field = 0;

	switch(nID)
	{
	case IDC_CUSTOM1_LABEL:
		field = 1;
		break;
	case IDC_CUSTOM2_LABEL:
		field = 2;
		break;
	case IDC_CUSTOM3_LABEL:
		field = 3;
		break;
	case IDC_CUSTOM4_LABEL:
		field = 4;
		break;
	default:
		field = 0;
		break;
	}

	return field;

}

void CGeneral1Dlg::OnSingleRad() 
{
	if (m_loading) return;
	try {

		CString strOld = "";
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT MaritalStatus FROM PatientsT WHERE PersonID = {INT}", m_id);		
		if(!rs->eof && rs->Fields->Item["MaritalStatus"]->Value.vt == VT_BSTR) {
			CString strStatus = CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal);
			if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "1"))
				strOld = "Single";
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "2"))
				strOld = "Married";
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "3"))
				strOld = "Other";
		}

		//DRT 11/14/2008 - PLID 32036 - Parameterized - I set the maritalStatus bit to a parameter so that this function can
		//	use the same parameter query as OnMarriedRad and OnOtherRad
		ExecuteParamSql("UPDATE PatientsT SET MaritalStatus = {INT} WHERE PersonID = {INT}", 1, m_id);
		if(!IsReproductive()) {
			SetDlgItemText(IDC_PARTNER_LABEL,"");
			CRect rc;
			GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);
			m_MarriageOther.ShowWindow(SW_HIDE);
		}

		if(strOld != "Single") {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientMaritalStatus, m_id, strOld, "Single", aepMedium, aetChanged);
		}

		// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
		UpdateHL7Data();

		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in Updating Marital Status");	
}

void CGeneral1Dlg::OnMarriedRad() 
{
	if (m_loading) return;
	try {

		CString strOld = "";
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT MaritalStatus FROM PatientsT WHERE PersonID = {INT}", m_id);		
		if(!rs->eof && rs->Fields->Item["MaritalStatus"]->Value.vt == VT_BSTR) {
			CString strStatus = CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal);
			if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "1"))
				strOld = "Single";
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "2"))
				strOld = "Married";
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "3"))
				strOld = "Other";
		}

		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PatientsT SET MaritalStatus = {INT} WHERE PersonID = {INT}", 2, m_id);
		if(!IsReproductive()) {
			SetDlgItemText(IDC_PARTNER_LABEL,"Spouse");
			CRect rc;
			GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);
			m_MarriageOther.ShowWindow(SW_SHOW);
		}

		if(strOld != "Married") {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientMaritalStatus, m_id, strOld, "Married", aepMedium, aetChanged);
		}

		// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
		UpdateHL7Data();

		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in Updating Marital Status");
}

void CGeneral1Dlg::OnOtherRad() 
{
	if (m_loading) return;
	try {

		CString strOld = "";
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT MaritalStatus FROM PatientsT WHERE PersonID = {INT}", m_id);		
		if(!rs->eof && rs->Fields->Item["MaritalStatus"]->Value.vt == VT_BSTR) {
			CString strStatus = CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal);
			if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "1"))
				strOld = "Single";
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "2"))
				strOld = "Married";
			else if (!strcmp(CString(rs->Fields->Item["MaritalStatus"]->Value.bstrVal), "3"))
				strOld = "Other";
		}

		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PatientsT SET MaritalStatus = {INT} WHERE PersonID = {INT}", 3, m_id);
		if(!IsReproductive()) {
			SetDlgItemText(IDC_PARTNER_LABEL,"Status");
			CRect rc;
			GetDlgItem(IDC_PARTNER_LABEL)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);
			m_MarriageOther.ShowWindow(SW_SHOW);
		}

		if(strOld != "Other") {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientMaritalStatus, m_id, strOld, "Other", aepMedium, aetChanged);
		}

		// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
		UpdateHL7Data();

		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in Updating Marital Status");
}
/* DRT 10/22/01 - Removed in favor of a combo box which contains an option for no gender!
void CGeneral1Dlg::OnFemaleRad() 
{
	if (m_loading) return;
	try {
		ExecuteSql("UPDATE PersonT SET Gender = 2 WHERE ID = %li",m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in Updating Gender");
}

void CGeneral1Dlg::OnMaleRad() 
{
	if (m_loading) return;
	try {
		ExecuteSql("UPDATE PersonT SET Gender = 1 WHERE ID = %li",m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in Updating Gender");
}
*/
void CGeneral1Dlg::OnUpdateMirror() 
{
	CString mfirst, mlast, nfirst, nlast;

	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, go to Module=>Link to Mirror, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}
	if (!CheckCurrentUserPermissions(bioMirrorIntegration, SPT_______1___))
		return;

	// (c.haag 2004-06-03 09:37) - PLID 12811 - If the patient is already linked, warn them about
	// the overwrite setting if necessary
	//DRT 11/13/2008 - PLID 32036 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset("SELECT MirrorID FROM PatientsT WHERE PersonID = {INT} AND MirrorID IS NOT NULL AND Len(MirrorID) > 0", m_id);
	if (!prs->eof)
	{
		if (!GetRemotePropertyInt("MirrorExportOverwrite", 1))
		{
			if (IDYES == MsgBox(MB_YESNO, "You currently have the Mirror link configured to not export demographics to existing Canfield Mirror patients. Would you like to re-activate this ability?\n\n"
				"If you click 'Yes', the link will be reconfigured to allow Practice to write demographics to the Mirror database, and this patient will be updated.\n"
				"If you click 'No', the update will be cancelled."))
			{
				SetRemotePropertyInt("MirrorExportOverwrite", 1);
			}
			else
			{
				return;
			}
		}
	}

	try 
	{
		if (Mirror::GetMirrorName(m_strMirrorID, mfirst, mlast))
		{	GetDlgItemText(IDC_FIRST_NAME_BOX, nfirst);
			GetDlgItemText(IDC_LAST_NAME_BOX, nlast);
			//if (mlast != nlast || mfirst != nfirst)
			if (mlast.CompareNoCase(nlast) || mfirst.CompareNoCase(nfirst))
			{
				CString msg;
				msg.Format("Are you sure you want to overwrite the Mirror record "
					"\"%s %s\" with the Nextech record \"%s %s\"?", 
					mfirst, mlast, nfirst, nlast);
				if (IDYES != AfxMessageBox(msg, MB_YESNO))
					return;
			}
		}
		else if (!m_strMirrorID.IsEmpty())
		{
			MsgBox("The mirror patient could not be updated because it does not exist.");
			return;
		}

		long result = Mirror::Export(GetDlgItemInt(IDC_ID_BOX),m_strMirrorID);
		if (result == Mirror::Addnew || result == Mirror::Update) 
		{
			UpdateView();
		}
		//if it failed, we won't change the button text from the last status
	}
	NxCatchAll("Error in updating Mirror");
}

void CGeneral1Dlg::OnUpdateUnited()
{
	CString mfirst, mlast, nfirst, nlast;
	if (!CheckCurrentUserPermissions(bioUnitedIntegration, SPT_______1___))
		return;

	// (j.jones 2011-07-15 12:02) - PLID 42073 - added a usage license call
	if(!g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrUse)) {
		MsgBox("The United link is not available without a license.\n"
			"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	try 
	{
		if (!UpdateUnited(m_id)) {
			UpdateView();
		}
	}
	NxCatchAll("Error in updating United");
}

void CGeneral1Dlg::UpdateEmailButton(){
	CString strEmail;
	GetDlgItemText(IDC_EMAIL_BOX, strEmail);
	if(strEmail != "")
		GetDlgItem(IDC_EMAIL)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_EMAIL)->EnableWindow(FALSE);
}

void CGeneral1Dlg::OnKillfocusNotes() 
{
	//TS 7/5/01:  This is now in OnCommand, where it should have been from the beginning.
	//ForceFieldLength((CNxEdit *)GetDlgItem(IDC_NOTES));
}

void CGeneral1Dlg::OnRightClickImage(HWND hWndClicked, UINT nFlags, CPoint pt)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	CMenu popup;

	//Figure out which button they right-clicked on.
	if(m_imageButton.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButton;
	else if(m_imageButtonLeft.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButtonLeft;
	else if(m_imageButtonRight.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButtonRight;
	else if(m_imageButtonUpperLeft.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButtonUpperLeft;
	else if(m_imageButtonUpperRight.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButtonUpperRight;
	else if(m_imageButtonLowerLeft.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButtonLowerLeft;
	else if(m_imageButtonLowerRight.GetSafeHwnd() == hWndClicked) m_RightClicked = &m_imageButtonLowerRight;
	else return;
	if (!m_RightClicked->m_image || m_RightClicked->m_nError != eNoError)
		return;

	//CPoint point(pts.x, pts.y);

	m_RightClicked->ClientToScreen(&pt);

	popup.CreatePopupMenu();
	popup.AppendMenu (MF_ENABLED, IDM_COPY_IMAGE, "Copy");
	popup.AppendMenu (MF_ENABLED, IDM_VIEW_IMAGE, "View");
	popup.TrackPopupMenu (TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON, 
		pt.x, pt.y, this);
}

LRESULT CGeneral1Dlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	//TES 2/25/2004: Moved to PreTranslateMessage.
	//if (message == NXM_RBUTTONCLK)
	//	OnRightClickImage(wParam, MAKEPOINTS(lParam));
		
	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CGeneral1Dlg::OnViewImage()
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	if(!m_RightClicked) return;
	CImageDlg dlg(this);
	CString first, last, middle;

	GetDlgItemText(IDC_FIRST_NAME_BOX, first);
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);
	GetDlgItemText(IDC_LAST_NAME_BOX, last);

	dlg.DoModal(m_RightClicked->m_image, last + ", " + first +  " " + middle);
}

void CGeneral1Dlg::OnCopyImage()
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	HBITMAP hBmp;
	try
	{
		if (!m_RightClicked->m_image || m_RightClicked->m_nError != eNoError)
		{
			MsgBox("Invalid image handle");
			return;
		}
		hBmp = CopyBitmap(m_RightClicked->m_image);
		if (!hBmp)
			AfxThrowNxException("Could not duplicate image");

		if (!OpenClipboard())
			AfxThrowNxException("Could not open clipboard");
	
		if (!EmptyClipboard())
			AfxThrowNxException("Could not clear clipboard");

		if (!SetClipboardData(CF_BITMAP, hBmp))
			AfxThrowNxException("Could not set bitmap to clipboard");

		if (!CloseClipboard())
			AfxThrowNxException("Could not close clipboard");
	}
	NxCatchAll("Could not copy image");
}

void CGeneral1Dlg::OnSelChosenPartnerList(long nRow) 
{
	try {

		if(IsReproductive()) {

			if(nRow == -1)
				return;

			BOOL bIsPartner = FALSE;

			long PatientID = m_id;
			long PartnerID = -1;
			long OldPartnerID = -1;
			_variant_t var = m_PartnerCombo->GetValue(nRow,0);

			if(var.vt == VT_I4)
				PartnerID = var.lVal;

			//first get the old partner ID, if it exists
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT PartnerID FROM PartnerLinkT WHERE PatientID = {INT} AND InactiveDate Is Null",PatientID);
			if(!rs->eof) {
				OldPartnerID = rs->Fields->Item["PartnerID"]->Value.lVal;
			}
			rs->Close();

			//now check and see if we're changing the link at all			
			if(PartnerID == OldPartnerID)
				return;

			//see if this patient is a partner, and then get the OldPartnerID
			
			//JJ - we do this AFTER the above statement, because selecting the same partner for a 'patient' does
			//not change the link. However, changing this on the 'partner' screen can possibly shift the roles
			//of this couple. We need to get the OldPartnerID incase they decide to change their minds and not alter
			//the link, then we can re-set the datalist to show the right name.
			if(OldPartnerID == -1) {
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				rs = CreateParamRecordset("SELECT PatientID FROM PartnerLinkT WHERE PartnerID = {INT} AND InactiveDate Is Null",PatientID);
				if(!rs->eof) {
					OldPartnerID = rs->Fields->Item["PatientID"]->Value.lVal;
					//we want to know later on if this patient is the partner
					bIsPartner = TRUE;
				}
				rs->Close();
			}		

			//it's very inefficient to remove the current patient from the list every time
			//we switch patients, so the current patient stays in the list. Don't let them
			//select the current patient!
			if(PartnerID == PatientID) {
				AfxMessageBox("You cannot select the current patient.");
				if (OldPartnerID > 0)
					m_PartnerCombo->TrySetSelByColumn(0,_variant_t(OldPartnerID));
				else
					m_PartnerCombo->TrySetSelByColumn(0,(long)0);
				return;
			}

			//otherwise, we are changing the link in some way
		
			
			//see if they might just be swapping roles
			//DRT 11/13/2008 - PLID 32036 - Parameterized
			rs = CreateParamRecordset("SELECT ID FROM PartnerLinkT WHERE PartnerID = {INT} AND PatientID = {INT} AND InactiveDate Is Null",PatientID,PartnerID);
			if(!rs->eof) {
				//they are indeed switching roles
				if(IDYES==MessageBox("This action will switch the roles of this patient and partner.\nAre you sure you wish to do this?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					//DRT 11/14/2008 - PLID 32036 - Cleaned up this mess and parameterized it.
					CString strSqlBatch = BeginSqlBatch();
					CNxParamSqlArray args;

					//switch the link
					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @MaxStatusHistoryID int;\r\n");
					AddStatementToSqlBatch(strSqlBatch, "SET @MaxStatusHistoryID = (SELECT COALESCE(Max(ID), 0) FROM PatientStatusHistoryT);\r\n");
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PartnerLinkT SET PatientID = {INT}, PartnerID = {INT} WHERE ID = {INT}",PatientID,PartnerID,rs->Fields->Item["ID"]->Value.lVal);
					
					//update statuses
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = 1 WHERE PersonID = {INT}",PatientID);
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = 3 WHERE PersonID = {INT}",PartnerID);
					// add a record of the status change for the patient
					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
						"VALUES (@MaxStatusHistoryID + 1, {INT}, 3, 1, {STRING}, {STRING})", PatientID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
					// add a record of the status change for the partner
					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
						"VALUES (@MaxStatusHistoryID + 2, {INT}, 1, 3, {STRING}, {STRING})", PartnerID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
					// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
					ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

					//update colors and info
					m_isPatient.SetCheck(TRUE);
					m_isProspect.SetCheck(FALSE);
					m_isPatientProspect.SetCheck(FALSE);
					//TES 1/6/2010 - PLID 36761 - New functions to update the datalist.
					GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(CPatientToolBar::ptbcCurrentStatus,(short)1);
					GetMainFrame()->m_patToolBar.SetValueByPersonID(PartnerID,CPatientToolBar::ptbcCurrentStatus,(short)3);
					CClient::RefreshTable(NetUtils::PatCombo, PatientID);
					CClient::RefreshTable(NetUtils::PatCombo, PartnerID);

					// UPDATE THE PALM RECORD
					UpdatePalm();
					SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
				}
				else {
					m_PartnerCombo->TrySetSelByColumn(0,_variant_t(OldPartnerID));
				}

				return;					
			}
			rs->Close();
			
			//okay, this means they are really changing the partner

			if(PartnerID <= 0) {
				//we're de-activating ('deleting') a record
				//DRT 11/13/2008 - PLID 32036 - Parameterized
				rs = CreateParamRecordset("SELECT PartnerLinkT.ID, PatientName.Last AS PatientLast, PatientName.First AS PatientFirst, PatientName.Middle AS PatientMiddle, "
								"PartnerName.Last AS PartnerLast, PartnerName.First AS PartnerFirst, PartnerName.Middle AS PartnerMiddle "
								"FROM PartnerLinkT INNER JOIN PersonT AS PatientName ON PartnerLinkT.PatientID = PatientName.ID INNER JOIN PersonT AS PartnerName ON PartnerLinkT.PartnerID = PartnerName.ID "
								"WHERE (PatientID = {INT} OR PartnerID = {INT}) AND InactiveDate Is Null",PatientID,PatientID);
			}
			else {
				//we're adding a new record, or changing the link

				//DRT 11/13/2008 - PLID 32036 - Parameterized
				rs = CreateParamRecordset("SELECT PartnerLinkT.ID, PatientName.Last AS PatientLast, PatientName.First AS PatientFirst, PatientName.Middle AS PatientMiddle, "
								"PartnerName.Last AS PartnerLast, PartnerName.First AS PartnerFirst, PartnerName.Middle AS PartnerMiddle "
								"FROM PartnerLinkT INNER JOIN PersonT AS PatientName ON PartnerLinkT.PatientID = PatientName.ID INNER JOIN PersonT AS PartnerName ON PartnerLinkT.PartnerID = PartnerName.ID "
								"WHERE InactiveDate Is Null AND ((PatientID = {INT} AND PartnerID <> {INT}) OR (PartnerID = {INT} AND PatientID <> {INT}))",PatientID,PartnerID,PatientID,PartnerID);
			}

			CString PatientName, PartnerName;
			long ID;

			if(!rs->eof) {

				ID = rs->Fields->Item["ID"]->Value.lVal;
				PatientName = CString(rs->Fields->Item["PatientLast"]->Value.bstrVal) + ", " + CString(rs->Fields->Item["PatientFirst"]->Value.bstrVal) + " " + CString(rs->Fields->Item["PatientMiddle"]->Value.bstrVal);
				PartnerName = CString(rs->Fields->Item["PartnerLast"]->Value.bstrVal) + ", " + CString(rs->Fields->Item["PartnerFirst"]->Value.bstrVal) + " " + CString(rs->Fields->Item["PartnerMiddle"]->Value.bstrVal);

				rs->Close();

				CString str;

				str.Format("This action will remove the link between patient '%s' and partner '%s'.\n\nAre you SURE you wish to do this?",PatientName,PartnerName);

				if(IDYES==MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					//DRT 11/14/2008 - PLID 32036 - Parameterized
					ExecuteParamSql("UPDATE PartnerLinkT SET InactiveDate = GetDate() WHERE ID = {INT}",ID);
				}
				else {					
					m_PartnerCombo->TrySetSelByColumn(0,_variant_t(OldPartnerID));
					return;
				}
			}

			if(PartnerID > 0) {

				BOOL bMakePartner = FALSE;

				//JJ - 9/4/2003 - by default we made the functionality be so that when you chose a partner,
				//the person in which you were currently on would become the "patient". However, if they are
				//currently the "partner" then we should prompt them to find out what role they should have.
				//(Note: if they select the same partner, with the intent to reverse the roles, that is handled
				//before this point.

				if(bIsPartner && IDNO == MessageBox("By default, this 'partner' will become the 'patient' in the new relationship.\n"
					"Are you sure you would like to switch this person's role?\n\n"
					"Click 'Yes' to make this person be the 'patient',\n"
					"Click 'No' to keep this person as the 'partner' and make their newly selected partner be the 'patient'.","Practice",MB_YESNO|MB_ICONQUESTION)) {
					bMakePartner = TRUE;
				}

				//we're adding a new link
				ID = NewNumber("PartnerLinkT","ID");
				if(!bMakePartner) {
					//DRT 11/14/2008 - PLID 32036 - Cleaned up and parameterized
					CString strSqlBatch = BeginSqlBatch();
					CNxParamSqlArray args;

					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @MaxStatusHistoryID int;\r\n");
					AddStatementToSqlBatch(strSqlBatch, "SET @MaxStatusHistoryID = (SELECT COALESCE(Max(ID), 0) FROM PatientStatusHistoryT);\r\n");

					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PartnerLinkT (ID,PatientID,PartnerID) VALUES ({INT},{INT},{INT})",ID,PatientID,PartnerID);
					//update statuses
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = 1 WHERE PersonID = {INT}",PatientID);
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = 3 WHERE PersonID = {INT}",PartnerID);
					// add a record of the status change
					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
						"VALUES (@MaxStatusHistoryID + 1, {INT}, 3, 1, {STRING}, {STRING})", PatientID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
					// add a record of the status change
					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
						"VALUES (@MaxStatusHistoryID + 2, {INT}, 1, 3, {STRING}, {STRING})", PartnerID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
					// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
					ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

					//update colors and info
					m_isPatient.SetCheck(TRUE);
					m_isProspect.SetCheck(FALSE);
					m_isPatientProspect.SetCheck(FALSE);
					//TES 1/6/2010 - PLID 36761 - New functions to update thedatalist.
					GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(CPatientToolBar::ptbcCurrentStatus,(short)1);
					GetMainFrame()->m_patToolBar.SetValueByPersonID(PartnerID,CPatientToolBar::ptbcCurrentStatus,(short)3);
					SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
				}
				else {
					//DRT 11/14/2008 - PLID 32036 - Cleaned up and parameterized
					CString strSqlBatch = BeginSqlBatch();
					CNxParamSqlArray args;

					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @MaxStatusHistoryID int;\r\n");
					AddStatementToSqlBatch(strSqlBatch, "SET @MaxStatusHistoryID = (SELECT COALESCE(Max(ID), 0) FROM PatientStatusHistoryT);\r\n");

					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PartnerLinkT (ID,PatientID,PartnerID) VALUES ({INT},{INT},{INT})",ID,PartnerID,PatientID);
					//update statuses
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = 1 WHERE PersonID = {INT}",PartnerID);
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET CurrentStatus = 3 WHERE PersonID = {INT}",PatientID);
					// add a record of the status change
					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
						"VALUES (@MaxStatusHistoryID + 1, {INT}, 3, 1, {STRING}, {STRING})", PatientID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
					// add a record of the status change
					AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientStatusHistoryT (ID, PersonID, OldStatus, NewStatus, DateConverted, ConvertedByUserName) "
						"VALUES (@MaxStatusHistoryID + 2, {INT}, 1, 3, {STRING}, {STRING})", PartnerID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), GetCurrentUserName());
					// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
					ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

					//update colors and info
					m_isPatient.SetCheck(FALSE);
					m_isProspect.SetCheck(FALSE);
					m_isPatientProspect.SetCheck(TRUE);
					//TES 1/6/2010 - PLID 36761 - New functions to update the datalist.
					GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(CPatientToolBar::ptbcCurrentStatus,(short)3);
					GetMainFrame()->m_patToolBar.SetValueByPersonID((long)PartnerID,CPatientToolBar::ptbcCurrentStatus,(short)1);
					SetColor(GetNxColor(GNC_PATIENT_STATUS, 3));
				}
				
				CClient::RefreshTable(NetUtils::PatCombo, PatientID);
				CClient::RefreshTable(NetUtils::PatCombo, PartnerID);

				// UPDATE THE PALM RECORD
				UpdatePalm();				
			}
		}

	}NxCatchAll("Error saving partner.");
}

void CGeneral1Dlg::OnClosedUpGenderList(long nSelRow) 
{
	//DRT 5/29/03 - Moved to OnSelChosenGenderList
}

BOOL CGeneral1Dlg::UserCanChangeName()
{
	if (m_loading) return TRUE;

	// The MessageBox takes focus away from General 1 into oblivion, so we need
	// a way to get it back.
	CMaintainFocus maintainFocus; // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus

	//DRT 7/22/02
	//if for some reason we can't get a focus (user switched to another program instead of tabbing out of this)
	//then just set it to the first name
	if(!maintainFocus)
		maintainFocus.Reset(GetDlgItem(IDC_FIRST_NAME_BOX));

	if (!CheckCurrentUserPermissions(bioPatientName, SPT___W_______))
	{
		return FALSE;
	}
	// (a.wetta 2007-03-20 10:20) - PLID 24983 - If we're processing a driver's license swipe, we don't need to prompt them again about the change
	else if (!m_bProcessingCard && IDNO == MessageBox("Are you absolutely sure you want to change the name of this patient?", NULL, MB_YESNO))
	{
		return FALSE;
	}

	return TRUE;
}

void CGeneral1Dlg::OnPartnerBtn() 
{
	try {

		long nRow = m_PartnerCombo->GetCurSel();

		if(nRow == -1)
			return;

		if(IsReproductive()) {

			_variant_t var = m_PartnerCombo->GetValue(nRow,0);
			if(var.vt == VT_I4) {
				long PatID = var.lVal;
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(GetMainFrame()->m_patToolBar.TrySetActivePatientID(PatID)) {
					GetMainFrame()->UpdateAllViews();
				}
			}
		}

	}NxCatchAll("Error switching to partner.");	
}



void CGeneral1Dlg::FillAreaCode(long nID)  {

	
		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 16:06) - PLID 35825 - use city if prefered
			CString strAreaCode, strZip, strCity;
			GetDlgItemText(IDC_ZIP_BOX, strZip);
			GetDlgItemText(IDC_CITY_BOX, strCity);			
			bool bResult = FALSE;
			if (!m_bLookupByCity) {
				bResult = GetZipInfo(strZip, NULL, NULL, &strAreaCode);
			}
			else {
				bResult = GetCityInfo(strCity, NULL, NULL, &strAreaCode);
			}
			if (bResult) {

				m_bFormattingAreaCode = true;
				SetDlgItemText(nID, strAreaCode);
				CString str = strAreaCode;
				str.TrimRight();
				if (str != "" && (GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
					if(m_bFormatPhoneNums) {
						m_bFormattingField = true;
						FormatItem (nID, m_strPhoneFormat);
						m_bFormattingField = false;
					}
				}
				m_bFormattingAreaCode = false;
				
				//set the member variable 
				FormatText(strAreaCode, m_strAreaCode, m_strPhoneFormat);


				//set the cursor
				::PostMessage(GetDlgItem(nID)->GetSafeHwnd(), EM_SETSEL, 5, 5);
			}
			else {
				//set the member variable to be blank
				m_strAreaCode = "";
			}
	
			
	  	}
		else {

			//set the member variable to be blank
			m_strAreaCode = "";
		}

}

bool CGeneral1Dlg::SaveAreaCode(long nID) {

	//is the member variable empty
	if (m_strAreaCode.IsEmpty() ) {
		//default to returning true becuase just becauase we didn't do anything with the areacode, doesn't mean they didn't change the number
		return true;
	}
	else {
		//check to see if that is the only thing that is in the box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (strPhone == m_strAreaCode) {
			//if they are equal then erase the area code
			m_bFormattingAreaCode = true;
			SetDlgItemText(nID, "");
			m_bFormattingAreaCode = false;
			return false;
		}
		else {
			return true;
		}

	}
	//set out member variable to blank
	m_strAreaCode = "";

}

void CGeneral1Dlg::OnGroups() 
{
	// (r.gonet 2010-08-31) - PLID 39939 - Added a try-catch handler to OnGroups
	try {
		if (!CheckCurrentUserPermissions(bioLWGroup, sptWrite))
			return;

		CPatientGroupsDlg dlg(this);
		dlg.m_bAutoWriteToData = true;
		dlg.m_nPatID = m_id;
		dlg.DoModal();
		GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();

		// (r.gonet 2010-08-31) - PLID 39939 - The patient's membership might have changed, update the color of the Groups button
		UpdateGroupsButton();
	} NxCatchAll(__FUNCTION__);
}

void CGeneral1Dlg::OnSelChangeFaxChoice() {
	
	//shouldn't get here anyways, but just in case
	if(!IsNexTechInternal())
		return;
	
	//update PatientsT.FaxChoice
	try {
		long nChoice;
		CComboBox* fax;
		fax = (CComboBox*)GetDlgItem(IDC_FAX_CHOICE);

		nChoice = fax->GetCurSel();

		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PatientsT SET FaxChoice = {INT} WHERE PersonID = {INT}", nChoice, m_id);

	} NxCatchAll("Error saving fax choice");
}


void CGeneral1Dlg::SecureControls()
{
	CWnd* pWnd;
	int i;


	// Secure the patient ID box
	if (!(GetCurrentUserPermissions(bioPatientID) & (SPT___W________ANDPASS)))
	{
		((CNxEdit*)GetDlgItem(IDC_ID_BOX))->SetReadOnly(TRUE);
	}

	if (!(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS)))
	{
		GetDlgItem(IDC_GROUPS)->EnableWindow(FALSE);
	}

	if (!(GetCurrentUserPermissions(bioPatientName) & (SPT___W________ANDPASS)))
	{
		((CNxEdit*)GetDlgItem(IDC_FIRST_NAME_BOX))->SetReadOnly(TRUE);
		((CNxEdit*)GetDlgItem(IDC_MIDDLE_NAME_BOX))->SetReadOnly(TRUE);
		((CNxEdit*)GetDlgItem(IDC_LAST_NAME_BOX))->SetReadOnly(TRUE);
	}

	//only disable it if they can't write to it
	if ((GetCurrentUserPermissions(bioPatientCoordinatorGen1) & (SPT___W________ANDPASS))) {
		//disable the patient coordinator list
		GetDlgItem(IDC_COORD_COMBO)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_COORD_COMBO)->EnableWindow(FALSE);
	}
	
	//Secure the provider dropdown.
	if ((GetCurrentUserPermissions(bioPatientProvider) & (SPT___W________ANDPASS))) {
		//disable the provider list
		GetDlgItem(IDC_DOCTOR_COMBO)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_DOCTOR_COMBO)->EnableWindow(FALSE);
	}

	// (j.gruber 2007-08-08 12:17) - PLID 25045 - set address, phone, and email fields appropriately
	if (!(GetCurrentUserPermissions(bioPatientAddress) & (SPT___W_______))  || 
	    !(GetCurrentUserPermissions(bioPatientAddress) & (SPT__R________))) { 
		//gray out the boxes
		GetDlgItem(IDC_ADDRESS1_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADDRESS2_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_CITY_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATE_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_ZIP_BOX)->EnableWindow(FALSE);

	}

	if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT___W_______))  || 
	    !(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) { 
		//gray out the boxes
		GetDlgItem(IDC_HOME_PHONE_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_WORK_PHONE_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_CELL_PHONE_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_OTHER_PHONE_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_PAGER_PHONE_BOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_FAX_PHONE_BOX)->EnableWindow(FALSE);

	}

	
	if (!(GetCurrentUserPermissions(bioPatientEmail) & (SPT___W_______))  || 
	    !(GetCurrentUserPermissions(bioPatientEmail) & (SPT__R________))) { 
		//gray out the boxes
		GetDlgItem(IDC_EMAIL_BOX)->EnableWindow(FALSE);
	}

	//TES 1/5/2010 - PLID 35774 - Secure the Security button
	// (j.gruber 2010-10-26 14:07) - PLID 40416 - split the permission to assign patients 
	if ((GetCurrentUserPermissions(bioSecurityGroup) & (SPT___W________ANDPASS)) ||
		(GetCurrentUserPermissions(bioSecurityGroup) & (SPT______0_____ANDPASS))
		) {
		GetDlgItem(IDC_EDIT_SECURITY_GROUPS)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_EDIT_SECURITY_GROUPS)->EnableWindow(FALSE);
	}

	//(e.lally 2010-05-10) PLID 36631 - First contact date permission
	if((GetCurrentUserPermissions(bioPatientFirstContactDate) & sptWrite)){
		GetDlgItem(IDC_CONTACT_DATE)->EnableWindow(TRUE);
	}
	else{
		GetDlgItem(IDC_CONTACT_DATE)->EnableWindow(FALSE);
	}

	//(a.wilson 2012-3-23) PLID 48472 - check to ensure they have permission to access recall system.
	// (j.armen 2012-03-28 10:55) - PLID 48480 - Check for the license.  Enable/Disable based on License and Permission
	m_btnRecall.EnableWindow(
		g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent) &&
		(GetCurrentUserPermissions(bioRecallSystem) & (sptRead | sptReadWithPass))
		? TRUE : FALSE);

	// (f.dinatale 2010-10-20) - PLID 33753 - Relocated the code which disables the SSN editbox.
	// (f.dinatale 2011-02-21) - PLID 33753 - Added another case to the if statement for when the Read permission is 
	// disabled and Disable Masking is enabled (Don't care condition, so just disable it.)
	if((!CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE))
		|| (CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE))
		|| (!CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE))) {
			((CNxEdit*)GetDlgItem(IDC_SOC_SEC_NO_BOX))->EnableWindow(FALSE);
			((CNxEdit*)GetDlgItem(IDC_SOC_SEC_NO_BOX))->SetReadOnly(TRUE);
	}

	// Return if we have write access
	if (GetCurrentUserPermissions(bioPatient) & (SPT___W________ANDPASS))
		return;

	// No write access. Traverse the controls to disable all edit boxes
	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT)) 
	{
		if (IsEditBox(pWnd) && pWnd->GetDlgCtrlID() != IDC_ID_BOX
			&& pWnd->GetDlgCtrlID() != IDC_FIRST_NAME_BOX
			&& pWnd->GetDlgCtrlID() != IDC_MIDDLE_NAME_BOX
			&& pWnd->GetDlgCtrlID() != IDC_LAST_NAME_BOX
			&& pWnd->GetDlgCtrlID() != IDC_ADDRESS1_BOX
			&& pWnd->GetDlgCtrlID() != IDC_ADDRESS2_BOX
			&& pWnd->GetDlgCtrlID() != IDC_CITY_BOX
			&& pWnd->GetDlgCtrlID() != IDC_STATE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_ZIP_BOX
			&& pWnd->GetDlgCtrlID() != IDC_HOME_PHONE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_WORK_PHONE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_CELL_PHONE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_OTHER_PHONE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_PAGER_PHONE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_FAX_PHONE_BOX
			&& pWnd->GetDlgCtrlID() != IDC_EMAIL_BOX)
		{
			((CNxEdit*)pWnd)->SetReadOnly(TRUE);
		}
	}

	// Disable date controls
	GetDlgItem(IDC_BIRTH_DATE_BOX)->EnableWindow(FALSE);

	// Disable certain buttons
	GetDlgItem(IDC_GROUPS)->EnableWindow(FALSE);
	if (GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->GetSafeHwnd())
		GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(FALSE);

	// Disable combos
	GetDlgItem(IDC_PREFIX_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_DOCTOR_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_PARTNER_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_GENDER_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_PREFERRED_CONTACT_LIST)->EnableWindow(FALSE);

	// Disable radio buttons and checkboxes
	GetDlgItem(IDC_IS_PATIENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_IS_PROSPECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_IS_PATIENT_PROSPECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_SINGLE_RAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_MARRIED_RAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_OTHER_RAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_INACTIVE_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_FOREIGN_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_HOME_PRIV_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_WORK_PRIV_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_EXCLUDE_MAILINGS)->EnableWindow(FALSE);

	// v.arth 06/24/2009 PLID 34386 - Disable country drop down list
	GetDlgItem(IDC_COUNTRY_LIST)->EnableWindow(FALSE);
}

BOOL CGeneral1Dlg::IsEditBox(CWnd* pWnd)
{
	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_ID_BOX:
		case IDC_TITLE_BOX: 
		case IDC_FIRST_NAME_BOX: 
		case IDC_MIDDLE_NAME_BOX: 
		case IDC_LAST_NAME_BOX: 
		case IDC_ADDRESS1_BOX: 
		case IDC_ADDRESS2_BOX: 
		case IDC_CITY_BOX: 
		case IDC_STATE_BOX: 
		case IDC_MARRIAGE_OTHER_BOX: 
		case IDC_EMERGENCY_FIRST_NAME: 
		case IDC_EMERGENCY_LAST_NAME:
		case IDC_DEAR_BOX:
		case IDC_ZIP_BOX:
		case IDC_HOME_PHONE_BOX: 
		case IDC_WORK_PHONE_BOX: 
		case IDC_CELL_PHONE_BOX: 
		case IDC_PAGER_PHONE_BOX: 
		case IDC_FAX_PHONE_BOX: 
		case IDC_OTHER_PHONE_BOX: 
		case IDC_EMERGENCY_HOME: 
		case IDC_EMERGENCY_WORK: 
		case IDC_EXT_PHONE_BOX: 
		case IDC_SOC_SEC_NO_BOX: 
		case IDC_BIRTH_DATE_BOX: 
		case IDC_CONTACT_DATE: 
		case IDC_EMERGENCY_RELATE:
		case IDC_CUSTOM1_BOX:
		case IDC_CUSTOM2_BOX:
		case IDC_CUSTOM3_BOX:
		case IDC_CUSTOM4_BOX:
		case IDC_NOTES:
		case IDC_EMAIL_BOX:
			return TRUE;
	}
	return FALSE;
}

// For use with JPEG decompression
// (a.walling 2008-04-10 12:52) - PuntoEXE image processing no longer used
/*
LRESULT CGeneral1Dlg::OnImageProcessingCompleted(WPARAM wParam, LPARAM lParam)
{
	CImageCodecManager* pCodecManager = (CImageCodecManager*)lParam;
	CFile* pFile = (CFile*)wParam;

	m_imageButton.m_progress.ShowWindow(SW_HIDE);
	m_imageButton.RedrawWindow();

	/////////////////////////////////////////////////////////////////
	// Load the image
	CImageBuffer* pCodecBuffer = pCodecManager->RemoveOutputImage(0);
	CSize size = pCodecBuffer->GetSize();
	CRect rcImg(0,0,size.cx,size.cy);

	/////////////////////////////////////////////////////////////////
	// Draw the image
	static CDC dcMem;
	HBITMAP hbmpDC = CreateCompatibleBitmap(GetMainFrame()->GetDC()->m_hDC, size.cx, size.cy);
	if (dcMem.m_hDC == NULL)
		dcMem.CreateCompatibleDC(NULL);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(dcMem.m_hDC, hbmpDC);

	CImageDrawProcessor drawProcessor;
	drawProcessor.DeclareInputImage(pCodecBuffer);			
	drawProcessor.Setup(&dcMem, &rcImg);
	if (!drawProcessor.ProcessorFunction())
	{
		SelectObject(dcMem.m_hDC, hbmpOld);

		// Draw the picture
		m_imageButton.m_image = hbmpDC;
		m_imageButton.Invalidate();
	}
	delete pCodecBuffer;
	delete pCodecManager;
	delete pFile;

	// TODO: Allow users to browse pictures without having
	// to wait for the current picture to load!
	GetDlgItem(IDC_IMAGE_LAST)->EnableWindow(TRUE);
	GetDlgItem(IDC_IMAGE_NEXT)->EnableWindow(TRUE);
	return 0;
}
*/

// For use with loading Mirror images
LRESULT CGeneral1Dlg::OnImageLoaded(WPARAM wParam, LPARAM lParam)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	CGeneral1ImageLoad* pLoadInfo = (CGeneral1ImageLoad*)wParam;
	HBITMAP hBitmap = (HBITMAP)lParam;
	BOOL bReload = FALSE;

	// Quit right away if we are being destroyed
	if (!GetSafeHwnd())
	{
		delete pLoadInfo;
		return 0;
	}

	if (pLoadInfo->m_nPatientID != m_id)
		bReload = TRUE;

	// If we loaded the image and did not change the patient since starting
	// the load, then draw the image.
	if (!bReload)
	{
		pLoadInfo->m_pButton->m_image = hBitmap;
		if (hBitmap == NULL)
			pLoadInfo->m_pButton->m_nError = eErrorUnspecified;
		pLoadInfo->m_pButton->Invalidate();
		GetDlgItem(IDC_IMAGE_LAST)->EnableWindow(TRUE);
		GetDlgItem(IDC_IMAGE_NEXT)->EnableWindow(TRUE);
	}

	// Destroy our thread object
	if (m_pLoadImageThread)
	{
		// Wait for the thread to terminate
		WaitForSingleObject(m_pLoadImageThread->m_hThread, 10000);

		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pLoadImageThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so post a quit message to it and let it delete itself
			// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
			m_pLoadImageThread->m_bAutoDelete = TRUE;
			PostThreadMessage(m_pLoadImageThread->m_nThreadID, WM_QUIT, 0, 0);
		} else {
			// The thread is finished, so just delete it
			delete m_pLoadImageThread;
		}
		m_pLoadImageThread = NULL;

		//Do we have another one waiting?
		CGeneral1ImageLoad *pNextOne = m_WaitingImages.GetCount() == 0 ? NULL : (CGeneral1ImageLoad*)m_WaitingImages.RemoveHead();
		if(pNextOne) {
			LoadImageAsync(pNextOne);
		}
	}

	// If the user went to another patient, load the thumbnail again
	if (bReload)
		LoadPatientImage();

	delete pLoadInfo;
	return 0;
}

long CGeneral1Dlg::GetLinks() {

	//returns -1 for none, 0 for multiple, 1 for Inform, 2 for Mirror, 3 for United, 4 for Quickbooks, and 5 for CareCredit.

	long Links = HAS_NO_LINKS;

	if(m_dwSendToStatus == 0)
		return HAS_NO_LINKS;
	
	BOOL bInform = FALSE, bMirror = FALSE, bUnited = FALSE, bQuickbooks = FALSE, bCareCredit = FALSE, bDevices = FALSE;

	if((m_dwSendToStatus & TPL_SEND_TO_INFORM) || (m_dwSendToStatus & TPL_UPDATE_INFORM))
		bInform = TRUE;
	
	if((m_dwSendToStatus & TPL_SEND_TO_MIRROR) || (m_dwSendToStatus & TPL_UPDATE_MIRROR))
		bMirror = TRUE;

	if((m_dwSendToStatus & TPL_SEND_TO_UNITED) || (m_dwSendToStatus & TPL_UPDATE_UNITED))
		bUnited = TRUE;

	if((m_dwSendToStatus & TPL_SEND_TO_QBOOKS) || (m_dwSendToStatus & TPL_UPDATE_QBOOKS))
		bQuickbooks = TRUE;

	if((m_dwSendToStatus & TPL_SEND_TO_CARECREDIT)) {
		bCareCredit = TRUE;
	}

	// (d.lange 2010-06-30 15:38) - PLID 38687 - Determine whether we have enabled device plugins that support the 'LaunchDevice' functionality
	// (j.gruber 2013-04-22 11:49) - PLID 56361 - fix this to use the variable
	if ((m_dwSendToStatus & TPL_HAS_LAUNCH_DEVICE)) {
		bDevices = TRUE;
	}	

	if(bInform && !bMirror && !bUnited && !bQuickbooks && !bCareCredit && !bDevices) {
		return HAS_ONLY_INFORM;
	}
	else if(!bInform && bMirror && !bUnited && !bQuickbooks && !bCareCredit && !bDevices) {
		return HAS_ONLY_MIRROR;
	}
	else if(!bInform && !bMirror && bUnited && !bQuickbooks && !bCareCredit && !bDevices) {
		return HAS_ONLY_UNITED;
	}
	else if(!bInform && !bMirror && !bUnited && bQuickbooks && !bCareCredit && !bDevices) {
		return HAS_ONLY_QBOOKS;
	}
	else if(!bInform && !bMirror && !bUnited && !bQuickbooks && bCareCredit && !bDevices) {
		return HAS_ONLY_CARECREDIT;
	}
	// (d.lange 2010-10-22 16:51) - PLID 41082 - bCareCredit was not negated so if both CareCredit and Devices were enabled it would return only devices
	else if(!bInform && !bMirror && !bUnited && !bQuickbooks && !bCareCredit && bDevices) {
		return HAS_ONLY_DEVICE;
	}
	else {
		return HAS_MULTIPLE_LINKS;
	}

}

void CGeneral1Dlg::UpdateSendToThirdPartyButton()
{
	//this function simply decides what to label the button, not the contents of the menu
	
	long Links = GetLinks();

	switch(Links) {

		case HAS_ONLY_INFORM:
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_SHOW);
			if(m_dwSendToStatus & TPL_SEND_TO_INFORM)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Send To Inform");
			else if(m_dwSendToStatus & TPL_UPDATE_INFORM)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Update Inform");

			if(m_dwSendToStatus & TPL_DISABLE_INFORM)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(FALSE);
			else
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(TRUE);
			break;		

		case HAS_ONLY_MIRROR:
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_SHOW);
			if(m_dwSendToStatus & TPL_SEND_TO_MIRROR)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Send To Mirror");
			else if(m_dwSendToStatus & TPL_UPDATE_MIRROR)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Update Mirror");

			if(m_dwSendToStatus & TPL_DISABLE_MIRROR)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(FALSE);
			else
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(TRUE);
			break;

		case HAS_ONLY_UNITED:
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_SHOW);
			if(m_dwSendToStatus & TPL_SEND_TO_UNITED)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Send To United");
			else if(m_dwSendToStatus & TPL_UPDATE_UNITED)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Update United");

			if(m_dwSendToStatus & TPL_DISABLE_UNITED)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(FALSE);
			else
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(TRUE);
			break;

		case HAS_ONLY_QBOOKS:
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_SHOW);
			if(m_dwSendToStatus & TPL_SEND_TO_QBOOKS)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Send To Quickbooks");
			else if(m_dwSendToStatus & TPL_UPDATE_QBOOKS)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Update Quickbooks");

			if(m_dwSendToStatus & TPL_DISABLE_QBOOKS)
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(FALSE);
			else
				GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(TRUE);
			break;
			
		case HAS_ONLY_CARECREDIT:
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_SHOW);
			ASSERT(m_dwSendToStatus == TPL_SEND_TO_CARECREDIT);
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("CareCredit");
			break;
		
		case HAS_MULTIPLE_LINKS:			
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Send To ...");		
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(TRUE);
			break;

		// (d.lange 2010-10-22 16:48) - PLID 41082 - When all other links are disabled, but devices are enabled display the button and label it "Send To..."
		case HAS_ONLY_DEVICE:
			// (j.gruber 2013-04-24 10:14) - PLID 56361
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->SetWindowText("Send To ...");		
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->EnableWindow(TRUE);
			break;

		case HAS_NO_LINKS:
		default:
			GetDlgItem(IDC_SEND_TO_THIRD_PARTY)->ShowWindow(SW_HIDE);
			break;
	}
}

void CGeneral1Dlg::OnSendToThirdParty() 
{
	long Links = GetLinks();
	switch(Links) {
		case HAS_ONLY_INFORM:
			OnUpdateInform();
			break;
		case HAS_ONLY_MIRROR:
			OnUpdateMirror();
			break;
		case HAS_ONLY_UNITED:
			OnUpdateUnited();
			break;
		case HAS_ONLY_QBOOKS:
			OnUpdateQBooks();
			break;
		case HAS_ONLY_CARECREDIT:
			OnLinkToCareCredit();
			break;
		case HAS_ONLY_DEVICE:			// (d.lange 2010-06-30 15:32) - PLID 38687 - Handle if devices are only available
			GenerateDeviceMenu();
			break;
		case HAS_MULTIPLE_LINKS:
			GenerateLinkMenu();
			break;
		case HAS_NO_LINKS:
		default:
			break;
	}
	GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();
}

void CGeneral1Dlg::GenerateLinkMenu() {

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();
	long nIndex = 0;

	BOOL bDisabled = FALSE;

	// (z.manning, 01/02/2007) - PLID 24056 - Added option for CareCredit link.
	if(NxCareCredit::GetCareCreditLicenseStatus() != cclsExpired) {
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_LINK_TO_CARECREDIT, "&CareCredit...");
	}

	int nFlags = MF_BYPOSITION;
	if(m_dwSendToStatus & TPL_DISABLE_MIRROR)
		nFlags |= MF_DISABLED | MF_GRAYED;
	
	if(m_dwSendToStatus & TPL_UPDATE_MIRROR)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_MIRROR, "Update &Mirror");
	else if(m_dwSendToStatus & TPL_SEND_TO_MIRROR)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_MIRROR, "Send To &Mirror");

	nFlags = MF_BYPOSITION;
	if(m_dwSendToStatus & TPL_DISABLE_INFORM)
		nFlags |= MF_DISABLED | MF_GRAYED;

	if(m_dwSendToStatus & TPL_UPDATE_INFORM)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_INFORM, "Update &Inform");
	else if(m_dwSendToStatus & TPL_SEND_TO_INFORM)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_INFORM, "Send To &Inform");

	nFlags = MF_BYPOSITION;
	if(m_dwSendToStatus & TPL_DISABLE_UNITED)
		nFlags |= MF_DISABLED | MF_GRAYED;

	if(m_dwSendToStatus & TPL_UPDATE_UNITED)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_UNITED, "Update &United");
	else if(m_dwSendToStatus & TPL_SEND_TO_UNITED)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_UNITED, "Send To &United");

	nFlags = MF_BYPOSITION;
	if(m_dwSendToStatus & TPL_DISABLE_QBOOKS)
		nFlags |= MF_DISABLED | MF_GRAYED;

	if(m_dwSendToStatus & TPL_UPDATE_QBOOKS)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_QBOOKS, "Update &Quickbooks");
	else if(m_dwSendToStatus & TPL_SEND_TO_QBOOKS)
		mnu.InsertMenu(nIndex++, nFlags, ID_UPDATE_QBOOKS, "Send To &Quickbooks");

	// (d.lange 2010-06-22 16:21) - PLID 38687 - Set up a menu item for sending patient demographics to an external device
	//Determine all the device plugins that are enabled
	// (c.haag 2010-06-30 15:27) - PLID 39424 - Added license checking
	// (j.gruber 2013-04-02 13:03) - PLID 56012 - Consolidate this
	CArray<DeviceLaunchUtils::DevicePlugin*, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;
	DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, &mnu, nIndex, TRUE, nFlags);

	mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);

	if(g_pLicense->CheckForLicense(CLicense::lcMirror, CLicense::cflrSilent))
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_LINK_TO_MIRROR, "Open Mi&rror Link...");

	if(g_pLicense->CheckForLicense(CLicense::lcInform, CLicense::cflrSilent))
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_LINK_TO_INFORM, "Open In&form Link...");

	if(g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrSilent))
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_LINK_TO_UNITED, "Open U&nited Link...");

	if(g_pLicense->GetHasQuickBooks())
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_LINK_TO_QUICKBOOKS, "Open Quick&books Link...");

	// (d.lange 2010-06-30 14:32) - PLID 38687 - Since I've added menu items for device plugins, I'm using the pointer as their menu IDs so I've
	// added the flag TPM_RETURNCMD to TrackPopupMenu. As a result I had to check for each menu item and call their associated event handler.
	CRect rc;
	CWnd *pWnd = GetDlgItem(IDC_SEND_TO_THIRD_PARTY);
	long nResult = 0;
	if (pWnd) {
		pWnd->GetWindowRect(&rc);
		nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
	} else {
		CPoint pt;
		GetCursorPos(&pt);
		nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
	}

	if(nResult == ID_LINK_TO_CARECREDIT) {
		OnLinkToCareCredit();

	}else if(nResult == ID_UPDATE_MIRROR) {
		OnUpdateMirror();

	}else if(nResult == ID_UPDATE_INFORM) {
		OnUpdateInform();

	}else if(nResult == ID_UPDATE_UNITED) {
		OnUpdateUnited();

	}else if(nResult == ID_UPDATE_QBOOKS) {
		OnUpdateQBooks();

	}else if(nResult == ID_LINK_TO_MIRROR) {
		OnLinkToMirror();

	}else if(nResult == ID_LINK_TO_INFORM) {
		OnLinkToInform();

	}else if(nResult == ID_LINK_TO_UNITED) {
		OnLinkToUnited();

	}else if(nResult == ID_LINK_TO_QUICKBOOKS) {
		OnLinkToQuickbooks();

	}else if(nResult > 0) {
		// (d.lange 2010-06-29) - PLID 38687 - the returned menu item is the memory location for the plugin
		// (j.gruber 2013-04-02 13:06) - PLID 56012 - consolidate
		DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nResult, GetActivePatientID());		
	}

	//Let's iterate through the array of loaded plugins and unload them
	// (j.gruber 2013-04-02 13:07) - PLID 56012 - consolidate
	DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);	
}

void CGeneral1Dlg::OnUpdateQBooks()
{
	if(!CheckCurrentUserPermissions(bioQuickbooksLink,sptView))
		return;

	try 
	{
		// Open the qb connection
		IQBSessionManagerPtr qb = QB_OpenSession();

		if(qb == NULL)
			return;

		// See if the customer exists
		
		CString strCustomerListID, strEditSequence = "";
		if (!QB_GetCustomerListID(qb, m_id, strCustomerListID, strEditSequence)) {
			// Customer doesn't exist, so create it
			if(QB_CreateCustomer(qb, m_id, strCustomerListID)) {
				//DRT 11/14/2008 - PLID 32036 - Parameterized
				ExecuteParamSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = {INT}",m_id);
				if(m_dwSendToStatus & TPL_SEND_TO_QBOOKS) {
					m_dwSendToStatus &= ~TPL_SEND_TO_QBOOKS;
					m_dwSendToStatus |= TPL_UPDATE_QBOOKS;
					UpdateSendToThirdPartyButton();
				}
			}
		}
		else {
			// Customer exists, so update based on the ListID, also update the status for accuracy
			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = {INT}",m_id);
			if(QB_UpdateCustomer(qb, m_id, strCustomerListID, strEditSequence)) {
				if(m_dwSendToStatus & TPL_SEND_TO_QBOOKS) {
					m_dwSendToStatus &= ~TPL_SEND_TO_QBOOKS;
					m_dwSendToStatus |= TPL_UPDATE_QBOOKS;
					UpdateSendToThirdPartyButton();
				}
			}
		}

		qb->EndSession();
	}
	NxCatchAll("Error in updating Quickbooks");
}

void CGeneral1Dlg::OnLinkToInform()
{
	if (UserPermission(InformIntegration))
	{	CInformLink dlg;
		dlg.DoModal();
	}
}

void CGeneral1Dlg::OnLinkToMirror()
{
	if (UserPermission(MirrorIntegration))
	{
		CMirrorLink *pMirror = GetMainFrame()->GetMirrorLink();
		if (pMirror) {
			pMirror->DoFakeModal();
		}
	}
}

void CGeneral1Dlg::OnLinkToUnited()
{
	if (UserPermission(UnitedIntegration))
	{
		CUnitedLink *pUnited = GetMainFrame()->GetUnitedLink();
		if (pUnited) {
			pUnited->OpenDlg();
		}
	}
}

void CGeneral1Dlg::OnLinkToQuickbooks()
{
	if(CheckCurrentUserPermissions(bioQuickbooksLink,sptView))
	{
		CQuickbooksLink dlg(this);
		dlg.DoModal();
	}
}

void CGeneral1Dlg::OnLinkToCareCredit()
{
	// (z.manning, 01/03/2007) - PLID 24078 - Open up the CCWare program.
	try {

		NxCareCredit::OpenCCWare(GetActivePatientID());

	}NxCatchAll("CGeneral1Dlg::OnLinkToCareCredit");
}

void CGeneral1Dlg::OnSelChosenPreferredContactList(long nRow) 
{
	// (a.walling 2006-10-05 14:33) - PLID 22841 - Fix the no selection exception
	if (m_loading || (nRow == sriNoRow)) return;
	try {
		//for auditing
		CString strOld;
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr prsAudit = CreateParamRecordset("SELECT PreferredContact FROM PatientsT WHERE PersonID = {INT}", m_id);
		if(!prsAudit->eof) {
			long nContact = AdoFldLong(prsAudit, "PreferredContact", -1);

			strOld = GetPreferredContactTypeStringFromID(nContact);
		}
		//

		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PatientsT SET PreferredContact = {INT} WHERE PersonID = {INT}",nRow,m_id);

		//for auditing
		CString strNew = GetPreferredContactTypeStringFromID(nRow);
		//

		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		if(strOld != strNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrefContact, m_id, strOld, strNew, aepLow, aetChanged);
			// (s.tullis 2016-01-18 11:36) - PLID 67929 - When you change the preferred contact type, we don't initiate an HL7 update.
			UpdateHL7Data();
		}

	} NxCatchAll("Error changing preferred contact method.");
}

void CGeneral1Dlg::OnDestroy() 
{
	//Destroy any waiting images.
	while(m_WaitingImages.GetCount()) {
		delete (CGeneral1ImageLoad*)m_WaitingImages.RemoveHead();
	}
	// Destroy our thread object
	if (m_pLoadImageThread)
	{
		// Wait for the thread to terminate
		WaitForSingleObject(m_pLoadImageThread->m_hThread, 2000);

		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pLoadImageThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so post a quit message to it and let it delete itself
			// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
			m_pLoadImageThread->m_bAutoDelete = TRUE;
			PostThreadMessage(m_pLoadImageThread->m_nThreadID, WM_QUIT, 0, 0);
		} else {
			// The thread is finished, so just delete it
			delete m_pLoadImageThread;
		}
		m_pLoadImageThread = NULL;
			
	}

	// (c.haag 2010-02-23 09:55) - PLID 37364 - Delete the Mirror image manager if it exists
	EnsureNotMirrorImageMgr();

	CNxDialog::OnDestroy();	
}

LRESULT CGeneral1Dlg::OnLostFocus(WPARAM wParam, LPARAM lParam)
{
	long nID;

	// wParam = The control ID
	switch ((nID = wParam)) {
		case IDC_HOME_PHONE_BOX: 
		case IDC_WORK_PHONE_BOX: 
		case IDC_CELL_PHONE_BOX: 
		case IDC_PAGER_PHONE_BOX: 
		case IDC_FAX_PHONE_BOX: 
		case IDC_OTHER_PHONE_BOX: 
		case IDC_EMERGENCY_HOME: 
		case IDC_EMERGENCY_WORK: 
			if (SaveAreaCode(nID)) {
				Save(LOWORD(wParam));
			}
		break;
		default:
			Save(LOWORD(wParam));
		break;
	}
	return 0;
}

void CGeneral1Dlg::OnExcludeMailings() 
{
	if (m_loading) return;
	try {
		long nStatus;
		
		if(m_ExcludeMailingsCheck.GetCheck() == 0){
			nStatus = 0;
		}
		else{
			nStatus = 1;
		}
		
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET ExcludeFromMailings = {INT} WHERE ID = {INT}", nStatus ,m_id);

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientExcludeMailing, m_id, nStatus == 1 ? "No" : "Yes", nStatus == 1 ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In OnExcludeMailings");
}

void CGeneral1Dlg::OnRequeryFinishedPrefixCombo(short nFlags) 
{
	IRowSettingsPtr pRow = m_PrefixCombo->GetRow(-1);
	pRow->PutValue(0, (long)0);
	pRow->PutValue(1, _bstr_t(""));
	pRow->PutValue(2, (long)0);
	m_PrefixCombo->InsertRow(pRow, 0);
}

void CGeneral1Dlg::OnSelChosenPrefixCombo(long nRow) 
{
	int nAuditID;
	_variant_t var;
	CString strOldGender;

	if (m_loading) return;
	try {
		if(nRow != -1) {
			CString strPrefix;
			strPrefix.Format("%li",VarLong(m_PrefixCombo->GetValue(nRow, 0)));
			if(!ReturnsRecords("SELECT ID FROM PrefixT WHERE ID = %s", strPrefix)) {
				if(strPrefix != "0") {
					MsgBox("The prefix you selected cannot be found.  It may have been deleted by another user on the network.");
					m_PrefixCombo->RemoveRow(nRow);
					m_PrefixCombo->CurSel = -1;
				}
				strPrefix = "NULL";
			}
			ExecuteSql("UPDATE PersonT SET PrefixID = %s WHERE ID = %li",strPrefix,m_id);

			//check the preference first
			if(strPrefix != "NULL" && GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 1) {
				//we need to update the gender based on the selection
				long nGender = VarLong(m_PrefixCombo->GetValue(nRow,2));
				if(nGender == 1){
					m_GenderCombo->PutCurSel(1);
					
					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT Gender FROM PersonT WHERE ID = {INT}", m_id);
					_variant_t var = rs->Fields->Item["Gender"]->Value;

					if (var.iVal != nGender)
					{
						if (var.iVal == 1) strOldGender = "Male";
						else if (var.iVal == 2) strOldGender = "Female";

						nAuditID = BeginNewAuditEvent();
						AuditEvent(m_id, m_strPatientName,nAuditID,aeiPatientGender,m_id,strOldGender,"Male",aepMedium,aetChanged);

						//DRT 11/14/2008 - PLID 32036 - Parameterized
						ExecuteParamSql("UPDATE PersonT SET Gender = {INT} WHERE ID = {INT}", 1, m_id);
				
						// (b.cardillo 2009-05-27 23:11) - PLID 34368 - This patient's gender just changed, so update the qualification records.
						UpdatePatientWellnessQualification_Gender(GetRemoteData(), m_id);

						//TES 11/15/2013 - PLID 59533 - Check CDS rules
						CDWordArray arNewCDSInterventions;
						UpdateDecisionRules(GetRemoteData(), m_id, arNewCDSInterventions);
						GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
					}
				}
				else if (nGender == 2){
					m_GenderCombo->PutCurSel(2);

					//DRT 11/13/2008 - PLID 32036 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT Gender FROM PersonT WHERE ID = {INT}", m_id);
					_variant_t var = rs->Fields->Item["Gender"]->Value;

					if (var.iVal != nGender)
					{
						if (var.iVal == 1) strOldGender = "Male";
						else if (var.iVal == 2) strOldGender = "Female";

						nAuditID = BeginNewAuditEvent();
						AuditEvent(m_id, m_strPatientName,nAuditID,aeiPatientGender,m_id,strOldGender,"Female",aepMedium,aetChanged);

						//DRT 11/14/2008 - PLID 32036 - Parameterized
						ExecuteParamSql("UPDATE PersonT SET Gender = {INT} WHERE ID = {INT}", 2, m_id);

						// (b.cardillo 2009-05-27 23:11) - PLID 34368 - This patient's gender just changed, so update the qualification records.
						UpdatePatientWellnessQualification_Gender(GetRemoteData(), m_id);

						//TES 11/15/2013 - PLID 59533 - Check CDS rules
						CDWordArray arNewCDSInterventions;
						UpdateDecisionRules(GetRemoteData(), m_id, arNewCDSInterventions);
						GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
					}
				}
			}

			// (j.gruber 2007-08-27 10:19) - PLID 24628 - Update HL7
			UpdateHL7Data();

			// UPDATE THE PALM RECORD
			UpdatePalm();
		}
	} NxCatchAll("Error in Changing Prefix");

}

void CGeneral1Dlg::OnSelChosenGenderList(long nRow) 
{
	if (m_loading) return;

	int nAuditID;
	CString strOldGender,strNewGender;

	try {
		
		if(nRow == sriNoRow){
			// if nothing is selected, set the selection to the blank line
			m_GenderCombo->PutCurSel(0);
			nRow = 0;
		}
		
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Gender FROM PersonT WHERE ID = {INT}", m_id);
		_variant_t var = rs->Fields->Item["Gender"]->Value;

		if (var.iVal != nRow)
		{
			if (var.iVal == 1) strOldGender = "Male";
			else if (var.iVal == 2) strOldGender = "Female";

			if (nRow == 1) strNewGender = "Male";
			else if (nRow == 2) strNewGender = "Female";

			nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName,nAuditID,aeiPatientGender,m_id,strOldGender,strNewGender,aepMedium,aetChanged);

			//DRT 11/14/2008 - PLID 32036 - Parameterized
			ExecuteParamSql("UPDATE PersonT SET Gender = {INT} WHERE ID = {INT}", nRow, m_id);

			// (b.cardillo 2009-05-27 23:11) - PLID 34368 - This patient's gender just changed, so update the qualification records.
			UpdatePatientWellnessQualification_Gender(GetRemoteData(), m_id);

			// (a.walling 2007-10-19 11:29) - PLID 27820 - Send a table checker when the gender changes
			CClient::RefreshTable(NetUtils::PatG1, m_id);
		}

		//check the preference first
		if(GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 1) {
			long nCurrentPrefixGender = m_PrefixCombo->CurSel == -1 ? -1 : VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->CurSel, 2), -1);
			if(nCurrentPrefixGender != 0) {
				if(nRow == 1 && nCurrentPrefixGender != 1) {
					//male
					long nNewPrefix = GetRemotePropertyInt("DefaultMalePrefix", 1, 0, "<None>", true);
					if(ReturnsRecords("SELECT ID FROM PrefixT WHERE ID = %li", nNewPrefix)) {
						m_PrefixCombo->SetSelByColumn(0, nNewPrefix);
						//DRT 11/14/2008 - PLID 32036 - Parameterized
						ExecuteParamSql("UPDATE PersonT SET PrefixID = {INT} WHERE ID = {INT}", nNewPrefix, m_id);
					}
				}
				else if(nRow == 2 && nCurrentPrefixGender != 2) {
					//female
					long nNewPrefix = GetRemotePropertyInt("DefaultFemalePrefix", 1, 0, "<None>", true);
					if(ReturnsRecords("SELECT ID FROM PrefixT WHERE ID = %li", nNewPrefix)) {
						m_PrefixCombo->SetSelByColumn(0, nNewPrefix);
						//DRT 11/14/2008 - PLID 32036 - Parameterized
						ExecuteParamSql("UPDATE PersonT SET PrefixID = {INT} WHERE ID = {INT}", nNewPrefix, m_id);
					}
				}
			}
		}

		// (j.gruber 2007-08-27 14:58) - PLID 24628 - Update HL7
		UpdateHL7Data();


		// UPDATE THE PALM RECORD
		UpdatePalm();
	} NxCatchAll("Error in changing gender. General1Dlg::OnClosedUpGenderList");
}

void CGeneral1Dlg::OnEditPrefixes() 
{
	CEditPrefixesDlg dlg(this);
	dlg.m_bChangeInformIds = false;
	if(IDOK == dlg.DoModal()) {
		long PrefixID = -1;
		if(m_PrefixCombo->GetCurSel() != -1)
			PrefixID = m_PrefixCombo->GetValue(m_PrefixCombo->GetCurSel(),0).lVal;

		m_PrefixCombo->Requery();

		if(PrefixID > -1)
			m_PrefixCombo->SetSelByColumn(0,(long)PrefixID);
		
		UpdateView();
	}
	GetDlgItem(IDC_PREFIX_COMBO)->SetFocus();
}

void CGeneral1Dlg::OnKillFocusBirthDateBox() 
{
	//DRT 9/29/03 - 1)  If it was set, and is now clear, save.  2)  If new date is <> old date, save.  3)  If was clear, and now not, save.
	if((m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 3) || ((long)m_nxtBirthDate->GetDateTime() != (long)m_dtBirthDate.m_dt) || (!m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 1)) {
		m_changed = true;
		Save(IDC_BIRTH_DATE_BOX);
	}
}

void CGeneral1Dlg::OnKillFocusContactDate() 
{
	if(m_nxtFirstContact->GetDateTime() != m_dtFirstContact) {
		m_changed = true;
		Save(IDC_CONTACT_DATE);
	}
}

void CGeneral1Dlg::OnKillfocusEmailBox() 
{
	//warn them if the email address looks invalid
	//for now that just means, "does it have an @ sign?"
	bool bInvalid = false;

	CString str;
	GetDlgItemText(IDC_EMAIL_BOX, str);

	str.TrimLeft();
	str.TrimRight();

	// May have gotten here from clicking "declined"
	if(str.IsEmpty() && !m_bDeclinedEmail) {
			return;
	}

	// User clicked decline, but we need to allow blank email addresses
	if (m_bDeclinedEmail && str.IsEmpty()) {
		m_strOldEmail = "";
		m_nxeEmail.SetWindowText("");
		Save(IDC_EMAIL_BOX);
		m_bDeclinedEmail = false;
		m_bDeclinedEmailWasBlank = true;
		return;
	}

	//(c.copits 2011-09-22) PLID 45632 - General 1 email validation lets through invalid addresses.
	/*
	if(str.Find("@") == -1) {
		//not found 
		bInvalid = true;
	}
	*/

	bInvalid = !IsValidEmailAddress(str);

	if(bInvalid) {
		
		//(c.copits 2011-09-15) PLID 28544 - Reject invalid email addresses
		BOOL bRejectInvalidEmailAddresses = GetRemotePropertyInt("RejectInvalidEmailAddresses", 0, 0, "<None>", true);

		CString strErrorMessage = "Your email address is invalid. Ensure that it contains:\n\n"
			"- A user name\n"
			"- One @ symbol\n"
			"- A Domain name\n"
			"- No special characters (such as spaces and quotes)\n\n"
			"Example: example@nextech.com";

		if (bRejectInvalidEmailAddresses) {
			
			// Don't change anything if they lack the permission to edit email addresses
			if (CheckCurrentUserPermissions(bioPatientEmail, SPT___W_______)) {
				strErrorMessage += "\n\nThis invalid email address will not be saved.";
				if (m_strOldEmail == "< Declined >") {
					// Tricky: If user declines the email, then sets RejectInvalidEmailAddresses to no,
					// then enters an invalid email, and then sets RejectInvalidEmailAddresses to yes, and
					// then clicks the email field, the invalid email address will disappear on screen
					// but still exist in data. So, we need to erase it from data.
					_RecordsetPtr rs = CreateParamRecordset("SELECT Email FROM PersonT WHERE ID = {INT}", m_id);
					if (!rs->eof)
					{
						CString strExistingEmailInData;
						m_nxeEmail.GetWindowText(strExistingEmailInData);
						_variant_t varExistingEmail = rs->Fields->Item["Email"]->Value;
						CString strExistingEmail = VarString(varExistingEmail, "");
						if (strExistingEmail == strExistingEmailInData) {
							ExecuteParamSql("UPDATE PersonT SET Email = {STRING} WHERE ID = {INT} ", "", m_id);
						}
					}
					m_strOldEmail = "";
					m_nxeEmail.SetWindowText("");
				}
				else {
					// Since the email is saved before the email is actually checked for validity, we must erase it here.
					ExecuteParamSql("UPDATE PersonT SET Email = {STRING} WHERE ID = {INT} ", m_strOldEmail, m_id);
				}
				AfxMessageBox(strErrorMessage);
				if (m_strOldEmail != "< Declined >") {
					SetDlgItemText(IDC_EMAIL_BOX, m_strOldEmail);
				}
			}
		}
		else {
			//give a message that tells them we think this address is invalid, but there is
			//always the possibility that they want something else in there, so don't do anything other
			//than that.
			//(c.copits 2011-09-29) PLID 28544 - Reject invalid email addresses
			MessageBox(strErrorMessage, "Invalid Email Address", MB_ICONWARNING|MB_OK);
			//we can't set the focus back to the email box, or we'll be stuck there forever, 
			//so just go back to the source and start over at first name
			GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();
		}
	}
}

void CGeneral1Dlg::OnTrySetSelFinishedDoctorCombo(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		//they may have an inactive provider
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT})", GetActivePatientID());
		if(!rsProv->eof) {
			m_DoctorCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
		}
		else 
			m_DoctorCombo->PutCurSel(-1);
	}
}

void CGeneral1Dlg::OnPagerPrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivPager = {INT} WHERE ID = {INT}",(long)m_btnPagerPriv.GetCheck(), m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivPager, m_id, m_btnPagerPriv.GetCheck() ? "No" : "Yes", m_btnPagerPriv.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnFaxPrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivFax = {INT} WHERE ID = {INT}",(long)m_btnFaxPriv.GetCheck(), m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivFax, m_id, m_btnFaxPriv.GetCheck() ? "No" : "Yes", m_btnFaxPriv.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnOtherPrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivOther = {INT} WHERE ID = {INT}",(long)m_btnOtherPriv.GetCheck(), m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivOther, m_id, m_btnOtherPriv.GetCheck() ? "No" : "Yes", m_btnOtherPriv.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnEmailPrivCheck() 
{
	if (m_loading) return;
	try {
		//DRT 11/14/2008 - PLID 32036 - Parameterized
		ExecuteParamSql("UPDATE PersonT SET PrivEmail = {INT} WHERE ID = {INT}",(long)m_btnEmailPriv.GetCheck(), m_id);
		// UPDATE THE PALM RECORD
		UpdatePalm();

		//DRT 6/30/2005 - PLID 16654 - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPrivEmail, m_id, m_btnEmailPriv.GetCheck() ? "No" : "Yes", m_btnEmailPriv.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("Error In Changing Privacy Flag");
}

void CGeneral1Dlg::OnTrySetSelFinishedCoordCombo(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		//maybe it's inactive?
		//DRT 11/13/2008 - PLID 32036 - Parameterized
		_RecordsetPtr rsCoord = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT EmployeeID FROM PatientsT WHERE PersonID = {INT})", GetActivePatientID());
		if(!rsCoord->eof) {
			m_CoordCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCoord, "Name", "")));
		}
		else 
			m_CoordCombo->PutCurSel(-1);
	}
}

void CGeneral1Dlg::LoadSingleImage(long nImageIndexAgainstSource, long nImageCountSource, CMirrorImageButton *pButton, EImageSource Source)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	// (c.haag 2009-04-01 17:02) - PLID 33630 - Before we do anything else, try to establish a connection
	// with Mirror. If the result is that the link is being established, put the "Initializing Mirror"
	// image on the thumbnail region, and quit immediately. Otherwise, proceed as before.
	if (DoesCanfieldSDKNeedToInitialize()) {
		return;
	}

	//Regardless...
	pButton->m_source = Source;
	switch(Source) {
	case eImageSrcMirror:
	{
		// Load from Mirror
		//Now that we know there's something to view, check whether they have permission to view it.
		if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
			//bShowArrows = false;
			// Clear whatever image is there
			if (pButton->m_image)
			{	DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_nError = eErrorNoPermission;		
		}
		else {
			// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
			const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
			if (bUsingSDKFunctionality && GetRemotePropertyInt("MirrorAllowAsyncOperations", 1))
			{
				pButton->m_image = (HBITMAP)MIRRORIMAGEBUTTON_PENDING;
				// (c.haag 2010-02-22 17:08) - PLID 37364 - We must ensure that the Mirror Image manager object exists before this thread begins.
				// We don't want it to initialize in the thread because the constructor accesses the global connection pointer.
				EnsureMirrorImageMgr();
				LoadImageAsync(new CGeneral1ImageLoad(this, m_pMirrorImageMgr, m_strMirrorID, nImageIndexAgainstSource, nImageCountSource, m_id, pButton));
			}
			else
			{
				pButton->m_source = eImageSrcMirror;
				// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
				// (c.haag 2010-02-23 10:05) - PLID 37364 - We now use a member function
				pButton->m_image = LoadMirrorImage(nImageIndexAgainstSource, nImageCountSource, -1);
				if (pButton->m_image == NULL)
					pButton->m_nError = eErrorUnspecified;
				else
					pButton->m_nError = eNoError;
			}
		}
	} 
	break;
	case eImageSrcUnited:
	{
		// Load from United
		//Now that we know there's something to view, check whether they have permission to view it.
		if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
			//bShowArrows = false;
			// Clear whatever image is there
			if (pButton->m_image)
			{	DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_nError = eErrorNoPermission;		
		}
		else {
			pButton->m_source = eImageSrcUnited;
			pButton->m_image = GetMainFrame()->GetUnitedLink()->LoadImage(m_nUnitedID, nImageIndexAgainstSource);
			if (pButton->m_image == NULL)
				pButton->m_nError = eErrorUnspecified;
			else
				pButton->m_nError = eNoError;
		}

	} 
	break;
	case eImageSrcPractice:
	{
		// Load from NexTech
		//Now that we know there's something to view, check whether they have permission to view it.
		if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
			//bShowArrows = false;
			// Clear whatever image is there
			if (pButton->m_image)
			{	DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_nError = eErrorNoPermission;		
		}
		else {
			pButton->m_source = eImageSrcPractice;
			if(pButton->m_image) {
				DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_image = LoadPatientAttachedImage(m_id, nImageIndexAgainstSource, &pButton->m_progress, pButton->m_strPracticeFileName);
			if (pButton->m_image == NULL)
				pButton->m_nError = eErrorUnspecified;
			else
				pButton->m_nError = eNoError;
		}
	}
	break;
	case eImageSrcRSIMMS:
		// (c.haag 2009-07-07 13:11) - PLID 34379 - Load from RSI MMS
		if (m_nMMSPatientID != -1) {
			//Now that we know there's something to view, check whether they have permission to view it.
			if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
				//bShowArrows = false;
				// Clear whatever image is there
				if (pButton->m_image)
				{	DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
				}
				pButton->m_nError = eErrorNoPermission;		
			}
			else {
				CWaitCursor wc;
				pButton->m_source = eImageSrcRSIMMS;
				if(pButton->m_image) {
					DeleteObject(pButton->m_image);
					pButton->m_image = NULL;
				}
				// Load the image, and store it's full path in pButton->m_strPracticeFileName. It's not technically the "Practice"
				// filename; but like Practice, images are stored individually on a drive.
				pButton->m_image = RSIMMSLink::LoadImage(m_nMMSPatientID, nImageIndexAgainstSource, &pButton->m_strPracticeFileName);
				if (pButton->m_image == NULL)
					pButton->m_nError = eErrorUnspecified;
				else
					pButton->m_nError = eNoError;
			}
		} // if (m_nMMSPatientID != -1) {
		break;
	}
}

void CGeneral1Dlg::ShowImagingButtons(int nCountToShow)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CEMRSummaryDlg also needs changed

	//TES 2/25/2004: I have this theory that calling ShowWindow(SW_HIDE) on an already hidden window causes 
	//some minor drawing issues.
	if(nCountToShow == 0) {//Hide everything.
		if(m_imageButton.GetStyle() & WS_VISIBLE) m_imageButton.ShowWindow(SW_HIDE);

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) m_imageButtonLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) m_imageButtonRight.ShowWindow(SW_HIDE);

		if(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE) m_imageButtonUpperLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE) m_imageButtonUpperRight.ShowWindow(SW_HIDE);
		if(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE) m_imageButtonLowerLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) m_imageButtonLowerRight.ShowWindow(SW_HIDE);
	}
	else if(nCountToShow == 1) {//Just show the big one.
		if(!(m_imageButton.GetStyle() & WS_VISIBLE)) m_imageButton.ShowWindow(SW_SHOW);

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) m_imageButtonLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) m_imageButtonRight.ShowWindow(SW_HIDE);

		if(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE) m_imageButtonUpperLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE) m_imageButtonUpperRight.ShowWindow(SW_HIDE);
		if(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE) m_imageButtonLowerLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) m_imageButtonLowerRight.ShowWindow(SW_HIDE);
	}
	else if(nCountToShow == 2) {//Just show the left and right.
		if(m_imageButton.GetStyle() & WS_VISIBLE) m_imageButton.ShowWindow(SW_HIDE);

		if(!(m_imageButtonLeft.GetStyle() & WS_VISIBLE)) m_imageButtonLeft.ShowWindow(SW_SHOW);
		if(!(m_imageButtonRight.GetStyle() & WS_VISIBLE)) m_imageButtonRight.ShowWindow(SW_SHOW);

		if(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE) m_imageButtonUpperLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE) m_imageButtonUpperRight.ShowWindow(SW_HIDE);
		if(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE) m_imageButtonLowerLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) m_imageButtonLowerRight.ShowWindow(SW_HIDE);
	}
	else if(nCountToShow == 3) {//Show three of the four small ones.
		if(m_imageButton.GetStyle() & WS_VISIBLE) m_imageButton.ShowWindow(SW_HIDE);

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) m_imageButtonLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) m_imageButtonRight.ShowWindow(SW_HIDE);

		if(!(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE)) m_imageButtonUpperLeft.ShowWindow(SW_SHOW);
		if(!(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE)) m_imageButtonUpperRight.ShowWindow(SW_SHOW);
		if(!(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE)) m_imageButtonLowerLeft.ShowWindow(SW_SHOW);
		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) m_imageButtonLowerRight.ShowWindow(SW_HIDE);
	}
	else {//Show the four small ones.
		if(m_imageButton.GetStyle() & WS_VISIBLE) m_imageButton.ShowWindow(SW_HIDE);

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) m_imageButtonLeft.ShowWindow(SW_HIDE);
		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) m_imageButtonRight.ShowWindow(SW_HIDE);

		if(!(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE)) m_imageButtonUpperLeft.ShowWindow(SW_SHOW);
		if(!(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE)) m_imageButtonUpperRight.ShowWindow(SW_SHOW);
		if(!(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE)) m_imageButtonLowerLeft.ShowWindow(SW_SHOW);
		if(!(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE)) m_imageButtonLowerRight.ShowWindow(SW_SHOW);
	}
}

void CGeneral1Dlg::OnRequeryFinishedDoctorCombo(short nFlags) 
{
	try {
		//DRT 6/18/2004 - PLID 13079
		IRowSettingsPtr pRow = m_DoctorCombo->GetRow(sriNoRow);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(4, "<No Provider>");
		m_DoctorCombo->InsertRow(pRow, 0);
	} NxCatchAll("Error in OnRequeryFinishedDoctorCombo");
}

//DRT 8/11/2004 - PLID 13858
void CGeneral1Dlg::OnRequeryFinishedCoordCombo(short nFlags) 
{
	try {
		IRowSettingsPtr pRow = m_CoordCombo->GetRow(-1);
		_variant_t var = (long)0;
		pRow->PutValue(0,var);
		pRow->PutValue(3,"<No Patient Coordinator>");
		m_CoordCombo->InsertRow(pRow,0);
	} NxCatchAll("Error in OnRequeryFinishedCoordCombo()");
}

LRESULT CGeneral1Dlg::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	// (a.wetta 2007-03-19 14:25) - PLID 24983 - Process the driver's license information
	CWaitCursor wait;

	// (a.wetta 2007-07-05 08:56) - PLID 26547 - Get the track information from the card swipe and then parse the tracks
	MSRTrackInfo *mtiInfo = (MSRTrackInfo*)wParam;	

	// (j.jones 2009-06-19 10:56) - PLID 33650 - changed boolean to be an enum
	if(mtiInfo->msrCardType == msrDriversLicense) {

		DriversLicenseInfo dliLicenseInfo = COPOSMSRDevice::ParseDriversLicenseInfoFromMSRTracks(mtiInfo->strTrack1, mtiInfo->strTrack2, mtiInfo->strTrack3);

		// (b.cardillo 2007-09-17 11:43) - PLID 24983 - Fixed up the order things happen here.  We used to prompt the user 
		// first, telling him that he had swiped a driver's license, even before we knew it wasn't a credit card.  Now we 
		// check first, then if it's a not a credit card prompt him, or if it is just tell him that credit cards can't be 
		// swiped for demographic info.  Notice the permission warning is also inside here, not just because it also says 
		// a driver's license was scanned, but because if it's a credit card, the more appropriate warning for users right 
		// now is that they are incorrectly trying to scan a credit card, not that they don't have permissions for editing.

		// (a.wetta 2007-03-20 09:55) - PLID 24983 - If they don't have permission to write then they shouldn't be able to swipe patient demographics
		// into the General 1 tab
		// (j.armen 2011-06-15 11:07) - PLID 40246 - Corrected Typo
		if (!CheckCurrentUserPermissions(bioPatient, SPT___W_______, FALSE, 0, TRUE)) {
			MessageBox("You do not have permission to write patient information, so you cannot retrieve patient demographics from\n"
						"a driver's license.  Please see your office manager.", 
						   "Magnetic Strip Reader", MB_ICONWARNING|MB_OK);
			return 0;
		}
		
		// Warn the user that the information from the license will overwrite the information already in the fields
		int nResult = MessageBox(
			"A driver's license has just been swiped.  The information from the license will fill the "
			"corresponding demographic fields for this patient, so any information already in those "
			"fields will be overwritten.\r\n\r\n"
			"Are you sure you want to import the information from the driver's license?", 
			"Magnetic Strip Reader", MB_YESNO|MB_ICONQUESTION);
		if (nResult == IDYES) {
			// Fill the information from the driver's license to the screen and be sure to handle the changes along the way (i.e. audit changes)
			m_bProcessingCard = true;

			SetDlgItemText(IDC_FIRST_NAME_BOX, FixCapitalization(dliLicenseInfo.m_strFirstName));
			m_changed = true;
			Save(IDC_FIRST_NAME_BOX);

			SetDlgItemText(IDC_MIDDLE_NAME_BOX, FixCapitalization(dliLicenseInfo.m_strSuffix));
			m_changed = true;
			Save(IDC_MIDDLE_NAME_BOX);

			SetDlgItemText(IDC_LAST_NAME_BOX, FixCapitalization(dliLicenseInfo.m_strLastName));
			m_changed = true;
			Save(IDC_LAST_NAME_BOX);

			SetDlgItemText(IDC_ADDRESS1_BOX, FixCapitalization(dliLicenseInfo.m_strAddress1));
			m_changed = true;
			Save(IDC_ADDRESS1_BOX);

			SetDlgItemText(IDC_ADDRESS2_BOX, FixCapitalization(dliLicenseInfo.m_strAddress2));
			m_changed = true;
			Save(IDC_ADDRESS2_BOX);

			SetDlgItemText(IDC_CITY_BOX, FixCapitalization(dliLicenseInfo.m_strCity));
			m_changed = true;
			Save(IDC_CITY_BOX);

			SetDlgItemText(IDC_STATE_BOX, dliLicenseInfo.m_strState);
			m_changed = true;
			Save(IDC_STATE_BOX);

			SetDlgItemText(IDC_ZIP_BOX, dliLicenseInfo.m_strPostalCode);
			m_changed = true;
			Save(IDC_ZIP_BOX);

			if (dliLicenseInfo.m_dtBirthdate != 0) {
				m_nxtBirthDate->SetDateTime(dliLicenseInfo.m_dtBirthdate);	
			}
			else {
				m_nxtBirthDate->Clear();
			}
			if((m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 3) || ((long)m_nxtBirthDate->GetDateTime() != (long)m_dtBirthDate.m_dt) || (!m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 1)) {
				m_changed = true;
				Save(IDC_BIRTH_DATE_BOX);
			}

			if (dliLicenseInfo.m_strSex == "male") {
				m_GenderCombo->SetSelByColumn(0, _variant_t("Male"));
				OnSelChosenGenderList(1);
			}
			else if (dliLicenseInfo.m_strSex == "female") {
				m_GenderCombo->SetSelByColumn(0, _variant_t("Female"));
				OnSelChosenGenderList(2);
			}
			else {
				m_GenderCombo->SetSelByColumn(0, _variant_t(""));
				OnSelChosenGenderList(-1);
			}
			//(a.wilson 2012-1-19) PLID 47485 - changed name to better suit its additional purpose
			m_bProcessingCard = false;
		}
	}
	// (j.jones 2009-06-19 10:58) - PLID 33650 - made this message more generic as we support more than two card types
	else if(UseOHIP() && mtiInfo->msrCardType == msrOHIPHealthCard) {

		OHIPHealthCardInfo ohciHealthCardInfo = COPOSMSRDevice::ParseOHIPHealthCardInfoFromMSRTracks(mtiInfo->strTrack1, mtiInfo->strTrack2, mtiInfo->strTrack3);

		// (j.armen 2011-06-15 11:07) - PLID 40246 - Corrected Typo
		if (!CheckCurrentUserPermissions(bioPatient, SPT___W_______, FALSE, 0, TRUE)) {
			MessageBox("You do not have permission to write patient information, so you cannot retrieve information from\n"
						"a health card.  Please see your office manager.", 
						   "Magnetic Strip Reader", MB_ICONWARNING|MB_OK);
			return 0;
		}

		// Warn the user that the information from the license will overwrite the information already in the fields
		int nResult = MessageBox(
			"A health card has just been swiped.  The information from the health card will fill the "
			"corresponding demographic fields for this patient, so any information already in those "
			"fields will be overwritten.\r\n\r\n"
			"Are you sure you want to import the information from the health card?", 
			"Magnetic Strip Reader", MB_YESNO|MB_ICONQUESTION);
		if (nResult == IDYES) {
			//(a.wilson 2012-1-19) PLID 47485 - changed name to better suit its additional purpose
			m_bProcessingCard = true;

			SetDlgItemText(IDC_FIRST_NAME_BOX, FixCapitalization(ohciHealthCardInfo.m_strFirstName));
			m_changed = true;
			Save(IDC_FIRST_NAME_BOX);

			SetDlgItemText(IDC_MIDDLE_NAME_BOX, FixCapitalization(ohciHealthCardInfo.m_strMiddleName));
			m_changed = true;
			Save(IDC_MIDDLE_NAME_BOX);

			SetDlgItemText(IDC_LAST_NAME_BOX, FixCapitalization(ohciHealthCardInfo.m_strLastName));
			m_changed = true;
			Save(IDC_LAST_NAME_BOX);

			if (ohciHealthCardInfo.m_dtBirthDate != 0) {
				m_nxtBirthDate->SetDateTime(ohciHealthCardInfo.m_dtBirthDate);	
			}
			else {
				m_nxtBirthDate->Clear();
			}
			if((m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 3) || ((long)m_nxtBirthDate->GetDateTime() != (long)m_dtBirthDate.m_dt) || (!m_bBirthDateSet && m_nxtBirthDate->GetStatus() == 1)) {
				m_changed = true;
				Save(IDC_BIRTH_DATE_BOX);
			}

			if (ohciHealthCardInfo.m_strSex == "1") {
				m_GenderCombo->SetSelByColumn(0, _variant_t("Male"));
				OnSelChosenGenderList(1);
			}
			else if (ohciHealthCardInfo.m_strSex == "2") {
				m_GenderCombo->SetSelByColumn(0, _variant_t("Female"));
				OnSelChosenGenderList(2);
			}
			else {
				m_GenderCombo->SetSelByColumn(0, _variant_t(""));
				OnSelChosenGenderList(-1);
			}

			//the health card number and version code are custom fields
			long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
			long nVersionCodeCustomField = GetRemotePropertyInt("OHIP_VersionCodeCustomField", 2, 0, "<None>", true);

			switch (nHealthNumberCustomField) {
			case 1:
				SetDlgItemText (IDC_CUSTOM1_BOX, ohciHealthCardInfo.m_strHealthCardNum);
				break;
			case 2:
				SetDlgItemText (IDC_CUSTOM2_BOX, ohciHealthCardInfo.m_strHealthCardNum);
				break;
			case 3:
				SetDlgItemText (IDC_CUSTOM3_BOX, ohciHealthCardInfo.m_strHealthCardNum);
				break;
			case 4:
				SetDlgItemText (IDC_CUSTOM4_BOX, ohciHealthCardInfo.m_strHealthCardNum);
				break;
			}

			SaveCustomInfo(nHealthNumberCustomField, ohciHealthCardInfo.m_strHealthCardNum);

			switch (nVersionCodeCustomField) {
			case 1:
				SetDlgItemText (IDC_CUSTOM1_BOX, ohciHealthCardInfo.m_strVersionCode);
				break;
			case 2:
				SetDlgItemText (IDC_CUSTOM2_BOX, ohciHealthCardInfo.m_strVersionCode);
				break;
			case 3:
				SetDlgItemText (IDC_CUSTOM3_BOX, ohciHealthCardInfo.m_strVersionCode);
				break;
			case 4:
				SetDlgItemText (IDC_CUSTOM4_BOX, ohciHealthCardInfo.m_strVersionCode);
				break;
			}

			SaveCustomInfo(nVersionCodeCustomField, ohciHealthCardInfo.m_strVersionCode);
			//(a.wilson 2012-1-19) PLID 47485 - changed name to better suit its additional purpose
			m_bProcessingCard = false;
		}
	}
	else {
		// (j.jones 2009-06-19 10:58) - PLID 33650 - made this message more generic as we support more than two card types
		if(UseOHIP()) {
			//give a message mentioning OHIP support
			MessageBox(
				"A card has just been swiped, but patient demographic information can only be retrieved from a driver's license or a health card.\r\n\r\n"
				"Please swipe a driver's license or health card to import patient information.", 
				"Magnetic Strip Reader", MB_OK|MB_ICONEXCLAMATION);
		}
		else {
			//normal message
			MessageBox(
				"A card has just been swiped, but patient demographic information can only be retrieved from a driver's license.\r\n\r\n"
				"Please swipe a driver's license to import patient information.", 
				"Magnetic Strip Reader", MB_OK|MB_ICONEXCLAMATION);
		}
	}

	return 0;
}

// (j.gruber 2007-08-27 09:39) - PLID 24628 - used for updating HL7
void CGeneral1Dlg::UpdateHL7Data() {

	try {

		// (z.manning 2009-01-08 15:20) - PLID 32663 - Moved this code to a global function
		UpdateExistingPatientInHL7(m_id);
		
	}NxCatchAll("Error in CGeneral1Dlg::UpdateHL7Data");
}

// (j.jones 2008-07-09 10:30) - PLID 24624 - added patient summary
void CGeneral1Dlg::OnBtnPatientSummary() 
{
	try {	

		CPatientSummaryDlg dlg(this);
		dlg.m_nPatientID = m_id;
		dlg.m_strPatientName = m_strPatientName;
		dlg.DoModal();

	} NxCatchAll("Error in CGeneral1Dlg::OnBtnPatientSummary");	
}

void CGeneral1Dlg::OnCellTextMessage()
{
	try
	{
		if(m_loading) {
			return;
		}

		// (z.manning 2008-07-11 09:12) - PLID 30678 - Update this patient's text message preference.
		ExecuteParamSql("UPDATE PersonT SET TextMessage = {INT} WHERE ID = {INT}"
			,m_btnTextMessage.GetCheck() == BST_CHECKED ? 1 : 0, m_id);

		// (z.manning 2008-07-11 09:13) - Auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientCellTextMessage, m_id, m_btnTextMessage.GetCheck() ? "No" : "Yes", m_btnTextMessage.GetCheck() ? "Yes" : "No", aepLow, aetChanged);

	}NxCatchAll("CGeneral1Dlg::OnCellTextMessage");
}

// (c.haag 2009-04-01 17:06) - PLID 33630 - This function will determine whether the Canfield SDK
// needs to initialize. It will do so by trying to asynchronously initialize the link. If the result
// is that the initialization is in progress, the image thumbnail is changed to the "Initializing Mirror"
// sentinel value, and this returns TRUE. In all other circumstances, will return FALSE.
BOOL CGeneral1Dlg::DoesCanfieldSDKNeedToInitialize()
{
	// Note: This should only be called from the main thread

	switch (Mirror::InitCanfieldSDK(FALSE, GetSafeHwnd())) {
		case Mirror::eCSDK_Success:
			return FALSE; // We got connected
		case Mirror::eCSDK_Connecting_WillPostMessage:
			// The link is initializing. Defer to the code after the switch.
			break;
		case Mirror::eCSDK_Failure_Link_Disabled:
			// We did not attempt a connection because the link is disabled. So, this counts as a negative.
			return FALSE;		
		case Mirror::eCSDK_Failure_Link_SDK_Disabled:
			// There is a ConfigRT property that disabled the express use of the Canfield SDK. This counts
			// as a negative.
			return FALSE;		
		case Mirror::eCSDK_Failure_NotAvailable:
			// Mirror was not detected on this machine
			return FALSE;
		case Mirror::eCSDK_Failure_TimeoutExpired:
			// A timeout occured attempting to connect to the Canfield SDK. We can't try again until the
			// user restarts Practice. This counts as a negative.
			return FALSE; 
		case Mirror::eCSDK_Failure_Error:
			// An unexpected error occured attempting to use the Canfield SDK. We can't try again until the
			// user restarts Practice. This counts as a negative.
			return FALSE;
		default:
			ASSERT(FALSE); // This should never happen
			return FALSE;
	}

	// If we get here, the link is asynchronously initializing. Update the thumbnail to reflect that,
	// and return an affirmative value. Make sure the thumbnail is visible.
	if (m_imageButton.m_nError != eImageBtnInitializingMirror) {
		m_imageButton.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IMAGE_LAST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IMAGE_NEXT)->ShowWindow(SW_HIDE);
		m_imageButton.m_nError = eImageBtnInitializingMirror;
		m_imageButton.Invalidate();
	} else {
		// It's already set to the error text; don't do anything
	}

	return TRUE;
}

// (c.haag 2009-04-01 17:17) - PLID 33630 - This message is handled after a prior call to
// DoesCanfieldSDKNeedToInitialize resulted in the asynchronous initialization of Mirror.
LRESULT CGeneral1Dlg::OnCanfieldSDKInitComplete(WPARAM wParam, LPARAM lParam)
{
	try {
		// (j.dinatale 2012-10-29 10:06) - PLID 51585 - Need to keep track if init failed. If init failed,
		//	that means Canfield internally threw and exception which means a potential faulty install/uninstall.
		//	At this point, we want to globally force a disable of Canfield linking.
		BOOL bInitFailed = (BOOL)lParam;
		if(bInitFailed){
			if(!Mirror::g_bForceDisable){
				this->MessageBox("Mirror Initialization Failed. This can be due to a faulty installation of Canfield Mirror. \r\n\r\n"
					"The Mirror link has been disabled for your current session. Please ensure that Mirror is installed correctly.", "Error!", MB_OK | MB_ICONERROR);
			}

			Mirror::g_bForceDisable = TRUE;
		}

		CanfieldLink::ELinkStatus result = (CanfieldLink::ELinkStatus)wParam;
		if (CanfieldLink::eStatusTryingToLink == result) {
			// This should never happen
			ThrowNxException("This function was called with an eStatusTryingToLink status");
		} else {
			// In all other cases, we're now ready to load the patient imaging info
			_RecordsetPtr rs = CreateParamRecordset(FormatString("%s WHERE ID = {INT};", m_sql), m_id);
			LoadPatientImagingInfo(rs);
		}
	}
	NxCatchAll("Error in CGeneral1Dlg::OnCanfieldSDKInitComplete");
	return 0;
}

// (z.manning 2009-07-08 15:45) - PLID 27251 - Handle the cursor when hovering over any hyperlinks
BOOL CGeneral1Dlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	try
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcEmailReq;
		m_nxlabelDeclinedEmail.GetWindowRect(rcEmailReq);
		ScreenToClient(&rcEmailReq);

		CRect rcSetEmailReq;
		m_nxlabelSetEmailDeclined.GetWindowRect(rcSetEmailReq);
		ScreenToClient(&rcSetEmailReq);
		// (s.dhole 2010-03-29 09:21) - PLID  37796 For internal only, make the company show up in G1.
		CRect rcCompany;
		m_nxlabelSetCompanyLink.GetWindowRect(rcCompany);
		ScreenToClient(&rcCompany);

		if((rcEmailReq.PtInRect(pt) && m_nxlabelDeclinedEmail.GetType() == dtsHyperlink && m_nxlabelDeclinedEmail.IsWindowVisible()) ||
			(rcSetEmailReq.PtInRect(pt) && m_nxlabelSetEmailDeclined.GetType() == dtsHyperlink && m_nxlabelSetEmailDeclined.IsWindowVisible()) ||
			(rcCompany.PtInRect(pt) && m_nxlabelSetCompanyLink.GetType() == dtsHyperlink && m_nxlabelSetCompanyLink.IsWindowVisible()) // (s.dhole 2010-03-26 17:26) - PLID 37796 For internal only, make the company show up in G1.
			)

		{
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (z.manning 2009-07-08 15:54) - PLID 27251 - Handle clicking on labels
LRESULT CGeneral1Dlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc)
		{
			case IDC_COMPANY_LINK_LABEL:
			// (s.dhole 2010-03-26 16:51) - PLID 37796 For internal only, make the company show up in G1.
			// We are swiching tab assuming both tab have permission and belong to same person, Active module is PATIENT_MODULE_NAME
				if(m_nxlabelSetCompanyLink.GetType() == dtsHyperlink) {
					CMainFrame *pMainFrame;
					pMainFrame = GetMainFrame();
					if (pMainFrame != NULL) {
					//	pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
						CNxTabView *pView =  (CPatientView*)pMainFrame->GetOpenView(PATIENT_MODULE_NAME) ;
						if(pView)
						{
							pView->SetActiveTab(PatientsModule::General2Tab);
							pView->UpdateView();
						}
					}
				}
				break;
			case IDC_EMAIL_DECLINED_LABEL:
				if(m_nxlabelDeclinedEmail.GetType() == dtsHyperlink) {
					if(CheckCurrentUserPermissions(bioPatientEmail, sptWrite)) {
						CString strEmail;
						// (z.manning 2009-07-09 11:32) - PLID 27251 - Prompt them to enter the patient's email address.
						// I'm intentionally not checking for blank email addresses since there's nothing wrong with that.
						int nResult = InputBoxLimitedWithParent(this, "Please enter this patient's e-mail address", strEmail, "", 50, false, false, NULL);
						if(nResult == IDOK) {
							//(c.copits 2011-09-22) PLID 45632 - General 1 email validation lets through invalid addresses.
							// Allow blank emails
							if (!IsValidEmailAddress(strEmail) && GetRemotePropertyInt("RejectInvalidEmailAddresses", 0, 0, "<None>", true)) {
								m_bDeclinedEmail = true;
								m_strOldEmail = "< Declined >";
								SetDlgItemText(IDC_EMAIL_BOX, strEmail);
								OnKillfocusEmailBox();
								CString strEmailNew;
								GetDlgItemText(IDC_EMAIL_BOX, strEmailNew);
								// Allow blank emails
								if (m_bDeclinedEmailWasBlank) {
									UpdateEmail(FALSE, "");
									m_bDeclinedEmailWasBlank = false;
									ExecuteParamSql("UPDATE PatientsT SET DeclinedEmail = 0 WHERE PersonID = {INT}", m_id);
								}
								// Anything else is invalid and we don't want it
								else {
									UpdateEmailButton();
									UpdateEmail(TRUE, "");
								}
								return 0;
							}
							m_nxeEmail.SetWindowText(strEmail);
							Save(IDC_EMAIL_BOX);
							ExecuteParamSql("UPDATE PatientsT SET DeclinedEmail = 0 WHERE PersonID = {INT}", m_id);
							UpdateEmail(FALSE, strEmail);
							OnKillfocusEmailBox();
						}
					}
				}
				break;

			case IDC_EMAIL_SET_DECLINED_LABEL:
				// (z.manning 2009-07-08 16:11) - PLID 27251 - Prompt them to mark email as "declined" so that
				// they can still have it as a requied field but be able to not fill it out for patients who
				// don't have e-mail or refuse to share it.
				int nResult = MessageBox("Would you like to mark e-mail as declined for this patient?", "Decline Email", MB_YESNO|MB_ICONQUESTION);
				if(nResult == IDYES) {
					m_nxeEmail.SetWindowText("");
					Save(IDC_EMAIL_BOX);
					ExecuteParamSql("UPDATE PatientsT SET DeclinedEmail = 1 WHERE PersonID = {INT}", m_id);
					AuditEvent(m_id, m_strPatientName, BeginNewAuditEvent(), aeiPatientDeclinedEmail, m_id, "", "Declined Email Address", aepMedium, aetChanged);
					UpdateEmail(TRUE, "");
				}
				break;
		}
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (z.manning 2009-07-09 11:56) - PLID 27251 - Created a function to handle logic of updating email controls
void CGeneral1Dlg::UpdateEmail(const BOOL bDeclinedEmail, const CString &strEmail)
{
	if(bDeclinedEmail) {
		// (z.manning 2009-07-09 11:56) - PLID 27251 - This patient declined email when he/she was created,
		// so let's hide the email field and indicate that it was declined.
		m_nxeEmail.ShowWindow(SW_HIDE);
		m_nxlabelDeclinedEmail.ShowWindow(SW_SHOW);
		m_nxlabelDeclinedEmail.SetType(dtsHyperlink);
		m_nxlabelDeclinedEmail.SetText("< Declined >");
		m_nxlabelSetEmailDeclined.ShowWindow(SW_HIDE);
		// (v.maida 2015-08-13 13:25) - PLID 66664 - Even though the email control is being hidden, set its text to the appropriate (blank) value anyway so that any future
		// attempts to pull directly from the textbox don't possibly pull out the e-mail address of a previously visited patient, which could still be residing in the hidden box.
		SetDlgItemText(IDC_EMAIL_BOX, strEmail);
	}
	else {
		// (z.manning 2009-07-09 11:57) - PLID 27251 - Email is not declined so show the email edit box as normal.
		m_nxeEmail.ShowWindow(SW_SHOW);
		m_nxlabelDeclinedEmail.ShowWindow(SW_HIDE);
		m_nxlabelDeclinedEmail.SetText("");
		m_nxlabelSetEmailDeclined.ShowWindow(SW_SHOW);
		m_nxlabelSetEmailDeclined.SetType(dtsHyperlink);
		m_nxlabelSetEmailDeclined.SetText("**");
		SetDlgItemText(IDC_EMAIL_BOX, strEmail);//,overwrite);
	}
}


void CGeneral1Dlg::OnEditSecurityGroups()
{
	try {
		//TES 1/5/2010 - PLID 35774 - Check permission.
		// (j.gruber 2010-10-26 13:56) - PLID 40416 - split permission to assign		
		CPermissions permWrite = GetCurrentUserPermissions(bioSecurityGroup) & SPT___W_______;
		CPermissions permWriteWithPass = GetCurrentUserPermissions(bioSecurityGroup) & SPT___W________ONLYWITHPASS;
		CPermissions permDynamic = GetCurrentUserPermissions(bioSecurityGroup) & SPT______0____;
		CPermissions permDynamicWithPass = GetCurrentUserPermissions(bioSecurityGroup) & SPT______0_____ONLYWITHPASS;

		BOOL bpermWrite = permWrite & sptWrite;
		BOOL bpermWriteWithPass = permWriteWithPass & sptWriteWithPass;
		BOOL bpermDynamic = permDynamic & sptDynamic0;
		BOOL bpermDynamicWithPass = permDynamicWithPass & sptDynamic0WithPass;
		BOOL bIsAdmin = IsCurrentUserAdministrator();

		//if they only have Dynamic with pass prompt them
		BOOL bGreyList = FALSE;
		if (bIsAdmin) {
			bGreyList = FALSE;
		}
		else {
			if (!bpermWrite && !bpermWriteWithPass && !bpermDynamic && !bpermDynamicWithPass ) {
				//they have nothing
				return;
			}
			else if (bpermDynamicWithPass) {
				//let's check their password
				if (!CheckCurrentUserPassword()) {
					//they failed, but can they write
					if (!bpermWrite && !bpermWriteWithPass) {
						return;
					}			
					else {
						//tell the idalog to grey out the list since they failed the check
						bGreyList = TRUE;
					}
				}
			}		
		}
		//TES 1/5/2010 - PLID 35774 - Pop up a dialog to allow them to assign this patient to various groups.
		// This button is disabled for this checkin, still need to do things like check permissions.
		// (j.gruber 2011-01-13 15:11) - PLID 40415 - we always want to update the current toolbar here.
		CSecurityGroupsDlg dlg(this, bGreyList, true);
		dlg.m_nPatientID = m_id;
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-02-23 10:21) - PLID 37364 - This function ensures that the Mirror image manager exists
// and is ready for getting image counts and loading images
void CGeneral1Dlg::EnsureMirrorImageMgr()
{
	if (NULL == m_pMirrorImageMgr) {
		m_pMirrorImageMgr = new CMirrorPatientImageMgr(m_id);
	}
}

// (c.haag 2010-02-23 15:08) - PLID 37364 - This function ensures that the Mirror image manager does
// not exist. If it did, it is deleted.
void CGeneral1Dlg::EnsureNotMirrorImageMgr()
{
	if (NULL != m_pMirrorImageMgr) {
		delete m_pMirrorImageMgr;
		m_pMirrorImageMgr = NULL;
	}
}

// (c.haag 2010-02-23 10:22) - PLID 37364 - Returns the patient's Mirror image count. This only includes
// images that were not imported by the Mirror image import app
long CGeneral1Dlg::GetMirrorImageCount()
{
	EnsureMirrorImageMgr();
	return m_pMirrorImageMgr->GetImageCount();
}

// (c.haag 2010-02-23 10:22) - PLID 37364 - Loads a Mirror image. The input index must be between 0 and
// the number of patient images that were not imported by the Mirror image import app
HBITMAP CGeneral1Dlg::LoadMirrorImage(long &nIndex, long &nCount, long nQualityOverride)
{
	EnsureMirrorImageMgr();
	return m_pMirrorImageMgr->LoadMirrorImage(nIndex, nCount, nQualityOverride);
}

// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the first valid Mirror image index for this patient
long CGeneral1Dlg::GetFirstValidMirrorImageIndex(const CString& strMirrorID)
{
	EnsureMirrorImageMgr();
	return m_pMirrorImageMgr->GetFirstValidImageIndex();
}

// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the last valid Mirror image index for this patient
long CGeneral1Dlg::GetLastValidMirrorImageIndex(const CString& strMirrorID)
{
	EnsureMirrorImageMgr();
	return m_pMirrorImageMgr->GetLastValidImageIndex();
}

// (d.lange 2010-06-30 15:40) - PLID 38687 - This function determines whether we have any device plugins that support the
// 'LaunchDevice' functionality
// (j.gruber 2013-04-02 16:28) - PLID 56012 - renamed and moved to launchutils

// (d.lange 2010-06-30 15:50) - PLID 38687 - Generate a menu for only devices if everything else is disabled
void CGeneral1Dlg::GenerateDeviceMenu()
{
	try {
		CMenu mnu;
		mnu.CreatePopupMenu();
		long nIndex = 0;

		// (d.lange 2010-06-22 16:21) - PLID 38687 - Set up a menu item for sending patient demographics to an external device
		//Determine all the device plugins that are enabled
		// (j.gruber 2013-04-02 16:17) - PLID 56012 - consolidate
		CArray<DeviceLaunchUtils::DevicePlugin*, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;				
		DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, &mnu, nIndex, TRUE, MF_BYPOSITION, FALSE);
		
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_SEND_TO_THIRD_PARTY);
		long nResult = 0;
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}
		
		if(nResult > 0) {
			// (j.gruber 2013-04-02 16:22) - PLID 56012 - Consolidate
			DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nResult, GetActivePatientID());			
			
		}

		//Let's iterate through the array of loaded plugins and unload them
		// (j.gruber 2013-04-02 16:23) - PLID 56012 - consolidate
		DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);		

	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-1-11) PLID 47485 - function to handle a scan from the barcode scanner
LRESULT CGeneral1Dlg::OnBarcodeScannerDataEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2012-01-17 12:55) - PLID 47120 - lParam is a BSTR which we now own
		_bstr_t bstrData((BSTR)lParam, false);
		CString strDLData = (LPCTSTR)bstrData;

		//always success, but still check anyway.
		if ((long)wParam == OPOS_SUCCESS) {
			//check permissions of user
			if (!CheckCurrentUserPermissions(bioPatient, SPT___W_______, FALSE, 0, TRUE)) {
				MessageBox("You do not have permission to write patient information, "
					"so you cannot retrieve patient demographics from "
					"a driver's license.  Please see your office manager.", 
					"2D Barcode Scanner", MB_ICONWARNING|MB_OK);

				return 0;
			}
			//warn user of overwritten data.
			int nResult = MessageBox(
			"A driver's license has just been scanned.  The information from the license will fill the "
			"corresponding demographic fields for this patient, so any information already in those "
			"fields will be overwritten.\r\n\r\n"
			"Are you sure you want to import the information from the driver's license?", 
			"2D Barcode Scanner", MB_YESNO|MB_ICONQUESTION);
			if (nResult == IDYES) {
				//parse the barcode data into a struct for ease of use.
				BarcodeUtils::DriversLicenseInfo dlInfo(strDLData);
				//(a.wilson 2012-1-19) PLID 47485 - changed name to better suit its additional purpose
				m_bProcessingCard = true;
				//first
				SetDlgItemText(IDC_FIRST_NAME_BOX, dlInfo.strFirst);
				m_changed = true;
				Save(IDC_FIRST_NAME_BOX);
				//middle
				SetDlgItemText(IDC_MIDDLE_NAME_BOX, dlInfo.strMiddle);
				m_changed = true;
				Save(IDC_MIDDLE_NAME_BOX);
				//last
				SetDlgItemText(IDC_LAST_NAME_BOX, dlInfo.strLast);
				m_changed = true;
				Save(IDC_LAST_NAME_BOX);
				//suffix
				SetDlgItemText(IDC_TITLE_BOX, dlInfo.strSuffix);
				m_changed = true;
				Save(IDC_TITLE_BOX);
				//address
				SetDlgItemText(IDC_ADDRESS1_BOX, dlInfo.strAddress);
				m_changed = true;
				Save(IDC_ADDRESS1_BOX);
				SetDlgItemText(IDC_ADDRESS2_BOX, "");
				m_changed = true;
				Save(IDC_ADDRESS2_BOX);
				//city
				SetDlgItemText(IDC_CITY_BOX, dlInfo.strCity);
				m_changed = true;
				Save(IDC_CITY_BOX);
				//state
				SetDlgItemText(IDC_STATE_BOX, dlInfo.strState);
				m_changed = true;
				Save(IDC_STATE_BOX);
				//zip
				SetDlgItemText(IDC_ZIP_BOX, dlInfo.strZip);
				m_changed = true;
				Save(IDC_ZIP_BOX);
				//gender
				if (dlInfo.strGender.CompareNoCase("male") == 0) {
					m_GenderCombo->SetSelByColumn(0, "Male");
					OnSelChosenGenderList(1);
				}
				else if (dlInfo.strGender.CompareNoCase("female") == 0) {
					m_GenderCombo->SetSelByColumn(0, "Female");
					OnSelChosenGenderList(2);
				}else {
					m_GenderCombo->SetSelByColumn(0, "");
					OnSelChosenGenderList(-1);
				}
				//birthdate
				COleDateTime cdtBirthdate;
				cdtBirthdate.ParseDateTime(dlInfo.strDOB, VAR_DATEVALUEONLY);

				if (cdtBirthdate.GetStatus() != COleDateTime::invalid && 
					cdtBirthdate.GetStatus() != COleDateTime::error && 
					cdtBirthdate.GetStatus() != COleDateTime::null) {
					
					m_nxtBirthDate->SetDateTime(cdtBirthdate);
					m_changed = true;
					Save(IDC_BIRTH_DATE_BOX);
				} else {
					m_nxtBirthDate->Clear();
					m_changed = true;
					Save(IDC_BIRTH_DATE_BOX);
				}
				//(a.wilson 2012-1-19) PLID 47485 - changed name to better suit its additional purpose
				m_bProcessingCard = false;
			}
		}
	} NxCatchAll(__FUNCTION__);
	
	return TRUE;
}

// (j.armen 2012-02-24 16:27) - PLID 48303
void CGeneral1Dlg::OnBtnRecallClicked()
{
	try {
		GetMainFrame()->ShowRecallsNeedingAttention(true);
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-02-27 16:42) - PLID 48354 Vision Prescription
void CGeneral1Dlg::OnBnClickedShowVisionPrescription()
{
	try
	{
		CGlassesEMNPrescriptionList dlg;
		dlg.m_strPatientName = m_strPatientName;
		// (s.dhole 2012-04-25 12:35) - PLID 47395 Change name
		dlg.m_PrescriptionWindowDisplayType = dlg.pwShowRxList;
		dlg.m_nPatientID = m_id;
		dlg.m_bCalledForGlassesAndContacts = true;
		//(r.wilson 6/4/2012) PLID 48952
		dlg.m_bHidePrintBtn = false;
		dlg.DoModal() ;
	} NxCatchAll(__FUNCTION__);
}

//(s.dhole 8/28/2014 1:39 PM ) - PLID 62747
void CGeneral1Dlg::OnBnClickedBtnSentReminder()
{
	try
	{
		CString slast, sfirst;
		GetDlgItemText(IDC_FIRST_NAME_BOX, sfirst);
		GetDlgItemText(IDC_LAST_NAME_BOX, slast);
		//(s.dhole 8/29/2014 4:04 PM ) - PLID 62751
		CPatientReminderSenthistoryDlg dlg(this, m_id, FormatString("%s, %s", slast, sfirst));
		dlg.DoModal();
		SetReminderLabel();
	} NxCatchAll(__FUNCTION__);
}


// (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
//(s.dhole 8/28/2014 1:39 PM ) - PLID 62747 Set Reminder label text 
void CGeneral1Dlg::SetReminderLabel()
{
	// TODO: Add your control notification handler code here
	_RecordsetPtr recordsetPtr = CreateParamRecordset("SELECT TOP 1 ReminderDate FROM PatientRemindersSentT WHERE   PatientID =  {INT} AND Deleted = 0 ORDER BY ReminderDate DESC", m_id);
	// If nothing was found
	if (!recordsetPtr->eof)
	{
		SetDlgItemText(IDC_STATIC_REMINDER, FormatString("Last: %s", FormatDateTimeForInterface(AdoFldDateTime(recordsetPtr, "ReminderDate"), DTF_STRIP_SECONDS, dtoDateTime)));
		m_nxstaticReminder.ShowWindow(SW_SHOW);
	}
	else
	{
		SetDlgItemText(IDC_STATIC_REMINDER, "");
		m_nxstaticReminder.ShowWindow(SW_HIDE);
	}
	
}

