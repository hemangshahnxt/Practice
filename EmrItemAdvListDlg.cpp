// EmrItemAdvListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrItemAdvListDlg.h"
#include "EMRItemEntryDlg.h"
#include "EMNDetail.h"
#include "EMRTopic.h"
#include "WindowlessUtils.h"
#include <foreach.h>
#include "EMN.h"
using namespace ADODB;

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-10-03 12:09) - PLID 53002 - Use list-specific fonts for list items

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

// (c.haag 2006-02-28 09:24) - PLID 18984 - I added this class to allow us to
// detect whether a CEmrItemAdvListDlg object is being destroyed in the middle
// of requesting a state change. If it is, a debug assertion is raised
//
class CReqStateChange
{
protected:
	CEmrItemAdvListDlg* m_pDlg;
public:
	CReqStateChange(CEmrItemAdvListDlg* pDlg) { m_pDlg = pDlg; pDlg->m_bRequestingStateChange = TRUE; }
	~CReqStateChange() { m_pDlg->m_bRequestingStateChange = FALSE; }
};

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvListDlg dialog


CEmrItemAdvListDlg::CEmrItemAdvListDlg(class CEMNDetail *pDetail)
	: CEmrItemAdvDlg(pDetail)
	, m_nFirstOverflowControlID(0)
{
	//{{AFX_DATA_INIT(CEmrItemAdvListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_eeialtType = eialtListSingleSelect;
	m_bRequestingStateChange = FALSE;
	m_clrHilightColor = 0;
}

BEGIN_MESSAGE_MAP(CEmrItemAdvListDlg, CEmrItemAdvDlg)
	//{{AFX_MSG_MAP(CEmrItemAdvListDlg)
	ON_CONTROL_RANGE(BN_CLICKED, MIN_CHECKBOX_IDC, MAX_CHECKBOX_IDC, OnButtonClicked)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CEmrItemAdvListDlg, CEmrItemAdvDlg)
    //{{AFX_EVENTSINK_MAP(CEmrItemAdvListDlg)
	ON_EVENT_RANGE(CEmrItemAdvListDlg, MIN_CHECKBOX_IDC, MAX_CHECKBOX_IDC, DISPID_CLICK /* Click */, OnButtonClickedEvent, VTS_I4)
	ON_EVENT_RANGE(CEmrItemAdvListDlg, MIN_LABEL_IDC, MAX_LABEL_IDC, DISPID_CLICK /* Click */, OnLabelClickedEvent, VTS_I4)
	ON_EVENT_RANGE(CEmrItemAdvListDlg, MIN_DATA_LABEL_IDC, MAX_DATA_LABEL_IDC, DISPID_CLICK /* Click */, OnLabelClickedEvent, VTS_I4)
	
	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	ON_EVENT(CEmrItemAdvListDlg, IDC_ADVLIST_MORE, DISPID_CLICK /* Click */, OnMoreButtonClicked, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvListDlg message handlers

void CEmrItemAdvListDlg::ReflectCurrentContent()
{
	// Clear out anything that's already there
	DestroyContent();
	
	// Create everything the parent wants
	CEmrItemAdvDlg::ReflectCurrentContent();

	// Add the label
	{
		// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Use the detail label text with special modifiers for onscreen presentation
		CString strLabel = GetLabelText(TRUE);
		if (!strLabel.IsEmpty()) {
			strLabel.Replace("&", "&&");
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			DWORD dwDisabled = m_pDetail->GetReadOnly() ? WS_DISABLED : 0;
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			m_wndLabel.CreateControl(strLabel, WS_VISIBLE | WS_GROUP | dwDisabled, CRect(0, 0, 0, 0), this, 0xffff);
			//m_wndLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
			// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
			m_wndLabel->NativeFont = EmrFonts::GetTitleFont();
		}
	}

	COLORREF backColor = GetHighlightColor();

	// (j.jones 2004-11-02 10:26) - make bold if it spawns an item, based on the preference
	long nActionBold = GetRemotePropertyInt("EMNTemplateActionBold", 1, 0, GetCurrentUserName(), true);
	//1 - only if it spawns details or topics
	//2 - if it spawns anything
	//0 - never bold

	EmrListState listState(m_pDetail);

	// Create the controls
	std::vector<long> visited;
	long nListElementCount = m_pDetail->GetListElementCount();
	for (long i=0; i < nListElementCount && i < 1000; i++) {
		ListElement le = m_pDetail->GetListElement(i);

		visited.clear();

		if (le.nParentLabelID != -1 && !listState.selected.count(le.nID)) 
		{
			// (a.walling 2012-12-03 12:50) - PLID 53983 - don't show unless we are expanded and all parents are also expanded
			bool bParentsExpanded = true;
			long nParentLabelID = le.nParentLabelID;
			do {
				visited.push_back(nParentLabelID);
				if (!m_expandedLabelIDs.count(nParentLabelID)) {
					bParentsExpanded = false;
					break;
				}
				nParentLabelID = listState.elements[nParentLabelID].nParentLabelID;
				if (find(visited.begin(), visited.end(), nParentLabelID) != visited.end()) {
					TRACE("Loop detected!\r\n");
					ASSERT(FALSE);
					break;
				}
			} while (nParentLabelID != -1);

			if (!bParentsExpanded) {
				continue;
			}
		}

		CString strValue = le.strName;
		// (r.gonet 09/18/2012) - PLID 52713 - We need to escape any ampersands otherwise they will be interpreted as keyboard shortcut definitions.
		strValue.Replace("&", "&&");

		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		DWORD dwDisabled = m_pDetail->GetReadOnly() ? WS_DISABLED : 0;

		CWnd* pWnd;
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		COLORREF foreColor = GetRegularTextColor();
				
		if (le.bIsLabel) {
			// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
			// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
			NxWindowlessLib::NxFreeLabelControl& label = *(new NxWindowlessLib::NxFreeLabelControl);
			
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			label.CreateControl(strValue, WS_VISIBLE | dwDisabled, CRect(0, 0, 0, 0), this, MIN_DATA_LABEL_IDC + i);
			
			pWnd = &label;
		
			m_arypControls.Add(pWnd);

			// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
			label->NativeFont = EmrFonts::GetUnderlineListFont();

			// (a.walling 2012-12-03 12:50) - PLID 53983 - Give clickable labels a blue color, purple if expanded
			if (listState.children.count(le.nID)) {
				if (m_expandedLabelIDs.count(le.nID)) {
					foreColor = HEXRGB(0x800080);
				} else {
					foreColor = HEXRGB(0x0000FF);
				}
				label->Interactive = VARIANT_TRUE;
			}
		}
		else {
			//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
			// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
			// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
			NxWindowlessLib::NxFreeButtonControl& button = *(new NxWindowlessLib::NxFreeButtonControl);
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			button.CreateControl(strValue, WS_VISIBLE | WS_TABSTOP | ((i == 0) ? WS_GROUP : 0) | dwDisabled, CRect(0, 0, 0, 0), this, MIN_CHECKBOX_IDC + i);

			if (m_eeialtType == eialtListSingleSelect) {
				button->RadioStyle = VARIANT_TRUE;
			}
			
			pWnd = &button;

			m_arypControls.Add(pWnd);

			// (j.jones 2006-05-25 10:35) - PLID 20803 - the ListElement's nActionsType will be
			// -1 if there are no actions, 1 if there are EMR item or Mint Item actions, and 2 if there are
			// other actions but no EMR item or Mint Item actions

			if(m_bIsTemplate && ((nActionBold == 1 && le.nActionsType == 1) ||
				(nActionBold == 2 && le.nActionsType >= 1))) {
				foreColor = GetSpawnItemTextColor();
			} else if (m_bGhostly) {
				foreColor = GetGhostlyTextColor();
			}

			// (j.jones 2011-04-28 14:39) - PLID 43122 - bold this if IsFloated
			if(le.bIsFloated) {
				// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
				button->NativeFont = EmrFonts::GetBoldListFont();
			} else {
				button->NativeFont = EmrFonts::GetRegularListFont();
			}

			//TES 1/28/2008 - PLID 28673 - If this element has popup information associated with it, draw a little
			// * that the user can click on to restore the multi-popup dialog.  We will give it an ID that is in the
			// same spot in the range of label IDs as the checkbox is in the range of checkbox IDs
			if(le.pDetailPopup) {
				//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
				// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
				// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
				NxWindowlessLib::NxFreeLabelControl& popupLabel = *(new NxWindowlessLib::NxFreeLabelControl);
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				popupLabel.CreateControl("*", WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), this, MIN_LABEL_IDC + i);
				popupLabel->NativeFont = EmrFonts::GetRegularListFont();
				popupLabel->ForeColor = RGB(0, 0, 0xFF);
				popupLabel->Interactive = VARIANT_TRUE;

				m_arypControls.Add(&popupLabel);
			}
		}

		pWnd->SetProperty(DISPID_FORECOLOR, VT_I4, foreColor);
		pWnd->SetProperty(DISPID_BACKCOLOR, VT_I4, backColor);
	}

	// Add our merge status icon button control
	if (m_pBtnMergeStatus) {
		m_arypControls.Add(m_pBtnMergeStatus);
	}
	// (c.haag 2006-06-30 17:05) - PLID 19977 - Add our problem status icon button control
	if (m_pBtnProblemStatus) {
		m_arypControls.Add(m_pBtnProblemStatus);
	}
}

