#include "stdafx.h"
#include "FinancialCorrection.h"
#include "Audittrail.h"
#include "GlobalAuditUtils.h"
#include "internationalUtils.h"
#include "GlobalFinancialUtils.h"

// (j.gruber 2011-08-03 11:26) - PLID 44836 - created for

CFinancialCorrection::CFinancialCorrection(void)
{
}

CFinancialCorrection::~CFinancialCorrection(void)
{
	try {
	
		//clear out our arrays
		ClearArrays();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-09-20 15:55) - PLID 45462 - there was no need for a filter here
void CFinancialCorrection::ClearArrays() {

	for(int i=m_aryBillCorrections.GetSize()-1;i>=0;i--) {
		BillCorrection *pBill = ((BillCorrection*)m_aryBillCorrections.GetAt(i));
		delete pBill;
	}
	m_aryBillCorrections.RemoveAll();

	for(int i=m_aryChargeCorrections.GetSize()-1;i>=0;i--) {
		ChargeCorrection *pCharge = ((ChargeCorrection*)m_aryChargeCorrections.GetAt(i));
		delete pCharge;
	}
	m_aryChargeCorrections.RemoveAll();

	for(int i=m_aryPaymentCorrections.GetSize()-1;i>=0;i--) {
		PaymentCorrection *pPay = ((PaymentCorrection*)m_aryPaymentCorrections.GetAt(i));
		delete pPay;
	}
	m_aryPaymentCorrections.RemoveAll();

	// (j.jones 2011-09-20 15:56) - PLID 45462 - added Undo feature for bills
	for(int i=m_aryBillCorrectionsToUndo.GetSize()-1;i>=0;i--) {
		BillCorrectionUndo *pBill = ((BillCorrectionUndo*)m_aryBillCorrectionsToUndo.GetAt(i));
		delete pBill;
	}
	m_aryBillCorrectionsToUndo.RemoveAll();

	// (j.jones 2011-09-20 15:56) - PLID 45562 - added Undo feature for charges
	for(int i=m_aryChargeCorrectionsToUndo.GetSize()-1;i>=0;i--) {
		ChargeCorrectionUndo *pCharge = ((ChargeCorrectionUndo*)m_aryChargeCorrectionsToUndo.GetAt(i));
		delete pCharge;
	}
	m_aryChargeCorrectionsToUndo.RemoveAll();

	// (j.jones 2011-09-20 15:56) - PLID 45563 - added Undo feature for payments
	for(int i=m_aryPaymentCorrectionsToUndo.GetSize()-1;i>=0;i--) {
		PaymentCorrectionUndo *pPay = ((PaymentCorrectionUndo*)m_aryPaymentCorrectionsToUndo.GetAt(i));
		delete pPay;
	}
	m_aryPaymentCorrectionsToUndo.RemoveAll();
}

// (j.gruber 2011-06-02 17:18) - PLID 44850 - added Corrections
// (j.jones 2012-04-23 16:03) - PLID 48032 - added TakebackIntoBatchPaymentID, which ties the correction to
// the batch payment and adds the voided payment's value to the batch payment balance
// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
void CFinancialCorrection::AddCorrection(CorrectionType ctType, long nID, CString strUserName, long nUserID,
										 BOOL bCorrect /*= TRUE*/, long nTakebackIntoBatchPaymentID /*= -1*/,
										CString strTakebackBatchPaymentCheckNo /*= ""*/, CString strTakebackBatchPaymentDate /*= ""*/)
{
	//can't send in ctAll here
	if (ctType == ctAll) {
		ASSERT(FALSE);
		ThrowNxException("Invalid type sent to AddCorrection.");
		return;
	}

	// (j.jones 2012-04-23 16:06) - PLID 48032 - can't use nTakebackIntoBatchPaymentID on anything but a payment
	if(nTakebackIntoBatchPaymentID != -1 && ctType != ctPayment) {
		ASSERT(FALSE);
		ThrowNxException("Non-payment type sent to AddCorrection with nTakebackIntoBatchPaymentID in use.");
		return;
	}

	switch (ctType) {

		case ctBill:
			{
				BillCorrection *pBill = new BillCorrection();
				pBill->nBillID = nID;
				pBill->nUserID = nUserID;
				pBill->strUserName = strUserName;
				// (j.gruber 2011-06-02 15:07) - PLID 44850
				pBill->bCorrect = bCorrect;

				m_aryBillCorrections.Add((BillCorrection*)pBill);

			}
		break;

		case ctCharge:
			{
				ChargeCorrection *pCharge = new ChargeCorrection();
				pCharge->nChargeID = nID;
				pCharge->nUserID = nUserID;
				pCharge->strUserName = strUserName;
				// (j.gruber 2011-06-02 15:07) - PLID 44850
				pCharge->bCorrect = bCorrect;
				
				m_aryChargeCorrections.Add((ChargeCorrection*)pCharge);
			}
		break;

		case ctPayment:
			{
				PaymentCorrection *pPay = new PaymentCorrection();
				pPay->nPaymentID = nID;
				pPay->nUserID = nUserID;
				pPay->strUserName = strUserName;
				// (j.gruber 2011-06-02 15:07) - PLID 44850
				pPay->bCorrect = bCorrect;
				// (j.jones 2012-04-23 16:12) - PLID 48032 - added nTakebackIntoBatchPaymentID
				pPay->nTakebackIntoBatchPaymentID = nTakebackIntoBatchPaymentID;
				// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
				pPay->strTakebackBatchPaymentCheckNo = strTakebackBatchPaymentCheckNo;
				pPay->strTakebackBatchPaymentDate = strTakebackBatchPaymentDate;
				
				m_aryPaymentCorrections.Add((PaymentCorrection*)pPay);
			}
		break;
	}

			
}

// (j.jones 2012-04-23 15:17) - PLID 49847 - added boolean for whether E-Remittance called this correction
// (j.jones 2015-06-16 16:51) - PLID 64932 - removed correction type, because execute should handle
// every correction in our list
void CFinancialCorrection::ExecuteCorrections(BOOL bCorrectedByERemit /*= FALSE*/)
{
	CSqlFragment sqlFrag(" SET NOCOUNT ON; \r\n "
		" SET XACT_ABORT ON \r\n"
		" BEGIN TRAN \r\n"
		// (j.jones 2011-10-31 14:33) - PLID 45462 - keep track of one consistent input date
		// for every correction in this batch, so they are tracked as being created at the exact time
		" DECLARE @dtInputDate DATETIME \r\n"
		" SET @dtInputDate = GetDate() \r\n"
		// (j.gruber 2011-06-24 17:18) - PLID 44851 - keep track of the bills we correct
		" DECLARE @AddedBillCorrections TABLE (ID INT NOT NULL) \r\n "
		" DECLARE @AddedLineItemCorrections TABLE (ID INT NOT NULL) \r\n "
		" DECLARE @BillCorrections TABLE (ID INT NOT NULL PRIMARY KEY, UserName nVarchar(50) NOT NULL, UserID INT NOT NULL, Correct BIT NOT NULL) \r\n "
		" DECLARE @ChargeCorrections TABLE (ID INT NOT NULL PRIMARY KEY, UserName nVarchar(50) NOT NULL, UserID INT NOT NULL, Correct BIT NOT NULL) \r\n "
		// (j.jones 2012-04-23 16:20) - PLID 48032 - supported BatchPaymen
		// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
		// (j.jones 2015-08-17 15:16) - PLID 59210 - added an OrderIndex identity, to enforce correcting in the order this table is filled
		" DECLARE @PaymentCorrections TABLE (OrderIndex INT NOT NULL PRIMARY KEY IDENTITY(1,1), \r\n"
		"	ID INT NOT NULL UNIQUE, \r\n"
		"	TakebackIntoBatchPaymentID INT NULL, TakebackBatchPaymentCheckNo nvarchar(50), TakebackBatchPaymentDate nvarchar(50), \r\n"
		"	UserName nvarchar(50) NOT NULL, UserID INT NOT NULL, Correct BIT NOT NULL) \r\n"

		/* need to declare this here*/
		" DECLARE @nNewBillID INT; \r\n "		
		);
	// (j.gruber 2011-08-08 10:08) - PLID 45596 - Adjustment Category
	sqlFrag += CSqlFragment( " DECLARE @nVoidChargeCat INT; \r\n "
		" SET @nVoidChargeCat = {INT}; \r\n "
		" IF @nVoidChargeCat = -1 BEGIN \r\n "
			" SET @nVoidChargeCat = 0; \r\n "
		" END \r\n "
		, GetRemotePropertyInt("VoidChargeAdj_DefaultCat", -1, 0, "<None>", true));


	// (j.jones 2015-06-16 16:53) - PLID 64932 - we now execute all corrections in our object
	sqlFrag += CorrectBills();
	sqlFrag += CorrectCharges();
	sqlFrag += CorrectPayments();
	
	sqlFrag += " SET NOCOUNT OFF; \r\n "
		// (j.gruber 2011-06-24 17:18) - PLID 44845 - auditing
		" SELECT LineItemCorrectionsT.ID, LineItemT.Type, LineItemCorrectionsT.OriginalLineItemID, LineItemCorrectionsT.NewLineItemID, LineItemT.PatientID, \r\n "
		" LineItemT.Description, CASE WHEN LineItemT.Type = 10 THEN dbo.GetchargeTotal(LineItemT.ID) ELSE LineItemT.Amount END as Amount, ChargesT.Quantity, LineItemT.Date, NewLineItemsT.Description as NewDescription "
		" FROM @AddedLineItemCorrections AddedLineItemCorrectionsT \r\n "
		" LEFT JOIN LineItemCorrectionsT ON AddedLineItemCorrectionsT.ID = LineItemCorrectionsT.ID \r\n "
		" LEFT JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID \r\n "
		" LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n "
		" LEFT JOIN LineItemT NewLineItemsT ON LineItemCorrectionsT.NewLineItemID = NewLineItemsT.ID "
		" UNION ALL \r\n "
		" SELECT BillCorrectionsT.ID, -1 as Type, BillCorrectionsT.OriginalBillID, BillCorrectionsT.NewBillID, BillsT.PatientID, \r\n "
		" BillsT.Description, 0 as ChargeTotal, -1 as Qty, BillsT.Date, NewBillsT.Description "
		" FROM @AddedBillCorrections AddedBillCorrectionsT \r\n "
		" LEFT JOIN BillCorrectionsT ON AddedBillCorrectionsT.ID = BillCorrectionsT.ID \r\n "
		" LEFT JOIN BillsT ON BillCorrectionsT.OriginalBillID = BillsT.ID \r\n "
		" LEFT JOIN BillsT NewBillsT ON BillCorrectionsT.NewBillID = NewBillsT.ID \r\n"
		" COMMIT TRAN \r\n";

	ADODB::_RecordsetPtr rsFinCorr = CreateParamRecordset(sqlFrag);

	long nAuditTransID = -1;
	try {
		// (j.gruber 2011-06-15 11:34) - PLID 44845 - audit!!		
		while (! rsFinCorr->eof) {

			if (nAuditTransID == -1 ){
				nAuditTransID = BeginAuditTransaction();
			}

			long nType = AdoFldLong(rsFinCorr->Fields, "Type");

			AuditEventItems aeiItemVoided;
			AuditEventItems aeiItemNew;
			CString strDescription;
			CString strNewDescription;
			switch (nType) {

				// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
				// (j.gruber 2011-08-05 15:51) - PLID 44845 - take out amount for bills
				case -1: //bill
					
					// (j.jones 2012-04-23 15:22) - PLID 49847 - An e-remit correction should never
					// correct a bill. This should be a huge red flag if it happened.
					if(bCorrectedByERemit) {
						ThrowNxException("Attempted a bill correction through an E-Remittance posting!");
					}

					aeiItemVoided = aeiVoidBill;
					aeiItemNew = aeiBillCreated;
					strDescription =  "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "Description", "");
						
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					// (j.gruber 2011-08-05 15:52) - PLID 44845  - take out amount for bills
					strNewDescription = "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "NewDescription", "");
						

				break;
				case 10: //charge

					// (j.jones 2012-04-23 15:22) - PLID 49847 - An e-remit correction should never
					// correct a charge. This should be a huge red flag if it happened.
					if(bCorrectedByERemit) {
						ThrowNxException("Attempted a charge correction through an E-Remittance posting!");
					}

					aeiItemVoided = aeiVoidCharge;
					aeiItemNew = aeiNewChargeFromVoid;
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strDescription = + "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0)) )
						+ " Quantity: " + AsString(rsFinCorr->Fields->Item["Quantity"]->Value) 						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "Description", ""); 


					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strNewDescription = "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0)) )
						+ " Quantity: " + AsString(rsFinCorr->Fields->Item["Quantity"]->Value) 						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "NewDescription", ""); 
					
				break;

				case 1:
					// (j.jones 2012-04-23 15:20) - PLID 49847 - use the ERemit audit types if called from an EOB posting
					if(bCorrectedByERemit) {
						aeiItemVoided  = aeiVoidPaymentByERemit;
						aeiItemNew = aeiPaymentCreatedByERemit;
					}
					else {
						aeiItemVoided  = aeiVoidPayment;
						aeiItemNew = aeiPaymentCreated;
					}
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strDescription =  "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0)))						 
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "Description", "");
						
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strNewDescription =  "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0)))						 						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "NewDescription", "");
						
				break;

				case 2:
					// (j.jones 2012-04-23 15:20) - PLID 49847 - use the ERemit audit types if called from an EOB posting
					if(bCorrectedByERemit) {
						aeiItemVoided  = aeiVoidAdjustmentByERemit;
						aeiItemNew = aeiAdjustmentCreatedByERemit;
					}
					else {
						aeiItemVoided  = aeiVoidAdjustment;
						aeiItemNew = aeiAdjustmentCreated;
					}
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strDescription =  "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0)) )						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "Description", "");
					
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strNewDescription = "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0)) )						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "NewDescription", "");
						

				break;

				case 3:
					// (j.jones 2012-04-23 15:20) - PLID 49847 - Use the ERemit audit types if called from an EOB posting.
					// This would never happen directly on a refund, but it might if a refund was applied to a payment
					// corrected by an E-Remit posting.
					if(bCorrectedByERemit) {
						aeiItemVoided  = aeiVoidRefundByERemit;
						aeiItemNew = aeiRefundCreatedByERemit;
					}
					else {
						aeiItemVoided  = aeiVoidRefund;
						aeiItemNew = aeiRefundCreated;
					}
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first
					strDescription = "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0))) 						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "Description", ""); 
					
					// (j.gruber 2011-08-04 17:36) - PLID 44881 - put date and amount first	
					strNewDescription = "Date: " + FormatDateTimeForInterface(AdoFldDateTime(rsFinCorr->Fields, "Date"))
						+ " Amount: " + FormatCurrencyForInterface(AdoFldCurrency(rsFinCorr->Fields, "Amount", COleCurrency(0,0))) 						
						+ " Description: " + AdoFldString(rsFinCorr->Fields, "NewDescription", ""); 
						
				break;
			}	

			long nPatID = AdoFldLong(rsFinCorr->Fields, "PatientID");
			AuditEvent(nPatID, GetExistingPatientName(nPatID), nAuditTransID, aeiItemVoided, nPatID, strDescription, "<Voided>", aepHigh, aetChanged);

			long nNewLineItemID = AdoFldLong(rsFinCorr->Fields, "NewLineItemID", -1);
			if (nNewLineItemID != -1) {
				//make the corrected item entry
				AuditEvent(nPatID, GetExistingPatientName(nPatID), nAuditTransID, aeiItemNew, nPatID, "", strNewDescription, aepHigh, aetCreated);
			}

			rsFinCorr->MoveNext();
		}
		if (nAuditTransID != -1) {
			CommitAuditTransaction(nAuditTransID);
		}
	

	}NxCatchAllCall(__FUNCTION__, 
		if (nAuditTransID != -1) {
			RollbackAuditTransaction(nAuditTransID);
		}
	);



}

long CFinancialCorrection::GetChargeCorrectionCount()
{
	return m_aryChargeCorrections.GetSize();
}

long CFinancialCorrection::GetPaymentCorrectionCount()
{
	return m_aryPaymentCorrections.GetSize();
}

long CFinancialCorrection::GetBillCorrectionCount()
{
	return m_aryBillCorrections.GetSize();
}

/*this function is responsible for correcting all charges in the given charge array*/
// (j.gruber 2011-08-09 09:49) - PLID 44952
CSqlFragment CFinancialCorrection::CorrectCharges()
{
	
	//first generate our Insert Statement	
	
	
	CSqlFragment sqlFrag(" /* BEGIN CHARGE CORRECTION */ \r\n ");
	
	for (int i = 0; i < m_aryChargeCorrections.GetSize(); i++) {
		ChargeCorrection *pCharge = ((ChargeCorrection*)m_aryChargeCorrections.GetAt(i));
		sqlFrag += GenerateChargeInsertSql(pCharge);
	}

	//first we need to create our negative charges
	sqlFrag += GenerateAllChargeCorrectionSql();	

	sqlFrag += " /*END CHARGE CORRECTION */ \r\n ";	

	return sqlFrag;
}

