// ApplyManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ApplyManagerDlg.h"
#include "globalfinancialutils.h"
#include "paymentdlg.h"
#include "NxStandard.h"
#include "MainFrm.h"
#include "MsgBox.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "BillingRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2015-02-24 14:22) - PLID 64940 - changed to an enumeration
enum ApplyManagerColumns {
	amcApplyID = 0,
	amcSourcePayID,
	amcDate,
	amcPaymentTypeID,
	amcPaymentTypeName,		// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
	amcChargeDesc,
	amcPaymentDesc,
	amcApplyAmount,
	amcPrimaryAmount,
	amcSecondaryAmount,
	amcOtherInsAmount,	
	amcChargeBackID,		//TES 7/24/2014 - PLID 62557
};

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_SHOWAPPLY 42304
#define ID_UNAPPLY 43041
#define ID_UNAPPLY_ALL 43042

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

/////////////////////////////////////////////////////////////////////////////
// CApplyManagerDlg dialog

CApplyManagerDlg::CApplyManagerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApplyManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApplyManagerDlg)
		m_iBillID = -2;
		m_iChargeID = -2;
		m_iPayID = -2;
	//}}AFX_DATA_INIT
}

CApplyManagerDlg::~CApplyManagerDlg() {
	
	// (j.jones 2008-05-02 08:43) - PLID 27305 - properly handled deleting from the array
	ClearApplyArray();
}

void CApplyManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApplyManagerDlg)
	DDX_Control(pDX, IDC_COMBO_CHARGES, m_ComboCharges);
	DDX_Control(pDX, IDC_COMBO_BILLS, m_ComboBills);
	DDX_Control(pDX, IDC_LABEL_BILLORPAYMENT, m_nxstaticLabelBillorpayment);
	DDX_Control(pDX, IDC_LABEL_CHARGE, m_nxstaticLabelCharge);
	DDX_Control(pDX, IDC_LABEL10, m_nxstaticLabel10);
	DDX_Control(pDX, IDC_LABEL11, m_nxstaticLabel11);
	DDX_Control(pDX, IDC_LABEL12, m_nxstaticLabel12);
	DDX_Control(pDX, IDC_LABEL13, m_nxstaticLabel13);
	DDX_Control(pDX, IDC_LABEL5, m_nxstaticLabel5);
	DDX_Control(pDX, IDC_LABEL_TTL_PATRESP, m_nxstaticLabelTtlPatresp);
	DDX_Control(pDX, IDC_LABEL_TTL_PRIRESP, m_nxstaticLabelTtlPriresp);
	DDX_Control(pDX, IDC_LABEL_TTL_SECRESP, m_nxstaticLabelTtlSecresp);
	DDX_Control(pDX, IDC_LABEL_TTL_TERRESP, m_nxstaticLabelTtlTerresp);
	DDX_Control(pDX, IDC_LABEL8, m_nxstaticLabel8);
	DDX_Control(pDX, IDC_LABEL_PAYMENTS, m_nxstaticLabelPayments);
	DDX_Control(pDX, IDC_LABEL_PRI_PAY, m_nxstaticLabelPriPay);
	DDX_Control(pDX, IDC_LABEL_SEC_PAY, m_nxstaticLabelSecPay);
	DDX_Control(pDX, IDC_LABEL_TER_PAY, m_nxstaticLabelTerPay);
	DDX_Control(pDX, IDC_LABEL3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_LABEL_ADJUSTMENTS, m_nxstaticLabelAdjustments);
	DDX_Control(pDX, IDC_LABEL_PRI_ADJ, m_nxstaticLabelPriAdj);
	DDX_Control(pDX, IDC_LABEL_SEC_ADJ, m_nxstaticLabelSecAdj);
	DDX_Control(pDX, IDC_LABEL_TER_ADJ, m_nxstaticLabelTerAdj);
	DDX_Control(pDX, IDC_LABEL4, m_nxstaticLabel4);
	DDX_Control(pDX, IDC_LABEL_REFUNDS, m_nxstaticLabelRefunds);
	DDX_Control(pDX, IDC_LABEL_PRI_REF, m_nxstaticLabelPriRef);
	DDX_Control(pDX, IDC_LABEL_SEC_REF, m_nxstaticLabelSecRef);
	DDX_Control(pDX, IDC_LABEL_TER_REF, m_nxstaticLabelTerRef);
	DDX_Control(pDX, IDC_LABEL18, m_nxstaticLabel18);
	DDX_Control(pDX, IDC_LABEL_PATRESP, m_nxstaticLabelPatresp);
	DDX_Control(pDX, IDC_LABEL_PRIRESP, m_nxstaticLabelPriresp);
	DDX_Control(pDX, IDC_LABEL_SECRESP, m_nxstaticLabelSecresp);
	DDX_Control(pDX, IDC_LABEL_TERRESP, m_nxstaticLabelTerresp);
	DDX_Control(pDX, IDC_LABEL2, m_nxstaticLabel2);
	DDX_Control(pDX, IDC_LABEL_CHARGES, m_nxstaticLabelCharges);
	DDX_Control(pDX, IDC_LABEL1, m_nxstaticLabel1);
	DDX_Control(pDX, IDC_LABEL_BALANCE, m_nxstaticLabelBalance);
	DDX_Control(pDX, IDC_PAT_NAME_LABEL, m_nxstaticPatNameLabel);
	DDX_Control(pDX, IDC_BTN_UNAPPLY, m_btnUnapply);
	DDX_Control(pDX, IDC_BTN_APPLY_NEW, m_btnApplyNew);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_GROUP1, m_btnGroup1);
	DDX_Control(pDX, IDC_DISPLAY_APPLIES, m_btnDisplayApplies);
	DDX_Control(pDX, IDC_DISPLAY_PAYMENTS, m_btnDisplayPayments);
	//}}AFX_DATA_MAP
}

BEGIN_EVENTSINK_MAP(CApplyManagerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CApplyManagerDlg)
	ON_EVENT(CApplyManagerDlg, IDC_LIST, 6 /* RButtonDown */, OnRButtonDownList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CApplyManagerDlg, IDC_TAB, 1 /* SelectTab */, OnSelectTab, VTS_I2 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CApplyManagerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CApplyManagerDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_BILLS, OnSelchangeComboBills)
	ON_CBN_SELCHANGE(IDC_COMBO_CHARGES, OnSelchangeComboCharges)
	ON_BN_CLICKED(IDC_BTN_APPLY_NEW, OnBtnApplyNew)
	ON_BN_CLICKED(IDC_BTN_UNAPPLY, OnBtnUnapply)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DISPLAY_APPLIES, OnBnClickedDisplayApplies)
	ON_BN_CLICKED(IDC_DISPLAY_PAYMENTS, OnBnClickedDisplayPayments)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApplyManagerDlg message handlers

BOOL CApplyManagerDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (d.thompson 2010-06-15) - PLID 39164 - Start batching settings
		g_propManager.CachePropertiesInBulk("ApplyManager", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'ApplyManager_ShowApplies' "
				")",
				_Q(GetCurrentUserName()));

		// (c.haag 2008-05-01 15:21) - PLID 29871 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnUnapply.AutoSet(NXB_MODIFY);
		m_btnApplyNew.AutoSet(NXB_NEW);

		// (d.thompson 2010-06-15) - PLID 39164 - Remember the setting for 'show applies'
		if(GetRemotePropertyInt("ApplyManager_ShowApplies", 1, 0, GetCurrentUserName(), true) == 0) {
			m_btnDisplayApplies.SetCheck(0);
			m_btnDisplayPayments.SetCheck(1);
		}
		else {
			m_btnDisplayApplies.SetCheck(1);
			m_btnDisplayPayments.SetCheck(0);
		}

		m_List = BindNxDataListCtrl(this,IDC_LIST,GetRemoteData(),false);

		SetDlgItemText(IDC_PAT_NAME_LABEL,GetActivePatientName());

		//Attach to the NxTab - probably should have a global function
		CWnd *pWnd = GetDlgItem(IDC_TAB);
		if (pWnd)
			m_tab = pWnd->GetControlUnknown();
		else m_tab = NULL;

		// (j.jones 2011-03-25 15:39) - PLID 41143 - added "Applies From Payments"
		m_tab->Size = 3;
		m_tab->TabWidth = 3;
		m_tab->Label[0] = "Applies To &Bills";
		m_tab->Label[1] = "Applies &From Payments";
		m_tab->Label[2] = "Applies &To Payments";

		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcApplyID, _T("ApplyID"), _T("ApplyID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcSourcePayID, _T("SourcePayID"), _T("SourcePayID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;		
		// (j.jones 2015-02-24 14:53) - PLID 64940 - ensured this doesn't show times
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcDate, _T("Date"), _T("Date"), 12, csVisible|csWidthPercent)))->FieldType = cftDateShort;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcPaymentTypeID, _T("TypeID"), _T("TypeID"), 0, csVisible | csFixedWidth)))->FieldType = cftTextSingleLine;
		// (j.jones 2015-02-24 14:27) - PLID 64940 - added payment type name
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcPaymentTypeName, _T("Type"), _T("Type"), 14, csVisible | csWidthPercent)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcChargeDesc, _T("ChargeDesc"), _T("Charge"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcPaymentDesc, _T("PayDesc"), _T("Payment"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcApplyAmount, _T("Amount"), _T("Patient"), 12, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcPrimaryAmount, _T("Ins1Amount"), _T("Primary"), 12, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcSecondaryAmount, _T("Ins2Amount"), _T("Secondary"), 12, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcOtherInsAmount, _T("Ins3Amount"), _T("Other"), 12, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
		//TES 7/24/2014 - PLID 62557 - Added ChargebackID; chargebacks can't be unapplied
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(amcChargeBackID, _T("ChargebackID"), _T("ChargebackID"), 0, csVisible | csFixedWidth)))->FieldType = cftTextSingleLine;
		m_List->IsComboBox = FALSE;
		m_List->GetColumn(amcApplyID)->PutSortPriority(0);
		m_List->GetColumn(amcApplyID)->PutSortAscending(TRUE);

		GetMainFrame()->DisableHotKeys();

		// Set default views
		// (j.jones 2011-03-25 15:59) - PLID 41143 - if a default payment ID was given to us,
		// start in the "Applied From Payments" tab
		if(m_iPayID > 0) {
			m_ActiveTab = PAYMENT_APPLIED_FROM_TAB;
			m_btnDisplayApplies.EnableWindow(FALSE);
			m_btnDisplayPayments.EnableWindow(FALSE);
			m_tab->PutCurSel(1);
		}
		else {
			m_ActiveTab = BILL_APPLY_TAB;
			m_tab->PutCurSel(0);
		}

		ChangeDisplay();
	}
	NxCatchAll("Error in CApplyManagerDlg::OnInitDialog");
		
	return TRUE;
}

