// ImportWizardPreviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NxStandard.h"
#include "ImportWizardPreviewDlg.h"
#include "ImportWizardDlg.h"
#include "GlobalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "WellnessDataUtils.h"
#include "PracticeRc.h"
#include "ImportBatchUtils.h"
// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
#include "GlobalDrawingUtils.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CImportWizardPreviewDlg property page

// (b.savon 2015-04-28 15:19) - PLID 65485 - Derive from our CNxPropertyPage
IMPLEMENT_DYNCREATE(CImportWizardPreviewDlg, CNxPropertyPage)

CImportWizardPreviewDlg::CImportWizardPreviewDlg() : CNxPropertyPage(CImportWizardPreviewDlg::IDD)
{
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	m_bNeedInit = TRUE;
	// (b.cardillo 2015-07-15 22:36) - PLID 66545 - We now remove most rows from the source, 
	// but we'll leave a requested number intact.
	m_nLeaveCountInFromList = 20;
	//{{AFX_DATA_INIT(CImportWizardPreviewDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CImportWizardPreviewDlg::~CImportWizardPreviewDlg()
{
}

void CImportWizardPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_IMPORT_PERFORM_BACKUP, m_chkPerformBackup);
	DDX_Control(pDX, IDC_CHK_APPLY_CAPSFIX, m_chkPerformCapsFix);
	//{{AFX_DATA_MAP(CImportWizardPreviewDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportWizardPreviewDlg, CNxPropertyPage)
	//{{AFX_MSG_MAP(CImportWizardPreviewDlg)
	ON_BN_CLICKED(IDC_IMPORT_SHOW_IGNORED, OnImportShowIgnored)
	//}}AFX_MSG_MAP
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportWizardPreviewDlg message handlers


BOOL CImportWizardPreviewDlg::OnInitDialog() 
{
	try{
		CNxPropertyPage::OnInitDialog();
	
		m_pPreviewList = BindNxDataList2Ctrl(this, IDC_IMPORT_PREVIEW_TABLE, NULL, false);
		m_bHasPatientID = FALSE;
		m_bHasSubCode = FALSE;

		// (b.savon 2015-03-19 10:48) - PLID 65248 - Default to perform the backup
		m_chkPerformBackup.SetCheck(BST_CHECKED);

		// (b.savon 2015-03-24 12:28) - PLID 65250 - Default to UNCHECKED
		m_chkPerformCapsFix.SetCheck(BST_UNCHECKED);

		// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
		//Get our current size
		{
			CRect rc;
			GetClientRect(&rc);
			//Remember our current size
			m_ClientSize = rc.Size();
		}

		GetDlgItem(IDC_STATIC)->GetWindowText(m_strOriginalInstructionText);
		
		//Remember that we no longer need to be initialized
		m_bNeedInit = FALSE;

		return TRUE;
	}NxCatchAll("Error in CImportWizardPreviewDlg::OnInitDialog");
	return FALSE;  
}


BOOL CImportWizardPreviewDlg::OnSetActive()
{
	((CImportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

	//(e.lally 2007-06-26) PLID 5288 - We need to recopy our preview on activation
	m_pPreviewList->SetRedraw(FALSE);
	CreatePreviewFromCopy();
	m_pPreviewList->SetRedraw(TRUE);

	// (r.goldschmidt 2016-01-27 16:00) - PLID 67976 - set up appointment related options and show them, but don't let them get changed
	ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;
	if (irtCurrent == irtAppointments) {
		GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION_SAVED)->ShowWindow(TRUE);
		GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION_SAVED)->EnableWindow(FALSE);
		CheckDlgButton(IDC_APPT_TYPES_AS_CONVERSION_SAVED, m_bApptConversionChecked);
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES_SAVED)->ShowWindow(TRUE);
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES_SAVED)->EnableWindow(FALSE);
		CheckDlgButton(IDC_APPT_TYPE_TO_NOTES_SAVED, m_bApptNotesChecked);
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES_SAVED)->SetWindowText(m_strApptNotesCheckboxText);
		// (r.goldschmidt 2016-02-02 17:13) - PLID 67976 - let the user know which columns won't convert
		// (r.goldschmidt 2016-03-16 12:54) - PLID 67974 - these two options are now independent of each other
		if (IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION_SAVED) || IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED)) {
			GetDlgItem(IDC_STATIC)->SetWindowText(m_strOriginalInstructionText + " " + m_strAdditionalExplanatoryText);
		}
	}
	else {
		GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION_SAVED)->ShowWindow(FALSE);
		GetDlgItem(IDC_APPT_TYPES_AS_CONVERSION_SAVED)->EnableWindow(FALSE);
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES_SAVED)->ShowWindow(FALSE);
		GetDlgItem(IDC_APPT_TYPE_TO_NOTES_SAVED)->EnableWindow(FALSE);
	}

	// (j.politis 2015-04-30 10:49) - PLID 65524 - Allow the import wizard to be resizable
	//When we become active, tell our parent
	CImportWizardDlg *pSheet = dynamic_cast<CImportWizardDlg *>(GetParentSheet());
	if (pSheet != NULL) {
		pSheet->OnPageSetActive(this);
	}

	return CNxPropertyPage::OnSetActive();
}

BOOL CImportWizardPreviewDlg::OnKillActive()
{
	try {
		return CNxPropertyPage::OnKillActive();
	}NxCatchAll("Error in CImportWizardPreviewDlg::OnKillActive");
	return FALSE;
}

// (b.cardillo 2015-07-15 22:36) - PLID 66545 - The preview page now removes most rows from the 
// source. 
// Tell the preview page what datalist to copy rows from, and since it now removes most, tell 
// it how many to leave intact (counting from the first row in the list).
void CImportWizardPreviewDlg::SetCopyFromDatalist(_DNxDataListPtr pCopyFrom, long nLeaveCountInFromList)
{
	try{
		m_pCopyFromList = pCopyFrom;
		m_nLeaveCountInFromList = nLeaveCountInFromList;
	}NxCatchAll("Error setting list to copy from");
}

// (r.farnworth 2015-03-24 08:16) - PLID 65246 - iterate through pMap and insert into m_mapPatientIDs
void CImportWizardPreviewDlg::SetMappingFromFields(CMap<CString, LPCTSTR, long, long> *pMap)
{
	try{
		//iterate through pMap and insert into m_mapPatientIDs
		CString xKey;
		long yKey;

		POSITION pos = pMap->GetStartPosition();
		while (pos != NULL)
		{
			pMap->GetNextAssoc(pos, xKey, yKey);
			m_mapPatientIDs.SetAt(xKey, yKey);
		}
	}NxCatchAll("Error reading Patient ID mapping ");
}

void CImportWizardPreviewDlg::ClearPreview()
{
	//(e.lally 2007-06-26) PLID 5288 - Remove the rows and delete the columns
	m_pPreviewList->Clear();

	for(int i = m_pPreviewList->GetColumnCount()-1; i>=0; i--){
		m_pPreviewList->RemoveColumn(i);
	}

}
//(s.dhole 4/29/2015 12:02 PM ) - PLID 65712 changed filed type to cftTextWordWrap
#define INSERT_COLUMN(nColNum, strDataField, strName, nWidth, nStyle, DataType) \
	m_pPreviewList->InsertColumn(nColNum, _bstr_t(strDataField), _bstr_t(strName), nWidth, nStyle); \
	pCol = m_pPreviewList->GetColumn(nColNum); \
	if(pCol){ \
		pCol->PutDataType(DataType); \
        pCol->PutFieldType(cftTextWordWrap); \
	} \

void CImportWizardPreviewDlg::CreatePreviewFromCopy()
{
	//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
	CShowProgressFeedbackDlg *pProgressDlg = NULL;
	// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
	BOOL bRedrawDisabled = FALSE;
	try{
		if(m_pCopyFromList == NULL)
			return;

		//reuse the existing progress dlg, or create a new one if none exists
		pProgressDlg = ((CImportWizardDlg*)GetParent())->m_pProgressDlg;
		if(pProgressDlg == NULL)
			pProgressDlg = new CShowProgressFeedbackDlg(0, FALSE);
		long nProgressMin =0, nProgressMax = 100, nCurrentProgress=0;

		pProgressDlg->SetProgress(nProgressMin, nProgressMax, 55);
		pProgressDlg->SetCaption("Formatting Preview Records...");

		//Get all of our preferences at once using the bulk cache
		// (j.jones 2010-01-14 16:44) - PLID 31927 - added NewPatientsDefaultTextMessagePrivacy
		g_propManager.CachePropertiesInBulk("IMPORTPREVIEW", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PhoneFormatString' OR "
			"Name = 'NewPatientDefault' OR "
			"Name = 'NewPatientsDefaultTextMessagePrivacy' "
			")",
			_Q(GetCurrentUserName()));

		CProgressParameter progParam;
		//This section is carried over from our full parse and validation,
		//it will finish the last 45 units
		progParam.Init(pProgressDlg, nProgressMin, nProgressMax);
		progParam.SetSubRange(55, 100);
		//We may have hidden the progress dlg, so force a show
		pProgressDlg->ShowProgress(SW_SHOW);

		//(e.lally 2007-06-26) PLID 5288 - We need to clear out our preview in case data is already in the datalist
		ClearPreview();

		//First, insert all the new columns
		IColumnSettingsPtr pCopyFromCol, pCol;
		CString strTitle, strDataField;
		long nWidth = 100;
		EColumnStyle csStyle = csVisible;
		short nDataType = VT_BSTR;

		//We need to format special fields like Genders
		long nGenderColumn = -1;
		long nHomePhoneColumn =-1;
		long nWorkPhoneColumn = -1;
		long nPagerColumn =-1;
		long nCellPhoneColumn=-1;
		long nOtherPhoneColumn =-1;
		long nSSNColumn=-1;
		long nPatientCoordColumn = -1;
		long nNurseColumn = -1;
		long nAnesthesiologistColumn = -1;
		long nTaxable1Column = -1;
		long nTaxable2Column = -1;
		long nBillableColumn = -1; // (r.farnworth 2015-04-01 09:09) - PLID 65239
		// (b.savon 2015-04-06 07:09) - PLID 65144 - Add custom checkbox and emerg phone
		long nCustomCheck1 = -1;
		long nCustomCheck2 = -1;
		long nCustomCheck3 = -1;
		long nCustomCheck4 = -1;
		long nCustomCheck5 = -1;
		long nCustomCheck6 = -1;
		long nEmergHomePhone = -1;
		long nEmergWorkPhone = -1;
		// (b.savon 2015-05-05 09:10) - PLID 65219
		long nApptIsEvent = -1;
		long nApptIsConfirmed = -1;
		long nApptIsCancelled = -1;
		long nApptIsNoShow = -1;
		// (v.maida 2015-05-05 11:38) - PLID 65743 - Keep track of appointment date, for later formatting.
		short nApptDate = -1;
		long nApptStartTime = -1;
		long nApptEndTime = -1;
		COleDateTime dtApptDate;
		// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
		//long nPromptForCopayColumn = -1;

		for(int i=0, nCount = m_pCopyFromList->GetColumnCount(); i<nCount; i++){
			pCopyFromCol = m_pCopyFromList->GetColumn(i);
			strTitle = VarString(pCopyFromCol->GetColumnTitle(), "");
			if(strTitle.IsEmpty())
				strTitle = GetImportFieldHeader(ifnIgnore);
			strDataField = VarString(pCopyFromCol->GetFieldName(), "");
			if(strDataField == GetImportFieldDataField(ifnGender))
				nGenderColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnHomePhone))
				nHomePhoneColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnWorkPhone))
				nWorkPhoneColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnPager))
				nPagerColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnCellPhone))
				nCellPhoneColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnOtherPhone))
				nOtherPhoneColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnSocialSecurity))
				nSSNColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnUserPatientCoord))
				nPatientCoordColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnOtherContactNurse))
				nNurseColumn = i;
			else if(strDataField == GetImportFieldDataField(ifnOtherContactAnesthesiologist))
				nAnesthesiologistColumn = i;
			// (j.jones 2010-04-02 17:55) - PLID 16717 - supported service codes
			else if(strDataField == GetImportFieldDataField(ifnTaxable1))
				nTaxable1Column = i;
			else if (strDataField == GetImportFieldDataField(ifnTaxable2))
				nTaxable2Column = i;
			// (r.farnworth 2015-04-01 09:10) - PLID 65239 - added nBillable
			else if (strDataField == GetImportFieldDataField(ifnProductBillable))
				nBillableColumn = i;
			// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
			/*else if(strDataField == GetImportFieldDataField(ifnPromptForCopay))
			nPromptForCopayColumn = i;*/
			// (b.savon 2015-04-06 07:09) - PLID 65144 - Add custom checkbox and emerg phone
			else if (strDataField == GetImportFieldDataField(ifnCustomCheckbox1)){
				nCustomCheck1 = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnCustomCheckbox2)){
				nCustomCheck2 = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnCustomCheckbox3)){
				nCustomCheck3 = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnCustomCheckbox4)){
				nCustomCheck4 = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnCustomCheckbox5)){
				nCustomCheck5 = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnCustomCheckbox6)){
				nCustomCheck6 = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnEmergencyContactHomePhone)){
				nEmergHomePhone = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnEmergencyContactWorkPhone)){
				nEmergWorkPhone = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentIsEvent)){
				// (b.savon 2015-05-05 09:10) - PLID 65219
				nApptIsEvent = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentIsConfirmed)){
				// (b.savon 2015-05-05 09:10) - PLID 65219
				nApptIsConfirmed = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentIsCancelled)){
				// (b.savon 2015-05-05 09:10) - PLID 65219
				nApptIsCancelled = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentIsNoShow)){
				// (b.savon 2015-05-05 09:10) - PLID 65219
				nApptIsNoShow = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentDate)){
				// (v.maida 2015-05-05 11:38) - PLID 65743 - Keep track of appointment date, for later formatting.
				nApptDate = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentStartTime)){
				// (v.maida 2015-05-05 11:38) - PLID 65743 - Keep track of appointment date, for later formatting.
				nApptStartTime = i;
			}
			else if (strDataField == GetImportFieldDataField(ifnAppointmentEndTime)){
				// (v.maida 2015-05-05 11:38) - PLID 65743 - Keep track of appointment date, for later formatting.
				nApptEndTime = i;
			}
			nDataType = pCopyFromCol->GetDataType();

			INSERT_COLUMN(i, strDataField, strTitle, nWidth, csStyle, nDataType);
		}

		//(e.lally 2007-06-07) PLID 26250 - Add ability to show/hide ignored columns
		//Default to hiding the ignored columns
		ShowIgnoredFields(FALSE);
		CheckDlgButton(IDC_IMPORT_SHOW_IGNORED, FALSE);

		// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
		m_pPreviewList->SetRedraw(VARIANT_FALSE);
		bRedrawDisabled = TRUE;
		//Next, copy all the rows from our copy datalist
		_variant_t varNewValue;
		nDataType = VT_BSTR;
		IRowSettingsPtr pPreviewRow, pCopyRow = m_pCopyFromList->GetFirstRow();
		long nRowCount = m_pCopyFromList->GetRowCount(), nCurrentRow =0;
		DWORD dwLastProgressUpdate = GetTickCount() - 150;
		while(pCopyRow){

			// We may be removing the source row from the source list below, so get its "next" now.
			IRowSettingsPtr pNextRow = pCopyRow->GetNextRow();

			// (b.cardillo 2015-07-15 22:36) - PLID 66545 - We now remove most rows from the source, 
			// but we'll leave a requested number intact.
			if (nCurrentRow >= m_nLeaveCountInFromList) {
				// To save memory, remove the row from the source list, it's not needed there anymore.
				m_pCopyFromList->RemoveRow(pCopyRow);
			}

			//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
			// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
			if ((GetTickCount() - dwLastProgressUpdate) >= 150) {
				progParam.SetProgress(0, nRowCount, nCurrentRow);
				dwLastProgressUpdate = GetTickCount();
			}
			nCurrentRow++;
			
			// (b.cardillo 2015-07-15 22:36) - PLID 66545 - We're rewriting every column anyway, so 
			// we might as well create a new row in our list rather than passing in the source row.
			pPreviewRow = m_pPreviewList->GetNewRow();

			// (v.maida 2015-05-05 11:38) - PLID 65743 - Keep track of appointment date, for later formatting.
			if (nApptDate != -1) {
				_variant_t varCurrentApptDate = pCopyRow->GetValue(nApptDate);
				CString strApptDate = VarString(varCurrentApptDate, "");
				dtApptDate = ParseDateTime(strApptDate, false);
			}
			for(i=0, nCount = m_pPreviewList->GetColumnCount(); i<nCount; i++){
				//We also need to resave the values with the new data type
				varNewValue = pCopyRow->GetValue(i);
				//We should trim the string before checking anything
				CString strTrimmedValue = VarString(varNewValue, "");
				strTrimmedValue.TrimLeft();
				strTrimmedValue.TrimRight();
				varNewValue = _bstr_t(strTrimmedValue);

				//Check special case fields first
				if(i == nGenderColumn){
					varNewValue = _bstr_t(GetGenderAsTextFromString(VarString(varNewValue, "")));
				}
				// (b.savon 2015-04-06 07:13) - PLID 65144 - Add Emerg #'s
				else if(i == nHomePhoneColumn || i == nWorkPhoneColumn || i == nPagerColumn || i == nCellPhoneColumn || i == nOtherPhoneColumn ||
						i == nEmergHomePhone || i == nEmergWorkPhone){ 
					CString strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
					CString strValue = FormatPhone(VarString(varNewValue, ""), strPhoneFormat);
					varNewValue = _bstr_t(strValue);
				}
				else if(i == nSSNColumn){
					CString strSSNFormat = "###-##-####";
					CString strValue = FormatSSN(VarString(varNewValue, ""), strSSNFormat);
					varNewValue = _bstr_t(strValue);
				}
				//(e.lally 2007-06-12) PLID 26273 - Format patient coordinator strings to have standard values
				//(e.lally 2007-06-12) PLID 26275 - Format nurses and anesthesiologist strings to have standard values
				// (j.jones 2010-04-05 09:45) - PLID 16717 - same for Service Code tax 1 & 2, and PromptForCopay
				// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
				// (r.farnworth 2015-04-01 09:10) - PLID 65239 - Added nBillable
				// (b.savon 2015-04-06 07:13) - PLID 65144 - Add CustomChecks
				// (b.savon 2015-05-05 09:10) - PLID 65219 - Add Appt checks
				else if(i == nPatientCoordColumn || i == nNurseColumn || i == nAnesthesiologistColumn
					|| i == nTaxable1Column || i == nTaxable2Column || i == nBillableColumn /*|| i == nPromptForCopayColumn*/
					|| i == nCustomCheck1 || i == nCustomCheck2 || i == nCustomCheck3 || i == nCustomCheck4 || i == nCustomCheck5 || i == nCustomCheck6
					|| i == nApptIsEvent || i == nApptIsCancelled || i == nApptIsConfirmed || i == nApptIsNoShow){
					varNewValue = _bstr_t(GetBooleanYesNoTextFromString(VarString(varNewValue, "")));
				}

				//Default case is our non-special fields
				else{
					nDataType = m_pPreviewList->GetColumn(i)->GetDataType();
					switch(nDataType){
					case VT_BSTR:
						break;
					case VT_I4: {
							varNewValue = (long) atol(VarString(varNewValue, ""));
						break;
					}
					case VT_DATE: {
						COleDateTime dtValue, dtEmpty;
						dtEmpty.ParseDateTime("");
						// (v.maida 2015-04-08 10:28) - PLID 65423 - Use the ParseDateTime() utility function, now updated to consider ISO-8601 date formats.
						dtValue = ParseDateTime(VarString(varNewValue, ""), false);
						//Make sure it is not invalid either
							if(dtValue == dtEmpty || dtValue.m_status != COleDateTime::valid){
							_variant_t varNull;
							varNull.vt = VT_NULL;
							varNewValue = varNull;
						}
						else{
							// (v.maida 2015-05-05 10:57) - PLID 65743 - Handle time-only values.
							if (dtValue.m_dt >= 0 && dtValue.m_dt < 1) {
								// there is no date portion to this field, just a time portion. Using a VT_DATE variant for this would result in a date of 1899 
								// being shown, so use a string, instead.
								varNewValue.vt = VT_BSTR;
								varNewValue = _bstr_t(dtValue.Format("%I:%M %p"));
							}
							else {
								// (v.maida 2015-05-05 10:57) - PLID 65743 - If we're looking at an appointment start/end time, it's possible that the date portion of the value does not
								// match up with the chosen appointment date value so, if that's the case, just change the date portion to the appointment date, since the SQL import will 
								// later be doing that same thing.
								if (dtApptDate != COleDateTime::invalid && (i == nApptStartTime || i == nApptEndTime)
									&& ( (dtApptDate.GetYear() != dtValue.GetYear()) || (dtApptDate.GetMonth() != dtValue.GetMonth()) || (dtApptDate.GetDay() != dtValue.GetDay()) ) ) {
									dtValue.SetDateTime(dtApptDate.GetYear(), dtApptDate.GetMonth(), dtApptDate.GetDay(), dtValue.GetHour(), dtValue.GetMinute(), dtValue.GetSecond());
								}

								// (v.maida 2015-05-05 10:57) - PLID 65743 - Handle time-only values.
								// this value has a date portion, so no need for special time-only formatting, but there may be need to show 12:00 AM 
								// for 0 value time portions.
								double timeFraction = dtValue.m_dt - ((long)dtValue.m_dt);

								if (timeFraction == 0 && (i == nApptStartTime || i == nApptEndTime))
								{
									// we have a time of 0, so 12:00 AM needs to be shown
									varNewValue.vt = VT_BSTR;
									varNewValue = _bstr_t(dtValue.Format() + " 12:00 AM");
								}
								else {
									varNewValue = (DATE)dtValue;
								if(varNewValue.vt == VT_R8)
										varNewValue.vt = VT_DATE;
								}
							}
						}
						break;
					}
					case VT_CY: {
						COleCurrency cyValue;
						cyValue.ParseCurrency(VarString(varNewValue, ""));
						varNewValue = cyValue;
						break;
					}
					case VT_R8: {
						// (j.jones 2010-04-05 12:22) - PLID 38050 - supported doubles
						double dblNewValue = (double)atof(VarString(varNewValue, ""));
						varNewValue = dblNewValue;
						break;
					}
					default:
						varNewValue.ChangeType(nDataType, &varNewValue);
						break;
					}
				}//end else
				
				//Set the value back with the new data type
				pPreviewRow->PutValue(i, varNewValue);
			}
			
			// Add the new row to the end of the preview list
			m_pPreviewList->AddRowAtEnd(pPreviewRow, NULL);

			// Go to the next row (which we had retreived at the beginning of the loop because the 
			// source row might be gone from the source list so no longer have a "next" row).
			pCopyRow = pNextRow;
		}

		// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
		progParam.SetProgress(0, nRowCount, nCurrentRow);

		// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
		if (bRedrawDisabled) {
			m_pPreviewList->SetRedraw(VARIANT_TRUE);
			bRedrawDisabled = FALSE;
		}

		//we can hide and then destroy this progress dlg since we are done with it for this action
		pProgressDlg->ShowProgress(SW_HIDE);
		delete pProgressDlg;
		pProgressDlg = NULL;
		((CImportWizardDlg*)GetParent())->m_pProgressDlg = NULL;
		return;
	}NxCatchAll("Error copying data to preview table");
	// (b.cardillo 2015-05-13 16:22) - PLID 66098 - Disable redraw while setting cell colors
	try {
		if (bRedrawDisabled) {
			if (m_pPreviewList) {
				m_pPreviewList->SetRedraw(VARIANT_TRUE);
				bRedrawDisabled = FALSE;
			}
		}
	} NxCatchAllIgnore();
	try {
		//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
		//Let's make sure the progress dlg is cleaned up if we catch an error
		if(pProgressDlg){
			pProgressDlg->ShowProgress(SW_HIDE);
			((CImportWizardDlg*)GetParent())->m_pProgressDlg = NULL;
			delete pProgressDlg;
			pProgressDlg = NULL;
		}
	} NxCatchAllIgnore();
}

