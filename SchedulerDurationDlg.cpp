// SchedulerDurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "SchedulerDurationDlg.h"
#include "MultiSelectDlg.h"
#include "ChangeDurationDlg.h"
#include "GlobalDrawingUtils.h"
#include "DontShowDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers
using namespace std;

typedef enum
{
	PC_ID = 0,
	PC_NAME,
	PC_DEFAULT_MINUTES,
	PC_MINIMUM_MINUTES,
	PC_MULTIPURPOSEIDS, // (c.haag 2004-06-28 17:11) - Only applies to the combination list
} P_COLUMN;

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSchedulerDurationDlg dialog


CSchedulerDurationDlg::CSchedulerDurationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedulerDurationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSchedulerDurationDlg)
	//}}AFX_DATA_INIT
	m_bModified = FALSE;

	m_bProvidersRequeried = false; //TES 3/26/2010 - PLID 33555
	m_bTypesRequeried = false; //TES 3/26/2010 - PLID 33555
}


void CSchedulerDurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchedulerDurationDlg)
	DDX_Control(pDX, IDC_ADD_DEFAULT_DURATIONS, m_btnAddDefaultDurations);
	DDX_Control(pDX, IDC_RADIO_DOPURPOSECOMBINATIONS, m_btnComboPurpose);
	DDX_Control(pDX, IDC_RADIO_DOINDIVIDUALPURPOSES, m_btnIndivPurpose);
	DDX_Control(pDX, IDC_STATIC_MULTIPROCNOTICE, m_nxstaticMultiprocnotice);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ADDPROCCOMBINATION, m_btnAddProcCombination);
	DDX_Control(pDX, IDC_BTN_DELETEPROCCOMBINATION, m_btnDeleteProcCombination);
	DDX_Control(pDX, IDC_BTN_REMOVEDURATIONS, m_btnRemoveDurations);
	DDX_Control(pDX, IDC_BTN_CHANGEDURATIONS, m_btnChangeDurations);
	DDX_Control(pDX, IDC_BTN_COPYDURATIONS, m_btnCopyDurations);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSchedulerDurationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSchedulerDurationDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CHANGEDURATIONS, OnBtnChangedurations)
	ON_BN_CLICKED(IDC_BTN_REMOVEDURATIONS, OnBtnRemovedurations)
	ON_BN_CLICKED(IDC_RADIO_DOINDIVIDUALPURPOSES, OnRadioDoindividualpurposes)
	ON_BN_CLICKED(IDC_RADIO_DOPURPOSECOMBINATIONS, OnRadioDopurposecombinations)
	ON_BN_CLICKED(IDC_BTN_ADDPROCCOMBINATION, OnBtnAddproccombination)
	ON_BN_CLICKED(IDC_BTN_DELETEPROCCOMBINATION, OnBtnDeleteproccombination)
	ON_BN_CLICKED(IDC_ADD_DEFAULT_DURATIONS, OnAddDefaultDurations)
	ON_BN_CLICKED(IDC_BTN_COPYDURATIONS, OnCopyDurations)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerDurationDlg message handlers

CSchedulerDurationDlg::ECurMode CSchedulerDurationDlg::GetMode()
{
	if (((CButton*)GetDlgItem(IDC_RADIO_DOINDIVIDUALPURPOSES))->GetCheck())
		return eIndividual;
	return eCombinations; 
}

void CSchedulerDurationDlg::EnsureButtons()
{
	if (GetMode() != eIndividual)
	{
		GetDlgItem(IDC_BTN_ADDPROCCOMBINATION)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_DELETEPROCCOMBINATION)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_CHANGEDURATIONS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_REMOVEDURATIONS)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_BTN_ADDPROCCOMBINATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_DELETEPROCCOMBINATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_CHANGEDURATIONS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_REMOVEDURATIONS)->ShowWindow(SW_SHOW);
	}
}

void CSchedulerDurationDlg::EnsureList()
{
	if (GetMode() == eIndividual)
	{
		GetDlgItem(IDC_MULTIPURPOSE_DURATION_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_MULTIPURPOSE_DURATION_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST)->ShowWindow(SW_SHOW);
	}
}

BOOL CSchedulerDurationDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	m_dlProviders = BindNxDataListCtrl(this, IDC_DOCTOR_COMBO, GetRemoteData(), true);
	m_dlTypes = BindNxDataListCtrl(this, IDC_DURATION_TYPE_COMBO, GetRemoteData(), true);
	m_dlPurposes = BindNxDataListCtrl(this, IDC_MULTIPURPOSE_DURATION_LIST, GetRemoteData(), false);
	m_dlPurposeCombinations = BindNxDataListCtrl(this, IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST, GetRemoteData(), false);
	((CButton*)GetDlgItem(IDC_RADIO_DOINDIVIDUALPURPOSES))->SetCheck(1);

	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnAddProcCombination.AutoSet(NXB_NEW);
	m_btnDeleteProcCombination.AutoSet(NXB_DELETE);
	m_btnRemoveDurations.AutoSet(NXB_DELETE);
	m_btnChangeDurations.AutoSet(NXB_MODIFY);
	m_btnCopyDurations.AutoSet(NXB_MODIFY);

	// (c.haag 2004-06-29 11:51) - Fill our appointment type map with the list of
	// current appointment purposes
	try 
	{
		_RecordsetPtr prs = CreateRecordset("SELECT ID, Name FROM AptPurposeT");
		while (!prs->eof)
		{
			m_mapPurpName[AdoFldLong(prs, "ID")] = AdoFldString(prs, "Name");
			prs->MoveNext();
		}
	}
	NxCatchAll("Error filling appointment purpose list");

	// (z.manning, 12/09/05, PLID 18511) - Set the add default duration checkbox
	int nAddDefaultDuration = GetRemotePropertyInt("AddApptDefaultDurations", 1, 0, "<None>", TRUE);
	((CButton*)GetDlgItem(IDC_ADD_DEFAULT_DURATIONS))->SetCheck(nAddDefaultDuration);

	// (c.haag 2004-06-28 17:30) - Fill our array with the current data
	LoadDurations();
	EnsureList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

long CSchedulerDurationDlg::GetSelectedProviderID()
{
	return VarLong(m_dlProviders->GetValue(m_dlProviders->GetCurSel(), 0));
}

long CSchedulerDurationDlg::GetSelectedAptTypeID()
{
	return VarLong(m_dlTypes->GetValue(m_dlTypes->GetCurSel(), 0));
}

void CSchedulerDurationDlg::ReflectDurations()
{
	// (c.haag 2004-06-28 16:01) - We need to go through our array of user made changes
	// and alter the default and minimum columns in the datalist
	try {
		if (GetMode() == eIndividual)
		{
			CDurationSet s;
			for (long i=0; i < m_dlPurposes->GetRowCount(); i++)
			{
				long nIndex;
				s.m_nProviderID = GetSelectedProviderID();
				s.m_nAptTypeID = GetSelectedAptTypeID();
				s.m_adwPurposeIDs.RemoveAll();
				s.m_adwPurposeIDs.Add(VarLong(m_dlPurposes->GetValue(i,PC_ID)));
				if (-1 != (nIndex = FindDurationSet(s)))
				{
					s.Copy(*m_aDurations[nIndex]);
					if (!s.m_bEnforced)
					{
						COleVariant vNull;
						vNull.vt = VT_NULL;
						m_dlPurposes->PutValue(i,PC_DEFAULT_MINUTES,vNull);
						m_dlPurposes->PutValue(i,PC_MINIMUM_MINUTES,vNull);
					}
					else
					{
						m_dlPurposes->PutValue(i,PC_DEFAULT_MINUTES,s.m_nDefaultLength);
						m_dlPurposes->PutValue(i,PC_MINIMUM_MINUTES,s.m_nMinimumLength);
					}
				}
			}
		}
		else if (GetMode() == eCombinations)
		{			
			m_dlPurposeCombinations->Clear();
			for (long i=0; i < m_aDurations.GetSize(); i++)
			{
				// (c.haag 2004-06-29 12:07) - Calculate what we want to fill in the list
				CString strPurposes, strPurposeIDs;
				CDurationSet* s = m_aDurations[i];
				if (!s->m_bEnforced) continue;
				if (s->m_nProviderID != GetSelectedProviderID()) continue;
				if (s->m_nAptTypeID != GetSelectedAptTypeID()) continue;

				for (long j=0; j < s->m_adwPurposeIDs.GetSize(); j++)
				{
					CString str;
					strPurposes += m_mapPurpName[s->m_adwPurposeIDs[j]] + ", ";
					str.Format("%d,", s->m_adwPurposeIDs[j]);
					strPurposeIDs += str;
				}
				if (strPurposes.GetLength()) {
					strPurposes = strPurposes.Left( strPurposes.GetLength() - 2 );
				}
				else {
					strPurposes = "< No Purposes >";
				}
				if (strPurposeIDs.GetLength()) strPurposeIDs = strPurposeIDs.Left( strPurposeIDs.GetLength() - 1 );
				
				// (c.haag 2004-06-29 12:07) - Add the row to the list
				IRowSettingsPtr pRow = m_dlPurposeCombinations->GetRow(-1);
				pRow->Value[PC_ID] = i;
				pRow->Value[PC_NAME] = _bstr_t(strPurposes);
				pRow->Value[PC_DEFAULT_MINUTES] = s->m_nDefaultLength;
				pRow->Value[PC_MINIMUM_MINUTES] = s->m_nMinimumLength;
				pRow->Value[PC_MULTIPURPOSEIDS] = _bstr_t(strPurposeIDs);
				m_dlPurposeCombinations->AddRow(pRow);
			}
		}
	}
	NxCatchAll("Error refreshing data");
}

