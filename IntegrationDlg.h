#pragma once


#include "stdafx.h"
#include "Practice.h"
#include "SupportUpgradeHistoryDlg.h"

//(j.deskurakis 2013-01-24) - PLID 53151 - Added columns to the dialog
// (f.dinatale 2010-07-07) - PLID 39527 - Added the Integration Info Dialog
// CIntegrationDlg dialog
enum IntegrationList
{
	dlcIntegrationID = 0,
	dlcIncidentID,
	dlcIntegrationCompany,
	dlcIntegrationExpiration,
	dlcIntegrationLabType,
	dlcIntegrationInterface,
	dlcIntegrationNotes, 
	dlcIntegrationAssigned,
	dlcIntegrationStart,
	dlcIntegrationTrained,
	dlcIntegrationGoLive,
	dlcIntegrationStatus,
	dlcIntegrationPO,
	dlcIntegrationPaid,






};

//(j.deskurakis 2013-01-24) - PLID 53151 - Added IntegrationBillToT
// (f.dinatale 2010-07-07) - PLID 39527 - Enum for the selection of which table to modify.
enum IntegrationTables
{
	eIntegrationBillToT,
	eIntegrationLabTypesT,
};

class CIntegrationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CIntegrationDlg)

public:
	CIntegrationDlg(CWnd* pParent = NULL, long nClientID = -1);   // standard constructor
	virtual ~CIntegrationDlg();

	// Dialog Data
	enum { IDD = IDD_INTEGRATION_INFO_DLG };

	//(j.deskurakis 2013-01-24) - PLID 53151 - Added OnClose and OnEditIntegrationBillTo
	// (f.dinatale 2010-07-07) - PLID 39527
	// Message Handlers
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnClose();
	afx_msg void OnEditIntegrationBillTo();
	afx_msg void OnEditLabTypes();
	afx_msg void OnIntegrationAddtodo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	//(j.deskurakis 2013-01-24) - PLID 53151 - changed to save on the fly by row
	// (f.dinatale 2010-08-13) - PLID 39527 - Refactored data saving.
	void SaveData(NXDATALIST2Lib::IRowSettingsPtr pRow);
	//(j.deskurakis 2013-01-24) - PLID 53151 - added close and billTo
	// (f.dinatale 2010-07-07) - PLID 39527
	void EditTables(long tableEnum = -1);

	NXDATALIST2Lib::_DNxDataListPtr m_dlcIntegration; // List of HL7 Integrations
	CList<long> m_listRemoved;
	CNxIconButton m_nxbAdd;
	CNxIconButton m_nxbDelete;
	CNxIconButton m_nxbClose;
	CNxIconButton m_nxbEditBillTo;
	CNxIconButton m_nxbEditLabTypes;

	// (f.dinatale 2010-07-15) - PLID 39565 - Added the ability to create To-Do Alarms.
	CNxIconButton m_nxbAddToDo;
	long m_nClientID;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void OnCurSelWasSet();
	void OnEditingFinishing(LPDISPATCH lpRow, short nCol, VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	//(j.deskurakis 2013-01-24) - PLID 53151 - added to handle hyperlink and save on the fly 
public:
	void EditingFinishedIntegration(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void LButtonDownIntegration(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGoToClient();

};
