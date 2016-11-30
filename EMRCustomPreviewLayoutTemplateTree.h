#pragma once

#include <NxAdvancedUILib/NxDockablePane.h>
#include <NxUILib/SafeMsgProc.h>
#include <NxDataUtilitiesLib/NxSafeArray.h>

// CEMRCustomPreviewLayoutTemplateTree
// (c.haag 2013-01-16) - PLID 54737 - This is a pane used in the Html MDI frame
// that lists all the available template topics and details for adding to the layout.

class CEMRCustomPreviewLayoutTemplateTree : public SafeMsgProc<CDockablePane>
{
	DECLARE_DYNAMIC(CEMRCustomPreviewLayoutTemplateTree)

private:
	// The tree control
	CWnd m_wndTree;
	NXDATALIST2Lib::_DNxDataListPtr m_Tree;

private:
	// Used for drag-and-drop operations
	COleDataSource m_dataSource;
	COleDropSource m_dropSource;

private:
	// The ID of the EMR template topic to query
	long m_nEmrTemplateID;
	CString m_strEmrTemplateName;

private:
	BOOL m_bShowHiddenItems;

public:
	CEMRCustomPreviewLayoutTemplateTree(long nEmrTemplateID, const CString& strEmrTemplateName);

public:
	inline BOOL GetShowHiddenItems() const { return m_bShowHiddenItems; }
	void SetShowHiddenItems(BOOL bShow);

	// (z.manning 2013-03-12 15:31) - PLID 55595
	void ExpandOrCollapseAllTopics(BOOL bExpand);

protected:
	// Creates the tree control
	void CreateControl();
	// Populates the tree control with data
	void RefreshControl();
	// Clears the tree
	void ClearTree();
	// (c.haag 2013-03-13) - PLID 55611 - Adds built-in fields to the tree (such as Signature)
	void AddBuiltInFieldsToTree();
	// (c.haag 2013-03-13) - PLID 55611 - Adds a built-in field to the tree (such as Signature)
	void AddBuiltInFieldToTree(NXDATALIST2Lib::IRowSettingsPtr pMasterRow, OLE_COLOR clr, const CString& strField);
	// (c.haag 2013-03-19) - PLID 55697 - Adds Letter Writing fields to the tree
	void AddLWFieldsToTree();
	// Populates the tree control with data
	void PopulateTreeRecurse(Nx::SafeArray<IUnknown *> saTopics);

protected:
	// (c.haag 2013-03-13) - PLID 55611 - Returns TRUE if the row is a built-in field
	BOOL IsBuiltInField(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (c.haag 2013-03-19) - PLID 55697 - Returns TRUE if the row is a letter writing field
	BOOL IsLWField(NXDATALIST2Lib::IRowSettingsPtr pRow);

protected:
	// Generate HTML from a tree row. It should either be a topic or a detail; and we will get
	// the appropriate text depending on what it is
	CString GenerateHtml(NXDATALIST2Lib::IRowSettingsPtr pRow);

protected:
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

	virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnSize(UINT nType, int cx, int cy);
	virtual void OnDestroy();
	virtual BOOL OnShowControlBarMenu(CPoint point);

protected:
	// (c.haag 2013-01-21) - PLID 54738 - Added support for drag and drop
	void OnDragBeginTree(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
};


