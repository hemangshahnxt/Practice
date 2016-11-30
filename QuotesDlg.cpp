// QuotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "QuotesDlg.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "BillingModuleDlg.h"
#include "NxStandard.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"
#include "BillingRc.h"
#include "PaymentDlg.h"
#include "ManageQuotePrepaysDlg.h"
#include "Barcode.h"
#include "CareCreditUtils.h"
#include "BillingModuleDlg.h"
#include "ProgressDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_DELQ			43040
#define ID_COPYQ		43041
#define ID_DEACTIVATEQ	43042
#define ID_REACTIVATEQ	43043
#define ID_NEWPAY		43044
#define ID_VIEWPAYS		43045
#define ID_PREVIEW_QUOTE	43046

//TES 8/10/2009 - NOTE: If you add/change/reorder columns here, you must also update 
// m_aDefColumnStyles, which is set in OnInitDialog()
#define COLUMN_QUOTE_ID			0
#define	COLUMN_QUOTE_DATE		1
#define	COLUMN_DESCRIPTION		2
#define	COLUMN_TOTAL_BEFORE_DISCOUNTS_AND_TAX		3	// (j.jones 2009-09-15 16:45) - PLID 34528 - renamed these total columns and added some more
#define COLUMN_DISCOUNTS		4
#define	COLUMN_TAXES			5
#define	COLUMN_TOTAL			6
#define COLUMN_BILLED			7	//TES 8/10/2009 - PLID 16920
#define	COLUMN_ACTIVE			8
#define COLUMN_INPUT_USER		9	// (d.lange 2011-06-24 14:25) - PLID 25117 - Added the Input User
#define	COLUMN_ISPACKAGE		10
#define	COLUMN_PACKAGE_TYPE		11
#define	COLUMN_TOTAL_COUNT		12
#define COLUMN_CURRENT_COUNT	13
#define	COLUMN_TOTAL_AMOUNT		14
#define	COLUMN_UNBILLED_AMOUNT	15
#define COLUMN_UNPAID_AMOUNT	16
#define COLUMN_EXPIRED			17	// (d.thompson 2009-08-18) - PLID 16758
#define COLUMN_ORIGINAL_CURRENT_AMOUNT	18

using namespace ADODB;
//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2.
using namespace NXDATALIST2Lib;
/////////////////////////////////////////////////////////////////////////////
// CQuotesDlg dialog

CQuotesDlg::CQuotesDlg(CWnd* pParent)
	: CPatientDialog(CQuotesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuotesDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Billing/Cosmetic_Billing/Quotes/create_a_quote.htm";
	//(e.lally 2008-06-17) PLID 27303 - initialize to NULL
	m_pRow = NULL;

	// (a.walling 2010-10-14 09:08) - PLID 40977
	m_id = -1;
}


void CQuotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuotesDlg)
	DDX_Control(pDX, IDC_SHOW_PACKAGE_INFO, m_ShowPackageInfo);
	DDX_Control(pDX, IDC_NEW_QUOTE, m_newQuoteButton);
	DDX_Control(pDX, IDC_QUOTES_BKG, m_BackGround);
	DDX_Control(pDX, IDC_REMEMBER_QUOTE_COL_WIDTHS, m_RememberColWidthsCheck);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CQuotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CQuotesDlg)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_NEW_QUOTE, OnNewQuote)
	ON_BN_CLICKED(IDC_SHOW_PACKAGE_INFO, OnShowPackageInfo)
	ON_BN_CLICKED(IDC_REMEMBER_QUOTE_COL_WIDTHS, OnRememberColumns)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_OPEN_CARE_CREDIT, OnOpenCareCredit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CQuotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CQuotesDlg)
	ON_EVENT(CQuotesDlg, IDC_QUOTE_LIST, 6 /* RButtonDown */, OnRButtonDownQuoteList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CQuotesDlg, IDC_QUOTE_LIST, 3 /* DblClickCell */, OnDblClickCellQuoteList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CQuotesDlg, IDC_QUOTE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedQuoteList, VTS_I2)
	ON_EVENT(CQuotesDlg, IDC_QUOTE_LIST, 22 /* ColumnSizingFinished */, OnColumnSizingFinished, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CQuotesDlg message handlers

BOOL CQuotesDlg::OnInitDialog() 
{
	try{
		CNxDialog::OnInitDialog();
		m_newQuoteButton.AutoSet(NXB_NEW);	
		//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2.
		m_QuoteList = BindNxDataList2Ctrl(IDC_QUOTE_LIST,false);
		//make sure our member variable pointer is set to NULL
		m_pRow = NULL;

		// (j.jones 2009-10-26 09:23) - PLID 32904 - added bulk caching
		g_propManager.CachePropertiesInBulk("CQuotesDlg-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PackageTaxEstimation' OR "
			"Name = 'RememberQuoteTabColumns' OR "
			"Name = 'ShowPackageInfoInQuotesTab' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CQuotesDlg-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefaultQuoteTabColumnSizes' "
			")",
			_Q(GetCurrentUserName()));

		// (z.manning, 01/10/2007) - PLID 24057 - Only show the CareCredit button if it hasn't expired.
		if(NxCareCredit::GetCareCreditLicenseStatus() == cclsExpired) {
			GetDlgItem(IDC_OPEN_CARE_CREDIT)->ShowWindow(SW_HIDE);
			m_hBitmap = NULL;
		}
		else {
			// (z.manning, 01/04/2007) - PLID 24056 - Set the CareCredit button's bitmap
			// (a.walling 2008-06-03 10:54) - PLID 27686 - Fix GDI leak
			m_hBitmap = ::LoadBitmap(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_CARE_CREDIT_LOGO_SMALL));
			((CButton*)GetDlgItem(IDC_OPEN_CARE_CREDIT))->SetBitmap(m_hBitmap);
		}

		// (c.haag 2004-07-30 09:28) - PLID 13728 - We need to remember the default
		// styles of each column so that we can switch back to them when the user
		// no longer wants to remember the column widths.
		for (short i=0; i < m_QuoteList->GetColumnCount(); i++)
		{
			m_aDefColumnStyles.Add(m_QuoteList->GetColumn(i)->GetColumnStyle());		
		}
		// (c.haag 2004-07-30 10:11) - These are the default widths straight from the
		// resources. If you change them there, you must change them here!
		// (e.lally 2008-06-17) - Updated default widths for 9pt font support
		// (j.jones 2009-09-15 16:54) - PLID 34528 - altered the widths of the totals columns and added a new one
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(82);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(161);
		m_aDefColumnWidths.Add(63);
		m_aDefColumnWidths.Add(102);
		m_aDefColumnWidths.Add(151);
		m_aDefColumnWidths.Add(50); //TES 8/10/2009 - PLID 16920 - Added a Billed? column
		m_aDefColumnWidths.Add(50);
		m_aDefColumnWidths.Add(80);	// (d.lange 2011-06-24 14:26) - PLID 25117 - Added Input User
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);
		m_aDefColumnWidths.Add(0);	// (d.thompson 2009-08-18) - PLID 16758 - Expired column
		m_aDefColumnWidths.Add(0);	//COLUMN_ORIGINAL_CURRENT_AMOUNT
		

		if(GetRemotePropertyInt("RememberQuoteTabColumns", 0, 0, GetCurrentUserName(), true) == 1) {
			CheckDlgButton(IDC_REMEMBER_QUOTE_COL_WIDTHS, TRUE);
		}
		else {
			CheckDlgButton(IDC_REMEMBER_QUOTE_COL_WIDTHS, FALSE);
		}
		// (a.walling 2010-10-14 09:08) - PLID 40977 - This will be called already after initially loading the tab
		//UpdateView(); // (c.haag 2003-09-03 09:49) - Our columns are resized in here.
		return TRUE;  
	
	}NxCatchAll("Error while initializing Quotes tab");
	return FALSE;
}

void CQuotesDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try{
		CNxDialog::OnShowWindow(bShow, nStatus);
	
		if (!bShow) return;

		m_newQuoteButton.SetFocus();
	}NxCatchAll("CQuotesDlg::OnShowWindow");
}

void CQuotesDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
//	OLE_COLOR	m_nColor;

	try {

		// (a.walling 2010-10-14 09:08) - PLID 40977
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();

		//(e.lally 2011-08-26) PLID 45210 - Auto create pref.
		if(GetRemotePropertyInt("ShowPackageInfoInQuotesTab",0,0,"<None>",true)==1) {
			m_ShowPackageInfo.SetCheck(TRUE);		
		}
		else {
			m_ShowPackageInfo.SetCheck(FALSE);
		}

		ResizeColumns();

		// (a.walling 2010-10-13 16:12) - PLID 40977
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		// (a.walling 2010-10-14 09:08) - PLID 40977 - Moved actual loading code to Refresh()
		if (bForceRefresh || m_ForceRefresh) {
			Refresh();	
		}
		m_ForceRefresh = false;
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-10-14 09:08) - PLID 40977
void CQuotesDlg::Refresh()
{
	try {
		m_id = GetActivePatientID();

		CString		strListSQL;

		// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
		// (j.gruber 2009-03-17 12:51) - PLID 33360 - update with new discount structure
		// (d.thompson 2009-08-18) - PLID 16758 - Added 'Expired' field
		// (j.jones 2009-09-15 16:29) - PLID 34528 - added more quote total columns
		// (d.lange 2011-06-24 14:39) - PLID 25117 - added Input User
		strListSQL.Format("(SELECT PatientBillsQ.ID AS QuoteID, PatientBillsQ.Date AS QuoteDate, PatientBillsQ.Description AS FirstOfNotes, PatientBillsQ.Active, "
			""
			//the SubTotal field is the Total Before Discounts, Before Tax
			"(CASE WHEN PackagesT.QuoteID Is Not NULL THEN NULL ELSE "
			"Sum(Round(Convert(money,("
			"[Amount]*[Quantity]"
			"*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)"
			")),2)) END) AS SubTotal, "
			""
			//TotalWithDiscounts is simply the discounted quote total, pre-tax
			"(CASE WHEN PackagesT.QuoteID Is Not NULL THEN NULL ELSE "
			"Sum(CASE WHEN [Amount] = 0 AND OthrBillFee > 0 THEN 0 ELSE "
			"Round(Convert(money, "
			"(([Amount]*[Quantity]"
			"*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)"
			"*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END)))"
			"),2) END) END) AS TotalWithDiscounts, "
			""
			//GrandTotal is the total quote value, with discounts, with tax
			"(CASE WHEN PackagesT.QuoteID Is Not NULL THEN NULL ELSE "
			"Sum(CASE WHEN [Amount] = 0 AND OthrBillFee > 0 THEN 0 ELSE "
			"Round(Convert(money, "
			"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END))) "
			"+(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) "
			"+(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
			"),2) END) END) AS GrandTotal, "
			""
			"convert(bit,(CASE WHEN PackagesT.QuoteID Is NULL THEN 0 ELSE 1 END)) AS IsPackage, TotalAmount, PackagesT.Type AS PackageType, CurrentAmount, OriginalCurrentAmount, TotalCount, CurrentCount, "
			"convert(bit, CASE WHEN PatientBillsQ.UseExp = 1 THEN CASE WHEN DATEADD(d, PatientBillsQ.ExpDays, PatientBillsQ.Date) < getdate() then 1 else 0 end ELSE 0 END) AS Expired, "
			"(SELECT Username FROM UsersT WHERE UsersT.PersonID = PatientBillsQ.InputName) AS InputUser "
			"FROM ((SELECT BillsT.* FROM BillsT WHERE (((BillsT.PatientID)=%li) AND ((BillsT.Deleted)=0))) AS PatientBillsQ LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, TotalPercentOffQ.TotalPercentOff, TotalDiscountQ.TotalDiscount FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "		
			"LEFT JOIN (SELECT ChargeID, Sum(PercentOff) AS TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentOffQ ON ChargesT.ID = TotalPercentOffQ.ChargeID "
			"LEFT JOIN (SELECT ChargeID, Sum(Discount) AS TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
			"WHERE (((LineItemT.PatientID)=%li) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))) AS PatientChargesQ ON PatientBillsQ.ID = PatientChargesQ.BillID) LEFT JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID "
			"LEFT JOIN "
			"	(SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, OriginalCurrentAmount, Type, "
			"	(CASE WHEN Type = 1 THEN PackagesT.TotalCount WHEN Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
			"	(CASE WHEN Type = 1 THEN PackagesT.CurrentCount WHEN Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
			"	FROM PackagesT "
			"	LEFT JOIN "
			"		(SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
			"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
			"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
			"		GROUP BY BillID "
			"		) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
			"	) AS PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID "
			"GROUP BY PatientBillsQ.ID, PatientBillsQ.Date, PatientBillsQ.EntryType, PatientBillsQ.Description, PatientBillsQ.Active, PackagesT.QuoteID, PackagesT.Type, TotalAmount, CurrentAmount, OriginalCurrentAmount, TotalCount, CurrentCount, "
			"convert(bit, CASE WHEN PatientBillsQ.UseExp = 1 THEN CASE WHEN DATEADD(d, PatientBillsQ.ExpDays, PatientBillsQ.Date) < getdate() then 1 else 0 end ELSE 0 END), PatientBillsQ.InputName "
			"HAVING (((PatientBillsQ.EntryType)=2))) AS Q",m_id,m_id);

		m_QuoteList->FromClause = _bstr_t(strListSQL);

		m_QuoteList->Requery();

		m_ForceRefresh = false;
	} NxCatchAllThrow(__FUNCTION__);
}