void CApplyManagerDlg::BuildComboBoxes()
{
	if (m_ActiveTab == BILL_APPLY_TAB) {
		BuildBillsCombo();
		BuildChargesCombo();
	}
	else {
		BuildPaymentsCombo();
	}
}

// Fill the bills combo box
void CApplyManagerDlg::BuildBillsCombo()
{
	_RecordsetPtr rs(__uuidof(Recordset));
	COleVariant var;
	
	try {
		m_adwBillIDs.RemoveAll();
		m_ComboBills.ResetContent();
		// (d.thompson 2010-06-15) - PLID 39164 - Added bill date
		// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
		rs = CreateParamRecordset("SELECT ID, Description, Date "
			"FROM (SELECT BillsT.ID, BillsT.Description, BillsT.Date "
			"	FROM BillsT "
			"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE BillsT.PatientID = {INT} AND BillsT.EntryType = 1 "
			"AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"GROUP BY BillsT.ID, BillsT.Description, BillsT.Date) AS PatientBillsQ", GetActivePatientID());
		while (!rs->eof) {
			var = rs->Fields->Item["Description"]->Value;
			if (var.vt == VT_NULL)
				m_ComboBills.AddString("(no description)");
			else {
				// (d.thompson 2010-06-15) - PLID 39164 - Added bill date
				COleDateTime dtDate = AdoFldDateTime(rs, "Date");
				m_ComboBills.AddString("(" + FormatDateTimeForInterface(dtDate, 0, dtoDate) + ") " + CString(var.bstrVal));
			}

			var = rs->Fields->Item["ID"]->Value;
			m_adwBillIDs.Add(var.lVal);
			rs->MoveNext();
		}
		rs->Close();
	}
	NxCatchAll("ApplyManagerDlg::BuildBillsCombo");

	if (m_ComboBills.GetCount() > 0 && m_iBillID == -2) {
		m_iBillID = m_adwBillIDs.GetAt(0);
		m_ComboBills.SetCurSel(0);
	}
	else if (m_iBillID != -2) {
		for (int i=0; i < m_ComboBills.GetCount(); i++) {
			if ((int)m_adwBillIDs.GetAt(i) == m_iBillID) {
				m_ComboBills.SetCurSel(i);
				break;
			}
		}
	}

	// (j.jones 2011-09-14 15:48) - PLID 44792 - if we tried to set
	// a voided bill, we won't have a selection, so handle accordingly
	if(m_ComboBills.GetCurSel() == -1) {		
		m_iBillID = -2;
		m_iChargeID = -2;
		GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(m_ActiveTab == BILL_APPLY_TAB);
	}
}

// Fill the charges combo box given the current bill
void CApplyManagerDlg::BuildChargesCombo()
{
	_RecordsetPtr rs(__uuidof(Recordset));
	COleVariant var;
	CString str;

	try {
		m_adwChargeIDs.RemoveAll();
		m_ComboCharges.ResetContent();

		// (d.thompson 2010-06-15) - PLID 39164 - Added date of service.
		// (d.thompson 2010-06-15) - PLID 39164 - Added join to CPTCodeT and selected the code
		// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void charges
		rs = CreateParamRecordset("SELECT ID, Description AS ItemDesc, Date, CPTCode "
			"FROM (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, CPTCodeT.Code AS CPTCode "
			"	FROM LineItemT "
			"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"	LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"	WHERE LineItemT.PatientID = {INT} "
			"	AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
			"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			") AS PatientChargesQ "
			"WHERE BillID = {INT}", GetActivePatientID(), m_iBillID);
		while (!rs->eof) {
			var = rs->Fields->Item["ItemDesc"]->Value;
			if (var.vt == VT_NULL)
				m_ComboCharges.AddString("(no description)");
			else {
				// (d.thompson 2010-06-15) - PLID 39164 - Added date of service and cpt code
				CString strCode = AdoFldString(rs, "CPTCode", "");
				COleDateTime dtDate = AdoFldDateTime(rs, "Date");
				CString strToAdd = "(" + FormatDateTimeForInterface(dtDate, 0, dtoDate) + ") " ;
				if(!strCode.IsEmpty()) {
					strToAdd += strCode + " - ";
				}
				strToAdd += CString(var.bstrVal);
				m_ComboCharges.AddString(strToAdd);
			}

			var = rs->Fields->Item["ID"]->Value;
			m_adwChargeIDs.Add(var.lVal);
			rs->MoveNext();
		}
		rs->Close();

		m_ComboCharges.AddString("(All Charges)");
		m_adwChargeIDs.Add(-2);

		if (m_iChargeID == -2) {
			m_ComboCharges.SetCurSel(m_ComboCharges.GetCount()-1);
		}
		else {
			for (int i=0; i < m_ComboCharges.GetCount(); i++) {
				if ((int)m_adwChargeIDs.GetAt(i) == m_iChargeID) {
					m_ComboCharges.SetCurSel(i);
					break;
				}
			}		
		}

		// (j.jones 2011-09-14 15:48) - PLID 44792 - if we tried to set
		// a voided charge, we won't have a selection, so handle accordingly
		GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(m_iBillID != -2 && m_ActiveTab == BILL_APPLY_TAB);
		if(m_ComboCharges.GetCurSel() == -1) {
			// (j.jones 2011-09-14 15:48) - PLID 44792 - need to ensure the tracked ID is now -2
			m_iChargeID = -2;
		}
	}
	NxCatchAll("ApplyManagerDlg::BuildChargesCombo");
}

void CApplyManagerDlg::BuildPaymentsCombo()
{
	_RecordsetPtr rs(__uuidof(Recordset));
	COleVariant var;
	
	try {
		// We'll use the bills combo box and just change
		// the visible name to 'Payments'.
		m_adwPayIDs.RemoveAll();
		m_adwBillIDs.RemoveAll();
		m_ComboBills.ResetContent();
		m_ComboCharges.ResetContent();
		// (d.thompson 2010-06-15) - PLID 39164 - Added payment date
		// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
		// (j.gruber 2013-02-21 13:22) - PLID 22908 - added Insurance Company name and amount
		rs = CreateParamRecordset("SELECT ID AS PaymentID, Description AS Notes, Date, InsuranceCoName, Amount "
			"FROM (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, InsuranceCoT.Name as InsurancecoName "
			"	FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
			"	LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
			"   LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
			"   LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"	WHERE LineItemT.PatientID = {INT} "
			"	AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
			"	AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
			"	AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
			") AS PatientPaymentsQ", GetActivePatientID());
		while (!rs->eof) {
			var = rs->Fields->Item["Notes"]->Value;
			if (var.vt == VT_NULL)
				m_ComboBills.AddString("(no description)");
			else {
				// (d.thompson 2010-06-15) - PLID 39164 - Added payment date
				// (j.gruber 2013-02-21 13:48) - PLID 22908 - ad inscoName and Amount - also increasded the drop down width
				m_ComboBills.SetDroppedWidth(400);
				COleDateTime dtDate = AdoFldDateTime(rs, "Date");
				CString strInsuranceCoName = AdoFldString(rs->Fields, "InsuranceCoName", "");
				COleCurrency cyAmount = AdoFldCurrency(rs->Fields, "Amount");
				CString strMoreInfo;
				if (strInsuranceCoName.IsEmpty()) {
					strMoreInfo = " - " + FormatCurrencyForInterface(cyAmount);
				}
				else {
					strMoreInfo = " - " + strInsuranceCoName + " - " + FormatCurrencyForInterface(cyAmount);
				}

				m_ComboBills.AddString("(" + FormatDateTimeForInterface(dtDate, 0, dtoDate) + ") " + CString(var.bstrVal) + strMoreInfo);
			}

			var = rs->Fields->Item["PaymentID"]->Value;
			m_adwPayIDs.Add(var.lVal);
			rs->MoveNext();
		}
		rs->Close();

		m_ComboBills.AddString("(All Payments)");
		m_adwPayIDs.Add(-2);
	}
	NxCatchAll("ApplyManagerDlg::BuildPaymentsCombo");


	if (m_ComboBills.GetCount() > 0 && m_iPayID == -2) {
		m_iPayID = m_adwPayIDs.GetAt(0);
		m_ComboBills.SetCurSel(0);
	}
	else if (m_iPayID != -2) {
		for (int i=0; i < m_ComboBills.GetCount(); i++) {
			if ((int)m_adwPayIDs.GetAt(i) == m_iPayID) {
				m_ComboBills.SetCurSel(i);
				break;
			}
		}
	}

	// (j.jones 2011-09-14 15:48) - PLID 44792 - if we tried to set
	// a voided payment, we won't have a selection, so handle accordingly
	if(m_ComboBills.GetCurSel() == -1) {
		// (j.jones 2011-09-14 15:48) - PLID 44792 - need to ensure the tracked ID is now -2
		m_iPayID = -2;
		// (j.jones 2015-02-25 10:16) - PLID 64939 - also need to select the (all) row
		m_ComboBills.SetCurSel(m_ComboBills.FindString(0, "(All Payments)"));
		return;
	}
}

