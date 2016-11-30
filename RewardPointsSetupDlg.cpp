// RewardPointsSetupDlg.cpp : implementation file
//

// (a.walling 2007-05-17 09:13) - PLID 20838 - Dialog to set up reward points.

#include "stdafx.h"
#include "administratorRc.h"
#include "RewardPointsSetupDlg.h"
#include "AdvRewardPointsSetup.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRewardPointsSetupDlg dialog


CRewardPointsSetupDlg::CRewardPointsSetupDlg(CWnd* pParent)
	: CNxDialog(CRewardPointsSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRewardPointsSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_arControlIDs.Add(IDC_POINTS_PER_BILL);
	m_arControlIDs.Add(IDC_EDIT_POINTS_PER_BILL);
	m_arControlIDs.Add(IDC_POINTS_PER_DOLLAR);
	m_arControlIDs.Add(IDC_EDIT_POINTS_PER_DOLLAR);
	m_arControlIDs.Add(IDC_BILL_POINTS_FOR_EVERY);
	m_arControlIDs.Add(IDC_EDIT_BILL_POINTS_FOR_EVERY);
	m_arControlIDs.Add(IDC_BILL_POINTS_BILL_POINTS);
	m_arControlIDs.Add(IDC_BILL_POINTS_ACCUMULATE);
	m_arControlIDs.Add(IDC_EDIT_BILL_POINTS_ACCUMULATE);
	m_arControlIDs.Add(IDC_BILL_POINTS_POINTS);
	m_arControlIDs.Add(IDC_DOLLARS_FOR_EVERY);
	m_arControlIDs.Add(IDC_EDIT_DOLLARS_FOR_EVERY);
	m_arControlIDs.Add(IDC_DOLLARS_DOLLARS);
	m_arControlIDs.Add(IDC_DOLLARS_ACCUMULATE);
	m_arControlIDs.Add(IDC_EDIT_DOLLARS_ACCUMULATE);
	m_arControlIDs.Add(IDC_DOLLARS_POINTS);
	m_arControlIDs.Add(IDC_BTN_SETUP_POINTS_DETAIL);
	m_arControlIDs.Add(IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS);
	m_arControlIDs.Add(IDC_EDIT_POINTS_PER_POINT);
	m_arControlIDs.Add(IDC_POINTS_PER_POINT);

	m_arEditControlIDs.Add(IDC_EDIT_POINTS_PER_DOLLAR);
	m_arEditControlIDs.Add(IDC_EDIT_BILL_POINTS_FOR_EVERY);
	m_arEditControlIDs.Add(IDC_EDIT_BILL_POINTS_ACCUMULATE);
	m_arEditControlIDs.Add(IDC_EDIT_DOLLARS_FOR_EVERY);
	m_arEditControlIDs.Add(IDC_EDIT_DOLLARS_ACCUMULATE);
	m_arEditControlIDs.Add(IDC_EDIT_POINTS_PER_POINT);


	ASSERT(m_arControlIDs.GetSize() == 20);

	m_nCurAction = eraInvalid;

	m_bModified = FALSE;
	m_nLoading = 0;
}


void CRewardPointsSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRewardPointsSetupDlg)
	DDX_Control(pDX, IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS, m_btnAccumNoPoints);
	DDX_Control(pDX, IDC_REDEEM_REWARDS, m_btnRedeemRewards);
		DDX_Control(pDX, IDC_BTN_SETUP_POINTS_DETAIL, m_btnSetupPointsDetail);
	DDX_Control(pDX, IDC_EDIT_POINTS_PER_BILL, m_nxeditEditPointsPerBill);
	DDX_Control(pDX, IDC_EDIT_POINTS_PER_DOLLAR, m_nxeditEditPointsPerDollar);
	DDX_Control(pDX, IDC_EDIT_POINTS_PER_POINT, m_nxeditEditPointsPerPoint);
	DDX_Control(pDX, IDC_EDIT_DOLLARS_FOR_EVERY, m_nxeditEditDollarsForEvery);
	DDX_Control(pDX, IDC_EDIT_BILL_POINTS_FOR_EVERY, m_nxeditEditBillPointsForEvery);
	DDX_Control(pDX, IDC_EDIT_DOLLARS_ACCUMULATE, m_nxeditEditDollarsAccumulate);
	DDX_Control(pDX, IDC_EDIT_BILL_POINTS_ACCUMULATE, m_nxeditEditBillPointsAccumulate);
	DDX_Control(pDX, IDC_POINTS_PER_POINT, m_nxstaticPointsPerPoint);
	DDX_Control(pDX, IDC_REWARD_POINTS_TITLE, m_nxstaticRewardPointsTitle);
	DDX_Control(pDX, IDC_POINTS_PER_BILL, m_nxstaticPointsPerBill);
	DDX_Control(pDX, IDC_POINTS_PER_DOLLAR, m_nxstaticPointsPerDollar);
	DDX_Control(pDX, IDC_DOLLARS_FOR_EVERY, m_nxstaticDollarsForEvery);
	DDX_Control(pDX, IDC_DOLLARS_DOLLARS, m_nxstaticDollarsDollars);
	DDX_Control(pDX, IDC_DOLLARS_ACCUMULATE, m_nxstaticDollarsAccumulate);
	DDX_Control(pDX, IDC_DOLLARS_POINTS, m_nxstaticDollarsPoints);
	DDX_Control(pDX, IDC_BILL_POINTS_FOR_EVERY, m_nxstaticBillPointsForEvery);
	DDX_Control(pDX, IDC_BILL_POINTS_BILL_POINTS, m_nxstaticBillPointsBillPoints);
	DDX_Control(pDX, IDC_BILL_POINTS_ACCUMULATE, m_nxstaticBillPointsAccumulate);
	DDX_Control(pDX, IDC_BILL_POINTS_POINTS, m_nxstaticBillPointsPoints);
	DDX_Control(pDX, IDC_RESET_REWARD_POINTS_BTN, m_btnResetRewardPoints);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRewardPointsSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRewardPointsSetupDlg)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT_DOLLARS_FOR_EVERY, OnChangeEditDollarsForEvery)
	ON_EN_CHANGE(IDC_EDIT_DOLLARS_ACCUMULATE, OnChangeEditDollarsAccumulate)
	ON_EN_CHANGE(IDC_EDIT_BILL_POINTS_FOR_EVERY, OnChangeEditBillPointsForEvery)
	ON_EN_CHANGE(IDC_EDIT_BILL_POINTS_ACCUMULATE, OnChangeEditBillPointsAccumulate)
	ON_WM_DESTROY()
	ON_EN_KILLFOCUS(IDC_EDIT_BILL_POINTS_FOR_EVERY, OnKillfocusEditBillPointsForEvery)
	ON_EN_KILLFOCUS(IDC_EDIT_BILL_POINTS_ACCUMULATE, OnKillfocusEditBillPointsAccumulate)
	ON_EN_KILLFOCUS(IDC_EDIT_DOLLARS_ACCUMULATE, OnKillfocusEditDollarsAccumulate)
	ON_EN_KILLFOCUS(IDC_EDIT_DOLLARS_FOR_EVERY, OnKillfocusEditDollarsForEvery)
	ON_EN_KILLFOCUS(IDC_EDIT_POINTS_PER_BILL, OnKillfocusEditPointsPerBill)
	ON_BN_CLICKED(IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS, OnCheckAccumulateWhenNoPoints)
	ON_EN_CHANGE(IDC_EDIT_POINTS_PER_BILL, OnChangeEditPointsPerBill)
	ON_BN_CLICKED(IDC_BTN_SETUP_POINTS_DETAIL, OnSetupPointsDetail)
	ON_BN_CLICKED(IDC_REDEEM_REWARDS, OnRedeemRewards)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RESET_REWARD_POINTS_BTN, &CRewardPointsSetupDlg::OnBnClickedResetRewardPointsBtn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRewardPointsSetupDlg message handlers

