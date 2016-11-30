#if !defined(AFX_RESENTRYDLG_H__F3C3BAE3_BE9A_11D1_8041_00104B2FE914__INCLUDED_)
#define AFX_RESENTRYDLG_H__F3C3BAE3_BE9A_11D1_8041_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "client.h"
#include "SchedulerView.h"

// (c.haag 2010-02-04 12:03) - PLID 37221 - Accesses to reservation objects should be done through the
// CReservation class
//#import "SingleDay.tlb" rename("Delete", "DeleteRes")
#include "Reservation.h"

#include "NewPatientAddInsuredDlg.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace SINGLEDAYLib;
// using namespace NXTIMELib;

class CSchedulerView;
struct APPT_INFO;
struct SchedAuditItems;
class SchedulerMixRule;
struct InsuranceInfo;
typedef CMap<long, long, InsuranceInfo*, InsuranceInfo*> AppointmentInsuranceMap;
enum SchedulerInsurancePlacements;
class SelectedFFASlot;

enum AppointmentTypeColumns
{
	atcID = 0,
	atcName,
	atcCategory,
	atcDefaultDuration,
	atcDefaultArrivalMins,
	atcColor // (a.walling 2008-05-08 10:48) - PLID 29786 - This was missing from this enum
};

enum AppointmentPurposeColumns
{
	apcID = 0,
	apcName,
	apcArrivalPrepMinutes
};

//(e.lally 2008-06-20) PLID 29500 - added enum for No Show combo columns
enum NoShowColumns{
	nscID =0,
	nscSymbolAndName,
	nscColor,
	nscPlainName,
};

// (j.jones 2010-01-04 10:34) - PLID 32935 - used to tell ValidateSaveClose what action
// we are performing after the save completes
enum AppointmentSaveAction {
	asaNormal = 0,
	asaCreateInvAllocation,
	asaCreateInvOrder,
	asaCreateCaseHistory,
	asaOpenCaseHistory,
	asaCreateBill,
	// (r.gonet 09/20/2013) - PLID 58416 - Added
	asaViewMedicationHistory,
};


// (j.jones 2014-11-14 11:00) - PLID 64169 - moved the InsurancePlacements enum
// to GlobalSchedUtils

/////////////////////////////////////////////////////////////////////////////
// CResEntryDlg dialog

class CResEntryDlg : public CNxDialog
{
// Construction
public:
	BOOL MoveToRes(long ResID);
	void MakeNewRes();
	void ZoomResNewAppointment(LPDISPATCH theRes, IN const long nDefaultResourceID, IN const COleDateTime &dtDefaultDate);
	void ZoomResExistingAppointment(LPDISPATCH theRes);
	// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
	// (a.walling 2010-12-30 11:18) - PLID 40081 - Use datalist2 to avoid row index race conditions
	NXDATALIST2Lib::IRowSettingsPtr UpdatePatient(long nID);
	
	// (j.luckoski 2012-04-26 09:26) - PLID 11597 - Variables for cancelled appts
	void DisableMenuForCancelled(BOOL bIsEnable);
	long m_nDateRange;
	long m_nCancelledAppt;
	long m_nCancelColor;
	bool m_bIsCancelled;
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
	bool m_bIsMixRuleOverridden;

	// (a.walling 2010-06-15 07:32) - PLID 30856
	void FillApptInfo(APPT_INFO& aptInfo);

	// CAH 5/9/2003 - These four functions are only called in
	// CResEntryDlg::ValidateData
	
	
	long GetCurPatientID();
	void SetCurPatient(long nNewPatientID);
	
	void ReflectCurPatient();
	void ReflectCurPatientNotFound();
	void ReflectCurPatientFailure();
	
	BOOL m_bIsCurPatientSet;
	long m_nCurPatientID;

	long m_nAptInitStatus;
	// (c.haag 2009-01-19 12:00) - PLID 32712 - Track the initial purposes of the appointment
	CDWordArray m_adwAptInitPurposes;
	// (c.haag 2010-03-08 16:47) - PLID 37326 - Other initial fields
	long m_nAptInitType;
	CDWordArray m_adwAptInitResources;
	COleDateTime m_dtAptInitStartTime;
	COleDateTime m_dtAptInitEndTime;


	BOOL m_bIsExtraInfoDirty;

	void EnableGoToPatient(BOOL bEnabled);
	void DisableSaveCreateCaseHistory();
	void EnableCancelAppointmentMenuOption(BOOL bEnabled);
	void ToggleSaveCreateCaseHistory(BOOL bSaveNew);
	BOOL DoesCaseHistoryExist(long nResID);

	// (j.jones 2007-11-21 10:59) - PLID 28147 - added ability to create an inventory allocation,
	// but this function will disable it if they don't have inventory or create permissions
	void DisableSaveCreateInvAllocation();

