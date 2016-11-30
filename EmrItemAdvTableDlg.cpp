// EmrItemAdvTableDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EMN.h"
#include "EMR.h"
#include "EmrItemAdvTableDlg.h"
#include "MultiSelectDlg.h"
#include "NxExpression.h"
#include "EmrTableEditCalculatedFieldDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "WindowlessUtils.h"
#include "EMRTopic.h"
#include "EMNDetail.h"
#include "NxCache.h"
#include "NxAutoQuantum.h"


// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

using namespace ADODB;

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvTableDlg dialog

CEmrItemAdvTableDlg::CEmrItemAdvTableDlg(class CEMNDetail *pDetail)
	: CEmrItemAdvDlg(pDetail)
{
	//{{AFX_DATA_INIT(CEmrItemAdvTableDlg)
	//}}AFX_DATA_INIT

	m_bIsActiveCurrentMedicationsTable = FALSE;
	m_bIsActiveAllergiesTable = FALSE;
	// (c.haag 2007-08-20 14:31) - PLID 27126 - By default, when we call
	// ReflectCurrentState, it should populate the datalist.
	m_ReflectCurrentStateDLHint = eRCS_FullDatalistUpdate;
	//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
	m_pEditingFinishedRow = NULL;
	m_nEditingFinishedCol = -1;
}

CEmrItemAdvTableDlg::~CEmrItemAdvTableDlg()
{
	// (c.haag 2008-01-15 17:40) - PLID 17936 - Clear out all dropdown source-related
	// content data
	ClearDropdownSourceInfoMap();
}

// (z.manning 2011-10-11 11:14) - PLID 42061 - Added stamp ID
CString CEmrItemAdvTableDlg::GetDropdownSource(long nColumnID, const long nStampID)
{
	// (z.manning 2011-03-11) - PLID 42778 - Moved this logic to the base class
	return CEmrItemAdvTableBase::GetDropdownSource(nColumnID, m_pDetail, TRUE, nStampID);
}

// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
BEGIN_EVENTSINK_MAP(CEmrItemAdvTableDlg, CEmrItemAdvDlg)
    //{{AFX_EVENTSINK_MAP(CEmrItemAdvTableDlg)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 10 /* EditingFinished */, OnEditingFinishedTable, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 5 /* LButtonUp */, OnLButtonUpTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 6 /* RButtonDown */, OnRButtonDownTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 32 /* ShowContextMenu */, OnShowContextMenuTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedTable, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 9 /* EditingFinishing */, OnEditingFinishingTable, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrItemAdvTableDlg, TABLE_IDC, 8 /* EditingStarting */, OnEditingStartingTable, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CEmrItemAdvTableDlg, CEmrItemAdvDlg)
	//{{AFX_MSG_MAP(CEmrItemAdvTableDlg)
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(NXM_EMR_ADD_NEW_DROPDOWN_COLUMN_SELECTION, OnAddNewDropdownColumnSelection)
	ON_MESSAGE(NXM_START_EDITING_EMR_TABLE, OnStartEditingEMRTable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvTableDlg message handlers

void CEmrItemAdvTableDlg::ReflectCurrentContent()
{
	try
	{
		// (a.walling 2013-07-23 21:13) - PLID 57685 - Ensure this item is in the Nx::Cache
		if (m_pDetail) {
			Nx::Cache::Emr::EnsureEmrInfo(m_pDetail->m_nEMRInfoID);
		}
		// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Use the detail label text with special modifiers for onscreen presentation
		CString strLabel = GetLabelText(TRUE);
		strLabel.Replace("&", "&&");

		// (z.manning 2011-05-05 15:09) - PLID 43568 - Moved this near the top of the function.
		// Create the datalist
		// (c.haag 2008-10-16 10:13) - PLID 31700 - Ensures the table is valid and has all rows and columns populated
		EnsureTableObject(m_pDetail, this, strLabel, m_pDetail->GetReadOnly());

		// (z.manning 2011-05-05 15:10) - PLID 43568 - Set redraw to false while we recreate the datalist.
		SetDatalistRedraw(FALSE);

		// Clear out anything that's already there
		DestroyContent();
		
		// Create everything the parent wants
		CEmrItemAdvDlg::ReflectCurrentContent();

		CalcComboSql();

		// (a.wetta 2007-02-07 14:23) - PLID 24564 - Let's determine if this table is the active Current Medications table
		m_bIsActiveCurrentMedicationsTable = FALSE;
		m_bIsActiveAllergiesTable = FALSE;
		if(m_pDetail->IsCurrentMedicationsTable()) {

			// (j.jones 2007-07-24 09:27) - PLID 26742 - the medications info ID is cached in CEMR
			long nActiveCurrentMedicationsInfoID = -2;
			//do memory checks
			if(m_pDetail->m_pParentTopic) {
				if(m_pDetail->m_pParentTopic->GetParentEMN()) {
					if(m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()) {
						nActiveCurrentMedicationsInfoID = m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID();
					}
				}
			}

			if(nActiveCurrentMedicationsInfoID == -2) {
				//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
				//but why don't we have an EMR?
				ASSERT(FALSE);
				nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
			}
			
			if(nActiveCurrentMedicationsInfoID == m_pDetail->m_nEMRInfoID) {
				m_bIsActiveCurrentMedicationsTable = TRUE;
			}
		}
		// (c.haag 2007-04-02 15:46) - PLID 25465 - Also determine if this is the active Allergies table
		else if(m_pDetail->IsAllergiesTable()) {

			// (j.jones 2007-07-24 09:27) - PLID 26742 - the allergies info ID is cached in CEMR
			long nActiveCurrentAllergiesInfoID = -2;
			//do memory checks
			if(m_pDetail->m_pParentTopic) {
				if(m_pDetail->m_pParentTopic->GetParentEMN()) {
					if(m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()) {
						nActiveCurrentAllergiesInfoID = m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID();
					}
				}
			}

			if(nActiveCurrentAllergiesInfoID == -2) {
				//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
				//but why don't we have an EMR?
				ASSERT(FALSE);
				nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
			}
			
			if(nActiveCurrentAllergiesInfoID == m_pDetail->m_nEMRInfoID) {
				m_bIsActiveAllergiesTable = TRUE;
			}
		}

		// Add the label
		if (!strLabel.IsEmpty()) {		
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			DWORD dwDisabled = m_pDetail->GetReadOnly() ? WS_DISABLED : 0;
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			m_wndLabel.CreateControl(strLabel, WS_VISIBLE | WS_GROUP | dwDisabled, CRect(0, 0, 0, 0), this, 0xffff);
			//m_wndLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
			// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
			m_wndLabel->NativeFont = EmrFonts::GetTitleFont();
			if (m_bIsActiveCurrentMedicationsTable || m_bIsActiveAllergiesTable) {
				m_wndLabel->ForeColor = RGB(0x00, 0x00, 0xFF);
			}
		}

		// (c.haag 2008-10-16 14:35) - PLID 31700 - Now add fields to the table (columns for non-flipped tables; or rows
		// for flipped tables)
		AddTableFields(m_pDetail, FALSE);
		// (c.haag 2008-10-16 11:01) - PLID 31700 - Add content to the table
		AddTableContent(m_pDetail);

		// (z.manning, 05/28/2008) - PLID 30155 - Now that we have gone through and populated all
		// non-calculated field values, let's go through and do the calculated ones.
		// (c.haag 2008-10-16 16:14) - PLID 31709 - Just call UpdateCalculatedFields because it does exactly the same thing
		
		// (a.walling 2010-04-14 11:17) - PLID 34406
		// (z.manning 2012-03-28 11:02) - PLID 33710 - Calculated cells are now saved to data so we now only update them
		// when the table is changed.
		//UpdateCalculatedFields(m_pDetail, this);

	}NxCatchAll("Error in CEmrItemAdvTableDlg::ReflectCurrentContent");

	// (z.manning 2011-05-05 15:11) - PLID 43568 - Set redraw back to true on the datalist
	SetDatalistRedraw(TRUE);

}

void CEmrItemAdvTableDlg::DestroyContent()
{
	// (c.haag 2008-01-14 11:22) - PLID 17936 - Clear out all dropdown source-related
	// content data
	// (j.jones 2012-10-29 10:11) - PLID 53450 - Don't clear this out! It's cleared only when
	// the table control is destroyed. SmartStamps frequently destroy and recreate the table
	// to cleanly show changes to the table, but this cached dropdown data should never change
	// while the dialog still exists.
	//ClearDropdownSourceInfoMap();

	//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2, so clear the map we are
	//	using to link things together.  See comments in ReflectCurrentContent.
	ClearIndexDatalistMaps();

	// (c.haag 2008-10-17 10:29) - PLID 31700 - Clear the table object
	ClearTableControl();

	CEmrItemAdvDlg::DestroyContent();

	if (IsControlValid(&m_wndLabel)) {
		m_wndLabel.DestroyWindow();
	}

	//DRT 7/10/2007 - PLID 24105 - The table column widths are a special thing that survive beyond the destruction of the content.  They should
	//	never be touched here.
}

// (c.haag 2008-10-17 11:44) - PLID 31709 - Overload necessary for EmrItemAdvTableBase support
// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
BOOL CEmrItemAdvTableDlg::RepositionTableControls(IN OUT CSize &szArea)
{
	return RepositionControls(szArea, FALSE);
}

BOOL CEmrItemAdvTableDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
{
	CEmrItemAdvDlg::RepositionControls(szArea, bCalcOnly);

	/*m.hancock - 4/4/2006 - PLID 19991 - Flickering on a table when changing values in the cells
	If in future, if we do any drawing between ReflectCurrentState and RepositionControls, we should
	use another approach such as reference counting.*/
	SetTableRedraw(FALSE);

	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CSize szMergeBtn;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;

	long cnMergeBtnMargin = 5;

	// Make sure the merge status icon button reflects the state of the data,
	// because that will have a direct influence on our size calculations.
	UpdateStatusButtonAppearances();

	CClientDC dc(this);

	// (c.haag 2006-06-30 17:00) - PLID 19977 - We now calculate
	// merge button dimensions if either the merge or problem
	// button is visible
	if (IsMergeButtonVisible() || IsProblemButtonVisible()) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, m_pBtnMergeStatus, szMergeBtn);
	} else {
		szMergeBtn = CSize(0,0);
		cnMergeBtnMargin = 0;
	}

	long nTopMargin = 3;
	long nBotMargin = 3;
	long nMinNecessaryWidth = 0;
	CRect rLabel;
	if (IsControlValid(&m_wndLabel)) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, sz);			

		//first see if the label is too wide to be displayed
		if(sz.cx > szArea.cx) {

			//if so, find out how many lines we must create
			int nHeight = 1;
			while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
				nHeight++;
			}
			
			//now increase the height of the item to match
			if((sz.cx / (nHeight - 1)) > szArea.cx) {
				sz.cx = szArea.cx;
				sz.cy *= nHeight;
			}
		}

		nTopMargin += sz.cy;

		if (nMinNecessaryWidth < sz.cx) {
			nMinNecessaryWidth = sz.cx;
		}
		if (!bCalcOnly) {
			//TES 10/27/2004  - We won't actually move the label yet, because the position of the controls may 
			//cause us to move the label.
			rLabel = CRect(6, 2, 6+sz.cx, 2+sz.cy);
		}
	}

	long nCurTop = nTopMargin;
	long nCurLeft = 10;
	long nMaxWidth = 0;
	long nMaxBottom = nCurTop;
	BOOL bAns = TRUE;
	
	
	if (IsTableControlValid()) {
		// Calc the dimensions of the control
		CSize sz(LONG_MAX, LONG_MAX);
		
		CalculateTableSize(m_pDetail, sz);

		// Decide the rect based on the current location, with the control's ideal dimensions
		CRect rc(CPoint(nCurLeft, nCurTop), sz);

		if (nMinNecessaryWidth < sz.cx) {
			nMinNecessaryWidth = sz.cx + 20;
		}

		if(!bCalcOnly) {

			// (a.wetta 2005-11-02 17:24) - PLID 16569 - Make the Width and Height of the table fit in window

			// (j.jones 2005-12-20 16:56) - to clarify, what we are doing is calculating the maximum size of the
			// table to fit the window (accounting for margins, the label, and the merge button), so that the
			// user can re-size the window any way they want and the table will always resize accordingly

			//re-size the width of the list to fit the window
			if(rc.Width() != szArea.cx - 20) {
				rc.right = rc.left + szArea.cx - 20;
			}

			//re-size the height of the list to fit the window
			if(rc.Height() != (szArea.cy - nTopMargin - szMergeBtn.cy - 10 - nBotMargin)) {
				rc.bottom = rc.top + (szArea.cy - nTopMargin - szMergeBtn.cy - 10 - nBotMargin);
			}

			//this will cause the table to refresh its columns, which - unless manually resized - will
			//cause the columns to stretch to fill the table's width
			// (r.gonet 02/14/2013) - PLID 40017 - The function now needs to know if the detail is popped up.
			UpdateColumnStyle(m_pDetail, FALSE);
		}

		// (c.haag 2011-03-18) - PLID 42906 - Prior to the 9800 release, rc would the variable that
		// defines the table rectangle. Now we store this in m_rcTableAndCommonButtonsArea and
		// size the table control down to make room for the buttons.
		m_rcTableAndCommonButtonsArea = rc;
		// Now adjust the table control rectangle to make room for the buttons. If the button region
		// exceeds m_rcTableAndCommonButtonsArea, then szCommonButtons will be CSize(0,0) and 
		// the buttons will not display.
		CSize szCommonButtons = CalculateCommonButtonRegionSize();
		rc.top += szCommonButtons.cy;

		// Unless we've been asked to just calculate, we want to also move the control.
		if (!bCalcOnly) {
			// Move it to that spot
			// (b.cardillo 2004-07-07 12:10) - PLID 13344 - Do nothing if the rect hasn't changed.
			CRect rcPrior;
			m_wndTable.GetWindowRect(&rcPrior);
			ScreenToClient(&rcPrior);
			if (!rcPrior.EqualRect(rc)) {
				// Move and don't repaint (we don't want to paint until later, but we do need to invalidate 
				// so we do so immediately after moving)
				m_wndTable.MoveWindow(rc, FALSE);
				// (c.haag 2011-03-18) - PLID 42906 - Move the common list buttons as well
				ResizeCommonListButtons();
				// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
				// children, invalidating the child area is not good enough, so we 
				// have to actually tell each child to invalidate itself.  Also, we 
				// used to ask it to erase too, because according to an old comment 
				// related to plid 13344, the "SetRedraw" state might be false at 
				// the moment.  (I'm deleting that comment on this check-in, so to 
				// see it you'll have to look in source history.)  But I think the 
				// "SetRedraw" state no longer can be false, and even if it were I 
				// think now that we're invalidating the control itself, we wouldn't 
				// need to erase here anyway.
				m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
				// (c.haag 2011-03-18) - PLID 42906 - Redraw the buttons
				for (int i=0; i < m_apCommonListButtons.GetSize(); i++) 
				{
					CNxIconButton* pBtn = m_apCommonListButtons[i];
					pBtn->RedrawWindow(NULL, NULL, RDW_INVALIDATE);
				}
			}
		}

		nMaxBottom = rc.bottom;
		nCurLeft = rc.left;

		long nWidth = rc.Width();
		if (nWidth > nMaxWidth) {
			nMaxWidth = nWidth;
		}
	}

	nMaxBottom += szMergeBtn.cy + 10;

	// (c.haag 2006-07-03 09:29) - PLID 19944 - We now use the variable x to calculate
	// the correct position of each iconic button
	int x = 10;
	if (!bCalcOnly && IsMergeButtonVisible()) {
		m_pBtnMergeStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
		x += szMergeBtn.cx + 3;
	}
	if (!bCalcOnly && IsProblemButtonVisible()) {
		m_pBtnProblemStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
	}

	//Now, put the label wherever we decided to put it.
	if(!bCalcOnly) {
		CRect rPrior;
		GetControlChildRect(this, &m_wndLabel, &rPrior);
		if(!rPrior.EqualRect(rLabel)) {
			m_wndLabel.MoveWindow(rLabel, FALSE);
			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);	
		}
	}
	
	// Store the ideal size
	CSize szIdeal(nCurLeft + nMaxWidth + 8, nMaxBottom + nBotMargin);
	if (szIdeal.cx < nMinNecessaryWidth) {
		szIdeal.cx = nMinNecessaryWidth;
	} else {
		// We're still within the label width so we might as well not complain about 
		// being imperfectly shaped
		bAns = TRUE;
	}
	
	// Return the given rect reflecting the new right and bottom sides
	// Adjust back on the border size since the caller wants window area
	szArea.cx = szIdeal.cx + szBorder.cx;
	szArea.cy = szIdeal.cy + szBorder.cy;

	//m.hancock - 4/4/2006 - PLID 19991 - Flickering on a table when changing values in the cells
	SetTableRedraw(TRUE);

	return bAns;
}