// Redraws the list
void CApplyManagerDlg::RefreshList()
{
	CString str,temp;
	_RecordsetPtr rs(__uuidof(Recordset));
	ApplyItem *pNew;

	// (j.jones 2008-05-02 08:43) - PLID 27305 - properly handled deleting from the array
	ClearApplyArray();

	if (m_ActiveTab == BILL_APPLY_TAB) {
		// (d.thompson 2010-06-15) - PLID 39161 - No more list/summary options, always display both
		{
			if (m_iBillID == -2) return;

			try {
				// Fill the list with all non-insurance applies
				if (m_iChargeID == -2) {

					if(m_btnDisplayPayments.GetCheck()) {
						// (d.thompson 2010-06-15) - PLID 39164 - New option, show all payments, not all applies.  Names 
						//	set to match older query, they aren't quite accurate.
						// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
						//TES 7/24/2014 - PLID 62557 - Added ChargebackID
						// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
						rs = CreateParamRecordset("SELECT PaymentsT.ID, LineItemT.Date, PaymentsT.ID AS SourceID, '' AS Description,  "
							"LineItemT.Description AS ApplyDesc, "
							"CASE WHEN PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL THEN LineItemT.Amount ELSE NULL END AS PatResp, "
							"LineItemT.Type,  "
							"CASE WHEN PaymentsT.InsuredPartyID = {INT} AND PaymentsT.InsuredPartyID > 0 THEN LineItemT.Amount ELSE NULL END AS PrimaryResp,  "
							"CASE WHEN PaymentsT.InsuredPartyID = {INT} AND PaymentsT.InsuredPartyID > 0 THEN LineItemT.Amount ELSE NULL END AS SecondaryResp,  "
							"CASE WHEN PaymentsT.InsuredPartyID <> {INT} AND PaymentsT.InsuredPartyID <> {INT} AND PaymentsT.InsuredPartyID > 0 THEN LineItemT.Amount ELSE NULL END AS InactiveResp, "
							"ChargebacksT.ID AS ChargebackID "
							"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
							"WHERE PaymentsT.ID IN ("
							"	SELECT PaymentsT.ID "
							"	FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
							"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
							"	LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
							"	LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
							"	LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID "
							"	LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID "
							"	LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
							"	WHERE AppliesT.PointsToPayments = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type IN (1,2,3) "
							"	AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
							"	AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
							"	AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
							"	AND BillsT.ID = {INT})", m_GuarantorID1, m_GuarantorID2, m_GuarantorID1, m_GuarantorID2, m_iBillID);
					}
					else {
						// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
						//TES 7/24/2014 - PLID 62557 - Added ChargebackID
						str.Format("SELECT PatientChargeAppliesQ.ID, PatientPaymentsQ.Date, "
							"PatientChargeAppliesQ.SourceID, PatientChargesQ.Description, PatientChargeAppliesQ.ApplyDesc, "
							"(CASE WHEN(PatientPaymentsQ.[InsuredPartyID] Is Null OR PatientPaymentsQ.[InsuredPartyID] < 0) THEN PatientChargeAppliesQ.[Amount] ELSE Null END) AS PatResp, "
							"PatientChargeAppliesQ.Type, "
							"(CASE WHEN(PatientPaymentsQ.[InsuredPartyID]=[Ins1ID] AND [Ins1ID] > 0) THEN PatientChargeAppliesQ.[Amount] ELSE Null END) AS PrimaryResp, "
							"(CASE WHEN(PatientPaymentsQ.[InsuredPartyID]=[Ins2ID] AND [Ins2ID] > 0) THEN PatientChargeAppliesQ.[Amount] ELSE Null END) AS SecondaryResp, "
							"(CASE WHEN(PatientPaymentsQ.[InsuredPartyID]<>[Ins1ID] And PatientPaymentsQ.[InsuredPartyID]<>[Ins2ID] And PatientPaymentsQ.[InsuredPartyID] Is Not Null And PatientPaymentsQ.[InsuredPartyID] > 0) "
							"THEN PatientChargeAppliesQ.[Amount] ELSE Null END) AS InactiveResp, "
							"PatientPaymentsQ.ChargebackID "
							"FROM (((SELECT BillsT.* FROM BillsT "
							"	WHERE (((BillsT.PatientID)=(PatientID)) AND ((BillsT.Deleted)=0))) AS PatientBillsQ "
							"LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
							"	FROM LineItemT "
							"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
							"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
							"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
							"	WHERE (((LineItemT.PatientID)=(PatientID)) "
							"	AND ((LineItemT.Deleted)=0) "
							"	AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
							"	AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
							"AND ((LineItemT.Type)>=10))) AS PatientChargesQ ON PatientBillsQ.ID = PatientChargesQ.BillID) "
							"LEFT JOIN (SELECT PatientAppliesQ.* FROM (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
							"FROM AppliesT "
							"LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID FROM LineItemT "
							"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
							"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
							"WHERE (((LineItemT.PatientID)=(PatientID)) AND ((LineItemT.Deleted)=0) "
							"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
							"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
							"AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID WHERE (((PatientPaymentsQ.ID) Is Not Null))) AS PatientAppliesQ "
							"WHERE (((PatientAppliesQ.PointsToPayments)=0))) AS PatientChargeAppliesQ ON PatientChargesQ.ID = PatientChargeAppliesQ.DestID) "
							"LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, ChargebacksT.ID AS ChargebackID "
							"	FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
							"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
							"	LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
							"	LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
							"	WHERE (((LineItemT.PatientID)=(PatientID)) AND ((LineItemT.Deleted)=0) "
							"	AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
							"	AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
							"	AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
							"	AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS PatientPaymentsQ ON PatientChargeAppliesQ.SourceID = PatientPaymentsQ.ID "
							"WHERE PatientBillsQ.ID = %li AND PatientChargeAppliesQ.ID Is Not Null", m_iBillID);
						temp.Format("%li",GetActivePatientID());
						str.Replace((LPCTSTR)"(PatientID)",(LPCTSTR)temp);
						temp.Format("%li",m_GuarantorID1);
						str.Replace((LPCTSTR)"[Ins1ID]",(LPCTSTR)temp);
						temp.Format("%li",m_GuarantorID2);
						str.Replace((LPCTSTR)"[Ins2ID]",(LPCTSTR)temp);
						rs = CreateRecordset(str);
					}
					while(!rs->eof) {
						pNew = new ApplyItem;
						// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed the first two values for clarity
						pNew->ApplyID = rs->Fields->Item["ID"]->Value;
						pNew->SourcePayID = rs->Fields->Item["SourceID"]->Value;
						pNew->Date = rs->Fields->Item["Date"]->Value;
						// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
						long nType = VarLong(rs->Fields->Item["Type"]->Value);
						pNew->PaymentTypeID = nType;
						CString strTypeName = "";
						if (nType == 1) {
							strTypeName = "Payment";
						}
						else if (nType == 2) {
							strTypeName = "Adjustment";
						}
						else if (nType == 3) {
							strTypeName = "Refund";
						}
						else if (nType == 10) {
							strTypeName = "Charge";
						}
						else {
							//what other type could it possibly be?
							strTypeName = "";
							ASSERT(FALSE);
						}
						pNew->PaymentTypeName = _bstr_t(strTypeName);
						pNew->ChargeDesc = rs->Fields->Item["Description"]->Value;
						pNew->PayDesc = rs->Fields->Item["ApplyDesc"]->Value;
						pNew->Amount = rs->Fields->Item["PatResp"]->Value;
						pNew->Ins1Amount = rs->Fields->Item["PrimaryResp"]->Value;
						pNew->Ins2Amount = rs->Fields->Item["SecondaryResp"]->Value;
						pNew->Ins3Amount = rs->Fields->Item["InactiveResp"]->Value;
						pNew->ChargebackID = rs->Fields->Item["ChargebackID"]->Value;
						m_paryApplyManagerT.Add(pNew);
						rs->MoveNext();
					}
					rs->Close();
					
				}
				else {
					if(m_btnDisplayPayments.GetCheck()) {
						// (d.thompson 2010-06-15) - PLID 39164 - New option, show all payments, not all applies.  Names 
						//	set to match older query, they aren't quite accurate.
						// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
						//TES 7/24/2014 - PLID 62557 - Added ChargebackID
						rs = CreateParamRecordset("SELECT PaymentsT.ID, LineItemT.Date, PaymentsT.ID AS SourceID, '' AS Description,  "
							"LineItemT.Description AS ApplyDesc, "
							"CASE WHEN PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL THEN LineItemT.Amount ELSE NULL END AS PatResp, "
							"LineItemT.Type,  "
							"CASE WHEN PaymentsT.InsuredPartyID = {INT} AND PaymentsT.InsuredPartyID > 0 THEN LineItemT.Amount ELSE NULL END AS PrimaryResp,  "
							"CASE WHEN PaymentsT.InsuredPartyID = {INT} AND PaymentsT.InsuredPartyID > 0 THEN LineItemT.Amount ELSE NULL END AS SecondaryResp,  "
							"CASE WHEN PaymentsT.InsuredPartyID <> {INT} AND PaymentsT.InsuredPartyID <> {INT} AND PaymentsT.InsuredPartyID > 0 THEN LineItemT.Amount ELSE NULL END AS InactiveResp, "
							"ChargebacksT.ID AS ChargebackID "
							"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
							"WHERE PaymentsT.ID IN ("
							"SELECT PaymentsT.ID "
							"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
							"LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID "
							"LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID "
							"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
							"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
							"WHERE AppliesT.PointsToPayments = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type IN (1,2,3) "
							"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
							"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
							"AND BillsT.ID = {INT} AND ChargesT.ID = {INT})", m_GuarantorID1, m_GuarantorID2, m_GuarantorID1, m_GuarantorID2, m_iBillID, m_iChargeID);
					}
					else {
						// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
						//TES 7/24/2014 - PLID 62557 - Added ChargebackID
						str.Format("SELECT [PatientChargeAppliesQ].ID, [PatientPaymentsQ].Date, [PatientChargeAppliesQ].SourceID, [PatientChargeAppliesQ].ApplyDesc, [PatientPaymentsQ].Description, (CASE WHEN([PatientPaymentsQ].[InsuredPartyID] Is Null OR [PatientPaymentsQ].[InsuredPartyID] < 0) THEN [PatientChargeAppliesQ].[Amount] ELSE Null END) AS PatResp, [PatientChargeAppliesQ].Type, (CASE WHEN([PatientPaymentsQ].[InsuredPartyID]=[Ins1ID] AND [Ins1ID] > 0) THEN [PatientChargeAppliesQ].[Amount] ELSE Null END) AS PrimaryResp, (CASE WHEN([PatientPaymentsQ].[InsuredPartyID]=[Ins2ID] AND [Ins2ID] > 0) THEN [PatientChargeAppliesQ].[Amount] ELSE Null END) AS SecondaryResp, (CASE WHEN([PatientPaymentsQ].[InsuredPartyID]<>[Ins1ID] And [PatientPaymentsQ].[InsuredPartyID]<>[Ins2ID] And [PatientPaymentsQ].[InsuredPartyID] Is Not Null And [PatientPaymentsQ].[InsuredPartyID] > 0) THEN [PatientChargeAppliesQ].[Amount] ELSE Null END) AS InactiveResp, "
								"PatientPaymentsQ.ChargebackID "
								"FROM ((SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
								"	FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
								"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
								"	WHERE (((LineItemT.PatientID)=(PatientID)) AND ((LineItemT.Deleted)=0) "
								"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
								"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
								"	AND ((LineItemT.Type)>=10))) AS PatientChargesQ "
								"LEFT JOIN (SELECT PatientAppliesQ.* "
								"	FROM (SELECT AppliesT.*, PatientPaymentsQ.Type, PatientPaymentsQ.Description AS ApplyDesc, PatientPaymentsQ.InsuredPartyID "
								"		FROM AppliesT LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID "
								"			FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								"			LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
								"			LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
								"			LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
								"			WHERE (((LineItemT.PatientID)=(PatientID)) AND ((LineItemT.Deleted)=0) "
								"			AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
								"			AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
								"			AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
								"			AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID "
								"WHERE (((PatientPaymentsQ.ID) Is Not Null))) AS PatientAppliesQ "
								"WHERE (((PatientAppliesQ.PointsToPayments)=0))) AS PatientChargeAppliesQ ON [PatientChargesQ].ID = [PatientChargeAppliesQ].DestID) "
								"LEFT JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, ChargebacksT.ID AS ChargebackID "
								"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
								"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
								"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
								"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
								"WHERE (((LineItemT.PatientID)=(PatientID)) AND ((LineItemT.Deleted)=0) "
								"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
								"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
								"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
								"AND ((LineItemT.Type)>=1 And (LineItemT.Type)<=3))) AS PatientPaymentsQ ON [PatientChargeAppliesQ].SourceID = [PatientPaymentsQ].ID "
								"WHERE [PatientChargesQ].ID = %li AND (([PatientPaymentsQ].ID) Is Not Null)", m_iChargeID);
						temp.Format("%li",GetActivePatientID());
						str.Replace((LPCTSTR)"(PatientID)",(LPCTSTR)temp);
						temp.Format("%li",m_GuarantorID1);
						str.Replace((LPCTSTR)"[Ins1ID]",(LPCTSTR)temp);
						temp.Format("%li",m_GuarantorID2);
						str.Replace((LPCTSTR)"[Ins2ID]",(LPCTSTR)temp);
						rs = CreateRecordset(str);
					}
					while(!rs->eof) {
						pNew = new ApplyItem;
						// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed the first two values for clarity
						pNew->ApplyID = rs->Fields->Item["ID"]->Value;
						pNew->SourcePayID = rs->Fields->Item["SourceID"]->Value;
						pNew->Date = rs->Fields->Item["Date"]->Value;
						// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
						long nType = VarLong(rs->Fields->Item["Type"]->Value);
						pNew->PaymentTypeID = nType;
						CString strTypeName = "";
						if (nType == 1) {
							strTypeName = "Payment";
						}
						else if (nType == 2) {
							strTypeName = "Adjustment";
						}
						else if (nType == 3) {
							strTypeName = "Refund";
						}
						else if (nType == 10) {
							strTypeName = "Charge";
						}
						else {
							//what other type could it possibly be?
							strTypeName = "";
							ASSERT(FALSE);
						}
						pNew->PaymentTypeName = _bstr_t(strTypeName);
						pNew->ChargeDesc = rs->Fields->Item["Description"]->Value;
						pNew->PayDesc = rs->Fields->Item["ApplyDesc"]->Value;
						pNew->Amount = rs->Fields->Item["PatResp"]->Value;
						pNew->Ins1Amount = rs->Fields->Item["PrimaryResp"]->Value;
						pNew->Ins2Amount = rs->Fields->Item["SecondaryResp"]->Value;
						pNew->Ins3Amount = rs->Fields->Item["InactiveResp"]->Value;
						pNew->ChargebackID = rs->Fields->Item["ChargebackID"]->Value;
						m_paryApplyManagerT.Add(pNew);
						rs->MoveNext();
					}
					rs->Close();
				}
			}
			NxCatchAll("ApplyManagerDlg::RefreshList (1)");

			// Redraw and color the list
			FillList();
			ColorList();
		}
		// (d.thompson 2010-06-15) - PLID 39161 - No more list/summary views, always display both
		{
			RefreshSummary();
		}
	}
	// (j.jones 2011-03-25 15:39) - PLID 41143 - added "Applies From Payments"
	else if (m_ActiveTab == PAYMENT_APPLIED_FROM_TAB) {
		try {
			// Fill the list with all non-insurance applies
			if (m_iPayID == -2) {
				//DRT TODO - This is hardcoded setup to pri/sec/inactive... it really needs to be revisited and see if 
				//		it should support the multiple insurance stuff.
				// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
				//TES 7/24/2014 - PLID 62557 - Added ChargebackID
				rs = CreateParamRecordset("SELECT AppliesT.ID, AppliesT.SourceID, [Dest Items].Type, "
					"[Dest Items].Date, [Dest Items].Description AS ApplyDesc, "
					"LineItemT.Description, "
					"(CASE WHEN RespTypeID Is Null THEN AppliesT.Amount ELSE NULL END) AS PatientAmount, "
					"(CASE WHEN RespTypeID = 1 THEN AppliesT.Amount ELSE NULL END) AS PrimaryAmount, "
					"(CASE WHEN RespTypeID = 2 THEN AppliesT.Amount ELSE NULL END) AS SecondaryAmount, "
					"(CASE WHEN RespTypeID = -1 THEN AppliesT.Amount ELSE NULL END) AS InactiveAmount, "
					"ChargebacksT.ID AS ChargebackID "
					"FROM LineItemT AS [Dest Items] "
					"INNER JOIN AppliesT ON [Dest Items].ID = AppliesT.DestID "
					"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
					"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
					"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
					"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
					"WHERE [Dest Items].PatientID = {INT} "
					"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
					"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
					"GROUP BY AppliesT.ID, AppliesT.SourceID, [Dest Items].Type, "
					"[Dest Items].Date, [Dest Items].Description, LineItemT.Description,"
					"AppliesT.Amount, AppliesT.DestID, InsuredPartyT.RespTypeID, ChargebacksT.ID", GetActivePatientID());
				while(!rs->eof) {
					pNew = new ApplyItem;
					// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed the first two values for clarity
					pNew->ApplyID = rs->Fields->Item["ID"]->Value;
					pNew->SourcePayID = rs->Fields->Item["SourceID"]->Value;
					pNew->Date = rs->Fields->Item["Date"]->Value;
					// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
					long nType = VarLong(rs->Fields->Item["Type"]->Value);
					pNew->PaymentTypeID = nType;
					CString strTypeName = "";
					if (nType == 1) {
						strTypeName = "Payment";
					}
					else if (nType == 2) {
						strTypeName = "Adjustment";
					}
					else if (nType == 3) {
						strTypeName = "Refund";
					}
					else if (nType == 10) {
						strTypeName = "Charge";
					}
					else {
						//what other type could it possibly be?
						strTypeName = "";
						ASSERT(FALSE);
					}
					pNew->PaymentTypeName = _bstr_t(strTypeName);
					pNew->ChargeDesc = rs->Fields->Item["ApplyDesc"]->Value;
					pNew->PayDesc = rs->Fields->Item["Description"]->Value;
					pNew->Amount = rs->Fields->Item["PatientAmount"]->Value;
					pNew->Ins1Amount = rs->Fields->Item["PrimaryAmount"]->Value;
					pNew->Ins2Amount = rs->Fields->Item["SecondaryAmount"]->Value;
					pNew->Ins3Amount = rs->Fields->Item["InactiveAmount"]->Value;
					pNew->ChargebackID = rs->Fields->Item["ChargebackID"]->Value;
					m_paryApplyManagerT.Add(pNew);
					rs->MoveNext();
				}
				rs->Close();

			}
			else {
				//DRT TODO - This is hardcoded to pri/sec/inactive ... see if it needs changed to support
				//		multi ins resps.
				// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
				//TES 7/24/2014 - PLID 62557 - Added ChargebackID
				rs = CreateParamRecordset("SELECT AppliesT.ID, AppliesT.SourceID, [Dest Items].Type, "
					"[Dest Items].Date, [Dest Items].Description AS ApplyDesc, "
					"LineItemT.Description, "
					"(CASE WHEN RespTypeID Is Null THEN AppliesT.Amount ELSE NULL END) AS PatientAmount, "
					"(CASE WHEN RespTypeID = 1 THEN AppliesT.Amount ELSE NULL END) AS PrimaryAmount, "
					"(CASE WHEN RespTypeID = 2 THEN AppliesT.Amount ELSE NULL END) AS SecondaryAmount, "
					"(CASE WHEN RespTypeID = -1 THEN AppliesT.Amount ELSE NULL END) AS InactiveAmount, "
					"ChargebacksT.ID AS ChargebackID "
					"FROM LineItemT AS [Dest Items] "
					"INNER JOIN AppliesT ON [Dest Items].ID = AppliesT.DestID "
					"INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON PaymentsT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON PaymentsT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
					"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON PaymentsT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
					"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
					"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
					"WHERE [Dest Items].PatientID = {INT} "
					"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
					"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
					"GROUP BY LineItemT.ID, AppliesT.ID, [Dest Items].ID, [Dest Items].Type, "
					"[Dest Items].Date, [Dest Items].Description, LineItemT.Description,"
					"AppliesT.Amount, AppliesT.SourceID, InsuredPartyT.RespTypeID, "
					"ChargebacksT.ID "
					"HAVING LineItemT.ID = {INT}", GetActivePatientID(), m_iPayID);
				while(!rs->eof) {
					pNew = new ApplyItem;
					// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed the first two values for clarity
					pNew->ApplyID = rs->Fields->Item["ID"]->Value;
					pNew->SourcePayID = rs->Fields->Item["SourceID"]->Value;
					pNew->Date = rs->Fields->Item["Date"]->Value;
					// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
					long nType = VarLong(rs->Fields->Item["Type"]->Value);
					pNew->PaymentTypeID = nType;
					CString strTypeName = "";
					if (nType == 1) {
						strTypeName = "Payment";
					}
					else if (nType == 2) {
						strTypeName = "Adjustment";
					}
					else if (nType == 3) {
						strTypeName = "Refund";
					}
					else if (nType == 10) {
						strTypeName = "Charge";
					}
					else {
						//what other type could it possibly be?
						strTypeName = "";
						ASSERT(FALSE);
					}
					pNew->PaymentTypeName = _bstr_t(strTypeName);
					pNew->ChargeDesc = rs->Fields->Item["ApplyDesc"]->Value;
					pNew->PayDesc = rs->Fields->Item["Description"]->Value;
					pNew->Amount = rs->Fields->Item["PatientAmount"]->Value;
					pNew->Ins1Amount = rs->Fields->Item["PrimaryAmount"]->Value;
					pNew->Ins2Amount = rs->Fields->Item["SecondaryAmount"]->Value;
					pNew->Ins3Amount = rs->Fields->Item["InactiveAmount"]->Value;
					pNew->ChargebackID = rs->Fields->Item["ChargebackID"]->Value;
					m_paryApplyManagerT.Add(pNew);
					rs->MoveNext();
				}
				rs->Close();

			}
		}
		NxCatchAll("ApplyManagerDlg::RefreshList (2)");

		// Redraw and color the list
		FillList();
		ColorList();
	}
	else {
		// (d.thompson 2010-06-15) - PLID 39161 - No more list/summary options, always the same view
		{

			try {
				// Fill the list with all non-insurance applies
				if (m_iPayID == -2) {
					//DRT TODO - This is hardcoded setup to pri/sec/inactive... it really needs to be revisited and see if 
					//		it should support the multiple insurance stuff.
					// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
					//TES 7/24/2014 - PLID 62557 - Added ChargebackID
					rs = CreateParamRecordset("SELECT AppliesT.ID, [Source Payments].ID AS SourceID, [Source Payments].Type, "
						"[Source Payments].Date, [Source Payments].Description AS ApplyDesc, "
						"LineItemT.Description, "
						"(CASE WHEN RespTypeID Is Null THEN AppliesT.Amount ELSE NULL END) AS PatientAmount, "
						"(CASE WHEN RespTypeID = 1 THEN AppliesT.Amount ELSE NULL END) AS PrimaryAmount, "
						"(CASE WHEN RespTypeID = 2 THEN AppliesT.Amount ELSE NULL END) AS SecondaryAmount, "
						"(CASE WHEN RespTypeID = -1 THEN AppliesT.Amount ELSE NULL END) AS InactiveAmount, "
						"ChargebacksT.ID AS ChargebackID "
						"FROM LineItemT AS [Source Payments] "
						"LEFT JOIN AppliesT ON [Source Payments].ID = AppliesT.SourceID "
						"LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
						"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON [Source Payments].ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON [Source Payments].ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
						"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON [Source Payments].ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
						"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
						"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
						"WHERE [Source Payments].PatientID = {INT} "
						"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
						"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
						"GROUP BY AppliesT.ID, [Source Payments].ID, [Source Payments].Type, "
						"[Source Payments].Date, [Source Payments].Description, LineItemT.Description,"
						"AppliesT.Amount, AppliesT.DestID, InsuredPartyT.RespTypeID, "
						"Convert(int,AppliesT.PointsToPayments), "
						"ChargebacksT.ID "
						"HAVING Convert(int,AppliesT.PointsToPayments) = 1",GetActivePatientID());
					while(!rs->eof) {
						pNew = new ApplyItem;
						// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed the first two values for clarity
						pNew->ApplyID = rs->Fields->Item["ID"]->Value;
						pNew->SourcePayID = rs->Fields->Item["SourceID"]->Value;
						pNew->Date = rs->Fields->Item["Date"]->Value;
						// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
						long nType = VarLong(rs->Fields->Item["Type"]->Value);
						pNew->PaymentTypeID = nType;
						CString strTypeName = "";
						if (nType == 1) {
							strTypeName = "Payment";
						}
						else if (nType == 2) {
							strTypeName = "Adjustment";
						}
						else if (nType == 3) {
							strTypeName = "Refund";
						}
						else if (nType == 10) {
							strTypeName = "Charge";
						}
						else {
							//what other type could it possibly be?
							strTypeName = "";
							ASSERT(FALSE);
						}
						pNew->PaymentTypeName = _bstr_t(strTypeName);
						pNew->ChargeDesc = rs->Fields->Item["Description"]->Value;
						pNew->PayDesc = rs->Fields->Item["ApplyDesc"]->Value;
						pNew->Amount = rs->Fields->Item["PatientAmount"]->Value;
						pNew->Ins1Amount = rs->Fields->Item["PrimaryAmount"]->Value;
						pNew->Ins2Amount = rs->Fields->Item["SecondaryAmount"]->Value;
						pNew->Ins3Amount = rs->Fields->Item["InactiveAmount"]->Value;
						pNew->ChargebackID = rs->Fields->Item["ChargebackID"]->Value;
						m_paryApplyManagerT.Add(pNew);
						rs->MoveNext();
					}
					rs->Close();

				}
				else {
					//DRT TODO - This is hardcoded to pri/sec/inactive ... see if it needs changed to support
					//		multi ins resps.
					// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
					//TES 7/24/2014 - PLID 62557 - Added ChargebackID
					rs = CreateParamRecordset("SELECT AppliesT.ID, [Source Payments].ID AS SourceID, [Source Payments].Type, "
						"[Source Payments].Date, [Source Payments].Description AS ApplyDesc, "
						"LineItemT.Description, "
						"(CASE WHEN RespTypeID Is Null THEN AppliesT.Amount ELSE NULL END) AS PatientAmount, "
						"(CASE WHEN RespTypeID = 1 THEN AppliesT.Amount ELSE NULL END) AS PrimaryAmount, "
						"(CASE WHEN RespTypeID = 2 THEN AppliesT.Amount ELSE NULL END) AS SecondaryAmount, "
						"(CASE WHEN RespTypeID = -1 THEN AppliesT.Amount ELSE NULL END) AS InactiveAmount, "
						"ChargebacksT.ID AS ChargebackID "
						"FROM LineItemT AS [Source Payments] "
						"LEFT JOIN AppliesT ON [Source Payments].ID = AppliesT.SourceID "
						"LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
						"LEFT JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON [Source Payments].ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID " 
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON [Source Payments].ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
						"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON [Source Payments].ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
						"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
						"LEFT JOIN ChargebacksT ON PaymentsT.ID = ChargebacksT.PaymentID OR PaymentsT.ID = ChargebacksT.AdjustmentID "
						"WHERE [Source Payments].PatientID = {INT} "
						"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
						"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null "
						"GROUP BY LineItemT.ID, AppliesT.ID, [Source Payments].ID, [Source Payments].Type, "
						"[Source Payments].Date, [Source Payments].Description, LineItemT.Description,"
						"AppliesT.Amount, AppliesT.DestID, InsuredPartyT.RespTypeID, "
						"Convert(int,AppliesT.PointsToPayments), "
						"ChargebacksT.ID "
						"HAVING Convert(int,AppliesT.PointsToPayments) = 1 AND LineItemT.ID = {INT}",GetActivePatientID(), m_iPayID);
					while(!rs->eof) {
						pNew = new ApplyItem;
						// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed the first two values for clarity
						pNew->ApplyID = rs->Fields->Item["ID"]->Value;
						pNew->SourcePayID = rs->Fields->Item["SourceID"]->Value;
						pNew->Date = rs->Fields->Item["Date"]->Value;
						// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
						long nType = VarLong(rs->Fields->Item["Type"]->Value);
						pNew->PaymentTypeID = nType;
						CString strTypeName = "";
						if (nType == 1) {
							strTypeName = "Payment";
						}
						else if (nType == 2) {
							strTypeName = "Adjustment";
						}
						else if (nType == 3) {
							strTypeName = "Refund";
						}
						else if (nType == 10) {
							strTypeName = "Charge";
						}
						else {
							//what other type could it possibly be?
							strTypeName = "";
							ASSERT(FALSE);
						}
						pNew->PaymentTypeName = _bstr_t(strTypeName);
						pNew->ChargeDesc = rs->Fields->Item["Description"]->Value;
						pNew->PayDesc = rs->Fields->Item["ApplyDesc"]->Value;
						pNew->Amount = rs->Fields->Item["PatientAmount"]->Value;
						pNew->Ins1Amount = rs->Fields->Item["PrimaryAmount"]->Value;
						pNew->Ins2Amount = rs->Fields->Item["SecondaryAmount"]->Value;
						pNew->Ins3Amount = rs->Fields->Item["InactiveAmount"]->Value;
						pNew->ChargebackID = rs->Fields->Item["ChargebackID"]->Value;
						m_paryApplyManagerT.Add(pNew);
						rs->MoveNext();
					}
					rs->Close();

				}
			}
			NxCatchAll("ApplyManagerDlg::RefreshList (3)");

			// Redraw and color the list
			FillList();
			ColorList();
		}
	}
}

