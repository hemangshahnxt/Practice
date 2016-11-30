#pragma once

//(e.lally 2010-10-25) PLID 40994 - Created
// CLinksSetupDlg dialog

class CLinksSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLinksSetupDlg)

//(e.lally 2011-04-27) PLID 41445 - Moved to the header
protected:
//These NexWebToDoEvents values are stored in data so do not change them!
//Update NexWeb.Methods.cs in the NexTech.Utilities when adding new ones.
// (j.armen 2011-11-22 10:37) - PLID 40420 - Renamed to NexWebEvents
enum NexWebEvent {
	nteInvalid = -1, //Not to be stored in data!
	nteNewProspectEntered = 1,
	ntePatientMessageReceived = 2,
	nteAppointmentRequestReceived = 3, //(e.lally 2011-02-28) PLID 42603
	nteAppointmentChangeRequestReceived = 4, // (j.armen 2011-07-20 11:58) - PLID 44208
	nteAppointmentCancelationRequestReceived = 5, // (j.armen 2011-07-20 13:59) - PLID 44208
};

//(e.lally 2011-04-29) PLID 43480
enum NexWebDisplaylistType
{
	nwltReferralSources = 0,
	nwltInsuranceCos,
	nwltEMRTemplates,
	nwltProcedures,
	nwltCustomFields,
	nwltCountries,				//(e.lally 2011-05-04) PLID 42209
	nwltLanguages,				//(e.lally 2011-05-04) PLID 42209
	nwltRaces,					//(e.lally 2011-05-04) PLID 42209
	nwltEthnicities,			//(e.lally 2011-05-04) PLID 42209
	nwltPrefixes,				//(e.lally 2011-05-04) PLID 42209
	nwltReferringPhysicians,	//(e.lally 2011-05-04) PLID 42209
	nwltPrimaryCareProviders,	//(e.lally 2011-05-04) PLID 42209
	nwltCustomDropdownList1,	//(e.lally 2011-05-09) PLID 42209
	nwltCustomDropdownList2,	//(e.lally 2011-05-09) PLID 42209
	nwltCustomDropdownList3,	//(e.lally 2011-05-09) PLID 42209
	nwltCustomDropdownList4,	//(e.lally 2011-05-09) PLID 42209
	nwltCustomDropdownList5,	//(e.lally 2011-05-09) PLID 42209
	nwltCustomDropdownList6,	//(e.lally 2011-05-09) PLID 42209

	nwltCount_DoNotUse,

};

//(e.lally 2011-04-29) PLID 43480
struct NexWebDisplaylistDetail
{
	NexWebDisplaylistType enumValue;
	CString strTableName;
	CString strPKIDFieldName;
	CString strFKIDFieldName;

};

public:
	CLinksSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CLinksSetupDlg();

// Dialog Data
	enum { IDD = IDD_LINKS_SETUP };
	NxButton		m_checkToDoEnabled;
	NxButton		m_checkActionEnabled;		// (j.armen 2011-11-22 10:38) - PLID 40420
	CNxEdit			m_nxeditToDoNote;
	CNxEdit			m_nxeditActionNote;			// (j.armen 2011-11-22 10:38) - PLID 40420
	CNxIconButton	m_btnNexWebToDoEventNext;
	CNxIconButton	m_btnNexWebToDoEventPrev;
	CNxIconButton	m_btnNexWebDisplaySetup;
	CNxIconButton	m_btnNexWebSubdomainNext;
	CNxIconButton	m_btnNexWebSubdomainPrev;
	CNxIconButton	m_btnNexWebSubdomainNew;
	CNxIconButton	m_btnNexWebSubdomainRename;
	CNxIconButton	m_btnNexWebSubdomainDelete;
	CNxIconButton	m_btnNexWebSubdomainMakeDefault;
	CNxIconButton	m_btnNexWebContentSettingUp;
	CNxIconButton	m_btnNexWebContentSettingDown;
	CNxIconButton	m_btnNexWebContentSettingAdd;
	CNxIconButton	m_btnNexWebContentSettingRemove;
	// (b.savon 2012-07-25 14:19) - PLID 50585
	CNxIconButton	m_btnNexWebPasswordRequirements;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLinksSetupDlg)
