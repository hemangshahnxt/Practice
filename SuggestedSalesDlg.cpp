// SuggestedSalesDlg.cpp : implementation file
//

// (a.walling 2007-03-28 13:18) - PLID 25356 - Dialog to setup suggested sales

#include "stdafx.h"
#include "SuggestedSalesDlg.h"
#include "MultiSelectDlg.h"
#include "EMRSelectServiceDlg.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_REASON_LENGTH 1000;

/////////////////////////////////////////////////////////////////////////////
// CSuggestedSalesDlg dialog

using namespace NXDATALIST2Lib;


CSuggestedSalesDlg::CSuggestedSalesDlg(CWnd* pParent)
	: CNxDialog(CSuggestedSalesDlg::IDD, pParent),
	m_cptcodeChecker(NetUtils::CPTCodeT),
	m_productChecker(NetUtils::Products)
{
	//{{AFX_DATA_INIT(CSuggestedSalesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (a.wetta 2007-05-16 09:17) - PLID 25960 - Set the color to match the rest of the controls on the NexSpa tab
	m_nColor = GetNxColor(GNC_ADMIN, 0);
	m_nServiceID = -1;
}


void CSuggestedSalesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSuggestedSalesDlg)
	DDX_Control(pDX, IDC_ENABLE_DRAG_DROP_REASONS, m_btnEnableDragDropCopying);
	DDX_Control(pDX, IDC_ENABLE_DRAG_DROP_ORDERING, m_btnEnableDragDropOrdering);
	DDX_Control(pDX, IDC_ORDER_BY, m_nxibOrderBy);
	DDX_Control(pDX, IDC_COPY_SUGGESTED_TO, m_nxibCopyTo);
	DDX_Control(pDX, IDC_COPY_SUGGESTED_FROM, m_nxibCopyFrom);
	DDX_Control(pDX, IDC_REMOVE_SUGGESTED_PRODUCT, m_nxibRemoveProduct);
	DDX_Control(pDX, IDC_ADD_SUGGESTED_PRODUCT, m_nxibAddProduct);
	DDX_Control(pDX, IDC_SUGGESTION_MOVE_UP, m_nxibMoveUp);
	DDX_Control(pDX, IDC_SUGGESTION_MOVE_DOWN, m_nxibMoveDown);
	DDX_Control(pDX, IDC_SUGGESTED_COLOR, m_ncColor);
	DDX_Control(pDX, IDC_SUGGESTED_SALES_CAPTION, m_nxstaticSuggestedSalesCaption);
	DDX_Control(pDX, IDC_SUGGESTED_SALES_TIP, m_nxstaticSuggestedSalesTip);
	DDX_Control(pDX, IDC_SUGGESTED_SALES_TITLE, m_nxstaticSuggestedSalesTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSuggestedSalesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSuggestedSalesDlg)
	ON_BN_CLICKED(IDC_ADD_SUGGESTED_PRODUCT, OnAddSuggestedProduct)
	ON_BN_CLICKED(IDC_ENABLE_DRAG_DROP_REASONS, OnEnableDragDropReasons)
	ON_BN_CLICKED(IDC_REMOVE_SUGGESTED_PRODUCT, OnRemoveSuggestedProduct)
	ON_BN_CLICKED(IDC_ENABLE_DRAG_DROP_ORDERING, OnEnableDragDropOrdering)
	ON_BN_CLICKED(IDC_SUGGESTION_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_SUGGESTION_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_ORDER_BY, OnOrderBy)
	ON_BN_CLICKED(IDC_COPY_SUGGESTED_FROM, OnCopySuggestedFrom)
	ON_BN_CLICKED(IDC_COPY_SUGGESTED_TO, OnCopySuggestedTo)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSuggestedSalesDlg message handlers