// Redraw all the numbers in the summary view
void CApplyManagerDlg::RefreshSummary()
{
	COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds;
	COleCurrency cyIns1Resp, cyIns2Resp, cyIns3Resp, cyIns1Applies, cyIns2Applies, cyIns3Applies;
	CString str, strFilter, strGroup;
	_RecordsetPtr rs(__uuidof(Recordset));
	COleVariant var;

	BeginWaitCursor();
	if (m_ActiveTab != BILL_APPLY_TAB) return;
	if (m_iBillID == -2) return;

	///////////////////////////////////////////////
	// Fill all the primary insurance apply information
	///////////////////////////////////////////////

	// (j.jones 2011-07-22 12:15) - PLID 42231 - get the primary insured party ID
	long nPriInsPartyID = GetInsuranceIDFromType(GetActivePatientID(), 1);
	if (m_iChargeID == -2)
		GetBillInsuranceTotals(m_iBillID, GetActivePatientID(), nPriInsPartyID, &cyIns1Resp, &cyPayments, &cyAdjustments, &cyRefunds);
	else
		GetChargeInsuranceTotals(m_iChargeID, GetActivePatientID(), nPriInsPartyID, &cyIns1Resp, &cyPayments, &cyAdjustments, &cyRefunds);

	cyIns1Applies = cyPayments + cyAdjustments + cyRefunds;	
	GetDlgItem(IDC_LABEL_TTL_PRIRESP)->SetWindowText(  FormatCurrencyForInterface(cyIns1Resp));
	GetDlgItem(IDC_LABEL_PRI_PAY)->SetWindowText( FormatCurrencyForInterface(cyPayments));
	// (a.walling 2007-11-05 13:20) - PLID 27974 - VS2008 - Ambiguous operator *
	GetDlgItem(IDC_LABEL_PRI_ADJ)->SetWindowText( FormatCurrencyForInterface(cyAdjustments * long(-1)));
	GetDlgItem(IDC_LABEL_PRI_REF)->SetWindowText( FormatCurrencyForInterface(cyRefunds));
	GetDlgItem(IDC_LABEL_PRIRESP)->SetWindowText( FormatCurrencyForInterface(cyIns1Resp - cyIns1Applies));

	///////////////////////////////////////////////
	// Fill all the secondary insurance apply information
	///////////////////////////////////////////////
	// (j.jones 2011-07-22 12:15) - PLID 42231 - get the secondary insured party ID
	long nSecInsPartyID = GetInsuranceIDFromType(GetActivePatientID(), 2);
	if (m_iChargeID == -2)
		GetBillInsuranceTotals(m_iBillID, GetActivePatientID(), nSecInsPartyID, &cyIns2Resp, &cyPayments, &cyAdjustments, &cyRefunds);
	else
		GetChargeInsuranceTotals(m_iChargeID, GetActivePatientID(), nSecInsPartyID, &cyIns2Resp, &cyPayments, &cyAdjustments, &cyRefunds);

	cyIns2Applies = cyPayments + cyAdjustments + cyRefunds;
	GetDlgItem(IDC_LABEL_TTL_SECRESP)->SetWindowText( FormatCurrencyForInterface(cyIns2Resp));
	GetDlgItem(IDC_LABEL_SEC_PAY)->SetWindowText( FormatCurrencyForInterface(cyPayments));
	// (a.walling 2007-11-05 13:20) - PLID 27974 - VS2008 - Ambiguous operator *
	GetDlgItem(IDC_LABEL_SEC_ADJ)->SetWindowText( FormatCurrencyForInterface(cyAdjustments * long(-1)));
	GetDlgItem(IDC_LABEL_SEC_REF)->SetWindowText( FormatCurrencyForInterface(cyRefunds));
	GetDlgItem(IDC_LABEL_SECRESP)->SetWindowText( FormatCurrencyForInterface(cyIns2Resp - cyIns2Applies));

	///////////////////////////////////////////////
	// Fill all the other insurance apply information
	///////////////////////////////////////////////
	//DRT 6/10/03 - This needs to loop through all insurance resps, not hardcoded to the 3rd one (which used to be inactive)
	//		Exclude pri/sec, but include inactive + all 3 and above.
	COleCurrency cyIns3Total, cyPayTotal, cyAdjTotal, cyRefTotal;
	try {
		// (b.cardillo 2011-06-17 16:30) - PLID 39176 - Get all non-primary non-secondary insured parties (since we 
		// got those above).
		long nPatientID = GetActivePatientID();
		_RecordsetPtr prs = CreateParamRecordset(
			_T("SELECT PersonID AS InsPartyID FROM InsuredPartyT ")
			_T("WHERE PatientID = {INT} AND RespTypeID NOT IN (1, 2) ")
			, nPatientID);
		FieldPtr fldInsPartyID = prs->GetFields()->GetItem(_T("InsPartyID"));
		while(!prs->eof) {
			long nInsPartyID = VarLong(fldInsPartyID->GetValue());
			if (m_iChargeID == -2)
				// (j.jones 2011-07-22 12:35) - PLID 42231 - renamed to GetBillInsuranceTotals
				GetBillInsuranceTotals(m_iBillID, GetActivePatientID(), nInsPartyID, &cyIns3Resp, &cyPayments, &cyAdjustments, &cyRefunds);
			else
				GetChargeInsuranceTotals(m_iChargeID, GetActivePatientID(), nInsPartyID, &cyIns3Resp, &cyPayments, &cyAdjustments, &cyRefunds);

			cyIns3Total += cyIns3Resp;
			cyPayTotal += cyPayments;
			cyAdjTotal += cyAdjustments;
			cyRefTotal += cyRefunds;

			prs->MoveNext();
		}
	} NxCatchAll("Error determining other insurance amounts.");

	cyIns3Applies = cyPayTotal + cyAdjTotal + cyRefTotal;
	GetDlgItem(IDC_LABEL_TTL_TERRESP)->SetWindowText( FormatCurrencyForInterface(cyIns3Total));
	GetDlgItem(IDC_LABEL_TER_PAY)->SetWindowText( FormatCurrencyForInterface(cyPayTotal));
	// (a.walling 2007-11-05 13:20) - PLID 27974 - VS2008 - Ambiguous operator *
	GetDlgItem(IDC_LABEL_TER_ADJ)->SetWindowText( FormatCurrencyForInterface(cyAdjTotal * long(-1)));
	GetDlgItem(IDC_LABEL_TER_REF)->SetWindowText( FormatCurrencyForInterface(cyRefTotal));
	GetDlgItem(IDC_LABEL_TERRESP)->SetWindowText( FormatCurrencyForInterface(cyIns3Total - cyIns3Applies));

	///////////////////////////////////////////////
	// Fill all the non-insurance apply information
	///////////////////////////////////////////////
	if (m_iChargeID == -2) {
		GetBillTotals(m_iBillID, GetActivePatientID(), &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, 0);
	}
	else {
		GetChargeTotals(m_iChargeID, GetActivePatientID(), &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, 0);
	}

	GetDlgItem(IDC_LABEL_CHARGES)->SetWindowText( FormatCurrencyForInterface(cyCharges));
	GetDlgItem(IDC_LABEL_PAYMENTS)->SetWindowText( FormatCurrencyForInterface(cyPayments));
	// (a.walling 2007-11-05 13:20) - PLID 27974 - VS2008 - Ambiguous operator *
	GetDlgItem(IDC_LABEL_ADJUSTMENTS)->SetWindowText( FormatCurrencyForInterface(cyAdjustments * long(-1)));
	GetDlgItem(IDC_LABEL_REFUNDS)->SetWindowText( FormatCurrencyForInterface(cyRefunds));
	GetDlgItem(IDC_LABEL_TTL_PATRESP)->SetWindowText( FormatCurrencyForInterface(cyCharges - cyIns1Resp - cyIns2Resp - cyIns3Resp));
	GetDlgItem(IDC_LABEL_PATRESP)->SetWindowText( FormatCurrencyForInterface(cyCharges - cyPayments - cyAdjustments - cyRefunds - cyIns1Resp - cyIns2Resp - cyIns3Resp));
	GetDlgItem(IDC_LABEL_BALANCE)->SetWindowText( FormatCurrencyForInterface(cyCharges - cyPayments - cyAdjustments - cyRefunds - cyIns1Applies - cyIns2Applies - cyIns3Applies));

	EndWaitCursor();
}