BOOL CSchedulerDurationDlg::WarnOfDuplicates()
{

// (b.spivey - February 4th, 2014) - PLID 60379 - We use this to flag duplicates found. 
	bool bDuplicateFound = false; 

	//Temporary struct to help report duplicates. 
	struct BaseDuration {
		long nProviderID;
		long nAptTypeID;
		vector<long> vecAptPurposeIDs;
		CString strFormattedPurposes; 

		BaseDuration(CDurationSet s, CString str) {
			nProviderID = s.m_nProviderID;
			nAptTypeID = s.m_nAptTypeID;

			foreach(long n, s.m_adwPurposeIDs) {
				vecAptPurposeIDs.push_back(n); 
			}
			sort(vecAptPurposeIDs.begin(), vecAptPurposeIDs.end());

			strFormattedPurposes = str; 
		};

		//This returns a string value that should be unique... unless there is a duplicate. 
		CString GetUniquifier() {
			CString strUniquifier = "", str = "";
			
			if (nProviderID > 0) {
				str.Format("%li;", nProviderID); 
				strUniquifier += str;
			}

			if (nAptTypeID > 0) {
				str.Format("%li;", nAptTypeID); 
				strUniquifier += str;
			}

			foreach(long n, vecAptPurposeIDs) {
				str.Format("%li;", n); 
				strUniquifier += str;
			}

			return strUniquifier; 
		};
	};

	//Finally, this will be used to build a warning string of duplicates. 
	map<CString, BaseDuration> mapDuplicates;

	for (long i=0; i < m_aDurations.GetSize(); i++)
	{
		if(!m_aDurations[i]->m_bEnforced)
			continue;

		CDurationSet* s = m_aDurations[i];
		if (-1 != FindDurationSet(*s, i))
		{
			// (c.haag 2004-06-29 14:16) - Report the fact we have a duplication
			CString str;
			for (long i=0; i < s->m_adwPurposeIDs.GetSize(); i++)
			{
				str += m_mapPurpName[s->m_adwPurposeIDs[i]] + ", ";
			}
			if (str.GetLength())
			{
				str = str.Left( str.GetLength() - 2 );
				if (s->m_adwPurposeIDs.GetSize() > 1) {
					//TES 3/29/2010 - PLID 37893 - We don't allow duplicate durations to ever exist any more, so this is now an error
					// rather than a warning.

					// (b.spivey - February 4th, 2014) - PLID 60379 - A duplicate has been found. 
					//	 Flag it and add it to our duplicates list
					bDuplicateFound = true; 

					BaseDuration bd(*s, str); 
					//If the uniquifier isn't found, we can add it to the map.
					if(mapDuplicates.find(bd.GetUniquifier()) == mapDuplicates.end()) {
						mapDuplicates.insert(pair<CString, BaseDuration>(bd.GetUniquifier(), bd)); 
					}
				}
				else {
					//TES 3/29/2010 - PLID 37893 - We don't allow duplicate durations to ever exist any more, so this is now an error
					// rather than a warning.

					// (b.spivey - February 4th, 2014) - PLID 60379 - A duplicate has been found. 
					//	 Flag it and add it to our duplicates list
					bDuplicateFound = true; 

					BaseDuration bd(*s, str); 
					//If the uniquifier isn't found, we can add it to the map.
					if(mapDuplicates.find(bd.GetUniquifier()) == mapDuplicates.end()) {
						mapDuplicates.insert(pair<CString, BaseDuration>(bd.GetUniquifier(), bd)); 
					}
				}
			}
			else
			{
				//TES 3/29/2010 - PLID 37893 - We don't allow duplicate durations to ever exist any more, so this is now an error
				// rather than a warning.

				// (b.spivey - February 4th, 2014) - PLID 60379 - A duplicate has been found. 
				//	 Flag it and add it to our duplicates list
				bDuplicateFound = true;

				BaseDuration bd(*s, "< No Purpose >"); 
				//If the uniquifier isn't found, we can add it to the map.
				if(mapDuplicates.find(bd.GetUniquifier()) == mapDuplicates.end()) {
					mapDuplicates.insert(pair<CString, BaseDuration>(bd.GetUniquifier(), bd)); 
				}
				
			}
		}
	}

	// (b.spivey - February 4th, 2014) - PLID 60379 - a duplicate has been found, lets report it. 
	if (bDuplicateFound) {

		std::map<CString, BaseDuration>::iterator it = mapDuplicates.begin();
		CString strErrorString = "";

		//The map should be sorted by provider, then appointment type, then finally purposes because std::map will 
		//	 automatically sort based on key, even when inserting into it. 
		while (it != mapDuplicates.end()) {
			BaseDuration bd = it->second; 
			long nRowIndex = m_dlProviders->FindByColumn(0, bd.nProviderID, -1, FALSE);
			CString strProvider = "< No Provider >";
			//If the row index is not found, then the provider is inactive or no longer exists. We allowed this to exist 
			//	 before, we'll leave it be now. 
			if (nRowIndex >= 0) {
				strProvider = VarString(m_dlProviders->GetValue(nRowIndex, 4), "");
			}
			else {
				it++;
				continue; //This provider doesn't exist in the list. 
			}
			strErrorString += strProvider; 


			nRowIndex = m_dlTypes->FindByColumn(0, bd.nAptTypeID, -1, FALSE);
			CString strAptType = " -- < No Appointment Type >";
			if (nRowIndex >= 0) {
				strAptType = " -- " + VarString(m_dlTypes->GetValue(nRowIndex, 1), "");
			}
			strErrorString += strAptType;
			

			strErrorString += " -- " + bd.strFormattedPurposes + "\r\n"; 
			it++;
		}

		//If we have a valid error string, lets report our findings and stop the saving. 
		if (!strErrorString.IsEmpty()) {
			MessageBox("Duplicate durations have been detected. This list of durations must be " 
				"cleaned up before Practice can save your changes.\r\n\r\n" + strErrorString,
				"Duplicate Durations Detected", MB_OK|MB_ICONWARNING);
			return TRUE; 
		}
	}
	return FALSE;
}

// (b.spivey - February 4th, 2014) - PLID 60379 - Added a default for when we're searching with ID in mind. 
long CSchedulerDurationDlg::FindDurationSet(const CDurationSet& s, long nExclude /* = -1*/, bool bFindAbsoluteDurationSet /*= false*/)
{
	for (long i=0; i < m_aDurations.GetSize(); i++)
	{
		if (i == nExclude) continue;
		CDurationSet* p = m_aDurations[i];

		if (p->m_bEnforced &&
			p->m_nProviderID == s.m_nProviderID &&
			p->m_nAptTypeID == s.m_nAptTypeID &&
			ArraysMatch(s.m_adwPurposeIDs, p->m_adwPurposeIDs) && 
			(!bFindAbsoluteDurationSet || p->m_nSetID == s.m_nSetID))
		{
			break; // (c.haag 2004-06-22 09:18) - We found a match
		}
	}
	if (i == m_aDurations.GetSize())
		return -1;
	return i;
}

BOOL CSchedulerDurationDlg::ArraysMatch(const CDWordArray& a1, const CDWordArray& a2)
{
	if (a1.GetSize() != a2.GetSize())
		return FALSE;

	for (long j=0; j < a1.GetSize(); j++)
	{
		for (long k=0; k < a2.GetSize(); k++)
		{
			if (a2[k] == a1[j])
				break;
		}
		if (k == a2.GetSize())
			return FALSE;
	}
	return TRUE;
}

