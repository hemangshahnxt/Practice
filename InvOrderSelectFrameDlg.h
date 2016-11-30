#pragma once
#include "InventoryRc.h"

// CInvOrderSelectFrameDlg dialog


enum eFramesSelect {
	efsID = 0, 
	efsManufacturer, 
	efsCollection, 
	efsColor, 
	efsStyle, 
	efsEye, 
	efsBridge, 
	efsTemple, 
	efsMaterial, 
	efsYear,
	efsWholesale, 
	};

class CInvOrderSelectFrameDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvOrderSelectFrameDlg)

public:
	CInvOrderSelectFrameDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvOrderSelectFrameDlg();

	long m_nFramesID;

// Dialog Data
	enum { IDD = IDD_INV_ORDER_SELECT_FRAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog(); 

	NXDATALIST2Lib::_DNxDataListPtr m_pManufacturerFilter; 
	NXDATALIST2Lib::_DNxDataListPtr m_pCollectionFilter; 
	NXDATALIST2Lib::_DNxDataListPtr m_pFramesSelect; 
	NXDATALIST2Lib::_DNxDataListPtr m_pBrandFilter; 

	CString m_strFrameSelectWhereClause; 
	CString m_strAllSelection; 
	CString m_strManufacture; 
	CString m_strCollection; 
	CString m_strBrand; 

	CNxIconButton m_btnOK; 
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnRefresh; 

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChangingManufactureFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingCollectionFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingBrandFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRefresh();
	void SelChangingFramesSelectList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