void CQuotesDlg::SetColor(OLE_COLOR nNewColor)
{
	m_BackGround.SetColor(nNewColor);
	CPatientDialog::SetColor(nNewColor);
}


void CQuotesDlg::OnRButtonDownQuoteList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax.
		//Cast as row pointer, check for null
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL){
			return;
		}
		m_QuoteList->PutCurSel(pRow);

		CMenu pMenu;

		m_pRow = pRow;

		// Build a menu popup with the ability to delete the current row
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_COPYQ, "Copy Quote");
		pMenu.InsertMenu(1, MF_BYPOSITION, ID_DELQ, "Delete Quote");
		//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax
		if(VarBool(m_QuoteList->GetCurSel()->GetValue(COLUMN_ACTIVE),TRUE))
			pMenu.InsertMenu(2, MF_BYPOSITION, ID_DEACTIVATEQ, "Mark Quote Inactive");
		else
			pMenu.InsertMenu(2, MF_BYPOSITION, ID_REACTIVATEQ, "Mark Quote Active");
		// (j.gruber 2007-08-14 11:16) - PLID 27068 - added support for package 
		if (VarBool(m_QuoteList->GetCurSel()->GetValue(COLUMN_ISPACKAGE), FALSE)) {
			pMenu.InsertMenu(3, MF_BYPOSITION, ID_PREVIEW_QUOTE, "Preview Package Quote");		
		}
		else {
			pMenu.InsertMenu(3, MF_BYPOSITION, ID_PREVIEW_QUOTE, "Preview Quote");		
		}
		
		// (j.jones 2007-02-20 10:50) - PLID 23706 - added a check for the billing license
		if(g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			pMenu.InsertMenu(4, MF_BYPOSITION|MF_SEPARATOR);
			pMenu.InsertMenu(5, MF_BYPOSITION, ID_NEWPAY, "Make A PrePayment");
			pMenu.InsertMenu(6, MF_BYPOSITION, ID_VIEWPAYS, "Manage Linked PrePayments");
		}
		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);

	}NxCatchAll("Error generating right-click list.");
}


