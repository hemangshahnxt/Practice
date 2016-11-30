// OpportunityProposalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "OpportunityProposalDlg.h"
#include "InternationalUtils.h"
#include "MergeEngine.h"
#include "LetterWriting.h"
#include "SelectSenderDlg.h"
#include "OpportunityAddDiscountDlg.h"
#include "PracticeRc.h"
#include "ProposalPricing.h"			// (d.lange 2010-4-1) PLID 38016

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

//DRT 5/27/2008 - PLID 29493 - Resident & Startup specific maximum months
#define PKG_SPT_MONTHS	9

enum eDiscountColumn {
	edcID = 0,
	edcChanged,
	edcUserID,
	edcDate,
	edcPercent,
	edcDollar,
};

//Returns number of days from today to the end of the quarter
//	dtToday parameter allows you to specify what date to start from.
long CalculateDaysToEndOfQuarter(COleDateTime dtToday)
{
	//This month
	long nMonth = dtToday.GetMonth();

	//date for EOQ
	COleDateTime dtEOQ;

	switch(nMonth) {
	case 1:
	case 2:
	case 3:
		//Q1 = 3/31/YYYY
		dtEOQ.SetDate(dtToday.GetYear(), 3, 31);
		break;
	case 4:
	case 5:
	case 6:
		//Q2 = 6/30/YYYY
		dtEOQ.SetDate(dtToday.GetYear(), 6, 30);
		break;
	case 7:
	case 8:
	case 9:
		//Q3 = 9/30/YYYY
		dtEOQ.SetDate(dtToday.GetYear(), 9, 30);
		break;
	case 10:
	case 11:
	case 12:
		//Q4 = 12/31/YYYY
		dtEOQ.SetDate(dtToday.GetYear(), 12, 31);
		break;
	}


	return dtEOQ.GetDayOfYear() - dtToday.GetDayOfYear();
}

/////////////////////////////////////////////////////////////////////////////
// COpportunityProposalDlg dialog

//This is only kept in this dialog, no need to take it outside
#define	NXM_LOAD_OPPORTUNITY_PROPOSAL		WM_USER + 10002

COpportunityProposalDlg::COpportunityProposalDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COpportunityProposalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpportunityProposalDlg)
	//}}AFX_DATA_INIT

	m_nID = -1;
	m_nOpportunityID = -1;
	m_nMailSentID = -1;
	m_nPriceStructureID = -1;
	m_nPatientID = -1;
	m_nExistingLoadFromID = -1;
	bRequestImmediateLoad = false;
	m_nCurrentDiscountUserID = -1;
	m_nTypeID = -1;
	m_nResidentAddOnDiscount = m_nStartupAddOnDiscount = 0;
	m_bSplitMerge = false;
	m_nMergeCount = 0;
	m_nTravelType = ettDomestic;
}

void COpportunityProposalDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpportunityProposalDlg)
	DDX_Control(pDX, IDC_CHK_UNITED, m_btnUnited);
	DDX_Control(pDX, IDC_CHK_TRACKING, m_btnTracking);
	DDX_Control(pDX, IDC_CHK_NEXPHOTO, m_btnNexPhoto);
	DDX_Control(pDX, IDC_CHK_SCHED, m_btnSched);
	DDX_Control(pDX, IDC_CHK_QUOTES, m_btnQuotes);
	DDX_Control(pDX, IDC_CHK_QUICKBOOKS, m_btnQBooks);
	DDX_Control(pDX, IDC_CHK_NEXSPA, m_btnNexSpa);
	DDX_Control(pDX, IDC_CHK_NEXFORMS, m_btnNexForms);
	DDX_Control(pDX, IDC_CHK_MIRROR, m_btnMirror);
	DDX_Control(pDX, IDC_CHK_LETTERS, m_btnLetters);
	DDX_Control(pDX, IDC_CHK_INV, m_btnInv);
	DDX_Control(pDX, IDC_CHK_INFORM, m_btnInform);
	DDX_Control(pDX, IDC_CHK_HL7, m_btnHL7);
	DDX_Control(pDX, IDC_CHK_HIE, m_btnHIE);
	DDX_Control(pDX, IDC_CHK_HCFA, m_btnHCFA);
	DDX_Control(pDX, IDC_CHK_EREMITTANCE, m_btnERemit);
	DDX_Control(pDX, IDC_CHK_EELIGIBILITY, m_btnEElig);
	DDX_Control(pDX, IDC_CHK_EBILLING, m_btnEBilling);
	DDX_Control(pDX, IDC_CHK_BILLING, m_btnBilling);
	DDX_Control(pDX, IDC_CHK_ASC, m_btnASC);
	DDX_Control(pDX, IDC_CHK_1LICENSE, m_btn1License);
	DDX_Control(pDX, IDC_CHK_SCHED_STANDARD, m_btnStdScheduler);
	DDX_Control(pDX, IDC_CHK_EMR_STANDARD, m_btnStdEMR);
	DDX_Control(pDX, IDC_CHK_EMR_COSMETIC, m_btnCosmeticEMR);
	DDX_Control(pDX, IDC_CHK_C_AND_A, m_btnCandA);
	DDX_Control(pDX, IDC_CHECK_ALL_COSMETIC, m_btnCheckAllCosmetic);
	DDX_Control(pDX, IDC_CHECK_ALL_FINANCIAL, m_btnCheckAllFinancial);
	DDX_Control(pDX, IDC_LABEL_SUPPORT, m_labelSupportMonthly);
	DDX_Control(pDX, IDC_SPECIAL_DISCOUNT_ADDON, m_labelSpecialDiscountAddOn);
	DDX_Control(pDX, IDC_CHANGE_DISCOUNT, m_btnChangeDiscount);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_MERGE_PROPOSAL, m_btnMerge);
	DDX_Control(pDX, IDC_QUOTE_ID, m_nxeditQuoteId);
	DDX_Control(pDX, IDC_CREATED_BY, m_nxeditCreatedBy);
	DDX_Control(pDX, IDC_PKG_SCHED, m_nxeditPkgSched);
	DDX_Control(pDX, IDC_SEL_SCHED, m_nxeditSelSched);
	DDX_Control(pDX, IDC_PKG_FINANCIAL, m_nxeditPkgFinancial);
	DDX_Control(pDX, IDC_SEL_FINANCIAL, m_nxeditSelFinancial);
	DDX_Control(pDX, IDC_PKG_COSMETIC, m_nxeditPkgCosmetic);
	DDX_Control(pDX, IDC_SEL_COSMETIC, m_nxeditSelCosmetic);
	DDX_Control(pDX, IDC_SEL_OTHER, m_nxeditSelOther);
	DDX_Control(pDX, IDC_NUM_WORKSTATIONS, m_nxeditNumWorkstations);
	DDX_Control(pDX, IDC_NUM_WORKSTATIONS_TS, m_nxeditNumWorkstationsTS);
	DDX_Control(pDX, IDC_NUM_EMRWORKSTATIONS, m_nxeditNumEMRWorkstations);		// (d.lange 2010-03-30 - PLID 37956 - EMR Workstations Edit Box
	DDX_Control(pDX, IDC_SEL_WORKSTATIONS, m_nxeditSelWorkstations);
	DDX_Control(pDX, IDC_SEL_WORKSTATIONS_TS, m_nxeditSelWorkstationsTS);
	DDX_Control(pDX, IDC_SEL_EMRWORKSTATIONS, m_nxeditSelEMRWorkstations);		// (d.lange 2010-03-30 - PLID 37956 - EMR Workstations Edit Box
	DDX_Control(pDX, IDC_NUM_DOCTORS, m_nxeditNumDoctors);
	DDX_Control(pDX, IDC_SEL_DOCTORS, m_nxeditSelDoctors);
	DDX_Control(pDX, IDC_NUM_PDA, m_nxeditNumPda);
	DDX_Control(pDX, IDC_SEL_PDA, m_nxeditSelPda);
	DDX_Control(pDX, IDC_NUM_NEXSYNC, m_nxeditNumNexSync);
	DDX_Control(pDX, IDC_SEL_NEXSYNC, m_nxeditSelNexSync);
	DDX_Control(pDX, IDC_SEL_MIRROR, m_nxeditSelMirror);
	DDX_Control(pDX, IDC_SEL_UNITED, m_nxeditSelUnited);
	DDX_Control(pDX, IDC_SEL_HL7, m_nxeditSelHl7);
	DDX_Control(pDX, IDC_SEL_HIE, m_nxeditSelHIE);
	DDX_Control(pDX, IDC_SEL_INFORM, m_nxeditSelInform);
	DDX_Control(pDX, IDC_SEL_QUICKBOOKS, m_nxeditSelQuickbooks);
	DDX_Control(pDX, IDC_NUM_EMR, m_nxeditNumEmr);
	DDX_Control(pDX, IDC_SEL_EMR, m_nxeditSelEmr);
	DDX_Control(pDX, IDC_SUBTOTAL_LIST, m_nxeditSubtotalList);
	DDX_Control(pDX, IDC_SUBTOTAL_PKG_DISCOUNTS, m_nxeditSubtotalPkgDiscounts);
	DDX_Control(pDX, IDC_SUBTOTAL_SEL, m_nxeditSubtotalSel);
	DDX_Control(pDX, IDC_DISCOUNT_USERNAME, m_nxeditDiscountUsername);
	DDX_Control(pDX, IDC_DISCOUNT_TOTAL_PERCENT, m_nxeditDiscountTotalPercent);
	DDX_Control(pDX, IDC_DISCOUNT_TOTAL_DOLLAR, m_nxeditDiscountTotalDollar);
	DDX_Control(pDX, IDC_NUM_TRAINING, m_nxeditNumTraining);
	DDX_Control(pDX, IDC_NUM_EMR_TRAINING, m_nxeditNumEmrTraining);
	DDX_Control(pDX, IDC_SEL_TRAINING, m_nxeditSelTraining);
	DDX_Control(pDX, IDC_SUPPORT_PERCENT, m_nxeditSupportPercent);
	DDX_Control(pDX, IDC_NUM_SUPPORT, m_nxeditNumSupport);
	DDX_Control(pDX, IDC_SEL_SUPPORT, m_nxeditSelSupport);
	DDX_Control(pDX, IDC_NUM_CONVERSION, m_nxeditNumConversion);
	DDX_Control(pDX, IDC_SEL_CONVERSION, m_nxeditSelConversion);
	DDX_Control(pDX, IDC_NUM_TRAVEL, m_nxeditNumTravel);
	DDX_Control(pDX, IDC_SEL_TRAVEL, m_nxeditSelTravel);
	DDX_Control(pDX, IDC_NUM_EXTRADB, m_nxeditNumExtradb);
	DDX_Control(pDX, IDC_SEL_EXTRADB, m_nxeditSelExtradb);
	DDX_Control(pDX, IDC_GRAND_TOTAL, m_nxeditGrandTotal);
	DDX_Control(pDX, IDC_LABEL_SCHED, m_nxstaticLabelSched);
	DDX_Control(pDX, IDC_LABEL_1LICENSE, m_nxstaticLabel1License);
	DDX_Control(pDX, IDC_LABEL_BILLING, m_nxstaticLabelBilling);
	DDX_Control(pDX, IDC_LABEL_HCFA, m_nxstaticLabelHcfa);
	DDX_Control(pDX, IDC_LABEL_EBILLING, m_nxstaticLabelEbilling);
	DDX_Control(pDX, IDC_LABEL_LETTERS, m_nxstaticLabelLetters);
	DDX_Control(pDX, IDC_LABEL_QUOTES, m_nxstaticLabelQuotes);
	DDX_Control(pDX, IDC_LABEL_TRACKING, m_nxstaticLabelTracking);
	DDX_Control(pDX, IDC_LABEL_NEXFORMS, m_nxstaticLabelNexforms);
	DDX_Control(pDX, IDC_LABEL_INV, m_nxstaticLabelInv);
	DDX_Control(pDX, IDC_LABEL_SPA, m_nxstaticLabelSpa);
	DDX_Control(pDX, IDC_LABEL_ASC, m_nxstaticLabelAsc);
	DDX_Control(pDX, IDC_LABEL_EREMITTANCE, m_nxstaticLabelEremittance);
	DDX_Control(pDX, IDC_LABEL_EELIGIBILITY, m_nxstaticLabelEeligibility);
	DDX_Control(pDX, IDC_LABEL_WORKSTATIONS, m_nxstaticLabelWorkstations);
	DDX_Control(pDX, IDC_LABEL_WORKSTATIONS_TS, m_nxstaticLabelWorkstationsTS);
	DDX_Control(pDX, IDC_LABEL_EMRWORKSTATIONS, m_nxstaticLabelEMRWorkstations);	// (d.lange 2010-03-30) - PLID 37956 - EMR Workstations Label
	DDX_Control(pDX, IDC_CHK_DISCOUNT_EMR, m_btnApplyDiscountSplit);		// (d.lange 2010-04-08) - PLID 38096 - Added checkbox for applying discount to both PM & EMR merge
	DDX_Control(pDX, IDC_LABEL_DOCTORS, m_nxstaticLabelDoctors);
	DDX_Control(pDX, IDC_LABEL_PDA, m_nxstaticLabelPda);
	DDX_Control(pDX, IDC_LABEL_NEXSYNC, m_nxstaticLabelNexSync);
	DDX_Control(pDX, IDC_LABEL_MIRROR, m_nxstaticLabelMirror);
	DDX_Control(pDX, IDC_LABEL_UNITED, m_nxstaticLabelUnited);
	DDX_Control(pDX, IDC_LABEL_HL7, m_nxstaticLabelHl7);
	DDX_Control(pDX, IDC_LABEL_HIE, m_nxstaticLabelHIE);
	DDX_Control(pDX, IDC_LABEL_INFORM, m_nxstaticLabelInform);
	DDX_Control(pDX, IDC_LABEL_QUICKBOOKS, m_nxstaticLabelQuickbooks);
	DDX_Control(pDX, IDC_LABEL_EMR, m_nxstaticLabelEmr);
	DDX_Control(pDX, IDC_LABEL_TRAINING, m_nxstaticLabelTraining);
	DDX_Control(pDX, IDC_SPECIAL_DISCOUNT_PERCENT, m_nxstaticSpecialDiscountPercent);
	DDX_Control(pDX, IDC_SPECIAL_DISCOUNT_DOLLAR, m_nxstaticSpecialDiscountDollar);
	DDX_Control(pDX, IDC_LABEL_C_AND_A, m_nxstaticCandA);
	DDX_Control(pDX, IDC_PROPOSAL_DATE, m_pickerProposalDate);
	DDX_Control(pDX, IDC_PROPOSAL_EXPIRES, m_pickerExpireDate);
	DDX_Control(pDX, IDC_CHK_NEXWEB_LEADS, m_btnNexWebLeads);
	DDX_Control(pDX, IDC_CHK_NEXWEB_PORTAL, m_btnNexWebPortal);
	DDX_Control(pDX, IDC_CHK_SPLITEMR, m_btnEMRSplit);			// (d.lange 2010-03-30 - PLID 37956 - EMR Split checkbox 
	DDX_Control(pDX, IDC_NUM_NEXWEB_ADDTL_DOMAINS, m_editNexWebAddtlDomains);
	DDX_Control(pDX, IDC_LABEL_NEXWEB_LEADS, m_nxstaticNexWebLeads);
	DDX_Control(pDX, IDC_LABEL_NEXWEB_PORTAL, m_nxstaticNexWebPortal);
	DDX_Control(pDX, IDC_LABEL_NEXWEB_ADDTL_DOMAINS, m_nxstaticNexWebAddtlDomains);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COpportunityProposalDlg, CNxDialog)
	//{{AFX_MSG_MAP(COpportunityProposalDlg)
	ON_BN_CLICKED(IDC_CHK_1LICENSE, OnChk1license)
	ON_BN_CLICKED(IDC_CHK_ASC, OnChkAsc)
	ON_BN_CLICKED(IDC_CHK_BILLING, OnChkBilling)
	ON_BN_CLICKED(IDC_CHK_EBILLING, OnChkEbilling)
	ON_BN_CLICKED(IDC_CHK_HCFA, OnChkHcfa)
	ON_BN_CLICKED(IDC_CHK_INV, OnChkInv)
	ON_BN_CLICKED(IDC_CHK_EREMITTANCE, OnChkEremittance)
	ON_BN_CLICKED(IDC_CHK_EELIGIBILITY, OnChkEeligibility)
	ON_BN_CLICKED(IDC_CHK_LETTERS, OnChkLetters)
	ON_BN_CLICKED(IDC_CHK_MIRROR, OnChkMirror)
	ON_BN_CLICKED(IDC_CHK_NEXFORMS, OnChkNexforms)
	ON_BN_CLICKED(IDC_CHK_NEXSPA, OnChkNexspa)
	ON_BN_CLICKED(IDC_CHK_QUOTES, OnChkQuotes)
	ON_BN_CLICKED(IDC_CHK_SCHED, OnChkSched)
	ON_BN_CLICKED(IDC_CHK_TRACKING, OnChkTracking)
	ON_BN_CLICKED(IDC_CHK_UNITED, OnChkUnited)
	ON_EN_CHANGE(IDC_NUM_CONVERSION, OnChangeNumConversion)
	ON_EN_CHANGE(IDC_NUM_EMRCONVERSION, OnChangeNumEmrConversion)
	ON_EN_CHANGE(IDC_NUM_FINCONVERSION, OnChangeNumFinancialConversion)
	ON_EN_CHANGE(IDC_NUM_DOCTORS, OnChangeNumDoctors)
	ON_EN_CHANGE(IDC_NUM_EMR, OnChangeNumEmr)
	ON_EN_CHANGE(IDC_NUM_PDA, OnChangeNumPda)
	ON_EN_CHANGE(IDC_NUM_SUPPORT, OnChangeNumSupport)
	ON_EN_CHANGE(IDC_NUM_TRAINING, OnChangeNumTraining)
	ON_EN_CHANGE(IDC_NUM_EMR_TRAINING, OnChangeNumEmrTraining)
	ON_EN_CHANGE(IDC_NUM_TRAVEL, OnChangeNumTravel)
	ON_EN_CHANGE(IDC_NUM_EXTRADB, OnChangeExtraDB)
	ON_EN_CHANGE(IDC_NUM_WORKSTATIONS, OnChangeNumWorkstations)
	ON_EN_CHANGE(IDC_NUM_WORKSTATIONS_TS, OnChangeNumWorkstationsTS)
	// (d.lange 2010-03-30 - PLID 37956 - EMR Workstations
	ON_EN_CHANGE(IDC_NUM_EMRWORKSTATIONS, OnChangeNumEmrWorkstations)			
	ON_BN_CLICKED(IDC_MERGE_PROPOSAL, OnMergeProposal)
	ON_MESSAGE(NXM_LOAD_OPPORTUNITY_PROPOSAL, OnLoadOpportunityProposal)
	ON_BN_CLICKED(IDC_CHANGE_DISCOUNT, OnChangeDiscount)
	ON_BN_CLICKED(IDC_CHECK_ALL_FINANCIAL, OnCheckAllFinancial)
	ON_BN_CLICKED(IDC_CHECK_ALL_COSMETIC, OnCheckAllCosmetic)
	ON_BN_CLICKED(IDC_CHK_HL7, OnChkHl7)
	ON_BN_CLICKED(IDC_CHK_HIE, OnChkHIE)
	ON_BN_CLICKED(IDC_CHK_INFORM, OnChkInform)
	ON_BN_CLICKED(IDC_CHK_QUICKBOOKS, OnChkQuickbooks)
	ON_BN_CLICKED(IDC_CHK_SCHED_STANDARD, OnChkSchedStandard)
	ON_BN_CLICKED(IDC_CHK_EMR_STANDARD, OnChkEmrStandard)
	ON_BN_CLICKED(IDC_CHK_EMR_COSMETIC, OnChkEmrCosmetic)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHK_C_AND_A, &COpportunityProposalDlg::OnBnClickedChkCAndA)
	ON_BN_CLICKED(IDC_CHK_NEXWEB_LEADS, &COpportunityProposalDlg::OnBnClickedChkNexwebLeads)
	ON_BN_CLICKED(IDC_CHK_NEXWEB_PORTAL, &COpportunityProposalDlg::OnBnClickedChkNexwebPortal)
	// (d.lange 2010-03-30) - PLID 37956 - EMR Split checkbox 
	ON_BN_CLICKED(IDC_CHK_SPLITEMR, &COpportunityProposalDlg::OnBnClickedChkEMRSplit)
	// (d.lange 2010-04-08) - PLID 38096 - Added 'Apply discount to both merges' checkbox
	ON_BN_CLICKED(IDC_CHK_DISCOUNT_EMR, OnBnClickedChkDiscountEMR)
	ON_EN_CHANGE(IDC_NUM_NEXWEB_ADDTL_DOMAINS, &COpportunityProposalDlg::OnEnChangeNumNexwebAddtlDomains)
	ON_BN_CLICKED(IDC_CHK_NEXPHOTO, &COpportunityProposalDlg::OnBnClickedChkNexphoto)
	ON_EN_CHANGE(IDC_NUM_NEXSYNC, &COpportunityProposalDlg::OnEnChangeNumNexsync)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpportunityProposalDlg message handlers

