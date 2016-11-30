#pragma once
#include "InventoryRc.h"
// (s.dhole 2011-03-14 18:10) - PLID 42795 New Catalog setup Dialog
// Most of the code I move from VisionWebCatalogSetup 
// CInvGlassesCatalogSetupDlg dialog

class CInvGlassesCatalogSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvGlassesCatalogSetupDlg)
	
	
public:
	CInvGlassesCatalogSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvGlassesCatalogSetupDlg();
	CString  m_strSupplier;
	long m_nSupplierID;
	//TES 2/23/2011 - PLID 40539
	CDWordArray m_dwaCurrentOrderTreatments;
// Dialog Data
	enum { IDD = IDD_GLASSES_CATALOG_SETUP };

protected:

	virtual BOOL OnInitDialog();
	CNxIconButton	m_btnNextSupplier;
	CNxIconButton	m_btnPrevSupplier;
	CNxIconButton	m_btnAddDesigns;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAddFrameType;
	CNxIconButton	m_btnAddMaterials;
	CNxIconButton	m_btnAddTreatments;
	CNxIconButton	m_btnVisionWebSync;
	CNxIconButton	m_btnBillingSetup;
	CNxIconButton	m_btnCopySetup;	// (j.dinatale 2013-03-26 10:08) - PLID 53425
	void UpdateSupplierArrows();
	
	void CInvGlassesCatalogSetupDlg::ReLoadAllDataList();
	void RemoveFrameType();
	void RemoveDesigns();
	void RemoveMaterials();
	void RemoveTreatments();
	void RemoveLocation();
	void GenerateMenu(int MenuTypeID, CPoint &pt);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//(c.copits 2011-10-04) PLID 45112 - Clicking Glasses Catalog with an inactive supplier generates errors
	void UpdateSupplierButtons();

	NXDATALIST2Lib::_DNxDataListPtr m_SelectedDesignsList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedFrameTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedMaterialsList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedTreatmentsList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedSupplierCombo;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnSupplierPrevious();
	afx_msg void OnSupplierNext();
	afx_msg void OnBtnAddFrameType();
	afx_msg void OnBtnAddDesigns();
	afx_msg void OnBtnAddMaterials();
	afx_msg void OnBtnAddTreatments();
	afx_msg void OnBtnVisionWebSync();
	afx_msg void OnBnClickedCancel( );

	afx_msg void OnRButtonDownFrameTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownDesignsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownMaterialsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownTreatmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChangingSelectedSupplierList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenSelectedSupplierList(LPDISPATCH lpRow);


	//afx_msg void OnRButtonDownSupplierLocation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

public:
	afx_msg void OnBillingSetup();
	afx_msg void OnBnClickedButtonCopySetup();	// (j.dinatale 2013-03-26 10:08) - PLID 53425
};
