// LabResultsTabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "LabResultsTabDlg.h"
#include "LabsSetupDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "RenameFileDlg.h"
#include "MultiSelectDlg.h"
#include "EditLabResultsDlg.h"
#include "LabEditDiagnosisDlg.h"
#include "EditComboBox.h"
#include "LabEntryDlg.h"
#include "MsgBox.h"
#include "EditLabStatusDlg.h"
#include "DecisionRuleUtils.h"
#include "SignatureDlg.h"
#include "NotesDlg.h"	// (j.dinatale 2010-12-27) - PLID 41591
#include "NxModalParentDlg.h"	// (j.dinatale 2010-12-27) - PLID 41591
#include "HL7Utils.h"
#include "ConfigureReportViewDlg.h"
#include "SingleSelectMultiColumnDlg.h"	// (j.dinatale 2013-03-04 15:58) - PLID 34339
#include "MedlinePlusUtils.h"
#include "MovePatientLabsDlg.h"
#include "NxAPI.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (b.spivey, July 10, 2013) - PLID 45194 - Some quick, arbitrary defines so we don't have magic numbers floating around. 
#define ID_SCAN_IMAGE		45194
#define ID_SCAN_PDF			45195
#define ID_SCAN_MULTIPDF	45196


// CLabResultsTabDlg dialog
//TES 11/20/2009 - PLID 36191 - Created.  The vast majority of code in this file was copied from LabEntryDlg.cpp

enum LabFlagColumns{
	lfcFlagID = 0,
	lfcName,
	lfcTodoPriority, //TES 8/6/2013 - PLID 51147
};

enum LabDescListColumns {
	ldlcLabID = 0,
	ldlcDiagID = 1,
	ldlcDesc = 2,
};

enum ClinicalDiagListColumns {
	cdlcLabID = 0,
	cdlcDiagID = 1,
	cdlcDesc = 2,
};

enum ClinicalDiagOptionsListColumns {
	cdolcID = 0,
	cdolcDesc = 1,
	cdolcDescForSorting = 2,
	cdolcHasLink = 3,
};

// (r.goldschmidt 2016-02-25 09:54) - PLID 68267 - possible values for has link field of clinical diag options list
enum FilterValues {
	fvNoLink = 0,
	fvHasLink = 1,
	fvSpecialShowAll = 2,
};

//TES 12/1/2008 - PLID 32191
enum LabStatusColumns{
	lscStatusID = 0,
	lscDescription,
};

//TES 5/2/2011 - PLID 43428
enum OrderStatusColumns {
	oscID = 0,
	oscDescription = 1,
	oscHL7Flag = 2,
};


IMPLEMENT_DYNAMIC(CLabResultsTabDlg, CNxDialog)

CLabResultsTabDlg::CLabResultsTabDlg(CLabEntryDlg *pParentDlg)
	: CNxDialog(CLabResultsTabDlg::IDD, pParentDlg)
{
	m_pLabEntryDlg = pParentDlg;
	m_nPatientID = -1;
	m_nInitialLabID = -1;
	m_nLabProcedureID = -1;
	m_ltType = ltInvalid;
	m_bControlsHidden = false; // (c.haag 2011-12-28) - PLID 41618
	m_pLabResultsAttachmentView = NULL;
	m_bLoadedResultFieldPositions = false;
	m_drsThis = new DialogRowStruct; // (b.spivey, July 18, 2013) - PLID 45194
}

CLabResultsTabDlg::~CLabResultsTabDlg()
{
	// (a.walling 2011-06-22 11:59) - PLID 44260 - This should all be done in OnDestroy, not the destructor.
}

void CLabResultsTabDlg::OnDestroy()
{	
	// (a.walling 2011-06-22 11:59) - PLID 44260 - Cleanup everything now that the window is going away
	try {
		// (r.gonet 06/12/2014) - PLID 40426 - Free the memory used by both maps.
		ClearResultsMap();
		ClearAttachedFilesMap();

		//unload any HTML reports
		UnloadHTMLReports();

		if (NULL != m_pLabResultsAttachmentView) {
			delete m_pLabResultsAttachmentView;
			m_pLabResultsAttachmentView = NULL;
		}
		// (b.spivey, July 17, 2013) - PLID 45194 - Make sure this gets deleted when we destroy the dialog.
		delete m_drsThis; 
		m_drsThis = NULL; 
	} NxCatchAll(__FUNCTION__)
	CNxDialog::OnDestroy();
}

// (c.haag 2011-01-27) - PLID 41618 - Returns the time on the server
COleDateTime CLabResultsTabDlg::GetServerTime()
{
	ADODB::_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS Now");
	return AdoFldDateTime(rs->Fields, "Now"); // We want an exception to be thrown if this is somehow eof
}

// (c.haag 2010-11-22 15:39) - PLID 40556 - Returns the number of results for the current lab
// (z.manning 2011-06-17 10:36) - PLID 44154 - This now returns all the result rows
//TES 11/6/2012 - PLID 53591 - Provide options to return rows for all specimens, only the current specimen, or check the preference
void CLabResultsTabDlg::GetResults(long nLabID, OUT CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResultRows, GetResultOptions gro)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	//TES 11/6/2012 - PLID 53591 - Include all results if a.) we're in the PDF view, b.) we've been explicitly told to, or c.) we've been told
	// to check the preference, and the preference says to do so.
	if(bInPDFView || gro == groAllSpecimens || (gro == groCheckPreference && GetRemotePropertyInt("SignAndAcknowledgeResultsForAllSpecimens", 0, 0, "<None>"))) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return;
		}
		arypLabRows.Add(pLabRow);
	}

	long nCount = 0;
	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pResultRow = pLabRow->GetFirstChildRow();
		while (pResultRow) {
			nCount++;
			// (z.manning 2011-06-17 10:45) - PLID 44154 - We now return the result rows to the caller
			arypResultRows.Add(pResultRow);
			pResultRow = pResultRow->GetNextRow();
		}
	}
}

// (c.haag 2010-11-23 16:17) - PLID 37372 - Returns TRUE if all the results for a specific lab are signed
BOOL CLabResultsTabDlg::AllResultsAreSigned(long nLabID)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	//TES 11/6/2012 - PLID 53591 - There's a preference now to sign/acknowledge all specimens
	if(bInPDFView || GetRemotePropertyInt("SignAndAcknowledgeResultsForAllSpecimens", 0, 0, "<None>")) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return FALSE; // No results, no signatures to view
		}
		arypLabRows.Add(pLabRow);
	}

	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pResultRow = pLabRow->GetFirstChildRow();
		while (pResultRow) {
			if (pResultRow->GetValue(lrtcSignatureInkData).vt == VT_NULL) {
				return FALSE;
			}
			pResultRow = pResultRow->GetNextRow();
		}
	}

	return TRUE;
}

// (c.haag 2010-12-02 09:59) - PLID 38633 - Returns TRUE if all the results for a specific lab are marked completed
BOOL CLabResultsTabDlg::AllResultsAreCompleted(long nLabID)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	if(bInPDFView) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return FALSE; // No results to complete
		}
		arypLabRows.Add(pLabRow);
	}

	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pResultRow = pLabRow->GetFirstChildRow();
		while (pResultRow) {
			if (-1 == VarLong(pResultRow->GetValue(lrtcCompletedBy), -1)) {
				return FALSE;
			}
			pResultRow = pResultRow->GetNextRow();
		}
	}

	return TRUE;
}

// (c.haag 2010-12-10 10:19) - PLID 40556 - Returns TRUE if all the results for a specific lab are acknowledged
BOOL CLabResultsTabDlg::AllResultsAreAcknowledged(long nLabID)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	//TES 11/6/2012 - PLID 53591 - There's a preference now to sign/acknowledge all specimens
	if(bInPDFView || GetRemotePropertyInt("SignAndAcknowledgeResultsForAllSpecimens", 0, 0, "<None>")) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return FALSE; // No results to acknowledge
		}
		arypLabRows.Add(pLabRow);
	}

	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pResultRow = pLabRow->GetFirstChildRow();
		while (pResultRow) 
		{
			if (GetTreeValue(pResultRow, lrfAcknowledgedBy, lrtcForeignKeyID).vt == VT_NULL) {
				return FALSE;
			}
			pResultRow = pResultRow->GetNextRow();
		}
	}

	return TRUE;
}

// (z.manning 2011-06-21 17:09) - PLID 44154
void CLabResultsTabDlg::GetUniqueSignatures(IN const CArray<IRowSettingsPtr,IRowSettingsPtr> &arypResultRows, OUT CLabCompletionInfoArray &aryLabSignatures)
{
	for(int nRowIndex = 0; nRowIndex < arypResultRows.GetCount(); nRowIndex++)
	{
		IRowSettingsPtr pResultRow = arypResultRows.GetAt(nRowIndex);
		LabCompletionInfo sig;
		sig.nUserID = VarLong(pResultRow->GetValue(lrtcSignedBy), -1);
		sig.dtDate = VarDateTime(pResultRow->GetValue(lrtcSignedDate), g_cdtInvalid);
		sig.dtDate.SetDateTime(sig.dtDate.GetYear(), sig.dtDate.GetMonth(), sig.dtDate.GetDay(), sig.dtDate.GetHour(), sig.dtDate.GetMinute(), 0);
		if(!aryLabSignatures.AlreadyExists(sig)) {
			aryLabSignatures.Add(sig);
		}
	}
}

// (c.haag 2010-11-23 16:17) - PLID 37372 - Formats the Signature button based on the current requisition and
// whether it has been signed
// (c.haag 2011-02-21) - PLID 41618 - We now pass in the active row. It can be a result, or a specimen, or null.
void CLabResultsTabDlg::FormatSignButtonText(IRowSettingsPtr pActiveRow)
{
	long nLabID = -1;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);
	BOOL bAllResultsSigned = AllResultsAreSigned(nLabID);
	// (f.gelderloos 2013-08-28 16:29) - PLID 57826
	BOOL bAllResultsAcknowledged = AllResultsAreAcknowledged(nLabID);
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypUnsignedResultRows;
	GetUnsignedResults(nLabID, arypUnsignedResultRows);

	// (c.haag 2011-02-21) - PLID 41618 - Update the static labels
	// (z.manning 2011-06-21 16:46) - PLID 44154 - Improved this to not be so dependent on the tree.
	{
		if (!bAllResultsSigned || arypResultRows.GetCount() == 0) {
			// (z.manning 2011-06-21 16:56) - PLID 44154 - There are either no results or some unsigned results in which
			// case let's set this to blank.
			SetDlgItemText(IDC_LAB_SIGNED_BY, "");
			m_nxstaticSignedByLabel.SetWindowText("");
			m_nxstaticLabSignedBy.SetToolTip("");
		} 
		else {
			// (z.manning 2011-06-21 17:32) - PLID 44154 - All results are signed.  Let's get a list of all unique signatures
			// and display info about them.
			CLabCompletionInfoArray aryLabSignatures;
			GetUniqueSignatures(arypResultRows, aryLabSignatures);
			CString strSignedByText;
			for(int nSigIndex = 0; nSigIndex < aryLabSignatures.GetCount(); nSigIndex++) {
				LabCompletionInfo sig = aryLabSignatures.GetAt(nSigIndex);
				strSignedByText += GetExistingUserName(sig.nUserID) + " at " + FormatDateTimeForInterface(sig.dtDate, DTF_STRIP_SECONDS, dtoDateTime) + ", ";
			}
			strSignedByText.TrimRight(", ");
			m_nxstaticLabSignedBy.SetWindowText(strSignedByText);
			m_nxstaticLabSignedBy.SetToolTip(strSignedByText);
			m_nxstaticSignedByLabel.SetWindowText("Signed By:");
		}
	}

	if (-2 == nLabID || 0 == arypResultRows.GetCount()) {
		// If no lab is selected or we have no results, change the window text to hint at the purpose of the signature button,
		// but disable the button until a lab is later selected.
		SetDlgItemText(IDC_LAB_SIGNATURE, "Sign Results");	
		GetDlgItem(IDC_LAB_SIGNATURE)->EnableWindow(FALSE);
		return;
	} else {
		GetDlgItem(IDC_LAB_SIGNATURE)->EnableWindow(TRUE);
	}

	// Update the button color. If at least one result needs signed, then it should be red. Otherwise, black.
	if (arypResultRows.GetCount() > 0 && !bAllResultsSigned) {
		m_signatureBtn.SetTextColor(RGB(255,0,0));
	} else {
		m_signatureBtn.SetTextColor(RGB(0,0,0));
	}

	if (1 == arypResultRows.GetCount()) 
	{
		if (arypResultRows.GetAt(0)->GetValue(lrtcSignatureInkData).vt != VT_NULL) {
			GetDlgItem(IDC_LAB_SIGNATURE)->SetWindowText("View Signature");
			GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(FALSE); // (j.luckoski 2013-03-21 10:42) - PLID 55424 - Hide upon this case
			
		} else {
			GetDlgItem(IDC_LAB_SIGNATURE)->SetWindowText("Sign Result");
		}
	} 
	else {
		// Multiple results. From here, we want the user to do one of the following:
		// - View the current signature if one exists
		// - Sign results
		// If all results are signed, deal only with viewing signatures
		if (bAllResultsSigned) {
			SetDlgItemText(IDC_LAB_SIGNATURE, "View Signature");
			GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(FALSE); // (j.luckoski 2013-03-21 10:42) - PLID 55424 - Hide
			GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(FALSE); // (f.gelderloos 2013-08-26 10:41) - PLID 57826
		}
		else if (arypUnsignedResultRows.GetSize() >= arypResultRows.GetSize()) {
			// The current result has no signature. We'll give the option to sign multiple results.
			GetDlgItem(IDC_LAB_SIGNATURE)->SetWindowText("Sign Results");
		}
		else {
			// The current result has a signature. Let the user decide whether to view the signature in a
			// context menu.
			SetDlgItemText(IDC_LAB_SIGNATURE, "Signature Actions");
		}		
	}
	// (f.gelderloos 2013-08-26 10:55) - PLID 57826
	if(!bAllResultsAcknowledged && !bAllResultsSigned) {
		GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(FALSE);
	}
}

// (z.manning 2011-06-21 17:09) - PLID 44154
void CLabResultsTabDlg::GetUniqueCompletionInfo(IN const CArray<IRowSettingsPtr,IRowSettingsPtr> &arypResultRows, OUT CLabCompletionInfoArray &aryCompletionInfo)
{
	for(int nRowIndex = 0; nRowIndex < arypResultRows.GetCount(); nRowIndex++)
	{
		IRowSettingsPtr pResultRow = arypResultRows.GetAt(nRowIndex);
		LabCompletionInfo info;
		info.nUserID = VarLong(pResultRow->GetValue(lrtcCompletedBy), -1);
		info.dtDate = VarDateTime(pResultRow->GetValue(lrtcCompletedDate), g_cdtInvalid);
		info.dtDate.SetDateTime(info.dtDate.GetYear(), info.dtDate.GetMonth(), info.dtDate.GetDay(), info.dtDate.GetHour(), info.dtDate.GetMinute(), 0);
		if(!aryCompletionInfo.AlreadyExists(info)) {
			aryCompletionInfo.Add(info);
		}
	}
}

// (c.haag 2010-12-02 10:28) - PLID 38633 - Formats the Completed button based on the current requisition and
// whether it has been completed
// (c.haag 2011-02-21) - PLID 41618 - Now takes in a row. pActiveRow can be a result or a specimen or null.
void CLabResultsTabDlg::FormatMarkCompletedButtonText(IRowSettingsPtr pActiveRow)
{
	long nLabID = -1;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);
	BOOL bAllResultsCompleted = AllResultsAreCompleted(nLabID);

	// (c.haag 2011-02-21) - PLID 41618 - Update the static labels
	{
		if (!bAllResultsCompleted || arypResultRows.GetCount() == 0) {
			// (z.manning 2011-06-21 16:56) - PLID 44154 - There are either no results or some incomplete results in which
			// case let's set this to blank.
			SetDlgItemText(IDC_LAB_COMPLETED_BY, "");
			m_nxstaticCompletedByLabel.SetWindowText("");
			m_nxstaticLabCompletedBy.SetToolTip("");
		}
		else {
			// (z.manning 2011-06-21 17:32) - PLID 44154 - All results are complete.  Let's get a list of all unique completion info
			// and display info about them.
			CLabCompletionInfoArray aryCompletionInfo;
			GetUniqueCompletionInfo(arypResultRows, aryCompletionInfo);
			CString strSignedByText;
			for(int nCompletionInfoIndex = 0; nCompletionInfoIndex < aryCompletionInfo.GetCount(); nCompletionInfoIndex++) {
				LabCompletionInfo info = aryCompletionInfo.GetAt(nCompletionInfoIndex);
				strSignedByText += GetExistingUserName(info.nUserID) + " at " + FormatDateTimeForInterface(info.dtDate, DTF_STRIP_SECONDS, dtoDateTime) + ", ";
			}
			strSignedByText.TrimRight(", ");
			m_nxstaticLabCompletedBy.SetWindowText(strSignedByText);
			m_nxstaticLabCompletedBy.SetToolTip(strSignedByText);
			m_nxstaticCompletedByLabel.SetWindowText("Completed By:");
		}
	}
	// This will always be the window text
	SetDlgItemText(IDC_LAB_MARK_COMPLETED, "Mark Lab\nCompleted");	

	if (-2 == nLabID || 0 == arypResultRows.GetCount()) {
		// If no lab is selected or we have no results, change the window text to hint at the purpose of the button,
		// but disable the button until a lab is later selected.		
		GetDlgItem(IDC_LAB_MARK_COMPLETED)->EnableWindow(FALSE);
		return;
	} else {
		GetDlgItem(IDC_LAB_MARK_COMPLETED)->EnableWindow(TRUE);
	}

	// Update the button color. If at least one result needs completed, then it should be red. Otherwise, black.
	if (!bAllResultsCompleted) {
		m_markCompletedBtn.SetTextColor(RGB(255,0,0));
	} else {
		m_markCompletedBtn.SetTextColor(RGB(0,0,0));
	}

	// If all results are completed, disable the button
	if (bAllResultsCompleted) {
		GetDlgItem(IDC_LAB_MARK_COMPLETED)->EnableWindow(FALSE);
		GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(FALSE); // (j.luckoski 2013-03-21 10:43) - PLID 55424 - Hide
	}
}

// (j.luckoski 2013-03-21 10:46) - PLID 55424 - Formats button to be visible or not. Name is a little misleading but I am
// sure eventually someone will have to make it say or do something else.
void CLabResultsTabDlg::FormatMarkAllCompleteButtonText(IRowSettingsPtr pActiveRow)
{

	long nLabID = -1;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}

	
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);
	BOOL bAllResultsCompleted = AllResultsAreCompleted(nLabID);
	BOOL bAllResultsSigned = AllResultsAreSigned(nLabID);
	BOOL bAllResultsAcknowledged = AllResultsAreAcknowledged(nLabID);


	if(!bAllResultsCompleted && !bAllResultsAcknowledged && !bAllResultsSigned  && -2 != nLabID ) {
		GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(FALSE);
	}
}

// (f.gelderloos 2013-08-28 15:58) - PLID 57826 Add formatting handling for the Acknowledge and Sign button
void CLabResultsTabDlg::FormatAcknowledgeAndSignButtonText(IRowSettingsPtr pActiveRow)
{

	long nLabID = -1;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);
	BOOL bAllResultsCompleted = AllResultsAreCompleted(nLabID);
	BOOL bAllResultsSigned = AllResultsAreSigned(nLabID);
	BOOL bAllResultsAcknowledged = AllResultsAreAcknowledged(nLabID);


	int x = arypResultRows.GetCount();
	// (f.gelderloos 2013-08-26 10:55) - PLID 57826
	if(!bAllResultsAcknowledged && !bAllResultsSigned &&
		(-2 != nLabID)) {
		GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(FALSE);
	}
}

// (z.manning 2011-06-21 17:09) - PLID 44154
void CLabResultsTabDlg::GetUniqueAcknowledgers(IN const CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResultRows, OUT CLabCompletionInfoArray &arystrAcknowledgers)
{
	for(int nRowIndex = 0; nRowIndex < arypResultRows.GetCount(); nRowIndex++)
	{
		IRowSettingsPtr pResultRow = arypResultRows.GetAt(nRowIndex);
		LabCompletionInfo info;
		info.nUserID = VarLong(GetTreeValue(pResultRow, lrfAcknowledgedBy, lrtcForeignKeyID), -1);
		info.dtDate = VarDateTime(GetTreeValue(pResultRow, lrfAcknowledgedOn, lrtcValue), g_cdtInvalid);
		info.dtDate.SetDateTime(info.dtDate.GetYear(), info.dtDate.GetMonth(), info.dtDate.GetDay(), info.dtDate.GetHour(), info.dtDate.GetMinute(), 0);
		if(!arystrAcknowledgers.AlreadyExists(info)) {
			arystrAcknowledgers.Add(info);
		}
	}
}

// (c.haag 2010-11-22 15:39) - PLID 40556 - Formats the Acknowledged button text based on the number
// of results and the state of the results.
// (c.haag 2011-02-21) - PLID 41618 - Now takes in a row. pActiveRow can be a result or a specimen or null.
void CLabResultsTabDlg::FormatAcknowledgedButtonText(IRowSettingsPtr pActiveRow)
{
	long nLabID = -1;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);
	BOOL bAllResultsAcknowledged = AllResultsAreAcknowledged(nLabID);
	BOOL bAllResultsSigned = AllResultsAreSigned(nLabID);
	
	// (c.haag 2011-02-23) - PLID 41618 - Update the acknowledged labels
	{
		if (!bAllResultsAcknowledged || arypResultRows.GetCount() == 0) {
			// (z.manning 2011-06-21 16:56) - PLID 44154 - There are either no results or some unacknowledged results in which
			// case let's set this to blank.
			m_nxstaticAcknowledgedBy.SetWindowText("");
			// (c.haag 2010-11-22 15:39) - PLID 40556 - New Acknowledged controls
			m_nxstaticAcknowledgedByLabel.SetWindowText("");
			m_nxstaticAcknowledgedBy.SetToolTip("");
		}
		else {
			// (z.manning 2011-06-21 17:32) - PLID 44154 - All results are acknowledged.  Let's get a list of all unique acknowledgers
			// and display info about them.
			CLabCompletionInfoArray aryAcknowledgeInfo;
			GetUniqueAcknowledgers(arypResultRows, aryAcknowledgeInfo);
			CString strSignedByText;
			for(int nAcknowledgeInfoIndex = 0; nAcknowledgeInfoIndex < aryAcknowledgeInfo.GetCount(); nAcknowledgeInfoIndex++) {
				LabCompletionInfo info = aryAcknowledgeInfo.GetAt(nAcknowledgeInfoIndex);
				strSignedByText += GetExistingUserName(info.nUserID) + " at " + FormatDateTimeForInterface(info.dtDate, DTF_STRIP_SECONDS, dtoDateTime) + ", ";
			}
			strSignedByText.TrimRight(", ");
			m_nxstaticAcknowledgedBy.SetWindowText(strSignedByText);
			m_nxstaticAcknowledgedBy.SetToolTip(strSignedByText);
			// (c.haag 2010-11-22 15:39) - PLID 40556 - New Acknowledged controls
			m_nxstaticAcknowledgedByLabel.SetWindowText("Acknowledged By:");
		}
	}

	if (-2 == nLabID || 0 == arypResultRows.GetCount()) {
		// If no lab is selected or we have no results, change the window text to hint at the purpose of the button,
		// but disable the button until a lab is later selected.
		SetDlgItemText(IDC_LAB_ACKNOWLEDGE, "Acknowledge Results");
		GetDlgItem(IDC_LAB_ACKNOWLEDGE)->EnableWindow(FALSE);
		return;
	}

	// Now update the button text and enable the button based on the current selection and result count.
	// When we get here, we KNOW there is a lab selected and we KNOW it has results.
	if (1 == arypResultRows.GetCount()) 
	{
		if (VT_NULL != GetTreeValue(arypResultRows.GetAt(0), lrfAcknowledgedBy, lrtcForeignKeyID).vt) {
			// Already acknowledged. This is effectively a reset button.
			SetDlgItemText(IDC_LAB_ACKNOWLEDGE, "Unacknowledge Result");
			GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(FALSE); // (j.luckoski 2013-03-21 10:47) - PLID 55424
		}
		else {
			SetDlgItemText(IDC_LAB_ACKNOWLEDGE, "Acknowledge Result");
		}
		GetDlgItem(IDC_LAB_ACKNOWLEDGE)->EnableWindow(TRUE);
	}
	else {
		// Special handling for when all results are acknowledged
		if (bAllResultsAcknowledged) {
			// If all results are acknowledged and no result is selected, there's nothing we can do
			SetDlgItemText(IDC_LAB_ACKNOWLEDGE, "Unacknowledge Result");
			GetDlgItem(IDC_LAB_ACKNOWLEDGE)->EnableWindow(TRUE);
			GetDlgItem(IDC_LAB_MARK_ALL_COMPLETE)->EnableWindow(FALSE); // (j.luckoski 2013-03-21 10:47) - PLID 55424
		}
		else {
			// If we get here, one or more results are not acknowledged. No matter what the
			// result selection is, we want them to be able to acknowledge anything that is not
			// already acknowledged. If we have a valid result selected and it's acknowledged, we
			// need to be able to unacknowledge it.
			//
			CArray<IRowSettingsPtr,IRowSettingsPtr> arypUnacknowledgedResults;
			GetUnacknowledgedResults(nLabID, arypUnacknowledgedResults);
			if (arypUnacknowledgedResults.GetSize() < arypResultRows.GetSize()) {
				// In the special case where a result is selected and it was acknowledged, we'll let the
				// user unacknowledge it.
				SetDlgItemText(IDC_LAB_ACKNOWLEDGE, "Acknowledgement Actions");
			}
			else {
				// In all other cases, just let them acknowledge results
				SetDlgItemText(IDC_LAB_ACKNOWLEDGE, "Acknowledge Results");
			}
			GetDlgItem(IDC_LAB_ACKNOWLEDGE)->EnableWindow(TRUE);
		}
	}

	// (f.gelderloos 2013-08-26 10:55) - PLID 57826
	if(!bAllResultsAcknowledged && !bAllResultsSigned) {
		GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_LAB_ACKNOWLEDGEANDSIGN)->EnableWindow(FALSE);
	}
}

void CLabResultsTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOC_PATH_LINK, m_nxlDocPath);
	DDX_Control(pDX, IDC_VALUE_LABEL, m_nxstaticValueLabel);
	DDX_Control(pDX, IDC_UNITS_LABEL, m_nxstaticUnitsLabel);
	DDX_Control(pDX, IDC_SLIDE_NUMBER_LABEL, m_nxstaticSlideLabel);
	DDX_Control(pDX, IDC_RESULT_NAME_LABEL, m_nxstaticNameLabel);
	DDX_Control(pDX, IDC_RESULT_LOINC_LABEL, m_nxlabelLOINC); // (a.walling 2010-01-18 10:38) - PLID 36936
	DDX_Control(pDX, IDC_REFERENCE_LABEL, m_nxstaticRefLabel);
	DDX_Control(pDX, IDC_RESULT_COMMENTS_LABEL, m_nxstaticResultCommentsLabel);
	DDX_Control(pDX, IDC_MICRO_DESC_LABEL, m_nxstaticMicroDesLabel);
	DDX_Control(pDX, IDC_FLAG_LABEL, m_nxstaticFlagLabel);
	DDX_Control(pDX, IDC_DIAGNOSIS_LABEL, m_nxstaticDiagnosisLabel);
	DDX_Control(pDX, IDC_DATE_RECEIVED_LABEL, m_nxstaticDateReceived);
	DDX_Control(pDX, IDC_ATTACH_FILE, m_btnAttachDoc);
	DDX_Control(pDX, IDC_DETACH_FILE, m_btnDetachDoc);
	DDX_Control(pDX, IDC_DELETE_RESULT, m_btnDeleteResult);
	DDX_Control(pDX, IDC_ADD_RESULT, m_btnAddResult);
	DDX_Control(pDX, IDC_BTN_ADD_DIAGNOSIS, m_btnAddDiagnosis);
	DDX_Control(pDX, IDC_DATE_RECEIVED, m_dtpDateReceived);
	DDX_Control(pDX, IDC_SLIDE_NUMBER, m_nxeditSlideNumber);
	DDX_Control(pDX, IDC_LAB_DIAG_DESCRIPTION, m_nxeditLabDiagDescription);
	DDX_Control(pDX, IDC_CLINICAL_DIAGNOSIS_DESC, m_nxeditClinicalDiagnosisDesc);
	DDX_Control(pDX, IDC_LAB_RESULT_LABEL, m_nxstaticLabResultLabel);
	DDX_Control(pDX, IDC_EDIT_FLAG, m_EditFlagBtn);
	DDX_Control(pDX, IDC_EDIT_LAB_DIAGNOSES, m_EditLabDiagnosesBtn);
	DDX_Control(pDX, IDC_EDIT_CLINICAL_DIAGNOSIS_LIST, m_EditClinicalDiagnosisList);
	DDX_Control(pDX, IDC_EDIT_RESULT_STATUS, m_nxbEditStatus);
	DDX_Control(pDX, IDC_RESULT_NAME, m_nxeditResultName);
	DDX_Control(pDX, IDC_RESULT_LOINC, m_nxeditResultLOINC); // (a.walling 2010-01-18 10:38) - PLID 36936
	DDX_Control(pDX, IDC_RESULT_REFERENCE, m_nxeditResultReference);
	DDX_Control(pDX, IDC_RESULT_VALUE, m_nxeditResultValue);
	DDX_Control(pDX, IDC_RESULT_UNITS, m_nxeditResultUnits);
	DDX_Text(pDX, IDC_RESULT_UNITS, m_strResultUnits);
	DDV_MaxChars(pDX, m_strResultUnits, 250);
	DDX_Control(pDX, IDC_RESULT_COMMENTS, m_nxeditResultComments);
	DDX_Control(pDX, IDC_ATTACHED_FILE_LABEL, m_nxstaticAttachedFileLabel);
	DDX_Control(pDX, IDC_ZOOM, m_btnZoom);
	DDX_Control(pDX, IDC_LAB_MARK_COMPLETED, m_markCompletedBtn);
	DDX_Control(pDX, IDC_LAB_MARK_ALL_COMPLETE, m_markAllCompleteBtn); // (j.luckoski 2013-03-21 10:48) - PLID 55424
	DDX_Control(pDX, IDC_LAB_ACKNOWLEDGEANDSIGN, m_AcknowledgeAndSignBtn);// (f.gelderloos 2013-08-26 10:57) - PLID 57826
	DDX_Control(pDX, IDC_LAB_SIGNATURE, m_signatureBtn); // (c.haag 2010-11-22 13:49) - PLID 37372
	DDX_Control(pDX, IDC_LAB_COMPLETED_BY, m_nxstaticLabCompletedBy);
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the completed date label
	//DDX_Control(pDX, IDC_LAB_RESULT_COMPLETED_DATE, m_nxstaticLabCompletedDate); // (c.haag 2010-12-02 16:23) - PLID 38633
	DDX_Control(pDX, IDC_COMPLETED_BY_LABEL, m_nxstaticCompletedByLabel); // (c.haag 2010-12-02 16:23) - PLID 38633
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the completed date heading label
	//DDX_Control(pDX, IDC_COMPLETED_RESULT_DATE_LABEL, m_nxstaticCompletedDateLabel); // (c.haag 2010-12-02 16:23) - PLID 38633
	DDX_Control(pDX, IDC_ACKNOWLEDGED_BY_LABEL, m_nxstaticAcknowledgedByLabel); // (c.haag 2010-11-22 15:39) - PLID 40556
	DDX_Control(pDX, IDC_ACKNOWLEDGED, m_nxstaticAcknowledgedBy);
	DDX_Control(pDX, IDC_SIGNED_BY_LABEL, m_nxstaticSignedByLabel); // (c.haag 2010-11-23 11:19) - PLID 37372
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the signed date heading label
	//DDX_Control(pDX, IDC_SIGNED_DATE_LABEL, m_nxstaticSignedDateLabel);
	DDX_Control(pDX, IDC_LAB_SIGNED_BY, m_nxstaticLabSignedBy); // (c.haag 2010-11-23 11:19) - PLID 37372
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the signed date label
	//DDX_Control(pDX, IDC_LAB_SIGNED_DATE, m_nxstaticLabSignedDate);
	DDX_Control(pDX, IDC_SCROLL_LEFT, m_btnScrollLeft); // (j.gruber 2010-11-24 11:28) - PLID 41607
	DDX_Control(pDX, IDC_SCROLL_RIGHT, m_btnScrollRight); // (j.gruber 2010-11-24 11:28) - PLID 41607
	DDX_Control(pDX, IDC_RESULT_SCROLL_LEFT, m_btnResultScrollLeft);
	DDX_Control(pDX, IDC_RESULT_SCROLL_RIGHT, m_btnResultScrollRight);
	DDX_Control(pDX, IDC_LABNOTES, m_btnNotes);
	DDX_Control(pDX, IDC_ADDLABNOTE, m_btnAddNote);
	DDX_Control(pDX, IDC_LAB_ORDER_STATUS_LABEL, m_nxsOrderStatusLabel);
	DDX_Control(pDX, IDC_EDIT_ORDER_STATUS, m_btnEditOrderStatus);
	DDX_Control(pDX, IDC_ANATAOMICAL_LOCATION_TEXT_DISCRETE, m_nxstaticAnatomicalLocationTextDiscrete);
	DDX_Control(pDX, IDC_ANATAOMICAL_LOCATION_TEXT_REPORT, m_nxstaticAnatomicalLocationTextReport);
	DDX_Control(pDX, IDC_PT_EDUCATION_LABEL, m_nxlabelPatientEducation);
	DDX_Control(pDX, IDC_BTN_PT_EDUCATION, m_btnPatientEducation);
}


BEGIN_MESSAGE_MAP(CLabResultsTabDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_DIAGNOSIS, &CLabResultsTabDlg::OnAddDiagnosis)
	ON_BN_CLICKED(IDC_EDIT_FLAG, &CLabResultsTabDlg::OnEditFlag)
	ON_BN_CLICKED(IDC_EDIT_LAB_DIAGNOSES, &CLabResultsTabDlg::OnEditLabDiagnoses)
	ON_BN_CLICKED(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST, &CLabResultsTabDlg::OnEditClinicalDiagnosisList)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CLabResultsTabDlg::OnLabelClick)
	ON_BN_CLICKED(IDC_ADD_RESULT, &CLabResultsTabDlg::OnAddResult)
	ON_BN_CLICKED(IDC_DELETE_RESULT, &CLabResultsTabDlg::OnDeleteResult)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_RECEIVED, &CLabResultsTabDlg::OnDateTimeChangedDateReceived)
	ON_BN_CLICKED(IDC_ATTACH_FILE, &CLabResultsTabDlg::OnAttachFile)
	ON_BN_CLICKED(IDC_DETACH_FILE, &CLabResultsTabDlg::OnDetachFile)
	ON_BN_CLICKED(IDC_EDIT_RESULT_STATUS, &CLabResultsTabDlg::OnEditResultStatus)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ZOOM, &CLabResultsTabDlg::OnZoom)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_PDFVIEW_RADIO, &CLabResultsTabDlg::OnBnClickedPdfviewRadio)
	ON_BN_CLICKED(IDC_REPORTVIEW_RADIO, &CLabResultsTabDlg::OnBnClickedReportviewRadio)
	ON_BN_CLICKED(IDC_DISCRETEVALUES_RADIO, &CLabResultsTabDlg::OnBnClickedDiscretevaluesRadio)
	ON_BN_CLICKED(IDC_LAB_MARK_COMPLETED, &CLabResultsTabDlg::OnLabMarkCompleted)
	ON_BN_CLICKED(IDC_LAB_SIGNATURE, OnSignature)
	ON_BN_CLICKED(IDC_LAB_ACKNOWLEDGE, OnAcknowledgeResult)
	ON_BN_CLICKED(IDC_SCROLL_LEFT, &CLabResultsTabDlg::OnBnClickedScrollLeft)
	ON_BN_CLICKED(IDC_SCROLL_RIGHT, &CLabResultsTabDlg::OnBnClickedScrollRight)
	ON_BN_CLICKED(IDC_RESULT_SCROLL_LEFT, OnBnClickedResultScrollLeft)
	ON_BN_CLICKED(IDC_RESULT_SCROLL_RIGHT, OnBnClickedResultScrollRight)
	ON_BN_CLICKED(IDC_LABNOTES, &CLabResultsTabDlg::OnViewNotes)
	ON_BN_CLICKED(IDC_ADDLABNOTE, &CLabResultsTabDlg::OnAddNote)
	ON_BN_CLICKED(IDC_EDIT_ORDER_STATUS, &CLabResultsTabDlg::OnEditOrderStatus)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CONFIGURE_REPORT_VIEW, &CLabResultsTabDlg::OnConfigureReportView)
	ON_BN_CLICKED(IDC_LAB_MARK_ALL_COMPLETE, &CLabResultsTabDlg::OnBnClickedLabMarkAllComplete) // (j.luckoski 2013-03-21 10:48) - PLID 55424
	ON_BN_CLICKED(IDC_LAB_ACKNOWLEDGEANDSIGN, &CLabResultsTabDlg::OnBnClickedLabAcknowledgeandsign)// (f.gelderloos 2013-08-26 11:20) - PLID 57826
	ON_WM_SHOWWINDOW() 
	ON_BN_CLICKED(IDC_BTN_PT_EDUCATION, &CLabResultsTabDlg::OnBtnPtEducation)
END_MESSAGE_MAP()


// CLabResultsTabDlg message handlers

void CLabResultsTabDlg::SetPatientID(long nPatientID)
{
	m_nPatientID = nPatientID;
}

//TES 11/30/2009 - PLID 36452 - We now set just the initial lab ID, as this tab may have multiple labs on it.
void CLabResultsTabDlg::SetInitialLabID(long nLabID)
{
	m_nInitialLabID = nLabID;
}

void CLabResultsTabDlg::SetLabProcedureID(long nLabProcedureID)
{
	m_nLabProcedureID = nLabProcedureID;
}

void CLabResultsTabDlg::SetLabProcedureType(LabType ltType)
{
	m_ltType = ltType;
}

// (c.haag 2011-12-28) - PLID 41618 - This function is used for quick sorting the attached file list.
// When finished, the array should be in order of attach date ascending.
int CompareAttachedLabFile(const void *a, const void *b) 
{
	CAttachedLabFile* pa = (CAttachedLabFile*)a;
	CAttachedLabFile* pb = (CAttachedLabFile*)b;
	if (pa->dtAttached > pb->dtAttached) {
		return 1;
	}
	else if (pa->dtAttached < pb->dtAttached) {
		return -1;
	}
	else {
		return 0;
	}
}

// (r.gonet 06/12/2014) - PLID 40426 - Clears the m_mapResults map.
void CLabResultsTabDlg::ClearResultsMap()
{
	POSITION pos = m_mapResults.GetStartPosition();
	stResults *pRes;
	long nKey;
	while (pos != NULL) {
		m_mapResults.GetNextAssoc(pos, nKey, pRes);
		delete pRes;
		m_mapResults.RemoveKey(nKey);
	}
}

// (r.gonet 06/12/2014) - PLID 40426 - Clears the m_mapAttachedFiles map.
void CLabResultsTabDlg::ClearAttachedFilesMap()
{
	POSITION pos = m_mapAttachedFiles.GetStartPosition();
	CMap<LPDISPATCH, LPDISPATCH, CString, CString&> *pMap = NULL;
	LPDISPATCH pKey;
	while (pos != NULL) {
		m_mapAttachedFiles.GetNextAssoc(pos, pKey, pMap);
		delete pMap;
		m_mapAttachedFiles.RemoveKey(pKey);
	}
}

//TES 11/20/2009 - PLID 36191 - Loads an existing lab (SetLabID() should have been called before this function).
void CLabResultsTabDlg::LoadExisting()
{
	//TES 11/22/2009 - PLID 36191 - Load all the results into our tree.
	m_pResultsTree->Clear();
	// (r.gonet 06/12/2014) - PLID 40426 - Free the memory used by both maps.
	ClearResultsMap();
	ClearAttachedFilesMap();	

	//TES 11/30/2009 - PLID 36452 - We now load all labs that have the same form number (and patient ID, just in case) as the "initial"
	// lab that we were given.
	// (a.walling 2010-01-18 10:17) - PLID 36936 - LOINC code
	// (a.walling 2010-02-25 15:49) - PLID 37546 - Date Performed
	// (z.manning 2010-05-12 10:30) - PLID 37400 - HL7 Message ID
	// (c.haag 2010-11-18 13:31) - PLID 37372 - Added signature fields
	// (c.haag 2010-12-02 10:28) - PLID 41590 - Added completed fields
	// (c.haag 2010-01-27) - PLID 41618 - Added attachdate
	//TES 4/28/2011 - PLID 43426 - Added DateReceivedByLab
	//TES 5/2/2011 - PLID 43428 - Added OrderStatusID
	//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
	//TES 8/6/2013 - PLID 51147 - Added FlagTodoPriority
	// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. Added specimen data
	// (d.singleton 2013-07-16 17:28) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
	// (d.singleton 2013-08-07 16:04) - PLID 57912 - need to show the Performing Provider on report view of labresult tab dlg
	// (d.singleton 2013-10-25 12:28) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
	//TES 9/10/2013 - PLID 58511 - Added LabWasReplaced and ResultWasReplaced
	//TES 11/5/2013 - PLID 59320 - Updated the ResultWasReplaced calculation, it now includes results that were replaced as part of a lab being replaced
	// (d.singleton 2013-11-26 10:20) - PLID 59379 - need to expand the funtionality of reflex tests to match on filler order number and other results of the same hl7 message file
	_RecordsetPtr rs = CreateParamRecordset("SELECT LabsT.ID AS LabID, LabsT.FormNumberTextID, LabsT.Specimen, "
		"LabResultsT.ResultID, LabResultsT.Name, DateReceived, SlideTextID, DiagnosisDesc, "
		"ClinicalDiagnosisDesc, FlagID, LabResultFlagsT.Name AS Flag, LabResultsT.Value, Reference, LabResultsT.MailID, MailSent.PathName, "
		"StatusID, LabResultStatusT.Description AS Status, Comments, Units, AcknowledgedUserID, AcknowledgedDate, "
		"LabResultsT.ResultCompletedDate, LabResultsT.ResultCompletedBy, CompletedUser.UserName AS ResultCompletedByUsername,  "
		"LabResultsT.ResultSignatureImageFile, LabResultsT.ResultSignatureInkData, LabResultsT.ResultSignatureTextData, "
		"UsersT.UserName AS AcknowledgedByUser, LabResultsT.LOINC, LabResultsT.DatePerformed, LabResultsT.HL7MessageID, "
		"LabResultsT.ResultSignedDate, LabResultsT.ResultSignedBy, SignedUser.UserName AS ResultSignedByUsername, "
		"MailSent.Date AS AttachDate, LabResultsT.DateReceivedByLab, LabsT.OrderStatusID, LabResultFlagsT.TodoPriority AS FlagTodoPriority, "
		"LabsT.SPecimenID, LabsT.SpecimenIdText, LabsT.SpecimenStartTime, LabsT.SpecimenEndTime, LabsT.SpecimenRejectReason, LabsT.SpecimenCondition, "
		"LabsT.ServiceStartTime, LabsT.ServiceEndTime, LabResultsT.PerformingProvider, LabResultsT.PerformingLabID, LocationsT.Name AS PerformingLabName, "
		"LocationsT.Address1 AS PerformingLabAddress, LocationsT.City AS PerformingLabCity, LocationsT.State AS PerformingLabState, "
		"LocationsT.Zip AS PerformingLabZip, LocationsT.Country AS PerformingLabCountry, LocationsT.ParishCode AS PerformingLabParish, LabResultsT.ObservationDate, "
		"convert(bit, CASE WHEN LinkedLabsQ.LinkedLabID Is Null THEN 0 ELSE 1 END) AS LabWasReplaced, "
		"convert(bit, CASE WHEN LinkedResultsQ.LinkedResultID Is Null THEN CASE WHEN LabResultsT.HL7MessageID NOT IN (SELECT HL7MessageID FROM LabResultsT WHERE LinkedLabID Is Not Null) AND LinkedLabsQ.LinkedLabID Is Not Null THEN 1 ELSE 0 END ELSE 1 END) AS ResultWasReplaced "
		"FROM LabsT INNER JOIN (SELECT * FROM LabsT WHERE ID = {INT}) AS RequestedLab "
		"ON LabsT.FormNumberTextID = RequestedLab.FormNumberTextID AND LabsT.PatientID = RequestedLab.PatientID "
		//(e.lally 2010-04-07) PLID 37374 - Remove the deleted results from the join, instead of removing the whole lab req. record
		"LEFT JOIN LabResultsT ON (LabsT.ID = LabResultsT.LabID OR LabsT.ParentResultID = LabResultsT.ResultID) AND LabResultsT.Deleted = 0 "
		"LEFT JOIN UsersT AS CompletedUser ON LabResultsT.ResultCompletedBy = CompletedUser.PersonID "
		"LEFT JOIN UsersT AS SignedUser ON LabResultsT.ResultSignedBy = SignedUser.PersonID "
		"LEFT JOIN LabResultFlagsT ON LabResultsT.FlagID = LabResultFlagsT.ID "
		"LEFT JOIN MailSent ON LabResultsT.MailID = MailSent.MailID "
		"LEFT JOIN LabResultStatusT ON LabResultsT.StatusID = LabResultStatusT.ID "
		"LEFT JOIN UsersT ON LabResultsT.AcknowledgedUserID = UsersT.PersonID "
		"LEFT JOIN (SELECT LinkedLabID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedLabID) LinkedLabsQ ON LabsT.ID = LinkedLabsQ.LinkedLabID "
		"LEFT JOIN (SELECT LinkedResultID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedResultID) LinkedResultsQ ON LabResultsT.ResultID = LinkedResultsQ.LinkedResultID "
		"LEFT JOIN LocationsT ON LabResultsT.PerformingLabID = LocationsT.ID "
		"WHERE LabsT.Deleted = 0 " 
		"AND {SQLFRAGMENT} "
		//(e.lally 2010-04-07) PLID 37374 - Remove the deleted results from the join, instead of removing the whole lab req. record,
		//otherwise we can't add new results to a req. that had all of its previous results deleted.
		//"AND COALESCE(LabResultsT.Deleted,0) = 0 "	
		// (d.singleton 2012-08-20 16:20) - PLID 42596 put in an order by on Specimen to make the results populate alphabetically.
		"ORDER BY LabsT.Specimen, LabsT.BiopsyDate DESC, LabsT.ID DESC, LabResultsT.DateReceived DESC", m_nInitialLabID, GetAllowedLocationClause_Param("LabsT.LocationID"));

	//TES 11/30/2009 - PLID 36452 - Our top level rows are now for labs, so track the current lab so we can tell when to make a new parent.
	long nCurrentLabID = -1;
	IRowSettingsPtr pLabRow = NULL;
	//We want all dates to default to an invalid date
	COleDateTime dtDefault = COleDateTime(0.00);
	dtDefault.SetStatus(COleDateTime::invalid);
	//TES 5/2/2011 - PLID 43428 - The Order Status will be the same for all rows, so only check it once
	bool bLoadedOrderStatus = false;
	while(!rs->eof) {
		//TES 11/30/2009 - PLID 36452 - First, is this a new lab?
		long nLabID = AdoFldLong(rs, "LabID");
		if(nLabID != nCurrentLabID) {
			//TES 11/30/2009 - PLID 36452 - It is, let's make a new parent row.
			nCurrentLabID = nLabID;
			CString strLabName = AdoFldString(rs, "FormNumberTextID","");
			CString strSpecimen = AdoFldString(rs, "Specimen","");
			if(!strSpecimen.IsEmpty()) {
				strLabName += " - " + strSpecimen;
			}
			pLabRow = GetNewResultsTreeRow();
			pLabRow->PutValue(lrtcResultID, g_cvarNull);
			pLabRow->PutValue(lrtcFieldID, (long)lrfLabName);
			pLabRow->PutValue(lrtcFieldName, _bstr_t("Form #"));
			pLabRow->PutValue(lrtcValue, _bstr_t(strLabName));
			pLabRow->PutValue(lrtcForeignKeyID, nLabID);
			pLabRow->PutValue(lrtcSpecimen, _bstr_t(strSpecimen));
			pLabRow->PutValue(lrtcHL7MessageID, g_cvarNull);
			// (b.spivey, April 09, 2013) - PLID 44387 - Assume false until we can find out otherwise. 
			pLabRow->PutValue(lrtcLoadedAsIncomplete, g_cvarFalse);
			//TES 9/10/2013 - PLID 58511 - Fill lrtcExtraValue with a flag for whether this lab has been replaced by a subsequent one
			pLabRow->PutValue(lrtcExtraValue, AdoFldBool(rs, "LabWasReplaced", FALSE)?g_cvarTrue:g_cvarFalse);
			// (d.singleton 2013-10-25 16:20) - PLID 59197 - need to move the specimen segment values to its own header in the html view for labs,  so it will show regardless of any results. 
			pLabRow->PutValue(lrtcSpecimenIDText, rs->Fields->Item["SpecimenIDText"]->Value);
			pLabRow->PutValue(lrtcSpecimenCollectionStartTime, rs->Fields->Item["SpecimenStartTime"]->Value);
			pLabRow->PutValue(lrtcSpecimenCollectionEndTime, rs->Fields->Item["SpecimenEndTime"]->Value);
			pLabRow->PutValue(lrtcSpecimenRejectReason, rs->Fields->Item["SpecimenRejectReason"]->Value);
			pLabRow->PutValue(lrtcSpecimenCondition, rs->Fields->Item["SpecimenCondition"]->Value);
			m_pResultsTree->AddRowAtEnd(pLabRow, NULL);
		}
		//TES 5/2/2011 - PLID 43428 - Pull the order status, if we haven't yet.
		if(!bLoadedOrderStatus) {
			IRowSettingsPtr pStatusRow = m_pOrderStatusCombo->SetSelByColumn(oscID, rs->Fields->Item["OrderStatusID"]->GetValue());
			if(pStatusRow) {
				m_strSavedOrderStatus = VarString(pStatusRow->GetValue(oscDescription));
			}
			else {
				m_strSavedOrderStatus = "<None>";
			}
			bLoadedOrderStatus = true;
		}
		long nResultID = AdoFldLong(rs, "ResultID", -1);
		if(nResultID != -1) {
			//TES 11/22/2009 - PLID 36191 - Track the results as we go, for auditing purposes.
			stResults *pstRes = new stResults;
			pstRes->nResultID = nResultID;
			IRowSettingsPtr pParentRow = GetNewResultsTreeRow();
			//TES 11/22/2009 - PLID 36191 - First, add the parent row, which is also the Name field.
			pParentRow->PutValue(lrtcResultID, nResultID);
			pParentRow->PutValue(lrtcFieldID, (long)lrfName);
			pParentRow->PutValue(lrtcFieldName, _bstr_t("Name"));
			_variant_t var = rs->Fields->GetItem("Name")->Value;
			pParentRow->PutValue(lrtcValue, var);
			pstRes->strResultName = VarString(var,"");
			pParentRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			// (z.manning 2010-05-12 10:37) - PLID 37400 - Added HL7 message ID
			pParentRow->PutValue(lrtcHL7MessageID, rs->GetFields()->GetItem("HL7MessageID")->GetValue());
			// (c.haag 2010-11-18 13:45) - PLID 37372 - Added columns for per-result signature info.
			pParentRow->PutValue(lrtcSignatureImageFile, _bstr_t(AdoFldString(rs->GetFields(), "ResultSignatureImageFile", "")));
			pParentRow->PutValue(lrtcSignatureInkData, rs->GetFields()->GetItem("ResultSignatureInkData")->Value);
			// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
			pParentRow->PutValue(lrtcSignatureTextData, rs->GetFields()->GetItem("ResultSignatureTextData")->Value);
			// (c.haag 2010-11-23 10:50) - PLID 37372 - Signed By fields
			pParentRow->PutValue(lrtcSignedBy, AdoFldLong(rs, "ResultSignedBy", -1));			
			pParentRow->PutValue(lrtcSignedUsername, _bstr_t(AdoFldString(rs, "ResultSignedByUsername", "")));
			COleDateTime dtSigned = AdoFldDateTime(rs, "ResultSignedDate", dtDefault);
			if (COleDateTime::valid == dtSigned.GetStatus()) {
				pParentRow->PutValue(lrtcSignedDate, _variant_t(dtSigned, VT_DATE));
			} else {
				pParentRow->PutValue(lrtcSignedDate, g_cvarNull);
			}
			pParentRow->PutValue(lrtcSavedSignedBy, pParentRow->GetValue(lrtcSignedBy));
			pParentRow->PutValue(lrtcSavedSignedDate, pParentRow->GetValue(lrtcSignedDate));

			// (c.haag 2010-12-02 10:28) - PLID 41590 - Completed Date
			COleDateTime dtCompleted = AdoFldDateTime(rs, "ResultCompletedDate", dtDefault);
			if (COleDateTime::valid == dtCompleted.GetStatus()) {
				pParentRow->PutValue(lrtcCompletedDate, _variant_t(dtCompleted, VT_DATE));
			} else {
				pParentRow->PutValue(lrtcCompletedDate, g_cvarNull);
			}
			// (c.haag 2010-12-02 10:28) - PLID 41590 - Completed By
			pParentRow->PutValue(lrtcCompletedBy, AdoFldLong(rs, "ResultCompletedBy", -1));
			pParentRow->PutValue(lrtcCompletedUsername, _bstr_t(AdoFldString(rs, "ResultCompletedByUsername", "")));
			pParentRow->PutValue(lrtcSavedCompletedBy, pParentRow->GetValue(lrtcCompletedBy));
			pParentRow->PutValue(lrtcSavedCompletedDate, pParentRow->GetValue(lrtcCompletedDate));
			//TES 9/10/2013 - PLID 58511 - Fill lrtcExtraValue with a flag indicating whether this result has been replaced by another one
			pParentRow->PutValue(lrtcExtraValue, AdoFldBool(rs, "ResultWasReplaced", FALSE)?g_cvarTrue:g_cvarFalse);

			//TES 11/22/2009 - PLID 36191 - Add it at the end.
			//TES 11/30/2009 - PLID 36452 - Add it as a child of the Lab row.
			m_pResultsTree->AddRowAtEnd(pParentRow, pLabRow);

			//TES 4/28/2011 - PLID 43426 - Date Received By Lab
			IRowSettingsPtr pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfDateReceivedByLab);
			pRow->PutValue(lrtcFieldName, _bstr_t("Date Rec'd (Lab)"));
			var = rs->Fields->GetItem("DateReceivedByLab")->Value;
			pRow->PutValue(lrtcValue, var);
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtReceivedByLab.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtReceivedByLab = VarDateTime(var);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Now add each of the other fields as child rows.
			//TES 11/22/2009 - PLID 36191 - Date Received
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfDateReceived);
			pRow->PutValue(lrtcFieldName, _bstr_t("Date Rec'd"));
			var = rs->Fields->GetItem("DateReceived")->Value;
			pRow->PutValue(lrtcValue, var);
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(var.vt == VT_NULL) {
				pRow->PutVisible(VARIANT_FALSE);
				//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
				pstRes->dtReceivedDate.SetStatus(COleDateTime::invalid);
			}
			else if (var.vt == VT_DATE) {
				COleDateTime dt = VarDateTime(var);
				if (dt.GetStatus() == COleDateTime::valid) {
					//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
					pstRes->dtReceivedDate = dt;
				}
				else {
					//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
					pstRes->dtReceivedDate.SetStatus(COleDateTime::invalid);
				}
			}
			else if (var.vt == VT_BSTR) {
				if (VarString(var).IsEmpty()) {
					//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
					pstRes->dtReceivedDate.SetStatus(COleDateTime::invalid);
				}
				else {
					COleDateTime dt;
					dt.ParseDateTime(VarString(var));
					if (dt.GetStatus() == COleDateTime::valid) {
						//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
						pstRes->dtReceivedDate = dt;
					}
					else {
						//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
						pstRes->dtReceivedDate.SetStatus(COleDateTime::invalid);
					}

				}
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (a.walling 2010-02-25 15:46) - PLID 37546 - Date performed
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfDatePerformed);
			pRow->PutValue(lrtcFieldName, _bstr_t("Date Perf'd"));
			var = rs->Fields->GetItem("DatePerformed")->Value;
			pRow->PutValue(lrtcValue, var);
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtPerformedDate.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtPerformedDate = VarDateTime(var);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (a.walling 2010-01-18 10:17) - PLID 36936 - LOINC code
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfLOINC);
			pRow->PutValue(lrtcFieldName, _bstr_t("LOINC"));
			var = rs->Fields->GetItem("LOINC")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strLOINC = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			//TES 11/22/2009 - PLID 36191 - Slide # is hidden on non-biopsy labs
			if(VarString(var, "").IsEmpty()) {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Value
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfValue);
			pRow->PutValue(lrtcFieldName, _bstr_t("Value"));
			//TES 11/22/2009 - PLID 36191 - Do we have a MailID?
			var = rs->Fields->GetItem("MailID")->Value;
			if(var.vt == VT_I4) {
				//TES 11/22/2009 - PLID 36191 - Yes, so this is a document
				pRow->PutValue(lrtcValue, rs->Fields->GetItem("PathName")->Value);
				pRow->PutValue(lrtcForeignKeyID, var);
				pstRes->nMailID = VarLong(var);
				pstRes->strDocPath = VarString(rs->Fields->GetItem("PathName")->Value,"");
				// (c.haag 2010-01-27) - PLID 41618 - Attachment date. We require one; GetServerTime should never be called except for bad data
				_variant_t vAttachedDate = rs->Fields->GetItem("AttachDate")->Value;
				if (VT_NULL == vAttachedDate.vt) {
					vAttachedDate = _variant_t(GetServerTime(), VT_DATE);
				}
				pRow->PutValue(lrtcDate, vAttachedDate);
				//TES 11/23/2009 - PLID 36192 - Remember that this file is attached to this result.
				//TES 1/27/2010 - PLID 36862 - Moved to its own function
				SetAttachedFile(pParentRow, pstRes->strDocPath);
			}
			else {
				//TES 11/22/2009 - PLID 36191 - No, so this is just the value.
				var = rs->Fields->GetItem("Value")->Value;
				pstRes->strValue = VarString(var,"");
				pRow->PutValue(lrtcValue, var);
				pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
				if(VarString(var,"") == "") {
					pRow->PutVisible(VARIANT_FALSE);
				}
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Slide #
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSlideNum);
			pRow->PutValue(lrtcFieldName, _bstr_t("Slide #"));
			var = rs->Fields->GetItem("SlideTextID")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strSlideNum = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			//TES 11/22/2009 - PLID 36191 - Slide # is hidden on non-biopsy labs
			if(m_ltType != ltBiopsy || VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Diagnosis
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfDiagnosis);
			pRow->PutValue(lrtcFieldName, _bstr_t("Diagnosis"));
			var = rs->Fields->GetItem("DiagnosisDesc")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strDiagnosis = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Microscopic Description
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfMicroscopicDescription);
			//TES 12/4/2009 - PLID 36191 - The newline is intentional; this is a data width column now.
			pRow->PutValue(lrtcFieldName, _bstr_t("Microscopic\r\nDesc."));
			var = rs->Fields->GetItem("ClinicalDiagnosisDesc")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strMicroDesc = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			//TES 11/22/2009 - PLID 36191 - Microscopic Description is hidden on non-biopsy labs
			if(m_ltType != ltBiopsy || VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Flag
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfFlag);
			pRow->PutValue(lrtcFieldName, _bstr_t("Flag"));
			var = rs->Fields->GetItem("FlagID")->Value;
			pstRes->nFlagID = VarLong(var,-1);
			_variant_t var2 = rs->Fields->GetItem("Flag")->Value;
			pRow->PutValue(lrtcValue, var2);
			pstRes->strFlag = VarString(var2,"");
			pRow->PutValue(lrtcForeignKeyID, var);
			if(var.vt == VT_NULL) {
				pRow->PutVisible(VARIANT_FALSE);
			}
			//TES 8/6/2013 - PLID 51147 - Store the TodoPriority in the tree
			var = rs->Fields->GetItem("FlagTodoPriority")->Value;
			pRow->PutValue(lrtcExtraValue, var);
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Status
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfStatus);
			pRow->PutValue(lrtcFieldName, _bstr_t("Status"));
			var = rs->Fields->GetItem("StatusID")->Value;
			pstRes->nStatusID = VarLong(var,-1);
			var2 = rs->Fields->GetItem("Status")->Value;
			pRow->PutValue(lrtcValue, var2);
			pstRes->strStatus = VarString(var2,"");
			pRow->PutValue(lrtcForeignKeyID, var);
			if(var.vt == VT_NULL) {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Reference
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfReference);
			pRow->PutValue(lrtcFieldName, _bstr_t("Reference"));
			var = rs->Fields->GetItem("Reference")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strReference = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Units
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfUnits);
			pRow->PutValue(lrtcFieldName, _bstr_t("Units"));
			var = rs->Fields->GetItem("Units")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strUnits = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Comments
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfComments);
			pRow->PutValue(lrtcFieldName, _bstr_t("Comments"));
			var = rs->Fields->GetItem("Comments")->Value;
			pRow->PutValue(lrtcValue, var);
			pstRes->strComments = VarString(var,"");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Acknowledged By
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfAcknowledgedBy);
			//TES 12/5/2009 - PLID 36191 - Since this row will alwasy be hidden, and the field name row is Data width now,
			// I abbreviated this description.
			pRow->PutValue(lrtcFieldName, _bstr_t("Ack. By"));
			var = rs->Fields->GetItem("AcknowledgedUserID")->Value;
			pRow->PutValue(lrtcForeignKeyID, var);
			pstRes->nAcknowledgedUserID = VarLong(var,-1);
			pRow->PutValue(lrtcValue, rs->Fields->GetItem("AcknowledgedByUser")->Value);
			//TES 11/22/2009 - PLID 36191 - We'll always hide this (consistent with old behavior.
			pRow->PutVisible(VARIANT_FALSE);
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			//TES 11/22/2009 - PLID 36191 - Acknowledged On
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfAcknowledgedOn);
			//TES 12/5/2009 - PLID 36191 - Since this row will alwasy be hidden, and the field name row is Data width now,
			// I abbreviated this description.
			pRow->PutValue(lrtcFieldName, _bstr_t("Ack. On"));
			var = rs->Fields->GetItem("AcknowledgedDate")->Value;
			pRow->PutValue(lrtcValue, var);
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			pstRes->dtAcknowledgedDate = VarDateTime(var,dtInvalid);
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			//TES 11/22/2009 - PLID 36191 - We'll always hide this (consistent with old behavior).
			pRow->PutVisible(VARIANT_FALSE);
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);	

			// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. add new specimen specific data from SPM segment
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSpecimenIdentifier);
			pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Id"));
			var = rs->Fields->GetItem("SpecimenID")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strSpecimenID = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
			// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. add new specimen specific data from SPM segment
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSpecimenIdText);
			pRow->PutValue(lrtcFieldName, _bstr_t("Specimen ID Text"));
			var = rs->Fields->GetItem("SpecimenIdText")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strSpecimenIdText = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
			// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. add new specimen specific data from SPM segment
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSpecimenStartTime);
			pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Collection Start Time"));
			var = rs->Fields->GetItem("SpecimenStartTime")->Value;
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			pRow->PutValue(lrtcValue, var);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtSpecimenStartTime.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtSpecimenStartTime = VarDateTime(var);
			}			 
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. add new specimen specific data from SPM segment
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSpecimenEndTime);
			pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Collection End Time"));
			var = rs->Fields->GetItem("SpecimenEndTime")->Value;
			pRow->PutValue(lrtcValue, var);
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtSpecimenEndTime.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtSpecimenEndTime = VarDateTime(var);
			}			 
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. add new specimen specific data from SPM segment
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSpecimenRejectReason);
			pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Reject Reason"));
			var = rs->Fields->GetItem("SpecimenRejectReason")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strRejectReason = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-06-20 14:33) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. add new specimen specific data from SPM segment
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfSpecimenCondition);
			pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Condition"));
			var = rs->Fields->GetItem("SpecimenCondition")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strCondition = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-07-16 17:30) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfServiceStartTime);
			pRow->PutValue(lrtcFieldName, _bstr_t("Lab Service Start Time"));
			var = rs->Fields->GetItem("ServiceStartTime")->Value;
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			pRow->PutValue(lrtcValue, var);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtServiceStartTime.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtServiceStartTime = VarDateTime(var);
			}			 
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-07-16 17:30) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfServiceEndTime);
			pRow->PutValue(lrtcFieldName, _bstr_t("Lab Service End Time"));
			var = rs->Fields->GetItem("ServiceEndTime")->Value;
			pRow->PutValue(lrtcValue, var);
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtServiceEndTime.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtServiceEndTime = VarDateTime(var);
			}			 
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-08-07 16:08) - PLID 57912 - need to show the Performing Provider on report view of labresult tab dlg
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerformingProvider);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Provider"));
			var = rs->Fields->GetItem("PerformingProvider")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerformingProvider = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-10-25 13:51) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerformingLab);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Name"));
			var = rs->Fields->GetItem("PerformingLabName")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerformingLab = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-04 17:01) - PLID 59294 - add observation date to the html view for lab results. 
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfObservationDate);
			pRow->PutValue(lrtcFieldName, _bstr_t("Observation Date"));
			var = rs->Fields->GetItem("ObservationDate")->Value;
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			pRow->PutValue(lrtcValue, var);
			if(var.vt != VT_DATE) {
				pRow->PutVisible(VARIANT_FALSE);
				pstRes->dtObservationDate.SetStatus(COleDateTime::null);
			} else {
				pstRes->dtObservationDate = VarDateTime(var);
			}			 
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-27 12:27) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerfLabAddress);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Address"));
			var = rs->Fields->GetItem("PerformingLabAddress")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerfLabAddress = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-27 12:27) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerfLabCity);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab City"));
			var = rs->Fields->GetItem("PerformingLabCity")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerfLabCity = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-27 12:27) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerfLabState);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab State"));
			var = rs->Fields->GetItem("PerformingLabState")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerfLabState = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-27 12:27) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerfLabZip);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Zip"));
			var = rs->Fields->GetItem("PerformingLabZip")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerfLabZip = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-27 12:27) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerfLabCountry);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Country"));
			var = rs->Fields->GetItem("PerformingLabCountry")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerfLabCountry = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

			// (d.singleton 2013-11-27 12:27) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
			pRow = GetNewResultsTreeRow();
			pRow->PutValue(lrtcResultID, nResultID);
			pRow->PutValue(lrtcFieldID, (long)lrfPerfLabParish);
			pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Parish"));
			var = rs->Fields->GetItem("PerformingLabParish")->Value;	
			pRow->PutValue(lrtcValue, var);
			pstRes->strPerfLabParish = VarString(var, "");
			pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
			if(VarString(var,"") == "") {
				pRow->PutVisible(VARIANT_FALSE);
			}
			m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
			
			pParentRow->PutExpanded(VARIANT_TRUE);
			m_mapResults.SetAt(nResultID, pstRes);
			//TES 12/7/2009 - PLID 36191 - Track which row this result is associated with.
			pstRes->pRow = pParentRow;
		}
		rs->MoveNext();
	}

	//TES 11/30/2009 - PLID 36452 - Make sure all our labs are expanded
	pLabRow = m_pResultsTree->GetFirstRow();
	while(pLabRow) {
		pLabRow->PutExpanded(VARIANT_TRUE);
		pLabRow = pLabRow->GetNextRow();
	}

	/*//TES 11/22/2009 - PLID 36191 - Now select the first row.
	//TES 11/30/2009 - PLID 36452 - Select the first child of the first row, if any.
	pLabRow = m_pResultsTree->GetFirstRow();
	if(pLabRow) {
		IRowSettingsPtr pFirstResultRow = pLabRow->GetFirstChildRow();
		if(pFirstResultRow == NULL) {
			m_pResultsTree->CurSel = pLabRow;
		}
		else {
			m_pResultsTree->CurSel = pFirstResultRow;
		}
	}
	LoadResult(m_pResultsTree->CurSel);*/

	// (c.haag 2010-12-29 09:30) - PLID 41618 - At this point in time, the attachments view is stilll empty, and all
	// the attachment information for all results have just been loaded into memory. The attachment view relies on this
	// information to determine what its default selection is, and now that the information has been loaded, it can do
	// that determination now.
	if (m_pLabResultsAttachmentView) {
		m_pLabResultsAttachmentView->SetDefaultSelection();
	} else {
		ThrowNxException("m_pLabResultsAttachmentView was null when trying to initialize!");
	}

	//TES 12/18/2009 - PLID 36665 - We don't want the first row, we want the row corresponding to our initial lab.
	SetCurrentLab(m_nInitialLabID);


	// (b.spivey, April 09, 2013) - PLID 44387 - Check for any labs that loaded as incomplete. 
	CArray<long, long> aryLabIDs; 
	GetAllLabIDsAry(aryLabIDs); 
	for (int i = 0; i < aryLabIDs.GetCount(); i++) {
		if(!AllResultsAreCompleted(aryLabIDs.GetAt(i)) || !DoesLabHaveResults(aryLabIDs.GetAt(i))) {
			IRowSettingsPtr pRow = GetLabRowByID(aryLabIDs.GetAt(i));
			pRow->PutValue(lrtcLoadedAsIncomplete, g_cvarTrue); 
		}
	}
}

//TES 11/20/2009 - PLID 36191 - Loads a new lab.
void CLabResultsTabDlg::LoadNew()
{
	//TES 11/20/2009 - PLID 36191 - We don't really need to load anything here, since a new lab won't have any results.
	// Just clear everything out.
	//TES 11/30/2009 - PLID 36452 - Actually, just add a new lab
	AddNew();
}

void CLabResultsTabDlg::AddNew()
{
	//TES 11/30/2009 - PLID 36452 - Simply create a new top-level row, and add it.
	IRowSettingsPtr pLabRow = GetNewResultsTreeRow();
	pLabRow->PutValue(lrtcResultID, g_cvarNull);
	pLabRow->PutValue(lrtcFieldID, (long)lrfLabName);
	pLabRow->PutValue(lrtcFieldName, _bstr_t("Form #"));
	pLabRow->PutValue(lrtcValue, _bstr_t(m_pLabEntryDlg->GetFormNumber()));
	pLabRow->PutValue(lrtcForeignKeyID, (long)-1);
	pLabRow->PutValue(lrtcSpecimen, _bstr_t(""));

	// (c.haag 2010-12-02 10:28) - PLID 38633 - Completed info
	pLabRow->PutValue(lrtcCompletedDate, g_cvarNull);
	pLabRow->PutValue(lrtcCompletedBy, (long)-1);
	pLabRow->PutValue(lrtcCompletedUsername, "");
	pLabRow->PutValue(lrtcSavedCompletedBy, pLabRow->GetValue(lrtcCompletedBy));
	pLabRow->PutValue(lrtcSavedCompletedDate, pLabRow->GetValue(lrtcCompletedDate));
	//TES 9/10/2013 - PLID 58511 - Lab rows now check this field
	pLabRow->PutValue(lrtcExtraValue, g_cvarFalse);

	// (b.spivey, April 09, 2013) - PLID 44387 - Any new lab is considered incomplete automatically. 
	pLabRow->PutValue(lrtcLoadedAsIncomplete, g_cvarTrue); 

	// (j.gruber 2011-02-18 16:24) - PLID 41606 - add it at the top instead
	m_pResultsTree->AddRowBefore(pLabRow, m_pResultsTree->GetFirstRow());
	pLabRow->PutExpanded(VARIANT_TRUE);
	m_pResultsTree->CurSel = pLabRow;
	LoadResult(pLabRow);

	// (j.dinatale 2010-12-14) - PLID 41438 - Only if the view hasnt been set yet, do we set the current view.
	//		Also, the default view should be the pdf view, as per the preference's default. If we already set up the view,
	//		set it again because of the z-order issue with the web browser active x control
	// (j.gruber 2010-12-08 13:37) - PLID 41662
	if(m_nCurrentView == rvNotSet){
		SetCurrentResultsView(GetRemotePropertyInt("LabResultUserView", (long)rvPDF, 0, GetCurrentUserName(), true));
	}else{
		SetCurrentResultsView(m_nCurrentView);
	}
}

extern CPracticeApp theApp;
BOOL CLabResultsTabDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (a.walling 2011-06-22 11:59) - PLID 44260 - If we want to keep the DLL loaded, notify the MainFrame that we will be using it now.
		GetMainFrame()->HoldAdobeAcrobatReference();

		m_btnAddDiagnosis.AutoSet(NXB_NEW);
		m_btnZoom.AutoSet(NXB_INSPECT);
		
		// (r.gonet 06/24/2014) - PLID 61685 - Removed the 255 character limit on the Result Comments box.

		if (NULL == (m_pLabResultsAttachmentView = new CLabResultsAttachmentView(*this))) {
			ThrowNxException("Could not initialize the result attachment view!");
		}

		// (j.jones 2010-05-27 15:26) - PLID 38863 - you now need special permission to edit this field
		if(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic2) {
			m_nxeditResultComments.SetReadOnly(FALSE);
		}
		else {
			m_nxeditResultComments.SetReadOnly(TRUE);
		}

		// (a.walling 2010-01-18 10:43) - PLID 36936
		m_nxeditResultLOINC.SetLimitText(20);
		m_nxlabelLOINC.SetText("LOINC:");
		m_nxlabelLOINC.SetType(dtsHyperlink);
		// (r.gonet 2014-01-27 15:29) - PLID 59339 - Init the preferences to show or hide the patient education related controls.
		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		// (j.jones 2013-10-18 09:46) - PLID 58979 - added infobutton abilities
		m_btnPatientEducation.SetIcon(IDI_INFO_ICON);
		m_nxlabelPatientEducation.SetText("Pat. Edu.");
		
		// (r.gonet 2014-01-27 10:21) - PLID 59339 - Have a preference to toggle patient education options off.
		if(!bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Hide the patient education info button
			m_btnPatientEducation.ShowWindow(SW_HIDE);
		}

		if(bShowPatientEducationLink) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Show the patient education link
			m_nxlabelPatientEducation.SetType(dtsHyperlink);
		} else if(!bShowPatientEducationLink && bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We just need the label as a description of the info button.
			m_nxlabelPatientEducation.SetType(dtsText);
		} else if(!bShowPatientEducationLink && !bShowPatientEducationButton) {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - We don't need the label as either a link or a description label, so hide it.
			m_nxlabelPatientEducation.ShowWindow(SW_HIDE);
		}

		m_pClinicalDiagOptionsList = BindNxDataList2Ctrl(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST);
		//TES 11/28/2008 - PLID 32191
		m_pStatusCombo = BindNxDataList2Ctrl(IDC_LAB_STATUS);

		m_pFlagCombo = BindNxDataList2Ctrl(IDC_LAB_FLAG);

		//TES 5/2/2011 - PLID 43428
		m_pOrderStatusCombo = BindNxDataList2Ctrl(IDC_LAB_ORDER_STATUS);
		m_pOrderStatusCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		IRowSettingsPtr pNoneRow = m_pOrderStatusCombo->GetNewRow();
		pNoneRow->PutValue(oscID, (long)-1);
		pNoneRow->PutValue(oscDescription, _bstr_t("<None>"));
		pNoneRow->PutValue(oscHL7Flag, _bstr_t(""));
		m_pOrderStatusCombo->AddRowBefore(pNoneRow, m_pOrderStatusCombo->GetFirstRow());

		m_btnAddResult.AutoSet(NXB_NEW);
		m_btnDeleteResult.AutoSet(NXB_DELETE);
		m_btnAttachDoc.AutoSet(NXB_MODIFY);
		m_btnDetachDoc.AutoSet(NXB_MODIFY);
		m_markAllCompleteBtn.AutoSet(NXB_MODIFY); // (j.luckoski 3-21-13) PLID 55424 - Set color to new button
		m_AcknowledgeAndSignBtn.SetTextColor(RGB(0,0,0));// (f.gelderloos 2013-08-26 10:57) - PLID 57826

		// (j.gruber 2010-11-24 11:27) - PLID 41607 - scroll buttons
		m_btnScrollLeft.AutoSet(NXB_LEFT);
		m_btnScrollRight.AutoSet(NXB_RIGHT);
		// (c.haag 2010-12-06 13:48) - PLID 41618 - Result scroll buttons
		m_btnResultScrollLeft.AutoSet(NXB_LEFT);
		m_btnResultScrollRight.AutoSet(NXB_RIGHT);

		m_nxlDocPath.SetColor(0x00FFB9A8);
		m_nxlDocPath.SetText("");
		m_nxlDocPath.SetType(dtsHyperlink);

		//TES 12/14/2009 - PLID 36585 - Make a couple of the labels bold.
		m_nxstaticLabResultLabel.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		m_nxstaticAttachedFileLabel.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_pResultsTree = BindNxDataList2Ctrl(IDC_RESULTS_TREE, false);

		//TES 11/23/2009 - PLID 36192 - Initialize our web browswer for previewing attached files.
		m_pBrowser = GetDlgItem(IDC_PDF_PREVIEW)->GetControlUnknown();
		//TES 11/23/2009 - PLID 36192 - Now set it to blank, and disable it.
		COleVariant varUrl("about:blank");
		if (m_pBrowser) {
			m_pBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
			// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
			m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
			GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(FALSE);
		}		

		//TES 2/3/2010 - PLID 37191 - Remember that we're not previewing anything.
		m_strCurrentFileName = "";
		//TES 2/3/2010 - PLID 37191 - Also disable the "Zoom" button.
		UpdateZoomButton();

		// (j.dinatale 2010-12-27) - PLID 41591 - more preferences need to be cached. Specifically the Lab notes location
		//		when in pdf and the default note category
		// (j.dinatale 2010-12-13) - PLID 41438 - Bulk Cache properties
		// (j.dinatale 2011-01-31) - PLID 41438 - make sure username is formatted properly
		// (r.gonet 03/07/2013) - PLID 43599 - Bulk cache the property for stripping extra whitespace.
		// (r.gonet 03/07/2013) - PLID 44465 - Bulk cache the property for font type.
		g_propManager.BulkCache("LabEntryDlg", propbitNumber, 
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"'LabAttachmentsDefaultCategory', "
			"'Labs_PromptForCommonMicroscopic', "
			"'SignatureCheckPasswordLab', "
			"'LabResultUserView', "
			"'LabNotesLevelPDFView', "
			"'LabNotesDefaultCategory', "
			"'SignAndAcknowledgeResultsForAllSpecimens', "
			"'LabReportViewFontType', "
			"'LabReportViewTrimExtraSpaces'"

		")", _Q(GetCurrentUserName()));

		// (j.dinatale 2010-12-14) - PLID 41438
		m_nCurrentView = rvNotSet;

		// (j.dinatale 2010-12-22) - PLID 41591 - set the icon to be the regular note icon by default
		m_btnNotes.AutoSet(NXB_NOTES);

		// (j.dinatale 2010-12-23) - PLID 41591 - set the icon of the add note button to be a plus icon
		m_btnAddNote.AutoSet(NXB_NEW);

	}NxCatchAll("Error in CLabResultsTabDlg::OnInitDialog()");

	return TRUE;
}

//TES 11/20/2009 - PLID 36191 - Call to handle any processing that should take place after loading a lab record.
void CLabResultsTabDlg::PostLoad()
{
	//TES 11/22/2009 - PLID 36191 - We don't actually need to do anything here, showing/hiding fields is handled by LoadResult().

	// (j.dinatale 2010-12-14) - PLID 41438 - not sure why this was here, but it was causing some trouble.
	// (k.messina 2011-11-16) - PLID 41438 - setting the default view to NexTech's report
	//SetCurrentResultsView(GetRemotePropertyInt("LabResultUserView", (long)rvNexTechReport, 0, GetCurrentUserName(), true));
}

// (b.spivey, August 28, 2013) - PLID 46295 - This should get called after the tab is opened, so we're 
//		gonna make sure the values in these text controls are ensured. 
void CLabResultsTabDlg::EnsureData()
{
	CString strVal = GetAnatomicalLocationAsString(GetCurrentLab());
	m_nxstaticAnatomicalLocationTextDiscrete.SetWindowTextA(strVal);
	m_nxstaticAnatomicalLocationTextReport.SetWindowTextA(strVal);
}

//TES 11/20/2009 - PLID 36191 - Saves and audits all results.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
void CLabResultsTabDlg::Save(long nNewLabID, long &nAuditTransactionID, BOOL &bSpawnToDo, IN OUT CDWordArray &arNewCDSInterventions)
{
	CString strSql;
	
	bSpawnToDo = FALSE;
	
	CString strPersonName = GetExistingPatientName(m_nPatientID);

	// (j.gruber 2010-02-24 10:01) - PLID 37510 - need array for new/saved results
	CDWordArray dwaryResultIDs;

	//find what resultID we are at in case we have any additions
	AddStatementToSqlBatch(strSql, "SET NOCOUNT ON \r\n");
	//AddDeclarationToSqlBatch(strSql, " DECLARE @nStartResultID  INT; \r\n");
	//AddDeclarationToSqlBatch(strSql, " SET @nStartResultID  = (SELECT COALESCE(Max(ResultID), 0)+1 FROM LabResultsT) \r\n");
	AddStatementToSqlBatch(strSql, "DECLARE @TempLabInsertsT TABLE ( \r\n"
			"	ID int, ArrayIndex INT )");

	//now do any detaches of documents
	for (int k=0; k < m_aryDetachedDocs.GetSize(); k++) {
		AddStatementToSqlBatch(strSql, "UPDATE LabResultsT SET MailID = NULL WHERE ResultID = %li ", m_aryDetachedDocs.GetAt(k));		
	}

	//now see if there are any deletions
	for (int i = 0; i < m_aryDeleteResults.GetSize(); i++) {
		long nResultID = m_aryDeleteResults.GetAt(i);
		AddStatementToSqlBatch(strSql, " UPDATE LabResultsT SET DELETED = 1, DeleteDate = GetDate(), DeletedBy = %li WHERE ResultID = %li; \r\n", GetCurrentUserID(), nResultID);
		if (nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		stResults *pDeletedRes;
		CString strAuditString;
		if (m_mapResults.Lookup(nResultID, pDeletedRes) ) {
			strAuditString = "Name: " + pDeletedRes->strResultName + ";";
			//TES 4/28/2011 - PLID 43426 - Date Received By Lab
			if(pDeletedRes->dtReceivedByLab.GetStatus() == COleDateTime::valid) {
				CString strDate = FormatDateTimeForInterface(pDeletedRes->dtReceivedByLab, NULL, dtoDate);
				strAuditString += "Date Received By Lab: " + strDate + ";";
			}
			else {
				strAuditString += "Date Received By Lab: ;";
			}

			//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
			if (pDeletedRes->dtReceivedDate.GetStatus() == COleDateTime::valid) {
				CString strDate = FormatDateTimeForInterface(pDeletedRes->dtReceivedDate, NULL, dtoDate);
				strAuditString += "Date Received: " + strDate + ";";
			}
			else {
				strAuditString += "Date Received: ;";
			}
			// (a.walling 2010-02-25 15:52) - PLID 37546
			if (pDeletedRes->dtPerformedDate.GetStatus() == COleDateTime::valid) {
				strAuditString += "Date Performed: " + FormatDateTimeForInterface(pDeletedRes->dtPerformedDate, NULL, dtoDate) + ";";
			} else {
				strAuditString += "Date Performed: ;";
			}
			// (a.walling 2010-01-18 10:52) - PLID 36936
			strAuditString += "LOINC: " + pDeletedRes->strLOINC + ";";
			strAuditString += "Slide: " + pDeletedRes->strSlideNum + ";";
			strAuditString += "Diagnosis: " + pDeletedRes->strDiagnosis + ";";
			strAuditString += "Microscopic Desc: " + pDeletedRes->strMicroDesc + ";";
			strAuditString += "Flag: " + pDeletedRes->strFlag + ";";
			strAuditString += "Value: " + pDeletedRes->strValue + ";";
			// (c.haag 2009-05-06 15:21) - PLID 33789
			strAuditString += "Units: " + pDeletedRes->strUnits + ";";
			strAuditString += "Reference: " + pDeletedRes->strReference + ";";
			// (z.manning 2009-04-30 16:55) - PLID 28560
			strAuditString += "Comments: " + pDeletedRes->strComments + ';';
			// (c.haag 2009-05-07 14:35) - PLID 28561
			if (pDeletedRes->nAcknowledgedUserID > -1) {
				strAuditString += "Acknowledged By: " + GetExistingUserName(pDeletedRes->nAcknowledgedUserID) + ";";
				strAuditString += "Acknowledged Date: " + FormatDateTimeForInterface(pDeletedRes->dtAcknowledgedDate, DTF_STRIP_SECONDS, dtoDateTime); 
			}
		}

		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultDeleted, nResultID, strAuditString, "<Deleted>", aepHigh, aetDeleted);
		
	}

	//now add/update all the new items
	//TES 11/30/2009 - PLID 36452 - Loop through each lab.
	NXDATALIST2Lib::IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow();
	CPtrArray aryNewRows;
	//TES 5/2/2011 - PLID 43428 - Grab the order status (it's the same for all labs).
	IRowSettingsPtr pOrderStatusRow = m_pOrderStatusCombo->CurSel;
	long nOrderStatusID = -1;
	CString strOrderStatus = "<None>";
	if(pOrderStatusRow) {
		nOrderStatusID = pOrderStatusRow->GetValue(oscID);
		strOrderStatus = VarString(pOrderStatusRow->GetValue(oscDescription));
	}			
		
	// (b.spivey, March 19, 2013) - PLID 55943 - Array for lab IDs. 
	CArray<long, long> aryLabIDs; 
	while (pLabRow) {
		long nLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1);
		if(nLabID == -1) {
			//TES 11/30/2009 - PLID 36452 - This is our one and only new lab, update it with the new ID we've been given.
			nLabID = nNewLabID;
			pLabRow->PutValue(lrtcForeignKeyID, nNewLabID);
		}

		//TES 5/2/2011 - PLID 43428 - First, update the Order Status for this lab.
		AddStatementToSqlBatch(strSql, "UPDATE LabsT SET OrderStatusID = %s WHERE ID = %li", nOrderStatusID == -1 ? "NULL" : AsString(nOrderStatusID), nLabID);
		//TES 5/2/2011 - PLID 43428 - Now audit the order status
		if(strOrderStatus != m_strSavedOrderStatus) {
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strFormNumberTextID = m_pLabEntryDlg->GetFormNumber();
			IRowSettingsPtr pSpecRow = GetSpecFromCurrentRow(pLabRow);
			CString strSpecimen = VarString(pSpecRow->GetValue(lrtcSpecimen),"");
			CString strOld = strFormNumberTextID + " - " + strSpecimen + ": " + m_strSavedOrderStatus;
			CString strNew = strOrderStatus;					
			AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiPatientLabOrderStatus, nLabID, strOld, strNew, aepMedium, aetChanged);
			//TES 5/2/2011 - PLID 43428 - We don't update the saved value here, we've changed the status for every specimen so we want
			// to audit for every specimen.
		}
		IRowSettingsPtr pParentRow = pLabRow->GetFirstChildRow();
		while(pParentRow) {
			// (c.haag 2009-05-07 14:57) - PLID 28561 - Added acknowledged fields
			long nResultID, nFlagID, nMailID, nStatusID, nAcknowledgedUserID;
			// (c.haag 2009-05-06 14:48) - PLID 33789 - Added Units
			// (a.walling 2010-01-18 10:46) - PLID 36936 - Added LOINC
			CString strName, strReceivedDate, strSlideNum, strDiagnosis, strMicroDesc, strFlagID, strValue, strUnits, strReference, strFlagName, strDocPath, strMailID, strStatus, strStatusID, strResultComments, strAcknowledgedDate, strLOINC;
			//TES 12/1/2008 - PLID 32281 - Track the actual date received, not just a string representation of it.
			COleDateTime dtReceived, dtAcknowledgedDate;
			// (c.haag 2010-01-27) - PLID 41618 - Attached date
			COleDateTime dtAttached;
			//TES 8/6/2013 - PLID 51147 - Default the todo priority to the no-flag default, and track whether we've overridden it with a flag priority
			long nTodoPriority = GetRemotePropertyInt("Lab_DefaultTodoPriority", 1, 0, "<None>");
			bool bFlagFound = false;
			
			// (a.walling 2010-02-25 15:55) - PLID 37546 - Added Date Performed
			// (a.walling 2010-02-25 15:59) - PLID 37546 - Actually, modification of date performed is on hold for now.
			//CString strPerformedDate;
			//COleDateTime dtPerformed;
			
			//TES 11/22/2009 - PLID 36191 - Pull the name out of our parent-level row.
			nResultID = VarLong(pParentRow->GetValue(lrtcResultID));
			strName = VarString(pParentRow->GetValue(lrtcValue), "");

			// (a.walling 2010-01-18 10:46) - PLID 36936
			_variant_t varLOINC = GetTreeValue(pParentRow, lrfLOINC, lrtcValue);
			if (varLOINC.vt == VT_BSTR) {
				strLOINC = VarString(varLOINC);
			}
			
			//TES 11/22/2009 - PLID 36191 - Now go through and pull the other values out of our tree (use GetTreeValue, it will find the right row).
			_variant_t varDateReceived = GetTreeValue(pParentRow, lrfDateReceived, lrtcValue);
			if (varDateReceived.vt == VT_DATE) {
				dtReceived = VarDateTime(varDateReceived);
				if (dtReceived.GetStatus() == COleDateTime::valid) {
					strReceivedDate = FormatDateTimeForSql(dtReceived);
				}
				else {
					strReceivedDate = "";
				}
			}
			else if (varDateReceived.vt == VT_BSTR) {
				if (VarString(varDateReceived).IsEmpty()) {
					strReceivedDate = "";
					dtReceived.SetStatus(COleDateTime::invalid);
				}
				else {
					dtReceived.ParseDateTime(VarString(varDateReceived));
					if (dtReceived.GetStatus() == COleDateTime::valid) {
						strReceivedDate = FormatDateTimeForSql(dtReceived);
					}
					else {
						strReceivedDate = "";
						dtReceived.SetStatus(COleDateTime::invalid);
					}
				}
			}
			else {
				strReceivedDate = "";
				dtReceived.SetStatus(COleDateTime::invalid);
			}

			// (a.walling 2010-02-25 15:56) - PLID 37546 - Date performed
			// (a.walling 2010-02-25 15:59) - PLID 37546 - Actually, modification of date performed is on hold for now.
			/*
			_variant_t varDatePerformed = GetTreeValue(pParentRow, lrfDatePerformed, lrtcValue);
			if (varDatePerformed.vt == VT_DATE) {
				dtPerformed = VarDateTime(varDatePerformed);
				if (dtPerformed.GetStatus() == COleDateTime::valid) {
					strPerformedDate = FormatDateTimeForSql(dtPerformed);
				}
				else {
					strPerformedDate = "";
				}
			}
			else {
				// this should not be anything other than a date or nothing.
				ASSERT(varDatePerformed.vt == VT_NULL || varDatePerformed.vt == VT_EMPTY);
				strPerformedDate = "";
				dtPerformed.SetStatus(COleDateTime::invalid);
			}
			*/

			_variant_t varValue = GetTreeValue(pParentRow, lrfSlideNum, lrtcValue);
			if(VT_BSTR == varValue.vt) {
				strSlideNum = VarString(varValue, "");
			}
			
			strDiagnosis = VarString(GetTreeValue(pParentRow, lrfDiagnosis, lrtcValue), "");

			varValue = GetTreeValue(pParentRow, lrfMicroscopicDescription, lrtcValue); // (r.galicki 2008-10-21 13:00) - PLID 31552
			if(VT_BSTR == varValue.vt) {
				strMicroDesc = VarString(varValue, "");
			}
			
			nFlagID = VarLong(GetTreeValue(pParentRow, lrfFlag,lrtcForeignKeyID), -1);
			strFlagName = VarString(GetTreeValue(pParentRow, lrfFlag, lrtcValue), "");
			if (nFlagID == -1) {
				strFlagID = "NULL";
			}
			else {
				//TES 8/6/2013 - PLID 51147 - If our priority hasn't already been overridden, or if our stored priority is lower than this priority,
				// then update the stored priority to match this priority
				// (j.armen 2014-07-08 10:06) - PLID 62150 - The value at the flag can be null.  In this case, just use the default priority.
				strFlagID.Format("%li", nFlagID);
				long nTmpPriority = VarLong(GetTreeValue(pParentRow, lrfFlag, lrtcExtraValue), nTodoPriority);
				if(!bFlagFound) {
					nTodoPriority = nTmpPriority;
					bFlagFound = true;
				}
				else {
					if(nTmpPriority < nTodoPriority) {
						nTodoPriority = nTmpPriority;
					}
				}
			}
			
			nMailID = VarLong(GetTreeValue(pParentRow, lrfValue,lrtcForeignKeyID),-1);
			// (c.haag 2010-01-27) - PLID 41618 - Fill dtAttached
			if (nMailID != -1) {
				dtAttached = VarDateTime(GetTreeValue(pParentRow, lrfValue, lrtcDate));
			} else {
				dtAttached = g_cdtInvalid;
			}
			// (j.gruber 2010-01-05 09:46) - PLID 36485
			BOOL bTryAttach = FALSE;
			//-2 indicates a document was added
			if (nMailID == -2) {
				//it kinda sucks, but we need to attach the document here so that we have the mailID
				strDocPath = VarString(GetTreeValue(pParentRow, lrfValue, lrtcValue), "");
				bTryAttach = TRUE;
				// (c.haag 2010-01-27) - PLID 41618 - Expect a CAttachedLabFile value
				CAttachedLabFile af = AttachFileToLab(strDocPath, dtAttached);
				nMailID = af.nMailID;
			}
			if (nMailID == -1) {

				if (bTryAttach) {
					// (j.gruber 2010-01-04 16:47) - PLID 36485 - let them know what is happening
					if (nResultID == -1) {
						MsgBox("The file %s could not be attached.  The Value field will be cleared.", strDocPath);
						strMailID = "NULL";
						strValue = "";
					}
					else {
						//we'll get it down below
					}
				}
				else {
					strMailID = "NULL";
					strValue = VarString(GetTreeValue(pParentRow, lrfValue,lrtcValue), "");
				}
			}
			else {
				strMailID.Format("%li", nMailID);
				// (s.dhole 2010-10-21 15:57) - PLID 36938 set new tree value from mailid
				SetTreeValue(pParentRow,  lrfValue,lrtcForeignKeyID  ,nMailID);  
				// (s.dhole 2010-10-21 15:57) - PLID 36938 get new file name from database
				CString  strDocPathTemp = GetDocumentName(nMailID);
				if (!strDocPathTemp.IsEmpty()) 
				{
					// (j.dinatale 2011-01-06) - PLID 42031 - if we end up with a MailID, then that means the file path must've changed, make sure that the current
					//		file being displayed is the one on the server, not on the local machine.
					if(m_strCurrentFileName.CompareNoCase(strDocPath) == 0){
						m_strCurrentFileName = GetPatientDocumentPath(m_nPatientID) ^ strDocPathTemp;
						// (j.dinatale 2011-03-01) - PLID 42031 - need to make sure we arent in report view, or in some cases the pdf gets loaded over the report
						if(m_nCurrentView != rvNexTechReport){
							ReloadCurrentPDF();
						}
					}

					strDocPath =strDocPathTemp;
					// (s.dhole 2010-10-21 15:57) - PLID 36938 set new tree value from File Path
					SetTreeValue(pParentRow,  lrfValue,lrtcValue  ,_variant_t(strDocPath)); 
					SetAttachedFile(pParentRow,strDocPath); 
				
				}
				//strDocPath = VarString(GetTreeValue(pParentRow, lrfValue,lrtcValue), "");
			}

			strUnits = VarString(GetTreeValue(pParentRow, lrfUnits,lrtcValue), ""); // (c.haag 2009-05-06 14:49) - PLID 33789

			strReference = VarString(GetTreeValue(pParentRow, lrfReference,lrtcValue), "");
			
			// (z.manning 2009-04-30 16:49) - PLID 28560
			strResultComments = VarString(GetTreeValue(pParentRow, lrfComments,lrtcValue), "");

			//TES 12/1/2008 - PLID 32191 - Track the Status field.
			nStatusID = VarLong(GetTreeValue(pParentRow, lrfStatus,lrtcForeignKeyID), -1);
			strStatus = VarString(GetTreeValue(pParentRow, lrfStatus,lrtcValue), "");
			if (nStatusID == -1) {
				strStatusID = "NULL";
			}
			else {
				strStatusID.Format("%li", nStatusID);			
			}

			// (c.haag 2009-05-07 14:57) - PLID 28561 - Acknowledged fields
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			nAcknowledgedUserID = VarLong(GetTreeValue(pParentRow, lrfAcknowledgedBy, lrtcForeignKeyID), -1);
			dtAcknowledgedDate = VarDateTime(GetTreeValue(pParentRow, lrfAcknowledgedOn, lrtcValue), dtInvalid);
			if (COleDateTime::valid == dtAcknowledgedDate.GetStatus()) {
				strAcknowledgedDate = FormatDateTimeForSql(dtAcknowledgedDate);
			} else {
				strAcknowledgedDate.Empty();
			}

			// (c.haag 2010-11-30 17:34) - PLID 37372 - Result signature fields
			CString strSignatureFileName = VarString(pParentRow->GetValue(lrtcSignatureImageFile), "");
			_variant_t varSignatureInkData = pParentRow->GetValue(lrtcSignatureInkData);
			_variant_t varSignatureTextData = pParentRow->GetValue(lrtcSignatureTextData);
			long nSignedByUserID = VarLong(pParentRow->GetValue(lrtcSignedBy),-1);
			COleDateTime dtSigned = VarDateTime(pParentRow->GetValue(lrtcSignedDate), dtInvalid);
			// (c.haag 2010-12-02 16:48) - PLID 41590 - Save completed fields
			long nCompletedByUserID = VarLong(pParentRow->GetValue(lrtcCompletedBy),-1);
			COleDateTime dtCompleted = VarDateTime(pParentRow->GetValue(lrtcCompletedDate), dtInvalid);

			CString strSigInkData;
			if(varSignatureInkData.vt == VT_NULL || varSignatureInkData.vt == VT_EMPTY) {
				strSigInkData = "NULL";
			} else {
				strSigInkData = CreateByteStringFromSafeArrayVariant(varSignatureInkData);
			}
			CString strSigTextData;
			if(varSignatureTextData.vt == VT_NULL || varSignatureTextData.vt == VT_EMPTY) {
				strSigTextData = "NULL";
			} else {
				strSigTextData = CreateByteStringFromSafeArrayVariant(varSignatureTextData);
			}
			CString strSignedDate = "NULL";
			if(dtSigned.GetStatus() == COleDateTime::valid && dtSigned > COleDateTime(0.00)) {
				strSignedDate = "'" + FormatDateTimeForSql(dtSigned) + "'";
			}
			CString strSignedBy = "NULL";
			if(nSignedByUserID > 0){
				strSignedBy.Format("%li", nSignedByUserID);
			}
			CString strCompletedDate = "NULL";
			if(dtCompleted.GetStatus() == COleDateTime::valid && dtCompleted > COleDateTime(0.00)) {
				strCompletedDate = "'" + FormatDateTimeForSql(dtCompleted) + "'";
			}
			CString strCompletedBy = "NULL";
			if (nCompletedByUserID > 0) {
				strCompletedBy.Format("%li", nCompletedByUserID);
			}
			
			if (nResultID == -1) {

				bSpawnToDo = TRUE;
				CString strTempDesc;
				strTempDesc.Format("Result:%s was added with Flag: %s", strName, strFlagName);
				//TES 8/6/2013 - PLID 51147 - Pass in the priority we calculated
				m_pLabEntryDlg->AddToDoDescription(strTempDesc, (TodoPriority)nTodoPriority, true);

				//its a new result
				//TES 12/1/2008 - PLID 32191 - Added StatusID
				// (z.manning 2009-04-30 16:56) - PLID 28560 - Added comments
				// (c.haag 2009-05-06 15:21) - PLID 33789 - Added Units
				// (a.walling 2010-01-18 10:48) - PLID 36936 - Added LOINC
				// (c.haag 2010-11-30 17:34) - PLID 37372 - Added result signature fields
				// (c.haag 2010-12-10 9:44) - PLID 41590 - Added result complete fields
				AddStatementToSqlBatch(strSql, "INSERT INTO LabResultsT "
					"(LabID, Name, DateReceived, SlideTextID, DiagnosisDesc, ClinicalDiagnosisDesc, FlagID, Value, Units, Reference, MailID, StatusID, Comments, AcknowledgedUserID, AcknowledgedDate, LOINC, "
					"ResultSignatureInkData, ResultSignatureImageFile, ResultSignatureTextData, ResultSignedDate, ResultSignedBy, "
					"ResultCompletedDate, ResultCompletedBy "
					") VALUES "
					" (%li, '%s', %s, '%s', '%s', '%s', %s, '%s', '%s', '%s', %s, %s, '%s', %s, %s, '%s', "
					"%s, '%s', %s, %s, %s, "
					"%s, %s "
					"); \r\n ", 
					nLabID, _Q(strName), 
					strReceivedDate.IsEmpty() ? "NULL" : "'" + strReceivedDate + "'", _Q(strSlideNum), _Q(strDiagnosis), _Q(strMicroDesc), strFlagID, _Q(strValue), _Q(strUnits), _Q(strReference), strMailID, strStatusID, _Q(strResultComments),
					(-1 == nAcknowledgedUserID) ? "NULL" : AsString(nAcknowledgedUserID), 
					(strAcknowledgedDate.IsEmpty()) ? "NULL" : ("'" + strAcknowledgedDate + "'"),_Q(strLOINC),
					strSigInkData, _Q(strSignatureFileName), strSigTextData, strSignedDate, strSignedBy,
					strCompletedDate, strCompletedBy
					);
				
				AddStatementToSqlBatch(strSql, "INSERT INTO @TempLabInsertsT (ID, ArrayIndex) SELECT SCOPE_IDENTITY(), %li", aryNewRows.GetSize());
						
				stResults *pRes = new stResults;
				//TES 12/7/2009 - PLID 36191 - The result ID will be filled in after we save.
				pRes->strResultName = strName;
				//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
				pRes->dtReceivedDate = dtReceived;
				// (a.walling 2010-01-18 10:48) - PLID 36936
				pRes->strLOINC = strLOINC;
				pRes->strSlideNum = strSlideNum;
				pRes->strDiagnosis = strDiagnosis;
				pRes->strMicroDesc = strMicroDesc;
				pRes->nFlagID = nFlagID;
				pRes->strFlag = strFlagName;
				pRes->strValue = strValue;
				pRes->strUnits = strUnits; // (c.haag 2009-05-06 15:22) - PLID 33789
				pRes->strReference = strReference;
				pRes->nMailID = nMailID;
				pRes->strDocPath = strDocPath;
				pRes->nStatusID = nStatusID;
				pRes->strStatus = strStatus; //TES 12/1/2008 - PLID 32191
				pRes->strComments = strResultComments; // (z.manning 2009-04-30 16:57) - PLID 28560
				pRes->nAcknowledgedUserID = nAcknowledgedUserID; // (c.haag 2009-05-07 14:56) - PLID 28561
				pRes->dtAcknowledgedDate = dtAcknowledgedDate; // (c.haag 2009-05-07 14:57) - PLID 28561
				//TES 12/7/2009 - PLID 36191 - Track which row this result is associated with.
				pRes->pRow = pParentRow;
				//mail ID is going to be taken care of with the value field			
				aryNewRows.Add(((stResults *)pRes));


				// (b.spivey, March 26, 2013) - PLID 55943 - Add this lab ID to the list to check for. 
				if (nSignedByUserID > 0) {
					aryLabIDs.Add(nLabID); 
				}
			}
			else {				

				// (j.gruber 2010-02-24 13:56) - PLID 37510 
				BOOL bCheckLab = FALSE;


				//we updated something
				bool bForceUpdate = false;	// (j.dinatale 2013-03-07 17:02) - PLID 34339 - in one case we want to force an update
				CString strUpdate = "UPDATE LabResultsT SET ";
				CString strUpdateName, strUpdateValue, strCheckValue;
				CString strAuditOldName, strAuditNewName;
				long nAuditID;
				enum upType {
					upNumber = 0,
					upString,
					upDate,
				} updateType;


				//TES 11/22/2009 - PLID 36191 - First check the name (parent row)
				stResults *pstRes;
				if (!m_mapResults.Lookup(nResultID, pstRes)) {
					ASSERT(FALSE);
				}
				strUpdateName = "Name";
				strCheckValue = pstRes->strResultName;
				strUpdateValue = strName;
				nAuditID = aeiLabResultName;
				strAuditOldName = strCheckValue;
				strAuditNewName = strUpdateValue;
				if (strCheckValue != strUpdateValue) {					

					CString strTemp;
					strTemp.Format(" %s = '%s', ", strUpdateName, _Q(strUpdateValue));
						
					strUpdate += strTemp;
				
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					if (strUpdateValue == "NULL") {
						strUpdateValue = "";
					}
					if (strCheckValue == "NULL") {
						strCheckValue = "";
					}
					if (nAuditID != -1) {
						AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, nAuditID, nResultID, strAuditOldName, strAuditNewName, aepMedium, aetChanged);
					}

					// (j.gruber 2010-02-24 13:56) - PLID 37510 
					bCheckLab = TRUE;
				}

				//TES 11/22/2009 - PLID 36191 - Now loop through the child rows
				IRowSettingsPtr pRow = pParentRow->GetFirstChildRow();
				while(pRow) {
					bForceUpdate = false;	// (j.dinatale 2013-03-07 17:02) - PLID 34339 - in one case we want to force an update

					stResults *pstRes;
					if (!m_mapResults.Lookup(nResultID, pstRes)) {
						ASSERT(FALSE);
					}

					// (j.jones 2010-04-21 10:27) - PLID 38300 - reset these fields
					// for each row, they are only filled if we are saving a change
					strUpdateName = "";
					strCheckValue = "";
					strUpdateValue = "";

					LabResultField lrf = (LabResultField)VarLong(pRow->GetValue(lrtcFieldID));
					switch (lrf) {

						// (j.jones 2010-04-21 10:27) - PLID 38300 - Date Performed is a potential field
						// in the list, but it is not currently updateable, so reset strUpdateName to be empty
						case lrfDatePerformed:
							strUpdateName = "";
							strCheckValue = "";
							strUpdateValue = "";
							break;

						case lrfDateReceived:
							strUpdateName = "DateReceived";
							updateType = upDate;
							//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
							if (pstRes->dtReceivedDate.GetStatus() != COleDateTime::valid) {
								strCheckValue = "NULL";
								updateType = upDate;
								strAuditOldName = "";
							}
							else {
								strCheckValue = FormatDateTimeForSql(pstRes->dtReceivedDate);
								updateType = upString;
								if (pstRes->dtReceivedDate.GetStatus() == COleDateTime::valid) {
									strAuditOldName = FormatDateTimeForInterface(pstRes->dtReceivedDate);
								}
								else {
									ASSERT(FALSE);
									strAuditOldName = "";
								}
							}

							if (dtReceived.GetStatus() != COleDateTime::valid) {
								strUpdateValue = "NULL";
								updateType = upDate;
								strAuditNewName = "";
							}
							else {
								strUpdateValue = FormatDateTimeForSql(dtReceived);
								updateType = upString;
								if (dtReceived.GetStatus() == COleDateTime::valid) {
									strAuditNewName = FormatDateTimeForInterface(dtReceived);
								}
								else {
									ASSERT(FALSE);
									strAuditNewName = "";
								}
							}
							
							nAuditID = aeiLabResultReceivedDate;					
						break;

						// (a.walling 2010-01-18 10:50) - PLID 36936
						case lrfLOINC:
							strUpdateName = "LOINC";
							updateType = upString;
							strCheckValue = pstRes->strLOINC;
							strUpdateValue = strLOINC;
							nAuditID = aeiLabResultLOINC;	
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						case lrfSlideNum:
							strUpdateName = "SlideTextID";
							updateType = upString;
							strCheckValue = pstRes->strSlideNum;
							strUpdateValue = strSlideNum;
							nAuditID = aeiLabResultSlideNumber;	
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						case lrfDiagnosis:
							strUpdateName = "DiagnosisDesc";
							updateType = upString;
							strCheckValue = pstRes->strDiagnosis;
							strUpdateValue = strDiagnosis;
							nAuditID = aeiLabResultDiagnosis;
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						case lrfMicroscopicDescription:
							strUpdateName = "ClinicalDiagnosisDesc";
							updateType = upString;
							strCheckValue = pstRes->strMicroDesc;
							strUpdateValue = strMicroDesc;
							nAuditID = aeiLabResultMicroDesc;
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						case lrfFlag:
							strUpdateName = "FlagID";
							updateType = upNumber;
							if (pstRes->nFlagID == -1) {
								strCheckValue = "NULL";
								strAuditOldName = "";
							}
							else {
								strCheckValue = AsString(pstRes->nFlagID);
								strAuditOldName = pstRes->strFlag;
							}

							if (nFlagID == -1) {
								strUpdateValue = "NULL";
								strAuditNewName = "";
							}
							else {
								strUpdateValue = AsString(nFlagID);
								strAuditNewName = strFlagName;
							}
							nAuditID = aeiLabResultFlagID;
						break;

						case lrfValue:
							{
							
								// (j.gruber 2010-01-05 09:46) - PLID 36485
								BOOL bTryAttach2 = FALSE;

								if(nMailID == -2) {
									//attach the document here, since we need the mail ID
									bTryAttach2 = TRUE;
									// (c.haag 2010-01-27) - PLID 41618 - Expect a CAttachedLabFile value
									CAttachedLabFile af = AttachFileToLab(strDocPath, dtAttached);
									nMailID = af.nMailID;
								}
								if(nMailID > 0) {
									bForceUpdate = true;	// (j.dinatale 2013-03-07 17:02) - PLID 34339 - we want to force an update here
									strUpdateName = "MailID";
									updateType = upNumber;
									if (pstRes->nMailID == -1) {
										strCheckValue = "NULL";
									}
									else {
										strCheckValue = AsString(pstRes->nMailID);
									}

									strUpdateValue = AsString(nMailID);

									// (j.dinatale 2011-01-06) - PLID 42031 - if we end up with a MailID, then that means the file path maybe changed, make sure that the current
									//		file being displayed the one on the server instead of the local copy.
									if(m_strCurrentFileName.CompareNoCase(strDocPath) == 0){
										CString  strDocPathTemp = GetDocumentName(nMailID);
										if (!strDocPathTemp.IsEmpty()) 
										{
											m_strCurrentFileName = GetPatientDocumentPath(m_nPatientID) ^ strDocPathTemp;
											// (j.dinatale 2011-03-01) - PLID 42031 - need to make sure we arent in report view, or in some cases the pdf gets loaded over the report
											if(m_nCurrentView != rvNexTechReport){
												ReloadCurrentPDF();
											}
										}
									}
								}
								else {
									if (bTryAttach || bTryAttach2) {
										MsgBox("The file %s could not be attached.  The Value field will be reverted.", strDocPath);
										//don't update anything
										strUpdateName = "";
										updateType = upString;
										strCheckValue = pstRes->strValue;
									}
									else {												
										strUpdateName = "Value";
										updateType = upString;
										strCheckValue = pstRes->strValue;
										strUpdateValue = strValue;
										nAuditID = aeiLabResultValue;
										strAuditOldName = strCheckValue;
										strAuditNewName = strUpdateValue;
									}
								}
							}
						break;
						
						// (c.haag 2009-05-06 14:51) - PLID 33789
						case lrfUnits:
							strUpdateName = "Units";
							updateType = upString;
							strCheckValue = pstRes->strUnits;
							strUpdateValue = strUnits;
							nAuditID = aeiLabResultUnits;
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						case lrfReference:
							strUpdateName = "Reference";
							updateType = upString;
							strCheckValue = pstRes->strReference;
							strUpdateValue = strReference;
							nAuditID = aeiLabResultReference;
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						// (z.manning 2009-04-30 16:49) - PLID 28560
						case lrfComments:
							strUpdateName = "Comments";
							updateType = upString;
							strCheckValue = pstRes->strComments;
							strUpdateValue = strResultComments;
							nAuditID = aeiLabResultComments;
							strAuditOldName = strCheckValue;
							strAuditNewName = strUpdateValue;
						break;

						case lrfStatus:
							//TES 12/1/2008 - PLID 32191 - Track the Status field
							strUpdateName = "StatusID";
							updateType = upNumber;
							if (pstRes->nStatusID == -1) {
								strCheckValue = "NULL";
								strAuditOldName = "";
							}
							else {
								strCheckValue = AsString(pstRes->nStatusID);
								strAuditOldName = pstRes->strStatus;
							}

							if (nStatusID == -1) {
								strUpdateValue = "NULL";
								strAuditNewName = "";
							}
							else {
								strUpdateValue = AsString(nStatusID);
								strAuditNewName = strStatus;
							}
							nAuditID = aeiLabResultStatusID;
						break;

						// (c.haag 2009-05-07 15:01) - PLID 28561 - Acknowledged fields
						case lrfAcknowledgedBy:
							strUpdateName = "AcknowledgedUserID";
							updateType = upNumber;
							if (pstRes->nAcknowledgedUserID == -1) {
								strCheckValue = "NULL";
								strAuditOldName = "";
							}
							else {
								strCheckValue = AsString(pstRes->nAcknowledgedUserID);
								strAuditOldName = GetExistingUserName(pstRes->nAcknowledgedUserID);
							}

							if (nAcknowledgedUserID == -1) {
								strUpdateValue = "NULL";
								strAuditNewName = "";
							}
							else {
								strUpdateValue = AsString(nAcknowledgedUserID);
								strAuditNewName = GetExistingUserName(nAcknowledgedUserID);
							}
							nAuditID = aeiLabResultAcknowledgedUserID;
							break;

						// (c.haag 2009-05-07 15:01) - PLID 28561 - Acknowledged fields
						case lrfAcknowledgedOn:
							strUpdateName = "AcknowledgedDate";
							updateType = upDate;
							if (pstRes->dtAcknowledgedDate.GetStatus() != COleDateTime::valid) {
								strCheckValue = "NULL";
								updateType = upDate;
								strAuditOldName = "";
							}
							else {
								strCheckValue = FormatDateTimeForSql(pstRes->dtAcknowledgedDate);
								updateType = upString;
								if (pstRes->dtAcknowledgedDate.GetStatus() == COleDateTime::valid) {
									strAuditOldName = FormatDateTimeForInterface(pstRes->dtAcknowledgedDate);
								}
								else {
									ASSERT(FALSE);
									strAuditOldName = "";
								}
							}

							if (dtAcknowledgedDate.GetStatus() != COleDateTime::valid) {
								strUpdateValue = "NULL";
								updateType = upDate;
								strAuditNewName = "";
							}
							else {
								strUpdateValue = FormatDateTimeForSql(dtAcknowledgedDate);
								updateType = upString;
								if (dtAcknowledgedDate.GetStatus() == COleDateTime::valid) {
									strAuditNewName = FormatDateTimeForInterface(dtAcknowledgedDate);
								}
								else {
									ASSERT(FALSE);
									strAuditNewName = "";
								}
							}
							
							nAuditID = aeiLabResultAcknowledgedDate;	
							break;
					}

					if (!strUpdateName.IsEmpty()) {
						// (j.dinatale 2013-03-07 17:02) - PLID 34339 - in one case we want to force an update
						if (bForceUpdate || strCheckValue != strUpdateValue) {
							CString strTemp;
							// (j.gruber 2010-02-24 13:56) - PLID 37510 							
							if (strUpdateName == "Value" || strUpdateName == "Name") {
								bCheckLab = TRUE;
							}
							if (updateType == upNumber) {
								if (strUpdateName == "FlagID") {
									//prompt for a todo creation no matter what changed
									//if (strCheckValue == "NULL" && strUpdateValue != "NULL") {
										bSpawnToDo = TRUE;
										CString strTempDesc;
										strTempDesc.Format("Result:%s had flag changed from %s to %s", 
											strName,
											strAuditOldName.IsEmpty() ? "<No Flag>" : strAuditOldName, 
											strAuditNewName.IsEmpty() ? "<No Flag>" : strAuditNewName);
										//TES 8/6/2013 - PLID 51147 - Pass in the todo priority we've calculated
										m_pLabEntryDlg->AddToDoDescription(strTempDesc, (TodoPriority)nTodoPriority, !strAuditNewName.IsEmpty());								
									//}
								}
							
								strTemp.Format(" %s = %s, ", strUpdateName, strUpdateValue);
							}
							else if (updateType == upString) {
								strTemp.Format(" %s = '%s', ", strUpdateName, _Q(strUpdateValue));
							}
							else if (updateType == upDate) {
								strTemp.Format(" %s = %s, ", strUpdateName, strUpdateValue);
							}
							else {
								ASSERT(FALSE);
							}
								
							strUpdate += strTemp;
						
							if (nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							if (strUpdateValue == "NULL") {
								strUpdateValue = "";
							}
							if (strCheckValue == "NULL") {
								strCheckValue = "";
							}
							if (nAuditID != -1) {
								AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, nAuditID, nResultID, strAuditOldName, strAuditNewName, aepMedium, aetChanged);
							}
						}
					}
					pRow = pRow->GetNextRow();
				}

				// (c.haag 2010-11-30 17:34) - PLID 37372 - Result signature fields
				long nSavedSignedByUserID = VarLong(pParentRow->GetValue(lrtcSavedSignedBy),-1);
				COleDateTime dtSavedSigned = VarDateTime(pParentRow->GetValue(lrtcSavedSignedDate), dtInvalid);
				BOOL bSaveInkData = FALSE;
				if (nSavedSignedByUserID != nSignedByUserID)
				{
					// (b.spivey, March 19, 2013) - PLID 55943 - Add this lab ID to the list to check for. 
					aryLabIDs.Add(nLabID); 
					bSaveInkData = TRUE;
					strUpdate += FormatString("ResultSignedBy = %s, ", (-1 == nSignedByUserID) ? "NULL" : AsString(nSignedByUserID));
					AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiPatientLabResultSigned, nResultID, "", "<Signed>", aepMedium, aetChanged);
				}
                // (j.kuziel 2011-10-12 11:24) - PLID 43789 - COleDateTime objects will not compare when both are invalid, an 'undefined' state.
                if(dtSavedSigned.GetStatus() != COleDateTime::valid && dtSigned.GetStatus() != COleDateTime::valid) {
                    // Both dates are not in valid states. Do not update.
                } else {
                    if (dtSavedSigned != dtSigned) 
				    {
    					bSaveInkData = TRUE;
					    strUpdate += FormatString("ResultSignedDate = %s, ", (COleDateTime::invalid == dtSigned.GetStatus()) ? "NULL" : ("'" + FormatDateTimeForSql(dtSigned) + "'"));
					    // This cannot be edited, and therefore does not need audited.  There is 1 generic audit for "signed".
				    }
                }
				if (bSaveInkData)
				{
					//ResultSignatureInkData, ResultSignatureImageFile, ResultSignatureTextData
					strUpdate += FormatString("ResultSignatureInkData = %s, ", strSigInkData);
					strUpdate += FormatString("ResultSignatureImageFile = '%s', ", _Q(strSignatureFileName));
					strUpdate += FormatString("ResultSignatureTextData = %s, ", strSigTextData);
					// This cannot be edited, and therefore does not need audited.  There is 1 generic audit for "signed".
				}

				// (c.haag 2010-12-02 16:48) - PLID 41590 - Save completed fields
				long nSavedCompletedByUserID = VarLong(pParentRow->GetValue(lrtcSavedCompletedBy),-1);
				COleDateTime dtSavedCompleted = VarDateTime(pParentRow->GetValue(lrtcSavedCompletedDate), dtInvalid);
				if (nSavedCompletedByUserID != nCompletedByUserID)
				{
					// (c.haag 2010-12-20) - PLID 38633 - We used to audit completing a lab at the lab level. Now when
					// auditing, we need to do it to the results, and include some lab information in the process.
					CString strFormNumberTextID = m_pLabEntryDlg->GetFormNumber();
					IRowSettingsPtr pSpecRow = GetSpecFromCurrentRow(pParentRow);
					CString strSpecimen = VarString(pSpecRow->GetValue(lrtcSpecimen),"");
					CString strResultName = VarString(GetTreeValue(pParentRow, lrfName, lrtcValue), "");
					CString strOld = strFormNumberTextID + " - " + strSpecimen + " - " + strResultName;
					strUpdate += FormatString("ResultCompletedBy = %s, ", (-1 == nCompletedByUserID) ? "NULL" : AsString(nCompletedByUserID));
					AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiPatientLabResultMarkedComplete, nResultID, strOld, "<Completed>", aepMedium, aetChanged);
				}
                // (j.kuziel 2011-10-12 11:08) - PLID 43789 - COleDateTime objects will not compare when both are invalid, an 'undefined' state.
                if (dtSavedCompleted.GetStatus() != COleDateTime::valid && dtCompleted.GetStatus() != COleDateTime::valid) {
                    // Both dates are not in valid states. Do not update.
                } else {
                    if(dtSavedCompleted != dtCompleted)	
                    {
					    strUpdate += FormatString("ResultCompletedDate = %s, ", (COleDateTime::invalid == dtCompleted.GetStatus()) ? "NULL" : ("'" + FormatDateTimeForSql(dtCompleted) + "'"));
					    // This cannot be edited, and therefore does not need audited.  There is 1 generic audit for "completed".
				    }
                }

				if (strUpdate != "UPDATE LabResultsT SET ") {
					//remove the last comma
					strUpdate = strUpdate.Left(strUpdate.GetLength() - 2);
					//add it to the batch
					strUpdate += " WHERE ResultID = " + AsString(nResultID) + ";\r\n";
					AddStatementToSqlBatch(strSql, "%s", strUpdate);

					// (j.gruber 2010-02-24 10:05) - PLID 37510 - add to our array
					if (bCheckLab) {
						dwaryResultIDs.Add(nResultID);
					}
					
				}



			}

			pParentRow = pParentRow->GetNextRow();
		}
		pLabRow = pLabRow->GetNextRow();

	}

	//now declare after the inserts, if there were any
	//AddDeclarationToSqlBatch(strSql, "DECLARE @nEndResultID  INT;  \r\n");
	//AddDeclarationToSqlBatch(strSql, "SET @nEndResultID = (SELECT COALESCE(Max(ResultID), 0)+1 FROM LabResultsT) \r\n");
	AddStatementToSqlBatch(strSql, "SET NOCOUNT OFF");
	//AddStatementToSqlBatch(strSql, " SELECT @nStartResultID as StartResultID, @nEndResultID as EndResultID; \r\n");
	AddStatementToSqlBatch(strSql, "SELECT ID, ArrayIndex FROM @TempLabInsertsT");

	//now execute
	_RecordsetPtr rs = CreateRecordsetStd("BEGIN TRAN \r\n " + strSql + " COMMIT TRAN \r\n");

	// (b.spivey, April 22, 2013) - PLID 55943 - only bother if we know we have steps to complete. 
	if (aryLabIDs.GetCount() > 0) {
		// (b.spivey, April 24, 2013) - PLID 55943 - There is a case where if we just created this, the lab may not have sync'd yet
		//		 so here, we pre-sync. 
		for (int i = 0; i < aryLabIDs.GetCount(); i++) {
			long nLabID = aryLabIDs.GetAt(i); 
			SyncTodoWithLab(nLabID, GetPatientID()); 
		}
		// (b.spivey, March 18, 2013) - PLID 55943 - Update any lab steps that should be 
		//		completed by signing all results.
		_RecordsetPtr prsStepsCompleteBySigning = CreateParamRecordset(
			"SELECT LST.StepID, LST.LabID "
			"FROM LabStepsT LST "
			"INNER JOIN LabProcedureStepsT LPST ON LST.LabProcedureStepID = LPST.StepID "
			"LEFT JOIN ( "
			"	SELECT MIN(CASE WHEN LabResultsT.ResultSignedDate IS NOT NULL THEN 1 ELSE 0 END) AS AllSigned, LabID "
			"	FROM LabResultsT "
			"	WHERE Deleted = 0 "
			"	GROUP BY LabID "
			") LabResultsT ON LST.LabID = LabResultsT.LabID "
			"WHERE LST.LabID IN ({INTARRAY}) AND LPST.CompletedBySigning = {BIT} "
			"	AND LabresultsT.AllSigned = 1 " 
			"ORDER BY LST.LabID, LST.StepOrder ", aryLabIDs, TRUE);

		while (!prsStepsCompleteBySigning->eof) {
			long nLabIDFromQuery = AdoFldLong(prsStepsCompleteBySigning->Fields, "LabID", -1); 
			long nStepID = AdoFldLong(prsStepsCompleteBySigning->Fields, "StepID", -1); 
			GlobalModifyStepCompletion(nLabIDFromQuery, nStepID, true, true); 
			prsStepsCompleteBySigning->MoveNext(); 
		}
	}

	{
		// (b.spivey, April 05, 2013) - PLID 44387 - This code block handles marking open lab steps as completed and syncing todos. 
		CArray<long, long> aryAllLabs;

		//Get all LabIDs for this req.
		GetAllLabIDsAry(aryAllLabs); 

		bool bCompleteAllLabSteps = false;  
		//I wanted the message to be a litttle clearer-- we only allow them to mark all open lab steps complete or not, not selectively per lab.
		CString strCompleteMultipleLabs = 
			(aryAllLabs.GetCount() > 1 ? "Do you want to mark all open lab steps for any newly completed specimen(s) as completed?" : "Do you want to mark all open lab steps for this lab as completed?");

		for(int i = 0; i < aryAllLabs.GetCount(); i++) {

			long nLabID = aryAllLabs.GetAt(i); 
			// (b.spivey, April 09, 2013) - PLID 44387 - We're checking to see if we should even bother this lab-- if it 
			//		started as completed then we leave it be. 
			bool bLoadedAsIncompleted = !!VarBool(GetLabRowByID(nLabID)->GetValue(lrtcLoadedAsIncomplete), FALSE); 

			// (b.spivey, April 05, 2013) - PLID 44387 - We have to do this check per lab because a lab may not be completed or have
			//	 results. 
			//Check that the lab started as incompleted. 
			//Checking for a lab having results, 
			//and that those results are completed, and that that lab has lab steps remaining to complete,
			//and either (a) we've decided to complete all lab steps for all valid labs or 
			//	(b) They explicitly say complete all lab steps for all valid labs
			//Be careful if you change this order. The check short circuits to avoid running the queries or throwing a message box 
			//	 unless we know we have to. And AllResultsAreCompleted will return true with no results. 
			if(bLoadedAsIncompleted 
				&& DoesLabHaveResults(nLabID) 
				&& AllResultsAreCompleted(nLabID)  
				&& ReturnsRecordsParam("SELECT StepID FROM LabStepsT WHERE StepCompletedDate IS NULL AND StepCompletedBy IS NULL AND LabID = {INT} ", nLabID) 
				&& (bCompleteAllLabSteps || MessageBox(strCompleteMultipleLabs, "NexTech Practice", MB_YESNO|MB_ICONWARNING) == IDYES)) {

				bCompleteAllLabSteps = true; 
				
				// Complete lab steps.
				// (b.spivey, April 23, 2013) - PLID 44387 - Complete steps. 
				_RecordsetPtr prsUncompleteSteps = CreateParamRecordset(
					"BEGIN TRAN "
					"SET NOCOUNT ON " 
					"SELECT StepID "
					"FROM LabStepsT "
					"WHERE StepCompletedDate IS NULL AND StepCompletedBy IS NULL "
					"	AND LabID = {INT} "
					"	"
					"UPDATE LabStepsT "
					"SET StepCompletedDate = GETDATE(), StepCompletedBy = {INT} "
					"WHERE StepCompletedDate IS NULL AND StepCompletedBy IS NULL "
					"	AND LabID = {INT} "
					"SET NOCOUNT OFF "
					"COMMIT TRAN "
					, nLabID
					, GetCurrentUserID()
					, nLabID); 

				// (b.spivey, April 23, 2013) - PLID 44387 - Audit.
				//We updated something!
				if(!prsUncompleteSteps->eof) {
					
					long nAuditID = BeginNewAuditEvent();
					CAuditTransaction auditTran;
					long nPatientID = GetPatientID(); 
					CString strPatientName = GetExistingPatientName(nPatientID); 
					//audit. 
					while(!prsUncompleteSteps->eof) {
						long nRecordID = AdoFldLong(prsUncompleteSteps->Fields, "StepID", -1); 
						CString strOld = GenerateStepCompletionAuditOld(nRecordID);
						CString strNew = "<Completed>"; 
						AuditEvent(nPatientID, strPatientName, auditTran, aeiPatientLabStepMarkedComplete, nRecordID, strOld, strNew, aepMedium, aetChanged);
						prsUncompleteSteps->MoveNext(); 
					}
					//Commit transaction.
					auditTran.Commit(); 

				}
			}

			// (b.spivey, April 22, 2013) - PLID 55943 - Need to sync todos for signed results. 
			// (f.gelderloos 2013-07-10 10:39) - PLID 57448 - This was causing too many TODO items to be created when a lab dlg is closed.
			//  Steps cannot be modified anyway, so this was pointless i think?
			//Sync todos for every lab. 
			//SyncTodoWithLab(nLabID, m_nPatientID); 
		}
	} //bye

	//TES 12/7/2009 - PLID 36191 - Track how many results we've added.
	int nNewResultCount = 0;
	while (! rs->eof) {
		
		//long nStartResultID = AdoFldLong(rs, "StartResultID");
		//long nEndResultID = AdoFldLong(rs, "EndResultID");
		//int nCount = 0;

		//for (int i = nStartResultID; i < nEndResultID; i++ ) {

		if (nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		stResults *pRes = ((stResults*)aryNewRows.GetAt(AdoFldLong(rs, "ArrayIndex")));

		long nResultID = AdoFldLong(rs, "ID");

		// (j.gruber 2010-02-24 10:05) - PLID 37510 - add to our array
		dwaryResultIDs.Add(nResultID);

		//TES 12/7/2009 - PLID 36191 - Copy the new result ID to the associated row, as well as its children.
		IRowSettingsPtr pResultRow = pRes->pRow;
		pResultRow->PutValue(lrtcResultID, nResultID);
		IRowSettingsPtr pChildRow = pResultRow->GetFirstChildRow();
		while(pChildRow) {
			pChildRow->PutValue(lrtcResultID, nResultID);
			pChildRow = pChildRow->GetNextRow();
		}
		//TES 12/7/2009 - PLID 36191 - This is now an existing result, so put it in our map.
		m_mapResults.SetAt(nResultID, pRes);
		nNewResultCount++;

		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultCreated, nResultID, "", "<Created - Entries to follow>", aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultName, nResultID, "", pRes->strResultName, aepHigh, aetCreated);
		//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultReceivedDate, nResultID, "", FormatDateTimeForInterface(pRes->dtReceivedDate), aepHigh, aetCreated);
		// (a.walling 2010-01-18 10:53) - PLID 36936
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultLOINC, nResultID, "", pRes->strLOINC, aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultSlideNumber, nResultID, "", pRes->strSlideNum, aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultDiagnosis, nResultID, "", pRes->strDiagnosis, aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultMicroDesc, nResultID, "", pRes->strMicroDesc, aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultFlagID, nResultID, "", pRes->strFlag, aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultValue, nResultID, "", pRes->strValue, aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultUnits, nResultID, "", pRes->strUnits, aepHigh, aetCreated); // (c.haag 2009-05-06 15:23) - PLID 33789
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultReference, nResultID, "", pRes->strReference, aepHigh, aetCreated);
		//TES 12/1/2008 - PLID 32191 - Audit the Status field
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultStatusID, nResultID, "", pRes->strStatus, aepHigh, aetCreated);				
		// (z.manning 2009-04-30 16:52) - PLID 28560 - Added lab result comments
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultComments, nResultID, "", pRes->strComments, aepHigh, aetCreated);
		// (c.haag 2009-05-07 15:08) - PLID 28561 - Added lab result acknowledged fields
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultAcknowledgedUserID, nResultID, "", (-1 == pRes->nAcknowledgedUserID) ? "" : GetExistingUserName(pRes->nAcknowledgedUserID), aepHigh, aetCreated);
		AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiLabResultAcknowledgedDate, nResultID, "", (COleDateTime::invalid == pRes->dtAcknowledgedDate.GetStatus()) ? "" : FormatDateTimeForInterface(pRes->dtAcknowledgedDate), aepHigh, aetCreated);
		// (c.haag 2011-01-25) - PLID 38633 - Audit whether this new result is signed or completed.
		long nSignedByUserID = VarLong(pResultRow->GetValue(lrtcSignedBy),-1);
		if (nSignedByUserID > 0) {
			AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiPatientLabResultSigned, nResultID, "", "<Signed>", aepMedium, aetCreated);
		}
		long nCompletedByUserID = VarLong(pResultRow->GetValue(lrtcCompletedBy),-1);
		if (nCompletedByUserID > 0) {
			AuditEvent(m_nPatientID, strPersonName, nAuditTransactionID, aeiPatientLabResultMarkedComplete, nResultID, "", "<Completed>", aepMedium, aetCreated);
		}
			
		rs->MoveNext();
	}

	//TES 5/2/2011 - PLID 43428 - We've saved now, so update our saved status
	m_strSavedOrderStatus = strOrderStatus;

	//now we have to delete the res pointers if they exist
	//TES 12/7/2009 - PLID 36191 - We no longer need to delete these, because we moved all of them to the existing results map.  However,
	// we do want to make sure we had the expected number of new results.
	if(nNewResultCount != aryNewRows.GetSize()) {
		AfxThrowNxException("Mismatched result count when saving new Lab Results (expected %i new results, found %i)", aryNewRows.GetSize(), nNewResultCount);
	}

	// (j.gruber 2010-02-24 10:05) - PLID 37510 - call our function to check the new results against decision rules
	if (dwaryResultIDs.GetSize() > 0) {
		// (c.haag 2010-09-21 12:15) - PLID 40612 - We now pass in the patient ID
		//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
		UpdateDecisionRules(GetRemoteData(), m_nPatientID, arNewCDSInterventions);
	}
		
	// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a labs table checker because lab result data has changed
	// (r.gonet 08/25/2014) - PLID 63221 - Send a table checker for all labs on the patient
	CClient::RefreshLabsTable(m_nPatientID, -1);
}

// (b.spivey, April 05, 2013) - PLID 44387 - Throw in an array, get all lab IDs for this lab. 
void CLabResultsTabDlg::GetAllLabIDsAry(OUT CArray<long, long>& aryLabIDs) 
{
	IRowSettingsPtr pRow = m_pResultsTree->GetFirstRow();
	while (pRow) {
		aryLabIDs.Add(VarLong(pRow->GetValue(lrtcForeignKeyID))); 
		pRow = pRow->GetNextRow();
	}
}

// (b.spivey, April 05, 2013) - PLID 44387 - Check if this lab has any results. 
bool CLabResultsTabDlg::DoesLabHaveResults(long nLabID) 
{
	IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
	if(pLabRow) {
		if (pLabRow->GetFirstChildRow()) {
			return true; 
		}
	}

	return false; 
}

// (c.haag 2010-01-27) - PLID 41618 - We now return a CAttachedLabFile object that has values for
// the mail ID and attachment, and take in an attachment date
CAttachedLabFile CLabResultsTabDlg::AttachFileToLab(CString strSourcePath, const COleDateTime& dtAttached) 
{
	CString strFileName = GetFileName(strSourcePath);
	CString strDstPath = GetPatientDocumentPath(m_nPatientID) ^ strFileName;
	long nMailID = -1;

	// (m.hancock 2006-06-30 11:10) - PLID 21280 - Adding a preference for assigning default attachment category
	long nCategoryID = (long)GetRemotePropertyInt("LabAttachmentsDefaultCategory",-1,0,"<None>",true);
	//TES 8/2/2011 - PLID 44814 - Check whether they're allowed to use the default category
	if(nCategoryID != -1 && !CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, nCategoryID, TRUE)) {
		nCategoryID = -1;
	}

	// (a.walling 2007-08-21 07:54) - PLID 26748 - Need to support Office 2007 files
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	CString strType;
	if ( (strDstPath.Right(4).CompareNoCase(".doc") == 0) || (strDstPath.Right(5).CompareNoCase(".docx") == 0) || (strDstPath.Right(5).CompareNoCase(".docm") == 0))
		strType = SELECTION_WORDDOC;
	else strType = SELECTION_FILE;

	// (j.jones 2012-10-08 14:03) - PLID 44263 - If the file is being attached from the
	// documents folder, find the existing MailID and return it, so we do not add a
	// redundant MailSent entry. In the off chance that the file is in the documents
	// folder but not attached, then we would need to properly attach it.
	if(strSourcePath == strDstPath) {

		_RecordsetPtr rs = CreateParamRecordset("SELECT MailID FROM MailSent WHERE PersonID = {INT} AND PathName = {STRING}", m_nPatientID, strFileName);
		if(!rs->eof) {
			nMailID = VarLong(rs->Fields->Item["MailID"]->Value);
			//do not update the date of the existing entry in MailSent
		}
		else {
			nMailID = ::AttachFileToHistory(strDstPath, m_nPatientID, GetSafeHwnd(), nCategoryID, strType, NULL, -1, -1);
			//Update the attachment date to the time the user actually added it to the lab. We
			//want to maintain a consistent ordering of attached document dates.
			ExecuteParamSql("UPDATE MailSent SET [Date] = {OLEDATETIME} WHERE MailID = {INT}", dtAttached, nMailID);
		}
		rs->Close();

		//return now, because do not need to actually copy this file
		CAttachedLabFile af;
		af.nMailID = nMailID;
		af.strFileName = GetPatientDocumentPath(m_nPatientID) ^ GetFileName(strSourcePath);
		af.dtAttached = dtAttached;
		return af;
	}

	if (CopyFile(strSourcePath, strDstPath, TRUE)) {
		// (m.hancock 2006-06-27 16:31) - PLID 21071 - Attach the file to history and this lab step, returning the MailID
		nMailID = ::AttachFileToHistory(strDstPath, m_nPatientID, GetSafeHwnd(), nCategoryID, strType, NULL, -1, -1);
	}
	else {
		//If the file already exists here, prompt for rename/cancel
		DWORD dwLastError = GetLastError();
		if(dwLastError == ERROR_FILE_EXISTS) {
			//TES 12/17/2009 - PLID 36634 - If this file already exists, maybe that's because we're attaching something that's already in
			// the default folder.
			if(strDstPath.CompareNoCase(strSourcePath) == 0) {
				//TES 12/17/2009 - PLID 36634 - Yup, that's the case, so no need to prompt the user to rename, just attach it in place.
				nMailID = ::AttachFileToHistory(strDstPath, m_nPatientID, GetSafeHwnd(), nCategoryID, strType, NULL, -1, -1);
				dwLastError = GetLastError();
			}
			else {
				strDstPath = GetPatientDocumentPath(m_nPatientID);
				CRenameFileDlg dlgRename(strSourcePath, strDstPath, this);
				try {
					if(dlgRename.DoModal() == IDOK) {
						strDstPath = dlgRename.m_strDstFullPath;
						if(CopyFile(strSourcePath, strDstPath, TRUE)) {
							// (m.hancock 2006-06-27 16:31) - PLID 21071 - Attach the file to history and this lab step, returning the MailID
							nMailID = ::AttachFileToHistory(strDstPath, m_nPatientID, GetSafeHwnd(), nCategoryID, strType, NULL, -1, -1);
						}
					}
			
					dwLastError = GetLastError();
				}NxCatchAll("Error in CPatientLabsDlg::OnImportAndAttachExistingFile:RenamingFile");
			}
		}
		//there's an error other than the file already existing
		if(dwLastError != ERROR_SUCCESS) {
			CString strError;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			if(IDNO == MsgBox(MB_YESNO, "The file '%s' could not be imported into the patient's documents folder. Windows returned the following error message:\r\n\r\n"
				"%s\r\n"
				"Would you like to continue?", GetFileName(strSourcePath), strError))
			{
				// (c.haag 2010-01-27) - PLID 41618 - Return a CAttachedLabFile object instead. Defaults to a -1 MailID.
				CAttachedLabFile af;
				return af;
			}
		}
	}

	if (nMailID == 0) {
		// (c.haag 2010-01-27) - PLID 41618 - Return a CAttachedLabFile object instead. Defaults to a -1 MailID.
		CAttachedLabFile af;
		return af;
	}
	else {
		// (c.haag 2010-01-27) - PLID 41618 - Update the attachment date to the time the user actually added it to the lab. We
		// want to maintain a consistent ordering of attached document dates.
		ExecuteParamSql("UPDATE MailSent SET [Date] = {OLEDATETIME} WHERE MailID = {INT}", dtAttached, nMailID);
		CAttachedLabFile af;
		af.nMailID = nMailID;
		af.strFileName = GetPatientDocumentPath(m_nPatientID) ^ GetFileName(strSourcePath);
		af.dtAttached = dtAttached;
		return af;
	}
}
BEGIN_EVENTSINK_MAP(CLabResultsTabDlg, CNxDialog)
	ON_EVENT(CLabResultsTabDlg, IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST, 16, CLabResultsTabDlg::OnSelChosenClinicalDiagnosisOptionsList, VTS_DISPATCH)
	ON_EVENT(CLabResultsTabDlg, IDC_LAB_FLAG, 20, CLabResultsTabDlg::OnTrySetSelFinishedLabFlag, VTS_I4 VTS_I4)
	ON_EVENT(CLabResultsTabDlg, IDC_LAB_FLAG, 16, CLabResultsTabDlg::OnSelChosenLabFlag, VTS_DISPATCH)
	ON_EVENT(CLabResultsTabDlg, IDC_LAB_FLAG, 18, CLabResultsTabDlg::OnRequeryFinishedLabFlag, VTS_I2)
	ON_EVENT(CLabResultsTabDlg, IDC_LAB_STATUS, 18, CLabResultsTabDlg::OnRequeryFinishedLabStatus, VTS_I2)
	ON_EVENT(CLabResultsTabDlg, IDC_LAB_STATUS, 16, CLabResultsTabDlg::OnSelChosenLabStatus, VTS_DISPATCH)
	ON_EVENT(CLabResultsTabDlg, IDC_RESULTS_TREE, 2, CLabResultsTabDlg::OnSelChangedResultsTree, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLabResultsTabDlg, IDC_RESULTS_TREE, 1, CLabResultsTabDlg::OnSelChangingResultsTree, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabResultsTabDlg, IDC_RESULTS_TREE, 6, CLabResultsTabDlg::OnRButtonDownResultsTree, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLabResultsTabDlg, IDC_PDF_PREVIEW, 270, CLabResultsTabDlg::OnFileDownloadPdfPreview, VTS_BOOL VTS_PBOOL)
	ON_EVENT(CLabResultsTabDlg, IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST, 18, CLabResultsTabDlg::RequeryFinishedClinicalDiagnosisOptionsList, VTS_I2)
END_EVENTSINK_MAP()

void CLabResultsTabDlg::OnAddDiagnosis()
{
	// (c.haag 2007-03-16 09:03) - PLID 21622 - Choose diagnoses from a multi-select list
	// (j.jones 2007-07-19 16:20) - PLID 26751 - the diagnosis description is now free-form text,
	// so we now just append to the end
	try {
		// Have the user choose diagnosis codes to add. The "Convert(nvarchar(1000)" statement was
		// carried over from the legacy code; the description field is an ntext type
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "DiagCodes");
		// (z.manning 2008-10-28 11:43) - PLID 24630 - We can now link diag codes to lab diagnoses,
		// so if we have a link, include the diag code in the text in the multi select dialog.
		CString strFrom = "LabDiagnosisT LEFT JOIN DiagCodes ON LabDiagnosisT.DiagID = DiagCodes.ID";
		CString strValueField = "CASE WHEN DiagCodes.CodeNumber IS NOT NULL THEN DiagCodes.CodeNumber + ' - ' ELSE '' END + convert(nvarchar(1000), LabDiagnosisT.Description)";
		if (IDOK != dlg.Open(strFrom, "", "LabDiagnosisT.ID", strValueField, "Please choose one or more diagnoses", 1))
			return;

		// Now get a list of all the ID's which the user selected
		std::set<long> arIDs;
		CString strSelectIDs = dlg.GetMultiSelectIDString(",");
		dlg.FillArrayWithIDs(arIDs);
		
		Nx::SafeArray<long> sa(arIDs.size(), begin(arIDs), end(arIDs));

		
		CString strCurDescription = "";

		GetDlgItemText(IDC_LAB_DIAG_DESCRIPTION, strCurDescription);

		// (b.spivey February 24, 2016) - PLID 68371 - one api function call
		CString strMicroscopic;
		GetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, strMicroscopic);

		bool bLoadMicroscopicDescriptions = (GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->IsWindowVisible()) ? true:false;

		// (b.spivey February 22, 2016) - PLID 68371 - get the diagnosis description and result comments, and other microscopic stuff in the near future. 
		NexTech_Accessor::_AddDiagnosisToInterfaceResultPtr diagPtr = GetAPI()->AddDiagnosisToLabInterface(GetAPISubkey(), GetAPILoginToken(), sa, _bstr_t(strCurDescription), _bstr_t(strMicroscopic), bLoadMicroscopicDescriptions);

		strCurDescription = (const char*)diagPtr->DiagnosisDescription;
		CString strAutoFillComment = (const char*)diagPtr->ResultComment;

		if (!strAutoFillComment.IsEmpty()) {
			CString str; 
			GetDlgItemText(IDC_RESULT_COMMENTS, str);

			if (!str.IsEmpty() && 
				AfxMessageBox("There are result comments linked to the diagnoses you selected and there are already result comments. "
					"Would you like to replace the existing result comments with the linked comments?", MB_ICONQUESTION | MB_YESNO) == IDNO) {
				//this is the case where we don't do anything. 
			}
			else {
				SetDlgItemText(IDC_RESULT_COMMENTS, strAutoFillComment); 
			}
		}

		//now update our description
		SetDlgItemText(IDC_LAB_DIAG_DESCRIPTION, strCurDescription);

		if (GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->IsWindowVisible()) {

			// get values from accessor pointer
			CString strMicro = (const char*)diagPtr->PromptMicroscopicDescription;
			NexTech_Accessor::_NullableIntPtr pNullableMicroID = diagPtr->PromptMicroscopicDescriptionID;
			bool bPromptForCommonMicroscopic = strMicro.IsEmpty() ? false : true;
			Nx::SafeArray<long> saClinicalDiagIDs(diagPtr->ClinicalDiagnosisIDs);

			// make clean variables from accessor output
			long nMicroID = VarLong(pNullableMicroID->GetValue(), -1);
			std::set<long> arClinicalDiagIDs(saClinicalDiagIDs.begin(), saClinicalDiagIDs.end());
			bool bHasDiagnosisLinkedToClinicalDiagnosis = !arClinicalDiagIDs.empty();
			bool bHasExactlyOneLinkedClinicalDiagnosis = arClinicalDiagIDs.size() == 1;

			if (bPromptForCommonMicroscopic) {
				if (IDYES == MsgBox(MB_YESNO, "Recent lab results with the Diagnosis '%s' have frequently had the "
					"following Microscopic Description:\r\n\r\n"
					"%s\r\n\r\n"
					"Would you like to use that value for this result's Microscopic Description field?",
					strCurDescription, strMicro)) {
					// (r.goldschmidt 2016-02-19 10:32) - PLID 68266 - When choosing a final diagnosis, if the preference to prompt for microscopic description is on and the user chooses yes on the prompt, append the microscopic description from the prompt to all lab diagnosis that they chose
					if (pNullableMicroID->IsNull() == VARIANT_FALSE) { // if it isn't null
						AddLabClinicalDiagnosisLink(arIDs, nMicroID);
					}
					SetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, strMicro);
				}
			}

			// (r.goldschmidt 2016-02-18 18:08) - PLID 68267 - Filter the microscopic description list for any linked descriptions that are linked with the final lab diagnoses that the user chose
			if (bHasDiagnosisLinkedToClinicalDiagnosis) {
				// (r.goldschmidt 2016-02-22 16:59) - PLID 68240 - If the final diagnosis code has only 1 linked microscopic description, automatically use it without prompting.
				if (bHasExactlyOneLinkedClinicalDiagnosis) {
					//lookup the description from the datalist
					std::set<long>::iterator itValue = arClinicalDiagIDs.begin();
					long nValuetoFind = *itValue;
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pClinicalDiagOptionsList->FindByColumn(cdolcID, nValuetoFind, NULL, FALSE);
					if (strMicroscopic.IsEmpty()) {
						if (pRow) {
							strMicroscopic = VarString(pRow->GetValue(cdolcDesc));							
						}
					}
					else {
						if (pRow) {
							strMicroscopic += "\r\n" + VarString(pRow->GetValue(cdolcDesc));
						}
					}
					SetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, strMicroscopic);
					RemoveFilterClinicalDiagnosisOptionsList();
				}
				else {
					FilterClinicalDiagnosisOptionsList(arClinicalDiagIDs);
				}
			}
			// if none are linked, make sure to repopulate the list if previously filtered
			else {
				RemoveFilterClinicalDiagnosisOptionsList();
			}
		}

		//(e.lally 2009-09-22) PLID 35609 - We have unsaved result changes
		m_bResultIsSaved = FALSE;

	} NxCatchAll("Error in CLabResultsTabDlg::OnAddDiagnosis()");
}

// (s.dhole 2010-10-21 15:57) - PLID 36938 get new file name from database
CString  CLabResultsTabDlg::GetDocumentName(long  nMailID)
{
	_RecordsetPtr rsMailSent = CreateParamRecordset("SELECT PathName FROM mailsent WHERE mailid = {INT}", nMailID);
	if(!rsMailSent->eof) {
		//TES 2/2/2010 - PLID 34336 - Got it, fill the text.
		return AdoFldString(rsMailSent, "PathName", "");
	}
	else {
		return "";
	}

}

void CLabResultsTabDlg::OnSelChosenClinicalDiagnosisOptionsList(LPDISPATCH lpRow)
{
	// (j.jones 2007-07-19 16:30) - PLID 26751 - the clinical diagnosis description is now free-form text,
	// so we now just append to the end

	try {
		IRowSettingsPtr pSelRow(lpRow);

		if(pSelRow != NULL) {

			// (r.goldschmidt 2016-02-18 18:46) - PLID 68268 - <show all> was clicked, remove filtering
			if (VarLong(pSelRow->GetValue(cdolcID)) == -1) {
				RemoveFilterClinicalDiagnosisOptionsList();
			}
			else {

				CString strCurDescription;
				GetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, strCurDescription);

				CString strNewDesc = VarString(pSelRow->GetValue(cdolcDesc), "");;

				if (!strCurDescription.IsEmpty())
					strCurDescription += "\r\n";

				strCurDescription += strNewDesc;

				//now update our description
				SetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, strCurDescription);

				//(e.lally 2009-09-22) PLID 35609 - We have unsaved result changes
				m_bResultIsSaved = FALSE;
			}
		}

	} NxCatchAll("Error in CLabResultsTab::OnSelChosenClinicalDiagnosisOptionsList()");
}

BOOL CLabResultsTabDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		
		// (j.jones 2007-07-19 16:34) - PLID 26751 - no commands are used currently,
		// as I removed the diagnosis/clinical diag lists

		// (j.gruber 2008-09-18 11:58) - PLID 31332 - new results interface
		switch(HIWORD(wParam))
		{	
			case EN_CHANGE: {
				
				//TES 11/22/2009 - PLID 36191 - We need to determine which LabResultField to look up in the tree..
				int nFieldID = -1;
				
				switch((LOWORD(wParam))) {

					case IDC_RESULT_NAME: 
						nFieldID = lrfName;
					break;

					case IDC_RESULT_LOINC: 
						nFieldID = lrfLOINC; // (a.walling 2010-01-18 10:39) - PLID 36936
					break;
					
					case IDC_SLIDE_NUMBER: 
						nFieldID = lrfSlideNum;
					break;
					case IDC_LAB_DIAG_DESCRIPTION: 
						nFieldID = lrfDiagnosis;
					break;
					case IDC_CLINICAL_DIAGNOSIS_DESC: 
						nFieldID = lrfMicroscopicDescription;
					break;
					case IDC_RESULT_VALUE:
						nFieldID = lrfValue;
					break;
					// (c.haag 2009-05-06 14:46) - PLID 33789
					case IDC_RESULT_UNITS:
						nFieldID = lrfUnits;
					break;
					case IDC_RESULT_REFERENCE:
						nFieldID = lrfReference;
					break;
					// (z.manning 2009-04-30 16:38) - PLID 28560
					case IDC_RESULT_COMMENTS:
						nFieldID = lrfComments;
					break;

					case IDC_DATE_RECEIVED:
						HandleDateReceivedChange();
						return 0;
					break;
					default:
					break;
				}

				if (nFieldID != -1) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
					if (pRow) {
						CString strCurVal, strListVal;
						GetDlgItemText(LOWORD(wParam), strCurVal);
						strListVal = VarString(GetTreeValue(pRow, (LabResultField)nFieldID, lrtcValue),"");
						if (strCurVal != strListVal) {
							//(e.lally 2009-09-22) PLID 35609 - We have unsaved result changes
							m_bResultIsSaved = FALSE;
						}
						
					}
				}
			}
			break;
		}
							
		

	} NxCatchAll("Error in CLabResultsTabDlg::OnCommand");

	return CNxDialog::OnCommand(wParam, lParam);
}

void CLabResultsTabDlg::OnEditFlag()
{
	try{
		
		CEditLabResultsDlg dlg(this);

		//Temporarily save the current selection for later
		IRowSettingsPtr pRow = m_pFlagCombo->GetCurSel();
		// (j.gruber 2007-08-06 08:58) - PLID 26771 - took out varValue and replaced with nValue
		long nValue = -1;
		//TES 2/2/2010 - PLID 34336 - Remember the name as well.
		CString strValue;
		if(pRow != NULL) {
			nValue = VarLong(pRow->GetValue(lfcFlagID), -1);
			strValue = VarString(pRow->GetValue(lfcName),"");

			// (j.jones 2007-07-20 12:13) - PLID 26749 - set the
			// current anatomy ID, if we have one
			if(nValue > 0)
				dlg.m_nCurResultIDInUse = nValue;
		}
		else {
			if(m_pFlagCombo->IsComboBoxTextInUse) {
				//TES 2/2/2010 - PLID 34336 - It must be an inactive flag, in which case the ID will be in the tree, and the name is
				// the combo box text.
				nValue = VarLong(GetTreeValue(m_pResultsTree->CurSel, lrfFlag, lrtcForeignKeyID),-1);
				strValue = CString((LPCTSTR)m_pFlagCombo->ComboBoxText);
			}
		}

		dlg.DoModal();

		m_pFlagCombo->Requery();
		//Set the selection back to what it was
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nResult = m_pFlagCombo->TrySetSelByColumn_Deprecated(lfcFlagID, nValue);
		if(nResult == sriNoRow) {
			//TES 2/2/2010 - PLID 34336 - This flag may be inactive now, so just set the text.
			if(nValue != -1) {
				m_pFlagCombo->PutComboBoxText(_bstr_t(strValue));
			}
		}
		else if(nResult == sriNoRowYet_WillFireEvent) {
			//TES 2/2/2010 - PLID 34336 - Let the user know what's happening.
			m_pFlagCombo->PutComboBoxText("Loading Flag...");
		}


	}NxCatchAll("Error in CLabResultsTabDlg::OnEditFlag()");
}

void CLabResultsTabDlg::OnEditLabDiagnoses()
{
	try{
		
		// (z.manning 2008-10-28 11:17) - PLID 24630 - We now have a dialog for this
		CLabEditDiagnosisDlg dlg(this);
		dlg.DoModal();

		// (j.jones 2007-07-19 16:37) - PLID 26751 - converted diag list to free-form text,
		// so updating it is no longer needed

	}NxCatchAll("Error in CLabResultsTabDlg::OnEditLabDiagnoses()");
}

void CLabResultsTabDlg::OnEditClinicalDiagnosisList()
{
	try{
		_variant_t varValue;
		IRowSettingsPtr pRow = m_pClinicalDiagOptionsList->GetCurSel();
		if (pRow != NULL)
			varValue = pRow->GetValue(cdolcID);

		//(e.lally 2009-08-10) PLID 31811 - Renamed Dissection to Description
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 15, m_pClinicalDiagOptionsList, "Edit Microscopic Description List").DoModal();

		// (j.jones 2007-07-19 16:37) - PLID 26751 - converted diag list to free-form text,
		// so updating it is no longer needed

	}NxCatchAll("Error in CLabResultsTabDlg::OnEditClinicalDiagnosisList");
}

void CLabResultsTabDlg::OnTrySetSelFinishedLabFlag(long nRowEnum, long nFlags)
{
	try{
		if(nFlags == dlTrySetSelFinishedFailure) {
			//TES 2/2/2010 - PLID 34336 - It may be inactive, look it up in data (pull the ID from the tree, we make sure it's always up to date).
			long nFlagID = VarLong(GetTreeValue(m_pResultsTree->CurSel, lrfFlag, lrtcForeignKeyID),-1);
			_RecordsetPtr rsFlag = CreateParamRecordset("SELECT Name FROM LabResultFlagsT WHERE ID = {INT}", nFlagID);
			if(!rsFlag->eof) {
				//TES 2/2/2010 - PLID 34336 - Got it, fill the text.
				m_pFlagCombo->PutComboBoxText(_bstr_t(AdoFldString(rsFlag, "Name", "")));
			}
			else {
				//TES 2/2/2010 - PLID 34336 - Else if it failed to load anything, check if we need to clear the combo box text
				if(m_pFlagCombo->GetIsComboBoxTextInUse() != FALSE)
					m_pFlagCombo->PutComboBoxText("");
			}
		}
	}NxCatchAll("Error in CLabResultsTabDlg::OnTrySetSelFinishedLabFlag()");
}

BOOL CLabResultsTabDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (a.walling 2010-01-18 11:22) - PLID 36936		
		CRect rcLOINC;
		m_nxlabelLOINC.GetWindowRect(rcLOINC);
		ScreenToClient(&rcLOINC);

		// (j.jones 2013-10-18 09:46) - PLID 58979 - added patient education link
		CRect rcPatEducation;
		m_nxlabelPatientEducation.GetWindowRect(rcPatEducation);
		ScreenToClient(&rcPatEducation);

		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		if ((rcLOINC.PtInRect(pt) && m_nxlabelLOINC.GetType() == dtsHyperlink)
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Only if we are in the pat edu link and we are showing it as a link....
			|| (bShowPatientEducationLink && rcPatEducation.PtInRect(pt) && m_nxlabelPatientEducation.GetType() == dtsHyperlink)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
		if (pRow) {
			long nMailID = VarLong(GetTreeValue(pRow, lrfValue, lrtcForeignKeyID), -1);

			if (nMailID != -1) {

				CRect rc;
				GetDlgItem(IDC_DOC_PATH_LINK)->GetWindowRect(rc);
				ScreenToClient(&rc);

				if (rc.PtInRect(pt)) {
					SetCursor(GetLinkCursor());
					return TRUE;
				}
			}
		}
	}NxCatchAllIgnore();

		
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CLabResultsTabDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2011-02-23) - PLID 41618 - Special handling when in the attachments view
		if(m_nCurrentView == rvPDF) 
		{
			if (NULL != m_pLabResultsAttachmentView) {
				return m_pLabResultsAttachmentView->OnLabelClick(wParam, lParam);
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnLabelClick without a valid attachment view!");
			}			
		}

		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_DOC_PATH_LINK:
			//open the file
			// (c.haag 2011-02-23) - PLID 41618 - We now pass in the current row
			ZoomAttachment(m_strCurrentFileName);
			break;
		case IDC_RESULT_LOINC_LABEL:
			// (a.walling 2010-01-18 15:20) - PLID 36955 - Popup the LOINC code editor/selector
			{
				CString strExistName, strExistCode;
				GetDlgItemText(IDC_RESULT_NAME, strExistName);
				GetDlgItemText(IDC_RESULT_LOINC, strExistCode);
				// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
				CEditComboBox dlg(this, 71, "Select or Edit LOINC Codes");
				if (!strExistCode.IsEmpty()) {
					dlg.m_varInitialSel = _variant_t(strExistCode);
				}
				if (IDOK == dlg.DoModal()) {
					if (dlg.m_nLastSelID != -1) {
						_RecordsetPtr prsLOINC = CreateParamRecordset("SELECT Code, ShortName FROM LabLOINCT WHERE ID = {INT}", dlg.m_nLastSelID);
						if (!prsLOINC->eof) {
							CString strCode = AdoFldString(prsLOINC, "Code");
							CString strName = AdoFldString(prsLOINC, "ShortName");


							if (strCode != strExistCode || strName != strExistName) {

								if (!strExistName.IsEmpty() || !strExistCode.IsEmpty()) {
									CString strExistFormatName;
									if (strExistCode.IsEmpty()) {
										strExistFormatName.Format("%s", strExistName);
									} else {
										strExistFormatName.Format("%s (%s)", strExistName, strExistCode);
									}
									CString strFormatName;
									if (strCode.IsEmpty()) {
										strFormatName.Format("%s", strName);
									} else {
										strFormatName.Format("%s (%s)", strName, strCode);
									}
									if (IDNO == MessageBox(FormatString("Are you sure you want to replace the current result:\r\n\r\n%s\r\n\r\nwith:\r\n\r\n%s?", strExistFormatName, strFormatName), NULL, MB_YESNO | MB_ICONQUESTION)) {
										return 0;
									}
								}

								SetDlgItemText(IDC_RESULT_NAME, strName);
								SetDlgItemText(IDC_RESULT_LOINC, strCode);
								m_bResultIsSaved = FALSE;
							}
						}
					}
				}
			}

			return 0;
		break;

		// (j.jones 2013-10-18 11:39) - PLID 58979 - added patient education link
		case IDC_PT_EDUCATION_LABEL: {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
			if(pRow == NULL || pRow->GetValue(lrtcResultID).vt == VT_NULL) {
				MessageBox("Patient Education cannot be loaded because no lab result has been selected.", "Practice", MB_ICONINFORMATION|MB_OK);
				return 0;
			}

			//force a save
			if(!SaveResult(pRow)) {
				return 0;
			}
			if(!m_pLabEntryDlg->Save()){
				return 0;
			}

			long nLabResultID = VarLong(pRow->GetValue(lrtcResultID), -1);
			if(nLabResultID == -1) {
				//should not be possible, because we forced a save
				ASSERT(FALSE);
				ThrowNxException("No valid lab result ID was found.");
			}

			// (r.gonet 10/30/2013) - PLID 58980 - The patient education hyperlink goes to the Medline Plus website
			LookupMedlinePlusInformationViaSearch(this, mlpLabResultID, nLabResultID);
			break;
			}
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.gruber 2008-10-27 16:39) - PLID 31332
BOOL CLabResultsTabDlg::CheckResultSave(BOOL bSilent) {

	

	if (! m_bResultIsSaved) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
		if (pRow) {	
			if (!bSilent) {
				if (IDYES == MsgBox(MB_YESNO, "The changes you made to this result are not saved yet, do you want to save them now? \nNote: If you save now, you can still undo the changes by clicking the cancel button for the entire lab.")) {
					return SaveResult(pRow);
				}
			}
			else {
				//just save quietly
				return SaveResult(pRow);
			}
		}
	}

	return TRUE;

}
void CLabResultsTabDlg::OnAddResult()
{
	try {

		//since this will change the current selection, warn them if they want to save
		if (!CheckResultSave(FALSE)) {
			return;
		}

		//TES 11/22/2009 - PLID 36191 - Call our utility function.
		AddResult();

	}NxCatchAll("Error in CLabResultsTabDlg::OnAddResult()");
}

void CLabResultsTabDlg::AddResult()
{
	//TES 11/30/2009 - PLID 36452 - If we don't have a lab selected, we can't add a result, as we won't know what to add it to.
	IRowSettingsPtr pCurSel = m_pResultsTree->CurSel;
	if(pCurSel == NULL) {
		return;
	}
	IRowSettingsPtr pLabRow = GetLabRow(pCurSel);
	//TES 11/22/2009 - PLID 36191 - Go through and add a row for every field, all blank and invisible for now, but they all need to be there.
	IRowSettingsPtr pParentRow = GetNewResultsTreeRow();
	//TES 11/22/2009 - PLID 36191 - First, add the parent row, which is also the Name field.
	pParentRow->PutValue(lrtcResultID, (long)-1);
	pParentRow->PutValue(lrtcFieldID, (long)lrfName);
	pParentRow->PutValue(lrtcFieldName, _bstr_t("Name"));
	pParentRow->PutValue(lrtcValue, _bstr_t(""));
	pParentRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	//TES 9/10/2013 - PLID 58511 - Result rows now check this field
	pParentRow->PutValue(lrtcExtraValue, g_cvarFalse);
	//TES 11/22/2009 - PLID 36191 - Add it at the beginning.
	//TES 11/30/2009 - PLID 36452 - Make it the first child of this lab.
	IRowSettingsPtr pFirstChild = pLabRow->GetFirstChildRow();
	if(pFirstChild) {
		m_pResultsTree->AddRowBefore(pParentRow, pFirstChild);
	}
	else {
		m_pResultsTree->AddRowAtEnd(pParentRow, pLabRow);
	}

	//TES 11/22/2009 - PLID 36191 - Now add each of the other fields as child rows.
	//TES 4/28/2011 - PLID 43426 - Date Received By Lab
	IRowSettingsPtr pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfDateReceivedByLab);
	pRow->PutValue(lrtcFieldName, _bstr_t("Date Rec'd (Lab)"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Date Received
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfDateReceived);
	pRow->PutValue(lrtcFieldName, _bstr_t("Date Rec'd"));
	//TES 11/22/2009 - PLID 36191 - Go ahead and initialize this field to the current date.
	pRow->PutValue(lrtcValue, _variant_t(FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoDate, false)));
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
	
	// (a.walling 2010-02-25 15:46) - PLID 37546 - Date performed
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfDatePerformed);
	pRow->PutValue(lrtcFieldName, _bstr_t("Date Perf'd"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
	
	// (a.walling 2010-01-18 10:17) - PLID 36936 - LOINC code
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfLOINC);
	pRow->PutValue(lrtcFieldName, _bstr_t("LOINC"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Value
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfValue);
	pRow->PutValue(lrtcFieldName, _bstr_t("Value"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Slide #
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSlideNum);
	pRow->PutValue(lrtcFieldName, _bstr_t("Slide #"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
	
	//Diagnosis
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfDiagnosis);
	pRow->PutValue(lrtcFieldName, _bstr_t("Diagnosis"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Microscopic Description
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfMicroscopicDescription);
	pRow->PutValue(lrtcFieldName, _bstr_t("Microscopic Desc."));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Flag
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfFlag);
	pRow->PutValue(lrtcFieldName, _bstr_t("Flag"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Status
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfStatus);
	pRow->PutValue(lrtcFieldName, _bstr_t("Status"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Reference
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfReference);
	pRow->PutValue(lrtcFieldName, _bstr_t("Reference"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Units
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfUnits);
	pRow->PutValue(lrtcFieldName, _bstr_t("Units"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Comments
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfComments);
	pRow->PutValue(lrtcFieldName, _bstr_t("Comments"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Acknowledged By
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfAcknowledgedBy);
	pRow->PutValue(lrtcFieldName, _bstr_t("Acknowledged By"));
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	//Acknowledged On
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfAcknowledgedOn);
	pRow->PutValue(lrtcFieldName, _bstr_t("Acknowledged On"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-09 17:23) - PLID 57937 Specimen Identifier
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSpecimenIdentifier);
	pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Identifier"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-09 17:23) - PLID 57937 Specimen ID Text
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSpecimenIdText);
	pRow->PutValue(lrtcFieldName, _bstr_t("Specimen ID Text"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-09 17:23) - PLID 57937 Specimen Start Time
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSpecimenStartTime);
	pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Collection Start Time"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-09 17:23) - PLID 57937 Specimen End Time
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSpecimenEndTime);
	pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Collection End Time"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-09 17:23) - PLID 57937 Specimen Reject Reason
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSpecimenRejectReason);
	pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Reject Reason"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-09 17:23) - PLID 57937 Specimen Condition
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfSpecimenCondition);
	pRow->PutValue(lrtcFieldName, _bstr_t("Specimen Condition"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-16 17:32) - PLID 57600 - show Collection Start Time
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfServiceStartTime);
	pRow->PutValue(lrtcFieldName, _bstr_t("Lab Service Start Time"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-07-16 17:32) - PLID 57600 - show Collection End Time
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfServiceEndTime);
	pRow->PutValue(lrtcFieldName, _bstr_t("Lab Service End Time"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-08-07 16:09) - PLID 57912 - need to show the Performing Provider on report view of labresult tab dlg
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerformingProvider);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Provider"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerformingLab);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Name"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfObservationDate);
	pRow->PutValue(lrtcFieldName, _bstr_t("Observation Date"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-11-06 14:50) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerfLabAddress);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Address"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-11-06 14:50) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerfLabCity);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab City"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-11-06 14:50) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerfLabState);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab State"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-11-06 14:50) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerfLabZip);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Zip"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-11-06 14:50) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerfLabCountry);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Country"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);

	// (d.singleton 2013-11-06 14:50) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	pRow = GetNewResultsTreeRow();
	pRow->PutValue(lrtcResultID, (long)-1);
	pRow->PutValue(lrtcFieldID, (long)lrfPerfLabParish);
	pRow->PutValue(lrtcFieldName, _bstr_t("Performing Lab Parish"));
	pRow->PutValue(lrtcValue, g_cvarNull);
	pRow->PutValue(lrtcForeignKeyID, g_cvarNull);
	pRow->PutVisible(VARIANT_FALSE);
	m_pResultsTree->AddRowAtEnd(pRow, pParentRow);
	
	pParentRow->PutExpanded(VARIANT_TRUE);
	m_pResultsTree->CurSel = pParentRow;

	//TES 11/22/2009 - PLID 36191 - Now show the new result
	LoadResult(pParentRow);
}

void CLabResultsTabDlg::OnDeleteResult()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
		if (pRow) {
			//TES 11/30/2009 - PLID 36452 - Are we actually on a result?
			IRowSettingsPtr pResultRow = GetResultRow(pRow);
			if(!pResultRow) {
				return;
			}
			
			if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to delete this result?")) {
				return;
			}

			//TES 12/2/2009 - PLID 36191 - We need to de-map any attached files.
			// (c.haag 2011-01-07) - PLID 42047 - Fixed incorrect usage. I want to note
			// that m_strCurrentFileName is not updated here. TryDisplayOldestAttachedFile()
			// takes care of that later on.
			//m_mapAttachedFiles.RemoveKey((LPDISPATCH)pResultRow);
			{
				CMap<LPDISPATCH,LPDISPATCH,CString,CString&>* pMap;
				if(m_mapAttachedFiles.Lookup(pResultRow->GetParentRow(), pMap)) {
					if (NULL != pMap) {
						pMap->RemoveKey((LPDISPATCH)pResultRow);
					}
				}
			}

			//see what the ID is
			long nResultID = VarLong(pRow->GetValue(lrtcResultID));
			if (nResultID == -1) {
				//its not saved to data, so just remove it
				//TES 11/22/2009 - PLID 36191 - Remove the parent row.
				m_pResultsTree->RemoveRow(pResultRow);
				//clear the cur sel
				m_pResultsTree->CurSel = NULL;
				//hide the rsults
				LoadResult(NULL);

			}
			else {
				long nMailID = VarLong(GetTreeValue(pRow, lrfValue, lrtcForeignKeyID), -1);
				if (nMailID > 0) {

					//it has a document attached
					MsgBox("This result has a document attached, this document will not be detached from the history tab");
					
					m_aryDetachedDocs.Add(nResultID);
				}


				//they want to do it, mark this for deletion
				m_aryDeleteResults.Add(nResultID);

				
				
				//TES 11/22/2009 - PLID 36191 - Remove the parent row
				m_pResultsTree->RemoveRow(pResultRow);
				//clear the cur sel
				m_pResultsTree->CurSel = NULL;
				//hide the rsults
				LoadResult(NULL);

			}
		}
	}NxCatchAll("Error in CLabResultsTabDlg::OnDeleteResult()");
}

// (j.gruber 2008-10-27 16:39) - PLID 31332
// (c.haag 2011-02-22) - PLID 42589 - All calls to showing/hiding or enabling/disabling windows have been moved to UpdateControlStates
void CLabResultsTabDlg::LoadResult(NXDATALIST2Lib::IRowSettingsPtr pRow) {

	try {
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		//TES 11/20/2009 - PLID 36191 - We allow NULL to be passed in here now.
		//TES 11/30/2009 - PLID 36452 - Also make sure this has a result ID (meaning it's not a top-level Lab row)
		if(pRow == NULL || pRow->GetValue(lrtcResultID).vt == VT_NULL) {
			SetDlgItemText(IDC_RESULT_NAME, "");
			SetDlgItemText(IDC_RESULT_LOINC, ""); // (a.walling 2010-01-18 10:39) - PLID 36936
			// (a.walling 2010-01-18 11:18) - PLID 36936			
			m_dtpDateReceived.SetValue(m_varCurDate);
			m_dtpDateReceived.SetValue(COleVariant());
			SetDlgItemText(IDC_SLIDE_NUMBER, "");

			SetDlgItemText(IDC_LAB_DIAG_DESCRIPTION, "");
			SetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, "");
			SetDlgItemText(IDC_RESULT_VALUE, "");
			SetDlgItemText(IDC_RESULT_UNITS, "");
			SetDlgItemText(IDC_RESULT_REFERENCE, "");
			SetDlgItemText(IDC_RESULT_COMMENTS, "");

			// (c.haag 2010-12-29 09:18) - Don't show the controls if they're supposed to be hidden
			SetDlgItemText(IDC_RESULT_VALUE, "");
			SetDlgItemText(IDC_RESULT_UNITS, "");
			
			m_pFlagCombo->CurSel = NULL;
			m_pStatusCombo->CurSel = NULL;

			// (j.jones 2013-11-08 15:30) - PLID 58979 - disable Pt. Education
			m_btnPatientEducation.EnableWindow(FALSE);
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - If we are showing it as a link, fine, but otherwise it is a label (or hidden elsewhere)
			bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
			m_nxlabelPatientEducation.SetType(bShowPatientEducationLink ? dtsDisabledHyperlink : dtsDisabledText);
			m_nxlabelPatientEducation.AskParentToRedrawWindow();

			if(pRow == NULL) {
				// (z.manning 2010-11-09 17:22) - PLID 41395 - We always want to display the attached PDF
				// if there's only one in the entire lab.
				TryDisplayOldestAttachedFile();
			}
			else {
				//TES 2/3/2010 - PLID 36862 - We're on a top-level row, go ahead and load the first attached file for this specimen (if any).
				CString strAttachedFile;
				CMap<LPDISPATCH,LPDISPATCH,CString,CString&> *pMap = NULL;
				if(m_mapAttachedFiles.Lookup(pRow, pMap)) {
					POSITION pos = pMap->GetStartPosition();
					if(pos) {
						LPDISPATCH pKey;
						pMap->GetNextAssoc(pos, pKey, strAttachedFile);
					}
				}
				if(!strAttachedFile.IsEmpty()) {
					//TES 11/23/2009 - PLID 36192 - Yes!  Navigate to it.
					CString strFileName = strAttachedFile;
					if(strFileName.Find("\\") == -1) {
						strFileName = GetPatientDocumentPath(m_nPatientID) ^ strFileName;
					}
					//TES 2/3/2010 - PLID 37191 - Remember the attached file.
					m_strCurrentFileName = strFileName;
					//TES 11/23/2009 - PLID 36192 - If this is a .pdf file, we want to append #toolbar=0, to hide the .pdf controls.
					if(strFileName.Right(4).CompareNoCase(".pdf") == 0) {
						strFileName += "#toolbar=0";
					}
					COleVariant varUrl(strFileName);
					// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
					m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
					GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE);
					//TES 2/3/2010 - PLID 37191 - Enable the Zoom button.
					m_btnZoom.EnableWindow(TRUE);
				}
				else {
					// (z.manning 2010-11-09 17:22) - PLID 41395 - We always want to display the attached PDF
					// if there's only one in the entire lab.
					TryDisplayOldestAttachedFile();
				}
			}
		}
		else {
			IRowSettingsPtr pParent = GetResultRow(pRow);
			
			SetDlgItemText(IDC_RESULT_NAME, VarString(pParent->GetValue(lrtcValue), ""));

			_variant_t var = GetTreeValue(pRow, lrfDateReceived, lrtcValue);
			if (var.vt == VT_DATE) {
				COleDateTime dt  = VarDateTime(var);
				if (dt.GetStatus() == COleDateTime::valid) {
					m_dtpDateReceived.SetValue(dt);
				}
				else {
					m_dtpDateReceived.SetValue(m_varCurDate);
					//but we want it unchecked
					m_dtpDateReceived.SetValue(COleVariant());
				}			
			}
			else if (var.vt == VT_BSTR) { 
				COleDateTime dtReceived;
				if (VarString(var, "").IsEmpty()) {
					m_dtpDateReceived.SetValue(m_varCurDate);
					//but we want it unchecked
					m_dtpDateReceived.SetValue(COleVariant());
				}
				else {
					dtReceived.ParseDateTime(VarString(var, ""));
					if (dtReceived.GetStatus() == COleDateTime::valid) {
						m_dtpDateReceived.SetValue(dtReceived);
					}
					else {
						//set it to be today unchecked
						m_dtpDateReceived.SetValue(m_varCurDate);
						m_dtpDateReceived.SetValue(COleVariant());
					}
				}
			}
			else {
				//set it to the current time
				m_dtpDateReceived.SetValue(m_varCurDate);
				//but we want it unchecked
				m_dtpDateReceived.SetValue(COleVariant());
			}

			// (j.jones 2013-11-08 15:30) - PLID 58979 - enable Pt. Education
			m_btnPatientEducation.EnableWindow(TRUE);
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - If we are showing it as a link, fine, but otherwise it is a label (or hidden elsewhere)
			bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
			m_nxlabelPatientEducation.SetType(bShowPatientEducationLink ? dtsHyperlink : dtsText);
			m_nxlabelPatientEducation.AskParentToRedrawWindow();

			// (a.walling 2010-01-18 11:18) - PLID 36936 - Update the hyperlink state if necessary
			if (m_nxlabelLOINC.GetType() != dtsHyperlink) {
				m_nxlabelLOINC.SetType(dtsHyperlink);
				m_nxlabelLOINC.AskParentToRedrawWindow();
			}
			_variant_t varLOINC = GetTreeValue(pRow, lrfLOINC, lrtcValue);
			if (varLOINC.vt == VT_BSTR) {
				SetDlgItemText(IDC_RESULT_LOINC, VarString(varLOINC));
			} else {
				SetDlgItemText(IDC_RESULT_LOINC, "");
			}

			// (r.galicki 2008-10-21 13:04) - PLID 31552 - Handle hidden columns
			if(m_ltType == ltBiopsy) { // (r.galicki 2008-10-17 14:50) - PLID 31552
				_variant_t varValue = GetTreeValue(pRow, lrfSlideNum, lrtcValue);
				if(VT_BSTR == varValue.vt) {
					SetDlgItemText(IDC_SLIDE_NUMBER, VarString(varValue, ""));
				}
				//TES 3/10/2009 - PLID 33433 - Even if the value is blank, we still need to update the edit box!
				else {
					SetDlgItemText(IDC_SLIDE_NUMBER, "");
				}
			}			
			
			SetDlgItemText(IDC_LAB_DIAG_DESCRIPTION, VarString(GetTreeValue(pRow, lrfDiagnosis, lrtcValue), ""));
			
			if(m_ltType == ltBiopsy) { // (r.galicki 2008-10-17 14:55) - PLID 31552
				// (c.haag 2010-12-29 09:18) - Don't show the controls if they're supposed to be hidden
				_variant_t varValue = GetTreeValue(pRow, lrfMicroscopicDescription, lrtcValue); // (r.galicki 2008-10-21 13:00) - PLID 31552
				if(VT_BSTR == varValue.vt) {
					SetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, VarString(varValue, ""));
				}
				//TES 3/10/2009 - PLID 33433 - Even if the value is blank, we still need to update the edit box!
				else {
					SetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, "");
				}

				// (b.spivey, August 27, 2013) - PLID 46295 - Show anatomical location on the discrete/report views.
				CString strAnatomicalLocation = GetAnatomicalLocationAsString(GetCurrentLab());
				m_nxstaticAnatomicalLocationTextDiscrete.SetWindowTextA(strAnatomicalLocation);
				m_nxstaticAnatomicalLocationTextReport.SetWindowTextA(strAnatomicalLocation);

				// (r.goldschmidt 2016-02-25 17:05) - PLID 68267 - remove the filter on the clinical diagnosis drop down
				RemoveFilterClinicalDiagnosisOptionsList();

			}

			long nMailID = VarLong(GetTreeValue(pRow, lrfValue, lrtcForeignKeyID),-1);
			if (nMailID == -1) 
			{
				SetDlgItemText(IDC_RESULT_VALUE, VarString(GetTreeValue(pRow, lrfValue, lrtcValue), ""));
			}
	
			SetDlgItemText(IDC_RESULT_UNITS, VarString(GetTreeValue(pRow, lrfUnits, lrtcValue), "")); // (c.haag 2009-05-06 14:53) - PLID 33789
			SetDlgItemText(IDC_RESULT_REFERENCE, VarString(GetTreeValue(pRow, lrfReference, lrtcValue), ""));
			SetDlgItemText(IDC_RESULT_COMMENTS, VarString(GetTreeValue(pRow, lrfComments, lrtcValue), ""));

			long nFlagID = VarLong(GetTreeValue(pRow, lrfFlag, lrtcForeignKeyID),-1);
			long nResult = m_pFlagCombo->TrySetSelByColumn_Deprecated(lfcFlagID, nFlagID);
			if(nResult == sriNoRow) {
				//TES 2/2/2010 - PLID 34336 - If it's inactive, look up the name and fill the combo box text.
				if(nFlagID != -1) {
					_RecordsetPtr rsFlag = CreateParamRecordset("SELECT Name FROM LabResultFlagsT WHERE ID = {INT}", nFlagID);
					m_pFlagCombo->PutComboBoxText(_bstr_t(AdoFldString(rsFlag, "Name","")));
				}
			}
			else if(nResult == sriNoRowYet_WillFireEvent) {
				//TES 2/2/2010 - PLID 34336 - Let the user know what's happening.
				m_pFlagCombo->PutComboBoxText("Loading Flag...");
			}
			
			//TES 12/1/2008 - PLID 32191 - Added a field for Status
			m_pStatusCombo->SetSelByColumn(lscStatusID, GetTreeValue(pRow, lrfStatus, lrtcForeignKeyID));

			//TES 11/23/2009 - PLID 36192 - Does this result have a file attached?
			CString strAttachedFile;
			//TES 1/27/2010 - PLID 36862 - Get all files for the current requisition
			CMap<LPDISPATCH,LPDISPATCH,CString,CString&> *pMap = NULL;
			if(GetAttachedFiles(pParent, pMap)) {
				//TES 1/27/2010 - PLID 36862 - Now, use the file attached to this specific result, if any; otherwise just pull the first 
				// attached file.
				if(!pMap->Lookup(pParent, strAttachedFile)) {
					POSITION pos = pMap->GetStartPosition();
					if(pos) {
						LPDISPATCH pKey;
						pMap->GetNextAssoc(pos, pKey, strAttachedFile);
					}
				}

				if(!strAttachedFile.IsEmpty()) {
					//TES 11/23/2009 - PLID 36192 - Yes!  Navigate to it.
					CString strFileName = strAttachedFile;
					if(strFileName.Find("\\") == -1) {
						strFileName = GetPatientDocumentPath(m_nPatientID) ^ strFileName;
					}
					//TES 2/3/2010 - PLID 37191 - Remember which file we're previewing.
					m_strCurrentFileName = strFileName;
					//TES 11/23/2009 - PLID 36192 - If this is a .pdf file, we want to append #toolbar=0, to hide the .pdf controls.
					if(strFileName.Right(4).CompareNoCase(".pdf") == 0) {
						strFileName += "#toolbar=0";
					}
					COleVariant varUrl(strFileName);
					// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
					m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
					GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE);
					//TES 2/3/2010 - PLID 37191 - Enable the Zoom button
					UpdateZoomButton();
				}
				else {
					// (z.manning 2010-11-09 17:22) - PLID 41395 - We always want to display the attached PDF
					// if there's only one in the entire lab.
					TryDisplayOldestAttachedFile();
				}

			}
			else {
				// (z.manning 2010-11-09 17:22) - PLID 41395 - We always want to display the attached PDF
				// if there's only one in the entire lab.
				TryDisplayOldestAttachedFile();
			}
		}

		// (c.haag 2010-12-01 11:45) - PLID 37372 - Signature fields
		IRowSettingsPtr pParent = GetResultRow(pRow);
		if (NULL == pParent) {
			// If we don't have a result row selected, it may be the case that a lab row is selected and it has a single
			// result. Try to assign that as our parent.
			pParent = GetFirstAndOnlyLabResultRow(pRow);
		}

		// (c.haag 2010-12-10 9:45) - PLID 40556 - Update the acknowledged button text
		// (c.haag 2011-02-21) - PLID 41618 - Pass in the active result
		FormatAcknowledgedButtonText(pRow);
		// (f.gelderloos 2013-08-28 15:51) - PLID 57826
		FormatAcknowledgeAndSignButtonText(pRow);
		// (c.haag 2010-12-01 11:57) - PLID 37372 - Update the signature button text
		// (c.haag 2011-02-21) - PLID 41618 - Pass in the active result
		FormatSignButtonText(pRow);
		// (c.haag 2010-12-02 10:28) - PLID 38633 - Update the mark completed button text
		// (c.haag 2011-02-21) - PLID 41618 - Pass in the active result
		FormatMarkCompletedButtonText(pRow);
		FormatMarkAllCompleteButtonText(pRow); // (j.luckoski 3-21-13) PLID 55424 - Format button
		// (c.haag 2010-12-06 13:48) - PLID 41618 - Update the scroll buttons
		UpdateScrollButtons();
		// (c.haag 2011-02-23) - PLID 41618 - Update the detach/attach button
		UpdateDetachButton();

		// (j.dinatale 2010-12-27) - PLID 41591 - Format buttons based on the row passed in
		FormatNotesButtons(pRow);

		// (c.haag 2011-02-22) - PLID 42589 - Update control visibilities and enabled/disabled states
		UpdateControlStates(m_bControlsHidden, pRow);

		m_bResultIsSaved = TRUE;
	
	}NxCatchAll("Error in CLabResultsTabDlg::LoadResult");

}

// (j.gruber 2008-10-27 16:39) - PLID 31332
BOOL CLabResultsTabDlg::SaveResult(NXDATALIST2Lib::IRowSettingsPtr pRow) {

	try {
		//TES 11/30/2009 - PLID 36452 - Make sure we're on a row, and that it's tied to a result.
		if(pRow == NULL || VarLong(pRow->GetValue(lrtcFieldID)) == (long)lrfLabName) {
			return TRUE;
		}

		// (c.haag 2009-05-06 15:08) - PLID 33789 - Added 'Units'
		// (a.walling 2010-01-18 10:41) - PLID 36936 - Added LOINC
		CString strName, strSlideNum, strDiagnosis, strMicroDesc, strValue, strUnits, strReference, strFlag, strResultComments, strLOINC;
		long nFlagID;
		COleDateTime dtReceived;

		GetDlgItemText(IDC_RESULT_NAME, strName);
		_variant_t varReceived = m_dtpDateReceived.GetValue();
		BOOL bUseDate = FALSE;
		if (varReceived.vt == VT_DATE) {
			COleDateTime dt = m_dtpDateReceived.GetDateTime();
			if (dt.GetStatus() == COleDateTime::valid) {
				dtReceived = dt;
				bUseDate = TRUE;
			}
			else {
				bUseDate = FALSE;
			}
		}
		else if (varReceived.vt == VT_NULL) {
			bUseDate = FALSE;
		}

		// (a.walling 2010-01-18 10:42) - PLID 36936
		GetDlgItemText(IDC_RESULT_LOINC, strLOINC);

		GetDlgItemText(IDC_SLIDE_NUMBER, strSlideNum);
		GetDlgItemText(IDC_LAB_DIAG_DESCRIPTION, strDiagnosis);
		GetDlgItemText(IDC_CLINICAL_DIAGNOSIS_DESC, strMicroDesc);
		GetDlgItemText(IDC_RESULT_VALUE, strValue);
		GetDlgItemText(IDC_RESULT_UNITS, strUnits); // (c.haag 2009-05-06 15:09) - PLID 33789
		GetDlgItemText(IDC_RESULT_REFERENCE, strReference);
		// (z.manning 2009-04-30 16:59) - PLID 28560 - Added result comments
		GetDlgItemText(IDC_RESULT_COMMENTS, strResultComments);


		NXDATALIST2Lib::IRowSettingsPtr pFlagRow = m_pFlagCombo->CurSel;
		if (pFlagRow) {
			nFlagID = VarLong(pFlagRow->GetValue(lfcFlagID), -1);
			strFlag = VarString(pFlagRow->GetValue(lfcName), "");
		}
		else {
			//TES 2/2/2010 - PLID 34336 - It may be inactive, in which case we'll pull from the tree and ComboBoxText.
			if(!m_pFlagCombo->IsComboBoxTextInUse) {
				nFlagID = -1;
				strFlag = "";
			}
			else {
				//(e.lally 2010-05-13) PLID 38629 - We want the long value here to avoid a type mismatch exception.
				nFlagID = VarLong(GetTreeValue(pRow, lrfFlag, lrtcForeignKeyID),-1);
				strFlag = CString((LPCTSTR)m_pFlagCombo->ComboBoxText);
			}
		}

		COleDateTime dtCurDate;
		//get the stored server date for comparison
		if(m_varCurDate.vt != VT_NULL)
			dtCurDate = VarDateTime(m_varCurDate);
		else{
			//The member variable was not set!
			ASSERT(FALSE);
			//We can just use the local time
			dtCurDate = COleDateTime::GetCurrentTime();
		}

		//Date Received
		if(m_dtpDateReceived.GetValue().vt != VT_NULL){
			//We just want to compare the date part in case the time got saved too.
			COleDateTime dtTemp, dtRecVal;
			dtTemp = VarDateTime(m_dtpDateReceived.GetValue());
			dtRecVal.SetDate(dtTemp.GetYear(), dtTemp.GetMonth(), dtTemp.GetDay());
			if(dtRecVal > dtCurDate){
				MessageBox("You cannot have a Date Received that is in the future.");
				return FALSE;
			}

			if (dtRecVal.GetStatus() != COleDateTime::valid) {
				MessageBox("Please enter a valid Date Received.");
				return FALSE;
			}

		}
		//Slide Number
		if(strSlideNum.GetLength() > 25){
			MessageBox("The Slide Number entered is longer than 25 characters. "
				"Please shorten it before saving.");
			GetDlgItem(IDC_SLIDE_NUMBER)->SetFocus();
			return FALSE;
		}

		//Result Name
		if(strName.GetLength() > 255){
			MessageBox("The Result Name entered is longer than 255 characters. "
				"Please shorten it before saving.");
			GetDlgItem(IDC_RESULT_NAME)->SetFocus();
			return FALSE;
		}

		//Reference
		if(strReference.GetLength() > 255){
			MessageBox("The Result Reference entered is longer than 255 characters. "
				"Please shorten it before saving.");
			GetDlgItem(IDC_RESULT_REFERENCE)->SetFocus();
			return FALSE;
		}

		// (r.gonet 06/24/2014) - PLID 61685 - Removed check for > 255 on Result Comments because the field is now NVARCHAR(MAX).

		//TES 12/1/2008 - PLID 32191 - Pull the Status field
		long nStatusID;
		CString strStatus;
		NXDATALIST2Lib::IRowSettingsPtr pStatusRow = m_pStatusCombo->CurSel;
		if (pStatusRow) {
			nStatusID = VarLong(pStatusRow->GetValue(lscStatusID), -1);
			strStatus = VarString(pStatusRow->GetValue(lscDescription), "");
		}
		else {
			nStatusID = -1;
			strStatus = "";
		}

		SetTreeValue(pRow, lrfName, lrtcValue, _variant_t(strName)); 
		if (bUseDate) {
			//TES 12/1/2008 - PLID 32281 - Put the actual date value in there, not a string representation (which,
			// incidentally, was stripping out the time).
			SetTreeValue(pRow, lrfDateReceived, lrtcValue, _variant_t(dtReceived, VT_DATE));
		}
		else {
			SetTreeValue(pRow, lrfDateReceived, lrtcValue, _variant_t(""));
		}

		// (a.walling 2010-01-18 10:42) - PLID 36936
		SetTreeValue(pRow, lrfLOINC, lrtcValue, strLOINC.IsEmpty() ? g_cvarNull : _variant_t(strLOINC));
				
		SetTreeValue(pRow, lrfSlideNum, lrtcValue, _variant_t(strSlideNum));
		SetTreeValue(pRow, lrfFlag, lrtcForeignKeyID, (long)nFlagID);
		SetTreeValue(pRow, lrfFlag, lrtcValue, _variant_t(strFlag));
		SetTreeValue(pRow, lrfDiagnosis, lrtcValue, _variant_t(strDiagnosis));
		SetTreeValue(pRow, lrfMicroscopicDescription, lrtcValue, _variant_t(strMicroDesc));
		if(VarLong(GetTreeValue(pRow, lrfValue, lrtcForeignKeyID),-1) == -1) {
			SetTreeValue(pRow, lrfValue, lrtcValue, _variant_t(strValue));
		}
		SetTreeValue(pRow, lrfUnits, lrtcValue, _variant_t(strUnits)); // (c.haag 2009-05-06 14:53) - PLID 33789
		SetTreeValue(pRow, lrfReference, lrtcValue, _variant_t(strReference));
		//TES 12/1/2008 - PLID 32191 - Added columns for the Status field
		SetTreeValue(pRow, lrfStatus, lrtcForeignKeyID, nStatusID);
		SetTreeValue(pRow, lrfStatus, lrtcValue, _variant_t(strStatus));
		// (z.manning 2009-04-30 16:59) - PLID 28560 - Added result comments
		SetTreeValue(pRow, lrfComments, lrtcValue, _variant_t(strResultComments));

		//Don't fill mailID or DocPath here since they are handled with the attach and detach buttons

		//m_btnSaveResultChanges.EnableWindow(FALSE);
		//m_btnCancelResultChanges.EnableWindow(FALSE);

		m_bResultIsSaved = TRUE;
		return TRUE;
			
	}NxCatchAll("Error in CLabResultsTabDlg::SaveResult()");
	
	return FALSE;

}

void CLabResultsTabDlg::OnDateTimeChangedDateReceived(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		HandleDateReceivedChange();

	}NxCatchAll("Error in CLabEntryDlg::OnDatetimechangeDateReceived");
	
	*pResult = 0;
}

// (j.gruber 2008-10-27 16:38) - PLID 31332
void CLabResultsTabDlg::HandleDateReceivedChange() {

	
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
		if (pRow) {
			
			COleDateTime dtColVal, dtCurVal;
			CString strColVal, strCurVal;
			_variant_t varDateReceived = GetTreeValue(pRow, lrfDateReceived, lrtcValue);
			if (varDateReceived.vt == VT_DATE) {
				dtColVal = VarDateTime(varDateReceived);
				if (dtColVal.GetStatus() == COleDateTime::valid) {
					strColVal = FormatDateTimeForSql(dtColVal, dtoDate);
				}
				else {
					strColVal = "";
				}
			}
			else if (varDateReceived.vt == VT_BSTR) {
				if (VarString(varDateReceived, "").IsEmpty()) {
					strColVal = "";
				}
				else {
					dtColVal.ParseDateTime(VarString(varDateReceived, ""));				
					if (dtColVal.GetStatus() == COleDateTime::valid) {
						strColVal = FormatDateTimeForSql(dtColVal);
					}
					else {
						strColVal = "";
					}
				}
			}
			dtCurVal = m_dtpDateReceived.GetValue();
			if (m_dtpDateReceived.GetValue().vt == VT_NULL) {
				strCurVal = "";
			}
			else {
				if (dtCurVal.GetStatus() == COleDateTime::valid) {
					strCurVal = FormatDateTimeForSql(dtCurVal, dtoDate);
				}
				else {
					strCurVal = "";
				}
			}
			if (strColVal != strCurVal) {
				//(e.lally 2009-09-22) PLID 35609 - We have unsaved result changes
				m_bResultIsSaved = FALSE;
			}
		}
}
void CLabResultsTabDlg::OnSelChosenLabFlag(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		long nNewFlagID;
		CString strNewFlag;
		//TES 8/6/2013 - PLID 51147 - Need to pull the Todo Priority as well
		_variant_t varNewTodoPriority = g_cvarNull;
		if (pRow) {
			nNewFlagID = VarLong(pRow->GetValue(lfcFlagID));
			strNewFlag = VarString(pRow->GetValue(lfcName));
			varNewTodoPriority = pRow->GetValue(lfcTodoPriority);
		}
		else {
			nNewFlagID = -1;
		}
		pRow = m_pResultsTree->CurSel;
		long nCurrentFlagID;
		if (pRow) {
			nCurrentFlagID = VarLong(GetTreeValue(pRow, lrfFlag, lrtcForeignKeyID),-1);
		}

		if (nNewFlagID != nCurrentFlagID) {
			//(e.lally 2009-09-22) PLID 35609 - We have unsaved result changes
			m_bResultIsSaved = FALSE;
		}

		//TES 2/2/2010 - PLID 34336 - We need to set this value in the tree right away, so we can look it up in there for inactive flags.
		SetTreeValue(pRow, lrfFlag, lrtcForeignKeyID, nNewFlagID);
		SetTreeValue(pRow, lrfFlag, lrtcValue, _bstr_t(strNewFlag));
		//TES 8/6/2013 - PLID 51147 - Save the new Todo Priority to the tree
		SetTreeValue(pRow, lrfFlag, lrtcExtraValue, varNewTodoPriority);

	}NxCatchAll("Error in CLabResultsTabDlg::OnSelChosenLabFlag()");
}

void CLabResultsTabDlg::OnRequeryFinishedLabFlag(short nFlags)
{
	try{
		IRowSettingsPtr pRow = m_pFlagCombo->GetNewRow();
		pRow->PutValue(lfcFlagID, (long)-1);
		pRow->PutValue(lfcName, "< No Flag >");
		m_pFlagCombo->AddRowBefore(pRow, m_pFlagCombo->GetFirstRow());	
	}NxCatchAll("Error in CLabResultsTabDlg::OnRequeryFinishedLabFlag()");
}

void CLabResultsTabDlg::OnAttachFile()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
		IRowSettingsPtr pParent = GetResultRow(pRow);
		//Make sure we actually have a row selected
		if(pRow != NULL && pParent != NULL) {

			// (b.spivey, July 11, 2013) - PLID 45194 - Menu options to scan as image/pdf/multipagepdf. 
			CMenu submenu;
			submenu.CreatePopupMenu();
			submenu.AppendMenu(MF_ENABLED|MF_BYPOSITION, ID_SCAN_PDF, "Scan as PDF...");
			submenu.AppendMenu(MF_ENABLED|MF_BYPOSITION, ID_SCAN_MULTIPDF, "Scan as Multi-Page PDF...");
			submenu.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
			submenu.AppendMenu(MF_ENABLED|MF_BYPOSITION, ID_SCAN_IMAGE, "Scan as Image...");

			// (j.dinatale 2013-03-04 12:38) - PLID 34339 - context menu here
			CMenu mnu;
			mnu.CreatePopupMenu();

			mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, 1, "&Existing File From History...");
			mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, 2, "&New File...");

			// (b.spivey, July 10, 2013) - PLID 45194 - Insert the menu created earlier. 
			mnu.InsertMenu(3, MF_BYPOSITION|MF_POPUP, (UINT_PTR)submenu.GetSafeHmenu(), "Import from &Scanner");

			CRect rButton;
			GetDlgItem(IDC_ATTACH_FILE)->GetWindowRect(rButton);
			int nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, rButton.right, rButton.top, this);

			switch(nSelection){
				case 1:		// attach existing
					AttachExistingFile(pRow, pParent);
					break;
				case 2:		// attach new
					AttachNewFile(pRow, pParent);
					break;
				case 0:		// nothing selected
					break;
				case ID_SCAN_IMAGE:
					DoTWAINAcquisition(NXTWAINlib::eScanToImage); 
					break;
				case ID_SCAN_PDF:
					DoTWAINAcquisition(NXTWAINlib::eScanToPDF); 
					break;
				case ID_SCAN_MULTIPDF:
					DoTWAINAcquisition(NXTWAINlib::eScanToMultiPDF); 
					break;
				default:
					ThrowNxException("CLabResultsTabDlg::OnAttachFile() - Invalid Menu Option!");
			}
		}


	}NxCatchAll("Error in CLabResultsTabDlg::OnAttachFile()");
}

void CLabResultsTabDlg::OnDetachFile()
{
	try {
		// (c.haag 2011-02-22) - PLID 41618 - Use different behavior based on what view we are in
		if (m_nCurrentView == rvPDF)
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnDetachFile();
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnDetachFile without a valid attachment view!");
			}		
		}
		else 
		{
			// (c.haag 2011-02-22) - PLID 41618 - We now track the attachment row here
			DetachFile(GetResultRow(m_pResultsTree->CurSel));
		}
	}
	NxCatchAll("Error in CLabResultsTabDlg::OnDetachFile()");
}

// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for detaching files.
void CLabResultsTabDlg::DetachFile(IRowSettingsPtr pRow)
{
	//get the mailID
	IRowSettingsPtr pParent = GetResultRow(pRow);
	if (pRow != NULL && pParent != NULL) {
		pRow = m_pResultsTree->FindByColumn(lrtcFieldID, (long)lrfValue, pParent, VARIANT_FALSE);
		ASSERT(pRow->GetParentRow() == pParent);
		
		long nMailID = VarLong(pRow->GetValue(lrtcForeignKeyID), -1);
		long nResultID = VarLong(pRow->GetValue(lrtcResultID), -1);

		//they shouldn't be ablt the click the detach button without a document attached or going to be attached
		ASSERT(nMailID != -1);

		if (nMailID == -2) {
			//they've added this document without saving to the database
			//so just reset everything			
		}
		else {

			if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to detach this file from this result?")) {
				return;
			}

			m_aryDetachedDocs.Add(nResultID);
		}

		pRow->PutValue(lrtcValue, _variant_t(""));
		SetTreeValue(pRow, lrfUnits, lrtcValue, _variant_t("")); // (c.haag 2009-05-06 14:54) - PLID 33789
		pRow->PutValue(lrtcForeignKeyID, (long)-1);
		SetDlgItemText(IDC_DOC_PATH_LINK, "");
		SetDlgItemText(IDC_RESULT_VALUE, "");
		SetDlgItemText(IDC_RESULT_UNITS, "");
		
		//TES 11/23/2009 - PLID 36192 - Remember that there is no file attached to this row.
		//TES 2/3/2010 - PLID 36862 - Look up in our new, two-level map.
		CMap<LPDISPATCH,LPDISPATCH,CString,CString&>* pMap;
		if(m_mapAttachedFiles.Lookup(pParent->GetParentRow(), pMap)) {
			//TES 2/8/2010 - PLID 36862 - Check if the file being detached is the one we're currently previewing it, and if so,
			// remember that we aren't previewing it any more.
			CString strFileName;
			pMap->Lookup(pParent, strFileName);
			if(strFileName == m_strCurrentFileName) {
				m_strCurrentFileName = "";
			}
			pMap->RemoveKey((LPDISPATCH)pParent);
		}
		//TES 11/23/2009 - PLID 36192 - Go ahead and re-load the result, this will update the browser and other controls.
		SaveResult(pRow);
		LoadResult(pRow);
	}
}

void CLabResultsTabDlg::OnRequeryFinishedLabStatus(short nFlags)
{
	try {
		//TES 11/28/2008 - PLID 32191 - Add a <None> row.
		IRowSettingsPtr pRow = m_pStatusCombo->GetNewRow();
		pRow->PutValue(lscStatusID, (long)-1);
		pRow->PutValue(lscDescription, "");
		m_pStatusCombo->AddRowBefore(pRow, m_pStatusCombo->GetFirstRow());	
	}NxCatchAll("Error in CLabResultsTabDlg::OnRequeryFinishedLabStatus()");
}

void CLabResultsTabDlg::OnEditResultStatus()
{
	try{
		
		CEditLabStatusDlg dlg(this);

		//TES 12/4/2008 - PLID 32191 - Temporarily save the current selection for later
		IRowSettingsPtr pRow = m_pStatusCombo->GetCurSel();
		long nValue = -1;
		if(pRow != NULL) {
			nValue = VarLong(pRow->GetValue(lscStatusID), -1);
		}

		//TES 12/4/2008 - PLID 32191 - Bring up the dialog.
		dlg.DoModal();

		//TES 12/4/2008 - PLID 32191 - Refresh the list, now that it's probably changed.
		m_pStatusCombo->Requery();

		//TES 12/4/2008 - PLID 32191 - Set the selection back to what it was
		m_pStatusCombo->SetSelByColumn(lscStatusID, nValue);

	}NxCatchAll("Error in CLabResultsTabDlg::OnEditResultStatus()");
}

void CLabResultsTabDlg::OnSelChosenLabStatus(LPDISPATCH lpRow)
{
	try {
		//TES 12/1/2008 - PLID 32191 - Added a status field, track when the user changes the selection.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		long nNewStatusID;
		if (pRow) {
			nNewStatusID = VarLong(pRow->GetValue(lscStatusID));
		}
		else {
			nNewStatusID = -1;
		}
		pRow = m_pResultsTree->CurSel;
		long nCurrentStatusID;
		if (pRow) {
			nCurrentStatusID = VarLong(GetTreeValue(pRow, lrfStatus, lrtcForeignKeyID),-1);
		}

		if (nNewStatusID != nCurrentStatusID) {
			//(e.lally 2009-09-22) PLID 35609 - We have unsaved result changes
			m_bResultIsSaved = FALSE;
		}

	}NxCatchAll("Error in CLabResultsTabDlg::OnSelChosenLabStatus()");
}

void CLabResultsTabDlg::SetCurrentDate(_variant_t varCurDate)
{
	m_varCurDate = varCurDate;
}

//TES 11/22/2009 - PLID 36191 - Pulls the value from the tree that corresponds to the result represented by pRow, the field represented by lrf,
// and the column represented by lrtc
_variant_t CLabResultsTabDlg::GetTreeValue(NXDATALIST2Lib::IRowSettingsPtr pRow, LabResultField lrf, LabResultTreeColumns lrtc)
{
	//TES 11/22/2009 - PLID 36191 - Make sure something's selected.
	if(pRow == NULL) {
		return g_cvarNull;
	}
	//TES 11/22/2009 - PLID 36191 - Pull the parent row for this result.
	//TES 11/30/2009 - PLID 36452 - This may be NULL, if we're on a Lab row.
	IRowSettingsPtr pResultParent = GetResultRow(pRow);
	if(pResultParent == NULL) {
		return g_cvarNull;
	}
	
	if(lrf == lrfName) {
		//TES 11/22/2009 - PLID 36191 - The name is the parent, so we've already got the right row.
		return pResultParent->GetValue(lrtc);
	}
	else {
		//TES 11/22/2009 - PLID 36191 - Find the row for this field
		pRow = m_pResultsTree->FindByColumn(lrtcFieldID, (long)lrf, pResultParent, VARIANT_FALSE);
		if(pRow == NULL || pRow->GetParentRow() != pResultParent) {
			//TES 11/22/2009 - PLID 36191 - That's impossible!  Every LabResultField value should be a child of this parent!
			AfxThrowNxException(FormatString("Failed to find stored value for LabResultField %i!", lrf));
		}
		//TES 11/22/2009 - PLID 36191 - Return the value.
		return pRow->GetValue(lrtc);
	}
}

//TES 11/22/2009 - PLID 36191 - Puts the given value into the tree, using the given column, and the row given by lrf that is for the same result
// as pRow is for.
void CLabResultsTabDlg::SetTreeValue(NXDATALIST2Lib::IRowSettingsPtr pRow, LabResultField lrf, LabResultTreeColumns lrtc, _variant_t varValue)
{
	//TES 11/22/2009 - PLID 36191 - Make sure we have a row.
	if(pRow == NULL) {
		return;
	}
	//TES 11/30/2009 - PLID 36452 - Make sure that row is for a result (i.e., not a top-level Lab row).
	IRowSettingsPtr pResultParent = GetResultRow(pRow);
	if(pResultParent == NULL) {
		return;
	}
	
	if(lrf == lrfName) {
		//TES 11/22/2009 - PLID 36191 - the name is the parent, so we've already got the right row.
		pResultParent->PutValue(lrtc, varValue);
	}
	else {
		//TES 11/22/2009 - PLID 36191 - Find the row for this field
		pRow = m_pResultsTree->FindByColumn(lrtcFieldID, (long)lrf, pResultParent, VARIANT_FALSE);
		if(pRow == NULL || pRow->GetParentRow() != pResultParent) {
			//TES 11/22/2009 - PLID 36191 - That's impossible!  Every LabResultField value should be a child of this parent!
			AfxThrowNxException(FormatString("Failed to find stored value for LabResultField %i!", lrf));
		}
		//TES 11/22/2009 - PLID 36191 - Set the value.
		pRow->PutValue(lrtc, varValue);
		//TES 11/22/2009 - PLID 36191 - Now, if this is a non-empty value (and this isn't one of our always-hidden fields), make sure
		// that the row gets shown.
		if(varValue.vt != VT_NULL && lrf != lrfAcknowledgedBy && lrf != lrfAcknowledgedOn) {
			if((varValue.vt != VT_BSTR || VarString(varValue) != "") &&
				(varValue.vt != VT_I4 || VarLong(varValue) != -1)) {
				pRow->Visible = VARIANT_TRUE;
			}
		}
	}
}

void CLabResultsTabDlg::OnSelChangedResultsTree(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewSel);
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);

		//we already prompted to save

		if (pNewRow != pOldRow) {

			//TES 11/22/2009 - PLID 36191 - Show the newly selected result.
			LoadResult(pNewRow);

			if(pNewRow) {
				//TES 11/30/2009 - PLID 36452 - Tell our dialog to synchronize the Requisitions tab with the lab we currently have selected.
				IRowSettingsPtr pLabRow = GetLabRow(pNewRow);
				m_pLabEntryDlg->SetCurrentLab(VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1));
			}
		}		
		
	}NxCatchAll("Error in CLabResultsTabDlg::OnSelChangedResultsTree()");
}
void CLabResultsTabDlg::OnSelChangingResultsTree(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);

		//see if we need to prompt to save
		if (pOldRow != NULL && !m_bResultIsSaved) {
			if (!SaveResult(pOldRow))  {
				SafeSetCOMPointer(lppNewSel, lpOldSel);				
			}
		}
	}NxCatchAll("Error in CLabResultsTabDlg::OnSelChangingResultsTree()");
}
void CLabResultsTabDlg::OnRButtonDownResultsTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		//TES 10/14/2008 - PLID 31637 - Just in case there's any information in the HL7 message that they need, allow
		// them to view the original message.  This is basically just a fail-safe that will allow any office that finds
		// they are missing information to work around that while we get them a patch.
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (r.gonet 06/02/2014) - PLID 40426 - Find the corresponding lab row and see if the row they right clicked on is the lab row.
		bool bIsLabRow = true;
		NXDATALIST2Lib::IRowSettingsPtr pLabRow = pRow;
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = NULL;
		while ((pParentRow = pLabRow->GetParentRow()) != NULL) {
			bIsLabRow = false;
			pLabRow = pParentRow;
		}

		// (r.gonet 06/02/2014) - PLID 40426 - Only show the context menu if we are either on a result row or on a lab row with results
		if ((bIsLabRow && pLabRow->GetFirstChildRow() != NULL) || !bIsLabRow) {
			// (r.gonet 06/02/2014) - PLID 40426 - Create a context menu
			CMenu pMenu;
			pMenu.CreatePopupMenu();
			long nPosition = 0;
			// (r.gonet 06/02/2014) - PLID 40426 - Depending on what was clicked, change the wording slightly.
			if (!bIsLabRow) {
				pMenu.InsertMenu(nPosition++, MF_BYPOSITION, 1, "Move Lab Result");
			} else {
				pMenu.InsertMenu(nPosition++, MF_BYPOSITION, 1, "Move Lab Results");
			}

			//TES 10/14/2008 - PLID 31637 - Pull the message from the linked HL7MessageQueueT (if any).
			// (r.goldschmidt 2015-11-02 15:47) - PLID 66437 - Update everywhere in Practice that displays HL7 messages to handle purged messages
			// Don't bother adding an option to view HL7 messages, if they're not licensed for HL7
			long nResultID = VarLong(pRow->GetValue(lrtcResultID), -1);
			_RecordsetPtr rsMessage = NULL;
			if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
			{
				rsMessage = CreateRecordset("SELECT HL7MessageQueueT.ID "
					"FROM HL7MessageQueueT INNER JOIN LabResultsT ON HL7MessageQueueT.ID = LabResultsT.HL7MessageID "
					"WHERE LabResultsT.ResultID = %li", nResultID);
				if (!rsMessage->eof) {
					//TES 10/14/2008 - PLID 31637 - There is a message to view, so give them the option to view it.
					pMenu.InsertMenu(nPosition++, MF_BYPOSITION, 2, "View Original HL7 Message");
				}
			}

			CPoint pt;
			GetCursorPos(&pt);
			int nReturn = pMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
			switch (nReturn) {
			case 1:
			{
				// (r.gonet 06/02/2014) - PLID 40426 - Move Lab Result(s)
				// We can't move a lab result record without permission to write
				if (!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic4)) {
					return;
				}

				// we are going to force a save, we cannot have notes added to a lab and then the user hitting cancel
				if (!m_pLabEntryDlg->Save()) {
					return;
				}

				// (r.gonet 06/02/2014) - PLID 40426 - Grab the lab ID
				long nLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID), -1);

				CMovePatientLabsDlg dlg(this, m_nPatientID, nLabID, nResultID);
				if (IDOK == dlg.DoModal()) {
					if (GetInitialLabID() == -1) {
						// This entire requisition was unsaved. We need to have an initial lab order when loading existing
						SetInitialLabID(nLabID);
					}
					// Reload the lab tree
					LoadExisting();
				}
				break;
			}
			case 2:
			{
				//TES 10/14/2008 - PLID 31637 - They chose to view it.  Use CMsgBox so that the whole message will be 
				// visible no matter how big it is.
				// (z.manning 2011-07-08 16:27) - PLID 38753 - We now have a function to show this
				// (r.goldschmidt 2015-11-02 15:48) - PLID 66437 - Update everywhere in Practice that displays HL7 messages to handle purged messages
				ViewHL7ImportMessage(AdoFldLong(rsMessage, "ID"));
				break;
			}
			}
		} else {
			// This is a lab row and it doesn't have results, so don't show the context menu.
		}
		
	}NxCatchAll("Error in CLabResultsTabDlg::OnRButtonDownResultsTree()");
}

void CLabResultsTabDlg::OnFileDownloadPdfPreview(BOOL ActiveDocument, BOOL* Cancel)
{
	try {
		if(!ActiveDocument) {
			//TES 11/23/2009 - PLID 36192 - If we get here, that means its about to pop up a dialog to download the file.  We don't want 
			// that, so set the Cancel parameter to TRUE to cancel that.  We do want to go ahead and disable the browser, but if we do that
			// here, it won't actually work, we have to post a message to do it, we'll go ahead and just set a timer.
			SetTimer(IDT_DISABLE_BROWSER, 0, NULL);
			*Cancel = TRUE;
		}
	} NxCatchAll("Error in CLabResultsTabDlg::OnFileDownloadPdfPreview()");
}

void CLabResultsTabDlg::OnTimer(UINT_PTR nIDEvent)
{
	try {
		switch(nIDEvent) {
			case IDT_DISABLE_BROWSER:
				//TES 11/23/2009 - PLID 36192 - We've been asked to disable the browser, so go ahead and do that.
				KillTimer(IDT_DISABLE_BROWSER);
				// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
				//TES 1/16/2014 - PLID 59209 - If we get here, Adobe isn't installed properly, so explain that to the user, rather than leaving it blank.
				m_pBrowser->Navigate2(COleVariant("nxres://0/AdobeWarning.html"), COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
				GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(FALSE);
				//TES 2/4/2010 - PLID 37191 - Note that we are NOT disabling the Zoom button.  If we get here, that means that
				// the attached file couldn't be previewed, but that's no reason to prevent them from opening it.
				break;
		}
	}NxCatchAll("Error in CLabResultsTabDlg::OnTimer()");
}

void CLabResultsTabDlg::HandleSpecimenChange(long nLabID, const CString &strNewSpecimen)
{
	//TES 11/30/2009 - PLID 36452 - One of the labs' specimens has changed, update our tree.
	IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow();
	while(pLabRow) {
		long nID = VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1);
		if(nID == nLabID) {
			//TES 11/30/2009 - PLID 36452 - Found it!  Calculate the new name.
			CString strNewName = m_pLabEntryDlg->GetFormNumber();
			if(!strNewSpecimen.IsEmpty()) {
				strNewName += " - " + strNewSpecimen;
			}
			pLabRow->PutValue(lrtcValue, _bstr_t(strNewName));
			//TES 11/30/2009 - PLID 36452 - Remember the specimen field separately, in case the form number changes.
			pLabRow->PutValue(lrtcSpecimen, _bstr_t(strNewSpecimen));
			return;
		}
		pLabRow = pLabRow->GetNextRow();
	}
}

// (c.haag 2010-11-18 17:35) - PLID 37372 - Returns the current lab ID
long CLabResultsTabDlg::GetCurrentLab() const
{
	//TES 11/30/2009 - PLID 36452 - Select a row corresponding to the given lab ID, if one isn't selected already.
	IRowSettingsPtr pCurSel = m_pResultsTree->CurSel;
	//TES 11/30/2009 - PLID 36452 - What's selected now?
	long nCurLabID = -2;
	if(pCurSel) {
		IRowSettingsPtr pLabRow = GetLabRow(pCurSel);
		nCurLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	return nCurLabID;
}

void CLabResultsTabDlg::SetCurrentLab(long nLabID)
{
	//TES 11/30/2009 - PLID 36452 - Select a row corresponding to the given lab ID, if one isn't selected already.
	IRowSettingsPtr pCurSel = m_pResultsTree->CurSel;
	//TES 11/30/2009 - PLID 36452 - What's selected now?
	long nCurLabID = -2;
	if(pCurSel) {
		IRowSettingsPtr pLabRow = GetLabRow(pCurSel);
		nCurLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	if(nCurLabID != nLabID) {
		//TES 11/30/2009 - PLID 36452 - They differ, so find the row that corresponds to this ID (note that there can only ever be one with
		// an ID of -1), and select its first result, or the top-level row if it has no results.
		IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow();
		while(pLabRow) {
			if(VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1) == nLabID) {
				if(pLabRow->GetFirstChildRow()) {
					m_pResultsTree->CurSel = pLabRow->GetFirstChildRow();
				}
				else {
					m_pResultsTree->CurSel = pLabRow;
				}
				LoadResult(m_pResultsTree->CurSel);

				
				// (j.dinatale 2010-12-13) - PLID 41438 - only on the first time this is called, go ahead get the user's prefered view, 
				//		if its the pdf view and theres no files attached
				//		shift the view over to report view since the pdf view will be blank. If this is not the first time we set the view,
				//		go ahead and set it to the current view to work around the z-order issue
				// (k.messina 2011-11-16) - PLID 41438 - setting the default view to NexTech's report
				if(m_nCurrentView == rvNotSet){
					long nPreferredView = GetRemotePropertyInt("LabResultUserView", (long)rvPDF, 0, GetCurrentUserName(), true);

					if(nPreferredView == rvPDF && m_mapAttachedFiles.IsEmpty())
						nPreferredView = rvNexTechReport;

					SetCurrentResultsView(nPreferredView);
				}else{
					SetCurrentResultsView(m_nCurrentView);
				}

				return;
			}
			pLabRow = pLabRow->GetNextRow();
		}
	}
}

void CLabResultsTabDlg::HandleFormNumberChange(const CString &strFormNumber)
{
	//TES 11/30/2009 - PLID 36452 - The form number has changed, so update all of our lab rows with this number, and their stored Specimen.
	IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow();
	while(pLabRow) {
		CString strSpecimen = VarString(pLabRow->GetValue(lrtcSpecimen));
		if(strSpecimen.IsEmpty()) {
			pLabRow->PutValue(lrtcValue, _bstr_t(strFormNumber));
		}
		else {
			pLabRow->PutValue(lrtcValue, _bstr_t(strFormNumber + " - " + strSpecimen));
		}
		pLabRow = pLabRow->GetNextRow();
	}
}

IRowSettingsPtr CLabResultsTabDlg::GetResultRow(IRowSettingsPtr pRow)
{
	//TES 11/30/2009 - PLID 36452 - Get the result-type row that matches this one (will be NULL if pRow is a top-level row).
	if(!pRow) {
		return NULL;
	}
	else {
		long nRowType = VarLong(pRow->GetValue(lrtcFieldID));
		if(nRowType == (long)lrfLabName) {
			return NULL;
		}
		else if(nRowType == (long)lrfName) {
			return pRow;
		}
		else {
			return pRow->GetParentRow();
		}
	}
}

IRowSettingsPtr CLabResultsTabDlg::GetLabRow(IRowSettingsPtr pRow) const
{
	//TES 11/30/2009 - PLID 36452 - Get the top-level parent for this row.
	if(!pRow) {
		return NULL;
	}
	else {
		long nRowType = VarLong(pRow->GetValue(lrtcFieldID));
		if(nRowType == (long)lrfLabName) {
			return pRow;
		}
		else if(nRowType == (long)lrfName) {
			return pRow->GetParentRow();
		}
		else {
			return pRow->GetParentRow()->GetParentRow();
		}
	}
}

// (c.haag 2010-11-18 16:08) - PLID 37372 - Returns the lab row given a lab ID
IRowSettingsPtr CLabResultsTabDlg::GetLabRowByID(const long nLabID) const
{
	IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow();
	while (pLabRow) 
	{
		if (VarLong(pLabRow->GetValue(lrtcForeignKeyID),-1) == nLabID) 
		{
			return pLabRow;
		}
		pLabRow = pLabRow->GetNextRow();
	}
	// The lab doesn't exist in the tree!
	return NULL;
}

//TES 1/27/2010 - PLID 36862 - Remember that the given file is attached to the given result.
void CLabResultsTabDlg::SetAttachedFile(NXDATALIST2Lib::IRowSettingsPtr pResultRow, CString strFile)
{
	//TES 1/27/2010 - PLID 36862 - Look up the result map in our requisition map.
	CMap<LPDISPATCH,LPDISPATCH,CString,CString&> *pMap = NULL;
	if(!m_mapAttachedFiles.Lookup((LPDISPATCH)pResultRow->GetParentRow(), pMap)) {
		//TES 1/27/2010 - PLID 36862 - It wasn't there, so we'll need a new one.
		pMap = new CMap<LPDISPATCH,LPDISPATCH,CString,CString&>;
	}
	//TES 1/27/2010 - PLID 36862 - Now, set the attached file in the result map.
	pMap->SetAt((LPDISPATCH)pResultRow, strFile);
	//TES 1/27/2010 - PLID 36862 - And put it back into the requisition map.
	m_mapAttachedFiles.SetAt((LPDISPATCH)pResultRow->GetParentRow(), pMap);
}

//TES 1/27/2010 - PLID 36862 - Get all result/file pairs for the requisition corresponding to the given result.
BOOL CLabResultsTabDlg::GetAttachedFiles(NXDATALIST2Lib::IRowSettingsPtr pResultRow, OUT CMap<LPDISPATCH,LPDISPATCH,CString,CString&>* &pMap)
{
	//TES 1/27/2010 - PLID 36862 - Just look it up in our map
	return m_mapAttachedFiles.Lookup((LPDISPATCH)pResultRow->GetParentRow(), pMap);
}

void CLabResultsTabDlg::OnZoom()
{
	try 
	{
		// (c.haag 2011-02-22) - PLID 41618 - Special handling when in the attachments view
		if (m_nCurrentView == rvPDF) 
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnZoom();
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnZoom without a valid attachment view!");
			}		
		}
		else 
		{
			ZoomAttachment(m_strCurrentFileName);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for zooming. I also added the MsgBox
// call when I noted that OpenLabDoc had it when I deprecated it.
void CLabResultsTabDlg::ZoomAttachment(const CString& strFilename)
{
	//TES 2/3/2010 - PLID 37191 - Just open up whatever file we're currently previewing.
	if (!strFilename.IsEmpty())
	{
		if (!OpenDocument(strFilename, m_nPatientID)) {
			MsgBox(RCS(IDS_NO_FILE_OPEN));
		}
	}
}

// (z.manning 2010-04-29 12:41) - PLID 38420
void CLabResultsTabDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);

		SetControlPositions();

		//TES 2/7/2012 - PLID 47996 - If we're in the Report View, reload our HTML, which is based on the size of the window.
		if(IsDlgButtonChecked(IDC_REPORTVIEW_RADIO)) {
			//TES 2/7/2012 - PLID 47996 - Find the currently selected specimen row.
			IRowSettingsPtr pRow = m_pResultsTree->CurSel;
			NXDATALIST2Lib::IRowSettingsPtr pSpecRow = NULL;
			if (pRow) {
				pSpecRow = GetSpecFromCurrentRow(pRow);
				if (pSpecRow) {
					//TES 2/7/2012 - PLID 47996 - Reload the HTML
					LoadHTMLReport(pSpecRow);
				}
			}
		}

	
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-12 10:42) - PLID 37400 - Returns a new row for the result tree with all values initialized to null
IRowSettingsPtr CLabResultsTabDlg::GetNewResultsTreeRow()
{
	IRowSettingsPtr pNewRow = m_pResultsTree->GetNewRow();
	short nColCount = m_pResultsTree->GetColumnCount();
	for(int nCol = 0; nCol < nColCount; nCol++) {
		pNewRow->PutValue(nCol, g_cvarNull);
	}
	return pNewRow;
}

// (z.manning 2010-05-12 11:08) - PLID 37400
void CLabResultsTabDlg::EnableResultWnd(BOOL bEnable, CWnd *pwnd)
{
	// (z.manning 2010-05-12 11:13) - PLID 37400 - This function will enable or disable the given
	// CWnd except for edit controls in which case it will make them read-only.
	BOOL bIsEdit = pwnd->IsKindOf(RUNTIME_CLASS(CEdit));
	if(bEnable) {
		pwnd->EnableWindow(TRUE);
		if(bIsEdit) {
			((CEdit*)pwnd)->SetReadOnly(FALSE);
		}
	}
	else {
		if(bIsEdit) {
			pwnd->EnableWindow(TRUE);
			((CEdit*)pwnd)->SetReadOnly(TRUE);
		}
		else {
			pwnd->EnableWindow(FALSE);
		}
	}
}

// (z.manning 2010-05-13 11:35) - PLID 37405 - Returns the currently active lab req dialog
CLabRequisitionDlg* CLabResultsTabDlg::GetActiveLabRequisitionDlg()
{
	IRowSettingsPtr pLabRow = GetLabRow(m_pResultsTree->GetCurSel());
	if(pLabRow == NULL) {
		return NULL;
	}

	const long nLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID), -1);
	return m_pLabEntryDlg->GetLabRequisitionDlgByID(nLabID);
}

// (z.manning 2010-11-09 17:17) - PLID 41395
// (c.haag 2010-12-06 15:49) - PLID 41618 - Also get the MailID and result row
void CLabResultsTabDlg::GetAllAttachedFiles(OUT CArray<CAttachedLabFile,CAttachedLabFile&> &aAttachedFiles)
{
	aAttachedFiles.RemoveAll();
	POSITION posMain = m_mapAttachedFiles.GetStartPosition();
	while(posMain != NULL)
	{
		LPDISPATCH lpKey;
		CMap<LPDISPATCH,LPDISPATCH,CString,CString&>* pmapFiles;
		m_mapAttachedFiles.GetNextAssoc(posMain, lpKey, pmapFiles);

		POSITION posTemp = pmapFiles->GetStartPosition();
		while(posTemp != NULL)
		{
			CString strFile;
			pmapFiles->GetNextAssoc(posTemp, lpKey, strFile);
			if(!strFile.IsEmpty()) {
				CAttachedLabFile alf;
				IRowSettingsPtr pResultRow(lpKey);
				alf.lpRow = lpKey;
				alf.nMailID = VarLong(GetTreeValue(pResultRow, lrfValue, lrtcForeignKeyID),-1);
				alf.strFileName = strFile;
				alf.dtAttached = VarDateTime(GetTreeValue(pResultRow, lrfValue, lrtcDate));
				aAttachedFiles.Add(alf);
			}
		}
	}
}

// (c.haag 2010-12-28 13:41) - PLID 41618 - Returns all attached files sorted by MailID descending.
// The first entry in this array is the oldest attachment.
void CLabResultsTabDlg::GetAllAttachedFilesSorted(OUT CArray<CAttachedLabFile,CAttachedLabFile&> &aAttachedFiles)
{
	GetAllAttachedFiles(aAttachedFiles);
	// Sort the attached files by MailID ascending. A value of -1 will be placed at the end because it is technically the
	// most recent attachment.
	qsort(aAttachedFiles.GetData(),aAttachedFiles.GetSize(),sizeof(CAttachedLabFile),CompareAttachedLabFile);
}

// (z.manning 2010-11-09 15:05) - PLID 41395 - This function will display the only attached file in the
// entire lab (regardless of specimen) if there's exactly one file attached.
void CLabResultsTabDlg::TryDisplayOldestAttachedFile()
{
	// (c.haag 2011-02-23) - PLID 41618 - Changed the array type
	CArray<CAttachedLabFile,CAttachedLabFile&> aAttachedFiles;
	GetAllAttachedFiles(aAttachedFiles);
	if(aAttachedFiles.GetSize() == 1)
	{
		CString strFileName = aAttachedFiles.GetAt(0).strFileName;
		if(strFileName.Find("\\") == -1) {
			strFileName = GetPatientDocumentPath(m_nPatientID) ^ strFileName;
		}

		// (z.manning 2010-11-09 17:28) - PLID 41395 - No need to do this if it's the same file.
		if(strFileName != m_strCurrentFileName)
		{
			//TES 2/3/2010 - PLID 37191 - Remember the attached file.
			m_strCurrentFileName = strFileName;
			//TES 11/23/2009 - PLID 36192 - If this is a .pdf file, we want to append #toolbar=0, to hide the .pdf controls.
			if(strFileName.Right(4).CompareNoCase(".pdf") == 0) {
				strFileName += "#toolbar=0";
			}
			COleVariant varUrl(strFileName);
			// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
			m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
			GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE);

			//TES 2/3/2010 - PLID 37191 - Enable the Zoom button.
			// (c.haag 2011-02-23) - PLID 41618 - Use UpdateZoomButton
			UpdateZoomButton();
		}
	}
	else
	{
		//TES 11/23/2009 - PLID 36192 - Nope, set our preview control to blank and disabled.
		COleVariant varUrl("about:blank");
		// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
		m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
		GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(FALSE);
		//TES 2/3/2010 - PLID 37191 - Remember that nothing's attached.
		m_strCurrentFileName = "";
		//TES 2/3/2010 - PLID 37191 - Make sure the Zoom button is disabled.
		// (c.haag 2011-02-23) - PLID 41618 - Use UpdateZoomButton
		UpdateZoomButton();
	}
}

// (c.haag 2011-02-22) - PLID 41618 - This function should be called when changing to
// either the report or discrete values view. The attachments view has its own internal
// way of handling these, and this function should never be called when the attachments
// view is visible.
void CLabResultsTabDlg::RefreshSharedControls(IRowSettingsPtr pActiveRow)
{
	if (m_nCurrentView == rvPDF) 
	{
		ThrowNxException("CLabResultsTabDlg::RefreshSharedControls was called from the attachments view!"); 
	}

	// 1. Update the result label
	m_nxstaticLabResultLabel.SetWindowText("Results");
	// 2. Update the detach/attach button
	UpdateDetachButton();
	// 3. Update the scroll buttons
	UpdateScrollButtons();
	// 4. Update the zoom button		
	UpdateZoomButton();
	// 5. Update the notes buttons
	// (j.dinatale 2010-12-27) - PLID 41591 - format the buttons accordingly
	FormatNotesButtons(pActiveRow);
	// 6. Update the acknowledged button text
	FormatAcknowledgedButtonText(pActiveRow);
	// 7. Update the signature button text
	FormatSignButtonText(pActiveRow);
	// 8. Update the mark completed button text
	FormatMarkCompletedButtonText(pActiveRow);
	FormatMarkAllCompleteButtonText(pActiveRow); // (j.luckoski 3-21-13) PLID 55424 - Format button
	FormatAcknowledgeAndSignButtonText(pActiveRow); // (f.gelderloos 2013-08-28 16:07) - PLID 57826
}

// (k.messina 2011-11-11) - PLID 41438 - utilizes enumeration to select view from within code
void CLabResultsTabDlg::SetCurrentResultsView(int nResultsView)
{
	switch(nResultsView)
	{
	case rvPDF :
		SetUpPDFView();
		break;

	case rvNexTechReport :
		SetUpReportView();
		break;

	case rvDiscreteValues :
		SetUpDiscreteView();
		break;
	}
}

// (j.dinatale 2010-12-14) - PLID 41438 - Utility function for PDF View
void CLabResultsTabDlg::SetUpPDFView()
{
	m_pLabResultsAttachmentView->Setup();
}

// (k.messina 2011-11-11) - PLID 41438 - PDF view has been selected
void CLabResultsTabDlg::OnBnClickedPdfviewRadio()
{
	try{	
		// (c.haag 2011-02-25) - PLID 41618 - Do nothing if we're already in the view
		if (m_nCurrentView != rvPDF) 
		{
			// (c.haag 2011-02-25) - PLID 41618 - Try to save the result first so the
			// view is updated
			SaveResult(m_pResultsTree->CurSel);
			// (j.dinatale 2010-12-14) - PLID 41438 - call the utility function for pdf view
			SetUpPDFView();
		}
	} NxCatchAll("Error in CLabResultsTabDlg::OnBnClickedPdfviewRadio");
}

// (j.dinatale 2010-12-14) - PLID 41438 - Utility function for setting up the report view
void CLabResultsTabDlg::SetUpReportView()
{
	try{
	// (j.gruber 2010-11-30 09:14) - PLID 41606 - unload any previous reports
		UnloadHTMLReports();

		//check the radio button if it isn't checked off
		if(!((CButton *)GetDlgItem(IDC_REPORTVIEW_RADIO))->GetCheck())
			SetDlgItemCheck(IDC_REPORTVIEW_RADIO, TRUE);

		// (j.dinatale 2010-12-14) - PLID 41438 - ensure that the other radio buttons arent in their checked states
		if(((CButton *)GetDlgItem(IDC_DISCRETEVALUES_RADIO))->GetCheck())
			SetDlgItemCheck(IDC_DISCRETEVALUES_RADIO, FALSE);

		if(((CButton *)GetDlgItem(IDC_REPORTVIEW_RADIO))->GetCheck())
			SetDlgItemCheck(IDC_PDFVIEW_RADIO, FALSE);

		//hide everything
		// (c.haag 2011-02-22) - PLID 42589 - Pass in the current tree selection
		UpdateControlStates(true, m_pResultsTree->CurSel);

		// (j.gruber 2010-11-24 11:42) - PLID 41607 
		RefreshHTMLReportOnlyControls(TRUE);

		//enlarge the webviewer
		// (c.haag 2010-11-23 11:19) - PLID 37372 - Make it a little shorter		
		// (a.walling 2011-04-29 08:26) - PLID 43501 - Don't mess with z-order or copying client bits
		SetSingleControlPos(IDC_PDF_PREVIEW, NULL, 5, 91, 945, 350, SWP_SHOWWINDOW|SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_FRAMECHANGED);		

		// (j.dinatale 2010-12-13) - PLID 41438 - No need to remember, the user will be able to setup their default view
		//set the ConfigRT to store the user's last view
		//SetRemotePropertyInt("LabResultUserView", (long)rvPDF, 0, GetCurrentUserName());

		// (j.gruber 2010-11-30 09:14) - PLID 41606 - run the report
		//get the currently selected Specimen
		IRowSettingsPtr pRow = m_pResultsTree->CurSel;		

		NXDATALIST2Lib::IRowSettingsPtr pSpecRow = NULL;
		if (pRow) {
			//save the result to update the tree
			SaveResult(pRow);
			pSpecRow = GetSpecFromCurrentRow(pRow);
		}
		else {
			//are there any rows
			pRow = m_pResultsTree->GetFirstRow();
			if (pRow) {				
				//set the current sel
				m_pResultsTree->CurSel = pRow;
				// (j.gruber 2011-02-21 10:00) - PLID 41606 - some fix ups for the buttons
				//save the result to update the tree
				//we don't need to save since we are selecting it
				//SaveResult(pRow);
				//we do need to load though
				LoadResult(pRow);
				pSpecRow = GetSpecFromCurrentRow(pRow);
			}
		}

		if (pSpecRow) {
			LoadHTMLReport(pSpecRow);
		}

		// (j.dinatale 2010-12-14) - PLID 41438 - until the z-order issue is fixed, we need to maintain the current view
		m_nCurrentView = rvNexTechReport;

		// (c.haag 2011-02-22) - PLID 41618 - The PDF view shares controls with the other views. We need to update
		// those controls now.
		RefreshSharedControls(pSpecRow);
	}
	NxCatchAll(__FUNCTION__);
}

// (k.messina 2011-11-11) - PLID 41438 - report view has been selected
void CLabResultsTabDlg::OnBnClickedReportviewRadio()
{
	try{
		// (j.dinatale 2010-12-14) - PLID 41438 - call the utility function for the report view
		SetUpReportView();
	} NxCatchAll("Error in CLabResultsTabDlg::OnBnClickedReportviewRadio");
}

// (j.dinatale 2010-12-14) - PLID 41438 - Utility function to set up the discrete values view
void CLabResultsTabDlg::SetUpDiscreteView()
{
	try{
		// (j.gruber 2010-11-30 09:14) - PLID 41606 - unload any previous reports
		UnloadHTMLReports();

		//check the radio button if it isn't checked off
		if(!((CButton *)GetDlgItem(IDC_DISCRETEVALUES_RADIO))->GetCheck())
			SetDlgItemCheck(IDC_DISCRETEVALUES_RADIO, TRUE);

		// (j.dinatale 2010-12-14) - PLID 41438 - ensure that the other radio buttons arent in their checked states
		if(((CButton *)GetDlgItem(IDC_PDFVIEW_RADIO))->GetCheck())
			SetDlgItemCheck(IDC_PDFVIEW_RADIO, FALSE);

		if(((CButton *)GetDlgItem(IDC_REPORTVIEW_RADIO))->GetCheck())
			SetDlgItemCheck(IDC_REPORTVIEW_RADIO, FALSE);

		//first show all the controls because the other views hide them
		// (c.haag 2011-02-22) - PLID 42589 - Pass in the current tree selection
		UpdateControlStates(false, m_pResultsTree->CurSel);

		// (j.gruber 2010-11-24 11:42) - PLID 41607 - needed for HTML Report controls
		RefreshHTMLReportOnlyControls(FALSE);

		//shrik the web viewer
		// (a.walling 2011-04-29 08:26) - PLID 43501 - Don't mess with z-order or copying client bits
		SetSingleControlPos(IDC_PDF_PREVIEW, NULL, 302, 90, 644, 145, SWP_SHOWWINDOW|SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_FRAMECHANGED);		

		//load the results for our current selection
		//LoadResult(m_pResultsTree->CurSel);
		// (j.gruber 2010-11-30 09:14) - PLID 41606
		ReloadCurrentPDF();

		// (j.dinatale 2010-12-13) - PLID 41438 - No need to remember, the user will be able to setup their default view
		//set the ConfigRT to store the user's last view
		//SetRemotePropertyInt("LabResultUserView", (long)rvPDF, 0, GetCurrentUserName());

		// (j.dinatale 2010-12-14) - PLID 41438 - until the z-order issue is fixed, we need to maintain the current view
		m_nCurrentView = rvDiscreteValues;

		// (c.haag 2011-02-22) - PLID 41618 - The PDF view shares controls with the other views. We need to update
		// those controls now.
		RefreshSharedControls(m_pResultsTree->CurSel);

	}NxCatchAll(__FUNCTION__);
}

// (k.messina 2011-11-11) - PLID 41438 - descrete values view has been selected
void CLabResultsTabDlg::OnBnClickedDiscretevaluesRadio()
{
	try{
		// (j.dinatale 2010-12-14) - PLID 41438 - call the utility function to set up discrete values
		SetUpDiscreteView();
	} NxCatchAll("Error in CLabResultsTabDlg::OnBnClickedDiscretevaluesRadio");
}

// (k.messina 2011-11-11) - PLID 41438 - consolidating the logic to hide controls
// (c.haag 2011-02-22) - PLID 42589 - Renamed to UpdateControlStates and we now take in the current selection
void CLabResultsTabDlg::UpdateControlStates(bool bHideControls, IRowSettingsPtr pCurTreeSel)
{
	//assume we are hiding the controls for preparation of expanding the web browser
	int nToggleHide = SW_HIDE;

	//check if we are not hiding them
	if(!bHideControls)
		nToggleHide = SW_SHOW;

	// (c.haag 2010-12-06 13:37) - PLID 41618
	BOOL bInReportView = ((CButton *)GetDlgItem(IDC_REPORTVIEW_RADIO))->GetCheck();
	BOOL bInPDFView = ((CButton *)GetDlgItem(IDC_PDFVIEW_RADIO))->GetCheck();

	// (c.haag 2010-12-06 13:37) - PLID 42589 - Update control visibilities and enabled states. This used
	// to be done in LoadResult but now LoadResult only populates textand some label content.
	if(pCurTreeSel == NULL || pCurTreeSel->GetValue(lrtcResultID).vt == VT_NULL) 
	{
		GetDlgItem(IDC_RESULT_NAME)->EnableWindow(FALSE);		
		GetDlgItem(IDC_RESULT_LOINC)->EnableWindow(FALSE);
		// (a.walling 2010-01-18 11:18) - PLID 36936			
		m_nxlabelLOINC.SetType(dtsDisabledHyperlink);
		m_nxlabelLOINC.AskParentToRedrawWindow();
		m_dtpDateReceived.EnableWindow(FALSE);
		GetDlgItem(IDC_SLIDE_NUMBER)->EnableWindow(FALSE);
		if(m_ltType != ltBiopsy) { // (r.galicki 2008-10-17 14:50) - PLID 31552
			GetDlgItem(IDC_SLIDE_NUMBER_LABEL)->ShowWindow(FALSE);
			GetDlgItem(IDC_SLIDE_NUMBER)->ShowWindow(FALSE);
		}
		else {
			// (c.haag 2010-12-29 09:18) - Don't show the controls if they're supposed to be hidden
			GetDlgItem(IDC_SLIDE_NUMBER_LABEL)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_SLIDE_NUMBER)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
		}
		GetDlgItem(IDC_LAB_DIAG_DESCRIPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->EnableWindow(FALSE);
		if(m_ltType != ltBiopsy) { // (r.galicki 2008-10-17 14:55) - PLID 31552
			GetDlgItem(IDC_MICRO_DESC_LABEL)->ShowWindow(FALSE);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->ShowWindow(FALSE);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->ShowWindow(FALSE);
			// (b.spivey, August 27, 2013) - PLID 46295 - These fields only apply for biopsies. 
			m_nxstaticAnatomicalLocationTextDiscrete.ShowWindow(FALSE); 
			m_nxstaticAnatomicalLocationTextReport.ShowWindow(FALSE); 
		}
		else {
			// (c.haag 2010-12-29 09:18) - Don't show the controls if they're supposed to be hidden
			GetDlgItem(IDC_MICRO_DESC_LABEL)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			// (b.spivey, August 27, 2013) - PLID 46295 - Show anatomical location on the discrete/report views.
			// (b.spivey, September 05, 2013) - PLID 46295 - hide the discrete text label if this is a parent row. 
			m_nxstaticAnatomicalLocationTextDiscrete.ShowWindow(SW_HIDE);
			m_nxstaticAnatomicalLocationTextReport.ShowWindow(bHideControls ? SW_SHOW : SW_HIDE);
		}
		GetDlgItem(IDC_RESULT_REFERENCE)->EnableWindow(FALSE);
		m_nxeditResultComments.EnableWindow(FALSE);
		GetDlgItem(IDC_RESULT_VALUE)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
		GetDlgItem(IDC_RESULT_VALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RESULT_UNITS)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
		GetDlgItem(IDC_RESULT_UNITS)->EnableWindow(FALSE);
		m_nxstaticUnitsLabel.ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
		GetDlgItem(IDC_LAB_FLAG)->EnableWindow(FALSE);
		GetDlgItem(IDC_LAB_STATUS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ADD_DIAGNOSIS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_LAB_DIAGNOSES)->EnableWindow(FALSE);
		GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RESULT_STATUS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FLAG)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE_RESULT)->EnableWindow(FALSE);
		//TES 11/30/2009 - PLID 36452 - If we have nothing selected, we can't add a result because we won't know what lab to add it to.
		GetDlgItem(IDC_ADD_RESULT)->EnableWindow(!(pCurTreeSel == NULL));
	}
	else {
		IRowSettingsPtr pParent = GetResultRow(pCurTreeSel);
		// (z.manning 2010-05-12 10:53) - PLID 37400 - We now disable the editing of HL7 imported results
		// unless they have a permission.
		long nHL7MessageID = VarLong(pParent->GetValue(lrtcHL7MessageID), -1);
		BOOL bEnableResults = TRUE;
		if(nHL7MessageID != -1 && !CheckCurrentUserPermissions(bioPatientLabs, sptDynamic1, FALSE, 0, TRUE)) {
			bEnableResults = FALSE;
		}
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_RESULT_NAME));
		EnableResultWnd(bEnableResults, &m_dtpDateReceived);
		// (a.walling 2010-01-18 10:40) - PLID 36936
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_RESULT_LOINC));

		// (r.galicki 2008-10-21 13:04) - PLID 31552 - Handle hidden columns
		if(m_ltType != ltBiopsy) { // (r.galicki 2008-10-17 14:50) - PLID 31552
			GetDlgItem(IDC_SLIDE_NUMBER_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_SLIDE_NUMBER_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SLIDE_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SLIDE_NUMBER)->ShowWindow(SW_HIDE);
		}
		else {
			// (c.haag 2010-12-29 09:18) - Don't show the controls if they're supposed to be hidden
			GetDlgItem(IDC_SLIDE_NUMBER_LABEL)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_SLIDE_NUMBER)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			EnableResultWnd(bEnableResults, GetDlgItem(IDC_SLIDE_NUMBER));
		}
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_LAB_DIAG_DESCRIPTION));

		if(m_ltType != ltBiopsy) { // (r.galicki 2008-10-17 14:50) - PLID 31552
			GetDlgItem(IDC_MICRO_DESC_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MICRO_DESC_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->EnableWindow(FALSE);
			// (b.spivey, August 27, 2013) - PLID 46295 - These fields only apply for biopsies. 
			m_nxstaticAnatomicalLocationTextDiscrete.ShowWindow(SW_HIDE); 
			m_nxstaticAnatomicalLocationTextReport.ShowWindow(SW_HIDE); 
		}
		else {
			GetDlgItem(IDC_MICRO_DESC_LABEL)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			EnableResultWnd(bEnableResults, GetDlgItem(IDC_CLINICAL_DIAGNOSIS_DESC));
			// (b.spivey, August 27, 2013) - PLID 46295 - Show anatomical location on the discrete/report views.
			m_nxstaticAnatomicalLocationTextDiscrete.ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			m_nxstaticAnatomicalLocationTextReport.ShowWindow(bHideControls ? SW_SHOW : SW_HIDE);
		}

		long nMailID = VarLong(GetTreeValue(pCurTreeSel, lrfValue, lrtcForeignKeyID),-1);
		if (nMailID != -1) {
			//Hide the value field and show the document link
			GetDlgItem(IDC_RESULT_VALUE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RESULT_UNITS)->ShowWindow(SW_HIDE); // (c.haag 2009-05-07 09:18) - PLID 33789
			m_nxstaticUnitsLabel.ShowWindow(SW_HIDE);
		}
		else {
			EnableResultWnd(bEnableResults, GetDlgItem(IDC_RESULT_VALUE));
			GetDlgItem(IDC_RESULT_VALUE)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
			GetDlgItem(IDC_RESULT_UNITS)->ShowWindow(bHideControls ? SW_HIDE : SW_SHOW); // (c.haag 2009-05-07 09:18) - PLID 33789
			m_nxstaticUnitsLabel.ShowWindow(bHideControls ? SW_HIDE : SW_SHOW);
		}

		EnableResultWnd(bEnableResults, GetDlgItem(IDC_RESULT_UNITS));
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_RESULT_REFERENCE));

		// (z.manning 2009-04-30 16:59) - PLID 28560 - Added result comments
		m_nxeditResultComments.EnableWindow(TRUE);
		// (j.jones 2010-05-27 15:26) - PLID 38863 - you now need special permission to edit this field
		if(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic2) {
			m_nxeditResultComments.SetReadOnly(FALSE);
		}
		else {
			m_nxeditResultComments.SetReadOnly(TRUE);
		}
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_LAB_FLAG));
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_LAB_STATUS));
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_BTN_ADD_DIAGNOSIS));
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_CLINICAL_DIAGNOSIS_OPTIONS_LIST));
		EnableResultWnd(bEnableResults, GetDlgItem(IDC_DELETE_RESULT));
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		if(bEnableResults) {
			m_nxlabelLOINC.SetType(dtsHyperlink);

			// (j.jones 2013-11-08 15:30) - PLID 58979 - enable Pt. Education
			m_btnPatientEducation.EnableWindow(TRUE);
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - If we are showing it as a link, fine, but otherwise it is a label (or hidden elsewhere)
			m_nxlabelPatientEducation.SetType(bShowPatientEducationLink ? dtsHyperlink : dtsText);
			m_nxlabelPatientEducation.AskParentToRedrawWindow();
		}
		else {
			m_nxlabelLOINC.SetType(dtsDisabledHyperlink);

			// (j.jones 2013-11-08 15:30) - PLID 58979 - disable Pt. Education
			m_btnPatientEducation.EnableWindow(FALSE);
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - If we are showing it as a link, fine, but otherwise it is a label (or hidden elsewhere)
			m_nxlabelPatientEducation.SetType(bShowPatientEducationLink ? dtsDisabledHyperlink : dtsDisabledText);
			m_nxlabelPatientEducation.AskParentToRedrawWindow();
		}

		// (z.manning 2010-05-12 11:35) - PLID 37400 - These controls are all still enabled on read-only lab results
		GetDlgItem(IDC_EDIT_LAB_DIAGNOSES)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_CLINICAL_DIAGNOSIS_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RESULT_STATUS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_FLAG)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADD_RESULT)->EnableWindow(TRUE);
	}

	// (c.haag 2010-12-06 13:37) - PLID 41618 - Just call UpdateDetachButton
	UpdateDetachButton();

	// (c.haag 2010-12-06 15:49) - PLID 41618 - The result scroll buttons should be visible in the PDF view only
	GetDlgItem(IDC_RESULT_SCROLL_LEFT)->ShowWindow( (bInPDFView) ? SW_SHOW : SW_HIDE );
	GetDlgItem(IDC_RESULT_SCROLL_RIGHT)->ShowWindow( (bInPDFView) ? SW_SHOW : SW_HIDE );

	//set the state of each
	GetDlgItem(IDC_ADD_RESULT)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_DELETE_RESULT)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_RESULTS_TREE)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_ATTACHED_FILE_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_RESULT_NAME_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_RESULT_NAME)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_DATE_RECEIVED_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_DATE_RECEIVED)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_VALUE_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_RESULT_LOINC_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_RESULT_LOINC)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_DIAGNOSIS_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_BTN_ADD_DIAGNOSIS)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_BTN_ADD_DIAGNOSIS)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_BTN_ADD_DIAGNOSIS)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LAB_DIAG_DESCRIPTION)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_EDIT_LAB_DIAGNOSES)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_STATIC)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LAB_STATUS)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_EDIT_RESULT_STATUS)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_RESULT_COMMENTS_LABEL)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_RESULT_COMMENTS)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_FLAG_LABEL)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_LAB_FLAG)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_EDIT_FLAG)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_REFERENCE_LABEL)->ShowWindow(nToggleHide);
    GetDlgItem(IDC_RESULT_REFERENCE)->ShowWindow(nToggleHide);
	// (j.jones 2013-10-29 17:03) - PLID 58979 - added infobutton
	// (r.gonet 2014-01-27 15:29) - PLID 59339 - Consider the user's preferences to show or hide the patient education controls.
	bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
	bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
	GetDlgItem(IDC_PT_EDUCATION_LABEL)->ShowWindow((bShowPatientEducationLink || bShowPatientEducationButton) ? nToggleHide : SW_HIDE);
	GetDlgItem(IDC_BTN_PT_EDUCATION)->ShowWindow(bShowPatientEducationButton ? nToggleHide : SW_HIDE);	

	// (b.spivey, September 6, 2013) - PLID 46295 - hide if in pdf view. 
	if (bInPDFView) {
		m_nxstaticAnatomicalLocationTextDiscrete.ShowWindow(SW_HIDE);
		m_nxstaticAnatomicalLocationTextReport.ShowWindow(SW_HIDE);
	}

	// (c.haag 2010-11-22 12:29) - PLID 37372 - Toggle the background NxColor display
	GetDlgItem(IDC_LABRESULTS_NXCOLORCTRL_BACK)->ShowWindow((SW_SHOW == nToggleHide) ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_LABRESULTS_NXCOLORCTRL_LEFT)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LABRESULTS_NXCOLORCTRL_RIGHT)->ShowWindow(nToggleHide);
	// (c.haag 2010-11-22 12:29) - Actually all of these things should persist for every view; but if we
	// ever change our minds, these controls are here for posterity
	/*// (c.haag 2010-11-22 12:29) - PLID 37372 - And now new signature and NxColor controls
    GetDlgItem(IDC_ACKNOWLEDGED_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_SIGNED_BY_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LAB_SIGNED_BY)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_SIGNED_DATE_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LAB_SIGNED_DATE)->ShowWindow(nToggleHide);
	// (c.haag 2010-11-22 15:43) - PLID 40556 - Acknowledged controls
	GetDlgItem(IDC_LAB_ACKNOWLEDGE)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_ACKNOWLEDGED_BY_LABEL)->ShowWindow(nToggleHide);	
	// (c.haag 2010-12-02 16:33) - PLID 38633 - Completed controls
	GetDlgItem(IDC_LAB_MARK_COMPLETED)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_COMPLETED_BY_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LAB_COMPLETED_BY)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_COMPLETED_RESULT_DATE_LABEL)->ShowWindow(nToggleHide);
	GetDlgItem(IDC_LAB_RESULT_COMPLETED_DATE)->ShowWindow(nToggleHide);*/

	//TES 2/24/2012 - PLID 44841 - Added a button to configure the Report View, only show it if we're in that view
	GetDlgItem(IDC_CONFIGURE_REPORT_VIEW)->ShowWindow((nToggleHide == SW_SHOW || bInPDFView) ? SW_HIDE : SW_SHOW);

	// (c.haag 2011-12-28) - PLID 41618 - We need to track the hidden state so that LoadResult can reference it.
	// Since HideControls takes in a boolean instead of checking the radio buttons, we can't assume that the radio
	// buttons dictate this state.
	m_bControlsHidden = bHideControls;
}

// (c.haag 2010-12-02 09:46) - PLID 38633 - This function is called to mark individual results as completed
void CLabResultsTabDlg::OnLabMarkCompleted()
{
	try{
		//DRT 7/7/2006 - PLID 21088 - Check permission
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic0))
			return;

		// (c.haag 2011-02-22) - PLID 41618 - Special handling for the attachments view
		if (m_nCurrentView == rvPDF)
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnLabMarkCompleted();
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnLabMarkCompleted without a valid attachment view!");
			}		
		}
		else 
		{
			MarkLabCompleted(m_pResultsTree->CurSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2011-02-21) - PLID 41618 - Now takes in a row. pActiveRow can be a result or a specimen or null.
// (j.luckoski 2013-03-20 11:40) - PLID 55424- added bool in order to mark all or allow specification
void CLabResultsTabDlg::MarkLabCompleted(IRowSettingsPtr pActiveRow,bool bIsMarkAll /* =false */)
{
	long nLabID;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - We only want the results for the current specimen.
	GetResults(nLabID, arypResultRows, groCurrentSpecimen);
	BOOL bAllResultsCompleted = AllResultsAreCompleted(nLabID);
	CArray<IRowSettingsPtr,IRowSettingsPtr> apIncompleteResults; // Array of row pointers to incomplete results. We use the ordinals as ID's in a multi-selection if necessary.

	// Quit right away if there's nothing to mark complete. The button should be disabled at this juncture.
	if (0 == arypResultRows.GetCount()) {
		return;
	}
	// If all results are marked completed, there's nothing to do. The button should be disabled at this juncture.
	else if (bAllResultsCompleted) {
		return;
	}		
	else {
		GetIncompleteSignedResults(nLabID, apIncompleteResults);
	}

	// (c.haag 2010-12-02 09:51) - PLID 38633 - When we get here, apIncompleteResults is filled with all the
	// possible results that can be marked completed. Have the user decide which ones to complete if
	// necessary.
	if (apIncompleteResults.GetSize() == 0) 
	{
		AfxMessageBox("There are no results that qualify for being marked completed. A result must be signed before being marked completed. ", MB_ICONSTOP);
		return;
	}
	else if (apIncompleteResults.GetSize() > 1 && !bIsMarkAll) 
	{
		// Decision time
		// (j.armen 2012-06-08 09:32) - PLID 49607 - Input for this multiselect dlg isn't cacheable, so let's set a manual cache name
		CMultiSelectDlg dlg(this, "LabResultsT");
		CStringArray astrResults; // Array of result names
		dlg.m_bPreSelectAll = TRUE;
		for (int i=0; i < apIncompleteResults.GetSize(); i++) 
		{
			astrResults.Add(VarString(apIncompleteResults[i]->GetValue(lrtcValue), ""));
		}
		if (IDOK != dlg.OpenWithStringArray(astrResults, "Please select one or more results to mark completed", 1)) {
			return;
		} else {
			// Replace apIncompleteResults with only the selected 	
			CArray<IRowSettingsPtr,IRowSettingsPtr> apSelectedResults;
			CArray<long,long> arSelections;
			dlg.FillArrayWithIDs(arSelections);
			for (i=0; i < arSelections.GetSize(); i++) 
			{
				apSelectedResults.Add( apIncompleteResults[ arSelections[i] ] );
			}
			apIncompleteResults.Copy(apSelectedResults);
		}
	}
	else {
		// This means there's only one result to mark completed; no need to make the user choose it.
	}

	// (c.haag 2010-12-02 09:51) - PLID 38633 - Get the completed time
	COleDateTime dtCompleted;
	{
		_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS CurDateTime");
		if(!rs->eof){
			dtCompleted = AdoFldDateTime(rs, "CurDateTime");
		}
		else {
			dtCompleted = COleDateTime::GetCurrentTime();
		}
	}

	// (c.haag 2010-12-02 09:51) - PLID 38633 - Time to mark completed
	for (int i=0; i < apIncompleteResults.GetSize(); i++) 
	{
		IRowSettingsPtr pResultRow = apIncompleteResults[i];
		// Sanity check: If the result was already marked completed (which should never happen), do nothing.
		// Once a result is completed, the completed date must never change.
		if (-1 == VarLong(pResultRow->GetValue(lrtcCompletedBy), -1))
		{
			pResultRow->PutValue(lrtcCompletedBy, (long)GetCurrentUserID());
			pResultRow->PutValue(lrtcCompletedDate, _variant_t(dtCompleted, VT_DATE));
			pResultRow->PutValue(lrtcCompletedUsername, _bstr_t(GetCurrentUserName()));
		}
		else {
			// The result was already marked completed! We absolutely need to raise an error here.
			ThrowNxException("Attempted to mark an already completed result as complete!");
		}
	}

	// (c.haag 2010-12-02 10:28) - PLID 38633 - Update the mark completed button text
	FormatMarkCompletedButtonText(pActiveRow);
}

// (c.haag 2010-12-01 11:03) - PLID 37372 - Views a signature given a result row
void CLabResultsTabDlg::ViewSignature(IRowSettingsPtr& pResultRow)
{
	CString strSignatureFileName = VarString(pResultRow->GetValue(lrtcSignatureImageFile), "");
	_variant_t varSignatureInkData = pResultRow->GetValue(lrtcSignatureInkData);
	_variant_t varSignatureTextData = pResultRow->GetValue(lrtcSignatureTextData);

	// (z.manning 2008-10-15 15:08) - PLID 21082 - Now must sign the lab as well
	CSignatureDlg dlgSign(this);
	// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
	dlgSign.SetSignature(strSignatureFileName, varSignatureInkData, varSignatureTextData);
	dlgSign.m_bRequireSignature = TRUE;
	dlgSign.m_bReadOnly = TRUE;
	// (z.manning 2008-12-09 09:07) - PLID 32260 - Added a preference for checking for password when loading signature.
	dlgSign.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordLab", 1, 0, GetCurrentUserName()) == 1);
	dlgSign.DoModal();
}

// (c.haag 2010-12-01 11:03) - PLID 37372 - Populates apResults with all the result rows for a specified lab that have no signatures.
void CLabResultsTabDlg::GetUnsignedResults(long nLabID, CArray<IRowSettingsPtr,IRowSettingsPtr>& apResults)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	//TES 11/6/2012 - PLID 53591 - There's a preference now to sign/acknowledge all specimens
	if(bInPDFView || GetRemotePropertyInt("SignAndAcknowledgeResultsForAllSpecimens", 0, 0, "<None>")) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return;
		}
		arypLabRows.Add(pLabRow);
	}

	apResults.RemoveAll();
	
	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pChildRow = pLabRow->GetFirstChildRow();
		while (pChildRow) {
			if (pChildRow->GetValue(lrtcSignatureInkData).vt == VT_NULL) {
				apResults.Add(pChildRow);
			}
			pChildRow = pChildRow->GetNextRow();
		}
	}
}

// (z.manning 2011-06-22 09:43) - PLID 44154
void CLabResultsTabDlg::GetUnacknowledgedResults(long nLabID, CArray<IRowSettingsPtr,IRowSettingsPtr> &arypResults)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	//TES 11/6/2012 - PLID 53591 - There's a preference now to sign/acknowledge all specimens
	if(bInPDFView || GetRemotePropertyInt("SignAndAcknowledgeResultsForAllSpecimens", 0, 0, "<None>")) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return;
		}
		arypLabRows.Add(pLabRow);
	}

	arypResults.RemoveAll();
	
	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pChildRow = pLabRow->GetFirstChildRow();
		while (pChildRow) {
			if (GetTreeValue(pChildRow, lrfAcknowledgedBy, lrtcForeignKeyID).vt == VT_NULL) {
				arypResults.Add(pChildRow);
			}
			pChildRow = pChildRow->GetNextRow();
		}
	}
}

// (c.haag 2010-12-02 09:59) - PLID 38633 - Populates apResults with all the result rows that are not marked completed
// but have been signed
void CLabResultsTabDlg::GetIncompleteSignedResults(long nLabID, CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr>& apResults)
{
	BOOL bInPDFView = (IsDlgButtonChecked(IDC_PDFVIEW_RADIO) == BST_CHECKED);

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypLabRows;
	//TES 2/17/2015 - PLID 63812 - Use the preference to sign/acknowledge all specimens
	if (bInPDFView || GetRemotePropertyInt("SignAndAcknowledgeResultsForAllSpecimens", 0, 0, "<None>")) {
		// (z.manning 2011-06-17 09:58) - PLID 44154 - Attachment view is not lab-specific
		for(IRowSettingsPtr pLabRow = m_pResultsTree->GetFirstRow(); pLabRow != NULL; pLabRow = pLabRow->GetNextRow()) {
			arypLabRows.Add(pLabRow);
		}
	}
	else {
		IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
		if (NULL == pLabRow) {
			return;
		}
		arypLabRows.Add(pLabRow);
	}

	apResults.RemoveAll();
	
	for(int nLabIndex = 0; nLabIndex < arypLabRows.GetSize(); nLabIndex++)
	{
		IRowSettingsPtr pLabRow = arypLabRows.GetAt(nLabIndex);
		IRowSettingsPtr pChildRow = pLabRow->GetFirstChildRow();
		while (pChildRow) {
			if (-1 == VarLong(pChildRow->GetValue(lrtcCompletedBy), -1) &&
				pChildRow->GetValue(lrtcSignatureInkData).vt != VT_NULL) {
				apResults.Add(pChildRow);
			}
			pChildRow = pChildRow->GetNextRow();
		}
	}
}

// (c.haag 2010-12-01 11:16) - PLID 37372 - Called when the user presses the signature button.
// The signature corresponds to a single result.
void CLabResultsTabDlg::OnSignature()
{
	try
	{
		// (c.haag 2010-12-10 10:08) - PLID 38633 - sptDynamic0 was defined as "Mark Completed". 
		// We actually now check sptDynamic3 which is "Signed". Anyone who had sptDynamic0 permission
		// before has sptDynamic3 now. This permission applies to both requisition signatures and result
		// signatures.
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic3))
			return;

		// (c.haag 2010-12-01 11:16) - PLID 37372 - Perform based on the current selection and
		// what results are signed
		// (c.haag 2011-02-22) - PLID 41618 - Special handling for the attachments view
		if (m_nCurrentView == rvPDF)
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnSignature();
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnSignature without a valid attachment view!");
			}		
		}
		else 
		{
			SignResults(m_pResultsTree->CurSel);
		}		
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-06-22 10:39) - PLID 44154
CString CLabResultsTabDlg::GetResultRowText(IRowSettingsPtr pResultRow)
{
	if(pResultRow == NULL) {
		return "";
	}

	CString strResultText = VarString(pResultRow->GetValue(lrtcValue), "");
	IRowSettingsPtr pLabRow = GetLabRow(pResultRow);
	if(pLabRow != NULL) {
		CString strLabNumber = VarString(pLabRow->GetValue(lrtcValue), "");
		if(!strLabNumber.IsEmpty()) {
			strResultText += " (" + strLabNumber + ")";
		}
	}

	return strResultText;
}

// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for signing results. pActiveRow can be a result or a specimen or null.
// (j.luckoski 2013-03-20 11:40) - PLID 55424- added bool in order to mark all or allow specification
void CLabResultsTabDlg::SignResults(NXDATALIST2Lib::IRowSettingsPtr pActiveRow,bool bIsMarkAll /* =false */)
{
	// (c.haag 2010-12-01 11:16) - PLID 37372 - Perform based on pActiveRow and what results are signed
	long nLabID;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);
	BOOL bAllResultsSigned = AllResultsAreSigned(nLabID);
	CArray<IRowSettingsPtr,IRowSettingsPtr> apUnsignedResults; // Array of row pointers to results. We use the ordinals as ID's in a multi-selection if necessary.

	// The following should never be true
	if (-2 == nLabID || 0 == arypResultRows.GetSize()) {
		return;
	}

	GetUnsignedResults(nLabID, apUnsignedResults);
	
	// If there's only one result, keep it simple.
	if (1 == arypResultRows.GetSize()) 
	{
		if (apUnsignedResults.GetSize() == 0) {
			ViewSignature(arypResultRows.GetAt(0));
			return;
		}
	}
	// If we have a possible mix of signed and unsigned results, act based on pSelectedResultRow
	else 
	{
		BOOL bShowMenu = FALSE;
		CMenu mnu;
		mnu.CreatePopupMenu();

		// If all results are signed, deal only with signatures
		if (bAllResultsSigned) 
		{
			if(arypResultRows.GetSize() == 1) {
				ViewSignature(arypResultRows.GetAt(0));
				return;
			}
			else {
				bShowMenu = TRUE;
			}
		}		
		else if(apUnsignedResults.GetCount() < arypResultRows.GetCount())
		{
			bShowMenu = TRUE;
			// We have an active result and it's signed. Track a menu that lets the user choose between viewing the signature 
			// and signing for other results
			mnu.AppendMenu(MF_ENABLED, -2, "Sign Other Results");
		}

		if(bShowMenu)
		{
			// (z.manning 2011-06-22 10:55) - PLID 44154 - Add menu options to view the signature for each signed row.
			// We'll use the value of the row pointer for the menu option so we can identify which row the user selected.
			for(int nResultIndex = 0; nResultIndex < arypResultRows.GetCount(); nResultIndex++) {
				IRowSettingsPtr pResultRow = arypResultRows.GetAt(nResultIndex);
				if(pResultRow->GetValue(lrtcSignatureInkData).vt != VT_NULL) {
					mnu.AppendMenu(MF_ENABLED, (UINT)((LPDISPATCH)pResultRow), "View Signature for '" + GetResultRowText(pResultRow) + "'");
				}
			}

			CPoint ptClicked;
			GetCursorPos(&ptClicked);
			int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
			// Now process the menu selection
			if(nResult > 0) {
				IRowSettingsPtr pSelectedRow((LPDISPATCH)nResult);
				if(pSelectedRow != NULL) {
					ViewSignature(pSelectedRow);
				}
				return;
			}
			else if (nResult == -2) {
				GetUnsignedResults(nLabID, apUnsignedResults);
			}
			else {
				return;
			}
		}
	}

	// (c.haag 2010-12-01 11:16) - PLID 37372 - When we get here, apUnsignedResults is filled with all the
	// possible results that can be signed. Have the user decide which ones to sign.
	if (apUnsignedResults.GetSize() > 1 && !bIsMarkAll) {
		// (j.armen 2012-06-08 09:32) - PLID 49607 - Input for this multiselect dlg isn't cacheable, so let's set a manual cache name
		CMultiSelectDlg dlg(this, "LabResultsT");
		CStringArray astrResults; // Array of result names
		dlg.m_bPreSelectAll = TRUE;
		for (int i=0; i < apUnsignedResults.GetSize(); i++) 
		{
			astrResults.Add(GetResultRowText(apUnsignedResults[i]));
		}
		if (IDOK != dlg.OpenWithStringArray(astrResults, "Please select one or more results to sign off", 1)) {
			return;
		} else {
			// Replace apUnsignedResults with only the selected 	
			CArray<IRowSettingsPtr,IRowSettingsPtr> apSelectedResults;
			CArray<long,long> arSelections;
			dlg.FillArrayWithIDs(arSelections);
			for (i=0; i < arSelections.GetSize(); i++) 
			{
				apSelectedResults.Add( apUnsignedResults[ arSelections[i] ] );
			}
			apUnsignedResults.Copy(apSelectedResults);
		}
	}
	else {
		// Only one result can be signed; no need for a multi-select popup
	}

	// (c.haag 2010-12-01 11:16) - PLID 37372 - Do the signature
	CString strSignatureFileName = "";
	_variant_t varSignatureInkData = g_cvarNull;
	_variant_t varSignatureTextData = g_cvarNull;
	// (z.manning 2008-10-15 15:08) - PLID 21082 - Now must sign the lab as well
	CSignatureDlg dlgSign(this);
	// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
	dlgSign.SetSignature(strSignatureFileName, varSignatureInkData, varSignatureTextData);
	dlgSign.m_bRequireSignature = TRUE;
	// (z.manning 2008-12-09 09:07) - PLID 32260 - Added a preference for checking for password when loading signature.
	dlgSign.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordLab", 1, 0, GetCurrentUserName()) == 1);
	if(dlgSign.DoModal() != IDOK) {
		return;
	}
	strSignatureFileName = dlgSign.GetSignatureFileName();
	varSignatureInkData = dlgSign.GetSignatureInkData();
	// (j.jones 2010-04-12 17:29) - PLID 38166 - added a date/timestamp
	varSignatureTextData = dlgSign.GetSignatureTextData();
	COleDateTime dtNow;
	{
		_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS CurDateTime");
		if(!rs->eof){
			dtNow = AdoFldDateTime(rs, "CurDateTime");
		}
		else {
			dtNow = COleDateTime::GetCurrentTime();
		}
	}
	if(varSignatureTextData.vt != VT_EMPTY && varSignatureTextData.vt != VT_NULL) 
	{			
		// (j.jones 2010-04-13 08:48) - PLID 38166 - the loaded text is the words "Date/Time",
		// we need to replace it with the actual date/time
		CNxInkPictureText nipt;
		nipt.LoadFromVariant(varSignatureTextData);
		CString strDate;
		if(dlgSign.GetSignatureDateOnly()) {
			strDate = FormatDateTimeForInterface(dtNow, NULL, dtoDate);					
		}
		else {
			strDate = FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime);					
		}

		//technically, there should only be one SIGNATURE_DATE_STAMP_ID in the array,
		//and currently we don't support more than one stamp in the signature
		//anyways, but just incase, replace all instances of the SIGNATURE_DATE_STAMP_ID with
		//the current date/time
		for(int i=0; i<nipt.GetStringCount(); i++) {
			if(nipt.GetStampIDByIndex(i) == SIGNATURE_DATE_STAMP_ID) {
				nipt.SetStringByIndex(i, strDate);
			}
		}
		_variant_t vNewSigTextData = nipt.GetAsVariant();
		varSignatureTextData = vNewSigTextData;
	}

	// (c.haag 2010-12-01 11:16) - PLID 37372 - Now populate the selected results
	for (int i=0; i < apUnsignedResults.GetSize(); i++) 
	{
		IRowSettingsPtr pResultRow = apUnsignedResults[i];
		pResultRow->PutValue(lrtcSignatureImageFile, _bstr_t( strSignatureFileName ));
		pResultRow->PutValue(lrtcSignatureInkData, varSignatureInkData);
		pResultRow->PutValue(lrtcSignatureTextData, varSignatureTextData);
		pResultRow->PutValue(lrtcSignedBy, (long)GetCurrentUserID());
		pResultRow->PutValue(lrtcSignedDate, _variant_t(dtNow, VT_DATE));
		pResultRow->PutValue(lrtcSignedUsername, _bstr_t(GetCurrentUserName()));
	}

	// (c.haag 2010-12-01 11:16) - PLID 37372 - Set the signature button text
	FormatSignButtonText(pActiveRow);
}

// (c.haag 2010-11-22 15:39) - PLID 40556 - This even handles the Acknowledge Result button press. The
// general logic flow should closely follow FormatAcknowledgedButtonText; please compare them when reviewing or
// changing code.
void CLabResultsTabDlg::OnAcknowledgeResult()
{
	try 
	{
		// (c.haag 2011-02-22) - PLID 41618 - Special handling when in the attachments view
		if (m_nCurrentView == rvPDF) 
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnAcknowledgeResult();
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnAcknowledgeResult without a valid attachment view!");
			}		
		}
		else 
		{
			AcknowledgeResults(m_pResultsTree->CurSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for acknowleding results. pActiveRow can be a result or a specimen or null.
// (j.luckoski 2013-03-20 11:40) - PLID 55424- added bool in order to mark all or allow specification
void CLabResultsTabDlg::AcknowledgeResults(IRowSettingsPtr pActiveRow, bool bIsMarkAll /* =false */)
{
	long nLabID;
	{
		IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
		IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
		nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
	}
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypResultRows;
	//TES 11/6/2012 - PLID 53591 - Check the preference to include all specimens
	GetResults(nLabID, arypResultRows, groCheckPreference);

	if (-2 == nLabID || 0 == arypResultRows.GetCount()) {
		// We should never get here
		return;
	}

	CArray<IRowSettingsPtr,IRowSettingsPtr> arypUnacknowledgedResults;
	GetUnacknowledgedResults(nLabID, arypUnacknowledgedResults);

	BOOL bAllResultsAcknowledged = AllResultsAreAcknowledged(nLabID);
	IRowSettingsPtr pUnacknowledgeRow = NULL;
	int nResult = 0; // 0 = Undefined, 1 = Unacknowledge, -2 = Acknowledge

	if (1 == arypResultRows.GetCount()) 
	{
		if (VT_NULL != GetTreeValue(arypResultRows.GetAt(0), lrfAcknowledgedBy, lrtcForeignKeyID).vt) {
			// Already acknowledged. The action will be to unacknowledge the result.
			nResult = 1;
			pUnacknowledgeRow = arypResultRows.GetAt(0);
		}
		else {
			// Let the user acknowledge it.
			nResult = -2;
		}
	}
	else
	{
		BOOL bShowMenu = FALSE;
		CMenu mnu;
		mnu.CreatePopupMenu();

		// Special handling for when all results are acknowledged
		if (bAllResultsAcknowledged) 
		{
			// Already acknowledged. The action will be to unacknowledge the result.
			nResult = 1;
			bShowMenu = TRUE;
		}
		else {
			// If we get here, one or more results are not acknowledged. No matter what the
			// result selection is, we want them to be able to acknowledge anything that is not
			// already acknowledged. If we have a valid result selected and it's acknowledged, we
			// need to be able to unacknowledge it.
			if (arypUnacknowledgedResults.GetCount() < arypResultRows.GetCount()) 
			{
				bShowMenu = TRUE;
				// We could unacknowledge this result, or we could acknowledge other ones. Who knows?
				mnu.AppendMenu(MF_ENABLED, -2, "Acknowledge Other Results");
			}
			else {
				nResult = -2; // Acknowledge results
			}
		}

		if(bShowMenu)
		{
			// (z.manning 2011-06-22 10:55) - PLID 44154 - Add menu options to unacknowledge for each acknowledged row.
			// We'll use the value of the row pointer for the menu option so we can identify which row the user selected.
			for(int nResultIndex = 0; nResultIndex < arypResultRows.GetCount(); nResultIndex++) {
				IRowSettingsPtr pResultRow = arypResultRows.GetAt(nResultIndex);
				if(GetTreeValue(pResultRow, lrfAcknowledgedBy, lrtcForeignKeyID).vt != VT_NULL) {
					mnu.AppendMenu(MF_ENABLED, (UINT)((LPDISPATCH)pResultRow), "Unacknowledge Result: '" + GetResultRowText(pResultRow) + "'");
				}
			}

			CPoint ptClicked;
			GetCursorPos(&ptClicked);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
			if(nResult > 0) {
				pUnacknowledgeRow = (LPDISPATCH)nResult;
				if(pUnacknowledgeRow == NULL) {
					return;
				}
				nResult = 1;
			}
		}
	}

	// Now process the user action
	switch (nResult)
	{
	case 1: // Unacknowledge
		if(pUnacknowledgeRow == NULL) {
			return;
		}
		if (IDNO == MsgBox(MB_YESNO, "The user '%s' acknowledged this lab result on %s. Are you SURE you wish to reset this status?",
			GetExistingUserName(VarLong(GetTreeValue(pUnacknowledgeRow, lrfAcknowledgedBy, lrtcForeignKeyID))), 
			FormatDateTimeForInterface(GetTreeValue(pUnacknowledgeRow, lrfAcknowledgedOn, lrtcValue), DTF_STRIP_SECONDS, dtoDateTime)))
		{
			return;
		}
		else {
			SetTreeValue(pUnacknowledgeRow, lrfAcknowledgedBy, lrtcForeignKeyID, g_cvarNull);
			SetTreeValue(pUnacknowledgeRow, lrfAcknowledgedOn, lrtcValue, g_cvarNull);
			m_bResultIsSaved = FALSE;
		}
		break;
	case -2: // Acknowledge 
		{
			// If there's just one, there's no need to pop up a multi-select dialog
			if (arypUnacknowledgedResults.GetSize() > 1 && !bIsMarkAll)
			{
				// (j.armen 2012-06-08 09:32) - PLID 49607 - Input for this multiselect dlg isn't cacheable, so let's set a manual cache name
				CMultiSelectDlg dlg(this, "LabResultsT");
				CStringArray astrResults; // Array of result names
				dlg.m_bPreSelectAll = TRUE;
				for (int i=0; i < arypUnacknowledgedResults.GetSize(); i++) 
				{
					IRowSettingsPtr pRow = arypUnacknowledgedResults[i];
					astrResults.Add(GetResultRowText(pRow));
				}
				if (IDOK != dlg.OpenWithStringArray(astrResults, "Please select one or more results to acknowledge", 1)) {
					return;
				} 
				else {
					// Replace apResults with only the selected 	
					CArray<IRowSettingsPtr,IRowSettingsPtr> apSelectedResults;
					CArray<long,long> arSelections;
					dlg.FillArrayWithIDs(arSelections);
					for (i=0; i < arSelections.GetSize(); i++) 
					{
						apSelectedResults.Add( arypUnacknowledgedResults[ arSelections[i] ] );
					}
					arypUnacknowledgedResults.Copy(apSelectedResults);
				}
			}
			else {
				// Only one result can be acknowledged; no need for a multi-select popup
			}

			// Acknowledge each selected result
			const long nUserID = GetCurrentUserID();
			const COleDateTime dtNow = COleDateTime::GetCurrentTime();
			for (int i=0; i < arypUnacknowledgedResults.GetSize(); i++) 
			{
				IRowSettingsPtr pR = arypUnacknowledgedResults[i];
				SetTreeValue(pR, lrfAcknowledgedBy, lrtcForeignKeyID, nUserID);
				SetTreeValue(pR, lrfAcknowledgedOn, lrtcValue, _variant_t(dtNow, VT_DATE));
			}
			m_bResultIsSaved = FALSE;
		}
		break;
	}

	// Update the button
	FormatAcknowledgedButtonText(pActiveRow);
	FormatAcknowledgeAndSignButtonText(pActiveRow);
	FormatMarkAllCompleteButtonText(pActiveRow);
}

// (j.gruber 2010-11-30 09:14) - PLID 41606
void CLabResultsTabDlg::LoadHTMLReport(NXDATALIST2Lib::IRowSettingsPtr pSpecRow)
{

	//load the HTML for each Result for this specimen
	CString strResultHTML;

	// (j.gruber 2010-12-08 13:42) - PLID 41662 - make sure everything is hidden so it doesn't draw though the PDF viewer
	// (c.haag 2011-02-22) - PLID 42589 - Pass in the current tree selection
	UpdateControlStates(true, m_pResultsTree->CurSel);
	
	NXDATALIST2Lib::IRowSettingsPtr pResult = pSpecRow->GetFirstChildRow();
	while (pResult) {

		strResultHTML += GetResultHTML(pResult);

		/*long nResultID = VarLong(pResult->GetValue(lrtcResultID));

		//look up the result in the map
		stResults *pstRes = NULL;
		if (!m_mapResults.Lookup(nResultID, pstRes)) {
			ASSERT(FALSE);
		}

		if (pstRes) {
			strResultHTML += GetResultHTML(pstRes);
		}*/

		pResult = pResult->GetNextRow();
	}

	//now that we have the result HTML Generate our wrapper HTML
	CString strHTML;
	// (j.gruber 2010-12-08 14:19) - PLID 41759
	strHTML = GetHeaderHTML(pSpecRow);

	strHTML += strResultHTML;

	strHTML += GetFooterHTML();

	// (j.gruber 2010-11-30 09:14) - PLID 41606 - unload any previous reports
	UnloadHTMLReports();

	//now create the filename
	long nLabID = VarLong(pSpecRow->GetValue(lrtcForeignKeyID));
	CString strFileName = GetHTMLFileName(nLabID);
	CFile fileTemp(strFileName, CFile::modeCreate|CFile::modeReadWrite);
	fileTemp.Write((LPCTSTR)strHTML, strHTML.GetLength());

	//load our array with our report
	m_straryLabReports.Add(strFileName);

	//flag it to get removed at reboot in case of crash or something
	MoveFileEx(strFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	
	//Now we have to make the control browse to it
	COleVariant varURL(strFileName);
	// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
	m_pBrowser->Navigate2(varURL, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
	GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE);

	// (b.spivey, August 27, 2013) - PLID 46295 - Show anatomical location on the discrete/report views.
	CString strAnatomicLocation = GetAnatomicalLocationAsString(nLabID);

	//discrete results 
	m_nxstaticAnatomicalLocationTextDiscrete.SetWindowTextA(strAnatomicLocation);
	m_nxstaticAnatomicalLocationTextDiscrete.ShowWindow(SW_HIDE); 

	//report view
	m_nxstaticAnatomicalLocationTextReport.SetWindowTextA(strAnatomicLocation);
	m_nxstaticAnatomicalLocationTextReport.ShowWindow(SW_SHOW); 

	// (j.gruber 2010-11-30 09:14) - PLID 41607
	UpdateScrollButtons();

}

// (j.gruber 2010-11-30 09:14) - PLID 41606
CString CLabResultsTabDlg::GetHTMLFileName(long nLabID)
{
	CString strFileName;
	strFileName.Format("nexlab_%li_%lu.htm", nLabID, GetTickCount());
	CString strFullName = GetNxTempPath() ^ strFileName;
	return strFullName;
}

// (j.gruber 2010-11-30 09:14) - PLID 41653
// (j.gruber 2010-12-08 14:19) - PLID 41759 - added row
CString CLabResultsTabDlg::GetHeaderHTML(NXDATALIST2Lib::IRowSettingsPtr pSpecRow)
{

	CString strHeader;

	//TES 2/24/2012 - PLID 44841 - Need doc type switching to get the tables to look right
	strHeader += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">";
	strHeader += "<HTML>\r\n";
	strHeader += GetStyle();
	strHeader += "\t<HEAD>\r\n";
	// (j.gruber 2010-12-08 11:49) - PLID 41759
	if (pSpecRow) {

		//get the Specimen number of the lab
		_variant_t varForm = pSpecRow->GetValue(lrtcValue);
		CString strForm;
		if (varForm.vt == VT_BSTR) {
			strForm = VarString(varForm);
		}
		else {
			strForm = "";
		}
		//TES 9/10/2013 - PLID 58511 - If this lab has been replaced, put a warning at the very top
		//TES 11/5/2013 - PLID 59320 - Modified the warning, now that the results are actually hidden
		if(VarBool(pSpecRow->GetValue(lrtcExtraValue), FALSE)) {
			strHeader += "\t\t<H2> Some results for this order have been replaced by subsequent results, and have been removed from this display</H2>\r\n";
		}
		// (r.gonet 07/26/2013) - PLID 57751 - Escape the value does not include any syntax characters.
		strHeader += "\t\t<H1> " + ConvertToQuotableXMLString(strForm) + "</H1>\r\n";

		// (d.singleton 2013-10-29 15:17) - PLID 59197 - need to move the specimen segment values to its own header in the html view for labs,  so it will show regardless of any results. 
		strHeader += GetSpecimenFieldsHTML(pSpecRow);
	}	

	strHeader += "\t</HEAD>\r\n";
	strHeader += "\t<BODY>\r\n";

	return strHeader;
}

// (j.gruber 2010-11-30 09:14) - PLID 41653
CString CLabResultsTabDlg::GetFooterHTML()
{
	CString strFooter;

	strFooter += "\t</BODY>\r\n";
	strFooter += "</HTML>\r\n";

	return strFooter;
}

//TES 2/24/2012 - PLID 44841 - New function to get the HTML for a single result field.  Takes in the row, a ResultFieldPosition that specifies
// both the field and its position in the report, and the column width, and outputs whether or not it actually filled any information.
CString CLabResultsTabDlg::GetResultFieldHTML(NXDATALIST2Lib::IRowSettingsPtr pResultRow, const CLabResultsTabDlg::ResultFieldPosition &rfp, bool &bFilled, long nColumnWidth)
{
	//TES 2/24/2012 - PLID 44841 - Get the name of the field, a name for the div, and the value of the field, based on rfp
	CString strFieldName, strDivName, strFieldValue;
	switch(rfp.lrf) {
		case lrfName:
			strFieldName = "Name";
			strDivName = "ResultName";
			strFieldValue = VarString(pResultRow->GetValue(lrtcValue), "");
			break;
		case lrfDateReceivedByLab:
			strFieldName = "Date Received By Lab";
			strDivName = "DateReceivedByLab";
			{
				_variant_t varDateReceivedByLab = GetTreeValue(pResultRow, lrfDateReceivedByLab, lrtcValue);
				COleDateTime dtReceivedByLab;
				if (varDateReceivedByLab.vt == VT_DATE) {
					dtReceivedByLab = VarDateTime(varDateReceivedByLab);
				}
				else if (varDateReceivedByLab.vt == VT_BSTR) {
					if (VarString(varDateReceivedByLab).IsEmpty()) {			
						dtReceivedByLab.SetStatus(COleDateTime::invalid);
					}
					else {
						dtReceivedByLab.ParseDateTime(VarString(varDateReceivedByLab));			
					}
				}
				else {
					dtReceivedByLab.SetStatus(COleDateTime::invalid);
				}

				if (dtReceivedByLab.GetStatus() == COleDateTime::valid) {
					strFieldValue = FormatDateTimeForInterface(dtReceivedByLab);
				}
			}
			break;
		case lrfDateReceived:
			strFieldName = "Date Received";
			strDivName = "DateReceived";
			{
				_variant_t varDateReceived = GetTreeValue(pResultRow, lrfDateReceived, lrtcValue);
				COleDateTime dtReceived;
				if (varDateReceived.vt == VT_DATE) {
					dtReceived = VarDateTime(varDateReceived);
				}
				else if (varDateReceived.vt == VT_BSTR) {
					if (VarString(varDateReceived).IsEmpty()) {			
						dtReceived.SetStatus(COleDateTime::invalid);
					}
					else {
						dtReceived.ParseDateTime(VarString(varDateReceived));			
					}
				}
				else {
					dtReceived.SetStatus(COleDateTime::invalid);
				}

				if (dtReceived.GetStatus() == COleDateTime::valid) {
					strFieldValue = FormatDateTimeForInterface(dtReceived);
				}
			}
			break;
		case lrfDatePerformed:
			strFieldName = "Date Performed";
			strDivName = "DatePerformed";
			{
				_variant_t varDatePerformed = GetTreeValue(pResultRow, lrfDatePerformed, lrtcValue);
				COleDateTime dtPerformed;
				if (varDatePerformed.vt == VT_DATE) {
					dtPerformed = VarDateTime(varDatePerformed);
				}
				else if (varDatePerformed.vt == VT_BSTR) {
					if (VarString(varDatePerformed).IsEmpty()) {			
						dtPerformed.SetStatus(COleDateTime::invalid);
					}
					else {
						dtPerformed.ParseDateTime(VarString(varDatePerformed));			
					}
				}
				else {
					dtPerformed.SetStatus(COleDateTime::invalid);
				}
				if (dtPerformed.GetStatus() == COleDateTime::valid) {
					strFieldValue = FormatDateTimeForInterface(dtPerformed);
				}
			}
			break;
		case lrfSlideNum:
			strFieldName = "Slide #";
			strDivName = "SlideNumber";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfSlideNum, lrtcValue),"");
			break;
		case lrfLOINC:
			strFieldName = strDivName = "LOINC";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfLOINC, lrtcValue),"");
			break;
		case lrfStatus:
			strFieldName = strDivName = "Status";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfStatus,lrtcValue), "");
			break;
		case lrfFlag:
			strFieldName = strDivName = "Flag";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfFlag, lrtcValue), "");
			break;
		case lrfAcknowledgedBy:
			strFieldName = strDivName = "Acknowledged";
			{
				long nAcknowledgedUserID = VarLong(GetTreeValue(pResultRow, lrfAcknowledgedBy, lrtcForeignKeyID), -1);
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				COleDateTime dtAcknowledgedDate = VarDateTime(GetTreeValue(pResultRow, lrfAcknowledgedOn, lrtcValue), dtInvalid);
				strFieldValue = (nAcknowledgedUserID != -1 ? "By " + GetExistingUserName(nAcknowledgedUserID) + " on " + FormatDateTimeForInterface(dtAcknowledgedDate) : "No");
			}
			break;
		case lrfUnits:
			strFieldName = strDivName = "Units";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfUnits,lrtcValue), "");
			break;
		case lrfReference:
			strFieldName = strDivName = "Reference";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfReference,lrtcValue), "");
			break;
		case lrfDiagnosis:
			strFieldName = strDivName = "Diagnosis";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfDiagnosis, lrtcValue), "");
			break;
		case lrfMicroscopicDescription:
			strFieldName = "Microscopic Description";
			strDivName = "MicroDesc";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfMicroscopicDescription, lrtcValue), "");
			break;		
		// (d.singleton 2013-07-16 17:35) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
		case lrfServiceStartTime:
			strFieldName = "Lab Service Start Time";
			strDivName = "ServiceStartTime";
			{
				_variant_t var = GetTreeValue(pResultRow, lrfServiceStartTime, lrtcValue);
				COleDateTime dtServiceStartTime;
				if(var.vt == VT_DATE) {
					dtServiceStartTime = VarDateTime(var);
				}
				else if(var.vt == VT_BSTR) {
					if(VarString(var).IsEmpty()) {
						dtServiceStartTime.SetStatus(COleDateTime::invalid);
					}
					else {
						dtServiceStartTime.ParseDateTime(VarString(var));
					}
				}
				else {
					dtServiceStartTime.SetStatus(COleDateTime::invalid);
				}
				if(dtServiceStartTime.GetStatus() == COleDateTime::valid) {
					strFieldValue = FormatDateTimeForInterface(dtServiceStartTime);
				}
			}
			break;	
		case lrfServiceEndTime:
			strFieldName = "Lab Service End Time";
			strDivName = "ServiceEndTime";
			{
				_variant_t var = GetTreeValue(pResultRow, lrfServiceEndTime, lrtcValue);
				COleDateTime dtServiceEndTime;
				if(var.vt == VT_DATE) {
					dtServiceEndTime = VarDateTime(var);
				}
				else if(var.vt == VT_BSTR) {
					if(VarString(var).IsEmpty()) {
						dtServiceEndTime.SetStatus(COleDateTime::invalid);
					}
					else {
						dtServiceEndTime.ParseDateTime(VarString(var));
					}
				}
				else {
					dtServiceEndTime.SetStatus(COleDateTime::invalid);
				}
				if(dtServiceEndTime.GetStatus() == COleDateTime::valid) {
					strFieldValue = FormatDateTimeForInterface(dtServiceEndTime);
				}
			}
			break;
		case lrfPerformingProvider:
			strFieldName = "Performing Provider";
			strDivName = "PerformingProvider";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerformingProvider, lrtcValue), "");
			break;
		// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
		case lrfPerformingLab:
			strFieldName = "Performing Lab";
			strDivName = "PerformingLab";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerformingLab, lrtcValue), "");
			break;
		case lrfPerfLabAddress:
			strFieldName = "Perf Lab Address";
			strDivName = "PerfLabAddress";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerfLabAddress, lrtcValue), "");
			break;
		case lrfPerfLabCity:
			strFieldName = "Perf Lab City";
			strDivName = "PerfLabCity";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerfLabCity, lrtcValue), "");
			break;
		case lrfPerfLabState:
			strFieldName = "Perf Lab State";
			strDivName = "PerfLabState";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerfLabState, lrtcValue), "");
			break;
		case lrfPerfLabZip:
			strFieldName = "Perf Lab Zip";
			strDivName = "PerfLabZip";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerfLabZip, lrtcValue), "");
			break;
		case lrfPerfLabCountry:
			strFieldName = "Perf Lab Country";
			strDivName = "PerfLabCountry";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerfLabCountry, lrtcValue), "");
			break;
		case lrfPerfLabParish:
			strFieldName = "Perf Lab Parish";
			strDivName = "PerfLabParish";
			strFieldValue = VarString(GetTreeValue(pResultRow, lrfPerfLabParish, lrtcValue), "");
			break;
		// (d.singleton 2013-11-04 17:06) - PLID 59294 - add observation date to the html view for lab results. 
		case lrfObservationDate:
			strFieldName = "Observation Date";
			strDivName = "ObservationDate";
			{
				_variant_t var = GetTreeValue(pResultRow, lrfObservationDate, lrtcValue);
				COleDateTime dtObservationDate;
				if(var.vt == VT_DATE) {
					dtObservationDate = VarDateTime(var);
				}
				else if(var.vt == VT_BSTR) {
					if(VarString(var).IsEmpty()) {
						dtObservationDate.SetStatus(COleDateTime::invalid);
					}
					else {
						dtObservationDate.ParseDateTime(VarString(var));
					}
				}
				else {
					dtObservationDate.SetStatus(COleDateTime::invalid);
				}
				if(dtObservationDate.GetStatus() == COleDateTime::valid) {
					strFieldValue = FormatDateTimeForInterface(dtObservationDate);
				}
			}
			break;
	}

	// (r.gonet 07/26/2013) - PLID 57751 - Escape the value does not include any syntax characters.
	strFieldName = ConvertToQuotableXMLString(strFieldName);
	// (r.gonet 07/26/2013) - PLID 57751 - Escape the value does not include any syntax characters.
	strFieldValue = ConvertToQuotableXMLString(strFieldValue);

	//TES 2/24/2012 - PLID 44841 - Now, if we have a value, return HTML outputting that value in a column of the width we were given.
	if(!strFieldValue.IsEmpty()) {
		bFilled = true;
		CString strHTML;
		strHTML += FormatString("\t\t\t\t<td colspan=\"1\">\r\n");
		strHTML += FormatString("\t\t\t\t\t<div id=\"%s\" style=\"width:%ipx;text-align:left;vertical-align:top;\"> \r\n", strDivName, nColumnWidth);
		strHTML += FormatString("\t\t\t\t\t\t<span class=\"MinorTitle\"> %s: </span> \r\n", strFieldName);
		// (r.gonet 07/26/2013) - PLID 57751 - Noticed linebreaks weren't being retained in the report view for mapped fields. Fixed.
		strFieldValue.Replace("\r\n", "<BR/>");
		// (r.gonet 03/07/2013) - PLID 43599 - If the setting is disabled, preserve consecutive whitespace.
		if(GetRemotePropertyInt("LabReportViewTrimExtraSpaces", TRUE) == FALSE) {
			strFieldValue.Replace(" ", "&nbsp;");
		} else {
			// The browser will trim the spaces.
		}
		strHTML += FormatString("\t\t\t\t\t\t<span class=\"MinorInfo\"> %s </span> \r\n", strFieldValue);
		strHTML += "\t\t\t\t\t</div>\r\n";
		strHTML += "\t\t\t\t</td>\r\n";
		return strHTML;

	}
	else {
		//TES 2/24/2012 - PLID 44841 - Just output a blank column of the width we were given.
		return FormatString("<td colspan=\"1\"><div style=\"width:%ipx;text-align:left;vertical-align:top;\"></div></td>", nColumnWidth);
	}
}

// (j.gruber 2010-11-30 09:14) - PLID 41653
CString CLabResultsTabDlg::GetResultHTML(NXDATALIST2Lib::IRowSettingsPtr pResultRow)
{
	//first let's Generate our HTML script
	CString strHTML;

	//TES 9/10/2013 - PLID 58511 - If this result has been replaced, put a warning at the very top
	//TES 11/5/2013 - PLID 59320 - Actually, instead we're just going to not include it in the HTML
	if(VarBool(pResultRow->GetValue(lrtcExtraValue),FALSE)) {
		//strHTML += "<H2> WARNING: This result has been replaced by a subsequent result, and should be ignored </H2>\r\n";
		return "";
	}
	
	//first the main table	
	strHTML += "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"table-layout:fixed\">";
	long nColumnCount = GetColumnCount();
	long nColumnWidth = (nColumnCount==0)?0:GetBrowserWidth()/nColumnCount;
	for(int i = 0; i < nColumnCount; i++) {
		strHTML += FormatString("<col width=%i>", nColumnWidth);
	}
	strHTML += "<tr>";
	for(int i = 0; i < nColumnCount; i++) {
		strHTML += FormatString("<td><div style=\"width:%i;height=0;\"></div></td>", nColumnWidth);
	}
	strHTML += "</tr>";
	
	//TES 2/24/2012 - PLID 44841 - Make sure we've loaded the positions of all the result fields.
	EnsureResultFieldPositions();

	//TES 2/24/2012 - PLID 44841 - Now, go through all our result field positions (which will have been sorted in the order they need to be
	// output), and add the HTML for each.
	int nCurrentRow = 0;
	CString strCurrentRowHTML;
	bool bCurrentRowHasData = false;
	CString strCurrentFieldHTML;
	int nCurrentFieldColumn = -1;
	for(int i = 0; i < m_arResultFieldPositions.GetSize(); i++) {
		ResultFieldPosition rfp = m_arResultFieldPositions[i];
		//TES 3/27/2012 - PLID 49209 - Check whether we're actually showing this field.
			if(rfp.bShowField) {
			//TES 2/24/2012 - PLID 44841 - Are we on a new row?
			if(rfp.nRow > nCurrentRow) {
				nCurrentRow = rfp.nRow;
				//TES 2/24/2012 - PLID 44841 - Did the previous row have any data?
				if(bCurrentRowHasData) {
					//TES 2/24/2012 - PLID 44841 - Close off the current field, it should span the rest of the columns.
					ASSERT(nCurrentFieldColumn > 0);
					int nColSpan = GetColumnCount()-nCurrentFieldColumn+1;
					strCurrentFieldHTML.Replace(FormatString("width:%ipx;", nColumnWidth), FormatString("width:%ipx;", nColumnWidth*nColSpan));
					strCurrentFieldHTML.Replace("colspan=\"1\"", FormatString("colspan=\"%i\"", nColSpan));
					strCurrentRowHTML += strCurrentFieldHTML;
					strCurrentFieldHTML = "";
					nCurrentFieldColumn = -1;
					//TES 2/24/2012 - PLID 44841 - Close off the current row.
					strCurrentRowHTML += "</tr>";
					strHTML += strCurrentRowHTML;
					strCurrentRowHTML = "<tr>";
					bCurrentRowHasData = false;
				}
				//TES 2/24/2012 - PLID 44841 - If the next field doesn't start at the beginning, add a blank cell spanning the columns in the
				// next row which don't have fields.
				if(rfp.nActualColumn > 1) {
					strCurrentRowHTML += FormatString("<td colspan=\"%i\"><div style=\"width:%ipx;text-align:left;vertical-align:top;\"></div></td>", (rfp.nActualColumn-1), nColumnWidth*(rfp.nActualColumn-1));
				}
				//TES 2/24/2012 - PLID 44841 - Load the next field.
				bool bFieldHasData = false;
				CString strFieldHTML = GetResultFieldHTML(pResultRow, rfp, bFieldHasData, nColumnWidth);
				if(bFieldHasData) {
					//TES 2/24/2012 - PLID 44841 - The field has data, that means that this row has data
					bCurrentRowHasData = true;
				}
				//TES 2/24/2012 - PLID 44841 - Remember this information, so we can close off this field when we find the next valid one.
				// Because we're at the beginning of a row, we treat this as a field even if it's empty.
				strCurrentFieldHTML = strFieldHTML;
				nCurrentFieldColumn = rfp.nActualColumn;
			}
			else {
				//TES 2/24/2012 - PLID 44841 - Load the next field.
				bool bFieldHasData = false;
				CString strFieldHTML = GetResultFieldHTML(pResultRow, rfp, bFieldHasData, nColumnWidth);
				if(bFieldHasData) {
					//TES 2/24/2012 - PLID 44841 - Close off our previous field.  It should span from wherever it started to here.
					ASSERT(nCurrentFieldColumn > 0);
					int nColSpan = rfp.nActualColumn-nCurrentFieldColumn;
					strCurrentFieldHTML.Replace(FormatString("width:%ipx;", nColumnWidth), FormatString("width:%ipx;", nColumnWidth*nColSpan));
					strCurrentFieldHTML.Replace("colspan=\"1\"", FormatString("colspan=\"%i\"", nColSpan));
					strCurrentRowHTML += strCurrentFieldHTML;
					strCurrentFieldHTML = strFieldHTML;
					nCurrentFieldColumn = rfp.nActualColumn;
					bCurrentRowHasData = true;
				}
			}
		}
	}
	//TES 2/24/2012 - PLID 44841 - Close off our previous field.  It should span from wherever it started to the end of the row.
	int nColSpan = nColumnCount-nCurrentFieldColumn+1;
	strCurrentFieldHTML.Replace(FormatString("width:%ipx;", nColumnWidth), FormatString("width:%ipx;", nColumnWidth*nColSpan));
	strCurrentFieldHTML.Replace("colspan=\"1\"", FormatString("colspan=\"%i\"", nColSpan));
	strCurrentRowHTML += strCurrentFieldHTML;
	if(bCurrentRowHasData) {
		//TES 2/24/2012 - PLID 44841 - Close off the current row.
		strCurrentRowHTML += "</tr>";
		strHTML += strCurrentRowHTML;
	}
	//TES 2/24/2012 - PLID 44841 - Close off the table	
	strHTML += "</table>";

	strHTML += "\t\t<div class=\"InnerTable\"> \r\n";

	
	//next row contains Result Comments
	CString strTemp;
	//Result Comments
	CString strComments = VarString(GetTreeValue(pResultRow, lrfComments,lrtcValue), "");
	if (!strComments.IsEmpty()) {
		strHTML += "\t\t\t<div class=\"InnerRow\">\r\n";
		strHTML += "\t\t\t\t<div class=\"InnerCell\">\r\n";
		strHTML += "\t\t\t\t\t<div id=\"Comments\"> ";
		strHTML += "\t\t\t\t\t\t<span class=\"MinorTitle\"> Comments: </span> \r\n";
		// (r.gonet 07/26/2013) - PLID 57751 - Escape the value does not include any syntax characters.
		strComments = ConvertToQuotableXMLString(strComments);
		strComments.Replace("\r\n", "<BR/>");
		// (r.gonet 03/07/2013) - PLID 43599 - If the setting is disabled, then preserve consecutive whitespace.
		if(GetRemotePropertyInt("LabReportViewTrimExtraSpaces", TRUE) == FALSE) {
			strComments.Replace(" ", "&nbsp;");
		} else {
			// The browser will trim the extra spaces.
		}
		strTemp.Format("\t\t\t\t\t\t<span class=\"MinorInfo\"> %s </span> \r\n", strComments);
		strHTML += strTemp;
		strHTML += "\t\t\t\t\t</div>";
		//close the Cell
		strHTML += "\t\t\t\t</div>\r\n";	
		//close the Row
		strHTML += "\t\t\t</div>\r\n";
	}

	//and last the values
	long nMailID = VarLong(GetTreeValue(pResultRow, lrfValue,lrtcForeignKeyID),-1);
	CString strValue = VarString(GetTreeValue(pResultRow, lrfValue,lrtcValue), "");
	if (!strValue.IsEmpty()) {
		strHTML += "\t\t\t<div class=\"InnerRow\">\r\n";
		strHTML += "\t\t\t\t<div class=\"InnerCell\">\r\n";
		strHTML += "\t\t\t\t\t<div id=\"Value\"> ";
		strHTML += "\t\t\t\t\t\t<span class=\"MinorTitle\"> Value: </span> \r\n";
		// (r.gonet 07/26/2013) - PLID 57751 - Escape the value does not include any syntax characters.
		strValue = ConvertToQuotableXMLString(strValue);
		if(strValue.Replace("\r\n", "<BR/>") > 0) {
			// (r.gonet 03/07/2013) - PLID 43599 - A multiline value is almost always a paragraph or a tabular form of data.
			//  It should not be on the same line. Things like blood work usually take one line though and are best visible as a single line.
			strValue = "<BR/>" + strValue;
		}
		// (r.gonet 03/07/2013) - PLID 43599 - If the setting is disabled, then preserve consecutive spaces.
		if(GetRemotePropertyInt("LabReportViewTrimExtraSpaces", TRUE) == FALSE) {
			strValue.Replace(" ", "&nbsp;");
		} else {
			// (r.gonet 03/07/2013) - PLID 43599 - The browser will trim the extra spaces.
		}
		strTemp.Format("\t\t\t\t\t\t<span class=\"MinorInfo\"> %s </span> \r\n", strValue);
		strHTML += strTemp;
		strHTML += "\t\t\t\t\t</div>";
		//close the Cell
		strHTML += "\t\t\t\t</div>";
		//close the Row
		strHTML += "\t\t\t</div>";

	}

	//close it off
	strHTML += "\t\t\t<div class=\"InnerRow\">\r\n";
	strHTML += "\t\t\t\t<div class=\"InnerCellWithBorder\">\r\n";
	strHTML += "\t\t\t\t\t<div id=\"End\"> ";
	strHTML += "\t\t\t\t\t\t<span class=\"MinorTitle\">  </span> \r\n";	
	strTemp.Format("\t\t\t\t\t\t<span class=\"MinorInfo\"> </span> \r\n");
	strHTML += strTemp;
	strHTML += "\t\t\t\t\t</div>";
	//close the Cell
	strHTML += "\t\t\t\t</div>";	
	//close the Row
	strHTML += "\t\t\t</div>";

	strHTML += "\t\t</div> \r\n";	

	return strHTML;
}

BOOL CLabResultsTabDlg::IsVerticalScrollBarVisible() {
	
	SCROLLBARINFO sbi;
	sbi.cbSize = sizeof(SCROLLBARINFO);	
	long hwbrowser;		
	m_pBrowser->get_HWND(&hwbrowser);
	::GetScrollBarInfo((HWND)hwbrowser, OBJID_VSCROLL, &sbi);
	return ((sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE) == 0);

}

//TES 2/24/2012 - PLID 44841 - Split out the code to calculate the width of the browser window into its own function.
long CLabResultsTabDlg::GetBrowserWidth()
{
	//get the width of the window we have
	CRect rcBrowser;
	GetDlgItem(IDC_PDF_PREVIEW)->GetWindowRect(rcBrowser);
	ScreenToClient(rcBrowser);	

	long nWidth = rcBrowser.Width();
	//if (IsVerticalScrollBarVisible()) {
		
		//subtract 50 to take care of the scrollbar and some padding
		nWidth -= 40;
	//}

	return nWidth;
}

// (j.gruber 2010-11-30 09:14) - PLID 41653
CString CLabResultsTabDlg::GetStyle()
{

	//TES 2/24/2012 - PLID 44841 - Split GetBrowserWidth() into its own function
	long nWidth = GetBrowserWidth();

	CString strStyle;

	CString strTemp;


	strStyle +=  "<style>\r\n";

	// (j.gruber 2010-12-08 11:22) - PLID 41759	
	strStyle += "\tH1\r\n";
	strStyle += "\t{\r\n";
	strStyle += "\t\ttext-align: center;\r\n";
	strStyle += "\t\tcolor:green;\r\n";		
	strStyle += "\t}\r\n";

	//TES 9/10/2013 - PLID 58511 - We'll use H2 to warn on replaced records
	strStyle += "\tH2\r\n";
	strStyle += "\t{\r\n";
	strStyle += "\t\ttext-align: center;\r\n";
	strStyle += "\t\tcolor:red;\r\n";		
	strStyle += "\t}\r\n";

	strStyle += "\t.FullTable\r\n";
	strStyle += "\t{\r\n";
	strTemp.Format("\t\twidth: %lipx;\r\n", nWidth);
	strStyle += strTemp; 
	strStyle += "\t\tborder: 1px;\r\n";
	strStyle += "\t}\r\n";
	
	strStyle += "\t.InnerRow\r\n";
	strStyle += "\t{\r\n";
	strTemp.Format("\t\twidth: %lipx;\r\n", nWidth);
	strStyle += strTemp;
	strStyle += "\t}\r\n";
	
	//TES 2/24/2012 - PLID 44841 - Find out how many columns we need, and thus the width of each column
	long nColumns = GetColumnCount();
	int nColumnWidth = (nColumns==0)?0:nWidth/nColumns;

	strStyle += "\ttable\r\n";
	strStyle += "\t{\r\n";
	strStyle += FormatString("\t\twidth: %lipx;\r\n", nWidth);
	strStyle += "\t}\r\n";

	strStyle += "\ttd\r\n";
	strStyle += "\t{\r\n";
	strStyle += "\t\tvertical-align:top;\r\n";
	strStyle += FormatString("\t\twidth: %li px;\r\n", nColumnWidth);
	strStyle += "\t}\r\n";

	//TES 2/24/2012 - PLID 44841 - We don't use the HalfInnerCell styles any more
	
	strStyle += "\t.InnerCell\r\n";
	strStyle += "\t{\r\n";
	strTemp.Format("\t\twidth: %lipx;\r\n", nWidth);
	strStyle += strTemp;	
	strStyle += "float: center;\r\n";	
	strStyle += "padding-top: 5px;\r\n";	
	strStyle += "colspan: 2;\r\n";
	strStyle += "\t}\r\n";

	strStyle += "\t.InnerCellWithBorder\r\n";
	strStyle += "\t{\r\n";
	strTemp.Format("\t\twidth: %lipx;\r\n", nWidth);
	strStyle += strTemp;
	strStyle += "padding-top: 5px;\r\n";	
	strStyle += "border-bottom: solid black;\r\n";
	strStyle += "float: center;\r\n";	
	strStyle += "colspan: 2;\r\n";
	strStyle += "\t}\r\n";
	
	strStyle += "\t.MinorTitle\r\n";
	strStyle += "\t{\r\n";
	strStyle += "\t\tfont-family: Verdana;\r\n";
	strStyle += "\t\tfont-size: 14px;\r\n";
	strStyle += "\t\tcolor: blue;\r\n";
	strStyle += "\t}\r\n";
	
	// (r.gonet 03/07/2013) - PLID 44465 - Depending on the setting, use either a monospaced font like Courier New or a proportial font like Verdana for the values.
	CString strMinorInfoFontName = GetRemotePropertyInt("LabReportViewFontType", (long)lrvftProportional) == lrvftMonospaced ? "Courier New" : "Verdana";
	strStyle += "\t.MinorInfo\r\n";
	strStyle += "\t{\r\n";
	strStyle += FormatString("\t\tfont-family: '%s';\r\n", strMinorInfoFontName);
	strStyle += "\t\tfont-size: 14px;\r\n";
	strStyle += "\t\tcolor: black;\r\n";
	strStyle += "\t}\r\n";
	
	strStyle += "</style>	\r\n";

	return strStyle;

}

// (j.gruber 2010-11-30 09:14) - PLID 41606
NXDATALIST2Lib::IRowSettingsPtr CLabResultsTabDlg::GetSpecFromCurrentRow(NXDATALIST2Lib::IRowSettingsPtr pRow) 
{
	//we are given a row, we need to send back its Specimen Row
	//we lucked out and only the specimen rows have values in lrtcSpecimen, so we can just check for that
	//first, let's check out our Specimen for this row
	if (pRow) {
		_variant_t varSpecimen = pRow->GetValue(lrtcSpecimen);
		if (varSpecimen.vt == VT_BSTR) {
			//we are on the specimen row
			return pRow;
		}
		else {
			//let's look at the parent
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if (pParentRow) {
				varSpecimen = pParentRow->GetValue(lrtcSpecimen);
				if (varSpecimen.vt == VT_BSTR) {
					//we found it, return!
					return pParentRow;
				}
				else {
					//look at the grandparent
					NXDATALIST2Lib::IRowSettingsPtr pGRow = pParentRow->GetParentRow();
					if (pGRow) {
						varSpecimen = pGRow->GetValue(lrtcSpecimen);
						if (varSpecimen.vt == VT_BSTR) {
							return pGRow;
						}
						else {
							//we should have found it by now
							ASSERT(FALSE);
							return NULL;
						}
					}
				}
			}
		}	
	}

	//if we got here, we failed
	return NULL;
}

// (j.gruber 2010-11-30 09:11) - PLID 41607
void CLabResultsTabDlg::OnBnClickedScrollLeft()
{
	// (j.gruber 2011-02-18 14:40) - PLID 41606, changed it to be the reverse of the tree
	// (d.singleton 2012-09-19 14:31) - PLID 42596, changed the way the tree loads so needed to change this back to not be reverse of tree
	try {
		
		//first get the specimen row that we are currently on
		IRowSettingsPtr pRow = m_pResultsTree->CurSel;

		NXDATALIST2Lib::IRowSettingsPtr pSpecRow = NULL;
		if (pRow) {
			pSpecRow = GetSpecFromCurrentRow(pRow);

			if (pSpecRow) {

				//go to its previous sibling
				NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pSpecRow->GetPreviousRow();
				if (pPrevRow) {
					m_pResultsTree->CurSel = pPrevRow;
					//we'll need to reload the row
					LoadResult(pPrevRow);
					//we are good to go, so refresh the report
					LoadHTMLReport(pPrevRow);

					//now update our buttons
					UpdateScrollButtons();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
	
}

// (j.gruber 2010-11-30 09:11) - PLID 41607
void CLabResultsTabDlg::OnBnClickedScrollRight()
{
	// (j.gruber 2011-02-18 14:40) - PLID 41606, changed it to be the reverse of the tree
	// (d.singleton 2012-09-19 14:31) - PLID 42596, changed the way the tree loads so needed to change this back to not be reverse of tree
	try {

		//first get the specimen row that we are currently on
		IRowSettingsPtr pRow = m_pResultsTree->CurSel;

		NXDATALIST2Lib::IRowSettingsPtr pSpecRow = NULL;
		if (pRow) {
			pSpecRow = GetSpecFromCurrentRow(pRow);

			if (pSpecRow) {

				//go to its previous sibling
				NXDATALIST2Lib::IRowSettingsPtr pNextRow = pSpecRow->GetNextRow();
				if (pNextRow) {
					m_pResultsTree->CurSel = pNextRow;

					//we'll need to reload the row
					LoadResult(pNextRow);

					//we are good to go, so refresh the report
					LoadHTMLReport(pNextRow);

					//now update our buttons
					UpdateScrollButtons();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
	
}

// (c.haag 2010-12-10 13:53) - PLID 37372 - Given an arbitrary row, this function will try to get the parent lab row, and
// then the result row out of it if one and only one exists.
IRowSettingsPtr CLabResultsTabDlg::GetFirstAndOnlyLabResultRow(IRowSettingsPtr pRow)
{
	// See if we have a lab row selected
	IRowSettingsPtr pLabRow = GetLabRow(pRow);
	if (NULL == pLabRow) {
		// No lab
		return NULL;
	}
	// If there is a lab, and it has only one result, return it.
	if (NULL != pLabRow->GetFirstChildRow() &&
		NULL == pLabRow->GetFirstChildRow()->GetNextRow())
	{
		return pLabRow->GetFirstChildRow();
	}
	else {
		// No results or multiple results
		return NULL;
	}
}

// (c.haag 2010-12-06 13:48) - PLID 41618 - Handler for the left result scroll button in the PDF view
void CLabResultsTabDlg::OnBnClickedResultScrollLeft()
{
	try {
		if(m_nCurrentView == rvPDF)
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnBnClickedResultScroll(-1);
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnBnClickedResultScrollLeft without a valid attachment view!");
			}
		} else {
			ThrowNxException("Called CLabResultsTabDlg::OnBnClickedResultScrollLeft outside the attachment view!");
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-12-06 13:48) - PLID 41618 - Handler for the right result scroll button in the PDF view
void CLabResultsTabDlg::OnBnClickedResultScrollRight()
{
	try {
		if(m_nCurrentView == rvPDF)
		{
			if (NULL != m_pLabResultsAttachmentView) {
				m_pLabResultsAttachmentView->OnBnClickedResultScroll(1);
			} else {
				ThrowNxException("Called CLabResultsTabDlg::OnBnClickedResultScrollRight without a valid attachment view!");
			}
		} else {
			ThrowNxException("Called CLabResultsTabDlg::OnBnClickedResultScrollRight outside the attachment view!");
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-11-30 09:11) - PLID 41607
void CLabResultsTabDlg::UpdateScrollButtons()
{
	try {

		//first get the specimen row that we are currently on
		IRowSettingsPtr pRow = m_pResultsTree->CurSel;

		NXDATALIST2Lib::IRowSettingsPtr pSpecRow = NULL;
		if (pRow) {
			pSpecRow = GetSpecFromCurrentRow(pRow);

			if (pSpecRow) {

				//is there a previous Spec Row
				NXDATALIST2Lib::IRowSettingsPtr pTempRow = pSpecRow->GetPreviousRow();
				if (pTempRow) {
					//enable the button
					// (j.gruber 2011-02-18 14:42) - PLID 41606 - reverse it frm the tree
					// (d.singleton 2012-09-19 14:31) - PLID 42596 changed the way the tree loads so needed to change this back to not be reverse of tree
					GetDlgItem(IDC_SCROLL_LEFT)->EnableWindow(TRUE);
				}
				else {
					GetDlgItem(IDC_SCROLL_LEFT)->EnableWindow(FALSE);
				}

				//now check the next row
				pTempRow = pSpecRow->GetNextRow();
				if (pTempRow) {
					//enable the button
					GetDlgItem(IDC_SCROLL_RIGHT)->EnableWindow(TRUE);
				}
				else {
					GetDlgItem(IDC_SCROLL_RIGHT)->EnableWindow(FALSE);
				}
			}
		}

	}NxCatchAll(__FUNCTION__);

}

// (c.haag 2011-02-22) - PLID 41618 - All detach/filename button toggles are now centralized here
void CLabResultsTabDlg::UpdateDetachButton()
{
	BOOL bInReportView = ((CButton *)GetDlgItem(IDC_REPORTVIEW_RADIO))->GetCheck();
	BOOL bInPDFView = ((CButton *)GetDlgItem(IDC_PDFVIEW_RADIO))->GetCheck();	

	if (bInReportView)
	{
		// These should not be visible in the report view
		GetDlgItem(IDC_ATTACH_FILE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DETACH_FILE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DOC_PATH_LINK)->ShowWindow(SW_HIDE);
	}
	else if (bInPDFView)
	{
		// The attachment view handles this
	}
	else 
	{
		// First, update just the document hyperlink
		if (!m_strCurrentFileName.IsEmpty())
		{
			GetDlgItem(IDC_DOC_PATH_LINK)->ShowWindow(SW_SHOW);
			m_nxlDocPath.SetText(GetFileName(m_strCurrentFileName));
			m_nxlDocPath.SetType(dtsHyperlink);
			InvalidateDlgItem(IDC_DOC_PATH_LINK);
		}
		else {
			GetDlgItem(IDC_DOC_PATH_LINK)->ShowWindow(SW_HIDE);
		}

		// Now update the button
		IRowSettingsPtr pResultRow = GetResultRow(m_pResultsTree->CurSel);
		if (NULL != pResultRow)
		{
			// We have a result row. Get its attachment.
			long nMailID = VarLong(GetTreeValue(pResultRow, lrfValue, lrtcForeignKeyID),-1);

			// If this is true, then we have an attachment, so show the detach button
			if(-1 != nMailID) 
			{
				// (z.manning 2010-05-12 12:26) - PLID 37400 - We allow them to attach files to read-only results
				// but not detach files as it may have came from HL7.
				BOOL bEnableResults = TRUE;
				if(VarLong(pResultRow->GetValue(lrtcHL7MessageID), -1) != -1 && !CheckCurrentUserPermissions(bioPatientLabs, sptDynamic1, FALSE, 0, TRUE)) 
				{
					bEnableResults = FALSE;
				}
				EnableResultWnd(bEnableResults, GetDlgItem(IDC_DETACH_FILE));
				GetDlgItem(IDC_ATTACH_FILE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DETACH_FILE)->ShowWindow(SW_SHOW);
			}
			// If this code block is called, the result has no attachment
			else 
			{
				GetDlgItem(IDC_ATTACH_FILE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_DETACH_FILE)->ShowWindow(SW_HIDE);				
			}
		}
		else
		{
			// If we have no result selected, we can't attach or detach anything
			GetDlgItem(IDC_ATTACH_FILE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DETACH_FILE)->ShowWindow(SW_HIDE);
		}
	}
}

// (c.haag 2011-02-22) - PLID 41618 - All zoom button toggles are now centralized here
void CLabResultsTabDlg::UpdateZoomButton()
{
	if (!m_strCurrentFileName.IsEmpty())
	{
		m_btnZoom.EnableWindow(TRUE);
	}
	else
	{
		m_btnZoom.EnableWindow(FALSE);
	}
}

// (j.gruber 2010-11-30 09:14) - PLID 41607
void CLabResultsTabDlg::RefreshHTMLReportOnlyControls(BOOL bOnReportView) {

	//we only want to show the scroll buttons on the report view and we want to hide the zoom
	m_btnZoom.ShowWindow(!bOnReportView);
	m_btnScrollLeft.ShowWindow(bOnReportView);
	m_btnScrollRight.ShowWindow(bOnReportView);			
}

// (j.gruber 2010-11-30 09:14) - PLID 41606
void CLabResultsTabDlg::UnloadHTMLReports() {

	try {

		//unload the pdf viewer
		COleVariant varUrl("about:blank");
		if (m_pBrowser) {
			m_pBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
			// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
			m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
		}		

		//go through our array and delete any files we created
		for (int i = m_straryLabReports.GetSize() - 1; i >= 0; i--) {
			if (DeleteFile(m_straryLabReports.GetAt(i))) {
				m_straryLabReports.RemoveAt(i);
			}
		}

	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2010-11-30 09:14) - PLID 41606
void CLabResultsTabDlg::ReloadCurrentPDF() {

	CString strFileName = m_strCurrentFileName;
	if (strFileName.IsEmpty()) {
		strFileName = "about:blank";
	}
	//TES 11/23/2009 - PLID 36192 - If this is a .pdf file, we want to append #toolbar=0, to hide the .pdf controls.
	if(strFileName.Right(4).CompareNoCase(".pdf") == 0) {
		strFileName += "#toolbar=0";
	}
	COleVariant varUrl(strFileName);
	// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
	m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
	GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE);

}

// (c.haag 2010-12-15 16:34) - PLID 41825 - If the lab is completed, this returns the user who completed it.
// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
// because it may come in handy someday and may even help developers better understand the lab code.
/*BOOL CLabResultsTabDlg::IsLabCompleted(long nLabID, OUT long& nCompletedBy, COleDateTime& dtCompletedDate)
{
	IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
	if (NULL == pLabRow) {
		return FALSE; // No lab, no results
	}
	IRowSettingsPtr pResultRow = pLabRow->GetFirstChildRow();
	if (NULL == pResultRow) {
		return FALSE; // No results
	}

	nCompletedBy = -1;
	while (pResultRow) {
		long nUserID;
		COleDateTime dt;
		if (-1 == (nUserID = VarLong(pResultRow->GetValue(lrtcCompletedBy), -1))) {
			return FALSE; // Result is not complete
		} else {
			// Result was completed
			dt = VarDateTime(pResultRow->GetValue(lrtcCompletedDate));
			if (-1 == nCompletedBy || (nCompletedBy > -1 && dt > dtCompletedDate)) {
				// If we get here, either this is the first result or this result has a more recent completed
				// date, so update the outputs.
				dtCompletedDate = dt;
				nCompletedBy = nUserID;				
			}

		}
		pResultRow = pResultRow->GetNextRow();
	}
	// At least one result exists and is marked complete
	return TRUE;
}*/

// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if this lab has any unsigned results
// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
// because it may come in handy someday and may even help developers better understand the lab code.
/*BOOL CLabResultsTabDlg::LabHasUnsignedResults(long nLabID)
{
	IRowSettingsPtr pLabRow = GetLabRowByID(nLabID);
	if (NULL == pLabRow) {
		return FALSE; // No lab, no signatures
	}
	IRowSettingsPtr pResultRow = pLabRow->GetFirstChildRow();
	while (pResultRow) {
		if (pResultRow->GetValue(lrtcSignatureInkData).vt == VT_NULL) {
			return TRUE; // This result is unsigned
		}
		pResultRow = pResultRow->GetNextRow();
	}
	return FALSE; // The lab has no results or all results are signed
}*/

// (j.dinatale 2010-12-22) - PLID 41591 - Event handle for the view notes button
void CLabResultsTabDlg::OnViewNotes()
{
	try {
		// set up a notes dialog with no auto added note
		SetUpNotesDlg(false);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-12-22) - PLID 41591 - Event handle for the add note button
void CLabResultsTabDlg::OnAddNote()
{
	try {
		// set up a notes dialog with an auto added note
		SetUpNotesDlg(true);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-12-23) - PLID 41591 - utility function to set up the notes dialog accordingly
void CLabResultsTabDlg::SetUpNotesDlg(bool bAutoAddNewNote)
{
	// if in discrete view
	if (m_nCurrentView == rvDiscreteValues)
	{
		// try to get a result row
		IRowSettingsPtr pResultRow = GetResultRow(m_pResultsTree->CurSel);

		if(pResultRow){
			// if we got a result row, show the results form of the notes dlg
			ShowNotesDlgForResult(pResultRow, bAutoAddNewNote);
		}else{
			// otherwise try and find the lab row
			IRowSettingsPtr pLabRow = GetLabRow(m_pResultsTree->CurSel);

			if(pLabRow){
				// if we got the lab row, try and show the lab notes form of the notes dlg
				ShowNotesDlgForLab(pLabRow, bAutoAddNewNote);
			}
		}
	}
	// if we are in the report view
	else if (m_nCurrentView == rvNexTechReport)
	{
		// try and get the lab row
		IRowSettingsPtr pLabRow = GetLabRow(m_pResultsTree->CurSel);

		if(pLabRow){
			// if we got the row, show the lab notes form of the notes dlg
			ShowNotesDlgForLab(pLabRow, bAutoAddNewNote);
		}
	}
	// else we are in pdf view
	else if (m_nCurrentView == rvPDF)
	{
		// (c.haag 2011-02-22) - PLID 41618 - Forward to the attachments view
		if (NULL == m_pLabResultsAttachmentView) {
			ThrowNxException("Called CLabResultsTabDlg::SetUpNotesDlg without a valid attachment view!");
		} else {
			m_pLabResultsAttachmentView->SetUpNotesDlg(bAutoAddNewNote);
		}
	}
}

// (j.dinatale 2010-12-22) - PLID 41591 - Function to display the notesdlg configured for a result
void CLabResultsTabDlg::ShowNotesDlgForResult(IRowSettingsPtr pResultRow, bool bAutoAddNewNote)
{
	try{
		// check if we have a valid row passed
		if(pResultRow){

			// we are going to force a save, we cannot have notes added to a lab and then the user hitting cancel
			if(!m_pLabEntryDlg->Save()){
				return;
			}

			// grab the result ID
			long nResultID = VarLong(pResultRow->GetValue(lrtcResultID), -1);

			// construct the notes dlg
			CNotesDlg dlgLabNotes(this);

			// assign it a lab result ID, a the patient ID, tell it that its displaying lab notes, and the color to use
			dlgLabNotes.SetPersonID(m_nPatientID);
			dlgLabNotes.m_nLabResultID = nResultID;
			dlgLabNotes.m_bIsLabNotes = true;
			dlgLabNotes.m_clrLabNote = ((CNxColor *)GetDlgItem(IDC_LABRESULTSVIEW_NXCOLORCTRL))->GetColor();

			// now attempt to get a lab row from the result row
			IRowSettingsPtr pLabRow = GetLabRow(pResultRow);
			CString strPrependToNote = "";

			// if we got a row...
			if(pLabRow){

				// get the lab ID
				long nLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID), -1);

				// if the id is not -1, go ahead and assign the lab ID to the notes dlg
				if(nLabID != -1){
					dlgLabNotes.m_nLabID = nLabID;
				}

				// also attempt to get the lab name
				CString strLabName = VarString(pLabRow->GetValue(lrtcValue), "");

				// if we have a lab name, we want to have it as part of the prepended text
				if(!strLabName.IsEmpty()){
					strPrependToNote +=  "(Form # " + strLabName + ")";
				}
			}

			// attempt to get the result name
			CString strResultName = VarString(GetTreeValue(pResultRow, lrfName, lrtcValue), "");

			// check if we have a result name, if so, added it to the prepend text in the proper formatting
			if(!strResultName.IsEmpty()){
				if(!strPrependToNote.IsEmpty()){
					strPrependToNote += ": ";
				}

				strPrependToNote += strResultName;
			}

			// give it to the notes dialog
			dlgLabNotes.m_strPrependText = strPrependToNote;

			// (j.dinatale 2011-08-12 15:00) - PLID 44861 - we were passing in a user name, that shouldnt happen because this is a global preference!
			// (j.dinatale 2011-01-03) - PLID 41966 - provide a category override
			dlgLabNotes.m_nCategoryIDOverride = GetRemotePropertyInt("LabNotesDefaultCategory",-1,0,"<None>",true);

			// tell the dialog whether or not it should have a new note ready to be typed into
			dlgLabNotes.m_bAutoAddNoteOnShow = bAutoAddNewNote;

			// display it as a modal dialog
			CNxModalParentDlg dlg(this, &dlgLabNotes, CString("Lab Result Notes: " + strPrependToNote));
			dlg.DoModal();

			// format the note buttons afterwards
			FormatNotesButtonsForResult(pResultRow);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-12-22) - PLID 41591 - Function to display the notes for an entire lab/specimen
void CLabResultsTabDlg::ShowNotesDlgForLab(IRowSettingsPtr pLabRow, bool bAutoAddNewNote)
{
	try{
		// if a row was passed in
		if(pLabRow){

			// we are going to prepend text
			CString strPrependToNote = "";

			// we are going to force a save, we cannot have notes added to a lab and then the user hitting cancel
			if(!m_pLabEntryDlg->Save()){
				return;
			}

			// get the lab ID
			long nLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID), -1);

			// get the lab name
			CString strLabName = VarString(pLabRow->GetValue(lrtcValue), "");

			// if the lab name is not empty, go ahead and added it to our prepended text
			if(!strLabName.IsEmpty()){
				strPrependToNote +=  "(Form # " + strLabName + ")";
			}

			// create the notes dialog
			CNotesDlg dlgLabNotes(this);

			// set the person ID, lab ID, let the dialog know its for a lab and assign the color. Pass in the prepend text too.
			dlgLabNotes.SetPersonID(m_nPatientID);
			dlgLabNotes.m_nLabID = nLabID;
			dlgLabNotes.m_bIsLabNotes = true;
			dlgLabNotes.m_clrLabNote = ((CNxColor *)GetDlgItem(IDC_LABRESULTSVIEW_NXCOLORCTRL))->GetColor();
			dlgLabNotes.m_strPrependText = strPrependToNote;

			// (j.dinatale 2011-08-12 15:00) - PLID 44861 - we were passing in a user name, that shouldnt happen because this is a global preference!
			// (j.dinatale 2011-01-03) - PLID 41966 - provide a category override
			dlgLabNotes.m_nCategoryIDOverride = GetRemotePropertyInt("LabNotesDefaultCategory",-1,0,"<None>",true);

			// tell the dialog whether or not it should have a new note ready to be typed into
			dlgLabNotes.m_bAutoAddNoteOnShow = bAutoAddNewNote;

			// display it as a modal dialog
			CNxModalParentDlg dlg(this, &dlgLabNotes, CString("Lab Specimen Notes: " + strPrependToNote));
			dlg.DoModal();

			// format the note buttons afterwards
			FormatNotesButtonsForLab(pLabRow);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-12-22) - PLID 41591 - Function to get the result row an attachment or file is attached to
NXDATALIST2Lib::IRowSettingsPtr CLabResultsTabDlg::GetResultRowFromAttachment(CString strFullFilePath)
{
	try{
		// if the file path is empty, then just return NULL
		if(strFullFilePath.IsEmpty()){
			return NULL;
		}

		IRowSettingsPtr pResultRowOfFile = NULL;

		// loop through each key
		POSITION posMain = m_mapAttachedFiles.GetStartPosition();
		while(posMain != NULL)
		{
			// get the associated items for the key one by one
			LPDISPATCH lpKey;
			CMap<LPDISPATCH,LPDISPATCH,CString,CString&>* pmapFiles;
			m_mapAttachedFiles.GetNextAssoc(posMain, lpKey, pmapFiles);

			POSITION posTemp = pmapFiles->GetStartPosition();
			while(posTemp != NULL)
			{
				CString strFile;
				pmapFiles->GetNextAssoc(posTemp, lpKey, strFile);

				CString strFullPath;

				// make sure we are looking at the full path of the file, the map stores file names sometimes
				if(strFile.Find("\\") == -1){
					strFullPath = GetPatientDocumentPath(m_nPatientID) ^ strFile;
				}else{
					strFullPath = strFile;
				}

				// if the file path is the same as the one passed in, this is the row we want
				if(strFullFilePath == strFullPath) {
					return (NXDATALIST2Lib::IRowSettingsPtr) lpKey;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);

	// if an exception happens, or we dont find anything, return null
	return NULL;
}

// (j.dinatale 2011-01-03) - PLID 41591 - added exception handling just in case
// (j.dinatale 2010-12-27) - PLID 41591 - Function to format buttons based on a result row in the result tree
void CLabResultsTabDlg::FormatNotesButtonsForResult(NXDATALIST2Lib::IRowSettingsPtr pResultRow)
{
	try{
		if(pResultRow){

			// grab the result ID
			long nResultID = VarLong(pResultRow->GetValue(lrtcResultID), -1);

			// check if there are any result notes
			if(nResultID > 0 && ReturnsRecordsParam("SELECT 1 FROM Notes WHERE LabResultID = {INT}", nResultID)){
				// if they are, the text of the view notes button should be red and have the red notes icon (like the billing tab)
				m_btnNotes.AutoSet(NXB_EXTRANOTES);
				m_btnNotes.SetTextColor(RGB(255, 0, 0));
			}else{
				// otherwise, the normal note icon and black text
				m_btnNotes.AutoSet(NXB_NOTES);
				m_btnNotes.SetTextColor(RGB(0, 0, 0));
			}

			// both buttons should be enabled
			GetDlgItem(IDC_LABNOTES)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADDLABNOTE)->EnableWindow(TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-01-03) - PLID 41591 - added exception handling just in case
// (j.dinatale 2010-12-27) - PLID 41591 - function to disable the note buttons
void CLabResultsTabDlg::DisableNotesButtons()
{
	try{
		// set the button to have a normal note icon, black text and disable both note buttons
		m_btnNotes.AutoSet(NXB_NOTES);
		m_btnNotes.SetTextColor(RGB(0, 0, 0));
		GetDlgItem(IDC_LABNOTES)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADDLABNOTE)->EnableWindow(FALSE);
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-01-03) - PLID 41591 - added exception handling just in case
// (j.dinatale 2010-12-27) - PLID 41591 - Function to format buttons based on a lab row in the result tree
void CLabResultsTabDlg::FormatNotesButtonsForLab(NXDATALIST2Lib::IRowSettingsPtr pLabRow)
{
	try{
		if(pLabRow){
			// get the lab ID
			long nLabID = VarLong(pLabRow->GetValue(lrtcForeignKeyID), -1);

			// check and see if there are any notes for the lab
			if(nLabID > 0 && ReturnsRecordsParam("SELECT 1 FROM Notes WHERE LabID = {INT}", nLabID)){
				// if they are, the text of the view notes button should be red and have the red notes icon (like the billing tab)
				m_btnNotes.AutoSet(NXB_EXTRANOTES);
				m_btnNotes.SetTextColor(RGB(255, 0, 0));
			}else{
				// otherwise, the normal note icon and black text
				m_btnNotes.AutoSet(NXB_NOTES);
				m_btnNotes.SetTextColor(RGB(0, 0, 0));
			}

			// both buttons should be enabled
			GetDlgItem(IDC_LABNOTES)->EnableWindow(TRUE);
			GetDlgItem(IDC_ADDLABNOTE)->EnableWindow(TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-12-27) - PLID 41591 - Function to format buttons based on any row in the result tree
void CLabResultsTabDlg::FormatNotesButtons(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		// if in discrete view
		if(m_nCurrentView == rvDiscreteValues){

			// try to get a result row
			IRowSettingsPtr pResultRow = GetResultRow(pRow);

			if(pResultRow){
				// if we got a result row, format note buttons according to that result
				FormatNotesButtonsForResult(pResultRow);
			}else{
				// (j.dinatale 2011-02-28) - PLID 41591 - should be getting the lab row corresponding to the row passed
				//	in, not the current selection
				// otherwise try and find the lab row
				IRowSettingsPtr pLabRow = GetLabRow(pRow);

				if(pLabRow){
					// if we got the lab row, format note buttons according to the lab
					FormatNotesButtonsForLab(pLabRow);
				}else{
					// otherwise, assume we are disabling the note buttons
					DisableNotesButtons();
				}
			}
		}else{
			// if we are in the report view
			if(m_nCurrentView == rvNexTechReport){
				// try and get the lab row
				IRowSettingsPtr pLabRow = GetLabRow(pRow);

				if(pLabRow){
					// if we got the lab row, format note buttons according to the lab
					FormatNotesButtonsForLab(pLabRow);
				}else{
					// otherwise assume we are disabling the note buttons
					DisableNotesButtons();
				}
			} 
			else
			{
				// if we are in the pdf viewer
				if(m_nCurrentView == rvPDF)
				{
					// (c.haag 2011-02-23) - PLID 41618 - The attachment view handles this
				}
				else
				{
					// we are not in pdf, report, or discrete views. It is likely the view is not set, so disable the buttons
					DisableNotesButtons();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2011-02-22) - PLID 41618 - Returns attachment information for a given row
CAttachedLabFile CLabResultsTabDlg::GetAttachedLabFile(LPDISPATCH lpRow)
{
	CArray<CAttachedLabFile,CAttachedLabFile&> aAttachedFiles;
	GetAllAttachedFiles(aAttachedFiles);
	for (int i=0; i < aAttachedFiles.GetSize(); i++) 
	{
		if (aAttachedFiles[i].lpRow == lpRow) 
		{
			return aAttachedFiles[i];
		}		
	}
	CAttachedLabFile empty;
	return empty;
}

void CLabResultsTabDlg::OnEditOrderStatus()
{
	try{
		
		//TES 5/2/2011 - PLID 43428 - Added the ability for this dialog to edit Order statuses as well as Result statuses.
		CEditLabStatusDlg dlg(this);
		dlg.m_bEditOrderStatus = true;

		//TES 5/2/2011 - PLID 43428 - Temporarily save the current selection for later
		IRowSettingsPtr pRow = m_pOrderStatusCombo->GetCurSel();
		long nValue = -1;
		if(pRow != NULL) {
			nValue = VarLong(pRow->GetValue(oscID), -1);
		}

		//TES 5/2/2011 - PLID 43428 - Bring up the dialog.
		dlg.DoModal();

		//TES 5/2/2011 - PLID 43428 - Refresh the list, now that it's probably changed.
		m_pOrderStatusCombo->Requery();

		//TES 12/4/2008 - PLID 32191 - Set the selection back to what it was
		m_pOrderStatusCombo->SetSelByColumn(oscID, nValue);

	}NxCatchAll(__FUNCTION__);
}

void CLabResultsTabDlg::OnConfigureReportView()
{
	try {
		//TES 2/24/2012 - PLID 44841 - Pop up the dialog.
		CConfigureReportViewDlg dlg;
		if(IDOK == dlg.DoModal()) {
			//TES 2/24/2012 - PLID 44841 - Reload the Report View
			IRowSettingsPtr pRow = m_pResultsTree->CurSel;
			if (pRow) {
				NXDATALIST2Lib::IRowSettingsPtr pSpecRow = GetSpecFromCurrentRow(pRow);
				if (pSpecRow) {
					LoadHTMLReport(pSpecRow);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//TES 2/24/2012 - PLID 44841 - Determines the actual number of columns that will be displayed in the report view, based on the configuration
// as well as the field values for the current result.
long CLabResultsTabDlg::GetColumnCount()
{
	EnsureResultFieldPositions();
	int nHighestColumn = 0;
	for(int i = 0; i < m_arResultFieldPositions.GetSize(); i++) {
		ResultFieldPosition rfp = m_arResultFieldPositions[i];
		//TES 3/27/2012 - PLID 49209 - Don't count fields we aren't showing
		if(rfp.bShowField && rfp.nActualColumn > nHighestColumn) {
			nHighestColumn = rfp.nActualColumn;
		}
	}
	return nHighestColumn;
}

//TES 2/24/2012 - PLID 44841 - Fills m_arResultFieldPositions from ConfigRT, if necessary.
void CLabResultsTabDlg::EnsureResultFieldPositions()
{
	//TES 2/24/2012 - PLID 44841 - If these are already loaded, we're done.
	if(m_bLoadedResultFieldPositions) {
		return;
	}

	//TES 2/24/2012 - PLID 44841 - Clear out anything that's already in there.
	m_arResultFieldPositions.RemoveAll();

	//TES 2/24/2012 - PLID 44841 - Now load each field out from ConfigRT (it uses the LabResultField enum as the Property number).
	// The default values roughly approximate how the report view looked prior to this change, although the HTML changes were significant,
	// so the appearance isn't precisely identical.
	ResultFieldPosition rfp;
	rfp.lrf = lrfName;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 1, lrfName, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfName, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfName, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfSlideNum;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 1, lrfSlideNum, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfSlideNum, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSlideNum, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfDateReceived;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 2, lrfDateReceived, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDateReceived, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfDateReceived, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfLOINC;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 2, lrfLOINC, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfLOINC, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfLOINC, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfDateReceivedByLab;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 3, lrfDateReceivedByLab, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDateReceivedByLab, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfDateReceivedByLab, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfDatePerformed;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 4, lrfDatePerformed, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDatePerformed, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfDatePerformed, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfStatus;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 5, lrfStatus, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfStatus, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfStatus, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfUnits;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 5, lrfUnits, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfUnits, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfUnits, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfFlag;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 6, lrfFlag, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfFlag, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfFlag, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfReference;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 6, lrfReference, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfReference, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfReference, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfAcknowledgedBy;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 7, lrfAcknowledgedBy, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfAcknowledgedBy, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfAcknowledgedBy, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfDiagnosis;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 8, lrfDiagnosis, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfDiagnosis, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfDiagnosis, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfMicroscopicDescription;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 8, lrfMicroscopicDescription, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfMicroscopicDescription, "<None>");
	//TES 3/26/2012 - PLID 49208 - Added an option to hide fields entirely
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfMicroscopicDescription, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-06-19 17:21) - PLID 57937 - Update HL7 lab messages to support latest MU requirements. need to display spm segment
	rfp.lrf = lrfSpecimenIdentifier;
	rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 9, lrfSpecimenIdentifier, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfSpecimenIdentifier, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSpecimenIdentifier, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfSpecimenIdText;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 9, lrfSpecimenIdText, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfSpecimenIdText, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSpecimenIdText, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfSpecimenStartTime;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 10, lrfSpecimenStartTime, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfSpecimenStartTime, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSpecimenStartTime, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfSpecimenEndTime;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 10, lrfSpecimenEndTime, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfSpecimenEndTime, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSpecimenEndTime, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfSpecimenRejectReason;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 11, lrfSpecimenRejectReason, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfSpecimenRejectReason, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSpecimenRejectReason, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfSpecimenCondition;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 11, lrfSpecimenCondition, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfSpecimenCondition, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfSpecimenCondition, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-08-07 17:10) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
	rfp.lrf = lrfServiceStartTime;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 12, lrfServiceStartTime, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfServiceStartTime, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfServiceStartTime, "<None>");
	m_arResultFieldPositions.Add(rfp);

	rfp.lrf = lrfServiceEndTime;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 12, lrfServiceEndTime, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfServiceEndTime, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfServiceEndTime, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-08-07 17:11) - PLID 57912 - need to show the Performing Provider on report view of labresult tab dlg
	rfp.lrf = lrfPerformingProvider;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 13, lrfPerformingProvider, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerformingProvider, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerformingProvider, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-11-04 17:09) - PLID 59294 - add observation date to the html view for lab results. 
	rfp.lrf = lrfObservationDate;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 13, lrfObservationDate, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 2, lrfObservationDate, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfObservationDate, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-10-25 13:53) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
	rfp.lrf = lrfPerformingLab;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 14, lrfPerformingLab, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerformingLab, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerformingLab, "<None>");
	m_arResultFieldPositions.Add(rfp);	

	// (d.singleton 2013-11-06 14:52) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	rfp.lrf = lrfPerfLabAddress;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 15, lrfPerfLabAddress, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabAddress, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerfLabAddress, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-11-06 14:52) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	rfp.lrf = lrfPerfLabCity;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 16, lrfPerfLabCity, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabCity, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerfLabCity, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-11-06 14:52) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	rfp.lrf = lrfPerfLabState;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 17, lrfPerfLabState, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabState, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerfLabState, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-11-06 14:52) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	rfp.lrf = lrfPerfLabZip;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 18, lrfPerfLabZip, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabZip, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerfLabZip, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-11-06 14:52) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	rfp.lrf = lrfPerfLabCountry;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 19, lrfPerfLabCountry, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabCountry, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerfLabCountry, "<None>");
	m_arResultFieldPositions.Add(rfp);

	// (d.singleton 2013-11-06 14:52) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
	rfp.lrf = lrfPerfLabParish;
	rfp.nRow = rfp.nRow = GetRemotePropertyInt("LabReportViewFieldRow", 20, lrfPerfLabParish, "<None>");
	rfp.nColumn = GetRemotePropertyInt("LabReportViewFieldColumn", 1, lrfPerfLabParish, "<None>");
	rfp.bShowField = GetRemotePropertyInt("LabReportViewFieldShow", TRUE, lrfPerfLabParish, "<None>");
	m_arResultFieldPositions.Add(rfp);


	//TES 2/24/2012 - PLID 44841 - Now we need to go through and find out which columns are actually used (based on the data for this result).
	CArray<long,long> arUsedColumns;
	for(int i = 0; i < m_arResultFieldPositions.GetSize(); i++) {
		ResultFieldPosition rfpCol = m_arResultFieldPositions[i];
		//TES 3/27/2012 - PLID 49209 - Don't look at fields we aren't showing
			if(rfpCol.bShowField) {
			long nColumn = rfpCol.nColumn;
			bool bUsed = false;
			for(int j = 0; j < arUsedColumns.GetSize() && !bUsed; j++) {
				if(nColumn == arUsedColumns[j]) bUsed = true;
			}
			if(!bUsed) {
				NXDATALIST2Lib::IRowSettingsPtr pSpecRow = m_pResultsTree->GetFirstRow();
				while(pSpecRow && !bUsed) {
					NXDATALIST2Lib::IRowSettingsPtr pResult = pSpecRow->GetFirstChildRow();
					while(pResult && !bUsed) {
						_variant_t varValue = GetTreeValue(pResult, rfpCol.lrf, lrtcValue);
						if(varValue.vt != VT_NULL && varValue.vt != VT_EMPTY) {
							bUsed = true;
						}
						pResult = pResult->GetNextRow();
					}
					pSpecRow = pSpecRow->GetNextRow();
				}
				if(bUsed) {
					bool bInserted = false;
					for(int nUsedCol = 0; nUsedCol < arUsedColumns.GetSize() && !bInserted; nUsedCol++) {
						if(arUsedColumns[nUsedCol] > nColumn) {
							arUsedColumns.InsertAt(nUsedCol, nColumn);
							bInserted = true;
						}
					}
					if(!bInserted) {
						arUsedColumns.Add(nColumn);
					}
				}
			}
		}
	}
	//TES 2/24/2012 - PLID 44841 - Now update the nActualColumn variables to reflect any empty columns which we'll not be displaying.
	for(int i = 0; i < m_arResultFieldPositions.GetSize(); i++) {
		ResultFieldPosition rfp = m_arResultFieldPositions[i];
		for(int j = 0; j < arUsedColumns.GetSize() && rfp.nActualColumn == -1; j++) {
			if(arUsedColumns[j] == rfp.nColumn) {
				rfp.nActualColumn = j+1;
			}
		}
		m_arResultFieldPositions.SetAt(i, rfp);
	}

	//TES 2/24/2012 - PLID 44841 - Now sort the array based on the order in which we're going to output the HTML
	CArray<ResultFieldPosition,ResultFieldPosition&> arSorted;
	for(int i = 0; i < m_arResultFieldPositions.GetSize(); i++) {
		ResultFieldPosition rfp = m_arResultFieldPositions[i];
		bool bInserted = false;
		for(int j = 0; j < arSorted.GetSize() && !bInserted; j++) {
			ResultFieldPosition rfp2 = arSorted[j];
			if(rfp2.nRow > rfp.nRow || (rfp2.nRow == rfp.nRow && rfp2.nColumn > rfp.nColumn)) {
				arSorted.InsertAt(j, rfp);
				bInserted = true;
			}
		}
		if(!bInserted) {
			arSorted.Add(rfp);
		}
	}
	m_arResultFieldPositions.RemoveAll();
	m_arResultFieldPositions.Append(arSorted);

	for(int i = 0; i < m_arResultFieldPositions.GetSize(); i++) {
		TRACE("{%i, %i}\r\n", m_arResultFieldPositions[i].nRow, m_arResultFieldPositions[i].nActualColumn);
	}
}

// (j.dinatale 2013-03-04 12:37) - PLID 34339 - function to attach a new file
void CLabResultsTabDlg::AttachNewFile(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IRowSettingsPtr pParent)
{
	// Code paraphrased from CHistoryDlg::OnClickAttach().
	CString path;
	//DRT 10/28/2003 - PLID 9921 - Added a row for "Commonly Attached" which contains *.doc, *.xls, *.jpg, 
	//*.bmp, *.pcx, *.tiff, *.wav.  This is the default selected item, so that users can quickly add things
	//without changing the file type.
	//	Also changed the description of the other items to include the types they show
	//PLID 18882 - added PDF's to the commonly selected files and their own type
	// (a.walling 2007-07-19 11:25) - PLID 26748 - Added office 2007 files (docx, xlsx) also jpeg
	// (a.walling 2007-09-17 16:11) - PLID 26748 - Also need PowerPoint 2007 files.
	// (a.walling 2007-10-12 14:50) - PLID 26342 - Moved to a shared string, and also include other formats
	//only allow 1 file to be selected
	// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT , szCommonlyAttached, this);

	long nMailID = VarLong(GetTreeValue(pRow, lrfValue, lrtcForeignKeyID), -1);
	long nResultID = VarLong(pRow->GetValue(lrtcResultID));
	CString strInitPath;
	if (nMailID != -1) {
		strInitPath = VarString(GetTreeValue(pRow, lrfValue, lrtcValue), GetPatientDocumentPath(m_nPatientID));
	}
	else {
		strInitPath = GetPatientDocumentPath(m_nPatientID);
	}

	if(!DoesExist(strInitPath)) {
		// This shouldn't be possible because either GetPatientDocumentPath should return a 
		// valid path to a folder that exists, or it should throw an exception
		ASSERT(FALSE);
		AfxThrowNxException("Person document folder '%s' could not be found", path);
		return;
	}

	dlg.m_ofn.lpstrInitialDir = strInitPath;
	//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.
	char * strFile = new char[5000];
	strFile[0] = 0;
	dlg.m_ofn.nMaxFile = 5000;
	dlg.m_ofn.lpstrFile = strFile;

	if (dlg.DoModal() == IDOK) {
		POSITION p = dlg.GetStartPosition();
		if (p) {	

			if (nMailID > 0) {

				//there is already a document attached, so we need to clear it
				m_aryDetachedDocs.Add(nResultID);

				//everything else will be replaced later
			}

			CString strPath = dlg.GetNextPathName(p);
			// (c.haag 2009-05-06 14:54) - PLID 33789 - Units
			SetTreeValue(pRow, lrfUnits, lrtcValue, _variant_t(""));
			SetDlgItemText(IDC_RESULT_UNITS, "");
			SetTreeValue(pRow, lrfValue, lrtcForeignKeyID, (long)-2);
			SetTreeValue(pRow, lrfValue, lrtcValue, _variant_t(strPath));
			// (c.haag 2011-01-27) - PLID 41618 - We always need a date. For now, use the current server
			// date so the attachments are still sorted chronologically
			SetTreeValue(pRow, lrfValue, lrtcDate, _variant_t(GetServerTime(), VT_DATE));

			//we are storing the path in the datalist, but not link
			m_nxlDocPath.SetText(GetFileName(strPath));
			m_nxlDocPath.SetType(dtsHyperlink);

			// (c.haag 2010-12-06 13:48) - PLID 41618 - Should only be showing if we're in the discrete values view
			if (!m_bControlsHidden)
			{
				ShowDlgItem(IDC_DOC_PATH_LINK, SW_SHOW);
				InvalidateDlgItem(IDC_DOC_PATH_LINK);
			}

			//TES 11/23/2009 - PLID 36192 - Remember that this file is attached to this result.
			//TES 1/27/2010 - PLID 36862 - Moved into its own function
			SetAttachedFile((LPDISPATCH)pParent, strPath);
			//TES 11/23/2009 - PLID 36192 - Go ahead and re-load the result, this will update the browser and other controls.
			SaveResult(pRow);
			LoadResult(pRow);
		}
	}
	// (a.walling 2008-10-17 11:24) - PLID 31724 - Clear the memory of the file buffer
	delete [] strFile;
}

// (j.dinatale 2013-03-04 12:37) - PLID 34339 - function to attach an existing file
void CLabResultsTabDlg::AttachExistingFile(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IRowSettingsPtr pParent)
{
	long nMailID = VarLong(GetTreeValue(pRow, lrfValue, lrtcForeignKeyID), -1);
	long nResultID = VarLong(pRow->GetValue(lrtcResultID));

	//1. set up the fields we are interested in having
	CStringArray aryColumns;
	aryColumns.Add("MailSent.MailID");
	aryColumns.Add("MailSent.ServiceDate");
	aryColumns.Add("MailSentNotesT.Note");
	aryColumns.Add("CatsSubQ.Description");
	aryColumns.Add("MailSent.PathName");

	//2. Column Headers (order matters)
	CStringArray aryColumnHeaders;
	aryColumnHeaders.Add("Mail ID");
	aryColumnHeaders.Add("Service Date");
	aryColumnHeaders.Add("Description");
	aryColumnHeaders.Add("Category");
	aryColumnHeaders.Add("File Name");

	//3. Sort order for columns (order matters)
	CSimpleArray<short> arySortOrder;
	arySortOrder.Add(-1);
	arySortOrder.Add(0);
	arySortOrder.Add(1);
	arySortOrder.Add(2);
	arySortOrder.Add(3);

	CString strWhere;
	strWhere.Format(" MailSent.PersonID = %li AND InternalRefID IS NULL AND MailSent.Selection <> '' AND MailSent.Selection <> 'BITMAP:FOLDER' ", m_nPatientID);

	// Open the single select dialog
	CSingleSelectMultiColumnDlg dlg(this);
	HRESULT hr = dlg.Open(
		" MailSent LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN ( "
		"	SELECT ID, Description "
		"	FROM NoteCatsF "
		"	WHERE IsPatientTab = 1 "
		" ) CatsSubQ ON MailSent.CategoryID = CatsSubQ.ID ",								/*From*/
		strWhere,																			/*Where*/
		aryColumns,																			/*Select*/
		aryColumnHeaders,																	/*Column Names*/
		arySortOrder,																		/*Sort Order*/
		"[1] - [2]",																		/*Display Columns*/
		"Please select an existing file to attach to this lab.",							/*Description*/
		"Select an Existing File"															/*Title Bar Header*/
		);

	// If they clicked OK, be sure to check if they made a valid selection
	if(hr == IDOK){
		CVariantArray varySelectedValues;
		dlg.GetSelectedValues(varySelectedValues);

		if(!varySelectedValues.GetSize()){
			this->MessageBox("You did not select an existing file to attach. Please try again.", "No File Selected", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		long nNewMailID = VarLong(varySelectedValues.GetAt(0), -1);
		CString strFileName = VarString(varySelectedValues.GetAt(4), "");

		// if we didnt get a valid mailID, somehow it was equal to the one we had, or if the file name is empty, bail
		if(nNewMailID <= 0 || nNewMailID == nMailID || strFileName.IsEmpty()){
			return;
		}

		CString strPatDocPath = GetPatientDocumentPath(m_nPatientID);
		if(!FileUtils::EnsureDirectory(strPatDocPath)) {
			// This shouldn't be possible because either GetPatientDocumentPath should return a 
			// valid path to a folder that exists, or it should throw an exception
			ASSERT(FALSE);
			AfxThrowNxException("Person document folder '%s' could not be found", strPatDocPath);
			return;
		}

		if (nMailID > 0) {
			m_aryDetachedDocs.Add(nResultID);
		}

		strFileName = strPatDocPath ^ strFileName;
		SetTreeValue(pRow, lrfUnits, lrtcValue, _variant_t(""));
		SetDlgItemText(IDC_RESULT_UNITS, "");
		SetTreeValue(pRow, lrfValue, lrtcForeignKeyID, (long)nNewMailID);
		SetTreeValue(pRow, lrfValue, lrtcValue, _variant_t(GetFileName(strFileName)));
		SetTreeValue(pRow, lrfValue, lrtcDate, _variant_t(GetServerTime(), VT_DATE));

		//we are storing the path in the datalist, but not link
		m_nxlDocPath.SetText(GetFileName(strFileName));
		m_nxlDocPath.SetType(dtsHyperlink);

		if (!m_bControlsHidden){
			ShowDlgItem(IDC_DOC_PATH_LINK, SW_SHOW);
			InvalidateDlgItem(IDC_DOC_PATH_LINK);
		}

		SetAttachedFile((LPDISPATCH)pParent, strFileName);
		SaveResult(pRow);
		LoadResult(pRow);
	}
}

// (j.luckoski 2013-03-20 11:39) - PLID 55424 - Mark all results ack, signed, complete in one step.
void CLabResultsTabDlg::OnBnClickedLabMarkAllComplete()
{
	try {
		// (b.cardillo 2013-08-07 12:21) - PLID 57903 - Can't allow signing or completing if you don't have the necessary permission
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic3) || !CheckCurrentUserPermissions(bioPatientLabs, sptDynamic0)) {
			return;
		}

		// (f.gelderloos 2013-08-26 10:31) - PLID 57826 Ack, sign, and complete the labs when this button is pressed.
		IRowSettingsPtr pActiveRow = m_pResultsTree->CurSel;
		long nLabID = -1;
		{
			IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
			IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
			nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
		}
		
		AcknowledgeResults(m_pResultsTree->CurSel, true);
		if (AllResultsAreAcknowledged(nLabID))	SignResults(m_pResultsTree->CurSel, true);
		if (AllResultsAreSigned(nLabID))	MarkLabCompleted(m_pResultsTree->CurSel,true);
	} NxCatchAll(__FUNCTION__);
}


// (b.spivey, July 12, 2013) - PLID 45194 - borrowed code from another file that used twain, 
//		and then I gutted it of near everything. 
void CLabResultsTabDlg::DoTWAINAcquisition(NXTWAINlib::EScanTargetFormat eScanTargetFormat)
{
	
	//Code paraphrased from CHistoryDlg::OnAcquire()
	if (NXTWAINlib::IsAcquiring())
	{
		MsgBox("Please wait for the current image acquisition to complete before starting a new one.");
		return;
	}

	//Get category ID using the default category
	long nCategoryID = (long)GetRemotePropertyInt("LabAttachmentsDefaultCategory",-1,0,"<None>",true);
	//PatID
	long nPatientID = GetPatientID();
	//And Current row. 
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;

	//Create struct and set pDialog and pRow. 
	m_drsThis->pDialog = this;
	m_drsThis->pRow = pRow;
	
	//This will attach the document to the patient's history for us. 
	NXTWAINlib::Acquire(nPatientID, GetPatientDocumentPath(nPatientID), OnTWAINCallback, OnNxTwainPreCompress, m_drsThis, "", nCategoryID, -1, -1, eScanTargetFormat);

}

// (b.spivey, July 15, 2013) - PLID 45194 - callback function when the scanner is done. 
void WINAPI CALLBACK CLabResultsTabDlg::OnTWAINCallback(NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
		const CString& strFileName, /* The full filename of the document that was scanned */
		BOOL& bAttach, void* pUserData, CxImage& cxImage)
{
	try {
		//The user data is the struct with a pointer to the dialog and the current row. 
		DialogRowStruct* drsThis = (DialogRowStruct*) pUserData;
		if (drsThis) {
			drsThis->pDialog->AttachFileToResult(strFileName, drsThis->pRow);
			// (b.spivey, July 18, 2013) - PLID 45194 - Once we're done, set this to null
			pUserData = NULL; 
		}
	}NxCatchAll_NoParent(__FUNCTION__); // (a.walling 2014-05-05 13:32) - PLID 61945
}

// (b.spivey, July 12, 2013) - PLID 45194 - This code was take from another function that attaches the file 
//	 to the result so that it is saved in memory or in data and when we reopen it will actually be there. 
//	 Left comments in because they're relevent. 
void CLabResultsTabDlg::AttachFileToResult(CString strFilePath, NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// (c.haag 2009-05-06 14:54) - PLID 33789 - Units
	SetTreeValue(pRow, lrfUnits, lrtcValue, _variant_t(""));
	SetDlgItemText(IDC_RESULT_UNITS, "");
	SetTreeValue(pRow, lrfValue, lrtcForeignKeyID, (long)-2);
	SetTreeValue(pRow, lrfValue, lrtcValue, _variant_t(strFilePath));
	// (c.haag 2011-01-27) - PLID 41618 - We always need a date. For now, use the current server
	// date so the attachments are still sorted chronologically
	SetTreeValue(pRow, lrfValue, lrtcDate, _variant_t(GetServerTime(), VT_DATE));

	//we are storing the path in the datalist, but not link
	m_nxlDocPath.SetText(GetFileName(strFilePath));
	m_nxlDocPath.SetType(dtsHyperlink);

	// (c.haag 2010-12-06 13:48) - PLID 41618 - Should only be showing if we're in the discrete values view
	if (!m_bControlsHidden)
	{
		ShowDlgItem(IDC_DOC_PATH_LINK, SW_SHOW);
		InvalidateDlgItem(IDC_DOC_PATH_LINK);
	}

	//TES 11/23/2009 - PLID 36192 - Remember that this file is attached to this result.
	//TES 1/27/2010 - PLID 36862 - Moved into its own function
	SetAttachedFile((LPDISPATCH)GetResultRow(pRow), strFilePath);
	//TES 11/23/2009 - PLID 36192 - Go ahead and re-load the result, this will update the browser and other controls.
	SaveResult(pRow);
	LoadResult(pRow);
}

// (b.spivey, August 23, 2013) - PLID 46295 - Get the anatomical location we display on the patient's lab tab. 
CString CLabResultsTabDlg::GetAnatomicalLocationAsString(long nLabID) 
{
	CString strLocation = "";
	strLocation = m_pLabEntryDlg->GetAnatomicLocationByLabID(nLabID); 

	// (b.spivey, September 05, 2013) - PLID 46295 - Changed to "Anatomic Location"
	return (strLocation.IsEmpty() ? "" : "Anatomic Location: " + strLocation);
}

// (f.gelderloos 2013-08-26 10:31) - PLID 57826
void CLabResultsTabDlg::OnBnClickedLabAcknowledgeandsign()
{
	try {
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic3))
			return;

		// (f.gelderloos 2013-08-26 10:31) - PLID 57826 Ack and Sign the labs when this button is pressed.
		IRowSettingsPtr pActiveRow = m_pResultsTree->CurSel;
		long nLabID = -1;
		{
			IRowSettingsPtr pActiveResultRow = GetResultRow(pActiveRow);
			IRowSettingsPtr pActiveLabRow = GetLabRow(pActiveRow);
			nLabID = (NULL == pActiveLabRow) ? -2 : VarLong(pActiveLabRow->GetValue(lrtcForeignKeyID),-1);
		}
		AcknowledgeResults(m_pResultsTree->CurSel, true);
		if (AllResultsAreAcknowledged(nLabID))	SignResults(m_pResultsTree->CurSel, true);

	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, September 05, 2013) - PLID 46295 - ensure data when we show the window. 
void CLabResultsTabDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		// (b.spivey, August 28, 2013) - PLID 46295 - made it a general function but for now this is to ensure that the labels 
		//	 show the most current anatomical location. 
		if (bShow) {
			EnsureData(); 
		}
		
	}NxCatchAll(__FUNCTION__); 
}

// (j.jones 2013-10-18 11:32) - PLID 58979 - added infobutton
void CLabResultsTabDlg::OnBtnPtEducation()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsTree->CurSel;
		if(pRow == NULL || pRow->GetValue(lrtcResultID).vt == VT_NULL) {
			MessageBox("Patient Education cannot be loaded because no lab result has been selected.", "Practice", MB_ICONINFORMATION|MB_OK);
			return;
		}

		//force a save
		if(!SaveResult(pRow)) {
			return;
		}
		if(!m_pLabEntryDlg->Save()){
			return;
		}

		long nLabResultID = VarLong(pRow->GetValue(lrtcResultID), -1);
		if(nLabResultID == -1) {
			//should not be possible, because we forced a save
			ASSERT(FALSE);
			ThrowNxException("No valid lab result ID was found.");
		}

		//the info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
		LookupMedlinePlusInformationViaURL(this, mlpLabResultID, nLabResultID);

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-10-29 12:08) - PLID 59197 - need to move the specimen segment values to its own header in the html view for labs,  so it will show regardless of any results. 
CString CLabResultsTabDlg::GetSpecimenFieldsHTML(NXDATALIST2Lib::IRowSettingsPtr pSpecRow)
{
	CString strHTML = "";
	if(pSpecRow) {
		CString strFieldName, strDivName, strFieldValue;		
		long nColumnCount = 2;
		long nColumnWidth = (nColumnCount==0)?0:GetBrowserWidth()/nColumnCount;
		// (d.singleton 2013-10-28 16:37) - PLID 59197 - need to move the specimen segment values to its own header in the html view for labs,  so it will show regardless of any results. 	
		for(long i = (long)lrtcSpecimenIDText; i <= (long)lrtcSpecimenCondition; i++) {
			//reset the values of our field name div and value
			strFieldName = strDivName = strFieldValue = "";
			switch((LabResultTreeColumns)i)
			{			
			case lrtcSpecimenIDText:
				{
					// (d.singleton 2014-01-16 16:16) - PLID 60225 - need an option to hide spm data on the report view of a lab result.
					if(GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenIdText, "<None>")) {											
						_variant_t varSpecIDText = pSpecRow->GetValue(lrtcSpecimenIDText);
						if(varSpecIDText.vt == VT_BSTR) {
							strFieldName = "Specimen ID Text";
							strFieldValue = ConvertToQuotableXMLString(VarString(varSpecIDText));
							strDivName = "SpecimenIDText";
						}
					}
				}
				break;
			case lrtcSpecimenCollectionStartTime:
				{
					// (d.singleton 2014-01-16 16:16) - PLID 60225 - need an option to hide spm data on the report view of a lab result.
					if(GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenStartTime, "<None>")) {
						strFieldName = "Specimen Collection Start Time";
						strDivName = "SecColStartTime";

						_variant_t var = pSpecRow->GetValue(lrtcSpecimenCollectionStartTime);
						COleDateTime dtSpecStartTime;
						if(var.vt == VT_DATE) {
							dtSpecStartTime = VarDateTime(var);
						}
						else if(var.vt == VT_BSTR) {
							if(VarString(var).IsEmpty()) {
								dtSpecStartTime.SetStatus(COleDateTime::invalid);
							}
							else {
								dtSpecStartTime.ParseDateTime(VarString(var));
							}
						}
						else {
							dtSpecStartTime.SetStatus(COleDateTime::invalid);
						}
						if(dtSpecStartTime.GetStatus() == COleDateTime::valid) {
							strFieldValue = FormatDateTimeForInterface(dtSpecStartTime);
						}
					}
				}
				break;
			case lrtcSpecimenCollectionEndTime:
				{
					// (d.singleton 2014-01-16 16:16) - PLID 60225 - need an option to hide spm data on the report view of a lab result.
					if(GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenEndTime, "<None>")) {
						strFieldName = "Specimen Collection End Time";
						strDivName = "SecColEndTime";

						_variant_t var = pSpecRow->GetValue(lrtcSpecimenCollectionEndTime);
						COleDateTime dtSpecEndTime;
						if(var.vt == VT_DATE) {
							dtSpecEndTime = VarDateTime(var);
						}
						else if(var.vt == VT_BSTR) {
							if(VarString(var).IsEmpty()) {
								dtSpecEndTime.SetStatus(COleDateTime::invalid);
							}
							else {
								dtSpecEndTime.ParseDateTime(VarString(var));
							}
						}
						else {
							dtSpecEndTime.SetStatus(COleDateTime::invalid);
						}
						if(dtSpecEndTime.GetStatus() == COleDateTime::valid) {
							strFieldValue = FormatDateTimeForInterface(dtSpecEndTime);
						}
					}
				}
				break;
			case lrtcSpecimenRejectReason:
				{
					// (d.singleton 2014-01-16 16:16) - PLID 60225 - need an option to hide spm data on the report view of a lab result.
					if(GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenRejectReason, "<None>")) {
						_variant_t varSpecRejReason = pSpecRow->GetValue(lrtcSpecimenRejectReason);
						if(varSpecRejReason.vt == VT_BSTR) {
							strFieldName = "Specimen Reject Reason";
							strFieldValue = ConvertToQuotableXMLString(VarString(varSpecRejReason));
							strDivName = "SpecimenRejectReason";
						}
					}
				}
				break;
			case lrtcSpecimenCondition:
				{
					// (d.singleton 2014-01-16 16:16) - PLID 60225 - need an option to hide spm data on the report view of a lab result.
					if(GetRemotePropertyInt("LabReportViewFieldShow", 1, lrfSpecimenCondition, "<None>")) {
						_variant_t varSpecCondition = pSpecRow->GetValue(lrtcSpecimenCondition);
						if(varSpecCondition.vt == VT_BSTR) {
							strFieldName = "Specimen Condition";
							strFieldValue = ConvertToQuotableXMLString(VarString(varSpecCondition));
							strDivName = "SpecimenCondition";
						}
					}
				}
				break;
			default:
				//if this happens you need to add code for your new column here
				ASSERT(FALSE);
				break;
			}

			if(!strFieldValue.IsEmpty()) {							
				strHTML += FormatString("\t\t\t\t<td colspan=\"1\">\r\n");
				strHTML += FormatString("\t\t\t\t\t<div id=\"%s\" style=\"width:%ipx;text-align:left;vertical-align:top;\"> \r\n", strDivName, nColumnWidth);
				strHTML += FormatString("\t\t\t\t\t\t<span class=\"MinorTitle\"> %s: </span> \r\n", strFieldName);
				// (r.gonet 07/26/2013) - PLID 57751 - Noticed linebreaks weren't being retained in the report view for mapped fields. Fixed.
				strFieldValue.Replace("\r\n", "<BR/>");
				// (r.gonet 03/07/2013) - PLID 43599 - If the setting is disabled, preserve consecutive whitespace.
				if(GetRemotePropertyInt("LabReportViewTrimExtraSpaces", TRUE) == FALSE) {
					strFieldValue.Replace(" ", "&nbsp;");
				} else {
					// The browser will trim the spaces.
				}
				strHTML += FormatString("\t\t\t\t\t\t<span class=\"MinorInfo\"> %s </span> \r\n", strFieldValue);
				strHTML += "\t\t\t\t\t</div>\r\n"; 
				strHTML += "\t\t\t\t</td>\r\n";
			}
			else {
				FormatString("<td colspan=\"1\"><div style=\"width:%ipx;text-align:left;vertical-align:top;\"></div></td>", nColumnWidth);
			}
		}
		if(!strHTML.IsEmpty()) {
				strHTML += "\t\t\t<div class=\"InnerRow\">\r\n";
				strHTML += "\t\t\t\t<div class=\"InnerCellWithBorder\">\r\n";
				strHTML += "\t\t\t\t\t<div id=\"End\"> ";
				strHTML += "\t\t\t\t\t\t<span class=\"MinorTitle\">  </span> \r\n";	
				strHTML += "\t\t\t\t\t\t<span class=\"MinorInfo\"> </span> \r\n";
				strHTML += "\t\t\t\t\t</div>\r\n";
				//close the Cell
				strHTML += "\t\t\t\t</div>\r\n";	
				//close the Row
				strHTML += "\t\t\t</div>\r\n";
		}
	}
	return strHTML;
}

// (r.goldschmidt 2016-02-18 17:52) - PLID 68267 - Filter the microscopic description list for any linked descriptions that are linked with the final lab diagnoses that the user chose
//  (only use when it is known that there is a link for the microscopic description)
void CLabResultsTabDlg::FilterClinicalDiagnosisOptionsList(const std::set<long>& arIDs)
{
	try {
		IRowSettingsPtr pRow = m_pClinicalDiagOptionsList->GetFirstRow();
		while (pRow) {
			if (VarLong(pRow->GetValue(cdolcHasLink)) == (long)fvSpecialShowAll) { // show the special <show all> option
				pRow->PutVisible(VARIANT_TRUE);
			}
			else if (arIDs.count(VarLong(pRow->GetValue(cdolcID))) > 0) { // there is a link, filter it in
				pRow->PutValue(cdolcHasLink, _variant_t(fvHasLink));
				pRow->PutVisible(VARIANT_TRUE);
			}
			else { // there is no link, filter it out
				pRow->PutValue(cdolcHasLink, _variant_t(fvNoLink));
				pRow->PutVisible(VARIANT_FALSE);
			}
			pRow = pRow->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__);
	
}


// (r.goldschmidt 2016-02-18 18:03) - PLID 68267 - Unfilter the list
void CLabResultsTabDlg::RemoveFilterClinicalDiagnosisOptionsList()
{
	try {
		IRowSettingsPtr pRow = m_pClinicalDiagOptionsList->GetFirstRow();
		while (pRow) {
			if (VarLong(pRow->GetValue(cdolcHasLink)) == (long)fvSpecialShowAll) { // show the special <show all> option
				pRow->PutVisible(VARIANT_FALSE);
			}
			else if (VarLong(pRow->GetValue(cdolcHasLink)) == (long)fvHasLink) { // there was a filtered link, reset
				pRow->PutValue(cdolcHasLink, _variant_t(fvNoLink));
				pRow->PutVisible(VARIANT_TRUE);
			}
			else { // there was no filtered link, show again
				pRow->PutVisible(VARIANT_TRUE);
			}
			pRow = pRow->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__);

}

// (r.goldschmidt 2016-02-18 18:03) - PLID 68268 - add <show all> row when requerying into a filtered list
void CLabResultsTabDlg::RequeryFinishedClinicalDiagnosisOptionsList(short nFlags)
{
	try {
		// add <show all>
		IRowSettingsPtr pRow = m_pClinicalDiagOptionsList->GetNewRow();
		pRow->PutValue(cdolcID, (long)-1);
		pRow->PutValue(cdolcDesc, _bstr_t(""));
		pRow->PutValue(cdolcDescForSorting, _bstr_t("<show all>"));
		pRow->PutValue(cdolcHasLink, (long)fvSpecialShowAll);
		pRow->PutVisible(VARIANT_FALSE);
		m_pClinicalDiagOptionsList->AddRowBefore(pRow, m_pClinicalDiagOptionsList->GetFirstRow());

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-02-19 10:37) - PLID 68266 - add diagnosis to clinical diagnosis linking
void CLabResultsTabDlg::AddLabClinicalDiagnosisLink(const std::set<long>& arDiagnosisIDs, long nClinicalDiagnosisID)
{
	try {

		Nx::SafeArray<long> saDiagIDs(arDiagnosisIDs.size(), begin(arDiagnosisIDs), end(arDiagnosisIDs));

		std::set<long> arClinicalDiagIDs;
		arClinicalDiagIDs.insert(nClinicalDiagnosisID);
		Nx::SafeArray<long> saClinicalDiagIDs(arClinicalDiagIDs.size(), begin(arClinicalDiagIDs), end(arClinicalDiagIDs));

		GetAPI()->CreateLabClinicalDiagnosisLinking(GetAPISubkey(), GetAPILoginToken(), saDiagIDs, saClinicalDiagIDs, VARIANT_FALSE);

	}NxCatchAll(__FUNCTION__);
}
