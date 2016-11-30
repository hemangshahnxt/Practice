#pragma once


#include "InventoryRc.h"
// CInvVisionWebCatalogSetupDlg dialog
// (s.dhole 2010-11-15 13:31) - PLID  41743 VisionWeb Suppler Catalog Setup

class CInvVisionWebCatalogSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvVisionWebCatalogSetupDlg)

private:
	// Addedd menuNone as non selected item for popup menu
	enum MenuType
	{
		menuNone=0,
		menuFrameType,
		menuDesign,
		menuMaterial,
		menuTreatment,
		menuSupplierLocation,
	};

	
public:
	CInvVisionWebCatalogSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvVisionWebCatalogSetupDlg();
// Dialog Data
	enum { IDD = IDD_INV_VISIONWEB_CATALOG_SETUP  };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAddFrameType;
	CNxIconButton	m_btnAddDesigns;
	CNxIconButton	m_btnAddMaterials;
	CNxIconButton	m_btnAddTreatments;
	CNxIconButton	m_btnVisionWebSync;
	CNxIconButton	m_btnAddLocation;
	CNxEdit			m_nxeditLocationCode;
	CNxStatic		m_nxstaticSupplierName;
	
protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void RemoveFrameType();
	void RemoveDesigns();
	void RemoveMaterials();
	void RemoveTreatments();
	void RemoveLocation();
	void GenerateMenu(int MenuTypeID, CPoint &pt);

	NXDATALIST2Lib::_DNxDataListPtr m_SelectedFrameTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedDesignsList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedMaterialsList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedTreatmentsList;
	NXDATALIST2Lib::_DNxDataListPtr m_LocationListCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierLocationList;
	
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBtnAddFrameType();
	afx_msg void OnBtnAddDesigns();
	afx_msg void OnBtnAddMaterials();
	afx_msg void OnBtnAddTreatments();
	afx_msg void OnBtnVisionWebSync();
	afx_msg void OnBtnAddLocation();
	afx_msg void OnClose( );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDownFrameTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownDesignsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownMaterialsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownTreatmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownSupplierLocation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedLocationComboList(short nFlags);
	afx_msg void SelChosenComboLocation(LPDISPATCH lpRow);
	afx_msg void EditingFinishingVisionwebSupplierLocationList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	//afx_msg void RequeryFinishedVisionwebSupplierLocationList(short nFlags);
public:
	CString  m_strSupplier;
	long m_nSupplierID;
	afx_msg void OnBnClickedCancel();
	//TES 2/23/2011 - PLID 40539
	CDWordArray m_dwaCurrentOrderTreatments;
};
