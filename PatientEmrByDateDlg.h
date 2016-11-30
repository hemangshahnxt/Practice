#pragma once
#include "EmrRc.h"
// (s.dhole 2010-02-08 13:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
// CPatientEmrByDateDlg dialog

class CPatientEmrByDateDlg : public CDialog
{
	DECLARE_DYNAMIC(CPatientEmrByDateDlg)

public:
	CPatientEmrByDateDlg(CWnd* pParent);   // standard constructor
	virtual ~CPatientEmrByDateDlg();
	long m_nPatientID;
	CString m_strTitle;	
// Dialog Data
	enum { IDD = IDD_PATIENT_EMR_BY_DATE };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	void	OpenEMN();
	void	ReloadEMRList();
	
	NXDATALIST2Lib::_DNxDataListPtr m_pExistingList;
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void LeftClickExistingListByDate(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedOpen();
	void SelSetExistingListByDate(LPDISPATCH lpSel);
	afx_msg void OnBnClickedCancel();
};
