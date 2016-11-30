// MirrorLinkCommonPtsDlg.cpp : implementation file
//
// (c.haag 2008-02-06 09:55) - PLID 28622 - This class allows the user to
// determine which patients can be linked between both softwares, and then
// link them together.
//
// For inputs, we are given two other datalists; one with Practice fields,
// and one with Mirror data fields. The fields include: Last/First name,
// Middle Name, SSN, Home Phone, Birthdate, Address 1, Address 2, City, State,
// Zip. We also get the full name and the chart number in each respective system
// for display purposes.
//

#include "stdafx.h"
#include "practice.h"
#include "practicerc.h"
#include "globalutils.h"
#include "MirrorLinkCommonPtsDlg.h"
#include "Mirror.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDT_WELCOME		1000
#define IDT_REQUERY		1001
#define IDT_LINK		1002

//
// Enums for m_dlInput_* lists
//
typedef enum {
	// Standard data fields
	ecFirstContactDate = 0L,
	ecName,
	ecChartNumber,
	ecInternalRemoteID,

	// Fields used for matching patients for linking purposes
	ecLastCommaFirst,	// Last + ', ' + First name
	ecMiddle,			// Middle name
	ecSocialSecurity,
	ecHomePhone,
	ecBirthdate,
	ecAddress1,
	ecAddress2,
	ecCity,
	ecState,
	ecZip
} ELinkInputListColumn;

//
// Enums for qualifying lists
//
typedef enum {
	eqcPracUserDefinedID = 0L,
	eqcPracName,
	eqcPracAddress,
	eqcPracSSN,
	eqcPracHomePhone,
	eqcPracBirthdate,
	eqcLink,
	eqcRemoteID,
	eqcRemoteChartNum,
	eqcRemoteName,
	eqcRemoteAddress,
	eqcRemoteSSN,
	eqcRemoteHomePhone,
	eqcRemoteBirthdate
} EQualifyColumn;

//
// Enums for disqualifying lists
//
typedef enum {
	edcID = 0L,
	edcChartNum,
	edcFullName,
	edcReason
} EDisqualifyColumn;

/////////////////////////////////////////////////////////////////////////////
// CMirrorLinkCommonPtsDlg dialog


CMirrorLinkCommonPtsDlg::CMirrorLinkCommonPtsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMirrorLinkCommonPtsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMirrorLinkCommonPtsDlg)
	m_bUseMiddleName = TRUE;
	m_bUseAddress1 = TRUE;
	m_bUseAddress2 = TRUE;
	m_bUseBirthdate = TRUE;
	m_bUseCity = TRUE;
	m_bUseHomePhone = TRUE;
	m_bUseSSN = TRUE;
	m_bUseState = TRUE;
	m_bUseZip = TRUE;
	m_bLinkPatIDToMRN = FALSE;
	m_bLinkPatIDToSSN = FALSE;
	//}}AFX_DATA_INIT
	m_bChangedData = FALSE;
	m_nInitialQualifyListSize = 0;
	ResetProgressCount();
}

int CMirrorLinkCommonPtsDlg::Open(NXDATALISTLib::_DNxDataListPtr dlPractice,
		NXDATALISTLib::_DNxDataListPtr dlRemote)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function invokes the
	// dialog.
	m_dlInput_Practice = dlPractice;
	m_dlInput_Remote = dlRemote;
	m_bChangedData = FALSE;

	return DoModal();
}

void CMirrorLinkCommonPtsDlg::RequeryAll()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function causes all
	// the lists on the form to be requeried based on the current linking
	// criteria.

	// Update the dialog data
	UpdateData();

	// Update buttons
	GetDlgItem(IDC_BTN_LINK_QUALIFYING_PATIENTS)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CALCULATE_LINK_PTS)->EnableWindow(FALSE);

	// Reset the progress bar
	const long nPracticeRecords = m_dlInput_Practice->GetRowCount();
	const long nRemoteRecords = m_dlInput_Remote->GetRowCount();
	m_ProgressRequery.SetRange(0, 10000);
	m_ProgressRequery.SetPos(0);

	// Reset the existing linked patient count
	m_nExistingLinkedPts = 0;
	
	// Now clear out all the lists
	m_dl2Qualify->Clear();
	m_dl2DisqualifyPractice->Clear();
	m_dl2DisqualifyRemote->Clear();

	// Reset the improperly linked patient counter
	m_anImproperlyLinkedPracticeOrdinals.RemoveAll();

	// At this point, we would run a loop through all our input rows, figure
	// out whether each patient qualifies or doesn't qualify to be linked,
	// and populate our lists. However, we know better than to just keep the
	// user waiting for the program to respond. We will perform each iteration
	// in a timer.
	ResetProgressCount();
	SetTimer(IDT_REQUERY, 10, NULL);
}

void CMirrorLinkCommonPtsDlg::StopTimedAction()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function is called when
	// a timed action is either completed or interrupted
	ResetProgressCount();

	// Kill all repeating timers
	KillTimer(IDT_REQUERY);
	KillTimer(IDT_LINK);

	// Update window appearances
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CALCULATE_LINK_PTS)->EnableWindow(TRUE);
	GetDlgItem(IDC_LIST_LINK_QUALIFY)->EnableWindow(TRUE);
	if (m_dl2Qualify->GetRowCount() > 0) {
		GetDlgItem(IDC_BTN_LINK_QUALIFYING_PATIENTS)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_BTN_LINK_QUALIFYING_PATIENTS)->EnableWindow(FALSE);
	}
}

void CMirrorLinkCommonPtsDlg::ResetProgressCount()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function is called when
	// we want to reset the progress bar and m_nProgressIteration
	m_nProgressIteration = 0;
	if (IsWindow(m_ProgressRequery.GetSafeHwnd())) {
		m_ProgressRequery.SetPos(0);
	}
}

HRESULT CMirrorLinkCommonPtsDlg::VariantCompare(_variant_t v1, _variant_t v2)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function encapsulates the
	// global variant compare function.
	if (VT_BSTR == v1.vt && VT_BSTR == v2.vt) {
		// We need to trim strings
		CString str1 = VarString(v1);
		CString str2 = VarString(v2);
		str1.TrimLeft(); str1.TrimRight();
		str2.TrimLeft(); str2.TrimRight();
		v1 = str1;
		v2 = str2;
	}
	return ::VariantCompare(&v1, &v2);
}

