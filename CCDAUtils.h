
// (b.savon 2014-02-25 14:28) - PLID 61029 - Created - Restructure the Clinical Summary and Summary of Care code in EMR so that 
// it can be used by multiple areas in Practice.

#pragma once

namespace CCDAUtils
{
	// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
	// (d.singleton 2013-11-15 11:58) - PLID 59513 - need to insert the CCDAType when generating a CCDA
	// (d.singleton 2013-11-15 14:03) - PLID 59527 - need to be able to create clinical summary and care summary from the emr ribbon.
	// (a.walling 2014-05-13 14:43) - PLID 61788 - Moved from GlobalUtils - now takes in description as well
	// (r.gonet 04/30/2014) - PLID 61805 - Added the PIC ID so that the MailSent record can be associated with a PIC.
	long AttachToHistory(Nx::SafeArray <BYTE> fileBytes, const CString& strDesc, const CString& strExtension, const CString& strExtraDesc, const CString& strSelection, CCDAType ctType, long nPICID, long nEMNID, long nPersonID, bool bGetCCDDescriptionInfo);

	void GenerateSummaryOfCare(long nPatientPersonID, long nEMNID = -1,  HWND hParentWnd = NULL);

	// (b.savon 2014-04-30 09:31) - PLID 61791 - User Story 10 - Auxiliary CCDA Items - Requirement 100150 - Added xml and pdf params
	// (r.gonet 05/07/2014) - PLID 61805 - Added parameter nPICID, which causes the clinical summary to attach to history
	// with an associated PIC
	void GenerateClinicalSummary(long nPatientPersonID, long nPICID, long nEMNID, HWND hParentWnd = NULL, bool bGenerateXML = true, bool bGeneratePDF = false );
}