BOOL CSuggestedSalesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_nxibMoveUp.AutoSet(NXB_UP);
		m_nxibMoveDown.AutoSet(NXB_DOWN);
		// (z.manning, 04/25/2008) - PLID 29566 - More button styles
		m_nxibAddProduct.AutoSet(NXB_NEW);
		m_nxibRemoveProduct.AutoSet(NXB_DELETE);
		m_nxibCopyTo.AutoSet(NXB_MODIFY);
		m_nxibCopyFrom.AutoSet(NXB_MODIFY);
		m_nxibOrderBy.AutoSet(NXB_MODIFY);

		m_brush.CreateSolidBrush(PaletteColor(m_nColor));

		m_ncColor.SetColor(m_nColor);

		extern CPracticeApp theApp;

		// (a.wetta 2007-05-21 13:49) - PLID 25960 - Bold the suggested sales title instead
		GetDlgItem(IDC_SUGGESTED_SALES_TITLE)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		
		m_bDragCopyEnabled = FALSE;
		m_bDragOrderEnabled = FALSE;
		m_bOrderModified = FALSE;
		m_bReady = FALSE;
		m_bBadDrag = FALSE;

		m_dlList = BindNxDataList2Ctrl(IDC_SUGGESTED_SALES, GetRemoteData(), false);
		m_dlServiceList = BindNxDataList2Ctrl(IDC_SUGGESTED_SALES_SERVICE_DROPDOWN, GetRemoteData(), true);	

		EnableDialogItems(TRUE, FALSE); // override cursel check, disable the items
		EnableDialogAll(FALSE);

		// (a.wetta 2007-05-16 08:53) - PLID 25960 - Set control positions for resizing purposes
		SetControlPositions();

	} NxCatchAll("Error initializing dialog in CSuggestedSalesDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CSuggestedSalesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSuggestedSalesDlg)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 7 /* RButtonUp */, OnRButtonUpSuggestedSales, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 9 /* EditingFinishing */, OnEditingFinishingSuggestedSales, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 10 /* EditingFinished */, OnEditingFinishedSuggestedSales, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 12 /* DragBegin */, OnDragBeginSuggestedSales, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 14 /* DragEnd */, OnDragEndSuggestedSales, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 28 /* CurSelWasSet */, OnCurSelWasSetSuggestedSales, VTS_NONE)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 18 /* RequeryFinished */, OnRequeryFinishedSuggestedSales, VTS_I2)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES_SERVICE_DROPDOWN, 1 /* SelChanging */, OnSelChangingSuggestedSalesServiceDropdown, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES_SERVICE_DROPDOWN, 2 /* SelChanged */, OnSelChangedSuggestedSalesServiceDropdown, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES, 8 /* EditingStarting */, OnEditingStartingSuggestedSales, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CSuggestedSalesDlg, IDC_SUGGESTED_SALES_SERVICE_DROPDOWN, 18 /* RequeryFinished */, OnRequeryFinishedSuggestedSalesServiceDropdown, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSuggestedSalesDlg::OnRButtonUpSuggestedSales(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	CMenu* mnuPopup = NULL;

	try {		
		{
			IRowSettingsPtr pRow(lpRow);

			if (pRow) {
				m_dlList->PutCurSel(pRow);
			}
		}

		CMenu mnu;
		mnu.LoadMenu(IDR_GENERICMENU);

		mnuPopup = mnu.GetSubMenu(0);

		POINT p;
		GetCursorPos(&p);

		{
			IRowSettingsPtr pCurSelRow = m_dlList->GetCurSel();

			if (pCurSelRow) {
				mnuPopup->EnableMenuItem(ID_REMOVE_SUGGESTION, MF_ENABLED);
			} else {
				mnuPopup->EnableMenuItem(ID_REMOVE_SUGGESTION, MF_DISABLED);
			}
		}

		DWORD dwSelection = mnuPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, p.x, p.y, this);

		if (dwSelection == ID_REMOVE_SUGGESTION) {
			RemoveSelectedProduct(); // this will throw any exceptions
		}
	} NxCatchAll("Error in CSuggestedSalesDlg::OnRButtonUpSuggestedSales");
}

void CSuggestedSalesDlg::OnEditingStartingSuggestedSales(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (a.wetta 2007-05-24 13:58) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite, FALSE, 0, TRUE, TRUE)) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll("Error in CSalesSetupDlg::OnSelChangedSaleList");	
}

void CSuggestedSalesDlg::OnEditingFinishingSuggestedSales(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (*pbCommit == FALSE) 
			return; // they are cancelling the edit

		// (a.wetta 2007-05-24 14:07) - PLID 25394 - If they have write permission with password, have them enter the password
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			*pbContinue = TRUE;
			*pbCommit = FALSE;
			return;
		}

		const long nMaxChars = MAX_REASON_LENGTH;
		long nLength = strlen(strUserEntered);

		if (nLength > nMaxChars) {
			*pbCommit = FALSE;

			int nResult = MessageBox(FormatString("Notes longer than %li characters cannot be stored in this field.  Would you like to continue?  If you answer yes, the note will be truncated to %li characters", nMaxChars, nMaxChars), "Invalid Field Length", MB_YESNOCANCEL);

			if (nResult == IDYES) {
				// truncate the text
				CString strNewReason = CString(strUserEntered).Left(nMaxChars);
				*pvarNewValue = _variant_t(strNewReason).Detach();
				*pbCommit = TRUE;
				*pbContinue = TRUE;
			} else if (nResult == IDNO) {
				//do not continue yet
				*pbContinue = FALSE;
				//Select the extraneous characters
				m_dlList->SetEditingHighlight(nMaxChars, nLength, FALSE);
			} else if (nResult == IDCANCEL) {
				// cancel the edit entirely
				*pbContinue = TRUE;
			} else { ASSERT(FALSE); }
		}
	} NxCatchAll("Error in CSuggestedSalesDlg::OnEditingFinishingSuggestedSales");
}

void CSuggestedSalesDlg::OnEditingFinishedSuggestedSales(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (bCommit) {
			IRowSettingsPtr pRow(lpRow);

			if (pRow) {
				if (nCol == essReason) {
					long nSuggestionID = VarLong(pRow->GetValue(essSuggestionID), -1);

					if (nSuggestionID > 0) {
						if (VarString(varOldValue, "") != VarString(varNewValue, "")) {
							ExecuteSql("UPDATE SuggestedSalesT SET Reason = '%s' WHERE ID = %li", _Q(VarString(varNewValue, "")), nSuggestionID);
						}
					}
				}
			}

		}
	} NxCatchAll("Error updating reason in CSuggestedSalesDlg::OnEditingFinishedSuggestedSales");
}

