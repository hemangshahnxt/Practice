// ProcessCreditCardDlg.cpp : implementation file
//

// (d.thompson 2009-04-10) - PLID 33957 - Renamed to CPAYMENTECH_ProcessCreditCardDlg for clarity (we now have other types)

#include "stdafx.h"
#include "billingrc.h"
#include "ProcessCreditCardDlg.h"
#include "InternationalUtils.h"
#include "PaymentechUtils.h"
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
#include "PaymentDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum TipListColumns {
	
	tlcID = 0,
	tlcProv,
	tlcAmount,
	tlcPayMethod,
};

enum CardListColumns {
	clcID = 0,
	clcName,
	clcType,
};
	
// (j.gruber 2007-08-03 16:19) - PLID 26620 - CC Processing
// (e.lally 2007-12-11) PLID 28325 - Updated "Privatize" cc number function to "Mask"

/////////////////////////////////////////////////////////////////////////////
// CPAYMENTECH_ProcessCreditCardDlg dialog


CPAYMENTECH_ProcessCreditCardDlg::CPAYMENTECH_ProcessCreditCardDlg(long nPaymentID, CString strCCNumber, CString strCCName, 
	  CString strCCExpDate, long nCCCardID, COleCurrency cyPayAmount, long nType, long nPatientID,
	  long nProviderID, BOOL bSwiped, SwipeType swType, CString strTrack1, CString strTrack2, CString strTrack3,
	  long nDrawerID, long nLocationID,
	  CWnd* pParent, CPaymentSaveInfo* pInfo /*=NULL*/) // (a.walling 2007-09-21 15:57) - PLID 27468 - Added the payment info struct
	: CNxDialog(CPAYMENTECH_ProcessCreditCardDlg::IDD, pParent)
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
	m_bSwiped = bSwiped;
	m_swtType = swType;
	m_strTrack1 = strTrack1;
	m_strTrack2 = strTrack2;
	m_strTrack3 = strTrack3;
	m_nDrawerID = nDrawerID;
	m_nLocationID = nLocationID;
	m_bIsDeleted = FALSE;
	m_pInfo = pInfo; // (a.walling 2007-09-21 17:09) - PLID 27468

	// (a.walling 2008-07-07 17:16) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	m_strPatientName = GetExistingPatientName(m_nPatientID);
	//{{AFX_DATA_INIT(CPAYMENTECH_ProcessCreditCardDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPAYMENTECH_ProcessCreditCardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPAYMENTECH_ProcessCreditCardDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_TOTAL_TRANS_AMT, m_nxeditTotalTransAmt);
	DDX_Control(pDX, IDC_CC_AUTH_NUMBER, m_nxeditCcAuthNumber);
	DDX_Control(pDX, IDC_CC_AUTH_NAME, m_nxeditCcAuthName);
	DDX_Control(pDX, IDC_CC_AUTH_EXP_DATE, m_nxeditCcAuthExpDate);
	DDX_Control(pDX, IDC_TOTAL_TIP_AMT, m_nxeditTotalTipAmt);
	DDX_Control(pDX, IDC_AUTHORIZATION_NUMBER, m_nxeditAuthorizationNumber);
	DDX_Control(pDX, IDC_MESSAGE_LABEL, m_nxstaticMessageLabel);
	DDX_Control(pDX, IDC_AUTH_NUMBER_LABEL, m_nxstaticAuthNumberLabel);
	DDX_Control(pDX, IDC_PROCESS_CC_CARD, m_btnProcess);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_TIP_BTN, m_btnAddTip);
	DDX_Control(pDX, IDC_AUTHORIZE_MANUALLY, m_btnAuthManually);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CPAYMENTECH_ProcessCreditCardDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPAYMENTECH_ProcessCreditCardDlg)
	ON_BN_CLICKED(IDC_PROCESS_CC_CARD, OnProcessCcCard)
	ON_BN_CLICKED(IDC_ADD_TIP_BTN, OnAddTipBtn)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_AUTHORIZE_MANUALLY, OnAuthorizeManually)
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_EN_UPDATE(IDC_CC_AUTH_EXP_DATE, OnUpdateCcAuthExpDate)
	ON_EN_KILLFOCUS(IDC_CC_AUTH_NUMBER, OnKillfocusCCNumber)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPAYMENTECH_ProcessCreditCardDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CPAYMENTECH_ProcessCreditCardDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPAYMENTECH_ProcessCreditCardDlg)
	ON_EVENT(CPAYMENTECH_ProcessCreditCardDlg, IDC_CARD_TYPE_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCardTypeList, VTS_I4 VTS_I4)
	ON_EVENT(CPAYMENTECH_ProcessCreditCardDlg, IDC_CARD_TYPE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCardTypeList, VTS_I2)
	ON_EVENT(CPAYMENTECH_ProcessCreditCardDlg, IDC_CC_AUTH_TIP_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCcAuthTipList, VTS_I2)
	ON_EVENT(CPAYMENTECH_ProcessCreditCardDlg, IDC_CC_AUTH_TIP_LIST, 6 /* RButtonDown */, OnRButtonDownCcAuthTipList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPAYMENTECH_ProcessCreditCardDlg, IDC_CC_AUTH_TIP_LIST, 10 /* EditingFinished */, OnEditingFinishedCcAuthTipList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CPAYMENTECH_ProcessCreditCardDlg::OnInitDialog() 
{
	try { 
		CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call
		
		// (c.haag 2008-04-23 16:05) - PLID 29761 - NxIconified buttons
		m_btnProcess.AutoSet(NXB_OK);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddTip.AutoSet(NXB_NEW);

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pTipList = BindNxDataList2Ctrl(IDC_CC_AUTH_TIP_LIST, false);
		m_pCardList = BindNxDataList2Ctrl(IDC_CARD_TYPE_LIST, true);

		CString strWhere;
		strWhere.Format("PaymentTipsT.PaymentID = %li", m_nPaymentID);
		m_pTipList->WhereClause = _bstr_t(strWhere);
		m_pTipList->Requery();

		//load the information
		//(e.lally 2007-10-30) PLID 27892 - Only display last 4 digits. Buffer with X's
		{
			CString strDisplayedCCNumber = MaskCCNumber(m_strCCNumber);
			SetDlgItemText(IDC_CC_AUTH_NUMBER, strDisplayedCCNumber);
		}
		SetDlgItemText(IDC_CC_AUTH_NAME, m_strCCName);
		SetDlgItemText(IDC_CC_AUTH_EXP_DATE, m_strCCExpDate);

		m_brush.CreateSolidBrush(PaletteColor(0x9CC294));

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

		//gray out the manual authorization
		GetDlgItem(IDC_AUTHORIZE_MANUALLY)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUTH_NUMBER_LABEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUTHORIZATION_NUMBER)->EnableWindow(FALSE);

		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PROCESS_CC_CARD)->ShowWindow(SW_SHOW);

		//paymentech limit
		((CNxEdit*)GetDlgItem(IDC_CC_AUTH_NUMBER))->LimitText(19);
		//database limit
		((CNxEdit*)GetDlgItem(IDC_CC_AUTH_NAME))->LimitText(50);
		//set to 5 to incorporate the month and the date with a slash, no more
		///((CNxEdit*)GetDlgItem(IDC_CC_AUTH_EXP_DATE))->LimitText(5);
		
		
	}NxCatchAll("Error in CPAYMENTECH_ProcessCreditCardDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPAYMENTECH_ProcessCreditCardDlg::SaveChanges() {

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
		COleDateTime dt;
		//set the date to be the last date of the exp. month
		CString strMonth = strExpDate.Left(strExpDate.Find("/",0));
		CString strYear = "20" + strExpDate.Right(strExpDate.Find("/",0));
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
		strExpDate = FormatDateTimeForSql(dt, dtoDate);
		// (j.jones 2007-03-13 14:45) - PLID 25183 - ensured we won't save an invalid date
		COleDateTime dtMin;
		dtMin.ParseDateTime("12/31/1899");
		if(dt.m_status == COleDateTime::invalid || dt < dtMin) {
			strExpDate = "Null";
		}
		else {
			strExpDate = CString("'") + _Q(strExpDate) + "'";
		}
		
		CString strSqlBatch = BeginSqlBatch();
		
		//(e.lally 2007-10-30) PLID 27892 - Use our member variable to save the card number.
		// (a.walling 2007-10-30 17:55) - PLID 27891 - Save the truncated CCNumber and the encrypted SecurePAN
		// (a.walling 2010-03-15 12:35) - PLID 37751 - Use NxCrypto
		CString strEncryptedCCNumber, strKeyIndex;
		NxCryptosaur.EncryptStringForSql(m_strCCNumber, strKeyIndex, strEncryptedCCNumber);
		AddStatementToSqlBatch(strSqlBatch, "UPDATE PaymentPlansT SET CreditCardID = %li, CCNumber = '%s', "
			" CCHoldersName = '%s', CCExpDate = %s, SecurePAN = %s, KeyIndex = %s WHERE PaymentPlansT.ID = %li",
			nCardType, _Q(m_strCCNumber.Right(4)), _Q(strName), strExpDate, strEncryptedCCNumber, strKeyIndex, m_nPaymentID);//incase the data is funky

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
		if(m_nType == 1) {	//refunds don't get tipped
				//loop through all tips

			//drawer setup
			CString strDrawer;
			if (m_nDrawerID == -1 ) {
				strDrawer = "NULL";
			}
			else {
				strDrawer.Format("%li", m_nDrawerID);
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
					AddStatementToSqlBatch(strSqlBatch, "SET @PaymentTipID = (SELECT (COALESCE(MAX(ID),0) + 1) FROM PaymentTipsT) \r\n"
						"INSERT INTO PaymentTipsT (ID, PaymentID, ProvID, Amount, PayMethod, DrawerID) "
						"values (@PaymentTipID, %li, %li, convert(money, '%s'), %li, %s);\r\n", 
						m_nPaymentID, nProvID, FormatCurrencyForSql(cyAmt), nPayMethod, strDrawer);

					//for auditing
					//(e.lally 2007-03-21) PLID 25258 - Switch to an audit transaction
					str.Format("Tip added for %s.", FormatCurrencyForInterface(cyAmt));
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					// (a.walling 2008-07-07 17:16) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
					AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipCreated, m_nPaymentID, "", str, aepMedium, aetCreated);
				}
				else {
					ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ProvID, Amount, PayMethod, DrawerID FROM PaymentTipsT WHERE ID = %li", nID);
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
							ADODB::_RecordsetPtr prsAudit = CreateRecordset("SELECT (SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = %li) AS OldProvider, "
								"(SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li) AS NewProvider", nOldProv, nProvID);
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
							AuditEvent(m_nPatientID, m_strPatientName, nAuditTransactionID, aeiTipMethod, m_nPaymentID, nOldMethod == 1 ? "Cash" : nOldMethod == 2 ? "Check" : "Charge", nPayMethod == 1 ? "Cash" : nPayMethod == 2 ? "Check" : "Charge", aepMedium, aetChanged);
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
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PaymentTipsT WHERE ID = %li;\r\n", nTipID);

					//for auditing
					ADODB::_RecordsetPtr prs = CreateRecordset("SELECT Amount FROM PaymentTipsT WHERE ID = %li", nTipID);
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
			ExecuteSqlBatch(strSqlBatch);			
		}

		//save the audit trans
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		return TRUE;
	}NxCatchAllCall("Error in CPAYMENTECH_ProcessCreditCardDlg::SaveChanges", 
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		return FALSE;);
}

