#if !defined(AFX_CONFIGUREAAFPRSSURVEYDLG_H__FA75CEC6_5BA4_4B28_AAB8_72C0B756FF16__INCLUDED_)
#define AFX_CONFIGUREAAFPRSSURVEYDLG_H__FA75CEC6_5BA4_4B28_AAB8_72C0B756FF16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureAAFPRSSurveyDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CConfigureAAFPRSSurveyDlg dialog

class CConfigureAAFPRSSurveyDlg : public CNxDialog
{

	enum {
		afsCosmeticSurgical = 1,
		afsCosmeticNonSurgical = 2,
		afsReconstructiveSurgical = 3,
		afsFacial = 4,
		afsFacialCosmetic = 5,
		afsSurgicalProcedures = 6,
		afsMinimallyInvasiveProcedures = 7,
	};

// Construction
public:
	CConfigureAAFPRSSurveyDlg(CWnd* pParent);   // standard constructor
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;

// Dialog Data
	//{{AFX_DATA(CConfigureAAFPRSSurveyDlg)
	enum { IDD = IDD_CONFIGURE_AAFPRS_SURVEY_DLG };
	CNxIconButton	m_Close;
	CNxIconButton	m_Move_One_Right;
	CNxIconButton	m_Move_One_Left;
	CNxIconButton	m_Move_All_Right;
	CNxIconButton	m_Move_All_Left;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureAAFPRSSurveyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void MoveOneLeft(LPDISPATCH lpRow);
	void MoveOneRight(LPDISPATCH lpRow);

	// Generated message map functions
	//{{AFX_MSG(CConfigureAAFPRSSurveyDlg)
	afx_msg void OnAafprsClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenAvailProcList(LPDISPATCH lpRow);
	afx_msg void OnDblClickCellAvailProcList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChosenSelectedProcList(LPDISPATCH lpRow);
	afx_msg void OnDblClickCellSelectedProcList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnAafprsMoveAllRight();
	afx_msg void OnAafprsMoveOneRight();
	afx_msg void OnAafprsMoveOneLeft();
	afx_msg void OnAafprsMoveAllLeft();
	afx_msg void OnSelChosenAafprsTypeList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingAafprsTypeList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingSelectedProcList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingAvailProcList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGUREAAFPRSSURVEYDLG_H__FA75CEC6_5BA4_4B28_AAB8_72C0B756FF16__INCLUDED_)
