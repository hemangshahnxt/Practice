// PhoneNumberEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PhoneNumberEntryDlg.h"
#include "Patientsrc.h"


// PhoneNumberEntryDlg dialog

IMPLEMENT_DYNAMIC(PhoneNumberEntryDlg, CNxDialog)

PhoneNumberEntryDlg::PhoneNumberEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(PhoneNumberEntryDlg::IDD, pParent)
{
	// Initialize the global variables
	countryCodeString = "";
	areaCodeString = "";
	localNumberString = "";
	extensionString = "";
	phoneNumber = "";
}

BOOL PhoneNumberEntryDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// Bind the countries drop down list to the database
	m_pCountryList = BindNxDataList2Ctrl(IDC_COUNTRY_REGION, true);

	// Initialize the country drop down list
	m_pCountryList->CurSel = m_pCountryList->SetSelByColumn(0, "United States");

	// By initializing the drop down list we must also initialize the selected value
	NXDATALIST2Lib::IRowSettingsPtr currentRowPtr = m_pCountryList->GetCurSel();
	countryCodeString = currentRowPtr->GetValue(1);
	UpdatePhoneNumberPreview();

	return TRUE;
}

PhoneNumberEntryDlg::~PhoneNumberEntryDlg()
{
}

void PhoneNumberEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PhoneNumberEntryDlg, CNxDialog)
	ON_EN_CHANGE(IDC_CITY_AREA_CODE, &PhoneNumberEntryDlg::OnAreaCode_Changed)
	ON_EN_CHANGE(IDC_LOCAL_NUMBER, &PhoneNumberEntryDlg::OnLocalNumber_Changed)
	ON_EN_CHANGE(IDC_EXTENSION, &PhoneNumberEntryDlg::OnExtension_Changed)
END_MESSAGE_MAP()

// This method generates the phone number preview
// the user sees.
void PhoneNumberEntryDlg::UpdatePhoneNumberPreview()
{
	CString formattedCountryCode = "";
	CString formattedAreaCode = "";
	CString formattedLocalNumber = "";
	CString formattedExtension = "";
	CString formattedNumberPreview = "";

	if (countryCodeString != "")
	{
		formattedCountryCode = "+" + countryCodeString;
	}
	if (areaCodeString != "")
	{
		formattedAreaCode = " (" + areaCodeString + ")";
	}
	if (localNumberString != "")
	{
		formattedLocalNumber = " " + localNumberString;
	}
	if (extensionString != "")
	{
		formattedExtension = " x " + extensionString;
	}

	if (formattedAreaCode != "" ||
		formattedLocalNumber != "")
	{
		formattedNumberPreview = formattedCountryCode + formattedAreaCode + formattedLocalNumber + formattedExtension;
	}
	else
	{
		formattedNumberPreview = formattedAreaCode + formattedLocalNumber + formattedExtension;
	}

	SetDlgItemText(IDC_NUMBER_PREVIEW, formattedNumberPreview);
	phoneNumber = formattedNumberPreview;
}

// Gets the country code and updates the phone number preview
void PhoneNumberEntryDlg::SelChosenCountryRegion(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr currentRowPtr = lpRow;
	countryCodeString = currentRowPtr->GetValue(1);

	UpdatePhoneNumberPreview();
}

// Gets the area code and updates the phone number preview
void PhoneNumberEntryDlg::OnAreaCode_Changed()
{
	GetDlgItemText(IDC_CITY_AREA_CODE, areaCodeString);

	UpdatePhoneNumberPreview();
}

// Gets the local number and updates the phone number preview
void PhoneNumberEntryDlg::OnLocalNumber_Changed()
{
	GetDlgItemText(IDC_LOCAL_NUMBER, localNumberString);

	UpdatePhoneNumberPreview();
}

// Gets the extension and updates the phone number preview
void PhoneNumberEntryDlg::OnExtension_Changed()
{
	GetDlgItemText(IDC_EXTENSION, extensionString);

	UpdatePhoneNumberPreview();
}

BEGIN_EVENTSINK_MAP(PhoneNumberEntryDlg, CNxDialog)
	ON_EVENT(PhoneNumberEntryDlg, IDC_COUNTRY_REGION, 16, PhoneNumberEntryDlg::SelChosenCountryRegion, VTS_DISPATCH)
END_EVENTSINK_MAP()