// MedicationHistoryMedInfoPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "MedicationHistoryMedInfoPopupDlg.h"
#include "PatientsRc.h"
#include "GenericBrowserDlg.h"
#include "GlobalDataUtils.h"
#include <NxDataUtilitiesLib/NxStream.h>

class CMedicationHistoryMedInfoPopupDlg : public CGenericBrowserDlg
{
	DECLARE_DYNAMIC(CMedicationHistoryMedInfoPopupDlg)

public:
	std::vector<shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo>>* m_pvMedHistoryRowInfo;
	CMedicationHistoryMedInfoPopupDlg(CWnd* pParent) : CGenericBrowserDlg(pParent) {
		m_pvMedHistoryRowInfo = NULL;
	};
		
	BOOL CMedicationHistoryMedInfoPopupDlg::OnInitDialog()
	{
		CGenericBrowserDlg::OnInitDialog();
		//show only the Make sure that only the X and the Maximize are enabled
		ModifyStyle(WS_MINIMIZEBOX, WS_MAXIMIZEBOX, 0);
		
		try {
			shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo> spMedInfo;
			//for(std::vector<shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo>>::iterator it = values.begin(); it != values.end(); ++it)
			for(unsigned int i= 0; i < m_pvMedHistoryRowInfo->size(); i++)
			{
				spMedInfo = m_pvMedHistoryRowInfo->operator[](i);
				ConvertRecordToHTML(spMedInfo);
			}	
		} NxCatchAll(__FUNCTION__);

		return TRUE;
	}

	// (a.wilson 2013-08-15 13:51) - PLID 58060 - this will generate the data contained within the med history into an html format.
	void CMedicationHistoryMedInfoPopupDlg::ConvertRecordToHTML(shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo> spMedRowInfo)
	{
		NexTech_Accessor::_PatientRxHistoryPtr pPatientHistory  = spMedRowInfo->pPatientRxHistory;		
		NexTech_Accessor::_ERxPatientInfoPtr pPatientInfo = pPatientHistory->GetPatientInfo();
		NexTech_Accessor::_ERxMedicationInfoPtr pMedicationInfo = spMedRowInfo->pMedicationInfo;
	
		CString strHtmlBody;		
		strHtmlBody += "<body>";
		
		//Patient
		{
			//Name
			CString strPatientName = FormatString("%s, %s %s", AsString(pPatientInfo->GetLastName()), 
				AsString(pPatientInfo->GetFirstName()), AsString(pPatientInfo->GetMiddleName()));
			CString strAddress, strPhoneNumber;
			//Address
			if (CString(AsString(pPatientInfo->GetAddress1())).Trim() != "") {
				strAddress = FormatString("%s %s %s, %s %s", AsString(pPatientInfo->GetAddress1()), 
					AsString(pPatientInfo->GetAddress2()), AsString(pPatientInfo->GetCity()), AsString(pPatientInfo->GetState()), 
					AsString(pPatientInfo->GetZip()));
			}
			//Phone
			CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
			Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(pPatientInfo->GetPhoneNumbers());
			sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
			if (!arRenewalRequestPhoneNumbers.IsEmpty())
			{
				for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
				{
					NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
					if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
					{
						strPhoneNumber = VarString(phoneNumber->GetNumber(), "");
					}
				}
			}

			strHtmlBody += FormatString(
				"<div id=\"patient\"> \r\n"
				"	<p> \r\n"
				"		<b>%s</b><br> \r\n"
				"		%s<br> \r\n"
				"		%s \r\n"
				"	</p> \r\n"
				"</div> \r\n",
				strPatientName, 
				strAddress, 
				(strPhoneNumber.IsEmpty() ? "" :FormatPhone(strPhoneNumber, 
					GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true))));
		}

		//Prescriber, Pharmacy
		{
			CString strPresciber, strPharmacy;
			NexTech_Accessor::_ERxPrescriberInfoPtr pPrescriberInfo = pMedicationInfo->GetPrescriberInfo();
			NexTech_Accessor::_ERxPharmacyInfoPtr pPharmacyInfo = pMedicationInfo->GetPharmacyInfo();

			if (AsString(pPrescriberInfo->GetLastName()).Trim() != "") {
				strPresciber.Format("<b>Prescriber:</b> %s, %s %s",
					FixCapitalization(AsString(pPrescriberInfo->GetLastName())), 
					FixCapitalization(AsString(pPrescriberInfo->GetFirstName())), 
					FixCapitalization(AsString(pPrescriberInfo->GetMiddleName())));
			}
			if (AsString(pPharmacyInfo->GetName()).Trim() != "") {
				CString strPhoneNumber;
				CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
				Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(pPatientInfo->GetPhoneNumbers());
				sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
				if (!arRenewalRequestPhoneNumbers.IsEmpty())
				{
					for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
					{
						NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
						if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
						{
							strPhoneNumber = VarString(phoneNumber->GetNumber(), "").Trim();
						}
					}
				}

				strPharmacy.Format("<b>Pharmacy:</b> %s%s%s", 
					AsString(pPharmacyInfo->GetName()), 
					(AsString(pPharmacyInfo->GetAddress1()).Trim() == "" ? "" : FormatString("<br>%s %s %s, %s %s", 
					AsString(pPharmacyInfo->GetAddress1()), AsString(pPharmacyInfo->GetAddress2()), 
					AsString(pPharmacyInfo->GetCity()), AsString(pPharmacyInfo->GetState()), 
					AsString(pPharmacyInfo->GetZip()))), 
					(strPhoneNumber.IsEmpty() ? "" : ("<br>") + FormatPhone(strPhoneNumber, 
					GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true)))); 
			}

