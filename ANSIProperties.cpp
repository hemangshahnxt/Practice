// ANSIProperties.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ANSIProperties.h"
#include "globaldatautils.h"
#include "GlobalDrawingUtils.h"
#include "DontShowDlg.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CANSIProperties dialog

CANSIProperties::CANSIProperties(CWnd* pParent /*=NULL*/)
	: CNxDialog(CANSIProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CANSIProperties)
		m_bShowAdvOptions = FALSE;
		// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010
		m_avLastSavedANSIVersion = av5010;
		m_avLastCheckedANSIVersion = av5010;
	//}}AFX_DATA_INIT
}


void CANSIProperties::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CANSIProperties)
	DDX_Control(pDX, IDC_HIDE_2300_PAT_PAID_WHEN_ZERO, m_checkHide2300PatAmtPaidWhenZero);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, ID_OK_BTN, m_btnOK);
	DDX_Control(pDX, IDC_DELETE_ANSI, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_SHOW_ADV_OPTIONS, m_btnShowAdvanced);
	DDX_Control(pDX, IDC_ADD_ANSI, m_btnAdd);
	DDX_Control(pDX, IDC_CHECK_EXPORT_2330B_ADDRESS, m_checkExport2330BAddress);
	DDX_Control(pDX, IDC_ALWAYS_RESEND_SUBMITTER_LOOPS, m_checkSendSubscriberPerClaim);
	DDX_Control(pDX, IDC_CHECK_FOUR_DIGIT_REV_CODE, m_checkFourDigitRevCode);
	DDX_Control(pDX, IDC_CHECK_1000A_PER, m_check1000APER);
	DDX_Control(pDX, IDC_SEPARATE_BATCH_BY_INSURANCE_CO, m_checkSeparateBatchesByInsCo);
	DDX_Control(pDX, IDC_CHECK_USE_ADDITIONAL_2010AA_ID, m_checkUseAddnl2010AAID);
	DDX_Control(pDX, IDC_CHECK_PREPEND_PAYER_NSF, m_checkPrependPayerNSF);
	DDX_Control(pDX, IDC_CHECK_EXPORT_2330B_PER_SEGMENT, m_checkExport2330BPERSegment);
	DDX_Control(pDX, IDC_CHECK_TRUNCATE_CURRENCY, m_checkTruncateCurrency);
	DDX_Control(pDX, IDC_CHECK_USE_SSN, m_checkUseSSN);
	DDX_Control(pDX, IDC_EXPORT_ALL_2010AA_IDS, m_checkExportAll2010AAIDs);
	//DDX_Control(pDX, IDC_CHECK_USE_THIN_PAYER_ID, m_checkUseTHINPayerIDs);
	DDX_Control(pDX, IDC_USE_2010AB, m_checkUse2010AB);
	DDX_Control(pDX, IDC_DONT_SUBMIT_SECONDARIES, m_DontSubmitSecondary);
	DDX_Control(pDX, IDC_HIDE_2310D, m_checkHide2310D);
	DDX_Control(pDX, IDC_HIDE_2310C_WHENBILLLOC, m_checkHide2310CWhenBillLocation);
	DDX_Control(pDX, IDC_UNPUNCTUATE_ANSI, m_UnPunctuate);
	DDX_Control(pDX, IDC_USE_SV106_FIELD, m_checkUseSV106);	
	DDX_Control(pDX, IDC_RADIO_INDIV, m_radioIndiv);
	DDX_Control(pDX, IDC_RADIO_GROUP, m_radioGroup);
	DDX_Control(pDX, IDC_USE_2420A, m_checkUse2420A);
	DDX_Control(pDX, IDC_ZIP_ANSI, m_zipfile);
	DDX_Control(pDX, IDC_CAPITALIZE_ANSI, m_capitalize);
	DDX_Control(pDX, IDC_RECEIVER_NAME, m_nxeditReceiverName);
	DDX_Control(pDX, IDC_ANSI_CONTACT, m_nxeditAnsiContact);
	DDX_Control(pDX, IDC_EDIT_ISA07_QUAL, m_nxeditEditIsa07Qual);
	DDX_Control(pDX, IDC_RECEIVER_ID_ISA08, m_nxeditReceiverIdIsa08);
	DDX_Control(pDX, IDC_RECEIVER_ID_GS03, m_nxeditReceiverIdGs03);
	DDX_Control(pDX, IDC_EDIT_1000B_QUAL, m_nxeditEdit1000BQual);
	DDX_Control(pDX, IDC_RECEIVER_ID_1000B, m_nxeditReceiverId1000B);
	DDX_Control(pDX, IDC_EDIT_ISA05_QUAL, m_nxeditEditIsa05Qual);
	DDX_Control(pDX, IDC_SUBMITTER_ID_ISA06, m_nxeditSubmitterIdIsa06);
	DDX_Control(pDX, IDC_SUBMITTER_ID_GS02, m_nxeditSubmitterIdGs02);
	DDX_Control(pDX, IDC_EDIT_1000A_QUAL, m_nxeditEdit1000AQual);
	DDX_Control(pDX, IDC_SUBMITTER_ID_1000A, m_nxeditSubmitterId1000A);
	DDX_Control(pDX, IDC_EDIT_1000A_PER05_QUAL, m_nxeditEdit1000APer05Qual);
	DDX_Control(pDX, IDC_EDIT_1000A_PER06_ID, m_nxeditEdit1000APer06Id);
	DDX_Control(pDX, IDC_EDIT_ISA01_QUAL, m_nxeditEditIsa01Qual);
	DDX_Control(pDX, IDC_EDIT_ISA02, m_nxeditEditIsa02);
	DDX_Control(pDX, IDC_EDIT_ISA03_QUAL, m_nxeditEditIsa03Qual);
	DDX_Control(pDX, IDC_EDIT_ISA04, m_nxeditEditIsa04);
	DDX_Control(pDX, IDC_EDIT_ADDNL_2010AA_QUAL, m_nxeditEditAddnl2010AaQual);
	DDX_Control(pDX, IDC_EDIT_ADDNL_2010AA, m_nxeditEditAddnl2010Aa);
	DDX_Control(pDX, IDC_FILENAME_ANSI, m_nxeditFilenameAnsi);
	DDX_Control(pDX, IDC_FILENAME_ELIG, m_nxeditFilenameElig);
	DDX_Control(pDX, IDC_ELIG_FILE_NAME_LABEL, m_nxstaticFilenameEligLabel);
	DDX_Control(pDX, IDC_CHECK_PREPEND_PATIENT_ID, m_checkPrependPatientID);
	DDX_Control(pDX, IDC_EDIT_PREPEND_PATIENT_ID, m_nxeditPrependPatientID);
	DDX_Control(pDX, IDC_RADIO_4010, m_radio4010);
	DDX_Control(pDX, IDC_RADIO_5010, m_radio5010);
	// (j.jones 2012-07-20 11:32) - PLID 47901 - added control for the ANSI Version label
	DDX_Control(pDX, IDC_LABEL_ANSI_VERSION, m_nxstaticANSIVersionLabel);
	// (a.wilson 2014-06-27 09:55) - PLID 62517 - control for the dontsendsecondaryondiagnosismismatch
	DDX_Control(pDX, IDC_DONT_SEND_SECONDARY_ON_DIAGNOSIS_MISMATCH, m_DontSendSecondaryDiagnosisMismatch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CANSIProperties, CNxDialog)
	//{{AFX_MSG_MAP(CANSIProperties)
	ON_BN_CLICKED(ID_OK_BTN, OnOkBtn)
	ON_BN_CLICKED(IDC_ADD_ANSI, OnAddAnsi)
	ON_BN_CLICKED(IDC_DELETE_ANSI, OnDeleteAnsi)
	ON_BN_CLICKED(IDC_BTN_SHOW_ADV_OPTIONS, OnBtnShowAdvOptions)
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN IDs
	//ON_BN_CLICKED(IDC_CHECK_USE_THIN_PAYER_ID, OnCheckUseThinPayerId)	
	ON_BN_CLICKED(IDC_CHECK_USE_ADDITIONAL_2010AA_ID, OnCheckUseAdditional2010aaId)
	ON_BN_CLICKED(IDC_SEPARATE_BATCH_BY_INSURANCE_CO, OnSeparateBatchByInsuranceCo)
	ON_BN_CLICKED(IDC_CHECK_1000A_PER, OnCheck1000aPer)
	ON_BN_CLICKED(IDC_CHECK_PREPEND_PATIENT_ID, &CANSIProperties::OnCheckPrependPatientId)
	ON_BN_CLICKED(IDC_RADIO_4010, OnRadio4010)
	ON_BN_CLICKED(IDC_RADIO_5010, OnRadio5010)
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_DONT_SUBMIT_SECONDARIES, &CANSIProperties::OnBnClickedDontSubmitSecondaries)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CANSIProperties message handlers

