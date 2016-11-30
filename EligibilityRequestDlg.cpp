// EligibilityRequestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilityRequestDlg.h"
#include "GlobalFinancialUtils.h"
#include "EEligibility.h"
#include "FinancialRc.h"
#include "DiagSearchUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-05-21 15:07) - PLID 8993 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (j.jones 2011-07-08 12:58) - PLID 38305 - store the default where clause as a define
#define PATIENT_COMBO_WHERE_CLAUSE "Archived = 0 AND PatientsT.PersonID <> -25 AND CurrentStatus <> 4 AND PatientsT.PersonID IN (SELECT PatientID FROM InsuredPartyT WHERE RespTypeID <> -1) "

// (j.jones 2011-07-08 13:00) - PLID 38305 - added an enum for the patient combo
enum EligibilityRequestPatientColumn {

	erpatcID = 0,
	erpatcUserDefinedID,
	erpatcLast,
	erpatcFirst,
	erpatcMiddle,
	erpatcFullName,
};

enum EligibilityRequestProviderColumn {

	erpcID = 0,
	erpcLast = 1,
	erpcFirst = 2,
	erpcMiddle = 3,
	erpcFullName = 4,
};

enum EligibilityRequestInsuranceColumn {

	ericID = 0,
	ericName = 1,
	ericRespID = 2,
	ericRespName = 3,
	ericAddress = 4,
};

enum EligibilityRequestCategoryColumn {

	erccID = 0,	// (j.jones 2013-03-28 15:18) - PLID 52182 - categories now have IDs
	erccCode,
	erccCategory,
};

enum EligibilityRequestModifierColumn {

	ermcModifier = 0,
	ermcDescription = 1,
};

// (j.jones 2014-02-27 15:47) - PLID 60767 - this is now a list with an index
enum EligibilityRequestDiagnosisColumn {

	erdcIndexNumber = 0,
	erdcCodeID,
	erdcCode,
	erdcDescription,
};

// (c.haag 2010-10-15 9:15) - PLID 40352
enum EligibilityRequestPOSColumn {
	erpID = 0,
	erpCode = 1,
	erpDescription = 2,
};

/////////////////////////////////////////////////////////////////////////////
// CEligibilityRequestDlg dialog


CEligibilityRequestDlg::CEligibilityRequestDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilityRequestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEligibilityRequestDlg)
		m_clrBackground = GetNxColor(GNC_FINANCIAL, 0);
		m_bNeedServiceRequery = TRUE;
		m_nID = -1;
		m_nDefaultInsuredPartyID = -1;
		m_nPendingPatientID = -1;
		m_nPendingInsuredPartyID = -1;
		m_nPendingProviderID = -1;
		m_nPendingLocationID = -1;
		m_nPendingServiceID = -1;
		m_strPendingModifier1 = "";
		m_strPendingModifier2 = "";
		m_strPendingModifier3 = "";
		m_strPendingModifier4 = "";
		m_nPendingPOSID = -1;
		m_bIsBatched = FALSE;
		m_nFormatID = -1;
		m_bModeless=FALSE;
	//}}AFX_DATA_INIT
}


void CEligibilityRequestDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEligibilityRequestDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_REQ_BENEFIT_CATEGORY, m_radioBenefitCategory);
	DDX_Control(pDX, IDC_RADIO_REQ_SERVICE_CODE, m_radioServiceCode);	
	DDX_Control(pDX, IDC_ELIG_REQUEST_BACKGROUND, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEligibilityRequestDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEligibilityRequestDlg)
	ON_BN_CLICKED(IDC_RADIO_REQ_BENEFIT_CATEGORY, OnRadioReqBenefitCategory)
	ON_BN_CLICKED(IDC_RADIO_REQ_SERVICE_CODE, OnRadioReqServiceCode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEligibilityRequestDlg message handlers

BOOL CEligibilityRequestDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (j.jones 2010-07-07 17:48) - PLID 39515 - added preference caching
		g_propManager.CachePropertiesInBulk("CEligibilityRequestDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'Eligibility_ExcludeFromOutputFile', "
			"	'GEDIEligibilityRealTime_Enabled', "
			// (j.jones 2016-05-19 10:27) - NX-100685 - cached FormatStyle
			"	'FormatStyle' "
			")"
			, _Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CEligibilityRequestDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefaultEligibilityBenefitCategory' " // (j.jones 2013-03-28 15:50) - PLID 52182 - added DefaultEligibilityBenefitCategory
			")"
			, _Q(GetCurrentUserName()));

		// (j.jones 2008-05-07 15:38) - PLID 29854 - added nxiconbuttons for modernization
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//colorize!
		m_bkg.SetColor(m_clrBackground);
		m_brush.CreateSolidBrush(m_clrBackground);
		
		m_PatientCombo = BindNxDataList2Ctrl(this,IDC_ELIGIBILITY_PATIENT_COMBO,GetRemoteData(), false);
		m_InsuredPartyCombo = BindNxDataList2Ctrl(this,IDC_ELIGIBILITY_INSURED_PARTY_COMBO,GetRemoteData(),false);
		m_ProviderCombo = BindNxDataList2Ctrl(this,IDC_ELIGIBILITY_PROVIDER_COMBO,GetRemoteData(),true);
		m_LocationCombo = BindNxDataList2Ctrl(this,IDC_ELIGIBILITY_LOCATION_COMBO,GetRemoteData(),true);
		m_CategoryCombo = BindNxDataList2Ctrl(this,IDC_ELIGIBILITY_CATEGORY_COMBO,GetRemoteData(),true);
		m_ServiceCombo = BindNxDataList2Ctrl(this,IDC_ELIGIBILITY_SERVICE_CODE_COMBO,GetRemoteData(),false);
		m_ModifierCombo1 = BindNxDataList2Ctrl(this,IDC_ELIG_MODIFIER_COMBO_1,GetRemoteData(),false);
		m_ModifierCombo2 = BindNxDataList2Ctrl(this,IDC_ELIG_MODIFIER_COMBO_2,GetRemoteData(),false);
		m_ModifierCombo3 = BindNxDataList2Ctrl(this,IDC_ELIG_MODIFIER_COMBO_3,GetRemoteData(),false);
		m_ModifierCombo4 = BindNxDataList2Ctrl(this,IDC_ELIG_MODIFIER_COMBO_4,GetRemoteData(),false);
		m_POSCombo = BindNxDataList2Ctrl(this, IDC_ELIG_POS_COMBO,GetRemoteData(),true);

		// (j.jones 2014-02-27 15:14) - PLID 60767 - added diag search and code list
		m_DiagSearchList = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_ELIG_DIAG_SEARCH_LIST, GetRemoteData());
		m_DiagCodeList = BindNxDataList2Ctrl(this, IDC_ELIG_DIAG_CODE_LIST, GetRemoteData(), false);
		//this query will not care if the diagnosis is inactive
		m_DiagCodeList->PutFromClause("("
			"SELECT EligibilityRequestsT.ID, 1 AS IndexNumber, "
			"DiagCodes.ID AS CodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"FROM EligibilityRequestsT "
			"LEFT JOIN DiagCodes ON EligibilityRequestsT.Diagnosis1 = DiagCodes.ID "
			""
			"UNION ALL "
			"SELECT EligibilityRequestsT.ID, 2 AS IndexNumber, "
			"DiagCodes.ID AS CodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"FROM EligibilityRequestsT "
			"LEFT JOIN DiagCodes ON EligibilityRequestsT.Diagnosis2 = DiagCodes.ID "
			""
			"UNION ALL "
			"SELECT EligibilityRequestsT.ID, 3 AS IndexNumber, "
			"DiagCodes.ID AS CodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"FROM EligibilityRequestsT "
			"LEFT JOIN DiagCodes ON EligibilityRequestsT.Diagnosis3 = DiagCodes.ID "
			""
			"UNION ALL "
			"SELECT EligibilityRequestsT.ID, 4 AS IndexNumber, "
			"DiagCodes.ID AS CodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc "
			"FROM EligibilityRequestsT "
			"LEFT JOIN DiagCodes ON EligibilityRequestsT.Diagnosis4 = DiagCodes.ID "
			") AS DiagsQ");

		//the category combo is built from scratch
		// (j.jones 2009-09-16 12:44) - PLID 26481 - this is now a global function
		// (j.jones 2013-03-28 15:25) - PLID 52182 - this is now in data, and thus obsolete
		//BuildEligibilityBenefitCategoryCombo(m_CategoryCombo, (long)erccCode, (long)erccCategory);
		
		// (c.haag 2010-10-15 9:15) - PLID 40352 - Place of Service type needs "none selected"
		IRowSettingsPtr pRow = m_POSCombo->GetNewRow();
		pRow->PutValue(erpID, (long)-1);
		pRow->PutValue(erpCode, _bstr_t(""));
		pRow->PutValue(erpDescription, _bstr_t(" <No Designation Selected>"));
		m_POSCombo->AddRowSorted(pRow, NULL);

		if(m_nID == -1) {
			
			//we're creating a new request

			//set the location to the current location
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_LocationCombo->TrySetSelByColumn_Deprecated(0, GetCurrentLocationID());

			//default to benefit category
			m_radioBenefitCategory.SetCheck(TRUE);
			OnRequestTypeChanged();
			
			//auto-select "30"
			// (j.jones 2013-03-28 15:50) - PLID 52182 - added DefaultEligibilityBenefitCategory, if it is not valid
			// then we will select 30 (which is also the default)
			CString strCategory = GetRemotePropertyText("DefaultEligibilityBenefitCategory", "30", 0, "<None>", TRUE);
			m_CategoryCombo->TrySetSelByColumn_Deprecated(erccCode, _bstr_t(strCategory));

			// (j.jones 2010-07-19 11:12) - PLID 31082 - supported default insured party ID
			long nPatientID = -1;
			if(m_nDefaultInsuredPartyID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, MainPhysician, "
					"PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, "
					"PersonT.Last +  ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName "
					"FROM InsuredPartyT "
					"INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"WHERE InsuredPartyT.PersonID = {INT}", m_nDefaultInsuredPartyID);
				if(!rs->eof) {
					nPatientID = AdoFldLong(rs, "PatientID");

					// (j.jones 2011-07-08 12:55) - PLID 38305 - load this patient's row first,
					// and requery everybody else
					CString strPatientWhere;
					strPatientWhere.Format("%s AND PatientsT.PersonID <> %li", PATIENT_COMBO_WHERE_CLAUSE, nPatientID);
					m_PatientCombo->PutWhereClause((LPCTSTR)strPatientWhere);
					m_PatientCombo->Requery();

					IRowSettingsPtr pPatRow = m_PatientCombo->GetNewRow();
					pPatRow->PutValue(erpatcID, (long)nPatientID);
					pPatRow->PutValue(erpatcUserDefinedID, rs->Fields->Item["UserDefinedID"]->Value);
					pPatRow->PutValue(erpatcLast, rs->Fields->Item["Last"]->Value );
					pPatRow->PutValue(erpatcFirst, rs->Fields->Item["First"]->Value);
					pPatRow->PutValue(erpatcMiddle, rs->Fields->Item["Middle"]->Value);
					pPatRow->PutValue(erpatcFullName, rs->Fields->Item["FullName"]->Value);
					m_PatientCombo->AddRowSorted(pPatRow, NULL);

					long nSel = m_PatientCombo->TrySetSelByColumn_Deprecated(erpatcID, (long)nPatientID);
					if(nSel == sriNoRow) {
						//maybe it's inactive?
						_RecordsetPtr rsPatient = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
							"WHERE ID = {INT}", nPatientID);
						if(!rsPatient->eof) {
							m_nPendingPatientID = nPatientID;
							m_PatientCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPatient, "Name", "")));
						}
						else 
							m_PatientCombo->PutCurSel(NULL);
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingPatientID = nPatientID;
					}

					//requery the insured party list accordingly
					CString strWhere;
					strWhere.Format("(RespTypeID <> -1 OR InsuredPartyT.PersonID = %li) AND PatientID = %li", m_nDefaultInsuredPartyID, nPatientID);
					m_InsuredPartyCombo->PutWhereClause(_bstr_t(strWhere));
					m_InsuredPartyCombo->Requery();

					//don't need to check for inactive, we already ensured that this insured party is in the where clause
					m_InsuredPartyCombo->TrySetSelByColumn_Deprecated(ericID, (long)m_nDefaultInsuredPartyID);
					
					long nProviderID = AdoFldLong(rs, "MainPhysician",-1);
					if(nProviderID != -1) {
						//handle inactive providers
						// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
						long nSel = m_ProviderCombo->TrySetSelByColumn_Deprecated(erpcID, (long)nProviderID);
						if(nSel == sriNoRow) {
							//maybe it's inactive?
							_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
								"WHERE ID = {INT}", nProviderID);
							if(!rsProv->eof) {
								m_nPendingProviderID = nProviderID;
								m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
							}
							else 
								m_ProviderCombo->PutCurSel(NULL);
						}
						else if(nSel == sriNoRowYet_WillFireEvent) {
							m_nPendingProviderID = nProviderID;
						}
					}
				}
				rs->Close();
			}

			// (j.jones 2011-07-08 12:55) - PLID 38305 - if we didn't load a default patient,
			// we still need to requery the patient list
			if(nPatientID == -1) {
				m_PatientCombo->PutWhereClause((LPCTSTR)PATIENT_COMBO_WHERE_CLAUSE);
				m_PatientCombo->Requery();
			}

			// (j.jones 2014-03-10 10:58) - PLID 60767 - diag codes need 4 dummy rows
			{
				IRowSettingsPtr pDiagRow = m_DiagCodeList->GetNewRow();
				pDiagRow->PutValue(erdcIndexNumber, (long)1);
				pDiagRow->PutValue(erdcCodeID, g_cvarNull);
				pDiagRow->PutValue(erdcCode, g_cvarNull);
				pDiagRow->PutValue(erdcDescription, g_cvarNull);
				m_DiagCodeList->AddRowAtEnd(pDiagRow, NULL);
				pDiagRow = m_DiagCodeList->GetNewRow();
				pDiagRow->PutValue(erdcIndexNumber, (long)2);
				pDiagRow->PutValue(erdcCodeID, g_cvarNull);
				pDiagRow->PutValue(erdcCode, g_cvarNull);
				pDiagRow->PutValue(erdcDescription, g_cvarNull);
				m_DiagCodeList->AddRowAtEnd(pDiagRow, NULL);
				pDiagRow = m_DiagCodeList->GetNewRow();
				pDiagRow->PutValue(erdcIndexNumber, (long)3);
				pDiagRow->PutValue(erdcCodeID, g_cvarNull);
				pDiagRow->PutValue(erdcCode, g_cvarNull);
				pDiagRow->PutValue(erdcDescription, g_cvarNull);
				m_DiagCodeList->AddRowAtEnd(pDiagRow, NULL);
				pDiagRow = m_DiagCodeList->GetNewRow();
				pDiagRow->PutValue(erdcIndexNumber, (long)4);
				pDiagRow->PutValue(erdcCodeID, g_cvarNull);
				pDiagRow->PutValue(erdcCode, g_cvarNull);
				pDiagRow->PutValue(erdcDescription, g_cvarNull);
				m_DiagCodeList->AddRowAtEnd(pDiagRow, NULL);
			}
		}
		else {

			//load an existing request

			// (j.jones 2014-02-27 15:36) - PLID 60767 - diag codes are now in a list that we can just requery
			{
				CString strWhereClause;
				strWhereClause.Format("ID = %li", m_nID);
				m_DiagCodeList->PutWhereClause(_bstr_t(strWhereClause));
				m_DiagCodeList->Requery();
				//force a wait so we're positive the display is up to date
				m_DiagCodeList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			}

			// (c.haag 2010-10-15 9:21) - PLID 40352 - Added PlaceOfServiceID
			// (j.jones 2013-03-28 15:07) - PLID 52182 - changed the category to be an ID, not a string
			_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyID, PatientID, ProviderID, LocationID, "
				"BenefitCategoryID, ServiceID, Modifier1, Modifier2, Modifier3, Modifier4, PlaceOfServiceID, "
				"Batched, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, "
				"PersonT.Last +  ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName "
				"FROM EligibilityRequestsT "
				"LEFT JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
				"INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"WHERE EligibilityRequestsT.ID = {INT}", m_nID);
			if(!rs->eof) {

				long nPatientID = AdoFldLong(rs, "PatientID",-1);
				//handle inactive patients
				if(nPatientID != -1) {

					// (j.jones 2011-07-08 12:55) - PLID 38305 - load this patient's row first,
					// and requery everybody else
					CString strPatientWhere;
					strPatientWhere.Format("%s AND PatientsT.PersonID <> %li", PATIENT_COMBO_WHERE_CLAUSE, nPatientID);
					m_PatientCombo->PutWhereClause((LPCTSTR)strPatientWhere);
					m_PatientCombo->Requery();

					IRowSettingsPtr pPatRow = m_PatientCombo->GetNewRow();
					pPatRow->PutValue(erpatcID, (long)nPatientID);
					pPatRow->PutValue(erpatcUserDefinedID, rs->Fields->Item["UserDefinedID"]->Value);
					pPatRow->PutValue(erpatcLast, rs->Fields->Item["Last"]->Value );
					pPatRow->PutValue(erpatcFirst, rs->Fields->Item["First"]->Value);
					pPatRow->PutValue(erpatcMiddle, rs->Fields->Item["Middle"]->Value);
					pPatRow->PutValue(erpatcFullName, rs->Fields->Item["FullName"]->Value);
					m_PatientCombo->AddRowSorted(pPatRow, NULL);

					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_PatientCombo->TrySetSelByColumn_Deprecated(erpatcID, (long)nPatientID);
					if(nSel == sriNoRow) {
						//maybe it's inactive?
						_RecordsetPtr rsPatient = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
							"WHERE ID = {INT}", nPatientID);
						if(!rsPatient->eof) {
							m_nPendingPatientID = nPatientID;
							m_PatientCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPatient, "Name", "")));
						}
						else 
							m_PatientCombo->PutCurSel(NULL);
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingPatientID = nPatientID;
					}
				}

				//requery the insured party list accordingly
				CString strWhere;
				strWhere.Format("RespTypeID <> -1 AND PatientID = %li", nPatientID);
				m_InsuredPartyCombo->PutWhereClause(_bstr_t(strWhere));
				m_InsuredPartyCombo->Requery();

				long nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
				if(nInsuredPartyID != -1) {
					//handle inactive insurance
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_InsuredPartyCombo->TrySetSelByColumn_Deprecated(ericID, (long)nInsuredPartyID);
					if(nSel == sriNoRow) {
						//maybe it's inactive?
						_RecordsetPtr rsInsured = CreateParamRecordset("SELECT Name FROM InsuranceCoT "
							"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
							"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
						if(!rsInsured->eof) {
							m_nPendingInsuredPartyID = nInsuredPartyID;
							m_InsuredPartyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsInsured, "Name", "")));
						}
						else 
							m_InsuredPartyCombo->PutCurSel(NULL);
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingInsuredPartyID = nInsuredPartyID;
					}
				}
				
				long nProviderID = AdoFldLong(rs, "ProviderID",-1);
				if(nProviderID != -1) {
					//handle inactive providers
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_ProviderCombo->TrySetSelByColumn_Deprecated(erpcID, (long)nProviderID);
					if(nSel == sriNoRow) {
						//maybe it's inactive?
						_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
							"WHERE ID = {INT}", nProviderID);
						if(!rsProv->eof) {
							m_nPendingProviderID = nProviderID;
							m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
						}
						else 
							m_ProviderCombo->PutCurSel(NULL);
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingProviderID = nProviderID;
					}
				}
				else {
					// (j.jones 2011-07-08 12:55) - PLID 38305 - if we didn't load a default patient,
					// we still need to requery the patient list
					m_PatientCombo->PutWhereClause((LPCTSTR)PATIENT_COMBO_WHERE_CLAUSE);
					m_PatientCombo->Requery();
				}

				long nLocationID = AdoFldLong(rs, "LocationID",-1);
				if(nLocationID != -1) {
					//handle inactive locations
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_LocationCombo->TrySetSelByColumn_Deprecated(0, (long)nLocationID);
					if(nSel == sriNoRow) {
						//maybe it's inactive?
						_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
						if(!rsLoc->eof) {
							m_nPendingLocationID = nLocationID;
							m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
						}
						else 
							m_LocationCombo->PutCurSel(NULL);
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingLocationID = nLocationID;
					}
				}

				long nServiceID = AdoFldLong(rs, "ServiceID",-1);

				//now we can determine whether this is a category or service-based request
				if(nServiceID != -1) {
					m_radioServiceCode.SetCheck(TRUE);
				}
				else {
					m_radioBenefitCategory.SetCheck(TRUE);

					//if not a service request, set the category code
					// (j.jones 2013-03-28 15:07) - PLID 52182 - changed the category to be an ID, not a string
					long nCategoryID = AdoFldLong(rs, "BenefitCategoryID", -1);
					if(nCategoryID != -1) {
						m_CategoryCombo->TrySetSelByColumn_Deprecated(erccID, nCategoryID);
					}
				}
				OnRequestTypeChanged();

				//now set the service, if one exists
				if(nServiceID != -1) {
					//handle inactive services
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_ServiceCombo->TrySetSelByColumn_Deprecated(0, (long)nServiceID);
					if(nSel == sriNoRow) {
						//maybe it's inactive?
						_RecordsetPtr rsService = CreateParamRecordset("SELECT Code FROM CPTCodeT WHERE ID = {INT}", nServiceID);
						if(!rsService->eof) {
							m_nPendingServiceID = nServiceID;
							m_ServiceCombo->PutComboBoxText(_bstr_t(AdoFldString(rsService, "Code", "")));
						}
						else 
							m_ServiceCombo->PutCurSel(NULL);
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingServiceID = nServiceID;
					}
				}

				CString strModifier1 = AdoFldString(rs, "Modifier1","");
				if(strModifier1 != "") {
					//handle inactive modifiers
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_ModifierCombo1->TrySetSelByColumn_Deprecated(ermcModifier, _bstr_t(strModifier1));
					if(nSel == sriNoRow) {
						m_strPendingModifier1 = strModifier1;
						m_ModifierCombo1->PutComboBoxText(_bstr_t(strModifier1));
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_strPendingModifier1 = strModifier1;
					}
				}

				CString strModifier2 = AdoFldString(rs, "Modifier2","");
				if(strModifier2 != "") {
					//handle inactive modifiers
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_ModifierCombo2->TrySetSelByColumn_Deprecated(ermcModifier, _bstr_t(strModifier2));
					if(nSel == sriNoRow) {
						m_strPendingModifier2 = strModifier2;
						m_ModifierCombo2->PutComboBoxText(_bstr_t(strModifier2));
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_strPendingModifier2 = strModifier2;
					}
				}

				CString strModifier3 = AdoFldString(rs, "Modifier3","");
				if(strModifier3 != "") {
					//handle inactive modifiers
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_ModifierCombo3->TrySetSelByColumn_Deprecated(ermcModifier, _bstr_t(strModifier3));
					if(nSel == sriNoRow) {
						m_strPendingModifier3 = strModifier3;
						m_ModifierCombo3->PutComboBoxText(_bstr_t(strModifier3));
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_strPendingModifier3 = strModifier3;
					}
				}

				CString strModifier4 = AdoFldString(rs, "Modifier4","");
				if(strModifier4 != "") {
					//handle inactive modifiers
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					long nSel = m_ModifierCombo4->TrySetSelByColumn_Deprecated(ermcModifier, _bstr_t(strModifier4));
					if(nSel == sriNoRow) {
						m_strPendingModifier4 = strModifier4;
						m_ModifierCombo4->PutComboBoxText(_bstr_t(strModifier4));
					}
					else if(nSel == sriNoRowYet_WillFireEvent) {
						m_strPendingModifier4 = strModifier4;
					}
				}

				// (c.haag 2010-10-15 9:15) - PLID 40352 - Place of Service type
				long nPOSID = AdoFldLong(rs, "PlaceOfServiceID",-1);
				if (-1 != nPOSID) {
					long nSel = m_POSCombo->TrySetSelByColumn_Deprecated(erpID, nPOSID);
					if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingPOSID = nPOSID;
					}
				}

				// (j.jones 2007-06-21 12:08) - PLID 26387 - in order to prompt to re-batch a request,
				// we need to know if it is unbatched
				m_bIsBatched = AdoFldBool(rs, "Batched",FALSE);
			}
			rs->Close();
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEligibilityRequestDlg::OnOK() 
{
	try {

		long nPatientID = -1;
		long nInsuredPartyID = -1;
		long nProviderID = -1;
		long nLocationID = -1;
		long nServiceID = -1;
		_variant_t varCategoryID = g_cvarNull;
		_variant_t varServiceID = g_cvarNull;
		_variant_t varModifier1 = g_cvarNull;
		_variant_t varModifier2 = g_cvarNull;
		_variant_t varModifier3 = g_cvarNull;
		_variant_t varModifier4 = g_cvarNull;
		long nPOSID = -1; // (c.haag 2010-10-15 9:21) - PLID 40352 - Place of Service type
		_variant_t varDiagID1 = g_cvarNull;
		_variant_t varDiagID2 = g_cvarNull;
		_variant_t varDiagID3 = g_cvarNull;
		_variant_t varDiagID4 = g_cvarNull;
		_variant_t varPOSID = g_cvarNull;

		//a patient, insured party, provider, and location are required

		IRowSettingsPtr pPatientRow = m_PatientCombo->GetCurSel();
		if(pPatientRow != NULL) {
			nPatientID = VarLong(pPatientRow->GetValue(erpatcID), -1);
		}
		else if((m_PatientCombo->IsComboBoxTextInUse || m_PatientCombo->GetIsTrySetSelPending()) && m_nPendingPatientID != -1) {
			nPatientID = m_nPendingPatientID;
		}

		if(nPatientID == -1) {
			AfxMessageBox("You must select a patient for this eligibility request.");
			return;
		}

		IRowSettingsPtr pInsuredRow = m_InsuredPartyCombo->GetCurSel();

		if(pInsuredRow != NULL) {
			nInsuredPartyID = VarLong(pInsuredRow->GetValue(ericID), -1);
		}
		else if((m_InsuredPartyCombo->IsComboBoxTextInUse || m_InsuredPartyCombo->GetIsTrySetSelPending()) && m_nPendingInsuredPartyID != -1) {
			nInsuredPartyID = m_nPendingInsuredPartyID;
		}

		if(nInsuredPartyID == -1) {
			AfxMessageBox("You must select an insured party for this eligibility request.");
			return;
		}

		IRowSettingsPtr pProviderRow = m_ProviderCombo->GetCurSel();

		if(pProviderRow != NULL) {
			nProviderID = VarLong(pProviderRow->GetValue(erpcID), -1);
		}
		else if((m_ProviderCombo->IsComboBoxTextInUse || m_ProviderCombo->GetIsTrySetSelPending()) && m_nPendingProviderID != -1) {
			nProviderID = m_nPendingProviderID;
		}

		if(nProviderID == -1) {
			AfxMessageBox("You must select a provider for this eligibility request.");
			return;
		}

		IRowSettingsPtr pLocationRow = m_LocationCombo->GetCurSel();

		if(pLocationRow != NULL) {
			nLocationID = VarLong(pLocationRow->GetValue(0), -1);
		}
		else if((m_LocationCombo->IsComboBoxTextInUse || m_LocationCombo->GetIsTrySetSelPending()) && m_nPendingLocationID != -1) {
			nLocationID = m_nPendingLocationID;
		}

		if(nLocationID == -1) {
			AfxMessageBox("You must select a location for this eligibility request.");
			return;
		}

		//now, what kind of request is this?

		BOOL bIsServiceCodeUsed = m_radioServiceCode.GetCheck();

		if(!bIsServiceCodeUsed) {

			//ensure a category is selected

			IRowSettingsPtr pCategoryRow = m_CategoryCombo->GetCurSel();
			if(pCategoryRow == NULL) {
				AfxMessageBox("You must select a category for a Benefit Category-based eligibility request.");
				return;
			}

			long nCategoryID = VarLong(pCategoryRow->GetValue(erccID));
			varCategoryID = nCategoryID;
		}
		else {

			//ensure a service code is selected

			IRowSettingsPtr pServiceRow = m_ServiceCombo->GetCurSel();

			if(pServiceRow != NULL) {
				nServiceID = VarLong(pServiceRow->GetValue(0), -1);
			}
			else if((m_ServiceCombo->IsComboBoxTextInUse || m_ServiceCombo->GetIsTrySetSelPending()) && m_nPendingServiceID != -1) {
				nServiceID = m_nPendingServiceID;
			}

			if(nServiceID == -1) {
				AfxMessageBox("You must select a service code for a service code-based eligibility request.");
				return;
			}
			
			varServiceID = nServiceID;

			//and now check the modifiers

			IRowSettingsPtr pModifier1Row = m_ModifierCombo1->GetCurSel();
			if(pModifier1Row != NULL) {
				CString strModifier1 = VarString(pModifier1Row->GetValue(ermcModifier), "");
				if(!strModifier1.IsEmpty()) {
					varModifier1 = _bstr_t(strModifier1);
				}
			}
			else if((m_ModifierCombo1->IsComboBoxTextInUse || m_ModifierCombo1->GetIsTrySetSelPending()) && m_strPendingModifier1 != "") {
				if(!m_strPendingModifier1.IsEmpty()) {
					varModifier1 = _bstr_t(m_strPendingModifier1);
				}
			}

			IRowSettingsPtr pModifier2Row = m_ModifierCombo2->GetCurSel();
			if(pModifier2Row != NULL) {
				CString strModifier2 = VarString(pModifier2Row->GetValue(ermcModifier), "");
				if(!strModifier2.IsEmpty()) {
					varModifier2 = _bstr_t(strModifier2);
				}
			}
			else if((m_ModifierCombo2->IsComboBoxTextInUse || m_ModifierCombo2->GetIsTrySetSelPending()) && m_strPendingModifier2 != "") {
				if(!m_strPendingModifier2.IsEmpty()) {
					varModifier2 = _bstr_t(m_strPendingModifier2);
				}
			}

			IRowSettingsPtr pModifier3Row = m_ModifierCombo3->GetCurSel();
			if(pModifier3Row != NULL) {
				CString strModifier3 = VarString(pModifier3Row->GetValue(ermcModifier), "");
				if(!strModifier3.IsEmpty()) {
					varModifier3 = _bstr_t(strModifier3);
				}
			}
			else if((m_ModifierCombo3->IsComboBoxTextInUse || m_ModifierCombo3->GetIsTrySetSelPending()) && m_strPendingModifier3 != "") {
				if(!m_strPendingModifier3.IsEmpty()) {
					varModifier3 = _bstr_t(m_strPendingModifier3);
				}
			}

			IRowSettingsPtr pModifier4Row = m_ModifierCombo4->GetCurSel();
			if(pModifier4Row != NULL) {
				CString strModifier4 = VarString(pModifier4Row->GetValue(ermcModifier), "");
				if(!strModifier4.IsEmpty()) {
					varModifier4 = _bstr_t(strModifier4);
				}
			}
			else if((m_ModifierCombo4->IsComboBoxTextInUse || m_ModifierCombo4->GetIsTrySetSelPending()) && m_strPendingModifier4 != "") {
				if(!m_strPendingModifier4.IsEmpty()) {
					varModifier4 = _bstr_t(m_strPendingModifier4);
				}
			}
		}

		// (j.jones 2014-02-27 17:03) - PLID 60767 - save the diagnosis codes from our list
		IRowSettingsPtr pDiagRow = m_DiagCodeList->GetFirstRow();
		while(pDiagRow) {
			long nIndex = VarLong(pDiagRow->GetValue(erdcIndexNumber));
			long nCodeID = VarLong(pDiagRow->GetValue(erdcCodeID), -1);

			//skip blank rows
			if(nCodeID != -1) {

				switch(nIndex) {
					case 1:
						varDiagID1 = nCodeID;
						break;
					case 2:
						varDiagID2 = nCodeID;
						break;
					case 3:
						varDiagID3 = nCodeID;
						break;
					case 4:
						varDiagID4 = nCodeID;
						break;
					default:
						//should not be possible
						ThrowNxException("Invalid diagnosis index %li found.", nIndex);
						break;
				}
			}

			pDiagRow = pDiagRow->GetNextRow();
		}

		// (c.haag 2010-10-15 9:21) - PLID 40352 - Place of Service type
		IRowSettingsPtr pPOSRow = m_POSCombo->GetCurSel();
		if(pPOSRow != NULL) {
			nPOSID = VarLong(pPOSRow->GetValue(erpID), -1);
		}
		else if((m_POSCombo->IsComboBoxTextInUse || m_POSCombo->GetIsTrySetSelPending()) && m_nPendingPOSID != -1) {
			nPOSID = m_nPendingPOSID;
		}

		if(nPOSID != -1) {
			varPOSID = nPOSID;
		}

		//and now save

		// (j.jones 2010-07-07 17:39) - PLID 39515 - if the Real-Time export is enabled,
		// auto-send if creating new, prompt if editing existing
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not clearinghouse-specific
		BOOL bUseRealTimeElig = (GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);
		BOOL bRealTimeExport = FALSE;

		if(m_nID == -1) {
			//save a new request

			long nID = NewNumber("EligibilityRequestsT", "ID");
			// (c.haag 2010-10-15 9:21) - PLID 40352 - Added PlaceOfServiceID
			// (j.jones 2013-03-28 15:07) - PLID 52182 - changed the category to be an ID, not a string
			ExecuteParamSql("INSERT INTO EligibilityRequestsT (ID, InsuredPartyID, ProviderID, LocationID, Batched, Selected, "
				"CreateDate, LastSentDate, BenefitCategoryID, ServiceID, Modifier1, Modifier2, Modifier3, Modifier4, "
				"Diagnosis1, Diagnosis2, Diagnosis3, Diagnosis4, PlaceOfServiceID) VALUES ({INT}, {INT}, {INT}, {INT}, 1, 0, "
				"GetDate(), NULL, {VT_I4}, {VT_I4}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, {VT_BSTR}, "
				"{VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4})",
				nID, nInsuredPartyID, nProviderID, nLocationID,
				varCategoryID, varServiceID, varModifier1, varModifier2, varModifier3, varModifier4, 	
				varDiagID1, varDiagID2, varDiagID3, varDiagID4, varPOSID);

			//if that succeeded, we can now assign nID to m_nID, which is accessible by the caller
			m_nID = nID;

			if(bUseRealTimeElig) {
				bRealTimeExport = TRUE;
			}
		}
		else {
			//update an existing request

			// (j.jones 2010-07-07 17:42) - PLID 39515 - if real-time exports are enabled,
			// ask if we want to export the changed request right now
			if(bUseRealTimeElig && IDYES == MessageBox("Would you like to submit this request now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				bRealTimeExport = TRUE;
			}
			
			// (j.jones 2007-06-21 12:08) - PLID 26387 - prior to saving the existing request,
			// see if we are unbatched, and if so, prompt to re-batch
			if(!bRealTimeExport && !m_bIsBatched && IDYES == MessageBox("This request is not currently in the E-Eligibility batch.\n"
				"Would you like to batch it now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				m_bIsBatched = TRUE;
			}

			// (c.haag 2010-10-15 9:21) - PLID 40352 - Added PlaceOfServiceID
			// (j.jones 2013-03-28 15:07) - PLID 52182 - changed the category to be an ID, not a string
			ExecuteParamSql("UPDATE EligibilityRequestsT SET InsuredPartyID = {INT}, ProviderID = {INT}, LocationID = {INT}, "
				"BenefitCategoryID = {VT_I4}, ServiceID = {VT_I4}, Modifier1 = {VT_BSTR}, Modifier2 = {VT_BSTR}, Modifier3 = {VT_BSTR}, Modifier4 = {VT_BSTR}, "
				"Diagnosis1 = {VT_I4}, Diagnosis2 = {VT_I4}, Diagnosis3 = {VT_I4}, Diagnosis4 = {VT_I4}, PlaceOfServiceID = {VT_I4}, "
				//this will retain the selected status if already batched, revert to 0 if unbatched
				"Selected = (CASE WHEN Batched = 0 THEN 0 ELSE Selected END), Batched = {INT} "
				"WHERE ID = {INT}", nInsuredPartyID, nProviderID, nLocationID,
				varCategoryID, varServiceID, varModifier1, varModifier2, varModifier3, varModifier4, 	
				varDiagID1, varDiagID2, varDiagID3, varDiagID4, varPOSID, m_bIsBatched ? 1 : 0, m_nID);
		}

		// (j.jones 2010-07-07 17:40) - PLID 39515 - if we're exporting, do so now
		if(bUseRealTimeElig && bRealTimeExport) {
			// (s.dhole 08/22/2012) PLID 52414  Will pass id if this is real time trasaction
			if (m_bModeless){
				GetMainFrame()->PostMessage(NXM_ELIGIBILITYREQUEST_CLOSED, IDOK,(LPARAM)m_nID);
			}
			else{
				//if current dialog is not modeless than  we need to explicitly call this routine
				//if we were not given a format, load it now
				if(m_nFormatID == -1) {
					// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
					m_nFormatID = GetDefaultEbillingANSIFormatID();
				}
				CEEligibility dlg(this);
				dlg.m_FormatID = m_nFormatID;
				//// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
				dlg.m_bUseRealTimeElig = bUseRealTimeElig;
				// (s.dhole 09/11/2012) PLID 52414  Will pass id if this is real time trasaction
				// do not show CEligibilityRequestDetailDlg since it is caller 	
				dlg.m_bCEligibilityRequestDetailDlg = FALSE;
				////add this request
				dlg.m_aryRequestIDsToExport.Add(m_nID);
				dlg.DoModal();
			}
		}
		else if (m_bModeless){
			// (s.dhole 07/23/2012) PLID 48693
			// If parent is exist than post message
			GetMainFrame()->PostMessage(NXM_ELIGIBILITYREQUEST_CLOSED, IDOK,(LPARAM)-1L);
		}
		CDialog::OnOK();
	}NxCatchAll("Error saving Eligibility request.");
}

void CEligibilityRequestDlg::OnCancel() 
{	
	// (s.dhole 07/23/2012) PLID 48693
	try{
		if (m_bModeless){
			GetMainFrame()->PostMessage(NXM_ELIGIBILITYREQUEST_CLOSED, IDCANCEL,(LPARAM)-1L);
		}
		CDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEligibilityRequestDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEligibilityRequestDlg)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIGIBILITY_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenEligibilityPatientCombo, VTS_DISPATCH)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIGIBILITY_PATIENT_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligibilityPatientCombo, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIGIBILITY_INSURED_PARTY_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligibilityInsuredPartyCombo, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIGIBILITY_PROVIDER_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligibilityProviderCombo, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIGIBILITY_LOCATION_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligibilityLocationCombo, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIGIBILITY_SERVICE_CODE_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligibilityServiceCodeCombo, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIG_MODIFIER_COMBO_1, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligModifierCombo1, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIG_MODIFIER_COMBO_2, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligModifierCombo2, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIG_MODIFIER_COMBO_3, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligModifierCombo3, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIG_MODIFIER_COMBO_4, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEligModifierCombo4, VTS_I4 VTS_I4)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIG_DIAG_SEARCH_LIST, 16 /* SelChosen */, OnSelChosenEligDiagSearchList, VTS_DISPATCH)
	ON_EVENT(CEligibilityRequestDlg, IDC_ELIG_DIAG_CODE_LIST, 6, OnRButtonDownEligDiagCodeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEligibilityRequestDlg::OnSelChosenEligibilityPatientCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//clear the insured party list
			m_InsuredPartyCombo->Clear();
			return;
		}

		long nPatientID = VarLong(pRow->GetValue(0),-1);

		//requery the insured party list accordingly

		CString strWhere;

		strWhere.Format("RespTypeID <> -1 AND PatientID = %li", nPatientID);

		m_InsuredPartyCombo->PutWhereClause(_bstr_t(strWhere));
		m_InsuredPartyCombo->Requery();

		//and now set the selection to the patient's first priority company
		IRowSettingsPtr pInsRow = m_InsuredPartyCombo->FindByColumn(ericRespID, (long)1, NULL, true);
		if(pInsRow == NULL) {
			//if there was a primary company, we would have auto-selected it,
			//and if not, FindByColumn would have waited for the list to finish requerying,
			//so a row would exist (if any rows are going to exist at all) and we can
			//then just select the first row
			m_InsuredPartyCombo->PutCurSel(m_InsuredPartyCombo->GetFirstRow());
		}

		//if nothing selected, switch the provider combo to the patient's G1 provider
		long nProviderID = -1;
		IRowSettingsPtr pProvRow = m_ProviderCombo->GetCurSel();
		if(pProvRow != NULL) {
			nProviderID = VarLong(pProvRow->GetValue(0), -1);
		}

		if(nProviderID == -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT}", nPatientID);
			if(!rs->eof) {
				nProviderID = AdoFldLong(rs, "MainPhysician", -1);
				m_ProviderCombo->SetSelByColumn(0, nProviderID);
			}
			rs->Close();
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnSelChosenEligibilityPatientCombo");
}

