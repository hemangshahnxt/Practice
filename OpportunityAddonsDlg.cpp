// OpportunityAddonsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "OpportunityAddonsDlg.h"
#include "PracticeRc.h"
#include "InternationalUtils.h"
#include "OpportunityAddDiscountDlg.h"
#include "MergeEngine.h"
#include "letterwriting.h"

// (d.lange 2010-07-16 17:20) - PLID 39691 - created
// COpportunityAddonsDlg dialog

using namespace ADODB;

#define NXM_LOAD_OPPORTUNITY_ADDON		WM_USER + 10003
#define PM_MAX_DISCOUNT		0.06
#define EMR_MAX_DISCOUNT	0.08
#define SUPPORT_MONTHLY		195

#define ADDMERGEDATA(head, data)	{	opmd.m_strHeaders += head;	opmd.m_strData += "\"" + data + "\",";	}

//Merge data
struct COPMergeData {
	CString m_strHeaders;
	CString m_strData;
};

IMPLEMENT_DYNAMIC(COpportunityAddonsDlg, CNxDialog)

COpportunityAddonsDlg::COpportunityAddonsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COpportunityAddonsDlg::IDD, pParent)
{
	m_nID = -1;
	m_nOpportunityID = -1;
	m_nPatientID = -1;
	m_strPatientName = "";
	m_nTypeID = -1;
	m_nExistingAddOnID = -1;
	m_nPriceStructure = -1;
	m_nMailSentID = -1;
	m_nCurrentDiscountUserID = -1;
	m_strUpdateValues = "";
}

COpportunityAddonsDlg::~COpportunityAddonsDlg()
{
}

void COpportunityAddonsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnSaveEdit);
	DDX_Control(pDX, IDC_BTN_MERGE_ADDON, m_btnMerge);
	DDX_Control(pDX, IDC_BTN_DELETE_ADDON, m_btnDelete);
	DDX_Control(pDX, IDC_SUBTOTAL_DISCOUNT, m_nxeditSubtotalDiscount);
	DDX_Control(pDX, IDC_SUBTOTAL_SEL_ADDON, m_nxeditSubtotalSel);
	DDX_Control(pDX, IDC_BTN_CHANGE_DISCOUNT, m_btnChangeDiscount);
	DDX_Control(pDX, IDC_GRAND_TOTAL_ADDON, m_nxeditGrandTotal);
	DDX_Control(pDX, IDC_SUPPORT_PERCENT_ADDON, m_nxeditSupportPercent);
	DDX_Control(pDX, IDC_SUPPORT_NUM_ADDON, m_nxeditSupportNum);
	DDX_Control(pDX, IDC_SUPPORT_SEL_ADDON, m_nxeditSupportTotal);
	DDX_Control(pDX, IDC_DISCOUNT_USERNAME_ADDON, m_nxeditDiscountUsername);
	DDX_Control(pDX, IDC_DISCOUNT_TOTAL_PERCENT_ADDON, m_nxeditDiscountPercent);
	DDX_Control(pDX, IDC_DISCOUNT_TOTAL_ADDON, m_nxeditDiscountTotal);
	DDX_Control(pDX, IDC_DATETIME_ADDON_DATE, m_pickerAddOnDate);
	DDX_Control(pDX, IDC_DATETIME_EXPIRES_ON_ADDON, m_pickerExpireDate);
	DDX_Control(pDX, IDC_LABEL_SUPPORT_ADDON, m_labelSupportMonthly);
	DDX_Control(pDX, IDC_TRAINING_NUM_PM_ADDON, m_nxeditPMTraining);
	DDX_Control(pDX, IDC_TRAINING_NUM_EMR_ADDON, m_nxeditEMRTraining);
	DDX_Control(pDX, IDC_TRAINING_SEL_ADDON, m_nxeditTrainingTotal);
	DDX_Control(pDX, IDC_TRAVEL_NUM_ADDON, m_nxeditTravelNum);
	DDX_Control(pDX, IDC_TRAVEL_SEL_ADDON, m_nxeditTravelTotal);
	DDX_Control(pDX, IDC_LABEL_TRAINING_ADDON, m_labelTraining);
}


BEGIN_MESSAGE_MAP(COpportunityAddonsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_DELETE_ADDON, &COpportunityAddonsDlg::OnBnClickedBtnDeleteAddon)
	ON_MESSAGE(NXM_LOAD_OPPORTUNITY_ADDON, &COpportunityAddonsDlg::OnLoadOpportunityAddOn)
	ON_BN_CLICKED(IDC_BTN_SAVE_EDIT, &COpportunityAddonsDlg::OnBnClickedBtnSaveEdit)
	ON_BN_CLICKED(IDOK, &COpportunityAddonsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &COpportunityAddonsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_CHANGE_DISCOUNT, &COpportunityAddonsDlg::OnBnClickedBtnChangeDiscount)
	ON_BN_CLICKED(IDC_BTN_MERGE_ADDON, &COpportunityAddonsDlg::OnBnClickedBtnMergeAddon)
	ON_EN_CHANGE(IDC_SUPPORT_NUM_ADDON, &COpportunityAddonsDlg::OnEnChangeSupportNumAddon)
END_MESSAGE_MAP()

// COpportunityAddonsDlg message handlers
BOOL COpportunityAddonsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		DWORD dwColor = GetRemotePropertyInt("InternalPropBGColor", 10542240, 0, "<None>", false);
		((CNxColor*)GetDlgItem(IDC_NXCOLOR_ADDON1))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_NXCOLOR_ADDON2))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_NXCOLOR_ADDON3))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_NXCOLOR_ADDON4))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_NXCOLOR_ADDON5))->SetColor(dwColor);

		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSaveEdit.AutoSet(NXB_OK);
		m_btnMerge.AutoSet(NXB_MERGE);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnChangeDiscount.AutoSet(NXB_MODIFY);

		m_btnDelete.EnableWindow(FALSE);

		m_dlAvailableAddOns = BindNxDataList2Ctrl(IDC_AVAILABLE_ADDONS_LIST, false);
		m_dlSelectedAddOns = BindNxDataList2Ctrl(IDC_SELECTED_ADDONS_LIST, false);

		//Fill the pricing array with the current addon prices
		FillPriceArray();

		//YOU MUST CALL THIS FUNCTION AFTER CALLING FillPriceArray
		CreateAvailableAddOnsCombo();

		OnLoadOpportunityAddOn(0, 0);
		
		UpdateSupportCosts();
		UpdateAllTotals();

	} NxCatchAll(__FUNCTION__);
	return TRUE;
}