// (j.gruber 2011-08-09 09:49) - PLID 44952
CSqlFragment CFinancialCorrection::GenerateChargeInsertSql(ChargeCorrection *pCharge) 
{
	// (j.gruber 2011-06-02 15:08) - PLID 44850 - added Correct Field
	return CSqlFragment(
		FormatString(
		" /* INSERT CHARGE CORRECTION*/ \r\n "
		" INSERT INTO @ChargeCorrections (ID, UserName, UserID, Correct) \r\n "
		" VALUES  ( {INT}, {STRING}, {INT}, {BIT} )\r\n"),  
		pCharge->nChargeID, pCharge->strUserName, pCharge->nUserID, pCharge->bCorrect);

}

// (j.gruber 2011-05-24 17:53) - PLID 44952
CSqlFragment CFinancialCorrection::GenerateAllChargeCorrectionSql() 
{
	CString strSql;

	// (j.gruber 2011-06-02 15:11) - PLID 44850 - added bCorrect field
	strSql.Format(
		" /*BEGIN CORRECTING CHARGES*/ \r\n "
		" DECLARE rsChargeCorrections CURSOR LOCAL SCROLL READ_ONLY FOR  \r\n "
		" 	SELECT ID, UserName, UserID, Correct FROM @ChargeCorrections ChargeCorrectionsT \r\n "
		" DECLARE @nChargeIDToFix INT; \r\n "
		" DECLARE @nUserID INT; \r\n "
		" DECLARE @strUserName nVarchar(50) \r\n "
		" DECLARE @bNegated BIT; \r\n "	
		" DECLARE @bCorrect BIT; \r\n "
		// (j.gruber 2011-06-02 16:55) - PLID 44848 - add special descriptions
		" DECLARE @strDescription NVarchar(50); \r\n "

		" DECLARE @nBillIDToUse INT; \r\n "
		" DECLARE @bUseNewBill BIT; \r\n "

		" DECLARE @nNewChargeRespID INT; \r\n "
		" DECLARE @nChargeRespDetailID INT; \r\n "
		" DECLARE @nNewChargeRespDetailID INT; \r\n "

		" DECLARE @nLineItemCorrectionID INT; \r\n "

		// (j.jones 2015-02-19 12:38) - PLID 64932 - need to track the void resp ID and new resp ID
		" DECLARE @ChargeRespVoidLinkT TABLE (OrigChargeRespID INT NOT NULL, VoidChargeRespID INT NOT NULL) \r\n "
		" DECLARE @ChargeRespNewLinkT TABLE (OrigChargeRespID INT NOT NULL, NewChargeRespID INT NOT NULL) \r\n "
		" DECLARE @ChargeRespDetailVoidLinkT TABLE (OrigChargeRespDetailID INT NOT NULL, VoidChargeRespDetailID INT NOT NULL) \r\n "
		" DECLARE @ChargeRespDetailNewLinkT TABLE (OrigChargeRespDetailID INT NOT NULL, NewChargeRespDetailID INT NOT NULL) \r\n "
		" DELETE FROM @ChargeRespVoidLinkT; \r\n "
		" DELETE FROM @ChargeRespNewLinkT; \r\n "
		" DELETE FROM @ChargeRespDetailVoidLinkT; \r\n "
		" DELETE FROM @ChargeRespDetailNewLinkT; \r\n "

		" DECLARE @ApplyInsertions TABLE (ApplyID INT NOT NULL) \r\n "

		" OPEN rsChargeCorrections; \r\n "
		" FETCH FROM rsChargeCorrections INTO @nChargeIDToFix, @strUserName, @nUserID, @bCorrect \r\n "
		" WHILE @@FETCH_STATUS = 0 BEGIN \r\n "		
		"	 DECLARE @nNewChargeID INT; \r\n "
		"    DECLARE @nLineID INT; \r\n "
		"	 DECLARE @nVoidedChargeID INT; \r\n "
		"	 DECLARE @nCorrectedChargeID INT; \r\n "	

		
		"	DECLARE @nChargeRespID INT; \r\n "
		"	DECLARE @nInsuredPartyID INT; \r\n "
		"	DECLARE @cyAmount money; \r\n "		
		
		);

	CSqlFragment sqlFrag(strSql);

	//now append the voided Charge Sql
	sqlFrag += CSqlFragment(" SET @bNegated = 1; \r\n ");
	sqlFrag += CSqlFragment(" SET @bUseNewBill = 0; \r\n ");	
	// (j.gruber 2011-06-02 16:55) - PLID 44848 - add special descriptions
	sqlFrag += CSqlFragment(" SET @strDescription = 'Voided Charge '; \r\n ");
	sqlFrag += CSqlFragment(" /* BEGIN NEGATIVE CHARGE */ \r\n ");
	sqlFrag += GenerateChargeCorrectionSql();
	sqlFrag += CSqlFragment(" /* END NEGATIVE CHARGE */ \r\n ");
	//set the voided charge and set the charge resps back to the beginning
	sqlFrag += CSqlFragment(" SET @nVoidedChargeID = @nNewChargeID; \r\n ");
		//" FETCH FIRST FROM rsChargeCorrections INTO @nChargeIDToFix, @strUserName, @nUserID, @bCorrect \r\n ");
	

	//now append the new charge
	sqlFrag += CSqlFragment( "SET @bNegated = 0; \r\n ");
	// (j.gruber 2011-06-02 16:55) - PLID 44848 - add special descriptions
	sqlFrag += CSqlFragment(" SET @strDescription = 'Corrected Charge '; \r\n ");
	// (j.gruber 2011-06-02 15:09) - PLID 44850 - Only do this if we are correcting
	sqlFrag += CSqlFragment(" IF @bCorrect = 1 BEGIN \r\n ");
	// (j.gruber 2011-06-06 16:42) - PLID 44851
	sqlFrag += CSqlFragment(" SET @nBillIDToUse = @nNewBillID; \r\n ");
	sqlFrag += CSqlFragment(" SET @bUseNewBill = (CASE WHEN @nNewBillID IS NULL THEN 0 ELSE 1 END); \r\n ");	
	sqlFrag += CSqlFragment(" /* BEGIN NEW CHARGE */ \r\n ");
	sqlFrag += GenerateChargeCorrectionSql();
	sqlFrag += CSqlFragment(" /* END NEW CHARGE */ \r\n ");
	sqlFrag += GenerateChargeContraintsSql();
	// (j.gruber 2011-06-02 15:10) - PLID 44850 - End the bCorrect IF
	sqlFrag += CSqlFragment(" SET @nCorrectedChargeID = @nNewChargeID; \r\n ");	
	sqlFrag += CSqlFragment(" END \r\n ");	

	// (j.dinatale 2011-10-31 17:39) - PLID 44923 - remove any linked products with serial numbers or expiration dates
	sqlFrag += " DELETE FROM ChargedProductItemsT WHERE ChargeID = @nChargeIDToFix \r\n; ";
	
	sqlFrag += " DECLARE @LineItemAdjQ TABLE (AdjustmentID INT) ";

	sqlFrag += CSqlFragment("INSERT INTO LineItemCorrectionsT(OriginalLineItemID, VoidingLineItemID, NewLineItemID, InputDate, InputUserID) \r\n"
		"	VALUES (@nChargeIDToFix, @nVoidedChargeID, @nCorrectedChargeID, @dtInputDate, @nUserID) \r\n"
		"   SET @nLineItemCorrectionID = SCOPE_IDENTITY(); "
		"	INSERT INTO @AddedLineItemCorrections SELECT @nLineItemCorrectionID; \r\n\r\n");


	//now we need to adjust off any original payments to the charge
	//sqlFrag += CSqlFragment(" /* BEGIN CHARGE APPLIES BY APPLIES */ \r\n ");
	//sqlFrag += GenerateChargeAppliesByAppliesToChargeSql();
	//sqlFrag += CSqlFragment(" /* END CHARGE APPLIES BY APPLIES */ \r\n ");

	//adjust off the negative charge completely
	sqlFrag += CSqlFragment(" /* BEGIN CHARGE APPLIES BY CHARGE RESP */ \r\n ");
	sqlFrag += GenerateChargeAppliesByChargeRespSql();	
	sqlFrag += CSqlFragment(" /* END CHARGE APPLIES BY CHARGE RESP */ \r\n ");	

	//finally make our lineItemCorrection adjustment records
	sqlFrag += CSqlFragment(" /* BEGIN CORRECTION ADJUSTMENTS*/ \r\n ");
	sqlFrag += GenerateChargeCorrectionAdjustmentsSql();
	sqlFrag += CSqlFragment(" /* END CORRECTION ADJUSTMENTS*/ \r\n ");

	//now add the applied payments
	// (j.jones 2012-04-23 16:20) - PLID 48032 - supported BatchPaymentID
	// (j.jones 2014-07-15 10:37) - PLID 62876 - ignore chargebacks, they remain solely on the original charge
	// (j.jones 2015-02-19 15:01) - PLID 64999 - Always correct applies to charges. If we're voiding a charge, we're not voiding its applies.
	// (j.jones 2015-11-04 16:22) - PLID 57416 - fixed to handle cases where one payment was applied twice
	sqlFrag += 	" INSERT INTO @PaymentCorrections (ID, TakebackIntoBatchPaymentID, UserName, UserID, Correct) "
		" SELECT SourceID, NULL, @strUserName, @nUserID, Convert(bit, 1) \r\n"
		" FROM (SELECT SourceID, DestID FROM AppliesT GROUP BY SourceID, DestID) AS AppliesQ  \r\n "
		" INNER JOIN LineItemT ON AppliesQ.SourceID = LineItemT.ID \r\n "
		" LEFT JOIN LineItemCorrectionsT OrigLineItem ON AppliesQ.SourceID = OrigLineItem.OriginalLineItemID \r\n "
		" LEFT JOIN LineItemCorrectionsT VoidLineItem ON AppliesQ.SourceID = VoidLineItem.VoidingLineItemID \r\n "
		" LEFT JOIN LineItemCorrectionsBalancingAdjT LCBA ON AppliesQ.SourceID = LCBA.BalancingAdjID \r\n "
		" LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON LineItemT.ID = Chargebacks_PaymentQ.PaymentID \r\n "
		" LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON LineItemT.ID = Chargebacks_AdjQ.AdjustmentID \r\n "
		" WHERE LineItemT.Deleted = 0 AND OrigLineItem.ID IS NULL AND VoidLineItem.ID IS NULL \r\n "
		" AND LCBA.ID IS NULL AND AppliesQ.DestID = @nChargeIDToFix \r\n "
		" AND Chargebacks_PaymentQ.PaymentID Is Null \r\n "
		" AND Chargebacks_AdjQ.AdjustmentID Is Null \r\n "
		" AND SourceID NOT IN (SELECT PaymentCorrections.ID FROM @PaymentCorrections PaymentCorrections) \r\n ";

	// (j.jones 2015-06-08 12:44) - PLID 64932 - clear the void link tables, do not clear the New link tables
	sqlFrag += " DELETE FROM @ChargeRespVoidLinkT; \r\n "
		" DELETE FROM @ChargeRespDetailVoidLinkT; \r\n ";

	//delete from Correction adjustments
	sqlFrag += " DELETE FROM @LineItemAdjQ \r\n ";

	//end our query
	sqlFrag += CSqlFragment(" FETCH FROM rsChargeCorrections INTO @nChargeIDToFix, @strUserName, @nUserID, @bCorrect  \r\n "
		" END "
		" CLOSE rsChargeCorrections \r\n "
		" DEALLOCATE rsChargeCorrections \r\n "
		" /*END CORRECTING CHARGE*/ \r\n ");
	
	return sqlFrag;

}

// (j.gruber 2011-08-09 09:49) - PLID 44952
CSqlFragment CFinancialCorrection::GenerateChargeCorrectionAdjustmentsSql()
{
	return CSqlFragment(" INSERT INTO LineItemCorrectionsBalancingAdjT (LineItemCorrectionID, BalancingAdjID) \r\n "
		" SELECT @nLineItemCorrectionID, AdjustmentID FROM @LineItemAdjQ LineItemAdjQ \r\n ");		
}

