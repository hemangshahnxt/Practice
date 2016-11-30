#if !defined(AFX_CONFIGEMRPREVIEWDLG_H__F43F2800_EEC4_4650_8020_E64B46B9F48A__INCLUDED_)
#define AFX_CONFIGEMRPREVIEWDLG_H__F43F2800_EEC4_4650_8020_E64B46B9F48A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigEMRPreviewDlg.h : header file
//

// (a.walling 2007-07-13 11:14) - PLID 26640 - Dialog to configure info to show on the more info part of the preview

/////////////////////////////////////////////////////////////////////////////
// CConfigEMRPreviewDlg dialog

// (a.walling 2008-10-15 10:23) - PLID 31404 - Structure for header/footer customizations
struct CHeaderFooterInfo {
	void Load();
	void Save();

	bool operator==(const CHeaderFooterInfo& hfi) const;

	CString strHeaderLeft;
	CString strHeaderRight;

	CString strFooterLeft;
	CString strFooterRight;

	CString strHeader2Left;
	CString strHeader2Right;

	CString strFooter2Left;
	CString strFooter2Right;
};

// (a.walling 2008-10-15 10:24) - PLID 31404 - Available fields for customization
struct CPreviewFields {
	struct CField {
		CField(long nNewID, CString strNewName) : nID(nNewID), strFieldName(strNewName) {};

		long nID;
		CString strFieldName;
		CString strOutput;
	};

	enum EFields {
		fFirstMLast = 0,
		fFirstMiddleLast,
		fID,
		fFirst,
		fMiddle,
		fMiddleInitial,
		fLast,
		fBirthDate,
		fAge,
		fGenderMaleFemale,
		fGenderMF,
		fInsuranceCompany,
		fEMNDate,
		fEMNLocation,
		fEMNDescription,
		fEMRDescription,
		fPrintDate,
		// (r.gonet 09-17-2010) - PLID 38968 - Add print_time and print_date_time field types
		fPrintTime,
		fPrintDateTime,
		fPage,
		fPageTotal,
		fEnumCount,
	};

	// (a.walling 2010-03-09 14:05) - PLID 36740 - Moved to cpp
	CPreviewFields();

	~CPreviewFields();

	static CString GetFieldCode(const CString& strFieldDescription);

	static long scm_IDOffset;
	CArray<CField*, CField*> arFields;
};

// (a.walling 2008-10-15 10:25) - PLID 31404 - CNxEditWithFields controls
// this allows us to override the context menu
class CNxEditWithFields : public CNxEdit
{
// Construction

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxEditWithFields)
	//}}AFX_VIRTUAL

protected:
	

	//{{AFX_MSG(CNxEditWithFields)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CConfigEMRPreviewDlg : public CNxDialog
{
// Construction
public:
	// (a.walling 2008-10-14 10:50) - PLID 31678 - CWnd parent rather than CView
	CConfigEMRPreviewDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2008-10-15 10:24) - PLID 31404 - Show help for customization and formatting
	void ShowHeaderFooterFormattingHelp();

	inline long GetOptions() {
		return m_nOptions;
	}

	// (a.walling 2007-12-17 13:42) - PLID 28354
	inline long GetDetailHideOptions() {
		return m_nDetailHideOptions;
	}

	// (b.savon 2012-05-23 16:37) - PLID 48092
	inline long GetMedAllergyOptions() {
		return m_nMedAllergyOptions;
	}

	// (a.walling 2007-12-17 13:55) - PLID 28354
	BOOL m_bMoreInfoChanged;
	BOOL m_bEMNGlobalDataChanged;