void CSuggestedSalesDlg::OnAddSuggestedProduct() 
{
	CMenu* mnuPopup = NULL;

	try {
		CMenu mnu;
		mnu.LoadMenu(IDR_GENERICMENU);

		mnuPopup = mnu.GetSubMenu(1);

		POINT p;
		GetCursorPos(&p);

		DWORD dwSelection = mnuPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, p.x, p.y, this);

		if (dwSelection == ID_ADD_PRODUCT_SUGGESTION) {
			AddSuggestion(eastProduct); // this will throw any exceptions
		} else if (dwSelection == ID_ADD_CODE_SUGGESTION) {
			AddSuggestion(eastCode); // this will throw any exceptions
		}

	} NxCatchAll("Error adding suggestion!");
}

// creates a multiselect box to choose which items to include
// in the list, using eastType (product, service code, etc)
void CSuggestedSalesDlg::AddSuggestion(EAddSuggestionType eastType) 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		if (m_dlList->IsRequerying()) {
			// wait until we are done!
			m_dlList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}

		CArray<long, long> arCurrentServiceIDs;

		arCurrentServiceIDs.SetSize(m_dlList->GetRowCount());

		IRowSettingsPtr pCurRow = m_dlList->GetFirstRow();
		int ix = 0;
		while (pCurRow) {
			arCurrentServiceIDs.SetAtGrow(ix, VarLong(pCurRow->GetValue(essServiceID)));
			ix++;
			pCurRow = pCurRow->GetNextRow();
		}
		ASSERT(m_dlList->GetRowCount() == arCurrentServiceIDs.GetSize());

		CStringArray saExtraFields, saExtraNames;

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "");
		dlg.SkipIDsInArray(arCurrentServiceIDs);

		CString strFrom, strWhere, strCaption;

		if (eastType == eastProduct) {
			strFrom = "ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID";
			strWhere = "Active = 1";
			strCaption = "Select products to suggest:";
			dlg.SetSizingConfigRT("ProductT");
		} else if (eastType == eastCode) {
			strFrom = "ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID";
			strWhere = "Active = 1 AND ServiceT.ID NOT IN (SELECT ID FROM ProductT)";
			strCaption = "Select service codes to suggest:";

			saExtraFields.Add("CPTCodeT.Code + ' ' + CPTCodeT.SubCode");
			saExtraNames.Add("Code");
			dlg.SetSizingConfigRT("CPTCodeT");
		} else { ASSERT(FALSE); }
		
		saExtraFields.Add("Price");
		saExtraNames.Add("Price");

		saExtraFields.Add("CategoriesT.Name");
		saExtraNames.Add("Category");

		if (IDOK == dlg.Open(
			strFrom,
			strWhere,
			"ServiceT.ID",
			"ServiceT.Name",
			strCaption,
			1,
			0xFFFFFFFF,
			&saExtraFields,
			&saExtraNames)) 
		{
			CArray<long, long> arSelectedServiceIDs;
			dlg.FillArrayWithIDs(arSelectedServiceIDs);
			
			if (arSelectedServiceIDs.GetSize() > 0) {
				//The following line will ensure there are no dupes by clearing them out first, but because we are skipping service IDs
				// that already exist in our list, this should be unnecessary. If there end up being duplicates, the query will fail
				// anyway since it violates our UNIQUE index.
				ExecuteSql("INSERT INTO SuggestedSalesT(MasterServiceID, ServiceID) SELECT %li, ID FROM ServiceT WHERE ID IN (%s)", m_nServiceID, ArrayAsString(arSelectedServiceIDs));
			}
		}
	} NxCatchAll("Error adding suggested products in CSuggestedSalesDlg::OnAddSuggestedProduct");

	ReloadAll();
	m_bOrderModified = TRUE; // we set this here, since the dialog will save the indexes first, then reload with all the
	// new items added to the bottom (with default index -1). By setting this flag, the indexes will be guaranteed to be
	// saved again when exiting.
}

void CSuggestedSalesDlg::OnDragBeginSuggestedSales(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags) 
{
	try {
		if (!m_bReady) {
			// datalist is still requerying...
			*pbShowDrag = FALSE;
			m_bBadDrag = TRUE;
			return;
		}

		if (m_bDragCopyEnabled) {
			if (nCol == essReason) {
				*pbShowDrag = TRUE;
			} else { 
				*pbShowDrag = FALSE;
			}
		} else if (m_bDragOrderEnabled) {
			*pbShowDrag = TRUE;
		} else {
			*pbShowDrag = FALSE;
		}
	} NxCatchAll("Error in CSuggestedSalesDlg::OnDragBeginSuggestedSales");
}