BOOL CImportWizardPreviewDlg::OnWizardFinish()
{
	try{
		// (b.savon 2015-03-19 08:38) - PLID 65248 - Add an option to perform a backup before importing files
		if (m_chkPerformBackup.GetCheck() == BST_CHECKED){
			if (IDYES == MessageBox("You are about to run a backup, this may impact system performance.  Please make sure the system is not under heavy load at this time.  Are you sure you wish to proceed?", "Nextech Backup", MB_ICONINFORMATION | MB_YESNO)){
				PerformManualDatabaseBackup(GetSafeHwnd());
			}
			else{
				return FALSE;
			}
		}
		else{
			if (IDNO == MessageBox("You have chosen NOT to backup the database before importing.  This process cannot be undone and these records will be permanently added to your database.  Are you sure you wish to proceed?", "Nextech", MB_ICONWARNING | MB_YESNO)){
				return FALSE;
			}
		}

		//They are really, really sure.
		SaveImportedData();

		return CNxPropertyPage::OnWizardFinish();

	}NxCatchAll("Error finishing import wizard");
	return FALSE;
}

// (r.goldschmidt 2016-02-02 17:13) - PLID 67976 - reset letting the user know which columns won't convert when going back
LRESULT CImportWizardPreviewDlg::OnWizardBack()
{
	try {

		GetDlgItem(IDC_STATIC)->SetWindowText(m_strOriginalInstructionText);

		return CNxPropertyPage::OnWizardBack();

	}NxCatchAll(__FUNCTION__);
	return FALSE;

}


void CImportWizardPreviewDlg::SaveImportedData()
{
	//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
	CShowProgressFeedbackDlg *pProgressDlg = NULL;
	long nAuditTransactionID = -1;
	try{
		//reuse the existing progress dlg, or create a new one if none exists
		pProgressDlg = ((CImportWizardDlg*)GetParent())->m_pProgressDlg;
		if(pProgressDlg == NULL)
		{
			// (j.politis 2015-05-01 14:04) - PLID 65526 - Add a progress bar when doing the import that tell you that an import is in progress
			pProgressDlg = new CShowProgressFeedbackDlg(200, FALSE, FALSE, TRUE);
		}

		long nProgressMin =0, nProgressMax = 100, nCurrentProgress=0;

		pProgressDlg->SetProgress(nProgressMin, nProgressMax, 0);
		// (j.politis 2015-05-01 14:04) - PLID 65526 - Add a progress bar when doing the import that tell you that an import is in progress
		pProgressDlg->SetCaption("An import is currently in progress. Please do not touch this system!");

		CProgressParameter progParam;
		//Initialize this subprogress to still use 0-100
		progParam.Init(pProgressDlg, nProgressMin, nProgressMax);
		//It will fill in all the units 
		progParam.SetSubRange(0, 100);
		//Since we may have hidden the dlg, force it to show
		pProgressDlg->ShowProgress(SW_SHOW);

		//Go through each field and generate the appropriate insert statements into a batch
		CString strSqlBatch = BeginSqlBatch();

		// (b.savon 2015-04-30 16:02) - PLID 65511 - Break out into a new function
		AddImportDeclareVariablesToBatch(strSqlBatch);
		
		// (r.goldschmidt 2016-01-27 18:33) - PLID 67976 - put appt type of 'Conversion' in if necessary
		CString strApptTypeName = GetApptTypeNameConversion();
		bool bApptTypeConversion = !!IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION_SAVED);
		if (bApptTypeConversion){
			if (IsRecordsetEmpty("SELECT 1 FROM AptTypeT WHERE Name = '%s'", strApptTypeName)) {
				long nId = NewNumber("AptTypeT", "ID");
				ExecuteSql("INSERT INTO AptTypeT (ID, Name) Values (%li, '%s')", nId, strApptTypeName);
				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if (nAuditID != -1) {
					AuditEvent(-1, "", nAuditID, aeiSchedTypeCreated, nId, "", strApptTypeName, aepMedium, aetCreated);
				}
			}
		}

		CString strDataField;
		
		IColumnSettingsPtr pCol;
		long nColumnCount = m_pPreviewList->GetColumnCount();
		//(e.lally 2007-06-29) PLID 26508 - Improve performance for larger files
		//To help optimize this process, let's remove the ignored columns
		// *Note* - there was a bug in the datalist where removing these columns still
		//left the data in the rows, so the column numbers and titles no longer matched the
		//data at the same index in the row. It should be fixed in PLID 26484.
		/*
		for(int j=0, nCur=0; j<nColumnCount; j++){
		pCol = m_pPreviewList->GetColumn(nCur);
		if(pCol != NULL){
		_variant_t varDataField = pCol->GetFieldName();
		strDataField = VarString(varDataField, "");
		if(strDataField == GetImportFieldDataField(ifnIgnore) || strDataField.IsEmpty()){
		m_pPreviewList->RemoveColumn(nCur);
		}
		else{
		nCur++;
		}
		}
		}
		//Recalculate the column count
		nColumnCount = m_pPreviewList->GetColumnCount();
		*/

		//Now get the first row
		IRowSettingsPtr pRow = m_pPreviewList->GetFirstRow();

		_variant_t varValue, varDataField;

		long nRowCount = m_pPreviewList->GetRowCount(), nPos =0;

		ImportRecordType irtCurrent = ((CImportWizardDlg*)GetParent())->m_irtRecordType;

		//(e.lally 2007-07-02) PLID 26503 - determine the audit event item we need to create.
		long nAuditItem =-1;
		switch (irtCurrent){
		case irtPatients:
		case irtMediNotes:
			nAuditItem = aeiPatientCreated; break;
		case irtProviders:
			nAuditItem = aeiProviderCreated; break;
		case irtReferringPhysicians:
			nAuditItem = aeiRefPhysCreated; break;
		case irtSuppliers:
			nAuditItem = aeiSupplierCreated; break;
		case irtUsers:
			nAuditItem = aeiUserCreated; break;
		case irtOtherContacts:
			nAuditItem = aeiOtherCreated; break;
		case irtServiceCodes:
			// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes
			nAuditItem = aeiCPTCreate;
			break;
		case irtResources:
			// (r.farnworth 2015-03-16 14:55) - PLID 65197 - Resources aren't audited
			break;
		case irtProducts:
			// (r.farnworth 2015-03-19 10:09) - PLID 65238 - Add a new import type, Products, to the't import utility
			nAuditItem = aeiProductCreate;
			break;
		case irtRecalls:
			// (b.savon 2015-05-01 09:08) - PLID 65236 - Audit recall
			nAuditItem = aeiPatientRecallCreatedWithTemplate;
			break;
		case irtInsuranceCos:
			// (r.farnworth 2015-04-01 16:17) - PLID 65166 - Despite all of the audit records for Insurance, there isn't one for adding a new company.
			break;
		case irtPatientNotes:
			//(s.dhole 4/7/2015 10:48 AM ) - PLID 65224 audit patient notes
			nAuditItem = aeiPatientNote;
			break;
		case irtAppointments:
			// (b.savon 2015-04-22 14:03) - PLID 65219 - Add validation for Appointment object -- Start Time, End Time, Duration, Is Event fields and ensure valid values are saved to data.
			nAuditItem = aeiApptStatus;
			break;
			//(s.dhole 4/28/2015 9:51 AM ) - PLID 65755  Add audit to  insured party 
		case irtInsuredParties :
			nAuditItem = aeiInsuredPartyAdded;
			break;
		case irtRaces:
			// (r.goldschmidt 2016-02-10 11:14) - PLID 68163 - add audit for race
			nAuditItem = aeiRaceCreated;
			break;
		}

		BOOL bRecallWasCreated = FALSE;

		long nBatchRecordCount =0;
		DWORD dwLastProgressUpdate = GetTickCount() - 150;
		while (pRow){
			//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
			// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
			if ((GetTickCount() - dwLastProgressUpdate) >= 150) {
				progParam.SetProgress(0, nRowCount, nPos);
				// (j.politis 2015-05-01 14:04) - PLID 65526 - Add a progress bar when doing the import that tell you that an import is in progress
				pProgressDlg->SetSubCaption(FormatString("Records imported: %li of %li", nPos, nRowCount));
				dwLastProgressUpdate = GetTickCount();
			}
			nPos++;

			//Each person/object will be in their own batch for the best performance using this algorithm
			strSqlBatch += "BEGIN TRAN \r\n";

			// (b.savon 2015-04-30 16:02) - PLID 65511 - Set all the fields and values back to empty for the new row
			ResetFieldsAndValues();

			for (int i = 0; i < nColumnCount; i++){
				pCol = m_pPreviewList->GetColumn(i);
				varDataField = pCol->GetFieldName();
				strDataField = VarString(varDataField, "");
				varValue = pRow->GetValue(i);

				// (b.savon 2015-03-24 11:22) - PLID 65251 - Run capsfix here; create function to check if the data field
				// qualifies as a capsfixer.  If so, feed it through the algorithm.
				CString strValue;
				if (varValue.vt == VT_BSTR && ShouldFixCaps(strDataField)){
					strValue = VarString(varValue, "");
					if (!strValue.IsEmpty()){
						FixCaps(strValue);
						varValue = _variant_t(AsBstr(strValue));
					}
				}

				//Append the appropriate value syntax
				AppendValue(irtCurrent, strDataField, varValue);

			}

			//Resource fields
			// (r.farnworth 2015-03-16 15:07) - PLID 65197 - Resource Fields
			if (irtCurrent == irtResources) {
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddResourceToBatch(strSqlBatch);
			}
			// Products fields
			// (r.farnworth 2015-03-19 10:38) - PLID 65238 - Add a new import type, Products, to the't import utility
			else if (irtCurrent == irtProducts) {
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddProductToBatch(strSqlBatch);
				}
			//Patient fields or MediNotes patient fields
			//(e.lally 2007-06-29) PLID 26509 - MediNotes patients
			else if (irtCurrent == irtPatients || irtCurrent == irtMediNotes){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddPatientToBatch(strSqlBatch);
				}
			//Provider fields
			//(e.lally 2007-06-08) PLID 26262 - Import list of providers
			else if (irtCurrent == irtProviders){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddProviderToBatch(strSqlBatch);
				}
			//Referring Physician fields
			//(e.lally 2007-06-11) PLID 10320 - Import list of referring physicians
			else if (irtCurrent == irtReferringPhysicians){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddReferringPhysicianToBatch(strSqlBatch);
			}
			//User fields
			//(e.lally 2007-06-12) PLID 26273 - Import list of users
			else if (irtCurrent == irtUsers){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddUserToBatch(strSqlBatch);
			}
			//Supplier fields
			//(e.lally 2007-06-11) PLID 26274 - Import list of suppliers
			else if (irtCurrent == irtSuppliers){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddSupplierToBatch(strSqlBatch);
			}
			//Other Contact fields
			//(e.lally 2007-06-12) PLID 26275 - Import list of Other Contacts
			else if (irtCurrent == irtOtherContacts){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddOtherContactToBatch(strSqlBatch);
			}
			// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes			
			//This section handles CPTCodeT.
			else if (irtCurrent == irtServiceCodes) {
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddServiceCodeToBatch(strSqlBatch);
				}
			//Recall fields
			// (b.savon 2015-04-01 11:47) - PLID 65235 - Add Recalls object 
			else if (irtCurrent == irtRecalls){
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddRecallToBatch(strSqlBatch);
				bRecallWasCreated = TRUE;
				}
			//Insurance Company fields
			// (r.farnworth 2015-04-01 16:36) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
			else if (irtCurrent == irtInsuranceCos) {
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddInsuranceCompanyToBatch(strSqlBatch);
				}
			//Patient note fields
			//(s.dhole 4/8/2015 1:38 PM ) - PLID 65224 save patient notes
			else if (irtCurrent == irtPatientNotes) {
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddPatientNoteToBatch(strSqlBatch);
				}
			//Appointment fields
			// (b.savon 2015-04-07 10:19) - PLID 65216 - Create fields for the Appointment object for the import utility.
			else if (irtCurrent == irtAppointments){
				AddAppointmentToBatch(strSqlBatch);
					}
			//Insured Party fields
			else if (irtCurrent == irtInsuredParties) {
				// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
				AddInsuredPartyToBatch(strSqlBatch);
					}
			//Race fields
			else if (irtCurrent == irtRaces) {
				// (r.goldschmidt 2016-02-10 11:20) - PLID 68163 - Split batch logic out
				AddRaceToBatch(strSqlBatch);
			}


			nBatchRecordCount++;
			pRow = pRow->GetNextRow();

			//Commit each individual person/object
			strSqlBatch += " COMMIT TRAN \r\n";

			// (b.savon 2015-03-30 16:57) - PLID 65231 - Recall doesnt have an audit for adding
			if (irtCurrent != irtResources && irtCurrent != irtRecalls && irtCurrent != irtAppointments && irtCurrent != irtRaces){
				//(e.lally 2007-07-02) PLID 26503 - Add each person to our audit transaction
				if (nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				//(e.lally) Ideally, we would have all the person IDs at our disposal for auditing the record ID
				//Unfortunately, there does not appear to be an efficient way to get that information using this
				//algorithm. We may decide that it is worth the speed hit to include that in the audit, but for now
				//it is going to have to be dropped.
				long nRecordID = -1;
				m_strAuditNewValue.TrimRight(": ");
				m_strAuditNewValue.TrimRight(", ");
				
				// (a.walling 2010-01-21 14:25) - PLID 37023 - See above comment for why we don't have the patient ID here
				AuditEvent(-1, m_strAuditPersonName, nAuditTransactionID, nAuditItem, nRecordID, "", m_strAuditNewValue, aepMedium, aetCreated);
			}
			else if (irtCurrent == irtAppointments){ // (b.savon 2015-04-22 14:28) - PLID 65219 - Add appt auditing
				if (nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				long nRecordID = -1;
				m_strAuditNewValue.TrimRight(": ");
				m_strAuditNewValue.TrimRight(", ");
				
				//For appts, we've hijacked the m_strAuditPersonName for the patientID.
				long nPatientID = atol(m_strAuditPersonName);

				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, nAuditItem, nRecordID, "", m_strAuditAppointmentValues, aepHigh, aetCreated);
			}
			else if (irtCurrent == irtRecalls){ 
				// (b.savon 2015-05-01 09:13) - PLID 65236 - Audit recall creation
				if (nAuditTransactionID == -1){
					nAuditTransactionID = BeginAuditTransaction();
				}

				long nRecordID = -1;
				CString strRecallCreation;
				strRecallCreation = m_strAuditNewValue + " Recall Created";

				//For appts, we've hijacked the m_strAuditPersonName for the patientID.
				long nPatientID = atol(m_strAuditPersonName);

				AuditEvent(nPatientID, ::GetExistingPatientName(nPatientID), nAuditTransactionID, nAuditItem, nRecordID, "", strRecallCreation, aepMedium, aetCreated);
			}
			else if (irtCurrent == irtRaces) {
				// (r.goldschmidt 2016-02-10 12:36) - PLID 68163 - audit race creation
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				
				long nRecordID = -1;

				CString strNewRace = "";
				
				// prep auditing adding race code to race
				if (m_bPairedToRaceCDCCode) { 
					ADODB::_RecordsetPtr rsRace = CreateParamRecordset("SELECT OfficialRaceName FROM RaceCodesT WHERE CDCID = {STRING}", m_strRaceCDCCodeValue);
					strNewRace = AdoFldString(rsRace, "OfficialRaceName", "<None>");
				}

				// if there was no preferred name, then the official name was added
				if (!m_bHasRacePreferredName) {
					m_strAuditNewValue = strNewRace;
				}
				
				// audit adding to race list
				AuditEvent(-1, "", nAuditTransactionID, nAuditItem, nRecordID, "", m_strAuditNewValue, aepMedium, aetCreated);

				// audit adding race code to race
				if (m_bPairedToRaceCDCCode) {
					AuditEvent(-1, "", nAuditTransactionID, aeiRaceRaceCode, nRecordID, "<None>", strNewRace, aepMedium, aetChanged);
				}
			}

			if(nBatchRecordCount >=100){
				//(e.lally 2007-06-29) PLID 26508 - Improve performance for larger files
				//Let's run our batches thus far (in groups of 100 for starters) because the SQL server doesn't appear to be able to
				//handle huge numbers of inserts at once (without using something like a bulk insert which this algorithm doesn't account for)
				//in a timely manner. This actually had the best performance out of all the variations
				//that were tried. Including batches of 100 objects at once, one batch of all single transactions
				// and one batch of one big transaction.

				//Execute the batch
				// (b.savon 2015-04-30 16:22) - PLID 65511 - Split out the batch execution logic
				RunBatch(nAuditTransactionID, strSqlBatch);

				//Set our count back
				nBatchRecordCount =0;
				//Reset our SQL batch declares etc
				strSqlBatch = BeginSqlBatch();
				
				// (b.savon 2015-04-30 16:02) - PLID 65511 - Break out into a new function
				AddImportDeclareVariablesToBatch(strSqlBatch);

				//Reset our audit transactionID
				nAuditTransactionID = -1;
			}

		}

		// (b.cardillo 2015-05-13 15:25) - PLID 66097 - Don't update the progress bar too often.
		progParam.SetProgress(0, nRowCount, nPos);
		pProgressDlg->SetSubCaption(FormatString("Records imported: %li of %li", nPos, nRowCount));

		if(nRowCount > 0){
			//(e.lally 2007-06-29) PLID 26508 - Improve performance for larger files
			if(nBatchRecordCount > 0){
				//Let's run the last of our batches
				// (b.savon 2015-04-30 16:22) - PLID 65511 - Split out the batch execution logic
				RunBatch(nAuditTransactionID, strSqlBatch);
						}

			//(e.lally 2007-06-29) PLID 26509 - Refresh patients for a MediNotes import too
			if(irtCurrent == irtPatients || irtCurrent == irtMediNotes)
				CClient::RefreshTable(NetUtils::PatCombo, -1);
			//(e.lally 2007-06-08) PLID 26262 - refresh provider lists
			else if(irtCurrent == irtProviders)
				CClient::RefreshTable(NetUtils::Providers, -1);
			//(e.lally 2007-06-11) PLID 10320 - refresh refPhys lists
			else if(irtCurrent == irtReferringPhysicians)
				CClient::RefreshTable(NetUtils::RefPhys, -1);
			//(e.lally 2007-06-12) PLID 26273 - refresh patient coordinator lists
			else if(irtCurrent == irtUsers)
				CClient::RefreshTable(NetUtils::Coordinators, -1);
			//(e.lally 2007-06-11) PLID 26274 - refresh supplier lists
			else if(irtCurrent == irtSuppliers)
				CClient::RefreshTable(NetUtils::Suppliers, -1);
			//(e.lally 2007-06-12) PLID 26275 - refresh other contact lists
			else if(irtCurrent == irtOtherContacts)
				CClient::RefreshTable(NetUtils::ContactsT, -1);
			// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes
			else if (irtCurrent == irtServiceCodes)
				CClient::RefreshTable(NetUtils::CPTCodeT, -1);
			// (r.farnworth 2015-03-16 15:23) - PLID 65197 - Add a new import type, Resources, to the import utility
			else if (irtCurrent == irtResources)
				CClient::RefreshTable(NetUtils::Resources, -1);
			// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the't import utility
			else if (irtCurrent == irtProducts)
				CClient::RefreshTable(NetUtils::Products, -1);
			// (r.farnworth 2015-04-01 16:49) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
			else if (irtCurrent == irtInsuranceCos)
				CClient::RefreshTable(NetUtils::InsuranceCoT, -1);
			//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
			else if (irtCurrent == irtInsuredParties)
				CClient::RefreshTable(NetUtils::PatInsParty, -1);
			
			if (bRecallWasCreated) {
				// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
				GetMainFrame()->HandleRecallChanged();
			}
		}

		//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
		//That's the end of the save so we can destroy our progress dlg
		pProgressDlg->ShowProgress(SW_HIDE);
		if(pProgressDlg != NULL){
			((CImportWizardDlg*)GetParent())->m_pProgressDlg = NULL;
			delete pProgressDlg;
			pProgressDlg = NULL;
		}

		return;
	}NxCatchAll("Error finishing import wizard");
	//(e.lally 2007-07-02) PLID 26503 - Rollback any pending audit transactions
	if(nAuditTransactionID != -1)
		RollbackAuditTransaction(nAuditTransactionID);
	//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
	//Let's make sure the progress dlg is cleaned up if we catch an error
	if(pProgressDlg){
		pProgressDlg->ShowProgress(SW_HIDE);
		((CImportWizardDlg*)GetParent())->m_pProgressDlg = NULL;
		delete pProgressDlg;
		pProgressDlg = NULL;
	}
}


