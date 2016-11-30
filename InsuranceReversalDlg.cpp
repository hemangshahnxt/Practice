#include "stdafx.h"
#include "Practice.h"
#include "InsuranceReversalDlg.h"
#include "FinancialCorrection.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (j.jones 2015-10-16 15:01) - PLID 67382 - capitation payments are not available for reversals
#define SQL_WHERE_FILTER	"dbo.AsDateNoTime(Date) = " \
							"	CASE WHEN EXISTS" \
							"		(SELECT ID FROM BatchPaymentsT WHERE Type = 1 AND Deleted = 0 AND Capitation = 0 AND dbo.AsDateNoTime(Date) = dbo.AsDateNoTime(GETDATE()))" \
							"		THEN dbo.AsDateNoTime(GETDATE())" \
							"	ELSE " \
							"		(SELECT dbo.AsDateNoTime(MAX(Date)) FROM BatchPaymentsT WHERE Type = 1 AND Deleted = 0 AND Capitation = 0)" \
							"	END" \

namespace BatchPayments
{
	enum {
		ID,
		Date,
		InsCoName,
		Description,
		CheckNo,
		Amount,
		Balance,
	};
}

IMPLEMENT_DYNAMIC(CInsuranceReversalDlg, CNxDialog)

CInsuranceReversalDlg::CInsuranceReversalDlg(CWnd* pParent, const long& nPaymentID)
	: CNxDialog(CInsuranceReversalDlg::IDD, pParent), m_nPaymentID(nPaymentID)
{
}

CInsuranceReversalDlg::~CInsuranceReversalDlg()
{
}

void CInsuranceReversalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REVERSAL_SHOW_ALL_DATES, m_btnShowAllDates);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CInsuranceReversalDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REVERSAL_SHOW_ALL_DATES, &CInsuranceReversalDlg::OnBnClickedReversalShowAllDates)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInsuranceReversalDlg, CNxDialog)
	ON_EVENT(CInsuranceReversalDlg, IDC_NXDL_BATCH_PAYMENTS, 2, CInsuranceReversalDlg::SelChangedNxdlBatchPayments, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CInsuranceReversalDlg, IDC_NXDL_BATCH_PAYMENTS, 18, CInsuranceReversalDlg::RequeryFinishedNxdlBatchPayments, VTS_I2)
END_EVENTSINK_MAP()

// (j.armen 2012-05-10 13:57) - PLID 14102 - Init Dlg
BOOL CInsuranceReversalDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try
	{
		g_propManager.CachePropertiesInBulk("InsuranceReversalDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name = "
			"	'InsuranceReversalShowAllDates'"
			, _Q(GetCurrentUserName()));

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pdlBatchPayments = BindNxDataList2Ctrl(IDC_NXDL_BATCH_PAYMENTS, false);

		// (j.jones 2015-10-16 15:01) - PLID 67382 - capitation payments are not available for reversals
		m_pdlBatchPayments->PutFromClause("(SELECT BatchPaymentsT.ID, BatchPaymentsT.Type, InsuranceCoT.PersonID, "
			"BatchPaymentsT.Amount, BatchPaymentsT.OriginalAmount, BatchPaymentsT.Description, "
			"BatchPaymentsT.CheckNo, BatchPaymentsT.Date, InsuranceCoT.Name, "
			"Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) AS AppliedAmount, "
			"BatchPaymentsT.Amount "
			" - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
			" + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
			" - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
			" AS RemainingAmount, "
			"InsuranceCoT.HCFASetupGroupID, Location, PayCatID, "
			"BatchPaymentsT.ProviderID, BatchPaymentsT.InputDate, HCFASetupT.Name AS HCFAGroupName "
			"FROM BatchPaymentsT "
			""
			//find child payments that are not voided, but include them if they are part of a takeback
			"LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
			"	FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
			"	AND PaymentsT.BatchPaymentID Is Not Null "
			"	GROUP BY PaymentsT.BatchPaymentID "
			") AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
			""
			//find payments that were part of takebacks, crediting this batch payment
			"LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
			"	FROM LineItemT "
			"	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"	AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
			"	GROUP BY LineItemCorrectionsT.BatchPaymentID "
			") AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
			""
			//find the batch payment's adjustments or refunds
			"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID "
			"	FROM BatchPaymentsT "
			"	WHERE Type <> 1 AND Deleted = 0 "
			"	GROUP BY AppliedBatchPayID, Deleted "
			") AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
			""
			"LEFT JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			""
			"WHERE BatchPaymentsT.Type = 1 AND BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.Capitation = 0 "
			"GROUP BY BatchPaymentsT.ID, BatchPaymentsT.Type, InsuranceCoT.PersonID, BatchPaymentsT.Amount, "
			"BatchPaymentsT.OriginalAmount, LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied, BatchPaymentsT.Description, "
			"BatchPaymentsT.CheckNo, BatchPaymentsT.Date, InsuranceCoT.Name, InsuranceCoT.HCFASetupGroupID, "
			"BatchPaymentsT.Deleted, Location, PayCatID, BatchPaymentsT.ProviderID, BatchPaymentsT.InputDate, "
			"HCFASetupT.Name "
			") AS BatchPaymentsQ");

		if(GetRemotePropertyInt("InsuranceReversalShowAllDates", 0, propNumber, GetCurrentUserName()))
		{
			m_pdlBatchPayments->WhereClause = _bstr_t("");
			m_btnShowAllDates.SetCheck(BST_CHECKED);
		}
		else
		{
			m_pdlBatchPayments->WhereClause = _bstr_t(SQL_WHERE_FILTER);
			m_btnShowAllDates.SetCheck(BST_UNCHECKED);
		}

		m_pdlBatchPayments->Requery();
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}