// (j.gruber 2011-08-09 09:49) - PLID 44952 - THIS IS NOT USED, JUST LEAVE IN FOR NOW
CSqlFragment CFinancialCorrection::GenerateChargeAppliesByAppliesToChargeSql() 
{

	// (j.jones 2014-07-15 10:37) - PLID 62876 - ignore chargebacks, they remain solely on the original charge
	return CSqlFragment(" DECLARE @nOrigApplyID INT; \r\n "
		"  DECLARE rsApplies CURSOR LOCAL READ_ONLY FORWARD_ONLY FOR \r\n "
		"  SELECT AppliesT.ID FROM AppliesT WHERE AppliesT.DestID = @nChargeIDToFix  \r\n "
		"  AND AppliesT.ID NOT IN (SELECT ApplyID FROM @ApplyInsertions ApplyInsertions) \r\n "
		"  LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON AppliesT.SourceID = Chargebacks_PaymentQ.PaymentID \r\n "
		"  LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON AppliesT.SourceID = Chargebacks_AdjQ.AdjustmentID \r\n "
		"  WHERE Chargebacks_PaymentQ.PaymentID Is Null \r\n "
		"  AND Chargebacks_AdjQ.AdjustmentID Is Null \r\n "
		"  OPEN rsApplies \r\n "
		"  FETCH FROM rsApplies INTO @nOrigApplyID \r\n "
		"  WHILE @@FETCH_STATUS = 0 BEGIN \r\n "

		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		// (j.jones 2015-02-23 11:14) - PLID 61532 - ensured we do not exceed the description size limit
		// (j.jones 2015-02-24 10:30) - PLID 64936 - don't append APPLY if the existing description already has it
		" 	/*First, create the adjustment*/ \r\n "
		" 	DECLARE @nAdjLineItemID INT; \r\n "
		" 	INSERT INTO LineItemT (PatientID, Type, Amount, Description, Date, InputDate, InputName, LocationID) \r\n "
		" 	SELECT PatientID, 2, -1*AppliesT.Amount, \r\n"
		"	(CASE WHEN Right(Description, 6) <> ' APPLY' THEN Left(Description, 255-6) + ' APPLY' ELSE Description END), \r\n"
		"	Date, @dtInputDate, @strUserName, LocationID \r\n "
		" 	FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID  \r\n "
		" 	WHERE AppliesT.ID = @nOrigApplyID AND LineItemT.Deleted = 0; \r\n "
		" 	SET @nAdjLineItemID = SCOPE_IDENTITY() \r\n "

		" 	INSERT INTO PaymentsT (ID, ProviderID, InsuredPartyID)  \r\n "
		" 	SELECT @nAdjLineItemID, ProviderID, InsuredPartyID \r\n "
		" 	FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID \r\n "
		" 	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n "
		" 	WHERE AppliesT.ID = @nOrigApplyID AND LineItemT.Deleted = 0; \r\n\r\n"

		" 	/*now the apply*/\r\n "
		// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
		// (j.jones 2015-02-19 12:38) - PLID 64932 - this uses the void charge resp link
		" 	INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName)\r\n"
		" 		SELECT\r\n"
		"			@nAdjLineItemID, ChargeRespT.ChargeID, ChargeRespVoidLinkT.VoidChargeRespID, -1 * AppliesT.Amount, 0, @dtInputDate, @strUserName\r\n"
		" 		FROM AppliesT\r\n"
		"		LEFT JOIN @ChargeRespVoidLinkT ChargeRespVoidLinkT ON AppliesT.RespID = ChargeRespVoidLinkT.OrigChargeRespID\r\n"
		"		LEFT JOIN ChargeRespT ON ChargeRespVoidLinkT.OrigChargeRespID = ChargeRespT.ID\r\n"
		" 		WHERE AppliesT.ID = @nOrigApplyID\r\n\r\n"

		" 	DECLARE @nAdjApplyID INT\r\n"
		"	SET @nAdjApplyID = SCOPE_IDENTITY()\r\n\r\n"

		"   INSERT INTO @ApplyInsertions (ApplyID) VALUES (@nAdjApplyID)\r\n\r\n"

		" 	/*loop for the details*/ \r\n "
		" 	DECLARE @nOrigApplyDetailID INT;	 \r\n "
		" 	DECLARE rsApplyDetails CURSOR LOCAL READ_ONLY FORWARD_ONLY FOR \r\n "
		" 	SELECT ID FROM ApplyDetailsT WHERE ApplyID = @nOrigApplyID \r\n "
		" 	OPEN rsApplyDetails  \r\n "
		" 	FETCH FROM rsApplyDetails INTO @nOrigApplyDetailID \r\n "
		" 	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
		// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
		" 		INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount)  \r\n "
		" 			SELECT @nAdjApplyID, ChargeRespDetailVoidLinkT.VoidChargeRespDetailID, -1 * Amount \r\n "
		" 			FROM ApplyDetailsT LEFT JOIN @ChargeRespDetailVoidLinkT ChargeRespDetailVoidLinkT ON ApplyDetailsT.DetailID = ChargeRespDetailVoidLinkT.OrigChargeRespDetailID  \r\n "
		" 			WHERE ApplyDetailsT.ID = @nOrigApplyID; \r\n\r\n"

		" 		FETCH FROM rsApplyDetails INTO @nOrigApplyDetailID \r\n "
		" 	END \r\n "
		" 	CLOSE rsApplyDetails \r\n "
		" 	DEALLOCATE rsApplyDetails \r\n "
		" 	 \r\n "
		" 	FETCH FROM rsApplies INTO @nOrigApplyID \r\n "
		" END \r\n "
		" CLOSE rsApplies  \r\n "
		" DEALLOCATE rsApplies \r\n ");	


}
// (j.gruber 2011-08-09 09:49) - PLID 44952
CSqlFragment CFinancialCorrection::GenerateChargeAppliesByChargeRespSql() 
{

	return CSqlFragment(" DECLARE @nApNewChargeRespID INT; \r\n "
		" DECLARE @nApOrigChargeRespID INT; \r\n "		
		" DECLARE @cyApAmount money; \r\n "
		" DECLARE @nApChargeID INT; \r\n "
		" DECLARE @strApChargeDescription nVarChar(255); \r\n "
		"  DECLARE rsChargeRespsLink CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
		// (j.gruber 2011-06-02 16:55) - PLID 44848- add special descriptions
		// (j.jones 2015-02-19 12:38) - PLID 64932 - this uses the void charge resp link
		"  SELECT ChargeRespVoidLinkT.VoidChargeRespID, OrigChargeRespT.ID, VoidChargeRespT.Amount AS Amount, VoidChargeRespT.ChargeID, OrigLineItemT.Description \r\n "
		"  FROM @ChargeRespVoidLinkT ChargeRespVoidLinkT \r\n"
		"  LEFT JOIN ChargeRespT OrigChargeRespT ON ChargeRespVoidLinkT.OrigChargeRespID = OrigChargeRespT.ID \r\n "
		"  LEFT JOIN LineItemT OrigLineItemT ON OrigchargeRespT.ChargeID = OrigLineItemT.ID \r\n "
		"  LEFT JOIN ChargeRespT VoidChargeRespT ON ChargeRespVoidLinkT.VoidChargeRespID = VoidChargeRespT.ID \r\n "
		"  UNION \r\n "
		"  SELECT ChargeRespT.ID, ChargeRespT.ID, ChargeRespT.Amount, ChargeRespT.ChargeID, LineItemT.Description FROM @ChargeRespVoidLinkT ChargeRespVoidLinkT LEFT JOIN \r\n "
		"  ChargeRespT ON ChargeRespVoidLinkT.OrigChargeRespID = ChargeRespT.ID \r\n "
		"  LEFT JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID \r\n "
		"  OPEN rsChargeRespsLink \r\n "
		"  FETCH FROM rsChargeRespsLink INTO @nApNewChargeRespID, @nApOrigChargeRespID, @cyApAmount, @nApChargeID, @strApChargeDescription \r\n "
		"  WHILE @@FETCH_STATUS = 0 BEGIN \r\n "

		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		" 	/*First, create the adjustment*/ \r\n "
		" 	DECLARE @nChAdjLineItemID INT; \r\n "	
		// (j.jones 2015-02-23 11:14) - PLID 61532 - ensured we do not exceed the description size limit
		// (j.jones 2015-02-24 10:30) - PLID 64936 - don't prepend 'Voided Charge Adjustment' if the existing description already has it
		" 	INSERT INTO LineItemT (PatientID, Type, Amount, Description, Date, InputDate, InputName, LocationID) \r\n "
		" 	SELECT PatientID, 2, @cyApAmount, \r\n"
		"	Left((CASE WHEN Left(@strApChargeDescription, Len('Voided Charge Adjustment ')) <> 'Voided Charge Adjustment ' THEN 'Voided Charge Adjustment ' + @strApChargeDescription ELSE @strApChargeDescription END), 255), \r\n"
		"	Date, @dtInputDate, @strUserName, LocationID \r\n "
		" 	FROM LineItemT  \r\n "
		" 	WHERE LineItemT.Deleted = 0 AND LineItemT.ID = @nApChargeID; \r\n "
		" 	SET @nChAdjLineItemID = SCOPE_IDENTITY() \r\n "

		// (j.gruber 2011-08-08 11:41) - PLID 45596 - add category
		" 	INSERT INTO PaymentsT (ID, ProviderID, InsuredPartyID, PaymentGroupID)  \r\n "
		" 	SELECT @nChAdjLineItemID, DoctorsProviders, CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END, @nVoidChargeCat \r\n "
		" 	FROM ChargeRespT INNER JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID \r\n "
		"   INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n "
		" 	WHERE LineItemT.Deleted = 0 AND ChargeRespT.ID = @nApNewChargeRespID; \r\n "
		"   INSERT INTO @LineItemAdjQ (AdjustmentID) VALUES (@nChAdjLineItemID)\r\n\r\n"

		" 	/*now the apply*/\r\n"
		// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
		" 	INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName)\r\n"
		" 		VALUES (@nChAdjLineItemID, @nApChargeID, @nApNewChargeRespID, @cyApAmount, 0, @dtInputDate, @strUserName)\r\n"

		" 	DECLARE @nChAdjApplyID INT\r\n"
		" 	SET @nChAdjApplyID = SCOPE_IDENTITY()\r\n\r\n"

		"   INSERT INTO @ApplyInsertions (ApplyID) VALUES (@nChAdjApplyID)\r\n\r\n"

		" 	/*loop for the details*/ \r\n "
		" 	DECLARE @nApOrigChargeRespDetailID INT; \r\n "
		" 	DECLARE @nApNewChargeRespDetailID INT; \r\n "
		"   DECLARE @cyApNewChargeRespDetailAmount money; \r\n "
		" 	DECLARE rsChargeRespDetailLinks CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
		
		" 	SELECT ChargeRespDetailT.ID, ChargeRespDetailT.ID, ChargeRespDetailT.Amount FROM ChargeRespDetailT  \r\n "
		"   WHERE ChargeRespDetailT.ChargeRespID = @nApNewChargeRespID  \r\n "
		" 	OPEN rsChargeRespDetailLinks \r\n "
		" 	FETCH FROM rsChargeRespDetailLinks INTO @nApOrigChargeRespDetailID, @nApNewChargeRespDetailID, @cyApNewChargeRespDetailAmount \r\n "
		" 	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
		// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
		" 		INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount)  \r\n "
		" 		VALUES (@nChAdjApplyID, @nApNewChargeRespDetailID, @cyApNewChargeRespDetailAmount) \r\n\r\n"

		" 	FETCH FROM rsChargeRespDetailLinks INTO @nApOrigChargeRespDetailID, @nApNewChargeRespDetailID, @cyApNewChargeRespDetailAmount \r\n "
		" 	END \r\n "
		" 	CLOSE rsChargeRespDetailLinks \r\n "
		" 	DEALLOCATE rsChargeRespDetailLinks \r\n "
		" 	 \r\n "
		"  FETCH FROM rsChargeRespsLink INTO @nApNewChargeRespID, @nApOrigChargeRespID, @cyApAmount, @nApChargeID, @strApChargeDescription \r\n "
		" END \r\n "
		" CLOSE rsChargeRespsLink  \r\n "
		" DEALLOCATE rsChargeRespsLink\r\n ");
		
}

// (j.gruber 2011-08-09 09:49) - PLID 44952
CSqlFragment CFinancialCorrection::GenerateChargeCorrectionSql() 
{

	CString strSql;
	// (j.dinatale 2011-09-29 11:44) - PLID 44380 - Copy GiftID from LineItemT
	// (j.jones 2011-12-12 12:16) - PLID 46088 - added ChargesT.Calls
	// (j.jones 2012-01-17 11:21) - PLID 47537 - added ChargesT.EMRChargeID
	// (d.singleton 2012-05-24 10:05) - PLID 48152 added ChargesT.SkillCode
	// (r.gonet 2015-03-27 18:58) - PLID 65279 - Copy the GCValue as well.
	strSql.Format(
		"		 \r\n "
		"   SET @nLineID = COALESCE((SELECT Max(COALESCE(LineID, 0)) + 1 FROM ChargesT WHERE BillID = CASE WHEN @bUseNewBill = 1 THEN @nBillIDToUse ELSE (SELECT BillID FROM ChargesT WHERE ID = @nChargeIDToFix) END), 1); \r\n "

		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		// (j.jones 2015-02-24 10:30) - PLID 64936 - don't prepend @strDescription if the existing description already has it
		"	INSERT INTO LineItemT (PatientID, Type, Amount, Description, Date, InputDate, InputName, LocationID, GiftID, GCValue) \r\n "
		"	SELECT PatientID, Type, Amount, \r\n"
		"	Left((CASE WHEN Left(Description, Len(@strDescription)) <> @strDescription THEN @strDescription + Description ELSE Description END), 255), \r\n"
		"	Date, @dtInputDate, @strUserName, LocationID, GiftID, (CASE WHEN @bNegated = 1 THEN -1*LineItemT.GCValue ELSE LineItemT.GCValue END) AS GCValue FROM LineItemT  \r\n "
		"	WHERE ID = @nChargeIDToFix AND LineItemT.Deleted = 0 \r\n "
		"	SET @nNewChargeID = SCOPE_IDENTITY() \r\n "

		// (j.gruber 2014-02-28 13:58) - PLID 61108 - take out whichCodes
		// (j.jones 2014-04-23 09:51) - PLID 61836 - added the ReferringProviderID, OrderingProviderID, SupervisingProviderID
		"	\r\n "
		"	INSERT INTO ChargesT (ID, BillID, ServiceId, ItemCode, ItemSubCode, Category, SubCategory,  \r\n "
		"	CPtModifier, CPTModifier2, TaxRate, Quantity, DoctorsProviders,  \r\n "
		"	ServiceDateFrom, ServiceDateTo, ServiceType, EPSDT, COB, OthrBillFee, LineID,  \r\n "
		"	TaxRate2, SuperBillID, PatCoordID, CPTModifier3, CPTModifier4, ServiceLocationID,  \r\n "
		"	CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4,  \r\n "
		"	Batched, PackageChargeRefID, PackageQtyRemaining, OriginalPackageQtyRemaining,  \r\n "
		"	NDCCode, AppointmentID, DrugUnitPrice, DrugUnitTypeQual, Allowable,  \r\n "
		"	IsEmergency, ClaimProviderID, GlassesOrderServiceID, Calls, EMRChargeID, SkillCode, \r\n "
		"	ReferringProviderID, OrderingProviderID, SupervisingProviderID) \r\n "
		"	SELECT @nNewChargeID, CASE WHEN @bUseNewBill = 1 THEN @nBillIDToUse ELSE BillID END, ServiceID, ItemCode, ItemSubCode, Category, SubCategory,  \r\n "
		"	CPTModifier, CPTModifier2, TaxRate, CASE WHEN @bNegated = 1 THEN -1 * Quantity ELSE Quantity END, DoctorsProviders,  \r\n "
		"	ServiceDateFrom, ServiceDateTo, ServiceType, EPSDT, COB, OthrBillFee, @nLineID,  \r\n "
		"	TaxRate2, SuperBillID, PatCoordID, CPTModifier3, CPTModifier4, ServiceLocationID,  \r\n "
		"	CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4,  \r\n "
		"	Batched, PackageChargeRefID, PackageQtyRemaining, OriginalPackageQtyRemaining,  \r\n "
		"	NDCCode, AppointmentID, DrugUnitPrice, DrugUnitTypeQual, Allowable,  \r\n "
		"	IsEmergency, ClaimProviderID, GlassesOrderServiceID, Calls, EMRChargeID, SkillCode, \r\n "
		"	ReferringProviderID, OrderingProviderID, SupervisingProviderID \r\n"
		"	FROM ChargesT WHERE ID = @nChargeIDToFix \r\n "
		"	\r\n "
		"	DECLARE rsChargeResps CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR  \r\n "
		" 	SELECT ID, InsuredPartyID, Amount FROM ChargeRespT WHERE ChargeID = @nChargeIDToFix \r\n "
		" \r\n "
		"	OPEN rsChargeResps; \r\n "
		"	FETCH FROM rsChargeResps INTO @nChargeRespID, @nInsuredPartyID, @cyAmount \r\n "
		"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
		// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
		" 		INSERT INTO ChargeRespT (ChargeID, InsuredPartyID, Amount) \r\n "
		" 			VALUES (@nNewChargeID, @nInsuredPartyID, CASE WHEN @bNegated = 1 THEN -1 * @cyAmount ELSE @cyAmount END); \r\n "
		" 		SET @nNewChargeRespID = SCOPE_IDENTITY() \r\n "
		" 		 \r\n "
		// (j.jones 2015-02-19 13:05) - PLID 64932 - previously this only tracked voided charge resps,
		// now we track voided ones and corrected ones, in separate tables
		"       IF @bNegated = 1 BEGIN \r\n "
		"	       INSERT INTO @ChargeRespVoidLinkT (OrigChargeRespID, VoidChargeRespID) VALUES (@nChargeRespID, @nNewChargeRespID); \r\n "
		"		END \r\n "
		"		ELSE BEGIN \r\n "
		"	       INSERT INTO @ChargeRespNewLinkT (OrigChargeRespID, NewChargeRespID) VALUES (@nChargeRespID, @nNewChargeRespID); \r\n "
		"		END \r\n "
		"	 	/*now for the detail*/ \r\n "
		" 		DECLARE rsChargeRespDetails CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
		" 		SELECT ID FROM ChargeRespDetailT WHERE ChargeRespID = @nChargeRespID \r\n "

		" 		 \r\n "
		" 			OPEN rsChargeRespDetails; \r\n "
		"	 		FETCH FROM rsChargeRespDetails INTO @nChargeRespDetailID \r\n "
		" 			WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
		// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
		"	 			INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date)	 \r\n "
		" 				SELECT @nNewChargeRespID, CASE WHEN @bNegated = 1 THEN -1 * Amount ELSE Amount END, Date FROM ChargeRespDetailT WHERE ID = @nChargeRespDetailID \r\n "
		" 				SET @nNewChargeRespDetailID = SCOPE_IDENTITY() \r\n "
		// (j.jones 2015-02-19 13:05) - PLID 64932 - previously this only tracked voided charge resps,
		// now we track voided ones and corrected ones, in separate tables
		"				IF @bNegated = 1 BEGIN \r\n "
		"					INSERT INTO @ChargeRespDetailVoidLinkT (OrigChargeRespDetailID, VoidChargeRespDetailID) VALUES (@nChargeRespDetailID, @nNewChargeRespDetailID); \r\n  "
		"				END \r\n "
		"				ELSE BEGIN \r\n "
		"					INSERT INTO @ChargeRespDetailNewLinkT (OrigChargeRespDetailID, NewChargeRespDetailID) VALUES (@nChargeRespDetailID, @nNewChargeRespDetailID); \r\n  "
		"				END \r\n "
		" 				FETCH FROM rsChargeRespDetails INTO @nChargeRespDetailID \r\n "
		" 			END /*rschargeRespDetails*/ \r\n "
		"			CLOSE rsChargeRespDetails \r\n "
		"			DEALLOCATE rsChargeRespDetails \r\n "
		" 		\r\n "
		" 		FETCH FROM rsChargeResps INTO @nChargeRespID, @nInsuredPartyID, @cyAmount \r\n "
		"	END	 /*rsChargeresps*/	 \r\n "
		"   CLOSE rsChargeResps \r\n "
		"   DEALLOCATE rsChargeResps \r\n "
		" 	 \r\n "
		// (j.gruber 2011-08-03 10:55) - PLID 44835 - need to negate discounts also
		"	/*Discounts*/ \r\n "
		"	INSERT INTO ChargeDiscountsT (ChargeID, PercentOff, Discount, CouponID, DiscountCategoryID, CustomDiscountDesc, Deleted, DeletedBy, DeleteDate) \r\n "
		"	SELECT @nNewChargeID, PercentOff, CASE WHEN @bNegated = 1 THEN -1 * Discount ELSE Discount END, CouponID, DiscountCategoryID, CustomDiscountDesc, Deleted, DeletedBy, DeleteDate \r\n "
		"	FROM ChargeDiscountsT WHERE ChargeID = @nChargeIDToFix \r\n "
		" \r\n  "

		// (j.gruber 2014-02-28 13:50) - PLID 61108 - new whichCodes structure
		" /*WhichCodes*/ \r\n "
		" INSERT INTO ChargeWhichCodesT (ChargeID, BillDiagCodeID) \r\n"
		" SELECT @nNewChargeID, NewBillDiagCodesT.ID "
		" FROM ChargeWhichCodesT "
		" INNER JOIN ChargesT ON ChargeWhichCodesT.ChargeID = ChargesT.ID \r\n "
		" INNER JOIN BillDiagCodeT BDCToCorrect ON ChargeWhichCodesT.BillDiagCodeID = BDCToCorrect.ID \r\n "
		" INNER JOIN BillDiagCodeT NewBillDiagCodesT ON (BDCToCorrect.ICD9DiagID IS NULL OR BDCToCorrect.ICD9DiagID = NewBillDiagCodesT.ICD9DiagID) \r\n "
		" AND (BDCToCorrect.ICD10DiagID IS NULL OR BDCToCorrect.ICD10DiagID = NewBillDiagCodesT.ICD10DiagID) \r\n "
		" WHERE ChargeWhichCodesT.ChargeID = @nChargeIDToFix AND NewBillDiagCodesT.BillID = (CASE WHEN @bUseNewBill = 1 THEN @nBillIDToUse ELSE ChargesT.BillID END) \r\n "
		
		);
			
	
	return CSqlFragment(strSql);

}
// (j.gruber 2011-06-24 17:18) - PLID lots - adding constraint fixes
CSqlFragment CFinancialCorrection::GenerateChargeContraintsSql() 
{

	//we are already inside a If bCorrect, so we don't have to worry about that
	CString strSql;	

	// (j.gruber 2011-06-22 11:03) - PLID 45601 - NoteInfoT.LineItemID - Replace
	strSql += " UPDATE NoteInfoT SET LineItemID = @nNewChargeID WHERE LineItemID = @nChargeIDToFix; \r\n ";
	// (j.gruber 2011-06-27 11:41) - PLID 44863 - ChargeAllowablesT
	// (j.jones 2011-11-21 16:47) - PLID 45562 - if undoing, we may already have these allowables,
	// if so we want to delete the old allowables, and keep the new
	strSql += " DELETE FROM ChargeAllowablesT WHERE ChargeID = @nNewChargeID AND InsuredPartyID IN (SELECT InsuredPartyID FROM ChargeAllowablesT WHERE ChargeID = @nChargeIDToFix); \r\n";
	strSql += " INSERT INTO ChargeAllowablesT (ChargeID, InsuredPartyID, Allowable, InputDate, EntryMethod) "
		" SELECT @nNewChargeID, InsuredPartyID, Allowable, @dtInputDate, EntryMethod FROM ChargeAllowablesT "
 		" WHERE ChargeID = @nChargeIDToFix; \r\n";
	// (j.jones 2011-12-19 15:39) - PLID 43925 - supported ChargeCoinsuranceT, behaves identically to ChargeAllowablesT
	strSql += " DELETE FROM ChargeCoinsuranceT WHERE ChargeID = @nNewChargeID AND InsuredPartyID IN (SELECT InsuredPartyID FROM ChargeCoinsuranceT WHERE ChargeID = @nChargeIDToFix); \r\n";
	strSql += " INSERT INTO ChargeCoinsuranceT (ChargeID, InsuredPartyID, Deductible, Coinsurance, InputDate, EntryMethod) "
		" SELECT @nNewChargeID, InsuredPartyID, Deductible, Coinsurance, @dtInputDate, EntryMethod FROM ChargeCoinsuranceT "
 		" WHERE ChargeID = @nChargeIDToFix; \r\n";
	// (j.gruber 2011-06-27 15:12) - PLID 44863 - ChargedAllocationDetailsT
	strSql += " UPDATE ChargedAllocationDetailsT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix; \r\n ";
	// (j.gruber 2011-06-27 15:44) - PLID 45601
	strSql += " UPDATE ClaimHistoryDetailsT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix; \r\n ";
	// (j.gruber 2011-06-28 11:09) - PLID 45600
	strSql += " UPDATE FinanceChargesT SET OriginalChargeID = @nNewChargeID WHERE OriginalChargeID = @nChargeIDToFix \r\n; ";
	// (j.gruber 2011-06-28 11:09) - PLID 45600
	strSql += " UPDATE FinanceChargesT SET FinanceChargeID = @nNewChargeID WHERE FinanceChargeID = @nChargeIDToFix \r\n; ";
	// (j.gruber 2011-06-28 11:09) - PLID 44597
	strSql += " UPDATE ReturnedProductsT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix \r\n; ";
	// (j.gruber 2011-06-28 11:33) - PLID 45600 
	strSql += " UPDATE RewardHistoryT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix \r\n; ";
	// (j.gruber 2011-08-05 16:25) - PLID 44863
	strSql += " UPDATE ChargedProductItemsT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix \r\n; ";	
	// (r.gonet 08/06/2014) - PLID 63098 - Handle the lab test codes linked to charges
	strSql += " UPDATE ChargeLabTestCodesT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix \r\n; ";
	// (j.jones 2015-10-20 09:49) - PLID 67385 - supported BatchPaymentCapitationDetailsT
	strSql += " UPDATE BatchPaymentCapitationDetailsT SET ChargeID = @nNewChargeID WHERE ChargeID = @nChargeIDToFix \r\n; ";

	return CSqlFragment(strSql);
}

