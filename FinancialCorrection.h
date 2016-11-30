#pragma once

// (j.gruber 2011-08-03 11:26) - PLID 44836 - created for

enum CorrectionType
{
	ctAll = -1,
	ctBill = 0,
	ctPayment,
	ctCharge,
};

struct BillCorrection
{
	long nBillID;
	CString strUserName;
	long nUserID;
	BOOL bCorrect; // (j.gruber 2011-06-02 17:18) - PLID 44850	
};

struct ChargeCorrection
{
	long nChargeID;
	CString strUserName;
	long nUserID;	
	BOOL bCorrect; // (j.gruber 2011-06-02 17:18) - PLID 44850	
};

struct PaymentCorrection
{
	long nPaymentID;
	CString strUserName;
	long nUserID;
	BOOL bCorrect; // (j.gruber 2011-06-02 17:18) - PLID 44850
	long nTakebackIntoBatchPaymentID; // (j.jones 2012-04-23 16:12) - PLID 48032 - added nTakebackIntoBatchPaymentID
	// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
	CString strTakebackBatchPaymentCheckNo;
	CString strTakebackBatchPaymentDate;
};

// (j.jones 2011-09-19 11:17) - PLID 45462 - added ability to Undo
enum CorrectionUndoType
{
	cutUndoAll = -1,
	cutUndoBill = 0,
	cutUndoPayment,
	cutUndoCharge,
};

struct BillCorrectionUndo
{
	long nBillCorrectionID;
	COleDateTime dtCorrectionInputDate;
	long nOriginalBillID;
	long nNewBillID;
	long nPatientID;
};

struct ChargeCorrectionUndo
{
	long nChargeCorrectionID;
	COleDateTime dtCorrectionInputDate;
	long nOriginalChargeID;
	long nVoidingChargeID;
	long nNewChargeID;
	long nPatientID;
};

struct PaymentCorrectionUndo
{
	long nPaymentCorrectionID;
	COleDateTime dtCorrectionInputDate;
	long nOriginalPaymentID;
	long nVoidingPaymentID;
	long nNewPaymentID;
	long nPatientID;
};

class CFinancialCorrection
{
public:
	CFinancialCorrection(void);
	~CFinancialCorrection(void);

	// (j.gruber 2011-06-02 15:04) - PLID 44850 - added Correct parameter, default to true to void and correct
	// (j.jones 2012-04-23 16:03) - PLID 48032 - added TakebackIntoBatchPaymentID, which ties the correction to
	// the batch payment and adds the voided payment's value to the batch payment balance
	// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date (already internationally formatted)
	void AddCorrection(CorrectionType ctType, long nID, CString strUserName, long nUserID,
					BOOL bCorrect = TRUE, long nTakebackIntoBatchPaymentID = -1,
					CString strTakebackBatchPaymentCheckNo = "", CString strTakebackBatchPaymentDate = "");

	// (j.jones 2012-04-23 15:17) - PLID 49847 - added boolean for whether E-Remittance called this correction
	// (j.jones 2015-06-16 16:51) - PLID 64932 - removed correction type, because execute should handle
	// every correction in our list
	void ExecuteCorrections(BOOL bCorrectedByERemit = FALSE);

	// (j.jones 2011-09-19 11:17) - PLID 45462 - added ability to Undo, which takes in an ID
	// from either BillCorrectionsT or LineItemCorrectionsT, the input date from that record,
	// and the BillsT/LineItemT IDs from that record
	// nVoidingItemID is always -1 for bills, nNewItemID is -1 if they only voided and did not correct
	void AddCorrectionToUndo(CorrectionUndoType cutUndoType, long nCorrectionID, COleDateTime dtCorrectionInputDate,
						long nOriginalItemID, long nVoidingItemID, long nNewItemID,
						long nPatientID);
	// (j.jones 2012-04-24 13:14) - PLID 49804 - added flag for when this is called by an E-Remit Undo
	void ExecuteCorrectionsToUndo(CorrectionUndoType cutUndoType, BOOL bFromBatchPaymentUndo = FALSE); // (b.eyers 2015-10-15) - PLID 67308 - renamed parameter
	
	long GetChargeCorrectionCount();
	long GetPaymentCorrectionCount();
	long GetBillCorrectionCount();

	CSqlFragment GenerateChargeContraintsSql();
	CSqlFragment GeneratePaymentContraintsSql();

private:

	// (j.jones 2011-09-20 15:55) - PLID 45462 - there was no need for a filter here
	void ClearArrays();
	CSqlFragment CorrectBills();
	CSqlFragment CorrectCharges();
	CSqlFragment CorrectPayments();

	CPtrArray m_aryChargeCorrections;
	CPtrArray m_aryPaymentCorrections;
	CPtrArray m_aryBillCorrections;

	CSqlFragment GenerateChargeInsertSql(ChargeCorrection *pCharge);
	CSqlFragment GenerateAllChargeCorrectionSql();
	CSqlFragment GenerateChargeCorrectionSql();
	CSqlFragment GenerateChargeAppliesByChargeRespSql();
	CSqlFragment GenerateChargeAppliesByAppliesToChargeSql();
	CSqlFragment GenerateChargeCorrectionAdjustmentsSql();

	// (j.dinatale 2011-05-26 17:58) - PLID 44810
	CSqlFragment GeneratePaymentInsertSql(PaymentCorrection *pPayment);
	CSqlFragment GeneratePaymentCorrectionSql();

	// (j.gruber 2011-08-03 17:49) - PLID 44851
	CSqlFragment GenerateBillInsertSql(BillCorrection *pBill);
	CSqlFragment GenerateBillCorrectionSql();

	// (j.jones 2011-09-19 16:44) - PLID 45462 - added Undo feature for bills
	CSqlFragment UndoBills();
	// (j.jones 2011-09-20 11:47) - PLID 45562 - added Undo feature for charges
	CSqlFragment UndoCharges();
	// (j.jones 2011-09-20 11:47) - PLID 45563 - added Undo feature for payments
	CSqlFragment UndoPayments();

	CPtrArray m_aryBillCorrectionsToUndo;
	CPtrArray m_aryChargeCorrectionsToUndo;
	CPtrArray m_aryPaymentCorrectionsToUndo;	

	CSqlFragment GenerateBillUndoInsertSql(BillCorrectionUndo *pBill);
	CSqlFragment GenerateChargeUndoInsertSql(ChargeCorrectionUndo *pCharge);
	CSqlFragment GeneratePaymentUndoInsertSql(PaymentCorrectionUndo *pPayment);

	CSqlFragment GenerateBillUndoSql();
	CSqlFragment GenerateChargeUndoSql();
	CSqlFragment GeneratePaymentUndoSql();
};
