#include "stdafx.h"
#include "Practice.h"
#include "BatchPaymentInsReversalDlg.h"
#include "FinancialCorrection.h"
#include "GlobalFinancialUtils.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (j.armen 2012-05-24 17:36) - PLID 50520 - Patient Combo dl columns
namespace PatientList {
	enum {
		PatientID,
		Last,
		First,
		Middle,
		UserDefinedID,
		BirthDate,
		SSN,
	};
};

// (j.armen 2012-05-24 17:36) - PLID 50520 - Payment and Selected Payment dl columns
namespace PaymentList {
	enum {
		LineItemID,
		PatientID,
		Patient,
		Date,
		Type,
		Description,
		Name,
		Amount,
		AppliedAmount,
		ChargeDate,
		ItemCode,
		ChargeDescription,
	};
};

IMPLEMENT_DYNAMIC(CBatchPaymentInsReversalDlg, CNxDialog)

CBatchPaymentInsReversalDlg::CBatchPaymentInsReversalDlg(CWnd* pParent, const long& nBatchPaymentID)
	: CNxDialog(CBatchPaymentInsReversalDlg::IDD, pParent), m_nBatchPaymentID(nBatchPaymentID)
{

}

CBatchPaymentInsReversalDlg::~CBatchPaymentInsReversalDlg()
{
}

// (j.armen 2012-05-24 17:37) - PLID 50520 - Function to ensure that the correct buttons are enabled / disabled
void CBatchPaymentInsReversalDlg::EnsureControlState()
{
	if(m_pdlPatientList->GetRowCount())
	{
		m_pdlPatientList->Enabled = VARIANT_TRUE;
		m_pdlPaymentList->Enabled = VARIANT_TRUE;
		m_pdlSelectedList->Enabled = VARIANT_TRUE;

		if(m_pdlPaymentList->GetRowCount())
			m_btnMoveRight.EnableWindow(TRUE);
		else
			m_btnMoveRight.EnableWindow(FALSE);
			

		if(m_pdlSelectedList->GetRowCount())
		{
			m_btnMoveLeft.EnableWindow(TRUE);
			m_btnOK.EnableWindow(TRUE);
		}
		else
		{
			m_btnMoveLeft.EnableWindow(FALSE);
			m_btnOK.EnableWindow(FALSE);
		}
	}
	else
	{
		m_btnOK.EnableWindow(FALSE);
		m_btnMoveRight.EnableWindow(FALSE);
		m_btnMoveLeft.EnableWindow(FALSE);
		m_pdlPatientList->Enabled = VARIANT_FALSE;
		m_pdlPaymentList->Enabled = VARIANT_FALSE;
		m_pdlSelectedList->Enabled = VARIANT_FALSE;
	}
}

// (j.armen 2012-05-24 17:37) - PLID 50520 - fill array with distinct line item id's from the selected list
void CBatchPaymentInsReversalDlg::GetSelectedLineItemIDs(CArray<long>& aryLineItemIDs)
{
	aryLineItemIDs.RemoveAll();
	for(IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		const long nLineItemID = VarLong(pRow->GetValue(PaymentList::LineItemID));
		for(int i = 0; i < aryLineItemIDs.GetSize(); i++) {
			if(aryLineItemIDs[i] == nLineItemID)
				continue;
		}
		aryLineItemIDs.Add(nLineItemID);
	}
}

// (j.armen 2012-05-24 17:38) - PLID 50520 - Handle selecting a line item.
void CBatchPaymentInsReversalDlg::SelectLineItem(const _variant_t& vtLineItemID)
{
	ASSERT(m_pdlPaymentList->ColumnCount == m_pdlSelectedList->ColumnCount);

	IRowSettingsPtr pRow = m_pdlPaymentList->GetFirstRow();
	while(pRow)
	{
		IRowSettingsPtr pNextRow = pRow->GetNextRow();
		if(pRow->GetValue(PaymentList::LineItemID) == vtLineItemID)
		{
			IRowSettingsPtr pNewRow = m_pdlSelectedList->TakeRowAddSorted(pRow);

			if(!m_pdlSelectedList->CurSel)
				m_pdlSelectedList->CurSel = pNewRow;

			m_pdlPaymentList->RemoveRow(pRow);

			if(!m_pdlPaymentList->CurSel)
				m_pdlPaymentList->CurSel = m_pdlPaymentList->GetFirstRow();
		}
		pRow = pNextRow;
	}
}