CSqlFragment CFinancialCorrection::GeneratePaymentContraintsSql() 
{

	//we are already inside a If bCorrect, so we don't have to worry about that
	CString strSql;

	// (j.dinatale 2011-09-13 12:24) - PLID 45277 - Replace
	strSql += " UPDATE ProcInfoPaymentsT SET PayID = @iNewLineItemID WHERE PayID = @nPaymentIDToFix; \r\n ";
	// (j.gruber 2011-06-22 11:03) - PLID 45601 - NoteInfoT.LineItemID - Replace
	strSql += " UPDATE NoteInfoT SET LineItemID = @iNewLineItemID WHERE LineItemID = @nPaymentIDToFix; \r\n ";
	// (j.gruber 2011-06-28 11:26) - PLID 44597 
	strSql += " UPDATE ReturnedProductsT SET FinAdjID = @iNewLineItemID WHERE FinAdjID = @nPaymentIDToFix; \r\n ";
	// (j.gruber 2011-06-28 11:27) - PLID 45597
	strSql += " UPDATE ReturnedProductsT SET FinRefundID = @iNewLineItemID WHERE FinRefundID = @nPaymentIDToFix; \r\n ";
	// (j.jones 2015-09-30 09:23) - PLID 67162 - update the Card Connect tables - refunds tie to CardConnect_CreditTransactionT
	// so we need to make a copy of that first, move the refund, delete the old transaction record
	strSql += "INSERT INTO CardConnect_CreditTransactionT (ID, UserID, Date, Amount, AccountID, Token, AuthRetRef, AuthAmount, AuthCode, WasSignedElectronically) \r\n "
		"	SELECT @iNewLineItemID, UserID, Date, Amount, AccountID, Token, AuthRetRef, AuthAmount, AuthCode, WasSignedElectronically \r\n "
		"	FROM CardConnect_CreditTransactionT WHERE ID = @nPaymentIDToFix \r\n ";
	strSql += " UPDATE CardConnect_CreditTransactionRefundT SET ID = @iNewLineItemID WHERE ID = @nPaymentIDToFix; \r\n ";
	// (j.jones 2015-09-30 09:23) - PLID 67162 - We also need to handle cases where the refund itself is being void and corrected
	strSql += " UPDATE CardConnect_CreditTransactionRefundT SET RefundingCreditTransactionID = @iNewLineItemID WHERE RefundingCreditTransactionID = @nPaymentIDToFix; \r\n ";
	strSql += " DELETE FROM CardConnect_CreditTransactionT WHERE ID = @nPaymentIDToFix; \r\n ";
	

	return CSqlFragment(strSql);
}

// (j.dinatale 2011-05-26 17:58) - PLID 44810
CSqlFragment CFinancialCorrection::CorrectPayments()
{

	// (j.gruber 2011-06-02 17:23) - PLID 44850 - add Correct	
	CSqlFragment sqlFrag;
	
	for (int i = 0; i < m_aryPaymentCorrections.GetSize(); i++) {
		PaymentCorrection *pPayment = ((PaymentCorrection*)m_aryPaymentCorrections.GetAt(i));
		sqlFrag += GeneratePaymentInsertSql(pPayment);
	}

	//first we need to create our negative charges
	sqlFrag += GeneratePaymentCorrectionSql();	

	return sqlFrag;
}

// (j.dinatale 2011-05-26 17:58) - PLID 44810
CSqlFragment CFinancialCorrection::GeneratePaymentInsertSql(PaymentCorrection *pPayment) 
{
	// (j.jones 2012-04-23 16:20) - PLID 48032 - supported BatchPaymentID
	_variant_t varTakebackIntoBatchPaymentID = g_cvarNull;
	if(pPayment->nTakebackIntoBatchPaymentID != -1) {
		varTakebackIntoBatchPaymentID = (long)pPayment->nTakebackIntoBatchPaymentID;
	}

	// (j.dinatale 2011-10-27 17:54) - PLID 46152 - no longer grab any line items applied to our payments in here, since charges expects
	//		those to be collected in GeneratePaymentCorrectionSql
	// (j.dinatale 2011-08-04 16:14) - PLID 44810 - also ensure that something doesnt end up in the table twice
	// (j.gruber 2011-06-02 17:23) - PLID 44850 - add Correct
	// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
	return CSqlFragment(
		"IF NOT EXISTS(SELECT 1 FROM @PaymentCorrections WHERE ID = {INT}) BEGIN \r\n"
		"	INSERT INTO @PaymentCorrections (ID, \r\n"
		"		TakebackIntoBatchPaymentID, TakebackBatchPaymentCheckNo, TakebackBatchPaymentDate, \r\n"
		"		UserName, UserID, Correct) \r\n "
		"	VALUES  ({INT}, \r\n"
		"		{VT_I4}, {STRING}, {STRING}, \r\n"
		"		{STRING}, {INT}, {BIT}) \r\n"
		"END\r\n\r\n",
		pPayment->nPaymentID,
		pPayment->nPaymentID,
		varTakebackIntoBatchPaymentID, pPayment->strTakebackBatchPaymentCheckNo, pPayment->strTakebackBatchPaymentDate,
		pPayment->strUserName, pPayment->nUserID, pPayment->bCorrect);
}