BOOL CQuotesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
		case ID_DELQ: {

			try {

			if(!CheckCurrentUserPermissions(bioPatientQuotes,sptDelete))
				break;

			CString strWarning;

			//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarBool for added safety
			BOOL bIsPackage = VarBool(m_pRow->GetValue(COLUMN_ISPACKAGE));

			if(bIsPackage) {
				long PackageCount = 0;
				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax
				_variant_t var = m_pRow->GetValue(COLUMN_CURRENT_COUNT);
				if(var.vt == VT_I4) {
					PackageCount = var.lVal;
				}
				if(PackageCount>0)
					strWarning = "This package is still in use, and should not be deleted.\n"
						"You will not be able to complete this package for this patient if you continue.\n\n"
						"Are you absolutely SURE you wish to delete this package?";
				else
					strWarning = "Are you sure you wish to delete this package?";

			}
			else {
				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLongs for added safety
				if(ReturnsRecords("SELECT ID FROM ProcInfoT WHERE ActiveQuoteID = %li", VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID)))) {
					//active quote on a procedure
					strWarning = "This quote is the active quote for a tracked procedure. Are you sure you wish to permanently delete this quote?";
				}
				else if(ReturnsRecords("SELECT ProcInfoID FROM ProcInfoQuotesT WHERE QuoteID = %li", VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID)))) {
					//attached to a procedure
					strWarning = "This quote is attached to a tracked procedure. Are you sure you wish to permanently delete this quote?";
				}
				else {
					//normal quote delete
					strWarning = "You are about to permanently delete this quote. Are you sure you wish to do this?";
				}
			}

			if (IDYES == MessageBox(strWarning, "NexTech", MB_YESNO)) {
				
					/////////////////////////////////////////////
					// Delete quote
					/////////////////////////////////////////////
					//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
					DeleteBill(VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID)));

					m_QuoteList->RemoveRow(m_pRow);
					m_pRow = NULL;
			}

			}NxCatchAll("Error deleting quote.");
		}
		break;

		case ID_COPYQ:

			try {	
				
				if(!CheckCurrentUserPermissions(bioPatientQuotes,sptCreate))
				break;
				/////////////////////////////////////////////
				// Copy Quote
				/////////////////////////////////////////////

				CWaitCursor pWait;
				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
				long ID = VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID));

				CopyQuote(ID);
				
				m_QuoteList->Requery();
			}
			NxCatchAll("Error copying quote.");

		break;

		case ID_DEACTIVATEQ:

			try {	
				
				//m.hancock - 11/25/2005 - PLID 18424 - When activating / deactivating a quote, the Write Quote
				//permission should be used instead of the Write Bill permission.
				//if(!CheckCurrentUserPermissions(bioBill,sptWrite))
				if(!CheckCurrentUserPermissions(bioPatientQuotes,sptWrite))
				break;
				/////////////////////////////////////////////
				// Deactivate Quote
				/////////////////////////////////////////////
				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
				long ID = VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID));

				if(ReturnsRecords("SELECT ID FROM ProcInfoT WHERE ActiveQuoteID = %li", ID)) {
					UINT nReturn = MsgBox(MB_YESNOCANCEL, "This quote is currently the active quote for a procedure.  Would you like to keep it as the active quote?\n"
						"Click Yes to retain it as the active quote for the procedure, No to set the procedure to have no active quote, or Cancel to not mark this quote as Inactive.");
					if(nReturn == IDNO) {
						ExecuteSql("UPDATE ProcInfoT SET ActiveQuoteID = NULL WHERE ActiveQuoteID = %li", ID);
					}
					else if(nReturn == IDCANCEL) {
						return CNxDialog::OnCommand(wParam, lParam);
					}
				}
				CWaitCursor pWait;
				
				ExecuteSql("UPDATE BillsT SET Active = 0 WHERE ID = %li",ID);

				bool bFalse = false;

				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax 
				m_pRow->PutValue(COLUMN_ACTIVE, _variant_t(bFalse));

				m_pRow->PutForeColor(RGB(112,112,112));
			}
			NxCatchAll("Error deactivating quote.");

		break;

		case ID_REACTIVATEQ:

			try {	
				
				//m.hancock - 11/25/2005 - PLID 18424 - When activating / deactivating a quote, the Write Quote
				//permission should be used instead of the Write Bill permission.
				//if(!CheckCurrentUserPermissions(bioBill,sptWrite))
				if(!CheckCurrentUserPermissions(bioPatientQuotes,sptWrite))
				break;
				/////////////////////////////////////////////
				// Reactivate Quote
				/////////////////////////////////////////////

				CWaitCursor pWait;
				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
				long ID = VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID));

				ExecuteSql("UPDATE BillsT SET Active = 1 WHERE ID = %li",ID);

				bool bTrue = true;

				//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax
				m_pRow->PutValue(COLUMN_ACTIVE, _variant_t(bTrue));
				m_pRow->PutForeColor(RGB(0,0,0));
			}
			NxCatchAll("Error reactivating quote.");

		break;

		case ID_NEWPAY:

			try {
				// (j.jones 2007-02-20 10:50) - PLID 23706 - added a check for the billing license
				if(CheckCurrentUserPermissions(bioPayment,sptCreate) && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {

					//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
					long QuoteID = VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID));
					COleCurrency cyRemBalance = COleCurrency(0,0);

					if(ReturnsRecords("SELECT BillsT.ID FROM BillsT "
						"INNER JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
						"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND BillsT.ID = %li ",QuoteID)) {
						//is a package
						cyRemBalance = CalculateRemainingPackageValueWithTax(QuoteID);
					}
					else {
						//this is more exact than searching the datalist
						// (j.gruber 2009-03-17 12:52) - PLID 33360 - update for new discount structure
						// (j.jones 2011-06-17 14:10) - PLID 38347 - fixed quote total calculation to account for modifiers
						_RecordsetPtr rs = CreateRecordset("SELECT "
										"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
										"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
										"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
										")),2)) AS Total "
										"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
										"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
										"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
										"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
										"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 11 AND BillsT.ID = %li ",QuoteID);

						if(rs->eof) {
							rs->Close();
							break;
						}

						if(rs->Fields->Item["Total"]->Value.vt == VT_CY)
							cyRemBalance = rs->Fields->Item["Total"]->Value.cyVal;
						else
							cyRemBalance = COleCurrency(0,0);

						rs->Close();
					}

					cyRemBalance -= CalculatePrePayments(GetActivePatientID(), QuoteID, false);

					CPaymentDlg dlg(this);
					dlg.m_varBillID = (long)QuoteID;
					dlg.m_bIsPrePayment = TRUE;
					dlg.m_QuoteID = QuoteID;
					dlg.m_cyFinalAmount = cyRemBalance;
					dlg.DoModal(__FUNCTION__, __LINE__);
				}
			}NxCatchAll("Error calculating prepayment amount.");

			break;

		case ID_VIEWPAYS:
			{
				try{
					// (j.jones 2007-02-20 10:50) - PLID 23706 - added a check for the billing license (silent),
					// because you can't do anything inside this dialog without the license anyways
					if(g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
						CManageQuotePrepaysDlg dlg(this);
						dlg.m_PatientID = GetActivePatientID();
						//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
						dlg.m_DefaultQuoteID = VarLong(m_pRow->GetValue(COLUMN_QUOTE_ID));
						dlg.DoModal();
					}
				}NxCatchAll("Error managing Quote Prepayments");
			}
			break;

		case ID_PREVIEW_QUOTE:
			{
				try {
					//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
					long nQuoteID = VarLong(m_pRow->GetValue( COLUMN_QUOTE_ID));
					// (j.gruber 2007-04-30 15:42) - PLID 25805 - make the extra text not show the quote description
					_RecordsetPtr rsDesc = CreateRecordset("SELECT ExtraDesc FROM BillsT WHERE ID = %li", nQuoteID);
					CString strDesc;
					if (!rsDesc->eof) {
						strDesc = AdoFldString(rsDesc, "ExtraDesc", "");
					}
					else {
						strDesc = "";
					}		
					//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax
					bool bIsPackage = VarBool(m_pRow->GetValue(COLUMN_ISPACKAGE))?true:false;
					// (j.gruber 2007-08-14 11:19) - PLID 27068 - support multiple report Ids
					long nReportID;
					if (bIsPackage) {
						nReportID = 608;
					}
					else {
						nReportID = 227;
					}

					// (j.jones 2013-06-04 11:50) - PLID 31874 - If they previously selected a different report
					// in the quote dialog and previewed, we would have saved the report they used.
					long nReportNumber = -2;
					CString strReportFileName = "";
					_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.LastQuoteReportID, BillsT.LastQuoteReportNumber, CustomReportsT.FileName "
						"FROM BillsT "
						"LEFT JOIN CustomReportsT ON BillsT.LastQuoteReportID = CustomReportsT.ID AND BillsT.LastQuoteReportNumber = CustomReportsT.Number "
						"WHERE BillsT.ID = {INT}", nQuoteID);
					if(!rs->eof) {
						//get the report ID, and ignore the saved report if it is not of the correct report type
						long nLastQuoteReportID = VarLong(rs->Fields->Item["LastQuoteReportID"]->Value, -1);
						if(nLastQuoteReportID == nReportID) {
							//We try to prevent previewing package reports for non-package quotes, so only use the
							//last previewed report if it is of the correct type.
							//If no custom report was used, use a number of -1 and a blank filename,
							//which will tell PreviewQuote to forcibly use the default report.
							nReportNumber = VarLong(rs->Fields->Item["LastQuoteReportNumber"]->Value, -1);
							if(nReportNumber != -1) {
								strReportFileName = VarString(rs->Fields->Item["FileName"]->Value, "");
							}
						}
					}
					rs->Close();

					PreviewQuote(nQuoteID, strDesc, bIsPackage, this, nReportID, nReportNumber, strReportFileName);

				}NxCatchAll("Error Previewing Quote");
			}
			break;
	}
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CQuotesDlg::OnNewQuote() 
{
	try{
		CBillingModuleDlg dlg(this);
		if(CheckCurrentUserPermissions(bioPatientQuotes,sptCreate)) {
			dlg.OpenWithBillID(-1, BillEntryType::Quote, 0);
		}

		m_QuoteList->Requery();
	}NxCatchAll("Error creating new Quote");
}

