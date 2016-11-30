// BillingExtraChargeInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillingExtraChargeInfoDlg.h"
#include "BillingDlg.h"
#include "BillingModuleDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-05-28 10:18) - PLID 28782 - created

/////////////////////////////////////////////////////////////////////////////
// CBillingExtraChargeInfoDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ChargeListColumns {

	clcLineID = 0,
	clcChargeID,
	clcServiceID,
	clcItemCode,
	clcSubCode,
	clcDescription,
	clcQuantity,
	clcUnitCost,	
	clcTotal,
	// (j.jones 2010-04-08 11:40) - PLID 15224 - added Emergency
	clcEmergency,
	clcNDCCode,
	// (j.jones 2009-08-12 18:15) - PLID 35206 - added more drug fields
	// (j.jones 2010-10-18 09:11) - PLID 32848 - Price is obsolete in 5010, the column
	// is currently width 0, but resizeable and still editable for the time being
	clcDrugUnitPrice,
	clcDrugUnitType,
	clcDrugUnitQuantity,
	clcPrescriptionNumber,
};

// (a.walling 2014-02-24 11:27) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems. Using a reference and setting in the constructor instead of a pointer.

CBillingExtraChargeInfoDlg::CBillingExtraChargeInfoDlg(CWnd* pParent, std::vector<BillingItemPtr>& billingItems, CBillingModuleDlg *pBillingModuleDlg, BOOL bIsReadOnly)
	: CNxDialog(CBillingExtraChargeInfoDlg::IDD, pParent)
	, m_billingItems(billingItems)
	, m_pBillingModuleDlg(pBillingModuleDlg)
	, m_bIsReadOnly(bIsReadOnly)
{}


void CBillingExtraChargeInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBillingExtraChargeInfoDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBillingExtraChargeInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBillingExtraChargeInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBillingExtraChargeInfoDlg message handlers

BOOL CBillingExtraChargeInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_ChargeList = BindNxDataList2Ctrl(IDC_CHARGE_LIST, false);

		// (j.jones 2010-04-08 11:40) - PLID 15224 - build the Emergency combo
		CString strEmergencyComboText;
		strEmergencyComboText.Format("%li; <Use Default Value>;%li; <Blank>;%li;No;%li;Yes;",
			(long)cietUseDefault, (long)cietBlank, (long)cietNo, (long)cietYes);
		IColumnSettingsPtr pEmergencyCol = m_ChargeList->GetColumn(clcEmergency);
		pEmergencyCol->PutComboSource(_bstr_t(strEmergencyComboText));

		//fill the list from m_billingItems
		for(int i=0; i<(int)m_billingItems.size(); i++) {
			BillingItemPtr pItem = m_billingItems[i];
			if(pItem) {
				IRowSettingsPtr pRow = m_ChargeList->GetNewRow();
				pRow->PutValue(clcLineID, pItem->LineID);
				pRow->PutValue(clcChargeID, pItem->ChargeID);
				pRow->PutValue(clcServiceID, pItem->ServiceID);
				pRow->PutValue(clcItemCode, pItem->CPTCode);
				pRow->PutValue(clcSubCode, pItem->CPTSubCode);
				pRow->PutValue(clcDescription, pItem->Description);
				pRow->PutValue(clcQuantity, pItem->Quantity);
				pRow->PutValue(clcUnitCost,	pItem->UnitCost);
				pRow->PutValue(clcTotal, pItem->LineTotal);
				// (j.jones 2010-04-08 11:40) - PLID 15224 - added Emergency
				pRow->PutValue(clcEmergency, pItem->IsEmergency);
				pRow->PutValue(clcNDCCode, pItem->NDCCode);
				// (j.jones 2009-08-12 18:15) - PLID 35206 - added more drug fields
				pRow->PutValue(clcDrugUnitPrice, pItem->DrugUnitPrice);
				pRow->PutValue(clcDrugUnitType, pItem->DrugUnitType);
				pRow->PutValue(clcDrugUnitQuantity, pItem->DrugUnitQuantity);
				pRow->PutValue(clcPrescriptionNumber, pItem->PrescriptionNumber);
				m_ChargeList->AddRowSorted(pRow, NULL);

				// (j.jones 2011-08-24 08:41) - PLID 44868 - color the foreground of
				// original and void charges light gray, so they are visibly marked
				// as non-editable
				if(VarBool(pItem->IsOriginalCharge, FALSE) || VarBool(pItem->IsVoidingCharge, FALSE)) {
					pRow->PutForeColor(CORRECTED_CHARGE_FOREGROUND_COLOR);
				}
			}
		}

		//if read only, disable editing the list
		if(m_bIsReadOnly) {
			// (j.jones 2011-08-16 11:46) - PLID 44773 - datalists need to use the ReadOnly flag, not Enabled
			m_ChargeList->ReadOnly = m_bIsReadOnly;
		}
	
	}NxCatchAll("Error in CBillingExtraChargeInfoDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBillingExtraChargeInfoDlg::OnOK() 
{
	try {

		if(m_pBillingModuleDlg == NULL) {
			ThrowNxException("Could not access the billing tab!");
		}

		//if read-only, don't check for bad data or
		//update the array, just leave
		if(m_bIsReadOnly) {
			CNxDialog::OnOK();
			return;
		}

		//warn if multiple NDC codes are entered,
		//warn if any NDC code has a non-number (excluding dashes)
		//warn if any NDC code does not have 11 digits (excluding dashes)
		//warn if any NDC-coded charge also is an anesthesia charge (regardless if the claim prints anesthesia times)

		long nCountNDCCodes = 0;
		BOOL bNonNumberNDCCodes = FALSE;
		BOOL bWrongSizedNDCCodes = FALSE;
		BOOL bAnesthesiaChargesWithNDCCodes = FALSE;

		{
			IRowSettingsPtr pRow = m_ChargeList->GetFirstRow();
			while(pRow) {

				CString strNDCCode = VarString(pRow->GetValue(clcNDCCode), "");
				strNDCCode.TrimLeft();
				strNDCCode.TrimRight();

				if(!strNDCCode.IsEmpty()) {

					//keep track that we have an NDC Code
					nCountNDCCodes++;

					CString strTemp = strNDCCode;
					strTemp.Replace("-","");
					
					//see if the length is not 11 digits
					if(!bWrongSizedNDCCodes && strTemp.GetLength() != 11) {
						bWrongSizedNDCCodes = TRUE;
					}

					if(!bNonNumberNDCCodes) {
						//confirm each character is number
						CString strNumbersOnly = strTemp.SpanIncluding("0123456789");
						if(strTemp.GetLength() != strNumbersOnly.GetLength()) {
							bNonNumberNDCCodes = TRUE;
						}
					}

					//see if this charge is an anesthesia code
					if(!bAnesthesiaChargesWithNDCCodes) {
						long nServiceID = VarLong(pRow->GetValue(clcServiceID), -1);
						_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM ServiceT WHERE ID = {INT} AND Anesthesia = 1", nServiceID);
						if(!rs->eof) {
							bAnesthesiaChargesWithNDCCodes = TRUE;
						}
						rs->Close();
					}
				}

				pRow = pRow->GetNextRow();
			}
		}

		//warn if anything is unusual

		if(nCountNDCCodes > 1 || bNonNumberNDCCodes || bWrongSizedNDCCodes) {

			CString strWarn = "The following potential problems were found with these charges:\n\n";

			if(nCountNDCCodes > 1) {
				CString str;
				str.Format("You have %li NDC Codes entered on this bill. Your insurance company might only allow one NDC Code per claim.\n\n", nCountNDCCodes);
				strWarn += str;
			}
			if(bNonNumberNDCCodes) {
				strWarn += "You have at least one NDC Code with a non-numeric value in it. A typical NDC Code only contains numbers and dashes.\n\n";
			}
			if(bWrongSizedNDCCodes) {
				strWarn += "You have at least one NDC Code that is not 11 digits. A typical NDC code is 11 digits, excluding dashes.\n\n";
			}
			if(bAnesthesiaChargesWithNDCCodes) {
				strWarn += "You have at least one NDC Code assigned to an Anesthesia service. Depending on your HCFA group settings, Anesthesia times may overwrite NDC codes for on paper HCFA or Image claims.\n\n";
			}

			strWarn += "Are you sure you wish to continue with these changes?";

			if(IDNO == MessageBox(strWarn, "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
				return;
			}
		}

		//if we get here, update the charges with the changed information

		{
			IRowSettingsPtr pRow = m_ChargeList->GetFirstRow();
			while(pRow) {

				long nLineID = VarLong(pRow->GetValue(clcLineID));

				// (j.jones 2010-04-08 11:40) - PLID 15224 - added Emergency
				ChargeIsEmergencyType eIsEmergency = cietUseDefault;
				_variant_t var = pRow->GetValue(clcEmergency);
				if(var.vt == VT_I4) {
					eIsEmergency = (ChargeIsEmergencyType)VarLong(var);
				}

				CString strNDCCode = "";
				// (j.jones 2009-06-04 10:30) - PLID 34435 - handle the possibility that this may be VT_EMPTY
				var = pRow->GetValue(clcNDCCode);
				if(var.vt == VT_BSTR) {
					strNDCCode = VarString(var, "");
				}
				strNDCCode.TrimLeft();
				strNDCCode.TrimRight();

				// (j.jones 2009-08-12 18:15) - PLID 35206 - added more drug fields
				COleCurrency cyDrugUnitPrice = VarCurrency(pRow->GetValue(clcDrugUnitPrice));
				CString strDrugUnitType = VarString(pRow->GetValue(clcDrugUnitType));
				double dblDrugUnitQuantity = VarDouble(pRow->GetValue(clcDrugUnitQuantity));
				CString strPrescriptionNumber = VarString(pRow->GetValue(clcPrescriptionNumber));

				//find the charge and update it

				BOOL bFound = FALSE;
				for(int i=0; i<(int)m_billingItems.size() && !bFound; i++) {
					BillingItemPtr pItem = m_billingItems[i];
					if(pItem != NULL && VarLong(pItem->LineID) == nLineID) {

						bFound = TRUE;

						// (j.jones 2010-04-08 11:40) - PLID 15224 - added Emergency
						pItem->IsEmergency = (long)eIsEmergency;
						pItem->NDCCode = (LPCTSTR)strNDCCode;
						// (j.jones 2009-08-12 18:15) - PLID 35206 - added more drug fields
						pItem->DrugUnitPrice = _variant_t(cyDrugUnitPrice);
						pItem->DrugUnitType = (LPCTSTR)strDrugUnitType;
						pItem->DrugUnitQuantity = dblDrugUnitQuantity;
						pItem->PrescriptionNumber = (LPCTSTR)strPrescriptionNumber;

						//if not new, mark as changed
						long nChargeID = VarLong(pItem->ChargeID, -2);

						// (j.jones 2011-08-24 08:41) - PLID 44868 - skip original/void charges
						if(nChargeID != -2 &&
							(VarBool(pItem->IsOriginalCharge, FALSE) || VarBool(pItem->IsVoidingCharge, FALSE))) {
							continue;
						}

						if(nChargeID != -2) {
							m_pBillingModuleDlg->AddToModifiedList(nChargeID);
						}
					}
				}

				//we should have always found the line ID
				ASSERT(bFound);

				pRow = pRow->GetNextRow();
			}
		}
	
		CNxDialog::OnOK();

	}NxCatchAll("Error in CBillingExtraChargeInfoDlg::OnOK");
}
BEGIN_EVENTSINK_MAP(CBillingExtraChargeInfoDlg, CNxDialog)
	ON_EVENT(CBillingExtraChargeInfoDlg, IDC_CHARGE_LIST, 9, OnEditingFinishingChargeList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CBillingExtraChargeInfoDlg, IDC_CHARGE_LIST, 8, OnEditingStartingChargeList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

// (j.jones 2009-08-13 12:39) - PLID 35206 - added OnEditingFinishingChargeList
void CBillingExtraChargeInfoDlg::OnEditingFinishingChargeList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		if(nCol == clcDrugUnitQuantity && pvarNewValue->vt == VT_R8) {

			if(VarDouble(pvarNewValue) < 0.0) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You may not have a unit quantity less than zero.");
				return;
			}
		}
		else if(nCol == clcDrugUnitPrice && pvarNewValue->vt == VT_CY) {

			if(VarCurrency(pvarNewValue) < COleCurrency(0,0)) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You may not have a drug unit price less than zero.");
				return;
			}
		}

	}NxCatchAll("Error in CBillingExtraChargeInfoDlg::OnEditingFinishingChargeList");
}

// (j.jones 2011-08-24 08:41) - PLID 44868 - added OnEditingStarting
void CBillingExtraChargeInfoDlg::OnEditingStartingChargeList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nChargeID = VarLong(pRow->GetValue(clcChargeID), -2);
	
		// (j.jones 2011-08-24 08:41) - PLID 44868 - nobody, not even administrators, can edit
		// Original or Void charges once a correction has been made
		if(nChargeID != -2) {
			for(int i=0;i<(int)m_billingItems.size();i++) {
				BillingItemPtr bi = m_billingItems[i];
				if(VarLong(bi->ChargeID) == nChargeID) {
					if(VarBool(bi->IsOriginalCharge, FALSE)) {
						MessageBox("This charge has been voided, and is no longer editable. If a corrected charge exists, you can edit that charge in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
						*pbContinue = FALSE;
						return;
					}
					else if(VarBool(bi->IsVoidingCharge, FALSE)) {
						MessageBox("This charge voids another charge, and is not editable. If a corrected charge exists, you can edit that charge in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
						*pbContinue = FALSE;
						return;
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}