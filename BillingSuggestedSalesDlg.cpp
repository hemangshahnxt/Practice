// BillingSuggestedSalesDlg.cpp : implementation file
//

// (a.walling 2007-05-07 16:28) - PLID 14717 - Dialog box to suggest sales to the user
// based on the current bill and another tab for the previous bill.

#include "stdafx.h"
#include "billingRc.h"
#include "BillingSuggestedSalesDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "BillingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBillingSuggestedSalesDlg dialog

namespace DatalistSubclass {
	WNDPROC g_dlThisListWndProc;
	WNDPROC g_dlAllListWndProc;
	WNDPROC g_dlCurThisListWndProc;
	WNDPROC g_dlCurAllListWndProc;
	HWND g_hwndSuggestedSales;

	LRESULT CALLBACK MouseMoveWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK MouseMoveWndProcAll(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
};


CBillingSuggestedSalesDlg::CBillingSuggestedSalesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillingSuggestedSalesDlg::IDD, pParent)
{
	m_dlList = NULL;
	m_pBillingDlg = NULL;
	m_nLastReasonSuggestionID = -1;

	m_nThisLastSuggestionID = -1;
	m_nAllLastSuggestionID = -1;

	m_nPatientID = -1;
	m_nBillID = -1;

	DatalistSubclass::g_hwndSuggestedSales = NULL;

	DatalistSubclass::g_dlThisListWndProc = NULL;
	DatalistSubclass::g_dlAllListWndProc = NULL;
	DatalistSubclass::g_dlCurThisListWndProc = NULL;
	DatalistSubclass::g_dlCurAllListWndProc = NULL;

	m_bAllListNeedsRefresh = TRUE;
	m_bThisListNeedsRefresh = TRUE;

	// (j.armen 2012-06-06 12:39) - PLID 50830 - Set min size
	SetMinSize(50, 55);
}

CBillingSuggestedSalesDlg::~CBillingSuggestedSalesDlg()
{
	DatalistSubclass::g_hwndSuggestedSales = NULL;

	DatalistSubclass::g_dlThisListWndProc = NULL;
	DatalistSubclass::g_dlAllListWndProc = NULL;
	DatalistSubclass::g_dlCurThisListWndProc = NULL;
	DatalistSubclass::g_dlCurAllListWndProc = NULL;
}


void CBillingSuggestedSalesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBillingSuggestedSalesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SUGGESTED_SALES_REASON, m_nxeditSuggestedSalesReason);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBillingSuggestedSalesDlg, CNxDialog)
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBillingSuggestedSalesDlg message handlers

