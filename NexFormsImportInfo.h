// NexFormsImportInfo.h: interface for the CNexFormsImportInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEXFORMSIMPORTINFO_H__36768314_2393_4A16_89AE_8F50A980354C__INCLUDED_)
#define AFX_NEXFORMSIMPORTINFO_H__36768314_2393_4A16_89AE_8F50A980354C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NexFormsUtils.h"
#include "ShowConnectingFeedbackDlg.h"

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
class NexFormsImportInfo
{
public:
	NexFormsImportInfo();
	~NexFormsImportInfo();

	// (z.manning, 07/12/2007) - Version info.
	long m_nCodeVersion;
	long m_nContentFileVersion;
	CString m_strAspsVersion;
	long m_nNexFormsVersion;

	CArray<NexFormsProcedure*,NexFormsProcedure*> m_arypProcedures;
	CArray<NexFormsLadder*,NexFormsLadder*> m_arypLadders;
	CArray<NexFormsWordTemplate*,NexFormsWordTemplate*> m_arypWordTemplates;
	CArray<NexFormsPacket*,NexFormsPacket*> m_arypPackets;

	CArray<ExistingLadder*,ExistingLadder*> m_aryExistingLadders;

	void CleanUpExistingLadders();
	void CleanUpNexFormsContent();

	// (c.haag 2009-02-03 15:06) - PLID 32647 - Added progress feedback dialog
	void ImportProceduresAndLadders(CShowConnectingFeedbackDlg *pFeedback);
	// (z.manning, 07/19/2007) - PLID 26746 - Added a CWnd parameter for message box purposes.
	BOOL ImportPacketsAndTemplates(CWnd *pwndParent, CShowConnectingFeedbackDlg *pFeedback);

	CWinThread *m_pPostProcedureThread;
	HANDLE m_hevDestroying;

	// (z.manning, 07/30/2007) - PLID 26869 - Auditing stuff.
	long m_nAuditTransactionID;
	void NexFormsImportAuditEvent(long nItem, long nRecordID, CString strOldValue, CString strNewValue);

	// (z.manning, 07/30/2007) - This is used so we can tell if this is a "new user" import or an "existing
	// user" import (we default things differently depending on this option).
	NexFormsImportType m_eImportType;

	// (z.manning, 08/01/2007) - There are 2 main parts to the import:
	//  1. Packets & Word templates
	//  2. Procedures, tracking ladders, and anything else
	// Since we do the packets/templates import first and in a separate transaction (though importing
	// word templates is just files and not technically part of that transaction) we have this boolean
	// that we set when it finishes so that if the 2nd part of the import fails unexpectedly, then we
	// won't bother with the first part again.
	BOOL m_bPacketsAndTemplatesHaveBeenSuccessfullyImported;

	// (z.manning, 10/11/2007) - PLID 27719 - Tells us if we should update the font of any existng procedures
	// we're importing over.
	BOOL m_bUpdateExistingProceduresFont;
};

BOOL ReadVersionInfoFromNexFormsContentFile(IN CFile *pFile, IN OUT NexFormsImportInfo &info);
BOOL ReadVersionInfoFromNexFormsContentFile(IN CString strFile, OUT long &nCodeVersion, OUT long &nContentFileVersion, OUT CString &strAspsVersion, OUT long &nNexFormsVersion);
void LoadFromNexFormsContentFile(IN CString strFile, OUT NexFormsImportInfo &info);



#endif // !defined(AFX_NEXFORMSIMPORTINFO_H__36768314_2393_4A16_89AE_8F50A980354C__INCLUDED_)
