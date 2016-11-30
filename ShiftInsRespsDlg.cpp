// ShiftInsRespsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ShiftInsRespsDlg.h"
#include "GlobalFinancialUtils.h"
#include "CalculatePercentageDlg.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CShiftInsRespsDlg dialog


CShiftInsRespsDlg::CShiftInsRespsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CShiftInsRespsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShiftInsRespsDlg)
		m_cySrcAmount = COleCurrency(0,0);
		m_cyDestResp = COleCurrency(0,0);
		m_nRevenueCode = -1;
		m_bWarnWhenShiftingEntireBalance = false;
		m_bDoNotSwitchClaimBatches = false;
	//}}AFX_DATA_INIT
}


void CShiftInsRespsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShiftInsRespsDlg)
	DDX_Control(pDX, IDC_CHECK_BATCH_CLAIM, m_checkBatchClaim);
	DDX_Control(pDX, IDC_CHECK_SWAP_INS_COS, m_checkSwapInsCos);
	DDX_Control(pDX, IDC_EDIT_SHIFT_AMOUNT, m_nxeditEditShiftAmount);
	DDX_Control(pDX, IDC_LABEL_DEST_RESP, m_nxstaticLabelDestResp);
	DDX_Control(pDX, IDC_LABEL_DEST_RESP_AMOUNT, m_nxstaticLabelDestRespAmount);
	DDX_Control(pDX, IDC_LABEL_SOURCE_RESP, m_nxstaticLabelSourceResp);
	DDX_Control(pDX, IDC_LABEL_SOURCE_RESP_AMOUNT, m_nxstaticLabelSourceRespAmount);
	DDX_Control(pDX, IDC_LABEL_AMOUNT_TO_SHIFT, m_nxstaticLabelAmountToShift);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_SHIFT, m_nxstaticCurrencySymbolShift);
	DDX_Control(pDX, IDC_LABEL_DATE_OF_SHIFT, m_nxstaticLabelDateOfShift);
	DDX_Control(pDX, IDC_BTN_SHIFT, m_btnShift);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CALC_PERCENT, m_btnCalcPercent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShiftInsRespsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CShiftInsRespsDlg)
	ON_BN_CLICKED(IDC_BTN_SHIFT, OnBtnShift)
	ON_BN_CLICKED(IDC_CALC_PERCENT, OnCalcPercent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShiftInsRespsDlg message handlers

BOOL CShiftInsRespsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-02 09:55) - PLID 29879 - NxIconify the buttons
		m_btnShift.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCalcPercent.AutoSet(NXB_MODIFY);

		GetMainFrame()->DisableHotKeys();

		// (j.jones 2013-08-21 08:44) - PLID 58194 - this should never be called with an empty audit string
		ASSERT(!m_strAuditFromProcess.IsEmpty());

		m_pFont = new CFont;
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pFont, 120, "Arial");

		GetDlgItem(IDC_LABEL_DEST_RESP_AMOUNT)->SetFont(m_pFont);
		GetDlgItem(IDC_LABEL_SOURCE_RESP_AMOUNT)->SetFont(m_pFont);

		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, strICurr.GetBuffer(2), 2, TRUE);
		strICurr.ReleaseBuffer();
		strICurr.TrimRight();
		if(strICurr == "2") {
			SetDlgItemText(IDC_CURRENCY_SYMBOL_SHIFT,GetCurrencySymbol() + " ");
		}
		else {
			SetDlgItemText(IDC_CURRENCY_SYMBOL_SHIFT,GetCurrencySymbol());
		}

		//set the labels
		CString strResp;
		strResp.Format("Source (%s)", m_nSrcInsPartyID == -1 ? "Patient" : GetNameFromRespPartyID(m_nSrcInsPartyID));
		SetDlgItemText(IDC_LABEL_SOURCE_RESP, strResp);

		strResp.Format("Destination (%s)", m_nDstInsPartyID == -1 ? "Patient" : GetNameFromRespPartyID(m_nDstInsPartyID));
		SetDlgItemText(IDC_LABEL_DEST_RESP, strResp);

		//load the total source resp. amount
		SetDlgItemText(IDC_LABEL_SOURCE_RESP_AMOUNT,FormatCurrencyForInterface(m_cySrcAmount));
		SetDlgItemText(IDC_EDIT_SHIFT_AMOUNT,FormatCurrencyForInterface(m_cySrcAmount,FALSE,TRUE));

		m_nxtTime = BindNxTimeCtrl(this, IDC_DATE_OF_SHIFT);
		m_BatchList = BindNxDataListCtrl(this,IDC_BATCH_LIST,GetRemoteData(),false);
		IRowSettingsPtr pRow = m_BatchList->GetRow(-1);
		pRow->PutValue(0,(long)0);
		pRow->PutValue(1,_bstr_t("Unbatched"));
		m_BatchList->AddRow(pRow);
		pRow = m_BatchList->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("Paper"));
		m_BatchList->AddRow(pRow);
		pRow = m_BatchList->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Electronic"));
		m_BatchList->AddRow(pRow);
		//get the default of the destination company
		int iBatch = 0;
		// (j.jones 2015-10-29 16:59) - PLID 67431 - added ability to skip batching the claim
		if(m_nDstInsPartyID > -1 && !m_bDoNotSwitchClaimBatches) {
			// (j.jones 2012-01-17 11:59) - PLID 47510 - do not try to force paper,
			// instead just accept the default return value
			iBatch = FindDefaultHCFABatch(m_nDstInsPartyID);
			m_BatchList->SetSelByColumn(0,(long)iBatch);
		}
		else {
			m_BatchList->Enabled = FALSE;
		}

		if(m_nDstInsPartyID > -1 && !m_bDoNotSwitchClaimBatches) {
			//if switching to an insurance company
			if(GetRemotePropertyInt("SwapInsuranceCos",1,0,"<None>",TRUE) == 1) {
				m_checkSwapInsCos.SetCheck(TRUE);
			}
		}
		else {
			m_checkSwapInsCos.EnableWindow(FALSE);
		}

		// (j.jones 2015-10-29 16:59) - PLID 67431 - added ability to skip batching the claim
		if(m_nDstInsPartyID > -1 && !m_bDoNotSwitchClaimBatches) {
			//if switching to an insurance company
			// (j.jones 2012-01-17 12:00) - PLID 47510 - do not check this if the default is to be unbatched
			// (j.jones 2012-03-13 11:02) - PLID 48687 - apparently we always want to check this, even if it's going to be unbatched
			// (d.thompson 2012-08-06) - PLID 51969 - Changed default to On
			if(GetRemotePropertyInt("AutoBatchOnSwitch",1,0,"<None>",TRUE)==1) {
				m_checkBatchClaim.SetCheck(TRUE);
			}
			else {
				m_checkBatchClaim.SetCheck(FALSE);
			}
		}
		else {
			m_checkBatchClaim.EnableWindow(FALSE);
		}

		//load the total destination resp. amount
		CString str;
		if (m_strLineType == "Bill")
			str.Format("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.PatientID = %li AND LineItemT.Deleted = 0 "
					"AND BillID = %li", m_PatientID, m_ID);
		else
			str.Format("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.PatientID = %li AND LineItemT.Deleted = 0 "
					"AND ChargesT.ID = %li", m_PatientID, m_ID);
		_RecordsetPtr rs;
		rs = CreateRecordset("%s",str);

		while (!rs->eof) {
			long ChargeID = AdoFldLong(rs, "ID");

			COleCurrency cyTotalResp, cyPayments, cyAdjustments, cyRefunds, cyBalance, cyInsurance;

			if (m_nDstInsPartyID != -1) {
				//insurance responsibility

				if (!GetChargeInsuranceTotals(ChargeID, m_PatientID, m_nDstInsPartyID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
			}
			else {
				//patient responsbility

				if (!GetChargeTotals(ChargeID, m_PatientID, &cyTotalResp, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					// This should never happen, but just in case...
					rs->MoveNext();
					continue;
				}
				cyTotalResp -= cyInsurance;
			}

			m_cyDestResp += (cyTotalResp - cyPayments - cyAdjustments - cyRefunds);

			rs->MoveNext();
		}
		rs->Close();

		SetDlgItemText(IDC_LABEL_DEST_RESP_AMOUNT,FormatCurrencyForInterface(m_cyDestResp));
		
		//load the date
		m_nxtTime->SetDateTime(COleDateTime::GetCurrentTime());
	}
	NxCatchAll("Error in CShiftInsRespsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CShiftInsRespsDlg::OnBtnShift() 
{
	try {

		long nBillID = -1;
		if(m_strLineType == "Bill") {
			nBillID = m_ID;
		}
		else if(m_strLineType == "Charge") {
			_RecordsetPtr rs = CreateRecordset("SELECT BillID FROM ChargesT WHERE ID = %li",m_ID);
			if(!rs->eof) {
				nBillID = AdoFldLong(rs, "BillID");
			}
			rs->Close();
		}

		if(m_checkBatchClaim.GetCheck() && m_BatchList->GetCurSel() == -1) {
			MsgBox("Please select a batch to send the claim to, or else uncheck the 'Send Claim To This Batch' box.");
			return;
		}

		////////////////////////////////////////////////////////
		// Make sure apply amount is not blank, has no bad characters,
		// and has no more than two places to the right of the
		// decimal point.
		////////////////////////////////////////////////////////
		CString strAmountToShift;
		GetDlgItem(IDC_EDIT_SHIFT_AMOUNT)->GetWindowText(strAmountToShift);
		if (strAmountToShift.GetLength() == 0) {
			MsgBox("Please fill in the 'Amount To Shift' box.");
			return;
		}

		if(!IsValidCurrencyText(strAmountToShift)) {
			MsgBox("Please fill correct information in the 'Amount To Shift' box.");
			return;
		}

		COleCurrency cyAmtToShift = ParseCurrencyFromInterface(strAmountToShift);
		if(cyAmtToShift.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Amount To Shift' box.");
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyAmtToShift.Format().Find(strDecimal) != -1 && cyAmtToShift.Format().Find(strDecimal) + (nDigits+1) < cyAmtToShift.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Amount To Shift' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			return;
		}

		//check that the amount they typed in is not greater than the source amount	
		if(cyAmtToShift > m_cySrcAmount) {
			MsgBox("The amount you are trying to transfer is greater than the source responsibility amount.\n"
				"Please correct this before attempting to transfer the responsibility.");
			return;
		}

		//check that the amount they typed in is greater than zero
		if(cyAmtToShift <= COleCurrency(0,0)) {
			MsgBox("You can only transfer an amount greater than zero.\n"
				"Please correct this before attempting to transfer the responsibility.");
			return;
		}

		//check the date
		//make sure the date is not in the future and that is is a valid date
		COleDateTime dtDate = m_nxtTime->GetDateTime();
		COleDateTime dtNow, dtMin, dtMax;
		dtNow = COleDateTime::GetCurrentTime();
		dtMin.SetDate(1753, 1, 1);
		dtMax.SetDate(9999, 12, 31);
		CString strDate;

		if (m_nxtTime->GetStatus() != 1)  {

			MsgBox("Please enter a complete, valid date");
			return;
		}
		else if (! (dtDate > dtMin && dtDate < dtMax && dtDate.GetStatus() == 0)) {
			MsgBox("Please enter a valid date");
			return;
		}
		else if (!(dtDate <= dtNow)) {
			MsgBox("Please enter a date that is not in the future.");
			return;
		}

		// (j.jones 2015-10-29 16:59) - PLID 67431 - for capitation, warn if they are shifting
		// the entire balance, since that is not likely to be what they want
		if (m_bWarnWhenShiftingEntireBalance && m_cySrcAmount == cyAmtToShift) {
			if (IDNO == MessageBox("You are about to shift the entire balance of this responsibility.\n\n"
				"Are you sure you wish to do this?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
				return;
			}
		}

		//before shifting, do one last check:
		//If they are shifting to an insurance responsibility, and are NOT sending to a batch,
		//and have NOT previously sent this bill to that insurance company, then warn them and give them
		//a chance to change their mind
		// (j.jones 2015-10-29 16:59) - PLID 67431 - added ability to skip batching the claim
		if(m_nDstInsPartyID > -1 && !m_checkBatchClaim.GetCheck() && !m_bDoNotSwitchClaimBatches) {
			if(IsRecordsetEmpty("SELECT ID FROM ClaimHistoryT WHERE InsuredPartyID = %li AND BillID = %li", m_nDstInsPartyID, nBillID)) {
				if(MessageBox("You are about to shift this balance to a company you haven't previously submitted this claim to.\n"
					"Are you sure you wish to do this without submitting this bill to a batch?","Practice",MB_YESNO|MB_ICONQUESTION)==IDNO) {
					return;
				}			
			}
		}

		// (j.jones 2006-12-29 10:50) - PLID 23160 - supported shifting by revenue code

		//now actually shift the balance
		// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
		ShiftInsuranceResponsibility(m_ID, m_PatientID, m_nSrcInsPartyID, m_nDstInsPartyID, m_strLineType, cyAmtToShift,
			m_strAuditFromProcess,
			dtDate, m_nRevenueCode);
		
		if(m_nDstInsPartyID > -1 && m_checkSwapInsCos.GetCheck()) {
			
			SwapInsuranceCompanies(nBillID, m_nSrcInsPartyID, m_nDstInsPartyID);
		}

		if(m_nDstInsPartyID > -1) {
			//insurance resp

			long nInsuredPartyID = -1;
			BOOL bBatchBill = FALSE;

			if(m_checkBatchClaim.GetCheck()) {
				
				_RecordsetPtr rsBill = CreateRecordset("SELECT BillsT.ID AS BillID, InsuredPartyID FROM BillsT WHERE ID = %li AND InsuredPartyID != -1",nBillID);
				if(!rsBill->eof) {
					nInsuredPartyID = AdoFldLong(rsBill, "InsuredPartyID",-1);				
				}
				rsBill->Close();

				if(nInsuredPartyID != -1) {
					bBatchBill = TRUE;
				}else {

					/* (j.jones 2003-09-16 13:46) Meikin said there's no need for a prompt, it should
					always be automated. But since I coded it, I'll just leave it in as a comment.

					CString str;
					str.Format("The claim can not be batched because no insurance company has been selected on the insurance tab of the bill.\n"
						"Would you like to use the %s insurance as the insurance company on the bill?\n\n"
						"(If not, you will need to edit the bill and select an insurance company, then re-batch the claim.)",
						GetNameFromRespPartyID(m_nDstInsPartyID));

					//no insurance company on the bill
					if(IDYES == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					*/
						ExecuteSql("UPDATE BillsT SET InsuredPartyID = %li WHERE ID = %li",m_nDstInsPartyID,nBillID);
						bBatchBill = TRUE;
					//}
				}

				if(bBatchBill) {
					int iBatch = m_BatchList->GetValue(m_BatchList->GetCurSel(),0).lVal;
					BatchBill(nBillID, iBatch);
				}
			}
		}

		CDialog::OnOK();

	}NxCatchAll("Error shifting responsibility.");
}

void CShiftInsRespsDlg::OnCalcPercent() 
{
	////////////////////////////////////////////////////////
	// Make sure apply amount is not blank, has no bad characters,
	// and has no more than two places to the right of the
	// decimal point.
	////////////////////////////////////////////////////////
	CString str;
	GetDlgItem(IDC_EDIT_SHIFT_AMOUNT)->GetWindowText(str);
	if (str.GetLength() == 0) {
		MsgBox("Please fill in the 'Amount To Shift' box.");
		return;
	}

	if(!IsValidCurrencyText(str)) {
		MsgBox("Please fill correct information in the 'Amount To Shift' box.");
		return;
	}

	CCalculatePercentageDlg dlg(this);
	dlg.m_color = 0x9CC294;
	//dlg.m_cyOriginalAmt = ParseCurrencyFromInterface(str);
	dlg.m_strOriginalAmt = str;
	if(dlg.DoModal() == IDOK) {
		str = FormatCurrencyForInterface(dlg.m_cyFinalAmt, FALSE);
		SetDlgItemText(IDC_EDIT_SHIFT_AMOUNT,str);
		((CNxEdit*)GetDlgItem(IDC_EDIT_SHIFT_AMOUNT))->SetSel(0, -1);
	}
}

BOOL CShiftInsRespsDlg::DestroyWindow() 
{
	//DRT 2/16/2004 - We need to destroy the font that we created
	//	in the OnInitDialog!
	if(m_pFont)
		delete m_pFont;

	GetMainFrame()->EnableHotKeys();
	
	return CDialog::DestroyWindow();
}
