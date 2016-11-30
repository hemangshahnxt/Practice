// ApplyListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PaymentDlg.h"
#include "ApplyListDlg.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "FinancialApply.h"
#include "NxStandard.h"
#include "FinancialDlg.h"
#include "GlobalDrawingUtils.h"

#include <afxtempl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*	Maybe useful later to have standards
#define PAYMENT_COLOR		0x00408000
#define ADJUSTMENT_COLOR	0x005F5F30
						  //0x00808040
#define REFUND_COLOR		0x00008080
*/

//column IDs
enum ApplyListTypeColumn {
	altcLineID = 0, 
	altcPayID, 
	altcDate, 
	altcDescription,
	altcRespAmt, 
	altcRespType,
	altcInsCo,
};

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CApplyListDlg dialog


CApplyListDlg::CApplyListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApplyListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApplyListDlg)
	//}}AFX_DATA_INIT

	m_iDestID = -1;
}

CApplyListDlg::~CApplyListDlg()
{
}


void CApplyListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApplyListDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADD_PAYMENT, m_btnNewPay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApplyListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CApplyListDlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD_PAYMENT, OnAddPayment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApplyListDlg message handlers

void CApplyListDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	m_List = BindNxDataListCtrl(this,IDC_LIST,GetRemoteData(),false);

	if (!bShow) return;

	//TODO:  These should really be in the resources...
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcLineID, _T("PayID"), _T("PayID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcPayID, _T("PayID"), _T("PayID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcDate, _T("Date"), _T("Date"), 14, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcDescription, _T("Description"), _T("Payment"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcRespAmt, _T("RespAmount"), _T("Resp"), 15, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcRespType, _T("RespType"), _T("Type"), 15, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(altcInsCo, _T("InsCoName"), _T("InsCo"), 15, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;

//DRT 5/29/03 - Removed!  Payments can only be 1 resp at a time, so why put 3 columns?
//	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(4, _T("PatResp"), _T("Patient"), 15, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
//	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(5, _T("InsResp1"), _T("Primary"), 15, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
//	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(6, _T("InsResp2"), _T("Secondary"), 15, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;

	m_List->IsComboBox = FALSE;
	m_List->GetColumn(altcDate)->PutSortPriority(0);
	m_List->GetColumn(altcDate)->PutSortAscending(FALSE);

	FillAppliesList();
}

BEGIN_EVENTSINK_MAP(CApplyListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CApplyListDlg)
	ON_EVENT(CApplyListDlg, IDC_LIST, 4 /* LButtonDown */, OnLButtonDownList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CApplyListDlg::FillAppliesList()
{
	_RecordsetPtr rs(__uuidof(Recordset));
	CString str, temp;

	//DRT 5/29/03 - Rewrote this to be roughly 1000x more efficient and readable.  Plus changed it to only pull the resp 
	//		for each payment, instead of 3 columns of nonsense
	//JMJ 7/23/03 - Included applies to payments, so the list showed the balance of a payment.
	str.Format("(SELECT PaysQ.PayID, Amount - (CASE WHEN ChargeAppliesQ.ApplyAmt IS NULL THEN 0 ELSE ChargeAppliesQ.ApplyAmt END) "
			"+ (CASE WHEN PaymentAppliesQ.ApplyAmt IS NULL THEN 0 ELSE PaymentAppliesQ.ApplyAmt END) AS RespAmount, "
			"CASE WHEN InsuranceCoT.Name IS NULL THEN '<None>' ELSE InsuranceCoT.Name END AS InsCoName, "
			"CASE WHEN RespTypeT.ID IS NULL THEN 'Patient' ELSE RespTypeT.TypeName END AS RespType, "
			"Description, Date "
			"FROM "
				"(/* The amount of all non-deleted payments for this patient */ "
				"SELECT LineItemT.ID AS PayID, Amount, PatientID, InsuredPartyID, Description, LineItemT.Date "
				"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"WHERE Deleted = 0 AND Type >= 1 AND Type <= 2 "
				") PaysQ "
			"LEFT JOIN "
				"(/* The amt of each payment that has been applied for this patient */ "
				"SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS ApplyAmt, PatientID "
				"FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
				"WHERE Deleted = 0 "
				"GROUP BY SourceID, PatientID "
				") ChargeAppliesQ "
			"ON PaysQ.PayID = ChargeAppliesQ.SourceID "
			"LEFT JOIN "
				"(/* The amt of each payment that has had applies to it*/ "
				"SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS ApplyAmt, PatientID "
				"FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
				"WHERE Deleted = 0 "
				"GROUP BY DestID, PatientID "
				") PaymentAppliesQ "
			"ON PaysQ.PayID = PaymentAppliesQ.DestID "
			"LEFT JOIN InsuredPartyT ON PaysQ.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE PaysQ.PatientID = %li) SubQ ", GetActivePatientID());

	//DRT 5/29/03 - Instead of doing something foolish, like, say, building a recordset, looping through it
	//		to put all the items in an array, then looping through that array and manually loading them into 
	//		the datalist, let's just make this the from clause and requery.
	m_List->FromClause = _bstr_t(str);
	m_List->WhereClause = _bstr_t("RespAmount <> 0");
	m_List->Requery();
}

void CApplyListDlg::OnOK()
{
	CDialog::OnOK();
}

void CApplyListDlg::OnLButtonDownList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1 || nCol == -1) return;  //The user didn't click on a valid cell.
	COleVariant varPayID = m_List->GetValue(nRow, altcPayID);
	if (varPayID.vt != VT_I4) // This should never happen
		return;

	try {
		
		// (j.jones 2011-08-24 17:40) - PLID 45176 - CanApplyLineItem will check the normal apply create permission,
		// but only after it checks to see if the source payment is closed
		// (j.jones 2013-07-01 10:45) - PLID 55517 - this function can now potentially correct the payment, if so
		// the payment ID will be changed to reflect the new, corrected line item
		long nPayID = VarLong(varPayID);
		ECanApplyLineItemResult eResult = CanApplyLineItem(nPayID);
		if(eResult == ecalirCannotApply) {
			return;
		}

		if(InvokeFinancialApplyDlg(m_iDestID, nPayID, GetActivePatientID(), m_strClickedType)) {
			FillAppliesList();
		}
		// (j.jones 2013-07-01 16:17) - PLID 55517 - if they don't apply, we still need to reload
		// the list if they corrected the line item
		else if(eResult == ecalirCanApply_IDHasChanged) {
			FillAppliesList();
		}

	}NxCatchAll("Error in applying payment.");
}

void CApplyListDlg::OnAddPayment() 
{
//	if (FALSE == CheckAccess("Financial", "Payment/Apply Transaction"))
	if (!UserPermission(EditPayment))
		return;

	///////////////////////////////////////////
	// Add a new payment (but don't apply it)
	CPaymentDlg dlg(this);
	if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
		FillAppliesList();
	}
}

BOOL CApplyListDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_brush.CreateSolidBrush(PaletteColor(0x9CC294));

		// (c.haag 2008-05-01 15:18) - PLID 29871 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnNewPay.AutoSet(NXB_NEW);
	}
	NxCatchAll("Error in CApplyListDlg::OnInitDialog");
	
	return TRUE;
}