//Use these to launch the dialog.  DO NOT USE DoModal().
int COpportunityAddonsDlg::OpenNewAddOn(long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID)
{
	return OpenAddOn(-1, nOpportunityID, nPatientID, strPatientName, nTypeID);
}

//Use these to launch the dialog.  DO NOT USE DoModal().
int COpportunityAddonsDlg::OpenExistingAddOn(long nOpportunityID, long nPatientID, long nExistingAddOnID, CString strPatientName, long nTypeID)
{
	m_nExistingAddOnID = nExistingAddOnID;

	return OpenAddOn(nExistingAddOnID, nOpportunityID, nPatientID, strPatientName, nTypeID);
}

int COpportunityAddonsDlg::OpenAddOn(long nAddOnID, long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID)
{
	m_nID = nAddOnID;
	m_nOpportunityID = nOpportunityID;
	m_nPatientID = nPatientID;
	m_strPatientName = strPatientName;
	m_nTypeID = nTypeID;

	return DoModal();
}

void COpportunityAddonsDlg::FillPriceArray()
{
	//Clear the pricing array and set the size
	m_aryPrices.RemoveAll();
	m_aryPrices.SetSize(ecapTotalPrices);

	//At this point we can either have a new AddOn or viewing an existing saved AddOn whose pricing structure could be based on a previous version
	CString strQuery = "SELECT * FROM OpportunityPriceStructureT WHERE ID = ";
	if(m_nID == -1) {
		//New AddOn, use the most current pricing structure
		strQuery += "(SELECT MAX(ID) FROM OpportunityPriceStructureT WHERE Active = 1)";

	}else {
		//Existing AddOn
		strQuery += FormatString("(SELECT PriceStructureID FROM OpportunityProposalsT WHERE ID = %li)", m_nID);
	}

	ADODB::_RecordsetPtr rs = CreateParamRecordset(strQuery);
	if(rs->eof){

	}else {
		//Populate the pricing array
		FieldsPtr pFlds = rs->Fields;

		//Clear array of AddOns
		m_aryAddOns.RemoveAll();
		//m_aryAddOns.SetSize(ecapTotalPrices);

		AddOn *newAddOn;

		newAddOn = new AddOn();
		newAddOn->nAddOnID = (long)ecapBilling;
		newAddOn->strAddOnName = "Cosmetic Billing";
		newAddOn->strProposalTColumn = "Billing";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Billing", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapHCFA;
		newAddOn->strAddOnName = "HCFA Billing";
		newAddOn->strProposalTColumn = "HCFA";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "HCFA", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapDoctor;
		newAddOn->strAddOnName = "Additional EMR Doctors";
		newAddOn->strProposalTColumn = "Doctors";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Doctors", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapEBilling;
		newAddOn->strAddOnName = "Electronic Billing";
		newAddOn->strProposalTColumn = "EBilling";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "EBilling", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapLetters;
		newAddOn->strAddOnName = "Letter Writing";
		newAddOn->strProposalTColumn = "Letters";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Letters", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapQuotes;
		newAddOn->strAddOnName = "Quotes";
		newAddOn->strProposalTColumn = "Quotes";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Quotes", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapTracking;
		newAddOn->strAddOnName = "NexTrak";
		newAddOn->strProposalTColumn = "Tracking";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Tracking", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapNexForms;
		newAddOn->strAddOnName = "NexForms";
		newAddOn->strProposalTColumn = "NexForms";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "NexForms", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapInventory;
		newAddOn->strAddOnName = "Inventory";
		newAddOn->strProposalTColumn = "Inventory";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Inventory", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapSpa;
		newAddOn->strAddOnName = "NexSpa";
		newAddOn->strProposalTColumn = "NexSpa";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "NexSpa", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapASC;
		newAddOn->strAddOnName = "NexASC";
		newAddOn->strProposalTColumn = "NexASC";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "NexASC", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapWorkstation;
		newAddOn->strAddOnName = "Additional Workstations";
		newAddOn->strProposalTColumn = "Workstations";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Workstations", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapPDA;
		newAddOn->strAddOnName = "NexPDA";
		newAddOn->strProposalTColumn = "PDA";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "PDA", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapMirror;
		newAddOn->strAddOnName = "Mirror Link";
		newAddOn->strProposalTColumn = "Mirror";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Mirror", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapUnited;
		newAddOn->strAddOnName = "United Link";
		newAddOn->strProposalTColumn = "United";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "United", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapTraining;
		newAddOn->strAddOnName = "Training";
		newAddOn->strProposalTColumn = "Training";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Training", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapConversion;
		newAddOn->strAddOnName = "Data Conversion";
		newAddOn->strProposalTColumn = "Conversion";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Conversion", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapHL7;
		newAddOn->strAddOnName = "HL7 Link";
		newAddOn->strProposalTColumn = "HL7";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "HL7", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapInform;
		newAddOn->strAddOnName = "Inform Link";
		newAddOn->strProposalTColumn = "Inform";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Inform", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapQuickbooks;
		newAddOn->strAddOnName = "Quickbooks Link";
		newAddOn->strProposalTColumn = "Quickbooks";
		newAddOn->cyPrice = AdoFldCurrency(pFlds, "Quickbooks", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapERemittance;
		newAddOn->strAddOnName = "E-Remittance";
		newAddOn->strProposalTColumn = "ERemittance";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "ERemittance", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapEEligibility;
		newAddOn->strAddOnName = "E-Eligibility";
		newAddOn->strProposalTColumn = "EEligibility";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "EEligibility", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapCandA;
		newAddOn->strAddOnName = "Consignment and Allocation";
		newAddOn->strProposalTColumn = "CAndA";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "CAndA", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapNexWebLeads;
		newAddOn->strAddOnName = "NexWeb (Lead Generation)";
		newAddOn->strProposalTColumn = "NexWebLeads";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "NexWebLeads", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapNexWebPortal;
		newAddOn->strAddOnName = "NexWeb (Patient Portal)";
		newAddOn->strProposalTColumn = "NexWebPortal";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "NexWebPortal", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapNexWebAddtlDomains;
		newAddOn->strAddOnName = "NexWeb Addtional Domains";
		newAddOn->strProposalTColumn = "NexWebAddtlDomain";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "NexWebAddtlDomain", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapNexPhoto;
		newAddOn->strAddOnName = "NexPhoto";
		newAddOn->strProposalTColumn = "NexPhoto";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "NexPhoto", COleCurrency(0, 0));
		newAddOn->bMultiple = FALSE;
		m_aryAddOns.Add(newAddOn);

		newAddOn = new AddOn();
		newAddOn->nAddOnID = ecapNexSync;
		newAddOn->strAddOnName = "NexSync";
		newAddOn->strProposalTColumn = "NexSync";
		newAddOn->cyPrice =  AdoFldCurrency(pFlds, "NexSync", COleCurrency(0, 0));
		newAddOn->bMultiple = TRUE;
		m_aryAddOns.Add(newAddOn);

		m_nPriceStructure = AdoFldLong(pFlds, "ID");
	}
}

