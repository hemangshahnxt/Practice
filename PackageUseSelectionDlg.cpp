// PackageUseSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PackageUseSelectionDlg.h"
#include "GlobalFinancialUtils.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_ID			0
#define COLUMN_LINE_ID		1
#define COLUMN_SERVICE_ID	2
#define	COLUMN_DESCRIPTION	3
#define	COLUMN_QTY_ORIG		4
#define COLUMN_QTY_REM		5
#define COLUMN_BILL_CHECK	6
#define	COLUMN_QTY_TO_BILL	7

using namespace NXDATALISTLib;
using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CPackageUseSelectionDlg dialog


CPackageUseSelectionDlg::CPackageUseSelectionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPackageUseSelectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPackageUseSelectionDlg)
		m_nQuoteID = -1;
		m_bPackageIsUsedUp = FALSE;
		m_cyMatchRemainingPackageAmount = COleCurrency(0,0);
		m_nMatchRemAmountToChargeID = -1;
	//}}AFX_DATA_INIT
}

CPackageUseSelectionDlg::~CPackageUseSelectionDlg()
{
	ClearPackageChargeArray();
}

void CPackageUseSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPackageUseSelectionDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPackageUseSelectionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPackageUseSelectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPackageUseSelectionDlg message handlers

BOOL CPackageUseSelectionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-05-01 17:43) - PLID 29876 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_PackageList = BindNxDataListCtrl(this, IDC_PACKAGE_LIST, GetRemoteData(), false);

		CString strWhere;
		// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
		strWhere.Format("Deleted = 0 AND BillID = %li AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0))", m_nQuoteID);
		m_PackageList->PutWhereClause(_bstr_t(strWhere));
		m_PackageList->Requery();
	}
	NxCatchAll("Error in CPackageUseSelectionDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPackageUseSelectionDlg::OnOK() 
{
	try {

		long nFirstChargeID = -1;

		BOOL bAreAnyChecked = FALSE;

		//used to tell BillingDlg whether or not they're using the last of the package
		m_bPackageIsUsedUp = TRUE;

		CString strProblemItemDescription = "";

		for(int i=0;i<m_PackageList->GetRowCount();i++) {

			_variant_t var = m_PackageList->GetValue(i,COLUMN_BILL_CHECK);

			BOOL bChecked = VarBool(var, FALSE);

			long nPackageChargeID = VarLong(m_PackageList->GetValue(i,COLUMN_ID));
			long nServiceID = VarLong(m_PackageList->GetValue(i,COLUMN_SERVICE_ID));

			double dblQtyRem = VarDouble(m_PackageList->GetValue(i,COLUMN_QTY_REM),0.0);
			double dblQtyToBill = VarDouble(m_PackageList->GetValue(i,COLUMN_QTY_TO_BILL),0.0);
			double dblNewRemainingAmount = dblQtyRem - dblQtyToBill;

			if(bChecked) {

				bAreAnyChecked = TRUE;

				//store the service ID and quantity				

				if(dblQtyToBill == 0) {
					AfxMessageBox("Each checked service must have a 'Qty. To Bill' greater than 0.");
					ClearPackageChargeArray();
					return;
				}

				// (j.jones 2007-03-26 14:38) - PLID 25287 - track the service code
				// for the first charge in the quote, which SHOULD be LineID = 1 but
				// that is not database-enforced so we have to calculate it, just once,
				// only if we are adding a code
				// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
				if(nFirstChargeID == -1) {
					nFirstChargeID = CalculateMultiUsePackageOverageChargeID(m_nQuoteID);
				}

				//if we are selecting the first charge ID, we need to do two things:
				//1. if we are billing enough such that there is less than 1 remaining
				//but greater than zero, then warn that we can't allow this.
				//2. if billing down to zero, assign it to the member variable
				if(nPackageChargeID == nFirstChargeID) {

					double dblNewRemainingAmount = dblQtyRem - dblQtyToBill;

					if(dblNewRemainingAmount > 0.0 && dblNewRemainingAmount < 1.0) {

						strProblemItemDescription = VarString(m_PackageList->GetValue(i,COLUMN_DESCRIPTION), "");

						//don't warn now, wait until we calculate the overage, then warn them if overage exists
					}
					else if(dblNewRemainingAmount == 0.0) {

						//we're billing the last of our target charge, so store the ID

						m_nMatchRemAmountToChargeID = nFirstChargeID;
					}
				}

				// (j.jones 2008-05-22 11:49) - PLID 28450 - added a new package charge array
				// that replaced the old service & quantity arrays
				PackageChargeObject *pNew = new PackageChargeObject;
				pNew->nPackageChargeID = nPackageChargeID;
				pNew->nServiceID = nServiceID;
				pNew->dblQuantity = dblQtyToBill;
				m_arypPackageCharges.Add(pNew);
			}

			if(dblNewRemainingAmount > 0.0) {
				//if there are any left after this bill, then it's not used up
				m_bPackageIsUsedUp = FALSE;
			}
		}

		if(!bAreAnyChecked) {
			AfxMessageBox("You must have at least one service checked before clicking 'OK'.");
			ClearPackageChargeArray();
			return;
		}

		// (j.jones 2007-03-26 14:38) - PLID 25287 - If we are billing the first charge in the package
		// for the last time, calculate the excess amount (if any) that needs to be assigned to this service.
		// Also calculate if we are billing that service down to a value between 0 and 1, such that we will
		// warn about this if an overage would be used on the final billing.
		if(m_nMatchRemAmountToChargeID != -1 || !strProblemItemDescription.IsEmpty()) {
			
			m_cyMatchRemainingPackageAmount = CalculateMultiUsePackageOverageAmount_ForBalance(m_nQuoteID);
				
			//don't claim success just yet, because if we have an overage amount
			//and are not billing a quantity >= 1.0, we can't bill this package
			if(!strProblemItemDescription.IsEmpty() && m_cyMatchRemainingPackageAmount != COleCurrency(0,0)) {
				CString str;
				str.Format("The charge '%s' is going to be assigned an extra amount of %s to balance with the package total.\r\n"
					"Because of this, its final quantity must be at least 1 or greater. "
					"You may not bill this item such that the remaining quantity is between 0 and 1.",
					strProblemItemDescription, FormatCurrencyForInterface(m_cyMatchRemainingPackageAmount, TRUE, TRUE));
				AfxMessageBox(str);
				ClearPackageChargeArray();
				return;
			}
		}

		//if we are truly not billing the final charge we've targeted to apply the "overage" to,
		//but we did calculate the overage amount, wipe out that amount
		if(m_nMatchRemAmountToChargeID == -1) {
			m_cyMatchRemainingPackageAmount = COleCurrency(0,0);
		}

		CDialog::OnOK();

	}NxCatchAll("Error in OnOK");	
}

void CPackageUseSelectionDlg::OnCancel() 
{
	if(IDNO == MessageBox("Cancelling this selection screen will cancel billing this package.\n"
		"Are you sure you wish to cancel billing this package?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
		return;
	}
	
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CPackageUseSelectionDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPackageUseSelectionDlg)
	ON_EVENT(CPackageUseSelectionDlg, IDC_PACKAGE_LIST, 8 /* EditingStarting */, OnEditingStartingPackageList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPackageUseSelectionDlg, IDC_PACKAGE_LIST, 9 /* EditingFinishing */, OnEditingFinishingPackageList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPackageUseSelectionDlg, IDC_PACKAGE_LIST, 10 /* EditingFinished */, OnEditingFinishedPackageList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPackageUseSelectionDlg, IDC_PACKAGE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPackageList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPackageUseSelectionDlg::OnEditingStartingPackageList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		if(nCol == COLUMN_QTY_TO_BILL) {

			//if the item is not checked, the user cannot edit the quantity to bill

			_variant_t var = m_PackageList->GetValue(nRow,COLUMN_BILL_CHECK);

			BOOL bChecked = VarBool(var, FALSE);

			if(!bChecked) {
				*pbContinue = FALSE;
				return;
			}
		}
		else if(nCol == COLUMN_BILL_CHECK) {
			//if the remaining quantity is zero, they cannot check the box
			double dblQtyRem = VarDouble(m_PackageList->GetValue(nRow, COLUMN_QTY_REM),0.0);
			if(dblQtyRem <= 0.0) {
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll("Error in OnEditingStartingPackageList");
}

void CPackageUseSelectionDlg::OnEditingFinishingPackageList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		if(nCol == COLUMN_QTY_TO_BILL) {

			//if the item is not checked, we shouldn't be able to get here, but if we do, don't allow a change

			_variant_t var = m_PackageList->GetValue(nRow,COLUMN_BILL_CHECK);

			BOOL bChecked = VarBool(var, FALSE);

			if(!bChecked) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				return;
			}
			
			if(pvarNewValue->vt == VT_R8) {

				if(pvarNewValue->dblVal <= 0.0) {
					*pvarNewValue = varOldValue;
					*pbCommit = FALSE;
					AfxMessageBox("You must bill a quantity greater than zero.");
					return;
				}
				else {
					//it's a valid quantity, but compare that it's a valid amount remaining

					if(pvarNewValue->dblVal != varOldValue.dblVal) {
						double dblCount = VarDouble(m_PackageList->GetValue(nRow,COLUMN_QTY_REM),0.0);
						if(pvarNewValue->dblVal > dblCount) {
							*pvarNewValue = varOldValue;
							*pbCommit = FALSE;
							AfxMessageBox("You cannot bill more than there are remaining uses for this service.");
							return;
						}
					}
				}
			}
		}

	}NxCatchAll("Error in OnEditingFinishingPackageList");
}

void CPackageUseSelectionDlg::OnEditingFinishedPackageList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if(nCol == COLUMN_BILL_CHECK) {

			BOOL bChecked = VarBool(varNewValue, FALSE);

			IRowSettingsPtr pRow = m_PackageList->GetRow(nRow);

			if(bChecked) {
				//mark the cell as black, indicating it's editable
				pRow->PutCellForeColor(COLUMN_QTY_TO_BILL, RGB(0,0,0));

				//set the default quantity to bill to 1, or the quantity remaining if it is a fractional quantity less than 1
				double dblOneIncrement = 1.0;
				double dblQtyRem = VarDouble(m_PackageList->GetValue(nRow, COLUMN_QTY_REM),0.0);
				if(dblQtyRem < 1.0)
					dblOneIncrement = dblQtyRem;
				pRow->PutValue(COLUMN_QTY_TO_BILL, (double)dblOneIncrement);
			}
			else {
				//mark the cell as gray, indicating it's not editable
				pRow->PutCellForeColor(COLUMN_QTY_TO_BILL, RGB(112,112,112));
				//set the default quantity to bill to 0
				pRow->PutValue(COLUMN_QTY_TO_BILL, (double)0.0);
			}
		}

	}NxCatchAll("Error in OnEditingFinishedPackageList");
}