BOOL CPAYMENTECH_ProcessCreditCardDlg::CheckFields() {
	
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

		//check the all the information is entered
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
		if (pRow) {
		}
		else {
			MsgBox("Please enter a card type.");
			return FALSE;
		}

		return TRUE;
	}NxCatchAll("Error in CheckFields");
	return FALSE;
}

void CPAYMENTECH_ProcessCreditCardDlg::OnProcessCcCard() 
{

	try { 
		if (!CheckFields()) {
			return;
		}

		//first, save any changes to the payment
		if (SaveChanges()) {
			
			if (ProcessTransaction()) {

				if (m_bIsDeleted) {

					//we deleted the payment, so close with a cancel
					CDialog::OnCancel();
				}
				else {
					//we succeded, we can close
					CDialog::OnOK();
				}
			}
			else {

				//it failed, we already popped up the message box, but we can't close yet
			}
		}
	}NxCatchAll("Error in CPAYMENTECH_ProcessCreditCardDlg::OnProcessCcCard() ");
	
	
}

void CPAYMENTECH_ProcessCreditCardDlg::OnCancel() 
{
	try {
		//Warn them that this will delete the payment
		CString strMessage;
		strMessage.Format("This payment has not been processed yet.  By cancelling you will be deleting the payment.\n"
			"Are you sure you want to do this?");

		if (IDYES == MsgBox(MB_YESNO, strMessage)) {

			//we have to delete the payment
			DeletePayment(m_nPaymentID, TRUE);
			CDialog::OnCancel();
		}
		else {
			//we are leaving it open because they don't want to close it
		}
	}NxCatchAll("Error in CPAYMENTECH_ProcessCreditCardDlg::OnCancel() ");
	
	
}

