#include "stdafx.h"
#include "CCDInterface.h"
#include "CCDUtils.h"
#include "SOAPUtils.h"
#include "GlobalParamUtils.h"
#include "SelectDlg.h"
#include "FileUtils.h"
#include "InternationalUtils.h"
#include "AdvanceDirectiveDlg.h"
#include "PracticeRc.h"
#include "NxXMLUtils.h"
#include "GlobalFinancialUtils.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// the open ehealth foundation's open source framework project is a good reference to see some specifics of xml structure, for example:
// http://gforge.openehealth.org/gf/project/ipf/scmsvn/?action=browse&path=/trunk/ipf/modules/cda/src/test/resources/builders/content/section/CCDProceduresExample.groovy&view=markup

// (a.walling 2009-05-12 11:03) - PLID 29924 - Initial creation of utility namespace for CCD documents
//(e.lally 2010-02-18) PLID 37438 - Moved all the practice/interface dependencies into their own source file
namespace CCD
{
#pragma region CCD Viewing And Parsing

	// (a.walling 2010-01-06 14:03) - PLID 36809 - Take a document pointer and handle CCD/CCR
	IStreamPtr GetOutputHTMLStream(MSXML2::IXMLDOMDocument2Ptr pDocument)
	{
		// (j.jones 2010-06-30 10:57) - PLID 38031 - GetXMLDocumentType is now
		// part of NxXMLUtils, not part of the CCD namespace
		NxXMLUtils::EDocumentType documentType = NxXMLUtils::GetXMLDocumentType(pDocument);

		if (documentType == NxXMLUtils::CCD_Document) {
			return ApplyXSLT(pDocument, GetCCDXSLResourcePath());
		} else if (documentType == NxXMLUtils::CCR_Document) {
			return ApplyXSLT(pDocument, GetCCRXSLResourcePath());
		// (j.gruber 2013-11-08 10:43) - PLID 59376 - include ccda
		} else if (documentType == NxXMLUtils::CCDA_Document) {
			return ApplyXSLT(pDocument, GetCCDAXSLResourcePath());
		} else {
			// (j.jones 2010-06-30 12:02) - PLID 38031 - GetGenericXMLXSLResourcePath
			// is now in GlobalUtils
			return ApplyXSLT(pDocument, GetGenericXMLXSLResourcePath());
		}
	}

	// (b.spivey, August 13, 2013) - PLID 57964 - CCDA XSL
	CString GetCCDAXSLResourcePath()
	{
		// (a.walling 2013-11-18 11:37) - PLID 59557 - XSL resources to named resources
		return "nxres://0/XSL/NxCCDA.xsl";
	}	

	CString GetCCDXSLResourcePath()
	{
		// (a.walling 2013-11-18 11:37) - PLID 59557 - XSL resources to named resources
		return "nxres://0/XSL/NxCCD.xsl";
	}	

	// (a.walling 2010-01-06 13:27) - PLID 36809 - XSL file for CCR
	CString GetCCRXSLResourcePath()
	{
		// (a.walling 2013-11-18 11:37) - PLID 59557 - XSL resources to named resources
		return "nxres://0/XSL/NxCCR.xsl";
	}
	
	COleDateTime CTimeRange::GetSingleTime() const
	{
		if (dtHigh.GetStatus() == COleDateTime::valid) {
			return dtHigh;
		} else if (dtLow.GetStatus() == COleDateTime::valid) {
			return dtLow;
		} else if (dtHigh.GetStatus() == COleDateTime::null) {
			return dtHigh;
		} else if (dtLow.GetStatus() == COleDateTime::null) {
			return dtLow;
		} else {
			return dtHigh; // invalid
		}
	}

	void CTimeRange::SetSingleTime(const COleDateTime& dt)
	{
		dtHigh = dt;
		dtLow.SetStatus(COleDateTime::invalid);
	}

	void CTimeRange::SetTimeRange(const COleDateTime& dtNewLow, const COleDateTime& dtNewHigh)
	{
		dtLow = dtNewLow;
		dtHigh = dtNewHigh;
	}

