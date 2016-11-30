#if !defined(AFX_EXPORTWIZARDFIELDSDLG_H__8F80373F_F71C_4097_A6E1_0CF1F9A249CB__INCLUDED_)
#define AFX_EXPORTWIZARDFIELDSDLG_H__8F80373F_F71C_4097_A6E1_0CF1F9A249CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportWizardFieldsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFieldsDlg dialog
#include "ExportUtils.h"

class CExportWizardFieldsDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CExportWizardFieldsDlg)

// Construction
public:
	CExportWizardFieldsDlg();
	~CExportWizardFieldsDlg();

// Dialog Data
	//{{AFX_DATA(CExportWizardFieldsDlg)
	enum { IDD = IDD_EXPORT_WIZARD_FIELDS };
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnAdd;
	CNxEdit	m_nxeditPlaceholderText;
	CNxEdit	m_nxeditFillChar;
	CNxEdit	m_nxeditPlaces;
	CNxEdit	m_nxeditDecimalChar;
	CNxEdit	m_nxeditFixedWidthPlaces;
	CNxEdit	m_nxeditCustomDateFormat;
	CNxEdit	m_nxeditCurrencySymbolText;
	CNxEdit	m_nxeditCustomPhoneFormat;
	CNxEdit	m_nxeditTrueText;
	CNxEdit	m_nxeditFalseText;
	CNxEdit	m_nxeditUnknownText;
	CNxEdit	m_nxeditAdvancedField;
	CNxStatic	m_nxstaticPlaceholderTextLabel;
	CNxStatic	m_nxstaticFillCharLabel;
	CNxStatic	m_nxstaticDecimalLabel;
	CNxStatic	m_nxstaticPlacesLabel;
	CNxStatic	m_nxstaticDecimalCharLabel;
	CNxStatic	m_nxstaticSampleOutput;
	CNxStatic	m_nxstaticFixedWidthPlacesLabel;
	CNxStatic	m_nxstaticPlaceholderLabel;
	CNxStatic	m_nxstaticCustomDateLabel;
	CNxStatic	m_nxstaticCurrencySymbolLabel;
	CNxStatic	m_nxstaticCurrencyPlacementLabel;
	CNxStatic	m_nxstaticCustomPhoneLabel;
	CNxStatic	m_nxstaticTrueLabel;
	CNxStatic	m_nxstaticFalseLabel;
	CNxStatic	m_nxstaticUnknownLabel;
	CNxStatic	m_nxstaticAdvancedLabel;
	NxButton	m_btnFormatGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExportWizardFieldsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pAvailFields, m_pSelectFields, m_pDateOptions, m_pCurrencyPlacement, m_pPhoneOptions;

	void UpdateFormat(); //Updates the current row's format based on the controls on screen.
	void UpdateSampleOutput(); //Updates what's showing in the "sample output" label.

	int m_nBasedOn; //Tracks what we think the export is based on, so if it changes we can clear everything out.

	bool IsValidField(SelectedExportField sef);

	void LoadActiveFields();

	// Generated message map functions
	//{{AFX_MSG(CExportWizardFieldsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddExportField();
	afx_msg void OnRemoveExportField();
	afx_msg void OnExportFieldDown();
	afx_msg void OnExportFieldUp();
	afx_msg void OnSelChangedSelectedExportFields(long nNewSel);
	afx_msg void OnDblClickCellAvailableExportFields(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedExportFields(long nRowIndex, short nColIndex);
	afx_msg void OnSelChangedAvailableExportFields(long nNewSel);
	afx_msg void OnChangePlaceholderText();
	afx_msg void OnCapitalizeText();
	afx_msg void OnChangeFillChar();
	afx_msg void OnRightJustify();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	afx_msg void OnSetfocusFillChar();
	afx_msg void OnDecimalFloating();
	afx_msg void OnDecimalFixed();
	afx_msg void OnChangeDecimalChar();
	afx_msg void OnChangePlaces();
	afx_msg void OnChangeFixedWidthPlaces();
	afx_msg void OnNegativeParens();
	afx_msg void OnSelChosenDateOptions(long nRow);
	afx_msg void OnChangeCustomDateFormat();
	afx_msg void OnSelChosenCurrencyPlacement(long nRow);
	afx_msg void OnChangeCurrencySymbol();
	afx_msg void OnChangeCustomPhoneFormat();
	afx_msg void OnSelChosenPhoneFormatOptions(long nRow);
	afx_msg void OnSsnIncludeHyphens();
	afx_msg void OnChangeUnknownText();
	afx_msg void OnChangeTrueText();
	afx_msg void OnChangeFalseText();
	afx_msg void OnChangeAdvancedField();
	afx_msg void OnShowAllItems();
	afx_msg void OnSentenceFormat();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTWIZARDFIELDSDLG_H__8F80373F_F71C_4097_A6E1_0CF1F9A249CB__INCLUDED_)
