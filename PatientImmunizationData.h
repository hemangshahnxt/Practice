// (d.thompson 2013-07-16) - PLID 57579 - Refactored immunization data
#pragma once

#include <NxHL7Lib/HL7DataUtils.h>

class CPatientImmunizationData
{
public:
	CPatientImmunizationData(void);
	~CPatientImmunizationData(void);

	//Functionality
	void LoadFromData(long nImmunizationID);
	bool SaveToData(const CPatientImmunizationData& dataPreviousForAudit, CString &strErrorMessage) const;
	bool IsValidObject(CString &strErrorMessage) const;
	bool IsNew() const;


	//Data fields - not audited
	long m_nImmunizationID;			//PatientImmunizationsT.ID
	long m_nPersonID;
	CString m_strPersonName;		//For auditing

	//Data fields - audited directly
	CString m_strDosage;
	CString m_strDosageUnits;
	CString m_strLotNumber;
	CString m_strManufacturer;
	CString m_strManufacturerCode;
	CString m_strReaction;
	CString m_strAdminNotes;
	_variant_t m_varAdminNotesCode;
	_variant_t m_varDateAdministered;
	_variant_t m_varExpDate;
	// (d.singleton 2013-10-21 17:30) - PLID 59133 - need check boxes for IIS consent and Immunization Refused
	_variant_t m_varRefused;

	// (a.walling 2013-11-11 15:00) - PLID 59414 - Immunizations - Vaccine Information Statements
	CString m_xmlVIS;

	std::vector<Nx::HL7::VIS> GetVIS() const
	{ return Nx::HL7::XmlToVIS(m_xmlVIS); }
	void SetVIS(const std::vector<Nx::HL7::VIS>& vis)
	{ m_xmlVIS = Nx::HL7::VISToXML(vis); }

	//Data field setters/getters.  Use these to ensure the fields are kept in sync
	void SetType(long nID, CString strTypeName);
	void SetRoute(_variant_t varID, CString strRoute);
	void SetSite(_variant_t varID, CString strSite);
	
	// (d.singleton 2013-08-23 15:25) - PLID 58274 - add ordering provider to the immunization dlg
	void SetOrderingProvider(const _variant_t& varID, const CString& strName)
	{
		m_varOrderingProviderID = varID;
		m_strOrderingProviderName = strName;
	}
	const _variant_t& GetOrderingProvider() const { return m_varOrderingProviderID; }

	// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
	void SetAdministeringProvider(const _variant_t& varID, const CString& strName)
	{
		m_varAdministeringProviderID = varID;
		m_strAdministeringProviderName = strName;
	}
	const _variant_t& GetAdministeringProvider() const { return m_varAdministeringProviderID; }

	long GetTypeID() const { return m_nTypeID; }
	_variant_t GetRoute() const { return m_varRouteID; }
	_variant_t GetSite() const { return m_varSiteID; }
	// (d.singleton 2013-10-01 17:35) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.  need to export this to hl7 as well. 
	// (a.walling 2013-11-11 09:59) - PLID 58843 - Using pre-defined hl7 subset for snomed immunity codes; also using variant for the code directly rather than an ID
	void SetSnomedImmunity(_variant_t varCode, const CString& strName, const CString& strDescription);
	const _variant_t& GetSnomedImmunityCode() const { return m_varSnomedImmunityCode; }
	// (d.singleton 2013-10-22 17:25) - PLID 59133 - need checkbox for  IIS consent and dropdown for Immunization Refused
	void SetRefusalReason(_variant_t varCode, CString strName);
	const _variant_t& GetRefusalReasonCode() const { return m_varRefusedReasonCode; }

	// (a.walling 2013-11-11 11:16) - PLID 58781 - immunization financial class / VFC eligibility
	void SetFinancialClass(const _variant_t& varFinancialClass, const CString& strFinancialClassName)
	{ 
		m_varFinancialClass = varFinancialClass;
		m_strFinancialClassName = strFinancialClassName;
	}
	const _variant_t& GetFinancialClass() const { return m_varFinancialClass; }

protected:
	//Data fields - audits compare the ID but audit the text name (pairs).  Use setter functions to ensure they are always 
	//	loaded properly.
	long m_nTypeID;					//This is required (-1 for new) and thus not a variant
	CString m_strTypeName;			//Auditing only
	_variant_t m_varRouteID;
	CString m_strRouteName;			//Auditing only
	_variant_t m_varSiteID;
	CString m_strSiteName;			//Auditing only
	// (d.singleton 2013-08-23 15:25) - PLID 58274 - add ordering provider to the immunization dlg
	_variant_t m_varOrderingProviderID;
	CString m_strOrderingProviderName;
	// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
	_variant_t m_varAdministeringProviderID;
	CString m_strAdministeringProviderName;
	// (d.singleton 2013-10-01 17:35) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.  need to export this to hl7 as well. 
	_variant_t m_varSnomedImmunityCode;
	CString m_strSnomedImmunityName;
	// (d.singleton 2013-10-22 16:58) - PLID 59133 - need checkbox for  IIS consent and dropdown for Immunization Refused
	_variant_t m_varRefusedReasonCode;
	CString m_strRefusedReasonName;
	// (a.walling 2013-11-11 11:16) - PLID 58781 - immunization financial class / VFC eligibility
	_variant_t m_varFinancialClass;
	CString m_strFinancialClassName;

protected:
	void Audit(const CPatientImmunizationData& dataPreviousForAudit) const;
	void AuditIndividualField(long nAuditID, AuditEventItems aei, _variant_t varOldID, CString strOldName, _variant_t varNewID, CString strNewName) const;
	void AuditIndividualFieldString(long nAuditID, AuditEventItems aei, CString strOld, CString strNew) const;
	void AuditIndividualFieldDate(long nAuditID, AuditEventItems aei, _variant_t varOld, _variant_t varNew) const;
};
