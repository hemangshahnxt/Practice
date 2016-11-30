#if !defined(AFX_IMPLEMENTATIONDLG_H__91EFA5E5_F0DF_4C83_8814_B225F60730AD__INCLUDED_)
#define AFX_IMPLEMENTATIONDLG_H__91EFA5E5_F0DF_4C83_8814_B225F60730AD__INCLUDED_
// (j.gruber 2008-12-22 09:42) - PLID 28023 - tab created
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImplementationDlg.h : header file
//

#include "PatientDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CImplementationDlg dialog

class CImplementationDlg : public CPatientDialog
{
// Construction
public:
	CImplementationDlg(CWnd* pParent);   // standard constructor
	virtual void SetColor(OLE_COLOR nNewColor);

// Dialog Data
	//{{AFX_DATA(CImplementationDlg)
	enum { IDD = IDD_IMPLEMENTATION_DLG };
	CNxIconButton	m_btnEditSpecialist;
	CNxIconButton	m_btnEditSecSpecialist; // (j.gruber 2010-07-29 14:14) - PLID 39878 - secondary specialist
	CNxIconButton	m_btnEditStatus;
	CMirrorImageButton	m_btnImage;
	CNxIconButton	m_btn_GenTempList;
	CNxIconButton	m_btn_LadderSetup;
	CNxIconButton	m_btn_Down;
	CNxIconButton	m_btn_Up;
	CNxIconButton	m_btn_DeleteLadder;
	CNxIconButton	m_btn_AddLadder;
	CNxIconButton	m_btn_AutoComplete;
	CNxEdit	m_nxeditStatusNote;
	CNxEdit	m_nxeditClientWebsite;
	CNxEdit	m_nxeditEmrPointPerson;
	CNxEdit	m_nxeditEmrPointEmail;
	CNxEdit	m_nxeditPmPointPerson;
	CNxEdit	m_nxeditPmPointEmail;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImplementationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.gruber 2007-11-08 16:15) - PLID 28026 - added specialist and status datalists
	NXDATALIST2Lib::_DNxDataListPtr m_pSpecialistList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSecSpecList; // (j.gruber 2010-07-29 14:14) - PLID 39878 - secondary specialist
	NXDATALIST2Lib::_DNxDataListPtr m_pStatusList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLadderList;
	// (j.gruber 2008-02-19 16:30) - PLID 28070 - added status date
	NXTIMELib::_DNxTimePtr	m_pStatusDate;
	// (j.gruber 2008-05-22 14:45) - PLID 29437 - added followup
	NXTIMELib::_DNxTimePtr	m_pFollowupDate;

	// (j.gruber 2008-05-22 11:25) - PLID 30144
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeList;
	CDWordArray m_dwExpandedLadders;

	CString m_strEMRStatus;

	// (r.gonet 06/09/2010) - PLID 38871 - Keep old values in order to compare new values against for auditing changes
	CString m_strOldStatusNote;
	CString m_strOldEmrClientPointPerson;
	CString m_strOldEmrPointPersonEmail;
	CString m_strOldPMClientPointPerson;
	CString m_strOldPMPointPersonEmail;
	CString m_strOldEmrSpecialistID;
	CString m_strOldEmrStatusID;
	CString m_strOldEmrTypeID;
	CString m_strOldEmrSpecialist;
	CString m_strOldEmrStatus;
	CString m_strOldEmrType;

	// (j.gruber 2010-07-29 14:14) - PLID 39878 - secondary specialist
	CString m_strOldSecEmrSpecID;
	CString m_strOldSecEmrSpec;

	void Load();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void AddLadderToClient(long nLadderID);
	void CheckButtonStatus();
	void LoadLadderList(BOOL bKeepSelection = TRUE);
	CString GenerateActionText(long nActionID, CDWordArray *dwActionIDs, CStringArray *strActionPaths);
	void EmailDocuments(long nStepID, long nTypeID);
	void MergeDocument(CString strTemplateName, long nDocCategoryID);
	void MergePacket(long nStepID, long nDocumentCategoryID);
	void ActivateStep(long nStepID);
	void CompleteImplementationStep(long nStepID, long nNextStepID);
	void OnInsertAbove();
	void OnInsertBelow();
	void InsertStepAtPosition(long nPos, long nLadderID);
	long GetStepCountForLadder(long nLadderID);
	void OnDeleteStep();
	BOOL IsActionComplete(long nStepID, long nActionID);

	// (r.gonet 06/21/2010) - PLID 38871 - Accessor functions
	long GetEmrSpecialistID();
	CString GetEmrSpecialist();
	long GetEmrStatusID();
	CString GetEmrStatus();
	long GetEmrTypeID();
	CString GetEmrType();

	// (j.gruber 2010-07-29 14:14) - PLID 39878 - secondary specialist
	long GetEmrSecSpecialistID();
	CString GetEmrSecSpecialist();

	virtual void StoreDetails();
	long m_nPatientID;
	void Save(long nID);

	COleDateTime m_dtFollowup;
	bool m_bFollowupSet;
	
	// Generated message map functions
	// (j.gruber 2007-12-11 12:24) - PLID 28327 - be able to edit the ladder name
	// (j.gruber 2007-12-11 17:10) - PLID 28329 - made a go button for the website field
	// (j.gruber 2008-04-02 16:59) - PLID 29443 - added ability to edit statuses
	// (j.gruber 2008-04-02 16:59) - PLID 28979 - added ability to edit specialists
	// (j.gruber 2008-05-22 12:30) - PLID 30146 - added ability to edit types
	// (j.gruber 2008-12-22 09:25) - PLID 28071 - added client picture, website, point person, point person email
	//{{AFX_MSG(CImplementationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenEmrStatusList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingEmrStatusList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingEmrSpecialistList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenEmrSpecialistList(LPDISPATCH lpRow);
	afx_msg void OnTrySetSelFinishedEmrSpecialistList(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEmrStatusList(long nRowEnum, long nFlags);
	afx_msg void OnLeftClickClientImplementationLadders(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonUpClientImplementationLadders(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRowExpandedClientImplementationLadders(LPDISPATCH lpRow);
	afx_msg void OnRowCollapsedClientImplementationLadders(LPDISPATCH lpRow);
	afx_msg void OnLadderSetup();
	afx_msg void OnAddImplementationLadder();
	afx_msg void OnKillfocusEmrPointPerson();
	afx_msg void OnKillfocusEmrPointEmail();
	afx_msg void OnKillfocusPmPointEmail();
	afx_msg void OnKillfocusPmPointPerson();
	afx_msg void OnMoveStepUp();
	afx_msg void OnMoveStepDown();
	afx_msg void OnKillfocusStatusNote();
	afx_msg void OnDblClickCellClientImplementationLadders(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnEditingFinishedClientImplementationLadders(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDeleteImplementationLadder();
	afx_msg void OnSelChangedClientImplementationLadders(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnAutoComplete();
	afx_msg void OnEditingStartingClientImplementationLadders(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnGoWebsite();
	afx_msg void OnEditSpecialist();
	afx_msg void OnEditStatus();
	afx_msg void OnSelChosenEmrTypeList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingEmrTypeList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnEditEmrTypeList();
	afx_msg void OnKillFocusFollowupDate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SelChangingSecEmrSpecialistList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenSecEmrSpecialistList(LPDISPATCH lpRow);
	void TrySetSelFinishedSecEmrSpecialistList(long nRowEnum, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPLEMENTATIONDLG_H__91EFA5E5_F0DF_4C83_8814_B225F60730AD__INCLUDED_)