void CANSIProperties::OnOkBtn() 
{
	CString str;

	if(!Save())
		return;
	
	CDialog::OnOK();
}

BOOL CANSIProperties::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_FINANCIAL, 0)));

		// (j.jones 2008-05-07 10:58) - PLID 29854 - added nxiconbuttons for modernization
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		m_radioGroup.SetCheck(FALSE);
		m_radioIndiv.SetCheck(TRUE);

		// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
		m_radio4010.SetCheck(FALSE);
		m_radio5010.SetCheck(TRUE);
		// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010
		m_avLastSavedANSIVersion = av5010;
		m_avLastCheckedANSIVersion = av5010;

		m_ANSIList = BindNxDataListCtrl(this,IDC_ANSI_STYLE_LIST,GetRemoteData(),TRUE);

		m_ANSIList->SetSelByColumn(0,m_FormatID);

		ShowAdvOptions(m_bShowAdvOptions);

		// (j.jones 2007-04-11 09:16) - PLID 25513 - ensured we called SetLimitText on all editable fields
		((CNxEdit*)GetDlgItem(IDC_RECEIVER_NAME))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_ANSI_CONTACT))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ISA07_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_RECEIVER_ID_ISA08))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_RECEIVER_ID_GS03))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_1000B_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_RECEIVER_ID_1000B))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ISA05_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_SUBMITTER_ID_ISA06))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_SUBMITTER_ID_GS02))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_1000A_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_SUBMITTER_ID_1000A))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_1000A_PER05_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_EDIT_1000A_PER06_ID))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ISA01_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ISA02))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ISA03_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ISA04))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ADDNL_2010AA_QUAL))->SetLimitText(10);
		((CNxEdit*)GetDlgItem(IDC_EDIT_ADDNL_2010AA))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_FILENAME_ANSI))->SetLimitText(50);
		// (j.jones 2008-10-13 12:50) - PLID 31636 - added eligibility filename
		((CNxEdit*)GetDlgItem(IDC_FILENAME_ELIG))->SetLimitText(50);
		// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
		m_nxeditPrependPatientID.SetLimitText(10);

		// (j.jones 2008-10-13 15:04) - PLID 31636 - hide the eligibility info.
		// if they don't have the license
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)) {
			GetDlgItem(IDC_ELIG_FILE_NAME_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FILENAME_ELIG)->ShowWindow(SW_HIDE);
		}
		
		Load();

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CANSIProperties::Save()
{
	try {

		CString strReceiverName, strContact,
			strReceiverISA07Qual, strReceiverISA08ID, strReceiverGS03ID, strReceiver1000BQual, strReceiver1000BID,
			strSubmitterISA05Qual, strSubmitterISA06ID, strSubmitterGS02ID, strSubmitter1000AQual, strSubmitter1000AID,
			strFilename, strISA01Qual, strISA02, strISA03Qual, strISA04, strAddnl2010AAQual, strAddnl2010AA, 
			strPER05Qual_1000A, strPER06ID_1000A, strFilenameElig, strPrependPatientID, str;
	
		GetDlgItemText(IDC_RECEIVER_NAME,strReceiverName);
		GetDlgItemText(IDC_ANSI_CONTACT,strContact);
		GetDlgItemText(IDC_EDIT_ISA07_QUAL,strReceiverISA07Qual);
		GetDlgItemText(IDC_RECEIVER_ID_ISA08,strReceiverISA08ID);
		GetDlgItemText(IDC_RECEIVER_ID_GS03,strReceiverGS03ID);
		GetDlgItemText(IDC_EDIT_1000B_QUAL,strReceiver1000BQual);
		GetDlgItemText(IDC_RECEIVER_ID_1000B,strReceiver1000BID);
		GetDlgItemText(IDC_EDIT_ISA05_QUAL,strSubmitterISA05Qual);
		GetDlgItemText(IDC_SUBMITTER_ID_ISA06,strSubmitterISA06ID);
		GetDlgItemText(IDC_SUBMITTER_ID_GS02,strSubmitterGS02ID);
		GetDlgItemText(IDC_EDIT_1000A_QUAL,strSubmitter1000AQual);
		GetDlgItemText(IDC_SUBMITTER_ID_1000A,strSubmitter1000AID);
		GetDlgItemText(IDC_EDIT_ISA05_QUAL,strSubmitterISA05Qual);
		GetDlgItemText(IDC_SUBMITTER_ID_ISA06,strSubmitterISA06ID);
		GetDlgItemText(IDC_EDIT_ADDNL_2010AA_QUAL,strAddnl2010AAQual);
		GetDlgItemText(IDC_EDIT_ADDNL_2010AA,strAddnl2010AA);		
		GetDlgItemText(IDC_FILENAME_ANSI,strFilename);
		// (j.jones 2008-10-13 12:50) - PLID 31636 - added eligibility filename
		GetDlgItemText(IDC_FILENAME_ELIG, strFilenameElig);

		GetDlgItemText(IDC_EDIT_ISA01_QUAL,strISA01Qual);
		GetDlgItemText(IDC_EDIT_ISA02, strISA02);
		GetDlgItemText(IDC_EDIT_ISA03_QUAL, strISA03Qual);
		GetDlgItemText(IDC_EDIT_ISA04, strISA04);		

		GetDlgItemText(IDC_EDIT_1000A_PER05_QUAL, strPER05Qual_1000A);
		GetDlgItemText(IDC_EDIT_1000A_PER06_ID, strPER06ID_1000A);

		// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
		GetDlgItemText(IDC_EDIT_PREPEND_PATIENT_ID, strPrependPatientID);
		strPrependPatientID.TrimLeft();
		strPrependPatientID.TrimRight();

		//validate the file name
		int pos = strFilename.Find("%");
		if(pos != -1) {
			//they are trying to use the batch number
			int pos2 = strFilename.Find("b",pos);
			// (j.jones 2008-05-01 09:30) - PLID 27832 - added a restriction from using % without %b
			if(pos2 == -1) {
				AfxMessageBox("You cannot put a % symbol in the claim file name without using %b or %#b to format the batch number, where the # is greater than 0.");
				return FALSE;
			}
			else if((pos2 - pos) > 2) {
				AfxMessageBox("Please only put one digit in the %b specification of the claim filename.");
				return FALSE;
			}
			else {
				//check that it is %, a number, and b, and nothing else
				CString strMid = strFilename.Mid(pos, pos2 - pos);
				strMid.TrimLeft("%");
				strMid.TrimRight("b");
				if(!strMid.IsEmpty() && atoi(strMid) == 0) {
					AfxMessageBox("You cannot put a % symbol in the claim file name without using %b or %#b to format the batch number, where the # is greater than 0.");
					return FALSE;
				}
			}
		}

		// (j.jones 2008-10-13 12:50) - PLID 31636 - validate the eligibility filename
		pos = strFilenameElig.Find("%");
		if(pos != -1) {
			//they are trying to use the batch number
			int pos2 = strFilenameElig.Find("b",pos);
			//don't let them use the % without %b
			if(pos2 == -1) {
				AfxMessageBox("You cannot put a % symbol in the eligibility file name without using %b or %#b to format the batch number, where the # is greater than 0.");
				return FALSE;
			}
			else if((pos2 - pos) > 2) {
				AfxMessageBox("Please only put one digit in the %b specification of the eligibility filename.");
				return FALSE;
			}
			else {
				//check that it is %, a number, and b, and nothing else
				CString strMid = strFilenameElig.Mid(pos, pos2 - pos);
				strMid.TrimLeft("%");
				strMid.TrimRight("b");
				if(!strMid.IsEmpty() && atoi(strMid) == 0) {
					AfxMessageBox("You cannot put a % symbol in the eligibility file name without using %b or %#b to format the batch number, where the # is greater than 0.");
					return FALSE;
				}
			}
		}

		// (j.jones 2010-04-16 08:52) - PLID 38149 - flat out disallow XX to be used here, in any circumstance
		if(m_checkUseAddnl2010AAID.GetCheck() && strAddnl2010AAQual.CompareNoCase("XX") == 0) {
			AfxMessageBox("The Additional 2010AA ID has an XX qualifier, which is not a valid qualifier to use.\n"
				"Please correct this qualifier before continuing.");
			return FALSE;
		}

		// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
		// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010
		ANSIVersion avVersion = av5010;
		if(m_radio4010.GetCheck()) {
			avVersion = av4010;
		}

		// (j.jones 2007-04-05 10:26) - PLID 25506 - added the 1000A PER05/06 override
		// (j.jones 2007-05-02 09:34) - PLID 25855 - added the FourDigitRevCode option
		// (j.jones 2008-02-15 11:51) - PLID 28943 - added SendSubscriberPerClaim
		// (j.jones 2008-02-19 11:45) - PLID 29004 - added checkbox for Send2330BAddress
		// (j.jones 2008-09-09 16:45) - PLID 26482 - added HidePatAmtPaid
		// (j.jones 2008-10-13 12:50) - PLID 31636 - added the eligibility filename
		// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF,
		// and removed the THIN payer IDs
		// (j.jones 2009-10-01 14:11) - PLID 35711 - added PrependPatientID and PrependPatientIDCode
		// (j.jones 2012-01-06 16:15) - PLID 47351 - added Hide2310CWhenBillLocation
		// (a.wilson 2014-06-27 09:49) - PLID 62517 - parameterized query.  also added saving DontSendSecondaryOnDiagMismatch.
		ExecuteParamSql(R"(
		UPDATE	EBillingFormatsT 
		SET		Name = {STRING}, Contact = {STR}, ReceiverISA07Qual = {STR}, ReceiverISA08ID = {STR}, ReceiverGS03ID = {STR}, 
				Receiver1000BQual = {STR}, Receiver1000BID = {STR}, SubmitterISA05Qual = {STR}, SubmitterISA06ID = {STR}, 
				SubmitterGS02ID = {STR}, Submitter1000AQual = {STR}, Submitter1000AID = {STR}, Capitalize = {INT}, 
				Filename = {STR}, FilenameElig = {STR}, Zipped = {INT}, IsGroup = {INT}, Use2420A = {INT}, UnPunctuate = {INT}, 
				Hide2310D = {INT}, UseSV106 = {INT}, DontSubmitSecondary = {INT}, Use2010AB = {INT}, ISA01Qual = {STR}, 
				ISA02 = {STR}, ISA03Qual = {STR}, ISA04 = {STR}, ExportAll2010AAIDs = {INT}, UseSSN = {INT}, TruncateCurrency = {INT}, 
				Export2330BPER = {INT}, PrependPayerNSF = {INT}, Use_Addnl_2010AA = {INT}, Addnl_2010AA_Qual = {STR}, Addnl_2010AA = {STR}, 
				SeparateBatchesByInsCo = {INT}, Use1000APER = {INT}, PER05Qual_1000A = {STR}, PER06ID_1000A = {STR}, FourDigitRevCode = {INT}, 
				SendSubscriberPerClaim = {INT}, Send2330BAddress = {INT}, HidePatAmtPaid = {INT}, PrependPatientID = {INT}, PrependPatientIDCode = {STR}, 
				ANSIVersion = {INT}, Hide2310CWhenBillLocation = {INT}, DontSendSecondaryOnDiagMismatch = {INT} 
		WHERE	ID = {INT})", 
		strReceiverName, strContact, strReceiverISA07Qual, strReceiverISA08ID, strReceiverGS03ID, strReceiver1000BQual, strReceiver1000BID, 
		strSubmitterISA05Qual, strSubmitterISA06ID, strSubmitterGS02ID, strSubmitter1000AQual, strSubmitter1000AID, (m_capitalize.GetCheck() ? 1 : 0), 
		strFilename, strFilenameElig, (m_zipfile.GetCheck() ? 1 : 0), (m_radioGroup.GetCheck() ? 1 : 0), (m_checkUse2420A.GetCheck() ? 1 : 0), 
		(m_UnPunctuate.GetCheck() ? 1 : 0), (m_checkHide2310D.GetCheck() ? 1 : 0), (m_checkUseSV106.GetCheck() ? 1 : 0), 
		(m_DontSubmitSecondary.GetCheck() ? 1 : 0), (m_checkUse2010AB.GetCheck() ? 1 : 0), strISA01Qual, strISA02, strISA03Qual, strISA04, 
		(m_checkExportAll2010AAIDs.GetCheck() ? 1 : 0), (m_checkUseSSN.GetCheck() ? 1 : 0), (m_checkTruncateCurrency.GetCheck() ? 1 : 0), 
		(m_checkExport2330BPERSegment.GetCheck() ? 1 : 0), (m_checkPrependPayerNSF.GetCheck() ? 1 : 0), (m_checkUseAddnl2010AAID.GetCheck() ? 1 : 0), 
		strAddnl2010AAQual, strAddnl2010AA, (m_checkSeparateBatchesByInsCo.GetCheck() ? 1 : 0), (m_check1000APER.GetCheck() ? 1 : 0), 
		strPER05Qual_1000A, strPER06ID_1000A, (m_checkFourDigitRevCode.GetCheck() ? 1 : 0), (m_checkSendSubscriberPerClaim.GetCheck() ? 1 : 0), 
		(m_checkExport2330BAddress.GetCheck() ? 1 : 0), (m_checkHide2300PatAmtPaidWhenZero.GetCheck() ? 1 : 0), 
		(m_checkPrependPatientID.GetCheck() ? 1 : 0), strPrependPatientID, (long)avVersion, (m_checkHide2310CWhenBillLocation.GetCheck() ? 1 : 0), 
		(m_DontSendSecondaryDiagnosisMismatch.GetCheck() ? 1 : 0), m_FormatID);

		// (j.jones 2010-10-11 08:55) - PLID 40878 - audit the ANSI version change
		if(m_avLastSavedANSIVersion != avVersion) {
			long nAuditID = BeginNewAuditEvent();
			CString strOld;
			strOld.Format("%s (Export Name: %s)", m_avLastSavedANSIVersion == av4010 ? "Version 4010" : "Version 5010", strReceiverName);
			AuditEvent(-1, "", nAuditID, aeiANSIVersion, m_FormatID, strOld, avVersion == av4010 ? "Version 4010" : "Version 5010", aepHigh, aetChanged);
		}

		m_avLastSavedANSIVersion = avVersion;
		m_avLastCheckedANSIVersion = avVersion;

		return TRUE;

	}NxCatchAll("Error in ANSIProperties::Save()");

	return FALSE;
}