void CQuotesDlg::OnShowPackageInfo() 
{
	try{
		if(m_ShowPackageInfo.GetCheck()) {			
			SetRemotePropertyInt("ShowPackageInfoInQuotesTab",1,0,"<None>");
		}
		else {
			SetRemotePropertyInt("ShowPackageInfoInQuotesTab",0,0,"<None>");
		}

		ResizeColumns();

		//we must recalculate all the package totals, because if the columns were previously
		//hidden, they would not have been filled
		if (m_ShowPackageInfo.GetCheck() && !m_QuoteList->IsRequerying()) {
			CalculateAllPackageTotals();
		}

	}NxCatchAll("Error in CQuotesDlg::OnShowPackageInfo");
}

long CQuotesDlg::GetColumnWidth(const CString& strColWidths, short nColumn)
{
	CString str;
	int nIndex = 0, nEndIndex = 0;
	for (short i=0; i < nColumn && nIndex != -1; i++)
	{
		nIndex = strColWidths.Find(',', nIndex+1);
	}
	if (nIndex == -1)
		return -1;
	nEndIndex = strColWidths.Find(',', nIndex+1);
	if (nEndIndex == -1)
		nEndIndex = strColWidths.GetLength();
	str = strColWidths.Mid(nIndex == 0 ? 0 : (nIndex+1), nEndIndex - (nIndex == 0 ? 0 : nIndex+1));
	return atoi(str);
}

