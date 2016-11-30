#pragma once

// (d.thompson 2014-02-04) - PLID 60638
//	This class simply encapsulates the results of a diagnosis code search.  This allows callers to not have to 
//	worry about if the search is set to icd9, icd10, or crosswalk.  Simply interpret the data in this object appropriately.
//	If the search doesn't support 9 or 10, you'll just get empty members.
//
//If the m_nDiagCodesID of your ICD9 or ICD10 is -1, then nothing was selected.  If it's anything else, that's the ID
//	in the DiagCodes table for that particular code.
class CDiagSearchResults {
public:

	//Encapsulate the diag code object itself.  Simply ID, code, description.
	class CDiagCode {
	public:

		//DiagCodes.ID.  You are guaranteed that any results in this class will already be imported into DiagCodes.
		//	If this value is -1, then there is no code.
		long m_nDiagCodesID;

		//The Code Number.  This could be the ICD-9 or ICD-10 code number.  Matches with DiagCodes.CodeNumber.
		CString m_strCode;

		//The Code description.  Note:  Descriptions are a little funny at present.  This will typically be the description
		//	from FullICD9T or FullICD10T (unless you are in a search mode that intentionally searches DiagCodes, at which
		//	point it will be the DiagCodes.CodeDesc field).  HOWEVER, note that once we import that info into DiagCodes, 
		//	it becomes user editable.  So your next search will show the description from FullICD10T, even though you may
		//	have changed it in DiagCodes.  We're pondering this for the future.
		CString m_strDescription;

		CDiagCode()
		{
			//Default to -1 for new, empty objects
			m_nDiagCodesID = -1;
		}
	};

	CDiagSearchResults();
	~CDiagSearchResults();

	//Data about the ICD-9 code selected.  If ID is -1, there is no code
	CDiagCode m_ICD9;

	//Data about the ICD-10 code selected.  If ID is -1, there is no code
	CDiagCode m_ICD10;

	//Is this a quicklist entry?
	bool m_bIsQuicklist;

	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS bit
	bool m_bPCS;
};