// support drag-ordering, and drag-copying of reason text.
void CSuggestedSalesDlg::OnDragEndSuggestedSales(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags) 
{
	try {
		if (m_bBadDrag) {
			// dragging began before the DL was finished requerying, so we are discarding this one.
			m_bBadDrag = FALSE;
			return;
		}
		if ( (nFlags == defFinished) && m_bReady) {
			// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
			if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
				return;
			}

			if (m_bDragCopyEnabled) {
				if (nFromCol == essReason) {
					IRowSettingsPtr pFromRow(lpFromRow);
					IRowSettingsPtr pRow(lpRow);

					if (pRow) {
						if (pFromRow) {
							long nSuggestionID = VarLong(pRow->GetValue(essSuggestionID), -1);
							if (nSuggestionID > 0) {
								_variant_t varFromValue = pFromRow->GetValue(essReason);
								
								CString strFromValue = VarString(varFromValue, "");

								if (strFromValue.GetLength() > 0) {
									// copying over a blank reason is probably a mistake, so we'll disallow any changes.
									pRow->PutValue(essReason, varFromValue);
									ExecuteSql("UPDATE SuggestedSalesT SET Reason = '%s' WHERE ID = %li", _Q(strFromValue), nSuggestionID);
								}
							}
						}
					}
				}
			} else if (m_bDragOrderEnabled) {
				IRowSettingsPtr pFromRow(lpFromRow);
				IRowSettingsPtr pDestRow(lpRow);

				if (pDestRow) {
					if (pFromRow) {
						// just to be safe
						if (pDestRow->IsSameRow(pFromRow)) {
							return; // quietly
						}

						// dest row precedes the from row, so iterate downwards
						IRowSettingsPtr pCurRow = pDestRow;
						IRowSettingsPtr pSwapRow;
						BOOL bGetNext = m_dlList->IsRowEarlierInList(pDestRow, pFromRow);
						do {
							// the do {...} while structure has a bad rap, but it really is the best
							// choice for this situation. pDestRow & pFromRow are guaranteed to be OK,
							// and pCurRow = pDestRow in the first iteration, and pSwapRow will either
							// be the next or previous row, which must exist, because if there was only
							// one row, then pDestRow and pFromRow would be the same (but the dragend
							// event is never even fired, since you cannot drag a row onto itself.)
							if (bGetNext) {
								pSwapRow = pCurRow->GetNextRow();
							} else {
								pSwapRow = pCurRow->GetPreviousRow();
							}

							_variant_t varFromIndex = pCurRow->GetValue(essOrderIndex);
							_variant_t varToIndex = pSwapRow->GetValue(essOrderIndex);

							pSwapRow->PutValue(essOrderIndex, varFromIndex);
							pCurRow->PutValue(essOrderIndex, varToIndex);

							pCurRow = pSwapRow;
						} while (!pFromRow->IsSameRow(pSwapRow));

						m_bOrderModified = TRUE;

						m_dlList->Sort();
						m_dlList->PutCurSel(pFromRow);

						SaveIndexes();
					}
				}
			}
		}
	} NxCatchAll("Error in CSuggestedSalesDlg::OnDragEndSuggestedSales");
}

// recalculates indexes for all items in the list, does not write today. for future use.
void CSuggestedSalesDlg::RecalcIndexes()
{
	try {
		if (m_dlList->IsRequerying())
			m_dlList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		long nMaxIndex = m_dlList->GetRowCount();
		long nCurIndex = nMaxIndex;
		IRowSettingsPtr pRow = m_dlList->GetFirstRow();

		while (pRow) {
			pRow->PutValue(essOrderIndex, nCurIndex);

			nCurIndex--;
			pRow = pRow->GetNextRow();
		}
	} NxCatchAll("Error recalculating indexes.");
}

// recalculates indexes and saves all items in the list.
void CSuggestedSalesDlg::SaveIndexes()
{
	try {
		CWaitCursor cws;
		if (m_bOrderModified) {
			if (m_dlList->IsRequerying())
				m_dlList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

			CString strSql, strSqlBatch = BeginSqlBatch();

			long nMaxIndex = m_dlList->GetRowCount();
			long nCurIndex = nMaxIndex;
			IRowSettingsPtr pRow = m_dlList->GetFirstRow();

			while (pRow) {
				long nID = VarLong(pRow->GetValue(essSuggestionID), -1);
				if (nID > 0) {
					AddStatementToSqlBatch(strSqlBatch, "UPDATE SuggestedSalesT SET OrderIndex = %li WHERE ID = %li", nCurIndex, nID);
				}

				nCurIndex--;
				pRow = pRow->GetNextRow();
			}

			if (nCurIndex != nMaxIndex) // this means that there was at least one iteration
				ExecuteSqlBatch(strSqlBatch);

			m_bOrderModified = FALSE;
		}
	} NxCatchAll("Error saving indexes");
}

// reloads everything. uses m_bReady.
void CSuggestedSalesDlg::ReloadAll()
{
	try {
		if (m_nServiceID != -1) {
			m_bReady = FALSE;
			SaveIndexes();
			m_dlList->Requery();
		}
	} NxCatchAll("Error calling requery in CSuggestedSalesDlg::ReloadAll");
}

void CSuggestedSalesDlg::OnRemoveSuggestedProduct() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		RemoveSelectedProduct();
	} NxCatchAll("Error removing product in CSuggestedSalesDlg::OnRemoveSuggestedProduct");
}