	// (j.jones 2008-03-18 13:54) - PLID 29309 - added ability to create an inventory order,
	// but this function will disable it if they don't have advanced inventory or create permissions
	void DisableSaveCreateInvOrder();

	// (r.gonet 09/19/2013) - PLID 58416 - Hides the Save and View Medication History menu item in the Actions menu.
	void DisableSaveViewMedicationHistory();

	long GetCurAptPurposeID();
	
protected:
	// (c.haag 2010-08-27 11:38) - PLID 40108 - List of functions that have "Reserved" this reservation so
	// as to prevent this object from getting destroyed during table checker processing. This can be non-empty
	// if an appointment is being opened, for example.
	CStringArray m_astrReservingFunctions;

	// (c.haag 2010-10-11 11:41) - PLID 40108 - Set to TRUE when IsReserved is called where the intent was
	// to update the view. We need this in cases where the view needs to be refreshed after the reservation is
	// no longer reserved.
	BOOL m_bWantedUpdateViewWhileReserved;

public:
	// (c.haag 2010-08-27 11:38) - PLID 40108 - This function will "reserve" the resentry dialog. In the current
	// implementation, we only do it to prevent scheduler refreshes when a reservation is/may be opening.
	void Reserve(const CString& strReservingFunction);
	// (c.haag 2010-08-27 11:38) - PLID 40108 - This function will "unreserve" the resentry dialog. In the current
	// implementation, we only do it to prevent scheduler refreshes when a reservation is/may be opening.
	void Unreserve(const CString& strUnreservingFunction);
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Returns TRUE if this object is "reserved" for use. In this
	// implementation, it likely means that the reservation is/may be opening.
	BOOL IsReserved(BOOL bWantsToUpdateView);
	// (c.haag 2010-10-11 12:05) - PLID 40108 - Returns TRUE if IsReserved was called while this object
	// was reserved and the caller intended to update the view
	BOOL WantedUpdateViewWhileReserved() const;

	//TES 7/29/03: These accessor functions a.) try to get info out of the datalist (since that's what the 
	//user sees), and b.) if the datalist doesn't have a row selected, but does have ComboBoxText set, which
	//is equal to m_strAptTypeName (and m_bAptTypeSet), then returns stored info for that type.
	long GetCurAptTypeID();
	long GetCurStatusID(); // (j.luckoski 2012-06-26 12:27) - PLID 11597
	CString GetCurAptTypeName();
	short GetCurAptTypeCategory();
	long GetCurAptTypeDuration();
	void SetCurAptType(long nTypeID);
	void SetCurAptType_PostRequery(long nTypeID);

	long GetCurLocationID();
	CString GetCurLocationName();
	void SetCurLocation(long nNewLocID, const CString &strNewLocName);
	
	void ReflectCurLocation();
	void ReflectCurLocationInactive();
	void ReflectCurLocationFailure();
	
	void SetCurRefPhys(long nNewRefPhysID); // (j.gruber 2013-01-07 16:11) - PLID 54414
	void ReflectCurRefPhysFailure(); // (j.gruber 2013-01-07 16:11) - PLID 54414
	void ReflectCurRefPhysInactive(); // (j.gruber 2013-01-07 16:11) - PLID 54414
	void ReflectCurRefPhys(); // (j.gruber 2013-01-07 16:11) - PLID 54414
	long GetCurRefPhysID(); // (j.gruber 2013-01-07 16:11) - PLID 54414
	void HideRefPhysInfo(); // (j.gruber 2013-01-07 16:11) - PLID 54414

	void AutoSelectTypePurposeByPatient();

	BOOL m_bIsCurLocationSet;
	long m_nCurLocationID;
	// (j.jones 2014-12-02 08:55) - PLID 64178 - added initial location ID
	long m_nInitLocationID;
	CString m_strCurLocationName;
	CString m_strResourceString;
	CString GenerateResourceString();
	CString GetResourceString();

	// (j.gruber 2013-01-07 16:13) - PLID 54414
	BOOL m_bIsCurRefPhysSet;
	long m_nCurRefPhysID;
	CString m_strCurRefPhysName;

	//TES 3/16/2004: This will tell the caller whether it's OK to make a change that might invalidate our ReservationPtr.
	bool AllowChangeView();

	~CResEntryDlg();

	// CAH 5/9/2003 - These are variables used by ::ValidateData to safely
	// retrieve what the selected value of a datalist should be,
	// even if it's still requerying or TrySetSelByColumn isn't done.
	//
	// Possible values:
	// VT_NULL: No selection has ever been attempted
	// VT_EMPTY: TrySetSel has been called and completed, so look
	//				to the control for a result.
	// All others: TrySetSel has been called but not completed, so
	//				this variant has what we tried to select.
protected:
	class CResDLSelection
	{
	protected:
		BOOL m_bValid;
		COleVariant m_v;
	public:
		CResDLSelection();
		virtual ~CResDLSelection();

