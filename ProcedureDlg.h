#include "commondialog.h"

#if !defined(AFX_PROCEDUREDLG_H__F1B00451_B8C3_4FB1_85F7_A5D9403BE0FF__INCLUDED_)
#define AFX_PROCEDUREDLG_H__F1B00451_B8C3_4FB1_85F7_A5D9403BE0FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureDlg.h : header file
//
#include "color.h"
#include "Client.h"

/////////////////////////////////////////////////////////////////////////////
// CProcedureDlg dialog

class CProcedureDlg : public CNxDialog
{
// Construction
public:
	CProcedureDlg(CWnd* pParent);

	virtual void Refresh();
	// (j.gruber 2007-02-20 10:14) - PLID 24440 - moved to be public for the report
	bool GetProcedureID (long &id);

	// (z.manning, 05/12/2008) - PLID 29702 - Converted custom fields from rich text to edit controls
// Dialog Data
	//{{AFX_DATA(CProcedureDlg)
	enum { IDD = IDD_PROCEDURE_DLG };
	NxButton	m_btnServiceCodes;
	NxButton	m_btnRecurring;
	NxButton	m_btnMasterProc;
	CNxIconButton	m_btnAdvancedLadderAssignment;
	CNxIconButton	m_btnProcEmrAction;
	CNxIconButton	m_btnNew;
	CNxIconButton	m_btnMakeNonProcedural;
	CNxIconButton	m_btnEditSections;
	CNxIconButton	m_btnEditProcGroup;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnDefault;
	CNxIconButton	m_btnSetupDiscounts;
	CNxIconButton	m_btnAnesthesiaSetup;
	CNxLabel	m_MultiLadder;
	CNxIconButton	m_btnRightProcedure;
	CNxIconButton	m_btnLeftProcedure;
	NxButton	m_btnProcedureProducts;
	NxButton	m_btnDetailProc;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditOfficialTerm;
	CNxEdit	m_nxeditCustom1;
	CNxEdit	m_nxeditArrivalTime;
	CNxStatic	m_nxstaticLadderCaption;
	CNxStatic	m_nxstaticGroupCaption;
	CNxStatic	m_nxstaticMasterListCaption;
	CNxStatic	m_nxstaticCustom1Admin;
	CNxStatic	m_nxstaticCustom2Admin;
	CNxStatic	m_nxstaticCustom4Admin;
	CNxStatic	m_nxstaticCustom5Admin;
	CNxStatic	m_nxstaticCustom3Admin;
	CNxStatic	m_nxstaticCustom6Admin;
	CNxEdit	m_nxeditCustom2;
	CNxEdit	m_nxeditCustom4;
	CNxEdit	m_nxeditCustom5;
	CNxEdit	m_nxeditCustom3;
	CNxEdit	m_nxeditCustom6;
	CNxIconButton	m_btnInactivate;
	CNxIconButton	m_btnShowInactiveProcedures;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Save(int nID);
	void OnRemoveService(int serviceID);
	//bool GetLadderID(long &ladder); // (d.moore 2007-06-11 12:53) - PLID 14670 - No longer needed. Multiple selections are now possible.
	BOOL ChangeCustomLabel (const int nID);
	int GetLabelFieldID(int nID);

	void EnableAppropriateFields();

	void OnRadioCodeTypeChanged();

	bool CanDeleteProcedure(long nProcedureID);

	NXDATALISTLib::_DNxDataListPtr m_pProcGroupCombo;

	DWORD m_color;
	CBrush m_brush;

	long m_rightClicked;

	NXDATALISTLib::_DNxDataListPtr m_cptList,
					m_cptCombo,
					m_productCombo,
					m_procedureCombo,
					m_pNurse, 
					m_pAnesth,
					m_pAnesthesia,
					m_pMasterProcList;
	// (d.moore 2007-06-07 15:18) - PLID 14670 - Converted m_ladderCombo to NxDataList2
	NXDATALIST2Lib::_DNxDataListPtr m_ladderCombo;

	CTableChecker	m_procedureNameChecker,
					m_CPTCodeChecker,
					m_ProductChecker,
					m_ladderChecker,
					m_ContactChecker;
	
	// (d.moore 2007-06-11 12:46) - PLID 14670 - Array used to track multiple ladder selection for a procedure.
	CArray <long, long> m_rgLadderID;
	// (d.moore 2007-06-11 13:02) - PLID 14670 - Open a dialog to select multiple ladders for a procedure.
	//(e.lally 2010-07-28) PLID -Depreciated, always open the advanced configuration
	//void OpenLadderMultiList(CArray<long, long> &rgIdList);

	
	// Generated message map functions
	//{{AFX_MSG(CProcedureDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenProcedure(long nRow);
	afx_msg void OnSelChosenCptCombo(long nRow);
	afx_msg void OnRButtonUpCptList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnNew();
	afx_msg void OnDelete();
	afx_msg void OnMakeNonProcedural();
	afx_msg void OnDefault();
	afx_msg void OnEditProcGroup();
	afx_msg void OnSelChosenProcGroup(long nRow);
	afx_msg void OnRequeryFinishedProcGroup(short nFlags);
	afx_msg void OnSelChosenNurses(long nRow);
	afx_msg void OnSelChosenAnesth(long nRow);
	afx_msg void OnEditSectionsBtn();
	afx_msg void OnAnesthesiaSetup();
	afx_msg void OnSelChosenDefAnesthesia(long nRow);
	afx_msg void OnMasterProc();
	afx_msg void OnDetailProc();
	afx_msg void OnSelChosenMasterProcs(long nRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLeftProcedure();
	afx_msg void OnRightProcedure();
	afx_msg void OnProcEmrAction();
	afx_msg void OnBtnSetupDiscounts();
	afx_msg void OnCheckProcRecurs();
	afx_msg void OnRadioProcedureCptCodes();
	afx_msg void OnRadioProcedureProducts();
	afx_msg void OnSelChosenProcedureProductCombo(long nRow);
	afx_msg void OnTrySetSelFinishedNurses(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedAnesth(long nRowEnum, long nFlags);
	afx_msg void OnSelChosenLadderCombo(LPDISPATCH lpRow);
	afx_msg void OnLadderMultiLabel();
	afx_msg void OnRequeryFinishedLadderCombo(short nFlags);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnAdvancedLadderAssignment();
	afx_msg void OnInactivate();
	afx_msg void OnShowInactiveProcedures();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDUREDLG_H__F1B00451_B8C3_4FB1_85F7_A5D9403BE0FF__INCLUDED_)