void COpportunityAddonsDlg::CreateAvailableAddOnsCombo()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	for (int i = 0; i < m_aryAddOns.GetSize(); i++) {
		pRow = m_dlAvailableAddOns->GetNewRow();
		long nTest = m_aryAddOns.GetAt(i)->nAddOnID;
		pRow->PutValue(aacAddOnID, m_aryAddOns.GetAt(i)->nAddOnID);
		pRow->PutValue(aacAddOnName, _bstr_t(m_aryAddOns.GetAt(i)->strAddOnName));
		pRow->PutValue(aacAddOnPrice, _bstr_t(FormatCurrencyForInterface(m_aryAddOns.GetAt(i)->cyPrice)));
		pRow->PutValue(aacAddOnType, m_aryAddOns.GetAt(i)->bMultiple);

		m_dlAvailableAddOns->AddRowSorted(pRow, NULL);
	}
}

BEGIN_EVENTSINK_MAP(COpportunityAddonsDlg, CNxDialog)
	ON_EVENT(COpportunityAddonsDlg, IDC_AVAILABLE_ADDONS_LIST, 16, COpportunityAddonsDlg::SelChosenAvailableAddonsList, VTS_DISPATCH)
	ON_EVENT(COpportunityAddonsDlg, IDC_SELECTED_ADDONS_LIST, 10, COpportunityAddonsDlg::EditingFinishedSelectedAddonsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(COpportunityAddonsDlg, IDC_SELECTED_ADDONS_LIST, 19, COpportunityAddonsDlg::LeftClickSelectedAddonsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COpportunityAddonsDlg, IDC_SELECTED_ADDONS_LIST, 29, COpportunityAddonsDlg::SelSetSelectedAddonsList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void COpportunityAddonsDlg::SelChosenAvailableAddonsList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr pAddedRow;

		NXDATALIST2Lib::IFormatSettingsPtr pFormatColumn(__uuidof(NXDATALIST2Lib::FormatSettings));
		pFormatColumn->Editable = _variant_t(VARIANT_FALSE, VT_BOOL);

		if(pRow) {
			long nAddOnID = VarLong(pRow->GetValue(aacAddOnID));
			BOOL bSelected = FALSE;
			NXDATALIST2Lib::IRowSettingsPtr pSelRow = m_dlSelectedAddOns->GetFirstRow();

			while(pSelRow) {
				long nCurAddOnID = VarLong(pSelRow->GetValue(aocAddOnID));
				if(nAddOnID == nCurAddOnID) {
					bSelected = TRUE;
					break;
				}
				pSelRow = pSelRow->GetNextRow();
			}

			if(!bSelected) {
				pAddedRow = m_dlSelectedAddOns->GetNewRow();

				pAddedRow->PutValue(aocAddOnID, (long)pRow->GetValue(aacAddOnID));
				pAddedRow->PutValue(aocAddOnName, _bstr_t(pRow->GetValue(aacAddOnName)));
				pAddedRow->PutValue(aocPrice, _bstr_t(pRow->GetValue(aacAddOnPrice)));
				pAddedRow->PutValue(aocQuantity, (long)1);
				pAddedRow->PutValue(aocSubTotal, _bstr_t(pRow->GetValue(aacAddOnPrice)));
				
				if(!pRow->GetValue(aacAddOnType)) {
					pAddedRow->CellFormatOverride[aocQuantity] = pFormatColumn;
				}

				m_dlSelectedAddOns->AddRowAtEnd(pAddedRow, NULL);

				UpdateSupportCosts();

				UpdateAllTotals();
			}else {
				AfxMessageBox("You have already selected this AddOn, please modify the quantity or selected another AddOn.");
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::EditingFinishedSelectedAddonsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow) {
			switch (nCol) {
				case aocQuantity:
					{
						COleCurrency cyTotal(0, 0);
						cyTotal.ParseCurrency(VarString(pRow->GetValue(aocPrice)));
						CString strSubtotal = GetLineItemSubtotal(pRow->GetValue(aocQuantity), cyTotal);

						pRow->PutValue(aocSubTotal, _bstr_t(strSubtotal));
					}
					break;
				default:
					break;
			}
		}
		UpdateSupportCosts();
		UpdateAllTotals();

	} NxCatchAll(__FUNCTION__);
}

CString COpportunityAddonsDlg::GetLineItemSubtotal(long nQuantity, COleCurrency cyPrice)
{
	return FormatCurrencyForInterface(cyPrice * nQuantity);
}

void COpportunityAddonsDlg::OnBnClickedBtnDeleteAddon()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSelectedAddOns->CurSel;

		if(pRow) {
			m_strUpdateValues += FormatString("%s = %li, ", GetTableColumnName(VarLong(pRow->GetValue(aocAddOnID))), (long)0);
			m_dlSelectedAddOns->RemoveRow(pRow);
		}
		UpdateSupportCosts();

		UpdateAllTotals();

	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::LeftClickSelectedAddonsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		/*NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			switch(nCol) {
				case aocQuantity:
					
			}
		}*/

	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::SelSetSelectedAddonsList(LPDISPATCH lpSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pSelRow(lpSel);

		//If a row is selected enable the delete button
		GetDlgItem(IDC_BTN_DELETE_ADDON)->EnableWindow(pSelRow != NULL);

		UpdateSupportCosts();
		UpdateAllTotals();
		
	} NxCatchAll(__FUNCTION__);
}