void CPAYMENTECH_ProcessCreditCardDlg::OnAddTipBtn() 
{
	try {
		//ensure they can use nex spa
		if(!IsSpa(TRUE)) {
			MsgBox("You must purchase the NexSpa module to use this feature.");
			return;
		}

		//ensure they have permission
		if(!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false))
			return;

		try {
			//we want to set the method to whatever they have currently chosen to pay with
			
			//we are going to default to charge
			long nMethod = 3;		
			
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTipList->GetNewRow();

			long nProvID = m_nProviderID;
			
			if(nProvID == -1) {
				//still have no provider!  maybe they unselected the combo there.  Regardless, we need *something* chosen
				//here, so just pick the first provider we can find.
				ADODB::_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 ID FROM ProvidersT INNER JOIN PersonT ON PersonID = ID WHERE Archived = 0");
				if(prs->eof) {
					//they don't have any providers
					MsgBox("You have no active providers in your database.  You cannot create a tip.");
					return;
				}
				else {
					//ok, stick this one in
					nProvID = AdoFldLong(prs, "ID");
				}
			}

			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, (long)nProvID);
			pRow->PutValue(2, _variant_t(COleCurrency(0, 0)));
			pRow->PutValue(3, (long)nMethod);

			m_pTipList->AddRowAtEnd(pRow, NULL);

			//and we want to start editing the amount field
			m_pTipList->StartEditing(pRow, 2);
		} NxCatchAll("Error in AddTipBtn()");
	}NxCatchAll("Error in CPAYMENTECH_ProcessCreditCardDlg::OnAddTipBtn() ");
	
}

void CPAYMENTECH_ProcessCreditCardDlg::OnTrySetSelFinishedCardTypeList(long nRowEnum, long nFlags) 
{
	try { 
		if (nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			
			// We only want to handle the case were the card type could not be selected after swiping
			if (GetMainFrame()->DoesOPOSMSRDeviceExist()) {
				MsgBox("The card type could not be found in the list, please add the card type to the Card Type list before trying to process it.");
				m_nCCCardID = -1;
			}
		}
	}NxCatchAll("Error in CPAYMENTECH_ProcessCreditCardDlg::OnTrySetSelFinishedCardTypeList");
	
}


BOOL CPAYMENTECH_ProcessCreditCardDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
	case ID_PAYMENTTIPS_ADD:	//popup menu - Add new tip
		OnAddTipBtn();
		return TRUE;

	case ID_PAYMENTTIPS_DELETE:	//popup menu - Delete current tip
		OnDeleteTip();
		return TRUE;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CPAYMENTECH_ProcessCreditCardDlg::OnDeleteTip()
{
	try {
		//ensure they have permission
		if(!CheckCurrentUserPermissions(bioPaymentTips, sptWrite, false))
			return;

		//delete the currently selected tip
	
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTipList->GetCurSel();
		if(pRow == NULL) {
			//this shouldn't be possible
			ASSERT(FALSE);
			return;
		}

		if(MsgBox(MB_YESNO, "Are you sure you wish to delete this tip?") != IDYES)
			return;

		//we don't really delete it, we just remove the row and set the ID in an array to delete later
		long nID = VarLong(pRow->GetValue(tlcID));

		//flag it in our delete list
		m_aryDeleted.Add(nID);

		//remove the row
		m_pTipList->RemoveRow(pRow);

		if (VarLong(pRow->GetValue(tlcPayMethod)) == 3) {
			//subtract from the total amount
			UpdateTotalTransAmt();
		}

		// (a.walling 2007-09-21 17:10) - PLID 27468 - We also need to update the payment info if we have it
		if ( (nID != -1) && (m_pInfo != NULL) ) {
			// unfortunately we have to query data since we don't have any guarantee the paytype was not 
			// changed before it was deleted.

			ADODB::_RecordsetPtr prs = CreateRecordset("SELECT PayMethod FROM PaymentTipsT WHERE ID = %li", nID);
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

		

	} NxCatchAll("Error in OnDeleteTip()");
}


void CPAYMENTECH_ProcessCreditCardDlg::UpdateTotalTransAmt()
{
	COleCurrency cy(0, 0);

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTipList->GetFirstRow();

		while (pRow) {
			if (VarLong(pRow->GetValue(tlcPayMethod)) == 3) {
				COleCurrency cyAmt = VarCurrency(pRow->GetValue(tlcAmount));

				cy += cyAmt;
			}
			pRow = pRow->GetNextRow();
		}

		//add the payment amount
		cy += m_cyPayAmount;

		//now update the box
		SetDlgItemText(IDC_TOTAL_TRANS_AMT, FormatCurrencyForInterface(cy));

	} NxCatchAll("Error in UpdateTotalTransAmt");
}

//Calculate and update the total amount of tips in the edit box provided
void CPAYMENTECH_ProcessCreditCardDlg::UpdateTotalTipAmt()
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

	} NxCatchAll("Error in UpdateTotalTipAmt");
}



HBRUSH CPAYMENTECH_ProcessCreditCardDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-05-12 13:30) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd->GetDlgCtrlID() != IDC_TOTAL_TRANS_AMT &&
		pWnd->GetDlgCtrlID() != IDC_TOTAL_TIP_AMT &&
		pWnd->GetDlgCtrlID() != IDC_AUTHORIZATION_NUMBER) {

		if (nCtlColor == CTLCOLOR_STATIC) {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x9CC294));
			return m_brush;
		}
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
	*/
}

