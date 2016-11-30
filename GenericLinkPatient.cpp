// GenericLinkPatient.cpp: implementation of the CGenericLinkPatient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "GenericLinkPatient.h"
#include "ExportDuplicates.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGenericLinkPatient::CGenericLinkPatient(CWnd* pParent, _ConnectionPtr& pConPractice, _ConnectionPtr& pConRemote, CString strRemoteDataPath, CString strLinkName, CString strRemotePassword)
	: m_pParent(pParent)
{
	m_pConPractice = pConPractice;
	m_pConRemote = pConRemote;
	m_strRemoteDataPath = strRemoteDataPath;
	m_strLinkName = strLinkName;
	m_strRemotePassword = strRemotePassword;
}

CGenericLinkPatient::~CGenericLinkPatient()
{

}

void CGenericLinkPatient::SetLocalSQL(const char* str)
{
	_RecordsetPtr prs(__uuidof(Recordset));
	FieldsPtr fields;

	if (m_pConPractice == NULL) return;

	try {		
		prs->Open(str, _variant_t((IDispatch *)m_pConPractice, true),
			adOpenStatic, adLockOptimistic, adCmdText);
	}
	catch (_com_error)
	{
		// TODO: Make some coherent error appear
		return;
	}

	fields = prs->Fields;
	m_astrLocalFields.RemoveAll();

	for (int i=0; i < fields->GetCount(); i++)
	{
		CString str;
		_variant_t vIndex = (long)i;
		m_astrLocalFields.Add((LPCTSTR)fields->Item[vIndex]->Name);
	}

	if (prs->Status == adStateOpen)
		prs->Close();
	prs.Release();
}

void CGenericLinkPatient::SetRemoteSQL(const char* str, ... /* Field lengths */)
{
	_RecordsetPtr prs(__uuidof(Recordset));
	FieldsPtr fields;
	va_list argList;
	
	if (m_pConRemote == NULL) return;

	try {		
		prs->Open(str, _variant_t((IDispatch *)m_pConRemote, true),
			adOpenStatic, adLockOptimistic, adCmdText);
	}
	catch (_com_error)
	{
		// TODO: Make some coherent error appear
		return;
	}

	fields = prs->Fields;
	m_astrRemoteFields.RemoveAll();
	m_adwRemoteFieldSizes.RemoveAll();

	va_start(argList, str);

	for (int i=0; i < fields->GetCount(); i++)
	{
		CString str;
		_variant_t vIndex = (long)i;
		m_astrRemoteFields.Add((LPCTSTR)fields->Item[vIndex]->Name);
		m_adwRemoteFieldSizes.Add( va_arg(argList, DWORD) );
	}

	va_end(argList);

	prs->Close();
	prs.Release();
}

DWORD CGenericLinkPatient::Export(DWORD dwPracticeID, DWORD& dwRemoteID, BOOL& bStop, EGenericLinkPersonAction& action, BOOL bAssumeOneMatchingNameLinks)
{
	_RecordsetPtr prs(__uuidof(Recordset));
	CString strSQL = "SELECT ";
	CString str;
	CString strFirst, strMiddle, strLast, strSSN;
	FieldsPtr fields;
	DWORD dwRes = 0;
	_variant_t v;

	//////////////////////////////////////////////////////////////
	// Build the local SQL string based on the field array
	if (m_astrLocalFields.GetSize() == 0)
	{
		// TODO: throw a huge huge error!!
		return 1;
	}
	if (m_pConRemote == NULL)
	{
		AfxMessageBox("The remote database could not be opened. The export cannot continue.");
		return 1;
	}
	if (m_pConPractice == NULL)
	{
		AfxMessageBox("The local database could not be opened. The export cannot continue.");
		return 1;
	}

	for (int i=0; i < m_astrLocalFields.GetSize(); i++)
	{
		strSQL += m_astrLocalFields[i] + ", ";
	}
	str.Format("%s%s FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PatientsT.PersonID = %d",
		strSQL, m_strLocalLinkIDField, dwPracticeID);
	strSQL = str;

	//////////////////////////////////////////////////////////////
	// Open the local recordset
	try {
		prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConPractice, true),
			adOpenStatic, adLockOptimistic, adCmdText);
		fields = prs->Fields;

		// See if we already have a remote ID for this record. A NULL or EMPTY
		// means we never linked.
		v = fields->Item[(LPCTSTR)m_strLocalLinkIDField]->Value;
		if (v.vt != VT_NULL && v.vt != VT_EMPTY)
		{
			// Hey! The record already exists! Lets do a HotSync!
			// TODO: Message box saying we're updating this patient
			// in the remote database
			dwRes = UpdateRemote(dwPracticeID, prs);
			dwRemoteID = v.lVal;
			action = EUpdatedRecord;
		}
		else {
			// The record does not exist in the remote database
			// given the ID. Lets add it.

			CExportDuplicates dlg(m_pParent);
			strFirst = AdoFldString(fields, "First");
			strMiddle = AdoFldString(fields, "Middle");
			strLast = AdoFldString(fields, "Last");
			strSSN = AdoFldString(fields, "SocialSecurity");

			if(dlg.FindDuplicates(strFirst,strLast,strMiddle,"Practice", m_strLinkName, m_pConRemote, m_strRemoteDataPath, TRUE, m_strRemotePassword, bAssumeOneMatchingNameLinks, strSSN))
			{
				switch (dlg.DoModal())
				{
				case 1: // Add new
					dwRes = AddToRemoteDatabase(dwPracticeID, prs, dwRemoteID);
					action = EAddedRecord;
					break;
				case 2: // Update the practice database to link this record.
					if (0 != (dwRes = LinkExistingToRemote(dwPracticeID, dlg.m_varIDToUpdate.lVal)))
					{
						action = ENone;
						break;
					}
					// Since LinkExistingToRemote altered the record corresponding to prs,
					// we need to requery prs.
					prs->Requery(adOptionUnspecified);

					dwRes = UpdateRemote(dwPracticeID, prs);
					dwRemoteID = dlg.m_varIDToUpdate.lVal;
					action = EUpdatedRecord;
					break;
				case 3: // Skip
					action = ENone;
					break;
				case 0: // Stop
					bStop = TRUE;
					break;
				}
			}
			else {
				dwRes = AddToRemoteDatabase(dwPracticeID, prs, dwRemoteID);
				action = EAddedRecord;
			}
		}

		if (prs->State == adStateOpen)
			prs->Close();
		prs.Detach();
		return dwRes;
	}
	NxCatchAll("CGenericLinkPatient::Export");
	return 1;
}

DWORD CGenericLinkPatient::Import(DWORD dwRemoteID, DWORD& dwPracticeID, DWORD& dwUserDefinedID, BOOL& bStop, EGenericLinkPersonAction& action)
{
	return 1;
}