			strHtmlBody += FormatString(
				"<div id=\"sources\"> \r\n"
				"	<p> \r\n"
				"		%s\r\n"
				"		<br> \r\n"
				"		%s\r\n"
				"	</p> \r\n"
				"</div> \r\n", strPresciber, strPharmacy);
		}
		strHtmlBody += "<div id=\"spacer\"></div>";

		//Medication Info
		{
			CString strQuantityUnit, strStrengthUnit, strDosageForm;
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT Name FROM DrugStrengthUnitsT WHERE NCItCode = {STRING} OR SureScriptsCode = {STRING} "
				"SELECT Name FROM DrugStrengthUnitsT WHERE NCItCode = {STRING} OR SureScriptsCode = {STRING} "
				"SELECT Name FROM DrugDosageFormsT WHERE NCItCode = {STRING} OR SureScriptsCode = {STRING} ",
				VarString(pMedicationInfo->GetQuantityUnits(), ""), VarString(pMedicationInfo->GetQuantityUnits(), ""), 
				VarString(pMedicationInfo->GetStrengthUnits(), ""), VarString(pMedicationInfo->GetStrengthUnits(), ""), 
				VarString(pMedicationInfo->GetDosageForm(), ""), VarString(pMedicationInfo->GetDosageForm(), ""));
						
			if (!prs->eof) { //Quantity Unit
				strQuantityUnit = AdoFldString(prs, "Name", "");
			}
			prs = prs->NextRecordset(NULL);
			if (!prs->eof) { //Strength Unit
				strStrengthUnit = AdoFldString(prs, "Name", "");
			}
			prs = prs->NextRecordset(NULL);
			if (!prs->eof) { //Dosage Form
				strDosageForm = AdoFldString(prs, "Name", "");
			}

			strHtmlBody += FormatString(
				"<div id=\"medication\"> \r\n"
				"	<p> \r\n"
				"		<h2>%s</h2> \r\n"								//Medication Name
				"		<b>%s</b> \r\n"									//DEA Schedule
				"		<b>%s</b><br> \r\n"								//IsSupply, IsCompound
				"		<b>Written Date:</b> %s &nbsp;&nbsp;&nbsp; \r\n"//Written Date
				"		<b>Last Fill Date:</b> %s<br> \r\n"				//Last Fill Date
				"		<b>Quantity:</b> %s %s &nbsp;&nbsp;&nbsp; \r\n"	//Quantity
				"		<b>Strength:</b> %s %s &nbsp;&nbsp;&nbsp; \r\n"	//Strength
				"		<b>Dosage Form:</b> %s<br> \r\n"				//Dosage Form
				"		<b>Days Supply:</b> %s &nbsp;&nbsp;&nbsp; \r\n"	//Days Supply
				"		<b>Refills:</b> %s<br> \r\n"					//Refills
				"		%s<br> \r\n"									//Substitutions
				"		<b>Directions:</b> %s<br> \r\n"					//Directions
				"		<b>Notes:</b> %s \r\n"							//Notes
				"	</p> \r\n" 
				"</div> \r\n", 
				AsString(pMedicationInfo->Getdescription()), 
				(AsString(pMedicationInfo->GetDEASchedule()) != "" ? (AsString(pMedicationInfo->GetDEASchedule()) + ("&nbsp;&nbsp;&nbsp;")) : ""), 
				FormatString("%s &nbsp;&nbsp;&nbsp; %s", (pMedicationInfo->GetIsCompoundDrug() == VARIANT_TRUE ? "Compound Drug " : ""), 
				(pMedicationInfo->GetIsSupply() == VARIANT_TRUE ? "Supply" : "")), 
				(pMedicationInfo->GetWrittenDate() ? COleDateTime(pMedicationInfo->GetWrittenDate()).Format("%x") : "N/A"), 
				(pMedicationInfo->GetLastFillDate() ? COleDateTime(pMedicationInfo->GetLastFillDate()).Format("%x") : "N/A"), 
				AsString(pMedicationInfo->GetQuantity()).TrimLeft("0"), strQuantityUnit, 
				AsString(pMedicationInfo->GetStrength()).TrimLeft("0"), strStrengthUnit, 
				strDosageForm, 
				AsString(pMedicationInfo->GetDaysSupply()).TrimLeft("0"), 
				AsString(pMedicationInfo->GetRefills()).TrimLeft("0"), 
				(AsString(pMedicationInfo->GetSubstitution()) == "0" ? "<b>Allow Substitutions</b>" : ""),
				FixCapitalization(AsString(pMedicationInfo->GetDirections())), 
				AsString(pMedicationInfo->Getnotes()));	
		}

		strHtmlBody += "</body>";
		m_strDocumentHtml = WrapWithHeadersAndCSS(strHtmlBody);
		IStreamPtr pHtmlStream = Nx::StreamFromString(m_strDocumentHtml);
		NavigateToStream(pHtmlStream);
	}
	// (a.wilson 2013-08-15 13:51) - PLID 58060 - header information for the html setup.
	CString CMedicationHistoryMedInfoPopupDlg::WrapWithHeadersAndCSS(CString strBody)
	{

		CString strCSS, strHeader,strFinalHtml, strCloseHeader;


		strCSS += 
			"<title>Medication History</title> \r\n"
			"\r\n"
			"<style type=\"text/css\"> \r\n"
			"\r\n"
			"	body \r\n"
			"	{ \r\n"
			"		font-family: Arial; \r\n"
			"		background-color: #ebedfd; \r\n"
			"		margin: 0; \r\n"
			"	} \r\n"
			"\r\n"
			"h2 \r\n"
			"{ \r\n"
			"	margin: 0; \r\n"
			"} \r\n"
			"\r\n"
			"	#patient \r\n"
				"{ \r\n"
			"		position: absolute; \r\n"
			"		top: 10; \r\n"
			"		left: 15; \r\n"
			"		width: 350px; \r\n"
			"	} \r\n"
			"\r\n"
			"	#sources \r\n"
			"	{ \r\n"
			"		text-align: right; \r\n"
			"		position: absolute; \r\n"
			"		top: 10; \r\n"
			"		right: 15; \r\n"
			"		width: 350px; \r\n"
			"	} \r\n"
			"\r\n"
			"	#medication \r\n"
			"	{ \r\n"
			"		position: absolute; \r\n"
			"		top: 0; \r\n"
			"		left: 15; \r\n"
			"		margin-top: 105; \r\n"
			"	} \r\n"
			"\r\n"
			"	#spacer \r\n"
			"	{ \r\n"
			"		position: absolute; \r\n"
			"		top: 0; \r\n"
			"		margin-top: 90; \r\n"
			"		width: 775px; \r\n"
			"		height: 8px; \r\n"
			"		background-color: #B3C2FF; \r\n"
			"		overflow: hidden; \r\n"
			"	} \r\n"
			"\r\n"
			"</style> \r\n";

		strHeader = "<head> \r\n" + strCSS + "</head>\r\n";
		strFinalHtml = "<Html>\r\n" + strHeader + strBody + "</html>";

		return strFinalHtml;
	}

	std::list<shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo>> m_ary_pMedHistoryRowInfo;

	CString m_strDocumentHtml;


	virtual ~CMedicationHistoryMedInfoPopupDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};


// CMedicationHistoryMedInfoPopupDlg dialog
//(r.wilson 8/9/2013) PLID 57947 - Created Dialog

IMPLEMENT_DYNAMIC(CMedicationHistoryMedInfoPopupDlg, CGenericBrowserDlg)

CMedicationHistoryMedInfoPopupDlg::~CMedicationHistoryMedInfoPopupDlg()
{
}

void CMedicationHistoryMedInfoPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CGenericBrowserDlg::DoDataExchange(pDX);
}




// CMedicationHistoryMedInfoPopupDlg message handlers



namespace MedicationHistory 
{
	void PopupInfo(CWnd* pParent, std::vector<shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo>>& values)
	{
		CMedicationHistoryMedInfoPopupDlg dlg(pParent);					
		dlg.m_pvMedHistoryRowInfo = &values;
		
		dlg.DoModal();

		

	}
}


BEGIN_MESSAGE_MAP(CMedicationHistoryMedInfoPopupDlg, CGenericBrowserDlg)
	//ON_EVENT(CMedicationHistoryMedInfoPopupDlg, IDC_GENERIC_BROWSER, 259, DocumentComplete, VTS_DISPATCH VTS_PVARIANT)	
END_MESSAGE_MAP()

