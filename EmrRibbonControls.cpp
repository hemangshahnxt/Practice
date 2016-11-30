#include "StdAfx.h"

#include "EmrRibbonControls.h"

// (a.walling 2012-10-01 09:04) - PLID 52119 - EmrRibbonControls - Ribbon controls with at least some EMR-specific functionality

// (a.walling 2012-09-11 11:16) - PLID 52581 - Specifically for the templates, this splits the filename from the path
// and displays the path right-aligned in a second column

BOOL CNxRibbonMergeTemplateComboBox::OnDrawDropListItem(CDC* pDC, int nIndex, CMFCToolBarMenuButton* pItem, BOOL bHighlight)
{
	CRect rect = pItem->Rect();
	rect.DeflateRect(2 * AFX_TEXT_MARGIN, 0);

	if (pItem->m_strText == m_strHighlightName) {
		COLORREF clrNormal = CMFCVisualManager::GetInstance()->GetMenuItemTextColor(pItem, bHighlight, FALSE);
		COLORREF clrHighlight = lerpColor(clrNormal, HEXRGB(0x0000FF), 0.75);
		pDC->SetTextColor(clrHighlight);
	}

	CString strText = FileUtils::GetFileName(pItem->m_strText);
	CString strSecondText = FileUtils::GetFilePath(pItem->m_strText);

	strText.Trim("\\/");
	strSecondText.Trim("\\/");
	
	CRect rectFirst = rect;
	pDC->DrawText(strText, &rectFirst, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CALCRECT);
	
	pDC->DrawText(strText, &rect, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	
	if (!strSecondText.IsEmpty()) {

		CRect rectSecond = rect;
		pDC->DrawText(strSecondText, &rectSecond, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CALCRECT);

		// right align now
		rect.left = rect.right - rectSecond.Width();

		if (rect.left < (rectFirst.right + 32)) {
			rect.left = rectFirst.right + 32;
		}

		pDC->DrawText(strSecondText, &rect, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_PATH_ELLIPSIS);
	}

	return TRUE;
}

CSize CNxRibbonMergeTemplateComboBox::OnGetDropListItemSize(CDC* pDC, int nIndex, CMFCToolBarMenuButton* pItem, CSize sizeDefault)
{
	CSize size = sizeDefault;
	size.cx += 32;

	return size;
}

void CNxRibbonMergeTemplateComboBox::CopyFrom(const CMFCRibbonBaseElement& s)
{
	__super::CopyFrom(s);

	CNxRibbonMergeTemplateComboBox& src = (CNxRibbonMergeTemplateComboBox&) s;

	m_strHighlightName = src.m_strHighlightName;
	// (b.cardillo 2013-07-24 20:47) - PLID 52844
	m_bSearchListByFileName = src.m_bSearchListByFileName;
}

void CNxRibbonMergeTemplateComboBox::SetHighlightName(const CString& strHighlightName)
{ 
	if (m_strHighlightName == strHighlightName) {
		return;
	}

	m_strHighlightName = strHighlightName;

	if (::IsWindow(m_pPopupMenu->GetSafeHwnd())) {
		m_pPopupMenu->Invalidate();
	}
}

IMPLEMENT_DYNCREATE(CNxRibbonMergeTemplateComboBox, CNxRibbonAutoComboBox)