void CEmrItemAdvTableDlg::ReflectCurrentState()
{
	CEmrItemAdvDlg::ReflectCurrentState();
	// (c.haag 2008-10-17 11:47) - PLID 31700 - Moved to this function
	ReflectTableState(this, m_pDetail, FALSE);
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvTableDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - The ReadOnly flag lives in the detail now
	//m_bReadOnly = bReadOnly;
	SetTableReadOnly(m_pDetail->GetReadOnly());
	
	if(IsControlValid(&m_wndLabel)) {
		m_wndLabel.EnableWindow(!m_pDetail->GetReadOnly());
	}

	CEmrItemAdvDlg::ReflectReadOnlyStatus(m_pDetail->GetReadOnly());

	//
	// (c.haag 2007-02-08 12:33) - PLID 24376 - If this is a Current Medications detail, old
	// or new, then the datalist itself must be read-only. We do not allow users to change
	// the state of Current Medications details on templates
	//
	if (IsTableControlValid() && m_pDetail->m_bIsTemplateDetail && m_pDetail->IsCurrentMedicationsTable()) {
		SetTableReadOnly(TRUE);
	}
	// (c.haag 2007-04-05 13:40) - PLID 25516 - If this is an Allergies detail, old or new,
	// then the datalist itself must be read-only. We do not allow users to change the state
	// of Allergies details on templates
	else if (IsTableControlValid() && m_pDetail->m_bIsTemplateDetail && m_pDetail->IsAllergiesTable()) {
		SetTableReadOnly(TRUE);
	}
}

//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableDlg::OnEditingFinishedTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// (c.haag 2008-10-20 09:42) - PLID 31700 - Moved to EmrItemAdvTableBase
	HandleEditingFinishedTable(lpRow, nCol, varOldValue, varNewValue, bCommit, this, NULL, m_pDetail, FALSE);
}

CString CEmrItemAdvTableDlg::GenerateNewVarState()
{
	//update the m_varState with the new values

	m_pDetail->RecreateStateFromContent();

	return AsString(m_pDetail->GetState());
}

void CEmrItemAdvTableDlg::CalcComboSql()
{
	//calculate the combo sql for all columns
	// (z.manning, 04/10/2007) - PLID 25560 - Move this code into the CEMNDetail class so 
	// the popup dialog could use it as well.
	m_pDetail->CalcComboSql();
}

void CEmrItemAdvTableDlg::UpdateLinkedItemList()
{
	CEmrItemAdvTableBase::UpdateLinkedItemList(m_pDetail, FALSE);
}

//DRT 7/30/2008 - PLID 30893 - Removed RemoveLinkedItemSelection

//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableDlg::OnLButtonUpTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (b.cardillo 2005-08-24 10:51) - PLID 16332 - Made it so this datalist starts editing 
		// with a single click by making it call StartEditing on lbutton up.  This has the one 
		// slight drawback that if you lbutton DOWN somewhere else (anywhere else really) and 
		// then move your mouse and lbutton UP here, it still starts editing this cell.  
		// Technically that's incorrect behavior, but I think most of our users prefer it this 
		// way.  The correct way would be to use LeftClick, but that has a much bigger drawback: 
		// when you have an embedded combo dropped down and you click off on another cell in the 
		// datalist, it dismisses the embedded combo but doesn't start editing that other cell 
		// you clicked on, because the embedded combo ate the lbutton DOWN, so the LeftClick is 
		// not fired.  Since this interface is generally designed for tablets, it almost seems 
		// preferable to have the editing start on lbutton up anyway, since people have a 
		// tendency to slide the stylus around on the tablet.
		if(lpRow != NULL && nCol != -1) {
			IRowSettingsPtr pRow(lpRow);
			StartTableEditing(pRow, nCol);
		}
	} NxCatchAll("Error in OnLButtonUpTable");
}

