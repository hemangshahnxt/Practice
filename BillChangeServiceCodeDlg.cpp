// BillChangeServiceCodeDlg.cpp : implementation file
// (s.dhole 2011-05-31 15:04) - PLID 46357 Added Service code selection Dialog

#include "stdafx.h"
#include "Practice.h"
#include "BillChangeServiceCodeDlg.h"
#include "billingRc.h"

using namespace NXDATALIST2Lib;
// CBillChangeServiceCodeDlg dialog

IMPLEMENT_DYNAMIC(CBillChangeServiceCodeDlg, CNxDialog)

enum ServicCodeColumns {
ServiceColumnTOS=0,
ServiceColumnCODE,
ServiceColumnSUBCODE,
ServiceColumnNAME,
ServiceColumnPRICE,
ServiceColumnCATEGORY,
ServiceColumnServiceID,
ServiceColumnTAXABLE1,
ServiceColumnTAXABLE2,
ServiceColumnBARCODE,
ServiceColumnIS_AMA,
ServiceColumnANESTHESIA,
ServiceColumnUSE_ANESTH_BILLING,
ServiceColumnFACILITY_FEE,
ServiceColumnUSE_FACILITY_BILLING,
ServiceColumnAsSISTINGCODE,
ServiceColumnSERVICECODE,
};

CBillChangeServiceCodeDlg::CBillChangeServiceCodeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillChangeServiceCodeDlg::IDD, pParent)
{

}

CBillChangeServiceCodeDlg::~CBillChangeServiceCodeDlg()
{
}

void CBillChangeServiceCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbtnOk);
	DDX_Control(pDX, IDCANCEL, m_nxbtnCancel);
}


BEGIN_MESSAGE_MAP(CBillChangeServiceCodeDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CBillChangeServiceCodeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBillChangeServiceCodeDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CBillChangeServiceCodeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBillChangeServiceCodeDlg)
	
	ON_EVENT(CBillChangeServiceCodeDlg, IDC_BILL_CHANGE_SERVICE_CODE_LIST, 1 /* SelChanging */, CBillChangeServiceCodeDlg::OnSelChangingCptCodeCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillChangeServiceCodeDlg, IDC_BILL_CHANGE_SERVICE_CODE_LIST, 16 /* TrySetSelFinished */, CBillChangeServiceCodeDlg::OnSelChosenCptCodeCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()



BOOL CBillChangeServiceCodeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
		
		m_nxbtnOk.AutoSet(NXB_OK);
		m_nxbtnCancel.AutoSet(NXB_CANCEL);
		m_CPTListCombo = BindNxDataList2Ctrl(IDC_BILL_CHANGE_SERVICE_CODE_LIST, true);
		IRowSettingsPtr pDesiredTopRow = m_CPTListCombo->FindByColumn(ServiceColumnServiceID, m_nServiceCodeID, NULL, VARIANT_TRUE );   // Cast as long
		if (pDesiredTopRow==NULL) 
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		long iCol = GetRemotePropertyInt("CPTSearch", 1, 0, GetCurrentUserName(), TRUE);
		m_CPTListCombo->TextSearchCol = (short)iCol;
		m_CPTListCombo->GetColumn((short)iCol)->PutSortPriority(0);
		m_CPTListCombo->GetColumn((short)iCol)->PutSortAscending(TRUE);
		m_CPTListCombo->Sort();
	}NxCatchAll("Error in BillChangeServiceCodeDlg::OnInitDialog");

	return TRUE;
}
void CBillChangeServiceCodeDlg::OnSelChangingCptCodeCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
	
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll("Error in CBillChangeServiceCodeDlg::OnSelChangingCptCodeCombo()")
}



void CBillChangeServiceCodeDlg::OnSelChosenCptCodeCombo(LPDISPATCH lpRow)
{
try {
	IRowSettingsPtr pRow(lpRow);
	if (pRow!=NULL){
		m_nServiceCodeID=VarLong(pRow->GetValue(ServiceColumnServiceID),-1) ;	
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else {
		m_nServiceCodeID=-1;
	}
	}NxCatchAll("Error in CBillChangeServiceCodeDlg::OnSelChosenCptCodeCombo()")
}


void CBillChangeServiceCodeDlg::OnBnClickedOk()
{
try {
	IRowSettingsPtr pRow = m_CPTListCombo->GetCurSel();
	if (pRow!=NULL){
		m_nServiceCodeID=VarLong(pRow->GetValue(ServiceColumnServiceID),-1) ;	
	}
	else {
		m_nServiceCodeID=-1;
	}
		CNxDialog::OnOK();
	}NxCatchAll("CBillChangeServiceCodeDlg::OnBnClickedOk");
}

void CBillChangeServiceCodeDlg::OnBnClickedCancel()
{
try{ 
	CNxDialog::OnCancel();	
	}NxCatchAll("Error in CBillChangeServiceCodeDlg::OnBnClickedCancel");
}

// CBillChangeServiceCodeDlg message handlers
