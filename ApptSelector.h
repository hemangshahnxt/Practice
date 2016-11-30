#if !defined(AFX_APPTTYPESELECTOR_H__CCF9D496_7BBD_4DBA_83E6_D63360F19DEC__INCLUDED_)
#define AFX_APPTTYPESELECTOR_H__CCF9D496_7BBD_4DBA_83E6_D63360F19DEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ApptTypeSelector.h : header file
//

#include "filter.h"

#define FILTER_ID_ALL					0

#define FILTER_FIELD_STATUS				288
#define FILTER_FIELD_DATE				278
#define FILTER_FIELD_PURPOSE			281
#define FILTER_FIELD_LOCATION			289
#define FILTER_FIELD_RESOURCE			286
#define FILTER_FIELD_TYPE				283
#define FILTER_FIELD_CANCELLED			389

/////////////////////////////////////////////////////////////////////////////
// CApptSelector dialog

class CApptSelector : public CNxDialog
{
// Construction
public:
	CApptSelector(BOOL bNewFilter = TRUE, long nFilterID = -1, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long) = NULL, BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*) = NULL);   // standard constructor

	BOOL m_bIncludeCancelled,
		 m_bNewFilter;

	long m_nFilterID;

	CString m_strOldFilterName;

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButton variables for OK and Cancel
// Dialog Data
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CApptSelector)
	enum { IDD = IDD_APPT_TYPE_SELECTOR };
	NxButton	m_btnShowInactive;
	NxButton	m_dateFilter;
	NxButton	m_includeCancelled;
	CNxIconButton	m_typeRemoveAllBtn;
	CNxIconButton	m_typeRemoveBtn;
	CNxIconButton	m_typeAddAllBtn;
	CNxIconButton	m_typeAddBtn;
	CNxIconButton	m_statusRemoveAllBtn;
	CNxIconButton	m_statusRemoveBtn;
	CNxIconButton	m_statusAddAllBtn;
	CNxIconButton	m_statusAddBtn;
	CNxIconButton	m_resRemoveAllBtn;
	CNxIconButton	m_resRemoveBtn;
	CNxIconButton	m_resAddAllBtn;
	CNxIconButton	m_resAddBtn;
	CNxIconButton	m_purRemoveAllBtn;
	CNxIconButton	m_purRemoveBtn;
	CNxIconButton	m_purAddAllBtn;
	CNxIconButton	m_purAddBtn;
	CNxIconButton	m_locRemoveAllBtn;
	CNxIconButton	m_locRemoveBtn;
	CNxIconButton	m_locAddAllBtn;
	CNxIconButton	m_locAddBtn;
	CDateTimePicker	m_dateTo;
	CDateTimePicker	m_dateFrom;
	CComboBox	m_cboFilterList;
	CNxEdit	m_nxeditFilterNameEdit;
	CNxStatic	m_nxstaticFilterBkgLabel;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApptSelector)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_resUnselected,
					m_resSelected,
					m_purUnselected,
					m_purSelected,
					m_statusUnselected,
					m_statusSelected,
					m_locUnselected,
					m_locSelected,
					m_typeUnselected,
					m_typeSelected;

	BOOL m_bListSet[5];

	CString CreateIDString(NXDATALISTLib::_DNxDataListPtr pDataList);

	void SelectItemsFromIDString(NXDATALISTLib::_DNxDataListPtr pUnselectedDataList, NXDATALISTLib::_DNxDataListPtr pSelectedDataList, CString strIDList);

	CString CreateWhereClause(CString strFieldName, NXDATALISTLib::_DNxDataListPtr pSelectedDataList);

	CFilter *CreateNewFilterFromDlg();

	BOOL TranslateFilterToDlg();

	void InitNewFilter();

	BOOL SetDataItemsOnDlg(long nInfoId, FieldOperatorEnum foOperator, CString strValue, bool bUseOr, CArray<CString, CString> &arystrDates, CArray<FieldOperatorEnum, FieldOperatorEnum> &aryfoeOperator, bool bFirstDetail);

	void AddListItemsToFilter(NXDATALISTLib::_DNxDataListPtr pDataList, CFilter *pFilter, long nFieldId);

	bool SaveFilterToData(CFilter *pFilter);

	BOOL SetDateRange(CArray<CString, CString> &arystrDates, CArray<FieldOperatorEnum, FieldOperatorEnum> &aryfoeOperator);

	BOOL HaveItemsForListBeenSelected(long nInfoId);

	void SetItemsForListHaveBeenSelected(long nInfoId);
	
	//Callback functions for writing to data.
	BOOL (WINAPI* m_pfnIsActionSupported)(SupportedActionsEnum, long);
	BOOL (WINAPI* m_pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*);

	// Generated message map functions
	//{{AFX_MSG(CApptSelector)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnShowInactive();
	afx_msg void OnTypeAdd();
	afx_msg void OnTypeAddAll();
	afx_msg void OnTypeRemove();
	afx_msg void OnTypeRemoveAll();
	afx_msg void OnStatusAdd();
	afx_msg void OnStatusAddAll();
	afx_msg void OnStatusRemove();
	afx_msg void OnStatusRemoveAll();
	afx_msg void OnResAdd();
	afx_msg void OnResAddAll();
	afx_msg void OnResRemove();
	afx_msg void OnResRemoveAll();
	afx_msg void OnPurAdd();
	afx_msg void OnPurAddAll();
	afx_msg void OnPurRemove();
	afx_msg void OnPurRemoveAll();
	afx_msg void OnLocAdd();
	afx_msg void OnLocAddAll();
	afx_msg void OnLocRemove();
	afx_msg void OnLocRemoveAll();
	afx_msg void OnDblClickCellTypeSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellTypeUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellStatusSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellStatusUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellResSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellResUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellPurSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellPurUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLocSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLocUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnUseDateRange();
	afx_msg void OnIncludeCancelled();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPTTYPESELECTOR_H__CCF9D496_7BBD_4DBA_83E6_D63360F19DEC__INCLUDED_)
