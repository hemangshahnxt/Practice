// ConfigBillColumnsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigBillColumnsDlg.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDrawingUtils.h"
#include "AdministratorRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CConfigBillColumnsDlg dialog


CConfigBillColumnsDlg::CConfigBillColumnsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigBillColumnsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigBillColumnsDlg)
		m_bBillColumnsChanged = FALSE;
		m_bQuoteColumnsChanged = FALSE;
		// (j.dinatale 2010-11-02) - PLID 41279 - dont worry about billing tab changes
		//m_bBillTabColumnsChanged = FALSE;
		m_nLocationID = -1;
	//}}AFX_DATA_INIT
}


void CConfigBillColumnsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigBillColumnsDlg)		
	/*DDX_Control(pDX, IDC_CHECK_BILLTAB_PROVIDER, m_checkBillTabProvider);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_POSNAME, m_checkBillTabPOSName);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_POSCODE, m_checkBillTabPOSCode);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_LOCATION, m_checkBillTabLocation);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_INPUT_DATE, m_checkBillTabInputDate);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_CREATEDBY, m_checkBillTabCreatedBy);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_CHARGE_DATE, m_checkBillTabChargeDate);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_BILLID, m_checkBillTabBillID);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_DISCOUNT_ICON, m_checkBillTabDiscountIcon);*/
	DDX_Control(pDX, IDC_CHECK_BILL_SERVICE_DATE_TO, m_checkBillServiceDateTo);
	DDX_Control(pDX, IDC_CHECK_QUOTE_MOD4, m_checkQuoteMod4);
	DDX_Control(pDX, IDC_CHECK_QUOTE_MOD3, m_checkQuoteMod3);
	DDX_Control(pDX, IDC_CHECK_QUOTE_MOD2, m_checkQuoteMod2);
	DDX_Control(pDX, IDC_CHECK_QUOTE_MOD1, m_checkQuoteMod1);
	DDX_Control(pDX, IDC_CHECK_BILL_MOD4, m_checkBillMod4);
	DDX_Control(pDX, IDC_CHECK_BILL_MOD3, m_checkBillMod3);
	DDX_Control(pDX, IDC_CHECK_BILL_TOTAL_DISCOUNT, m_checkBillTotalDiscount);
	DDX_Control(pDX, IDC_CHECK_BILL_PATIENT_COORDINATOR, m_checkBillPatientCoordinator);
	DDX_Control(pDX, IDC_CHECK_BILL_INPUT_DATE, m_checkBillInputDate);
	DDX_Control(pDX, IDC_CHECK_QUOTE_TAX2, m_checkQuoteTax2);
	DDX_Control(pDX, IDC_CHECK_QUOTE_TAX1, m_checkQuoteTax1);
	DDX_Control(pDX, IDC_CHECK_QUOTE_PROVIDER, m_checkQuoteProvider);
	DDX_Control(pDX, IDC_CHECK_QUOTE_TOTAL_DISCOUNT, m_checkQuoteTotalDiscount);
	DDX_Control(pDX, IDC_CHECK_QUOTE_OUTSIDE_PRACTICE_COST, m_checkQuoteOutsideCost);	
	DDX_Control(pDX, IDC_CHECK_QUOTE_CPT_SUBCODE, m_checkQuoteCPTSubCode);
	DDX_Control(pDX, IDC_CHECK_QUOTE_CPT_CODE, m_checkQuoteCPTCode);
	DDX_Control(pDX, IDC_CHECK_BILL_TYPE_OF_SERVICE, m_checkBillTOS);
	DDX_Control(pDX, IDC_CHECK_BILL_TAX2, m_checkBillTax2);
	DDX_Control(pDX, IDC_CHECK_BILL_TAX1, m_checkBillTax1);
	DDX_Control(pDX, IDC_CHECK_BILL_MOD2, m_checkBillMod2);
	DDX_Control(pDX, IDC_CHECK_BILL_MOD1, m_checkBillMod1);
	DDX_Control(pDX, IDC_CHECK_BILL_CPT_DIAGCS, m_checkBillDiagCs);
	DDX_Control(pDX, IDC_CHECK_BILL_CPT_SUB_CODE, m_checkBillCPTSubCode);
	DDX_Control(pDX, IDC_CHECK_BILL_CPT_CODE, m_checkBillCPTCode);
	DDX_Control(pDX, IDC_CHECK_QUOTE_ALLOWABLE, m_checkQuoteAllowable);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	/*DDX_Control(pDX, IDC_CHECK_BILLTAB_DIAGCODE1, m_checkBillTabDiagCode1);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_DIAGCODEDESC, m_checkBillTabDiagCode1WithDesc);
	DDX_Control(pDX, IDC_CHECK_BILLTAB_DIAGCODELIST, m_checkBillTabAllDiagCodes);*/
	DDX_Control(pDX, IDC_CHECK_BILL_ALLOWABLE, m_checkBillAllowable);
	/*DDX_Control(pDX, IDC_CHECK_BILLTAB_ALLOWABLE, m_checkBillTabAllowable);*/
	DDX_Control(pDX, IDC_CHECK_BILL_CLAIM_PROVIDER, m_checkBillClaimProvider);
	DDX_Control(pDX, IDC_CHECK_BILL_CPT_CATEGORY, m_checkBillChargeCategory); // (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
	DDX_Control(pDX, IDC_CHECK_QUOTE_CPT_CATEGORY, m_checkQuoteChargeCategory); // (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigBillColumnsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigBillColumnsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigBillColumnsDlg message handlers