_variant_t CMirrorLinkCommonPtsDlg::CleanSSN(_variant_t v)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function takes in
	// a variant that should contain an SSN, and tries to format it.
	// If the type is VT_NULL, this function returns an empty string.
	// If it's VT_BSTR, it is formatted. Anything else is returned exactly
	// as it is.
	if (VT_NULL == v.vt) {
		v = "";
	}
	else if (VT_BSTR == v.vt) {
		CString str = VarString(v);
		str.TrimLeft();
		str.TrimRight();
		if (!str.IsEmpty()) {
			CString strSSN = FormatSSN(str);
			strSSN.TrimRight("#");
			v = _bstr_t(strSSN);
		} else {
			v = "";
		}
	}
	return v;
}

HRESULT CMirrorLinkCommonPtsDlg::CompareSSNs(_variant_t v1, _variant_t v2)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function compares two SSN's
	v1 = CleanSSN(v1);
	v2 = CleanSSN(v2);
	return ::VariantCompare(&v1, &v2);
}

_variant_t CMirrorLinkCommonPtsDlg::CleanPhoneNumber(_variant_t v)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function takes in
	// a variant that should contain a phone number, and tries to format
	// it. If the type is VT_NULL, this function returns an empty string.
	// If it's VT_BSTR, it is formatted. Anything else is returned exactly
	// as it is.
	if (VT_NULL == v.vt) {
		v = "";
	}
	else if (VT_BSTR == v.vt) {
		CString str = VarString(v);
		str.TrimLeft();
		str.TrimRight();
		if (!str.IsEmpty()) {
			if (GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
				const CString strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
				v = _bstr_t(FormatPhone(str, strPhoneFormat));
			} else {
				v = _bstr_t(str);
			}
		} else {
			v = "";
		}
	}
	return v;
}

HRESULT CMirrorLinkCommonPtsDlg::ComparePhones(_variant_t v1, _variant_t v2)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function compares two
	// phone numbers
	v1 = CleanPhoneNumber(v1);
	v2 = CleanPhoneNumber(v2);
	return ::VariantCompare(&v1, &v2);
}

_variant_t CMirrorLinkCommonPtsDlg::CleanZip(_variant_t v)
{
	// (c.haag 2008-03-11 11:15) - PLID 28622 - This function takes in
	// a variant that should contain a zip code, and ensures there are no
	// trailing dashes or spaces.
	if (VT_NULL == v.vt) {
		v = "";
	}
	else if (VT_BSTR == v.vt) {
		CString str = VarString(v);
		str.TrimLeft(" -\\/");
		str.TrimRight(" -\\/");
		v = _bstr_t(str);
	}
	return v;
}

HRESULT CMirrorLinkCommonPtsDlg::CompareZips(_variant_t v1, _variant_t v2)
{
	// (c.haag 2008-03-11 11:13) - PLID 28622 - This function compares two
	// zip codes
	v1 = CleanZip(v1);
	v2 = CleanZip(v2);
	return ::VariantCompare(&v1, &v2);
}

NXDATALIST2Lib::IRowSettingsPtr CMirrorLinkCommonPtsDlg::FailQualifyPracticeRecord(long nOrdinal, LPCTSTR szReason, BOOL bAddSorted /*= TRUE */)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function is called if a record in
	// the Practice input list failed to qualify to link with another remote record. The
	// interface will be reflected to update this fact.
	NXDATALISTLib::IRowSettingsPtr pDisqRow = m_dlInput_Practice->GetRow(nOrdinal);
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dl2DisqualifyPractice->GetNewRow();
	pNewRow->Value[ edcID ] = pDisqRow->Value[ ecChartNumber ];
	pNewRow->Value[ edcChartNum ] = pDisqRow->Value[ ecChartNumber ];
	pNewRow->Value[ edcFullName ] = pDisqRow->Value[ ecName ];
	pNewRow->Value[ edcReason ] = szReason;
	if (bAddSorted) {
		m_dl2DisqualifyPractice->AddRowSorted(pNewRow, NULL);
	} else {
		m_dl2DisqualifyPractice->AddRowBefore(pNewRow, m_dl2DisqualifyPractice->GetFirstRow());
	}
	return pNewRow;
}

void CMirrorLinkCommonPtsDlg::FailQualifyMirrorRecord(long nOrdinal, LPCTSTR szReason)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function is called if a record in
	// the remote input list failed to qualify to link with another Practice record. The
	// interface will be reflected to update this fact.
	NXDATALISTLib::IRowSettingsPtr pDisqRow = m_dlInput_Remote->GetRow(nOrdinal);
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dl2DisqualifyRemote->GetNewRow();
	pNewRow->Value[ edcID ] = pDisqRow->Value[ ecInternalRemoteID ];
	pNewRow->Value[ edcChartNum ] = pDisqRow->Value[ ecChartNumber ];
	pNewRow->Value[ edcFullName ] = pDisqRow->Value[ ecName ];
	pNewRow->Value[ edcReason ] = szReason;
	m_dl2DisqualifyRemote->AddRowSorted(pNewRow, NULL);
}

