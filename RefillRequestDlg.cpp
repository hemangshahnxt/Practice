// RefillRequestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "RefillRequestDlg.h"
#include "SOAPUtils.h"
#include "SureScriptsPractice.h"
#include "SelectDlg.h"
#include "PrescriptionEditDlg.h"
#include "DontShowDlg.h"
#include "MedicationSelectDlg.h"
#include "EditMedicationListDlg.h"
#include "PatientView.h"
#include "FirstDataBankUtils.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CRefillRequestDlg dialog
// (a.walling 2009-04-24 13:59) - PLID 33897 - Dialog to handle refill requests and responding to them

IMPLEMENT_DYNAMIC(CRefillRequestDlg, CNxDialog)

CRefillRequestDlg::CRefillRequestDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRefillRequestDlg::IDD, pParent)
{
	m_nPrescriptionID = -1;
	m_nMedicationID = -1;
	m_nPharmacyID = -1;
	m_nPrescriberID = -1;
	m_nLocationID = -1;
	m_nPatientID = -1;
	m_atAction = SureScripts::atInvalid;
	m_bUpdatePatientLink = false;
}

CRefillRequestDlg::~CRefillRequestDlg()
{
}

void CRefillRequestDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REFILL_MEDICATION_NAME_LBL, m_nxsMedicationName);
	DDX_Control(pDX, IDC_REFILL_NOTE_LBL, m_nxsNote);
	DDX_Control(pDX, IDC_REFILL_EXPLANATION_LBL, m_nxsExplanation);
	DDX_Control(pDX, IDC_REFILL_DENIAL_EXPLANATION_LBL, m_nxsDenialExplanation);
	DDX_Control(pDX, IDC_REFILL_DENIALCODE_LBL, m_nxsDenialCode);
	DDX_Control(pDX, IDC_REFILL_REFILLS_LBL, m_nxlabelRefills);
	DDX_Control(pDX, IDC_REFILL_NOTE, m_nxeditNote);
	DDX_Control(pDX, IDC_REFILL_REFILLS, m_nxeditRefills);
	DDX_Control(pDX, IDC_REFILL_APPROVE, m_nxibApprove);
	DDX_Control(pDX, IDC_REFILL_DENY, m_nxibDeny);
	DDX_Control(pDX, IDC_REFILL_DENY_SEND_NEW, m_nxibDenySendNew);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	DDX_Control(pDX, IDC_REFILL_NXCOLORCTRL, m_nxcolor);
	DDX_Control(pDX, IDC_REFILL_PRESCRIPTION_INFO_LBL, m_nxsPrescriptionInfo);
	DDX_Control(pDX, IDC_REFILL_VIEW_ORIGINAL, m_nxibViewOriginal);	
	DDX_Control(pDX, IDC_REFILL_PATIENT_LBL, m_nxsPatientInfo);
	DDX_Control(pDX, IDC_REFILL_PRESCRIBER_LBL, m_nxsPrescriberInfo);
	DDX_Control(pDX, IDC_REFILL_PHARMACY_LBL, m_nxsPharmacyInfo);
	DDX_Control(pDX, IDC_REFILL_ADD_PHARMACY, m_nxibAddPharmacy);
	DDX_Control(pDX, IDC_REFILL_FIND_PATIENT, m_nxibFindPatient);
	DDX_Control(pDX, IDC_REFILL_GROUP_REQUEST, m_nxgroupRequest);
	DDX_Control(pDX, IDC_REFILL_GROUP_RESPONSE, m_nxgroupResponse);
	DDX_Control(pDX, IDC_REFILL_FIND_MEDICATION, m_nxibFindMedication);
}


BEGIN_MESSAGE_MAP(CRefillRequestDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REFILL_APPROVE, &CRefillRequestDlg::OnBnClickedRefillApprove)
	ON_BN_CLICKED(IDC_REFILL_DENY, &CRefillRequestDlg::OnBnClickedRefillDeny)
	ON_BN_CLICKED(IDC_REFILL_DENY_SEND_NEW, &CRefillRequestDlg::OnBnClickedRefillDenySendNew)
	ON_BN_CLICKED(IDC_REFILL_VIEW_ORIGINAL, &CRefillRequestDlg::OnBnClickedRefillViewOriginal)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CRefillRequestDlg::OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_REFILL_FIND_PATIENT, &CRefillRequestDlg::OnBnClickedRefillFindPatient)
	ON_BN_CLICKED(IDC_REFILL_ADD_PHARMACY, &CRefillRequestDlg::OnBnClickedRefillAddPharmacy)
	ON_BN_CLICKED(IDC_REFILL_FIND_MEDICATION, &CRefillRequestDlg::OnBnClickedRefillFindMedication)
END_MESSAGE_MAP()


// CRefillRequestDlg message handlers