		void SetValue(const COleVariant& vValue);
		COleVariant GetValue();
		COleVariant GetSafeSelection();

		void SetValid(BOOL bValid);
		BOOL IsValid();		
	};
//	CResDLSelection m_vPatientIDSel;

protected:
	// (c.haag 2009-01-14 09:54) - PLID 32712 - This function checks whether this is a surgery appointment; and if so,
	// looks for the most recent consult for that patient, and ensures it has the same master procedures this appt has.
	void SyncSurgeryPurposesWithConsult(IN const long nPatID, const COleDateTime& dtDate, const COleDateTime& dtStart);
public:
	void CheckCreateCaseHistory(IN const long nPatID, IN const COleDateTime &dtSurgeryDate, const long nAppointmentID);
	void RefreshDefaultDurationList();
	void FillDefaultEndTime();
	void FillDefaultArrivalTime();

// Dialog Data
	// (a.walling 2008-05-13 10:41) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CResEntryDlg)
	enum { IDD = IDD_RES_ENTRY_DLG };
	CButton	m_btnBold8;
	CButton	m_btnBold7;
	CButton	m_btnBold6;
	CButton	m_btnBold5;
	CButton	m_btnBold4;
	CButton	m_btnBold3;
	CButton	m_btnBold2;
	CButton	m_btnBold1;
	CButton	m_btnPin;
	CString	m_Notes;
	CDateTimePicker m_date;
	CDateTimePicker m_dateEventStart;
	CDateTimePicker m_dateEventEnd;
	CNxEdit	m_nxeditNotesBox;
	CNxEdit	m_nxeditResPatientIdBox;
	CNxEdit	m_nxeditResPatientHomePhoneBox;
	CNxEdit	m_nxeditResPatientWorkPhoneBox;
	CNxEdit	m_nxeditResPatientBirthDateBox;
	CNxEdit	m_nxeditResPatientCellPhoneBox;
	CNxEdit	m_nxeditResPatientPreferredContactBox;
	CNxEdit	m_nxeditResPtbalBox;
	CNxEdit	m_nxeditResInsbalBox;
	CNxStatic	m_nxstaticMoveup;
	CNxStatic   m_nxstaticRefPhysLabel; // (j.gruber 2013-01-07 16:56) - PLID 54414
	CNxStatic	m_nxstaticConfirmed;
	CNxStatic	m_nxstaticConfirmed3;
	CNxStatic	m_nxstaticDate;
	CNxStatic	m_nxstaticStartDate;
	CNxStatic	m_nxstaticNotApplicable;
	CNxStatic	m_nxstaticStart;
	CNxStatic	m_nxstaticEnd;
	CNxStatic	m_nxstaticArrival;
	CNxStatic	m_nxstaticResource;
	CNxStatic	m_nxstaticListofpurposes;
	CNxStatic	m_nxstaticEndDate;
	CNxStatic	m_nxstaticExtraInfoOffsetLabel;
	CNxStatic	m_nxstaticRequestedResourceLabel;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnGotoPatient;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnCancel;
	NxButton	m_btnMoreInfoGroupbox;
	NxButton	m_checkPatientID;
	NxButton	m_checkHomePhone;
	NxButton	m_checkWorkPhone;
	NxButton	m_checkBirthDate;
	NxButton	m_checkCellPhone;
	NxButton	m_checkPreferredContact;
	NxButton	m_checkPatBal;
	NxButton	m_checkInsBal;
	CNxIconButton m_btnGotoInsurance;
	// (b.savon 2012-02-28 11:37) - PLID 48441 - Create recall from resentrydlg
	CNxIconButton m_btnCreateRecall;
	// (j.gruber 2012-08-08 11:06) - PLID 51830
	CNxIconButton m_btnMoreInsurance;
	//}}AFX_DATA

	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);

	CResEntryDlg(CWnd* pParent, UINT nIDTemplate, CSchedulerView *pSchedViewParent);   // standard constructor; suggested parameters in comments

	NXDATALIST2Lib::_DNxDataListPtr m_dlAptPatient; // (a.walling 2010-12-30 10:40) - PLID 40081 - Use datalist2 to avoid row index race conditions

	NXDATALISTLib::_DNxDataListPtr m_dlAptPurpose;
	NXDATALISTLib::_DNxDataListPtr m_dlAptType;

	// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
	NXDATALIST2Lib::_DNxDataListPtr m_dlAptLocation;
	NXDATALISTLib::_DNxDataListPtr m_dlAptResource;
	NXDATALISTLib::_DNxDataListPtr m_dlRequestedResource;

	NXDATALISTLib::_DNxDataListPtr m_dlNoShow;

	//TES 12/17/2008 - PLID 32479 - Changed to datalist2s.
	NXDATALIST2Lib::_DNxDataListPtr m_pMoveUpCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pConfirmedCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pReadyCombo;

	// (j.gruber 2013-01-07 16:12) - PLID 54414
	NXDATALIST2Lib::_DNxDataListPtr m_pRefPhysList;

	NXTIMELib::_DNxTimePtr m_nxtArrival;
	NXTIMELib::_DNxTimePtr m_nxtStart;
	NXTIMELib::_DNxTimePtr m_nxtEnd;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	COleDateTime m_dtArrivalTime;
	COleDateTime m_dtStartTime;
	COleDateTime m_dtEndTime;
	void PreSave();
	CString GetExtraInfo(int nOrdinal);
	// (a.walling 2010-06-14 14:33) - PLID 23560 - Resource Sets
	// (j.jones 2014-12-02 15:55) - PLID 64178 - if a mix rule is overridden, it is passed up to the caller
	BOOL ValidateDataWithResourceSets(BOOL bCheckRules, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT shared_ptr<SelectedFFASlot> &pSelectedFFASlot);
	
	struct ResourceSet {
		ResourceSet(int nResourceSetID) : m_nResourceSetID(nResourceSetID), m_pNext(NULL) {};

		int m_nResourceSetID;
		CDWordArray m_arResources;
		ResourceSet* m_pNext;
	};

	BOOL FitResourceSets(const CDWordArray& dwaBaseResources, CDWordArray& dwaFinalResources, ResourceSet* pResourceSet, int nRuleTolerance, bool& bIgnoredWarnings);
	bool CheckResources(const CDWordArray& dwaResources, int nRuleTolerance);

	// (a.walling 2010-06-14 14:33) - PLID 23560 - Resource Sets
	// (j.jones 2014-12-02 15:55) - PLID 64178 - if a mix rule is overridden, it is passed up to the caller
	BOOL ValidateDataNoResourceSets(BOOL bCheckRules, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT shared_ptr<SelectedFFASlot> &pSelectedFFASlot);
	ADODB::_RecordsetPtr CreatePatientInfoRecordset();
	void ApplyToExtraInfo(ADODB::_RecordsetPtr prsPatient = NULL);
	//TES 11/5.2003: This function does everything that needs to be done when the patient changes, 
	//including calling ApplyToExtraInfo (which now ONLY fills in the extra info boxes).
	// (j.jones 2014-11-14 10:30) - PLID 64116 - now takes in a boolean to state whether
	// the user has changed the patient on the appointment
	void ApplyCurrentPatient(bool bPatientChanged);
	bool m_bExtraInfo;
	void ShowExtraInfo(bool bShow);
	// (j.luckoski 2012-06-20 11:28) - PLID 11597 - added bIsCancelled
	// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
	int AddTextItem(CString &strFormat, CString &strAddTo, int iPos, bool bIsCancelled, bool bIsMixRuleOverridden);
	void RevertActive();
	void HandleCancelRestore(); // (j.luckoski 2012-06-26 12:26) - PLID 11597
	void ApplyToResBox();
	void Activate(bool toBeActive = true);
	long m_ActiveSetID;
	OLE_COLOR m_ActiveBorderColor;
	OLE_COLOR m_OldBorderColor;
	OLE_COLOR m_ActiveStatusColor;
	OLE_COLOR m_OldStatusColor;
	CString m_OldText;
	OLE_COLOR ObtainActiveBorderColor();
	OLE_COLOR ObtainActiveStatusColor(); // (j.luckoski 2012-06-26 12:27) - PLID 11597
	OLE_COLOR GetActiveBorderColor();
	OLE_COLOR GetActiveStatusColor();
	
	CRect m_rcProcedures;

	// (j.gruber 2012-07-26 14:23) - PLID 51830 - insurnace information
	CRect m_rcPriIns;
	CRect m_rcSecIns;
	// (j.gruber 2012-08-08 10:50) - PLID 51882 - handle greying out more box
	CRect m_rcInsMore;
	
	void ClearInsurancePlacements(BOOL bClearOriginals);
	long GetInsuranceID(SchedulerInsurancePlacements insPlace);
	CString GetInsuranceName(SchedulerInsurancePlacements insPlace);
	void SetInsurance(long nPlacement, long nInsuredPartyID, CString strInsCoName, CString strRespType, bool bErrorOnExistence = false);
	
	AppointmentInsuranceMap m_mapInsPlacements;
	AppointmentInsuranceMap m_mapOrigIns;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsCatList;
	
	// (j.gruber 2012-08-01 10:37) - PLID 51885 - this is the patient information we'll need  to send to the new ins party dlg
	CNewPatientInsuredParty m_patientInsInfo;

	CDWordArray m_adwInactivePtInActiveList;

	// (c.haag 2005-06-29 12:40) - PLID 6400 - Inactive resources list
	CDWordArray m_adwInactiveResIDs;
	CStringArray m_astrInactiveResNames;

	//TES 12/17/2008 - PLID 32479 - These are no longer DDX'd with MS Combos
	bool m_bMoveUp;
	long m_nConfirmed;
	bool m_bReady;

	bool GetModifiedFlag();
	void SetModifiedFlag(bool bModified = true);
	// (j.jones 2010-01-04 10:53) - PLID 32935 - added parameter to disable checking for required allocations
	void SaveRes(bool bAutoDelete = true, bool bCheckForAllocations = true);
	bool m_bCheckActive;
	bool m_bRevertActive; // (j.luckoski 2012-06-26 12:27) - PLID 11597
	CString GetActiveText();
	CString GetActiveText(bool bIsCancelled); // (j.luckoski 2012-06-20 11:29) - PLID 11597 - Add vaue that stores the bool.
	void SetComboValue(NXDATALISTLib::_DNxDataListPtr &cboCombo, const _variant_t &varValue, bool bDropDown = true);
	void SetComboRecordNumber(NXDATALISTLib::_DNxDataListPtr &cboCombo, long nPos, bool bDropDown = true);
	BOOL IsEvent(); // CAH 4/17: Returns TRUE if this is an event

	// (j.gruber 2013-01-07 15:11) - PLID 54414 - added ref phys
	// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
	CTableChecker m_apttypeChecker, m_aptpurposeChecker, m_ResourceChecker, m_locationChecker, m_showstateChecker, m_refPhysChecker;
	// (b.cardillo 2006-07-06 17:56) - PLID 21342 - Now instead of 4 table-checkers, we just have one 
	// boolean, which is set by CSchedulerView if any of the four general1 custom field names changes.
	BOOL m_bG1CustomFieldNameChanged;

	// These are like a pdl->Requery() except they also add the special entries (like "multiple resources" for the resource datalist)
	void DoRequeryPatientList();
	void DoRequeryResourcesDatalist();


	void RequeryPersons();
	void RequeryAptTypes();
	void RequeryAptPurposes(_variant_t *pVarTrySetSel = NULL);

	// (j.gruber 2013-01-07 16:13) - PLID 54414
	void RequeryReferringPhys();

	void RequeryLocations(BOOL bKeepCurLocationSelected);
	void RequeryResources();
	void RequeryShowStates();
	void RefreshResourceCombo();
	void RefreshPurposeCombo();
	// (r.gonet 06/05/2012) - PLID 49059 - Reflects the location template
	void RefreshLocationTemplate();
	void DoClickHyperlink(UINT nFlags, CPoint point);
	void DrawResourceHyperlinkList(CDC *pdc);
	void DrawProcedureHyperlinkList(CDC *pdc);

	void OnBlockTimeCatSelected(BOOL bIsBlockTime);

	void EnsurePurposeComboVisibility();
	void EnsureResourceComboVisibility();

	// (j.gruber 2012-07-26 14:24) - PLID 51830 - insurance information
	void ChooseInsParty(SchedulerInsurancePlacements placement);
	void DrawPrimaryInsuranceText(CDC *pdc);
	void DrawSecondaryInsuranceText(CDC *pdc);

	SchedAuditItems &m_schedAuditItems;

	CReservation m_Res;

	// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
	// action is being performed after saving the appointment
	BOOL ValidateSaveClose(AppointmentSaveAction asaAction);