// (j.jones 2009-09-16 12:44) - PLID 26481 - moved to globalfinancialutils
/*
void CEligibilityRequestDlg::AddToCategoryCombo(CString strCode, CString strCategory)
{
	//add a new row to the category combo
	
	IRowSettingsPtr pRow = m_CategoryCombo->GetNewRow();
	pRow->PutValue(erccCode, _bstr_t(strCode));
	pRow->PutValue(erccCategory, _bstr_t(strCategory));
	m_CategoryCombo->AddRowAtEnd(pRow, NULL);	
}

void CEligibilityRequestDlg::BuildCategoryCombo()
{
	try {

		//build this insanely long combo
		AddToCategoryCombo("1", "Medical Care");
		AddToCategoryCombo("2", "Surgical");
		AddToCategoryCombo("3", "Consultation");
		AddToCategoryCombo("4", "Diagnostic X-Ray");
		AddToCategoryCombo("5", "Diagnostic Lab");
		AddToCategoryCombo("6", "Radiation Therapy");
		AddToCategoryCombo("7", "Anesthesia");
		AddToCategoryCombo("8", "Surgical Assistance");
		AddToCategoryCombo("9", "Other Medical");
		AddToCategoryCombo("10", "Blood Charges");
		AddToCategoryCombo("11", "Used Durable Medical Equipment");
		AddToCategoryCombo("12", "Durable Medical Equipment Purchase");
		AddToCategoryCombo("13", "Ambulatory Service Center Facility");
		AddToCategoryCombo("14", "Renal Supplies in the Home");
		AddToCategoryCombo("15", "Alternate Method Dialysis");
		AddToCategoryCombo("16", "Chronic Renal Disease (CRD) Equipment");
		AddToCategoryCombo("17", "Pre-Admission Testing");
		AddToCategoryCombo("18", "Durable Medical Equipment Rental");
		AddToCategoryCombo("19", "Pneumonia Vaccine");
		AddToCategoryCombo("20", "Second Surgical Opinion");
		AddToCategoryCombo("21", "Third Surgical Opinion");
		AddToCategoryCombo("22", "Social Work");
		AddToCategoryCombo("23", "Diagnostic Dental");
		AddToCategoryCombo("24", "Periodontics");
		AddToCategoryCombo("25", "Restorative");
		AddToCategoryCombo("26", "Endodontics");
		AddToCategoryCombo("27", "Maxillofacial Prosthetics");
		AddToCategoryCombo("28", "Adjunctive Dental Services");
		AddToCategoryCombo("30", "Health Benefit Plan Coverage");
		AddToCategoryCombo("32", "Plan Waiting Period");
		AddToCategoryCombo("33", "Chiropractic");
		AddToCategoryCombo("34", "Chiropractic Office Visits");
		AddToCategoryCombo("35", "Dental Care");
		AddToCategoryCombo("36", "Dental Crowns");
		AddToCategoryCombo("37", "Dental Accident");
		AddToCategoryCombo("38", "Orthodontics");
		AddToCategoryCombo("39", "Prosthodontics");
		AddToCategoryCombo("40", "Oral Surgery");
		AddToCategoryCombo("41", "Routine (Preventive) Dental");
		AddToCategoryCombo("42", "Home Health Care");
		AddToCategoryCombo("43", "Home Health Prescriptions");
		AddToCategoryCombo("44", "Home Health Visits");
		AddToCategoryCombo("45", "Hospice");
		AddToCategoryCombo("46", "Respite Care");
		AddToCategoryCombo("47", "Hospital");
		AddToCategoryCombo("48", "Hospital - Inpatient");
		AddToCategoryCombo("49", "Hospital - Room and Board");
		AddToCategoryCombo("50", "Hospital - Outpatient");
		AddToCategoryCombo("51", "Hospital - Emergency Accident");
		AddToCategoryCombo("52", "Hospital - Emergency Medical");
		AddToCategoryCombo("53", "Hospital - Ambulatory Surgical");
		AddToCategoryCombo("54", "Long Term Care");
		AddToCategoryCombo("55", "Major Medical");
		AddToCategoryCombo("56", "Medically Related Transportation");
		AddToCategoryCombo("57", "Air Transportation");
		AddToCategoryCombo("58", "Cabulance");
		AddToCategoryCombo("59", "Licensed Ambulance");
		AddToCategoryCombo("60", "General Benefits");
		AddToCategoryCombo("61", "In-vitro Fertilization");
		AddToCategoryCombo("62", "MRI/CAT Scan");
		AddToCategoryCombo("63", "Donor Procedures");
		AddToCategoryCombo("64", "Acupuncture");
		AddToCategoryCombo("65", "Newborn Care");
		AddToCategoryCombo("66", "Pathology");
		AddToCategoryCombo("67", "Smoking Cessation");
		AddToCategoryCombo("68", "Well Baby Care");
		AddToCategoryCombo("69", "Maternity");
		AddToCategoryCombo("70", "Transplants");
		AddToCategoryCombo("71", "Audiology Exam");
		AddToCategoryCombo("72", "Inhalation Therapy");
		AddToCategoryCombo("73", "Diagnostic Medical");
		AddToCategoryCombo("74", "Private Duty Nursing");
		AddToCategoryCombo("75", "Prosthetic Device");
		AddToCategoryCombo("76", "Dialysis");
		AddToCategoryCombo("77", "Otological Exam");
		AddToCategoryCombo("78", "Chemotherapy");
		AddToCategoryCombo("79", "Allergy Testing");
		AddToCategoryCombo("80", "Immunizations");
		AddToCategoryCombo("81", "Routine Physical");
		AddToCategoryCombo("82", "Family Planning");
		AddToCategoryCombo("83", "Infertility");
		AddToCategoryCombo("84", "Abortion");
		AddToCategoryCombo("85", "AIDS");
		AddToCategoryCombo("86", "Emergency Services");
		AddToCategoryCombo("87", "Cancer");
		AddToCategoryCombo("88", "Pharmacy");
		AddToCategoryCombo("89", "Free Standing Prescription Drug");
		AddToCategoryCombo("90", "Mail Order Prescription Drug");
		AddToCategoryCombo("91", "Brand Name Prescription Drug");
		AddToCategoryCombo("92", "Generic Prescription Drug");
		AddToCategoryCombo("93", "Podiatry");
		AddToCategoryCombo("94", "Podiatry - Office Visits");
		AddToCategoryCombo("95", "Podiatry - Nursing Home Visits");
		AddToCategoryCombo("96", "Professional (Physician)");
		AddToCategoryCombo("97", "Anesthesiologist");
		AddToCategoryCombo("98", "Professional (Physician) Visit - Office");
		AddToCategoryCombo("99", "Professional (Physician) Visit - Inpatient");
		AddToCategoryCombo("A0", "Professional (Physician) Visit - Outpatient");
		AddToCategoryCombo("A1", "Professional (Physician) Visit - Nursing Home");
		AddToCategoryCombo("A2", "Professional (Physician) Visit - Skilled Nursing Facility");
		AddToCategoryCombo("A3", "Professional (Physician) Visit - Home");
		AddToCategoryCombo("A4", "Psychiatric");
		AddToCategoryCombo("A5", "Psychiatric - Room and Board");
		AddToCategoryCombo("A6", "Psychotherapy");
		AddToCategoryCombo("A7", "Psychiatric - Inpatient");
		AddToCategoryCombo("A8", "Psychiatric - Outpatient");
		AddToCategoryCombo("A9", "Rehabilitation");
		AddToCategoryCombo("AA", "Rehabilitation - Room and Board");
		AddToCategoryCombo("AB", "Rehabilitation - Inpatient");
		AddToCategoryCombo("AC", "Rehabilitation - Outpatient");
		AddToCategoryCombo("AD", "Occupational Therapy");
		AddToCategoryCombo("AE", "Physical Medicine");
		AddToCategoryCombo("AF", "Speech Therapy");
		AddToCategoryCombo("AG", "Skilled Nursing Care");
		AddToCategoryCombo("AH", "Skilled Nursing Care - Room and Board");
		AddToCategoryCombo("AI", "Substance Abuse");
		AddToCategoryCombo("AJ", "Alcoholism");
		AddToCategoryCombo("AK", "Drug Addiction");
		AddToCategoryCombo("AL", "Vision (Optometry)");
		AddToCategoryCombo("AM", "Frames");
		AddToCategoryCombo("AN", "Routine Exam");
		AddToCategoryCombo("AO", "Lenses");
		AddToCategoryCombo("AQ", "Nonmedically Necessary Physical");
		AddToCategoryCombo("AR", "Experimental Drug Therapy");
		AddToCategoryCombo("BA", "Independent Medical Evaluation");
		AddToCategoryCombo("BB", "Partial Hospitalization (Psychiatric)");
		AddToCategoryCombo("BC", "Day Care (Psychiatric)");
		AddToCategoryCombo("BD", "Cognitive Therapy");
		AddToCategoryCombo("BE", "Massage Therapy");
		AddToCategoryCombo("BF", "Pulmonary Rehabilitation");
		AddToCategoryCombo("BG", "Cardiac Rehabilitation");
		AddToCategoryCombo("BH", "Pediatric");
		AddToCategoryCombo("BI", "Nursery");
		AddToCategoryCombo("BJ", "Skin");
		AddToCategoryCombo("BK", "Orthopedic");
		AddToCategoryCombo("BL", "Cardiac");
		AddToCategoryCombo("BM", "Lymphatic");
		AddToCategoryCombo("BN", "Gastrointestinal");
		AddToCategoryCombo("BP", "Endocrine");
		AddToCategoryCombo("BQ", "Neurology");
		AddToCategoryCombo("BR", "Eye");
		AddToCategoryCombo("BS", "Invasive Procedures");

	}NxCatchAll("Error in CEligibilityRequestDlg::BuildCategoryCombo");
}
*/

