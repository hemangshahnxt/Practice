#pragma once
#include "SchedulerRc.h"

class CRoomManagerDlg;
// (d.lange 2010-06-28 12:25) - PLID 37317 - created
// CRoomManagerConfigureColumnsDlg dialog

class CRoomManagerConfigureColumnsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRoomManagerConfigureColumnsDlg)

public:
	CRoomManagerConfigureColumnsDlg(CRoomManagerDlg* pParent = NULL);   // standard constructor
	virtual ~CRoomManagerConfigureColumnsDlg();
	
	CNxIconButton m_btnOK;

// Dialog Data
	enum { IDD = IDD_ROOM_MANAGER_CONFIG_COL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void LoadWaitingAreaColumns();
	void LoadRoomsColumns();
	void LoadCheckoutColumns();

	NXDATALIST2Lib::_DNxDataListPtr m_pWaitingAreaColumns;
	NXDATALIST2Lib::_DNxDataListPtr m_pRoomsColumns;
	NXDATALIST2Lib::_DNxDataListPtr m_pCheckoutColumns;

	CRoomManagerDlg *m_pParentRoomManager;

	CStringArray m_aryWaitingAreaColumnNames;
	CStringArray m_aryRoomsColumnNames;
	CStringArray m_aryCheckoutColumnNames;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedWaitingAreaColumns(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishedRoomsColumns(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishedCheckoutColumns(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
