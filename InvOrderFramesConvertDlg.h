#pragma once
#include "InventoryRc.h"
#include "FramesData.h"

// CInvOrderFramesConvertDlg dialog
// (b.spivey, November 22, 2011) - PLID 45265 - Created. 

enum eMarkUp{
	emuID = 0, 
	emuName, 
	emuFormula,
	emuRoundUp,
};

class CInvOrderFramesConvertDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvOrderFramesConvertDlg)

public:
	CInvOrderFramesConvertDlg(const long nSupplierID, const CString strSupplierName, const CString strBarcodeNum, CWnd* pParent);   // standard constructor
	virtual ~CInvOrderFramesConvertDlg();

// Dialog Data
	enum { IDD = IDD_INV_ORDER_ADD_PRODUCT };

	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

	long m_nProductID; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	long m_nSupplierID; 
	long m_nFramesID; 

	CString m_strBarcodeNum; 
	CString m_strProductInfo; 
	CString m_strSupplierName;
	CString m_strNoMarkUp; 

	CFramesData m_fdFramesData;

	NXDATALIST2Lib::_DNxDataListPtr m_pMarkUpSelect; 

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog(); 
	BOOL CreateProductFromFrame();
	
	afx_msg void OnBnClickedSelectFrame();
	DECLARE_EVENTSINK_MAP()
	void SelChangingMarkupSelect(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