void CEmrItemAdvListDlg::DestroyContent()
{
	m_nFirstOverflowControlID = 0;

	// Remove our merge status icon button control from the array because
	// our parent will destroy the button.
	if (m_pBtnMergeStatus) {
		for (long i=0; i<m_arypControls.GetSize(); i++) {
			if (m_arypControls[i] == m_pBtnMergeStatus) {
				m_arypControls.RemoveAt(i--);
			}
		}
	}
	if (m_pBtnProblemStatus) {
		for (long i=0; i<m_arypControls.GetSize(); i++) {
			if (m_arypControls[i] == m_pBtnProblemStatus) {
				m_arypControls.RemoveAt(i--);
			}
		}
	}

	m_arySpawnItems.RemoveAll();

	CEmrItemAdvDlg::DestroyContent();

	if (IsControlValid(&m_wndLabel)) {
		m_wndLabel.DestroyWindow();
	}
	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	if (IsControlValid(&m_overflowButton)) {
		m_overflowButton.DestroyWindow();
	}
	for (long i=0; i<m_arypControls.GetSize(); i++) {
		CWnd *pwnd = m_arypControls.GetAt(i);
		if (pwnd) {
			pwnd->DestroyWindow();
			delete pwnd;
		}
	}
	m_arypControls.RemoveAll();
}

BOOL CEmrItemAdvListDlg::OnButtonClickedEvent(UINT nID)
{
	PostMessage(WM_COMMAND, MAKEWPARAM(nID, BN_CLICKED), 0);
	return TRUE;
}

void CEmrItemAdvListDlg::OnButtonClicked(UINT nID)
{
	try {
		BOOL bNewChecked = IsDlgButtonChecked(nID);

		CWaitCursor pWait;
		CString strNewState;
		for (long i=0; i<m_pDetail->GetListElementCount(); i++) {
			//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
			if (IsDlgButtonChecked(MIN_CHECKBOX_IDC+i)) {

				// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
				// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
				NxWindowlessLib::NxFreeButtonControl* pButton = dynamic_cast<NxWindowlessLib::NxFreeButtonControl*>(GetDlgItem(MIN_CHECKBOX_IDC+i));

				if (!pButton) {
					ASSERT(FALSE);
					continue;
				}

				if (bNewChecked && (m_eeialtType == eialtListSingleSelect) && (nID != MIN_CHECKBOX_IDC+i)) {

					if (pButton) {
						(*pButton)->Value = BST_UNCHECKED;
					}

				} else {
					if (!strNewState.IsEmpty()) {
						strNewState += "; ";
					}
					strNewState += FormatString("%li",m_pDetail->GetListElement(i).nID);
				}
			}
		}

		// Don't request the state change if nothing's actually changing
		// (c.haag 2007-05-17 10:20) - PLID 26046 - Use GetStateVarType to get the detail state type
		if (m_pDetail->GetStateVarType() == VT_EMPTY || strNewState.Compare(VarString(m_pDetail->GetState(), "")) != 0) {
			// The state is definitely changing
			CReqStateChange rsc(this);
			m_pDetail->RequestStateChange((LPCTSTR)strNewState);
		}
	} NxCatchAll("CEmrItemAdvListDlg::OnButtonClicked");
}

