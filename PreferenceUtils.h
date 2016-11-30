//PreferenceUtils.h

#ifndef PREFERENCE_UTILS_H
#define PREFERENCE_UTILS_H

#pragma once



// CAH 5/7/03 - Preference flags for required new patient entry
#define NEWPT_REQUIRED_MIDDLE		0x00000001
#define NEWPT_REQUIRED_ADDRESS1		0x00000002
#define NEWPT_REQUIRED_CITY			0x00000004
#define NEWPT_REQUIRED_STATE		0x00000008
#define NEWPT_REQUIRED_ZIP			0x00000010
#define NEWPT_REQUIRED_SSN			0x00000020
#define NEWPT_REQUIRED_DOB			0x00000040
#define NEWPT_REQUIRED_DOCTOR		0x00000080
#define NEWPT_REQUIRED_COORDINATOR	0x00000100
#define NEWPT_REQUIRED_GENDER		0x00000200
#define NEWPT_REQUIRED_HOMENO		0x00000400
#define NEWPT_REQUIRED_WORKNO		0x00000800
#define NEWPT_REQUIRED_REFERRAL		0x00001000
#define NEWPT_REQUIRED_REF_PHYS		0x00002000
#define NEWPT_REQUIRED_REF_PAT		0x00004000
#define NEWPT_REQUIRED_PAT_TYPE		0x00008000
#define NEWPT_REQUIRED_PROCEDURE	0x00010000
#define NEWPT_REQUIRED_EMAIL		0x00020000
#define NEWPT_REQUIRED_CELL			0x00040000
#define NEWPT_REQUIRED_NOTE			0x00080000
// (c.haag 2009-03-02 15:45) - PLID 17142 - Prefix
#define NEWPT_REQUIRED_PREFIX		0x00100000
#define NEWPT_REQUIRED_COUNTRY		0x00200000
// (r.gonet 05/16/2012) - PLID 48561 - Affiliate Physician
#define NEWPT_REQUIRED_AFFILIATE_PHYS	0x00400000
// (s.dhole 2013-06-04 13:27) - PLID 12018 Location
#define NEWPT_REQUIRED_LOCATION			0x00800000

//(e.lally 2007-06-20) PLID 24334 - Additional "one or more" requirements for new patient entry
#define NEWPT_ADDITIONAL_HOMENO		0x00000001
#define NEWPT_ADDITIONAL_WORKNO		0x00000002
#define NEWPT_ADDITIONAL_CELL		0x00000004
#define NEWPT_ADDITIONAL_EMAIL		0x00000008

// (j.jones 2016-01-21 10:30) - PLID 67971 - added bitmasked options for the IncludeFreeTextFDBSearchResults preference
#define INCLUDEFREETEXT_PRESCRIPTIONS		0x00000001
#define INCLUDEFREETEXT_CURMEDS				0x00000002
#define INCLUDEFREETEXT_ALLERGIES			0x00000004

enum PreferenceItem {
	piRoot				= 0,
	piPatientsModule	= 1,
	piSchedulerModule	= 2,
	piLetterWModule		= 3,
	piContactsModule	= 4,
	piMarketingModule	= 5,
	piInventoryModule	= 6,
	piFinancialModule	= 7,
	piReportsModule		= 8,
	piAdminModule		= 9,
	piAscModule			= 10,
	
	piNewPatient		= 11,
	piGeneral1			= 12,
	piGeneral2			= 13,
	piCustom			= 14,
	piTracking			= 15,
	piFollowUp			= 16,
	piNotes				= 17,
	piEMR				= 18,
	piInsurance			= 19,
	piBilling			= 20,
	piQuotes			= 21,
	piAppointments		= 22,
	piHistory			= 23,
	piMedications		= 24,
	
	piCptCodes			= 25,
	piSurgeries			= 26,
	
	piSuperbill			= 27,
	//This was replaced by piReportsModule a while ago
	//piReports			= 28,

	piNewContact		= 29,

	piPracYakker		= 30,

	piEbilling			= 31,

	piBatchPayments		= 32,

	piPtToolBar			= 33,

	piCredentialing		= 34,

	piOther				= 35,

	// (m.hancock 2006-05-09 15:29) - PLID 20433 - Adding a tab for ToDo Prefs for future consideration
	piToDo				= 36,

	// (c.haag 2006-06-27 15:40) - PLID 21185 - Adding an entry for labs
	piLabs				= 37,

	//TES 21764 - Adding NxSupport preferences.
	piNxSupport			= 38,

	// (z.manning, 10/10/2006) - PLID 5812 - Moved Rooms to its own area in the tree to make room for scheduler => display 2 tab.
	piRooms				= 39,

	// (j.jones 2008-04-23 14:45) - PLID 29597 - added HL7 tab
	piHL7				= 40,

	piOHIP				= 41,

	// (e.lally 2009-06-02) PLID 34396 - Added preferences for auditing
	piAuditing			= 42,

	// (j.jones 2009-08-25 17:37) - PLID 35338
	piPreferenceCards	= 43,

	// (c.haag 2009-11-05 17:35) - PLID 35920
	piNexPhoto			= 44,
	// (d.thompson 2009-11-17) - PLID 36134
	piLinksModule		= 45,
	// (d.lange 2010-08-02 15:26) - PLID 38249 - Added preference for zip codes
	piZipCodes			= 46,
	//(e.lally 2010-07-20) PLID 37982
	piNexWeb			= 47,
	// (j.jones 2010-10-26 09:21) - PLID 41068 - added Devices section
	piDevices			= 48,
	// (j.jones 2010-11-03 14:41) - PLID 39620 - added an Alberta section
	piAlbertaHLINK		= 49,
	//TES 11/17/2010 - PLID 41528 - Added a Glasses Order (VisionWeb) section
	piGlassesOrder		= 50,
	//(a.wilson 2012-3-5) PLID 49702 - added group for patient recall system.
	piRecallSystem	= 51,
};

// (c.haag 2005-10-31 13:01) - PLID 16595 - We now use the Practice
// property manager to handle ConfigRT functions
//
//TES 6/23/2009 - PLID 34155 - Added a parameter for whether this user can edit Global preferences.
// (j.luckoski 2012-08-15 09:56) - PLID 23345 - Added parameter for location
AFX_EXT_CLASS int ShowPreferencesDlg(class CNxPropManager* pPropManager, LPDISPATCH pCon, CString strUserName, CString strRegistryBase, PreferenceItem piStartTab = piPatientsModule, BOOL bCanEditGlobalPrefs = TRUE, CString strLocation = "");

//These definitions are the IDs for each tree item.
//These CAN be changed/incremented/removed/rearranged. They are not used in data.
//We are only using these defines as a better way to remember the numbers,
//and so switch statements work. (Otherwise, we would just identify selected tree items
//by the text description, and who wants to do that?)


#define MAX_TREE_POS		piAuditing	//Special define, must be equal to highest of others.

#define MIN_TREE_POS		piPatientsModule //Special define, must be equal to smallest of the others.

#endif