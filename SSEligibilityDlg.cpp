// SSEligibilityDlg.cpp : implementation file
//

// (j.fouts 2013-09-19 11:55) - PLID 58701 - Created

#include "stdafx.h"
#include "Practice.h"
#include "SSEligibilityDlg.h"
#include "PrescriptionUtilsNonAPI.h"

enum ResponseListColumns
{
	rlcIsFailure, 			//BIT NOT NULL
	rlcFormularyProvider, 	//NVARCHAR(60) NOT NULL
	rlcPlanName, 			//NVARCHAR(60) NOT NULL
	rlcStatus,				//NVARCHAR(20) NULL
	rlcHelpMatchPatientMessage, //NVARCHAR(264) NOT NULL - TES 9/23/2013 - PLID 58359
	rlcName,				//NVARCHAR(133) NOT NULL
	rlcAddress,				//NVARCHAR(160) NULL
	rlcPatientDateOfBirth,	//DATETIME NULL
	rlcGender,				//NVARCHAR(6) NOT NULL
	rlcActive,				//BIT NOT NULL
	rlcDate,				//DATETIME NOT NULL
	rlcPharmacyCoverage,	//NVACHAR(MAX) NULL
};

// CSSEligibilityDlg dialog

IMPLEMENT_DYNAMIC(CSSEligibilityDlg, CNxDialog)


CSSEligibilityDlg::CSSEligibilityDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSSEligibilityDlg::IDD, pParent)
{

}

CSSEligibilityDlg::~CSSEligibilityDlg()
{
}

void CSSEligibilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SSELIGIBILITY_COLOR, m_nxcBackground);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CSSEligibilityDlg, CNxDialog)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CSSEligibilityDlg, CNxDialog)
	ON_EVENT(CSSEligibilityDlg, IDC_SSINSURANCE_SELECT_LIST, 18, CSSEligibilityDlg::RequeryFinishedNxdlResponseList, VTS_I2)
END_EVENTSINK_MAP()

// CSSEligibilityDlg message handlers