// (a.walling 2011-11-11 11:11) - PLID 46622 - Emr list item positioning improvements
BOOL CEmrItemAdvListDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
{
	CEmrItemAdvDlg::RepositionControls(szArea, bCalcOnly);
	if (m_arypControls.IsEmpty()) {
		szArea.cx = 0;
		szArea.cy = 0;
		return TRUE;
	}

	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;

	// Make sure the merge status icon button reflects the state of the data,
	// because that will have a direct influence on our size calculations.
	UpdateStatusButtonAppearances();

	CClientDC dc(this);

	const CPoint ptLabelOrigin(6, 2);
	CSize szLabel(0, 0);
	if (IsControlValid(&m_wndLabel)) {
		CSize szLabelArea = szArea;
		szLabelArea.cx -= ptLabelOrigin.x;

		szLabel.SetSize(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel);

		//first see if the label is too wide to be displayed
		if(szLabel.cx > szLabelArea.cx) {
			szLabel.SetSize(szLabelArea.cx, LONG_MAX);
			CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel, TRUE);
		}
	}

	CRect rLabel(ptLabelOrigin, szLabel);
	
	// (a.walling 2011-11-11 11:11) - PLID 46622 - Now we've calculated out our label and have our new area, so pass to the
	// calc or display function as necessary
	BOOL bRet;
	if (bCalcOnly) {
		bRet = RepositionControlsCalcOnly(dc, szArea, rLabel);
	} else {
		bRet = RepositionControlsForDisplay(dc, szArea, rLabel);
	}

	szArea.cx += szBorder.cx;
	szArea.cy += szBorder.cy;

	return bRet;
}

static const long cnContentMargin = 2;
static const long cnColumnBuffer = 8;

// (a.walling 2011-11-11 11:11) - PLID 46622 - Emr list item positioning improvements - as before, this simply assumes a single column layout
BOOL CEmrItemAdvListDlg::RepositionControlsCalcOnly(CDC& dc, IN OUT CSize &szArea, CRect& rLabel)
{
	CSize szColumn(0, 0);

	for (long i=0; i<m_arypControls.GetSize(); i++) {
		CWnd *pWnd = ((CWnd *)m_arypControls.GetAt(i));
		
		// Don't have the merge status icon button influence our list
		// if it's not visible
		// (c.haag 2006-07-03 10:27) - PLID 19977 - Same with problem
		// buttons
		//if (pWnd == m_pBtnMergeStatus && !m_bShowMergeStatusButton)
		//	continue;
		if (pWnd == m_pBtnMergeStatus && !IsMergeButtonVisible())
			continue;
		if (pWnd == m_pBtnProblemStatus && !IsProblemButtonVisible())
			continue;

		if (!IsControlValid(pWnd)) {
			continue;
		}

		if(pWnd->GetDlgCtrlID() >= MIN_LABEL_IDC && pWnd->GetDlgCtrlID() <= MAX_LABEL_IDC) {
			continue;
		}

		CSize sz(LONG_MAX, LONG_MAX);

		// Calc the dimensions of the control
		CalcControlIdealDimensions(&dc, pWnd, sz);

		szColumn.cy += sz.cy;
		szColumn.cy += GetControlBufferWidth(); // more padding by default
		szColumn.cy += GetControlBufferWidth();
		if (szColumn.cx < sz.cx) {
			szColumn.cx = sz.cx;
		}
	}

	szColumn.cx += cnColumnBuffer;

	CSize szIdeal(0, 0);

	// Label
	szIdeal.cy += rLabel.Height();
	// Content margin top
	szIdeal.cy += cnContentMargin;
	// Content
	szIdeal.cy += szColumn.cy;
	// Content margin bottom
	szIdeal.cy += cnContentMargin;

	
	// Append margins to total control size
	szColumn.cx += cnContentMargin;
	szColumn.cx += cnContentMargin;

	// Use content or label width
	szIdeal.cx += max(rLabel.right, szColumn.cx);

	BOOL bAns = FALSE;

	if (szIdeal.cx < szArea.cx) {
		bAns = TRUE;
	}
	
	// Return the given rect reflecting the new right and bottom sides
	szArea = szIdeal;

	return bAns;
}

// (a.walling 2011-11-11 11:11) - PLID 46622 - Simple holder for a control and it's size, window, associated label window, origin, and assigned column
struct ControlSize
{
	ControlSize()
		: pWnd(NULL)
		, pLabel(NULL)
		, sz(0, 0)
		, ptOrigin(0, 0)
		, nColumn(0)
	{}

	ControlSize(CWnd* pWnd, const CSize& sz)
		: pWnd(pWnd)
		, pLabel(NULL)
		, sz(sz)
		, ptOrigin(0, 0)
		, nColumn(0)
	{}

	CWnd* pWnd;
	CWnd* pLabel;
	CSize sz;
	CPoint ptOrigin;
	int nColumn;
};

// (a.walling 2011-11-11 11:11) - PLID 46622 - Emr list item positioning improvements - Helper for calculating the layout
struct ControlLayout
{
	ControlLayout(CEmrItemAdvListDlg* pListDlg, CDC& dc, const CPoint& ptContentOrigin, const CSize& szContent, const CRect& rLabel)
		: m_pListDlg(pListDlg)
		, m_dc(dc)
		, m_rcContent(ptContentOrigin, szContent)
		, m_szColumnContent(0, 0)
		, m_firstOverflowControl(m_controlSizes.end())
		, m_rLabel(rLabel)
	{
		m_columnSizes.reserve(8);
	}

	CEmrItemAdvListDlg* m_pListDlg;
	CDC& m_dc;
	CRect m_rcContent;
	CRect m_rLabel;
	std::vector<ControlSize> m_controlSizes;

	long GetAverageItemHeight() const
	{
		if (m_controlSizes.empty()) return 0;

		long nTotal = 0;
		
		for (std::vector<ControlSize>::const_iterator pos = m_controlSizes.begin(); pos != m_controlSizes.end(); ++pos) {
			nTotal += pos->sz.cy;
		}

		return nTotal / m_controlSizes.size();
	}

	void ResetDimensions(const CPoint& ptContentOrigin, const CSize& szContent, bool bResetAll = true)
	{
		if (bResetAll) {
			Reset();
		}
		m_rcContent = CRect(ptContentOrigin, szContent);
	}

	CSize m_szColumnContent;
	std::vector<long> m_columnSizes;

	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	std::vector<ControlSize>::iterator m_firstOverflowControl;
	int GetFirstOverflowCtrlID() const
	{
		if (m_controlSizes.end() == m_firstOverflowControl) {
			return 0;
		}

		return m_firstOverflowControl->pWnd->GetDlgCtrlID();
	}

	void Reset()
	{
		m_firstOverflowControl = m_controlSizes.end();
		m_columnSizes.clear();
		m_szColumnContent.SetSize(0, 0);
	}

	bool IsOverflow() const
	{
		return m_szColumnContent.cx > m_rcContent.Width();
	}

	bool IsOverflowY() const
	{
		return m_szColumnContent.cy > m_rcContent.Height();
	}