#define IS_FIELD strDataField == GetImportFieldDataField
#define APPEND_PERSON_STRING \
	m_strPersonFields += " " + strDataField +","; \
	m_strPersonValues += " '" + _Q(VarString(varValue, ""))+"', "; \
	return;
#define APPEND_PERSON_DATE \
	m_strPersonFields += " " + strDataField +","; \
	if(varValue.vt == VT_NULL){ \
		m_strPersonValues += " NULL, "; }\
					else { \
		m_strPersonValues += " '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDate)+"', ";}\
	return;
#define APPEND_PERSON_PHONE \
	m_strPersonFields += " " + strDataField +","; \
	m_strPersonValues += " '" + _Q(FormatPhoneForSql(VarString(varValue, "")))+"', "; \
	return;

// (b.savon 2015-03-23 08:36) - PLID 65156 - Add Patient int macro
#define APPEND_PATIENT_INT \
	m_strPatientFields += " " + strDataField + ", "; \
	m_strPatientValues += " " + AsString(nVarIntValue) + ", "; \
	return;

// (b.savon 2015-03-23 16:09) - PLID 65161 - Add Patient string macro
#define APPEND_PATIENT_STRING \
	m_strPatientFields += " " + strDataField + ", "; \
	m_strPatientValues += " '" + _Q(VarString(varValue, ""))+"', "; \
	return;

#define APPEND_PROVIDER_STRING \
	m_strProviderFields += " " + strDataField +","; \
	m_strProviderValues += " '" + _Q(VarString(varValue, ""))+"', "

#define APPEND_REF_PHYS_STRING \
	m_strRefPhysFields += " " + strDataField +","; \
	m_strRefPhysValues += " '" + _Q(VarString(varValue, ""))+"', "

#define APPEND_SUPPLIER_STRING \
	m_strSupplierFields += " " + strDataField +","; \
	m_strSupplierValues += " '" + _Q(VarString(varValue, ""))+"', "

#define APPEND_USER_STRING \
	m_strUserFields += " " + strDataField +","; \
	m_strUserValues += " '" + _Q(VarString(varValue, ""))+"', "

#define APPEND_USER_DATE \
	m_strUserFields += " " + strDataField +","; \
	if(varValue.vt == VT_NULL){ \
		m_strUserValues += " NULL, "; }\
					else { \
		m_strUserValues += " '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDate)+"', ";}

// (j.jones 2010-04-05 09:47) - PLID 16717 - added service codes
#define APPEND_SERVICE_STRING \
	m_strServiceFields += " " + strDataField +","; \
	m_strServiceValues += " '" + _Q(VarString(varValue, ""))+"', "

// (j.jones 2010-04-05 09:47) - PLID 16717 - added service codes
#define APPEND_CPT_CODE_STRING \
	m_strCPTCodeFields += " " + strDataField +","; \
	m_strCPTCodeValues += " '" + _Q(VarString(varValue, ""))+"', "

// (r.farnworth 2015-03-16 15:47) - PLID 65197 - Add a new import type, Resources, to the import utility
#define APPEND_RESOURCE_STRING \
	m_strResourceFields += " " + strDataField +","; \
	m_strResourceValues += " '" + _Q(VarString(varValue, ""))+"', "

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_SERVICET_STRING \
	m_strProductServiceTFields += " " + strDataField +","; \
	m_strProductServiceTValues += " '" + _Q(VarString(varValue, ""))+"', "

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_SERVICET_MONEY \
	m_strProductServiceTFields += " " + strDataField + ","; \
	CString strTemp; \
	COleCurrency cy; \
	if (varValue.vt == VT_CY && VarCurrency(varValue) >= COleCurrency(0, 0)) { \
		cy = VarCurrency(varValue); \
		} \
	strTemp.Format(" Convert(money,'%s'), ", _Q(FormatCurrencyForSql(cy))); \
	m_strProductServiceTValues += strTemp;

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_PRODUCTT_MONEY \
	m_strProductProductTFields += " " + strDataField + ","; \
	CString strTemp; \
	COleCurrency cy; \
	if (varValue.vt == VT_CY && VarCurrency(varValue) >= COleCurrency(0, 0)) { \
		cy = VarCurrency(varValue); \
			} \
	strTemp.Format(" Convert(money,'%s'), ", _Q(FormatCurrencyForSql(cy))); \
	m_strProductProductTValues += strTemp;

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_SERVICET_BIT \
	m_strProductServiceTFields += " " + strDataField + ","; \
	CString strTemp; \
	strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, ""))); \
	m_strProductServiceTValues += strTemp;

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_PRODUCTT_STRING \
	m_strProductProductTFields += " " + strDataField + ","; \
	m_strProductProductTValues += " '" + _Q(VarString(varValue, "")) + "', "

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_PRODUCTLOCATIONINFOT_BIT \
	m_strProductProductLocationInfoTFields += " " + strDataField + ","; \
	CString strTemp; \
	strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, ""))); \
	m_strProductProductLocationInfoTValues += strTemp; 
	

// (r.farnworth 2015-03-19 10:47) - PLID 65238 - Add a new import type, Products, to the import utility
#define APPEND_PRODUCT_PRODUCTADJUSTMENTS_LONG \
	m_strProductProductAdjustmentsTFields += " " + strDataField + ","; \
	CString strTemp; \
	strTemp.Format(" %li, ",  VarLong(varValue, 0)); \
	m_strProductProductAdjustmentsTValues += strTemp;

// (b.savon 2015-03-20 08:45) - PLID 65153 - Add Race
#define APPEND_PERSON_RACE_STRING \
	m_strPersonRaceFields += " " + strDataField + ", "; 

// (b.savon 2015-03-20 15:01) - PLID 65154 - Add Ethnicity
#define APPEND_PERSON_ETHNICITY_INT \
	m_strPersonEthnicityFields += " " + strDataField + ", "; \

// (b.savon 2015-03-23 07:32) - PLID 65155 - Add Language
#define APPEND_PERSON_LANGUAGE_INT \
	m_strPersonLanguageFields += " " + strDataField + ", "; \

// (b.savon 2015-03-23 10:41) - PLID 65157 - Add Location
#define APPEND_LOCATION_INT \
	m_strLocationFields += " " + strDataField + ", "; \

// (b.savon 2015-03-23 11:28) - PLID 65158 - Add Referral Source Name
#define APPEND_REFERRAL_INT \
	m_strReferralSourceFields += " " + strDataField + ", "; \

// (b.savon 2015-03-25 07:45) - PLID 65144 - Add Custom Text
#define APPEND_CUSTOM_TEXT \
if (!_Q(VarString(varValue, "")).IsEmpty()){ \
	m_strCustomFieldsQuery += "INSERT INTO CustomFieldDataT(PersonID, FieldID, IntParam, TextParam, DateParam) VALUES (@PersonID, " + strCustomFieldID + ", NULL, '" + _Q(VarString(varValue, "")) + "', NULL) \r\n"; \
}

// (b.savon 2015-03-25 07:45) - PLID 65144 - Add Custom Int
#define APPEND_CUSTOM_CHECKBOX \
if (!_Q(VarString(varValue, "")).IsEmpty()){ \
	m_strCustomFieldsQuery += "INSERT INTO CustomFieldDataT(PersonID, FieldID, IntParam, TextParam, DateParam) VALUES (@PersonID, " + strCustomFieldID + ", " + AsString(GetSqlValueFromBoolString(_Q(VarString(varValue, "")))) + ", NULL, NULL) \r\n"; \
}

