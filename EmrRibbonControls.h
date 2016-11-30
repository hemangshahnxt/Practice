#pragma once

#include <NxAdvancedUILib/NxRibbonControls.h>

#include <NxDataUtilitiesLib/iString.h>

// (a.walling 2012-10-01 09:04) - PLID 52119 - EmrRibbonControls - Ribbon controls with at least some EMR-specific functionality

// (a.walling 2012-09-11 11:16) - PLID 52581 - Specifically for the templates, this splits the filename from the path
// and displays the path right-aligned in a second column
class CNxRibbonMergeTemplateComboBox 
	: public CNxRibbonAutoComboBox
{
	DECLARE_DYNCREATE(CNxRibbonMergeTemplateComboBox);
public:
	CNxRibbonMergeTemplateComboBox(UINT nID, BOOL bHasEditBox = TRUE, int nWidth = -1, LPCTSTR lpszLabel = NULL, int nImage = -1)
		: CNxRibbonAutoComboBox(nID, bHasEditBox, nWidth, lpszLabel, nImage)
	{
		// (b.cardillo 2013-07-24 20:47) - PLID 52844 - Allow user to search by template name under PIC (merge)
		m_bSearchListByFileName = TRUE;
	}

	CiString m_strHighlightName;

	virtual void CopyFrom(const CMFCRibbonBaseElement& s) override;

	void SetHighlightName(const CString& strHighlightName);

	virtual BOOL OnDrawDropListItem(CDC* pDC, int nIndex, CMFCToolBarMenuButton* pItem, BOOL bHighlight);

	virtual CSize OnGetDropListItemSize(CDC* pDC, int nIndex, CMFCToolBarMenuButton* pItem, CSize sizeDefault);

protected:
	CNxRibbonMergeTemplateComboBox()
		: CNxRibbonAutoComboBox()
	{
		// (b.cardillo 2013-07-24 20:47) - PLID 52844 - Allow user to search by template name under PIC (merge)
		m_bSearchListByFileName = TRUE;
	}
};
