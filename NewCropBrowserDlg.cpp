// NewCropBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewCropBrowserDlg.h"
#include "NewCropUtils.h"
#include "SOAPUtils.h"
#include "NewCropSoapFunctions.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "SingleSelectDlg.h"
#include "MsgBox.h"
#include "FirstDataBankUtils.h"
#include "DecisionRuleUtils.h"
#include <NxDataUtilitiesLib/CLRUtils.h>
#include "foreach.h"
#include "practice.h"
#include "NxAPI.h"

// (j.jones 2013-04-17 09:49) - PLID 56275 - this code doesn't currently need anything in the API header,
// but could easily change to use it, because this class already includes the API in its own header
#include "PrescriptionUtilsNonAPI.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// CNewCropBrowserDlg dialog

// (j.jones 2009-02-16 12:24) - PLID 33053 - created

using namespace ADODB;

// (j.jones 2009-03-04 12:50) - PLID 33332 - required for OnPostShowWindow
#define ID_NEWCROP_BROWSER_POST_SHOW_WINDOW	33332

// (j.gruber 2009-03-30 17:25) - PLID 33736 - added nEMNID and nPatientID
// (j.gruber 2009-03-30 17:25) - PLID 33728 - added closing message window
// (j.gruber 2009-05-15 17:13) - PLID 28541 - added strXML for renewal responses
// (j.gruber 2009-06-08 09:40) - PLID 34515 - added role 
// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
CNewCropBrowserDlg::CNewCropBrowserDlg(long nPatientPersonID, long nPatientUserDefinedID, long nEMNID, CWnd* pWndToSendClosingMsgTo, CString strXML, CWnd* pParent)
	: CNxDialog(CNewCropBrowserDlg::IDD, pParent)
{
	m_ncatDefaultAction = ncatInvalid;
	m_nActionID = -1;
	m_nPatientPersonID = nPatientPersonID;
	// (j.jones 2013-01-04 08:52) - PLID 49624 - added UserDefinedID
	m_nPatientUserDefinedID = nPatientUserDefinedID;
	m_bIsPopupWindow = FALSE;
	m_nEMNID = nEMNID;
	m_pWndToSendClosingMsgTo = pWndToSendClosingMsgTo;
	// (c.haag 2009-05-13 16:43) - PLID 34257 - Retain this value. Even if another user
	// edits the allergy list, the caller may be from an EMR, and wanting to update all
	// the active allergy details no matter who changed the master allergy list.
	m_nOriginalAllergiesInfoID = GetActiveAllergiesInfoID();
	// (c.haag 2009-05-15 09:55) - PLID 34271 - Retain this value as well.
	m_nOriginalCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();

	// (j.gruber 2009-05-15 17:14) - PLID 28541 - added strXML for renewal responses
	m_strPassedInXML = strXML;

	// (j.jones 2009-08-13 14:31) - PLID 35213 - added a boolean to make sure the
	// default action function absolutely cannot be called twice in one browser window
	m_bHasPerformedDefaultAction = FALSE;

	// (j.jones 2010-01-21 17:46) - PLID 37004 - we now require FirstDataBank here
	m_bHasEnsuredFirstDataBankOnce = FALSE;

	// (j.armen 2012-06-06 12:39) - PLID 50830 - Set min size
	SetMinSize(780, 580);
}

CNewCropBrowserDlg::~CNewCropBrowserDlg()
{
	
}

void CNewCropBrowserDlg::PostNcDestroy() 
{
	try {

		CNxDialog::PostNcDestroy();

		//if we are the mainframe's main dialog, destroy ourselves
		if(!m_bIsPopupWindow) {
			
			ASSERT(GetMainFrame()->m_pNewCropBrowserDlg == this);

			GetMainFrame()->m_pNewCropBrowserDlg = NULL;
			delete this;
		}

	}NxCatchAll("Error in CNewCropBrowserDlg::PostNcDestroy");
}

void CNewCropBrowserDlg::OnDestroy() 
{
	try {

		m_pBrowser->Quit();

		int i=0;
		for(i=m_paryPopupWindows.GetSize()-1; i>=0; i--) {
			CNewCropBrowserDlg *pDlg = (CNewCropBrowserDlg*)m_paryPopupWindows.GetAt(i);
			if(pDlg) {
				delete pDlg;
				pDlg = NULL;
			}
		}
		m_paryPopupWindows.RemoveAll();

		// (j.jones 2009-03-04 11:27) - PLID 33332 - save our window size
		//By using GetWindowPlacement, we are saving the rect for the window when it isn't 
		//minimized/maximized, even if at the moment it happens to be minimized/maximized.
		//So I added an extra flag to save whether it is currently maximized, so we can restore that.
		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);
		BOOL bMaximized = FALSE;
		if(wp.showCmd == SW_SHOWMAXIMIZED) {
			bMaximized = TRUE;
		}
		CString strBuffer;
		strBuffer.Format("%d,%d,%d,%d,%d", wp.rcNormalPosition.left, wp.rcNormalPosition.top, wp.rcNormalPosition.right, wp.rcNormalPosition.bottom, bMaximized ? 1 : 0);
		if(m_bIsPopupWindow) {
			AfxGetApp()->WriteProfileString("Settings", "NewCropBrowserSizePopup", strBuffer);
		}
		else {
			AfxGetApp()->WriteProfileString("Settings", "NewCropBrowserSizeMain", strBuffer);
		}

		if(!m_bIsPopupWindow) {
			//if not a popup window, re-enable mainframe
			GetMainFrame()->EnableWindow(TRUE);

			// (j.jones 2009-08-13 14:14) - PLID 35213 - added a mutex for NewCrop patient account access,
			// this applies to the NewCrop browser actions only (such as ncatAccessPatientAccount and
			// ncatProcessRenewalRequest), not any follow-up SOAP calls we may be attempting
			GetMainFrame()->SetIsBrowsingNewCropPatientAccount(FALSE);
		}

	}NxCatchAll("Error in CNewCropBrowserDlg::OnDestroy");

	CNxDialog::OnDestroy();
}

void CNewCropBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_UPDATE_AND_CLOSE, m_btnUpdateClose);
	DDX_Control(pDX, IDC_BROWSER_STATUS, m_editBrowserStatus);
}


BEGIN_MESSAGE_MAP(CNewCropBrowserDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_UPDATE_AND_CLOSE, OnBtnUpdateAndClose)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_DESTROY()
	ON_MESSAGE(ID_NEWCROP_BROWSER_POST_SHOW_WINDOW, OnPostShowWindow)
END_MESSAGE_MAP()

BOOL CNewCropBrowserDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (j.jones 2010-04-13 16:00) - PLID 38183 - added bulk cache
		g_propManager.CachePropertiesInBulk("CNewCropBrowserDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewCropHideProviderSuffix' OR "
			"Name = 'NewCropAccountLocationID' OR "
			// (j.jones 2010-06-24 15:47) - PLID 39013 - this ConfigRT setting has been renamed
			//"Name = 'NewCropUseProductionServer' "
			"Name = 'NewCrop_ProductionStatusOverride' OR "
			// (j.jones 2011-03-07 11:46) - PLID 42313 - added an option to not send patient gender
			"Name = 'NewCrop_DoNotSendPatientGender' "
			")",
			_Q(GetCurrentUserName()));

		m_btnUpdateClose.AutoSet(NXB_CLOSE);

		// (j.gruber 2009-03-26 15:10) - PLID 33051 - changed to update and Close
		m_btnUpdateClose.SetWindowTextA("Update && Close");

		m_pBrowser = GetDlgItem(IDC_NEWCROP_BROWSER)->GetControlUnknown();

		if(!m_strDefaultWindowText.IsEmpty()) {
			//set our window text
			SetWindowText(m_strDefaultWindowText);
		}

		//if a popup window, we want to alter the interface somewhat
		if(m_bIsPopupWindow) {
			//make sure our button to close only says "Close", not "Update & Close",
			//as we don't actually update the patient's account
			m_btnUpdateClose.SetWindowText("Close");

			//TODO: if we ever add other controls to this dialog (for other actions, etc.),
			//be sure to hide them here, when a popup

			// (j.jones 2009-03-04 12:11) - PLID 33332 - remove the maximize button
			LONG nStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
			// Remove flags we must no longer have
			nStyle &= ~WS_MAXIMIZEBOX;
			// Set the new style
			SetWindowLong(m_hWnd, GWL_STYLE, nStyle);
		}

		if(!m_bIsPopupWindow) {
			//if not a popup window, disable mainframe to fake being modal
			GetMainFrame()->EnableWindow(FALSE);
			EnableWindow();
		}

		GetControlPositions();

		// (j.jones 2009-03-04 11:21) - PLID 33332 - Size the window to the last size it was
		{
			// Get the work area to make sure that wherever we put it, it's accessible
			CRect rcWork;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
			// Get the last size and position of the window
			CRect rcDialog;
			CString strBuffer = "";
			if(m_bIsPopupWindow) {
				strBuffer = AfxGetApp()->GetProfileString("Settings", "NewCropBrowserSizePopup");
			}
			else {
				strBuffer = AfxGetApp()->GetProfileString("Settings", "NewCropBrowserSizeMain");
			}
			long nMaximized = 0;
			if (strBuffer.IsEmpty() || _stscanf(strBuffer, "%d,%d,%d,%d,%d", &rcDialog.left, &rcDialog.top, &rcDialog.right, &rcDialog.bottom, &nMaximized) != 5) {
				// We couldn't get the registry setting for some reason - Just leave it at whatever size it was default generated at
				GetWindowRect(rcDialog);
				ScreenToClient(rcDialog);

				if(m_bIsPopupWindow) {
					//don't center popups by default, put them in the upper-left corner so they won't completely obscure our main window
					rcDialog.MoveToXY(10, 10);
				}
			}
			// Make sure if we put the dialog at rcDialog it's accessible (we consider 'accessible' 
			// to mean that the dialog title bar is visible vertically, and 1/3 visible horizontally)
			if(rcDialog.top+rcDialog.Height()/8<rcWork.bottom && rcDialog.top>rcWork.top &&
				rcDialog.left<rcWork.right-rcDialog.Width()/3 && rcDialog.right>rcWork.left+rcDialog.Width()/3) {
				// It's accessible so leave it
			} else {
				// It's not accessible so center it if the main window, put in the upper left corner is a popup
				if(m_bIsPopupWindow) {
					rcDialog.MoveToXY(10, 10);		
				}
				else {
					CSize ptDlgHalf(rcDialog.Width()/2, rcDialog.Height()/2);
					CPoint ptScreenCenter(rcWork.CenterPoint());
					rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
				}
			}
			// Move the window to its new position
			//MoveWindow(rcDialog);

			WINDOWPLACEMENT wp;
			GetWindowPlacement(&wp);
			wp.rcNormalPosition = rcDialog;			
			//if maximized, now maximize from that position
			if(nMaximized == 1) {
				wp.showCmd = SW_SHOWMAXIMIZED;
			}
			else {
				wp.showCmd = SW_SHOWNORMAL;
			}
			SetWindowPlacement(&wp);
		}

		// (j.jones 2009-03-04 12:52) - PLID 33332 - post a message to call OnPostShowWindow
		PostMessage(ID_NEWCROP_BROWSER_POST_SHOW_WINDOW);
		
	}NxCatchAll("Error in CNewCropBrowserDlg::OnInitDialog");

	return TRUE;
}

// CNewCropBrowserDlg message handlers