void CSchedulerDurationDlg::LoadDurations()
{
	try {
		//DRT 5/9/2006 - PLID 20440 - Greatly increased the speed of this code.  Previously, we got a list of all the DurationSets.  Then we looped through
		//	that list, and ran a query to select all the appointment purposes.  We then looped through that query and added them to an array.  I changed this
		//	so that we load it all in 1 query, selecting all the details, with their DurationSet info, ordered by DurationSet.  We then loop once, creating
		//	a new DurationSet when we hit a new ID, and adding the purpose to the array otherwise.

		long nLastSetID = -9999;
		CDurationSet* pds = NULL;
		_RecordsetPtr prsData = CreateRecordset("SELECT ID, ProviderID, AptTypeID, DurationMinimum, DurationMinutes, ProviderSchedDefDurationDetailT.AptPurposeID "
			"FROM ProviderSchedDefDurationT "
			"LEFT JOIN ProviderSchedDefDurationDetailT ON ProviderSchedDefDurationT.ID = ProviderSchedDefDurationDetailT.ProviderSchedDefDurationID "
			"ORDER BY ID");

		// (b.spivey - February 4th, 2014) - PLID 60379 - The "originals" only matter if they're the very latest in data. 
		//	 Always update this array.
		for (int i = 0; i < m_aOriginalDurations.GetSize(); i++) {
			delete m_aOriginalDurations.GetAt(i); 
		}
		m_aOriginalDurations.RemoveAll();

		while (!prsData->eof) {
			long nSetID = AdoFldLong(prsData, "ID");

			if(nSetID != nLastSetID) {
				//We are on to a new set.  Clear the old one.
				if(pds != NULL) {		//first time only NULL
					//It's possible this stuff changed. Lets make sure to update it. 
					if(!UpdateSingleDurationSet(pds)) {
						m_aDurations.Add(pds);
					}
					//TES 3/29/2010 - PLID 37893 - We want to track the original setup to make sure our change tracking is correct.
					CDurationSet *pdsOrig = new CDurationSet(*pds);
					m_aOriginalDurations.Add(pdsOrig);
				}

				//Create a new set
				pds = new CDurationSet;

				//fill in the default info - this is the same for all records in our search
				pds->m_nSetID = nSetID;
				pds->m_nProviderID = AdoFldLong(prsData, "ProviderID");
				pds->m_nAptTypeID = AdoFldLong(prsData, "AptTypeID");
				pds->m_bEnforced = TRUE;
				pds->m_nDefaultLength = AdoFldLong(prsData, "DurationMinutes");
				pds->m_nMinimumLength = AdoFldLong(prsData, "DurationMinimum");
			}

			//Whether we were on the first row of a new set, or we are on a consecutive row of a set, 
			//	there is a purpose ID (if not null).  If this is NULL, it means we're on a DurationSet
			//	that does not have any purposes.
			_variant_t var = prsData->Fields->Item["AptPurposeID"]->Value;
			if(var.vt == VT_I4)
				pds->m_adwPurposeIDs.Add(VarLong(var));

			//the setID from this loop becomes to the "last set id"
			nLastSetID = pds->m_nSetID;

			//Move to next record
			prsData->MoveNext();
		}

		//we have 1 left after the end
		if(pds) {
			if(!UpdateSingleDurationSet(pds)) {
				m_aDurations.Add(pds);
			}
			//TES 3/29/2010 - PLID 37893 - We want to track the original setup to make sure our change tracking is correct.
			CDurationSet *pdsOrig = new CDurationSet(*pds);
			m_aOriginalDurations.Add(pdsOrig);
		}

	} NxCatchAll("Error loading default appointment durations");
}

void CSchedulerDurationDlg::SaveDurations()
{
	CMap<long, long, CString, CString&> mapProviders;
	CMap<long, long, CString, CString&> mapTypes;
	CMap<long, long, CString, CString&> mapPurposes;
	CWaitCursor wc;
	try {
		// (c.haag 2004-06-29 13:26) - Do everything in one shot
		//CString strSQLFinal = "DELETE FROM ProviderSchedDefDurationDetailT;DELETE FROM ProviderSchedDefDurationT;";

		// (c.haag 2004-06-29 13:36) - Load the array of active providers and appointment
		// types so we know they all exist at save time. We deal with purposes later on down.
		//TES 3/26/2010 - PLID 37893 - Pull the names for auditing, and let's go ahead and pull the purposes while we're at it.
		_RecordsetPtr prsProviders = CreateRecordset("SELECT PersonID, Last + ', ' + First + ' ' + Middle AS Name "
			"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID");
		while (!prsProviders->eof)
		{
			mapProviders[AdoFldLong(prsProviders, "PersonID")] = AdoFldString(prsProviders, "Name");
			prsProviders->MoveNext();
		}
		prsProviders->Close();
		_RecordsetPtr prsTypes = CreateRecordset("SELECT ID, Name FROM AptTypeT");
		while (!prsTypes->eof)
		{
			mapTypes[AdoFldLong(prsTypes, "ID")] = AdoFldString(prsTypes, "Name");
			prsTypes->MoveNext();
		}
		prsTypes->Close();
		_RecordsetPtr prsPurposes = CreateRecordset("SELECT ID, Name FROM AptPurposeT");
		while (!prsPurposes->eof)
		{
			mapPurposes[AdoFldLong(prsPurposes, "ID")] = AdoFldString(prsPurposes, "Name");
			prsPurposes->MoveNext();
		}
		prsPurposes->Close();
	}NxCatchAll("Error saving default durations while trying to map providers and types.");

	//TES 3/29/2010 - PLID 37893 - Go through all the original durations, and make sure that original duration + change = current duration.
	for(int i = 0; i < m_aOriginalDurations.GetSize(); i++) {
		CDurationSet *pdsOrig = m_aOriginalDurations[i];
		//TES 3/29/2010 - PLID 37893 - Was this changed?
		long nChangeIndex = FindChangedDuration(*pdsOrig, true);
		if(nChangeIndex == -1) {
			//TES 3/29/2010 - PLID 37893 - No, so it should be the same in our current set.
			long nNewIndex = FindDurationSet(*pdsOrig, -1, true);
			if(nNewIndex == -1) {
				//TES 3/29/2010 - PLID 37893 - Throw an exception
				AfxThrowNxException("Unchanged original duration could not be found in saved duration data.");
			}
			else {
				if(!((*pdsOrig) == (*m_aDurations[nNewIndex]))) {
					//TES 3/29/2010 - PLID 37893 - Throw an exception
					AfxThrowNxException("Untracked change found in saved duration data.");
				}
			}
		}
		else {
			//TES 3/29/2010 - PLID 37893 - Yes, so it should be in our current set, with whatever change was made.
			ChangedDuration cd = m_aChangedDurations[nChangeIndex];
			long nNewIndex = FindDurationSet(*pdsOrig, -1, true);
			if(nNewIndex == -1) {
				if(cd.nNewDefaultLength != 0 || cd.nNewMinimumLength != 0) {
					//TES 3/29/2010 - PLID 37893 - Throw an exception
					AfxThrowNxException("Changed duration could not be found in saved duration data.");
				}
			}
			else {
				CDurationSet *pdsNew = m_aDurations[nNewIndex];
				if((pdsNew->m_nDefaultLength != cd.nNewDefaultLength || pdsNew->m_nMinimumLength != cd.nNewMinimumLength) && pdsNew->m_nSetID == cd.nSetID) {
					//TES 3/29/2010 - PLID 37893 - Throw an exception
					AfxThrowNxException("Duration change does not match saved duration data.");
				}
			}
		}
	}

	long nAuditTransactionID = -1;
	try{
		//TES 3/25/2010 - PLID 37893 - We used to just delete everything and re-write all the durations.  Now, we go through each of the
		// changes we've recorded, and apply them.
		CString strSql = BeginSqlBatch();
		//TES 3/25/2010 - PLID 37893 - Start off by going through everything that was deleted.
		CDWordArray dwaDeletedSets;
		long nNextSetID = NewNumber("ProviderSchedDefDurationT", "ID");
		for(int i = 0; i < m_aChangedDurations.GetSize(); i++) {
			ChangedDuration cd = m_aChangedDurations[i];
			//TES 3/26/2010 - PLID 37893 - Pull the names for auditing.
			CString strProvName, strTypeName;
			CString strPurposes;
			if (!mapProviders.Lookup(cd.nProviderID, strProvName) || !mapTypes.Lookup(cd.nAptTypeID, strTypeName))
				continue;
			for(int j = 0; j < cd.adwPurposeIDs.GetSize(); j++) {
				CString strPurpose;
				if(!mapPurposes.Lookup(cd.adwPurposeIDs[j], strPurpose)) {
					continue;
				}
				else {
					strPurposes += strPurpose + ",";
				}
			}
			strPurposes.TrimRight(",");
			if(strPurposes.IsEmpty()) {
				strPurposes = "<None>";
			}
			//TES 3/25/2010 - PLID 37893 - This duration was deleted if both new lengths are 0.
			if(cd.nNewDefaultLength == 0 && cd.nNewMinimumLength == 0) {
				//TES 3/25/2010 - PLID 37893 - If the SetID is -1, then there's nothing to delete.
				if(cd.nSetID != -1) {
					AddStatementToSqlBatch(strSql, "DELETE FROM ProviderSchedDefDurationDetailT WHERE ProviderSchedDefDurationID = %li", cd.nSetID);
					AddStatementToSqlBatch(strSql, "DELETE FROM ProviderSchedDefDurationT WHERE ID = %li", cd.nSetID);
					//TES 3/25/2010 - PLID 37893 - Remember we deleted this set so we don't try to update it later.
					dwaDeletedSets.Add(cd.nSetID);

					//TES 3/26/2010 - PLID 37893 - Audit.
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, "", nAuditTransactionID, aeiDefaultDurationDeleted, cd.nSetID, strProvName + " - " + strTypeName + " - " + strPurposes, "<Deleted>", aepMedium);
				}
			}
		}
		//TES 3/25/2010 - PLID 37893 - Now go through all the rest.
		for(i = 0; i < m_aChangedDurations.GetSize(); i++) {
			ChangedDuration cd = m_aChangedDurations[i];
			if(cd.nNewDefaultLength != 0 || cd.nNewMinimumLength != 0) {
				//TES 3/26/2010 - PLID 37893 - Pull the names for auditing.
				CString strProvName, strTypeName;
				CString strPurposes;
				if (!mapProviders.Lookup(cd.nProviderID, strProvName) || !mapTypes.Lookup(cd.nAptTypeID, strTypeName))
					continue;
				for(int j = 0; j < cd.adwPurposeIDs.GetSize(); j++) {
					CString strPurpose;
					if(!mapPurposes.Lookup(cd.adwPurposeIDs[j], strPurpose)) {
						continue;
					}
					else {
						strPurposes += strPurpose + ",";
					}
				}
				strPurposes.TrimRight(",");
				if(strPurposes.IsEmpty()) {
					strPurposes = "<None>";
				}
				//TES 3/25/2010 - PLID 37893 - Make sure something actually changed.
				if(cd.nOldDefaultLength != cd.nNewDefaultLength || cd.nOldMinimumLength != cd.nNewMinimumLength) {
					//TES 3/25/2010 - PLID 37893 - Now, do we need to insert, or update?
					long nSetID = cd.nSetID;
					bool bInsert = false;
					if(nSetID == -1) {
						//TES 3/25/2010 - PLID 37893 - This is new, definitely insert.
						bInsert = true;
					}
					else {
						//TES 3/25/2010 - PLID 37893 - If this set was deleted, then we need to re-insert it.
						for(int j = 0; j < dwaDeletedSets.GetSize() && !bInsert; j++) {
							if(dwaDeletedSets[j] == nSetID) bInsert = true;
						}
					}
					if(bInsert) {
						//TES 3/25/2010 - PLID 37893 - Insert the new records.
						nSetID = nNextSetID;
						nNextSetID++;
						AddStatementToSqlBatch(strSql, "INSERT INTO ProviderSchedDefDurationT (ID, ProviderID, AptTypeID, DurationMinutes, DurationMinimum) "
							"VALUES (%li, %li, %li, %li, %li)", nSetID, cd.nProviderID, cd.nAptTypeID, cd.nNewDefaultLength, cd.nNewMinimumLength);
						for(int j = 0; j < cd.adwPurposeIDs.GetSize(); j++) {
							AddStatementToSqlBatch(strSql, "INSERT INTO ProviderSchedDefDurationDetailT (ProviderSchedDefDurationID, AptPurposeID) "
								"SELECT %li, %li FROM AptPurposeT WHERE ID = %li", nSetID, cd.adwPurposeIDs[j], cd.adwPurposeIDs[j]);
						}

						//TES 3/26/2010 - PLID 37893 - Audit.
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, "", nAuditTransactionID, aeiDefaultDurationCreated, cd.nSetID, strProvName + " - " + strTypeName + " - " + strPurposes, "Default: " + AsString(cd.nNewDefaultLength) + ", Minimum: " + AsString(cd.nNewMinimumLength), aepMedium);
					}
					else {
						//TES 3/25/2010 - PLID 37893 - Just update the default and minimum lengths.
						AddStatementToSqlBatch(strSql, "UPDATE ProviderSchedDefDurationT SET DurationMinutes = %li, DurationMinimum = %li WHERE ID = %li",
							cd.nNewDefaultLength, cd.nNewMinimumLength, nSetID);
						//TES 3/26/2010 - PLID 37893 - Audit.
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, "", nAuditTransactionID, aeiDefaultDurationChanged, cd.nSetID, strProvName + " - " + strTypeName + " - " + strPurposes + " - Default: " + AsString(cd.nOldDefaultLength) + ", Minimum: " + AsString(cd.nOldMinimumLength), "Default: " + AsString(cd.nNewDefaultLength) + ", Minimum: " + AsString(cd.nNewMinimumLength), aepMedium);
					}
				}
			}
		}
		if(!strSql.IsEmpty()) {
			//TES 3/25/2010 - PLID 37893 - Fire away!
			ExecuteSqlBatch(strSql);
		}

		//TES 3/26/2010 - PLID 37893 - Commit our auditing.
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}
	}
	NxCatchAll("Error saving default durations");
	//TES 3/26/2010 - PLID 37893 - Rollback any auditing we may have started.
	if(nAuditTransactionID != -1) {
		RollbackAuditTransaction(nAuditTransactionID);
	}
}

