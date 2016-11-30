#pragma once

#include "MarkUpFormula.h" //r.wilson 3/9/2012 PLID 46664
// CInvFramesDlg dialog
// (z.manning 2010-06-17 15:46) - PLID 39222 - Created

class CInvFramesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvFramesDlg)

public:
	CInvFramesDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvFramesDlg();

	virtual BOOL OnInitDialog();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	enum { IDD = IDD_INV_FRAMES };
	CNxIconButton m_btnImportFramesData;
	CNxIconButton m_btnCreateProducts;
	CNxIconButton m_btnUpdateExistingProducts;
	// (j.gruber 2010-06-23 14:42) - PLID 39314
	CNxIconButton m_btnOptions;
	CNxStatic m_nxstaticFramesCount;
	CNxStatic m_nxstaticFramesDate;
	CNxIconButton m_btnReload;
	CNxIconButton m_btnEditMarkups;
	CNxIconButton m_btnApplyMarkups;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pdlFramesList;
	enum FramesListColumns {
		flcCheck = 0,
		flcFramesDataID,
		flcProductID,
		flcRowBackColor,
		flcMarkup, // (z.manning 2010-09-22 09:33) - PLID 40619
		flcBegingDynamicFramesDataCols,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlManufacturerFilter;
	enum ManufacturerComboColumns {
		mccName = 0,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlStyleFilter;
	enum StyleComboColumns {
		sccName = 0,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlCollectionFilter;
	enum CollectionComboColumns {
		cccName = 0,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlBrandFilter;
	enum BrandComboColumns {
		bccName = 0,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlGroupFilter;
	enum GroupComboColumns {
		gccName = 0,
	};

	const CString m_strFilterAllText;

	BOOL m_bDatalistFiltered; // (z.manning 2010-10-01 11:18) - PLID 40761

	void RequeryFramesList();
	void RequeryFilters();
	CString GetWhereClause();
	CString GetFilterFromClause(const CString strFieldName);

	// (z.manning 2010-06-23 10:43) - PLID 39311
	void SelectAllFramesRows(BOOL bSelect);

	// (z.manning 2010-06-23 10:43) - PLID 39311
	void CreateProductsFromFramesDataRows(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void CreateProductsFromFramesDataRows(CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypFramesDataRows);
	// (z.manning 2010-09-22 13:04) - PLID 40619
	void CommitNewProducts(const CString &strSqlBatch, OUT long &nNewProductID, OUT BOOL &bImportedAtLeastOneCategory);

	void UpdateImportVersionLabel();

	BOOL IsRowProduct(NXDATALIST2Lib::IRowSettingsPtr pRow);

	//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
	long CheckChangedPrices();
	void ShowPriceChangeReport();

	void PopulateFormulaMap(OUT CMap<long,long,CMarkUpFormula,CMarkUpFormula> &mapMarkupIDToFormula);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void SelChosenFramesComboManufacturer(LPDISPATCH lpRow);
	void SelChosenFramesComboCollection(LPDISPATCH lpRow);
	void SelChosenFramesComboStyle(LPDISPATCH lpRow);
	afx_msg void OnBnClickedFramesImportButton(); // (z.manning 2010-06-22 17:01) - PLID 39306
	afx_msg void OnBnClickedFramesSelectAll();
	afx_msg void OnBnClickedFramesSelectNone();
	afx_msg void OnBnClickedFramesCreateSelectedProducts();
	afx_msg void OnBnClickedFramesUpdateExistingProducts();
	afx_msg void OnBnClickedFramesOptions();
	void RequeryFinishedFramesComboManufacturer(short nFlags);
	void RequeryFinishedFramesComboBrand(short nFlags);
	void RequeryFinishedFramesComboCollection(short nFlags);
	void RequeryFinishedFramesComboGroup(short nFlags);
	void RequeryFinishedFramesComboStyle(short nFlags);
	void SelChosenFramesComboGroup(LPDISPATCH lpRow);
	void SelChosenFramesComboBrand(LPDISPATCH lpRow);
	void RequeryFinishedFramesCatalogList(short nFlags);
	void RButtonDownFramesCatalogList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishingFramesCatalogList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedFramesCatalogList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnBnClickedFramesReload();
	afx_msg void OnBnClickedPriceChangeReport();
	afx_msg void OnBnClickedFramesEditMarkups(); // (z.manning 2010-09-15 10:23) - PLID 40319
	afx_msg void OnBnClickedFramesApplyMarkup(); // (z.manning 2010-09-21 14:53) - PLID 40619
};
