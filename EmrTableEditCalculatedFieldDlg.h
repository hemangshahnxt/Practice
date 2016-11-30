#if !defined(AFX_EMRTABLEEDITCALCULATEDFIELDDLG_H__B8B9DEFD_6EDD_4146_B760_1CE412D6DFE5__INCLUDED_)
#define AFX_EMRTABLEEDITCALCULATEDFIELDDLG_H__B8B9DEFD_6EDD_4146_B760_1CE412D6DFE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTableEditCalculatedFieldDlg.h : header file
//

#include "EmrItemEntryDlg.h"


// (z.manning, 05/30/2008) - PLID 16443 - Added global function to get a column's operand based on its index
CString GetColumnOperand(long nColIndex);
CString GetRowOperand(long nRowIndex);
BOOL IsDataElementReferencedInAnyFormula(CEmrInfoDataElement *peide, CEmrInfoDataElementArray &aryRows, CEmrInfoDataElementArray &aryColumns, OUT CString &strReferencingItem);
BOOL IsDataElementReferencedInElement(CEmrInfoDataElement *peidePotentialReference, CEmrInfoDataElement *peideFormula);
BOOL IsDataElementReferencedInElement(CEmrInfoDataElement *peidePotentialReference, const CString strFormula);
// (z.manning 2008-06-05 17:18) - PLID 30155 - Returns the index of the row or column or -1 if invalid
long GetRowIndexFromOperand(CString strOperand);
short GetColumnIndexFromOperand(CString strOperand);


// (z.manning, 05/22/2008) - PLID 30145 - Created
/////////////////////////////////////////////////////////////////////////////
// CEmrTableEditCalculatedFieldDlg dialog

class CEmrTableEditCalculatedFieldDlg : public CNxDialog
{
// Construction
public:
	// (z.manning, 05/29/2008) - PLID 16443 - The constructor now takes a CEmrInfoDataElement instead of just an index.
	// (j.jones 2011-07-08 13:18) - PLID 43032 - added eDataSubType
	CEmrTableEditCalculatedFieldDlg(CEmrInfoDataElementArray &aryRows, CEmrInfoDataElementArray &aryColumns,
		CEmrItemEntryDlg* pParent, EmrInfoSubType eDataSubType);   // standard constructor
	~CEmrTableEditCalculatedFieldDlg();

	// (z.manning, 05/22/2008) - PLID 16643
	CString GetFormula();

	void SetEditingRow(long nRowSortOrder);
	void SetEditingColumn(long nColumnSortOrder);
	void ResizeHeight();

	CEmrInfoDataElementArray* GetRows();
	CEmrInfoDataElementArray* GetColumns();

public:
	// (c.haag 2008-10-23 12:41) - PLID 31834 - Table flipping support
	BOOL m_bTableRowsAsFields;

// Dialog Data
	//{{AFX_DATA(CEmrTableEditCalculatedFieldDlg)
	enum { IDD = IDD_EMR_TABLE_EDIT_CALCULATED_FIELD };
	CRichEditCtrl	m_richeditFormula;
	CNxIconButton	m_btnTestFormula;
	CNxIconButton	m_btnPlus;
	CNxIconButton	m_btnParentheses;
	CNxIconButton	m_btnNumber9;
	CNxIconButton	m_btnNumber8;
	CNxIconButton	m_btnNumber7;
	CNxIconButton	m_btnNumber6;
	CNxIconButton	m_btnNumber5;
	CNxIconButton	m_btnNumber4;
	CNxIconButton	m_btnNumber3;
	CNxIconButton	m_btnNumber2;
	CNxIconButton	m_btnNumber1;
	CNxIconButton	m_btnNumber0;
	CNxIconButton	m_btnMultiply;
	CNxIconButton	m_btnMinus;
	CNxIconButton	m_btnExponent;
	CNxIconButton	m_btnDot;
	CNxIconButton	m_btnDivide;
	CNxIconButton	m_btnModulo; // (z.manning 2011-05-26 10:24) - PLID 43851
	CNxIconButton	m_btnClearFormula;
	CNxIconButton	m_btnBackspace;
	CNxIconButton	m_btnOk;
	CNxStatic	m_nxstaticFormulaLabel;
	CNxEdit	m_nxeditDecimalPlaces;
	CNxIconButton	m_btnCancel;
	BYTE	m_nDecimalPlaces;
	CString	m_strFormula;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrTableEditCalculatedFieldDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CEmrInfoDataElementArray m_aryRows;
	CEmrInfoDataElementArray m_aryColumns;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlTable;

	long m_nEditingColumnSortOrder;
	long m_nEditingRowSortOrder;

	long m_nCaretIndex;
	long m_nIdealTableWidth; // (j.luckoski 2012-08-30 14:41) - PLID 51755
	bool m_bRunResize;

	CEmrItemEntryDlg *m_pEmrItemEntryDlg;

	// (j.jones 2011-07-08 13:18) - PLID 43032 - tracks if this table is a Current Medications or Allergies table
	EmrInfoSubType m_DataSubType;

	CFont *m_pButtonFont;

	BOOL TestFormula();

	void UpdateView(BOOL bReload);

	void InsertFormulaText(const CString str);

	void Save();

	BOOL CanHaveFormula(CEmrInfoDataElement *peide, BOOL bSilent = FALSE);

	CEmrInfoDataElement* GetCurrentData();

	// (z.manning 2008-06-09 11:05) - PLID 30145 - Returns true if the item being referenced is allowed to be
	// in the formula item's formula.
	BOOL CanItemBeReferencedInFormulaForItem(IN CEmrInfoDataElement *peideReference, IN CEmrInfoDataElement *peideFormula);

	void SetReadOnly(BOOL bReadOnly);

	void ReflectCurrentFormula();

	// Generated message map functions
	//{{AFX_MSG(CEmrTableEditCalculatedFieldDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTestFormula();
	afx_msg void OnLeftClickSampleTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDivide();
	afx_msg void OnExponent();
	afx_msg void OnMinus();
	afx_msg void OnParentheses();
	afx_msg void OnPlus();
	afx_msg void OnMultiply();
	virtual void OnOK();
	afx_msg void OnClearFormula();
	afx_msg void OnDot();
	afx_msg void OnNumber0();
	afx_msg void OnNumber1();
	afx_msg void OnNumber2();
	afx_msg void OnNumber3();
	afx_msg void OnNumber4();
	afx_msg void OnNumber5();
	afx_msg void OnNumber6();
	afx_msg void OnNumber7();
	afx_msg void OnNumber8();
	afx_msg void OnNumber9();
	afx_msg void OnSelchangeFormula(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBackspace();
	afx_msg void OnChangeFormula();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedModulo();
	afx_msg void OnBnClickedFormulaForTransform(); // (z.manning 2011-05-31 09:40) - PLID 42131
public:
	void ColumnSizingFinishedSampleTable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void SelSetSampleTable(LPDISPATCH lpSel);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTABLEEDITCALCULATEDFIELDDLG_H__B8B9DEFD_6EDD_4146_B760_1CE412D6DFE5__INCLUDED_)
