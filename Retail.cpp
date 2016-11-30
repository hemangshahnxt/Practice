// Retail.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "Retail.h"
#include "Barcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.wetta 2007-03-29 13:27) - PLID 25407 - Created Retail tab

/////////////////////////////////////////////////////////////////////////////
// CRetail dialog


CRetail::CRetail(CWnd* pParent)
	: CNxDialog(CRetail::IDD, pParent)
	, m_CommissionSetupWnd(this)
	, m_CouponsSetupDlg(this)
	, m_SalesSetupDlg(this)
	, m_SuggestedSalesDlg(this)
	, m_RewardPointsSetupDlg(this)
{
	//{{AFX_DATA_INIT(CRetail)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (a.walling 2007-10-26 15:22) - PLID 27886 - Added link to the NexSpa section of help
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Administrator_Module/NexSpa/The_NexSpa_Tab.htm";
}


void CRetail::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRetail)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_REWARDS_PLACEHOLDER, m_nxstaticRewardsPlaceholder);
	DDX_Control(pDX, IDC_COMMISSION_PLACEHOLDER, m_nxstaticCommissionPlaceholder);
	DDX_Control(pDX, IDC_COUPON_PLACE_HOLDER, m_nxstaticCouponPlaceHolder);
	DDX_Control(pDX, IDC_SALES_PLACEHOLDER, m_nxstaticSalesPlaceholder);
	DDX_Control(pDX, IDC_SUGGESTED_SALES_PLACEHOLDER, m_nxstaticSuggestedSalesPlaceholder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRetail, CNxDialog)
	//{{AFX_MSG_MAP(CRetail)
	ON_WM_SIZE()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRetail message handlers

