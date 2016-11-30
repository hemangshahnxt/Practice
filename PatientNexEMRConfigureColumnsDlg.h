#if !defined(AFX_PATIENTNEXEMRCONFIGURECOLUMNS_H__4A85ED3C_77C8_4BD8_817D_DE2D240642CA__INCLUDED_)
#define AFX_PATIENTNEXEMRCONFIGURECOLUMNS_H__4A85ED3C_77C8_4BD8_817D_DE2D240642CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientNexEMRConfigureColumns.h : header file
//

class CPatientNexEMRDlg;

/////////////////////////////////////////////////////////////////////////////
// CPatientNexEMRConfigureColumnsDlg dialog
// (z.manning, 04/05/2007) - PLID 25518
class CPatientNexEMRConfigureColumnsDlg : public CNxDialog
{
// Construction
public:
	CPatientNexEMRConfigureColumnsDlg(CPatientNexEMRDlg* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPatientNexEMRConfigureColumnsDlg)
	enum { IDD = IDD_NEXEMR_CONFIGURE_COLUMNS };
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientNexEMRConfigureColumnsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pColumns;

	CStringArray m_arystrColumnNames;

	void Load();

	CPatientNexEMRDlg *m_pParentNexEMRDlg;

	// Generated message map functions
	//{{AFX_MSG(CPatientNexEMRConfigureColumnsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedColumnList(LPDISPATCH lpRow, short nCol, const _variant_t &varOldValue, const _variant_t &varNewValue, VARIANT_BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTNEXEMRCONFIGURECOLUMNS_H__4A85ED3C_77C8_4BD8_817D_DE2D240642CA__INCLUDED_)