	// (a.walling 2011-11-11 11:11) - PLID 46622 - Refresh the control positions within the maximum column width
	void Refresh(long nMaxColumnWidth)
	{
		Reset();

		CPoint pt = m_rcContent.TopLeft();
		std::vector<ControlSize>::iterator pos = m_controlSizes.begin();
		while (pos != m_controlSizes.end()) {
			CSize szColumn;
			// (a.walling 2012-08-24 06:10) - PLID 52287 - Ensure no less than cnColumnBuffer is passed as the max width
			std::vector<ControlSize>::iterator nextPos = 
				FillColumn(m_columnSizes.size(), szColumn, pt, max(cnColumnBuffer, nMaxColumnWidth - cnColumnBuffer), pos, m_controlSizes.end());

			m_columnSizes.push_back(szColumn.cx);

			m_szColumnContent.cx += szColumn.cx;
			if (szColumn.cy > m_szColumnContent.cy) {
				m_szColumnContent.cy = szColumn.cy;
			}

			pt.x += szColumn.cx;

			// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
			// (a.walling 2012-08-24 06:10) - PLID 52287 - Consider Y-overflow as well
			if ( (pt.x >= m_rcContent.right || pt.y >= m_rcContent.bottom) && (m_firstOverflowControl == m_controlSizes.end()) ) {
				m_firstOverflowControl = pos;
			}

			// (a.walling 2012-08-24 06:10) - PLID 52287 - Ensure we always go to the next item
			if (nextPos == pos) {
				nextPos++;
			}
			pos = nextPos;
		}
	}
	
	// (a.walling 2011-11-11 11:11) - PLID 46622 - Fills a column with as many controls as possible; if the first control in a column cannot fit vertically, it is still put into the column.
	// will modify the ControlSize if it does not fit nMaxWidth!
	std::vector<ControlSize>::iterator FillColumn(int nColumn, CSize& szColumn, const CPoint& ptColumnOrigin, long nMaxWidth, const std::vector<ControlSize>::iterator& begin, const std::vector<ControlSize>::iterator& end)
	{
		std::vector<ControlSize>::iterator pos = begin;
		szColumn.SetSize(0, 0);

		CPoint pt = ptColumnOrigin;

		while (pos != end) {
			if (pos->sz.cx == -1 && pos->sz.cy == -1) {
				CSize sz(LONG_MAX, LONG_MAX);
				
				CalcControlIdealDimensions(&m_dc, pos->pWnd, sz, TRUE);

				pos->sz = sz;
			}

			long nNewBottom = pt.y + pos->sz.cy;

			if ((nNewBottom > m_rcContent.bottom) && pos != begin) {
				return pos;
			}

			if (pos->sz.cx > nMaxWidth) {
				CSize sz(nMaxWidth, LONG_MAX);

				CalcControlIdealDimensions(&m_dc, pos->pWnd, sz, TRUE);

				// (a.walling 2012-08-24 06:10) - PLID 52287 - Ensure no zero values for the size calc
				if (0 == sz.cx) {
					sz.cx = pos->sz.cx;
				}
				if (0 == sz.cy) {
					sz.cy = pos->sz.cy;
				}
				pos->sz = sz;

				nNewBottom = pt.y + pos->sz.cy;
			}

			if (nNewBottom > m_rcContent.bottom && pos != begin) {
				return pos;
			}

			pos->ptOrigin = pt;
			pos->nColumn = nColumn;

			pt.y += pos->sz.cy;

			szColumn.cy += pos->sz.cy;

			if (szColumn.cx < pos->sz.cx) {
				szColumn.cx = pos->sz.cx;
			}

			++pos;
		}

		szColumn.cx += cnColumnBuffer;

		return pos;
	}

	bool IsRightOfLabel() const
	{
		return m_rcContent.top < m_rLabel.bottom;
	}

	// (a.walling 2011-11-11 11:11) - PLID 46622 - Emr list item positioning improvements - actually reposition the controls, expanding if necessary into the available area
	// to minimize unsightly whitespace
	void RepositionControls()
	{		
		// If we have some empty space, let's expand a bit, so we don't have unsightly gaps.
		float extendY = 1.0f;
		int padX = 0;
		int padY = 0;

		if (IsControlValid(&m_pListDlg->m_overflowButton)) {
			m_pListDlg->m_overflowButton.DestroyWindow();
		}

		if (!IsRightOfLabel()) {
			// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
			if (!IsOverflow()) {

				if (m_columnSizes.size() > 1) {
					int nUnused = m_rcContent.Width() - m_szColumnContent.cx;

					nUnused -= cnColumnBuffer;
					nUnused -= cnColumnBuffer;

					if (nUnused > 0) {
						padX = nUnused / (m_columnSizes.size() - 1);
					}
				}
			}

			padX = min(16, padX);

			/*if (m_rcContent.Height() > m_szColumnContent.cy) {
				int nUnused = m_rcContent.Height() - m_szColumnContent.cy;
				int nCenteredPadY = nUnused / 2;

				if (nCenteredPadY > 0 && nCenteredPadY <= cnColumnBuffer) {
					padY = nCenteredPadY;
				} else if (nUnused > 0) {
					padY = cnColumnBuffer;
				}

				extendY = float(m_rcContent.Height() - padY) / m_szColumnContent.cy;
			}*/

			if (m_rcContent.Height() > m_szColumnContent.cy) {

				int maxExtend = m_szColumnContent.cy + 64;
				maxExtend = min(m_rcContent.Height(), maxExtend);

				int nUnused = maxExtend - m_szColumnContent.cy;
				int nCenteredPadY = nUnused / 2;

				if (nCenteredPadY > 0 && nCenteredPadY <= cnColumnBuffer) {
					padY = nCenteredPadY;
				} else if (nUnused > 0) {
					padY = cnColumnBuffer;
				}

				extendY = float(maxExtend - padY) / m_szColumnContent.cy;
			}
		} else {
			// right of label, try to put more space between the label and the content
		}

		for (std::vector<ControlSize>::iterator it = m_controlSizes.begin(); it != m_controlSizes.end(); ++it) {

			CPoint ptPadded(
				it->ptOrigin.x + (padX * it->nColumn),
				int(((it->ptOrigin.y - m_rcContent.top) * extendY) + m_rcContent.top) + padY
			);

			//CPoint ptPadded(
			//	it->ptOrigin.x,
			//	it->ptOrigin.y
			//);

			CRect rcNew(ptPadded, it->sz);

			MoveControl(it->pWnd, rcNew);

			if (it->pLabel) {
				//TES 1/28/2008 - PLID 28673 - Also move our * if we have one.
				CRect rcLabel(
					rcNew.right + 1,
					rcNew.top,
					rcNew.right + 1 + 7,
					rcNew.bottom
				);
				
				MoveControl(it->pLabel, rcLabel);
			}
		}

		MoveControl(&m_pListDlg->m_wndLabel, m_rLabel);

		// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
		if (IsOverflow()) {
			if (!IsControlValid(&m_pListDlg->m_overflowButton)) {
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				m_pListDlg->m_overflowButton.CreateControl("More", WS_VISIBLE, CRect(0,0,0,0), m_pListDlg, IDC_ADVLIST_MORE);
				// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
				m_pListDlg->m_overflowButton->NativeFont = EmrFonts::GetTinyUnderlineFont();
				m_pListDlg->m_overflowButton->ForeColor = RGB(0, 0, 0xff);
				m_pListDlg->m_overflowButton->Interactive = VARIANT_TRUE;
				// (a.walling 2012-06-13 08:49) - PLID 46623 - Opaque background fore 'more' label
				m_pListDlg->m_overflowButton->BackStyle = 1; // transparent
			}

			CSize szOverflow;
			CalcControlIdealDimensions(&m_dc, &m_pListDlg->m_overflowButton, szOverflow);

			CRect rcClient;
			m_pListDlg->GetClientRect(&rcClient);
			
			CRect rcOverflow(
				rcClient.right - szOverflow.cx,
				rcClient.bottom - szOverflow.cy,
				rcClient.right,
				rcClient.bottom
			);

			MoveControl(&m_pListDlg->m_overflowButton, rcOverflow);

			ChangeZOrder(m_pListDlg, &m_pListDlg->m_overflowButton, &CWnd::wndTop);
		}
	}

