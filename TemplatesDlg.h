//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_TEMPLATESDLG_H__2309DE43_5A34_11D2_80D8_00104B2FE914__INCLUDED_)
#define AFX_TEMPLATESDLG_H__2309DE43_5A34_11D2_80D8_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplatesDlg.h : header file
//
#include "commondialog.h"
#include "TemplateItemEntryGraphicalDlg.h"
#include "NxAPI.h"

class CNxSchedulerDlg;

/////////////////////////////////////////////////////////////////////////////
// CTemplatesDlg dialog

class CTemplatesDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2014-12-03 14:30) - PLID 64332 - Added param for dialog type
	CTemplatesDlg(ESchedulerTemplateEditorType eType, CWnd* pParent);   // standard constructor
	// (c.haag 2014-12-15) - PLID 64246 - Set a default collection ID
	void SetDefaultCollectionID(_bstr_t bstrCollectionID);

	long SetSelTemplateID(long nTemplateID);
	long GetSelTemplateID();
	void EnableControls();
	void RequeryTemplateList();
	void ReflectTemplateChangesOnSchedule();

	// (z.manning 2014-12-04 11:18) - PLID 64215
	NexTech_Accessor::_SchedulerTemplateCollectionPtr GetSelectedCollection();

	// (z.manning 2014-12-17 14:01) - PLID 64427
	NexTech_Accessor::_SchedulerTemplateCollectionTemplatePtr GetCollectionTemplateByID(const long nCollectionTemplateID);

	// (z.manning, 07/28/2008) - PLID 30865 - Added an NxButton for the show old templates check
// Dialog Data
	//{{AFX_DATA(CTemplatesDlg)
	enum { IDD = IDD_TEMPLATES_DLG };
	CNxIconButton	m_movePriorityDownBtn;
	CNxIconButton	m_movePriorityUpBtn;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnEdit;
	NxButton	m_btnCheckOldTemplates;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplatesDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CNxSchedulerDlg *m_pdlgScheduler;
	
	NXDATALISTLib::_DNxDataListPtr m_lstTemplates;
	enum TemplateListColumns {
		tlcID = 0,
		tlcColorValue,
		tlcColorDisplay,
		tlcName,
		tlcPriority,
	};

	std::map<long, NexTech_Accessor::_SchedulerTemplateCollectionPtr> m_mapCollections;

	CCommonDialog60	m_ctrlColorPicker;

	// (z.manning 2014-12-03 14:31) - PLID 64332
	ESchedulerTemplateEditorType m_eEditorType;

	//TES 6/19/2010 - PLID 5888
	EBuiltInObjectIDs m_bio;

	// (c.haag 2014-12-15) - PLID 64246 - Default collection ID
	_bstr_t m_bstrDefaultCollectionID;

	CString m_strShowOldTempaltesPropertyName;

	long SwapTemplates(long nRow1, long nRow2);
	DWORD GetTemplateFromDateTime(COleDateTime dtDay, COleDateTime dtTime);

	// (c.haag 2014-12-15) - PLID 64245 - This should be called any time a collection is added to the list.
	// Returns the ordinal of the newly created row.
	long AddCollectionToList(NexTech_Accessor::_SchedulerTemplateCollectionPtr pCollection);

	// (c.haag 2014-12-12 10:02) - PLID 64244 - Invoked to remove a collection
	void OnRemoveCollection();
	// (c.haag 2014-12-12 10:02) - PLID 64244 - Invoked to remove a template. Moved from OnRemoveTemplateBtn
	void OnRemoveTemplate();

	// Generated message map functions
	//{{AFX_MSG(CTemplatesDlg)
	afx_msg void OnAddTemplateBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnRemoveTemplateBtn();
	afx_msg void OnEditTemplateBtn();	
	afx_msg void OnCopyCollectionBtn(); // (c.haag 2014-12-15) - PLID 64245 - Called when the user elects to copy the selected collection
	afx_msg void OnMovePriorityUpBtn();
	afx_msg void OnMovePriorityDownBtn();
	afx_msg void OnCheckOldTemplates();
	afx_msg void OnSelChangedTemplatesList(long nRow);
	afx_msg void OnLButtonDownTemplatesList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonUpTemplatesList(long nRow, short nCol, long x, long y, long nFlags); // (z.manning 2014-12-10 16:43) - PLID 64228
	afx_msg void OnDblClickCellTemplatesList(long nRowIndex, short nColIndex);
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC); // (z.manning 2015-01-15 10:09) - PLID 64210
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATESDLG_H__2309DE43_5A34_11D2_80D8_00104B2FE914__INCLUDED_)
