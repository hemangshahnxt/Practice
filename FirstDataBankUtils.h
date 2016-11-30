#pragma once


namespace FirstDataBank {

	//TES 11/12/2009 - PLID 36241 - All FirstDataBank importing is now done in NxServer, which made 90% of the FirstDataBankUtils code invalid, 
	// so it's been taken out.


	//TES 10/1/2009 - PLID 34252 - Checks FirstDataBank data for the given NDC.  Returns true, and sets bContinueEditing
	// to false, if the NDC number exists and has the same name as strPracticeDrugName.  If it returns false, it will 
	// ask the user if they want to continue editing, and set bContinueEditing accordingly.  Throws exceptions.
	bool ValidateNDCNumber(const CString &strNDC, const CString &strPracticeDrugName, OUT bool &bContinueEditing);

	// (a.walling 2009-11-16 12:54) - PLID 36239 - Ensure the FirstDataBank database is available, with UI messages
	bool EnsureDatabase(CWnd* pMessageParent, bool bNotifyNxServer);

	// (a.walling 2009-11-16 13:00) - PLID 36239 - Is the FirstDataBank database valid?
	bool IsDatabaseValid();

	//TES 11/18/2009 - PLID 36360 - This function returns either strDEA_Practice, or a DEA Schedule pulled from the matching drug in the 
	// FirstDataBank database, if that DEA is stricter.
	CString CalcDEASchedule(const CString &strDEA_Practice, long nMedicationID);

	// (j.jones 2010-01-21 17:03) - PLID 37004 - added ability to calculate the NDC number,
	// using the NewCropID as the FirstDataBank ID
	// (j.jones 2010-01-26 17:40) - PLID 37078 - moved to FirstDataBankUtils
	CString ChooseNDCNumberFromFirstDataBank(long nFirstDataBankID);
}