BOOL CBillingSuggestedSalesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// restore our position
		CRect rRect, rNewRect;
		GetWindowRect(rRect);
		long nWidth = GetRemotePropertyInt("SuggestedSalesPopupWidth", rRect.Width(), 0, GetCurrentUserName(), true);
		long nHeight = GetRemotePropertyInt("SuggestedSalesPopupHeight", rRect.Height(), 0, GetCurrentUserName(), true);
		long nTop = GetRemotePropertyInt("SuggestedSalesPopupTop", rRect.top, 0, GetCurrentUserName(), true);
		long nLeft = GetRemotePropertyInt("SuggestedSalesPopupLeft", rRect.left, 0, GetCurrentUserName(), true);

		if ( (nWidth > 50) && (nHeight > 50) ) {
			// ensure a reasonable rect
			rNewRect.left = nLeft;
			rNewRect.top = nTop;
			rNewRect.right = rNewRect.left + nWidth;
			rNewRect.bottom = rNewRect.top + nHeight;

			CRect rDesktopRect;
			GetDesktopWindow()->GetWindowRect(rDesktopRect);
			rDesktopRect.DeflateRect(0, 0, 30, 50); // deflate the rect's bottom right corner to ensure the top left
				// corner of the new window rect is visible and able to move

			// either the top left or top right corner should be in our desktop rect
			CPoint ptTopLeft = rNewRect.TopLeft();
			CPoint ptTopRight = ptTopLeft;
			ptTopRight.x += rNewRect.Width();

			if (! (rDesktopRect.PtInRect(ptTopLeft) || rDesktopRect.PtInRect(ptTopRight) )) {
				rNewRect.CopyRect(rRect); // get the initial rect
				// now set the width and height
				rNewRect.right = rNewRect.left + nWidth;
				rNewRect.bottom = rNewRect.top + nHeight;
			}

			MoveWindow(rNewRect);
		} else {
			ASSERT(FALSE);
		}

		DatalistSubclass::g_hwndSuggestedSales = GetSafeHwnd();

		m_dlList = BindNxDataList2Ctrl(this, IDC_DATALIST_SUGGESTED_SALES, GetRemoteData(), false);	
		m_dlListAll = BindNxDataList2Ctrl(this, IDC_DATALIST_SUGGESTED_SALES_ALL, GetRemoteData(), false);	

		CWnd* pDatalist = GetDlgItem(IDC_DATALIST_SUGGESTED_SALES);
		if ((pDatalist != NULL) && pDatalist->GetSafeHwnd())
		{
			HWND hwnd = ::GetWindow(pDatalist->GetSafeHwnd(), GW_CHILD);
			if (hwnd) {
				DatalistSubclass::g_dlThisListWndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (long)DatalistSubclass::MouseMoveWndProc);
				DatalistSubclass::g_dlCurThisListWndProc = DatalistSubclass::g_dlThisListWndProc;
			}
		}

		CWnd* pDatalistAll = GetDlgItem(IDC_DATALIST_SUGGESTED_SALES_ALL);
		if ((pDatalistAll != NULL) && pDatalistAll->GetSafeHwnd())
		{
			HWND hwnd = ::GetWindow(pDatalistAll->GetSafeHwnd(), GW_CHILD);
			if (hwnd) {
				DatalistSubclass::g_dlAllListWndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (long)DatalistSubclass::MouseMoveWndProcAll);
				DatalistSubclass::g_dlCurAllListWndProc = DatalistSubclass::g_dlAllListWndProc;
			}
		}

		m_pTabs = GetDlgItemUnknown(IDC_TAB_SUGGESTED_SALES);
		if (m_pTabs) {
			m_pTabs->PutLabel(0, "This bill");
			m_pTabs->PutLabel(1, "Previous bills");

			m_pTabs->PutTabWidth(2);
			m_pTabs->PutSize(2);
		} else {
			ThrowNxException("Could not bind NxTab control!");
		}

		// (j.jones 2016-04-20 11:01) - NX-100214 - Set HeaderMode to false, which will
		// use a slightly different theme than the module tabs use.
		// A HeaderMode of false looks nicer when the tab is next to a datalist.
		m_pTabs->HeaderMode = false;

		RefreshIfNeeded();
	} NxCatchAll("Error initializing suggested sales dialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBillingSuggestedSalesDlg::OnCancel()
{
	try {
		if (m_pBillingDlg) {
			((CBillingDlg*)m_pBillingDlg)->SetSuggestedSalesHidden();
		}
	} NxCatchAll("Could not interact with the billing dialog!");

	ShowWindow(SW_HIDE);
}

// subclass the datalists so we can capture mousemove events to display the appropriate reasons

LRESULT CALLBACK DatalistSubclass::MouseMoveWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if ( (g_hwndSuggestedSales != NULL) && (iMsg == WM_MOUSEMOVE) ) {
		::SendMessage(g_hwndSuggestedSales, iMsg, wParam, lParam);
	}

	return CallWindowProc(g_dlCurThisListWndProc, hWnd, iMsg, wParam, lParam);
}

LRESULT CALLBACK DatalistSubclass::MouseMoveWndProcAll(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	if ( (g_hwndSuggestedSales != NULL) && (iMsg == WM_MOUSEMOVE) ) {
		::SendMessage(g_hwndSuggestedSales, iMsg, wParam, lParam);
	}

	return CallWindowProc(g_dlCurAllListWndProc, hWnd, iMsg, wParam, lParam);
}

void CBillingSuggestedSalesDlg::AddService(long nID)
{
	int i = 0;

	while ( (i < m_arServices.GetSize()) && (m_arServices[i] < nID) ) {
		i++;
	}

	m_arServices.InsertAt(i, nID);

	m_bThisListNeedsRefresh = TRUE;
}

BOOL CBillingSuggestedSalesDlg::ServicesHaveChanged()
{
	if (m_arServices.GetSize() != m_arCurServices.GetSize())
		return TRUE;

	if (m_arServices.GetSize() == 0)
		return TRUE;

	for (int i = 0; i < m_arServices.GetSize(); i++) {
		if (m_arServices[i] != m_arCurServices[i])
			return TRUE;
	}

	return FALSE;
}

