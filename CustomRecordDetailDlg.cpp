// CustomRecordDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CustomRecordDetailDlg.h"
#include "CustomRecordItemDlg.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordDetailDlg dialog


CCustomRecordDetailDlg::CCustomRecordDetailDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCustomRecordDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomRecordDetailDlg)
		m_ProcedureID = -1;
		m_EMRID = -1;
		m_nScrolledHeight = 0;
	//}}AFX_DATA_INIT
}

CCustomRecordDetailDlg::~CCustomRecordDetailDlg()
{
	for(int i = m_aryEMRItemDlgs.GetSize() - 1; i>=0; i--) {
		//DRT 12/17/2004 - PLID 15001 - MUST cleanup after ourselves!
		CCustomRecordItemDlg* pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i));
		pItem->DestroyWindow();
		delete pItem;
	}
	m_aryEMRItemDlgs.RemoveAll();
}


void CCustomRecordDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordDetailDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomRecordDetailDlg, CDialog)
	//{{AFX_MSG_MAP(CCustomRecordDetailDlg)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordDetailDlg message handlers

BOOL CCustomRecordDetailDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ShowScrollBar(SB_VERT, TRUE);

	RefreshScrollBar();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CCustomRecordDetailDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_EMR_ITEM_CHANGED:
		{
			if(((long)lParam) == -1) {
				//if it is not the last row, we're removing a detail
				long index = ((long)wParam);
				if(index != m_aryEMRItemDlgs.GetSize() - 1) {
					DeleteEMRItem(index);
				}
				RefreshScrollBar();
			}
			else {
				//for some as-yet-unknown reason, hiding and invalidating a datalist in the child doesn't work,
				//so you have to invalidate from the parent
				CRect rect;
				((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(wParam))->GetWindowRect(rect);
				ScreenToClient(rect);
				InvalidateRect(rect);

				//if we changed the last (empty) row, then we add a new one
				if(((long)wParam) == m_aryEMRItemDlgs.GetSize() - 1) {
					AddDetail();					
				}

				//send to the main window so it can process action items
				GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, lParam);
			}
		}
		break;
	case NXM_EMR_DATA_CHANGED:
		{
			//send to the main window so it can process action items
			GetParent()->SendMessage(NXM_EMR_DATA_CHANGED, wParam);
		}
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CCustomRecordDetailDlg::AddDetail(long InfoID /* = -1 */, long DetailID /* = -1 */, BOOL bIsNew /* = TRUE*/)
{
	CCustomRecordItemDlg *pItemDlg = new CCustomRecordItemDlg(this);

	long nOldSize = m_aryEMRItemDlgs.GetSize();

	if(pItemDlg) {

		pItemDlg->m_strInfoWhereClause = m_strInfoWhereClause;

		CRect rect;
		GetWindowRect(rect);
		ScreenToClient(rect);

		int left = rect.left + 2;
		int top = rect.top + 2;
		
		//get the bottom of the last item
		if(m_aryEMRItemDlgs.GetSize() > 0) {
			((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(m_aryEMRItemDlgs.GetSize()-1))->GetWindowRect(rect);
			ScreenToClient(rect);
			top = rect.bottom;
		}

		pItemDlg->Create(IDD_CUSTOM_RECORD_ITEM_DLG, this);
		pItemDlg->GetWindowRect(rect);
		pItemDlg->MoveWindow(left,top,rect.Width(),rect.Height());
		pItemDlg->BringWindowToTop();

		m_aryEMRItemDlgs.Add(pItemDlg);

		pItemDlg->ShowWindow(SW_SHOW);

		pItemDlg->m_index = m_aryEMRItemDlgs.GetSize() - 1;

		if(InfoID != -1)
			pItemDlg->SendMessage(NXM_EMR_CHANGE_ITEM, InfoID, bIsNew ? 1 : 0);

		if(DetailID != -1) {
			pItemDlg->SendMessage(NXM_EMR_LOAD_DETAIL, DetailID, bIsNew ? 1 : 0);
		}
		else {
			//TES 12/23/2004 - NXM_EMR_CHANGE_ITEM will have done this already.
			/*//if -1, see if a default ID exists
			_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 EmrDataID AS DefaultData "
				"FROM EMRInfoT INNER JOIN EmrInfoDefaultsT ON EmrInfoT.ID = EmrInfoDefaultsT.EmrInfoID "
				"WHERE EmrInfoT.ID = %li",InfoID);
			if(!rs->eof) {
				_variant_t var = rs->Fields->Item["DefaultData"]->Value;
				if(var.vt == VT_I4)
					pItemDlg->SendMessage(NXM_EMR_SET_DEFAULT_DATA, (long)var.lVal);
			}
			rs->Close();
			*/
		}		
	}

	long nNewSize = m_aryEMRItemDlgs.GetSize();

	RefreshScrollBar();

	if(nOldSize < nNewSize) {
		DoScrollTo(CUSTOM_RECORD_SCROLL_BOTTOM_POS);
	}
}

void CCustomRecordDetailDlg::TryAddBlankDetail() {

	BOOL bAddDetail = FALSE;

	//if there are no items in the list, add it
	if(m_aryEMRItemDlgs.GetSize() == 0)
		bAddDetail = TRUE;

	//if there are items in the list, ensure the last one is blank, if not, add it
	if(!bAddDetail) {
		long index = m_aryEMRItemDlgs.GetSize() - 1;
		CCustomRecordItemDlg *pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(index));
		long nTmp;
		int iTmp;
		CString strTmp;
		if(ITEM_NULL != pItem->GetItemInfo(nTmp, iTmp, nTmp, strTmp)) {
			bAddDetail = TRUE;
		}
	}

	//now do it
	if(bAddDetail) {
		AddDetail();
	}
}