BEGIN_EVENTSINK_MAP(CSchedulerDurationDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSchedulerDurationDlg)
	ON_EVENT(CSchedulerDurationDlg, IDC_DOCTOR_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedDoctorCombo, VTS_I2)
	ON_EVENT(CSchedulerDurationDlg, IDC_DURATION_TYPE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedTypeCombo, VTS_I2)
	ON_EVENT(CSchedulerDurationDlg, IDC_MULTIPURPOSE_DURATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPurposeCombo, VTS_I2)
	ON_EVENT(CSchedulerDurationDlg, IDC_DOCTOR_COMBO, 16 /* SelChosen */, OnSelChosenDoctorCombo, VTS_I4)
	ON_EVENT(CSchedulerDurationDlg, IDC_DURATION_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenTypeCombo, VTS_I4)
	ON_EVENT(CSchedulerDurationDlg, IDC_MULTIPURPOSE_DURATION_LIST, 15 /* SelSet */, OnSelSetMultipurposeDurationList, VTS_I4)
	ON_EVENT(CSchedulerDurationDlg, IDC_MULTIPURPOSE_DURATION_LIST, 10 /* EditingFinished */, OnEditingFinishedMultipurposeDurationList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSchedulerDurationDlg, IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST, 19 /* LeftClick */, OnLeftClickMultipurposeDurationCombinationList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSchedulerDurationDlg, IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST, 10 /* EditingFinished */, OnEditingFinishedMultipurposeDurationCombinationList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSchedulerDurationDlg, IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST, 15 /* SelSet */, OnSelSetMultipurposeDurationCombinationList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSchedulerDurationDlg::OnRequeryFinishedDoctorCombo(short nFlags) 
{
	if (m_dlProviders->GetRowCount() == 0) return;
	m_dlProviders->CurSel = 0;
	m_bProvidersRequeried = true;
	//TES 3/26/2010 - PLID 33555 - If both lists are loaded now, go ahead and fill the purposes
	if(m_bTypesRequeried) {
		RequeryPurposes();
	}
}

void CSchedulerDurationDlg::OnRequeryFinishedTypeCombo(short nFlags) 
{
	if (m_dlTypes->GetRowCount() == 0) return;
	m_dlTypes->CurSel = 0;
	m_bTypesRequeried = true;
	//TES 3/26/2010 - PLID 33555 - If both lists are loaded now, go ahead and fill the purposes
	if(m_bProvidersRequeried) {
		RequeryPurposes();
	}
}

void CSchedulerDurationDlg::OnRequeryFinishedPurposeCombo(short nFlags) 
{
	ReflectDurations();
}

void CSchedulerDurationDlg::OnSelChosenDoctorCombo(long nNewSel) 
{
	if(nNewSel == -1) m_dlProviders->CurSel = 0;
	RequeryPurposes();
}

void CSchedulerDurationDlg::OnSelChosenTypeCombo(long nNewSel) 
{
	if(nNewSel == -1) m_dlTypes->CurSel = 0;
	RequeryPurposes();
}

void CSchedulerDurationDlg::RequeryPurposes()
{
	try {		
		if (GetMode() == eIndividual)
		{
			// (c.haag 2004-06-28 17:09) - Requery the individual purposes list. We
			// don't fill in the default durations until later.
			CString strWhere;
			long nAptTypeID = GetSelectedAptTypeID();
			// (c.haag 2008-12-17 17:17) - PLID 32376 - Suppress inactive procedures
			strWhere.Format("ID IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %d) AND ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)", nAptTypeID);
			m_dlPurposes->WhereClause = _bstr_t(strWhere);
			m_dlPurposes->CurSel = -1;
			m_dlPurposes->Requery();
		}
		else
		{
			// (c.haag 2004-06-29 11:43) - There is no query for the purpose combination
			// list; we just fill them in the list from our array.
			ReflectDurations();
		}
	}
	NxCatchAll("Error requerying purposes");
}

void CSchedulerDurationDlg::OnOK()
{	
	try {
		//TES 3/29/2010 - PLID 37893 - WarnOfDuplicates() throws exceptions now.
		// (b.spivey - February 4th, 2014) - PLID 60379 - Refresh the list and update the interface. 
		LoadDurations(); 
		EnsureList(); 
		ReflectDurations(); 
		if (WarnOfDuplicates()) return;
		if (m_bModified) SaveDurations(); // (c.haag 2004-06-22 09:12) - Now write to the data.

		CDialog::OnOK();
	} NxCatchAll("Error in CSchedulerDurationDlg::OnOK()");
}

void CSchedulerDurationDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	for (long i=0; i < m_aDurations.GetSize(); i++)
	{
		delete m_aDurations[i];
	}

	//TES 3/29/2010 - PLID 37893 - Clean up our array for the original state.
	for (long i=0; i < m_aOriginalDurations.GetSize(); i++) 
	{
		delete m_aOriginalDurations[i];
	}
}

