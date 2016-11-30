// UpdatePriceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdatePriceDlg.h"
#include "AuditTrail.h"

#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CUpdatePriceDlg dialog


CUpdatePriceDlg::CUpdatePriceDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdatePriceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdatePriceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CUpdatePriceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdatePriceDlg)
	DDX_Control(pDX, IDC_CATEGORY_CHECK, m_btnCategory);
	DDX_Control(pDX, IDC_ROUND_CHARGES, m_btnRound);
	DDX_Control(pDX, IDC_SURGERY_CHECK, m_btnSurgery);
	DDX_Control(pDX, IDC_UPDATE_PERCENTAGE, m_nxeditUpdatePercentage);
	DDX_Control(pDX, IDC_DESC_TEXT, m_nxstaticDescText);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdatePriceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdatePriceDlg)
	ON_BN_CLICKED(IDC_CATEGORY_CHECK, OnCategoryCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdatePriceDlg message handlers

BOOL CUpdatePriceDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_listCategory = BindNxDataListCtrl(this, IDC_UPDATE_CATEGORY_LIST, GetRemoteData(), true);

	if(nType == 1) {
		//we're updating CPT Codes
		SetDlgItemText(IDC_DESC_TEXT, "Updating Prices for Service Codes to ...");
	}
	else if(nType == 2) {
		//We're updating inventory items
		SetDlgItemText(IDC_DESC_TEXT, "Updating Prices for Inventory Items to ...");
	}
	else {
		//invalid type
		MsgBox("Invalid dialog type.");
		EndDialog(0);
	}

	//surgeries are on by default
	CheckDlgButton(IDC_SURGERY_CHECK, true);

	//round by default
	CheckDlgButton(IDC_ROUND_CHARGES, true);

	OnCategoryCheck();	//disable the dl

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdatePriceDlg::OnOK() 
{
	try {
		double dblPercent;
		CString str, strExec;

		if(IsDlgButtonChecked(IDC_CATEGORY_CHECK)) {
			if(m_listCategory->GetCurSel() == -1) {
				MsgBox("You must choose a category before proceeding.");
				return;
			}
		}

		//must have a percent > 0 and < 1001 (for safety)
		GetDlgItemText(IDC_UPDATE_PERCENTAGE, str);

		if(str.IsEmpty()) {
			MsgBox("You must enter a percentage to continue updating prices.");
			return;
		}

		if(str.SpanIncluding("1234567890.") != str) {
			MsgBox("You have entered invalid characters for the percentage.  Please fix this and try again.");
			return;
		}

		dblPercent = atof(str);

		//see if it's close to 0
		if(dblPercent <= 0.49) {	//rounded down to 0
			//This allows them to set all prices to 0. It will warn them once of this specific fact, and once again 
			//in the normal confirmation.  
			if(MsgBox(MB_YESNO, "You are attempting to update the prices to 0.  This will clear the price of every item in your data.\n"
				"Are you sure you wish to do this?")==IDNO)
				return;
		}

		if(dblPercent < 0.0) {
			MsgBox("You cannot update a percentage to a negative value.  Please enter a positive percentage and try again.");
			return;
		}

		if(dblPercent > 1000.0) {
			MsgBox("You cannot automatically update to a percentage > 1000.  If you really need to update to a number that high, update to 1000, "
				"and then run another update.");
			return;
		}

		//confirmation
		COleCurrency cyTest(0, 0);
		CString strTest;
		strTest.Format("%g", dblPercent);
		cyTest.ParseCurrency(strTest);	//we'll use this currency value to test against

		if(MsgBox(MB_YESNO, "You are about to update all of your prices to %g percent of their current cost.  For example, a $100 charge will become %s.\n"
			"Are you sure you wish to do this?", dblPercent, FormatCurrencyForInterface(cyTest, true)) == IDNO)
			return;

		// (m.hancock 2006-10-17 11:30) - PLID 13782 - We're ready to start updating, so display a wait cursor to show that we're busy.
		CWaitCursor cwait;

		//all ready for updating
		CString strUpdateType = "";
		if(nType == 1) {
			//CPT Codes			
			strUpdateType = "CPTCodeT";
		}
		else if(nType == 2) {
			//Inventory Items
			strUpdateType = "ProductT";
		}
		else {
			MsgBox("Invalid update type.");
			return;
		}

		long nRound = 2;	//round to 2 decimal places
		if(IsDlgButtonChecked(IDC_ROUND_CHARGES)) {
			if(MsgBox(MB_YESNO, "You have chosen to round to the nearest dollar.  Be aware that this is an irreversible change!  Are you sure you want to do this?") == IDNO)
				return;
			nRound = 0;		//round to 0 decimal places (nearest dollar)
		}

		BEGIN_TRANS("UpdatePrices") {
			strExec.Format("UPDATE ServiceT SET Price = Round(convert(money, (Price * %g)), %li) "
				"WHERE ID IN (SELECT ID FROM %s) AND Price > 0 AND UseAnesthesiaBilling = 0 AND UseFacilityBilling = 0 ",
				dblPercent/100.0, nRound, strUpdateType);

			if(IsDlgButtonChecked(IDC_CATEGORY_CHECK)) {
				str.Format(" AND Category = %li ", VarLong(m_listCategory->GetValue(m_listCategory->GetCurSel(), 0)));
				strExec += str;
			}

			ExecuteSql("%s", strExec);

			//now check to see if they wanted to do surgery prices for these items
			if(IsDlgButtonChecked(IDC_SURGERY_CHECK)) {
				strExec.Format("UPDATE SurgeryDetailsT SET Amount = Round(convert(money, (Amount * %g)), %li) "
					"WHERE ServiceID IN (SELECT ID FROM %s) "
					"AND ServiceID IN (SELECT ID FROM ServiceT WHERE UseAnesthesiaBilling = 0 AND UseFacilityBilling = 0) ",
					dblPercent/100.0, nRound, strUpdateType);

				if(IsDlgButtonChecked(IDC_CATEGORY_CHECK)) {
					str.Format(" AND ServiceID IN (SELECT ID FROM ServiceT WHERE Category = %li) ", VarLong(m_listCategory->GetValue(m_listCategory->GetCurSel(), 0)));
					strExec += str;
				}

				ExecuteSql("%s", strExec);
			}

		} END_TRANS_CATCH_ALL("UpdatePrices");

		//auditing
		CString strNew;
		strNew.Format("to %g%%", dblPercent);
		AuditEventItems aeiItem;

		if(nType == 1)
			aeiItem = aeiUpdateCPTPrices;
		else if(nType == 2)
			aeiItem = aeiUpdateInvPrices;

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiItem, -1, "", strNew, aepHigh, aetChanged);

	} NxCatchAll("Error in CUpdatePriceDlg::OnOK()");

	CDialog::OnOK();
}

void CUpdatePriceDlg::OnCancel() 
{
	//nada

	CDialog::OnCancel();
}

void CUpdatePriceDlg::OnCategoryCheck() 
{
	//if it's checked, enable, otherwise, disable
	if(IsDlgButtonChecked(IDC_CATEGORY_CHECK))
		m_listCategory->Enabled = true;
	else 
		m_listCategory->Enabled = false;
}