BOOL COpportunityProposalDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (d.thompson 2012-04-05) - PLID 49453 - Bulk cache new properties (did not fix old)
		g_propManager.CachePropertiesInBulk("InternalPropTextProps", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'InternalOpp_DiscountDesc' "
				")",
				_Q(GetCurrentUserName()));


		//Colorize, "prettify" things
		DWORD dwColor = GetRemotePropertyInt("InternalPropBGColor", 10542240, 0, "<None>", false);

		//Note:  I made a bunch of these and ended up not using #1, so it's gone
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR1))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR2))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR3))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR4))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR5))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR6))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR7))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR8))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR9))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR10))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR11))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR12))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR13))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR14))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_PROPOSAL_COLOR15))->SetColor(dwColor);

		m_labelSupportMonthly.SetColor(dwColor);
		m_labelSpecialDiscountAddOn.SetColor(dwColor);
		m_labelSpecialDiscountAddOn.SetHzAlign(DT_RIGHT);
		m_labelSpecialDiscountAddOn.SetText(FormatCurrencyForInterface(COleCurrency(0, 0)));

		//NxIconButton setup
		m_btnChangeDiscount.AutoSet(NXB_MODIFY);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnMerge.AutoSet(NXB_MERGE);

		if(!m_strPatientName.IsEmpty()) {
			SetWindowText("Proposal - " + m_strPatientName);
		}

		//If we were not given an opportunity ID, we must fail
		if(m_nOpportunityID == -1) {
			MessageBox("Failed to obtain opportunity ID number, proposal creation failed.");
			CDialog::OnCancel();
			return TRUE;
		}

		//DRT 3/31/2008 - PLID 29493 - Ensure we've got a type as well
		if(m_nTypeID == -1) {
			MessageBox("Failed to obtain a proposal type, proposal creation failed.");
			CDialog::OnCancel();
			return TRUE;
		}

		// (d.lange 2010-3-31) - PLID 37956 - disable Split EMR checkbox if the proposal type is not new sale
		if(m_nTypeID != eptNewSale) {
			(GetDlgItem(IDC_CHK_SPLITEMR))->EnableWindow(FALSE);
		}

		//Do this first to figure out what we are charging.  These prices may change.
		FillPriceArray();

		//Reflect the pricing on the screen
		ReflectPriceArray();

		// (d.lange 2010-04-02) - PLID 38016 - Clear the array then assign enough space
		m_aryModules.RemoveAll();
		m_aryModules.SetSize(ecmTotalModules);
		
		if(!bRequestImmediateLoad) {
			//Post a message to start the load, this gives the appearance of speed by letting the dialog draw
			//	before the loading halts message pumping again.
			PostMessage(NXM_LOAD_OPPORTUNITY_PROPOSAL, 0, 0);
		}
		else {
			OnLoadOpportunityProposal(0, 0);
		}
	
		// (d.lange 2010-04-08) - PLID 38096 - Hide checkbox until 'Split EMR' is selected
		(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->ShowWindow(FALSE);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Use these to launch the dialog, not a separate call to DoModal().
// (d.thompson 2009-08-27) - PLID 35365 - Added financing data, which is saved outside the proposal
int COpportunityProposalDlg::OpenNewProposal(long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID, bool bIsFinancing)
{
	//Just call the main one with a -1 current ID
	return OpenProposal(-1, nOpportunityID, nPatientID, strPatientName, nTypeID, bIsFinancing);
}

// (d.thompson 2009-08-27) - PLID 35365 - Added financing data, which is saved outside the proposal
int COpportunityProposalDlg::OpenProposal(long nProposalID, long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID, bool bIsFinancing)
{
	m_nID = nProposalID;
	m_nOpportunityID = nOpportunityID;
	m_nPatientID = nPatientID;
	m_strPatientName = strPatientName;
	m_nTypeID = nTypeID;
	m_bIsFinancing = bIsFinancing;

	return DoModal();
}

// (d.thompson 2009-08-27) - PLID 35365 - Added financing data, which is saved outside the proposal
int COpportunityProposalDlg::OpenFromExisting(long nOpportunityID, long nPatientID, long nExistingProposalID, CString strPatientName, long nTypeID, bool bIsFinancing)
{
	//Set the existing load from
	m_nExistingLoadFromID = nExistingProposalID;

	//Just call the default for the rest
	return OpenProposal(-1, nOpportunityID, nPatientID, strPatientName, nTypeID, bIsFinancing);
}

LRESULT COpportunityProposalDlg::OnLoadOpportunityProposal(WPARAM wParam, LPARAM lParam)
{
	try {
		//TODO:  We need to figure out what to do with support % for international clients.  Most recent suggestion was
		//	to flag them and double it.
		SetDlgItemInt(IDC_SUPPORT_PERCENT, 16);


		//
		//Variables for loading - Depending how you are loading, set these variables appropriately.  At the end of the function, we
		//	then transfer them to the interface.  All are left uninitialized, you must set them to a default value.
		COleDateTime dtPropDate, dtExpireDate;
		long nQuoteNum;
		CString strUsername, strDiscountUser;
		BOOL bSched, b1License, bBilling, bHCFA, bEBilling, bLetters, bQuotes, bTracking, bNexForms, bInventory, bNexSpa;
		BOOL bNexASC, bMirror, bUnited, bHL7, bInform, bQuickbooks;
		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		BOOL bHIE;
		//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
		BOOL bERemittance, bEEligibility;
		// (d.thompson 2009-11-13) - PLID 36124 - Added NexSync
		// (d.lange 2010-3-31) - PLID 37956 - Added EMR Workstations
		long nWorkstations, nEMRWorkstations, nDoctors, nPDA, nNexSync, nEMR, nTraining, nEmrTraining, nSupport, nConversion, nTravel;
		// (d.lange 2010-11-11 15:54) - PLID 40361 - Added EMR Conversion and Finanical Conversion
		long nEMRConversion, nFinancialConversion;
		//DRT 2/13/2008 - PLID 28905 - Added multi DB
		long nExtraDB;
		COleCurrency cyDiscountAmount(0, 0);
		//DRT 4/3/2008 - PLID 29493 - For special packages
		long nSpecialAddOnPct = 0;
		COleCurrency cySpecialDiscount(0, 0);
		//DRT 9/18/2008 - PLID 31395 - For 'Standard' edition
		//d.thompson 2008-12-02 - PLID 32143 - We removed LW Standard Edition.
		//	It was never needed and no features were ever developed for it.  I just
		//	removed all the code, did not document each place I cleaned up.  There are
		//	a few outstanding proposals with the price listed for it (mostly tests), which will look
		//	odd if loaded, but otherwise can be ignored.  All data structure will remain intact 
		//	even though it is no longer in use.
		BOOL bIsStandardSched = FALSE, bIsStandardEMR = FALSE;
		// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic emr
		BOOL bIsCosmeticEMR = FALSE;
		// (d.thompson 2009-02-02) - PLID 32890 - Added C&A
		BOOL bCandA;
		// (d.thompson 2009-08-10) - PLID 35152 - Added NexWeb
		BOOL bNexWebLeads, bNexWebPortal;
		// (d.thompson 2009-11-06) - PLID 36123 - Added NexPhoto
		BOOL bNexPhoto;
		// (d.lange 2010-3-31) - PLID 37956 - variable for setting the Split EMR checkbox
		BOOL bSplitEMR;
		// (d.lange 2010-04-08) - PLID 38096 - variable for setting the apply discount to emr merge checkbox
		BOOL bEMRDiscount;
		long nNexWebAddtlDomains = 0;
		// (d.thompson 2012-10-12) - PLID 53155 - TS workstations
		long nWorkstationsTS = 0;
		//

		//Perform the loading.
		if(m_nID != -1) {
			//We are loading an existing proposal
			_RecordsetPtr prsLoad = CreateRecordset("SELECT OpportunityProposalsT.*, UsersT.Username, "
				"DiscountUsersT.Username AS DiscountedByUser, DiscountUsersT.PersonID AS DiscountedByUserID "
				"FROM OpportunityProposalsT LEFT JOIN UsersT ON OpportunityProposalsT.CreatedByUserID = UsersT.PersonID "
				"LEFT JOIN UsersT DiscountUsersT ON OpportunityProposalsT.DiscountedBy = DiscountUsersT.PersonID "
				"WHERE ID = %li", m_nID);
			if(prsLoad->eof) {
				//This should be impossible
				AfxThrowNxException("Attempted to load invalid proposal ID %li.", m_nID);
			}

			//Load the "general" fields
			FieldsPtr pFlds = prsLoad->Fields;
			dtPropDate = VarDateTime(pFlds->Item["ProposalDate"]->Value);
			dtExpireDate = VarDateTime(pFlds->Item["ExpiresOn"]->Value);
			nQuoteNum = AdoFldLong(pFlds, "QuoteNum");
			strUsername = AdoFldString(pFlds, "Username");

			//Load all the selected values
			bSched = AdoFldBool(pFlds, "Scheduler");
			b1License = AdoFldBool(pFlds, "LicenseSched");
			bBilling = AdoFldBool(pFlds, "Billing");
			bHCFA = AdoFldBool(pFlds, "HCFA");
			bEBilling = AdoFldBool(pFlds, "EBilling");
			bLetters = AdoFldBool(pFlds, "Letters");
			bQuotes = AdoFldBool(pFlds, "Quotes");
			bTracking = AdoFldBool(pFlds, "Tracking");
			bNexForms = AdoFldBool(pFlds, "NexForms");
			bInventory = AdoFldBool(pFlds, "Inventory");
			bNexSpa = AdoFldBool(pFlds, "NexSpa");
			bNexASC = AdoFldBool(pFlds, "NexASC");
			bMirror = AdoFldBool(pFlds, "Mirror");
			bUnited = AdoFldBool(pFlds, "United");
			bHL7 = AdoFldBool(pFlds, "HL7");
			// (d.thompson 2013-07-08) - PLID 57334 - I made this an int in the database as I think we'll eventually support multiple.  Convert to BOOL for now.
			bHIE = AdoFldLong(pFlds, "HIELink");
			bInform = AdoFldBool(pFlds, "Inform");
			bQuickbooks = AdoFldBool(pFlds, "Quickbooks");
			//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
			bERemittance= AdoFldBool(pFlds, "ERemittance");
			bEEligibility = AdoFldBool(pFlds, "EEligibility");
			// (d.thompson 2009-02-02) - PLID 32890 - C&A
			bCandA = AdoFldBool(pFlds, "CandA");
			// (d.thompson 2009-08-10) - PLID 35152
			bNexWebLeads = AdoFldBool(pFlds, "NexWebLeads");
			bNexWebPortal = AdoFldBool(pFlds, "NexWebPortal");
			nNexWebAddtlDomains = AdoFldLong(pFlds, "NexWebAddtlDomains");
			// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
			bNexPhoto = AdoFldBool(pFlds, "NexPhoto");
			// (d.lange 2010-3-31) - PLID 37956 - grab the value for the Split EMR checkbox
			bSplitEMR = AdoFldBool(pFlds, "SplitEMR");
			// (d.lange 2010-04-08) - PLID 38096 - variable for setting the apply discount to emr merge checkbox
			bEMRDiscount = AdoFldBool(pFlds, "ApplyEMRDiscount");

			nWorkstations = AdoFldLong(pFlds, "Workstations");
			// (d.thompson 2012-10-12) - PLID 53155
			nWorkstationsTS = AdoFldLong(pFlds, "WorkstationsTS");
			// (d.lange 2010-3-31) - PLID 37956 - add EMR Workstations
			nEMRWorkstations = AdoFldLong(pFlds, "EMRWorkstations");
			nDoctors = AdoFldLong(pFlds, "Doctors");
			nPDA = AdoFldLong(pFlds, "PDA");
			nNexSync = AdoFldLong(pFlds, "NexSync");
			nEMR = AdoFldLong(pFlds, "EMR");
			nTraining = AdoFldLong(pFlds, "Training");
			nEmrTraining = AdoFldLong(pFlds, "EmrTraining"); // (z.manning, 11/26/2007) - PLID 28159
			nSupport = AdoFldLong(pFlds, "Support");
			nConversion = AdoFldLong(pFlds, "Conversion");
			nTravel = AdoFldLong(pFlds, "Travel");
			//DRT 2/13/2008 - PLID 28905 - Added multi DB
			nExtraDB = AdoFldLong(pFlds, "ExtraDB");
			nEMRConversion = AdoFldLong(pFlds, "EMRConversion");
			nFinancialConversion = AdoFldLong(pFlds, "FinancialConversion");
			// (d.lange 2010-12-21 14:29) - PLID 41889 - Added Travel Type
			m_nTravelType = AdoFldLong(pFlds, "TravelType");

			//DRT 4/3/2008 - PLID 29493 - We don't want to load the discount amount if this is a 'special' package.
			if(m_nTypeID != eptNexRes && m_nTypeID != eptNexStartup) {
				cyDiscountAmount = AdoFldCurrency(pFlds, "DiscountAmount");
				strDiscountUser = AdoFldString(pFlds, "DiscountedByUser", "");
				m_nCurrentDiscountUserID = AdoFldLong(pFlds, "DiscountedByUserID", -1);
			}

			//DRT 9/18/2008 - PLID 31395 - For 'Standard' edition
			bIsStandardSched = AdoFldBool(pFlds, "IsSchedStandard", FALSE);
			bIsStandardEMR = AdoFldBool(pFlds, "IsEMRStandard", FALSE);
			// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic
			bIsCosmeticEMR = AdoFldBool(pFlds, "IsEMRCosmetic", FALSE);

			m_nMailSentID = AdoFldLong(pFlds, "MailSentID", -1);

			if(bSplitEMR) {
				(GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->EnableWindow(TRUE);
				// (d.lange 2010-04-08) - PLID 38096 - when returning from a save that hasn't been merged follow these rules
				if(strDiscountUser.IsEmpty()) {
					(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(FALSE);
				}else{
					(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(TRUE);
				}
			}else{
				(GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->EnableWindow(FALSE);
			}
			//Rules:  A proposal is allowed to be saved & reopened / modified as many times as the user wishes, UNTIL
			//	it has been merged.  Once a proposal is merged, that proposal is permanent.
			if(m_nMailSentID != -1) {
				DisableAllInterface();
			}
		}
		else {
			if(m_nExistingLoadFromID != -1) {
				//Load from an existing proposal, we are just copying over the selections
				_RecordsetPtr prsLoad = CreateRecordset("SELECT OpportunityProposalsT.*, UsersT.Username, "
					"DiscountUsersT.Username AS DiscountedByUser, DiscountUsersT.PersonID AS DiscountedByUserID "
					"FROM OpportunityProposalsT LEFT JOIN UsersT ON OpportunityProposalsT.CreatedByUserID = UsersT.PersonID "
					"LEFT JOIN UsersT DiscountUsersT ON OpportunityProposalsT.DiscountedBy = DiscountUsersT.PersonID "
					"WHERE ID = %li", m_nExistingLoadFromID);
				if(prsLoad->eof) {
					//This should be impossible
					AfxThrowNxException("Attempted to create new proposal from invalid proposal ID %li.", m_nID);
				}

				//Load the "general" fields
				FieldsPtr pFlds = prsLoad->Fields;

				//Load all the selected values
				bSched = AdoFldBool(pFlds, "Scheduler");
				b1License = AdoFldBool(pFlds, "LicenseSched");
				bBilling = AdoFldBool(pFlds, "Billing");
				bHCFA = AdoFldBool(pFlds, "HCFA");
				bEBilling = AdoFldBool(pFlds, "EBilling");
				bLetters = AdoFldBool(pFlds, "Letters");
				bQuotes = AdoFldBool(pFlds, "Quotes");
				bTracking = AdoFldBool(pFlds, "Tracking");
				bNexForms = AdoFldBool(pFlds, "NexForms");
				bInventory = AdoFldBool(pFlds, "Inventory");
				bNexSpa = AdoFldBool(pFlds, "NexSpa");
				bNexASC = AdoFldBool(pFlds, "NexASC");
				bMirror = AdoFldBool(pFlds, "Mirror");
				bUnited = AdoFldBool(pFlds, "United");
				bHL7 = AdoFldBool(pFlds, "HL7");
				// (d.thompson 2013-07-08) - PLID 57334 - I made this an int in the database as I think we'll eventually support multiple.  Convert to BOOL for now.
				bHIE = AdoFldLong(pFlds, "HIELink");
				bInform = AdoFldBool(pFlds, "Inform");
				bQuickbooks = AdoFldBool(pFlds, "Quickbooks");
				//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
				bERemittance= AdoFldBool(pFlds, "ERemittance");
				bEEligibility = AdoFldBool(pFlds, "EEligibility");
				// (d.thompson 2009-02-02) - PLID 32890
				bCandA = AdoFldBool(pFlds, "CandA");
				// (d.thompson 2009-08-10) - PLID 35152
				bNexWebLeads = AdoFldBool(pFlds, "NexWebLeads");
				bNexWebPortal = AdoFldBool(pFlds, "NexWebPortal");
				nNexWebAddtlDomains = AdoFldLong(pFlds, "NexWebAddtlDomains");
				// (d.thompson 2009-11-06) - PLID 36123
				bNexPhoto = AdoFldBool(pFlds, "NexPhoto");
				// (d.lange 2010-3-31) - PLID 37956 - grab the value for the Split EMR checkbox
				bSplitEMR = AdoFldBool(pFlds, "SplitEMR");
				// (d.lange 2010-04-08) - PLID 38096 - variable for setting the apply discount to emr merge checkbox
				bEMRDiscount = AdoFldBool(pFlds, "ApplyEMRDiscount");

				nWorkstations = AdoFldLong(pFlds, "Workstations");
				// (d.thompson 2012-10-12) - PLID 53155
				nWorkstationsTS = AdoFldLong(pFlds, "WorkstationsTS");
				// (d.lange 2010-3-31) - PLID 37956 - add EMR Workstations
				nEMRWorkstations = AdoFldLong(pFlds, "EMRWorkstations");
				nDoctors = AdoFldLong(pFlds, "Doctors");
				nPDA = AdoFldLong(pFlds, "PDA");
				nNexSync = AdoFldLong(pFlds, "NexSync");
				nEMR = AdoFldLong(pFlds, "EMR");
				nTraining = AdoFldLong(pFlds, "Training");
				nEmrTraining = AdoFldLong(pFlds, "EmrTraining"); // (z.manning, 11/26/2007) - PLID 28159
				nSupport = AdoFldLong(pFlds, "Support");
				nConversion = AdoFldLong(pFlds, "Conversion");
				nTravel = AdoFldLong(pFlds, "Travel");
				//DRT 2/13/2008 - PLID 28905 - Added multi DB
				nExtraDB = AdoFldLong(pFlds, "ExtraDB");
				nEMRConversion = AdoFldLong(pFlds, "EMRConversion");
				nFinancialConversion = AdoFldLong(pFlds, "FinancialConversion");
				// (d.lange 2010-12-21 14:52) - PLID 41889 - Added Travel Type
				m_nTravelType = AdoFldLong(pFlds, "TravelType");

				//DRT 4/3/2008 - PLID 29493 - We don't want to load the discount amount if this is a 'special' package.
				if(m_nTypeID != eptNexRes && m_nTypeID != eptNexStartup) {
					cyDiscountAmount = AdoFldCurrency(pFlds, "DiscountAmount");
					strDiscountUser = AdoFldString(pFlds, "DiscountedByUser", "");
					m_nCurrentDiscountUserID = AdoFldLong(pFlds, "DiscountedByUserID", -1);
				}

				//DRT 9/18/2008 - PLID 31395 - For 'Standard' edition
				bIsStandardSched = AdoFldBool(pFlds, "IsSchedStandard", FALSE);
				bIsStandardEMR = AdoFldBool(pFlds, "IsEMRStandard", FALSE);
				// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic
				bIsCosmeticEMR = AdoFldBool(pFlds, "IsEMRCosmetic", FALSE);

				if(bSplitEMR) {
					(GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->EnableWindow(TRUE);
					// (d.lange 2010-04-08) - PLID 38096 - when returning from a save that hasn't been merged follow these rules
					if(strDiscountUser.IsEmpty()) {
						(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(FALSE);
					}else{
						(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(TRUE);
					}
				}else{
					(GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->EnableWindow(FALSE);
				}
			}
			else {
				//On a new proposal, fill all default values.  These are the most common starting points.

				//Load all the selected values
				bSched = TRUE;
				b1License = TRUE;
				bBilling = FALSE;
				bHCFA = FALSE;
				bEBilling = FALSE;
				bLetters = FALSE;
				bQuotes = FALSE;
				bTracking = FALSE;
				bNexForms = FALSE;
				bInventory = FALSE;
				bNexSpa = FALSE;
				bNexASC = FALSE;
				bMirror = FALSE;
				bUnited = FALSE;
				bHL7 = FALSE;
				// (d.thompson 2013-07-08) - PLID 57334
				bHIE = FALSE;
				bInform = FALSE;
				bQuickbooks = FALSE;
				//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
				bERemittance = FALSE;
				bEEligibility = FALSE;
				// (d.thompson 2009-02-02) - PLID 32890 - C&A
				bCandA = FALSE;
				// (d.thompson 2009-08-10) - PLID 35152
				bNexWebLeads = FALSE;
				bNexWebPortal = FALSE;
				nNexWebAddtlDomains = 0;
				// (d.thompson 2009-11-06) - PLID 36123
				bNexPhoto = FALSE;
				// (d.lange 2010-3-31) - PLID 37956 - unselect the Split EMR checkbox because this is a new proposal
				bSplitEMR = FALSE;
				// (d.lange 2010-04-08) - PLID 38096 - variable for setting the apply discount to emr merge checkbox
				bEMRDiscount = FALSE;

				nWorkstations = 0;
				// (d.thompson 2012-10-12) - PLID 53155
				nWorkstationsTS = 0;
				// (d.lange 2010-3-31) - PLID 37956 - initalize EMR Workstations
				nEMRWorkstations = 0;
				nDoctors = 0;
				nPDA = 0;
				nNexSync = 0;
				nEMR = 0;
				nTraining = 1;
				nEmrTraining = 0; // (z.manning, 11/26/2007) - PLID 28159
				nSupport = 9;
				nConversion = 0;
				nTravel = 1;
				//DRT 2/13/2008 - PLID 28905 - Added multi DB
				nExtraDB = 0;
				// (d.lange 2010-09-17 12:05) - PLID 40361 - Set the EMR & Financial Conversion quantity
				nEMRConversion = 0;
				nFinancialConversion = 0;
				// (d.lange 2010-12-21 14:53) - PLID 41889 - Added Travel Type
				// This is a new proposal, we'll need to set the travel type based on City or Country
				SetTravelType();

				//DRT 9/18/2008 - PLID 31395 - For 'Standard' edition
				bIsStandardSched = FALSE;
				bIsStandardEMR = FALSE;
				// (d.thompson 2013-07-05) - PLID 57333 - cosmetic
				bIsCosmeticEMR = FALSE;



				//DRT 3/31/2008 - PLID 29493 - We are now going to tailor our defaults based on the "type" of
				//	package, so we'll modify the default selections.
				switch(m_nTypeID) {
				case eptNewSale:
					{
						//Nothing differs from the default
					}
					break;

				case eptNexRes:
				case eptNexStartup:
					{
						//These 2 are actually the same just with a different discount applied.
						bBilling = TRUE;
						bHCFA = TRUE;
						nWorkstations = 1;
						nTravel = 0;
					}
					break;

				case eptRenewSupport:
				case ept3MonthAddOn:
				case eptAddOn:
				default:
					{
						//TODO:  Not quite yet supported
						AfxMessageBox("NOT YET SUPPORTED");
						EndDialog(0);
						return 0;
					}
					break;
				}
			}

			//The following settings apply to all new proposals, whether they copied from existing
			//	or made a new blank one.
			dtPropDate = COleDateTime::GetCurrentTime();

			//Expiration date.  Per m.rosenberg, they want no proposals outstanding at
			//	the end of a quarter.  So the behavior here is:  Find the end of the quarter
			//	days from now.  Set the default expiration to the lesser of that number
			//	or 30 days.
			//NOTE:  This will use the local system date when calculating the days to end of Qtr and setting the defaults.
			//	The validation of the proposal uses the server time.  This is done slightly for optimization -- the server
			//	time check is only to avoid people cheating the system.  We don't need to do it here, it will be caught
			//	before saved.
			long nDaysToEndOfQuarter = CalculateDaysToEndOfQuarter(COleDateTime::GetCurrentTime());
			dtExpireDate = dtPropDate + COleDateTimeSpan(nDaysToEndOfQuarter > 14 ? 14 : nDaysToEndOfQuarter, 0, 0, 0);

			strUsername = GetCurrentUserName();

			//Fill in next available quote ID #.  This follows the "new patient" system, if someone else
			//	saves a proposal while this one is being created, it will just warn you and generate the next one.
			nQuoteNum = NewNumber("OpportunityProposalsT", "QuoteNum");
		}

		//DRT 4/3/2008 - PLID 29493 - For certain types, we want to apply a special discount.  This is outside the loading above, 
		//	as we want this to show up no matter how we opened (new, load existing, etc), as it loads from the default pricing data.
		if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
			//We also apply a discount to the entire package, as well as a set a percent off
			//	on all addons.
			cySpecialDiscount = m_aryPrices[ecpSpecialStartup];
			nSpecialAddOnPct = m_nStartupAddOnDiscount;
			if(m_nTypeID == eptNexRes) {
				cySpecialDiscount = m_aryPrices[ecpSpecialResident];
				nSpecialAddOnPct = m_nResidentAddOnDiscount;
			}

			//Also we want to completely disable the discount abilities
			GetDlgItem(IDC_CHANGE_DISCOUNT)->EnableWindow(FALSE);

			//DRT 5/27/2008 - PLID 29493 - For now, we want to disable the ability to change support.  Since the proposal
			//	is split, you cannot simply add onto the support costs (the first page is not allowed to change), 
			//	you would have to make a new line item for "addtl support", which would then calculate off the
			//	full price, not off the addon price alone.  So we're just disabling the ability altogether, 
			//	m.rosenberg says it's never come up and is fine with that.
			GetDlgItem(IDC_NUM_SUPPORT)->EnableWindow(FALSE);
		}

		//DRT 9/23/2008 - PLID 31395 - For now, if we are on a NexRes or NexStartup package, the 'Standard' editions
		//	are unavailable.  This is likely to change later.
		//(d.thompson 2009-03-17) - PLID 33553 - EMR Standard is now allowed for these.  Scheduler standard is not, nor
		//	have we any intentions at this time to do so in the future.
		if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
			//These should be unchecked, but make sure.
			bIsStandardSched = FALSE;

			//Disable the checkboxes 
			GetDlgItem(IDC_CHK_SCHED_STANDARD)->EnableWindow(FALSE);
		}

		//
		//Now that all our variables are set, load them into the interface
		m_pickerProposalDate.SetValue(_variant_t(dtPropDate));
		m_pickerExpireDate.SetValue(_variant_t(dtExpireDate));
		SetDlgItemInt(IDC_QUOTE_ID, nQuoteNum);
		SetDlgItemText(IDC_CREATED_BY, strUsername);
		SetDlgItemText(IDC_DISCOUNT_USERNAME, strDiscountUser);

		//Load all the selected values
		CheckDlgButton(IDC_CHK_SCHED, bSched);
		CheckDlgButton(IDC_CHK_1LICENSE, b1License);
		CheckDlgButton(IDC_CHK_BILLING, bBilling);
		CheckDlgButton(IDC_CHK_HCFA, bHCFA);
		CheckDlgButton(IDC_CHK_EBILLING, bEBilling);
		CheckDlgButton(IDC_CHK_LETTERS, bLetters);
		CheckDlgButton(IDC_CHK_QUOTES, bQuotes);
		CheckDlgButton(IDC_CHK_TRACKING, bTracking);
		CheckDlgButton(IDC_CHK_NEXFORMS, bNexForms);
		CheckDlgButton(IDC_CHK_INV, bInventory);
		CheckDlgButton(IDC_CHK_NEXSPA, bNexSpa);
		CheckDlgButton(IDC_CHK_ASC, bNexASC);
		CheckDlgButton(IDC_CHK_MIRROR, bMirror);
		CheckDlgButton(IDC_CHK_UNITED, bUnited);
		CheckDlgButton(IDC_CHK_HL7, bHL7);
		// (d.thompson 2013-07-08) - PLID 57334
		CheckDlgButton(IDC_CHK_HIE, bHIE);
		CheckDlgButton(IDC_CHK_INFORM, bInform);
		CheckDlgButton(IDC_CHK_QUICKBOOKS, bQuickbooks);
		//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
		CheckDlgButton(IDC_CHK_EREMITTANCE, bERemittance);
		CheckDlgButton(IDC_CHK_EELIGIBILITY, bEEligibility);

		//DRT 9/18/2008 - PLID 31395 - For 'Standard' edition.  Note that I do these above EMR so that when the OnChange is fired, 
		//	it will update with the correct price.
		CheckDlgButton(IDC_CHK_SCHED_STANDARD, bIsStandardSched);
		CheckDlgButton(IDC_CHK_EMR_STANDARD, bIsStandardEMR);
		// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic EMR
		CheckDlgButton(IDC_CHK_EMR_COSMETIC, bIsCosmeticEMR);
		// (d.thompson 2009-02-02) - PLID 32890 - C&A
		CheckDlgButton(IDC_CHK_C_AND_A, bCandA);
		// (d.thompson 2009-08-10) - PLID 35152
		CheckDlgButton(IDC_CHK_NEXWEB_LEADS, bNexWebLeads);
		CheckDlgButton(IDC_CHK_NEXWEB_PORTAL, bNexWebPortal);
		SetDlgItemInt(IDC_NUM_NEXWEB_ADDTL_DOMAINS, nNexWebAddtlDomains);
		// (d.thompson 2009-11-06) - PLID 36123
		CheckDlgButton(IDC_CHK_NEXPHOTO, bNexPhoto);
		// (d.lange 2010-3-31) - PLID 37956 - set the Split EMR checkbox
		CheckDlgButton(IDC_CHK_SPLITEMR, bSplitEMR);
		// (d.lange 2010-04-08) - PLID 38096 - variable for setting the apply discount to emr merge checkbox
		CheckDlgButton(IDC_CHK_DISCOUNT_EMR, bEMRDiscount);

		SetDlgItemInt(IDC_NUM_WORKSTATIONS, nWorkstations);
		// (d.thompson 2012-10-12) - PLID 53155 - TS workstations
		SetDlgItemInt(IDC_NUM_WORKSTATIONS_TS, nWorkstationsTS);
		// (d.lange 2010-3-31) - PLID 37956 - set EMR Workstations edit box
		SetDlgItemInt(IDC_NUM_EMRWORKSTATIONS, nEMRWorkstations);
		SetDlgItemInt(IDC_NUM_DOCTORS, nDoctors);
		SetDlgItemInt(IDC_NUM_PDA, nPDA);
		// (d.thompson 2009-11-13) - PLID 36124
		SetDlgItemInt(IDC_NUM_NEXSYNC, nNexSync);
		SetDlgItemInt(IDC_NUM_EMR, nEMR);
		SetDlgItemInt(IDC_NUM_TRAINING, nTraining);
		SetDlgItemInt(IDC_NUM_EMR_TRAINING, nEmrTraining); // (z.manning, 11/26/2007) - PLID 28159
		SetDlgItemInt(IDC_NUM_SUPPORT, nSupport);
		SetDlgItemInt(IDC_NUM_CONVERSION, nConversion);
		SetDlgItemInt(IDC_NUM_TRAVEL, nTravel);
		//DRT 2/13/2008 - PLID 28905 - Added multi DB
		SetDlgItemInt(IDC_NUM_EXTRADB, nExtraDB);
		SetDlgItemInt(IDC_NUM_EMRCONVERSION, nEMRConversion);
		SetDlgItemInt(IDC_NUM_FINCONVERSION, nFinancialConversion);

		//DRT 4/3/2008 - PLID 29493 - Special discounts for packages
		SetDlgItemText(IDC_SPECIAL_DISCOUNT_DOLLAR, FormatCurrencyForInterface(cySpecialDiscount));
		SetDlgItemText(IDC_SPECIAL_DISCOUNT_PERCENT, FormatString("%li%%", nSpecialAddOnPct));

		//If any of our "standards" are set, we need to re-display the pricing array, it is loaded in
		//	OnInitDialog with the assumption none of these are set.
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic too
		if(bIsStandardSched || bIsStandardEMR || bIsCosmeticEMR) {
			ReflectPriceArray();
		}

		//Update interface elements
		ReflectSchedulerPackage();
		ReflectFinancialPackage();
		ReflectCosmeticPackage();
		ReflectOtherModules();
		// (d.thompson 2009-08-10or) - PLID 35152
		ReflectNexWebPackage();

		OnChkMirror();
		OnChkUnited();
		OnChkHl7();
		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		OnChkHIE();
		OnChkInform();
		OnChkQuickbooks();

		//discounts
		SetDlgItemText(IDC_DISCOUNT_TOTAL_DOLLAR, FormatCurrencyForInterface(cyDiscountAmount, TRUE));

		//Update the totals
		UpdateAllTotals();
		// (d.lange 2010-04-06) - PLID 38016 - fill the module array now
		FillModuleArray();
		
		// (d.lange 2010-04-06) - PLID 38096 - Show checkbox if split emr is checked
		if(IsDlgButtonChecked(IDC_CHK_SPLITEMR)) {
			(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->ShowWindow(TRUE);
		}
	} NxCatchAll("Error in OnLoadOpportunityProposal");

	return 0;
}

//Fill @ eCurrentPrice enum positions
void COpportunityProposalDlg::FillPriceArray()
{
	//Clear the array then make enough space
	m_aryPrices.RemoveAll();
	m_aryPrices.SetSize(ecpTotalPrices);

	//The design is that we look for the max ID that is active (just in case -- there should never be more than 1 active
	//	price structure), and load those prices.  If we are loading an existing proposal, then we must use the pricing
	//	that was saved.
	CString strSql = "SELECT * FROM OpportunityPriceStructureT WHERE ID = ";
	if(m_nID == -1) {
		//New proposal, find the current pricing structure
		strSql += "(SELECT MAX(ID) FROM OpportunityPriceStructureT WHERE Active = 1)";
	}
	else {
		//This is an existing proposal, use its pricing structure
		CString strTmp;
		strTmp.Format("(SELECT PriceStructureID FROM OpportunityProposalsT WHERE ID = %li)", m_nID);
		strSql += strTmp;
	}

	_RecordsetPtr prsPrices = CreateRecordsetStd(strSql);
	if(prsPrices->eof) {
		//No prices found that are active!  Must fail
		AfxThrowNxException("No active prices are available.  Please contact NexTech Technical Support.");
	}
	else {
		//We've got the active pricing, setup our array.  Note that these fields allow NULL in case of an old structure being
		//	looked at (new modules added, old modules removed)
		FieldsPtr pFlds = prsPrices->Fields;
		m_aryPrices[ecpScheduler] = AdoFldCurrency(pFlds, "Scheduler", COleCurrency(0, 0));
		m_aryPrices[ecp1License] = AdoFldCurrency(pFlds, "LicenseSched", COleCurrency(0, 0));
		m_aryPrices[ecpBilling] = AdoFldCurrency(pFlds, "Billing", COleCurrency(0, 0));
		m_aryPrices[ecpHCFA] = AdoFldCurrency(pFlds, "HCFA", COleCurrency(0, 0));
		m_aryPrices[ecpEBilling] = AdoFldCurrency(pFlds, "EBilling", COleCurrency(0, 0));
		m_aryPrices[ecpLetters] = AdoFldCurrency(pFlds, "Letters", COleCurrency(0, 0));
		m_aryPrices[ecpQuotes] = AdoFldCurrency(pFlds, "Quotes", COleCurrency(0, 0));
		m_aryPrices[ecpTracking] = AdoFldCurrency(pFlds, "Tracking", COleCurrency(0, 0));
		m_aryPrices[ecpNexForms] = AdoFldCurrency(pFlds, "NexForms", COleCurrency(0, 0));
		m_aryPrices[ecpInventory] = AdoFldCurrency(pFlds, "Inventory", COleCurrency(0, 0));
		m_aryPrices[ecpSpa] = AdoFldCurrency(pFlds, "NexSpa", COleCurrency(0, 0));
		m_aryPrices[ecpASC] = AdoFldCurrency(pFlds, "NexASC", COleCurrency(0, 0));
		m_aryPrices[ecpWorkstation] = AdoFldCurrency(pFlds, "Workstations", COleCurrency(0, 0));
		m_aryPrices[ecpDoctor] = AdoFldCurrency(pFlds, "Doctors", COleCurrency(0, 0));
		m_aryPrices[ecpPDA] = AdoFldCurrency(pFlds, "PDA", COleCurrency(0, 0));
		m_aryPrices[ecpMirror] = AdoFldCurrency(pFlds, "Mirror", COleCurrency(0, 0));
		m_aryPrices[ecpUnited] = AdoFldCurrency(pFlds, "United", COleCurrency(0, 0));
		m_aryPrices[ecpHL7] = AdoFldCurrency(pFlds, "HL7", COleCurrency(0, 0));
		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		m_aryPrices[ecpHIE] = AdoFldCurrency(pFlds, "HIELink", COleCurrency(0, 0));
		m_aryPrices[ecpInform] = AdoFldCurrency(pFlds, "Inform", COleCurrency(0, 0));
		m_aryPrices[ecpQuickbooks] = AdoFldCurrency(pFlds, "Quickbooks", COleCurrency(0, 0));
		m_aryPrices[ecpEMRFirstDoctor] = AdoFldCurrency(pFlds, "EMRFirst", COleCurrency(0, 0));
		m_aryPrices[ecpEMRAddtlDoctor] = AdoFldCurrency(pFlds, "EMRAddtl", COleCurrency(0, 0));
		m_aryPrices[ecpTraining] = AdoFldCurrency(pFlds, "Training", COleCurrency(0, 0));
		//TODO:  Support?
		m_aryPrices[ecpConversion] = AdoFldCurrency(pFlds, "Conversion", COleCurrency(0, 0));
		m_aryPrices[ecpTravel] = AdoFldCurrency(pFlds, "Travel", COleCurrency(0, 0));
		m_aryPrices[ecpPkgSched] = AdoFldCurrency(pFlds, "PackageScheduler", COleCurrency(0, 0));
		m_aryPrices[ecpPkgFinancial] = AdoFldCurrency(pFlds, "PackageFinancial", COleCurrency(0, 0));
		m_aryPrices[ecpPkgCosmetic] = AdoFldCurrency(pFlds, "PackageCosmetic", COleCurrency(0, 0));
		//DRT 2/11/2008 - PLID 28881 - Added ERemit and EElig
		m_aryPrices[ecpERemittance] = AdoFldCurrency(pFlds, "ERemittance", COleCurrency(0, 0));
		m_aryPrices[ecpEEligibility] = AdoFldCurrency(pFlds, "EEligibility", COleCurrency(0, 0));
		//DRT 2/13/2008 - PLID 28905 - Added multi DB
		m_aryPrices[ecpExtraDB] = AdoFldCurrency(pFlds, "ExtraDB", COleCurrency(0, 0));
		//DRT 4/2/2008 - PLID 29493 - Added Resident & Startup packages.  AddOn values are percent off.
		m_aryPrices[ecpSpecialResident] = AdoFldCurrency(pFlds, "SpecialPkgResident", COleCurrency(0, 0));
		m_aryPrices[ecpSpecialStartup] = AdoFldCurrency(pFlds, "SpecialPkgStartup", COleCurrency(0, 0));
		m_nResidentAddOnDiscount = AdoFldLong(pFlds, "SpecialPkgAddOnResident", 0);
		m_nStartupAddOnDiscount = AdoFldLong(pFlds, "SpecialPkgAddOnStartup", 0);
		//DRT 9/17/2008 - PLID 31395 - Added 'Standard' packages.
		m_aryPrices[ecpSchedulerStandard] = AdoFldCurrency(pFlds, "SchedulerStandard", COleCurrency(0, 0));
		m_aryPrices[ecpLettersStandard] = AdoFldCurrency(pFlds, "LettersStandard", COleCurrency(0, 0));
		m_aryPrices[ecpEMRFirstStandard] = AdoFldCurrency(pFlds, "EMRFirstStandard", COleCurrency(0, 0));
		m_aryPrices[ecpEMRAddtlStandard] = AdoFldCurrency(pFlds, "EMRAddtlStandard", COleCurrency(0, 0));
		m_aryPrices[ecpPkgSchedStandard] = AdoFldCurrency(pFlds, "PackageSchedulerStandard", COleCurrency(0, 0));
		m_aryPrices[ecpPkgCosmeticStandard] = AdoFldCurrency(pFlds, "PackageCosmeticStandard", COleCurrency(0, 0));
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic emr
		m_aryPrices[ecpEMRCosmetic] = AdoFldCurrency(pFlds, "EMRCosmetic", COleCurrency(0, 0));
		// (d.thompson 2009-01-30) - PLID 32890 - Added Consignment and Allocation package
		m_aryPrices[ecpCandA] = AdoFldCurrency(pFlds, "CandA", COleCurrency(0, 0));
		// (d.thompson 2009-08-10) - PLID 35152 - Added NexWeb
		m_aryPrices[ecpPkgNexWeb] = AdoFldCurrency(pFlds, "PackageNexWeb", COleCurrency(0, 0));
		m_aryPrices[ecpNexWebLeads] = AdoFldCurrency(pFlds, "NexWebLeads", COleCurrency(0, 0));
		m_aryPrices[ecpNexWebPortal] = AdoFldCurrency(pFlds, "NexWebPortal", COleCurrency(0, 0));
		m_aryPrices[ecpNexWebAddtlDomains] = AdoFldCurrency(pFlds, "NexWebAddtlDomain", COleCurrency(0, 0));
		// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
		m_aryPrices[ecpNexPhoto] = AdoFldCurrency(pFlds, "NexPhoto", COleCurrency(0, 0));
		// (d.thompson 2009-11-13) - PLID 36124
		m_aryPrices[ecpNexSync] = AdoFldCurrency(pFlds, "NexSync", COleCurrency(0, 0));
		// (d.lange 2010-09-17 12:28) - PLID 40361 - Include EMR & Finanical conversion prices
		m_aryPrices[ecpEmrConversion] = AdoFldCurrency(pFlds, "EmrConversion", COleCurrency(0, 0));
		m_aryPrices[ecpFinancialConversion] = AdoFldCurrency(pFlds, "FinancialConversion", COleCurrency(0, 0));
		// (d.lange 2010-12-21 17:07) - PLID 41889 - Support additional travel rules
		m_aryPrices[ecpTravelCanada] = AdoFldCurrency(pFlds, "TravelCanada", COleCurrency(0, 0));
		m_aryPrices[ecpTravelInternational] = AdoFldCurrency(pFlds, "TravelInternational", COleCurrency(0, 0));
		m_aryPrices[ecpTravelNewYorkCity] = AdoFldCurrency(pFlds, "TravelNewYorkCity", COleCurrency(0, 0));

		//Save the price ID that we used
		m_nPriceStructureID = AdoFldLong(pFlds, "ID");
	}
}

// (d.lange 2010-4-1) - PLID 38016 - fill the module array based on what is checked
void COpportunityProposalDlg::FillModuleArray()
{
	try {
		
		// Set each index of the modules array
		m_aryModules[ecmScheduler] = (IsDlgButtonChecked(IDC_CHK_SCHED) ? 1 : 0);
		m_aryModules[ecmStdScheduler] = (IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD) ? 1 : 0);
		m_aryModules[ecm1License] = (IsDlgButtonChecked(IDC_CHK_1LICENSE) ? 1 : 0);
		m_aryModules[ecmBilling] = (IsDlgButtonChecked(IDC_CHK_BILLING) ? 1 : 0);
		m_aryModules[ecmHCFA] = (IsDlgButtonChecked(IDC_CHK_HCFA) ? 1 : 0);
		m_aryModules[ecmEBilling] = (IsDlgButtonChecked(IDC_CHK_EBILLING) ? 1 : 0);
		m_aryModules[ecmLetterWriting] = (IsDlgButtonChecked(IDC_CHK_LETTERS) ? 1 : 0);
		m_aryModules[ecmQuotesMarketing] = (IsDlgButtonChecked(IDC_CHK_QUOTES) ? 1 : 0);
		m_aryModules[ecmNexTrak] = (IsDlgButtonChecked(IDC_CHK_TRACKING) ? 1 : 0);
		m_aryModules[ecmNexForms] = (IsDlgButtonChecked(IDC_CHK_NEXFORMS) ? 1 : 0);
		m_aryModules[ecmInventory] = (IsDlgButtonChecked(IDC_CHK_INV) ? 1 : 0);
		m_aryModules[ecmCandA] = (IsDlgButtonChecked(IDC_CHK_C_AND_A) ? 1 : 0);
		m_aryModules[ecmNexSpa] = (IsDlgButtonChecked(IDC_CHK_NEXSPA) ? 1 : 0);
		m_aryModules[ecmASC] = (IsDlgButtonChecked(IDC_CHK_ASC) ? 1 : 0);
		m_aryModules[ecmERemittance] = (IsDlgButtonChecked(IDC_CHK_EREMITTANCE) ? 1 : 0);
		m_aryModules[ecmEEligibility] = (IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) ? 1 : 0);
		m_aryModules[ecmNexPhoto] = (IsDlgButtonChecked(IDC_CHK_NEXPHOTO) ? 1 : 0);
		m_aryModules[ecmMirror] = (IsDlgButtonChecked(IDC_CHK_MIRROR) ? 1 : 0);
		m_aryModules[ecmUnited] = (IsDlgButtonChecked(IDC_CHK_UNITED) ? 1 : 0);
		m_aryModules[ecmHL7] = (IsDlgButtonChecked(IDC_CHK_HL7) ? 1 : 0);
		// (d.thompson 2013-07-08) - PLID 57334
		m_aryModules[ecmHIE] = (IsDlgButtonChecked(IDC_CHK_HIE) ? 1 : 0);
		m_aryModules[ecmInform] = (IsDlgButtonChecked(IDC_CHK_INFORM) ? 1 : 0);
		m_aryModules[ecmQuickBooks] = (IsDlgButtonChecked(IDC_CHK_QUICKBOOKS) ? 1 : 0);
		m_aryModules[ecmLeadGen] = (IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS) ? 1 : 0);
		m_aryModules[ecmPatientPortal] = (IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL) ? 1 : 0);
		m_aryModules[ecmStdEMR] = (IsDlgButtonChecked(IDC_CHK_EMR_STANDARD) ? 1 : 0);
		// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic EMR
		m_aryModules[ecmCosmeticEMR] = (IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC) ? 1 : 0);
		m_aryModules[ecmPDA] = GetDlgItemInt(IDC_NUM_PDA);
		m_aryModules[ecmNexSync] = GetDlgItemInt(IDC_NUM_NEXSYNC);
		m_aryModules[ecmEMR] = GetDlgItemInt(IDC_NUM_EMR);
		m_aryModules[ecmWorkstations] = GetDlgItemInt(IDC_NUM_WORKSTATIONS);
		// (d.thompson 2012-10-12) - PLID 53155
		m_aryModules[ecmWorkstationsTS] = GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS);
		m_aryModules[ecmEMRWorkstations] = GetDlgItemInt(IDC_NUM_EMRWORKSTATIONS);
		m_aryModules[ecmMultiDoctor] = GetDlgItemInt(IDC_NUM_DOCTORS);
		m_aryModules[ecmAddDomains] = GetDlgItemInt(IDC_NUM_NEXWEB_ADDTL_DOMAINS);
		m_aryModules[ecmPMTraining] = GetDlgItemInt(IDC_NUM_TRAINING);
		m_aryModules[ecmEMRTraining] = GetDlgItemInt(IDC_NUM_EMR_TRAINING);
		m_aryModules[ecmSupportPercent] = GetDlgItemInt(IDC_SUPPORT_PERCENT);
		m_aryModules[ecmSupportMonths] = GetDlgItemInt(IDC_NUM_SUPPORT);
		m_aryModules[ecmConversion] = GetDlgItemInt(IDC_NUM_CONVERSION);
		m_aryModules[ecmTravel] = GetDlgItemInt(IDC_NUM_TRAVEL);
		m_aryModules[ecmExtraDB] = GetDlgItemInt(IDC_NUM_EXTRADB);
		
	} NxCatchAll("Error on FillModuleArray");
}

void COpportunityProposalDlg::ReflectPriceArray()
{
	//DRT 9/18/2008 - PLID 31395 - Option for 'Standard' package
	if(!IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
		SetDlgItemText(IDC_LABEL_SCHED, FormatCurrencyForInterface(m_aryPrices[ecpScheduler]));
	}
	else {
		SetDlgItemText(IDC_LABEL_SCHED, FormatCurrencyForInterface(m_aryPrices[ecpSchedulerStandard]));
	}
	SetDlgItemText(IDC_LABEL_1LICENSE, FormatCurrencyForInterface(m_aryPrices[ecp1License]));
	SetDlgItemText(IDC_LABEL_BILLING, FormatCurrencyForInterface(m_aryPrices[ecpBilling]));
	SetDlgItemText(IDC_LABEL_HCFA, FormatCurrencyForInterface(m_aryPrices[ecpHCFA]));
	SetDlgItemText(IDC_LABEL_EBILLING, FormatCurrencyForInterface(m_aryPrices[ecpEBilling]));
	SetDlgItemText(IDC_LABEL_LETTERS, FormatCurrencyForInterface(m_aryPrices[ecpLetters]));
	SetDlgItemText(IDC_LABEL_QUOTES, FormatCurrencyForInterface(m_aryPrices[ecpQuotes]));
	SetDlgItemText(IDC_LABEL_TRACKING, FormatCurrencyForInterface(m_aryPrices[ecpTracking]));
	SetDlgItemText(IDC_LABEL_NEXFORMS, FormatCurrencyForInterface(m_aryPrices[ecpNexForms]));
	SetDlgItemText(IDC_LABEL_INV, FormatCurrencyForInterface(m_aryPrices[ecpInventory]));
	SetDlgItemText(IDC_LABEL_SPA, FormatCurrencyForInterface(m_aryPrices[ecpSpa]));
	SetDlgItemText(IDC_LABEL_ASC, FormatCurrencyForInterface(m_aryPrices[ecpASC]));
	SetDlgItemText(IDC_LABEL_WORKSTATIONS, FormatCurrencyForInterface(m_aryPrices[ecpWorkstation]));
	// (d.thompson 2012-10-12) - PLID 53155 - Added ts workstations.  However, we are going to keep using the
	//	same pricing that workstations use, because these are only being split for ease-of-reading.
	SetDlgItemText(IDC_LABEL_WORKSTATIONS_TS, FormatCurrencyForInterface(m_aryPrices[ecpWorkstation]));
	// (d.lange 2010-3-31) - PLID 37956 - EMR Workstations are the same price as standard workstations
	SetDlgItemText(IDC_LABEL_EMRWORKSTATIONS, FormatCurrencyForInterface(m_aryPrices[ecpWorkstation]));
	SetDlgItemText(IDC_LABEL_DOCTORS, FormatCurrencyForInterface(m_aryPrices[ecpDoctor]));
	SetDlgItemText(IDC_LABEL_PDA, FormatCurrencyForInterface(m_aryPrices[ecpPDA]));
	// (d.thompson 2009-11-13) - PLID 36124
	SetDlgItemText(IDC_LABEL_NEXSYNC, FormatCurrencyForInterface(m_aryPrices[ecpNexSync]));
	SetDlgItemText(IDC_LABEL_MIRROR, FormatCurrencyForInterface(m_aryPrices[ecpMirror]));
	SetDlgItemText(IDC_LABEL_UNITED, FormatCurrencyForInterface(m_aryPrices[ecpUnited]));
	SetDlgItemText(IDC_LABEL_HL7, FormatCurrencyForInterface(m_aryPrices[ecpHL7]));
	// (d.thompson 2013-07-08) - PLID 57334
	SetDlgItemText(IDC_LABEL_HIE, FormatCurrencyForInterface(m_aryPrices[ecpHIE]));
	SetDlgItemText(IDC_LABEL_INFORM, FormatCurrencyForInterface(m_aryPrices[ecpInform]));
	SetDlgItemText(IDC_LABEL_QUICKBOOKS, FormatCurrencyForInterface(m_aryPrices[ecpQuickbooks]));
	//DRT 9/18/2008 - PLID 31395 - Option for 'Standard' package
	// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic EMR too
	if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
		SetDlgItemText(IDC_LABEL_EMR, FormatCurrencyForInterface(m_aryPrices[ecpEMRFirstStandard]));
	}
	else if(IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC)) {
		SetDlgItemText(IDC_LABEL_EMR, FormatCurrencyForInterface(m_aryPrices[ecpEMRCosmetic]));
	}
	else {
		SetDlgItemText(IDC_LABEL_EMR, FormatCurrencyForInterface(m_aryPrices[ecpEMRFirstDoctor]));
	}
//	SetDlgItemText(IDC_LABEL_EMR2, FormatCurrencyForInterface(m_aryPrices[ecpEMRAddtlDoctor]));
	SetDlgItemText(IDC_LABEL_TRAINING, FormatCurrencyForInterface(m_aryPrices[ecpTraining]));
//	SetDlgItemText(IDC_LABEL_CONVERSION, FormatCurrencyForInterface(m_aryPrices[ecpConversion]));
//	SetDlgItemText(IDC_LABEL_TRAVEL, FormatCurrencyForInterface(m_aryPrices[ecpTravel]));
	//DRT 9/18/2008 - PLID 31395 - Option for 'Standard' package
	if(!IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) { 
		SetDlgItemText(IDC_PKG_SCHED, FormatCurrencyForInterface(m_aryPrices[ecpPkgSched]));
	}
	else {
		SetDlgItemText(IDC_PKG_SCHED, FormatCurrencyForInterface(m_aryPrices[ecpPkgSchedStandard]));
	}
	//DRT 5/27/2008 - PLID 29493 - Resident & startup packages do not have a financial package.
	if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
		SetDlgItemText(IDC_PKG_FINANCIAL, FormatCurrencyForInterface(m_aryPrices[ecpBilling] + m_aryPrices[ecpHCFA] + m_aryPrices[ecpEBilling]));
	}
	else {
		SetDlgItemText(IDC_PKG_FINANCIAL, FormatCurrencyForInterface(m_aryPrices[ecpPkgFinancial]));
	}
	SetDlgItemText(IDC_PKG_COSMETIC, FormatCurrencyForInterface(m_aryPrices[ecpPkgCosmetic]));
	//DRT 2/11/2008 - PLID 28881 - Added Eremit and Eelig
	SetDlgItemText(IDC_LABEL_EREMITTANCE, FormatCurrencyForInterface(m_aryPrices[ecpERemittance]));
	SetDlgItemText(IDC_LABEL_EELIGIBILITY, FormatCurrencyForInterface(m_aryPrices[ecpEEligibility]));
	// (d.thompson 2009-01-30) - PLID 32890
	SetDlgItemText(IDC_LABEL_C_AND_A, FormatCurrencyForInterface(m_aryPrices[ecpCandA]));
	// (d.thompson 2009-08-10) - PLID 35152
	SetDlgItemText(IDC_LABEL_NEXWEB_LEADS, FormatCurrencyForInterface(m_aryPrices[ecpNexWebLeads]));
	SetDlgItemText(IDC_LABEL_NEXWEB_PORTAL, FormatCurrencyForInterface(m_aryPrices[ecpNexWebPortal]));
	SetDlgItemText(IDC_PKG_NEXWEB, FormatCurrencyForInterface(m_aryPrices[ecpPkgNexWeb]));
	SetDlgItemText(IDC_LABEL_NEXWEB_ADDTL_DOMAINS, FormatCurrencyForInterface(m_aryPrices[ecpNexWebAddtlDomains]));
	// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
	SetDlgItemText(IDC_LABEL_NEXPHOTO, FormatCurrencyForInterface(m_aryPrices[ecpNexPhoto]));
	// (d.lange 2010-11-11 16:05) - PLID 40361 - Conversion labels
	SetDlgItemText(IDC_LABEL_CONVERSION, FormatCurrencyForInterface(m_aryPrices[ecpConversion]));
	SetDlgItemText(IDC_LABEL_EMRCONVERSION, FormatCurrencyForInterface(m_aryPrices[ecpEmrConversion]));
	SetDlgItemText(IDC_LABEL_FINCONVERSION, FormatCurrencyForInterface(m_aryPrices[ecpFinancialConversion]));
}

