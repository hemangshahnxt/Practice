// PatientWellnessDlg.cpp : implementation file
//

// (j.gruber 2009-05-26 15:27) - PLID 34348 - created for
#include "stdafx.h"
#include "Practice.h"
#include "PatientWellnessDlg.h"
#include "PatientWellnessAlertDlg.h"
#include "WellnessUtils.h"
#include "SelectDlg.h"
#include "WellnessDataUtils.h"

// (j.gruber 2009-06-03 11:57) - PLID 34457 - changed IsPrequalified template to Status
enum AlertListColumns {

	alcID = 0,
	alcDate,
	alcStatus,
	alcName,
	alcNotes,
	alcView,
};

#define LABEL_ROW_BACK_COLOR  RGB(0,0,0)
#define LABEL_ROW_FORE_COLOR  RGB(255,255,255)

// CPatientWellnessDlg dialog

IMPLEMENT_DYNAMIC(CPatientWellnessDlg, CNxDialog)

CPatientWellnessDlg::CPatientWellnessDlg(long nPatientID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientWellnessDlg::IDD, pParent)
{
	m_nPatientID = nPatientID;
}

CPatientWellnessDlg::~CPatientWellnessDlg()
{
}

void CPatientWellnessDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PAT_WELLNESS_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_PAT_WELLNESS_BKG, m_bkg);
}


BEGIN_MESSAGE_MAP(CPatientWellnessDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PAT_WELLNESS_CLOSE, &CPatientWellnessDlg::OnBnClickedPatWellnessClose)
END_MESSAGE_MAP()


// CPatientWellnessDlg message handlers
BOOL CPatientWellnessDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnClose.AutoSet(NXB_CLOSE);

		m_pAlertList = BindNxDataList2Ctrl(IDC_PAT_WELLNESS_LIST, false);

		LoadAlertList();

		
		//set the color
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));		

		CString strTitle;
		strTitle.Format("Wellness Alerts for %s", GetExistingPatientName(m_nPatientID));
		SetWindowText(strTitle);
	}NxCatchAll("Error in CPatientWellnessDlg::OnInitDialog() ");

	return TRUE;
}