void CSuggestedSalesDlg::OnEnableDragDropReasons() 
{
	try {
		m_bDragCopyEnabled = IsDlgButtonChecked(IDC_ENABLE_DRAG_DROP_REASONS);
		if (m_bDragCopyEnabled) { // only one of these should be checked
			CheckDlgButton(IDC_ENABLE_DRAG_DROP_ORDERING, FALSE);
			m_bDragOrderEnabled = FALSE;
		}
	} NxCatchAll("Error accessing dialog checkbox");
}

void CSuggestedSalesDlg::OnEnableDragDropOrdering() 
{
	try {
		m_bDragOrderEnabled = IsDlgButtonChecked(IDC_ENABLE_DRAG_DROP_ORDERING);
		if (m_bDragOrderEnabled) { // only one of these should be checked
			CheckDlgButton(IDC_ENABLE_DRAG_DROP_REASONS, FALSE);
			m_bDragCopyEnabled = FALSE;
		}
	} NxCatchAll("Error accessing dialog checkbox");
}

void CSuggestedSalesDlg::OnMoveDown() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		IRowSettingsPtr pRow = m_dlList->GetCurSel();

		if (pRow) {
			IRowSettingsPtr pNextRow = pRow->GetNextRow();

			if (pNextRow) {
				_variant_t varFromIndex = pRow->GetValue(essOrderIndex);
				_variant_t varToIndex = pNextRow->GetValue(essOrderIndex);

				pRow->PutValue(essOrderIndex, varToIndex);
				pNextRow->PutValue(essOrderIndex, varFromIndex);

				m_bOrderModified = TRUE;

				m_dlList->Sort();
				EnableDialogItems();

				SaveIndexes();
			}
		}
	} NxCatchAll("Error moving row index down");
}

void CSuggestedSalesDlg::OnMoveUp() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		IRowSettingsPtr pRow = m_dlList->GetCurSel();

		if (pRow) {
			IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();

			if (pPrevRow) {
				_variant_t varFromIndex = pRow->GetValue(essOrderIndex);
				_variant_t varToIndex = pPrevRow->GetValue(essOrderIndex);

				pRow->PutValue(essOrderIndex, varToIndex);
				pPrevRow->PutValue(essOrderIndex, varFromIndex);

				m_bOrderModified = TRUE;

				m_dlList->Sort();
				EnableDialogItems();

				SaveIndexes();
			}
		}
	} NxCatchAll("Error moving row index up");
}

// enabled dialog items depending on cursel,
// or using bShow if bOverride is true (only in initdialog currently).
void CSuggestedSalesDlg::EnableDialogItems(BOOL bOverride /* = FALSE*/, BOOL bShow /* = FALSE*/)
{
	try {
		// (a.wetta 2007-05-24 14:09) - PLID 25394 - If they don't have write permission, don't enable the edit controls
		BOOL bWritePermission = CheckCurrentUserPermissions(bioSuggestedSales, sptWrite, FALSE, 0, TRUE, TRUE);

		if (!bOverride) {
			// if override is not set, then enable based on cursel
			IRowSettingsPtr pCurSelRow = m_dlList->GetCurSel();

			if (pCurSelRow) {
				// there is a selection
				GetDlgItem(IDC_REMOVE_SUGGESTED_PRODUCT)->EnableWindow(bWritePermission && TRUE);

				IRowSettingsPtr pPrevRow = pCurSelRow->GetPreviousRow();
				IRowSettingsPtr pNextRow = pCurSelRow->GetNextRow();

				GetDlgItem(IDC_SUGGESTION_MOVE_UP)->EnableWindow(bWritePermission && (pPrevRow != NULL));
				GetDlgItem(IDC_SUGGESTION_MOVE_DOWN)->EnableWindow(bWritePermission && (pNextRow != NULL));
			} else {
				// there is no selection
				GetDlgItem(IDC_SUGGESTION_MOVE_UP)->EnableWindow(FALSE);
				GetDlgItem(IDC_SUGGESTION_MOVE_DOWN)->EnableWindow(FALSE);
				GetDlgItem(IDC_REMOVE_SUGGESTED_PRODUCT)->EnableWindow(FALSE);
			}
		} else {
			GetDlgItem(IDC_SUGGESTION_MOVE_UP)->EnableWindow(bWritePermission && bShow);
			GetDlgItem(IDC_SUGGESTION_MOVE_DOWN)->EnableWindow(bWritePermission && bShow);
			GetDlgItem(IDC_REMOVE_SUGGESTED_PRODUCT)->EnableWindow(bWritePermission && bShow);
		}
	} NxCatchAll("Error enabling dialog items!");
}

void CSuggestedSalesDlg::OnOrderBy() 
{
	//ID_SUGGESTION_ORDER_BY_NAME_AND_TYPE
	CMenu* mnuPopup = NULL;

	try {
		CMenu mnu;
		mnu.LoadMenu(IDR_GENERICMENU);

		mnuPopup = mnu.GetSubMenu(2);

		POINT p;
		GetCursorPos(&p);

		DWORD dwSelection = mnuPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, p.x, p.y, this);

		if (dwSelection == ID_SUGGESTION_ORDER_BY_NAME) {
			OrderByName();
		} else if (dwSelection == ID_SUGGESTION_ORDER_BY_NAME_AND_TYPE) {
			OrderByNameAndType();
		}

	} NxCatchAll("Error ordering items");
}