void CBillingSuggestedSalesDlg::RefreshIfNeeded(BOOL bForceIfHidden /*= FALSE*/)
{
	try {
		// Don't refresh if our dialog has not been created yet
		if ( (GetSafeHwnd() == NULL) || (m_pTabs == NULL) )
			return;

		// or if we are hidden
		if (!bForceIfHidden && !IsWindowVisible())
			return;
		
		if ( (m_pTabs->GetCurSel() == ettThisBill) && (m_bThisListNeedsRefresh) )
			Refresh();
		else if ( (m_pTabs->GetCurSel() == ettAllBills) && (m_bAllListNeedsRefresh) )
			Refresh();
	} NxCatchAll("Could not try to refresh!");
}

void CBillingSuggestedSalesDlg::Refresh()
{
	try {
		if ( (GetSafeHwnd() == NULL) || (m_pTabs == NULL) )
			return;

		CString strFrom = FormatString(
         "(SELECT NULL AS RootID, "
                 "MasterServiceID AS ParentID, "
                 "MasterServiceID, "
                 "SuggestedSalesT.ID AS SuggestionID, "
                 "ServiceID, "
                 "ServiceT.Name, "
                 "CASE  "
                   "WHEN CPTCodeT.ID IS NULL THEN '<Inventory Item>' "
                   "ELSE CPTCodeT.Code + ' ' + CPTCodeT.SubCode "
                 "END AS Code, "
                 "ServiceT.Price, "
                 "Reason, "
                 "ServiceT.Active, "
                 "CASE  "
                   "WHEN CPTCodeT.ID IS NULL THEN 0 "
                   "ELSE 1 "
                 "END AS ServiceType, "
                 "OrderIndex, "
                 "NULL AS RecentBillDate "
          "FROM   SuggestedSalesT "
                 "LEFT JOIN ServiceT "
                   "ON SuggestedSalesT.ServiceID = ServiceT.ID "
                 "LEFT JOIN CPTCodeT "
                   "ON SuggestedSalesT.ServiceID = CPTCodeT.ID "
          "UNION  "
          "SELECT DISTINCT MasterServiceID AS RootID, "
                          "NULL AS ParentID, "
                          "MasterServiceID, "
                          "NULL AS SuggestionID, "
                          "MasterServiceID AS ServiceID, "
                          "ServiceT.Name, "
                          "CASE  "
                            "WHEN CPTCodeT.ID IS NULL THEN '<Inventory Item>' "
                            "ELSE CPTCodeT.Code + ' ' + CPTCodeT.SubCode "
                          "END AS Code, "
                          "NULL AS Price, "
                          "NULL AS Reason, "
                          "1 AS Active, "
                          "CASE  "
                            "WHEN CPTCodeT.ID IS NULL THEN 0 "
                            "ELSE 1 "
                          "END AS ServiceType, "
                          "-1 AS OrderIndex, "
                          "(SELECT   TOP 1 BillsT.Date "
                           "FROM     BillsT "
                                    "LEFT JOIN ChargesT "
                                      "ON BillsT.ID = ChargesT.BillID "
                           "WHERE    BillsT.PatientID = %li "
                                    "AND ChargesT.ServiceID = MasterServiceID "
                           "ORDER BY BillsT.Date DESC) AS RecentBillDate "
          "FROM   SuggestedSalesT "
                 "LEFT JOIN ServiceT "
                   "ON SuggestedSalesT.MasterServiceID = ServiceT.ID "
                 "LEFT JOIN CPTCodeT "
                   "ON SuggestedSalesT.MasterServiceID = CPTCodeT.ID) SuggestedSalesQ ",
				   m_nPatientID);

		if (m_pTabs->GetCurSel() == ettThisBill) {
			if ((m_dlList != NULL) && ServicesHaveChanged()) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetCurSel();
				if (pRow) {
					m_nThisLastSuggestionID = VarLong(pRow->GetValue(esscSuggestionID), -1);
				} else {
					m_nThisLastSuggestionID = -1;
				}

				CString strWhere = FormatString(" SuggestedSalesQ.MasterServiceID IN (%s) AND SuggestedSalesQ.Active = 1", ArrayAsString(m_arServices));
				m_arCurServices.Copy(m_arServices);

				m_dlList->PutFromClause(_bstr_t(strFrom));
				m_dlList->PutWhereClause(_bstr_t(strWhere));

				m_dlList->SetRedraw(VARIANT_FALSE);
				m_dlList->Requery();
				m_bThisListNeedsRefresh = FALSE;
			} else {
				m_bThisListNeedsRefresh = TRUE;
			}
		} else {
			if (m_dlListAll != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlListAll->GetCurSel();
				if (pRow) {
					m_nAllLastSuggestionID = VarLong(pRow->GetValue(esscSuggestionID), -1);
				} else {
					m_nAllLastSuggestionID = -1;
				}

				CString strWhere = FormatString(
					"SuggestedSalesQ.MasterServiceID IN (SELECT ChargesT.ServiceID FROM BillsT "
					"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"WHERE BillsT.Deleted = 0 AND BillsT.PatientID = %li AND BillsT.ID NOT IN (%li)) "
					"AND SuggestedSalesQ.Active = 1", m_nPatientID, m_nBillID);

				m_dlListAll->PutFromClause(_bstr_t(strFrom));
				m_dlListAll->PutWhereClause(_bstr_t(strWhere));

				m_dlListAll->SetRedraw(VARIANT_FALSE);
				m_dlListAll->Requery();
				m_bAllListNeedsRefresh = FALSE;
			} else {
				m_bAllListNeedsRefresh = TRUE;
			}
		}
	} NxCatchAll("Could not refresh");
}

