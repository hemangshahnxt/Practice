// GCRechargeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GCRechargeDlg.h"
#include "barcode.h"
#include "GlobalReportUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CGCRechargeDlg dialog

enum GiftListColumns {
	glcID = 0,
	glcGiftID,
	glcTypeName,
	glcPurchDate,
	glcExpDate,
	glcPurchBy,
	glcRecBy,
	glcValue,
	glcSpent,
	glcBalance,
};

CGCRechargeDlg::CGCRechargeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGCRechargeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGCRechargeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bIsExpired = false;
}


void CGCRechargeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGCRechargeDlg)
	DDX_Control(pDX, IDC_RECHARGE_EXPIRED, m_btnIncludeExpired);
	DDX_Control(pDX, IDC_RECHARGE_FILTER, m_btnOnlyThisPerson);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGCRechargeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CGCRechargeDlg)
	ON_BN_CLICKED(IDC_RECHARGE_FILTER, OnRechargeFilter)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_RECHARGE_EXPIRED, OnRechargeExpired)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCRechargeDlg message handlers

BOOL CGCRechargeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-05-01 17:04) - PLID 29876 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pList = BindNxDataListCtrl(this, IDC_GC_RECHARGE_LIST, GetRemoteData(), false);

		// (j.jones 2015-04-27 11:28) - PLID 65747 - moved the from clause into code,
		// and borrowed the gift certificate balance query from reports
		CString strFromClause = "GiftCertificatesT "
			"INNER JOIN (" + GetGiftCertificateValueQuery() + ") AS GCBalanceQ ON GiftCertificatesT.ID = GCBalanceQ.ID "
			"LEFT JOIN PersonT PurchT ON GiftCertificatesT.PurchasedBy = PurchT.ID "
			"LEFT JOIN PersonT RecT ON GiftCertificatesT.ReceivedBy = RecT.ID "
			"LEFT JOIN GCTypesT ON GiftCertificatesT.DefaultTypeID = GCTypesT.ServiceID "
			"LEFT JOIN ServiceT ON GCTypesT.ServiceID = ServiceT.ID ";
		m_pList->PutFromClause(_bstr_t(strFromClause));

		CheckDlgButton(IDC_RECHARGE_FILTER, TRUE);
		CheckDlgButton(IDC_RECHARGE_EXPIRED, FALSE);
		OnRechargeFilter();

		//register for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this))
				MsgBox("Error registering for barcode scans.  You may not be able to scan.");
		}


	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGCRechargeDlg::OnOK() 
{
	try {
		if(m_pList->GetCurSel() == sriNoRow) {
			MsgBox("You must select a gift certificate to recharge.");
			return;
		}

		m_nID = VarLong(m_pList->GetValue(m_pList->GetCurSel(), glcID));
		// (r.gonet 2015-07-10) - PLID 65279 - Save the certificate number as a member variable for callers.
		m_strCertNumber = VarString(m_pList->GetValue(m_pList->GetCurSel(), glcGiftID), "");

		_variant_t var = m_pList->GetValue(m_pList->GetCurSel(), glcExpDate);

		if(var.vt == VT_DATE) {		
			// warn recharging expired
			COleDateTime tExpDate = var;
			if(tExpDate < COleDateTime::GetCurrentTime()) {
				m_bIsExpired = true;

				if (IDOK != MsgBox(MB_OKCANCEL, "You are recharging an expired gift certificate. The expiration date will be adjusted."))
				{
					OnCancel();
					return;
				}
			}
		}

	} NxCatchAll("Error in OnOK()");

	//unregister for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->UnregisterForBarcodeScan(this))
			MsgBox("Error unregistering for barcode scans.");
	}

	CDialog::OnOK();
}

void CGCRechargeDlg::OnCancel() 
{
	// (j.jones 2007-02-21 12:07) - PLID 24835 - I changed the billing such that if this fails, the bill will not save,
	// and instead the user has to manually remove the charge. Thus the warning changed appropriately.
	if(MsgBox(MB_YESNO, "If you cancel, this bill will not be saved. You will need to manually remove the Recharge Gift Card charge if you wish to save the bill.\n"
		"Are you sure you wish to cancel?") != IDYES)
		return;

	//unregister for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->UnregisterForBarcodeScan(this))
			MsgBox("Error unregistering for barcode scans.");
	}

	CDialog::OnCancel();
}

void CGCRechargeDlg::OnRechargeFilter() 
{
	UpdateListFilters();
}

void CGCRechargeDlg::OnRechargeExpired()
{
	UpdateListFilters();	
}

void CGCRechargeDlg::UpdateListFilters()
{
	// a.walling 4/12/06 update datalist to show/hide personal and expired. voids are never shown
	try {
		CString str, strWhere;

		strWhere = "(Voided = 0) AND ";

		if(IsDlgButtonChecked(IDC_RECHARGE_FILTER))
		{
			str.Format("(PurchasedBy = %li OR ReceivedBy = %li) AND ", m_nPatientID, m_nPatientID);
			strWhere += str;
		}
		if(!IsDlgButtonChecked(IDC_RECHARGE_EXPIRED))
		{
			
			str.Format("(GiftCertificatesT.ExpDate IS NULL OR (GiftCertificatesT.ExpDate IS NOT NULL AND GiftCertificatesT.ExpDate >= GetDate())) AND ");
			strWhere += str;
		}

		strWhere.TrimRight(" AND ");

		m_pList->PutWhereClause(_bstr_t(strWhere));
		m_pList->Requery();
	} NxCatchAll("Error filtering the recharge list.");
}

void CGCRechargeDlg::SetPatient(long nID)
{
	m_nPatientID = nID;
}

LRESULT CGCRechargeDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
		CString strNumber = (LPCTSTR)_bstr_t((BSTR)lParam);
		
		// (j.jones 2005-11-10 11:01) - PLID 17710 - GiftCertificateIDs are now strings, albeit numeric only

		//we want to select the barcode they scanned (if we can find it)
		long nRow = m_pList->SetSelByColumn(1, _bstr_t(strNumber));
	} NxCatchAll("Error handling barcode scan.");

	return 0;
}