	void MoveControl(CWnd* pWnd, const CRect& rcNew)
	{
		CRect rcPrior;

		GetControlChildRect(m_pListDlg, pWnd, &rcPrior);

		if (!rcPrior.EqualRect(rcNew)) {
			pWnd->MoveWindow(rcNew);
		}
	}
};

// (a.walling 2011-11-11 11:11) - PLID 46622 - Emr list item positioning improvements
BOOL CEmrItemAdvListDlg::RepositionControlsForDisplay(CDC& dc, IN OUT CSize &szArea, CRect& rLabel)
{
	CPoint ptContentOrigin(cnContentMargin, rLabel.bottom + cnContentMargin);

	CSize szContent = szArea;
	szContent.cy -= ptContentOrigin.y; // label + margin
	szContent.cy -= cnContentMargin; // bottom
	szContent.cx -= cnContentMargin; // left
	szContent.cx -= cnContentMargin; // right

	ControlLayout layout(this, dc, ptContentOrigin, szContent, rLabel);

	layout.m_controlSizes.reserve(m_arypControls.GetSize());

	for (long i=0; i<m_arypControls.GetSize(); i++) {
		CWnd *pWnd = ((CWnd *)m_arypControls.GetAt(i));
		
		// Don't have the merge status icon button influence our list
		// if it's not visible
		// (c.haag 2006-07-03 10:27) - PLID 19977 - Same with problem
		// buttons
		//if (pWnd == m_pBtnMergeStatus && !m_bShowMergeStatusButton)
		//	continue;
#pragma TODO("PLID 46622 - These buttons were not properly accounted for, even previously")
		if (pWnd == m_pBtnMergeStatus && !IsMergeButtonVisible())
			continue;
		if (pWnd == m_pBtnProblemStatus && !IsProblemButtonVisible())
			continue;

		if (!IsControlValid(pWnd)) {
			continue;
		}

		if(pWnd->GetDlgCtrlID() >= MIN_LABEL_IDC && pWnd->GetDlgCtrlID() <= MAX_LABEL_IDC) {
			ASSERT(!layout.m_controlSizes.empty());
			if (!layout.m_controlSizes.empty()) {
				layout.m_controlSizes.back().pLabel = pWnd;
			}
			continue;
		}

		layout.m_controlSizes.push_back(ControlSize(pWnd, CSize(-1, -1)));
	}

	ASSERT(!layout.m_controlSizes.empty());

	if (layout.m_controlSizes.empty()) {
		ThrowNxException("Encountered an empty list!");
	}	

	// check for right of label approach
	{
		CSize szColumn;
		layout.FillColumn(0, szColumn, layout.m_rcContent.TopLeft(), LONG_MAX, layout.m_controlSizes.begin(), ++layout.m_controlSizes.begin());

		if (szColumn.cy > szContent.cy)  {
			// single line exceeds the available content height! Try going to the right of the label.
			CPoint ptNewContentOrigin(rLabel.right + cnContentMargin, rLabel.top);

			CSize szNewContent = szArea;
			szNewContent.cx -= cnContentMargin; // left
			szNewContent.cx -= cnContentMargin; // right

			szNewContent.cx -= rLabel.Width();

			if (szNewContent.cx > 0) {
				szContent = szNewContent;
				ptContentOrigin = ptNewContentOrigin;

				layout.ResetDimensions(ptNewContentOrigin, szContent);
			}
		}
	}

	long nMaxColumnWidth = szContent.cx;

	layout.Refresh(nMaxColumnWidth);

	if (layout.IsOverflow()) {
		// definitely need multiple columns

		long nHalfContentWidth = szContent.cx / 2;

		if (nHalfContentWidth < 64) {
			nHalfContentWidth = szContent.cx;
		}

		// first set to the average
		nMaxColumnWidth = layout.m_szColumnContent.cx / layout.m_columnSizes.size();

		if (nMaxColumnWidth > nHalfContentWidth) {
			nMaxColumnWidth = nHalfContentWidth;
		}

		layout.Refresh(nMaxColumnWidth);
	}

	if (!layout.IsOverflow() && !layout.IsOverflowY()) {

		CSize szLastFitContent;
		CSize szNewContent = szContent;
		size_t columnCount = layout.m_columnSizes.size();

		// (a.walling 2012-08-24 06:10) - PLID 52287 - Provide an escape hatch just in case
		int iterLimit = 0;
		static const int safeIterMax = 100;

		while ( (!layout.IsOverflow() && !layout.IsOverflowY() && layout.m_columnSizes.size() == columnCount) && ++iterLimit < safeIterMax) {
			szLastFitContent = szNewContent;

			/*
			long nAdjust = (layout.m_szColumnContent.cy - layout.m_columnSizes.back() - layout.GetAverageItemHeight()) / layout.m_columnSizes.size();

			if (nAdjust <= 0) {
				break;
			}

			szNewContent.cy -= nAdjust;
			*/

			long averageHeight = layout.GetAverageItemHeight();

			szNewContent.cy -= averageHeight;

			// (a.walling 2012-08-23 10:26) - PLID 52279 - Break out of loop if average item height is zero
			if (szNewContent.cy <= 0 || averageHeight <= 0) {
				szNewContent = szLastFitContent;
				break;
			}

			layout.ResetDimensions(ptContentOrigin, szNewContent);			
			layout.Refresh(nMaxColumnWidth);
		}
		_ASSERTE(iterLimit < safeIterMax);

		// now scale back until it does not overflow
		// (a.walling 2012-08-24 06:10) - PLID 52287 - Provide an escape hatch just in case
		iterLimit = 0;
		while ( ( (layout.IsOverflow() || layout.IsOverflowY()) && layout.m_columnSizes.size() != columnCount) && ++iterLimit < safeIterMax) {
			long averageHeight = layout.GetAverageItemHeight();

			// (a.walling 2012-08-23 10:26) - PLID 52279 - Break out of loop if average item height is zero
			if (averageHeight <= 0) break;

			szNewContent.cy += averageHeight;

			layout.ResetDimensions(ptContentOrigin, szNewContent);			
			layout.Refresh(nMaxColumnWidth);
		}
		_ASSERTE(iterLimit < safeIterMax);

		// now just set the client area but keep our minimal arrangement
		layout.ResetDimensions(ptContentOrigin, szContent, false);
	}

	BOOL bAns = FALSE;
	{
		CSize szIdeal(0, 0);

		// Label
		szIdeal.cy += rLabel.Height();
		// Content
		if (!layout.IsRightOfLabel()) {
			szIdeal.cy += layout.m_szColumnContent.cy;
			// Content margin top
			szIdeal.cy += cnContentMargin;
			// Content margin bottom
			szIdeal.cy += cnContentMargin;
		} else {
			szIdeal.cy = max(szIdeal.cy, layout.m_szColumnContent.cy);
		}
		
		// Append margins to total control size
		long nIdealWidth = layout.m_szColumnContent.cx;	
		nIdealWidth += cnContentMargin;
		nIdealWidth += cnContentMargin;

		// Use content or label width
		szIdeal.cx += max(rLabel.right, nIdealWidth);

		if (szIdeal.cx <= szArea.cx && szIdeal.cy <= szArea.cy) {
			bAns = TRUE;
		} else {		
			// Return the given rect reflecting the new right and bottom sides
			szArea = szIdeal;
		}
	}

	// do the actual positioning
	layout.RepositionControls();

	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	m_nFirstOverflowControlID = layout.GetFirstOverflowCtrlID();

	return bAns;
}

// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
void CEmrItemAdvListDlg::OnMoreButtonClicked()
{
	if (!m_nFirstOverflowControlID) return;

	try {
		// (a.walling 2012-06-15 08:32) - PLID 46623 - For now just go ahead and display all items to reduce confusion
		PopupAllElements();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
void CEmrItemAdvListDlg::PopupAllElements()
{
	try {	
		DWORD dwDisabled = m_pDetail->GetReadOnly() ? (MF_DISABLED | MF_GRAYED) : 0;

		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu menu;

		menu.CreatePopupMenu();

		EmrListState listState(m_pDetail);

		long nListElementCount = m_pDetail->GetListElementCount();
		for (int nElement = 0; nElement < nListElementCount; ++nElement) {

			ListElement le = m_pDetail->GetListElement(nElement);

			if (le.nParentLabelID != -1 && !listState.selected.count(le.nID)) 
			{
				// (a.walling 2012-12-03 12:50) - PLID 53983 - don't show unless we are expanded and all parents are also expanded
				bool bParentsExpanded = true;
				long nParentLabelID = le.nParentLabelID;
				do {
					if (!m_expandedLabelIDs.count(nParentLabelID)) {
						bParentsExpanded = false;
						break;
					}
					nParentLabelID = listState.elements[nParentLabelID].nParentLabelID;
				} while (nParentLabelID != -1);

				if (!bParentsExpanded) {
					continue;
				}
			}

			DWORD dwFlags = dwDisabled;

			int nCtrlID;
			if (le.bIsLabel) {
				nCtrlID = MIN_DATA_LABEL_IDC + nElement;

				menu.AppendMenu(dwFlags | MF_DISABLED, nCtrlID, ConvertToControlText(le.strName));
				menu.AppendMenu(dwFlags | MF_SEPARATOR, 0, "");
			} else {
				nCtrlID = MIN_CHECKBOX_IDC + nElement;

				if (le.bIsSelected) {
					dwFlags |= MF_CHECKED;
				}
				menu.AppendMenu(dwFlags, nCtrlID, ConvertToControlText(le.strName));
			}
		}

		CPoint pt;
		::GetCursorPos(&pt);

		int nRet = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if (0 != nRet) {
			// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
			// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
			NxWindowlessLib::NxFreeButtonControl* pButton = dynamic_cast<NxWindowlessLib::NxFreeButtonControl*>(GetDlgItem(nRet));

			if (!pButton) {
				ASSERT(FALSE);
			} else {
				(*pButton)->DoClick();
			}
		}
	} NxCatchAllThrow(__FUNCTION__);
}

// (a.walling 2012-12-11 09:44) - PLID 53988 - Try to reflect current state, and recreate content if necessary due to state
void CEmrItemAdvListDlg::ReflectCurrentState()
{
	if (!TryReflectCurrentState()) {
		ReflectCurrentContent();

		CRect rcWindow;
		GetWindowRect(&rcWindow);
		RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);

		TryReflectCurrentState();
	}
}

// (a.walling 2012-12-11 09:44) - PLID 53988 - Try to reflect current state
bool CEmrItemAdvListDlg::TryReflectCurrentState()
{
	CEmrItemAdvDlg::ReflectCurrentState();

	// First clear all checkboxes/radiobuttons
	// (b.cardillo 2006-02-23 17:54) - PLID 19376 - Notice we invalidate them all here too now.  
	// They were all being invalidated implicitly before because we used to not clip children.  
	// Now that we do, we have to ask our children to draw themselves.
	{
		for (long i=0; i<m_pDetail->GetListElementCount(); i++) {
			//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.	
			// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
			// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
			NxWindowlessLib::NxFreeButtonControl* pButton = dynamic_cast<NxWindowlessLib::NxFreeButtonControl*>(GetDlgItem(MIN_CHECKBOX_IDC+i));
			if (pButton) {
				(*pButton)->Value = BST_UNCHECKED;
			}
		}
	}

	bool bCheckedAll = true;

	// Then set whichever ones are in the curstate list
	// (c.haag 2007-05-17 10:20) - PLID 26046 - Use GetStateVarType to get the detail state type
	if (m_pDetail->GetStateVarType() != VT_NULL && m_pDetail->GetStateVarType() != VT_EMPTY) {
		CString strKeyList = VarString(m_pDetail->GetState());
		CString strValidKeyList;
		long nPos = 0;
		while (nPos < strKeyList.GetLength()) {
			// Get the current key and move the index to the beginning of the next key
			long nLen = strKeyList.Find(";", nPos) - nPos;
			if (nLen < 0) {
				nLen = strKeyList.GetLength() - nPos;
			}
			CString strCurKey = strKeyList.Mid(nPos, nLen);
			nPos = nPos + nLen + 2;
			// Find this key in the list
			long nKeyIndex;
			{
				nKeyIndex = -1;
				for (long i=0; i<m_pDetail->GetListElementCount(); i++) {
					if (AsString(m_pDetail->GetListElement(i).nID) == strCurKey) {
						nKeyIndex = i;
						break;
					}
				}
			}
			// Select that control
			if (nKeyIndex != -1) {
				if (bCheckedAll) {
					CWnd* pButton = GetDlgItem(MIN_CHECKBOX_IDC + nKeyIndex);
					if (!pButton) {
						bCheckedAll = false;
					}
				}

				if (m_eeialtType == eialtListSingleSelect) {
					// Found the item, select it (and only it) and we're done
					//TES 1/28/2008 - PLID 28673 - This calculation relied on the IDs being stored in consecutive order
					// which is not necessarily true now, so I updated it.
					CheckRadioButton(MIN_CHECKBOX_IDC, MIN_CHECKBOX_IDC + m_pDetail->GetListElementCount() - 1, MIN_CHECKBOX_IDC + nKeyIndex);
					// (a.walling 2008-05-27 15:54) - PLID 29391 - Ensure we get an actual item before calling invalidate, or risk an AV
					return bCheckedAll;
				} else if (m_eeialtType == eialtListMultiSelect) {
					// Found this item, select it and break out of the inner loop but keep looping in the outer one
					//TES 1/28/2008 - PLID 28673 - This calculation relied on the IDs being stored in consecutive order
					// which is not necessarily true now, so I updated it.
					CheckDlgButton(MIN_CHECKBOX_IDC + nKeyIndex, BST_CHECKED);
				}

				if (strValidKeyList.GetLength())
					strValidKeyList += "; " + strCurKey;
				else
					strValidKeyList = strCurKey;
			} else {
				// Couldn't find the control associated with this key; this could mean the key no longer 
				// exists in data, the owner (usually the EMRDlg) should have prevented this situation.
				//ASSERT(FALSE);
			}
		}
		// (c.haag 2004-06-21 12:12) - PLID 13121 - If strValidKeyList does not match m_pDetail->m_varState, it means that
		// our state would violate the integrity of m_aryKeys. This can happen if you make a list detail, then select an
		// element from the list, then delete that element from Add/Edit Items. The approach here is to assign a new state
		// to this detail that does not violate data integrity, and then redraw the detail.
		//
		// Though this code helps us in the problem of data validation, this is not always called before a chart
		// is saved in funky or improper Practice setups, so there is not a 100% guarantee that when a
		// chart is saved, you won't get an error about invalid multi-list selections.
		if (VarString(m_pDetail->GetState()) != strValidKeyList)
		{
			m_pDetail->SetState(_bstr_t(strValidKeyList));
			RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
		}
	}

	return bCheckedAll;
}

COLORREF CEmrItemAdvListDlg::GetHighlightColor()
{
	if (m_clrHilightColor & 0x80000000) {
		return GetSysColor(m_clrHilightColor & 0x000000FF);
	} else {
		return PaletteColor(m_clrHilightColor);
	}
}

void CEmrItemAdvListDlg::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	BOOL bHandledContextMenu = FALSE;
	
	// Try to pop up the menu ourselves if we're supposed to
	try {
		bHandledContextMenu = DataElementPopUp(CalcContextMenuPos(pWnd, pos));
	} NxCatchAll("CEmrItemAdvListDlg::OnContextMenu");

	if (!bHandledContextMenu) {
		// Give the base class its chance
		CEmrItemAdvDlg::OnContextMenu(pWnd, pos);
	}
}

