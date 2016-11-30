// (d.thompson 2013-07-16) - PLID 57579 - Refactored immunization data
#include "stdafx.h"
#include "PatientImmunizationData.h"
#include "NxAdo.h"
#include "WellnessDataUtils.h"
#include "AuditTrail.h"
#include <NxPracticeSharedLib/NxXMLUtils.h>
using namespace ADODB;

#define NEW_IMMUNIZATION_ID -1

CPatientImmunizationData::CPatientImmunizationData(void) :
	m_nImmunizationID(NEW_IMMUNIZATION_ID),
	m_nTypeID(-1), 
	m_varRouteID(g_cvarNull),
	m_varSiteID(g_cvarNull),
	m_varExpDate(g_cvarNull),
	m_varDateAdministered(g_cvarNull),
	m_varAdminNotesCode(g_cvarNull),
	m_varOrderingProviderID(g_cvarNull),
	m_varAdministeringProviderID(g_cvarNull),
	m_varSnomedImmunityCode(g_cvarNull),
	m_varRefused(g_cvarNull),
	m_varRefusedReasonCode(g_cvarNull),
	m_strRefusedReasonName(""),
	m_varFinancialClass(g_cvarNull) // (a.walling 2013-11-11 11:16) - PLID 58781 - immunization financial class / VFC eligibility
{
}

CPatientImmunizationData::~CPatientImmunizationData(void)
{
}

void CPatientImmunizationData::SetType(long nID, CString strTypeName)
{
	m_nTypeID = nID;
	m_strTypeName = strTypeName;
}

void CPatientImmunizationData::SetRoute(_variant_t varID, CString strRoute)
{
	m_varRouteID = varID;
	m_strRouteName = strRoute;
}

void CPatientImmunizationData::SetSite(_variant_t varID, CString strSite)
{
	m_varSiteID = varID;
	m_strSiteName = strSite;
}

// (d.singleton 2013-10-01 17:35) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
// (a.walling 2013-11-11 09:59) - PLID 58843 - Using pre-defined hl7 subset for snomed immunity codes; also using variant for the code directly rather than an ID
void CPatientImmunizationData::SetSnomedImmunity(_variant_t varCode, const CString& strName, const CString& strDescription)
{
	m_varSnomedImmunityCode = varCode;
	m_strSnomedImmunityName = AsString(varCode);
	if (!strName.IsEmpty()) {
		if (!m_strSnomedImmunityName.IsEmpty()) {
			m_strSnomedImmunityName += " - ";
		}
		m_strSnomedImmunityName += strName;
	}
	if (!strDescription.IsEmpty()) {
		if (!m_strSnomedImmunityName.IsEmpty()) {
			m_strSnomedImmunityName += " - ";
		}
		m_strSnomedImmunityName += strDescription;
	}
}

// (d.singleton 2013-10-22 17:29) - PLID 59133 - need checkbox for  IIS consent and dropdown for Immunization Refused
void CPatientImmunizationData::SetRefusalReason(_variant_t varCode, CString strName)
{
	m_varRefusedReasonCode = varCode;
	m_strRefusedReasonName = strName;
}