BEGIN_EVENTSINK_MAP(CBillingSuggestedSalesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBillingSuggestedSalesDlg)
	ON_EVENT(CBillingSuggestedSalesDlg, IDC_DATALIST_SUGGESTED_SALES, 18 /* RequeryFinished */, OnRequeryFinishedDatalistSuggestedSales, VTS_I2)
	ON_EVENT(CBillingSuggestedSalesDlg, IDC_DATALIST_SUGGESTED_SALES, 19 /* LeftClick */, OnLeftClickDatalistSuggestedSales, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBillingSuggestedSalesDlg, IDC_DATALIST_SUGGESTED_SALES_ALL, 18 /* RequeryFinished */, OnRequeryFinishedDatalistSuggestedSalesAll, VTS_I2)
	ON_EVENT(CBillingSuggestedSalesDlg, IDC_DATALIST_SUGGESTED_SALES_ALL, 19 /* LeftClick */, OnLeftClickDatalistSuggestedSalesAll, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBillingSuggestedSalesDlg, IDC_TAB_SUGGESTED_SALES, 1 /* SelectTab */, OnSelectTab, VTS_I2 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CBillingSuggestedSalesDlg::OnRequeryFinishedDatalistSuggestedSales(short nFlags) 
{
	try {
		if (nFlags == NXDATALIST2Lib::dlRequeryFinishedCompleted) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetFirstRow();

			while (pRow) {
				pRow->PutExpanded(VARIANT_TRUE);

				pRow = pRow->GetNextRow();
			}

			if (m_nThisLastSuggestionID != -1) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->FindByColumn(esscSuggestionID, _variant_t(m_nThisLastSuggestionID, VT_I4), NULL, VARIANT_TRUE); // set selection
				if (pRow) {
					m_dlList->EnsureRowInView(pRow);
				}
			}

			if (m_dlList->GetRowCount() == 0) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetNewRow();

				pRow->PutValue(esscRootID, g_cvarNull);
				pRow->PutValue(esscParentID, g_cvarNull);
				pRow->PutValue(esscName, "<No suggestions>");

				m_dlList->AddRowAtEnd(pRow, NULL);
			}
		}

		m_dlList->SetRedraw(VARIANT_TRUE);
	} NxCatchAll("Error expanding columns");
}

void CBillingSuggestedSalesDlg::OnRequeryFinishedDatalistSuggestedSalesAll(short nFlags) 
{
	try {
		if (nFlags == NXDATALIST2Lib::dlRequeryFinishedCompleted) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlListAll->GetFirstRow();

			while (pRow) {
				pRow->PutExpanded(VARIANT_TRUE);

				pRow = pRow->GetNextRow();
			}

			if (m_nAllLastSuggestionID != -1) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlListAll->FindByColumn(esscSuggestionID, _variant_t(m_nAllLastSuggestionID, VT_I4), NULL, VARIANT_TRUE); // set selection
				if (pRow) {
					m_dlList->EnsureRowInView(pRow);
				}
			}

			if (m_dlListAll->GetRowCount() == 0) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlListAll->GetNewRow();

				pRow->PutValue(esscRootID, g_cvarNull);
				pRow->PutValue(esscParentID, g_cvarNull);
				pRow->PutValue(esscName, "<No suggestions>");

				m_dlListAll->AddRowAtEnd(pRow, NULL);
			}
		}

		m_dlListAll->SetRedraw(VARIANT_TRUE);
	} NxCatchAll("Error expanding columns all");
}