//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableDlg::OnRButtonDownTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);

		SetTableCurSel(pRow);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-11-06 11:56) - PLID 53609 - Display context menu during datalist ShowContextMenu event
void CEmrItemAdvTableDlg::OnShowContextMenuTable(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		SetTableCurSel(pRow);

		//we only want to use the menu if we are right clicking on a selected linked detail
		if(pRow != NULL && nCol > 0) {

			*pbContinue = VARIANT_FALSE;

			int nMenuCmd = -1;

			// (z.manning 2009-08-07 17:14) - PLID 35144 - We need to use the detail column index
			// when getting the column here or this won't work for flipped tables.
			long nDetailRow, nDetailCol;
			TablePositionToDetailPosition(m_pDetail, LookupTableRowIndexFromDatalistRow(pRow), nCol, nDetailRow, nDetailCol);
			TableColumn tc = m_pDetail->GetColumn(nDetailCol);
			// (a.walling 2007-08-30 13:12) - PLID 19106
			TableColumn* ptc = m_pDetail->GetColumnPtr(nDetailCol);
			TableRow *ptr = m_pDetail->GetRowPtr(nDetailRow);

			long nColID = tc.nID;

			if(tc.nType == LIST_TYPE_LINKED) {

				// (c.haag 2008-01-11 16:43) - PLID 17936 - We must convert the datalist value into a table state data value,
				// no matter what type the table column is
				CString strItem = VarString(UnformatDataListValue(m_pDetail, nDetailRow, (short)nDetailCol, pRow->GetValue(nCol)), "");

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu pMenu;
				pMenu.CreatePopupMenu();
				CEMNDetail *pLinkedDetail = NULL;

				//ok we have an item selected, so give the menu option
				if(strItem != "") {
					CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();	

					//try to find the linked item
					pLinkedDetail = pEMN->GetDetailByUniqueIdentifier(strItem);
					if(pLinkedDetail) {
						CString strLabel;
						strLabel.Format("&Pop Up '%s'",pLinkedDetail->GetLabelText());	
						pMenu.InsertMenu(0, MF_BYPOSITION, ID_SHOW_LINKED_DETAIL, strLabel);

						pMenu.AppendMenu(MF_SEPARATOR);
					}
				}

				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				AppendSharedMenuOptions(&pMenu, pRow, nCol, m_pDetail);

				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD,pt.x, pt.y,this);
				if(nMenuCmd == ID_SHOW_LINKED_DETAIL && pLinkedDetail != NULL) {
					pLinkedDetail->Popup();
				}
				pMenu.DestroyMenu();
			} else if (tc.nType == LIST_TYPE_CHECKBOX) {
				// (a.walling 2007-07-06 10:54) - PLID 19106 - ability to check/uncheck all boxes on a table

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu pMenu;
				pMenu.CreatePopupMenu();

				// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
				int i = 0;

				long menuCheckAll = 0xad;
				long menuUncheckAll = 0xae;

				BOOL bUncheck = FALSE;
				int nRows = m_pDetail->GetRowCount();

				// we want to inspect the rows to determine what options are valid for the user
				// ensure this is not the medications or allergy item; seems like an extremely dangerous idea
				// to allow them to be mass-updated like this.
				if (!m_pDetail->IsCurrentMedicationsTable() && !m_pDetail->IsAllergiesTable()) {

					if (nRows > 1) {
						// should have at least two rows for this. Now we need to inspect the rows to see how many are checked and unchecked.
						TableElement te;

						long nChecked = 0;
						long nUnchecked = 0;

						// (a.walling 2007-08-30 12:16) - PLID 19106
						for(i = 0; i < m_pDetail->m_arTableElements.GetSize(); i++) {
							const TableElement &teTmp = m_pDetail->m_arTableElements[i];
							if (teTmp.m_pColumn->nID == tc.nID) { // matches column
								if (teTmp.m_bChecked)
									nChecked++;
							}
						}

						nUnchecked = nRows - nChecked;

						// (a.walling 2007-07-23 16:35) - PLID 19106 - MF_DISABLED items appear as gray and disabled on vista,
						// but they appear normal on XP/2000. So I need to ensure that we are using MF_GRAYED as well!
						DWORD dwEnabledFlags = MF_BYPOSITION|MF_ENABLED;
						DWORD dwDisabledFlags = MF_BYPOSITION|MF_DISABLED|MF_GRAYED;

						pMenu.InsertMenu(0, dwDisabledFlags, menuCheckAll-2, FormatString("%li box%s checked", nChecked, nChecked == 1 ? "" : "es"));
						pMenu.InsertMenu(1, MF_SEPARATOR, menuCheckAll-1, (LPCTSTR)NULL);
						pMenu.InsertMenu(2, nUnchecked == 0 ? dwDisabledFlags : dwEnabledFlags, menuCheckAll, "&Check All");
						pMenu.InsertMenu(3, nChecked == 0 ? dwDisabledFlags : dwEnabledFlags, menuUncheckAll, "&Uncheck All");

						pMenu.AppendMenu(MF_SEPARATOR);
					}
				}

				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				AppendSharedMenuOptions(&pMenu, pRow, nCol, m_pDetail);

				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
				
				if(nMenuCmd == menuCheckAll || nMenuCmd == menuUncheckAll)
				{
					pMenu.DestroyMenu();
					CWaitCursor cws;

					if (nMenuCmd == menuUncheckAll) 
						bUncheck = TRUE;
					else bUncheck = FALSE;

					// alright, let's modify the table
					CMap<__int64, __int64, long, long> mapTable;
					const int nTableElements = m_pDetail->m_arTableElements.GetSize();
					for(i = 0; i < nTableElements; i++) {
						TableElement& teTmp = m_pDetail->m_arTableElements[i];
						// (z.manning 2010-02-18 11:45) - PLID 37427 - The table row ID is now a struct
						__int64 nKey = (((__int64)teTmp.m_pRow) << 32) + (__int64)teTmp.m_pColumn->nID;
						mapTable[nKey] = i;
						//TRACE("%s", FormatString("adding key %I64i (row %I64i, col %I64i)\n", nKey, __int64(teTmp.m_pRow->nID), __int64(teTmp.m_pColumn->nID)));
					}

					// alright, let's modify the table
					long nCols = m_pDetail->GetColumnCount();

					CArray<TableElement, TableElement> arNewTableElements;

					for (int iRow = 0; iRow < nRows; iRow++)
					{
						TableRow *pTableRow = m_pDetail->GetRowPtr(iRow);

						// (z.manning 2010-02-18 11:45) - PLID 37427 - The table row ID is now a struct
						__int64 nnKey = (__int64(pTableRow) << 32) + __int64(tc.nID);
						//TRACE("%s", FormatString("looking for key %I64i (row %I64i, col %I64i)... ", nnKey, __int64(pTableRow->nID), __int64(tc.nID)));

						long nTableIndex = -1;

						if (mapTable.Lookup(nnKey, nTableIndex))
						{
							//TRACE("found\n");
							TableElement teMod = *(m_pDetail->GetTableElementPtrByIndex(nTableIndex));

							// (z.manning 2011-03-23 09:03) - PLID 30608 - If this was previously unchecked and we are now
							// checking it then we need to update any autofill columns.
							if(!teMod.m_bChecked && !bUncheck) {
								m_pDetail->UpdateAutofillColumns(&teMod);
							}

							//TES 7/22/2011 - PLID 42098 - Don't check labels
							if(!teMod.m_pRow->m_bIsLabel) {
								teMod.m_bChecked = !bUncheck;
							}
							m_pDetail->SetTableElement(teMod, TRUE, FALSE);

							if (teMod.m_bChecked && tc.bIsGrouped) {
								// grouped! we need to check for any others to uncheck.
								for (int iCol = 0; iCol < nCols; iCol++) {
									TableColumn* pTableCol = m_pDetail->GetColumnPtr(iCol);

									if ( (pTableCol->nID != tc.nID) && (pTableCol->bIsGrouped) ) {
										nnKey = (__int64(pTableRow) << 32) + __int64(pTableCol->nID);
										//TRACE("%s", FormatString("grouping key %I64i (row %I64i, col %I64i)... ", nnKey, __int64(pTableRow->nID), __int64(pTableCol->nID)));

										long nGroupedElementIndex;
										if (mapTable.Lookup(nnKey, nGroupedElementIndex)) {
											//TRACE("found\n");
											// we found one, unset it!
											TableElement& teGrouped = *(m_pDetail->GetTableElementPtrByIndex(nGroupedElementIndex));

											teGrouped.LoadValueFromVariant(g_cvarEmpty, NULL);
										} else {
											//TRACE("not found\n");
											// if it was checked, it should exist in the map, so ignore.
										}
									}
								}
							}
						}
						else if (!bUncheck)
						{
							//TRACE("not found\n");
							// if we are unchecking, we don't care if we can't find the element in the map, since if it
							// is not in the map it is already unchecked. However, if we are checking, we need to create
							// the table element.

							// create and add the new checked table element
							TableElement teNew;
							teNew.m_pColumn = const_cast<TableColumn*>(ptc);
							teNew.m_pRow = const_cast<TableRow*>(pTableRow);
							//TES 7/22/2011 - PLID 42098 - Don't check labels
							if(!teNew.m_pRow->m_bIsLabel) {
								teNew.m_bChecked = TRUE;
							}
							arNewTableElements.Add(teNew);

							// (z.manning 2011-03-23 09:03) - PLID 30608 - This was previously unchecked and we are now
							// checking it so we need to update any autofill columns.
							m_pDetail->UpdateAutofillColumns(&teNew);

							// uncheck any grouped items
							if (tc.bIsGrouped) {
								for (int iCol = 0; iCol < nCols; iCol++) {
									TableColumn* pTableCol = m_pDetail->GetColumnPtr(iCol);

									// (a.walling 2007-11-07 11:30) - PLID 27998 - VS2008 - Shift count too big
									if ( (pTableCol->nID != tc.nID) && (pTableCol->bIsGrouped) ) {
										nnKey = (__int64(pTableRow) << 32) + pTableCol->nID;

										long nGroupedElementIndex;
										if (mapTable.Lookup(nnKey, nGroupedElementIndex)) {
											// we found one, uncheck it!
											TableElement& teGrouped = *(m_pDetail->GetTableElementPtrByIndex(nGroupedElementIndex));

											teGrouped.LoadValueFromVariant(g_cvarEmpty, NULL);
										} else {
											// create the grouped item as well
											TableElement teGroupedNew;
											teGroupedNew.m_pColumn = pTableCol;
											teGroupedNew.m_pRow = const_cast<TableRow*>(pTableRow);
											teGroupedNew.m_bChecked = FALSE;
											arNewTableElements.Add(teGroupedNew);
										}
									}
								}
							}
						}
						else {
							// element not found in the map, and we are unchecking items.
							//TRACE("not found\n");
						}
					}

					mapTable.RemoveAll();

					// add the new table elements
					for (int iNew = 0; iNew < arNewTableElements.GetSize(); iNew++) {
						m_pDetail->m_arTableElements.Add(arNewTableElements[iNew]);
						//TES 3/18/2011 - PLID 41108 - Also add to our cache of info about selected dropdown items.
						m_pDetail->m_arypTableSelectedDropdowns.Add(new CEMNDetail::TableElementSelectedDropdownItems);
					}
					arNewTableElements.RemoveAll();

					// we are not messing with linked details, so we can preserve the linked details cached flag
					// however we still need to recreate the state
					m_pDetail->RecreateStateFromContent();

					// (a.walling 2007-07-06 12:01) - PLID 19106 - Generate the new state and refresh
					CString strNewVarState = GenerateNewVarState();
					if (m_pDetail->RequestStateChange((LPCTSTR)strNewVarState)) {
						CRect rc;
						m_wndTable.GetWindowRect(&rc);
						ScreenToClient(&rc);
						// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
						// children, invalidating the child area is not good enough, so we 
						// have to actually tell each child to invalidate itself.  Also, we 
						// used to ask it to erase too, because according to an old comment 
						// related to plid 13344, the "SetRedraw" state might be false at 
						// the moment.  (I'm deleting that comment on this check-in, so to 
						// see it you'll have to look in source history.)  But I think the 
						// "SetRedraw" state no longer can be false, and even if it were I 
						// think now that we're invalidating the control itself, we wouldn't 
						// need to erase here anyway.
						m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
					}
				}
			}
			else if(tc.nType == LIST_TYPE_TEXT) {

				// (j.jones 2009-10-02 11:17) - PLID 35161 - added ability to append date information

				BOOL bEditable = TRUE;

				//disabled on read only tables, and templates
				//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
				if(m_pDetail->GetReadOnly() || m_bIsTemplate) {
					bEditable = FALSE;
				}

				//make sure we have a patient ID
				long nPatientID = m_pDetail->GetPatientID();
				if(nPatientID == -1) {
					bEditable = FALSE;
				}
				
				// (z.manning 2010-03-02 10:54) - PLID 37230 - Don't do this if the column is read only
				// (z.manning 2010-04-19 16:20) - PLID 38228 - Improved this check
				if(ptr->IsReadOnly() || ptc->IsReadOnly()) {
					bEditable = FALSE;
				}

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu pMenu;
				pMenu.CreatePopupMenu();

				CMenu pSubMenu;
				pSubMenu.CreatePopupMenu();
				
				enum {
					eTodaysDate = 1,
					// (j.jones 2010-12-21 11:26) - PLID 41904 - added ability to show times
					eCurrentDateTime,
					eCurrentTime,
					// (j.jones 2009-12-18 14:14) - PLID 36570 - added EMN Date
					eEMNDate,
					eLastApptDate,
					eLastApptDays,
					eLastProcDate,
					eLastProcDays,					
				};

				if(bEditable) {
					pSubMenu.InsertMenu(0, MF_BYPOSITION, eTodaysDate, "Today's &Date");
					// (j.jones 2010-12-21 11:26) - PLID 41904 - added ability to show times
					pSubMenu.InsertMenu(1, MF_BYPOSITION, eCurrentDateTime, "&Current Date && Time");
					pSubMenu.InsertMenu(2, MF_BYPOSITION, eCurrentTime, "Current &Time");
					// (j.jones 2009-12-18 14:14) - PLID 36570 - added EMN Date
					pSubMenu.InsertMenu(3, MF_BYPOSITION, eEMNDate, "&EMN Date");
					pSubMenu.InsertMenu(4, MF_BYPOSITION, eLastApptDate, "&Last Appointment Date");
					pSubMenu.InsertMenu(5, MF_BYPOSITION, eLastApptDays, "Days Since Last &Appointment");
					pSubMenu.InsertMenu(6, MF_BYPOSITION, eLastProcDate, "Last &Procedure Date");
					pSubMenu.InsertMenu(7, MF_BYPOSITION, eLastProcDays, "Days &Since Last Procedure");

					pMenu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu.m_hMenu, "&Add Field...");
					pMenu.AppendMenu(MF_SEPARATOR);
				}
				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				AppendSharedMenuOptions(&pMenu, pRow, nCol, m_pDetail);

				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

				CString strTextToAppend = "";

				switch(nMenuCmd) {
					case eTodaysDate: {

						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strTextToAppend = FormatDateTimeForInterface(dtNow, NULL, dtoDate);
						break;
						}
					// (j.jones 2010-12-21 11:26) - PLID 41904 - added ability to show times, to the minute
					case eCurrentDateTime: {

						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strTextToAppend = FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime);
						break;
						}
					case eCurrentTime: {

						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strTextToAppend = FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoTime);
						break;
						}

					// (j.jones 2009-12-18 14:14) - PLID 36570 - added EMN Date
					case eEMNDate: {

						COleDateTime dtEMN = m_pDetail->m_pParentTopic->GetParentEMN()->GetEMNDate();
						strTextToAppend = FormatDateTimeForInterface(dtEMN, NULL, dtoDate);
						break;
						}
					case eLastApptDate:
					case eLastApptDays: {

						//get the last appointment *prior* to today (not on today's date)
						_RecordsetPtr rs = CreateParamRecordset("SELECT Max(Date) AS LastDate, "
							"DateDiff(day, dbo.AsDateNoTime(Max(Date)), dbo.AsDateNoTime(GetDate())) AS Days "
							"FROM AppointmentsT "
							"WHERE PatientID = {INT} AND Status <> 4 "
							"AND dbo.AsDateNoTime(Date) < dbo.AsDateNoTime(GetDate()) "
							"GROUP BY PatientID", nPatientID);
						if(!rs->eof) {
							if(nMenuCmd == eLastApptDate) {
								strTextToAppend = FormatDateTimeForInterface(AdoFldDateTime(rs, "LastDate"), NULL, dtoDate);
							}
							else if(nMenuCmd == eLastApptDays) {
								strTextToAppend.Format("%li days", AdoFldLong(rs, "Days"));
							}
						}
						else {
							AfxMessageBox("This patient has no previous appointment.");
							return;
						}
						rs->Close();

						break;
						}
					case eLastProcDate:
					case eLastProcDays: {

						//get the last procedural appointment *prior* to today (not on today's date)
						_RecordsetPtr rs = CreateParamRecordset("SELECT Max(Date) AS LastDate, "
							"DateDiff(day, dbo.AsDateNoTime(Max(Date)), dbo.AsDateNoTime(GetDate())) AS Days "
							"FROM AppointmentsT "
							"WHERE PatientID = {INT} AND Status <> 4 "
							"AND dbo.AsDateNoTime(Date) < dbo.AsDateNoTime(GetDate()) "
							"AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) "
							"GROUP BY PatientID", nPatientID);
						if(!rs->eof) {
							if(nMenuCmd == eLastProcDate) {
								strTextToAppend = FormatDateTimeForInterface(AdoFldDateTime(rs, "LastDate"), NULL, dtoDate);
							}
							else if(nMenuCmd == eLastProcDays) {
								strTextToAppend.Format("%li days", AdoFldLong(rs, "Days"));
							}
						}
						else {
							AfxMessageBox("This patient has no previous procedure appointment.");
							return;
						}
						rs->Close();

						break;
						}
				}

				if(!strTextToAppend.IsEmpty()) {

					AppendTextToTableCell(pRow, nCol, m_pDetail, strTextToAppend, this, NULL, FALSE);
				}
			}
			else {
				// (z.manning 2010-12-20 09:58) - PLID 41886 - Clear menu options
				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu menu;
				menu.CreatePopupMenu();
				AppendSharedMenuOptions(&menu, pRow, nCol, m_pDetail);
				
				CPoint pt;
				GetCursorPos(&pt);
				nMenuCmd = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
			}

			// (z.manning 2010-12-20 10:05) - PLID 41886 - Clear options
			EClearTableType eClearType = cttInvalid;
			if(nMenuCmd == IDM_CLEAR_TABLE_CELL) {
				eClearType = cttCell;
			}
			else if(nMenuCmd == IDM_CLEAR_TABLE_ROW) {
				eClearType = cttRow;
			}
			else if(nMenuCmd == IDM_CLEAR_TABLE_COLUMN) {
				eClearType = cttColumn;
			}
			else if(nMenuCmd == IDM_CLEAR_TABLE) {
				eClearType = cttTable;
			}

			if(eClearType != cttInvalid) {
				ClearTable(eClearType, pRow, nCol, m_pDetail, this, NULL, FALSE);
			}

			// (z.manning 2010-12-22 15:25) - PLID 41887 - Copy options
			if( nMenuCmd == IDM_COPY_TABLE_ROW_DOWN || nMenuCmd == IDM_COPY_TABLE_ROW_UP ||
				nMenuCmd == IDM_COPY_TABLE_CELL_DOWN || nMenuCmd == IDM_COPY_TABLE_CELL_UP)
			{
				BOOL bCellOnly, bCopyFoward;
				switch(nMenuCmd)
				{
					case IDM_COPY_TABLE_ROW_DOWN:
						bCellOnly = FALSE;
						bCopyFoward = TRUE;
						break;
					case IDM_COPY_TABLE_ROW_UP:
						bCellOnly = FALSE;
						bCopyFoward = FALSE;
						break;
					case IDM_COPY_TABLE_CELL_DOWN:
						bCellOnly = TRUE;
						bCopyFoward = TRUE;
						break;
					case IDM_COPY_TABLE_CELL_UP:
						bCellOnly = TRUE;
						bCopyFoward = FALSE;
						break;
					default:
						ASSERT(FALSE);
						break;
				}

				CopyAndPasteTableRowToAdjacentRow(pRow, nCol, m_pDetail, bCellOnly, bCopyFoward, this, NULL, FALSE);
			}

			switch(nMenuCmd)
			{
				case IDM_TRANSFORM_ROW:
					// (z.manning 2011-05-27 10:42) - PLID 42131
					ApplyTransformation(m_pDetail, pRow, nCol, this, NULL, FALSE);
					break;

				case IDM_EDIT_CODING_GROUP: // (z.manning 2011-07-14 10:23) - PLID 44469
					OpenChargePromptDialogForRow(m_pDetail, pRow, nCol);
					break;
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (a.wetta 2005-11-04 17:40) - 18168 - If the user begins to edit the column sizes, the column widths should be 
// saved and not auto recalculated.
//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableDlg::OnColumnSizingFinishedTable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// Update detail member variable
		m_pDetail->SetSaveTableColumnWidths(TRUE);
		// (r.gonet 02/14/2013) - PLID 40017 - Need to know if the detail is popped up now.
		UpdateColumnStyle(m_pDetail, FALSE);
		// (m.hancock 2006-06-19 14:48) - PLID 20929 - Set the detail and topic as unsaved, 
		// then tell the interface to reflect that.
		// (j.jones 2006-09-01 15:33) - PLID 22380 - only if unlocked
		// (a.walling 2008-08-21 17:20) - PLID 23138 - also only if writable
		if(m_pDetail->m_pParentTopic->GetParentEMN()->GetStatus() != 2 && m_pDetail->m_pParentTopic->GetParentEMN()->IsWritable()) {
			m_pDetail->SetUnsaved();
			m_pDetail->m_pParentTopic->SetUnsaved();
			GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)m_pDetail);	
		}
	// (r.gonet 02/14/2013) - PLID 40017 - Clarified the exception text by adding the class to it, since we now have mulitple functions by this name.
	} NxCatchAll("Error in CEmrItemAdvTableDlg::OnColumnSizingFinished");
}