// (j.dinatale 2011-05-26 17:58) - PLID 44810
CSqlFragment CFinancialCorrection::GeneratePaymentCorrectionSql()
{
	// So the order to this is going to be the following:
	//		1. Create the correction adjustment that will negate the original payment (either applied or unapplied)
	//		2. Create the correction adjustment that will be used as the new "payment" which can be applied towards things
	//		3. If the payment is applied, recreate all the applies as they were just with the sourceID as the counter payment so that way all applies are negated
	//				individually.
	//		4. Otherwise, we need to apply the counter adjustment to the unapplied payment to negate it
	//		5. Need to make an entry in LineItemCorrectionsT to reflect that the payment was corrected and set each of the fields accordingly

	// (j.jones 2012-04-23 16:20) - PLID 48032 - supported BatchPaymentID
	CSqlFragment sqlFrag;
	sqlFrag += 
		// (j.dinatale 2011-10-27 17:54) - PLID 46152 - need to get all the applied line items down here instead, we can get them by doing some
		//		nifty query work
		// (j.jones 2014-07-15 10:37) - PLID 62876 - ignore chargebacks, they remain solely on the original charge
		// (j.jones 2015-02-19 15:01) - PLID 64999 - Always correct applies to payments. If we're voiding a payment, we're not voiding its applies.
		"WHILE( \r\n"
		"	EXISTS ( \r\n"
		"		SELECT TOP 1 1 \r\n"
		"			FROM AppliesT \r\n"
		"				LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON AppliesT.SourceID = OrigLineItemsT.OriginalLineItemID \r\n"
		"				LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON AppliesT.SourceID = VoidingLineItemsT.VoidingLineItemID \r\n"
		"				LEFT JOIN @PaymentCorrections CurrCorrections ON AppliesT.SourceID = CurrCorrections.ID  \r\n"
		"				INNER JOIN @PaymentCorrections AssocCorrections ON AppliesT.DestID = AssocCorrections.ID \r\n"
		"				LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON AppliesT.SourceID = Chargebacks_PaymentQ.PaymentID \r\n "
		"				LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON AppliesT.SourceID = Chargebacks_AdjQ.AdjustmentID \r\n "
		"			WHERE (OrigLineItemsT.ID IS NULL AND VoidingLineItemsT.ID IS NULL) AND CurrCorrections.ID IS NULL \r\n"
		"			AND Chargebacks_PaymentQ.PaymentID Is Null \r\n "
		"			AND Chargebacks_AdjQ.AdjustmentID Is Null \r\n "
		"			) \r\n"
		")BEGIN \r\n"
		// (j.jones 2015-11-04 16:22) - PLID 57416 - fixed to handle cases where one payment was applied twice
		"	INSERT INTO @PaymentCorrections (ID, TakebackIntoBatchPaymentID, UserName, UserID, Correct)  \r\n"
		"		SELECT AppliesQ.SourceID, NULL, AssocCorrections.Username, AssocCorrections.UserID, Convert(bit, 1) \r\n"
		"		FROM (SELECT SourceID, DestID FROM AppliesT GROUP BY SourceID, DestID) AS AppliesQ  \r\n"
		"			LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON AppliesQ.SourceID = OrigLineItemsT.OriginalLineItemID \r\n"
		"			LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON AppliesQ.SourceID = VoidingLineItemsT.VoidingLineItemID \r\n"
		"			LEFT JOIN @PaymentCorrections CurrCorrections ON AppliesQ.SourceID = CurrCorrections.ID  \r\n"
		"			INNER JOIN @PaymentCorrections AssocCorrections ON AppliesQ.DestID = AssocCorrections.ID \r\n"
		"			LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON AppliesQ.SourceID = Chargebacks_PaymentQ.PaymentID \r\n "
		"			LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON AppliesQ.SourceID = Chargebacks_AdjQ.AdjustmentID \r\n "
		"		WHERE (OrigLineItemsT.ID IS NULL AND VoidingLineItemsT.ID IS NULL) AND CurrCorrections.ID IS NULL \r\n"
		"		AND Chargebacks_PaymentQ.PaymentID Is Null \r\n "
		"		AND Chargebacks_AdjQ.AdjustmentID Is Null \r\n "
		"		GROUP BY SourceID, AssocCorrections.Username, AssocCorrections.UserID, AssocCorrections.Correct \r\n"
		"END \r\n\r\n"

		"DECLARE @iPayAdjLineItemID INT; \r\n"
		"DECLARE @iNewLineItemID INT; \r\n"
		"DECLARE @iNextApplyID INT;  \r\n"
		"DECLARE @mAmountOfApply MONEY; \r\n"
		"DECLARE @iNextUniquePayID INT; \r\n\r\n"

		// (j.gruber 2011-06-02 17:25) - PLID 44850 - added correct
		// (j.jones 2015-08-17 15:17) - PLID 59210 - sort by OrderIndex, which is the table identity, to enforce that
		// we correct payments in the same order the table was filled
		"DECLARE rsPaymentCorrections CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR  \r\n "
		"	SELECT ID, TakebackIntoBatchPaymentID, TakebackBatchPaymentCheckNo, TakebackBatchPaymentDate, UserName, UserID, Correct \r\n"
		"	FROM @PaymentCorrections PaymentCorrectionsT \r\n "
		"	ORDER BY OrderIndex \r\n"
		"DECLARE @nPaymentIDToFix INT; \r\n "
		// (j.jones 2012-04-23 16:20) - PLID 48032 - supported BatchPaymentID
		"DECLARE @nTakebackIntoBatchPaymentID INT; \r\n "
		// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
		"DECLARE @strTakebackBatchPaymentCheckNo nvarchar(50); \r\n"
		"DECLARE @strTakebackBatchPaymentDate nvarchar(50); \r\n"
		"DECLARE @nPayUserID INT; \r\n "
		"DECLARE @strPayUserName nVarchar(50) \r\n "
		// (j.gruber 2011-06-02 17:18) - PLID 44850 - added Correct
		"DECLARE @bPayCorrect BIT \r\n "

		"OPEN rsPaymentCorrections; \r\n "
		"FETCH FROM rsPaymentCorrections INTO @nPaymentIDToFix, @nTakebackIntoBatchPaymentID, @strTakebackBatchPaymentCheckNo, @strTakebackBatchPaymentDate, @strPayUserName, @nPayUserID, @bPayCorrect \r\n "
		"WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
		
		// (j.gruber 2011-06-02 16:55) - PLID 44848 - add special descriptions
		// (j.jones 2012-10-02 09:09) - PLID 52529 - if this is a reversal into a batch payment, add the batch payment date and check # to the description
		// (j.jones 2015-02-24 10:45) - PLID 64936 - moved the prefix logic to its own variable
		"	DECLARE @PaymentDescriptionPrefix nvarchar(255); \r\n"
		"	SET @PaymentDescriptionPrefix = IsNull((\r\n"
		"		SELECT \r\n"
		"		CASE WHEN @nTakebackIntoBatchPaymentID Is Not Null THEN \r\n"
		"			'Insurance Reversal ' + CASE WHEN @strTakebackBatchPaymentDate <> '' THEN @strTakebackBatchPaymentDate + ' ' ELSE '' END \r\n"
		"			+ CASE WHEN @strTakebackBatchPaymentCheckNo <> '' THEN '(Check #' + @strTakebackBatchPaymentCheckNo + ') ' ELSE '' END \r\n"
		"		ELSE \r\n"
		"		'Voided ' + (CASE WHEN LineItemT.Type = 1 THEN 'Payment ' WHEN LineItemT.Type = 2 THEN 'Adjustment ' WHEN LineItemT.Type = 3 THEN 'Refund ' ELSE 'Item ' END) \r\n"
		"		END \r\n"
		"		FROM LineItemT WHERE LineItemT.ID = @nPaymentIDToFix \r\n"
		"	), '') \r\n"

		// (j.dinatale 2011-09-29 09:26) - PLID 44380 - Need to copy GiftID from LineItemsT
		// (j.dinatale 2011-09-13 15:50) - PLID 44810 - need to copy prepayment flag and batchpaymentID, no longer need to copy tips
		// (j.dinatale 2011-09-13 13:59) - PLID 45371 - do not copy the Deposited flag, DepositDate should be null, and DepositInputDate should be null as well
		// (j.dinatale 2011-08-26 09:10) - PLID 44810 - need to include paymethod, and change the new line item's type to a payment, also need to copy PaymentTipsT and need to copy PaymentsT.PayMethod too.
		// (j.dinatale 2011-08-04 17:13) - PLID 44810 - set PaymentsT.PayMethod to 0 for both the voiding and correcting line item because it was breaking banking
		// (j.gruber 2011-06-28 17:19) - took out quoteID and prepayment flags		
		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		// (s.dhole 9/29/2014 11:10 AM) - PLID 63756  will try to insert only left 255 chracter in LineItemT.Description
		// (j.jones 2015-02-24 10:30) - PLID 64936 - don't prepend anything to the description if the existing description already has that prefix
		// (r.gonet 2015-03-27 18:58) - PLID 65279 - Copy the GCValue as well, but negate it so it cancels out the original line item.
		"	INSERT INTO LineItemT (PatientID, Type, Amount, Description, Date, InputDate, InputName, LocationID, GiftID, GCValue) \r\n"
		"		SELECT PatientID, Type, -1 * Amount, \r\n"
		"		Left((CASE WHEN Left(Description, Len(@PaymentDescriptionPrefix)) <> @PaymentDescriptionPrefix THEN @PaymentDescriptionPrefix + Description ELSE Description END), 255), \r\n"
		"		Date, @dtInputDate, @strPayUserName, LocationID, GiftID, -1*GCValue FROM LineItemT WHERE LineItemT.ID = @nPaymentIDToFix \r\n"

		"	SET @iPayAdjLineItemID = SCOPE_IDENTITY() \r\n"

		// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
		// (r.gonet 2015-04-29 15:17) - PLID 65657 - Copy the RefundedFromGiftID
		"	INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
		"	SET @iNextUniquePayID = SCOPE_IDENTITY()\r\n"

		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
		"	INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PaymentGroup, Deposited, DepositDate, PayMethod, DepositInputDate, CashReceived, "
		"	PaymentUniqueID, GroupCodeID, ReasonCodeID, PrePayment, BatchPaymentID, RefundedFromGiftID, CCProcessType)  "
		"		SELECT @iPayAdjLineItemID, InsuredPartyID, ProviderID, PaymentGroupID, PaymentGroup, 0, NULL, PayMethod, NULL, CashReceived, "
		"		@iNextUniquePayID, GroupCodeID, ReasonCodeID, PrePayment, BatchPaymentID, RefundedFromGiftID, CCProcessType FROM PaymentsT WHERE ID = @nPaymentIDToFix \r\n"
		"	INSERT INTO PaymentPlansT (ID, CheckNo, BankNo, CheckAcctNo, CCNumber, CCHoldersName, CCExpDate, CCAuthNo, BankRoutingNum, SecurePAN, CreditCardID, KeyIndex) "
		"		SELECT @iPayAdjLineItemID, CheckNo, BankNo, CheckAcctNo, CCNumber, CCHoldersName, CCExpDate, CCAuthNo, BankRoutingNum, SecurePAN, CreditCardID, KeyIndex FROM PaymentPlansT WHERE ID = @nPaymentIDToFix \r\n"

		" IF @bPayCorrect = 1 BEGIN \r\n "

		// (j.gruber 2011-06-02 16:55) - PLID 44848 - add special descriptions
		// (j.jones 2015-02-24 10:45) - PLID 64936 - moved the prefix logic to its own variable
		"	SET @PaymentDescriptionPrefix = IsNull((\r\n"
		"		SELECT \r\n"
		"		'Corrected ' + (CASE WHEN LineItemT.Type = 1 THEN 'Payment ' WHEN LineItemT.Type = 2 THEN 'Adjustment ' WHEN LineItemT.Type = 3 THEN 'Refund ' ELSE 'Item ' END) \r\n"
		"		FROM LineItemT WHERE LineItemT.ID = @nPaymentIDToFix \r\n"
		"	), '') \r\n"
		
		// (j.dinatale 2011-09-29 09:26) - PLID 44380 - Need to copy GiftID from LineItemsT
		// (j.dinatale 2011-09-13 15:50) - PLID 44810 - need to copy prepayment flag and batchpaymentID, no longer need to copy tips
		// (j.dinatale 2011-09-13 13:59) - PLID 45371 - do not copy the Deposited flag, DepositDate should be null, and DepositInputDate should be null as well
		// (j.dinatale 2011-09-13 12:39) - PLID 45278 - need to copy quoteID
		// (j.dinatale 2011-08-26 09:10) - PLID 44810 - need to include paymethod, and change the new line item's type to a payment, also need to copy PaymentTipsT and need to copy PaymentsT.PayMethod too.		
		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		// (j.jones 2015-02-23 11:14) - PLID 61532 - ensured we do not exceed the description size limit
		// (j.jones 2015-02-24 10:30) - PLID 64936 - don't prepend anything to the description if the existing description already has that prefix
		// (r.gonet 2015-03-27 18:58) - PLID 65279 - Copy the GCValue as well.
		"	INSERT INTO LineItemT (PatientID, Type, Amount, Description, Date, InputDate, InputName, LocationID, GiftID, GCValue) \r\n"
		"		SELECT PatientID, Type, Amount, "
		"		Left((CASE WHEN Left(Description, Len(@PaymentDescriptionPrefix)) <> @PaymentDescriptionPrefix THEN @PaymentDescriptionPrefix + Description ELSE Description END), 255), \r\n"
		"		Date, @dtInputDate, @strPayUserName, LocationID, GiftID, GCValue FROM LineItemT WHERE ID = @nPaymentIDToFix \r\n"
		"	SET @iNewLineItemID = SCOPE_IDENTITY() \r\n"

		// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
		// (r.gonet 2015-04-29 15:17) - PLID 65657 - Copy the RefundedFromGiftID
		"	INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
		"	SET @iNextUniquePayID = SCOPE_IDENTITY()\r\n"

		// (j.jones 2015-09-30 08:58) - PLID 67157 - added CCProcessType
		"	INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PaymentGroup, Deposited, DepositDate, PayMethod, DepositInputDate, CashReceived, "
		"	PaymentUniqueID, GroupCodeID, ReasonCodeID, PrePayment, BatchPaymentID, QuoteID, RefundedFromGiftID, CCProcessType)  "
		"		SELECT @iNewLineItemID, InsuredPartyID, ProviderID, PaymentGroupID, PaymentGroup, 0, NULL, PayMethod, NULL, CashReceived, "
		"		@iNextUniquePayID, GroupCodeID, ReasonCodeID, PrePayment, BatchPaymentID, QuoteID, RefundedFromGiftID, CCProcessType FROM PaymentsT WHERE ID = @nPaymentIDToFix \r\n"
		"	INSERT INTO PaymentPlansT (ID, CheckNo, BankNo, CheckAcctNo, CCNumber, CCHoldersName, CCExpDate, CCAuthNo, BankRoutingNum, SecurePAN, CreditCardID, KeyIndex) "
		"		SELECT @iNewLineItemID, CheckNo, BankNo, CheckAcctNo, CCNumber, CCHoldersName, CCExpDate, CCAuthNo, BankRoutingNum, SecurePAN, CreditCardID, KeyIndex FROM PaymentPlansT WHERE ID = @nPaymentIDToFix \r\n";

		sqlFrag += GeneratePaymentContraintsSql();

		// (j.gruber 2011-06-02 17:26) - PLID 44850
		sqlFrag += " END \r\n "

			// (j.dinatale 2011-08-30 16:44) - PLID 45278 - Need to remove the PaymentsT.QuoteID of the old line item, the prepayment would not be able to be relinked
			//		to the quote anyways, since it is voided
			" UPDATE PaymentsT SET QuoteID = NULL WHERE ID = @nPaymentIDToFix; \r\n"

			////////////////////////////////////////////////////////////////////////////////////
			/// Begin adding voiding applies from the void adjustment.
			////////////////////////////////////////////////////////////////////////////////////

			// (j.armen 2013-07-01 10:54) - PLID 27387 - We'll now just keep track of the New ID's and Old ID's in a temp table
			"DECLARE @ApplyLink TABLE (NewID INT NOT NULL, OldID INT NOT NULL) \r\n"
			"DELETE FROM @ApplyLink \r\n"

			// (j.armen 2013-07-01 10:57) - PLID 57385 - Idenitate AppliesT
			// (j.armen 2013-07-01 10:54) - PLID 27387 - Add our new applies.  We will update the Amount to be correct in the next step.
			//	Be sure to save our map of new ID's to old ID's
			// (j.jones 2014-07-15 10:37) - PLID 62876 - ignore chargebacks, they remain solely on the original charge
			"	INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName) \r\n"
			"		OUTPUT inserted.ID, CONVERT(INT, inserted.Amount) INTO @ApplyLink \r\n"
			"		SELECT \r\n"
			"			@iPayAdjLineItemID, DestID, RespID, CONVERT(MONEY, ID), PointsToPayments, @dtInputDate, @strPayUserName \r\n"
			"		FROM AppliesT\r\n"
			"		LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON AppliesT.SourceID = Chargebacks_PaymentQ.PaymentID \r\n "
			"		LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON AppliesT.SourceID = Chargebacks_AdjQ.AdjustmentID \r\n "
			"		WHERE SourceID = @nPaymentIDToFix \r\n"
			"		AND Chargebacks_PaymentQ.PaymentID Is Null \r\n "
			"		AND Chargebacks_AdjQ.AdjustmentID Is Null \r\n\r\n"

			// (j.armen 2013-07-01 10:54) - PLID 27387 - Now update the Amount based on our map
			"	UPDATE NewApply \r\n"
			"		SET NewApply.Amount = (-1 * OldApply.Amount) \r\n"
			"	FROM @ApplyLink ApplyLink \r\n"
			"	INNER JOIN AppliesT NewApply ON ApplyLink.NewID = NewApply.ID \r\n"
			"	INNER JOIN AppliesT OldApply ON ApplyLink.OldID = OldApply.ID \r\n"

			// insert the apply details with negated amounts
			// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
			// (j.armen 2013-07-01 10:54) - PLID 27387 - We have a map of new details to old details.  Use that to insert instead
			"	INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount) \r\n"
			"		SELECT \r\n"
			"			ApplyLink.NewID, DetailID, -1 * Amount \r\n"
			"		FROM ApplyDetailsT \r\n"
			"		INNER JOIN @ApplyLink ApplyLink ON ApplyDetailsT.ApplyID = ApplyLink.OldID \r\n"

			// We need a new apply to "negate" the original unapplied payment (we need to apply the adj to it)
			// get the amount of the apply from the correction adjustment because thats going to be the amount of the apply
			"	SET @mAmountOfApply = "
			"	(SELECT LineItemT.Amount - SUM(COALESCE(AppliesT.Amount, 0)) FROM LineItemT LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"	WHERE LineItemT.ID = @nPaymentIDToFix GROUP BY LineItemT.Amount); \r\n"

			"	IF @mAmountOfApply <> 0 BEGIN \r\n"
			// (j.armen 2013-06-29 13:47) - PLID 57385 - Idenitate AppliesT
			// ok, insert all the information to AppliesT
			"		INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName) \r\n"
			"			VALUES (@iPayAdjLineItemID, @nPaymentIDToFix, NULL, -1 * @mAmountOfApply, 1, @dtInputDate, @strPayUserName) \r\n\r\n"

			"		SET @iNextApplyID = SCOPE_IDENTITY() \r\n\r\n"

			// and make a corresponding apply detail entry
			// (j.armen 2013-06-29 14:14) - PLID 57386 - Idenitate ApplyDetailsT
			"		INSERT INTO ApplyDetailsT (ApplyID, DetailID, Amount) \r\n"
			"			VALUES (@iNextApplyID, NULL, -1 * @mAmountOfApply) \r\n"

			"	END \r\n\r\n"

			////////////////////////////////////////////////////////////////////////////////////
			/// End adding voiding applies from the void adjustment.
			////////////////////////////////////////////////////////////////////////////////////

			////////////////////////////////////////////////////////////////////////////////////
			/// Begin adding applies from the corrected payment.
			////////////////////////////////////////////////////////////////////////////////////

			// (j.jones 2015-02-19 11:06) - PLID 64932 - if we are correcting a payment, we want
			// to rebuild all applies from it, using today's date as the input date
			" IF @bPayCorrect = 1 BEGIN \r\n "

			// Add our new applies.		
			// Ignore chargebacks, they remain solely on the original charge.

			"	DECLARE rsApplies CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR  \r\n "
			"		SELECT \r\n"
			"			AppliesT.ID AS OldApplyID, \r\n"
			// Remember that the destination may or may not have been corrected, so we may be reapplying to an uncorrected charge,
			// or a newly corrected charge from this correction process.
			"			Coalesce(DestCorrectionQ.NewLineItemID, AppliesT.DestID) AS NewDestID, \r\n"
			"			(CASE WHEN DestCorrectionQ.NewLineItemID Is Not Null THEN ChargeRespNewLinkT.NewChargeRespID ELSE AppliesT.RespID END) AS NewChargeRespID \r\n"
			"			FROM AppliesT \r\n"
			"			LEFT JOIN LineItemCorrectionsT AS DestCorrectionQ ON AppliesT.DestID = DestCorrectionQ.OriginalLineItemID \r\n"
			"			LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON AppliesT.SourceID = Chargebacks_PaymentQ.PaymentID \r\n"
			"			LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON AppliesT.SourceID = Chargebacks_AdjQ.AdjustmentID \r\n"
			"			LEFT JOIN @ChargeRespNewLinkT ChargeRespNewLinkT ON AppliesT.RespID = ChargeRespNewLinkT.OrigChargeRespID \r\n"
			"			WHERE AppliesT.SourceID = @nPaymentIDToFix \r\n"
			"			AND Chargebacks_PaymentQ.PaymentID Is Null \r\n"
			"			AND Chargebacks_AdjQ.AdjustmentID Is Null \r\n"
			// If the destination charge was voided, then we can only reapply the payment if there is a new charge (e.g. void & correct, not void only)
			// If the destination charge was never voided/corrected, then that's fine, we apply to the original charge.
			"			AND (DestCorrectionQ.ID Is Null OR DestCorrectionQ.NewLineItemID Is Not Null) \r\n\r\n"

			"	DECLARE @OldApplyID INT, @NewDestID INT, @NewChargeRespID INT; \r\n"

			"	OPEN rsApplies; \r\n "
			"	FETCH FROM rsApplies INTO @OldApplyID, @NewDestID, @NewChargeRespID \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "

			"		INSERT INTO AppliesT (SourceID, DestID, RespID, Amount, PointsToPayments, InputDate, InputName) \r\n"
			"		SELECT @iNewLineItemID, @NewDestID, @NewChargeRespID, AppliesT.Amount, AppliesT.PointsToPayments, @dtInputDate, @strPayUserName \r\n"
			"		FROM AppliesT \r\n"
			"		WHERE AppliesT.ID = @OldApplyID \r\n"

			"		SET @iNextApplyID = SCOPE_IDENTITY() \r\n\r\n"

			// insert the apply details
			"		INSERT INTO ApplyDetailsT (ApplyID, Amount, DetailID) \r\n"
			"			SELECT @iNextApplyID, ApplyDetailsT.Amount, \r\n"
			// Remember that the destination may or may not have been corrected, so we may be reapplying to an uncorrected charge,
			// or a newly corrected charge from this correction process.
			"			Coalesce(NewChargeRespDetailT.ID, ApplyDetailsT.DetailID) AS NewChargeRespDetailID\r\n"
			"			FROM ApplyDetailsT \r\n"
			"			LEFT JOIN @ChargeRespDetailNewLinkT ChargeRespDetailNewLinkT ON ApplyDetailsT.DetailID = ChargeRespDetailNewLinkT.OrigChargeRespDetailID \r\n"
			"			LEFT JOIN ChargeRespDetailT NewChargeRespDetailT ON ChargeRespDetailNewLinkT.NewChargeRespDetailID = NewChargeRespDetailT.ID \r\n"
			"			WHERE ApplyDetailsT.ApplyID = @OldApplyID \r\n"
			"			AND (NewChargeRespDetailT.ID Is Null OR NewChargeRespDetailT.ChargeRespID = @NewChargeRespID) \r\n"

			"		FETCH FROM rsApplies INTO @OldApplyID, @NewDestID, @NewChargeRespID \r\n "

			"	END \r\n"
			"	CLOSE rsApplies \r\n "
			"	DEALLOCATE rsApplies \r\n "
			" END \r\n\r\n"

			////////////////////////////////////////////////////////////////////////////////////
			/// End adding applies from the corrected payment.
			////////////////////////////////////////////////////////////////////////////////////

		// (j.jones 2012-04-23 16:20) - PLID 48032 - supported BatchPaymentID
		"	INSERT INTO LineItemCorrectionsT (OriginalLineItemID, VoidingLineItemID, NewLineItemID, BatchPaymentID, InputDate, InputUserID) \r\n"
		"		VALUES (@nPaymentIDToFix, @iPayAdjLineItemID, @iNewLineItemID, @nTakebackIntoBatchPaymentID, @dtInputDate, @nPayUserID) \r\n"
		"	INSERT INTO @AddedLineItemCorrections SELECT SCOPE_IDENTITY() \r\n\r\n"
		
		// (b.spivey, September 11, 2014) - PLID 63652 - Set the status to manual and break any existing links.
		"	UPDATE LP "
		"	SET LP.Manual = 1 "
		"	FROM LockboxPaymentT LP "
		"	INNER JOIN LockboxPaymentMapT LPM ON LP.ID = LPM.LockboxPaymentID "
		"	WHERE LPM.PaymentID = @nPaymentIdToFix	"

		"	DELETE FROM LockboxPaymentMapT WHERE PaymentID = @nPaymentIdToFix	"

		// (j.gruber 2011-06-02 17:26) - PLID 44850
		"FETCH FROM rsPaymentCorrections INTO @nPaymentIDToFix, @nTakebackIntoBatchPaymentID, @strTakebackBatchPaymentCheckNo, @strTakebackBatchPaymentDate, @strPayUserName, @nPayUserID, @bPayCorrect \r\n "
		"END \r\n"
		"CLOSE rsPaymentCorrections \r\n "
		"DEALLOCATE rsPaymentCorrections \r\n ";


	return sqlFrag;
}