// (j.gruber 2009-03-30 17:32) - PLID 33051 - Call the Soap function
// (j.gruber 2009-05-14 13:33) - PLID 34259 - added prescription array
BOOL CNewCropBrowserDlg::GetNewCropPrescriptionHistory(CPtrArray *paryMedications, CPtrArray *paryPrescriptions) {
	//get the prescription history
	CString strCredentials, strAccountRequest, strPatientRequest, 
		strPrescriptionHistoryRequest, strPatientInformationRequester, 
		strPatientTypeID;

	if (!GetCredentials(strCredentials) ) {
		return FALSE;
	}
	if (!GetAccountRequest(strAccountRequest)) {
		return FALSE;
	}

	// (j.gruber 2009-05-19 17:27) - PLID 34306 - needs to send the patientID
	if (!GetPatientRequest(strPatientRequest, m_nPatientPersonID)) {
		return FALSE;
	}

	if (!GetPrescriptionHistoryRequest(strPrescriptionHistoryRequest)) {
		return FALSE;
	}

	if (! GetPatientInformationRequester(strPatientInformationRequester)) {
		return FALSE;
	}

	strPatientTypeID = "1";

	CString strErrorMessage;

	if (GetPatientFullMedicationHistory(strCredentials, strAccountRequest, strPatientRequest,
		strPrescriptionHistoryRequest, strPatientInformationRequester, strPatientTypeID, paryMedications, paryPrescriptions, strErrorMessage) ) {
		return TRUE;

	}
	else {
		AfxMessageBox(strErrorMessage);
		return FALSE;
	}
}


void CNewCropBrowserDlg::OnBtnUpdateAndClose()
{
	try {
		//don't try to update the patient account yet if it's a popup window
		if (!m_bIsPopupWindow) {
			OnUpdatePatientAccount();
		}
	
		// (j.gruber 2009-03-30 17:27) - PLID 33728 - send a closing message
		// (c.haag 2009-05-13 16:34) - PLID 34257 - We now pass a structure rather than selective ID's
		if (m_pWndToSendClosingMsgTo && m_pWndToSendClosingMsgTo->GetSafeHwnd())
		{
			NewCropBrowserResult* pNCBR = new NewCropBrowserResult;
			pNCBR->nEMNID = m_nEMNID;
			pNCBR->nPatientID = m_nPatientPersonID;
			pNCBR->nOldAllergiesInfoID = m_nOriginalAllergiesInfoID;
			pNCBR->nNewAllergiesInfoID = GetActiveAllergiesInfoID();
			// (c.haag 2009-05-15 09:55) - PLID 34271 - Current medications
			pNCBR->nOldCurrentMedicationsInfoID = m_nOriginalCurrentMedicationsInfoID;
			pNCBR->nNewCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
			// (c.haag 2010-02-18 09:56) - PLID 37424
			pNCBR->aNewlyAddedPatientPrescriptions.Copy( m_aNewlyAddedPatientPrescriptions );
			// I don't expect that this dialog will exist much longer; but this will make things 100% certain that the
			// array is clear if this dialog is reused.
			m_aNewlyAddedPatientPrescriptions.RemoveAll();

			// Post the message
			m_pWndToSendClosingMsgTo->PostMessage(NXM_NEWCROP_BROWSER_DLG_CLOSED, (WPARAM)pNCBR);
		}

		CNxDialog::OnOK();

		::DestroyWindow(GetSafeHwnd());

	}NxCatchAll("Error in CNewCropBrowserDlg::OnBtnUpdateAndClose");
}

void CNewCropBrowserDlg::OnCancel()
{
	try {
		//don't try to update the patient account yet if it's a popup window
		if (!m_bIsPopupWindow) {
			OnUpdatePatientAccount();
		}

		// (j.gruber 2009-03-30 17:27) - PLID 33728 - send a closing message
		// (c.haag 2009-05-13 16:34) - PLID 34257 - We now pass a structure rather than selective ID's
		if(m_pWndToSendClosingMsgTo && m_pWndToSendClosingMsgTo->GetSafeHwnd())
		{
			NewCropBrowserResult* pNCBR = new NewCropBrowserResult;
			pNCBR->nEMNID = m_nEMNID;
			pNCBR->nPatientID = m_nPatientPersonID;
			pNCBR->nOldAllergiesInfoID = m_nOriginalAllergiesInfoID;
			pNCBR->nNewAllergiesInfoID = GetActiveAllergiesInfoID();
			// (c.haag 2009-05-15 09:55) - PLID 34271 - Current medications
			pNCBR->nOldCurrentMedicationsInfoID = m_nOriginalCurrentMedicationsInfoID;
			pNCBR->nNewCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
			// (c.haag 2010-02-18 09:56) - PLID 37424
			pNCBR->aNewlyAddedPatientPrescriptions.Copy( m_aNewlyAddedPatientPrescriptions );

			// Post the message
			m_pWndToSendClosingMsgTo->PostMessage(NXM_NEWCROP_BROWSER_DLG_CLOSED, (WPARAM)pNCBR);
		}

		//Because OnCancel calls OnUpdatePatientAccount, I think it may
		//be wise to continue to return the OnOK message, though no
		//caller should really care. Places that actually close the dialog
		//before processing anything however do close with OnCancel.
		CNxDialog::OnOK();

		::DestroyWindow(GetSafeHwnd());

	}NxCatchAll("Error in CNewCropBrowserDlg::OnCancel");
}

