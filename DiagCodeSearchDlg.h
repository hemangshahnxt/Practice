#pragma once

#include "WindowlessTimer.h"

// (j.armen 2014-03-20 09:58) - PLID 60943 - Created to allow for searching of diag codes in a pop-up style.
// This mechanism is fairly generic, and could be expanded to allow for multi select, and searching of 10 codes as well

class CDiagCodeSearchDlg : CNxDialog
{
public:
	static boost::optional<class DiagCode9> DoManagedICD9Search(CWnd* pParent);

protected:
	CDiagCodeSearchDlg(CWnd* pParent);
	~CDiagCodeSearchDlg();

	virtual BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX) override;
	virtual int SetControlPositions() override;

	void TriggerAsyncSearch();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnSearchModified();
	afx_msg LRESULT OnMessageSearchResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNcHitTest(CPoint point);

	DECLARE_EVENTSINK_MAP()
	void DblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void SelSetUnselectedList(LPDISPATCH lpRow);

private:
	CNxColor m_nxcolor;
	CNxEdit m_editSearch;
	WindowlessTimer::InstancePtr m_TimerInstance;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUnselected;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	void SetControlState();

	shared_ptr<struct AsyncDiagSearchQuery> m_pData;		// The current set of search data. Ready for use.
	shared_ptr<struct AsyncDiagSearchQuery> m_pPendingData;	// The pending set of search data. Not ready for use.
};