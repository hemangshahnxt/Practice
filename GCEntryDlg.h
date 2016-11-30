#if !defined(AFX_GCENTRYDLG_H__B79A65C9_1339_45C3_BBA1_8C777EF5E27B__INCLUDED_)
#define AFX_GCENTRYDLG_H__B79A65C9_1339_45C3_BBA1_8C777EF5E27B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GCEntryDlg.h : header file
//

class CGCEntryPresets;

// (r.gonet 2015-04-20) - PLID 65327 - Added an enumeration to track where this dialog is being called from. Different dialogs have different rules.
// (j.jones 2015-05-11 16:57) - PLID 65714 - tweaked this to be usage based, such that the enum defines what it will cause the dialog to do
enum class GCCreationStyle {
	eCreateNewGC = 0,			//The user is making a new GC first, and a bill needs created.
	eCreateNewGC_FromBill,		//The user added a GC to a bill, so we do not need to create one.
	eRefundToNewGC,				// (r.gonet 2015-04-20) - PLID 65327 - the GC is being created & credited from a refund
	eTransferToNewGC,			// (j.jones 2015-05-11 17:02) - PLID 65714 - the GC is being created by a balance transfer from another GC
};

/////////////////////////////////////////////////////////////////////////////
// CGCEntryDlg dialog
class CGCEntryDlg : public CNxDialog
{
public:
// Construction

	// (j.jones 2015-05-11 16:57) - PLID 65714 - added GCCreationStyle as an optional parameter
	CGCEntryDlg(CWnd* pParent, GCCreationStyle eCreationStyle = GCCreationStyle::eCreateNewGC);   // standard constructor

	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the certificate number text field in the dialog.
	void SetCertNumber(CString strCertNumber);	
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the provider dropdown field in the dialog.
	void SetProvider(long nID);
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the location dropdown field in the dialog.
	void SetLocation(long nID);
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the purchased by patient dropdown field in the dialog.
	void SetPurchasedByPatientID(long nID);
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the received by patient dropdown field in the dialog.
	// If nID is not a valid patient ID, then the Received By will be set to Unknown.
	void SetReceivedByPatientID(long nID);
	void SetService(long nID);
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Sets the default value of the Value field in the dialog.
	// If SetPrice is not called as well, then the price will be filled by default with the preset Value
	// field value.
	void SetValue(COleCurrency cy);
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - If called, then the amount shown in the Value field
	// will use this rather than the amount set by SetValue. Should be called with SetValue because that
	// amount will be what is saved with the gift certificate to LineItemT.GCValue.
	void SetDisplayedValue(COleCurrency cy);
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Renamed Amount to Price.
	void SetPrice(COleCurrency cy);
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Renamed slightly for consistency.
	void SetPurchaseDate(COleDateTime dt);
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the expiraton date datetimepicker field.
	// If dt is g_cdtNull, then there will be no expiration date.
	void SetExpirationDate(COleDateTime dt);
	void ViewReport();

	//ID number for this gift cert.
	long m_nID;
	CString m_strCertNumber;

	//if we are editing, they can change a few of the fields.  Many fields are still 
	//	filled in from billing, and therefore cannot be changed from here.
	bool m_bEditing;

// Dialog Data
	// (a.walling 2008-05-22 16:54) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CGCEntryDlg)
	enum { IDD = IDD_GC_ENTRY_DLG };
	NxButton	m_btnPat;
	NxButton	m_btnUnknown;
	NxButton	m_btnExpDate;
	CDateTimePicker	m_dtPurchase;
	CDateTimePicker	m_dtExpire;
	CNxEdit	m_nxeditCertNumber;
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added an edit box for Value.
	CNxEdit	m_nxeditCertValue;
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Renamed Amount to Price.
	CNxEdit	m_nxeditCertPrice;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnsurePatInfo();
	void GenerateNewGCID();
	void AddNewPatient(NXDATALISTLib::_DNxDataListPtr pList);
	// (j.jones 2015-03-18 14:24) - PLID 64974 - Category and SubCategory are now nullable
	void GetCategoriesFromServiceID(long nServiceID, _variant_t &varCategoryID, _variant_t &varSubCategoryID);

