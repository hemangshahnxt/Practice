#pragma once

// (j.armen 2012-03-06 16:45) - PLID 48304 - Created
// (j.armen 2012-03-23 12:22) - PLID 49057 - Renamed all references of 'Recall Path' to 'Recall Template'

// CRecallSetupDlg

class CRecallSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRecallSetupDlg)

public:
	CRecallSetupDlg(CWnd* pParent);
	virtual ~CRecallSetupDlg();

	enum { IDD = IDD_RECALL_SETUP_DLG };

protected:
	NXDATALIST2Lib::IRowSettingsPtr GetActiveRecallTemplateRow(LPDISPATCH lpRow = NULL);
	NXDATALIST2Lib::IRowSettingsPtr GetActiveRecallStepRow(LPDISPATCH lpRow = NULL);
	void SetActiveRecallTemplateID(const long& nRecallTemplateID);
	void SetActiveRecallStepID(const long& nRecallStepID);
	long m_nRecallTemplateID;
	long m_nRecallStepID;
	long GetMaxStepOrder();
	void SwapSteps(bool bMoveDown);

	bool UpdateRecallStepName(const long& nRecallStepID, CString& strNewValue, CString& strOldValue);
	bool UpdateRecallStepTime(const long& nRecallStepID, const long& nNewValue, const long& nOldValue, const CString& strField);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedNewRecallTemplate();
	afx_msg void OnBnClickedNewCopyRecallTemplate();
	afx_msg void OnBnClickedDeleteRecallTemplate();
	afx_msg void OnBnClickedRenameRecallTemplate();
	afx_msg void OnBnClickedNewRecallStep();
	afx_msg void OnBnClickedDeleteRecallStep();
	afx_msg void OnBnClickedRecallStepUp();
	afx_msg void OnBnClickedRecallStepDown();
	afx_msg void OnBnClickedRecallRepeatStep();

	DECLARE_EVENTSINK_MAP()
	afx_msg void OnSelChosenRecallTemplate(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedRecallTemplate(short nFlags);
	afx_msg void OnSelChosenRecallStep(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedRecallStep(short nFlags);
	afx_msg void EditingFinishingNxdlRecallSteps(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);

protected:
	CNxColor m_nxcolor1;
	CNxColor m_nxcolor2;
	CNxColor m_nxcolor3;
	CNxColor m_nxcolor4;
	CNxIconButton m_btnNewRecallTemplate;
	CNxIconButton m_btnNewCopyRecallTemplate;
	CNxIconButton m_btnDeleteRecallTemplate;
	CNxLabel m_nxlRecallTemplate;
	// (a.wilson 2014-02-17 12:04) - PLID 60775 - new search for diagnosis codes.
	NXDATALIST2Lib::_DNxDataListPtr m_pdlRecallDiagnosisSearch; 
	NXDATALIST2Lib::_DNxDataListPtr m_pdlRecallTemplate;
	enum eRecallTemplate {
		eRecallTemplateID,
		eRecallTemplateName,
		eRecallTemplateRepeatLastStep,
	};
	CNxIconButton m_btnRenameRecallTemplate;
	CNxIconButton m_btnNewRecallStep;
	CNxIconButton m_btnDeleteRecallStep;
	NxButton m_btnRepeatLastStep;
	CNxIconButton m_btnRecallStepUp;
	CNxIconButton m_btnRecallStepDown;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlRecallSteps;
	enum eRecallStep {
		eRecallStepID,
		eRecallStepOrder,
		eRecallStepName,
		eRecallStepDay,
		eRecallStepWeek,
		eRecallStepMonth,
		eRecallStepYear,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDiagCode;
	enum eDiagCode {
		eDiagCodeID,
		eDiagCode,
		eDiagCodeDescription,
	};
	CNxIconButton m_btnClose;
	virtual BOOL OnInitDialog();
	void SelChosenRecallSetupDiagnosisSearch(LPDISPATCH lpRow);
	void RButtonDownNxdlRecallDiagCodeSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};