BOOL CEmrItemAdvListDlg::DataElementPopUp(CPoint pt)
{
	//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
	if(m_pDetail->GetReadOnly()) return FALSE;

	// Find out which ID was right-clicked on
	long nSelButtonDataID = -1;
	BOOL bAnythingSelected = FALSE;
	{
		// Loop through the whole list to determine both the button the point 
		// is over and whether ANY button is selected.
		nSelButtonDataID = -1;
		bAnythingSelected = FALSE;

		// (a.walling 2012-12-03 12:50) - PLID 53983 - Index in m_arypControls does not correlate 
		// with the index of the ListElement in the detail any longer
		CRect rect;
		for (int i = 0; i < m_arypControls.GetSize(); ++i) {
			CWnd *pBtn = m_arypControls.GetAt(i);

			if (!pBtn) {
				continue;
			}

			int nID = pBtn->GetDlgCtrlID();
			int nIndex = nID - MIN_CHECKBOX_IDC;

			if (nID < MIN_CHECKBOX_IDC || nID > MAX_CHECKBOX_IDC) {
				continue;
			}

			GetControlWindowRect(this, pBtn, &rect);
			
			if (!rect.IsRectEmpty() && rect.PtInRect(pt)) {
				// Got the button, calculate its data ID
				ASSERT(nSelButtonDataID == -1); // There ought to be only one button that the mouse is over.
				nSelButtonDataID = m_pDetail->GetListElement(nIndex).nID;
				// And give it focus because it was just right-clicked upon
				pBtn->SetFocus();
			}

			if (!bAnythingSelected && IsDlgButtonChecked(nID)) {
				// An item is selected, so now we know
				bAnythingSelected = TRUE;
			}
		}
	}

	// If we got a data ID above, then give the user the context menu
	if (nSelButtonDataID != -1) {
		// Just use an enum instead of using const variables or number literals
		enum {
			miUnselect = 1,
			miEdit,
			// (j.jones 2008-07-24 08:55) - PLID 30779 - add ability to add a new problem always,
			// and add the ability to edit existing problems, per list item
			miNewProblem,
			miEditProblem,
			miExistingProblem, // (c.haag 2009-05-27 14:39) - PLID 34249
		};

		CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();

		// Create the menu
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		mnu.CreatePopupMenu();

		BOOL bShow = FALSE;

		// Only add the unselect menu item if this item is selected
		if (m_eeialtType == eialtListSingleSelect) {
			mnu.AppendMenu((bAnythingSelected ? MF_ENABLED : MF_DISABLED|MF_GRAYED)|MF_STRING|MF_BYPOSITION, miUnselect, "&Unselect");
			bShow = TRUE;
		}

		if(m_bCanEditContent) {
			if(bShow) {
				mnu.AppendMenu(MF_SEPARATOR);
			}
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEdit, "&Edit");
			bShow = TRUE;
		}

		// (j.jones 2008-07-24 08:56) - PLID 30779 - add ability to add a new problem always,
		// and add the ability to edit existing problems
		if(nSelButtonDataID != -1 && !m_bIsTemplate) {
			if(bShow) {
				mnu.AppendMenu(MF_SEPARATOR);
			}

			// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
			// if the EMN is not writeable
			mnu.AppendMenu(MF_STRING|MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), miNewProblem, "Link with New &Problem");
			// (c.haag 2009-05-27 14:36) - PLID 34249 - Link with other problems
			{
				CEMRTopic *pTopic = m_pDetail->m_pParentTopic;
				CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
				CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
				if (NULL != pEMR) {
					mnu.AppendMenu(MF_STRING|MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), miExistingProblem, "Link with Existing Problems");
				}
			}
			if (m_pDetail->HasProblems(nSelButtonDataID)) {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEditProblem, "Update Problem &Information");
			}
			bShow = TRUE;
		}

		if(!bShow)
			return FALSE;

		switch (mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this)) {

		case miUnselect:
			// Call the unselect functionality
			{
				// Just ask the detail to change the state to "nothing selected" and then reflect the 
				// current state on our own screen (which we do ourselves because technically it's our 
				// responsibility, even though in most cases the RequestStateChange() function will 
				// call it for us too).
				CReqStateChange rsc(this);
				if (m_pDetail->RequestStateChange("")) {
					ReflectCurrentState();
					RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
				}
			}
			break;
		case miEdit: {
			// (a.walling 2008-03-25 08:11) - PLID 28811 - Pass our detail pointer to the now static function
			OpenItemEntryDlg(m_pDetail, m_bIsTemplate);
			break;
		}
			// (j.jones 2008-07-24 09:51) - PLID 30779 - support adding a new problem to this ded
		case miNewProblem: {
			NewProblem(eprtEmrDataItem, nSelButtonDataID);
			break;
		}

		case miEditProblem: {
			EditProblem(eprtEmrDataItem, nSelButtonDataID);
			break;
		}

		// (c.haag 2009-05-27 14:34) - PLID 34249 - We can now link this item with multiple problems
		case miExistingProblem: {
			LinkProblems(eprtEmrItem, nSelButtonDataID);
			break;
		}

		case 0:
			// The user canceled the menu, do nothing
			break;
		default:
			// This should never happen because we only have 2 possible menu items (miUnselect and miEdit)
			ASSERT(FALSE);
			break;
		}
		// Return TRUE because a menu was popped up (even if the user didn't do anything with it)
		return TRUE;
	} else {
		// Return FALSE because it wasn't our job to pop up a menu
		return FALSE;
	}
}

