// FilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "FilterDlg.h"
#include "FilterDetailDlg.h"
#include "FilterFieldInfo.h"
#include "LetterWritingRc.h"
#include "nxmessagedef.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

IMPLEMENT_DYNAMIC(CFilterDlg, CDialog)

CFilterDlg::CFilterDlg(long nFilterId, CWnd* pParent, long nFilterType, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&))
	: CDialog(CFilterDlg::IDD, pParent),
	  CFilter(nFilterId, nFilterType, pfnIsActionSupported, pfnCommitSubfilterAction, pfnGetNewFilterString)
{
	//{{AFX_DATA_INIT(CFilterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nScrolledHeight = 0;
}

CFilterDlg::~CFilterDlg()
{
}

void CFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilterDlg, CDialog)
	//{{AFX_MSG_MAP(CFilterDlg)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg message handlers

BOOL CFilterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ShowScrollBar(SB_VERT, TRUE);
	Refresh();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

inline long CalcVertHeight(long nPos) {
	return (((nPos / SCROLL_POS_PER_ITEM) * ITEM_SIZE_VERT) + ((nPos % SCROLL_POS_PER_ITEM) * SCROLL_POS_HEIGHT));
}

long CFilterDlg::DoScrollTo(long nNewTopPos)
{
	// Make sure the new position is not out of range
	if (nNewTopPos > SCROLL_BOTTOM_POS) nNewTopPos = SCROLL_BOTTOM_POS;
	if (nNewTopPos < SCROLL_TOP_POS) nNewTopPos = SCROLL_TOP_POS;

	// Calculate the amount we are going to need to scroll
	long nOldHeight, nNewHeight;
	nOldHeight = CalcVertHeight(GetScrollPos(SB_VERT));
	nNewHeight = CalcVertHeight(nNewTopPos);
	
	// Scroll
	SetScrollPos(SB_VERT, nNewTopPos);
	long nScrollHeight = nOldHeight - nNewHeight;
	m_nScrolledHeight += nScrollHeight;
	ScrollWindow(0, nScrollHeight);

	return 0;
}

void CFilterDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (nSBCode) {
	case SB_LINEUP:
		DoScrollTo(GetScrollPos(SB_VERT)-1);
		break;
	case SB_LINEDOWN:
		DoScrollTo(GetScrollPos(SB_VERT)+1);
		break;
	case SB_PAGEUP:
		DoScrollTo(GetScrollPos(SB_VERT)-SCROLL_POS_PER_PAGE);
		break;
	case SB_PAGEDOWN:
		DoScrollTo(GetScrollPos(SB_VERT)+SCROLL_POS_PER_PAGE);
		break;
	case SB_TOP:
		DoScrollTo(SCROLL_TOP_POS);
		break;
	case SB_BOTTOM:
		DoScrollTo(SCROLL_BOTTOM_POS);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		DoScrollTo((int)nPos);
		break;
	default:
		// Do nothing special
		break;
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

// Appropriately sets the m_bUseOrAfter variable of the detail prior to nPreviousToThisDetailIndex and reflects the change on screen
void CFilterDlg::UpdatePreviousUseOrAfter(long nPreviousToThisDetailIndex)
{
	try {
		// Make sure the previous item in the list knows the status of the item that changed
		long nDetailData = nPreviousToThisDetailIndex;
		if (nDetailData > 0) {
			// Not the first element
			CFilterDetailDlg *pDetailDlg = static_cast<CFilterDetailDlg *>(m_arypFilterDetails[nDetailData-1]);
			ASSERT(pDetailDlg);
			if (pDetailDlg) {
				// Got the detail dialog object, so set the m_bUseOrAfter value and redraw it
				if ((long)pDetailDlg->m_nDetailData == ((m_nItemCount - 1) - 1)) {
					// The last non-empty row still needs to use OR because it looks better.  As soon as a new filter detail is 
					// created after this one and its value set to not-empty, this row's UseOrAfter value 
					// will be set appropriately
					pDetailDlg->m_bUseOrAfter = true;
				} else {
					pDetailDlg->m_bUseOrAfter = m_arypFilterDetails[nDetailData]->m_bUseOr;
				}
				pDetailDlg->RefreshUseOr();
			}
		}
	} NxCatchAll("CFilterDlg::UpdatePreviousUseOrAfter");
}

LRESULT CFilterDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_FIELD_SEL_CHANGED:
		{
			// If the item that changed was the empty row, we're going to need a new empty row
			// This relies on the fact that AddDetail will not add an item if current last item is blank

			long nOldItemCount = m_nItemCount;

			AddDetail();

			long nNewItemCount = m_nItemCount;

			// Tell the previous item this one's UseOr status may have changed, or the list of filter details may have changed
			UpdatePreviousUseOrAfter((long)wParam);

			//if we added an item, make sure we scroll to it
			if(nOldItemCount < nNewItemCount)
				DoScrollTo(SCROLL_BOTTOM_POS);
		}
		break;
	case NXM_FILTER_USE_OR_CHANGED:
		{
			// Tell the previous item this one's UseOr status may have changed, or the list of filter details may have changed
			UpdatePreviousUseOrAfter((long)wParam);
		}
		break;
	case NXM_FILTER_REMOVE_ITEM:
		{
			long nDetailData = (long)wParam;
			// We don't want to do anything if the nDetailData is -1 because 
			// that means it hasn't yet been placed on the screen in a given position
			if (nDetailData >= 0) {
				// Remove the detail
				DoScrollTo(GetScrollPos(SB_VERT)-SCROLL_POS_PER_ITEM);
				RemoveDetail(nDetailData);
				Refresh();
			}

			// Tell the previous item this one's UseOr status may have changed, or the list of filter details may have changed
			UpdatePreviousUseOrAfter((long)wParam);
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

CFilterDetail *CFilterDlg::CreateNewDetail()
{
	CFilterDetailDlg *pDlg = new CFilterDetailDlg(this, m_nItemCount, m_nFilterBase, m_pfnIsActionSupported, m_pfnCommitSubfilterAction, m_pfnGetNewFilterString);
	if (pDlg) {
		// Try to create the window
		pDlg->Create(IDD_FILTER_DETAIL_DLG, this);
		if (pDlg->m_hWnd) {
			pDlg->SetPosition(m_nItemCount);
			if (m_nItemCount > 0) {
				// Empty rows assume "or" because it looks better.  As soon as a new filter detail is 
				// created after this one and its value set to not-empty, this row's UseOrAfter value 
				// will be set appropriately
				((CFilterDetailDlg *)m_arypFilterDetails[m_nItemCount-1])->m_bUseOrAfter = true;
				m_arypFilterDetails[m_nItemCount-1]->Refresh();
			}
			return pDlg;
		} else {
			// We allocated the window, but couldn't create it so deallocate it
			delete pDlg;
			pDlg = NULL;
			AfxThrowNxException("Could not create allocated filter detail dialog");
		}
	}

	// If we made it to here, return failure
	return NULL; 
}

void CFilterDlg::Refresh()
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_DISABLENOSCROLL|SIF_PAGE|SIF_RANGE;
	si.nMin = 0;
	si.nMax = m_nItemCount*SCROLL_POS_PER_ITEM;
	si.nPage = SCROLL_POS_PER_PAGE;
	SetScrollInfo(SB_VERT, &si, TRUE);
}

BOOL CFilterDlg::SetDetailFocus(long nDetailIndex, EnumDetailFocusPos nFocusPos)
{
	if (nDetailIndex >= 0 && nDetailIndex < m_nItemCount) {
		CFilterDetailDlg *pDlg = (CFilterDetailDlg *)m_arypFilterDetails[nDetailIndex];
		if (pDlg) {
			switch (nFocusPos) {
			case dfpFieldType:
				//pDlg->m_cboFields.SetFocus();
				pDlg->GetDlgItem(IDC_COMBO_FIELD)->SetFocus();
				return TRUE;
			case dfpOperator:
				pDlg->m_cboOperators.SetFocus();
				return TRUE;
			case dfpValue:
				{
					// Set the focus to the combo if it's there
					pDlg->GetDlgItem(IDC_VALUE_LIST)->SetFocus();
					// Set the focus to the edit box if it's there
					CWnd *pValueWnd = pDlg->GetDlgItem(IDC_VALUE_EDIT);
					if (pValueWnd) {
						pValueWnd->SetFocus();
					}
					return TRUE;
				}
			}
		}
	}
	// If we made it here we failed
	return FALSE;
}

// (b.cardillo 2006-05-19 18:14) - PLID 20593 - This function checks to see if the sql statement 
// you passed was actually "SELECT Name FROM CustomFieldsT WHERE ID = <n>" where <n> is a number.
// If your statement matches this exactly, then it looks up the requested custom field name in 
// the global custom field name cache and returns TRUE.  If your statement is not of that form, 
// then it just returns FALSE.
BOOL CheckCacheIfCustomFieldName(LPCTSTR strSqlGetCustomField, OUT CString &strAns)
{
	static LPCTSTR cstrSqlBase = "SELECT Name FROM CustomFieldsT WHERE ID = ";
	static const long cnSqlBase = strlen(cstrSqlBase);
	if (strnicmp(strSqlGetCustomField, cstrSqlBase, cnSqlBase) == 0) {
		const TCHAR *pstrRemainder = strSqlGetCustomField + cnSqlBase;
		long nCustomFieldIndex = 0;
		while (true) {
			TCHAR ch = *pstrRemainder;
			if (ch == '\0') {
				// End of string, we're all set so break out of the loop
				break;
			} else if (ch >= '0' && ch <= '9') {
				// Numeric digit, so increment our number
				nCustomFieldIndex = nCustomFieldIndex * 10 + ch - '0';
			} else {
				// Some other character, which means we can't parse this string ourselves
				return FALSE;
			}
			pstrRemainder++;
		}
		// If we made it here, we know we need to grab the custom field name, see if we've 
		// cached them yet.  And if we haven't, then do so.
		return GetCustomFieldName(nCustomFieldIndex, strAns);
	} else {
		return FALSE;
	}
}

//DRT 2/3/2004 - PLID 10867 - Fixed a bug that was causing issues if you were missing records in CustomFieldsT.
void CFilterDlg::FormatFieldNameApparent(CString &strFieldNameApparent)
{
	//this can't take parentheses as part of the base name
	if (strncmp(strFieldNameApparent, "REPLACE_FIELD_NAME(", 19) == 0) {
		try {
			long nCloseParenIndex = strFieldNameApparent.Find(')', 19);
			if (nCloseParenIndex > 19) {
				//Have we already looked this up?
				CString strName;
				if(!m_mapReplaceFieldNames.Lookup(strFieldNameApparent.Mid(nCloseParenIndex+2), strName)) {
					//Nope, let's look it up.
					// (b.cardillo 2006-05-19 18:09) - PLID 20593 - But don't look it up if we've 
					// already got it cached (i.e. if it's a query for a custom field name).
					if (!CheckCacheIfCustomFieldName(strFieldNameApparent.Mid(nCloseParenIndex+2), strName)) {
						_RecordsetPtr prs = CreateRecordsetStd(strFieldNameApparent.Mid(nCloseParenIndex+2));
						if(!prs->eof)
							strName = AdoFldString(prs, "Name");
						else
							strName = strFieldNameApparent.Mid(19, nCloseParenIndex-19);	//if we can't find a record, just pull the name from our strFieldNameApparent
					}
					m_mapReplaceFieldNames.SetAt(strFieldNameApparent.Mid(nCloseParenIndex+2), strName);
				}

				/*This was here, but was taken out.  It looks like someone changed their mind about putting the 
				base name in.  I added functionality to allow a category in, not just a base name -JMM*/
				//CString strBaseName = m_strFieldNameApparent.Mid(19, nCloseParenIndex-19);
				//return strBaseName + " - " + AdoFldString(CreateRecordsetStd(m_strFieldNameApparent.Mid(nCloseParenIndex+2))->Fields, "Name");

				//look for a comma before the end parenthesis
				long nCommaIndex = strFieldNameApparent.Find(',', 19);
				if (nCommaIndex > 0) {
					CString strBaseName = strFieldNameApparent.Mid(19, (nCommaIndex - 19));
					strFieldNameApparent = strBaseName + " - " + strName;
				}
				else {
					strFieldNameApparent = strName;
				}
			} else if (nCloseParenIndex == 19) {
				// Nothing in parentheses
				//Have we already looked this up?
				CString strName;
				if(!m_mapReplaceFieldNames.Lookup(strFieldNameApparent.Mid(nCloseParenIndex+2), strName)) {
					//Nope, let's look it up.
					_RecordsetPtr prs = CreateRecordsetStd(strFieldNameApparent.Mid(nCloseParenIndex+2));
					if(!prs->eof)
						strName = AdoFldString(prs, "Name", "Custom Field");
					else
						strName = strFieldNameApparent.Mid(19, nCloseParenIndex-19);	//if we can't find a record, just pull the name from our strFieldNameApparent
					m_mapReplaceFieldNames.SetAt(strFieldNameApparent.Mid(nCloseParenIndex+2), strName);
				}
				strFieldNameApparent = strName;
			} else {
				// No closing parentheses!
				ASSERT(FALSE);
				strFieldNameApparent = strFieldNameApparent.Mid(19);
			}
		} NxCatchAll("CFilterDlg::FormatFieldNameApparent");
	}
}
