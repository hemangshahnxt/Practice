#pragma once

#include "SchedulerRc.h"
#include "PhaseTracking.h"

// CResolveConsSurgBookingDlg dialog
// (c.haag 2009-01-19 16:08) - PLID 32712 - Initial implementation

class CResolveConsSurgBookingDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CResolveConsSurgBookingDlg)

public:
	// (j.gruber 2009-07-09 10:22) - PLID 34303 - added aptcategory
	CResolveConsSurgBookingDlg(PhaseTracking::AptCategory aptCat, CWnd* pParent);   // standard constructor
	virtual ~CResolveConsSurgBookingDlg();

// Dialog Data
	enum { IDD = IDD_RESOLVE_CONS_SURG_BOOKING };

public:
	enum { eDoNothing, eAddProcedures, eReplaceProcedures } m_Action;

public:
	CString m_strApptText;

protected:
	CNxStatic m_nxstaticApptText;
	NxButton m_radAdd;
	NxButton m_radReplace;
	NxButton m_radNoChange;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	PhaseTracking::AptCategory m_aptCat;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadioAdd();
	afx_msg void OnBnClickedRadioReplace();
	afx_msg void OnBnClickedRadioNoChange();
	virtual BOOL OnInitDialog();
};
