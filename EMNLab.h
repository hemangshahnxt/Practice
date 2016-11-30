#pragma once

// (z.manning 2008-10-06 10:50) - PLID 21094
class EMNLab
{
public:
	long nID;
	// (z.manning 2008-10-09 12:51) - PLID 31628 - Need the lab procedure ID for unspawning
	long nLabProcedureID;
	BOOL bIsNew;
	
	// (z.manning 2009-02-26 17:48) - PLID 33141 - Use the new source action info class
	SourceActionInfo sai;

	// (z.manning 2008-10-07 11:04) - PLID 31561 - Added fields to display lab info within EMR
	CString strAnatomicLocation;
	//TES 11/10/2009 - PLID 36260 - Replaced AnatomicSide with AnatomyQualifierID
	//TES 12/7/2009 - PLID 36470 - It's back!
	AnatomySide eAnatomicSide;
	CString strAnatomicLocationQualifier;
	CString strClinicalData;
	LabType eType;
	CString strToBeOrdered; // (z.manning 2008-10-24 10:35) - PLID 31807

	EMNLab()
	{
		nID = -1;
		nLabProcedureID = -1;
		bIsNew = TRUE;
		eAnatomicSide = asNone;
		eType = ltBiopsy;
	}

	void CopyLabDetailsOnly(const EMNLab &source)
	{
		strAnatomicLocation = source.strAnatomicLocation;
		eAnatomicSide = source.eAnatomicSide;
		strAnatomicLocationQualifier = source.strAnatomicLocationQualifier;
		strClinicalData = source.strClinicalData;
		strToBeOrdered = source.strToBeOrdered;
	}

	// (d.thompson 2009-09-24) - PLID 32043 - Created defined names for the other 3 lab types
	CString GetTitle()
	{
		if(eType == ltBiopsy) {
			return "Biopsy";
		}
		else if(eType == ltLabWork) {
			return "Lab Work";
		}
		else if(eType == ltCultures) {
			return "Cultures";
		}
		else if(eType == ltDiagnostics) {
			return "Diagnostics";
		}
		else {
			//Failover, in case we add a new type and its missed, we'll just fall back to using the generic 'Lab'
			return "Lab";
		}
	}

	// (z.manning 2008-10-07 17:08) - PLID 31561 - Formats the text for creating a lab detail.
	// (z.manning 2008-10-30 09:41) - PLID 31613
	//
	// IMPORTANT NOTE: This function needs to stay in sync with the section of 
	// CEMNDetail::GenerateSaveString_State that updates lab details.
	CString GetText()
	{
		CString strText;
		if(eType == ltBiopsy) {
			//TES 11/10/2009 - PLID 36260 - Replaced AnatomicSide with AnatomyQualifierID
			//TES 12/8/2009 - PLID 36512 - AnatomySide is back.
			// (z.manning 2010-03-25 09:06) - PLID 37553 - We now have a function to format the anatomic location text
			strText = ::FormatAnatomicLocation(strAnatomicLocation, strAnatomicLocationQualifier, eAnatomicSide);
			if(!strText.IsEmpty()) {
				strText = "Anatomic Location:\r\n" + strText;
			}
		}
		else {
			if(!strToBeOrdered.IsEmpty()) {
				strText += "To be ordered:\r\n" + strToBeOrdered;
			}
		}

		if(!strClinicalData.IsEmpty()) {
			strText += "\r\n\r\nComments:\r\n" + strClinicalData;
		}

		// IMPORTANT NOTE: This function needs to stay in sync with the section of 
		// CEMNDetail::GenerateSaveString_State that updates lab details.

		return strText;
	}
};