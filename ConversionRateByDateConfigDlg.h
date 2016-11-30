#if !defined(AFX_CONVERSIONRATEBYDATECONFIGDLG_H__3EB3D154_4458_4F17_A128_D1E63EF42387__INCLUDED_)
#define AFX_CONVERSIONRATEBYDATECONFIGDLG_H__3EB3D154_4458_4F17_A128_D1E63EF42387__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConversionRateByDateConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConversionRateByDateConfigDlg dialog

class CConversionRateByDateConfigDlg : public CNxDialog
{
// Construction
public:
	CConversionRateByDateConfigDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pNewConsultList;
	NXDATALISTLib::_DNxDataListPtr m_pExistingConsultList;
	NXDATALISTLib::_DNxDataListPtr m_pAllConsultList;
	NXDATALISTLib::_DNxDataListPtr m_pSurgeryList;
	void CheckDataList(NXDATALISTLib::_DNxDataListPtr pDataList, CString strChecks);
	CString GenerateSaveString(NXDATALISTLib::_DNxDataListPtr pDataList);
	BOOL IsDataListChecked(NXDATALISTLib::_DNxDataListPtr pDataList);
	void Refresh();
	CBrush m_brush;

// Dialog Data
	//{{AFX_DATA(CConversionRateByDateConfigDlg)
	enum { IDD = IDD_CONFIGURE_CONVERSION_GRAPH };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditNewConsultLabel;
	CNxEdit	m_nxeditExistingConsultLabel;
	CNxEdit	m_nxeditSurgeryLabel;
	CNxEdit	m_nxeditAllConsultLabel;
	CNxStatic	m_nxstaticNewLabel;
	CNxStatic	m_nxstaticAllLabel;
	CNxStatic	m_nxstaticExistingLabel;
	CNxStatic	m_nxstaticCons1Label;
	CNxStatic	m_nxstaticConsLabel;
	CNxStatic	m_nxstaticCons2Label;
	CNxIconButton m_btnOK;
	NxButton	m_radioAllConsults;
	NxButton	m_radioSplitConsults;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConversionRateByDateConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConversionRateByDateConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSplitConsults();
	afx_msg void OnAllConsults();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEnKillfocusNewConsultLabel();
	afx_msg void OnEnKillfocusExistingConsultLabel();
	afx_msg void OnEnKillfocusAllConsultLabel();
	afx_msg void OnEnKillfocusSurgeryLabel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	//(e.lally 2009-09-14) PLID 35527
	BOOL m_bSplitConsults;
	CString m_strSplitConsultIDs;
	CString m_strSingleConsultIDs;
	CString m_strSplitConsultLabels;
	CString m_strSingleConsultLabel;
	CString m_strSurgeryIDs;
	CString m_strSurgeryLabel;

	void SaveConsultTypeLists();
	void SaveProcedureTypeList();
	void SaveConsultLabels();
	void SaveProcedureLabel();
	BOOL IsValidSetup(BOOL bWarnUser);
public:
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishedApttypeSurgeryList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnEditingFinishedApttypeNewConsultList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnEditingFinishedApttypeExistingConsultList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnEditingFinishedApttypeAllConsultList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERSIONRATEBYDATECONFIGDLG_H__3EB3D154_4458_4F17_A128_D1E63EF42387__INCLUDED_)