COleCurrency COpportunityProposalDlg::GetCurrentSubTotalList()
{
	CString str;
	GetDlgItemText(IDC_SUBTOTAL_LIST, str);
	COleCurrency cySubTotal;
	cySubTotal.ParseCurrency(str);
	if(cySubTotal.GetStatus() == COleCurrency::invalid)
		//Not yet filled out, just set it to $0.00.
		cySubTotal = COleCurrency(0, 0);

	return cySubTotal;
}

COleCurrency COpportunityProposalDlg::GetCurrentSubTotalSelected()
{
	CString str;
	GetDlgItemText(IDC_SUBTOTAL_SEL, str);
	COleCurrency cySubTotal;
	cySubTotal.ParseCurrency(str);
	if(cySubTotal.GetStatus() == COleCurrency::invalid)
		//Not yet filled out, just set it to $0.00.
		cySubTotal = COleCurrency(0, 0);

	return cySubTotal;
}

//Update the training values
void COpportunityProposalDlg::UpdateTraining()
{
	//TODO:  We need an override, if it's set, do nothing
	long nDays = 0, nEmrDays = 0, nTrips = 0;

	CalculateTrainingFromCurrentInterface(nDays, nEmrDays, nTrips);

	//Now set these
	SetDlgItemInt(IDC_NUM_TRAINING, nDays);
	SetDlgItemInt(IDC_NUM_EMR_TRAINING, nEmrDays);

	//DRT 4/2/2008 - PLID 29493 - We do not do travel on resident & startup packages, they are sold internet training.
	//DRT 6/10/2008 - PLID 30346 - Actually there is travel, but only if there are addons.  I moved the calculations to 
	//	the CalculateTrainingFromCurrentInterface() function.
	SetDlgItemInt(IDC_NUM_TRAVEL, nTrips);
}

#define ADDAMOUNT(idc, cy)	{CString str;	GetDlgItemText(idc, str);	COleCurrency cyTmp(0, 0);	cyTmp.ParseCurrency(str);	if(cyTmp.GetStatus() == COleCurrency::valid) cy += cyTmp;}

void COpportunityProposalDlg::UpdateAllTotals()
{
	//Get the old subtotal
	COleCurrency cyOldSubTotal(0, 0);
	ADDAMOUNT(IDC_SUBTOTAL_SEL, cyOldSubTotal);

	//Pull the values from the "selection" boxes
	COleCurrency cySubTotal(0, 0);		//Includes package pricing
	COleCurrency cyListTotal(0, 0);		//Raw cost, no discounts from package pricing
	{
		//Calculate non-package pricing items first
		ADDAMOUNT(IDC_SEL_OTHER, cySubTotal);
		ADDAMOUNT(IDC_SEL_WORKSTATIONS, cySubTotal);
		// (d.thompson 2012-10-12) - PLID 53155
		ADDAMOUNT(IDC_SEL_WORKSTATIONS_TS, cySubTotal);
		// (d.lange 2010-03-06) - PLID 37999 - update EMR workstations subtotal
		ADDAMOUNT(IDC_SEL_EMRWORKSTATIONS, cySubTotal);
		ADDAMOUNT(IDC_SEL_DOCTORS, cySubTotal);
		ADDAMOUNT(IDC_SEL_PDA, cySubTotal);
		// (d.thompson 2009-11-13) - PLID 36124
		ADDAMOUNT(IDC_SEL_NEXSYNC, cySubTotal);
		ADDAMOUNT(IDC_SEL_MIRROR, cySubTotal);
		ADDAMOUNT(IDC_SEL_UNITED, cySubTotal);
		ADDAMOUNT(IDC_SEL_HL7, cySubTotal);
		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		ADDAMOUNT(IDC_SEL_HIE, cySubTotal);
		ADDAMOUNT(IDC_SEL_INFORM, cySubTotal);
		ADDAMOUNT(IDC_SEL_QUICKBOOKS, cySubTotal);
		ADDAMOUNT(IDC_SEL_EMR, cySubTotal);

		//At this point, subtotal & list total are the same -- none of the above options have
		//	any package pricing.  From here out, we calculate separately.
		cyListTotal = cySubTotal;

		ADDAMOUNT(IDC_SEL_SCHED, cySubTotal);

		// (d.thompson 2009-08-10) - PLID 35152 - NexWeb
		ADDAMOUNT(IDC_SEL_NEXWEB, cySubTotal);
		ADDAMOUNT(IDC_SEL_NEXWEB_ADDTL_DOMAINS, cySubTotal);

		//DRT 5/27/2008 - PLID 29493 - If this is a resident or startup package, and the hcfa
		//	is NOT set, and the letters IS set, then the cosmetic package does not exist, b/c
		//	the letters has been moved out of here.
		if( (m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) && !IsDlgButtonChecked(IDC_CHK_HCFA) && IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			//Add things up manually
			if(IsDlgButtonChecked(IDC_CHK_LETTERS))
				cySubTotal += m_aryPrices[ecpLetters];
			if(IsDlgButtonChecked(IDC_CHK_QUOTES))
				cySubTotal += m_aryPrices[ecpQuotes];
			if(IsDlgButtonChecked(IDC_CHK_TRACKING))
				cySubTotal += m_aryPrices[ecpTracking];
			if(IsDlgButtonChecked(IDC_CHK_NEXFORMS))
				cySubTotal += m_aryPrices[ecpNexForms];
		}
		else {
			//Normal proposal, or they're using HCFA
			ADDAMOUNT(IDC_SEL_COSMETIC, cySubTotal);
		}

		//DRT 5/27/2008 - PLID 29493 - If this is a resident or startup package, the financial
		//	package does not apply, because it is split -- billing & hcfa on the "package", 
		//	ebilling is a standalone addon.
		if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
			//add the pieces up
			if(IsDlgButtonChecked(IDC_CHK_BILLING))
				cySubTotal += m_aryPrices[ecpBilling];
			if(IsDlgButtonChecked(IDC_CHK_HCFA))
				cySubTotal += m_aryPrices[ecpHCFA];
			if(IsDlgButtonChecked(IDC_CHK_EBILLING))
				cySubTotal += m_aryPrices[ecpEBilling];
			// (d.lange 2010-12-06 18:19) - PLID 41711 - Moved ERemittance and EEligibility to Finanical area
			if(IsDlgButtonChecked(IDC_CHK_EREMITTANCE))
				cySubTotal += m_aryPrices[ecpERemittance];
			if(IsDlgButtonChecked(IDC_CHK_EELIGIBILITY))
				cySubTotal += m_aryPrices[ecpEEligibility];
		}
		else {
			//Normal usage
			ADDAMOUNT(IDC_SEL_FINANCIAL, cySubTotal);
		}


		{
			//This code is pretty much just a copy/paste of the 3 ReflectXXXPackage() functions
			if(IsDlgButtonChecked(IDC_CHK_SCHED)) {
				//DRT 9/18/2008 - PLID 31395
				if(IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
					cyListTotal += m_aryPrices[ecpSchedulerStandard];
				}
				else {
					cyListTotal += m_aryPrices[ecpScheduler];
				}
			}
			if(IsDlgButtonChecked(IDC_CHK_1LICENSE))
				cyListTotal += m_aryPrices[ecp1License];

			if(IsDlgButtonChecked(IDC_CHK_BILLING))
				cyListTotal += m_aryPrices[ecpBilling];
			if(IsDlgButtonChecked(IDC_CHK_HCFA))
				cyListTotal += m_aryPrices[ecpHCFA];
			if(IsDlgButtonChecked(IDC_CHK_EBILLING))
				cyListTotal += m_aryPrices[ecpEBilling];
			// (d.lange 2010-12-06 18:19) - PLID 41711 - Moved ERemittance and EEligibility to Finanical area
			if(IsDlgButtonChecked(IDC_CHK_EREMITTANCE))
				cyListTotal += m_aryPrices[ecpERemittance];
			if(IsDlgButtonChecked(IDC_CHK_EELIGIBILITY))
				cyListTotal += m_aryPrices[ecpEEligibility];

			if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
				cyListTotal += m_aryPrices[ecpLetters];
			}
			if(IsDlgButtonChecked(IDC_CHK_QUOTES))
				cyListTotal += m_aryPrices[ecpQuotes];
			if(IsDlgButtonChecked(IDC_CHK_TRACKING))
				cyListTotal += m_aryPrices[ecpTracking];
			//DRT 9/23/2008 - PLID 31395 - We do NOT need to worry about the Letters Standard transformation here, as
			//	this applies to the list price regardless.
			if(IsDlgButtonChecked(IDC_CHK_NEXFORMS))
				cyListTotal += m_aryPrices[ecpNexForms];
			// (d.thompson 2009-08-10) - PLID 35152 - NexWeb
			if(IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS))
				cyListTotal += m_aryPrices[ecpNexWebLeads];
			if(IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL))
				cyListTotal += m_aryPrices[ecpNexWebPortal];
			ADDAMOUNT(IDC_SEL_NEXWEB_ADDTL_DOMAINS, cyListTotal);
		}

		SetDlgItemText(IDC_SUBTOTAL_SEL, FormatCurrencyForInterface(cySubTotal));
	}

	//Fill in the subtotal list price.  Changing this value must force the support
	//	cost to change, as it is based on the subtotal.
	{
		SetDlgItemText(IDC_SUBTOTAL_LIST, FormatCurrencyForInterface(cyListTotal));
		//Update the support price field, since it is based off the list price
		UpdateSupportCosts();
	}

	//Fill in the subtotal package discount field -- the amount saved by getting some
	//	packages.
	{
		COleCurrency cyDiscount = cyListTotal - cySubTotal;
		SetDlgItemText(IDC_SUBTOTAL_PKG_DISCOUNTS, FormatCurrencyForInterface(cyDiscount));
	}

	//Discounts.  If the new subtotal is less than the old subtotal, we will change the discount dollar value
	//	to retain the old percentage.  This is to combat a sales rep giving an $x discount that is 10%, then
	//	the prospect removing a component, and that discount percentage jumping to 15%, etc.  If the package
	//	increases in price, we leave the discount as-is, and the percentage will fall lower.
	CString strTmpCurrentDiscount;
	GetDlgItemText(IDC_DISCOUNT_TOTAL_DOLLAR, strTmpCurrentDiscount);
	if((strTmpCurrentDiscount != FormatCurrencyForInterface(COleCurrency(0, 0))) && cyOldSubTotal > cySubTotal) {
		//Our package has DECREASED in cost
		//Get the current percentage 
		double dblPercent = 0.0;
		CString strTmp;
		GetDlgItemText(IDC_DISCOUNT_TOTAL_PERCENT, strTmp);
		dblPercent = atof(strTmp);
		dblPercent /= 100.0;

		//Now calculate the new discount dollar value from this percent and the subtotal
		COleCurrency cyNewAmt = cySubTotal * dblPercent;
		SetDlgItemText(IDC_DISCOUNT_TOTAL_DOLLAR, FormatCurrencyForInterface(cyNewAmt, TRUE));

		//Inform the user we just changed it
		MessageBox("You have just decreased your total package cost.  As a result, your given discount was lowered appropriately.  "
			"Please ensure the discount is as you intend.");
	}

	//Calculate the discount percentage.  This is the discount amount divided by the subtotal
	COleCurrency cyDiscountAmt(0, 0);
	{
		//TODO - Any better way to do this?  We lose a little precision here
		ADDAMOUNT(IDC_DISCOUNT_TOTAL_DOLLAR, cyDiscountAmt);
		double dblPercent = (double)cyDiscountAmt.m_cur.int64 / (double)cySubTotal.m_cur.int64;
		dblPercent *= 100.0;

		CString strFmt;
		strFmt.Format("%.2f%%", dblPercent);
		SetDlgItemText(IDC_DISCOUNT_TOTAL_PERCENT, strFmt);
	}

	//Update the support cost per month
	COleCurrency cyMonthlySupport(0, 0);
	{
		cyMonthlySupport = CalculateCurrentMonthlySupport();
		m_labelSupportMonthly.SetText(FormatCurrencyForInterface(cyMonthlySupport));
		GetDlgItem(IDC_LABEL_SUPPORT)->Invalidate();
	}

	//DRT 4/1/2008 - PLID 29493 - if this is a special package, we apply a flat discount to 
	//	the amount which is beyond the "base package".  Makes this calculation a little bit 
	//	tricky...
	COleCurrency cySpecialDiscounts(0, 0);
	if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {

		ADDAMOUNT(IDC_SPECIAL_DISCOUNT_DOLLAR, cySpecialDiscounts);

		COleCurrency cyModBase = cySubTotal;

		//Package piece:  scheduler & ws base
		if(IsDlgButtonChecked(IDC_CHK_SCHED) && IsDlgButtonChecked(IDC_CHK_1LICENSE)) {
			cyModBase -= m_aryPrices[ecpPkgSched];
		}
		//Package piece:  cosmetic billing
		if(IsDlgButtonChecked(IDC_CHK_BILLING)) {
			cyModBase -= m_aryPrices[ecpBilling];
		}
		//Package piece:  EITHER HCFA OR LW (HCFA takes precedence)
		if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
			cyModBase -= m_aryPrices[ecpHCFA];
		}
		else if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			cyModBase -= m_aryPrices[ecpLetters];
		}
		//Package piece:  1 workstation extra
		// (d.thompson 2012-10-12) - PLID 53155 - regular or TS license counts
		if(GetDlgItemInt(IDC_NUM_WORKSTATIONS) + GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS) >= 1) {
			cyModBase -= m_aryPrices[ecpWorkstation];
		}

		//Calculate the addon percentage based on our current type
		COleCurrency cySpecialAddOn = (cyModBase * ((double)(m_nTypeID == eptNexRes ? m_nResidentAddOnDiscount : m_nStartupAddOnDiscount) / 100.0));

		m_labelSpecialDiscountAddOn.SetText(FormatCurrencyForInterface(cySpecialAddOn));
		GetDlgItem(IDC_SPECIAL_DISCOUNT_ADDON)->Invalidate();
		cySpecialDiscounts += cySpecialAddOn;
	}


	//Now figure out discounts, training, travel, etc. to come up with the grand total
	COleCurrency cyGrandTotal = cySubTotal;
	{
		ADDAMOUNT(IDC_SEL_TRAINING, cyGrandTotal);
		ADDAMOUNT(IDC_SEL_SUPPORT, cyGrandTotal);
		ADDAMOUNT(IDC_SEL_CONVERSION, cyGrandTotal);
		// (d.lange 2010-11-11 16:29) - PLID 40361 - Add up the totals for EMR & Financial Conversions
		ADDAMOUNT(IDC_SEL_EMRCONVERSION, cyGrandTotal);
		ADDAMOUNT(IDC_SEL_FINCONVERSION, cyGrandTotal);
		ADDAMOUNT(IDC_SEL_TRAVEL, cyGrandTotal);
		//DRT 2/13/2008 - PLID 28905 - Added multi DB
		ADDAMOUNT(IDC_SEL_EXTRADB, cyGrandTotal);

		//Now subtract out discounts
		cyGrandTotal -= cyDiscountAmt;
		//DRT 4/1/2008 - PLID 29493 - Subtract special discounts
		cyGrandTotal -= cySpecialDiscounts;

		SetDlgItemText(IDC_GRAND_TOTAL, FormatCurrencyForInterface(cyGrandTotal));
	}
}

