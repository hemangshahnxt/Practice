#pragma once
#include "PatientsRc.h"

// CICD10GoLiveDateDlg dialog

// (b.spivey - March 6th, 2014) - PLID 61196 - Created. 

class CICD10GoLiveDateDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CICD10GoLiveDateDlg)

public:
	CICD10GoLiveDateDlg(long nInsCoID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CICD10GoLiveDateDlg();

// Dialog Data
	enum { IDD = IDD_ICD10_GO_LIVE_UPDATE_DLG };

protected:

	/*Control variables*/
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;

	NxButton m_checkGoLiveDate; 

	CDateTimePicker m_dtICD10GoLiveDate;


	NXDATALIST2Lib::_DNxDataListPtr m_dlInsList; 

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	long m_nLoadingInsCoID; 

	/* Control Functions */ 
	void OnCancel();
	void OnOK();

	afx_msg void OnGoLiveDateChecked();

	afx_msg void OnRequeryFinishedInsList(short nFlags); // (b.spivey, March 12th, 2014) - PLID 61196 - 

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