void CANSIProperties::Load()
{
	CString str = "";
	long tempint = -1;

	try {

		_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM EbillingFormatsT WHERE ID = {INT}", m_FormatID);

		if(!rs->eof) {			
			SetDlgItemText(IDC_RECEIVER_NAME,AdoFldString(rs, "Name",""));
			SetDlgItemText(IDC_ANSI_CONTACT,AdoFldString(rs, "Contact",""));
			SetDlgItemText(IDC_EDIT_ISA07_QUAL,AdoFldString(rs, "ReceiverISA07Qual",""));
			SetDlgItemText(IDC_RECEIVER_ID_ISA08,AdoFldString(rs, "ReceiverISA08ID",""));
			SetDlgItemText(IDC_RECEIVER_ID_GS03,AdoFldString(rs, "ReceiverGS03ID",""));
			SetDlgItemText(IDC_EDIT_1000B_QUAL,AdoFldString(rs, "Receiver1000BQual",""));
			SetDlgItemText(IDC_RECEIVER_ID_1000B,AdoFldString(rs, "Receiver1000BID",""));
			SetDlgItemText(IDC_EDIT_ISA05_QUAL,AdoFldString(rs, "SubmitterISA05Qual",""));
			SetDlgItemText(IDC_SUBMITTER_ID_ISA06,AdoFldString(rs, "SubmitterISA06ID",""));
			SetDlgItemText(IDC_SUBMITTER_ID_GS02,AdoFldString(rs, "SubmitterGS02ID",""));
			SetDlgItemText(IDC_EDIT_1000A_QUAL,AdoFldString(rs, "Submitter1000AQual",""));
			SetDlgItemText(IDC_SUBMITTER_ID_1000A,AdoFldString(rs, "Submitter1000AID",""));
			SetDlgItemText(IDC_FILENAME_ANSI,AdoFldString(rs, "Filename",""));
			// (j.jones 2008-10-13 12:50) - PLID 31636 - added the eligibility filename
			SetDlgItemText(IDC_FILENAME_ELIG, AdoFldString(rs, "FilenameElig",""));
			SetDlgItemText(IDC_EDIT_ISA01_QUAL,AdoFldString(rs, "ISA01Qual",""));
			SetDlgItemText(IDC_EDIT_ISA02, AdoFldString(rs, "ISA02",""));
			SetDlgItemText(IDC_EDIT_ISA03_QUAL, AdoFldString(rs, "ISA03Qual",""));
			SetDlgItemText(IDC_EDIT_ISA04, AdoFldString(rs, "ISA04",""));
			m_capitalize.SetCheck(AdoFldBool(rs, "Capitalize",FALSE));
			m_UnPunctuate.SetCheck(AdoFldBool(rs, "UnPunctuate",FALSE));
			m_zipfile.SetCheck(AdoFldBool(rs, "Zipped",FALSE));
			m_checkUse2420A.SetCheck(AdoFldBool(rs, "Use2420A",FALSE));
			m_checkHide2310D.SetCheck(AdoFldBool(rs, "Hide2310D",FALSE));
			// (j.jones 2012-01-06 16:15) - PLID 47351 - added Hide2310CWhenBillLocation
			m_checkHide2310CWhenBillLocation.SetCheck(AdoFldBool(rs, "Hide2310CWhenBillLocation",FALSE));
			m_checkUseSV106.SetCheck(AdoFldBool(rs, "UseSV106",FALSE));			
			m_DontSubmitSecondary.SetCheck(AdoFldBool(rs, "DontSubmitSecondary",FALSE));
			// (a.wilson 2014-06-27 09:11) - PLID 62517 - load new checkbox option.
			m_DontSendSecondaryDiagnosisMismatch.SetCheck(AdoFldBool(rs, "DontSendSecondaryOnDiagMismatch", TRUE));
			if (m_DontSubmitSecondary.GetCheck())
				m_DontSendSecondaryDiagnosisMismatch.EnableWindow(FALSE);
			else
				m_DontSendSecondaryDiagnosisMismatch.EnableWindow(TRUE);

			m_checkUse2010AB.SetCheck(AdoFldBool(rs, "Use2010AB",FALSE));
			// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN IDs
			//m_checkUseTHINPayerIDs.SetCheck(AdoFldBool(rs, "UseTHINPayerIDs",FALSE));
			//OnCheckUseThinPayerId();
			m_checkExportAll2010AAIDs.SetCheck(AdoFldBool(rs, "ExportAll2010AAIDs",FALSE));
			m_checkUseSSN.SetCheck(AdoFldBool(rs, "UseSSN",FALSE));
			m_checkTruncateCurrency.SetCheck(AdoFldBool(rs, "TruncateCurrency",FALSE));
			m_checkExport2330BPERSegment.SetCheck(AdoFldBool(rs, "Export2330BPER",FALSE));
			// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF
			m_checkPrependPayerNSF.SetCheck(AdoFldBool(rs, "PrependPayerNSF",FALSE));
			m_checkUseAddnl2010AAID.SetCheck(AdoFldBool(rs, "Use_Addnl_2010AA",FALSE));
			OnCheckUseAdditional2010aaId();
			SetDlgItemText(IDC_EDIT_ADDNL_2010AA_QUAL, AdoFldString(rs, "Addnl_2010AA_Qual",""));
			SetDlgItemText(IDC_EDIT_ADDNL_2010AA, AdoFldString(rs, "Addnl_2010AA",""));
			// (j.jones 2006-12-06 15:45) - PLID 23631 - added the SeparateBatches option
			m_checkSeparateBatchesByInsCo.SetCheck(AdoFldBool(rs, "SeparateBatchesByInsCo",FALSE));
			OnSeparateBatchByInsuranceCo();
			// (j.jones 2007-04-05 10:26) - PLID 25506 - added the 1000A PER05/06 override
			m_check1000APER.SetCheck(AdoFldBool(rs, "Use1000APER",FALSE));
			OnCheck1000aPer();
			SetDlgItemText(IDC_EDIT_1000A_PER05_QUAL, AdoFldString(rs, "PER05Qual_1000A",""));
			SetDlgItemText(IDC_EDIT_1000A_PER06_ID, AdoFldString(rs, "PER06ID_1000A",""));
			// (j.jones 2007-05-02 09:36) - PLID 25855 - added the FourDigitRevCode option
			m_checkFourDigitRevCode.SetCheck(AdoFldBool(rs, "FourDigitRevCode",TRUE));
			// (j.jones 2008-02-15 11:51) - PLID 28943 - added SendSubscriberPerClaim
			m_checkSendSubscriberPerClaim.SetCheck(AdoFldBool(rs, "SendSubscriberPerClaim",TRUE));
			// (j.jones 2008-02-19 11:45) - PLID 29004 - added checkbox for Send2330BAddress
			m_checkExport2330BAddress.SetCheck(AdoFldBool(rs, "Send2330BAddress",FALSE));
			// (j.jones 2008-09-09 16:45) - PLID 26482 - added HidePatAmtPaid
			m_checkHide2300PatAmtPaidWhenZero.SetCheck(AdoFldBool(rs, "HidePatAmtPaid",TRUE));
			// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
			m_checkPrependPatientID.SetCheck(AdoFldBool(rs, "PrependPatientID",FALSE));
			OnCheckPrependPatientId();
			m_nxeditPrependPatientID.SetWindowText(AdoFldString(rs, "PrependPatientIDCode",""));

			if(AdoFldBool(rs, "IsGroup",FALSE)) {
				m_radioGroup.SetCheck(TRUE);
				m_radioIndiv.SetCheck(FALSE);
			}
			else {
				m_radioGroup.SetCheck(FALSE);
				m_radioIndiv.SetCheck(TRUE);
			}

			// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
			// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010 (although this field is not nullable)
			//0 - 4010, 1 - 5010
			ANSIVersion avVersion = (ANSIVersion)AdoFldLong(rs, "ANSIVersion", (long)av5010);
			BOOL b5010 = avVersion == (ANSIVersion)av5010;
			m_radio4010.SetCheck(!b5010);
			m_radio5010.SetCheck(b5010);
			m_avLastSavedANSIVersion = avVersion;
			m_avLastCheckedANSIVersion = avVersion;

			// (j.jones 2012-07-20 11:24) - PLID 47901 - We now hide the ANSI toggle, and all 4010-specific options,
			// if we loaded as 5010. The idea is that nobody should be 4010. The only way the toggle would even be
			// visible is if they loaded as 4010. Then they could switch to 5010, and the option will vanish only
			// on the next load. The toggle wouldn't vanish immediately upon switching to 5010.
			m_nxstaticANSIVersionLabel.ShowWindow(b5010 ? SW_HIDE : SW_SHOW);
			m_radio4010.ShowWindow(b5010 ? SW_HIDE : SW_SHOW);
			m_radio5010.ShowWindow(b5010 ? SW_HIDE : SW_SHOW);
			m_checkUseSSN.ShowWindow(b5010 ? SW_HIDE : SW_SHOW);
			m_checkExport2330BPERSegment.ShowWindow(b5010 ? SW_HIDE : SW_SHOW);
			m_checkExport2330BAddress.ShowWindow(b5010 ? SW_HIDE : SW_SHOW);

			//this has to be the last call because some fields disable/enable
			//based on the values of other settings
			DisableBoxes(TRUE);
		}
		else {
			DisableBoxes(FALSE);
		}
		rs->Close();

	} NxCatchAll("Error in ANSIProperties::Load()");
}

