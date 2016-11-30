// CreditCardProcessingDlg.cpp : implementation file
//

#include "Practice.h"
#include "stdafx.h"
#include "billingrc.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "VerifyCardHolderInfoDlg.h"
#include "globalUtils.h"
#include "AuditTrail.h"
#include "OPOSMSRDevice.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "QBMSProcessingUtils.h"
#include "TimedMessageBoxDlg.h"
#include "CreditCardProcessingDlg.h"
#include "ChaseProcessingUtils.h"
#include "KeyboardCardSwipeDlg.h"
#include "PaymentDlg.h"

// (d.lange 2010-09-01 11:41) - PLID 40310 - A complete copy of QBMS_ProcessCreditCardDlg, but with a more generic name
// CCreditCardProcessingDlg dialog

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum TipListColumns {
	tlcID = 0,
	tlcProv,
	tlcAmount,
	tlcPayMethod,
};

enum CardListColumns {
	clcID = 0,
	clcName,
};

// (d.thompson 2010-11-10) - PLID 40351 - Modified to include some fields only used by Chase, not QBMS
enum eAccountColumns {
	eacID = 0,			//QBMS + Chase
	eacDescription,		//QBMS + Chase
	eacIsProduction,	//Chase Only
	eacUsername,		//Chase Only
	eacPassword,		//Chase Only
	eacMerchantID,		//Chase Only
	eacTerminalID,		//Chase Only
};

// (e.lally 2007-12-11) PLID 28325 - Updated "Privatize" cc number function to "Mask"

CCreditCardProcessingDlg::CCreditCardProcessingDlg(long nPaymentID, CString strCCNumber, CString strCCName, 
	  CString strCCExpDate, long nCCCardID, COleCurrency cyPayAmount, long nType, long nPatientID,
	  long nProviderID, long nDrawerID, long nLocationID, bool bCCSwiped, CString strTrack2Data,
	  CWnd* pParent, CPaymentSaveInfo* pInfo, bool bIsNew, 
	  CString strApplyToPaymentTransID)
	: CNxDialog(CCreditCardProcessingDlg::IDD, pParent)
{
	m_nPaymentID = nPaymentID;
	m_strCCNumber = strCCNumber;
	m_strCCName = strCCName;
	m_strCCExpDate = strCCExpDate;
	m_nCCCardID = nCCCardID;
	m_cyPayAmount = cyPayAmount;
	m_nType = nType;
	m_nPatientID = nPatientID;
	m_nProviderID = nProviderID;
	m_nDrawerID = nDrawerID;
	m_nLocationID = nLocationID;
	m_bIsSwiped = bCCSwiped;
	m_strTrack2Data = strTrack2Data;
	m_pInfo = pInfo; // (a.walling 2007-09-21 17:09) - PLID 27468
	m_bIsNew = bIsNew;
	m_strApplyToPaymentTransID = strApplyToPaymentTransID;
	m_strPatientName = GetExistingPatientName(m_nPatientID);
}

void CCreditCardProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreditCardProcessingDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_TOTAL_TRANS_AMT, m_nxeditTotalTransAmt);
	DDX_Control(pDX, IDC_CC_AUTH_NUMBER, m_nxeditCcAuthNumber);
	DDX_Control(pDX, IDC_CC_AUTH_NAME, m_nxeditCcAuthName);
	DDX_Control(pDX, IDC_CC_AUTH_EXP_DATE, m_nxeditCcAuthExpDate);
	DDX_Control(pDX, IDC_TOTAL_TIP_AMT, m_nxeditTotalTipAmt);
	DDX_Control(pDX, IDC_CC_SECURITY_CODE, m_nxeditCcSecurityCode);
	DDX_Control(pDX, IDC_MESSAGE_LABEL, m_nxstaticMessageLabel);
	DDX_Control(pDX, IDC_PROCESS_CC_CARD, m_btnProcess);
	DDX_Control(pDX, IDC_SAVE_AND_PROCESS_LATER, m_btnSaveAndProcessLater);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_TIP_BTN, m_btnAddTip);
	DDX_Control(pDX, IDC_CC_BILLING_ADDRESS, m_nxeditBillingAddress);
	DDX_Control(pDX, IDC_CC_BILLING_ZIP, m_nxeditBillingZip);
	DDX_Control(pDX, IDC_CARD_IS_PRESENT, m_btnIsCardPresent);
	DDX_Control(pDX, IDC_CARD_SWIPED, m_btnCardIsSwiped);
	DDX_Control(pDX, IDC_KEYBOARD_CARD_SWIPE, m_btnKeyboardCardSwipe); 
	//}}AFX_DATA_MAP
}


// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CCreditCardProcessingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCreditCardProcessingDlg)
	ON_BN_CLICKED(IDC_PROCESS_CC_CARD, OnProcessCcCard)
	ON_BN_CLICKED(IDC_ADD_TIP_BTN, OnAddTipBtn)
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_EN_UPDATE(IDC_CC_AUTH_EXP_DATE, OnUpdateCcAuthExpDate)
	ON_EN_KILLFOCUS(IDC_CC_AUTH_NUMBER, OnKillfocusCCNumber)
	ON_BN_CLICKED(IDC_SAVE_AND_PROCESS_LATER, OnBnClickedSaveAndProcessLater)
	ON_BN_CLICKED(IDC_KEYBOARD_CARD_SWIPE, &CCreditCardProcessingDlg::OnBnClickedKeyboardCardSwipe)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CCreditCardProcessingDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCreditCardProcessingDlg)
	ON_EVENT(CCreditCardProcessingDlg, IDC_CARD_TYPE_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCardTypeList, VTS_I4 VTS_I4)
	ON_EVENT(CCreditCardProcessingDlg, IDC_CARD_TYPE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCardTypeList, VTS_I2)
	ON_EVENT(CCreditCardProcessingDlg, IDC_CC_AUTH_TIP_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCcAuthTipList, VTS_I2)
	ON_EVENT(CCreditCardProcessingDlg, IDC_CC_AUTH_TIP_LIST, 6 /* RButtonDown */, OnRButtonDownCcAuthTipList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCreditCardProcessingDlg, IDC_CC_AUTH_TIP_LIST, 10 /* EditingFinished */, OnEditingFinishedCcAuthTipList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CCreditCardProcessingDlg, IDC_ACCOUNTS_LIST, 16, CCreditCardProcessingDlg::SelChosenAccountsList, VTS_DISPATCH)
END_EVENTSINK_MAP()

BOOL CCreditCardProcessingDlg::OnInitDialog() 
{
	try { 
		CNxDialog::OnInitDialog();

		// (d.thompson 2011-09-27) - PLID 44148 - Batch any ConfigRT settings here
		g_propManager.CachePropertiesInBulk("CCreditCardProcessingDlg", propNumber, 
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ChaseProductionURL' "
			"OR Name = 'ChasePreProductionURL' "
			"OR Name = 'AutoImportCCAddress' " // (j.luckoski 2013-03-25 14:27) - PLID 42771
			")", _Q(GetCurrentUserName()));

		// (b.spivey, December 13, 2011) - PLID 40567 - Cache workstation properties.
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
		//manager rather than the license object.
		g_propManager.CachePropertiesInBulk("CCreditCardProcessingDlg-IntParam-Workstation", propNumber, 
			"(Username = '%s') AND ("
			"Name = 'MSR_UseDevice' "
			"OR Name = 'CCreditCardProcessingDlg_RecallLastAccount' "
			"OR Name = 'CCreditCardProcessingDlg_RememberedAccountID' "
			")", _Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		// (c.haag 2008-04-23 16:05) - PLID 29761 - NxIconified buttons
		m_btnProcess.AutoSet(NXB_OK);
		m_btnSaveAndProcessLater.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddTip.AutoSet(NXB_NEW);

		// (d.thompson 2010-11-12) - PLID 40351 - "card present" doesn't apply to Chase.  It's either swiped or not swiped.
		if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
			GetDlgItem(IDC_CARD_IS_PRESENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CARD_IS_PRESENT)->ShowWindow(SW_HIDE);
		}

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pTipList = BindNxDataList2Ctrl(IDC_CC_AUTH_TIP_LIST, false);
		m_pCardList = BindNxDataList2Ctrl(IDC_CARD_TYPE_LIST, true);
		// (d.thompson 2010-11-08) - PLID 41367 - Added account list
		m_pAccountList = BindNxDataList2Ctrl(IDC_ACCOUNTS_LIST, false);
		//Load the list from memory
		LoadAccountsList();
		//Attempt to make a default selection for new accounts
		if(m_bIsNew) {
			SelectDefaultAccount();
		}

		CString strWhere;
		strWhere.Format("PaymentTipsT.PaymentID = %li", m_nPaymentID);
		m_pTipList->WhereClause = _bstr_t(strWhere);
		//TES 3/30/2015 - PLID 65177 - Need to reverse the tip amount, so that it's the same sign as the refund amount (which is positive)
		//TES 4/15/2015 - PLID 65177 - We also need to update the pay method values to the refund versions
		if (m_nType == 3) {
			m_pTipList->GetColumn(tlcAmount)->FieldName = _bstr_t("-1 * Amount");
			m_pTipList->GetColumn(tlcPayMethod)->ComboSource = _bstr_t("SELECT 7, 'Cash' UNION SELECT 8, 'Check' UNION SELECT 9, 'Charge'");
		}
		m_pTipList->Requery();

		//Text limits
		m_nxeditBillingAddress.SetLimitText(30);
		m_nxeditBillingZip.SetLimitText(9);
		m_nxeditCcSecurityCode.SetLimitText(4);
		m_nxeditCcAuthName.SetLimitText(30);

		//load the information
		//(e.lally 2007-10-30) PLID 27892 - Only display last 4 digits. Buffer with X's
		{
			CString strDisplayedCCNumber = MaskCCNumber(m_strCCNumber);
			SetDlgItemText(IDC_CC_AUTH_NUMBER, strDisplayedCCNumber);
		}
		SetDlgItemText(IDC_CC_AUTH_NAME, m_strCCName);
		SetDlgItemText(IDC_CC_AUTH_EXP_DATE, m_strCCExpDate);

		//Loading based on new/edit status.
		BOOL bEnableSaveLater = FALSE;
		// (j.luckoski 2013-03-25 14:21) - PLID 42771 - If office wants to skip address import to prevent errors, allow them to
		if (!GetRemotePropertyInt("AutoImportCCAddress", 0, 0, "<None>", true)) {
			if(m_bIsNew) {
				// (d.thompson 2009-04-13) - PLID 33957 - Load the patient billing address
				_RecordsetPtr prsBillingInfo = CreateParamRecordset("SELECT Address1, Address2, Zip FROM PersonT WHERE ID = {INT};", m_nPatientID);
				if(!prsBillingInfo->eof) {
					//Combine to a single address field
					CString strAddress = AdoFldString(prsBillingInfo, "Address1", "");
					CString strAddr2 = AdoFldString(prsBillingInfo, "Address2", "");
					if(!strAddr2.IsEmpty()) {
						strAddress += " " + strAddr2;
					}

					SetDlgItemText(IDC_CC_BILLING_ADDRESS, strAddress);
					SetDlgItemText(IDC_CC_BILLING_ZIP, AdoFldString(prsBillingInfo, "Zip", ""));
				}
			}
			else {
				// (d.thompson 2010-09-02) - PLID 40371 - Handle Chase separately from Intuit
				if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
					//If we're editing, there are a few changes:
					//	1)  Patient billing and security code need loaded from saved data, not from the patient
					// (d.thompson 2010-11-08) - PLID 41367 - Account too
					_RecordsetPtr prsLoadInfo = CreateParamRecordset("SELECT BillingAddress, BillingZip, AccountID "
						"FROM QBMS_CreditTransactionsT WHERE ID = {INT};", m_nPaymentID);
					if(!prsLoadInfo->eof) {
						SetDlgItemText(IDC_CC_BILLING_ADDRESS, AdoFldString(prsLoadInfo, "BillingAddress", ""));
						SetDlgItemText(IDC_CC_BILLING_ZIP, AdoFldString(prsLoadInfo, "BillingZip", ""));
						// (d.thompson 2010-11-08) - You cannot ever edit an approved transaction to get here.  So if we try to reselect
						//	an account, but it fails (most likely because it was inactivated), then we'll just leave the selection at
						//	NULL.  If the account is inactive, it's unlikely they want to process to it anymore, so we shouldn't make
						//	it easy to do so.  They will be forced to pick a new account.
						m_pAccountList->SetSelByColumn(eacID, (long)AdoFldLong(prsLoadInfo, "AccountID"));
						SelChosenAccountsList(m_pAccountList->GetCurSel());
					}
					//	2)  We want to allow save & process later to be enabled
					bEnableSaveLater = TRUE;
				}
				else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
					// (d.thompson 2010-11-11) - PLID 40351 - This basically follows the same logic as QBMS
					//If we're editing, there are a few changes:
					//	1)  Patient billing info and account info needs loaded from saved data, not from the patient
					_RecordsetPtr prsLoadInfo = CreateParamRecordset("SELECT BillingAddress, BillingZip, AccountID "
						"FROM Chase_CreditTransactionsT WHERE ID = {INT};", m_nPaymentID);
					if(!prsLoadInfo->eof) {
						SetDlgItemText(IDC_CC_BILLING_ADDRESS, AdoFldString(prsLoadInfo, "BillingAddress", ""));
						SetDlgItemText(IDC_CC_BILLING_ZIP, AdoFldString(prsLoadInfo, "BillingZip", ""));
						// (d.thompson 2010-11-08) - You cannot ever edit an approved transaction to get here.  So if we try to reselect
						//	an account, but it fails (most likely because it was inactivated), then we'll just leave the selection at
						//	NULL.  If the account is inactive, it's unlikely they want to process to it anymore, so we shouldn't make
						//	it easy to do so.  They will be forced to pick a new account.
						m_pAccountList->SetSelByColumn(eacID, (long)AdoFldLong(prsLoadInfo, "AccountID"));
						SelChosenAccountsList(m_pAccountList->GetCurSel());
					}
					//	2)  We want to allow save & process later to be enabled
					bEnableSaveLater = TRUE;
				}
			}
		}

		//Tips are disabled under a few scenarios:
		//	- We're editing an already-saved transaction.
		//	- We're on a refund
		//Note: Pending future decisions, tips are being left here, and the "add tip" button handler
		//	alerts the user about the licensing if they don't have NexSpa.
		//TES 3/30/2015 - PLID 65177 - Refunds can now have tips
		if(!m_bIsNew /*|| m_nType == 3*/) {
			//	1)  Tips cannot be modified.  Disable the interface (but still display them)
			GetDlgItem(IDC_ADD_TIP_BTN)->EnableWindow(FALSE);
			m_pTipList->ReadOnly = VARIANT_TRUE;
		}

		// (d.thompson 2009-06-30) - PLID 34751 - Added preference to default this value
		CheckDlgButton(IDC_CARD_IS_PRESENT, GetRemotePropertyInt("DefaultCardIsPresent", 1, 0, "<None>"));

		CFont fnt;
		fnt.CreateFont(
			27,                        // nHeight
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_BOLD,                 // nWeight
			FALSE,                     // bItalic
			FALSE,                     // bUnderline
			0,                         // cStrikeOut
			ANSI_CHARSET,              // nCharSet
			OUT_DEFAULT_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			DEFAULT_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			_T("Arial"));                 // lpszFacename
		GetDlgItem(IDC_MESSAGE_LABEL)->SetFont(&fnt);

		if(m_bIsSwiped) {
			DisableAfterSwipe();

			//Check the button so the user knows why things are disabled
			CheckDlgButton(IDC_CARD_SWIPED, TRUE);
		}

		//The save & process later button is hidden by default until they actually get a rejection, or if we're
		//	editing from the start (previously rejected)
		GetDlgItem(IDC_SAVE_AND_PROCESS_LATER)->ShowWindow(bEnableSaveLater ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SAVE_AND_PROCESS_LATER)->EnableWindow(bEnableSaveLater);
		GetDlgItem(IDC_PROCESS_CC_CARD)->ShowWindow(SW_SHOW);

		// (b.spivey, October 03, 2011) - PLID 40567 - Check for keyboard mode. If not keyboard mode, we don't show this.
		// (b.spivey, November 18, 2011) - PLID 40567 - corrected bool type. 
		if(GetPropertyInt("MSR_UseDevice", 0, 0, true) == emmKeyboard){
			GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_SHOW); 
		}
		else{
			GetDlgItem(IDC_KEYBOARD_CARD_SWIPE)->ShowWindow(SW_HIDE); 
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (d.thompson 2010-11-08) - PLID 41367 - Load accounts from the cache (cache will load if empty on first access) so
//	that users can have multiple accounts available.
void CCreditCardProcessingDlg::LoadAccountsList()
{
	if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
		CArray<CQBMSProcessingAccount, CQBMSProcessingAccount&> ary;
		QBMSProcessingUtils::GetAccountIDsAndNames(&ary);

		//Now load these into the list
		for(int i = 0; i < ary.GetSize(); i++) {
			CQBMSProcessingAccount acct = ary.GetAt(i);

			//Only display active accounts
			if(!acct.GetIsInactive()) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetNewRow();
				pRow->PutValue(eacID, (long)acct.GetID());
				pRow->PutValue(eacDescription, _bstr_t(acct.GetDescription()));
				m_pAccountList->AddRowSorted(pRow, NULL);
			}
		}
	}
	else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
		// (d.thompson 2010-11-10) - PLID 40351 - Just load from data for Chase accounts
		m_pAccountList->FromClause = _bstr_t("Chase_SetupDataT");
		m_pAccountList->WhereClause = _bstr_t("Inactive = 0");
		m_pAccountList->Requery();
		m_pAccountList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}
	else {
		//We should not be able to get into this dialog without a credit card license, so this is "blow up" bad.  We will not
		//	run this check anywhere else in this dialog since we're doing it in initdialog.
		AfxThrowNxException("Invalid licensing options for credit card processing.");
	}
}

