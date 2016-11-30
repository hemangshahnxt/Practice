#if !defined(AFX_NEXFORMSIMPORTLADDERSHEET_H__93EA1A1C_8F15_4D86_A47A_F58037D409B6__INCLUDED_)
#define AFX_NEXFORMSIMPORTLADDERSHEET_H__93EA1A1C_8F15_4D86_A47A_F58037D409B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportLadderSheet.h : header file
//

#include "NexFormsImportWizardSheet.h"

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportLadderSheet dialog

class CNexFormsImportLadderSheet : public CNexFormsImportWizardSheet
{
// Construction
public:
	CNexFormsImportLadderSheet(CNexFormsImportWizardMasterDlg* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNexFormsImportLadderSheet)
	enum { IDD = IDD_NEXFORMS_IMPORT_LADDER_SHEET };
		// NOTE: the ClassWizard will add data members here
	CNxIconButton m_btnAddAllLadders;
	CNxIconButton m_btnAddLadder;
	CNxIconButton m_btnRemoveLadder;
	CNxIconButton m_btnRemoveAllLadders;
	//}}AFX_DATA

	virtual void Load();
	virtual BOOL Validate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportLadderSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlExistingLadders;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlAvailableLadders;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlLaddersToImport;

	void MoveRowFromAvailableListToImportList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void MoveRowFromImportListToAvailableList(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void FillDefaultLaddersToImport();

	void PopupContextMenu(LPDISPATCH lpDatalist, LPDISPATCH lpRow);

	void UpdateMasterExistingProcedureArray();

	void UpdateDependentActions(NexFormsLadder *ladder, BOOL bNeeded);

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportLadderSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddLadder();
	afx_msg void OnAddAllLadders();
	afx_msg void OnRemoveLadder();
	afx_msg void OnRemoveAllLadders();
	afx_msg void OnDblClickCellAvailableLadders(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellLaddersToImport(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRButtonDownAvailableLadders(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownLaddersToImport(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingAvailableLadders(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedAvailableLadders(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingLaddersToImport(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedLaddersToImport(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedExistingLadders(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTLADDERSHEET_H__93EA1A1C_8F15_4D86_A47A_F58037D409B6__INCLUDED_)