CSqlFragment CFinancialCorrection::CorrectBills()
{
	
	CSqlFragment sqlFrag(" /* BEGIN BILL CORRECTION */ \r\n ");
	
	for (int i = 0; i < m_aryBillCorrections.GetSize(); i++) {
		BillCorrection *pBill = ((BillCorrection*)m_aryBillCorrections.GetAt(i));
		sqlFrag += GenerateBillInsertSql(pBill);
	}

	//first we need to create our negative charges
	sqlFrag += GenerateBillCorrectionSql();	

	sqlFrag += " /*END CHARGE CORRECTION */ \r\n ";	


	//create a new bill

	//copy all the new charges to the new bill
	
	
	return sqlFrag;
}

// (j.gruber 2011-08-03 17:49) - PLID 44851
CSqlFragment CFinancialCorrection::GenerateBillInsertSql(BillCorrection *pBill) 
{
	return CSqlFragment(		
		" /* INSERT BILL CORRECTION*/ \r\n "
		" INSERT INTO @BillCorrections (ID, UserName, UserID, Correct) \r\n "
		" VALUES  ( {INT}, {STRING}, {INT}, {BIT} )\r\n",  
		pBill->nBillID, pBill->strUserName, pBill->nUserID, pBill->bCorrect);
}

// (j.gruber 2011-08-03 17:49) - PLID 44851
CSqlFragment CFinancialCorrection::GenerateBillCorrectionSql() 
{
	CSqlFragment sqlFrag;

	sqlFrag +=
		" /*BEGIN CORRECTING BILLS*/ \r\n "
		" DECLARE rsBillCorrections CURSOR LOCAL SCROLL READ_ONLY FOR  \r\n "
		" 	SELECT ID, UserName, UserID, Correct FROM @BillCorrections BillCorrectionsT \r\n "
		" DECLARE @nBillIDToFix INT; \r\n "
		" DECLARE @nBillUserID INT; \r\n "
		" DECLARE @strBillUserName nVarchar(50) \r\n "
		" DECLARE @bBillCorrect BIT; \r\n "

		" OPEN rsBillCorrections; \r\n "
		" FETCH FROM rsBillCorrections INTO @nBillIDToFix, @strBillUserName, @nBillUserID, @bBillCorrect\r\n "
		" WHILE @@FETCH_STATUS = 0 BEGIN \r\n "

		" IF @bBillCorrect = 1 BEGIN \r\n "

		// (j.gruber 2011-10-13 15:02) - PLID 45358 - affiliate physician status and date
		// (j.gruber 2011-10-13 15:03) - PLID 45356 - affiliate physician ID, amount, note
		// (j.jones 2011-11-21 16:32) - PLID 41558 - added assisting fields
		// (j.armen 2013-06-28 10:37) - PLID 57367 - Idenitate BillsT
		// (j.jones 2013-08-14 12:45) - PLID 57902 - added HCFA Boxes 8, 9b, 9c, 11bQual, 11b
		// (j.gruber 2014-02-28 13:59) - PLID 61108 - take out DiagIDs
		// (r.gonet 07/07/2014) - PLID 62531 - Added BillsT.StatusID and BillsT.StatusModifiedDate
		// (r.gonet 07/24/2014) - PLID 62524 - The corrected bill always gets the current bill status note.
		// (a.walling 2016-03-09 15:25) - PLID 68560 - UB04 Enhancements - update bill splitting for financial corrections and SplitChargesIntoNewBill
		// - added UB04ClaimInfo
		// - removed UB92Box32|UB92Box33|UB92Box34|UB92Box35|UB92Box36|UB04Box36")
		// (r.gonet 2016-04-07) - NX-100072 - Split FirstConditionDate into mulitple date fields. Added ConditionDateType, which was missing before.
		"	INSERT INTO BillsT (PatientID, Date, EntryType, ExtraDesc, RelatedToEmp, RelatedToAutoAcc, State, RelatedToOther, \r\n "
		"	ConditionDate, \r\n"
		"	ConditionDateType, \r\n"
		"	FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, \r\n"
		"	NoWorkFrom, NoWorkTo, HospFrom, HospTo, RefPhyID, OutsideLab, OutsideLabCharges, \r\n "
		"	MedicaidResubmission, OriginalRefNo, PriorAuthNum, HCFABlock19, Note, Location, InsuredPartyID, OthrInsuredPartyID, \r\n "
		"	InputDate, InputName, Deleted, DeleteDate, Deletedby, Description, PatCoord, UseExp, ExpDays, FormType, \r\n "
		"	InsuranceReferralID, Active, \r\n "
		"	HCFABox8, HCFABox9b, HCFABox9c, HCFABox10D, HCFABox11bQual, HCFABox11b, \r\n"
		"	AnesthStartTime, AnesthEndTime, FacilityStartTime, FacilityEndTime, AnesthesiaMinutes, FacilityMinutes, \r\n "
		"	SendCorrespondence, UB92Box79, DeductibleLeftToMeet, \r\n "
		"	CoInsurance, OutofPocketLeftToMeet, UB92Box44, DischargeStatusID, SendPaperwork, PaperworkType, PaperworkTx, \r\n "
		"	SendTestResults, TestResultID, TestResultType, TestResult, SupervisingProviderID, ManualReview, \r\n "
		"	Ansi_ClaimTypeCode, PriorAuthType, HCFABox13Over, AffiliatePhysID, AffiliatePhysAmount, AffiliateNote, AffiliateStatusID, \r\n"
		"	AssistingStartTime, AssistingEndTime, AssistingMinutes, StatusID, StatusModifiedDate, StatusNoteID, UB04ClaimInfo) \r\n"
		"	SELECT PatientID, Date, EntryType, ExtraDesc, RelatedToEmp, RelatedToAutoAcc, State, RelatedToOther, \r\n "
		"	ConditionDate, \r\n"
		"	ConditionDateType, \r\n"
		"	FirstVisitOrConsultationDate, InitialTreatmentDate, LastSeenDate, AcuteManifestationDate, LastXRayDate, HearingAndPrescriptionDate, AssumedCareDate, RelinquishedCareDate, AccidentDate, \r\n"
		"	NoWorkFrom, NoWorkTo, HospFrom, HospTo, RefPhyID, OutsideLab, OutsideLabCharges, \r\n "
		"	MedicaidResubmission, OriginalRefNo, PriorAuthNum, HCFABlock19, Note, Location, InsuredPartyID, OthrInsuredPartyID, \r\n "
		"	InputDate, InputName, Deleted, DeleteDate, Deletedby, Description, PatCoord, UseExp, ExpDays, FormType, \r\n "
		"	InsuranceReferralID, Active, \r\n "
		"	HCFABox8, HCFABox9b, HCFABox9c, HCFABox10D, HCFABox11bQual, HCFABox11b, \r\n"
		"	AnesthStartTime, AnesthEndTime, FacilityStartTime, FacilityEndTime, AnesthesiaMinutes, FacilityMinutes, \r\n "
		"	SendCorrespondence, UB92Box79, DeductibleLeftToMeet, \r\n "
		"	CoInsurance, OutofPocketLeftToMeet, UB92Box44, DischargeStatusID, SendPaperwork, PaperworkType, PaperworkTx, \r\n "
		"	SendTestResults, TestResultID, TestResultType, TestResult, SupervisingProviderID, ManualReview, \r\n "
		"	Ansi_ClaimTypeCode, PriorAuthType, HCFABox13Over, AffiliatePhysID, AffiliatePhysAmount, AffiliateNote, AffiliateStatusID, \r\n"
		"	AssistingStartTime, AssistingEndTime, AssistingMinutes, StatusID, StatusModifiedDate, StatusNoteID, UB04ClaimInfo \r\n"
		"	FROM BillsT WHERE ID = @nBillIDToFix \r\n "

		" SET @nNewBillID = SCOPE_IDENTITY() \r\n "

		// (j.gruber 2011-10-13 15:06) - PLID 45358 - dates
		" INSERT INTO BillAffiliateStatusHistoryT (BillID, StatusID, Date) \r\n "
		" SELECT @nNewBillID, StatusID, Date FROM BillAffiliateStatusHistoryT WHERE BillID = @nBillIDToFix \r\n "

		// (j.gruber 2011-06-22 11:21) - PLID 44865 - BillExtraDiagCodesT
		// (j.gruber 2014-02-28 13:59) - PLID 61108 - change to BillDiagCodeT and update for ICD10
		"	INSERT INTO BillDiagCodeT (BillID, ICD9DiagID, ICD10DiagID, OrderIndex)  \r\n "
		"	SELECT @nNewBillID, ICD9DiagID, ICD10DiagID, OrderIndex FROM BillDiagCodeT WHERE BillID = @nBillIDToFix \r\n "

		// (j.gruber 2011-06-22 15:36) - PLID 44894 - billEmnsT
		"	UPDATE BilledEMNST SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix \r\n "
		// (j.gruber 2011-06-27 11:20) - PLID 44874
		"	UPDATE BilledCaseHistoriesT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.gruber 2011-06-27 11:21) - PLID 45601
		// (r.gonet 07/24/2014) - PLID 62524 - Don't move historical bill status notes. The current bill status note and notes that were never bill status notes will be moved.
		"	UPDATE NoteInfoT SET BillID = @nNewBillID \r\n"
		"	FROM NoteInfoT \r\n"
		"	LEFT JOIN BillsT ON NoteInfoT.NoteID = BillsT.StatusNoteID \r\n"
		"	WHERE BillID = @nBillIDToFix AND (BillsT.ID IS NOT NULL OR NoteInfoT.IsBillStatusNote = 0); \r\n "
		// (j.gruber 2011-06-27 15:35) - PLID 45601
		"   UPDATE ClaimHistoryT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.gruber 2011-06-28 10:37) - PLID 44863 - HCFATrackT
		"   UPDATE HCFATrackT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.gruber 2011-06-28 15:58) - PLID 44863 
		"   UPDATE BilledQuotesT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.gruber 2011-08-05 13:23) - PLID 44560 
		"   UPDATE RewardHistoryT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.gruber 2011-08-05 14:25) - PLID 44906 
		"   UPDATE TopsSubmissionHistoryT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.gruber 2011-08-08 15:58) - PLID 44930
		"   UPDATE ProcInfoT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n "
		// (j.dinatale 2012-03-22 11:17) - PLID 48876 - handle GlassesOrderT
		"	UPDATE GlassesOrderT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix; \r\n"
		// (j.dinatale 2012-07-10 17:13) - PLID 51468 - handle ProcInfoBillsT
		"	UPDATE ProcInfoBillsT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix \r\n"
		// (j.jones 2013-07-10 16:25) - PLID 57148 - supported BillInvoiceNumbersT
		"	UPDATE BillInvoiceNumbersT SET BillID = @nNewBillID WHERE BillID = @nBillIDToFix \r\n"
		// (r.gonet 07/24/2014) - PLID 62524 - The old bill should no longer have a bill status note associated with it.
		"	UPDATE BillsT SET StatusNoteID = NULL WHERE ID = @nBillIDToFix \r\n";
		// (r.gonet 07/28/2014) - PLID 62569 - The old bill should not be on hold any longer.
	sqlFrag += CSqlFragment(
		"	UPDATE BillsT SET StatusID = NULL, Description = (CASE WHEN CHARINDEX('ON HOLD: ', Description) = 1 THEN SUBSTRING(Description, 10, LEN(Description) - 9) ELSE Description END) \r\n"
		"	FROM BillsT \r\n"
		"	INNER JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID \r\n"
		"	WHERE BillsT.ID = @nBillIDToFix AND BillStatusT.Type = {CONST_INT} \r\n"
		, (long)EBillStatusType::OnHold);
	sqlFrag +=
		//end if @bCorrect
		" END \r\n ";

		//now for all the charges on the bill to fix, put a line in @ChargeCorrections
		sqlFrag += "  INSERT INTO @ChargeCorrections (ID, UserID, UserName, Correct) \r\n "
			" SELECT ChargesT.ID, @nBillUserID, @strBillUserName, @bBillCorrect FROM ChargesT \r\n "
			" INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID \r\n "
			" LEFT JOIN LineItemCorrectionsT LCOrigT ON ChargesT.ID = LCOrigT.OriginalLineItemID \r\n "
			" LEFT JOIN LineItemCorrectionsT LCVoidT ON ChargesT.ID = LCVoidT.VoidingLineItemID \r\n "
			" WHERE BillID = @nBillIDToFix AND LineItemT.Deleted = 0 and LCOrigT.ID IS NULL AND LCVoidT.ID IS NULL \r\n ";

		sqlFrag += " INSERT INTO BillCorrectionsT(OriginalBillID, NewBillID, InputDate, InputUserID) \r\n "
			" VALUES (@nBillIDToFix, @nNewBillID, @dtInputDate, @nBillUserID); \r\n "
			"	INSERT INTO @AddedBillCorrections SELECT SCOPE_IDENTITY() \r\n"	;

		sqlFrag += " FETCH FROM rsBillCorrections INTO @nBillIDToFix, @strBillUserName, @nBillUserID, @bBillCorrect \r\n "
			" END \r\n "
			" CLOSE rsBillCorrections \r\n "
			" DEALLOCATE rsBillCorrections \r\n ";

		//now call the charge code
		//sqlFrag += 	GenerateAllChargeCorrectionSql();

		//now we have to delete from ChargeCorrections
		//sqlFrag += " DELETE FROM @ChargeCorrections \r\n ";

		return sqlFrag;	

}

// (j.jones 2011-09-19 11:17) - PLID 45462 - added ability to Undo, which takes in an ID
// from either BillCorrectionsT or LineItemCorrectionsT, the input date from that record,
// and the BillsT/LineItemT IDs from that record
// nVoidingItemID is always -1 for bills, nNewItemID is -1 if they only voided and did not correct
void CFinancialCorrection::AddCorrectionToUndo(CorrectionUndoType cutUndoType, long nCorrectionID, COleDateTime dtCorrectionInputDate,
						long nOriginalItemID, long nVoidingItemID, long nNewItemID,
						long nPatientID)
{
	//can't send in cutAll here
	if (cutUndoType == cutUndoAll) {
		ASSERT(FALSE);
		ThrowNxException("Invalid type sent to AddCorrectionToUndo");
		return;
	}		

	switch (cutUndoType) {

		case cutUndoBill:
			{
				BillCorrectionUndo *pBill = new BillCorrectionUndo();
				pBill->nBillCorrectionID = nCorrectionID;
				pBill->dtCorrectionInputDate = dtCorrectionInputDate;
				pBill->nOriginalBillID = nOriginalItemID;
				pBill->nNewBillID = nNewItemID;
				pBill->nPatientID = nPatientID;
				m_aryBillCorrectionsToUndo.Add((BillCorrectionUndo*)pBill);
			}
		break;

		case cutUndoCharge:
			{
				ChargeCorrectionUndo *pCharge = new ChargeCorrectionUndo();
				pCharge->nChargeCorrectionID = nCorrectionID;
				pCharge->dtCorrectionInputDate = dtCorrectionInputDate;
				pCharge->nOriginalChargeID = nOriginalItemID;
				pCharge->nVoidingChargeID = nVoidingItemID;
				pCharge->nNewChargeID = nNewItemID;
				pCharge->nPatientID = nPatientID;
				m_aryChargeCorrectionsToUndo.Add((ChargeCorrectionUndo*)pCharge);
			}
		break;

		case cutUndoPayment:
			{
				PaymentCorrectionUndo *pPay = new PaymentCorrectionUndo();
				pPay->nPaymentCorrectionID = nCorrectionID;
				pPay->dtCorrectionInputDate = dtCorrectionInputDate;
				pPay->nOriginalPaymentID = nOriginalItemID;
				pPay->nVoidingPaymentID = nVoidingItemID;
				pPay->nNewPaymentID = nNewItemID;
				pPay->nPatientID = nPatientID;
				m_aryPaymentCorrectionsToUndo.Add((PaymentCorrectionUndo*)pPay);
			}
		break;
	}
}