private:
	// (c.haag 2015-11-12) - PLID 67577 - All functions listed here were created so that events which
	// used to call other events can now instead invoke the business logic directly. This prevents
	// confusion with figuring out where exceptions were actually thrown from.

	void HandleAptPurposeComboSelection(long nCurSel);
	void HandleAptResourceComboSelection(long nCurSel);
	void HandleResEntryPinChange();
	void HandleSaveAndCloseReservation();
	void HandleCancelAndCloseReservation();

protected:

	CDWordArray m_adwPurposeID;

	// (j.jones 2008-02-21 10:46) - PLID 29048 - added TryAddPurposeIDToAppt which will
	// add the purpose to the appt. only if it doesn't already exist
	BOOL TryAddPurposeIDToAppt(long nNewPurposeID);

	COleDateTime PatientComboRefresh;
	COleDateTime m_ActiveDate;
	bool m_ExtraActive;
	bool m_DoSave;
	bool m_PromptForCaseHistory;
	bool m_CheckboxesUpdated; // CAH 10/26/01: True if any of the checkboxes were updated
							// when the dialog was up and running
	long m_nWarnedForPatientID = -1;
	long m_ResID;
	//long m_TemplateItemID;	// (c.haag 2006-12-05 09:31) - PLID 23666 - If this is not -1, then
							// this dialog represents a new appointment that came from a template
							// line item block
	//TES 10/27/2010 - PLID 40868 - Replaced m_TemplateItemID with m_nCalculatedTemplateItemID and m_nInitialTemplateItemID.
	// m_nInitialTemplateItemID is the precision template that was clicked on originally for new templates (this is essentially
	// the same as m_TemplateItemID); m_nCalculatedTemplateItemID is re-calculated as needed based on the time/type/purpose of the appointment.
	long m_nCalculatedTemplateItemID;
	long m_nInitialTemplateItemID;
	// (r.gonet 06/05/2012) - PLID 49059 - Color of the location template
	long m_nLocationTemplateColor;

	//TES 10/27/2010 - PLID 40868 - Determines the precision template block associated with the current appointment, if any.
	long GetTemplateItemID();
	//TES 10/27/2010 - PLID 40868 - Resets the calculated value for the TemplateItemID
	void RefreshTemplateItemID();

	bool m_NewRes;

	// (j.jones 2007-11-21 12:22) - PLID 28147 - needed to track the new reservation ID
	// for patient allocations
	long m_nNewResID;

	unsigned long m_nOldStamp;
	bool m_bAutoUpdate;

	bool m_bOldMoveUp; // (c.haag 2010-09-08 10:52) - PLID 37734

	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;

	bool m_bValidAptTypeStored;
	long m_nAptTypeID;
	CString m_strAptTypeName;
	short m_nAptTypeCategory;
	long m_nAptTypeDuration;
	BOOL m_bShowArrivalTime;

	long m_nTemplateItemAptTypeID;

	//TES 2004-01-30: Store this, so  we can put it in the "extra info."
	CString m_strInputName;
	//TES 11/12/2008 - PLID 11465 - Store the input date, so we can put it in the "extra info."
	COleDateTime m_dtInput;
	COleDateTime m_dtLastLM; // (c.haag 2004-02-26 11:29) - Same concept with last LM date
	COleDateTime m_dtModified;
	CString m_strModifiedLogin;

	// Variables used to track if the appointment has changed
	bool m_bModified;		// true if the appointment was modified in this session
	

	CString GetExtraFieldText(ADODB::FieldsPtr fPatInfo, ExtraFields efField);
	CString FormatTextForReadset(ExtraFields efField, const CString &strText);

	BOOL TryCorrectingPatientSelResultFailure();
	bool WarnIfPatientInReschedulingQueue(); // (a.walling 2015-01-27 12:14) - PLID 64416 - Warn if patient in rescheduler queue
	void WarnIfInactivePatientSelected();
	void HighlightSheetTimeColumn(BOOL bHighlight);
	BOOL EnsureValidEndTime(COleDateTime &dtEndTime);
	BOOL EnsureValidArrivalTime(COleDateTime &dtArrivalTime);

	//TES 4/20/2005 - Pulls from the appropriate date control based on whether we're an event or not.
	COleDateTime GetDate();

	void ShowArrivalTime(BOOL bShow);

	void TryAutoFillPurposes();