// (d.thompson 2010-11-08) - PLID 41367 - Choose the default account to be selected when the dialog loads.
void CCreditCardProcessingDlg::SelectDefaultAccount()
{
	//If there is 1 and only 1 account in the list, go ahead and choose it.  Most offices will use only 1 account.
	if(m_pAccountList->GetRowCount() == 1) {
		m_pAccountList->PutCurSel( m_pAccountList->GetFirstRow() );
	}
	else {
		// (d.thompson 2013-06-06) - PLID 56334 - If the preference is on, recall the last account that was used to 
		//	successfully process a card payment.
		if(GetPropertyInt("CCreditCardProcessingDlg_RecallLastAccount", 1)) {
			m_pAccountList->SetSelByColumn(eacID, (long)GetPropertyInt("CCreditCardProcessingDlg_RememberedAccountID"));
		}
	}

	//We will have a future item (PLID 41380) which allows some preset preferences to determine how the default selection is made, if there
	//	are more than 1 rows.

	//Ensure any processing is handled
	SelChosenAccountsList(m_pAccountList->GetCurSel());
}

BOOL CCreditCardProcessingDlg::SaveChanges()
{
	long nAuditTransactionID = -1;

	try {
		//see if they changed anything
		CString strName;
		GetDlgItemText(IDC_CC_AUTH_NAME, strName);

		//(e.lally 2007-10-30) PLID 27892 - This is the privatized number, buffered with X's
		CString strNumber;
		GetDlgItemText(IDC_CC_AUTH_NUMBER, strNumber);

		CString strExpDate;
		GetDlgItemText(IDC_CC_AUTH_EXP_DATE, strExpDate);

		long nCardType;
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCardList->CurSel;
		if (pRow) {
			nCardType = VarLong(pRow->GetValue(clcID));
		}
		else {
			MsgBox("Please choose a card type");
			return FALSE;
		}

		//format the date correctly
		COleDateTime dtExpDate;
		//set the date to be the last date of the exp. month
		CString strMonth = strExpDate.Left(strExpDate.Find("/",0));
		CString strYear = "20" + strExpDate.Right(strExpDate.Find("/",0));
		if(strMonth=="12") {
			//the method we use to store dates acts funky with December, so
			//we cannot just increase the month by 1. However, we know the last
			//day in December is always 31, so it's an easy fix.
			dtExpDate.SetDate(atoi(strYear),atoi(strMonth),31);
		}
		else {
			//this method works well for all other months. Set the date to be
			//the first day of the NEXT month, then subtract one day.
			//The result will always be the last day of the month entered.
			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dtExpDate.SetDate(atoi(strYear),atoi(strMonth)+1,1);
			dtExpDate = dtExpDate - dtSpan;
		}
		strExpDate = FormatDateTimeForSql(dtExpDate, dtoDate);

		// (j.jones 2007-03-13 14:45) - PLID 25183 - ensured we won't save an invalid date
		// (d.thompson 2009-04-13) - PLID 33957 - Changed how we save using a _variant_t
		_variant_t varExpDate;
		varExpDate.vt = VT_NULL;
		COleDateTime dtMin;
		dtMin.ParseDateTime("12/31/1899");
		if(dtExpDate.m_status == COleDateTime::invalid || dtExpDate < dtMin) {
			//leave variant as null
		}
		else {
			varExpDate = COleVariant(dtExpDate);
		}

		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray args;

		//(e.lally 2007-10-30) PLID 27892 - Use our member variable to save the card number.
		// (a.walling 2007-10-30 17:55) - PLID 27891 - Save the truncated CCNumber and the encrypted SecurePAN
		// (a.walling 2010-03-15 12:39) - PLID 37751 - Use NxCrypto		
		_variant_t varKeyIndex, varEncryptedCCNumber;
		NxCryptosaur.EncryptStringToVariant(m_strCCNumber, varKeyIndex, varEncryptedCCNumber);
		// (a.walling 2010-03-15 12:40) - PLID 37751 - Param statements support variants now
		AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PaymentPlansT SET CreditCardID = {INT}, CCNumber = {STRING}, "
			" CCHoldersName = {STRING}, CCExpDate = {VT_DATE}, SecurePAN = {VARBINARY}, KeyIndex = {VT_I4} WHERE PaymentPlansT.ID = {INT}",
			nCardType, m_strCCNumber.Right(4), strName, varExpDate, varEncryptedCCNumber, varKeyIndex, m_nPaymentID);

		m_bHasChanged = FALSE;
		//see if they changed since last time we saved
		if (m_nCCCardID != nCardType) {
			m_bHasChanged = TRUE;
		}

		if (m_strCCName.CompareNoCase(strName) != 0) {
			m_bHasChanged = TRUE;
		}

		//(e.lally 2007-10-30) PLID 27892 - we need to make the comparison when the control loses focus.
		//We can force that here for safety.
		HandleOnKillfocusCCNumber();

		if (m_strCCExpDate.CompareNoCase(strExpDate) != 0) {
			m_bHasChanged = TRUE;
		}

		//save the IDs 
		m_nCCCardID = nCardType;
		m_strCCName = strName;
		//(e.lally 2007-10-30) PLID 27892 - This is already up to date.
		//m_strCCNumber = strNumber;
		m_strCCExpDate = strExpDate;

		//this isn't audited


		//Save the tips!
		// (d.thompson 2009-04-15) - PLID 33957 - You are not allowed to edit tips if this is not a new processing attempt.
		//	This prevents malicious users from saving a transaction for later, then modifying the tip before submitting it.
		//TES 4/7/2015 - PLID 65177 - Refunds can have tips now
		if((m_nType == 1 || m_nType == 3) && m_bIsNew) {
			//loop through all tips

			//drawer setup
			_variant_t varDrawerID;
			if (m_nDrawerID == -1 ) {
				varDrawerID.vt = VT_NULL;
			}
			else {
				varDrawerID = (long)m_nDrawerID;
			}

			//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
			AddStatementToSqlBatch(strSqlBatch, "DECLARE @PaymentTipID INT \r\n");
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pTipList->GetFirstRow();
			nAuditTransactionID = -1;
			if (m_pInfo) {
				// (a.walling 2007-09-21 16:02) - PLID 27468 - Clear out the total tip count from before, since it will be recalculated here.
				m_pInfo->nTotalTips = 0;
			}
			while (pRow) {
				long nID = VarLong(pRow->GetValue(tlcID));
				long nProvID = VarLong(pRow->GetValue(tlcProv));
				COleCurrency cyAmt = VarCurrency(pRow->GetValue(tlcAmount));
				//TES 4/7/2015 - PLID 65177 - If this is a refund, we need to reverse the sign (we display the opposite of what's in data on this screen)
				if (m_nType == 3) {
					cyAmt = cyAmt * -1;
				}
				long nPayMethod = VarLong(pRow->GetValue(tlcPayMethod));

				// (a.walling 2007-09-21 16:03) - PLID 27468 - Increment the total tip count
				if (m_pInfo) {
					m_pInfo->nTotalTips++;
				}

				if(nID == -1) {
					// (a.walling 2007-09-21 16:01) - PLID 27468 - Add to count of new tips with this pay method
					if (m_pInfo) {
						m_pInfo->arNewTips[nPayMethod-1]++;
					}
					CString str;
					//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
					AddParamStatementToSqlBatch(strSqlBatch, args, "SET @PaymentTipID = (SELECT (COALESCE(MAX(ID),0) + 1) FROM PaymentTipsT) \r\n"
						"INSERT INTO PaymentTipsT (ID, PaymentID, ProvID, Amount, PayMethod, DrawerID) "
						"values (@PaymentTipID, {INT}, {INT}, convert(money, {STRING}), {INT}, {VT_I4});\r\n", 
						m_nPaymentID, nProvID, FormatCurrencyForSql(cyAmt), nPayMethod, varDrawerID);

					//for auditing
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					str.Format("Tip added for %s.", FormatCurrencyForInterface(cyAmt));
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					// (a.walling 2008-07-07 17:16) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
					AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipCreated, m_nPaymentID, "", str, aepMedium, aetCreated);
				}
				else {
					_RecordsetPtr prs = CreateParamRecordset("SELECT ProvID, Amount, PayMethod, DrawerID FROM PaymentTipsT WHERE ID = {INT}", nID);
					if(!prs->eof) {
						long nOldProv = AdoFldLong(prs, "ProvID");
						long nOldMethod = AdoFldLong(prs, "PayMethod");
						COleCurrency cyOldAmt = AdoFldCurrency(prs, "Amount");
						long nOldDrawerID = AdoFldLong(prs, "DrawerID", -1);
						CString strOldDrawer = "NULL";
						if(nOldDrawerID != -1)
							strOldDrawer.Format("%li", nOldDrawerID);

						bool bUser = false, bMethod = false, bAmt = false;	//have we changed?
						CString str = "UPDATE PaymentTipsT SET ";
						CString temp;
						//we only need to update what has changed
						if(nOldProv != nProvID) {
							//need to update the provider
							temp.Format("ProvID = %li,", nProvID);
							str += temp;
							bUser = true;

							//lookup the old & new providers
							//(e.lally 2007-03-21) PLID 25258 - Move these into one query
							_RecordsetPtr prsAudit = CreateParamRecordset("SELECT (SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = {INT}) AS OldProvider, "
								"(SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}) AS NewProvider", nOldProv, nProvID);
							CString strProv1, strProv2;
							if(!prsAudit->eof){
								strProv1 = AdoFldString(prsAudit, "OldProvider");
								strProv2 = AdoFldString(prsAudit, "NewProvider");
							}
							prsAudit->Close();
							//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							// (a.walling 2008-07-07 17:16) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
							AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipProvider, m_nPaymentID, strProv1, strProv2, aepMedium, aetChanged);
						}

						if(nOldMethod != nPayMethod) {
							// (a.walling 2007-09-21 16:05) - PLID 27468 - Add to count of changed tips with this pay method
							if (m_pInfo) {
								// This is an existing tip whose pay method has changed. This means that the existing SavePaymentInfo should
								// already have an entry for this tip, but we do not know whether it was save or changed! So do we need to 
								// change the new tips array or the changed tips array? I had to add an arExistingTipIDs array to the structure.
								if (m_pInfo->IsIDInExistingTipIDs(nID)) {
									// alright, this tip was an existing tip in the payment dialog. If it was modified then, it will be in the
									// arChangedTips array
									if (m_pInfo->IsIDInModifiedTipIDs(nID)) {
										m_pInfo->arChangedTips[nOldMethod-1]--;
									} else {
										// the tip was never modified, and it was existing, so it is not in any array, and we don't need to update anything.
									}
								} else {
									// this tip was a new tip in the payment dialog, so it is guaranteed to be in our arNewTips array.
									// so decrement that count
									m_pInfo->arNewTips[nOldMethod-1]--;
								}

								// now increment the changed tips array
								m_pInfo->arChangedTips[nPayMethod-1]++;
							}

							//need to update the method
							temp.Format(" PayMethod = %li,", nPayMethod);
							str += temp;
							bMethod = true;
							//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
							//TES 4/15/2015 - PLID 65177 - Allow for the refund paymethods
							AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipMethod, m_nPaymentID, (nOldMethod == 1 || nOldMethod == 7) ? "Cash" : (nOldMethod == 2 || nOldMethod == 8)? "Check" : "Charge", (nPayMethod == 1 || nPayMethod == 7) ? "Cash" : (nPayMethod == 2 || nPayMethod == 8)? "Check" : "Charge", aepMedium, aetChanged);
						}

						if(cyOldAmt != cyAmt) {
							// (a.walling 2007-09-21 16:05) - PLID 27468 - Add to count of changed tips with this pay method
							if (m_pInfo) {
								if (m_pInfo->IsIDInExistingTipIDs(nID)) {
									// This was an existing tip in the payment dialog.
									if (m_pInfo->IsIDInModifiedTipIDs(nID)) {
										// and it was modified, so we don't need to update anything, since it should
										// already be reflected in arChangedTips.
									} else {
										// this was an existing tip which was not modified, which now has been modified.
										// so all we need to do is update arChangedTips
										m_pInfo->arChangedTips[nPayMethod-1]++;
									}
								} else {
									// this was a new tip in the payment dialog. It should already be in arNewTips.
									// so we don't need to update anything.
								}
							}

							//need to update the amt
							temp.Format(" Amount = convert(money, '%s'),", FormatCurrencyForSql(cyAmt));
							str += temp;
							bAmt = true;

							//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
							if(nAuditTransactionID == -1)
								nAuditTransactionID = BeginAuditTransaction();
							// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
							AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipAmount, m_nPaymentID, FormatCurrencyForInterface(cyOldAmt), FormatCurrencyForInterface(cyAmt), aepMedium, aetChanged);
						}

						if(bAmt || bMethod || bUser) {
							//something changed, we must update & audit
							str.TrimRight(",");
							temp.Format("WHERE ID = %li;\r\n", nID);
							str += temp;
							//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
							AddStatementToSqlBatch(strSqlBatch, str);
						}

						// (a.walling 2007-09-21 17:17) - PLID 27468 - We are done with this tip, add its ID to existing and to modified if necessary
						if (m_pInfo) {
							if (bAmt) {
								m_pInfo->AddModifiedTipID(nID);
							}
							m_pInfo->AddExistingTipID(nID);
						}
					}
				}

				pRow = pRow->GetNextRow();
			}

			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();

			//and now we have to take care of any tips which might have been deleted while they were editing this payment
			for(long i = 0; i < m_aryDeleted.GetSize(); i++) {
				long nTipID = m_aryDeleted.GetAt(i);
				// (a.walling 2007-09-28 10:59) - PLID 27468 - Update the Deleted tips in the Info
				if (m_pInfo) {
					m_pInfo->arDeletedTipIDs.Add(nTipID);
				}
				// (a.walling 2007-09-28 10:58) - PLID 27561 - Don't bother doing this if the ID is -1
				if (nTipID != -1) {
					//(e.lally 2007-03-21) PLID 25258 - execute sql statements from this section in one batch
					AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM PaymentTipsT WHERE ID = {INT};\r\n", nTipID);

					//for auditing
					_RecordsetPtr prs = CreateParamRecordset("SELECT Amount FROM PaymentTipsT WHERE ID = {INT}", nTipID);
					if(!prs->eof) {
						CString str;
						str.Format("Tip deleted for %s.", FormatCurrencyForInterface(AdoFldCurrency(prs, "Amount")));
						//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
						// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
						AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipDeleted, nTipID, "", str, aepMedium, aetDeleted);
					}
				}
			}
		}

		//now save the changes
		if(!strSqlBatch.IsEmpty()){
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

			// (d.thompson 2009-04-23) - PLID 33957 - We have now updated any new tips to be saved.  This function
			//	may be called again, so we must refresh the tip list.
			m_pTipList->Requery();
		}

		//save the audit trans
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		return TRUE;
	} NxCatchAllCall("Error in CCreditCardProcessingDlg::SaveChanges", 
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		return FALSE;
	);
}

