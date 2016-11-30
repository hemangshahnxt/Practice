// MultipleAdjustmentEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultipleAdjustmentEntryDlg.h"

// (j.jones 2012-05-07 09:58) - PLID 50215 - created

// CMultipleAdjustmentEntryDlg dialog

using namespace NXDATALIST2Lib;

enum AdjustmentListColumns
{
	alcAmount = 0,
	alcGroupCodeID,
	alcReasonCodeID,
};

IMPLEMENT_DYNAMIC(CMultipleAdjustmentEntryDlg, CNxDialog)

CMultipleAdjustmentEntryDlg::CMultipleAdjustmentEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultipleAdjustmentEntryDlg::IDD, pParent)
{
	m_cyChargeTotal = COleCurrency(0,0);
	m_dtDateOfService = g_cdtInvalid;
	m_cyInsResp = COleCurrency(0,0);
	m_cyInsBalance = COleCurrency(0,0);
}

CMultipleAdjustmentEntryDlg::~CMultipleAdjustmentEntryDlg()
{
}

void CMultipleAdjustmentEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ADD_ADJUSTMENT, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_REMOVE_ADJUSTMENT, m_btnRemove);
	DDX_Control(pDX, IDC_LABEL_ADJUSTMENT_TOTAL, m_nxstaticAdjTotal);
	DDX_Control(pDX, IDC_LABEL_CHARGE_TYPE, m_nxstaticChargeType);
	DDX_Control(pDX, IDC_LABEL_CHARGE_CODE, m_nxstaticChargeCode);
	DDX_Control(pDX, IDC_LABEL_CHARGE_AMOUNT, m_nxstaticChargeAmount);
	DDX_Control(pDX, IDC_LABEL_CHARGE_DATE, m_nxstaticChargeDate);
	DDX_Control(pDX, IDC_LABEL_INS_RESP, m_nxstaticInsResp);
}


BEGIN_MESSAGE_MAP(CMultipleAdjustmentEntryDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_ADD_ADJUSTMENT, OnBtnAddAdjustment)
	ON_BN_CLICKED(IDC_BTN_REMOVE_ADJUSTMENT, OnBtnRemoveAdjustment)
END_MESSAGE_MAP()

