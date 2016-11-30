#if !defined(AFX_LABSSETUPDLG_H__500ED870_5B0A_4846_B6A8_170D04371668__INCLUDED_)
#define AFX_LABSSETUPDLG_H__500ED870_5B0A_4846_B6A8_170D04371668__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LabsSetupDlg.h : header file
//

//TES 4/7/2010 - PLID 38040 - Moved the LabType enum to GlobalLabUtils.h

/////////////////////////////////////////////////////////////////////////////
// CLabsSetupDlg dialog

class CLabsSetupDlg : public CNxDialog
{
// Construction
public:
	CLabsSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_LabProcedureList;
	NXDATALIST2Lib::_DNxDataListPtr m_StepsList;
	// (r.galicki 2008-10-16 11:06) - PLID 31552 - add lab type
	NXDATALIST2Lib::_DNxDataListPtr m_LabTypeList;

	BOOL m_bUserNotifiedOfChange;

// Dialog Data
	//{{AFX_DATA(CLabsSetupDlg)
	enum { IDD = IDD_LABS_SETUP };
	CNxIconButton	m_btnRenameLabProcedure;
	CNxIconButton	m_btnDeleteLabProcedure;
	CNxIconButton	m_btnNewLabProcedure;
	CNxIconButton	m_btnModifyStep;
	CNxIconButton	m_btnDeleteStep;
	CNxIconButton	m_btnNewStep;
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnEditClinDiag;
	CNxIconButton	m_btnEditLabDesc;
	CNxStatic	m_nxstaticLabProceduresStatic;
	CNxIconButton	m_btnEditSignature;
	CNxIconButton	m_btnCreateCopy; // (z.manning 2010-05-07 10:22) - PLID 35537
	CNxIconButton	m_btnLabelPrintSetup; // (z.manning 2010-05-13 16:49) - PLID 37876
	NxButton		m_btnSendSpecimenLabel;	// (d.thompson 2010-06-14) - PLID 36791
	NxButton		m_btnSendRequestForm;	// (d.thompson 2010-06-14) - PLID 36791
	// (s.tullis 2015-11-16 10:50) - PLID 54285 - Create a way to import the labs' compendiums of orderable test codes, which are usually in CSV or Excel files.
	CNxIconButton		m_btnImportLOINC;
	// (j.jones 2010-06-25 15:52) - PLID 39185 - added ability to link locations and 'to be ordered' entries
	CNxIconButton	m_btnConfigLabToBeOrderedLoc;
	// (r.gonet 09/21/2011) - PLID 45584 - Opens up the lab custom fields template selector/editor.
	CNxIconButton	m_btnEditCustomFields;
	// (r.gonet 10/11/2011) - PLID 45924 - Opens the lab barcode setup dialog.
	CNxIconButton	m_btnBarcodeSetup;
	// (r.gonet 03/29/2012) - PLID 49299
	CNxIconButton m_btnLabProcedureGroupsSetup;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabsSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void ResetCurrentStepsListOrdering();
	void FixDuplicateStepOrdering(long nProcedureID, long nDuplicateStepOrder, long nStepIDToIncrement);

	// Generated message map functions
	//{{AFX_MSG(CLabsSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenLabProceduresList(LPDISPATCH lpRow);
	afx_msg void OnNewLabProcedure();
	afx_msg void OnDeleteLabProcedure();
	afx_msg void OnRenameLabProcedure();
	afx_msg void OnEditingFinishedStepsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnNewStep();
	afx_msg void OnDeleteStep();
	afx_msg void OnModifyStep();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnEditLabDesc();
	afx_msg void OnEditClinicalDiags();
	afx_msg void OnRequeryFinishedStepsList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// (z.manning 2008-10-10 12:10) - PLID 21108
	void OnDblClickCellStepsList(LPDISPATCH lpRow, short nColIndex);
	void SelChosenNxdlLabtype(LPDISPATCH lpRow);
	void RequeryLabSteps();
	afx_msg void OnBnClickedLabEditSignature(); // (z.manning 2008-10-22 16:07) - PLID 21082
	afx_msg void OnBnClickedCopyLabProcedure();
	afx_msg void OnBnClickedLabEditLabelPrintSettings(); // (z.manning 2010-05-13 16:46) - PLID 37876
	afx_msg void OnBnClickedSendSpecimenLabel();
	afx_msg void OnBnClickedSendRequestForm();
	// (j.jones 2010-06-25 15:52) - PLID 39185 - added ability to link locations and 'to be ordered' entries
	afx_msg void OnBtnConfigLabToBeOrderedLoc();
public:
	afx_msg void OnBnClickedEditCustomFields();
	afx_msg void OnBnClickedLabBarcodeSetupBtn();
	// (r.gonet 03/29/2012) - PLID 49299
	afx_msg void OnBnClickedLabProcedureGroupSetupBtn();
	// (s.tullis 2015-11-16 10:50) - PLID 54285 - Create a way to import the labs' compendiums of orderable test codes, which are usually in CSV or Excel files.
	afx_msg void OnBnClickedImportLoinc();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABSSETUPDLG_H__500ED870_5B0A_4846_B6A8_170D04371668__INCLUDED_)