void CMirrorLinkCommonPtsDlg::TryQualifyPracticeRecord(long nOrdinal)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function will examine the
	// record in m_dlInput_Practice for the given ordinal, and either add it to
	// the m_dl2Qualify list, or added it to m_dl2DisqualifyPractice based on
	// whether it can be matched up with exactly one Mirror record based on the
	// user criteria

	_variant_t vName = m_dlInput_Practice->GetValue(nOrdinal, ecLastCommaFirst);
	_variant_t vCurrentMirrorID = m_dlInput_Practice->GetValue(nOrdinal, ecInternalRemoteID);
	BOOL bQualifies = FALSE; // (c.haag 2010-04-08 11:38) - PLID 38109 - We now have a flag so that we can
									// be more fluid with our conditionals.
	long nRow;

	// First, see if the patient is already linked
	if (VT_NULL != vCurrentMirrorID.vt) {
		// Already linked. Technically, if the link is proper, then the patient has
		// already been qualified, so we don't want it to appear in the list
		// and clutter other results that are clearly of more interest.

		// (c.haag 2008-02-19 17:21) - Before dismissing the patient, make sure we can
		// find a perfect first-last name match in Mirror
		nRow = m_dlInput_Remote->FindByColumn(ecLastCommaFirst, vName, 0, FALSE);
		if (nRow > -1 && vCurrentMirrorID == m_dlInput_Remote->GetValue(nRow, ecInternalRemoteID)) {
			// If we get here, the internal Mirror ID and Mirror first-last names match.
			// Definitely a perfect link.
			m_nExistingLinkedPts++;
		} else {
			// (c.haag 2008-02-20 09:26) - If we get here, either the internal Mirror ID
			// is nowhere to be found in the Mirror input list; or, the ID was found but 
			// the first-last names don't match case-insensitively.
			//
			// At the time of this comment, The Link to Mirror dialog only checks by
			// Mirror ID to determine if a patient is improperly linked. While this newer
			// implementation is inconsistent because it also checks for matching names;
			// it's also more correct, and will identify name typos or flat out incorrectly
			// linked patients.
			
			// Add this patient to our improper link array. Later on, we will stack these
			// patients at the top of the Practice disqualify list. We use InsertAt so that
			// the improperly linked patients themselves are sorted A-Z.
			m_anImproperlyLinkedPracticeOrdinals.InsertAt(0, nOrdinal);
		}
		return;
	}

	// (c.haag 2010-04-08 11:26) - PLID 38109 - If the office links Practice UserDefinedID's (chart numbers) and Mirror
	// MRN's, try to find a matching patient by first, last and MRN. If we find one, run with it. If not, defer to the name
	// search.
	if (m_bLinkPatIDToMRN) {

		// Get the user-defined ID. This should never be null, and the MRN is actually stored as a string in the Mirror list.
		CString strUserDefinedID = AsString(m_dlInput_Practice->GetValue(nOrdinal, ecChartNumber));
		nRow = m_dlInput_Remote->FindByColumn(ecChartNumber, _bstr_t(strUserDefinedID), 0, FALSE);
		if (nRow > -1) {

			// We found a matching MRN. Search for a duplicate MRN in Mirror, just in case.
			long nDupRow = m_dlInput_Remote->FindByColumn(ecChartNumber, _bstr_t(strUserDefinedID), nRow+1, FALSE);
			if (nDupRow > -1 && nDupRow != nRow) {
				// Wow...this MRN appears twice in Mirror. Just defer to the name search.
			}
			else {				
				// Verify this is a matching patient by name
				CString strNexTechName = VarString(vName, "");
				CString strMirrorName = VarString(m_dlInput_Remote->GetValue(nRow, ecLastCommaFirst));
				if (!strMirrorName.CompareNoCase(strNexTechName)) {
					// Success. Leave nRow intact and set the qualify flag.
					bQualifies = TRUE;
				}
				else {
					// MRN matches, but names do not match. This record does not qualify; defer to the name search.
					// Even if there is indeed one and only one matching name, the logic in the name search disqualifies the
					// patient from linking if the MRN does not match.
				}
			}

		}
		else {
			// Did not find the MRN; defer to the name search
		}
	}

	// (c.haag 2010-04-08 11:40) - PLID 38109 - Don't search by name if we're already qualified
	if (!bQualifies) {

		// Now check if this name exists more than once in Practice, and if so, ignore it
		nRow = m_dlInput_Practice->FindByColumn(ecLastCommaFirst, vName, nOrdinal + 1, FALSE);
		if (nRow > -1 && nRow != nOrdinal) {
			
			// If we found a row that doesn't match the ordinal, then this row is disqualified
			// because the name appears more than once in Practice.
			FailQualifyPracticeRecord(nOrdinal, "More than one matching first and last names were found in NexTech Practice");
			return;
		}

		// Now compare by name
		nRow = m_dlInput_Remote->FindByColumn(ecLastCommaFirst, vName, 0, FALSE);
		if (nRow == -1) {

			// If we didn't find a row, the name doesn't exist in the remote list,
			// so that disqualifies this row
			//
			FailQualifyPracticeRecord(nOrdinal, "No matching name found in Canfield Mirror");
			return;
		}
		
		long nDupRow = m_dlInput_Remote->FindByColumn(ecLastCommaFirst, vName, nRow+1, FALSE);
		if (nDupRow > -1 && nDupRow != nRow) {
			// If we get here, we found a row, but then we found another one with a matching name.
			// That disqualifies this row.
			//
			FailQualifyPracticeRecord(nOrdinal, "More than one matching first and last names were found in Canfield Mirror");
			return;
		}

	} // if (!bQualifies) {

	// If we get here, we've found exactly one match by name. 

	// If we are linking MRN's, then check whether the Mirror patient already has an MRN. If it does,
	// and it doesn't match the Practice chart number, then we can't link it
	if (m_bLinkPatIDToMRN) {
		CString strMRN = VarString(m_dlInput_Remote->GetValue(nRow, ecChartNumber), "");
		const CString strUserDefinedID = AsString(m_dlInput_Practice->GetValue(nOrdinal, ecChartNumber));
		strMRN.TrimLeft();
		strMRN.TrimRight();
		if (!strMRN.IsEmpty() && strMRN != strUserDefinedID) {
			// If we get here, the Mirror patient has an MRN. We can't link it because
			// it will destory the existing MRN.
			FailQualifyPracticeRecord(nOrdinal, "The patient in Canfield Mirror has an MRN value that does not match their Practice chart number");
			return;
		}
	}

	// Do the same check for SSN's
	if (m_bLinkPatIDToSSN) {
		CString strSSN = VarString(m_dlInput_Remote->GetValue(nRow, ecSocialSecurity), "");
		const CString strUserDefinedID = AsString(m_dlInput_Practice->GetValue(nOrdinal, ecChartNumber));
		strSSN.TrimLeft();
		strSSN.TrimRight();
		if (!strSSN.IsEmpty() && strSSN != strUserDefinedID) {
			// If we get here, the Mirror patient has an SSN. We can't link it because
			// it will destory the existing SSN.
			FailQualifyPracticeRecord(nOrdinal, "The patient in Canfield Mirror has an SSN value that does not match their Practice chart number");
			return;
		}
	}
	
	// Check whether the other fields match. By this point in time, all text files should be upper-case.
	//
	if (m_bUseMiddleName && VARCMP_EQ != VariantCompare(m_dlInput_Practice->GetValue(nOrdinal, ecMiddle), m_dlInput_Remote->GetValue(nRow, ecMiddle))) {
		FailQualifyPracticeRecord(nOrdinal, "Middle names do not match");
	}
	else if (m_bUseSSN && !m_bLinkPatIDToSSN && VARCMP_EQ != CompareSSNs(m_dlInput_Practice->GetValue(nOrdinal, ecSocialSecurity), m_dlInput_Remote->GetValue(nRow, ecSocialSecurity))) {
		FailQualifyPracticeRecord(nOrdinal, "Social security numbers do not match");
	}
	else if (m_bUseHomePhone && VARCMP_EQ != ComparePhones(m_dlInput_Practice->GetValue(nOrdinal, ecHomePhone), m_dlInput_Remote->GetValue(nRow, ecHomePhone))) {
		FailQualifyPracticeRecord(nOrdinal, "Home phone numbers do not match");
	}
	else if (m_bUseBirthdate && VARCMP_EQ != VariantCompare(m_dlInput_Practice->GetValue(nOrdinal, ecBirthdate), m_dlInput_Remote->GetValue(nRow, ecBirthdate))) {
		FailQualifyPracticeRecord(nOrdinal, "Birth dates do not match");
	}
	else if (m_bUseAddress1 && VARCMP_EQ != VariantCompare(m_dlInput_Practice->GetValue(nOrdinal, ecAddress1), m_dlInput_Remote->GetValue(nRow, ecAddress1))) {
		FailQualifyPracticeRecord(nOrdinal, "Address 1 lines do not match");
	}
	else if (m_bUseAddress2 && VARCMP_EQ != VariantCompare(m_dlInput_Practice->GetValue(nOrdinal, ecAddress2), m_dlInput_Remote->GetValue(nRow, ecAddress2))) {
		FailQualifyPracticeRecord(nOrdinal, "Address 2 lines do not match");
	}
	else if (m_bUseCity && VARCMP_EQ != VariantCompare(m_dlInput_Practice->GetValue(nOrdinal, ecCity), m_dlInput_Remote->GetValue(nRow, ecCity))) {
		FailQualifyPracticeRecord(nOrdinal, "City fields not match");
	}
	else if (m_bUseState && VARCMP_EQ != VariantCompare(m_dlInput_Practice->GetValue(nOrdinal, ecState), m_dlInput_Remote->GetValue(nRow, ecState))) {
		FailQualifyPracticeRecord(nOrdinal, "State fields do not match");
	}
	else if (m_bUseZip && VARCMP_EQ != CompareZips(m_dlInput_Practice->GetValue(nOrdinal, ecZip), m_dlInput_Remote->GetValue(nRow, ecZip))) {
		FailQualifyPracticeRecord(nOrdinal, "Zip code fields do not match");
	}
	else {
		// If we get here, the records match perfectly
		CString strFullPracAddress =
			VarString(m_dlInput_Practice->GetValue(nOrdinal, ecAddress1), "") + " " +
			VarString(m_dlInput_Practice->GetValue(nOrdinal, ecAddress2), "") + " " +
			VarString(m_dlInput_Practice->GetValue(nOrdinal, ecCity), "") + " " +
			VarString(m_dlInput_Practice->GetValue(nOrdinal, ecState), "") + " " +
			VarString(m_dlInput_Practice->GetValue(nOrdinal, ecZip), "");

		CString strFullMirrorAddress =
			VarString(m_dlInput_Remote->GetValue(nRow, ecAddress1), "") + " " +
			VarString(m_dlInput_Remote->GetValue(nRow, ecAddress2), "") + " " +
			VarString(m_dlInput_Remote->GetValue(nRow, ecCity), "") + " " +
			VarString(m_dlInput_Remote->GetValue(nRow, ecState), "") + " " +
			VarString(m_dlInput_Remote->GetValue(nRow, ecZip), "");

		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dl2Qualify->GetNewRow();
		pNewRow->Value[eqcPracUserDefinedID] = m_dlInput_Practice->GetValue(nOrdinal, ecChartNumber);
		pNewRow->Value[eqcPracName] = m_dlInput_Practice->GetValue(nOrdinal, ecName);
		pNewRow->Value[eqcPracAddress] = _bstr_t(strFullPracAddress);
		pNewRow->Value[eqcPracSSN] = CleanSSN(m_dlInput_Practice->GetValue(nOrdinal, ecSocialSecurity));
		pNewRow->Value[eqcPracHomePhone] = CleanPhoneNumber(m_dlInput_Practice->GetValue(nOrdinal, ecHomePhone));
		pNewRow->Value[eqcPracBirthdate] = m_dlInput_Practice->GetValue(nOrdinal, ecBirthdate);
		pNewRow->Value[eqcLink] = _variant_t(VARIANT_TRUE, VT_BOOL);
		pNewRow->Value[eqcRemoteID] = m_dlInput_Remote->GetValue(nRow, ecInternalRemoteID);
		pNewRow->Value[eqcRemoteChartNum] = m_dlInput_Remote->GetValue(nRow, ecChartNumber);
		pNewRow->Value[eqcRemoteName] = m_dlInput_Remote->GetValue(nRow, ecName);
		pNewRow->Value[eqcRemoteAddress] = _bstr_t(strFullMirrorAddress);
		pNewRow->Value[eqcRemoteSSN] = CleanSSN(m_dlInput_Remote->GetValue(nRow, ecSocialSecurity));
		pNewRow->Value[eqcRemoteHomePhone] = CleanPhoneNumber(m_dlInput_Remote->GetValue(nRow, ecHomePhone));
		pNewRow->Value[eqcRemoteBirthdate] = m_dlInput_Remote->GetValue(nRow, ecBirthdate);
		m_dl2Qualify->AddRowSorted(pNewRow, NULL);
	}
}

