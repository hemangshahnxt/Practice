#pragma once

// (z.manning 2013-05-20 11:20) - PLID 56777 - Renamed
class CHL7Client_Practice;

// CSendLabsDlg dialog
// (a.vengrofski 2010-05-28 14:40) - PLID <38919> - Created this file.

class CSendLabsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSendLabsDlg)

public:
	CSendLabsDlg(CWnd* pParent);   // standard constructor
	virtual ~CSendLabsDlg();

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	enum { IDD = IDD_HL7_SEND_LABS_DLG };
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSendLabs;
	CNxIconButton	m_btnHL7Settings;
	CNxIconButton	m_btnExportLabResults; // (d.singleton 2012-12-14 12:32) - PLID 53282

protected:
	// (c.haag 2010-08-11 09:55) - PLID 39799
	HANDLE  m_hIconRedX;

	// (r.gonet 12/11/2012) - PLID 54116 - Manages transmissions of HL7 between us and NxServer.
	// (z.manning 2013-05-20 11:21) - PLID 56777 - Renamed
	CHL7Client_Practice *m_pHL7Client;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlUnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSelectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdcGroupFilterCombo;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void PopulateLabLists();
	void MoveToOtherList(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource, NXDATALIST2Lib::_DNxDataListPtr pdlDest);
	// (r.gonet 02/26/2013) - PLID 47534 - Dismisses a lab order message that is pending export.
	void DismissMessage(long nMessageID, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource);

	// (z.manning 2011-07-08 16:56) - PLID 38753
	// (r.gonet 02/26/2013) - PLID 47534 - Added pRow and pdlSource
	void PopupMenu(const long nMessageID, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnSendLabs();
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedSelectOneHl7SendLab();
	afx_msg void OnBnClickedSelectHl7AllSendLab();
	afx_msg void OnBnClickedUnselectHl7OneSendLab();
	afx_msg void OnBnClickedUnselectHl7AllSendLab();
	// (d.singleton 2012-10-19 14:13) - PLID 53282 add dialog to export lab results
	afx_msg void OnBnClickedExportLabResults();
	void CheckAndMoveRows();
	void SelChosenHl7SendLabsGroupFilter(LPDISPATCH lpRow);
	void DblClickCellHl7SendLabsUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellHl7SendLabsSelectedList(LPDISPATCH lpRow, short nColIndex);
	void RequeryFinishedHl7SendLabsGroupFilter(short nFlags);
	afx_msg void OnBnClickedBtnHl7Settings();
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	void LeftClickHl7SendLabsUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickHl7SendLabsSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (z.manning 2011-07-08 16:54) - PLID 38753 - Added right click handlers
	void RButtonDownHl7SendLabsUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownHl7SendLabsSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