BOOL CCreditCardProcessingDlg::CheckFields()
{
	try { 
		//check the tips
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTipList->GetFirstRow();
		while (pRow) {
			if (VarCurrency(pRow->GetValue(tlcAmount)) < COleCurrency(0,0)) {
				MsgBox("You may not enter a negative tip.");
				return FALSE;
			}
			pRow = pRow->GetNextRow();
		}

		//check the all the required information is entered
		CString strCCName, strCCNumber, strCCExpDate;
		//(e.lally 2007-10-30) PLID 27892 - We are only checking for emptiness, so we can
		//let it use the displayed card number.
		GetDlgItemText(IDC_CC_AUTH_NUMBER, strCCNumber);
		GetDlgItemText(IDC_CC_AUTH_NAME, strCCName);
		GetDlgItemText(IDC_CC_AUTH_EXP_DATE, strCCExpDate);

		strCCNumber.TrimLeft();
		strCCNumber.TrimRight();

		strCCName.TrimLeft();
		strCCName.TrimRight();

		strCCExpDate.TrimLeft();
		strCCExpDate.TrimRight();

		if (strCCNumber.IsEmpty()) {
			MsgBox("Please enter a card number.");
			return FALSE;
		}

		if (strCCName.IsEmpty()) {
			MsgBox("Please enter the card holder's name.");
			return FALSE;
		}

		if (strCCExpDate.IsEmpty()) {
			MsgBox("Please enter the card expiration date.");
			return FALSE;
		}

		//format the date correctly
		COleDateTime dt;
		long nResult  = strCCExpDate.Find("/");
		CString strTempYear = strCCExpDate.Right(strCCExpDate.GetLength() - (nResult + 1));
		if (strTempYear.GetLength() > 2) {
			MsgBox("Please enter a 2 digit year.");
			return FALSE;
		}
		//set the date to be the last date of the exp. month
		CString strMonth = strCCExpDate.Left(strCCExpDate.Find("/",0));
		CString strYear = "20" + strCCExpDate.Right(strCCExpDate.Find("/",0));
		if (atoi(strMonth) < 1 || atoi(strMonth) > 12 || strMonth.Find ("#")!=-1) {
			MsgBox("Please enter a valid expiration date.");
			return FALSE;
		}
		if(strMonth=="12") {
			//the method we use to store dates acts funky with December, so
			//we cannot just increase the month by 1. However, we know the last
			//day in December is always 31, so it's an easy fix.
			dt.SetDate(atoi(strYear),atoi(strMonth),31);
		}
		else {
			//this method works well for all other months. Set the date to be
			//the first day of the NEXT month, then subtract one day.
			//The result will always be the last day of the month entered.
			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dt.SetDate(atoi(strYear),atoi(strMonth)+1,1);
			dt = dt - dtSpan;
		}
		strCCExpDate = FormatDateTimeForSql(dt, dtoDate);
		// (j.jones 2007-03-13 14:45) - PLID 25183 - ensured we won't save an invalid date
		COleDateTime dtMin;
		dtMin.ParseDateTime("12/31/1899");
		if(dt.m_status == COleDateTime::invalid || dt < dtMin) {
			MsgBox("Please enter a valid date");
			return FALSE;
		}

		pRow = m_pCardList->CurSel;
		if (pRow == NULL) {
			MsgBox("Please enter a card type.");
			return FALSE;
		}

		// (d.thompson 2010-11-08) - PLID 41367 - Ensure that an account is selected, we can't continue without one.  Make sure
		//	this message is provider-agnostic (qbms vs chase vs etc)
		NXDATALIST2Lib::IRowSettingsPtr pAcctRow = m_pAccountList->GetCurSel();
		if(pAcctRow == NULL) {
			//Required for selection.
			AfxMessageBox("Please select an account to use for processing.  This account will determine where "
				"the transaction is deposited.");
			return FALSE;
		}

		return TRUE;
	} NxCatchAll("Error in CheckFields");
	return FALSE;
}

void CCreditCardProcessingDlg::OnProcessCcCard() 
{
	try {
		if(!CheckFields()) {
			return;
		}

		//first, save any changes to the payment
		if (SaveChanges()) {
			// (d.thompson 2010-09-02) - PLID 40371 - Handle CC licensing
			if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
				// (d.thompson 2010-09-02) - PLID 40371 - Renamed to denote QBMS-specific function
				if (QBMS_ProcessTransaction()) {
					//we succeded, we can close
					// (d.thompson 2013-06-06) - PLID 56334 - Move successful close to its own function
					CloseAfterSuccess();
				}
				else {
					//it failed, we already popped up the message box, but we can't close yet
				}
			}
			// (d.thompson 2010-09-02) - PLID 40371 - Handle CC licensing
			else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
				// (d.thompson 2010-09-07) - PLID 40351 - Implement Chase-specific code
				if(Chase_ProcessTransaction()) {
					//success, close
					// (d.thompson 2013-06-06) - PLID 56334 - Move successful close to its own function
					CloseAfterSuccess();
				}
				else {
					//Failed, the process function should alert what the error is, but don't close the dialog.
				}
			}
			else {
				//We already blew up at the load of this dialog, but in this case a developer has defined a new credit card 
				//	processing type, but not implemented processing here.
			}
		}
	} NxCatchAll(__FUNCTION__);
}