// CMultipleAdjustmentEntryDlg message handlers
BOOL CMultipleAdjustmentEntryDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);

		m_List = BindNxDataList2Ctrl(IDC_MULTI_ADJUSTMENT_LIST, false);

		//set the combo box SQL
		IColumnSettingsPtr pGroupCodeCol = m_List->GetColumn(alcGroupCodeID);
		pGroupCodeCol->PutComboSource("SELECT -1 AS ID, ' <Use Default Group Code>' AS Name "
			"UNION SELECT ID, Code + ' - ' + Convert(nvarchar(4000), Description) AS Name "
			"FROM AdjustmentCodesT "
			"WHERE Type = 1 AND Inactive = 0 ORDER BY Name ASC");

		IColumnSettingsPtr pReasonCodeCol = m_List->GetColumn(alcReasonCodeID);
		pReasonCodeCol->PutComboSource("SELECT -1 AS ID, ' <Use Default Reason Code>' AS Name "
			"UNION SELECT ID, Code + ' - ' + Convert(nvarchar(4000), Description) AS Name "
			"FROM AdjustmentCodesT "
			"WHERE Type = 2 AND Inactive = 0 ORDER BY Name ASC");

		//load the data from the array
		for(int i=0; i<m_aryAdjustmentInfo.GetSize(); i++) {
			AdjustmentInfo* pAdj = (AdjustmentInfo*)m_aryAdjustmentInfo.GetAt(i);
			IRowSettingsPtr pRow = m_List->GetNewRow();
			pRow->PutValue(alcAmount, _variant_t(pAdj->cyAmount));
			pRow->PutValue(alcGroupCodeID, pAdj->nGroupCodeID);
			pRow->PutValue(alcReasonCodeID, pAdj->nReasonCodeID);
			m_List->AddRowAtEnd(pRow, NULL);
		}

		//update the labels
		m_nxstaticChargeType.SetWindowText(m_strLineType + ":");
		//If there is no item code, we're at a bit of a loss, this charge has no unique description loaded.
		//Fortunately this is only used on insurance postings and products would never be paid without codes.
		m_nxstaticChargeCode.SetWindowText(m_strItemCode);
		m_nxstaticChargeAmount.SetWindowText(FormatCurrencyForInterface(m_cyChargeTotal));
		m_nxstaticChargeDate.SetWindowText(FormatDateTimeForInterface(m_dtDateOfService, NULL, dtoDate));
		m_nxstaticInsResp.SetWindowText(FormatCurrencyForInterface(m_cyInsResp));

		//add up the adjustment total and update its label
		CalculateAdjustmentTotal();

		//update the window text
		if(!m_strInsuranceCoName.IsEmpty()) {
			CString strWindowText;
			strWindowText.Format("Edit Multiple Adjustments for %s", m_strInsuranceCoName);
			SetWindowText(strWindowText);
		}

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultipleAdjustmentEntryDlg::OnOK()
{
	try {

		//add up the total and update the label, returns false if the total is > m_cyInsResp
		COleCurrency cyTotalAdj = CalculateAdjustmentTotal();
		if(cyTotalAdj > m_cyInsResp) {
			// (j.jones 2016-05-09 15:55) - NX-100502 - overadjustments are now allowed
			CString strWarn;
			strWarn.Format("The total adjustments entered are greater than the insurance responsibility of %s.\n\n"
				"Are you sure you wish to continue?", FormatCurrencyForInterface(m_cyInsResp));
			if (IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION | MB_YESNO)) {
				return;
			}
		}

		if(m_List->GetRowCount() == 0) {
			if(IDNO == MessageBox("No adjustments have been entered. Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		else if(cyTotalAdj == COleCurrency(0,0)) {
			if(IDNO == MessageBox("The adjustment total is zero. These adjustments will be posted as zero if you continue.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//clear the existing content
		for(int i=0; i<m_aryAdjustmentInfo.GetSize(); i++) {
			AdjustmentInfo* pAdj = (AdjustmentInfo*)m_aryAdjustmentInfo.GetAt(i);
			if(pAdj) {
				delete pAdj;
			}
		}
		m_aryAdjustmentInfo.RemoveAll();

		//make new pointers for our new content to save
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {
			AdjustmentInfo* pAdj = new AdjustmentInfo;
			pAdj->cyAmount = VarCurrency(pRow->GetValue(alcAmount), COleCurrency(0,0));
			pAdj->nGroupCodeID = VarLong(pRow->GetValue(alcGroupCodeID), -1);
			pAdj->nReasonCodeID = VarLong(pRow->GetValue(alcReasonCodeID), -1);
			m_aryAdjustmentInfo.Add(pAdj);

			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CMultipleAdjustmentEntryDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CMultipleAdjustmentEntryDlg::OnBtnAddAdjustment()
{
	try {

		//add a new line and start editing the amount
		IRowSettingsPtr pRow = m_List->GetNewRow();
		pRow->PutValue(alcAmount, _variant_t(COleCurrency(0,0)));
		pRow->PutValue(alcGroupCodeID, (long)-1);
		pRow->PutValue(alcReasonCodeID, (long)-1);
		m_List->AddRowAtEnd(pRow, NULL);
		m_List->StartEditing(pRow, alcAmount);

	}NxCatchAll(__FUNCTION__);
}

void CMultipleAdjustmentEntryDlg::OnBtnRemoveAdjustment()
{
	try {

		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("Please select an adjustment before removing.");
			return;
		}

		//if I used the accelerator to remove the row, the datalist freaked out if I was still editing the cell
		m_List->StopEditing(VARIANT_FALSE);

		//this isn't in data yet, so there is no need to warn
		m_List->RemoveRow(pRow);

		//recalculate the total
		CalculateAdjustmentTotal();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CMultipleAdjustmentEntryDlg, CNxDialog)
	ON_EVENT(CMultipleAdjustmentEntryDlg, IDC_MULTI_ADJUSTMENT_LIST, 6, OnRButtonDownMultiAdjustmentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultipleAdjustmentEntryDlg, IDC_MULTI_ADJUSTMENT_LIST, 9, OnEditingFinishingMultiAdjustmentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMultipleAdjustmentEntryDlg, IDC_MULTI_ADJUSTMENT_LIST, 10, OnEditingFinishedMultiAdjustmentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CMultipleAdjustmentEntryDlg::OnRButtonDownMultiAdjustmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_List->CurSel = pRow;

		//add an ability to remove this row
		enum {
			eRemoveAdj = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveAdj, "&Remove Adjustment");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveAdj) {
			OnBtnRemoveAdjustment();
		}

	}NxCatchAll(__FUNCTION__);
}

void CMultipleAdjustmentEntryDlg::OnEditingFinishingMultiAdjustmentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == alcAmount) {
			if(*pbCommit) {
				COleCurrency cyNewAmt = COleCurrency(pvarNewValue->cyVal);

				// (j.jones 2016-05-11 8:45) - NX-100503 - you can now enter negative adjustments
				/*
				if(cyNewAmt < COleCurrency(0,0)) {
					AfxMessageBox("You cannot enter a negative adjustment in line item posting.");
					*pbCommit = FALSE;
					return;
				}
				*/

				//Don't ever allow one adjustment to be larger than the insurance resp. we're applying to.
				//They can temporarily enter a *total* greater than this amount, but they won't be able to exit the dialog.
				// (j.jones 2016-05-09 15:55) - NX-100502 - overadjustments are now allowed
				if(cyNewAmt > m_cyInsResp) {
					CString strWarn;					
					strWarn.Format("The adjustment you entered is greater than the current insurance responsibility of %s.\n\n"
						"Are you sure you wish to continue?", FormatCurrencyForInterface(m_cyInsResp));					
					if (IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION | MB_YESNO)) {
						*pbCommit = FALSE;
						return;
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CMultipleAdjustmentEntryDlg::OnEditingFinishedMultiAdjustmentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2016-05-09 15:55) - NX-100502 - overadjustments are now allowed, so there is nothing to warn about

	}NxCatchAll(__FUNCTION__);
}

//add up the total and update the label, returns the total amount
COleCurrency CMultipleAdjustmentEntryDlg::CalculateAdjustmentTotal()
{
	COleCurrency cyTotal = COleCurrency(0,0);

	try {

		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {
			COleCurrency cyAmount = VarCurrency(pRow->GetValue(alcAmount), COleCurrency(0,0));

			cyTotal += cyAmount;

			pRow = pRow->GetNextRow();
		}

		m_nxstaticAdjTotal.SetWindowText(FormatCurrencyForInterface(cyTotal));

	}NxCatchAll(__FUNCTION__);

	return cyTotal;
}