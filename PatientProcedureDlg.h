#if !defined(AFX_PATIENTPROCEDUREDLG_H__DCFA248A_51B9_4280_8831_3E56B08D1A67__INCLUDED_)
#define AFX_PATIENTPROCEDUREDLG_H__DCFA248A_51B9_4280_8831_3E56B08D1A67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientProcedureDlg.h : header file
//
#include "PatientDialog.h"

class CBillingModuleDlg;

/////////////////////////////////////////////////////////////////////////////
// CPatientProcedureDlg dialog
class CPatientProcedureDlg : public CPatientDialog
{
// Construction
public:
	CPatientProcedureDlg(CWnd* pParent);   // standard constructor

	CTableChecker m_tcTask;
	// (j.jones 2010-10-11 16:37) - PLID 35424 - added tablechecker for NetUtils::LaddersT
	CTableChecker m_tcLadders;

	virtual void SetColor(OLE_COLOR nNewColor);
// Dialog Data
	//{{AFX_DATA(CPatientProcedureDlg)
	enum { IDD = IDD_PATIENT_PROCEDURE_DLG };
	CNxIconButton	m_btnDeleteProc;
	CNxIconButton	m_btnNewProc;
	NxButton	m_checkRememberColWidths;
	NxButton	m_radioActive;
	NxButton	m_radioInactive;
	NxButton	m_radioAll;
	NxButton	m_hideEMROnlyPICs; // (b.eyers 2015-06-25) - PLID 39619
	CNxColor	m_bkg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientProcedureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.walling 2010-10-13 07:26) - PLID 40977 - Keep track of the patient ID
	long m_id;

	// (a.walling 2010-10-12 17:40) - PLID 40977
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void Refresh();
	void SecureControls();

	// (c.haag 2009-08-12 11:07) - PLID 35157 - TRUE if the popup menu is visible
	BOOL m_bPopupMenuVisible;
	// (c.haag 2009-08-12 11:07) - PLID 35157 - TRUE if we need to refresh the tab after
	// the popup menu was dismissed (usually due to the reception of a table checker)
	BOOL m_bNeedPostPopupRefresh;

	NXDATALISTLib::_DNxDataListPtr	m_procedureList;
	BOOL m_bReadOnly; // (r.galicki 2008-09-02 14:59) - PLID 27363
	void OnGo();
	void Go(long nRow);
	void OnAdd();
	void OnDelete();

	void OnApplyEvent();
	void OnChangeStatus();

	typedef enum ColumnPos
	{
		CP_RowType = 0,
		CP_LadderID = 1,
		CP_PicID = 2,
		CP_ProcInfoID = 3,
		CP_StepID = 4,
		CP_TopArrow = 5,
		CP_Date = 6,
		CP_Name = 7,
		CP_Description = 8,
		CP_Notes = 9,
		CP_UserNames = 10,	// (j.jones 2008-12-01 08:43) - PLID 32262 - changed UserID to UserNames
		CP_Done = 11,
		CP_DoneDate = 12,
		CP_StepOrder = 13,
		CP_EventID = 14,
		CP_EventType = 15,
		CP_Skippable = 16,
		CP_OpenPIC = 17,	// (j.jones 2008-11-17 17:48) - PLID 30926 - added OpenPIC column
		CP_DefaultScope = 18,	//TES 7/16/2010 - PLID 39400
	} ColumnPos;

	//Puts a top or mid-level row into the datalist at nStartRow, as well as any other visible rows(if it is expanded).
	//Returns the row index after the last row drawn by the function.
	//You can set bAddSubRows to true if you only want to insert one row.
	//TES 9/29/2004 - These rows may not represent a ladder, so pass in the procinfoid as well (ladderid may be -1).
	// (j.jones 2008-09-03 16:53) - PLID 10417 - InsertTopLevel now no longer runs a recordset to get its data,
	// and can instead take in all the information it needs. I added InsertTopLevel_FromData for the rare cases where
	// we do indeed need to load a recordset for the given ladder.
	// (j.jones 2008-12-01 08:57) - PLID 32262 - changed nUserID to strUserNames
	int InsertTopLevel(int nStartRow, long nLadderID, long nPicID, long nProcInfoID,
						 CString strLadderName, BOOL bHasProcedure, long nNextStepID,
						 CString strStatus, BOOL bIsActive, COleDateTime dtDate,
						 CString strNotes, _variant_t varDoneDate, CString strUserNames,
						 bool bAddSubRows = true);
	int InsertTopLevel_FromData(int nStartRow, long nLadderID, long nPicID, long nProcInfoID, bool bAddSubRows = true);

	// (j.jones 2008-09-04 08:59) - PLID 10471 - InsertMidLevel now no longer runs a recordset to get its data,
	// and can instead take in all the information it needs.
	// (j.jones 2008-11-17 17:50) - PLID 30926 - added varOpenPIC
	// (j.jones 2008-11-26 16:00) - PLID 32262 - changed nUserID to strUserNames
	//TES 7/16/2010 - PLID 39400 - Added varDefaultScope
	int InsertMidLevel(int nStartRow, long nLadderID, long nPicID, long nProcInfoID, long nStepID, 
						long nEventID, long nEventType, CString strStepName, long nAction,
						_variant_t varActiveDate, CString strNote, _variant_t varCompletedDate,
						CString strUserNames, long nStepOrder, _variant_t varSkippable, _variant_t varOpenPIC,
						_variant_t varDefaultScope,
						bool bAddSubRows = true);

	void RemoveLadder(long nLadderID);
	void RefreshLadder(long nLadderID);

	//m.hancock - 5/2/2006 - PLID 20308 - Replaced RemoveLadder() and RefreshLadder with these two functions
	//because we need to treat records by the ProcInfoID rather than the LadderID
	void RemoveProcInfo(long nProcInfoID);
	void RefreshProcInfo(long nProcInfoID);

	//Puts an event-level row into the datalist.  This will only ever take one row.
	//void InsertEventLevel(int nRow, long nEventID, long nLadderID, long nStepID);

	//nRowType is 1 for a top-level row (nStepID will be ignored), 2 for a mid-level row.
	bool IsRowExpanded(int nID, int nRowType);

	//Used to store the state of all the arrows
	CMap<long,long,bool,bool> m_mTopLevel;

	CBillingModuleDlg* m_pBillingDlg;
	CWnd* m_pOldFinancialDlg;

	CString m_strOriginalColSizes;
	void SetColumnSizes();
	void ResetColumnSizes();

	//Called from InsertTopLevel as well as OnTableChanged.
	void SetLadderDescription(NXDATALISTLib::IRowSettingsPtr pRow, long nLadderID, long nNextStepID, CString strStatusActive, BOOL bIsStatusActive);

	// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - OnPostEditBill should use WPARAM/LPARAM
	// Generated message map functions
	//{{AFX_MSG(CPatientProcedureDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewProcedure();
	afx_msg void OnDeleteProcedure();
	afx_msg void OnEditingFinishedProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnGotoProcedure();
	afx_msg void OnLeftClickProcedureListTrackable(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRButtonDownProcedureListTrackable(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingProcedureListTrackable(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnShowProcInfo();
	afx_msg void OnDblClickCellProcedureListTrackable(long nRowIndex, short nColIndex);
	afx_msg void OnRequeryFinishedProcedureListTrackable(short nFlags);
	afx_msg void OnUnskipStep();
	afx_msg void OnSkipStep();
	afx_msg LRESULT OnPostEditBill(WPARAM iBillID, LPARAM iSaveType);
	afx_msg void OnSelChangedProcedureListTrackable(long nNewSel);
	afx_msg void OnPutOnHold();
	afx_msg void OnActivate();
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnActiveLadders();
	afx_msg void OnInactiveLadders();
	afx_msg void OnAllLadders();
	afx_msg void OnColumnClickingProcedureListTrackable(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnTrackingRememberColWidths();
	afx_msg void OnColumnSizingFinishedProcedureListTrackable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);	
	afx_msg void OnMarkDone();
	afx_msg void OnMergeLadder();
	afx_msg void OnHideEMROnlyPICs(); // (b.eyers 2015-06-25) - PLID 39619
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTPROCEDUREDLG_H__DCFA248A_51B9_4280_8831_3E56B08D1A67__INCLUDED_)
