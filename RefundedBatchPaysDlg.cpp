// RefundedBatchPaysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RefundedBatchPaysDlg.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CRefundedBatchPaysDlg dialog

enum RefundedBatchPayColumns {

	rbpcID = 0,
	rbpcAppliedPayID,
	rbpcInsCoName,
	rbpcPayDesc,
	rbpcAmount,
	rbpcPayDate,
	rbpcApplyType,
	rbpcApplyAmt,
	rbpcDateApplied,
	rbpcUser,
};

CRefundedBatchPaysDlg::CRefundedBatchPaysDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRefundedBatchPaysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRefundedBatchPaysDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRefundedBatchPaysDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRefundedBatchPaysDlg)
	DDX_Control(pDX, IDC_BTN_UNAPPLY_REFUNDED_BATCH_PAY, m_btnUnapplyItem);
	DDX_Control(pDX, IDOK, m_btnClose);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRefundedBatchPaysDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRefundedBatchPaysDlg)
	ON_BN_CLICKED(IDC_BTN_UNAPPLY_REFUNDED_BATCH_PAY, OnBtnUnapplyRefundedBatchPay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRefundedBatchPaysDlg message handlers

BOOL CRefundedBatchPaysDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	

	// (j.jones 2008-05-08 10:44) - PLID 29953 - added nxiconbuttons for modernization
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnUnapplyItem.AutoSet(NXB_MODIFY);
	
	m_RefundedPaysList = BindNxDataListCtrl(this,IDC_REFUNDED_BATCH_PAYMENTS_LIST,GetRemoteData(),true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRefundedBatchPaysDlg::OnBtnUnapplyRefundedBatchPay() 
{
	
	try {
	
		// (s.tullis 2014-06-24 09:34) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
		if (!GetCurrentUserPermissions(bioBatchPayment, sptWrite))
			return;

		if(m_RefundedPaysList->CurSel == -1) {
			AfxMessageBox("Please select a refund or adjustment to unapply.");
			return;
		}


		long nRefundID = VarLong(m_RefundedPaysList->GetValue(m_RefundedPaysList->GetCurSel(), rbpcAppliedPayID));

		// (j.jones 2008-04-30 14:58) - PLID 28358 - added permissions
		// Check permissions, but also check CanEdit.
		// In most cases CanEdit is not needed, so first we must
		// silently see if they have permission, and if they do, then move ahead normally. But if
		// they don't, or they need a password, then check CanEdit prior to the permission check that
		// would stop or password-prompt the user.
		CString strType = VarString(m_RefundedPaysList->GetValue(m_RefundedPaysList->GetCurSel(), rbpcApplyType), "");
		if(strType == "Adjustment") {
			// (c.haag 2009-03-10 10:50) - PLID 32433 - We now use CanChangeHistoricFinancial
			if (!CanChangeHistoricFinancial("BatchAdjustment", nRefundID, bioAdjustment, sptDelete)) {
				return;
			}
			/*
			if(!(GetCurrentUserPermissions(bioAdjustment) & sptDelete)
				&& !CanEdit("BatchPayment", nRefundID)
				&& !CheckCurrentUserPermissions(bioAdjustment, sptDelete)) {
				return;
			}*/
		}
		else {			
			// (c.haag 2009-03-10 10:50) - PLID 32433 - We now use CanChangeHistoricFinancial
			if (!CanChangeHistoricFinancial("BatchRefund", nRefundID, bioRefund, sptDelete)) {
				return;
			}
			/*
			if(!(GetCurrentUserPermissions(bioRefund) & sptDelete)
				&& !CanEdit("BatchPayment", nRefundID)
				&& !CheckCurrentUserPermissions(bioRefund, sptDelete)) {
				return;
			}*/
		}

		if(IDNO == MessageBox("Unapplying this item will delete the refund / adjustment and return the balance to the batch payment.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		// (j.jones 2008-05-12 14:38) - PLID 30004 - properly filled in the DeleteDate and DeletedBy fields
		ExecuteParamSql("UPDATE BatchPaymentsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {INT} WHERE ID = {INT}", GetCurrentUserID(), nRefundID);

		m_RefundedPaysList->RemoveRow(m_RefundedPaysList->GetCurSel());

		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID!=-1) {
			AuditEvent(-1, "",AuditID,aeiBatchPayRefundDeleted,nRefundID,"","<Deleted>",aepHigh);
		}

	}NxCatchAll("Error unapplying item.");
	
}

BEGIN_EVENTSINK_MAP(CRefundedBatchPaysDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRefundedBatchPaysDlg)
	ON_EVENT(CRefundedBatchPaysDlg, IDC_REFUNDED_BATCH_PAYMENTS_LIST, 3 /* DblClickCell */, OnDblClickCellRefundedBatchPaymentsList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRefundedBatchPaysDlg::OnDblClickCellRefundedBatchPaymentsList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_RefundedPaysList->CurSel = nRowIndex;
	OnBtnUnapplyRefundedBatchPay();	
}