	NXDATALISTLib::_DNxDataListPtr m_pPurchList;
	NXDATALISTLib::_DNxDataListPtr m_pRecList;
	NXDATALISTLib::_DNxDataListPtr m_pTypeList;
	NXDATALISTLib::_DNxDataListPtr m_pProvList;
	NXDATALISTLib::_DNxDataListPtr m_pLocationList;

	// If we are coming from the billing, we do not create a bill/charge, and we do
	//	not prompt for a payment.
	// (j.jones 2015-05-11 17:04) - PLID 65714 - added m_eCreationStyle to track the create type
	GCCreationStyle m_eCreationStyle;

	bool m_bShowingRec;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The certificate number to prefill the cert number field with when it loads.
	CString m_strPresetCertNumber;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The ID of the provider to preselect from the provider dropdown when it loads.
	long m_nPresetProviderID = -1;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The ID of the location to preselect from the location dropdown when it loads.
	long m_nPresetLocationID = -1;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The GCTypesT.ID (ServiceT.ID) of the gift certificate type to preselect from the type dropdown
	// when it loads.
	long m_nPresetServiceID = -1;
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added a preset for Value and renamed Amount to Price.
	// Whatever these store is loaded into the Gift Certificate Value and Price fields when the dialog is loaded,
	// though the only place where that is useful is from billing.
	COleCurrency m_cyPresetValue = g_ccyNull;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - If not null, then the amount shown in the Value field
	// will use this rather than the amount in g_cyPresetValue. Intended to be cosmetic only. Not saved
	// to LineItemT.GCValue. When set, then we try to save the m_cyPresetValue to LineItemT.GCValue.
	COleCurrency m_cyPresetDisplayedValue = g_ccyNull;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The amount to prefill the price field with when it loads.
	COleCurrency m_cyPresetPrice = g_ccyNull;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The datetime to prefill the Purchase Date field with when it loads.
	COleDateTime m_dtPresetPurchaseDate = g_cdtNull;
	bool m_bPurchaseDatePreset = false;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The datetime to prefill the Expiration Date field with when it loads.
	// If m_dtPresetExpirationDate is null, then the expiration date field will disable and there will be no expiration.
	COleDateTime m_dtPresetExpirationDate = g_cdtNull;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Whether the expiration date should be preset or not. Without this flag,
	// then a value of g_cdtNull for m_dtPresetExpirationDate would be ambiguous.
	bool m_bExpirationDatePreset = false;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The ID of the patient to prefill the Purchaser dropdown field with when it loads.
	long m_nPresetPurchasedByPatientID = -1;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - The ID of the patient to prefill the Received By patient field with when it loads.
	// If -1, then the field will be prefilled to Unknown.
	long m_nPresetReceivedByPatientID = -1;
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Flag to indicate whether the Received By patient should be filled at all.
	bool m_bReceivedByPatientIDPreset = false;
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Control flag to tell the dialog when to push the changes from the Value field
	// to the Price field. Needed because programmatic changes to the Value field call the edit box's Changed event handler as well.
	// Only turns on when focus is given to the Value field. Turns off when the Value field loses focus.
	bool m_bSyncPriceWithValue;
	// (r.gonet 2015-05-13 15:22) - PLID 65276 - Even if we would normally sync otherwise, we can flag the Price never to sync.
	bool m_bSyncPriceAllowed = true;

	// Generated message map functions
	//{{AFX_MSG(CGCEntryDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRecTypePat();
	afx_msg void OnCopyPurch();
	afx_msg void OnOpenGcTypes();
	afx_msg void OnSelChosenGcTypeList(long nRow);
	afx_msg void OnAddNewPurchaser();
	afx_msg void OnAddNewReceiver();
	afx_msg void OnRecTypeUnknown();
	afx_msg void OnExpDateCheck();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added a handler for when the Value field loses focus.
	afx_msg void OnEnKillfocusCertValueEdit();
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Renamed Price to Value
	afx_msg void OnKillfocusCertPrice();
	// (r.gonet 2015-07-06 18:02) - PLID 65327
	afx_msg void OnTrySetSelFinishedRecPatList(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added a handler for when the Value field obtains focus.
	afx_msg void OnEnSetfocusCertValueEdit();
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added a handler for when the Value field is changed.
	afx_msg void OnEnChangeCertValueEdit();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCENTRYDLG_H__B79A65C9_1339_45C3_BBA1_8C777EF5E27B__INCLUDED_)