#define ADDAMOUNT(idc, cy)	{CString str;	GetDlgItemText(idc, str);	COleCurrency cyTmp(0, 0);	cyTmp.ParseCurrency(str);	if(cyTmp.GetStatus() == COleCurrency::valid) cy += cyTmp;}

//This function iterates through the selected AddOns, adds all the subtotals and updates the subtotal editbox
void COpportunityAddonsDlg::UpdateAllTotals()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_dlSelectedAddOns->FindAbsoluteFirstRow(VARIANT_TRUE);

		COleCurrency cySubTotal(0, 0);

		while(pFirstRow) {
			//Grab the subtotal of the current row
			CString strSubTotal = VarString(pFirstRow->GetValue(aocSubTotal));

			COleCurrency cyPrice(0, 0);
			cyPrice.ParseCurrency(strSubTotal);

			cySubTotal += cyPrice;

			pFirstRow = pFirstRow->GetNextRow();
		}

		//Support Costs
		COleCurrency cyMonthlySupport(0, 0);
		{
			cyMonthlySupport = CalculateCurrentMonthlySupport();
			m_labelSupportMonthly.SetText(FormatCurrencyForInterface(cyMonthlySupport));
			GetDlgItem(IDC_LABEL_SUPPORT_ADDON)->Invalidate();
		}

		//Total Subtotal without discounts
		SetDlgItemText(IDC_SUBTOTAL_SEL_ADDON, FormatCurrencyForInterface(cySubTotal));

		//Discount Total
		COleCurrency cyDiscountAmt(0, 0);
		{
			ADDAMOUNT(IDC_DISCOUNT_TOTAL_ADDON, cyDiscountAmt);
			double dblPercent = (double)cyDiscountAmt.m_cur.int64 / (double)cySubTotal.m_cur.int64;
			dblPercent *= 100.0;

			CString strFmt;
			strFmt.Format("%.2f%%", dblPercent);
			SetDlgItemText(IDC_DISCOUNT_TOTAL_PERCENT_ADDON, strFmt);
		}

		//Total Subtotal with the discount
		SetDlgItemText(IDC_SUBTOTAL_DISCOUNT, FormatCurrencyForInterface(cySubTotal - cyDiscountAmt));

		//Grand Total
		COleCurrency cyGrandTotal = cySubTotal;
		{

			//Subtract the discount
			cyGrandTotal -= cyDiscountAmt;

			SetDlgItemText(IDC_GRAND_TOTAL_ADDON, FormatCurrencyForInterface(cyGrandTotal));
		}

	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::InsertSelectedAddOn(AddOn* addOn, long nQuantity)
{
	NXDATALIST2Lib::IRowSettingsPtr pAddedRow;

	NXDATALIST2Lib::IFormatSettingsPtr pFormatColumn(__uuidof(NXDATALIST2Lib::FormatSettings));
	pFormatColumn->Editable = _variant_t(VARIANT_FALSE, VT_BOOL);

	
	long nAddOnID = addOn->nAddOnID;
	BOOL bSelected = FALSE;
	NXDATALIST2Lib::IRowSettingsPtr pSelRow = m_dlSelectedAddOns->GetFirstRow();

	//Determine if this AddOn is already in the selected list
	while(pSelRow) {
		long nCurAddOnID = VarLong(pSelRow->GetValue(aocAddOnID));
		if(nAddOnID == nCurAddOnID) {
			bSelected = TRUE;
			break;
		}
		pSelRow = pSelRow->GetNextRow();
	}

	if(!bSelected) {
		pAddedRow = m_dlSelectedAddOns->GetNewRow();

		pAddedRow->PutValue(aocAddOnID, nAddOnID);
		pAddedRow->PutValue(aocAddOnName, _bstr_t(addOn->strAddOnName));
		pAddedRow->PutValue(aocPrice, _bstr_t(FormatCurrencyForInterface(addOn->cyPrice)));
		pAddedRow->PutValue(aocQuantity, (long)nQuantity);
		pAddedRow->PutValue(aocSubTotal, _bstr_t(GetLineItemSubtotal(nQuantity, addOn->cyPrice)));
		
		if(!addOn->bMultiple) {
			pAddedRow->CellFormatOverride[aocQuantity] = pFormatColumn;
		}

		m_dlSelectedAddOns->AddRowAtEnd(pAddedRow, NULL);

		UpdateAllTotals();
	}
}

AddOn* COpportunityAddonsDlg::GetAddOnFromColumnName(CString strColName)
{
	AddOn* addOn = new AddOn();
	for(int i = 0; i < m_aryAddOns.GetSize(); i++) {
		if(m_aryAddOns[i]->strProposalTColumn.CompareNoCase(strColName) == 0) {
			return m_aryAddOns[i];
		}
	}
	return addOn;
}