void CBillingSuggestedSalesDlg::OnLeftClickDatalistSuggestedSales(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if ( (nCol >= esscName) && (nCol <= esscPrice) ) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			
			if (pRow) {
				long nRootID = VarLong(pRow->GetValue(esscRootID), -2);
				long nParentID = VarLong(pRow->GetValue(esscParentID), -1);

				
				if ( (nRootID == -1) && (nParentID == -1) ) // this is a message rather than an interactive row
					return;

				if (nParentID == -1) {
					// this row is not a suggestion, but a parent, so toggle the expansion

					VARIANT_BOOL vbIsExpanded = pRow->GetExpanded();
					pRow->PutExpanded( (vbIsExpanded == VARIANT_TRUE) ? VARIANT_FALSE : VARIANT_TRUE); // toggle
				} else {
					m_dlList->PutCurSel(pRow);
					long nServiceID = VarLong(pRow->GetValue(esscServiceID), -1);

					if (nServiceID != -1) {
						CBillingDlg* pBillingDlg = (CBillingDlg*)m_pBillingDlg;

						// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
						// you can add new charges even if you have partial access
						if (pBillingDlg->m_eHasAccess != batNoAccess) {
							long nServiceType = VarLong(pRow->GetValue(esscServiceType), -1);

							if (nServiceType == 0) {
								// Product
								pBillingDlg->AddNewProductToBillByServiceID(nServiceID);
							} else if (nServiceType == 1) {
								// CPT code
								pBillingDlg->AddNewChargeToBill(nServiceID);
							} else {
								ASSERT(FALSE);
								ThrowNxException("Bad service type value in datalist!");
							}
						} else {
							MessageBox("The bill is current marked as read-only. New charges cannot be added.");
						}
					}
				}
			}
		}
	} NxCatchAll("Error in OnLeftClickDatalistSuggestedSales");
}

void CBillingSuggestedSalesDlg::OnLeftClickDatalistSuggestedSalesAll(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if ( (nCol >= esscName) && (nCol <= esscPrice) ) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			
			if (pRow) {
				long nParentID = VarLong(pRow->GetValue(esscParentID), -1);

				if (nParentID == -1) {
					// this row is not a suggestion, but a parent, so toggle the expansion

					VARIANT_BOOL vbIsExpanded = pRow->GetExpanded();
					pRow->PutExpanded( (vbIsExpanded == VARIANT_TRUE) ? VARIANT_FALSE : VARIANT_TRUE); // toggle
				} else {
					m_dlListAll->PutCurSel(pRow);

					long nServiceID = VarLong(pRow->GetValue(esscServiceID), -1);

					if (nServiceID != -1) {
						CBillingDlg* pBillingDlg = (CBillingDlg*)m_pBillingDlg;

						// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
						// you can add new charges even if you have partial access
						if (pBillingDlg->m_eHasAccess != batNoAccess) {
							long nServiceType = VarLong(pRow->GetValue(esscServiceType), -1);

							if (nServiceType == 0) {
								// Product
								pBillingDlg->AddNewProductToBillByServiceID(nServiceID);
							} else if (nServiceType == 1) {
								// CPT code
								pBillingDlg->AddNewChargeToBill(nServiceID);
							} else {
								ASSERT(FALSE);
								ThrowNxException("Bad service type value in datalist!");
							}
						} else {
							MessageBox("The bill is current marked as read-only. New charges cannot be added.");
						}
					}
				}
			}
		}
	} NxCatchAll("Error in OnLeftClickDatalistSuggestedSalesAll");
}

void CBillingSuggestedSalesDlg::OnSelectTab(short newTab, short oldTab) 
{
	try {
		if (newTab == ettAllBills) {
			GetDlgItem(IDC_DATALIST_SUGGESTED_SALES_ALL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DATALIST_SUGGESTED_SALES)->ShowWindow(SW_HIDE);

			if (m_bAllListNeedsRefresh)
				Refresh();
		} else {
			GetDlgItem(IDC_DATALIST_SUGGESTED_SALES_ALL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DATALIST_SUGGESTED_SALES)->ShowWindow(SW_SHOW);

			if (m_bThisListNeedsRefresh)
				Refresh();
		}
	} NxCatchAll("Error in CBillingSuggestedSalesDlg::OnSelectTab");
}

