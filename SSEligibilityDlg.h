#pragma once

#include "PatientsRc.h"

// (j.fouts 2013-09-19 11:55) - PLID 58701 - Created

// CSSEligibilityDlg dialog

class CSSEligibilityDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSSEligibilityDlg)

public:
	CSSEligibilityDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSSEligibilityDlg();

// Dialog Data
	enum { IDD = IDD_SSELIGIBILITY_DLG };

public:
	//Overload with our cool new version of DoModal that takes in a patient ID
	int DoModal(long nPatientID);

private:
	//Controls
	CNxColor m_nxcBackground;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlResponseList;
	CNxIconButton m_btnClose;

	long m_nPatientID;

private:
	//Hide the standard DoModal in favor of our overloaded version
	virtual int DoModal();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	void RequeryFinishedNxdlResponseList(short nFlags);
};
