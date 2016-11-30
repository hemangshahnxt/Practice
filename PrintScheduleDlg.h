//{{AFX_INCLUDES()
#include "commondialog.h"
//}}AFX_INCLUDES
#if !defined(AFX_PRINTSCHEDULEDLG_H__015BDC93_3D36_11D2_80C6_00104B2FE914__INCLUDED_)
#define AFX_PRINTSCHEDULEDLG_H__015BDC93_3D36_11D2_80C6_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PrintScheduleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrintScheduleDlg dialog

class CPrintScheduleDlg : public CNxDialog
{
public:
	enum pmPrintMode {
		pmGraphical,
		pmReport
	};
// Construction
public:
	long m_nPurposeSetId;
	CString m_strMultiTypeIds;
	long m_nPurposeID;
	long m_nPrintExtent;
	long ZoomPrint(CPrintInfo * pInfo);
	pmPrintMode GetPrintMode();
	bool Init();
	COleDateTime m_dtReportCenterDate;
	COleDateTime m_dtReportEndDate;//Ignored for all except Week view.

	//(a.wilson 2012-2-16) PLID 39893
	// (r.gonet 06/10/2013) - PLID 56503 - DWORD Array that holds multiple location IDs that we'll use when filtering the scheduler report.
	CDWordArray m_dwLocations;
	
	BOOL m_bShowApptShowState;
	BOOL m_bShowMonthProcedures;
	BOOL m_bShowTemplateColors;
	BOOL m_bClassicShowCancelled;
	BOOL m_bUseLandscape;
	BOOL m_bPrintGrid;
	BOOL m_bViewedResources;
	BOOL m_bPrintTemplates; // (c.haag 2008-06-18 16:13) - PLID 26135

	CString m_strActiveItemID;
	CPrintScheduleDlg(CWnd* pParent);   // standard constructor
	void OpenReport();

	bool m_bGotPrac97;

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPrintScheduleDlg)
	enum { IDD = IDD_PRINT_SCHEDULE_DLG };
	CNxIconButton	m_btnPreview;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnSchedPrintStyleGroupbox;
	NxButton	m_btnOrientationStatic;
	NxButton	m_btnPropertiesGroupbox;
	NxButton	m_radioGraphical;
	NxButton	m_radioTextual;
	NxButton	m_radioPortrait;
	NxButton	m_radioLandscape;
	NxButton	m_checkShowCancelledAppts;
	NxButton	m_checkPrintPending;
	NxButton	m_checkPrintTemplateColor;
	NxButton	m_checkPrintGrid;
	NxButton	m_checkViewedResources;
	NxButton	m_checkPrintProcedures;
	NxButton	m_checkPrintTemplates;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrintScheduleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPrintInfo * m_pInfo;
	pmPrintMode m_pmPrintMode;
	void SetCenterDate(COleDateTime dateBase, COleDateTime dateEnd, COleDateTime &dateFrom, COleDateTime &dateTo);

	// Generated message map functions
	//{{AFX_MSG(CPrintScheduleDlg)
	afx_msg void OnPreviewBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnGraphicalRadio();
	afx_msg void OnTextualRadio();
	afx_msg void OnPrintTemplateClr();
	afx_msg void OnPrintPending();
	afx_msg void OnPrintProcedures();
	afx_msg void OnShowCancelledAppts();
	afx_msg void OnLandscapeRadio();
	afx_msg void OnPortraitRadio();
	afx_msg void OnPrintGrid();
	afx_msg void OnCheckViewedResources();
	afx_msg void OnCheckPrintTemplates();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRINTSCHEDULEDLG_H__015BDC93_3D36_11D2_80C6_00104B2FE914__INCLUDED_)