void CSchedulerDurationDlg::OnSelSetMultipurposeDurationList(long nRow) 
{
	EnsureButtons();
}

void CSchedulerDurationDlg::OnSelSetMultipurposeDurationCombinationList(long nRow) 
{
	EnsureButtons();	
}

void CSchedulerDurationDlg::OnBtnChangedurations() 
{
	CChangeDurationDlg dlg(this);	
	if (!m_dlPurposes->GetFirstSelEnum()) return;
	if (IDOK == dlg.Open(0,0))
	{
		// (c.haag 2004-06-28 13:01) - Assign durations to each item
		// in the list, and preserve them in case the user goes back
		// to that purpose list later.
		long p = m_dlPurposes->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		while (p)
		{	
			CDurationSet s;
			m_dlPurposes->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			s.m_nProviderID = GetSelectedProviderID();
			s.m_nAptTypeID = GetSelectedAptTypeID();
			s.m_adwPurposeIDs.Add(VarLong(pRow->Value[0]));
			s.m_bEnforced = TRUE;
			pRow->Value[2] = s.m_nDefaultLength = dlg.GetDefaultDuration();
			pRow->Value[3] = s.m_nMinimumLength = dlg.GetMinimumDuration();
			pDisp->Release();

			//TES 3/25/2010 - PLID 37893 - Track what the previous lengths were.
			long nOldDefaultLength = -1, nOldMinimumLength = -1;

			// (c.haag 2004-06-28 15:01) - Update the array of preserved values
			// so that we can save all of our changes in one shot later.
			long nIndex = FindDurationSet(s, -1, true);
			if (-1 == nIndex)
			{
				// (c.haag 2004-06-22 09:16) - If we didn't find it, add it to the
				// array.
				nIndex = m_aDurations.GetSize();
				m_aDurations.Add(new CDurationSet(s));
				//TES 3/25/2010 - PLID 37893 - This previously didn't exist, so the old lengths are 0.
				nOldDefaultLength = 0;
				nOldMinimumLength = 0;
			}
			else
			{
				// (c.haag 2004-06-22 09:16) - Otherwise, assign this to the array

				//TES 3/25/2010 - PLID 37893 - Remember the SetID
				s.m_nSetID = m_aDurations[nIndex]->m_nSetID;
				//TES 3/25/2010 - PLID 37893 - Remember the original lengths.
				nOldDefaultLength = m_aDurations[nIndex]->m_nDefaultLength;
				nOldMinimumLength = m_aDurations[nIndex]->m_nMinimumLength;
				m_aDurations[nIndex]->Copy(s);
			}

			//TES 3/25/2010 - PLID 37893 - Now, remember this change.  Have we changed this set already?
			long nChangeIndex = FindChangedDuration(s);
			if(nChangeIndex == -1) {
				//TES 3/25/2010 - PLID 37893 - Nope, so create a new record, based off the CDurationSet object.
				ChangedDuration cd(s);
				cd.nOldDefaultLength = nOldDefaultLength;
				cd.nOldMinimumLength = nOldMinimumLength;
				cd.nNewDefaultLength = s.m_nDefaultLength;
				cd.nNewMinimumLength = s.m_nMinimumLength;
				m_aChangedDurations.Add(cd);
			}
			else {
				//TES 3/25/2010 - PLID 37893 - Yep, so just update the new lengths (the old lengths will be whatever they were originally).
				ChangedDuration cd = m_aChangedDurations[nChangeIndex];
				cd.nNewDefaultLength = s.m_nDefaultLength;
				cd.nNewMinimumLength = s.m_nMinimumLength;
				m_aChangedDurations.SetAt(nChangeIndex, cd);
			}
		}
		m_bModified = TRUE;
	}
}

void CSchedulerDurationDlg::OnBtnRemovedurations() 
{
	COleVariant v;
	long p = m_dlPurposes->GetFirstSelEnum();
	LPDISPATCH pDisp = NULL;
	v.vt = VT_NULL;
	while (p)
	{	
		CDurationSet s;
		m_dlPurposes->GetNextSelEnum(&p, &pDisp);
		IRowSettingsPtr pRow(pDisp);
		s.m_nProviderID = GetSelectedProviderID();
		s.m_nAptTypeID = GetSelectedAptTypeID();
		s.m_adwPurposeIDs.Add(VarLong(pRow->Value[0]));
		s.m_nDefaultLength = 0;
		s.m_nMinimumLength = 0;
		s.m_bEnforced = FALSE;
		pRow->Value[2] = v;
		pRow->Value[3] = v;
		pDisp->Release();

		//TES 3/25/2010 - PLID 37893 - Remember the previous lengths
		long nOldDefaultLength = -1, nOldMinimumLength = -1;

		// (c.haag 2004-06-28 15:01) - Update the array of preserved values
		// so that we can save all of our changes in one shot later.
		long nIndex = FindDurationSet(s, -1, true);
		if (-1 == nIndex)
		{
			// (c.haag 2004-06-22 09:16) - If we didn't find it, add it to the
			// array.
			nIndex = m_aDurations.GetSize();
			m_aDurations.Add(new CDurationSet(s));
			//TES 3/25/2010 - PLID 37893 - This didn't exist, so the old lengths were 0.
			nOldDefaultLength = 0;
			nOldMinimumLength = 0;
		}
		else
		{
			// (c.haag 2004-06-22 09:16) - Otherwise, assign this to the array
			//TES 3/25/2010 - PLID 37893 - Remember the Set ID
			s.m_nSetID = m_aDurations[nIndex]->m_nSetID;
			//TES 3/25/2010 - PLID 37893 - Pull the lengths before we update them.
			nOldDefaultLength = m_aDurations[nIndex]->m_nDefaultLength;
			nOldMinimumLength = m_aDurations[nIndex]->m_nMinimumLength;
			m_aDurations[nIndex]->Copy(s);
		}
		//TES 3/25/2010 - PLID 37893 - Have we previously changed this set?
		long nChangeIndex = FindChangedDuration(s, true);
		if(nChangeIndex == -1) {
			//TES 3/25/2010 - PLID 37893 - Nope, create a new change based on this CDurationSet
			ChangedDuration cd(s);
			cd.nOldDefaultLength = nOldDefaultLength;
			cd.nOldMinimumLength = nOldMinimumLength;
			cd.nNewDefaultLength = s.m_nDefaultLength;
			cd.nNewMinimumLength = s.m_nMinimumLength;
			m_aChangedDurations.Add(cd);
		}
		else {
			//TES 3/25/2010 - PLID 37893 - Just set the new lengths, the old ones will be whatever they were originally.
			ChangedDuration cd = m_aChangedDurations[nChangeIndex];
			cd.nNewDefaultLength = s.m_nDefaultLength;
			cd.nNewMinimumLength = s.m_nMinimumLength;
			m_aChangedDurations.SetAt(nChangeIndex, cd);
		}
		m_bModified = TRUE;
	}
}