BOOL CRewardPointsSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (a.wetta 2007-05-24 17:02) - PLID 25394 - We're loading
		m_nLoading++;

		// (a.wetta 2007-05-21 14:23) - PLID 25960 - Bold the title
		extern CPracticeApp theApp;
		GetDlgItem(IDC_REWARD_POINTS_TITLE)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		SetControlPositions();
		
		// (z.manning, 05/14/2008) - PLID 29566
		m_btnRedeemRewards.AutoSet(NXB_MODIFY);
		m_btnSetupPointsDetail.AutoSet(NXB_MODIFY);
		m_btnResetRewardPoints.SetTextColor(RGB(255, 50, 50)); // (a.wilson 2013-02-22 11:30) - PLID 50591

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
		m_dlAction = BindNxDataList2Ctrl(IDC_REWARD_ACTION_LIST, GetRemoteData(), false);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAction->GetNewRow();

		pRow->PutValue(eacID, _variant_t((long)eraBill));
		pRow->PutValue(eacName, _variant_t("When saving a new bill..."));
		m_dlAction->AddRowAtEnd(pRow, NULL);
		m_dlAction->PutCurSel(pRow);
		m_nCurAction = eraBill;

		pRow = m_dlAction->GetNewRow();
		pRow->PutValue(eacID, _variant_t((long)eraCharge));
		pRow->PutValue(eacName, _variant_t("When saving a new charge..."));
		m_dlAction->AddRowAtEnd(pRow, NULL);

		pRow = m_dlAction->GetNewRow();
		pRow->PutValue(eacID, _variant_t((long)eraRefPatient));
		pRow->PutValue(eacName, _variant_t("When referring a patient..."));
		m_dlAction->AddRowAtEnd(pRow, NULL);

		pRow = m_dlAction->GetNewRow();
		pRow->PutValue(eacID, _variant_t((long)eraRefBill));
		pRow->PutValue(eacName, _variant_t("When a referred patient has a new bill saved..."));
		m_dlAction->AddRowAtEnd(pRow, NULL);

		m_bModified = FALSE;	

		HandleSelChanged(eraInvalid, eraBill);

		// (a.wilson 2013-02-25 10:20) - PLID 50591 - determine whether button should be enabled on initial tab creation.
		BOOL bAdmin = IsCurrentUserAdministrator();
		GetDlgItem(IDC_RESET_REWARD_POINTS_BTN)->EnableWindow(bAdmin);

		// (a.wetta 2007-05-24 14:09) - PLID 25394 - If they don't have write permission, don't enable the edit controls
		BOOL bWritePermission = CheckCurrentUserPermissions(bioRewardPoints, sptWrite, FALSE, 0, TRUE, TRUE);
		GetDlgItem(IDC_EDIT_DOLLARS_FOR_EVERY)->EnableWindow(bWritePermission);
		GetDlgItem(IDC_EDIT_DOLLARS_ACCUMULATE)->EnableWindow(bWritePermission);
		GetDlgItem(IDC_EDIT_POINTS_PER_BILL)->EnableWindow(bWritePermission);
		GetDlgItem(IDC_EDIT_BILL_POINTS_FOR_EVERY)->EnableWindow(bWritePermission);
		GetDlgItem(IDC_EDIT_BILL_POINTS_ACCUMULATE)->EnableWindow(bWritePermission);
		GetDlgItem(IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS)->EnableWindow(bWritePermission);
		GetDlgItem(IDC_BTN_SETUP_POINTS_DETAIL)->EnableWindow(bWritePermission);

		// (a.walling 2007-10-24 14:29) - PLID 20838 - International support
		GetDlgItem(IDC_POINTS_PER_DOLLAR)->SetWindowText(FormatString("Points per %s", GetCurrencySymbol()));
		GetDlgItem(IDC_DOLLARS_DOLLARS)->SetWindowText(FormatString("%s, ", GetCurrencySymbol()));

		m_nLoading--;

	} NxCatchAll("Error initializing reward points setup dialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CRewardPointsSetupDlg::SaveChanges(long nCurSel)
{
	if (!m_bModified)
		return TRUE;

	// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
	if (CheckCurrentUserPermissions(bioRewardPoints, sptWrite)) {
		switch ((ERewardActions)nCurSel) {
		case eraBill:
			{
				long nDollars = GetDlgItemInt(IDC_EDIT_DOLLARS_FOR_EVERY, NULL, FALSE);
				long nPoints = GetDlgItemInt(IDC_EDIT_DOLLARS_ACCUMULATE, NULL, FALSE);

				if (nDollars == 0 || nPoints == 0) {
					nDollars = nPoints = 0;
				}

				long nPointsPerBill = GetDlgItemInt(IDC_EDIT_POINTS_PER_BILL, NULL, FALSE);

				// (a.walling 2007-10-26 15:52) - PLID 20838 - Throw an exception if any are
				// negative. This can only happen when pasting a negative value in the box.
				if ( (nPointsPerBill < 0) || (nDollars < 0) || (nPoints < 0) ) {
					ThrowNxException("Cannot save negative values!");
				}

				SaveValue("Bill_PointsPerBill", nPointsPerBill);
				SaveValue("Bill_Dollars", nDollars);
				SaveValue("Bill_Points", nPoints);
			}
			break;
		case eraCharge: 
			{
				long nDollars = GetDlgItemInt(IDC_EDIT_DOLLARS_FOR_EVERY, NULL, FALSE);
				long nPoints = GetDlgItemInt(IDC_EDIT_DOLLARS_ACCUMULATE, NULL, FALSE);

				if (nDollars == 0 || nPoints == 0) {
					nDollars = nPoints = 0;
				}

				BOOL bOnlyUsePriceWhenNoPointsSet = IsDlgButtonChecked(IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS);

				// (a.walling 2007-10-26 15:52) - PLID 20838 - Throw an exception if any are
				// negative. This can only happen when pasting a negative value in the box.
				if ( (nDollars < 0) || (nPoints < 0) ) {
					ThrowNxException("Cannot save negative values!");
				}

				SaveValue("Charge_Dollars", nDollars);
				SaveValue("Charge_Points", nPoints);
				SaveValue("Charge_UsePriceWhenNoPoints", bOnlyUsePriceWhenNoPointsSet);
			}
			break;
		case eraRefPatient: 
			{
				long nPointsPerReferral = GetDlgItemInt(IDC_EDIT_POINTS_PER_BILL, NULL, FALSE);

				// (a.walling 2007-10-26 15:52) - PLID 20838 - Throw an exception if any are
				// negative. This can only happen when pasting a negative value in the box.
				if ( (nPointsPerReferral < 0) ) {
					ThrowNxException("Cannot save negative values!");
				}

				SaveValue("Ref_PointsPerReferral", nPointsPerReferral);
			}
			break;
		case eraRefBill: 
			{
				long nDollars = GetDlgItemInt(IDC_EDIT_DOLLARS_FOR_EVERY, NULL, FALSE);
				long nPoints = GetDlgItemInt(IDC_EDIT_DOLLARS_ACCUMULATE, NULL, FALSE);

				if (nDollars == 0 || nPoints == 0) {
					nDollars = nPoints = 0;
				}

				long nRefBillPoints = GetDlgItemInt(IDC_EDIT_BILL_POINTS_FOR_EVERY, NULL, FALSE);
				long nBillPoints = GetDlgItemInt(IDC_EDIT_BILL_POINTS_ACCUMULATE, NULL, FALSE);

				if (nRefBillPoints == 0 || nBillPoints == 0) {
					nRefBillPoints = nBillPoints = 0;
				}

				long nPointsPerBill = GetDlgItemInt(IDC_EDIT_POINTS_PER_BILL, NULL, FALSE);;

				// (a.walling 2007-10-26 15:52) - PLID 20838 - Throw an exception if any are
				// negative. This can only happen when pasting a negative value in the box.
				if ( (nPointsPerBill < 0) || (nDollars < 0) || (nPoints < 0) || (nRefBillPoints < 0) || (nBillPoints < 0) ) {
					ThrowNxException("Cannot save negative values!");
				}

				SaveValue("RefBill_PointsPerBill", nPointsPerBill);
				SaveValue("RefBill_Dollars", nDollars);
				SaveValue("RefBill_Points", nPoints);
				SaveValue("RefBill_RefBillPoints", nRefBillPoints);
				SaveValue("RefBill_BillPoints", nBillPoints);
			}
			break;
		default: {

				 }
			break;
		}
	}
	else {
		// (a.wetta 2007-05-24 16:42) - PLID 25394 - Reset the values
		LoadValues(nCurSel);
	}
	
	m_bModified = FALSE;	

	return TRUE;
}

void CRewardPointsSetupDlg::HandleSelChanged(long nOldSel, long nNewSel)
{
	try {
		if (nOldSel != eraInvalid) {
			if (!SaveChanges(nOldSel)) {
				m_dlAction->FindByColumn(eacID, _variant_t(nOldSel), NULL, VARIANT_TRUE);
			}
		}
		
		ShowAllControls(FALSE);

		m_nCurAction = (ERewardActions)nNewSel;

		switch ((ERewardActions)nNewSel) {
		case eraBill: // When saving a new bill...
			// show points per bill, points per $
			ShowPointsPer("Bill", TRUE);
			ShowPointsPerDollar(TRUE);
			break;
		case eraCharge: // When saving a new charge...
			// show points per $, setup point values
			ShowPointsPerDollar(TRUE);
			ShowSetupPointValues(TRUE); // includes checkbox
			//IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS
			break;
		case eraRefPatient: // When referring a patient...
			// show points per referral
			ShowPointsPer("Referral", TRUE);
			break;
		case eraRefBill: // When a referred patient has a new bill saved...
			// show points per bill, points per $, points per points
			ShowPointsPer("Bill", TRUE);
			ShowPointsPerDollar(TRUE);
			ShowPointsPerPoints(TRUE);
			break;
		default:
			ASSERT(FALSE);
			break;
		}

		LoadValues(nNewSel);
	} NxCatchAll("Error in HandleSelChanged");
}

void CRewardPointsSetupDlg::LoadValues(long nAction)
{
	// (a.wetta 2007-05-24 17:03) - PLID 25394 - We're loading
	m_nLoading++;

	switch ((ERewardActions)nAction) {
	case eraBill: // When saving a new bill...
		LoadValue("Bill_PointsPerBill", IDC_EDIT_POINTS_PER_BILL);
		LoadValue("Bill_Dollars", IDC_EDIT_DOLLARS_FOR_EVERY);
		LoadValue("Bill_Points", IDC_EDIT_DOLLARS_ACCUMULATE);
		UpdateDollarsPer();
		break;
	case eraCharge: // When saving a new charge...
		//IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS
		LoadValue("Charge_Dollars", IDC_EDIT_DOLLARS_FOR_EVERY);
		LoadValue("Charge_Points", IDC_EDIT_DOLLARS_ACCUMULATE);

		CheckDlgButton(IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS, GetRemotePropertyInt("Charge_UsePriceWhenNoPoints", 0, 0, "<None>", false));
		UpdateDollarsPer();
		break;
	case eraRefPatient: // When referring a patient...
		LoadValue("Ref_PointsPerReferral", IDC_EDIT_POINTS_PER_BILL);
		break;
	case eraRefBill: // When a referred patient has a new bill saved...
		LoadValue("RefBill_PointsPerBill", IDC_EDIT_POINTS_PER_BILL);
		LoadValue("RefBill_Dollars", IDC_EDIT_DOLLARS_FOR_EVERY);
		LoadValue("RefBill_Points", IDC_EDIT_DOLLARS_ACCUMULATE);
		LoadValue("RefBill_RefBillPoints", IDC_EDIT_BILL_POINTS_FOR_EVERY);
		LoadValue("RefBill_BillPoints", IDC_EDIT_BILL_POINTS_ACCUMULATE);

		UpdateDollarsPer();
		UpdatePointsPer();
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	m_nLoading--;
}

void CRewardPointsSetupDlg::ShowAllControls(BOOL bShow)
{
	int nShow = bShow ? SW_SHOW : SW_HIDE;

	for (int i = 0; i < m_arControlIDs.GetSize(); i++) {
		GetDlgItem(m_arControlIDs[i])->ShowWindow(nShow);
	}

	// (a.wetta 2007-05-24 17:03) - PLID 25394 - We're loading
	m_nLoading++;

	for (int j = 0; j < m_arEditControlIDs.GetSize(); j++) {
		SetDlgItemText(m_arEditControlIDs[j], "0");
	}

	m_nLoading--;
}

void CRewardPointsSetupDlg::ShowPointsPer(CString strText, BOOL bShow)
{
	int nShow = bShow ? SW_SHOW : SW_HIDE;

	SetDlgItemText(IDC_POINTS_PER_BILL, FormatString("Points per %s", strText));
	GetDlgItem(IDC_POINTS_PER_BILL)->ShowWindow(nShow);

	GetDlgItem(IDC_EDIT_POINTS_PER_BILL)->ShowWindow(nShow);
}

void CRewardPointsSetupDlg::ShowPointsPerDollar(BOOL bShow)
{
	int nShow = bShow ? SW_SHOW : SW_HIDE;

	long nCount = 8;
	long nArray[] = {
		IDC_POINTS_PER_DOLLAR,
		IDC_EDIT_POINTS_PER_DOLLAR,
		IDC_DOLLARS_FOR_EVERY,
		IDC_EDIT_DOLLARS_FOR_EVERY,
		IDC_DOLLARS_DOLLARS,
		IDC_DOLLARS_ACCUMULATE,
		IDC_EDIT_DOLLARS_ACCUMULATE,
		IDC_DOLLARS_POINTS
	};

	for (int i = 0; i < nCount; i++) {
		GetDlgItem(nArray[i])->ShowWindow(nShow);
	}
}

void CRewardPointsSetupDlg::ShowPointsPerPoints(BOOL bShow)
{
	int nShow = bShow ? SW_SHOW : SW_HIDE;

	long nCount = 8;
	long nArray[] = {
		IDC_BILL_POINTS_FOR_EVERY,
		IDC_EDIT_BILL_POINTS_FOR_EVERY,
		IDC_BILL_POINTS_BILL_POINTS,
		IDC_BILL_POINTS_ACCUMULATE,
		IDC_EDIT_BILL_POINTS_ACCUMULATE,
		IDC_BILL_POINTS_POINTS,
		IDC_EDIT_POINTS_PER_POINT,
		IDC_POINTS_PER_POINT
	};

	for (int i = 0; i < nCount; i++) {
		GetDlgItem(nArray[i])->ShowWindow(nShow);
	}
}

void CRewardPointsSetupDlg::ShowSetupPointValues(BOOL bShow)
{
	int nShow = bShow ? SW_SHOW : SW_HIDE;

	long nCount = 2;
	long nArray[] = {
		IDC_BTN_SETUP_POINTS_DETAIL,
		IDC_CHECK_ACCUMULATE_WHEN_NO_POINTS
	};

	for (int i = 0; i < nCount; i++) {
		GetDlgItem(nArray[i])->ShowWindow(nShow);
	}
}

BEGIN_EVENTSINK_MAP(CRewardPointsSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRewardPointsSetupDlg)
	ON_EVENT(CRewardPointsSetupDlg, IDC_REWARD_ACTION_LIST, 1 /* SelChanging */, OnSelChangingRewardActionList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CRewardPointsSetupDlg, IDC_REWARD_ACTION_LIST, 2 /* SelChanged */, OnSelChangedRewardActionList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRewardPointsSetupDlg::OnSelChangingRewardActionList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}	
}

void CRewardPointsSetupDlg::OnSelChangedRewardActionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
	NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewSel);

	long nOldSel, nNewSel;

	nOldSel = pOldRow != NULL ? VarLong(pOldRow->GetValue(eacID), -1) : -1;
	nNewSel = pNewRow != NULL ? VarLong(pNewRow->GetValue(eacID), -1) : -1;

	HandleSelChanged(nOldSel, nNewSel);
}