// (j.armen 2012-05-24 17:38) - PLID 50520 - handle deselecting a line item
void CBatchPaymentInsReversalDlg::DeselectLineItem(const _variant_t& vtLineItemID)
{
	ASSERT(m_pdlPaymentList->ColumnCount == m_pdlSelectedList->ColumnCount);

	IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow();
	while(pRow)
	{
		IRowSettingsPtr pNextRow = pRow->GetNextRow();
		if(pRow->GetValue(PaymentList::LineItemID) == vtLineItemID)
		{
			if(m_pdlPatientList->CurSel->GetValue(PatientList::PatientID) == pRow->GetValue(PaymentList::PatientID))
			{
				IRowSettingsPtr pNewRow = m_pdlPaymentList->TakeRowAddSorted(pRow);
				if(!m_pdlPaymentList->CurSel)
					m_pdlPaymentList->CurSel = pNewRow;
			}

			m_pdlSelectedList->RemoveRow(pRow);

			if(!m_pdlSelectedList->CurSel)
				m_pdlSelectedList->CurSel = m_pdlSelectedList->GetFirstRow();
		}
		pRow = pNextRow;
	}
}

// (j.armen 2012-05-24 17:38) - PLID 50520 - validate line item correction statuses.
//	this should almost never get called unless two people are doing corrections at the same time.
//	the user will be warned, then this fn is called to highlight the invalid entries.
void CBatchPaymentInsReversalDlg::ValidateLineItemCorrectionStatus()
{
	for(IRowSettingsPtr pRow = m_pdlPaymentList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		switch(GetLineItemCorrectionStatus(VarLong(pRow->GetValue(PaymentList::LineItemID)))) {
			case licsOriginal:
			case licsVoid:
				pRow->PutBackColor(RGB(255, 102, 102));
				pRow->PutBackColorSel(RGB(255, 102, 102));
				break;
			default:
				break;
		}
	}

	for(IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		switch(GetLineItemCorrectionStatus(VarLong(pRow->GetValue(PaymentList::LineItemID)))) {
			case licsOriginal:
			case licsVoid:
				pRow->PutBackColor(RGB(255, 102, 102));
				pRow->PutBackColorSel(RGB(255, 102, 102));
				break;
			default:
				break;
		}
	}
}

void CBatchPaymentInsReversalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REVERSAL_SELECT, m_btnMoveRight);
	DDX_Control(pDX, IDC_REVERSAL_DESELECT, m_btnMoveLeft);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_REVERSAL_NXCOLOR, m_NxColor);
}

BOOL CBatchPaymentInsReversalDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try
	{
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnMoveRight.AutoSet(NXB_DOWN);
		m_btnMoveLeft.AutoSet(NXB_UP);
		m_NxColor.SetColor(GetNxColor(GNC_FINANCIAL, 0));

		m_pdlPatientList = BindNxDataList2Ctrl(IDC_REVERSAL_PATIENT, false);
		m_pdlPaymentList = BindNxDataList2Ctrl(IDC_REVERSAL_PAYMENT_ADJUSTMENT, false);
		m_pdlSelectedList = BindNxDataList2Ctrl(IDC_REVERSAL_SELECTED_PAYMENT_ADJUSTMENT, false);

		EnsureControlState();

		// (j.jones 2014-07-14 16:08) - PLID 62876 - chargebacks cannot be reversed, so they are hidden in this screen
		m_pdlPatientList->FromClause = _bstr_t(
			"("
			"	SELECT PatientsT.PersonID, PersonT.Last, PersonT.First, PersonT.Middle, PatientsT.UserDefinedID, PersonT.BirthDate, PersonT.SocialSecurity"
			"	FROM PatientsT"
			"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID"
			"	WHERE PatientsT.PersonID IN ("
			"		SELECT DISTINCT LineItemT.PatientID"
			"		FROM LineItemT"
			"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID"
			"		INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID"
			"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"		LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
			"		LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON LineItemT.ID = Chargebacks_PaymentQ.PaymentID "
			"		LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON LineItemT.ID = Chargebacks_AdjQ.AdjustmentID "
			"		WHERE Type IN (1,2)"
			"			AND Deleted = 0"
			"			AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL "
			"			AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL "
			"			AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID IS NULL "
			"			AND Chargebacks_PaymentQ.PaymentID Is Null "
			"			AND Chargebacks_AdjQ.AdjustmentID Is Null "
			"	)"
			") SubQ");

		// (j.jones 2014-07-14 16:08) - PLID 62876 - chargebacks cannot be reversed, so they are hidden in this screen
		m_pdlPaymentList->FromClause = _bstr_t(
			"("
			"	SELECT LineItemT.ID AS LineItemID, LineItemT.PatientID, PersonT.Last, PersonT.First, PersonT.Middle, LineItemT.Date, LineItemT.Type, LineItemT.Description, InsuranceCoT.Name, LineItemT.Amount, AppliedChargesQ.AppliedAmount, AppliedChargesQ.ChargeDate, AppliedChargesQ.ItemCode, AppliedChargesQ.ChargeDescription"
			"	FROM LineItemT"
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID"
			"	INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID"
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID"
			"	INNER JOIN PersonT ON InsuredPartyT.PatientID = PersonT.ID"
			"	LEFT JOIN ("
			"		SELECT AppliesT.SourceID, AppliesT.Amount AS AppliedAmount, LineItemT.Date AS ChargeDate, ChargesT.ItemCode, LineItemT.Description AS ChargeDescription"
			"		FROM AppliesT"
			"		INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID"
			"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID"
			"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID"
			"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID"
			"		LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON AppliesT.DestID = Chargebacks_PaymentQ.PaymentID "
			"		LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON AppliesT.DestID = Chargebacks_AdjQ.AdjustmentID "
			"		WHERE LineItemT.Deleted = 0"
			"			AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL "
			"			AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL "
			"			AND Chargebacks_PaymentQ.PaymentID Is Null "
			"			AND Chargebacks_AdjQ.AdjustmentID Is Null "
			"	) AppliedChargesQ ON PaymentsT.ID = AppliedChargesQ.SourceID"
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID"
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID"
			"	LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID"
			"	LEFT JOIN (SELECT PaymentID FROM ChargebacksT) AS Chargebacks_PaymentQ ON LineItemT.ID = Chargebacks_PaymentQ.PaymentID "
			"	LEFT JOIN (SELECT AdjustmentID FROM ChargebacksT) AS Chargebacks_AdjQ ON LineItemT.ID = Chargebacks_AdjQ.AdjustmentID "
			"	WHERE LineItemT.Type IN (1,2)"
			"		AND LineItemT.Deleted = 0"
			"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL "
			"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL "
			"		AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID IS NULL "
			"		AND Chargebacks_PaymentQ.PaymentID Is Null "
			"		AND Chargebacks_AdjQ.AdjustmentID Is Null "
			") SubQ");

		m_pdlPatientList->Requery();

		CDontShowDlg(this).DoModal(
			"This feature should be used when the payment has been reversed by the insurance company.\r\n\r\n"
			"The following actions will occur:\r\n"
			"\r\n"
			"- A reverse payment will be created and applied to the original payment to offset its amount.\r\n"
			"- The payment amount will be credited towards the selected batch payment.\r\n"
			"- If a payment is linked with a quote, it will be unlinked.",
			"BatchPaymentsInsuranceReversalInstructions");


	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CBatchPaymentInsReversalDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REVERSAL_SELECT, &CBatchPaymentInsReversalDlg::OnBnClickedReversalSelect)
	ON_BN_CLICKED(IDC_REVERSAL_DESELECT, &CBatchPaymentInsReversalDlg::OnBnClickedReversalDeselect)
	ON_BN_CLICKED(IDOK, &CBatchPaymentInsReversalDlg::OnOk)
END_MESSAGE_MAP()

