#pragma once


// CPatientGraphConfigDlg dialog
// (d.thompson 2009-05-20) - PLID 28486
#include "PatientsRc.h"

class CPatientGraphConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientGraphConfigDlg)

public:
	CPatientGraphConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CPatientGraphConfigDlg();

// Dialog Data
	enum { IDD = IDD_PATIENT_GRAPH_CONFIG_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pGraphList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAddDetailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pChosenDetailList;
	// (j.gruber 2010-02-17 10:27) - PLID 37396
	NXDATALIST2Lib::_DNxDataListPtr m_pStatNormList;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnRemoveSelected;
	CNxIconButton m_btnPreview;
	CNxIconButton m_btnCancel;
	NxButton m_btnDoNotFillEmpty;
	// (j.gruber 2010-02-17 10:03) - PLID 37396 - added radio buttons
	NxButton m_btnGraphDate;
	NxButton m_btnGraphAge;

	void LoadInterfaceForCurrentSelection();
	void EnableInterface(BOOL bEnable);

	// (j.gruber 2010-02-09 17:14) - PLID 37291
	void LoadColumnValuesForAllRows();
	void LoadDateColumnValuesForOneRow(LPDISPATCH lpRow);
	void LoadColumnValuesForOneRow(LPDISPATCH lpRow, long nListType, int nColID, BOOL bAddAll);
	BOOL VerifyList();
	void SaveChangesToData(NXDATALIST2Lib::IRowSettingsPtr pRow, long nNewValue, long nOldValue, short nColID);
	BOOL CanEdit(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol, long nNewValue);
	// (j.gruber 2010-02-16 15:25) - PLID 37148
	void SetFormulaGraphSql(CString strValuesTableName, CString strInfoTableName);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void SelChosenGraphOptionList(LPDISPATCH lpRow);
	void SelChosenAddGraphDetailList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedAddGraphOption();
	afx_msg void OnBnClickedDeleteGraphOption();
	afx_msg void OnBnClickedRemoveDetail();
	afx_msg void OnBnClickedGraphDoNotFillEmpty();
public:
	void EditingFinishedChosenGraphDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void SelChangingGraphOptionList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void EditingStartingChosenGraphDetailList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishingChosenGraphDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void RequeryFinishedStatisticalNormList(short nFlags);
	afx_msg void OnBnClickedGraphByDate();
	afx_msg void OnBnClickedGraphByAge();
};