BOOL CRetail::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//TES 3/31/2008 - PLID 29479 - There has long been a bug in MFC where it does not initialize the control
		// container for windows which have child windows with controls.  In VS 2008, however, this bug became
		// exposed by some other MFC changes they made, so now creating such windows will crash the program.  This
		// line of code resolves that crash.  See http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=2601961&SiteID=1
		InitControlContainer();

		/////////////////////
		// Initiate all of the dialogs in the tab

		// COMMISSION
		// Create the commission dialog
		m_CommissionSetupWnd.m_bTabMode = TRUE;
		m_CommissionSetupWnd.Create(IDD_COMMISSION_SETUP_WND, this);

		// Get the size and position of the commission's placeholder
		CRect rcCommissionPlaceholder;
		GetDlgItem(IDC_COMMISSION_PLACEHOLDER)->GetWindowRect(&rcCommissionPlaceholder);
		ScreenToClient(&rcCommissionPlaceholder);

		// Move the commission dialog to the correct position
		m_CommissionSetupWnd.SetWindowPos(NULL, rcCommissionPlaceholder.left, rcCommissionPlaceholder.top, 
										rcCommissionPlaceholder.Width(), rcCommissionPlaceholder.Height(), SWP_SHOWWINDOW|SWP_NOZORDER);

		// COUPONS
		// (j.gruber 2007-04-02 15:44) - PLID 25164 - coupons dialog
		// Create the coupons dialog
		m_CouponsSetupDlg.Create(IDD_COUPON_SETUP, this);

		// Get the size and position of the discount's placeholder
		CRect rcCouponsPlaceholder;
		GetDlgItem(IDC_COUPON_PLACE_HOLDER)->GetWindowRect(&rcCouponsPlaceholder);
		ScreenToClient(&rcCouponsPlaceholder);

		// Move the discounts dialog to the correct position
		m_CouponsSetupDlg.SetWindowPos(NULL, rcCouponsPlaceholder.left, rcCouponsPlaceholder.top, 
										rcCouponsPlaceholder.Width(), rcCouponsPlaceholder.Height(), SWP_SHOWWINDOW|SWP_NOZORDER);
		GetDlgItem(IDC_COUPON_PLACE_HOLDER)->ShowWindow(SW_HIDE);

		// SALES
		// (a.wetta 2007-04-26 16:36) - PLID 15998 - Create the discounts dialog
		m_SalesSetupDlg.Create(IDD_SALES_SETUP_DLG, this);

		// Get the size and position of the discount's placeholder
		CRect rcSalesPlaceholder;
		GetDlgItem(IDC_SALES_PLACEHOLDER)->GetWindowRect(&rcSalesPlaceholder);
		ScreenToClient(&rcSalesPlaceholder);

		// Move the discounts dialog to the correct position
		m_SalesSetupDlg.SetWindowPos(NULL, rcSalesPlaceholder.left, rcSalesPlaceholder.top, 
										rcSalesPlaceholder.Width(), rcSalesPlaceholder.Height(), SWP_SHOWWINDOW|SWP_NOZORDER);

		// SUGGESTED SALES
		// (a.wetta 2007-05-15 17:53) - PLID 25960 - Added the suggested sales dialog
		m_SuggestedSalesDlg.Create(IDD_SUGGESTED_SALES_DLG, this);

		// Get the size and position of the discount's placeholder
		CRect rcSuggestedSalesPlaceholder;
		GetDlgItem(IDC_SUGGESTED_SALES_PLACEHOLDER)->GetWindowRect(&rcSuggestedSalesPlaceholder);
		ScreenToClient(&rcSuggestedSalesPlaceholder);

		// Move the discounts dialog to the correct position
		m_SuggestedSalesDlg.SetWindowPos(NULL, rcSuggestedSalesPlaceholder.left, rcSuggestedSalesPlaceholder.top, 
										rcSuggestedSalesPlaceholder.Width(), rcSuggestedSalesPlaceholder.Height(), SWP_SHOWWINDOW|SWP_NOZORDER);

		// REWARD POINTS
		// (a.wetta 2007-05-15 17:54) - PLID 25960 - Added the reward points dialog
		m_RewardPointsSetupDlg.Create(IDD_REWARD_POINTS_SETUP_DLG, this);

		// Get the size and position of the discount's placeholder
		CRect rcRewardPointsSetupPlaceholder;
		GetDlgItem(IDC_REWARDS_PLACEHOLDER)->GetWindowRect(&rcRewardPointsSetupPlaceholder);
		ScreenToClient(&rcRewardPointsSetupPlaceholder);

		// Move the discounts dialog to the correct position
		m_RewardPointsSetupDlg.SetWindowPos(NULL, rcRewardPointsSetupPlaceholder.left, rcRewardPointsSetupPlaceholder.top, 
										rcRewardPointsSetupPlaceholder.Width(), rcRewardPointsSetupPlaceholder.Height(), SWP_SHOWWINDOW|SWP_NOZORDER);

		/////////////////////

		SetControlPositions();

	}NxCatchAll("Error in CRetail::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRetail::OnSize(UINT nType, int cx, int cy) 
{
	try {
		CNxDialog::OnSize(nType, cx, cy);

		SetControlPositions();

		// Be sure to move all of the dialogs on the tab
		if (m_CommissionSetupWnd)
		{	
			CRect rcCommissionPlaceholder;
			GetDlgItem(IDC_COMMISSION_PLACEHOLDER)->GetWindowRect(&rcCommissionPlaceholder);
			ScreenToClient(&rcCommissionPlaceholder);
			m_CommissionSetupWnd.MoveWindow(rcCommissionPlaceholder);
		}

		if (m_CouponsSetupDlg)
		{	
			CRect rcCouponPlaceholder;
			GetDlgItem(IDC_COUPON_PLACE_HOLDER)->GetWindowRect(&rcCouponPlaceholder);
			ScreenToClient(&rcCouponPlaceholder);
			m_CouponsSetupDlg.MoveWindow(rcCouponPlaceholder);
		}

		// (a.wetta 2007-04-26 16:39) - PLID 15998 - Size the sales setup dialog
		if (m_SalesSetupDlg)
		{	
			CRect rcSalesPlaceholder;
			GetDlgItem(IDC_SALES_PLACEHOLDER)->GetWindowRect(&rcSalesPlaceholder);
			ScreenToClient(&rcSalesPlaceholder);
			m_SalesSetupDlg.MoveWindow(rcSalesPlaceholder);
		}

		// (a.wetta 2007-05-15 17:53) - PLID 25960 - Size the suggested sales dialog
		if (m_SuggestedSalesDlg)
		{	
			CRect rcSuggestedSalesPlaceholder;
			GetDlgItem(IDC_SUGGESTED_SALES_PLACEHOLDER)->GetWindowRect(&rcSuggestedSalesPlaceholder);
			ScreenToClient(&rcSuggestedSalesPlaceholder);
			m_SuggestedSalesDlg.MoveWindow(rcSuggestedSalesPlaceholder);
		}

		// (a.wetta 2007-05-15 17:54) - PLID 25960 - Size the reward points dialog
		if (m_RewardPointsSetupDlg)
		{	
			CRect rcRewardPointsSetupPlaceholder;
			GetDlgItem(IDC_REWARDS_PLACEHOLDER)->GetWindowRect(&rcRewardPointsSetupPlaceholder);
			ScreenToClient(&rcRewardPointsSetupPlaceholder);
			m_RewardPointsSetupDlg.MoveWindow(rcRewardPointsSetupPlaceholder);
		}

	}NxCatchAll("Error in CRetail::OnSize");
}

void CRetail::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{ 
	try {
		// (a.wetta 2007-04-18 09:29) - PLID 25407 - Call each dialogs UpdateView individually and explicitly
		m_CommissionSetupWnd.UpdateView(bForceRefresh);
		m_CouponsSetupDlg.UpdateView(bForceRefresh);
		// (a.wetta 2007-04-26 16:40) - PLID 15998 - Update sales setup dialog
		m_SalesSetupDlg.UpdateView(bForceRefresh);
		// (a.wetta 2007-05-15 17:58) - PLID 25960 - Added suggested sales and reward points dialogs
		m_SuggestedSalesDlg.UpdateView(bForceRefresh);
		// (a.walling 2007-05-17 09:11) - PLID 20838 - Update the reward points dialog
		m_RewardPointsSetupDlg.UpdateView(bForceRefresh);

	}NxCatchAll("Error in CRetail::UpdateView");
}

LRESULT CRetail::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-05-09 13:00) - PLID 25171 - Pass our barcode message to the coupon dialog
	ASSERT(m_CouponsSetupDlg.GetSafeHwnd());

	if (m_CouponsSetupDlg.GetSafeHwnd()) {
		m_CouponsSetupDlg.SendMessage(WM_BARCODE_SCAN, wParam, lParam);
	}

	return 0;
}