// (j.jones 2011-09-19 11:17) - PLID 45462 - added ability to Undo
// (j.jones 2012-04-24 13:14) - PLID 49804 - added flag for when this is called by an E-Remit Undo
// (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
void CFinancialCorrection::ExecuteCorrectionsToUndo(CorrectionUndoType cutUndoType, BOOL bFromBatchPaymentUndo /*= FALSE*/)
{
	// (j.jones 2012-04-24 14:19) - PLID 49804 - e-remits cannot undo any type but payments
	if (bFromBatchPaymentUndo && cutUndoType != cutUndoPayment) { // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
		ThrowNxException("ExecuteCorrectionsToUndo called from batch payment with a non-payment type!"); // (b.eyers 2015-10-15) - PLID 67308 - renamed
	}

	CSqlFragment sqlFrag(" SET NOCOUNT ON; \r\n "
		" SET XACT_ABORT ON \r\n"
		" BEGIN TRAN \r\n"
		// (j.jones 2011-10-31 14:33) - PLID 45462 - keep track of one consistent input date for every action in this batch
		" DECLARE @dtInputDate DATETIME \r\n"
		" SET @dtInputDate = GetDate() \r\n"
		//these tables track the correction records we're deleting (BillCorrectionsT/LineItemCorrectionsT)
		//the "new" IDs are -1 instead of NULL, if no new record exists
		" DECLARE @RemovedBillCorrections TABLE (ID INT NOT NULL, InputDate datetime NOT NULL, OriginalBillID INT NOT NULL, NewBillID INT NOT NULL, PatientID INT NOT NULL) \r\n "
		" DECLARE @RemovedChargeCorrections TABLE (ID INT NOT NULL, InputDate datetime NOT NULL, OriginalChargeID INT NOT NULL, VoidingChargeID INT NOT NULL, NewChargeID INT NOT NULL, PatientID INT NOT NULL) \r\n "
		" DECLARE @RemovedPaymentCorrections TABLE (ID INT NOT NULL, InputDate datetime NOT NULL, OriginalPaymentID INT NOT NULL, VoidingPaymentID INT NOT NULL, NewPaymentID INT NOT NULL, PatientID INT NOT NULL) \r\n "	
		//these tables track the records we're marking as deleted (BillsT/LineItemT)
		" DECLARE @RemovedBills TABLE (ID INT NOT NULL) \r\n "
		" DECLARE @RemovedCharges TABLE (ID INT NOT NULL) \r\n "
		" DECLARE @RemovedPayments TABLE (ID INT NOT NULL) \r\n "
		//track the current userID and name
		" DECLARE @CurrentUserID INT \r\n "
		" SET @CurrentUserID = {INT} \r\n "
		" DECLARE @CurrentUserName nvarchar(255) \r\n "
		" SET @CurrentUserName = {STRING} \r\n ",
		GetCurrentUserID(), GetCurrentUserName());

	//This needs to go in reverse order of corrections, because unlike corrections
	//the undo code does NOT propagate undoing other corrections. The calling code
	//needs to validate that it is safe to undo other corrections, and then
	//add each correction manually. So charges on a corrected bill have to be
	//undone before the bill correction can itself be undone.

	if (cutUndoType == cutUndoAll || cutUndoType == cutUndoPayment) {
		sqlFrag += UndoPayments();
	}

	if (cutUndoType == cutUndoAll || cutUndoType == cutUndoCharge) {
		sqlFrag += UndoCharges();
	}

	if(cutUndoType == cutUndoAll || cutUndoType == cutUndoBill) {
		sqlFrag += UndoBills();
	}

	enum ERemovedType {

		eRemovedBillCorrection = -21,
		eRemovedChargeCorrection = -22,
		eDeletedBill = -23,
	};
	
	CSqlFragment sqlFragAudit(" SET NOCOUNT OFF; \r\n "
		//for auditing
		" SELECT {CONST} AS Type, ID, InputDate AS Date, PatientID, NewBillID AS NewID, \r\n"
		" NULL AS Description, NULL AS Amount, NULL AS Quantity \r\n"
		" FROM @RemovedBillCorrections RemovedBillCorrectionsT \r\n"
		" UNION ALL \r\n"
		" SELECT {CONST} AS Type, ID, InputDate AS Date, PatientID, NewChargeID AS NewID, \r\n"
		" NULL AS Description, NULL AS Amount, NULL AS Quantity \r\n"
		" FROM @RemovedChargeCorrections RemovedChargeCorrectionsT \r\n"
		" UNION ALL \r\n"
		" SELECT -(LineItemT.Type), RemovedPaymentCorrectionsT.ID, \r\n"
		" RemovedPaymentCorrectionsT.InputDate AS Date, RemovedPaymentCorrectionsT.PatientID, RemovedPaymentCorrectionsT.NewPaymentID AS NewID, \r\n"
		" NULL AS Description, NULL AS Amount, NULL AS Quantity \r\n"
		" FROM @RemovedPaymentCorrections RemovedPaymentCorrectionsT \r\n"
		" INNER JOIN LineItemT ON RemovedPaymentCorrectionsT.OriginalPaymentID = LineItemT.ID "
		" UNION ALL \r\n"
		" SELECT {CONST} AS Type, BillsT.ID, BillsT.Date, BillsT.PatientID, NULL AS NewID, \r\n"
		" BillsT.Description, NULL AS Amount, NULL AS Quantity \r\n"
		" FROM @RemovedBills RemovedBillsT  \r\n"
		" INNER JOIN BillsT ON RemovedBillsT.ID = BillsT.ID "
		" UNION ALL \r\n"
		" SELECT LineItemT.Type, LineItemT.ID, LineItemT.Date, LineItemT.PatientID, NULL AS NewID, \r\n"
		" LineItemT.Description, LineItemT.Amount, ChargesT.Quantity \r\n"
		" FROM @RemovedCharges RemovedChargesT \r\n"
		" INNER JOIN LineItemT ON RemovedChargesT.ID = LineItemT.ID "
		" INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
		" UNION ALL \r\n"
		" SELECT LineItemT.Type, LineItemT.ID, LineItemT.Date, LineItemT.PatientID, NULL AS NewID, \r\n"
		" LineItemT.Description, LineItemT.Amount, NULL AS Quantity \r\n"
		" FROM @RemovedPayments RemovedPaymentsT \r\n"
		" INNER JOIN LineItemT ON RemovedPaymentsT.ID = LineItemT.ID \r\n"
		" \r\n"
		" COMMIT TRAN \r\n",
		eRemovedBillCorrection, eRemovedChargeCorrection, eDeletedBill);

	sqlFrag += sqlFragAudit;

	ADODB::_RecordsetPtr rsUndo = CreateParamRecordset(sqlFrag);

	long nAuditTransactionID = -1;
	try {
		//now audit
		while(!rsUndo->eof) {

			if(nAuditTransactionID == -1 ){
				nAuditTransactionID = BeginAuditTransaction();
			}

			long nType = AdoFldLong(rsUndo->Fields, "Type");
			long nID = AdoFldLong(rsUndo->Fields, "ID");
			COleDateTime dtDate = AdoFldDateTime(rsUndo->Fields, "Date");
			long nPatientID = AdoFldLong(rsUndo->Fields, "PatientID");
			CString strPatientName = GetExistingPatientName(nPatientID);

			switch(nType) {

				case eRemovedBillCorrection: //deleted bill correction
					{

						if (bFromBatchPaymentUndo) { // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
						//this should have been impossible, however it's already committed,
						//so don't throw an exception, audit normally and just assert to warn developers
						ASSERT(FALSE);
					}

					CString strOldValue;
					long nNewID = AdoFldLong(rsUndo->Fields, "NewID", -1);
					if(nNewID == -1) {
						strOldValue.Format("Bill Voided on %s", FormatDateTimeForInterface(dtDate, dtoDate));
					}
					else {
						strOldValue.Format("Bill Voided and Corrected on %s", FormatDateTimeForInterface(dtDate, dtoDate));
					}

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiUndoBillCorrection, nPatientID, strOldValue, "<Reversed>", aepHigh, aetDeleted);

					}
					break;

				case eRemovedChargeCorrection: //deleted charge correction
					{

						if (bFromBatchPaymentUndo) { // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
						//this should have been impossible, however it's already committed,
						//so don't throw an exception, audit normally and just assert to warn developers
						ASSERT(FALSE);
					}

					CString strOldValue;
					long nNewID = AdoFldLong(rsUndo->Fields, "NewID", -1);
					if(nNewID == -1) {
						strOldValue.Format("Charge Voided on %s", FormatDateTimeForInterface(dtDate, dtoDate));
					}
					else {
						strOldValue.Format("Charge Voided and Corrected on %s", FormatDateTimeForInterface(dtDate, dtoDate));
					}

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiUndoChargeCorrection, nPatientID, strOldValue, "<Reversed>", aepHigh, aetDeleted);

					}
					break;

				case -(LineItem::Payment):		//deleted payment correction
				case -(LineItem::Adjustment):	//deleted adjustment correction
				case -(LineItem::Refund):		//deleted refund correction
					{

					AuditEventItems aeiItem = aeiUndoPaymentCorrection;
					CString strType;
					if(nType == -(LineItem::Refund)) {
						strType = "Refund";

						// (j.jones 2012-04-24 14:19) - PLID 49804 - if E-Remit, use a different audit
						if (bFromBatchPaymentUndo) { // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
							//this would only happen if the refund was tied to a remit payment (unlikely)
							aeiItem = aeiUndoRefundCorrectionByBatchPaymentUndo; // (b.eyers 2015-10-15) - PLID 67308 - renamed
						}
						else {
							aeiItem = aeiUndoRefundCorrection;
						}
					}
					else if(nType == -(LineItem::Adjustment)) {
						strType = "Adjustment";

						// (j.jones 2012-04-24 14:19) - PLID 49804 - if E-Remit, use a different audit
						if (bFromBatchPaymentUndo) { // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
							aeiItem = aeiUndoAdjustmentCorrectionByBatchPaymentUndo; // (b.eyers 2015-10-15) - PLID 67308 - renamed
						}
						else {
							aeiItem = aeiUndoAdjustmentCorrection;
						}
					}
					else {
						strType = "Payment";

						// (j.jones 2012-04-24 14:19) - PLID 49804 - if E-Remit, use a different audit
						if (bFromBatchPaymentUndo) { // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
							aeiItem = aeiUndoPaymentCorrectionByBatchPaymentUndo; // (b.eyers 2015-10-15) - PLID 67308 - renamed
						}
						else {
							aeiItem = aeiUndoPaymentCorrection;
						}
					}

					CString strOldValue;
					long nNewID = AdoFldLong(rsUndo->Fields, "NewID", -1);
					if(nNewID == -1) {
						strOldValue.Format("%s Voided on %s", strType, FormatDateTimeForInterface(dtDate, dtoDate));
					}
					else {
						strOldValue.Format("%s Voided and Corrected on %s", strType, FormatDateTimeForInterface(dtDate, dtoDate));
					}

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiItem, nPatientID, strOldValue, "<Reversed>", aepHigh, aetDeleted);

					}
					break;

				case eDeletedBill: //deleted bill

					{

					CString strBillInfo;
					CString strDesc = AdoFldString(rsUndo, "Description", "(No Description)");
					strBillInfo.Format("Description: '%s', Date: %s", strDesc, FormatDateTimeForInterface(dtDate, dtoDate));

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiBillDeleted, nID, strBillInfo, "<Deleted>", aepHigh, aetDeleted);

					}
					break;

				case LineItem::Charge: //deleted charge
					{

					CString strChargeInfo = "";					
					CString strDesc = AdoFldString(rsUndo, "Description", "(No Description)");
					//the charge audit uses the unit cost
					COleCurrency cyAmt = AdoFldCurrency(rsUndo, "Amount", COleCurrency(0,0));
					double dblQty = AdoFldDouble(rsUndo, "Quantity", 1.0);
					strChargeInfo.Format("Description: '%s', Amount: %s, Qty: %g, Date: %s", strDesc, FormatCurrencyForInterface(cyAmt,TRUE,TRUE),
						dblQty, FormatDateTimeForInterface(dtDate, dtoDate));

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiChargeLineDeleted, nID, strChargeInfo, "<Deleted>", aepHigh, aetDeleted);
					}
					break;

				case LineItem::Payment:		//deleted payment
				case LineItem::Adjustment:	//deleted adjustment
				case LineItem::Refund:		//deleted refund
					{

					AuditEventItems aeiItem = aeiPaymentDeleted;
					CString strType;
					if(nType == LineItem::Refund) {
						strType = "Refund";
						aeiItem = aeiRefundDeleted;
					}
					else if(nType == LineItem::Adjustment) {
						strType = "Adjustment";
						aeiItem = aeiAdjustmentDeleted;
					}
					else {
						strType = "Payment";
						aeiItem = aeiPaymentDeleted;
					}

					CString strPayInfo;
					//unlike the bill and charge audits, this defaults to no description
					CString strDesc = AdoFldString(rsUndo, "Description", "");
					COleCurrency cyAmt = AdoFldCurrency(rsUndo, "Amount", COleCurrency(0,0));
					strPayInfo.Format("%s %s, %s, %s", FormatCurrencyForInterface(cyAmt), strType, FormatDateTimeForInterface(dtDate), strDesc);

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiItem, nID, strPayInfo, "<Deleted>", aepHigh, aetDeleted);

					}
					break;

				default:
					//should be impossible, not a supported type
					ASSERT(FALSE);
					break;
						
				break;
			}

			rsUndo->MoveNext();
		}
		rsUndo->Close();

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

	}NxCatchAllCall(__FUNCTION__, 
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2011-09-19 16:44) - PLID 45462 - added Undo feature for bills
CSqlFragment CFinancialCorrection::UndoBills()
{
	
	CSqlFragment sqlFrag(" /* BEGIN BILL CORRECTION UNDO */ \r\n ");
	
	for (int i = 0; i < m_aryBillCorrectionsToUndo.GetSize(); i++) {
		BillCorrectionUndo *pBill = ((BillCorrectionUndo*)m_aryBillCorrectionsToUndo.GetAt(i));
		sqlFrag += GenerateBillUndoInsertSql(pBill);
	}

	sqlFrag += GenerateBillUndoSql();

	sqlFrag += " /*END BILL CORRECTION UNDO */ \r\n ";
	
	return sqlFrag;
}

// (j.jones 2011-09-20 11:47) - PLID 45562 - added Undo feature for charges
CSqlFragment CFinancialCorrection::UndoCharges()
{	
	CSqlFragment sqlFrag(" /* BEGIN CHARGE CORRECTION UNDO*/ \r\n ");
	
	for (int i = 0; i < m_aryChargeCorrectionsToUndo.GetSize(); i++) {
		ChargeCorrectionUndo *pCharge = ((ChargeCorrectionUndo*)m_aryChargeCorrectionsToUndo.GetAt(i));
		sqlFrag += GenerateChargeUndoInsertSql(pCharge);
	}

	sqlFrag += GenerateChargeUndoSql();

	sqlFrag += " /*END CHARGE CORRECTION UNDO*/ \r\n ";	

	return sqlFrag;
}

// (j.jones 2011-09-20 11:47) - PLID 45563 - added Undo feature for payments
CSqlFragment CFinancialCorrection::UndoPayments()
{

	CSqlFragment sqlFrag(" /* BEGIN PAYMENT CORRECTION UNDO*/ \r\n ");
	
	for (int i = 0; i < m_aryPaymentCorrectionsToUndo.GetSize(); i++) {
		PaymentCorrectionUndo *pPayment = ((PaymentCorrectionUndo*)m_aryPaymentCorrectionsToUndo.GetAt(i));
		sqlFrag += GeneratePaymentUndoInsertSql(pPayment);
	}

	sqlFrag += GeneratePaymentUndoSql();

	sqlFrag += " /*END PAYMENT CORRECTION UNDO*/ \r\n ";	

	return sqlFrag;
}

// (j.jones 2011-09-19 16:44) - PLID 45462 - added Undo feature for bills
CSqlFragment CFinancialCorrection::GenerateBillUndoInsertSql(BillCorrectionUndo *pBill)
{
	//NewBillID is -1 instead of NULL, if there is no new bill
	return CSqlFragment(
		" /* INSERT BILL CORRECTION UNDO*/ \r\n "
		" INSERT INTO @RemovedBillCorrections (ID, InputDate, OriginalBillID, NewBillID, PatientID) \r\n "
		" VALUES ({INT}, {VT_DATE}, {INT}, {INT}, {INT}) \r\n",  
		pBill->nBillCorrectionID, COleVariant(pBill->dtCorrectionInputDate), pBill->nOriginalBillID, pBill->nNewBillID,
		pBill->nPatientID);
}