// (j.gruber 2009-03-30 17:25) - PLID 33051 - update/add the drug
// (j.jones 2009-09-10 17:16) - PLID 35508 - I renamed CreateNewDrug to EnsureDrugExists, which will create the drug
// if it doesn't exist, update the drug if the name changed, or simply return the existing drug.
// Returns the DrugID, or -1 if an error occurred.
long CNewCropBrowserDlg::EnsureDrugExists(NewCropPatientMedication *pMed, CString &strFullDrugDescription) {
	
	long nDrugID = -1;
	BOOL bNeedClearNewCropID = FALSE;

	//
	// (c.haag 2007-01-30 16:32) - PLID 24422 - This function is called to add a medication to the DrugList
	// table. We must update the EMR data as well.
	//
	//DRT 11/26/2008 - PLID 32177 - Optimized bad query setup

	// (j.jones 2009-09-10 16:34) - PLID 35508 - This combination does not match what NewCrop displays on their website,
	// for example a DosageForm might be "application" but the name of the drug containts "ointment". Version 6 of
	// GetPatientFullMedicationHistory includes the full name now in the DrugInfo field.
	
	//strFullDrugDescription = pMed->strDrugName + " " + pMed->strStrength + " " + pMed->strStrengthUOM + " " + pMed->strDosageForm;
	strFullDrugDescription = pMed->strDrugInfo;

	// (j.jones 2009-09-10 17:16) - PLID 35508 - see if the drug exists, and is in the current medications list
	//if so, we do not need to create a new one	
	long nActiveMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
	// (j.jones 2010-01-21 17:01) - PLID 37004 - grab the existing NDCNumber
	// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
	// (j.gruber 2010-09-23 11:01) - PLID 40647 - check deacode
	_RecordsetPtr rsDrug = CreateParamRecordset("SELECT DrugList.ID, EMRDataT.ID AS EMRDataID, "
		"EMRDataT.Data AS FullDescriptionName, DrugList.NDCNumber, DrugList.DEASchedule "
		"FROM DrugList "
		"INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
		"WHERE DrugList.FDBID = {STRING} AND EMRDataT.EMRInfoID = {INT}", pMed->strDrugID, nActiveMedicationsInfoID);
	if(!rsDrug->eof) {
		nDrugID = AdoFldLong(rsDrug, "ID");
		long nEMRDataID = AdoFldLong(rsDrug, "EMRDataID");
		CString strExistingDrugDescription = AdoFldString(rsDrug, "FullDescriptionName");
		CString strDEASchedule = AdoFldString(rsDrug, "DEASchedule", "");

		//has the name of the drug changed?
		// (j.gruber 2012-08-08 15:54) - PLID 51576 - take this out
		/*if(strExistingDrugDescription.CompareNoCase(strFullDrugDescription) != 0) {
			//the name of the drug has changed, so clear the nDrugID, we're going to 
			//create it new, and remove the NewCropID from the existing drug
			nDrugID = -1;
			bNeedClearNewCropID = TRUE;
		}*/
		//else {
			//continue with the existing drug description, just incase the case changed
			strFullDrugDescription = strExistingDrugDescription;

			// (j.jones 2010-01-21 17:01) - PLID 37004 - while we will use this drug,
			// we need to see if the NDC is filled in, and if not, try to calculate it
			CString strNDCNumber = AdoFldString(rsDrug, "NDCNumber", "");
			strNDCNumber.TrimRight();

			// (j.gruber 2010-09-23 11:00) - PLID 40647 - newcrop added DEA codes we should store			
			//only update it if its blank
			CString strDEACode;
			if (!strDEASchedule.IsEmpty()) {
				strDEACode = strDEASchedule;
			}
			else {
				if (pMed->strDEAClassCode == "1") {
					strDEACode = "I";
				}
				else if (pMed->strDEAClassCode == "2") {
					strDEACode = "II";
				}
				else if (pMed->strDEAClassCode == "3") {
					strDEACode = "III";
				}
				else if (pMed->strDEAClassCode == "4") {
					strDEACode = "IV";
				}
				else if (pMed->strDEAClassCode == "5") {
					strDEACode = "V";
				}
				else if (pMed->strDEAClassCode == "6") {
					strDEACode = "VI";
				}
				else if (pMed->strDEAClassCode == "7") {
					strDEACode = "VII";
				}
				else if (pMed->strDEAClassCode == "8") {
					strDEACode = "VIII";
				}
				else if (pMed->strDEAClassCode == "9") {
					strDEACode = "IX";
				}
				else if (pMed->strDEAClassCode == "10") {
					strDEACode = "X";
				}
			}

			//only update if it was empty and we now have a value
			if (strDEASchedule.IsEmpty() && !strDEACode.IsEmpty()) {
				ExecuteParamSql("UPDATE Druglist SET DEASchedule = {STRING} WHERE ID = {INT}", strDEACode, nDrugID);
			}

			//DrugTypeID is F if DrugID is a FirstDataBankID, to my knowledge
			//NewCrop never sends us anything but F
			if(strNDCNumber.IsEmpty() && pMed->strDrugTypeID == "F") {
				//try to find the drug's NDC

				// (j.jones 2010-01-26 17:43) - PLID 37078 - this function has been moved
				// to FirstDataBankUtils

				//ensure the FirstDataBank database exists
				BOOL bIsFDBDatabaseValid = FALSE;
				if(!m_bHasEnsuredFirstDataBankOnce) {
					bIsFDBDatabaseValid = FirstDataBank::EnsureDatabase(this, true);
					m_bHasEnsuredFirstDataBankOnce = TRUE;
				} else {
					bIsFDBDatabaseValid = FirstDataBank::EnsureDatabase(NULL, true); // no UI
				}

				if(bIsFDBDatabaseValid) {
					strNDCNumber = FirstDataBank::ChooseNDCNumberFromFirstDataBank(atoi(pMed->strDrugID));
				}

				//we may not necessarily have an NDC
				if(!strNDCNumber.IsEmpty()) {
					//we have one, so save it
					ExecuteParamSql("UPDATE DrugList SET NDCNumber = {STRING} WHERE ID = {INT}", strNDCNumber, nDrugID);
				}
			}
			
			//return now with this drug ID
			return nDrugID;
		//}
	}
	rsDrug->Close();

	//try to find the strength unit and dosageform
	_variant_t varStrengthUnitID, varDosageFormID;
	CString strStrengthUOM, strDosageForm;

	strStrengthUOM = pMed->strStrengthUOM;
	strDosageForm = pMed->strDosageForm;

	//try to replace the simple ones
	if (strStrengthUOM.CompareNoCase("mg") == 0) { 
		strStrengthUOM = "Milligram";
	}

	if (strStrengthUOM.CompareNoCase("ml") == 0) { 
		strStrengthUOM = "Milliliter";
	}

	if (strStrengthUOM.CompareNoCase("g") == 0) { 
		strStrengthUOM = "gram";
	}	
	
	strDosageForm.Replace("tablet(s)", "tablet");
	strDosageForm.Replace("capsule(s)", "capsule");	

	_RecordsetPtr rsStrengthUOM = CreateParamRecordset("SELECT ID FROM DrugStrengthUnitsT WHERE name = {STRING}", strStrengthUOM);
	if (rsStrengthUOM->eof) {
		varStrengthUnitID = g_cvarNull;
	}
	else {
		long nStrengthUOM = AdoFldLong(rsStrengthUOM, "ID");
		varStrengthUnitID = (long)nStrengthUOM;		
	}
	rsStrengthUOM->Close();

	_RecordsetPtr rsDosageForm = CreateParamRecordset("SELECT ID FROM DrugDosageFormsT WHERE name = {STRING}", strDosageForm);
	if (rsDosageForm->eof) {
		varDosageFormID = g_cvarNull;
	}
	else {
		long nDosageForm = AdoFldLong(rsDosageForm, "ID");
		varDosageFormID = (long)nDosageForm;		
	}
	rsDosageForm->Close();

	// (j.jones 2010-01-21 17:01) - PLID 37004 - try to calculate an NDC Number
	CString strNDCNumber = "";

	//DrugTypeID is F if DrugID is a FirstDataBankID, to my knowledge
	//NewCrop never sends us anything but F
	if(pMed->strDrugTypeID == "F") {

		//try to find the drug's NDC

		// (j.jones 2010-01-26 17:43) - PLID 37078 - this function has been moved
		// to FirstDataBankUtils

		//ensure the FirstDataBank database exists
		BOOL bIsFDBDatabaseValid = FALSE;
		if(!m_bHasEnsuredFirstDataBankOnce) {
			bIsFDBDatabaseValid = FirstDataBank::EnsureDatabase(this, true);
			m_bHasEnsuredFirstDataBankOnce = TRUE;
		} else {
			bIsFDBDatabaseValid = FirstDataBank::EnsureDatabase(NULL, true); // no UI
		}

		if(bIsFDBDatabaseValid) {			
			strNDCNumber = FirstDataBank::ChooseNDCNumberFromFirstDataBank(atoi(pMed->strDrugID));
		}
		
		//we may not necessarily have an NDC now, but otherwise we'll save a blank string
	}

	// (j.gruber 2010-09-23 10:47) - PLID 40647 - DEA code
	CString strDEACode;
	if (pMed->strDEAClassCode == "1") {
		strDEACode = "I";
	}
	else if (pMed->strDEAClassCode == "2") {
		strDEACode = "II";
	}
	else if (pMed->strDEAClassCode == "3") {
		strDEACode = "III";
	}
	else if (pMed->strDEAClassCode == "4") {
		strDEACode = "IV";
	}
	else if (pMed->strDEAClassCode == "5") {
		strDEACode = "V";
	}
	else if (pMed->strDEAClassCode == "6") {
		strDEACode = "VI";
	}
	else if (pMed->strDEAClassCode == "7") {
		strDEACode = "VII";
	}
	else if (pMed->strDEAClassCode == "8") {
		strDEACode = "VIII";
	}
	else if (pMed->strDEAClassCode == "9") {
		strDEACode = "IX";
	}
	else if (pMed->strDEAClassCode == "10") {
		strDEACode = "X";
	}


	BEGIN_TRANS("ExecuteAddition")
		long nEmrInfoID = GetActiveCurrentMedicationsInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchCurrentMedicationsInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		CString strSqlBatch;
		CNxParamSqlArray args;

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		strOldValue.Format("Item: 'Current Medications'");
		strNewValue.Format("New Table Row: '%s'", strFullDrugDescription);
		
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemCreated, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetCreated);

		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON;\r\n");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDataGroupID int;\r\n");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDataID int;\r\n");		
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDrugID int;\r\n");		
		// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataGroupID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataGroupsT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDrugID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM DrugList);\r\n");

		AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO EMRDataGroupsT (ID) VALUES (@NewDataGroupID);\r\n");

		// Now add the new medication to the EMRDataT table
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			"INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, ListType, EmrDataGroupID) "
			"SELECT @NewDataID, {INT}, {STRING}, "
			"(SELECT COALESCE(Max(SortOrder),0) + 1 FROM EMRDataT WHERE EMRInfoID = {INT}), "
			"2, @NewDataGroupID ",
			nEmrInfoID, strFullDrugDescription, nEmrInfoID);	

		// (j.jones 2009-09-11 09:27) - PLID 35508 - if bNeedClearNewCropID is TRUE, it means we need to remove the NewCropID
		// from the existing drug that already uses it
		// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
		if(bNeedClearNewCropID) {
			AddParamStatementToSqlBatch(strSqlBatch, args, 
				"UPDATE DrugList SET FDBID = NULL WHERE FDBID = {STRING}", pMed->strDrugID);
		}

		// Now add the medication to DrugList
		// (c.haag 2007-02-02 16:14) - PLID 24561 - The DrugList table no longer has a name field
		// (d.thompson 2008-12-01) - PLID 32175 - Cleaned up for new fields.
		// (d.thompson 2009-03-11) - PLID 33477 - Added QuantityUnitID, which is always 'Tablet' to start
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @QtyUnitID int;\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @QtyUnitID = (SELECT ID FROM DrugStrengthUnitsT WHERE Name = 'Tablet');\r\n");	

		// (j.jones 2010-01-21 17:01) - PLID 37004 - save the NDC Number, even if it is empty
		// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
		// (j.gruber 2010-09-23 10:21) - PLID 40647 - added DEA Code
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			"INSERT INTO DrugList (ID, DefaultRefills, DefaultQuantity, EMRDataID, "
			"DrugName, Strength, StrengthUnitID, "
			"DosageFormID, QuantityUnitID, FDBID, NDCNumber, DEASchedule) "
			"VALUES (@NewDrugID, '0', '0', @NewDataID, "
			"{STRING}, {STRING}, {VT_I4}, "
			"{VT_I4}, @QtyUnitID, {STRING}, {STRING}, {STRING})", 
			pMed->strDrugName, pMed->strStrength, 
			varStrengthUnitID, varDosageFormID, pMed->strDrugID, strNDCNumber, strDEACode); 

		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT OFF;\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SELECT @NewDrugID as DrugID;\r\n");

		// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
		_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
		nDrugID = AdoFldLong(prs, "DrugID");

		END_TRANS("ExecuteAddition")

			
		// (c.haag 2007-02-07 13:49) - PLID 24422 - This function will pop up an exception message if,
		// for some unexpected reason, a record exists in DrugList with a bad EmrDataID, or a record exists
		// in EmrDataT for the active Current Medications item that does not correspond to DrugList
		WarnEmrDataDiscrepanciesWithDrugList();		

		return nDrugID;
}

// (j.gruber 2009-05-13 15:21) - PLID 34251 - sync allergies back
BOOL CNewCropBrowserDlg::GetNewCropAllergyHistory(CPtrArray *paryAllergies) {
	
	//get the allergy history
	CString strCredentials, strAccountRequest, strPatientRequest, strPatientInformationRequester;

	if (!GetCredentials(strCredentials) ) {
		return FALSE;
	}
	if (!GetAccountRequest(strAccountRequest)) {
		return FALSE;
	}

	// (j.gruber 2009-05-19 17:27) - PLID 34306 - needs to take the patient id
	if (!GetPatientRequest(strPatientRequest, m_nPatientPersonID)) {
		return FALSE;
	}

	if (! GetPatientInformationRequester(strPatientInformationRequester)) {
		return FALSE;
	}

	CString strErrorMessage;

	if (GetPatientAllergyHistory(strCredentials, strAccountRequest, strPatientRequest,
		strPatientInformationRequester, paryAllergies, strErrorMessage) ) {
		return TRUE;

	}
	else {
		AfxMessageBox(strErrorMessage);
		return FALSE;
	}
}

// (j.gruber 2009-05-13 15:21) - PLID 34251 - sync allergies back
long CNewCropBrowserDlg::CreateNewAllergy(NewCropPatientAllergy *pAllergy, CString strAllergyName)
{
		long nAllergyID = -1;

		// (j.jones 2012-10-12 14:34) - PLID 52642 - if NewCrop provides an FDB ConceptID and ConceptTypeID, save them
		_variant_t varConceptID = g_cvarNull;
		_variant_t varConceptTypeID = g_cvarNull;
		if(pAllergy->strSource.CompareNoCase("F") == 0) {
			long nConceptID = atoi(pAllergy->strConceptID);
			long nConceptTypeID = atoi(pAllergy->strConceptTypeID);
			//need both to be valid in order to use them
			if(nConceptID > 0 && nConceptTypeID > 0) {
				varConceptID = (long)nConceptID;
				varConceptTypeID = (long)nConceptTypeID;
			}
		}

		BEGIN_TRANS("ExecuteAddition")

		long nEmrInfoID = GetActiveAllergiesInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchAllergiesInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		strOldValue.Format("Item: 'Allergies'");
		strNewValue.Format("New Table Row: " + strAllergyName);
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemCreated, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetCreated);

		// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
		long nEMRDataGroupID = NewNumber("EMRDataGroupsT", "ID");
		ExecuteParamSql("INSERT INTO EMRDataGroupsT (ID) VALUES ({INT})", nEMRDataGroupID);

		long nDataID = NewNumber("EMRDataT", "ID");

		// Now add the new allergy to the EMRDataT table
		ExecuteParamSql("INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, ListType, EmrDataGroupID) "
			"SELECT {INT}, {INT}, {STRING}, "
			"(SELECT COALESCE(Max(SortOrder),0) + 1 FROM EMRDataT WHERE EMRInfoID = {INT}), "
			"2, {INT}",
			nDataID, nEmrInfoID, strAllergyName, nEmrInfoID, nEMRDataGroupID);

		// Now add the allergy to AllergyT
		// (j.jones 2012-10-12 14:36) - PLID 52642 - now potentially includes the FDB ConceptID, ConceptTypeID
		nAllergyID = NewNumber("AllergyT", "ID");
		ExecuteParamSql("INSERT INTO AllergyT (ID, EMRDataID, ConceptID, ConceptIDType) "
			"VALUES ({INT}, {INT}, {VT_I4}, {VT_I4})", nAllergyID, nDataID, varConceptID, varConceptTypeID);

	END_TRANS("ExecuteAddition")

	// (c.haag 2007-04-04 16:14) - PLID 25498 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in AllergyT with a bad EmrDataID, or a record
	// exists in EmrDataT for the active Allergies item that does not correspond to AllergyT
	WarnEmrDataDiscrepanciesWithAllergyT();

	return nAllergyID;

}

