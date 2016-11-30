#if !defined(AFX_IMPORTWIZARDFIELDS_H__4804B482_7898_45BD_B07F_56D8819906FE__INCLUDED_)
#define AFX_IMPORTWIZARDFIELDS_H__4804B482_7898_45BD_B07F_56D8819906FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportWizardFields.h : header file
//
#include "ImportWizardDlg.h"
#include "ImportValidationUtils.h"
/////////////////////////////////////////////////////////////////////////////
// CImportWizardFieldsDlg dialog

enum FieldValidityType{
	fvtValid =0,
	fvtInvalid,
	fvtInvalidSize,
};
enum RowValidityType{
	rvtValid=0,
	rvtInvalid,
	rvtInvalidSize,
};
struct AppointmentData{
	int nColumnIndex = -1;
	CString strData = "";
};

// (b.savon 2015-04-28 15:19) - PLID 65485 - Derive from our CNxPropertyPage
class CImportWizardFieldsDlg : public CNxPropertyPage
{
	DECLARE_DYNCREATE(CImportWizardFieldsDlg)

// Construction
public:
	CImportWizardFieldsDlg();
	~CImportWizardFieldsDlg();

	BOOL m_bNeedsReset;
	BOOL m_bHasBeenResized;

	CMap<CString, LPCTSTR, long, long> m_mapPersonIDstoImportedUserDefined; // (r.farnworth 2015-03-23 11:49) - PLID 65246

// Dialog Data
	//{{AFX_DATA(CImportWizardFieldsDlg)
	enum { IDD = IDD_IMPORT_WIZARD_FIELDS };
	CNxIconButton	m_btnPrevField;
	CNxIconButton	m_btnNextField;
	CNxEdit	m_nxeditImportFieldHeader;
	CNxStatic m_nxstaticMappingLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CImportWizardFieldsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	BOOL   m_bNeedInit;
	CSize  m_ClientSize;
	// (r.farnworth 2015-03-23 11:04) - PLID 65246 - Added m_pPatientIDMapping
	NXDATALIST2Lib::_DNxDataListPtr m_pFieldList, m_pAvailFields, m_pPatientIDMapping;
	long m_nPreviewRecords;
	CStringArray m_aryPreviewRecords;
	CString m_strDelimiter;
	CString m_strTextQualifier;
	short m_nCurrentColumn;
	BOOL m_bCanContinue;
	CMap <long, long, CString, CString> m_mapHeaders;
	BOOL m_bFirstRowIsHeader;
	CMap<int, int, FieldValidityType, FieldValidityType> m_mapCellValidity;
	CMap<int, int, int, int> m_mapStrColumnMaxLength;
	BOOL m_bContainsTruncatedData;
	BOOL m_bContainsBlankNames; // (r.gonet 2010-09-01) - PLID 39733 - Whether or not the data contains a patient row w/ blank names
	bool m_bHasApptPurpose; // (r.goldschmidt 2016-02-02 16:23) - PLID 67974 
	bool m_bHasApptType; // (r.goldschmidt 2016-03-16 16:23) - PLID 67974 
	BOOL m_bTooManyFields; // (b.eyers 2016-06-01) - PLID-68613

	void LoadAvailableFields(ImportRecordType irtCurrent);
	void LoadPersonFields();
	void LoadPatientFields();
	void LoadProviderFields();
	void LoadRefPhysFields();
	void LoadUserFields();
	void LoadSupplierFields();
	void LoadOtherContactFields();
	// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
	void LoadServiceFields();
	void LoadCPTCodeFields();
	void LoadResourceFields(); // (r.farnworth 2015-03-16 14:44) - PLID 65197
	// (b.savon 2015-03-19 14:48) - PLID 65144 - Make location name its own function
	void LoadLocationFields();
	void LoadProductFields(); // (r.farnworth 2015-03-19 09:13) - PLID 65238
	void LoadInsuranceCoFields(); // (r.farnworth 2015-04-01 14:49) - PLID 65166
	// (b.savon 2015-03-30 17:24) - PLID 65233 - Recall fields
	void LoadRecallFields();
	void LoadPatientNoteFields(); //(s.dhole 4/2/2015 4:52 PM ) - PLID 65225 load patient not fields
	// (b.savon 2015-04-06 09:12) - PLID 65216 - Appointment fields
	void LoadAppointmentFields();
	//(s.dhole 4/13/2015 3:24 PM ) - PLID  65190 Patitn Insured Party
	void LoadInsuredPartiesFields();
	// (r.goldschmidt 2016-02-09 15:25) - PLID 68163
	void LoadRaceFields();
	// (v.maida 2015-04-22 10:41) - PLID 65667 - Checks if a patient mapping ID is valid.
	BOOL ValidatePatientMappingID(CCachedData &cache, CString& mappingID);
	// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Cache the map of mappingIDs to nPersonIDs
	BOOL ValidatePatientMappingRecords(CCachedData &cache, long nMapID, CString& mappingID, long& nPersonID);
	// (b.savon 2015-07-06 12:36) - PLID 66490
	void TryAutoMatchColumnToDataField();

	void LoadFile();
	void CreateColumns(ImportRecordType irtSelected);
	void ParseFileToInterface(CFile &fImportFile, BOOL bValidateFullFile = FALSE);
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
	RowValidityType IsValid(NXDATALIST2Lib::IRowSettingsPtr pRow, class CCachedData &cache);
	void InvalidateRow(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bWarningOnly = FALSE);
	void ValidateRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void SetMediNotesColumns();
	void HandleSelectedColumn(short nCol);
	void UpdateArrows();
	void UpdateColumnDataTypes();
	void HandleFieldHeaders();
	void AddColumnExtendedInfo(long nColumn, int nMaxLength);
	void RemoveColumnExtendedInfo(long nColumn);
	void FillMappingCombo(); // (r.farnworth 2015-03-23 11:41) - PLID 65246
	void ShowMappingCombo(ImportRecordType irtCurrent); // (r.farnworth 2015-03-23 11:41) - PLID 65246
	CSqlFragment  GetPatietnSQL(CCachedData &cache, NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (b.cardillo 2015-05-13 16:20) - PLID 66099 - Share a single FormatSettings object
	NXDATALIST2Lib::IFormatSettingsPtr m_pHyperLink;
	NXDATALIST2Lib::IFormatSettingsPtr GetHyperlinkFormatSettings();
	// Generated message map functions
	//{{AFX_MSG(CImportWizardFieldsDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	afx_msg void OnLeftClickFieldTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnImportFieldPrevious();
	afx_msg void OnImportFieldNext();
	afx_msg void OnSelChosenAvailField(LPDISPATCH lpRow);
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	afx_msg void OnImportFileHasHeader();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	//(s.dhole 4/29/2015 10:18 AM ) - PLID 65712 parse column value if it has textQualifier
	void GetTextQualifierColumnValue(CString &strLineText, long &nPos, long &nRow);
	long LastIndexOf(const CString& strWord, const CString& strSearch);
	// (r.goldschmidt 2016-02-02 12:44) - PLID 67976 - check if Conversion appointment type is valid
	bool ValidateApptTypeAsConversion();
public:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedApptTypesAsConversion1(); // (r.goldschmidt 2016-01-27 16:00) - PLID 67976
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTWIZARDFIELDS_H__4804B482_7898_45BD_B07F_56D8819906FE__INCLUDED_)
