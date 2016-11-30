// LicenseInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "LicenseInfoDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GenericBrowserDlg.h"

#include <rapidjson/document.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLicenseInfoDlg dialog


CLicenseInfoDlg::CLicenseInfoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLicenseInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLicenseInfoDlg)
		// NoTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLicenseInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseInfoDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_LICENSE_INFO_BOX, m_nxeditLicenseInfoBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLicenseInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLicenseInfoDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ANALYTICS_USERS, &CLicenseInfoDlg::OnBnClickedAnalyticsUsers)
END_MESSAGE_MAP()


// (b.cardillo 2006-10-24 16:49) - PLID 17859 - Generate a more informative 
// string to indicate the license state for the given module.
CString CalcLicensedComponentStateString(CLicense::ELicensedComponent elcLicensedComponent)
{
	// Get all the license details for the this module
	int nUsesAllowed, nUsesUsed;
	COleDateTime dtExpiration;
	if (g_pLicense->CheckForLicense(elcLicensedComponent, CLicense::cflrSilent, FALSE, NULL, &nUsesAllowed, &nUsesUsed, &dtExpiration)) {
		// It's licensed so the string will start with "yes"
		CString strAns = "Yes";
		// If there are a limited number of uses, indicate how many are left
		if (nUsesAllowed != -1) {
			CString str;
			str.Format("(%li/%li uses left)", nUsesAllowed - nUsesUsed, nUsesAllowed);
			strAns += " " + str;
		}
		// If the module expires append the expiration date
		if (dtExpiration.GetStatus() == COleDateTime::valid) {
			strAns += " - Expires " + FormatDateTimeForInterface(dtExpiration, 0, dtoDate, false);
		}
		// Return the results
		return strAns;
	} else {
		// It's not licensed at all so just return that fact
		return "No";
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLicenseInfoDlg message handlers

BOOL CLicenseInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnClose.AutoSet(NXB_CLOSE);

		CString strInfoString = "";
		CString str;

		str.Format("License Key: %i\r\n", g_pLicense->GetLicenseKey());
		strInfoString += str;
			
		COleDateTime dt = g_pLicense->GetExpirationDate();
		if(dt == COleDateTime(1899,12,30,0,0,0)) {
			str = "No Support Expiration Date\r\n\r\n";
		}
		else {
			str.Format("Support Expires: %s\r\n\r\n",FormatDateTimeForInterface(dt, 0, dtoDate, false));
		}
		strInfoString += str;

		// (b.cardillo 2006-10-24 16:51) - PLID 17859 - Let the user know how many uses of Practice 
		// they have left (after the current one, which is why we have to subtract 1).  Notice, this 
		// applies to licenses that haven't been activated as well as licenses that explicitly only 
		// allow a limited number of uses of Practice.
		{
			int nUsesAllowed, nUsesUsed;
			if (g_pLicense->CheckForLicense(CLicense::lcPractice, CLicense::cflrSilent, FALSE, NULL, &nUsesAllowed, &nUsesUsed, NULL)) {
				if (nUsesAllowed != -1) {
					str.Format("LIMITED TO %li REMAINING USE%s OF PRACTICE!\r\n\r\n", nUsesAllowed - nUsesUsed - 1, (nUsesAllowed - nUsesUsed - 1) == 1 ? "" : "S");
					strInfoString += str;
				}
			}
		}

		str.Format("Workstation Licenses Used: %li\r\n",g_pLicense->GetWorkstationCountUsed());
		strInfoString += str;

		str.Format("Total Workstation Licenses Available: %li\r\n",g_pLicense->GetWorkstationCountAllowed());
		strInfoString += str;

		// (z.manning 2012-06-11 15:26) - PLID 50877
		str.Format("iPad Licenses Used: %li\r\n", g_pLicense->GetIpadCountUsed());
		strInfoString += str;
		str.Format("Total iPad Licenses Available: %li\r\n", g_pLicense->GetIpadCountAllowed());
		strInfoString += str;

		// (d.thompson 2011-10-26) - PLID 46138
		str.Format("Concurrent TS Licenses Available: %li\r\n", g_pLicense->GetConcurrentTSCountAllowed());
		strInfoString += str;

		CStringArray saPool;
		g_pLicense->GetTSMachinePool(saPool);
		if(saPool.GetSize() > 0) {
			//Print available machines
			str = "Available Machines:  ";
			str += GenerateDelimitedListFromStringArray(saPool, ", ") + "\r\n";
			strInfoString += str;
		}

		str = "\r\n";
		strInfoString += str;

		str.Format("Number Of Providers Licensed: %li\r\n",g_pLicense->GetDoctorCountAllowed());
		strInfoString += str;

		// (z.manning 2009-10-16 15:10) - PLID 35749 - NexSync licensing
		str.Format("Number Of NexSync Users Licensed: %li\r\n", g_pLicense->GetNexSyncCountAllowed());
		strInfoString += str;

		str.Format("Number Of Palm Pilots Licensed: %li\r\n",g_pLicense->GetPalmCountAllowed());
		strInfoString += str;

		str.Format("Number Of PDAs Licensed: %li\r\n",g_pLicense->GetPPCCountAllowed());
		strInfoString += str;

		// (z.manning 2013-08-16 14:44) - PLID 55816
		str.Format("Number Of Anchor Callers: %li\r\n", g_pLicense->GetAnchorCallerCount());
		strInfoString += str;

		// (a.walling 2015-05-06 15:49) - PLID 65920 - Analytics
		str.Format("Number Of Analytics Users: %li\r\n", g_pLicense->GetAnalyticsUserCountAllowed());
		strInfoString += str;

		// (b.cardillo 2016-03-08 14:02) - PLID 68409 - Iagnosis provider count
		str.Format("Number Of Iagnosis Providers Licensed: %li\r\n", g_pLicense->GetIagnosisProviderCount());
		strInfoString += str;

		strInfoString += "\r\n";

		// (a.walling 2006-12-18 17:17) - PLID 23397 - Show the license EMR provider info here
		str.Format("Number Of EMR Provider Licenses Used: %li\r\n",g_pLicense->GetEMRProvidersCountUsed());
		strInfoString += str;

		str.Format("Number Of EMR Provider Licenses Available: %li\r\n",g_pLicense->GetEMRProvidersCountAllowed());
		strInfoString += str;

		// (j.luckoski 01/28/2013) - PLID 49892 - Show amounts of prescriber licenses used.

		str.Format("Number Of ERX Licensed Prescribers Used: %li\r\n",g_pLicense->GetERXLicPrescribersCountUsed());
		strInfoString += str;

		str.Format("Number Of ERX Licensed Prescribers Available: %li\r\n",g_pLicense->GetERXLicPrescribersCountAllowed());
		strInfoString += str;

		str.Format("Number Of ERX Midlevel Prescribers Used: %li\r\n",g_pLicense->GetERXMidPrescribersCountUsed());
		strInfoString += str;

		str.Format("Number Of ERX Midlevel Prescribers Available: %li\r\n",g_pLicense->GetERXMidPrescribersCountAllowed());
		strInfoString += str;

		str.Format("Number Of ERX Nurse/Staff Prescribers Used: %li\r\n",g_pLicense->GetERXStaffPrescribersCountUsed());
		strInfoString += str;

		str.Format("Number Of ERX Nurse/Staff Prescribers Available: %li\r\n",g_pLicense->GetERXStaffPrescribersCountAllowed());
		strInfoString += str;

		// (z.manning 2013-01-31 15:13) - PLID 54954
		str.Format("Number Of Dictation Users Used: %li\r\n", g_pLicense->GetNuanceUserCountUsed());
		strInfoString += str;
		str.Format("Number Of Dictation Users Available: %li\r\n", g_pLicense->GetNuanceUserCountAllowed());
		strInfoString += str;

		// (z.manning 2015-06-17 16:48) - PLID 66287
		str.Format("Number Of Portal Providers Used: %li\r\n", g_pLicense->GetPortalProviderCountUsed());
		strInfoString += str;
		str.Format("Number Of Portal Providers Available: %li\r\n", g_pLicense->GetPortalProviderCountAllowed());
		strInfoString += str;

		// (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
		strInfoString += FormatString("Subscription Local Hosted: %s\r\n", g_pLicense->GetSubscriptionLocalHosted() ? "YES" : "NO");

		// (r.gonet 2016-05-11) - NX-100478 - Added Azure RemoteApp
		str.Format("Azure RemoteApp: %s\r\n", (g_pLicense->GetAzureRemoteApp() ? "YES" : "NO"));
		strInfoString += str;

		strInfoString +=  "\r\n Stations Using a License:\r\n";
		strInfoString += "--------------------------------------------------------------------------\r\n";

		//get the logged in names
		CStringArray saWorkstations;
		g_pLicense->GetUsedWorkstations(saWorkstations);
		// (d.thompson 2013-08-13) - PLID 24326 - Hey, this sorts!
		std::sort(saWorkstations.GetData(), saWorkstations.GetData() + saWorkstations.GetSize());

		for (int i = 0; i < saWorkstations.GetSize(); i++)
		{
			CString station;
			station = saWorkstations.GetAt(i);
			station.Replace("practice.mde","");
			station.Replace("Practice.mde",""); // (a.walling 2006-12-19 16:26) - The Practice.mde is usually initially capped
			str.Format("%s\r\n",station);
			strInfoString += str;
		}

		// (z.manning 2012-06-11 15:55) - PLID 50877 - Added iPads
		CStringArray saIpads;
		g_pLicense->GetUsedIpads(saIpads);
		// (d.thompson 2013-08-13) - PLID 24326 - Hey, this sorts!
		std::sort(saIpads.GetData(), saIpads.GetData() + saIpads.GetSize());

		for(int nIpadIndex = 0; nIpadIndex < saIpads.GetSize(); nIpadIndex++)
		{
			CString strIpad = saIpads.GetAt(nIpadIndex);
			strInfoString += strIpad + "\r\n";
		}

		strInfoString += "--------------------------------------------------------------------------\r\n";

		// (a.walling 2006-12-18 17:19) - PLID 23397 - Show which providers are using a license here
		CDWordArray dwaUsedEMRProvIDs;

		g_pLicense->GetUsedEMRProviders(dwaUsedEMRProvIDs);

		if (dwaUsedEMRProvIDs.GetSize() > 0) {
			strInfoString +=  "\r\n EMR Provider Licenses:\r\n";
			strInfoString += "--------------------------------------------------------------------------\r\n";

			for (i = 0; i < dwaUsedEMRProvIDs.GetSize(); i++) {
				strInfoString += GetExistingContactName(dwaUsedEMRProvIDs.GetAt(i)) + "\r\n";
			}

			strInfoString += "--------------------------------------------------------------------------\r\n";
		}

		strInfoString += "\r\n Purchased Modules \r\n";
		strInfoString += "--------------------------------------------------------------------------\r\n";

		str.Format("Scheduler: %s\r\n", CalcLicensedComponentStateString(CLicense::lcSchedule));
		strInfoString += str;

		/*
		str.Format("Patients: %s\r\n",g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflr) ? "Yes" : "No");
		strInfoString += str;

		str.Format("Performance: %s\r\n",g_pLicense->CheckForLicense(CLicense::lcPerform, CLicense::cflr) ? "Yes" : "No");
		strInfoString += str;

		str.Format("Practice: %s\r\n",g_pLicense->CheckForLicense(CLicense::lcPractice, CLicense::cflr) ? "Yes" : "No");
		strInfoString += str;
		*/

		str.Format("Quotes: %s\r\n", CalcLicensedComponentStateString(CLicense::lcQuotes));
		strInfoString += str;

		str.Format("Billing: %s\r\n", CalcLicensedComponentStateString(CLicense::lcBill));
		strInfoString += str;

		// (a.walling 2007-07-30 12:53) - PLID 26758 - Added CC Processing
		// (d.lange 2010-12-15 17:20) - PLID 40312 - Renamed to 'Intuit CC Processing' from 'Credit Card Processing'
		str.Format("Intuit CC Processing: %s\r\n", CalcLicensedComponentStateString(CLicense::lcCCProcessing));
		strInfoString += str;

		str.Format("HCFA: %s\r\n", CalcLicensedComponentStateString(CLicense::lcHCFA));
		strInfoString += str;

		str.Format("Ebilling: %s\r\n", CalcLicensedComponentStateString(CLicense::lcEbill));
		strInfoString += str;

		// (j.jones 2007-06-28 09:20) - PLID 23951 - added licensing for E-Remittance
		str.Format("E-Remittance: %s\r\n", CalcLicensedComponentStateString(CLicense::lcERemittance));
		strInfoString += str;

		// (j.jones 2007-06-28 09:20) - PLID 23950 - added licensing for E-Eligibility
		str.Format("E-Eligibility: %s\r\n", CalcLicensedComponentStateString(CLicense::lcEEligibility));
		strInfoString += str;		

		str.Format("Inventory: %s\r\n", CalcLicensedComponentStateString(CLicense::lcInv));
		strInfoString += str;

		// (a.walling 2008-02-14 13:02) - PLID 28388 - added licensing for Advanced Inventory
		// (d.thompson 2009-01-26) - PLID 32851 - Renamed to 'Consignment and Allocation' from 'Advanced Inventory'
		str.Format("Consignment and Allocation: %s\r\n", CalcLicensedComponentStateString(CLicense::lcAdvInventory));
		strInfoString += str;

		str.Format("Letter Writing: %s\r\n", CalcLicensedComponentStateString(CLicense::lcLetter));
		strInfoString += str;

		str.Format("Marketing: %s\r\n", CalcLicensedComponentStateString(CLicense::lcMarket));
		strInfoString += str;		

		str.Format("Surgery Center: %s\r\n", CalcLicensedComponentStateString(CLicense::lcSurgeryCenter));
		strInfoString += str;

		str.Format("CareCredit Link: %s\r\n", CalcLicensedComponentStateString(CLicense::lcCareCredit));
		strInfoString += str;

		str.Format("Mirror Link: %s\r\n", CalcLicensedComponentStateString(CLicense::lcMirror));
		strInfoString += str;

		str.Format("Inform Link: %s\r\n", CalcLicensedComponentStateString(CLicense::lcInform));
		strInfoString += str;

		str.Format("United Link: %s\r\n", CalcLicensedComponentStateString(CLicense::lcUnited));
		strInfoString += str;

		str.Format("Quickbooks Link: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcQuickBooks));
		strInfoString += str;
		
		str.Format("ChaseHealthAdvance Link: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcUnicorn));
		strInfoString += str;

		str.Format("NexSpa: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcNexSpa));
		strInfoString += str;

		str.Format("NexForms: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcNexForms));
		strInfoString += str;

		str.Format("NexTrak: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcNexTrak));
		strInfoString += str;

		str.Format("EMR (Level 1): %s\r\n",  CalcLicensedComponentStateString(CLicense::lcCustomRecords));
		strInfoString += str;
		
		str.Format("EMR (Level 2): %s\r\n",  CalcLicensedComponentStateString(CLicense::lcNexEMR));
		strInfoString += str;
		
		// (a.walling 2008-07-16 15:44) - PLID 30734 - added licensing for Transcriptions
		str.Format("Transcription Parsing: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcTranscriptions));
		strInfoString += str;

		//DRT 7/31/2008 - PLID 30890 - Added licensing for Faxing
		str.Format("Faxing: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcFaxing));
		strInfoString += str;

		//DRT 7/31/2008 - PLID 30890 - Added licensing for Barcode Reading
		str.Format("Barcode Reading: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcBarcodeRead));
		strInfoString += str;

		str.Format("HL7: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcHL7));
		strInfoString += str;

		// (c.haag 2016-03-09) - PLID 68175 - Deprecated
		//str.Format("NexVoice: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcNexVoice));
		//strInfoString += str;

		str.Format("NexWeb: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcNexWeb));
		strInfoString += str;

		str.Format("Retention: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcRetention));
		strInfoString += str;

		str.Format("Labs: %s\r\n",  CalcLicensedComponentStateString(CLicense::lcLabs));
		strInfoString += str;

		//DRT 10/9/2008 - PLID 31491 - Added licensing for Standard modules
		str.Format("Scheduler Standard Edition:  %s\r\n", CalcLicensedComponentStateString(CLicense::lcSchedStandard));
		strInfoString += str;

		// (d.thompson 2008-12-03) - PLID 32144 - LW Standard is gone
		//str.Format("Letter Writing Standard Edition:  %s\r\n", CalcLicensedComponentStateString(CLicense::lcLWStandard));
		//strInfoString += str;

		str.Format("EMR Standard Edition:  %s\r\n", CalcLicensedComponentStateString(CLicense::lcEMRStandard));
		strInfoString += str;

		//DRT 10/22/2008 - PLID 31788 - Added licensing for Cycle of Care
		str.Format("Cycle Of Care:  %s\r\n", CalcLicensedComponentStateString(CLicense::lcCycleOfCare));
		strInfoString += str;

		//DRT 11/20/2008 - PLID 32085 - Added licensing for ePrescribe
		str.Format("ePrescribing:  %s\r\n", CalcLicensedComponentStateString(CLicense::lcePrescribe));
		strInfoString += str;

		// (d.thompson 2009-03-24) - PLID 33583 - Added licensing for NewCrop
		str.Format("NewCrop:  %s\r\n", CalcLicensedComponentStateString(CLicense::lcNewCrop));
		strInfoString += str;

		//TES 10/7/2009 - PLID 35171 - Added new NexWeb license objects
		str.Format("NexWeb Lead Generation: %s\r\n", CalcLicensedComponentStateString(CLicense::lcNexWebLeads));
		strInfoString += str;
		str.Format("NexWeb Patient Portal: %s\r\n", CalcLicensedComponentStateString(CLicense::lcNexWebPortal));
		strInfoString += str;
		// (z.manning 2011-12-05 10:09) - PLID 44162 - NexWeb EMR
		str.Format("NexWeb EMR: %s\r\n", CalcLicensedComponentStateString(CLicense::lcNexWebEmr));
		strInfoString += str;

		//TES 10/7/2009 - PLID 35802 - Added FirstDataBank licensing
		str.Format("FirstDataBank Integration: %s\r\n", CalcLicensedComponentStateString(CLicense::lcFirstDataBank));
		strInfoString += str;

		// (c.haag 2009-10-28 10:34) - PLID 36067 - Added NexPhoto licensing
		str.Format("NexPhoto Integration: %s\r\n", CalcLicensedComponentStateString(CLicense::lcNexPhoto));
		strInfoString += str;

		// (j.jones 2009-11-04 10:06) - PLID 36135 - added TOPS license,
		str.Format("TOPS Link: %s\r\n", CalcLicensedComponentStateString(CLicense::lcTOPSLink));
		strInfoString += str;

		// (c.haag 2010-06-01 15:40) - PLID 38965 - Added CellTrust licensing
		// (z.manning 2010-11-17 12:05) - PLID 40643 - Renamed to NxReminder
		str.Format("NexReminder: %s\r\n", CalcLicensedComponentStateString(CLicense::lcNxReminder));
		strInfoString += str;

		// (j.gruber 2010-06-02 08:31) - PLID 38935 - BOLD Licensing
		str.Format("BOLD Link: %s\r\n", CalcLicensedComponentStateString(CLicense::lcBold));
		strInfoString += str;

		// (c.haag 2010-06-29 17:08) - PLID 39402 - Frames licensing
		str.Format("Frames: %s\r\n", CalcLicensedComponentStateString(CLicense::lcFrames));
		strInfoString += str;

		// (c.haag 2010-06-30 09:00) - PLID 37986 - Device Import
		str.Format("Device Import: %s\r\n", CalcLicensedComponentStateString(CLicense::lcDeviceImport));
		strInfoString += str;

		// (d.lange 2010-09-08 09:55) - PLID 40370 - Chase CC Processing
		str.Format("Chase CC Processing: %s\r\n", CalcLicensedComponentStateString(CLicense::lcChaseCCProcessing));
		strInfoString += str;

		//TES 12/9/2010 - PLID 41701 - Glasses Orders
		// (j.dinatale 2012-05-11 09:58) - PLID 49078 - now optical orders
		str.Format("Optical Orders: %s\r\n", CalcLicensedComponentStateString(CLicense::lcGlassesOrders));
		strInfoString += str;

		// (z.manning 2011-01-11 10:48) - PLID 42057 - Patient check-in
		str.Format("Patient Check-in: %s\r\n", CalcLicensedComponentStateString(CLicense::lcPatientCheckIn));
		strInfoString += str;

		// (a.wilson 2011-11-8) PLID 46335 - Eyemaginations license
		str.Format("Eyemaginations: %s\r\n", CalcLicensedComponentStateString(CLicense::lcEyemaginations));
		strInfoString += str;

		// (d.singleton 2012-03-21 11:49) - PLID 49086 license for code correct
		str.Format("CodeCorrect: %s\r\n", CalcLicensedComponentStateString(CLicense::lcCodeCorrect));
		strInfoString += str;

		// (j.armen 2012-03-27 17:00) - PLID 49244 - Recall
		strInfoString += FormatString("Recall: %s\r\n", CalcLicensedComponentStateString(CLicense::lcRecall));

		// (j.armen 2012-05-30 14:01) - PLID 50688 - NexEMRCosmetic
		strInfoString += FormatString("NexEMR Cosmetic: %s\r\n", CalcLicensedComponentStateString(CLicense::lcNexEMRCosmetic));

		// (b.spivey - November 25th, 2013) - PLID 59590 - Direct Message 
		strInfoString += FormatString("Direct Message: %s\r\n", CalcLicensedComponentStateString(CLicense::lcDirectMessage)); 

		// (a.wilson 2014-07-11 14:45) - PLID 62516 - VisionPayments
		strInfoString += FormatString("Vision Payments: %s\r\n", CalcLicensedComponentStateString(CLicense::lcVisionPayments));

		// (b.spivey - August 8th, 2014) - PLID 62926
		strInfoString += FormatString("Lockbox Payments: %s\r\n", CalcLicensedComponentStateString(CLicense::lcLockboxPayments)); 

		// (z.manning 2015-05-12 15:06) - PLID 65961
		strInfoString += FormatString("State ASC Reports: %s\r\n", CalcLicensedComponentStateString(CLicense::lcStateASCReports));

		// (z.manning 2015-06-17 09:11) - PLID 66407
		strInfoString += FormatString("MyPatientVisit - Patients: %s\r\n", CalcLicensedComponentStateString(CLicense::lcMPVPatients));

		// (z.manning 2015-07-21 13:21) - PLID 66599
		strInfoString += FormatString("Integrated Credit Card Processing: %s\r\n", CalcLicensedComponentStateString(CLicense::lcICCP));

		// (r.farnworth 2015-11-11 13:20) - PLID 67567
		strInfoString += FormatString("MyPatientVisit – Enhanced Messaging: %s\r\n", CalcLicensedComponentStateString(CLicense::lcMPVMessaging));

		strInfoString += FormatString("Active Directory Login: %s\r\n", CalcLicensedComponentStateString(CLicense::lcActiveDirectoryLogin));

		strInfoString += FormatString("Enterprise Reporting: %s\r\n", CalcLicensedComponentStateString(CLicense::lcEnterpriseReporting));

		str = "\r\n";
		strInfoString += str;		

		/*
		if(g_pLicense->m_practicetype == 0)
			str = "License Type - Plastic";
		else if(g_pLicense->m_practicetype == 1)
			str = "License Type - Reproductive";
		else if(g_pLicense->m_practicetype == 2)
			str = "License Type - Refractive";
		else if(g_pLicense->m_practicetype == 3)
			str = "License Type - Practice Internal";
		strInfoString += str;
		*/

		SetDlgItemText(IDC_LICENSE_INFO_BOX,strInfoString);

	}NxCatchAll("Error loading license information.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2015-06-15 15:26) - PLID 65923 - Show messagebox with analytics users + documents, and/or messagetext
void CLicenseInfoDlg::OnBnClickedAnalyticsUsers()
{
	try {
		long licenseKey = g_pLicense->GetLicenseKey();

		MSXML2::IXMLHTTPRequestPtr req("Msxml2.ServerXMLHTTP.3.0");

		CString url = FormatString("https://services.nextech.com/Client/AnalyticsAccess/api/Access?licenseKey=%li", licenseKey);

		static const char ridiculousAnalyticsPassword[] = "b7xEW6!okNjzpRU8$Zbu9z(Az!pf!$0E!Bw7H8Br4YK.m24mJR!N39QM._QC=DT";
		
		{
			CWaitCursor wait;
			req->open("GET", (const char*)url, false, "AnalyticsServices", ridiculousAnalyticsPassword);
			req->send();
		}

		long status = req->status;

		if (status != 200) {
			ThrowNxException("HTTP status %li", status);
		}

		CString response = (const char*)req->responseText;

		rapidjson::Document doc;
		doc.Parse(response);
		

		CString str = R"(<!doctype html>
<head>
<title>Analytics Users</title>
<style>

body {
	font-family: Segoe UI, Helvetica, Arial, Sans-serif;
}

.message {
	width: 80%;
	padding: 2em;
}

td {
	padding-right: 2em;
	padding-left: 2em;
}

table {
	margin: 0 auto;
}

</style>
<body>
)";

		if (doc.HasMember("messagetext")) {
			CString message = doc["messagetext"].GetString();
			if (!message.IsEmpty()) {
				str.AppendFormat("<div class='message'>%s</div>\r\n", ConvertToHTMLEmbeddable(message));
			}
		}

		if (doc.HasMember("Documents") && doc["Documents"].IsArray()) {
			auto& documents = doc["Documents"];

			std::multimap<CiString, CiString> users;

			for (auto it = documents.Begin(); it != documents.End(); ++it) {
				auto& lic = *it;

				CString documentName = lic["documentName"].GetString();

				CString username = lic["username"].GetString();

				users.emplace(documentName, username);
			}

			if (!users.empty()) {

				CString curDocument;

				str.Append("<table>\r\n");
				str.Append("<thead><tr><th>Document</th><th>Users</th></tr></thead>\r\n");
				str.Append("<tbody>\r\n");

				for (auto&& entry : users) {
					str.Append("<tr>");
					if (entry.first != curDocument) {
						curDocument = entry.first;
						str.AppendFormat("<td>%s</td>", ConvertToHTMLEmbeddable(curDocument));
					}
					else {						
						str.Append("<td>&nbsp;</td>");
					}
					str.AppendFormat("<td>%s</td>", ConvertToHTMLEmbeddable(entry.second));
					
					str.Append("</tr>\r\n");
				}
				
				str.Append("</tbody>\r\n</table>\r\n");
			}
		}

		CGenericBrowserDlg dlg(this, "CLicenseInfoDlg::AnalyticsUsers");

		dlg.NavigateToHtml(str);

		dlg.DoModal();
		
	} NxCatchAll(__FUNCTION__);
}