// (j.gruber 2009-05-13 15:21) - PLID 34251 - sync allergies back
// (a.walling 2010-01-14 13:41) - PLID 36888 - Pass in the old RxNorm concept unique identifier
// (j.jones 2012-10-17 10:32) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
// This function will clear that status if it adds an allergy.
BOOL CNewCropBrowserDlg::UpdatePatientAllergy(BOOL bIsNew, long nAllergyID, NewCropPatientAllergy *pAllergy, CString strFullAllergyDescription,
											  CString &strSqlBatch, long &nAuditTransactionID, CNxParamSqlArray &args,
											  BOOL &bCurHasNoAllergiesStatus, BOOL &bNeedsReviewed, BOOL bDiscontinued,
											  CString strOldNotes /*=""*/, CString strOldRXCUI /*= ""*/)
{
	try {

		if (bIsNew) {

			//we have to create the new record
			// (c.haag 2009-12-22 12:30) - PLID 35766 - Added EnteredDate
			// (a.walling 2010-01-14 13:40) - PLID 36888 - add the new RXCUI
			AddParamStatementToSqlBatch(strSqlBatch, args, "SET @nNewID = COALESCE((SELECT Max(ID) FROM PatientAllergyT), 0) + 1");
			AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PatientAllergyT "
				"(ID, AllergyID, PersonID, Description, FromNewCrop, EnteredDate, RXCUI) VALUES "
				"(@nNewID, {INT}, {INT}, {STRING}, 1, GetDate(), {STRING}) ",
				nAllergyID, m_nPatientPersonID, pAllergy->strNotes, pAllergy->strRXCUI);

			//audit
			CString strPatientName = GetExistingPatientName(m_nPatientPersonID);
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(m_nPatientPersonID, strPatientName, nAuditTransactionID, aeiPatientAllergyAdd, m_nPatientPersonID, "", strFullAllergyDescription, aepMedium, aetCreated);

			bNeedsReviewed = TRUE;

			// (j.jones 2012-10-17 10:13) - PLID 53179 - since we added allergies, clear the HasNoAllergies status,
			// but only if it is currently set (passed into this function through bCurHasNoAllergiesStatus)
			if(bCurHasNoAllergiesStatus) {

				AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientsT SET HasNoAllergies = 0 WHERE PersonID = {INT}", m_nPatientPersonID);
				AuditEvent(m_nPatientPersonID, strPatientName, nAuditTransactionID, aeiPatientHasNoAllergiesStatus, m_nPatientPersonID, "'No Known Allergies' Status Selected", "'No Known Allergies' Status Cleared", aepMedium, aetChanged);

				//set the status to false, to reflect our change
				bCurHasNoAllergiesStatus = FALSE;
			}
		}
		else {

			//the only thing that can change here is the notes or the RXCUI, so let's see if that happened
			if (strOldNotes.Compare(pAllergy->strNotes) == 0 && strOldRXCUI.Compare(pAllergy->strRXCUI) == 0) {

				if (bDiscontinued) {
					//check the discontinued status also
					// (c.haag 2010-08-06 13:30) - PLID 40022 - Set the discontinued date
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientAllergyT SET Discontinued = 0, DiscontinuedDate = NULL WHERE ID = {INT}", nAllergyID);

					//audit
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOldAudit, strNewAudit;
					strOldAudit.Format("%s - Inactive", strFullAllergyDescription);
					strNewAudit.Format("Active");
					CString strPatientName = GetExistingPatientName(m_nPatientPersonID);
					AuditEvent(m_nPatientPersonID, strPatientName, nAuditTransactionID, aeiPatientActivateAllergy, m_nPatientPersonID, strOldAudit, strNewAudit, aepMedium, aetChanged);

					bNeedsReviewed = TRUE;
				}
				
			}
			else {
				// (a.walling 2010-01-14 13:45) - PLID 36888 - Did we get an RXCUI that differs? if so, update
				if (strOldRXCUI.Compare(pAllergy->strRXCUI) != 0) {
					//the RXCUI changed, so update it

					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientAllergyT SET RXCUI = {STRING} WHERE ID = {INT}", pAllergy->strRXCUI, nAllergyID);

					// (a.walling 2010-01-14 13:46) - PLID 36888 - No need to audit this, I believe.
				}

				// (a.walling 2010-01-14 13:45) - PLID 36888 - Continue with old behavior if notes have changed
				if (strOldNotes.Compare(pAllergy->strNotes) != 0) {
					//the note changed, so update the notes
					//also update the discontinued because if we got here, its active
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE PatientAllergyT SET Description = {STRING}, Discontinued = 0 WHERE ID = {INT}", pAllergy->strNotes, nAllergyID);
					
					//audit
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strOldAudit, strNewAudit;
					strOldAudit.Format("Allergy: '%s' - Description: %s", strFullAllergyDescription, strOldNotes);
					strNewAudit.Format("Description: %s", pAllergy->strNotes);
					CString strPatientName = GetExistingPatientName(m_nPatientPersonID);
					AuditEvent(m_nPatientPersonID, strPatientName, nAuditTransactionID, aeiPatientAllergyDescription, m_nPatientPersonID, strOldAudit, strNewAudit, aepMedium, aetChanged);

					bNeedsReviewed = TRUE;

					if (bDiscontinued) {

						//audit
						if (nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						CString strOldAudit, strNewAudit;
						strOldAudit.Format("Allergy: '%s' - Inactive", strFullAllergyDescription);
						strNewAudit.Format("Active");
						CString strPatientName = GetExistingPatientName(m_nPatientPersonID);
						AuditEvent(m_nPatientPersonID, strPatientName, nAuditTransactionID, aeiPatientActivateAllergy, m_nPatientPersonID, strOldAudit, strNewAudit, aepMedium, aetChanged);
					}
				}
			}
		}

		return TRUE;
	}NxCatchAllCall("Error in NewCropBrowserDlg::UpdatePatientAllergies",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return FALSE;
}

// (j.gruber 2009-05-13 15:21) - PLID 34251 - sync allergies back
// (j.jones 2013-03-21 16:44) - PLID 49625 - obsolete now, we use the API to do this
/*
BOOL CNewCropBrowserDlg::UpdatePatientAllergies()
{

	long nAuditTransactionID = -1;
	
	try {

		BOOL bNeedsReviewed = FALSE;
		BOOL bMarkAsReviewed = FALSE; // (c.haag 2009-06-15 10:01) - PLID 34611
		
		// (j.gruber 2009-05-12 16:40) - PLID 34251 - sync back the allergies		
		CPtrArray aryAllergies;
		CString strSqlBatch, strDefaultSqlBatch;	
		CNxParamSqlArray args;
		if (!GetNewCropAllergyHistory(&aryAllergies)) {

			//clear the list
			for(int i=0; i<aryAllergies.GetSize(); i++) {
				NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));
				delete pAllergy;
			}
			aryAllergies.RemoveAll();

			return FALSE;
		}
		
		AddDeclarationToSqlBatch(strDefaultSqlBatch, "DECLARE @nNewID INT");
		strSqlBatch = strDefaultSqlBatch;

		BOOL bCurHasNoAllergiesStatus = FALSE;

		// (j.gruber 2009-05-12 16:41) - PLID 34251 - first run through the patient's list of allergies and mark them discontinued as necessary
		// (j.jones 2012-10-17 09:32) - PLID 53179 - Added HasNoAllergies
		_RecordsetPtr rsAllergies = CreateParamRecordset("SELECT HasNoAllergies FROM PatientsT WHERE PersonID = {INT}\r\n"
			""
			"SELECT PatientAllergyT.ID, Name FROM PatientAllergyT LEFT JOIN (SELECT ID, (SELECT Data FROM EMRDataT WHERE ID = AllergyT.EMRDataID) as Name FROM AllergyT) AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID WHERE FromNewCrop = 1 AND PersonID = {INT} AND Discontinued = 0",
			m_nPatientPersonID, m_nPatientPersonID);
		if(!rsAllergies->eof) {
			bCurHasNoAllergiesStatus = VarBool(rsAllergies->Fields->Item["HasNoAllergies"]->Value, FALSE);
		}
		rsAllergies = rsAllergies->NextRecordset(NULL);
		while (!rsAllergies->eof) {
			CString strAllergyName = AdoFldString(rsAllergies, "Name", "");
			// (d.singleton 2012-05-23 17:51) - PLID 50613 trim any leading spaces so we can successfully match it up with the newcrop allergy
			strAllergyName.Trim();
				
			if (!strAllergyName.IsEmpty()) {

				//run through our medication list and see if its there
				BOOL bFound = FALSE;
				for (int i=0; i < aryAllergies.GetCount(); i++) {
					NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));

					if (strAllergyName.CompareNoCase(pAllergy->strName) == 0) {
						bFound = TRUE;
					}
				}

				if (bFound == FALSE) {
					//we didn't get it back in the list, so its been discontinued
					long nID = AdoFldLong(rsAllergies, "ID");
					// (c.haag 2010-08-06 13:30) - PLID 40022 - Set the discontinued date
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientAllergyT SET Discontinued = 1, DiscontinuedDate = GetDate() WHERE ID = %li", nID);				

					//audit
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strOldAudit, strNewAudit;
					strOldAudit.Format(strAllergyName + " - Active");
					strNewAudit.Format("Inactive");
					CString strPatientName = GetExistingPatientName(m_nPatientPersonID);
					AuditEvent(m_nPatientPersonID, strPatientName, nAuditTransactionID, aeiPatientInactivateAllergy, m_nPatientPersonID, strOldAudit, strNewAudit, aepMedium, aetChanged);

					bNeedsReviewed = TRUE;

					
				}
			}
			rsAllergies->MoveNext();
		}				

		//loop through the array and check the data, adding as necessary
		for (int i=0; i < aryAllergies.GetCount(); i++) {

			NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));

			if(pAllergy) {

				
				//first look to see if we have an allergy for this
				long nAllergyID;
				CString strAllergyName;

				//check for the FDB source
				long nNewCropConceptID = -1;
				long nNewCropConceptTypeID = -1;
				if(pAllergy->strSource.CompareNoCase("F") == 0) {
					nNewCropConceptID = atoi(pAllergy->strConceptID);
					nNewCropConceptTypeID = atoi(pAllergy->strConceptTypeID);
				}

				// (j.jones 2012-10-12 14:37) - PLID 52642 - pull the ConceptID and ConceptTypeID,
				// also sort such that if multiple allergies match by name, pull the first allergy that
				// matches by the FDBIDs
				_RecordsetPtr rsAllergy = CreateParamRecordset("SELECT TOP 1 AllergyT.ID, EMRDataT.Data AS FullDescriptionName, "
					"AllergyT.ConceptID, AllergyT.ConceptIDType "
					"FROM AllergyT "
					"INNER JOIN EMRDataT ON AllergyT.EMRDataID = EMRDataT.ID "
					"WHERE EMRDataT.Data = {STRING} "
					"ORDER BY (CASE WHEN ConceptID = {INT} AND ConceptIDType = {INT} AND {INT} <> -1 AND {INT} <> -1 THEN 0 ELSE 1 END) ASC, "
					"AllergyT.ID ASC",
					pAllergy->strName, nNewCropConceptID, nNewCropConceptTypeID, nNewCropConceptID, nNewCropConceptTypeID);
				if (!rsAllergy->eof) {
					nAllergyID = AdoFldLong(rsAllergy, "ID");
					strAllergyName = AdoFldString(rsAllergy, "FullDescriptionName");

					// (j.jones 2012-10-12 14:39) - PLID 52642 - if both the ConceptID and ConceptTypeID are null,
					// and NewCrop provided them with FDB as the source, update the allergy, but NOT if another
					// allergy is already using those IDs
					long nAllergyConceptID = AdoFldLong(rsAllergy, "ConceptID", -1);
					long nAllergyConceptTypeID = AdoFldLong(rsAllergy, "ConceptIDType", -1);

					if(nAllergyConceptID == -1 && nAllergyConceptTypeID == -1) {						
						//need both to be valid in order to use them
						if(nNewCropConceptID > 0 && nNewCropConceptTypeID > 0) {
							//update the allergy, but only if no other allergy uses these IDs
							ExecuteParamSql("UPDATE AllergyT SET ConceptID = {INT}, ConceptIDType = {INT} "
								"WHERE AllergyT.ID = {INT} "
								"AND NOT EXISTS (SELECT ID FROM AllergyT WHERE ConceptID = {INT} AND ConceptIDType = {INT})",
								nNewCropConceptID, nNewCropConceptTypeID,
								nAllergyID,
								nNewCropConceptID, nNewCropConceptTypeID);
						}
					}
				}
				else {
					
					nAllergyID = CreateNewAllergy(pAllergy, pAllergy->strName);
					if (nAllergyID == -1) {
						//clear the list
						for(int i=0; i<aryAllergies.GetSize(); i++) {
							NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));
							delete pAllergy;
						}
						aryAllergies.RemoveAll();
						return FALSE;
					}
					strAllergyName = pAllergy->strName;
				}

				//check to see if this allergy exists on this patient
				long nPatAllergyID = -1;
				
				//we allow duplicate entries of allergies, so if they already have a non-newcrop allergy that is the exact same as the newcrop allergy, we are going to add the duplicate				
				// (a.walling 2010-01-14 13:42) - PLID 36888 - Retrieve the RXCUI to detect changes
				_RecordsetPtr rsPatAllergy = CreateParamRecordset("SELECT PatientAllergyT.ID, "
					"PatientAllergyT.Description, AllergyT.Name, PatientAllergyT.Discontinued, PatientAllergyT.RXCUI "
					"FROM PatientAllergyT "					
					"LEFT JOIN (SELECT ID, (SELECT Data FROM EMRDataT WHERE ID = AllergyT.EMRDataID) as Name FROM AllergyT) AllergyT "
					"ON PatientAllergyT.AllergyID = AllergyT.ID "
					"WHERE AllergyT.Name = {STRING} AND FromNewCrop = 1 AND PersonID = {INT} ", pAllergy->strName, m_nPatientPersonID);
				if (!rsPatAllergy->eof) {
					//this allergy already exists, so we need to update and audit
					nPatAllergyID = AdoFldLong(rsPatAllergy, "ID");

					CString strOldNotes;
					
					strOldNotes = AdoFldString(rsPatAllergy, "Description");

					BOOL bDiscontinued = AdoFldBool(rsPatAllergy, "Discontinued");

					// (a.walling 2010-01-14 13:42) - PLID 36888 - The old RXCUI
					CString strOldRXCUI = AdoFldString(rsPatAllergy, "RXCUI");
										
					// (j.jones 2012-10-17 10:34) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
					// This function will clear that status if it adds an allergy. It will also set bCurHasNoAllergiesStatus to FALSE if it
					// changes the value from TRUE to FALSE.
					if (!UpdatePatientAllergy(FALSE, nPatAllergyID, pAllergy, strAllergyName,
						strSqlBatch, nAuditTransactionID, args,
						bCurHasNoAllergiesStatus, bNeedsReviewed, bDiscontinued,
						strOldNotes, strOldRXCUI)) {

						//clear the list
						for(int i=0; i< aryAllergies.GetSize(); i++) {
							NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));
							delete pAllergy;
						}
						aryAllergies.RemoveAll();

						return FALSE;
					}
				}
				else {

					//create a new Allergy

					// (j.jones 2012-10-17 10:34) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
					// This function will clear that status if it adds an allergy. It will also set bCurHasNoAllergiesStatus to FALSE if it
					// changes the value from TRUE to FALSE.
					if (!UpdatePatientAllergy(TRUE, nAllergyID, pAllergy, strAllergyName,
						strSqlBatch, nAuditTransactionID, args,
						bCurHasNoAllergiesStatus, bNeedsReviewed, FALSE) ) {

						//clear the list
						for(int i=0; i<aryAllergies.GetSize(); i++) {
							NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));
							delete pAllergy;
						}
						aryAllergies.RemoveAll();

						return FALSE;
					}						
				}				
			}
		}

		//need to uncheck the reviewed status if something changed and we are coming 
		//from the medications tab (EMR already does it for us)
		// (c.haag 2009-06-15 09:44) - PLID 34611 - If we're in an EMR, we need to prompt the user as to what to do about the allergy list review state.
		// This is consistent with behavior in EMR, where, if you edit an Allergies item, you will get the prompt as well. I copied the message text from
		// code in CEMR::PostSaveUpdateAllergies
		//
		// Also, I believe the comment "(EMR already does it for us)" may have been correct in the distant past, but definitely not in the recent past
		// based on my experiences when testing. The reason for this is that patient allergies and EMR allergies are synchronized after the newcrop dialog
		// is closed...and the saving code will only change the reviewed status if the two are different.
		//
		if (bNeedsReviewed) {

			if (m_nEMNID == -1) {
				// (c.haag 2009-06-15 09:55) - PLID 34611 - bMarkAsReviewed should be false (this is the default behavior)
				bMarkAsReviewed = FALSE;
			}
			else {
				BOOL bHasAllergyReviewPerms = GetCurrentUserPermissions(bioPatientAllergyReview) & SPT___W________ANDPASS;
				
				if(bHasAllergyReviewPerms && MessageBox("This patient's allergy information has changed.\n"
					"Do you wish to certify that you have reviewed the allergy list with the patient?\n\n"
					"If not, you can always review the allergy list later in their Medications Tab.", "Practice", MB_ICONQUESTION|MB_YESNO) == IDYES) {
					//now check the permission, it may need a password
					if(CheckCurrentUserPermissions(bioPatientAllergyReview, sptWrite)) {
						bMarkAsReviewed = TRUE;
					}
				}
			}

			// (c.haag 2009-06-15 10:05) - PLID 34611 - Should we flag this as reviewed with the patient?
			if (bMarkAsReviewed) {
				// Yes
				AddParamStatementToSqlBatch(strSqlBatch,args, 
					"DECLARE @dtNow datetime \r\n"
					"SET @dtNow = GetDate() \r\n"
					"UPDATE PatientsT SET AllergiesReviewedOn = @dtNow, AllergiesReviewedBy = {INT} WHERE PersonID = {INT}", GetCurrentUserID(), m_nPatientPersonID);
				// Don't audit until after the batch has been run -- we won't know what the actual reviewed-on date is until then.
				// In fact, to find it out, we need to select it here. This must always be the last statement in the batch.
				AddParamStatementToSqlBatch(strSqlBatch,args, "SELECT @dtNow AS CurDate");
			} else {
				//check to see if the checkbox is checked
				_RecordsetPtr rsCheck = CreateParamRecordset("SELECT AllergiesReviewedOn FROM PatientsT WHERE PersonID = {INT}", m_nPatientPersonID);
				BOOL bNeedsUpdate = FALSE;
				COleDateTime dtNull;
				dtNull.SetDate(1899,12,31);
				if (!rsCheck->eof) {
					COleDateTime dtTemp = AdoFldDateTime(rsCheck, "AllergiesReviewedOn", dtNull);

					if (dtTemp.GetYear() == 1899 && dtTemp.GetMonth() == 12 && dtTemp.GetDay() == 31) {
						bNeedsUpdate = FALSE;
					}
					else {
						bNeedsUpdate = TRUE;
					}
				}

				if (bNeedsUpdate) {
					// No (maybe the user didn't have permission; but nothing we can do about that)
					AddParamStatementToSqlBatch(strSqlBatch,args, "UPDATE PatientsT SET AllergiesReviewedOn = NULL, AllergiesReviewedBy = NULL WHERE PersonID = {INT}", m_nPatientPersonID);
					//audit even if it was already marked not reviewed because the description just sudits that the allergy list hasn't been reviewed, not that something actually changed, which is true.
					AuditEvent(m_nPatientPersonID, GetExistingPatientName(m_nPatientPersonID), nAuditTransactionID, aeiPatientAllergiesReviewed, m_nPatientPersonID, "", "Allergy information has not been reviewed.", aepMedium, aetChanged);
				}
			}
		} // if (bNeedsReviewed) {

		//if we got here, we can execute our stuff
		if(strSqlBatch != strDefaultSqlBatch) {
			// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
			_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
			// (c.haag 2009-06-15 10:05) - PLID 34611 - If the batch marked the allergy list as having been reviewed, we need to
			// fetch the date and then audit it. For the record, I think this is shady; but it has to be done.
			if (bMarkAsReviewed) {
				// Seek to the last recordset. I won't assume there is a consistent number of recordsets in this query from
				// one release to the next. The important thing is that "SELECT @dtNow AS CurDate" is the last one
				_RecordsetPtr prsNext = prs->NextRecordset(NULL);
				while (NULL != prsNext) {
					prs = prsNext;
					prsNext = prs->NextRecordset(NULL);
				}
				COleDateTime dtReviewedOn = AdoFldDateTime(prs, "CurDate");
				CString strNew;
				strNew.Format("Allergy information has been reviewed by %s on %s.", GetCurrentUserName(), FormatDateTimeForInterface(dtReviewedOn, DTF_STRIP_SECONDS, dtoDateTime));
				AuditEvent(m_nPatientPersonID, GetExistingPatientName(m_nPatientPersonID), nAuditTransactionID, aeiPatientAllergiesReviewed, m_nPatientPersonID, "", strNew, aepMedium, aetChanged);
			}
			CommitAuditTransaction(nAuditTransactionID);	
		}

		//clear the list
		for(int i=0; i<aryAllergies.GetSize(); i++) {
			NewCropPatientAllergy *pAllergy = ((NewCropPatientAllergy*)aryAllergies.GetAt(i));
			delete pAllergy;
		}
		aryAllergies.RemoveAll();

		return TRUE;

	}NxCatchAllCall("Error in CNewCropBrowserDlg::UpdatePatientAllergies",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		});

	return FALSE;
}
*/

//OnUpdatePatientAccount should be called before closing this dialog,
//and will return TRUE upon success, FALSE if a problem occurred
// (j.luckoski 2013-01-31 15:30) - PLID 49625 - Function now calls the API and then then updates teh new prescriptions with the results
void CNewCropBrowserDlg::OnUpdatePatientAccount()
{	
	CWaitCursor cwWait;
	// (r.farnworth 2015-07-27 15:53) - PLID 64583 -Added a try within a try so we could catch our error's description but still have an NxCatchAllCall at the end of it all
	try{
		try {
			m_aNewlyAddedPatientPrescriptions.RemoveAll();

			NexTech_Accessor::_NewCropPatientMedicationArrayPtr newPrescriptions;

			//(r.wilson 3/1/2013) pl 49625 - Get the UserDefinedID
			CString strPatID = "";
			strPatID.Format("%li", m_nPatientUserDefinedID);

			CString strEMNID = "";
			strEMNID.Format("%li", m_nEMNID);

			// (j.luckoski 2013-04-29 17:54) - PLID 56489 - Needs a nullable bool for skipping reconcile
			NexTech_Accessor::_NullableBoolPtr pSkipReconcile(__uuidof(NexTech_Accessor::NullableBool));
			pSkipReconcile->SetBool(VARIANT_TRUE);

			// (j.luckoski 2013-04-29 15:16) - PLID 56489 - Reconile only if requested.
			if (m_nEMNID > 0) {
				newPrescriptions = GetAPI()->UpdatePatientAndEMNFromNewCrop(GetAPISubkey(), GetAPILoginToken(), (_bstr_t)strPatID, (_bstr_t)strEMNID, GetAPI()->GetNewCropUserSettings(GetAPISubkey(), GetAPILoginToken()), pSkipReconcile);
			}
			else {
				newPrescriptions = GetAPI()->UpdatePatientFromNewCrop(GetAPISubkey(), GetAPILoginToken(), (_bstr_t)strPatID, GetAPI()->GetNewCropUserSettings(GetAPISubkey(), GetAPILoginToken()), pSkipReconcile);
			}


			if (newPrescriptions) {
				Nx::SafeArray<IUnknown *> saryPrescriptions(newPrescriptions->Medications);

				foreach(NexTech_Accessor::_NewCropPatientMedicationPtr pPrescription, saryPrescriptions)
				{
					if (pPrescription){
						NewCropPatientMedication med;
						////(r.wilson 3/1/2013) pl 49625 - Now we get the ncPrescriptionGUID as well
						med.strPrescriptionGUID = CString((LPCTSTR)pPrescription->ncPrescriptionGUID);
						med.strPrescriptionDate = CString((LPCTSTR)pPrescription->ncPrescriptionDate);
						med.strDrugID = CString((LPCTSTR)pPrescription->ncDrugID);
						med.strDrugTypeID = CString((LPCTSTR)pPrescription->ncDrugTypeID);
						med.strDrugName = CString((LPCTSTR)pPrescription->ncDrugName);
						med.strDrugInfo = CString((LPCTSTR)pPrescription->ncDrugInfo);
						med.strStrength = CString((LPCTSTR)pPrescription->ncStrength);
						med.strStrengthUOM = CString((LPCTSTR)pPrescription->ncStrengthUOM);
						med.strDosageNumberDescription = CString((LPCTSTR)pPrescription->ncDosageNumberDescription);
						med.strDosageForm = CString((LPCTSTR)pPrescription->ncDosageForm);
						med.strRoute = CString((LPCTSTR)pPrescription->ncRoute);
						med.strDosageFrequencyDescription = CString((LPCTSTR)pPrescription->ncDosageFrequencyDescription);
						med.strDispense = CString((LPCTSTR)pPrescription->ncDispense);
						med.strDispenseAsWritten = CString((LPCTSTR)pPrescription->ncDispenseAsWritten);
						med.strRefills = CString((LPCTSTR)pPrescription->ncRefills);
						med.strPrescriptionNotes = CString((LPCTSTR)pPrescription->ncPrescriptionNotes);
						med.strExternalPhysicianID = CString((LPCTSTR)pPrescription->ncExternalPhysicianID);
						med.strExternalSource = CString((LPCTSTR)pPrescription->ncExternalSource);
						med.strDEAClassCode = CString((LPCTSTR)pPrescription->ncDEAClassCode);
						med.strFinalDestinationType = CString((LPCTSTR)pPrescription->ncFinalDestinationType);
						med.strFinalStatusType = CString((LPCTSTR)pPrescription->ncFinalStatusType);
						med.strRXcui = CString((LPCTSTR)pPrescription->ncRXCUI);

						m_aNewlyAddedPatientPrescriptions.Add(med);
					}
				}
			}
		}
		// (r.farnworth 2015-07-27 14:24) - PLID 64583 - Change NxCatchAll into a catch statement so we can disect its elements
		catch (_com_error &e) {
			if (HandleAPIComError(e)) {
				Log("CNewCropBrowserDlg::OnUpdatePatientAccount Error: Unknown error  %li: %s (%s)", e.Error(), e.ErrorMessage(), (LPCTSTR)e.Description());
			}
			else {
				throw;
			}
		}
	} NxCatchAllCall(__FUNCTION__, {
		AfxMessageBox("A problem occurred while trying to update prescriptions or allergies in Practice.\n"
			"Please contact NexTech Technical Support for assistance.");
	}
	);
}

// (j.gruber 2009-05-15 14:13) - PLID 28541 - moved this to its own function since I wanted to reuse it
// (j.gruber 2009-05-15 17:38) - PLID 28541 - added ability to change route page
// (j.jones 2013-04-16 17:00) - PLID 56275 - the requested page enum is now in the API
BOOL CNewCropBrowserDlg::GeneratePatientLoginXML(CString &strXmlDocument, NexTech_Accessor::NewCropRequestedPageType ncrptRequestedPage, BOOL bAddNxScriptFooter /*= TRUE*/) {

	if(m_nPatientPersonID == -1) {
		AfxMessageBox("No patient is currently selected. The e-prescribing account login will be cancelled.");
		//because we are not processing anything, return OnCancel
		CNxDialog::OnCancel();

		::DestroyWindow(GetSafeHwnd());
		return FALSE;
	}

	// (j.jones 2013-01-03 16:24) - PLID 49624 - This login function now uses the API.
	// The loading of the API is now wrapped in its own exception handler so we know
	// specifically when the load fails, and can give an exact message indicating that
	// it failed right here. If it fails, it will end the login process gracefully
	// once the exception is dismissed.
	NexTech_Accessor::_PracticeMethods *pAPI = NULL;

	try {
		pAPI = GetAPI();
	}NxCatchAllCall("Error accessing the API in CNewCropBrowserDlg::GeneratePatientLoginXML", 
	{
		//close the dialog
		CNxDialog::OnCancel();

		::DestroyWindow(GetSafeHwnd());
		return FALSE;
	});

	// (j.jones 2013-01-03 16:24) - PLID 49624 - We loaded the API successfully.
	// Now we get the user settings from the API.
	NexTech_Accessor::_NewCropUserSettingsPtr pUserSettings = NULL;
	try {
		try {
			pUserSettings = pAPI->GetNewCropUserSettings(GetAPISubkey(), GetAPILoginToken());
		} catch (_com_error &e) {
			//this function will turn expected soap exceptions into messageboxes,
			//return TRUE if it did, FALSE if the error still needs handled
			if(!HandleAPIComError(e)) {
				throw e;
			}

			//close the window
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
	}NxCatchAllCall("Error calling GetNewCropUserSettings in CNewCropBrowserDlg::GeneratePatientLoginXML (2)",
	{
		//close the window
		CNxDialog::OnCancel();

		::DestroyWindow(GetSafeHwnd());
		return FALSE;
	});

	if(pUserSettings == NULL) {
		//shouldn't be possible without having an already-caught exception
		ThrowNxException("GetNewCropUserSettings failed to return a value from the API.");
	}

	//convert the user settings into more useful types

	NewCropUserTypes ncuType = ncutNone;
	if(atoi(pUserSettings->userType) > 0) {
		ncuType = (NewCropUserTypes)atoi(pUserSettings->userType);
	}

	long nLicensedPrescriberID = -1;
	if(atoi(pUserSettings->licensedPrescriberID) > 0) {
		nLicensedPrescriberID = atoi(pUserSettings->licensedPrescriberID);
	}

	NewCropProviderRoles eLicensedProvRole = ncprNone;
	if(atoi(pUserSettings->licensedPrescriberRole) > 0) {
		eLicensedProvRole = (NewCropProviderRoles)atoi(pUserSettings->licensedPrescriberRole);
	}
	
	long nMidlevelProviderID = -1;
	if(atoi(pUserSettings->midlevelProviderID) > 0) {
		nMidlevelProviderID = atoi(pUserSettings->midlevelProviderID);
	}

	NewCropProviderRoles eMidlevelProvRole = ncprNone;
	if(atoi(pUserSettings->midlevelProviderRole) > 0) {
		eMidlevelProvRole = (NewCropProviderRoles)atoi(pUserSettings->midlevelProviderRole);
	}

	long nSupervisingProviderID = -1;
	if(atoi(pUserSettings->supervisingProviderID) > 0) {
		nSupervisingProviderID = atoi(pUserSettings->supervisingProviderID);
	}

	NewCropProviderRoles eSupervisingProvRole = ncprNone;
	if(atoi(pUserSettings->supervisingProviderRole) > 0) {
		eSupervisingProvRole = (NewCropProviderRoles)atoi(pUserSettings->supervisingProviderRole);
	}
	
	// (j.jones 2011-06-17 10:57) - PLID 41709 - split out licensed prescriber and midlevel providers
	if(ncuType == ncutLicensedPrescriber) {

		if(nLicensedPrescriberID == -1) {
			AfxMessageBox("Your user account is configured to be a Licensed Prescriber in the E-Prescribing setup, but has no Licensed Prescriber selected.\n\n"
				"You must have a Licensed Prescriber selected for each Licensed Prescriber user in order to access Electronic Prescribing.");
			//because we are not processing anything, return OnCancel
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
		else if(eLicensedProvRole != ncprLicensedPrescriber) {
			// (j.jones 2011-06-20 09:01) - PLID 41709 - their role must be a licensed prescriber

			AfxMessageBox("Your user account is configured to be a Licensed Prescriber in the E-Prescribing setup, but the provider selected as a Licensed Prescriber "
				"does not have a Licensed Prescriber role assigned.\n\n"
				"Each provider must have a role assigned in the in the E-Prescribing setup before they can be used in Electronic Prescribing.");
			//because we are not processing anything, return OnCancel
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
		else {
			// (j.jones 2011-06-20 11:06) - PLID 44127 - Validate this provider's DEA number,
			// to make sure no other provider with a licensed prescriber role has the same DEA
			// number. Do not let them log in to NewCrop until this is resolved.
			CString strInvalidProviders = "";
			if(!EnsureUniqueDEANumber(strInvalidProviders, nLicensedPrescriberID)) {
				CString strWarning;
				strWarning.Format("Your user account is linked to a Licensed Prescriber provider that has the same "
					"DEA number of at least one other Licensed Prescriber:\n\n"
					"%s\n"
					"You cannot have two providers with the same DEA Number configured with the Licensed Prescriber role. "
					"Please edit either the E-Prescribing setup or the provider record in the Contacts module "
					"to ensure that no Licensed Prescribers have duplicate DEA Numbers.", strInvalidProviders);

				AfxMessageBox(strWarning);
				//because we are not processing anything, return OnCancel
				CNxDialog::OnCancel();

				::DestroyWindow(GetSafeHwnd());
				return FALSE;				
			}
		}
	}

	if(ncuType == ncutMidlevelProvider) {

		if(nMidlevelProviderID == -1) {
			AfxMessageBox("Your user account is configured to be a Midlevel Provider in the E-Prescribing setup, but has no Midlevel Provider selected.\n\n"
				"You must have a Midlevel Provider selected for each Midlevel Provider user in order to access Electronic Prescribing.");
			//because we are not processing anything, return OnCancel
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
		else if(eMidlevelProvRole != ncprMidlevelProvider) {
			// (j.jones 2011-06-20 09:01) - PLID 41709 - their role must be a midlevel provider

			AfxMessageBox("Your user account is configured to be a Midlevel Provider in the E-Prescribing setup, but the provider selected as a Midlevel Provider "
				"does not have a Midlevel Provider role assigned.\n\n"
				"Each provider must have a role assigned in the in the E-Prescribing setup before they can be used in Electronic Prescribing.");
			//because we are not processing anything, return OnCancel
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
	}

	if(IsNewCropSupervisedRole(ncuType)) {

		if(nSupervisingProviderID != -1 && eSupervisingProvRole != ncprLicensedPrescriber) {
			// (j.jones 2011-06-20 08:56) - PLID 41709 - their role must be a licensed prescriber, because
			// supervising providers are licensed prescribers, just sent in a different manner

			AfxMessageBox("Your user account is configured to be a Midlevel Prescriber or Staff in the E-Prescribing setup, "
				"but the provider selected as a Supervising Provider does not have a Licensed Prescriber role assigned.\n\n"
				"Each provider must have a role assigned in the in the E-Prescribing setup before they can be used in Electronic Prescribing.");
			//because we are not processing anything, return OnCancel
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
		else if(nSupervisingProviderID == -1) {
			//-1 is allowed here, it means we need to prompt the user

			CString strWhere;
			// (j.jones 2011-06-17 11:03) - PLID 41709 - now we exclude the midlevel provider ID,
			// and only filter on licensed prescriber roles 
			// (j.jones 2013-01-04 11:02) - PLID 49624 - this is now all done in the API
			NexTech_Accessor::_NewCropProvidersPtr pSupervisingProviderResult = NULL;
			try {
				try {
					pSupervisingProviderResult = pAPI->GetNewCropSupervisingProviders(GetAPISubkey(), GetAPILoginToken(), pUserSettings->midlevelProviderID);
				} catch (_com_error &e) {
					//this function will turn expected soap exceptions into messageboxes,
					//return TRUE if it did, FALSE if the error still needs handled
					if(!HandleAPIComError(e)) {
						throw e;
					}

					//close the window
					CNxDialog::OnCancel();

					::DestroyWindow(GetSafeHwnd());
					return FALSE;
				}
			}NxCatchAllCall("Error calling GetNewCropSupervisingProviders in CNewCropBrowserDlg::GeneratePatientLoginXML (2)",
			{
				//close the window
				CNxDialog::OnCancel();

				::DestroyWindow(GetSafeHwnd());
				return FALSE;
			});

			Nx::SafeArray<IUnknown *> sarySupervisingProviders(pSupervisingProviderResult->Providers);

			CMap<long, long, long, long> mapProviderIndexToID;
			CStringArray arySupervisingProviders;

			foreach(NexTech_Accessor::_NewCropProviderPtr pSupervisingProvider, sarySupervisingProviders) {
				long nProviderID = atoi((LPCTSTR)(pSupervisingProvider->GetID()));
				CString strProviderName = (LPCTSTR)(pSupervisingProvider->GetFullName());
				arySupervisingProviders.Add(strProviderName);
				mapProviderIndexToID.SetAt(arySupervisingProviders.GetSize()-1, nProviderID);
			}

			if(arySupervisingProviders.GetSize() == 0) {
				//give a context-sensitive warning as to why it is empty
				if(nMidlevelProviderID == -1) {
                    AfxMessageBox("A supervising provider could not be chosen because no providers are configured to be Licensed Prescribers the E-Prescribing setup.");
                }
                else {
                    AfxMessageBox("A supervising provider could not be chosen because no additional providers are configured to be Licensed Prescribers the E-Prescribing setup, aside from the current Midlevel Provider.");
                }

				//we cannot continue
				CNxDialog::OnCancel();

				::DestroyWindow(GetSafeHwnd());
				return FALSE;
			}

			CSingleSelectDlg dlg(this);
			//(e.lally 2011-08-12) PLID 37933 - Use the force selection option
			if(IDOK == dlg.Open(&arySupervisingProviders, "Select a Supervising Provider to submit prescriptions under:", true)) {

				// (j.jones 2011-06-20 08:56) - PLID 41709 - don't need to validate that their role is
				//a licensed prescriber, because this prompt was filtered on only licensed prescribers

				// (j.jones 2013-01-04 10:16) - PLID 49624 - The returned value is an index. Get the ID
				// from our map, assign the ID to our struct, and use the license provider role
				long nProviderIndex = dlg.GetSelectedID();
				mapProviderIndexToID.Lookup(nProviderIndex, nSupervisingProviderID);
				if(nSupervisingProviderID == -1) {
					//should be impossible
					ThrowNxException("Failed to load a supervising provider ID from chosen selection.");
				}
				eSupervisingProvRole = ncprLicensedPrescriber;
				pUserSettings->supervisingProviderID = (LPCTSTR)AsString(nSupervisingProviderID);
				pUserSettings->supervisingProviderRole = (LPCTSTR)AsString((long)eSupervisingProvRole);
			}
			else {
				AfxMessageBox("Your user account is configured to be a Midlevel Prescriber or Staff in the E-Prescribing setup.\n"
					"All Midlevel Prescriber and Staff users must select a Supervising Provider in order to access Electronic Prescribing.");
				//because we are not processing anything, return OnCancel
				CNxDialog::OnCancel();

				::DestroyWindow(GetSafeHwnd());
				return FALSE;
			}
		}

		// (j.jones 2011-06-20 11:06) - PLID 44127 - Validate this provider's DEA number,
		// to make sure no other provider with a licensed prescriber role has the same DEA
		// number. Do not let them log in to NewCrop until this is resolved.
		CString strInvalidProviders = "";
		if(!EnsureUniqueDEANumber(strInvalidProviders, nSupervisingProviderID)) {
			CString strWarning;
			strWarning.Format("The selected Supervising Provider has the same DEA number of at least one other Licensed Prescriber:\n\n"
				"%s\n"
				"You cannot have two providers with the same DEA Number configured with the Licensed Prescriber role. "
				"Please edit either the E-Prescribing setup or the provider record in the Contacts module "
				"to ensure that no Licensed Prescribers have duplicate DEA Numbers.", strInvalidProviders);

			AfxMessageBox(strWarning);
			//because we are not processing anything, return OnCancel
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;				
		}
	}

	CString strUserDefinedID;
	strUserDefinedID.Format("%li", m_nPatientUserDefinedID);

	// (j.jones 2013-01-03 17:07) - PLID 49624 - get the login XML from the API
	NexTech_Accessor::_NewCropPatientLoginXmlPtr pLoginXml = NULL;
	try {
		try {
			// (j.jones 2013-04-16 17:00) - PLID 56275 - the requested page enum is now in the API,
			// we have to build our passed-in parameter into a nullable pointer, because that's
			// the only way the API function can accept it
			NexTech_Accessor::_NullableNewCropRequestedPageTypePtr pPageType(__uuidof(NexTech_Accessor::NullableNewCropRequestedPageType));
			pPageType->SetNullableNewCropRequestedPageType(ncrptRequestedPage);

			pLoginXml = pAPI->GenerateNewCropPatientLoginXML(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strUserDefinedID), pUserSettings, pPageType);
		} catch (_com_error &e) {
			//this function will turn expected soap exceptions into messageboxes,
			//return TRUE if it did, FALSE if the error still needs handled
			if(!HandleAPIComError(e)) {
				throw e;
			}

			//close the window
			CNxDialog::OnCancel();

			::DestroyWindow(GetSafeHwnd());
			return FALSE;
		}
	}NxCatchAllCall("Error calling GenerateNewCropPatientLoginXML in CNewCropBrowserDlg::GeneratePatientLoginXML (2)",
	{
		//close the window
		CNxDialog::OnCancel();

		::DestroyWindow(GetSafeHwnd());
		return FALSE;
	});

	if(pLoginXml == NULL) {
		//should be impossible to get here without an already-handled exception
		ThrowNxException("Error in CNewCropBrowserDlg::GeneratePatientLoginXML: GenerateNewCropPatientLoginXML returned empty XML.");
	}

	strXmlDocument = (LPCTSTR)(pLoginXml->PatientLoginXml);

	return TRUE;
}


void CNewCropBrowserDlg::PerformDefaultAction()
{
	try {

		CWaitCursor pWait;

		// (j.jones 2009-08-13 14:31) - PLID 35213 - added a boolean to make sure the
		// default action function absolutely cannot be called twice in one browser window
		if(m_bHasPerformedDefaultAction) {
			//how could that have happened?
			ASSERT(FALSE);

			//do nothing, we should only be firing this function once
			return;
		}
		else {
			//track that we are now performing the default action
			m_bHasPerformedDefaultAction = TRUE;
		}

		switch(m_ncatDefaultAction) {
			
			case ncatAccessPatientAccount:
				{
					// (j.jones 2009-02-27 15:02) - PLID 33053 - GetNewCropRxEntryURL() will return the
					// preproduction or production URL, per the current NewCrop setup
					CString strURL = GetNewCropRxEntryURL();

					CString strXmlDocument;
					if (!GeneratePatientLoginXML(strXmlDocument, NexTech_Accessor::NewCropRequestedPageType_ncrptCompose)) {
						return;
					}				

#ifdef DEBUG
					CString strDebugXml = strXmlDocument;
					strDebugXml.Replace("%", "%%");
					CMsgBox dlg(this);
					dlg.msg = strDebugXml;
					dlg.DoModal();
#endif

					SendXMLToBrowser(strURL, strXmlDocument);
				}
				break;

			// (j.gruber 2009-05-15 14:16) - PLID 28541 - renewal requests
			case ncatProcessRenewalRequest:
				{

					CString strURL = GetNewCropRxEntryURL();

					CString strXmlDocument;
					// (j.jones 2013-04-16 17:00) - PLID 56275 - the requested page enum is now in the API
					if (!GeneratePatientLoginXML(strXmlDocument, NexTech_Accessor::NewCropRequestedPageType_ncrptRenewal, FALSE)) {
						return;
					}

					// (j.jones 2013-01-04 14:37) - PLID 49624 - we need to insert our passed-in XML
					// just before the end footer segment
					CString strXmlFooter;
					GenerateNCScriptFooter(strXmlFooter);
					long nFooterStart = strXmlDocument.Find(strXmlFooter);
					if(nFooterStart > 0) {
						strXmlDocument.Insert(nFooterStart, m_strPassedInXML);
					}
					else {
						//should be impossible
						ThrowNxException("Renewal XML is missing NCScript footer!");
					}

#ifdef DEBUG
					CString strDebugXml = strXmlDocument;
					strDebugXml.Replace("%", "%%");
					CMsgBox dlg(this);
					dlg.msg = strDebugXml;
					dlg.DoModal();
#endif

					SendXMLToBrowser(strURL, strXmlDocument);
				}
				break;
					
			
			/*
			case ncatSubmitPrescription:
				{
					// (j.jones 2009-02-27 15:02) - PLID 33053 - GetNewCropRxEntryURL() will return the
					// preproduction or production URL, per the current NewCrop setup
					CString strURL = GetNewCropRxEntryURL();

					CString strXmlDocument;
					if(!GenerateNewCropPrescriptionXML(strXmlDocument, m_nActionID)) {
						//Something failed in the XML generation, probably a bad parameter.
						AfxMessageBox("A warning or error occurred that cancelled submitting this prescription.");
						return;
					}

#ifdef DEBUG
					CString strDebugXml = strXmlDocument;
					strDebugXml.Replace("%", "%%");
					MsgBox(strDebugXml);
#endif

					SendXMLToBrowser(strURL, strXmlDocument);
				}
				break;
			*/

			case ncatNewBrowserWindow:
				{
					//this is called when a new browser window is opened, but we will have already
					//handled everything in OnInitDialog by checking m_bIsPopupWindow, so we
					//don't really need to do anything here
					ASSERT(m_bIsPopupWindow);		
				}
				break;

			case ncatInvalid:
			default:
				{
					//do nothing
					COleVariant varUrl("about:blank");

					if (m_pBrowser) {
						m_pBrowser->Navigate2(varUrl, NULL, NULL, NULL, NULL);
					}
				}
				break;			
		}
	
	}NxCatchAll("Error in CNewCropBrowserDlg::PerformDefaultAction");
}

// (a.walling 2009-04-07 13:20) - PLID 33306 - const CString reference
void CNewCropBrowserDlg::SendXMLToBrowser(CString strURL, const CString &strXmlDocument)
{
	try {

		ASSERT(!strURL.IsEmpty());
		ASSERT(!strXmlDocument.IsEmpty());

		// (j.jones 2009-09-10 12:30) - PLID 35511 - Log this XML
		// (d.thompson 2010-10-05) - PLID 40800 - This should have been fixed to not log on client use.  Using same
		//	solution as PLID 38671
		DEBUGLOG("NewCrop SendXMLToBrowser sending:\r\n%s", strXmlDocument);

		// (j.gruber 2009-03-02 16:25) - PLID 33273 - added XML Schema Validation
		if (! ValidateNewCropXMLSchemas(strXmlDocument)) {

			// (j.jones 2009-09-10 12:30) - PLID 35511 - Log that the sent XML failed
			Log("***NewCrop SendXMLToBrowser XML failed validation!***");

			MsgBox("An error in validation occurred that cancelled submitting this patient.");
			OnCancel();
			return;
		}
		
		COleVariant varUrl(strURL);

		CString strPostInfo = "RxInput=" + strXmlDocument;

		COleVariant varHeaders("Content-Type: application/x-www-form-urlencoded");
		
		// Fill the variant array
		COleSafeArray sa;
		BYTE* pData = NULL;
		sa.CreateOneDim(VT_UI1, strPostInfo.GetLength());
		sa.AccessData((LPVOID *)&pData);
		memcpy(pData, (LPCTSTR)strPostInfo, strPostInfo.GetLength());
		sa.UnaccessData();

		if (m_pBrowser) {
			m_pBrowser->Navigate2(varUrl, NULL, NULL, sa, varHeaders);
		}

	}NxCatchAll("Error in CNewCropBrowserDlg::SendXMLToBrowser");
}

BEGIN_EVENTSINK_MAP(CNewCropBrowserDlg, CNxDialog)
ON_EVENT(CNewCropBrowserDlg, IDC_NEWCROP_BROWSER, 250, OnBeforeNavigate2NewcropBrowser, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CNewCropBrowserDlg, IDC_NEWCROP_BROWSER, 252, OnNavigateComplete2NewcropBrowser, VTS_DISPATCH VTS_PVARIANT)
ON_EVENT(CNewCropBrowserDlg, IDC_NEWCROP_BROWSER, 259, OnDocumentCompleteNewcropBrowser, VTS_DISPATCH VTS_PVARIANT)
ON_EVENT(CNewCropBrowserDlg, IDC_NEWCROP_BROWSER, 251, OnNewWindow2, VTS_PDISPATCH VTS_PBOOL)
END_EVENTSINK_MAP()

void CNewCropBrowserDlg::OnBeforeNavigate2NewcropBrowser(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel)
{
	try {

		if(URL != NULL) {
			_variant_t varUrl = URL;
			if(varUrl.vt == VT_BSTR && VarString(varUrl) != "about:blank") {
				m_editBrowserStatus.SetWindowText("Loading Patient Account...");
			}
		}

	}NxCatchAll("Error in CNewCropBrowserDlg::OnBeforeNavigate2NewcropBrowser");
}

void CNewCropBrowserDlg::OnNavigateComplete2NewcropBrowser(LPDISPATCH pDisp, VARIANT* URL)
{
	try {

	}NxCatchAll("Error in CNewCropBrowserDlg::OnNavigateComplete2NewcropBrowser");
}

void CNewCropBrowserDlg::OnDocumentCompleteNewcropBrowser(LPDISPATCH pDisp, VARIANT* URL)
{
	try {

		if(URL != NULL) {
			_variant_t varUrl = URL;
			if(varUrl.vt == VT_BSTR && VarString(varUrl) != "about:blank") {
				m_editBrowserStatus.SetWindowText("Page Load Complete");
			}
		}

	}NxCatchAll("Error in CNewCropBrowserDlg::OnDocumentCompleteNewcropBrowser");
}

void CNewCropBrowserDlg::OnNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	try {

		//we're popping up a new browser window

		// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
		CNewCropBrowserDlg *pNewPopup = new CNewCropBrowserDlg(-1, -1, -1, NULL, "", this);
		
		//set our action to simply be a new window, and transfer our current description over
		pNewPopup->m_ncatDefaultAction = ncatNewBrowserWindow;
		pNewPopup->m_strDefaultWindowText = m_strDefaultWindowText;
		pNewPopup->m_bIsPopupWindow = TRUE;

		//now create the window and register it as our popup
		pNewPopup->Create(IDD_NEWCROP_BROWSER_DLG, this);
		pNewPopup->m_pBrowser->put_RegisterAsBrowser(TRUE);
		pNewPopup->m_pBrowser->get_Application(ppDisp);

		//display the window
		pNewPopup->ShowWindow(SW_SHOW);

		//track this window
		m_paryPopupWindows.Add(pNewPopup);

	}NxCatchAll("Error in CNewCropBrowserDlg::OnNewWindow2");
}

// (j.jones 2009-03-04 12:48) - PLID 33332 - added OnPostShowWindow to be called
// after the dialog has loaded
LRESULT CNewCropBrowserDlg::OnPostShowWindow(WPARAM wParam, LPARAM lParam)
{
	try {	

		//before we do anything, see if we are using the production account with a license key of 70116 or 72977, or in debug mode
		//if so, warn the user (clearly NexTech) that they should not do this
		// (j.jones 2010-06-24 15:47) - PLID 39013 - production status is now calculated by GetNewCropIsProduction(),
		// however if you manage to enable your production status on an internal db, keep these warnings!
		// (j.jones 2010-08-27 08:33) - PLID 40230 - At NewCrop's demand, we made the 72977 license key default to production,
		// and not warn at all. Per NewCrop, only development should use pre-production. Sales should just prescribe to a dummy
		// pharmacy.
		BOOL bIsProduction = GetNewCropIsProduction();
		int nLicenseKey = g_pLicense->GetLicenseKey();
		BOOL bIsDevelopmentLicense = (nLicenseKey == 70116);
		if(bIsProduction) {			
			if(bIsDevelopmentLicense) {
				if(IDNO == MessageBox("You are attempting to access a production server using the 70116 test license.\n"
					"You must be very careful in using this live, production account.\n\n"
					"Are you sure you want to continue using the production server?", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

					//leave this screen
					CNxDialog::OnCancel();

					::DestroyWindow(GetSafeHwnd());
					return 0;
				}
			}
			else {
				#ifdef DEBUG
				if(IDNO == MessageBox("You are attempting to access a production server using the a debug build.\n"
					"You must be very careful in using this live, production account.\n\n"
					"Are you sure you want to continue using the production server?", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

					//leave this screen
					CNxDialog::OnCancel();

					::DestroyWindow(GetSafeHwnd());
					return 0;
				}
				#endif
			}
		}
		else { //pre-production
			// (j.jones 2010-06-24 15:47) - PLID 39013 - warn if using pre-production on a non-developer license,
			// even if in debug mode
			if(!bIsDevelopmentLicense) {
				if(IDNO == MessageBox("You are attempting to access a pre-production server. "
					"This server is NOT SECURE, and should NEVER be used with live patient data.\n"
					"It is not recommended to use this server unless directed to do so by NexTech.\n\n"
					"If you feel this message is in error, please contact NexTech to have your account switched to production.\n\n"
					"Are you SURE you want to continue using the pre-production server?", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

					//leave this screen
					CNxDialog::OnCancel();

					::DestroyWindow(GetSafeHwnd());
					return 0;
				}
			}
		}

		PerformDefaultAction();

	}NxCatchAll("Error in CNewCropBrowserDlg::OnPostShowWindow");

	return 0;
}

// (j.jones 2010-01-21 17:03) - PLID 37004 - added ability to calculate the NDC number,
// using the NewCropID as the FirstDataBank ID
// (j.jones 2010-01-26 17:40) - PLID 37078 - moved to FirstDataBankUtils
//CString ChooseNDCNumberFromFirstDataBank(NewCropPatientMedication *pMed, CString strFullDescription);

// (j.jones 2013-01-04 13:46) - PLID 49624 - added special handling for when we access the API
// to handle the NewCrop login, this function will turn expected soap exceptions into messageboxes,
// return TRUE if it did, FALSE if the error still needs handled
BOOL CNewCropBrowserDlg::HandleAPIComError(_com_error &e)
{
	CString strDetailXml = CLRUtils::GetCLRSoapExceptionDetail(e);
	if(strDetailXml.IsEmpty()) {
		//there was no detail XML in this error
		return FALSE;
	}

	//parse the XML
	MSXML2::IXMLDOMDocument2Ptr pDocument(__uuidof(MSXML2::DOMDocument60));
	if (VARIANT_TRUE != pDocument->loadXML((LPCTSTR)strDetailXml)) {
		//this was not valid XML
		return FALSE;
	}

	CString strXMLResult = PrettyPrintXML(pDocument);

	MSXML2::IXMLDOMNodeListPtr pNodes = pDocument->childNodes;
	if(pNodes) {
		for (int n=0; n < pNodes->Getlength(); n++) {
			MSXML2::IXMLDOMNodePtr pDetailNode = pNodes->Getitem(n);
			if(pDetailNode != NULL) {
				CString strNodeName = (LPCTSTR)pDetailNode->nodeName;
				if(strNodeName == "detail") {
					//we've found the <detail> node, now extract its error
					MSXML2::IXMLDOMNodePtr pErrorNode = pDetailNode->firstChild;
					if(pErrorNode) {
						CString strErrorType = (LPCTSTR)pErrorNode->nodeName;

						//make sure this is a NexTech Exception, they always begin with nxn:
						if(strErrorType.Left(4).CompareNoCase("nxn:") != 0) {
							//This is not a NexTech Exception and should not be turned into a messagebox.
							//ASSERT so we can see the contents of strXMLResult before declaring failure.
							ASSERT(FALSE);
							return FALSE;
						}

						//confirm this is a NewCrop exception type
						if(strErrorType.CompareNoCase("nxn:NewCropSetupWarning") != 0
							&& strErrorType.CompareNoCase("nxn:NewCropInvalidContentWarning") != 0
							&& strErrorType.CompareNoCase("nxn:NewCropSOAPFailure") != 0) {

							//If we get here, this is a NexTech Exception, and probably should still be a
							//messagebox (and we will proceed as though it should be), but this code isn't
							//expecting anything but a NewCrop exception.
							//ASSERT, because perhaps one day NexTech Exceptions might need more handling
							//than just messageboxes. If the found exception type should still be a
							//messagebox, then just add it to the above list.
							ASSERT(FALSE);
						}

						//now get the remaining contents of the NexTech Exception

						CString strWarningMessage = "";
						CString strErrorCode = "";
						MSXML2::IXMLDOMNamedNodeMapPtr pAttributes = pErrorNode->Getattributes();
						for (int a=0; a < pAttributes->Getlength(); a++) {
							MSXML2::IXMLDOMNodePtr pAttribute = pAttributes->Getitem(a);
							CString strAttributeName = (LPCTSTR)pAttribute->GetbaseName();
							if(strAttributeName.CompareNoCase("WarningMessage") == 0 || strAttributeName.CompareNoCase("ErrorMessage") == 0) {
								strWarningMessage = (LPCTSTR)pAttribute->Gettext();
							}
							else if(strAttributeName.CompareNoCase("ErrorCode") == 0) {
								strErrorCode = (LPCTSTR)pAttribute->Gettext();
							}
						}

						//we need a warning message
						if(strWarningMessage.IsEmpty()) {
							//The API must have failed to generate this error correctly.
							//Check the contents of strXMLResult and find the code in the API that
							//threw this error, and figure out why it failed.
							ASSERT(FALSE);
							return FALSE;
						}
						
						//We currently don't use the error code, but assert if we didn't find it,
						//because every NexTech Exception ought to have one.
						ASSERT(strErrorCode.GetLength() > 0);

						AfxMessageBox(strWarningMessage);

						//Return TRUE to indicate we handled the error with a message box.
						//In most cases, the caller should be closing the dialog after calling this function.
						return TRUE;
					}
				}
			}
		}
	}

	//If we got here, we had valid XML, but it wasn't an error type we can handle.
	//But currently this code should handle all error types, so what changed?
	//ASSERT to find out what the contents of strXMLResult is.
	ASSERT(FALSE);
	return FALSE;
}