// (j.armen 2012-05-10 13:57) - PLID 14102 - Process Reversal
void CInsuranceReversalDlg::OnOK()
{
	try
	{
		if(IDYES == AfxMessageBox("This insurance reversal is only reversible using the Undo Correction feature.\nAre you sure you want to perform this reversal?", MB_YESNO))
		{
			CFinancialCorrection finCor;

			const CString strUsername = GetCurrentUserName();
			const long nCurrUserID = GetCurrentUserID();
			const long nBatchPaymentID = VarLong(m_pdlBatchPayments->GetCurSel()->GetValue(BatchPayments::ID));

			if(ReturnsRecordsParam("SELECT ID FROM PaymentsT WHERE ID = {INT} AND BatchPaymentID = {INT}", m_nPaymentID, nBatchPaymentID))
			{
				if(IDNO == AfxMessageBox("The reversing batch payment you have selected is the same batch payment responsible for the original payment.  Are you sure this is correct?", MB_YESNO|MB_ICONEXCLAMATION))
				{
					return;
				}
			}

			// (j.jones 2012-10-02 09:15) - PLID 52529 - load the takeback batch payment check number and date
			CString strBatchPaymentCheckNo = "", strBatchPaymentDate = "";
			if(nBatchPaymentID != -1) {
				_RecordsetPtr rsBatchPay = CreateParamRecordset("SELECT CheckNo, Date FROM BatchPaymentsT WHERE ID = {INT}", nBatchPaymentID);
				if(!rsBatchPay->eof) {
					strBatchPaymentCheckNo = VarString(rsBatchPay->Fields->Item["CheckNo"]->Value, "");
					strBatchPaymentCheckNo.TrimLeft(); strBatchPaymentCheckNo.TrimRight();
					strBatchPaymentDate = FormatDateTimeForInterface(VarDateTime(rsBatchPay->Fields->Item["Date"]->Value), NULL, dtoDate);
				}
				rsBatchPay->Close();
			}

			//void, but do not correct, and track this batch payment ID so the voided amount is credited towards the batch payment balance
			// (j.jones 2012-10-02 09:15) - PLID 52529 - added the takeback batch payment check number and date
			finCor.AddCorrection(ctPayment, m_nPaymentID, strUsername, nCurrUserID, FALSE,
				nBatchPaymentID, strBatchPaymentCheckNo, strBatchPaymentDate);

			// (c.haag 2013-10-23) - PLID 59147 - Don't add applied line items to the financial corrections object. CFinancialCorrection
			// already factors in applies and does so correctly; see CFinancialCorrection::GeneratePaymentCorrectionSql.
			//_RecordsetPtr prs = CreateParamRecordset("SELECT SourceID FROM AppliesT WHERE DestID = {INT} GROUP BY SourceID", m_nPaymentID);
			//for(; !prs->eof; prs->MoveNext())
			//{
			//	//void, but do not correct, and do not credit towards the batch payment balance
			//	finCor.AddCorrection(ctPayment, AdoFldLong(prs, "SourceID"), strUsername, nCurrUserID, FALSE);
			//}

			finCor.ExecuteCorrections(FALSE);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-10 13:58) - PLID 14102 - Handle disableing OK if there are no items in the dropdown.
//	Also resize the dropdown based on data
void CInsuranceReversalDlg::RequeryFinishedNxdlBatchPayments(short nFlags)
{
	try
	{
		IRowSettingsPtr pRow = m_pdlBatchPayments->GetCurSel();

		if(!pRow && m_pdlBatchPayments->GetTopRow())
			m_pdlBatchPayments->PutCurSel(m_pdlBatchPayments->GetTopRow());
		else if(!pRow)
			m_btnOK.EnableWindow(FALSE);

		ResizeBatchPaymentsDropdown();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-10 13:58) - PLID 14102 - Ensure that we have a valid selection
void CInsuranceReversalDlg::SelChangedNxdlBatchPayments(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		if(!lpNewSel)
			m_pdlBatchPayments->PutCurSel(IRowSettingsPtr(lpOldSel));
		else
			m_btnOK.EnableWindow(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-10 13:58) - PLID 14102 - Handle sizing the dropdown to data width
void CInsuranceReversalDlg::ResizeBatchPaymentsDropdown()
{
	long nDropDownWidth = 0;
	for(short i = 0; i < m_pdlBatchPayments->GetColumnCount(); i++)
	{
		if(i == BatchPayments::ID)	continue;
		long nColWidth = m_pdlBatchPayments->GetColumn(i)->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
		m_pdlBatchPayments->GetColumn(i)->PutStoredWidth(nColWidth);
		nDropDownWidth += nColWidth;
	}

	m_pdlBatchPayments->PutDropDownWidth(nDropDownWidth + GetSystemMetrics(SM_CXVSCROLL) + 2);
}

// (j.armen 2012-05-15 15:23) - PLID 14102 - Handle clicking show all dates button
void CInsuranceReversalDlg::OnBnClickedReversalShowAllDates()
{
	try
	{
		if(m_btnShowAllDates.GetCheck() == BST_CHECKED)
		{
			SetRemotePropertyInt("InsuranceReversalShowAllDates", 1, propNumber, GetCurrentUserName());
			m_pdlBatchPayments->WhereClause = _bstr_t("");
		}
		else
		{
			SetRemotePropertyInt("InsuranceReversalShowAllDates", 0, propNumber, GetCurrentUserName());
			m_pdlBatchPayments->WhereClause = _bstr_t(SQL_WHERE_FILTER);
		}

		m_pdlBatchPayments->Requery();
			
	}NxCatchAll(__FUNCTION__);
}