//Saves the current transaction record.  You must save upon a successful authorization, but the user may
//	also opt to save if they cannot transmit right now, in order to transmit later.
//If bApproved is false, all other parameters are ignored.  They only need provided if the transaction succeeded.
// (d.thompson 2010-09-02) - PLID 40371 - Renamed to denote QBMS-specific behavior
// (d.thompson 2010-11-08) - PLID 41367 - Added the account ID to be saved with this transaction
bool CCreditCardProcessingDlg::QBMS_SaveTransaction(bool bApproved, QBMS_TransTypes qbTransType, CString strCreditCardTransID, CString strAuthorizationCode, CString strAVSStreet, 
		CString strAVSZip, CString strCardSecurityCodeMatch, CString strPaymentStatus, COleDateTime dtTxnAuthorizationTime, long nAccountID)
{
	//values to save
	CString strAddress, strZip, strSecurityCode;
	GetDlgItemText(IDC_CC_BILLING_ADDRESS, strAddress);
	GetDlgItemText(IDC_CC_BILLING_ZIP, strZip);
	GetDlgItemText(IDC_CC_SECURITY_CODE, strSecurityCode);

	COleCurrency cyTotalAmount;
	if(!GetTotalTransactionAmount(cyTotalAmount)) {
		return false;
	}

	//SecurityCode cannot be saved.  If it's filled, we must give them the option
	//	to abort.  If the transaction was already approved, we don't care, because they don't need the data again.
	if(!bApproved && !strSecurityCode.IsEmpty()) {
		if(IDNO == AfxMessageBox("Per industry regulations, the card security code information cannot be saved.  If you continue saving this "
			"transaction, that information will be lost.  Are you sure you wish to continue?", MB_YESNO))
		{
			return false;
		}
	}

	//Values that may or may not be set depending if we are approved or just saving to 
	//	transmit later.  These values are only saved on successful authorization.
	_variant_t varTransID = g_cvarNull;
	_variant_t varAuthCode = g_cvarNull;
	_variant_t varAVSStreet = g_cvarNull;
	_variant_t varAVSZip = g_cvarNull;
	_variant_t varSecCodeMatch = g_cvarNull;
	_variant_t varPayStatus = g_cvarNull;
	_variant_t varServerAuthTime = g_cvarNull;
	_variant_t varSentDate = g_cvarNull;
	_variant_t varSentByUserID = g_cvarNull;

	bool bUseLocalDateTime = false;
	if(bApproved) {
		varTransID = _bstr_t(strCreditCardTransID);
		varAuthCode = _bstr_t(strAuthorizationCode);
		varAVSStreet = _bstr_t(strAVSStreet);
		varAVSZip = _bstr_t(strAVSZip);
		varSecCodeMatch = _bstr_t(strCardSecurityCodeMatch);
		varPayStatus = _bstr_t(strPaymentStatus);
		varServerAuthTime = COleVariant(dtTxnAuthorizationTime);

		bUseLocalDateTime = true;
		varSentByUserID = (long)GetCurrentUserID();
	}

	//Auth number has historically been tracked in PaymentPlansT, so update that as well.
	if(m_bIsNew) {
		//Insert the transaction, with NULL for the return values, as we have not yet transmitted
		//	This doesn't quite parameterize perfectly because of the SentDate -- but I need the server
		//	datetime saved, not the client datetime.  There will be 2 possible queries.
		// (d.thompson 2010-11-03) - PLID 40558 - Use parameter batch -- if one of these fail, we want both
		//	to fail so the data is not in an unknown state.
		// (d.thompson 2010-11-08) - PLID 41367 - Added account ID
		CParamSqlBatch batch;
		batch.Add( FormatString("INSERT INTO QBMS_CreditTransactionsT "
			"(ID, TotalAmount, BillingAddress, BillingZip, TransactionType, "
			"IsApproved, TransID, AVSStreet, AVSZip, CardSecurityCodeMatch, "
			"PaymentStatus, ServerApprovalDateTime, LocalApprovalDateTime, SentByUserID, AccountID) "
			"values ({INT}, convert(money, {STRING}), {STRING}, {STRING}, {INT}, "
			"{INT}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, "
			"{VT_DATE}, %s, {VT_I4}, {INT});\r\n", bUseLocalDateTime ? "GetDate()" : "NULL"),
			m_nPaymentID, FormatCurrencyForSql(cyTotalAmount), strAddress, strZip, (int)qbTransType,
			bApproved ? 1 : 0, varTransID, varAVSStreet, 
			varAVSZip, varSecCodeMatch, varPayStatus, varServerAuthTime,
			varSentByUserID, nAccountID);

		batch.Add("UPDATE PaymentPlansT SET CCAuthNo = {VT_BSTR} WHERE ID = {INT};", varAuthCode, m_nPaymentID);
		batch.Execute(GetRemoteData());
	}
	else {
		//We can use this dialog to edit existing, saved transactions and run them.
		// (d.thompson 2010-11-03) - PLID 40558 - Use parameter batch -- if one of these fail, we want both
		//	to fail so the data is not in an unknown state.
		// (d.thompson 2010-11-08) - PLID 41367 - Added account ID
		CParamSqlBatch batch;
		batch.Add( FormatString("UPDATE QBMS_CreditTransactionsT "
			"SET TotalAmount = convert(money, {STRING}), BillingAddress = {STRING}, BillingZip = {STRING}, "
			"IsApproved = {INT}, TransID = {VT_BSTR}, "
			"AVSStreet = {VT_BSTR}, AVSZip = {VT_BSTR}, CardSecurityCodeMatch = {VT_BSTR}, "
			"PaymentStatus = {VT_BSTR}, ServerApprovalDateTime = {VT_DATE}, "
			"LocalApprovalDateTime = %s, SentByUserID = {VT_I4}, AccountID = {INT} "
			"WHERE ID = {INT};\r\n", bUseLocalDateTime ? "GetDate()" : "NULL"), 
			FormatCurrencyForSql(cyTotalAmount), strAddress, strZip, bApproved ? 1 : 0, 
			varTransID, varAVSStreet, varAVSZip, varSecCodeMatch, 
			varPayStatus, varServerAuthTime, varSentByUserID, nAccountID, m_nPaymentID);
		batch.Add("UPDATE PaymentPlansT SET CCAuthNo = {VT_BSTR} WHERE ID = {INT};", varAuthCode, m_nPaymentID);
		batch.Execute(GetRemoteData());
	}

	//Now audit our saved data
	CString strResultDesc = (bApproved ? "Approved" : "Not Yet Approved");
	CString strDesc;
	strDesc.Format("%s Transaction: %s (%s)", m_nType == 1 ? "Payment" : "Refund",
		FormatCurrencyForInterface(cyTotalAmount), strResultDesc);
	// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiProcessTransaction, m_nPaymentID, "", strDesc, aepHigh, m_bIsNew ? aetCreated : aetChanged);

	return true;
}

// (d.thompson 2010-09-07) - PLID 40351 - Implemented Chase-specific behaviors
// (d.thompson 2010-11-11) - PLID 40351 - Added Account ID parameter
// (d.thompson 2010-12-20) - PLID 41895 - Added auth code
bool CCreditCardProcessingDlg::Chase_SaveTransaction(bool bApproved, ChaseProcessingUtils::Chase_TransTypes cpTransType, CString strOrderID, 
							CString strTxRefNum, COleDateTime dtProcDateTime, CString strProcStatus, CString strApprovalStatus, 
							CString strRespCode, CString strAVSRespCode, CString strCVVRespCode, CString strRetryTrace, 
							CString strRetryAttemptCount, COleCurrency cyReversalAmtVoided, COleCurrency cyReversalOutstandingAmt, 
							long nAccountID, CString strAuthorizationCode)
{
	//values to save
	CString strAddress, strZip, strSecurityCode;
	GetDlgItemText(IDC_CC_BILLING_ADDRESS, strAddress);
	GetDlgItemText(IDC_CC_BILLING_ZIP, strZip);
	GetDlgItemText(IDC_CC_SECURITY_CODE, strSecurityCode);

	COleCurrency cyTotalAmount;
	if(!GetTotalTransactionAmount(cyTotalAmount)) {
		return false;
	}

	//SecurityCode cannot be saved.  If it's filled, we must give them the option
	//	to abort.  If the transaction was already approved, we don't care, because they don't need the data again.
	if(!bApproved && !strSecurityCode.IsEmpty()) {
		if(IDNO == AfxMessageBox("Per industry regulations, the card security code information cannot be saved.  If you continue saving this "
			"transaction, that information will be lost.  Are you sure you wish to continue?", MB_YESNO))
		{
			return false;
		}
	}

	//Values that may or may not be set depending if we are approved or just saving to 
	//	transmit later.  These values are only saved on successful authorization.
	_variant_t varOrderID = g_cvarNull;
	_variant_t varTxRefNum = g_cvarNull;
	_variant_t varProcDateTime = g_cvarNull;
	_variant_t varProcStatus = g_cvarNull;
	_variant_t varApprovalStatus = g_cvarNull;
	_variant_t varRespCode = g_cvarNull;
	_variant_t varAVSRespCode = g_cvarNull;
	_variant_t varCVVRespCode = g_cvarNull;
	_variant_t varRetryTrace = g_cvarNull;
	_variant_t varRetryAttemptCount = g_cvarNull;
	_variant_t varVoidOutstandingAmt = g_cvarNull;
	_variant_t varVoidAmt = g_cvarNull;
	_variant_t varSentByUserID = g_cvarNull;
	_variant_t varAuthCode = g_cvarNull;

	bool bUseLocalDateTime = false;
	if(bApproved) {
		varOrderID = _bstr_t(strOrderID);
		varTxRefNum = _bstr_t(strTxRefNum);
		varProcDateTime = COleVariant(dtProcDateTime);
		varProcStatus = _bstr_t(strProcStatus);
		varApprovalStatus = _bstr_t(strApprovalStatus);
		varRespCode = _bstr_t(strRespCode);
		varAVSRespCode = _bstr_t(strAVSRespCode);
		varCVVRespCode = _bstr_t(strCVVRespCode);
		varRetryTrace = _bstr_t(strRetryTrace);
		varRetryAttemptCount = _bstr_t(strRetryAttemptCount);
		varAuthCode = _bstr_t(strAuthorizationCode);
		//We'll let SQL convert to money, so use text here
		if(cpTransType == ChaseProcessingUtils::cttRefund) {
			//These are only returned in the case of a Reversal message
			varVoidAmt = _bstr_t(FormatCurrencyForSql(cyReversalAmtVoided));
			varVoidOutstandingAmt = _bstr_t(FormatCurrencyForSql(cyReversalOutstandingAmt));
		}

		bUseLocalDateTime = true;
		varSentByUserID = (long)GetCurrentUserID();
	}

	//Auth number has historically been tracked in PaymentPlansT, so update that as well.
	if(m_bIsNew) {
		//Insert the transaction, with NULL for the return values, as we have not yet transmitted
		//	This doesn't quite parameterize perfectly because of the SentDate -- but I need the server
		//	datetime saved, not the client datetime.  There will be 2 possible queries.
		// (d.thompson 2010-11-11) - PLID 40351 - Support Account ID field
		// (d.thompson 2010-12-20) - PLID 41895 - Added authcode
		CParamSqlBatch batch;
		batch.Add(
			FormatString("INSERT INTO Chase_CreditTransactionsT "
			"(ID, TotalAmount, BillingAddress, BillingZip, TransactionType, IsApproved, "
			"OrderID, TxRefNum, ProcDateTime, ProcStatus, ApprovalStatus, "
			"RespCode, AVSRespCode, CVVRespCode, RetryTrace, retryAttemptCount, "
			"VoidOutstandingAmount, VoidAmount, LocalApprovalDateTime, SentByUserID, "
			"AccountID, AuthorizationCode) "

			"values ({INT}, convert(money, {STRING}), {STRING}, {STRING}, {INT}, {INT}, "	//Through IsApproved
			"{VT_BSTR}, {VT_BSTR}, {VT_DATE}, {VT_BSTR}, {VT_BSTR}, "	//Through ApprovalStatus
			"{VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, "	//Through RetryAttemptCount
			"convert(money, {VT_BSTR}), convert(money, {VT_BSTR}), %s, {VT_I4}, {INT}, {VT_BSTR});\r\n",		//Through AuthorizationCode
			bUseLocalDateTime ? "GetDate()" : "NULL"), 
			//Insert params
			m_nPaymentID, FormatCurrencyForSql(cyTotalAmount), strAddress, strZip, (int)cpTransType, bApproved ? 1 : 0,
			varOrderID, varTxRefNum, varProcDateTime, varProcStatus, varApprovalStatus,
			varRespCode, varAVSRespCode, varCVVRespCode, varRetryTrace, varRetryAttemptCount,
			varVoidOutstandingAmt, varVoidAmt, varSentByUserID, nAccountID, varAuthCode
		);
		batch.Add("UPDATE PaymentPlansT SET CCAuthNo = {VT_BSTR} WHERE ID = {INT};", varAuthCode, m_nPaymentID);
		batch.Execute(GetRemoteData());
	}
	else {
		//We can use this dialog to edit existing, saved transactions and run them.
		// (d.thompson 2010-11-11) - PLID 40351 - Support Account ID field
		// (d.thompson 2010-12-20) - PLID 41895 - Added authcode
		CParamSqlBatch batch;
		batch.Add(
			FormatString("UPDATE Chase_CreditTransactionsT "
			"SET TotalAmount = convert(money, {STRING}), BillingAddress = {STRING}, BillingZip = {STRING}, "
			"IsApproved = {INT}, OrderID = {VT_BSTR}, "
			"TxRefNum = {VT_BSTR}, ProcDateTime = {VT_DATE}, ProcStatus = {VT_BSTR}, "
			"ApprovalStatus = {VT_BSTR}, RespCode = {VT_BSTR}, "
			"AVSRespCode = {VT_BSTR}, CVVRespCode = {VT_BSTR}, RetryTrace = {VT_BSTR}, retryAttemptCount = {VT_BSTR}, "
			"VoidOutstandingAmount = convert(money, {VT_BSTR}), VoidAmount = convert(money, {VT_BSTR}), "

			"LocalApprovalDateTime = %s, SentByUserID = {VT_I4}, AccountID = {INT}, "
			"AuthorizationCode = {VT_BSTR} "
			"WHERE ID = {INT};\r\n"
			, bUseLocalDateTime ? "GetDate()" : "NULL"),
			//Chase params
			FormatCurrencyForSql(cyTotalAmount), strAddress, strZip, bApproved ? 1 : 0, 
			varOrderID, varTxRefNum, varProcDateTime, varProcStatus, varApprovalStatus,
			varRespCode, varAVSRespCode, varCVVRespCode, varRetryTrace, varRetryAttemptCount,
			varVoidOutstandingAmt, varVoidAmt, varSentByUserID, nAccountID, varAuthCode, m_nPaymentID
		);
		batch.Add("UPDATE PaymentPlansT SET CCAuthNo = {VT_BSTR} WHERE ID = {INT};", varAuthCode, m_nPaymentID);
		batch.Execute(GetRemoteData());
	}

	//Now audit our saved data
	CString strResultDesc = (bApproved ? "Approved" : "Not Yet Approved");
	CString strDesc;
	strDesc.Format("%s Transaction: %s (%s)", m_nType == 1 ? "Payment" : "Refund",
		FormatCurrencyForInterface(cyTotalAmount), strResultDesc);
	// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiProcessTransaction, m_nPaymentID, "", strDesc, aepHigh, m_bIsNew ? aetCreated : aetChanged);

	return true;
}