//DRT 5/27/2008 - PLID 29493 - This function calculates the current monthly support support value, basing it off the subtotal list price.
//	Now for Resident & Startup packages, this calculates the ADDON cost only.
COleCurrency COpportunityProposalDlg::CalculateCurrentMonthlySupport()
{
	//Determine the monthly support value (support percent * sub total / 12)
	long nSupportPercent = GetDlgItemInt(IDC_SUPPORT_PERCENT);		//Percent currently being used
	COleCurrency cySubTotal = GetCurrentSubTotalList();		//Get the current LIST PRICE subtotal, support is always based off this number

	//DRT 5/23/2008 - PLID 29493 - For resident & startup types, we are not working off the full subtotal, but instead
	//	off the subtotal of the addons (total minus whatever is on the standard form)
	if(m_nTypeID == eptNexStartup || m_nTypeID == eptNexRes) {
		//Subtract the subtotal list data on the standard form
		//	Sched + 1 license + billing + hcfa + 1 addtl ws
		cySubTotal -= m_aryPrices[ecpScheduler];
		cySubTotal -= m_aryPrices[ecp1License];
		cySubTotal -= m_aryPrices[ecpBilling];
		//Package piece:  EITHER HCFA OR LW (HCFA takes precedence)
		if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
			cySubTotal -= m_aryPrices[ecpHCFA];
		}
		else if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			cySubTotal -= m_aryPrices[ecpLetters];
		}

		cySubTotal -= m_aryPrices[ecpWorkstation];
	}

	// (a.walling 2007-11-07 09:12) - PLID 27998 - VS2008 - Need to disambiguate this
	// COleCurrency cyMonthlySupport = cySubTotal / 12 * nSupportPercent / 100;
	COleCurrency cyMonthlySupport = (((double)nSupportPercent / 100) * cySubTotal) / long(12);

	//Matt & Paul said we should always do a ceiling calculation (not round) on the support fees.  This keeps change out of the proposal, 
	//	and apparently avoids some problems with Quickbooks unable to properly handle .33 * 9 type calculations.
	__int64 nTotal = cyMonthlySupport.m_cur.int64;
	//the value is set 1/10,000th's
	__int64 nCents = nTotal % 10000;	//For example, 66 cents would be 6600
	__int64 nDollars = nTotal / 10000;

	if(nCents != 0)
		cyMonthlySupport.SetCurrency((long)(nDollars + 1), 0);

	return cyMonthlySupport;
}

//DRT 5/23/2008 - PLID 29493 - Continued from the above -- we may need to know what the support costs
//	are on the "main package" only, for NexResident & NexStartup packages.  This calculates the subtotal 
//	of just those pieces, then determines the support costs of that.
COleCurrency COpportunityProposalDlg::CalculateCurrentMonthlySupport_PackageOnly()
{
	if(m_nTypeID == eptNexStartup || m_nTypeID == eptNexRes) {
		//Determine the monthly support value (support percent * sub total / 12)
		long nSupportPercent = GetDlgItemInt(IDC_SUPPORT_PERCENT);		//Percent currently being used

		//The subtotal is just adding up these basic packages
		COleCurrency cySubTotal(0, 0);
		cySubTotal += m_aryPrices[ecpScheduler];
		cySubTotal += m_aryPrices[ecp1License];
		cySubTotal += m_aryPrices[ecpBilling];
		//If they don't have HCFA, use LW instead
		cySubTotal += (IsDlgButtonChecked(IDC_CHK_HCFA) ? m_aryPrices[ecpHCFA] : m_aryPrices[ecpLetters]);
		cySubTotal += m_aryPrices[ecpWorkstation];

		COleCurrency cyMonthlySupport = (((double)nSupportPercent / 100) * cySubTotal) / long(12);

		//Matt & Paul said we should always do a ceiling calculation (not round) on the support fees.  This keeps change out of the proposal, 
		//	and apparently avoids some problems with Quickbooks unable to properly handle .33 * 9 type calculations.
		__int64 nTotal = cyMonthlySupport.m_cur.int64;
		//the value is set 1/10,000th's
		__int64 nCents = nTotal % 10000;	//For example, 66 cents would be 6600
		__int64 nDollars = nTotal / 10000;

		if(nCents != 0)
			cyMonthlySupport.SetCurrency((long)(nDollars + 1), 0);

		return cyMonthlySupport;

	}
	else {
		//If it's not, just return 0
		return COleCurrency(0, 0);
	}
}

//Checks for invalid proposal options.  These are criteria defined by the users of things that need to be
//	required.
bool COpportunityProposalDlg::IsValidProposal()
{
	//Do things like make sure there are no modules purchased that aren't allowed combinations
	//	(ebilling w/o hcfa, for example).
	CString strFailureMessage;

	//HCFA Requires cosmetic billing
	if(IsDlgButtonChecked(IDC_CHK_HCFA) && !IsDlgButtonChecked(IDC_CHK_BILLING)) {
		strFailureMessage += " - You may not choose the Insurance Billing package without also selecting Cosmetic Billing.\r\n";
	}

	//E-Billing requires HCFA and Cosmetic.  We already checked HCFA->Cosmetic, so just need to verify HCFA here.
	if(IsDlgButtonChecked(IDC_CHK_EBILLING) && !IsDlgButtonChecked(IDC_CHK_HCFA)) {
		strFailureMessage += " - You may not choose the Electronic Billing package without also selecting the Insurance and Cosmetic Billing packages.\r\n";
	}

	//NexForms requires letter writing and NexTrak
	if(IsDlgButtonChecked(IDC_CHK_NEXFORMS)) {
		if(!IsDlgButtonChecked(IDC_CHK_TRACKING) || !IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			strFailureMessage += " - You may not choose the NexForms package without also selecting both NexTrak and Letter Writing packages.\r\n";
		}
	}

	//NexSpa requires cosmetic billing
	if(IsDlgButtonChecked(IDC_CHK_NEXSPA) && !IsDlgButtonChecked(IDC_CHK_BILLING)) {
		strFailureMessage += " - You may not choose the NexSpa package without also selecting Cosmetic Billing.\r\n";
	}

	//ASC requires inventory
	if(IsDlgButtonChecked(IDC_CHK_ASC) && !IsDlgButtonChecked(IDC_CHK_INV)) {
		strFailureMessage += " - You may not choose the ASC package without also selecting Inventory.\r\n";
	}

	//May not select both united and canfield
	if(IsDlgButtonChecked(IDC_CHK_MIRROR) && IsDlgButtonChecked(IDC_CHK_UNITED)) {
		strFailureMessage += " - You may not choose both the Mirror and United links on the same proposal.\r\n";
	}

	//DRT 2/12/2008 - PLID 28881 - ERemit and EElig require ebilling
	if(IsDlgButtonChecked(IDC_CHK_EREMITTANCE) && !IsDlgButtonChecked(IDC_CHK_EBILLING)) {
		strFailureMessage += " - You must select EBilling to include ERemittance.\r\n";
	}
	if(IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) && !IsDlgButtonChecked(IDC_CHK_EBILLING)) {
		strFailureMessage += " - You must select EBilling to include EEligibility.\r\n";
	}

	// (d.thompson 2009-03-26) - PLID 33704 - Standard EMR requires Letter Writing now.
	if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD) && GetDlgItemInt(IDC_NUM_EMR) > 0 && !IsDlgButtonChecked(IDC_CHK_LETTERS)) {
		strFailureMessage += " - You may not choose the EMR Standard package without also selecting Letter Writing.\r\n";
	}

	// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic EMR
	if(IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC)) {
		//This module has a restriction that the user cannot also buy ebilling, eremit, or elig
		if(m_aryModules[ecmEBilling] > 0 || m_aryModules[ecmERemittance] > 0 || m_aryModules[ecmEEligibility] > 0) {
			strFailureMessage += " - You may not choose Cosmetic EMR and also sell eBilling, eRemittance, or eEligibility.\r\n";
		}
	}

	//DRT 4/3/2008 - PLID 29493 - Ensure that if we're on a resident or startup special package, that all the appropriate modules
	//	are purchased.
	if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
		bool bFailed = false;

		//Require sched & 1 license
		if(!IsDlgButtonChecked(IDC_CHK_SCHED))
			bFailed = true;
		if(!IsDlgButtonChecked(IDC_CHK_1LICENSE))
			bFailed = true;

		//Require billing & (either hcfa or LW)
		if(!IsDlgButtonChecked(IDC_CHK_BILLING))
			bFailed = true;
		if(!IsDlgButtonChecked(IDC_CHK_HCFA) && !IsDlgButtonChecked(IDC_CHK_LETTERS))
			bFailed = true;

		//Require 1 addtl license
		// (d.thompson 2012-10-12) - PLID 53155 - regular or TS license counts
		if(GetDlgItemInt(IDC_NUM_WORKSTATIONS) + GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS) < 1)
			bFailed = true;

		if(bFailed) {
			strFailureMessage += " - You must select the Scheduler Package, Billing, HCFA or Letter Writing, and at least 1 workstation for this package.\r\n";
		}

		//DRT 4/3/2008 - PLID 29493 - Additionally, since HCFA and LW are interchangeable, if they select both, warn the user.  This will not cause a failure unless
		//	the user wishes to cancel.  This only applies to resident & startup packages.
		if(IsDlgButtonChecked(IDC_CHK_HCFA) && IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			if(MessageBox("You are using a special package, but have selected both HCFA and Letter Writing.  The HCFA module will be put toward the system price, and "
				"the LetterWriting module will be counted as an addon.\r\n\r\n"
				"Are you sure you wish to include both?", "Practice", MB_YESNO) == IDNO)
			{
				return false;
			}
		}
	}

	//DRT 9/24/2008 - PLID 31395 - Added Standard packages.  You could do some weird things like check "LW Standard" then uncheck LW, which would
	//	be confusing because that causes other changes to the package layouts.  We'll just confirm before saving that all is clear.
	if(IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
		//Ensure scheduler is checked
		if(!IsDlgButtonChecked(IDC_CHK_SCHED)) {
			strFailureMessage += " - If you select the 'Standard' option for the Scheduler, you must include the Scheduler.\r\n";
		}
	}
	if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
		//Ensure EMR is set
		if(GetDlgItemInt(IDC_NUM_EMR) <= 0) {
			strFailureMessage += " - If you select the 'Standard' option for EMR, you must include at least 1 EMR license.\r\n";
		}
	}

	// (d.thompson 2009-02-02) - PLID 32890 - Inv C&A requires inv
	if(IsDlgButtonChecked(IDC_CHK_C_AND_A) && !IsDlgButtonChecked(IDC_CHK_INV)) {
		strFailureMessage += " - If you select the C&A module, you must include the Inventory.\r\n";
	}

	//Expiration Rules:
	//	- The maximum expiration is 30 days.
	//	- If there is a discount applied, the maximum expiration is 14 days.
	//	- No expiration date may ever pass the end of the quarter, even if that
	//	date is tomorrow.
	{
		bool bHasDiscount = false;
		CString strTmp;
		GetDlgItemText(IDC_DISCOUNT_TOTAL_DOLLAR, strTmp);
		if(strTmp != FormatCurrencyForInterface(COleCurrency(0, 0)))
			bHasDiscount = true;

		//We must use the server time, not the local system time
		COleDateTime dtToday;
		{
			_RecordsetPtr prsToday = CreateRecordset("SELECT getdate() AS TodayDate");
			dtToday = AdoFldDateTime(prsToday, "TodayDate");
		}

		long nDaysToEndOfQuarter = CalculateDaysToEndOfQuarter(dtToday);

		COleDateTime dtLastValidExpiration = dtToday;
		long nAddDays = 0;
		if(nDaysToEndOfQuarter < 14)
			nAddDays = nDaysToEndOfQuarter;		//Cannot go past EOQ
		else if(bHasDiscount)
			nAddDays = 14;						//Cannot go past 2 weeks
		else if(nDaysToEndOfQuarter < 30)
			nAddDays += nDaysToEndOfQuarter;	//Cannot go past EOQ
		else 
			nAddDays = 30;						//30 days is absolute max

		dtLastValidExpiration += COleDateTimeSpan(nAddDays, 0, 0, 0);

		COleDateTime dtExpires = VarDateTime(m_pickerExpireDate.GetValue());
		if(dtExpires > dtLastValidExpiration) {
			//Not allowed
			strFailureMessage += " - You may not set an expiration date past:\r\n"
				"   - The end of the quarter.\r\n"
				"   - 14 days if there is a discount applied.\r\n"
				"   - 30 days if no discount is applied.";
		}
	}

	if(!strFailureMessage.IsEmpty() && m_nMergeCount == 0) {
		//something failed
		MessageBox("You may not save the proposal for the following reason(s):\r\n" + strFailureMessage);
		return false;
	}

	//If the doctor count and EMR count differ, warn the user.  This may be valid, and does not force a stop.
	//	Only if EMR is non zero
	{
		//DRT - The purchase of a system counts as 1 doctor automatically.  This number added is only for
		//	additional doctors ON TOP OF that one.  So we need to add 1 to the count.
		long nDocCount = GetDlgItemInt(IDC_NUM_DOCTORS) + 1;
		long nEMRCount = GetDlgItemInt(IDC_NUM_EMR);
		// (d.lange 2010-04-07) - PLID 37999 - don't show message more than once if its an EMR split
		if(nEMRCount > 0 && nDocCount != nEMRCount && m_nMergeCount == 0) {
			if(MessageBox("The number of selected doctors does not match the number of selected EMR doctors.  Are you sure you have entered the right values?", "Warning", MB_YESNO) != IDYES)
				return false;
		}

		// (z.manning, 11/26/2007) - Make sure that there's no EMR training days if they did not buy EMR.
		unsigned int nEmrTrainingDays = GetDlgItemInt(IDC_NUM_EMR_TRAINING);
		if(nEmrTrainingDays > 0 && nEMRCount <= 0) {
			MessageBox("You may not have any EMR training days because this proposal does not contain any EMR doctors.");
			return false;
		}
	}


	//Passed all criteria, proposal is valid
	return true;
}

//helper for SQL output, return value is able to be put into a BIT data type
#define BitFormat(idc) IsDlgButtonChecked(idc) ? 1 : 0

bool COpportunityProposalDlg::ApplyChanges()
{
	//First, ensure that the proposal does not have any invalid data.
	if(!IsValidProposal())
		return false;

	//The proposal is valid, now we need to check for various data conditions that are invalid.  These are more technical
	//	requirements, things like empty text boxes, invalid date values, etc.
	//TODO:  I couldn't actually think of any

	long nWorkstations, nDoctors, nPDA, nNexSync, nEMR, nTraining, nEmrTraining, nSupport, nConversion, nTravel, nExtraDB,
		nNexWebAddtlDomains, nEMRWorkstations, nEmrConversion, nFinancialConversion, nWorkstationsTS;

	nWorkstations = GetDlgItemInt(IDC_NUM_WORKSTATIONS);
	// (d.thompson 2012-10-12) - PLID 53155
	nWorkstationsTS = GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS);
	nDoctors = GetDlgItemInt(IDC_NUM_DOCTORS);
	nPDA = GetDlgItemInt(IDC_NUM_PDA);
	// (d.thompson 2009-11-13) - PLID 36124
	nNexSync = GetDlgItemInt(IDC_NUM_NEXSYNC);
	nEMR = GetDlgItemInt(IDC_NUM_EMR);
	nTraining = GetDlgItemInt(IDC_NUM_TRAINING);
	nEmrTraining = GetDlgItemInt(IDC_NUM_EMR_TRAINING); // (z.manning, 11/26/2007) - PLID 28159
	nSupport = GetDlgItemInt(IDC_NUM_SUPPORT);
	nConversion = GetDlgItemInt(IDC_NUM_CONVERSION);
	nTravel = GetDlgItemInt(IDC_NUM_TRAVEL);
	nExtraDB = GetDlgItemInt(IDC_NUM_EXTRADB);			//DRT 2/13/2008 - PLID 28905
	nNexWebAddtlDomains = GetDlgItemInt(IDC_NUM_NEXWEB_ADDTL_DOMAINS);		// (d.thompson 2009-08-10) - PLID 35152
	nEMRWorkstations = GetDlgItemInt(IDC_NUM_EMRWORKSTATIONS);				// (d.lange 2010-04-06) - PLID 37999 - save the EMR Workstations qty
	nEmrConversion = GetDlgItemInt(IDC_NUM_EMRCONVERSION);					// (d.lange 2010-11-11 16:40) - PLID 40361 - Added EMR Conversion
	nFinancialConversion = GetDlgItemInt(IDC_NUM_FINCONVERSION);			// (d.lange 2010-11-11 16:40) - PLID 40361 - Added EMR Conversion

	//
	//Now do the actual saving work.  Always a new entry.
	//
	CString strMailSentID = "NULL";
	if(m_nMailSentID != -1)
		strMailSentID.Format("%li", m_nMailSentID);

	//Get the discount amount - we don't care about the %
	CString strDiscount;
	GetDlgItemText(IDC_DISCOUNT_TOTAL_DOLLAR, strDiscount);
	CString strDiscountUser = m_nCurrentDiscountUserID == -1 ? "NULL" : FormatString("%li", m_nCurrentDiscountUserID);

	//DRT 4/3/2008 - For resident & startup types, the discount is a specially tracked user.  It is not possible to have a "standard discount" applied.
	if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
		//Add the package discount + special addon discount
		COleCurrency cySpecialDisc(0, 0);
		ADDAMOUNT(IDC_SPECIAL_DISCOUNT_DOLLAR, cySpecialDisc);
		//Oddly, NxLabel controls don't allow you to get their text with GetDlgItemText(), so we'll have to do this manually
		{
			COleCurrency cyTmpSpecial(0, 0);
			cyTmpSpecial.ParseCurrency(m_labelSpecialDiscountAddOn.GetText());
			cySpecialDisc += cyTmpSpecial;
		}
		
		strDiscount = FormatCurrencyForSql(cySpecialDisc);

		//This is hardcoded to the 'Special Discount' user in internal.  It's inactive to keep people away from it.
		strDiscountUser = "19652";
	}

	//DRT - In the efforts of efficiency, we're going to actually save the proposal total at the time of saving the proposal.  This makes calculations
	//	and reports infinitely easier.  At this time, users cannot ever edit the proposal data, so it never needs updating.
	CString strTotal;
	GetDlgItemText(IDC_GRAND_TOTAL, strTotal);

	//Only on new proposals
	long nQuoteNum = -1;
	if(m_nID == -1) {
		//We are following the "new patient" system.  We assign the next available quote ID in the init dialog, and if that
		//	ID is still unused, we just save.  If it has been used in the meantime, we just tell the user and change the ID to
		//	the next available.  To cut down on recordset access, we're just going to query the next avail and count that way.
		long nNextAvail = NewNumber("OpportunityProposalsT", "QuoteNum");
		nQuoteNum = GetDlgItemInt(IDC_QUOTE_ID);
		if(nQuoteNum != nNextAvail) {
			//Someone else used it and saved
			MessageBox("The current quote number has been used by another user.  This quote number will be changed to the next available.");
			nQuoteNum = nNextAvail;
			SetDlgItemInt(IDC_QUOTE_ID, nQuoteNum);
		}
	}

	CString strSql;
	if(m_nID == -1) {
		//New proposal
		// (z.manning, 11/26/2007) - PLID 28159 - Added EmrTraining field.
		//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
		//DRT 2/13/2008 - PLID 28905 - Added multi DB
		//DRT 9/22/2008 - PLID 31395 - Added 'Standard' editions
		// (d.thompson 2009-02-02) - PLID 32890 - Added C&A
		// (d.thompson 2009-08-10) - PLID 35152 - NexWeb
		// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
		// (d.thompson 2009-11-13) - PLID 36124 - NexSync
		// (d.lange 2010-3-31) - PLID 37956 - added SplitEMR & EMRWorkstations
		// (d.lange 2010-04-08) - PLID 38096 - added ApplyEMRDiscount
		// (d.lange 2010-09-21 13:02) - PLID 40361 - Added EMR & Financial Conversion
		// (d.lange 2010-12-21 17:43) - PLID 41889 - Added TravelType
		// (d.thompson 2012-10-12) - PLID 53155 - Added workstations TS
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic EMR
		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		strSql.Format("DECLARE @newID int;\r\n"
			"INSERT INTO OpportunityProposalsT (OpportunityID, MailSentID, ProposalDate, ExpiresOn, QuoteNum, CreatedByUserID, "
			"Scheduler, LicenseSched, Billing, HCFA, EBilling, Letters, Quotes, Tracking, NexForms, Inventory, NexSpa, NexASC, Mirror, United, "
			"HL7, Inform, Quickbooks, ERemittance, EEligibility, "
			"Workstations, Doctors, PDA, EMR, Training, EmrTraining, Support, Conversion, Travel, ExtraDB, PriceStructureID, "
			"SavedTotal, DiscountAmount, DiscountedBy, IsSchedStandard, IsLettersStandard, IsEMRStandard, CandA, "
			"NexWebLeads, NexWebPortal, NexWebAddtlDomains, NexPhoto, NexSync, SplitEMR, EMRWorkstations, ApplyEMRDiscount, EMRConversion, FinancialConversion, TravelType, "
			"WorkstationsTS, IsEMRCosmetic, HIELink) values "
			"(%li, %s, '%s', '%s', %li, %li, %li, %li, %li, %li, %li, "	//through ebilling
			"%li, %li, %li, %li, "	//through nexforms
			"%li, %li, %li, %li, %li, "	//through united
			"%li, %li, %li, %li, %li, "	//through eeligibility
			"%li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, convert(money, '%s'), convert(money, '%s'), %s, %li, %li, %li, %li, "
			"%li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, "
			"%li, %li, %li);\r\n"
			"SET @newID = (SELECT @@identity);\r\n", 
			m_nOpportunityID, strMailSentID, FormatDateTimeForSql(VarDateTime(m_pickerProposalDate.GetValue()), dtoDate), 
			FormatDateTimeForSql(VarDateTime(m_pickerExpireDate.GetValue()), dtoDate), nQuoteNum, GetCurrentUserID(), 
			BitFormat(IDC_CHK_SCHED), BitFormat(IDC_CHK_1LICENSE), BitFormat(IDC_CHK_BILLING), BitFormat(IDC_CHK_HCFA), BitFormat(IDC_CHK_EBILLING), 
			BitFormat(IDC_CHK_LETTERS), BitFormat(IDC_CHK_QUOTES), BitFormat(IDC_CHK_TRACKING), BitFormat(IDC_CHK_NEXFORMS), 
			BitFormat(IDC_CHK_INV), BitFormat(IDC_CHK_NEXSPA), BitFormat(IDC_CHK_ASC), BitFormat(IDC_CHK_MIRROR), BitFormat(IDC_CHK_UNITED), 
			BitFormat(IDC_CHK_HL7), BitFormat(IDC_CHK_INFORM), BitFormat(IDC_CHK_QUICKBOOKS), BitFormat(IDC_CHK_EREMITTANCE), BitFormat(IDC_CHK_EELIGIBILITY), 
			nWorkstations, nDoctors, nPDA, nEMR, nTraining, nEmrTraining, nSupport, nConversion, nTravel, nExtraDB, m_nPriceStructureID, strTotal, strDiscount, 
			strDiscountUser, BitFormat(IDC_CHK_SCHED_STANDARD), 0/*PLID 32143 - Removed LW Std*/, BitFormat(IDC_CHK_EMR_STANDARD), 
			BitFormat(IDC_CHK_C_AND_A), BitFormat(IDC_CHK_NEXWEB_LEADS), BitFormat(IDC_CHK_NEXWEB_PORTAL), nNexWebAddtlDomains,
			BitFormat(IDC_CHK_NEXPHOTO), nNexSync, BitFormat(IDC_CHK_SPLITEMR), nEMRWorkstations, BitFormat(IDC_CHK_DISCOUNT_EMR), nEmrConversion, 
			nFinancialConversion, m_nTravelType, nWorkstationsTS, BitFormat(IDC_CHK_EMR_COSMETIC), BitFormat(IDC_CHK_HIE));
	}
	else {
		// (z.manning, 11/26/2007) - PLID 28159 - Added EmrTraining field.
		//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
		//DRT 2/13/2008 - PLID 28905 - Added multi DB
		//DRT 9/22/2008 - PLID 31395 - Added 'Standard' editions
		//Modifying an existing proposal - I left out fields that cannot change
		// (d.thompson 2009-02-02) - PLID 32890 - C&A
		// (d.thompson 2009-08-10) - PLID 35152 - NexWeb
		// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
		// (d.thompson 2009-11-13) - PLID 36124 - NexSync
		// (d.lange 2010-3-31) - PLID 37956 - added SplitEMR
		// (d.lange 2010-04-08) - PLID 38096 - added ApplyEMRDiscount
		// (d.lange 2010-09-21 13:03) - PLID 40361 - Added EMR & Financial Conversion
		// (d.lange 2010-12-21 17:43) - PLID 41889 - Added TravelType
		// (d.thompson 2012-10-12) - PLID 53155 - Added workstations TS
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic EMR
		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		strSql.Format("UPDATE OpportunityProposalsT SET MailSentID = %s, ProposalDate = '%s', ExpiresOn = '%s', "
			"Scheduler = %li, LicenseSched = %li, Billing = %li, HCFA = %li, EBilling = %li, Letters = %li, Quotes = %li, Tracking = %li, NexForms = %li, "
			"Inventory = %li, NexSpa = %li, NexASC = %li, Mirror = %li, United = %li, HL7 = %li, Inform = %li, Quickbooks = %li, "
			"ERemittance = %li, EEligibility = %li, "
			"Workstations = %li, Doctors = %li, PDA = %li, EMR = %li, Training = %li, EmrTraining = %li, Support = %li, Conversion = %li, Travel = %li, "
			"ExtraDB = %li, "
			"SavedTotal = convert(money, '%s'), DiscountAmount = convert(money, '%s'), DiscountedBy = %s, "
			"IsSchedStandard = %li, IsLettersStandard = %li, IsEMRStandard = %li, CandA = %li, "
			"NexWebLeads = %li, NexWebPortal = %li, NexWebAddtlDomains = %li, NexPhoto = %li, NexSync = %li, SplitEMR = %li , EMRWorkstations = %li, "
			"ApplyEMRDiscount = %li, EMRConversion = %li, FinancialConversion = %li, TravelType = %li, WorkstationsTS = %li, IsEMRCosmetic = %li, "
			"HIELink = %li "
			"WHERE ID = %li;\r\n", 
			strMailSentID, FormatDateTimeForSql(VarDateTime(m_pickerProposalDate.GetValue()), dtoDate), 
			FormatDateTimeForSql(VarDateTime(m_pickerExpireDate.GetValue()), dtoDate), 
			BitFormat(IDC_CHK_SCHED), BitFormat(IDC_CHK_1LICENSE), BitFormat(IDC_CHK_BILLING), BitFormat(IDC_CHK_HCFA), BitFormat(IDC_CHK_EBILLING), 
			BitFormat(IDC_CHK_LETTERS), BitFormat(IDC_CHK_QUOTES), BitFormat(IDC_CHK_TRACKING), BitFormat(IDC_CHK_NEXFORMS), 
			BitFormat(IDC_CHK_INV), BitFormat(IDC_CHK_NEXSPA), BitFormat(IDC_CHK_ASC), BitFormat(IDC_CHK_MIRROR), BitFormat(IDC_CHK_UNITED), 
			BitFormat(IDC_CHK_HL7), BitFormat(IDC_CHK_INFORM), BitFormat(IDC_CHK_QUICKBOOKS), BitFormat(IDC_CHK_EREMITTANCE), BitFormat(IDC_CHK_EELIGIBILITY), 
			nWorkstations, nDoctors, nPDA, nEMR, nTraining, nEmrTraining, nSupport, nConversion, nTravel, nExtraDB, strTotal, strDiscount, 
			strDiscountUser, BitFormat(IDC_CHK_SCHED_STANDARD), 0/*PLID 32143 - Removed LW Std*/, BitFormat(IDC_CHK_EMR_STANDARD), 
			BitFormat(IDC_CHK_C_AND_A), BitFormat(IDC_CHK_NEXWEB_LEADS), BitFormat(IDC_CHK_NEXWEB_PORTAL), nNexWebAddtlDomains, 
			BitFormat(IDC_CHK_NEXPHOTO), nNexSync, BitFormat(IDC_CHK_SPLITEMR), nEMRWorkstations, BitFormat(IDC_CHK_DISCOUNT_EMR), 
			nEmrConversion, nFinancialConversion, m_nTravelType, nWorkstationsTS, BitFormat(IDC_CHK_EMR_COSMETIC), BitFormat(IDC_CHK_HIE), m_nID);
	}

	//Anytime we save a proposal, that proposal immediately becomes the active one.  The user can change this manually if they so desire.
	//	We additionally need to update the "estimated price" field.  We attempt to keep it in sync with the active proposal.
	if(m_nID == -1) {
		strSql += FormatString("UPDATE OpportunitiesT SET ActiveProposalID = @newID, EstPrice = convert(money, '%s') WHERE ID = %li;\r\n", _Q(strTotal), m_nOpportunityID);
	}
	else {
		strSql += FormatString("UPDATE OpportunitiesT SET ActiveProposalID = %li, EstPrice = convert(money, '%s') WHERE ID = %li;\r\n", m_nID, _Q(strTotal), m_nOpportunityID);
	}

	//Wrap it with a selection to get the ID back
	CString strFmt;
	strFmt.Format("SET NOCOUNT ON;\r\n"
		"%s"
		"SET NOCOUNT OFF;\r\n"
		"%s", strSql, m_nID == -1 ? "SELECT convert(int, @newID) AS NewID;\r\n" : "");

	//Do all work in a transaction
	bool bRetVal = false;
	BEGIN_TRANS("Error in ApplyChanges")

		_RecordsetPtr prsExec = CreateRecordsetStd(strFmt);

		if(m_nID == -1) {
			//Pull out the identity value, if this is a new 
			m_nID = AdoFldLong(prsExec, "NewID");
		}

		//If we got here, success!
		bRetVal = true;
	END_TRANS_CATCH_ALL("Error in ApplyChanges");

	return bRetVal;
}

