#pragma once


#include "InventoryRc.h"
// CInvVisionWebLocationDlg dialog
// (s.dhole 2010-11-15 13:31) - PLID 42897 standalone interface to assign VisionWeb location and ID to specific supplier

class CInvVisionWebLocationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvVisionWebLocationDlg)

private:
	enum MenuType
	{
		menuNone=0,
		menuSupplierLocation,
	};

	
public:
	CInvVisionWebLocationDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvVisionWebLocationDlg();
// Dialog Data
	enum { IDD = IDD_INV_VISIONWEB_LOCATION_SETUP_DLG  };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAddLocation;
	CNxEdit			m_nxeditLocationCode;
	CNxStatic		m_nxstaticSupplierName;
	
protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	void RemoveLocation();
	void GenerateMenu(int MenuTypeID, CPoint &pt);
	NXDATALIST2Lib::_DNxDataListPtr m_LocationListCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierLocationList;
	
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBtnAddLocation();
	afx_msg void OnClose( );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDownSupplierLocation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedLocationComboList(short nFlags);
	afx_msg void SelChosenComboLocation(LPDISPATCH lpRow);
	afx_msg void EditingFinishingVisionwebSupplierLocationList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnBnClickedCancel();
public:
	CString  m_strSupplier;
	long m_nSupplierID;
};