void CQuotesDlg::ResizeColumns()
{
	//(e.lally 2011-08-26) PLID 45210 - Check if we are remembering widths before tring to get them.
	CString strColWidths;
	BOOL bShowInfo = m_ShowPackageInfo.GetCheck();
	BOOL bRememberColumnWidths = GetRemotePropertyInt("RememberQuoteTabColumns", 0, 0, GetCurrentUserName(), true);
	if(bRememberColumnWidths){
		strColWidths = GetRemotePropertyText("DefaultQuoteTabColumnSizes", "", 0, GetCurrentUserName(), false);
	}
	IColumnSettingsPtr pCol;
	short i;
	//
	// (c.haag 2004-07-30 09:30) - PLID 13728 - Do all style assignments here
	//
	for (i = COLUMN_QUOTE_ID; i < COLUMN_ISPACKAGE; i++)
	{
		m_QuoteList->GetColumn(i)->ColumnStyle = (bRememberColumnWidths) ? csVisible : m_aDefColumnStyles[i];
	}

	// (d.thompson 2012-05-29) - PLID 50675 - This needs to go through the expired column, it was stopping 1 short.
	for (i = COLUMN_ISPACKAGE; i <= COLUMN_ORIGINAL_CURRENT_AMOUNT; i++)
	{
		//we never show the OriginalCurrentAmount
		if (bShowInfo && i != COLUMN_ORIGINAL_CURRENT_AMOUNT) {
			m_QuoteList->GetColumn(i)->ColumnStyle = (bRememberColumnWidths) ? (csVisible) : (csVisible | csWidthPercent);
		}
		else {
			m_QuoteList->GetColumn(i)->ColumnStyle = csVisible | csFixedWidth;
		}
	}
	//
	// Resize all the non-package related columns.
	//
	if(bRememberColumnWidths)
	{
		for (i = COLUMN_QUOTE_ID; i < COLUMN_ISPACKAGE; i++)
		{
			m_QuoteList->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);
		}
	}
	//
	// Resize package related columns
	//
	if(bShowInfo) {
		if(bRememberColumnWidths)
		{
			// (d.thompson 2012-05-29) - PLID 50675 - This needs to go through COLUMN_EXPIRED, it was stopping way short.
			for (i = COLUMN_ISPACKAGE; i <= COLUMN_ORIGINAL_CURRENT_AMOUNT; i++)
			{
				m_QuoteList->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);				
			}
		}
		else
		{
			// (e.lally 2008-06-17) - Updated default widths for 9pt font support
			pCol = m_QuoteList->GetColumn(COLUMN_ISPACKAGE);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol->PutStoredWidth(62);			
			pCol = m_QuoteList->GetColumn(COLUMN_PACKAGE_TYPE);
			pCol->PutColumnStyle(csVisible|csWidthData);
			pCol->PutStoredWidth(87);
			pCol = m_QuoteList->GetColumn(COLUMN_TOTAL_COUNT);
			pCol->PutColumnStyle(csVisible|csWidthData);
			pCol->PutStoredWidth(80);
			pCol = m_QuoteList->GetColumn(COLUMN_CURRENT_COUNT);
			pCol->PutColumnStyle(csVisible|csWidthData);
			pCol->PutStoredWidth(62);
			pCol = m_QuoteList->GetColumn(COLUMN_TOTAL_AMOUNT);
			pCol->PutColumnStyle(csVisible|csWidthData);
			pCol->PutStoredWidth(72);
			pCol = m_QuoteList->GetColumn(COLUMN_UNBILLED_AMOUNT);
			pCol->PutColumnStyle(csVisible|csWidthData);
			pCol->PutStoredWidth(100);
			pCol = m_QuoteList->GetColumn(COLUMN_UNPAID_AMOUNT);
			pCol->PutColumnStyle(csVisible|csWidthData);
			pCol->PutStoredWidth(95);
			// (d.thompson 2012-05-29) - PLID 50675 - For consistency, I added this, otherwise sometimes it has a style (if you
			//	remember columns) and sometimes it does not (if you are not remembering).
			pCol = m_QuoteList->GetColumn(COLUMN_EXPIRED);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol->PutStoredWidth(0);
			pCol = m_QuoteList->GetColumn(COLUMN_ORIGINAL_CURRENT_AMOUNT);
			pCol->PutColumnStyle(csVisible | csFixedWidth);
			pCol->PutStoredWidth(0);
		}
	}
	else {
		// (c.haag 2003-09-03 09:50) - These should be hidden even if you
		// remember column widths.
		pCol = m_QuoteList->GetColumn(COLUMN_ISPACKAGE);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_PACKAGE_TYPE);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_TOTAL_COUNT);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_CURRENT_COUNT);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_TOTAL_AMOUNT);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_UNBILLED_AMOUNT);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_UNPAID_AMOUNT);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		// (d.thompson 2012-05-29) - PLID 50675 - For consistency, I added this, otherwise sometimes it has a style (if you
		//	remember columns) and sometimes it does not (if you are not remembering).
		pCol = m_QuoteList->GetColumn(COLUMN_EXPIRED);
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_QuoteList->GetColumn(COLUMN_ORIGINAL_CURRENT_AMOUNT);
		pCol->PutColumnStyle(csVisible | csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}

void CQuotesDlg::OnRequeryFinishedQuoteList(short nFlags) 
{
	try {

		//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLongs and VarBools for added safety
		IRowSettingsPtr pRow = m_QuoteList->GetTopRow();
		while(pRow != NULL) {

			//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax
			if(VarBool(pRow->GetValue(COLUMN_ACTIVE),TRUE)){
				// (d.thompson 2009-08-18) - PLID 16758 - Check for expiration.  This field is a bool
				if(VarBool(pRow->GetValue(COLUMN_EXPIRED))) {
					//Not inactive, but it is expired
					pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
				}
				else {
					//Not expired and not inactive
					pRow->PutForeColor(RGB(0, 0, 0));
				}

			}
			else{
				// (d.thompson 2009-08-18) - PLID 16758 - Moved color to a define in globalfinancialutils
				pRow->PutForeColor(INACTIVE_QUOTE_FORECOLOR);
			}

			pRow = pRow->GetNextRow();
		}

		//don't calculate the package totals if the columns are hidden
		if (m_ShowPackageInfo.GetCheck()) {
			CalculateAllPackageTotals();
		}

	}NxCatchAll("Error in CQuotesDlg::OnRequeryFinishedQuoteList");
}

