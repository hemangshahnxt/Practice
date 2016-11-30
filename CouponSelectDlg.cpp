// CouponSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingRc.h"
#include "CouponSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCouponSelectDlg dialog

// (j.gruber 2007-04-04 14:22) - PLID 9796 - class created for coupons
CCouponSelectDlg::CCouponSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCouponSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCouponSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCouponSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCouponSelectDlg)
	DDX_Control(pDX, IDC_SHOW_EXPIRED, m_btnShowExpired);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCouponSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCouponSelectDlg)
	ON_BN_CLICKED(IDC_SHOW_EXPIRED, OnShowExpired)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCouponSelectDlg message handlers

BOOL CCouponSelectDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-05-01 15:41) - PLID 29871 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CheckDlgButton(IDC_SHOW_EXPIRED_COUPONS, 0);

		m_pCouponList = BindNxDataList2Ctrl(this, IDC_COUPON_SELECT_LIST, GetRemoteData(), false);

		m_pCouponList->WhereClause = "CONVERT(datetime, Convert(nvarchar, StartDate, 101)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 101)) AND EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 101))";

		m_pCouponList->Requery();

		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pCouponList->GetColumn(2);
		pCol->StoredWidth = 0;		
	}
	NxCatchAll("Error in CCouponSelectDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCouponSelectDlg::OnShowExpired() 
{
	try {

		//check to see if it is already checked
		if (IsDlgButtonChecked(IDC_SHOW_EXPIRED)) {

			//they want to show expired items
			m_pCouponList->WhereClause = "CONVERT(datetime, Convert(nvarchar, StartDate, 101)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 101))";

			NXDATALIST2Lib::IColumnSettingsPtr pCol;
			pCol = m_pCouponList->GetColumn(2);

			//pCol->ColumnStyle = NXDATALIST2Lib::csWidthPercent;
			pCol->StoredWidth = 20;
			
		}
		else {

			//they are already showing expired, so we have to unshow them
			m_pCouponList->WhereClause = "CONVERT(datetime, Convert(nvarchar, StartDate, 101)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 101)) AND EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 101))";

			NXDATALIST2Lib::IColumnSettingsPtr pCol;
			pCol = m_pCouponList->GetColumn(2);

			//pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
			pCol->StoredWidth = 0;			
		}

		m_pCouponList->Requery();

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCouponList->GetFirstRow();

	}NxCatchAll("Error in CCouponSetupDlg::OnShowExpiredCoupons");
	
	
}

void CCouponSelectDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CCouponSelectDlg::OnOK() 
{
	//set the value for the coupon ID
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pCouponList->CurSel;

	if (pRow) {

		//check to see if they are choosing an espired one
		long nExpired = VarLong(pRow->GetValue(2));
		if (nExpired != 0) {
			if (MsgBox(MB_YESNO, "This coupon is expired.  Are you sure you wish to do this?") == IDYES) {
				//let 'em
				m_nCouponID = VarLong(pRow->GetValue(0));
				m_strCouponName = VarString(pRow->GetValue(1));
			}
			else {
				return;
			}
		}
		else {
			m_nCouponID = VarLong(pRow->GetValue(0));
			m_strCouponName = VarString(pRow->GetValue(1));
		}
	}
	else {
		MsgBox("Please select a coupon from the list or click cancel.");
		return;
	}
	
	CDialog::OnOK();
}