// (d.thompson 2010-09-02) - PLID 40371 - Renamed to denote QBMS-specific function
QBMS_TransTypes CCreditCardProcessingDlg::QBMS_GetCurrentTransType()
{
	QBMS_TransTypes qbTransType = (QBMS_TransTypes)0;
	//Payments
	if (m_nType == 1) {
		qbTransType = qbttSale;
	}
	//Refunds
	else if (m_nType == 3) {
		qbTransType = qbttVoidOrReturn;
	}
	else {
		//Unsupported and we should never get here, blow up
		AfxThrowNxException("Invalid payment method, only payments and refunds are valid.");
	}

	return qbTransType;
}

// (d.thompson 2010-09-07) - PLID 40351 - Implemented chase-specific processing
ChaseProcessingUtils::Chase_TransTypes CCreditCardProcessingDlg::Chase_GetCurrentTransType()
{
	ChaseProcessingUtils::Chase_TransTypes cpTransType = (ChaseProcessingUtils::Chase_TransTypes)0;
	//Payments
	if (m_nType == 1) {
		cpTransType = ChaseProcessingUtils::cttAuthCapture;
	}
	//Refunds
	else if (m_nType == 3) {
		cpTransType = ChaseProcessingUtils::cttRefund;
	}
	else {
		//Unsupported and we should never get here, blow up
		AfxThrowNxException("Invalid payment method, only payments and refunds are valid.");
	}

	return cpTransType;
}

// (d.thompson 2010-09-02) - PLID 40351 - Split out to handle QBMS and Chase both
bool CCreditCardProcessingDlg::GetDialogData(OUT CString &strCleanCCNumber, OUT CString &strExpDate, OUT CString &strMM, OUT CString &strYY, 
											 OUT COleCurrency &cyAmount, OUT CString &strNameOnCard, OUT CString &strSecurityCode, 
											 OUT CString &strAddress, OUT CString &strZip, OUT bool &bCardPresent)
{
	//The credit card number should be formatted to only numbers, nothing else.
	for(int i = 0; i < m_strCCNumber.GetLength(); i++) {
		char ch = m_strCCNumber.GetAt(i);
		if(ch >= '0' && ch <= '9') {
			strCleanCCNumber += ch;
		}
	}

	//Get the expiration date into MM and YY fields
	GetDlgItemText(IDC_CC_AUTH_EXP_DATE, strExpDate);
	if(strExpDate.GetLength() != 5 || strExpDate.GetAt(2) != '/') {
		AfxMessageBox("Your expiration date is invalid.  Please correct it to a valid date and try again.");
		return FALSE;
	}
	strMM = strExpDate.Left(2);
	strYY = strExpDate.Right(2);
	//Ensure they are both 2 digit numbers and nothing else
	if(strMM.SpanIncluding("0123456789") != strMM || strYY.SpanIncluding("0123456789") != strYY) {
		AfxMessageBox("Your expiration date is invalid.  Please correct it to a valid date and try again.");
		return FALSE;
	}

	//Fill in the total transaction amount, including tips
	if(!GetTotalTransactionAmount(cyAmount)) {
		return FALSE;
	}

	//Name on the card
	GetDlgItemText(IDC_CC_AUTH_NAME, strNameOnCard);

	//
	//The following fields are only set on this dialog.  These fields are not currently saved to 
	//	the database.
	GetDlgItemText(IDC_CC_SECURITY_CODE, strSecurityCode);

	GetDlgItemText(IDC_CC_BILLING_ADDRESS, strAddress);
	GetDlgItemText(IDC_CC_BILLING_ZIP, strZip);
	//end one-time only fields
	//


	/////////////////////////////////
	//Now generate the remainder of the data, and 
	//	actually process the transaction.
	/////////////////////////////////
	bCardPresent = false;
	if(IsDlgButtonChecked(IDC_CARD_IS_PRESENT)) {
		bCardPresent = true;
	}

	return TRUE;
}

// (d.thompson 2010-09-02) - PLID 40371 - Renamed.  This function expects to ONLY be called when QBMS processing is in effect.
BOOL CCreditCardProcessingDlg::QBMS_ProcessTransaction()
{
	try {
		//////////////////////////////////
		//First we need to gather a little bit of 
		//	information off this dialog that can't
		//	be looked up.
		//////////////////////////////////
		CString strCleanCCNumber, strExpDate, strMM, strYY, strNameOnCard, strSecurityCode, strAddress, strZip;
		COleCurrency cyAmount;
		bool bCardPresent;

		if(!GetDialogData(strCleanCCNumber, strExpDate, strMM, strYY, cyAmount, strNameOnCard, strSecurityCode, strAddress, strZip, bCardPresent)) {
			//One of the dialog items failed, we cannot continue
			return FALSE;
		}

		//Is this a payment or a refund?
		QBMS_TransTypes qbTransType = QBMS_GetCurrentTransType();

		CString strCreditCardTransID,  strAuthorizationCode,  strAVSStreet, strAVSZip, 
			strCardSecurityCodeMatch, strPaymentStatus;
		COleDateTime dtTxnAuthorizationTime;

		// (d.thompson 2010-11-08) - PLID 41367 - Now that we support multiple QBMS transactions, we need to
		//	supply the connection ticket to use for each one when processing.
		NXDATALIST2Lib::IRowSettingsPtr pAcctRow = m_pAccountList->GetCurSel();

		//Use the selected account
		long nAccountID = VarLong(pAcctRow->GetValue(eacID));
		CQBMSProcessingAccount acct = QBMSProcessingUtils::GetAccountByID(nAccountID);

		// (d.thompson 2010-11-08) - PLID 41367 - Pass in the connection ticket and production flag
		if(!QBMSProcessingUtils::ProcessQBMSTransaction(acct.GetIsProduction(), acct.GetDecryptedTicket(), qbTransType, m_nPaymentID, strCleanCCNumber, 
			cyAmount, strNameOnCard, strMM, strYY, strSecurityCode, strAddress, strZip, 
			m_bIsSwiped, m_strTrack2Data, bCardPresent, m_strApplyToPaymentTransID,
			m_nPatientID, // (a.walling 2010-01-25 09:02) - PLID 37026 - Pass in the patient ID
			m_strPatientName, m_nPaymentID,
			//Output data
			strCreditCardTransID, strAuthorizationCode, strAVSStreet, 
			strAVSZip, strCardSecurityCodeMatch, strPaymentStatus, dtTxnAuthorizationTime
			))
		{
			//Failure while processing.  Offer that they can save it for editing later.
			OfferSaveForLater("");
			return FALSE;
		}

		//We need to save any info returned by successful authorization
		// (d.thompson 2010-11-03) - PLID 40558 - Extra safety.  At this point, the cc HAS been approved.  We need to save this
		//	information to data... but what if it fails?  Most common reason would be loss of connection to the server.  If so, 
		//	we can't allow them to process this payment again (it already processed), but if we can't save, how do we inform
		//	the user of this?  We'll just have to alert them to call support immediately.
		try {
			if(!QBMS_SaveTransaction(true, qbTransType, strCreditCardTransID, strAuthorizationCode, strAVSStreet, 
				strAVSZip, strCardSecurityCodeMatch, strPaymentStatus, dtTxnAuthorizationTime, nAccountID)) 
			{
				// (d.thompson 2010-11-03) - PLID 40558 - The above function is assumed to NEVER fail in this case.  At present time
				//	it has only 2 points that can return false, and neither can possibly be reached on a successful authorization.  If
				//	we get here, we need to treat this as an exception case.  The payment has already been successfully processed, we 
				//	cannot offer to process it again!
				AfxThrowNxException("QBMS credit card transaction was successful, however there was a problem saving the information to data.");
			}
		} NxCatchAll("Your card was processed successfully, however NexTech was unable to save this information to the database.  Please contact "
			"NexTech Technical Support immediately to help resolve this issue.  Please do not attempt to process the payment again.\r\n\r\n"
			"Auth Code:  " + strAuthorizationCode);

		// (d.thompson 2009-07-08) - PLID 34809 - Alert the user that the transaction was successful.
		CTimedMessageBoxDlg dlgMsg(this);
		dlgMsg.m_bShowCancel = false;
		dlgMsg.m_bAllowTimeout = true;
		dlgMsg.m_nTimeoutSeconds = 2;
		dlgMsg.m_strMessageText = "Your transaction has been processed successfully!";
		dlgMsg.DoModal();

		//We have successfully processed the credit card charge.  Go ahead and print a receipt
		//	for the patient.
		PrintReceipt(strAuthorizationCode, qbTransType == qbttSale ? "Sale" : "Refund", m_bIsSwiped);

		return TRUE;
	} NxCatchAll("Error in CCreditCardProcessingDlg::ProcessTransaction");

	try {
		OfferSaveForLater("Your connection to the processing server appears to have failed.");
	} NxCatchAll("Error in CCreditCardProcessingDlg::ProcessTransaction : Post-Exception");

	return FALSE;
}