void CANSIProperties::OnAddAnsi() 
{
	//first save the existing information
	if(m_FormatID > -1 && IDNO == MessageBox("Any changes made to the current ANSI configuration will be saved.\n"
		"Do you still wish to add a new ANSI configuration?","Practice",MB_ICONQUESTION|MB_YESNO))
		return;

	if(!Save())
		return;

	//now add the new record

	try {

		CString strResult;

		int nResult = InputBoxLimited(this, "Enter a new receiver name:",strResult,"",50,false,false,NULL);

		if(nResult == IDOK && strResult != "") {

			m_FormatID = NewNumber("EbillingFormatsT","ID");

			// (j.jones 2012-02-01 11:10) - PLID 40880 - force the default to be 5010 (though it is this default in data as well)
			ExecuteParamSql("INSERT INTO EbillingFormatsT (ID, Name, Contact, ANSIVersion) "
				"VALUES ({INT}, {STRING}, {STRING}, {CONST})", m_FormatID, strResult, GetCurrentUserName(), (long)av5010);

			m_ANSIList->Requery();
			m_ANSIList->SetSelByColumn(0,m_FormatID);
			Load();
		}

	}NxCatchAll("Error in OnAddAnsi()");
}

void CANSIProperties::OnDeleteAnsi() 
{
	try {

		if(m_ANSIList->CurSel == -1)
			return;

		if(IDNO == MessageBox("Are you sure you wish to permanently delete this vendor?","Practice",MB_ICONEXCLAMATION|MB_YESNO))
			return;

		ExecuteSql("DELETE FROM EbillingFormatsT WHERE ID = %li",m_ANSIList->GetValue(m_ANSIList->CurSel,0).lVal);

		m_ANSIList->RemoveRow(m_ANSIList->CurSel);

		SetDlgItemText(IDC_RECEIVER_NAME,"");
		SetDlgItemText(IDC_ANSI_CONTACT,"");
		SetDlgItemText(IDC_EDIT_ISA07_QUAL,"");
		SetDlgItemText(IDC_RECEIVER_ID_ISA08,"");
		SetDlgItemText(IDC_RECEIVER_ID_GS03,"");
		SetDlgItemText(IDC_EDIT_1000B_QUAL,"");
		SetDlgItemText(IDC_RECEIVER_ID_1000B,"");
		SetDlgItemText(IDC_EDIT_ISA05_QUAL,"");
		SetDlgItemText(IDC_SUBMITTER_ID_ISA06,"");
		SetDlgItemText(IDC_SUBMITTER_ID_GS02,"");
		SetDlgItemText(IDC_EDIT_1000A_QUAL,"");
		SetDlgItemText(IDC_SUBMITTER_ID_1000A,"");
		SetDlgItemText(IDC_FILENAME_ANSI,"");
		SetDlgItemText(IDC_FILENAME_ELIG,"");
		SetDlgItemText(IDC_EDIT_ISA01_QUAL,"");
		SetDlgItemText(IDC_EDIT_ISA02, "");
		SetDlgItemText(IDC_EDIT_ISA03_QUAL, "");
		SetDlgItemText(IDC_EDIT_ISA04, "");
		m_capitalize.SetCheck(FALSE);
		m_UnPunctuate.SetCheck(FALSE);
		m_zipfile.SetCheck(FALSE);
		m_checkUse2420A.SetCheck(FALSE);
		m_checkHide2310D.SetCheck(FALSE);		
		// (j.jones 2012-01-06 16:15) - PLID 47351 - added Hide2310CWhenBillLocation
		m_checkHide2310CWhenBillLocation.SetCheck(FALSE);
		m_checkUseSV106.SetCheck(FALSE);
		m_DontSubmitSecondary.SetCheck(FALSE);
		m_DontSendSecondaryDiagnosisMismatch.SetCheck(FALSE);
		m_checkUse2010AB.SetCheck(FALSE);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN IDs
		//m_checkUseTHINPayerIDs.SetCheck(FALSE);
		//OnCheckUseThinPayerId();
		m_checkUseSSN.SetCheck(FALSE);
		m_checkTruncateCurrency.SetCheck(FALSE);
		m_checkExport2330BPERSegment.SetCheck(FALSE);
		m_radioIndiv.SetCheck(FALSE);
		m_radioGroup.SetCheck(FALSE);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF
		m_checkPrependPayerNSF.SetCheck(FALSE);
		m_checkUseAddnl2010AAID.SetCheck(FALSE);
		m_checkSeparateBatchesByInsCo.SetCheck(FALSE);
		// (j.jones 2007-04-05 10:26) - PLID 25506 - added the 1000A PER05/06 override
		m_check1000APER.SetCheck(FALSE);
		OnCheck1000aPer();
		SetDlgItemText(IDC_EDIT_1000A_PER05_QUAL, "");
		SetDlgItemText(IDC_EDIT_1000A_PER06_ID, "");
		// (j.jones 2007-05-02 09:36) - PLID 25855 - added the FourDigitRevCode option
		m_checkFourDigitRevCode.SetCheck(FALSE);
		// (j.jones 2008-02-15 11:51) - PLID 28943 - added SendSubscriberPerClaim
		m_checkSendSubscriberPerClaim.SetCheck(FALSE);
		// (j.jones 2008-02-19 11:45) - PLID 29004 - added checkbox for Send2330BAddress
		m_checkExport2330BAddress.SetCheck(FALSE);
		// (j.jones 2008-09-09 16:44) - PLID 26482 - added m_checkHide2300PatAmtPaidWhenZero
		m_checkHide2300PatAmtPaidWhenZero.SetCheck(FALSE);
		// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
		m_checkPrependPatientID.SetCheck(FALSE);
		OnCheckPrependPatientId();
		m_nxeditPrependPatientID.SetWindowText("");

		m_ANSIList->CurSel = 0;

		if(m_ANSIList->CurSel == -1) {
			m_FormatID = -1;
			DisableBoxes(FALSE);
		}
		else {
			m_FormatID = m_ANSIList->GetValue(m_ANSIList->CurSel,0).lVal;
			Load();
		}

	}NxCatchAll("Error in OnDeleteAnsi()");	
}

