#if !defined(AFX_EDITCHARGERESPDETAILDLG_H__33E1FCDD_CCAD_4DF8_BF0F_35CE5C02D7A2__INCLUDED_)
#define AFX_EDITCHARGERESPDETAILDLG_H__33E1FCDD_CCAD_4DF8_BF0F_35CE5C02D7A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditChargeRespDetailDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditChargeRespDetailDlg dialog

class CEditChargeRespDetailDlg : public CNxDialog
{
// Construction
public:
	CEditChargeRespDetailDlg(CWnd* pParent);   // standard constructor
	CEditChargeRespDetailDlg(long nBillID, long nChargeID, CWnd* pParent);

	NXDATALISTLib::_DNxDataListPtr  m_pChargeList;
	NXDATALISTLib::_DNxDataListPtr  m_pDetailList;

	long m_nBillID;
	long m_nChargeID;
	
// Dialog Data
	//{{AFX_DATA(CEditChargeRespDetailDlg)
	enum { IDD = IDD_EDIT_CHARGERESP_DLG };
	CNxIconButton	m_btnCloseChResp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditChargeRespDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL m_bWarnedDateChange;

	// Generated message map functions
	//{{AFX_MSG(CEditChargeRespDetailDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenDetailList(long nRow);
	afx_msg void OnSelChosenChargeList(long nRow);
	afx_msg void OnCloseChresp();
	afx_msg void OnEditingFinishingCrdetailList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedCrdetailList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCHARGERESPDETAILDLG_H__33E1FCDD_CCAD_4DF8_BF0F_35CE5C02D7A2__INCLUDED_)