// (d.thompson 2010-09-07) - PLID 40351 - Chase specific processing
BOOL CCreditCardProcessingDlg::Chase_ProcessTransaction()
{
	try {
		//Gather dialog data for processing.
		CString strCleanCCNumber, strExpDate, strMM, strYY, strNameOnCard, strSecurityCode, strAddress, strZip;
		COleCurrency cyAmount;
		bool bCardPresent;

		//These are out output data fields
		CString strOrderID, strTxRefNum, strProcStatus, strApprovalStatus, strAVSRespCode, strCVVRespCode, strRespCode,
			strRetryTrace, strRetryAttemptCount, strAuthorizationCode;
		COleDateTime dtProcDateTime;
		COleCurrency cyReversalAmtVoided, cyReversalOutstandingAmt;

		// (d.thompson 2010-11-11) - PLID 40351 - Added support for multiple accounts
		NXDATALIST2Lib::IRowSettingsPtr pAcctRow = m_pAccountList->GetCurSel();
		long nAccountID = VarLong(pAcctRow->GetValue(eacID));
		BOOL bIsProduction = VarBool(pAcctRow->GetValue(eacIsProduction));
		CString strUsername = VarString(pAcctRow->GetValue(eacUsername));
		CString strMerchantID = VarString(pAcctRow->GetValue(eacMerchantID));
		CString strTerminalID = VarString(pAcctRow->GetValue(eacTerminalID));
		CString strTextPassword;
		{
			//Password is loaded as a variant and needs decrypted
			_variant_t varPassword = pAcctRow->GetValue(eacPassword);
			strTextPassword = DecryptStringFromVariant(varPassword);
		}

		if(!GetDialogData(strCleanCCNumber, strExpDate, strMM, strYY, cyAmount, strNameOnCard, strSecurityCode, strAddress, strZip, bCardPresent)) {
			//One of the dialog items failed, we cannot continue
			return FALSE;
		}

		//Future:  We should really get away from the y2k problems of assuming on 2 digit years, and the exp dates should all 
		//	be using 4 digit years.  In fact, we save the date as 4 years to data, then try to hide it in the interface!  For
		//	now, we'll just prepend the century, this will be good for another 90 years.  Added PLID 41559 to do this.
		CString strYYYY = "20" + strYY;

		//What type of processing are we handling?  Payment?  Refund?  etc
		ChaseProcessingUtils::Chase_TransTypes cpTransType = Chase_GetCurrentTransType();

		//Process the transaction
		// (d.thompson 2010-11-17) - PLID 41516 - Added account details as parameters
		// (d.thompson 2010-12-20) - PLID 41895 - Added auth code
		if(!ChaseProcessingUtils::ProcessChaseTransaction(cpTransType, bIsProduction, strUsername,
			strTextPassword, strMerchantID, strTerminalID, 
			m_nPaymentID, strCleanCCNumber, 
			cyAmount, strNameOnCard, strMM, strYYYY, strSecurityCode, strAddress, strZip, 
			m_bIsSwiped, m_strTrack2Data, m_strApplyToPaymentTransID,
			m_nPatientID, m_strPatientName, m_nPaymentID,
			/* Output Data fields */
			strOrderID, strTxRefNum, dtProcDateTime, strProcStatus, strApprovalStatus, strRespCode,
			strAVSRespCode, strCVVRespCode, strRetryTrace, strRetryAttemptCount, cyReversalAmtVoided, cyReversalOutstandingAmt, strAuthorizationCode))
		{
			//Failure while processing.  Offer that they can save it for editing later.
			OfferSaveForLater("");
			return FALSE;
		}

		//Otherwise, we succeeded! Yay...  Now save any of the info that we got back from the process function.

		// (d.thompson 2010-11-03) - PLID 40351 by way of 40558 - Extra safety.  At this point, the cc HAS been approved.  We need to save this
		//	information to data... but what if it fails?  Most common reason would be loss of connection to the server.  If so, 
		//	we can't allow them to process this payment again (it already processed), but if we can't save, how do we inform
		//	the user of this?  We'll just have to alert them to call support immediately.
		// (d.thompson 2010-12-20) - PLID 41895 - added auth code
		try {
			if(!Chase_SaveTransaction(true, cpTransType, 
				strOrderID, strTxRefNum, dtProcDateTime, strProcStatus, strApprovalStatus, strRespCode,
				strAVSRespCode, strCVVRespCode, strRetryTrace, strRetryAttemptCount, 
				cyReversalAmtVoided, cyReversalOutstandingAmt, nAccountID, strAuthorizationCode))
			{
				// (d.thompson 2010-11-03) - PLID 40351 by way of 40558 - The above function is assumed to NEVER fail in this case.  At present time
				//	it has only 2 points that can return false, and neither can possibly be reached on a successful authorization.  If
				//	we get here, we need to treat this as an exception case.  The payment has already been successfully processed, we 
				//	cannot offer to process it again!
				AfxThrowNxException("Chase credit card transaction was successful, however there was a problem saving the information to data.");
			}
		} NxCatchAll("Your card was processed successfully, however NexTech was unable to save this information to the database.  Please contact "
			"NexTech Technical Support immediately to help resolve this issue.  Please do not attempt to process the payment again.\r\n\r\n"
			"Auth Code:  " + strAuthorizationCode);

		// (d.thompson 2009-07-08) - PLID 34809 - Alert the user that the transaction was successful.
		CTimedMessageBoxDlg dlgMsg(this);
		dlgMsg.m_bShowCancel = false;
		dlgMsg.m_bAllowTimeout = true;
		dlgMsg.m_nTimeoutSeconds = 2;
		dlgMsg.m_strMessageText = "Your transaction has been processed successfully!";
		dlgMsg.DoModal();

		//We have successfully processed the credit card charge.  Go ahead and print a receipt
		//	for the patient.
		PrintReceipt(strAuthorizationCode, cpTransType == ChaseProcessingUtils::cttAuthCapture ? "Sale" : "Refund", m_bIsSwiped);

		//reach here, everything went well, we're done and happy
		return TRUE;
	} NxCatchAll("Error in CCreditCardProcessingDlg::Chase_ProcessTransaction");

	//An exception occurred somewhere in the above code.  Most likely culprit is a connection failure somewhere.  
	//	Offer to save as best we can.
	try {
		OfferSaveForLater("Your connection to the processing server appears to have failed.");
	} NxCatchAll("Error in CCreditCardProcessingDlg::Chase_ProcessTransaction : Post-Exception");

	return FALSE;
}


void CCreditCardProcessingDlg::OfferSaveForLater(CString strPreText)
{
	//We need to offer them the ability to save for later
	if(!strPreText.IsEmpty()) { 
		strPreText += "\r\n\r\n";
	}
	//Add an additional warning about swiped cards.  We are not allowed to save the track data, and if
	//	they re-process later, they may not get a reduced rate on the transaction.
	CString strPostText;
	if(m_bIsSwiped) {
		strPostText = "\r\n\r\n"
			"Note:  This transaction is based off a swiped credit card.  Because this information cannot be saved, "
			"re-processing the transaction at a later point may result in increased fees through your service.";
	}
	AfxMessageBox(strPreText + 
		"If you would like to save this information and try again later, please press the 'Save and Process Later' button.\r\n"
		"If you would like to cancel this processing attempt entirely, please press the 'Cancel' button.\r\n"
		"Otherwise, you may attempt to re-process the transaction now." + strPostText);

	GetDlgItem(IDC_SAVE_AND_PROCESS_LATER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SAVE_AND_PROCESS_LATER)->EnableWindow(TRUE);
}

// (d.thompson 2009-04-13) - PLID 33957 - This function is now 'save and process later'.  It will save the transaction in data, 
//	unprocessed, and leave it for someone else to check later.
void CCreditCardProcessingDlg::OnOK() 
{
	OnBnClickedSaveAndProcessLater();
}

void CCreditCardProcessingDlg::OnBnClickedSaveAndProcessLater()
{
	try {
		if(!CheckFields()) {
			return;
		}

		//save changes to the payment just in case they changed anything
		SaveChanges();

		// (d.thompson 2010-09-02) - PLID 40371 - Handle specific requirements per processing type
		if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
			// (d.thompson 2010-11-08) - PLID 41367 - Now that we support multiple QBMS transactions, we need to
			//	supply the connection ticket to use for each one when processing.
			NXDATALIST2Lib::IRowSettingsPtr pAcctRow = m_pAccountList->GetCurSel();

			//Use the selected account
			long nAccountID = VarLong(pAcctRow->GetValue(eacID));
			CQBMSProcessingAccount acct = QBMSProcessingUtils::GetAccountByID(nAccountID);

			//Now save the transaction record in QBMS_CreditTransactionsT
			QBMS_TransTypes qbTransType = QBMS_GetCurrentTransType();
			COleDateTime dt;
			if(!QBMS_SaveTransaction(false, qbTransType, "", "", "", "", "", "", dt, acct.GetID())) {
				return;
			}
		}
		// (d.thompson 2010-09-02) - PLID 40371 - Handle specific requirements per processing type
		else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
			// (d.thompson 2010-11-11) - PLID 40351 - Implement Chase-specific handling to save to process later.
			NXDATALIST2Lib::IRowSettingsPtr pAcctRow = m_pAccountList->GetCurSel();
			long nAccountID = VarLong(pAcctRow->GetValue(eacID));

			//Go ahead and save the request.  All response fields will be empty
			ChaseProcessingUtils::Chase_TransTypes chTransType = Chase_GetCurrentTransType();
			COleDateTime dt;
			dt.SetStatus(COleDateTime::invalid);	//save function will ignore invalid dates
			if(!Chase_SaveTransaction(false, chTransType, "", "", dt, "", "", "", "", "", ""/*RetryTrace*/, ""/*retrycount*/, COleCurrency(0, 0), COleCurrency(0, 0), nAccountID, "")) {
				return;
			}
		}

		CDialog::OnOK();
	} NxCatchAll(__FUNCTION__);	
}

void CCreditCardProcessingDlg::OnCancel() 
{
	try {
		// (d.thompson 2009-04-15) - PLID 33957 - If we are editing an existing transaction to re-process, 
		//	cancelling is a valid operation -- it will just not do anything.
		if(!m_bIsNew) {
			CDialog::OnCancel();
			return;
		}

		//This no longer deletes, it just drops them back to the payment dialog.
		if (IDYES == AfxMessageBox("This transaction has not been processed yet.\r\n"
			"Are you sure you want to do this?", MB_YESNO))
		{
			CDialog::OnCancel();
		}
		else {
			//we are leaving it open because they don't want to close it
		}
	} NxCatchAll(__FUNCTION__);
}

