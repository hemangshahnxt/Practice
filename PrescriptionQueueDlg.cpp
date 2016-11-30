// PrescriptionQueueDlg.cpp : implementation file
//

// (j.fouts 2012-11-01 16:28) - PLID 53566 - Created

#include "stdafx.h"
#include "Practice.h"
#include "PrescriptionQueueDlg.h"
#include <NxUILib/WindowUtils.h>	// (a.wilson 2012-10-24 10:44) - PLID 51711
#include "PrescriptionEditDlg.h"
#include "MedicationSelectDlg.h"
#include "EditMedicationListDlg.h"
#include "PrescriptionTemplateSetupDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "MergeEngine.h"
#include "FirstDataBankUtils.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
// (j.fouts 2012-12-26 11:49) - PLID 54340 - Support newcrop if they do not have our ePrescribing
#include "NewCropBrowserDlg.h"
#include "NewCropUtils.h"
// (b.savon 2013-01-07 09:15) - PLID 54461 - Moved from medications dlg
#include "FavoritePharmaciesEditDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "ReconcileMedicationsUtils.h"
#include "PharmacyDirectorySearchDlg.h"
#include "PatientView.h"
#include "AssignPatientRenewalDlg.h"
#include "DecisionRuleUtils.h"
#include "NexERxQuickListDlg.h"
#include "NexERxSetupDlg.h"
#include "DrugInteractionDlg.h"
#include "EditAllergyListDlg.h" // (b.savon 2013-03-01 12:54) - PLID 54704 - Edit allergy
#include <NxAlgorithm.h>	// (a.wilson 2013-04-30 17:57) - PLID 56509 - include for boost::exists()
#include "MedicationDlg.h"
#include "AuditTrail.h"
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "NexFormularyDlg.h" // (b.savon 2013-07-16 14:11) - PLID 57587
#include "PrescriptionUtilsAPI.h"
#include "MedicationHistoryDlg.h" // (r.gonet 09/20/2013) - PLID 58416
#include "MedicationHistoryUtils.h" // (r.gonet 09/20/2013) - PLID 58396
#include "MedlinePlusUtils.h"
#include "ProgressDialog.h"
#include "PatientsRc.h"
#include "MultiSelectComboController.h"
#include "ErxPrescriptionReviewDlg.h"

using namespace ADODB;

// (b.savon 2013-03-08 13:12) - PLID 55518 - Changed everywhere to use EMNSpawnSource, Provider, Supervisor, NurseStaff objects.

// (j.fouts 2012-11-08 10:33) - PLID 53574 - Showing these 
/*****IF YOU CHANGE THESE, ALSO CHANGE THEM IN EMNMoreInfoDlg.cpp as well as medicationdlg.cpp******/
#define EPRESCRIBE_COLOR_DISCONTINUED RGB(222, 225, 231)
#define EPRESCRIBE_COLOR_ACTIVE RGB(178, 251, 197)

// (j.jones 2008-06-09 14:32) - PLID 29154 - added standard "default template" define
#define USE_DEFAULT_TEMPLATE_TEXT "<Use Default Template>"

// (j.fouts 2012-11-30 10:07) - PLID 53954 - Id's for the right click commands
#define IDM_DELETE_PRESCRIPTION 41765
#define IDM_CHECK_SEND			41772
#define IDM_CHECK_PRINT			41773
#define IDM_MARK_INCOMPLETE		41774
#define IDM_MARK_PRINTED		41775
#define IDM_DELETE_CURRENT_MED	41776 // (b.savon 2013-01-14 12:33) - PLID 54592 - Delete current med
#define IDM_DELETE_CURRENT_ALLERGY 41777 // (b.savon 2013-01-14 17:38) - PLID 54704 - Delete current allergy
#define IDM_DISCONTINUE_CURRENT_MED 41778 // (b.savon 2013-01-25 12:30) - PLID 54854 - Discontinue current med/allergy
#define IDM_DISCONTINUE_CURRENT_ALLERGY 41779
#define IDM_HOLD_PRESCRIPTION	41780
// (b.savon 2013-06-19 16:54) - PLID 56880 - Monograph and Leaflets
#define IDM_SHOW_MONO			41781
#define IDM_SHOW_LEAFLET		41782
// (b.savon 2013-09-04 11:11) - PLID 58212 - Add a new 'Void' type for Prescriptions
#define IDM_VOID_PRESCRIPTION	41783
#define IDM_DOCTOR_REVIEW_PRESCRIPTION 41784
// (r.farnworth 2016-01-08 15:37) - PLID 58692 - Wants to be able to quickly re-prescribe a prescription. Perhaps a right click and "Re-send" or "Re-Prescribe" (This functionality was in NewCrop)
#define IDM_REPRESCRIPE	41785 
// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
#define IDM_PRESCRIPTION_DISPENSED_IN_HOUSE 41786



// (b.savon 2013-01-23 18:19) - PLID 54782
#define IDT_HIDE_QUICK_LIST_RESULTS 1033
#define IDT_HIDE_QUICK_LIST_SUPERVISOR_RESULTS 1034

// (b.savon 2013-01-29 18:16) - PLID 54919
// (b.savon 2013-01-29 18:16) - PLID 54920
#define IDT_START_SEARCH_ALLERGIES 1036
// (s.tullis 2016-01-28 17:45) - PLID 67965
// (s.tullis 2016-01-28 17:46) - PLID 68090
#define IDT_SAVE_REMINDER          1037



// (j.fouts 2013-01-21 12:26) - PLID 54719 - Added a preferance on when to show these columns
#define SHOW_SUPERVISOR			0x1
#define SHOW_NURSESTAFF			0x2

extern CPracticeApp theApp;

enum AllergyListColumns {
	alcID = 0,
	alcEnteredDate = 1,
	alcName = 2,
	aldNotes = 3,
	alcFromNewCrop = 4,
	alcIsDiscontinued = 5,
	alcRXCUI = 6,
	alcFDBImport = 7,
	alcFDBOutOfDate = 8, //TES 5/9/2013 - PLID 56614
	alcLastUpdateDate = 9, // (s.dhole 2013-07-05 15:47) - PLID 56931
};

enum CurMedListColumns {
	cmlcID = 0,
	cmlcPatientID,
	cmlcMedName,
	cmlcInfoButton,		// (j.jones 2013-10-17 15:27) - PLID 58983 - added infobutton and medication ID
	cmlcMedicationID,
	cmlcEMRDataID,
	cmlcNewCropGUID,
	cmlcDiscontinued,
	cmlcSig,
	cmlcFDBID,
	cmlcFDBOutOfDate,	//TES 5/9/2013 - PLID 56614
	cmlcStartDate ,	// (s.dhole 2013-06-06 13:10) - PLID 56926
	cmlcLastUpdateDate ,	// (s.dhole 2013-07-05 15:51) - PLID 56926
};

// (j.fouts 2012-09-26 10:32) - PLID 52973 - Columns for the prescription queue datalist
// (s.dhole 2013-01-10 15:56) - PLID 53421 added pqcErrorDescription
// (j.fouts 2013-01-14 10:40) - PLID 54464 - Added Pharmacy
enum PrescriptionQueueColumns {
	pqcPrescriptionID = 0,
	pqcMedID,
	pqcMedErrorInd, // (s.dhole 2012-10-26 ) - PLID 53421
	pqcMedErrorIndVal, // (s.dhole 2012-10-26 ) - PLID 53421
	pqcSend,
	pqcPrint,
	pqcMedName,
	pqcStatus,
	// (j.fouts 2013-01-21 12:26) - PLID 54719 - Added A Patinet UserDefined ID column
	pqcPatientUserDefiniedID,
	// (j.fouts 2012-10-22 11:41) - PLID 53156 - Added a new column for Patient Name
	pqcPatient,
	pqcCreationDate,
	pqcStatusID,
	pqcErrorDescription,
	pqcPharmacy,
	pqcPharmacyPhone,	// (j.fouts 2013-04-22 14:12) - PLID 54719 - Added Pharmacy Phone #
	// (j.fouts 2013-01-21 12:26) - PLID 54719 - Added columns for prescriber info
	pqcPrescriberID,
	pqcPrescriber,
	pqcSupervisorID,
	pqcSupervisor,
	pqcNurseStaffID,
	pqcNurseStaff,
	pqcDenyNewRxResponseID,
	pqcNewCropGUID,
	pqcMedDescription, // (s.dhole 2013-04-16 09:35) - PLID 54026 
	pqcEnglishSig,	// (j.fouts 2013-04-22 14:12) - PLID 54719 - Added English Sig
	pqcFDBID, // (b.savon 2013-06-20 10:49) - PLID 56880
};

// (a.wilson 2012-11-05 11:48) - PLID 53797
enum RenewalRequestColumns {
	rrcRenewalID = 0,
	rrcPrescription = 1,
	rrcResponseStatusID = 2,
	rrcResponseReason = 3,
	rrcResponseStatus = 4,
	rrcETransmitStatusID = 5,
	rrcETansmitReason = 6,
	rrcETransmitStatus = 7,
	rrcPatientID = 8,
	rrcPatient = 9,
	rrcDate = 10,
	rrcPharmacy = 11,
	rrcPrescriberID = 12,
	rrcPrescriber = 13,	
};

// (b.eyers 2016-02-12) - PLID 67989 - added description
enum RenewalRequestFilterColumns {
	rrfcID = 0,
	rrfcName = 1,
	rrfcDescription = 2,
};

// (j.fouts 2012-09-26 10:32) - PLID 52973 - Background colors of the queue
enum QueueBackgroundColors {
	qbcIncomplete = RGB(255,230,230),
	qbcCompleted = RGB(230,255,230),
	qbcPrinted = RGB(240,240,240),
	qbcESubmitted = RGB(240,240,240),
};

// (b.eyers 2016-02-12) - PLID 67989 - added description
enum QueueStatusFilterColumns {
	qsfcID = 0,
	qsfcStatus = 1,
	qsfcDescription = 2,
};

// (b.savon 2013-01-25 09:06) - PLID 54846
enum PrescriberFilterColumns{
	pfcID = 0,
	pfcName,
};

// (s.dhole 2012-11-27 15:41) - PLID 54890
enum RxRxReadyForSureScrip{
	elNotValidForSureScript = 0,
	elValidForSureScript  = 1,
//	dlcValidForSureScriptWithWarning  = 2,
	
};

// (b.savon 2013-01-23 12:54) - PLID 54782
// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added route and unit columns
enum EQuickListColumns{
	qlcID = 0,
	qlcOrderIndex, // (r.gonet 2016-02-10 13:34) - PLID 58689
	qlcCheck,
	qlcName,
	qlcDrugListID,
	qlcFDBID,  // (b.savon 2013-08-23 14:43) - PLID 58236
	qlcRefill,
	qlcQuantity,
	qlcSig,
	qlcDosageRouteID,
	qlcDosageRoute,
	qlcDosageFrequency,
	qlcDosageQuantity,
	qlcDosageUnitID,
	qlcDosageUnit,
	qlcNotes,
	qlcIcon, // (b.savon 2013-08-23 13:32) - PLID 58236 - Renamed
};


// (j.fouts 2012-11-06 12:21) - PLID 53574 - Made this more extendable
CCollapseableBox::CCollapseableBox(CWnd* pParent, bool bCollapsed /*=false*/)
{
	m_bCollapsed = bCollapsed;
	m_pParent = pParent;
	m_nHeaderHeight = 0;
	m_nFooterHeight = 0;
}

// (j.fouts 2012-11-06 12:22) - PLID 53574 - Binds the controls
void CCollapseableBox::BindControls(CNxColor* pColorBkg, CNxStatic* pLabel /*=NULL*/, CNxIconButton* pIconButton /*=NULL*/)
{
	//Calculate its starting size
	pColorBkg->GetWindowRect(&m_rectSize);
	m_pParent->ClientToScreen(&m_rectSize);

	m_pLabel = pLabel;
	m_pColorBkg = pColorBkg;
	m_pIconButton = pIconButton;

	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Bind all the controls
	foreach(int nCtrlID, m_aryBodyControls)
	{
		CRect rectCtrl;
		m_pParent->GetDlgItem(nCtrlID)->GetWindowRect(&rectCtrl);
		m_pColorBkg->ScreenToClient(&rectCtrl);
		m_aryRelativePositions.SetAt(nCtrlID, rectCtrl);
	}

	long nBottomMost = 0;
	foreach(int nCtrlID, m_aryHeaderControls)
	{
		CRect rectCtrl;
		m_pParent->GetDlgItem(nCtrlID)->GetWindowRect(&rectCtrl);
		m_pColorBkg->ScreenToClient(&rectCtrl);
		m_aryRelativePositions.SetAt(nCtrlID, rectCtrl);
		if(nBottomMost < rectCtrl.bottom)
		{
			nBottomMost = rectCtrl.bottom;
		}
	}

	long nTopMost = m_rectSize.Height();
	foreach(int nCtrlID, m_aryFooterControls)
	{
		CRect rectCtrl;
		m_pParent->GetDlgItem(nCtrlID)->GetWindowRect(&rectCtrl);
		m_pColorBkg->ScreenToClient(&rectCtrl);
		m_aryRelativePositions.SetAt(nCtrlID, rectCtrl);
		if(nTopMost > rectCtrl.top)
		{
			nTopMost = rectCtrl.top;
		}
	}
	m_nHeaderHeight = nBottomMost; 
	m_nFooterHeight = m_rectSize.Height() - nTopMost;
}

// (j.fouts 2012-11-06 12:22) - PLID 53574 - Expose m_bCollapsed
bool CCollapseableBox::IsCollapsed()
{
	return m_bCollapsed;
}

// (j.fouts 2012-11-06 12:22) - PLID 53574 - Adds a control to the specified location
// (j.fouts 2012-11-14 11:42) - PLID 53439 - Controls positions are now relative to the size of the collapsable box
void CCollapseableBox::AddHeaderControl(int nCtrl)
{
	m_aryHeaderControls.Add(nCtrl);
}

void CCollapseableBox::AddBodyControl(int nCtrl)
{
	m_aryBodyControls.Add(nCtrl);
}

void CCollapseableBox::AddFooterControl(int nCtrl)
{
	m_aryFooterControls.Add(nCtrl);
}


// (j.fouts 2012-11-06 12:23) - PLID 53574 - Made this more extendable
//Repositions all the controls based on the size of the box (rectSize), and if the box is collapsed
// (j.fouts 2012-11-14 11:42) - PLID 53439 - Controls are now repositioned relative to their positions insid of the box
void CCollapseableBox::RepositionControls(CRect &rectSize)
{
	if(!m_bCollapsed)
	{
		//Determine the new scale of the box
		float fScaleX = ((float)rectSize.Width()) / m_rectSize.Width();
		float fScaleY = ((float)rectSize.Height() - m_nHeaderHeight - m_nFooterHeight) / (m_rectSize.Height() - m_nHeaderHeight - m_nFooterHeight);

		foreach(int nCtrlID, m_aryHeaderControls)
		{
			CRect rectControlSize = m_aryRelativePositions[nCtrlID];

			m_pParent->GetDlgItem(nCtrlID)->MoveWindow(
				(int)(rectControlSize.left * fScaleX) + rectSize.left, 
				(rectControlSize.top) + rectSize.top, 
				(int)(rectControlSize.Width() * fScaleX), 
				(rectControlSize.Height()));

			//Enable and show the control
			m_pParent->GetDlgItem(nCtrlID)->EnableWindow(TRUE);
			m_pParent->GetDlgItem(nCtrlID)->ShowWindow(SW_SHOW);
		}

		foreach(int nCtrlID, m_aryFooterControls)
		{
			CRect rectControlSize = m_aryRelativePositions[nCtrlID];

			m_pParent->GetDlgItem(nCtrlID)->MoveWindow(
				(int)(rectControlSize.left * fScaleX) + rectSize.left, 
				rectSize.bottom - (m_rectSize.Height() - rectControlSize.top), 
				(int)(rectControlSize.Width() * fScaleX), 
				(rectControlSize.Height()));

			//Enable and show the control
			m_pParent->GetDlgItem(nCtrlID)->EnableWindow(TRUE);
			m_pParent->GetDlgItem(nCtrlID)->ShowWindow(SW_SHOW);
		}

		foreach(int nCtrlID, m_aryBodyControls)
		{
			//Get the relaticve position of the control when it was bound
			CRect rectControlSize = m_aryRelativePositions[nCtrlID];

			//Move the control, scaling its relative location/size accordingly
			m_pParent->GetDlgItem(nCtrlID)->MoveWindow(
				(int)(rectControlSize.left * fScaleX) + rectSize.left, 
				(int)((rectControlSize.top - m_nHeaderHeight) * fScaleY) + rectSize.top + m_nHeaderHeight, 
				(int)(rectControlSize.Width() * fScaleX), 
				(int)(rectControlSize.Height() * fScaleY));

			//Enable and show the control
			m_pParent->GetDlgItem(nCtrlID)->EnableWindow(TRUE);
			m_pParent->GetDlgItem(nCtrlID)->ShowWindow(SW_SHOW);
		}

		if(m_pLabel)
		{
			CRect rectControlSize;
			m_pLabel->GetWindowRect(&rectControlSize);
			//The label has a preset position, rather than relative
			m_pLabel->MoveWindow(rectSize.left + 12, rectSize.top + 3, rectControlSize.Width(), rectControlSize.Height());
		}
		if(m_pIconButton)
		{
			CRect rectControlSize;
			m_pIconButton->GetWindowRect(&rectControlSize);
			//The button has a preset position, rather than relative
			m_pIconButton->MoveWindow(rectSize.right - 16 - rectControlSize.Width(), rectSize.top, rectControlSize.Width(), rectControlSize.Height());
			//Update the icon and tooltip
			m_pIconButton->AutoSet(NXB_DOWN);
			m_pIconButton->SetToolTip("Collapse");
		}

		//The background will be the full rectangle
		m_pColorBkg->MoveWindow(rectSize);
	}
	else
	{
		//We are collapsed so hide the controls
		foreach(int nCtrlID, m_aryHeaderControls)
		{
			m_pParent->GetDlgItem(nCtrlID)->ShowWindow(SW_HIDE);
			m_pParent->GetDlgItem(nCtrlID)->EnableWindow(FALSE);
		}
		foreach(int nCtrlID, m_aryBodyControls)
		{
			m_pParent->GetDlgItem(nCtrlID)->ShowWindow(SW_HIDE);
			m_pParent->GetDlgItem(nCtrlID)->EnableWindow(FALSE);
		}
		foreach(int nCtrlID, m_aryFooterControls)
		{
			m_pParent->GetDlgItem(nCtrlID)->ShowWindow(SW_HIDE);
			m_pParent->GetDlgItem(nCtrlID)->EnableWindow(FALSE);
		}

		if(m_pLabel)
		{
			CRect rectControlSize;
			m_pLabel->GetWindowRect(&rectControlSize);
			//The label has a preset position, rather than relative
			m_pLabel->MoveWindow(rectSize.left + 12, rectSize.top + 3, rectControlSize.Width(), rectControlSize.Height());
		}
		if(m_pIconButton)
		{
			CRect rectControlSize;
			m_pIconButton->GetWindowRect(&rectControlSize);
			//The button has a preset position, rather than relative
			m_pIconButton->MoveWindow(rectSize.right - 16 - rectControlSize.Width(), rectSize.top, rectControlSize.Width(), rectControlSize.Height());
			//Update the icon and tooltip
			m_pIconButton->AutoSet(NXB_UP);
			m_pIconButton->SetToolTip("Expand");
		}
		
		//The background will be the full rectangle
		m_pColorBkg->MoveWindow(rectSize);
	}
}

// (j.fouts 2012-11-06 12:24) - PLID 53574 - Collapse/Expand the box
void CCollapseableBox::ToggleCollapsed()
{
	m_bCollapsed = !m_bCollapsed;
}

// (j.jones 2016-02-03 16:43) - PLID 68118 - added ForceCollapsed, ForceOpen
void CCollapseableBox::ForceCollapsed()
{
	m_bCollapsed = true;
}

void CCollapseableBox::ForceOpen()
{
	m_bCollapsed = false;
}

// (r.gonet 2016-01-22) - PLID 67967 - Creates a new CRxPrescriberFilterController object.
CPrescriptionQueueDlg::CRxPrescriberFilterController::CRxPrescriberFilterController(CWnd *pParent)
	: CMultiSelectComboController(pParent, "Provider", "Please select one or more prescribers:")
{
	SetSpecialRowID(ESpecialRow::All, _bstr_t("-1"), " < All Prescribers >");
	SetSpecialRowID(ESpecialRow::Multiple, _bstr_t("-2"), " < Multiple Prescribers >");
}

// (r.gonet 2016-01-22) - PLID 67967 - Loads the Rx prescriber combo with the prescriber rows.
void CPrescriptionQueueDlg::CRxPrescriberFilterController::LoadComboRows()
{
	CMultiSelectComboController::LoadComboRows();

	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"(
SELECT
	ProvidersT.PersonID AS ID,
	PersonT.FullName AS Name
FROM ProvidersT
INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID
WHERE COALESCE(ProvidersT.NexERxProviderTypeID, -1) <> -1
ORDER BY PersonT.FullName
)");
	while (!prs->eof) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->GetNewRow();
		_variant_t varID;
		switch (m_vtIDType) {
		case VT_I4:
			varID = _variant_t(AsLong(AdoFldVar(prs->Fields, "ID")), VT_I4);
			break;
		case VT_BSTR:
			varID = _variant_t(_bstr_t(AsString(AdoFldVar(prs->Fields, "ID"))));
			break;
		default:
			ThrowNxException("%s : Unhandled ID variant type.", __FUNCTION__);
		}
		pRow->PutValue(m_nIDColumnIndex, varID);
		pRow->PutValue(m_nDescriptionColumnIndex, _bstr_t(AdoFldString(prs->Fields, "Name")));
		m_pCombo->AddRowSorted(pRow, NULL);

		prs->MoveNext();
	}
}

// (r.gonet 2016-01-22) - PLID 67973 - Creates a new CRenewalPrescriberFilterController object.
CPrescriptionQueueDlg::CRenewalPrescriberFilterController::CRenewalPrescriberFilterController(CWnd *pParent)
	: CMultiSelectComboController(pParent, "Provider", "Please select one or more prescribers:")
{
	SetSpecialRowID(ESpecialRow::All, _bstr_t("-1"), " < All Prescribers >");
	SetSpecialRowID(ESpecialRow::Multiple, _bstr_t("-2"), " < Multiple Prescribers >");
}

// (r.gonet 2016-01-22) - PLID 67973 - Loads the Renewal prescriber combo with the prescriber rows.
void CPrescriptionQueueDlg::CRenewalPrescriberFilterController::LoadComboRows()
{
	CMultiSelectComboController::LoadComboRows();

	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"(
SELECT
	PersonT.ID,
	PersonT.FullName AS Name
FROM PersonT 
INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID 
INNER JOIN NexERxPrescriberRegistrationT ON ProvidersT.PersonID = NexERxPrescriberRegistrationT.ProviderID 
INNER JOIN NexERxPrescriberT ON NexERxPrescriberRegistrationT.SPI = NexERxPrescriberT.SPI
WHERE PersonT.Archived = 0
GROUP BY PersonT.ID, PersonT.FullName
)");
	while (!prs->eof) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->GetNewRow();
		_variant_t varID;
		switch (m_vtIDType) {
		case VT_I4:
			varID = _variant_t(AsLong(AdoFldVar(prs->Fields, "ID")), VT_I4);
			break;
		case VT_BSTR:
			varID = _variant_t(_bstr_t(AsString(AdoFldVar(prs->Fields, "ID"))));
			break;
		default:
			ThrowNxException("%s : Unhandled ID variant type.", __FUNCTION__);
		}
		pRow->PutValue(m_nIDColumnIndex, varID);
		pRow->PutValue(m_nDescriptionColumnIndex, _bstr_t(AdoFldString(prs->Fields, "Name")));
		m_pCombo->AddRowSorted(pRow, NULL);

		prs->MoveNext();
	}
}


// CPrescriptionQueueDlg dialog

IMPLEMENT_DYNAMIC(CPrescriptionQueueDlg, CNxDialog)

// (j.fouts 2012-11-06 12:17) - PLID 53574 - Added the boxes to the constructor
// (j.fouts 2012-11-15 10:36) - PLID 53573 - Added bEmbeded
// (j.jones 2012-11-20 10:16) - PLID 52818 - added optional default provider, location, and date,
// (j.fouts 2013-01-16 09:42) - PLID 51712 - Added an option to open to renewal dlg
// such that new prescriptions defaulted to these values
// (j.jones 2013-03-26 15:30) - PLID 53532 - renamed nEMNID to nCurrentlyOpenedEMNID to reflect what
// the variable really means, it's the ID of the currently opened EMN, such that creating new prescriptions
// would use this EMN ID
CPrescriptionQueueDlg::CPrescriptionQueueDlg(CWnd* pParent /*=NULL*/, long nPatientID /*=-1*/, long nCurrentlyOpenedEMNID /*=-1*/,
											 bool bShowMedsAllergies /*=true*/, bool bInMedicationsTab /*=false*/,
											 long nProviderID /*= -1*/, long nLocationID /*= GetCurrentLocationID()*/, COleDateTime dtDate /*= COleDateTime::GetCurrentTime()*/,
											 bool bOpenToRenewals /*=false*/)
	: CNxDialog(CPrescriptionQueueDlg::IDD, pParent),
	m_nPatientID(nPatientID),
	m_nPrescriptionID(-1),
	m_nCurrentlyOpenedEMNID(nCurrentlyOpenedEMNID),
	m_pendingBox(this),
	m_renewalsBox(this),
	m_medsAllergiesBox(this),
	m_bShowMedsAllergies(bShowMedsAllergies),
	m_bInMedicationsTab(bInMedicationsTab),
	// (j.jones 2013-04-05 14:18) - PLID 56114 - added tablechecker variables
	m_CurrentMedsChecker(NetUtils::CurrentPatientMedsT),
	m_PatientAllergiesChecker(NetUtils::PatientAllergyT),
	// (r.gonet 09/20/2013) - PLID 58396 - Added a table checker for medication history respones so we can update the button.
	m_MedicationHistoryResponseChecker(NetUtils::MedicationHistoryResponseT),
	// (r.gonet 2016-01-22) - PLID 67967 - Initialize the controller for the Rx Prescriber filter.
	m_rxPrescriberController(this),
	// (r.gonet 2016-01-22) - PLID 67973 - Initialize the controller for the Renewal Prescriber filter.
	m_renewalPrescriberController(this)
{
	try
	{
	
		m_bHasNoAllergies = FALSE;
		m_bHasNoMeds = FALSE;
		m_nCountPrintSelected = 0;
		m_nCountSendSelected = 0;
		m_nDefTemplateRowIndex = -1;
		// (r.gonet 08/19/2013) - PLID 58416 - Should we show the medication history button?
		m_bShowMedHistory = true;
		// (b.savon 2014-01-03 07:45) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr)
		// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
		m_bShowNexFormulary = true;
		// (j.fouts 2012-11-16 11:02) - PLID 53156 - Default to showing these
		m_bShowInteractions = true;
		m_bShowWrite = true;

		// (j.jones 2012-11-19 17:52) - PLID 52818 - added provider ID, location ID, and date, used for defaults for new prescriptions
		m_nDefaultProviderID = nProviderID;
		m_nDefaultLocationID = nLocationID;
		m_dtDefaultPrescriptionDate = dtDate;
		// (j.fouts 2012-11-20 15:24) - PLID 53840 - Added a way to hide renewals and sending
		m_bShowRenewals = true;
		m_bShowSend = true;
		// (a.wilson 2013-01-07 17:44) - PLID 54410
		m_bOpenToRenewals = false;
		m_urtCurrentuser = urtNone;
		m_bCachedIsLicensedConfiguredUser = FALSE;
		m_bRenewalFilterTimeChanged = false;
		// (b.savon 2013-08-23 13:46) - PLID 58236
		m_hIconArrow = NULL;
		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		m_hInfoButtonIcon = NULL;
		// (b.eyers 2016-01-21) - PLID 67966
		m_bDateFromDown = FALSE;
		m_bDateToDown = FALSE;

		// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
		m_bHasSureScriptsLicense = (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts);
	}
	NxCatchAll(__FUNCTION__);
}

CPrescriptionQueueDlg::~CPrescriptionQueueDlg()
{
	try
	{
		// (s.Dhole 2012-10-29 ) - PLID 54890 - Warning ICON cleanup
		if (m_hIconHasSureScriptError!=NULL)
		{
			DestroyIcon((HICON)m_hIconHasSureScriptError );
		}
		// (b.savon 2013-07-16 17:54) - PLID 57377
		if( m_hIconNexFormulary != NULL ){
			DestroyIcon((HICON)m_hIconNexFormulary);
		}

		// (b.savon 2013-08-23 13:38) - PLID 58236 - Add arrow icon
		if( m_hIconArrow != NULL ){
			DestroyIcon((HICON)m_hIconArrow);
		}

		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		if(m_hInfoButtonIcon) {
			DestroyIcon(m_hInfoButtonIcon);
		}
	}
	NxCatchAll("Error in ~CPrescriptionQueueDlg()");
}

void CPrescriptionQueueDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PRESCRIPTION_QUEUE_COLOR, m_queueBkg);
	DDX_Control(pDX, IDC_EXPAND_FIRST, m_btnExpandRenewals);
	DDX_Control(pDX, IDC_RENEWAL_REQUEST_COLOR, m_renewalBkg);
	DDX_Control(pDX, IDC_MEDS_ALLERGIES_COLOR, m_medsAllergiesBkg);
	DDX_Control(pDX, IDC_SHOW_TEXT, m_nxstaticPendingText);
	DDX_Control(pDX, IDC_FIRST_TEXT, m_nxstaticRenewalText);
	DDX_Control(pDX, IDC_MEDS_ALLERGIES_TEXT, m_nxstaticMedsAllergiesText);
	DDX_Control(pDX, IDC_SEND_TO_PRINTER, m_checkSendToPrinter);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_TEMPLATES, m_btnConfigureTemplates);
	DDX_Control(pDX, IDC_BTN_EDIT_PRESCRIPTION_TEMPLATE, m_btnEditTemplate);
	DDX_Control(pDX, IDC_INTERACTIONS_BUTTON, m_btnShowInteractions);
	DDX_Control(pDX, IDC_ESUBMIT_ALL, m_btnESubmit);
	DDX_Control(pDX, IDC_PRINT_ALL_PRESCTIPTIONS, m_btnPreview);
	DDX_Control(pDX, IDC_RADIO_EMN_MEDS, m_radioFilterOnEMNMeds);
	DDX_Control(pDX, IDC_RADIO_PATIENT_MEDS, m_radioFilterOnPatientMeds);
	DDX_Control(pDX, IDC_FILTER_RENEWAL_FROM, m_dtpRenewalFilterFrom);
	DDX_Control(pDX, IDC_FILTER_RENEWAL_TO, m_dtpRenewalFilterTo);
	DDX_Control(pDX, IDC_WRITE_PICK_LIST, m_btnWritePickList);
	DDX_Control(pDX, IDC_BTN_EDIT_FAVORITE_PHARMACIES, m_btnEditFavoritePharmacies);
	DDX_Control(pDX, IDC_BTN_RX_NEEDING_ATTENTION, m_btnRxNeedingAttention);
	DDX_Control(pDX, IDC_BTN_EDIT_MEDS_QUEUE, m_btnEditMeds);
	DDX_Control(pDX, IDC_BTN_EDIT_ALLERGIES_QUEUE, m_btnEditAllergies);
	DDX_Control(pDX, IDC_ALLERGIES_REVIEWED_QUEUE, m_checkReviewedAllergies);
	DDX_Control(pDX, IDC_NO_ALLERGY_QUEUE, m_checkHasNoAllergies);
	DDX_Control(pDX, IDC_NO_MEDS_CHECK_QUEUE, m_checkHasNoMeds);
	DDX_Control(pDX, IDC_BTN_MORE_MEDS, m_btnWriteMoreMeds);
	DDX_Control(pDX, IDC_BTN_NEXFORMULARY, m_btnNexFormulary);
	DDX_Control(pDX, IDC_CHECK_HIDE_DISCONTINUED_ALLERGIES_QUEUE, m_checkHideDiscontinuedAllergies);
	DDX_Control(pDX, IDC_HIDE_DISCONTINUED_MEDS_QUEUE, m_checkHideDiscontinuedMedications);
	DDX_Control(pDX, IDC_RX_QUEUE_MED_HISTORY_BTN, m_btnMedicationHistory);
	DDX_Control(pDX, IDC_RX_ATTENTION_FROM, m_DateFrom); // (b.eyers 2016-01-21) - PLID 67966
	DDX_Control(pDX, IDC_RX_ATTENTION_TO, m_DateTo); // (b.eyers 2016-01-21) - PLID 67966
	DDX_Control(pDX, IDC_USE_PRESCRIPTION_DATE, m_chkUseDateFilter); // (b.eyers 2016-01-21) - PLID 67966
	DDX_Control(pDX, IDC_CHECK_INCLUDE_FREE_TEXT_RX, m_checkIncludeFreeTextRx);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_FREE_TEXT_MEDS, m_checkIncludeFreeTextCurMeds);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_FREE_TEXT_ALLERGIES, m_checkIncludeFreeTextAllergies);
	DDX_Control(pDX, IDC_MULTI_RX_PRESCRIBERS_LABEL, m_nxlMultiRxPrescribers);
	DDX_Control(pDX, IDC_MULTI_RENEWAL_PRESCRIBERS_LABEL, m_nxlMultiRenewalPrescribers);
	DDX_Control(pDX, IDC_ABOUT_RX_MEDICATION_COLORS, m_icoAboutRxMedicationColors); 
	DDX_Control(pDX, IDC_ABOUT_CURRENT_MEDICATION_COLORS, m_icoAboutCurrentMedicationColors); 
	DDX_Control(pDX, IDC_ABOUT_CURRENT_ALLERGIES_COLORS, m_icoAboutCurrentAllergiesColors); 
	DDX_Control(pDX, IDC_ERX_HOUR_SPIN, m_PrescriptionNeedingAttention.m_hourSpin);
	DDX_Control(pDX, IDC_STATIC_HOURS, m_PrescriptionNeedingAttention.m_nxstaticHour);
	DDX_Control(pDX, IDC_STATIC_MINUTES, m_PrescriptionNeedingAttention.m_nxstaticMinutes);
	DDX_Control(pDX, IDC_ERX_REMIND_ME_GROUP, m_PrescriptionNeedingAttention.m_btnRemindMeGroup);
	DDX_Control(pDX, IDC_ERX_MINUTE_SPIN, m_PrescriptionNeedingAttention.m_minuteSpin);
	DDX_Control(pDX, IDC_RADIO_ERX_REMINDLOGIN, m_PrescriptionNeedingAttention.m_remindLoginBtn);
	DDX_Control(pDX, IDC_RADIO_ERX_HOUR, m_PrescriptionNeedingAttention.m_hourBtn);
	DDX_Control(pDX, IDC_ERX_RADIO_MINUTES, m_PrescriptionNeedingAttention.m_minuteBtn);
	DDX_Control(pDX, IDC_RADIO_ERX_DONTREMIND, m_PrescriptionNeedingAttention.m_neverBtn);
	DDX_Control(pDX, IDC_ERX_MINUTE_TIME, m_PrescriptionNeedingAttention.m_nxeditRemindMinuteTime);
	DDX_Control(pDX, IDC_ERX_HOUR_TIME, m_PrescriptionNeedingAttention.m_nxeditRemindHourTime);
	DDX_Control(pDX, IDC_RENEWAL_HOUR_SPIN, m_Renewals.m_hourSpin);
	DDX_Control(pDX, IDC_RENEWAL_STATIC_HOURS, m_Renewals.m_nxstaticHour);
	DDX_Control(pDX, IDC_RENEWAL_STATIC_MINUTES, m_Renewals.m_nxstaticMinutes);
	DDX_Control(pDX, IDC_RENEWAL_REMIND_ME_GROUP, m_Renewals.m_btnRemindMeGroup);
	DDX_Control(pDX, IDC_RENEWAL_MINUTE_SPIN, m_Renewals.m_minuteSpin);
	DDX_Control(pDX, IDC_RADIO_RENEWAL_REMINDLOGIN, m_Renewals.m_remindLoginBtn);
	DDX_Control(pDX, IDC_RADIO_RENEWAL_REMINDHOUR, m_Renewals.m_hourBtn);
	DDX_Control(pDX, IDC_RADIO_RENEWAL_REMINDMINUTE, m_Renewals.m_minuteBtn);
	DDX_Control(pDX, IDC_RADIO_RENEWAL_DONTREMIND, m_Renewals.m_neverBtn);
	DDX_Control(pDX, IDC_RENEWAL_MINUTE_TIME, m_Renewals.m_nxeditRemindMinuteTime);
	DDX_Control(pDX, IDC_RENEWAL_HOUR_TIME, m_Renewals.m_nxeditRemindHourTime);

	
	//m_nxeditRemindMinuteTime
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPrescriptionQueueDlg, CNxDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_EXPAND_FIRST, OnBnClickedExpandFirst)
	ON_BN_CLICKED(IDC_UPDATE_RENEWALS, OnBnClickedUpdateRenewals)
	ON_BN_CLICKED(IDC_MEDS_ALLERGIES_BTN, OnBnClickedMedsAllergiesBtn)
	ON_BN_CLICKED(IDC_PRINT_ALL_PRESCTIPTIONS, OnBnClickedPrintAllPresctiptions)
	ON_BN_CLICKED(IDC_SEND_TO_PRINTER, OnBnClickedSendToPrinter)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_TEMPLATES, OnBnClickedBtnConfigureTemplates)
	ON_BN_CLICKED(IDC_BTN_EDIT_PRESCRIPTION_TEMPLATE, OnBnClickedBtnEditPrescriptionTemplate)
	ON_BN_CLICKED(IDC_INTERACTIONS_BUTTON, OnBnClickedInteractionsButton)
	ON_BN_CLICKED(IDC_RADIO_EMN_MEDS, OnRadioEmnMeds)
	ON_BN_CLICKED(IDC_RADIO_PATIENT_MEDS, OnRadioPatientMeds)
	ON_BN_CLICKED(IDC_ESUBMIT_ALL, &CPrescriptionQueueDlg::OnBnClickedEsubmitAll)
	ON_MESSAGE(NXM_NEWCROP_BROWSER_DLG_CLOSED, OnNewCropBrowserClosed)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FILTER_RENEWAL_FROM, &CPrescriptionQueueDlg::OnDtnDatetimechangeFilterRenewalFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FILTER_RENEWAL_TO, &CPrescriptionQueueDlg::OnDtnDatetimechangeFilterRenewalTo)
	ON_NOTIFY(NM_KILLFOCUS, IDC_FILTER_RENEWAL_FROM, &CPrescriptionQueueDlg::OnNMKillfocusFilterRenewalFrom)
	ON_NOTIFY(NM_KILLFOCUS, IDC_FILTER_RENEWAL_TO, &CPrescriptionQueueDlg::OnNMKillfocusFilterRenewalTo)
	ON_BN_CLICKED(IDC_WRITE_PICK_LIST, &CPrescriptionQueueDlg::OnBnClickedWritePickList)
	ON_BN_CLICKED(IDC_BTN_EDIT_FAVORITE_PHARMACIES, &CPrescriptionQueueDlg::OnBnClickedBtnEditFavoritePharmacies)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_BTN_RX_NEEDING_ATTENTION, &CPrescriptionQueueDlg::OnBnClickedBtnRxNeedingAttention)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_EDIT_MEDS_QUEUE, &CPrescriptionQueueDlg::OnBnClickedBtnEditMedsQueue)
	ON_BN_CLICKED(IDC_NO_MEDS_CHECK_QUEUE, &CPrescriptionQueueDlg::OnBnClickedNoMedsCheckQueue)
	ON_BN_CLICKED(IDC_NO_ALLERGY_QUEUE, &CPrescriptionQueueDlg::OnBnClickedNoAllergyQueue)
	ON_BN_CLICKED(IDC_ALLERGIES_REVIEWED_QUEUE, &CPrescriptionQueueDlg::OnBnClickedAllergiesReviewedQueue)
	ON_BN_CLICKED(IDC_BTN_MORE_MEDS, &CPrescriptionQueueDlg::OnBnClickedBtnMoreMeds)
	ON_BN_CLICKED(IDC_BTN_EDIT_ALLERGIES_QUEUE, &CPrescriptionQueueDlg::OnBnClickedBtnEditAllergiesQueue)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_NEXFORMULARY, &CPrescriptionQueueDlg::OnBnClickedBtnNexformulary)
	ON_BN_CLICKED(IDC_CHECK_HIDE_DISCONTINUED_ALLERGIES_QUEUE, OnCheckHideDiscontinuedAllergiesQueue)
	ON_BN_CLICKED(IDC_HIDE_DISCONTINUED_MEDS_QUEUE, OnCheckHideDiscontinuedMedsQueue)
	ON_BN_CLICKED(IDC_RX_QUEUE_MED_HISTORY_BTN, &CPrescriptionQueueDlg::OnBnClickedRxQueueMedHistoryBtn)
	ON_MESSAGE(NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE, &CPrescriptionQueueDlg::OnBackgroundMedHistoryRequestComplete)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_FREE_TEXT_RX, OnCheckIncludeFreeTextRx)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_FREE_TEXT_MEDS, OnCheckIncludeFreeTextMeds)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_FREE_TEXT_ALLERGIES, OnCheckIncludeFreeTextAllergies)
	// (b.eyers 2016-01-21) - PLID 67966
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RX_ATTENTION_FROM, OnChangeRxAttentionFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RX_ATTENTION_TO, OnChangeRxAttentionToDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_RX_ATTENTION_FROM, OnDtnDropdownRxAttentionFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_RX_ATTENTION_FROM, OnDtnCloseupRxAttentionFromDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_RX_ATTENTION_TO, OnDtnDropdownRxAttentionToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_RX_ATTENTION_TO, OnDtnCloseupRxAttentionToDate)
	ON_BN_CLICKED(IDC_USE_PRESCRIPTION_DATE, OnPrescriptionUseDate)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CPrescriptionQueueDlg::OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_RADIO_ERX_DONTREMIND, &CPrescriptionQueueDlg::OnBnClickedRadioErxDontremind2)
	ON_BN_CLICKED(IDC_RADIO_ERX_REMINDLOGIN, &CPrescriptionQueueDlg::OnBnClickedRadioErxRemindlogin)
	ON_BN_CLICKED(IDC_RADIO_ERX_HOUR, &CPrescriptionQueueDlg::OnBnClickedRadioErxHour)
	ON_BN_CLICKED(IDC_ERX_RADIO_MINUTES, &CPrescriptionQueueDlg::OnBnClickedErxRadioMinutes)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ERX_MINUTE_SPIN, &CPrescriptionQueueDlg::OnDeltaposErxMinuteSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_ERX_HOUR_SPIN, &CPrescriptionQueueDlg::OnDeltaposErxHourSpin)
	ON_BN_CLICKED(IDC_RADIO_RENEWAL_REMINDMINUTE, &CPrescriptionQueueDlg::OnBnClickedRenewalsRadioMinutes)
	ON_BN_CLICKED(IDC_RADIO_RENEWAL_REMINDHOUR, &CPrescriptionQueueDlg::OnBnClickedRenewalsRadioHour)
	ON_BN_CLICKED(IDC_RADIO_RENEWAL_REMINDLOGIN, &CPrescriptionQueueDlg::OnBnClickedRenewalsRadioLogin)
	ON_BN_CLICKED(IDC_RADIO_RENEWAL_DONTREMIND, &CPrescriptionQueueDlg::OnBnClickedRenewalsRadioDontRemind)
	ON_NOTIFY(UDN_DELTAPOS, IDC_RENEWAL_MINUTE_SPIN, &CPrescriptionQueueDlg::OnBnClickedRenewalsMinuteSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_RENEWAL_HOUR_SPIN, &CPrescriptionQueueDlg::OnBnClickedRenewalsHourSpin)
	ON_EN_CHANGE(IDC_RENEWAL_MINUTE_TIME, &CPrescriptionQueueDlg::OnEnChangeRenewalMinuteTime)
	ON_EN_CHANGE(IDC_RENEWAL_HOUR_TIME, &CPrescriptionQueueDlg::OnEnChangeRenewalHourTime)
	ON_EN_CHANGE(IDC_ERX_MINUTE_TIME, &CPrescriptionQueueDlg::OnEnChangeErxMinuteTime)
	ON_EN_CHANGE(IDC_ERX_HOUR_TIME, &CPrescriptionQueueDlg::OnEnChangeErxHourTime)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPrescriptionQueueDlg, CNxDialog)
	ON_EVENT(CPrescriptionQueueDlg, IDC_QUEUE_STATUS_FILTER, 16, CPrescriptionQueueDlg::SelChosenQueueStatusFilter, VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_PRESCRIPTION_QUEUE_LIST, 19, CPrescriptionQueueDlg::LeftClickPrescriptionQueueList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_RENEWAL_REQUEST_LIST, 19, CPrescriptionQueueDlg::LeftClickRenewalRequestList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_PRESCRIPTION_QUEUE_LIST, 3, CPrescriptionQueueDlg::DblClickCellPrescriptionQueueList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CPrescriptionQueueDlg, IDC_PRESCRIPTION_QUEUE_LIST, 2, CPrescriptionQueueDlg::SelChangedPrescriptionQueueList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 18, CPrescriptionQueueDlg::RequeryFinishedCurrentMedsList, VTS_I2)
	ON_EVENT(CPrescriptionQueueDlg, IDC_ALLERGIES_LIST, 18, CPrescriptionQueueDlg::RequeryFinishedAllergiesList, VTS_I2)
	ON_EVENT(CPrescriptionQueueDlg, IDC_PRESCRIPTION_QUEUE_LIST, 10, CPrescriptionQueueDlg::EditingFinishedPrescriptionQueueList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPrescriptionQueueDlg, IDC_PRESCRIPTION_QUEUE_LIST, 7, CPrescriptionQueueDlg::RButtonUpPrescriptionQueueList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_FILTER_RENEWAL_RESPONSE, 2, CPrescriptionQueueDlg::SelChangedFilterRenewalResponse, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_FILTER_RENEWAL_TRANSMIT, 2, CPrescriptionQueueDlg::SelChangedFilterRenewalTransmit, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_FILTER_RENEWAL_PROVIDER, 2, CPrescriptionQueueDlg::SelChangedFilterRenewalProvider, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_RENEWAL_REQUEST_LIST, 7, CPrescriptionQueueDlg::RButtonUpRenewalRequestList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUICK_LIST_POPOUT, 19, CPrescriptionQueueDlg::LeftClickNxdlQuickListPopout, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUICK_LIST_POPOUT, 24, CPrescriptionQueueDlg::FocusLostNxdlQuickListPopout, VTS_NONE)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT, 24, CPrescriptionQueueDlg::FocusLostNxdlQuickListSupervisorPopout, VTS_NONE)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT, 25, CPrescriptionQueueDlg::FocusGainedNxdlQuickListSupervisorPopout, VTS_NONE)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUICK_LIST_POPOUT, 25, CPrescriptionQueueDlg::FocusGainedNxdlQuickListPopout, VTS_NONE)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUEUE_PRESCRIBER_FILTER, 16, CPrescriptionQueueDlg::SelChosenNxdlQueuePrescriberFilter, VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 7, CPrescriptionQueueDlg::RButtonUpCurrentMedsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_ALLERGIES_LIST, 7, CPrescriptionQueueDlg::RButtonUpAllergiesList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT, 19, CPrescriptionQueueDlg::LeftClickNxdlQuickListSupervisorPopout, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 10, CPrescriptionQueueDlg::EditingFinishedCurrentMedsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 8, CPrescriptionQueueDlg::EditingStartingCurrentMedsList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPrescriptionQueueDlg, IDC_ALLERGIES_LIST, 9, CPrescriptionQueueDlg::EditingFinishingAllergiesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPrescriptionQueueDlg, IDC_ALLERGIES_LIST, 10, CPrescriptionQueueDlg::EditingFinishedAllergiesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 19, CPrescriptionQueueDlg::OnLeftClickCurrentMedsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 22, CPrescriptionQueueDlg::ColumnSizingFinishedCurrentMedsList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_ALLERGIES_LIST, 22, CPrescriptionQueueDlg::ColumnSizingFinishedAllergiesList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_CURRENT_MEDS_LIST, 9, CPrescriptionQueueDlg::EditingFinishingCurrentMedsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMedicationDlg, IDC_NXDL_MED_SEARCH_RESULTS_QUEUE, 16, CPrescriptionQueueDlg::SelChosenNxdlMedSearchResultsQueue, VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_WRITE_RX, 4, CPrescriptionQueueDlg::LButtonDownNxdlWriteRx, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_WRITE_RX, 16, CPrescriptionQueueDlg::SelChosenNxdlWriteRx, VTS_DISPATCH)
	ON_EVENT(CPrescriptionQueueDlg, IDC_NXDL_ALLERGY_SEARCH_RESULTS_QUEUE, 16, CPrescriptionQueueDlg::SelChosenNxdlAllergySearchResultsQueue, VTS_DISPATCH)
END_EVENTSINK_MAP()

// CPrescriptionQueueDlg message handlers

BOOL CPrescriptionQueueDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();
		// (b.savon 2013-02-05 11:24) - PLID 54982 - Show the license config. if they don't have any configured but do have some allowed and the user is an admin
		if (m_lNexERxLicense.GetUsedLicensedPrescriberCount() +
			m_lNexERxLicense.GetUsedMidlevelPrescriberCount() +
			m_lNexERxLicense.GetUsedNurseStaffCount() <= 0 &&
			m_lNexERxLicense.GetAllowedLicensedPrescriberCount() +
			m_lNexERxLicense.GetAllowedMidlevelPrescriberCount() +
			m_lNexERxLicense.GetAllowedNurseStaffCount() > 0 &&
			IsCurrentUserAdministrator() &&
			m_lNexERxLicense.HasNexERx()) {
			CString strMessage;
			strMessage.Format("You have %li E-Prescribing licenses, but have not assigned providers or users to any of them. Would you like to setup your E-Prescribing licenses now?\r\n\r\nYou will not be able to E-Prescribe until you have assigned at least one provider to a license.\r\n\r\nYou can configure your E-Prescribing licenses at any time by going to \"Tools->E-Prescribing->NexERx Settings...->Configure NexERx Licenses\"",
				m_lNexERxLicense.GetAllowedLicensedPrescriberCount() +
				m_lNexERxLicense.GetAllowedMidlevelPrescriberCount() +
				m_lNexERxLicense.GetAllowedNurseStaffCount());
			if (IDYES == MessageBox(strMessage, "NexERx License Configuration", MB_ICONINFORMATION | MB_YESNO)) {
				CNexERxSetupDlg dlg(this);
				dlg.DoModal();
			}
		}

		// (s.Dhole 2012-10-29 ) - PLID 54890 - Warning ICON
		// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
		if (m_bHasSureScriptsLicense)
		{
			m_hIconHasSureScriptError = (HICON)LoadImage(AfxGetApp()->m_hInstance,
				MAKEINTRESOURCE(IDI_WARNING_ICO), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

			// (b.savon 2013-07-16 17:54) - PLID 57377
			m_hIconNexFormulary = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_NEXFORMULARY), IMAGE_ICON, 16, 16, 0);
		} else {
			//// (s.dhole 2013-03-27 15:43) - PLID 54890 mark handale to null
			m_hIconHasSureScriptError = NULL;
			// (b.savon 2013-07-16 17:54) - PLID 57377
			m_hIconNexFormulary = NULL;
		}

		// (r.gonet 2016-01-22 16:07) - PLID 67967 - Multi labels are hidden by default and datalist dropdowns are shown instead.
		m_nxlMultiRxPrescribers.SetType(dtsDisabledHyperlink);
		m_nxlMultiRxPrescribers.SetSingleLine(true);

		// (r.gonet 2016-01-22) - PLID 67973 - Multi labels are hidden by default and datalist dropdowns are shown instead.
		m_nxlMultiRenewalPrescribers.SetType(dtsDisabledHyperlink);
		m_nxlMultiRenewalPrescribers.SetSingleLine(true);

		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		m_hInfoButtonIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INFO_ICON), IMAGE_ICON, 16, 16, 0);

		m_pPresQueueList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_QUEUE_LIST, false);
		// (j.fouts 2012-10-03 16:38) - PLID 53009 - QueueStatus Filter List
		m_pQueueStatusFilter = BindNxDataList2Ctrl(IDC_QUEUE_STATUS_FILTER, true);
		// (a.wilson 2012-10-24 11:16) - PLID 53797 - add renewals.
		m_pRenewalList = BindNxDataList2Ctrl(IDC_RENEWAL_REQUEST_LIST, false);
		// (j.fouts 2012-11-06 12:17) - PLID 53574 - Added meds and allergies
		m_pMedsList = BindNxDataList2Ctrl(IDC_CURRENT_MEDS_LIST, false);
		m_pAllergiesList = BindNxDataList2Ctrl(IDC_ALLERGIES_LIST, false);
		// (a.wilson 2013-01-02 15:55) - PLID 54410 - assign renewail filters
		// (r.gonet 2016-01-22) - PLID 67973 - Don't auto-requery this so we can load in our special rows easier.
		// The Rx prescriber filter does that too, so there is some prescedent.
		m_pRenewalFilterPrescriberList = BindNxDataList2Ctrl(IDC_FILTER_RENEWAL_PROVIDER, false);
		m_pRenewalFilterResponseList = BindNxDataList2Ctrl(IDC_FILTER_RENEWAL_RESPONSE, false);
		m_pRenewalFilterTransmitList = BindNxDataList2Ctrl(IDC_FILTER_RENEWAL_TRANSMIT, false);
		// (j.fouts 2012-11-14 11:42) - PLID 53744 - Added templates to the queue
		m_pTemplateList = BindNxDataListCtrl(IDC_PRESCRIPTION_TEMPLATE_LIST, false);
		
		// (j.jones 2016-01-22 08:45) - PLID 67993 - if they do not have FDB, don't show the free text prescription search
		// option, because all searches are free text only without FDBs
		// (j.jones 2016-01-22 09:40) - PLID 67996 - same for current meds
		// (j.jones 2016-01-22 10:21) - PLID 67997 - and same for allergies
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			//no FDB? no checkbox, and always include free-text meds, because that's all you've got
			m_checkIncludeFreeTextRx.SetCheck(TRUE);
			m_checkIncludeFreeTextRx.ShowWindow(SW_HIDE);
			//same for current meds
			m_checkIncludeFreeTextCurMeds.SetCheck(TRUE);
			m_checkIncludeFreeTextCurMeds.ShowWindow(SW_HIDE);
			//and allergies
			m_checkIncludeFreeTextAllergies.SetCheck(TRUE);
			m_checkIncludeFreeTextAllergies.ShowWindow(SW_HIDE);

			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color, hide if no fdb
			m_icoAboutCurrentAllergiesColors.ShowWindow(SW_HIDE);
			m_icoAboutCurrentMedicationColors.ShowWindow(SW_HIDE);
			m_icoAboutRxMedicationColors.ShowWindow(SW_HIDE);
		}
		else {
			// (j.jones 2016-01-22 08:48) - PLID 67993 - added option to include free text meds in the prescription search
			// This preference intentionally only controls the default when entering the tab, never on refresh.
			// This means that changing the pref. while the tab is open intentionally does not update the checkbox.
			long nIncludeFreeTextFDBSearchResults = GetRemotePropertyInt("IncludeFreeTextFDBSearchResults", 0, 0, GetCurrentUserName(), true);
			m_checkIncludeFreeTextRx.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_PRESCRIPTIONS);
			// (j.jones 2016-01-22 09:40) - PLID 67996 - this pref. also controls the current meds checkbox
			m_checkIncludeFreeTextCurMeds.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_CURMEDS);
			// (j.jones 2016-01-22 10:22) - PLID 67997 - the pref also controls the allergies checkbox
			m_checkIncludeFreeTextAllergies.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_ALLERGIES);

			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
			CString strMedsToolTipText = "All medications with a salmon background are imported and are checked for interactions. \r\n"
				"All medications with a red background have changed since being imported, and must be updated before being used on new prescriptions. \r\n"
				"Using free text medications (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			CString strAllergyToolTipText = "All allergies with a salmon background are imported and are checked for interactions. \r\n"
				"All allergies with a red background have changed since being imported, and should be updated. \r\n"
				"Using free text allergies (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			m_icoAboutRxMedicationColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strMedsToolTipText, false, false, false);
			m_icoAboutCurrentMedicationColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strMedsToolTipText, false, false, false);
			m_icoAboutCurrentAllergiesColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strAllergyToolTipText, false, false, false);
			m_icoAboutCurrentAllergiesColors.EnableClickOverride();
			m_icoAboutCurrentMedicationColors.EnableClickOverride();
			m_icoAboutRxMedicationColors.EnableClickOverride();
		}

		// (b.savon 2013-01-18 11:19) - PLID 54678
		//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
		// (j.jones 2016-01-22 09:23) - PLID 67996 - moved the bind to ResetCurrentMedsSearchProvider
		ResetCurrentMedsSearchProvider();
		
		// (b.savon 2013-01-21 09:40) - PLID 54704
		// (j.jones 2016-01-22 09:59) - PLID 67997 - moved the bind into ResetAllergiesSearchProvider
		ResetAllergiesSearchProvider();
		
		// (b.savon 2013-01-23 12:32) - PLID 54782
		m_nxdlQuickList = BindNxDataList2Ctrl(IDC_NXDL_QUICK_LIST_POPOUT, false);
		m_nxdlQuickListSupervisor = BindNxDataList2Ctrl(IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT, false);
		// (b.savon 2013-01-24 18:48) - PLID 54846
		m_nxdlPrescriberFilter = BindNxDataList2Ctrl(IDC_NXDL_QUEUE_PRESCRIBER_FILTER, false);

		// (r.gonet 2016-01-22 16:08) - PLID 67967 - Bind the rx prescriber controller to the datalist and label
		// on the window.
		m_rxPrescriberController.BindController(m_nxlMultiRxPrescribers, m_nxdlPrescriberFilter, IDC_NXDL_QUEUE_PRESCRIBER_FILTER, false, pfcID, pfcName);
		// (r.gonet 2016-01-22) - PLID 67973 - Bind the renewal prescriber controller to the datalist and label
		// on the window.
		m_renewalPrescriberController.BindController(m_nxlMultiRenewalPrescribers, m_pRenewalFilterPrescriberList, IDC_FILTER_RENEWAL_PROVIDER, false, rrfcID, rrfcName);

		// (b.savon 2013-01-11 09:46) - PLID 53848 - Request table checker for sending prescription update.
		GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());

		// (j.fouts 2012-11-08 13:04) - PLID 53574 - Set the NO indicators font to bold and hide it at first
		CFont *fBold = &theApp.m_boldFont;

		// (b.savon 2013-01-07 10:27) - PLID 54464 - Changed to have icons
		// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved merge templates into the queue
		// (j.jones 2008-06-06 11:40) - PLID 29154 - added m_btnConfigureTemplates
		m_btnConfigureTemplates.AutoSet(NXB_PRINT);
		m_btnEditTemplate.AutoSet(NXB_MERGE);

		// (j.fouts 2012-11-14 11:42) - PLID 53573 - Moved drug interactions into the queue
		m_btnShowInteractions.AutoSet(NXB_INTERACTIONS);
		// (r.gonet 09/21/2013) - PLID 58416 - Icon for the med history button
		m_btnMedicationHistory.AutoSet(NXB_MEDICATION_HISTORY);

		// (j.fouts 2012-11-14 11:42) - PLID 53745 - Use Icons
		m_btnESubmit.AutoSet(NXB_ERX);
		// (j.fouts 2012-11-14 11:42) - PLID 53743 - Use Icons
		m_btnPreview.AutoSet(NXB_MERGE);
		// (j.fouts 2013-01-04 13:31) - PLID 54456 - Added a new button to write from pick list
		m_btnWritePickList.AutoSet(NXB_NEW);
		// (b.savon 2013-07-16 12:45) - PLID 57377 - Added NexFormulary Button
		m_btnNexFormulary.AutoSet(NXB_NEXFORMULARY);

		// (b.savon 2013-01-07 09:26) - PLID 54461
		m_btnEditFavoritePharmacies.AutoSet(NXB_PILLBOTTLE);
		// (b.savon 2013-01-10 16:07) - PLID 54567
		m_btnRxNeedingAttention.AutoSet(NXB_SCRIPT_ATTENTION);

		// (j.fouts 2012-11-16 11:02) - PLID 53156 - If we are in patientless mode, hide patient specific controls
		if(m_nPatientID <= 0)
		{
			m_bShowMedsAllergies = false;
			// (r.gonet 09/20/2013) - PLID 58416 - Don't show the medication history button for non-patients.
			m_bShowMedHistory = false;
			// (b.savon 2014-01-03 07:45) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr)
			// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
			m_bShowNexFormulary = false;
			m_bShowInteractions = false;
			m_bShowWrite = false;

		}

		// (b.savon 2013-01-18 11:19) - PLID 54678 bind search control
		//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
		// (j.jones 2016-01-22 08:50) - PLID 67993 - moved this bind to be in ResetPrescriptionSearchProvider
		ResetPrescriptionSearchProvider();

		if (!m_bShowNexFormulary)
		{
			HideNexFormularyColumn(m_nxdlWritePrescriptionResults, mrcNexFormulary);
		}
		// (j.jones 2013-01-17 16:52) - PLID 53085 - added ability to remember EMN/Patient filter
		// (a.wilson 2013-01-07 11:43) - PLID 54484 - add prescriber per user preference.
		// (r.gonet 09/20/2013) - PLID 58397 - Added MedHistory_UseLastFillDateRange and MedHistory_DefaultFromNumMonthsInPast
		g_propManager.CachePropertiesInBulk("PrescriptionQueueDlg", propNumber,
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ShowEnglishPrescriptionColumn' OR "
			"Name = 'PrescriptionQueueDefaultEMNFilter' OR "
			"Name = 'Prescription_ShowColumns' OR "
			"Name = 'FormatPhoneNums' "
			// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
			"OR Name = 'HideDiscontinuedAllergies' "
			"OR Name = 'HideInactiveMedications' "
			"OR Name = 'MedHistory_UseLastFillDateRange' "
			"OR Name = 'MedHistory_DefaultFromNumMonthsInPast' "
			// (s.dhole 2013-07-08 14:29) - PLID 56931
			// (j.jones 2016-01-12 09:33) - PLID 67855 - renamed CurrentPatientMedicationQueueColumnWidth to CurrentPatientMedicationQueueColumnWidths
			// to effectively reset the stored widths for current medications
			"OR Name = 'CurrentPatientMedicationQueueColumnWidths' "
			"OR Name = 'CurrentPatientAllergyQueueColumnWidth' "// (s.dhole 2013-07-08 14:29) - PLID 56931
			"OR Name = 'ShowPatientEducationLinks' " // (r.gonet 2014-01-27 15:29) - PLID 59339
			"OR Name = 'ShowPatientEducationButtons' " // (r.gonet 2014-01-27 15:29) - PLID 59339
			"OR Name = 'RememberProviderSelectionRxNeedingAttention' "
			"OR Name = 'IncludeFreeTextFDBSearchResults'" // (j.jones 2016-01-22 08:44) - PLID 67993
			// (s.tullis 2016-01-28 17:46) - PLID 68090
			"OR Name = 'PrescriptionNeedingAttentionReminderSelection' "
			"OR Name = 'PrescriptionNeedingAttentionReminderValue' "
			// (s.tullis 2016-01-28 17:46) - PLID 67965
			"OR Name = 'RenewalsReminderSelection' "
			"OR Name = 'RenewalsReminderValue' "
			")"
			")", 
			_Q(GetCurrentUserName()));
		// (a.wilson 2013-02-07 17:24) - PLID 51711 - cache phoneformatstring for renewals.
		g_propManager.CachePropertiesInBulk("PrescriptionQueueDlg-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PhoneFormatString' OR "
			// (s.tullis 2016-01-28 23:54) - PLID 67968 - As a user, I want to have the prescriber field in the Rx Needing Attention screen to be remembered per user.
			"Name = 'DefaultRenewalPrescriberFilter' OR "
			"Name = 'DefaultAttentionPrescriberFilter' "
			")",
			_Q(GetCurrentUserName()));

		// (r.gonet 2016-01-22) - PLID 67967 - Encapsulate the rx prescription filter loading in a function.
		InitializeRxFilters();

		// (a.wilson 2013-01-02 15:55) - PLID 54410 - construct and set defaults for renewal filters.
		// (r.gonet 2016-01-22) - PLID 67973 - Moved this up.
		InitializeRenewalFilters();

		//If we are not licensed for SureScripts Prescriptions should never be collapsed
		// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
		if(m_bHasSureScriptsLicense)
		{
			
			if(m_bOpenToRenewals)
			{
				// (j.fouts 2012-11-14 11:42) - PLID 53439 - Hide prescriptions when renewals is shown
				m_pendingBox.ToggleCollapsed();
			}
			else
			{
				m_renewalsBox.ToggleCollapsed();
			}
		}

		// (b.savon 2013-01-21 12:23) - PLID 54720 - Store the user role and update the buttons
		m_bCachedIsLicensedConfiguredUser = IsLicensedConfiguredUser();
		// (a.wilson 2013-04-30 16:46) - PLID 56509 - Get the users erx settings.
		m_lNexERxLicense.GetNexERxUser(m_nexerxUser);
		m_urtCurrentuser = m_nexerxUser.urtUserRole;
		
		// (j.fouts 2012-11-20 14:23) - PLID 53840 - Hide anything that they don't have a license for
		UpdateControlsForLicense();

		//Add controls to the pending box
		m_pendingBox.AddHeaderControl(IDC_QUEUE_STATUS_FILTER);
		m_pendingBox.AddFooterControl(IDC_ESUBMIT_ALL);
		m_pendingBox.AddFooterControl(IDC_PRINT_ALL_PRESCTIPTIONS);
		m_pendingBox.AddFooterControl(IDC_SEND_TO_PRINTER);
		// (b.savon 2013-07-16 12:47) - PLID 57377 - Added NexFormulary
		// (b.savon 2014-01-03 07:47) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr)
		// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
		if( m_bShowNexFormulary ){
			m_pendingBox.AddFooterControl(IDC_BTN_NEXFORMULARY);
		} else {
			// Otherwise hide it.
			m_btnNexFormulary.ShowWindow(SW_HIDE);
			m_btnNexFormulary.EnableWindow(FALSE);
		}
		m_pendingBox.AddHeaderControl(IDC_WRITE_PICK_LIST);
		m_pendingBox.AddHeaderControl(IDC_TEMPLATE_TEXT);
		m_pendingBox.AddHeaderControl(IDC_PRESCRIPTION_TEMPLATE_LIST);
		m_pendingBox.AddHeaderControl(IDC_BTN_CONFIGURE_TEMPLATES);
		m_pendingBox.AddHeaderControl(IDC_BTN_EDIT_PRESCRIPTION_TEMPLATE);
		m_pendingBox.AddBodyControl(IDC_PRESCRIPTION_QUEUE_LIST);
		// (j.jones 2012-11-16 16:03) - PLID 53085 - added EMN filter
		m_pendingBox.AddHeaderControl(IDC_EMR_FILTER_TEXT);
		m_pendingBox.AddHeaderControl(IDC_RADIO_EMN_MEDS);
		m_pendingBox.AddHeaderControl(IDC_RADIO_PATIENT_MEDS);
		// (b.savon 2013-01-07 10:11) - PLID 54461
		m_pendingBox.AddHeaderControl(IDC_BTN_EDIT_FAVORITE_PHARMACIES);
		// (b.savon 2013-01-10 17:04) - PLID 54567
		m_pendingBox.AddHeaderControl(IDC_BTN_RX_NEEDING_ATTENTION);
		// (b.savon 2013-01-21 10:22) - PLID 54678
		m_pendingBox.AddHeaderControl(IDC_NXDL_WRITE_RX);
		m_pendingBox.AddHeaderControl(IDC_STATIC_WRITE_RX);
		m_pendingBox.AddHeaderControl(IDC_BTN_MORE_MEDS);
		// (b.savon 2013-01-24 18:50) - PLID 54846
		m_pendingBox.AddHeaderControl(IDC_NXDL_QUEUE_PRESCRIBER_FILTER);
		// (r.gonet 2016-01-22) - PLID 67967 - Add the multi prescribers label to the header controls.
		m_pendingBox.AddHeaderControl(IDC_MULTI_RX_PRESCRIBERS_LABEL);
		// (b.savon 2013-04-18 14:08) - PLID 54782
		m_pendingBox.AddHeaderControl(IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT);
		m_pendingBox.AddHeaderControl(IDC_NXDL_QUICK_LIST_POPOUT);
		// (j.jones 2016-01-21 17:11) - PLID 67993 - added option to include free text meds in the prescription search
		m_pendingBox.AddHeaderControl(IDC_CHECK_INCLUDE_FREE_TEXT_RX);
		// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
		m_pendingBox.AddHeaderControl(IDC_ABOUT_RX_MEDICATION_COLORS);
		// (b.eyers 2016-01-21) - PLID 67966 - these are only for the rx needing attention dialog
		if (m_nPatientID == -1) {
			m_pendingBox.AddHeaderControl(IDC_RX_ATTENTION_FROM);
			m_pendingBox.AddHeaderControl(IDC_RX_ATTENTION_TO);
			m_pendingBox.AddHeaderControl(IDC_RX_FROM_LABEL);
			m_pendingBox.AddHeaderControl(IDC_RX_TO_LABEL);
			m_pendingBox.AddHeaderControl(IDC_USE_PRESCRIPTION_DATE);

			m_PrescriptionNeedingAttention.m_nxeditRemindMinuteTime.SetLimitText(3);
			m_PrescriptionNeedingAttention.m_nxeditRemindHourTime.SetLimitText(3);
			m_pendingBox.AddHeaderControl(IDC_RADIO_ERX_REMINDLOGIN);
			m_pendingBox.AddHeaderControl(IDC_ERX_RADIO_MINUTES);
			m_pendingBox.AddHeaderControl(IDC_ERX_MINUTE_TIME);
			m_pendingBox.AddHeaderControl(IDC_ERX_MINUTE_SPIN);
			m_pendingBox.AddHeaderControl(IDC_STATIC_MINUTES);
			m_pendingBox.AddHeaderControl(IDC_RADIO_ERX_HOUR);
			m_pendingBox.AddHeaderControl(IDC_ERX_HOUR_TIME);
			m_pendingBox.AddHeaderControl(IDC_ERX_HOUR_SPIN);
			m_pendingBox.AddHeaderControl(IDC_STATIC_HOURS);
			m_pendingBox.AddHeaderControl(IDC_RADIO_ERX_DONTREMIND);
			m_pendingBox.AddHeaderControl(IDC_ERX_REMIND_ME_GROUP);
		}
		else {
			ShowHideReminderControls(FALSE);
		}

		// (j.fouts 2012-11-20 15:24) - PLID 53840 - Only add if we want to show renewals
		if(m_bShowRenewals)
		{
			//Add controls to the renewal box
			m_renewalsBox.AddFooterControl(IDC_UPDATE_RENEWALS);
			m_renewalsBox.AddBodyControl(IDC_RENEWAL_REQUEST_LIST);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_PROVIDER);
			// (r.gonet 2016-01-22) - PLID 67973 - Added the multi renewal prescribers label to the header controls.
			m_renewalsBox.AddHeaderControl(IDC_MULTI_RENEWAL_PRESCRIBERS_LABEL);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_RESPONSE);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_TRANSMIT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_TEXT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_PROVIDER_TEXT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_RESPONSE_TEXT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_TRANSMIT_TEXT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_FROM_TEXT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_TO_TEXT);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_FROM);
			m_renewalsBox.AddHeaderControl(IDC_FILTER_RENEWAL_TO);
			// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed the Default Filtered Prescriber checkbox.
			if (m_nPatientID == -1)
			{
				// (s.tullis 2016-01-28 17:46) - PLID 67965
				m_Renewals.m_nxeditRemindMinuteTime.SetLimitText(3);
				m_Renewals.m_nxeditRemindHourTime.SetLimitText(3);
				m_renewalsBox.AddHeaderControl(IDC_RADIO_RENEWAL_REMINDLOGIN);
				m_renewalsBox.AddHeaderControl(IDC_RADIO_RENEWAL_REMINDMINUTE);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_MINUTE_TIME);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_MINUTE_SPIN);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_STATIC_MINUTES);
				m_renewalsBox.AddHeaderControl(IDC_RADIO_RENEWAL_REMINDHOUR);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_HOUR_TIME);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_HOUR_SPIN);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_STATIC_HOURS);
				m_renewalsBox.AddHeaderControl(IDC_RADIO_RENEWAL_DONTREMIND);
				m_renewalsBox.AddHeaderControl(IDC_RENEWAL_REMIND_ME_GROUP);

			}

			
		}

		if(m_bShowMedsAllergies)
		{
			// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added no meds/allergies/and reviewed check boxes
			m_medsAllergiesBox.AddHeaderControl(IDC_NO_MEDS_CHECK_QUEUE);
			m_medsAllergiesBox.AddHeaderControl(IDC_NO_ALLERGY_QUEUE);

			m_medsAllergiesBox.AddFooterControl(IDC_ALLERGIES_REVIEWED_QUEUE);

			//Add controls to the meds and allergies box
			m_medsAllergiesBox.AddBodyControl(IDC_CURRENT_MEDS_LIST);
			m_medsAllergiesBox.AddBodyControl(IDC_ALLERGIES_LIST);
			// (j.fouts 2012-11-08 13:04) - PLID 53574 - Added a string control to the bottom of this
			m_medsAllergiesBox.AddHeaderControl(IDC_MEDS_ALLERGIES_TEXT);
			// (b.savon 2013-01-21 10:23) - PLID 54691
			m_medsAllergiesBox.AddHeaderControl(IDC_NXDL_MED_SEARCH_RESULTS_QUEUE);
			m_medsAllergiesBox.AddHeaderControl(IDC_BTN_EDIT_MEDS_QUEUE);
			m_medsAllergiesBox.AddHeaderControl(IDC_STATIC_MED_SEARCH);
			// (r.gonet 09/20/2013) - PLID 58416 - If we should show the medication history button, add it to the header controls.
			if(m_bShowMedHistory) {
				m_medsAllergiesBox.AddHeaderControl(IDC_RX_QUEUE_MED_HISTORY_BTN);
			} else {
				// (r.gonet 09/20/2013) - PLID 58416 - Otherwise hide it.
				m_btnMedicationHistory.ShowWindow(SW_HIDE);
				m_btnMedicationHistory.EnableWindow(FALSE);
			}
			// (b.savon 2013-01-21 10:23) - PLID 54704
			m_medsAllergiesBox.AddHeaderControl(IDC_STATIC_ALLER_SEARCH_QUEUE);
			m_medsAllergiesBox.AddHeaderControl(IDC_BTN_EDIT_ALLERGIES_QUEUE);
			m_medsAllergiesBox.AddHeaderControl(IDC_NXDL_ALLERGY_SEARCH_RESULTS_QUEUE);
			// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
			m_medsAllergiesBox.AddHeaderControl(IDC_CHECK_HIDE_DISCONTINUED_ALLERGIES_QUEUE);
			m_medsAllergiesBox.AddHeaderControl(IDC_HIDE_DISCONTINUED_MEDS_QUEUE);
			// (j.fouts 2013-01-07 10:44) - PLID 54468 - Interactions are grouped with current meds and allergies
			// (j.fouts 2012-11-16 11:02) - PLID 53156 - Hide interactions if we should
			if(m_bShowInteractions)
			{
				m_medsAllergiesBox.AddHeaderControl(IDC_INTERACTIONS_BUTTON);
			}
			else
			{
				m_btnShowInteractions.ShowWindow(SW_HIDE);
				m_btnShowInteractions.EnableWindow(FALSE);
			}

			// (j.jones 2016-01-21 17:06) - PLID 67996 - added option to include free text meds in the current meds search
			m_medsAllergiesBox.AddHeaderControl(IDC_CHECK_INCLUDE_FREE_TEXT_MEDS);
			// (j.jones 2016-01-21 17:06) - PLID 67997 - added option to include free text allergies in the allergy search
			m_medsAllergiesBox.AddHeaderControl(IDC_CHECK_INCLUDE_FREE_TEXT_ALLERGIES);
			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
			m_medsAllergiesBox.AddHeaderControl(IDC_ABOUT_CURRENT_MEDICATION_COLORS);
			m_medsAllergiesBox.AddHeaderControl(IDC_ABOUT_CURRENT_ALLERGIES_COLORS);
		}

		// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
		m_checkHideDiscontinuedAllergies.SetCheck(GetRemotePropertyInt("HideDiscontinuedAllergies", 0, 0, GetCurrentUserName(), true));
		m_checkHideDiscontinuedMedications.SetCheck(GetRemotePropertyInt("HideInactiveMedications", 0, 0, GetCurrentUserName(), true));
		// (j.fouts 2012-11-06 12:18) - PLID 53574 - Bind controls to their boxes
		m_pendingBox.BindControls(&m_queueBkg, &m_nxstaticPendingText);
		m_renewalsBox.BindControls(&m_renewalBkg, &m_nxstaticRenewalText, &m_btnExpandRenewals);
		m_medsAllergiesBox.BindControls(&m_medsAllergiesBkg);

		
		m_aryCollapseableBoxes.Add(&m_pendingBox);
		
		// (j.fouts 2012-11-20 15:24) - PLID 53840 - Only Add if we want to show renewals
		if(m_bShowRenewals)
		{
			m_aryCollapseableBoxes.Add(&m_renewalsBox);
		}

		// (j.fouts 2012-11-06 12:19) - PLID 53574 - Added meds and allergies
		if(m_bShowMedsAllergies)
		{
			//Only add this if we want to show it
			m_aryCollapseableBoxes.Add(&m_medsAllergiesBox);
			
		}
		else
		{
			//Otherwize hide/disable it
			HideDisableMedsAllergies();
		}

		// (j.fouts 2012-11-14 11:42) - PLID 53743 - Moved printing into the queue
		// (c.haag 2009-08-18 13:04) - PLID 15479 - Recall the merge to printer check state
		long nMergeToPrinter = GetRemotePropertyInt("PatientMedMergePrescriptionToPrinter", 0, 0, GetCurrentUserName(), true);
		if (nMergeToPrinter == 0){
			m_checkSendToPrinter.SetCheck(0);
		}
		else {
			m_checkSendToPrinter.SetCheck(1);
		}

		// (j.fouts 2012-09-26 10:33) - PLID 52973 - Color the new NxColors
		// (a.wilson 2012-10-24 11:13) - PLID 51711 - add renewals
		// (j.fouts 2012-11-06 12:20) - PLID 53574 - Added meds and allergies
		m_queueBkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_renewalBkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_medsAllergiesBkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		
		// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved merge templates into the queue
		RequeryWordTemplateList();
		// (j.fouts 2012-11-14 11:42) - PLID 53743 - Moved printing to queue
		UpdatePrintPreviewText();
		// (j.fouts 2012-11-14 11:42) - PLID 53439 - Update patient specific controls
		ChangePatient(m_nPatientID, true); //Just a note, this requeries the prescription list only when in the medication tab, not in the rx needing attention (onshowwindow does that)

		UpdatePreviewButton();
		UpdateSendButton();
		// (j.fouts 2013-04-22 14:12) - PLID 54719 - Update the hidden/shown columns
		UpdateView();
		if (m_nPatientID == -1) {
			DisableEnableReminderControls(FALSE);
		}

		return TRUE;
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.fouts 2012-11-06 12:21) - PLID 53574 - Hide/Disable the Current Meds and Allergies controls
void CPrescriptionQueueDlg::HideDisableMedsAllergies()
{
	m_medsAllergiesBkg.ShowWindow(SW_HIDE);
	m_nxstaticMedsAllergiesText.ShowWindow(SW_HIDE);
	m_medsAllergiesBkg.EnableWindow(FALSE);
	m_nxstaticMedsAllergiesText.EnableWindow(FALSE);
	m_pMedsList->Enabled = VARIANT_FALSE;
	m_pAllergiesList->Enabled = VARIANT_FALSE;
	GetDlgItem(IDC_CURRENT_MEDS_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_ALLERGIES_LIST)->ShowWindow(SW_HIDE);
	// (j.fouts 2013-01-07 10:44) - PLID 54468 - Interactions are grouped with current meds and allergies
	m_btnShowInteractions.ShowWindow(SW_HIDE);
	m_btnShowInteractions.EnableWindow(FALSE);
	// (r.gonet 08/19/2013) - PLID 58416 - Hide the medication history button.
	m_btnMedicationHistory.ShowWindow(SW_HIDE);
	m_btnMedicationHistory.EnableWindow(FALSE);
	// (b.savon 2013-01-21 10:26) - PLID 54691
	GetDlgItem(IDC_NXDL_MED_SEARCH_RESULTS_QUEUE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NXDL_MED_SEARCH_RESULTS_QUEUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_MED_SEARCH)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_MED_SEARCH)->EnableWindow(FALSE);
	m_btnEditMeds.ShowWindow(SW_HIDE);
	m_btnEditMeds.EnableWindow(FALSE);
	// (b.savon 2013-01-21 10:26) - PLID 54704
	m_btnEditAllergies.ShowWindow(SW_HIDE);
	m_btnEditAllergies.EnableWindow(FALSE);
	GetDlgItem(IDC_NXDL_ALLERGY_SEARCH_RESULTS_QUEUE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NXDL_ALLERGY_SEARCH_RESULTS_QUEUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_ALLER_SEARCH_QUEUE)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATIC_ALLER_SEARCH_QUEUE)->ShowWindow(SW_HIDE);
	// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added no meds/allergies/and reviewed check boxes
	m_checkReviewedAllergies.ShowWindow(SW_HIDE);
	m_checkHasNoAllergies.ShowWindow(SW_HIDE);
	m_checkHasNoMeds.ShowWindow(SW_HIDE);
	m_checkReviewedAllergies.EnableWindow(FALSE);
	m_checkHasNoAllergies.EnableWindow(FALSE);
	m_checkHasNoMeds.EnableWindow(FALSE);
	// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
	m_checkHideDiscontinuedAllergies.ShowWindow(SW_HIDE);
	m_checkHideDiscontinuedMedications.ShowWindow(SW_HIDE);
	m_checkHideDiscontinuedAllergies.EnableWindow(FALSE);
	m_checkHideDiscontinuedMedications.EnableWindow(FALSE);
	// (j.jones 2016-01-21 17:06) - PLID 67996 - added option to include free text meds in the current meds search
	m_checkIncludeFreeTextCurMeds.ShowWindow(SW_HIDE);
	// (j.jones 2016-01-21 17:06) - PLID 67997 - added option to include free text allergies in the allergy search
	m_checkIncludeFreeTextAllergies.ShowWindow(SW_HIDE);
	// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
	m_icoAboutCurrentMedicationColors.ShowWindow(SW_HIDE);
	m_icoAboutCurrentAllergiesColors.ShowWindow(SW_HIDE);
}

// (j.fouts 2012-09-26 10:34) - PLID 52906 - Calls the API to requery the prescription queue
// (j.fouts 2012-11-14 11:42) - PLID 53573 - Turned on interaction checks
// (j.jones 2012-11-19 11:03) - PLID 53085 - added bShowInteractionChecks, such that if
// the requery is only due to changing a filter, we don't always have to show the interactions
void CPrescriptionQueueDlg::RequeryQueue(bool bShowInteractionChecks /*= true*/)
{
	try
	{
		// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(this, true)) 
			{
				//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
				return;
			}
		}

		// (j.fouts 2013-05-30 17:27) - PLID 56807 - Drug Interactions are tied to NexERx
		// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
		if(!m_bHasSureScriptsLicense)
		{
			bShowInteractionChecks = false;
		}

		NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pRequest(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));

		pRequest->filters = GetQueueFilters();

		CWaitCursor pWait;

		// (j.jones 2016-01-22 14:24) - PLID 63732 - begin a progress bar
		CProgressDialog progressDialog;
		BeginProgressDialog(progressDialog);

		bShowInteractionChecks &= m_nPatientID > 0;
		pRequest->Action = NexTech_Accessor::UpdatePresQueueAction_None;
		pRequest->DrugDrugInteracts = (VARIANT_BOOL)bShowInteractionChecks;
		// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
		pRequest->ExcludeMonographInformation = VARIANT_TRUE;
		pRequest->DrugAllergyInteracts = (VARIANT_BOOL)bShowInteractionChecks;
		pRequest->DrugDiagnosisInteracts = (VARIANT_BOOL)bShowInteractionChecks;
		pRequest->RequeryQueue = VARIANT_TRUE;
		// (s.dhole 2012-12-05 17:21) - PLID 54067 Pass Surescripts licence
		// (b.savon 2013-03-12 13:03) - PLID 55518 - Dont pass surescripts license
		// (j.fouts 2013-04-24 16:54) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
		NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(
			GetAPISubkey(), GetAPILoginToken(), _bstr_t(FormatString("%li", m_nPatientID)), pRequest);

		//Fill the datalist
		RefreshQueueFromArray(pResults->PrescriptionsInQueue, pResults->Pharmacies, progressDialog);

		progressDialog.SetLine(2, "Calculating drug interactions...");

		// (j.fouts 2013-05-30 17:27) - PLID 56807 - Drug Interactions is tied to NexERx
		if(bShowInteractionChecks)
		{
			RefreshInteractionsFromArray(
				pResults->DrugDrugInteracts, 
				pResults->DrugAllergyInteracts,
				pResults->DrugDiagnosisInteracts
				);
		}

		// (j.jones 2016-01-22 14:24) - PLID 63732 - end the progress bar
		progressDialog.Stop();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-05 13:38) - PLID 56114 - added function to reload allergies & meds, will do nothing
// if the list is hidden due to being embedded in the Medications tab
void CPrescriptionQueueDlg::RequeryAllergiesAndMeds(bool bRequeryAllergies /*= true*/, bool bRequeryMeds /*= true*/)
{
	//throw exceptions to the caller

	if(!m_bShowMedsAllergies) {
		//shouldn't have called this function
		ASSERT(FALSE);
		return;
	}

	if(!bRequeryAllergies && !bRequeryMeds) {
		//This function should have never been called with both false.
		//But go ahead and return, because after all, that is what they asked for.
		ASSERT(FALSE);
		return;
	}

	CString strWhere;
	if(bRequeryMeds) {
		//re-set the where clause, we might have changed patients

		// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued meds
		if (m_checkHideDiscontinuedMedications.GetCheck()) {
			strWhere.Format("PatientID = %li AND Discontinued = 0", m_nPatientID);
		}
		else {
			strWhere.Format("PatientID = %li", m_nPatientID);
		}
		m_pMedsList->WhereClause = (bstr_t)strWhere;
		m_pMedsList->Requery();
	}

	if(bRequeryAllergies) {
		//re-set the where clause, we might have changed patients
		// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
		if (m_checkHideDiscontinuedAllergies.GetCheck()) {
			strWhere.Format("PersonID = %li AND Discontinued = 0", m_nPatientID);
		}
		else {
			strWhere.Format("PersonID = %li", m_nPatientID);
		}
		m_pAllergiesList->WhereClause = (bstr_t)strWhere;
		m_pAllergiesList->Requery();
	}

	BOOL bReviewedAllergies = FALSE;
	COleDateTime dtReviewedOn, dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	dtReviewedOn = dtInvalid;
	CString strReviewedBy = "";

	_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.DisplayAllergyWarning, "
		"PatientsT.AllergiesReviewedOn, UsersT.Username, PatientsT.HasNoAllergies, PatientsT.HasNoMeds "
		"FROM PatientsT "
		"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
		"LEFT JOIN UsersT ON PatientsT.AllergiesReviewedBy = UsersT.PersonID "
		"WHERE PatientsT.PersonID = {INT}", m_nPatientID);
	if(!rs->eof)
	{
		dtReviewedOn = AdoFldDateTime(rs, "AllergiesReviewedOn", dtInvalid);
		if(dtReviewedOn.GetStatus() != COleDateTime::invalid) {
			bReviewedAllergies = TRUE;
			strReviewedBy = AdoFldString(rs, "Username", "<Unknown User>");
		}
		else {
			bReviewedAllergies = FALSE;
			strReviewedBy = "";
		}

		m_bHasNoAllergies = AdoFldBool(rs, "HasNoAllergies", FALSE);
		m_bHasNoMeds = AdoFldBool(rs, "HasNoMeds", FALSE);
	}

	//no need to wrap this in a boolean flag, since we already ran the recordset
	//we might as well update both checkboxes and the allergy review status
	m_checkHasNoAllergies.SetCheck(m_bHasNoAllergies);
	m_checkHasNoMeds.SetCheck(m_bHasNoMeds);

	// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added reviewed check box
	UpdateAllergyReviewCtrls(!!bReviewedAllergies,dtReviewedOn,strReviewedBy);
}

// (j.jones 2016-01-22 14:14) - PLID 63732 - creates a progress bar for use prior to loading the data from the API,
// ends after the screen has finished updating
void CPrescriptionQueueDlg::BeginProgressDialog(CProgressDialog &progressDialog)
{
	progressDialog.Start(NULL, CProgressDialog::NoCancel | CProgressDialog::AutoTime | CProgressDialog::NoMinimize | CProgressDialog::MarqueeProgress,
		"NexTech Practice", "Please wait...");
	progressDialog.SetLine(1, "Loading prescriptions...");
	progressDialog.SetLine(2, "Retrieving prescription list, please wait...");
}

// (j.fouts 2012-09-26 11:28) - PLID 52973 - Clears the queue and fills it with the data passed in
// (j.fouts 2013-01-14 10:41) - PLID 54464 - Added a Pharmacy Array
// (j.jones 2016-01-22 14:18) - PLID 63732 - now takes in a progress dialog
void CPrescriptionQueueDlg::RefreshQueueFromArray(Nx::SafeArray<IUnknown *> saryPrescriptions, Nx::SafeArray<IUnknown *> saryPharmacies, CProgressDialog &progressDialog)
{
	// (j.jones 2016-01-22 12:06) - PLID 63732 - added a progress bar for when we have a huge list,
	// set the progress to reflect the total prescriptions we are loading
	progressDialog.SetProgress(0, saryPrescriptions.GetLength());	

	// (j.fouts 2012-12-03 14:09) - PLID 53743 - Before clearing the list lets save the checks for print
	// (j.fouts 2012-12-03 14:10) - PLID 53745 - Before clearing the list lets save the checks for send
	struct SendPrintFlags{
		BOOL bSend;
		BOOL bPrint;
	};

	// (j.jones 2016-01-22 11:04) - PLID 63732 - converted to an unordered_map
	std::unordered_map<long, SendPrintFlags> mapPrescriptionFlags;

	for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();
		pRow; pRow = pRow->GetNextRow())
	{
		// (s.dhole 2012-12-24 11:09) - PLID 53334 check null
		SendPrintFlags flags = {VarBool(pRow->GetValue(pqcSend),g_cvarFalse), VarBool(pRow->GetValue(pqcPrint),g_cvarFalse)};

		// (j.jones 2016-01-22 11:04) - PLID 63732 - converted to an unordered_map
		mapPrescriptionFlags[VarLong(pRow->GetValue(pqcPrescriptionID))] = flags;
	}

	//Clear the list out first
	m_pPresQueueList->Clear();

	// (j.fouts 2013-04-25 12:46) - PLID 53146 - Seperated Favorite Pharmacies from full pharmacy list
	m_saryAllPharmacies = saryPharmacies;

	long nCountPrescriptions = 0;

	foreach(NexTech_Accessor::_QueuePrescriptionPtr pPrescription, saryPrescriptions)
	{
		// (j.jones 2016-01-22 12:25) - PLID 63732 - added a progress bar
		nCountPrescriptions++;
		progressDialog.SetProgress(nCountPrescriptions, saryPrescriptions.GetLength());
		progressDialog.SetLine(2, "Retrieving prescription %li of %li", nCountPrescriptions, saryPrescriptions.GetLength());

		long nPrescriptionID = AsLong(pPrescription->PrescriptionID);
		
		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
		// (j.jones 2016-01-22 11:04) - PLID 63732 - converted to an unordered_map
		std::unordered_map<long, SendPrintFlags>::iterator it = mapPrescriptionFlags.find(nPrescriptionID);
		if(it != mapPrescriptionFlags.end())
		{
			SendPrintFlags flags = mapPrescriptionFlags[nPrescriptionID];
			InsertPrescriptionIntoQueue(pPrescription, !!flags.bSend, !!flags.bPrint);
		}
		else
		{
			InsertPrescriptionIntoQueue(pPrescription);
		}
	}
}

// (j.fouts 2012-11-06 12:25) - PLID 53574 - Skip controls that should be auto resized by the dlg
BOOL CPrescriptionQueueDlg::IncludeChildPosition(HWND hwnd)
{
	//Only skip these, they do not size with the dlg they have fixed positions
	if((GetDlgItem(IDC_SHOW_TEXT)->GetSafeHwnd() == hwnd) ||
		(GetDlgItem(IDC_FIRST_TEXT)->GetSafeHwnd() == hwnd) ||
		(GetDlgItem(IDC_MEDS_ALLERGIES_TEXT)->GetSafeHwnd() == hwnd) ||
		(GetDlgItem(IDC_EXPAND_FIRST)->GetSafeHwnd() == hwnd))
	{
		return FALSE;
	}

	return TRUE;
}

// (j.fouts 2012-11-07 16:23) - PLID 53566 - Insert a prescription into the datalist and select it if it is our current selection
// (j.fouts 2012-12-03 14:09) - PLID 53743 - Add bCheckPrint
// (j.fouts 2012-12-03 14:10) - PLID 53745 - Add bCheckSend
// (j.fouts 2013-01-14 10:41) - PLID 54464 - Added a Pharmacy Column
// (j.fouts 2013-04-25 12:46) - PLID 53146 - Seperated Favorite Pharmacies from full pharmacy list
void CPrescriptionQueueDlg::InsertPrescriptionIntoQueue(NexTech_Accessor::_QueuePrescriptionPtr pPrescription, bool bCheckSend /*=false*/, bool bCheckPrint /*=false*/)
{
	//Check the Status filter
	if(m_pQueueStatusFilter->CurSel)
	{
		long nFilterStatus = VarLong(m_pQueueStatusFilter->CurSel->GetValue(qsfcID));

		if(nFilterStatus > 0)
		{
			//They have a filter set, map it into something useable
			NexTech_Accessor::PrescriptionStatus psFilterStatus = MapQueueStatusToAccessor(nFilterStatus);

			if(pPrescription->status != psFilterStatus)
			{
				//Our status doesn't match our filter, so we wont insert this prescription
				return;
			}
		}
	}

	// (j.jones 2012-11-16 16:23) - PLID 53085 - apply the EMN filter, if we have an EMNID
	// and the filter is set to only show prescriptions from that EMN, return if this
	// prescription does not belong to the EMN
	if(m_nCurrentlyOpenedEMNID != -1 && m_radioFilterOnEMNMeds.GetCheck() && VarLong(pPrescription->EMRPrescriptionSource->emnID->GetValue(), -1) != m_nCurrentlyOpenedEMNID) {
		//we're filtering by an EMN, and this prescription isn't on that EMN
		return;
	}

	//Create format settings for pharmacy using the combo source
	NXDATALIST2Lib::IFormatSettingsPtr pfsPharmacy(__uuidof(NXDATALIST2Lib::FormatSettings));
	pfsPharmacy->PutDataType(VT_BSTR);
	pfsPharmacy->PutFieldType(NXDATALIST2Lib::cftComboSimple);
	pfsPharmacy->PutConnection(_variant_t((LPDISPATCH)NULL));
	pfsPharmacy->PutEditable(VARIANT_TRUE);
	// (j.fouts 2013-04-25 12:46) - PLID 53146 - Seperated Favorite Pharmacies from full pharmacy list
	pfsPharmacy->PutComboSource(_bstr_t(GeneratePharmacySourceStringFull(pPrescription->FavoritePharmacies)));

	//Everything else goes in the Queue
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetNewRow();
	// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
	pRow->PutValue(pqcPrescriptionID, AsLong(pPrescription->PrescriptionID));
	pRow->PutValue(pqcMedID, AsLong(pPrescription->Medication->DrugListID));
	// (b.savon 2013-06-20 10:56) - PLID 56880
	if( pPrescription->Medication->FDBMedID.GetBSTR() != NULL ){
		pRow->PutValue(pqcFDBID, AsLong(pPrescription->Medication->FDBMedID));
	}else{
		pRow->PutValue(pqcFDBID, (long)-1);
	}
	// (s.Dhole 2012-10-29 ) - PLID 54890 - Warning ICON
	// (s.dhole 2012-12-04 13:30) - PLID 53334 check error flag
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if (m_bHasSureScriptsLicense)
	{
		// (s.dhole 2013-05-01 13:01) - PLID 54067 We allow printed prescription to validate chages
		pRow->PutValue(pqcMedErrorInd, _variant_t((pPrescription->ValidationList->ErrorStatus==elValidForSureScript   
			|| (pPrescription->status ==NexTech_Accessor::PrescriptionStatus_Legacy ) 
			)?g_cvarNull:(long)m_hIconHasSureScriptError));
	}
	pRow->CellForeColor[pqcMedErrorInd] = RGB(255,0,0);
	pRow->PutValue(pqcMedErrorIndVal, _variant_t( pPrescription->ValidationList->ErrorStatus));

	// (j.fouts 2013-02-05 14:39) - PLID 54463 - Moved this into a utility function
	CString strSIG = AsString(pPrescription->PatientExplanation);
	CString strDisplay = AsString(pPrescription->Medication->MedicationName);
	// (s.dhole 2013-04-16 09:35) - PLID 54026 Set medication description
	pRow->PutValue(pqcMedDescription, _variant_t(strDisplay));
	if(!strSIG.IsEmpty())
	{
		strDisplay += " - " + strSIG;
	}
	
	pRow->PutValue(pqcMedName, _variant_t(strDisplay));

	pRow->PutValue(pqcStatusID, _variant_t(pPrescription->status));
	// (s.dhole 2013-01-10 15:56) - PLID 53421 set pqcErrorDescription 
	pRow->PutValue(pqcErrorDescription, _bstr_t(SureScripts::LoadSureScriptErrorDesc(pPrescription->ValidationList)));
	
	// (j.fouts 2012-10-22 11:38) - PLID 53156 - New Column for patient name
	pRow->PutValue(pqcPatient, pPrescription->PatientInformation->PatientName);

	// (j.fouts 2013-01-21 12:26) - PLID 54719 - Added A Patinet UserDefined ID column and Prescriber info columns
	pRow->PutValue(pqcPatientUserDefiniedID, AsLong(pPrescription->PatientInformation->PatientUserDefinedID));

	

	if(!AsString(pPrescription->Provider->personID).IsEmpty())
	{
		pRow->PutValue(pqcPrescriberID, AsLong(pPrescription->Provider->personID));
		pRow->PutValue(pqcPrescriber, pPrescription->Provider->PersonName);
	}

	
	if(!AsString(pPrescription->Supervisor->personID).IsEmpty())
	{
		pRow->PutValue(pqcSupervisorID, AsLong(pPrescription->Supervisor->personID));
		pRow->PutValue(pqcSupervisor, pPrescription->Supervisor->PersonName);
	}

	
	if(!AsString(pPrescription->NurseStaff->personID).IsEmpty())
	{
		pRow->PutValue(pqcNurseStaffID, AsLong(pPrescription->NurseStaff->personID));
		pRow->PutValue(pqcNurseStaff, pPrescription->NurseStaff->PersonName);
	}

    pRow->PutValue(pqcPrint, bCheckPrint? g_cvarTrue : g_cvarFalse);

	NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
	pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);

	// (j.fouts 2012-12-26 11:48) - PLID 53840 - Hide the check box when we don't allow sending.
	bool bAllowSending = true;

	// (j.fouts 2012-12-27 16:50) - PLID 53160 - Created a helper function to get the Text based on a status
	pRow->PutValue(pqcStatus, _bstr_t(QueueStatusTextFromID(pPrescription->status)));

	switch(pPrescription->status)
	{
		case NexTech_Accessor::PrescriptionStatus_Printed:
			pfsPharmacy->PutEditable(VARIANT_FALSE);
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
			bAllowSending = false;
			pfsPharmacy->PutEditable(VARIANT_FALSE);
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitError:;
			pRow->PutRefCellFormatOverride(pqcStatus, pHyperLink);
			pRow->PutCellBackColor(pqcStatus, prscFailure);
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
			bAllowSending = false;
			pRow->PutRefCellFormatOverride(pqcStatus, pHyperLink);
			pRow->PutCellBackColor(pqcStatus, prscSuccess);
			pfsPharmacy->PutEditable(VARIANT_FALSE);
			break;
		case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
			bAllowSending = false;
			pRow->PutRefCellFormatOverride(pqcStatus, pHyperLink);
			pRow->PutCellBackColor(pqcStatus, prscSuccess);
			pfsPharmacy->PutEditable(VARIANT_FALSE);
			break;
		case NexTech_Accessor::PrescriptionStatus_eFaxed:
			bAllowSending = false;
			pfsPharmacy->PutEditable(VARIANT_FALSE);
			break;
		case NexTech_Accessor::PrescriptionStatus_Legacy:
			bAllowSending = false;
			break;
		// (b.savon 2013-09-04 15:15) - PLID 58212 - Add a new 'Void' type for Prescriptions
		case NexTech_Accessor::PrescriptionStatus_Void:
			{
				bAllowSending = false;
				pRow->PutValue(pqcPrint, g_cvarNull);
				pfsPharmacy->PutEditable(VARIANT_FALSE);
				pRow->PutCellBackColor(pqcStatus, ERX_NO_RESULTS_COLOR);
			}
			break;
		// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
		case NexTech_Accessor::PrescriptionStatus_DispensedInHouse:
			bAllowSending = false; 
			pRow->PutValue(pqcPrint, g_cvarNull);
			pfsPharmacy->PutEditable(VARIANT_FALSE);
			break;

	}

	// (j.fouts 2013-04-01 12:53) - PLID 53840
	if(!(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS))
	{
		pfsPharmacy->PutEditable(VARIANT_FALSE);
	}

	//pRow->PutValue(pqcSend,  g_cvarFalse);
	// (s.dhole 2012-12-04 13:30) - PLID 53334 check error flag
	// (s.dhole 2012-12-06 08:53) - PLID 54067 check License
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if (m_bHasSureScriptsLicense)
	{
		bAllowSending =((pPrescription->ValidationList->ErrorStatus==elNotValidForSureScript)?false:bAllowSending ) ;
	} 


	pRow->PutValue(pqcSend, (bAllowSending)? (bCheckSend? g_cvarTrue : g_cvarFalse) : g_cvarNull);

	// (j.fouts 2012-12-28 12:51) - PLID 54340 - Keep the old NewCrop colors
	if(!(CString((LPCTSTR)pPrescription->NewCropGUID)).IsEmpty())
	{
		pRow->PutValue(pqcStatus, "NewCrop");
		if(pPrescription->Discontinued)
		{
			pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);
		}
		else
		{
			pRow->PutBackColor(EPRESCRIBE_COLOR_ACTIVE);
		}

		//Disable Sending on NewCrop Prescriptions
		pRow->PutValue(pqcSend, g_cvarNull);
		pfsPharmacy->PutEditable(VARIANT_FALSE);
	}

	if(!pPrescription->PrescriptionDate->IsNull())
	{
		pRow->PutValue(pqcCreationDate, pPrescription->PrescriptionDate->GetValue());
	}

	// (a.wilson 2013-02-07 16:52) - PLID 55014 - add reponseid if its from a renewal.
	long nResponseID = atol(CString((LPCTSTR)pPrescription->GetDenialResponseID()));
	if (nResponseID > 0)
	{
		pRow->PutValue(pqcDenyNewRxResponseID, nResponseID);
		pfsPharmacy->PutEditable(VARIANT_FALSE);
	}
	
	//Fill the Pharmacy Column
	pRow->PutRefCellFormatOverride(pqcPharmacy, pfsPharmacy);

	// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
	// (j.fouts 2013-04-22 14:12) - PLID 54719 - PharmacyID is passed in a Pharmacy object
	if(AsString(pPrescription->Pharmacy->PharmacyID).IsEmpty())
	{
		pRow->PutValue(pqcPharmacy,-1);
	}
	else
	{
		// (j.fouts 2013-04-22 14:12) - PLID 54719 - Load and format the pharmacy's phone number
		pRow->PutValue(pqcPharmacy, AsLong(pPrescription->Pharmacy->PharmacyID));

		BOOL bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
		CString strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
		CString strPhone((LPCTSTR)pPrescription->Pharmacy->PharmacyPhone);
		if(bFormatPhoneNums && !strPhone.IsEmpty())
		{
			pRow->PutValue(pqcPharmacyPhone, _bstr_t(FormatPhone(strPhone, strPhoneFormat)));
		} else {
			pRow->PutValue(pqcPharmacyPhone, pPrescription->Pharmacy->PharmacyPhone);
		}
	}

	pRow->PutValue(pqcNewCropGUID, pPrescription->NewCropGUID);
	// (j.fouts 2013-04-22 14:12) - PLID 54719 - Load the English Sig
	pRow->PutValue(pqcEnglishSig, pPrescription->EnglishDescription);

	m_pPresQueueList->AddRowSorted(pRow, NULL);

	//If this matches the prescription we had selected, select it
	if(m_nPrescriptionID == AsLong(pPrescription->PrescriptionID))
	{
		m_pPresQueueList->PutCurSel(pRow);
	}
}

// (a.wilson 2012-10-24 10:47) - PLID 51711 - updates the state of the side controls to make the change.
void CPrescriptionQueueDlg::OnBnClickedExpandFirst()
{
	try {
		m_renewalsBox.ToggleCollapsed();
		// (j.fouts 2012-11-14 11:42) - PLID 53439 - Hide pending if renewals is shown
		m_pendingBox.ToggleCollapsed();
		UpdateShowState();
		// (s.tullis 2016-01-28 17:46) - PLID 68090
		// (s.tullis 2016-01-28 17:46) - PLID 67965
		LoadReminder(!m_pendingBox.IsCollapsed() ? m_PrescriptionNeedingAttention : m_Renewals, !m_pendingBox.IsCollapsed());
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-10-24 10:47) - PLID 51711 - this will update all the controls to their necessary state.
// (j.fouts 2012-11-06 12:26) - PLID 53574 - Modified to make it more extendable, this should not need to change when adding new boxes
void CPrescriptionQueueDlg::UpdateShowState()
{
	long nCollapsedSize = 24, nOffsetSize = 3;
	CRect rectWindow;
	CRect rectPending, rectRenewal, rectHold;

	//The total number of boxes
	long nBoxCount = m_aryCollapseableBoxes.GetCount();

	//Calculate the number of open boxes
	short nExpanded = 0;
	foreach(CCollapseableBox* pBox, m_aryCollapseableBoxes)
	{
		if(!pBox->IsCollapsed())
		{
			nExpanded++;
		}
	}

	//Calculte the size of each expanded box
	GetWindowRect(rectWindow);
	long nExpandedSize = 0;
	if(nExpanded != 0)
	{
		nExpandedSize = (rectWindow.Height() - ((nBoxCount - nExpanded) * nCollapsedSize) - ((nBoxCount-1) * nOffsetSize)) / nExpanded;
	}

	long nPrevBottom = 0;
	foreach(CCollapseableBox* pBox, m_aryCollapseableBoxes)
	{
		long nBottom = nPrevBottom;
		if(pBox->IsCollapsed()) 
		{
			nBottom += nCollapsedSize;
		}
		else
		{
			nBottom += nExpandedSize;
		}

		CRect rectControl(0,nPrevBottom,rectWindow.Width(),nBottom);
		pBox->RepositionControls(rectControl);
		nPrevBottom = nOffsetSize + nBottom;
	}

	// (j.fouts 2012-11-16 11:03) - PLID 53156 - Since the dlg scales large without current meds/allergies enforce a max height on some controls
	EnforceControlSize();
	EnforceDisabledControls();

	// (j.fouts 2012-11-20 14:23) - PLID 53840 - When setting the show state we may have reset visabily on some controls, so secure them again
	SecureControls();
	UpdateControlsForLicense();
}

// (j.fouts 2012-11-16 11:03) - PLID 53156 - Since the dlg scales large without current meds/allergies enforce a max height on some controls
void CPrescriptionQueueDlg::EnforceControlSize()
{
	// (j.fouts 2012-11-08 13:04) - PLID 53574 - Update the "no indicators" and the reviewed text
	if(m_bShowMedsAllergies)
	{	
		//Move the "no indicators" over the datalists
		CRect rectMeds;
		CRect rectAllergies;
		GetDlgItem(IDC_CURRENT_MEDS_LIST)->GetWindowRect(&rectMeds);
		GetDlgItem(IDC_ALLERGIES_LIST)->GetWindowRect(&rectAllergies);
		ScreenToClient(&rectMeds);
		ScreenToClient(&rectAllergies);

		// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added no meds/allergies check boxes
		m_checkHasNoAllergies.SetCheck(m_bHasNoAllergies);
		m_checkHasNoMeds.SetCheck(m_bHasNoMeds);
	}

	// (b.savon 2013-01-17 10:50) - PLID 54567 - Size and center then template config and edit buttons to account
	// for the missing rxneedingattention and editfavpharmacies
	if( m_nPatientID == -1 ){
		// Get the rect of the Template.Print buttons and the pharmacies button
		CRect rectTemplateBtn;
		m_btnEditTemplate.GetWindowRect(&rectTemplateBtn);
		ScreenToClient(&rectTemplateBtn);

		CRect rectPrintBtn;
		m_btnConfigureTemplates.GetWindowRect(&rectPrintBtn);
		ScreenToClient(&rectPrintBtn);

		CRect rectPharmaciesBtn;
		m_btnEditFavoritePharmacies.GetWindowRect(&rectPharmaciesBtn);
		ScreenToClient(&rectPharmaciesBtn);

		CRect rectColor;
		m_queueBkg.GetWindowRect(&rectColor);
		ScreenToClient(&rectColor);

		CRect rectQueue;
		GetDlgItem(IDC_PRESCRIPTION_QUEUE_LIST)->GetWindowRect(&rectQueue);
		ScreenToClient(&rectQueue);

		//top of datalist - top of nxcolor
		long nButtonSpaceHeight = rectQueue.top - rectColor.top;
		long nCombineButtonHeight = rectPrintBtn.Height() + rectTemplateBtn.Height();
		long nOpenSpace = nButtonSpaceHeight - nCombineButtonHeight;
		long nBuffer = nOpenSpace / 3;  //We want equal spacing between the buttons and the top of the dl and top of the color
		//((rectPharmaciesBtn.left+rectTemplateBtn.left)/2)
		// Set the height of the pharmacies button to the bottom of the template button - top of the pharmacies button
		// to account for the hidden RxNeedingAttention button
		m_btnEditTemplate.SetWindowPos(NULL,(rectQueue.right - rectTemplateBtn.Width()), rectQueue.top-nBuffer-rectPrintBtn.Height(),
												rectTemplateBtn.Width(), rectTemplateBtn.Height(), 
												SWP_NOSIZE); 
		//((rectPharmaciesBtn.left+rectPrintBtn.left)/2)
		m_btnConfigureTemplates.SetWindowPos(NULL, (rectQueue.right - rectPrintBtn.Width()), rectColor.top+nBuffer,
												rectPrintBtn.Width(), rectPrintBtn.Height(), 
												SWP_NOSIZE); 
	}
}

void CPrescriptionQueueDlg::SelChosenQueueStatusFilter(LPDISPATCH lpRow)
{
	try
	{
		// (j.jones 2012-11-19 11:14) - PLID 53758 - This filter only applies to listed medications,
		//not interactions. Interactions should have already been displayed once when the queue
		//was opened, so we don't need to show them again.
		RequeryQueue(false);
	}
	NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-17 10:56) - PLID 53797 - a function for the thread to call to prevent lockup for the user.
static void CheckForRenewals()
{
	if (GetAPI()) {
		GetAPI()->ImportSureScriptQueuedMessages(_bstr_t(GetAPISubkey()), _bstr_t(GetAPILoginToken()));
	}
}

// (a.wilson 2012-10-24 16:14) - PLID 53797 - to check for new renewals in the relay.
void CPrescriptionQueueDlg::OnBnClickedUpdateRenewals()
{
	try {
		if (!m_UpdateRenewalsThread || !m_UpdateRenewalsThread->Joinable()) {
			m_UpdateRenewalsThread = NxThread(&CheckForRenewals);
		} else {
			//thread is currently running.
		}
	} NxCatchAllThread(__FUNCTION__);
}


// (s.dhole 2013-03-01 16:02) - PLID 55399
CString GetFormattedXMLErrorResponseMessage(const CString& strMessageID, const CString& strMessage, 
																	   const COleDateTime& dtSentResponseTime)
{
	CString strFormatted;
	if( IsUserAnAdmin() ){
		strFormatted = "Message (" + strMessageID + ") XML Validation \r\n\r\n";
	}
	BOOL bTime = FALSE;
	
	if( dtSentResponseTime.GetStatus() == COleDateTime::valid && dtSentResponseTime != COleDateTime() ){
		strFormatted += "Process Time: " + FormatDateTimeForInterface(dtSentResponseTime, NULL) + "\r\n";
		bTime = TRUE;
	}
	if( bTime ){
		strFormatted += "\r\n";
	}
	strFormatted += strMessage;
	return strFormatted;
}


// (s.dhole 2012-10-26 ) - PLID 53421
void CPrescriptionQueueDlg::LeftClickPrescriptionQueueList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		switch(nCol)
		{
		case pqcMedErrorInd:
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
				if (m_bHasSureScriptsLicense && pRow )
				{
					// (s.dhole 2012-12-04 13:30) - PLID 53334 check error flag
					if (VarLong(  pRow->GetValue(pqcMedErrorIndVal),elNotValidForSureScript) ==elNotValidForSureScript)
					{ 
						LoadSureScriptError (pRow);
					}
				}
			}
			break;
		// (b.savon 2012-11-28 16:57) - PLID 51705 - Display SureScripts response message if we clicked on the Status column, it is a valid row, and
		// this is a SureScripts Response.
		case pqcStatus:
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if(pRow){
					int nPrescriptionID = -1;
					if(((nPrescriptionID = VarLong(pRow->GetValue(pqcPrescriptionID))) > 0) && IsSureScriptsResponse(VarLong(pRow->GetValue(pqcStatusID)))){
						_RecordsetPtr prs = CreateParamRecordset(
						"SELECT NexERxMessageID, NexERxResponseDisplayMessage, NexERxSentTime, NexERxSentResponseTime \r\n"
						"FROM	PatientMedications \r\n"
						"WHERE	ID = {INT} \r\n", nPrescriptionID);
						
						CString strMessageID;
						CString strNexERxResponseMessage;
						COleDateTime dtSentTime;
						COleDateTime dtSentResponseTime;
						if(!prs->eof){
							strMessageID = AdoFldString(prs->Fields, "NexERxMessageID", "");
							strNexERxResponseMessage = AdoFldString(prs->Fields, "NexERxResponseDisplayMessage", "");
							dtSentTime = AdoFldDateTime(prs->Fields, "NexERxSentTime", COleDateTime());
							dtSentResponseTime = AdoFldDateTime(prs->Fields, "NexERxSentResponseTime", COleDateTime());
						}
						// (s.dhole 2013-03-01 16:02) - PLID 55399 show xml error if messageid and prescriptionid are same
						if (strMessageID == AsString(nPrescriptionID))
						{
							MessageBox(GetFormattedXMLErrorResponseMessage(strMessageID, strNexERxResponseMessage,  dtSentResponseTime),
							(LPCTSTR)"eRx XML Validation", MB_ICONINFORMATION);
						}
						else
						{
							MessageBox(GetFormattedSureScriptsResponseMessage(strMessageID, strNexERxResponseMessage, dtSentTime, dtSentResponseTime),
							(LPCTSTR)"SureScripts Response", MB_ICONINFORMATION);
						}

						
					}
				}
			}
			break;
		case pqcMedName:
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if(pRow)
				{
					long nPrescriptionID = pRow->GetValue(pqcPrescriptionID);
					// (j.jones 2012-11-06 10:02) - PLID 52819 - pass in our EMNID and template status
					CPrescriptionEditDlg dlg(GetParent(), m_nCurrentlyOpenedEMNID);
					// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription
					long nPatientID = GetExistingPatientIDByUserDefinedID(VarLong(pRow->GetValue(pqcPatientUserDefiniedID), -1));
					// (j.fouts 2013-03-12 10:16) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
					// (b.savon 2014-08-19 16:07) - PLID 63403 - Don't popup interactions
					// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
					PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(false, LoadFullPrescription(nPrescriptionID), NULL);
					// (j.fouts 2012-11-07 16:23) - PLID 53566 - Changes could have been made, check for them and requery
					if(epdrvReturn == epdrvDeleteRx || dlg.GetChangesMade()) {
						// (j.jones 2013-01-31 17:05) - PLID 53454 - when we requery the queue to reflect the changes,
						// we must also show interactions
						RequeryQueue(true);
					}
				}
			}
			break;
		// (b.savon 2013-09-19 09:22) - PLID 58693 - The patient name on Rx Needing Attention should take you to the patients module just like Renewals Needing Attention.
		case pqcPatient:
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if(pRow)
				{
					long nPatientID = GetExistingPatientIDByUserDefinedID(AsLong(pRow->GetValue(pqcPatientUserDefiniedID)));
					//must be a valid patient selection.
					CMainFrame *pMainFrame(GetMainFrame());
					if (pMainFrame && nPatientID > 0)
					{
						pMainFrame->GotoPatient(nPatientID, true);
						CNxTabView *pView = pMainFrame->GetActiveView();
						if (pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) {
							((CPatientView *)pView)->SetActiveTab(PatientsModule::MedicationTab);
						}
					}
				}
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-11-28 16:52) - PLID 51705
BOOL CPrescriptionQueueDlg::IsSureScriptsResponse(long nStatusID)
{
	switch(nStatusID){
		case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
		case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
		case NexTech_Accessor::PrescriptionStatus_eTransmitError:
			return TRUE;
		default:
			return FALSE;
	}
}

// (b.savon 2012-11-28 16:57) - PLID 51705
CString CPrescriptionQueueDlg::GetFormattedSureScriptsResponseMessage(const CString& strMessageID, const CString& strMessage, 
																	  const COleDateTime& dtSentTime, const COleDateTime& dtSentResponseTime)
{
	CString strFormatted;
	if( IsUserAnAdmin() ){
		strFormatted = "SureScripts Response for Message (" + strMessageID + ")\r\n\r\n";
	}
	BOOL bTime = FALSE;
	if( dtSentTime.GetStatus() == COleDateTime::valid && dtSentTime != COleDateTime() ){
		strFormatted += "Sent Time: " + FormatDateTimeForInterface(dtSentTime, NULL) + "\r\n";
		bTime = TRUE;
	}
	if( dtSentResponseTime.GetStatus() == COleDateTime::valid && dtSentResponseTime != COleDateTime() ){
		strFormatted += "Response Time: " + FormatDateTimeForInterface(dtSentResponseTime, NULL) + "\r\n";
		bTime = TRUE;
	}
	if( bTime ){
		strFormatted += "\r\n";
	}
	strFormatted += strMessage;
	return strFormatted;
}

// (a.wilson 2013-01-10 09:43) - PLID 54515 - if they click on the patient's name it will send them to the patient's module.
void CPrescriptionQueueDlg::LeftClickRenewalRequestList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
		try {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow)
			{
				switch (nCol)
				{
				case rrcPatient: 
					//send the main dialog to the patient's general 1 tab.
					{
						// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
						long nPatientID = GetExistingPatientIDByUserDefinedID(AsLong(pRow->GetValue(rrcPatientID)));
						//must be a valid patient selection.
						CMainFrame *pMainFrame(GetMainFrame());
						if (pMainFrame && nPatientID > 0)
						{
							pMainFrame->GotoPatient(nPatientID, true);
							CNxTabView *pView = pMainFrame->GetActiveView();
							if (pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) {
								((CPatientView *)pView)->SetActiveTab(PatientsModule::MedicationTab);
							}
						}
					}
					break;
				case rrcPrescription:
					{
						long nRenewalID = VarLong(pRow->GetValue(rrcRenewalID), 0);
						// (j.jones 2012-11-06 10:02) - PLID 52819 - pass in our EMNID
						if (nRenewalID > 0) {
							CPrescriptionEditDlg dlg(GetParent(), m_nCurrentlyOpenedEMNID);
							// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription
							PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.ViewRenewalRequest(nRenewalID);
						}
					}
					break;
				case rrcResponseStatus:
					if (VarString(pRow->GetValue(rrcResponseReason), "") != "") {
						MessageBox(VarString(pRow->GetValue(rrcResponseReason), ""), "Response Status Reason", MB_OK);
					}
					break;
				case rrcETransmitStatus:
					{
						if (VarString(pRow->GetValue(rrcETansmitReason), "") != "") {
							MessageBox(VarString(pRow->GetValue(rrcETansmitReason), ""), "ETransmit Status Reason", MB_OK);
						}
					}
					break;
				default:
					//do nothing.
					break;
				}
			}
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-10-26 ) - PLID 53421
void CPrescriptionQueueDlg::LoadSureScriptError(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		CString strError=  VarString(pRow->GetValue(pqcErrorDescription), "");
		if (!strError.IsEmpty())
		{
		// (s.dhole 2012-12-04 13:24) - PLID 54026 load text file
		SureScripts::ShowSureScriptValidationMsg((HWND)this,GetCurrentUserName(), VarString(pRow->GetValue(pqcPatient ), ""),
				VarString(pRow->GetValue(pqcMedDescription), ""),
				FormatDateTimeForInterface(pRow->GetValue(pqcCreationDate)),strError ,
				GetPracPath(PracPath::SessionPath) ^ "PrescriptionValidation.txt");

		}
		
	} NxCatchAll(__FUNCTION__);
}

NexTech_Accessor::_QueuePrescriptionPtr CPrescriptionQueueDlg::LoadPrescription(NexTech_Accessor::_QueuePrescriptionPtr pPrescription)
{
	// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
	if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
	{
		//Also check that the database exists
		if(!FirstDataBank::EnsureDatabase(this, true)) 
		{
			//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
			NexTech_Accessor::_QueuePrescriptionPtr pNull(__uuidof(NexTech_Accessor::QueuePrescription));
			return pNull;
		}
	}

	NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));
	//Set our action to load a prescription
	pExpects->Action = NexTech_Accessor::UpdatePresQueueAction_Load;
	//We are just loading, so no need to recheck interactions
	pExpects->DrugDrugInteracts = VARIANT_FALSE;
	// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
	pExpects->ExcludeMonographInformation = VARIANT_TRUE;
	pExpects->DrugAllergyInteracts = VARIANT_FALSE;
	pExpects->DrugDiagnosisInteracts = VARIANT_FALSE;
	pExpects->RequeryQueue = VARIANT_FALSE;

	//Pass the partial prescription to the API for it to fill in the rest
	CArray<NexTech_Accessor::_QueuePrescriptionPtr, NexTech_Accessor::_QueuePrescriptionPtr> aryPrescriptions;
	aryPrescriptions.Add(pPrescription);
	Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);

	// (b.savon 2013-03-12 13:11) - PLID 55518 - Use new object structure
	pExpects->PrescriptionsToLoad = saryPrescriptions;

	// (s.dhole 2012-12-05 17:21) - PLID 54067 Pass Surescripts licence
	// (j.fouts 2013-04-24 16:55) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
	//Call the API
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(),
		_bstr_t(""), pExpects);

	//Return the prescription
	// (b.savon 2013-03-11 12:31) - PLID 55518 - Fix to use array
	NexTech_Accessor::_QueuePrescriptionPtr prescriptionLoaded;
	if( pResults->PrescriptionsLoaded != NULL ){
		Nx::SafeArray<IUnknown *> saryResults = pResults->PrescriptionsLoaded;
		prescriptionLoaded = saryResults[0];
	}else{
		ThrowNxException("Error in PrescriptionQueueDlg::LoadPrescription.\r\n\r\nUnable to load prescription!");
	}

	return prescriptionLoaded;
}

void CPrescriptionQueueDlg::DblClickCellPrescriptionQueueList(LPDISPATCH lpRow, short nColIndex)
{

}

// (a.wilson 2012-11-02 14:37) - PLID 53797 - created to generate renewal list.
// (b.savon 2013-04-22 16:17) - PLID 54472 - Override From filter
void CPrescriptionQueueDlg::RequeryRenewalRequests(const COleDateTime &dtFromOverride /*= g_cdtNull*/)
{
	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	
	if (pApi)
	{
		m_pRenewalList->Clear();
		//get filters.
		NexTech_Accessor::RenewalResponseStatusFilter eResponseStatus = NexTech_Accessor::RenewalResponseStatusFilter_All;
		NexTech_Accessor::RenewalQueueStatusFilter eTransmitStatus = NexTech_Accessor::RenewalQueueStatusFilter_All;
		// (r.gonet 2016-01-22) - PLID 67973 - Gather the selected prescriber IDs.
		const CVariantArray& arySelectedIDs = m_renewalPrescriberController.GetSelectedIDs();
		Nx::SafeArray<BSTR> saPrescriberIDs;
		for (int i = 0; i < arySelectedIDs.GetSize(); i++) {
			saPrescriberIDs.Add(AsString(arySelectedIDs[i]), true);
		}

		if (m_pRenewalFilterResponseList->GetCurSel())
			eResponseStatus = (NexTech_Accessor::RenewalResponseStatusFilter)VarLong(m_pRenewalFilterResponseList->GetCurSel()->GetValue(rrfcID), 0);
		if (m_pRenewalFilterTransmitList->GetCurSel())
			eTransmitStatus = (NexTech_Accessor::RenewalQueueStatusFilter)VarLong(m_pRenewalFilterTransmitList->GetCurSel()->GetValue(rrfcID), 0);

		// (b.eyers 2016-01-28) - PLID 67985 - when etransmitstatus is -2, that means we want to filter on statuses that require action
		NexTech_Accessor::_NullableBoolPtr pNeedsAction(__uuidof(NexTech_Accessor::NullableBool));
		if (eTransmitStatus == -2) {
			pNeedsAction->SetBool(VARIANT_TRUE);
			eTransmitStatus = NexTech_Accessor::RenewalQueueStatusFilter_All;
		}
		else {
			pNeedsAction->SetNull();
		}
		// (b.savon 2013-04-22 16:17) - PLID 54472 - Override From filter
		COleDateTime dtFrom;
		if( dtFromOverride == g_cdtNull || dtFromOverride.GetStatus() != COleDateTime::valid ){
			dtFrom = m_dtpRenewalFilterFrom.GetDateTime();	
		}else{
			dtFrom = dtFromOverride;
			m_dtpRenewalFilterFrom.SetValue(dtFrom);
		}
		COleDateTime dtTo = m_dtpRenewalFilterTo.GetDateTime();
		//get list of renewal based on filters.
		NexTech_Accessor::_ERxRenewalRequestQueueArrayPtr pRenewalRequestsArray(__uuidof(NexTech_Accessor::ERxRenewalRequestQueueArray));

		// (r.gonet 2016-01-22) - PLID 67973 - Changed to pass an empty string as the single prescriber ID and instead pass an array of selected prescriber IDs.
		// (b.eyers 2016-01-28) - PLID 67985 - Pass the needsaction bool to the API
		pRenewalRequestsArray = pApi->UpdateRenewalRequestQueue(GetAPISubkey(), GetAPILoginToken(), _bstr_t(GetExistingPatientUserDefinedID(m_nPatientID)), 
			eResponseStatus, eTransmitStatus, dtFrom, dtTo, saPrescriberIDs, pNeedsAction);
		
		if (pRenewalRequestsArray && pRenewalRequestsArray->GetRenewalRequests()) {
			long nPendingCount = 0;
			NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
			pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);
			CArray<NexTech_Accessor::_ERxRenewalRequestQueuePtr> arRenewalRequestsQueue;
			Nx::SafeArray<IUnknown *> sarRenewalRequests(pRenewalRequestsArray->GetRenewalRequests());
			sarRenewalRequests.ToArray(arRenewalRequestsQueue);
			//ensure it converted correctly
			if (!arRenewalRequestsQueue.IsEmpty()) {
				for (int i = 0; i < arRenewalRequestsQueue.GetCount(); i++)
				{
					NexTech_Accessor::_ERxRenewalRequestQueuePtr RenewalRequest = arRenewalRequestsQueue.GetAt(i);
					if (RenewalRequest) {
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRenewalList->GetNewRow();
						pRow->PutValue(rrcRenewalID, RenewalRequest->GetID());
						pRow->PutValue(rrcPatientID, RenewalRequest->GetpatientID());
						pRow->PutValue(rrcPatient, RenewalRequest->GetPatientName());
						if (VarString(RenewalRequest->GetPatientName(), "<Unknown Patient>") != "<Unknown Patient>")
							pRow->PutRefCellFormatOverride(rrcPatient, pHyperLink);
						// (a.wilson 2013-01-03 11:29) - PLID 54410 - adding prescriber for filter
						pRow->PutValue(rrcPrescriberID, RenewalRequest->GetPrescriberID());
						pRow->PutValue(rrcPrescriber, _bstr_t(RenewalRequest->GetPrescriberName()));
						pRow->PutValue(rrcPharmacy, _bstr_t(RenewalRequest->GetPharmacyName()));
						pRow->PutValue(rrcPrescription, _bstr_t(RenewalRequest->GetDrugDescription()));
						COleDateTime dtDate(RenewalRequest->GetReceivedDate());
						pRow->PutValue(rrcDate, _variant_t(dtDate, VT_DATE));
						CString strResponseReason = Trim(VarString(RenewalRequest->GetResponseReason(), ""));
						CString strQueueReason = Trim(VarString(RenewalRequest->GetQueueReason(), ""));
						pRow->PutValue(rrcResponseReason, _bstr_t(strResponseReason));
						pRow->PutValue(rrcETansmitReason, _bstr_t(strQueueReason));
						if (!strResponseReason.IsEmpty())
							pRow->PutRefCellFormatOverride(rrcResponseStatus, pHyperLink);
						if (!strQueueReason.IsEmpty())
							pRow->PutRefCellFormatOverride(rrcETransmitStatus, pHyperLink);

						switch (RenewalRequest->GetResponseStatus())
						{
							case NexTech_Accessor::RenewalResponseStatus_Approved:
								pRow->PutValue(rrcResponseStatusID, (long)NexTech_Accessor::RenewalResponseStatus_Approved);
								pRow->PutValue(rrcResponseStatus, _bstr_t("Approved"));
								pRow->PutCellBackColor(rrcResponseStatus, prscSuccess);
								break;
							case NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges:
								pRow->PutValue(rrcResponseStatusID, (long)NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges);
								pRow->PutValue(rrcResponseStatus, _bstr_t("Approved with Changes"));
								pRow->PutCellBackColor(rrcResponseStatus, prscSuccess);
								break;
							case NexTech_Accessor::RenewalResponseStatus_Denied:
								pRow->PutValue(rrcResponseStatusID, (long)NexTech_Accessor::RenewalResponseStatus_Denied);
								pRow->PutValue(rrcResponseStatus, _bstr_t("Denied"));
								pRow->PutCellBackColor(rrcResponseStatus, prscFailure);
								break;
							case NexTech_Accessor::RenewalResponseStatus_DeniedNewRx:
								pRow->PutValue(rrcResponseStatusID, (long)NexTech_Accessor::RenewalResponseStatus_DeniedNewRx);
								pRow->PutValue(rrcResponseStatus, _bstr_t("Denied with Rewrite"));
								pRow->PutCellBackColor(rrcResponseStatus, prscFailure);
								break;
							case NexTech_Accessor::RenewalResponseStatus_Pending:
							default:
								pRow->PutValue(rrcResponseStatusID, (long)NexTech_Accessor::RenewalResponseStatus_Pending);
								pRow->PutValue(rrcResponseStatus, _bstr_t("Pending"));
								nPendingCount++;
								break;
						}

						switch (RenewalRequest->GetQueueStatus())
						{
							case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
								pRow->PutValue(rrcETransmitStatusID, (long)NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized);
								pRow->PutValue(rrcETransmitStatus, _bstr_t("eTransmit Authorized"));
								break;
							case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
								pRow->PutValue(rrcETransmitStatusID, (long)NexTech_Accessor::PrescriptionStatus_eTransmitPending);
								pRow->PutValue(rrcETransmitStatus, _bstr_t("eTransmit Pending"));
								break;
							case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
								pRow->PutValue(rrcETransmitStatusID, (long)NexTech_Accessor::PrescriptionStatus_eTransmitSuccess);
								pRow->PutValue(rrcETransmitStatus, _bstr_t("eTransmit Success"));
								pRow->PutCellBackColor(rrcETransmitStatus, prscSuccess);
								break;
							case NexTech_Accessor::PrescriptionStatus_eTransmitError:
								pRow->PutValue(rrcETransmitStatusID, (long)NexTech_Accessor::PrescriptionStatus_eTransmitError);
								pRow->PutValue(rrcETransmitStatus, _bstr_t("eTransmit Error"));
								pRow->PutCellBackColor(rrcETransmitStatus, prscFailure);
								break;
							default:
								pRow->PutValue(rrcETransmitStatusID, (long)NexTech_Accessor::PrescriptionStatus_Incomplete);
								pRow->PutValue(rrcETransmitStatus, _bstr_t("Pending"));
								break;
						}				

						m_pRenewalList->AddRowAtEnd(pRow, NULL);
					}
				}
				//update renewal title based on whether visible pending renewals are within the list.
				m_nxstaticRenewalText.SetWindowText(FormatString("Renewal Requests %s", (nPendingCount > 0 ? FormatString("(%li)", nPendingCount) : "")));
				if (nPendingCount > 0)
					m_nxstaticRenewalText.SetColor(RGB(255, 50, 50));
				else
					m_nxstaticRenewalText.SetColor(RGB(0, 0, 0));
				m_pRenewalList->Sort();
			} else {
				m_nxstaticRenewalText.SetWindowText("Renewal Requests");
				m_nxstaticRenewalText.SetColor(RGB(0, 0, 0));
			}
		} 
	}		
}

// (j.fouts 2012-11-06 12:27) - PLID 53574 - Toggle collapse/expand when this is clicked
void CPrescriptionQueueDlg::OnBnClickedMedsAllergiesBtn()
{
	try {
		m_medsAllergiesBox.ToggleCollapsed();
		UpdateShowState();
	} NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-07 16:23) - PLID 53566 - Maintain m_nPrescriptionID
void CPrescriptionQueueDlg::SelChangedPrescriptionQueueList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		if(lpNewSel)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
			m_nPrescriptionID = VarLong(pRow->GetValue(pqcPrescriptionID));
		}
		else
		{
			m_nPrescriptionID = -1;
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-08 10:31) - PLID 53574 - Color this the same as the medications tab
void CPrescriptionQueueDlg::RequeryFinishedCurrentMedsList(short nFlags)
{
	try {

		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		_variant_t varInfoButtonIcon = (long)m_hInfoButtonIcon;

		//we don't need to check for the license because maybe they had newcrop and ditched it, they'll still need to see the prescriptions
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedsList->GetFirstRow();pRow;pRow = pRow->GetNextRow()) 
		{
			// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
			pRow->PutValue(cmlcInfoButton, varInfoButtonIcon);

			CString strNewCropGUID = VarString(pRow->GetValue(cmlcNewCropGUID), "");
			
			// (b.savon 2013-01-25 12:58) - PLID 54854 - Rework this to allow discontinued for our erx
			BOOL bIsDiscoed = VarBool(pRow->GetValue(cmlcDiscontinued), FALSE);
			long nFromFDB = VarLong(pRow->GetValue(cmlcFDBID), -1);
			if (bIsDiscoed) {
				// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
				pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);			
			}else{
				if (!strNewCropGUID.IsEmpty()) {
					// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
					pRow->PutBackColor(EPRESCRIBE_COLOR_ACTIVE);			
				}else if( nFromFDB > 0 ){ // (b.savon 2013-01-07 13:18) - PLID 54459 - Color imported meds
					//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
					if(VarBool(pRow->GetValue(cmlcFDBOutOfDate), FALSE)) {
						pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					}
					else {
						pRow->PutBackColor(ERX_IMPORTED_COLOR);
					}
				}
			}
		}	
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-08 10:31) - PLID 53574 - Color this the same as the medications tab
void CPrescriptionQueueDlg::RequeryFinishedAllergiesList(short nFlags)
{
	try {
		//we don't need to check for the license because maybe they had newcrop and ditched it, they'll still need to see the prescriptions
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAllergiesList->GetFirstRow();pRow;pRow = pRow->GetNextRow()) 
		{	
			BOOL bFromNewCrop = VarBool(pRow->GetValue(alcFromNewCrop), FALSE);
			
			// (b.savon 2013-01-25 12:58) - PLID 54854 - Rework this to allow discontinued for our erx
			BOOL bIsDiscoed = VarBool(pRow->GetValue(alcIsDiscontinued), FALSE);
			long nFromFDB = VarLong(pRow->GetValue(alcFDBImport), -1);
			if (bIsDiscoed) {
				// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
				pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);			
			}else{
				if (bFromNewCrop) {
					// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
					pRow->PutBackColor(EPRESCRIBE_COLOR_ACTIVE);			
				}else if( nFromFDB > 0 ){ // (b.savon 2013-01-07 13:18) - PLID 54459 - Color imported meds
					//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
					if(VarBool(pRow->GetValue(alcFDBOutOfDate), FALSE)) {
						pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					}
					else {
						pRow->PutBackColor(ERX_IMPORTED_COLOR);
					}
				}
			}
		}	
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-14 11:42) - PLID 53743 - Moved printing prescriptions to the queue
void CPrescriptionQueueDlg::OnBnClickedPrintAllPresctiptions()
{
	try
	{	
		if (m_pTemplateList->CurSel == -1) {
			CString msg;
			msg.Format("There is no template selected. The prescription%s cannot be printed.",
				(m_nCountPrintSelected > 1 || m_nCountPrintSelected == 0? "s " : ""));
			MsgBox(MB_OK|MB_ICONINFORMATION, msg);
			return;
		}

		// (j.fouts 2013-09-17 16:53) - PLID 58496 - Keep a seperate list for each patinet, some may be removed
		typedef std::pair<long,CString> IDNamePair;
		typedef std::pair<CArray<long,long>,CArray<CString,CString&>> ListPair;
		typedef boost::unordered_map<IDNamePair,ListPair> PatientMap;

		PatientMap mapSelectedPatients;

		CArray<long, long> aryPrescriptionIDs;
		CArray<CString, CString&> aryChangeStatusPrescriptionIDs;
		long nCompletedPrescriptions = 0;

		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();pRow;pRow = pRow->GetNextRow())
		{
			if(m_nCountPrintSelected == 0 || VarBool(pRow->GetValue(pqcPrint), FALSE))
			{
				// (b.savon 2014-12-03 08:49) - PLID 64142 - Display a message to the user if they try to merge a NewCrop prescription to Word from within Nextech
				if (!AsString(pRow->GetValue(pqcNewCropGUID)).IsEmpty()){
					MessageBox("At least 1 prescription selected was written in NewCrop.  Please unselect it and try again.  If you wish to print the NewCrop prescription, please do so from NewCrop.", "Nextech", MB_ICONEXCLAMATION);
					return;
				}

				long nPrescriptionID = VarLong(pRow->GetValue(pqcPrescriptionID));

				//Create a key of the ID and Name
				long nID = GetExistingPatientIDByUserDefinedID(VarLong(pRow->GetValue(pqcPatientUserDefiniedID)));
				IDNamePair key(nID, VarString(pRow->GetValue(pqcPatient)));

				//Don't change the status of the Prescription to 'Printed' if the status was an eTransmit Status, other than error
				NexTech_Accessor::PrescriptionStatus queueStaus = (NexTech_Accessor::PrescriptionStatus)VarLong(pRow->GetValue(pqcStatusID), 0 /*No Status*/);
				if(queueStaus != NexTech_Accessor::PrescriptionStatus_eFaxed &&
					queueStaus != NexTech_Accessor::PrescriptionStatus_Printed &&
					queueStaus != NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized &&
					queueStaus != NexTech_Accessor::PrescriptionStatus_eTransmitPending &&
					queueStaus != NexTech_Accessor::PrescriptionStatus_eTransmitSuccess)
				{
					if(AsString(pRow->GetValue(pqcNewCropGUID)).IsEmpty())
					{
						CString strPrescriptionID;
						strPrescriptionID.Format("%li", nPrescriptionID);
						//The second element in our pair is the status change list
						mapSelectedPatients[key].second.Add(strPrescriptionID);
					}
				}
				else
				{
					++nCompletedPrescriptions;
				}

				//The first element in our pair is the prescription ID list
				mapSelectedPatients[key].first.Add(nPrescriptionID);
			}
		}

		//Do the interaction checks before the template checks because the number of selected 
		//prescriptions may change from this.
		for(PatientMap::iterator i = mapSelectedPatients.begin();i!=mapSelectedPatients.end();i++)
		{
			// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
			//Iterator's first is the key, which has ID then Name
			if(PromptInteractions(i->first.first, i->first.second))
			{
				//Iterator's second is the value, which has ID list then status change list
				aryPrescriptionIDs.Append(i->second.first);
				aryChangeStatusPrescriptionIDs.Append(i->second.second);
			}
		}
		//Bail if we just elimiated all prescriptions
		if(aryPrescriptionIDs.GetSize() == 0)
		{
			return;
		}

		// Get template to merge to
		CString strTemplateName, strTemplatePath;
		strTemplateName = (LPCSTR)_bstr_t(m_pTemplateList->Value[m_pTemplateList->CurSel][0]);

		// (j.jones 2008-06-09 14:37) - PLID 29154 - use the default
		if(strTemplateName == USE_DEFAULT_TEMPLATE_TEXT) {

			// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
			// based on how many prescriptions are printed
			long nCountPrescriptions = aryPrescriptionIDs.GetSize();
			BOOL bExactCountFound = FALSE;
			BOOL bOtherCountFound = FALSE;
			strTemplateName = GetDefaultPrescriptionTemplateByCount(nCountPrescriptions, bExactCountFound, bOtherCountFound);

			if(strTemplateName == "") {
				CString msg;
				msg.Format("There is no default template set up. The prescription%s cannot be printed.\n"
					"Please edit the 'Prescription Template Setup' to configure your default prescription templates.",
					(nCountPrescriptions > 1? "s " : ""));
				MsgBox(MB_OK|MB_ICONINFORMATION, msg);
				return;
			}
			//if no template was found for the exact count, and there are some for other counts,
			//ask if they wish to continue or not (will use the standard default otherwise)
			else if(!bExactCountFound && bOtherCountFound) {
				CString str;
				str.Format("There is no default template configured for use with %li prescription%s, "
					"but there are templates configured for other counts of prescriptions.\n\n"
					"Would you like to continue merging using the standard prescription template?",
					nCountPrescriptions, nCountPrescriptions == 1 ? "" : "s");
				if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					AfxMessageBox("The prescription will not be be printed.\n"
						"Please edit the 'Prescription Template Setup' to configure your default prescription templates.");
					return;
				}
			}
		}

		// (j.fouts 2013-04-23 10:53) - PLID 52907 - Warn when printing
		if(nCompletedPrescriptions > 0)
		{
			if(MessageBox(FormatString("Printing prescriptions that have completed statuses is not recommended. "
				"%li selected prescription%s a completed status. Please be sure not to give duplicate copies to a patient.\n\r"
				"Are you sure you wish to continue printing this prescription?", nCompletedPrescriptions, nCompletedPrescriptions==1?" has":"s have")
				, "Continue Printing?", MB_ICONQUESTION|MB_YESNO) == IDNO)
			{
				return;
			}
		}

		if(DontShowMeAgain(this, "If you continue printing you will no longer be able "
			"to edit or delete the printed prescriptions.\n\r"
			"Are you sure you wish to continue printing?"
			, "NexERx_PrintLockWarning", "Warning", FALSE, TRUE) == IDNO)
		{
			return;
		}

		strTemplatePath = GetTemplatePath("Forms", strTemplateName);			

		// (j.jones 2008-05-16 09:19) - PLID 29732 - made merging prescriptions a global function
		// (c.haag 2009-08-18 13:12) - PLID 15479 - Added option to merge straight to printer
		// (j.fouts 2013-03-05 08:34) - PLID 55427 - Removed PatientID paramater
		if(MergePrescriptionsToWord(aryPrescriptionIDs, strTemplateName, strTemplatePath,  m_checkSendToPrinter.GetCheck()? TRUE : FALSE))
		{
			// (j.fouts 2012-12-27 16:50) - PLID 53160 - Call the API to set the status to printed
			//Now that everything has been merged to word/printer we can mark them as printed
			SetPrescriptionStatus(aryChangeStatusPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_Printed);
		}
		aryPrescriptionIDs.RemoveAll();

		UpdatePreviewButton();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-14 11:42) - PLID 53439 - Handle changing patients
void CPrescriptionQueueDlg::ChangePatient(long nPatientID, bool bForceRefresh /*=false*/)
{
	if(m_nPatientID == nPatientID && !bForceRefresh)
	{
		return;
	}

	m_nPatientID = nPatientID;

	if(m_bShowMedsAllergies)
	{
		// (j.jones 2013-04-05 13:38) - PLID 56114 - moved the requery of allergies and current meds
		// to their own function
		RequeryAllergiesAndMeds(true, true);
	}

	// (j.fouts 2012-10-22 11:34) - PLID 53156 - If we are not in patientless mode hide the Patient Column
	if(nPatientID >= 0)
	{
		NXDATALIST2Lib::IColumnSettingsPtr pColumn = m_pPresQueueList->GetColumn(pqcPatient);
		pColumn->StoredWidth = -1;
		pColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth;

		// (j.fouts 2013-01-21 12:26) - PLID 54719 - Added A Patinet UserDefined ID column
		pColumn = m_pPresQueueList->GetColumn(pqcPatientUserDefiniedID);
		pColumn->StoredWidth = -1;
		pColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
		// (a.wilson 2013-01-03 17:30) - PLID 51711 - hide patient field if its a patient specific area.
		pColumn = m_pRenewalList->GetColumn(rrcPatient);
		pColumn->StoredWidth = -1;
		pColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
		//patient id column as well.
		pColumn = m_pRenewalList->GetColumn(rrcPatientID);
		pColumn->StoredWidth = -1;
		pColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
		// (b.eyers 2016-01-22) - PLID 67966 - Moved this here so this only gets called when moving between patients in the medication tab
		RequeryQueue();
		// (b.eyers 2016-01-28) - PLID 67985 - moved this up so it only gets called moving between patients in the medication tab
		RequeryRenewalRequests();
	}

	//RequeryQueue();
	//RequeryRenewalRequests();
	// (r.gonet 09/20/2013) - PLID 58396 - Reset the text color of the medication history button
	// and try to see if this patient has any medication history (threaded). If so, we'll display it as red text.
	m_btnMedicationHistory.SetTextColor(RGB(0,0,0));
	MedicationHistoryUtils::BeginBackgroundRequestMedHistory(this->GetSafeHwnd(), m_nPatientID);
}

// (j.fouts 2012-11-14 12:01) - PLID 53439 - Allow the calling code to set the color
void CPrescriptionQueueDlg::SetColor(OLE_COLOR nNewColor)
{
	// (j.fouts 2012-09-26 10:33) - PLID 52973 - Color the new NxColors
	// (c.haag 2009-01-27 16:13) - PLID 32868 - If this was invoked from an EMN template, use the template color
	// (a.wilson 2012-10-24 11:13) - PLID 51711 - add renewals
	// (j.fouts 2012-11-06 12:20) - PLID 53574 - Added meds and allergies
	m_queueBkg.SetColor(nNewColor);
	m_renewalBkg.SetColor(nNewColor);
	m_medsAllergiesBkg.SetColor(nNewColor);
}

// (j.fouts 2012-11-14 12:01) - PLID 53743 - Update the botton text and save the property
void CPrescriptionQueueDlg::OnBnClickedSendToPrinter()
{
	try
	{
		UpdatePrintPreviewText();
		SetRemotePropertyInt("PatientMedMergePrescriptionToPrinter", m_checkSendToPrinter.GetCheck(), 0, GetCurrentUserName());
	}
	NxCatchAll(__FUNCTION__);
}

void CPrescriptionQueueDlg::UpdatePrintPreviewText()
{
	try
	{
		// (j.fouts 2012-11-30 10:06) - PLID 53954 - Removed send/print all buttons and replaced with a check all option
		// (j.jones 2016-02-22 11:01) - PLID 68353 - the button now says "Print Preview", not "Preview"
		SetDlgItemText(IDC_PRINT_ALL_PRESCTIPTIONS, m_checkSendToPrinter.GetCheck()? "Print" : "Print Preview");
	}
	NxCatchAll(__FUNCTION__);
}
void CPrescriptionQueueDlg::EditingFinishedPrescriptionQueueList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		switch(nCol)
		{
			case pqcPrint:
			{
				// (j.fouts 2012-11-30 10:06) - PLID 53954 - Moved into a function
				UpdatePreviewButton();
				SecureControls();
				break;
			}
			case pqcSend:
			{
				// (j.fouts 2012-11-30 10:06) - PLID 53954 - Moved into a function
				UpdateSendButton();
				SecureControls();
				break;
			}
			// (j.fouts 2013-01-14 10:41) - PLID 54464 - Added a dropdown for the pharmacies column
			case pqcPharmacy:
			{
				if(lpRow)
				{
 					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

					// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
					if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
					{
						//Also check that the database exists
						if(!FirstDataBank::EnsureDatabase(this, true)) 
						{
							//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
							pRow->PutValue(nCol, varOldValue);	//Revert the change
							return;
						}
					}

					if(varNewValue.vt == VT_EMPTY)
					{
						pRow->PutValue(nCol, varOldValue);	//Revert the change
						return;
					}

					// (j.fouts 2013-04-01 10:15) - PLID 54464 - If they got here somehow on an invalid prescription, tell them they cannot change this value
					NexTech_Accessor::PrescriptionStatus nStatus = (NexTech_Accessor::PrescriptionStatus)VarLong(pRow->GetValue(pqcStatusID));
					if(nStatus == NexTech_Accessor::PrescriptionStatus_Printed ||
						nStatus == NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized ||
						nStatus == NexTech_Accessor::PrescriptionStatus_eTransmitPending ||
						nStatus == NexTech_Accessor::PrescriptionStatus_eTransmitSuccess ||
						nStatus == NexTech_Accessor::PrescriptionStatus_eFaxed)
					{
						MessageBox("This prescription has been completed. The selected pharmacy cannot change.", "Practice", MB_ICONEXCLAMATION|MB_OK);
						pRow->PutValue(nCol, varOldValue);	//Revert the change
						return;
					} 
					else if (!AsString(pRow->GetValue(pqcNewCropGUID)).IsEmpty())
					{
						MessageBox("This prescription is from NewCrop. The selected pharmacy cannot change.", "Practice", MB_ICONEXCLAMATION|MB_OK);
						pRow->PutValue(nCol, varOldValue);	//Revert the change
						return;
					}
					long nPharmacyID = AsLong(varNewValue);
					long nOldPharmacyID = AsLong(varOldValue);
					bool bSaveChanges = false;
					bool bNewPharmacyAdded = false;

					if(nPharmacyID == 0)
					{
						nPharmacyID = -1;
					}

					if(nPharmacyID == -2)
					{
						//Add
						CPharmacyDirectorySearchDlg dlg(this);
						dlg.m_bMultiMode = FALSE;

						if (IDOK == dlg.DoModal()) {
							long nSelectedPharmacyID = dlg.m_nSelectedID;

							if (nSelectedPharmacyID != -1) {
								if(nOldPharmacyID != nSelectedPharmacyID)
								{
									bSaveChanges = true;
									bNewPharmacyAdded = true;
									nPharmacyID = nSelectedPharmacyID;
								}
							}
							else
							{
								pRow->PutValue(pqcPharmacy, nOldPharmacyID);
							}
						}
						else
						{
							pRow->PutValue(pqcPharmacy, nOldPharmacyID);
						}
					}
					else 
					{
						if(nOldPharmacyID != nPharmacyID)
						{
							bSaveChanges = true;
						}
					}

					if(bSaveChanges)
					{
						//Take ownership
						CPrescriptionEditDlg dlg(this);
						// (j.fouts 2013-03-12 10:16) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
						// (b.savon 2014-08-19 16:12) - PLID 63403 - Don't show interactions.
						// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
						dlg.EditPrescription(false, LoadFullPrescription(AsLong(pRow->GetValue(pqcPrescriptionID))), NULL, FALSE, true);

						// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
						NexTech_Accessor::_PrescriptionCommitPtr pCommit(__uuidof(NexTech_Accessor::PrescriptionCommit));
						// (b.savon 2013-03-12 13:11) - PLID 55518 - Use new object structure
						NexTech_Accessor::_QueuePrescriptionPtr pRxQueue(__uuidof(NexTech_Accessor::QueuePrescription));
						// (j.fouts 2013-04-22 14:12) - PLID 54719 - PharmacyID is passed in a Pharmacy object
						NexTech_Accessor::_ERxQueuePharmacyPtr pPharmacy(__uuidof(NexTech_Accessor::ERxQueuePharmacy));
						pCommit->Prescription = pRxQueue;
						pCommit->Prescription->PrescriptionID = _bstr_t(AsString(pRow->GetValue(pqcPrescriptionID)));
						pCommit->CommitPharmacyID = TRUE;
						pCommit->Prescription->Pharmacy = pPharmacy;

						if(nPharmacyID == -1)
						{
							pCommit->Prescription->Pharmacy->PharmacyID = _bstr_t("");
						}
						else
						{
							CString strPharmacyID;
							strPharmacyID.Format("%li", nPharmacyID);
							pCommit->Prescription->Pharmacy->PharmacyID = _bstr_t(strPharmacyID);
						}

						CArray<NexTech_Accessor::_PrescriptionCommitPtr,NexTech_Accessor::_PrescriptionCommitPtr> aryPrescriptions;
						aryPrescriptions.Add(pCommit);

						Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);
					// (s.dhole 2013-01-31 13:05) - PLID 54975 now refresh error
						NexTech_Accessor::_SavePrescriptionResultArrayPtr pResults = GetAPI()->SavePrescription(GetAPISubkey(), GetAPILoginToken(), saryPrescriptions);

						if(bNewPharmacyAdded)
						{
							//Every pharmacy list is going to need the new pharmacy in their combo, we need to requery
							RequeryQueue(false);
						}
						else
						{
							// s.dhole  always one record
							Nx::SafeArray<IUnknown *> saryResults = pResults->Results;
							foreach(NexTech_Accessor::_SavePrescriptionResultPtr pResult, saryResults){
								// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
								if (m_bHasSureScriptsLicense &&
									pResult->ErrorStatus !=NexTech_Accessor::eRxErrorStatus_eRxNoError ){
									pRow->PutValue(pqcErrorDescription, _bstr_t(SureScripts::LoadSureScriptErrorDesc(pResult->ValidationList)));
									pRow->PutValue(pqcSend,  g_cvarNull);
									pRow->PutValue(pqcMedErrorInd, _variant_t((long)m_hIconHasSureScriptError));

									pRow->CellForeColor[pqcMedErrorInd] = RGB(255,0,0);
									pRow->PutValue(pqcMedErrorIndVal, _variant_t( pResult->ErrorStatus ));
									
								}
								else{
									pRow->PutValue(pqcErrorDescription, _bstr_t(SureScripts::LoadSureScriptErrorDesc(pResult->ValidationList)));
									_variant_t varSend = pRow->GetValue(pqcSend);
									pRow->PutValue(pqcMedErrorInd, g_cvarNull);
									pRow->PutValue(pqcMedErrorIndVal, _variant_t(0));
									// if value is not bool than we do not change  
									if(varSend.vt != VT_BOOL)
									{
										pRow->PutValue(pqcSend,  g_cvarFalse );
									}
								}
								UpdateSendButton();
								SecureControls();
							}
						}
							 	

					}
				}
			}
			default:
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CPrescriptionQueueDlg::OnBnClickedBtnConfigureTemplates()
{
	try {

		//previously making a template be the default required this permission,
		//so I chose to use the same permission for the new advanced template setup
		if(!CheckCurrentUserPermissions(bioPatientMedication,sptWrite)) {
			return;
		}

		CPrescriptionTemplateSetupDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			//they may have changed the default template, so update the colors if they did

			CString strDefTemplate = GetPropertyText("DefaultPrescriptionFilename", "", 0, false);

			//do a manual search to make sure we use CompareNoCase when updating the default template's coloring
			
			BOOL bFound = FALSE;
			long nIndexToSwitchTo = -1;

			for(int i=0;i<m_pTemplateList->GetRowCount() && !bFound;i++) {

				CString strTemplate = VarString(m_pTemplateList->GetValue(i,0), "");
				if(strTemplate.CompareNoCase(strDefTemplate) == 0) {
					bFound = TRUE;

					nIndexToSwitchTo = i;
				}
			}

			//now set the colors
			if(m_nDefTemplateRowIndex != nIndexToSwitchTo) {
				NXDATALISTLib::IRowSettingsPtr(m_pTemplateList->GetRow(m_nDefTemplateRowIndex))->ForeColor = NXDATALISTLib::dlColorNotSet;
				NXDATALISTLib::IRowSettingsPtr(m_pTemplateList->GetRow(nIndexToSwitchTo))->ForeColor = RGB(255,0,0);
				m_nDefTemplateRowIndex = nIndexToSwitchTo;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved merge templates to the queue
// (c.haag 2009-09-15 12:52) - PLID 21400 - Moved code to requery the word template list into its own utility function
void CPrescriptionQueueDlg::RequeryWordTemplateList()
{
	// Load the name of the default template
	CString strDefTemplate = GetPropertyText("DefaultPrescriptionFilename", "", 0, false);

	// (c.haag 2009-09-15 13:03) - PLID 21400 - Retain the last selected value
	CString strPrevSel = (m_pTemplateList->CurSel == -1) ? "" : VarString(m_pTemplateList->GetValue(m_pTemplateList->CurSel, 0), "");

	m_pTemplateList->Clear();

	// (j.jones 2008-06-09 14:30) - PLID 29154 - add a "use default template" option
	NXDATALISTLib::IRowSettingsPtr pNewRow = m_pTemplateList->GetRow(-1);
	pNewRow->PutValue(0, USE_DEFAULT_TEMPLATE_TEXT);
	m_pTemplateList->AddRow(pNewRow);

	NXDATALISTLib::IRowSettingsPtr pRow;

	//build file list (automatically selecting the default template)

	// (a.walling 2007-06-14 16:16) - PLID 26342 - Support Word2007 templates

	CFileFind finder;
	CString strFind = GetSharedPath() ^ "Templates\\Forms";
	int i;
	// interestingly, searching for *.dot will also match *.dot*
	if (finder.FindFile(strFind ^ "*.dot"))
	{
		i = 0;
		long nIndex;
		while (finder.FindNextFile())
		{
			CString str = finder.GetFileName();
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			if ((str.Right(5).CompareNoCase(".dotx") == 0) || (str.Right(5).CompareNoCase(".dotm") == 0) || (str.Right(4).CompareNoCase(".dot") == 0)) {
				pRow = m_pTemplateList->GetRow(-1);
				pRow->Value[0] = _bstr_t(finder.GetFileName());
				nIndex = m_pTemplateList->AddRow(pRow);
				i++;
				if (finder.GetFileName().CompareNoCase(strDefTemplate) == 0) {
					m_nDefTemplateRowIndex = nIndex;
					pRow->ForeColor = RGB(255,0,0);
				}
			}
		}
		//do once more
		CString str = finder.GetFileName();
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		if ((str.Right(5).CompareNoCase(".dotx") == 0) || (str.Right(5).CompareNoCase(".dotm") == 0) || (str.Right(4).CompareNoCase(".dot") == 0)) {
			pRow = m_pTemplateList->GetRow(-1);
			pRow->Value[0] = _bstr_t(finder.GetFileName());
			nIndex = m_pTemplateList->AddRow(pRow);
			i++;
			if (finder.GetFileName().CompareNoCase(strDefTemplate) == 0) {
				m_nDefTemplateRowIndex = nIndex;
				pRow->ForeColor = RGB(255,0,0);
			}
		}

		// If nothing else was selected, default to the first element
		if (m_pTemplateList->CurSel == -1) {
			m_pTemplateList->CurSel = 0;
			// (c.haag 2009-09-15 13:04) - PLID 21400 - Default to the previously selected item if it exists
			if (!strPrevSel.IsEmpty()) {
				m_pTemplateList->FindByColumn(0, _bstr_t(strPrevSel), 0, VARIANT_TRUE);
			}
		}
	} else {
		// Nothing was placed into the list, which means we won't be able 
		// to run the superbill because there's no template to run it against
		MsgBox(MB_OK|MB_ICONINFORMATION, 
			"There are no templates in the shared templates Forms sub-folder:\n\n%s\n\n"
			"Without any prescription templates, the prescription cannot be printed.", strFind);
	}
}

// (j.fouts 2012-11-14 11:42) - PLID 53744 - Moved merge templates to the queue
void CPrescriptionQueueDlg::OnBnClickedBtnEditPrescriptionTemplate()
{
	try {
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		CString path, strInitPath;

		// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CString strFilter;
		// Always support Word 2007 templates
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		strInitPath = GetTemplatePath() + "Forms\\"; // We need to store this because the next line is a pointer to it
		dlg.m_ofn.lpstrInitialDir = strInitPath;
		dlg.m_ofn.lpstrTitle = "Select a template to edit";
		
		if (dlg.DoModal() == IDOK) {
			path = dlg.GetPathName();
		} else {
			// (c.haag 2009-09-15 12:51) - PLID 21400 - Requery the template list anyway because the user has the
			// freedom to rename templates from the file dialog.
			RequeryWordTemplateList();
			return;
		}

		CString strMergeInfoFilePath;

		try {
			// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
			pApp->EnsureValid();

			// Create an empty MergeInfo.nxt
			long nFlags = BMS_HIDE_ALL_DATA	| BMS_DEFAULT | /*BMS_HIDE_PRACTICE_INFO |*/
						/*BMS_HIDE_PERSON_INFO | BMS_HIDE_DATE_INFO | BMS_HIDE_PRESCRIPTION_INFO |*/
						/*BMS_HIDE_CUSTOM_INFO | BMS_HIDE_INSURANCE_INFO |*/ BMS_HIDE_BILL_INFO |
						BMS_HIDE_PROCEDURE_INFO /*| BMS_HIDE_DOCTOR_INFO*/;

			strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, NULL, NULL);
			if (!strMergeInfoFilePath.IsEmpty()) {

				// Open the template
				// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
				// (c.haag 2016-04-22 10:40) - NX-100275 - OpenTemplate no longer returns a document. We never did anything with it except throw an exception if it were null
				// anyway, and now OpenTemplate does that for us
				pApp->OpenTemplate(path, strMergeInfoFilePath);

				// We can't delete the merge info text file right now because it is in use, but 
				// it's a temp file so mark it to be deleted after the next reboot
				DeleteFileWhenPossible(strMergeInfoFilePath);
				strMergeInfoFilePath.Empty();
			} else {
				AfxThrowNxException("Could not create blank merge info");
			}
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		}NxCatchAll("CLetterWriting::OnEditTemplate");

		if (!strMergeInfoFilePath.IsEmpty()) {
			// This means the file wasn't used and/or it wasn't 
			// marked for deletion at startup, so delete it now
			DeleteFile(strMergeInfoFilePath);
		}

		// (c.haag 2009-09-15 12:51) - PLID 21400 - Now requery the template list
		RequeryWordTemplateList();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/20/2013) - PLID 58416 - Handler for when the user clicks the Med History button.
void CPrescriptionQueueDlg::OnBnClickedRxQueueMedHistoryBtn()
{
	try
	{
		// (r.gonet 08/19/2013) - PLID 58416 - Check for the license and permission.
		// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
		if(m_bHasSureScriptsLicense &&
			CheckCurrentUserPermissions(bioPatientMedicationHistory, sptRead))
		{
			CMedicationHistoryDlg dlg(this, m_nPatientID);
			// (r.gonet 09/20/2013) - PLID 58416 - Set the color to be the same as the rest of the backgrounds in the queue dialog.
			dlg.SetColor(m_medsAllergiesBkg.GetColor());
			dlg.DoModal();
			// (a.wilson 2013-08-30 16:55) - PLID 57844
			if (dlg.m_bImportedHistoryMeds == true) {
				RequeryAllergiesAndMeds(false, true);
				
				// (r.gonet 12/20/2013) - PLID 57844 - Show the interactions if there are any.
				long nCount = GetMainFrame()->ShowDrugInteractions(m_nPatientID);
				CMedicationDlg* pMedsDlg = dynamic_cast<CMedicationDlg*>(m_pParent);
				if(pMedsDlg)
				{
					pMedsDlg->SetInteractionCount(nCount);
				}

				SetInteractionCount(nCount);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-14 11:42) - PLID 53573 - Moved interactions to the queue
void CPrescriptionQueueDlg::OnBnClickedInteractionsButton()
{
	try
	{
		// (j.fouts 2013-05-30 09:26) - PLID 56807 - Drug Interactions is now tied to NexERx
		// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
		if(m_bHasSureScriptsLicense)
		{
			// (j.fouts 2012-09-25 09:27) - PLID 52825 - Check that the database exists
			if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
			{
				//Also check that the database exists
				if(!FirstDataBank::EnsureDatabase(this, true))
				{
					//We decided not to have an additional warning here about drug interactions not
					//being check because if FDB is not built yet then there should really be no way
					//that they have FDB drugs/allergies in their database yet.
					return;
				}
			}

			// (j.fouts 2012-11-15 10:36) - PLID 53573 - If we are embeded use the main frame, else use the queue's dlg
			if(m_bInMedicationsTab)
			{
				// (j.fouts 2012-09-05 15:28) - PLID 52482 - Made this modeless
				// (j.jones 2012-09-26 11:29) - PLID 52872 - Added patient ID, and changed this to always
				// force a requery, so that when the user clicks the button they are guaranteed up to date content.
				// (j.fouts 2013-02-28 14:24) - PLID 54429 - Update the count with the return value
				long nCount = GetMainFrame()->ShowDrugInteractions(m_nPatientID, true, true, true);
				CMedicationDlg* pMedsDlg = dynamic_cast<CMedicationDlg*>(GetParent());
				if(pMedsDlg)
				{
					pMedsDlg->SetInteractionCount(nCount);
				}

				SetInteractionCount(nCount);
			}
			else
			{

				if (!m_pDrugInteractionDlg) {
					// (b.savon 2012-11-30 10:30) - PLID 53773 - Pass in the parent enum
					m_pDrugInteractionDlg.reset(new CDrugInteractionDlg(this, eipQueue));
				}

				if (!IsWindow(m_pDrugInteractionDlg->GetSafeHwnd()))
				{
					m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
				}

				m_pDrugInteractionDlg->ShowOnInteraction(m_nPatientID, true, true, true);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-14 11:42) - PLID 53439 - We want to reposition the queue manualy
int CPrescriptionQueueDlg::SetControlPositions()
{
	int nVal = CNxDialog::SetControlPositions();

	try
	{
		UpdateShowState();
	}
	NxCatchAll(__FUNCTION__);

	return nVal;
}

// (j.jones 2012-11-16 16:03) - PLID 53085 - added EMN/patient filter
void CPrescriptionQueueDlg::OnRadioEmnMeds()
{
	try {

		//remember their selection
		//0 - EMN, 1 - Patient
		if(m_radioFilterOnEMNMeds.GetCheck()) {
			SetRemotePropertyInt("PrescriptionQueueDefaultEMNFilter", 0, 0, GetCurrentUserName());
		}

		//This filter only applies to listed medications, not interactions.
		//Interactions should have already been displayed once when the queue
		//was opened, so we don't need to show them again.
		RequeryQueue(false);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-11-16 16:03) - PLID 53085 - added EMN/patient filter
void CPrescriptionQueueDlg::OnRadioPatientMeds()
{
	try {

		//remember their selection
		//0 - EMN, 1 - Patient
		if(m_radioFilterOnPatientMeds.GetCheck()) {
			SetRemotePropertyInt("PrescriptionQueueDefaultEMNFilter", 1, 0, GetCurrentUserName());
		}

		//This filter only applies to listed medications, not interactions.
		//Interactions should have already been displayed once when the queue
		//was opened, so we don't need to show them again.
		RequeryQueue(false);

	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-11-19 16:00) - PLID 53831 - Send all prescriptions
void CPrescriptionQueueDlg::OnBnClickedEsubmitAll()
{
	try{
		// (j.fouts 2012-12-26 11:48) - PLID 53840 - Check their license before sending
		if(SureScripts::IsEnabled())
		{
			//Go through all the prescritpions in the array adding the ones that are checked for sending.
			CArray<CString, CString&> aryChangeStatusPrescriptionIDs;

			// (j.fouts 2013-09-17 16:53) - PLID 58496 - Keep a seperate list for each patinet, some may be removed
			typedef std::pair<long,CString> IDNamePair;
			typedef boost::unordered_map<IDNamePair,CArray<long,long>> PatientMap;

			PatientMap mapSelectedPatients;

			CArray<long, long> aryPrescriptionIDs;

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();

			CString strMsg = "The following prescriptions have been printed:\n\r";
			long nPrintedCount = 0;

			while(pRow)
			{
				if( VarBool(pRow->GetValue(pqcSend),g_cvarFalse) == TRUE ){
					NexTech_Accessor::PrescriptionStatus queueStaus = (NexTech_Accessor::PrescriptionStatus)VarLong(pRow->GetValue(pqcStatusID), 0 /*No Status*/);
					if(queueStaus == NexTech_Accessor::PrescriptionStatus_Printed)
					{
						strMsg += VarString(pRow->GetValue(pqcMedName), "") + "\n\r";
						++nPrintedCount;
					}


					//Create a key of the ID and Name
					long nID = GetExistingPatientIDByUserDefinedID(VarLong(pRow->GetValue(pqcPatientUserDefiniedID)));
					IDNamePair key(nID, VarString(pRow->GetValue(pqcPatient)));
	
					//The value is the prescription ID list
					mapSelectedPatients[key].Add(VarLong(pRow->GetValue(pqcPrescriptionID)));
				}

				pRow = pRow->GetNextRow();
			}

			if(nPrintedCount > 0)
			{
				if(MessageBox(FormatString("Sending prescriptions electronically that have been printed is not recommended. "
					"%li selected prescription%s a printed status. Please be sure not to give duplicate copies to a patient.\n\r"
					"Are you sure you wish to continue electronically sending this prescription?", nPrintedCount, nPrintedCount==1?" has":"s have")
					, "Continue Sending?", MB_ICONQUESTION|MB_YESNO) == IDNO)
				{
					return;
				}
			}

			// (j.fouts 2013-04-23 10:53) - PLID 52907 - Warn when sending
			if(DontShowMeAgain(this, "If you continue electronically submitting you will no longer be able "
				"to edit, delete, eFax, or electronically submit the submitted prescriptions.\n\r"
				"Are you sure you wish to continue electronically submitting?"
				, "NexERx_SendLockWarning", "Warning", FALSE, TRUE) == IDNO)
			{
				return;
			}
			
			for(PatientMap::iterator i = mapSelectedPatients.begin();i!=mapSelectedPatients.end();i++)
			{
			// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
				//Iterator's first is the key, which has ID then Name
				if(PromptInteractions(i->first.first, i->first.second))
			{
					//Iterator's second is the value, our ID list
					aryPrescriptionIDs.Append(i->second);
				}
			}
			
			//Bail if we just elimiated all prescriptions
			if(aryPrescriptionIDs.GetSize() == 0)
			{
				return;
			}

			// (j.jones 2016-02-22 14:05) - PLID 68354 - show all prescriptions we're about to send,
			// do this prior to the error checking below such that they review what they selected
			CErxPrescriptionReviewDlg dlgReview(this, GetNxColor(GNC_PATIENT_STATUS, 1));
			for (int idx = 0; idx < aryPrescriptionIDs.GetCount(); idx++) {
				LPARAM id = aryPrescriptionIDs.GetAt(idx);
				dlgReview.m_aryPrescriptionIDs.push_back((long)id);
			}
			if (IDCANCEL == dlgReview.DoModal()) {
				return;
			}

			// (b.savon 2013-01-28 10:08) - PLID 51705
			//Ensure each prescription has its sender based on the current logged in user.
			CArray<long, long> aryErrorRxs;
			for(int idx = 0; idx < aryPrescriptionIDs.GetCount(); idx++ ){
				// (j.jones 2012-11-06 10:02) - PLID 52819 - pass in our EMNID and template status
				CPrescriptionEditDlg dlg(GetParent(), m_nCurrentlyOpenedEMNID);
				// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription
				// (j.fouts 2013-03-12 10:18) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
				// (b.savon 2014-08-19 16:13) - PLID 63403 - Don't show interactions
				// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
				PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(false,
					LoadFullPrescription(aryPrescriptionIDs.GetAt(idx)), NULL, FALSE, true);
				if( epdrvReturn == epdrvErrorRx ){
					aryErrorRxs.Add(aryPrescriptionIDs.GetAt(idx));
				}
				else if (epdrvReturn == epdrvIDCANCEL) {
					// (b.savon 2016-04-19 7:30) - PLID-68647
					// ignore, don't do anything.  They effectively cancelled opening the prescription because they didn't want to
					// take ownership.  So, don't send it by not adding it to the aryChangeStatusPrescriptionIDs array.
				}
				else {
					CString strID;
					strID.Format("%li", aryPrescriptionIDs.GetAt(idx));
					aryChangeStatusPrescriptionIDs.Add(strID);
				}
			}

			// (j.fouts 2012-12-27 16:50) - PLID 53160 - Call the API to authorize e-transmitting
			if( aryChangeStatusPrescriptionIDs.GetSize() > 0) {

				SetPrescriptionStatus(aryChangeStatusPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized);
			}

			if( aryErrorRxs.GetSize() > 0 ){
				MessageBox("There is at least one prescription that needs to be corrected before it may be sent.\r\n\r\n"
						   "Please correct the issue and send it again.", "NexTech Practice", MB_ICONINFORMATION);
			}

			UpdateSendButton();
		}
		else if(g_pLicense->CheckForLicense(CLicense::lcNewCrop, CLicense::cflrUse)) 
		{
			// (j.fouts 2012-12-26 11:48) - PLID 53840 - If they have the newcrop license then open NewCrop
			// (j.gruber 2009-03-31 11:40) - PLID 33328 - check to see if they have permission
			// (j.gruber 2009-06-08 10:39) - PLID 34515 - get back the role
			NewCropUserTypes ncuTypeID;
			if (!CheckEPrescribingStatus(ncuTypeID, GetCurrentUserID(), GetCurrentUserName())) {
				//this pops up the message for us
				return;
			}

			CString strWindowDescription;
			strWindowDescription.Format("Accessing Electronic Prescription Account for Patient: %s", GetExistingPatientName(m_nPatientID));
			
			// (j.gruber 2009-03-30 09:36) - PLID 33728 - we'll need to requery the list since we are syncing medications back into Practice now
			long nUserDefinedID = GetExistingPatientUserDefinedID(m_nPatientID);
			if(m_bInMedicationsTab)
			{
				GetMainFrame()->OpenNewCropBrowser(strWindowDescription, ncatAccessPatientAccount, m_nPatientID, m_nPatientID, nUserDefinedID, m_nCurrentlyOpenedEMNID, GetParent(), "");
			}
			else
			{
				GetMainFrame()->OpenNewCropBrowser(strWindowDescription, ncatAccessPatientAccount, m_nPatientID, m_nPatientID, nUserDefinedID, m_nCurrentlyOpenedEMNID, this, "");
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-20 14:23) - PLID 53840 - Disable/Hide all controls that require SureScripts
// (j.fouts 2012-12-26 11:49) - PLID 54340 - We may still want to show the send button since we will use it to open NewCrop
void CPrescriptionQueueDlg::HideSureScriptsControls(bool bHideSendButton /*=true*/)
{
	// (j.fouts 2013-05-30 09:28) - PLID 56807 - Drug Interactions should be tied to the NexERx License rather than FDB.
	m_bShowInteractions = false;
	m_btnShowInteractions.EnableWindow(FALSE);
	m_btnShowInteractions.ShowWindow(SW_HIDE);

	// (r.gonet 08/19/2013) - PLID 58416 - Hide the medication history button.
	m_bShowMedHistory = false;
	m_btnMedicationHistory.EnableWindow(FALSE);
	m_btnMedicationHistory.ShowWindow(SW_HIDE);

	// (b.savon 2014-01-03 07:44) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately 
	// (in medications tab as well as emr) when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
	m_bShowNexFormulary = false;
	m_btnNexFormulary.EnableWindow(FALSE);
	m_btnNexFormulary.ShowWindow(SW_HIDE);
	//(s.dhole 3/10/2015 11:26 AM ) - PLID 64561 hide formulary column from search datalist
	HideNexFormularyColumn(m_nxdlWritePrescriptionResults, mrcNexFormulary);

	m_bShowRenewals = false;
	m_bShowSend = false;

	//Disable the controls
	m_pRenewalList->PutEnabled(VARIANT_FALSE);
	GetDlgItem(IDC_UPDATE_RENEWALS)->EnableWindow(FALSE);
	m_nxstaticRenewalText.EnableWindow(FALSE);
	m_renewalBkg.EnableWindow(FALSE);
	m_btnExpandRenewals.EnableWindow(FALSE);
	m_pRenewalFilterPrescriberList->PutEnabled(VARIANT_FALSE);
	m_pRenewalFilterResponseList->PutEnabled(VARIANT_FALSE);
	m_pRenewalFilterTransmitList->PutEnabled(VARIANT_FALSE);
	m_dtpRenewalFilterFrom.EnableWindow(FALSE);
	m_dtpRenewalFilterTo.EnableWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_TEXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_PROVIDER_TEXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_RESPONSE_TEXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_TRANSMIT_TEXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_FROM_TEXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_TO_TEXT)->EnableWindow(FALSE);
	m_btnRxNeedingAttention.EnableWindow(FALSE);
	// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed the Default Filtered Prescriber checkbox.

	//Hide the controls
	m_nxstaticRenewalText.ShowWindow(SW_HIDE);
	m_renewalBkg.ShowWindow(SW_HIDE);
	m_btnExpandRenewals.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_RENEWAL_REQUEST_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_UPDATE_RENEWALS)->ShowWindow(SW_HIDE);
	m_dtpRenewalFilterFrom.ShowWindow(SW_HIDE);
	m_dtpRenewalFilterTo.ShowWindow(FALSE);
	GetDlgItem(IDC_FILTER_RENEWAL_PROVIDER)->ShowWindow(SW_HIDE);
	// (r.gonet 2016-01-22) - PLID 67973 - Hide the multi-prescriber label.
	GetDlgItem(IDC_MULTI_RENEWAL_PRESCRIBERS_LABEL)->ShowWindow(SW_HIDE);
	// (r.gonet 2016-01-22) - PLID 67973 - Tell the prescriber controller to not show
	// the prescriber combo or label during its ensuring of controls.
	m_renewalPrescriberController.SetVisible(false);
	GetDlgItem(IDC_FILTER_RENEWAL_RESPONSE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_TRANSMIT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_TEXT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_PROVIDER_TEXT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_RESPONSE_TEXT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_TRANSMIT_TEXT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_FROM_TEXT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_FILTER_RENEWAL_TO_TEXT)->ShowWindow(SW_HIDE);
	m_btnRxNeedingAttention.ShowWindow(SW_HIDE);
	// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed the Default Filtered Prescriber checkbox.

	using namespace NXDATALIST2Lib;
	/*** Disable the "Send" column in the datalist ***/
	IColumnSettingsPtr pCol = m_pPresQueueList->GetColumn(pqcSend);

	//Disable
	pCol->PutEditable(VARIANT_FALSE);

	//Hide the column, by making it 0px wide
	pCol->PutStoredWidth(0);
	//Remove rezising the column
	long nStyle = pCol->GetColumnStyle();
	if(nStyle & csVisible) {
		nStyle = nStyle & ~csVisible;
		pCol->PutColumnStyle(nStyle);
	}
	// (s.dhole 2012-12-24 12:18) - PLID 53334 check error 
	
	pCol = m_pPresQueueList->GetColumn(pqcMedErrorInd);
	//Disable
	pCol->PutEditable(VARIANT_FALSE);

	//Hide the column, by making it 0px wide
	pCol->PutStoredWidth(0);
	//Remove rezising the column
	nStyle = pCol->GetColumnStyle();
	if(nStyle & csVisible) {
		nStyle = nStyle & ~csVisible;
		pCol->PutColumnStyle(nStyle);
	}

	/*** Disable the "Send All/Selected" button ***/
	if(bHideSendButton)
	{
		m_btnESubmit.EnableWindow(FALSE);
		m_btnESubmit.ShowWindow(SW_HIDE);
		BOOL bHasNoERxLicense = (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNone);
		m_btnPreview.EnableWindow(bHasNoERxLicense ? TRUE : FALSE);
		m_btnPreview.ShowWindow(bHasNoERxLicense ? SW_SHOW : SW_HIDE);
		m_checkSendToPrinter.EnableWindow(bHasNoERxLicense ? TRUE : FALSE);
		m_checkSendToPrinter.ShowWindow(bHasNoERxLicense ? SW_SHOW : SW_HIDE);
	}

	//Hide the e-Transmitted column in the filter datalist
	CString strWhere = (LPTSTR)m_pQueueStatusFilter->GetWhereClause();
	if(strWhere.Find("InternalID NOT IN (8,9,10,11)") < 0)
	{
		if(strWhere.IsEmpty())
		{
			strWhere = "InternalID NOT IN (8,9,10,11)";
		}
		else
		{
			strWhere += " AND InternalID NOT IN (8,9,10,11)";
		}

		IRowSettingsPtr pRow = m_pQueueStatusFilter->GetCurSel();
		m_pQueueStatusFilter->PutWhereClause(_bstr_t(strWhere));
		m_pQueueStatusFilter->Requery();
		m_pQueueStatusFilter->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		/*IRowSettingsPtr pNewRow = m_pQueueStatusFilter->GetNewRow();
		pNewRow->PutValue(qsfcID, -1);
		pNewRow->PutValue(qsfcStatus, " < Show All >");
		m_pQueueStatusFilter->AddRowBefore(pNewRow, m_pQueueStatusFilter->GetFirstRow());*/
		// (b.eyers 2016-01-25) - PLID 67983 - Adding a row for filtering statuses that require action
		// at the moment these are: incomplete, on hold, e-transmit authorized, e-transmit error, ready for review
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pQueueStatusFilter->GetNewRow();
		pNewRow->PutValue(qsfcID, -2);
		pNewRow->PutValue(qsfcStatus, " < Prescriptions Requiring Action >");
		m_pQueueStatusFilter->AddRowBefore(pNewRow, m_pQueueStatusFilter->GetFirstRow());

		if (m_nPatientID > 0)
		{
			//Add a <Show All> row to Queue Status
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pQueueStatusFilter->GetNewRow();
			pNewRow->PutValue(qsfcID, pqsInvalid);
			pNewRow->PutValue(qsfcStatus, " < Show All Statuses >");
			m_pQueueStatusFilter->PutCurSel(m_pQueueStatusFilter->AddRowBefore(pNewRow, m_pQueueStatusFilter->GetFirstRow()));
		}
		else
		{
			// (b.eyers 2016-01-25) - PLID 67983 - for the prescription needing attention dialog, the 'requires action' statues are default
			//m_pQueueStatusFilter->FindByColumn(qsfcID, pqsIncomplete, NULL, VARIANT_TRUE);
			m_pQueueStatusFilter->FindByColumn(qsfcID, -2, NULL, VARIANT_TRUE);
		}

		/*long nSelection = pRow->GetValue(qsfcID);
		if(!m_pQueueStatusFilter->FindByColumn(qsfcID, nSelection, NULL, VARIANT_TRUE))
		{
			m_pQueueStatusFilter->PutCurSel(m_pQueueStatusFilter->GetFirstRow());
		}*/
	}
}

// (j.fouts 2012-11-20 14:23) - PLID 53840 - Disables/Hides all controls that use FDB including SureScripts controls
//void CPrescriptionQueueDlg::HideFirstDataBankControls()
//{
//	//Without FirstDataBank we cannot use SureScripts
//	// (b.savon 2013-05-13 17:19) - PLID 56671 - Check if they have NewCrop.  If they do, update the button text
//	bool bHasNewCrop = g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop;
//	if( bHasNewCrop ){
//		// (b.savon 2013-05-21 08:59) - PLID 56795
//		ShowNewCropPrescriptionButton();	
//	}
//	//Override the flag so that the NewCrop and Preview buttons are displayed.
//	HideSureScriptsControls(!bHasNewCrop);
//}

// (j.fouts 2012-11-20 14:23) - PLID 53840 - Hide controls and set hide flags for controls that they don't have a license for
void CPrescriptionQueueDlg::UpdateControlsForLicense()
{

	/*** Check for the FirstDataBank License ***/
	// (j.fouts 2013-05-30 09:35) - PLID 56807 - I am removing this for now, we moved drug interactions
	// under the license for NexERx(SureScripts). So now there is nothing on this dlg that is tied directly
	// to FirstDataBank. Leaving it here though incase something gets added that depends on the FDB license.
	//if(!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent))
	//{
	//	//Without FirstDataBank we should not show interactions
	//	HideFirstDataBankControls();
	//}

	/*** Check for the EPrescribing License ***/
	CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
	switch(ept)
	{
	case CLicense::eptSureScripts:
		// (j.fouts 2013-01-28 14:46) - PLID 53840 - We do not want the RXCUI column with NexERx
		m_pAllergiesList->GetColumn(alcRXCUI)->PutStoredWidth(0);
		m_pAllergiesList->GetColumn(alcRXCUI)->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csFixedWidth);
		break;

	case CLicense::eptNewCrop:
		// (b.savon 2013-05-21 08:59) - PLID 56795
		ShowNewCropPrescriptionButton();
		HideSureScriptsControls(false);
		break;

	default:
		//Someone must have added a new EPrescribing license type. This will need a case for the new type
		ASSERT(FALSE);
		//No break, we are going to fall through and treat this as though they have no ePrescribing

	case CLicense::eptNone:
		// (j.fouts 2012-12-26 11:49) - PLID 54340 - Hide RXCUI if they don't have NewCrop
		m_pAllergiesList->GetColumn(alcRXCUI)->PutStoredWidth(0);
		m_pAllergiesList->GetColumn(alcRXCUI)->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csFixedWidth);
		HideSureScriptsControls();
		break;

	}

	/*** Check for the Faxing License ***/
	// (j.fouts 2013-03-01 10:12) - PLID 55800 - If they do not have Faxing we should hide the fax status
	if(!g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent))
	{
		CString strWhere = (LPTSTR)m_pQueueStatusFilter->GetWhereClause();
		if(strWhere.Find("InternalID <> 13") < 0)
		{
			if(strWhere.IsEmpty())
			{
				strWhere = "InternalID <> 13";
			}
			else
			{
				strWhere += " AND InternalID <> 13";
			}

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pQueueStatusFilter->GetCurSel();
			m_pQueueStatusFilter->PutWhereClause(_bstr_t(strWhere));
			m_pQueueStatusFilter->Requery();
			m_pQueueStatusFilter->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			/*NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pQueueStatusFilter->GetNewRow();
			pNewRow->PutValue(qsfcID, -1);
			pNewRow->PutValue(qsfcStatus, "<Show All>");
			m_pQueueStatusFilter->AddRowBefore(pNewRow, m_pQueueStatusFilter->GetFirstRow());*/
			// (b.eyers 2016-01-25) - PLID 67983 - Adding a row for filtering statuses that require action
			// at the moment these are: incomplete, on hold, e-transmit authorized, e-transmit error, ready for review
			//NXDATALIST2Lib::IRowSettingsPtr pRow = m_pQueueStatusFilter->GetNewRow();
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pQueueStatusFilter->GetNewRow();
			pNewRow->PutValue(qsfcID, -2);
			pNewRow->PutValue(qsfcStatus, " < Prescriptions Requiring Action >");
			m_pQueueStatusFilter->AddRowBefore(pNewRow, m_pQueueStatusFilter->GetFirstRow());

			if (m_nPatientID > 0)
			{
				//Add a <Show All> row to Queue Status
				NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pQueueStatusFilter->GetNewRow();
				pNewRow->PutValue(qsfcID, pqsInvalid);
				pNewRow->PutValue(qsfcStatus, " < Show All Statuses >");
				m_pQueueStatusFilter->PutCurSel(m_pQueueStatusFilter->AddRowBefore(pNewRow, m_pQueueStatusFilter->GetFirstRow()));
			}
			else
			{
				// (b.eyers 2016-01-25) - PLID 67983 - for the prescription needing attention dialog, the 'requires action' statues are default
				//m_pQueueStatusFilter->FindByColumn(qsfcID, pqsIncomplete, NULL, VARIANT_TRUE);
				m_pQueueStatusFilter->FindByColumn(qsfcID, -2, NULL, VARIANT_TRUE);
			}

			/*long nSelection = pRow->GetValue(qsfcID);
			if(!m_pQueueStatusFilter->FindByColumn(qsfcID, nSelection, NULL, VARIANT_TRUE))
			{
				m_pQueueStatusFilter->PutCurSel(m_pQueueStatusFilter->GetFirstRow());
			}*/
		}
	}
}

// (j.fouts 2012-11-20 14:23) - PLID 53840 - Check permissions and disable controls
void CPrescriptionQueueDlg::SecureControls()
{
	//  We will need to secure the send prescriptions button, as well as the Send collumn in the datalist. 
	//	The Renewals CollapseableBox should be hidden/disabled. Remove e-Prescribing statuses from the filter.
	//TODO: Questions to still have answered:
	//	- Should we also disable Interaction checks?
	//	- Should we disable validation (or use a simplified version)?

	/*** Permissions ***/
	//Check if they have permissions to create/delete prescriptions
	BOOL bCreate = GetCurrentUserPermissions(bioPatientMedication) & SPT____C_______ANDPASS;
	BOOL bDelete = GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS;

	// (r.gonet 09/19/2013) - PLID 58416 - Get the permission to be able to read medication history.
	BOOL bMedicationHistoryRead = GetCurrentUserPermissions(bioPatientMedicationHistory) & SPT__R_________ANDPASS;
	
	// (b.savon 2013-01-21 12:23) - PLID 54720 - Hide the print/send columns
	if( !m_bCachedIsLicensedConfiguredUser ){
		m_pPresQueueList->GetColumn(pqcSend)->PutColumnStyle(NXDATALIST2Lib::csFixedWidth|NXDATALIST2Lib::csVisible);	
		m_pPresQueueList->GetColumn(pqcSend)->PutStoredWidth(0);
	}

	// (r.gonet 09/19/2013) - PLID 58416 - Disable the med history button if we can't read med history.
	GetDlgItem(IDC_RX_QUEUE_MED_HISTORY_BTN)->EnableWindow(bMedicationHistoryRead);

	// (r.gonet 2016-01-22) - PLID 67967 - Allow the prescriber controller to show the prescriber combo or label
	// only if renewals are being shown currently.
	m_rxPrescriberController.SetVisible(!m_bShowRenewals || m_renewalsBox.IsCollapsed());
	// (r.gonet 2016-01-22) - PLID 67973 - Allow the prescriber controller to show the prescriber combo or label
	// only if renewals are being shown currently.
	m_renewalPrescriberController.SetVisible(m_bShowRenewals && !m_renewalsBox.IsCollapsed());

	// (j.jones 2016-02-02 17:19) - PLID 67993 - if they do not have FDB, don't show the free text prescription search
	// option, because all searches are free text only without FDBs
	// (j.jones 2016-02-02 17:19) - PLID 67996 - same for current meds
	// (j.jones 2016-02-02 17:19) - PLID 67997 - and same for allergies
	if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
		m_checkIncludeFreeTextRx.ShowWindow(SW_HIDE);
		m_checkIncludeFreeTextCurMeds.ShowWindow(SW_HIDE);
		m_checkIncludeFreeTextAllergies.ShowWindow(SW_HIDE);
	}
}

// (j.fouts 2012-11-30 10:07) - PLID 53954 - Added a right click menu
void CPrescriptionQueueDlg::RButtonUpPrescriptionQueueList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		// (j.fouts 2012-11-30 10:07) - PLID 53954 - Coppied/Modified from the old Prescription History datalist

		//TS:  I'm taking all this out on Kamal's advice; but I am intentionally leaving the functions to
		//actually delete Prescriptions intact, so if it ever turns out someone desperately needs this 
		//functionality, all we'll have to do is uncomment this code right here.  Basically what I'm saying
		//is, removing functionality makes me nervous.

		//TS:  TODO: My foresight has been rewarded, and this code is back in.  However, we need to come 
		//up with some sort of real resolution to this, instead of taking this code in and out as the whim takes
		//us, confusing our clients and ourselves, and in the end leaving our code, and our psyches, as shattered, 
		//spaghetti-like clumps of random commands with no logic or guiding purpose.

		if(!lpRow)
		{
			return;
		}

		//Get the cursor position first, so it will only appear where they clicked
		CPoint pt;
		pt.x = x;
		pt.y = y;
		GetCursorPos(&pt);

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		// (b.savon 2013-09-04 17:13) - PLID 58212 - Add a new 'Void' type for Prescriptions
		NexTech_Accessor::PrescriptionStatus queueStatus = NexTech_Accessor::PrescriptionStatus_NoStatus;

		//Select the row
		m_pPresQueueList->PutCurSel(pRow);

		bool bSendAllChecked = true;	//Are all the send boxes checked
		{
			for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();pRow;pRow = pRow->GetNextRow())
			{
				// (j.fouts 2012-12-26 11:48) - PLID 53840 - Support null values, since they may not have the license
				_variant_t varSend = pRow->GetValue(pqcSend);
				if(varSend.vt == VT_BOOL)
				{
					if(!VarBool(varSend))
					{
						//Not checked
						bSendAllChecked = false;
						break;
					}
				}
	// (s.dhole 2012-12-24 11:21) - PLID  53334 check null
				if(!VarBool(pRow->GetValue(pqcSend),g_cvarFalse ))
				{
					//Not checked
					bSendAllChecked = false;
					break;
				}
			}
		}

		bool bPrintAllChecked = true;	//Are all the print boxes checked
		{
			for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();pRow;pRow = pRow->GetNextRow())
			{
				// (j.fouts 2012-12-26 11:48) - PLID 53840 - Support null values, since they may not have the license
				_variant_t varPrint = pRow->GetValue(pqcPrint);
				if(varPrint.vt == VT_BOOL)
				{
					if(!VarBool(varPrint))
					{
						//Not checked
						bPrintAllChecked = false;
						break;
					}
				}
			}
		}

		// Build a menu popup with the ability to delete the current row
		CMenu  menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		if(nCol == 	pqcSend && m_nPatientID != -1)
		{
			menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_CHECK_SEND, bSendAllChecked? "Deselect All" : "Select All");
		}
		else if(nCol == pqcPrint && m_nPatientID != -1)
		{
			menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_CHECK_PRINT, bPrintAllChecked? "Deselect All" : "Select All");
		}
		else
		{
			// (a.wilson 2013-04-11 10:07) - PLID 55014 - prevent deleting denynewrxs
			queueStatus = (NexTech_Accessor::PrescriptionStatus)VarLong(pRow->GetValue(pqcStatusID), 0 /*No Status*/);
			bool bFromNewCrop = !AsString(pRow->GetValue(pqcNewCropGUID)).IsEmpty();
			bool bFromDeny = (AsLong(pRow->GetValue(pqcDenyNewRxResponseID)) > 0);	
			
			// (j.fouts 2013-06-13 16:07) - PLID 57161 - Allow deleting NewCrop Prescriptions
			// (b.savon 2013-09-04 15:27) - PLID 58212 - Add ReadyForDoctorReview
			if(GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS && !bFromDeny
				&& (queueStatus == NexTech_Accessor::PrescriptionStatus_Incomplete 
				|| queueStatus == NexTech_Accessor::PrescriptionStatus_eTransmitError 
				|| queueStatus == NexTech_Accessor::PrescriptionStatus_Legacy 
				|| queueStatus == NexTech_Accessor::PrescriptionStatus_OnHold
				|| queueStatus == NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview))
			{
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_DELETE_PRESCRIPTION, "Delete");
			}
			else
			{
				menPopup.InsertMenu(-1, MF_BYPOSITION|MF_GRAYED, IDM_DELETE_PRESCRIPTION, "Delete");
			}

			// (j.fouts 2013-04-01 12:53) - PLID 53840
			// (j.fouts 2013-01-28 16:34) - PLID 53025 - Add On Hold status
			// (b.savon 2013-09-04 15:27) - PLID 58212 - Add ReadyForDoctorReview
			if(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS &&
				(queueStatus == NexTech_Accessor::PrescriptionStatus_Incomplete ||
				queueStatus == NexTech_Accessor::PrescriptionStatus_eTransmitError ||
				queueStatus == NexTech_Accessor::PrescriptionStatus_Legacy ||
				queueStatus == NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview) && !bFromNewCrop)
			{
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_HOLD_PRESCRIPTION, "Put on hold");
			}
			else if(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS &&
				queueStatus == NexTech_Accessor::PrescriptionStatus_OnHold && !bFromNewCrop)
			{
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_HOLD_PRESCRIPTION, "Remove hold");
			}
			else
			{
				menPopup.InsertMenu(-1, MF_BYPOSITION|MF_GRAYED, IDM_HOLD_PRESCRIPTION, "Put on hold");
			}
			

			// (j.fouts 2013-01-15 10:02) - PLID 55800 - Allow people with legacy status to clean up their data if they want
			if(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS 
				&& queueStatus == NexTech_Accessor::PrescriptionStatus_Legacy 
				&& queueStatus != NexTech_Accessor::PrescriptionStatus_Void)
			{
				if(!bFromNewCrop)
				{
					menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_MARK_INCOMPLETE, "Mark Incomplete");
					menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_MARK_PRINTED, "Mark Printed");
				}
				else
				{
					menPopup.InsertMenu(-1, MF_BYPOSITION|MF_GRAYED, IDM_MARK_INCOMPLETE, "Mark Incomplete");
					menPopup.InsertMenu(-1, MF_BYPOSITION|MF_GRAYED, IDM_MARK_PRINTED, "Mark Printed");
				}
			}

			// (b.savon 2013-09-04 11:13) - PLID 58212 - Add a new 'Void' type for Prescriptions
			// (b.eyers 2016-02-03) - PLID 67982 - changed client facing name  
			// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
			if(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS && !bFromNewCrop && !bFromDeny ){
				if( queueStatus != NexTech_Accessor::PrescriptionStatus_Void ){
					menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_VOID_PRESCRIPTION, "Void Prescription");
					if ( queueStatus != NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview &&
						 queueStatus != NexTech_Accessor::PrescriptionStatus_Printed && 
						 queueStatus != NexTech_Accessor::PrescriptionStatus_eTransmitSuccess &&
						 queueStatus != NexTech_Accessor::PrescriptionStatus_eTransmitPending &&
						 queueStatus != NexTech_Accessor::PrescriptionStatus_DispensedInHouse){
						menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_DOCTOR_REVIEW_PRESCRIPTION, "Ready for Review");
					}
				}
			}

			// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
			if (GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS && !bFromNewCrop &&
				(queueStatus != NexTech_Accessor::PrescriptionStatus_Void && 
					queueStatus != NexTech_Accessor::PrescriptionStatus_Printed &&
					queueStatus != NexTech_Accessor::PrescriptionStatus_eTransmitSuccess &&
					queueStatus != NexTech_Accessor::PrescriptionStatus_eTransmitPending && 
					queueStatus != NexTech_Accessor::PrescriptionStatus_DispensedInHouse)) {
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_PRESCRIPTION_DISPENSED_IN_HOUSE, "Mark Dispensed In-House");
			}

			// (b.savon 2013-06-19 16:53) - PLID 56880 - Only add the menu options if they have NexERx
			// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
			if(m_bHasSureScriptsLicense) {
				long nFDBMedID = VarLong(pRow->GetValue(pqcFDBID), -1);

				menPopup.InsertMenu(-1, MF_BYPOSITION, MF_SEPARATOR);

				if(nFDBMedID >= 0)
				{
					menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_SHOW_MONO, "View Monograph");	
					menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_SHOW_LEAFLET, "View Leaflet");
				}
				else
				{
					menPopup.InsertMenu(-1, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_MONO, "No Monograph Available");
					menPopup.InsertMenu(-1, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_LEAFLET, "No Leaflet Available");
				}
			}

			// (r.farnworth 2016-01-08 15:37) - PLID 58692 - Wants to be able to quickly re-prescribe a prescription. Perhaps a right click and "Re-send" or "Re-Prescribe" (This functionality was in NewCrop)
			if (GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS &&
				queueStatus != NexTech_Accessor::PrescriptionStatus_Void && !bFromNewCrop)
			{
				menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_REPRESCRIPE, "Re-prescribe");
			}
		}

		long nCmdID = menPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		switch(nCmdID)
		{
		case IDM_DELETE_PRESCRIPTION:
			{
				if (!CheckCurrentUserPermissions(bioPatientMedication, sptDelete))
					return;

				// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
				if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
				{
					//Also check that the database exists
					if(!FirstDataBank::EnsureDatabase(this, true)) 
					{
						//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
						return;
					}
				}

				//m.hancock - 4/9/2006 - PLID 20020 - You should not be able to delete medications if they exist for EMN's
				//Check if this medication is associated with an EMN
				// (j.jones 2013-02-01 15:44) - PLID 53532 - if this is an EMR prescription, disallow if the EMN
				// is locked, or if the queue wasn't opened from that EMN
				long nSelVal = VarLong(pRow->GetValue(pqcPrescriptionID));
				_RecordsetPtr rs = CreateParamRecordset("SELECT EMRID FROM EMRMedicationsT WHERE Deleted = 0 AND MedicationID = {INT} ", nSelVal);
				if(!rs->eof) {
					long nEMNID = VarLong(rs->Fields->Item["EMRID"]->Value);
					if(m_nCurrentlyOpenedEMNID == -1 || m_nCurrentlyOpenedEMNID != nEMNID) {
						//we did not open the queue from the same EMN that this prescription linked to, so they can't delete it
						AfxMessageBox("This medication cannot be deleted because it exists on an EMN for this patient.");
						return;
					}
					else {
						//we are viewing the correct EMN, so they should be able to delete it, provided they currently have writeable access
						long nStatus = -1;
						BOOL bHasAccess = FALSE;
						if(!CanEditEMN(nEMNID, nStatus, bHasAccess)) {
							//if the EMN is locked, say so
							if(nStatus == 2) {
								AfxMessageBox("This medication cannot be deleted from this EMN because the EMN is locked.");
							}
							//otherwise warn if they do not have access to the EMN
							else if(!bHasAccess) {
								AfxMessageBox("This medication cannot be deleted from this EMN because you do not currently have writeable access to the EMN.");
							}
							else {
								//should be impossible
								ThrowNxException("CanEditEMN returned FALSE for unknown reasons.");
							}
							return;
						}
						//if we get here, then the user does currently have writeable access to the EMN,
						//so deletion is allowed
					}
				}

				// (a.walling 2009-04-23 13:12) - PLID 34046 - Warn if e-presciption
				CString strMsg;

				// (c.haag 2009-07-02 11:57) - PLID 34102 - Don't discriminate on the message type being mtPendingRx. Just don't allow the deletion
				// if the prescription exists in the SureScripts table.
				// (j.jones 2012-10-29 14:45) - PLID 53259 - also cannot delete if the prescription status is E-Prescribed
				// (b.eyers 2016-02-05) - PLID 67980 - added dispensed in house
				if(ReturnsRecordsParam("SELECT TOP 1 SureScriptsMessagesT.ID "
					"FROM SureScriptsMessagesT "
					"WHERE PatientMedicationID = {INT} "
					"UNION SELECT TOP 1 PatientMedications.ID "
					"FROM PatientMedications "
					"WHERE PatientMedications.ID = {INT} AND PatientMedications.QueueStatus IN ({SQL})",
					nSelVal, nSelVal, GetERxStatusFilter())) {

					// (a.walling 2009-04-24 13:53) - PLID 34046 - Prevent deleting entirely
					// (b.savon 2013-09-23 07:31) - PLID 58486 - Changed the wording
					MessageBox("This prescription cannot be deleted because it has been printed, voided, electronically prescribed, or dispensed in-house.", NULL, MB_ICONSTOP);

					break;
				}

				strMsg += "Are you sure you want to remove this prescription?";

				if (IDYES == MsgBox(MB_YESNO, strMsg)) {

					CWaitCursor pWait;

					// (j.jones 2016-01-22 14:24) - PLID 63732 - begin a progress bar
					CProgressDialog progressDialog;
					BeginProgressDialog(progressDialog);

					NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));

					pExpects->Action = NexTech_Accessor::UpdatePresQueueAction_Delete;
					pExpects->DrugAllergyInteracts = m_nPatientID > 0 ? VARIANT_TRUE : VARIANT_FALSE;
					// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
					pExpects->ExcludeMonographInformation = VARIANT_TRUE;
					pExpects->DrugDiagnosisInteracts = m_nPatientID > 0 ? VARIANT_TRUE : VARIANT_FALSE;
					pExpects->DrugDrugInteracts = m_nPatientID > 0 ? VARIANT_TRUE : VARIANT_FALSE;
					pExpects->RequeryQueue = VARIANT_TRUE;
					pExpects->filters = GetQueueFilters();

					CArray<NexTech_Accessor::_QueuePrescriptionPtr,NexTech_Accessor::_QueuePrescriptionPtr> aryPrescriptions;
					NexTech_Accessor::_QueuePrescriptionPtr pPrescription(__uuidof(NexTech_Accessor::QueuePrescription));
					// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
					pPrescription->PrescriptionID = _bstr_t(AsString(pRow->GetValue(pqcPrescriptionID)));
					aryPrescriptions.Add(pPrescription);

					Nx::SafeArray<IUnknown*> saryPrescriptions = Nx::SafeArray<IUnknown*>::From(aryPrescriptions);
					// (b.savon 2013-03-12 13:11) - PLID 55518 - Use new object structure
					pExpects->PrescriptionsToDelete = saryPrescriptions;
					
					// (j.fouts 2013-04-24 16:55) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter

					NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), 
						GetAPILoginToken(), _bstr_t(FormatString("%li", m_nPatientID)), pExpects);

					// (j.fouts 2013-01-14 10:41) - PLID 54464 - Added Pharmacies
					RefreshQueueFromArray(pResults->PrescriptionsInQueue, pResults->Pharmacies, progressDialog);

					progressDialog.SetLine(2, "Calculating drug interactions...");

					// (j.jones 2013-01-31 17:05) - PLID 53454 - we must show drug interactions
					RefreshInteractionsFromArray(
						pResults->DrugDrugInteracts, 
						pResults->DrugAllergyInteracts,
						pResults->DrugDiagnosisInteracts
						);

					// (j.jones 2016-01-22 14:24) - PLID 63732 - end the progress bar
					progressDialog.Stop();
				}
				break;
			}
		case IDM_CHECK_PRINT:
			{
				//Check all the boxes for print
				for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();pRow;pRow = pRow->GetNextRow())
				{
					// (j.fouts 2012-12-26 11:49) - PLID 53840 - Support null values, since they may not have the license
					if(pRow->GetValue(pqcPrint).vt == VT_BOOL)
					{
						pRow->PutValue(pqcPrint, bPrintAllChecked? g_cvarFalse : g_cvarTrue);
					}
				}
				
				
				UpdatePreviewButton();
				SecureControls();

				break;
			}
		case IDM_CHECK_SEND:
			{
				//Check all the boxes for send
				for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetFirstRow();pRow;pRow = pRow->GetNextRow())
				{
					// (j.fouts 2012-12-26 11:49) - PLID 53840 - Support null values, since they may not have the license
					if(pRow->GetValue(pqcSend).vt == VT_BOOL)
					{
						pRow->PutValue(pqcSend, bSendAllChecked? g_cvarFalse : g_cvarTrue);
					}
				}

				UpdateSendButton();
				SecureControls();
				
				break;
			}
		case IDM_MARK_INCOMPLETE:
			{
				// (j.fouts 2013-01-15 10:02) - PLID 55800 - Added a way to mark a legacy prescription as incomplete
				if(IDYES == MessageBox("Are you sure you want to mark this as Incomplete?\n\rOnce you change this it cannot be changed back.", "Status Change", MB_ICONQUESTION|MB_YESNO))
				{
					CArray<CString, CString&> aryPrescriptionIDs;
					CString strID;
					strID.Format("%li", VarLong(pRow->GetValue(pqcPrescriptionID), -1));
					aryPrescriptionIDs.Add(strID);

					SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_Incomplete);
				}
				break;
			}
		// (b.savon 2013-09-04 15:03) - PLID 58212 - Add a new 'Void' type for Prescriptions
		case IDM_VOID_PRESCRIPTION:
			{
				CString strVoidMessage;
				CString strCallPharmacy;
				if( queueStatus == NexTech_Accessor::PrescriptionStatus_eTransmitSuccess || queueStatus == NexTech_Accessor::PrescriptionStatus_eTransmitPending ){
					strCallPharmacy.Format(
						"\r\n\r\nNexTech will void this prescription in the patient's record but you must call %s at %s to cancel dispensing of the medication.",
						VarString(pRow->GetOutputValue(pqcPharmacy), "the pharmacy"),
						VarString(pRow->GetValue(pqcPharmacyPhone), "(unknown phone number)")
					);
				}

				strVoidMessage.Format(
					"Are you sure you want to void this prescription?  Once you change this it cannot be undone.%s",
					strCallPharmacy
				);

				if(IDYES == MessageBox(strVoidMessage, "Status Change", MB_ICONQUESTION|MB_YESNO))
				{
					CArray<CString, CString&> aryPrescriptionIDs;
					CString strID;
					strID.Format("%li", VarLong(pRow->GetValue(pqcPrescriptionID), -1));
					aryPrescriptionIDs.Add(strID);

					SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_Void);
				}
			}
			break;
		// (b.savon 2013-09-04 15:05) - PLID 58212 - Add a new 'Ready for Doctor Review' type for Prescriptions
		case IDM_DOCTOR_REVIEW_PRESCRIPTION:
			{
				CArray<CString, CString&> aryPrescriptionIDs;
				CString strID;
				strID.Format("%li", VarLong(pRow->GetValue(pqcPrescriptionID), -1));
				aryPrescriptionIDs.Add(strID);

				SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_ReadyForDoctorReview);
			}
			break;
		// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
		case IDM_PRESCRIPTION_DISPENSED_IN_HOUSE:
		{
			CString strVoidMessage;

			strVoidMessage.Format(
				"Are you sure you want to mark this as Dispensed In-House?  Once you change this it cannot be undone.");

			if (IDYES == MessageBox(strVoidMessage, "Status Change", MB_ICONQUESTION | MB_YESNO))
			{
				CArray<CString, CString&> aryPrescriptionIDs;
				CString strID;
				strID.Format("%li", VarLong(pRow->GetValue(pqcPrescriptionID), -1));
				aryPrescriptionIDs.Add(strID);

				SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_DispensedInHouse);
			}
		}
		break;
		case IDM_MARK_PRINTED:
			{
				// (j.fouts 2013-01-15 10:02) - PLID 55800 - Added a way to mark a legacy prescription as printed
				if(IDYES == MessageBox("Are you sure you want to mark this as Printed?\n\rOnce you change this it cannot be changed back.", "Status Change", MB_ICONQUESTION|MB_YESNO))
				{
					CArray<CString, CString&> aryPrescriptionIDs;
					CString strID;
					strID.Format("%li", VarLong(pRow->GetValue(pqcPrescriptionID), -1));
					aryPrescriptionIDs.Add(strID);

					SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_Printed);
				}
				break;
			}
		case IDM_HOLD_PRESCRIPTION:
			{
				// (j.fouts 2013-01-28 16:34) - PLID 53025 - Add On Hold status
				CArray<CString, CString&> aryPrescriptionIDs;
				CString strID;
				strID.Format("%li", VarLong(pRow->GetValue(pqcPrescriptionID), -1));
				aryPrescriptionIDs.Add(strID);

				NexTech_Accessor::PrescriptionStatus queueStatus = (NexTech_Accessor::PrescriptionStatus)VarLong(pRow->GetValue(pqcStatusID), 0 /*No Status*/);
				if(queueStatus == NexTech_Accessor::PrescriptionStatus_OnHold)
				{
					SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_Incomplete);
				}
				else
				{
					SetPrescriptionStatus(aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus_OnHold);
				}
				break;
			}
			// (b.savon 2013-06-19 17:07) - PLID 56880 - Show monograph
		case IDM_SHOW_MONO:
			try
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetCurSel();
				// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
				if(pRow)
				{
					//Get the MedId for the selected drug
					long nFDBMedID = VarLong(pRow->GetValue(pqcFDBID), -1);

					// (j.fouts 2012-08-20 09:26) - PLID 51719 - Don't give an option for monograph to non FDB drugs
					if(nFDBMedID >= 0)
					{
						// (j.fouts 2012-09-25 09:27) - PLID 52825 - Check that the database exists
						if(FirstDataBank::EnsureDatabase(this, true))
						{
							ShowMonograph(nFDBMedID, this);
						}
					}
				}
			}
			NxCatchAll(__FUNCTION__);
			break;
		// (b.savon 2013-06-19 17:29) - PLID 56880 - Show Leaflet
		case IDM_SHOW_LEAFLET: 
			try
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetCurSel();
				// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
				if(pRow)
				{
					long nFDBMedID = VarLong(pRow->GetValue(pqcFDBID), -1);

					if(nFDBMedID >= 0)
					{
						// (j.fouts 2013-06-10 11:18) - PLID 56808 - If the database does not exist we cannot query it
						if(FirstDataBank::EnsureDatabase(this, true))
						{
							ShowLeaflet(nFDBMedID, this);
						}
					}
				}
			}
			NxCatchAll(__FUNCTION__);
			break;
		case IDM_REPRESCRIPE:
			try
			{
				// (r.farnworth 2016-01-06 14:28) - PLID 58692 - Wants to be able to quickly re-prescribe a prescription. Perhaps a right click and "Re-send" or "Re-Prescribe" (This functionality was in NewCrop)
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->GetCurSel();
				if (pRow)
				{
					long nPrescriptionID = pRow->GetValue(pqcPrescriptionID);
					long nPatientID = GetExistingPatientIDByUserDefinedID(VarLong(pRow->GetValue(pqcPatientUserDefiniedID), -1));
					long nMedicationID = pRow->GetValue(pqcMedID);

					long nInsuranceID = -1;
					// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
					if (m_bHasSureScriptsLicense) {
						nInsuranceID = CheckExistingFormularyData();
					}

					CPrescriptionEditDlg dlg(GetParent(), m_nCurrentlyOpenedEMNID);

					PrescriptionDialogReturnValue epdrvReturn = 
						(PrescriptionDialogReturnValue)dlg.EditPrescription(false, CopyPrescription(nPrescriptionID, nMedicationID, nPatientID, nInsuranceID), NULL, FALSE, false, TRUE);

					if (epdrvReturn == epdrvEditRx || epdrvReturn == epdrvErrorRx) {
						
						CDWordArray arNewCDSInterventions;
						ReconcileCurrentMedicationsWithOneNewPrescription(nPatientID, dlg.GetPrescriptionID(), m_queueBkg.GetColor(), this, arNewCDSInterventions);
						GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
						RequeryQueue(false);
					}
				}
			}
			NxCatchAll(__FUNCTION__);
			break;
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-30 10:06) - PLID 53954 - Moved disabling the send button into a function
void CPrescriptionQueueDlg::UpdateSendButton()
{
	// (j.fouts 2012-12-26 11:49) - PLID 53840 - Update only if they have SureScripts
	if(SureScripts::IsEnabled())
	{
		m_nCountSendSelected = 0;
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pPresQueueList->FindByColumn(pqcSend, _variant_t(true), NULL, false);
		NXDATALIST2Lib::IRowSettingsPtr pRow = pFirstRow;

		while(pRow)
		{
			m_nCountSendSelected++;

			pRow = m_pPresQueueList->FindByColumn(pqcSend, _variant_t(true), pRow->GetNextRow(), false);
			if (pRow->IsSameRow(pFirstRow) == VARIANT_TRUE) {
				break;
			}
		}

		// (b.savon 2013-01-21 12:22) - PLID 54720
		if(m_nCountSendSelected <= 0 || !m_bCachedIsLicensedConfiguredUser)
		{
			m_btnESubmit.EnableWindow(FALSE);
		}
		else
		{
			m_btnESubmit.EnableWindow(TRUE);
		}
	}
}

// (j.fouts 2012-11-30 10:07) - PLID 53954 - Moved disabling the preview button into a function
void CPrescriptionQueueDlg::UpdatePreviewButton()
{
	m_nCountPrintSelected = 0;
	NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pPresQueueList->FindByColumn(pqcPrint, _variant_t(true), NULL, false);  
	NXDATALIST2Lib::IRowSettingsPtr pRow = pFirstRow;

	while(pRow)
	{
		m_nCountPrintSelected++;

		pRow = m_pPresQueueList->FindByColumn(pqcPrint, _variant_t(true), pRow->GetNextRow(), false);
		if (pRow->IsSameRow(pFirstRow) == VARIANT_TRUE) {
			break;
		}					
	}

	// (b.savon 2013-01-21 12:21) - PLID 54720
	if(m_nCountPrintSelected <= 0)
	{
		m_btnPreview.EnableWindow(FALSE);
		m_checkSendToPrinter.EnableWindow(FALSE);
	}
	else
	{
		m_btnPreview.EnableWindow(TRUE);
		m_checkSendToPrinter.EnableWindow(TRUE);
	}
}

// (j.fouts 2012-12-26 11:49) - PLID 54340 - Handle the changes from NewCrop in this dlg
LRESULT CPrescriptionQueueDlg::OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (c.haag 2009-05-13 16:18) - PLID 34257 - wParam now holds a pointer to a structure
		NewCropBrowserResult* pNCBR = (NewCropBrowserResult*)wParam;

		if (pNCBR->nPatientID == m_nPatientID) 
		{
			// (c.haag 2010-02-18 09:57) - PLID 37424 - Let the user add newly added prescriptions to the current medication list
			CStringArray astrNewCropRxGUIDs;
			for (int i=0; i < pNCBR->aNewlyAddedPatientPrescriptions.GetSize(); i++) {
				astrNewCropRxGUIDs.Add( pNCBR->aNewlyAddedPatientPrescriptions[i].strPrescriptionGUID );
			}

			if(m_bInMedicationsTab)
			{
				// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also now takes in an
				// optional prescription ID list, but it is not used here
				if(astrNewCropRxGUIDs.GetSize() > 0) {
					CArray<long, long> aryNewPrescriptionIDs;
					CDWordArray arNewCDSInterventions;
					//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
					ReconcileCurrentMedicationsWithMultipleNewPrescriptions(m_nPatientID, aryNewPrescriptionIDs, astrNewCropRxGUIDs, m_queueBkg.GetColor(), this, arNewCDSInterventions);
					GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
				}
			}
			else
			{
				// (j.fouts 2012-12-26 10:48) - PLID 54340
				//If you hit this assert then there is now the functionality to open the queue in EMR
				//with the NewCrop License. Since this came from EMR we should not call MainFrame to 
				//Reconcile Current Medications With New Prescriptions. We should insted implement this
				//in the EMR parent window, or better yet some utility class that can be used into both
				//places.
				ASSERT(FALSE);
			}

			//Current Meds, Allergies, Prescriptions, or Interactions could have changed so refresh
			RequeryQueue(true);
		}

		delete pNCBR;
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (a.wilson 2013-01-03 10:52) - PLID 54410 - update list when response filter changes
void CPrescriptionQueueDlg::SelChangedFilterRenewalResponse(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel), pNew(lpNewSel);

		if (pOld != pNew && pNew)
		{
			RequeryRenewalRequests();
		}
		else if (!pNew)
		{
			m_pRenewalFilterResponseList->PutCurSel(pOld);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-03 10:52) - PLID 54410 - update list when transmit filter changes
void CPrescriptionQueueDlg::SelChangedFilterRenewalTransmit(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel), pNew(lpNewSel);

		if (pOld != pNew && pNew)
		{
			RequeryRenewalRequests();
		}
		else if (!pNew)
		{
			m_pRenewalFilterTransmitList->PutCurSel(pOld);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-03 10:52) - PLID 54410 - update list when from filter changes
void CPrescriptionQueueDlg::OnDtnDatetimechangeFilterRenewalFrom(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		*pResult = 0;
		m_bRenewalFilterTimeChanged = true;
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-03 10:52) - PLID 54410 - update list when to filter changes
void CPrescriptionQueueDlg::OnDtnDatetimechangeFilterRenewalTo(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		*pResult = 0;
		m_bRenewalFilterTimeChanged = true;
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-03 10:52) - PLID 54410 - update list when provider changes
void CPrescriptionQueueDlg::SelChangedFilterRenewalProvider(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel), pNew(lpNewSel);
		if (pNew == NULL || pOld == pNew ) {
			m_pRenewalFilterPrescriberList->PutCurSel(pOld);
			return;
		}

		// (r.gonet 2016-01-22) - PLID 67973 - Have the controller handle the user's selection from the dropdown.
		if (m_renewalPrescriberController.HandleSelection()) {
			RequeryRenewalRequests();
		}

		// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed logic checking the Save as Default Filtered Prescriber checkbox.
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-30 10:06) - PLID 53954 - Make sure these disabled when they need to be
void CPrescriptionQueueDlg::EnforceDisabledControls()
{
	UpdatePreviewButton();
	UpdateSendButton();

	// (j.jones 2012-11-16 16:19) - PLID 53085 - hide the EMN/Patient filter if we don't have an EMNID to filter on
	// (j.fouts 2013-01-04 11:24) - PLID 53970 - Filter on text is now used for all filters
	if(m_nCurrentlyOpenedEMNID == -1) {
		//hide the filters
		GetDlgItem(IDC_RADIO_EMN_MEDS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_PATIENT_MEDS)->ShowWindow(SW_HIDE);
	}

	// (b.savon 2013-01-11 09:07) - PLID 54567
	if( m_nPatientID == -1){ 
		// Don't show the button if we're already on the RxNeedingAttention view
		m_btnRxNeedingAttention.ShowWindow(SW_HIDE);
		m_btnEditFavoritePharmacies.ShowWindow(SW_HIDE);
		// (b.savon 2013-07-16 14:27) - PLID 57377
		m_btnNexFormulary.ShowWindow(SW_HIDE);
		m_btnNexFormulary.EnableWindow(FALSE);
		m_btnRxNeedingAttention.EnableWindow(FALSE);
		m_btnEditFavoritePharmacies.ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_WRITE_RX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_WRITE_RX)->EnableWindow(FALSE);
		GetDlgItem(IDC_WRITE_PICK_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_WRITE_PICK_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_NXDL_WRITE_RX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NXDL_WRITE_RX)->EnableWindow(FALSE);
		GetDlgItem(IDC_NXDL_QUICK_LIST_POPOUT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NXDL_QUICK_LIST_POPOUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT)->EnableWindow(FALSE);
		m_btnWriteMoreMeds.ShowWindow(SW_HIDE);
		m_btnWriteMoreMeds.EnableWindow(FALSE);
		// (j.jones 2016-01-21 17:11) - PLID 67993 - added option to include free text meds in the prescription search
		m_checkIncludeFreeTextRx.ShowWindow(SW_HIDE);
		// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
		m_icoAboutRxMedicationColors.ShowWindow(SW_HIDE);
	}
	else if(!m_bInMedicationsTab)
	{
		m_btnRxNeedingAttention.ShowWindow(SW_HIDE);
		m_btnRxNeedingAttention.EnableWindow(FALSE);
		
		CRect rectMe;
		m_btnEditFavoritePharmacies.GetWindowRect(&rectMe);
		ScreenToClient(&rectMe);

		CRect rectRxButton;
		m_btnRxNeedingAttention.GetWindowRect(&rectRxButton);
		ScreenToClient(&rectRxButton);
	
		m_btnEditFavoritePharmacies.SetWindowPos(NULL, rectMe.left, rectMe.top, rectMe.Width(), rectRxButton.bottom-rectMe.top, NULL);
	}
}

// (a.wilson 2013-01-03 17:36) - PLID 54410 - update list if a date was changed.
void CPrescriptionQueueDlg::OnNMKillfocusFilterRenewalFrom(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		*pResult = 0;
		if (m_bRenewalFilterTimeChanged)
		{
			RequeryRenewalRequests();
			m_bRenewalFilterTimeChanged = false;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-03 17:36) - PLID 54410 - update list if a date was changed.
void CPrescriptionQueueDlg::OnNMKillfocusFilterRenewalTo(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		*pResult = 0;
		if (m_bRenewalFilterTimeChanged)
		{
			RequeryRenewalRequests();
			m_bRenewalFilterTimeChanged = false;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-01-22) - PLID 67967 - Encapsulates the prescription filter loading logic.
void CPrescriptionQueueDlg::InitializeRxFilters()
{
	// (s.tullis 2016-01-28 23:54) - PLID 67968 - As a user, I want to have the prescriber field in the Rx Needing Attention screen to be remembered per user.
	//Prescriber Filter
	if (m_nPatientID == -1) {
		// the prescriber selection.
		m_rxPrescriberController.EnableRemembering("DefaultAttentionPrescriberFilter");
	}
	else {
		m_rxPrescriberController.DisableRemembering();
	}
	// (j.fouts 2012-10-03 16:38) - PLID 53009 - Default to Incomplete
	{
		{
			// (b.eyers 2016-01-25) - PLID 67983 - Adding a row for filtering statuses that require action
			// at the moment these are: incomplete, on hold, e-transmit authorized, e-transmit error, ready for review
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pQueueStatusFilter->GetNewRow();
			pRow->PutValue(qsfcID, -2);
			pRow->PutValue(qsfcStatus, " < Prescriptions Requiring Action >");
			m_pQueueStatusFilter->AddRowBefore(pRow, m_pQueueStatusFilter->GetFirstRow());

			if (m_nPatientID > 0)
			{
				//Add a <Show All> row to Queue Status
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pQueueStatusFilter->GetNewRow();
				pRow->PutValue(qsfcID, pqsInvalid);
				pRow->PutValue(qsfcStatus, " < Show All Statuses >");
				m_pQueueStatusFilter->PutCurSel(m_pQueueStatusFilter->AddRowBefore(pRow, m_pQueueStatusFilter->GetFirstRow()));
			}
			else
			{
				// (b.eyers 2016-01-25) - PLID 67983 - for the prescription needing attention dialog, the 'requires action' statues are default
				//m_pQueueStatusFilter->FindByColumn(qsfcID, pqsIncomplete, NULL, VARIANT_TRUE);
				m_pQueueStatusFilter->FindByColumn(qsfcID, -2, NULL, VARIANT_TRUE);
			}

		}
	}

	// Prescribers
	// (r.gonet 2016-01-22 16:08) - PLID 67967 - Otherwise we'll let the controller default the selection to < All >.

	// (b.savon 2013-01-25 09:07) - PLID 54846 - Add <Show All> option and fill with all prescribers that have a 
	// user role
	// (r.gonet 2016-01-22 16:08) - PLID 67967 - We now use the controller to load up the rows in the prescribers datalist.
	m_rxPrescriberController.InitializeCombo(true);

	// (j.jones 2012-11-16 16:19) - PLID 53085 - hide the EMN/Patient filter if we don't have an EMNID to filter on,
	// otherwise, default to filtering by that EMNID
	// (j.fouts 2013-01-04 11:24) - PLID 53970 - Filter on text is now used for all filters
	if (m_nCurrentlyOpenedEMNID == -1) {
		//hide the filters
		GetDlgItem(IDC_RADIO_EMN_MEDS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_PATIENT_MEDS)->ShowWindow(SW_HIDE);
	} else {
		//show the EMN/Patient filter, remember the user's last setting for the default
		//0 - EMN, 1 - Patient (defaults to patient)
		long nPrescriptionQueueDefaultEMNFilter = GetRemotePropertyInt("PrescriptionQueueDefaultEMNFilter", 1, 0, GetCurrentUserName(), true);
		if (nPrescriptionQueueDefaultEMNFilter == 0) {
			//filter on the EMNID
			m_radioFilterOnEMNMeds.SetCheck(TRUE);
		} else {
			m_radioFilterOnPatientMeds.SetCheck(TRUE);
		}
	}
}

// (a.wilson 2013-01-03 17:36) - PLID 54410 - setup all the renewal filters.
void CPrescriptionQueueDlg::InitializeRenewalFilters()
{
	// (s.tullis 2016-01-28 23:54) - PLID 67968 - As a user, I want to have the prescriber field in the Rx Needing Attention screen to be remembered per user.
	//Prescriber Filter
	if (m_nPatientID == -1) {
		// (r.gonet 2016-01-22 15:33) - PLID 67975 - If we are in Rx Needing Attenion popup dialog, then remember
		// the prescriber selection.
		m_renewalPrescriberController.EnableRemembering("DefaultRenewalPrescriberFilter");
	} else {
		// (r.gonet 2016-01-22 15:33) - PLID 67975 - Otherwise we'll let the controller default the selection to < All >.
		m_renewalPrescriberController.DisableRemembering();
	}
	// (r.gonet 2016-01-22 15:33) - PLID 67973 - Load up the rows in the prescribers datalist.
	m_renewalPrescriberController.InitializeCombo(true);

	// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed logic checking the Save as Default Filtered Prescriber checkbox.
	//Response Filter
	{
		NXDATALIST2Lib::IRowSettingsPtr pRenewalResponseFilterRow = m_pRenewalFilterResponseList->GetNewRow();
		if (pRenewalResponseFilterRow) {
			pRenewalResponseFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_All);
			pRenewalResponseFilterRow->PutValue(rrfcName, _bstr_t(" < All Statuses >"));
			m_pRenewalFilterResponseList->AddRowSorted(pRenewalResponseFilterRow, NULL);
		}
		pRenewalResponseFilterRow = m_pRenewalFilterResponseList->GetNewRow();
		if (pRenewalResponseFilterRow) {
			pRenewalResponseFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_Approved);
			pRenewalResponseFilterRow->PutValue(rrfcName, _bstr_t("Approved"));
			m_pRenewalFilterResponseList->AddRowAtEnd(pRenewalResponseFilterRow, NULL);
		}
		pRenewalResponseFilterRow = m_pRenewalFilterResponseList->GetNewRow();
		if (pRenewalResponseFilterRow) {
			pRenewalResponseFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_ApprovedWithChanges);
			pRenewalResponseFilterRow->PutValue(rrfcName, _bstr_t("Approved with Changes"));
			m_pRenewalFilterResponseList->AddRowAtEnd(pRenewalResponseFilterRow, NULL);
		}
		pRenewalResponseFilterRow = m_pRenewalFilterResponseList->GetNewRow();
		if (pRenewalResponseFilterRow) {
			pRenewalResponseFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_Denied);
			pRenewalResponseFilterRow->PutValue(rrfcName, _bstr_t("Denied"));
			m_pRenewalFilterResponseList->AddRowAtEnd(pRenewalResponseFilterRow, NULL);
		}
		pRenewalResponseFilterRow = m_pRenewalFilterResponseList->GetNewRow();
		if (pRenewalResponseFilterRow) {
			pRenewalResponseFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_DeniedNewRx);
			pRenewalResponseFilterRow->PutValue(rrfcName, _bstr_t("Denied with Rewrite"));
			m_pRenewalFilterResponseList->AddRowAtEnd(pRenewalResponseFilterRow, NULL);
		}
		pRenewalResponseFilterRow = m_pRenewalFilterResponseList->GetNewRow();
		if (pRenewalResponseFilterRow) {
			pRenewalResponseFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_Pending);
			pRenewalResponseFilterRow->PutValue(rrfcName, _bstr_t("Pending"));
			m_pRenewalFilterResponseList->AddRowAtEnd(pRenewalResponseFilterRow, NULL);
			m_pRenewalFilterResponseList->PutCurSel(pRenewalResponseFilterRow);
		}
	}
	//Transmit Filter
	// (b.eyers 2016-02-12) - PLID 67989 - added descriptions for e-transmit filter
	{
		NXDATALIST2Lib::IRowSettingsPtr pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalQueueStatusFilter_All);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t(" < All Statuses >"));
			m_pRenewalFilterTransmitList->AddRowSorted(pRenewalTransmitFilterRow, NULL);
			m_pRenewalFilterTransmitList->PutCurSel(pRenewalTransmitFilterRow);
		}
		// (b.eyers 2016-01-28) - PLID 67985 - add new filter option
		pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, -2);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t(" < Renewals Requiring Action >"));
			//force this to be the second row
			m_pRenewalFilterTransmitList->AddRowAtEnd(pRenewalTransmitFilterRow, NULL);
		}
		pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalQueueStatusFilter_Pending);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t("Pending"));
			pRenewalTransmitFilterRow->PutValue(rrfcDescription, _bstr_t("The renewal has not been sent out to the pharmacy yet."));
			m_pRenewalFilterTransmitList->AddRowSorted(pRenewalTransmitFilterRow, NULL);
		}
		pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalQueueStatusFilter_eTransmitAuthorized);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t("e-Transmit Authorized"));
			pRenewalTransmitFilterRow->PutValue(rrfcDescription, _bstr_t("If an Rx has this status for more than 15 minutes, call NexTech."));
			m_pRenewalFilterTransmitList->AddRowSorted(pRenewalTransmitFilterRow, NULL);
		}
		pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalQueueStatusFilter_eTransmitPending);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t("e-Transmit Pending"));
			pRenewalTransmitFilterRow->PutValue(rrfcDescription, _bstr_t("Any Rx that has successfully been sent to the pharmacy."));
			m_pRenewalFilterTransmitList->AddRowSorted(pRenewalTransmitFilterRow, NULL);
		}
		pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalQueueStatusFilter_eTransmitError);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t("e-Transmit Error"));
			pRenewalTransmitFilterRow->PutValue(rrfcDescription, _bstr_t("Any Rx that has encountered an error in the transmission process."));
			m_pRenewalFilterTransmitList->AddRowSorted(pRenewalTransmitFilterRow, NULL);
		}
		pRenewalTransmitFilterRow = m_pRenewalFilterTransmitList->GetNewRow();
		if (pRenewalTransmitFilterRow) {
			pRenewalTransmitFilterRow->PutValue(rrfcID, NexTech_Accessor::RenewalQueueStatusFilter_eTransmitSuccess);
			pRenewalTransmitFilterRow->PutValue(rrfcName, _bstr_t("e-Transmit Success"));
			pRenewalTransmitFilterRow->PutValue(rrfcDescription, _bstr_t("Any Rx that has been sent and received by the pharmacy successfully."));
			m_pRenewalFilterTransmitList->AddRowSorted(pRenewalTransmitFilterRow, NULL);
		}

	}
	//To, From Filter
	{
		COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtMonth(30, 0, 0, 0);
		m_dtpRenewalFilterTo.SetValue(dtCurrent);
		m_dtpRenewalFilterFrom.SetValue((dtCurrent - dtMonth));
	}
}

// (j.fouts 2013-01-03 12:28) - PLID 54429 - Tell the interactions dlg to check for interactions
void CPrescriptionQueueDlg::CheckInteractions()
{
	// (j.fouts 2013-05-30 09:40) - PLID 56807 - Drug Interactions is now tied to the SureScripts License
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if(m_bHasSureScriptsLicense) 
	{
		// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(this, true)) 
			{
				//We decided not to have an additional warning here about drug interactions not
				//being check because if FDB is not built yet then there should really be no way
				//that they have FDB drugs/allergies in their database yet.
				return;
			}
		}

		NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));
		pExpects->DrugAllergyInteracts = VARIANT_TRUE;
		pExpects->DrugDiagnosisInteracts = VARIANT_TRUE;
		pExpects->DrugDrugInteracts = VARIANT_TRUE;
		// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
		pExpects->ExcludeMonographInformation = VARIANT_TRUE;
		pExpects->RequeryQueue = VARIANT_FALSE;

		// (j.fouts 2013-04-24 16:55) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
		// (b.savon 2013-03-12 13:11) - PLID 55518 - Remove last param
		NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), 
			GetAPILoginToken(), _bstr_t(FormatString("%li", m_nPatientID)), pExpects);

		// (j.fouts 2012-08-10 09:46) - PLID 52089 - Allergies Changed Check Interactions
		// (j.fouts 2013-01-07 10:44) - PLID 54468 - Don't requery the whole queue, just check interactions
		RefreshInteractionsFromArray(
			pResults->DrugDrugInteracts, 
			pResults->DrugAllergyInteracts,
			pResults->DrugDiagnosisInteracts
		);
	}
}
// (j.fouts 2013-01-04 13:31) - PLID 54456 - Added a new button to write from pick list
void CPrescriptionQueueDlg::OnBnClickedWritePickList()
{
	try
	{
		CString sql, description;

		//check to see that they have permissions - this will prompt a password
		if(!CheckCurrentUserPermissions(bioPatientMedication, sptCreate)) {
			return;
		}

		// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(this, true)) 
			{
				//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
				return;
			}
		}

		// (b.savon 2013-01-23 11:56) - PLID 54782
		m_nxdlQuickList->Clear();
		long nHeight = 0;
		long nResults = 0;
		// Insert the Write prescription row
		NXDATALIST2Lib::IRowSettingsPtr pRow = InsertWritePrescriptionRow(m_nxdlQuickList);
		if( pRow ){
			nHeight += pRow->GetHeight();
			++nResults;
		}

		NexTech_Accessor::_ERxQuickListPtr pQuickList = GetAPI()->GetQuickList(GetAPISubkey(), GetAPILoginToken(), _bstr_t(GetCurrentUserID()));
		// Add My QuickList
		BOOL bHasQuickList = FALSE;
		Nx::SafeArray<IUnknown *> saryMedications(pQuickList->Medications);
		foreach(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication, saryMedications){
			//Add each of our drugs to the list
			pRow = m_nxdlQuickList->GetNewRow();
			// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added route and unit
			if( pRow ){
				pRow->PutValue(qlcID, atol(pMedication->QuickListID));
				// (r.gonet 2016-02-10 13:34) - PLID 58689 - Stow the order index, which will
				// control the display order of this medication in the quicklist.
				pRow->PutValue(qlcOrderIndex, pMedication->OrderIndex);
				pRow->PutValue(qlcCheck, g_cvarFalse);
				pRow->PutValue(qlcName, AsBstr(GetDisplayName(pMedication)));
				pRow->PutValue(qlcDrugListID, atol(pMedication->DrugListID));
				// (b.savon 2013-08-23 14:45) - PLID 58236 - Add FDBID
				pRow->PutValue(qlcFDBID, atol(pMedication->FDBID));
				pRow->PutValue(qlcRefill, pMedication->Refills);
				pRow->PutValue(qlcQuantity, pMedication->Quantity);
				pRow->PutValue(qlcSig, pMedication->Sig);
				pRow->PutValue(qlcDosageRouteID, atol(pMedication->DosageRoute->ID));
				pRow->PutValue(qlcDosageRoute, pMedication->DosageRoute->Route);
				pRow->PutValue(qlcDosageFrequency, pMedication->DosageFrequency);
				pRow->PutValue(qlcDosageQuantity, pMedication->DosageQuantity);
				pRow->PutValue(qlcDosageUnitID, atol(pMedication->DosageUnit->ID));
				pRow->PutValue(qlcDosageUnit, pMedication->DosageUnit->Unit);
				pRow->PutValue(qlcNotes, pMedication->NoteToPharmacist);
				// (b.savon 2013-08-23 13:42) - PLID 58236 - Add formulary icon
				if( atol(pMedication->FDBID) != -1 ){
					pRow->PutValue(qlcIcon, (long)m_hIconNexFormulary);
				}
				pRow = m_nxdlQuickList->AddRowSorted(pRow, NULL);
				nHeight += pRow->GetHeight();
				++nResults;
				bHasQuickList = TRUE;
			}
		}

		//If we don't have a quicklist, remove the Write Selected row
		if( !bHasQuickList ){
			nHeight -= pRow->GetHeight();
			--nResults;
			m_nxdlQuickList->RemoveRow(pRow);
		}

		// Add My Supervisors (if any)
		Nx::SafeArray<IUnknown *> sarySupervisorLists(pQuickList->SupervisorLists);
		long nIndexCount = 1;
		m_arySupervisorMeds.RemoveAll();
		foreach(NexTech_Accessor::_ERxQuickListPtr pList, sarySupervisorLists){
			pRow = m_nxdlQuickList->GetNewRow();
			if( pRow ){
				// (b.savon 2013-08-23 13:40) - PLID 58236 - Load Icon
				if( m_hIconArrow == NULL ){					
					m_hIconArrow = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_TB_RIGHT_ARROW), IMAGE_ICON, 16, 16, 0);
				}
				pRow->PutCellLinkStyle(qlcName, NXDATALIST2Lib::dlLinkStyleTrue);
				pRow->PutValue(qlcID, (long)(QUICK_LIST_SUPERVISOR_ROW*nIndexCount));
				pRow->PutValue(qlcCheck, g_cvarNull);
				pRow->PutValue(qlcName, pList->PrescriberName);
				// (b.savon 2013-08-23 13:32) - PLID 58236 - Use the Arrow icon
				pRow->PutValue(qlcIcon, (long)m_hIconArrow);
				pRow->PutBackColor(dlcSupervisor);
				m_nxdlQuickList->AddRowAtEnd(pRow, NULL);
				nHeight += pRow->GetHeight();
				++nResults;
				++nIndexCount;

				//Store their meds in a array.
				Nx::SafeArray<IUnknown *> sarySupervisorMedications(pList->Medications);
				m_arySupervisorMeds.Add(sarySupervisorMedications);	
			}
		}

		// Add The < Configure... > Row
		pRow = InsertConfigureRow(m_nxdlQuickList);
		if( pRow ){
			nHeight += pRow->GetHeight();
			++nResults;
		}

		SetQuickListResultsSize(TRUE, nResults, nHeight);
		GetDlgItem(IDC_NXDL_QUICK_LIST_POPOUT)->SetFocus();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-04 13:31) - PLID 54456 - Moved opening the Prescription Edit dlg into a function as not to duplicate code
// (a.wilson 2013-01-28 10:17) - PLID 53912 add the ability to pass a patientid for prescriptions needing attention.
// (s.dhole 2013-10-19 11:24) - PLID 59068  added nFInsuranceID
void CPrescriptionQueueDlg::WritePrescription(long nMedicationID, long nPatientID /* = -1 */, long nFInsuranceID /* = -1 */)
{
	//TES 5/19/2008 - PLID 28523 - OK, they either picked one of the quick list, or they picked "<More Medications...>"
	// and then selected one from there, so we're ready to go ahead and make the prescription.
	if (nPatientID == -1)
		nPatientID = m_nPatientID;
	//check allergies before moving on to the prescription screen
	if(!CheckAllergies(nPatientID, nMedicationID)) {
		return;
	}

	// (j.jones 2008-05-13 12:31) - PLID 29732 - now we use a prescription editor
	// (j.jones 2012-11-06 10:02) - PLID 52819 - pass in our EMNID
	CPrescriptionEditDlg dlg(this, m_nCurrentlyOpenedEMNID);

	// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we created or deleted a prescription
	// (j.jones 2012-11-19 17:52) - PLID 52818 - added default provider ID, location ID, and date
	// (j.fouts 2013-03-12 10:17) - PLID 52973 - Seperated Creating a prescription out from the prescription Edit Dlg
	// (s.dhole 2013-10-19 11:25) - PLID 59068 Pass nFInsuranceID
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = SaveNewPrescription(nMedicationID, TRUE, GetQueueFilters(), nPatientID,
		m_nDefaultProviderID, m_nDefaultLocationID, m_dtDefaultPrescriptionDate, m_nCurrentlyOpenedEMNID,0,SourceActionInfo(),-1,-1,nFInsuranceID);

	// (b.savon 2013-03-19 09:14) - PLID 55477
	if( pResults->PrescriptionsAdded == NULL ){
		ThrowNxException("Unable to Add Prescription in CPrescriptionQueueDlg::WritePrescription!");
	}

	Nx::SafeArray<IUnknown*> saryPrescriptions(pResults->PrescriptionsAdded);

	if(saryPrescriptions.GetCount() == 0)
	{
		ThrowNxException("Attempted to create a new prescription, but no prescription was returned."); 
	}

	// (j.jones 2016-01-22 14:24) - PLID 63732 - begin a progress bar
	CProgressDialog progressDialog;
	BeginProgressDialog(progressDialog);

	// (b.savon 2013-03-19 17:08) - PLID 55477 - Reworked to use API
	PrescriptionInfo rxInformation;
	NexTech_Accessor::_QueuePrescriptionPtr pPrescription = saryPrescriptions[0];
	rxInformation.pPrescription = pPrescription;
	rxInformation.erxUserRole = pResults->UserRole;
	rxInformation.saryPrescribers = pResults->Prescriber;
	rxInformation.sarySupervisors = pResults->Supervisor;
	rxInformation.saryNurseStaff = pResults->NurseStaff;

	RefreshQueueFromArray(pResults->PrescriptionsInQueue, pResults->Pharmacies, progressDialog);

	// (j.jones 2016-01-22 14:24) - PLID 63732 - end the progress bar
	progressDialog.Stop();

	// (j.jones 2013-11-25 09:55) - PLID 59772 - for new prescriptions we pass in the drug interactions info
	DrugInteractionInfo drugInteractionInfo;
	if(pResults->DrugDrugInteracts) {
		drugInteractionInfo.saryDrugDrugInteracts = pResults->DrugDrugInteracts;
	}
	if(pResults->DrugAllergyInteracts) {
		drugInteractionInfo.saryDrugAllergyInteracts = pResults->DrugAllergyInteracts;
	}
	if(pResults->DrugDiagnosisInteracts) {
		drugInteractionInfo.saryDrugDiagnosisInteracts = pResults->DrugDiagnosisInteracts;
	}

	// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
	PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(true, rxInformation, &drugInteractionInfo);

	// (j.fouts 2013-03-12 15:07) - PLID 52973 - The EditDlg is no longer responsible for creating new prescriptions, so
	// this should only return Edit or Delete
	// (s.dhole 2013-05-20 10:30) - PLID 56734 this may return epdrvErrorRx if there is sure script error
	if(epdrvReturn == epdrvEditRx || epdrvReturn == epdrvErrorRx) {
		// (c.haag 2010-02-17 10:19) - PLID 37384 - Let the user apply the prescriptions to the current medications list.
		// (j.jones 2010-08-23 09:23) - PLID 40178 - this is user-created so the NewCropGUID is empty
		// (j.jones 2011-05-02 15:39) - PLID 43450 - pass in the patient explanation as the sig
		// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also no longer needs the Sig nor NewCropGUID
		//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
		CDWordArray arNewCDSInterventions;
		ReconcileCurrentMedicationsWithOneNewPrescription(nPatientID, dlg.GetPrescriptionID(), m_queueBkg.GetColor(), this, arNewCDSInterventions);
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

		// (j.jones 2013-01-31 17:05) - PLID 53454 - when we requery the queue to show the new prescription,
		// we must also show drug interactions
		// (j.jones 2013-11-25 14:50) - PLID 59812 - we don't need to show drug interactions again,
		// the new prescription would have already done so
		RequeryQueue(false);
	}
}

// (b.savon 2013-01-07 12:08) - PLID 54461 - Moved from medicationsdlg
void CPrescriptionQueueDlg::OnBnClickedBtnEditFavoritePharmacies()
{
	try {

		//it makes sense that only those who have permissions to edit prescriptions
		//should have permissions to edit the patient's pharmacy list
		if(!CheckCurrentUserPermissions(bioPatientMedication,sptWrite)) {
			return;
		}

		CFavoritePharmaciesEditDlg dlg(this);
		dlg.m_nPatientID = m_nPatientID;
		dlg.m_strPatientName = m_bInMedicationsTab ? GetActivePatientName() : GetExistingPatientName(m_nPatientID);
		dlg.DoModal();

		// (j.fouts 2013-03-05 10:52) - PLID 54464 - We need to requery since the Pharmacy combo could have changed
		RequeryQueue(false);

	}NxCatchAll("Error in CPrescriptionQueueDlg::OnBnClickedBtnEditFavoritePharmacies");
}

// (b.savon 2013-01-11 09:46) - PLID 53848 - Handle sending prescription status changes
LRESULT CPrescriptionQueueDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
		case NetUtils::PatientMedications:
			try {
				//Only requery if the queue is open for the patient we sent the table checker for or were in 'RxNeedingAttention' mode.
				_variant_t varPrescriptionID((long)lParam);
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->FindByColumn(pqcPrescriptionID, varPrescriptionID, NULL, VARIANT_FALSE);
				if(pRow)
				{
					//The row has changed
					RefreshSingleQueueRow(pRow);
				}
			} NxCatchAll("Error in CPrescriptionQueueDlg::OnTableChanged:PatientMedications");
			break;
		// (a.wilson 2013-01-16 16:13) - PLID 54660 - renewals table checker
		case NetUtils::RenewalRequests:
			try {
				//requery if the table checker is for rxneeding attention, for patient in meds tab or emr or its new messages.
				if ((long)lParam == m_nPatientID && (m_bInMedicationsTab || m_nCurrentlyOpenedEMNID != -1)
					|| m_nPatientID == -1 
					|| (long)lParam == 0) {

					RequeryRenewalRequests();
				}
			} NxCatchAll("Error in CPrescriptionQueueDlg::OnTableChanged:RenewalRequests");
			break;
		// (j.jones 2013-04-05 13:38) - PLID 56114 - if showing allergies or meds, requery them
		case NetUtils::CurrentPatientMedsT:
			try {
				//requery only if the tablechecker variable reports as changed (ie. we didn't just change it)
				//and it's for all patients, or just this patient
				if (((long)lParam == m_nPatientID || m_nPatientID == -1 || (long)lParam <= 0)
					&& m_CurrentMedsChecker.Changed()
					&& m_bShowMedsAllergies) {

					RequeryAllergiesAndMeds(false, true);
				}
			} NxCatchAll("Error in CPrescriptionQueueDlg::OnTableChanged:CurrentPatientMedsT");
			break;
		// (j.jones 2013-04-05 13:38) - PLID 56114 - if showing allergies or meds, requery them
		case NetUtils::PatientAllergyT:
			try {
				//requery only if the tablechecker variable reports as changed (ie. we didn't just change it)
				//and it's for all patients, or just this patient
				if (((long)lParam == m_nPatientID || m_nPatientID == -1 || (long)lParam <= 0)
					&& m_PatientAllergiesChecker.Changed()
					&& m_bShowMedsAllergies) {

					RequeryAllergiesAndMeds(true, false);
				}
			} NxCatchAll("Error in CPrescriptionQueueDlg::OnTableChanged:PatientAllergyT");
			break;
		case NetUtils::MedicationHistoryResponseT:
			try {
				if((long)lParam == m_nPatientID) {
					// (r.gonet 09/20/2013) - PLID 58396 - This patient has history, so color
					// this button red so that the user will be alerted to that fact.
					m_btnMedicationHistory.SetTextColor(RGB(255,0,0));
				}
			} NxCatchAll("Error in CPrescriptionQueueDlg::OnTableChanged:MedicationHistoryResponseT");
			break;
		}
	} NxCatchAll("Error in CPrescriptionQueueDlg::OnTableChanged");

	return 0;
}

// (b.savon 2013-01-11 09:46) - PLID 53848 - Handle sending prescription status changes
void CPrescriptionQueueDlg::OnDestroy()
{
	try{
		//Unrequest the table checker messages when done.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

// (r.gonet 2016-01-22 15:38) - PLID 67977 - Removed OnBnClickedRenewalDefaultPrescriber
// (b.savon 2013-01-10 16:09) - PLID 54567 - Rx Needing attention on medications dlg
void CPrescriptionQueueDlg::OnBnClickedBtnRxNeedingAttention()
{
	try{

		GetMainFrame()->ShowPrescriptionsNeedingAttention();

	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-01-16 12:44) - PLID 53797 - display options when the user right clicks on a row.
void CPrescriptionQueueDlg::RButtonUpRenewalRequestList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		// (a.wilson 2013-04-02 11:12) - PLID 56001 - enumeration to define each context menu option.
		enum RenewalQueueMenuOptions
		{
			rqmoNone = 0,
			rqmoOpen = 1,
			rqmoAssignPatient = 2,
			rqmoRetransmit = 3,
		};

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_pRenewalList->PutCurSel(pRow);

			CPoint pt(x, y);
			GetCursorPos(&pt);
			CMenu  menPopup;
			menPopup.m_hMenu = CreatePopupMenu();

			menPopup.InsertMenu(-1, MF_BYPOSITION, rqmoOpen, "Open");
			// (a.wilson 2013-04-30 17:57) - PLID 56509 - check if the current user can modify renewals.
			if (boost::exists(m_nexerxUser.aryPrescribingIDs, VarLong(pRow->GetValue(rrcPrescriberID), -1))) {

				if (m_nPatientID <= 0 && AsLong(pRow->GetValue(rrcPatientID)) <= 0 && AsLong(pRow->GetValue(rrcResponseStatusID)) == (long)NexTech_Accessor::RenewalResponseStatus_Pending)
				{
					menPopup.InsertMenu(-1, MF_BYPOSITION, rqmoAssignPatient, "Assign Patient");
				} else if (AsLong(pRow->GetValue(rrcPatientID)) > 0 && AsLong(pRow->GetValue(rrcResponseStatusID)) == (long)NexTech_Accessor::RenewalResponseStatus_Pending) {
					menPopup.InsertMenu(-1, MF_BYPOSITION, rqmoAssignPatient, "Reassign Patient");
				}
				// (a.wilson 2013-04-01 15:15) - PLID 56001 - add the ability to resend a renewal if an error occurs.
				if (AsLong(pRow->GetValue(rrcETransmitStatusID)) == (long)NexTech_Accessor::PrescriptionStatus_eTransmitError)
				{
					menPopup.InsertMenu(-1, MF_BYPOSITION, rqmoRetransmit, "Retransmit");
				}
			}

			long nCmdID = menPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

			switch (nCmdID)
			{
			case rqmoOpen:	//Open
				LeftClickRenewalRequestList(lpRow, rrcPrescription, x, y, nFlags);
				break;
			case rqmoAssignPatient:	//Assign Patient
				{
					NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();

					if (pApi) {
						NexTech_Accessor::_ERxRenewalRequestInfoPtr pRenewalInfo = pApi->GetRenewalRequestInfo(GetAPISubkey(), 
							GetAPILoginToken(), _bstr_t(pRow->GetValue(rrcRenewalID)));

						if (pRenewalInfo && pRenewalInfo->GetPatientInfo()) {
							CAssignPatientRenewalDlg dlg(this, pRenewalInfo->GetPatientInfo());

							if (IDOK == (dlg.DoModal()))
							{
								//update renewal pointer and database.
								long nPatientID = dlg.GetAssignedPatientID();
								if (nPatientID != -1)
								{
									pApi->CommitAssignedRenewalPatientID(GetAPISubkey(), GetAPILoginToken(), 
	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
										_bstr_t(GetExistingPatientUserDefinedID(nPatientID)), _bstr_t(pRenewalInfo->GetID()));

									// (a.wilson 2013-04-11 16:28) - PLID 55973 - remove the row if it was assigned to a different patient in meds tab.
									if (m_nPatientID > 0 && m_nPatientID != nPatientID)
									{
										m_pRenewalList->RemoveRow(pRow);
									}
								}
							}
						}
					}
				}
				break;
			// (a.wilson 2013-04-02 11:08) - PLID 56001 - retransmit the renewal request if it errored.
			case rqmoRetransmit:	//Retransmit errored renewal.
				{
					CString strErrorMessage = AsString(pRow->GetValue(rrcETansmitReason));
					if (IDYES == MessageBox(FormatString("The renewal you are trying to retransmit has the following error:\r\n\r\n%s\r\n\r\n"
						"Are you sure you want to retransmit this renewal?", strErrorMessage), "Retransmit Renewal", MB_YESNO | MB_ICONQUESTION)) {
						//run api function to reset renewal response to authorized.
						NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
						long nRenewalID = AsLong(pRow->GetValue(rrcRenewalID));

						if (pApi && nRenewalID > 0) {
							pApi->RetransmitRenewalResponse(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nRenewalID));
						} else {
							//this should not happen.
							ASSERT(FALSE);
						}
					}
				}
				break;
			}
		}

	} NxCatchAll(__FUNCTION__);
}




// (b.savon 2013-01-21 14:40) - PLID 54678

void CPrescriptionQueueDlg::OnClickWritePrescriptionFromSearchResults(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		// The caller should responsible for passing a valid row
		if( pRow == NULL ){
			return;
		}

		//check to see that they have permissions - this will prompt a password
		if(!CheckCurrentUserPermissions(bioPatientMedication, sptCreate)) {
			return;
		}
		//(s.dhole 3/10/2015 11:37 AM ) - PLID 64561
		long nMedicationID = VarLong(pRow->GetValue(mrcMedicationID), -1);
		
		// (b.savon 2013-01-30 09:53) - PLID 54922
		//If the medicationid is -1, we need to import it from FDB
		if( nMedicationID == -1 ){
			ImportMedication(pRow, nMedicationID);
		}

		if(nMedicationID != -1) {
			// (s.dhole 2013-10-25 17:24) - PLID 59189 Check if folrmuary information exist 
			long nInsuranceID = -1;
			// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
			if (m_bHasSureScriptsLicense){
				 nInsuranceID = CheckExistingFormularyData();
			}
			
			
			WritePrescription(nMedicationID,-1,nInsuranceID);
			//must requery now no matter what, because the Latin naming may have changed
			RequeryQueue();
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-10-25 17:24) - PLID 59189 Check if folrmuary information exist 
long CPrescriptionQueueDlg::CheckExistingFormularyData()
{
	// (b.savon 2014-08-26 10:38) - PLID 63401 - Forward to our utility func
	return ::CheckExistingFormularyData(this, m_nPatientID);
}

void CPrescriptionQueueDlg::OnTimer(UINT_PTR nIDEvent)
{
	try{
		switch (nIDEvent) {
			// (b.savon 2013-01-24 14:31) - PLID 54782 - Kill the timers and hide the appropriate lists
			case IDT_HIDE_QUICK_LIST_RESULTS:
			{
				KillTimer(IDT_HIDE_QUICK_LIST_RESULTS);
				KillTimer(IDT_HIDE_QUICK_LIST_SUPERVISOR_RESULTS);
				SetQuickListResultsSize(FALSE, 0, 0);
				SetQuickListSupervisorResultsSize(FALSE, 0, 0);
			}
			break;
			case IDT_HIDE_QUICK_LIST_SUPERVISOR_RESULTS:
			{
				KillTimer(IDT_HIDE_QUICK_LIST_SUPERVISOR_RESULTS);
				SetQuickListSupervisorResultsSize(FALSE, 0, 0);
			}
			break;
			// (s.tullis 2016-01-28 17:45) - PLID 67965
			// (s.tullis 2016-01-28 17:46) - PLID 68090
			// Timer invoked to save
			case IDT_SAVE_REMINDER:
			{
				KillTimer(IDT_SAVE_REMINDER);
				SaveReminder(m_renewalsBox.IsCollapsed() ? m_PrescriptionNeedingAttention : m_Renewals);
			}
			break;
		}

	}NxCatchAll(__FUNCTION__)

	CNxDialog::OnTimer(nIDEvent);
}

// (b.savon 2013-01-18 14:39) - PLID 54691
void CPrescriptionQueueDlg::OnBnClickedBtnEditMedsQueue()
{
	try{

		// (will prompt for passwords)
		if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
			return;
		}

		//open the edit medication dialog

		CEditMedicationListDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-21 14:41) - PLID 54691
void CPrescriptionQueueDlg::AddMedToCurrentMedicationsList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{

		CString sql, description;

		//Make sure we have a valid row
		if( pRow == NULL ){
			return;
		}

		//check to see that they have permissions - this will prompt a password - Split 'Current Meds' to their own permission
		if(!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptCreate))
			return;

		//Get the MedicationID and FirstDataBankID
		//(s.dhole 3/10/2015 11:37 AM ) - PLID 64561
		long nMedicationID = VarLong(pRow->GetValue(mrcMedicationID), -1);
		
		// (b.savon 2013-01-30 09:53) - PLID 54922
		//If the medicationid is -1, we need to import it from FDB
		if( nMedicationID == -1 ){
			ImportMedication(pRow, nMedicationID);
		}
		//(s.dhole 3/10/2015 11:37 AM ) - PLID 64561
		long nFirstDataBankID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);

		//check to make sure that it isn't a duplicate
		// (b.savon 2013-03-26 08:51) - PLID 54854 - Parameterized and only check if the med is a duplicate of a active current med.
		// If there is a med that is discontinued, we'll update its status to active.
		if (ReturnsRecordsParam("SELECT ID FROM CurrentPatientMedsT WHERE PatientID = {INT} AND MedicationID = {INT} AND Discontinued = 0", m_nPatientID, nMedicationID)) {

			MsgBox("This medication is already in the list for this patient.");
			return;
		}

		//Check that the patient isn't allergic to it.
		CString strAllergies;
		int nAllergyCount = 0;
		if(!CheckAllergies(GetActivePatientID(), nMedicationID, TRUE, &strAllergies, &nAllergyCount)) {
			CString strMessage;
			if(nAllergyCount > 1) {
				strMessage.Format("Warning: The patient has the following allergies, which conflict with this medication:\r\n%s", strAllergies);
			}
			else {
				strMessage.Format("Warning: The patient has an allergy (%s) which conflicts with this medication.", strAllergies);
			}
			MsgBox("%s", strMessage);
		}
											
		// (c.haag 2007-02-02 17:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		// (c.haag 2007-10-22 10:35) - PLID 27827 - We also need the Emr Data ID
		_RecordsetPtr rs = CreateParamRecordset("SELECT EMRDataT.Data AS Name, DrugList.EMRDataID FROM DrugList "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE DrugList.ID = {INT}", nMedicationID);
			
		FieldsPtr fields;
		fields = rs->Fields;

		//Put the variables from the recordset into local variables
		CString strName;
		long nEmrDataID;
		
		strName = VarString(fields->Item["Name"]->Value);
		nEmrDataID = VarLong(fields->Item["EMRDataID"]->Value);
		
		rs->Close();

		//Insert all the defaults into the PatientMedications Table
		// (j.jones 2010-01-22 11:31) - PLID 37016 - supported InputByUserID
		// (b.savon 2013-03-26 09:14) - PLID 54854 - Update the status if discontinued, otherwise add it as new.
		// (j.armen 2013-06-27 17:10) - PLID 57359 - Idenitate CurrentPatientMedsT
		// (s.dhole 2013-11-19 09:38) - PLID 56926 Added last updatedate
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"IF EXISTS (SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID = {INT} AND PatientID = {INT} AND Discontinued = 1) \r\n"
			"BEGIN \r\n"
			"	UPDATE CurrentPatientMedsT SET Discontinued = 0, DiscontinuedDate = NULL ,LastUpdateDate = GETDATE() "
			"	OUTPUT inserted.ID "
			"	WHERE MedicationID = {INT} AND PatientID = {INT} \r\n"
			"END "
			"ELSE "
			"BEGIN "
			"	INSERT INTO CurrentPatientMedsT (PatientID, MedicationID, InputByUserID,LastUpdateDate) "
			"		OUTPUT inserted.ID "
			"		VALUES ({INT}, {INT}, {INT},GETDATE()) "
			"END ",
			nMedicationID, m_nPatientID, nMedicationID, m_nPatientID, m_nPatientID, nMedicationID, GetCurrentUserID());

		long nID = AdoFldLong(prs, "ID");
			
		// (c.haag 2007-10-19 17:13) - PLID 27827 - Audit
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiCurrentPatientMedsAdd, nEmrDataID, "", strName, aepHigh, aetCreated);

		// (j.jones 2012-10-17 16:11) - PLID 51713 - clear the HasNoMeds status
		if(m_checkHasNoMeds.GetCheck()) {
			m_checkHasNoMeds.SetCheck(FALSE);
			//this function will save, audit
		// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
			UpdateHasNoMedsStatus(FALSE, m_nPatientID);
		}
		
		// (c.haag 2007-01-25 12:42) - PLID 24420 - Warn the user about any EMR's with "official"
		// Current Medications details that are inconsistent with the patient's official medication list
		// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
			WarnMedicationDiscrepanciesWithEMR(m_nPatientID);
		}
		// (c.haag 2010-09-21 13:43) - PLID 40610 - Create todo alarms for decisions
		CDWordArray arNewCDSInterventions;
		//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
		UpdateDecisionRules(GetRemoteData(), m_nPatientID, arNewCDSInterventions);
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

		//must requery now, because Latin naming may have changed
		// (j.fouts 2012-11-14 11:59) - PLID 53439 - Requery the queue
		RequeryQueue();

		m_pMedsList->Requery();

		// (j.jones 2013-04-05 14:22) - PLID 56114 - send a tablechecker
		m_CurrentMedsChecker.Refresh(m_nPatientID);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-21 14:42) - PLID 54704
void CPrescriptionQueueDlg::AddAllergyToCurrentAllergyList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		//Don't do anything if they don't have permissions
		if(!CheckCurrentUserPermissions(bioPatientAllergies, sptCreate))
			return;

		if( pRow ){
			long nAllergyID = VarLong(pRow->GetValue(aslcAllergyID), -1);
			long nMedFromFDB = VarLong(pRow->GetValue(aslcStatus), -1);
			long nConceptIDType = VarLong(pRow->GetValue(aslcConceptTypeID), -1);
			// (b.savon 2013-01-30 09:53) - PLID 54922
			//If the medicationid is -1, we need to import it from FDB
			if( nAllergyID == -1 ){
				ImportAllergy(pRow, nAllergyID);
			}
			
			CString strAllergyName = VarString(pRow->GetValue(aslcAllergyName), "");
			if( strAllergyName.IsEmpty() ){
				return;
			}

			// (b.savon 2012-09-26 09:56) - PLID 52107 - check to make sure that it isn't a duplicate from Practice/FDB Imported Meds.
			// NewCropBrowserDlg line 1802 it says "we allow duplicate entries of allergies, so if they already have a non-newcrop allergy
			// that is the exact same as the newcrop allergy, we are going to add the duplicate"
			// (b.savon 2013-03-26 09:37) - PLID 54854 - Added Distonctinued
			if (ReturnsRecordsParam(
				"SELECT	1 \r\n"
				"FROM	PatientAllergyT  \r\n"
				"	INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID  \r\n"
				"	INNER JOIN EMRDataT ON AllergyT.EmrDataID = EMRDataT.ID \r\n"
				"WHERE PersonID = {INT} AND PatientAllergyT.RXCUI = '' AND EMRDataT.Data = {STRING} AND Discontinued = 0 \r\n"
				, m_nPatientID, strAllergyName)) {

				MsgBox((LPCTSTR)(strAllergyName + " is already in the list for this patient."));
				return;
			}

			// (c.haag 2007-02-02 17:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			_RecordsetPtr rsDrugs = CreateRecordset("SELECT EMRDataT.Data AS Name FROM DrugList INNER JOIN DrugAllergyT ON DrugList.ID = "
				"DrugAllergyT.DrugID "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"WHERE DrugAllergyT.AllergyID = %li AND (DrugList.ID IN "
				"(SELECT MedicationID FROM CurrentPatientMedsT WHERE PatientID = %li) OR DrugList.ID IN "
				"(SELECT MedicationID FROM PatientMedications WHERE PatientID = %li AND Deleted = 0))", nAllergyID, m_nPatientID,
				m_nPatientID);
			if(!rsDrugs->eof) {
				CString strDrugNames;
				while(!rsDrugs->eof) {
					strDrugNames += AdoFldString(rsDrugs, "Name") + "\r\n";
					rsDrugs->MoveNext();
				}
				strDrugNames = strDrugNames.Left(strDrugNames.GetLength()-2);
				MsgBox("Warning!  The following drugs are contraindicated for patients with this allergy, and are currently being taken by this patient: \r\n%s", strDrugNames);
			}

			// (j.luckoski 2013-03-26 10:09) - PLID 53806 - We need the ID for the ingredient import
			long nPatAllergyID = NewNumber("PatientAllergyT", "ID");

			//Insert the new record if the clicked OK;
			// (c.haag 2009-12-22 12:30) - PLID 35766 - Added EnteredDate
			// (b.savon 2013-03-26 09:35) - PLID 54854 - parameterized and update discontinue flag if set for allergy
			// (s.dhole 2013-11-19 09:39) - PLID 56931 Added updatedate
			_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON; "
			"DECLARE @allergyID INT  "
			"SET @allergyID = (SELECT TOP 1 ID FROM PatientAllergyT WHERE AllergyID = {INT} AND PersonID = {INT} AND Discontinued = 1 ORDER BY ID DESC)  "
			"IF @allergyID IS NOT NULL  "
			"BEGIN  "
			"	UPDATE PatientAllergyT SET Discontinued = 0, DiscontinuedDate = NULL , LastUpdateDate = GETDATE() WHERE ID = @allergyID " 
			"	SET NOCOUNT OFF;  "
			"    SELECT @allergyID AS ID  "
			"END "
			"ELSE "
			"BEGIN "
			"	INSERT INTO PatientAllergyT (ID, PersonID, AllergyID, EnteredDate,LastUpdateDate) "
			"						 VALUES ({INT}, {INT}, {INT}, GetDate(), GetDate())"
			"	SET NOCOUNT OFF; "
			"	SELECT {INT} AS ID "
			"END ",
			nAllergyID, m_nPatientID, nPatAllergyID, m_nPatientID, nAllergyID, nPatAllergyID);

			long nIngredientAllergyID = -1;
			if( !prs->eof ){
				nIngredientAllergyID = AdoFldLong(prs->Fields, "ID", -1);
			}

			// (c.haag 2007-10-19 11:56) - PLID 27822 - Do the audit, yo!
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiPatientAllergyAdd, m_nPatientID, "", strAllergyName, aepMedium, aetCreated);

			// (j.luckoski 2013-03-26 10:08) - PLID 53806 - Add ingredients when queue adds allergy, yo!
			if(FirstDataBank::EnsureDatabase(NULL, true) && SureScripts::IsEnabled() && nMedFromFDB == 1 && nConceptIDType == 2 && nIngredientAllergyID != -1) {
				CArray<NexTech_Accessor::_FDBIngredientImportInputPtr, NexTech_Accessor::_FDBIngredientImportInputPtr> aryIngredients;

				NexTech_Accessor::_FDBIngredientImportInputPtr ingredient(__uuidof(NexTech_Accessor::FDBIngredientImportInput));

				ingredient->PatAllergyID = nIngredientAllergyID;
				aryIngredients.Add(ingredient);


				if(aryIngredients.GetSize() > 0) {
					//	Create our SAFEARRAY to be passed to the IngredientImport function in the API
					Nx::SafeArray<IUnknown *> saryIngredients = Nx::SafeArray<IUnknown *>::From(aryIngredients);

					//	Call the API to import the ingredients and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
					NexTech_Accessor::_FDBIngredientImportResultsArrayPtr importResults = GetAPI()->IngredientImport(GetAPISubkey(), GetAPILoginToken(), saryIngredients);

					// (j.luckoski 2013-02-27 09:32) - PLID 53806 - Don't display the results as multiple medications could 
					// have the same ingredient causing the issue, but I am not deleting in case we decide to use
					// it in the future as it could be valuable.

					/*Nx::SafeArray<IUnknown *> saryIngredientResults(importResults->FDBIngredientImportResults);

					CString strFailure = "\r\nThe following ingredients are already imported:\r\n";
					foreach(NexTech_Accessor::_FDBIngredientImportOutputPtr pIngredientResult, saryIngredientResults)
					{
					if( pIngredientResult->Success == VARIANT_FALSE ){
					strFailure += CString((LPCTSTR)pIngredientResult->AllergyName) + "\r\n";
					}
					}

					CString strResults;

					if( strFailure != "\r\nThe following ingredients are already imported:\r\n" ){
					strResults += strFailure;
					}

					if( !strResults.IsEmpty() ){
					MsgBox("%s", strResults);
					}*/

				}
			}

			m_pAllergiesList->Requery();

			//(j.jones 2012-10-17 10:01) - PLID 53179 - clear the HasNoAllergies status
			if(m_checkHasNoAllergies.GetCheck()) {
				m_checkHasNoAllergies.SetCheck(FALSE);
				//this function will save, audit, and fire a tablechecker
				// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
				UpdateHasNoAllergiesStatus(FALSE, m_nPatientID);
			}

			// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
			UpdateAllergyReviewStatus(FALSE);

			// (j.jones 2013-04-05 14:22) - PLID 56114 - send a tablechecker
			m_PatientAllergiesChecker.Refresh(m_nPatientID);

			// (c.haag 2007-04-05 17:55) - PLID 25524 - Warn the user about any EMR's with "official"
			// Allergies details that are inconsistent with the patient's official allergy list
			// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
			// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
			if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
				WarnAllergyDiscrepanciesWithEMR(m_nPatientID);
			}

			CheckInteractions();

			//TES 11/13/2013 - PLID 59475 - Check CDS rules
			CDWordArray arNewCDSInterventions;
			UpdateDecisionRules(GetRemoteData(), m_nPatientID, arNewCDSInterventions);
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-21 14:42) - PLID 54704
void CPrescriptionQueueDlg::SetInteractionCount(long nInteractionCount)
{
	try
	{
		CString strText;
		if(nInteractionCount > 0)
		{
			strText.Format("Interactions (%li)", nInteractionCount);
		}
		else
		{
			strText = "Interactions";
		}

		m_btnShowInteractions.SetWindowText(strText);
	}
	NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-23 12:48) - PLID 54782
void CPrescriptionQueueDlg::SetQuickListResultsSize(BOOL bShow, long nResults, long nHeight)
{
	try{
		//Get the rect of the Write for quick list button
		CRect rectQuickListButton;
		m_btnWritePickList.GetWindowRect(&rectQuickListButton);
		ScreenToClient(&rectQuickListButton);

		//Get the rect of the queue
		// (b.savon 2013-08-23 13:33) - PLID 58236 - Use the queue rect as a size boundry
		CRect rectRxQueue;
		GetDlgItem(IDC_PRESCRIPTION_QUEUE_LIST)->GetWindowRect(&rectRxQueue);
		ScreenToClient(&rectRxQueue);
		
		//Determine the size of the search list
		long nX = rectQuickListButton.left;
		long nY = rectQuickListButton.bottom;
		// (b.savon 2013-08-23 13:32) - PLID 58236 - Calculate the width differently
		long nWidth = (rectRxQueue.Width()/2);
		// If we have more than our defined allowed results, cap it.  Otherwise, use our combined row heights
		// plus a bit of a buffer.
		long ncY = nHeight + (MAX_MED_SEARCH_ROW_HEIGHT/3);

		if( ncY + nY > rectRxQueue.bottom ){
			ncY = rectRxQueue.bottom - nY;
		}

		// (b.savon 2014-01-03 08:01) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr)
		// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
		if( !m_bShowNexFormulary ){
			HideNexFormularyColumn(m_nxdlQuickList, qlcIcon);
		}
		//Set the size
		GetDlgItem(IDC_NXDL_QUICK_LIST_POPOUT)->SetWindowPos(NULL, nX, nY, nWidth, (bShow ? ncY : 0), SWP_SHOWWINDOW);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-23 12:48) - PLID 54782
void CPrescriptionQueueDlg::SetQuickListSupervisorResultsSize(BOOL bShow, long nResults, long nHeight)
{
	try{
		//Get the rect of the Write for quick list button
		CRect rectQuickList;
		GetDlgItem(IDC_NXDL_QUICK_LIST_POPOUT)->GetWindowRect(&rectQuickList);
		ScreenToClient(&rectQuickList);
		
		//Determine the size of the search list
		long nX = rectQuickList.right;
		long nY = rectQuickList.top;
		long nWidth = rectQuickList.Width();
		// If we have more than our defined allowed results, cap it.  Otherwise, use our combined row heights
		// plus a bit of a buffer.
		long ncY = nHeight + (MAX_MED_SEARCH_ROW_HEIGHT/3);

		CRect rectQueue;
		GetDlgItem(IDC_PRESCRIPTION_QUEUE_LIST)->GetWindowRect(&rectQueue);
		ScreenToClient(&rectQueue);

		if( ncY + nY > rectQueue.bottom ){
			ncY = rectQueue.bottom - nY;
		}

		// (b.savon 2014-01-03 08:01) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr)
		// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
		if( !m_bShowNexFormulary ){
			HideNexFormularyColumn(m_nxdlQuickListSupervisor, qlcIcon);
		}
		//Set the size
		GetDlgItem(IDC_NXDL_QUICK_LIST_SUPERVISOR_POPOUT)->SetWindowPos(NULL, nX, nY, nWidth, (bShow ? ncY : 0), SWP_SHOWWINDOW);

	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionQueueDlg::LeftClickNxdlQuickListPopout(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){

			// If they clicked the <Write Selected> row
			if( VarLong(pRow->GetValue(qlcID)) == QUICK_LIST_WRITE_SELECTED_ROW ){
				NXDATALIST2Lib::IRowSettingsPtr pCycleRow = m_nxdlQuickList->GetFirstRow();
				CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> arySelectedRows;
				while( pCycleRow ){
					if( VarBool(pCycleRow->GetValue(qlcCheck), FALSE) == TRUE ){
						arySelectedRows.Add(pCycleRow);
					}
					pCycleRow = pCycleRow->GetNextRow();
				}
				// If they have any checked, Create and save them
				if( arySelectedRows.GetCount() > 0 ){
					CreateAndSavePrescriptions(arySelectedRows);
					SetQuickListResultsSize(FALSE, 0, 0);
					SetQuickListSupervisorResultsSize(FALSE, 0, 0);
					return;
				}else{
					MessageBox("There are no items selected to prescribe.  Please select items and try again.", "Write from quick list", MB_ICONINFORMATION);
					return;
				}
			}

			// If they clicked the <Configure..> row
			if( VarLong(pRow->GetValue(qlcID)) == QUICK_LIST_CONFIGURE_ROW ){
				CNexERxQuickListDlg dlg(this);
				dlg.DoModal();
				SetQuickListResultsSize(FALSE, 0, 0);
				SetQuickListSupervisorResultsSize(FALSE, 0, 0);
				return;
			}

			// If they clicked a supervisor row -> Populate and show the supervisor quick list
			if( VarLong(pRow->GetValue(qlcID)) <= QUICK_LIST_SUPERVISOR_ROW ){
				PopulateSupervisorPopout(pRow);
				return;
			}

			// We took focus from the popout (if it was), hide the list
			switch( nCol ){
				case qlcIcon: // (b.savon 2013-08-23 13:34) - PLID 58236 - rename
					{
						SetQuickListSupervisorResultsSize(FALSE, 0, 0);
						// (b.savon 2013-08-23 14:47) - PLID 58236 - Invoke the fomulary dlg
						long nFDBID = VarLong(pRow->GetValue(qlcFDBID), -1);
						if( nFDBID != -1 ){
							InvokeNexFormularyDlg(pRow, nFDBID, VarString(pRow->GetValue(qlcName)), nfsQuickList);
						}
					}
					break;
				case qlcCheck:			
				case qlcName:
				default:
					{
						SetQuickListSupervisorResultsSize(FALSE, 0, 0);
					}
					break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-24 14:37) - PLID 54782
void CPrescriptionQueueDlg::PopulateSupervisorPopout(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	m_nxdlQuickListSupervisor->Clear();
	long nHeight = 0;
	long nResults = 0;
	//Insert the Write row
	NXDATALIST2Lib::IRowSettingsPtr pInsertRow = InsertWritePrescriptionRow(m_nxdlQuickListSupervisor);
	if( pInsertRow ){
		nHeight += pInsertRow->GetHeight();
		++nResults;
	}

	//insert the supervisors quick list
	// We store the medication ptrs in an array (0 based)
	// We add supervisors to the main popout starting with id -10, and then -20 and so on.
	// To get the correct index into our array, divide their id with the supervisor row const and subtract 1
	// (i.e. I have 1 supervisor and their id is stored as -10.  I take their id (-10) / (-10 <- QUICK_LIST_SUPERVISOR_ROW) and subtract 1
	// to get my 0 based index into the array
	long nSupervisorArrayID = (VarLong(pRow->GetValue(qlcID)) / QUICK_LIST_SUPERVISOR_ROW) - 1;
	Nx::SafeArray<IUnknown *> saryMedications = m_arySupervisorMeds.GetAt(nSupervisorArrayID);
	foreach(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication, saryMedications){
		//Add each of our drugs to the list
		pInsertRow = m_nxdlQuickListSupervisor->GetNewRow();
		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added route and unit
		if( pInsertRow ){
			pInsertRow->PutValue(qlcID, atol(pMedication->QuickListID));
			// (r.gonet 2016-02-10 15:05) - PLID 58689 - Add the order index.
			pInsertRow->PutValue(qlcOrderIndex, pMedication->OrderIndex);
			pInsertRow->PutValue(qlcCheck, g_cvarFalse);
			pInsertRow->PutValue(qlcName, AsBstr(GetDisplayName(pMedication)));
			pInsertRow->PutValue(qlcDrugListID, atol(pMedication->DrugListID));
			// (b.savon 2013-08-23 14:45) - PLID 58236 - Add FDBID
			pInsertRow->PutValue(qlcFDBID, atol(pMedication->FDBID));
			pInsertRow->PutValue(qlcRefill, pMedication->Refills);
			pInsertRow->PutValue(qlcQuantity, pMedication->Quantity);
			pInsertRow->PutValue(qlcSig, pMedication->Sig);
			pInsertRow->PutValue(qlcDosageRouteID, atol(pMedication->DosageRoute->ID));
			pInsertRow->PutValue(qlcDosageRoute, pMedication->DosageRoute->Route);
			pInsertRow->PutValue(qlcDosageFrequency, pMedication->DosageFrequency);
			pInsertRow->PutValue(qlcDosageQuantity, pMedication->DosageQuantity);
			pInsertRow->PutValue(qlcDosageUnitID, atol(pMedication->DosageUnit->ID));
			pInsertRow->PutValue(qlcDosageUnit, pMedication->DosageUnit->Unit);
			pInsertRow->PutValue(qlcNotes, pMedication->NoteToPharmacist);
			// (b.savon 2013-08-23 13:43) - PLID 58236 - Add NexFormulary icon
			if( atol(pMedication->FDBID) != -1 ){
				pInsertRow->PutValue(qlcIcon, (long)m_hIconNexFormulary);
			}
			pInsertRow = m_nxdlQuickListSupervisor->AddRowSorted(pInsertRow, NULL);
			nHeight += pInsertRow->GetHeight();
			++nResults;
		}
	}

	//Show
	SetQuickListSupervisorResultsSize(TRUE, nResults, nHeight);
}

// (b.savon 2013-01-24 14:41) - PLID 54782
NXDATALIST2Lib::IRowSettingsPtr CPrescriptionQueueDlg::InsertWritePrescriptionRow(NXDATALIST2Lib::_DNxDataListPtr nxdlQuickList)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = nxdlQuickList->GetNewRow();
	if( pRow ){
		NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);
		pRow->PutFormatOverride(pHyperLink);
		pRow->PutValue(qlcID, (long)QUICK_LIST_WRITE_SELECTED_ROW);
		// (r.gonet 2016-02-10 15:04) - PLID 58689 - Write Selected is always on top. The OrderIndex is the sort column.
		pRow->PutValue(qlcOrderIndex, (long)-1);
		pRow->PutValue(qlcCheck, g_cvarNull);
		pRow->PutValue(qlcName, AsBstr("< Write Selected >"));
		pRow->PutBackColor(dlcWriteSelected);
		nxdlQuickList->AddRowSorted(pRow, NULL);
	}

	return pRow;
}

// (b.savon 2013-01-24 14:41) - PLID 54782
NXDATALIST2Lib::IRowSettingsPtr CPrescriptionQueueDlg::InsertConfigureRow(NXDATALIST2Lib::_DNxDataListPtr nxdlQuickList)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = nxdlQuickList->GetNewRow();
	if( pRow ){	
		NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);
		pRow->PutFormatOverride(pHyperLink);
		pRow->PutValue(qlcID, (long)QUICK_LIST_CONFIGURE_ROW);
		pRow->PutValue(qlcCheck, g_cvarNull);
		pRow->PutValue(qlcName, AsBstr("< Configure... >"));
		pRow->PutBackColor(dlcConfigure);
		nxdlQuickList->AddRowAtEnd(pRow, NULL);
	}
	return pRow;
}

// (b.savon 2013-01-24 14:41) - PLID 54782 - Call the API to add the quick list prescriptions
void CPrescriptionQueueDlg::CreateAndSavePrescriptions(CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> &arySelectedRows)
{
	// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
	if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
	{
		//Also check that the database exists
		if(!FirstDataBank::EnsureDatabase(this, true)) 
		{
			//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
			return;
		}
	}
	// (s.dhole 2013-10-25 17:24) - PLID 59189 Check if folrmuary information exist 
	long nInsuranceID =-1;
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if (m_bHasSureScriptsLicense){
		 nInsuranceID=  CheckExistingFormularyData();
	}

	//Pass the partial prescription to the API for it to fill in the rest
	CArray<NexTech_Accessor::_QueuePrescriptionPtr, NexTech_Accessor::_QueuePrescriptionPtr> aryPrescriptions;
	NexTech_Accessor::_UpdatePresQueueExpectedResultsPtr pExpects(__uuidof(NexTech_Accessor::UpdatePresQueueExpectedResults));

	//Set our action to add a prescription
	pExpects->Action = NexTech_Accessor::UpdatePresQueueAction_Add;
	pExpects->DrugDrugInteracts = VARIANT_TRUE;
	// (b.savon 2014-01-28 10:31) - PLID 60499 - Don't return monograph data when updating the queue and doing drug interaction checks
	pExpects->ExcludeMonographInformation = VARIANT_TRUE;
	pExpects->DrugAllergyInteracts = VARIANT_TRUE;
	pExpects->DrugDiagnosisInteracts = VARIANT_TRUE;
	pExpects->RequeryQueue = VARIANT_TRUE;
	pExpects->filters = GetQueueFilters();

	foreach(NXDATALIST2Lib::IRowSettingsPtr pRow, arySelectedRows){	

		//Populate the Prescription Objets
		// (j.fouts 2013-02-06 17:54) - PLID 51712 - Changed ID fields to be passed as strings
		NexTech_Accessor::_QueuePrescriptionPtr pPrescription(__uuidof(NexTech_Accessor::QueuePrescription));
		NexTech_Accessor::_NexERxDrugPtr pDrug(__uuidof(NexTech_Accessor::NexERxDrug));
		NexTech_Accessor::_NexERxMedicationDosagePtr pDosage(__uuidof(NexTech_Accessor::NexERxMedicationDosage));

		pPrescription->Medication = pDrug;
		pPrescription->Dosage = pDosage;
		pPrescription->Medication->DrugListID = _bstr_t(AsString(pRow->GetValue(qlcDrugListID)));
		pPrescription->RefillsAllowed = AsBstr(VarString(pRow->GetValue(qlcRefill)));
		// (j.fouts 2013-03-19 12:06) - PLID 55740 - Added renamed to QuantityValue
		pPrescription->QuantityValue = AsBstr(VarString(pRow->GetValue(qlcQuantity)));
		pPrescription->PatientExplanation = AsBstr(VarString(pRow->GetValue(qlcSig)));
		pPrescription->Dosage->DosageFrequency = AsBstr(VarString(pRow->GetValue(qlcDosageFrequency)));
		pPrescription->Dosage->DosageQuantity = AsBstr(VarString(pRow->GetValue(qlcDosageQuantity)));
		NexTech_Accessor::_EMNSpawnSourcePtr pEMRPrescriptionSource(__uuidof(NexTech_Accessor::EMNSpawnSource));
		NexTech_Accessor::_NullableIntPtr pNullableInt(__uuidof(NexTech_Accessor::NullableInt));
		pEMRPrescriptionSource->emnID = pNullableInt;
		if(m_nCurrentlyOpenedEMNID < 0){
			pEMRPrescriptionSource->emnID->SetNull();
		}else{
			pEMRPrescriptionSource->emnID->SetInt(m_nCurrentlyOpenedEMNID);
		}
		pPrescription->EMRPrescriptionSource = pEMRPrescriptionSource;

		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added route and unit
		NexTech_Accessor::_NexERxDosageRoutePtr pRoute(__uuidof(NexTech_Accessor::NexERxDosageRoute));
		CString strDosageRouteID;
		strDosageRouteID.Format("%li", VarLong(pRow->GetValue(qlcDosageRouteID)));		
		pRoute->ID = _bstr_t(strDosageRouteID);
		pPrescription->Dosage->DosageRoute = pRoute;

		NexTech_Accessor::_NexERxDosageUnitPtr pUnit(__uuidof(NexTech_Accessor::NexERxDosageUnit));
		CString strDosageUnitID;
		strDosageUnitID.Format("%li", VarLong(pRow->GetValue(qlcDosageUnitID)));
		pUnit->ID = _bstr_t(strDosageUnitID);
		pPrescription->Dosage->DosageUnit = pUnit;
		
		pPrescription->PharmacistNote = AsBstr(VarString(pRow->GetValue(qlcNotes)));
		NexTech_Accessor::_NullableBoolPtr pNullBool(__uuidof(NexTech_Accessor::NullableBool));
		pNullBool->SetBool(VARIANT_TRUE);
		pPrescription->AddWithOverrides = pNullBool;
		// (s.dhole 2013-10-28 09:30) - PLID 59189
		if (nInsuranceID>-1){
			pPrescription->InsuranceDetailID =AsBstr(AsString(nInsuranceID)); // (s.dhole 2013-10-28 09:30) - PLID 59189
		}
		aryPrescriptions.Add(pPrescription);
	}

	if( aryPrescriptions.GetCount() > 0 ){

		// (j.jones 2016-01-22 14:24) - PLID 63732 - begin a progress bar
		CProgressDialog progressDialog;
		BeginProgressDialog(progressDialog);

		Nx::SafeArray<IUnknown *> saryPrescriptions = Nx::SafeArray<IUnknown *>::From(aryPrescriptions);
		// (b.savon 2013-03-12 13:11) - PLID 55518 - Use new object structure
		pExpects->PrescriptionsToAdd = saryPrescriptions;

		// (j.fouts 2013-04-24 16:55) - PLID 52906 - Cleaned up the UpdatePrescriptionQueue PersonID Parameter
		//Call the API
		NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = GetAPI()->UpdatePrescriptionQueue(GetAPISubkey(), GetAPILoginToken(),
			_bstr_t(FormatString("%li", m_nPatientID)), pExpects);

		RefreshQueueFromArray(pResults->PrescriptionsInQueue, pResults->Pharmacies, progressDialog);

		// (j.jones 2016-01-22 14:24) - PLID 63732 - end the progress bar
		progressDialog.Stop();

		// (b.savon 2013-04-18 14:18) - PLID 54782 - Reconcile
		if( pResults->PrescriptionsAdded == NULL ){
			ThrowNxException("Unable to retrieve the prescriptions that were added!");
		}
		
		CArray<long, long> aryPrescriptionIDs;
		CStringArray staryNewCropIDs;
		Nx::SafeArray<IUnknown*> saryPrescriptionsAdded(pResults->PrescriptionsAdded);
		
		// (r.gonet 2016-01-29 09:05) - PLID 67959 - Sort the array so the prescriptions are in alphabetical order by drug name.
		std::sort(saryPrescriptionsAdded.begin(), saryPrescriptionsAdded.end(), 
			[](NexTech_Accessor::_QueuePrescriptionPtr pPrescriptionA, NexTech_Accessor::_QueuePrescriptionPtr pPrescriptionB) -> bool
		{
			CString strMedicationNameA((LPCTSTR)pPrescriptionA->GetMedication()->MedicationName);
			CString strMedicationNameB((LPCTSTR)pPrescriptionB->GetMedication()->MedicationName);
			// The compare needs to be strict weak ordering.
			return strMedicationNameA.CompareNoCase(strMedicationNameB) < 0;
		});

		// (s.tullis 2016-01-20 15:39) - PLID 67959 - As a user, when I write multiple prescriptions from my quicklist, I need them to pop-up for review/changes.
		foreach(NexTech_Accessor::_QueuePrescriptionPtr pPrescription, saryPrescriptionsAdded)
		{
			PrescriptionInfo rxInformation;
			rxInformation.pPrescription = pPrescription;
			rxInformation.erxUserRole = pResults->UserRole;
			rxInformation.saryPrescribers = pResults->Prescriber;
			rxInformation.sarySupervisors = pResults->Supervisor;
			rxInformation.saryNurseStaff = pResults->NurseStaff;

			CPrescriptionEditDlg dlg(this, m_nCurrentlyOpenedEMNID);

			// (j.jones 2013-11-25 09:55) - PLID 59772 - for new prescriptions we pass in the drug interactions info
			DrugInteractionInfo drugInteractionInfo;
			if(pResults->DrugDrugInteracts) {
				drugInteractionInfo.saryDrugDrugInteracts = pResults->DrugDrugInteracts;
			}
			if(pResults->DrugAllergyInteracts) {
				drugInteractionInfo.saryDrugAllergyInteracts = pResults->DrugAllergyInteracts;
			}
			if(pResults->DrugDiagnosisInteracts) {
				drugInteractionInfo.saryDrugDiagnosisInteracts = pResults->DrugDiagnosisInteracts;
			}

			// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
			PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(true, rxInformation, &drugInteractionInfo);

			// (j.fouts 2013-03-12 15:07) - PLID 52973 - The EditDlg is no longer responsible for creating new prescriptions, so
			// this should only return Edit or Delete
			// (s.dhole 2013-05-20 10:30) - PLID 56734 this may return epdrvErrorRx if there is sure script error
			if(epdrvReturn == epdrvEditRx || epdrvReturn == epdrvErrorRx) {
				// (c.haag 2010-02-17 10:19) - PLID 37384 - Let the user apply the prescriptions to the current medications list.
				// (j.jones 2010-08-23 09:23) - PLID 40178 - this is user-created so the NewCropGUID is empty
				// (j.jones 2011-05-02 15:39) - PLID 43450 - pass in the patient explanation as the sig
				// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also no longer needs the Sig nor NewCropGUID
				//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
				aryPrescriptionIDs.Add(atol(CString((LPCTSTR)pPrescription->PrescriptionID)));			
			}
		}

		// (s.tullis 2016-01-21 10:02) - PLID 67959 - If we edited the meds or there was an error we need to reconcile them
		if (aryPrescriptionIDs.GetSize() > 0)
		{
			CDWordArray arNewCDSInterventions;
			ReconcileCurrentMedicationsWithMultipleNewPrescriptions(m_nPatientID, aryPrescriptionIDs, staryNewCropIDs, m_queueBkg.GetColor(), GetParent(), arNewCDSInterventions);
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}

		RefreshInteractionsFromArray(
			pResults->DrugDrugInteracts, 
			pResults->DrugAllergyInteracts,
			pResults->DrugDiagnosisInteracts
			);		
	}
}

// (b.savon 2013-01-24 14:42) - PLID 54782
void CPrescriptionQueueDlg::FocusLostNxdlQuickListPopout()
{
	try{
		SetTimer(IDT_HIDE_QUICK_LIST_RESULTS, HIDE_SEARCH_RESULTS_TIMER, NULL);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-24 14:42) - PLID 54782
void CPrescriptionQueueDlg::FocusGainedNxdlQuickListPopout()
{
	try{
		KillTimer(IDT_HIDE_QUICK_LIST_RESULTS);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-24 14:42) - PLID 54782
void CPrescriptionQueueDlg::FocusLostNxdlQuickListSupervisorPopout()
{
	try{
		SetTimer(IDT_HIDE_QUICK_LIST_SUPERVISOR_RESULTS, HIDE_SEARCH_RESULTS_TIMER, NULL);
		SetTimer(IDT_HIDE_QUICK_LIST_RESULTS, HIDE_SEARCH_RESULTS_TIMER, NULL);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-24 14:42) - PLID 54782
void CPrescriptionQueueDlg::FocusGainedNxdlQuickListSupervisorPopout()
{
	try{
		KillTimer(IDT_HIDE_QUICK_LIST_RESULTS);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-24 14:42) - PLID 54782 - Construct our datalist name to show to the user
// (j.fouts 2013-02-01 15:06) - PLID 54985 - Updated the display name with the new fields
CString CPrescriptionQueueDlg::GetDisplayName(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication)
{
	// (j.fouts 2013-02-05 14:39) - PLID 54463 - Moved this into a utility function
	// (j.fouts 2013-04-22 10:26) - PLID 56155 - This now just uses the defined sig
	return GenerateDrugSigDisplayName(
		CString((LPCTSTR)pMedication->MedicationName),
		CString((LPCTSTR)pMedication->Sig));
}

// (b.savon 2013-01-25 09:57) - PLID 54846
void CPrescriptionQueueDlg::SelChosenNxdlQueuePrescriberFilter(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			// (r.gonet 2016-01-22 16:02) - PLID 67967 - Have the controller handle the selection, possibly showing the CMultiSelectDlg.
			if (m_rxPrescriberController.HandleSelection()) {
				RequeryQueue(false);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-25 14:51) - PLID 54854
void CPrescriptionQueueDlg::RButtonUpCurrentMedsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow && nCol > -1)
		{	
			// (b.savon 2013-01-14 12:30) - PLID 54592
			BOOL bPermission = CheckCurrentUserPermissions(bioPatientCurrentMeds, sptDelete);
			CMenu pMenu;
			pMenu.CreatePopupMenu();
			int nMenuIndex = 0;

			if(bPermission){
				m_pMedsList->CurSel = pRow;
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DELETE_CURRENT_MED, "Delete");
				// (b.savon 2013-01-25 12:44) - PLID 54854
				if( !VarBool(pRow->GetValue(cmlcDiscontinued), FALSE) ){
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DISCONTINUE_CURRENT_MED, "Discontinue");
				}
			}

			// (b.savon 2013-06-19 16:53) - PLID 56880 - Only add the menu options if they have NexERx
			// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
			if(m_bHasSureScriptsLicense) {
				long nFDBMedID = VarLong(pRow->GetValue(cmlcFDBID), -1);

				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, MF_SEPARATOR);

				if(nFDBMedID >= 0)
				{
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_SHOW_MONO, "View Monograph");	
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_SHOW_LEAFLET, "View Leaflet");
				}
				else
				{
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_MONO, "No Monograph Available");
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_LEAFLET, "No Leaflet Available");
				}
			}

			// (b.savon 2013-01-14 17:40) - PLID 54592 - Pop it if we have 1 of the 2
			if (bPermission || m_bHasSureScriptsLicense) {
				CPoint pt;
				GetCursorPos(&pt);
				pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-25 14:50) - PLID 54854
BOOL CPrescriptionQueueDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	
	switch ((wParam)) {
		case IDM_DELETE_CURRENT_MED:
			try {

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedsList->GetCurSel();

				if (pRow == NULL) {

					MsgBox("Please choose a medication to delete");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				//give them a messagebox to make sure they are sure they want to do this
				if (IDYES == MsgBox(MB_YESNO, "This action is unrecoverable, are you sure you want to delete this medication?")) {

					//let's do it!
					long nID = VarLong(pRow->GetValue(0));

					ExecuteParamSql("DELETE FROM CurrentPatientMedsT WHERE ID = {INT}", nID);

					// (c.haag 2007-10-19 17:13) - PLID 27827 - Audit
					long nAuditID = BeginNewAuditEvent();
					long nEMRDataID = VarLong(pRow->GetValue(3));
					CString strName = VarString(pRow->GetValue(2), "");
					//strName.TrimRight(" - {Inactive}");
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiCurrentPatientMedsDeleted, nEMRDataID, strName, "<Deleted>", aepHigh, aetDeleted);

					m_pMedsList->RemoveRow(pRow);

					// (j.jones 2013-04-05 14:22) - PLID 56114 - send a tablechecker
					m_CurrentMedsChecker.Refresh(m_nPatientID);

					// (c.haag 2007-01-25 12:42) - PLID 24420 - Warn the user about any EMR's with "official"
					// Current Medications details that are inconsistent with the patient's official medication list
					// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
					// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
					if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
						WarnMedicationDiscrepanciesWithEMR(m_nPatientID);
					}
					// (c.haag 2010-09-21 13:43) - PLID 40610 - Do not create todo alarms for decisions if we're only deleting a med
					//TodoCreateForDecisionRules(GetRemoteData(), m_nPatientID);
				}

				// (j.fouts 2012-08-10 09:46) - PLID 52089 - Meds Changed Check Interactions
				// (j.fouts 2013-01-07 10:44) - PLID 54468 - Check interactions rather than requering the whole queue
				CheckInteractions();
			}NxCatchAll("Error Removing Current Medication!");
			break;
		// (b.savon 2013-01-25 12:41) - PLID 54854 - Discontinue Med
		case IDM_DISCONTINUE_CURRENT_MED:
			try {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedsList->GetCurSel();

				if (pRow == NULL) {

					MsgBox("Please choose a medication to discontinue.");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				if(MsgBox(MB_YESNO, "Are you sure you wish to discontinue this medication?") == IDNO) {
					return CNxDialog::OnCommand(wParam, lParam);
				}
				// (s.dhole 2013-11-19 09:57) - PLID	56926	
				UpdateCurrentMedRecord(pRow,cmlcDiscontinued,_variant_t(1L));
				//let's do it!
				/*long nID = VarLong(pRow->GetValue(0));

				ExecuteParamSql("UPDATE CurrentPatientMedsT SET Discontinued = 1, DiscontinuedDate = GETDATE() WHERE ID = {INT}", nID);*/
				pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);
				pRow->PutValue(cmlcDiscontinued, _variant_t(VARIANT_TRUE, VT_BOOL));

				// (c.haag 2007-10-19 17:13) - PLID 27827 - Audit
				long nAuditID = BeginNewAuditEvent();
				long nEMRDataID = VarLong(pRow->GetValue(3));
				CString strName = VarString(pRow->GetValue(2), "");
				//strName.TrimRight(" - {Inactive}");
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiCurrentMedicationInactivated, nEMRDataID, strName, "<Discontinued>", aepHigh, aetChanged);

				// (c.haag 2007-01-25 12:42) - PLID 24420 - Warn the user about any EMR's with "official"
				// Current Medications details that are inconsistent with the patient's official medication list
				// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
				// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
				if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
					WarnMedicationDiscrepanciesWithEMR(m_nPatientID);
				}

				// (j.jones 2013-04-05 14:22) - PLID 56114 - send a tablechecker
				m_CurrentMedsChecker.Refresh(m_nPatientID);

				// (b.savon 2013-03-25 15:54) - PLID 54854 - Check interactions
				CheckInteractions();
			}NxCatchAll(__FUNCTION__);
			break;
		case IDM_DELETE_CURRENT_ALLERGY:
			try {
				//if they selected something, delete it, otherwise tell them to select something
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAllergiesList->GetCurSel();
				
				if (pRow == NULL) {
					//they didn't select anything
					MsgBox("Please select an allergy to delete");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				if(AfxMessageBox("Are you sure you wish to delete this allergy?", MB_YESNO) == IDNO) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				// (z.manning 2010-01-06 09:42) - PLID 35766 - Use the alcName enum here instead of hard-coded column value
				CString strOld = CString(pRow->GetValue(alcName).bstrVal);

				// (j.gruber 2009-06-08 15:40) - PLID 34395 - trim the {Inactive} off
				// (j.jones 2012-04-04 11:17) - PLID 49411 - We should not have used TrimRight,
				// it would trim off additional letters in the word Inactive. However, we actually
				// want to keep the word Inactive in our audit, so we know that an inactive allergy
				// was deleted.
				//strOld.TrimRight(" - {Inactive}");

				// (j.luckoski 2013-03-26 10:37) - PLID 53806 - Delete from AllergyIngredientT
				long nDelID = VarLong(pRow->GetValue(alcID));
				ExecuteParamSql("DELETE FROM AllergyIngredientT WHERE PatientAllergyID = {INT}", nDelID);
				ExecuteParamSql("DELETE FROM PatientAllergyT WHERE ID = {INT}", nDelID);
				//update the datalist
				m_pAllergiesList->Requery();

				// (c.haag 2007-04-05 17:54) - PLID 25524 - Warn the user about any EMR's with "official"
				// Allergies details that are inconsistent with the patient's official allergy list
				// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
				// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
				if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
					WarnAllergyDiscrepanciesWithEMR(m_nPatientID);
				}

				//audit
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientAllergyDelete, GetActivePatientID(), strOld, "<Deleted>", aepMedium, aetDeleted);

				// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
				//UpdateAllergyReviewStatus(FALSE);

				// (j.jones 2013-04-05 14:22) - PLID 56114 - send a tablechecker
				m_PatientAllergiesChecker.Refresh(m_nPatientID);

				//EnableAppropriateButtons();

				// (j.fouts 2012-08-10 09:46) - PLID 52089 - Allergies Changed Check Interactions	
				// (j.fouts 2013-01-07 10:44) - PLID 54468 - Don't requery the whole queue, just check interactions
				CheckInteractions();
			}NxCatchAll("Error in OnClickDeleteAllergy");
			break;
		// (b.savon 2013-01-25 12:41) - PLID 54854 - Discontinue Allergy
		case IDM_DISCONTINUE_CURRENT_ALLERGY:
			try{
				//if they selected something, delete it, otherwise tell them to select something
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAllergiesList->GetCurSel();
				
				if (pRow == NULL) {
					//they didn't select anything
					MsgBox("Please select an allergy to discontinue.");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				if(MsgBox(MB_YESNO, "Are you sure you wish to discontinue this allergy?") == IDNO) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				// (z.manning 2010-01-06 09:42) - PLID 35766 - Use the alcName enum here instead of hard-coded column value
				CString strOld = CString(pRow->GetValue(alcName).bstrVal);

				// (j.gruber 2009-06-08 15:40) - PLID 34395 - trim the {Inactive} off
				// (j.jones 2012-04-04 11:17) - PLID 49411 - We should not have used TrimRight,
				// it would trim off additional letters in the word Inactive. However, we actually
				// want to keep the word Inactive in our audit, so we know that an inactive allergy
				// was deleted.
				//strOld.TrimRight(" - {Inactive}");

				// (j.luckoski 2013-03-26 10:37) - PLID 53806 - Delete from AllergyIngredientT
				long nDelID = VarLong(pRow->GetValue(alcID));
				ExecuteParamSql("DELETE FROM AllergyIngredientT where PatientAllergyID = {INT}", nDelID);
				// (s.dhole 2013-07-05 16:04) - PLID 56931 Added updatedate
				ExecuteParamSql("UPDATE PatientAllergyT SET Discontinued = 1, DiscontinuedDate = GETDATE(), LastUpdateDate = GETDATE() WHERE ID = {INT}", nDelID);
				//update the datalist
				m_pAllergiesList->Requery();

				// (c.haag 2007-04-05 17:54) - PLID 25524 - Warn the user about any EMR's with "official"
				// Allergies details that are inconsistent with the patient's official allergy list
				// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
				// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
				if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
					WarnAllergyDiscrepanciesWithEMR(m_nPatientID);
				}

				//audit
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientInactivateAllergy, GetActivePatientID(), strOld, "<Discontinued>", aepHigh, aetChanged);

				// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
				//UpdateAllergyReviewStatus(FALSE);

				// (j.jones 2013-04-05 14:22) - PLID 56114 - send a tablechecker
				m_PatientAllergiesChecker.Refresh(m_nPatientID);

				//EnableAppropriateButtons();				
				// (b.savon 2013-03-25 15:54) - PLID 54854 - Check interactions
				CheckInteractions();
			}NxCatchAll(__FUNCTION__);
			break;
		// (b.savon 2013-06-19 17:07) - PLID 56880 - Show monograph
		case IDM_SHOW_MONO:
			try
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedsList->GetCurSel();
				// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
				if(pRow)
				{
					//Get the MedId for the selected drug
					long nFDBMedID = VarLong(pRow->GetValue(cmlcFDBID), -1);

					// (j.fouts 2012-08-20 09:26) - PLID 51719 - Don't give an option for monograph to non FDB drugs
					if(nFDBMedID >= 0)
					{
						// (j.fouts 2012-09-25 09:27) - PLID 52825 - Check that the database exists
						if(FirstDataBank::EnsureDatabase(this, true))
						{
							ShowMonograph(nFDBMedID, this);
						}
					}
				}
			}
			NxCatchAll(__FUNCTION__);
			break;
		// (b.savon 2013-06-19 17:29) - PLID 56880 - Show Leaflet
		case IDM_SHOW_LEAFLET: 
			try
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedsList->GetCurSel();
				// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
				if(pRow)
				{
					long nFDBMedID = VarLong(pRow->GetValue(cmlcFDBID), -1);

					if(nFDBMedID >= 0)
					{
						// (j.fouts 2013-06-10 11:18) - PLID 56808 - If the database does not exist we cannot query it
						if(FirstDataBank::EnsureDatabase(this, true))
						{
							ShowLeaflet(nFDBMedID, this);
						}
					}
				}
			}
			NxCatchAll(__FUNCTION__);
			break;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

// (b.savon 2013-01-25 14:50) - PLID 54854
void CPrescriptionQueueDlg::RButtonUpAllergiesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// if its a valid row and column
		if(pRow && nCol > -1)
		{	
			// (b.savon 2013-01-14 12:30) - PLID 54704
			if(CheckCurrentUserPermissions(bioPatientAllergies, sptDelete)){
				m_pAllergiesList->CurSel = pRow;
				CMenu pMenu;
				pMenu.CreatePopupMenu();
				int nMenuIndex = 0;	
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DELETE_CURRENT_ALLERGY, "Delete");
				// (b.savon 2013-01-25 12:43) - PLID 54854
				if( !VarBool(pRow->GetValue(alcIsDiscontinued), FALSE) ){
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DISCONTINUE_CURRENT_ALLERGY, "Discontinue");
				}
				CPoint pt;
				GetCursorPos(&pt);
				pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added no meds check box
void CPrescriptionQueueDlg::OnBnClickedNoMedsCheckQueue()
{
	try {

		BOOL bHasNoMeds = m_checkHasNoMeds.GetCheck();

		//since this is just as important as adding a medication, we use the same permission
		if(!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptCreate)) {
			//reverse their selection
			m_checkHasNoMeds.SetCheck(!bHasNoMeds);
			return;
		}

		if(bHasNoMeds) {
			//if they have active current meds. selected, disallow checking this box
			//(check in data for accuracy)
			if(ReturnsRecordsParam("SELECT TOP 1 ID FROM CurrentPatientMedsT WHERE Discontinued = 0 AND PatientID = {INT}", m_nPatientID)) {
				m_checkHasNoMeds.SetCheck(FALSE);
				MessageBox("The 'patient has no medications' status cannot be selected while the patient has active current medications in their list.", "Practice", MB_OK|MB_ICONINFORMATION);
				return;
			}
		}

		//this function will save, audit, and fire a tablechecker
		UpdateHasNoMedsStatus(bHasNoMeds, m_nPatientID);
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added no allergies check box
void CPrescriptionQueueDlg::OnBnClickedNoAllergyQueue()
{
	try {

		BOOL bHasNoAllergies = m_checkHasNoAllergies.GetCheck();

		//since this is just as important as adding an allergy, use the same permission
		if(!CheckCurrentUserPermissions(bioPatientAllergies, sptCreate)) {
			//reverse their selection
			m_checkHasNoAllergies.SetCheck(!bHasNoAllergies);
			return;
		}

		if(bHasNoAllergies) {
			//if they have active allergies selected, disallow checking this box
			//(check in data for accuracy)
			if(ReturnsRecordsParam("SELECT TOP 1 ID FROM PatientAllergyT WHERE Discontinued = 0 AND PersonID = {INT}", m_nPatientID)) {
				m_checkHasNoAllergies.SetCheck(FALSE);
				MessageBox("The 'patient has no known allergies' status cannot be selected while the patient has active allergies in their list.", "Practice", MB_OK|MB_ICONINFORMATION);
				return;
			}
		}

		//this function will save, audit, and fire a tablechecker
		UpdateHasNoAllergiesStatus(bHasNoAllergies, m_nPatientID);
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Added allergies reviewed chack box
void CPrescriptionQueueDlg::OnBnClickedAllergiesReviewedQueue()
{
	try {

		BOOL bReviewedAllergies = m_checkReviewedAllergies.GetCheck();

		if(!CheckCurrentUserPermissions(bioPatientAllergyReview, sptWrite)) {
			//reverse the checkbox
			m_checkReviewedAllergies.SetCheck(!bReviewedAllergies);
			return;
		}

		//UpdateAllergyReviewStatus will handle the data change, interface change, and auditing
		UpdateAllergyReviewStatus(!!bReviewedAllergies);
		
	}NxCatchAll("Error in CMedicationDlg::OnCheckReviewedAllergies");
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Save/audit allergies reviewed and udpate the controls
void CPrescriptionQueueDlg::UpdateAllergyReviewStatus(bool bReviewedAllergies)
{
	//UpdatePatientAllergyReviewStatus will save the status change and audit
	COleDateTime dtReviewedOn = UpdatePatientAllergyReviewStatus(m_nPatientID, bReviewedAllergies);

	UpdateAllergyReviewCtrls(bReviewedAllergies, dtReviewedOn, CString(GetCurrentUserName()));
}

// (j.fouts 2013-01-25 13:37) - PLID 53574 - Update the allergies reviewed controls
void CPrescriptionQueueDlg::UpdateAllergyReviewCtrls(bool bReviewedAllergies, COleDateTime& dtReviewedOn, CString& strUserName)
{
	CString strLabel;

	if(bReviewedAllergies) {
		strLabel.Format("Allergy information has been reviewed by %s on %s.", strUserName, FormatDateTimeForInterface(dtReviewedOn, DTF_STRIP_SECONDS, dtoDateTime));
	}
	else {
		strLabel = "Have the allergies been reviewed with the patient?";
	}

	m_checkReviewedAllergies.SetWindowText(strLabel);
	m_checkReviewedAllergies.SetCheck(bReviewedAllergies);
}

// (j.fouts 2013-01-28 16:34) - PLID 53025 - Create a function to set the status and update the queue with the results
void CPrescriptionQueueDlg::SetPrescriptionStatus(CArray<CString,CString&> &aryPrescriptionIDs, NexTech_Accessor::PrescriptionStatus psStatus)
{
	// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
	if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
	{
		//Also check that the database exists
		if(!FirstDataBank::EnsureDatabase(this, true)) 
		{
			//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
			return;
		}
	}

	Nx::SafeArray<BSTR> saryPrescriptionIDs = Nx::SafeArray<BSTR>::From(aryPrescriptionIDs);

	NexTech_Accessor::_UpdatePrescriptionStatusOutputPtr pOutput = GetAPI()->UpdatePrescriptionStatus(
		GetAPISubkey(), GetAPILoginToken(), saryPrescriptionIDs, psStatus);

	Nx::SafeArray<IUnknown *> saryResults(pOutput->Results);
	//For every changed status we will try to find it in our data list and update it
	foreach(NexTech_Accessor::_UpdatePrescriptionStatusResultPtr pResult, saryResults)
	{
		long nID = atol(pResult->PrescriptionID);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPresQueueList->FindByColumn(pqcPrescriptionID, _variant_t(nID), NULL, FALSE);
		if(pRow)
		{
			//Check if it no longer matches our filters
			if(m_pQueueStatusFilter->CurSel)
			{
				long nFilterStatus = VarLong(m_pQueueStatusFilter->CurSel->GetValue(qsfcID));

				if(nFilterStatus > 0)
				{
					NexTech_Accessor::PrescriptionStatus psFilterStatus = MapQueueStatusToAccessor(nFilterStatus);

					if(pResult->NewPrescriptionQueueStatus != psFilterStatus)
					{
						m_pPresQueueList->RemoveRow(pRow);
						continue;
					}
				}
			}
			bool bAllowSending = true;
			switch(pResult->NewPrescriptionQueueStatus)
				{
					case NexTech_Accessor::PrescriptionStatus_Printed:
						pRow->CellFormatOverride[pqcPharmacy]->Editable = VARIANT_FALSE;
						break;
					case NexTech_Accessor::PrescriptionStatus_eTransmitAuthorized:
					case NexTech_Accessor::PrescriptionStatus_eTransmitPending:
					case NexTech_Accessor::PrescriptionStatus_eTransmitSuccess:
					case NexTech_Accessor::PrescriptionStatus_eFaxed:
						pRow->CellFormatOverride[pqcPharmacy]->Editable = VARIANT_FALSE;
					case NexTech_Accessor::PrescriptionStatus_Legacy:
						bAllowSending = false;
						break;
					// (b.savon 2013-09-04 16:36) - PLID 58212 - Add a new 'Void' type for Prescriptions
					case NexTech_Accessor::PrescriptionStatus_Void:
						{
							pRow->CellFormatOverride[pqcPharmacy]->Editable = VARIANT_FALSE;
							bAllowSending = false;
							pRow->PutValue(pqcPrint, g_cvarNull);
							pRow->PutCellBackColor(pqcStatus, ERX_NO_RESULTS_COLOR);
						}
						break;
					// (b.eyers 2016-02-05) - PLID 67980 - added a new 'Dispensed In-House' status for prescriptions
					case NexTech_Accessor::PrescriptionStatus_DispensedInHouse:
						pRow->CellFormatOverride[pqcPharmacy]->Editable = VARIANT_FALSE;
						pRow->PutValue(pqcPrint, g_cvarNull);
						bAllowSending = false;
						break;
				}
		
			pRow->PutValue(pqcStatusID, pResult->NewPrescriptionQueueStatus);
			pRow->PutValue(pqcStatus, _bstr_t(QueueStatusTextFromID(pResult->NewPrescriptionQueueStatus)));

			// (s.dhole 2013-03-19 15:22) - PLID 55753 added validation and refresh to selected record status
			// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
			if (m_bHasSureScriptsLicense)
				{
					bAllowSending =((pResult->ValidationList->ErrorStatus==elNotValidForSureScript)?false:bAllowSending );
					if (pResult->ValidationList->ErrorStatus==elNotValidForSureScript){
						// (s.dhole 2013-05-01 13:01) - PLID 54067 We allow printed prescription to validate chages
						pRow->PutValue(pqcMedErrorInd, _variant_t((pResult->ValidationList->ErrorStatus==elValidForSureScript  
								|| (pResult->NewPrescriptionQueueStatus ==NexTech_Accessor::PrescriptionStatus_Legacy ) 
								)?g_cvarNull:(long)m_hIconHasSureScriptError));
						pRow->PutValue(pqcMedErrorIndVal, _variant_t( pResult->ValidationList->ErrorStatus));
						pRow->CellForeColor[pqcMedErrorInd] = RGB(255,0,0);
					}
					else{
						pRow->PutValue(pqcMedErrorInd, g_cvarNull);
						pRow->PutValue(pqcMedErrorIndVal, _variant_t(0));
					}
					pRow->PutValue(pqcErrorDescription, _bstr_t(SureScripts::LoadSureScriptErrorDesc(pResult->ValidationList)));
					pRow->PutValue(pqcSend, (bAllowSending)? g_cvarFalse:g_cvarNull);
			} 
		}
		else
		{
			if(m_pQueueStatusFilter->CurSel)
			{
				long nFilterStatus = VarLong(m_pQueueStatusFilter->CurSel->GetValue(qsfcID));

				NexTech_Accessor::PrescriptionStatus psFilterStatus = MapQueueStatusToAccessor(nFilterStatus);

				if(pResult->NewPrescriptionQueueStatus == psFilterStatus || 
					psFilterStatus == NexTech_Accessor::PrescriptionStatus_NoStatus)
				{
					//There is the possiblity that a prescription had its status set to something that would place it into our current
					//filter when it was not there previously. So we would not find it in the datalist, so we would need to insert it into 
					//the datalist. Unfortuantely that would require UpdatePrescriptionStatusResult to return the full prescription, which 
					//it does not currently do. However as of right now there is no way to change the status of a prescription that is not 
					//currently in the datalist. So this is currently unimplemented. If you hit this you or someone you know has added a 
					//way to change the status of soemthing not in the data list! Please update the code here.
					ASSERT(FALSE);
					continue;
				}
			}
		}
	}
}

// (b.savon 2013-01-30 09:52) - PLID 54922
//(s.dhole 3/10/2015 11:37 AM ) - PLID 64561 change column emn 
VARIANT_BOOL CPrescriptionQueueDlg::ImportMedication(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewDrugListID)
{
	// (r.farnworth 2013-08-14 10:51) - PLID 58001 - Moved a lot of the function to PrescriptionUtilsAPI
	if( pRow != NULL ){
		long nFDBID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);
		CString strMedname = VarString(pRow->GetValue(mrcMedicationName), "");
		return ::ImportMedication(nFDBID, strMedname, nNewDrugListID);
	}
	return VARIANT_FALSE;
}

// (b.savon 2013-01-30 10:56) - PLID 54927
VARIANT_BOOL CPrescriptionQueueDlg::ImportAllergy(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewAllergyID)
{
	VARIANT_BOOL vbSuccess = VARIANT_FALSE;
	try{
		// (j.fouts 2013-03-19 3:10) - PLID 53840 - Ensure database before we do trying to use FDB
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(this, true)) 
			{
				//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
				return VARIANT_FALSE;
			}
		}

		CArray<NexTech_Accessor::_FDBAllergyImportInputPtr, NexTech_Accessor::_FDBAllergyImportInputPtr> aryAllergies;
		NexTech_Accessor::_FDBAllergyImportInputPtr allergy(__uuidof(NexTech_Accessor::FDBAllergyImportInput));
		
		allergy->AllergyName = _bstr_t(pRow->GetValue(aslcAllergyName));
		allergy->ConceptID = VarLong(pRow->GetValue(aslcConceptID));
		allergy->ConceptTypeID = VarLong(pRow->GetValue(aslcConceptTypeID));
		
		aryAllergies.Add(allergy);

		//	Create our SAFEARRAY to be passed to the AllergyImport function in the API
		Nx::SafeArray<IUnknown *> saryAllergies = Nx::SafeArray<IUnknown *>::From(aryAllergies);

		CWaitCursor cwait;

		//	Call the API to import the allergies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		NexTech_Accessor::_FDBAllergyImportResultsArrayPtr importResults = GetAPI()->AllergyImport(GetAPISubkey(), GetAPILoginToken(), saryAllergies);

		//	If for some reason we get nothing back (although, this should never happen), tell the user and bail.
		if( importResults->FDBAllergyImportResults == NULL ){
			//MsgBox("There were no allergies to import.");
			return VARIANT_FALSE;
		}

		Nx::SafeArray<IUnknown *> saryAllergyResults(importResults->FDBAllergyImportResults);

		//	Prepare our results message so the user knows which imported succesfully (or not)
		foreach(NexTech_Accessor::_FDBAllergyImportOutputPtr pAllergyResult, saryAllergyResults)
		{
			nNewAllergyID = pAllergyResult->AllergyID;
			vbSuccess = pAllergyResult->Success;
		}

		return vbSuccess;
	}NxCatchAll(__FUNCTION__);
	return vbSuccess;
}

// (b.savon 2013-01-31 13:12) - PLID 54782
void CPrescriptionQueueDlg::LeftClickNxdlQuickListSupervisorPopout(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){

			// If they clicked the <Write Selected> row
			if( VarLong(pRow->GetValue(qlcID)) == QUICK_LIST_WRITE_SELECTED_ROW ){
				NXDATALIST2Lib::IRowSettingsPtr pCycleRow = m_nxdlQuickListSupervisor->GetFirstRow();
				CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> arySelectedRows;
				while( pCycleRow ){
					if( VarBool(pCycleRow->GetValue(qlcCheck), FALSE) == TRUE ){
						arySelectedRows.Add(pCycleRow);
					}
					pCycleRow = pCycleRow->GetNextRow();
				}
				// If they have any checked, Create and save them
				if( arySelectedRows.GetCount() > 0 ){
					CreateAndSavePrescriptions(arySelectedRows);
					SetQuickListResultsSize(FALSE, 0, 0);
					SetQuickListSupervisorResultsSize(FALSE, 0, 0);
					return;
				}else{
					MessageBox("There are no items selected to prescribe.  Please select items and try again.", "Write from quick list", MB_ICONINFORMATION);
					return;
				}
			}

			switch( nCol ){
				case qlcIcon: // (b.savon 2013-08-23 13:34) - PLID 58236 - rename
					{		
						// (b.savon 2013-08-23 14:47) - PLID 58236 - Invoke the fomulary dlg
						long nFDBID = VarLong(pRow->GetValue(qlcFDBID), -1);
						if( nFDBID != -1 ){
							SetQuickListSupervisorResultsSize(FALSE, 0, 0);
							InvokeNexFormularyDlg(pRow, nFDBID, VarString(pRow->GetValue(qlcName)), nfsQuickList);
						}
					}
					break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-02-04 10:01) - PLID 53954 - Moved this into a utility function
Nx::SafeArray<IUnknown *> CPrescriptionQueueDlg::GetQueueFilters()
{
	// (j.fouts 2012-10-03 16:39) - PLID 53009 - Create a filter and pass it to the API
	// (j.fouts 2012-10-22 11:37) - PLID 53156 - Filter on patient aswell
	CArray<NexTech_Accessor::_ERxPrescriptionFilterPtr, NexTech_Accessor::_ERxPrescriptionFilterPtr> aryFilters;
	NexTech_Accessor::_ERxPrescriptionFilterPtr pFilter(__uuidof(NexTech_Accessor::ERxPrescriptionFilter));

	// (b.eyers 2016-01-25) - PLID 67983 - moved the below here
	long nStatusFilter = m_pQueueStatusFilter->CurSel->GetValue(qsfcID);

	if(m_pQueueStatusFilter->CurSel)
	{
		//long nStatusFilter = m_pQueueStatusFilter->CurSel->GetValue(qsfcID); 
		pFilter->status = MapQueueStatusToAccessor(nStatusFilter);
	}

	// (b.eyers 2016-01-25) - PLID 67983 - if the 'needs action' statues option is selected, set this as true
	NexTech_Accessor::_NullableBoolPtr pNeedsAction(__uuidof(NexTech_Accessor::NullableBool));
	if (nStatusFilter == -2) {
		pNeedsAction->SetBool(VARIANT_TRUE);
		pFilter->statusNeedingAction = pNeedsAction;
	}
	else {
		pNeedsAction->SetNull();
		pFilter->statusNeedingAction = pNeedsAction;
	}

	// (b.savon 2013-01-25 13:07) - PLID 54846 - Prescriber filter
	// (j.fouts 2013-03-20 16:34) - PLID 51712 - Changed to strings
	// (j.fouts 2013-08-27 09:02) - PLID 56976 - ProviderID changed to providerID
	// (r.gonet 2016-01-22) - PLID 67967 - Get the selected IDs from the controller rather than directly from the datalist
	// since there may be multiple prescribers selected.
	const CVariantArray& arySelectedIDs = m_rxPrescriberController.GetSelectedIDs();
	Nx::SafeArray<BSTR> saPrescriberIDs;
	for (int i = 0; i < arySelectedIDs.GetSize(); i++) {
		saPrescriberIDs.Add(AsString(arySelectedIDs[i]), true);
	}
	pFilter->providerIDs = saPrescriberIDs;

	if(m_nPatientID < 0)
	{
		pFilter->patientID = _bstr_t("");
	}
	else
	{
		CString strPatientID;
		strPatientID.Format("%li", m_nPatientID);
		pFilter->patientID = _bstr_t(strPatientID);
	}

	// (b.eyers 2016-01-21) - PLID 67966 - send the date filter to the API, null if there isn't one
	NexTech_Accessor::_NullableDateTimePtr pNullableToDate(__uuidof(NexTech_Accessor::NullableDateTime));
	NexTech_Accessor::_NullableDateTimePtr pNullableFromDate(__uuidof(NexTech_Accessor::NullableDateTime));
	
	if (m_nPatientID < 0 && IsDlgButtonChecked(IDC_USE_PRESCRIPTION_DATE)) {

		pNullableToDate->SetDateTime(m_DateTo.GetValue());
		pNullableFromDate->SetDateTime(m_DateFrom.GetValue());

		pFilter->toDate = pNullableToDate;
		pFilter->fromDate = pNullableFromDate;

	}
	else {
		pNullableToDate->SetNull();
		pNullableFromDate->SetNull();

		pFilter->toDate = pNullableToDate;
		pFilter->fromDate = pNullableFromDate;
	}
	

	aryFilters.Add(pFilter);

	return Nx::SafeArray<IUnknown *>::From(aryFilters);
}

// (b.savon 2013-02-05 11:23) - PLID 51705
BOOL CPrescriptionQueueDlg::IsLicensedConfiguredUser()
{
	return m_lNexERxLicense.IsCurrentUserLicensedForNexERx();
}

// (j.fouts 2013-02-06 09:41) - PLID 54472 - Update the filters to show what needs attention for the current user
// (j.jones 2016-02-03 16:12) - PLID 68118 - renamed to simply respond to which notification link the user clicked
// if bShowRenewals is false, show the Rx Queue, else renewals
void CPrescriptionQueueDlg::LoadFromNotificationClick(bool bShowRenewals)
{
	// (j.jones 2016-02-03 16:32) - PLID 68159 - we no longer change the provider filters,
	// nor do we filter on transmit error, we instead use the dialog default of < Prescriptions Requiring Action >
		
	// (j.jones 2016-02-03 17:03) - PLID 68159 - continue to calculate the renewal date
	// because even if we don't default to the renewal view, it still needs to filter the same

	COleDateTime dtFrom = g_cdtNull;

	// (b.savon 2013-04-23 17:25) - PLID 54472 - Get the date to filter on the oldest renewal
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT  MIN(ReceivedDate) AS FromDate \r\n"
		"FROM	RenewalRequestsT \r\n"
		"	LEFT JOIN RenewalResponsesT ON RenewalRequestsT.ID = RenewalResponsesT.RequestID \r\n"
		"WHERE	RenewalResponsesT.ID IS NULL \r\n");
	if (!prs->eof) {
		dtFrom = AdoFldDateTime(prs->Fields, "FromDate", g_cdtNull);
	}
	prs->Close();

	// (j.jones 2016-02-03 17:05) - PLID 68118 - keep loading both screens, since the user can toggle
	// in between
	RequeryQueue(false);
	// (b.savon 2013-04-23 17:25) - PLID 54472 - Pass it as an override
	RequeryRenewalRequests(dtFrom);
	
	// (j.jones 2016-02-03 16:41) - PLID 68118 - don't call ToggleCollapsed, that only flips
	// whatever the current view is: instead force the display of the desired screen
	if(bShowRenewals) {
		m_pendingBox.ForceCollapsed();
		m_renewalsBox.ForceOpen();
	}
	else {
		m_renewalsBox.ForceCollapsed();
		m_pendingBox.ForceOpen();
	}

	UpdateShowState();

	LoadReminder(bShowRenewals ? m_Renewals : m_PrescriptionNeedingAttention, !bShowRenewals);
}

// (b.savon 2013-02-11 14:42) - PLID 54678
void CPrescriptionQueueDlg::OnBnClickedBtnMoreMeds()
{
	try{
		// (will prompt for passwords)
		if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
			return;
		}

		//open the edit medication dialog

		CEditMedicationListDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionQueueDlg::RefreshInteractionsFromArray(Nx::SafeArray<IUnknown*> saryDrugDrugInteracts,
		Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts,
		Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts)
{
	// (j.fouts 2013-05-30 09:45) - PLID 56807 - Drug Interactions is tied to NexERx, also moved this check
	// to be around the whole function rather than just the non-meds tab check
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if(m_bHasSureScriptsLicense)
	{
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(this, true))
			{
				//We decided not to have an additional warning here about drug interactions not
				//being check because if FDB is not built yet then there should really be no way
				//that they have FDB drugs/allergies in their database yet.
				return;
			}
		}

		// (j.jones 2013-05-14 13:49) - PLID 56634 - There was code here that summed up the array totals
		// and updated the interaction button, but it should be getting the count from the interactions
		// dialog like everywhere else does.
		long nInteractionCount = 0;

		//Show interactions if they were found
		// (j.fouts 2012-11-15 10:36) - PLID 53573 - If we are embeded use the main frame, else use the queue's dlg
		if(m_bInMedicationsTab)
		{
			nInteractionCount = GetMainFrame()->ShowDrugInteractions(saryDrugDrugInteracts,saryDrugAllergyInteracts,saryDrugDiagnosisInteracts, m_nPatientID);
			CMedicationDlg* pMedsDlg = dynamic_cast<CMedicationDlg*>(GetParent());
			if(pMedsDlg)
			{
				pMedsDlg->SetInteractionCount(nInteractionCount);
			}
		}
		else
		{
			if (!m_pDrugInteractionDlg) {
				// (b.savon 2012-11-30 10:30) - PLID 53773 - Pass in the parent enum
				m_pDrugInteractionDlg.reset(new CDrugInteractionDlg(this, eipQueue));
			}

			if (!IsWindow(m_pDrugInteractionDlg->GetSafeHwnd()))
			{
				m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
			}

			m_pDrugInteractionDlg->ShowOnInteraction(saryDrugDrugInteracts,saryDrugAllergyInteracts,saryDrugDiagnosisInteracts, m_nPatientID);
			// (j.jones 2013-05-14 13:49) - PLID 56634 - get the interaction count
			nInteractionCount = m_pDrugInteractionDlg->InteractionCount();
		}

		// (j.jones 2013-05-14 13:49) - PLID 56634 - regardless of how we got the count,
		// set it now in our dialog
		SetInteractionCount(nInteractionCount);
	}
}

// (b.savon 2013-03-01 12:54) - PLID 54704 - Edit allergy
void CPrescriptionQueueDlg::OnBnClickedBtnEditAllergiesQueue()
{
	try{
		//Don't open the list if they don't have permission to edit it.
		if(!CheckCurrentUserPermissions(bioPatientAllergies,sptDynamic0))
			return;

		//They want to edit the medication list so we need to open that dialog
		CEditAllergyListDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}


void CPrescriptionQueueDlg::RefreshSingleQueueRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = LoadFullPrescription(AsString(pRow->GetValue(pqcPrescriptionID)));

	Nx::SafeArray<IUnknown*> saryPrescriptions(pResults->PrescriptionsLoaded);
	foreach(NexTech_Accessor::_QueuePrescriptionPtr pPrescription, saryPrescriptions)
	{
		// (j.fouts 2013-04-25 12:46) - PLID 53146 - Seperated Favorite Pharmacies from full pharmacy list
		InsertPrescriptionIntoQueue(pPrescription, VarBool(pRow->GetValue(pqcSend),g_cvarFalse) == TRUE, VarBool(pRow->GetValue(pqcPrint),g_cvarFalse) == TRUE);
	}

	m_pPresQueueList->RemoveRow(pRow);
	UpdateSendButton();
	UpdatePreviewButton();
}

// (j.fouts 2013-04-22 14:12) - PLID 54719 - Update the hidden/shown columns
void CPrescriptionQueueDlg::UpdateView(bool bForceRefresh /*=true*/)
{
	CNxDialog::UpdateView(bForceRefresh);

	try
	{
		using namespace NXDATALIST2Lib;
		//check to see if they chose to hide the english prescription column
		BOOL bHideEnglishColumn = GetRemotePropertyInt("ShowEnglishPrescriptionColumn",0,0,GetCurrentUserName(),true) == 0;
		if(bHideEnglishColumn) {
			m_pPresQueueList->GetColumn(pqcEnglishSig)->PutColumnStyle(csVisible|csFixedWidth);
			m_pPresQueueList->GetColumn(pqcEnglishSig)->PutStoredWidth(0);
		} else {
			m_pPresQueueList->GetColumn(pqcEnglishSig)->PutColumnStyle(csVisible|csWidthAuto);
		}

		BOOL bHidePharmPhoneColumn = GetRemotePropertyInt("MedicationsTabShowPharmacyPhone",1,0,GetCurrentUserName(),true) == 0;
		if(bHidePharmPhoneColumn)
		{
			m_pPresQueueList->GetColumn(pqcPharmacyPhone)->PutColumnStyle(csVisible|csFixedWidth);
			m_pPresQueueList->GetColumn(pqcPharmacyPhone)->PutStoredWidth(0);
		} else {
			m_pPresQueueList->GetColumn(pqcPharmacyPhone)->PutColumnStyle(csVisible|csWidthAuto);
		}

		// (j.fouts 2013-01-21 12:26) - PLID 54719 - Added a preferance on hideing/showing these columns
		long nShowColumns = GetRemotePropertyInt("Prescription_ShowColumns", 0);
		if(!(nShowColumns & SHOW_SUPERVISOR))
		{
			m_pPresQueueList->GetColumn(pqcSupervisor)->PutColumnStyle(csFixedWidth|csVisible);
			m_pPresQueueList->GetColumn(pqcSupervisor)->PutStoredWidth(0);
		} else {
			m_pPresQueueList->GetColumn(pqcSupervisor)->PutColumnStyle(csVisible|csWidthAuto);	
		}

		if(!(nShowColumns & SHOW_NURSESTAFF))
		{
			m_pPresQueueList->GetColumn(pqcNurseStaff)->PutColumnStyle(csFixedWidth|csVisible);	
			m_pPresQueueList->GetColumn(pqcNurseStaff)->PutStoredWidth(0);
		} else {
			m_pPresQueueList->GetColumn(pqcNurseStaff)->PutColumnStyle(csVisible|csWidthAuto);
		}

		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		NXDATALIST2Lib::IColumnSettingsPtr pInfoButtonColumn = m_pMedsList->GetColumn(cmlcInfoButton);
		if(bShowPatientEducationButton) {
			pInfoButtonColumn->ColumnStyle |= NXDATALIST2Lib::csVisible;
		} else {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Hide the patient education info button column
			pInfoButtonColumn->ColumnStyle &= ~NXDATALIST2Lib::csVisible;
		}
		NXDATALIST2Lib::IColumnSettingsPtr pMedNameColumn = m_pMedsList->GetColumn(cmlcMedName);
		if(bShowPatientEducationLink) {
			pMedNameColumn->FieldType = NXDATALIST2Lib::cftTextSingleLineLink;
		} else {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Make the med name a non-hyperlink.
			pMedNameColumn->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		}
		//(s.dhole 3/23/2015 11:12 AM ) - PLID 64562 Work-arround to add place holder since we create this dialog runtime
		m_nxdlCurrentMedicationResults->PutSearchPlaceholderText("Medication Search...");
		m_nxdlWritePrescriptionResults->PutSearchPlaceholderText("Medication Search...");
		//(s.dhole 3/23/2015 11:13 AM ) - PLID 64562 Work-arround to add place holder since we create this dialog runtime
		m_nxdlCurrentAllergyResults->PutSearchPlaceholderText("Allergy Search...");
		// (s.dhole 2013-11-19 10:30) - PLID  56926
		CString strColWidths;
		// (j.jones 2016-01-12 09:33) - PLID 67855 - renamed CurrentPatientMedicationQueueColumnWidth to CurrentPatientMedicationQueueColumnWidths
		// to effectively reset the stored widths for current medications
		strColWidths = GetRemotePropertyText("CurrentPatientMedicationQueueColumnWidths", "", 0, GetCurrentUserName(), false);
		ResizeCurrentMedicationColumns(strColWidths);
		// (s.dhole 2013-11-19 10:30) - PLID  56931 resize column width
		strColWidths = GetRemotePropertyText("CurrentPatientAllergyQueueColumnWidth", "", 0, GetCurrentUserName(), false);
		ResizeAllergyColumns(strColWidths);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-04-22 16:03) - PLID 54719 - Update View when we show
void CPrescriptionQueueDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	try
	{
		// (b.eyers 2016-01-21) - PLID 67966 - if this is the rx needing attention, reset the date filter
		if (bShow) {
			// (b.eyers 2016-01-25) - PLID 67983 - we are in the rx needing attention dialog, select 'needs action' statues as the default
			m_pQueueStatusFilter->FindByColumn(qsfcID, -2, NULL, VARIANT_TRUE);
			CheckDlgButton(IDC_USE_PRESCRIPTION_DATE, 1);
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			COleDateTimeSpan aWeek(7);
			COleDateTime dtFrom = dtNow - aWeek;
			m_DateFrom.SetValue(_variant_t(dtFrom));
			m_DateTo.SetValue(_variant_t(dtNow));
			//This calls the requery for when in the rx needing attention dlg
			OnPrescriptionUseDate();
			// (b.eyers 2016-01-28) - PLID 67985 - we are in the rx needing attention dlg, set the queue status filter to it's default and requery renewals
			m_pRenewalFilterTransmitList->FindByColumn(rrfcID, -2, NULL, VARIANT_TRUE);
			// (b.eyers 2016-02-03) - PLID 67984 - default the response filter to all everytime the window opens
			m_pRenewalFilterResponseList->FindByColumn(rrfcID, NexTech_Accessor::RenewalResponseStatusFilter_All, NULL, VARIANT_TRUE);
			RequeryRenewalRequests();
		}
		UpdateView();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-04-25 12:46) - PLID 53146 - Generate a combo source string for a set of pharmacies ignoring another set of pharmacies
CString CPrescriptionQueueDlg::GeneratePharmacySourceString(Nx::SafeArray<IUnknown*> saryPharmacies, std::vector<int>* aryIgnoreList /*=NULL*/)
{
	CString strPharmacyCombo = "";
	if(aryIgnoreList)
	{
		std::sort(aryIgnoreList->begin(),aryIgnoreList->end());
	}

	foreach(NexTech_Accessor::_ERxQueuePharmacyPtr pPharmacy, saryPharmacies)
	{
		if(!aryIgnoreList || !std::binary_search(aryIgnoreList->begin(), aryIgnoreList->end(), atol(pPharmacy->PharmacyID)))
		{
			CString strPharm;
			CString strPharmName((LPCTSTR)pPharmacy->PharmacyName);
			strPharmName.Replace(";","");
			strPharm.Format("%s;%s;%li;", AsString(pPharmacy->PharmacyID), strPharmName, pPharmacy->Active);
			strPharmacyCombo += strPharm;
		}
	}
	return strPharmacyCombo;
}

// (j.fouts 2013-04-25 12:46) - PLID 53146 - Generate the full combo source string for pharmacies
CString CPrescriptionQueueDlg::GeneratePharmacySourceStringFull(Nx::SafeArray<IUnknown*> saryFavoritePharmacies)
{
	CString strPharmacyCombo;
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if(m_bHasSureScriptsLicense)
	{
		strPharmacyCombo.Format(";;1;;%li;< Add from Pharmacy Directory >;1;%li;< No Pharmacy Selected >;1;", -2, -1);
	}
	else
	{
		strPharmacyCombo.Format(";;1;;%li;< No Pharmacy Selected >;1;", -1);
	}

	std::vector<int> aryFavoritePharmacies;
	foreach(NexTech_Accessor::_ERxQueuePharmacyPtr pPharmacy, saryFavoritePharmacies)
	{
		aryFavoritePharmacies.push_back(atol(pPharmacy->PharmacyID));
	}

	strPharmacyCombo += GeneratePharmacySourceString(saryFavoritePharmacies);
	strPharmacyCombo += GeneratePharmacySourceString(m_saryAllPharmacies, &aryFavoritePharmacies);
	return strPharmacyCombo;
}

// (j.fouts 2013-04-26 09:30) - PLID 53146 - Warn about interactions
// (j.fouts 2013-09-17 16:53) - PLID 58496 - Handle the case when we are in needing attention, also added paramaters
bool CPrescriptionQueueDlg::PromptInteractions(long nPatientID, const CString &strPatientName /*=""*/)
{

	long nCount;
	if(m_bInMedicationsTab)
	{
		GetMainFrame()->ShowDrugInteractions(nPatientID, true, false, true);
		nCount = GetMainFrame()->GetDrugInteractionsFilteredCount();
	}
	else if(m_nPatientID == -1) 
	{
		CDrugInteractionDlg dlg(this);
		dlg.Create(IDD_DRUG_INTERACTION_DLG, this);
		dlg.ShowOnInteraction(nPatientID, true, false, true);

		if(dlg.GetFilteredInteractionCount() > 0)
		{
			//Since this comes from the needing attention dlg we can have multiple patients, be sure we specify that you are not continuing
			//with just this patient's prescriptions
			if(MessageBox(strPatientName == ""? "This patient has drug interactions. Please review them.\n\rAre you sure you want to continue?" :
				strPatientName + " has drug interactions. Please review them.\n\rAre you sure you want to continue with his\\her prescriptions?"
				, "Interactions Warning", MB_ICONWARNING|MB_YESNO) == IDYES)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		return true;	//There were no interactions
	}
	else
	{

		// (b.spivey, May 22, 2015) - PLID 66158 - make sure this is initialized properly before calling out to it. 
		if (!m_pDrugInteractionDlg) {
			// (b.spivey, June 8, 2015) - PLID 66158 - use the right enum. 
			m_pDrugInteractionDlg.reset(new CDrugInteractionDlg(this, eipQueue));
		}

		if (!IsWindow(m_pDrugInteractionDlg->GetSafeHwnd()))
		{
			m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
		}

		m_pDrugInteractionDlg->ShowOnInteraction(nPatientID, true, false, true);
		nCount = m_pDrugInteractionDlg->GetFilteredInteractionCount();
	}

	SetInteractionCount(nCount);

	if(nCount > 0)
	{
		if(MessageBox(strPatientName == ""? "This patient has drug interactions. Please review them.\n\rAre you sure you want to continue?" :
			strPatientName + " has drug interactions. Please review them.\n\rAre you sure you want to continue?"
			, "Interactions Warning", MB_ICONWARNING|MB_YESNO) == IDYES)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;	//There were no interactions
}

// (b.savon 2013-05-21 08:59) - PLID 56795
void CPrescriptionQueueDlg::ShowNewCropPrescriptionButton()
{
	m_btnESubmit.AutoSet(NXB_EXPORT);
	m_btnESubmit.SetWindowText("Send Electronic Prescription");	
}

void CPrescriptionQueueDlg::EditingFinishedCurrentMedsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow == NULL ){
			return;
		}

		if(bCommit) {

		// (s.dhole 2013-06-07 11:44) - PLID 56926
			switch(nCol){
			case cmlcSig:
			// (b.savon 2013-07-11 16:59) - PLID 57527 - Save the sig
				{
					CString strOldSig = VarString(varOldValue, "");
					CString strNewSig = VarString(varNewValue, "");

					if(strOldSig != strNewSig) {
			
						long nID = VarLong(pRow->GetValue(cmlcID));
						CString strDrugName = VarString(pRow->GetValue(cmlcMedName));
						// (s.dhole 2013-11-19 09:47) - PLID  56926
						UpdateCurrentMedRecord(pRow,cmlcSig,varNewValue);
						//audit
						long nAuditID = BeginNewAuditEvent();
						CString strPatientName = GetExistingPatientName(m_nPatientID);
						CString strOldAudit;
						strOldAudit.Format("Medication: %s, Sig: %s", strDrugName, strOldSig);
						AuditEvent(m_nPatientID, strPatientName, nAuditID, aeiCurrentMedicationSig, m_nPatientID, strOldAudit, strNewSig, aepHigh, aetChanged);

						//send a tablechecker
						m_CurrentMedsChecker.Refresh(m_nPatientID);
					}
				}
				break;
			case cmlcStartDate :
				{
						COleDateTime dtInvalid;
						dtInvalid.SetStatus(COleDateTime::invalid);
						COleDateTime dtNew = VarDateTime(varNewValue,dtInvalid);
						COleDateTime dtOld 	= VarDateTime(varOldValue,dtInvalid);
						if(dtNew.m_dt !=dtOld.m_dt ) {
							// (s.dhole 2013-06-07 11:44) - PLID 56926
							UpdateCurrentMedRecord(pRow,cmlcStartDate,varNewValue);
							CString strDrugName = VarString(pRow->GetValue(cmlcMedName));
							// (s.dhole 2013-06-18 14:38) - PLID 56927 audit changes
							long nAuditID = BeginNewAuditEvent();
							CString strPatientName = GetExistingPatientName(m_nPatientID);
							CString strOldAudit;
							if (dtOld.m_status == COleDateTime::invalid || (dtOld.m_dt < 1.0) )
							{
								strOldAudit.Format("Medication: %s, Start Date: -", strDrugName);
							}
							else
							{
								strOldAudit.Format("Medication: %s, Start Date: %s", strDrugName, FormatDateTimeForInterface (dtOld));
							}
							
							CString strNewDate= FormatDateTimeForInterface (dtOld);
							if (dtNew.m_status == COleDateTime::invalid || (dtNew.m_dt < 1.0) )
							{
								strNewDate.Format("%s"," - ");
							}
							else
							{
								strNewDate.Format("%s", FormatDateTimeForInterface (dtNew));
							}
							
							AuditEvent(m_nPatientID, strPatientName, nAuditID, aeiCurrentMedicationStartDate, m_nPatientID, strOldAudit, strNewDate, aepHigh, aetChanged);

							m_CurrentMedsChecker.Refresh(m_nPatientID);
						}
					}
					break;
			}
		}
		

	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionQueueDlg::EditingStartingCurrentMedsList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow == NULL ){
			return;
		}

		// (b.savon 2013-07-11 17:08) - PLID 57527 - Mirror the meds tab; Dont allow editing the sig if there is something there from NewCrop
		if(nCol == cmlcSig) {

			// (j.jones 2011-05-03 08:43) - PLID 43450 - disallow editing the sig if it's a NewCrop medication
			CString strNewCropGUID = VarString(pRow->GetValue(cmlcNewCropGUID), "");

			if(!strNewCropGUID.IsEmpty()) {
				//silently leave, the NewCrop meds are already colored and read-only elsewhere
				*pbContinue = FALSE;
				return;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-07-11 17:15) - PLID 57527 - Mirror the meds tab; Don't allow notes to be edited if they dont have permissions to write
void CPrescriptionQueueDlg::EditingFinishingAllergiesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	// (a.walling 2006-11-27 14:47) - PLID 20179 - Prevent editing of notes if Write is disabled
	if(!CheckCurrentUserPermissions(bioPatientAllergies, sptWrite)) {
		// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
		VariantClear(pvarNewValue);
		*pvarNewValue = _variant_t(varOldValue).Detach();
		*pbCommit = FALSE;
		return;
	}
}

// (b.savon 2013-07-11 17:21) - PLID 57527 - Save the notes if they edit
void CPrescriptionQueueDlg::EditingFinishedAllergiesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow == NULL ){
			return;
		}

		// (c.haag 2010-01-04 14:12) - PLID 35766 - This conditional used to test the number 2 instead
		// of the proper enumeration. I am not a number! I am a free enum!
		if (nCol == aldNotes) {

			CString strOldDesc = (VT_EMPTY == varOldValue.vt) ? "" : VarString(varOldValue,"");
			CString strNewDesc = VarString(varNewValue,"");

			// (c.haag 2007-10-19 12:01) - PLID 27822 - Do nothing if the names match
			if (strOldDesc == strNewDesc)
				return;

			//get the ID of the allergy
			long nAllergyID;
			nAllergyID  = VarLong(pRow->GetValue(alcID));
			// (s.dhole 2013-11-21 13:04) - PLID 56931 update date
			ExecuteParamSql("UPDATE PatientAllergyT SET Description = {STRING}, LastUpdateDate=GETDATE() WHERE ID = {INT}", strNewDesc, nAllergyID);

			// (c.haag 2007-10-19 12:01) - PLID 27822 - Audit the description change
			long nAuditID = BeginNewAuditEvent();
			CString strOldAudit, strNewAudit;
			CString strAllergyName;
			// (j.gruber 2009-06-08 15:40) - PLID 34395 - trim the {Inactive} off
			strAllergyName = VarString(pRow->GetValue(alcName));
			// (j.jones 2012-04-04 11:17) - PLID 49411 - We should not have used TrimRight,
			// it would trim off additional letters in the word Inactive. However, we actually
			// want to keep the word Inactive in our audit, so we know that an inactive allergy
			// was changed.
			//strAllergyName.TrimRight(" - {Inactive}");
			strOldAudit.Format("Allergy: '%s' - Description: %s", strAllergyName, strOldDesc);
			strNewAudit.Format("Description: %s", strNewDesc);
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiPatientAllergyDescription, m_nPatientID, strOldAudit, strNewAudit, aepMedium, aetChanged);
			
			m_pAllergiesList->Requery();

			// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
			UpdateAllergyReviewStatus(FALSE);

			// (j.jones 2008-11-25 12:20) - PLID 32183 - send a tablechecker, after the allergy review status is changed
			m_PatientAllergiesChecker.Refresh(m_nPatientID);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-07-16 12:44) - PLID 57377 - Add a button to access to formulary information from the queue
void CPrescriptionQueueDlg::OnBnClickedBtnNexformulary()
{
	try{
		 // (r.farnworth 2013-09-24 16:21) - PLID 58386 - Created
		// (r.farnworth 2013-10-18 14:17) - PLID 59095 - Need to pass in parent dialog to produce the truncation warning
		long ProviderID = GetGen1ProviderID(m_nPatientID, this);
		if (ProviderID == -1)
		{
			return; 
		}

		// (s.dhole 2013-09-16 09:05) - PLID 58357
		if (!IsPatientEventExist(this,m_nPatientID))
		{
			return; 
		}

		NexFormularyDrug nfdDrug;
		CNexFormularyDlg dlg(nfsQueue, m_nPatientID, nfdDrug, ProviderID); // (r.farnworth 2013-09-24 15:41) - PLID 58386 - Added ProviderID

		// (r.farnworth 2013-08-14 10:52) - PLID 58001 - If a writeable prescription was clicked, handle the prescription writing
		if( dlg.DoModal() == IDOK){
			long nWriteDrugListID = dlg.GetDrugListID();
			// (s.dhole 2013-10-19 11:25) - PLID 59068  Pass insurance ID
			long nFnInsuranceID = dlg.GetFInsuranceID(); 
			if (nWriteDrugListID != -1) {
				WritePrescription(nWriteDrugListID, m_nPatientID,nFnInsuranceID);
			}
		}


	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-08-23 14:49) - PLID 58236 - Break this out into a function
void CPrescriptionQueueDlg::InvokeNexFormularyDlg(NXDATALIST2Lib::IRowSettingsPtr pRow, long nFDBID, const CString &strDrugName, ENexFormularySource nfsSource)
{
	try
	{
		// (r.farnworth 2013-09-24 16:21) - PLID 58386 - Created
		// (r.farnworth 2013-10-18 14:17) - PLID 59095 - Need to pass in parent dialog to produce the truncation warning
		long ProviderID = GetGen1ProviderID(m_nPatientID, this); 
		if (ProviderID == -1)
		{
			return; 
		}

		// (s.dhole 2013-09-16 09:05) - PLID 58357
		if (!IsPatientEventExist(this,m_nPatientID))
		{
			return; 
		}

		NexFormularyDrug nfdDrug;
		nfdDrug.nFDBID = nFDBID;
		nfdDrug.strDrugName = strDrugName;
		CNexFormularyDlg dlg(nfsQueue, m_nPatientID, nfdDrug, ProviderID); // (r.farnworth 2013-09-24 15:41) - PLID 58386 - Added ProviderID

		// (r.farnworth 2013-08-14 10:49) - PLID 58001 - If a writeable medication was clicked, handle the prescription writing
		if( dlg.DoModal() == IDOK){
			long nWriteDrugListID = dlg.GetDrugListID();
			
			//If we're from the quicklist, write that med in quicklist form
			if( nWriteDrugListID != -1 && nfsSource == nfsQuickList && VarLong(pRow->GetValue(qlcDrugListID)) == nWriteDrugListID ){
				CArray<NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> aryQuicklist;
				aryQuicklist.Add(pRow);
				CreateAndSavePrescriptions(aryQuicklist);
			}else if (nWriteDrugListID != -1) {
				// (s.dhole 2013-10-25 10:38) - PLID 59110 Added insuranceid
				long nInsuranceID =  dlg.GetFInsuranceID();
				WritePrescription(nWriteDrugListID, m_nPatientID,nInsuranceID);
			}else{
				//This isn't writable because it isn't in our DrugList
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkboxes to show/hide discontinued allergies and meds
void CPrescriptionQueueDlg::OnCheckHideDiscontinuedAllergiesQueue()
{
	try {

		SetRemotePropertyInt("HideDiscontinuedAllergies", m_checkHideDiscontinuedAllergies.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		RequeryAllergiesAndMeds(true, false);

	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionQueueDlg::OnCheckHideDiscontinuedMedsQueue()
{
	try {

		SetRemotePropertyInt("HideInactiveMedications", m_checkHideDiscontinuedMedications.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		RequeryAllergiesAndMeds(false, true);

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/20/2013) - PLID 58396 - Handle the message to refresh the medication history button.
LRESULT CPrescriptionQueueDlg::OnBackgroundMedHistoryRequestComplete(WPARAM wParam, LPARAM lParam)
{
	try
	{
		long nPatientID = (long)wParam;
		BOOL bHasMedicationHistory = (BOOL)lParam;
		if(nPatientID != m_nPatientID) {
			return 0;
		}
		if(m_bInMedicationsTab) {
			// This should be handled by the medication tab then because the button is is owned by that dialog in that case.
			GetParent()->SendMessage(NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE, wParam, lParam);
		} else {
			// (r.gonet 09/20/2013) - PLID 58396 - Set the text color to red to indicate that the patient history history.
			m_btnMedicationHistory.SetTextColor(bHasMedicationHistory ? RGB(255,0,0) : RGB(0,0,0));
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2013-10-17 16:13) - PLID 58983 - added left click handler
void CPrescriptionQueueDlg::OnLeftClickCurrentMedsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
	
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_pMedsList->PutCurSel(pRow);

		switch(nCol) {
			// (j.jones 2013-10-17 16:14) - PLID 58983 - added Patient Education support
			case cmlcMedName:
			case cmlcInfoButton:
				{
				bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
				if(nCol == cmlcMedName && (!bShowPatientEducationLink || m_pMedsList->GetColumn(cmlcMedName)->FieldType != NXDATALIST2Lib::cftTextSingleLineLink)) {
					// (r.gonet 2014-01-27 15:29) - PLID 59339 - This is simply text. Don't go anywhere.
					break;
				}
				long nMedicationID = VarLong(pRow->GetValue(cmlcMedicationID), -1);
				if(nMedicationID == -1) {
					//should not be possible
					ASSERT(FALSE);
					ThrowNxException("No valid medication ID was found.");
				}

				if(nCol == cmlcMedName) {
					// (r.gonet 10/30/2013) - PLID 58980 - The drug name hyperlink goes to the MedlinePlus website
					LookupMedlinePlusInformationViaSearch(this, mlpDrugListID, nMedicationID);
				}
				else if(nCol == cmlcInfoButton) {
					//the info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
					LookupMedlinePlusInformationViaURL(this, mlpDrugListID, nMedicationID);
				}
				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2013-06-07 11:44) - PLID 56926
void CPrescriptionQueueDlg::ResizeCurrentMedicationColumns(const CString strColWidths)
{
	
	short i ;
	if (strColWidths.IsEmpty())
	{
		return;
	}
	else
	{
		for (i = cmlcID; i <= cmlcLastUpdateDate ; i++)
		{
				// we should not do this but err can cause issue to show hiddent coulmn
			switch(i){ 
				case cmlcMedName:
				case cmlcSig:
				case cmlcStartDate:
				case cmlcLastUpdateDate:
				case cmlcInfoButton:
					m_pMedsList->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);
					break;
				default:
					m_pMedsList->GetColumn(i)->StoredWidth = 0;
					break;
			}
		}
	}
}

// (s.dhole 2013-06-07 11:44) - PLID 56931 Resize allergy column width
void CPrescriptionQueueDlg::ResizeAllergyColumns(const CString strColWidths)
{
	short i ;
	if (strColWidths.IsEmpty()){
		return;
	}
	else{
		for (i = cmlcID; i <= alcLastUpdateDate ; i++){
			switch(i){ // we should not do this but err can cause issue to show hiddent coulmn
				case alcEnteredDate:
				case alcName:
				case aldNotes:
				case alcLastUpdateDate:
					m_pAllergiesList->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);				
					break;
				default:
					m_pAllergiesList->GetColumn(i)->StoredWidth =0;
					break;
			}
		}
	}
}


// (s.dhole 2013-06-07 11:44) - PLID 56926
CString  CPrescriptionQueueDlg::ReadColumnWidths(NXDATALIST2Lib::_DNxDataListPtr pList)
{
	CString strWidths;
	for (short i=0; i < pList->ColumnCount; i++)
	{
		NXDATALIST2Lib::IColumnSettingsPtr  pCol = pList->GetColumn(i);
		CString str;
		if (i == 0){
			str.Format("%d", pCol->StoredWidth);
		}
		else{
			str.Format(",%d", pCol->StoredWidth);
		}
		strWidths += str;
	}
	return strWidths;
	
}


// (s.dhole 2013-06-07 11:44) - PLID 56926
long CPrescriptionQueueDlg::GetColumnWidth(const CString& strColWidths, short nColumn)
{
	CString str;
	int nIndex = 0, nEndIndex = 0;
	for (short i=0; i < nColumn && nIndex != -1; i++)
	{
		nIndex = strColWidths.Find(',', nIndex+1);
	}
	if (nIndex == -1)
		return -1;
	nEndIndex = strColWidths.Find(',', nIndex+1);
	if (nEndIndex == -1)
		nEndIndex = strColWidths.GetLength();
	str = strColWidths.Mid(nIndex == 0 ? 0 : (nIndex+1), nEndIndex - (nIndex == 0 ? 0 : nIndex+1));
	return atoi(str);
}
// (s.dhole 2013-11-19 09:36) - PLID 56926 Update current Medication
// no try catch since it is called from another function
void  CPrescriptionQueueDlg::UpdateCurrentMedRecord(NXDATALIST2Lib::IRowSettingsPtr pRow, short nColumn,const VARIANT&  varValue )
{
	long nID =  VarLong(pRow->GetValue( cmlcID),-1);
	_variant_t varNewValue;
	if (nID >0)
	{
		CString strFiled;  
		switch(nColumn) 
		{
			case cmlcSig: 
				{
				strFiled="Sig = {VT_BSTR} " ;
				varNewValue = _variant_t(varValue.bstrVal);
				}
				break;
			case cmlcDiscontinued: 
				{
				strFiled="Discontinued = {INT} , DiscontinuedDate = GETDATE() ";
				varNewValue = _variant_t(varValue.lVal , VT_I4 );
				}
				break;
			case cmlcStartDate: 
				{
				strFiled="StartDate = {VT_DATE}";
					
					if(varValue.vt == VT_NULL ) {
						varNewValue = g_cvarNull ;
					}
					else {
						varNewValue = _variant_t(varValue.date, VT_DATE);
					}
				}
				break;

		}
		_RecordsetPtr pRs = CreateParamRecordset( FormatString( 
		" SET NOCOUNT ON \r\n"
		" DECLARE @ID AS INT \r\n"
		" SET @ID = {INT} \r\n"
		" UPDATE CurrentPatientMedsT SET %s, LastUpdateDate =  GETDATE()   \r\n"
		" OUTPUT inserted.LastUpdateDate  \r\n"
		" WHERE ID = @ID ; \r\n", strFiled )
     	,nID,varNewValue);
		if (!pRs->eof){
			_variant_t var =pRs->Fields->Item["LastUpdateDate"]->Value;
			pRow->PutValue(cmlcLastUpdateDate,var);
		}
	}
}

// (s.dhole 2013-11-19 09:36) - PLID 56926 
void CPrescriptionQueueDlg::ColumnSizingFinishedCurrentMedsList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try{
		// (j.jones 2016-01-12 09:33) - PLID 67855 - renamed CurrentPatientMedicationQueueColumnWidth to CurrentPatientMedicationQueueColumnWidths
		// to effectively reset the stored widths for current medications
		SetRemotePropertyText("CurrentPatientMedicationQueueColumnWidths",ReadColumnWidths(m_pMedsList), 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-11-21 11:46) - PLID 56931 save column size
void CPrescriptionQueueDlg::ColumnSizingFinishedAllergiesList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try{
		SetRemotePropertyText("CurrentPatientAllergyQueueColumnWidth",ReadColumnWidths(m_pAllergiesList), 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-11-19 09:36) - PLID 56926 
void CPrescriptionQueueDlg::EditingFinishingCurrentMedsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		// (s.dhole 2014-01-16 12:43) - PLID 60367 check permission
		if (!pbCommit || *pbCommit == FALSE ){
			return;
		}
		if(!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptWrite)) {
			VariantClear(pvarNewValue);
			
			*pbCommit = FALSE;
			return;
		}
		switch(nCol){
		case cmlcStartDate:
			{
				COleDateTime dt = pvarNewValue->date; 
				if(dt.m_dt  == 0.0) {
					_variant_t varNull;
					varNull.vt = VT_NULL;
					*pvarNewValue= varNull;
					return;
				}
				else if(dt.m_status == COleDateTime::invalid || (dt.m_dt < 1.0) ||  dt.GetYear() < 1900) {
					MsgBox("Please enter a valid date.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-01-03 08:06) - PLID 58984 - Verify prescriptions, current medications, and allergies work appropriately (in medications tab as well as emr)
// when licensed for newcrop only, licensed for nexerx only, or licensed for neither newcrop nor nexerx.
void CPrescriptionQueueDlg::HideNexFormularyColumn(NXDATALIST2Lib::_DNxDataListPtr nxdlDatalist, const short column)
{
	try{
		NXDATALIST2Lib::IColumnSettingsPtr pNexFormularyColumn = nxdlDatalist->GetColumn(column);
		pNexFormularyColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth|NXDATALIST2Lib::csVisible;			
		pNexFormularyColumn->PutStoredWidth(0);
	}NxCatchAll(__FUNCTION__);
}


//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
void CPrescriptionQueueDlg::SelChosenNxdlMedSearchResultsQueue(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(mlcMedicationID)) == NO_RESULTS_ROW){
				return;
			}
			AddMedToCurrentMedicationsList(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
void CPrescriptionQueueDlg::LButtonDownNxdlWriteRx(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(mrcMedicationID)) == NO_RESULTS_ROW){
				return;
			}
			if (nCol == mrcNexFormulary){ // (b.savon 2013-07-17 08:39) - PLID 57377
				long nMedicationID = AsLong(pRow->GetValue(mrcMedicationID));
				if (nMedicationID == -1){
					CWaitCursor cwait;
					ImportMedication(pRow, nMedicationID);
				}

				// (b.savon 2013-08-23 14:51) - PLID 58236 - Call func
				InvokeNexFormularyDlg(pRow, VarLong(pRow->GetValue(mrcFirstDataBankID), -1), AsString(pRow->GetValue(mrcMedicationName)), nfsWriteRx);
			}
		}
	}NxCatchAll(__FUNCTION__);
}


//(s.dhole 3/10/2015 1:46 PM ) - PLID 64561
void CPrescriptionQueueDlg::SelChosenNxdlWriteRx(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			OnClickWritePrescriptionFromSearchResults(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2016-01-06 14:30) - PLID 58692 - Wants to be able to quickly re-prescribe a prescription. Perhaps a right click a	nd "Re-send" or "Re-Prescribe" (This functionality was in NewCrop)
PrescriptionInfo  CPrescriptionQueueDlg::CopyPrescription(long nPrescriptionID, long nMedicationID, long nPatientID /* = -1 */, long nFInsuranceID /* = -1 */, bool bIsTemplate /*=false*/)
{
	if (nPatientID == -1)
		nPatientID = m_nPatientID;

	CPrescriptionEditDlg dlg(this, m_nCurrentlyOpenedEMNID);
	
	NexTech_Accessor::_UpdatePresQueueResultsPtr pNewResult = SaveNewPrescription(nMedicationID, TRUE, GetQueueFilters(), nPatientID,
		m_nDefaultProviderID, m_nDefaultLocationID, m_dtDefaultPrescriptionDate, m_nCurrentlyOpenedEMNID, 0, SourceActionInfo(), -1, -1, nFInsuranceID);

	if (pNewResult->PrescriptionsAdded == NULL) {
		ThrowNxException("Unable to Add Prescription in CPrescriptionQueueDlg::WritePrescription!");
	}

	CString strPrescriptionID;
	strPrescriptionID.Format("%li", nPrescriptionID);
	NexTech_Accessor::_UpdatePresQueueResultsPtr pLoadResult = LoadFullPrescription(strPrescriptionID, bIsTemplate);

	if (pLoadResult->PrescriptionsLoaded == NULL) {
		ThrowNxException("Unable to Load Prescription!");
	}

	PrescriptionInfo rxInformation;

	Nx::SafeArray<IUnknown*> saryLoadPrescriptions(pLoadResult->PrescriptionsLoaded);
	NexTech_Accessor::_QueuePrescriptionPtr pLoadPrescription = saryLoadPrescriptions[0];

	Nx::SafeArray<IUnknown*> saryNewPrescriptions(pNewResult->PrescriptionsAdded);
	NexTech_Accessor::_QueuePrescriptionPtr pNewPrescription = saryNewPrescriptions[0];

	//Prescription ID
	pLoadPrescription->PrescriptionID = pNewPrescription->PrescriptionID;

	//Prescription Date
	NexTech_Accessor::_NullableDateTimePtr pToday(__uuidof(NexTech_Accessor::NullableDateTime));
	pToday->SetDateTime(COleDateTime::GetCurrentTime());
	pLoadPrescription->PrescriptionDate = pToday;

	//Supervisor
	NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pSupervisor(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
	Nx::SafeArray<IUnknown*> sarySupervisors = pNewResult->Supervisor;
	if ((int)sarySupervisors.GetCount() == 1) {
		pSupervisor = sarySupervisors[0];
	}
	
	pLoadPrescription->Supervisor = pSupervisor;

	//Nurse/Staff
	NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pNurseStaff(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
	Nx::SafeArray<IUnknown*> saryNurseStaff = pNewResult->NurseStaff;
	if ((int)saryNurseStaff.GetCount() == 1) {
		pNurseStaff = saryNurseStaff[0];
	}
	
	pLoadPrescription->NurseStaff = pNurseStaff;

	//Prescriber
	NexTech_Accessor::_QueuePrescriptionPrescriberUserPtr pProvider(__uuidof(NexTech_Accessor::QueuePrescriptionPrescriberUser));
	Nx::SafeArray<IUnknown*> saryProvider = pNewResult->Prescriber;
	if ((int)saryProvider.GetCount() == 1) {
		pProvider = saryProvider[0];
	}
	
	pLoadPrescription->Provider = pProvider;

	//Location
	pLoadPrescription->locationID = FormatBstr("%li", GetCurrentLocationID());

	//Insurance - We cannot just blindly fill this in, we need to check the formulary information
	long nInsuranceID = -1;
	// (j.jones 2016-01-22 10:43) - PLID 63732 - cached whether they have the SureScripts license
	if (m_bHasSureScriptsLicense) {
		nInsuranceID = CheckExistingFormularyData();
	}

	if (nInsuranceID > -1) {
		pLoadPrescription->InsuranceDetailID = AsBstr(AsString(nInsuranceID)); // (s.dhole 2013-10-28 09:30) - PLID 59189
	}

	// (r.farnworth 2016-02-15 10:24) - PLID 58692 - Wants to be able to quickly re-prescribe a prescription. Perhaps a right click and "Re-send" or "Re-Prescribe" (This functionality was in NewCrop)
	pLoadPrescription->status = NexTech_Accessor::PrescriptionStatus_Incomplete;

	//Configure the dropdowns to show/hide based on the current user settings.
	rxInformation.pPrescription = pLoadPrescription;
	rxInformation.erxUserRole = pNewResult->UserRole;
	rxInformation.saryPrescribers = pNewResult->Prescriber;
	rxInformation.sarySupervisors = pNewResult->Supervisor;
	rxInformation.saryNurseStaff = pNewResult->NurseStaff;

	// (j.jones 2016-01-22 14:24) - PLID 63732 - we can't begin the progress bar futher up due to possible formulary warnings
	CProgressDialog progressDialog;
	BeginProgressDialog(progressDialog);

	RefreshQueueFromArray(pNewResult->PrescriptionsInQueue, pNewResult->Pharmacies, progressDialog);

	DrugInteractionInfo drugInteractionInfo;
	if (pNewResult->DrugDrugInteracts) {
		drugInteractionInfo.saryDrugDrugInteracts = pNewResult->DrugDrugInteracts;
	}
	if (pNewResult->DrugAllergyInteracts) {
		drugInteractionInfo.saryDrugAllergyInteracts = pNewResult->DrugAllergyInteracts;
	}
	if (pNewResult->DrugDiagnosisInteracts) {
		drugInteractionInfo.saryDrugDiagnosisInteracts = pNewResult->DrugDiagnosisInteracts;
	}

	// (j.jones 2016-01-22 14:24) - PLID 63732 - end the progress bar
	progressDialog.Stop();

	return rxInformation;
}

// (b.savon 2016-01-18 12:39) - PLID 67804 - Change all med/allergy datalist search ui to not use link cell types
void CPrescriptionQueueDlg::SelChosenNxdlAllergySearchResultsQueue(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(aslcAllergyID)) == NO_RESULTS_ROW) {
				return;
			}
			// Add the allergy to the list using existing logic
			// Hide the results box, and reset the search text to nothing.  Set focus back to the search box as well.
			AddAllergyToCurrentAllergyList(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-21 16:59) - PLID 67993 - added option to include free text meds in the prescription search
void CPrescriptionQueueDlg::OnCheckIncludeFreeTextRx()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextRx.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, false)) {
				//they changed their minds
				m_checkIncludeFreeTextRx.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextRx.SetFocus();
		}

		//reflect their choice in the search results
		ResetPrescriptionSearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-22 08:42) - PLID 67993 - resets the prescription search provider based
// on the value of the prescription' 'include free text' checkbox
void CPrescriptionQueueDlg::ResetPrescriptionSearchProvider()
{
	try {

		bool bIncludeFDBMedsOnly = m_checkIncludeFreeTextRx.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBMedsOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlWritePrescriptionResults = BindMedicationSearchListCtrl(this, IDC_NXDL_WRITE_RX, GetRemoteData(), m_bShowNexFormulary ? true : false, bIncludeFDBMedsOnly);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-21 17:06) - PLID 67996 - added option to include free text meds in the current meds search
void CPrescriptionQueueDlg::OnCheckIncludeFreeTextMeds()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextCurMeds.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, false)) {
				//they changed their minds
				m_checkIncludeFreeTextCurMeds.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextCurMeds.SetFocus();
		}

		//reflect their choice in the search results
		ResetCurrentMedsSearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-22 09:22) - PLID 67996 - resets the current meds search provider based
// on the value of the current meds' 'include free text' checkbox
void CPrescriptionQueueDlg::ResetCurrentMedsSearchProvider()
{
	try {

		bool bIncludeFDBMedsOnly = m_checkIncludeFreeTextCurMeds.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBMedsOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlCurrentMedicationResults = BindMedicationSearchListCtrl(this, IDC_NXDL_MED_SEARCH_RESULTS_QUEUE, GetRemoteData(), false, bIncludeFDBMedsOnly);
		
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-21 17:06) - PLID 67997 - added option to include free text allergies in the allergy search
void CPrescriptionQueueDlg::OnCheckIncludeFreeTextAllergies()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextAllergies.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, true)) {
				//they changed their minds
				m_checkIncludeFreeTextAllergies.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextAllergies.SetFocus();
		}

		//reflect their choice in the search results
		ResetAllergiesSearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-22 09:58) - PLID 67997 - resets the allergy search provider based
// on the value of the allergies' 'include free text' checkbox
void CPrescriptionQueueDlg::ResetAllergiesSearchProvider()
{
	try {

		bool bIncludeFDBAllergiesOnly = m_checkIncludeFreeTextAllergies.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBAllergiesOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlCurrentAllergyResults = BindAllergySearchListCtrl(this, IDC_NXDL_ALLERGY_SEARCH_RESULTS_QUEUE, GetRemoteData(), bIncludeFDBAllergiesOnly);

	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnChangeRxAttentionFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

		if (!m_bDateFromDown) {

			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_DateFrom.GetValue());
			dtTo = COleDateTime(m_DateTo.GetValue());
			if (dtFrom.GetStatus() == COleDateTime::invalid) {
				AfxMessageBox("You have entered an invalid 'From' date. Please correct this date.");
				m_DateFrom.SetValue(_variant_t(dtTo));
			}
			else {

				//if dtFrom > dtTo, update dtTo
				if (dtFrom > dtTo) {
					dtTo = dtFrom;
					m_DateTo.SetValue(_variant_t(dtTo));
				}

				RequeryQueue(false);
			}
		}

		*pResult = 0;

	}NxCatchAll("CPrescriptionQueueDlg::OnChangeRxAttentionFromDate");
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnChangeRxAttentionToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

		if (!m_bDateToDown) {

			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_DateFrom.GetValue());
			dtTo = COleDateTime(m_DateTo.GetValue());
			if (dtTo.GetStatus() == COleDateTime::invalid) {
				AfxMessageBox("You have entered an invalid 'To' date. Please correct this date.");
				m_DateTo.SetValue(_variant_t(dtFrom));
			}
			else {

				//if dtFrom > dtTo, update dtFrom
				if (dtFrom > dtTo) {
					dtFrom = dtTo;
					m_DateFrom.SetValue(_variant_t(dtFrom));
				}

				RequeryQueue(false);
			}
		}

		*pResult = 0;

	}NxCatchAll("CPrescriptionQueueDlg::OnChangeRxAttentionToDate");
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnDtnDropdownRxAttentionFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateFromDown = TRUE;

		*pResult = 0;

	}NxCatchAll("CPrescriptionQueueDlg::OnDtnDropdownRxAttentionFromDate");
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnDtnDropdownRxAttentionToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateToDown = TRUE;

		*pResult = 0;

	}NxCatchAll("CPrescriptionQueueDlg::OnDtnDropdownRxAttentionToDate");
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnDtnCloseupRxAttentionFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateFromDown = FALSE;

		COleDateTime dtFrom, dtTo;
		dtFrom = COleDateTime(m_DateFrom.GetValue());
		dtTo = COleDateTime(m_DateTo.GetValue());
		if (dtFrom.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("You have entered an invalid 'From' date. Please correct this date.");
			m_DateFrom.SetValue(_variant_t(dtTo));
		}
		else {

			//if dtFrom > dtTo, update dtTo
			if (dtFrom > dtTo) {
				dtTo = dtFrom;
				m_DateTo.SetValue(_variant_t(dtTo));
			}

			RequeryQueue(false);
		}

		*pResult = 0;

	}NxCatchAll("CPrescriptionQueueDlg::OnDtnCloseupRxAttentionFromDate");
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnDtnCloseupRxAttentionToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		m_bDateToDown = FALSE;

		COleDateTime dtFrom, dtTo;
		dtFrom = COleDateTime(m_DateFrom.GetValue());
		dtTo = COleDateTime(m_DateTo.GetValue());
		if (dtTo.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("You have entered an invalid 'To' date. Please correct this date.");
			m_DateTo.SetValue(_variant_t(dtFrom));
		}
		else {

			//if dtFrom > dtTo, update dtFrom
			if (dtFrom > dtTo) {
				dtFrom = dtTo;
				m_DateFrom.SetValue(_variant_t(dtFrom));
			}

			RequeryQueue(false);
		}

		*pResult = 0;

	}NxCatchAll("CPrescriptionQueueDlg::OnDtnCloseupRxAttentionToDate");
}

// (b.eyers 2016-01-21) - PLID 67966
void CPrescriptionQueueDlg::OnPrescriptionUseDate()
{

	try {
		if (IsDlgButtonChecked(IDC_USE_PRESCRIPTION_DATE)) {

			m_DateTo.EnableWindow(TRUE);
			m_DateFrom.EnableWindow(TRUE);
			RequeryQueue(false);
		}
		else {

			m_DateTo.EnableWindow(FALSE);
			m_DateFrom.EnableWindow(FALSE);
			RequeryQueue(false);
		}

	}NxCatchAll("Error in CPrescriptionQueueDlg::OnPrescriptionUseDate");

}

// (r.gonet 2016-01-22 15:33) - PLID 67973 - Handle when the user clicks a label.
LRESULT CPrescriptionQueueDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		case IDC_MULTI_RX_PRESCRIBERS_LABEL:
			// (r.gonet 2016-01-22 16:05) - PLID 67967 - If they clicked the multi rx prescribers label, then
			// have the controller launch the CMultiSelectDlg.
			if (m_rxPrescriberController.DoMultiSelect()) {
				// (r.gonet 2016-01-22) - PLID 67967 - The filter may have changed.  Requery the queue.
				RequeryQueue(false);
			}
			break;
		case IDC_MULTI_RENEWAL_PRESCRIBERS_LABEL:
			// (r.gonet 2016-01-22 15:33) - PLID 67973 - If they clicked the multi renewal prescribers label, then
			// have the controller launch the CMultiSelectDlg.
			if (m_renewalPrescriberController.DoMultiSelect()) {
				// (r.gonet 2016-01-22) - PLID 67973 - The filter may have changed.  Requery the requests.
				RequeryRenewalRequests();
			}
			break;

		default:
			//Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (r.gonet 2016-01-22 15:33) - PLID 67973 - Handle when the cursor changes.
BOOL CPrescriptionQueueDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (r.gonet 2016-01-22 16:06) - PLID 67967 - If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiRxPrescribers.IsWindowVisible() && m_nxlMultiRxPrescribers.IsWindowEnabled()) {
			m_nxlMultiRxPrescribers.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// (r.gonet 2016-01-22 15:33) - PLID 67973 - If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiRenewalPrescribers.IsWindowVisible() && m_nxlMultiRenewalPrescribers.IsWindowEnabled()) {
			m_nxlMultiRenewalPrescribers.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAllCallIgnore({
		if (m_bNotifyOnce) {
			m_bNotifyOnce = false;
			try { throw; }NxCatchAll(__FUNCTION__);
		}
	});
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnBnClickedRadioErxDontremind2()
{
	OnBnClickedRadioDontremind(m_PrescriptionNeedingAttention);
	SaveReminder(m_PrescriptionNeedingAttention);
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnBnClickedRadioErxRemindlogin()
{
	OnBnClickedRadioRemindlogin(m_PrescriptionNeedingAttention);
	SaveReminder(m_PrescriptionNeedingAttention);
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnBnClickedRadioErxHour()
{
	OnBnClickedRadioHour(m_PrescriptionNeedingAttention);
	SaveReminder(m_PrescriptionNeedingAttention);
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnBnClickedErxRadioMinutes()
{
	OnBnClickedRadioMinutes(m_PrescriptionNeedingAttention);
	SaveReminder(m_PrescriptionNeedingAttention);
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnDeltaposErxMinuteSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposMinuteSpin(pNMHDR, pResult, m_PrescriptionNeedingAttention);
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnDeltaposErxHourSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposHourSpin(pNMHDR, pResult, m_PrescriptionNeedingAttention);
}
// (s.tullis 2016-01-28 17:45) - PLID 67965
// (s.tullis 2016-01-28 17:46) - PLID 68090
// Function to show/hide the controls
void CPrescriptionQueueDlg::ShowHideReminderControls(BOOL bShow)
{
	try {
		

			GetDlgItem(IDC_RADIO_ERX_REMINDLOGIN)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_ERX_RADIO_MINUTES)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_ERX_MINUTE_TIME)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_ERX_MINUTE_SPIN)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_STATIC_MINUTES)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RADIO_ERX_HOUR)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_ERX_HOUR_TIME)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_ERX_HOUR_SPIN)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_STATIC_HOURS)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RADIO_ERX_DONTREMIND)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_ERX_REMIND_ME_GROUP)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);

			GetDlgItem(IDC_RADIO_RENEWAL_REMINDLOGIN)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RADIO_RENEWAL_REMINDMINUTE)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_MINUTE_TIME)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_MINUTE_SPIN)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_STATIC_MINUTES)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RADIO_RENEWAL_REMINDHOUR)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_HOUR_TIME)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_HOUR_SPIN)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_STATIC_HOURS)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RADIO_RENEWAL_DONTREMIND)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_RENEWAL_REMIND_ME_GROUP)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);

	}NxCatchAll(__FUNCTION__) 
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
// (s.tullis 2016-01-28 17:46) - PLID 68090
// Function to enable/disable the controls
void CPrescriptionQueueDlg::DisableEnableReminderControls(BOOL bEnable)
{
	try {
		GetDlgItem(IDC_ERX_MINUTE_TIME)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_ERX_MINUTE_SPIN)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_RENEWAL_MINUTE_TIME)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_RENEWAL_MINUTE_SPIN)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_RENEWAL_HOUR_TIME)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_RENEWAL_HOUR_SPIN)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_ERX_HOUR_TIME)->EnableWindow(bEnable ? TRUE : FALSE);
		GetDlgItem(IDC_ERX_HOUR_SPIN)->EnableWindow(bEnable ? TRUE : FALSE);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:45) - PLID 67965
// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnBnClickedRadioDontremind(SReminderControls &sControls)
{
	try {
		sControls.m_selection = ReminderOptions::Never;
		sControls.m_minuteBtn.SetCheck(BST_UNCHECKED);
		sControls.m_hourBtn.SetCheck(BST_UNCHECKED);
		sControls.m_neverBtn.SetCheck(BST_CHECKED);
		sControls.m_remindLoginBtn.SetCheck(BST_UNCHECKED);
		sControls.m_nxeditRemindHourTime.EnableWindow(FALSE);
		sControls.m_minuteSpin.EnableWindow(FALSE);
		sControls.m_hourSpin.EnableWindow(FALSE);
		sControls.m_nxeditRemindMinuteTime.EnableWindow(FALSE);
		sControls.m_noldHourValue = 0;
		sControls.m_noldMinuteValue = 0;
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRadioRemindlogin(SReminderControls &sControls)
{
	try {
		sControls.m_selection = ReminderOptions::Login;
		sControls.m_minuteBtn.SetCheck(BST_UNCHECKED);
		sControls.m_hourBtn.SetCheck(BST_UNCHECKED);
		sControls.m_neverBtn.SetCheck(BST_UNCHECKED);
		sControls.m_remindLoginBtn.SetCheck(BST_CHECKED);
		sControls.m_nxeditRemindHourTime.EnableWindow(FALSE);
		sControls.m_minuteSpin.EnableWindow(FALSE);
		sControls.m_hourSpin.EnableWindow(FALSE);
		sControls.m_nxeditRemindMinuteTime.EnableWindow(FALSE);
		sControls.m_noldHourValue = 0;
		sControls.m_noldMinuteValue = 0;
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRadioHour(SReminderControls &sControls)
{
	try {
		CString strValue;
		sControls.m_selection = ReminderOptions::Hour;
		sControls.m_minuteBtn.SetCheck(BST_UNCHECKED);
		sControls.m_hourBtn.SetCheck(BST_CHECKED);
		sControls.m_neverBtn.SetCheck(BST_UNCHECKED);
		sControls.m_remindLoginBtn.SetCheck(BST_UNCHECKED);
		sControls.m_nxeditRemindHourTime.EnableWindow(TRUE);
		sControls.m_minuteSpin.EnableWindow(FALSE);
		sControls.m_hourSpin.EnableWindow(TRUE);
		sControls.m_nxeditRemindMinuteTime.EnableWindow(FALSE);
		sControls.m_noldMinuteValue = 0;
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRadioMinutes(SReminderControls &sControls)
{
	try {
		CString strValue;
		sControls.m_selection = ReminderOptions::Minute;
		sControls.m_minuteBtn.SetCheck(BST_CHECKED);
		sControls.m_hourBtn.SetCheck(BST_UNCHECKED);
		sControls.m_neverBtn.SetCheck(BST_UNCHECKED);
		sControls.m_remindLoginBtn.SetCheck(BST_UNCHECKED);
		sControls.m_nxeditRemindHourTime.EnableWindow(FALSE);
		sControls.m_minuteSpin.EnableWindow(TRUE);
		sControls.m_hourSpin.EnableWindow(FALSE);
		sControls.m_nxeditRemindMinuteTime.EnableWindow(TRUE);
		sControls.m_noldHourValue = 0;
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:45) - PLID 67965
// fuction to control spin control.. should not be allowd negative values
void CPrescriptionQueueDlg::OnDeltaposMinuteSpin(NMHDR *pNMHDR, LRESULT *pResult, SReminderControls &sControls)
{
	try {
		CString strVal;
		NM_UPDOWN* pUpDown = (NM_UPDOWN*)pNMHDR;
		sControls.m_nxeditRemindMinuteTime.GetWindowText(strVal);
		int nMinVal = atoi(strVal);
		//Make sure no negative values and the Radio is selected
		if (((nMinVal - pUpDown->iDelta * 5) < 1) && sControls.m_minuteBtn.GetCheck())
		{
			return;
		}

		strVal = FormatString("%li", nMinVal - pUpDown->iDelta * 5);
		sControls.m_nxeditRemindMinuteTime.SetWindowText(strVal);
		// Kill the Timer if it's set and set a new one to save the input later
		KillTimer(IDT_SAVE_REMINDER);
		SetTimer(IDT_SAVE_REMINDER, 1000,NULL);

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:45) - PLID 67965
// function to control spin control.. should not be allowed negative values
void CPrescriptionQueueDlg::OnDeltaposHourSpin(NMHDR *pNMHDR, LRESULT *pResult, SReminderControls &sControls)
{
	try {
		CString strVal;
		NM_UPDOWN* pUpDown = (NM_UPDOWN*)pNMHDR;
		sControls.m_nxeditRemindHourTime.GetWindowText(strVal);
		int nHourVal = atoi(strVal);
		//Make sure no negative values and the Radio is selected
		if (((nHourVal - pUpDown->iDelta * 1) < 1) && sControls.m_hourBtn.GetCheck())
		{
			return;
		}

		strVal = FormatString("%li", nHourVal - pUpDown->iDelta * 1);
		sControls.m_nxeditRemindHourTime.SetWindowText(strVal);
		// Kill the Timer if it's set and set a new one to save the input later
		KillTimer(IDT_SAVE_REMINDER);
		SetTimer(IDT_SAVE_REMINDER, 1000,NULL);

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRenewalsRadioMinutes()
{
	OnBnClickedRadioMinutes(m_Renewals);
	SaveReminder(m_Renewals, false);
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRenewalsRadioHour()
{
	OnBnClickedRadioHour(m_Renewals);
	SaveReminder(m_Renewals, false);
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRenewalsRadioLogin()
{
	OnBnClickedRadioRemindlogin(m_Renewals);
	SaveReminder(m_Renewals,false);
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRenewalsRadioDontRemind()
{
	OnBnClickedRadioDontremind(m_Renewals);
	SaveReminder(m_Renewals,false);
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRenewalsMinuteSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposMinuteSpin(pNMHDR, pResult, m_Renewals);
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnBnClickedRenewalsHourSpin(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnDeltaposHourSpin(pNMHDR, pResult, m_Renewals);
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
// (s.tullis 2016-01-28 17:46) - PLID 68090
// Save the Reminder Control Values
void CPrescriptionQueueDlg::SaveReminder(SReminderControls &sControls, bool bPrescriptionNeedingAttention /*= true*/)
{
	try {
		// don't try to save if we're loading reminders
		if (m_bLoadingReminders || !CheckWarnInvalidInput(sControls))
		{
			return;
		}
		sControls.m_bChanged = false;
		CString strPropSelect = bPrescriptionNeedingAttention ? "PrescriptionNeedingAttentionReminderSelection" : "RenewalsReminderSelection";
		CString strPropValue = bPrescriptionNeedingAttention ? "PrescriptionNeedingAttentionReminderValue" : "RenewalsReminderValue";
		CString strValue = "";
		switch (sControls.m_selection)
		{
			case ReminderOptions::Hour:
			{
				sControls.m_nxeditRemindHourTime.GetWindowText(strValue);
				if (sControls.m_noldHourValue != atoi(strValue)) {
					sControls.m_bChanged = true;
					sControls.m_noldHourValue = atoi(strValue);
					SetRemotePropertyInt(strPropSelect, (int)ReminderOptions::Hour, 0, GetCurrentUserName());
					SetRemotePropertyInt(strPropValue, atoi(strValue), 0, GetCurrentUserName());
				}
				
			}
				break;
			case ReminderOptions::Minute:
			{
				sControls.m_nxeditRemindMinuteTime.GetWindowText(strValue);
				if (sControls.m_noldMinuteValue != atoi(strValue)) {
					sControls.m_bChanged = true;
					sControls.m_noldMinuteValue = atoi(strValue);
					SetRemotePropertyInt(strPropSelect, (int)ReminderOptions::Minute, 0, GetCurrentUserName());
					SetRemotePropertyInt(strPropValue, atoi(strValue), 0, GetCurrentUserName());
				}
			}
				break;
			case ReminderOptions::Login:
			{
				if (sControls.m_Oldselection != ReminderOptions::Login) {
					SetRemotePropertyInt(strPropSelect, (int)ReminderOptions::Login, 0, GetCurrentUserName());
					SetRemotePropertyInt(strPropValue, 0, 0, GetCurrentUserName());
					sControls.m_Oldselection = ReminderOptions::Login;
					sControls.m_bChanged = true;
				}
		
			}
				break;
			case ReminderOptions::Never:
			{
				if (sControls.m_Oldselection != ReminderOptions::Never) {
					SetRemotePropertyInt(strPropSelect, (int)ReminderOptions::Never, 0, GetCurrentUserName());
					SetRemotePropertyInt(strPropValue, 0, 0, GetCurrentUserName());
					sControls.m_Oldselection = ReminderOptions::Never;
					sControls.m_bChanged = true;
				}
			}
				break;
		}

		if ((sControls.m_bChanged))
		{
			if (bPrescriptionNeedingAttention) {
				GetMainFrame()->ResetRxAttentionTimer();
			}
			else
			{
				GetMainFrame()->ResetRenewalTimer();
			}
			
		}

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:45) - PLID 67965
// Load the Reminder Control Values
void CPrescriptionQueueDlg::LoadReminder(SReminderControls &sControls, bool bPrescriptionNeedingAttention /*= true*/)
{
	m_bLoadingReminders = true;
	try {
		CString strPropSelect = bPrescriptionNeedingAttention ? "PrescriptionNeedingAttentionReminderSelection" : "RenewalsReminderSelection";
		CString strPropValue = bPrescriptionNeedingAttention ? "PrescriptionNeedingAttentionReminderValue" : "RenewalsReminderValue";
		ReminderOptions radioselect = (ReminderOptions) GetRemotePropertyInt(strPropSelect, (int)ReminderOptions::Hour, 0, GetCurrentUserName());
		CString strValue = "";
		// load defaults for hour / minute selections
		sControls.m_nxeditRemindMinuteTime.SetWindowText("5");
		sControls.m_nxeditRemindHourTime.SetWindowText("1");
		switch (radioselect)
		{
			case ReminderOptions::Hour:
			{
				int nHour = GetRemotePropertyInt(strPropValue, 1, 0, GetCurrentUserName());
				sControls.m_noldHourValue = nHour;
				sControls.m_Oldselection = ReminderOptions::Hour;
				strValue = FormatString("%li",nHour);
				sControls.m_nxeditRemindHourTime.SetWindowText(strValue);
				OnBnClickedRadioHour(sControls);
			}
			break;
			case ReminderOptions::Minute:
			{
				int nMinutes = GetRemotePropertyInt(strPropValue, 5, 0, GetCurrentUserName());
				sControls.m_noldMinuteValue = nMinutes;
				sControls.m_Oldselection = ReminderOptions::Minute;
				strValue = FormatString("%li", nMinutes);
				sControls.m_nxeditRemindMinuteTime.SetWindowText(strValue);
				OnBnClickedRadioMinutes(sControls);
			}
			break;
			case ReminderOptions::Login:
			{
				sControls.m_Oldselection = ReminderOptions::Login;
				OnBnClickedRadioRemindlogin(sControls);
			}
			break;
			case ReminderOptions::Never:
			{
				sControls.m_Oldselection = ReminderOptions::Never;
				OnBnClickedRadioDontremind(sControls);
			}
			break;
		}
	}NxCatchAll(__FUNCTION__)
	m_bLoadingReminders = false;
}
// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnEnChangeRenewalMinuteTime()
{
	try {
		SaveReminder(m_Renewals, false);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-01-28 17:45) - PLID 67965
void CPrescriptionQueueDlg::OnEnChangeRenewalHourTime()
{
	try {
		SaveReminder(m_Renewals, false);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnEnChangeErxMinuteTime()
{
	try {
		SaveReminder(m_PrescriptionNeedingAttention);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-01-28 17:46) - PLID 68090
void CPrescriptionQueueDlg::OnEnChangeErxHourTime()
{
	try {
		SaveReminder(m_PrescriptionNeedingAttention);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-28 17:45) - PLID 67965
// (s.tullis 2016-01-28 17:46) - PLID 68090
BOOL CPrescriptionQueueDlg::CheckWarnInvalidInput(SReminderControls &sControls)
{
	BOOL bContinue = FALSE;
	try {
		CString strValue;
		switch (sControls.m_selection)
		{
		case ReminderOptions::Hour:
		{
			sControls.m_nxeditRemindHourTime.GetWindowText(strValue);
			if (atoi(strValue) < 1) {
				MessageBox("The hours reminder must have a value greater than zero. Please enter another value to save the reminder.", NULL, MB_OK | MB_ICONEXCLAMATION);
				sControls.m_nxeditRemindHourTime.SetFocus();
			}
			else {
				bContinue = TRUE;
			}

		}
		break;
		case ReminderOptions::Minute:
		{
			sControls.m_nxeditRemindMinuteTime.GetWindowText(strValue);
			if (atoi(strValue) < 1) {
				MessageBox("The minutes reminder must have a value greater than zero. Please enter another value to save the reminder.", NULL, MB_OK | MB_ICONEXCLAMATION);
				sControls.m_nxeditRemindMinuteTime.SetFocus();
			}
			else {
				bContinue = TRUE;
			}

		}
		break;
		default:
		{
			bContinue = TRUE;
		}
		break;
		}
	}NxCatchAll(__FUNCTION__)
	return	bContinue;
}