// (a.wetta 2005-11-04 11:45) - PLID 18618 - Called when an EMN is being saved from CEMRDlg::SaveEmrDetails
// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize 
Nx::Quantum::Batch CEmrItemAdvTableDlg::UpdateColumnInfoInData(BOOL bSaveTableColumnWidths, BOOL bIsTemplate /*= FALSE*/) 
{
	Nx::Quantum::Batch strSqlBatch;

	CString strWidthTable = "EMRTableColumnWidthsT";
	CString strDetailsTable = "EMRDetailsT";
	
	long lDetailID = m_pDetail->m_nEMRDetailID;	
	CString strID;
	if(!m_bIsTemplate) {
		if(lDetailID != -1) {
			strID.Format("%li",lDetailID);
		}
		else {
			// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs
			CString str;
			str.Format("SET @nEMRDetailID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotDetail, (long)m_pDetail);
			AddStatementToSqlBatch(strSqlBatch, str);
			strID = "@nEMRDetailID";
		}
	}
	else {
		strWidthTable = "EMRTemplateTableColumnWidthsT";
		strDetailsTable = "EMRTemplateDetailsT";
		lDetailID = m_pDetail->m_nEMRTemplateDetailID;
		if(lDetailID != -1) {
			strID.Format("%li",lDetailID);
		}
		else {
			// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs
			CString str;
			str.Format("SET @nEMRTemplateDetailID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotDetail, (long)m_pDetail);
			AddStatementToSqlBatch(strSqlBatch, str);
			strID = "@nEMRTemplateDetailID";
		}
	}

	if (bSaveTableColumnWidths) {
		int nNewWidth = GetTableColumnStoredWidth(0);
		// (r.gonet 02/14/2013) - PLID 40017 - Need to also save the popped up table's column widths.
		int nNewPopupWidth = m_pDetail->GetColumnWidths()->GetFirstPopupColumnWidth(false);
		CString strNewPopupWidth = FormatString("%s", nNewPopupWidth == -1 ? "NULL" : AsString(nNewPopupWidth));
		// (c.haag 2008-10-21 11:23) - PLID 31709 - If the table is flipped, then EMRDataID_X actually takes the role of a column
		// in the context of table column widths. So, we need to use that field instead of EMRDataID_Y. EMRDataID_X did not exist
		// in the column width SQL table prior to 9000 scope.
		CString strEMRDataField = (m_pDetail->m_bTableRowsAsFields) ? "EMRDataID_X" : "EMRDataID_Y";

		// (r.gonet 02/14/2013) - PLID 40017 -Added PopupColumnWidth to the save statements.
		// (j.armen 2014-01-28 10:23) - PLID 60497 - Quantum saving - Place tablenames inline instead of formatting
		AddStatementToSqlBatch(strSqlBatch, 
			"IF EXISTS (SELECT * FROM " + strWidthTable + " WHERE EMRDetailID = %s AND " + strEMRDataField + " IS NULL) "
			"UPDATE " + strWidthTable + " SET ColumnWidth = %li, PopupColumnWidth = %s WHERE EMRDetailID = %s AND " + strEMRDataField + " IS NULL "
			"ELSE "
			"INSERT INTO " + strWidthTable + " (EMRDetailID, ColumnWidth, PopupColumnWidth) VALUES (%s, %li, %s)",
			strID,
			nNewWidth, strNewPopupWidth, strID,
			strID, GetTableColumnStoredWidth(0), strNewPopupWidth);
		int nNumColumn = GetTableColumnCount();

		for (int i = 1; i < nNumColumn; i++) {
			CString strColumnTitle = GetTableColumnTitle(i);

			nNewWidth = GetTableColumnStoredWidth(i);
			// (r.gonet 02/14/2013) - PLID 40017 - Need to also save the popped up table's column widths.
			nNewPopupWidth = m_pDetail->GetColumnWidths()->GetPopupWidthByName(strColumnTitle, false);
			strNewPopupWidth = FormatString("%s", nNewPopupWidth == -1 ? "NULL" : AsString(nNewPopupWidth));
			
			int nEMRInfoID = m_pDetail->m_nEMRInfoID;

			// (c.haag 2008-10-23 11:16) - PLID 31709 - If the table is flipped, we need to actually refer to row data elements. This is the one
			// time where we need to query data differently because column widths pertain to datalist columns; not logical detail columns
			if (m_pDetail->m_bTableRowsAsFields) {
				AddStatementToSqlBatch(strSqlBatch, "SET @nEMRDataID = (SELECT ID FROM EMRDataT WHERE EMRInfoID = %li AND Data = '%s' AND ListType = 2)", 
					nEMRInfoID, _Q(strColumnTitle));
			} else {
				AddStatementToSqlBatch(strSqlBatch, "SET @nEMRDataID = (SELECT ID FROM EMRDataT WHERE EMRInfoID = %li AND Data = '%s' AND ListType >= 3)", 
					nEMRInfoID, _Q(strColumnTitle));
			}

			// (r.gonet 02/14/2013) - PLID 40017 -Added PopupColumnWidth to the save statements.
			// (j.armen 2014-01-28 10:23) - PLID 60497 - Quantum saving - Place tablenames inline instead of formatting
			AddStatementToSqlBatch(strSqlBatch, 
				"IF EXISTS (SELECT * FROM " + strWidthTable + " WHERE EMRDetailID = %s AND " + strEMRDataField + " = @nEMRDataID) "
				"UPDATE " + strWidthTable + " SET ColumnWidth = %li, PopupColumnWidth = %s WHERE EMRDetailID = %s AND " + strEMRDataField + " = @nEMRDataID "
				"ELSE "
				"INSERT INTO " + strWidthTable + " (EMRDetailID, " + strEMRDataField + ", ColumnWidth, PopupColumnWidth) "
				"VALUES (%s, @nEMRDataID, %li, %s)",
				strID,
				nNewWidth, strNewPopupWidth, strID,
				strID, nNewWidth, strNewPopupWidth);
		}
	}
	else {
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM " + strWidthTable + " WHERE EMRDetailID = %s", strID);
	}

	AddStatementToSqlBatch(strSqlBatch, "UPDATE " + strDetailsTable + " SET SaveTableColumnWidths = %li WHERE ID = %s", bSaveTableColumnWidths, strID);

	return strSqlBatch;
}

//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableDlg::OnEditingFinishingTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	// (c.haag 2008-10-20 12:32) - PLID 31700 - Moved logic to EmrItemAdvTableBase
	HandleEditingFinishingTable(lpRow, nCol, varOldValue, strUserEntered, pvarNewValue, pbCommit, pbContinue, this, m_pDetail);
}

LRESULT CEmrItemAdvTableDlg::OnAddNewDropdownColumnSelection(WPARAM wParam, LPARAM lParam) 
{
	// (c.haag 2008-01-22 11:08) - PLID 28686 - This function is called when the user elects
	// to add a new selection in the embedded combo of a dropdown column. The Emr Item Edit
	// dialog will open, and take the user straight into the dropdown selection list configuration.
	try {
		const long nDetailCol = (long)wParam;
		long nDataID = m_pDetail->GetColumn(nDetailCol).nID;

		// Check permissions
		if(!CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			return 0;
		}

		// Update the item BEFORE we pass in a column ID
		if(!m_pDetail->IsActiveInfo()) {
			// (c.haag 2008-03-12 15:55) - PLID 28686 - We unforunately cannot just update the item automatically for
			// these reasons: UpdateItem() doesn't actually update table column ID's. It posts a message that updates the
			// table column ID's later on. We also can't run a query because there's no guarantee that the EmrDataGroupID's
			// have not changed, and there's no way to tell what the new ID's are given the old ones.
			MessageBox("The detail you are editing is on an outdated version. You may not add a new dropdown item because the table has been changed since it was added to this EMN.  "
				"You can bring this item up to date by right clicking on it and choosing 'Bring item up to date.'", NULL, MB_OK | MB_ICONHAND);
			return 0;
		}

		// Now do the edit
		CWaitCursor wc;		

		// (a.walling 2008-03-25 08:11) - PLID 28811 - Pass our detail pointer to the now static function
		OpenItemEntryDlg(m_pDetail, m_bIsTemplate, eEmrItemEntryDlgBehavior_AddNewDropdownColumnSelection, (LPVOID)nDataID );
	}
	NxCatchAll("Error in CEmrItemAdvTableDlg::OnAddNewDropdownColumnSelection");
	return 0;
}

// (j.jones 2008-06-05 09:46) - PLID 18529 - TryAdvanceNextDropdownList will potentially send
// NXM_START_EDITING_EMR_TABLE which will fire this function, which will start editing the
// row and column in question
//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
//	Also updating comments.  This exists as a PostMessage handler because you cannot call StartEditing on 1 cell
//	from within the OnEditingFinished of another cell, or the datalist doesn't allow you to ever really commit
//	the 2nd edit.
// (a.walling 2008-10-02 09:16) - PLID 31564 - VS2008 - Message handlers must fit the LRESULT fn(WPARAM, LPARAM) format
LRESULT CEmrItemAdvTableDlg::OnStartEditingEMRTable(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2008-10-15 16:25) - PLID 31700 - The core logic is now in EmrItemAdvTableBase
		StartEditingEMRTable(wParam, lParam);
	}NxCatchAll("Error in CEmrItemAdvTableDlg::OnStartEditingEMRTable");
	return 0;
}