// (j.jones 2011-09-20 11:47) - PLID 45562 - added Undo feature for charges
CSqlFragment CFinancialCorrection::GenerateChargeUndoInsertSql(ChargeCorrectionUndo *pCharge)
{
	//NewChargeID is -1 instead of NULL, if there is no new charge
	return CSqlFragment(
		" /* INSERT CHARGE CORRECTION UNDO*/ \r\n "
		" INSERT INTO @RemovedChargeCorrections (ID, InputDate, OriginalChargeID, VoidingChargeID, NewChargeID, PatientID) \r\n "
		" VALUES ({INT}, {VT_DATE}, {INT}, {INT}, {INT}, {INT}) \r\n",  
		pCharge->nChargeCorrectionID, COleVariant(pCharge->dtCorrectionInputDate), pCharge->nOriginalChargeID, pCharge->nVoidingChargeID, pCharge->nNewChargeID,
		pCharge->nPatientID);
}

// (j.jones 2011-09-20 11:47) - PLID 45563 - added Undo feature for payments
CSqlFragment CFinancialCorrection::GeneratePaymentUndoInsertSql(PaymentCorrectionUndo *pPayment)
{
	//NewPaymentID is -1 instead of NULL, if there is no new payment
	return CSqlFragment(
		" /* INSERT PAYMENT CORRECTION UNDO*/ \r\n "
		" INSERT INTO @RemovedPaymentCorrections (ID, InputDate, OriginalPaymentID, VoidingPaymentID, NewPaymentID, PatientID) \r\n "
		" VALUES ({INT}, {VT_DATE}, {INT}, {INT}, {INT}, {INT}) \r\n",  
		pPayment->nPaymentCorrectionID, COleVariant(pPayment->dtCorrectionInputDate), pPayment->nOriginalPaymentID, pPayment->nVoidingPaymentID, pPayment->nNewPaymentID,
		pPayment->nPatientID);
}

// (j.jones 2011-09-19 16:44) - PLID 45462 - added Undo feature for bills
CSqlFragment CFinancialCorrection::GenerateBillUndoSql()
{
	CSqlFragment sqlFrag;

	sqlFrag += 
		" /*BEGIN UNDOING BILLS*/ \r\n "
		" DECLARE rsRemovedCorrections CURSOR LOCAL SCROLL READ_ONLY FOR  \r\n "
		" 	SELECT ID, InputDate, OriginalBillID, NewBillID FROM @RemovedBillCorrections RemovedBillCorrectionsT \r\n "
		" DECLARE @nBillCorrectionIDToRemove INT; \r\n "
		" DECLARE @nBillCorrectionInputDate DATETIME; \r\n "
		" DECLARE @nBillCorrectionOriginalBillID INT; \r\n "
		" DECLARE @nBillCorrectionNewBillID INT; \r\n "	//may be -1

		" OPEN rsRemovedCorrections; \r\n "
		" FETCH FROM rsRemovedCorrections INTO @nBillCorrectionIDToRemove, @nBillCorrectionInputDate, @nBillCorrectionOriginalBillID, @nBillCorrectionNewBillID \r\n "
		" WHILE @@FETCH_STATUS = 0 BEGIN \r\n ";

	//if there is a new bill, delete it, and track it in @RemovedBills
	sqlFrag += "IF @nBillCorrectionNewBillID <> -1 BEGIN \r\n "
		"	INSERT INTO @RemovedBills (ID) VALUES (@nBillCorrectionNewBillID) \r\n"

		//reassign IDs back to the main bill
		// (j.dinatale 2012-03-22 15:18) - PLID 48876 - handle GlassesOrderT
		// (j.dinatale 2012-07-10 17:13) - PLID 51468 - handle ProcInfoBillsT
		// (j.jones 2013-07-10 16:25) - PLID 57148 - supported BillInvoiceNumbersT
		// (r.gonet 07/24/2014) - PLID 62524 - Restore the original bill's status note.
		"	UPDATE BilledEMNST SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID \r\n "
		"	UPDATE BilledCaseHistoriesT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"	UPDATE NoteInfoT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"   UPDATE ClaimHistoryT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"   UPDATE HCFATrackT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"   UPDATE BilledQuotesT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"   UPDATE RewardHistoryT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"   UPDATE TopsSubmissionHistoryT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"   UPDATE ProcInfoT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n "
		"	UPDATE GlassesOrderT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID; \r\n"
		"	UPDATE ProcInfoBillsT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID \r\n"
		"	UPDATE BillInvoiceNumbersT SET BillID = @nBillCorrectionOriginalBillID WHERE BillID = @nBillCorrectionNewBillID \r\n"
		"	UPDATE BillsT SET StatusNoteID = (SELECT StatusNoteID FROM BillsT WHERE BillsT.ID = @nBillCorrectionNewBillID) WHERE BillsT.ID = @nBillCorrectionOriginalBillID; \r\n"

		//mark the bill as deleted
		// (r.gonet 07/24/2014) - PLID 62524 - Remove the status note from the corrected bill.
		"	UPDATE BillsT SET Deleted = -1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserID, StatusNoteID = NULL WHERE ID = @nBillCorrectionNewBillID \r\n"

		//delete all charges that are not already deleted
		"	INSERT INTO @RemovedCharges (ID) SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0 \r\n"
		"	DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0)) \r\n"
		"	DELETE FROM AppliesT WHERE DestID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0) \r\n"
		"	DELETE FROM ChargedProductItemsT WHERE ChargeID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0) \r\n"
		"	UPDATE RewardHistoryT SET Deleted = 1, DeletedDate = @dtInputDate WHERE Deleted = 0 AND Source = -1 AND ChargeID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0) \r\n"
		"	UPDATE ChargeDiscountsT SET Deleted = 1, DeletedBy = @CurrentUserName, DeleteDate = @dtInputDate WHERE ChargeID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0) \r\n"
		"	UPDATE LineItemT SET Deleted = 1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserName WHERE ID IN (SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = @nBillCorrectionNewBillID AND Deleted = 0) \r\n"

	//end if @nBillCorrectionNewBillID <> -1
	" END \r\n ";

	//delete our bill correction
	sqlFrag += " DELETE FROM BillCorrectionsT WHERE ID = @nBillCorrectionIDToRemove \r\n ";

	//get the next correction to undo
	sqlFrag += " FETCH FROM rsRemovedCorrections INTO @nBillCorrectionIDToRemove, @nBillCorrectionInputDate, @nBillCorrectionOriginalBillID, @nBillCorrectionNewBillID \r\n "
		" END \r\n "
		" CLOSE rsRemovedCorrections \r\n "
		" DEALLOCATE rsRemovedCorrections \r\n ";

	return sqlFrag;
}

// (j.jones 2011-09-20 11:47) - PLID 45562 - added Undo feature for charges
CSqlFragment CFinancialCorrection::GenerateChargeUndoSql()
{
	CSqlFragment sqlFrag;

	sqlFrag += 
		" /*BEGIN UNDOING CHARGES*/ \r\n "
		" DECLARE rsRemovedCorrections CURSOR LOCAL SCROLL READ_ONLY FOR  \r\n "
		" 	SELECT ID, InputDate, OriginalChargeID, VoidingChargeID, NewChargeID FROM @RemovedChargeCorrections RemovedChargeCorrectionsT \r\n "
		" DECLARE @nChargeCorrectionIDToRemove INT; \r\n "
		" DECLARE @nChargeCorrectionInputDate DATETIME; \r\n "
		" DECLARE @nChargeCorrectionOriginalChargeID INT; \r\n "
		" DECLARE @nChargeCorrectionVoidingChargeID INT; \r\n "
		" DECLARE @nChargeCorrectionNewChargeID INT; \r\n "	//may be -1

		" OPEN rsRemovedCorrections; \r\n "
		" FETCH FROM rsRemovedCorrections INTO @nChargeCorrectionIDToRemove, @nChargeCorrectionInputDate, @nChargeCorrectionOriginalChargeID, @nChargeCorrectionVoidingChargeID, @nChargeCorrectionNewChargeID \r\n "
		" WHILE @@FETCH_STATUS = 0 BEGIN \r\n ";

	//delete content from LineItemCorrectionsBalancingAdjT
	//they can never have anything applied to them, so we only need to remove source applies
	sqlFrag += 
		" INSERT INTO @RemovedPayments (ID) SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT WHERE LineItemCorrectionID = @nChargeCorrectionIDToRemove \r\n"
		" DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID IN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT WHERE LineItemCorrectionID = @nChargeCorrectionIDToRemove)) \r\n"
		" DELETE FROM AppliesT WHERE SourceID IN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT WHERE LineItemCorrectionID = @nChargeCorrectionIDToRemove) \r\n"
		" UPDATE LineItemT SET Deleted = 1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserName WHERE ID IN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT WHERE LineItemCorrectionID = @nChargeCorrectionIDToRemove) \r\n"
		" DELETE FROM LineItemCorrectionsBalancingAdjT WHERE LineItemCorrectionID = @nChargeCorrectionIDToRemove \r\n";

	//if there is a new charge, delete it, and track it in @RemovedCharges
	sqlFrag += "IF @nChargeCorrectionNewChargeID <> -1 BEGIN \r\n "
		"	INSERT INTO @RemovedCharges (ID) VALUES (@nChargeCorrectionNewChargeID) \r\n"

		//reassign IDs back to the main charge
		"	DECLARE @nNewChargeID INT; \r\n "
		"	DECLARE @nChargeIDToFix INT; \r\n "
		"	SET @nNewChargeID = @nChargeCorrectionOriginalChargeID \r\n"
		"	SET @nChargeIDToFix = @nChargeCorrectionNewChargeID \r\n";

		//reassign all external constraints
		sqlFrag += GenerateChargeContraintsSql();

		//mark the charge as deleted
		sqlFrag += 
		"	DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID = @nChargeCorrectionNewChargeID) \r\n"
		"	DELETE FROM AppliesT WHERE DestID = @nChargeCorrectionNewChargeID \r\n"
		"	DELETE FROM ChargedProductItemsT WHERE ChargeID = @nChargeCorrectionNewChargeID \r\n"
		"	UPDATE RewardHistoryT SET Deleted = 1, DeletedDate = @dtInputDate WHERE Deleted = 0 AND Source = -1 AND ChargeID = @nChargeCorrectionNewChargeID \r\n"
		"	UPDATE ChargeDiscountsT SET Deleted = 1, DeletedBy = @CurrentUserName, DeleteDate = @dtInputDate WHERE ChargeID = @nChargeCorrectionNewChargeID \r\n"
		"	UPDATE LineItemT SET Deleted = 1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserName WHERE ID = @nChargeCorrectionNewChargeID \r\n"

	//end if @nChargeCorrectionNewChargeID <> -1
	" END \r\n ";

	//delete the voiding charge
	sqlFrag += 
		" INSERT INTO @RemovedCharges (ID) VALUES (@nChargeCorrectionVoidingChargeID) \r\n"
		" DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID = @nChargeCorrectionVoidingChargeID) \r\n"
		" DELETE FROM AppliesT WHERE DestID = @nChargeCorrectionVoidingChargeID \r\n"
		" DELETE FROM ChargedProductItemsT WHERE ChargeID = @nChargeCorrectionVoidingChargeID \r\n"
		" UPDATE RewardHistoryT SET Deleted = 1, DeletedDate = @dtInputDate WHERE Deleted = 0 AND Source = -1 AND ChargeID = @nChargeCorrectionVoidingChargeID \r\n"
		" UPDATE ChargeDiscountsT SET Deleted = 1, DeletedBy = @CurrentUserName, DeleteDate = @dtInputDate WHERE ChargeID = @nChargeCorrectionVoidingChargeID \r\n"
		" UPDATE LineItemT SET Deleted = 1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserName WHERE ID = @nChargeCorrectionVoidingChargeID \r\n";

	//delete our charge correction
	sqlFrag += " DELETE FROM LineItemCorrectionsT WHERE ID = @nChargeCorrectionIDToRemove \r\n ";

	//get the next correction to undo
	sqlFrag += " FETCH FROM rsRemovedCorrections INTO @nChargeCorrectionIDToRemove, @nChargeCorrectionInputDate, @nChargeCorrectionOriginalChargeID, @nChargeCorrectionVoidingChargeID, @nChargeCorrectionNewChargeID \r\n "
		" END \r\n "
		" CLOSE rsRemovedCorrections \r\n "
		" DEALLOCATE rsRemovedCorrections \r\n ";

	return sqlFrag;
}

// (j.jones 2011-09-20 11:47) - PLID 45563 - added Undo feature for payments
CSqlFragment CFinancialCorrection::GeneratePaymentUndoSql()
{
	CSqlFragment sqlFrag;

	sqlFrag += 
		" /*BEGIN UNDOING PAYMENTS*/ \r\n "
		" DECLARE rsRemovedCorrections CURSOR LOCAL SCROLL READ_ONLY FOR  \r\n "
		" 	SELECT ID, InputDate, OriginalPaymentID, VoidingPaymentID, NewPaymentID FROM @RemovedPaymentCorrections RemovedPaymentCorrectionsT \r\n "
		" DECLARE @nPaymentCorrectionIDToRemove INT; \r\n "
		" DECLARE @nPaymentCorrectionInputDate DATETIME; \r\n "
		" DECLARE @nPaymentCorrectionOriginalPaymentID INT; \r\n "
		" DECLARE @nPaymentCorrectionVoidingPaymentID INT; \r\n "
		" DECLARE @nPaymentCorrectionNewPaymentID INT; \r\n "	//may be -1

		" OPEN rsRemovedCorrections; \r\n "
		" FETCH FROM rsRemovedCorrections INTO @nPaymentCorrectionIDToRemove, @nPaymentCorrectionInputDate, @nPaymentCorrectionOriginalPaymentID, @nPaymentCorrectionVoidingPaymentID, @nPaymentCorrectionNewPaymentID \r\n "
		" WHILE @@FETCH_STATUS = 0 BEGIN \r\n ";

	//if there is a new payment, delete it, and track it in @RemovedPayments
	sqlFrag += "IF @nPaymentCorrectionNewPaymentID <> -1 BEGIN \r\n "
		"	INSERT INTO @RemovedPayments (ID) VALUES (@nPaymentCorrectionNewPaymentID) \r\n"

		//reassign IDs back to the main payment
		"	DECLARE @iNewLineItemID INT; \r\n "
		"	DECLARE @nPaymentIDToFix INT; \r\n "
		"	SET @iNewLineItemID = @nPaymentCorrectionOriginalPaymentID \r\n"
		"	SET @nPaymentIDToFix = @nPaymentCorrectionNewPaymentID \r\n"

		//we previously removed the QuoteID from the original payment, re-link it to whatever the current QuoteID is
		"	UPDATE PaymentsT SET QuoteID = (SELECT QuoteID FROM PaymentsT WHERE ID = @nPaymentCorrectionNewPaymentID) WHERE ID = @nPaymentCorrectionOriginalPaymentID \r\n";

		//reassign all external constraints
		sqlFrag += GeneratePaymentContraintsSql();

		//mark the payment as deleted
		sqlFrag +=
		"	DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID = @nPaymentCorrectionNewPaymentID) \r\n"
		"	DELETE FROM AppliesT WHERE SourceID = @nPaymentCorrectionNewPaymentID \r\n"
		"	DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID = @nPaymentCorrectionNewPaymentID) \r\n"
		"	DELETE FROM AppliesT WHERE DestID = @nPaymentCorrectionNewPaymentID \r\n"
		"	UPDATE LineItemT SET Deleted = 1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserName WHERE ID = @nPaymentCorrectionNewPaymentID \r\n"

	//end if @nPaymentCorrectionNewPaymentID <> -1
	" END \r\n ";

	//delete the voiding payment
	sqlFrag += " INSERT INTO @RemovedPayments (ID) VALUES (@nPaymentCorrectionVoidingPaymentID) \r\n"
		" DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID = @nPaymentCorrectionVoidingPaymentID) \r\n"
		" DELETE FROM AppliesT WHERE SourceID = @nPaymentCorrectionVoidingPaymentID \r\n"
		" DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID = @nPaymentCorrectionVoidingPaymentID) \r\n"
		" DELETE FROM AppliesT WHERE DestID = @nPaymentCorrectionVoidingPaymentID \r\n"
		" UPDATE LineItemT SET Deleted = 1, DeleteDate = @dtInputDate, DeletedBy = @CurrentUserName WHERE ID = @nPaymentCorrectionVoidingPaymentID \r\n";

	//delete our payment correction
	sqlFrag += " DELETE FROM LineItemCorrectionsT WHERE ID = @nPaymentCorrectionIDToRemove \r\n ";

	//get the next correction to undo
	sqlFrag += " FETCH FROM rsRemovedCorrections INTO @nPaymentCorrectionIDToRemove, @nPaymentCorrectionInputDate, @nPaymentCorrectionOriginalPaymentID, @nPaymentCorrectionVoidingPaymentID, @nPaymentCorrectionNewPaymentID \r\n "
		" END \r\n "
		" CLOSE rsRemovedCorrections \r\n "
		" DEALLOCATE rsRemovedCorrections \r\n ";

	return sqlFrag;
}