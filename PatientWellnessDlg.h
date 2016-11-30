#pragma once
#include "PatientsRc.h"

// (j.gruber 2009-05-26 15:27) - PLID 34348 - created for
// CPatientWellnessDlg dialog

class CPatientWellnessDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientWellnessDlg)

public:
	CPatientWellnessDlg(long nPatientID, CWnd* pParent);   // standard constructor
	virtual ~CPatientWellnessDlg();

// Dialog Data
	enum { IDD = IDD_PATIENT_WELLNESS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CNxIconButton m_btnClose;

	void LoadAlertList();
	// (j.gruber 2009-06-03 15:59) - PLID 34456 - moved createalert to new function
	void CreateAlert();

	long m_nPatientID;

	CNxColor	m_bkg;
	

	NXDATALIST2Lib::_DNxDataListPtr m_pAlertList;

	DECLARE_MESSAGE_MAP()
	// (j.gruber 2009-06-03 11:35) - PLID 34456 - added right click
public:
	DECLARE_EVENTSINK_MAP()
	void LeftClickPatWellnessList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedPatWellnessClose();
	void RButtonDownPatWellnessList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
