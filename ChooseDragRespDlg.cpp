// ChooseDragRespDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChooseDragRespDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CChooseDragRespDlg dialog


CChooseDragRespDlg::CChooseDragRespDlg(CWnd* pParent)
	: CNxDialog(CChooseDragRespDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseDragRespDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nTargetType = 1;
	m_strLineType = "";
	m_nLineID = 0;
	m_strRespTypeIDsToIgnore = "";
}


void CChooseDragRespDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDragRespDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_TARGET_TEXT, m_nxstaticTargetText);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CChooseDragRespDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChooseDragRespDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseDragRespDlg message handlers

BOOL CChooseDragRespDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 15:35) - PLID 29871 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Set the text to explain what's going on
		CString str;
		if(m_nTargetType == 1) {
			str.Format("Please choose an insured party source:");
		}
		else if(m_nTargetType == 2) {
			str.Format("Please choose an insured party destination:");
		}
		else if(m_nTargetType == 3) {
			str.Format("Please choose the insured party you would like to apply a payment to:");
		}
		SetDlgItemText(IDC_TARGET_TEXT, str);

		m_pResps = BindNxDataListCtrl(IDC_RESPONSIBILITY_LIST, false);

		str.Format("InsuredPartyT  "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID");

		m_pResps->PutFromClause(_bstr_t(str));

		str.Format("(RespTypeID > 2 OR RespTypeID < 1) AND InsuredPartyT.PatientID = %li ", GetActivePatientID());

		// (j.jones 2010-06-18 14:32) - PLID 39150 - we now take in an optional comma delimited list of RespTypeIDs to ignore,
		// to be appended to the existing ignored list of IDs 1 and 2
		if(!m_strRespTypeIDsToIgnore.IsEmpty()) {
			CString strWhere2;
			strWhere2.Format(" AND RespTypeID NOT IN (%s) ", m_strRespTypeIDsToIgnore);
			str += strWhere2;
		}

		m_pResps->PutWhereClause(_bstr_t(str));
		m_pResps->Requery();

		//If there is only 1 item in the list, then we'll just pick it for them and go on our merry way.
		m_pResps->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		if(m_pResps->GetRowCount() == 1) {
			m_pResps->PutCurSel(0);	//select the first item
			OnOK();		//end the dialog with that choice
		}
	}
	NxCatchAll("Error in CChooseDragRespDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseDragRespDlg::OnOK() 
{
	long nSel = m_pResps->GetCurSel();
	if(nSel == -1) {
		MsgBox("You must choose a selection before continuing.");
		return;
	}

	m_nRespTypeID = VarLong(m_pResps->GetValue(nSel, 2));
	m_nInsPartyID = VarLong(m_pResps->GetValue(nSel, 0));

	CDialog::OnOK();
}

void CChooseDragRespDlg::OnCancel() 
{
	CDialog::OnCancel();
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CChooseDragRespDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CChooseDragRespDlg)
	ON_EVENT(CChooseDragRespDlg, IDC_RESPONSIBILITY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedResponsibilityList, VTS_I2)
	ON_EVENT(CChooseDragRespDlg, IDC_RESPONSIBILITY_LIST, 3 /* DblClickCell */, OnDblClickCellResponsibilityList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CChooseDragRespDlg::OnRequeryFinishedResponsibilityList(short nFlags) 
{
	//we manually fill in the amount column using the handy billing functions
	long nInsPartyID = 0;
	long nRespID = 0;

	for(int i = 0; i < m_pResps->GetRowCount(); i++) {
		nInsPartyID = VarLong(m_pResps->GetValue(i, 0));
		nRespID = VarLong(m_pResps->GetValue(i, 2));

		COleCurrency cyAmt = COleCurrency(0, 0);
		if(m_strLineType == "Bill") {
			COleCurrency cyCharges = COleCurrency(0,0),
				cyPayments = COleCurrency(0,0),
				cyAdjustments = COleCurrency(0,0),
				cyRefunds = COleCurrency(0,0);
			
			if(nRespID != -1) {
				// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
				GetBillInsuranceTotals(m_nLineID, GetActivePatientID(), nInsPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds);
			}
			else {
				GetInactiveInsTotals(nInsPartyID, m_nLineID, -1, GetActivePatientID(), cyCharges, cyPayments);
			}
			cyAmt = cyCharges - cyPayments - cyAdjustments - cyRefunds;
		}
		else if(m_strLineType == "Charge")
			cyAmt = GetChargeInsBalance(m_nLineID, GetActivePatientID(), nInsPartyID);


		m_pResps->PutValue(i, 4, _variant_t(cyAmt));

	}
}

void CChooseDragRespDlg::OnDblClickCellResponsibilityList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex != -1)
		OnOK();	
}