//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableDlg::OnEditingStartingTable(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	// (c.haag 2008-10-20 09:48) - PLID 31700 - The core logic is now in EmrItemAdvTableBase
	HandleEditingStartingTable(lpRow, nCol, pvarValue, pbContinue, m_pDetail);
}

// (c.haag 2011-03-18) - PLID 42891 - Destroy the common list buttons when the dialog is destroyed
void CEmrItemAdvTableDlg::OnDestroy() 
{
	try {
		ClearCommonListButtons();
	}
	NxCatchAll(__FUNCTION__);

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

// (c.haag 2011-03-18) - PLID 42891 - Common list button handler. This is where we let people quickly populate a medications
// or allergies item with selections.
LRESULT CEmrItemAdvTableDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	try 
	{
		if (WM_COMMAND == message)
		{
			if (HandleCommonListButtonPress(m_pDetail, wParam))
			{
				// User made a selection

				// (a.walling 2011-03-21 12:34) - PLID 42962 - This calls RecreateStateFromContent, 
				// which is then passed to RequestStateChange
				CString strNewVarState = GenerateNewVarState();
				if (m_pDetail->RequestStateChange((LPCTSTR)strNewVarState)) {
					// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
					// children, invalidating the child area is not good enough, so we 
					// have to actually tell each child to invalidate itself.  Also, we 
					// used to ask it to erase too, because according to an old comment 
					// related to plid 13344, the "SetRedraw" state might be false at 
					// the moment.  (I'm deleting that comment on this check-in, so to 
					// see it you'll have to look in source history.)  But I think the 
					// "SetRedraw" state no longer can be false, and even if it were I 
					// think now that we're invalidating the control itself, we wouldn't 
					// need to erase here anyway.
					m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
				}
			}
			else
			{
				// User changed their mind and selected nothing
			}
		}
	}
	NxCatchAll(__FUNCTION__);
	return CEmrItemAdvDlg::WindowProc(message, wParam, lParam);
}

// (a.walling 2012-08-30 07:05) - PLID 51953 - Detect actual VK_TAB keypresses to ignore stuck keys
BOOL CEmrItemAdvTableDlg::PreTranslateMessage(MSG* pMsg)
{
	if (BOOL bRet = CEmrItemAdvTableBase::HandlePreTranslateMessage(pMsg)) {
		return bRet;
	}

	return __super::PreTranslateMessage(pMsg);
}