void CQuotesDlg::OnDblClickCellQuoteList(LPDISPATCH lpRow, short nColIndex) 
{
	try{
		//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL){
			return;
		}
		//DRT 7/25/03 - Moved here from OnLButtonDown
		CBillingModuleDlg dlg(this);

		if (CheckCurrentUserPermissions(bioPatientQuotes,sptRead)) {
			BeginWaitCursor();
			//(e.lally 2008-06-17) PLID 27303 - Switch to datalist2 syntax, use VarLong for added safety
			dlg.OpenWithBillID(VarLong(pRow->GetValue(COLUMN_QUOTE_ID)), BillEntryType::Quote, 0);
			EndWaitCursor();

			m_QuoteList->Requery();
		}
	}NxCatchAll("Error in CQuotesDlg::OnDblClickCellQuoteList");
}

void CQuotesDlg::OnRememberColumns() 
{
	try{
		long nRemember;
		if(IsDlgButtonChecked(IDC_REMEMBER_QUOTE_COL_WIDTHS))
		{
			SaveColumnWidths();
			nRemember = 1;
		}
		else
		{
			nRemember = 0;
			// (c.haag 2004-07-30 09:46) PLID 13728 - We need to restore the original
			// column widths and styles because the current ones are experessed in pixels,
			// and the column types will definitely change. Not doing this will result
			// in column sizes exploding off the screen.
			for (short i=0; i < m_QuoteList->GetColumnCount(); i++)
			{
				long n = m_aDefColumnWidths[i];
				m_QuoteList->GetColumn(i)->ColumnStyle = m_aDefColumnStyles[i];
				m_QuoteList->GetColumn(i)->StoredWidth = m_aDefColumnWidths[i];
			}
		}
		SetRemotePropertyInt("RememberQuoteTabColumns", nRemember, 0, GetCurrentUserName());

		//size the datalist appropriately
		ResizeColumns();
	}NxCatchAll("Error saving remembered columns in Quotes tab");
}

void CQuotesDlg::SaveColumnWidths()
{
	CString strCurrentColWidths = GetRemotePropertyText("DefaultQuoteTabColumnSizes", "", 0, GetCurrentUserName(), false);
	CString strWidths;
	for (short i=0; i < m_QuoteList->ColumnCount; i++)
	{
		IColumnSettingsPtr pCol = m_QuoteList->GetColumn(i);
		CString str;
		if (i == 0)
		{
			str.Format("%d", pCol->StoredWidth);
		}
		else if (i >= COLUMN_ISPACKAGE)
		{
			if (m_ShowPackageInfo.GetCheck())
			{
				str.Format(",%d", pCol->StoredWidth);
			}
			else
			{
				// (c.haag 2003-09-03 10:27) - If we get here, the
				// package info is hidden and we're trying to save
				// the widths of columns that are forced to be invisible.
				long nWidth;
				if (-1 != (nWidth=GetColumnWidth(strCurrentColWidths, i)))
				{
					str.Format(",%d", nWidth);
				}
				else
				{
					switch (i)
					{
					case COLUMN_ISPACKAGE: str = "60"; break;
					case COLUMN_TOTAL_COUNT: str = ",60"; break;
					case COLUMN_CURRENT_COUNT: str = ",60"; break;
					case COLUMN_TOTAL_AMOUNT: str = ",120"; break;
					case COLUMN_UNBILLED_AMOUNT: str = ",120"; break;
					case COLUMN_UNPAID_AMOUNT: str = ",120"; break;
					}
				}
			}
		}
		else
		{
			str.Format(",%d", pCol->StoredWidth);
		}
		strWidths += str;
	}
	SetRemotePropertyText("DefaultQuoteTabColumnSizes", strWidths, 0, GetCurrentUserName());
}

void CQuotesDlg::OnColumnSizingFinished(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try{
		if(GetRemotePropertyInt("RememberQuoteTabColumns", 0, 0, GetCurrentUserName(), true) == 1)
			SaveColumnWidths();
	}NxCatchAll("Error resizing column width in Quotes tab");
}


//DRT 5/5/2004 - PLID 12207 - Handle barcode scan messages.  Code copied and slightly modified from the CFinancialDlg::OnBarcodeScan
LRESULT CQuotesDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	//The billing module dialog registers for barcode messages, so we don't need to 
	//send them from here.

	// CH 4/5 Temporary idea: Open the billing
	// dialog
	//(e.lally 2008-06-17) - Put try/catch outside of check for permission in case that fails
	try{
		if(CheckCurrentUserPermissions(bioPatientQuotes,sptCreate)) {
			// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
			_bstr_t bstr = (BSTR)lParam;
			CString strCode = (LPCTSTR)bstr;

			//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
			GetBestUPCProduct(strCode);
			/*
			// (c.haag 2003-07-31 17:49) - Don't open it if it's not a barcode anywhere
			// (j.jones 2004-02-24 10:19) - it won't compare to see if the product is billable against the current location, just any location
			_RecordsetPtr prs = CreateRecordset("SELECT ServiceT.ID FROM ServiceT LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
				"WHERE BARCODE = '%s' AND Active <> 0 AND (ProductT.ID IS NULL OR ProductLocationInfoT.Billable <> 0)", _Q(strCode) );
			if (!prs->eof)
			{
				prs->Close();
				CBillingModuleDlg dlg;
				dlg.m_strBarcode = strCode;
				dlg.OpenWithBillID(-1, 2, 0);
				m_QuoteList->Requery();
			}
			*/
		}
	}
	NxCatchAll("Error validating barcode");

	return 0;
}