BOOL CRefillRequestDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		extern CPracticeApp theApp;
		m_nxsMedicationName.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_nxibApprove.AutoSet(NXB_OK);
		m_nxibDeny.AutoSet(NXB_CANCEL);
		m_nxibDenySendNew.AutoSet(NXB_NEW);
		m_nxibCancel.AutoSet(NXB_CLOSE);
		m_nxibViewOriginal.AutoSet(NXB_INSPECT);

		m_nxibFindPatient.AutoSet(NXB_INSPECT);
		m_nxibAddPharmacy.AutoSet(NXB_NEW);

		// (a.walling 2009-07-06 15:37) - PLID 34263
		m_nxibFindMedication.AutoSet(NXB_INSPECT);
		
		m_nxeditNote.SetLimitText(70);
		m_nxeditRefills.SetLimitText(3);

		m_pDenialCodes = BindNxDataList2Ctrl(IDC_REFILL_DENIALCODE_LIST, false);
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow;

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AA"));
			pRow->PutValue(1, _variant_t("Patient unknown to the Prescriber"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AB"));
			pRow->PutValue(1, _variant_t("Patient never under Prescriber care"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AC"));
			pRow->PutValue(1, _variant_t("Patient no longer under Prescriber care"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AD"));
			pRow->PutValue(1, _variant_t("Patient has requested refill too soon"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AE"));
			pRow->PutValue(1, _variant_t("Medication never prescribed for the patient"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AF"));
			pRow->PutValue(1, _variant_t("Patient should contact Prescriber first"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AG"));
			pRow->PutValue(1, _variant_t("Refill not appropriate"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AH"));
			pRow->PutValue(1, _variant_t("Patient has picked up prescription"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AJ"));
			pRow->PutValue(1, _variant_t("Patient has picked up partial fill of prescription"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AK"));
			pRow->PutValue(1, _variant_t("Patient has not picked up prescription, drug returned to stock"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AL"));
			pRow->PutValue(1, _variant_t("Change not appropriate"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AM"));
			pRow->PutValue(1, _variant_t("Patient needs appointment"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AN"));
			pRow->PutValue(1, _variant_t("Prescriber not associated with this practice or location."));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AO"));
			pRow->PutValue(1, _variant_t("No attempt will be made to obtain Prior Authorization."));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);

			pRow = m_pDenialCodes->GetNewRow();
			pRow->PutValue(0, _variant_t("AP"));
			pRow->PutValue(1, _variant_t("Request already responded to by other means (e.g. phone or fax)"));
			m_pDenialCodes->AddRowAtEnd(pRow, NULL);
		}

		m_nxlabelRefills.GetWindowRect(m_rcOriginalRefillRect);
		ScreenToClient(m_rcOriginalRefillRect);
		ToggleRefillAsNeeded(FALSE);

		m_nxlabelRefills.SetType(dtsHyperlink);
		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		LoadInfoFromMessage();
		UpdateControls();
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRefillRequestDlg::LoadInfoFromMessage()
{
	/*
	{
		MSXML2::IXMLDOMDocument2Ptr xmlMessage(__uuidof(MSXML2::DOMDocument60));
		xmlMessage->load("file:///C:/data/surescripts/example_refreq2.xml");
		m_pMessage = xmlMessage;
	}
	*/

	if (m_pMessage) {
		m_pMessage->setProperty("SelectionNamespaces", "xmlns:ss='http://www.surescripts.com/messaging'");

		// fill all the info fields
		{
			m_nxsMedicationName.SetWindowTextA(SureScripts::GetMedicationName(m_pMessage));
			m_nxsPatientInfo.SetWindowText("Patient: " + SureScripts::GetPatientInfo(m_pMessage));
			m_nxsPrescriberInfo.SetWindowText("Prescriber: " + SureScripts::GetPrescriberInfo(m_pMessage));
			m_nxsPharmacyInfo.SetWindowText("Pharmacy: " + SureScripts::GetPharmacyInfo(m_pMessage));

			{
				CString strRefillsRequestedQualifier = GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Refills/ss:Qualifier");
				if (strRefillsRequestedQualifier.CompareNoCase("PRN") == 0) {
					m_nxeditRefills.SetWindowText("As Needed");
					m_strOriginalRefillRequested = "As Needed";
					ToggleRefillAsNeeded(TRUE);
				} else {
					CString strRefillsRequestedQuantity = GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Refills/ss:Quantity");
					if (strRefillsRequestedQuantity.IsEmpty() || strRefillsRequestedQuantity == "0") {
						m_nxeditRefills.SetWindowText("");
						m_strOriginalRefillRequested.Empty();
					} else {
						m_nxeditRefills.SetWindowText(strRefillsRequestedQuantity);
						m_strOriginalRefillRequested = strRefillsRequestedQuantity;
					}
					
					ToggleRefillAsNeeded(FALSE);
				}
			}

			m_strRxReferenceNumber = GetTextFromXPath(m_pMessage, "//ss:RefillRequest/ss:RxReferenceNumber");
			m_strPatientExplanation = GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Directions");

			//TES 8/10/2009 - PLID 35131 - Pull the string that will display information about the refill in the
			// "Information" box.
			m_strMedicationInfo = "Medication: " + SureScripts::GetMedicationName(m_pMessage) + "\r\n";
			m_strMedicationInfo += "SIG: " + GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Directions") + "\r\n";
			m_strMedicationInfo += "Quantity: " + GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Quantity/ss:Value") + "\r\n";
			CString strUnits = GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Quantity/ss:Qualifier") + "\r\n";
			strUnits.TrimRight();
			ADODB::_RecordsetPtr rsUnit = CreateParamRecordset("SELECT Name FROM DrugStrengthUnitsT WHERE SureScriptsCode = {STRING}", strUnits);
			if(!rsUnit->eof) {
				strUnits = AdoFldString(rsUnit, "Name", "");
			}
			rsUnit->Close();
			m_strMedicationInfo += "Units: " + strUnits + "\r\n";
			CString strSubstitutions = GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Substitutions");
			m_strMedicationInfo += "Substitutions: " + SureScripts::GetSubstitutionsDescription(strSubstitutions) + "\r\n";
			m_strMedicationInfo += "Refills: " + m_strOriginalRefillRequested + "\r\n";
			CString strLastFilled = GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:LastFillDate");
			m_strMedicationInfo += "Last Filled On: " + CString(strLastFilled.IsEmpty() ? "" : strLastFilled.Mid(2,2) + "/" + strLastFilled.Right(2) + "/" + strLastFilled.Left(4) ) + "\r\n";
			m_strMedicationInfo += "Notes: " + GetTextFromXPath(m_pMessage, "//ss:MedicationPrescribed/ss:Note") + "\r\n";

		}

		// first see if we have a prescriberordernumber
		MSXML2::IXMLDOMNodePtr pPrescriberOrderNumberNode = m_pMessage->selectSingleNode("//ss:RefillRequest/ss:PrescriberOrderNumber");
		
		if (pPrescriberOrderNumberNode) {
			m_strPrescriberOrderNumber = (LPCTSTR)pPrescriberOrderNumberNode->text;			
		}

		if (!m_strPrescriberOrderNumber.IsEmpty()) {
			ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ID, PatientID, MedicationID, ProviderID FROM PatientMedications WHERE UID = {STRING}", FormatGUIDAsStringForSql(m_strPrescriberOrderNumber));

			if (!rs->eof) {
				m_nPrescriptionID = AdoFldLong(rs, "ID", -1);
				m_nPatientID = AdoFldLong(rs, "PatientID", -1);
				m_nMedicationID = AdoFldLong(rs, "MedicationID", -1);
				m_nPrescriberID = AdoFldLong(rs, "ProviderID", -1);
			} else {
				// invalid Prescriber order number!!
				m_strPrescriberOrderNumber.Empty();
			}
		}

		if (m_strPrescriberOrderNumber.IsEmpty()) {
			// now what? this must not have been a NewRx. Perhaps a refill request to a paper prescription?
			// we can try to match.

			// let's do the hard part first -- match the patient.
			if (m_nPatientID == -1) {
				//TES 4/20/2009 - PLID 33901 - This function now takes a Node, not a Document.
				MSXML2::IXMLDOMNodePtr pMessage = FindChildNode(m_pMessage, "Message");
				m_nPatientID = SureScripts::FindMatchingPatient(GetRemoteData(), pMessage);
			}
			
			// and the medication
			if (m_nMedicationID == -1) {
				m_nMedicationID = SureScripts::FindMatchingMedication(GetRemoteData(), m_pMessage);
			}

			// now match the prescriber (should be easy, via SPI)
			if (m_nPrescriberID == -1) {
				m_nPrescriberID = SureScripts::FindMatchingPrescriber(GetRemoteData(), m_pMessage);
			}

			// maybe try to find a prescription via date? one that was not sent in a NewRX message?
			if (m_nPatientID != -1 && m_nMedicationID != -1 && m_nPrescriberID != -1) {
				ADODB::_RecordsetPtr rs = CreateParamRecordset(
					"SELECT TOP 1 ID, REPLACE(CONVERT(nvarchar(40), PatientMedications.UID), '-', '') AS PatientMedicationUID FROM PatientMedications WHERE PatientID = {INT} AND MedicationID = {INT} AND ProviderID = {INT} ORDER BY PrescriptionDate DESC, ID DESC",
					m_nPatientID, m_nMedicationID, m_nPrescriberID);
				if (!rs->eof) {
					m_nPrescriptionID = AdoFldLong(rs, "ID", -1);
					m_strPrescriberOrderNumber = AdoFldString(rs, "PatientMedicationUID", "");
				}
			}
		}

		// (a.walling 2009-07-08 08:31) - PLID 34261 - Get the DEA Schedule from the medication
		if (m_nMedicationID != -1) {
			m_strDEASchedule.Empty();
			ADODB::_RecordsetPtr prsSchedule = CreateParamRecordset("SELECT DEASchedule FROM DrugList WHERE ID = {INT}", m_nMedicationID);
			if (!prsSchedule->eof) {
				//TES 11/18/2009 - PLID 36360 - Determine which DEA Schedule to use.
				m_strDEASchedule = FirstDataBank::CalcDEASchedule(AdoFldString(prsSchedule, "DEASchedule", ""), m_nMedicationID);
			}
		}
		
		// match the pharmacy (NCPDP)
		if (m_nPharmacyID == -1 && m_strPharmacyNCPDPID.IsEmpty()) {
			long nPharmacyID = -1;
			CString strPharmacyNCPDPID;
			if (SureScripts::FindMatchingPharmacy(GetRemoteData(), m_pMessage, nPharmacyID, strPharmacyNCPDPID)) {
				m_nPharmacyID = nPharmacyID; // might be -1 for pharmacy in directory but not db
				m_strPharmacyNCPDPID = strPharmacyNCPDPID;
			}
		}
	} else {
		// error!
		ASSERT(FALSE);
		ThrowNxException("No refill request message available");
	}	
}

void CRefillRequestDlg::UpdateControls()
{
	CString strText;

	BOOL bHasAccess = TRUE;
	if (!CheckCurrentUserPermissions(bioPatientMedication, sptCreate, FALSE, 0, TRUE)) {
		bHasAccess = FALSE;
	}
	if (m_nPatientID == -1) {
		// no patient
		strText += "The patient cannot be found. To approve this refill, you must choose a patient.\r\n\r\n";
	}

	if (m_nMedicationID == -1) {
		// can't find the med
		strText += "The requested medication cannot be found. This refill cannot be approved until this medication exists in the database.\r\n\r\n";
	}

	if (m_nPrescriberID == -1) {
		// can't find the prescriber
		strText += "The prescriber cannot be found. This refill cannot be approved unless the prescriber exists in the system.\r\n\r\n";
	}

	if (m_nPrescriptionID == -1) {
		// we don't have an existing prescription ID
		if (m_strPrescriberOrderNumber.IsEmpty()) {
			strText += "The original prescription cannot be found.\r\n\r\n";
		} else {
			strText += "Received an invalid or unknown prescription ID.\r\n\r\n";
		}
	}

	if (m_nPharmacyID == -1) {
		if (m_strPharmacyNCPDPID.IsEmpty()) {
			strText += "The pharmacy is not in your locations. To approve this refill, you must choose 'Add' to insert this pharmacy into the system.\r\n\r\n";
		} else {
			strText += "The pharmacy could not be found in the directory. This refill can still be approved. Choose 'Add' to insert this pharmacy into the system.\r\n\r\n";
		}
	}

	// we can always deny
	BOOL bApprove = FALSE;
	BOOL bDenySendNew = FALSE;


	if (m_nPatientID != -1 && m_nMedicationID != -1 && m_nPharmacyID != -1 && m_nPrescriberID != -1) {
		// we can approve, and deny, and deny and send new
		bApprove = TRUE;
		bDenySendNew = TRUE;
	} else {
		// we can still deny
		if (m_nPatientID != -1 && m_nPrescriberID != -1 && m_nPharmacyID != -1) {
			// we can deny and send a new one
			bDenySendNew = TRUE;
		} else {
			// we can only deny
		}
	}
	
	// (a.walling 2009-07-08 08:38) - PLID 34261 - Give info about scheduled medication and refills
	if (m_nMedicationID != -1 && !m_strDEASchedule.IsEmpty()) {
		if (m_strDEASchedule == "I") {
			strText += "Schedule I medications cannot be prescribed.\r\n\r\n";
			bApprove = FALSE;
		} else if (m_strDEASchedule == "II") {				
			strText += "Current DEA regulations prohibit refills for Schedule II medications.\r\n\r\n";
			bApprove = FALSE;
		} else {		
			strText += "Refills for scheduled medications must be printed and faxed to the pharmacy, or given to the patient. Approving this refill will actually send a 'Denied, new prescription to follow' response to the pharmacy. Please ensure the note instructs the pharmacy to expect the prescription via fax or patient.\r\n\r\n";
			// they can approve, but it cannot be e-prescribed.
			// Basically we need to send a 'DenySendNew' message that says we will fax the refill over as a new prescription.

			CString strNoteToPharmacist;
			m_nxeditNote.GetWindowText(strNoteToPharmacist);

			if (strNoteToPharmacist.IsEmpty()) {
				strNoteToPharmacist = "A prescription for the controlled substance will be faxed.";

				m_nxeditNote.SetWindowText(strNoteToPharmacist);
			}
		}
	}

	if (!bHasAccess) {
		bApprove = FALSE;
		bDenySendNew = FALSE;

		m_nxibDeny.EnableWindow(FALSE);
		m_nxeditNote.EnableWindow(FALSE);
		m_pDenialCodes->PutReadOnly(VARIANT_TRUE);
	}

	m_nxlabelRefills.SetType(bApprove ? dtsHyperlink : dtsDisabledHyperlink);
	{		
		CRect Rect;
		m_nxlabelRefills.GetWindowRect(&Rect);
		ScreenToClient(&Rect);
		RedrawWindow(Rect, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);
	}
	m_nxeditRefills.EnableWindow(bApprove);
	m_nxsExplanation.ShowWindow(bApprove ? SW_SHOW : SW_HIDE);
	m_nxibApprove.EnableWindow(bApprove);

	m_nxibDenySendNew.EnableWindow(bDenySendNew);

	m_nxibAddPharmacy.EnableWindow(m_nPharmacyID == -1 ? TRUE : FALSE);
	m_nxibViewOriginal.EnableWindow((m_nPrescriptionID != -1 && m_nPatientID != -1) ? TRUE : FALSE);
	m_nxibFindPatient.EnableWindow(m_nPrescriptionID == -1 ? TRUE : FALSE);
	
	// (a.walling 2009-07-06 15:40) - PLID 34263
	m_nxibFindMedication.EnableWindow(m_nPrescriptionID == -1 ? TRUE : FALSE);

	if (bApprove) {
		strText += "Please approve or deny this refill request.";
	} else if (bDenySendNew) {
		strText += "This refill request must be denied, or denied with a new prescription following.";
	} else if (!bHasAccess) {
		strText += "You do not have permission to respond to this message. Please contact your administrator.";
	} else {
		strText += "This refill request must be denied.";
	}

	//TES 8/10/2009 - PLID 35131 - Include information about the medication being refilled.
	m_nxsPrescriptionInfo.SetWindowText(m_strMedicationInfo + "\r\n" + strText);
}

void CRefillRequestDlg::OnBnClickedRefillApprove()
{
	try {
		// (a.walling 2009-07-08 08:50) - PLID 34261 - Approving a scheduled med is effectively a DenySendNew action
		if (!m_strDEASchedule.IsEmpty()) {
			DenySendNew(TRUE);
			return;
		}

		ASSERT(m_nPatientID != -1 && m_nMedicationID != -1 && m_nPharmacyID != -1 && m_nPrescriberID != -1);

		if (m_nPatientID != -1 && m_nMedicationID != -1 && m_nPharmacyID != -1 && m_nPrescriberID != -1) {		
			
			CString strRefillQuantity;
			if (m_nxlabelRefills.GetText() == "Refill As Needed") {
				strRefillQuantity = "As Needed";
			} else {
				m_nxeditRefills.GetWindowText(strRefillQuantity);
			}
			if (strRefillQuantity.IsEmpty() || strRefillQuantity == "0" || (strRefillQuantity != "As Needed" && atoi(strRefillQuantity) == 0)) {
				MessageBox("You must enter a refill quantity to approve!", NULL, MB_ICONSTOP);
				return;
			}
			//TES 8/7/2009 - PLID 35127 - Make sure the refill quantity is a positive integer.
			else if(strRefillQuantity != "As Needed" && strRefillQuantity.SpanIncluding("1234567890").GetLength() != strRefillQuantity.GetLength()) {
				MessageBox("The refill quantity must be either \"As Needed,\" or a positive whole number (not a fraction).", NULL, MB_ICONSTOP);
				return;
			}

			BOOL bChanged = FALSE;
			// it is changed only if the refill quantity does not match the original one passed to us,
			// but if the pharmacy sends up a blank, 0, or PRN (as needed) amount initially, it is not considered 'changed';
			// we are expected to give them an amount in response.
			if (!m_strOriginalRefillRequested.IsEmpty() && (m_strOriginalRefillRequested != "As Needed") && (strRefillQuantity != m_strOriginalRefillRequested)) {
				bChanged = TRUE;
			}

			CString strNoteToPharmacist;
			m_nxeditNote.GetWindowText(strNoteToPharmacist);

			// (a.walling 2009-04-15 13:42) - PLID 33897 - Approve the refill, creating a new rx to save to our data as well.
			CPrescriptionEditDlg dlg(this);

			// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we created or deleted a prescription
//			PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.CreateNewRefill_FromRequest(m_pMessage, bChanged, m_strPatientExplanation, strNoteToPharmacist, strRefillQuantity, m_nPatientID, GetExistingPatientName(m_nPatientID), m_nPrescriberID, m_nMedicationID, m_nPharmacyID, FALSE);
		//	if(epdrvReturn != epdrvDeleteRx) {
		//		m_atAction = SureScripts::atRefillRequestApproved;
		//		//TES 4/24/2009 - PLID 33901 - We've linked the patient now, make sure that link persists.
		//		UpdatePatientLink();
		//		
		//		OnOK();
		//		return;
		//	}
		}
	} NxCatchAll(__FUNCTION__);
}

void CRefillRequestDlg::OnBnClickedRefillDeny()
{	
	try {
		CString strNoteToPharmacist;
		m_nxeditNote.GetWindowText(strNoteToPharmacist);

		CString strDenialCode;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDenialCodes->GetCurSel();
		if (pRow) {
			strDenialCode = VarString(pRow->GetValue(0), "");
		}

		if (strDenialCode.IsEmpty()) {
			MessageBox("You must choose a denial code!", NULL, MB_ICONSTOP);
			return;
		}

		if (IDYES != MessageBox("Are you sure you want to deny this refill?", NULL, MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		// (a.walling 2009-04-24 10:31) - PLID 34033
		CString strSureScriptsValidationError;
		try {
			SureScripts::Messages::RefillResponseMessage refillResponse;
			// (a.walling 2009-04-24 09:22) - PLID 33897 - Send in the patient ID
			SureScripts::GenerateDeniedRefillResponseFromMessage(GetRemoteData(), m_pMessage, FALSE, strDenialCode, strNoteToPharmacist, m_nPatientID, refillResponse);
			SureScripts::QueueOutgoingMessage(GetRemoteData(), refillResponse, m_nPatientID, TRUE, -1);

			m_atAction = SureScripts::atRefillRequestDenied;
			//TES 4/24/2009 - PLID 33901 - We've linked the patient now, make sure that link persists.
			UpdatePatientLink();

			OnOK();
			return;
		} catch(CNxValidationException* e) {
			// (a.walling 2009-04-23 17:19) - PLID 34033
			strSureScriptsValidationError = e->m_strDescription;
			e->Delete();
		}

		if (!strSureScriptsValidationError.IsEmpty()) {
			MessageBox(FormatString(
				"The denial could not be sent due to the following error:\r\n\r\n%s", strSureScriptsValidationError
			), NULL, MB_ICONINFORMATION);
		}

	} NxCatchAll(__FUNCTION__);
}

void CRefillRequestDlg::OnBnClickedRefillDenySendNew()
{
	try {
		DenySendNew(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CRefillRequestDlg::DenySendNew(BOOL bApprovingScheduled)
{
	long nMedicationID = -1;
	
	CString strRefillQuantity;

	if (bApprovingScheduled) {
		nMedicationID = m_nMedicationID;

		// (a.walling 2009-07-24 13:41) - PLID 34261 - Prepare the refills if we are approving a scheduled med
		if (m_nxlabelRefills.GetText() == "Refill As Needed") {
			strRefillQuantity = "As Needed";
		} else {
			m_nxeditRefills.GetWindowText(strRefillQuantity);
		}

		if (strRefillQuantity.IsEmpty() || strRefillQuantity == "0" || (strRefillQuantity != "As Needed" && atoi(strRefillQuantity) == 0)) {
			strRefillQuantity.Empty();
		}
	} else {
		nMedicationID = GetNewMedication();
	}

	if (nMedicationID != -1) {
		CString strNoteToPharmacist;
		m_nxeditNote.GetWindowText(strNoteToPharmacist);

		// (a.walling 2009-04-24 10:31) - PLID 34033
		CString strSureScriptsValidationError;
		try {
			SureScripts::Messages::RefillResponseMessage refillResponse;
			SureScripts::GenerateDeniedRefillResponseFromMessage(GetRemoteData(), m_pMessage, TRUE, "", strNoteToPharmacist, m_nPatientID, refillResponse);

			// (a.walling 2009-04-15 13:42) - PLID 33897 - Create new prescription tied into this one via the RxReferenceNumber
			CPrescriptionEditDlg dlg(this);

			// (a.walling 2009-07-24 13:41) - PLID 34261 - Pass in the refills if we are approving a scheduled med
			// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we created or deleted a prescription
//			PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.CreateNewPrescription_FromRequest(m_pMessage, refillResponse, m_strRxReferenceNumber, strRefillQuantity, m_nPatientID, GetExistingPatientName(m_nPatientID), m_nPrescriberID, nMedicationID, m_nPharmacyID, FALSE);
			//if(epdrvReturn != epdrvDeleteRx) {
			//	m_atAction = SureScripts::atRefillRequestDeniedNewRx;
			//	//TES 4/24/2009 - PLID 33901 - We've linked the patient now, make sure that link persists.
			//	UpdatePatientLink();
			//	
			//	OnOK();	
			//	return;
			//}
		} catch(CNxValidationException* e) {
			// (a.walling 2009-04-23 17:19) - PLID 34033
			strSureScriptsValidationError = e->m_strDescription;
			e->Delete();
		}

		if (!strSureScriptsValidationError.IsEmpty()) {
			MessageBox(FormatString(
				"The denial response could not be sent due to the following error:\r\n\r\n%s", strSureScriptsValidationError
			), NULL, MB_ICONINFORMATION);
			return;
		}
	}

}

// this should really be a shared utility function, but i'm already running out of time
long CRefillRequestDlg::GetNewMedication()
{
	CString sql, description;
	long nMedicationID = -1;
	
	try {
		//check to see that they have permissions - this will prompt a password
		if(!CheckCurrentUserPermissions(bioPatientMedication, sptCreate)) {
			return -1;
		}

		//TES 5/19/2008 - PLID 28523 - We will pop out a context menu with their "Quick List" medications.
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		//TES 5/19/2008 - PLID 28523 - Pull the Quick List
		CArray<int,int> aryQuickListMeds;
		GetRemotePropertyArray("MedicationsQuickList", aryQuickListMeds, 0, GetCurrentUserName());

		int nCmdId = 0;

		//TES 5/19/2008 - PLID 28523 - We need to look up the names.  Note that this will also filter out any bad ids that
		// are in the array (maybe one got deleted), and make sure that it's ordered properly on the menu.
		ADODB::_RecordsetPtr rsDrugList = CreateRecordset("SELECT DrugList.ID, EmrDataT.Data "
			"FROM DrugList INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID "
			"WHERE EMRDataT.Inactive = 0 AND DrugList.ID IN (%s) ORDER BY Data",
			ArrayAsString(aryQuickListMeds, false));
		if(rsDrugList->eof) {
			//TES 5/19/2008 - PLID 28523 - They don't have any in their quick list.  Therefore, we're not going to pop out
			// the menu, but just go with the old behavior (which, as it happens, is the same as they get if they click 
			// "<More Medications...>", which we are assigning ID -2.
			nCmdId = -2;
			
			//TES 5/19/2008 - PLID 28523 - However, let them know about this new feature, otherwise nobody might ever use it.
			// But give them the option to turn it off.
			DontShowMeAgain(this, "For your convenience, you can add commonly prescribed medications to your 'Quick List.'\r\n\r\n"
				"To do this, click the '...' button on the following dialog, and check off the 'Quick List' column for any "
				"medications you would like to have easy access to when prescribing.", "MedicationQuickListSetup");
			
		}
		else {
			//TES 5/19/2008 - PLID 28523 - Go through and add menu items for all the drugs in the quick list.
			while(!rsDrugList->eof) {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, AdoFldLong(rsDrugList, "ID"), 
					AsMenuItem(AdoFldString(rsDrugList, "Data")));
				rsDrugList->MoveNext();
			}

		
			//TES 5/19/2008 - PLID 28523 - Now add the "<More Medications...>" option.
			mnu.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -2, "<More Medications...>");
			//TES 5/19/2008 - PLID 28523 - And the "<Configure>" option, which takes them to the CEditMedicationsDlg.
			mnu.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
			//TES 5/19/2008 - PLID 28523 - Disable the menu option if they don't have permission.
			DWORD dwEnabled = MF_DISABLED|MF_GRAYED;
			if(GetCurrentUserPermissions(bioPatientMedication) & SPT______0_____ANDPASS) {
				dwEnabled = MF_ENABLED;
			}
			mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, -3, "<Configure...>");
			
			
			//TES 5/19/2008 - PLID 28523 - Now show the menu.
			CRect rc;
			m_nxibDenySendNew.GetWindowRect(rc);
			nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);			
		}

		if(nCmdId == 0) {
			//TES 5/19/2008 - PLID 28523 - They clicked off the menu, do nothing.
			return -1;
		}
		else if(nCmdId == -2) {
			//TES 5/19/2008 - PLID 28523 - They chose "<More Medications...>", just do it the old way.

			//Open the Medication Selection Dialog
			CMedicationSelectDlg dlg(this);
			// (j.jones 2008-05-20 15:39) - PLID 30079 - removed the todo creation code
			long nResult;
			nResult = dlg.DoModal();

			if (nResult == IDOK) {
				nMedicationID = dlg.m_nMedicationID;
			}
		}
		else if(nCmdId == -3) {
			//TES 5/19/2008 - PLID 28523 - They chose "<Configure>" so make sure they have permission.
			if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
				return -1;
			}

			//open the edit medication dialog

			CEditMedicationListDlg dlg(this);
			dlg.DoModal();
		}
		else {
			//TES 5/19/2008 - PLID 28523 - They chose one of the quick list medications.
			ASSERT(nCmdId > 0);
			if (nCmdId > 0) {
				nMedicationID = nCmdId;
			}
		}
	}NxCatchAll(__FUNCTION__);

	return nMedicationID;
}

void CRefillRequestDlg::OnCancel()
{
	try {
		CNxDialog::OnCancel();
	} NxCatchAll(__FUNCTION__);
}

void CRefillRequestDlg::OnOK()
{
	try {
		// (a.walling 2009-04-28 15:55) - PLID 33951 - Refresh the patients module if this is the appropriate patient
		if (m_atAction != SureScripts::atRefillRequestDenied && m_atAction != SureScripts::atInvalid && (m_nPatientID == -1 || m_nPatientID == GetActivePatientID())) {
			CNxView* pView = GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
			if (pView) {
				((CPatientView*)pView)->UpdateView();
			}
		}
	} NxCatchAll(__FUNCTION__);	

	CNxDialog::OnOK();
}


void CRefillRequestDlg::OnBnClickedRefillFindPatient()
{
	try {
		if (m_nPrescriptionID != -1) {
			ASSERT(FALSE);
			return;
		}

		if (m_nPatientID != -1) {
			if (IDYES != MessageBox("This refill request is already linked to a patient. Are you sure you want to find a different patient?", NULL, MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		CSelectDlg dlg(this);
		dlg.m_strTitle = "Select a patient";
		dlg.m_strCaption = "Select a patient to associate with this refill";
		dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
		// (a.walling 2009-04-28 12:29) - PLID 33951 - Exclude the -25 built-in patient, and inquiries
		dlg.m_strWhereClause = "PersonT.Archived = 0 AND PersonT.ID >= 0 AND CurrentStatus <> 4";
		// (a.walling 2009-04-28 12:32) - PLID 33951 - The CSelectDlg assigns sort priorities in order of visible columns added,
		// so I rearranged these a bit to sort by last, first, middle, ID, birthdate, gender, etc.
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
		}
		if(dlg.DoModal() == IDOK) {
			m_nPatientID = VarLong(dlg.m_arSelectedValues[0], -1);
			if(m_nPatientID != -1) {
				//TES 4/24/2009 - PLID 33901 - Remember that the patient has been manually linked, so that once this
				// response is committed, we can also persist the patient link for future messages.
				m_bUpdatePatientLink = true;
			}
			UpdateControls();
		}
	} NxCatchAll(__FUNCTION__);
}

void CRefillRequestDlg::OnBnClickedRefillAddPharmacy()
{
	try {
		if (m_nPharmacyID == -1 && m_strPharmacyNCPDPID.IsEmpty() && (IDOK == MessageBox("This pharmacy was not found in the pharmacy directory. Would you like to add it to your locations now?", NULL, MB_ICONQUESTION | MB_OKCANCEL))) {
			m_nPharmacyID = SureScripts::AddPharmacy(GetRemoteData(), m_pMessage);

			UpdateControls();
		} else if (m_nPharmacyID == -1 && !m_strPharmacyNCPDPID.IsEmpty() && (IDOK == MessageBox("This pharmacy exists in the pharmacy directory. Would you like to add it to your locations now?", NULL, MB_ICONQUESTION | MB_OKCANCEL))) {
			// we could add the pharmacy directly from the directory, but this works just as well for now. It will be
			// updated the next time the directory is updated, anyway.
			//m_nPharmacyID = SureScripts::AddPharmacyFromDirectory(GetRemoteData(), m_pMessage);
			m_nPharmacyID = SureScripts::AddPharmacy(GetRemoteData(), m_pMessage);

			UpdateControls();
		}
	} NxCatchAll(__FUNCTION__);
}


void CRefillRequestDlg::OnBnClickedRefillViewOriginal()
{
	try {
		if (m_nPatientID != -1 && m_nPrescriptionID != -1) {
			CPrescriptionEditDlg dlg(this);

			// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription,
			// though the result is not actually used here
			// (j.fouts 2013-03-12 10:18) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
			// (j.jones 2013-11-25 09:55) - PLID 59772 - this does not need the drug interaction info
			// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
			PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(false, LoadFullPrescription(m_nPrescriptionID), NULL, TRUE);
		} else {
			ASSERT(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

LRESULT CRefillRequestDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_nxlabelRefills.GetText() == "Refills:") {
			ToggleRefillAsNeeded(TRUE);
		} else {
			ToggleRefillAsNeeded(FALSE);
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

BOOL CRefillRequestDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		// (a.walling 2009-04-06 16:35) - PLID 33870 - Same with refills
		CRect rcRefills;
		m_nxlabelRefills.GetWindowRect(rcRefills);
		ScreenToClient(&rcRefills);

		if (rcRefills.PtInRect(pt) && m_nxlabelRefills.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2009-04-06 16:44) - PLID 33870 - Toggle interface for 'Refill As Needed'
void CRefillRequestDlg::ToggleRefillAsNeeded(BOOL bRefillAsNeeded)
{
	if (bRefillAsNeeded) {
		m_nxlabelRefills.SetHzAlign(DT_CENTER);
		m_nxlabelRefills.SetText("Refill As Needed");

		CRect rc;
		m_nxeditRefills.GetWindowRect(rc);

		CRect rcLabel;
		m_nxlabelRefills.GetWindowRect(rcLabel);

		rcLabel.right = rc.right;

		ScreenToClient(rcLabel);

		m_nxeditRefills.ShowWindow(SW_HIDE);
		m_nxlabelRefills.MoveWindow(rcLabel);
	} else {
		m_nxlabelRefills.SetHzAlign(DT_LEFT);
		m_nxlabelRefills.SetText("Refills:");

		if (!m_rcOriginalRefillRect.IsRectEmpty()) {
			m_nxlabelRefills.MoveWindow(m_rcOriginalRefillRect);
		}

		m_nxeditRefills.ShowWindow(SW_SHOW);
	}
}

void CRefillRequestDlg::UpdatePatientLink()
{
	if(m_bUpdatePatientLink) {
		//TES 4/24/2009 - PLID 33901 - Pull the two IDs we trust (MutuallyDefined and FileID) out of the Identification,
		// and tie them to our patient ID (unless those IDs are already tied to a patient).
		MSXML2::IXMLDOMNodePtr pID = m_pMessage->selectSingleNode("//ss:Patient/ss:Identification/ss:MutuallyDefined");
		if(pID) {
			ExecuteParamSql("IF NOT EXISTS (SELECT PatientID FROM SureScriptsPatientLinkT WHERE IDType = 'MutuallyDefined' "
				"AND SureScriptsID = {STRING}) "
				"BEGIN "
				"	INSERT INTO SureScriptsPatientLinkT (IDType, SureScriptsID, PatientID) "
				"	VALUES ('MutuallyDefined', {STRING}, {INT}) "
				"END", CString((LPCTSTR)pID->text), CString((LPCTSTR)pID->text), m_nPatientID);
		}
		pID = m_pMessage->selectSingleNode("//ss:Patient/ss:Identification/ss:FileID");
		if(pID) {
			ExecuteParamSql("IF NOT EXISTS (SELECT PatientID FROM SureScriptsPatientLinkT WHERE IDType = 'FileID' "
				"AND SureScriptsID = {STRING}) "
				"BEGIN "
				"	INSERT INTO SureScriptsPatientLinkT (IDType, SureScriptsID, PatientID) "
				"	VALUES ('FileID', {STRING}, {INT}) "
				"END", CString((LPCTSTR)pID->text), CString((LPCTSTR)pID->text), m_nPatientID);
		}
	}
}

// (a.walling 2009-07-06 15:40) - PLID 34263 - Find the medication
void CRefillRequestDlg::OnBnClickedRefillFindMedication()
{
	try {
		if (m_nPrescriptionID != -1) {
			ASSERT(FALSE);
			return;
		}

		CMedicationSelectDlg dlg(this);

		if (IDOK == dlg.DoModal()) {
			m_nMedicationID = dlg.m_nMedicationID;

			m_strDEASchedule.Empty();			
			// (a.walling 2009-07-08 08:31) - PLID 34261 - Get the DEA Schedule from the medication
			if (m_nMedicationID != -1) {
				ADODB::_RecordsetPtr prsSchedule = CreateParamRecordset("SELECT DEASchedule FROM DrugList WHERE ID = {INT}", m_nMedicationID);
				if (!prsSchedule->eof) {
					//TES 11/18/2009 - PLID 36360 - Determine which DEA Schedule to use.
					m_strDEASchedule = FirstDataBank::CalcDEASchedule(AdoFldString(prsSchedule, "DEASchedule", ""), m_nMedicationID);
				}
			}

			UpdateControls();
		}
	} NxCatchAll(__FUNCTION__);
}