void COpportunityProposalDlg::OnOK() 
{
	try {
		if(ApplyChanges()) {
			//Successfully saved, do any cleanup here

			//Success, we may close
			CDialog::OnOK();
		}
		else {
			//Failed to save.  We must quit here, dialog remains open
			return;
		}
	} NxCatchAll("Error in OnOK");
}

void COpportunityProposalDlg::OnCancel() 
{
	try {
		//TODO:  Do any cleanup here


		CDialog::OnCancel();
	} NxCatchAll("Error in OnCancel");
}


///////////////////////////////
//	Package deals
void COpportunityProposalDlg::ReflectSchedulerPackage()
{
	COleCurrency cyTotal(0, 0);

	//If they have both scheduler + 1 license, it counts as a package
	if(IsDlgButtonChecked(IDC_CHK_SCHED) && IsDlgButtonChecked(IDC_CHK_1LICENSE)) {
		//DRT 9/18/2008 - PLID 31395
		if(IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
			cyTotal = m_aryPrices[ecpPkgSchedStandard];
		}
		else {
			cyTotal = m_aryPrices[ecpPkgSched];
		}
	}
	else {
		//Not all, add up the individuals
		if(IsDlgButtonChecked(IDC_CHK_SCHED)) {
			//DRT 9/18/2008 - PLID 31395
			if(IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
				cyTotal += m_aryPrices[ecpSchedulerStandard];
			}
			else {
				cyTotal += m_aryPrices[ecpScheduler];
			}
		}
		if(IsDlgButtonChecked(IDC_CHK_1LICENSE))
			cyTotal += m_aryPrices[ecp1License];
	}

	SetDlgItemText(IDC_SEL_SCHED, FormatCurrencyForInterface(cyTotal));
}

void COpportunityProposalDlg::ReflectFinancialPackage()
{
	COleCurrency cyTotal(0, 0);

	//If they have all 3 of the financial pieces, they get package pricing
	BOOL bBilling = IsDlgButtonChecked(IDC_CHK_BILLING);
	BOOL bHCFA = IsDlgButtonChecked(IDC_CHK_HCFA);
	BOOL bEBilling = IsDlgButtonChecked(IDC_CHK_EBILLING);
	// (d.lange 2010-12-03 09:28) - PLID 41711 - Moved E-Eligibility and E-Remittance to financial area
	BOOL bEEligibility = IsDlgButtonChecked(IDC_CHK_EELIGIBILITY);
	BOOL bERemittance = IsDlgButtonChecked(IDC_CHK_EREMITTANCE);

	//DRT 5/27/2008 - PLID 29493 - In resident & startup packages, the financial discount does not
	//	apply.
	if(bBilling && bHCFA && bEBilling && bEEligibility && bERemittance && !(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup)) {
		//package cost
		cyTotal = m_aryPrices[ecpPkgFinancial];
	}
	else {
		//Use individual pricing
		if(bBilling)
			cyTotal += m_aryPrices[ecpBilling];
		if(bHCFA)
			cyTotal += m_aryPrices[ecpHCFA];
		if(bEBilling)
			cyTotal += m_aryPrices[ecpEBilling];
		// (d.lange 2010-12-03 09:31) - PLID 41711 - Moved E-Eligibility and E-Remittance to financial area
		if(bEEligibility)
			cyTotal += m_aryPrices[ecpEEligibility];
		if(bERemittance)
			cyTotal += m_aryPrices[ecpERemittance];
	}

	SetDlgItemText(IDC_SEL_FINANCIAL, FormatCurrencyForInterface(cyTotal));
}

void COpportunityProposalDlg::ReflectCosmeticPackage()
{
	COleCurrency cyTotal(0, 0), cyNexFormsTotal(0, 0);

	//If they have all 4, use the package pricing
	BOOL bLetters = IsDlgButtonChecked(IDC_CHK_LETTERS);
	BOOL bQuotes = IsDlgButtonChecked(IDC_CHK_QUOTES);
	BOOL bTracking = IsDlgButtonChecked(IDC_CHK_TRACKING);
	BOOL bNexForms = IsDlgButtonChecked(IDC_CHK_NEXFORMS);

	//DRT 5/27/2008 - PLID 29493 - When on resident & startup packages, if hcfa is not set and letters is, the cosmetic package vanishes.
	bool bAllowed = true;
	if( (m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) && !IsDlgButtonChecked(IDC_CHK_HCFA) && bLetters) {
		bAllowed = false;
	}

	if(bLetters && bQuotes && bTracking && bNexForms && bAllowed) {
		//All 4 are hit, we have package cost
		cyTotal = m_aryPrices[ecpPkgCosmetic];
	}
	else {
		if(bLetters) {
			cyTotal += m_aryPrices[ecpLetters];
		}
		if(bQuotes)
			cyTotal += m_aryPrices[ecpQuotes];
		if(bTracking)
			cyTotal += m_aryPrices[ecpTracking];
		if(bNexForms)
			cyTotal += m_aryPrices[ecpNexForms];
	}

	SetDlgItemText(IDC_SEL_COSMETIC, FormatCurrencyForInterface(cyTotal));

	//DRT 5/27/2008 - PLID 29493 - If we've hit our !hcfa && letters flag, then we need to change the "package" price as well
	//	to not confuse people.  This only happens once HCFA is cleared.
	//DRT 9/23/2008 - No PLID - I noticed this while fixing something else... this should be nexres/startup only, otherwise
	//	it causes the cosmetic package price to get blanked out on normal proposals.
	if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
		if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
			SetDlgItemText(IDC_PKG_COSMETIC, FormatCurrencyForInterface(m_aryPrices[ecpPkgCosmetic]));
		}
		else {
			SetDlgItemText(IDC_PKG_COSMETIC, FormatCurrencyForInterface(cyTotal));
		}
	}
}

void COpportunityProposalDlg::ReflectOtherModules()
{
	//There is no package pricing for "other", this just adds them up and puts them in 1 box
	COleCurrency cyTotal(0, 0);

	if(IsDlgButtonChecked(IDC_CHK_INV))
		cyTotal += m_aryPrices[ecpInventory];
	if(IsDlgButtonChecked(IDC_CHK_NEXSPA))
		cyTotal += m_aryPrices[ecpSpa];
	if(IsDlgButtonChecked(IDC_CHK_ASC))
		cyTotal += m_aryPrices[ecpASC];
	//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
	// (d.lange 2010-12-03 09:42) - PLID 41711 - Moved ERemittance and EEligibility to financial area
	/*if(IsDlgButtonChecked(IDC_CHK_EREMITTANCE)) {
		cyTotal += m_aryPrices[ecpERemittance];
	}
	if(IsDlgButtonChecked(IDC_CHK_EELIGIBILITY)) {
		cyTotal += m_aryPrices[ecpEEligibility];
	}*/
	// (d.thompson 2009-01-30) - PLID 32890 - Added Consignment and Allocation module
	if(IsDlgButtonChecked(IDC_CHK_C_AND_A)) {
		cyTotal += m_aryPrices[ecpCandA];
	}
	// (d.thompson 2009-11-06) - PLID 36123 - Added NexPhoto
	if(IsDlgButtonChecked(IDC_CHK_NEXPHOTO)) {
		cyTotal += m_aryPrices[ecpNexPhoto];
	}

	SetDlgItemText(IDC_SEL_OTHER, FormatCurrencyForInterface(cyTotal));
}

// (d.thompson 2009-08-10) - PLID 35152 - Added NexWeb
void COpportunityProposalDlg::ReflectNexWebPackage()
{
	COleCurrency cyTotal(0, 0);

	//If they buy both modules, they get package pricing
	BOOL bLeads = IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS);
	BOOL bPortal = IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL);

	if(bLeads && bPortal) {
		//package cost
		cyTotal = m_aryPrices[ecpPkgNexWeb];
	}
	else {
		//Use individual pricing
		if(bLeads)
			cyTotal += m_aryPrices[ecpNexWebLeads];
		if(bPortal)
			cyTotal += m_aryPrices[ecpNexWebPortal];
	}

	SetDlgItemText(IDC_SEL_NEXWEB, FormatCurrencyForInterface(cyTotal));
}

// end packages
///////////////////////////////

void COpportunityProposalDlg::OnChkSched() 
{
	try {
		//Handle this particular package
		ReflectSchedulerPackage();

		//This affects training
		UpdateTraining();

		//Update the totals
		UpdateAllTotals();

		m_aryModules[ecmScheduler] = (IsDlgButtonChecked(IDC_CHK_SCHED) ? 1 : 0);

	} NxCatchAll("Error in OnChkSched");
}

void COpportunityProposalDlg::OnChk1license() 
{
	try {
		//Handle this particular package
		ReflectSchedulerPackage();

		//Update the totals
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecm1License] = (IsDlgButtonChecked(IDC_CHK_1LICENSE) ? 1 : 0);

	} NxCatchAll("Error in OnChk1license");
}

void COpportunityProposalDlg::OnChkBilling() 
{
	try {
		//Handle this package
		ReflectFinancialPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmBilling] = (IsDlgButtonChecked(IDC_CHK_BILLING) ? 1 : 0);

	} NxCatchAll("Error in OnChkBilling");
}

void COpportunityProposalDlg::OnChkHcfa() 
{
	try {
		//Handle this package
		ReflectFinancialPackage();

		//DRT 5/27/2008 - PLID 29493 - For resident & startup packages, this
		//	can trigger changes in the cosmetic package.
		if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
			ReflectCosmeticPackage();
		}

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmHCFA] = (IsDlgButtonChecked(IDC_CHK_HCFA) ? 1 : 0);

	} NxCatchAll("Error in OnChkHcfa");
}

void COpportunityProposalDlg::OnChkEbilling() 
{
	try {
		//Handle this package
		ReflectFinancialPackage();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmEBilling] = (IsDlgButtonChecked(IDC_CHK_EBILLING) ? 1 : 0);

	} NxCatchAll("Error in OnChkEbilling");
}

void COpportunityProposalDlg::OnChkLetters() 
{
	try {
		//Handle this package
		ReflectCosmeticPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmLetterWriting] = (IsDlgButtonChecked(IDC_CHK_LETTERS) ? 1 : 0);

	} NxCatchAll("Error in OnChkLetters");
}

void COpportunityProposalDlg::OnChkQuotes() 
{
	try {
		//Handle this package
		ReflectCosmeticPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmQuotesMarketing] = (IsDlgButtonChecked(IDC_CHK_QUOTES) ? 1 : 0);

	} NxCatchAll("Error in OnChkQuotes");
}

void COpportunityProposalDlg::OnChkTracking() 
{
	try {
		//Handle this package
		ReflectCosmeticPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmNexTrak] = (IsDlgButtonChecked(IDC_CHK_TRACKING) ? 1 : 0);

	} NxCatchAll("Error in OnChkTracking");
}

void COpportunityProposalDlg::OnChkNexforms() 
{
	try {
		//Handle this package
		ReflectCosmeticPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmNexForms] = (IsDlgButtonChecked(IDC_CHK_NEXFORMS) ? 1 : 0);

	} NxCatchAll("Error in OnChkNexforms");
}

void COpportunityProposalDlg::OnChkInv() 
{
	try {
		//Handle this "group" (this is not a package cost set)
		ReflectOtherModules();

		//DRT 6/10/2008 - PLID 30346 - This affects training (always should have, fixed it along with
		//	my item because it was required there as well).
		UpdateTraining();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmInventory] = (IsDlgButtonChecked(IDC_CHK_INV) ? 1 : 0);

	} NxCatchAll("Error in OnChkInv");
}

//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
void COpportunityProposalDlg::OnChkEremittance() 
{
	try {
		//Handle this "group" (this is not a package cost set)
		//ReflectOtherModules();
		// (d.lange 2010-12-03 09:08) - PLID 41711 - Moved E-Remittance to the Financial area
		ReflectFinancialPackage();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmERemittance] = (IsDlgButtonChecked(IDC_CHK_EREMITTANCE) ? 1 : 0);

	} NxCatchAll("Error in OnChkEremittance");
}

//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
void COpportunityProposalDlg::OnChkEeligibility() 
{
	try {
		//Handle this "group" (this is not a package cost set)
		//ReflectOtherModules();
		// (d.lange 2010-12-03 09:08) - PLID 41711 - Moved E-Remittance to the Financial area
		ReflectFinancialPackage();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmEEligibility] = (IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) ? 1 : 0);

	} NxCatchAll("Error in OnChkEeligibility");
}

void COpportunityProposalDlg::OnChkNexspa() 
{
	try {
		//Handle this "group" (this is not a package cost set)
		ReflectOtherModules();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmNexSpa] = (IsDlgButtonChecked(IDC_CHK_NEXSPA) ? 1 : 0);

	} NxCatchAll("Error in OnChkNexspa");
}

void COpportunityProposalDlg::OnChkAsc() 
{
	try {
		//Handle this "group" (this is not a package cost set)
		ReflectOtherModules();

		//This affects training
		UpdateTraining();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmASC] = (IsDlgButtonChecked(IDC_CHK_ASC) ? 1 : 0);

	} NxCatchAll("Error in OnChkAsc");
}

void COpportunityProposalDlg::OnChkMirror() 
{
	try {
		//Just update this row
		COleCurrency cy(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_MIRROR))
			cy += m_aryPrices[ecpMirror];

		SetDlgItemText(IDC_SEL_MIRROR, FormatCurrencyForInterface(cy));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmMirror] = (IsDlgButtonChecked(IDC_CHK_MIRROR) ? 1 : 0);

	} NxCatchAll("Error in OnChkMirror");
}

void COpportunityProposalDlg::OnChkUnited() 
{
	try {
		//Just update this row
		COleCurrency cy(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_UNITED))
			cy += m_aryPrices[ecpUnited];

		SetDlgItemText(IDC_SEL_UNITED, FormatCurrencyForInterface(cy));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmUnited] = (IsDlgButtonChecked(IDC_CHK_UNITED) ? 1 : 0);

	} NxCatchAll("Error in OnChkUnited");
}

void COpportunityProposalDlg::OnChkHl7() 
{
	try {
		//Just update this row
		COleCurrency cy(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_HL7))
			cy += m_aryPrices[ecpHL7];

		SetDlgItemText(IDC_SEL_HL7, FormatCurrencyForInterface(cy));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmHL7] = (IsDlgButtonChecked(IDC_CHK_HL7) ? 1 : 0);

	} NxCatchAll("Error in OnChkHl7");
}

// (d.thompson 2013-07-08) - PLID 57334
void COpportunityProposalDlg::OnChkHIE() 
{
	try {
		//Just update this row
		COleCurrency cy(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_HIE))
			cy += m_aryPrices[ecpHIE];

		SetDlgItemText(IDC_SEL_HIE, FormatCurrencyForInterface(cy));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmHIE] = (IsDlgButtonChecked(IDC_CHK_HIE) ? 1 : 0);

	} NxCatchAll(__FUNCTION__);
}

void COpportunityProposalDlg::OnChkInform() 
{
	try {
		//Just update this row
		COleCurrency cy(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_INFORM))
			cy += m_aryPrices[ecpInform];

		SetDlgItemText(IDC_SEL_INFORM, FormatCurrencyForInterface(cy));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmInform] = (IsDlgButtonChecked(IDC_CHK_INFORM) ? 1 : 0);

	} NxCatchAll("Error in OnChkInform");
}

void COpportunityProposalDlg::OnChkQuickbooks() 
{
	try {
		//Just update this row
		COleCurrency cy(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_QUICKBOOKS))
			cy += m_aryPrices[ecpQuickbooks];

		SetDlgItemText(IDC_SEL_QUICKBOOKS, FormatCurrencyForInterface(cy));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmQuickBooks] = (IsDlgButtonChecked(IDC_CHK_QUICKBOOKS) ? 1 : 0);

	} NxCatchAll("Error in OnChkQuickbooks");
}

void COpportunityProposalDlg::OnChangeNumWorkstations() 
{
	try {
		//We set the value to the number of workstations * the price per
		long nValue = GetDlgItemInt(IDC_NUM_WORKSTATIONS);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpWorkstation];

		SetDlgItemText(IDC_SEL_WORKSTATIONS, FormatCurrencyForInterface(cyTotal));

		//This affects training
		UpdateTraining();

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmWorkstations] = GetDlgItemInt(IDC_NUM_WORKSTATIONS);

	} NxCatchAll("Error in OnChangeNumWorkstations");
}

// (d.thompson 2012-10-12) - PLID 53155 - TS Workstations.  Continue to use the normal workstation pricing
void COpportunityProposalDlg::OnChangeNumWorkstationsTS() 
{
	try {
		//We set the value to the number of workstations * the price per
		long nValue = GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpWorkstation];

		SetDlgItemText(IDC_SEL_WORKSTATIONS_TS, FormatCurrencyForInterface(cyTotal));

		//This affects training
		UpdateTraining();

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmWorkstationsTS] = GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS);

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-03-30 - PLID 37956 - EMR Workstation
void COpportunityProposalDlg::OnChangeNumEmrWorkstations()
{
	try {
		//We set the value to the number of workstations * the price per
		long nValue = GetDlgItemInt(IDC_NUM_EMRWORKSTATIONS);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpWorkstation];

		SetDlgItemText(IDC_SEL_EMRWORKSTATIONS, FormatCurrencyForInterface(cyTotal));

		//This affects training
		UpdateTraining();

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmEMRWorkstations] = GetDlgItemInt(IDC_NUM_EMRWORKSTATIONS);

	} NxCatchAll("Error in OnChangeEmrNumWorkstations");
}

void COpportunityProposalDlg::OnChangeNumDoctors() 
{
	try {
		//We set the value to the number of doctors * the price per
		long nValue = GetDlgItemInt(IDC_NUM_DOCTORS);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpDoctor];

		SetDlgItemText(IDC_SEL_DOCTORS, FormatCurrencyForInterface(cyTotal));

		//This affects training
		UpdateTraining();

		UpdateAllTotals();

		// (d.lange 2010-4-16) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmMultiDoctor] = GetDlgItemInt(IDC_NUM_DOCTORS);

	} NxCatchAll("Error in OnChangeNumDoctors");
}

void COpportunityProposalDlg::OnChangeNumPda() 
{
	try {
		//We set the value to the number of PDA * the price per
		long nValue = GetDlgItemInt(IDC_NUM_PDA);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpPDA];

		SetDlgItemText(IDC_SEL_PDA, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmPDA] = GetDlgItemInt(IDC_NUM_PDA);

	} NxCatchAll("Error in OnChangeNumPda");
}

void COpportunityProposalDlg::OnChangeNumEmr() 
{
	try {
		//EMR is sold at price for 1 doctor + (# docs - 1 * price for addtl doctors)
		long nValue = GetDlgItemInt(IDC_NUM_EMR);

		COleCurrency cyTotal(0, 0);
		
		if(nValue >= 1) {
			//DRT 9/18/2008 - PLID 31395
			if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
				cyTotal += m_aryPrices[ecpEMRFirstStandard];
			}
			// (d.thompson 2013-07-05) - PLID 57333 - cosmetic
			else if(IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC)) {
				cyTotal += m_aryPrices[ecpEMRCosmetic];
			}
			else {
				cyTotal += m_aryPrices[ecpEMRFirstDoctor];
			}
		}

		if(nValue > 1) {
			//DRT 9/18/2008 - PLID 31395
			if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
				cyTotal += (nValue - 1) * m_aryPrices[ecpEMRAddtlStandard];
			}
			// (d.thompson 2013-07-05) - PLID 57333 - cosmetic is same price for 1st and additional
			else if(IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC)) {
				cyTotal += (nValue - 1) * m_aryPrices[ecpEMRCosmetic];
			}
			else {
				cyTotal += (nValue - 1) * m_aryPrices[ecpEMRAddtlDoctor];
			}
		}

		SetDlgItemText(IDC_SEL_EMR, FormatCurrencyForInterface(cyTotal));

		//This affects training
		UpdateTraining();

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmEMR] = GetDlgItemInt(IDC_NUM_EMR);

	} NxCatchAll("Error in OnChangeNumEmr");
}

void COpportunityProposalDlg::OnChangeNumTraining() 
{
	try {
		//Training is in days * cost per day
		// (z.manning, 11/26/2007) - Make sure to add in EMR training days as it's now tracked separately.
		long nValue = GetDlgItemInt(IDC_NUM_TRAINING) + GetDlgItemInt(IDC_NUM_EMR_TRAINING);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpTraining];

		SetDlgItemText(IDC_SEL_TRAINING, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmPMTraining] = GetDlgItemInt(IDC_NUM_TRAINING);

	} NxCatchAll("Error in OnChangeNumTraining");
}

// (z.manning, 11/26/2007) - PLID 28159
void COpportunityProposalDlg::OnChangeNumEmrTraining()
{
	try
	{
		OnChangeNumTraining();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmEMRTraining] = GetDlgItemInt(IDC_NUM_EMR_TRAINING);

	}NxCatchAll("COpportunityProposalDlg::OnChangeNumEmrTraining");
}

void COpportunityProposalDlg::OnChangeNumSupport() 
{
	try {
		//Update the actual values
		UpdateSupportCosts();

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmSupportMonths] = GetDlgItemInt(IDC_NUM_SUPPORT);

	} NxCatchAll("Error in OnChangeNumSupport");
}

//This is a separate function than the above OnChangeNumSupport because this function
//	must be called to update the support cost each time the subtotal changes.  But when
//	this value changes, it also changes the grand total, so we have to go back to the 
//	UpdateAllTotals() function.
void COpportunityProposalDlg::UpdateSupportCosts()
{
	long nSupportMonths = GetDlgItemInt(IDC_NUM_SUPPORT);

	if(m_nTypeID == eptNexStartup || m_nTypeID == eptNexRes) {
		//DRT 5/23/2008 - PLID 29493 - This needs to be reworked a little with the packages.  We are only displaying 1 
		//	grand total and 1 form, but it's actually calculated as 2 separate forms.  The support costs are calculated
		//	on the modules in the "standard" form, then the second support cost on the addons.  To avoid rounding issues, 
		//	we should calculate these separately and add them together, that is how they will be merged.
		COleCurrency cyCostAddOn = nSupportMonths * CalculateCurrentMonthlySupport();
		COleCurrency cyPackageCost = PKG_SPT_MONTHS * CalculateCurrentMonthlySupport_PackageOnly();	//Only applies to certain types, $0 otherwise

		//For the display, just add them together
		SetDlgItemText(IDC_SEL_SUPPORT, FormatCurrencyForInterface(cyCostAddOn + cyPackageCost));
	}
	else {
		//Support is in months * cost per month
		COleCurrency cyTotal = nSupportMonths * CalculateCurrentMonthlySupport();
		SetDlgItemText(IDC_SEL_SUPPORT, FormatCurrencyForInterface(cyTotal));
	}
}

void COpportunityProposalDlg::OnChangeNumConversion() 
{
	try {
		//Conversions is # of conversions * cost per conversion
		//Sometimes there are extra / different charges for data conversions.  Handle this?
		long nValue = GetDlgItemInt(IDC_NUM_CONVERSION);

		COleCurrency cyTotal(0, 0);
		cyTotal = nValue * m_aryPrices[ecpConversion];
		
		SetDlgItemText(IDC_SEL_CONVERSION, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmConversion] = GetDlgItemInt(IDC_NUM_CONVERSION);

	} NxCatchAll("Error in OnChangeNumConversion");
}