// order all items in the list by name, then saves.
void CSuggestedSalesDlg::OrderByName() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		if (IDYES == MessageBox("All ordering will be overwritten and sorted by name. Would you like to continue?", "Practice", MB_YESNO | MB_ICONQUESTION))
		{
			IColumnSettingsPtr pIndexCol = m_dlList->GetColumn(essOrderIndex);
			IColumnSettingsPtr pNameCol = m_dlList->GetColumn(essName);

			short sIndexPriority = pIndexCol->GetSortPriority();
			short sNamePriority = pNameCol->GetSortPriority();

			pIndexCol->PutSortPriority(sNamePriority);
			pNameCol->PutSortPriority(sIndexPriority);

			m_dlList->Sort();

			m_bOrderModified = TRUE; // the order has modified
			SaveIndexes(); // this will rebuild our indexes and save.

			// restore sort criteria
			pNameCol->PutSortPriority(sNamePriority);
			pIndexCol->PutSortPriority(sIndexPriority);

			ReloadAll();
		}
	} NxCatchAll("Error ordering by name!");
}

// order all items in the list by type and name, then saves.
void CSuggestedSalesDlg::OrderByNameAndType() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		if (IDYES == MessageBox("All ordering will be overwritten and sorted by type, then name. Would you like to continue?", "Practice", MB_YESNO | MB_ICONQUESTION))
		{
			IColumnSettingsPtr pIndexCol = m_dlList->GetColumn(essOrderIndex);
			IColumnSettingsPtr pNameCol = m_dlList->GetColumn(essName);
			IColumnSettingsPtr pTypeCol = m_dlList->GetColumn(essType);

			short sIndexPriority = pIndexCol->GetSortPriority(); // 0
			short sNamePriority = pNameCol->GetSortPriority(); // 1
			short sTypePriority = pTypeCol->GetSortPriority(); // -1

			pIndexCol->PutSortPriority(sTypePriority);
			// pNameCol->PutSortPriority(); // stays the same
			pTypeCol->PutSortPriority(sIndexPriority);

			m_dlList->Sort();

			m_bOrderModified = TRUE; // the order has modified
			SaveIndexes(); // this will rebuild our indexes and save.

			// restore sort criteria
			pIndexCol->PutSortPriority(sIndexPriority);
			pTypeCol->PutSortPriority(sTypePriority);	
			
			ReloadAll();
		}
	} NxCatchAll("Error ordering by name and type!");
}


void CSuggestedSalesDlg::OnRequeryFinishedSuggestedSales(short nFlags) 
{
	RecalcIndexes(); // catches its own exceptions

	try {
		// we want to try to find an inactive item so we can tell the user.
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->FindByColumn(essForeColor, _variant_t((long)10526880, VT_I4), NULL, VARIANT_FALSE);

		if (pRow) {
			SetDlgItemText(IDC_SUGGESTED_SALES_TIP, "NOTE: Some inactive items are listed as suggestions, and appear as gray. "
				"These will not be visible to the user, and may safely be removed.");
		}
	} NxCatchAll("Error in OnRequeryFinishedSuggestedSales");

	m_bReady = TRUE;
}

// copies current suggestions from selected services/products
void CSuggestedSalesDlg::OnCopySuggestedFrom() 
{
	// huzzah, more select dialogs
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		CEMRSelectServiceDlg dlg(this);
		dlg.m_bGeneric = TRUE;

		if (m_dlList->GetRowCount() > 0) {
			DontShowMeAgain(this, "Copying suggestions from another service will not overwrite suggestions that already exist for this service. The copied suggestions will be added instead.", "SuggestedSalesCopyFrom", "");
		}

		if (IDOK == dlg.DoModal())
		{
			long nFromServiceID = dlg.m_ServiceID;

			ADODB::_RecordsetPtr prs = CreateRecordset("SELECT Count(*) AS NumRecords, Max(OrderIndex) as MaxIndex FROM SuggestedSalesT WHERE MasterServiceID = %li", nFromServiceID);
			long nNumRecords = AdoFldLong(prs, "NumRecords", 0);

			if (nNumRecords > 0) {
				long nMaxIndex = AdoFldLong(prs, "MaxIndex", 0);

				// (a.walling 2007-03-27 15:14) - PLID 25356 - Copy over the suggestions, ensure not to duplicate any serviceIDs.
				ExecuteSql("INSERT INTO SuggestedSalesT(MasterServiceID, ServiceID, Reason, OrderIndex) "
					"SELECT %li, ServiceID, Reason, -(%li-OrderIndex+1) FROM SuggestedSalesT "
					"WHERE MasterServiceID = %li AND ServiceID NOT IN "
					"(SELECT ServiceID FROM SuggestedSalesT WHERE MasterServiceID = %li)",
					m_nServiceID,
					nMaxIndex,
					nFromServiceID,
					m_nServiceID);

				ReloadAll();
				m_bOrderModified = TRUE; // force the indexing to be saved
			}
		}
	} NxCatchAll("Error copying suggestions from service!");
}

