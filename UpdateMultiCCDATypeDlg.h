#pragma once

// (r.goldschmidt 2014-05-06 11:34) - PLID 61789 - Created
// CUpdateMultiCCDATypeDlg dialog

class CUpdateMultiCCDATypeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CUpdateMultiCCDATypeDlg)

public:
	CUpdateMultiCCDATypeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUpdateMultiCCDATypeDlg();

// Dialog Data
	enum { IDD = IDD_UPDATE_MULTI_CCDA_TYPE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxIconButton m_btnRemoveAll;
	CNxIconButton m_btnRemove;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	NXDATALIST2Lib::_DNxDataListPtr m_pUnselected;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelected;
	NXDATALIST2Lib::_DNxDataListPtr m_pCCDAType;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRemoveAll();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnBnClickedAdd();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex);
};
