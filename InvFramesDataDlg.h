#pragma once


// CInvFramesDataDlg dialog
// (z.manning 2010-06-21 09:59) - PLID 39257 - Created

class CInvFramesDataDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvFramesDataDlg)

public:
	// (j.gruber 2010-06-23 15:27) - PLID 39323 - set editable on construction
	// (s.dhole 2012-04-30 11:11) - PLID 466662 Added bIsNewFrame
	CInvFramesDataDlg(const long nProductID, const  BOOL bIsNewFrame, BOOL bEditable, CWnd* pParent);   // standard constructor
	virtual ~CInvFramesDataDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_INV_FRAMES_DATA };
	CNxIconButton m_btnClose;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxEdit m_nxeditStyle;
	CNxEdit m_nxeditColor;
	CNxEdit m_nxeditColorCode;
	CNxEdit m_nxeditLensColor;
	CNxEdit m_nxeditEye;
	CNxEdit m_nxeditBridge;
	CNxEdit m_nxeditTemple;
	CNxEdit m_nxeditDBL;
	CNxEdit m_nxeditA;
	CNxEdit m_nxeditB;
	CNxEdit m_nxeditED;
	CNxEdit m_nxeditCircumference;
	CNxEdit m_nxeditEDAngle;
	CNxEdit m_nxeditFrontPrice;
	CNxEdit m_nxeditHalfTemplesPrice;
	CNxEdit m_nxeditTemplesPrice;
	CNxEdit m_nxeditCompletePrice;
	CNxEdit m_nxeditManufacturer;
	CNxEdit m_nxeditBrand;
	CNxEdit m_nxeditCollection;
	CNxEdit m_nxeditGender;
	CNxEdit m_nxeditAgeGroup;
	CNxEdit m_nxeditActiveStatus;
	CNxEdit m_nxeditProductGroupName;
	CNxEdit m_nxeditRimType;
	CNxEdit m_nxeditMaterial;
	CNxEdit m_nxeditFrameShape;
	CNxEdit m_nxeditCountry;
	CNxEdit m_nxeditSKU;
	CNxEdit m_nxeditYearIntroduced;	
	long m_nProductID;
	// (s.dhole 2012-04-30 10:19) - PLID 46662 
	BOOL  m_bIsNewFrame;
	CString  m_strFinalProductName;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
	// (j.gruber 2010-06-23 15:27) - PLID 39323
	BOOL m_bIsEditable;
	void SetControls();
	void Save();

	long SaveNewFrame();
	CString GetFrameName();
	CString GetFrameDataInsertSql(IN BOOL bIsCatalog ,const CString strNewFrameDataID);
	
	void AddCreateFrameProductSqlToBatch(IN OUT CString &strSqlBatch, const CString strProductID, CString strName, BOOL bBillable, BOOL bTaxable1, BOOL bTaxable2, long nUserID);

	void Load();
	CNxIconButton	m_btnPickCategory;
	CNxIconButton	m_btnRemoveCategory;
	
	// (j.jones 2015-03-03 16:39) - PLID 65111 - products can now have multiple categories
	std::vector<long> m_aryCategoryIDs;
	long m_nDefaultCategoryID;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedClose();
	
	afx_msg void OnEnKillfocusFramesFrontPrice();
	afx_msg void OnEnKillfocusFramesHalfTemplesPrice();
	afx_msg void OnEnKillfocusFramesTemplesPrice();
	afx_msg void OnEnKillfocusFramesCompletePrice();
	//s.dhole 2012-06-06 PLID 46662  change as style 
	afx_msg void OnEnKillfocusFramesStyle();
	afx_msg void OnEnKillfocusFramesCollection();
	afx_msg void OnEnKillfocusFramesColor();
	afx_msg void OnEnKillfocusFramesColorCode();
	afx_msg void OnEnKillfocusFramesEye();
	afx_msg void OnEnKillfocusFramesBridge();
	afx_msg void OnEnKillfocusFramesTemple();
	afx_msg void OnCategoryPicker();
	afx_msg void OnCategoryRemove();
};