// (b.savon 2015-03-25 07:45) - PLID 65144 - Add Custom Date
#define APPEND_CUSTOM_DATE \
if (varValue.vt != VT_NULL && varValue.vt == VT_DATE){ \
	m_strCustomFieldsQuery += "INSERT INTO CustomFieldDataT(PersonID, FieldID, IntParam, TextParam, DateParam) VALUES (@PersonID, " + strCustomFieldID + ", NULL, NULL, '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDate) + "') \r\n"; \
}

// (b.savon 2015-03-25 13:26) - PLID 65150 - Add ProviderID
#define APPEND_PATIENT_PROVIDER_STRING \
	m_strPatientProviderFields += " " + strDataField + ", "; \

// (b.savon 2015-03-26 13:26) - PLID 65151 - Add Referring Physician ID
#define APPEND_PATIENT_REFPHYS_STRING \
	m_strPatientReferringPhysFields += " " + strDataField + ", "; \

// (b.savon 2015-03-26 14:23) - PLID 65152 - Add PCP Name 
#define APPEND_PATIENT_PCP_STRING \
	m_strPatientPCPFields += " " + strDataField + ", "; \

// (b.savon 2015-04-01 11:45) - PLID 65235 - Add Recall date
#define APPEND_RECALL_DATE \
	if(varValue.vt != VT_NULL && varValue.vt == VT_DATE){ \
		m_strPatientRecallFields += " " + strDataField +","; \
		m_strPatientRecallValues += " '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDate)+"', "; \
		} \
	return;

// (b.savon 2015-04-02 09:26) - PLID 65236 - Add Recall Template 
#define APPEND_RECALL_TEMPLATE \
	m_strPatientRecallTemplateFields += " " + strDataField + ","; \

// (r.farnworth 2015-04-01 17:17) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
// (r.farnworth 2015-06-09 15:04) - PLID 65166 - EBillingIDs are not unique so we must choose 1. Any 1 is okay as long as it exists.
#define APPEND_INSURANCECOT_PAYERID \
	m_strInsuranceCoTFields += " " + strDataField + ", "; \
	m_strInsuranceCoTValues += " (SELECT TOP 1 ID FROM EBillingInsCoIDs WHERE EBillingID = '" + _Q(VarString(varValue, "")) + "'), "

// (r.farnworth 2015-04-01 17:17) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
#define APPEND_INSURANCECOT_STRING \
	m_strInsuranceCoTFields += " " + strDataField + ","; \
	m_strInsuranceCoTValues += " '" + _Q(VarString(varValue, "")) + "', ";

// (r.farnworth 2015-04-01 17:17) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
#define APPEND_INSURANCECONTACT_STRING \
	m_strInsuranceContactFields += " " + strDataField + ","; \
	m_strInsuranceContactValues += " '" + _Q(VarString(varValue, "")) + "', ";

// (r.farnworth 2015-04-01 17:24) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
#define APPEND_INSURANCECONTACT_PHONE \
	m_strInsuranceContactFields += " " + strDataField + ","; \
	m_strInsuranceContactValues += " '" + _Q(FormatPhoneForSql(VarString(varValue, ""))) + "', ";


//(s.dhole 4/7/2015 3:41 PM ) - PLID 65227 note date
#define APPEND_PATIENT_NOTE_DATE\
	if(varValue.vt != VT_NULL && varValue.vt == VT_DATE){ \
		m_strPatientNotesFields += " " + strDataField + ","; \
		m_strPatientNotesValues += " '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDateTime)+"', "; \
			} \
	return;
//(s.dhole 4/7/2015 3:41 PM ) - PLID 65224
#define APPEND_NOTE_CATEGORY \
	m_strPatientNoteCategoryFields += " " + strDataField + ","; \

//(s.dhole 4/8/2015 1:42 PM ) - PLID 65229
#define APPEND_NOTE_PRIORITY \
	m_strPatientNotePriorityFields += " " + strDataField + ","; \

//(s.dhole 4/8/2015 1:42 PM ) - PLID 65230
#define APPEND_PATIENT_NOTE_STRING \
	m_strPatientNotesTextFields += " " + strDataField +","; \
	m_strPatientNotesTextValues += VarString(varValue, ""); \

// (b.savon 2015-04-07 10:19) - PLID 65216 - Create fields for the Appointment object for the import utility.
#define APPEND_APPT_STRING \
	m_strAppointmentFields += " " + strDataField + ","; \
	m_strAppointmentValues += " '" + _Q(VarString(varValue, "")) + "', ";

// (b.savon 2015-04-07 14:59) - PLID 65218 - Add Appointment Date
#define APPEND_APPT_DATE \
	if (varValue.vt != VT_NULL && varValue.vt == VT_DATE){ \
			m_strAppointmentFields += " " + strDataField + ","; \
			m_strAppointmentValues += " '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDate) + "', "; \
		} \
	return;

// (b.savon 2015-04-07 15:24) - PLID 65220 - Add Appointment Type
#define APPEND_APPT_TYPE \
	m_strAppointmentTypeFields += " " + strDataField + ", "; \

// (b.savon 2015-04-09 08:10) - PLID 65223 - Add Appointment Resource Name 
#define APPEND_APPT_RESOURCE \
	m_strResourceFields += "AppointmentID, ResourceID,"; \
	m_strResourceValues += "(SELECT ID FROM ResourceT WHERE Item = '" + _Q(VarString(varValue, "")) + "') "; \

// (b.savon 2015-04-10 11:04) - PLID 65221 - Add Appointment Purpose
#define APPEND_APPT_PURPOSE \
	m_strAppointmentPurposeFields += " " + strDataField + ", "; \

// (b.savon 2015-04-14 09:25) - PLID 65216 - Add Field
#define APPEND_APPT_FIELD \
	m_strAppointmentFields += " " + strDataField + ","; \


//(s.dhole 4/28/2015 10:00 AM ) - PLID 65755 Isured effective and inactive date
#define APPEND_INSURED_PARTY_EFFECTIVE_DATE\
	if(varValue.vt != VT_NULL && varValue.vt == VT_DATE){ \
		m_strInsurdPartyFields += " " + strDataField + ","; \
		m_strInsurdPartyValues += " '" + FormatDateTimeForSql(VarDateTime(varValue), dtoDate)+"', "; \
				} \

//(s.dhole 4/28/2015 10:01 AM ) - PLID 65755  copay ammount
// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
#define APPEND_INSURED_PARTY_COPAY_AMT \
m_strInsurdPartyCoPayFields += " " + strDataField + ","; \
CString strTemp; \
COleCurrency cy; \
if (varValue.vt == VT_CY && VarCurrency(varValue) >= COleCurrency(0, 0)) {	\
		cy = VarCurrency(varValue); \
		RoundCurrency(cy); \
} \
strTemp.Format(" Convert(money,'%s'), ", _Q(FormatCurrencyForSql(cy))); \
m_strInsurdPartyCoPayValues += strTemp;



void CImportWizardPreviewDlg::AppendValue(ImportRecordType irtCurrent, CString strDataField, _variant_t varValue)
{
	try{
		if(IS_FIELD(ifnIgnore) || strDataField.IsEmpty()){
			return;
		}

		long nVarIntValue = 0;
		CString strCustomFieldID;

		//TODO: There has to be a better way to check this

		// (r.farnworth 2015-04-01 16:54) - PLID 65166 - We need to check this first as to not conflict with other PersonT imports
		//Ins Co Fields
		if (irtCurrent == irtInsuranceCos) {
			if (IS_FIELD(ifnHCFAPayerID)
				|| IS_FIELD(ifnUBPayerID)
				|| IS_FIELD(ifnEligibilityPayerID)){
				APPEND_INSURANCECOT_PAYERID;
			}
			else if (IS_FIELD(ifnInsCoName)
				|| IS_FIELD(ifnConversionID)) {
				APPEND_INSURANCECOT_STRING
			}
			else if (IS_FIELD(ifnContactFirst)
				|| IS_FIELD(ifnContactLast)
				|| IS_FIELD(ifnContactTitle)
				|| IS_FIELD(ifnContactFax)
				|| IS_FIELD(ifnContactNote)){
				APPEND_INSURANCECONTACT_STRING;
				return;
			}
			else if (IS_FIELD(ifnContactPhone)) {
				APPEND_INSURANCECONTACT_PHONE;
				return;
			}
		}

		// (b.savon 2015-04-07 10:19) - PLID 65216 - Create fields for the Appointment object for the import utility.
		if (irtCurrent == irtAppointments){
			if (IS_FIELD(ifnAppointmentNotes)){
				// (r.goldschmidt 2016-01-27 18:50) - PLID 67974 - note might be constructed from type/purpose/note, in which case skip now
				if (!IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED)) {
					APPEND_APPT_STRING;
				}
				else {
					m_strAppointmentNoteReplacedValue = VarString(varValue, "");
				}
			}
			else if (IS_FIELD(ifnCustomPatientID)){ // (b.savon 2015-04-08 10:18) - PLID 65217 - Add Patient Mapping ID 
				m_strAppointmentFields += "PatientID, ";
				m_strAuditPersonName = AsString(m_mapPatientIDs[VarString(varValue, "")]); // (b.savon 2015-04-22 14:28) - PLID 65219 - Hijack for auditing
				m_strAppointmentValues += m_strAuditPersonName + ", ";
			}
			else if (IS_FIELD(ifnAppointmentDate)){ // (b.savon 2015-04-07 14:59) - PLID 65218 - Add Appointment Date
				m_strAppointmentDate = FormatDateTimeForSql(VarDateTime(varValue), dtoDate);
				APPEND_APPT_DATE;
			}
			// (r.goldschmidt 2016-01-27 18:50) - PLID 67976 - appt type has option to be forced to conversion and/or added to appt note
			// (r.goldschmidt 2016-03-15 18:12) - PLID 67974 - blank out type in data if prepending to note
			else if (IS_FIELD(ifnAppointmentType) && (!_Q(VarString(varValue, "")).IsEmpty() || IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION_SAVED))){ // (b.savon 2015-04-07 15:24) - PLID 65220 - Add Appointment Type
				if (!IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED) || IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION_SAVED)) {
					m_bAddedApptType = true;
					APPEND_APPT_TYPE;
					m_strAppointmentTypeValues += " (SELECT	ID FROM AptTypeT WHERE Name = '" + _Q(IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION_SAVED) ? GetApptTypeNameConversion() : VarString(varValue, "")) + "') ";
				}
				if (IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED) && !_Q(VarString(varValue, "")).IsEmpty()) {
					m_strAppointmentTypeReplacedValue = VarString(varValue, "");
				}
			}
			else if (IS_FIELD(ifnLocation)){ // (b.savon 2015-04-08 10:56) - PLID 65222 - Add Appointment Location
				strDataField += "ID";
				APPEND_LOCATION_INT;
				if (_Q(VarString(varValue, "")).IsEmpty()){
					CString strLocationID;
					strLocationID.Format("%li", GetCurrentLocationID());
					m_strLocationValues += strLocationID + " ";
				}
				else{
					m_strLocationValues += " (SELECT ID FROM LocationsT WHERE Name = '" + _Q(VarString(varValue, "")) + "') ";
				}
			}
			else if (IS_FIELD(ifnResourceName)){ // (b.savon 2015-04-09 08:10) - PLID 65223 - Add Appointment Resource Name 
				APPEND_APPT_RESOURCE;
			}
			// (r.goldschmidt 2016-01-27 18:50) - PLID 67974 - appt purpose may get added to constructed note instead of data
			// (r.goldschmidt 2016-03-16 08:46) - PLID 67976 - please note, when conversion is checked, validation fails unless prepend to notes is also checked
			else if (IS_FIELD(ifnAppointmentPurpose) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-04-10 11:04) - PLID 65221 - Add Appointment Purpose
				if (!IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED)) {
					APPEND_APPT_PURPOSE;
					m_strAppointmentPurposeValues += " (SELECT	ID FROM AptPurposeT WHERE Name = '" + _Q(VarString(varValue, "")) + "') ";
				}
				else {
					m_strAppointmentPurposeReplacedValue = VarString(varValue, "");
				}
			} 
			else if (IS_FIELD(ifnAppointmentStartTime)){ // (b.savon 2015-04-10 13:29) - PLID 65219 - Add Appointment Start Time
				//Earlier validation checks this, but if its an event, we don't validate the start/end time.  Set to a min date,
				//it wont be added to data.

				// (v.maida 2015-05-05 11:38) - PLID 65743 - It's possible that the cell was converted to VT_BSTR, for formatting purposes, so, if that's the case, 
				// change the value back to a date now
				if (varValue.vt == VT_BSTR) {
					COleDateTime dtUnformatted = ParseDateTime(VarString(varValue, ""));
					varValue.vt = VT_DATE;
					varValue = dtUnformatted;
				}

				if (varValue.vt == VT_NULL || varValue.vt != VT_DATE){
					m_strAppointmentStartTime = FormatDateTimeForSql(COleDateTime(1900, 1, 1, 1, 1, 1), dtoDateTime);
				}
				else{
					m_strAppointmentStartTime = FormatDateTimeForSql(VarDateTime(varValue, COleDateTime(1900, 1, 1, 1, 1, 1)), dtoDateTime);
				}
			}
			else if (IS_FIELD(ifnAppointmentEndTime)){ // (b.savon 2015-04-10 13:29) - PLID 65219 - Add Appointment End Time
				//Earlier validation checks this, but if its an event, we don't validate the start/end time.  Set to a min date,
				//it wont be added to data.

				// (v.maida 2015-05-05 11:38) - PLID 65743 - It's possible that the cell was converted to VT_BSTR, for formatting purposes, so, if that's the case, 
				// change the value back to a date now
				if (varValue.vt == VT_BSTR) {
					COleDateTime dtUnformatted = ParseDateTime(VarString(varValue, ""));
					varValue.vt = VT_DATE;
					varValue = dtUnformatted;
				}

				if (varValue.vt == VT_NULL || varValue.vt != VT_DATE){
					m_strAppointmentEndTime = FormatDateTimeForSql(COleDateTime(1900, 1, 1, 1, 1, 1), dtoDateTime);
			}
				else{
					m_strAppointmentEndTime = FormatDateTimeForSql(VarDateTime(varValue, COleDateTime(1900, 1, 1, 1, 1, 1)), dtoDateTime);
				}
			}
			else if (IS_FIELD(ifnAppointmentDuration)){ // (b.savon 2015-04-10 13:29) - PLID 65219 - Add Appointment Duration
				nVarIntValue = VarLong(varValue, 0);
				if (nVarIntValue > 0){
					CString strInt;
					strInt.Format("%li", nVarIntValue);
					m_strAppointmentDuration = strInt;
				}
			}
			else if (IS_FIELD(ifnAppointmentIsEvent)){ // (b.savon 2015-04-10 13:29) - PLID 65219 - Add Appointment Event
				if (GetSqlValueFromBoolString(_Q(VarString(varValue, "")))){
					m_strAppointmentEvent = "1";
				}
			}
			else if (IS_FIELD(ifnAppointmentIsCancelled)){ // (b.savon 2015-04-14 09:25) - PLID 65216 - Add cancelled
				APPEND_APPT_FIELD;
				if (GetSqlValueFromBoolString(_Q(VarString(varValue, "")))){
					m_strAppointmentValues += "4, ";
				}
				else{
					m_strAppointmentValues += "1, ";
				}
			}
			else if (IS_FIELD(ifnAppointmentIsConfirmed)){ // (b.savon 2015-04-14 09:25) - PLID 65216 - Add confirmed
				APPEND_APPT_FIELD;
				m_strAppointmentValues += " " + AsString(GetSqlValueFromBoolString(_Q(VarString(varValue, "")))) + ", ";
			}
			else if (IS_FIELD(ifnAppointmentIsNoShow)){ // (b.savon 2015-04-14 09:25) - PLID 65216 - Add Show state
				APPEND_APPT_FIELD;
				if (GetSqlValueFromBoolString(_Q(VarString(varValue, "")))){
					m_strAppointmentValues += "3, ";
				}
				else{
					m_strAppointmentValues += "0, ";
				}
			}

			return;
		}

		//Person fields
		if(IS_FIELD(ifnLastName)){
			//(e.lally 2007-07-02) PLID 26503 - Format the last name for auditing
			m_strAuditPersonName = VarString(varValue, "") + ", " + m_strAuditPersonName;
			m_strAuditNewValue = "Last: " + VarString(varValue, "") + ", " + m_strAuditNewValue;

			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnFirstName)){
			//(e.lally 2007-07-02) PLID 26503 - Format the first name for auditing
			m_strAuditPersonName += VarString(varValue, "");
			m_strAuditNewValue += "First: "+ VarString(varValue, "") + ", ";

			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnMiddleName)){
			//(e.lally 2007-07-09) PLID 26503 - Format the middle name for auditing
			m_strAuditPersonName.TrimRight();
			m_strAuditPersonName += " "+ VarString(varValue, "");

			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnPatientCurrentStatus)){
			// (b.savon 2015-03-23 08:54) - PLID 65156 - Add Patients Current Status field 
			//use the preference to tell if it should be patient or prospect
			nVarIntValue = 1;
			
			if (_Q(VarString(varValue, "")).IsEmpty()){
				if (GetRemotePropertyInt("NewPatientDefault", 2, 0, "<None>", true) == 1){
					nVarIntValue = 1;
				}
				else{
					nVarIntValue = 2;
				}
			}
			else{
				if (_Q(VarString(varValue, "")).CompareNoCase("Patient") == 0){
					nVarIntValue = 1;
				}
				else if (_Q(VarString(varValue, "")).CompareNoCase("Prospect") == 0){
					nVarIntValue = 2;
				}
				else if (_Q(VarString(varValue, "")).CompareNoCase("Patient Prospect") == 0){
					nVarIntValue = 3;
				}
				else{
					//This isn't possible.  If we get here, that means we added a new patient status and it isn't
					//handled in the ImportValidationUtils method and it somehow wasn't flagged as invalid.
					ASSERT(FALSE);
				}
			}

			APPEND_PATIENT_INT;
		}
		else if(IS_FIELD(ifnAddress1)){
			APPEND_PERSON_STRING;
		}
		//(e.lally 2010-01-15) PLID 36907 - Here's an idea, let's go ahead and save the Address2!
		else if(IS_FIELD(ifnAddress2)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnCity)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnState)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnZip)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnSocialSecurity)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnHomePhone)){
			APPEND_PERSON_PHONE;
		}
		else if(IS_FIELD(ifnBirthdate)){
			APPEND_PERSON_DATE; 
		}
		else if (IS_FIELD(ifnGender)){
			m_strPersonFields += " " + strDataField +",";
			CString strTemp;
			strTemp.Format(" %li, ", GetGenderValueFromString(VarString(varValue, "")));
			m_strPersonValues += strTemp;
		}
		else if(IS_FIELD(ifnWorkPhone)){
			APPEND_PERSON_PHONE;
		}
		else if(IS_FIELD(ifnWorkExt)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnCellPhone)){
			APPEND_PERSON_PHONE;
		}
		else if(IS_FIELD(ifnPager)){
			APPEND_PERSON_PHONE;
		}
		else if(IS_FIELD(ifnOtherPhone)){
			APPEND_PERSON_PHONE;
		}
		else if(IS_FIELD(ifnFax)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnEmail)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnTitle)){
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnCompany)){
			//(e.lally 2007-07-02) PLID 26503 - Audit the company name for auditing
			m_strAuditPersonName += " - " + VarString(varValue, "");
			m_strAuditNewValue += "Company: " + VarString(varValue, "") + ", ";
			APPEND_PERSON_STRING;
		}
		else if(IS_FIELD(ifnAccount)){
			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnRace) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-03-20 08:45) - PLID 65153 - Add Race
			APPEND_PERSON_RACE_STRING;
			m_strPersonRaceValues += " (SELECT TOP 1 RaceT.ID FROM RaceT LEFT JOIN RaceCodesT ON RaceT.RaceCodeID = RaceCodesT.ID WHERE	RaceT.Name = '" + _Q(VarString(varValue, "")) + "' OR RaceCodesT.CDCID = '" + _Q(VarString(varValue, "")) + "' OR RaceCodesT.HierarchicalCode = '" + _Q(VarString(varValue, "")) + "' OR RaceCodesT.OfficialRaceName = '" + _Q(VarString(varValue, "")) + "' ORDER BY RaceT.RaceCodeID ) ";
		}
		else if (IS_FIELD(ifnEthnicity) && !_Q(VarString(varValue, "")).IsEmpty()){// (b.savon 2015-03-20 15:01) - PLID 65154 - Add Ethnicity
			APPEND_PERSON_ETHNICITY_INT;
			m_strPersonEthnicityValues += " (SELECT	EthnicityT.ID FROM	EthnicityT LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID WHERE EthnicityT.Name = '" + _Q(VarString(varValue, "")) + "' OR EthnicityCodesT.CDCID = '" + _Q(VarString(varValue, "")) + "' OR EthnicityCodesT.HierarchicalCode = '" + _Q(VarString(varValue, "")) + "' OR EthnicityCodesT.OfficialEthnicityName = '" + _Q(VarString(varValue, "")) + "') ";
		} 
		else if (IS_FIELD(ifnLanguage) && !_Q(VarString(varValue, "")).IsEmpty()){// (b.savon 2015-03-23 07:32) - PLID 65155 - Add Language
			APPEND_PERSON_LANGUAGE_INT;
			m_strPersonLanguageValues += " (SELECT LanguageT.ID FROM LanguageT LEFT JOIN LanguageCodesT ON LanguageT.LanguageCodeID = LanguageCodesT.ID WHERE LanguageT.Name = '" + _Q(VarString(varValue, "")) + "' OR LanguageCodesT.OfficialName = '" + _Q(VarString(varValue, "")) + "' OR LanguageCodesT.LanguageCode = '" + _Q(VarString(varValue, "")) + "' OR LanguageCodesT.LanguageCodeAlpha3 = '" + _Q(VarString(varValue, "")) + "') ";
		}
		else if (IS_FIELD(ifnLocation) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-03-23 10:44) - PLID 65157 - Add Location
			APPEND_LOCATION_INT;
			m_strLocationValues += " (SELECT ID FROM LocationsT WHERE Name = '" + _Q(VarString(varValue, "")) + "') ";
		}
		else if (IS_FIELD(ifnReferralSourceName) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-03-23 11:28) - PLID 65158 - Add Referral Source Name
			APPEND_REFERRAL_INT;
			m_strReferralSourceValues += " (SELECT TOP 1 PersonID FROM ReferralSourceT WHERE Name = '" + _Q(VarString(varValue, "")) + "') ";
		}
		else if (IS_FIELD(ifnFirstContactDate)){ // (b.savon 2015-03-23 13:25) - PLID 65159 - Add First Contact Date
			if (varValue.vt == VT_NULL && _Q(VarString(varValue, "")).IsEmpty()){
				varValue = _variant_t(COleDateTime::GetCurrentTime(), VT_DATE);
			}
			APPEND_PERSON_DATE;
		}
		else if (IS_FIELD(ifnMaritalStatus)){
			CString strValue = _Q(VarString(varValue, ""));
			if (varValue.vt != NULL && !strValue.IsEmpty()){
				CString strMaritalCode;
				if (strValue.CompareNoCase("Single") == 0 || strValue.CompareNoCase("S") == 0){
					strMaritalCode = "1";
				}
				else if (strValue.CompareNoCase("Married") == 0 || strValue.CompareNoCase("M") == 0){
					strMaritalCode = "2";
				}
				else if (strValue.CompareNoCase("Other") == 0 || strValue.CompareNoCase("O") == 0){
					strMaritalCode = "3";
				}
				else{
					//This isn't possible.  If we get here, that means we added a new marital status and it isn't
					//handled in the ImportValidationUtils method and it somehow wasn't flagged as invalid.
					ASSERT(FALSE);
				}

				varValue = _variant_t(AsBstr(strMaritalCode));
			}

			APPEND_PATIENT_STRING;
		}
		else if (IS_FIELD(ifnEmergencyContactFirstName)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Emerg First
			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnEmergencyContactLastName)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Emerg Last
			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnEmergencyContactRelation)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Emerg Relation
			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnEmergencyContactHomePhone)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Emerg HPhone
			APPEND_PERSON_PHONE;
		}
		else if (IS_FIELD(ifnEmergencyContactWorkPhone)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Emerg WPhone
			APPEND_PERSON_PHONE;
		}
		else if (IS_FIELD(ifnWarningMessage)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Warning Msg
			if (!_Q(VarString(varValue, "")).IsEmpty()){ // If we have a warning message, set the flag to display it
				m_strPersonFields += " DisplayWarning, "; \
					m_strPersonValues += " 1, "; \
			}
			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnNote)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Notes
			APPEND_PERSON_STRING;
		}
		else if (IS_FIELD(ifnGen1Custom1)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Gen1 Custom1
			strCustomFieldID = "1";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnGen1Custom2)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Gen1 Custom2
			strCustomFieldID = "2";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnGen1Custom3)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Gen1 Custom3
			strCustomFieldID = "3";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnGen1Custom4)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Gen1 Custom4
			strCustomFieldID = "4";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText1)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText1
			strCustomFieldID = "11";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText2)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText2
			strCustomFieldID = "12";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText3)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText3
			strCustomFieldID = "13";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText4)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText4
			strCustomFieldID = "14";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText5)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText5
			strCustomFieldID = "15";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText6)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText6
			strCustomFieldID = "16";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomNote)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add Custom Note
			strCustomFieldID = "17";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText7)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText7
			strCustomFieldID = "90";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText8)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText8
			strCustomFieldID = "91";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText9)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText9
			strCustomFieldID = "92";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText10)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText10
			strCustomFieldID = "93";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText11)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText11
			strCustomFieldID = "94";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomText12)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomText12
			strCustomFieldID = "95";
			APPEND_CUSTOM_TEXT;
		}
		else if (IS_FIELD(ifnCustomCheckbox1)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomCheckbox1
			strCustomFieldID = "41";
			APPEND_CUSTOM_CHECKBOX;
		}
		else if (IS_FIELD(ifnCustomCheckbox2)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomCheckbox2
			strCustomFieldID = "42";
			APPEND_CUSTOM_CHECKBOX;
		}
		else if (IS_FIELD(ifnCustomCheckbox3)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomCheckbox3
			strCustomFieldID = "43";
			APPEND_CUSTOM_CHECKBOX;
		}
		else if (IS_FIELD(ifnCustomCheckbox4)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomCheckbox4
			strCustomFieldID = "44";
			APPEND_CUSTOM_CHECKBOX;
		}
		else if (IS_FIELD(ifnCustomCheckbox5)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomCheckbox5
			strCustomFieldID = "45";
			APPEND_CUSTOM_CHECKBOX;
		}
		else if (IS_FIELD(ifnCustomCheckbox6)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomCheckbox6
			strCustomFieldID = "46";
			APPEND_CUSTOM_CHECKBOX;
		}
		else if (IS_FIELD(ifnCustomDate1)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomDate1
			strCustomFieldID = "51";
			APPEND_CUSTOM_DATE;
		}
		else if (IS_FIELD(ifnCustomDate2)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomDate2
			strCustomFieldID = "52";
			APPEND_CUSTOM_DATE;
		}
		else if (IS_FIELD(ifnCustomDate3)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomDate3
			strCustomFieldID = "53";
			APPEND_CUSTOM_DATE;
		}
		else if (IS_FIELD(ifnCustomDate4)){ // (b.savon 2015-03-25 07:02) - PLID 65144 - Add CustomDate4
			strCustomFieldID = "54";
			APPEND_CUSTOM_DATE;
		}
		else if (IS_FIELD(ifnProviderName) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-03-25 13:26) - PLID 65150 - Add ProviderID
			APPEND_PATIENT_PROVIDER_STRING;
			m_strPatientProviderValues += " (SELECT	PersonID FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE (LTRIM(RTRIM(PersonT.First)) + ' ' + LTRIM(RTRIM(PersonT.Last)) = '" + _Q(VarString(varValue, "")) + "') OR (LTRIM(RTRIM(PersonT.Last)) + ', ' + LTRIM(RTRIM(PersonT.First)) = '" + _Q(VarString(varValue, "")) + "')) ";
		}
		else if (IS_FIELD(ifnReferringPhysicianName) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-03-26 13:26) - PLID 65151 - Add Referring Physician ID
			APPEND_PATIENT_REFPHYS_STRING;
			m_strPatientReferringPhysValues += " (SELECT PersonID FROM ReferringPhysT INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE (LTRIM(RTRIM(PersonT.First)) + ' ' + LTRIM(RTRIM(PersonT.Last)) = '" + _Q(VarString(varValue, "")) + "') OR (LTRIM(RTRIM(PersonT.Last)) + ', ' + LTRIM(RTRIM(PersonT.First)) = '" + _Q(VarString(varValue, "")) + "')) ";
		}
		else if (IS_FIELD(ifnPrimaryCarePhysicianName) && !_Q(VarString(varValue, "")).IsEmpty()){ // (b.savon 2015-03-26 14:23) - PLID 65152 - Add PCP Name 
			APPEND_PATIENT_PCP_STRING;
			m_strPatientPCPValues += " (SELECT PersonID FROM ReferringPhysT INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE (LTRIM(RTRIM(PersonT.First)) + ' ' + LTRIM(RTRIM(PersonT.Last)) = '" + _Q(VarString(varValue, "")) + "') OR (LTRIM(RTRIM(PersonT.Last)) + ', ' + LTRIM(RTRIM(PersonT.First)) = '" + _Q(VarString(varValue, "")) + "')) ";
		}

		//Patient fields or MediNotes patient fields
		//(e.lally 2007-06-29) PLID 26509 - MediNotes patients
		if(irtCurrent == irtPatients || irtCurrent == irtMediNotes){
			if(IS_FIELD(ifnPatientID) && VarLong(varValue, 0) > 0){ 
				// (v.maida 2015-04-07 09:08) - PLID 65440 - If the patient ID is a valid value, use that, otherwise auto-generate a new ID later on, within the SaveImportedData() function.
				m_bHasPatientID = TRUE;
				m_strPatientFields += " " + strDataField +",";
				CString strValue;
				strValue.Format("%li", VarLong(varValue));
				m_strPatientValues += " " + strValue +",";
			}
			//No need to check the rest of the record types, return
			return;
		}

		//Provider fields
		//(e.lally 2007-06-08) PLID 26262 - Import list of providers
		if(irtCurrent == irtProviders){
			if(IS_FIELD(ifnProviderNPI)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderFederalEmpNumber)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderWorkersCompNumber)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderMedicaidNumber)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderLicenseNumber)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderBCBSNumber)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderTaxonomyCode)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderUPIN)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderMedicare)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderOtherID)){
				APPEND_PROVIDER_STRING;
			}
			else if(IS_FIELD(ifnProviderDEANumber)){
				APPEND_PROVIDER_STRING;
			}
			//No need to check the rest of the record types, return
			return;
		}

		//Referring Physician fields
		//(e.lally 2007-06-11) PLID 10320 - Import list of referring physicians
		if(irtCurrent == irtReferringPhysicians){
			if(IS_FIELD(ifnRefPhysNPI)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysFederalEmpNumber)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysWorkersCompNumber)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysMedicaidNumber)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysLicenseNumber)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysBCBSNumber)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysTaxonomyCode)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysUPIN)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysMedicare)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysOtherID)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysDEANumber)){
				APPEND_REF_PHYS_STRING;
			}
			else if(IS_FIELD(ifnRefPhysID)){
				APPEND_REF_PHYS_STRING;
			}
			//No need to check the rest of the record types, return
			return;
		}

		//User fields
		//(e.lally 2007-06-12) PLID 26273 - Import list of user
		if(irtCurrent == irtUsers){
			if(IS_FIELD(ifnUserUsername)){
				//(e.lally 2007-07-02) PLID 26503 - Format the username for auditing
				m_strAuditNewValue += "Username: " + VarString(varValue, "") + ", ";
				//The username should already be trimmed, just like the contacts module does.
				APPEND_USER_STRING;
			}
			else if(IS_FIELD(ifnUserNationalEmpNum)){
				APPEND_USER_STRING;
			}
			else if(IS_FIELD(ifnUserDateOfHire)){
				APPEND_USER_DATE;
			}
			else if(IS_FIELD(ifnUserPatientCoord)){
				m_strUserFields += " " + strDataField +",";
				CString strTemp;
				strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, "")));
				m_strUserValues += strTemp;
			}

			//No need to check the rest of the record types, return
			return;
		}

		//Supplier fields
		//(e.lally 2007-06-11) PLID 26274 - Import list of suppliers
		if(irtCurrent == irtSuppliers){
			if(IS_FIELD(ifnSupplierPayMethod)){
				APPEND_SUPPLIER_STRING;
			}
			
			//No need to check the rest of the record types, return
			return;
		}

		//Other Contact fields
		//(e.lally 2007-06-12) PLID 26275 - Import list of Other Contacts
		if(irtCurrent == irtOtherContacts){
			if(IS_FIELD(ifnOtherContactNurse)){
				m_strOtherContactFields += " " + strDataField +",";
				CString strTemp;
				strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, "")));
				m_strOtherContactValues += strTemp;
			}
			else if(IS_FIELD(ifnOtherContactAnesthesiologist)){
				m_strOtherContactFields += " " + strDataField +",";
				CString strTemp;
				strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, "")));
				m_strOtherContactValues += strTemp;
			}
			
			//No need to check the rest of the record types, return
			return;
		}

		// (j.jones 2010-03-30 08:52) - PLID 16717 - added support for service codes
		if(irtCurrent == irtServiceCodes) {
			if(IS_FIELD(ifnServiceName)) {
				APPEND_SERVICE_STRING;
			}
			else if(IS_FIELD(ifnServicePrice)) {
				m_strServiceFields += " " + strDataField +",";
				CString strTemp;
				COleCurrency cy;
				if(varValue.vt == VT_CY && VarCurrency(varValue) >= COleCurrency(0,0)) {
					cy = VarCurrency(varValue);
					// (b.eyers 2015-11-06) - PLID 34061 - round this before saving
					RoundCurrency(cy);
				}
				strTemp.Format(" Convert(money,'%s'), ", _Q(FormatCurrencyForSql(cy)));
				m_strServiceValues += strTemp;
			}
			else if(IS_FIELD(ifnTaxable1)) {
				m_strServiceFields += " " + strDataField +",";
				CString strTemp;
				strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, "")));
				m_strServiceValues += strTemp;
			}
			else if(IS_FIELD(ifnTaxable2)) {
				m_strServiceFields += " " + strDataField +",";
				CString strTemp;
				strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, "")));
				m_strServiceValues += strTemp;
			}
			else if(IS_FIELD(ifnBarcode)) {
				APPEND_SERVICE_STRING;
			}
			else if(IS_FIELD(ifnServiceCode)) {
				APPEND_CPT_CODE_STRING;
				if(!m_strAuditNewValue.IsEmpty()) {
					m_strAuditNewValue = VarString(varValue, "") + " " + m_strAuditNewValue;
				}
				else {
					m_strAuditNewValue = VarString(varValue, "");
				}
			}
			else if(IS_FIELD(ifnServiceSubCode)) {
				// (j.jones 2010-04-05 11:27) - PLID 16717 - we won't need to forcibly fill in the SubCode
				m_bHasSubCode = TRUE;
				APPEND_CPT_CODE_STRING;

				if(!m_strAuditNewValue.IsEmpty()) {
					m_strAuditNewValue += " " + VarString(varValue, "");
				}
				else {
					m_strAuditNewValue = VarString(varValue, "");
				}
			}
			else if(IS_FIELD(ifnRVU)) {
				m_strCPTCodeFields += " " + strDataField +",";
				if(varValue.vt == VT_R8 && VarDouble(varValue) > 0) {
					CString strValue;
					strValue.Format("%g", VarDouble(varValue));
					m_strCPTCodeValues += " " + strValue +",";
				}
				else{
					m_strCPTCodeValues += " 0.0, ";
				}
			}
			else if(IS_FIELD(ifnGlobalPeriod)) {
				m_strCPTCodeFields += " " + strDataField +",";
				//if zero, import as NULL
				if(varValue.vt == VT_I4 && VarLong(varValue) > 0) {
					CString strValue;
					strValue.Format("%li", VarLong(varValue));
					m_strCPTCodeValues += " " + strValue +",";
				}
				else{
					m_strCPTCodeValues += " NULL, ";
				}
			}
			// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
			/*else if(IS_FIELD(ifnPromptForCopay)) {
			m_strCPTCodeFields += " " + strDataField +",";
			CString strTemp;
			strTemp.Format(" %li, ", GetSqlValueFromBoolString(VarString(varValue, "")));
			m_strCPTCodeValues += strTemp;
			}*/
			return;
		}

		// (r.farnworth 2015-03-16 15:40) - PLID 65197 - Add a new import type, Resources, to the import utility
		if (irtCurrent == irtResources) {
			if(IS_FIELD(ifnResourceName)){
				APPEND_RESOURCE_STRING;
			}
			return;
		}
		// (r.farnworth 2015-03-19 11:10) - PLID 65238 - Add a new import type, Products, to the't import utility
		if (irtCurrent == irtProducts) {
			if (IS_FIELD(ifnProductName)){
				APPEND_PRODUCT_SERVICET_STRING;
				m_strAuditNewValue = VarString(varValue, "");
			}
			else if (IS_FIELD(ifnProductDescription)){
				APPEND_PRODUCT_PRODUCTT_STRING;
			}
			else if (IS_FIELD(ifnProductPrice)){
				APPEND_PRODUCT_SERVICET_MONEY;
			}
			else if (IS_FIELD(ifnProductLastCost)){
				APPEND_PRODUCT_PRODUCTT_MONEY
			}
			else if (IS_FIELD(ifnProductTaxable1)){
				APPEND_PRODUCT_SERVICET_BIT;
			}
			else if (IS_FIELD(ifnProductTaxable2)){
				APPEND_PRODUCT_SERVICET_BIT;
			}
			else if (IS_FIELD(ifnProductBarcode)){
				APPEND_PRODUCT_SERVICET_STRING;
			}
			else if (IS_FIELD(ifnProductBillable)){
				APPEND_PRODUCT_PRODUCTLOCATIONINFOT_BIT;
			}
			else if (IS_FIELD(ifnProductOnHand)){
				APPEND_PRODUCT_PRODUCTADJUSTMENTS_LONG;
				// (r.farnworth 2015-03-20 14:03) - PLID 65244
				m_nProductOnHandAmt = VarLong(varValue, 0);
			}
			return;
		}

		// (b.savon 2015-04-01 11:42) - PLID 65235 - Add Recall 
		if (irtCurrent == irtRecalls){
			if (IS_FIELD(ifnRecallDate)){
				APPEND_RECALL_DATE;
			}
			else if (IS_FIELD(ifnRecallTemplateName)){ // (b.savon 2015-04-02 09:32) - PLID 65236 - Add Template
				APPEND_RECALL_TEMPLATE;
				m_strPatientRecallTemplateValues += " (SELECT ID FROM RecallTemplateT WHERE Name = '" + _Q(VarString(varValue, "")) + "') ";
				m_strAuditNewValue = _Q(VarString(varValue, "")); // (b.savon 2015-05-01 09:11) - PLID 65236 - Save for auditing
			}
			else if (IS_FIELD(ifnCustomPatientID)){ // (b.savon 2015-04-02 10:47) - PLID 65234 - Add Patient Mapping ID
				m_strPatientRecallFields += "PatientID, ";
				m_strAuditPersonName = AsString(m_mapPatientIDs[VarString(varValue, "")]); // (b.savon 2015-04-22 14:28) - PLID 65219 - Hijack for auditing
				m_strPatientRecallValues += m_strAuditPersonName + ", "; // (b.savon 2015-05-01 09:11) - PLID 65236 - save for auditing
			}
		}

		//(s.dhole 4/7/2015 3:39 PM ) - PLID 65225
		if (irtCurrent == irtPatientNotes){
			
			if (IS_FIELD(ifnPatientNoteText)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65230
				APPEND_PATIENT_NOTE_STRING;
			}
			else if (IS_FIELD(ifnPatientNoteDateTime)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65227
				APPEND_PATIENT_NOTE_DATE;
			}
			else if (IS_FIELD(ifnPatientNoteCategory)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65228
				APPEND_NOTE_CATEGORY;
				m_strPatientNoteCategoryValues += " (SELECT ID FROM NoteCatsF WHERE Description = '" + _Q(VarString(varValue, "")) + "') ";
			}
			else if (IS_FIELD(ifnPatientNotePriority)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65229
				APPEND_NOTE_PRIORITY;
				long nDefaultPriority = GetRemotePropertyInt("MyDefaultNotesPriority", 3 /*PRIORITY_LOW*/, 0, GetCurrentUserName(), TRUE);
				//m_strPatientNotesFields += "Priority, ";
				long nProirity = GetNotePriorityFromString(VarString(varValue, ""));
				m_strPatientNotePriorityValues += FormatString(" (SELECT  CASE WHEN %li = 0 THEN  %li ELSE  %li END ) AS Priority, ", nProirity, nDefaultPriority, nProirity);
			}
			else if (IS_FIELD(ifnCustomPatientID)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65226
				m_strPatientIDFields += "PersonID, ";
				m_strPatientIDValues += AsString(m_mapPatientIDs[VarString(varValue, "")])  ;
			}
		}
		//(s.dhole 4/13/2015 3:07 PM ) - PLID 65190   - Add a new import type, Insure Party, to the import utility
		if (irtCurrent == irtInsuredParties){
			if (IS_FIELD(ifnInsuranceCompanyConversionID) && !VarString(varValue, "").IsEmpty()){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65193
				
				m_strInsurdPartyInsuConvIDValues += " (SELECT PersonID FROM InsuranceCoT WHERE ConversionID = '" + _Q(VarString(varValue, "")) + "') ";
				m_strInsurdPartyInsuConvId  += "Insurance Conversion ID: " + VarString(varValue, "");
			}
			else if (IS_FIELD(ifnInsuranceCompanyName)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65193
				
				m_strInsurdPartyInsuCoNameValues += " (SELECT PersonID FROM InsuranceCoT WHERE Name = '" + _Q(VarString(varValue, "")) + "') "; 
				m_strInsurdPartyInsuCoName += "Insurance Name: " + VarString(varValue, "");
			}
			else if (IS_FIELD(ifnCustomPatientID)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65226
				m_strInsurdPartyFields += "PatientID, ";
				m_strInsurdPartyValues += AsString(m_mapPatientIDs[VarString(varValue, "")]) + ", ";
				m_strPatientIDValues += AsString(m_mapPatientIDs[VarString(varValue, "")]);
			}
			else if (IS_FIELD(ifnInsuredEmployer)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65755
				m_strInsurdPartyFields += " " + strDataField + ",";
				m_strInsurdPartyValues += "'" + _Q(VarString(varValue, "")) + "', ";
			}
			else if (IS_FIELD(ifnInsuredInsuranceID)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65755
				m_strInsurdPartyFields += " " + strDataField + ",";
				m_strInsurdPartyValues += "'" + _Q(VarString(varValue, "")) + "', ";
			}
			else if (IS_FIELD(ifnInsuredGroupNo)){
				//(s.dhole 5/6/2015 10:55 AM ) - PLID 65755
				m_strInsurdPartyFields += " " + strDataField + ",";
				m_strInsurdPartyValues += "'" + _Q(VarString(varValue, "")) + "', ";
			}
			else if (IS_FIELD(ifnInsuredCopay)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65755
				APPEND_INSURED_PARTY_COPAY_AMT
			}
			else if (IS_FIELD(ifnInsuredCopayPercent)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65755
				m_strInsurdPartyCoPayFields += " " + strDataField + ",";
				CString strTemp;
				if (VarLong(varValue, 0) == 0)
				{
					strTemp = "NULL, ";
				}
				else 
				{
					strTemp.Format(" %li, ", VarLong(varValue, 0));
				}
				m_strInsurdPartyCoPayValues += strTemp ;
			}
			else if (IS_FIELD(ifnInsuredEffectiveDate)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65755
				APPEND_INSURED_PARTY_EFFECTIVE_DATE
			}
			else if (IS_FIELD(ifnInsuredInactiveDate)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65755
				APPEND_INSURED_PARTY_EFFECTIVE_DATE
			}
			else if (IS_FIELD(ifnInsuredPartyRespTypeID)){
				//(s.dhole 4/28/2015 10:20 AM ) - PLID 65195  Insured Reposiblity type
				CString sTemp = VarString(varValue, "");
				if (!sTemp.IsEmpty())
				{
					m_strInsurdRespoTypeValue += "(SELECT ID FROM RespTypeT WHERE Priority =" + VarString(varValue, "") + ") ";
				}
			}

			else if (IS_FIELD(ifnInsuredPartyRelation)){
				//(s.dhole 4/8/2015 1:43 PM ) - PLID 65196
				m_strInsurdPartyFields += " " + strDataField + ",";
				m_strInsurdPartyValues += "'" + _Q(VarString(varValue, "")) + "', ";
			}
		}
		// (r.goldschmidt 2016-02-10 11:08) - PLID 68163 - Add a new import type, Race, to the import utility
		if (irtCurrent == irtRaces) {
			if (IS_FIELD(ifnRaceCDCCode) && !VarString(varValue, "").IsEmpty()) {
				m_bPairedToRaceCDCCode = true;
				m_strRaceCDCCodeValue = VarString(varValue, "");
			}
			else if (IS_FIELD(ifnRacePreferredName) && !VarString(varValue, "").IsEmpty()) {
				m_bHasRacePreferredName = true;
				m_strRacePreferredNameValue = _Q(VarString(varValue, ""));
				m_strAuditNewValue = VarString(varValue, "");
			}
		}
	}NxCatchAll("AppendValue: Error generating proper cell value");
}