/***************************************
		Receipt Printing Code
***************************************/
#pragma region Receipt Printing Code
// (j.gruber 2007-08-02 17:13) - PLID 26650 - customer copy receipt
// (d.thompson 2011-01-05) - PLID 41991 - Added params:
//	strSaleType:  String that will go on the receipt explaining the type of sale.  This is a PCI guideline for printed receipts.  Should be something like "Sale" or "Refund"
//	bIsSwiped:  Flag to state if the transaction was swiped.  This indication is also placed on the receipt.
BOOL CCreditCardProcessingDlg::PrintReceipt(CString strApprovalCode, CString strSaleType, bool bIsSwiped)
{
	try {
		// (j.gruber 2007-07-30 10:15) - PLID 26720 - run a regular report if they don't have a receipt printer
		if (GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE)) {
			//first let's acquire the receipt printer
			CWnd *pMainFrame = GetMainFrame();

			if (pMainFrame && GetMainFrame()->CheckPOSPrinter()) {
				CString strOutput;
				// (d.thompson 2011-01-05) - PLID 41994 - Refunds don't get auth codes for either chase or qbms, so don't 
				//	try to format if it's empty.
				if(!strApprovalCode.IsEmpty()) {
					strOutput = "Approval Code: " + strApprovalCode;
				}

				//TES 12/6/2007 - PLID 28192 - We're ready to start using the POS Printer, so claim it.
				// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
				// not to mention providing exception safety.
				//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();
				POSPrinterAccess pPOSPrinter;
				if (pPOSPrinter) {
					//First print the header
					if (!PrintReceiptHeader(pPOSPrinter)) {
						//GetMainFrame()->ReleasePOSPrinter();
						return FALSE;
					}

					// (d.thompson 2011-01-05) - PLID 41994 - Added sale type
					if (!PrintReceiptMiddle(pPOSPrinter, strOutput, strSaleType)) {
						//GetMainFrame()->ReleasePOSPrinter();
						return FALSE;
					}

					if (! PrintReceiptFooter(pPOSPrinter)) {
						//GetMainFrame()->ReleasePOSPrinter();
						return FALSE;
					}

					//feed some line out
					// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
					pPOSPrinter->PrintText("\n\n\n\n\n\n");

					// (a.walling 2011-04-28 10:02) - PLID 43492
					if (!pPOSPrinter->FlushAndTryCut()) {
						return FALSE;
					}

					//TES 12/6/2007 - PLID 28192 - Now release the POS Printer, since we're done with it, so any third-party
					// applications that want it can use it.
					//GetMainFrame()->ReleasePOSPrinter();
					return TRUE;
				}
				else {
					return FALSE;
				}
			}
			else {
				return FALSE;
			}
		}
		else {
			//they don't have a receipt printer configured, run the standard one
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(604)]);
			infReport.nExtraID = m_nPaymentID;

			//Set up the parameters.
			CPtrArray paParams;
			CRParameterInfo *paramInfo;

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = GetCurrentUserName();
			paramInfo->m_Name = "CurrentUserName";
			paParams.Add(paramInfo);

			// (d.thompson 2009-07-06) - PLID 34764 - Must follow the tip line preference
			paramInfo = new CRParameterInfo;
			paramInfo->m_Name = "ShowTipLine";
			paramInfo->m_Data = FormatString("%li", GetRemotePropertyInt("CCProcessingShowTipLine", 0, 0, "<None>", true));
			paParams.Add(paramInfo);

			// (d.thompson 2011-01-05) - PLID 41991 - Need the card swipe info and trans type info, requirements for the receipt.  This 
			//	code matches that in paymentdlg for the merchant copy.
			paramInfo = new CRParameterInfo;
			if (bIsSwiped) {
				paramInfo->m_Data = "Card Swiped";
			}
			else {
				paramInfo->m_Data = "Card Manually Entered";
			}
			paramInfo->m_Name = "CardSwipeInfo";
			paParams.Add(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strSaleType;
			paramInfo->m_Name = "TransactionType";
			paParams.Add(paramInfo);


			CPrintInfo prInfo;
			CPrintDialog* dlg;
			dlg = new CPrintDialog(FALSE);
			prInfo.m_bPreview = false;
			prInfo.m_bDirect = false;
			prInfo.m_bDocObject = false;
			if (prInfo.m_pPD) delete prInfo.m_pPD;
			prInfo.m_pPD = dlg;

			BOOL bReturn = RunReport(&infReport, &paParams, FALSE, this, "Credit Card Customer Copy", &prInfo);
			ClearRPIParameterList(&paParams);

			return bReturn;
		}

	} NxCatchAll("Error in CCreditCardProcessingDlg::PrintReceipt");

	return FALSE;
}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
BOOL CCreditCardProcessingDlg::PrintReceiptFooter(COPOSPrinterDevice *pPOSPrinter)
{
	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();

	// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
	strOutput += "\n\n\n";
	CString strAmount = "Amount: " + FormatCurrencyForInterface(m_cyPayAmount);

	//put the amount
	long nWhiteLen = nLineWidth - strAmount.GetLength();
	for (int i = 0; i < nWhiteLen; i++) {
		strAmount = " " + strAmount;
	}
	strOutput += LeftJustify(strAmount, nLineWidth);

	//first give them some lines

	// (j.gruber 2007-08-07 10:37) - PLID 26997 - only show the tip and total lines if they want to
	// (d.thompson 2009-07-06) - PLID 34764 - Slightly reworked this preference, default to off
	if(GetRemotePropertyInt("CCProcessingShowTipLine", 0, 0, "<None>", true)) {
		//now print the tip line
		COleCurrency cyTipAmount = GetChargeTipTotal();
		CString strDesc = "Tip: " + FormatCurrencyForInterface(cyTipAmount);
		//figure out the white space
		nWhiteLen = nLineWidth - strDesc.GetLength();
		for (i = 0; i < nWhiteLen; i++) {
			strDesc = " " + strDesc;
		}
		strOutput += LeftJustify(strDesc, nLineWidth);

		//now the Total line
		strDesc = "Total: " + FormatCurrencyForInterface(m_cyPayAmount + cyTipAmount);
		nWhiteLen = nLineWidth - strDesc.GetLength();
		for (i = 0; i < nWhiteLen; i++) {
			strDesc = " " + strDesc;
		}
		strOutput += LeftJustify(strDesc, nLineWidth);
	}

	strOutput += "\n\n";
	//This copy is destined for the customer
	strOutput += CenterLine("Customer Copy", nLineWidth);

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

CString CCreditCardProcessingDlg::LeftJustify(CString strText, long nLineWidth)
{
	//this is really just printing out the line regularly, but I put it in a function in case we 
	//do something with it in the future
	CString strDesc;
	strDesc = strText;

	return strDesc + "\n";
}

CString CCreditCardProcessingDlg::LeftRightJustify(CString strTextLeft, CString strTextRight, long nLineWidth)
{
	CString strDesc;

	long nSpaceLength = nLineWidth - strTextLeft.GetLength() - strTextRight.GetLength();
	strDesc = strTextLeft;
	for (int i = 0; i < nSpaceLength - 1; i++) {
		strDesc += " ";
	}

	strDesc += strTextRight;

	return strDesc + "\n";
}

CString CCreditCardProcessingDlg::CenterLine(CString strText, long nLineWidth)
{
	CString strDesc;
	
	long nCenterLine = (nLineWidth)/2;
	long nCenterName = strText.GetLength()/2;
	long nDiff = nCenterLine - nCenterName;
	for (int i = 0; i < nDiff; i++) {
		strDesc += " ";
	}
	strDesc += strText;

	return strDesc + "\n";
}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
BOOL CCreditCardProcessingDlg::PrintReceiptHeader(COPOSPrinterDevice *pPOSPrinter)
{
	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();
	CString strLine;

	//get the location information
	_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, Phone "
		"FROM LocationsT WHERE ID = {INT}", m_nLocationID);

	if (rsLocation->eof) {
		ThrowNxException("Could not obtain location information");
		return FALSE;
	}

	CString strName, strAdd1, strAdd2, strCity, strState, strZip, strPhone;

	strName = AdoFldString(rsLocation, "Name");
	strAdd1 = AdoFldString(rsLocation, "Address1", "");
	strAdd2 = AdoFldString(rsLocation, "Address2", "");
	strCity = AdoFldString(rsLocation, "City", "");
	strState = AdoFldString(rsLocation, "State", "");
	strZip = AdoFldString(rsLocation, "Zip", "");
	strPhone = AdoFldString(rsLocation, "Phone", "");

	//figure out the centering for each value
	strOutput += CenterLine(strName, nLineWidth);
	strOutput += CenterLine(strAdd1, nLineWidth);

	if (!strAdd2.IsEmpty()) {
		strOutput += CenterLine(strAdd2, nLineWidth);
	}

	CString strCSZ = strCity + ", " + strState + " " + strZip;
	strOutput += CenterLine(strCSZ, nLineWidth);
	strOutput += CenterLine(strPhone, nLineWidth);
	// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
	strOutput += "\n\n";

	//output them in bold, centered
	CString strTmp = char(27);
	strOutput = strTmp + "|bC" + strOutput;

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
BOOL CCreditCardProcessingDlg::PrintReceiptMiddle(COPOSPrinterDevice *pPOSPrinter, CString strOutputMessage, CString strSaleType)
{
	//now put the input name
	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();

	//the user who processed it
	strOutput += LeftJustify(GetCurrentUserName(), nLineWidth);

	//now the date and time
	COleDateTime dtNow;
	dtNow = COleDateTime::GetCurrentTime();
	strOutput += LeftRightJustify(FormatDateTimeForInterface(dtNow, NULL, dtoDate, FALSE), 
		FormatDateTimeForInterface(dtNow, NULL, dtoTime, FALSE), nLineWidth);

	CString strCardNumber;
	//to get here, they have to have a card number
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pCardList->CurSel;
	CString strCardName = VarString(pRow->GetValue(clcName));
	//(e.lally 2007-10-30) PLID 27892 - The control only shows the last 4 digits now always, so
	//we don't have to redo that formatting.
	GetDlgItem(IDC_CC_AUTH_NUMBER)->GetWindowText(strCardNumber);

	//now the card type and number
	strOutput += LeftRightJustify(strCardName, strCardNumber, nLineWidth);

	// (d.thompson 2011-01-05) - PLID 41994 - Visa guidelines indicate we need to include the status  whether it was 
	//	sale/refund, and whether it was manually entered or swiped.
	//sale type = strSaleType parameter

	//whether the card was swiped
	CString strSwiped;
	if (m_bIsSwiped) {
		strSwiped = "Card Swiped";
	}
	else {
		strSwiped  = "Card Manually Entered";
	}

	//Output both of these.
	strOutput += LeftRightJustify(strSwiped, strSaleType, nLineWidth);


	//Next output our output message, if one exists
	if(!strOutputMessage.IsEmpty()) {
		strOutput += LeftJustify(strOutputMessage, nLineWidth);
	}

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

#pragma endregion


/***************************************
		Interface Functionality
***************************************/
#pragma region In-memory Interface Functionality

void CCreditCardProcessingDlg::OnTrySetSelFinishedCardTypeList(long nRowEnum, long nFlags) 
{
	try {
		if (nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			// We only want to handle the case were the card type could not be selected after swiping
			if (GetMainFrame()->DoesOPOSMSRDeviceExist()) {
				MsgBox("The card type could not be found in the list, please add the card type to the Card Type list.");
				m_nCCCardID = -1;
			}
		}
	} NxCatchAll("Error in CCreditCardProcessingDlg::OnTrySetSelFinishedCardTypeList");
}

BOOL CCreditCardProcessingDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch(wParam) {
		case ID_PAYMENTTIPS_ADD:	//popup menu - Add new tip
			OnAddTipBtn();
			return TRUE;

		case ID_PAYMENTTIPS_DELETE:	//popup menu - Delete current tip
			OnDeleteTip();
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CCreditCardProcessingDlg::OnDeleteTip()
{
	try {
		//ensure they have permission
		if(!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false))
			return;

		//delete the currently selected tip
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTipList->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		if(MsgBox(MB_YESNO, "Are you sure you wish to delete this tip?") != IDYES) {
			return;
		}

		//we don't really delete it, we just remove the row and set the ID in an array to delete later
		long nID = VarLong(pRow->GetValue(tlcID));

		//flag it in our delete list
		m_aryDeleted.Add(nID);

		//remove the row
		m_pTipList->RemoveRow(pRow);

		//TES 4/15/2015 - PLID 65177 - Allow for the Charge Refund paymethod
		if (VarLong(pRow->GetValue(tlcPayMethod)) == 3 || VarLong(pRow->GetValue(tlcPayMethod == 9))) {
			//subtract from the total amount
			UpdateTotalTransAmt();
		}

		// (a.walling 2007-09-21 17:10) - PLID 27468 - We also need to update the payment info if we have it
		if ( (nID != -1) && (m_pInfo != NULL) ) {
			// unfortunately we have to query data since we don't have any guarantee the paytype was not 
			// changed before it was deleted.

			_RecordsetPtr prs = CreateParamRecordset("SELECT PayMethod FROM PaymentTipsT WHERE ID = {INT}", nID);
			if(!prs->eof) {
				long nOldMethod = AdoFldLong(prs, "PayMethod");

				if (m_pInfo->IsIDInExistingTipIDs(nID)) {
					// this was an existing tip in the payment dialog. Was it modified?
					if (m_pInfo->IsIDInModifiedTipIDs(nID)) {
						// yeah, it was modified, so we need to update arChangedTips
						m_pInfo->arChangedTips[nOldMethod-1]--;
					} else {
						// it was not modified, so it isn't in the payment info struct at all, so we are good.
					}
				} else {
					// this was a new tip in the payment dialog, so it will be in arNewTips
					m_pInfo->arNewTips[nOldMethod-1]--;
				}
			}
		}

		//update the total
		UpdateTotalTipAmt();

	} NxCatchAll(__FUNCTION__);
}

void CCreditCardProcessingDlg::UpdateTotalTransAmt()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTipList->GetFirstRow();

		COleCurrency cy(0, 0);
		while (pRow) {
			//TES 4/15/2015 - PLID 65177 - Allow for the Charge Refund paymethod
			if (VarLong(pRow->GetValue(tlcPayMethod)) == 3 || VarLong(pRow->GetValue(tlcPayMethod)) == 9) {
				COleCurrency cyAmt = VarCurrency(pRow->GetValue(tlcAmount));
				cy += cyAmt;
			}
			pRow = pRow->GetNextRow();
		}

		//add the payment amount
		cy += m_cyPayAmount;

		//now update the box
		SetDlgItemText(IDC_TOTAL_TRANS_AMT, FormatCurrencyForInterface(cy));

	} NxCatchAll(__FUNCTION__);
}

void CCreditCardProcessingDlg::OnUpdateCcAuthExpDate() 
{
	try {
		CString str;
		GetDlgItem(IDC_CC_AUTH_EXP_DATE)->GetWindowText(str);
		FormatItemText(GetDlgItem(IDC_CC_AUTH_EXP_DATE), str, "##/##nn");
	} NxCatchAll("Error in OnUpdateCcAuthExpDate");
}

void CCreditCardProcessingDlg::OnKillfocusCCNumber()
{
	try {
		HandleOnKillfocusCCNumber();
	} NxCatchAll("Error in OnKillfocusCCNumber");
}

void CCreditCardProcessingDlg::HandleOnKillfocusCCNumber()
{
	//(e.lally 2007-10-30) PLID 27892 - We need to set our member variable for credit card number
	//	-If the card number actually changed (this could be tricky)
	CString strDisplayedCardNumber;
	GetDlgItemText(IDC_CC_AUTH_NUMBER, strDisplayedCardNumber);
	if(MaskCCNumber(m_strCCNumber) != strDisplayedCardNumber){
		//Check if someone manually entered an 'X' in the card number, or
		//edited the card number, but left one or more 'X' in the box.
		if(strDisplayedCardNumber.Find("X") >= 0){
			CString strMessage = "When changing the credit card number, you must re-enter the entire number.\n"
				"The card number will be set back to the previous value.";
			MessageBox(strMessage,"Practice",MB_OK|MB_ICONINFORMATION);
			//Set the display back to what it was
			strDisplayedCardNumber = MaskCCNumber(m_strCCNumber);
			SetDlgItemText(IDC_CC_AUTH_NUMBER, strDisplayedCardNumber);
			SetFocus();
			//go no further
			return;
		}

		m_strCCNumber = strDisplayedCardNumber;
		strDisplayedCardNumber = MaskCCNumber(m_strCCNumber);
		SetDlgItemText(IDC_CC_AUTH_NUMBER, strDisplayedCardNumber);
		m_bHasChanged = TRUE;
	}
}

COleCurrency CCreditCardProcessingDlg::GetChargeTipTotal()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pTipList->GetFirstRow();

	COleCurrency cy(0, 0);
	while (pRow) {
		//TES 4/15/2015 - PLID 65177 - Allow for the Charge Refund paymethod
		if (VarLong(pRow->GetValue(tlcPayMethod)) == 3 || VarLong(pRow->GetValue(tlcPayMethod)) == 9) {
			COleCurrency cyAmt = VarCurrency(pRow->GetValue(tlcAmount));
			cy += cyAmt;
		}
		pRow = pRow->GetNextRow();
		
	}

	return cy;
}