void CQuotesDlg::OnOpenCareCredit() 
{
	try {
		
		NxCareCredit::OpenCCWare(GetActivePatientID());

	}NxCatchAll("CQuotesDlg::OnOpenCareCredit");
}

// (a.walling 2008-06-03 10:04) - PLID 27686 - Free the bitmap
void CQuotesDlg::OnDestroy() 
{
	//(e.lally 2008-06-18) PLID 27303 - Add error handling. Generally, the NxDialg OnDestroy calls have been 
		//made first, outside the try/catch in other areas of code. I am leaving the execution sequence the same
		//but leaving it outside the try/catch.
	try{
		if (m_hBitmap)
			DeleteObject(m_hBitmap);
	}NxCatchAll("CQuotesDlg::OnDestroy()")
	CNxDialog::OnDestroy();
}

//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns matching service code or all matching inventory UPCs.

void CQuotesDlg::GetBestUPCProduct(CString strCode) 
{
try {

	_RecordsetPtr prs = CreateRecordset("SELECT ServiceT.ID FROM ServiceT LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
					"WHERE BARCODE = '%s' AND Active <> 0 AND (ProductT.ID IS NULL OR ProductLocationInfoT.Billable <> 0)", _Q(strCode) );
				if (!prs->eof)
				{
					prs->Close();
					CBillingModuleDlg dlg(this);
					dlg.m_strBarcode = strCode;
					dlg.OpenWithBillID(-1, BillEntryType::Quote, 0);
					m_QuoteList->Requery();
				}		
				
	} NxCatchAll(__FUNCTION__);
}

//updates the package totals in the currently displayed quote list
void CQuotesDlg::CalculateAllPackageTotals()
{
	try {

		m_QuoteList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

				//do we have any packages in the list that need calculated?
		std::vector<long> aryPackageIDs;

		{
			IRowSettingsPtr pRow = m_QuoteList->GetTopRow();
			while (pRow != NULL) {

				BOOL bIsPackage = VarBool(pRow->GetValue(COLUMN_ISPACKAGE));
				if (bIsPackage) {
					//have we already calculated the total?
					_variant_t varTotal = pRow->GetValue(COLUMN_UNPAID_AMOUNT);
					if (varTotal.vt != VT_CY) {
						//we need to calculate the total
						aryPackageIDs.push_back(VarLong(pRow->GetValue(COLUMN_QUOTE_ID)));
					}
				}
				pRow = pRow->GetNextRow();
			}
		}

		if ((long)aryPackageIDs.size() == 0) {
			//no packages to update, get out of here
			return;
		}

		CProgressDialog progressDialog;
		progressDialog.Start(NULL, CProgressDialog::NoCancel | CProgressDialog::AutoTime | CProgressDialog::NoMinimize | CProgressDialog::MarqueeProgress,
			"NexTech Practice", "Please wait...");
		progressDialog.SetLine(1, "Loading package information...");
		progressDialog.SetLine(2, "Calculating package balances for this patient, please wait...");

		progressDialog.SetProgress(0, (long)aryPackageIDs.size());

		long nIndex = 0;
		for each (long nPackageID in aryPackageIDs)
		{
			nIndex++;
			progressDialog.SetProgress(nIndex, m_QuoteList->GetRowCount());
			progressDialog.SetLine(2, "Calculating balance for package %li of %li", nIndex, (long)aryPackageIDs.size());

			IRowSettingsPtr pRow = m_QuoteList->FindByColumn(COLUMN_QUOTE_ID, (long)nPackageID, m_QuoteList->GetFirstRow(), VARIANT_FALSE);
			if (pRow != NULL && VarBool(pRow->GetValue(COLUMN_ISPACKAGE))) {
				
				COleCurrency cyRemBalanceNoTax = COleCurrency(0, 0);

				_variant_t varOriginalCurrentAmount = pRow->GetValue(COLUMN_ORIGINAL_CURRENT_AMOUNT);
				COleCurrency cyOriginalCurrentAmount = VarCurrency(varOriginalCurrentAmount, COleCurrency(0, 0));

				//if this package has never been billed, simply get the current amount minus any prepayments
				if (!VarBool(pRow->GetValue(COLUMN_BILLED), FALSE)) {
					cyRemBalanceNoTax = cyOriginalCurrentAmount;
					cyRemBalanceNoTax -= CalculatePrePayments(m_id, nPackageID, false);

					//there's no need to make it less than zero
					if (cyRemBalanceNoTax < COleCurrency(0, 0)) {
						cyRemBalanceNoTax = COleCurrency(0, 0);
					}
				}
				else {
					//calculate the full balance the slow way
					
					//this screen only wants the no-tax balance
					CalculateRemainingPackageBalance(nPackageID, m_id, varOriginalCurrentAmount, cyRemBalanceNoTax);
				}

				pRow->PutValue(COLUMN_UNPAID_AMOUNT, _variant_t(cyRemBalanceNoTax));
			}
			else {
				//either the row wasn't found, or it is not a package,
				//this should be impossible!
				ASSERT(FALSE);
			}
		}

		progressDialog.Stop();

	}NxCatchAll(__FUNCTION__);
}