void CPAYMENTECH_ProcessCreditCardDlg::OnRequeryFinishedCardTypeList(short nFlags) 
{
	try {
		//set the currently selected card type
		m_pCardList->SetSelByColumn(0, (long)m_nCCCardID);
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::OnRequeryFinishedCardTypeList");
	
}

BOOL CPAYMENTECH_ProcessCreditCardDlg::DoAddressVerification() {

	try { 
		//see if there is an account number
		CString strTemp;
		//(e.lally 2007-10-30) PLID 27892 - We are only checking for emptiness, so we can
		//let it use the displayed card number.
		GetDlgItemText(IDC_CC_AUTH_NUMBER, strTemp);

		if (strTemp.IsEmpty()) {
			MessageBox("Please enter the credit card number");
			return FALSE;
		}

		//check the expire data also
		GetDlgItemText(IDC_CC_AUTH_EXP_DATE, strTemp);

		if (strTemp.IsEmpty()) {
			MessageBox("Please enter the expiration date");
			return FALSE;
		}

		m_tsTransaction.edsEntryDataSource = PaymentechUtils::edsManuallyEntered;

		//we need to check the the billing information including zip code
		CString strCardholderName;
		GetDlgItemText(IDC_CC_AUTH_NAME, strCardholderName);
		CVerifyCardholderInfoDlg dlg(strCardholderName, m_nPatientID, this);

		if (dlg.DoModal() == IDOK) {

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pCardList->CurSel;

			if (pRow) {

				//they confirmed it, add it to the transaction
				m_tsTransaction.strCardHolderStreet = dlg.m_strCardholderAddress1;
				m_strCardHolderAddress = m_tsTransaction.strCardHolderStreet;
				if (VarLong(pRow->GetValue(clcType)) == pctVisa) {
					m_tsTransaction.strExtendedStreetInfo = dlg.m_strCardholderAddress2;
					m_strCardHolderAddress2 = m_tsTransaction.strExtendedStreetInfo;
				}
			}
			m_tsTransaction.strCardHolderZip = dlg.m_strCardholderZip;
			m_strCardHolderZip = m_tsTransaction.strCardHolderZip;

			m_tsTransaction.strCVD  = dlg.m_strCardholderSecurityCode;		

			if (m_tsTransaction.strCVD.IsEmpty()) {
				
				//check to see if they checked any of the checkboxes
				if (dlg.m_bCodeIllegible) {
					m_tsTransaction.piPresenceIndicator = PaymentechUtils::piIllegible;
				}
				else if (dlg.m_bCodeNotProvidedByCustomer) {
					m_tsTransaction.piPresenceIndicator = PaymentechUtils::piUnavailable;
				}
				else {

					//they just didn't get it
					m_tsTransaction.piPresenceIndicator = PaymentechUtils::piBypassed;
				}
			}
			else {
				m_tsTransaction.piPresenceIndicator = PaymentechUtils::piProvided;
			}					

		}
		else {

			if (IDYES == (MsgBox(MB_YESNO, "You chose to cancel the address verification, do you still wish to authorize this payment?  Note: having a manually entered transaction without verifying at least the zip code of the card holder may result in higher interchange rates."))) {
				 //they want to proceed

				//set the presence indicator
				m_tsTransaction.piPresenceIndicator = PaymentechUtils::piBypassed;
			}
			else {
				// they want to cancel
				return FALSE;
			}
		}

		return TRUE;
	}NxCatchAll("Error in DoAddressVerification");

	return FALSE;

}

// (j.gruber 2007-08-03 10:05) - PLID 26926 - added for auditing
CString CPAYMENTECH_ProcessCreditCardDlg::GetDescFromTransactionType(PaymentechUtils::TransactionCode tcCode) {

	switch (tcCode) {

		case PaymentechUtils::tcSale:
			return "Sale";
		break;

		case PaymentechUtils::tcReturn:
			return "Refund";
		break;

		case PaymentechUtils::tcVOID:
			return "Void";
		break;

		default:
			return "";
		break;
	}
}




void CPAYMENTECH_ProcessCreditCardDlg::SaveTransaction(PaymentechUtils::ResponseStruct *ptsResponse, long nIsProcessed, CString strExtraAuditMessage) {
	
	try {

		//we only save if the transaction was approved or falied for communication reasons, we don't save declines

		//save the transaction
		//see if we already have this batch
		ADODB::_RecordsetPtr rsBatchID = CreateRecordset(" SELECT ID FROM TransactionBatchT WHERE BatchNumber = '%s' AND BatchReleased = 0 ", ptsResponse->strBatchNumber);
		long nBatchID;

		if (! rsBatchID->eof) {
			
			//we found an open batch, so we don't need to insert this one
			nBatchID = AdoFldLong(rsBatchID, "ID");

		}
		else {

			//this is a new batch, we need to insert into the batch table

			//we can only enter the batch number at the moment because we don't know anything else
			//look for the batch number that we got back in the transaction
			ADODB::_RecordsetPtr rsBatch = CreateRecordset(" SET NOCOUNT OFF; \r\n"
					" DECLARE @Batch_ID INT; \r\n"
					" \r\n"
					" INSERT INTO TransactionBatchT (BatchNumber) VALUES ('%s') "
					" SET NOCOUNT OFF; \r\n "
					" SET @Batch_ID = @@IDENTITY; \r\n"
					" \r\n "
					" SELECT @Batch_ID as NewID; \r\n ", ptsResponse->strBatchNumber	);

			_variant_t v;
			rsBatch = rsBatch->NextRecordset(&v);
			nBatchID = AdoFldLong(rsBatch, "NewID");
		}

		if (nIsProcessed == 1) {
			//now that we have our batchID, we can insert our transaction
			ExecuteSql("INSERT INTO CreditTransactionsT (ID, TransactionCode, PinCapabilityCode, EntryDataSource, "
				" TransactionAmount, CardHolderStreetAddress, CardHolderStreetAddress2, "
				" CardHolderZip, AddVerificationResp, "
				" BatchID, RetrievalRefNum, RespMessage, "
				" IsProcessed, IsApproved, TerminalID) "
				" VALUES (%li, %li, %li, %li, "
				" Convert(money, '%s'), '%s', '%s', "
				" '%s', '%s', "
				" %li, %li, '%s', %li, %li, '%s') ",
				m_nPaymentID, m_tsTransaction.tcTransactionCode, m_tsTransaction.nPINCapabilityMode, m_tsTransaction.edsEntryDataSource,
				FormatCurrencyForSql(m_tsTransaction.cyTransAmount), _Q(m_tsTransaction.strCardHolderStreet.Left(20)), _Q(m_tsTransaction.strExtendedStreetInfo.Left(20)),
				_Q(m_tsTransaction.strCardHolderZip), _Q(CString(ptsResponse->chAddressVerificationCode)), 
				nBatchID, ptsResponse->nRetreivalReferenceNumber, _Q(ptsResponse->strResponseMessage), nIsProcessed, ptsResponse->bApproved, GetPropertyText("CCProcessingTerminalNumber", "", 0, TRUE));

			// (j.gruber 2007-08-03 10:06) - PLID 26926 - Auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1) {
				CString strDesc;
				CString strProcessDesc;
				if (m_bSwiped) {
					strProcessDesc = "Card Swiped";
				}
				else {
					strProcessDesc = "Manually Entered";
				}

				CString strResultDesc;
				if (ptsResponse->bApproved) {
					strResultDesc = "Approved";
				}
				else {
					strResultDesc = "Declined: " + ptsResponse->strResponseMessage;
				}


				strDesc.Format("%s Transaction: %s - %s - Result: %s", GetDescFromTransactionType(m_tsTransaction.tcTransactionCode),
					FormatCurrencyForInterface(m_tsTransaction.cyTransAmount), 
					strProcessDesc, strResultDesc);
				// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
				AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiProcessTransaction, m_nPaymentID, "", strDesc, 1, aetCreated);
			}

		}
		else {

			ExecuteSql("INSERT INTO CreditTransactionsT (ID, TransactionCode, PinCapabilityCode, EntryDataSource, "
				" TransactionAmount, CardHolderStreetAddress, CardHolderStreetAddress2, "
				" CardHolderZip, AddVerificationResp, "
				" BatchID, RetrievalRefNum, RespMessage, "
				" IsProcessed, IsApproved, TerminalID) "
				" VALUES (%li, %li, %li, %li, "
				" Convert(money, '%s'), '%s', '%s', "
				" '%s', '%s', "
				" %li, NULL, '%s', %li, %li, '%s') ",
				m_nPaymentID, m_tsTransaction.tcTransactionCode, m_tsTransaction.nPINCapabilityMode, m_tsTransaction.edsEntryDataSource,
				FormatCurrencyForSql(m_tsTransaction.cyTransAmount), _Q(m_tsTransaction.strCardHolderStreet.Left(20)), _Q(m_tsTransaction.strExtendedStreetInfo.Left(20)),
				_Q(m_tsTransaction.strCardHolderZip), "", 
				nBatchID, "", 0, 0, GetPropertyText("CCProcessingTerminalNumber", "", 0, TRUE));

			// (j.gruber 2007-08-03 10:06) - PLID 26926 - auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1) {
				CString strDesc;
				CString strProcessDesc;
				if (m_bSwiped) {
					strProcessDesc = "Card Swiped";
				}
				else {
					strProcessDesc = "Manually Entered";
				}

				//they haven't really processed it yet
				CString strResultDesc;
				strResultDesc = "Pending Approval - " + strExtraAuditMessage;
				

				strDesc.Format("%s Transaction: %s - %s - Result: %s", GetDescFromTransactionType(m_tsTransaction.tcTransactionCode),
					FormatCurrencyForInterface(m_tsTransaction.cyTransAmount), 
					strProcessDesc, strResultDesc);
				// (a.walling 2008-07-07 17:17) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
				AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiProcessTransaction, m_nPaymentID, "", strDesc, 1, aetCreated);
			}
		}

			



	}NxCatchAll("Error in CPAYMENTECH_ProcessCreditCardDlg::SaveTransaction()");

}



