#pragma once

#include "WindowlessTimer.h"

// (j.armen 2014-03-10 08:33) - PLID 61210 - Created
// (j.armen 2014-03-10 08:33) - PLID 61284 - Methods for data search / loading
class CImportICD10CodesDlg : CNxDialog
{
public:
	CImportICD10CodesDlg(CWnd* pParent);
	~CImportICD10CodesDlg();

	int DoModal() override;

protected:
	virtual BOOL OnInitDialog() override;
	virtual void DoDataExchange(CDataExchange* pDX) override;
	virtual int SetControlPositions() override;

	void TriggerAsyncSearch();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnSearchModified();
	afx_msg LRESULT OnMessageSearchResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedMoveRight();
	afx_msg void OnBnClickedMoveLeft();
	afx_msg void OnBnClickedMoveLLeft();
	afx_msg LRESULT OnNcHitTest(CPoint point);

	DECLARE_EVENTSINK_MAP()
	void DblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex);
	void SelSetUnselectedList(LPDISPATCH lpRow);
	void SelSetSelectedList(LPDISPATCH lpRow);

private:
	CNxColor m_nxcolor;
	CNxEdit m_editSearch;
	WindowlessTimer::InstancePtr m_TimerInstance;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUnselected;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSelected;
	CNxIconButton m_btnMoveRight;
	CNxIconButton m_btnMoveLeft;
	CNxIconButton m_btnMoveLLeft;
	CNxIconButton m_btnImport;
	CNxIconButton m_btnCancel;

	void SetControlState();

	// (j.armen 2014-03-20 09:47) - PLID 60943 - Renamed struct
	shared_ptr<struct AsyncDiagSearchQuery> m_pData;		// The current set of search data. Ready for use.
	shared_ptr<struct AsyncDiagSearchQuery> m_pPendingData;	// The pending set of search data. Not ready for use.
};
