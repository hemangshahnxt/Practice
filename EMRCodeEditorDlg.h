#pragma once


// CEMRCodeEditorDlg dialog
// (j.gruber 2013-10-01 13:41) - PLID 58675 - created for


class CEMRCodeEditorDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMRCodeEditorDlg)

public:
	CEMRCodeEditorDlg(CEMRCodeArray *pAry, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEMRCodeEditorDlg();

// Dialog Data
	enum { IDD = IDD_EMR_CODE_EDITOR_DLG };

	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadCodeList();

	NXDATALIST2Lib::_DNxDataListPtr m_pCodeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;

	CEMRCodeArray *m_paryCodes;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChangingCodeSelectedList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenCodeSelectedList(LPDISPATCH lpRow);
	void RButtonUpCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedOpenUmls();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