LRESULT COpportunityAddonsDlg::OnLoadOpportunityAddOn(WPARAM wParam, LPARAM lParam)
{
	try {
		SetDlgItemInt(IDC_SUPPORT_PERCENT_ADDON, 18);

		CString strUsername;
		CString strDiscountUser;
		long nQuoteNum;
		long nSupportAmt;
		COleDateTime dtAddOnDate;
		COleDateTime dtExpireDate;
		COleCurrency cyDiscountAmount(0, 0);


		if(m_nID != -1) {
			//We are loading an existing AddOn
			_RecordsetPtr prsLoad = CreateRecordset("SELECT OpportunityProposalsT.*, UsersT.Username, "
				"DiscountUsersT.Username AS DiscountedByUser, DiscountUsersT.PersonID AS DiscountedByUserID "
				"FROM OpportunityProposalsT LEFT JOIN UsersT ON OpportunityProposalsT.CreatedByUserID = UsersT.PersonID "
				"LEFT JOIN UsersT DiscountUsersT ON OpportunityProposalsT.DiscountedBy = DiscountUsersT.PersonID "
				"WHERE ID = %li", m_nID);

			if(prsLoad->eof) {
				//This should be impossible
				AfxThrowNxException("Attempted to load invalid proposal ID %li.", m_nID);
			}

			FieldsPtr pFlds = prsLoad->Fields;
			dtAddOnDate = VarDateTime(pFlds->Item["ProposalDate"]->Value);
			dtExpireDate = VarDateTime(pFlds->Item["ExpiresOn"]->Value);
			nQuoteNum = AdoFldLong(pFlds, "QuoteNum");
			strUsername = AdoFldString(pFlds, "Username");

			if(AdoFldBool(pFlds, "Billing"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Billing"), (long)AdoFldBool(pFlds, "Billing"));

			if(AdoFldBool(pFlds, "HCFA"))
				InsertSelectedAddOn(GetAddOnFromColumnName("HCFA"), (long)AdoFldBool(pFlds, "HCFA"));

			if(AdoFldBool(pFlds, "EBilling"))
				InsertSelectedAddOn(GetAddOnFromColumnName("EBilling"), (long)AdoFldBool(pFlds, "EBilling"));

			if(AdoFldBool(pFlds, "Letters"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Letters"), (long)AdoFldBool(pFlds, "Letters"));

			if(AdoFldBool(pFlds, "Quotes"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Quotes"), (long)AdoFldBool(pFlds, "Quotes"));

			if(AdoFldBool(pFlds, "Tracking"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Tracking"), (long)AdoFldBool(pFlds, "Tracking"));

			if(AdoFldBool(pFlds, "NexForms"))
				InsertSelectedAddOn(GetAddOnFromColumnName("NexForms"), (long)AdoFldBool(pFlds, "NexForms"));

			if(AdoFldBool(pFlds, "Inventory"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Inventory"), (long)AdoFldBool(pFlds, "Inventory"));

			if(AdoFldBool(pFlds, "NexSpa"))
				InsertSelectedAddOn(GetAddOnFromColumnName("NexSpa"), (long)AdoFldBool(pFlds, "NexSpa"));

			if(AdoFldBool(pFlds, "NexASC"))
				InsertSelectedAddOn(GetAddOnFromColumnName("NexASC"), (long)AdoFldBool(pFlds, "NexASC"));

			if(AdoFldBool(pFlds, "Mirror"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Mirror"), (long)AdoFldBool(pFlds, "Mirror"));

			if(AdoFldBool(pFlds, "United"))
				InsertSelectedAddOn(GetAddOnFromColumnName("United"), (long)AdoFldBool(pFlds, "United"));

			if(AdoFldBool(pFlds, "HL7"))
				InsertSelectedAddOn(GetAddOnFromColumnName("HL7"), (long)AdoFldBool(pFlds, "HL7"));

			if(AdoFldBool(pFlds, "Inform"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Inform"), (long)AdoFldBool(pFlds, "Inform"));

			if(AdoFldBool(pFlds, "Quickbooks"))
				InsertSelectedAddOn(GetAddOnFromColumnName("Quickbooks"), (long)AdoFldBool(pFlds, "Quickbooks"));

			if(AdoFldBool(pFlds, "ERemittance"))
				InsertSelectedAddOn(GetAddOnFromColumnName("ERemittance"), (long)AdoFldBool(pFlds, "ERemittance"));

			if(AdoFldBool(pFlds, "EEligibility"))
				InsertSelectedAddOn(GetAddOnFromColumnName("EEligibility"), (long)AdoFldBool(pFlds, "EEligibility"));

			if(AdoFldBool(pFlds, "CandA"))
				InsertSelectedAddOn(GetAddOnFromColumnName("CandA"), (long)AdoFldBool(pFlds, "CandA"));

			if(AdoFldBool(pFlds, "NexWebLeads"))
				InsertSelectedAddOn(GetAddOnFromColumnName("NexWebLeads"), (long)AdoFldBool(pFlds, "NexWebLeads"));

			if(AdoFldBool(pFlds, "NexWebPortal"))
				InsertSelectedAddOn(GetAddOnFromColumnName("NexWebPortal"), (long)AdoFldBool(pFlds, "NexWebPortal"));

			if(AdoFldLong(pFlds, "NexWebAddtlDomains") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("NexWebAddtlDomains"), (long)AdoFldLong(pFlds, "NexWebAddtlDomains"));

			if(AdoFldBool(pFlds, "NexPhoto"))
				InsertSelectedAddOn(GetAddOnFromColumnName("NexPhoto"), (long)AdoFldBool(pFlds, "NexPhoto"));

			if(AdoFldLong(pFlds, "Workstations") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("Workstations"), (long)AdoFldLong(pFlds, "Workstations"));

			if(AdoFldLong(pFlds, "Doctors") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("Doctors"), (long)AdoFldLong(pFlds, "Doctors"));

			if(AdoFldLong(pFlds, "PDA") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("PDA"), (long)AdoFldLong(pFlds, "PDA"));

			if(AdoFldLong(pFlds, "NexSync") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("NexSync"), (long)AdoFldLong(pFlds, "NexSync"));

			if(AdoFldLong(pFlds, "EMR") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("NexSync"), (long)AdoFldLong(pFlds, "EMR"));

			if(AdoFldLong(pFlds, "Conversion") > 0)
				InsertSelectedAddOn(GetAddOnFromColumnName("NexSync"), (long)AdoFldLong(pFlds, "Conversion"));

			nSupportAmt = AdoFldLong(pFlds, "Support", 0);
			cyDiscountAmount = AdoFldCurrency(pFlds, "DiscountAmount");
			strDiscountUser = AdoFldString(pFlds, "DiscountedByUser", "");
			m_nCurrentDiscountUserID = AdoFldLong(pFlds, "DiscountedByUserID", -1);

		}else {
			if(m_nExistingAddOnID != -1) {
				//We are loading from an existing AddOn
				_RecordsetPtr prsLoad = CreateRecordset("SELECT OpportunityProposalsT.*, UsersT.Username, "
					"DiscountUsersT.Username AS DiscountedByUser, DiscountUsersT.PersonID AS DiscountedByUserID "
					"FROM OpportunityProposalsT LEFT JOIN UsersT ON OpportunityProposalsT.CreatedByUserID = UsersT.PersonID "
					"LEFT JOIN UsersT DiscountUsersT ON OpportunityProposalsT.DiscountedBy = DiscountUsersT.PersonID "
					"WHERE ID = %li", m_nExistingAddOnID);
				if(prsLoad->eof) {
					//This should be impossible
					AfxThrowNxException("Attempted to create new proposal from invalid proposal ID %li.", m_nID);
				}

				FieldsPtr pFlds = prsLoad->Fields;
				dtAddOnDate = VarDateTime(pFlds->Item["ProposalDate"]->Value);
				dtExpireDate = VarDateTime(pFlds->Item["ExpiresOn"]->Value);
				nQuoteNum = AdoFldLong(pFlds, "QuoteNum");
				strUsername = AdoFldString(pFlds, "Username");

				if(AdoFldBool(pFlds, "Billing"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Billing"), (long)AdoFldBool(pFlds, "Billing"));

				if(AdoFldBool(pFlds, "HCFA"))
					InsertSelectedAddOn(GetAddOnFromColumnName("HCFA"), (long)AdoFldBool(pFlds, "HCFA"));

				if(AdoFldBool(pFlds, "EBilling"))
					InsertSelectedAddOn(GetAddOnFromColumnName("EBilling"), (long)AdoFldBool(pFlds, "EBilling"));

				if(AdoFldBool(pFlds, "Letters"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Letters"), (long)AdoFldBool(pFlds, "Letters"));

				if(AdoFldBool(pFlds, "Quotes"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Quotes"), (long)AdoFldBool(pFlds, "Quotes"));

				if(AdoFldBool(pFlds, "Tracking"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Tracking"), (long)AdoFldBool(pFlds, "Tracking"));

				if(AdoFldBool(pFlds, "NexForms"))
					InsertSelectedAddOn(GetAddOnFromColumnName("NexForms"), (long)AdoFldBool(pFlds, "NexForms"));

				if(AdoFldBool(pFlds, "Inventory"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Inventory"), (long)AdoFldBool(pFlds, "Inventory"));

				if(AdoFldBool(pFlds, "NexSpa"))
					InsertSelectedAddOn(GetAddOnFromColumnName("NexSpa"), (long)AdoFldBool(pFlds, "NexSpa"));

				if(AdoFldBool(pFlds, "NexASC"))
					InsertSelectedAddOn(GetAddOnFromColumnName("NexASC"), (long)AdoFldBool(pFlds, "NexASC"));

				if(AdoFldBool(pFlds, "Mirror"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Mirror"), (long)AdoFldBool(pFlds, "Mirror"));

				if(AdoFldBool(pFlds, "United"))
					InsertSelectedAddOn(GetAddOnFromColumnName("United"), (long)AdoFldBool(pFlds, "United"));

				if(AdoFldBool(pFlds, "HL7"))
					InsertSelectedAddOn(GetAddOnFromColumnName("HL7"), (long)AdoFldBool(pFlds, "HL7"));

				if(AdoFldBool(pFlds, "Inform"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Inform"), (long)AdoFldBool(pFlds, "Inform"));

				if(AdoFldBool(pFlds, "Quickbooks"))
					InsertSelectedAddOn(GetAddOnFromColumnName("Quickbooks"), (long)AdoFldBool(pFlds, "Quickbooks"));

				if(AdoFldBool(pFlds, "ERemittance"))
					InsertSelectedAddOn(GetAddOnFromColumnName("ERemittance"), (long)AdoFldBool(pFlds, "ERemittance"));

				if(AdoFldBool(pFlds, "EEligibility"))
					InsertSelectedAddOn(GetAddOnFromColumnName("EEligibility"), (long)AdoFldBool(pFlds, "EEligibility"));

				if(AdoFldBool(pFlds, "CandA"))
					InsertSelectedAddOn(GetAddOnFromColumnName("CandA"), (long)AdoFldBool(pFlds, "CandA"));

				if(AdoFldBool(pFlds, "NexWebLeads"))
					InsertSelectedAddOn(GetAddOnFromColumnName("NexWebLeads"), (long)AdoFldBool(pFlds, "NexWebLeads"));

				if(AdoFldBool(pFlds, "NexWebPortal"))
					InsertSelectedAddOn(GetAddOnFromColumnName("NexWebPortal"), (long)AdoFldBool(pFlds, "NexWebPortal"));

				if(AdoFldLong(pFlds, "NexWebAddtlDomains") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("NexWebAddtlDomains"), (long)AdoFldLong(pFlds, "NexWebAddtlDomains"));

				if(AdoFldBool(pFlds, "NexPhoto"))
					InsertSelectedAddOn(GetAddOnFromColumnName("NexPhoto"), (long)AdoFldBool(pFlds, "NexPhoto"));

				if(AdoFldLong(pFlds, "Workstations") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("Workstations"), (long)AdoFldLong(pFlds, "Workstations"));

				if(AdoFldLong(pFlds, "Doctors") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("Doctors"), (long)AdoFldLong(pFlds, "Doctors"));

				if(AdoFldLong(pFlds, "PDA") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("PDA"), (long)AdoFldLong(pFlds, "PDA"));

				if(AdoFldLong(pFlds, "NexSync") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("NexSync"), (long)AdoFldLong(pFlds, "NexSync"));

				if(AdoFldLong(pFlds, "EMR") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("NexSync"), (long)AdoFldLong(pFlds, "EMR"));

				if(AdoFldLong(pFlds, "Conversion") > 0)
					InsertSelectedAddOn(GetAddOnFromColumnName("NexSync"), (long)AdoFldLong(pFlds, "Conversion"));

				nSupportAmt = AdoFldLong(pFlds, "Support", 0);
				cyDiscountAmount = AdoFldCurrency(pFlds, "DiscountAmount");
				strDiscountUser = AdoFldString(pFlds, "DiscountedByUser", "");
				m_nCurrentDiscountUserID = AdoFldLong(pFlds, "DiscountedByUserID", -1);

			}else {
				//A brand new AddOn
				nSupportAmt = 0;
				strUsername = GetCurrentUserName();
				nQuoteNum = NewNumber("OpportunityProposalsT", "QuoteNum");
			}
		}

		SetDlgItemInt(IDC_SUPPORT_NUM_ADDON, nSupportAmt);
		SetDlgItemText(IDC_CREATED_BY_EDIT, strUsername);
		SetDlgItemInt(IDC_QUOTE_ADDON_EDIT, nQuoteNum);
		SetDlgItemText(IDC_DISCOUNT_USERNAME_ADDON, strDiscountUser);
		SetDlgItemText(IDC_DISCOUNT_TOTAL_ADDON, FormatCurrencyForInterface(cyDiscountAmount, TRUE));

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void COpportunityAddonsDlg::OnBnClickedBtnSaveEdit()
{
	try {

	} NxCatchAll(__FUNCTION__);
}

CString COpportunityAddonsDlg::GetTableColumnName(long nAddOnEnum)
{
	CString strColumnName = "";

	for(int i = 0; i < m_aryAddOns.GetSize(); i++) {
		if(m_aryAddOns[i]->nAddOnID == nAddOnEnum)
			strColumnName = m_aryAddOns[i]->strProposalTColumn;
	}
	
	return strColumnName;
}

/*CArray<AddOn*, AddOn*> COpportunityAddonsDlg::GetCurrentSelectedAddOns()
{
	CArray<AddOn*, AddOn*> aryCurrentAddOns;
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSelectedAddOns->GetFirstRow();
	return aryCurrentAddOns;
}*/

BOOL COpportunityAddonsDlg::ApplyChanges()
{
	//Any validation here?

	CString strMailSentID = "NULL";
	if(m_nMailSentID != -1)
		strMailSentID.Format("%li", m_nMailSentID);

	CString strDiscount;
	GetDlgItemText(IDC_DISCOUNT_TOTAL_ADDON, strDiscount);
	CString strDiscountUser = m_nCurrentDiscountUserID == -1 ? "NULL" : FormatString("%li", m_nCurrentDiscountUserID);

	CString strTotal;
	GetDlgItemText(IDC_GRAND_TOTAL_ADDON, strTotal);

	CString strColNames;
	CString strColValues;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSelectedAddOns->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pRow) {
		strColNames += GetTableColumnName(VarLong(pRow->GetValue(aocAddOnID))) + ", ";
		strColValues += FormatString("%li, ", VarLong(pRow->GetValue(aocQuantity)));

		m_strUpdateValues += FormatString("%s = %li, ", GetTableColumnName(VarLong(pRow->GetValue(aocAddOnID))), VarLong(pRow->GetValue(aocQuantity)));

		pRow = pRow->GetNextRow();
	}

	long nQuoteNum = -1;
	CString strSQL;
	if(m_nID == -1) {
		//New AddOn
		long nNextAvail = NewNumber("OpportunityProposalsT", "QuoteNum");
		nQuoteNum = GetDlgItemInt(IDC_QUOTE_ADDON_EDIT);
		if(nQuoteNum != nNextAvail) {
			//Someone else used it and saved
			MessageBox("The current quote number has been used by another user.  This quote number will be changed to the next available.");
			nQuoteNum = nNextAvail;
			SetDlgItemInt(IDC_QUOTE_ADDON_EDIT, nQuoteNum);
		}

		strSQL.Format("DECLARE @newID int;\r\n"
			"INSERT INTO OpportunityProposalsT (OpportunityID, MailSentID, ProposalDate, ExpiresOn, QuoteNum, CreatedByUserID, "
			"SavedTotal, PriceStructureID, DiscountAmount, DiscountedBy, Support, ");
		strColNames.TrimRight(", ");
		strSQL += strColNames;
		//TODO: Discount Amount and DiscountedBy
		strSQL += FormatString(") VALUES (%li, %s, '%s', '%s', %li, %li, convert(money, '%s'), %li, convert(money, '%s'), %s, %li, ", 
			m_nOpportunityID, strMailSentID, 
			FormatDateTimeForSql(VarDateTime(m_pickerAddOnDate.GetValue()), dtoDate), 
			FormatDateTimeForSql(VarDateTime(m_pickerExpireDate.GetValue()), dtoDate), nQuoteNum, GetCurrentUserID(), strTotal, 
			m_nPriceStructure, strDiscount, strDiscountUser, GetDlgItemInt(IDC_SUPPORT_NUM_ADDON));

		strColValues.TrimRight(", ");
		strSQL += strColValues;
		strSQL += ");\r\n";
		strSQL += "SET @newID = (SELECT @@identity);\r\n";

	}else {
		//Existing AddOn
		strSQL.Format("UPDATE OpportunityProposalsT SET MailSentID = %s, ProposalDate = '%s', ExpiresOn = '%s', "
				"SavedTotal = convert(money, '%s'), DiscountAmount = convert(money, '%s'), DiscountedBy = %s, Support = %li, ",
				strMailSentID, FormatDateTimeForSql(VarDateTime(m_pickerAddOnDate.GetValue()), dtoDate),
				FormatDateTimeForSql(VarDateTime(m_pickerExpireDate.GetValue()), dtoDate), strTotal, strDiscount, strDiscountUser, 
				GetDlgItemInt(IDC_SUPPORT_NUM_ADDON));

		m_strUpdateValues.TrimRight(", ");
		strSQL += m_strUpdateValues;
		strSQL += FormatString(" WHERE ID = %li;\r\n", m_nID);
	}

	//Should we have active AddOns?
	if(m_nID == -1) {
		strSQL += FormatString("UPDATE OpportunitiesT SET ActiveProposalID = @newID, EstPrice = convert(money, '%s') WHERE ID = %li;\r\n", _Q(strTotal), m_nOpportunityID);
	}
	else {
		strSQL += FormatString("UPDATE OpportunitiesT SET ActiveProposalID = %li, EstPrice = convert(money, '%s') WHERE ID = %li;\r\n", m_nID, _Q(strTotal), m_nOpportunityID);
	}

	CString strFmt;
	strFmt.Format("SET NOCOUNT ON;\r\n"
		"%s"
		"SET NOCOUNT OFF;\r\n"
		"%s", strSQL, m_nID == -1 ? "SELECT convert(int, @newID) AS NewID;\r\n" : "");

	BOOL bRetVal = FALSE;
	BEGIN_TRANS("Error in ApplyChanges")

		_RecordsetPtr prsExec = CreateParamRecordset(strFmt);

		if(m_nID == -1) {
			//Pull out the identity value, if this is a new 
			m_nID = AdoFldLong(prsExec->Fields, "NewID");
		}

		//If we got here, success!
		bRetVal = TRUE;
	END_TRANS_CATCH_ALL("Error in ApplyChanges");

	return bRetVal;
}

void COpportunityAddonsDlg::OnBnClickedOk()
{
	try {
		if(ApplyChanges()) {
			//Saving was a success

			CDialog::OnOK();
		}else {
			//We failed at saving
			return;
		}
	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::OnBnClickedCancel()
{
	try {
		CDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::OnBnClickedBtnChangeDiscount()
{
	try {
		COpportunityAddDiscountDlg dlg(this);
		COleCurrency cySubTotal(0, 0);
		ADDAMOUNT(IDC_SUBTOTAL_SEL_ADDON, cySubTotal);
		dlg.m_cySubTotal = cySubTotal;
		if(dlg.DoModal() == IDOK) {
			//Update our text label
			SetDlgItemText(IDC_DISCOUNT_TOTAL_ADDON, FormatCurrencyForInterface(dlg.m_cyFinalDiscount, TRUE));
			UpdateAllTotals();

			//In case there was an override of the user, we need to get the values from this dialog
			SetDlgItemText(IDC_DISCOUNT_USERNAME_ADDON, dlg.m_strDiscountUserName);
			//Save the user id
			m_nCurrentDiscountUserID = dlg.m_nDiscountUserID;
			
		}
	} NxCatchAll(__FUNCTION__);
}

BOOL COpportunityAddonsDlg::IsValidDiscount()
{
	COleCurrency cyDiscountAmt(0, 0);
	ADDAMOUNT(IDC_DISCOUNT_TOTAL_ADDON, cyDiscountAmt);

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlSelectedAddOns->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pRow) {
		

		pRow = pRow->GetNextRow();
	}

	return FALSE;
}

//return merge information
CString CALLBACK COpportunityAddons__ExtraMergeFields(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam)
{
	try {
		COPMergeData *popmd = (COPMergeData*)pParam;
		if (bFieldNamesInsteadOfData) {
			return popmd->m_strHeaders;
		} else {
			return popmd->m_strData;
		}
	} NxCatchAllCallIgnore({
		return "";
	});
}

void COpportunityAddonsDlg::MergeFields()
{
	if(!ApplyChanges()) {
			return;
	}

	CString strPathToDocument = "";

	if(GetAsyncKeyState(VK_SHIFT)) {
		CString strFilter;
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		CString initialDir = GetTemplatePath();
		dlg.m_ofn.lpstrInitialDir = initialDir;
		if (dlg.DoModal() == IDOK) {
			strPathToDocument = dlg.GetPathName();
		}
		else {
			return;
		}
	}else {

	}

	CMergeEngine mi;

	CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatientID);
		CString strMergeT = CreateTempIDTable(strSql, "ID");

	if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
	mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

	if(!mi.LoadSenderInfo(TRUE)) {
			return;
	}

	long nCategoryID = -1;
	if(nCategoryID != -1) {
		mi.m_nCategoryID = nCategoryID;
	}

	COPMergeData opmd;
	
	//Name, City, State, Expiration Date
	{
		ADDMERGEDATA("Internal_Prop_Proposal_Expires,",		FormatDateTimeForInterface(VarDateTime(m_pickerExpireDate.GetValue()), 0, dtoDate))
	}

	//Trim trailing comma off each
	opmd.m_strHeaders.TrimRight(",");
	opmd.m_strData.TrimRight(",");

	//Pass in our header/data information and a function to handle it to the merge engine.
	mi.m_pfnCallbackExtraFields = COpportunityAddons__ExtraMergeFields;
	mi.m_pCallbackParam = &opmd;

	//Perform the actual merge
	mi.MergeToWord(strPathToDocument, std::vector<CString>(), strMergeT);
}


void COpportunityAddonsDlg::OnBnClickedBtnMergeAddon()
{
	try {
		MergeFields();
	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::OnEnChangeSupportNumAddon()
{
	try {
		UpdateSupportCosts();

		UpdateAllTotals();

	} NxCatchAll(__FUNCTION__);
}

void COpportunityAddonsDlg::UpdateSupportCosts()
{
	long nSupportAmt = GetDlgItemInt(IDC_SUPPORT_NUM_ADDON);
	
	COleCurrency cyMonthlySupport = CalculateCurrentMonthlySupport();

	cyMonthlySupport *= nSupportAmt;
	
	SetDlgItemText(IDC_SUPPORT_SEL_ADDON, FormatCurrencyForInterface(cyMonthlySupport));
}

COleCurrency COpportunityAddonsDlg::CalculateCurrentMonthlySupport()
{
	long nSupportPercent = GetDlgItemInt(IDC_SUPPORT_PERCENT_ADDON);
	COleCurrency cySubTotal = GetCurrentSubTotalList();

	COleCurrency cyMonthlySupport = (((double)nSupportPercent / 100) * cySubTotal) / long(12);
	
	__int64 nTotal = cyMonthlySupport.m_cur.int64;
	//the value is set 1/10,000th's
	__int64 nCents = nTotal % 10000;	//For example, 66 cents would be 6600
	__int64 nDollars = nTotal / 10000;

	if(nCents != 0)
		cyMonthlySupport.SetCurrency((long)(nDollars + 1), 0);

	return cyMonthlySupport;
}

COleCurrency COpportunityAddonsDlg::GetCurrentSubTotalList()
{
	CString str;
	GetDlgItemText(IDC_SUBTOTAL_SEL_ADDON, str);
	COleCurrency cySubTotal;
	cySubTotal.ParseCurrency(str);
	if(cySubTotal.GetStatus() == COleCurrency::invalid)
		//Not yet filled out, just set it to $0.00.
		cySubTotal = COleCurrency(0, 0);

	return cySubTotal;
}
