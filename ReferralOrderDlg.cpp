// ReferralOrderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "ReferralOrderDlg.h"
#include "ReferralOrderEntryDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



using namespace NXDATALIST2Lib;

// CReferralOrderDlg dialog
// (z.manning 2009-05-05 09:44) - PLID 34172 - Created

IMPLEMENT_DYNAMIC(CReferralOrderDlg, CNxDialog)

CReferralOrderDlg::CReferralOrderDlg(const long nPatientID, CWnd* pParent)
	: CNxDialog(CReferralOrderDlg::IDD, pParent)
{
	m_nPatientID = nPatientID;
}

CReferralOrderDlg::~CReferralOrderDlg()
{
}

void CReferralOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_NEW_REFERRAL_ORDER, m_btnNewReferralOrder);
	DDX_Control(pDX, IDC_EDIT_REFERRAL_ORDER, m_btnEditReferralOrder);
	DDX_Control(pDX, IDC_DELETE_REFERRAL_ORDER, m_btnDeleteReferralOrder);
	DDX_Control(pDX, IDC_REFERRAL_ORDER_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDC_REFERRAL_ORDER_HEADER, m_nxstaticHeader);
	DDX_Control(pDX, IDC_PRINT_REF_ORDER, m_btnPrint);
	DDX_Control(pDX, IDC_PREVIEW_REF_ORDER, m_btnPreview);
}


BEGIN_MESSAGE_MAP(CReferralOrderDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NEW_REFERRAL_ORDER, &CReferralOrderDlg::OnBnClickedNewReferralOrder)
	ON_BN_CLICKED(IDC_EDIT_REFERRAL_ORDER, &CReferralOrderDlg::OnBnClickedEditReferralOrder)
	ON_BN_CLICKED(IDC_DELETE_REFERRAL_ORDER, &CReferralOrderDlg::OnBnClickedDeleteReferralOrder)
	ON_BN_CLICKED(IDC_PREVIEW_REF_ORDER, &CReferralOrderDlg::OnBnClickedPreviewRefOrder)
	ON_BN_CLICKED(IDC_PRINT_REF_ORDER, &CReferralOrderDlg::OnBnClickedPrintRefOrder)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CReferralOrderDlg, CNxDialog)
	ON_EVENT(CReferralOrderDlg, IDC_REFERRAL_ORDER_LIST, 3, CReferralOrderDlg::DblClickCellReferralOrderList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CReferralOrderDlg, IDC_REFERRAL_ORDER_LIST, 2, CReferralOrderDlg::SelChangedReferralOrderList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CReferralOrderDlg, IDC_REFERRAL_ORDER_LIST, 18, CReferralOrderDlg::RequeryFinishedReferralOrderList, VTS_I2)
END_EVENTSINK_MAP()


// CReferralOrderDlg message handlers

