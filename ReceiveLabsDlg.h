#pragma once

#include "Client.h"

// CReceiveLabsDlg dialog
// (a.vengrofski 2010-07-22 11:42) - PLID <38919> - Created this file.

class CReceiveLabsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReceiveLabsDlg)

public:
	CReceiveLabsDlg(CWnd* pParent);   // standard constructor
	virtual ~CReceiveLabsDlg();

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	enum { IDD = IDD_HL7_RECEIVE_LABS_DLG };
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnReceiveLabs;
	// (r.gonet 03/01/2013) - PLID 48419 - Button to dismiss selected result messages.
	CNxIconButton	m_btnDismissLabResults;
	CNxIconButton	m_btnHL7Settings;
	CNxIconButton	m_btnHL7NewMessages;
	// (r.gonet 05/01/2014) - PLID 49432 - Button to access the HL7 log
	CNxIconButton	m_btnHL7Log;

	// (j.dinatale 2011-09-16 15:37) - PLID 40024
	NxButton m_btnShowPrevImported;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlUnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSelectedList; 
	NXDATALIST2Lib::_DNxDataListPtr m_pdcGroupFilterCombo;

	// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderFilterCombo;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void PopulateLabLists();
	void MoveToOtherList(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlDest);
	// (r.gonet 02/26/2013) - PLID 48419 - Dismisses a lab result message if you are the NexTech Technical Support.
	bool DismissMessage(long nMessageID, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::_DNxDataListPtr pdlSource);
	CString GetMessageActionDescription(long nActionID);
	void ProcessHL7Files(BOOL bSilent);

	// (b.savon 2011-09-28 10:28) - PLID 42805 - Be able to filter pending lab imports by provider
	void FilterLabListsByProvider();
	void PopulateHL7LabProviders(CString strMessageIDs, CArray<CString, LPCSTR> &aryProviderNames, CArray<CString, LPCSTR> &aryProviderIDs);
	CTableChecker m_providerChecker;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenHl7ReceiveLabsGroupFilter(LPDISPATCH lpRow);
	void RequeryFinishedHl7ReceiveLabsGroupFilter(short nFlags);
	afx_msg void OnBnClickedBtnHl7Settings();
	afx_msg void OnBnClickedBtnReceiveLabs();
	void DblClickCellHl7ReceiveLabsUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellHl7ReceiveLabsSelectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedSelectOneHl7ReceiveLab();
	afx_msg void OnBnClickedSelectAllHl7ReceiveLab();
	afx_msg void OnBnClickedUnselectOneHl7ReceiveLab();
	afx_msg void OnBnClickedUnselectAllHl7ReceiveLab();
	afx_msg void OnBnClickedBtnCheckForMessages();
	void CheckAndMoveRows();
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	void RButtonDownHl7ReceiveLabsUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownHl7ReceiveLabsSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedShowprevimportedlabs();
	void SelChosenProvidersFilter(LPDISPATCH lpRow);
	// (r.gonet 03/01/2013) - PLID 48419 - Handles button click to dismiss selected results.
	afx_msg void OnBnClickedDismissHl7LabResultsBtn();
	// (r.gonet 05/01/2014) - PLID 49432 - Handles button click to open the HL7 log dialog.
	afx_msg void OnBnClickedOpenHl7LogBtn();
};
