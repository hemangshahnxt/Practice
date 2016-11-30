// ConfigureReportViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "ConfigureReportViewDlg.h"
#include <GlobalLabUtils.h>

//TES 2/24/2012 - PLID 44841 - Created
// CConfigureReportViewDlg dialog

IMPLEMENT_DYNAMIC(CConfigureReportViewDlg, CNxDialog)

CConfigureReportViewDlg::CConfigureReportViewDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureReportViewDlg::IDD, pParent)
{

}

CConfigureReportViewDlg::~CConfigureReportViewDlg()
{
}

void CConfigureReportViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureReportViewDlg)		
	DDX_Control(pDX, IDC_FONT_TYPE_PROPORTIONAL_RADIO, m_radioProportionalFont);
	DDX_Control(pDX, IDC_FONT_TYPE_MONOSPACED_RADIO, m_radioMonospacedFont);
	DDX_Control(pDX, IDC_TRIM_EXTRA_SPACES_CHECK, m_checkTrimExtraSpaces);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureReportViewDlg, CNxDialog)
	ON_BN_CLICKED(IDC_FONT_TYPE_HELP_BTN, OnBtnFontTypeHelp)
END_MESSAGE_MAP()

using namespace NXDATALIST2Lib;

enum FieldListColumns {
	flcFieldEnum = 0,
	flcFieldName = 1,
	flcShow = 2, //TES 3/26/2012 - PLID 49208
	flcRow = 3,
	flcColumn = 4,
};

