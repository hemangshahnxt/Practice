// SupportEditLicenseDlg.cpp : implementation file
//

// (d.singleton 2012-01-10 17:37) - PLID 28219 - files added for new dialog

#include "stdafx.h"
#include "Practice.h"
#include "SupportEditLicenseDlg.h"
#include "SupportDlg.h"
#include "AuditSupportDlg.h" // (r.ries 2013-01-09) - PLID 54643 - Perform actions on Modules
#include <NxPracticeSharedLib\SupportUtils.h> // (r.ries 2013-01-09) - PLID 54643 - Perform actions on Modules
// CSupportEditLicenseDlg dialog

// (b.eyers 2015-03-05) - PLID 65097
#define LOCKOUT_DAYS	7
#define GRACE_PERIOD_MINUTES	10

using namespace NXDATALIST2Lib;
using namespace ADODB; // (r.ries 2013-01-09) - PLID 54643 - Perform actions on Modules

IMPLEMENT_DYNAMIC(CSupportEditLicenseDlg, CNxDialog)


// enum for datalist columns
enum eLicenseTypeCol
{
	ltcID = 0,
	ltcName,
	ltcBought,
	ltcInUse,
	ltcPending,
};

CSupportEditLicenseDlg::CSupportEditLicenseDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CSupportEditLicenseDlg::IDD, pParent)
{
	m_bInvalid = FALSE;
	m_bIsTestAccount = FALSE;
	bBeginNewLockoutPeriod = FALSE; // (b.eyers 2015-03-05) - PLID 65097
	m_bRedHelpText = FALSE; // (b.eyers 2015-03-05) - PLID 65099
	// Existing Bought
	m_nBLicenses = -1;
	m_nBIpads = -1;
	m_nBConc = -1;
	m_nBDoctors = -1;
	m_nBNexSync = -1;
	m_nBPalm = -1;
	m_nBNexPDA = -1;
	m_nBDatabases = -1;
	m_nBEMRProv = -1;
	m_nBNxWeb = -1;
	m_nBLicPres = -1; //(r.farnworth 2013-07-19) PLID 57522
	m_nBMidPres =  -1;  //(r.farnworth 2013-07-19) PLID 57522
	m_nBStaffPres = -1;  //(r.farnworth 2013-07-19) PLID 57522
	m_nBDictation = -1;
	m_nBPortalProviders = -1;

	// Existing Pending
	m_nPLicenses = -1;
	m_nPIpads = -1;
	m_nPConc = -1;
	m_nPDoctors = -1;
	m_nPNexSync = -1;
	m_nPPalm = -1;
	m_nPNexPDA = -1;
	m_nPDatabases = -1;
	m_nPEMRProv = -1;
	m_nPNxWeb = -1;
	m_nPLicPres = -1; //(r.farnworth 2013-07-19) PLID 57522
	m_nPMidPres =  -1;  //(r.farnworth 2013-07-19) PLID 57522
	m_nPStaffPres = -1;  //(r.farnworth 2013-07-19) PLID 57522
	m_nPDictation = -1;
	m_nPPortalProviders = -1;

	// Existing in-use
	m_nIULicenses = -1;
	m_nIUIpads = -1;
	m_nIUConc = -1;
	m_nIUDoctors = -1;
	m_nIUNexSync = -1;
	m_nIUPalm = -1;
	m_nIUNexPDA = -1;
	m_nIUDatabases = -1;
	m_nIUEMRProv = -1;
	m_nIUNxWeb = -1;
    m_nIUNxCloudUser= -1;	// (s.tullis 2013-11-15 10:36) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	m_nIUNxCloudVPN= -1;	// (s.tullis 2013-11-15 10:36) - PLID 57553 - Nexcloud in Practice support tab change number of licences
	m_nIULicPres = -1; //(r.farnworth 2013-07-19) PLID 57522
	m_nIUMidPres =  -1;  //(r.farnworth 2013-07-19) PLID 57522
	m_nIUStaffPres = -1;  //(r.farnworth 2013-07-19) PLID 57522
	m_nIUDictation = -1;
	m_nIUPortalProviders = -1;
}

CSupportEditLicenseDlg::~CSupportEditLicenseDlg()
{
}

void CSupportEditLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_LICENSE_ADJUST, m_LicenseAdjust);
	DDX_Control(pDX, IDC_ADD_ON, m_AddOn);
	DDX_Control(pDX, IDC_REACTIVATE, m_Reactivate);
	DDX_Control(pDX, IDC_DEACTIVATE, m_Deactivate);
	DDX_Control(pDX, IDC_SWAP_LICENSE, m_SwapLicense);
	DDX_Control(pDX, IDC_HELP_TEXT_STATIC, m_HelpText);
}


BEGIN_MESSAGE_MAP(CSupportEditLicenseDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CSupportEditLicenseDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ADD_ON, &CSupportEditLicenseDlg::OnBnClickedAddOn)
	ON_BN_CLICKED(IDC_REACTIVATE, &CSupportEditLicenseDlg::OnBnClickedReactivate)
	ON_BN_CLICKED(IDC_DEACTIVATE, &CSupportEditLicenseDlg::OnBnClickedDeactivate)
	ON_EN_CHANGE(IDC_LICENSE_AMOUNT, &CSupportEditLicenseDlg::OnEnChangeLicenseAmount)
	ON_BN_CLICKED(IDC_SWAP_LICENSE, &CSupportEditLicenseDlg::OnBnClickedSwapLicense)
	ON_BN_CLICKED(IDC_ENABLE_EXPIRATION, &CSupportEditLicenseDlg::OnBnClickedEnableExpiration) // (r.ries 2013-02-4) - PLID 54956
	ON_BN_CLICKED(IDC_NO_EXPIRATION, &CSupportEditLicenseDlg::OnBnClickedNoExpiration) // (r.ries 2013-02-4) - PLID 54956
	ON_BN_CLICKED(IDC_SET_EXPIRATION, &CSupportEditLicenseDlg::OnBnClickedSetExpiration) // (r.ries 2013-02-4) - PLID 54956
	ON_BN_CLICKED(IDC_BTN_CLEAR_LOCKOUT_PERIOD, &CSupportEditLicenseDlg::OnBtnClearLockoutPeriod) // (b.eyers 2015-03-05) PLID 65101 
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CSupportEditLicenseDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {	
		//set button icons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//set colors
		((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL_EDIT_LICENSE3))->SetColor(m_Color);
		((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL_EDIT_LICENSE2))->SetColor(m_Color);
		((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL_EDIT_LICENSE))->SetColor(m_Color);

		m_LicenseAdjust.SetBuddy(GetDlgItem(IDC_LICENSE_AMOUNT));
		m_LicenseAdjust.SetRange(0, MAX_LIMIT);

		m_pdlLicenseType = BindNxDataList2Ctrl(IDC_LICENSE_TYPE, false);
		m_pdlLicenseTypeSwap = BindNxDataList2Ctrl(IDC_LICENSE_TYPE_SWAP, false);
		m_pModuleExpDate = BindNxTimeCtrl(this, IDC_MODULE_EXP_DATE); // (r.ries 2013-02-4) - PLID 54956

		CPopulateLicenseTypeDataList(m_pdlLicenseType);

		EnableControls();

		RefreshHelpText();
		RefreshLicenseLockoutControls(); // (b.eyers 2015-03-05) - PLID 65100 

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CSupportEditLicenseDlg::CPopulateLicenseTypeDataList(_DNxDataListPtr pdlLicenseType)
{
	IRowSettingsPtr pRow = pdlLicenseType->GetNewRow();

	//build the datalist
	// add license row
	pRow->PutValue(ltcID, long(ltidLicenses));
	pRow->PutValue(ltcName, "Workstation License");
	pRow->PutValue(ltcBought, m_nBLicenses);
	pRow->PutValue(ltcInUse, m_nIULicenses);
	pRow->PutValue(ltcPending, m_nPLicenses);
	pdlLicenseType->AddRowAtEnd(pRow, NULL);

	// (z.manning 2012-06-12 15:53) - PLID 50878 - Added iPad licenses
	pRow = m_pdlLicenseType->GetNewRow();
	pRow->PutValue(ltcID, long(ltidIpads));
	pRow->PutValue(ltcName, "iPad License");
	pRow->PutValue(ltcBought, m_nBIpads);
	pRow->PutValue(ltcInUse, m_nIUIpads);
	pRow->PutValue(ltcPending, m_nPIpads);
	pdlLicenseType->AddRowAtEnd(pRow, NULL);

	//add conc ts license row
	pRow = m_pdlLicenseType->GetNewRow();
	pRow->PutValue(ltcID, long(ltidConcLicenses));
	pRow->PutValue(ltcName, "Conc. TS License");
	pRow->PutValue(ltcBought, m_nBConc);
	pRow->PutValue(ltcInUse, m_nIUConc);
	pRow->PutValue(ltcPending, m_nPConc);
	pdlLicenseType->AddRowAtEnd(pRow, NULL);

	//add nexsync row
	pRow = m_pdlLicenseType->GetNewRow();
	pRow->PutValue(ltcID, long(ltidNexsync));
	pRow->PutValue(ltcName, "NexSync");
	pRow->PutValue(ltcBought, m_nBNexSync);
	pRow->PutValue(ltcInUse, m_nIUNexSync);
	pRow->PutValue(ltcPending, m_nPNexSync);
	pdlLicenseType->AddRowAtEnd(pRow, NULL);

	//add the palm pilot row
	pRow = m_pdlLicenseType->GetNewRow();
	pRow->PutValue(ltcID, long(ltidPalmPilot));
	pRow->PutValue(ltcName, "Palm Pilot");
	pRow->PutValue(ltcBought, m_nBPalm);
	pRow->PutValue(ltcInUse, m_nIUPalm);
	pRow->PutValue(ltcPending, m_nPPalm);
	pdlLicenseType->AddRowAtEnd(pRow, NULL);

	//add the nexpda row
	pRow = m_pdlLicenseType->GetNewRow();
	pRow->PutValue(ltcID, long(ltidNexPDA));
	pRow->PutValue(ltcName, "NexPDA");
	pRow->PutValue(ltcBought, m_nBNexPDA);
	pRow->PutValue(ltcInUse, m_nIUNexPDA);
	pRow->PutValue(ltcPending, m_nPNexPDA);
	pdlLicenseType->AddRowAtEnd(pRow, NULL);

	// (b.eyers 2015-02-24) - PLID 64990 - If user has the SupportBought permission, they can access all the items in this dropdown. 
	// User's with only the SupportSwapLicenses can only access the above six items
	if (GetCurrentUserPermissions(bioSupportBought, sptWrite) || m_bIsTestAccount)
	{

		//add # doctors row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidNumDoctors));
		pRow->PutValue(ltcName, "# Doctors");
		pRow->PutValue(ltcBought, m_nBDoctors);
		pRow->PutValue(ltcInUse, m_nIUDoctors);
		pRow->PutValue(ltcPending, m_nPDoctors);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		//add the databases row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidDatabases));
		pRow->PutValue(ltcName, "Database");
		pRow->PutValue(ltcBought, m_nBDatabases);
		pRow->PutValue(ltcInUse, m_nIUDatabases);
		pRow->PutValue(ltcPending, m_nPDatabases);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		//add the EMR Prov row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidEMRProv));
		pRow->PutValue(ltcName, "EMR Prov");
		pRow->PutValue(ltcBought, m_nBEMRProv);
		pRow->PutValue(ltcInUse, m_nIUEMRProv);
		pRow->PutValue(ltcPending, m_nPEMRProv);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		//add the NxWeb row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidNxWeb));
		pRow->PutValue(ltcName, "NxWeb");
		pRow->PutValue(ltcBought, m_nBNxWeb);
		pRow->PutValue(ltcInUse, m_nIUNxWeb);
		pRow->PutValue(ltcPending, m_nPNxWeb);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);



		// (s.tullis 2013-11-15 10:48) - PLID 57553 - Nexcloud in Practice support tab change number of licences
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidNxCloudUser));
		pRow->PutValue(ltcName, "NxCloud #Users");
		pRow->PutValue(ltcBought, 0);
		pRow->PutValue(ltcInUse, m_nIUNxCloudUser);
		pRow->PutValue(ltcPending, 0);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);


		// (s.tullis 2013-11-15 10:48) - PLID 57553 - Nexcloud in Practice support tab change number of licences
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidNxCloudVPN));
		pRow->PutValue(ltcName, "NxCloud #VPN");
		pRow->PutValue(ltcBought, 0);
		pRow->PutValue(ltcInUse, m_nIUNxCloudVPN);
		pRow->PutValue(ltcPending, 0);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);


		// (r.ries 2013-01-24) - PLID 54820 - add the Max # of Patients
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidMaxPatients));
		pRow->PutValue(ltcName, "Max # of Patients");
		pRow->PutValue(ltcBought, 0);
		pRow->PutValue(ltcInUse, m_nIUNumPatients);
		pRow->PutValue(ltcPending, 0);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (r.ries 2013-01-24) - PLID 54820 - add the Max # of Login attempts
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidMaxLogins));
		pRow->PutValue(ltcName, "Max # of Login attempts");
		pRow->PutValue(ltcBought, 0);
		pRow->PutValue(ltcInUse, m_nIULoginAttempts);
		pRow->PutValue(ltcPending, 0);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (r.ries 2013-01-09) - PLID 54643 - add the Module row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidModules));
		pRow->PutValue(ltcName, "Modules");
		pRow->PutValue(ltcBought, 0);
		pRow->PutValue(ltcInUse, 0);
		pRow->PutValue(ltcPending, 0);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (r.farnworth 2013-07-19) - PLID 57522 - add the Licensed Prescribers row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidLicensedPres));
		pRow->PutValue(ltcName, "Licensed Prescribers");
		pRow->PutValue(ltcBought, m_nBLicPres);
		pRow->PutValue(ltcInUse, m_nIULicPres);
		pRow->PutValue(ltcPending, m_nPLicPres);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (r.farnworth 2013-07-19) - PLID 57522 - add the Mid-Level Prescribers row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidMidPres));
		pRow->PutValue(ltcName, "Mid-Level Prescribers");
		pRow->PutValue(ltcBought, m_nBMidPres);
		pRow->PutValue(ltcInUse, m_nIUMidPres);
		pRow->PutValue(ltcPending, m_nPMidPres);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (r.farnworth 2013-07-19) - PLID 57522 - add the Mid-Level Prescribers row
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidStaffPres));
		pRow->PutValue(ltcName, "Staff Prescribers");
		pRow->PutValue(ltcBought, m_nBStaffPres);
		pRow->PutValue(ltcInUse, m_nIUStaffPres);
		pRow->PutValue(ltcPending, m_nPStaffPres);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (z.manning 2014-01-31 14:19) - PLID 55147
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidDictation));
		pRow->PutValue(ltcName, "Dictation Users");
		pRow->PutValue(ltcBought, m_nBDictation);
		pRow->PutValue(ltcInUse, m_nIUDictation);
		pRow->PutValue(ltcPending, m_nPDictation);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);

		// (z.manning 2015-06-17 14:36) - PLID 66278
		pRow = m_pdlLicenseType->GetNewRow();
		pRow->PutValue(ltcID, long(ltidPortalProviders));
		pRow->PutValue(ltcName, "Portal Providers");
		pRow->PutValue(ltcBought, m_nBPortalProviders);
		pRow->PutValue(ltcInUse, m_nIUPortalProviders);
		pRow->PutValue(ltcPending, m_nPPortalProviders);
		pdlLicenseType->AddRowAtEnd(pRow, NULL);
	}
}

