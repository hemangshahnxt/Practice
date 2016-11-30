#pragma once

#include "DiagSearchResults.h"

//settings for the view mode we desire
enum DiagCodeSearchStyle
{
	eInvalidCodeSearchStyle = -1,	//invalid, not initialized yet
	eICD9_10_Crosswalk = 1,			//the complex ICD-9/10 search using crosswalks (default)
	eManagedICD9_Search,			//searching only the managed ICD-9 list
	eManagedICD10_Search,			//searching only the managed ICD-10 list
	eFullICD9or10_Search,			//searches a combined full 9 & 10 list, no crosswalk, not a managed list
	eFullICD10_Search,				//searches the full ICD-10 list, not a managed list
	eFullICD9_Search,				// (j.jones 2015-09-01 14:45) - PLID 66993 - searches the full ICD-9 list, not a managed list
};

// (d.thompson 2014-02-04) - PLID 60638
//	One of these should be instantiated globally per application.  It contains all the information for how the 
//	search behavior works across all diagnosis code searching in the program, including creating datalists, 
//	parsing the results back for you, and all configuration data.
//NOTE:  This is an abstract class.  You cannot instantiate this class itself, but should instead use the derived 
//	classes.  If you need to define a new search format, you should derive a new class from this one.  
//	See CDiagSearchConfig_Crosswalk.h as an example.
class CDiagSearchConfig
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearchConfig(bool bIncludePCS);	
	CDiagSearchConfig(DiagCodeSearchStyle eSS, bool bIncludePCS);

	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	bool GetIncludePCS();

	//Simply returns the search style.  You should not need to override this in any base classes, just
	//	make sure the style is set properly in your constructor.
	DiagCodeSearchStyle GetSearchStyle();

	//Abstract function, derived classes must provide an implementation.
	//	This is the creation of the results datalist for this type of diag search.  Please implement this
	//	to create the columns on the given datalist.
	virtual void CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon) = 0;

	//Abstract function, derived classes must provide an implementation.
	//	This function takes the selected pSearchRow (generally from an OnSelChosen event) and converts the 
	//	data to a CDiagSearchResults object.  This is done to simplify the types of data that users of
	//	the DiagSearch types of datalists need to implement.  They can safely know that by looking at 
	//	the CDiagSearchResults values, they will always properly interpret the selection no matter
	//	if the search is on ICD-9, ICD-10, or other.
	virtual CDiagSearchResults ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow) = 0;


	//FUTURE
	//	We have intentionally made sure that the search style cannot change.  The idea is that one of these
	//	configuration objects exists, tied to the app/mainframe/etc of your application.  All datalists
	//	throughout the program need to be the same style.  We wouldn't want you to load g2, get a crosswalk, 
	//	change your preference, load billing then get an icd9 search - or you can't use the GetSearchStyle().
	//	If you want to change search style, you'll want to change your preference and restart the whole program.
	//
	//That said, I believe we could implement change of style by doing the following.  Consider if this is really
	//	necessary:
	// - track an array of all datalists that come through CreateResultList
	// - implement a SwitchStyle() function
	// - on SwitchStyle(), destroy all the datalist columns and re-run the new CreateResultList() on them
	// - I'm not 100% sure how to manage closed datalists -- open g2, then close the patients module.  We'd still
	//		have a pointer, and would need to monitor for removal.  That could be messy.


protected:
	//Base class implementation to be shared by all derived classes.  You should not need to override this in any
	//	derived class.  This function should be called by all implementations of CreateResultList() to ensure the
	//	datalist is in a proper state.
	void CreateResultListBase(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon);

	// (j.jones 2015-03-23 13:45) - PLID 65281 - moved SetMinDropdownWidth to GlobalUtils

	// (d.thompson 2014-02-10) - PLID 60716
	//Base class implementation to be shared by all derived classes.  You should not need to override this in any 
	//	derived class.
	//This is a helper function to ensure that your chosen icd9 and icd10 codes exist in the DiagCodes table
	//	before you send them back to the rest of the program.  It basically works like this:
	//	1)  Fill a CDiagSearchResults structure with values.  It's OK if m_nDiagCodesID is -1.
	//	2)  Pass a pointer to this function
	//	3)  This function will evaluate the results.  If any IDs are -1, it will create the appropriate record in
	//		the DiagCodes table, it will then update your CDiagSearchResults object with those new IDs, and return
	//		back to you.
	//	4)  If your results already have valid IDs, this function will do nothing.
	//	5)  It is possible to choose 9's without 10's and vice versa.  In that case, we also look at
	//		the m_strCode member.  If that is blank for a -1 ID, we will not create a record.
	void EnsureDataInDiagCodes(IN OUT CDiagSearchResults *pResults);


protected:
	//Search style.  This is really provided as a convenience if anyone wants to lookup and check what type
	//	of search we're doing without the hassle of trying to compare the typeof() derived class.
	const DiagCodeSearchStyle m_eSearchStyle;
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference, it's a member
	// variable because it should not change after construction
	bool m_bIncludePCS;
};