// (d.thompson 2010-06-15) - PLID 39161 - Summary view function
void CApplyManagerDlg::ShowSummary(BOOL bShow)
{
	int nCmdShow = SW_HIDE;
	if(bShow) {
		nCmdShow = SW_SHOW;
	}

	GetDlgItem(IDC_GROUP1)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL10)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL11)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL12)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL13)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL5)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL8)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL3)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL4)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL18)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_CHARGES)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL1)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_BALANCE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TTL_PATRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_PAYMENTS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_ADJUSTMENTS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_REFUNDS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_PATRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TTL_PRIRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_PRI_PAY)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_PRI_ADJ)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_PRI_REF)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_PRIRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TTL_SECRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_SEC_PAY)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_SEC_ADJ)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_SEC_REF)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_SECRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TTL_TERRESP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TER_PAY)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TER_ADJ)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TER_REF)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_LABEL_TERRESP)->ShowWindow(nCmdShow);
}

// Color the list according to the payment method
// of each apply
void CApplyManagerDlg::ColorList()
{
	COleVariant var;
	int count = m_List->GetRowCount();
	IRowSettingsPtr pRow;

	for (int i=0; i < count; i++) {
		var = m_List->GetValue(i, amcPaymentTypeID);
		if (var.lVal == 2) {
			pRow = m_List->GetRow(i);
			pRow->PutForeColor(0x00664900);
		}
		else if (var.lVal == 3) {
			pRow = m_List->GetRow(i);
			pRow->PutForeColor(0x00005050);
		}
		else {
			pRow = m_List->GetRow(i);
			pRow->PutForeColor(0x00153C00);
		}
	}
}