void CPatientWellnessDlg::LoadAlertList() 
{

	try {

		CWaitCursor cwait;

		//first we need to add our label records
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pAlertList->GetNewRow();
		pRow->PutValue(alcID, (long)-1);
		pRow->PutValue(alcName, _variant_t("Existing Alerts"));
		pRow->PutValue(alcStatus, (long)0);
		pRow->PutBackColor(LABEL_ROW_BACK_COLOR);
		pRow->PutForeColor(LABEL_ROW_FORE_COLOR);
		m_pAlertList->AddRowSorted(pRow, NULL);

		pRow = m_pAlertList->GetNewRow();
		pRow->PutValue(alcID, (long)-2);
		pRow->PutValue(alcName, _variant_t("Upcoming Alerts"));
		pRow->PutValue(alcStatus, (long)1);
		// (j.gruber 2009-06-03 16:00) - PLID 34456 - let's make this easier to find
		pRow->PutValue(alcView, _variant_t("Create"));
		pRow->PutBackColor(LABEL_ROW_BACK_COLOR);
		pRow->PutForeColor(LABEL_ROW_FORE_COLOR);
		m_pAlertList->AddRowSorted(pRow, NULL);

		pRow = m_pAlertList->GetNewRow();
		pRow->PutValue(alcID, (long)-3);
		pRow->PutValue(alcName, _variant_t("Completed/Skipped Alerts"));
		pRow->PutValue(alcStatus, (long)2);
		pRow->PutBackColor(LABEL_ROW_BACK_COLOR);
		pRow->PutForeColor(LABEL_ROW_FORE_COLOR);
		m_pAlertList->AddRowSorted(pRow, NULL);

		//now add our records
		// (j.gruber 2009-06-03 12:56) - PLID 34457 - also added a union for completed items
		/****NOTE: A VERY SIMILIAR QUERY IS USED IN PatientNexEmrDlg.cpp::RefreshWellness
		THEY ONLY DIFFER IN WHAT THEY RETURN
		SO IF THIS CHANGES, MAKE SURE TO CHECK THAT ONE ALSO******/
		//TES 7/8/2009 - PLID 34534 - Moved the query in question to a #define in WellnessDataUtils.h, so it only
		// needs to be maintained in one place now.
		ADODB::_RecordsetPtr rsAlerts = CreateParamRecordset(GetRemoteData(), "SET NOCOUNT ON;"
			"	 DECLARE @nPatientID INT  "
			" 	 SET @nPatientID = {INT}; "
			/*" 	 SET @nPatientID = 12496; "*/
			"  "
			+ WELLNESS_INSTANTIATION_SQL + 
			""
			" 	/*now select our records that we need to show*/ "
			" 	SET NOCOUNT OFF; "
			" 	 "
			" 	SELECT ID, Name, FirstPresentedDate, Note, 0 as Status FROM PatientWellnessT WHERE DELETED = 0 AND PatientID = @nPatientID AND CompletedDate IS NULL "
			" 	UNION "
			" 	SELECT ID, Name, FirstPresentedDate, Note, 1 as Status FROM @RecordsToShow "
			"   UNION "
			" 	SELECT ID, Name, FirstPresentedDate, Note, 2 as Status FROM PatientWellnessT WHERE DELETED = 0 AND PatientID = @nPatientID AND CompletedDate IS NOT NULL; "
			, m_nPatientID, GetCurrentUserID()); 
			
 		CString strName, strNotes; 
 		long nStatus; 
 		COleDateTime dtDate;	 
 		long nID; 

		COleDateTime dtNull;
		dtNull.SetDate(1800,12,31);

		ADODB::FieldsPtr flds = rsAlerts->Fields;

		while (!rsAlerts->eof) {

			//get everything out of the query that we'll need
			nID = AdoFldLong(flds, "ID");
			strName = AdoFldString(flds, "Name", "");
			_variant_t varDate;
			varDate.vt = VT_DATE;
			varDate.date = AdoFldDateTime(flds, "FirstPresentedDate", dtNull);			
			nStatus = AdoFldLong(flds, "Status");
			strNotes = AdoFldString(flds, "Note", "");

			//now add to the list
			pRow = m_pAlertList->GetNewRow();
			pRow->PutValue(alcID, nID);
			pRow->PutValue(alcName, _variant_t(strName));
			pRow->PutValue(alcDate, varDate);
			pRow->PutValue(alcStatus, nStatus);
			if (nStatus == 0) {				
				pRow->PutBackColor(PATIENT_ALERT);
			}
			else if (nStatus == 1) {				
				pRow->PutBackColor(PREQUALIFIED_ALERT);
			}
			else if (nStatus == 2) {
				pRow->PutBackColor(COMPLETED_ALERT);
			}
			pRow->PutValue(alcNotes, _variant_t(strNotes));
			pRow->PutValue(alcView, _variant_t("View"));
			
			m_pAlertList->AddRowSorted(pRow, NULL);		

			rsAlerts->MoveNext();
		}		

	}NxCatchAll("Error in CPatientWellnessDlg::LoadAlertList() ");
}BEGIN_EVENTSINK_MAP(CPatientWellnessDlg, CNxDialog)
ON_EVENT(CPatientWellnessDlg, IDC_PAT_WELLNESS_LIST, 19, CPatientWellnessDlg::LeftClickPatWellnessList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CPatientWellnessDlg, IDC_PAT_WELLNESS_LIST, 6, CPatientWellnessDlg::RButtonDownPatWellnessList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CPatientWellnessDlg::LeftClickPatWellnessList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		switch (nCol) {

			case alcView:

				//get the row
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

				if (pRow) {

					long nID = pRow->GetValue(alcID);
					if (nID == -2) {
						//they want to create one
						CreateAlert();
					}else if (nID > 0) {
						long nStatus = pRow->GetValue(alcStatus);
						BOOL bLoadFromTemplate;
						if (nStatus == 1) {
							bLoadFromTemplate = TRUE;
						}
						else {
							bLoadFromTemplate = FALSE;
						}

						// (j.gruber 2009-06-03 11:57) - PLID 34457 - make completed items read only
						BOOL bOpenReadOnly = FALSE;
						if (nStatus == 2) {
							bOpenReadOnly = TRUE;
						}

						CPatientWellnessAlertDlg dlg(nID, bLoadFromTemplate, m_nPatientID, bOpenReadOnly, this);
						long nResult = dlg.DoModal();

						// (j.gruber 2009-06-01 09:48) - PLID 34401 - if they ran a report, close this screen
						if (nResult == ID_PREVIEW_REPORT) {
							//close the list
							OnBnClickedPatWellnessClose();
						}
						else if (nResult == IDOK) {
							//reload the list
							m_pAlertList->Clear();
							LoadAlertList();
						}
					}
				}				
			break;
		}

	}NxCatchAll("Error in CPatientWellnessDlg::LeftClickPatWellnessList");

}