void CPackageUseSelectionDlg::OnRequeryFinishedPackageList(short nFlags) 
{
	try {

		//we want to color rows appropriately

	for(int i=0;i<m_PackageList->GetRowCount();i++) {

			// (j.jones 2008-05-22 11:49) - PLID 28450 - added a new package charge array
			// that replaced the old service & quantity arrays

			//the arrays may have been prefilled, we need to adjust the remaining quantity appropriately
			for(int j=0;j<m_arypPackageCharges.GetSize();j++) {

				PackageChargeObject *pCharge = (PackageChargeObject*)(m_arypPackageCharges.GetAt(j));

				if(pCharge->nPackageChargeID == VarLong(m_PackageList->GetValue(i,COLUMN_ID))) {
					//adjust accordingly
					double dblCurRemQty = VarDouble(m_PackageList->GetValue(i, COLUMN_QTY_REM),0.0);
					// (j.gruber 2007-01-16 14:55) - PLID 24261 - this should be j, not i
					dblCurRemQty -= pCharge->dblQuantity;
					if(dblCurRemQty < 0.0) {
						//shouldn't really be possible
						ASSERT(FALSE);
						dblCurRemQty = 0.0;
					}
					m_PackageList->PutValue(i, COLUMN_QTY_REM, dblCurRemQty);
				}
			}

			//if there are no uses remaining, gray out the row
			if(VarDouble(m_PackageList->GetValue(i, COLUMN_QTY_REM),0.0) <= 0.0) {
				IRowSettingsPtr pRow = m_PackageList->GetRow(i);
				pRow->PutForeColor(RGB(112,112,112));

				//also try to remove the checkbox altogether
				_variant_t varNull;
				varNull.vt = VT_NULL;
				pRow->PutValue(COLUMN_BILL_CHECK, varNull);
				pRow->PutValue(COLUMN_QTY_TO_BILL, varNull);
			}
		}

		//by default, color the "Qty. To Bill" column gray

		IColumnSettingsPtr pCol = m_PackageList->GetColumn(COLUMN_QTY_TO_BILL);
		pCol->PutForeColor(RGB(112,112,112));

		//the arrays may have been prefilled, but are no longer needed
		ClearPackageChargeArray();

	}NxCatchAll("Error in OnRequeryFinishedPackageList");
}

// (j.jones 2008-05-22 11:45) - PLID 28450 - this function will properly clear m_arypPackageCharges
void CPackageUseSelectionDlg::ClearPackageChargeArray()
{
	try {

		for(int i=m_arypPackageCharges.GetSize()-1; i>=0; i--) {
			delete (PackageChargeObject*)(m_arypPackageCharges.GetAt(i));
		}
		m_arypPackageCharges.RemoveAll();

	}NxCatchAll("Error in CPackageUseSelectionDlg::ClearPackageChargeArray");
}