public:
	void RepopulateExtraInfoCombos();
	BOOL IsCurResourceListMultiResource();
	const CDWordArray &GetCurResourceIDList();
	long GetCurResourceID_ExpectSingle();
	long GetNoShowComboSel();

	// (a.walling 2010-06-14 14:21) - PLID 23560 - Resource Sets
	void ChooseMultipleResources(bool bFromUser, CDWordArray& dwaResources);

protected:
	//(e.lally 2008-06-20) PLID 29500 - Function to grab the plain text (no symbol) name of the no show state
		//from the cached combo box
	CString GetNoShowComboSelName();

protected:
	CDWordArray m_arydwSelResourceIDs;

protected:
	CSchedulerView *m_pSchedulerView;

protected:
	void ZoomResInternal(LPDISPATCH theRes);
	// (b.savon 2012-06-04 16:52) - PLID 49860 - Center dialog.
	void CenterDialog();

protected:
	// Close the dialog (TODO: right now, this is just one function that looks to the 
	// m_DoSave to decide whether to save or cancel.  Obviously it would be nicer to 
	// have separate save and cancel operations.)
	// (j.jones 2010-01-04 10:53) - PLID 32935 - added parameter to disable checking for required allocations
	void CleanUp(bool bCheckForAllocations = true);
	
	// Delete the m_Res object from the screen (this means decrement our reference, but 
	// ALSO tell the IReservation object to take itself out of whatever Singleday it's in)
	void KillThisResObject(BOOL bDetach);

	// The proper way to dismiss this dialog, which can be only done from the inside 
	// right now (by the user clicking a save or cancel button or whatever)
	void Dismiss();

	// Assigns the Where clause to the patients dropdown based on the settings in
	// the preferences dialog.
	void SetPtComboWhereClause();

	//TES 1/18/2010 - PLID 36895 - Gets the appropriate FROM clause for our patient list, which will hide demographic information for
	// any patients which are blocked.
	CString GetPatientFromClause();

