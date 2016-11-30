#if !defined(AFX_NEXFORMSIMPORTNEWPROCEDURESHEET_H__CA71CA6C_C15B_476D_9B00_B491C70082BB__INCLUDED_)
#define AFX_NEXFORMSIMPORTNEWPROCEDURESHEET_H__CA71CA6C_C15B_476D_9B00_B491C70082BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportNewProcedureSheet.h : header file
//

#include "NexFormsImportWizardSheet.h"

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/


struct ExistingProcedure
{
	long nID;
	CString strName;
	long nMasterProcedureID;
	CArray<long,long> arynLadderTemplateIDs;
	long nNexFormsVersion; // (z.manning, 07/23/2007) - PLID 26774

	ExistingProcedure()
	{
		nID = 0;
		nMasterProcedureID = 0;
		nNexFormsVersion = 0;
	}

	ExistingProcedure& operator=(const ExistingProcedure &source)
	{
		nID = source.nID;
		strName = source.strName;
		nMasterProcedureID = source.nMasterProcedureID;
		nNexFormsVersion = source.nNexFormsVersion;

		// (z.manning, 02/12/2008) - PLID 28901 - Make sure we remove any pre-existing ladder IDs from this
		// existing procedure object to ensure an exact assignment.
		arynLadderTemplateIDs.RemoveAll();
		for(int i = 0; i < source.arynLadderTemplateIDs.GetSize(); i++)
		{
			arynLadderTemplateIDs.Add(source.arynLadderTemplateIDs.GetAt(i));
		}

		return *this;
	}

	ExistingProcedure(const ExistingProcedure &procToBeCopied)
	{
		*this = procToBeCopied;
	}
};

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportNewProcedureSheet dialog

class CNexFormsImportNewProcedureSheet : public CNexFormsImportWizardSheet
{
// Construction
public:
	CNexFormsImportNewProcedureSheet(CNexFormsImportWizardMasterDlg* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNexFormsImportNewProcedureSheet)
	enum { IDD = IDD_NEXFORMS_IMPORT_NEW_PROCEDURE_SHEET };
	CNxIconButton	m_btnSelectNone;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectDermOnly;
	//}}AFX_DATA

	virtual void Load();
	virtual BOOL Validate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportNewProcedureSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlProcedureList;

	CArray<ExistingProcedure,ExistingProcedure&> m_aryExistingProcedures;

	BOOL m_bInitialLoad;

	void LoadExistingProcedures();

	void MatchProceduresWithExisting();

	void SelectHighlightedRows(BOOL bSelect);
	void SelectAllRows(BOOL bSelect);

	void RefreshMasterProcedureEmbeddedCombo();
	void RefreshLadderEmbeddedCombo();

	void UpdateMasterProcedureIDs(long nCurrentMasterProcedureID, long nNewMasterProcedureID);

	void UpdateLadderCombo(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void UpdateProcedureTrackingLadders();

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportNewProcedureSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedNexformsProcedureList(LPDISPATCH lpRow, short nCol, const _variant_t &varOldValue, const _variant_t &varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownNexformsProcedureList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingNexformsProcedureList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnSelectDermOnly();
	afx_msg void OnEditingStartingNexformsProcedureList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTNEWPROCEDURESHEET_H__CA71CA6C_C15B_476D_9B00_B491C70082BB__INCLUDED_)
