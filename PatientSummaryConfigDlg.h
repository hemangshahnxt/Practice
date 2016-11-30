#pragma once
#include "PatientsRc.h"
#include "PatientSummaryDlg.h"

// CPatientSummaryConfigDlg dialog
// (j.gruber 2010-06-15 14:28) - PLID  39174  - Created

class CPatientSummaryConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientSummaryConfigDlg)

private:
	CNxIconButton m_btnLeftMoveUp;
	CNxIconButton m_btnLeftMoveDown;
	CNxIconButton m_btnRightMoveUp;
	CNxIconButton m_btnRightMoveDown;
	CNxIconButton m_btnMoveOneLeft;
	CNxIconButton m_btnMoveOneRight;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_stDesc;

	void LoadConfigValues();
	void SetConfigInfo(CString strBase, CString strName);
public:
	CPatientSummaryConfigDlg(CMap<long, long, CString, LPCTSTR> *pmapCustomFields, CWnd* pParent);   // standard constructor
	virtual ~CPatientSummaryConfigDlg();

// Dialog Data
	enum { IDD = IDD_PAT_SUMMARY_CONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedPscLeftMoveUp();
	afx_msg void OnBnClickedPscLeftMoveDown();
	afx_msg void OnBnClickedPscMoveOneRight();
	afx_msg void OnBnClickedPscMoveOneLeft();
	afx_msg void OnBnClickedPscRightMoveUp();
	afx_msg void OnBnClickedPscRightMoveDown();
	CMap<long, long, CString, LPCTSTR> *m_pmapCustomFields;

	void MoveRowUp(NXDATALIST2Lib::_DNxDataListPtr pList);
	void MoveRowDown(NXDATALIST2Lib::_DNxDataListPtr pList);
	void FixSorts();
	CString GetCustomTitle(long nField);
	void UpdateButtons();

	NXDATALIST2Lib::_DNxDataListPtr m_pLeftList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRightList;
public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChangedPatSumConfigLeft(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangedPatSumConfigRight(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
