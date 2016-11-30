//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_MULTIFEEADJUSTMENTDLG_H__EBD07361_A96A_11D2_AB77_00A0246CDDA1__INCLUDED_)
#define AFX_MULTIFEEADJUSTMENTDLG_H__EBD07361_A96A_11D2_AB77_00A0246CDDA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiFeeAdjustmentDlg.h : header file
//

// (a.walling 2008-07-07 17:43) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
#ifdef INCLUDEDEADCODE

struct MultiFeeAdj {

	_variant_t LineID;
	_variant_t ChargeID;
	_variant_t CPTCode;
	_variant_t CPTSubCode;
	_variant_t Description;
	_variant_t ChargedFee;
	_variant_t InsuranceFee;
	_variant_t Adjustment;
	_variant_t WriteOffAdjustment;

};

static CPtrArray g_aryMultiFeeAdjustmentT;

/////////////////////////////////////////////////////////////////////////////
// CMultiFeeAdjustmentDlg dialog

class CMultiFeeAdjustmentDlg : public CDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_List;
	void EmptyList();
	int GetChargeCount();
	void AddCharge(int iChargeID, CString strCPTCode, CString strCPTSubCode, CString strDescription, COleCurrency cyChargedFee, COleCurrency cyInsuranceFee);
	CMultiFeeAdjustmentDlg(CWnd* pParent);   // standard constructor
	~CMultiFeeAdjustmentDlg();

// Dialog Data
	//{{AFX_DATA(CMultiFeeAdjustmentDlg)
	enum { IDD = IDD_MULTI_FEE_ADJUSTMENT };
	CNxStatic	m_nxstaticLabel1;
	CNxStatic	m_nxstaticLabel2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiFeeAdjustmentDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultiFeeAdjustmentDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int CreateAdjustment(COleCurrency cyAdjustment, CString strChargeDesc);
	int m_ItemCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif

#endif // !defined(AFX_MULTIFEEADJUSTMENTDLG_H__EBD07361_A96A_11D2_AB77_00A0246CDDA1__INCLUDED_)