void CPatientImmunizationData::LoadFromData(long nImmunizationID)
{
	*this = CPatientImmunizationData();

	//Assign the ID
	m_nImmunizationID = nImmunizationID;

	//Short circuit:  Don't bother querying data for new records
	if(m_nImmunizationID == NEW_IMMUNIZATION_ID)
		return;

	{
		// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
		_RecordsetPtr prs = CreateParamRecordset("SELECT PatientImmunizationsT.*, "
			"ImmunizationsT.Type AS TypeName, "
			"ImmunizationRoutesT.Name AS RouteName, ImmunizationSitesT.Name AS SiteName, "
			"OrderingProviderT.FullName AS OrderingProviderName, AdministeringProviderT.FullName AS AdministeringProviderName, "
			"SnomedImmunityCodesT.Name as SnomedImmunityName, SnomedImmunityCodesT.Description as SnomedImmunityDesc, "
			"ImmunizationFinancialClassT.Name AS FinancialClassCodeName "
			"FROM PatientImmunizationsT "
			"LEFT JOIN ImmunizationsT ON PatientImmunizationsT.ImmunizationID = ImmunizationsT.ID "
			"LEFT JOIN ImmunizationRoutesT ON PatientImmunizationsT.RouteID = ImmunizationRoutesT.ID "
			"LEFT JOIN ImmunizationSitesT ON PatientImmunizationsT.SiteID = ImmunizationSitesT.ID "
			"LEFT JOIN PersonT OrderingProviderT ON PatientImmunizationsT.OrderingProviderID = OrderingProviderT.ID "
			"LEFT JOIN PersonT AdministeringProviderT ON PatientImmunizationsT.AdministeringProviderID = AdministeringProviderT.ID "
			"LEFT JOIN ImmunizationFinancialClassT ON PatientImmunizationsT.FinancialClassCode = ImmunizationFinancialClassT.Code "
			"LEFT JOIN SnomedImmunityCodesT ON SnomedImmunityCodesT.Code = PatientImmunizationsT.SnomedImmunityCode "
			"WHERE PatientImmunizationsT.ID = {INT};", m_nImmunizationID);
		if(!prs->eof) {
			SetType(AdoFldLong(prs, "ImmunizationID"), AdoFldString(prs, "TypeName", ""));
			SetRoute(prs->Fields->Item["RouteID"]->Value, AdoFldString(prs, "RouteName", ""));
			SetSite(prs->Fields->Item["SiteID"]->Value, AdoFldString(prs, "SiteName", ""));
			m_strDosage = AdoFldString(prs, "Dosage");
			m_strDosageUnits = AdoFldString(prs, "DosageUnits"); // (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
			m_strLotNumber = AdoFldString(prs, "LotNumber");
			m_strManufacturer = AdoFldString(prs, "Manufacturer");
			m_strManufacturerCode = AdoFldString(prs, "ManufacturerCode"); // (a.walling 2010-09-13 08:20) - PLID 40497
			m_varDateAdministered = prs->Fields->Item["DateAdministered"]->Value;
			m_varExpDate = prs->Fields->Item["ExpirationDate"]->Value;
			//(e.lally 2010-01-04) PLID 35768 - Load Reaction field
			m_strReaction = AdoFldString(prs, "Reaction");
			// (d.singleton 2013-07-11 12:33) - PLID 57515 - Immunizations need to track an "Administrative Notes"
			m_strAdminNotes = AdoFldString(prs, "AdministrativeNotes", "");
			SetOrderingProvider(prs->Fields->Item["OrderingProviderID"]->Value, AdoFldString(prs, "OrderingProviderName", "<None>"));
			// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
			SetAdministeringProvider(prs->Fields->Item["AdministeringProviderID"]->Value, AdoFldString(prs, "AdministeringProviderName", "<None>"));
			m_varAdminNotesCode = AdoFldVar(prs, "AdministrativeNotesCode");
			// (d.singleton 2013-10-02 15:56) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
			SetSnomedImmunity(AdoFldVar(prs, "SnomedImmunityCode"), AdoFldString(prs, "SnomedImmunityName", "<None>"), AdoFldString(prs, "SnomedImmunityDesc", ""));
			// (d.singleton 2013-10-21 17:54) - PLID 59133 - need check boxes for IIS consent and Immunization Refused
			m_varRefusedReasonCode = prs->Fields->Item["RefusalReasonCode"]->Value;
			// (a.walling 2013-11-11 11:16) - PLID 58781 - immunization financial class / VFC eligibility
			SetFinancialClass(AdoFldVar(prs, "FinancialClassCode"), AdoFldString(prs, "FinancialClassCodeName", "<None>"));
			// (a.walling 2013-11-11 15:00) - PLID 59414 - Immunizations - Vaccine Information Statements
			m_xmlVIS = AdoFldString(prs, "VIS", "");
		}
	}
}

