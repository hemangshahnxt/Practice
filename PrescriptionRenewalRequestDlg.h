#if !defined(AFX_PRESCRIPTIONRENEWALREQUEST_H__111285E5_2F6C_43CB_86FA_864C4BF297D5__INCLUDED_)
#define AFX_PRESCRIPTIONRENEWALREQUEST_H__111285E5_2F6C_43CB_86FA_864C4BF297D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// (j.gruber 2009-06-16 12:42) - PLID 28541 - added functionality for renewal requests

// CPrescriptionRenewalRequestDlg dialog

class CPrescriptionRenewalRequestDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPrescriptionRenewalRequestDlg)

public:
	CPrescriptionRenewalRequestDlg(CWnd* pParent);   // standard constructor	

	CNxIconButton m_btnClose;
	CNxIconButton m_btnAssignToPatient;

// Dialog Data
	enum { IDD = IDD_PRESCRIPTION_RENEWAL_REQUEST_DLG };

protected:
	void LoadPatientRenewals();
	void LoadAccountStatus();

	NXDATALIST2Lib::_DNxDataListPtr m_pRenewalList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	// (j.gruber 2009-05-15 11:30) - PLID 28541 - renewal requests
	long CPrescriptionRenewalRequestDlg::FindPatientByRenewalInfo(long &nUserDefinedID, CString strPatientFirstName, CString strPatientMiddleName, CString strPatientLastName, CString strPatientGender, CString strPatientDOB);

	afx_msg LRESULT OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	DECLARE_EVENTSINK_MAP()
	void LeftClickPrescriptionRenewalRequestList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RequeryFinishedPrescriptionRenewalRequestList(short nFlags);
	afx_msg void OnBnClickedPrrAssignToPatient();
	void SelChangedPrescriptionRenewalRequestList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedRenewalRequestClose();
};
#endif