BOOL CConfigBillColumnsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29852 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_LocationCombo = BindNxDataListCtrl(this,IDC_CBQ_LOCATION_COMBO,GetRemoteData(),TRUE);
	m_LocationCombo->SetSelByColumn(0,GetCurrentLocation());
	m_nLocationID = GetCurrentLocation();

	// (j.jones 2008-09-17 09:36) - PLID 31412 - the Billing Tab's Bill ID column
	// is always on in Internal, and shouldn't be usable
	/*if(IsNexTechInternal()) {
		m_checkBillTabBillID.EnableWindow(FALSE);
	}*/

	Load();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigBillColumnsDlg::OnOK() 
{
	// (c.haag 2004-06-21 10:23) - PLID 8023 - No point in saving if we haven't changed
	if(m_bBillColumnsChanged|| m_bQuoteColumnsChanged /*|| m_bBillTabColumnsChanged*/) {
		Save();
	}
	
	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CConfigBillColumnsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigBillColumnsDlg)
	ON_EVENT(CConfigBillColumnsDlg, IDC_CBQ_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenCbqLocationCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigBillColumnsDlg::OnSelChosenCbqLocationCombo(long nRow) 
{
	// (j.dinatale 2010-11-02) - PLID 41279 - dont worry about billing tab changes anymore
	if(m_bBillColumnsChanged|| m_bQuoteColumnsChanged /*|| m_bBillTabColumnsChanged*/) {
		if(IDYES == MessageBox("You have changed the settings for the previous location. Do you wish to save those changes?",
			"Practice",MB_ICONQUESTION|MB_YESNO)) {
			Save();
		}
	}

	if(nRow == -1)
		return;

	m_nLocationID = m_LocationCombo->GetValue(nRow,0).lVal;

	Load();
}

void CConfigBillColumnsDlg::Load()
{
	try {

		if(m_nLocationID == -1)
			return;

		//eof is allowed so we must default them all to true

		BOOL bBillServiceDateTo = FALSE;
		BOOL bBillInputDate = FALSE;
		BOOL bBillPatientCoordinator = FALSE;
		BOOL bBillCPTCode = TRUE;
		BOOL bBillCPTSubCode = TRUE;
		BOOL bBillCPTChargeCategory = TRUE;// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		BOOL bBillTOS = TRUE;
		BOOL bBillMod1 = TRUE;
		BOOL bBillMod2 = TRUE;
		BOOL bBillMod3 = FALSE;
		BOOL bBillMod4 = FALSE;
		BOOL bBillDiagCs = TRUE;		
		BOOL bBillTotalDiscount = TRUE;
		BOOL bBillTax1 = TRUE;
		BOOL bBillTax2 = TRUE;
		// (j.jones 2010-08-31 17:42) - PLID 40330 - added bill allowable
		BOOL bBillAllowable = FALSE;

		BOOL bQuoteCPTCode = TRUE;
		BOOL bQuoteCPTSubCode = TRUE;
		BOOL bQuoteCPTChargeCategory = TRUE;// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		BOOL bQuoteProvider = TRUE;
		BOOL bQuoteMod1 = FALSE;
		BOOL bQuoteMod2 = FALSE;
		BOOL bQuoteMod3 = FALSE;
		BOOL bQuoteMod4 = FALSE;
		BOOL bQuoteOutsideCost = TRUE;
		// (j.gruber 2009-03-20 10:36) - PLID 33385 - combined percentoff and discount into total discount
		BOOL bQuoteTotalDiscount = TRUE;		
		BOOL bQuoteTax1 = TRUE;
		BOOL bQuoteTax2 = TRUE;
		// (j.gruber 2009-10-19 13:30) - PLID 36000 - quote allowable
		BOOL bQuoteAllowable = FALSE;

		// (j.jones 2008-09-16 15:41) - PLID 31412 - added support for billing tab settings
		//BOOL bBillTabProvider = TRUE;
		//BOOL bBillTabPOSName = FALSE;
		//BOOL bBillTabPOSCode = FALSE;
		//BOOL bBillTabLocation = FALSE;
		//BOOL bBillTabInputDate = FALSE;
		//BOOL bBillTabCreatedBy = FALSE;
		//BOOL bBillTabChargeDate = FALSE;
		//BOOL bBillTabBillID = FALSE;
		//BOOL bBillTabDiscountIcon = TRUE;

		//// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options
		//BOOL bBillTabDiagCode1 = FALSE;
		//BOOL bBillTabDiagCode1WithDesc = FALSE;
		//BOOL bBillTabAllDiagCodes = FALSE;

		//// (j.jones 2010-08-31 17:53) - PLID 40331 - added bill tab allowable
		//BOOL bBillTabAllowable = FALSE;

		// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
		BOOL bBillClaimProvider = FALSE;

		_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM ConfigBillColumnsT WHERE LocationID = {INT}",m_nLocationID);
		if(!rs->eof) {
			bBillServiceDateTo = AdoFldBool(rs, "BillServiceDateTo",FALSE);
			bBillInputDate = AdoFldBool(rs, "BillInputDate",FALSE);
			bBillPatientCoordinator = AdoFldBool(rs, "BillPatientCoordinator",FALSE);
			bBillCPTCode = AdoFldBool(rs, "BillCPTCode",TRUE);
			bBillCPTSubCode = AdoFldBool(rs, "BillCPTSubCode",TRUE);
			bBillCPTChargeCategory = AdoFldBool(rs, "BillChargeCategory", TRUE);// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
			bBillTOS = AdoFldBool(rs, "BillTOS",TRUE);
			bBillMod1 = AdoFldBool(rs, "BillMod1",TRUE);
			bBillMod2 = AdoFldBool(rs, "BillMod2",TRUE);
			bBillMod3 = AdoFldBool(rs, "BillMod3",FALSE);
			bBillMod4 = AdoFldBool(rs, "BillMod4",FALSE);
			bBillDiagCs = AdoFldBool(rs, "BillDiagCs",TRUE);			
			bBillTotalDiscount = AdoFldBool(rs, "BillTotalDiscount",TRUE);
			bBillTax1 = AdoFldBool(rs, "BillTax1",TRUE);
			bBillTax2 = AdoFldBool(rs, "BillTax2",TRUE);
			// (j.jones 2010-08-31 17:42) - PLID 40330 - added bill allowable
			bBillAllowable = AdoFldBool(rs, "BillAllowable",FALSE);

			bQuoteCPTCode = AdoFldBool(rs, "QuoteCPTCode",TRUE);
			bQuoteCPTSubCode = AdoFldBool(rs, "QuoteCPTSubCode",TRUE);
			bQuoteCPTChargeCategory = AdoFldBool(rs, "QuoteChargeCategory", TRUE);// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
			bQuoteProvider = AdoFldBool(rs, "QuoteProvider",TRUE);
			bQuoteMod1 = AdoFldBool(rs, "QuoteMod1",FALSE);
			bQuoteMod2 = AdoFldBool(rs, "QuoteMod2",FALSE);
			bQuoteMod3 = AdoFldBool(rs, "QuoteMod3",FALSE);
			bQuoteMod4 = AdoFldBool(rs, "QuoteMod4",FALSE);
			bQuoteOutsideCost = AdoFldBool(rs, "QuoteOutsideCost",TRUE);
			bQuoteTotalDiscount = AdoFldBool(rs, "QuoteTotalDiscount",TRUE);			
			bQuoteTax1 = AdoFldBool(rs, "QuoteTax1",TRUE);
			bQuoteTax2 = AdoFldBool(rs, "QuoteTax2",TRUE);

			// (j.dinatale 2010-11-02) - PLID 41279 - no longer need these fields
			// (j.jones 2008-09-16 15:41) - PLID 31412 - added support for billing tab settings
			/*bBillTabProvider = AdoFldBool(rs, "BillTabProvider",TRUE);
			bBillTabPOSName = AdoFldBool(rs, "BillTabPOSName",FALSE);
			bBillTabPOSCode = AdoFldBool(rs, "BillTabPOSCode",FALSE);
			bBillTabLocation = AdoFldBool(rs, "BillTabLocation",FALSE);
			bBillTabInputDate = AdoFldBool(rs, "BillTabInputDate",FALSE);
			bBillTabCreatedBy = AdoFldBool(rs, "BillTabCreatedBy",FALSE);
			bBillTabChargeDate = AdoFldBool(rs, "BillTabChargeDate",FALSE);
			bBillTabBillID = AdoFldBool(rs, "BillTabBillID",FALSE);
			bBillTabDiscountIcon = AdoFldBool(rs, "BillTabDiscountIcon",TRUE);*/

			// (j.gruber 2009-10-19 13:31) - PLID 36000 - quote allowable
			bQuoteAllowable = AdoFldBool(rs, "QuoteAllowable",FALSE);

			// (j.dinatale 2010-11-02) - PLID 41279 - no longer need these fields
			// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options
			/*bBillTabDiagCode1 = AdoFldBool(rs, "BillTabDiagCode1",FALSE);
			bBillTabDiagCode1WithDesc = AdoFldBool(rs, "BillTabDiagCode1WithDesc",FALSE);
			bBillTabAllDiagCodes = AdoFldBool(rs, "BillTabAllDiagCodes",FALSE);*/

			// (j.dinatale 2010-11-02) - PLID 41279 - no longer need this field
			// (j.jones 2010-08-31 17:53) - PLID 40331 - added bill tab allowable
			/*bBillTabAllowable = AdoFldBool(rs, "BillTabAllowable",TRUE);*/

			// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
			bBillClaimProvider = AdoFldBool(rs, "BillClaimProvider",FALSE);
		}
		rs->Close();

		//if(IsNexTechInternal()) {
		//	//Bill ID is always shown in internal, so might as well make the interface reflect that
		//	bBillTabBillID = TRUE;
		//}

		//now set the checkboxes
		m_checkBillServiceDateTo.SetCheck(bBillServiceDateTo);
		m_checkBillInputDate.SetCheck(bBillInputDate);
		m_checkBillPatientCoordinator.SetCheck(bBillPatientCoordinator);
		m_checkBillCPTCode.SetCheck(bBillCPTCode);
		m_checkBillCPTSubCode.SetCheck(bBillCPTSubCode);
		m_checkBillChargeCategory.SetCheck(bBillCPTChargeCategory);// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		m_checkBillTOS.SetCheck(bBillTOS);
		m_checkBillMod1.SetCheck(bBillMod1);
		m_checkBillMod2.SetCheck(bBillMod2);
		m_checkBillMod3.SetCheck(bBillMod3);
		m_checkBillMod4.SetCheck(bBillMod4);
		m_checkBillDiagCs.SetCheck(bBillDiagCs);
		// (j.gruber 2009-03-20 10:38) - PLID 33385 - combine percentoff and discount into total discount
		m_checkBillTotalDiscount.SetCheck(bBillTotalDiscount);
		m_checkBillTax1.SetCheck(bBillTax1);
		m_checkBillTax2.SetCheck(bBillTax2);
		// (j.jones 2010-08-31 17:42) - PLID 40330 - added bill allowable
		m_checkBillAllowable.SetCheck(bBillAllowable);
		m_checkQuoteCPTCode.SetCheck(bQuoteCPTCode);
		m_checkQuoteCPTSubCode.SetCheck(bQuoteCPTSubCode);
		m_checkQuoteChargeCategory.SetCheck(bQuoteCPTChargeCategory);// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		m_checkQuoteProvider.SetCheck(bQuoteProvider);
		m_checkQuoteMod1.SetCheck(bQuoteMod1);
		m_checkQuoteMod2.SetCheck(bQuoteMod2);
		m_checkQuoteMod3.SetCheck(bQuoteMod3);
		m_checkQuoteMod4.SetCheck(bQuoteMod4);
		m_checkQuoteOutsideCost.SetCheck(bQuoteOutsideCost);
		// (j.gruber 2009-03-20 10:38) - PLID 33385 - combined percent off and discount into total discount
		m_checkQuoteTotalDiscount.SetCheck(bQuoteTotalDiscount);		
		m_checkQuoteTax1.SetCheck(bQuoteTax1);
		m_checkQuoteTax2.SetCheck(bQuoteTax2);
		// (j.gruber 2009-10-19 13:35) - PLID 36000 - quote allowable
		m_checkQuoteAllowable.SetCheck(bQuoteAllowable);

		// (j.dinatale 2010-11-02) - PLID 41279 - no longer need to set these
		//m_checkBillTabProvider.SetCheck(bBillTabProvider);
		//m_checkBillTabPOSName.SetCheck(bBillTabPOSName);
		//m_checkBillTabPOSCode.SetCheck(bBillTabPOSCode);
		//m_checkBillTabLocation.SetCheck(bBillTabLocation);
		//m_checkBillTabInputDate.SetCheck(bBillTabInputDate);
		//m_checkBillTabCreatedBy.SetCheck(bBillTabCreatedBy);
		//m_checkBillTabChargeDate.SetCheck(bBillTabChargeDate);		
		//m_checkBillTabBillID.SetCheck(bBillTabBillID);
		//m_checkBillTabDiscountIcon.SetCheck(bBillTabDiscountIcon);
		//// (j.jones 2010-08-31 17:53) - PLID 40331 - added bill tab allowable
		//m_checkBillTabAllowable.SetCheck(bBillTabAllowable);

		// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options,
		// which are mutually exclusive of each other, but have their own data fields,
		// so make sure only one is checked
		/*if(bBillTabDiagCode1) {
			m_checkBillTabDiagCode1.SetCheck(TRUE);
			m_checkBillTabDiagCode1WithDesc.SetCheck(FALSE);
			m_checkBillTabAllDiagCodes.SetCheck(FALSE);
		}
		else if(bBillTabDiagCode1WithDesc) {
			m_checkBillTabDiagCode1.SetCheck(FALSE);
			m_checkBillTabDiagCode1WithDesc.SetCheck(TRUE);
			m_checkBillTabAllDiagCodes.SetCheck(FALSE);
		}
		else if(bBillTabAllDiagCodes) {
			m_checkBillTabDiagCode1.SetCheck(FALSE);
			m_checkBillTabDiagCode1WithDesc.SetCheck(FALSE);
			m_checkBillTabAllDiagCodes.SetCheck(TRUE);
		}
		else {
			m_checkBillTabDiagCode1.SetCheck(FALSE);
			m_checkBillTabDiagCode1WithDesc.SetCheck(FALSE);
			m_checkBillTabAllDiagCodes.SetCheck(FALSE);
		}*/

		// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
		m_checkBillClaimProvider.SetCheck(bBillClaimProvider);

	}NxCatchAll("Error loading column configuration.");
}
	
void CConfigBillColumnsDlg::Save()
{
	try {

		if(m_nLocationID == -1)
			return;

		BOOL bBillServiceDateTo = m_checkBillServiceDateTo.GetCheck();
		BOOL bBillInputDate = m_checkBillInputDate.GetCheck();
		BOOL bBillPatientCoordinator = m_checkBillPatientCoordinator.GetCheck();
		BOOL bBillCPTCode = m_checkBillCPTCode.GetCheck();
		BOOL bBillCPTSubCode = m_checkBillCPTSubCode.GetCheck();
		BOOL bBillCPTCategory = m_checkBillChargeCategory.GetCheck();// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		BOOL bBillTOS = m_checkBillTOS.GetCheck();
		BOOL bBillMod1 = m_checkBillMod1.GetCheck();
		BOOL bBillMod2 = m_checkBillMod2.GetCheck();
		BOOL bBillMod3 = m_checkBillMod3.GetCheck();
		BOOL bBillMod4 = m_checkBillMod4.GetCheck();
		BOOL bBillDiagCs = m_checkBillDiagCs.GetCheck();
		// (j.gruber 2009-03-20 10:39) - PLID 33385 - combine percentoff and discount into total discount
		BOOL bBillTotalDiscount = m_checkBillTotalDiscount.GetCheck();
		BOOL bBillTax1 = m_checkBillTax1.GetCheck();
		BOOL bBillTax2 = m_checkBillTax2.GetCheck();
		// (j.jones 2010-08-31 17:42) - PLID 40330 - added bill allowable
		BOOL bBillAllowable = m_checkBillAllowable.GetCheck();
		BOOL bQuoteCPTCode = m_checkQuoteCPTCode.GetCheck();
		BOOL bQuoteCPTSubCode = m_checkQuoteCPTSubCode.GetCheck();
		BOOL bQuoteCPTCategory = m_checkQuoteChargeCategory.GetCheck();// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		BOOL bQuoteProvider = m_checkQuoteProvider.GetCheck();
		BOOL bQuoteMod1 = m_checkQuoteMod1.GetCheck();
		BOOL bQuoteMod2 = m_checkQuoteMod2.GetCheck();
		BOOL bQuoteMod3 = m_checkQuoteMod3.GetCheck();
		BOOL bQuoteMod4 = m_checkQuoteMod4.GetCheck();
		BOOL bQuoteOutsideCost = m_checkQuoteOutsideCost.GetCheck();
		// (j.gruber 2009-03-20 10:39) - PLID 33385 - combine percentoff and discount into total discount
		BOOL bQuoteTotalDiscount = m_checkQuoteTotalDiscount.GetCheck();		
		BOOL bQuoteTax1 = m_checkQuoteTax1.GetCheck();
		BOOL bQuoteTax2 = m_checkQuoteTax2.GetCheck();
		// (j.gruber 2009-10-19 13:35) - PLID 36000 - quote allowable
		BOOL bQuoteAllowable = m_checkQuoteAllowable.GetCheck();

		// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
		BOOL bBillClaimProvider = m_checkBillClaimProvider.GetCheck();

		// (j.dinatale 2010-11-02) - PLID 41279 - no longer need to worry about bill tab settings
		// (j.jones 2008-09-16 15:52) - PLID 31412 - added support for billing tab settings
		//BOOL bBillTabProvider = m_checkBillTabProvider.GetCheck();
		//BOOL bBillTabPOSName = m_checkBillTabPOSName.GetCheck();
		//BOOL bBillTabPOSCode = m_checkBillTabPOSCode.GetCheck();
		//BOOL bBillTabLocation = m_checkBillTabLocation.GetCheck();
		//BOOL bBillTabInputDate = m_checkBillTabInputDate.GetCheck();
		//BOOL bBillTabCreatedBy = m_checkBillTabCreatedBy.GetCheck();
		//BOOL bBillTabChargeDate = m_checkBillTabChargeDate.GetCheck();
		//BOOL bBillTabBillID = m_checkBillTabBillID.GetCheck();
		//BOOL bBillTabDiscountIcon = m_checkBillTabDiscountIcon.GetCheck();

		//// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options
		//BOOL bBillTabDiagCode1 = m_checkBillTabDiagCode1.GetCheck();
		//BOOL bBillTabDiagCode1WithDesc = m_checkBillTabDiagCode1WithDesc.GetCheck();
		//BOOL bBillTabAllDiagCodes = m_checkBillTabAllDiagCodes.GetCheck();

		//// (j.jones 2010-08-31 17:55) - PLID 40331 - added bill tab allowable
		//BOOL bBillTabAllowable = m_checkBillTabAllowable.GetCheck();

		//create the record if it doesn't exist
		if(IsRecordsetEmpty("SELECT LocationID FROM ConfigBillColumnsT WHERE LocationID = %li",m_nLocationID))
			ExecuteSql("INSERT INTO ConfigBillColumnsT (LocationID) VALUES (%li)",m_nLocationID);

		// (j.jones 2008-09-16 15:52) - PLID 31412 - added billing tab settings
		// (j.gruber 2009-03-20 10:39) - PLID 33385 - combine percentoff and discount into total discount
		// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options
		// (j.jones 2010-08-31 17:42) - PLID 40330 - added bill allowable
		// (j.jones 2010-08-31 17:53) - PLID 40331 - added bill tab allowable
		// (j.dinatale 2010-11-02) - PLID 41279 - removed all bill tab settings, they will be in a new dialog
		// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
		// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
		ExecuteSql("UPDATE ConfigBillColumnsT SET BillServiceDateTo = %li, BillInputDate = %li, "
			"BillPatientCoordinator = %li, BillCPTCode = %li, BillCPTSubCode = %li, BillTOS = %li, "
			"BillMod1 = %li, BillMod2 = %li, BillMod3 = %li, BillMod4 = %li, "
			"BillDiagCs = %li, BillTotalDiscount = %li, "
			"BillTax1 = %li, BillTax2 = %li, BillAllowable = %li, "
			"QuoteCPTCode = %li, QuoteCPTSubCode = %li, QuoteProvider = %li, "
			"QuoteMod1 = %li, QuoteMod2 = %li, QuoteMod3 = %li, QuoteMod4 = %li, "
			"QuoteOutsideCost = %li, QuoteTotalDiscount = %li, QuoteTax1 = %li, QuoteTax2 = %li, "
			"QuoteAllowable = %li, BillClaimProvider = %li, BillChargeCategory =%li , QuoteChargeCategory = %li "
			"WHERE LocationID = %li", (bBillServiceDateTo ? 1 : 0), (bBillInputDate ? 1 : 0),
			(bBillPatientCoordinator ? 1 : 0), (bBillCPTCode ? 1 : 0), (bBillCPTSubCode ? 1 : 0), (bBillTOS ? 1 : 0),
			(bBillMod1 ? 1 : 0), (bBillMod2 ? 1 : 0), (bBillMod3 ? 1 : 0), (bBillMod4 ? 1 : 0),
			(bBillDiagCs ? 1 : 0), (bBillTotalDiscount ? 1 : 0), 
			(bBillTax1 ? 1 : 0), (bBillTax2 ? 1 : 0), (bBillAllowable ? 1 : 0),
			(bQuoteCPTCode ? 1 : 0), (bQuoteCPTSubCode ? 1 : 0), (bQuoteProvider ? 1 : 0),
			(bQuoteMod1 ? 1 : 0), (bQuoteMod2 ? 1 : 0), (bQuoteMod3 ? 1 : 0), (bQuoteMod4 ? 1 : 0),
			(bQuoteOutsideCost ? 1 : 0), (bQuoteTotalDiscount ? 1 : 0), (bQuoteTax1 ? 1 : 0), (bQuoteTax2 ? 1 : 0),
			(bQuoteAllowable ? 1 : 0), (bBillClaimProvider ? 1 : 0), (bBillCPTCategory ? 1 : 0), (bQuoteCPTCategory ? 1 : 0 ), m_nLocationID);

		// (j.dinatale 2010-11-02) - PLID 41279 - no longer need to fire the table checker
		// (j.jones 2008-09-17 08:51) - PLID 31412 - send a tablechecker to indicate this changed,
		// but only if billing tab columns changed, since that is currently the only place that
		// checks this tablechecker
		/*if(m_bBillTabColumnsChanged) {
			CClient::RefreshTable(NetUtils::ConfigBillColumnsT);
		}*/

		// (c.haag 2004-06-21 10:24) - PLID 8023 - If the user has defined column widths,
		// reset them to their defaults.
		// (j.jones 2008-09-16 15:55) - PLID 31412 - split the bChanged variable into
		// three parts so we didn't reset stored bill/quote widths unnecessarily	
		// (d.thompson 2012-08-01) - PLID 51898 - Changed default to 1
		if((m_bBillColumnsChanged || m_bQuoteColumnsChanged) && GetRemotePropertyInt("RememberBillingColumnWidths",1,0,GetCurrentUserName(),TRUE))
		{
			if(m_bBillColumnsChanged) {
				SetRemotePropertyText("DefaultBillColumnSizes", GetDefaultBillingColumnWidths(), 0, GetCurrentUserName());
			}
			if(m_bQuoteColumnsChanged) {
				// (j.jones 2009-12-23 09:19) - PLID 32587 - added bShowInitialValue
				SetRemotePropertyText("DefaultQuoteColumnSizes", GetDefaultQuoteColumnWidths(FALSE, FALSE), 0, GetCurrentUserName());
			}
			if(m_bBillColumnsChanged && m_bQuoteColumnsChanged) {
				MsgBox("Because you have changed which columns are visible, your saved bill and quote column widths have been reset to the system defaults.");
			}
			else if(m_bBillColumnsChanged && !m_bQuoteColumnsChanged) {
				MsgBox("Because you have changed which columns are visible, your saved bill column widths have been reset to the system defaults.");
			}
			else if(!m_bBillColumnsChanged && m_bQuoteColumnsChanged) {
				MsgBox("Because you have changed which columns are visible, your saved quote column widths have been reset to the system defaults.");
			}
		}

		m_bBillColumnsChanged = FALSE;
		m_bQuoteColumnsChanged = FALSE;
		// (j.dinatale 2010-11-02) - PLID 41279 - dont worry about billing tab changes anymore
		//m_bBillTabColumnsChanged = FALSE;

	}NxCatchAll("Error saving column configuration.");
}

// (j.dinatale 2010-11-02) - PLID 41279 - Removed any messages involving Bill Tab IDs
BOOL CConfigBillColumnsDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD	nID;

	switch (HIWORD(wParam))
	{	case BN_CLICKED:
		{	switch (nID = LOWORD(wParam))
			{
				case IDC_CHECK_QUOTE_TAX2:
				case IDC_CHECK_QUOTE_TAX1:
				case IDC_CHECK_QUOTE_PROVIDER:
				// (j.gruber 2009-03-20 10:39) - PLID 33385 - combine percentoff and discount into total discount
				case IDC_CHECK_QUOTE_TOTAL_DISCOUNT:
				case IDC_CHECK_QUOTE_OUTSIDE_PRACTICE_COST:
				case IDC_CHECK_QUOTE_MOD4:
				case IDC_CHECK_QUOTE_MOD3:
				case IDC_CHECK_QUOTE_MOD2:
				case IDC_CHECK_QUOTE_MOD1:				
				case IDC_CHECK_QUOTE_CPT_SUBCODE:
				// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
				case IDC_CHECK_QUOTE_CPT_CATEGORY:
				case IDC_CHECK_QUOTE_CPT_CODE:
				// (j.gruber 2009-10-19 13:38) - PLID 36000 - quote allowable
				case IDC_CHECK_QUOTE_ALLOWABLE:
					m_bQuoteColumnsChanged = TRUE;
					break;
	
				case IDC_CHECK_BILL_TYPE_OF_SERVICE:
				case IDC_CHECK_BILL_TAX2:
				case IDC_CHECK_BILL_TAX1:
				case IDC_CHECK_BILL_MOD4:
				case IDC_CHECK_BILL_MOD3:
				case IDC_CHECK_BILL_MOD2:
				case IDC_CHECK_BILL_MOD1:
				case IDC_CHECK_BILL_CPT_DIAGCS:				
				case IDC_CHECK_BILL_CPT_SUB_CODE:
				case IDC_CHECK_BILL_CPT_CODE:
				case IDC_CHECK_BILL_INPUT_DATE:
				case IDC_CHECK_BILL_PATIENT_COORDINATOR:
				// (j.gruber 2009-03-20 10:39) - PLID 33385 - combine percentoff and discount into total discount
				case IDC_CHECK_BILL_TOTAL_DISCOUNT:
				case IDC_CHECK_BILL_SERVICE_DATE_TO:
				// (j.jones 2010-08-31 17:51) - PLID 40330 - added Bill Allowable
				case IDC_CHECK_BILL_ALLOWABLE:
				// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
				case IDC_CHECK_BILL_CLAIM_PROVIDER:
				// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
				case IDC_CHECK_BILL_CPT_CATEGORY:
					m_bBillColumnsChanged = TRUE;
					break;

				// (j.dinatale 2010-11-02) - PLID 41279 - removed all billing tab column settings from this dialog
				//case IDC_CHECK_BILLTAB_PROVIDER:
				//case IDC_CHECK_BILLTAB_POSNAME:
				//case IDC_CHECK_BILLTAB_POSCODE:
				//case IDC_CHECK_BILLTAB_LOCATION:
				//case IDC_CHECK_BILLTAB_INPUT_DATE:
				//case IDC_CHECK_BILLTAB_CREATEDBY:
				//case IDC_CHECK_BILLTAB_CHARGE_DATE:
				//case IDC_CHECK_BILLTAB_BILLID:
				//case IDC_CHECK_BILLTAB_DISCOUNT_ICON:
				//// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options
				//case IDC_CHECK_BILLTAB_DIAGCODE1:
				//case IDC_CHECK_BILLTAB_DIAGCODEDESC:
				//case IDC_CHECK_BILLTAB_DIAGCODELIST:
				//// (j.jones 2010-08-31 17:51) - PLID 40331 - added Bill Tab Allowable
				//case IDC_CHECK_BILLTAB_ALLOWABLE:
				//	m_bBillTabColumnsChanged = TRUE;
				//	break;
			}

			// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options, which
			// are mutually exclusive, so just handle that here
			/*if(LOWORD(wParam) == IDC_CHECK_BILLTAB_DIAGCODE1) {
				if(m_checkBillTabDiagCode1.GetCheck()) {
					m_checkBillTabDiagCode1WithDesc.SetCheck(FALSE);
					m_checkBillTabAllDiagCodes.SetCheck(FALSE);
				}
			}
			else if(LOWORD(wParam) == IDC_CHECK_BILLTAB_DIAGCODEDESC) {
				if(m_checkBillTabDiagCode1WithDesc.GetCheck()) {
					m_checkBillTabDiagCode1.SetCheck(FALSE);
					m_checkBillTabAllDiagCodes.SetCheck(FALSE);
				}
			}
			else if(LOWORD(wParam) == IDC_CHECK_BILLTAB_DIAGCODELIST) {
				if(m_checkBillTabAllDiagCodes.GetCheck()) {
					m_checkBillTabDiagCode1.SetCheck(FALSE);
					m_checkBillTabDiagCode1WithDesc.SetCheck(FALSE);
				}
			}*/
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