void CSchedulerDurationDlg::OnEditingFinishedMultipurposeDurationList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (!bCommit) return;
	// (c.haag 2004-06-28 16:27) - Make sure this purpose is in our modified array	
	CDurationSet s;
	s.m_nProviderID = GetSelectedProviderID();
	s.m_nAptTypeID = GetSelectedAptTypeID();
	s.m_adwPurposeIDs.Add(VarLong(m_dlPurposes->GetValue(nRow, PC_ID)));
	s.m_nDefaultLength = VarLong(m_dlPurposes->GetValue(nRow, PC_DEFAULT_MINUTES),0);
	s.m_nMinimumLength = VarLong(m_dlPurposes->GetValue(nRow, PC_MINIMUM_MINUTES),0);
	if (s.m_nDefaultLength == 0 && s.m_nMinimumLength == 0)
	{
		COleVariant vNull;
		s.m_bEnforced = FALSE;
		// (c.haag 2004-06-28 16:36) - If both items are zero, we don't want
		// to enforce the duration
		vNull.vt = VT_NULL;
		m_dlPurposes->PutValue(nRow, PC_DEFAULT_MINUTES, vNull);
		m_dlPurposes->PutValue(nRow, PC_MINIMUM_MINUTES, vNull);
	}
	else
	{		
		s.m_bEnforced = TRUE;
		// (c.haag 2004-06-28 16:32) - Make sure both fields have a number
		m_dlPurposes->PutValue(nRow, PC_DEFAULT_MINUTES, s.m_nDefaultLength);
		m_dlPurposes->PutValue(nRow, PC_MINIMUM_MINUTES, s.m_nMinimumLength);
	}

	//TES 3/25/2010 - PLID 37893 - Track the previous lengths.
	long nOldDefaultLength = -1, nOldMinimumLength = -1;
	// (c.haag 2004-06-28 15:01) - Update the array of preserved values
	// so that we can save all of our changes in one shot later.
	// (b.spivey - February 7th, 2014) - PLID 60379 - Don't find by set ID, we can't get it so we have to hope this will never fail. 
	long nIndex = FindDurationSet(s);
	if (-1 == nIndex)
	{
		// (c.haag 2004-06-22 09:16) - If we didn't find it, add it to the
		// array.
		nIndex = m_aDurations.GetSize();
		m_aDurations.Add(new CDurationSet(s));
		//TES 3/25/2010 - PLID 37893 - This didn't exist, so the old lengths are 0.
		nOldDefaultLength = 0;
		nOldMinimumLength = 0;
	}
	else
	{
		// (c.haag 2004-06-22 09:16) - Otherwise, assign this to the array
		//TES 3/25/2010 - PLID 37893 - Update the SetID
		s.m_nSetID = m_aDurations[nIndex]->m_nSetID;
		//TES 3/25/2010 - PLID 37893 - Pull the old lengths before we update them.
		nOldDefaultLength = m_aDurations[nIndex]->m_nDefaultLength;
		nOldMinimumLength = m_aDurations[nIndex]->m_nMinimumLength;
		m_aDurations[nIndex]->Copy(s);
	}
	//TES 3/25/2010 - PLID 37893 - Have we already changed this set?
	long nChangeIndex = FindChangedDuration(s, true);
	if(nChangeIndex == -1) {
		//TES 3/25/2010 - PLID 37893 - Nope, create a new change, based on this CDurationSet object.
		ChangedDuration cd(s);
		cd.nOldDefaultLength = nOldDefaultLength;
		cd.nOldMinimumLength = nOldMinimumLength;
		cd.nNewDefaultLength = s.m_nDefaultLength;
		cd.nNewMinimumLength = s.m_nMinimumLength;
		m_aChangedDurations.Add(cd);
	}
	else {
		//TES 3/25/2010 - PLID 37893 - Yup, just change the new lengths, the old lengths will be whatever they were originally.
		ChangedDuration cd = m_aChangedDurations[nChangeIndex];
		cd.nNewDefaultLength = s.m_nDefaultLength;
		cd.nNewMinimumLength = s.m_nMinimumLength;
		m_aChangedDurations.SetAt(nChangeIndex, cd);
	}
	m_bModified = TRUE;
}

void CSchedulerDurationDlg::OnEditingFinishedMultipurposeDurationCombinationList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (!bCommit) return;
	// (c.haag 2004-06-29 13:16) - Save the durations in the multi-purpose duration list
	CDurationSet* s = m_aDurations[VarLong(m_dlPurposeCombinations->GetValue(nRow, PC_ID))];
	//TES 3/25/2010 - PLID 37893 - Remember the previous lengths.
	long nOldDefaultLength = s->m_nDefaultLength;
	long nOldMinimumLength = s->m_nMinimumLength;
	s->m_nDefaultLength = VarLong(m_dlPurposeCombinations->GetValue(nRow, PC_DEFAULT_MINUTES),0);
	s->m_nMinimumLength = VarLong(m_dlPurposeCombinations->GetValue(nRow, PC_MINIMUM_MINUTES),0);
	//TES 3/25/2010 - PLID 37893 - Have we already changed this duration?
	long nChangeIndex = FindChangedDuration(*s, true);
	if(nChangeIndex == -1) {
		//TES 3/25/2010 - PLID 37893 - Nope, create a new change, based on this CDurationSet.
		ChangedDuration cd(*s);
		cd.nOldDefaultLength = nOldDefaultLength;
		cd.nOldMinimumLength = nOldMinimumLength;
		cd.nNewDefaultLength = s->m_nDefaultLength;
		cd.nNewMinimumLength = s->m_nMinimumLength;
		m_aChangedDurations.Add(cd);
	}
	else {
		//TES 3/25/2010 - PLID 37893 - Yep, just update the new lengths, the old ones will be whatever they were originally.
		ChangedDuration cd = m_aChangedDurations[nChangeIndex];
		cd.nNewDefaultLength = s->m_nDefaultLength;
		cd.nNewMinimumLength = s->m_nMinimumLength;
		m_aChangedDurations.SetAt(nChangeIndex, cd);
	}
	m_bModified = TRUE;
}

void CSchedulerDurationDlg::OnLeftClickMultipurposeDurationCombinationList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2004-06-29 12:26) - Handle left clicks over hyperlinks
	if (nCol != PC_NAME) return;
	
	//TES 3/29/2010 - PLID 37893 - Don't allow them to add a combination that already exists, our change tracking depends on all combinations
	// being unique.  Loop until they enter a unique combination, or cancel.
	BOOL bContinue = TRUE;
	while(bContinue) {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptPurposeT");
		// (c.haag 2004-06-29 12:53) - Preselect existing purposes
		CString str = VarString(m_dlPurposeCombinations->GetValue(nRow, PC_MULTIPURPOSEIDS), "");
		long nComma = str.Find(",");
		while(nComma > 0) {
			dlg.PreSelect(atoi(str.Left(nComma)));
			str = str.Right(str.GetLength() - (nComma + 1));
			nComma = str.Find(",");
		}
		if (str.GetLength()) {
			dlg.PreSelect(atoi(str));
		}

		// (c.haag 2004-06-29 12:53) - Have the user select purposes
		// (c.haag 2008-12-17 17:05) - PLID 32376 - Filter out inactive procedures unless they're already in the combination array
		CString strWhere;
		if (!VarString(m_dlPurposeCombinations->GetValue(nRow, PC_MULTIPURPOSEIDS),"").IsEmpty()) { 
			strWhere.Format(" OR ID IN (%s)", VarString(m_dlPurposeCombinations->GetValue(nRow, PC_MULTIPURPOSEIDS), ""));
		}
		if(IDOK == dlg.Open("AptPurposeT", FormatString("ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) %s", strWhere), "ID", "Name", "Select purposes"))
		{
			CDurationSet* s = m_aDurations[VarLong(m_dlPurposeCombinations->GetValue(nRow, PC_ID))];
			CDurationSet sNew(*s);
			CString strPurposes;

			// (c.haag 2004-06-29 12:54) - Store our values in our array. The ID column
			// of this list stores the index of the array.
			//TES 3/25/2010 - PLID 37893 - Remember the previous purpose list.
			CDWordArray adwPurposeIDsOld, adwPurposeIDs;
			adwPurposeIDsOld.Append(s->m_adwPurposeIDs);
			sNew.m_adwPurposeIDs.RemoveAll();
			str = dlg.GetMultiSelectIDString();
			str.Replace(" ", ",");
			nComma = str.Find(",");
			while(nComma > 0) {
				long nPurposeID = atoi(str.Left(nComma));
				sNew.m_adwPurposeIDs.Add(nPurposeID);
				strPurposes += m_mapPurpName[nPurposeID] + ", ";
				str = str.Right(str.GetLength() - (nComma + 1));
				nComma = str.Find(",");
			}
			if (str.GetLength()) {
				sNew.m_adwPurposeIDs.Add(atoi(str));
				strPurposes += m_mapPurpName[atoi(str)] + ", ";
			}
			if (strPurposes.GetLength()) {
				strPurposes = strPurposes.Left( strPurposes.GetLength() - 2 );
			}
			else {
				strPurposes = "< No Purposes >";
			}

			if (!ArraysMatch(adwPurposeIDsOld, sNew.m_adwPurposeIDs)) {
				//TES 3/29/2010 - PLID 37893 - Does this combination already exist?
				long nIndex = FindDurationSet(sNew);
				if(nIndex == -1) {
					//TES 3/29/2010 - PLID 37893 - It didn't exist, so we can go ahead and create it.
					bContinue = false;
					s->Copy(sNew);
					//TES 3/25/2010 - PLID 37893 - We're treating this as deleting the old combination, and creating a new one.
					CDurationSet sOld = *s;
					//TES 3/25/2010 - PLID 37893 - Pull the original purpose list.
					sOld.m_adwPurposeIDs.RemoveAll();
					sOld.m_adwPurposeIDs.Append(adwPurposeIDsOld);
					ChangedDuration cdOld(sOld);
					//TES 3/25/2010 - PLID 37893 - Since this is "deleted", set the new lengths to 0.
					cdOld.nNewDefaultLength = 0;
					cdOld.nNewMinimumLength = 0;
					//TES 3/25/2010 - PLID 37893 - Did we previously change this set?
					long nIndex = FindChangedDuration(sOld, true);
					if(nIndex == -1) {
						//TES 3/25/2010 - PLID 37893 - Nope, create a new duration.
						cdOld.nOldDefaultLength = s->m_nDefaultLength;
						cdOld.nOldMinimumLength = s->m_nMinimumLength;
						m_aChangedDurations.Add(cdOld);
					}
					else {
						//TES 3/25/2010 - PLID 37893 - Yup, update it in our array.
						m_aChangedDurations.SetAt(nIndex, cdOld);
					}

					//TES 3/25/2010 - PLID 37893 - Now, the set we "created"
					ChangedDuration cdNew(*s);
					cdNew.nNewDefaultLength = s->m_nDefaultLength;
					cdNew.nNewMinimumLength = s->m_nMinimumLength;
					//TES 3/25/2010 - PLID 37893 - Have we changed this duration previously?
					nIndex = FindChangedDuration(*s, true);
					if(nIndex == -1) {
						//TES 3/25/2010 - PLID 37893 - Nope, create a new change (it originally didn't exist, so the old lengths are 0.
						cdNew.nOldDefaultLength = 0;
						cdNew.nOldMinimumLength = 0;
						m_aChangedDurations.Add(cdNew);
					}
					else {
						//TES 3/25/2010 - PLID 37893 - Yep, update it in our array.
						m_aChangedDurations.SetAt(nIndex, cdNew);
					}
					
					// (c.haag 2004-06-29 12:54) - Now update the visible list
					str = dlg.GetMultiSelectIDString();
					str.Replace(" ", ",");
					m_dlPurposeCombinations->PutValue(nRow, PC_MULTIPURPOSEIDS, _bstr_t(str));
					m_dlPurposeCombinations->PutValue(nRow, PC_NAME, _bstr_t(strPurposes));
					m_bModified = TRUE;
				}
				else {
					//TES 3/29/2010 - PLID 37893 - This would be a duplicate, let them know, and keep looping.
					MsgBox("This provider, type, and purpose combination already has a default duration set.  Please select a different combination of purposes.");
				}
			}
			else {
				//TES 3/29/2010 - PLID 37893 - The arrays matched, so we're not actually changing anything, so there's no need to keep
				// looping.
				bContinue = FALSE;
			}
		}
		else {
			//TES 3/29/2010 - PLID 37893 - They cancelled.
			bContinue = FALSE;
		}
	}
}