// copies current suggestions to selected services/products
void CSuggestedSalesDlg::OnCopySuggestedTo() 
{
	try {
		// (a.wetta 2007-05-24 14:07) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioSuggestedSales, sptWrite)) {
			return;
		}

		// first, ensure that WE are saved.
		
		m_bOrderModified = TRUE; // force the indexing to be saved
		SaveIndexes();

		if (m_dlList->GetRowCount() > 0) {
			DontShowMeAgain(this, "Copying suggestions to another service will not overwrite suggestions that already exist. The copied suggestions will be added instead.", "SuggestedSalesCopyTo", "");
		}

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ServiceT");

		CArray<long, long> arThisService;
		arThisService.Add(m_nServiceID);

		CStringArray saExtraFields, saExtraNames;

		saExtraFields.Add("CPTCodeT.Code + ' ' + CPTCodeT.SubCode");
		saExtraNames.Add("Code");

		saExtraFields.Add("CASE WHEN ProductT.ID IS NULL THEN 'Service' ELSE 'Product' END AS Type");
		saExtraNames.Add("Type");

		dlg.SkipIDsInArray(arThisService);
		if (IDOK == dlg.Open(
			"ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID",
			"ServiceT.Active = 1",
			"ServiceT.ID",
			"ServiceT.Name",
			"Choose one or more services or products to copy suggestions to:",
			0, 0xFFFFFFFF,
			&saExtraFields,
			&saExtraNames))
		{
			CWaitCursor cws;
			CArray<long, long> arSelectedServices;

			dlg.FillArrayWithIDs(arSelectedServices);

			if (arSelectedServices.GetSize() > 0) {
				CString strBatchSql = BeginSqlBatch();

				long nFromServiceID = m_nServiceID;

				ADODB::_RecordsetPtr prs = CreateRecordset(
					"SELECT Count(*) AS NumRecords, Max(OrderIndex) as MaxIndex "
					"FROM SuggestedSalesT "
					"WHERE MasterServiceID = %li ", nFromServiceID);

				if (prs->eof) {ASSERT(FALSE); return;}
			
				long nNumRecords = AdoFldLong(prs, "NumRecords", 0);
				long nMaxIndex = AdoFldLong(prs, "MaxIndex", 0);

				if (nNumRecords > 0) {
					CString strSqlBatch = BeginSqlBatch();

					for (int i = 0; i < arSelectedServices.GetSize(); i++) {
						// (a.walling 2007-03-27 15:14) - PLID 25356 - Copy over the suggestions, ensure not to duplicate any serviceIDs.

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO SuggestedSalesT(MasterServiceID, ServiceID, Reason, OrderIndex) "
							"SELECT %li, ServiceID, Reason, -(%li-OrderIndex+1) FROM SuggestedSalesT "
							"WHERE MasterServiceID = %li AND ServiceID NOT IN "
							"(SELECT ServiceID FROM SuggestedSalesT WHERE MasterServiceID = %li)",
							arSelectedServices[i],
							nMaxIndex,
							nFromServiceID,
							arSelectedServices[i]);

						// now we have to update the indexes to be positive.
						AddStatementToSqlBatch(strSqlBatch, "UPDATE SuggestedSalesT "
							"SET OrderIndex = OrderIndex + "
								"(SELECT ABS(MIN(OrderIndex))+1 FROM SuggestedSalesT WHERE MasterServiceID = %li) "
							"WHERE MasterServiceID = %li", arSelectedServices[i], arSelectedServices[i]);
					}

					ExecuteSqlBatch(strSqlBatch);
				}
			}
		}

	} NxCatchAll("Error copying suggestions to service(s)!");
}


HBRUSH CSuggestedSalesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	if(nCtlColor == CTLCOLOR_STATIC
		&& (pWnd->GetDlgCtrlID() == IDC_ENABLE_DRAG_DROP_REASONS
		|| pWnd->GetDlgCtrlID() == IDC_ENABLE_DRAG_DROP_ORDERING
		|| pWnd->GetDlgCtrlID() == IDC_SUGGESTED_SALES_CAPTION
		|| pWnd->GetDlgCtrlID() == IDC_SUGGESTED_SALES_TIP
		|| pWnd->GetDlgCtrlID() == IDC_STATIC
		|| pWnd->GetDlgCtrlID() == IDC_SUGGESTED_SALES_TITLE)) { // (a.wetta 2007-05-21 14:16) - PLID 25960 - Also color the title

		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(m_nColor));
		return m_brush;
	}

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}


//---- Utility functions -- exceptions thrown to caller

void CSuggestedSalesDlg::OnCurSelWasSetSuggestedSales() 
{
	EnableDialogItems();	// this function catches its own exceptions
}

