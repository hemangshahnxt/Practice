#pragma once

#include <afxtempl.h>
#include "PracticeRc.h"

// CReconcileMedicationsDlg dialog
//
// (c.haag 2010-02-17 10:05) - PLID 37403 - Initial implementation. This dialog brings up a list
// of patient Current Medications and patient Prescriptions; and gives the user the opportunity
// to select patient Prescriptions to put into the patient Current Medications list.
//

class CReconcileMedicationsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReconcileMedicationsDlg)

private:
	// This class represents a single current medication used as an input in this dialog
	struct Medication
	{
		long nCurrentPatientMedsID; // The ID in CurrentPatientMedsT
		long nMedicationID; // The MedicationID value in CurrentPatientMedsT
									// (corresponds to DrugList.ID. To get the EmrDataID, look up DrugList.EMRDataID).
		long nEMRDataID; // The EmrDataID of the medication
		// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
		CString strNewCropGUID;
		CString strName; // The name of the medication
		// (j.jones 2011-05-02 16:20) - PLID 43450 - added Sig
		CString strSig;
		BOOL bActive;
		// (s.dhole 2013-06-18 15:26) - PLID 56926 Added last date 
		COleDateTime dtLastDate;
		// (b.eyers 2015-12-18) - PLID 67749 - add start date
		COleDateTime dtStartDate;
	};
	typedef CArray<Medication,Medication&> CMedicationArray;

	// This class represents a single prescription used as an input in this dialog
	struct Prescription
	{
		long nPatientMedicationsID; // The ID in PatientMedications
		long nMedicationID; // The MedicationID value of the PatientMedications (prescription) table
		long nEMRDataID; // The EmrDataID of the medication
		// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
		CString strNewCropGUID;
		CString strName; // The name of the prescription
		// (j.jones 2011-05-02 16:20) - PLID 43450 - added PatientExplanation
		CString strPatientExplanation;
		BOOL bActive;
		// (s.dhole 2013-06-18 15:26) - PLID 56926 Added last date 
		COleDateTime dtLastDate;
		// (b.eyers 2015-12-18) - PLID 67749 - prescription date
		COleDateTime dtPrescriptionDate;
		
	};
	typedef CArray<Prescription,Prescription&> CPrescriptionArray;

public:
	// (s.dhole 2013-06-21 16:18) - PLID 55964 Restructure routin
	// This class represents a change we need to make
	//struct Change
	//{
	//	enum { eAddRx, eDeleteCurMed,eMergeCurMed  } op; // Indicates what we're doing to the record
	//	long nInternalID; // The internal ID used for deleting a current medication
	//	long nMedicationID; // The MedicationID value (uniform with the other classes)
	//	long nEMRDataID; // The EmrDataID of the medication
	//	CString strName; // The medication name
	//	CString strNewCropGUID;	// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
	//	CString strSig;			// (j.jones 2011-05-02 16:07) - PLID 45450 - supported Sig
	//	COleDateTime dtLastDate;
	//};
	//typedef CArray<Change,Change&> CChangeArray;

public:
	// If a user does not have access to create or delete patient current medications, then returns FALSE. Otherwise, returns TRUE.
	static BOOL CanCurrentUserAccess();

public:
	CReconcileMedicationsDlg(long nPatientID, CWnd* pParent);   // standard constructor
	virtual ~CReconcileMedicationsDlg();

// Dialog Data
	enum { IDD = IDD_RECONCILE_MEDICATIONS };

private:
	NXDATALIST2Lib::_DNxDataListPtr	m_dlMedRx;
	NXDATALIST2Lib::_DNxDataListPtr	m_dlOldMedRx; // (s.dhole 2013-06-18 15:43) - PLID 55964
	CNxColor m_nxcBack;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	OLE_COLOR m_clrBack;

private:
	// The array of patient current medications that existed when this dialog was opened
	CMedicationArray m_aOldCurMeds;
	// The array of prescriptions that would act as candidates for adding to the patient current medications list
	CPrescriptionArray m_aPrescriptions;
public:
	// (s.dhole 2013-09-18 14:28) - PLID 56625 
	struct MergeMedication
	{
		enum { eKeepCurMed  , eAddMed, eDeleteMed, eMergeCurMed , eExcludeCurMed } Action; // Indicates what we're doing to the record
		long nPatientMedicationsID; // The ID in PatientMedications
		long nMedicationID; // The MedicationID value of the PatientMedications (prescription) table
		long nEMRDataID; // The EmrDataID of the medication
		CString strNewCropGUID;
		CString strName; // The name of the prescription
		CString strPatientExplanation;
		COleDateTime dtLastDate;
		COleDateTime dtStartDate; // (b.eyers 2015-12-18) - PLID 67749
		BOOL bIsActive;
		long nCurrentPatientMedsID; // The ID in CurrentPatientMedsT
		long nCurrentMedicationID; // The MedicationID value in CurrentPatientMedsT
		long nCurrentEMRDataID; // The EmrDataID of the medication
		CString strCurrentNewCropGUID;
		CString strCurrentName; // The name of the medication
		CString strCurrentSig;
		BOOL bCurrentIsActive;
		COleDateTime dtCurrentLastDate;
		COleDateTime dtCurrentStartDate; // (b.eyers 2015-12-18) - PLID 67749
		CString strCurrentNameSig;
	};
	typedef CArray<MergeMedication,MergeMedication&> CMergeMedicationArray;
	CMergeMedicationArray m_aMergeMedication;

private:



	// The output list of changes created after the user clicks OK
	// (s.dhole 2013-06-21 16:18) - PLID 55964 Restructure routin
	CMergeMedicationArray m_aRequestedChanges;
private:
	// The ID of the patient involved in this dialog
	long m_nPatientID;

// Data population functions
public:
	// Adds all current medications of the member patient to the input current meds list
	void AddCurrentMedicationsFromData();
	// Adds medications defined by EmrDataT ID's to the input current meds list
	// (j.jones 2011-05-04 14:37) - PLID 43527 - added mapDataIDsToSig, which tracks the Sig for each current medication
	void AddCurrentMedicationsFromEmrDataIDs(const CArray<long,long>& anEmrDataIDs, CMap<long, long, CString, LPCTSTR> &mapDataIDsToSig);
public:
	// Adds a single prescription, by patient medication ID, to the input Rx list
	void AddPrescriptionByPMID(long nPatientMedicationsID);
	// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to add multiple prescriptions at once
	void AddPrescriptionByPMIDs(CArray<long, long> &aryPrescriptionIDs);
	// Adds a single prescription, by medication ID, to the input Rx list
	// (j.jones 2011-05-04 16:51) - PLID 43527 - added strPatientExplanation
	void AddPrescriptionByMedicationID(long nMedicationID, CString strPatientExplanation);
	// Adds multiple prescriptions from NewCrop. Returns the number of prescriptions added.
	int AddPrescriptionsByNewCropRxGUIDs(const CStringArray& astrNewCropRxGUIDs);

// Color functions
public:
	inline void SetBackColor(OLE_COLOR clr) { m_clrBack = clr; }

// Output functions
public:
	// This function should only be called from the patient medications tab. Its purpose is
	// to commit the requested user changes to data.
	// (s.dhole 2013-06-21 16:18) - PLID 55964 Restructure routin
	inline CMergeMedicationArray& GetRequestedChanges() { return m_aRequestedChanges; }

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		void SetExistingItemFlag(NXDATALIST2Lib::IRowSettingsPtr pRow,BOOL bShoCheckBox);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
   void EditingFinishingListMedNewRx(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