// (j.armen 2012-05-24 17:42) - PLID 50520 - handler for reversal select btn
void CBatchPaymentInsReversalDlg::OnBnClickedReversalSelect()
{
	try
	{
		SelectLineItem(m_pdlPaymentList->GetFirstSelRow()->GetValue(PaymentList::LineItemID));
		EnsureControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:42) - PLID 50520 - handler for reversal deselect btn
void CBatchPaymentInsReversalDlg::OnBnClickedReversalDeselect()
{
	try
	{
		DeselectLineItem(m_pdlSelectedList->GetFirstSelRow()->GetValue(PaymentList::LineItemID));
		EnsureControlState();
	}NxCatchAll(__FUNCTION__);
}



BEGIN_EVENTSINK_MAP(CBatchPaymentInsReversalDlg, CNxDialog)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_PATIENT, 1, CBatchPaymentInsReversalDlg::SelChangingEnsureSelection, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_PATIENT, 16, CBatchPaymentInsReversalDlg::SelChosenReversalPatient, VTS_DISPATCH)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_PATIENT, 18, CBatchPaymentInsReversalDlg::RequeryFinishedReversalPatient, VTS_I2)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_PAYMENT_ADJUSTMENT, 1, CBatchPaymentInsReversalDlg::SelChangingEnsureSelection, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_PAYMENT_ADJUSTMENT, 3, CBatchPaymentInsReversalDlg::DblClickCellReversalPaymentAdjustment, VTS_DISPATCH VTS_I2)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_PAYMENT_ADJUSTMENT, 18, CBatchPaymentInsReversalDlg::RequeryFinishedReversalPaymentAdjustment, VTS_I2)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_SELECTED_PAYMENT_ADJUSTMENT, 1, CBatchPaymentInsReversalDlg::SelChangingEnsureSelection, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBatchPaymentInsReversalDlg, IDC_REVERSAL_SELECTED_PAYMENT_ADJUSTMENT, 3, CBatchPaymentInsReversalDlg::DblClickCellReversalSelectedPaymentAdjustment, VTS_DISPATCH VTS_I2)	
END_EVENTSINK_MAP()

