#pragma once

#include <NxUILib/NxHtmlControl.h>

// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing control

struct GraphInfo;

class CEmrGraphingCtrl : public CNxHtmlControl
{
	DECLARE_DISPATCH_MAP();
	DECLARE_MESSAGE_MAP();
public:
	CEmrGraphingCtrl();
	~CEmrGraphingCtrl();

protected:
	/// CNxHtmlControl virtual	
	virtual void OnInitializing();

	/// Internal methods

	/// DocHostUIHandler virtual
	virtual HRESULT OnShowContextMenu(DWORD dwID, LPPOINT ppt,
		LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved);

	/// IDispatch methods
	void Configure();
	
	// (a.walling 2012-05-01 09:41) - PLID 50090 - Return the available graphs
	BSTR GetAvailableGraphsJson();
	// (a.walling 2012-05-01 09:41) - PLID 50090 - Notified when the list of visible graphs changes
	void UpdateVisibleGraphs(LPCTSTR szGraphIDs);
	// (a.walling 2012-05-02 12:17) - PLID 50156 - Now we keep track of hidden / closed graphs as compared to collapsed graphs
	void UpdateHiddenGraphs(LPCTSTR szGraphIDs);
	// (a.walling 2012-05-02 18:12) - PLID 50157 - Now we also keep track of the sort order
	void UpdateGraphSortOrder(LPCTSTR szGraphIDs);
	// (a.walling 2012-05-01 09:41) - PLID 50090 - Return the list of visible graphs
	BSTR GetVisibleGraphs();

	VARIANT LoadGraphData(long nGraphID);

	void InvalidateGraphInfo();
	shared_ptr<GraphInfo> GetGraphInfo();
	shared_ptr<GraphInfo> m_pGraphInfo;

	// (a.walling 2012-05-08 16:04) - PLID 50105 - Print handling
	afx_msg void OnPrint();
	afx_msg void OnPrintPreview();

	void DoPrint(bool bPreview);

	// (a.walling 2012-05-08 16:04) - PLID 50105 - Context menu
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

};
