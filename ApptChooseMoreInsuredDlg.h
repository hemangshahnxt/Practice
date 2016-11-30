#pragma once
#include "GlobalSchedUtils.h"
#include "NewPatientAddInsuredDlg.h"

// CApptChooseMoreInsuredDlg dialog
// (j.gruber 2012-08-03 14:23) - PLID 51896 - created for

class CApptChooseMoreInsuredDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptChooseMoreInsuredDlg)

public:
	CApptChooseMoreInsuredDlg(long nCurPatientID, CNewPatientInsuredParty patientInsInfo,
		AppointmentInsuranceMap *mapInsuranceInfo, CWnd* pParent = NULL);   // standard constructor
	virtual ~CApptChooseMoreInsuredDlg();

// Dialog Data
	enum { IDD = IDD_APPT_CHOOSE_MORE_INSURED_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void ClearInsuranceMap();
	void ColorInsList();

	AppointmentInsuranceMap *m_pmapInsuranceInfo;
	long m_nCurPatientID;
	CNewPatientInsuredParty m_patientInsInfo;
	

	NXDATALIST2Lib::_DNxDataListPtr m_pInsList;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsOrderList;

	CNxIconButton m_btnUp;
	CNxIconButton m_btnDown;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedResInsMoveUp();
	afx_msg void OnBnClickedResInsMoveDown();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void SelChangingResInsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenResInsList(LPDISPATCH lpRow);
	void RequeryFinishedResInsList(short nFlags);
	void RButtonUpResInsOrderList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
