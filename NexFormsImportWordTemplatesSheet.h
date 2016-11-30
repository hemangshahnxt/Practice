#if !defined(AFX_NEXFORMSIMPORTWORDTEMPLATESSHEET_H__25E487B1_058C_488D_BB68_BE0FE600CE7E__INCLUDED_)
#define AFX_NEXFORMSIMPORTWORDTEMPLATESSHEET_H__25E487B1_058C_488D_BB68_BE0FE600CE7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportWordTemplatesSheet.h : header file
//

#include "NexFormsImportWizardSheet.h"

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

struct SelectedCategory
{
	CString strName;
	BOOL bSelected;
	CString strParent;

	SelectedCategory()
	{
		bSelected = TRUE;
	}
};

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWordTemplatesSheet dialog

class CNexFormsImportWordTemplatesSheet : public CNexFormsImportWizardSheet
{
// Construction
public:
	CNexFormsImportWordTemplatesSheet(CNexFormsImportWizardMasterDlg* pParent);   // standard constructor
	~CNexFormsImportWordTemplatesSheet();

// Dialog Data
	//{{AFX_DATA(CNexFormsImportWordTemplatesSheet)
	enum { IDD = IDD_NEXFORMS_IMPORT_WORD_TEMPLATES_SHEET };
	CNxIconButton	m_btnUnselectAllTemplates;
	CNxIconButton	m_btnUnselectAllPackets;
	CNxIconButton	m_btnSelectAllTemplates;
	CNxIconButton	m_btnSelectAllPackets;
	//}}AFX_DATA

	virtual void Load();
	virtual BOOL Validate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportWordTemplatesSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlCategoryTree;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlWordTemplates;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlPackets;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlGroupFilter;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlExistingPackets;

	CArray<SelectedCategory*,SelectedCategory*> m_arySelectedCategories;

	BOOL m_bInitialLoad;

	void SelectAllWordTemplates(BOOL bSelect);
	void SelectAllPackets(BOOL bSelect);

	void SelectNonExistingTemplateNames();
	void SelectNonExistingPackets();

	void ShowTemplateByType(CString strType, BOOL bShow);
	void ShowTemplateByCategory(CString strCategory, BOOL bShow);

	BOOL DoesPacketExistInList(LPDISPATCH lpDatalist, short nCol, CString strPacketName, LPDISPATCH lpRowToIgnore);

	void UpdateRowVisibility(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void HandleGroupChange();

	// (z.manning, 07/19/2007) - PLID 26746 - Opens a word template from the given row from the word template list.
	void OpenWordTemplateFromRow(LPDISPATCH lpRow);

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportWordTemplatesSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedWordTemplateCategoryTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelectAllPackets();
	afx_msg void OnUnselectAllPackets();
	afx_msg void OnSelectAllTemplates();
	afx_msg void OnUnselectAllTemplates();
	afx_msg void OnEditingFinishedPacketList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownPacketList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingPacketList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedGroupFilter(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRButtonDownWordTemplateList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedWordTemplateList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTWORDTEMPLATESSHEET_H__25E487B1_058C_488D_BB68_BE0FE600CE7E__INCLUDED_)
