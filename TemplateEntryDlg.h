//{{AFX_INCLUDES()
#include "commondialog.h"
//}}AFX_INCLUDES
#if !defined(AFX_TEMPLATEENTRYDLG_H__0FC55233_59F5_11D2_80D7_00104B2FE914__INCLUDED_)
#define AFX_TEMPLATEENTRYDLG_H__0FC55233_59F5_11D2_80D7_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateEntryDlg.h : header file
//

#include "TemplateLineItemDlg.h"
#include "TemplateExceptionDlg.h"
#include <afxtempl.h>
class CTemplateLineItemInfo;
class CTemplateRuleInfo;
class CTemplateExceptionInfo;
struct CAuditTransaction;

/////////////////////////////////////////////////////////////////////////////
// CTemplateEntryDlg dialog

class CTemplateEntryDlg : public CNxDialog
{
// Construction
public:
	OLE_COLOR m_nColor;
	// (z.manning 2014-12-11 09:32) - PLID 64230 - Added bUseResourceAvailTemplates to constructor
	CTemplateEntryDlg(bool bUseResourceAvailTemplates, CWnd* pParent);   // standard constructor
	//TES 6/19/2010 - PLID 39262 - Pass in whether we're using Resource Availability templates
	int AddTemplate(); // (z.manning, 02/27/2007) - Changed return type from long to int.
	int EditTemplate(long nTemplateID);
	// (c.haag 2006-11-02 13:23) - PLID 23329 - Date ranges are now associated with template line items, not templates
	// (c.haag 2006-11-13 09:16) - PLID 5993 - We read template exceptions now
	//TES 6/19/2010 - PLID 39262 - We also output a location ID, if this is a Resource Availability template
	// (z.manning 2011-12-07 14:34) - PLID 46910 - Removed the line item array param since we already have a member variable for that.
	void ReadTemplate(long nTemplateID, CString &strTemplateName, OLE_COLOR &nColor, long &nLocationID);
	// (c.haag 2006-11-02 13:26) - PLID 23329 - There are no longer template-specific resources.
	// They are now specific to template line items. TemplateConnectT has been rendered obselete.
	//void ReadTemplateConnect(long nTemplateID, CDWordArray &aryResourceIDs);
	//TES 6/19/2010 - PLID 39262 - Dead code
	//long GetTemplateID(const CString &strTemplateName);
	CString FormatExceptionText(const CTemplateExceptionInfo*);

	BOOL m_bAllowEdit;
	BOOL m_bIsPreviewing;