// (d.lange 2010-11-11 16:27) - PLID 40361 - Update EMR Conversion total
void COpportunityProposalDlg::OnChangeNumEmrConversion()
{
	try {
		long nValue = GetDlgItemInt(IDC_NUM_EMRCONVERSION);

		COleCurrency cyTotal(0, 0);
		cyTotal = nValue * m_aryPrices[ecpEmrConversion];

		SetDlgItemText(IDC_SEL_EMRCONVERSION, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		m_aryModules[ecmEmrConversion] = GetDlgItemInt(IDC_NUM_EMRCONVERSION);

	} NxCatchAll("Error in OnChangeNumEmrConversion");
}

// (d.lange 2010-11-11 16:27) - PLID 40361 - Update Financial Conversion total
void COpportunityProposalDlg::OnChangeNumFinancialConversion()
{
	try {
		long nValue = GetDlgItemInt(IDC_NUM_FINCONVERSION);

		COleCurrency cyTotal(0, 0);
		cyTotal = nValue * m_aryPrices[ecpFinancialConversion];

		SetDlgItemText(IDC_SEL_FINCONVERSION, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		m_aryModules[ecmFinancialConversion] = GetDlgItemInt(IDC_NUM_FINCONVERSION);

	} NxCatchAll("Error in OnChangeNumFinancialConversion");
}

void COpportunityProposalDlg::OnChangeNumTravel() 
{
	try {
		//Travel is in trips * cost per trip
		long nValue = GetDlgItemInt(IDC_NUM_TRAVEL);

		// (d.lange 2010-12-29 10:00) - PLID 41889 - Additional travel rules
		COleCurrency cyTotal(0, 0);// = nValue * m_aryPrices[ecpTravel];
		switch(m_nTravelType) {
			case ettCanada:
				cyTotal = nValue * m_aryPrices[ecpTravelCanada];
				break;
			case ettInternational:
				cyTotal = nValue * m_aryPrices[ecpTravelInternational];
				break;
			case ettNewYorkCity:
				cyTotal = nValue * m_aryPrices[ecpTravelNewYorkCity];
				break;
			default:
				cyTotal = nValue * m_aryPrices[ecpTravel];
				break;
		}
		
		SetDlgItemText(IDC_SEL_TRAVEL, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmTravel] = GetDlgItemInt(IDC_NUM_TRAVEL);

	} NxCatchAll("Error in OnChangeNumTravel");
}

//DRT 2/13/2008 - PLID 28905 - Added multi DB
void COpportunityProposalDlg::OnChangeExtraDB() 
{
	try {
		//Travel is in trips * cost per trip
		long nValue = GetDlgItemInt(IDC_NUM_EXTRADB);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpExtraDB];

		SetDlgItemText(IDC_SEL_EXTRADB, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - when value has changed update the array
		m_aryModules[ecmExtraDB] = GetDlgItemInt(IDC_NUM_EXTRADB);
	} NxCatchAll("Error in OnChangeExtraDB");
}

COleCurrency COpportunityProposalDlg::CalculateDiscountFromPercent(double dblPercent)
{
	//Get the current subtotal - with package pricing
	COleCurrency cySubTotal = GetCurrentSubTotalSelected();

	return cySubTotal * dblPercent / 100.0;
}

//Merge data
struct COPMergeData {
	CString m_strHeaders;
	CString m_strData;
};

//return merge information
CString CALLBACK COpportunityProposal__ExtraMergeFields(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam)
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

//For use in OnMergeProposal only
#define ADDMERGEDATA(head, data)	{	opmd.m_strHeaders += head;	opmd.m_strData += "\"" + data + "\",";	}
#define ADDMERGEDATACHECK(head, idc)	{	opmd.m_strHeaders += head;	opmd.m_strData += FormatString("\"%li\",", IsDlgButtonChecked(idc) ? 1 : 0);	}

//	Note:  This only works on US currency or anything that ends in .XX
CString StripUSCents(CString strDollar, bool bIsNegative = false)
{
	//find the last .
	long nPos = strDollar.ReverseFind('.');

	if(!bIsNegative) {
		//always 3rd from end
		ASSERT(nPos == (strDollar.GetLength() - 3));
		return strDollar.Left(nPos);
	}
	else {
		//ASSERT(nPos == (strDollar.GetLength() - 4));
		//Just re-attach the ending ) for negative numbers
		// (d.thompson 2010-04-27) - PLID 38380 - Be a little more careful with the trailing ).  Some of the discount values
		//	can be $0, not really negative.  Technically the flag should be "try to negatize".  It would be more work than
		//	it's worth to change all callers to test for negativity before setting that flag, so we'll just be more cautious here.
		bool bEndsInParen = false;
		if(strDollar.Right(1) == ")") {
			bEndsInParen = true;
		}

		return strDollar.Left(nPos) + (bEndsInParen == false ? "" : ")");
	}
}

CString MergeFormatCurrencyNoCents(COleCurrency cy)
{
	//Anything that is not sold is "N/A", not $0 (i.e. free)
	if(cy == COleCurrency(0, 0))
		return "N/A";

	//strip the cents out
	return StripUSCents(FormatCurrencyForInterface(cy, FALSE));
}

void COpportunityProposalDlg::OnMergeProposal() 
{
	// (d.lange 2010-04-06) - PLID 37999 - when split EMR checkbox is selected, alter the array copies and pass them into a ProposalPricing object
	try {
		// (z.manning 2016-01-29 11:46) - No longer supported
		/*COleCurrency cySupportCost = CalculateCurrentMonthlySupport();
		COleCurrency cySubTotal = GetCurrentSubTotalSelected();
		COleCurrency cyPMSupport(0, 0);
		COleCurrency cyTotalDiscount(0, 0);
		COleCurrency cyPMDiscount(0, 0);

		if(IsDlgButtonChecked(IDC_CHK_SPLITEMR)){

			CArray<int, int> aryForClient;
			CArray<int, int> aryForThirdParty;

			//Set the size equal to the number of possible selected fields
			aryForClient.SetSize(ecmTotalModules);
			aryForThirdParty.SetSize(ecmTotalModules);

			//Create copies of the array containing all selected values
			aryForClient.Copy(m_aryModules);
			aryForThirdParty.Copy(m_aryModules);

			//Create an instance of ProposalPricing with the modified array for client
			CProposalPricing clientProposal(m_aryPrices, SplitEMRForClient(aryForClient));
			CProposalPricing thirdPartyProposal(m_aryPrices, SplitEMRForThirdParty(aryForThirdParty));

			m_bSplitMerge = true;

			//Determine the PM only support cost
			cyPMSupport += clientProposal.GetSupportMonthlyTotal(m_nTypeID);
			
			//Calculate the percent based on the dollar amount
			ADDAMOUNT(IDC_DISCOUNT_TOTAL_DOLLAR, cyTotalDiscount)
			double dblPercent = (double)cyTotalDiscount.m_cur.int64 / (double)cySubTotal.m_cur.int64;
		
			//Get the total dollar discount for just PM merge
			COleCurrency cyDiscount = clientProposal.GetSubTotal(m_nTypeID);
			COleCurrency cyTemp = dblPercent * cyDiscount;
			
			CString strRound = FormatCurrencyForInterface(cyTemp, false, true, false);
			cyPMDiscount += ParseCurrencyFromInterface(strRound);

			//Pass the ProposalPricing object to do the merge, EMR support = Total Support - PM Support, 
			//EMR Discount = Total Discount - PM Discount
			MergeFields(clientProposal, cySupportCost, cyPMSupport, cyTotalDiscount, cyPMDiscount);

			//If first merge is succussful then go ahead with the second
			if(m_nMergeCount)
				MergeFields(thirdPartyProposal, cySupportCost, cyPMSupport, cyTotalDiscount, cyPMDiscount);

			//Reset counter
			m_nMergeCount = 0;
		}else{
			//This is for a normal merge, not splitting EMR
			CProposalPricing proposal(m_aryPrices, m_aryModules);
			MergeFields(proposal, cySupportCost, cyPMSupport, cyTotalDiscount, cyPMDiscount);

			//Reset counter
			m_nMergeCount = 0;
		}*/

		MessageBox("This is no longer supported.", "Not Supported", MB_ICONSTOP);

	} NxCatchAll("Error on OnMergeProposal");
}

// (d.lange 2010-04-05) - PLID 37956 - Put merge fields into a function and passed a ProposalPricing object
// (z.manning 2016-01-29 11:41) - No longer supported
/*
void COpportunityProposalDlg::MergeFields(CProposalPricing &proposal, COleCurrency cySupportTotal, COleCurrency cyPMSupport, COleCurrency cyTotalDiscount, COleCurrency cyPMDiscount)
{
	try {
		//We must first save the proposal to make sure it's valid
		if(!ApplyChanges()) {
			return;
		}

		//Only 1 merge per proposal - this should never happen
		if(m_nMailSentID != -1) {
			if(!m_bSplitMerge){
				MessageBox("You may only merge a proposal once.");
				return;
			}
		}


		//DRT - They'd prefer this to auto-select, so we'll have to pick the file on a ConfigRT entry.  Holding shift
		//	will allow an override for selection, so that we can test new versions.
		//DRT 2/13/2008 - PLID 28894 - The "shift behavior" remains the same with our change to 2 separate proposals.
		CString strPathToDocument = "";
		CString strPathToDocumentAddons = "";


		if(GetAsyncKeyState(VK_SHIFT)) {
			// Get template to merge to
			// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			// Always support Word 2007 templates
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

			//We need to be able to pick 2 documents for nexres/nexstartup types
			if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
				AfxMessageBox("Now you must choose a 2nd file for the addons.");

				CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
				CString initialDir = GetTemplatePath();
				dlg.m_ofn.lpstrInitialDir = initialDir;
				if (dlg.DoModal() == IDOK) {
					strPathToDocumentAddons = dlg.GetPathName();
				}
				else {
					return;
				}
			}

		}
		else {
			//DRT 4/3/2008 - PLID 29493 - We now have to merge differently based on type.  Different forms, etc.
			switch(m_nTypeID) {
			case eptNewSale:
				{
					//DRT 2/13/2008 - PLID 28894 - It has been decreed that we should keep using the old 1 page proposal anytime
					//	new things are not needed.  So therefore we need to look and see if anything "new" is set.  This is pretty
					//	much arbitrary as to what is considered "new" (some of it is older things that are hidden on the 1 page, and
					//	we'll probably change our minds later).  So if one of these things is set, we use the 2 page version.  Otherwise
					//	we use the 1 page version.

					//Here is the current list
					bool bUse2Page = false;

					if(IsDlgButtonChecked(IDC_CHK_EREMITTANCE)) {
						bUse2Page = true;
					}
					if(IsDlgButtonChecked(IDC_CHK_EELIGIBILITY)) {
						bUse2Page = true;
					}
					if(IsDlgButtonChecked(IDC_CHK_INFORM)) {
						bUse2Page = true;
					}
					if(IsDlgButtonChecked(IDC_CHK_QUICKBOOKS)) {
						bUse2Page = true;
					}
					long nCount = GetDlgItemInt(IDC_NUM_EXTRADB);
					if(nCount > 0) {
						bUse2Page = true;
					}
					//DRT 5/29/2008 - PLID 28894 - Forgot HL7
					if(IsDlgButtonChecked(IDC_CHK_HL7)) {
						bUse2Page = true;
					}
					// (d.thompson 2013-07-08) - PLID 57334 - HIE
					if(IsDlgButtonChecked(IDC_CHK_HIE)) {
						bUse2Page = true;
					}
					// (d.thompson 2009-02-02) - PLID 32890 - Added C&A
					if(IsDlgButtonChecked(IDC_CHK_C_AND_A)) {
						bUse2Page = true;
					}
					// (d.thompson 2009-08-10) - PLID 35152 - Added NexWeb
					if(IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS) || IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL)) {
						bUse2Page = true;
					}
					// (d.thompson 2009-11-06) - PLID 36123
					if(IsDlgButtonChecked(IDC_CHK_NEXPHOTO)) {
						bUse2Page = true;
					}
					// (d.thompson 2012-10-12) - PLID 53155 - any TS workstations
					if(GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS) > 0) {
						bUse2Page = true;
					}

					if(bUse2Page) {
						strPathToDocument = GetSharedPath() ^ GetRemotePropertyText("InternalPropPath2Page", "", 0, "<None>", true);
					}
					else {
						//Location of our "defaulted" document
						strPathToDocument = GetSharedPath() ^ GetRemotePropertyText("InternalPropPath", "", 0, "<None>", true);
					}
				}
				break;

			case eptNexStartup:
				{
					//This location is always the same
					strPathToDocument = GetSharedPath() ^ GetRemotePropertyText("InternalPropPathNexStartupMainPage", "", 0, "<None>", true);

					//Additionally, there's a second page to merge for addons
					strPathToDocumentAddons = GetSharedPath() ^ GetRemotePropertyText("InternalPropPathNexStartupAddons", "", 0, "<None>", true);
				}
				break;

			case eptNexRes:
				{
					//This location is always the same
					strPathToDocument = GetSharedPath() ^ GetRemotePropertyText("InternalPropPathNexResidentMainPage", "", 0, "<None>", true);

					//Additionally, there's a second page to merge for addons
					strPathToDocumentAddons = GetSharedPath() ^ GetRemotePropertyText("InternalPropPathNexResidentAddons", "", 0, "<None>", true);
				}
				break;

			default:
				{
					AfxMessageBox("Not yet implemented.");
					return;
				}
				break;
			}
		}

		//
		//"Already counted" feature.  In some cases, certain modules are already counted for the merge.  For right now, 
		//	this happens for NexResident and NexStartup packages.  These modules are printed out on a "standard" form
		//	that cannot be changed, and includes those modules.  So when we print out the merge-calculated form, we want 
		//	the values on the standard to be removed (for example, the standard form contains 1 Workstation License, but
		//	if they purchase 10 total, 9 should be merged through, + the 1 on the standard form).
		//long nWorkstationsCounted = 0;
		//long nTrainingCounted = 0;
		//if(m_nTypeID == eptNexStartup || m_nTypeID == eptNexRes) {
			//These are both the same
			//nWorkstationsCounted = 1;
		//	nTrainingCounted = 1;
		//}





		CWaitCursor wc;

		//Our merge engine object
		CMergeEngine mi;

		//Generate the temp table from the patient given
		CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatientID);
		CString strMergeT = CreateTempIDTable(strSql, "ID");

		// Merge
		if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
		mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
		//Copy the PromptHistoryMerge functionality from the history dialog.  This will provide the sender information.
		// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
		if(!mi.LoadSenderInfo(TRUE)) {
			return;
		}

		//TODO:  Should these default to a certain category?
		long nCategoryID = -1;
		if(nCategoryID != -1) {
			mi.m_nCategoryID = nCategoryID;
		}

		//CProposalPricing proposal(m_aryPrices, m_aryModules);
		//
		//Special merging for proposal dollar values
		//
		COPMergeData opmd;
		// (d.lange 2010-04-02) - PLID 38029 - Now references the Proposal Pricing class
		{	//Scheduler
			//DRT 9/22/2008 - PLID 31395 - These are added when we did the 'standard' package
			//	offerings, they were hardcoded on the form previously.
			//COleCurrency cyList(0, 0);
			//if(IsDlgButtonChecked(IDC_CHK_SCHED)) {
			//	//DRT 9/22/2008 - PLID 31395
			//	if(IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
			//		cyList += m_aryPrices[ecpSchedulerStandard];
			//	}
			//	else {
			//		cyList += m_aryPrices[ecpScheduler];
			//	}
			//}
			//if(IsDlgButtonChecked(IDC_CHK_1LICENSE))
			//	cyList += m_aryPrices[ecp1License];
			
			//This is the sum of all line costs, before package pricing is applied //cyList
			ADDMERGEDATA("Internal_Prop_Sched_Line,",	MergeFormatCurrencyNoCents(proposal.GetPMTotal()))
			
			//COleCurrency cyLine(0, 0);
			//ADDAMOUNT(IDC_SEL_SCHED, cyLine);
			//This is the final price -- the lesser of the package price or the line cost above //cyLine
			ADDMERGEDATA("Internal_Prop_Sched_Package,",	MergeFormatCurrencyNoCents(proposal.GetPMPkgTotal()))

		}
		// (d.lange 2010-04-02) - PLID 38029 - Now references the Proposal Pricing class
		{	//Financial

			//COleCurrency cyList(0, 0);
			//if(IsDlgButtonChecked(IDC_CHK_BILLING))
			//	cyList += m_aryPrices[ecpBilling];
			//if(IsDlgButtonChecked(IDC_CHK_HCFA))
			//	cyList += m_aryPrices[ecpHCFA];
			//if(IsDlgButtonChecked(IDC_CHK_EBILLING))
			//	cyList += m_aryPrices[ecpEBilling];

			
			//This is the sum of all line costs, before package pricing is applied //cyList
			ADDMERGEDATA("Internal_Prop_Fin_Line,",		MergeFormatCurrencyNoCents(proposal.GetIndividualFinTotal()))

			//COleCurrency cyLine(0, 0);
			//ADDAMOUNT(IDC_SEL_FINANCIAL, cyLine);
			//This is the final price -- the lesser of the package price or the line cost above //cyLine
			ADDMERGEDATA("Internal_Prop_Fin_Package,",	MergeFormatCurrencyNoCents(proposal.GetFinancialPkgTotal()))

			//DRT 5/23/2008 - PLID 29493 - In the new package forms we need to know the status of ebilling alone.
			ADDMERGEDATA("Internal_Prop_EBilling_Cost,",	MergeFormatCurrencyNoCents(IsDlgButtonChecked(IDC_CHK_EBILLING) ? m_aryPrices[ecpEBilling] : COleCurrency(0, 0)))
		}
		// (d.lange 2010-04-02) - PLID 38029 - Now references the Proposal Pricing class
		{	//Cosmetic
			//COleCurrency cyList(0, 0);
			//
			////DRT 5/27/2008 - PLID 29493 - If resident or startup, and HCFA not set, this is pulled to the first page, 
			////	not as part of the cosmetic package.  It is already in the subtotals.
			//if((m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) && !IsDlgButtonChecked(IDC_CHK_HCFA)) {
			//	//Do NOT add Letter Writing in this case
			//}
			//else {
			//	if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			//		cyList += m_aryPrices[ecpLetters];
			//	}
			//}
			//if(IsDlgButtonChecked(IDC_CHK_QUOTES))
			//	cyList += m_aryPrices[ecpQuotes];
			//if(IsDlgButtonChecked(IDC_CHK_TRACKING))
			//	cyList += m_aryPrices[ecpTracking];
			//if(IsDlgButtonChecked(IDC_CHK_NEXFORMS))
			//	cyList += m_aryPrices[ecpNexForms];

			//This is the sum of all line costs, before package pricing is applied //cyList
			ADDMERGEDATA("Internal_Prop_Cosm_Line,",	MergeFormatCurrencyNoCents(proposal.GetIndividualCosTotal(m_nTypeID)))

			//DRT 5/27/2008 - PLID 29493 - If we're on a resident or startup, and hcfa is not set, we don't
			//	want the "package" cost to exist at all, it's just confusing.
			//DRT 9/18/2008 - PLID 31395
			COleCurrency cyTmpCosmPkg;
			cyTmpCosmPkg = m_aryPrices[ecpPkgCosmetic];

			CString strPkgP2 = StripUSCents(FormatCurrencyForInterface(cyTmpCosmPkg, false));
			if( (m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) && !IsDlgButtonChecked(IDC_CHK_HCFA) && IsDlgButtonChecked(IDC_CHK_LETTERS)) {
				strPkgP2 = "";
			}
			ADDMERGEDATA("Internal_Prop_Cosm_Pkg_Startup_P2,",	strPkgP2)
			ADDMERGEDATA("Internal_Prop_Letters_Price,",		MergeFormatCurrencyNoCents(m_aryPrices[ecpLetters]))

			//COleCurrency cyLine(0, 0);
			//ADDAMOUNT(IDC_SEL_COSMETIC, cyLine);

			//DRT 5/27/2008 - PLID 29493 - If we're on a resident or startup, and hcfa is not set, take the
			//	letters portion off here if it's on.
			//if((m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) && !IsDlgButtonChecked(IDC_CHK_HCFA) && IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			//	cyLine -= m_aryPrices[ecpLetters];
			//}

			//This is the final price -- the lesser of the package price or the line cost above //cyLine
			ADDMERGEDATA("Internal_Prop_Cosm_Package,",	MergeFormatCurrencyNoCents(proposal.GetCosmeticPkgTotal(m_nTypeID)))

			//DRT 9/24/2008 - PLID 31395 - This was previously hardcoded into the document, but may be needed now with LW Standard
			//COleCurrency cyCosmPkgBaseCost(0, 0);
			//cyCosmPkgBaseCost = m_aryPrices[ecpPkgCosmetic];

			ADDMERGEDATA("Internal_Prop_Cosmetic_Package_BaseCost,",	MergeFormatCurrencyNoCents(proposal.GetCosmeticPkgBasePrice())) //cyCosmPkgBaseCost
		}

		//DRT 5/27/2008 - PLID 29493 - For resident & startup packages, they can use the LW in place of the HCFA module.  If
		//	they do this, we need to change the text on the form to say LW, and this also changes the pricing options.
		CString strText;
		CString strListPrice;
		CString strPrice;		//Despite there being 3 fields for list / pkg / extension, they are always the same number, so 1 merge field.
		CString strSubtotalList, strSubtotalExt, strGrandTotal;
		CString strSupportMonthly, strSupportTotal;
		CString strLWText, strLWListPrice;
		if(m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) {
			//If they do have HCFA, use it first.
			if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
				strText = "HCFA 1500 Insurance Billing";
				//How much does this indiv module cost?
				strListPrice = MergeFormatCurrencyNoCents(m_aryPrices[ecpHCFA]);
				//How much is the whole "financial" section cost?
				strPrice = MergeFormatCurrencyNoCents(m_aryPrices[ecpHCFA] + m_aryPrices[ecpBilling]);

				//For page 2, LW will remain as usual
				strLWText = "MS Word Mail Merge, Letter Writing (Req. W. NexForms)";
				//Must include the $ here
				strLWListPrice = StripUSCents(FormatCurrencyForInterface(m_aryPrices[ecpLetters]), false);
			}
			else {
				//Use letterwriting
				strText = "MS Word Mail Merge, Letter Writing (Req. W. NexForms)";
				//How much does this indiv module cost?
				strListPrice = MergeFormatCurrencyNoCents(m_aryPrices[ecpLetters]);
				//How much is the whole "financial" section cost?
				strPrice = MergeFormatCurrencyNoCents(m_aryPrices[ecpLetters] + m_aryPrices[ecpBilling]);
			}

			//We then need to calculate the Subtotal List & extension values, and the grand total values, since this is configurable.
			COleCurrency cyList = COleCurrency(0, 0);
			COleCurrency cyExt = COleCurrency(0, 0);
			COleCurrency cyGrandTotal = COleCurrency(0, 0);
			COleCurrency cySptMonthly = COleCurrency(0, 0);
			COleCurrency cySptTotal = COleCurrency(0, 0);
			//These are hardcoded values other than our hcfa/lw swap.
			cyList = m_aryPrices[ecpScheduler] + m_aryPrices[ecp1License] + m_aryPrices[ecpBilling] + m_aryPrices[ecpWorkstation]
					+ (IsDlgButtonChecked(IDC_CHK_HCFA) ? m_aryPrices[ecpHCFA] :  m_aryPrices[ecpLetters]);
			cyExt = m_aryPrices[ecpPkgSched] + m_aryPrices[ecpBilling] + m_aryPrices[ecpWorkstation]
					+ (IsDlgButtonChecked(IDC_CHK_HCFA) ? m_aryPrices[ecpHCFA] :  m_aryPrices[ecpLetters]);
			//Again, hardcoded values
			cySptMonthly = CalculateCurrentMonthlySupport_PackageOnly();
			// (a.walling 2008-10-02 09:28) - PLID 31567 - Must specify whether this define should be interpreted
			// as a floating point or integer value.
			cySptTotal = cySptMonthly * (long)PKG_SPT_MONTHS;
			cyGrandTotal = cyExt + m_aryPrices[ecpTraining] + cySptTotal
				- (m_nTypeID == eptNexRes ? m_aryPrices[ecpSpecialResident] : m_aryPrices[ecpSpecialStartup]);

			strSubtotalList = MergeFormatCurrencyNoCents(cyList);
			strSubtotalExt = MergeFormatCurrencyNoCents(cyExt);
			strSupportMonthly = MergeFormatCurrencyNoCents(cySptMonthly);
			strSupportTotal = MergeFormatCurrencyNoCents(cySptTotal);
			strGrandTotal = MergeFormatCurrencyNoCents(cyGrandTotal);
		}
		//And lastly add the fields.  These are always added and blank if we're not in a res/startup.
		ADDMERGEDATA("Internal_Prop_Startup_P1_FinText2,", strText)
		ADDMERGEDATA("Internal_Prop_Startup_P1_Fin2IndivPrice,", strListPrice)
		ADDMERGEDATA("Internal_Prop_Startup_P1_Fin2Price,", strPrice)
		ADDMERGEDATA("Internal_Prop_Startup_P1_SubtotalList,", strSubtotalList)
		ADDMERGEDATA("Internal_Prop_Startup_P1_SubtotalExt,", strSubtotalExt)
		ADDMERGEDATA("Internal_Prop_Startup_P1_SupportMonthly,", strSupportMonthly)
		ADDMERGEDATA("Internal_Prop_Startup_P1_SupportTotal,", strSupportTotal)
		ADDMERGEDATA("Internal_Prop_Startup_P1_GrandTotal,", strGrandTotal)

		//Then, we need the page 2 corresponding fields.  If the LW is going to show up in P1, it cannot
		//	also show up in P2
		ADDMERGEDATA("Internal_Prop_Startup_P2_LWText,", strLWText)
		ADDMERGEDATA("Internal_Prop_Startup_P2_LWListPrice,", strLWListPrice)

		// (d.lange 2010-04-02) - PLID 38029 - Now references the Proposal Pricing class
		ADDMERGEDATA("Internal_Prop_Inv_Bought,", FormatString("%li", proposal.GetInventoryQty())) //IDC_CHK_INV
		ADDMERGEDATA("Internal_Prop_Spa_Bought,",	FormatString("%li", proposal.GetNexSpaQty())) //IDC_CHK_NEXSPA
		ADDMERGEDATA("Internal_Prop_ASC_Bought,",	FormatString("%li", proposal.GetASCQty())) //IDC_CHK_ASC
		//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
		ADDMERGEDATA("Internal_Prop_ERemit_Bought,", FormatString("%li", proposal.GetERemittanceQty())) //IDC_CHK_EREMITTANCE
		ADDMERGEDATA("Internal_Prop_EElig_Bought,", FormatString("%li", proposal.GetEEligibilityQty())) //IDC_CHK_EELIGIBILITY
		ADDMERGEDATA("Internal_Prop_Inv_Cost,",		MergeFormatCurrencyNoCents(proposal.GetInventoryTotal())) //IsDlgButtonChecked(IDC_CHK_INV) ? m_aryPrices[ecpInventory] : COleCurrency(0, 0))
		ADDMERGEDATA("Internal_Prop_Spa_Cost,",		MergeFormatCurrencyNoCents(proposal.GetNexSpaTotal())) //IsDlgButtonChecked(IDC_CHK_NEXSPA) ? m_aryPrices[ecpSpa] : COleCurrency(0, 0))
		ADDMERGEDATA("Internal_Prop_ASC_Cost,",		MergeFormatCurrencyNoCents(proposal.GetASCTotal())) //IsDlgButtonChecked(IDC_CHK_ASC) ? m_aryPrices[ecpASC] : COleCurrency(0, 0))
		//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
		ADDMERGEDATA("Internal_Prop_ERemit_Cost,",		MergeFormatCurrencyNoCents(proposal.GetERemittanceTotal())) //IsDlgButtonChecked(IDC_CHK_EREMITTANCE) ? m_aryPrices[ecpERemittance] : COleCurrency(0, 0))
		ADDMERGEDATA("Internal_Prop_EElig_Cost,",		MergeFormatCurrencyNoCents(proposal.GetEEligibilityTotal())) //IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) ? m_aryPrices[ecpEEligibility] : COleCurrency(0, 0))
		//DRT 5/23/2008 - PLID 29493 - Subtract the "already counted" value
		
		if(!m_nMergeCount) { //Standard Workstations, first merge with and/or without splitting EMR
			ADDMERGEDATA("Internal_Prop_WS_Bought,",	FormatString("%li", proposal.GetWorkstationQty(m_nTypeID))) //GetDlgItemInt(IDC_NUM_WORKSTATIONS) - nWorkstationsCounted
			ADDMERGEDATA("Internal_Prop_WS_Cost,",		MergeFormatCurrencyNoCents((proposal.GetWorkstationQty(m_nTypeID) * proposal.GetWorkstationPrice()))) //(GetDlgItemInt(IDC_NUM_WORKSTATIONS) - nWorkstationsCounted) * m_aryPrices[ecpWorkstation]
		}else{ //EMR Workstations, second merge when splitting EMR
			ADDMERGEDATA("Internal_Prop_WS_Bought,",	FormatString("%li", proposal.GetEMRWorkstationQty())) //GetDlgItemInt(IDC_NUM_WORKSTATIONS) - nWorkstationsCounted
			ADDMERGEDATA("Internal_Prop_WS_Cost,",		MergeFormatCurrencyNoCents((proposal.GetEMRWorkstationQty() * proposal.GetWorkstationPrice()))) //(GetDlgItemInt(IDC_NUM_WORKSTATIONS) - nWorkstationsCounted) * m_aryPrices[ecpWorkstation]
		}

		// (d.thompson 2012-10-12) - PLID 53155 - Added TS Workstations.  These are completely outside the split emr workstations above
		ADDMERGEDATA("Internal_Prop_WS_TS_Bought,",	FormatString("%li", proposal.GetWorkstationTSQty()))
		ADDMERGEDATA("Internal_Prop_WS_TS_Cost,",	MergeFormatCurrencyNoCents((proposal.GetWorkstationTSQty() * proposal.GetWorkstationPrice())))


		ADDMERGEDATA("Internal_Prop_Doc_Bought,",	FormatString("%li", proposal.GetMultiDoctorQty())) //FormatString("%li", GetDlgItemInt(IDC_NUM_DOCTORS))
		ADDMERGEDATA("Internal_Prop_Doc_Cost,",		MergeFormatCurrencyNoCents((proposal.GetMultiDoctorQty() * proposal.GetMultiDoctorPrice()))) //GetDlgItemInt(IDC_NUM_DOCTORS) * m_aryPrices[ecpDoctor]
		ADDMERGEDATA("Internal_Prop_PDA_Bought,",	FormatString("%li", proposal.GetPDAQty())) //FormatString("%li", GetDlgItemInt(IDC_NUM_PDA))
		ADDMERGEDATA("Internal_Prop_PDA_Cost,",		MergeFormatCurrencyNoCents(proposal.GetPDATotal())) //GetDlgItemInt(IDC_NUM_PDA) * m_aryPrices[ecpPDA]
		// (d.thompson 2009-11-13) - PLID 36124
		ADDMERGEDATA("Internal_Prop_NexSync_Bought,",	FormatString("%li", proposal.GetNexSyncQty())) //FormatString("%li", GetDlgItemInt(IDC_NUM_NEXSYNC))
		ADDMERGEDATA("Internal_Prop_NexSync_Cost,",		MergeFormatCurrencyNoCents(proposal.GetNexSyncTotal())) //GetDlgItemInt(IDC_NUM_NEXSYNC) * m_aryPrices[ecpNexSync]
		// (d.thompson 2009-01-30) - PLID 32890 - Added C and A module
		ADDMERGEDATA("Internal_Prop_CandA_Bought,", FormatString("%li", proposal.GetCandAQty())) //IDC_CHK_C_AND_A
		ADDMERGEDATA("Internal_Prop_CandA_Cost,",	MergeFormatCurrencyNoCents(proposal.GetCandATotal())) //IsDlgButtonChecked(IDC_CHK_C_AND_A) ? m_aryPrices[ecpCandA] : COleCurrency(0, 0)
		{	// (d.thompson 2009-08-10) - PLID 35152 - NexWeb
			//COleCurrency cyList(0, 0);
			//if(IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS))
			//	cyList += m_aryPrices[ecpNexWebLeads];
			//if(IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL))
			//	cyList += m_aryPrices[ecpNexWebPortal];

			// (d.lange 2010-04-19 11:27) - PLID 38029 - No longer displays 'N/A' on the PM merge
			CString strNexWebListTotal = StripUSCents(FormatCurrencyForInterface(proposal.GetIndividualNexWebTotal()));
			//This is the sum of all line costs, before package pricing is applied //cyList
			ADDMERGEDATA("Internal_Prop_NexWeb_List,",		MergeFormatCurrencyNoCents(proposal.GetIndividualNexWebTotal()))

			//COleCurrency cyLine(0, 0);
			//ADDAMOUNT(IDC_SEL_NEXWEB, cyLine);
			//This is the final price -- the lesser of the package price or the line cost above //cyLine
			ADDMERGEDATA("Internal_Prop_NexWeb_Package,",	MergeFormatCurrencyNoCents(proposal.GetNexWebPkgTotal()))
		}
		// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
		// (d.lange 2010-04-02) - PLID 38029 - Now references the Proposal Pricing class
			ADDMERGEDATA("Internal_Prop_NexPhoto_Bought,", FormatString("%li", proposal.GetNexPhotoQty())) //IDC_CHK_NEXPHOTO
			ADDMERGEDATA("Internal_Prop_NexPhoto_Cost,", MergeFormatCurrencyNoCents(proposal.GetNexPhotoTotal())) //IsDlgButtonChecked(IDC_CHK_NEXPHOTO) ? m_aryPrices[ecpNexPhoto] : COleCurrency(0, 0)


		{	//Links -- this is output as 1 merge field
			//long nCnt = 0;
			//COleCurrency cyTotal(0, 0);
			//if(IsDlgButtonChecked(IDC_CHK_MIRROR)) {
			//	nCnt += 1;
			//	cyTotal += m_aryPrices[ecpMirror];
			//}
			//if(IsDlgButtonChecked(IDC_CHK_UNITED)) {
			//	nCnt += 1;
			//	cyTotal += m_aryPrices[ecpUnited];
			//}

			ADDMERGEDATA("Internal_Prop_Link_Bought,",	FormatString("%li", (proposal.GetMirrorQty() == 1 ? proposal.GetMirrorQty() : (proposal.GetUnitedQty() == 1 ? proposal.GetUnitedQty() : 0)))) //FormatString("%li", nCnt)
			ADDMERGEDATA("Internal_Prop_Link_Cost,",	MergeFormatCurrencyNoCents((proposal.GetMirrorQty() == 1 ? proposal.GetMirrorTotal() : (proposal.GetUnitedQty() == 1 ? proposal.GetUnitedTotal() : COleCurrency (0, 0)))))
		}

		//DRT 2/12/2008 - PLID 28894 - Added merge fields for HL7
			ADDMERGEDATA("Internal_Prop_HL7_Bought,", FormatString("%li", proposal.GetHL7Qty())) //IDC_CHK_HL7
			ADDMERGEDATA("Internal_Prop_HL7_Cost,",		MergeFormatCurrencyNoCents(proposal.GetHL7Total())) //IsDlgButtonChecked(IDC_CHK_HL7) ? m_aryPrices[ecpHL7] : COleCurrency(0, 0)

		// (d.thompson 2013-07-08) - PLID 57334 - HIE
		ADDMERGEDATA("Internal_Prop_HIE_Bought,", FormatString("%li", proposal.GetHIEQty()))
		ADDMERGEDATA("Internal_Prop_HIE_Cost,",		MergeFormatCurrencyNoCents(proposal.GetHIETotal()))

		//DRT 2/12/2008 - PLID 28894 - This is a dynamic merge field based on whatever they've picked in the
		//	"secret" options.  These are modules we don't advertise, and don't really encourage to be sold, 
		//	but that are available.  Currently this is just Inform and QB, though the 'Additional Database'
		//	will need put in here as well.
		{
			//CString strSecretText;
			//COleCurrency cySecretPrice(0, 0);	//List and total will be identical

			////If no options are selected, we don't want to merge a thing
			//if(IsDlgButtonChecked(IDC_CHK_INFORM)) {
			//	//Set the text for the left side of the proposal
			//	strSecretText = FormatString("Software Link to Inform ($%s)", MergeFormatCurrencyNoCents(m_aryPrices[ecpInform]));

			//	//Add to the price
			//	cySecretPrice += m_aryPrices[ecpInform];
			//}
			//if(IsDlgButtonChecked(IDC_CHK_QUICKBOOKS)) {
			//	//Set the text for the left side of the proposal
			//	if(!strSecretText.IsEmpty()) {
			//		strSecretText += ", ";
			//	}
			//	strSecretText += FormatString("Software Link to Quickbooks ($%s)", MergeFormatCurrencyNoCents(m_aryPrices[ecpQuickbooks]));

			//	//Add to the price
			//	cySecretPrice += m_aryPrices[ecpQuickbooks];
			//}

			//CString strSecretPrice = "";
			//if(cySecretPrice > COleCurrency(0, 0)) {
			//	strSecretPrice = "$\t" + MergeFormatCurrencyNoCents(cySecretPrice);
			//}
			CString strInformPrice = MergeFormatCurrencyNoCents(proposal.GetInformTotal());
			CString strQuickbooksPrice = MergeFormatCurrencyNoCents(proposal.GetQuickbooksTotal());
			//Now format appropriately for the merge
			ADDMERGEDATA("Internal_Prop_Secret_Text,", proposal.GetInformQuickbooksString(strInformPrice, strQuickbooksPrice)) //strSecretText
			ADDMERGEDATA("Internal_Prop_Secret_Price,", (proposal.GetInformQuickbooksTotal() > COleCurrency(0, 0) ? "$\t" + MergeFormatCurrencyNoCents(proposal.GetInformQuickbooksTotal()) : "")) //strSecretPrice
		}

		{	//EMR - We split this into 2 merge fields -- "1st dr" and "addtl doctors"
			long nEMR = GetDlgItemInt(IDC_NUM_EMR);
			ADDMERGEDATA("Internal_Prop_EMR_Bought_First,",	FormatString("%li", proposal.GetEMRQty() >= 1 ? 1 : 0)) //nEMR >= 1 ? 1 : 0
			ADDMERGEDATA("Internal_Prop_EMR_Bought_Addtl,",	FormatString("%li", proposal.GetEMRQty() >= 1 ? proposal.GetEMRQty() - 1 : 0)) //nEMR >= 1 ? nEMR - 1 : 0
			// (d.thompson 2010-12-29) - PLID 41948 - No longer doing First & Addtl, just all.  Leave the above just in case we go back in the future.
			ADDMERGEDATA("Internal_Prop_EMR_Bought_All,",	FormatString("%li", proposal.GetEMRQty()))

			//DRT 9/22/2008 - PLID 31395 - With EMR Std pricing, we need to merge that pricing as
			//	well to the left side of the form.
			//COleCurrency cyCostFirst(0, 0), cyCostAddtl(0, 0);
			//if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
			//	cyCostFirst = m_aryPrices[ecpEMRFirstStandard];
			//	cyCostAddtl = m_aryPrices[ecpEMRAddtlStandard];
			//}
			//else {
			//	cyCostFirst = m_aryPrices[ecpEMRFirstDoctor];
			//	cyCostAddtl = m_aryPrices[ecpEMRAddtlDoctor];
			//}
			ADDMERGEDATA("Internal_Prop_EMR_BasePrice_First,",	MergeFormatCurrencyNoCents(proposal.GetEMRFirstBasePrice())) //cyCostFirst
			ADDMERGEDATA("Internal_Prop_EMR_BasePrice_Addtl,",	MergeFormatCurrencyNoCents(proposal.GetEMRAddtlBasePrice())) //cyCostAddtl
			// (d.thompson 2010-12-29) - PLID 41948 - We no longer have "first" and "addtl", all pricing is the same.  However, given the frequency of
			//	mind-changing that goes on, I'm still going to leave the above as-is, we'll just quit showing them on the proposal.  This way, if
			//	they go back to the old method, we can just update the forms and be done.
			ADDMERGEDATA("Internal_Prop_EMR_BasePrice_All,",	MergeFormatCurrencyNoCents(proposal.GetEMRAllBasePrice())) //cyCostAddtl

			//COleCurrency cyTotalFirst(0, 0), cyTotalAddtl(0, 0);
			//if(nEMR >= 1) {
			//	//DRT 9/18/2008 - PLID 31395
			//	if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
			//		cyTotalFirst += m_aryPrices[ecpEMRFirstStandard];
			//	}
			//	else {
			//		cyTotalFirst += m_aryPrices[ecpEMRFirstDoctor];
			//	}
			//}

			//if(nEMR > 1) {
			//	//DRT 9/18/2008 - PLID 31395
			//	if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
			//		cyTotalAddtl += (nEMR - 1) * m_aryPrices[ecpEMRAddtlStandard];
			//	}
			//	else {
			//		cyTotalAddtl += (nEMR - 1) * m_aryPrices[ecpEMRAddtlDoctor];
			//	}
			//}

			ADDMERGEDATA("Internal_Prop_EMR_Cost_First,",		MergeFormatCurrencyNoCents(proposal.GetEMRTotal(embFirstEMR))) //cyTotalFirst
			ADDMERGEDATA("Internal_Prop_EMR_Cost_Addtl,",		MergeFormatCurrencyNoCents(proposal.GetEMRTotal(embAddtlEMR))) //cyTotalAddtl
			// (d.thompson 2010-12-29) - PLID 41948 - Got rid of first & addtl pricing, all the same now.  We keep the above just in case
			//	it goes back.  Add a new merge field for all.
			ADDMERGEDATA("Internal_Prop_EMR_Cost_All,",			MergeFormatCurrencyNoCents(proposal.GetEMRTotal(embFirstEMR) + proposal.GetEMRTotal(embAddtlEMR)))
		}

		{	//Subtotal
			//COleCurrency cyList(0, 0);
			//ADDAMOUNT(IDC_SUBTOTAL_LIST, cyList);
			ADDMERGEDATA("Internal_Prop_Subtotal_List,",		MergeFormatCurrencyNoCents(proposal.GetListSubTotal(m_nTypeID))) //cyList

			COleCurrency cySubDisc(0, 0);
			//ADDAMOUNT(IDC_SUBTOTAL_PKG_DISCOUNTS, cySubDisc);
			cySubDisc += (proposal.GetListSubTotal(m_nTypeID) - proposal.GetSubTotal(m_nTypeID));
			
			cySubDisc *= -1;	//negative value
			
			ADDMERGEDATA("Internal_Prop_Subtotal_Discount,",	StripUSCents(FormatCurrencyForInterface(cySubDisc), true)) //cySubDisc

			//COleCurrency cySubTotal(0, 0);
			//ADDAMOUNT(IDC_SUBTOTAL_SEL, cySubTotal);
			ADDMERGEDATA("Internal_Prop_Subtotal_Total,",		MergeFormatCurrencyNoCents(proposal.GetSubTotal(m_nTypeID))) //cySubTotal

			COleCurrency cyDiscount(0, 0);
			ADDAMOUNT(IDC_DISCOUNT_TOTAL_DOLLAR, cyDiscount);
			cyDiscount *= -1;	//negative value
			//If no discount, the whole thing is hidden
			//This is just a description field for the discount
			if(cyDiscount == COleCurrency(0, 0)) {
				ADDMERGEDATA("Internal_Prop_Discount_Total,",		CString(""))
				ADDMERGEDATA("Internal_Prop_Discount_Desc,",		CString(""))
			}
			else {
				// (d.thompson 2012-04-05) - PLID 49453 - Changed default text, also moved to ConfigRT because this just keeps changing
				CString strDiscountDescription = GetRemotePropertyText("InternalOpp_DiscountDesc", "Professional Referrals and Technical Suggestions Discount", 0, "<None>", true);
				if(m_bSplitMerge) {
					if(m_nMergeCount == 1 && IsDlgButtonChecked(IDC_CHK_DISCOUNT_EMR)){ //EMR page
						ADDMERGEDATA("Internal_Prop_Discount_Total,",		StripUSCents(FormatCurrencyForInterface( -1 * (cyTotalDiscount - cyPMDiscount)), true))
						// (d.lange 2010-06-07 15:57) - PLID 39044 - changed from "Professional Package Discount" to "Professional Referrals Discount"
						ADDMERGEDATA("Internal_Prop_Discount_Desc,",		CString(strDiscountDescription))
					}else if(m_nMergeCount == 0 && IsDlgButtonChecked(IDC_CHK_DISCOUNT_EMR)){
						ADDMERGEDATA("Internal_Prop_Discount_Total,",		StripUSCents(FormatCurrencyForInterface(-1 * cyPMDiscount), true))
						// (d.lange 2010-06-07 15:57) - PLID 39044 - changed from "Professional Package Discount" to "Professional Referrals Discount"
						ADDMERGEDATA("Internal_Prop_Discount_Desc,",		CString(strDiscountDescription))
					}else if(m_nMergeCount == 0 && !IsDlgButtonChecked(IDC_CHK_DISCOUNT_EMR)) {
						ADDMERGEDATA("Internal_Prop_Discount_Total,",		StripUSCents(FormatCurrencyForInterface(-1 * cyTotalDiscount), true))
						// (d.lange 2010-06-07 15:57) - PLID 39044 - changed from "Professional Package Discount" to "Professional Referrals Discount"
						ADDMERGEDATA("Internal_Prop_Discount_Desc,",		CString(strDiscountDescription))
					}else{
						ADDMERGEDATA("Internal_Prop_Discount_Total,",		CString(""))
						ADDMERGEDATA("Internal_Prop_Discount_Desc,",		CString(""))
					}
				}else{
					ADDMERGEDATA("Internal_Prop_Discount_Total,",		StripUSCents(FormatCurrencyForInterface(cyDiscount), true))
					// (d.lange 2010-06-07 15:57) - PLID 39044 - changed from "Professional Package Discount" to "Professional Referrals Discount"
					ADDMERGEDATA("Internal_Prop_Discount_Desc,",		CString(strDiscountDescription))
				}
			}

			//DRT 5/23/2008 - PLID 29493 - We need to generate entirely separate fields for our resident & startup packages to merge.
			if(m_nTypeID == eptNexStartup || m_nTypeID == eptNexRes) {
				COleCurrency cyList(0, 0);
				ADDAMOUNT(IDC_SUBTOTAL_LIST, cyList);
				//Now remove the list prices as they are in the "standard" form on page 1
				//	Scheduler module, default 1 ws, HCFA & Cosm. Billing, 1 Workstation
				cyList -= m_aryPrices[ecpScheduler];
				cyList -= m_aryPrices[ecp1License];
				cyList -= m_aryPrices[ecpBilling];
				//Package piece:  EITHER HCFA OR LW (HCFA takes precedence)
				if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
					cyList -= m_aryPrices[ecpHCFA];
				}
				else if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
					cyList -= m_aryPrices[ecpLetters];
				}

				cyList -= m_aryPrices[ecpWorkstation];
				ADDMERGEDATA("Internal_Prop_Startup_Subtotal_List,",		MergeFormatCurrencyNoCents(cyList))

				//Subtotal Discount is always $0, there is no possible discount on this.

				COleCurrency cySubTotal(0, 0);
				ADDAMOUNT(IDC_SUBTOTAL_SEL, cySubTotal);
				//Now remove the list prices as they are in the "standard" form on page 1
				//	Scheduler package, HCFA & Cosm. Billing, 1 Workstation
				cySubTotal -= m_aryPrices[ecpPkgSched];
				cySubTotal -= m_aryPrices[ecpBilling];
				//Package piece:  EITHER HCFA OR LW (HCFA takes precedence)
				if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
					cySubTotal -= m_aryPrices[ecpHCFA];
				}
				else if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
					cySubTotal -= m_aryPrices[ecpLetters];
				}
				cySubTotal -= m_aryPrices[ecpWorkstation];
				ADDMERGEDATA("Internal_Prop_Startup_Subtotal_Total,",		MergeFormatCurrencyNoCents(cySubTotal))

				COleCurrency cyDiscountPkg(0, 0);
				ADDAMOUNT(IDC_SPECIAL_DISCOUNT_DOLLAR, cyDiscountPkg);
				cyDiscountPkg *= -1;	//negative value
				COleCurrency cyDiscountAddOn(0, 0);
				cyDiscountAddOn.ParseCurrency(m_labelSpecialDiscountAddOn.GetText());
				ADDAMOUNT(IDC_SPECIAL_DISCOUNT_ADDON, cyDiscountAddOn);
				cyDiscountAddOn *= -1;	//negative value

				ADDMERGEDATA("Internal_Prop_Startup_P2_Subtotal_Discount,",		(cySubTotal - cyList == COleCurrency(0, 0) ? CString("") : StripUSCents(FormatCurrencyForInterface(cySubTotal - cyList), true)))
				

				//Support costs
				ADDMERGEDATA("Internal_Prop_Startup_P2_SupportMonthly,",		CString(""))
				

				//This discount is always applied if we're in this type
				ADDMERGEDATA("Internal_Prop_Startup_Discount_Package,",		StripUSCents(FormatCurrencyForInterface(cyDiscountPkg), true))

				//If no discount, the whole thing is hidden
				//This is just a description field for the discount
				if(cyDiscountAddOn == COleCurrency(0, 0)) {
					ADDMERGEDATA("Internal_Prop_Startup_Discount_AddOn,",		CString(""))
					ADDMERGEDATA("Internal_Prop_Startup_Discount_Desc,",		CString(""))
				}
				else {
					ADDMERGEDATA("Internal_Prop_Startup_Discount_AddOn,",		StripUSCents(FormatCurrencyForInterface(cyDiscountAddOn), true))
					//These differ depending which form we're in
					if(m_nTypeID == eptNexStartup) {
						ADDMERGEDATA("Internal_Prop_Startup_Discount_Desc,",		CString("NexTech Start-up Special Discount"))
					}
					else if(m_nTypeID == eptNexRes) {
						ADDMERGEDATA("Internal_Prop_Startup_Discount_Desc,",		CString("NexTech Residents Consideration Discount"))
					}
				}
			}
		}

		// (z.manning, 11/26/2007) - PLID 28159 - We display EMR training days separate from PM training.
		//DRT 5/23/2008 - PLID 29493 - Subtract the "already counted" value
		ADDMERGEDATA("Internal_Prop_Training_Count,", FormatString("%li", proposal.GetPMTrainingQty(m_nTypeID)) + (proposal.GetEMRQty() > 0 ? FormatString(" PM\r\n%li EMR", proposal.GetEMRTrainingQty()) : ""))
		//FormatString("%li", GetDlgItemInt(IDC_NUM_TRAINING) - nTrainingCounted) + (GetDlgItemInt(IDC_NUM_EMR) > 0 ? FormatString(" PM\r\n%li EMR", GetDlgItemInt(IDC_NUM_EMR_TRAINING)) : "")
		ADDMERGEDATA("Internal_Prop_Training_Cost,",		MergeFormatCurrencyNoCents((proposal.GetPMTrainingQty(m_nTypeID) + proposal.GetEMRTrainingQty()) * proposal.GetTrainingPrice()))
		//(GetDlgItemInt(IDC_NUM_TRAINING) - nTrainingCounted + GetDlgItemInt(IDC_NUM_EMR_TRAINING)) * m_aryPrices[ecpTraining]

		{	//Support
			long nCnt = GetDlgItemInt(IDC_NUM_SUPPORT);
			COleCurrency cyEMRSupport = (cySupportTotal - cyPMSupport);

			ADDMERGEDATA("Internal_Prop_Support_Count,",	FormatString("%li", nCnt))
			if(m_bSplitMerge) {
				if(m_nMergeCount){ //EMR page
					ADDMERGEDATA("Internal_Prop_Support_Month,",	StripUSCents(FormatCurrencyForInterface(cyEMRSupport, FALSE)))
					ADDMERGEDATA("Internal_Prop_Support_Cost,",		StripUSCents(FormatCurrencyForInterface(cyEMRSupport * nCnt, FALSE)))	//Do not strip cents
				}else{	//PM page
					ADDMERGEDATA("Internal_Prop_Support_Month,",	StripUSCents(FormatCurrencyForInterface(cyPMSupport, FALSE)))
					ADDMERGEDATA("Internal_Prop_Support_Cost,",		StripUSCents(FormatCurrencyForInterface(cyPMSupport * nCnt, FALSE)))	//Do not strip cents
				}
			}else{
				ADDMERGEDATA("Internal_Prop_Support_Month,",	StripUSCents(FormatCurrencyForInterface(cySupportTotal, FALSE)))			//Do not strip cents
				ADDMERGEDATA("Internal_Prop_Support_Cost,",		StripUSCents(FormatCurrencyForInterface(cySupportTotal * nCnt, FALSE)))	//Do not strip cents
			}
			
			ADDMERGEDATA("Internal_Prop_Support_Percent,",	FormatString("%li", GetDlgItemInt(IDC_SUPPORT_PERCENT)))
		}

		//ADDMERGEDATA("Internal_Prop_Conversion_Count,",		FormatString("%li", proposal.GetConversionQty())) //GetDlgItemInt(IDC_NUM_CONVERSION)

		{	//TODO:  I cannot explain this.  Attempting to put the code below into a single statement (like all the ones above) just throws
			//	a random neen exception in release mode only.  Once I split it up into smaller chunks, it works fine.
			//long nValue;
			//CString str;

			//nValue = GetDlgItemInt(IDC_NUM_CONVERSION);
			//COleCurrency cyConversion = m_aryPrices[ecpConversion];
			//cyConversion *= nValue;
			//str = MergeFormatCurrencyNoCents(cyConversion);

			// (d.lange 2010-09-17 14:32) - PLID 40361 - Get the Conversion counts
			long nConversion = proposal.GetConversionQty();
			long nEmrConversion = proposal.GetEmrConversionQty();
			long nFinancialConversion = proposal.GetFinancialConversionQty();

			//We'll show Patient Demographic Conversion by default
			CString strConversionDesc = "- Patient Demographic Conversion for Each Converted Database";
			
			CString strEmrConversionDesc = "";
			if(nEmrConversion > 0) {
				// (d.thompson 2010-12-29) - PLID 41950 - Re-worded.  We are NOT converting to PDF, we're just taking
				//	the data as given to us and attaching it to history.
				strEmrConversionDesc = "- EMR Documents attached to history";
			}
			
			CString strFinancialConversionDesc = "";
			if(nFinancialConversion > 0) {
				strFinancialConversionDesc = "- Financial Data Conversion for Each Converted Database";
			}

			ADDMERGEDATA("Internal_Prop_Conversion_Desc,", strConversionDesc);
			ADDMERGEDATA("Internal_Prop_Conversion_Desc2,", strEmrConversionDesc);
			ADDMERGEDATA("Internal_Prop_Conversion_Desc3,", strFinancialConversionDesc);
			
			// We'll show the Patient Demographic Conversion Line cost by default
			CString strConversionLine = "$   " + MergeFormatCurrencyNoCents(m_aryPrices[ecpConversion]);
			CString strEmrConversionLine = (nEmrConversion == 0 ? "" : "$   " + MergeFormatCurrencyNoCents(m_aryPrices[ecpEmrConversion]));
			CString strFinancialConversionLine = (nFinancialConversion == 0 ? "" : "$   " + MergeFormatCurrencyNoCents(m_aryPrices[ecpFinancialConversion]));

			ADDMERGEDATA("Internal_Prop_Conversion_Line,", strConversionLine);
			ADDMERGEDATA("Internal_Prop_Conversion_Line2,", strEmrConversionLine);
			ADDMERGEDATA("Internal_Prop_Conversion_Line3,", strFinancialConversionLine);

			ADDMERGEDATA("Internal_Prop_Conversion_Count,", FormatString("%li", nConversion));
			ADDMERGEDATA("Internal_Prop_Conversion_Count2,", (nEmrConversion > 0 ? FormatString("%li", nEmrConversion) : ""));
			ADDMERGEDATA("Internal_Prop_Conversion_Count3,", (nFinancialConversion > 0 ? FormatString("%li", nFinancialConversion) : ""));

			// Calculate the total conversion costs
			CString strConversionCost = "$   " + MergeFormatCurrencyNoCents(proposal.GetConversionTotal());
			ADDMERGEDATA("Internal_Prop_Conversion_Cost,", strConversionCost) //str
		}

		ADDMERGEDATA("Internal_Prop_Travel_Count,",			FormatString("%li", proposal.GetTravelQty())) //GetDlgItemInt(IDC_NUM_TRAVEL)

		{	//TODO:  I cannot explain this.  Attempting to put the code below into a single statement (like all the ones above) just throws
			//	a random neen exception in release mode only.  Once I split it up into smaller chunks, it works fine.
			//long nValue;
			//CString str;

			//nValue = GetDlgItemInt(IDC_NUM_TRAVEL);
			//COleCurrency cyTravel = m_aryPrices[ecpTravel];
			//cyTravel *= nValue;
			//str = StripUSCents(FormatCurrencyForInterface(cyTravel, FALSE));
			
			// (d.lange 2010-12-22 10:40) - PLID 41889 - Support additional travel rules
			CString strTravelLine = "";
			switch(m_nTravelType) {
				case ettCanada:
					strTravelLine = MergeFormatCurrencyNoCents(m_aryPrices[ecpTravelCanada]);
					break;
				case ettInternational:
					strTravelLine = MergeFormatCurrencyNoCents(m_aryPrices[ecpTravelInternational]);
					break;
				case ettNewYorkCity:
					strTravelLine = MergeFormatCurrencyNoCents(m_aryPrices[ecpTravelNewYorkCity]);
					break;
				default:
					strTravelLine = MergeFormatCurrencyNoCents(m_aryPrices[ecpTravel]);
					break;
			}
			ADDMERGEDATA("Internal_Prop_Travel_Line,", strTravelLine);

			CString strTravelCost = MergeFormatCurrencyNoCents(proposal.GetTravelTotal(m_nTravelType));
			ADDMERGEDATA("Internal_Prop_Travel_Cost,", strTravelCost) //str
		}

		//DRT 2/13/2008 - PLID 28905 - Added multi DB.  This is another "secret" field, meaning it's not displayed on the
		//	standard form unless someone buys it.
		{
			CString strSecretAddtlText = "";
			long nCount = GetDlgItemInt(IDC_NUM_EXTRADB);

			CString strLabelText, strSingleCost, strPriceText, strExtraDBCount;

			if(nCount > 0) {
				strLabelText = "Additional Database(s)";
				//strCountText = FormatString("%li", nCount);
				//strPriceText = "$\t" + StripUSCents(FormatCurrencyForInterface(m_aryPrices[ecpExtraDB] * nCount, FALSE));
				strPriceText = "$\t" + StripUSCents(FormatCurrencyForInterface(proposal.GetExtraDBQty() * m_aryPrices[ecpExtraDB], FALSE));
				strExtraDBCount = FormatString("%li", proposal.GetExtraDBQty());
				//This is just a 1 unit price
				strSingleCost = "$\t" + StripUSCents(FormatCurrencyForInterface(m_aryPrices[ecpExtraDB], FALSE));
			}

			//Setup merge fields, which will be blank if we don't have anything
			ADDMERGEDATA("Internal_Prop_ExtraDB_Label,",		strLabelText)
			ADDMERGEDATA("Internal_Prop_ExtraDB_Count,",		strExtraDBCount)
			ADDMERGEDATA("Internal_Prop_ExtraDB_Cost,",			strPriceText)
			ADDMERGEDATA("Internal_Prop_ExtraDB_SingleCost,",	strSingleCost)
		}


		{	//Grand total!
			COleCurrency cyTotal(0, 0);
			ADDAMOUNT(IDC_GRAND_TOTAL, cyTotal)

			// (d.lange 2010-04-08) - PLID 38096 - determine the EMR Discount 
			COleCurrency cyEMRDiscount = (cyTotalDiscount - cyPMDiscount);
			COleCurrency cyEMRSupportTotal = (proposal.GetNumSupport() * (cySupportTotal - cyPMSupport));

			if(m_bSplitMerge) {
				if(m_nMergeCount == 1 && IsDlgButtonChecked(IDC_CHK_DISCOUNT_EMR)) {
					ADDMERGEDATA("Internal_Prop_Total,",			FormatCurrencyForInterface(proposal.GetGrandTotal(m_nTypeID, m_nTravelType, (proposal.GetNumSupport() * (cySupportTotal - cyPMSupport)), cyEMRDiscount)))
				}else if(m_nMergeCount == 0 && IsDlgButtonChecked(IDC_CHK_DISCOUNT_EMR)) {
					ADDMERGEDATA("Internal_Prop_Total,",			FormatCurrencyForInterface(proposal.GetGrandTotal(m_nTypeID, m_nTravelType, (proposal.GetNumSupport() * (cyPMSupport)), cyPMDiscount)))
				}else if(m_nMergeCount == 0 && !IsDlgButtonChecked(IDC_CHK_DISCOUNT_EMR)){//PM Grand Total
					ADDMERGEDATA("Internal_Prop_Total,",			FormatCurrencyForInterface(proposal.GetGrandTotal(m_nTypeID, m_nTravelType, (proposal.GetNumSupport() * cyPMSupport), cyTotalDiscount)))
				}else{
					ADDMERGEDATA("Internal_Prop_Total,",			FormatCurrencyForInterface(proposal.GetGrandTotal(m_nTypeID, m_nTravelType, (proposal.GetNumSupport() * (cySupportTotal - cyPMSupport)), COleCurrency(0,0))))
				}
			}else{
				ADDMERGEDATA("Internal_Prop_Total,",			FormatCurrencyForInterface(cyTotal))
			}
			//DRT 5/23/2008 - PLID 29493 - We need to generate entirely separate fields for our resident & startup packages to merge.
			if(m_nTypeID == eptNexStartup || m_nTypeID == eptNexRes) {
				COleCurrency cyTotalStartup(0, 0);
				ADDAMOUNT(IDC_GRAND_TOTAL, cyTotalStartup);
				//Subtract out that which is on the first page
				//	Subtotal pieces + 1 day training + 9 months support - standard discount
				{	//Builds the subtotal
					//	Scheduler package, HCFA & Cosm. Billing, 1 Workstation
					cyTotalStartup -= m_aryPrices[ecpPkgSched];
					cyTotalStartup -= m_aryPrices[ecpBilling];
					//Package piece:  EITHER HCFA OR LW (HCFA takes precedence)
					if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
						cyTotalStartup -= m_aryPrices[ecpHCFA];
					}
					else if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
						cyTotalStartup -= m_aryPrices[ecpLetters];
					}
					cyTotalStartup -= m_aryPrices[ecpWorkstation];
				}
				cyTotalStartup -= m_aryPrices[ecpTraining];
				//Note:  The support costs of this addon page are *only* calculated from the modules
				//	listed on this page.  Therefore we need to subtract a specific calculation of
				//	what exists on the "standard" form page.
				cyTotalStartup -= (PKG_SPT_MONTHS*CalculateCurrentMonthlySupport_PackageOnly());
				if(m_nTypeID == eptNexStartup) {
					cyTotalStartup += m_aryPrices[ecpSpecialStartup];
				}
				else if(m_nTypeID == eptNexRes) {
					cyTotalStartup += m_aryPrices[ecpSpecialResident];
				}

				ADDMERGEDATA("Internal_Prop_Startup_Total,",			StripUSCents(FormatCurrencyForInterface(cyTotalStartup)))
			}
		}

		ADDMERGEDATA("Internal_Prop_Proposal_Expires,",		FormatDateTimeForInterface(VarDateTime(m_pickerExpireDate.GetValue()), 0, dtoDate))

		//Special data - checkboxes.  The merge engine now handles special fields of 'NXCHECKBOXFORMFIELDCHECKED' and 'NXCHECKBOXFORMFIELDUNCHECKED'.
		//	We just need to specify which value is set at this time.
		CString strChecked = "NXCHECKBOXFORMFIELDCHECKED";
		CString strUnchecked = "NXCHECKBOXFORMFIELDUNCHECKED";

		CString strCosmBilling = "";
		CString strHCFA = "";
		CString strEBilling = "";
		CString strLetters = "";
		CString strQuotes = "";
		CString strTracking = "";
		CString strNexForms = "";
		// (d.thompson 2009-08-10) - PLID 35152
		CString strNexWebLeads = "";
		CString strNexWebPortal = "";
		// (d.lange 2010-12-03 09:53) - PLID 41711 - Moved ERemittance and EEligibility to financial area
		CString strERemittance = "";
		CString strEEligibility = "";
		
		if(m_bSplitMerge){
			if(m_nMergeCount == 1){
				strCosmBilling = strUnchecked;
				strHCFA = strUnchecked;
				strEBilling = strUnchecked;
				strLetters = strUnchecked;
				strQuotes = strUnchecked;
				strTracking = strUnchecked;
				strNexForms = strUnchecked;
				strERemittance = strUnchecked;
				strEEligibility = strUnchecked;

				if(IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS)){
					strNexWebLeads = strChecked;
				}else{
					strNexWebLeads = strUnchecked;
				}
				if(IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL)){
					strNexWebPortal = strChecked;
				}else{
					strNexWebPortal = strUnchecked;
				}
			}else{
				strNexWebLeads = strUnchecked;
				strNexWebPortal = strUnchecked;
				strCosmBilling = IsDlgButtonChecked(IDC_CHK_BILLING) ? strChecked : strUnchecked;
				strHCFA = IsDlgButtonChecked(IDC_CHK_HCFA) ? strChecked : strUnchecked;
				strEBilling = IsDlgButtonChecked(IDC_CHK_EBILLING) ? strChecked : strUnchecked;
				strLetters = IsDlgButtonChecked(IDC_CHK_LETTERS) ? strChecked : strUnchecked;
				strQuotes = IsDlgButtonChecked(IDC_CHK_QUOTES) ? strChecked : strUnchecked;
				strTracking = IsDlgButtonChecked(IDC_CHK_TRACKING) ? strChecked : strUnchecked;
				strNexForms = IsDlgButtonChecked(IDC_CHK_NEXFORMS) ? strChecked : strUnchecked;
				strERemittance = IsDlgButtonChecked(IDC_CHK_EREMITTANCE) ? strChecked : strUnchecked;
				strEEligibility = IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) ? strChecked : strUnchecked;
			}
		}else{
			strNexWebLeads = IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS) ? strChecked : strUnchecked;
			strNexWebPortal = IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL) ? strChecked : strUnchecked;
			strCosmBilling = IsDlgButtonChecked(IDC_CHK_BILLING) ? strChecked : strUnchecked;
		    strHCFA = IsDlgButtonChecked(IDC_CHK_HCFA) ? strChecked : strUnchecked;
			strEBilling = IsDlgButtonChecked(IDC_CHK_EBILLING) ? strChecked : strUnchecked;
			strLetters = IsDlgButtonChecked(IDC_CHK_LETTERS) ? strChecked : strUnchecked;
			strQuotes = IsDlgButtonChecked(IDC_CHK_QUOTES) ? strChecked : strUnchecked;
			strTracking = IsDlgButtonChecked(IDC_CHK_TRACKING) ? strChecked : strUnchecked;
			strNexForms = IsDlgButtonChecked(IDC_CHK_NEXFORMS) ? strChecked : strUnchecked;
			strERemittance = IsDlgButtonChecked(IDC_CHK_EREMITTANCE) ? strChecked : strUnchecked;
			strEEligibility = IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) ? strChecked : strUnchecked;
		}
		
			
		

		//DRT 5/27/2008 - PLID 29493 - If we're on a resident or startup package, and they don't have HCFA,
		//	and do have Letter Writing, we do not want this to merge, it will go on page 1.
		if( (m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup) && !IsDlgButtonChecked(IDC_CHK_HCFA) && IsDlgButtonChecked(IDC_CHK_LETTERS)) {
			strLetters = "";
		}

		ADDMERGEDATA("Internal_Prop_Check_CosmBilling,",	strCosmBilling)
		ADDMERGEDATA("Internal_Prop_Check_HCFA,",			strHCFA)
		ADDMERGEDATA("Internal_Prop_Check_EBilling,",		strEBilling)
		ADDMERGEDATA("Internal_Prop_Check_Letters,",		strLetters)
		ADDMERGEDATA("Internal_Prop_Check_Quotes,",			strQuotes)
		ADDMERGEDATA("Internal_Prop_Check_Tracking,",		strTracking)
		ADDMERGEDATA("Internal_Prop_Check_NexForms,",		strNexForms)
		// (d.thompson 2009-08-10) - PLID 35152 - NexWeb
		ADDMERGEDATA("Internal_Prop_Check_NexWeb_Leads,",		strNexWebLeads)
		ADDMERGEDATA("Internal_Prop_Check_NexWeb_Portal,",		strNexWebPortal)
		// (d.lange 2010-12-03 09:53) - PLID 41711 - Moved ERemittance and EEligibility to financial area
		ADDMERGEDATA("Internal_Prop_Check_ERemittance,",	strERemittance)
		ADDMERGEDATA("Internal_Prop_Check_EEligibility,",	strEEligibility)

		// (d.thompson 2009-08-10) - PLID 35152 - k.majeed instructed me NOT to include the addtl domains
		//	on the proposal, they're apparently rarely used.  So if they get selected on the form, we want
		//	to warn the merger.  They will come out in the totals properly, but they just won't display.  Thus
		//	if someone went through the whole form and totalled them up, it won't add up.
		long nNexWebAddtlDomains = GetDlgItemInt(IDC_NUM_NEXWEB_ADDTL_DOMAINS);
		if(nNexWebAddtlDomains > 0 && m_nMergeCount == 0) {	// (d.lange 2010-04-16 16:38) - PLID 37999 - make sure it displays on first merge
			AfxMessageBox("You have selected additional NexWeb domains on your proposal.  However, these domains "
				"do not print line items on current proposal forms.  You will need to make a manual note of the additional cost "
				"on the proposal.  These values are properly added into your totals, only the line items are missing.");
		}

		//DRT 9/30/2008 - PLID 31395 - Add descriptions for std vs ent so they know what they are buying.
		{
			CString strEMR = "(Enterprise Edition)", strSched = "(Enterprise Edition)";
			if(IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD)) {
				strSched = "(Standard Edition)";
			}
			if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
				strEMR = "(Standard Edition)";
			}
			// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic
			if(IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC)) {
				strEMR = "(Cosmetic Edition)";
			}

			ADDMERGEDATA("Internal_Prop_Label_Sched_Edition,",		strSched)
			ADDMERGEDATA("Internal_Prop_Label_EMR_Edition,",		strEMR)
		}

		//We want to merge the quote number as a field too
		CString strQuoteNum;
		GetDlgItemText(IDC_QUOTE_ID, strQuoteNum);
		ADDMERGEDATA("Internal_Prop_QuoteNum,",	strQuoteNum);			

		// (d.thompson 2009-08-27) - PLID 35365 - If we have enabled financing, add some merge text for it.
		CString strSymbol;
		CString strLegalText;
		if(m_bIsFinancing) {
			strSymbol = "†";
			//I load this from a property, because I'm 99% certain that once it goes live someone in sales will
			//	make it be changed.
			strLegalText = GetRemotePropertyText("InternalPropFinancingLegalText", 
				"† Special third party financing subject to credit approval.  If declined, standard payment terms apply.",
				0, "<None>", true);
		}
		ADDMERGEDATA("Internal_Prop_Financing_Symbol,", strSymbol);
		ADDMERGEDATA("Internal_Prop_Financing_LegalText,", strLegalText);

		//Trim trailing comma off each
		opmd.m_strHeaders.TrimRight(",");
		opmd.m_strData.TrimRight(",");

		//Pass in our header/data information and a function to handle it to the merge engine.
		mi.m_pfnCallbackExtraFields = COpportunityProposal__ExtraMergeFields;
		mi.m_pCallbackParam = &opmd;

		//Perform the actual merge
		mi.MergeToWord(strPathToDocument, strMergeT);

		// (d.lange 2010-04-06) - PLID 37999 - when splitting a merge, if we already merged once (for the client) we need to set the bool back to false
		if(m_nMailSentID != -1 && IsDlgButtonChecked(IDC_CHK_SPLITEMR)) {
			m_bSplitMerge = false;
		}

		//TODO:  There really is no way to know if this succeeded or failed (as long as it's not an exception).  If anything
		//	bad happened during the merge, it just returns, we don't know if it created a document or not.
		_RecordsetPtr prsMail = CreateRecordset("SELECT MAX(MailID) AS MaxID FROM MailSent WHERE PersonID = %li", m_nPatientID);
		if(!prsMail->eof) {
			m_nMailSentID = AdoFldLong(prsMail, "MaxID");

			//We must update the record to know about this mailsent id
			ExecuteSql("UPDATE OpportunityProposalsT SET MailSentID = %li WHERE ID = %li", m_nMailSentID, m_nID);
		}

		

		//DRT 5/23/2008 - PLID 29493 - We now sometimes have secondary documents for AddOn forms.
		if(!strPathToDocumentAddons.IsEmpty()) {
			mi.MergeToWord(strPathToDocumentAddons, strMergeT);
		}

		//Now that we have merged, we want to disable the interface -- everything is saved & permanent!
		DisableAllInterface();

		//No reason to keep this dialog open once you merge, just close it (we already saved, so we cancel)
		OnCancel();

		m_nMergeCount++;

	} NxCatchAll("Error in OnMergeProposal");
}
*/

// (d.lange 2010-04-02) - PLID 37999 - remove all EMR related fields for client merge
CArray<int, int>& COpportunityProposalDlg::SplitEMRForClient(CArray<int, int> &aryModules)
{
	//Basically, turn off any EMR related fields in the array
	//EMR Training Days, EMR Workstations, EMR cost, Support Cost
	for(int nIndex = 0; nIndex < ecmTotalModules; nIndex++)
	{
		if(nIndex == ecmEMR) {
			aryModules[ecmEMR] = 0;
		}

		if(nIndex == ecmEMRWorkstations) {
			aryModules[ecmEMRWorkstations] = 0;
		}

		if(nIndex == ecmEMRTraining) {
			aryModules[ecmEMRTraining] = 0;
		}

		if(nIndex == ecmLeadGen) {
			aryModules[ecmLeadGen] = 0;
		}

		if(nIndex == ecmPatientPortal) {
			aryModules[ecmPatientPortal] = 0;
		} 

		if(nIndex == ecmAddDomains) {
			aryModules[ecmAddDomains] = 0;
		}
		
	}

	return aryModules;
}

// (d.lange 2010-04-02) - PLID 37999 - remove all EMR related fields for 3rd party merge
CArray<int, int>& COpportunityProposalDlg::SplitEMRForThirdParty(CArray<int, int> &aryModules)
{
	//Basically, turn off everything in the array except anything EMR related
	//EMR Training Days, EMR Workstations, EMR cost, Support Cost
	for(int nIndex = 0; nIndex < ecmTotalModules; nIndex++)
	{
		//Do not change values related to EMR
		if((nIndex == ecmEMR) || (nIndex == ecmEMRWorkstations) || (nIndex == ecmEMRTraining) || (nIndex == ecmLeadGen) || 
			(nIndex == ecmPatientPortal) || (nIndex == ecmSupportMonths) || (nIndex == ecmAddDomains) || (nIndex == ecmStdEMR)
			|| (nIndex == ecmCosmeticEMR)		// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic EMR
			)
		{	// (d.lange 2010-04-07) - PLID 38096 - we don't want to alter discount values
			//Do nothing
		}else{
			aryModules[nIndex] = 0;
		}
	}

	return aryModules;
}

//This function returns whatever is currently in the "grand total" box.  This function is primarily designed
//	to be called from an external source for generating the total price.  If you are doing so, please
//	remember to call SetImmediateLoad(true) so that all loading takes place immediately.
COleCurrency COpportunityProposalDlg::CalculateSilentTotal()
{
	COleCurrency cyTotal(0, 0);
	ADDAMOUNT(IDC_GRAND_TOTAL, cyTotal);

	return cyTotal;
}

void COpportunityProposalDlg::DisableAllInterface()
{
	m_pickerProposalDate.EnableWindow(FALSE);
	m_pickerExpireDate.EnableWindow(FALSE);

	(GetDlgItem(IDC_CHK_SCHED))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_1LICENSE))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_BILLING))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_HCFA))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_EBILLING))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_LETTERS))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_QUOTES))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_TRACKING))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_NEXFORMS))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_INV))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_NEXSPA))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_ASC))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_MIRROR))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_UNITED))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_HL7))->EnableWindow(FALSE);
	// (d.thompson 2013-07-08) - PLID 57334
	(GetDlgItem(IDC_CHK_HIE))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_INFORM))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_QUICKBOOKS))->EnableWindow(FALSE);
	//DRT 2/12/2008 - PLID 28881 - Added ERemittance and EEligibility
	(GetDlgItem(IDC_CHK_EREMITTANCE))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_EELIGIBILITY))->EnableWindow(FALSE);
	(GetDlgItem(IDOK))->EnableWindow(FALSE);
	(GetDlgItem(IDC_MERGE_PROPOSAL))->EnableWindow(FALSE);
	// (d.thompson 2009-02-02) - PLID 32890 - C&A
	(GetDlgItem(IDC_CHK_C_AND_A))->EnableWindow(FALSE);
	// (d.thompson 2009-11-06) - PLID 36123 - NexPhoto
	(GetDlgItem(IDC_CHK_NEXPHOTO))->EnableWindow(FALSE);
	// (d.lange 2010-3-31) - PLID 37956 - disable Split EMR checkbox
	(GetDlgItem(IDC_CHK_SPLITEMR))->EnableWindow(FALSE);
	// (d.lange 2010-3-31) - PLID 38096 - Disable apply emr discount checkbox
	(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(FALSE);

	// (d.lange 2010-04-08) - PLID 37999 - set to read only after merge
	((CNxEdit*)GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_WORKSTATIONS))->SetReadOnly(TRUE);
	// (d.thompson 2012-10-12) - PLID 53155
	((CNxEdit*)GetDlgItem(IDC_NUM_WORKSTATIONS_TS))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_DOCTORS))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_PDA))->SetReadOnly(TRUE);
	// (d.thompson 2009-11-13) - PLID 36124
	((CNxEdit*)GetDlgItem(IDC_NUM_NEXSYNC))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_EMR))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_TRAINING))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_EMR_TRAINING))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_SUPPORT))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_CONVERSION))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_TRAVEL))->SetReadOnly(TRUE);
	((CNxEdit*)GetDlgItem(IDC_NUM_EXTRADB))->SetReadOnly(TRUE);		//DRT 2/13/2008 - PLID 28905

	//DRT 9/24/2008 - PLID 31395 - Disable Std buttons
	(GetDlgItem(IDC_CHK_SCHED_STANDARD))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_EMR_STANDARD))->EnableWindow(FALSE);
	// (d.thompson 2013-07-05) - PLID 57333 - Cosmetic
	(GetDlgItem(IDC_CHK_EMR_COSMETIC))->EnableWindow(FALSE);

	// (d.thompson 2009-02-02) - PLID 32911 - Forgot these
	(GetDlgItem(IDC_CHECK_ALL_FINANCIAL))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHECK_ALL_COSMETIC))->EnableWindow(FALSE);
	// (d.thompson 2010-03-29) - PLID 37951 - Forgot these... gotta get better at testing here.
	(GetDlgItem(IDC_CHK_NEXWEB_LEADS))->EnableWindow(FALSE);
	(GetDlgItem(IDC_CHK_NEXWEB_PORTAL))->EnableWindow(FALSE);
	((CNxEdit*)GetDlgItem(IDC_NUM_NEXWEB_ADDTL_DOMAINS))->SetReadOnly(TRUE);


	m_btnChangeDiscount.EnableWindow(FALSE);

	//I'm not entirely sure why, but if the focus is on IDOK, and you disable that button, you
	//	can no longer press the Escape key to close the dialog.  Setting the focus to Cancel
	//	once OK is disabled fixes that problem.
	GetDlgItem(IDCANCEL)->SetFocus();
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(COpportunityProposalDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(COpportunityProposalDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void COpportunityProposalDlg::CalculateTrainingFromCurrentInterface(long &nDays, long &nEmrDays, long &nTrips)
{
	//Converted from Meikin's training calculator excel sheet.  Based on options selected, calculate
	//	how many training days & trips are needed.

	//DRT 7/25/2007 - Updated to newest numbers

	//DRT 6/10/2008 - PLID 30346 - For NexRes and NexStartup packages, a PM trip is only included if there's an 
	//	addon module.  The base package for these includes:
	//	sched, billing, (hcfa or LW).  If anything else is added, it then requires travel.
	//Note:  Per m.rosenberg, the WS or NumDoctors values does NOT influence travel.
	bool bReqTravel = false;

	// (d.thompson 2010-12-29) - PLID 41946 - New training calculator values

	//
	//Practice management training

	double dblHours = 0;
	dblHours += 5;		//Patients Module	5	(always selected)
	if(IsDlgButtonChecked(IDC_CHK_SCHED))
		dblHours += 3;	//Scheduler			3
	// (d.thompson 2010-12-29) - PLID 41946 - Upped from 3 to 4
	if(IsDlgButtonChecked(IDC_CHK_BILLING))
		dblHours += 4;	//Cosmetic billing	4
	// (d.thompson 2010-12-29) - PLID 41946 - Upped from 3 to 8
	if(IsDlgButtonChecked(IDC_CHK_HCFA))
		dblHours += 8;	//HCFA				8
	// (d.thompson 2010-12-29) - PLID 41946 - Upped Quotes from 2 to 3
	if(IsDlgButtonChecked(IDC_CHK_QUOTES)) {
		dblHours += 4;	//Quotes			3		Marketing 1 (these are sold together)
		bReqTravel = true;
	}
	// (d.thompson 2010-12-29) - PLID 41946 - Upped from 3 to 4
	if(IsDlgButtonChecked(IDC_CHK_TRACKING)) {
		dblHours += 4;//Tracking			4
		bReqTravel = true;
	}
	if(IsDlgButtonChecked(IDC_CHK_NEXFORMS)) {
		dblHours += 3;	//NexForms			3
		bReqTravel = true;
	}
	// (d.thompson 2010-12-29) - PLID 41946 - Upped from 1 to 2
	if(IsDlgButtonChecked(IDC_CHK_LETTERS)) {
		dblHours += 2;	//Letters			2
		//If HCFA & LW, it's an addon, if no HCFA, it's part of the base.
		if(IsDlgButtonChecked(IDC_CHK_HCFA)) {
			bReqTravel = true;
		}
	}
	if(IsDlgButtonChecked(IDC_CHK_INV)) {
		dblHours += 2;//Inv					2
		bReqTravel = true;
	}
	if(IsDlgButtonChecked(IDC_CHK_ASC)) {
		dblHours += 4;	//ASC				4
		bReqTravel = true;
	}
	//If 10+ staff, add a day
	//(d.thompson 2009-05-21) - PLID 34319 - New doctor instructions:
	//	If 10 or more workstations are purchased and only 1 doctor, add 1 day to training.
	//	OR (ignoring workstations) If there is more than 1 doctor, add 1 day to training for each additional doctor.
	{
		// (d.thompson 2012-10-12) - PLID 53155 - added TS Workstations
		long nWS = GetDlgItemInt(IDC_NUM_WORKSTATIONS) + GetDlgItemInt(IDC_NUM_WORKSTATIONS_TS);
		//This number is the ADDITIONAL doctors (there is 1 built in by default)
		long nDocs = GetDlgItemInt(IDC_NUM_DOCTORS);
		if(nDocs == 0) {
			//If there is only 1 doctor, only add more training if there are 10 or more workstations
			if(nWS >= 10) {
				dblHours += 8;
			}
		}
		else {
			//Add a day for each doctor
			dblHours += (8.0 * nDocs);
		}
	}

	// (d.thompson 2009-02-02) - PLID 32890 - 10 hours for C&A
	if(IsDlgButtonChecked(IDC_CHK_C_AND_A)) {
		dblHours += 10;	//C&A
		bReqTravel = true;
	}
	// (d.thompson 2009-11-06) - PLID 36123 - 2 hours for NexPhoto
	// (d.thompson 2009-11-13) - PLID 36123 - After that "final" decision, they randomly
	//	changed their minds to 4 hours.
	if(IsDlgButtonChecked(IDC_CHK_NEXPHOTO)) {
		dblHours += 4;
		bReqTravel = true;
	}

	//Add all full days
	nDays = (int)(dblHours/8.0);

	//If >= 2 hours on remainder, round up to another full day.  Must calculate in integers, just cut off any half hours extra
	if((int)dblHours % 8 >= 2)
		nDays++;

	//1 trip for practice management training
	nTrips = 1;

	//DRT 6/10/2008 - PLID 30346 - For resident & startup, if they just have the base package, 
	//	there is no trip required, internet training is provided.
	if(!bReqTravel && (m_nTypeID == eptNexRes || m_nTypeID == eptNexStartup)) {
		nTrips--;
	}

	//
	//EMR Training
	dblHours = 0.0;	//reset for new calc
	long nEMR = GetDlgItemInt(IDC_NUM_EMR);
	if(nEMR > 0) {
		// (d.thompson 2008-12-01) - PLID 32279 - For EMR Standard, we are now giving 2 days for the first
		//	doctor, and 1 day for each additional doctor.  These days are online/phone only, and require no travel.
		// (d.thompson 2009-02-24 16:35) - PLID 33227 - Changed EMR Standard to 1 day of training for the first dr, 
		//		1 day for each additional.
		// (d.thompson 2009-02-24 17:07) - PLID 33227 garbaged -- we're back to 2 days.  I want to keep this documented, 
		//		as there's a good bit of argument back and forth on which is preferred.
		if(IsDlgButtonChecked(IDC_CHK_EMR_STANDARD)) {
			//Training configuration for EMR Standard
			dblHours = 16.0;		//First doctor
			dblHours += (nEMR - 1) * 8.0;	//1 day for each additional doctor past the first

			nEmrDays = (int)(dblHours/8.0);

			//no trip for standard
		}
		// (d.thompson 2013-07-05) - PLID 57333 - cosmetic emr
		else if(IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC)) {
			dblHours = 16.0 * nEMR;		//2 days of training per doc
			nEmrDays = (int)(dblHours/8.0);

			//no trip for cosmetic
		}
		else {
			//Training configuration for EMR Enterprise
			//As of 7/25/2007, this is now 2 days per doctor
			// (d.thompson 2012-06-25) - PLID 51170 - As of 6/25/2012, this was reduced to "3 days total", so I bumped 
			//	this one down and the left the below at 2 days (for 3 total - no, I don't know why they're split).
			dblHours = nEMR * 8.0;

			//We additionally add in 2 day phone / online training per doctor
			dblHours += (16.0 * nEMR);

			// (z.manning, 11/26/2007) - PLID 28159 - We now track EMR training days separately.
			nEmrDays = (int)(dblHours/8.0);

			// (j.luckoski 4/10/12) - PLID 49492 - For Doctors in specific specialties, don't add EMR travel time if < 3 doctors.
			long nDocs = GetDlgItemInt(IDC_NUM_DOCTORS);	

			_RecordsetPtr prsEMRTravel = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT * FROM CustomListDataT INNER JOIN SpecialtyNoTravelT "
				"ON SpecialtyNoTravelT.SpecialtyID = CustomListDataT.CustomListItemsID WHERE FieldID = 24 AND CustomListDataT.PersonID = {INT}", m_nPatientID);

			// SHould be < 2 because that means when it is 2 (which is three docs) than it goes to extra trip.
			if(nDocs < 2 && !prsEMRTravel->eof) {
				nTrips += 0;
			}
			else {
				//1 more trip for EMR
				nTrips += 1;
			}
		}
	}
}

