#pragma once
#include "afxwin.h"


// CEditResourceSetDlg dialog

// (a.walling 2010-06-15 15:49) - PLID 39184 - Configuration dialog for a resource set

class CEditResourceSetDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditResourceSetDlg)

public:
	CEditResourceSetDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditResourceSetDlg();

// Dialog Data
	enum { IDD = IDD_EDIT_RESOURCE_SET_DLG };

	int m_nResourceSetID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pResourceSetList;
	NXDATALIST2Lib::_DNxDataListPtr m_pResourceCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pResourceSetUsedOn;	

	void UpdateArrowButtons();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnMovedown();
	afx_msg void OnBnDoubleclickedBtnMovedown();
	afx_msg void OnBnClickedBtnMoveup();
	afx_msg void OnBnDoubleclickedBtnMoveup();
	DECLARE_EVENTSINK_MAP()
	void RButtonUpListResourceSet(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChosenListResources(LPDISPATCH lpRow);
protected:
	virtual void OnOK();
public:
	CNxIconButton m_btnMoveUp;
	CNxIconButton m_btnMoveDown;
	CNxEdit m_editName;
	CNxStatic m_nxsNameLabel;
	CNxStatic m_nxsUsedWithLabel;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnOK;
	CNxColor m_bkgColor;
	void RButtonDownListResourceSet(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownListUsedOn(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonUpListUsedOn(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void CurSelWasSetListResourceSet();
};