	void CTimeRange::LoadFromElement(MSXML2::IXMLDOMNodePtr pNode)
	{
		if (pNode) {
			CString strValue = GetTextFromXPath(pNode, "@value");
			
			if (!strValue.IsEmpty()) {
				// value in parent element, no low or high
				dtLow.SetStatus(COleDateTime::invalid);
				// set in dtHigh
				dtHigh = ParseFromString(strValue);
			} else {
				dtLow = ParseFromString(GetTextFromXPath(pNode, "n1:low/@value"));
				dtHigh = ParseFromString(GetTextFromXPath(pNode, "n1:high/@value"));
			}
		}
	}

	COleDateTime CTimeRange::ParseFromString(const CString& strTime)
	{
		COleDateTime dt;
		if (strTime.IsEmpty()) {
			dt.SetStatus(COleDateTime::invalid);
		} else if (strTime == "0") {
			dt.SetStatus(COleDateTime::null);
		} else if (strTime.GetLength() >= 8) {
			if (strTime.Left(4) == "0000") {
				dt.SetStatus(COleDateTime::null);
			} else {
				if (strTime.GetLength() >= 14) {
					dt.SetDateTime(
						atoi(strTime.Mid(0, 4)),
						atoi(strTime.Mid(4, 2)),
						atoi(strTime.Mid(6, 2)),
						atoi(strTime.Mid(8, 2)),
						atoi(strTime.Mid(10, 2)),
						atoi(strTime.Mid(12, 2)));
				} else {
					dt.SetDateTime(
						atoi(strTime.Mid(0, 4)),
						atoi(strTime.Mid(4, 2)),
						atoi(strTime.Mid(6, 2)),
						0, 0, 0);
				}
			}
		} else {
			dt.SetStatus(COleDateTime::invalid);
		}

		return dt;
	}

	// (a.walling 2010-01-19 14:09) - PLID 36972 - Validation via NIST web service removed (service out of date)

	// (a.walling 2009-05-21 09:25) - PLID 34176 - Consolidated some CCD description functions
	void GetDescriptionInfo(const CString& strFile, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime)
	{		
		strSuggestedDescription = "Clinical Document: ";

		MSXML2::IXMLDOMDocument2Ptr pDocument(__uuidof(MSXML2::DOMDocument60));

		if (pDocument->load((LPCTSTR)strFile)) {
			GetDescriptionInfo(pDocument, strDefault, strTitle, strDisplay, strSuggestedDescription, trTime);
		} else {
			strSuggestedDescription += strDefault;
		}
	}
	