// (r.ries 2013-1-3) - PLID 50881 - License swap function
void CSupportEditLicenseDlg::PopulateLicenseTypeSwapDataList(_DNxDataListPtr pdlLicenseType)
{
	IRowSettingsPtr pRow = pdlLicenseType->GetNewRow();

	if(!(m_strExcludeFromSwapList.Compare("Workstation License") && m_strExcludeFromSwapList.Compare("iPad License") && m_strExcludeFromSwapList.Compare("Conc. TS License")))
	{
		//build the datalist
		// add license row
		if(m_strExcludeFromSwapList.Compare("Workstation License"))
		{
			pRow->PutValue(ltcID, long(ltidLicenses));
			pRow->PutValue(ltcName, "Workstation License");
			pRow->PutValue(ltcBought, m_nBLicenses);
			pRow->PutValue(ltcInUse, m_nIULicenses);
			pRow->PutValue(ltcPending, m_nPLicenses);
			pdlLicenseType->AddRowAtEnd(pRow, NULL);
		}

		// (z.manning 2012-06-12 15:53) - PLID 50878 - Added iPad licenses
		if(m_strExcludeFromSwapList.Compare("iPad License"))
		{
			pRow = m_pdlLicenseType->GetNewRow();
			pRow->PutValue(ltcID, long(ltidIpads));
			pRow->PutValue(ltcName, "iPad License");
			pRow->PutValue(ltcBought, m_nBIpads);
			pRow->PutValue(ltcInUse, m_nIUIpads);
			pRow->PutValue(ltcPending, m_nPIpads);
			pdlLicenseType->AddRowAtEnd(pRow, NULL);
		}

		//add conc ts license row
		if(m_strExcludeFromSwapList.Compare("Conc. TS License"))
		{
			pRow = m_pdlLicenseType->GetNewRow();
			pRow->PutValue(ltcID, long(ltidConcLicenses));
			pRow->PutValue(ltcName, "Conc. TS License");
			pRow->PutValue(ltcBought, m_nBConc);
			pRow->PutValue(ltcInUse, m_nIUConc);
			pRow->PutValue(ltcPending, m_nPConc);
			pdlLicenseType->AddRowAtEnd(pRow, NULL);
		}

	}
	// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
	else if (!(m_strExcludeFromSwapList.Compare("Palm Pilot") && m_strExcludeFromSwapList.Compare("NexPDA") && m_strExcludeFromSwapList.Compare("NexSync")))
	{
		if (m_strExcludeFromSwapList.Compare("Palm Pilot"))
		{
			pRow = m_pdlLicenseType->GetNewRow();
			pRow->PutValue(ltcID, long(ltidPalmPilot));
			pRow->PutValue(ltcName, "Palm Pilot");
			pRow->PutValue(ltcBought, m_nBPalm);
			pRow->PutValue(ltcInUse, m_nIUPalm);
			pRow->PutValue(ltcPending, m_nPPalm);
			pdlLicenseType->AddRowAtEnd(pRow, NULL);
		}

		if (m_strExcludeFromSwapList.Compare("NexPDA"))
		{
			pRow = m_pdlLicenseType->GetNewRow();
			pRow->PutValue(ltcID, long(ltidNexPDA));
			pRow->PutValue(ltcName, "NexPDA");
			pRow->PutValue(ltcBought, m_nBNexPDA);
			pRow->PutValue(ltcInUse, m_nIUNexPDA);
			pRow->PutValue(ltcPending, m_nPNexPDA);
			pdlLicenseType->AddRowAtEnd(pRow, NULL);
		}

		if (m_strExcludeFromSwapList.Compare("NexSync"))
		{
			pRow = m_pdlLicenseType->GetNewRow();
			pRow->PutValue(ltcID, long(ltidNexsync));
			pRow->PutValue(ltcName, "NexSync");
			pRow->PutValue(ltcBought, m_nBNexSync);
			pRow->PutValue(ltcInUse, m_nIUNexSync);
			pRow->PutValue(ltcPending, m_nPNexSync);
			pdlLicenseType->AddRowAtEnd(pRow, NULL);
		}
	}
}

