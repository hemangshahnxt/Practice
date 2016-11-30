// AppliedSuperbillsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppliedSuperbillsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;

////////////////
//Column defines
#define COL_CHARGEID		0
#define COL_CHARGECODE		1
#define COL_CHARGEDATE		2
#define COL_CHARGEDESC		3
#define COL_CHARGEAMT		4
#define COL_SUPERBILLID		5
#define COL_PRINTDATE		6

/////////////////////////////////////////////////////////////////////////////
// CAppliedSuperbillsDlg dialog


CAppliedSuperbillsDlg::CAppliedSuperbillsDlg(CWnd* pParent)
	: CNxDialog(CAppliedSuperbillsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppliedSuperbillsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nCurrentBillID = -1;
}


void CAppliedSuperbillsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppliedSuperbillsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_BILLID_TEXT, m_nxstaticBillidText);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAppliedSuperbillsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAppliedSuperbillsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppliedSuperbillsDlg message handlers

BOOL CAppliedSuperbillsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 15:13) - PLID 29871 - NxIconify the close button
		m_btnOK.AutoSet(NXB_CLOSE);

		m_listApplies = BindNxDataListCtrl(IDC_APPLY_LIST, false);

		//filter the list on only this patient and this bill if not -1 (and non-quote and not deleted haha!)
		CString sql, strText;
		if(m_nCurrentBillID == -1) {
			sql.Format("LineItemT.PatientID = %li AND LineItemT.Type = 10 AND LineItemT.Deleted = 0", m_nCurrentPatientID);
			//and fix the text
			strText.Format("Showing all Charges");
			SetDlgItemText(IDC_BILLID_TEXT, strText);
		}
		else {
			sql.Format("LineItemT.PatientID = %li AND ChargesT.BillID = %li AND LineItemT.Type = 10 AND LineItemT.Deleted = 0", m_nCurrentPatientID, m_nCurrentBillID);
			strText.Format("Showing all Charges for Bill Number:  %li", m_nCurrentBillID);
			SetDlgItemText(IDC_BILLID_TEXT, strText);
		}
		m_listApplies->PutWhereClause(_bstr_t(sql));

		//fill in the combo clause
		//if an item is already applied, we must show it, even if void.  otherwise, leave it out
		sql.Format("SELECT -1 AS ID, '<None Selected>' AS Text UNION SELECT SavedID AS ID, convert(nvarchar, SavedID) AS Text FROM PrintedSuperbillsT "
				"WHERE PatientID = %li AND (PrintedSuperbillsT.Void = 0 OR PrintedSuperbillsT.SavedID IN "
				"(SELECT SuperbillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE SuperbillID IS NOT NULL AND Deleted = 0 AND Type = 10))", 
			m_nCurrentPatientID);

		m_listApplies->GetColumn(COL_SUPERBILLID)->PutComboSource(_bstr_t(sql));

		//requery
		m_listApplies->Requery();
	}
	NxCatchAll("Error in CAppliedSuperbillsDlg::OnInitDialog");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAppliedSuperbillsDlg::OnOK() 
{
	CDialog::OnOK();
}

void CAppliedSuperbillsDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CAppliedSuperbillsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAppliedSuperbillsDlg)
	ON_EVENT(CAppliedSuperbillsDlg, IDC_APPLY_LIST, 10 /* EditingFinished */, OnEditingFinishedApplyList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAppliedSuperbillsDlg::OnEditingFinishedApplyList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow < 0)
		return;

	switch(nCol) {
	case COL_SUPERBILLID:
		try {
			//check for bad data (no selection in the combo most likely)
			if(varNewValue.vt != VT_I4)
				return;

			long nID = VarLong(varNewValue);
			long nChargeID = VarLong(m_listApplies->GetValue(nRow, COL_CHARGEID));
			
			//insert this ID into the charges table if not nothing
			if(nID != -1)
				ExecuteSql("UPDATE ChargesT SET SuperbillID = %li WHERE ID = %li", nID, nChargeID);
			else	//remove what was previously in that field
				ExecuteSql("UPDATE ChargesT SET SuperbillID = NULL WHERE ID = %li", nChargeID);

			//now update the print date
			if(nID == -1) {
				//removal, empty the field
				_variant_t var;
				var.vt = VT_NULL;
				m_listApplies->PutValue(nRow, COL_PRINTDATE, var);
			}
			else {
				//adding a field, we need to put the date in
				_RecordsetPtr rs = CreateRecordset("SELECT convert(nvarchar, PrintedOn, 1) AS PrintedOn FROM PrintedSuperbillsT WHERE SavedID = %li", nID);
				m_listApplies->PutValue(nRow, COL_PRINTDATE, rs->Fields->Item["PrintedOn"]->Value);
			}

		} NxCatchAll("Error updating superbill ID");
		break;
	default:
		break;
	}
}
