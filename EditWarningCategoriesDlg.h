#pragma once


// CEditWarningCategoriesDlg dialog

// (a.walling 2010-07-02 15:54) - PLID 39514 - Dialog to add and modify warning categories.
// It is inevitable that this will eventually be expanded when we start implementing multiple
// warnings for a patient.

class CEditWarningCategoriesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditWarningCategoriesDlg)

public:
	CEditWarningCategoriesDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditWarningCategoriesDlg();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_EDIT_WARNING_CATEGORIES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	CNxColor m_bkgColor;

	void UpdateButtons();
	void RemoveWarningCategoryByRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	CList<int> m_listRemovedCategories;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonMovecatup();
	afx_msg void OnBnDoubleclickedButtonMovecatup();
	afx_msg void OnBnClickedButtonMovecatdown();
	afx_msg void OnBnDoubleclickedButtonMovecatdown();
	afx_msg void OnBnClickedButtonChooseColor();
	afx_msg void OnBnClickedButtonAddWarningCategory();
	afx_msg void OnBnClickedButtonRemoveWarningCategory();
	afx_msg void OnBnClickedOk();

	DECLARE_EVENTSINK_MAP()
	void CurSelWasSetListWarningCategories();
	void DblClickCellListWarningCategories(LPDISPATCH lpRow, short nColIndex);
	void EditingFinishingListWarningCategories(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void RButtonDownListWarningCategories(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonUpListWarningCategories(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