	// (c.haag 2007-02-21 11:24) - PLID 23784 - We now use datalist 2's in the form
	NXDATALIST2Lib::_DNxDataListPtr m_pLineItemList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRuleList;
	NXDATALIST2Lib::_DNxDataListPtr m_pExceptionList;
	//TES 6/19/2010 - PLID 39262 - A dropdown for the location, used on Resource Availability templates
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CTemplateEntryDlg)
	enum { IDD = IDD_TEMPLATE_ENTRY_DLG };
	//CListBox	m_lstLineItems;
	//CListBox	m_lstRules;
	//CListBox	m_lstExceptions;
	CString	m_strTemplateName;
	CCommonDialog60	m_ctrlColorPicker;
	CNxEdit	m_nxeditTemplateNameEdit;
	CNxIconButton	m_btnAddLineItem;
	CNxIconButton	m_btnEditLineItem;
	CNxIconButton	m_btnRemoveLineItem;
	CNxIconButton	m_btnAddRule;
	CNxIconButton	m_btnEditRule;
	CNxIconButton	m_btnRemoveRule;
	CNxIconButton	m_btnAddException;
	CNxIconButton	m_btnEditException;
	CNxIconButton	m_btnRemoveException;
	CNxIconButton	m_btnPreviewReport;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_checkHideOldLineItems;
	//}}AFX_DATA

	// (c.haag 2006-11-02 13:26) - PLID 23329 - There are no longer template-specific resources.
	// They are now specific to template line items.
	//NXDATALISTLib::_DNxDataListPtr	m_pResourceList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nTemplateID;
	//TES 6/19/2010 - PLID 39262
	bool m_bUseResourceAvailTemplates;

	//TES 6/19/2010 - PLID 39262 - Used for inactive locations
	long m_nPendingLocationID;

	// (z.manning 2011-12-08 10:16) - PLID 46906 - Keep track of deleted objects
	CArray<long,long> m_arynDeletedLineItemIDs;
	CArray<long,long> m_arynDeletedRuleIDs;
	CArray<long,long> m_arynDeletedExceptionIDs;

	BOOL Save();
	//TES 6/19/2010 - PLID 39262 - Pass in the Location ID (for Resource Availability templates)
	void SaveCurrentTemplate(long nLocationID);
	// (c.haag 2006-11-02 13:23) - PLID 23329 - Date ranges are now associated with template line items, not templates
	void PrepareWriteTemplate(CString &strSql, CAuditTransaction *pAuditTran, long &nTemplateID, CString strTemplateName, long nLocationID, OLE_COLOR nColor, CPtrArray *pLineItems, CArray<CTemplateRuleInfo*,CTemplateRuleInfo*> *paryRules, CPtrArray *pExceptions);
	// (c.haag 2006-11-02 13:26) - PLID 23329 - There are no longer template-specific resources.
	// They are now specific to template line items. TemplateConnectT has been rendered obselete.
	//void PrepareWriteTemplateConnect(CString &strSql, const long &nTemplateID, const CDWordArray &aryResourceIDs);
	void PrepareAddTemplateRule(CString &strSql, long nTemplateID, CTemplateRuleInfo *pRule);
	void PrepareAddException(CString& strSql, long nTemplateID, CTemplateExceptionInfo* pException);
	void LoadCurrentTemplate(long nTemplateID);
	CTemplateLineItemInfo *NewTemplateLineItem(ADODB::FieldsPtr &pfldsTemplateItemT);
	CTemplateRuleInfo *NewTemplateRule(ADODB::FieldsPtr &pfldsTemplateRuleT);
	CTemplateExceptionInfo* NewException(ADODB::FieldsPtr &pfldsTemplateExceptionT);
	
	CTemplateLineItemDlg m_dlgLineItem;
	CTemplateExceptionDlg m_dlgException;
	void ReflectCurrentStateOnBtns();

	BOOL ShouldLineItemBeVisible(CTemplateLineItemInfo* pLineItem);
	void RequeryLineItemList();

	void CleanUp(); // (z.manning 2011-12-07 17:44) - PLID 46906

	// Generated message map functions
	//{{AFX_MSG(CTemplateEntryDlg)
	afx_msg void OnAddLineItemBtn();
	afx_msg void OnEditLineItemBtn();
	afx_msg void OnRemoveLineItemBtn();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChooseColorBtn();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void OnCancel();
	afx_msg void OnAddRuleBtn();
	afx_msg void OnEditRuleBtn();
	afx_msg void OnRemoveRuleBtn();
	afx_msg void OnSelchangeRuleList();
	afx_msg void OnDblclkRuleList();
	afx_msg void OnBtnPreviewReport();
	afx_msg void OnAddExceptionBtn();
	afx_msg void OnEditExceptionBtn();
	afx_msg void OnRemoveExceptionBtn();
	afx_msg void OnSelchangeExceptionList();
	afx_msg void OnDblclkExceptionList();
	afx_msg void OnCheckHideOldLineitems();
	afx_msg void OnCheckHideAppliedLineItems(); // (z.manning 2014-12-23 10:49) - PLID 64296
	afx_msg void OnSelChangedListLineItems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellListLineItems(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChangedListRules(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellListRules(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChangedListExceptions(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellListExceptions(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void OnTrySetSelFinishedTemplateLocation(long nRowEnum, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEENTRYDLG_H__0FC55233_59F5_11D2_80D7_00104B2FE914__INCLUDED_)