void CApplyManagerDlg::OnSelchangeComboBills() 
{
	if (m_ComboBills.GetCurSel() == -1) {
		// (j.jones 2011-09-14 15:48) - PLID 44792 - need to ensure the tracked IDs are now -2
		if(m_ActiveTab == BILL_APPLY_TAB) {
			m_iBillID = -2;
			m_iChargeID = -2;
		}
		else {
			m_iPayID = -2;
		}
		GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(FALSE);
		return;
	}

	GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(m_ActiveTab == BILL_APPLY_TAB);

	if (m_ActiveTab == BILL_APPLY_TAB) {
		m_iBillID = m_adwBillIDs[ m_ComboBills.GetCurSel() ];
		m_iChargeID = -2;
		BuildChargesCombo();
	}
	else {
		m_iPayID = m_adwPayIDs[ m_ComboBills.GetCurSel() ];
	}
	RefreshList();
}

void CApplyManagerDlg::OnSelchangeComboCharges() 
{
	GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(m_iBillID != -2 && m_ActiveTab == BILL_APPLY_TAB);

	if(m_ComboCharges.GetCurSel() == -1) {
		// (j.jones 2011-09-14 15:48) - PLID 44792 - need to ensure the tracked ID is now -2
		m_iChargeID = -2;
		return;
	}

	m_iChargeID = m_adwChargeIDs[ m_ComboCharges.GetCurSel() ];
	RefreshList();	
}