void CImportWizardPreviewDlg::OnImportShowIgnored() 
{
	//(e.lally 2007-06-07) PLID 26250 - Add ability to show/hide ignored fields
	try{
		if(IsDlgButtonChecked(IDC_IMPORT_SHOW_IGNORED))
			ShowIgnoredFields(TRUE);
		else
			ShowIgnoredFields(FALSE);

	}NxCatchAll("Error showing or hiding ignored fields");
}

void CImportWizardPreviewDlg::ShowIgnoredFields(BOOL bShow)
{
	//(e.lally 2007-06-07) PLID 26250 - Add ability to show/hide ignored fields
	IColumnSettingsPtr pCol;
	for(int i=0, nCount = m_pPreviewList->GetColumnCount(); i<nCount; i++){
		pCol = m_pPreviewList->GetColumn(i);
		if(pCol){
			//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
			if(CString((LPCTSTR)pCol->GetColumnTitle()) == GetImportFieldHeader(ifnIgnore)){
				if(bShow == FALSE){
					//hide this column
					pCol->PutStoredWidth(0);
				}
				else{
					//show this column
					pCol->PutStoredWidth(100);
				}
			}//end if ignore column
		}//end if pCol
	}//end for loop

}

// (b.savon 2015-03-24 12:34) - PLID 65251 - Create a function to designate which columns are "fields" in the import so that the capsfix algorithm is only run on desired columns
bool CImportWizardPreviewDlg::ShouldFixCaps(const CString &strField)
{
	return (m_chkPerformCapsFix.GetCheck() == BST_CHECKED && IsEligibleCapsFixField(strField));
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardPreviewDlg::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);

	if (m_bNeedInit) {
		return;
	}

	//Get the delta, the difference in size
	int dx = cx - m_ClientSize.cx;
	int dy = cy - m_ClientSize.cy;

	//Rember the new size
	m_ClientSize.cx = cx;
	m_ClientSize.cy = cy;

	try
	{
		//This scope is meant to keep the static text at the top centered to the dialog.
		{
			CRect itemRect;
			CWnd *item = GetDlgItem(IDC_STATIC);
			ASSERT(item->GetSafeHwnd());
			if (item->GetSafeHwnd()) {
				item->GetWindowRect(&itemRect);
				ScreenToClient(&itemRect);
				int newX = (cx - itemRect.Width()) / 2;
				item->SetWindowPos(NULL, newX, itemRect.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
			}
		}

		//Change the positions of each of the items, according to their anchors
		ChangeDlgItemPos(this, IDC_IMPORT_PREVIEW_TABLE, dx, dy, EChangeDlgItemPosAnchor::LeftTopRightBottom);
		ChangeDlgItemPos(this, IDC_IMPORT_SHOW_IGNORED, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
		ChangeDlgItemPos(this, IDC_CHK_IMPORT_PERFORM_BACKUP, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
		ChangeDlgItemPos(this, IDC_CHK_APPLY_CAPSFIX, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
		ChangeDlgItemPos(this, IDC_APPT_TYPES_AS_CONVERSION_SAVED, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
		ChangeDlgItemPos(this, IDC_APPT_TYPE_TO_NOTES_SAVED, dx, dy, EChangeDlgItemPosAnchor::BottomLeft);
	}NxCatchAllIgnore();
}

// (b.savon 2015-04-30 16:02) - PLID 65511 - Break out into a new function
void CImportWizardPreviewDlg::AddImportDeclareVariablesToBatch(CString &strSqlBatch)
{
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @PersonID INT \r\n");
	// (j.jones 2010-04-05 11:05) - PLID 16717 - supported service codes
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ServiceID INT \r\n");
	// (b.cardillo 2009-05-28 14:55) - PLID 34369 - We need to collect the list of the PersonIDs of all patients that we add
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @tPatientIDs TABLE (PersonID INT) \r\n");
	// (r.farnworth 2015-03-16 15:23) - PLID 65197 - Add a new import type, Resources, to the import utility
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ResourceID INT \r\n");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @SecurityObjectID INT \r\n");
	// (b.savon 2015-03-20 13:51) - PLID 65153 - Save Person Race
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @RaceID INT \r\n");
	// (r.farnworth 2015-03-19 10:07) - PLID 65238 - Add a new import type, Products, to the't import utility
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ProductID INT \r\n");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @LocationID INT \r\n");
	// (b.savon 2015-03-20 15:01) - PLID 65154 - Add Ethnicity
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @EthnicityID INT \r\n");
	// (b.savon 2015-03-23 07:32) - PLID 65155 - Add Language
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @LanguageID INT \r\n");
	// (v.maida 2015-04-06 11:01) - PLID 65440 - Use a userdefinedID variable set to a subquery when importing a patient, rather than directly using a subquery.
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @UserDefinedID INT \r\n");
	// (b.savon 2015-03-23 11:28) - PLID 65158 - Add Referral Source 
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ReferralSourceID INT \r\n");
	// (b.savon 2015-03-25 13:26) - PLID 65150 - Add ProviderID
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ProviderID INT \r\n");
	// (b.savon 2015-03-26 13:26) - PLID 65151 - Add Referring Physician ID
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @RefPhysID INT \r\n");
	// (b.savon 2015-03-26 14:23) - PLID 65152 - Add PCP Name 
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @PCPID INT \r\n");
	// (b.savon 2015-04-02 09:23) - PLID 65236 - Add Recal Template 
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @RecallTemplateID INT \r\n");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @SetID INT \r\n");
	// (r.farnworth 2015-04-01 16:15) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @InsuranceContactID INT \r\n");

	//(s.dhole 4/8/2015 8:46 AM ) - PLID 65230 support note categorey and new note id
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @PatientNoteCategoryID INT \r\n");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewPatientNoteID INT \r\n");

	// (b.savon 2015-04-07 15:24) - PLID 65220 - Add validation for Appointment Type
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @AppointmentTypeID INT \r\n");
	// (b.savon 2015-04-09 08:10) - PLID 65223 - Add Appointment Resource Name
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @AppointmentID INT \r\n");
	// (b.savon 2015-04-10 11:01) - PLID 65221 - Add validation for Appointment Purpose
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @AppointmentPurposeID INT \r\n");
	//(s.dhole 4/28/2015 10:20 AM ) - PLID 65195  Insured Reposiblity type
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @InsuRespoPartyID INT \r\n");

	// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
	AddDeclarationToSqlBatch(strSqlBatch,
		"DECLARE @SecurityGroupID INT\r\n"
		"SET @SecurityGroupID = (\r\n"
		"	SELECT IntParam\r\n"
		"	FROM ConfigRT\r\n"
		"	INNER JOIN SecurityGroupsT ON SecurityGroupsT.ID = ConfigRT.IntParam\r\n"
		"	WHERE ConfigRT.Username = '<None>' AND ConfigRT.Name = 'DefaultSecurityGroup')\r\n");
}