// Pass 0 to clear the hilight color
void CEmrItemAdvListDlg::ChangeHilightColor(COLORREF clrNewColor)
{
	if (m_clrHilightColor != clrNewColor) {
		// Set the new color
		m_clrHilightColor = clrNewColor;
	}

	for (int i = 0; i < m_arypControls.GetSize(); ++i) {
		CWnd* pWnd = m_arypControls[i];
		int ctrlID = pWnd->GetDlgCtrlID();

		if (ctrlID >= MIN_CHECKBOX_IDC && ctrlID <= MAX_CHECKBOX_IDC) {
			pWnd->SetProperty(DISPID_BACKCOLOR, VT_I4, GetHighlightColor());
		}
	}
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvListDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - The ReadOnly flag lives in the detail now
	//m_bReadOnly = bReadOnly;
	if(GetSafeHwnd()) {
		m_wndLabel.EnableWindow(!m_pDetail->GetReadOnly());
		for(int i = 0; i < m_arypControls.GetSize(); i++) {
			//TES 10/12/2004 - We still want the merge status button to be enabled.
			// (c.haag 2006-11-20 16:21) - PLID 22052 - Same with problem buttons
			if((CWnd*)m_arypControls.GetAt(i) != m_pBtnMergeStatus && 
				(CWnd*)m_arypControls.GetAt(i) != m_pBtnProblemStatus) {
				((CWnd*)m_arypControls.GetAt(i))->EnableWindow(!m_pDetail->GetReadOnly());
			}			
		}
	}
	CEmrItemAdvDlg::ReflectReadOnlyStatus(bReadOnly);
}

void CEmrItemAdvListDlg::OnDestroy() 
{
	if (m_bRequestingStateChange) {
		//
		// (c.haag 2006-02-28 09:25) - If we get here, it means
		// we are being destroyed in the middle of requesting a state change
		// (look in this source file for calls to m_pDetail->RequestStateChange())
		//
		// In lamens terms, that means if you change a single or multi-select
		// list selection, the item you clicked on disappears.
		//
		// Hopefully this explicit observation will come in handy in future development
	}
	CEmrItemAdvDlg::OnDestroy();
}

BOOL CEmrItemAdvListDlg::OnLabelClickedEvent(UINT nID)
{
	PostMessage(NXM_NXLABEL_LBUTTONDOWN, (WPARAM)nID, 0);
	return TRUE;
}

LRESULT CEmrItemAdvListDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 1/28/2008 - PLID 28673 - This must be one of our * labels, which have IDs between MIN_LABEL_IDC and 
		// MAX_LABEL_IDC
		UINT nIdc = (UINT)wParam;

		if (nIdc >= MIN_LABEL_IDC && nIdc <= MAX_LABEL_IDC) {

			//TES 1/28/2008 - PLID 28673 - Get the associated list element, and use its info to recreate the multipopupdlg.
			ListElement le = m_pDetail->GetListElement(nIdc - MIN_LABEL_IDC);
			if(le.pDetailPopup) {
				m_pDetail->m_pParentTopic->GetParentEMN()->RestoreMultiPopup(le.pDetailPopup);
			}
		} else if (nIdc >= MIN_DATA_LABEL_IDC && nIdc <= MAX_DATA_LABEL_IDC) {

			// (a.walling 2012-12-03 12:50) - PLID 53983 - Expand or contract the parent label if necessary

			ListElement le = m_pDetail->GetListElement(nIdc - MIN_DATA_LABEL_IDC);
			if (m_expandedLabelIDs.count(le.nID)) {
				m_expandedLabelIDs.erase(le.nID);
			} else {
				m_expandedLabelIDs.insert(le.nID);
			}

			foreach(const ListElement& l, m_pDetail->GetListElements()) {
				if (l.nParentLabelID == le.nID) {
					ReflectCurrentContent();
					ReflectCurrentState();
										
					// Here's the important part: tell the derived class it's time to rearrange itself
					CRect rcWindow;
					GetWindowRect(&rcWindow);
					RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);

					if (CWnd* pCur = GetDlgItem(nIdc)) {
						pCur->SetFocus();
					}

					// (b.cardillo 2006-02-22 10:44) - PLID 19376 - Now since the control positions have likely 
					// changed we need to invalidate the area around the controls; also the label too, but only 
					// if it actually HAS changed.
					RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_ERASE);

					break;
				}
			}
		}

	}NxCatchAll("Error in CEmrItemAdvListDlg::OnLabelClick()");
	return 0;
}