void CSchedulerDurationDlg::OnRadioDoindividualpurposes() 
{
	EnsureList();
	EnsureButtons();
	RequeryPurposes();
	GetDlgItem(IDC_STATIC_MULTIPROCNOTICE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_MULTIPURPOSE_DURATION_LIST)->SetFocus();
}

void CSchedulerDurationDlg::OnRadioDopurposecombinations() 
{
	EnsureList();
	EnsureButtons();
	RequeryPurposes();
	GetDlgItem(IDC_STATIC_MULTIPROCNOTICE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST)->SetFocus();
}

void CSchedulerDurationDlg::OnBtnAddproccombination() 
{
	try {
		//TES 3/29/2010 - PLID 37893 - We can't allow them to add a combination that already exists, our change tracking depends on all combinations
		// being unique.  So prompt right away, and loop until they enter a unique combination, or cancel.
		BOOL bContinue = TRUE;
		while(bContinue) {
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "AptPurposeT");
			if(IDOK == dlg.Open("AptPurposeT", FormatString("ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)"), "ID", "Name", "Select purposes"))
			{
				CDurationSet s;
				s.m_nProviderID = GetSelectedProviderID();
				s.m_nAptTypeID = GetSelectedAptTypeID();
				s.m_adwPurposeIDs.RemoveAll();
				CString str = dlg.GetMultiSelectIDString();
				str.Replace(" ", ",");
				int nComma = str.Find(",");
				while(nComma > 0) {
					long nPurposeID = atoi(str.Left(nComma));
					s.m_adwPurposeIDs.Add(nPurposeID);
					str = str.Right(str.GetLength() - (nComma + 1));
					nComma = str.Find(",");
				}
				if (str.GetLength()) {
					s.m_adwPurposeIDs.Add(atoi(str));
				}
				s.m_nDefaultLength = 15;
				s.m_nMinimumLength = 15;
				//TES 3/29/2010 - PLID 37893 - Does this duration already exist?
				int nIndex = FindDurationSet(s);
				if(nIndex == -1) {
					//TES 3/29/2010 - PLID 37893 - It's unique, so we can go ahead and create it.
					bContinue = FALSE;
					m_aDurations.Add(new CDurationSet(s));
					ReflectDurations();

					// (c.haag 2009-09-14 15:14) - PLID 15380 - Jump to and ensure that the first item
					// in the combination list that resembles the duration set is visible and highlighted
					CString strPurposeIDs;
					for (long j=0; j < s.m_adwPurposeIDs.GetSize(); j++)
					{
						CString str;
						str.Format("%d,", s.m_adwPurposeIDs[j]);
						strPurposeIDs += str;
					}
					if (strPurposeIDs.GetLength()) {
						strPurposeIDs = strPurposeIDs.Left( strPurposeIDs.GetLength() - 1 );
					}
					for (long i=0; i < m_dlPurposeCombinations->GetRowCount(); i++) {
						IRowSettingsPtr pRow = m_dlPurposeCombinations->GetRow(i);
						if (VarLong(pRow->Value[PC_DEFAULT_MINUTES]) == s.m_nDefaultLength &&
							VarLong(pRow->Value[PC_MINIMUM_MINUTES]) == s.m_nMinimumLength &&
							VarString(pRow->Value[PC_MULTIPURPOSEIDS],"") == strPurposeIDs)
						{
							m_dlPurposeCombinations->EnsureRowVisible(i);
							m_dlPurposeCombinations->CurSel = i;
							break;
						}
					}
					// Give the list focus so that the highlight comes out in a dark blue instead of a pale gray
					GetDlgItem(IDC_MULTIPURPOSE_DURATION_COMBINATION_LIST)->SetFocus();

					//TES 3/25/2010 - PLID 37893 - Have we previously changed this duration?
					long nChangeIndex = FindChangedDuration(s, true);
					if(nChangeIndex == -1) {
						//TES 3/25/2010 - PLID 37893 - Nope, create a new change (the old lengths will be 0 since this didn't exist before).
						ChangedDuration cd(s);
						cd.nOldDefaultLength = 0;
						cd.nOldMinimumLength = 0;
						cd.nNewDefaultLength = s.m_nDefaultLength;
						cd.nNewMinimumLength = s.m_nMinimumLength;
						m_aChangedDurations.Add(cd);
					}
					else {
						//TES 3/25/2010 - PLID 37893 - Yep, just update the lengths.
						ChangedDuration cd = m_aChangedDurations[nChangeIndex];
						cd.nNewDefaultLength = s.m_nDefaultLength;
						cd.nNewMinimumLength = s.m_nMinimumLength;
						m_aChangedDurations.SetAt(nChangeIndex, cd);
					}

					m_bModified = TRUE;
				}
				else {
					//TES 3/29/2010 - PLID 37893 - This was a duplicate, let the user know, and keep looping.
					MsgBox("This provider, type, and purpose combination already has a default duration set.  Please select a different combination of purposes.");
				}
			}			
			else {
				//TES 3/29/2010 - PLID 37893 - They cancelled.
				bContinue = FALSE;
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CSchedulerDurationDlg::OnBtnDeleteproccombination() 
{
	long nCurSel = m_dlPurposeCombinations->CurSel;
	if (nCurSel < 0) return;
	long nID = VarLong(m_dlPurposeCombinations->GetValue(nCurSel, PC_ID));
	CDurationSet* s = m_aDurations[nID];
	//TES 3/25/2010 - PLID 37893 - Have we previously changed this duration?
	long nChangeIndex = FindChangedDuration(*s, true);
	if(nChangeIndex == -1) {
		//TES 3/25/2010 - PLID 37893 - Nope, create a new change (the new lengths are 0, since this is being deleted).
		ChangedDuration cd(*s);
		cd.nOldDefaultLength = s->m_nDefaultLength;
		cd.nOldMinimumLength = s->m_nMinimumLength;
		cd.nNewDefaultLength = 0;
		cd.nNewMinimumLength = 0;
		m_aChangedDurations.Add(cd);
	}
	else {
		//TES 3/25/2010 - PLID 37893 - Yup, update the new lengths to 0.
		ChangedDuration cd = m_aChangedDurations[nChangeIndex];
		cd.nNewDefaultLength = 0;
		cd.nNewMinimumLength = 0;
		m_aChangedDurations.SetAt(nChangeIndex, cd);
	}
			
	m_aDurations.RemoveAt(nID);
	delete s;
	ReflectDurations();	
	m_bModified = TRUE;
}

// (z.manning, 12/09/05, PLID 18511)
void CSchedulerDurationDlg::OnAddDefaultDurations()
{
	int nCheck;
	if(IsDlgButtonChecked(IDC_ADD_DEFAULT_DURATIONS)) {
		nCheck = 1;
	}
	else {
		nCheck = 0;
	}
	SetRemotePropertyInt("AddApptDefaultDurations", nCheck, 0, "<None>");
}


void CSchedulerDurationDlg::OnCopyDurations()
{
	// (c.haag 2008-08-29 17:17) - PLID 23981 - Let the user copy the default
	// durations to other providers
	try {
		const long nSrcProviderID = GetSelectedProviderID();

		// First, tell the user what this will actually do.
		const CString strMsg = "This feature will let you copy all default durations for all appointment types from the currently selected provider to other providers.";
		DontShowMeAgain(this, ConvertToControlText(strMsg), "CopyDefaultDurations", "Practice", FALSE, FALSE);

		// Gather a list of provider ID's we can copy to
		CArray<long,long> anCandidateIDs;
		int nProviders = m_dlProviders->GetRowCount();
		int i;
		for (i=0; i < nProviders; i++) {
			long nID = VarLong(m_dlProviders->GetValue(i, 0));
			if (nID != nSrcProviderID) {
				anCandidateIDs.Add(nID);
			}
		}

		// Do a check to see if there are any providers besides the selected one. If there are not, then quit
		if (0 == anCandidateIDs.GetSize()) {
			MsgBox(MB_ICONERROR | MB_OK, "There are no other providers to copy default durations to.");
			return;
		}

		// Now display a list of providers to choose from
		CString strIDs;
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
		if (IDOK == dlg.Open("ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID",
			FormatString("PersonID IN (%s)", ArrayAsString(anCandidateIDs)),
			"PersonT.ID",
			"Last + ', ' + First + ' ' + Middle",
			"Please select providers to copy default durations to",
			1))
		{
			strIDs = dlg.GetMultiSelectIDString();
		}
		if (strIDs.IsEmpty()) {
			// If we get here, the user cancelled
			return;
		}

		// Before we copy anything over, check for existing default durations for the selected providers,
		// and warn the user if there is anything
		BOOL bWarn = FALSE;
		CArray<long,long> anProviderIDs;
		ParseDelimitedStringToLongArray(strIDs, " ", anProviderIDs);
		for (i=0; i < m_aDurations.GetSize() && !bWarn; i++) {
			CDurationSet* p = m_aDurations[i];
			for (int j=0; j < anProviderIDs.GetSize() && !bWarn; j++) {
				if (p->m_nProviderID == anProviderIDs[j] && p->m_bEnforced) {
					bWarn = TRUE;
				}
			}
		}
		if (bWarn) {
			//TES 5/12/2010 - PLID 38506 - Strengthened and clarified this warning message, changed the default button to No.
			if (IDNO == MsgBox(MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2, "At least one selected provider already has default durations assigned.\r\n\r\n"
				"WARNING: ALL default durations for any selected providers, including durations which are not currently displayed, will be "
				"PERMANENTLY overwritten by the current provider's default durations.  This includes type/purpose combinations for which "
				"the current provider does not have a default duration assigned.  In that case, the default durations for those combinations "
				"will be deleted.  This cannot be undone!\r\n"
				"\r\nAre you SURE you wish to continue?")) {
				return;
			}
		}

		// Clear all existing durations for the destination providers
		for (i=0; i < m_aDurations.GetSize(); i++) {
			CDurationSet* p = m_aDurations[i];
			BOOL bFound = FALSE;
			for (int j=0; j < anProviderIDs.GetSize() && !bFound; j++) {
				if (p->m_nProviderID == anProviderIDs[j]) {
					//TES 3/29/2010 - PLID 37893 - Remember that we deleted this duration.
					long nChangeIndex = FindChangedDuration(*p, true);
					if(nChangeIndex == -1) {
						ChangedDuration cd(*p);
						cd.nOldDefaultLength = p->m_nDefaultLength;
						cd.nOldMinimumLength = p->m_nMinimumLength;
						cd.nNewDefaultLength = 0;
						cd.nNewMinimumLength = 0;
						m_aChangedDurations.Add(cd);
					}
					else {
						ChangedDuration cd = m_aChangedDurations[nChangeIndex];
						cd.nNewDefaultLength = 0;
						cd.nNewMinimumLength = 0;
						m_aChangedDurations.SetAt(nChangeIndex, cd);
					}
					m_aDurations.RemoveAt(i--);
					delete p;
					bFound = TRUE;
				}
			}
		}

		// Finally, do the copy. For all duration sets, find the ones that belong to the source provider
		for (i=0; i < m_aDurations.GetSize(); i++) {
			CDurationSet* pSrc = m_aDurations[i];
			if (pSrc->m_nProviderID == nSrcProviderID) {
				// Now copy the set to all the target providers
				for (int j=0; j < anProviderIDs.GetSize(); j++) {
					CopyDurationSet(pSrc, anProviderIDs[j]);
				}
			}
		}

		// Success
		MsgBox(MB_ICONINFORMATION | MB_OK, "The default durations were copied successfully.");

		//TES 9/1/2009 - PLID 34034 - Remember that we've made a change that needs saving.
		m_bModified = TRUE;

		//TES 4/7/2010 - PLID 38098 - We need to call ReflectDurations(), because even though the displayed information hasn't changed,
		// the array indexes may have, and those are stored in the datalist if we're in combination mode.
		ReflectDurations();
	}
	NxCatchAll("Error in CSchedulerDlg::OnCopyDurations");
}

// (c.haag 2008-09-02 10:38) - PLID 23981 - Copies a duration set to a provider
void CSchedulerDurationDlg::CopyDurationSet(CDurationSet* pSrc, long nDestProviderID)
{
	CDurationSet sSearch;
	CDurationSet* pDest = NULL;
	
	// Find an matching duration set for the destination provider
	sSearch.Copy(*pSrc);
	sSearch.m_nProviderID = nDestProviderID;
	int nIndex = FindDurationSet(sSearch);
	//TES 3/25/2010 - PLID 37893 - Track the previous lengths.
	long nOldDefaultLength = -1, nOldMinimumLength = -1;
	//TES 3/25/2010 - PLID 37893 - Also track the SetID
	long nSetID = -1;
	if (nIndex > -1) {
		pDest = m_aDurations[nIndex];
		nOldDefaultLength = pDest->m_nDefaultLength;
		nOldMinimumLength = pDest->m_nMinimumLength;
		nSetID = pDest->m_nSetID;
	} else {
		m_aDurations.Add((pDest = new CDurationSet));
		//TES 3/25/2010 - PLID 37893 - This didn't previously exist, so the old lengths are 0.
		nOldDefaultLength = 0;
		nOldMinimumLength = 0;
	}
	pDest->Copy(*pSrc);
	//TES 3/25/2010 - PLID 37893 - Restore the correct SetID
	pDest->m_nSetID = nSetID;
	pDest->m_nProviderID = nDestProviderID;

	//TES 3/25/2010 - PLID 37893 - Have we previously changed this duration?
	long nChangeIndex = FindChangedDuration(*pDest, true);
	if(nChangeIndex == -1) {
		//TES 3/25/2010 - PLID 37893 - Nope, create a new change.
		ChangedDuration cd(*pDest);
		cd.nSetID = -1;
		cd.nOldDefaultLength = nOldDefaultLength;
		cd.nOldMinimumLength = nOldMinimumLength;
		cd.nNewDefaultLength = pDest->m_nDefaultLength;
		cd.nNewMinimumLength = pDest->m_nMinimumLength;
		m_aChangedDurations.Add(cd);
	}
	else {
		//TES 3/25/2010 - PLID 37893 - Yep, just update the new lengths.
		ChangedDuration cd = m_aChangedDurations[nChangeIndex];
		cd.nNewDefaultLength = pDest->m_nDefaultLength;
		cd.nNewMinimumLength = pDest->m_nMinimumLength;
		m_aChangedDurations.SetAt(nChangeIndex, cd);
	}
}

//TES 3/25/2010 - PLID 37893 - Find a ChangedDuration that matches the given CDurationSet.  Note that this does NOT look at SetID, but
// matches on the provider, type, and purposes.
// (b.spivey - February 4th, 2014) - PLID 60379 - Updated to look for set ID if asked to. 
long CSchedulerDurationDlg::FindChangedDuration(CDurationSet s, bool bFindAbsoluteDurationSet /*= false*/)
{
	//TES 3/25/2010 - PLID 37893 - Find a ChangedDuration object that has the same ProviderID, TypeID, and list of Purposes.
	for(int i = 0; i < m_aChangedDurations.GetSize(); i++) {
		ChangedDuration cd = m_aChangedDurations[i];
		if(cd.nProviderID == s.m_nProviderID && cd.nAptTypeID == s.m_nAptTypeID && cd.adwPurposeIDs.GetSize() == s.m_adwPurposeIDs.GetSize()
			&& ArraysMatch(cd.adwPurposeIDs, s.m_adwPurposeIDs)
			&& (!bFindAbsoluteDurationSet || cd.nSetID == s.m_nSetID)) {
				return i;
		}
	}
	return -1;
}

// (b.spivey - February 4th, 2014) - PLID 60379 - This function will update a specific set ID.
bool CSchedulerDurationDlg::UpdateSingleDurationSet(CDurationSet* pds)
{
	//if null or set ID is invalid, we don't need to try. 
	if(pds == NULL || pds->m_nSetID < 0) {
		return false;
	}

	//If it has been changed, we don't need to update it. 
	for (int i = 0; i < m_aChangedDurations.GetSize(); i++) {

		ChangedDuration cd = m_aChangedDurations.GetAt(i); 

		if(pds->m_nSetID == cd.nSetID) {
			return true;
		}
	}

	//if we got this far, it either exists and needs updating, or it doesn't exist and needs adding.
	for (int i = 0; i < m_aDurations.GetSize(); i++) {
		CDurationSet* s = m_aDurations.GetAt(i);
		if(s != NULL && pds->m_nSetID == s->m_nSetID) {
			s->Copy(*pds); 
			return true; 
		}
	}

	return false; 
}