// (b.savon 2015-04-30 16:02) - PLID 65511 - Set all the fields and values back to empty for the new row
void CImportWizardPreviewDlg::ResetFieldsAndValues()
{
	m_bHasPatientID = FALSE;
	// (j.jones 2010-04-05 11:27) - PLID 16717 - we will fill in the SubCode if the user does not
	m_bHasSubCode = FALSE;

	//Empty any existing inserts
	m_strPersonFields = "";
	m_strPersonValues = "";
	m_strPatientFields = "";
	m_strPatientValues = "";
	//(e.lally 2007-06-08) PLID 26262 - clear out provider strings
	m_strProviderFields = "";
	m_strProviderValues = "";
	//(e.lally 2007-06-11) PLID 10320 - clear out ref phys strings
	m_strRefPhysFields = "";
	m_strRefPhysValues = "";
	//(e.lally 2007-06-11) PLID 26274 - clear out supplier strings
	m_strSupplierFields = "";
	m_strSupplierValues = "";
	//(e.lally 2007-06-12) PLID 26274 - clear out user strings
	m_strUserFields = "";
	m_strUserValues = "";
	//(e.lally 2007-06-12) PLID 26275 - clear out other contact strings
	m_strOtherContactFields = "";
	m_strOtherContactValues = "";
	// (j.jones 2010-04-05 11:20) - PLID 16717 - clear service strings
	m_strServiceFields = "";
	m_strServiceValues = "";
	m_strCPTCodeFields = "";
	m_strCPTCodeValues = "";

	//(e.lally 2007-07-02) PLID 26503 - Clear out the audit strings
	m_strAuditPersonName = "";
	m_strAuditNewValue = "";
	// (r.farnworth 2015-03-16 14:55) - PLID 65197 - Add a new import type, Resources, to the import utility
	m_strResourceFields = "";
	m_strResourceValues = "";

	// (r.farnworth 2015-03-19 10:09) - PLID 65238 - Add a new import type, Products, to the't import utility
	m_strProductServiceTFields = "";
	m_strProductServiceTValues = "";
	m_strProductProductTFields = "";
	m_strProductProductTValues = "";
	m_strProductProductLocationInfoTFields = "";
	m_strProductProductLocationInfoTValues = "";
	m_strProductProductAdjustmentsTFields = "";
	m_strProductProductAdjustmentsTValues = "";

	// (b.savon 2015-03-20 08:45) - PLID 65153 - Add Race
	m_strPersonRaceFields = "";
	m_strPersonRaceValues = "";
	// (b.savon 2015-03-20 15:01) - PLID 65154 - Add Ethnicity
	m_strPersonEthnicityFields = "";
	m_strPersonEthnicityValues = "";
	// (b.savon 2015-03-23 07:32) - PLID 65155 - Add Language
	m_strPersonLanguageFields = "";
	m_strPersonLanguageValues = "";
	// (b.savon 2015-03-23 10:41) - PLID 65157 - Add Location
	m_strLocationFields = "";
	m_strLocationValues = "";
	// (b.savon 2015-03-23 11:28) - PLID 65158 - Add Referral Source Name
	m_strReferralSourceFields = "";
	m_strReferralSourceValues = "";
	// (b.savon 2015-03-25 07:46) - PLID 65144 - Add Custom Fields
	m_strCustomFieldsQuery = "";
	// (b.savon 2015-03-25 13:26) - PLID 65150 - Add ProviderID
	m_strPatientProviderFields = "";
	m_strPatientProviderValues = "";
	// (b.savon 2015-03-26 13:26) - PLID 65151 - Add Referring Physician ID
	m_strPatientReferringPhysFields = "";
	m_strPatientReferringPhysValues = "";
	// (b.savon 2015-03-26 14:23) - PLID 65152 - Add PCP Name 
	m_strPatientPCPFields = "";
	m_strPatientPCPValues = "";
	// (b.savon 2015-04-01 11:47) - PLID 65235 - Add Recall date
	m_strPatientRecallFields = "";
	m_strPatientRecallValues = "";
	// (b.savon 2015-04-02 09:23) - PLID 65236 - Add Recall Template Name
	m_strPatientRecallTemplateFields = "";
	m_strPatientRecallTemplateValues = "";
	// (r.farnworth 2015-04-01 16:29) - PLID 65166 - Add new import type, Insurance Companies, to the import utility
	m_strInsuranceCoTFields = "";
	m_strInsuranceCoTValues = "";
	m_strInsuranceContactFields = "";
	m_strInsuranceContactValues = "";

	//(s.dhole 4/7/2015 10:30 AM ) - PLID 65224
	m_strPatientNotesFields = "";
	m_strPatientNotesValues = "";
	//(s.dhole 4/8/2015 7:59 AM ) - PLID 65228
	m_strPatientNoteCategoryFields = m_strPatientNoteCategoryValues = "";
	//(s.dhole 4/8/2015 7:59 AM ) - PLID 65226
	m_strPatientNoteIDFields = m_strPatientNoteIDValues = "";
	//(s.dhole 4/8/2015 7:59 AM ) - PLID 65228 
	m_strPatientNotePriorityFields = m_strPatientNotePriorityValues = "";
	m_strPatientNotesTextFields = m_strPatientNotesTextValues = "";
	m_strPatientIDFields = m_strPatientIDValues = "";

	// (b.savon 2015-04-07 10:19) - PLID 65216 - Create fields for the Appointment object for the import utility.
	m_strAppointmentFields = "";
	m_strAppointmentValues = "";
	// (b.savon 2015-04-07 15:24) - PLID 65220 - Add Appointment Type
	m_strAppointmentTypeFields = "";
	m_strAppointmentTypeValues = "";
	// (b.savon 2015-04-10 11:01) - PLID 65221 - Add Appointment Purpose
	m_strAppointmentPurposeFields = "";
	m_strAppointmentPurposeValues = "";
	// (r.goldschmidt 2016-01-28 15:32) - PLID 67976 - add appt type/purpose options
	m_strAppointmentTypeReplacedValue = "";
	m_strAppointmentPurposeReplacedValue = "";
	m_strAppointmentNoteReplacedValue = "";
	m_bAddedApptType = false;

	//(s.dhole 4/8/2015 7:59 AM ) - PLID 65190 
	m_strInsurdPartyFields, m_strInsurdPartyValues = "";
	//(s.dhole 4/28/2015 9:52 AM ) - PLID 65193 Insure Comp name or conversion id
	m_strInsurdPartyInsuCoNameValues = m_strInsurdPartyInsuConvIDValues = "";
	m_strInsurdPartyInsuCoName = m_strInsurdPartyInsuConvId = "";
	//(s.dhole 4/28/2015 9:52 AM ) - PLID 65190 copay
	m_strInsurdPartyCoPayFields = m_strInsurdPartyCoPayValues = "";
	//(s.dhole 4/28/2015 10:09 AM ) - PLID 65195 Repo Party
	m_strInsurdRespoTypeValue = "";

	// (b.savon 2015-04-13 08:13) - PLID 65219 - Store appointment data for group dependency insertion
	m_strAppointmentDate = "";
	m_strAppointmentStartTime = "";
	m_strAppointmentEndTime = "";
	m_strAppointmentDuration = "";
	m_strAppointmentEvent = "";
	m_strAuditAppointmentValues = ""; // (b.savon 2015-04-22 14:08) - PLID 65219

	// (r.goldschmidt 2016-02-10 12:30) - PLID 68163 - store race data
	m_bPairedToRaceCDCCode = false;
	m_strRaceCDCCodeValue = "";
	m_bHasRacePreferredName = false;
	m_strRacePreferredNameValue = "";

}