BEGIN_EVENTSINK_MAP(CANSIProperties, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CANSIProperties)
	ON_EVENT(CANSIProperties, IDC_ANSI_STYLE_LIST, 16 /* SelChosen */, OnSelChosenAnsiStyleList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CANSIProperties::OnSelChosenAnsiStyleList(long nRow) 
{
	if(nRow == -1)
		return;

	long nNewFormatID = m_ANSIList->GetValue(nRow,0).lVal;

	if(nNewFormatID == m_FormatID)
		return;

	if(m_FormatID > -1 && IDNO == MessageBox("Any changes made to the previous ANSI configuration will be saved.\n"
		"Do you still wish to switch to a different ANSI configuration?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		m_ANSIList->SetSelByColumn(0, m_FormatID);
		return;
	}

	Save();

	m_FormatID = nNewFormatID;

	Load();
}

void CANSIProperties::OnBtnShowAdvOptions() 
{
	//toggle
	m_bShowAdvOptions = !m_bShowAdvOptions;

	// (j.jones 2008-05-07 11:03) - PLID 29854 - renamed the button when toggled
	if(m_bShowAdvOptions) {
		m_btnShowAdvanced.SetWindowText("Hide Advanced Options");
	}
	else {
		m_btnShowAdvanced.SetWindowText("Show Advanced Options");
	}

	ShowAdvOptions(m_bShowAdvOptions);
}

void CANSIProperties::ShowAdvOptions(BOOL bShow) 
{
	CRect rcWindow;
	GetWindowRect(rcWindow);

	MoveWindow(rcWindow.left,rcWindow.top,rcWindow.Width() + ((bShow ? 1 : -1) * 395), rcWindow.Height(), SWP_NOZORDER);
}

void CANSIProperties::DisableBoxes(BOOL bEnable)
{
	try {

		GetDlgItem(IDC_RECEIVER_NAME)->EnableWindow(bEnable);
		GetDlgItem(IDC_ANSI_CONTACT)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ISA07_QUAL)->EnableWindow(bEnable);
		GetDlgItem(IDC_RECEIVER_ID_ISA08)->EnableWindow(bEnable);
		GetDlgItem(IDC_RECEIVER_ID_GS03)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_1000B_QUAL)->EnableWindow(bEnable);
		GetDlgItem(IDC_RECEIVER_ID_1000B)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ISA05_QUAL)->EnableWindow(bEnable);
		GetDlgItem(IDC_SUBMITTER_ID_ISA06)->EnableWindow(bEnable);
		GetDlgItem(IDC_SUBMITTER_ID_GS02)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_1000A_QUAL)->EnableWindow(bEnable);
		GetDlgItem(IDC_SUBMITTER_ID_1000A)->EnableWindow(bEnable);
		GetDlgItem(IDC_FILENAME_ANSI)->EnableWindow(bEnable);
		GetDlgItem(IDC_FILENAME_ELIG)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ISA01_QUAL)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ISA02)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ISA03_QUAL)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ISA04)->EnableWindow(bEnable);
		GetDlgItem(IDC_EXPORT_ALL_2010AA_IDS)->EnableWindow(bEnable ? !m_checkSeparateBatchesByInsCo.GetCheck() : FALSE);
		m_capitalize.EnableWindow(bEnable);
		m_UnPunctuate.EnableWindow(bEnable);
		m_zipfile.EnableWindow(bEnable);
		m_checkUse2420A.EnableWindow(bEnable);
		m_checkHide2310D.EnableWindow(bEnable);
		// (j.jones 2012-01-06 16:15) - PLID 47351 - added Hide2310CWhenBillLocation, 5010 only
		m_checkHide2310CWhenBillLocation.EnableWindow(bEnable && m_radio5010.GetCheck());

		// (j.jones 2010-10-14 10:19) - PLID 40878 - rename the service facility option,
		//it's 2310D in 4010, 2310C in 5010
		if(m_radio4010.GetCheck()) {
			//4010
			m_checkHide2310D.SetWindowText("Hide 2310D Record When Place Of Service Code is 11 or 12");
		}
		else {
			//5010
			m_checkHide2310D.SetWindowText("Hide 2310C Record When Place Of Service Code is 11 or 12");
		}

		m_checkUseSV106.EnableWindow(bEnable);
		m_DontSubmitSecondary.EnableWindow(bEnable);
		// (a.wilson 2014-06-27 10:48) - PLID 62517
		m_DontSendSecondaryDiagnosisMismatch.EnableWindow(bEnable ? (m_DontSubmitSecondary.GetCheck() ? FALSE : TRUE) : FALSE);
		m_checkUse2010AB.EnableWindow(bEnable);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs
		//m_checkUseTHINPayerIDs.EnableWindow(bEnable);
		// (j.jones 2010-10-14 10:30) - PLID 40878 - SSN is always used in 5010
		m_checkUseSSN.EnableWindow(bEnable && m_radio4010.GetCheck());
		m_checkTruncateCurrency.EnableWindow(bEnable);
		// (j.jones 2010-10-14 10:30) - PLID 40878 - 2330B PER doesn't exist in 5010
		m_checkExport2330BPERSegment.EnableWindow(bEnable && m_radio4010.GetCheck());
		// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF
		m_checkPrependPayerNSF.EnableWindow(bEnable);
		m_radioIndiv.EnableWindow(bEnable);
		m_radioGroup.EnableWindow(bEnable);
		m_checkUseAddnl2010AAID.EnableWindow(bEnable);
		m_checkSeparateBatchesByInsCo.EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_ADDNL_2010AA_QUAL)->EnableWindow(bEnable ? m_checkUseAddnl2010AAID.GetCheck() : FALSE);
		GetDlgItem(IDC_EDIT_ADDNL_2010AA)->EnableWindow(bEnable ? m_checkUseAddnl2010AAID.GetCheck() : FALSE);
		GetDlgItem(IDC_EDIT_1000A_PER05_QUAL)->EnableWindow(bEnable ? m_check1000APER.GetCheck() : FALSE);
		GetDlgItem(IDC_EDIT_1000A_PER06_ID)->EnableWindow(bEnable ? m_check1000APER.GetCheck() : FALSE);
		// (j.jones 2007-05-02 09:36) - PLID 25855 - added the FourDigitRevCode option
		m_checkFourDigitRevCode.EnableWindow(bEnable);
		// (j.jones 2008-02-15 11:51) - PLID 28943 - added SendSubscriberPerClaim
		m_checkSendSubscriberPerClaim.EnableWindow(bEnable);
		// (j.jones 2008-02-19 11:45) - PLID 29004 - added checkbox for Send2330BAddress
		// (j.jones 2010-10-14 10:30) - PLID 40878 - 2330B Address always exists in 5010
		m_checkExport2330BAddress.EnableWindow(bEnable && m_radio4010.GetCheck());
		// (j.jones 2008-09-09 16:44) - PLID 26482 - added m_checkHide2300PatAmtPaidWhenZero
		m_checkHide2300PatAmtPaidWhenZero.EnableWindow(bEnable);
		// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
		m_checkPrependPatientID.EnableWindow(bEnable);
		m_nxeditPrependPatientID.EnableWindow(bEnable ? m_checkPrependPatientID.GetCheck() : FALSE);
		// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
		m_radio4010.EnableWindow(bEnable);
		m_radio5010.EnableWindow(bEnable);

	}NxCatchAll("Error disabling boxes.");
}

// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN IDs
/*
void CANSIProperties::OnCheckUseThinPayerId() 
{
	m_checkPrependTHINPayerNSF.EnableWindow(m_checkUseTHINPayerIDs.GetCheck());
}
*/

void CANSIProperties::OnCheckUseAdditional2010aaId() 
{
	GetDlgItem(IDC_EDIT_ADDNL_2010AA_QUAL)->EnableWindow(m_checkUseAddnl2010AAID.GetCheck());
	GetDlgItem(IDC_EDIT_ADDNL_2010AA)->EnableWindow(m_checkUseAddnl2010AAID.GetCheck());	
}

void CANSIProperties::OnSeparateBatchByInsuranceCo() 
{
	// (j.jones 2006-12-06 15:46) - PLID 23631 - if separate batches is checked,
	// OutputAll2010AAIDs is not used
	GetDlgItem(IDC_EXPORT_ALL_2010AA_IDS)->EnableWindow(!m_checkSeparateBatchesByInsCo.GetCheck());	
}

// (j.jones 2007-04-05 10:09) - PLID 25506 - added configuration for the 1000A PER05/06 field
void CANSIProperties::OnCheck1000aPer() 
{
	GetDlgItem(IDC_EDIT_1000A_PER05_QUAL)->EnableWindow(m_check1000APER.GetCheck());
	GetDlgItem(IDC_EDIT_1000A_PER06_ID)->EnableWindow(m_check1000APER.GetCheck());
}

// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
void CANSIProperties::OnCheckPrependPatientId()
{
	try {

		GetDlgItem(IDC_EDIT_PREPEND_PATIENT_ID)->EnableWindow(m_checkPrependPatientID.GetCheck());

	}NxCatchAll("Error in CANSIProperties::OnCheckPrependPatientId");
}

// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
void CANSIProperties::OnRadio4010()
{
	try {

		//warn about selecting 4010 (uses the "last checked" so we don't warn too often for click-happy people)
		if(m_radio4010.GetCheck() && m_avLastCheckedANSIVersion != av4010) {
			if(DontShowMeAgain(this, "You have chosen to change your ANSI version to 4010. This will affect all electronic claims and eligibility requests.\n\n"
				"You must confirm with your clearinghouse that you are switching versions to 4010.\n\n"
				"Are you absolutely SURE you wish to switch to the 4010 version?", "ANSIVersion4010", "Switching ANSI Export Version", FALSE, TRUE) == IDNO) {
				m_radio4010.SetCheck(FALSE);
				m_radio5010.SetCheck(TRUE);
			}
			else {
				m_avLastCheckedANSIVersion = av4010;
			}
		}

		//some controls might enable/disable based on the ANSI version, so call that function now
		DisableBoxes(TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CANSIProperties::OnRadio5010()
{
	try {

		//warn about selecting 5010 (uses the "last checked" so we don't warn too often for click-happy people)
		if(m_radio5010.GetCheck() && m_avLastCheckedANSIVersion != av5010) {
			if(DontShowMeAgain(this, "You have chosen to change your ANSI version to 5010. This will affect all electronic claims and eligibility requests.\n\n"
				"You must confirm with your clearinghouse that you are switching versions to 5010.\n\n"
				"Are you absolutely SURE you wish to switch to the 5010 version?", "ANSIVersion5010", "Switching ANSI Export Version", FALSE, TRUE) == IDNO) {
				m_radio4010.SetCheck(TRUE);
				m_radio5010.SetCheck(FALSE);
			}
			else {
				m_avLastCheckedANSIVersion = av5010;
			}
		}

		//some controls might enable/disable based on the ANSI version, so call that function now
		DisableBoxes(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-06-27 14:22) - PLID 62517 - disable/enable diagnosis mismatch option based on this checkboxes status.
void CANSIProperties::OnBnClickedDontSubmitSecondaries()
{
	try {
		if (m_DontSubmitSecondary.GetCheck())
			m_DontSendSecondaryDiagnosisMismatch.EnableWindow(FALSE);
		else
			m_DontSendSecondaryDiagnosisMismatch.EnableWindow(TRUE);
	} NxCatchAll(__FUNCTION__);
}
