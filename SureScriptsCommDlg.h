#pragma once

#include "SureScriptsPractice.h"

// SureScriptsCommDlg dialog
//TES 4/7/2009 - PLID 33882 - Adam created this dialog a while back, but it was empty and unreferenced, 
// I'm now actually implementing this dialog.

class CSureScriptsCommDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSureScriptsCommDlg)

public:
	CSureScriptsCommDlg(CWnd* pParent);   // standard constructor
	virtual ~CSureScriptsCommDlg();

	//TES 4/14/2009 - PLID 33888 - Set this before opening the dialog to default the filter options (if this function
	// is not called, or if an empty array is passed in, the dialog will filter on all providers).
	// NOTE: This function takes an array of ints rather than longs as a convenience, as they're stored in ConfigRT
	// which returns an int array.
	void PrefilterProviders(const CArray<int,int> &arProviderIDs);
	
// Dialog Data
	enum { IDD = IDD_SURESCRIPTS_COMM_DLG };

protected:

	CNxIconButton m_btnClose;
	CNxStatic m_nxsMessageInformation;
	CNxEdit m_nxeActionRequired; //TES 11/18/2009 - PLID 36353 - Replaced static control with edit control
	CNxEdit m_nxeMessageText;
	CNxIconButton m_btnAction;
	CNxIconButton m_btnViewCompleteMessage;
	CNxStatic m_nxsActionTaken;
	NxButton m_btnShowAll;
	NxButton m_btnShowNeedingAttention;
	NxButton m_btnFilterAllProviders;
	NxButton m_btnFilterSelectedProviders;
	CNxColor m_nxcolor;

	CNxStatic m_nxsMessageInfoLabel;
	CNxStatic m_nxsActionTakenLabel;
	CNxStatic m_nxsActionRequiredLabel;

	CNxLabel m_nxlMultiProviderFilter;

	NXDATALIST2Lib::_DNxDataListPtr m_pMessageList;
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderFilterList;

	//TES 4/16/2009 - PLID 33888 - Variables keeping track of which providers to filter on; an empty list means
	// All Providers.
	CArray<long,long> m_arPreFilterProviderIDs;
	bool m_bUsePreFilter;
	CArray<long,long> m_arCurrentFilterProviderIDs;
	//TES 4/27/2009 - PLID 33888 - Track whether we're currently filtering on one or more providers.
	bool m_bUseProviderFilter;

	//TES 5/1/2009 - PLID 34141 - Remember the filter we're using on our provider list.
	CString m_strProvWhere;

	// (e.lally 2009-07-10) PLID 34039 - For simple enabling/disabling of all the modify controls, use a read only mode
	BOOL m_bReadOnlyMode;

	//TES 4/7/2009 - PLID 33882 - Refreshes the list of messages based on the current filters.
	//TES 4/13/2009 - PLID 33882 - This will attempt to keep the same message selected as is selected before this function
	// is called.
	void RefreshList();

	// (a.walling 2009-04-21 15:31) - PLID 34032 - Edit the prescription
	void EditPrescription();

	//TES 4/14/2009 - PLID 33890 - Should only be called when the currently selected row is a SureScripts Error message.
	// Will flag that the user has acknowledged the error, and therefore it does not need attention.
	void AcknowledgeError();

	//TES 4/17/2009 - PLID 33890 - Essentially the same as AcknowledgeError(), except its for invalid messages
	// (Error messages are valid, they just represent errors).
	void AcknowledgeInvalidMessage();

	// (a.walling 2009-04-16 17:04) - PLID 33951
	void RespondToRefillRequest();

	// (a.walling 2009-04-16 16:43) - PLID 33951
	//TES 4/17/2009 - PLID 33890 - Added a strPatientName parameter, for auditing.
	// (a.walling 2009-04-21 16:32) - PLID 34032 - Moved to SureScripts namespace in SureScriptsPractice
	//void CreateAction(long nID, SureScripts::ActionType actionType, const CString& strDescription, const CString &strPatientName);

	//TES 4/13/2009 - PLID 33882 - Used to keep a message selected after a requery.
	long m_nSelectedID;

	//TES 11/18/2009 - PLID 36353 - Set the text in the "Action Required" field.
	void SetActionRequired(const CString &strRequiredAction);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedSsList(short nFlags);
	afx_msg void OnRButtonDownSsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangedSsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnViewCompleteMessage();
	afx_msg void OnBtnAction();
	afx_msg void OnShowAllMessages();
	afx_msg void OnShowNeedingAttention();
	afx_msg void OnFilterAllProviders();
	afx_msg void OnFilterSelectedProviders();
	afx_msg void OnSelChangingProviderFilterList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenProviderFilterList(LPDISPATCH lpRow);
	afx_msg void OnMultiProviderFilter();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnUpdateView(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