void CApplyManagerDlg::OnBtnApplyNew() 
{
	CPaymentDlg dlg(this);
	COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;

	if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) {
		return;
	}
	
	// (j.jones 2011-09-14 15:48) - PLID 44792 - check the combo boxes to make sure that
	// something is selected in our dropdowns
	if(m_iBillID == -2 && m_ComboBills.GetCurSel() != -1) {

		AfxMessageBox("Please select a bill before trying to apply.");
		return;
	}
	else if(m_iChargeID == -2 || m_ComboCharges.GetCurSel() == -1) {
		dlg.m_varBillID = COleVariant((long)m_iBillID);
		dlg.m_ApplyOnOK = TRUE;
		GetBillTotals(m_iBillID, GetActivePatientID(), &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
	}
	else {
		dlg.m_varChargeID = COleVariant((long)m_iChargeID);
		dlg.m_ApplyOnOK = TRUE;
		GetChargeTotals(m_iChargeID, GetActivePatientID(), &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
	}

	dlg.m_cyFinalAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;
	dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;

	dlg.DoModal(__FUNCTION__, __LINE__);

	RefreshList();
}

//BVB - in the future, try not to use PreTranslateMessage for hotkeys, either use a hook, or the global hotkeys (which should also use a hook)
// Used for hot keys
// (d.thompson 2010-06-15) - PLID 39161 - Removed 'S' and 'T' hotkeys when removing Summary and List views.
BOOL CApplyManagerDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_SYSKEYDOWN) {
		switch (pMsg->wParam) {
		case 'O':
			OnOK();
		case 'L':
			if (m_ComboBills.IsWindowEnabled() && m_ActiveTab == BILL_APPLY_TAB)
				m_ComboBills.SetFocus();
			break;
		case 'A':
			if (m_ComboBills.IsWindowEnabled() && (m_ActiveTab == PAYMENT_APPLIED_TO_TAB || m_ActiveTab == PAYMENT_APPLIED_FROM_TAB))
				m_ComboBills.SetFocus();
			break;
		case 'C':
			if (m_ComboCharges.IsWindowEnabled())
				m_ComboCharges.SetFocus();
			break;
		case 'B':
			m_tab->CurSel = 0;
			{
			short tab = 0;
			OnSelectTab(tab,tab);
			}
			break;
		// (j.jones 2011-03-25 15:44) - PLID 41143 - added "Applies From Payments",
		// which also changed the hotkeys and tab indexes for payment options
		case 'F':
			m_tab->CurSel = 1;
			{
			short tab = 1;
			OnSelectTab(tab,tab);
			}
			break;
		case 'T':
			m_tab->CurSel = 2;
			{
			short tab = 2;
			OnSelectTab(tab,tab);
			}
			break;

//		case 'L':
//			if (GetDlgItem(IDC_COMBO_BILLS)->IsWindowEnabled())
//				GetDlgItem(IDC_COMBO_BILLS)->SetFocus();
//			break;
		case 'N':
			if (GetDlgItem(IDC_BTN_APPLY_NEW)->IsWindowEnabled()) {
				GetDlgItem(IDC_BTN_APPLY_NEW)->SetFocus();
				OnBtnApplyNew();
			}
		case 'U':
			if (GetDlgItem(IDC_BTN_UNAPPLY)->IsWindowEnabled()) {
				GetDlgItem(IDC_BTN_UNAPPLY)->SetFocus();
				OnBtnUnapply();
			}
			break;
/*		case 'A':
			if (GetDlgItem(IDC_RADIO_PATIENT)->IsWindowEnabled()) {
				GetDlgItem(IDC_RADIO_PATIENT)->SetFocus();
				((CButton*)GetDlgItem(IDC_RADIO_PATIENT))->SetCheck(1);
				((CButton*)GetDlgItem(IDC_RADIO_INSURANCE))->SetCheck(0);
				RefreshList();
			}
			break;
		case 'I':
			if (GetDlgItem(IDC_RADIO_INSURANCE)->IsWindowEnabled()) {
				GetDlgItem(IDC_RADIO_INSURANCE)->SetFocus();
				((CButton*)GetDlgItem(IDC_RADIO_INSURANCE))->SetCheck(1);
				((CButton*)GetDlgItem(IDC_RADIO_PATIENT))->SetCheck(0);
				RefreshList();
			}
			break;*/
		}
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CApplyManagerDlg::OnBtnUnapply() 
{
	try {
	
		if(m_List->GetCurSel() == -1) {
			AfxMessageBox("Please make a selection before unapplying.");
			return;
		}

		COleVariant varApplyID;
		varApplyID = m_List->GetValue(m_List->GetCurSel(), amcApplyID);
	
		if (varApplyID.vt != VT_EMPTY) {

			// (j.jones 2011-09-13 15:35) - PLID 44887 - disallow on original/void items
			if(IsOriginalOrVoidApply(VarLong(varApplyID))) {
				AfxMessageBox("This apply is part of a line item correction and cannot be unapplied.");
				return;
			}

			//TES 7/24/2014 - PLID 62557 - Prevent chargebacks from being unapplied
			if (VarLong(m_List->GetValue(m_List->CurSel, amcChargeBackID), -1) != -1) {
				AfxMessageBox("This apply is part of a Chargeback and cannot be unapplied.");
				return;
			}

			if (IDNO == MessageBox("Are you sure you wish to unapply the highlighted item?", NULL, MB_YESNO))
				return;

			//CanUnapplyLineItem will check permissions, check against a financial close,
			//and potentially offer to correct a closed payment first.
			//If so, nApplyID will be changed.
			long nApplyID = VarLong(varApplyID);
			if (!CanUnapplyLineItem(nApplyID)) {
				return;
			}

			DeleteApply(nApplyID);
		}
		else {
			MsgBox("Please click on the item you wish to unapply before pressing this button.");
		}

		//m_List.RemoveAllHighlights();
		RefreshList();

	}NxCatchAll("Error unapplying item.");
}

void CApplyManagerDlg::OnDestroy() 
{
	try {
		// (d.thompson 2010-06-15) - PLID 39164 - Save the status of the applies vs payments selection
		BOOL bShowApplies = IsDlgButtonChecked(IDC_DISPLAY_APPLIES);
		SetRemotePropertyInt("ApplyManager_ShowApplies", bShowApplies ? 1 : 0, 0, GetCurrentUserName());

		CDialog::OnDestroy();
		
		GetMainFrame()->EnableHotKeys();
	} NxCatchAll(__FUNCTION__);
}

void CApplyManagerDlg::ChangeDisplay()
{
	if (m_ActiveTab == BILL_APPLY_TAB) {
		GetDlgItem(IDC_LABEL_BILLORPAYMENT)->SetWindowText("Bi&ll");
		GetDlgItem(IDC_LABEL_CHARGE)->SetWindowText("&Charge");
		GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(m_iBillID != -2);
		m_ComboCharges.EnableWindow(TRUE);
		// (d.thompson 2010-06-15) - PLID 39161 - Summary displays during bill tab
		ShowSummary(TRUE);
		// (d.thompson 2010-06-15) - PLID 39164 - Unapply not allowed on billing/show payments setup
		if(m_btnDisplayPayments.GetCheck()) {
			GetDlgItem(IDC_BTN_UNAPPLY)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_BTN_UNAPPLY)->EnableWindow(TRUE);
		}
	}
	else {
		// (d.thompson 2010-06-15) - PLID 39164 - Shortened text to give more real estate
		GetDlgItem(IDC_LABEL_BILLORPAYMENT)->SetWindowText("P&ay");
		GetDlgItem(IDC_LABEL_CHARGE)->SetWindowText("");

		GetDlgItem(IDC_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_UNAPPLY)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_APPLY_NEW)->EnableWindow(FALSE);
		//GetDlgItem(IDC_GROUP1)->ShowWindow(SW_HIDE);

		m_ComboCharges.EnableWindow(FALSE);
		// (d.thompson 2010-06-15) - PLID 39161 - Summary hides during payments tab
		ShowSummary(FALSE);
	}

	BuildComboBoxes();
	RefreshList();	
	// (d.thompson 2010-06-15) - PLID 39161 - No more list/summary differences, load the summary too
	RefreshSummary();
}

void CApplyManagerDlg::FillList() {

	CWaitCursor wait;

	m_List->SetRedraw(FALSE);

	try{
		m_List->Clear();
		for(int i=0; i<m_paryApplyManagerT.GetSize();i++) {
			IRowSettingsPtr pRow = m_List->GetRow(-1);
			// (j.jones 2011-10-27 16:25) - PLID 46159 - used proper defines here
			pRow->PutValue(amcApplyID,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->ApplyID);
			pRow->PutValue(amcSourcePayID,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->SourcePayID);
			pRow->PutValue(amcDate,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->Date);
			// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
			pRow->PutValue(amcPaymentTypeID, ((ApplyItem*)m_paryApplyManagerT.GetAt(i))->PaymentTypeID);
			pRow->PutValue(amcPaymentTypeName, ((ApplyItem*)m_paryApplyManagerT.GetAt(i))->PaymentTypeName);
			pRow->PutValue(amcChargeDesc,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->ChargeDesc);
			pRow->PutValue(amcPaymentDesc,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->PayDesc);
			pRow->PutValue(amcApplyAmount,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->Amount);
			pRow->PutValue(amcPrimaryAmount,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->Ins1Amount);
			pRow->PutValue(amcSecondaryAmount,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->Ins2Amount);
			pRow->PutValue(amcOtherInsAmount,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->Ins3Amount);			
			//TES 7/24/2014 - PLID 62557 - Added ChargebackID
			pRow->PutValue(amcChargeBackID,((ApplyItem*)m_paryApplyManagerT.GetAt(i))->ChargebackID);
			m_List->AddRow(pRow);
		}
	}NxCatchAll("Error in FillList()");

	m_List->SetRedraw(TRUE);
}

void CApplyManagerDlg::OnRButtonDownList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow == -1)
		return;

	m_iRow = nRow;

	CMenu pMenu;
	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, ID_SHOWAPPLY, "&Show Apply Source");
	// (d.thompson 2010-06-15) - PLID 39164 - If on the 'bills' tab and showing as payments, no unapplying allowed (except wholly)
	UINT nMenuFlags = MF_BYPOSITION;
	if(m_ActiveTab == BILL_APPLY_TAB && m_btnDisplayPayments.GetCheck()) {
		nMenuFlags |= MF_GRAYED;
	}
	pMenu.InsertMenu(1, nMenuFlags, ID_UNAPPLY, "&Unapply Item");
	pMenu.InsertMenu(2, MF_BYPOSITION, ID_UNAPPLY_ALL, "&Unapply Item Completely");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);	
}

