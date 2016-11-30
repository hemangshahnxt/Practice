// MultiFeeAdjustmentDlg.cpp : implementation file
//

#include "stdafx.h"

// (a.walling 2008-07-07 17:43) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
#ifdef INCLUDEDEADCODE

#include "practice.h"
#include "MultiFeeAdjustmentDlg.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMultiFeeAdjustmentDlg dialog


CMultiFeeAdjustmentDlg::CMultiFeeAdjustmentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiFeeAdjustmentDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiFeeAdjustmentDlg)
	m_ItemCount = 0;
	//}}AFX_DATA_INIT
}

CMultiFeeAdjustmentDlg::~CMultiFeeAdjustmentDlg() {
	for(int i=g_aryMultiFeeAdjustmentT.GetSize()-1;i>=0;i--)
		delete g_aryMultiFeeAdjustmentT.GetAt(i);
	g_aryMultiFeeAdjustmentT.RemoveAll();
}

void CMultiFeeAdjustmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiFeeAdjustmentDlg)
	DDX_Control(pDX, IDC_LABEL1, m_nxstaticLabel1);
	DDX_Control(pDX, IDC_LABEL2, m_nxstaticLabel2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiFeeAdjustmentDlg, CDialog)
	//{{AFX_MSG_MAP(CMultiFeeAdjustmentDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiFeeAdjustmentDlg message handlers

void CMultiFeeAdjustmentDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	m_List = BindNxDataListCtrl(this,IDC_LIST,GetRemoteData(),false);
	
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(0, _T("LineID"), _T("LineID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(1, _T("CPTCode"), _T("Code"), 60, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(2, _T("CPTSubCode"), _T("Sub"), 45, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(3, _T("Description"), _T("Description"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(4, _T("ChargedFee"), _T("Charged Fee"), 85, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(5, _T("InsuranceFee"), _T("Insurance Fee"), 85, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(6, _T("Adjustment"), _T("Adjustment"), 85, csVisible|csFixedWidth|csEditable)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(7, _T("WriteOffAdjustment"), _T("Adjust"), 65, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	//TODO: the WriteOffAdjustment should be an editable boolean!
	m_List->IsComboBox = FALSE;
	m_List->GetColumn(0)->PutSortPriority(0);
	m_List->GetColumn(0)->PutSortAscending(TRUE);

	m_List->GetColumn(1)->PutForeColor(0x0097DCE8);
	m_List->GetColumn(2)->PutForeColor(0x0097DCE8);
	m_List->GetColumn(3)->PutForeColor(0x0097DCE8);
	m_List->GetColumn(4)->PutForeColor(0x00FFC0C0);
	m_List->GetColumn(5)->PutForeColor(0x00B0B0EE);
	m_List->GetColumn(6)->PutForeColor(0x00C0FFC0);
	m_List->GetColumn(7)->PutForeColor(0x0097DCE8);
}

void CMultiFeeAdjustmentDlg::AddCharge(int iChargeID, CString strCPTCode, CString strCPTSubCode, CString strDescription, COleCurrency cyChargedFee, COleCurrency cyInsuranceFee)
{
	COleCurrency cyDifference;
	COleVariant var;

	try {
		MultiFeeAdj *pNew = new MultiFeeAdj;
		if(g_aryMultiFeeAdjustmentT.GetSize()==0)
			pNew->LineID = (long)1;
		else
			pNew->LineID = (long)(((MultiFeeAdj*)g_aryMultiFeeAdjustmentT.GetAt(g_aryMultiFeeAdjustmentT.GetSize()-1))->LineID.lVal + 1);


		pNew->ChargeID = (long)iChargeID;
		pNew->CPTCode = _bstr_t(strCPTCode);
		pNew->CPTSubCode = _bstr_t(strCPTSubCode);
		pNew->Description = _bstr_t(strDescription);
		pNew->ChargedFee = cyChargedFee;
		pNew->InsuranceFee = cyInsuranceFee;
		var.vt = VT_BOOL;
		var.boolVal = -1;
		pNew->WriteOffAdjustment = var;

		cyDifference = cyInsuranceFee - cyChargedFee;
		pNew->Adjustment = cyDifference;
		g_aryMultiFeeAdjustmentT.Add(pNew);

		m_ItemCount++;
	}
	NxCatchAll("Error in MultiFeeAdjustment::AddCharge()");	

}

int CMultiFeeAdjustmentDlg::GetChargeCount()
{
	return m_ItemCount;
}

void CMultiFeeAdjustmentDlg::EmptyList()
{
	for(int i=g_aryMultiFeeAdjustmentT.GetSize()-1;i>=0;i--)
		delete g_aryMultiFeeAdjustmentT.GetAt(i);
	g_aryMultiFeeAdjustmentT.RemoveAll();
	m_ItemCount = 0;
}

BEGIN_EVENTSINK_MAP(CMultiFeeAdjustmentDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CMultiFeeAdjustmentDlg)
	ON_EVENT(CMultiFeeAdjustmentDlg, IDC_LIST, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMultiFeeAdjustmentDlg, IDC_LIST, 10 /* EditingFinished */, OnEditingFinishedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

int CMultiFeeAdjustmentDlg::CreateAdjustment(COleCurrency cyAdjustment, CString strChargeDesc)
{
	try {
		BeginWaitCursor();

		//////////////////////////////////////////////////////////
		// Write new adjustment to payments table
		//
		// MISSING: Provider ID
		//////////////////////////////////////////////////////////

		strChargeDesc = CString("For ") + strChargeDesc;

		// Add data to Payments table
		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"INSERT INTO LineItemT (\r\n"
			"	PatientID, LocationID, InputName, InputDate, Date,\r\n"
			"	Amount, Type, Description\r\n"
			") VALUES (\r\n"
			"	{INT}, {INT}, {STRING}, GetDate(), GetDate(),\r\n"
			"	{OLECURRENCY}, 2, {STRING})\r\n"
			
			"DECLARE @LineItemID INT\r\n"
			"SET @LineItemID = SCOPE_IDENTITY()\r\n"

			"INSERT INTO PaymentsT (ID, PayMethod, InsuredPartyID) VALUES (@LineItemID, 6, -1)\r\n"
			
			"SELECT @LineItemID AS LineItemID\r\n"
			"SET NOCOUNT OFF\r\n",
			GetActivePatientID(), GetCurrentLocation(), GetCurrentUserName(),
			cyAdjustment, strChargeDesc);

		long nPaymentID = AdoFldLong(prs, "LineItemID");

		EndWaitCursor();
		return nPaymentID;
	}
	NxCatchAll("Error in CreateAdjustment()");
	return -1;
}

void CMultiFeeAdjustmentDlg::OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	//TODO: the WriteOffAdjustment (7) should be an editable boolean!
	
	if(nCol == 6) {
		if(pvarNewValue->vt == VT_CY) {
			*pbCommit = TRUE;
			*pbContinue = TRUE;
		}
	}
	
}