void COpportunityProposalDlg::OnChangeDiscount() 
{
	try {
		COpportunityAddDiscountDlg dlg(this);
		COleCurrency cySubTotal(0, 0);
		ADDAMOUNT(IDC_SUBTOTAL_SEL, cySubTotal);
		dlg.m_cySubTotal = cySubTotal;
		if(dlg.DoModal() == IDOK) {
			//Update our text label
			SetDlgItemText(IDC_DISCOUNT_TOTAL_DOLLAR, FormatCurrencyForInterface(dlg.m_cyFinalDiscount, TRUE));
			UpdateAllTotals();

			//In case there was an override of the user, we need to get the values from this dialog
			SetDlgItemText(IDC_DISCOUNT_USERNAME, dlg.m_strDiscountUserName);
			//Save the user id
			m_nCurrentDiscountUserID = dlg.m_nDiscountUserID;

			// (d.lange 2010-04-08) - PLID 38096 - Disable if a discount is not applied
			if(IsDlgButtonChecked(IDC_CHK_SPLITEMR)) {
				(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->ShowWindow(TRUE);
				(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(TRUE);
			}
			
		}

	} NxCatchAll("Error in OnChangeDiscount");
}

void COpportunityProposalDlg::OnCheckAllFinancial() 
{
	try {
		CheckDlgButton(IDC_CHK_BILLING, TRUE);
		CheckDlgButton(IDC_CHK_HCFA, TRUE);
		CheckDlgButton(IDC_CHK_EBILLING, TRUE);
		// (d.lange 2010-12-03 09:11) - PLID 41711 - Moved E-Remittance and E-Eligibility to financial area
		CheckDlgButton(IDC_CHK_EELIGIBILITY, TRUE);
		CheckDlgButton(IDC_CHK_EREMITTANCE, TRUE);

		//Handle this package
		ReflectFinancialPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmBilling] = (IsDlgButtonChecked(IDC_CHK_BILLING) ? 1 : 0);
		m_aryModules[ecmHCFA] = (IsDlgButtonChecked(IDC_CHK_HCFA) ? 1 : 0);
		m_aryModules[ecmEBilling] = (IsDlgButtonChecked(IDC_CHK_EBILLING) ? 1 : 0);
		// (d.lange 2010-12-03 09:11) - PLID 41711 - Moved E-Remittance and E-Eligibility to financial area
		m_aryModules[ecmEEligibility] = (IsDlgButtonChecked(IDC_CHK_EELIGIBILITY) ? 1 : 0);
		m_aryModules[ecmERemittance] = (IsDlgButtonChecked(IDC_CHK_EREMITTANCE) ? 1 : 0);

	} NxCatchAll("Error in OnCheckAllFinancial");
}

void COpportunityProposalDlg::OnCheckAllCosmetic() 
{
	try {
		CheckDlgButton(IDC_CHK_LETTERS, TRUE);
		CheckDlgButton(IDC_CHK_QUOTES, TRUE);
		CheckDlgButton(IDC_CHK_TRACKING, TRUE);
		CheckDlgButton(IDC_CHK_NEXFORMS, TRUE);

		//Handle this package
		ReflectCosmeticPackage();

		//This affects training
		UpdateTraining();

		//Update the total fields
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmLetterWriting] = (IsDlgButtonChecked(IDC_CHK_LETTERS) ? 1 : 0);
		m_aryModules[ecmQuotesMarketing] = (IsDlgButtonChecked(IDC_CHK_QUOTES) ? 1 : 0);
		m_aryModules[ecmNexTrak] = (IsDlgButtonChecked(IDC_CHK_TRACKING) ? 1 : 0);
		m_aryModules[ecmNexForms] = (IsDlgButtonChecked(IDC_CHK_NEXFORMS) ? 1 : 0);

	} NxCatchAll("Error in OnCheckAllCosmetic");
}