public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	
protected:
	//(e.lally 2011-04-26) PLID 41445
	long m_nSubdomainID;
	//(e.lally 2011-04-27) PLID 41445 - Track the current ToDo Event
	NexWebEvent m_eEvent;
	//(e.lally 2011-04-29) PLID 43480
	long m_nDisplaylistTypeID;
	bool m_bDisplaylistSetupIsValid;
	//(e.lally 2011-05-19) PLID 43333 - Added content setting controls
	CString m_strContentSettingUID, m_strContentSettingDesc, m_strContentTypeUID;
	CArray<NexWebDisplaylistDetail, NexWebDisplaylistDetail> m_displaylistSetup;

	NXDATALIST2Lib::_DNxDataListPtr m_pSubdomainList;
	//(e.lally 2010-11-09) PLID 35819
	NXDATALIST2Lib::_DNxDataListPtr m_pEventList, m_pToDoPriority, m_pToDoMethod, m_pToDoCategory, m_pToDoAssignTo;
	NXDATALIST2Lib::_DNxDataListPtr m_pDisplaylistType, m_pDisplaylistDetails;
	//(e.lally 2011-05-19) PLID 43333 - Added content setting controls
	NXDATALIST2Lib::_DNxDataListPtr m_pContentSettingList, m_pContentSettingDetailTable;
	//(e.lally 2010-11-29) PLID 41577
	BOOL m_bAssignToPatCood;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	//(e.lally 2011-04-27) PLID 41445
	virtual void Refresh();
	afx_msg void OnEnableToDo();
	afx_msg void OnEnableAction();		// (j.armen 2011-11-22 10:38) - PLID 40420
	afx_msg void OnPrevToDoEvent();
	afx_msg void OnNextToDoEvent();
	afx_msg void OnBnNexWebDisplaySetup();
	// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
	afx_msg void OnBnClickedApptprototypeSetupBtn();
	afx_msg void OnPrevSubdomain();
	afx_msg void OnNextSubdomain();
	afx_msg void OnNewSubdomain();
	afx_msg void OnRenameSubdomain();
	afx_msg void OnDeleteSubdomain();
	afx_msg void OnMakeDefaultSubdomain();
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	//(e.lally 2010-11-09) PLID 35819
	void OnSelChosenToDoEvent(LPDISPATCH lpRow);
	void OnSelChosenToDoPriority(LPDISPATCH lpRow);
	void OnSelChosenToDoMethod(LPDISPATCH lpRow);
	void OnSelChosenToDoCategory(LPDISPATCH lpRow);
	afx_msg void OnEditingFinishedToDoAssignToUsers(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	void OnRequeryFinishedToDoCategory(short nFlags);
	//(e.lally 2010-11-29) PLID 41577
	void OnRequeryFinishedToDoAssignToUsers(short nFlags);
	void OnSelChangingSubdomainList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	void OnSelChosenSubdomainList(LPDISPATCH lpRow);
	void OnSelChosenDisplaylistType(LPDISPATCH lpRow);
	afx_msg void OnEditingFinishedNexWebListDisplay(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingNexWebListDisplay(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnSelectallDisplaylistBtn();
	afx_msg void OnEditingFinishedContentSettingDetailTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Handle actual integer type values, including min/max enforcement
	afx_msg void OnEditingFinishingContentSettingDetailTable(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	

	//(e.lally 2010-11-09) PLID 35819
	//(e.lally 2011-04-26) PLID 43445 - Renamed functions to be ToDo specific
	// (j.armen 2011-11-22 10:38) - PLID 40420 - Renamed functions to be Event specific
	void LoadEvent();
	void EnsureEventControls();
	void ResetEventControls();
	void InitializeEventControls();
	void InitializeDisplaylistControls();
	void InitializeContentSettingControls();

	void EnsureAllControls();
	void LoadSubdomain();
	void EnsureSubdomainControls();

	void LoadDisplaylistDetails();
	void EnsureDisplaylistControls();
	void EnsureDisplaylistColumns();
	CString GetDisplaylistDetailsFromClause(NexWebDisplaylistType nwListType);

	//(e.lally 2011-05-19) PLID 43333 - Added content setting controls
	void LoadContentSetting();
	void EnsureContentSettingControls();
	void EnsureContentSettingButtons();

	// (j.armen 2011-11-22 10:39) - PLID 40420 added functions to determine if 
	// events have actions or create todo's (or both) and get the name of an action
	bool HasActionEvent();
	bool HasToDoEvent();
	CString GetActionEventName();

	// (d.singleton 2013-02-01 12:08) - PLID 54979 functions to load text and boolean content type settings
	void LoadContentSettingData();
	void LeftClickNexwebSetupContentSettingDetailTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	NXDATALIST2Lib::IFormatSettingsPtr m_pfsText;
	NXDATALIST2Lib::IFormatSettingsPtr m_pfsBoolean;
	// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Handle actual integer type values, including min/max enforcement
	NXDATALIST2Lib::IFormatSettingsPtr m_pfsInteger;
	void ChangeContentSettingValue_IntParam(LPDISPATCH lpRow, const VARIANT FAR& varNewValue, bool bTreatAsBoolean);
	// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
	void ChangeContentSettingValue_Text(LPDISPATCH lpRow, const VARIANT FAR& varNewValue);

public:
	afx_msg void OnBnClickedBtnNexwebPasswordReq();	
};