// (b.savon 2015-04-30 16:22) - PLID 65511 - Split out the batch execution logic
void CImportWizardPreviewDlg::RunBatch(long nAuditTransactionID, const CString &strSqlBatch)
{
	ADODB::_RecordsetPtr prsPatientIDs = CreateRecordsetStd(
		"SET NOCOUNT ON \r\n" +
		strSqlBatch +
		"SET NOCOUNT OFF \r\n"
		"SELECT PersonID FROM @tPatientIDs \r\n");

	//Commit our audit transaction
	if (nAuditTransactionID != -1)
		CommitAuditTransaction(nAuditTransactionID);

	// (b.cardillo 2009-05-28 14:55) - PLID 34369 - Update wellness template qualification records
	// If we added any patients, we now have to create wellness template qualification records for each of them.  
	if (!prsPatientIDs->eof) {
		// Get the list of IDs
		CDWordArray aryPatientIDs;
		{
			ADODB::FieldPtr fldPatientID = prsPatientIDs->GetFields()->GetItem("PersonID");
			while (!prsPatientIDs->eof) {
				aryPatientIDs.Add(AdoFldLong(fldPatientID));
				prsPatientIDs->MoveNext();
			}
			prsPatientIDs->Close();
		}
		// Pass in the list of new patient ids to update the qualification records for all of them in one shot
		UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), aryPatientIDs);
	}
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddAppointmentToBatch(CString &strSqlBatch)
{

	// (r.goldschmidt 2016-02-01 14:50) - PLID 67976 - add one more time in case there is no appt type column set
	if (!m_bAddedApptType && IsDlgButtonChecked(IDC_APPT_TYPES_AS_CONVERSION_SAVED)) { 
		CString strDataField = "AptTypeID";
		APPEND_APPT_TYPE;
		m_strAppointmentTypeValues += " (SELECT	ID FROM AptTypeT WHERE Name = '" + _Q(GetApptTypeNameConversion()) + "') ";
	}

	// (b.savon 2015-04-14 07:48) - PLID 65219 - Add Appointment Start Time, End Time, Duration, Is Event fields
	COleDateTime dtDate = ParseDate(m_strAppointmentDate);
	//Empty Event = Not an event
	if (m_strAppointmentEvent.IsEmpty()){
		/* Set the start time */
		COleDateTime dtStartTime = ParseTime(m_strAppointmentStartTime);
		dtStartTime.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		m_strAppointmentStartTime = FormatDateTimeForSql(dtStartTime, dtoDateTime);

		/* Fill End time based on populated end time or construct based on duration */
		long nDuration = atol(m_strAppointmentDuration);
		CString strDuration;
		strDuration.Format("%li", nDuration);
		COleDateTime dtEndTime;
		if (m_strAppointmentEndTime.IsEmpty() && nDuration > 0){
			dtEndTime = dtStartTime + COleDateTimeSpan(0, 0, nDuration, 0);
		}
		else{
			dtEndTime = ParseTime(m_strAppointmentEndTime);
			dtEndTime.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());
		}
		m_strAppointmentEndTime = FormatDateTimeForSql(dtEndTime, dtoDateTime);

		// (b.savon 2015-04-22 14:33) - PLID 65219 
		m_strAuditAppointmentValues.Format("Created for %s", FormatDateTimeForInterface(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond()), DTF_STRIP_SECONDS, dtoDateTime));
	}
	else{
		COleDateTime dtStartTime = dtDate;
		m_strAppointmentStartTime = FormatDateTimeForSql(dtStartTime, dtoDate);

		COleDateTime dtEndTime = dtDate;
		m_strAppointmentEndTime = FormatDateTimeForSql(dtEndTime, dtoDate);

		// (b.savon 2015-04-22 14:33) - PLID 65219 
		m_strAuditAppointmentValues.Format("Event Created for %s", FormatDateTimeForInterface(dtDate, NULL, dtoDate));
	}

	m_strAppointmentFields += "CreatedDate, CreatedLogin, ModifiedDate, ModifiedLogin, RecordID, Ready, StartTime, EndTime, ArrivalTime, ";
	m_strAppointmentValues += "GETDATE(), '" + (CString)GetCurrentUserName() + "', GETDATE(), '" + (CString)GetCurrentUserName() + "', -1, 0, '" + m_strAppointmentStartTime + "', '" + m_strAppointmentEndTime + "', '" + m_strAppointmentStartTime + "', ";

	// (b.savon 2015-04-07 15:24) - PLID 65220 - Add Appointment Type
	if (!m_strAppointmentTypeFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @AppointmentTypeID = %s", m_strAppointmentTypeValues);
		m_strAppointmentFields += m_strAppointmentTypeFields;
		m_strAppointmentValues += "@AppointmentTypeID, ";
	}

	// (b.savon 2015-04-10 11:04) - PLID 65221 - Add Appointment Purpose
	// (r.goldschmidt 2016-01-28 16:44) - PLID 67974 - skip if we are adding to the note instead of data
	if (!m_strAppointmentPurposeFields.IsEmpty() && !IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED)){
		AddStatementToSqlBatch(strSqlBatch, "SET @AppointmentPurposeID = %s ", m_strAppointmentPurposeValues);
		m_strAppointmentFields += m_strAppointmentPurposeFields;
		m_strAppointmentValues += "@AppointmentPurposeID, ";
	}

	// (r.goldschmidt 2016-01-28 15:33) - PLID 67974 - add adaptive note
	// (r.goldschmidt 2016-03-01 16:12) - PLID 67974 - make string safe for sql
	if (IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED)) {
		m_strAppointmentFields += "Notes, ";
		m_strAppointmentValues += "'" + _Q(ConstructAppointmentNote()) + "', ";
	}

	// (b.savon 2015-04-08 10:53) - PLID 65222 - Add Location
	if (!m_strLocationFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @LocationID = %s", m_strLocationValues);
		m_strAppointmentFields += m_strLocationFields;
		m_strAppointmentValues += "@LocationID, ";
	}
	else{ 
		// (b.savon 2015-06-30 09:24) - PLID 65222 - Need to import into current logged in location if they don't select a location
		CString strLocationID;
		strLocationID.Format("%li", GetCurrentLocationID());
		AddStatementToSqlBatch(strSqlBatch, "SET @LocationID = %s ", strLocationID);
		m_strAppointmentFields += " LocationID, ";
		m_strAppointmentValues += " @LocationID, ";
	}

	m_strAppointmentFields.TrimRight();
	m_strAppointmentFields.TrimRight(",");
	m_strAppointmentValues.TrimRight();
	m_strAppointmentValues.TrimRight(",");

	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AppointmentsT (%s) SELECT %s ", m_strAppointmentFields, m_strAppointmentValues);
	AddStatementToSqlBatch(strSqlBatch, "SET @AppointmentID = SCOPE_IDENTITY() ");

	// (b.savon 2015-04-09 08:29) - PLID 65223 - Add Appointment Resource Name 
	/* Appointment Resource */
	if (!m_strResourceFields.IsEmpty()){

		m_strResourceFields.TrimRight();
		m_strResourceFields.TrimRight(",");
		m_strResourceValues.TrimRight();
		m_strResourceValues.TrimRight(",");

		AddStatementToSqlBatch(strSqlBatch, "SET @ResourceID = %s ", m_strResourceValues);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AppointmentResourceT(%s) SELECT @AppointmentID, @ResourceID ", m_strResourceFields);
	}

	// (b.savon 2015-04-10 11:04) - PLID 65221 - Add Appointment Purpose
	/* Appointment Purpose */
	// (r.goldschmidt 2016-01-28 16:44) - PLID 67974 - also skip if we are adding to the note instead of data
	if (!m_strAppointmentPurposeFields.IsEmpty() && !IsDlgButtonChecked(IDC_APPT_TYPE_TO_NOTES_SAVED)){

		m_strAppointmentPurposeFields.TrimRight();
		m_strAppointmentPurposeFields.TrimRight(",");
		m_strAppointmentPurposeValues.TrimRight();
		m_strAppointmentPurposeValues.TrimRight(",");

		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AppointmentPurposeT(AppointmentID, PurposeID) SELECT @AppointmentID, @AppointmentPurposeID ");
	}
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddInsuranceCompanyToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	m_strInsuranceCoTFields = "PersonID, " + m_strInsuranceCoTFields + " UserDefinedID, RVUMultiplier, EBillingClaimOffice, TaxType ";
	m_strInsuranceCoTValues = "SELECT @PersonID, " + m_strInsuranceCoTValues + " '', 0, '', 2 ";

	//Trim our trailing spaces and comma
	m_strInsuranceCoTFields.TrimRight();
	m_strInsuranceCoTFields.TrimRight(",");
	m_strInsuranceCoTValues.TrimRight();
	m_strInsuranceCoTValues.TrimRight(",");

	//Insert Into PatientsT is already handled above
	AddStatementToSqlBatch(strSqlBatch,
		"INSERT INTO InsuranceCoT (%s) \r\n"
		"%s ; \r\n "
		, m_strInsuranceCoTFields, m_strInsuranceCoTValues);

	// (r.farnworth 2015-06-16 16:57) - PLID 65166 - We no longer check if the values field is full or not, we must create a contact regardless
	m_strInsuranceContactFields = "ID, " + m_strInsuranceContactFields;
	m_strInsuranceContactValues = "SELECT @InsuranceContactID, " + m_strInsuranceContactValues;

	m_strInsuranceContactFields.TrimRight();
	m_strInsuranceContactFields.TrimRight(",");
	m_strInsuranceContactValues.TrimRight();
	m_strInsuranceContactValues.TrimRight(",");

	AddStatementToSqlBatch(strSqlBatch,
		"SET @InsuranceContactID = (SELECT COALESCE(MAX(ID), 0) +1 FROM PersonT) \r\n"
		"INSERT INTO PersonT (%s) \r\n"
		"%s ; \r\n"
		"INSERT INTO InsuranceContactsT (PersonID, InsuranceCoID) \r\n"
		"SELECT @InsuranceContactID, @PersonID "
		, m_strInsuranceContactFields, m_strInsuranceContactValues);

}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddInsuredPartyToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//(s.dhole 4/28/2015 9:58 AM ) - PLID 65755 audit
	//Set audit person name
	m_strAuditPersonName = GetExistingPatientName(atol(m_strPatientIDValues));
	//Set audit new value
	m_strAuditNewValue = m_strPatientNotesTextValues;
	
	
	

	m_strInsurdPartyFields.TrimRight();
	m_strInsurdPartyFields.TrimRight(",");
	m_strInsurdPartyValues.TrimRight();
	m_strInsurdPartyValues.TrimRight(",");

	//(s.dhole 4/28/2015 10:20 AM ) - PLID 65195  Insured Reposiblity type
	if (!m_strInsurdRespoTypeValue.IsEmpty())
	{
		AddStatementToSqlBatch(strSqlBatch, "SET @InsuRespoPartyID = %s", m_strInsurdRespoTypeValue);
	}
	else
	{
		AddStatementToSqlBatch(strSqlBatch, "SET @InsuRespoPartyID = (SELECT ID FROM RespTypeT WHERE Priority = -1) ");
	}

	//(s.dhole 4/28/2015 10:20 AM ) - PLID 65195  Insured Reposiblity type
	if (!m_strInsurdPartyInsuConvIDValues.IsEmpty())
	{
		AddStatementToSqlBatch(strSqlBatch, "SET @InsuranceContactID = (SELECT TOP  1 PersonID  FROM  InsuranceContactsT  WHERE InsuranceCoID =%s  ORDER BY [Default] DESC) \r\n", m_strInsurdPartyInsuConvIDValues);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuredPartyT (InsuranceContactID,AssignDate,PersonID,InsuranceCoID, RespTypeId, %s ) SELECT @InsuranceContactID, GETDATE(), @PersonID, %s,@InsuRespoPartyID, %s ", m_strInsurdPartyFields, m_strInsurdPartyInsuConvIDValues, m_strInsurdPartyValues);
	}
	else
	{
		AddStatementToSqlBatch(strSqlBatch, "SET @InsuranceContactID = (SELECT TOP  1 PersonID  FROM  InsuranceContactsT  WHERE InsuranceCoID =%s  ORDER BY [Default] DESC) \r\n", m_strInsurdPartyInsuCoNameValues);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuredPartyT (InsuranceContactID,AssignDate,PersonID,InsuranceCoID, RespTypeId, %s ) SELECT  @InsuranceContactID, GETDATE(), @PersonID, %s,@InsuRespoPartyID, %s ", m_strInsurdPartyFields, m_strInsurdPartyInsuCoNameValues, m_strInsurdPartyValues);
	}

	//(s.dhole 4/28/2015 9:58 AM ) - PLID 65755 audit
	//Set audit person name
	m_strAuditPersonName = GetExistingPatientName(atol(m_strPatientIDValues));
	//Set audit new value
	if (!m_strInsurdPartyInsuCoName.IsEmpty())
	{
		m_strAuditNewValue = m_strInsurdPartyInsuCoName;
	}
	else
	{
		m_strAuditNewValue = m_strInsurdPartyInsuConvId;
	}


	if (!m_strInsurdPartyCoPayFields.IsEmpty())
	{
		m_strInsurdPartyCoPayFields.TrimRight();
		m_strInsurdPartyCoPayFields.TrimRight(",");
		m_strInsurdPartyCoPayValues.TrimRight();
		m_strInsurdPartyCoPayValues.TrimRight(",");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID,PayGroupID, %s ) SELECT  @PersonID, (Select ID  from ServicePayGroupsT WHERE Name ='Copay'), %s ", m_strInsurdPartyCoPayFields, m_strInsurdPartyCoPayValues);
	}
	m_strInsurdPartyValues = m_strInsurdPartyFields = m_strInsurdPartyInsuConvId = m_strInsurdPartyInsuCoName = "";
	m_strPatientIDValues = m_strInsurdPartyInsuConvIDValues = m_strInsurdPartyInsuCoNameValues = m_strPersonFields = m_strPersonValues = m_strInsurdPartyCoPayFields = m_strInsurdPartyCoPayValues = "";
	//(s.dhole 4/28/2015 10:20 AM ) - PLID 65195  Insured Reposiblity type
	m_strInsurdRespoTypeValue = "";
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddPatientNoteToBatch(CString &strSqlBatch)
{
	//(s.dhole 4/8/2015 1:39 PM ) - PLID 65228 patitn note category
	if (!m_strPatientNoteCategoryFields.IsEmpty())
	{
		AddStatementToSqlBatch(strSqlBatch, "SET @PatientNoteCategoryID = %s", m_strPatientNoteCategoryValues);
		m_strPatientNotesFields += m_strPatientNoteCategoryFields;
		m_strPatientNotesValues += "@PatientNoteCategoryID, ";
	}
	m_strPatientNotesFields += "  UserID, NoteInputDate, ";
	m_strPatientNotesFields += m_strPatientIDFields;
	CString strDefaultValues;
	strDefaultValues.Format("  %li, GETDATE() AS NoteInputDate, %s,", GetCurrentUserID(), m_strPatientIDValues);
	m_strPatientNotesValues += strDefaultValues;

	if (m_strPatientNotePriorityFields.IsEmpty())
	{
		m_strPatientNotePriorityFields = "Priority,";
		long nDefaultPriority = GetRemotePropertyInt("MyDefaultNotesPriority", 3 /*PRIORITY_LOW*/, 0, GetCurrentUserName(), TRUE);
		m_strPatientNotePriorityValues += FormatString("%li,", nDefaultPriority);
	}
	m_strPatientNotesFields += m_strPatientNotePriorityFields;
	m_strPatientNotesValues += m_strPatientNotePriorityValues;
	
	m_strPatientNotesFields += m_strPatientNotesTextFields;
	//m_strPatientNotesFields += ", ";


	
	
	m_strPatientNotesFields.TrimRight();
	m_strPatientNotesFields.TrimRight(",");
	m_strPatientNotesValues.TrimRight();
	m_strPatientNotesValues.TrimRight(",");
	//Set audit person name
	m_strAuditPersonName = GetExistingPatientName(atol(m_strPatientIDValues));
	//Set audit new value
	m_strAuditNewValue = m_strPatientNotesTextValues;
	//(s.dhole 4/8/2015 1:41 PM ) - PLID 65230 support longert note text , if text more than 4000 character then we will add multiple notes
	CString strNoteTemp = m_strPatientNotesTextValues;
	long nNoteTextMaxLimit = 4000; // Max limit of note text is 4000  character
	if (strNoteTemp.IsEmpty())
	{
		CString strTemp = strNoteTemp.Left(nNoteTextMaxLimit);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO NoteDataT (%s) SELECT %s, '%s'", m_strPatientNotesFields, m_strPatientNotesValues, _Q(strNoteTemp));
		AddStatementToSqlBatch(strSqlBatch, "SET @NewPatientNoteID = SCOPE_IDENTITY() ");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO NoteInfoT(NoteID)  VALUES(@NewPatientNoteID )");

	}
	else
	{
	while (!strNoteTemp.IsEmpty())
	{
		CString strTemp = strNoteTemp.Left(nNoteTextMaxLimit);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO NoteDataT (%s) SELECT %s, '%s'", m_strPatientNotesFields, m_strPatientNotesValues, _Q(strTemp));
		AddStatementToSqlBatch(strSqlBatch, "SET @NewPatientNoteID = SCOPE_IDENTITY() ");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO NoteInfoT(NoteID)  VALUES(@NewPatientNoteID )");
		if (strNoteTemp.GetLength() > nNoteTextMaxLimit)
		{
			strNoteTemp = strNoteTemp.Mid(nNoteTextMaxLimit);
		}
		else
		{
			strNoteTemp = "";
		}
	}
	}
	m_strPatientNotesFields = "";
	m_strPatientNotesValues = "";
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddRecallToBatch(CString &strSqlBatch)
{
	// (b.savon 2015-04-02 09:26) - PLID 65236 - Add Recall Template Name
	if (!m_strPatientRecallTemplateFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @RecallTemplateID = %s", m_strPatientRecallTemplateValues);
		m_strPatientRecallFields += m_strPatientRecallTemplateFields;
		m_strPatientRecallValues += "@RecallTemplateID, ";
	}

	// (b.savon 2015-04-02 10:47) - PLID 65234 - Add Patient Mapping ID and default values
	AddStatementToSqlBatch(strSqlBatch, "SET @SetID = ISNULL((SELECT MAX(ISNULL(RecallT.SetID, 0)) + 1 FROM RecallT), 0)");
	m_strPatientRecallFields += " RecallStepID, CreatedUserID, SetID, SetOrder, ";
	CString strDefaultValues;
	strDefaultValues.Format(" %li, %li, @SetID, %li, ", 1, GetCurrentUserID(), 1);
	m_strPatientRecallValues += strDefaultValues;

	m_strPatientRecallFields.TrimRight();
	m_strPatientRecallFields.TrimRight(",");
	m_strPatientRecallValues.TrimRight();
	m_strPatientRecallValues.TrimRight(",");

	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO RecallT (%s) SELECT %s", m_strPatientRecallFields, m_strPatientRecallValues);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddServiceCodeToBatch(CString &strSqlBatch)
{
	m_strServiceFields = "ID," + m_strServiceFields;
	m_strServiceValues = "SELECT @ServiceID, " + m_strServiceValues;
	//Trim our trailing spaces and comma
	m_strServiceFields.TrimRight();
	m_strServiceFields.TrimRight(",");
	m_strServiceValues.TrimRight();
	m_strServiceValues.TrimRight(",");
	AddStatementToSqlBatch(strSqlBatch,
		"SET @ServiceID = (SELECT COALESCE(MAX(ID), 0) +1 FROM ServiceT) \r\n"
		"INSERT INTO ServiceT (%s) "
		"%s ;\r\n",
		m_strServiceFields, m_strServiceValues);

	// (j.gruber 2012-12-05 11:05) - PLID 48566
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
		"SELECT @ServiceID, ID FROM LocationsT WHERE LocationsT.Managed = 1 ");

	//Add in the ServiceID, this ensures that our other contact strings aren't blank, leaving a trailing comma

	// (j.jones 2010-04-05 11:27) - PLID 16717 - we will fill in the SubCode if the user does not
	if (!m_bHasSubCode) {
		m_strCPTCodeFields += " Subcode, ";
		m_strCPTCodeValues += " '', ";
	}

	m_strCPTCodeFields += "ID, ";
	m_strCPTCodeValues += "@ServiceID, ";
	//Trim our trailing spaces and comma
	m_strCPTCodeFields.TrimRight();
	m_strCPTCodeFields.TrimRight(",");
	m_strCPTCodeValues.TrimRight();
	m_strCPTCodeValues.TrimRight(",");

	//Add to sql batch, we know the strings can't be blank because of the values above.
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO CPTCodeT (%s) "
		"SELECT %s ;",
		m_strCPTCodeFields, m_strCPTCodeValues);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddProductToBatch(CString &strSqlBatch)
{
	m_strProductServiceTFields = "ID," + m_strProductServiceTFields;
	m_strProductServiceTValues = "SELECT @ProductID, " + m_strProductServiceTValues;
	m_strProductProductTFields = "ID," + m_strProductProductTFields;
	m_strProductProductTValues = "SELECT @ProductID, " + m_strProductProductTValues;
	m_strProductProductLocationInfoTFields = "ProductID, LocationID, TrackableStatus, " + m_strProductProductLocationInfoTFields;
	if (m_nProductOnHandAmt > 0)
	{
		m_strProductProductLocationInfoTValues = "SELECT @ProductID, ID, 2, " + m_strProductProductLocationInfoTValues;
	}
	else
	{
		m_strProductProductLocationInfoTValues = "SELECT @ProductID, ID, 0, " + m_strProductProductLocationInfoTValues;
	}

	//Trim our trailing spaces and comma
	m_strProductServiceTFields.TrimRight();
	m_strProductServiceTFields.TrimRight(",");
	m_strProductServiceTValues.TrimRight();
	m_strProductServiceTValues.TrimRight(",");

	m_strProductProductTFields.TrimRight();
	m_strProductProductTFields.TrimRight(",");
	m_strProductProductTValues.TrimRight();
	m_strProductProductTValues.TrimRight(",");

	m_strProductProductLocationInfoTFields.TrimRight();
	m_strProductProductLocationInfoTFields.TrimRight(",");
	m_strProductProductLocationInfoTValues.TrimRight();
	m_strProductProductLocationInfoTValues.TrimRight(",");

	// (r.farnworth 2015-03-20 15:59) - PLID 65238 - We need to add ProductLocationInfoT records for all Locations
	// (b.spivey 2016-05-20 10:46) - NX-100384 - Filter on managed locations, insert into ServiceLocationInfoT. 
	AddStatementToSqlBatch(strSqlBatch,
		"SET @ProductID = (SELECT COALESCE(MAX(ID), 0) +1 FROM ServiceT) \r\n"
		"SET @LocationID = %li \r\n"
		"INSERT INTO ServiceT (%s) \r\n"
		"%s ;\r\n"
		"INSERT INTO ProductT (%s) \r\n"
		"%s ;\r\n"
		"INSERT INTO ProductLocationInfoT (%s) \r\n"
		"%s FROM LocationsT where TypeID = 1 AND Managed = 1;\r\n"
		"INSERT INTO ProductResponsibilityT VALUES (@ProductID, %li, @LocationID) \r\n"
		"INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
		"SELECT @ProductID, ID FROM LocationsT WHERE Managed = 1 \r\n"
		, GetCurrentLocationID(), m_strProductServiceTFields, m_strProductServiceTValues, m_strProductProductTFields, m_strProductProductTValues
		, m_strProductProductLocationInfoTFields, m_strProductProductLocationInfoTValues, GetCurrentUserID());

	// (r.farnworth 2015-03-20 14:23) - PLID 65244 - Only apply On-Hand amount fields for the user's currently logged in location if the amount is not 0 
	if (m_nProductOnHandAmt > 0) {
		//When Importing on hand amounts that are not 0, set the adjustment to $0 and add a note of 'Conversion'
		m_strProductProductAdjustmentsTFields = "ID, ProductID, LocationID, Amount, Notes, " + m_strProductProductAdjustmentsTFields;
		m_strProductProductAdjustmentsTValues = "SELECT (SELECT COALESCE(MAX(ID), 0) +1 FROM ProductAdjustmentsT) AS AdjustmentID, @ProductID, @LocationID, 0, 'Conversion', "
			+ m_strProductProductAdjustmentsTValues;

		m_strProductProductAdjustmentsTFields.TrimRight();
		m_strProductProductAdjustmentsTFields.TrimRight(",");
		m_strProductProductAdjustmentsTValues.TrimRight();
		m_strProductProductAdjustmentsTValues.TrimRight(",");

		AddStatementToSqlBatch(strSqlBatch,
			"INSERT INTO ProductAdjustmentsT (%s) \r\n"
			"%s ;\r\n"
			, m_strProductProductAdjustmentsTFields, m_strProductProductAdjustmentsTValues);
	}
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddResourceToBatch(CString &strSqlBatch)
{
	CString strResourceName = m_strResourceValues;
	m_strResourceFields = " ID, " + m_strResourceFields + " ItemType ";
	m_strResourceValues = " SELECT @ResourceID, " + m_strResourceValues + " 'Doctor' ";
	//Trim our trailing spaces and comma
	strResourceName.TrimRight();
	strResourceName.TrimRight(",");
	m_strResourceValues.TrimRight();
	m_strResourceValues.TrimRight(",");
	AddStatementToSqlBatch(strSqlBatch,
		"SET @ResourceID = (SELECT COALESCE(MAX(ID), 0) +1 FROM ResourceT) \r\n"
		"SET @SecurityObjectID = (SELECT COALESCE(MAX(ID), 0) +1 FROM SecurityObjectT) \r\n"
		"INSERT INTO ResourceT (%s) \r\n"
		"%s ;\r\n"
		"INSERT INTO UserResourcesT(UserID, ResourceID, Relevence) SELECT PersonID AS UserID, @ResourceID AS ResourceID, 1 AS Relevence FROM UsersT \r\n"
		"INSERT INTO SecurityObjectT(ID, BuiltInID, ObjectValue, DisplayName, Description, AvailablePermissions) "
		"VALUES (@SecurityObjectID, %li, @ResourceID, %s, 'Controls access to view or schedule appointments for this resource.', 63) \r\n",
		m_strResourceFields, m_strResourceValues, bioSchedIndivResources, strResourceName);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddOtherContactToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//Add in the person ID, this ensures that our other contact strings aren't blank, leaving a trailing comma
	m_strOtherContactFields += "PersonID, ";
	m_strOtherContactValues += "@PersonID, ";
	//Trim our trailing spaces and comma
	m_strOtherContactFields.TrimRight();
	m_strOtherContactFields.TrimRight(",");
	m_strOtherContactValues.TrimRight();
	m_strOtherContactValues.TrimRight(",");

	//Add to sql batch, we know the strings can't be blank because of the values above.
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ContactsT (%s) "
		"SELECT %s ;",
		m_strOtherContactFields, m_strOtherContactValues);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddSupplierToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//Add in the person ID, this ensures that our supplier strings aren't blank, leaving a trailing comma
	m_strSupplierFields += "PersonID, ";
	m_strSupplierValues += "@PersonID, ";
	//Trim our trailing spaces and comma
	m_strSupplierFields.TrimRight();
	m_strSupplierFields.TrimRight(",");
	m_strSupplierValues.TrimRight();
	m_strSupplierValues.TrimRight(",");

	//Add to sql batch, we know the strings can't be blank because of the values above.
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO SupplierT (%s) "
		"SELECT %s ;",
		m_strSupplierFields, m_strSupplierValues);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddUserToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//Add in the person ID, this ensures that our user strings aren't blank, leaving a trailing comma
	m_strUserFields += "PersonID, ";
	m_strUserValues += "@PersonID, ";
	//Trim our trailing spaces and comma
	m_strUserFields.TrimRight();
	m_strUserFields.TrimRight(",");
	m_strUserValues.TrimRight();
	m_strUserValues.TrimRight(",");

	//Add to sql batch, we know the strings can't be blank because of the values above.
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO UsersT (%s) "
		"SELECT %s ;",
		m_strUserFields, m_strUserValues);

	//We also need to add default records into the additional associated tables for new users
	//Create a permissions entry but don't assign any permissions.
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO UserPermissionsT (ID) VALUES (@PersonID);");
	//allow these users access to the current location
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO UserLocationT (PersonID, LocationID) VALUES (@PersonID, %li)", GetCurrentLocationID());
	//Make sure users have a default resource ordering in the scheduler
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO UserResourcesT (UserID, ResourceID, Relevence) "
		"SELECT @PersonID, ID, (SELECT COUNT(SubQ.ID) FROM ResourceT SubQ WHERE SubQ.ID <= ResourceT.ID) AS Relevance FROM ResourceT; ");

}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddReferringPhysicianToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//Add in the person ID, this ensures that our ref phys strings aren't blank, leaving a trailing comma
	m_strRefPhysFields += "PersonID, ";
	m_strRefPhysValues += "@PersonID, ";
	//Trim our trailing spaces and comma
	m_strRefPhysFields.TrimRight();
	m_strRefPhysFields.TrimRight(",");
	m_strRefPhysValues.TrimRight();
	m_strRefPhysValues.TrimRight(",");

	//Add to sql batch, we know the strings can't be blank because of the values above.
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ReferringPhysT (%s) "
		"SELECT %s ;",
		m_strRefPhysFields, m_strRefPhysValues);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddProviderToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//Add in the default location, this ensures that our provider strings aren't blank, leaving a trailing comma
	//TES 9/8/2008 - PLID 27727 - This field no longer exists, so we'll just have to handle the case where the
	// strings are blank.
	//m_strProviderFields += "DefLocationID, ";
	//m_strProviderValues += AsString(GetCurrentLocationID()) + ", ";
	if (!m_strProviderFields.IsEmpty()) {
		m_strProviderFields = ", " + m_strProviderFields;
	}
	if (!m_strProviderValues.IsEmpty()) {
		m_strProviderValues = ", " + m_strProviderValues;
	}
	//Trim our trailing spaces and comma
	m_strProviderFields.TrimRight();
	m_strProviderFields.TrimRight(",");
	m_strProviderValues.TrimRight();
	m_strProviderValues.TrimRight(",");

	//Add to sql batch, defaulting claim provider to themself,
	//and default location ID to the first active managed location (like the contacts module does).
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProvidersT (PersonID, ClaimProviderID%s) "
		"SELECT @PersonID, @PersonID%s ;",
		m_strProviderFields, m_strProviderValues);
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddPatientToBatch(CString &strSqlBatch)
{
	AddPersonToBatch(strSqlBatch);

	//Check if the user totally forgot about PatientID
	if (m_bHasPatientID == FALSE){
		// (v.maida 2015-04-06 15:58) - PLID 65440 - Use a SQL variable set to a subquery, instead of just a subquery.
		AddStatementToSqlBatch(strSqlBatch, "SET @UserDefinedID = (SELECT COALESCE(MAX(UserDefinedID), 0) +1 FROM PatientsT) ");
		//this ensures that our patient strings aren't blank, leaving a trailing comma
		m_strPatientFields += " UserdefinedID, ";
		m_strPatientValues += "@UserDefinedID, ";
	}

	// (b.savon 2015-03-23 11:40) - PLID 65158 - Add Referral Source Name
	if (!m_strReferralSourceFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @ReferralSourceID = %s", m_strReferralSourceValues);
		m_strPatientFields += m_strReferralSourceFields;
		m_strPatientValues += "@ReferralSourceID, ";
	}

	// (b.savon 2015-03-25 13:26) - PLID 65150 - Add ProviderID 
	if (!m_strPatientProviderFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @ProviderID = %s", m_strPatientProviderValues);
		m_strPatientFields += m_strPatientProviderFields;
		m_strPatientValues += "@ProviderID, ";
	}

	// (b.savon 2015-03-26 13:26) - PLID 65151 - Add Referring Physician ID
	if (!m_strPatientReferringPhysFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @RefPhysID = %s", m_strPatientReferringPhysValues);
		m_strPatientFields += m_strPatientReferringPhysFields;
		m_strPatientValues += "@RefPhysID, ";
	}

	// (b.savon 2015-03-26 14:23) - PLID 65152 - Add PCP Name 
	if (!m_strPatientPCPFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @PCPID = %s", m_strPatientPCPValues);
		m_strPatientFields += m_strPatientPCPFields;
		m_strPatientValues += "@PCPID, ";
	}

	// (b.savon 2015-07-06 10:08) - PLID 65156 - If they don't choose a column, use the preference.
	if (m_strPatientFields.Find("CurrentStatus, ") == -1){
		long nCurrentStatus = 1;
		if (GetRemotePropertyInt("NewPatientDefault", 2, 0, "<None>", true) == 1){
			nCurrentStatus = 1;
		}
		else{
			nCurrentStatus = 2;
		}

		m_strPatientFields += " CurrentStatus, "; 
		m_strPatientValues += " " + AsString(nCurrentStatus) + ", ";
	}

	//Trim our trailing spaces and comma
	m_strPatientFields.TrimRight();
	m_strPatientFields.TrimRight(",");
	m_strPatientValues.TrimRight();
	m_strPatientValues.TrimRight(",");

	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientsT (PersonID, %s) SELECT @PersonID, %s ;",
		m_strPatientFields, m_strPatientValues);

	// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
	AddStatementToSqlBatch(strSqlBatch,
		"IF (@SecurityGroupID IS NOT NULL)\r\n"
		"BEGIN\r\n"
		"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, @PersonID)\r\n"
		"END\r\n");

	// (b.savon 2015-03-20 12:36) - PLID 65153 - Add Patient Race to database
	AddRaceStatementToBatch(strSqlBatch, m_strPersonRaceFields, m_strPersonRaceValues);

	// (b.savon 2015-03-23 11:55) - PLID 65158 - Add Referral Source Name batch statement for MultiReferralT
	AddMultiReferalSourceToBatch(strSqlBatch, m_strReferralSourceFields, m_strReferralSourceValues);

	// (b.savon 2015-03-25 08:22) - PLID 65144 - Add Custom Fields query batch statement
	AddCustomFieldsToBatch(strSqlBatch, m_strCustomFieldsQuery);

	// (b.cardillo 2009-05-28 14:55) - PLID 34369 - We need to collect the list of the PersonIDs of all patients that we add
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @tPatientIDs (PersonID) SELECT @PersonID;");
}