void COpportunityProposalDlg::OnChkSchedStandard() 
{
	try {
		ReflectPriceArray();
		ReflectSchedulerPackage();
		UpdateAllTotals();
		
		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmStdScheduler] = (IsDlgButtonChecked(IDC_CHK_SCHED_STANDARD) ? 1 : 0);

	} NxCatchAll("Error in OnChkSchedStandard");
}

void COpportunityProposalDlg::OnChkEmrStandard() 
{
	try {
		// (d.thompson 2013-07-05) - PLID 57333 - Make sure emr cosmetic is removed
		CheckDlgButton(IDC_CHK_EMR_COSMETIC, FALSE);

		ReflectPriceArray();
		OnChangeNumEmr();
		UpdateAllTotals();

		// (d.lange 2010-4-19) - PLID 38016 - Update the module array
		m_aryModules[ecmStdEMR] = (IsDlgButtonChecked(IDC_CHK_EMR_STANDARD) ? 1 : 0);

	} NxCatchAll("Error in OnChkSchedStandard");
}

// (d.thompson 2013-07-05) - PLID 57333 - cosmetic emr
void COpportunityProposalDlg::OnChkEmrCosmetic() 
{
	try {
		//Make sure emr std is removed
		CheckDlgButton(IDC_CHK_EMR_STANDARD, FALSE);

		ReflectPriceArray();
		OnChangeNumEmr();
		UpdateAllTotals();

		m_aryModules[ecmCosmeticEMR] = (IsDlgButtonChecked(IDC_CHK_EMR_COSMETIC) ? 1 : 0);

	} NxCatchAll(__FUNCTION__);
}

void COpportunityProposalDlg::OnBnClickedChkCAndA()
{
	try {
		//Handle this "group" (this is not a package cost set)
		ReflectOtherModules();

		UpdateTraining();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmCandA] = (IsDlgButtonChecked(IDC_CHK_C_AND_A) ? 1 : 0);

	} NxCatchAll("Error in OnChkSchedStandard");
}

void COpportunityProposalDlg::OnBnClickedChkNexwebLeads()
{
	try {
		ReflectPriceArray();
		ReflectNexWebPackage();
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmLeadGen] = (IsDlgButtonChecked(IDC_CHK_NEXWEB_LEADS) ? 1 : 0);

	} NxCatchAll(__FUNCTION__);
}

void COpportunityProposalDlg::OnBnClickedChkNexwebPortal()
{
	try {
		ReflectPriceArray();
		ReflectNexWebPackage();
		UpdateAllTotals();
		
		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmPatientPortal] = (IsDlgButtonChecked(IDC_CHK_NEXWEB_PORTAL) ? 1 : 0);

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-03-30 - PLID 37956 - EMR Split checkbox
void COpportunityProposalDlg::OnBnClickedChkEMRSplit()
{
	try {
		if (IsDlgButtonChecked(IDC_CHK_SPLITEMR)) {
			//Prompt the user about EMR Workstations
			MessageBox("Selecting to split EMR expenses will separate the EMR expenses from the proposal "
				"and create a separate page on merge. "
				"\r\r"
				"From the total number of workstations, please enter the number that can be classified as EMR workstations. "
				, NULL, MB_ICONEXCLAMATION);
			//Enable the EMR Workstation Editbox
			(GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->EnableWindow(TRUE);
			// (d.lange 2010-04-08) - PLID 38096 - disable checkbox if no discount is applied
			CString strUsername;
			GetDlgItemTextA(IDC_DISCOUNT_USERNAME, strUsername);
			if(strUsername.IsEmpty()){
				(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->ShowWindow(TRUE);
				(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(FALSE);
			}else{
				(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->ShowWindow(TRUE);
				(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->EnableWindow(TRUE);
			}
		}else{
			(GetDlgItem(IDC_NUM_EMRWORKSTATIONS))->EnableWindow(FALSE);
			// (d.lange 2010-04-08) - PLID 38096 - Do not show the checkbox since split emr is not selected
			(GetDlgItem(IDC_CHK_DISCOUNT_EMR))->ShowWindow(FALSE);
			// (d.lange 2010-09-01 15:52) - PLID 40333 - When disabled lets set the editbox value to 0, then reflect the change
			SetDlgItemInt(IDC_NUM_EMRWORKSTATIONS, (long)0);
			OnChangeNumEmrWorkstations();
		}

	} NxCatchAll(__FUNCTION__);
}

void COpportunityProposalDlg::OnBnClickedChkDiscountEMR()
{
	try {

	} NxCatchAll(__FUNCTION__);
}

void COpportunityProposalDlg::OnEnChangeNumNexwebAddtlDomains()
{
	try {
		//We set the value to the number of domains * the price per
		long nValue = GetDlgItemInt(IDC_NUM_NEXWEB_ADDTL_DOMAINS);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpNexWebAddtlDomains];

		SetDlgItemText(IDC_SEL_NEXWEB_ADDTL_DOMAINS, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-19) - PLID 38016 - Update the module array
		m_aryModules[ecmAddDomains] = GetDlgItemInt(IDC_NUM_NEXWEB_ADDTL_DOMAINS);

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2009-11-06) - PLID 36123 
void COpportunityProposalDlg::OnBnClickedChkNexphoto()
{
	try {
		//Handle this "group" (this is not a package cost set)
		ReflectOtherModules();

		//Update any training
		UpdateTraining();

		//Update the total
		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmNexPhoto] = (IsDlgButtonChecked(IDC_CHK_NEXPHOTO) ? 1 : 0);

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2009-11-13) - PLID 36124
void COpportunityProposalDlg::OnEnChangeNumNexsync()
{
	try {
		//We set the value to the number of NexSync * the price per
		long nValue = GetDlgItemInt(IDC_NUM_NEXSYNC);

		COleCurrency cyTotal = nValue * m_aryPrices[ecpNexSync];

		SetDlgItemText(IDC_SEL_NEXSYNC, FormatCurrencyForInterface(cyTotal));

		UpdateAllTotals();

		// (d.lange 2010-4-1) - PLID 38016 - Update the module array
		m_aryModules[ecmNexSync] = GetDlgItemInt(IDC_NUM_NEXSYNC);

	} NxCatchAll(__FUNCTION__);	
}

// (d.lange 2010-04-19 11:47) - PLID 38029 - Remove '$' from the string and replace with nothing
CString COpportunityProposalDlg::RemoveDollarSign(CString strDollarValue)
{
	if(strDollarValue.Find("$") != -1){
		strDollarValue.Replace("$", "");
		return strDollarValue;
	}else{
		return strDollarValue;
	}
}

void COpportunityProposalDlg::SetTravelType() 
{
	try {
		_RecordsetPtr rs = CreateParamRecordset("SELECT City, Country FROM PersonT WHERE ID = {INT}", m_nPatientID);

		CString strCity, strCountry;
		if(!rs->eof) {
			strCity = AdoFldString(rs->Fields, "City", "");
			strCountry = AdoFldString(rs->Fields, "Country", "");

			if(!strCountry.IsEmpty() && strCountry.CompareNoCase("Canada") != 0 && strCountry.CompareNoCase("United States") != 0) {
				m_nTravelType = ettInternational;
			}else if(strCountry.CompareNoCase("Canada") == 0) {
				m_nTravelType = ettCanada;
			}else if(strCity.CompareNoCase("New York") == 0 || strCity.CompareNoCase("New York City") == 0) {
				m_nTravelType = ettNewYorkCity;
			}else {
				m_nTravelType = ettDomestic;
			}
		}
		rs->Close();

	} NxCatchAll(__FUNCTION__);
}