void CPatientWellnessDlg::OnBnClickedPatWellnessClose()
{
	OnOK();
}


// (j.gruber 2009-06-03 16:00) - PLID 34456 - moved creation to new function
void CPatientWellnessDlg::CreateAlert() {

	try {

		//pop up the selection dialog with our list of templates in it
		CSelectDlg dlg(this);
		dlg.m_strTitle = "Select a Wellness Template";
		dlg.m_strCaption = "Select a Wellness Template to create for this patient.";
		dlg.m_strFromClause = "(SELECT ID, Name From WellnessTemplateT WHERE SpecificToPatientID IS NULL AND Deleted = 0) Q";					
		
		dlg.AddColumn("ID", "ID", FALSE, FALSE);
		dlg.AddColumn("Name", "Name", TRUE, FALSE, FALSE);						
		
		if (IDOK == dlg.DoModal()) {
			long nWellnessTemplateID;
			nWellnessTemplateID = VarLong(dlg.m_arSelectedValues[0], -1);

			if (nWellnessTemplateID != -1) {

				//first make sure this template doesn't already exist for this patient
				ADODB::_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM WellnessTemplateT WHERE OriginalWellnessTemplateID = {INT} AND SpecificToPatientID = {INT}", nWellnessTemplateID, m_nPatientID);
				if (! rsCheck->eof) {
					//they already have one, let them edit it
					if (IDYES == MsgBox(MB_YESNO, "There is already a patient specific template that exists for the patient/template combination, you cannot create a new one.\nWould you like to edit the existing template instead?")) {
						long nTemplateID = AdoFldLong(rsCheck, "ID");
						CPatientWellnessAlertDlg dlg(nTemplateID, TRUE, m_nPatientID, FALSE, this);
						long nResult = dlg.DoModal();

						// (j.gruber 2009-06-01 09:48) - PLID 34401 - if they ran a report, close this screen
						if (nResult == ID_PREVIEW_REPORT) {
							//close the list
							OnBnClickedPatWellnessClose();
							return;
						}
						else if (nResult == IDOK) {
							//reload the list
							m_pAlertList->Clear();
							LoadAlertList();
							return;
						}
						else {
							return;
						}
					}
					else {
						return;
					}
				}
				
				//pop up the dialog
				CPatientWellnessAlertDlg dlg(nWellnessTemplateID, TRUE, m_nPatientID, FALSE, this);
				long nResult = dlg.DoModal();

				// (j.gruber 2009-06-01 09:48) - PLID 34401 - if they ran a report, close this screen
				if (nResult == ID_PREVIEW_REPORT) {
					//close the list
					OnBnClickedPatWellnessClose();
				}
				else if (nResult == IDOK)  {
					//reload the list
					m_pAlertList->Clear();
					LoadAlertList();
				}
			}
		}	

	}NxCatchAll("Error in CPatientWellnessDlg::CreateAlert()");



}

// (j.gruber 2009-06-03 09:43) - PLID 34456 - ability to create a patient alert on the fly
void CPatientWellnessDlg::RButtonDownPatWellnessList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			//make sure this is our patient alert header row
			long nID = VarLong(pRow->GetValue(alcID));

			if (nID == -2) {

				//pop up a menu
				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.AppendMenu(MF_ENABLED, 1, "Create template for this patient");
				CPoint ptClicked(x, y);
				GetDlgItem(IDC_PAT_WELLNESS_LIST)->ClientToScreen(&ptClicked);
				int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
				if(nResult == 1) {
					CreateAlert();
					
				}
			}

		}
	}NxCatchAll("Error in CPatientWellnessDlg::RButtonDownPatWellnessList");
}