// (d.thompson 2013-07-16) - PLID 57579 - Saves the object to the database.  No changes are made to the structure as a result
//	of the save (including the ID).
// (d.thompson 2013-07-16) - PLID 57513 - Include a copy of the old data so we can audit.
bool CPatientImmunizationData::SaveToData(const CPatientImmunizationData& dataPreviousForAudit, CString &strErrorMessage) const
{
	//First, do we have a valid object?
	if(!IsValidObject(strErrorMessage)) {
		return false;
	}

	//
	//All is well, let us save
	if(IsNew()) {
		//(e.lally 2010-01-04) PLID 35768 - Added Reaction field
		// (a.walling 2010-09-13 08:19) - PLID 40497 - Added ManufacturerCode
		// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
		// (d.singleton 2013-07-11 12:33) - PLID 57515 - Immunizations need to track an "Administrative Notes"
		// (d.singleton 2013-10-21 18:01) - PLID 59133 - need check boxes for Immunization Refused
		// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
		ExecuteParamSql("INSERT INTO PatientImmunizationsT (PersonID, ImmunizationID, Dosage, DosageUnits, DateAdministered, LotNumber, "
			"ExpirationDate, Manufacturer, ManufacturerCode, RouteID, SiteID, CreatedUserID, Reaction, AdministrativeNotes, OrderingProviderID, AdministeringProviderID, AdministrativeNotesCode, "
			"SnomedImmunityCode, RefusalReasonCode, FinancialClassCode, VIS) values "
			"({INT}, {INT}, {STRING}, {STRING}, {VT_DATE}, {STRING}, "
			"{VT_DATE}, {STRING}, {STRING}, {VT_I4}, {VT_I4}, {INT}, {STRING}, {STRING}, {VT_I4}, {VT_I4}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {STR});",
			m_nPersonID, m_nTypeID, m_strDosage, m_strDosageUnits, m_varDateAdministered, m_strLotNumber, 
			m_varExpDate, m_strManufacturer, m_strManufacturerCode, m_varRouteID, m_varSiteID, GetCurrentUserID(), 
			m_strReaction, m_strAdminNotes, m_varOrderingProviderID, m_varAdministeringProviderID, m_varAdminNotesCode, m_varSnomedImmunityCode, m_varRefusedReasonCode,
			m_varFinancialClass, m_xmlVIS); // (a.walling 2013-11-11 15:00) - PLID 59414 - Immunizations - Vaccine Information Statements
		
		// (b.cardillo 2009-06-07 23:01) - PLID 34506 - Auto-complete any wellness completion items for this 
		// patient that were waiting for this kind of immunization being created.
		//UpdateWellnessCompletion_Immunization(GetRemoteData(), m_nPersonID, nTypeID);

		// (b.cardillo 2009-06-08 17:57) - PLID 34511 - A patient immunization was just created, update 
		// patient qualifications that might be based on it.
		UpdatePatientWellness_Immunization(GetRemoteData(), m_nPersonID, m_nTypeID, GetCurrentUserID());

	}
	else {
		// (b.cardillo 2009-06-08 18:12) - PLID 34511 - Get the type of the immunization we're updating, in case we're changing it
		long nOldTypeID;
		{
			//(e.lally 2010-01-04) PLID 35768 - Added Reaction field
			// (a.walling 2010-09-13 08:19) - PLID 40497 - Added ManufacturerCode
			// (d.singleton 2013-07-11 12:33) - PLID 57515 - Immunizations need to track an "Administrative Notes"
			// (d.singleton 2013-10-02 14:52) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
			// (d.singleton 2013-10-21 18:01) - PLID 59133 - need check boxes for IIS consent and Immunization Refused
			// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"DECLARE @nOldTypeID INT \r\n"
				"SET @nOldTypeID = (SELECT ImmunizationID FROM PatientImmunizationsT WHERE ID = {INT}) \r\n"
				" \r\n"
				"UPDATE PatientImmunizationsT SET ImmunizationID = {INT}, Dosage = {STRING}, DosageUnits = {STRING}, DateAdministered = {VT_DATE}, "
				"LotNumber = {STRING}, ExpirationDate = {VT_DATE}, Manufacturer = {STRING}, ManufacturerCode = {STRING}, "
				"RouteID = {VT_I4}, SiteID = {VT_I4}, "
				"Reaction = {STRING}, "
				"AdministrativeNotes = {STRING}, AdministrativeNotesCode = {VT_BSTR}, "
				"OrderingProviderID = {VT_I4}, AdministeringProviderID = {VT_I4}, SnomedImmunityCode = {VT_BSTR}, RefusalReasonCode = {VT_BSTR}, "
				"FinancialClassCode = {VT_BSTR}, VIS = {STR} " // (a.walling 2013-11-11 15:00) - PLID 59414 - Immunizations - Vaccine Information Statements
				"WHERE ID = {INT}; \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nOldTypeID As OldTypeID WHERE @nOldTypeID IS NOT NULL \r\n"
				, 
				m_nImmunizationID, m_nTypeID, m_strDosage, m_strDosageUnits, m_varDateAdministered, m_strLotNumber, 
				m_varExpDate, m_strManufacturer, m_strManufacturerCode, m_varRouteID, m_varSiteID, m_strReaction, m_strAdminNotes, m_varAdminNotesCode, 
				m_varOrderingProviderID, m_varAdministeringProviderID, m_varSnomedImmunityCode, m_varRefusedReasonCode, m_varFinancialClass, m_xmlVIS,
				m_nImmunizationID );
			if (!prs->eof) {
				nOldTypeID = AdoFldLong(prs->GetFields()->GetItem("OldTypeID"));
			} else {
				nOldTypeID = -1;
			}
			prs->Close();
		}

		// (b.cardillo 2009-06-08 17:57) - PLID 34511 - A patient immunization was just changed, so we 
		// at least need to update patient qualifications that might be based on it.
		//TES 7/8/2009 - PLID 34534 - This function now updates both qualifications and completion items.
		UpdatePatientWellness_Immunization(GetRemoteData(), m_nPersonID, m_nTypeID, GetCurrentUserID());
		
		// And also the patient qualifications that might be based on the prior type
		if (nOldTypeID != -1 && nOldTypeID != m_nTypeID) {
			//TES 7/8/2009 - PLID 34534 - This function now updates both qualifications and completion items.
			UpdatePatientWellness_Immunization(GetRemoteData(), m_nPersonID, nOldTypeID, GetCurrentUserID());
		}
	}

	// (d.thompson 2013-07-16) - PLID 57513 - Regardless of save method, we need to audit every field.
	Audit(dataPreviousForAudit);

	return true;
}

