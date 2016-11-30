// (c.haag 2011-06-17) - PLID 36477 - This class encapsulates the population of a pop-up menu pertaining
// to an appointment in the scheduler, room manager and appointments tab of the patients module. In a perfect
// world this would be inherited from a CMenu and invoked from all three places; but due to their diverse needs
// and the time that would be necessary to refactor all the code, this class will only have part of the functionality
// needed between the three.
//
// The commands added to the menu are actually ID's from AppointmentsDlg.h, SchedulerRc.h, and special
// sentinel values (like MIN_ID_SUPERBILL_TEMPLATES) that have large number ranges, and miMarkAsShowStateStatus.
// The numeric menu values are like:
//
// Less than negative 1000: miMarkAsShowStateStatus
// 10000 - 19999: Superbill Templates
// 20000 - 29999: Co-pay resps
// > 30000: wParams used in WM_COMMAND messages

#pragma once



class CApptPopupMenuManager
{
private:
	// The menu we're manipulating
	CMenu& m_mnu;
	// The parent window for if and when we open dialogs
	CWnd* m_pParent;
	// The appointment ID
	long m_nApptID;
	// The patient ID
	long m_nPatientID;

private:
	// (c.haag 2011-06-23) - PLID 44286 - The Superbills menu (if needed)
	CMenu m_mnuSuperbills;
	// (c.haag 2011-06-23) - PLID 44286 - Array of Superbill templates (if needed)
	CStringArray m_arySuperbillTemplates;
	// (c.haag 2011-08-22) - PLID 44286 - True if we prompt for a superbill path if the path is empty
	bool m_bPromptIfSuperbillEmpty;

private:
	// (c.haag 2011-06-24) - PLID 44317 - Billing variables
	long& m_nPrimaryInsuredPartyID;
	CArray<long, long>& m_aryCopayInsuredPartyIDs;

public:
	// Constructor
	CApptPopupMenuManager(CMenu& mnu, long& nPrimaryInsuredPartyID, CArray<long, long>& aryCopayInsuredPartyIDs, CWnd* pParent, long nApptID, long nPatientID);

public:
	inline const CStringArray& GetSuperbillTemplates() const { return m_arySuperbillTemplates; }

	//TES 1/17/2012 - PLID 47471 - We need to provide access to the PromptIfSuperbillEmpty flag
	inline const bool GetPromptIfSuperbillEmpty() const { return m_bPromptIfSuperbillEmpty; }

public:
	// This function adds confirmation-related options to an appointment pop-up menu. This is utilized in the scheduler,
	// room manager, and appointments tab.
	void FillConfirmedOptions(long nConfirmed);

	// (c.haag 2011-06-23) - PLID 44286 - This function adds confirmation-related options to an appointment pop-up menu.
	// This is utilized in the scheduler and room manager.
	void FillSuperbillOptions();

	// (c.haag 2011-06-23) - PLID 44287 - This function adds inventory-related options to an appointment pop-up menu. This is utilized in the scheduler,
	// room manager, and appointments tab. Returns TRUE if items were added to the menu.
	BOOL FillInventoryOptions();

	// (c.haag 2011-06-24) - PLID 44317 - This function adds billing-related items to an appointment pop-up menu. This is utilized
	// in the scheduler, room manager, and appointments tab in the patients module. Returns TRUE if items were added to the menu.
	BOOL FillBillingOptions(BOOL bAddedSeparator);

	// (c.haag 2011-06-24) - PLID 44319 - This function adds e-eligibility-related items to an appointment pop-up menu. This is utilized
	// in the scheduler, room manager, and appointments tab in the patients module. Returns TRUE if items were added to the menu.
	BOOL FillEEligibilityOptions(BOOL bAddedSeparator);

	// (c.haag 2011-06-24) - PLID 44319 - This function adds case history-related items to an appointent pop-up menu. This is utilized
	// in the scheduler, room manager, and appointments tab in the patients module. Returns TRUE if items were added to the menu.
	BOOL FillCaseHistoryOptions(BOOL bAddedSeparator, long nAptTypeCategory);

public:
	// This should be called to handle pop-up menu results that aren't handled from outside this object. Returns TRUE
	// if we did anything, FALSE if not.
	BOOL HandlePopupMenuResult(int nCmdID);
};