protected:
	// Makes the reservation box look simliar to that of a template block
	void RevertResAppearanceToTemplateItem();

protected:
	// (c.haag 2006-12-07 16:28) - PLID 23767 - Calculates what type this reservation should
	// be if it was newly created from a template item
	void CalculateDefaultAptTypeByTemplateItemID();

	// (z.manning 2008-11-24 11:20) - PLID 31130 - Added a utility function to save and then 
	// open the patients module to the given tab or the default tab if -1 is specified.
	void SaveAndGotoPatient(short nPatientTab);

	//(a.wilson 2012-2-29) PLID 48420 - function that handles the linked of recalls if the current patient has any unlinked.
	void CheckForRecalls();

protected:
	friend class CKeepDialogActive;
	void IncActiveCount();
	void DecActiveCount();

	long m_nKeepActiveCount;

	CKeepDialogActive *m_Pinned;
	CKeepDialogActive *m_Modified;

protected:

	// Generated message map functions
	// (j.jones 2007-11-21 10:59) - PLID 28147 - added ability to create an inventory allocation
	// (j.jones 2008-03-18 13:51) - PLID 29309 - added ability to create an inventory order
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use notify event handlers
	// (j.jones 2008-06-24 10:05) - PLID 30455 - added ability to create a bill
	// (z.manning 2008-11-24 10:10) - PLID 31130 - Added OnGotoInsurance
	//{{AFX_MSG(CResEntryDlg)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDeleteRes();
	virtual void OnCancel();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNotesBox();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelChosenNoShowCombo(long nRow);
	afx_msg void OnExtraInfoBtn();
	afx_msg void OnPatientIdCheck();
	afx_msg void OnHomePhoneCheck();
	afx_msg void OnWorkPhoneCheck();
	afx_msg void OnCellPhoneCheck();
	afx_msg void OnBirthDateCheck();
	afx_msg void OnPatBalCheck();
	afx_msg void OnInsBalCheck();
	afx_msg void OnPreferredContactCheck();
	virtual void OnOK();
	afx_msg void OnDropDownPurposeCombo();
	afx_msg void OnDropDownPurposeSetCombo();
	afx_msg void OnSelChangedPurposeCombo(long nNewRow) ;
	afx_msg void OnSelChangedPurposeSetCombo(long iNewRow);
	afx_msg void OnGotopatient();
	afx_msg void OnDropDownDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownDateEventStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDateEventStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownDateEventEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDateEventEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChangedApttypeCombo(long nRow);
	afx_msg void OnSelChangedAptPurposeCombo(long nRow);
	afx_msg void OnSelChangingAptPurposeCombo(long FAR* nNewSel);
	afx_msg void OnKillfocusNotesBox();
	afx_msg void OnBtnChangepatient();
	afx_msg void OnBtnNewpatient();
	// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
	afx_msg void OnSelChangingAptLocationCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnSelChangingApttypeCombo(long FAR* nNewSel);
	afx_msg void OnSelChosenAptResourceCombo(long nCurSel);
	afx_msg void OnSelChosenAptPurposeCombo(long nCurSel);
	afx_msg void OnSelChosenAptTypeCombo(long nCurSel);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnBtnAptLinking();	
	afx_msg void OnSaveCreateInvAllocation();	
	afx_msg void OnSaveCreateInvOrder();
	afx_msg void OnSaveCreateBill();
	afx_msg void OnSaveCreateCaseHistory();
	// (r.gonet 09/19/2013) - PLID 58416 - Saves the appointment and opens the medication history dialog for the patient.
	afx_msg void OnSaveViewMedicationHistory();
	afx_msg void OnSaveOpenCaseHistory();
	afx_msg void OnTrySetSelFinishedAptResourceCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedLocationCombo(long nRowEnum, long nFlags);
	// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
	afx_msg void OnSelChosenAptLocationCombo(LPDISPATCH lpCurSel);
	afx_msg void OnSelchangeExtraFields1();
	afx_msg void OnSelchangeExtraFields2();
	afx_msg void OnSelchangeExtraFields3();
	afx_msg void OnSelchangeExtraFields4();
	afx_msg void OnSelchangeExtraFields5();
	afx_msg void OnSelchangeExtraFields6();
	afx_msg void OnSelchangeExtraFields7();
	afx_msg void OnSelchangeExtraFields8();
	afx_msg void OnKillFocusArrivalTimeBox();
	afx_msg void OnKillFocusStartTimeBox();
	afx_msg void OnKillFocusEndTimeBox();
	afx_msg void OnChangedArrivalTimeBox();
	afx_msg void OnChangedStartTimeBox();
	afx_msg void OnChangedEndTimeBox();
	afx_msg void OnPinResentry();
	afx_msg void OnExtraBold1();
	afx_msg void OnExtraBold2();
	afx_msg void OnExtraBold3();
	afx_msg void OnExtraBold4();
	afx_msg void OnExtraBold5();
	afx_msg void OnExtraBold6();
	afx_msg void OnExtraBold7();
	afx_msg void OnExtraBold8();
	afx_msg void OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-12 15:23) - PLID 63200 - added Ex handler
	afx_msg LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelChosenRequestedResource(long nRow);
	afx_msg void OnRequeryFinishedAptpurposeCombo(short nFlags);
	afx_msg void OnRequeryFinishedAptTypeCombo(short nFlags);
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGotoInsurance();
	afx_msg void OnNewInsuredParty(); // (j.gruber 2012-08-01 09:27) - PLID 51885
	afx_msg void OnChooseAdditionalInsParties(); // (j.gruber 2012-08-01 09:27) - PLID 51896
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void OnSelChosenMoveup(LPDISPATCH lpRow);
	void OnSelChosenConfirmed(LPDISPATCH lpRow);
	void OnSelChosenReady(LPDISPATCH lpRow);
	void OnSelChangingMoveup(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingConfirmed(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingReady(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	//afx_msg void OnDropDownPatientCombo();
	//afx_msg void OnSelChosenPatientCombo(long nCurSel);
	//afx_msg void OnSelChangingPatientCombo(long FAR* nNewSel);
	//afx_msg void OnTrySetSelFinishedPatientCombo(long nRowEnum, long nFlags);
	void SelChangingPatientCombo2(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenPatientCombo2(LPDISPATCH lpRow);
	void TrySetSelFinishedPatientCombo2(long nRowEnum, long nFlags);

	afx_msg void OnBnClickedNxbtnCreateRecallSched();
	afx_msg void OnBnClickedResInsMore(); // (j.gruber 2012-07-30 14:36) - PLID 51830
	void SelChosenResInsCat(LPDISPATCH lpRow); // (j.gruber 2012-07-30 14:36) - PLID 51830
	void SelChangingResInsCat(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel); // (j.gruber 2012-07-30 14:36) - PLID 51830
	void RequeryFinishedReferringPhysList(short nFlags);
	void SelChosenReferringPhysList(LPDISPATCH lpRow);
	void TrySetSelFinishedReferringPhysList(long nRowEnum, long nFlags);
	void SelChangingReferringPhysList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel); // (j.gruber 2013-02-11 14:21) - PLID 54483
};

class CKeepDialogActive {
public:
	CKeepDialogActive(CResEntryDlg * pParent) { m_pParent = pParent; m_pParent->IncActiveCount(); };
	//TES 2004-02-06: The active count is "stackable", so don't assert.
	~CKeepDialogActive() { m_pParent->DecActiveCount(); /*ASSERT(m_pParent->m_nKeepActiveCount >= 0);*/ };
protected:
	CResEntryDlg *m_pParent;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

// (c.haag 2010-08-27 11:38) - PLID 40108 - Use this class if you might open a reservation. Its purpose is to
// begin a chain of events to ensure that the scheduler is not refreshed during this time.
class CReserveResEntry
{
	CResEntryDlg* m_pResEntry;
	CString m_strReservingFunction;

public:
	CReserveResEntry(CResEntryDlg* pResEntry, const CString& strReservingFunction)
	{
		m_strReservingFunction = strReservingFunction;
		if (NULL != (m_pResEntry = pResEntry)) {
			m_pResEntry->Reserve(strReservingFunction);
		}
	}
	~CReserveResEntry()
	{
		if (NULL != m_pResEntry)
		{
			BOOL bNeedUpdateView = m_pResEntry->WantedUpdateViewWhileReserved();
			m_pResEntry->Unreserve(m_strReservingFunction);
			// (c.haag 2010-10-11 12:05) - PLID 40108 - If Practice wanted to update the view
			// while we disallowed refreshing, then proceed to update the view now.
			if (!m_pResEntry->IsReserved(FALSE) && bNeedUpdateView) {
				if (NULL != GetMainFrame()) {
					CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
					if (pView) {
						pView->UpdateView();
					}
				}
			}
		}
	}
};

#endif // !defined(AFX_RESENTRYDLG_H__F3C3BAE3_BE9A_11D1_8041_00104B2FE914__INCLUDED_)
