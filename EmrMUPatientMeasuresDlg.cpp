// EmrMUPatientMeasuresDlg.cpp : implementation file
#include "stdafx.h"
#include "EmrMUPatientMeasuresDlg.h"
#include "CCHITReportInfoListing.h" // (r.gonet 06/12/2013) - PLID 55151
#include "CCHITReportInfo.h" // (r.gonet 06/12/2013) - PLID 55151
#include "EmrRc.h"

//(e.lally 2012-02-28) PLID 48265 - Created

enum MeasureListColumns
{
	mlcCompleted =0,
	mlcDisplayName,
	mlcDescription,
};

// CEmrMUPatientMeasuresDlg dialog

IMPLEMENT_DYNAMIC(CEmrMUPatientMeasuresDlg, CNxDialog)

//(e.lally 2012-02-28) PLID 48265 - Take in a window title override text and reportInfoListing pointer which is used to load the list
CEmrMUPatientMeasuresDlg::CEmrMUPatientMeasuresDlg(CWnd* pParent /*=NULL*/, const CString& strWindowTitleOverride /* = "" */, CCHITReportInfoListing* pCchitReportListing /* = NULL */)
	: CNxDialog(CEmrMUPatientMeasuresDlg::IDD, pParent)
{
	m_strWindowTitle = strWindowTitleOverride;
	if(m_strWindowTitle.IsEmpty()){
		m_strWindowTitle = "Meaningful Use Measures";
	}
	m_pCchitReportListing = pCchitReportListing;
}

CEmrMUPatientMeasuresDlg::~CEmrMUPatientMeasuresDlg()
{
	if(m_hIconGreenCheck){ DestroyIcon((HICON)m_hIconGreenCheck); }
	if(m_hIconRedX){ DestroyIcon((HICON)m_hIconRedX); }
}

void CEmrMUPatientMeasuresDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CEmrMUPatientMeasuresDlg, CNxDialog)
END_MESSAGE_MAP()


// CEmrMUPatientMeasuresDlg message handlers

BOOL CEmrMUPatientMeasuresDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		SetWindowText(m_strWindowTitle);
		
		m_pMeasureList = BindNxDataList2Ctrl(this, IDC_EMR_PAT_MU_MEASURE_LIST, GetRemoteDataSnapshot(), false);

		m_hIconGreenCheck = (HICON)LoadImage(AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDI_CHECKMARK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		m_hIconRedX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
			MAKEINTRESOURCE(IDI_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		if(m_pCchitReportListing != NULL){
			CCCHITReportInfo report;
			NXDATALIST2Lib::IRowSettingsPtr pRow;

			//(e.lally 2012-02-28) PLID 48265 - Loop through our report results and display them to the user
			for(int i =0; i< m_pCchitReportListing->m_aryReports.GetSize(); i++){
				report = m_pCchitReportListing->m_aryReports[i];
				//The results has to have been calculated already and the patient has to have been part of the denominator to be eligible for that measure
				//(e.lally 2012-03-21) PLID 48707 - denominator can be greater than or equal to one
				if(report.HasBeenCalculated() && report.GetDenominator() >= 1){
					long nNumerator = report.GetNumerator();
					long nDenominator = report.GetDenominator();
					long nPercentToPass = report.GetPercentToPass(); 
					bool bPassed = false;

					//(e.lally 2012-03-21) PLID 48707 - Check if the percent to pass is unset. If so, the numerator should be 0 or 1, counting just this patient
					if(nPercentToPass < 0 && nNumerator >= 1){
						bPassed = true;
					}
					//(e.lally 2012-03-21) PLID 48707 - Check if the percent to pass is set, if so, check if numerator over denominator is greater than this percentage
						//This is used for measures like "prescriptions sent electronically". The current patient may have 5 Rx but only 3 were sent electronically.
						//We are only going to count this patient as fulfilling the requirement if they meet the overall percentage being tested against.
					//NOTE that the percent has to be MORE than percentToPass in order to qualify. So nPercentToPass = 40% really needs 40.1% (not sure what precision is actually used here) to be accepted
					else if(nPercentToPass >= 0 && nDenominator > 0 && ((nNumerator/(float)nDenominator)*100) > nPercentToPass){
						bPassed = true;
					}
					pRow = m_pMeasureList->GetNewRow();
					//(e.lally 2012-02-28) PLID 48265 - Green checkmark for complete, red X for incomplete
					pRow->PutValue(mlcCompleted, (long)(bPassed ? m_hIconGreenCheck : m_hIconRedX));
					//(e.lally 2012-03-21) PLID 48707 - Include the percent based on in the report name
					CString strDisplayName = report.m_strDisplayName;
					if(nPercentToPass >=0 ){
						strDisplayName += FormatString(" (>%li%%)", nPercentToPass);
					}
					pRow->PutValue(mlcDisplayName, _bstr_t(strDisplayName));
					pRow->PutValue(mlcDescription, _bstr_t(report.GetHelpGeneral()));

					m_pMeasureList->AddRowSorted(pRow, NULL);
				}	
			}
		}
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}