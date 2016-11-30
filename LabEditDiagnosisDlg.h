#pragma once


// CLabEditDiagnosisDlg dialog
// (z.manning 2008-10-27 09:36) - PLID 24630 - Created

class CLabEditDiagnosisDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabEditDiagnosisDlg)

public:
	CLabEditDiagnosisDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabEditDiagnosisDlg();

// Dialog Data
	enum { IDD = IDD_LABS_EDIT_DIAGNOSIS };

protected:
	
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDiagCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlLabDiagList;

	enum EDiagComboColumns
	{
		dccID = 0,
		dccCode,
		dccDescription,
	};

	// (b.spivey, February 22, 2016) - PLID 68241 - Added a new column. 
	enum ELabDiagListColumns
	{
		ldlcID = 0,
		ldlcDiag,
		ldlcDescription,
		ldlcResultComment,
	};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	CNxIconButton m_btnClose;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnEdit;
	CNxIconButton m_btnDelete;
	afx_msg void OnBnClickedAddLabDiag();
	afx_msg void OnBnClickedEditLabDiag();
	afx_msg void OnBnClickedDeleteLabDiag();
	DECLARE_EVENTSINK_MAP()
	void SelChosenSelectDiagCombo(LPDISPATCH lpRow);
	// (r.gonet 03/27/2014) - PLID 60776 - Restored function from an errant delete.
	void EditingFinishedLabDiagnosisList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void SelChangedLabDiagnosisList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void RequeryFinishedLabDiagnosisList(short nFlags);
public:
	afx_msg void OnBnClickedLinkDiagnosis();
};
