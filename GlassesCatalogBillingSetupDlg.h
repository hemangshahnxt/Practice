#pragma once


// CGlassesCatalogBillingSetupDlg dialog

class CGlassesCatalogBillingSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CGlassesCatalogBillingSetupDlg)

public:
	CGlassesCatalogBillingSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CGlassesCatalogBillingSetupDlg();

// Dialog Data
	enum { IDD = IDD_GLASSES_CATALOG_BILLING_SETUP };

protected:
	CNxIconButton m_nxbOK, m_nxbCancel;
	NXDATALIST2Lib::_DNxDataListPtr m_pDesignList, m_pMaterialList, m_pTreatmentList;
	//TES 5/20/2011 - PLID 43698 - Either m_pDesignList, m_pMaterialList or m_pTreatmentList, depending on which radio button is selected.
	
	NXDATALIST2Lib::_DNxDataListPtr m_pCptList;
	CNxStatic m_nxsBillingSetupLabel;
	NxButton m_nxbDesigns, m_nxbMaterials, m_nxbTreatments;

	
	CMap<long,long&,CString,CString&> m_mapCptIDToAuditValue;
// (s.dhole 2012-03-06 11:41) - PLID 48638 
	
	enum GlassesListType {
		lstDesign = 0,
		lstTreatment = 1,
		lstMaterial = 2,
	};
	//TES 5/20/2011 - PLID 43698 - This is the same code for all three lists, so I put it in this function.

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg BOOL OnInitDialog();
	virtual void OnOK();
	void OnMultiSelectCPT(LPDISPATCH lpRow,GlassesListType typ);
	BOOL IsServiceCodeChange(CString  strCptIds,  CString  strSavedCptIds);
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnDesigns();
	afx_msg void OnMaterials();
	afx_msg void OnTreatments();
	
	void LeftClickDesignList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickTreatmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickMaterialList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	
};
