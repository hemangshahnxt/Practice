#pragma once


// CEditImmunizationDlg dialog
// (d.thompson 2009-05-12) - PLID 34232 - Created
#include "PatientsRc.h"
#include "PatientImmunizationData.h"

class CEditImmunizationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditImmunizationDlg)

public:
	CEditImmunizationDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditImmunizationDlg();

	//ID of the record we are editing, -1 if new
	long m_nID;
	//PersonID of the record we are editing or adding newly to
	long m_nPersonID;
	// (d.thompson 2013-07-16) - PLID 57513 - For auditing
	CString m_strPersonName;

// Dialog Data
	enum { IDD = IDD_EDIT_IMMUNIZATION_DLG };

protected:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxEdit m_nxeditDosage;
	CNxEdit m_nxeditLotNumber;
	CNxEdit m_nxeditManufacturer;
	CNxEdit m_nxeditReaction;
	CNxEdit m_nxeditDosageUnits; // (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
	CNxLabel m_nxlabelUnits; // (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRouteList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSiteList;
	NXDATALIST2Lib::_DNxDataListPtr m_pManufacturerList; // (a.walling 2010-09-13 08:19) - PLID 40497
	// (d.singleton 2013-08-23 15:37) - PLID 58274 - add ordering provider and location to the immunization dlg
	NXDATALIST2Lib::_DNxDataListPtr m_pOrderingProviderList;
	// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
	NXDATALIST2Lib::_DNxDataListPtr m_pAdministeringProviderList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAdminNotes;
	// (d.singleton 2013-10-01 16:45) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
	NXDATALIST2Lib::_DNxDataListPtr m_pEvidenceImmunity;
	// (a.walling 2013-11-11 11:33) - PLID 58781 - immunization financial class / VFC eligibility
	NXDATALIST2Lib::_DNxDataListPtr m_pFinancialClass;

	// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
	NXDATALIST2Lib::_DNxDataListPtr m_pVISList;
	void LoadImmTypes();

	NXTIMELib::_DNxTimePtr m_pDateAdministered;
	NXTIMELib::_DNxTimePtr m_pExpDate;

	// (d.thompson 2013-07-16) - PLID 57579 - Refactor immunization data.  This object represents the last saved data 
	//	set.  For a new immunization, it will be empty.  When loading an existing immunization, it will be the data loaded.
	CPatientImmunizationData m_dataLastSaved;

	// (d.singleton 2013-07-11 12:30) - PLID 57515 - Immunizations need to track an "Administrative Notes"
	CNxEdit m_nxeditAdminNotes;

	// (d.singleton 2013-10-21 17:30) - PLID 59133 - need check boxes for IIS consent and Immunization Refused
	NxButton m_nxbIISConsent;
	NXDATALIST2Lib::_DNxDataListPtr m_pRefusalReason;	

	// (d.thompson 2013-07-16) - PLID 57579
	void ReflectDataToUI();
	CPatientImmunizationData GetImmunizationDataFromUI();

	// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
	bool CheckVISData();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEditImmType();
	afx_msg void OnBnClickedEditImmRoute();
	afx_msg void OnBnClickedEditImmSite();
	afx_msg void OnSelChosenTypeList(LPDISPATCH lpRow);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (a.walling 2010-09-13 14:35) - PLID 40505 - Support dosage units for vaccinations
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (a.walling 2010-09-13 14:35) - PLID 40505 - Support dosage units for vaccinations
	afx_msg void OnBnClickedAddVIS(); // (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
public:
	void SelChosenManufacturerList(LPDISPATCH lpRow);
	void SelChosenAdministrativeNotesList(LPDISPATCH lpRow);
};
