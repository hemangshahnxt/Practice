#pragma once

#include "PatientsRc.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

class CNxAPI;

// (j.fouts 2012-08-15 13:07) - PLID 52145 - Added

// CDrugInteractionDlg dialog

class CDrugInteractionDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDrugInteractionDlg)

public:
	// (b.savon 2012-11-30 10:30) - PLID 53773 - Pass in the parent enum
	CDrugInteractionDlg(CWnd* pParent = NULL, EInteractionParent eipParent = eipMedications);
	virtual ~CDrugInteractionDlg();

// Dialog Data
	enum { IDD = IDD_DRUG_INTERACTION_DLG };

protected:
	//Datalist for the interactions
	NXDATALIST2Lib::_DNxDataListPtr m_pInteractionList;
	//NxColor background
	CNxColor m_bkg;
	CNxIconButton m_btnClose;
	// (j.jones 2013-05-10 14:37) - PLID 55955 - added ability to configure severity filters
	CNxIconButton m_btnConfigSeverityFilters;
	// (j.jones 2013-05-14 16:50) - PLID 56634 - added ability to toggle filtering on and off
	CNxIconButton m_btnToggleFilter;
	CNxStatic m_nxstaticLabelInteractionCount;

	// (b.savon 2012-11-30 08:38) - PLID 53773
	EInteractionParent m_eipParent;

	long m_nCurrentPatientID;

	//Interaction arrays
	// (j.jones 2013-05-14 08:50) - PLID 56634 - renamed to "all" interactions to indicate that
	// these arrays include all current interactions, counting those that might be hidden
	Nx::SafeArray<IUnknown *>  m_aryAllDrugDrugInteractions;		// (j.fouts 2012-09-07 11:00) - PLID 51708 - Use this safe array to avoid a bug
	Nx::SafeArray<IUnknown *>  m_aryAllDrugAllergyInteractions;	// (j.fouts 2012-09-07 11:00) - PLID 51709 - Use this safe array to avoid a bug
	Nx::SafeArray<IUnknown *>  m_aryAllDrugDiagnosisInteractions;	// (j.fouts 2012-09-07 11:00) - PLID 51710 - Use this safe array to avoid a bug

	// (j.jones 2013-05-14 08:51) - PLID 56634 - Added "filtered" interactions to represent
	// the subset of interactions that are never hidden, hiding low priority ones the user
	// may have chosen to suppress. The filtered list may or may not be the list displayed
	// on the screen, due to the "show all" toggle ability.
	Nx::SafeArray<IUnknown *>  m_aryFilteredDrugDrugInteractions;
	Nx::SafeArray<IUnknown *>  m_aryFilteredDrugAllergyInteractions;
	Nx::SafeArray<IUnknown *>  m_aryFilteredDrugDiagnosisInteractions;

	// (j.jones 2012-11-28 16:03) - PLID 53194 - tracks whether the current refresh actually changed the content
	// of any of the three interactions lists
	bool m_bInteractionsChanged;

	// (b.savon 2012-11-29 16:51) - PLID 53773 - Restore Window Size
	void LoadWindowSize();
	CString GetInteractionParent();

public:
	//Replaces the call to DoModal, will call DoModal if an interaction exists or bForceShow is true
	// (j.jones 2012-09-26 14:09) - PLID 52872 - added patient ID
	// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
	// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
	// will show the dialog even if its current interactions have not changed since the last popup.
	void ShowOnInteraction(long nPatientID, bool bRequery = true,
							bool bForceShowEvenIfBlank = false, bool bForceShowEvenIfUnchanged = false);

	// (j.fouts 2012-11-14 11:42) - PLID 53573 - Created a ShowOnInteractions to show without calling the API, this will use the arrays
	//	of interactions that are passed
	// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
	// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
	// will show the dialog even if its current interactions have not changed since the last popup.
	void ShowOnInteraction(Nx::SafeArray<IUnknown*> saryDrugDrugInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts,
									  long nPatientID,
									  bool bForceShowEvenIfBlank = false, bool bForceShowEvenIfUnchanged = false);

// (j.fouts 2012-09-07 13:18) - PLID 52482 - Need to handle the patient changing
	void HandleActivePatientChange();

	// (j.jones 2012-11-28 16:24) - PLID 53194 - assigns our tracked interaction arrays
	// and toggles the m_bInteractionsChanged boolean
	bool UpdateInteractionArrays(Nx::SafeArray<IUnknown *> aryAllNewDrugDrugInteractions,
								Nx::SafeArray<IUnknown *> aryAllNewDrugAllergyInteractions,
								Nx::SafeArray<IUnknown *> aryAllNewDrugDiagnosisInteractions);

	// (j.fouts 2013-01-03 12:27) - PLID 54429 - Counts the current number of interactions
	long InteractionCount();
	// (j.jones 2013-05-14 16:57) - PLID 56634 - returns the count of interactions that
	// remain displayed after low-severity interactions are hidden (if any)
	long GetFilteredInteractionCount();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void Refresh();	// (j.fouts 2012-09-06 08:38) - PLID 52482 - Need to refresh the datalist

	DECLARE_MESSAGE_MAP()

private:

	void ShowIfForced(BOOL bForceShow);
	void PopulateDrugDrugInteractions();
	void PopulateDrugDiagnosisInteractions();
	void PopulateDrugAllergyInteractions();

	//TES 11/10/2013 - PLID 59399 - Function so that rows with no monograph don't show the link cursor on the monograph cell
	NXDATALIST2Lib::IFormatSettingsPtr GetMonographCellFormat(BOOL bHasMonograph);

	// (j.jones 2013-05-14 09:06) - PLID 56634 - added functions to filter a list of "all" interactions
	// down to a list of interactions that will be displayed, ignoring low-severity interactions that
	// the user has chosen to suppress
	Nx::SafeArray<IUnknown *> FilterDrugDrugInteractionsBySeverity(Nx::SafeArray<IUnknown *> aryAllDrugDrugInteractions);
	Nx::SafeArray<IUnknown *> FilterDrugAllergyInteractionsBySeverity(Nx::SafeArray<IUnknown *> aryAllDrugAllergyInteractions);
	Nx::SafeArray<IUnknown *> FilterDrugDiagnosisInteractionsBySeverity(Nx::SafeArray<IUnknown *> aryAllDrugDiagnosisInteractions);

	// (j.jones 2013-05-14 17:06) - PLID 56634 - added ability to toggle filtering on and off
	bool m_bShowFilteredInteractions;
	
	DECLARE_EVENTSINK_MAP()
	void LeftClickInteractionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDestroy();
	// (j.jones 2013-05-10 14:37) - PLID 55955 - added ability to configure severity filters
	afx_msg void OnBtnConfigureSeverityFilters();
	// (j.jones 2013-05-14 16:50) - PLID 56634 - added ability to toggle filtering on and off
	afx_msg void OnBtnToggleInteractionFilter();
};
