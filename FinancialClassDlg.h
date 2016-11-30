// (c.haag 2009-03-17 11:10) - PLID 9148 - Initial implementation
//
// This dialog lets users quickly assign multiple insurance companies
// to "financial classes", which are generic billing categories used
// for reporting purposes.
//
#pragma once

#include "PatientsRc.h"

// CFinancialClassDlg dialog

class CFinancialClassDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CFinancialClassDlg)

private:
	COLORREF m_clrStartup;
	BOOL m_bNeedToRequeryUnselectedList;

public:
	CFinancialClassDlg(CWnd* pParent);   // standard constructor
	virtual ~CFinancialClassDlg();

// Dialog Data
	enum { IDD = IDD_FINANCIAL_CLASS };

private:
	CNxIconButton m_btnClose;
	CNxIconButton m_btnNewClass;
	CNxIconButton m_btnRenameClass;
	CNxIconButton m_btnDeleteClass;

	CNxIconButton m_btnRemoveAll;
	CNxIconButton m_btnRemove;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnAddAll;

private:
	CNxColor		m_bkg;
	CNxColor		m_bkg2;

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dlFinClasses;
	NXDATALIST2Lib::_DNxDataListPtr m_dlUnselected;
	NXDATALIST2Lib::_DNxDataListPtr m_dlSelected;

public:
	void SetStartupColor(const COLORREF& clr);

private:
	void EnableControls(BOOL bEnable);
private:
	void LoadFirstAvailableFinancialClass();
private:
	BOOL Load(long nID);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNewClass();
	afx_msg void OnBnClickedRenameClass();
	afx_msg void OnBnClickedDeleteClass();
	afx_msg void OnBnClickedRemoveAllCompanies();
	afx_msg void OnBnClickedRemoveCompany();
	afx_msg void OnBnClickedAddCompany();
	afx_msg void OnBnClickedAddAllCompanies();
	DECLARE_EVENTSINK_MAP()
	void SelChangingComboFinclasses(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenComboFinclasses(LPDISPATCH lpRow);
	void DblClickCellFinclassUnselected(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellFinclassSelected(LPDISPATCH lpRow, short nColIndex);
};