void CCreditCardProcessingDlg::OnRequeryFinishedCcAuthTipList(short nFlags) 
{
	try {
		//add up the amounts
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTipList->GetFirstRow();

		COleCurrency cyTransAmount;
		COleCurrency cyTipAmount;

		while (pRow) {
			//if its a charge tip
			//TES 4/15/2015 - PLID 65177 - Allow for the Charge Refund paymethod
			if (VarLong(pRow->GetValue(tlcPayMethod)) == 3 || VarLong(pRow->GetValue(tlcPayMethod)) == 9) {
				cyTransAmount += VarCurrency(pRow->GetValue(tlcAmount));
			}

			cyTipAmount += VarCurrency(pRow->GetValue(tlcAmount));
			pRow = pRow->GetNextRow();
		}

		SetDlgItemText(IDC_TOTAL_TIP_AMT, FormatCurrencyForInterface(cyTipAmount));
		cyTransAmount += m_cyPayAmount;

		SetDlgItemText(IDC_TOTAL_TRANS_AMT, FormatCurrencyForInterface(cyTransAmount));
	} NxCatchAll(__FUNCTION__);
}

void CCreditCardProcessingDlg::OnRButtonDownCcAuthTipList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//set the selection
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pTipList->PutCurSel(pRow);

		//pop up a menu with the ability to add or delete
		CMenu mnu;
		mnu.LoadMenu(IDR_PAYMENT_TIPS);
		CMenu *pmnuSub = mnu.GetSubMenu(0);
		if (pmnuSub) {
			CPoint pt;
			pt.x = x;	pt.y = y;
			GetDlgItem(IDC_CC_AUTH_TIP_LIST)->ClientToScreen(&pt);

			// Hide certain items if we're not on a row
			if (pRow) {
				pmnuSub->SetDefaultItem(ID_PAYMENTTIPS_ADD);				
			} else {
				pmnuSub->EnableMenuItem(ID_PAYMENTTIPS_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			}
			// Show the popup
			pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		}
	
	} NxCatchAll(__FUNCTION__);	
}

void CCreditCardProcessingDlg::OnEditingFinishedCcAuthTipList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		//update the transaction amount if necessary
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		switch (nCol) {
			case tlcAmount:
				//TES 4/15/2015 - PLID 65177 - Allow for the Charge Refund paymethod
				if (VarLong(pRow->GetValue(tlcPayMethod)) == 3 || VarLong(pRow->GetValue(tlcPayMethod)) == 9) {
					UpdateTotalTransAmt();
				}	
				UpdateTotalTipAmt();
			break;

			case tlcPayMethod:
				//TES 4/15/2015 - PLID 65177 - Allow for the Charge Refund paymethod
				if ((VarLong(varNewValue) == 3 || VarLong(varNewValue) == 9) && (VarLong(varOldValue) != 3 && VarLong(varOldValue) != 9)) {
					UpdateTotalTransAmt();
				}	
				else if ((VarLong(varOldValue) == 3 || VarLong(varOldValue) == 9) && (VarLong(varNewValue) != 3 && VarLong(varNewValue) != 9)) {
					//they changed from charge to something else
					UpdateTotalTransAmt();
				}	
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

LRESULT CCreditCardProcessingDlg::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		//alrighty, let's fill our information with this card
		MSRTrackInfo *mtiInfo = (MSRTrackInfo*)wParam;
		// (j.jones 2009-06-19 10:56) - PLID 33650 - changed boolean to be an enum
		if(mtiInfo->msrCardType == msrCreditCard) {
			if (IDYES == MsgBox(MB_YESNO, "A credit card has been swiped, would you like to process this transaction with the newly swiped card?") ) {

				CreditCardInfo *cciCardInfo = (CreditCardInfo*)lParam;

				// Fill the information from the credit card to the screen
				// Set the card type
				CString strSwipedCardName = cciCardInfo->m_strCardType;
				// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
				// (e.lally 2007-07-12) PLID 26590 - The member card name string needs to be reset so the trysetsel
					//doesn't accidently add an inactive card somehow
				strSwipedCardName = "";
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				long nResult = m_pCardList->TrySetSelByColumn_Deprecated(clcName ,_variant_t(cciCardInfo->m_strCardType));
				if (nResult == NXDATALIST2Lib::sriNoRow) {
					// The credit card type could not be selected
					CString strMsg;
					strMsg.Format("The credit card type \'%s\' could not be selected for the card just swiped because is does not\n"
								"exist in the card type list.  Please add this card type to the list before trying to process this card.", cciCardInfo->m_strCardType);
					MessageBox(strMsg);
					// Set the card type to be nothing
					// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum
					m_nCCCardID = -1;

					//set the drop down to be blank
					m_pCardList->CurSel = NULL;
				}
				else if (nResult == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
					// OnTrySetSelFinishedComboCardName will handle it the selection and it will use m_strSwipedCardName to 
					// determine that it was called because a card was swiped and to get the name of the card
				}
				else {
					// Set m_strSwipedCardName to blank because the card swipe has been handled
					m_nCCCardID = -1;
				}

				// Set the expiration date
				// Ensure we do not set it to an invalid date
				COleDateTime dtMin, dt = cciCardInfo->m_dtExpDate;
				dtMin.ParseDateTime("12/31/1899");
				if(dt.m_status != COleDateTime::invalid && dt >= dtMin) {
					GetDlgItem(IDC_CC_AUTH_EXP_DATE)->SetWindowText(dt.Format("%m/%y"));
					m_strCCExpDate = dt.Format("%m/%y");
				}

				// Set the credit card number
				//(e.lally 2007-10-30) PLID 27892 - Only display the last 4 digits. Buffer with X's
				{
					CString strDisplayedCCNumber = MaskCCNumber(cciCardInfo->m_strCardNum);
					GetDlgItem(IDC_CC_AUTH_NUMBER)->SetWindowText(strDisplayedCCNumber);
					m_strCCNumber = cciCardInfo->m_strCardNum;
				}

				// Set the name on the credit card
				// Format the name
				CString strName = "";
				if (cciCardInfo->m_strMiddleInitial == "" || cciCardInfo->m_strMiddleInitial == " ") {
					strName.Format("%s %s", cciCardInfo->m_strFirstName, cciCardInfo->m_strLastName);
				}
				else {
					strName.Format("%s %s %s", cciCardInfo->m_strFirstName, cciCardInfo->m_strMiddleInitial, cciCardInfo->m_strLastName);
				}

				// Add the Title if there is one
				if (cciCardInfo->m_strTitle != "") {
					CString strTemp;
					strTemp.Format("%s %s", cciCardInfo->m_strTitle, strName);
					strName = strTemp;
				}

				// Add the Suffix if there is one
				if (cciCardInfo->m_strSuffix != "") {
					CString strTemp;
					strTemp.Format("%s, %s", strName, cciCardInfo->m_strSuffix);
					strName = strTemp;
				}

				GetDlgItem(IDC_CC_AUTH_NAME)->SetWindowText(FixCapitalization(strName));
				m_strCCName = FixCapitalization(strName);

				//Save the track data and a flag so we get "credit" for it when doing QBMS authorization
				m_bIsSwiped = true;
				m_strTrack2Data = mtiInfo->strTrack2;
				DisableAfterSwipe();

				CheckDlgButton(IDC_CARD_SWIPED, TRUE);
			}
		}
	} NxCatchAll("Error in CCreditCardProcessingDlg::OnMSRDataEvent");

	return 0;
}

void CCreditCardProcessingDlg::OnAddTipBtn() 
{
	try {
		//ensure they can use nexspa
		if(!IsSpa(TRUE)) {
			MsgBox("You must purchase the NexSpa module to use this feature.");
			return;
		}

		//ensure they have permission
		if(!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false))
			return;

		//we want to set the method to whatever they have currently chosen to pay with
		//TES 4/15/2015 - PLID 65177 - If this is a refund, use the Charge Refund paymethod
		long nMethod = (m_nType == 3?9:3);

		//Require a provider for the tip
		long nProvID = m_nProviderID;
		if(nProvID == -1) {
			AfxThrowNxException("A valid provider must exist to create a tip.");
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTipList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, (long)nProvID);
		pRow->PutValue(2, _variant_t(COleCurrency(0, 0)));
		pRow->PutValue(3, (long)nMethod);

		m_pTipList->AddRowAtEnd(pRow, NULL);

		//and we want to start editing the amount field
		m_pTipList->StartEditing(pRow, 2);
	} NxCatchAll(__FUNCTION__);
}

void CCreditCardProcessingDlg::OnRequeryFinishedCardTypeList(short nFlags) 
{
	try {
		//set the currently selected card type
		m_pCardList->SetSelByColumn(clcID, (long)m_nCCCardID);
	} NxCatchAll(__FUNCTION__);
}

void CCreditCardProcessingDlg::UpdateTotalTipAmt()
{
	COleCurrency cy(0, 0);

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTipList->GetFirstRow();

		while (pRow) {
			COleCurrency cyAmt = VarCurrency(pRow->GetValue(tlcAmount));
			cy += cyAmt;
			pRow = pRow->GetNextRow();
		}

		//now update the box
		SetDlgItemText(IDC_TOTAL_TIP_AMT, FormatCurrencyForInterface(cy));

	} NxCatchAll(__FUNCTION__);
}

bool CCreditCardProcessingDlg::GetTotalTransactionAmount(OUT COleCurrency& cyAmount)
{
	CString strTransAmount;
	GetDlgItemText(IDC_TOTAL_TRANS_AMT, strTransAmount);
	cyAmount.ParseCurrency(strTransAmount);
	if(cyAmount.GetStatus() != COleCurrency::valid) {
		AfxMessageBox("You have entered an invalid total amount.  Please correct this and try again.");
		return false;
	}
	return true;
}

//If there is swipe data available, we pass that data (and not the cc number / exp date info) to QBMS.  Therefore, 
//	we should not allow the user to edit these fields, because no matter what they type, that information would not
//	transfer to QBMS.
void CCreditCardProcessingDlg::DisableAfterSwipe()
{
	GetDlgItem(IDC_CC_AUTH_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_CARD_TYPE_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_CC_AUTH_EXP_DATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_CARD_IS_PRESENT)->EnableWindow(FALSE);
}

void CCreditCardProcessingDlg::SelChosenAccountsList(LPDISPATCH lpRow)
{
	try {
		CString strWindowText = "Credit Card Processing - Please select an account";

		if(lpRow == NULL) {
			//No selection, set it back to default text.
		}
		else {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			//Assume production, then change appropriately if not
			strWindowText = "Credit Card Processing";

			// (d.thompson 2010-11-08) - PLID 41367 - This was previously done upon entry.  We now want to do it every time they make a selection from the list.
			if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
				CQBMSProcessingAccount acct = QBMSProcessingUtils::GetAccountByID(VarLong(pRow->GetValue(eacID)));

				if(!acct.GetIsProduction()) {
					//For testing purposes, make sure the user is aware that we're not submitting live transactions
					strWindowText = "Credit Card Processing -- PREPRODUCTION";
				}
				else {
					//Production account -- use the default window text
				}
			}
			else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent) && !VarBool(pRow->GetValue(eacIsProduction))) {
				// (d.thompson 2010-11-10) - PLID 40351 - Make sure it's aware of the preproduction status
				strWindowText = "Credit Card Processing -- PREPRODUCTION";
			}
			else {
				//No licensing, this is handled elsewhere in the dialog.
			}
		}

		//We now know, so update the title bar
		SetWindowText(strWindowText);

	} NxCatchAll(__FUNCTION__);
}

#pragma endregion

// (b.spivey, October 04, 2011) - PLID 40567 - Launch the dialog, and then pass the parameters to the OnMSRDataEvent like a 
//		normal card swipe. 
void CCreditCardProcessingDlg::OnBnClickedKeyboardCardSwipe()
{
	// (b.spivey, November 18, 2011) - PLID 40567 - Added try/catch. 
	try {
		CKeyboardCardSwipeDlg dlg(this);
		if(dlg.DoModal() == IDOK){
			OnMSRDataEvent((WPARAM)&dlg.mtiKeyboardSwipe, (LPARAM)&dlg.cciKeyboardSwipe); 
			
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2013-06-06) - PLID 56334 - We have successfully processed the card and are ready to close up shop
void CCreditCardProcessingDlg::CloseAfterSuccess()
{
	//Do any saving of options/remember/etc.  Remember - this only happens if the processing succeeded (not if 
	//	they canceled or chose to save & process later)
	SaveLastUsedAccount();

	//Close the dialog
	CDialog::OnOK();
}

// (d.thompson 2013-06-06) - PLID 56334 - Save the account id that is last used
void CCreditCardProcessingDlg::SaveLastUsedAccount()
{
	//Note:  There is a preference to recall the last used account.  We're always going to save the value, 
	//	regardless of that preference.  It just controls whether we load the selection or not.

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetCurSel();
	if(pRow != NULL) {
		//Note:  We're just tracking the account ID.  If for some reason they swapped from chase to inuit or vice versa, 
		//	it could remember a random account.  This is allowed (I'm not aware this has ever happened).
		long nAcctID = pRow->GetValue(eacID);
		SetPropertyInt("CCreditCardProcessingDlg_RememberedAccountID", nAcctID, 0);
	}
	else {
		//Somehow we succeeded without any account being selected?  This should be impossible, but if it happens, 
		//	we just won't remember - no big deal.
		ASSERT(FALSE);
	}
}
