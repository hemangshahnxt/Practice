#pragma once
// (j.gruber 2011-05-06 16:31) - PLID 43550 - created for

// CConfigureApptConvGroupsDlg dialog

class CConfigureApptConvGroupsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureApptConvGroupsDlg)

public:
	CConfigureApptConvGroupsDlg(CWnd* pParent);   // standard constructor
	virtual ~CConfigureApptConvGroupsDlg();

	CNxIconButton m_btnAdd;
	CNxIconButton m_btnRename;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnTypeLeft;
	CNxIconButton m_btnTypeRight;
	CNxIconButton m_btnCodeLeft;
	CNxIconButton m_btnCodeRight;
	CNxIconButton m_btnClose;

	NXDATALIST2Lib::_DNxDataListPtr m_pConvGroupList;
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeSelectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCodeAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCodeSelectedList;

	void LoadDialog(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void MoveCodeLeft(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void MoveCodeRight(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void MoveTypeLeft(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void MoveTypeRight(NXDATALIST2Lib::IRowSettingsPtr pRow);

// Dialog Data
	enum { IDD = IDD_CONFIGURE_APPT_GROUPS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAcgAdd();
	afx_msg void OnBnClickedAcgRename();
	afx_msg void OnBnClickedAcgDelete();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellApptTypeAvailList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedTypeMoveLeft();
	afx_msg void OnBnClickedTypeMoveRight();
	void DblClickCellApptTypeSelectedList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellCodeAvailList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedCodeMoveRight();
	afx_msg void OnBnClickedCodeMoveLeft();
	void DblClickCellCodeSelectList(LPDISPATCH lpRow, short nColIndex);	
	virtual void OnCancel();
	void SelChosenApptConvGroupList(LPDISPATCH lpRow);
	void RequeryFinishedApptConvGroupList(short nFlags);
	afx_msg void OnBnClickedAcgClose();
	afx_msg void OnEnKillfocusConversionDays();
};