BOOL CSSEligibilityDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_nxdlResponseList = BindNxDataList2Ctrl(IDC_SSINSURANCE_SELECT_LIST, false);
		//TES 9/23/2013 - PLID 58359 - Added HelpMatchPatientMessage
		// (r.farnworth 2013-10-18 10:45) - PLID 59094 - Gender should not show anything when nothing is returned.

		m_nxdlResponseList->PutFromClause(AsBstr(
			"(SELECT "
				"PatientID, "
				"PBMName, "
				"PlanName, "
				"PatientFirstName + CASE WHEN PatientMiddleName <> '' THEN ' ' + PatientMiddleName ELSE '' END + CASE WHEN PatientLastName <> '' THEN ' ' + PatientLastName ELSE '' END + CASE WHEN PatientSuffix <> '' THEN ' ' + PatientSuffix ELSE '' END AS Name, "
				"PatientDateOfBirth, "
				"CASE PatientGender WHEN 'M' THEN 'Male' WHEN 'F' THEN 'Female' WHEN 'U' THEN 'Unknown' ELSE '' END AS Gender, "
				"PatientAddress1 + CASE WHEN PatientAddress2 <> '' THEN CHAR(13) + CHAR(10) + PatientAddress2 ELSE '' END + CASE WHEN PatientCity <> '' OR PatientState <> '' OR PatientZip <> '' THEN CHAR(13) + CHAR(10) ELSE '' END + PatientCity + CASE WHEN PatientState <> '' THEN ' ' + PatientState ELSE '' END + CASE WHEN PatientZip <> '' THEN ' ' + PatientZip ELSE '' END AS [Address], "
				"CAST(CASE WHEN SureScriptsEligibilityDetailT.Coverage = 1 THEN 1 ELSE 0 END AS BIT) AS Active, "
				"SureScriptsEligibilityDetailT.IsFailure, "
				"SureScriptsEligibilityResponseT.ReceivedDateUTC AS Date, "
				"SureScriptsEligibilityDetailT.HelpMatchPatientMessage, "
				"(SELECT STUFF((SELECT ',' + CHAR(13)+CHAR(10) + ' ' + (CASE WHEN  ServiceType =1 THEN 'Retail ' + "
					"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' "
					"END "
					"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"ELSE + '' "
					  "END) "
					"WHEN  ServiceType =2 THEN 'Mail Order ' "
					"+ "
					"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' "
					"END "
					 "+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"ELSE + '' "
					  "END) "
					"WHEN  ServiceType =3 THEN 'Speciality ' "
					"+ "
					"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' "
					"END "
					"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"ELSE + '' "
					   "END) "
					"WHEN  ServiceType =4 THEN 'Long Term ' "
					"+ "
					"CASE WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=1 THEN '(Active)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=2 THEN '(Inactive)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=3 THEN '(Out Of Pocket)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=4 THEN '(Non Covered)' "
					"WHEN SureScriptsEligibilityPharmacyCoverageT.CoverageStatus=5 THEN '(Could Not Process)' "
					"END "
					"+ (CASE  WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NOT NULL  THEN ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101)  + ' - ' + 'End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NOT NULL AND CoverageEndDate IS NULL  THEN  ' :[Start ' + CONVERT(VARCHAR(10), [CoverageStartDate], 101) + ']' "
							"WHEN [CoverageStartDate] IS NULL AND CoverageEndDate IS NOT NULL  THEN  ' :[End ' + CONVERT(VARCHAR(10), [CoverageEndDate], 101) + ']' "
							"ELSE + '' "
					  "END) "
					"END) "
					  "As PharmacyCov "
					"FROM [SureScriptsEligibilityPharmacyCoverageT] WHERE SureScriptsEligibilityPharmacyCoverageT.ResponseDetailID = SureScriptsEligibilityDetailT.ID    FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1, 3, '')) AS PharmacyCoverage "
			"FROM SureScriptsEligibilityDetailT "
			"INNER JOIN SureScriptsEligibilityResponseT "
				"ON SureScriptsEligibilityResponseT.ID = SureScriptsEligibilityDetailT.ResponseID "
			"INNER JOIN SureScriptsEligibilityRequestT "
				"ON SureScriptsEligibilityRequestT.ID = SureScriptsEligibilityResponseT.RequestID "
			"WHERE CASE WHEN (DATEDIFF( HH , SureScriptsEligibilityResponseT.ReceivedDateUTC ,GETUTCDATE()) <=  72   AND DATEDIFF( HH, SureScriptsEligibilityResponseT.ReceivedDateUTC,GETUTCDATE()) >= 0) THEN 0 ELSE 1 END = 0 ) DetailsQ"));

		m_nxdlResponseList->PutWhereClause(AsBstr(FormatString("DetailsQ.PatientID = %li", m_nPatientID)));
		m_nxdlResponseList->Requery();

		return TRUE;
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;
}

int CSSEligibilityDlg::DoModal(long nPatientID)
{
	try
	{
		m_nPatientID = nPatientID;
		return DoModal();
	}
	NxCatchAll(__FUNCTION__);

	return IDABORT;
}

int CSSEligibilityDlg::DoModal()
{
	try
	{
		return CNxDialog::DoModal();
	}
	NxCatchAll(__FUNCTION__);

	return IDABORT;
}

void CSSEligibilityDlg::RequeryFinishedNxdlResponseList(short nFlags)
{
	try
	{
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlResponseList->GetFirstRow();pRow != NULL;pRow = pRow->GetNextRow())
		{
			BOOL bFailure = VarBool(pRow->GetValue(rlcIsFailure));
			BOOL bActive = VarBool(pRow->GetValue(rlcActive));

			if(bFailure)
			{
				pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
				pRow->PutValue(rlcStatus, "Failure");
			}
			else if(!bActive)
			{
				pRow->PutBackColor(ERX_NO_RESULTS_COLOR);
				pRow->PutValue(rlcStatus, "Inactive");
			}
			else	
			{
				pRow->PutValue(rlcStatus, "Active");
			}

		}		
	}
	NxCatchAll(__FUNCTION__);
}