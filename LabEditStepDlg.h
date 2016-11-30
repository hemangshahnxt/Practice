#pragma once


// CLabEditStepDlg dialog
// (z.manning 2008-10-10 10:32) - PLID 21108 - Created

class CLabEditStepDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabEditStepDlg)

public:
	CLabEditStepDlg(long nLabProcedureID, CString strLabProcedureName, CWnd* pParent);   // standard constructor
	virtual ~CLabEditStepDlg();

	void SetStepID(const long nStepID);
	long GetStepID();
	CString GetStepName();

	//TES 1/5/2011 - PLID 37877 - Accessors for some flags.
	BOOL GetCreateLadder();
	BOOL GetCompletedByHL7();

	//TES 1/5/2011 - PLID 37877 - Need to call these functions to tell the dialog whether either of these things are true, so that
	// it can warn the user appropriately of invalid setups.
	void SetPrevStepIsCompletedByHL7();
	void SetNextStepIsCreateLadder();

// Dialog Data
	enum { IDD = IDD_LAB_EDIT_STEP };

protected:

	long m_nLabProcedureID;
	long m_nStepID;
	CString m_strLabProcedureName; // (r.goldschmidt 2014-08-20 10:52) - PLID 53701 - Auditing for Adminstrator>Labs tab
	CString m_strStepName;
	_variant_t m_varTodoCategoryID;

	//TES 1/5/2011 - PLID 37877 - Added
	BOOL m_bCreateLadder;
	BOOL m_bCompletedByHL7;

	//TES 1/5/2011 - PLID 37877 - Added, used to warn the user of invalid setups.
	BOOL m_bPrevStepIsCompletedByHL7;
	BOOL m_bNextStepIsCreateLadder;

	CArray<long,long> m_arynTodoUserIDs;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlUsers;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlTodoCategory;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlTodoPriority;

	enum EUserListColumns
	{
		ulcID = 0,
		ulcUsername,
	};
	enum ETodoCategoryListColumns
	{
		tclcID = 0,
		tclcCategory,
	};
	enum ETodoPriorityListColumns
	{
		tplcPriority = 0,
		tplcPriorityName,
	};

	void LoadExisting();
	BOOL Save();

	void UpdateTodoControls();
	void HandleTodoUserMutiSelection();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	//TES 1/4/2011 - PLID 37877 - Track whether the ladder being edited has another step that is flagged as CompletedByHL7, so that we can
	// warn them if they try to flag two steps that way.
	BOOL m_bOtherStepsCompletedByHL7;

	// (b.spivey, March 18, 2013) - PLID 44188 - Track if other steps already have this checked off. 
	bool m_bOtherStepsMarkedCompletedBySigning;


	DECLARE_MESSAGE_MAP()
	CNxEdit m_nxeditStepName;
	NxButton m_nxbtnCreateTodo;
	NxButton m_nxbtnCompletedByHL7;
	NxButton m_nxbtnCompletedBySigning; 
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedLabStepUser(short nFlags);
	void OnRequeryFinishedLabTodoCategory(short nFlags);
	afx_msg void OnBnClickedLabCreateTodo();
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	NxButton m_nxbtnCreateLadder; // (z.manning 2008-10-20 17:39) - PLID 31371
	CNxLabel m_nxlblTodoAssignees;
	void SelChosenLabStepUser(LPDISPATCH lpRow);
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	// (b.spivey, March 18, 2013) - PLID 44188 - Event handler.
	afx_msg void OnCompletedBySigning(); 
	afx_msg void OnCompletedByHl7();
	afx_msg void OnLabCreateLadder();
};