// (j.armen 2012-05-24 17:43) - PLID 50520 - handle patient list requery finishing
void CBatchPaymentInsReversalDlg::RequeryFinishedReversalPatient(short nFlags)
{
	try
	{
		m_pdlPatientList->DropDownWidth = GetSystemMetrics(SM_CXVSCROLL) + 2;
		for(short i = 0; i < m_pdlPatientList->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = m_pdlPatientList->GetColumn(i);
			if(!(pCol->ColumnStyle & csFixedWidth && pCol->StoredWidth == 0))
				m_pdlPatientList->DropDownWidth += pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_TRUE);
		}

		if(!m_pdlPatientList->CurSel)
			if(!m_pdlPatientList->SetSelByColumn(PatientList::PatientID, GetActivePatientID()))
				m_pdlPatientList->CurSel = m_pdlPatientList->GetFirstRow();

		IRowSettingsPtr pRow = m_pdlPatientList->CurSel;

		if(pRow)
		{
			m_pdlPaymentList->WhereClause = _bstr_t(
				FormatString("PatientID = %li", VarLong(pRow->GetValue(PatientList::PatientID))));

			EnsureControlState();
			m_pdlPaymentList->Requery();
		}
		else
		{
			EnsureControlState();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:43) - PLID 50520 - handle sel chosen of the patient list
void CBatchPaymentInsReversalDlg::SelChosenReversalPatient(LPDISPATCH lpRow)
{
	try
	{
		// (j.armen 2012-05-24 17:43) - PLID 50520 - Get a list of currently selected line item id's.
		//	we do not want them to show
		CArray<long> aryLineItemIDs;
		GetSelectedLineItemIDs(aryLineItemIDs);

		m_pdlPaymentList->WhereClause = _bstr_t(
			CSqlFragment(
				"PatientID = {INT} AND LineItemID NOT IN ({INTARRAY})", 
				VarLong(m_pdlPatientList->CurSel->GetValue(PatientList::PatientID)),
				aryLineItemIDs
			).Flatten());
		
		EnsureControlState();
		m_pdlPaymentList->Requery();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:43) - PLID 50520 - handle requery finished on unselected list
void CBatchPaymentInsReversalDlg::RequeryFinishedReversalPaymentAdjustment(short nFlags)
{
	try
	{
		m_pdlPaymentList->CurSel = m_pdlPaymentList->GetFirstRow();
		EnsureControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:44) - PLID 50520 - handle double click on unselected list
void CBatchPaymentInsReversalDlg::DblClickCellReversalPaymentAdjustment(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		SelectLineItem(IRowSettingsPtr(lpRow)->GetValue(PaymentList::LineItemID));
		EnsureControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:44) - PLID 50520 - handle double click on selected list
void CBatchPaymentInsReversalDlg::DblClickCellReversalSelectedPaymentAdjustment(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		DeselectLineItem(IRowSettingsPtr(lpRow)->GetValue(PaymentList::LineItemID));
		EnsureControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:44) - PLID 50520 - called by all datalists in window to ensure selection
void CBatchPaymentInsReversalDlg::SelChangingEnsureSelection(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if(!*lppNewSel) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:45) - PLID 50520 - handle ok for processing takebacks
void CBatchPaymentInsReversalDlg::OnOk()
{
	try
	{
		//do not allow re-correcting an already corrected payment
		for(IRowSettingsPtr pRow = m_pdlSelectedList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
		{
			switch(GetLineItemCorrectionStatus(VarLong(pRow->GetValue(PaymentList::LineItemID)))) {
				case licsOriginal:
				case licsVoid:
					AfxMessageBox("The selected list of reversals contains items that have already been voided or corrected.  These can no longer be modified.  Please remove it from the list before continuing.");
					ValidateLineItemCorrectionStatus();
					return;
				default:
					break;
			}

		}

		if(IDYES == AfxMessageBox("These insurance reversals are only reversible using the Undo Correction feature.\nAre you sure you want to perform these reversals?", MB_YESNO))
		{
			CFinancialCorrection finCor;

			const CString strUsername = GetCurrentUserName();
			const long nCurrUserID = GetCurrentUserID();

			CArray<long> aryLineItemIDs;

			GetSelectedLineItemIDs(aryLineItemIDs);

			if(ReturnsRecordsParam("SELECT ID FROM PaymentsT WHERE ID IN ({INTARRAY}) AND BatchPaymentID = {INT}", aryLineItemIDs, m_nBatchPaymentID))
				if(IDNO == AfxMessageBox("At least one of the selected payments is in the same batch payment as the reversing batch payment.  Are you sure this is correct?", MB_YESNO|MB_ICONEXCLAMATION))
					return;

			// (j.jones 2012-10-02 09:15) - PLID 52529 - load the takeback batch payment check number and date
			CString strBatchPaymentCheckNo = "", strBatchPaymentDate = "";
			if(m_nBatchPaymentID != -1) {
				_RecordsetPtr rsBatchPay = CreateParamRecordset("SELECT CheckNo, Date FROM BatchPaymentsT WHERE ID = {INT}", m_nBatchPaymentID);
				if(!rsBatchPay->eof) {
					strBatchPaymentCheckNo = VarString(rsBatchPay->Fields->Item["CheckNo"]->Value, "");
					strBatchPaymentCheckNo.TrimLeft(); strBatchPaymentCheckNo.TrimRight();
					strBatchPaymentDate = FormatDateTimeForInterface(VarDateTime(rsBatchPay->Fields->Item["Date"]->Value), NULL, dtoDate);
				}
				rsBatchPay->Close();
			}

			//void, but do not correct, and track this batch payment ID so the voided amount is credited towards the batch payment balance
			// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date
			for(int i = 0; i < aryLineItemIDs.GetSize(); i++) {
				finCor.AddCorrection(ctPayment, aryLineItemIDs[i], strUsername, nCurrUserID, FALSE,
					m_nBatchPaymentID, strBatchPaymentCheckNo, strBatchPaymentDate);
			}

			//void, but do not correct, and do not credit towards the batch payment balance
			// (c.haag 2013-10-23) - PLID 59147 - Don't add applied line items to the financial corrections object. CFinancialCorrection
			// already factors in applies and does so correctly; see CFinancialCorrection::GeneratePaymentCorrectionSql.
			//_RecordsetPtr prs = CreateParamRecordset("SELECT SourceID FROM AppliesT WHERE DestID IN ({INTARRAY}) GROUP BY SourceID", aryLineItemIDs);
			//for(; !prs->eof; prs->MoveNext())
			//	finCor.AddCorrection(ctPayment, AdoFldLong(prs, "SourceID"), strUsername, nCurrUserID, FALSE);

			finCor.ExecuteCorrections(FALSE);
			
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}