bool CPatientImmunizationData::IsValidObject(CString &strErrorMessage) const
{
	bool bRetVal = true;

	// (a.walling 2010-09-13 08:19) - PLID 40497
	// (d.thompson 2013-07-16) - PLID 57579 - Moved from UI check.  This previously checked the UI
	//	datalist for a selection, I now look for a non-empty string.  At this point in time, we
	//	define the manufacturers and there are no empty ones, so this is equivalent.
	if(m_strManufacturerCode.IsEmpty()) {
		strErrorMessage += " - You must select a manufacturer to save.\r\n";
		bRetVal = false;
	}

	//Must have an immunization type
	if(m_nTypeID == -1) {
		strErrorMessage += " - You must select an immunization type to save.\r\n";
		bRetVal = false;
	}

	//Must have a Date Administered
	if(m_varDateAdministered.vt != VT_DATE) {
		strErrorMessage += " - You must enter a date the immunization was administered to save.\r\n";
		bRetVal = false;
	}

	strErrorMessage.TrimRight();

	return bRetVal;
}

bool CPatientImmunizationData::IsNew() const
{
	return m_nImmunizationID == NEW_IMMUNIZATION_ID ? true : false;
}

// (d.thompson 2013-07-16) - PLID 57513 - Audit all the fields if they changed (or all of them that are not empty
//	if new).
void CPatientImmunizationData::Audit(const CPatientImmunizationData& dataPreviousForAudit) const
{
	//Audit all as a batch
	long nAuditID = BeginAuditTransaction();
	AuditIndividualField(nAuditID, aeiPatientImmType, dataPreviousForAudit.m_nTypeID, dataPreviousForAudit.m_strTypeName, m_nTypeID, m_strTypeName);
	AuditIndividualFieldString(nAuditID, aeiPatientImmDosage, dataPreviousForAudit.m_strDosage, m_strDosage);
	AuditIndividualFieldString(nAuditID, aeiPatientImmDosageUnits, dataPreviousForAudit.m_strDosageUnits, m_strDosageUnits);
	AuditIndividualFieldDate(nAuditID, aeiPatientImmDateAdministered, dataPreviousForAudit.m_varDateAdministered, m_varDateAdministered);
	AuditIndividualField(nAuditID, aeiPatientImmRoute, dataPreviousForAudit.m_varRouteID, dataPreviousForAudit.m_strRouteName, m_varRouteID, m_strRouteName);
	AuditIndividualField(nAuditID, aeiPatientImmSite, dataPreviousForAudit.m_varSiteID, dataPreviousForAudit.m_strSiteName, m_varSiteID, m_strSiteName);
	AuditIndividualFieldString(nAuditID, aeiPatientImmLotNum, dataPreviousForAudit.m_strLotNumber, m_strLotNumber);
	AuditIndividualFieldDate(nAuditID, aeiPatientImmExpDate, dataPreviousForAudit.m_varExpDate, m_varExpDate);
	AuditIndividualFieldString(nAuditID, aeiPatientImmManufacturer, dataPreviousForAudit.m_strManufacturer, m_strManufacturer);
	AuditIndividualFieldString(nAuditID, aeiPatientImmManufacturerCode, dataPreviousForAudit.m_strManufacturerCode, m_strManufacturerCode);
	AuditIndividualFieldString(nAuditID, aeiPatientImmAdverseReaction, dataPreviousForAudit.m_strReaction, m_strReaction);
	AuditIndividualFieldString(nAuditID, aeiPatientImmAdminNotes, dataPreviousForAudit.m_strAdminNotes, m_strAdminNotes);
	// (d.singleton 2013-08-23 15:25) - PLID 58274 - add ordering provider to the immunization dlg
	AuditIndividualFieldString(nAuditID, aeiPatientImmOrderingProvider, dataPreviousForAudit.m_strOrderingProviderName, m_strOrderingProviderName);
	// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
	AuditIndividualFieldString(nAuditID, aeiPatientImmOrderingProvider, dataPreviousForAudit.m_strAdministeringProviderName, m_strAdministeringProviderName);
	// (d.singleton 2013-10-02 14:52) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.  need to export this to hl7 as well. 
	AuditIndividualFieldString(nAuditID, aeiPatientImmEvidenceImmunity, dataPreviousForAudit.m_strSnomedImmunityName, m_strSnomedImmunityName);
	// (a.walling 2013-11-11 11:16) - PLID 58781 - immunization financial class / VFC eligibility
	AuditIndividualFieldString(nAuditID, aeiPatientImmFinancialClass, dataPreviousForAudit.m_strFinancialClassName, m_strFinancialClassName);	
	CommitAuditTransaction(nAuditID);
}

