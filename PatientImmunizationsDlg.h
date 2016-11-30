#pragma once


// CPatientImmunizationsDlg dialog
// (d.thompson 2009-05-12) - PLID 34232 - Created
#include "PatientsRc.h"

class CPatientImmunizationsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientImmunizationsDlg)

public:
	CPatientImmunizationsDlg(CWnd* pParent);   // standard constructor
	virtual ~CPatientImmunizationsDlg();

	//PersonID that we are displaying for.  Required for use.
	long m_nPersonID;
	// (d.thompson 2013-07-16) - PLID 57513 - For auditing we need the patient name
	CString m_strPersonName;

// Dialog Data
	enum { IDD = IDD_PATIENT_IMMUNIZATIONS_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnModify;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnPreview;
	// (a.walling 2010-02-18 10:12) - PLID 37434
	CNxIconButton m_btnExport;
	// (d.singleton 2012-12-11 10:30) - PLID 54141
	CNxIconButton m_btnHL7ExportConfig;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	// (d.singleton 2013-06-05 14:39) - PLID 57057 - now need to track group id
	long m_nImmunizationHL7GroupID;
	
	// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
	NXDATALIST2Lib::_DNxDataListPtr m_pPublicity;
	CDateTimePicker	m_dtpPublicity;
	NXDATALIST2Lib::_DNxDataListPtr m_pRegistryStatus;
	CDateTimePicker	m_dtpRegistry;
	
	CDateTimePicker	m_dtpIISConsent;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedImmAdd();
	afx_msg void OnBnClickedImmModify();
	afx_msg void OnBnClickedImmDelete();
	afx_msg void OnDblClickCellList(LPDISPATCH lpRow, short nColIndex);
	// (d.singleton 2012-12-11 10:31) - PLID 54141
	afx_msg void OnBnClickedHL7ExportConfig();
	afx_msg void OnBnClickedPreviewReport();
	// (a.walling 2010-02-18 10:13) - PLID 37434
	afx_msg void OnBnClickedExportImmunizations();

	// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
	afx_msg void OnChangePublicityDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeRegistryDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeIISConsentDate(NMHDR* pNMHDR, LRESULT* pResult);
	
	void OnSelChosenPublicityCode(LPDISPATCH lpRow);
	void OnSelChosenRegistryCode(LPDISPATCH lpRow);
	afx_msg void OnClickIISConsent(); // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent

	// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
	struct ImmunizationInfo {
		_variant_t publicityCode;
		_variant_t publicityDate;
		_variant_t registryCode;
		_variant_t registryDate;
		_variant_t iisConsent; // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
		_variant_t iisConsentDate;

		ImmunizationInfo()
			: publicityCode(g_cvarNull)
			, publicityDate(g_cvarNull)
			, registryCode(g_cvarNull)
			, registryDate(g_cvarNull)
			, iisConsent(g_cvarNull) // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
			, iisConsentDate(g_cvarNull)
		{}

		friend bool operator==(const ImmunizationInfo& l, const ImmunizationInfo& r) {
			return l.publicityCode == r.publicityCode
				&& l.publicityDate == r.publicityDate
				&& l.registryCode == r.registryCode
				&& l.registryDate == r.registryDate
				&& l.iisConsent == r.iisConsent
				&& l.iisConsentDate == r.iisConsentDate; // (a.walling 2013-11-11 14:23) - PLID 59405 - IIS Consent
		}
	};

	ImmunizationInfo m_info;

	// (a.walling 2013-11-11 13:37) - PLID 59172 - Immunization publicity code, registry code, and effective dates for both.
	void ApplyInfo(const ImmunizationInfo& newInfo);
};
