// NexFormsUtils.h: interface for the NexFormsUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEXFORMSUTILS_H__09A7E457_3221_420B_B358_25AB2839F8AE__INCLUDED_)
#define AFX_NEXFORMSUTILS_H__09A7E457_3221_420B_B358_25AB2839F8AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "GlobalDataUtils.h"
#include <SharedNexFormsUtils.h> // (z.manning 2011-06-24 17:20) - PLID 42916

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/


// (f.dinatale 2010-09-02) - PLID 40362 - Replacing the READ_NEXFORMS_STRING with a function call.
void ReadNexFormsString(CString & str, UINT & nBytesRead, CFile *pFile);

// (z.manning, 06/19/2007) - PLID 18359 - Functions for reading from the NexForms content file.
void ReadNexFormsProcedureFromFile(CFile *pFile, NexFormsProcedure *procedure);
void ReadNexFormsLadderFromFile(CFile *pFile, NexFormsLadder *ladder);
void ReadNexFormsPacketFromFile(CFile *pFile, NexFormsPacket *packet);
// (z.manning, 07/05/2007) - Returns the number of bytes read.
UINT ReadNexFormsWordTemplateFromFile(CFile *pFile, NexFormsWordTemplate *word);

// (z.manning, 08/02/2007) - This functions returns true if the specified procedure is a derm procedure.
BOOL IsDermProcedure(CString strProcedure);

// (z.manning, 08/30/2007) - Returns true if the 2 procedure names match, false otherwise.
BOOL IsSameProcedureName(CString strProcedure1, CString strProcedure2);

// (z.manning, 08/03/2007) - Move these functions here from ProcedureSectionEditDlg and added the
// rich edit control parameter.
// (d.moore 2007-06-29 12:25) - PLID 23863 - Adds in text at the begining of the section for 'Needs Reviewed'.
void InsertNeedsReviewedText(CRichEditCtrl *pRichEditCtrl);
// (d.moore 2007-06-29 11:40) - PLID 23863 - Strip out text that is normally displayed when a section is marked 'Needs Reviewed'
void RemoveNeedsReviewedText(CRichEditCtrl *pRichEditCtrl);

void UpdateFontForExistingProcedures(CArray<long,long> &arynProcedureIDs);


#endif // !defined(AFX_NEXFORMSUTILS_H__09A7E457_3221_420B_B358_25AB2839F8AE__INCLUDED_)