BOOL CPAYMENTECH_ProcessCreditCardDlg::ProcessTransaction() {

	try {
		//first, we need to setup the information from our transaction
		if (!PaymentechUtils::IsPaymentechInstalled()) {
			MsgBox("Paymentech is not installed properly, please check the installation and try processing again.");
			return FALSE;
		}

		//clear the transaction first
		PaymentechUtils::ClearMessage(m_tsTransaction);
		
		//is this a payment or a refund?
		BOOL bIsPayment = FALSE;
		if (m_nType == 1) {
			bIsPayment = TRUE;
		}	
		else if (m_nType == 3) {
			bIsPayment = FALSE;
		}
		else {
			//we shouldn't have credit adjustments
			ASSERT(FALSE);
		}

		m_tsTransaction.bSwiped = m_bSwiped;
		
		//TODO j.gruber - fix this once the credit card types are redone
		// (e.lally 2007-07-09) PLID 25993 - Changed the column number to use the new enum for now
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCardList->CurSel;
		if (pRow == NULL) {
			ASSERT(FALSE);
			return FALSE;
		}

		if (VarLong(pRow->GetValue(clcType)) == pctInterac) {

			//we are doing Canadian Debit

			//is this a payment or a refund
			if (bIsPayment) {
				m_tsTransaction.tcTransactionCode = PaymentechUtils::tcInteracSale;
				m_tsTransaction.iTransID = PaymentechUtils::itiPurchase;
			}
			else {
				m_tsTransaction.tcTransactionCode = PaymentechUtils::tcInteracReturn;
				m_tsTransaction.iTransID = PaymentechUtils::itiRefund;
			}

			m_tsTransaction.ctCardType = PaymentechUtils::ctInterac;
					
			

		}
		else {

			//we are doing US or Canada Credit
			//TODO j.gruber - add US debit when we know what is happening with it.
			m_tsTransaction.ctCardType = PaymentechUtils::ctCredit;

			//for this we do an authorize first and then a sale
			if (bIsPayment) {
				m_tsTransaction.tcTransactionCode = PaymentechUtils::tcSale;
			}
			else {
				m_tsTransaction.tcTransactionCode = PaymentechUtils::tcReturn;
			}
		}

		//get the track information
		if (m_tsTransaction.bSwiped) {

			m_tsTransaction.piPresenceIndicator = PaymentechUtils::piBypassed;

			switch (m_swtType) {

				case swtMSRDevice:
					{
					CString strTrack1, strTrack2, strTrack3;
					// (a.wetta 2007-07-05 10:25) - You can't directly get the track info anymore. Get the info in the OnMSRDataEvent function
					//m_pMSRDevice->GetMSRTrackInfo(strTrack1, strTrack2, strTrack3);

					m_tsTransaction.strMagneticStripe = m_strTrack1;
					m_tsTransaction.edsEntryDataSource = PaymentechUtils::edsTrack1;
					}
					
				break;

				case swtPinPad:

					//we have to use Track 2 for Debit
					//if we got here, then they swiped on the pinpad and so we already have the information
					m_tsTransaction.strMagneticStripe = m_strTrack2;
					ASSERT(!m_tsTransaction.strMagneticStripe.IsEmpty());
					m_tsTransaction.edsEntryDataSource = PaymentechUtils::edsTrack2;				
				break;

				default:
					//how did we get here, if they swiped then it had to either be on the MSR or pin pad
					ASSERT(FALSE);				
				break;
			}

		}
		else {

			//nothing has been swiped yet, let's see what they want to do
			if (IDYES == MsgBox(MB_YESNO, "A card has not been swiped, would you like to authorize via manually entered information?")) {

				if (!DoAddressVerification()) {
					return FALSE;
				}			

			}
			else {
				MsgBox("Please swipe a card and choose Process again or choose to authorize via a manually entered card.");
				return FALSE;
			}
		}



		//get the account number 
		//(e.lally 2007-10-30) PLID 27892 - The control only shows the last 4 digits, we need to use our
		//member variable here.
		m_tsTransaction.strAccountNumber = m_strCCNumber;
		//Expiration date
		CString strExpDate;
		GetDlgItemText(IDC_CC_AUTH_EXP_DATE, strExpDate);
		
		//we need to format it correctly into MMYY
		strExpDate.Replace("/", "");
		m_tsTransaction.strExpireDate = strExpDate;
		
		//fill in the amount
		COleCurrency cyAmountReceived;
		COleCurrency cyTips;

		//first get the amount received
		//CString strAmount;
		//GetDlgItemText(IDC_CASH_RECEIVED, strAmount);
		//cyAmountReceived = ParseCurrencyFromInterface(strAmount);

		CString strTransAmount;
		GetDlgItemText(IDC_TOTAL_TRANS_AMT, strTransAmount);
		
		m_tsTransaction.cyTransAmount.ParseCurrency(strTransAmount);

		//Invoice number, will be the paymentID
		m_tsTransaction.nInvoiceNumber = m_nPaymentID;

		m_tsTransaction.nTransactionSequenceFlag = 1;
		m_tsTransaction.nSequenceNumber = 1;

		//firgure this out from the card once we know how
		m_tsTransaction.lcLangCode = PaymentechUtils::lcEnglish;
		

		PaymentechUtils::ResponseStruct tsResponse;
		//ok, now that we have everything filled, submit the transaction

		BOOL bAuthorizeResponse = PaymentechUtils::AuthorizeTransaction(m_tsTransaction, tsResponse);
		


		long nIsProcessed;
		if (bAuthorizeResponse) {
			nIsProcessed = 1;
		}
		else {
			nIsProcessed = 0;
		}

		if (!bAuthorizeResponse) {
			long nResult;
			if (m_nType == 1) {
				// the only reason why this would fail is if there was a bad connection, or something like that
				nResult = MsgBox(MB_YESNOCANCEL, "There was a communications error processing the transaction.  Would you like to save the payment and authorize later?\n"
					" - Click Yes to save the transaction and process it later\n"
					" - Click No to authorize the transaction via voice authorization\n"
					" - Click Cancel to cancel this transaction and delete the payment");
			}
			else {
				//its a refund
				ASSERT(m_nType == 3);
				nResult = MsgBox(MB_OKCANCEL, "There was a communications error processing the transaction.  Would you like to save the refund and authorize later?\n"
					" - Click OK to save the transaction and process it later\n "
					" - Click Cancel to cancel this transaction and delete the refund");
			}
			if (nResult == IDYES || nResult == IDOK) {

				if (m_bSwiped) {
					
					//they have a swiped card, but we have to process manually, so we need them to verify the address now
					MsgBox("This transaction will be processed manually, please verify the client's address information");
					if (!DoAddressVerification() ) {
						//they decided to cancel completely 
						DeletePayment(m_nPaymentID, TRUE);
						m_bIsDeleted = TRUE;
						return TRUE;
					}
				}

				//they are authorizing later
				SaveTransaction(&tsResponse, nIsProcessed, "Transaction Saved For Later Processing");
				return TRUE;
			}
			else if (nResult == IDNO) {

				if (m_bSwiped) {
					
					//they have a swiped card, but we have to process manually, so we need them to verify the address now
					MsgBox("This transaction will be processed manually, please verify the client's address information");
					if (!DoAddressVerification() ) {
						//they decided to cancel completely 
						DeletePayment(m_nPaymentID, TRUE);
						m_bIsDeleted = TRUE;
						return TRUE;
					}
				}


				SaveTransaction(&tsResponse, nIsProcessed, "Voice Authorization");

				//they need to manually authorize by phone
				MsgBox("Please call Chase Paymentech to complete the voice authorization, enter the authorization number and click OK on the process transaction dialog.");
				GetDlgItem(IDC_AUTHORIZE_MANUALLY)->EnableWindow(TRUE);

				//show the ok button
				GetDlgItem(IDC_PROCESS_CC_CARD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDOK)->ShowWindow(SW_SHOW);

				return FALSE;
			}
			else {

				//they want to cancel the entire shootin match
				DeletePayment(m_nPaymentID, TRUE);
				m_bIsDeleted = TRUE;
				return TRUE;
			}
		}

		if (tsResponse.bApproved) {

			SaveTransaction(&tsResponse, nIsProcessed, "");		

			//write the approval code to the data
			ExecuteSql("UPDATE PaymentPlansT SET CCAuthNo = '%s' WHERE ID = %li", tsResponse.strAuthCode, m_nPaymentID);

			//they got approved!!!
			//print the receipt
			if (!PrintReceipt(tsResponse) ){ 

					//we have to output a message saying that we couldn't print the receipt for some reason
					//we can still return true though
					
					MsgBox("The transaction was approved, however the receipt could not be printed.  Please check the printer.");
					return TRUE;
			}
			return TRUE;
			
		}	
		else {

			//they got denied :(
				
			//we are required to print a receipt for this too
			if (!PrintReceipt(tsResponse) ){ 

				//we have to output a message saying that we couldn't print the receipt for some reason
				//we can still return true though
				///TODO: output this message
				MsgBox("The transaction was declined, the receipt could not be printed.  Please check the printer.");
				return TRUE;
				
			}


			CString strMessage;
			strMessage.Format("The transaction was declined:\n%s\n"
				"Would you like to be returned to the to try to correct the information or run a different card?\n"
				" - Click Yes to be returned to the processing dialog\n"
				" - Click No to delete the payment"
				, PaymentechUtils::MessageLookup(tsResponse.strErrorCode, tsResponse.strResponseMessage, tsResponse.lcLangCode));

			long nResult = MsgBox(MB_YESNO, strMessage);

			switch (nResult) {

				case IDNO:
					//we are deleting the payment 
					//TODO make this put in the audit description that it was deleted due to CC processing
					DeletePayment(m_nPaymentID, TRUE);
					m_bIsDeleted = TRUE;
					return TRUE;
				break;

				case IDYES:
					//they want to correct the information
					//requery the tip list to fill in the IDs
					m_pTipList->Requery();
					return FALSE;
				break;

				default:
					ASSERT(FALSE);
					return FALSE;
				break;
			}
					
		}
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::ProcessTransaction");
	return FALSE;

}

CString CPAYMENTECH_ProcessCreditCardDlg::LeftJustify(CString strText, long nLineWidth) {

	//this is really just printing out the line regularly, but I put it in a function in case we 
	//do something with it in the future
	CString strDesc;

	strDesc = strText;

	long nLineLength = nLineWidth - strText.GetLength();
	/*for (int i = 0; i < nLineLength; i++) {
		strDesc += " ";
	}*/
	return strDesc + "\n";

}


CString CPAYMENTECH_ProcessCreditCardDlg::LeftRightJustify(CString strTextLeft, CString strTextRight, long nLineWidth) {

	CString strDesc;

	long nSpaceLength = nLineWidth - strTextLeft.GetLength() - strTextRight.GetLength();
	strDesc = strTextLeft;
	for (int i = 0; i < nSpaceLength - 1; i++) {
		strDesc += " ";
	}

	strDesc += strTextRight;

	return strDesc + "\n";
}

CString CPAYMENTECH_ProcessCreditCardDlg::CenterLine(CString strText, long nLineWidth) {

	CString strDesc;
	
	long nCenterLine = (nLineWidth)/2;
	long nCenterName = strText.GetLength()/2;
	long nDiff = nCenterLine - nCenterName;
	for (int i = 0; i < nDiff; i++) {
		strDesc += " ";
	}
	strDesc += strText;
	/*for (i = 0; i < nDiff; i++) {
		strDesc += " ";
	}*/

	return strDesc + "\n";
}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
BOOL CPAYMENTECH_ProcessCreditCardDlg::PrintReceiptHeader(COPOSPrinterDevice *pPOSPrinter, EApprovalCode appCode) {

	CString strOutput;
	long nLineWidth = pPOSPrinter->GetLineWidth();
	CString strLine;

	//get the location information
	ADODB::_RecordsetPtr rsLocation = CreateRecordset("SELECT Name, Address1, Address2, City, State, Zip, Phone "
		" FROM LocationsT WHERE ID = %li", m_nLocationID);

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

	if (! strAdd2.IsEmpty()) {
		strOutput += CenterLine(strAdd2, nLineWidth);
	}

	CString strCSZ = strCity + ", " + strState + " " + strZip;
	strOutput += CenterLine(strCSZ, nLineWidth);

	strOutput += CenterLine(strPhone, nLineWidth);

	// (a.walling 2011-04-27 10:08) - PLID 43459 - Linefeed fixes. Don't use CR! Use LF or CRLF instead.
	strOutput += "\n\n";

	//let's write out whether they were approved here
	switch(appCode) {
		case IsApproved:
			strOutput += CenterLine("APPROVED", nLineWidth);
		break;

		case IsDeclined:
			strOutput += CenterLine("DECLINED", nLineWidth);
		break;

		case IsManuallyApproved:
			strOutput += CenterLine("MANUALLY APPROVED", nLineWidth);
		break;
	}
	

	//output them in bold, centered
	CString strTmp = char(27);
	strOutput = "\x1b|bC" + strOutput + "\x1b|N";

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}

}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
BOOL CPAYMENTECH_ProcessCreditCardDlg::PrintReceiptMiddle(COPOSPrinterDevice *pPOSPrinter, CString strOutputMessage) {

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

	
	//whether the card was swiped
	if (m_bSwiped) {
		if (m_nType == 1) {
			strOutput += LeftRightJustify("Card Swiped", "Sale", nLineWidth);
		}
		else {
			strOutput += LeftRightJustify("Card Swiped", "Refund", nLineWidth);
		}
	}
	else {
		if (m_nType == 1) {
			strOutput += LeftRightJustify("Card Manually Processed", "Sale", nLineWidth);
		}
		else {
			strOutput += LeftRightJustify("Card Manually Processed", "Refund", nLineWidth);
		}
	}

	//transaction type
	if (m_tsTransaction.tcTransactionCode != PaymentechUtils::tcReturn) {
		//they don't give approval codes for credit refunds
		strOutput += LeftJustify(strOutputMessage, nLineWidth);
	}		
	else {
		strOutput += LeftJustify(strOutputMessage, nLineWidth);
		//strOutput += LeftJustify("Declined" + PaymentechUtils::MessageLookup(tsResponse.strErrorCode, tsResponse.strResponseMessage, tsResponse.lcLangCode), nLineWidth);
	}

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}


}