// (b.savon 2015-04-30 16:26) - PLID 65511 - Split batch logic out
void CImportWizardPreviewDlg::AddPersonToBatch(CString &strSqlBatch)
{
	// (j.jones 2010-01-14 16:39) - PLID 31927 - check the default text message privacy field
	long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);

	//this ensures that our person strings aren't blank, leaving a trailing comma
	// (j.jones 2010-01-14 16:35) - PLID 31927 - supported defaulting the text message privacy field
	m_strPersonFields = "ID,TextMessage," + m_strPersonFields;
	m_strPersonValues = "SELECT @PersonID, " + AsString(nTextMessagePrivacy) + ", " + m_strPersonValues;

	// (b.savon 2015-03-20 15:26) - PLID 65154 - Add Ethnicity Import
	//Trim our trailing spaces and comma
	if (!m_strPersonEthnicityFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @EthnicityID = %s", m_strPersonEthnicityValues);
		m_strPersonFields += m_strPersonEthnicityFields;
		m_strPersonValues += "@EthnicityID, ";
	}

	// (b.savon 2015-03-23 07:32) - PLID 65155 - Add Language
	//Trim our trailing spaces and comma
	if (!m_strPersonLanguageFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @LanguageID = %s", m_strPersonLanguageValues);
		m_strPersonFields += m_strPersonLanguageFields;
		m_strPersonValues += "@LanguageID, ";
	}

	// (b.savon 2015-03-23 10:41) - PLID 65157 - Add Location
	if (!m_strLocationFields.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "SET @LocationID = %s", m_strLocationValues);
		m_strPersonFields += m_strLocationFields;
		m_strPersonValues += "@LocationID, ";
	}

	//Trim our trailing spaces and comma
	m_strPersonFields.TrimRight();
	m_strPersonFields.TrimRight(",");
	m_strPersonValues.TrimRight();
	m_strPersonValues.TrimRight(",");
	AddStatementToSqlBatch(strSqlBatch,
		"SET @PersonID = (SELECT COALESCE(MAX(ID), 0) +1 FROM PersonT) \r\n"
		"INSERT INTO PersonT (%s) "
		"%s ;\r\n",
		m_strPersonFields, m_strPersonValues);
}

// (r.goldschmidt 2016-02-10 11:23) - PLID 68163 - Split batch logic out
void CImportWizardPreviewDlg::AddRaceToBatch(CString &strSqlBatch)
{	
	if (m_bHasRacePreferredName && m_bPairedToRaceCDCCode) {
		// add preferred race name to master race list and associate it with the cdc code
		AddStatementToSqlBatch(strSqlBatch,
			"SET @RaceID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM RaceT) \r\n"
			"INSERT INTO RaceT (ID, Name, RaceCodeID) \r\n"
			"	SELECT TOP 1 @RaceID AS ID, '%s' AS Name, ID AS RaceCodeID \r\n"
			"	FROM RaceCodesT WHERE CDCID = '%s' \r\n"
			, m_strRacePreferredNameValue, m_strRaceCDCCodeValue);
	}
	else if (m_bHasRacePreferredName) { // implied !m_bPairedToRaceCDCCode
		// add preferred race name to master race list
		AddStatementToSqlBatch(strSqlBatch,
			"SET @RaceID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM RaceT) \r\n"
			"INSERT INTO RaceT (ID, Name) VALUES (@RaceID, '%s') \r\n"
			, m_strRacePreferredNameValue);
	}
	else {
		ASSERT(FALSE); // should have never validated to this stage
	}
}

// (r.goldschmidt 2016-01-28 11:56) - PLID 67976 - get name of type used for conversion button option
CString CImportWizardPreviewDlg::GetApptTypeNameConversion() 
{
	return "Conversion";
}

// (r.goldschmidt 2016-01-28 12:37) - PLID 67974 - Build note from type and purpose and note when the setting is enabled
CString CImportWizardPreviewDlg::ConstructAppointmentNote() 
{

	// build string
	CString strFinalApptNote = m_strAppointmentTypeReplacedValue;

	if (!m_strAppointmentPurposeReplacedValue.IsEmpty()) { // if there is a purpose
		if (!strFinalApptNote.IsEmpty()) { // if there is a note
			strFinalApptNote += " - " + m_strAppointmentPurposeReplacedValue;
		}
		else {
			strFinalApptNote = m_strAppointmentPurposeReplacedValue;
		}
	}
	if (!m_strAppointmentNoteReplacedValue.IsEmpty()) { // if there is a note
		if (!strFinalApptNote.IsEmpty()) { // if there is a type with or without a purpose
			strFinalApptNote += " - " + m_strAppointmentNoteReplacedValue;
		}
		else { // should not be possible, but just in case
			strFinalApptNote = m_strAppointmentNoteReplacedValue;
		}
	}

	return strFinalApptNote;
}

// (r.goldschmidt 2016-01-28 16:53) - PLID 67976 - Carry over settings from fields panel to preview panel
void CImportWizardPreviewDlg::SetApptConversionCheckbox(BOOL bChecked)
{
	m_bApptConversionChecked = bChecked;
}

// (r.goldschmidt 2016-01-28 16:53) - PLID 67976 - Carry over settings from fields panel to preview panel
void CImportWizardPreviewDlg::SetApptNoteCheckbox(BOOL bChecked)
{
	m_bApptNotesChecked = bChecked;
}

// (r.goldschmidt 2016-01-28 16:53) - PLID 67976 - Carry over settings from fields panel to preview panel
void CImportWizardPreviewDlg::SetApptNoteCheckboxText(CString strText)
{
	m_strApptNotesCheckboxText = strText;
}

// (r.goldschmidt 2016-02-02 17:13) - PLID 67976 -let the user know which columns won't convert 
void CImportWizardPreviewDlg::AddExplanatoryText(CString strText)
{
	m_strAdditionalExplanatoryText = strText;
}