BOOL CReferralOrderDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pdlOrders = BindNxDataList2Ctrl(IDC_REFERRAL_ORDER_LIST, false);
		m_pdlOrders->PutWhereClause(_bstr_t(FormatString("ReferralOrdersT.PatientID = %li", m_nPatientID)));
		m_pdlOrders->Requery();

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnNewReferralOrder.AutoSet(NXB_NEW);
		m_btnEditReferralOrder.AutoSet(NXB_MODIFY);
		m_btnDeleteReferralOrder.AutoSet(NXB_DELETE);
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPreview.AutoSet(NXB_PRINT_PREV);

		m_nxstaticHeader.SetWindowText(FormatString("Referral orders for patient: %s - %li"
			, GetExistingPatientName(m_nPatientID), GetExistingPatientUserDefinedID(m_nPatientID)));

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CReferralOrderDlg::OnBnClickedNewReferralOrder()
{
	try
	{
		// (z.manning 2009-05-08 11:57) - PLID 34172 - Permissions
		if(!CheckCurrentUserPermissions(bioReferralOrders, sptCreate)) {
			return;
		}

		// (z.manning 2009-05-05 09:56) - PLID 28529
		CReferralOrderEntryDlg dlg(m_nPatientID, this);
		if(dlg.DoModal() == IDOK) {
			// (z.manning 2009-05-12 15:32) - PLID 34219 - Check and see if we need to close this dialog
			// (they may have previewed a report).
			if(dlg.m_bCloseParent) {
				EndDialog(IDCANCEL);
			}
			else {
				m_pdlOrders->Requery();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderDlg::OnBnClickedEditReferralOrder()
{
	try
	{
		// (z.manning 2009-05-08 11:57) - PLID 34172 - Permissions
		if(!CheckCurrentUserPermissions(bioReferralOrders, sptWrite)) {
			return;
		}

		IRowSettingsPtr pRow = m_pdlOrders->GetCurSel();
		if(pRow == NULL) {
			MessageBox("You must select a referral order first.");
			return;
		}

		ReferralOrder reforder;
		reforder.nID = VarLong(pRow->GetValue(rocID));
		reforder.dtDate = VarDateTime(pRow->GetValue(rocDate));
		reforder.nReferToID = VarLong(pRow->GetValue(rocReferToID));
		reforder.varReferredByID = pRow->GetValue(rocReferredByID);
		reforder.strReason = VarString(pRow->GetValue(rocReason), "");

		// (z.manning 2009-05-06 12:55) - PLID 28530 - Needed for auditing
		reforder.nPatientID = m_nPatientID;
		reforder.strReferToName = VarString(pRow->GetValue(rocReferToName), "");
		reforder.strReferredByName = VarString(pRow->GetValue(rocReferredByName), "");

		CReferralOrderEntryDlg dlg(m_nPatientID, this);
		if(dlg.EditExistingReferralOrder(reforder) == IDOK) {
			// (z.manning 2009-05-12 15:32) - PLID 34219 - Check and see if we need to close this dialog
			// (they may have previewed a report).
			if(dlg.m_bCloseParent) {
				EndDialog(IDCANCEL);
			}
			else {
				m_pdlOrders->Requery();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderDlg::OnBnClickedDeleteReferralOrder()
{
	try
	{
		// (z.manning 2009-05-08 11:57) - PLID 34172 - Permissions
		if(!CheckCurrentUserPermissions(bioReferralOrders, sptDelete)) {
			return;
		}

		IRowSettingsPtr pRow = m_pdlOrders->GetCurSel();
		if(pRow == NULL) {
			MessageBox("You must select a referral order first.");
			return;
		}

		int nResult = MessageBox("Are you sure you want to delete this referral order?", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}

		long nReferralOrderID = VarLong(pRow->GetValue(rocID));

		ExecuteParamSql("DELETE FROM ReferralOrdersT WHERE ID = {INT}", nReferralOrderID);

		// (z.manning 2009-05-06 15:00) - PLID 28530 - Audit the deletion
		CString strReferToName = VarString(pRow->GetValue(rocReferToName), "");
		COleDateTime dtDate = VarDateTime(pRow->GetValue(rocDate));
		CString strOld = FormatString("Referral to %s on %s", strReferToName, FormatDateTimeForInterface(dtDate,0,dtoDate));
		AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiReferralOrderDeleted, nReferralOrderID
			, strOld, "<Deleted>", aepHigh, aetDeleted);

		m_pdlOrders->RemoveRow(pRow);

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderDlg::DblClickCellReferralOrderList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		if(lpRow != NULL) {
			OnBnClickedEditReferralOrder();
		}

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderDlg::SelChangedReferralOrderList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		BOOL bEnable;
		if(lpNewSel == NULL) {
			bEnable = FALSE;
		}
		else {
			bEnable = TRUE;
		}

		m_btnEditReferralOrder.EnableWindow(bEnable);
		m_btnDeleteReferralOrder.EnableWindow(bEnable);
		m_btnPrint.EnableWindow(bEnable);
		m_btnPreview.EnableWindow(bEnable);

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderDlg::RequeryFinishedReferralOrderList(short nFlags)
{
	try
	{
		SelChangedReferralOrderList(NULL, m_pdlOrders->GetCurSel());

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-12 15:29) - PLID 34219
void CReferralOrderDlg::OnBnClickedPreviewRefOrder()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlOrders->GetCurSel();
		if(pRow == NULL) {
			MessageBox("You must select a referral order first.");
			return;
		}

		long nReferralOrderID = VarLong(pRow->GetValue(rocID));
		PrintReferralOrder(nReferralOrderID, TRUE);
		EndDialog(IDCANCEL);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-12 15:29) - PLID 34219
void CReferralOrderDlg::OnBnClickedPrintRefOrder()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlOrders->GetCurSel();
		if(pRow == NULL) {
			MessageBox("You must select a referral order first.");
			return;
		}

		long nReferralOrderID = VarLong(pRow->GetValue(rocID));
		PrintReferralOrder(nReferralOrderID, FALSE);

	}NxCatchAll(__FUNCTION__);
}