void CMirrorLinkCommonPtsDlg::TryQualifyRemoteRecord(long nOrdinal)
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function will examine the
	// record in m_dlInput_Remote for the given ordinal. This is done after we
	// test Practice records for qualification. Any Mirror records that could not
	// be carried over to the qualification list will be added to m_dl2DisqualifyRemote.

	_variant_t vName = m_dlInput_Remote->GetValue(nOrdinal, ecLastCommaFirst);
	_variant_t vInternalMirrorID = m_dlInput_Remote->GetValue(nOrdinal, ecInternalRemoteID);
	long nRow;

	// Check whether this record is already linked with NexTech Practice
	if (-1 != m_dlInput_Practice->FindByColumn(ecInternalRemoteID, vInternalMirrorID, 0, FALSE)) {

		// Already linked. Don't add it to the list. Technically, it has
		// already been qualified, so we don't want it to appear in the list
		// and clutter other results that are clearly of more interest.
		return;
	}

	// Check whether this record was already flagged as having been qualified
	if (NULL != m_dl2Qualify->FindByColumn(eqcRemoteID, vInternalMirrorID, NULL, VARIANT_FALSE)) {

		// If we get here, it must mean the patient has already qualified. Don't
		// add it to the list; just skip this record.
		return;
	}

	// Check whether the patient's name exists more than once in Mirror
	nRow = m_dlInput_Remote->FindByColumn(ecLastCommaFirst, vName, nOrdinal + 1, FALSE);
	if (nRow > -1 && nRow != nOrdinal) {

		// If we get here, it must mean the patient's name already exists
		// more than once in Mirror
		FailQualifyMirrorRecord(nOrdinal, "More than one matching first and last names were found in Canfield Mirror");
		return;
	}

	// Check whether the patient's name exists in NexTech Practice
	nRow = m_dlInput_Practice->FindByColumn(ecLastCommaFirst, vName, 0, FALSE);
	if (nRow == -1) {

		// If we didn't find a row, the name doesn't exist in NexTech Practice
		// so that disqualifies this row
		//
		FailQualifyMirrorRecord(nOrdinal, "No matching name found in NexTech Practice");
		return;
	}
	else {
		long nDupRow = m_dlInput_Practice->FindByColumn(ecLastCommaFirst, vName, nRow+1, FALSE);
		if (nDupRow > -1 && nDupRow != nRow) {
			// If we get here, we found a row, but then we found another one with a matching name.
			// That disqualifies this row.
			//
			FailQualifyMirrorRecord(nOrdinal, "More than one matching first and last names were found in NexTech Practice");
			return;
		}
		else {
			// If we get here, it means the name exists once and only once in both databases.
			// We've already figured out why it fails in Practice, so carry over that reason.

			NXDATALISTLib::IRowSettingsPtr pInputRow = m_dlInput_Practice->GetRow(nRow);
			_variant_t vPracChartNumber = pInputRow->Value[ ecChartNumber ];

			NXDATALIST2Lib::IRowSettingsPtr pDisqRow = m_dl2DisqualifyPractice->FindByColumn(edcChartNum, vPracChartNumber, NULL, VARIANT_FALSE);
			if (NULL == pDisqRow) {
				ASSERT(FALSE);
				ThrowNxException("TryQualifyRemoteRecord detected a disqualifying record that did not exist in the disqualification list!");
			}
			FailQualifyMirrorRecord(nOrdinal, VarString(pDisqRow->Value[ edcReason ]));
		}
	}
}

void CMirrorLinkCommonPtsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMirrorLinkCommonPtsDlg)
	DDX_Control(pDX, IDC_CHECK_USEMIDDLENAME, m_btnUseMiddleName);
	DDX_Control(pDX, IDC_CHECK_USESSN, m_btnUseSSN);
	DDX_Control(pDX, IDC_CHECK_USEHOMEPHONE, m_btnUseHPhone);
	DDX_Control(pDX, IDC_CHECK_USEBIRTHDATE, m_btnUseBDate);
	DDX_Control(pDX, IDC_CHECK_USEADDR1, m_btnUseAddr1);
	DDX_Control(pDX, IDC_CHECK_USEADDR2, m_btnUseAddr2);
	DDX_Control(pDX, IDC_CHECK_USECITY, m_btnUseCity);
	DDX_Control(pDX, IDC_CHECK_USESTATE, m_btnUseState);
	DDX_Control(pDX, IDC_CHECK_USEZIP, m_btnUseZip);
	DDX_Control(pDX, IDC_CHECK_SHOWSSN, m_btnShowSSN);
	DDX_Control(pDX, IDC_CHECK_SHOWHOMEPHONE, m_btnShowHPhone);
	DDX_Control(pDX, IDC_CHECK_SHOWBIRTHDATE, m_btnShowBDate);
	DDX_Control(pDX, IDC_LINK_PATID_TO_SSN, m_btnPatIDToSSN);
	DDX_Control(pDX, IDC_LINK_PATID_TO_MRN, m_btnPatIDToMRN);
	DDX_Control(pDX, IDC_PROGRESS_REQUERY, m_ProgressRequery);
	DDX_Check(pDX, IDC_CHECK_USEMIDDLENAME, m_bUseMiddleName);
	DDX_Check(pDX, IDC_CHECK_USEADDR1, m_bUseAddress1);
	DDX_Check(pDX, IDC_CHECK_USEADDR2, m_bUseAddress2);
	DDX_Check(pDX, IDC_CHECK_USEBIRTHDATE, m_bUseBirthdate);
	DDX_Check(pDX, IDC_CHECK_USECITY, m_bUseCity);
	DDX_Check(pDX, IDC_CHECK_USEHOMEPHONE, m_bUseHomePhone);
	DDX_Check(pDX, IDC_CHECK_USESSN, m_bUseSSN);
	DDX_Check(pDX, IDC_CHECK_USESTATE, m_bUseState);
	DDX_Check(pDX, IDC_CHECK_USEZIP, m_bUseZip);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BTN_CALCULATE_LINK_PTS, m_btnCalculate);
	DDX_Control(pDX, IDC_BTN_LINK_QUALIFYING_PATIENTS, m_btnLink);
	DDX_Control(pDX, IDC_BTN_CANCEL, m_btnClose);
	DDX_Check(pDX, IDC_LINK_PATID_TO_MRN, m_bLinkPatIDToMRN);
	DDX_Check(pDX, IDC_LINK_PATID_TO_SSN, m_bLinkPatIDToSSN);
	DDX_Control(pDX, IDC_STATIC_LINK_REMOTE_PTS_FILTER, m_btnLinkRemotePtsFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMirrorLinkCommonPtsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMirrorLinkCommonPtsDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_LINK_QUALIFYING_PATIENTS, OnBtnLinkQualifyingPatients)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_CALCULATE_LINK_PTS, OnBtnCalculateLinkPts)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_BN_CLICKED(IDC_BTN_SSNMRN_HELP, OnBtnSSNMRNHelp)
	ON_BN_CLICKED(IDC_CHECK_SHOWSSN, OnCheckShowSSN)
	ON_BN_CLICKED(IDC_CHECK_SHOWHOMEPHONE, OnCheckShowHomePhone)
	ON_BN_CLICKED(IDC_CHECK_SHOWBIRTHDATE, OnCheckShowBirthdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMirrorLinkCommonPtsDlg message handlers

void CMirrorLinkCommonPtsDlg::OnOK()
{
}

void CMirrorLinkCommonPtsDlg::OnCancel()
{
}

void CMirrorLinkCommonPtsDlg::OnBtnLinkQualifyingPatients()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This begins the process of linking
	// qualifying patients (the ones in the top list)
	try {

		// Assign the first row value to m_pCurrentLinkRow; fail out if there is none
		if (NULL == (m_pCurrentLinkRow = m_dl2Qualify->GetFirstRow())) {
			MessageBox("There are no qualifying patients to link. Please review your filter criteria, and try searching for qualified patients again.", "Practice", MB_ICONSTOP | MB_OK);
			return;
		}

		// Confirm the user wants to do this
		if (IDNO == MessageBox("Are you sure you wish to link all selected patients together between NexTech Practice and Canfield Mirror?",
			"Practice", MB_YESNO | MB_ICONQUESTION))
		{
			return;			
		}

		// Warn the user to make a backup of their Mirror data
		if (IDNO == MessageBox("Before you continue, you must verify that you have access to a backup of "
			"your Mirror data created within the past 24 hours. If you are unsure whether one is available, "
			"please contact a technical representative at Canfield Clinical Systems.\n\n"
			"Do you have access to a backup of your Mirror data made within the past 24 hours?",
			"Practice", MB_YESNO | MB_ICONWARNING))
		{
			return;			
		}


		// Reset all counters and window appearances
		ResetProgressCount();
		GetDlgItem(IDC_LIST_LINK_QUALIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CALCULATE_LINK_PTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);

		// Remember how many qualifying patients we have for progress bar tracking
		m_nInitialQualifyListSize = m_dl2Qualify->GetRowCount();

		// Kick off the link timer
		SetTimer(IDT_LINK, 10, NULL);
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnBtnLinkQualifyingPatients");
}

void CMirrorLinkCommonPtsDlg::OnBtnClose()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function will dismiss this dialog
	try {
		// (c.haag 2008-02-05 11:58) - There should not be any timed actions in motion 
		// by this point, but ensure they are stopped anyway.
		StopTimedAction();

		// Warn the user if there is at least one qualifying patient in the top list;
		// regardless of their "Link" checkbox state
		if (m_dl2Qualify->GetRowCount() > 0) {
			if (IDNO == MessageBox(
				FormatString("There are currently %d patient(s) in the qualifying list. "
				"Are you sure you wish to close the window?", m_dl2Qualify->GetRowCount()),
				"Practice", MB_YESNO)) {
				return;
			}
		}

		CDialog::OnCancel();
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnBtnClose");
}

void CMirrorLinkCommonPtsDlg::OnBtnCalculateLinkPts()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This function will begin the process
	// of searching for patients in both systems who meet the criteria for being linked
	try {

		// Warn the user if there's already patients in the list
		if (m_dl2Qualify->GetRowCount() > 0) {
			if (IDNO == MessageBox( FormatString("There are currently %d patient(s) in the qualified list. "
				"This action will clear all patients from the list before repopulating it. Are you sure you wish to continue?",
				m_dl2Qualify->GetRowCount()), "Practice", MB_ICONWARNING | MB_YESNO))
			{
				return;
			}
		}

		// Warn the user if no filters are selected
		UpdateData();
		if (!m_bUseMiddleName && !m_bUseAddress1 && !m_bUseAddress2 && !m_bUseBirthdate && !m_bUseCity &&
			!m_bUseHomePhone && !m_bUseSSN && !m_bUseState && !m_bUseZip) 
		{
			if (IDNO == MessageBox("The only criteria which will be used to qualify patients are first and last name. "
				"It is recommended that you include additional criteria, such as middle name, SSN or home phone number "
				"to match patients together."
				"\n\nAre you sure you wish to continue?",
				"Practice", MB_ICONWARNING | MB_YESNO))
			{
				return;
			}
		}


		// Repopulate all the lists
		RequeryAll();
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnBtnCalculateLinkPts");
}

void CMirrorLinkCommonPtsDlg::OnBtnStop()
{
	// (c.haag 2008-02-06 09:55) - PLID 28622 - This will halt any operation in progress
	try {
		StopTimedAction();
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnBtnStop");
}

BOOL CMirrorLinkCommonPtsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnClose.AutoSet(NXB_CLOSE);

		// Bind the dialog lists
		m_dl2Qualify = BindNxDataList2Ctrl(this, IDC_LIST_LINK_QUALIFY, NULL, false);
		m_dl2DisqualifyPractice = BindNxDataList2Ctrl(this, IDC_LIST_LINK_PRAC_DISQUALIFY, NULL, false);
		m_dl2DisqualifyRemote = BindNxDataList2Ctrl(this, IDC_LIST_LINK_REMOTE_DISQUALIFY, NULL, false);

		// Update the link-to-SSN/MRN checkboxes. Users are NOT allowed to change these values because 
		// they must be consistent with the Advanced Mirror Link Settings.
		m_bLinkPatIDToSSN = Mirror::GetLinkMirrorSSNToUserDefinedID();
		m_bLinkPatIDToMRN = Mirror::GetLinkMirrorMRNToUserDefinedID();
		UpdateData(FALSE);

		// Now update the extended information column sizes based on the checkbox states
		UpdateSSNColumnWidths();
		UpdateHomePhoneColumnWidths();
		UpdateBirthdateColumnWidths();

		// Set a timer to invoke a don't-show-me-again dialog
		// to introduce the user to this window
		SetTimer(IDT_WELCOME, 10, NULL);
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMirrorLinkCommonPtsDlg::OnTimer(UINT nIDEvent) 
{
	try {
		CDialog::OnTimer(nIDEvent);

		if (IDT_WELCOME == nIDEvent) {
			// (c.haag 2008-02-06 09:56) - PLID 28622 - When we get here, we invoke
			// a message box summarizing how to begin using this dialog
			KillTimer(IDT_WELCOME);

			MessageBox("The Link Common Patients feature allows you to link patients who exist "
				"in both NexTech Practice and Canfield Mirror. After these patients are linked, you will "
				"be able to see their existing Mirror images in NexTech Practice, as well as be able to "
				"update demographic information in Canfield Mirror for those patients.\n\n"
				"To get started, please refer to the filter checkboxes at the top-left corner of the Link Common Patients window. "
				"After choosing which fields to match patient demographics by, press the 'Search For Qualifying Patients' "
				"button to start the linking process.", "Practice", MB_ICONINFORMATION|MB_OK);

		} // if (IDT_WELCOME == nIDEvent) {
		else if (IDT_REQUERY == nIDEvent) {
			// (c.haag 2008-02-06 09:56) - PLID 28622 - When we get here,
			// we are performing another iteration in populating the qualify
			// lists.
			KillTimer(IDT_REQUERY);

			const long nPracticeRecords = m_dlInput_Practice->GetRowCount();
			const long nRemoteRecords = m_dlInput_Remote->GetRowCount();
			const long nTotalRecords = nPracticeRecords + nRemoteRecords;
			long nOrdinal = m_nProgressIteration++;

			// Update the progress bar
			m_ProgressRequery.SetPos( (short)( (double) (nOrdinal*10000) / (double)nTotalRecords ) );

			if (nOrdinal < nPracticeRecords) {

				// If we get here, we are going to determine whether the Practice
				// record at nOrdinal qualifies, and if it does, then put it on
				// the list at the top
				TryQualifyPracticeRecord(nOrdinal);

			} else {
				nOrdinal -= nPracticeRecords;

				// If this is the very first ordinal, we need to populate the
				// top of the Practice list with improperly linked patients so
				// that they are as visible as possible
				if (0 == nOrdinal && m_anImproperlyLinkedPracticeOrdinals.GetSize() > 0) {
					const int nItems = m_anImproperlyLinkedPracticeOrdinals.GetSize();
					for (int i=0; i < nItems; i++) {
						NXDATALIST2Lib::IRowSettingsPtr pRow = FailQualifyPracticeRecord(m_anImproperlyLinkedPracticeOrdinals[i], "This patient is improperly linked with Canfield Mirror", FALSE);
						pRow->BackColor = 0xFFFF; // Yellow
					}
				}

				if (nOrdinal < nRemoteRecords) {

					// If we get here, we are going to determine whether the remote
					// record at nOrdinal qualifies, and if it does, then put it on
					// the list at the top
					TryQualifyRemoteRecord(nOrdinal);

				} else {
					// We've covered all records from both databases. We're done.
					StopTimedAction();

					// Notify the user the linking is done
					CString strLinkedPts;
					if (m_nExistingLinkedPts > 0) {
						strLinkedPts = FormatString("\n\nAdditionally, %d patient(s) who are currently linked between NexTech Practice and Canfield Mirror "
							"were omitted from the search results.", m_nExistingLinkedPts);
					}
					CString strInvalidPts;
					if (m_anImproperlyLinkedPracticeOrdinals.GetSize() > 0) {
						strInvalidPts = FormatString("\n\n%d disqualified patient(s) were found to be improperly linked either because the patient is missing from Canfield Mirror, "
							"or the spelling of the first and last names are different. Please review these patients after dismissing this feature.",
							m_anImproperlyLinkedPracticeOrdinals.GetSize());
					}

					if (0 == m_dl2Qualify->GetRowCount()) {
						MessageBox(FormatString("The qualification process is complete. There are no unlinked patients who qualify to be linked together.%s%s", strLinkedPts, strInvalidPts),
							"Practice", MB_ICONINFORMATION|MB_OK);
					} else {
						MessageBox(FormatString("The qualification process is complete. %d patient(s) qualify to be linked between NexTech Practice and Canfield Mirror.%s%s"
							"\n\nPlease review which patients should be linked, and press 'Link Qualifying Patients' on "
							"the bottom of the window to complete the linking process.", m_dl2Qualify->GetRowCount(), strLinkedPts, strInvalidPts)
							, "Practice", MB_ICONINFORMATION|MB_OK);
					}
					return;
				}
			}
			SetTimer(IDT_REQUERY, 10, NULL);
		} // if (IDT_REQUERY == nIDEvent) {
		else if (IDT_LINK == nIDEvent) {

			// (c.haag 2008-02-06 09:56) - PLID 28622 - This timer event is fired
			// when we want to link a single patient between Practice and Mirror.
			KillTimer(IDT_LINK);

			// Fail right away if we have no current row
			if (NULL == m_pCurrentLinkRow) {
				ASSERT(FALSE);
				ThrowNxException("Attempted to create a link using a non-existent qualifying row!");
			}

			// Update the progress bar
			long nOrdinal = m_nProgressIteration++;
			m_ProgressRequery.SetPos( (short)( (double) (nOrdinal*10000) / (double)m_nInitialQualifyListSize ) );

			// Get the next row
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = m_pCurrentLinkRow->GetNextRow();

			if (VarBool(m_pCurrentLinkRow->Value[eqcLink])) {

				// If we get here, we want to link the patients for this row
				long nPracUserDefinedID = VarLong(m_pCurrentLinkRow->Value[eqcPracUserDefinedID]);
				CString strRemoteID = VarString(m_pCurrentLinkRow->Value[eqcRemoteID]);
				if (S_OK != Mirror::Link(nPracUserDefinedID, strRemoteID)) {
					
					// If we get here, the linking failed. There should have already been an
					// error message by now, but we will show another one telling the user that
					// the linking has failed. If an exception were thrown, it will be caught
					// by the catch-all at the end of this function, and StopTimedAction() will
					// cease the linking
					MessageBox("An error has occured, and the patient linking process will now stop.", "Practice", MB_ICONERROR|MB_OK);
					StopTimedAction();
					return;
				}
				m_bChangedData = TRUE;

				// Remove the current row
				m_dl2Qualify->RemoveRow(m_pCurrentLinkRow);

			} else {
				// We want to skip this row
			}

			// Advance to the next row
			if (NULL == (m_pCurrentLinkRow = pNextRow)) {

				// If we get here, there are no more rows. We're done.
				StopTimedAction();

				// Disable the link button. We don't want to let the user just merrily and repeatedly click it.
				GetDlgItem(IDC_BTN_LINK_QUALIFYING_PATIENTS)->EnableWindow(FALSE);

				// Now notify the user the linking is done, and dismiss this window
				MessageBox("All selected patients have been linked successfully.", "Practice", MB_ICONINFORMATION|MB_OK);

				// (c.haag 2008-02-12 17:36) - PLID 28622 - Dismiss the dialog if there are
				// no patients in the Qualify list
				if (0 == m_dl2Qualify->GetRowCount()) {
					PostMessage(WM_COMMAND, IDC_BTN_CANCEL);
				}
				return;
			}

			SetTimer(IDT_LINK, 10, NULL);
		} // if (IDT_LINK == nIDEvent) {

	} NxCatchAllCall("Error in CMirrorLinkCommonPtsDlg::OnTimer", StopTimedAction(););
}

void CMirrorLinkCommonPtsDlg::OnBtnSSNMRNHelp()
{
	try {
		// (c.haag 2008-02-06 09:56) - PLID 28622 - This is the help button that pertains to
		// the disabled SSN\MRN linking checkboxes.
		MessageBox("NexTech Practice allows you to transfer a Practice patient ID to "
			"either the SSN or MRN fields of the corresponding Canfield Mirror patient.\n\n"
			"This is a system-wide setting that may not be changed from this window. If you wish "
			"to change this setting, press the 'Close' button, and then click on 'Advanced Settings' "
			"from the 'Integration with Canfield's Mirror Products' window.",
			"Practice", MB_ICONINFORMATION|MB_OK);
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnBtnSSNMRNHelp");
}

void CMirrorLinkCommonPtsDlg::UpdateSSNColumnWidths()
{
	// (c.haag 2008-02-06 09:56) - PLID 28622 - Assigns the width of the
	// SSN columns based on the state of the "Show SSN" checkbox
	if (((CButton*)GetDlgItem(IDC_CHECK_SHOWSSN))->GetCheck()) {
		m_dl2Qualify->GetColumn(eqcPracSSN)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData;
		m_dl2Qualify->GetColumn(eqcPracSSN)->StoredWidth = m_nPracSSNColLen;

		m_dl2Qualify->GetColumn(eqcRemoteSSN)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData;
		m_dl2Qualify->GetColumn(eqcRemoteSSN)->StoredWidth = m_nRemoteSSNColLen;

	} else {
		// Hide the column
		m_nPracSSNColLen = m_dl2Qualify->GetColumn(eqcPracSSN)->StoredWidth;
		m_dl2Qualify->GetColumn(eqcPracSSN)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
		m_dl2Qualify->GetColumn(eqcPracSSN)->StoredWidth = 0;

		m_nRemoteSSNColLen = m_dl2Qualify->GetColumn(eqcRemoteSSN)->StoredWidth;
		m_dl2Qualify->GetColumn(eqcRemoteSSN)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
		m_dl2Qualify->GetColumn(eqcRemoteSSN)->StoredWidth = 0;
	}
}

void CMirrorLinkCommonPtsDlg::UpdateHomePhoneColumnWidths()
{
	// (c.haag 2008-02-06 09:56) - PLID 28622 - Assigns the width of the
	// SSN columns based on the state of the "Show Home Phone" checkbox
	if (((CButton*)GetDlgItem(IDC_CHECK_SHOWHOMEPHONE))->GetCheck()) {
		m_dl2Qualify->GetColumn(eqcPracHomePhone)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData;
		m_dl2Qualify->GetColumn(eqcPracHomePhone)->StoredWidth = m_nPracHomePhoneColLen;

		m_dl2Qualify->GetColumn(eqcRemoteHomePhone)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData;
		m_dl2Qualify->GetColumn(eqcRemoteHomePhone)->StoredWidth = m_nRemoteHomePhoneColLen;

	} else {
		// Hide the column
		m_nPracHomePhoneColLen = m_dl2Qualify->GetColumn(eqcPracHomePhone)->StoredWidth;
		m_dl2Qualify->GetColumn(eqcPracHomePhone)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
		m_dl2Qualify->GetColumn(eqcPracHomePhone)->StoredWidth = 0;

		m_nRemoteHomePhoneColLen = m_dl2Qualify->GetColumn(eqcRemoteHomePhone)->StoredWidth;
		m_dl2Qualify->GetColumn(eqcRemoteHomePhone)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
		m_dl2Qualify->GetColumn(eqcRemoteHomePhone)->StoredWidth = 0;
	}
}

void CMirrorLinkCommonPtsDlg::UpdateBirthdateColumnWidths()
{
	// (c.haag 2008-02-06 09:56) - PLID 28622 - Assigns the width of the
	// SSN columns based on the state of the "Show Birthdate" checkbox
	if (((CButton*)GetDlgItem(IDC_CHECK_SHOWBIRTHDATE))->GetCheck()) {
		m_dl2Qualify->GetColumn(eqcPracBirthdate)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData;
		m_dl2Qualify->GetColumn(eqcPracBirthdate)->StoredWidth = m_nPracBirthdateColLen;

		m_dl2Qualify->GetColumn(eqcRemoteBirthdate)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData;
		m_dl2Qualify->GetColumn(eqcRemoteBirthdate)->StoredWidth = m_nRemoteBirthdateColLen;

	} else {
		// Hide the column
		m_nPracBirthdateColLen = m_dl2Qualify->GetColumn(eqcPracBirthdate)->StoredWidth;
		m_dl2Qualify->GetColumn(eqcPracBirthdate)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
		m_dl2Qualify->GetColumn(eqcPracBirthdate)->StoredWidth = 0;

		m_nRemoteBirthdateColLen = m_dl2Qualify->GetColumn(eqcRemoteBirthdate)->StoredWidth;
		m_dl2Qualify->GetColumn(eqcRemoteBirthdate)->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
		m_dl2Qualify->GetColumn(eqcRemoteBirthdate)->StoredWidth = 0;
	}
}

void CMirrorLinkCommonPtsDlg::OnCheckShowSSN()
{
	try {
		// (c.haag 2008-02-06 09:56) - PLID 28622 - Handles the user pressing
		// the "Show SSN" checkbox
		UpdateSSNColumnWidths();
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnCheckShowSSN");
}

void CMirrorLinkCommonPtsDlg::OnCheckShowHomePhone()
{
	try {
		// (c.haag 2008-02-06 09:56) - PLID 28622 - Handles the user pressing
		// the "Show Home Phone" checkbox
		UpdateHomePhoneColumnWidths();
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnCheckShowHomePhone");
}

void CMirrorLinkCommonPtsDlg::OnCheckShowBirthdate()
{
	try {
		// (c.haag 2008-02-06 09:56) - PLID 28622 - Handles the user pressing
		// the "Show Birthdate" checkbox
		UpdateBirthdateColumnWidths();
	}
	NxCatchAll("Error in CMirrorLinkCommonPtsDlg::OnCheckShowBirthdate");
}