HBRUSH CRewardPointsSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	/*
	BOOL bFound = FALSE;

	int i = 0;
	while (!bFound && i < m_arControlIDs.GetSize()) {
		bFound = nCtlColor == m_arControlIDs[i];
		i++;
	}
		
	if (bFound || nCtlColor == IDC_STATIC)
	{
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x008080FF));
		return m_brush;
	}
	return hbr;
	*/
}

void CRewardPointsSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAction->GetCurSel();

		if (pRow) {
			long nCurAction = VarLong(pRow->GetValue(eacID));
			HandleSelChanged(nCurAction, nCurAction);
		}
	} NxCatchAll("Error in CRewardPointsSetupDlg::UpdateView");
}

void CRewardPointsSetupDlg::OnSize(UINT nType, int cx, int cy) 
{
	try {
		CNxDialog::OnSize(nType, cx, cy);
		
		SetControlPositions();
		Invalidate();
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnSize");
}

void CRewardPointsSetupDlg::UpdateDollarsPer()
{
	try {
		long nDollars = GetDlgItemInt(IDC_EDIT_DOLLARS_FOR_EVERY, NULL, FALSE);
		long nPoints = GetDlgItemInt(IDC_EDIT_DOLLARS_ACCUMULATE, NULL, FALSE);

		double dPointsPerDollar = nDollars > 0 ? (double)nPoints / (double)nDollars : 0;
		
		// I've heard the four digit precision is confusing to the user, so just use 2 as in currency.
		SetDlgItemText(IDC_EDIT_POINTS_PER_DOLLAR, FormatString("%.2f", dPointsPerDollar));
	} NxCatchAll("Error in CRewardPointsSetupDlg::UpdateDollarsPer");
}

void CRewardPointsSetupDlg::UpdatePointsPer()
{
	try {
		long nRefBillPoints = GetDlgItemInt(IDC_EDIT_BILL_POINTS_FOR_EVERY, NULL, FALSE);
		long nBillPoints = GetDlgItemInt(IDC_EDIT_BILL_POINTS_ACCUMULATE, NULL, FALSE);

		double dPointsPerPoints = nRefBillPoints > 0 ? (double)nBillPoints / (double)nRefBillPoints : 0;
		
		// I've heard the four digit precision is confusing to the user, so just use 2 as in currency.
		SetDlgItemText(IDC_EDIT_POINTS_PER_POINT, FormatString("%.2f", dPointsPerPoints));	
	} NxCatchAll("Error in CRewardPointsSetupDlg::UpdatePointsPer");
}

void CRewardPointsSetupDlg::OnChangeEditDollarsForEvery() 
{
	// (a.wetta 2007-05-24 17:04) - PLID 25394 - Nothing was modified if we're loading
	if (m_nLoading == 0)
		m_bModified = TRUE;	
	UpdateDollarsPer();
}

void CRewardPointsSetupDlg::OnChangeEditDollarsAccumulate() 
{
	// (a.wetta 2007-05-24 17:04) - PLID 25394 - Nothing was modified if we're loading
	if (m_nLoading == 0)
		m_bModified = TRUE;	
	UpdateDollarsPer();
}

void CRewardPointsSetupDlg::OnChangeEditBillPointsForEvery() 
{
	// (a.wetta 2007-05-24 17:04) - PLID 25394 - Nothing was modified if we're loading
	if (m_nLoading == 0)
		m_bModified = TRUE;	
	UpdatePointsPer();
}

void CRewardPointsSetupDlg::OnChangeEditBillPointsAccumulate() 
{
	// (a.wetta 2007-05-24 17:04) - PLID 25394 - Nothing was modified if we're loading
	if (m_nLoading == 0)
		m_bModified = TRUE;	
	UpdatePointsPer();
}

void CRewardPointsSetupDlg::SaveValue(const CString &strName, long nVal) {
	SetRemotePropertyInt(strName, nVal, 0, "<None>");
}

void CRewardPointsSetupDlg::LoadValue(const CString &strName, long nID) {
	SetDlgItemInt(nID, GetRemotePropertyInt(strName, 0, 0, "<None>", false), FALSE);
}

// (a.walling 2007-10-26 15:57) - PLID 20838 - Added try/catch handling

void CRewardPointsSetupDlg::OnDestroy() 
{
	try {
		SaveChanges(m_nCurAction);
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnDestroy");

	CNxDialog::OnDestroy();
}

void CRewardPointsSetupDlg::OnKillfocusEditBillPointsForEvery() 
{
	try {
		SaveChanges(m_nCurAction);	
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnKillfocusEditBillPointsForEvery");
}

void CRewardPointsSetupDlg::OnKillfocusEditBillPointsAccumulate() 
{
	try {
		SaveChanges(m_nCurAction);	
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnKillfocusEditBillPointsAccumulate");
}

void CRewardPointsSetupDlg::OnKillfocusEditDollarsAccumulate() 
{
	try {
		SaveChanges(m_nCurAction);
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnKillfocusEditDollarsAccumulate");
}

void CRewardPointsSetupDlg::OnKillfocusEditDollarsForEvery() 
{
	try {
		SaveChanges(m_nCurAction);	
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnKillfocusEditDollarsForEvery");
}

void CRewardPointsSetupDlg::OnKillfocusEditPointsPerBill() 
{
	try {
		SaveChanges(m_nCurAction);	
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnKillfocusEditPointsPerBill");
}

void CRewardPointsSetupDlg::OnCheckAccumulateWhenNoPoints() 
{
	try {
		// (a.wetta 2007-05-24 17:04) - PLID 25394 - Nothing was modified if we're loading
		if (m_nLoading == 0)
			m_bModified = TRUE;	
		SaveChanges(m_nCurAction);	
	} NxCatchAll("Error in CRewardPointsSetupDlg::OnCheckAccumulateWhenNoPoints");
}

void CRewardPointsSetupDlg::OnChangeEditPointsPerBill() 
{
	// (a.wetta 2007-05-24 17:04) - PLID 25394 - Nothing was modified if we're loading
	if (m_nLoading == 0)
		m_bModified = TRUE;	
}

// (a.wetta 2007-05-21 16:04) - PLID 26078 - Open up the advanced reward points setup dialog
void CRewardPointsSetupDlg::OnSetupPointsDetail()
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioRewardPoints, sptWrite)) {
			return;
		}

		CAdvRewardPointsSetup dlg(this);
		// (a.walling 2007-05-29 17:45) - PLID 26172
		dlg.SetType(CAdvRewardPointsSetup::erpstAccumulate);

		dlg.DoModal();

	}NxCatchAll("Error in CRewardPointsSetupDlg::OnSetupPointsDetail");
}

void CRewardPointsSetupDlg::OnRedeemRewards() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioRewardPoints, sptWrite)) {
			return;
		}

		CAdvRewardPointsSetup dlg(this);
		// (a.walling 2007-05-29 17:45) - PLID 26172
		dlg.SetType(CAdvRewardPointsSetup::erpstRedeem);

		dlg.DoModal();

	}NxCatchAll("Error in CRewardPointsSetupDlg::OnRedeemRewards");	
}