	// (a.walling 2009-05-21 09:25) - PLID 34176 - Consolidated some CCD description functions
	void GetDescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime)
	{
		// (a.walling 2010-01-06 12:20) - PLID 36809 - Handle this for CCR documents as well
		// (j.jones 2010-06-30 10:57) - PLID 38031 - GetXMLDocumentType is now
		// part of NxXMLUtils, not part of the CCD namespace
		NxXMLUtils::EDocumentType documentType = NxXMLUtils::GetXMLDocumentType(pDocument);

		if (documentType == NxXMLUtils::CCD_Document) {
			GetCCDDescriptionInfo(pDocument, strDefault, strTitle, strDisplay, strSuggestedDescription, trTime);
		} else if (documentType == NxXMLUtils::CCR_Document) {
			GetCCRDescriptionInfo(pDocument, strDefault, strTitle, strDisplay, strSuggestedDescription, trTime);
			// (j.gruber 2013-11-08 11:00) - PLID 59376 - ccda
		} else if (documentType == NxXMLUtils::CCDA_Document) {
			GetCCDADescriptionInfo(pDocument, strDefault, strTitle, strDisplay, strSuggestedDescription, trTime);
		} else {
			ThrowNxException("This is not a valid CCD, CDA, or CCR document.");
		}
	}

	// (a.walling 2010-01-06 13:06) - PLID 36809 - Get the date time from the CCR ISO 8601 string
	COleDateTime GetCCRDateTimeFromNode(MSXML2::IXMLDOMNodePtr pNode)
	{		
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		if (!pNode) {
			return dtInvalid;
		}

		bool bApprox = false;
		CString strDateTime;
		strDateTime = GetXMLNodeText(pNode, "ExactDateTime");
		if (strDateTime.IsEmpty()) {
			bApprox = true;
			strDateTime = GetXMLNodeText(pNode, "ApproximateDateTime");
		}

		if (strDateTime.IsEmpty()) {
			return dtInvalid;
		}

		long nLength = strDateTime.GetLength();

		long nYear = nLength >= 4 ? atoi(strDateTime.Mid(0, 4)) : 0;
		long nMonth = nLength >= 7 ? atoi(strDateTime.Mid(5, 2)) : 0;
		long nDay = nLength >= 10 ? atoi(strDateTime.Mid(8, 2)) : 0;

		if (nYear == 0 || nMonth == 0 || nDay == 0) {
			return dtInvalid;
		}

		if (!bApprox) {
			long nHour = nLength >= 13 ? atoi(strDateTime.Mid(11, 2)) : 0;
			long nMin = nLength >= 16 ? atoi(strDateTime.Mid(14, 2)) : 0;
			long nSec = nLength >= 19 ? atoi(strDateTime.Mid(17, 2)) : 0;

			return COleDateTime(nYear, nMonth, nDay, nHour, nMin, nSec);
		} else {
			return COleDateTime(nYear, nMonth, nDay, 0, 0, 0);
		}
	}

	// (a.walling 2010-01-06 12:24) - PLID 36809 - Get CCR description info
	void GetCCRDescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime)
	{
		strSuggestedDescription = "CCR: ";

		pDocument->setProperty("SelectionNamespaces", "xmlns:ccr='urn:astm-org:CCR'");		

		// normally this is the patient name in CCD. We don't really need to bother with it here.
		//strTitle = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:title");

		strDisplay = GetTextFromXPath(pDocument, "//ccr:ContinuityOfCareRecord/ccr:Purpose/ccr:Description/ccr:Text");

		trTime.SetSingleTime(GetCCRDateTimeFromNode(pDocument->selectSingleNode("//ccr:ContinuityOfCareRecord/ccr:DateTime")));
		
		if (!strDisplay.IsEmpty()) {
			strSuggestedDescription += strDisplay;
		} else {
			strSuggestedDescription += strDefault;
		}
	}

	// (a.walling 2010-01-06 12:24) - PLID 36809 - Get CCD description info
	void GetCCDDescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime)
	{
		strSuggestedDescription = "CCD: ";

		pDocument->setProperty("SelectionNamespaces", "xmlns:n1='urn:hl7-org:v3'");		

		strTitle = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:title");
		strDisplay = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:code/@displayName");

		trTime.LoadFromElement(pDocument->selectSingleNode("//n1:ClinicalDocument/n1:effectiveTime"));

		if (!strTitle.IsEmpty()) {
			strSuggestedDescription += strTitle;

			if (!strDisplay.IsEmpty()) {
				strSuggestedDescription += " - ";
				strSuggestedDescription += strDisplay;
			}
		} else if (!strDisplay.IsEmpty()) {
			strSuggestedDescription += strDisplay;
		} else {
			strSuggestedDescription += strDefault;
		}
	}

	// (j.gruber 2013-11-08 11:01) - PLID 59376 - Get CCDA description info
	void GetCCDADescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime)
	{
		strSuggestedDescription = "CCDA: ";

		pDocument->setProperty("SelectionNamespaces", "xmlns:n1='urn:hl7-org:v3'");		

		strTitle = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:title");
		strDisplay = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:code/@displayName");

		trTime.LoadFromElement(pDocument->selectSingleNode("//n1:ClinicalDocument/n1:effectiveTime"));

		if (!strTitle.IsEmpty()) {
			strSuggestedDescription += strTitle;

			if (!strDisplay.IsEmpty()) {
				strSuggestedDescription += " - ";
				strSuggestedDescription += strDisplay;
			}
		} else if (!strDisplay.IsEmpty()) {
			strSuggestedDescription += strDisplay;
		} else {
			strSuggestedDescription += strDefault;
		}
	}

	// (j.gruber 2013-11-08 10:20) - PLID 59376 - Review and match CCDA patient		
	/*long ReviewAndMatchCCDAPatient(const CString& strFile, long nCurPatientID, CWnd* pParentWnd) throw(...)
	{		
		MSXML2::IXMLDOMDocument2Ptr pDocument = NxXMLUtils::LoadXMLDocument(strFile);

		CArray<long, long> arPatientIDs;
		GetMatchingPatient(pDocument, arPatientIDs);
		{
			CReviewCCDDlg dlg(pParentWnd);

			// (a.walling 2010-01-06 14:05) - PLID 36809 - Pass in the document itself
			dlg.m_pPendingStream = GetOutputHTMLStream(pDocument);
			if (dlg.m_pPendingStream != NULL) {
				if (arPatientIDs.GetSize() > 0) {
					dlg.m_nPatientID = arPatientIDs[0];
				}

				dlg.m_nCurrentPatientID = nCurPatientID;

				if (IDOK == dlg.DoModal()) {
					return dlg.m_nPatientID;
				} else {
					return -1;
				}
			} else {
				return -1;
			}
		}

		return -1;
	}*/

	// (a.walling 2009-05-13 12:34) - PLID 34243 - Review the CCD and choose a matching patient
	long ReviewAndMatchPatient(const CString& strFile, long nCurPatientID, CWnd* pParentWnd) throw(...)
	{
		// (j.jones 2010-06-30 10:58) - PLID 38031 - LoadXMLDocument has been moved to the NxXMLUtils namespace
		MSXML2::IXMLDOMDocument2Ptr pDocument = NxXMLUtils::LoadXMLDocument(strFile);

		CArray<long, long> arPatientIDs;
		GetMatchingPatient(pDocument, arPatientIDs);

		{
			CReviewCCDDlg dlg(pParentWnd);

			// (a.walling 2010-01-06 14:05) - PLID 36809 - Pass in the document itself
			dlg.m_pPendingStream = GetOutputHTMLStream(pDocument);
			if (dlg.m_pPendingStream != NULL) {
				if (arPatientIDs.GetSize() > 0) {
					dlg.m_nPatientID = arPatientIDs[0];
				}

				dlg.m_nCurrentPatientID = nCurPatientID;

				if (IDOK == dlg.DoModal()) {
					return dlg.m_nPatientID;
				} else {
					return -1;
				}
			} else {
				return -1;
			}
		}

		return -1;
	}

	
	IMPLEMENT_DYNAMIC(CReviewCCDDlg, CGenericBrowserDlg)

	LRESULT CReviewCCDDlg::OnLabelClicked(WPARAM wParam, LPARAM lParam)
	{
		try {
			CSelectDlg dlg(this);
			dlg.m_strTitle = "Select a patient";
			dlg.m_strCaption = "Select a patient to associate with this document";
			dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
			dlg.m_strWhereClause = "PersonT.Archived = 0 AND PersonT.ID >= 0 AND CurrentStatus <> 4";
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			dlg.AddColumn("Last", "Last", TRUE, FALSE, TRUE);
			dlg.AddColumn("First", "First", TRUE, FALSE, TRUE);
			dlg.AddColumn("Middle", "Middle", TRUE, FALSE, TRUE);
			dlg.AddColumn("UserDefinedID", "PatientID", TRUE, FALSE, TRUE);
			dlg.AddColumn("BirthDate", "Birth Date", TRUE, FALSE, TRUE);
			dlg.AddColumn("(CASE WHEN Gender = 2 THEN 'F' WHEN Gender = 1 THEN 'M' END)", "Gender", TRUE, FALSE, TRUE);
			dlg.AddColumn("Address1", "Address1", TRUE, FALSE, TRUE);
			dlg.AddColumn("Address2", "Address2", TRUE, FALSE, TRUE);
			dlg.AddColumn("City", "City", TRUE, FALSE, TRUE);
			dlg.AddColumn("State", "State", TRUE, FALSE, TRUE);
			dlg.AddColumn("Zip", "Zip", TRUE, FALSE, TRUE);

			if (m_nPatientID != -1) {
				dlg.SetPreSelectedID(0, m_nPatientID);
			} else if (m_nCurrentPatientID != -1) {
				dlg.SetPreSelectedID(0, m_nCurrentPatientID);
			}
			if(dlg.DoModal() == IDOK) {
				m_nPatientID = VarLong(dlg.m_arSelectedValues[0], -1);
				UpdateControls();
			}
		} NxCatchAll("Error selecting patient for document");
		return 0;
	}

	void CReviewCCDDlg::InitializeControls()
	{
		m_nxibCancel.SetWindowText("&Cancel");

		m_nxibOK.AutoSet(NXB_OK);
		m_nxibCancel.AutoSet(NXB_CANCEL);

		m_nxlabelCaption.SetType(dtsHyperlink);
		UpdateControls();
	}

	void CReviewCCDDlg::UpdateControls()
	{
		m_nxibOK.EnableWindow(m_nPatientID == -1 ? FALSE : TRUE);
	
		m_nxlabelCaption.SetText(GetCaption());
	}
	
	CString CReviewCCDDlg::GetCaption()
	{
		if (m_nPatientID == -1) {
			return "This document could not be automatically matched with a patient.";
		} else {
			CString strMessage;
			strMessage.Format("Document for patient: \t%s", GetExistingPatientName(m_nPatientID));

			return strMessage;
		}
	}

	void CReviewCCDDlg::OnCancel()
	{
		try {
			if (m_nPatientID == -1) {
				if (IDYES == MessageBox("No patient is associated with this document. Without a patient, this document cannot be imported. Do you want to continue?", NULL, MB_ICONSTOP|MB_YESNO)) {
					CGenericBrowserDlg::OnCancel();
				} else {
					// don't cancel
				}
			} else {
				CGenericBrowserDlg::OnCancel();
			}
		} NxCatchAll(__FUNCTION__);
	}

	// (a.walling 2009-05-13 08:41) - PLID 34243 - Get matching patient from CCD information
	// we will match only on patient first and last name, gender (optionally), and birthdate.
	void GetMatchingPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs)
	{	
		// (a.walling 2010-01-06 12:20) - PLID 36809 - Handle this for CCR documents as well
		// (j.jones 2010-06-30 10:57) - PLID 38031 - GetXMLDocumentType is now
		// part of NxXMLUtils, not part of the CCD namespace
		// (j.gruber 2013-11-08 10:22) - PLID 59376 - added CCDA finding
		NxXMLUtils::EDocumentType documentType = NxXMLUtils::GetXMLDocumentType(pDocument);

		if (documentType == NxXMLUtils::CCD_Document) {
			GetMatchingCCDPatient(pDocument, arPatientIDs);
		} else if (documentType == NxXMLUtils::CCR_Document) {
			GetMatchingCCRPatient(pDocument, arPatientIDs);
		} else if (documentType == NxXMLUtils::CCDA_Document) {
			GetMatchingCCDAPatient(pDocument, arPatientIDs);
		} else {
			ThrowNxException("This is not a valid CCDA, CCD, CDA, or CCR document.");
		}
	}
	
	// (a.walling 2010-01-06 13:03) - PLID 36809 - Get maching patient from given demographics
	void GetMatchingPatientFromDemographics(const CString& strFirst, const CString& strLast, const COleDateTime& dtBirthDate, long nGender, CArray<long, long>& arPatientIDs)
	{
// don't bother checking these if we don't even have a name
		if (!strFirst.IsEmpty() && !strLast.IsEmpty()) {
			// has birthdate and gender defined in document
			if (dtBirthDate.GetStatus() == COleDateTime::valid && nGender != -1) {
				ADODB::_RecordsetPtr prs = CreateParamRecordset(
					"SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"WHERE First = {STRING} AND Last = {STRING} AND (Gender = 0 OR Gender = {INT}) "
					"AND BirthDate = {OLEDATETIME} ORDER BY Gender DESC",
					strFirst, strLast, nGender, dtBirthDate
					);

				while (!prs->eof) {
					long nID = AdoFldLong(prs, "ID");
					arPatientIDs.Add(nID);
					prs->MoveNext();
				}
			}

			if (arPatientIDs.GetSize() > 0) return;

			// has birthdate defined in document
			if (dtBirthDate.GetStatus() == COleDateTime::valid) {
				ADODB::_RecordsetPtr prs = CreateParamRecordset(
					"SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"WHERE First = {STRING} AND Last = {STRING} "
					"AND BirthDate = {OLEDATETIME}",
					strFirst, strLast, dtBirthDate
					);

				while (!prs->eof) {
					long nID = AdoFldLong(prs, "ID");
					arPatientIDs.Add(nID);
					prs->MoveNext();
				}
			}			

			if (arPatientIDs.GetSize() > 0) return;

			// has no birthdate, and maybe a gender
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT ID, Gender FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE First = {STRING} AND Last = {STRING} ORDER BY Gender DESC",
				strFirst, strLast
				);

			while (!prs->eof) {
				long nPatientGender = AdoFldByte(prs, "Gender", 0);

				if (nGender == -1 || (nGender == nPatientGender) || (nPatientGender == 0)) {
					long nID = AdoFldLong(prs, "ID");
					arPatientIDs.Add(nID);
				}

				prs->MoveNext();
			}

			if (arPatientIDs.GetSize() > 0) return;
		}
	}

	// (a.walling 2010-01-06 13:03) - PLID 36809 - Get matching patient for CCR document
	void GetMatchingCCRPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs)
	{
		pDocument->setProperty("SelectionNamespaces", "xmlns:ccr='urn:astm-org:CCR'");

		CString strPatientActorID = GetTextFromXPath(pDocument, "//ccr:ContinuityOfCareRecord/ccr:Patient/ccr:ActorID");
		if (!strPatientActorID.IsEmpty()) {
			// I was using an XPath for this, but for some reason it always seems to have trouble handling child element value selectors with namespaces
			MSXML2::IXMLDOMNodeListPtr pActors = pDocument->selectNodes("//ccr:ContinuityOfCareRecord/ccr:Actors/ccr:Actor");

			if (pActors) {
				for (int i = 0; i < pActors->Getlength(); i++) {
					MSXML2::IXMLDOMNodePtr pActor = pActors->Getitem(i);

					CString strActorObjectID = GetXMLNodeText(pActor, "ActorObjectID");
					if (strActorObjectID.CompareNoCase(strPatientActorID) == 0) {
						CString strFirst = GetTextFromXPath(pActor, "ccr:Person/ccr:Name/ccr:CurrentName/ccr:Given");
						CString strLast = GetTextFromXPath(pActor, "ccr:Person/ccr:Name/ccr:CurrentName/ccr:Family");

						COleDateTime dtBirthDate = GetCCRDateTimeFromNode(pActor->selectSingleNode("ccr:Person/ccr:DateOfBirth"));

						long nGender = -1;
						// match only text, ignore SNOMED codes and etc
						CString strGender = GetTextFromXPath(pActor, "ccr:Person/ccr:Gender/ccr:Text");
						if (!strGender.IsEmpty()) {
							strGender.MakeUpper();
							if (strGender.GetAt(0) == 'M') {
								nGender = 1;				
							} else if (strGender.GetAt(0) == 'F') {
								nGender = 2;
							}
						}

						GetMatchingPatientFromDemographics(strFirst, strLast, dtBirthDate, nGender, arPatientIDs);
						return;
					}
				}
			}
		}
	}

	void GetMatchingCCDPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs)
	{
		pDocument->setProperty("SelectionNamespaces", "xmlns:n1='urn:hl7-org:v3'");
		// let's just gather all the data from the document that we will need.

		CString strFirst = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:name/n1:given");
		CString strLast = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:name/n1:family");

		CString strGender = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:administrativeGenderCode/@code");
		strGender.MakeUpper();
		long nGender = -1;
		if (strGender == "M") {
			nGender = 1;				
		} else if (strGender == "F") {
			nGender = 2;
		}
		// If nGender is -1, we'll ignore it. If the gender is undefined/0 in our database, we'll ignore it, too.

		CTimeRange trBirthDate;
		trBirthDate.LoadFromElement(pDocument->selectSingleNode("//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:birthTime"));
		COleDateTime dtBirthDate = trBirthDate.GetSingleTime();

		// we have IDs from both the patientRole and the healthcare provider; I am not sure if we will use either of these.
		//CString strPatientID = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:id/@n1:extension");
		//CString strPatientIDFromProvider = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:documentationOf/n1:serviceEvent/n1:performer/n1:assignedEntity/sdtc:patient/sdtc:id[@sdtc:extension='MedicalRecordNumber']/@n1:root");

		GetMatchingPatientFromDemographics(strFirst, strLast, dtBirthDate, nGender, arPatientIDs);
	}

	// (j.gruber 2013-11-08 10:22) - PLID 59376 - match CCDA patient
	void GetMatchingCCDAPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs)
	{	
		pDocument->setProperty("SelectionNamespaces", "xmlns:n1='urn:hl7-org:v3'");

		CString strFirst = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:name/n1:given");
		CString strLast = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:name/n1:family");

		CString strGender = GetTextFromXPath(pDocument, "//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:administrativeGenderCode/@code");
		strGender.MakeUpper();
		long nGender = -1;
		if (strGender == "M") {
			nGender = 1;				
		} else if (strGender == "F") {
			nGender = 2;
		}
		// If nGender is -1, we'll ignore it. If the gender is undefined/0 in our database, we'll ignore it, too.

		CTimeRange trBirthDate;
		trBirthDate.LoadFromElement(pDocument->selectSingleNode("//n1:ClinicalDocument/n1:recordTarget/n1:patientRole/n1:patient/n1:birthTime"));
		COleDateTime dtBirthDate = trBirthDate.GetSingleTime();
		
		GetMatchingPatientFromDemographics(strFirst, strLast, dtBirthDate, nGender, arPatientIDs);
	}

	// (j.jones 2010-06-30 11:41) - PLID 38031 - moved the GenericXMLBrowserDlg into its own class
	

	// (j.jones 2010-06-30 10:35) - PLID 38031 - moved a number of XML file validation functions
	// into NxXMLUtils
	// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs a patient ID
	void UpdateCCDInformation(const CString& strFile, long nMailID, long nPatientID) {
		// (a.walling 2009-05-12 11:38) - PLID 29924 - Call into the utilities in the CCD namespace
		CString strTitle, strDisplay;
		CTimeRange tr;

		CString strDescription;
		CString strDefault = "Imported";

		// (a.walling 2009-05-21 09:25) - PLID 34176 - Consolidated some CCD description functions
		GetDescriptionInfo(strFile, strDefault, strTitle, strDisplay, strDescription, tr);

		ExecuteParamSql("UPDATE MailSentNotesT SET Note = {STRING} WHERE MailID = {INT}", strDescription, nMailID);

		COleDateTime dtTime = tr.GetSingleTime();
		if (dtTime.GetStatus() == COleDateTime::valid) {
			ExecuteParamSql("UPDATE MailSent SET ServiceDate = {OLEDATETIME} WHERE MailID = {INT}", dtTime, nMailID);
		}

		// (j.jones 2014-08-04 13:31) - PLID 63157 - this now sends an Ex tablechecker, we know IsPhoto is always false here
		CClient::RefreshMailSentTable(nPatientID, nMailID);
	}

#pragma endregion

/*****************************************************/
}