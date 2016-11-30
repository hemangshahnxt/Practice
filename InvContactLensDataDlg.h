#pragma once
#include "InventoryRc.h"
// (s.dhole 2012-03-13 15:31) - PLID 48856 New Dialog
// CInvContactLensDataDlg dialog

class CInvContactLensDataDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvContactLensDataDlg)

public:
	CInvContactLensDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvContactLensDataDlg();

// Dialog Data
	enum { IDD = IDD_INV_CONTACT_LENS_DATADLG };
	
	long m_nProductID;
	CString  m_strFinalName;
	CString  GetContactLensName();
	void LoadData();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_pLensType ,m_pLensManufacturer;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnOK;
	CNxIconButton	m_btnPickCategory;
	CNxIconButton	m_btnRemoveCategory;
	CNxIconButton	m_btnEditType;
	CNxEdit m_nxeditName;
	CNxEdit m_nxeditCategory;
	
	CNxEdit m_nxeditProduct;
	CNxEdit m_nxeditQtyPerBox;
	CNxEdit m_nxeditTint;
	// (s.dhole 2012-03-19 15:52) - PLID 48973
	CNxIconButton	m_nxbtneditManufacturer;
	long m_nContactLensType, m_nContactLensManufacturerID;
	long SaveNewContactLensData();
	void SaveContactLensData();

	// (j.jones 2015-03-03 16:22) - PLID 65110 - products can now have multiple categories
	std::vector<long> m_aryCategoryIDs;
	long m_nDefaultCategoryID;

	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnKillfocusContactLensProduct();
	afx_msg void OnEnKillfocusContactLensQtyPerPack();
	afx_msg void OnEnKillfocusFramesCircumference();
	afx_msg void OnEnKillfocusFramesFrontPrice();
	afx_msg void OnCategoryPicker();
	afx_msg void OnCategoryRemove();
	afx_msg void OnBnClickedBtnContactLensType();
	afx_msg void OnBnClickedBtnContactLensManufacturer();
	DECLARE_EVENTSINK_MAP()
	void SelChosenDdContactLensType(LPDISPATCH lpRow);
	void SelChangingDdContactLensType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RequeryFinishedDdContactLensType(short nFlags);
	void SelChosenDdContactLensManufacturer(LPDISPATCH lpRow);
	void SelChangingDdContactLensManufacturer(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RequeryFinishedDdContactLensManufacturer(short nFlags);
	
	afx_msg void OnEnKillfocusContactLensTint();
};