void CMultiFeeAdjustmentDlg::OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//TODO: the WriteOffAdjustment (7) should be an editable boolean!
	if(bCommit) {
		if(nCol == 6) {
			((MultiFeeAdj*)g_aryMultiFeeAdjustmentT.GetAt(nRow))->Adjustment = varNewValue;
		}
	}	
}

void CMultiFeeAdjustmentDlg::OnOK() 
{
	COleVariant var;
	CString strSQL;
	int iChargeID, iPaymentID;
	COleCurrency cyAdjustment;

	try {
		// Go through all the records where WriteOffAdjustment is true, and
		// make new adjustments for those line items
		for(int i=0;i<g_aryMultiFeeAdjustmentT.GetSize();i++) {
			if(((MultiFeeAdj*)g_aryMultiFeeAdjustmentT.GetAt(i))->WriteOffAdjustment.boolVal == TRUE) {
				iChargeID = ((MultiFeeAdj*)g_aryMultiFeeAdjustmentT.GetAt(i))->ChargeID.lVal;
				cyAdjustment = ((MultiFeeAdj*)g_aryMultiFeeAdjustmentT.GetAt(i))->Adjustment.cyVal;
				cyAdjustment *= -1;
				var = ((MultiFeeAdj*)g_aryMultiFeeAdjustmentT.GetAt(i))->Description;

				// Create an adjustment for the charge
				iPaymentID = CreateAdjustment(cyAdjustment, CString(var.bstrVal));
				AutoApplyPayToBill(iPaymentID, GetActivePatientID(), "Charge", iChargeID);
			}
		}
	}
	NxCatchAll("Error in OnClickBtnOk()");		

	CDialog::OnOK();
}

void CMultiFeeAdjustmentDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

#endif