void CEligibilityRequestDlg::OnRadioReqBenefitCategory() 
{
	OnRequestTypeChanged();
}

void CEligibilityRequestDlg::OnRadioReqServiceCode() 
{
	OnRequestTypeChanged();
}

void CEligibilityRequestDlg::OnRequestTypeChanged()
{
	try {

		//enable the request-specific fields accordingly

		BOOL bIsServiceCodeUsed = m_radioServiceCode.GetCheck();

		m_CategoryCombo->Enabled = bIsServiceCodeUsed ? VARIANT_FALSE : VARIANT_TRUE;
		m_ServiceCombo->Enabled = bIsServiceCodeUsed ? VARIANT_TRUE : VARIANT_FALSE;
		m_ModifierCombo1->Enabled = bIsServiceCodeUsed ? VARIANT_TRUE : VARIANT_FALSE;
		m_ModifierCombo2->Enabled = bIsServiceCodeUsed ? VARIANT_TRUE : VARIANT_FALSE;
		m_ModifierCombo3->Enabled = bIsServiceCodeUsed ? VARIANT_TRUE : VARIANT_FALSE;
		m_ModifierCombo4->Enabled = bIsServiceCodeUsed ? VARIANT_TRUE : VARIANT_FALSE;

		//see if we have requeried the service list yet
		if(bIsServiceCodeUsed && m_bNeedServiceRequery) {

			m_ServiceCombo->Requery();
			m_ModifierCombo1->Requery();
			m_ModifierCombo2->Requery();
			m_ModifierCombo3->Requery();
			m_ModifierCombo4->Requery();

			//modifiers need "none selected" options
			IRowSettingsPtr pRow = m_ModifierCombo1->GetNewRow();
			pRow->PutValue(ermcModifier, _bstr_t(""));
			pRow->PutValue(ermcDescription, _bstr_t("<No Modifier Selected>"));
			m_ModifierCombo1->AddRowAtEnd(pRow, NULL);
			pRow = m_ModifierCombo2->GetNewRow();
			pRow->PutValue(ermcModifier, _bstr_t(""));
			pRow->PutValue(ermcDescription, _bstr_t("<No Modifier Selected>"));
			m_ModifierCombo2->AddRowAtEnd(pRow, NULL);
			pRow = m_ModifierCombo3->GetNewRow();
			pRow->PutValue(ermcModifier, _bstr_t(""));
			pRow->PutValue(ermcDescription, _bstr_t("<No Modifier Selected>"));
			m_ModifierCombo3->AddRowAtEnd(pRow, NULL);
			pRow = m_ModifierCombo4->GetNewRow();
			pRow->PutValue(ermcModifier, _bstr_t(""));
			pRow->PutValue(ermcDescription, _bstr_t("<No Modifier Selected>"));
			m_ModifierCombo4->AddRowAtEnd(pRow, NULL);

			m_bNeedServiceRequery = FALSE;
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnRequestTypeChanged");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityPatientCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsPatient = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = {INT}", m_nPendingPatientID);
			if(!rsPatient->eof) {
				m_PatientCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPatient, "Name", "")));
			}
			else 
				m_PatientCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingPatientID = -1;
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityPatientCombo");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityInsuredPartyCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsInsured = CreateParamRecordset("SELECT Name FROM InsuranceCoT "
				"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = {INT}", m_nPendingInsuredPartyID);
			if(!rsInsured->eof) {
				m_InsuredPartyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsInsured, "Name", "")));
			}
			else 
				m_InsuredPartyCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingInsuredPartyID = -1;
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityInsuredPartyCombo");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityProviderCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE PersonT.ID = {INT}", m_nPendingProviderID);
			if(!rsProv->eof) {
				m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
			else 
				m_ProviderCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingProviderID = -1;
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityProviderCombo");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityLocationCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT "
				"WHERE ID = {INT}", m_nPendingLocationID);
			if(!rsLoc->eof) {
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else 
				m_LocationCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingLocationID = -1;
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityLocationCombo");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityServiceCodeCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsService = CreateParamRecordset("SELECT Code FROM CPTCodeT "
				"WHERE ID = {INT}", m_nPendingServiceID);
			if(!rsService->eof) {
				m_ServiceCombo->PutComboBoxText(_bstr_t(AdoFldString(rsService, "Code", "")));
			}
			else 
				m_ServiceCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingServiceID = -1;
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligibilityServiceCodeCombo");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo1(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			m_ModifierCombo1->PutComboBoxText(_bstr_t(m_strPendingModifier1));
		}
		else {
			m_strPendingModifier1 = "";
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo1");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo2(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			m_ModifierCombo2->PutComboBoxText(_bstr_t(m_strPendingModifier2));
		}
		else {
			m_strPendingModifier2 = "";
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo2");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo3(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			m_ModifierCombo3->PutComboBoxText(_bstr_t(m_strPendingModifier3));
		}
		else {
			m_strPendingModifier3 = "";
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo3");
}

void CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo4(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			m_ModifierCombo4->PutComboBoxText(_bstr_t(m_strPendingModifier4));
		}
		else {
			m_strPendingModifier4 = "";
		}

	}NxCatchAll("Error in CEligibilityRequestDlg::OnTrySetSelFinishedEligModifierCombo4");
}

// (j.jones 2014-02-27 15:14) - PLID 60767 - added diag search and code list
void CEligibilityRequestDlg::OnSelChosenEligDiagSearchList(LPDISPATCH lpRow)
{
	try {

		if(lpRow) {
			CDiagSearchResults results = DiagSearchUtils::ConvertDualSearchResults(lpRow);

			if(results.m_ICD9.m_nDiagCodesID == -1 && results.m_ICD10.m_nDiagCodesID == -1) {
				//no code selected
				return;
			}
			
			//if we have 4 rows, do not add a 5th
			long nCountCodes = 0;
			long nFirstEmptyIndex = -1;
			NXDATALIST2Lib::IRowSettingsPtr pFirstEmptyRow = NULL;
			{
				
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow();
				while(pRow) {
					long nDiagID = VarLong(pRow->GetValue(erdcCodeID), -1);
					if(nDiagID != -1) {
						nCountCodes++;
					}
					else if(nFirstEmptyIndex == -1) {
						pFirstEmptyRow = pRow;
						nFirstEmptyIndex = VarLong(pFirstEmptyRow->GetValue(erdcIndexNumber));
					}
					pRow = pRow->GetNextRow();
				}
			}

			if(nCountCodes >= 4) {
				AfxMessageBox("No more than four diagnosis codes can be added to an E-Eligibility Request.");
				return;
			}
			
			if(pFirstEmptyRow == NULL || nFirstEmptyIndex == -1 || nFirstEmptyIndex > 4) {
				//above code should have caught this!
				ASSERT(FALSE);
				AfxMessageBox("No more than four diagnosis codes can be added to an E-Eligibility Request.");
				return;
			}

			//we're only adding an ICD-9 or ICD-10 code
			long nDiagCodeID = results.m_ICD9.m_nDiagCodesID;
			CString strDiagCode = results.m_ICD9.m_strCode;
			CString strDiagDesc = results.m_ICD9.m_strDescription;
			if(results.m_ICD10.m_nDiagCodesID != -1) {
				//should not have both an ICD-9 and an ICD-10
				ASSERT(nDiagCodeID == -1);
				nDiagCodeID = results.m_ICD10.m_nDiagCodesID;
				strDiagCode = results.m_ICD10.m_strCode;
				strDiagDesc = results.m_ICD10.m_strDescription;
			}

			//don't add duplicates
			if(m_DiagCodeList->FindByColumn(erdcCodeID, nDiagCodeID, m_DiagCodeList->GetFirstRow(), VARIANT_FALSE)) {
				AfxMessageBox("This diagnosis code has already been selected.");
				return;
			}
			
			//update the row
			{
				pFirstEmptyRow->PutValue(erdcCodeID, nDiagCodeID);
				pFirstEmptyRow->PutValue(erdcCode, _bstr_t(strDiagCode));
				pFirstEmptyRow->PutValue(erdcDescription, _bstr_t(strDiagDesc));
				m_DiagCodeList->Sort();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-28 11:25) - PLID 60867 - added right click ability to remove default codes
void CEligibilityRequestDlg::OnRButtonDownEligDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_DiagCodeList->CurSel = pRow;

		//if no code is selected, do nothing
		long nDiagID = VarLong(pRow->GetValue(erdcCodeID), -1);
		if(nDiagID == -1) {
			return;
		}

		long nIndexToRemove = VarLong(pRow->GetValue(erdcIndexNumber));

		enum {
			eRemoveDiag = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		CString strLabel;
		strLabel.Format("&Remove Diagnosis Code %li", nIndexToRemove);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveDiag, strLabel);

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveDiag) {

			//clear this row
			pRow->PutValue(erdcCodeID, g_cvarNull);
			pRow->PutValue(erdcCode,g_cvarNull);
			pRow->PutValue(erdcDescription, g_cvarNull);

			IRowSettingsPtr pFixRow = m_DiagCodeList->GetFirstRow();
			while(pFixRow) {
				long nCurIndex = VarLong(pFixRow->GetValue(erdcIndexNumber));
				if(nCurIndex > nIndexToRemove) {
					//move to the prior row
					IRowSettingsPtr pPrevRow = pFixRow->GetPreviousRow();
					if(pPrevRow) {
						pPrevRow->PutValue(erdcCodeID, pFixRow->GetValue(erdcCodeID));
						pPrevRow->PutValue(erdcCode, pFixRow->GetValue(erdcCode));
						pPrevRow->PutValue(erdcDescription, pFixRow->GetValue(erdcDescription));
					}
					//clear this row
					pFixRow->PutValue(erdcCodeID, g_cvarNull);
					pFixRow->PutValue(erdcCode,g_cvarNull);
					pFixRow->PutValue(erdcDescription, g_cvarNull);
				}

				pFixRow = pFixRow->GetNextRow();
			}

			m_DiagCodeList->Sort();
		}

	}NxCatchAll(__FUNCTION__);
}