void CBillingSuggestedSalesDlg::SetBillInfo(long nPatientID, long nBillID)
{
	try {
		m_nPatientID = nPatientID;
		m_nBillID = nBillID;

		// Reset all vars
		if (m_dlList) {
			m_dlList->CancelRequery();
			m_dlList->Clear();
		}

		if (m_dlListAll) {
			m_dlListAll->CancelRequery();
			m_dlListAll->Clear();
		}

		m_arServices.RemoveAll();
		m_arCurServices.RemoveAll();
		m_nThisLastSuggestionID = -1;
		m_nAllLastSuggestionID = -1;
		m_nLastReasonSuggestionID = -1;

		m_bAllListNeedsRefresh = TRUE;
		m_bThisListNeedsRefresh = TRUE;

		m_nLastReasonSuggestionID = -1;

		if (GetSafeHwnd())
			SetDlgItemText(IDC_SUGGESTED_SALES_REASON, "");
	} NxCatchAll("Error in CBillingSuggestedSalesDlg::SetBillInfo");
}

void CBillingSuggestedSalesDlg::OnSize(UINT nType, int cx, int cy) 
{
	try {
		CDialog::OnSize(nType, cx, cy);

		if (nType != SIZE_RESTORED)
			return;

		if (GetSafeHwnd() == NULL)
			return;

		CWnd* pTabs = GetDlgItem(IDC_TAB_SUGGESTED_SALES);
		CWnd* pDatalist = GetDlgItem(IDC_DATALIST_SUGGESTED_SALES);
		CWnd* pDatalistAll = GetDlgItem(IDC_DATALIST_SUGGESTED_SALES_ALL);
		CWnd* pReason = GetDlgItem(IDC_SUGGESTED_SALES_REASON);

		if ((pTabs != NULL) && (pDatalist != NULL) && (pReason != NULL)) {
		
			CRect rcClient;
			GetClientRect(rcClient);

			CRect rcTab, rcDatalist, rcReason;

			pTabs->GetClientRect(rcTab);
			pDatalist->GetClientRect(rcDatalist);
			pReason->GetClientRect(rcReason);

			rcTab.left = 0;
			rcTab.right = rcClient.right; // tabs are always at 0,0 to right, 10

			rcReason.left = 0;
			rcReason.top = rcClient.bottom - 35; // reason always at 0, bottom-35 to right, bottom
			rcReason.bottom = rcClient.bottom;
			rcReason.right = rcClient.right;

			rcDatalist.left = 0;
			rcDatalist.right = rcClient.right; // datalist is always at 0, rcTab.bottom to right, rcReason.top
			rcDatalist.top = rcTab.bottom;
			rcDatalist.bottom = rcReason.top;

			pTabs->MoveWindow(rcTab);
			pDatalist->MoveWindow(rcDatalist);
			pDatalistAll->MoveWindow(rcDatalist);
			pReason->MoveWindow(rcReason);
		}
	} NxCatchAll("Error in CBillingSuggestedSalesDlg::OnSize");
}

void CBillingSuggestedSalesDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	try {
		if (GetSafeHwnd() == NULL)
			return;

		CRect rcDatalist;
		GetClientRect(rcDatalist);

		if (rcDatalist.PtInRect(point)) {
			// mouse is in the datalist!
			GetCursorPos(&point); // this will give us screen coords rather than client

			CWnd* pDatalist = NULL;
			NXDATALIST2Lib::_DNxDataListPtr dlList = NULL;

			if (m_pTabs->GetCurSel() == ettThisBill) {
				pDatalist = GetDlgItem(IDC_DATALIST_SUGGESTED_SALES);
				dlList = m_dlList;
			}
			else {
				pDatalist = GetDlgItem(IDC_DATALIST_SUGGESTED_SALES_ALL);
				dlList = m_dlListAll;
			}

			if (pDatalist) {
				pDatalist->ScreenToClient(&point); // now we have a client point for the datalist

				NXDATALIST2Lib::IRowSettingsPtr pRow = dlList->GetRowFromPoint(point.x, point.y);
				
				if (pRow) {
					_variant_t varReason = pRow->GetValue(esscReason);

					if ( (varReason.vt == VT_BSTR) || (varReason.vt == VT_NULL) ) {
						//dlList->PutCurSel(pRow); // let's not PutCurSel

						_variant_t varParent = pRow->GetValue(esscParentID);
						_variant_t varRoot = pRow->GetValue(esscRootID);

						if ( (VarLong(varParent, -1) == -1) && (varRoot.vt == VT_I4) ) {
							// this is a parent row.
							if (m_pTabs->GetCurSel() == ettAllBills) {
								CString strCode = VarString(pRow->GetValue(esscCode), "No Code");
								strCode.TrimRight();

								// all bills. Put some info in the reason box.
								CString strInfo = FormatString("%s (%s)\r\n - Last billed on %s",
								VarString(pRow->GetValue(esscName), ""),
								strCode,
								FormatDateTimeForInterface(VarDateTime(pRow->GetValue(esscRecentBillDate))));

								SetDlgItemText(IDC_SUGGESTED_SALES_REASON, strInfo);
								m_nLastReasonSuggestionID = -1;
							} else {
								SetDlgItemText(IDC_SUGGESTED_SALES_REASON, "");
								m_nLastReasonSuggestionID = -1;
							}
						} else {
							CString strReason = VarString(varReason, "");
							long nLastReasonSuggestionID = VarLong(pRow->GetValue(esscSuggestionID), -1);

							if (nLastReasonSuggestionID != m_nLastReasonSuggestionID) {
								m_nLastReasonSuggestionID = nLastReasonSuggestionID;

								if (strReason.IsEmpty()) {
									if (pRow->GetValue(esscRootID).vt == VT_NULL)
										strReason = "<No reason specified>";
								}

								SetDlgItemText(IDC_SUGGESTED_SALES_REASON, strReason);
							}
						}
					}
				}
			}
		}
	} NxCatchAll("Error in CBillingSuggestedSalesDlg::OnMouseMove");

	CDialog::OnMouseMove(nFlags, point);
}

void CBillingSuggestedSalesDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	try {
		// (a.walling 2007-06-11 10:24) - PLID 14717 - Don't be fooled; this should be the final release of the datalist.
		m_dlList = NULL;
		m_dlListAll = NULL;

		DatalistSubclass::g_hwndSuggestedSales = NULL;

		DatalistSubclass::g_dlThisListWndProc = NULL;
		DatalistSubclass::g_dlAllListWndProc = NULL;
		DatalistSubclass::g_dlCurThisListWndProc = NULL;
		DatalistSubclass::g_dlCurAllListWndProc = NULL;

		CRect rRect;
		GetWindowRect(rRect);

		if ( (rRect.Width() > 50) && (rRect.Height() > 50) ) {
			// ensure a reasonable rect
			SetRemotePropertyInt("SuggestedSalesPopupWidth", rRect.Width(), 0, GetCurrentUserName());
			SetRemotePropertyInt("SuggestedSalesPopupHeight", rRect.Height(), 0, GetCurrentUserName());
			SetRemotePropertyInt("SuggestedSalesPopupTop", rRect.top, 0, GetCurrentUserName());
			SetRemotePropertyInt("SuggestedSalesPopupLeft", rRect.left, 0, GetCurrentUserName());
		} else {
			ASSERT(FALSE);
		}
	} NxCatchAll("Error saving suggested sales position");
}


/*static*/
BOOL CBillingSuggestedSalesDlg::CheckSuggestionsExist(CArray<long, long> &arServiceIDs, CWnd* pBillingDlg)
{
	try {
		BOOL bResult;

		// (a.walling 2007-05-21 12:11) - PLID 14717 - Need to SetSuggestionsExist even if there are no services (will be false)
		if (arServiceIDs.GetSize() == 0) 
			bResult = FALSE;
		else
			bResult = ReturnsRecords("SELECT TOP 1 ID FROM SuggestedSalesT WHERE MasterServiceID IN (%s)", ArrayAsString(arServiceIDs));
		
		if (pBillingDlg) {
			((CBillingDlg*)pBillingDlg)->SetSuggestionsExist(bResult);
		}
		
		return bResult;
	} NxCatchAll_NoParent("Error in SuggestionsExist"); // (a.walling 2014-05-05 13:32) - PLID 61945

	return FALSE;
}

void CBillingSuggestedSalesDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow) {
		RefreshIfNeeded(TRUE); // force if hidden
	}
}
