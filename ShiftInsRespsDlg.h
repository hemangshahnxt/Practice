#if !defined(AFX_SHIFTINSRESPSDLG_H__D19207A0_A493_4C6A_B404_1348786B65B6__INCLUDED_)
#define AFX_SHIFTINSRESPSDLG_H__D19207A0_A493_4C6A_B404_1348786B65B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShiftInsRespsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShiftInsRespsDlg dialog

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;

class CShiftInsRespsDlg : public CNxDialog
{
// Construction
public:
	CShiftInsRespsDlg(CWnd* pParent);   // standard constructor

	long m_ID,
		m_PatientID,
		m_nSrcInsPartyID,
		m_nDstInsPartyID;

	COleCurrency m_cyDestResp,
				m_cySrcAmount;

	long m_nRevenueCode;

	// (j.jones 2013-08-21 08:44) - PLID 58194 - added a required audit string so auditing the shift more clearly states what process shifted and why
	CString m_strAuditFromProcess;

	NXTIMELib::_DNxTimePtr m_nxtTime;
	
	CString m_strLineType;
	CFont *m_pFont;

	// (j.jones 2015-10-29 16:59) - PLID 67431 - added ability to warn when the user is shifting the entire balance
	bool m_bWarnWhenShiftingEntireBalance;
	// (j.jones 2015-10-29 16:59) - PLID 67431 - added ability to skip batching the claim
	bool m_bDoNotSwitchClaimBatches;

// Dialog Data
	//{{AFX_DATA(CShiftInsRespsDlg)
	enum { IDD = IDD_SHIFT_INS_RESPS_DLG };
	NxButton	m_checkBatchClaim;
	NxButton	m_checkSwapInsCos;
	CNxEdit	m_nxeditEditShiftAmount;
	CNxStatic	m_nxstaticLabelDestResp;
	CNxStatic	m_nxstaticLabelDestRespAmount;
	CNxStatic	m_nxstaticLabelSourceResp;
	CNxStatic	m_nxstaticLabelSourceRespAmount;
	CNxStatic	m_nxstaticLabelAmountToShift;
	CNxStatic	m_nxstaticCurrencySymbolShift;
	CNxStatic	m_nxstaticLabelDateOfShift;
	CNxIconButton	m_btnShift;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnCalcPercent;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_BatchList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShiftInsRespsDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShiftInsRespsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnShift();
	afx_msg void OnCalcPercent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHIFTINSRESPSDLG_H__D19207A0_A493_4C6A_B404_1348786B65B6__INCLUDED_)