// (a.wilson 2013-02-22 11:16) - PLID 50591 - a feature to reset all reward points back to 0 for all patients.
void CRewardPointsSetupDlg::OnBnClickedResetRewardPointsBtn()
{
	try {
		//first check if the current user is an administrator
		if (!IsCurrentUserAdministrator())
		{
			AfxMessageBox("You must be an administrator to perform this action.");
			return;
		}

		//prompt them to ensure they want to perform this action, ensure they understand that this process cannot be reversed.
		CString strWarning = ("Performing this action will reset every single patient's reward points back to a zero balance. "
							  "This action cannot be reversed.\r\n\r\nAre you sure you want to do this?");
		if (IDYES == MsgBox((UINT)(MB_YESNO | MB_ICONEXCLAMATION), strWarning))
		{
			long nPatientCount = 0;
			//run the query to do so. don't forget to audit this action as well.
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"BEGIN TRAN "
					"SET NOCOUNT ON "
					"SET XACT_ABORT ON "
					"INSERT INTO	RewardHistoryT (ModifiedDate, DeletedDate, Deleted, PatientID, Source, SourceValue, BillID, RefPatientID, Points, ChargeID) "
					"SELECT			GETDATE(), NULL, 0, PatientID, 0, 0, NULL, NULL, (SUM(Points) * -1), NULL "
					"FROM			RewardHistoryT "
					"WHERE			Deleted = 0 "
					"GROUP BY		PatientID "
					"HAVING			SUM(Points) <> 0 "
					"SELECT			@@ROWCOUNT AS Count "
				"COMMIT TRAN ");
			//get the number of patients we modified for auditing.
			if (!prs->eof) {
				nPatientCount = AdoFldLong(prs, "Count", 0);
			}
			CString strPatientsChanged = FormatString("%li Patient%s had their Reward Points set to 0.", nPatientCount, ((nPatientCount == 1) ? "" : "s"));
			//only audit if data was changed.
			if (nPatientCount > 0) {
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiResetRewardPoints, -1, "", strPatientsChanged, aepHigh, aetChanged);
			}
			//display a message explaining how many patients had their data modified.
			AfxMessageBox(strPatientsChanged);
		}		

	} NxCatchAll(__FUNCTION__);
}