void CSupportEditLicenseDlg::OnBnClickedOk()
{
	try {
		RefreshHelpText(); // (r.ries 2013-01-25) - PLID 54820 - Must be called to ensure error messages are current
		BOOL bSuccessfulAudit = true; // (r.ries 2013-01-28) - PLID 54643 - Must use to decide whether user successfully audited module change
		//make sure they have valid data selected before continuing
		if(m_bInvalid)
		{
			CString str;
			GetDlgItemText(IDC_HELP_TEXT_STATIC, str);
			AfxMessageBox(str);
			return;
		}
		//find out what license was changed and change the license amount accordingly

		// (b.eyers 2015-03-05) - PLID 65097 - Warn users that are about to swap that there is a lockout on swapping if they continue
		if (IsDlgButtonChecked(IDC_SWAP_LICENSE)) {

			BOOL bIsGracePeriodEnded = false;

			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 UsersT.Username, NxClientsLicenseSwapHistoryT.SwapDate, NxClientsLicenseSwapHistoryT.ExpirationDate "
				"FROM NxClientsLicenseSwapHistoryT "
				"LEFT JOIN UsersT ON NxClientsLicenseSwapHistoryT.SwappedByUserID = UsersT.PersonID "
				"LEFT JOIN( "
				"SELECT ClientID, Min(SwapDate) AS FirstSwapDate "
				"FROM NxClientsLicenseSwapHistoryT "
				"WHERE ExpirationDate > GetDate() "
				"GROUP BY ClientID "
				"HAVING Min(SwapDate) > DATEADD(minute, -{INT}, GetDate()) "
				") AS GracePeriodQ ON NxClientsLicenseSwapHistoryT.ClientID = GracePeriodQ.ClientID "
				"WHERE NxClientsLicenseSwapHistoryT.ClientID = {INT} AND NxClientsLicenseSwapHistoryT.ExpirationDate > GetDate() AND GracePeriodQ.ClientID Is Null "
				"ORDER BY NxClientsLicenseSwapHistoryT.ExpirationDate ASC", (long)GRACE_PERIOD_MINUTES, m_nPatientID);

			if (!rs->eof) {
				bIsGracePeriodEnded = true;
			}

			if (bIsGracePeriodEnded && GetCurrentUserPermissions(bioSupportSwapLicenses) && !GetCurrentUserPermissions(bioSupportBought) && !m_bIsTestAccount) {
				//10 minute grace period has ended

				CString strWarn = "The 10 minute grace period has ended. This swap will not be saved.";
				MessageBox(strWarn, "Lockout Warning", MB_ICONEXCLAMATION | MB_OK);
				CNxDialog::OnCancel();
				return; 
			}
			else {

				CString strWarn = "Swapping licenses will begin a lockout period before they can be swapped again. "
					"Only support managers can swap licenses until the lockout period expires.\n\n"
					"You will have a 10 minute grace period to swap additional licenses. Are you sure you wish to swap these licenses now?";

				//different warning for managers
				if (GetCurrentUserPermissions(bioSupportBought, sptWrite) || m_bIsTestAccount) {
					COleDateTime dtNow = GetRemoteServerTime();
					COleDateTimeSpan dtSpan(LOCKOUT_DAYS, 0, 0, 0);
					dtNow += dtSpan;

					strWarn.Format("Swapping licenses will begin a lockout period of %li days, "
						"during which time only support managers can swap licenses. "
						"Technical support staff will be unable to swap licenses again until %s.\n\n", 
						LOCKOUT_DAYS, FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime));

					if (!bIsGracePeriodEnded) {
						strWarn.Append("Technical support staff will have a 10 minute grace period to swap additional licenses. ");
					}
					strWarn.Append("Are you sure you wish to swap the licenses now?");
				}

				if (IDNO == MessageBox(strWarn, "Lockout Warning", MB_ICONQUESTION | MB_YESNO)) {
					return;
				}

				bBeginNewLockoutPeriod = true;
			}
			rs->Close();
		}
		else
			bBeginNewLockoutPeriod = false;


		switch(m_ltidLicenseType)
		{
		case ltidLicenses:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBLicenses += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPLicenses += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPLicenses -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atSwap:
					{
						m_nBLicenses -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nLicenseEditAmt = GetDlgItemInt(IDC_LICENSE_AMOUNT);
						break;
					}
				}
			}
			break;
		case ltidIpads: // (z.manning 2012-06-12 15:55) - PLID 50878
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBIpads += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPIpads += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPIpads -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atSwap:
					{
						m_nBIpads -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nLicenseEditAmt = GetDlgItemInt(IDC_LICENSE_AMOUNT);
						break;
					}
				}
			}
			break;
		case ltidConcLicenses:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBConc += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPConc += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPConc -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atSwap:
					{
						m_nBConc -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nLicenseEditAmt = GetDlgItemInt(IDC_LICENSE_AMOUNT);
						break;
					}
				}
			}
			break;
		case ltidNumDoctors:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBDoctors += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPDoctors += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPDoctors -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;

		case ltidNexsync:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBNexSync += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPNexSync += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPNexSync -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
				case atSwap:
					{
						m_nBNexSync -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nLicenseEditAmt = GetDlgItemInt(IDC_LICENSE_AMOUNT);
						break;
					}
				}
			}
			break;

		case ltidDictation: // (z.manning 2014-01-31 14:20) - PLID 55147
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBDictation += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPDictation += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPDictation -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;

		case ltidPalmPilot:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBPalm += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPPalm += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPPalm -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
				case atSwap:
					{
						m_nBPalm -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nLicenseEditAmt = GetDlgItemInt(IDC_LICENSE_AMOUNT);
						break;
					}
				}
			}
			break;
		case ltidNexPDA:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBNexPDA += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPNexPDA += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPNexPDA -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
				case atSwap:
					{
						m_nBNexPDA -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nLicenseEditAmt = GetDlgItemInt(IDC_LICENSE_AMOUNT);
						break;
					}
				}
			}
			break;
		case ltidDatabases:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBDatabases += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPDatabases += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPDatabases -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
		case ltidEMRProv:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBEMRProv += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPEMRProv += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPEMRProv -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
		case ltidNxWeb:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBNxWeb += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPNxWeb += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPNxWeb -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
			
			
			case ltidNxCloudUser:// (s.tullis 2014-01-30 15:32) - PLID 57553 - Nexcloud in Practice support tab change number of licences
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nIUNxCloudUser += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nIUNxCloudUser += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nIUNxCloudUser -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
			case ltidNxCloudVPN:// (s.tullis 2014-01-30 15:32) - PLID 57553 - Nexcloud in Practice support tab change number of licences
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nIUNxCloudVPN += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nIUNxCloudVPN += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nIUNxCloudVPN -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;

		case ltidMaxPatients:  // (r.ries 2013-01-24) - PLID 54820
			{
				switch(m_atType)
				{
				case atAddOn:
					{
						m_nIUNumPatients += (m_nIUNumPatients == -1) ? 1 : 0;
						m_nIUNumPatients += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					}
					break;
				case atDeactivate:
					{
						m_nIUNumPatients = -1; // (r.ries 2013-01-24) - PLID 54820 - We want to set the value to -1 if we're deactivating
					}
					break;
				}
			}
			break;
		case ltidMaxLogins:  // (r.ries 2013-01-24) - PLID 54820
			{
				switch(m_atType)
				{
				case atAddOn:
					{
						m_nIULoginAttempts += (m_nIULoginAttempts == -1) ? 1 : 0;
						m_nIULoginAttempts += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					}
					break;
				case atDeactivate:
					{
						m_nIULoginAttempts -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
						m_nIULoginAttempts -= (m_nIULoginAttempts == 0) ? 1 : 0;
					}
					break;
				}
			}
			break;
		case ltidModules: // (r.ries 2013-01-09) - PLID 54643 - Perform actions on Modules
			{
				bSuccessfulAudit = PerformModuleAction(); // (r.ries 2013-01-28) - PLID 54643 - If PMA fails, OnOK will not execute
			}
			break;
		//(r.farnworth 2013-07-19) PLID 57522
		case ltidLicensedPres:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBLicPres += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPLicPres += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPLicPres -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
		//(r.farnworth 2013-07-19) PLID 57522
		case ltidMidPres:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBMidPres += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPMidPres += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPMidPres -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
		//(r.farnworth 2013-07-19) PLID 57522
		case ltidStaffPres:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBStaffPres += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPStaffPres += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPStaffPres -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;

		// (z.manning 2015-06-17 14:37) - PLID 66278
		case ltidPortalProviders:
			{
				switch(m_atType)
				{
				case atAddOn:
					m_nBPortalProviders += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atReactivate:
					m_nPPortalProviders += GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				case atDeactivate:
					m_nPPortalProviders -= GetDlgItemInt(IDC_LICENSE_AMOUNT);
					break;
				}
			}
			break;
		default:
			break;
		}
		if (bSuccessfulAudit)
		{
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OnBnClickedAddOn()
{
	try {
		UINT nChecked;
		GetDlgItemCheck(IDC_ADD_ON, nChecked); // (r.ries 2013-02-27) - PLID 54956 - This had to be added to prevent a race condition that was sporadically causing an infinite loop
		if (m_atType != atAddOn && m_atType != atSetExpire) // (r.ries 2013-02-4) - PLID 54956 - If user switches between Addon and SetExpire, allow them to retain the list
		{
			m_ModuleIDs.RemoveAll();
		}
		if (m_atType != atAddOn)
		{
			m_atType = atAddOn;
		}
		// (r.ries 2013-01-09) - PLID 54643 - Pull up Multi Select dialog to allow user to pick what modules to add
		if (m_ltidLicenseType == ltidModules)
		{
			if(nChecked)
			{
				OpenAddonModulesMultiselect();
			}
		}
		RefreshHelpText();
		EnableControls();
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OnBnClickedReactivate()
{
	try {
		UINT nChecked;
		GetDlgItemCheck(IDC_REACTIVATE, nChecked); // (r.ries 2013-02-27) - PLID 54956 - This had to be added to prevent a race condition that was sporadically causing an infinite loop
		if (m_atType != atReactivate)
		{
			m_ModuleIDs.RemoveAll();
			m_atType = atReactivate;
		}
		// (r.ries 2013-1-9) - PLID 53277 - Pull up Multi Select dialog to allow user to pick what modules to add
		if (m_ltidLicenseType == ltidModules)
		{
			if(nChecked)
			{
				OpenReactivateModulesMultiselect();
			}
		}
		RefreshHelpText();
		EnableControls();
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OnBnClickedDeactivate()
{
	try {
		UINT nChecked;
		GetDlgItemCheck(IDC_DEACTIVATE, nChecked); // (r.ries 2013-02-27) - PLID 54956 - This had to be added to prevent a race condition that was sporadically causing an infinite loop
		if (m_atType != atDeactivate)
		{
			m_ModuleIDs.RemoveAll();
			m_atType = atDeactivate;
		}
		// (r.ries 2013-1-9) - PLID 53277 - Pull up Multi Select dialog to allow user to pick what modules to add
		if (m_ltidLicenseType == ltidModules)
		{
			if(nChecked)
			{
				OpenDeactivateModulesMultiselect();
			}
		}
		if (m_ltidLicenseType == ltidMaxPatients) // (r.ries 2013-01-24) - PLID 54820 - Display number we're deactivting
		{
			SetDlgItemInt(IDC_LICENSE_AMOUNT, m_nIUNumPatients);
		}
		if(m_ltidLicenseType == ltidMaxLogins)
		{
			SetDlgItemInt(IDC_LICENSE_AMOUNT, m_nIULoginAttempts);
		}
		RefreshHelpText();
		EnableControls();
	}NxCatchAll(__FUNCTION__);
}

// (r.ries 2013-1-3) - PLID 50881 - License swap function
void CSupportEditLicenseDlg::OnBnClickedSwapLicense()
{
	try {
		m_atType = atSwap;
		RefreshHelpText();
		EnableControls();
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OnBnClickedEnableExpiration() // (r.ries 2013-02-4) - PLID 54956
{
	try {
		UINT nChecked;
		GetDlgItemCheck(IDC_ENABLE_EXPIRATION, nChecked); // (r.ries 2013-02-27) - PLID 54956 - This had to be added to prevent a race condition that was sporadically causing an infinite loop
		if (m_atType != atSetExpire && m_atType != atAddOn) // (r.ries 2013-02-4) - PLID 54956 - If user switches between Addon and SetExpire, allow them to retain the list
		{
			m_ModuleIDs.RemoveAll();
		}
		if (m_atType != atSetExpire)
		{
			m_atType = atSetExpire;
		}
		if(nChecked)
		{
			OpenAddonModulesMultiselect();
		}
		EnableControls();
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OnBnClickedNoExpiration()
{
	EnableControls();
}

void CSupportEditLicenseDlg::OnBnClickedSetExpiration()
{
	EnableControls();
}

void CSupportEditLicenseDlg::RefreshHelpText()
{
	try {
		//Build the help text depending on the selected control's values
		CString strActionType, strLicenseType, strHelpText;
		long nLicenseAmount, nInUse, nPending, nBought;
		COleDateTime dtModExpDate = m_pModuleExpDate->GetDateTime(); // (r.ries 2013-02-14) - PLID 54956

		m_bRedHelpText = FALSE; // (b.eyers 2015-03-05) - PLID 65099 - reset the text color back to black

		//Get the license type
		// (b.eyers 2015-03-10) - PLID 64986 - Get the InUse, Bought, and Pending to check if the license has anything to swap
		IRowSettingsPtr pRow = m_pdlLicenseType->GetCurSel();
		if(pRow)
		{
			strLicenseType = VarString(pRow->GetValue(ltcName));
			nInUse = VarLong(pRow->GetValue(ltcInUse));
			nPending = VarLong(pRow->GetValue(ltcPending));
			nBought = VarLong(pRow->GetValue(ltcBought));
		}
		else
		{
			//no row selected
			m_bInvalid = TRUE;
			SetDlgItemText(IDC_HELP_TEXT_STATIC, "Please select a valid license type."); // (b.eyers 2015-03-02) - PLID 64986 - Added a period to the end of the sentence
			return;
		}

		//Get the action type
		switch(m_atType)
		{
		case atAddOn:
			strActionType = "Add on";
			break;
		case atReactivate:
			strActionType = "Reactivate";
			break;
		case atDeactivate:
			strActionType = "Deactivate";
			break;
		case atSwap:
			strActionType = "Move"; // (b.eyers 2015-03-02) - PLID 64986 - Changed the word 'swap' to 'move', fits better into the sentence structure
			break;
		case atSetExpire: // (r.ries 2013-02-14) - PLID 54956
			strActionType = "set an expiration date on";
			break;
		default:
			// UNKNOWN TYPE!
			m_bInvalid = TRUE;
			SetDlgItemText(IDC_HELP_TEXT_STATIC, "Please select a valid license action.");  // (b.eyers 2015-03-02) - PLID 64986 - Added a period to the end of the sentence
			return;
		}

		//Get the amount to change
		if(GetDlgItemInt(IDC_LICENSE_AMOUNT) == 0 && m_ltidLicenseType != ltidModules)
		{
			// (b.eyers 2015-03-10) - PLID 64986 - Check if there are licenses to deactivate/reactivate/swap and update helptext if not
			if (m_atType == atSwap && ((nInUse + nPending) <= 0)) {
				SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are no licenses of this type to swap from. Please select a different license type or action type.");
			}
			else if (m_atType == atReactivate && ((nBought - (nInUse + nPending)) <= 0)) {
				SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are no deactivated licenses of this type to reactivate. Please select a different license type or action type."); 
			}
			else if (m_atType == atDeactivate && ((nInUse + nPending) <= 0)) {
				SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are no active licenses of this type to deactivate. Please select a different license type or action type."); // (r.ries 2012-02-15) - PLID 54767 - This wording makes more sense now

			}
			else 
				SetDlgItemText(IDC_HELP_TEXT_STATIC, "Please enter a number in the 'Number of Licenses Affected' field."); // (b.eyers 2015-03-02) - PLID 64986 - Updated the wording
			m_bInvalid = TRUE;
			return;
		}
		else
		{
			nLicenseAmount = GetDlgItemInt(IDC_LICENSE_AMOUNT);
		}

		//if we get here we must have a valid help text,  so show it
		// (b.eyers 2015-03-02) - PLID 64986 - All the changes in this if statement are updates to the wording of the sentence
		// Added the display of the new total bought for add/remove/swap and correct pluralization of the word license
		if(m_ltidLicenseType != ltidModules)
		{
			strHelpText.Format("You are attempting to %s %li %s", strActionType, nLicenseAmount, strLicenseType); 
			if (!(m_ltidLicenseType == ltidLicenses || m_ltidLicenseType == ltidConcLicenses || m_ltidLicenseType == ltidIpads)) 
			{
				strHelpText.Append(" license");
			}
			if (nLicenseAmount>1)
			{
				strHelpText.Append("s");
			}
			if (m_atType == atAddOn)
			{
				CString strCountText;
				long nNewCount;
				nNewCount = VarLong(pRow->GetValue(ltcBought));
				nNewCount = nNewCount + nLicenseAmount;
				strCountText.Format(" for a total bought of %li", nNewCount);
				strHelpText.Append(strCountText);
			}
			else if (m_atType == atDeactivate)
			{
				CString strCountText;
				long nNewCount;
				nNewCount = VarLong(pRow->GetValue(ltcBought));
				nNewCount = nNewCount - nLicenseAmount;
				strCountText.Format(" for a total bought of %li", nNewCount);
				strHelpText.Append(strCountText);
			}
			if(m_atType == atSwap && m_pdlLicenseTypeSwap->CurSel == NULL) // (r.ries 2013-1-25) - PLID 50881 - Module swap help text
			{
				SetDlgItemText(IDC_HELP_TEXT_STATIC, "Please select a valid license to swap with."); //64986 - added period
				m_bInvalid = TRUE;
			}
			else if (m_atType == atSwap)
			{
				CString strTemp, strSwapLicense, strCountText;
				long nNewCount, nSwapCount;
				IRowSettingsPtr pRowSwap = m_pdlLicenseTypeSwap->GetCurSel();
				strSwapLicense = VarString(pRowSwap->GetValue(ltcName));
				strTemp.Format(" to %s", strSwapLicense);
				if (!(m_ltidLicenseTypeSwap == ltidLicenses || m_ltidLicenseTypeSwap == ltidConcLicenses || m_ltidLicenseTypeSwap == ltidIpads))
				{
					strTemp.Append(" license");
				}
				if (nLicenseAmount>1)
				{
					strTemp.Append("s.\r\n");
				}
				else
				{
					strTemp.Append(".\r\n");
				}
				strHelpText.Append(strTemp);
				nNewCount = VarLong(pRow->GetValue(ltcBought));
				nSwapCount = VarLong(pRowSwap->GetValue(ltcBought));
				nNewCount = nNewCount - nLicenseAmount;
				nSwapCount = nSwapCount + nLicenseAmount;
				strCountText.Format("The new total bought will be %li %s", nNewCount, strLicenseType);
				if (!(m_ltidLicenseType == ltidLicenses || m_ltidLicenseType == ltidConcLicenses || m_ltidLicenseType == ltidIpads))
				{
					strCountText.Append(" license");
				}
				if (nNewCount>1)
				{
					strCountText.Append("s");
				}
				strHelpText.Append(strCountText);
				strCountText.Format(" and %li %s", nSwapCount, strSwapLicense);
				if (!(m_ltidLicenseTypeSwap == ltidLicenses || m_ltidLicenseTypeSwap == ltidConcLicenses || m_ltidLicenseTypeSwap == ltidIpads))
				{
					strCountText.Append(" license");
				}
				if (nSwapCount>1)
				{
					strCountText.Append("s.");
				}
				else
				{
					strCountText.Append(".");
				}
				strHelpText.Append(strCountText);
				SetDlgItemText(IDC_HELP_TEXT_STATIC, strHelpText);
				m_bInvalid = FALSE;
			}
			else
			{
				strHelpText.Append(".");
				SetDlgItemText(IDC_HELP_TEXT_STATIC, strHelpText);
				m_bInvalid = FALSE;
			}
		}
		else// (r.ries 2013-1-9) - PLID 53277 - Module help text
		{
			if(m_ModuleIDs.GetCount() < 1) // If they didn't select any modules from the multiselect
			{
				m_bInvalid = TRUE;
				SetDlgItemText(IDC_HELP_TEXT_STATIC, "It seems you have not selected any modules. \r\n\r\nPlease re-select an action type and select the modules you wish to change.");
				EnableControls();
			}
			else
			{
				CString strModuleText;
				int nModuleIdLength = m_ModuleIDs.GetCount();
				for(int i = 0; i < nModuleIdLength; i++)
				{
					strModuleText.Append(m_ModuleIDs[i]);
					if(nModuleIdLength > 1 && i != nModuleIdLength)
					{
						if(i < nModuleIdLength-2)
						{
							strModuleText.Append(", ");
						}
						if(i == nModuleIdLength-2)
						{
							strModuleText.Append(" and ");
						}
					}
				}
				strModuleText.Append(" Module");
				if(nModuleIdLength>1)
				{
					strModuleText.Append("s");
				}
				strHelpText.Format("You are attempting to %s %s", strActionType, strModuleText);
				if(m_atType == atSetExpire) // (r.ries 2013-02-14) - PLID 54956 - Set help text for expiration date
				{
					if(IsDlgButtonChecked(IDC_SET_EXPIRATION))
					{
						strHelpText.Append(" of " + dtModExpDate.Format("%m/%d/%Y") + ".");
					}
					else
					{
						strHelpText.Append(" to no expiration date.");
					}
				}
				// (b.eyers 2015-03-02) - PLID 64986 - Added a period to the end of the sentence
				else
				{
					strHelpText.Append(".");
				}
				SetDlgItemText(IDC_HELP_TEXT_STATIC, strHelpText);
				m_bInvalid = FALSE;
				if(IsDlgButtonChecked(IDC_SET_EXPIRATION)) // (r.ries 2013-02-14) - PLID 54956 - Ensure the exp date makes some sense
				{
					if (dtModExpDate.GetYear() < 1990 || dtModExpDate.GetStatus() != COleDateTime::valid)
					{
						m_bInvalid = TRUE;
						SetDlgItemText(IDC_HELP_TEXT_STATIC, "Please type a valid expiration date above the year 1990.");
					}
				}
				if (m_atType == atSetExpire && !(IsDlgButtonChecked(IDC_NO_EXPIRATION) || IsDlgButtonChecked(IDC_SET_EXPIRATION))) // (r.ries 2013-02-14) - PLID 54956 - Make the user
				{
					m_bInvalid = TRUE;
					SetDlgItemText(IDC_HELP_TEXT_STATIC, "Please choose whether you want to set or clear the expiration date.");
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// set initial bought
void CSupportEditLicenseDlg::SetBLicenses(long nLicenses)
{
	m_nBLicenses = nLicenses;
}

// (z.manning 2012-06-12 15:51) - PLID 50878
void CSupportEditLicenseDlg::SetBIpads(long nIpadsBought)
{
	m_nBIpads = nIpadsBought;
}

void CSupportEditLicenseDlg::SetBConc(long nConc)
{
	m_nBConc = nConc;
}

void CSupportEditLicenseDlg::SetBDoctors(long nDoctors)
{
	m_nBDoctors = nDoctors;
}

void CSupportEditLicenseDlg::SetBNexSync(long nNexSync)
{
	m_nBNexSync = nNexSync;
}

// (z.manning 2014-01-31 14:20) - PLID 55147
void CSupportEditLicenseDlg::SetBDictation(const long nDictation)
{
	m_nBDictation = nDictation;
}

// (z.manning 2015-06-17 14:43) - PLID 66278
void CSupportEditLicenseDlg::SetBPortalProviders(const long nPortalProviders)
{
	m_nBPortalProviders = nPortalProviders;
}

void CSupportEditLicenseDlg::SetBPalm(long nPalm)
{
	m_nBPalm = nPalm;
}

void CSupportEditLicenseDlg::SetBNexPDA(long nNexPDA)
{
	m_nBNexPDA = nNexPDA;
}

void CSupportEditLicenseDlg::SetBDatabases(long nDatabases)
{
	m_nBDatabases = nDatabases;
}

void CSupportEditLicenseDlg::SetBEMRProv(long nEMRProv)
{
	m_nBEMRProv = nEMRProv;
}

// (r.ries 2012-01-22) - PLID 54767
void CSupportEditLicenseDlg::SetBNxWeb(long nNxWebBought)
{
	m_nBNxWeb = nNxWebBought;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetBLicensedPres(long nLicPresBought)
{
	m_nBLicPres = nLicPresBought;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetBMidPres(long nMidPresBought)
{
	m_nBMidPres = nMidPresBought;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetBStaffPres(long nStaffPresBought)
{
	m_nBStaffPres = nStaffPresBought;
}

// initial pending
void CSupportEditLicenseDlg::SetPLicenses(long nLicenses)
{
	m_nPLicenses = nLicenses;
}

// (z.manning 2012-06-12 15:51) - PLID 50878
void CSupportEditLicenseDlg::SetPIpads(long nIpadsPending)
{
	m_nPIpads = nIpadsPending;
}

void CSupportEditLicenseDlg::SetPConc(long nConc)
{
	m_nPConc = nConc;
}

void CSupportEditLicenseDlg::SetPDoctors(long nDoctors)
{
	m_nPDoctors = nDoctors;
}

void CSupportEditLicenseDlg::SetPNexSync(long nNexSync)
{
	m_nPNexSync = nNexSync;
}

// (z.manning 2014-01-31 14:20) - PLID 55147
void CSupportEditLicenseDlg::SetPDictation(const long nDictation)
{
	m_nPDictation = nDictation;
}

// (z.manning 2015-06-17 14:44) - PLID 66278
void CSupportEditLicenseDlg::SetPPortalProviders(const long nPortalProviders)
{
	m_nPPortalProviders = nPortalProviders;
}

void CSupportEditLicenseDlg::SetPPalm(long nPalm)
{
	m_nPPalm = nPalm;
}

void CSupportEditLicenseDlg::SetPNexPDA(long nNexPDA)
{
	m_nPNexPDA = nNexPDA;
}

void CSupportEditLicenseDlg::SetPDatabases(long nDatabases)
{
	m_nPDatabases = nDatabases;
}

void CSupportEditLicenseDlg::SetPEMRProv(long nEMRProv)
{
	m_nPEMRProv = nEMRProv;
}

// (r.ries 2012-01-22) - PLID 54767
void CSupportEditLicenseDlg::SetPNxWeb(long nNxWebPending)
{
	m_nPNxWeb = nNxWebPending;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetPLicensedPres(long nLicPresPending)
{
	m_nPLicPres = nLicPresPending;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetPMidPres(long nMidPresPending)
{
	m_nPMidPres = nMidPresPending;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetPStaffPres(long nStaffPresPending)
{
	m_nPStaffPres = nStaffPresPending;
}


// initial in use
void CSupportEditLicenseDlg::SetIULicenses(long nLicenses)
{
	m_nIULicenses = nLicenses;
}

// (z.manning 2012-06-12 15:51) - PLID 50878
void CSupportEditLicenseDlg::SetIUIpads(long nIpadsInUse)
{
	m_nIUIpads = nIpadsInUse;
}

void CSupportEditLicenseDlg::SetIUConc(long nConc)
{
	m_nIUConc = nConc;
}

void CSupportEditLicenseDlg::SetIUDoctors(long nDoctors)
{
	m_nIUDoctors = nDoctors;
}

void CSupportEditLicenseDlg::SetIUNexSync(long nNexSync)
{
	m_nIUNexSync = nNexSync;
}

// (z.manning 2014-01-31 14:21) - PLID 55147
void CSupportEditLicenseDlg::SetIUDictation(const long nDictation)
{
	m_nIUDictation = nDictation;
}

// (z.manning 2015-06-17 14:45) - PLID 66278
void CSupportEditLicenseDlg::SetIUPortalProviders(const long nPortalProviders)
{
	m_nIUPortalProviders = nPortalProviders;
}

void CSupportEditLicenseDlg::SetIUPalm(long nPalm)
{
	m_nIUPalm = nPalm;
}

void CSupportEditLicenseDlg::SetIUNexPDA(long nNexPDA)
{
	m_nIUNexPDA = nNexPDA;
}

void CSupportEditLicenseDlg::SetIUDatabases(long nDatabases)
{
	m_nIUDatabases = nDatabases;
}

void CSupportEditLicenseDlg::SetIUEMRProv(long nEMRProv)
{
	m_nIUEMRProv = nEMRProv;
}

// (r.ries 2012-01-22) - PLID 54767
void CSupportEditLicenseDlg::SetIUNxWeb(long nNxWebInUse)
{
	m_nIUNxWeb = nNxWebInUse;
}

// (r.ries 2013-01-24) - PLID 54820
void CSupportEditLicenseDlg::SetIUNumPatients(long nNumPatientsInUse)
{
	m_nIUNumPatients = nNumPatientsInUse;
}

// (r.ries 2013-01-24) - PLID 54820
void CSupportEditLicenseDlg::SetIULoginAttempts(long nLoginAttemptsInUse)
{
	m_nIULoginAttempts = nLoginAttemptsInUse;
}

// (s.tullis 2014-01-30 15:29) - PLID 57553 - Nexcloud in Practice support tab change number of licences
void CSupportEditLicenseDlg::SetIUNxCloudUsers(long nNxCloudUsers)
{
	m_nIUNxCloudUser = nNxCloudUsers;
}
// (s.tullis 2014-01-30 15:29) - PLID 57553 - Nexcloud in Practice support tab change number of licences
void CSupportEditLicenseDlg::SetIUNxCloudVPN(long nNxCloudVPN)
{
	m_nIUNxCloudVPN = nNxCloudVPN;
}


// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetIULicensedPres(long nLicPresInUse)
{
	m_nIULicPres = nLicPresInUse;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetIUMidPres(long nMidPresInUse)
{
	m_nIUMidPres = nMidPresInUse;
}

// (r.farnworth 2013-07-19) - PLID 57522
void CSupportEditLicenseDlg::SetIUStaffPres(long nStaffPresInUse)
{
	m_nIUStaffPres = nStaffPresInUse;
}


BEGIN_EVENTSINK_MAP(CSupportEditLicenseDlg, CNxDialog)
	ON_EVENT(CSupportEditLicenseDlg, IDC_LICENSE_TYPE, 16, CSupportEditLicenseDlg::SelChosenLicenseType, VTS_DISPATCH)
	ON_EVENT(CSupportEditLicenseDlg, IDC_LICENSE_TYPE, 1, CSupportEditLicenseDlg::SelChangingLicenseType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CSupportEditLicenseDlg, IDC_LICENSE_TYPE_SWAP, 1, CSupportEditLicenseDlg::SelChangingLicenseTypeSwap, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CSupportEditLicenseDlg, IDC_LICENSE_TYPE_SWAP, 16, CSupportEditLicenseDlg::SelChosenLicenseTypeSwap, VTS_DISPATCH)
	ON_EVENT(CSupportEditLicenseDlg, IDC_MODULE_EXP_DATE, 1, CSupportEditLicenseDlg::KillFocusModuleExpDate, VTS_NONE) // (r.ries 2013-02-14) - PLID 54956
END_EVENTSINK_MAP()

void CSupportEditLicenseDlg::SelChosenLicenseType(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow)
		{
			m_ltidLicenseType = (eLicenseTypeID)VarLong(pRow->GetValue(ltcID));
			m_pdlLicenseTypeSwap->Clear();
			m_strExcludeFromSwapList = pRow->GetValue(ltcName);
			PopulateLicenseTypeSwapDataList(m_pdlLicenseTypeSwap);
			if (m_ltidLicenseType == ltidMaxPatients && m_atType == atDeactivate && m_nIUNumPatients != -1) // (r.ries 2013-01-24) - PLID 54820 - Display number we're deactivting
			{
				SetDlgItemInt(IDC_LICENSE_AMOUNT, m_nIUNumPatients);
			}
			if(m_ltidLicenseType == ltidMaxLogins && m_atType == atDeactivate && m_nIULoginAttempts != -1)
			{
				SetDlgItemInt(IDC_LICENSE_AMOUNT, m_nIULoginAttempts);
			}
			RefreshHelpText();
			EnableControls();
		}
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::SelChangingLicenseType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		if(*lppNewSel == NULL)
		{
			//dont let them select no row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OnEnChangeLicenseAmount()
{
	try {
		RefreshHelpText();
		SetUpperLimit();
	}NxCatchAll(__FUNCTION__)
}

// (r.ries 2013-1-3) - PLID 50881 - License swap function
void CSupportEditLicenseDlg::SelChangingLicenseTypeSwap(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL)
		{
			//Don't let them select no row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::SelChosenLicenseTypeSwap(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow)
		{
			m_ltidLicenseTypeSwap = (eLicenseTypeID)VarLong(pRow->GetValue(ltcID));
		}
		RefreshHelpText(); // (b.eyers 2015-03-02) - PLID 64986 - Refresh the helptext after a new item is selected in the swap license dropdown
	}NxCatchAll(__FUNCTION__);
}

 // (a.levy 2013-03-11 12:08) - PLID 55561 - Modified function to call re-usable object to filter modules
 // (r.ries 2013-01-09) - PLID 54643 - Open Multiselect and fill with proper modules
void CSupportEditLicenseDlg::OpenMultiselectAndFilterModules(CString strWhere)
{
	try {
		HRESULT hRes;
		CMultiSelectDlg dlg(this, "");
		CString strIDField, strValueField, strDescription; 

		

		strIDField = "Modules";
		strValueField = "Modules";
		strDescription = "Please select Modules you would like to " + HelpActionEnumToString(m_atType);

		dlg.m_strNameColTitle = "Module";
		dlg.SetSizingConfigRT("SupportEditLicenseAddOn");

		if (!m_ModuleIDs.IsEmpty()) // (r.ries 2013-01-28) - PLID 54643 - Multiselect should pre-fill with what I already picked
		{
			CVariantArray strModuelsToPreselect;
			for(int i = 0; i < m_ModuleIDs.GetSize(); i++)
			{
				strModuelsToPreselect.Add(_variant_t(m_ModuleIDs[i]));
			}
			dlg.PreSelect(strModuelsToPreselect);
		}
        hRes = dlg.Open(SupportUtils::ModuleFieldPerClient(m_nPatientID), strWhere, strIDField, strValueField, strDescription, 1); // (a.levy 2013-03-11 12:08) - PLID 55561 - OBO for Module filter 

		if (hRes == IDOK)
		{
			m_ModuleIDs.RemoveAll();
			CVariantArray varModuleIDs;
			dlg.FillArrayWithIDs(varModuleIDs);

			for(int i = 0; i < varModuleIDs.GetSize(); i++)
			{
				m_ModuleIDs.Add(varModuleIDs[i]);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.ries 2013-01-09) - PLID 54643 - Converts action to plain text, past tense string description
CString CSupportEditLicenseDlg::ActionEnumToString(eActionType nAction)
{
	switch(nAction)
	{
	case atAddOn:
		return "Bought";
		break;
	case atReactivate:
		return "Activated";
		break;
	case atDeactivate:
		return "De-activated";
		break;
	case atSwap:
		return "Swapped";
		break;
	default:
		return "Unknown";
		break;
	}
}

// (r.ries 2013-01-14) - PLID 54643 - May be used to help text, converts enum to plain text string
CString CSupportEditLicenseDlg::HelpActionEnumToString(eActionType nAction)
{
	switch(nAction)
	{
	case atAddOn:
		return "Add on";
		break;
	case atReactivate:
		return "Reactivate";
		break;
	case atDeactivate:
		return "Deactivate";
		break;
	case atSetExpire:
		return "Set Expiration";
		break;
	default:
		return "Error";
		break;
	}
}

// (r.ries 2013-01-09) - PLID 54643 - Open Audit dialog, perform batch action on modules
BOOL CSupportEditLicenseDlg::PerformModuleAction()
{
	try {
		CAuditSupportDlg dlg(this);
		if(dlg.DoModal() == IDOK) { //If Audit text is accepted

			CString strField, strAudit, strNewValue, strNewDescription;
			COleDateTime dtModExpDate = m_pModuleExpDate->GetDateTime(); // (r.ries 2013-02-14) - PLID 54956

			strNewDescription = ActionEnumToString(m_atType); // Used to translate action to correct database representation
			switch(m_atType)
			{
			case atAddOn:
				strNewValue = "1"; //Bought
				break;
			case atReactivate:
				strNewValue = "2"; //In use
				break;
			case atDeactivate:
				strNewValue = "3"; //De-activated
				break;
			}

			CStringArray strModuleColumns;
			SupportUtils::ConvertPrettyNametoColumnName(strModuleColumns, m_ModuleIDs);

			CParamSqlBatch sqlBatch;
			int nModuleIdLength = strModuleColumns.GetSize();
			_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM NxClientsT WHERE PersonID = {INT}", m_nPatientID); // Select these rows to get old module value
			if(!prs->eof) {
				for(int i = 0; i < nModuleIdLength; i++)  //Construct list of items to be updated
				{
					if (IsDlgButtonChecked(IDC_ENABLE_EXPIRATION)) // (r.ries 2013-02-14) - PLID 54956 - Set the expiration date
					{
						CString strTemp = strModuleColumns[i] + "Expiration";
						CString strOldExpression = "";
						if (AdoFldDateTime(prs, strTemp, COleDateTime()) != COleDateTime()) // If there was an expiration date before, let's audit it (AdoFldDateTime(prs, strTemp).GetStatus() == COleDateTime::valid)
						{
							COleDateTime dtOldExpiration = AdoFldDateTime(prs, strTemp);
							strOldExpression = dtOldExpiration.Format("%m/%d/%Y");
						}
						else // If there wasn't, let's say so
						{
							strOldExpression = "None";
						}
						if (IsDlgButtonChecked(IDC_NO_EXPIRATION)) //If we want to get rid of the expiration date, set the field to NULL
						{
							sqlBatch.Add("UPDATE NxClientsT SET {CONST_STRING}Expiration = NULL WHERE PersonID = {INT} ", strModuleColumns[i], m_nPatientID);
							strNewValue = "None";
						}
						else
						{
							sqlBatch.Add("UPDATE NxClientsT SET {CONST_STRING}Expiration = {OLEDATETIME} WHERE PersonID = {INT} ", strModuleColumns[i], dtModExpDate, m_nPatientID);
							strNewValue = dtModExpDate.Format("%m/%d/%Y");
						}
						if (strOldExpression.Compare(strNewValue))
						{
							SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), m_ModuleIDs[i] + "Expiration", strOldExpression, strNewValue, _Q(dlg.m_strText));
						}
					}
					else
					{
						long oldStatus = AdoFldLong(prs, strModuleColumns[i]); // Get the old status of particular module column
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), m_ModuleIDs[i],  SupportUtils::StatusIntToStringDescription(oldStatus), strNewDescription, _Q(dlg.m_strText));
						sqlBatch.Add("UPDATE NxClientsT SET {CONST_STRING} = {STRING} WHERE PersonID = {INT} ", strModuleColumns[i], strNewValue, m_nPatientID);
					}
				}
			}
			sqlBatch.Execute(GetRemoteData());
			return TRUE; // Return true if audit succeeds
		}
		else
		{
			return FALSE; // So we can go back to the dialog if it doesn't succeed
		}
	}NxCatchAll(__FUNCTION__);
	return FALSE;
}

// (r.ries 2013-01-23) - PLID 54767 -  consolidate enable/disable
void CSupportEditLicenseDlg::EnableControls()
{
	// Even though they shouldn't be able to get here without permissions, set access here anyway just in case
	// (b.eyers 2015-02-24) - PLID 64990 - Added a check of the SupportSwapLicenses permission
	if(!(GetCurrentUserPermissions(bioSupportBought, sptWrite) || GetCurrentUserPermissions(bioSupportSwapLicenses, sptWrite)) && !m_bIsTestAccount) 
	{
		GetDlgItem(IDC_ADD_ON)->EnableWindow(FALSE);
		GetDlgItem(IDC_REACTIVATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEACTIVATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(FALSE);
		GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(FALSE);
		GetDlgItem(IDC_LICENSE_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	// (b.eyers 2015-02-24) - PLID 64990 - If user has the SupportSwapLicenses permission but not the SupportBought permission, they can only access swapping
	else if (GetCurrentUserPermissions(bioSupportSwapLicenses, sptWrite) && !GetCurrentUserPermissions(bioSupportBought, sptWrite) && !m_bIsTestAccount)
	{
		GetDlgItem(IDC_ADD_ON)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_REACTIVATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DEACTIVATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_TYPE_SWAP)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_TYPE)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_AMOUNT)->ShowWindow(SW_SHOW); 
		GetDlgItem(IDC_LICENSE_ADJUST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ADJUST_DESC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SWAP_LICENSE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MODULE_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NO_EXPIRATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SET_EXPIRATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MODULE_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ENABLE_EXPIRATION)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_ADD_ON)->EnableWindow(TRUE);
		GetDlgItem(IDC_REACTIVATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_DEACTIVATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_TYPE_SWAP)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_TYPE)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_LICENSE_AMOUNT)->ShowWindow(SW_SHOW); // (r.ries 2013-02-14) - PLID 54956 - enable/disable hide/show expiration items
		GetDlgItem(IDC_LICENSE_ADJUST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ADJUST_DESC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SWAP_LICENSE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MODULE_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NO_EXPIRATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SET_EXPIRATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MODULE_EXP_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ENABLE_EXPIRATION)->ShowWindow(SW_HIDE);
	}
		// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
		if (m_ltidLicenseType == ltidLicenses || m_ltidLicenseType == ltidConcLicenses || m_ltidLicenseType == ltidIpads || m_ltidLicenseType == ltidNexPDA || m_ltidLicenseType == ltidPalmPilot || m_ltidLicenseType == ltidNexsync)
		{
			if (m_atType == atSwap) // (r.ries 2013-02-20) - PLID 55244 - Disable swap multiselect unless associated radio is checked
			{
				GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(TRUE);
			}
			else
			{
				GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(FALSE);
			}
		}
		else
		{
			if (m_atType == atSwap) // If it's not a license you can swap, but swap is selected, disable everything and clear the action
			{
				m_atType = atNoAction;
			}
			m_pdlLicenseTypeSwap->Clear();
			m_pdlLicenseTypeSwap->Enabled = FALSE;
			GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(FALSE);
			GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(FALSE);
			CheckDlgButton(IDC_SWAP_LICENSE, BST_UNCHECKED);
		}

		if (m_ltidLicenseType == ltidModules)
		{
			CheckDlgButton(IDC_SWAP_LICENSE, BST_UNCHECKED);
			GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(FALSE);
			SetDlgItemText(IDC_LICENSE_AMOUNT,"");
			GetDlgItem(IDC_MODULE_EXP_DATE)->ShowWindow(SW_SHOW); // (r.ries 2013-02-14) - PLID 54956 - Bunch of new stuff to enable/disable, hide/show
			GetDlgItem(IDC_LICENSE_AMOUNT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LICENSE_ADJUST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SWAP_LICENSE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LICENSE_TYPE_SWAP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADJUST_DESC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NO_EXPIRATION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SET_EXPIRATION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MODULE_EXP_DATE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ENABLE_EXPIRATION)->ShowWindow(SW_SHOW);
			if (IsDlgButtonChecked(IDC_ENABLE_EXPIRATION) == BST_CHECKED)
			{
				GetDlgItem(IDC_NO_EXPIRATION)->EnableWindow(TRUE);
				GetDlgItem(IDC_SET_EXPIRATION)->EnableWindow(TRUE);
				GetDlgItem(IDC_MODULE_EXP_DATE)->EnableWindow(TRUE);
				if (IsDlgButtonChecked(IDC_SET_EXPIRATION) == BST_CHECKED)
				{
					GetDlgItem(IDC_MODULE_EXP_DATE)->EnableWindow(TRUE);
				}
				else
				{
					GetDlgItem(IDC_MODULE_EXP_DATE)->EnableWindow(FALSE);
				}
			}
			else
			{
				GetDlgItem(IDC_NO_EXPIRATION)->EnableWindow(FALSE);
				GetDlgItem(IDC_SET_EXPIRATION)->EnableWindow(FALSE);
				GetDlgItem(IDC_MODULE_EXP_DATE)->EnableWindow(FALSE);
				CheckDlgButton(IDC_NO_EXPIRATION, BST_UNCHECKED);
				CheckDlgButton(IDC_SET_EXPIRATION, BST_UNCHECKED);
			}
			if (m_ModuleIDs.IsEmpty()) // (r.ries 2013-02-20) - PLID 55244 - Don't want multiselect dialog to popup when "modules" is selected, so set "No action"
			{
				m_atType = atNoAction;
			}
		}
		else // m_ltidLicenseType != ltidModules // (r.ries 2013-02-14) - PLID 54956 - disable expiration items and set to no action if we're not editing modules
		{
			if (m_atType == atSetExpire)
			{
				m_atType = atNoAction;
			}
			GetDlgItem(IDC_NO_EXPIRATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_SET_EXPIRATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_MODULE_EXP_DATE)->EnableWindow(FALSE);
			CheckDlgButton(IDC_NO_EXPIRATION, BST_UNCHECKED);
			CheckDlgButton(IDC_SET_EXPIRATION, BST_UNCHECKED);
		}
		if (m_ltidLicenseType == ltidMaxPatients || m_ltidLicenseType == ltidMaxLogins)
		{
			GetDlgItem(IDC_REACTIVATE)->EnableWindow(FALSE);
			CheckDlgButton(IDC_REACTIVATE, BST_UNCHECKED);
			if(m_atType == atReactivate) // (r.ries 2013-01-24) - PLID 54820 - Can't "Reactivate" max MaxPatients or MaxLogins
			{
				m_atType = atAddOn;
				CheckDlgButton(IDC_DEACTIVATE, BST_UNCHECKED);
			}

			// (r.ries 2013-01-24) - PLID 54820 - If already "disabled", do not allow to disable again
			if((m_ltidLicenseType == ltidMaxPatients && m_nIUNumPatients == -1) || (m_ltidLicenseType == ltidMaxLogins && m_nIULoginAttempts == -1))
			{
				CheckDlgButton(IDC_DEACTIVATE, BST_UNCHECKED);
				GetDlgItem(IDC_DEACTIVATE)->EnableWindow(FALSE);
				m_atType = atAddOn;
			}
			if (m_atType == atDeactivate) //Don't want user to edit this value, it's all or nuthin'
			{
				GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(FALSE);
			}
		}
	//}

	switch (m_atType) // (r.ries 2013-01-28) - PLID 54820 - Looks better, makes more sense
	{
		case atAddOn:
			CheckDlgButton(IDC_ADD_ON, BST_CHECKED);
			break;
		case atReactivate:
			CheckDlgButton(IDC_REACTIVATE, BST_CHECKED);
			break;
		case atDeactivate:
			CheckDlgButton(IDC_DEACTIVATE, BST_CHECKED);
			break;
		case atSwap:
			CheckDlgButton(IDC_SWAP_LICENSE, BST_CHECKED);
			break;
		case atSetExpire: // (r.ries 2013-02-14) - PLID 54956
			CheckDlgButton(IDC_ENABLE_EXPIRATION, BST_CHECKED);
			break;
		default:
			{
				CheckDlgButton(IDC_ADD_ON, BST_UNCHECKED);
				CheckDlgButton(IDC_REACTIVATE, BST_UNCHECKED);
				CheckDlgButton(IDC_DEACTIVATE, BST_UNCHECKED);
				CheckDlgButton(IDC_SWAP_LICENSE, BST_UNCHECKED);
				// (r.ries 2013-02-14) - PLID 54956 - disable expiration items
				CheckDlgButton(IDC_ENABLE_EXPIRATION, BST_UNCHECKED);
				GetDlgItem(IDC_NO_EXPIRATION)->EnableWindow(FALSE);
				GetDlgItem(IDC_SET_EXPIRATION)->EnableWindow(FALSE);
				GetDlgItem(IDC_MODULE_EXP_DATE)->EnableWindow(FALSE);
				CheckDlgButton(IDC_NO_EXPIRATION, BST_UNCHECKED);
				CheckDlgButton(IDC_SET_EXPIRATION, BST_UNCHECKED);
			}
			break;
	}

	SetUpperLimit(); // More enabling/disabling 
	RefreshHelpText();
}

//set the upper limit of the spin control
void CSupportEditLicenseDlg::SetUpperLimit()
{
	try {
		//need to get selected values first
		long nBought, nInUse, nPending;
		IRowSettingsPtr pRow = m_pdlLicenseType->GetCurSel();
		if(pRow)
		{
			nBought = VarLong(pRow->GetValue(ltcBought));
			nInUse = VarLong(pRow->GetValue(ltcInUse));
			nPending = VarLong(pRow->GetValue(ltcPending));
		}
		else
		{
			//no row selected so just return and keep limit at MAX_LICENSE
			return;
		}

		//get the action type and set the upper limit
		switch(m_atType)
		{
		case atAddOn:
			//set to MAX_LIMIT
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			m_LicenseAdjust.SetRange(0, MAX_LIMIT);	
			//if the license amount is currently set above the max limit ( 1000 ) set back to max limit
			if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) > MAX_LIMIT)
			{
				SetDlgItemInt(IDC_LICENSE_AMOUNT, MAX_LIMIT);
			}
			break;
		case atReactivate:
			//limit is set as bought - (InUse + Pending)
			//check for out of range issues due to bad data ( if there are more in use/pending than bought licenses )
			if((nBought - (nInUse + nPending)) <= 0)
			{
				if(!(m_ltidLicenseType == ltidModules))
				{
					m_LicenseAdjust.SetRange(0, 0);
					GetDlgItem(IDOK)->EnableWindow(FALSE);
					GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(FALSE);
					SetDlgItemText(IDC_LICENSE_AMOUNT,0);
					SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are no deactivated licenses of this type to reactivate. Please select a different license type or action type."); // (r.ries 2012-02-15) - PLID 54767 - This wording makes more sense now
				}
				if(m_ltidLicenseType != ltidMaxLogins && m_ltidLicenseType != ltidMaxPatients) // (r.ries 2013-01-24) - PLID 54820 - You can never reacivate these licenseTypes, don't display this error
				{
					if((nBought - (nInUse + nPending)) < 0) // (r.ries 2012-02-15) - PLID 54767 - Had to change this to fix enable/disable issue
					{
						SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are more pending / in-use licenses than bought licenses, check data for accuracy. You will not be able to reactivate any more licenses for this license type."); // (b.eyers 2015-03-02) - PLID 64986 - Added periods to the end of the sentences
					}
				}
			}
			else
			{
				GetDlgItem(IDOK)->EnableWindow(TRUE);
				m_LicenseAdjust.SetRange(0, (short)(nBought - (nInUse + nPending)));
				//check to see if our license amount text is already higher than the new upper limit,  if so change the value to the new limit
				if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) > (nBought - (nInUse + nPending)))
				{
					SetDlgItemInt(IDC_LICENSE_AMOUNT, (nBought - (nInUse + nPending)));
				}
				//check for negative number from bad data, set to 0 if thats the case
				else if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) < 0)
				{
					SetDlgItemInt(IDC_LICENSE_AMOUNT, 0);
				}
			}
			break;
		case atDeactivate:
			//limit is set as <= InUse + pending value
			//check for InUse + pending being equal to a negative number ( bad data ) 
			if((nInUse + nPending) <= 0)
			{
				if(!(m_ltidLicenseType == ltidModules))
				{
					m_LicenseAdjust.SetRange(0, 0);
					GetDlgItem(IDOK)->EnableWindow(FALSE);
					GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(FALSE);
					SetDlgItemText(IDC_LICENSE_AMOUNT,0);
					SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are no active licenses of this type to deactivate. Please select a different license type or action type."); // (r.ries 2012-02-15) - PLID 54767 - This wording makes more sense now
				}
				if(!((m_ltidLicenseType == ltidMaxLogins && (nInUse) == -1) || (m_ltidLicenseType == ltidMaxPatients && (nInUse) == -1))) // (r.ries 2013-01-24) - PLID 54820 - Special case because -1 indicates deactivated
				{
					if((nInUse + nPending) < 0)
					{
						SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are more pending deactivations than licenses in use, check data for accuracy. You will not be able to deactivate any more licenses for this license type."); // (b.eyers 2015-03-02) - PLID 64986 - Added periods to the end of the sentences
					}
				}
			}
			else
			{
				GetDlgItem(IDOK)->EnableWindow(TRUE);
				m_LicenseAdjust.SetRange(0, (short)(nInUse + nPending));
				//check to see if our license amount text is already higher than the new upper limit,  if so change the value to the new limit
				if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) > (nInUse + nPending))
				{
					SetDlgItemInt(IDC_LICENSE_AMOUNT, (nInUse + nPending));
				}
				//check for negative number from bad data, set to 0 if thats the case
				else if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) < 0)
				{
					SetDlgItemInt(IDC_LICENSE_AMOUNT, 0);
				}
			}
			break;
		case atSwap: // (r.ries 2013-1-3) - PLID 50881 - License swap function
			{
				//limit is set as <= InUse + pending value
				//check for InUse + pending being equal to a negative number ( bad data ) 
				// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
				if(m_ltidLicenseType == ltidLicenses || m_ltidLicenseType == ltidIpads || m_ltidLicenseType == ltidConcLicenses || m_ltidLicenseType == ltidNexPDA || m_ltidLicenseType == ltidPalmPilot || m_ltidLicenseType == ltidNexsync) // (r.ries 2013-01-24) - PLID 54820 - Only these special categories can be swapped
				{
					if((nInUse + nPending) <= 0)
					{
						m_LicenseAdjust.SetRange(0, 0);
						GetDlgItem(IDOK)->EnableWindow(FALSE);
						GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(FALSE);
						SetDlgItemText(IDC_LICENSE_AMOUNT,0);
						SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are no licenses of this type to swap from. Please select a different license type or action type."); // (r.ries 2012-02-15) - PLID 54767 - This wording makes more sense now
						if((nInUse + nPending) < 0)
						{
							SetDlgItemText(IDC_HELP_TEXT_STATIC, "There are more pending deactivations than licenses in use, check data for accuracy. You will not be able to swap any more licenses for this license type."); // (b.eyers 2015-03-02) - PLID 64986 - Added periods to the end of the sentences
						}
					}
					else
					{
						GetDlgItem(IDOK)->EnableWindow(TRUE);
						m_LicenseAdjust.SetRange(0, (short)(nInUse + nPending));
						//check to see if our license amount text is already higher than the new upper limit,  if so change the value to the new limit
						if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) > (nInUse + nPending))
						{
							SetDlgItemInt(IDC_LICENSE_AMOUNT, (nInUse + nPending));
						}
						//check for negative number from bad data, set to 0 if thats the case
						else if((long)GetDlgItemInt(IDC_LICENSE_AMOUNT) < 0)
						{
							SetDlgItemInt(IDC_LICENSE_AMOUNT, 0);
						}
					}
				}
				else
				{
					m_atType = atAddOn;
				}
			}
			break;
		default:
			// UNKNOWN TYPE! just return
			return;
			}
	}NxCatchAll(__FUNCTION__);
}

void CSupportEditLicenseDlg::OpenAddonModulesMultiselect() // (r.ries 2013-01-25) - PLID 54643 - Separated to new function
{
	CString strWhere = "Status = 0"; //Not Bought
	OpenMultiselectAndFilterModules(strWhere);
}

void CSupportEditLicenseDlg::OpenReactivateModulesMultiselect() // (r.ries 2013-01-25) - PLID 54643 - Separated to new function
{
	CString strWhere = "Status = 3"; //De-activated
	OpenMultiselectAndFilterModules(strWhere);
}

void CSupportEditLicenseDlg::OpenDeactivateModulesMultiselect() // (r.ries 2013-01-25) - PLID 54643 - Separated to new function
{
	CString strWhere = "Status = 2 OR Status = 1"; //Activated or Bought
	OpenMultiselectAndFilterModules(strWhere);
}
void CSupportEditLicenseDlg::OnBnClickedSetModExpDate() // (r.ries 2013-02-14) - PLID 54956
{
	RefreshHelpText();
}

void CSupportEditLicenseDlg::OnBnClickedClearModExpDate() // (r.ries 2013-02-14) - PLID 54956
{
	RefreshHelpText();
}

void CSupportEditLicenseDlg::OnEnKillfocusLicenseAmount() // (r.ries 2013-02-14) - PLID 54956
{
	SetUpperLimit();
}

void CSupportEditLicenseDlg::KillFocusModuleExpDate() // (r.ries 2013-02-14) - PLID 54956
{
	RefreshHelpText();
}

// (b.eyers 2015-03-05) - PLID 65100 - Check to see if there is a lockout in effect
void CSupportEditLicenseDlg::RefreshLicenseLockoutControls()
{
	BOOL bIsManagerUser = FALSE;

	try {

		if (GetCurrentUserPermissions(bioSupportBought, sptWrite) || m_bIsTestAccount)
		{
			bIsManagerUser = TRUE;
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 UsersT.Username, NxClientsLicenseSwapHistoryT.SwapDate, NxClientsLicenseSwapHistoryT.ExpirationDate "
			"FROM NxClientsLicenseSwapHistoryT "
			"LEFT JOIN UsersT ON NxClientsLicenseSwapHistoryT.SwappedByUserID = UsersT.PersonID "
			"LEFT JOIN( "
			"SELECT ClientID, Min(SwapDate) AS FirstSwapDate "
			"FROM NxClientsLicenseSwapHistoryT "
			"WHERE ExpirationDate > GetDate() "
			"GROUP BY ClientID "
			"HAVING Min(SwapDate) > DATEADD(minute, -{INT}, GetDate()) "
			") AS GracePeriodQ ON NxClientsLicenseSwapHistoryT.ClientID = GracePeriodQ.ClientID "
			"WHERE NxClientsLicenseSwapHistoryT.ClientID = {INT} AND NxClientsLicenseSwapHistoryT.ExpirationDate > GetDate() AND GracePeriodQ.ClientID Is Null "
			"ORDER BY NxClientsLicenseSwapHistoryT.ExpirationDate DESC", (long)GRACE_PERIOD_MINUTES, m_nPatientID);

		if (!rs->eof) {
			//a lockout is in effect

			//GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(bIsManagerUser);
			//GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(bIsManagerUser);
			GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(bIsManagerUser);
			GetDlgItem(IDC_LICENSE_ADJUST)->EnableWindow(bIsManagerUser);
			GetDlgItem(IDC_LICENSE_TYPE)->EnableWindow(bIsManagerUser);
			GetDlgItem(IDOK)->EnableWindow(bIsManagerUser);
			//if disabled, uncheck them if they are checked
			if (!bIsManagerUser) {
				CheckDlgButton(IDC_SWAP_LICENSE, FALSE);
				CheckDlgButton(IDC_LICENSE_TYPE_SWAP, FALSE);
				CheckDlgButton(IDC_LICENSE_AMOUNT, FALSE);
				CheckDlgButton(IDC_LICENSE_ADJUST, FALSE);
				CheckDlgButton(IDC_LICENSE_TYPE, FALSE);
			}

			// (b.eyers 2015-03-05) - PLID 65099 - Warning if a lockout is in effect, using the existing helptext label
			CString strLabel = "Licenses have been recently swapped.\n"
				"Only support managers can swap this client's licenses at this time.";
			if (bIsManagerUser) {
				strLabel.Format("Licenses were swapped by %s on %s.\n"
					"Only support managers can swap this client's licenses until %s.",
					AdoFldString(rs->Fields, "Username", "<unknown user>"),
					FormatDateTimeForInterface(AdoFldDateTime(rs->Fields, "SwapDate"), DTF_STRIP_SECONDS, dtoDateTime),
					FormatDateTimeForInterface(AdoFldDateTime(rs->Fields, "ExpirationDate"), DTF_STRIP_SECONDS, dtoDateTime));
			}
			m_bRedHelpText = TRUE; 
			SetDlgItemText(IDC_HELP_TEXT_STATIC, strLabel);

			// (b.eyers 2015-03-05) PLID 65101 - Show the Clear Lockout button if the user is a manager and there is a lockout in effect
			GetDlgItem(IDC_BTN_CLEAR_LOCKOUT_PERIOD)->ShowWindow(bIsManagerUser ? SW_SHOWNA : SW_HIDE);

		}
		else {
			//no lockout is in effect

			//GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(TRUE);
			//GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(TRUE);
			GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_LICENSE_ADJUST)->EnableWindow(TRUE);
			GetDlgItem(IDC_LICENSE_TYPE)->EnableWindow(TRUE);

			// (b.eyers 2015-03-05) PLID 65101 - Hide the Clear Lockout button if there is no lockout in effect
			GetDlgItem(IDC_BTN_CLEAR_LOCKOUT_PERIOD)->ShowWindow(SW_HIDE);
		}
		rs->Close();

		return;

	}NxCatchAll(__FUNCTION__);

	//if this fails, disable the checkboxes to be safe, unless we know the user is a manager
	ASSERT(FALSE);
	//GetDlgItem(IDC_SWAP_LICENSE)->EnableWindow(bIsManagerUser);
	//GetDlgItem(IDC_LICENSE_TYPE_SWAP)->EnableWindow(bIsManagerUser);
	GetDlgItem(IDC_LICENSE_AMOUNT)->EnableWindow(bIsManagerUser);
	GetDlgItem(IDC_LICENSE_ADJUST)->EnableWindow(bIsManagerUser);
	GetDlgItem(IDC_LICENSE_TYPE)->EnableWindow(bIsManagerUser);
	GetDlgItem(IDOK)->EnableWindow(bIsManagerUser);
	//if disabled, uncheck them if they are checked
	if (!bIsManagerUser) {
		CheckDlgButton(IDC_SWAP_LICENSE, FALSE);
		CheckDlgButton(IDC_LICENSE_TYPE_SWAP, FALSE);
		CheckDlgButton(IDC_LICENSE_AMOUNT, FALSE);
		CheckDlgButton(IDC_LICENSE_ADJUST, FALSE);
		CheckDlgButton(IDC_LICENSE_TYPE, FALSE);
	}
	GetDlgItem(IDC_BTN_CLEAR_LOCKOUT_PERIOD)->ShowWindow(SW_HIDE);
}

HBRUSH CSupportEditLicenseDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pDC && pWnd->GetSafeHwnd()) {
		switch (pWnd->GetDlgCtrlID()) {
			// (b.eyers 2015-03-05) - PLID 65099 - show the label text as red if it's the lockout warning
		case IDC_HELP_TEXT_STATIC:
			if (m_bRedHelpText){
				pDC->SetTextColor(RGB(255, 0, 0));
			}
			break;
		}
	}

	// Return the default
	return hbr;
}

// (b.eyers 2015-03-05) PLID 65101 - Clear Lockout button 
void CSupportEditLicenseDlg::OnBtnClearLockoutPeriod()
{
	try {

		if (!GetCurrentUserPermissions(bioSupportBought, sptWrite) && !m_bIsTestAccount) {
			//this button should not be shown to non-managers
			ASSERT(FALSE);
			AfxMessageBox("Only support managers may clear license swap lockout periods.");
			GetDlgItem(IDC_BTN_CLEAR_LOCKOUT_PERIOD)->ShowWindow(SW_HIDE);
			return;
		}

		if (IDNO == MessageBox("Clearing the lockout period will allow all users to swap this client's licenses again.\n\n"
			"Are you sure you wish to end the current lockout period?", "Lockout Warning", MB_ICONQUESTION | MB_YESNO)) {
			return;
		}

		//update all active lockout periods to expire right now
		//it is possible for there to be multiple active lockouts
		ExecuteParamSql("UPDATE NxClientsLicenseSwapHistoryT "
			"SET ExpirationDate = GetDate(), LockoutEndedByUserID = {INT} "
			"WHERE ClientID = {INT} AND ExpirationDate > GetDate()", GetCurrentUserID(), m_nPatientID);

		RefreshLicenseLockoutControls();
		RefreshHelpText();

	}NxCatchAll(__FUNCTION__);
}