// removes cursel from list and data
void CSuggestedSalesDlg::RemoveSelectedProduct() 
{
	// utility function -- doesn't catch its own exceptions, throws to caller.

	IRowSettingsPtr pRow = m_dlList->GetCurSel();

	if (pRow) {
		m_bOrderModified = TRUE;
		long nSuggestionID = VarLong(pRow->GetValue(essSuggestionID));
		m_dlList->RemoveRow(pRow);
		ExecuteSql("DELETE FROM SuggestedSalesT WHERE ID = %li", nSuggestionID);

		// well that was easy.
	}
}

// (a.wetta 2007-05-16 08:50) - PLID 25960 - Added UpdateView function for NexSpa tab
void CSuggestedSalesDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
	try {
		BOOL bCheckForCode = FALSE;
		if (m_cptcodeChecker.Changed() || m_productChecker.Changed()) {
			bCheckForCode = TRUE;
			m_dlServiceList->Requery();
			m_dlList->Clear();

			EnableDialogItems(TRUE, FALSE); // override cursel check, disable the items
			EnableDialogAll(FALSE);

			GetDlgItem(IDC_SUGGESTED_SALES)->EnableWindow(FALSE);
			GetDlgItem(IDC_SUGGESTED_SALES_SERVICE_DROPDOWN)->EnableWindow(FALSE);

			/*long nResult = m_dlServiceList->TrySetSelByColumn(esssdcServiceID, _variant_t(m_nServiceID));

			if (nResult == NXDATALIST2Lib::sriNoRow) {
				m_nServiceID = -1;
			} else if (nResult == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
				// hooray!
			}*/
		}
	}NxCatchAll("Error in CSuggestedSalesDlg::UpdateView");
}

// (a.wetta 2007-05-16 08:50) - PLID 25960 - Make the dialog sizable
void CSuggestedSalesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	SetControlPositions();
	Invalidate();	
}


// (a.walling 2007-05-21 10:02) - PLID 25356 - Enable/disable the dialog if needed
void CSuggestedSalesDlg::EnableDialogAll(BOOL bEnable) {

	GetDlgItem(IDC_SUGGESTED_SALES)->EnableWindow(bEnable);

	// (a.wetta 2007-05-24 14:09) - PLID 25394 - If they don't have write permission, don't enable the edit controls
	BOOL bWritePermission = CheckCurrentUserPermissions(bioSuggestedSales, sptWrite, FALSE, 0, TRUE, TRUE);
	GetDlgItem(IDC_ADD_SUGGESTED_PRODUCT)->EnableWindow(bWritePermission && bEnable);
	GetDlgItem(IDC_COPY_SUGGESTED_FROM)->EnableWindow(bWritePermission && bEnable);
	GetDlgItem(IDC_COPY_SUGGESTED_TO)->EnableWindow(bWritePermission && bEnable);
	GetDlgItem(IDC_ORDER_BY)->EnableWindow(bWritePermission && bEnable);
	GetDlgItem(IDC_ENABLE_DRAG_DROP_ORDERING)->EnableWindow(bWritePermission && bEnable);
	GetDlgItem(IDC_ENABLE_DRAG_DROP_REASONS)->EnableWindow(bWritePermission && bEnable);
}

void CSuggestedSalesDlg::OnSelChangingSuggestedSalesServiceDropdown(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		} else {
			SaveIndexes();
		}
	} NxCatchAll("Error in OnSelChangingSuggestedSalesServiceDropdown");
}

// (a.walling 2007-05-21 10:02) - PLID 25356 - Added a service dropdown so it exists as an embedded dialog on the NexSpa tab
void CSuggestedSalesDlg::OnSelChangedSuggestedSalesServiceDropdown(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		m_bOrderModified = FALSE;
		m_bBadDrag = FALSE;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {
			m_nServiceID = VarLong(pRow->GetValue(esssdcServiceID), -1);
			CString strWhere = FormatString("SuggestedSalesT.MasterServiceID = %li", m_nServiceID);

			m_dlList->PutWhereClause(static_cast<LPCTSTR>(strWhere));

			EnableDialogAll(TRUE);
			EnableDialogItems();
		} else {
			m_nServiceID = -1;
			EnableDialogAll(FALSE);
		}

		ReloadAll(); // m_bOrderModified MUST be FALSE!
	} NxCatchAll("Error in OnSelChangedSuggestedSalesServiceDropdown");
}

void CSuggestedSalesDlg::OnRequeryFinishedSuggestedSalesServiceDropdown(short nFlags) 
{
	try {
		// this was causing an access violation in the datalist when using trysetsel
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlServiceList->SetSelByColumn(esssdcServiceID, _variant_t(m_nServiceID));	

		// we have to ensure it exists here.

		if (pRow) {
			EnableDialogAll(TRUE);
			EnableDialogItems();
		} else {
			m_nServiceID = -1;
		}
			
		ReloadAll();
		
		GetDlgItem(IDC_SUGGESTED_SALES)->EnableWindow(TRUE);
		GetDlgItem(IDC_SUGGESTED_SALES_SERVICE_DROPDOWN)->EnableWindow(TRUE);

	} NxCatchAll("Error resetting selection in CSuggestedSalesDlg::OnRequeryFinishedSuggestedSalesServiceDropdown");
}