COleCurrency CPAYMENTECH_ProcessCreditCardDlg::GetChargeTipTotal() {

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTipList->GetFirstRow();

		COleCurrency cy;

		while (pRow) {

			if (VarLong(pRow->GetValue(tlcPayMethod)) == 3) {
				COleCurrency cyAmt = VarCurrency(pRow->GetValue(tlcAmount));

				cy += cyAmt;
			}

			pRow = pRow->GetNextRow();
			
		}

		return cy;
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::GetChargeTipTotal");
	return COleCurrency(0,0);

}

//TES 12/6/2007 - PLID 28192 - This now takes the POS Printer as a parameter.
BOOL CPAYMENTECH_ProcessCreditCardDlg::PrintReceiptFooter(COPOSPrinterDevice *pPOSPrinter) {


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
		strDesc = "Total: " + FormatCurrencyForInterface (m_cyPayAmount + cyTipAmount);
		nWhiteLen = nLineWidth - strDesc.GetLength();
		for (i = 0; i < nWhiteLen; i++) {
			strDesc = " " + strDesc;
		}
		strOutput += LeftJustify(strDesc, nLineWidth);
	}

	strOutput += "\n\n";
	
	//add the customer copy
	strOutput += CenterLine("Customer Copy", nLineWidth);

	if (pPOSPrinter->PrintText(strOutput)) {
		return TRUE;
	}
	else {
		return FALSE;
	}

}