// (d.thompson 2013-07-16) - PLID 57513 - Audit string fields using case sensitive comparison
void CPatientImmunizationData::AuditIndividualFieldString(long nAuditID, AuditEventItems aei, CString strOld, CString strNew) const
{
	if(strOld != strNew) {
		AuditEvent(m_nPersonID, m_strPersonName, nAuditID, aei, m_nImmunizationID, 
			strOld, strNew, aepMedium, aetChanged);
	}
}

// (d.thompson 2013-07-16) - PLID 57513 - Audit date fields
void CPatientImmunizationData::AuditIndividualFieldDate(long nAuditID, AuditEventItems aei, _variant_t varOld, _variant_t varNew) const
{
	if(varOld != varNew) {
		CString strOld, strNew;
		if(varOld.vt != VT_DATE)
			strOld = "No Date Selected";
		else
			strOld = FormatDateTimeForInterface(VarDateTime(varOld));
		if(varNew.vt != VT_DATE)
			strNew = "No Date Selected";
		else
			strNew = FormatDateTimeForInterface(VarDateTime(varNew));

		AuditEvent(m_nPersonID, m_strPersonName, nAuditID, aei, m_nImmunizationID, 
			strOld, strNew, aepMedium, aetChanged);
	}
}

// (d.thompson 2013-07-16) - PLID 57513 - Audit a 'field'.  Compare the identifiers, but audit the name that goes with that identifier
void CPatientImmunizationData::AuditIndividualField(long nAuditID, AuditEventItems aei, _variant_t varOldID, CString strOldName, _variant_t varNewID, CString strNewName) const
{
	if(varOldID != varNewID) {
		AuditEvent(m_nPersonID, m_strPersonName, nAuditID, aei, m_nImmunizationID, 
			strOldName, strNewName, aepMedium, aetChanged);
	}
}