// CConfigureReportViewDlg message handlers
BOOL CConfigureReportViewDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (r.gonet 03/07/2013) - PLID 44465 - Added a bulk cache
		// (r.gonet 03/07/2013) - PLID 43599 - Added the trim white space preference.
		g_propManager.BulkCache("ConfigureReportViewDlg", propbitNumber,
			"(Username = '<None>') AND ("
			"Name = 'LabReportViewFontType' OR "
			"Name = 'LabReportViewTrimExtraSpaces' "
			")");
		
		//TES 2/24/2012 - PLID 44841 - NxIconify
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		m_pFieldList = BindNxDataList2Ctrl(IDC_REPORT_VIEW_FIELDS, false);

		
		//TES 2/24/2012 - PLID 44841 - Now we need to go through each of the lab result fields, and add it to our list.
		// Default values all approximate the positions they were in previous to this change, although because the whole HTML
		// format has changed it can't be 100% precise.
		IRowSettingsPtr pRow = m_pFieldList->GetNewRow();
		// (d.singleton 2013-07-15 15:31) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
		// (d.singleton 2014-02-04 10:05) - PLID 60225 - grey out the row and column cells for the specimen fields,  make them read only.
		IFormatSettingsPtr pFormatSettings(__uuidof(NXDATALIST2Lib::FormatSettings));
		// (d.singleton 2013-07-15 15:31) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfSpecimenIdText);
		pRow->PutValue(flcFieldName, _bstr_t("Specimen ID Text"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenIdText, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, g_cvarNull);
		// (d.singleton 2014-02-04 10:05) - PLID 60225 - grey out the row and column cells for the specimen fields,  make them read only.
		pRow->PutCellBackColor(flcRow, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcRow, pFormatSettings);
		pRow->PutValue(flcColumn, g_cvarNull);			
		pRow->PutCellBackColor(flcColumn, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcColumn, pFormatSettings);
		m_pFieldList->AddRowAtEnd(pRow, NULL);

		// (d.singleton 2013-07-15 15:31) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfSpecimenStartTime);
		pRow->PutValue(flcFieldName, _bstr_t("Specimen Collection Start Time"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenStartTime, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, g_cvarNull);
		// (d.singleton 2014-02-04 10:05) - PLID 60225 - grey out the row and column cells for the specimen fields,  make them read only.
		pRow->PutCellBackColor(flcRow, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcRow, pFormatSettings);
		pRow->PutValue(flcColumn, g_cvarNull);			
		pRow->PutCellBackColor(flcColumn, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcColumn, pFormatSettings);
		m_pFieldList->AddRowAtEnd(pRow, NULL);

		// (d.singleton 2013-07-15 15:31) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfSpecimenEndTime);
		pRow->PutValue(flcFieldName, _bstr_t("Specimen Collection End Time"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenEndTime, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, g_cvarNull);
		// (d.singleton 2014-02-04 10:05) - PLID 60225 - grey out the row and column cells for the specimen fields,  make them read only.
		pRow->PutCellBackColor(flcRow, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcRow, pFormatSettings);
		pRow->PutValue(flcColumn, g_cvarNull);			
		pRow->PutCellBackColor(flcColumn, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcColumn, pFormatSettings);
		m_pFieldList->AddRowAtEnd(pRow, NULL);

		// (d.singleton 2013-07-15 15:31) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfSpecimenRejectReason);
		pRow->PutValue(flcFieldName, _bstr_t("Specimen Rejection Reason"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenRejectReason, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, g_cvarNull);
		// (d.singleton 2014-02-04 10:05) - PLID 60225 - grey out the row and column cells for the specimen fields,  make them read only.
		pRow->PutCellBackColor(flcRow, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcRow, pFormatSettings);
		pRow->PutValue(flcColumn, g_cvarNull);			
		pRow->PutCellBackColor(flcColumn, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcColumn, pFormatSettings);
		m_pFieldList->AddRowAtEnd(pRow, NULL);

		// (d.singleton 2013-07-15 15:31) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfSpecimenCondition);
		pRow->PutValue(flcFieldName, _bstr_t("Specimen Condition"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenCondition, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, g_cvarNull);
		// (d.singleton 2014-02-04 10:05) - PLID 60225 - grey out the row and column cells for the specimen fields,  make them read only.
		pRow->PutCellBackColor(flcRow, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcRow, pFormatSettings);
		pRow->PutValue(flcColumn, g_cvarNull);			
		pRow->PutCellBackColor(flcColumn, RGB(200,200,200));
		pRow->PutRefCellFormatOverride(flcColumn, pFormatSettings);
		m_pFieldList->AddRowAtEnd(pRow, NULL);		

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfName);
		pRow->PutValue(flcFieldName, _bstr_t("Name"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfName, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 1, lrfName, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfName, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfSlideNum);
		pRow->PutValue(flcFieldName, _bstr_t("Slide #"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSlideNum, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 1, lrfSlideNum, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfSlideNum, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfDateReceived);
		pRow->PutValue(flcFieldName, _bstr_t("Date Received"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfDateReceived, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 2, lrfDateReceived, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDateReceived, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfLOINC);
		pRow->PutValue(flcFieldName, _bstr_t("LOINC"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfLOINC, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 2, lrfLOINC, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfLOINC, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfDateReceivedByLab);
		pRow->PutValue(flcFieldName, _bstr_t("Date Received By Lab"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfDateReceivedByLab, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 3, lrfDateReceivedByLab, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDateReceivedByLab, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfDatePerformed);
		pRow->PutValue(flcFieldName, _bstr_t("Date Performed"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfDatePerformed, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 4, lrfDatePerformed, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDatePerformed, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfStatus);
		pRow->PutValue(flcFieldName, _bstr_t("Status"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfStatus, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 5, lrfStatus, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfStatus, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfUnits);
		pRow->PutValue(flcFieldName, _bstr_t("Units"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfUnits, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 5, lrfUnits, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfUnits, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfFlag);
		pRow->PutValue(flcFieldName, _bstr_t("Flag"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfFlag, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 6, lrfFlag, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfFlag, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfReference);
		pRow->PutValue(flcFieldName, _bstr_t("Reference"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfReference, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 6, lrfReference, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfReference, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfAcknowledgedBy);
		pRow->PutValue(flcFieldName, _bstr_t("Acknowledged"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfAcknowledgedBy, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 7, lrfAcknowledgedBy, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfAcknowledgedBy, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfDiagnosis);
		pRow->PutValue(flcFieldName, _bstr_t("Diagnosis"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfDiagnosis, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 8, lrfDiagnosis, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDiagnosis, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfMicroscopicDescription);
		pRow->PutValue(flcFieldName, _bstr_t("Microscopic Description"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfMicroscopicDescription, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 8, lrfMicroscopicDescription, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfMicroscopicDescription, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-07-16 17:24) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfServiceStartTime);
		pRow->PutValue(flcFieldName, _bstr_t("Service Start Time"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfServiceStartTime, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 12, lrfServiceStartTime, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfServiceStartTime, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfServiceEndTime);
		pRow->PutValue(flcFieldName, _bstr_t("Service End Time"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfServiceEndTime, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 12, lrfServiceEndTime, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfServiceEndTime, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);
	
		// (d.singleton 2013-08-07 16:02) - PLID 57912 - need to show the Performing Provider on report view of labresult tab dlg
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerformingProvider);
		pRow->PutValue(flcFieldName, _bstr_t("Performing Provider"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerformingProvider, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 13, lrfPerformingProvider, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerformingProvider, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-11-04 12:14) - PLID 59294 - add observation date to the html view for lab results. 
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfObservationDate);
		pRow->PutValue(flcFieldName, _bstr_t("Observation Date"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfObservationDate, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 13, lrfObservationDate, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfObservationDate, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerformingLab);
		pRow->PutValue(flcFieldName, _bstr_t("Performing Lab"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerformingLab, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 14, lrfPerformingLab, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerformingLab, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerfLabAddress);
		pRow->PutValue(flcFieldName, _bstr_t("Perf Lab Address"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerfLabAddress, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 15, lrfPerfLabAddress, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabAddress, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerfLabCity);
		pRow->PutValue(flcFieldName, _bstr_t("Perf Lab City"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerfLabCity, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 16, lrfPerfLabCity, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabCity, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerfLabState);
		pRow->PutValue(flcFieldName, _bstr_t("Perf Lab State"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerfLabState, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 17, lrfPerfLabState, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabState, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerfLabZip);
		pRow->PutValue(flcFieldName, _bstr_t("Perf Lab Zip"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerfLabZip, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 18, lrfPerfLabZip, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabZip, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);		

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerfLabCountry);
		pRow->PutValue(flcFieldName, _bstr_t("Perf Lab Country"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerfLabCountry, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 19, lrfPerfLabCountry, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabCountry, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfPerfLabParish);
		pRow->PutValue(flcFieldName, _bstr_t("Perf Lab Parish"));
		pRow->PutValue(flcShow, GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfPerfLabParish, "<None>")?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(flcRow, GetRemotePropertyInt("LabReportViewFieldRow", 20, lrfPerfLabParish, "<None>"));
		pRow->PutValue(flcColumn, GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabParish, "<None>"));
		m_pFieldList->AddRowAtEnd(pRow, NULL);
		ColorRow(pRow);

		//TES 2/24/2012 - PLID 44841 - Comments are hardcoded to the first row after the highest configured row.
		//Find the highest row in the list
		long nHighestRow = 0;
		pRow = m_pFieldList->GetFirstRow();
		while(pRow) {
			long nRow = VarLong(pRow->GetValue(flcRow), -1);
			if(nRow > nHighestRow) nHighestRow = nRow;
			pRow = pRow->GetNextRow();
		}
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfComments);
		pRow->PutValue(flcFieldName, _bstr_t("Comments"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, g_cvarTrue);
		pRow->PutValue(flcRow, nHighestRow+1);
		pRow->PutValue(flcColumn, 1);
		pRow->PutBackColor(RGB(200,200,200));
		m_pFieldList->AddRowAtEnd(pRow, NULL);

		//TES 2/24/2012 - PLID 44841 - Value is hardcoded to the second row after the highest configured row
		pRow = m_pFieldList->GetNewRow();
		pRow->PutValue(flcFieldEnum, lrfValue);
		pRow->PutValue(flcFieldName, _bstr_t("Value"));
		//TES 3/26/2012 - PLID 49208 - Added Show column
		pRow->PutValue(flcShow, g_cvarTrue);
		pRow->PutValue(flcRow, nHighestRow+2);
		pRow->PutValue(flcColumn, 1);
		pRow->PutBackColor(RGB(200,200,200));
		m_pFieldList->AddRowAtEnd(pRow, NULL);

		// (r.gonet 03/07/2013) - PLID 44465 - Load the font type radio buttons. Default to the proportional font type.
		long nFontType = GetRemotePropertyInt("LabReportViewFontType", (long)lrvftProportional);
		if(nFontType == (long)lrvftMonospaced) {
			m_radioMonospacedFont.SetCheck(BST_CHECKED);
		} else {
			m_radioProportionalFont.SetCheck(BST_CHECKED);
		}

		// (r.gonet 03/07/2013) - PLID 43599 - Load the checkbox to trim consecutive white space.
		BOOL bTrimExtraSpaces = GetRemotePropertyInt("LabReportViewTrimExtraSpaces", TRUE) ? TRUE : FALSE;
		m_checkTrimExtraSpaces.SetCheck(bTrimExtraSpaces ? BST_CHECKED : BST_UNCHECKED);
		

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}BEGIN_EVENTSINK_MAP(CConfigureReportViewDlg, CNxDialog)
ON_EVENT(CConfigureReportViewDlg, IDC_REPORT_VIEW_FIELDS, 8, CConfigureReportViewDlg::OnEditingStartingReportViewFields, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CConfigureReportViewDlg, IDC_REPORT_VIEW_FIELDS, 9, CConfigureReportViewDlg::OnEditingFinishingReportViewFields, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
ON_EVENT(CConfigureReportViewDlg, IDC_REPORT_VIEW_FIELDS, 10, CConfigureReportViewDlg::OnEditingFinishedReportViewFields, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CConfigureReportViewDlg::OnEditingStartingReportViewFields(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//TES 2/24/2012 - PLID 44841 - Don't let them edit the position of the Comments or Value field, those are hardcoded
		LabResultField lrf = (LabResultField)VarLong(pRow->GetValue(flcFieldEnum));
		if(lrf == lrfComments || lrf == lrfValue) {
			*pbContinue = FALSE;
		}
		//TES 3/26/2012 - PLID 49208 - Don't let them edit the row or column of a field that's not being shown
		else if(!VarBool(pRow->GetValue(flcShow)) && nCol != flcShow) {
			*pbContinue = FALSE;
		}
	} NxCatchAll(__FUNCTION__);
}

void CConfigureReportViewDlg::OnEditingFinishingReportViewFields(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//TES 2/24/2012 - PLID 44841 - Limit the number of rows and columns to, say, 15.  It doesn't actually matter much, because
		// we'll collapse any empty rows/columns, but might as well make it reasonable.
		// (d.singleton 2014-01-17 15:15) - PLID 60225 - this needs to be more than 15 since we added a bunch of values. and since it was
		// arbitrary to begin with lets now go with 30 to give a bit of a cushion for any other new fields.
		if((*pbCommit == TRUE) && (nCol == flcRow || nCol == flcColumn)) {
			long nNewValue = AsLong(pvarNewValue);
			if(nNewValue <= 0 || nNewValue > 30) {
				MsgBox("Please enter a number between 1 and 30 for this value.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CConfigureReportViewDlg::OnEditingFinishedReportViewFields(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if(bCommit && nCol == flcRow) {
			//TES 2/24/2012 - PLID 44841 - Update the hardcoded positions of the Comments and Value fields
			//Find the highest row in the list
			// (d.singleton 2014-02-04 10:05) - PLID 60225 - use -1 for default value when null
			long nHighestRow = 0;
			IRowSettingsPtr pRow = m_pFieldList->GetFirstRow();
			while(pRow) {
				long nRow = VarLong(pRow->GetValue(flcRow), -1);
				LabResultField lrf = (LabResultField)VarLong(pRow->GetValue(flcFieldEnum));
				if(lrf != lrfComments && lrf != lrfValue) {
					if(nRow > nHighestRow) nHighestRow = nRow;
				}
				pRow = pRow->GetNextRow();
			}

			pRow = m_pFieldList->FindByColumn(flcFieldEnum, (long)lrfComments, NULL, g_cvarFalse);
			if(pRow) {
				pRow->PutValue(flcRow, nHighestRow+1);
			}
			pRow = m_pFieldList->FindByColumn(flcFieldEnum, (long)lrfValue, NULL, g_cvarFalse);
			if(pRow) {
				pRow->PutValue(flcRow, nHighestRow+2);
			}
		}
		//TES 3/26/2012 - PLID 49208 - If they edit the show column, re-color the row to "enable" or "disable" it
		else if(bCommit && nCol == flcShow) {
			IRowSettingsPtr pRow(lpRow);
			if(pRow) {
				// (d.singleton 2014-02-04 10:05) - PLID 60225 - we do not want to recolor the row if its specimen data
				if(VarLong(pRow->GetValue(flcRow), -1) != -1) {
					ColorRow(pRow);
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/07/2013) - PLID 44465 - Some help for what the font type radio buttons are about.
void CConfigureReportViewDlg::OnBtnFontTypeHelp()
{
	try {
		MessageBox("In a proportional font, the widths of letters, numbers, and other characters vary creating a more pleasing look for reading paragraphs or general text. "
			"Unless your lab company sends textual tables in results, you will want to have the Proportional setting enabled.\r\n"
			"\r\n"
			"In a monospaced font, all letters, numbers, and other characters have the same width. This is useful for when text contains tabular data that must stay in alignment. "
			"NexTech has found that some lab companies will send results that contain textual tables that need to be aligned in order to be viewed properly. The Monospaced setting "
			"will allow you to view these results correctly.",
			"Font Type Help", MB_OK|MB_ICONINFORMATION);
	} NxCatchAll(__FUNCTION__);
}

void CConfigureReportViewDlg::OnOK()
{
	try {
		//TES 2/24/2012 - PLID 44841 - Make sure that all rows and columns are between 1 and 15, and that no two fields are configured
		// for the same position.
		// (d.singleton 2014-02-04 09:03) - PLID 60225 - need an option to hide spm data on the report view of a lab result.
		//	spm data does not change position,  only show or hide.
		IRowSettingsPtr pRow = m_pFieldList->GetFirstRow();
		while(pRow) {
			long nRow = VarLong(pRow->GetValue(flcRow), -1);
			long nColumn = VarLong(pRow->GetValue(flcColumn), -1);
			LabResultField lrf = (LabResultField)VarLong(pRow->GetValue(flcFieldEnum));
			//TES 3/26/2012 - PLID 49208 - Don't validate the position on hidden fields.
			// (d.singleton 2014-02-04 09:36) - PLID 60225 - or specimen fields, also upped the col / row check to 30
			BOOL bShow = VarBool(pRow->GetValue(flcShow));
			if((bShow && lrf != lrfComments && lrf != lrfValue && lrf != lrfSpecimenIdentifier && lrf != lrfSpecimenIdText && lrf != lrfSpecimenStartTime &&
				lrf != lrfSpecimenEndTime && lrf != lrfSpecimenRejectReason && lrf != lrfSpecimenCondition ) &&
				(nRow < 0 || nRow > 30 || nColumn < 0 || nColumn > 30)) {
				CString strField = VarString(pRow->GetValue(flcFieldName));
				MsgBox("The field %s has an invalid value entered.  "
					"Please ensure that the row and column are set to a value between 1 and 20.",
					strField);
				return;
			}
			// (d.singleton 2014-02-04 09:36) - PLID 60225 - so if we get -1 for our row/column we know its the specimen data since
			// code does not allow null values in these rows/columns so we do not need the below validation
			if(bShow && nRow != -1 && nColumn != -1) {
				IRowSettingsPtr pOtherRow = pRow->GetNextRow();
				while(pOtherRow) {
					// (d.singleton 2014-02-04 09:36) - PLID 60225 - need default values to avoid exceptions
					long nOtherRow = VarLong(pOtherRow->GetValue(flcRow), -1);
					long nOtherColumn = VarLong(pOtherRow->GetValue(flcColumn), -1);
					if(nRow == nOtherRow && nColumn == nOtherColumn) {
						if(VarBool(pOtherRow->GetValue(flcShow))) {
							CString strField = VarString(pRow->GetValue(flcFieldName));
							CString strOtherField = VarString(pOtherRow->GetValue(flcFieldName));
							MsgBox("The fields %s and %s are both assigned to the same location (Row %li, Column %li).  "
								"Please ensure that each field is assigned to a unique location.",
								strField, strOtherField, nRow, nColumn);
							return;
						}
					}
					pOtherRow = pOtherRow->GetNextRow();
				}
			}
			pRow = pRow->GetNextRow();
		}

		//TES 2/24/2012 - PLID 44841 - Now store all the values back to ConfigRT
		pRow = m_pFieldList->GetFirstRow();
		while(pRow) {
			LabResultField lrf = (LabResultField)VarLong(pRow->GetValue(flcFieldEnum));
			if(lrf != lrfComments && lrf != lrfValue) {
				// (d.singleton 2014-02-04 09:36) - PLID 60225 - we do not update row or column for specimen fields
				if(lrf != lrfSpecimenIdentifier && lrf != lrfSpecimenIdText && lrf != lrfSpecimenStartTime &&
					lrf != lrfSpecimenEndTime && lrf != lrfSpecimenRejectReason && lrf != lrfSpecimenCondition) {
					SetRemotePropertyInt("LabReportViewFieldRow", VarLong(pRow->GetValue(flcRow)), lrf, "<None>");
					SetRemotePropertyInt("LabReportViewFieldColumn", VarLong(pRow->GetValue(flcColumn)), lrf, "<None>");
				}
				//TES 3/26/2012 - PLID 49208 - Added the Show column
				SetRemotePropertyInt("LabReportViewFieldShow", VarBool(pRow->GetValue(flcShow)), lrf, "<None>");
			}
			pRow = pRow->GetNextRow();
		}

		// (r.gonet 03/07/2013) - PLID 44465 - Save the font type
		ELabReportViewFontType lrvftFontType;
		if(m_radioMonospacedFont.GetCheck()) {
			lrvftFontType = lrvftMonospaced;
		} else {
			lrvftFontType = lrvftProportional;
		}
		SetRemotePropertyInt("LabReportViewFontType", (long)lrvftFontType);

		// (r.gonet 03/07/2013) - PLID 43599 - Save the setting to trim extra spaces.
		BOOL bTrimExtraSpaces = m_checkTrimExtraSpaces.GetCheck() != BST_UNCHECKED ? TRUE : FALSE;
		SetRemotePropertyInt("LabReportViewTrimExtraSpaces", (long)bTrimExtraSpaces);

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

//TES 3/26/2012 - PLID 49208 - Color the row as "greyed out" if it's not being shown, normal otherwise
void CConfigureReportViewDlg::ColorRow(IRowSettingsPtr pRow)
{
	if(VarBool(pRow->GetValue(flcShow))) {
		pRow->PutCellBackColor(flcRow, RGB(255,255,255));
		pRow->PutCellBackColor(flcColumn, RGB(255,255,255));
	}
	else {
		pRow->PutCellBackColor(flcRow, RGB(200,200,200));
		pRow->PutCellBackColor(flcColumn, RGB(200,200,200));
	}
}