BOOL CPAYMENTECH_ProcessCreditCardDlg::PrintReceiptManuallyProcessed() {

	try {
		// (j.gruber 2007-07-30 10:15) - PLID 26720 - run a regular report if they don't have a receipt printer
		if (GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE)) {
			//first let's acquire the receipt printer
			CWnd *pMainFrame = GetMainFrame();

			if (pMainFrame) {
				//TES 12/6/2007 - PLID 28192 - We're ready to start using the POS Printer, so claim it.
				// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
				// not to mention providing exception safety.
				//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();

				POSPrinterAccess pPOSPrinter;

				if (pPOSPrinter) {
					
					//First print the header
					if (!PrintReceiptHeader(pPOSPrinter, IsManuallyApproved)) {
						//GetMainFrame()->ReleasePOSPrinter();
						return FALSE;
					}
					CString strApprovalCode;
					GetDlgItemText(IDC_AUTHORIZATION_NUMBER, strApprovalCode);
					CString strOutput = "Voice Approval Code: " + strApprovalCode;


					if (!PrintReceiptMiddle(pPOSPrinter, strOutput)) {
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

			paramInfo = new CRParameterInfo;
			if (m_bSwiped) {
				paramInfo->m_Data = "Card Swiped";
			}
			else {
				paramInfo->m_Data = "Card Manually Entered";
			}
			paramInfo->m_Name = "CardSwipeInfo";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			if (m_nType == 1) {
				paramInfo->m_Data = "Sale";
			}
			else if (m_nType == 3) {
				paramInfo->m_Data = "Refund";
			}
			paramInfo->m_Name = "TransactionType";
			paParams.Add((void *)paramInfo);

			CString strApprovalCode;
			GetDlgItemText(IDC_AUTHORIZATION_NUMBER, strApprovalCode);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "VOICE AUTHORIZATION: " + strApprovalCode;
			paramInfo->m_Name = "StatusMessage";
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
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::PrintReceiptManuallyProcessed");

	return FALSE;



}

// (j.gruber 2007-08-02 17:13) - PLID 26650 - customer copy receipt
BOOL CPAYMENTECH_ProcessCreditCardDlg::PrintReceipt(PaymentechUtils::ResponseStruct &tsResponse) {

	try {
		// (j.gruber 2007-07-30 10:15) - PLID 26720 - run a regular report if they don't have a receipt printer
		if (GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE)) {
			//first let's acquire the receipt printer
			CWnd *pMainFrame = GetMainFrame();

			if (pMainFrame && GetMainFrame()->CheckPOSPrinter()) {

				EApprovalCode appCode;
				CString strOutput;

				if (tsResponse.bApproved) {
					appCode = IsApproved;
					strOutput = "Approval Code: " + tsResponse.strAuthCode;
				}
				else {
					appCode = IsDeclined;
					strOutput = "Declined: " + PaymentechUtils::MessageLookup(tsResponse.strErrorCode, tsResponse.strResponseMessage, tsResponse.lcLangCode);
				}

				//TES 12/6/2007 - PLID 28192 - We're ready to start using the POS Printer, so claim it.
				// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
				// not to mention providing exception safety.
				//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();
				POSPrinterAccess pPOSPrinter;
				if (pPOSPrinter) {

					//First print the header
					if (!PrintReceiptHeader(pPOSPrinter, appCode)) {
						//GetMainFrame()->ReleasePOSPrinter();
						return FALSE;
					}

					if (!PrintReceiptMiddle(pPOSPrinter, strOutput)) {
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

			paramInfo = new CRParameterInfo;
			if (m_bSwiped) {
				paramInfo->m_Data = "Card Swiped";
			}
			else {
				paramInfo->m_Data = "Card Manually Entered";
			}
			paramInfo->m_Name = "CardSwipeInfo";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			if (m_nType == 1) {
				paramInfo->m_Data = "Sale";
			}
			else if (m_nType == 3) {
				paramInfo->m_Data = "Refund";
			}
			paramInfo->m_Name = "TransactionType";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			if (tsResponse.bApproved) {
				paramInfo->m_Data = "APPROVED";
			}
			else {
				CString strResp = tsResponse.strResponseMessage;
				strResp.TrimRight();
				strResp.TrimLeft();
				paramInfo->m_Data = "DECLINED: " + strResp;
			}
			paramInfo->m_Name = "StatusMessage";
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

	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::PrintReceipt");

	return FALSE;


}

void CPAYMENTECH_ProcessCreditCardDlg::OnRequeryFinishedCcAuthTipList(short nFlags) 
{
	try {
		//add up the amounts
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTipList->GetFirstRow();

		COleCurrency cyTransAmount;
		COleCurrency cyTipAmount;

		while (pRow) {

			//if its a charge tip
			if (VarLong(pRow->GetValue(tlcPayMethod)) == 3) {

				cyTransAmount += VarCurrency(pRow->GetValue(tlcAmount));
			}

			cyTipAmount += VarCurrency(pRow->GetValue(tlcAmount));

			pRow = pRow->GetNextRow();
		}

		SetDlgItemText(IDC_TOTAL_TIP_AMT, FormatCurrencyForInterface(cyTipAmount));
		
		cyTransAmount += m_cyPayAmount;

		SetDlgItemText(IDC_TOTAL_TRANS_AMT, FormatCurrencyForInterface(cyTransAmount));
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::OnRequeryFinishedCCAuthTipList");
}

void CPAYMENTECH_ProcessCreditCardDlg::OnRButtonDownCcAuthTipList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
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
	
	} NxCatchAll("Error in OnRButtonDownCcAuthTipList()");	
}

void CPAYMENTECH_ProcessCreditCardDlg::OnEditingFinishedCcAuthTipList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		//update the transaction amount if it is a charge
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		switch (nCol) {

			case tlcAmount:

				if (VarLong(pRow->GetValue(tlcPayMethod)) == 3) {

					UpdateTotalTransAmt();
							
				}	

				UpdateTotalTipAmt();
			break;

			case tlcPayMethod:

				if ((VarLong(varNewValue) == 3) && VarLong(varOldValue) != 3) {

					UpdateTotalTransAmt();
				}	
				else if ((VarLong(varOldValue) == 3) && VarLong(varNewValue) != 3) {

					//they changed from charge to something else
					UpdateTotalTransAmt();
				}	
			break;
		}
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::OnEditingFinishedCcAuthTipList");
	
}


LRESULT CPAYMENTECH_ProcessCreditCardDlg::OnMSRDataEvent(WPARAM wParam, LPARAM lParam) {

	try {
		//alrighty, let's fill our information with this card
		MSRTrackInfo *mtiInfo = (MSRTrackInfo*)wParam;
		// (j.jones 2009-06-19 11:03) - PLID 33650 - support the msrCardType enum
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
								"exist in the card type list.  Please add this card type to the list before trying to proces this card.", cciCardInfo->m_strCardType);
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
					ASSERT(nResult >= 0);
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

				// (j.gruber 2007-07-03 16:01) - PLID 15416 - update our boolean that we have swiped the card
				m_bSwiped = TRUE;
				m_swtType = swtMSRDevice;
				m_strTrack1 = mtiInfo->strTrack1;
				m_strTrack2 = mtiInfo->strTrack2;
				m_strTrack3 = mtiInfo->strTrack3;
			}
		}
	}NxCatchAll("Error in  CPAYMENTECH_ProcessCreditCardDlg::OnMSRDataEvent");

	return 0;

}


void CPAYMENTECH_ProcessCreditCardDlg::OnOK() 
{
	try {

		if (!CheckFields()) {
			return;
		}


		//check to make sure they filled in an Auth Number
		CString strAuthCode;
		GetDlgItemText(IDC_AUTHORIZATION_NUMBER, strAuthCode);
		strAuthCode.TrimLeft();
		strAuthCode.TrimRight();
		if (strAuthCode.IsEmpty()) {
			MsgBox("Please enter an authorization code or click cancel.");
			return;
		}
		if (strAuthCode.GetLength() != 6) {
			MsgBox("The authorization code should be 6 characters. Please check the code.");
			return;
		}

		//save just in case they changed anything
		SaveChanges();

		if (m_bHasChanged) {
			//they changed some card information, pop up the Street address dialog
			if (!DoAddressVerification() ) {
				//they decided to cancel completely 
				DeletePayment(m_nPaymentID, TRUE);
				m_bIsDeleted = TRUE;
				CDialog::OnCancel();
			}
		}

		//update the transaction
		ExecuteSql("UPDATE CreditTransactionsT SET ManuallyProcessed = 1, "
			" CardHolderStreetAddress = '%s', CardHolderStreetAddress2 = '%s', CardHolderZip = '%s'  "
			" WHERE ID = %li", m_strCardHolderAddress.Left(20), m_strCardHolderAddress2.Left(20), m_strCardHolderZip, m_nPaymentID); 

		ExecuteSql("UPDATE PaymentPlansT SET CCAuthNo = '%s' WHERE ID = %li", _Q(strAuthCode), m_nPaymentID); 

		//print out the receipt
		PrintReceiptManuallyProcessed();

		CDialog::OnOK();
	}NxCatchAll("Error in OnOK");
}

void CPAYMENTECH_ProcessCreditCardDlg::OnAuthorizeManually() 
{
	if (IsDlgButtonChecked(IDC_AUTHORIZE_MANUALLY)) {

		GetDlgItem(IDC_AUTH_NUMBER_LABEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_AUTHORIZATION_NUMBER)->EnableWindow(TRUE);

		GetDlgItem(IDC_PROCESS_CC_CARD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
	}
	else {
		
		GetDlgItem(IDC_AUTH_NUMBER_LABEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUTHORIZATION_NUMBER)->EnableWindow(FALSE);

		GetDlgItem(IDC_PROCESS_CC_CARD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
	}
	
}

void CPAYMENTECH_ProcessCreditCardDlg::OnUpdateCcAuthExpDate() 
{
	CString str;
	GetDlgItem(IDC_CC_AUTH_EXP_DATE)->GetWindowText(str);
	FormatItemText(GetDlgItem(IDC_CC_AUTH_EXP_DATE), str, "##/##nn");
	
}

void CPAYMENTECH_ProcessCreditCardDlg::OnKillfocusCCNumber()
{
	try{
		HandleOnKillfocusCCNumber();
	}NxCatchAll("Error in OnKillfocusCCNumber");
}

void CPAYMENTECH_ProcessCreditCardDlg::HandleOnKillfocusCCNumber()
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