BOOL CApplyManagerDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
	case ID_SHOWAPPLY: {

		// (j.jones 2011-10-27 16:30) - PLID 46159 - this is the source payment ID, I clarified the column define
		long nPayID = VarLong(m_List->GetValue(m_iRow, amcSourcePayID));
		long nType = VarLong(m_List->GetValue(m_iRow, amcPaymentTypeID), -1);
		if(nType == 1 && !CheckCurrentUserPermissions(bioPayment,sptRead))
			break;
		else if(nType == 3 && !CheckCurrentUserPermissions(bioRefund,sptRead))
			break;
		else if((nType == 2 || nType == 0) && !CheckCurrentUserPermissions(bioAdjustment,sptRead))
			break;

		CPaymentDlg dlg(this);
		dlg.m_varPaymentID = nPayID;
		dlg.DoModal(__FUNCTION__, __LINE__);
		}
		break;
	case ID_UNAPPLY:
		if(m_List->CurSel != m_iRow)
			m_List->PutCurSel(m_iRow);
		OnBtnUnapply();
		break;
	case ID_UNAPPLY_ALL: {

		long AuditID = -1;
		
		try {

		if(m_List->CurSel != m_iRow)
			m_List->PutCurSel(m_iRow);
		
		COleVariant varPayID;
		// (j.jones 2011-10-27 16:30) - PLID 46159 - this is the source payment ID, I clarified the column define
		varPayID = m_List->GetValue(m_List->GetCurSel(), amcSourcePayID);
		
		if (varPayID.vt == VT_I4) {

			long nPaymentID = VarLong(varPayID);

			// (j.jones 2011-09-13 15:35) - PLID 44887 - disallow on original/void items
			LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPaymentID);
			if(licsStatus == licsOriginal) {
				AfxMessageBox("This line item has been corrected, and can no longer be modified.");
				break;
			}
			else if(licsStatus == licsVoid) {
				AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
				break;
			}

			//TES 7/24/2014 - PLID 62557 - Prevent chargebacks from being unapplied
			if (VarLong(m_List->GetValue(m_List->CurSel, amcChargeBackID), -1) != -1) {
				AfxMessageBox("This apply is part of a Chargeback and cannot be unapplied.");
				break;
			}

			if (IDNO == MessageBox("Are you sure you wish to unapply the highlighted item completely?", NULL, MB_YESNO))
				break;

			//find out if the payment is closed, if so, we can try and void & correct
			bool bAutoCorrect = false;
			if (IsLineItemClosed(nPaymentID))
			{
				if (IDNO == MessageBox("This payment has been closed. Would you like to void & correct the payment prior to unapplying?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
					break;
				}

				bAutoCorrect = true;
			}

			//check apply permissions
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 AppliesT.ID "
				"FROM AppliesT "
				"WHERE SourceID = {INT} "
				"ORDER BY InputDate ASC", nPaymentID);
			if (!rs->eof) {
				//check permissions, but ignore the close if we are going to auto-correct
				if (!CanChangeHistoricFinancial("Apply", AdoFldLong(rs, "ID"), bioApplies, sptDelete, FALSE, NULL, bAutoCorrect ? TRUE : FALSE)) {
					break;
				}
			}
			rs->Close();
			
			if(bAutoCorrect) 
			{
				// (j.jones 2013-07-01 13:58) - PLID 55517 - try to correct the line item,
				// if it fails then they cannot 
				CString strSourceType = VarString(m_List->GetValue(m_List->GetCurSel(), amcPaymentTypeName));
				strSourceType.MakeLower();
				if (!VoidAndCorrectPayAdjRef(nPaymentID, "payment", true, true)) {
					//don't try to ask if they want to unapply anyways,
					//because they chose to correct, and it didn't work,
					//so abort
					break;
				}

				//remember nPaymentID has now changed
			}

			//save amount for Auditing
			//TES 7/10/2008 - PLID 30671 - We also need the source and dest IDs and Types
			CArray<long,long> arDestIDs;
			CArray<long,long> arDestTypes;
			CArray<COleCurrency,COleCurrency&> arDestAmounts;
			COleCurrency cyTotalAmount;
			long nSourceID = -1;
			long nSourceType = -1;
			//TES 7/10/2008 - PLID 30671 - Went ahead and parameterized while I was in the vicinity.
			// (j.jones 2011-01-19 14:43) - PLID 42149 - moved this recordset to be before the permission
			// check, and ordered it by oldest apply first
			// (j.jones 2011-09-14 15:48) - PLID 44792 - skip original & void lines
			rs = CreateParamRecordset("SELECT AppliesT.ID, SourceID, SourceItem.Type AS SourceType, DestID, "
				"DestItem.Type AS DestType, AppliesT.Amount "
				"FROM AppliesT "
				"INNER JOIN LineItemT SourceItem ON AppliesT.SourceID = SourceItem.ID "
				"INNER JOIN LineItemT DestItem ON AppliesT.DestID = DestItem.ID "
				"WHERE SourceID = {INT} "
				"ORDER BY AppliesT.InputDate", nPaymentID);
			while(!rs->eof) {
				if(nSourceID == -1) nSourceID = AdoFldLong(rs, "SourceID");
				if(nSourceType == -1) nSourceType = AdoFldLong(rs, "SourceType");
				arDestIDs.Add(AdoFldLong(rs, "DestID"));
				arDestTypes.Add(AdoFldLong(rs, "DestType"));
				COleCurrency cyAmount = AdoFldCurrency(rs, "Amount");
				cyTotalAmount += cyAmount;
				arDestAmounts.Add(cyAmount);
				rs->MoveNext();
			}
			rs->Close();

				//Delete from ApplyDetailsT
				//TES 7/10/2008 - PLID 30671 - Went ahead and parameterized while I was in the vicinity.
				ExecuteParamSql("DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID = {INT})", nPaymentID);
		
				//TES 7/10/2008 - PLID 30671 - Went ahead and parameterized while I was in the vicinity.
				ExecuteParamSql("DELETE FROM AppliesT WHERE SourceID = {INT}", nPaymentID);

				AuditID = BeginAuditTransaction();
				if(AuditID != -1) {
					//AuditEvent(GetActivePatientID(), GetActivePatientName(),AuditID,aeiApplyDeleted,varPayID.lVal,"",strAmount,aepHigh,aetDeleted);
					//TES 7/10/2008 - PLID 30671 - We now audit both the source and the dest that are being unapplied
					//Source
					AuditEventItems aeiSourceType;
					switch(nSourceType) {
					case 1:
						aeiSourceType = aeiPaymentUnapplied;
						break;
					case 2:
						aeiSourceType = aeiAdjustmentUnapplied;
						break;
					case 3:
						aeiSourceType = aeiRefundUnapplied;
						break;
					default:
						AfxThrowNxException("Bad Source Type %li found while auditing unapply!", nSourceType);
						break;
					}
					CString strTotalAmount = FormatCurrencyForInterface(cyTotalAmount, TRUE, TRUE);
					strTotalAmount.Replace("(", "");
					strTotalAmount.Replace(")", "");
					AuditEvent(GetActivePatientID(), GetActivePatientName(), AuditID, aeiSourceType, nSourceID, "", strTotalAmount + " Unapplied", aepHigh, aetDeleted);
					for(int i = 0; i < arDestIDs.GetSize(); i++) {
						long nDestType = arDestTypes[i];
						AuditEventItems aeiDestType;
						switch(nDestType) {
							case 10:
								aeiDestType = aeiItemUnappliedFromCharge;
								break;
							case 1:
								aeiDestType = aeiItemUnappliedFromPayment;
								break;
							case 2:
								aeiDestType = aeiItemUnappliedFromAdjustment;
								break;
							case 3:
								aeiDestType = aeiItemUnappliedFromRefund;
								break;
							default:
								AfxThrowNxException("Bad Dest Type %li found while auditing unapply!", nDestType);
							break;
						}
						CString strAmount = FormatCurrencyForInterface(arDestAmounts[i], TRUE, TRUE);
						strAmount.Replace("(", "");
						strAmount.Replace(")", "");
						AuditEvent(GetActivePatientID(), GetActivePatientName(), AuditID, aeiDestType, arDestIDs[i], "", "Applies reduced by " + strAmount, aepHigh, aetDeleted);
					}
				}
				CommitAuditTransaction(AuditID);
		}
		else {
			MsgBox("Please click on the item you wish to unapply before pressing this button.");
		}

		}NxCatchAllCall("Error unapplying item.", {if(AuditID != -1) RollbackAuditTransaction(AuditID);});

		RefreshList();
		break;
		}
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CApplyManagerDlg::OnSelectTab(short newTab, short oldTab) 
{
	switch (newTab)	
	{	
		case 0:
			m_ActiveTab = BILL_APPLY_TAB;
			// (d.thompson 2010-06-15) - PLID 39164 - Disable these in payments tab
			m_btnDisplayApplies.EnableWindow(TRUE);
			m_btnDisplayPayments.EnableWindow(TRUE);
			break;
		// (j.jones 2011-03-25 15:39) - PLID 41143 - added "Applies From Payments"
		case 1:
			m_ActiveTab = PAYMENT_APPLIED_FROM_TAB;
			m_btnDisplayApplies.EnableWindow(FALSE);
			m_btnDisplayPayments.EnableWindow(FALSE);
			break;
		case 2:
			m_ActiveTab = PAYMENT_APPLIED_TO_TAB;
			// (d.thompson 2010-06-15) - PLID 39164 - Disable these in payments tab
			m_btnDisplayApplies.EnableWindow(FALSE);
			m_btnDisplayPayments.EnableWindow(FALSE);
			break;
	}
	ChangeDisplay();
}

HBRUSH CApplyManagerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_LABEL11:
		case IDC_LABEL12:
		case IDC_LABEL13:
		case IDC_LABEL_TTL_PRIRESP:
		case IDC_LABEL_TTL_SECRESP:
		case IDC_LABEL_TTL_TERRESP:
		case IDC_LABEL_PRIRESP:
		case IDC_LABEL_SECRESP:
		case IDC_LABEL_TERRESP:
		case IDC_LABEL1:
		case IDC_LABEL_BALANCE:
			pDC->SetTextColor(0x000040);
			break;
		case IDC_LABEL8:
		case IDC_LABEL_PAYMENTS:
		case IDC_LABEL_PRI_PAY:
		case IDC_LABEL_SEC_PAY:
		case IDC_LABEL_TER_PAY:
			pDC->SetTextColor(0x285100);
			break;
		case IDC_LABEL3:
		case IDC_LABEL_ADJUSTMENTS:
		case IDC_LABEL_PRI_ADJ:
		case IDC_LABEL_SEC_ADJ:
		case IDC_LABEL_TER_ADJ:
			pDC->SetTextColor(0x553900);
			break;
		case IDC_LABEL4:
		case IDC_LABEL_REFUNDS:
		case IDC_LABEL_PRI_REF:
		case IDC_LABEL_SEC_REF:
		case IDC_LABEL_TER_REF:
			pDC->SetTextColor(5050);
			break;
		default:
			break;
	}
	
	return hbr;
}

// (j.jones 2008-05-02 08:43) - PLID 27305 - properly handled deleting from the array
void CApplyManagerDlg::ClearApplyArray()
{
	try {

		for(int i=m_paryApplyManagerT.GetSize()-1;i>=0;i--) {
			delete (ApplyItem*)(m_paryApplyManagerT.GetAt(i));
		}
		m_paryApplyManagerT.RemoveAll();

	}NxCatchAll("Error in CApplyManagerDlg::ClearApplyArray");
}

// (d.thompson 2010-06-15) - PLID 39164
void CApplyManagerDlg::OnBnClickedDisplayApplies()
{
	try {
		ChangeDisplay();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-06-15) - PLID 39164
void CApplyManagerDlg::OnBnClickedDisplayPayments()
{
	try {
		ChangeDisplay();

	} NxCatchAll(__FUNCTION__);
}