// Dialog Data
	// (a.walling 2008-10-15 10:25) - PLID 31404 - CNxEditWithFields controls
	// (a.walling 2009-08-03 10:46) - PLID 34542 - Toggle for detail dash prefix
	// (a.walling 2010-03-24 20:26) - PLID 29293 - Option for overriding the font
	// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
	//{{AFX_DATA(CConfigEMRPreviewDlg)
	enum { IDD = IDD_CONFIGURE_PREVIEW_DLG };
	NxButton	m_btnEnableNarrativeLinks;
	NxButton	m_btnOverrideFont;
	NxButton	m_btnPrefixDash;
	NxButton	m_btnDisplayLocationLogo;

	// (a.walling 2010-11-11 11:51) - PLID 40848
	NxButton	m_btnDisplayAsTopics; 
	NxButton	m_btnShowEmpty;

	NxButton	m_btnHideEmptyTopicsPrint;
	NxButton	m_btnHideEmptyTopics;
	NxButton	m_btnHideOnNarrativePrint;
	NxButton	m_btnHideOnNarrative;
	NxButton	m_btnHideEmptyOnPrint;
	NxButton	m_btnHideEmpty;
	NxButton	m_btnMedicatiosn;
	NxButton	m_btnDiagCodes;
	NxButton	m_btnServiceCodes;
	NxButton	m_btnSecProviders;
	NxButton	m_btnTechnicians;	// (d.lange 2011-04-25 15:49) - PLID 43380 - Added checkbox for Assistant/Technicians
	NxButton	m_btnProviders;
	NxButton	m_btnProcedures;
	NxButton	m_btnNotes;

	// (b.savon 2012-05-23 15:51) - PLID 48092 - Added Medications and Allergies
	NxButton	m_btnMedications;
	NxButton	m_btnMedDiscontinued;
	NxButton	m_btnAllergies;

	// (b.eyers 2016-02-23) - PLID 68322 - added asc fields
	NxButton	m_btnASCTimes;
	NxButton	m_btnASCStatus;

	// (b.savon 2012-05-23 15:50) - PLID 48092 - Redesigned the dialog
	CNxColor	m_nxcHeader;
	CNxColor	m_nxcMoreInfo;
	CNxColor	m_nxcVisibility;
	CNxColor	m_nxcMedicationsAllergies;
	CNxColor	m_nxcMisc;

	CNxColor	m_color;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnHelp;
	NxButton	m_btnEmrPreviewGroupboxHeaderFooter;
	NxButton	m_btnImagesUseFileSize;

	CNxEditWithFields		m_editHeaderLeft;
	CNxEditWithFields		m_editHeaderRight;
	CNxEditWithFields		m_editHeader2Left;
	CNxEditWithFields		m_editHeader2Right;
	CNxEditWithFields		m_editFooterLeft;
	CNxEditWithFields		m_editFooterRight;
	CNxEditWithFields		m_editFooter2Left;
	CNxEditWithFields		m_editFooter2Right;

	//}}AFX_DATA

	enum EHeaderFooterColumns {
		eLeft = 0,
		eRight = 1,
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigEMRPreviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long GetNewOptions();
	long m_nOptions;

	// (b.savon 2012-05-23 16:35) - PLID 48092
	long GetNewMedAllergyOptions();
	long m_nMedAllergyOptions;

	// (a.walling 2007-12-17 13:42) - PLID 28354
	long GetNewDetailHideOptions();
	long m_nDetailHideOptions;
	// (a.walling 2008-07-01 16:57) - PLID 30586
	long m_nLogoOption;

	// (a.walling 2009-08-03 10:46) - PLID 34542 - Toggle for detail dash prefix
	BOOL m_bPrefixDash;

	// (a.walling 2010-03-24 20:26) - PLID 29293 - Option for overriding the font
	BOOL m_bOverrideFont;
	
	// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
	BOOL m_bEnableNarrativeLinks;

	//TES 1/25/2012 - PLID 47505 - Setting to use the original file size when sizing image details
	BOOL m_bImagesUseFileSize;

	// (a.walling 2008-10-14 11:27) - PLID 31404
	CHeaderFooterInfo m_HeaderFooterInfo;

	COLORREF m_bkgColor;

	// Generated message map functions
	//{{AFX_MSG(CConfigEMRPreviewDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnHeadfoothelp();
	afx_msg void OnBnClickedCheckDisplaymeds();
	afx_msg void OnBnClickedCheckDismeds();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGEMRPREVIEWDLG_H__F43F2800_EEC4_4650_8020_E64B46B9F48A__INCLUDED_)