void CCustomRecordDetailDlg::TryRemoveBlankDetail() {

	if(m_aryEMRItemDlgs.GetSize() > 0) {
		long index = m_aryEMRItemDlgs.GetSize() - 1;
		CCustomRecordItemDlg *pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(index));
		//ensure it is blank
		if(pItem->m_InfoCombo->GetCurSel() == -1 || pItem->m_InfoCombo->GetValue(pItem->m_InfoCombo->GetCurSel(),0).lVal == -1) {
			//DRT 12/17/2004 - PLID 15001 - MUST cleanup after ourselves!
			pItem->DestroyWindow();
			delete pItem;
			m_aryEMRItemDlgs.RemoveAt(index);
		}
	}		
}

void CCustomRecordDetailDlg::ClearDetails()
{
	for(int i = m_aryEMRItemDlgs.GetSize() - 1; i>=0; i--) {

		//check to see if this is a previously saved detail, and if so, mark it to be deleted on save
		long DetailID = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i))->m_ID;					
		if(DetailID != -1)
			m_aryDeletedDetails.Add((long)DetailID);

		//DRT 12/17/2004 - PLID 15001 - MUST cleanup after ourselves!
		CCustomRecordItemDlg* pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i));
		pItem->DestroyWindow();
		delete pItem;
	}
	m_aryEMRItemDlgs.RemoveAll();

	RefreshScrollBar();
}

inline long CalcVertHeight(long nPos) {
	return (((nPos / EMR_SCROLL_POS_PER_ITEM) * EMR_ITEM_SIZE_VERT) + ((nPos % EMR_SCROLL_POS_PER_ITEM) * EMR_SCROLL_POS_HEIGHT));
}

long CCustomRecordDetailDlg::DoScrollTo(long nNewTopPos)
{
	// Make sure the new position is not out of range
	if (nNewTopPos > CUSTOM_RECORD_SCROLL_BOTTOM_POS) nNewTopPos = CUSTOM_RECORD_SCROLL_BOTTOM_POS;
	if (nNewTopPos < EMR_SCROLL_TOP_POS) nNewTopPos = EMR_SCROLL_TOP_POS;

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

void CCustomRecordDetailDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (nSBCode) {
	case SB_LINEUP:
		DoScrollTo(GetScrollPos(SB_VERT)-1);
		break;
	case SB_LINEDOWN:
		DoScrollTo(GetScrollPos(SB_VERT)+1);
		break;
	case SB_PAGEUP:
		DoScrollTo(GetScrollPos(SB_VERT)-EMR_SCROLL_POS_PER_PAGE);
		break;
	case SB_PAGEDOWN:
		DoScrollTo(GetScrollPos(SB_VERT)+EMR_SCROLL_POS_PER_PAGE);
		break;
	case SB_TOP:
		DoScrollTo(EMR_SCROLL_TOP_POS);
		break;
	case SB_BOTTOM:
		DoScrollTo(CUSTOM_RECORD_SCROLL_BOTTOM_POS);
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

void CCustomRecordDetailDlg::RefreshScrollBar()
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_DISABLENOSCROLL|SIF_PAGE|SIF_RANGE;
	si.nMin = 0;
	si.nMax = m_aryEMRItemDlgs.GetSize() * EMR_SCROLL_POS_PER_ITEM;
	si.nPage = EMR_SCROLL_POS_PER_PAGE;
	SetScrollInfo(SB_VERT, &si, TRUE);
}

void CCustomRecordDetailDlg::RefreshAllItems()
{
	CWaitCursor pWait;

	LoadInfoWhereClause();

	// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
	int i = 0;

	for(i=0;i<m_aryEMRItemDlgs.GetSize();i++) {
		((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i))->RefreshList(m_strInfoWhereClause);
	}

	//now clean out any invalid items (some may have been removed)
	//ignore the blank line at the bottom
	for(i=m_aryEMRItemDlgs.GetSize()-2;i>=0;i--) {
		CCustomRecordItemDlg *pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i));
		if(pItem->m_InfoCombo->GetCurSel() == -1 || pItem->m_InfoCombo->GetValue(pItem->m_InfoCombo->GetCurSel(),0).lVal == -1)
			DeleteEMRItem(i);
	}
}

void CCustomRecordDetailDlg::AddDiagCodeID(long DiagID) {

	m_dwDiagCodeIDs.Add(DiagID);
}

void CCustomRecordDetailDlg::RemoveDiagCodeID(long DiagID) {

	for(int i=0;i<m_dwDiagCodeIDs.GetSize();i++) {
		if((long)(m_dwDiagCodeIDs.GetAt(i)) == DiagID) {
			m_dwDiagCodeIDs.RemoveAt(i);
		}
	}	
}

void CCustomRecordDetailDlg::LoadInfoWhereClause() {

	try {

		CString str;
		str.Format("ProcedureID = %li OR ",m_ProcedureID);
		for(int i=0; i < m_dwDiagCodeIDs.GetSize(); i++) {
			CString strAdd;
			strAdd.Format("DiagCodeID = %li OR ",m_dwDiagCodeIDs.GetAt(i));
			str += strAdd;
		}
		str.TrimRight(" OR ");

		m_strInfoWhereClause = str;

	}NxCatchAll("Error loading EMR detail information.");
}

void CCustomRecordDetailDlg::DeleteEMRItem(long index) {

	//check to see if this is a previously saved detail, and if so, mark it to be deleted on save
	CCustomRecordItemDlg* pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(index));
	long DetailID = pItem->m_ID;			
	if(DetailID != -1)
		m_aryDeletedDetails.Add((long)DetailID);

	DoScrollTo(GetScrollPos(SB_VERT)-EMR_SCROLL_POS_PER_ITEM);

	pItem->DestroyWindow();
	delete pItem;
	m_aryEMRItemDlgs.RemoveAt(index);

	for(int i=0;i<m_aryEMRItemDlgs.GetSize();i++) {
		if(i >= index) {
			((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i))->m_index--;
			CRect rect;
			((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i))->GetWindowRect(rect);
			ScreenToClient(rect);
			((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i))->MoveWindow(rect.left,rect.top - rect.Height(),rect.Width(),rect.Height());
		}
	}
	RefreshScrollBar();
}

BOOL CCustomRecordDetailDlg::IsInfoInList(long InfoID) {

	for(int i=0;i<m_aryEMRItemDlgs.GetSize();i++) {
		CCustomRecordItemDlg *pItem = ((CCustomRecordItemDlg*)m_aryEMRItemDlgs.GetAt(i));
		if(pItem->m_InfoCombo->GetCurSel() != -1 && pItem->m_InfoCombo->GetValue(pItem->m_InfoCombo->GetCurSel(),0).lVal == InfoID) {
			//found it
			return TRUE;
		}
	}

	return FALSE;
}
