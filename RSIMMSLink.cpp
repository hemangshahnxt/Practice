//
// (c.haag 2009-07-08 11:32) - PLID 34379 - Initial implementation
//
// This namespace provides Practice with basic functionality to show images from MSI
// Medical Media System in NexTech Practice
//
#include "stdafx.h"
#include "RSIMMSLink.h"
#include "PracProps.h"
#include "ShowConnectingFeedbackDlg.h"

using namespace ADODB;

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdolong GetCommandTimeout();

namespace RSIMMSLink
{
	//////////////////////////////////////////////////////////////////
	// Private member variables
	_ConnectionPtr g_pConRSIMMS;
	CString g_strLastConnString;

	//////////////////////////////////////////////////////////////////
	// Private functions
	CString GetConnString()
	{
		CString strConnString = GetRemotePropertyText("RSIMMSConnString");
		return strConnString;
	}

	CString GetSharedPath()
	{
		CString strInstallPath = GetRemotePropertyText("RSIMMSSharedPath");
		return strInstallPath;
	}

	CString GetImagePath()
	{
		return GetSharedPath() ^ "img";
	}

	CString GetBaseQuery()
	{
		return "select top 100 percent photo.* from patnt "
			"inner join pat_prcdur on pat_prcdur.patnt_ik = patnt.patnt_ik "
			"inner join proc_fld on proc_fld.pat_prcdur_ik = pat_prcdur.pat_prcdur_ik "
			"inner join photo on photo.proc_fld_ik = proc_fld.proc_fld_ik "
			"where patnt.patnt_ik = {INT} and patnt.bDelRecycle = 1 and photo.bDelRecycle = 1 "
			"order by photo_ik "
			;
	}

	_ConnectionPtr GetRSIMMSData()
	{
		try {
			// See if the connection string changed. If so, reset the connection.
			if (g_pConRSIMMS != NULL && g_strLastConnString != GetConnString()) {
				EnsureNotLink();
			}

			// If we don't have a valid connection pointer, create one
			if (g_pConRSIMMS == NULL) {
				g_pConRSIMMS.CreateInstance(__uuidof(ADODB::Connection));
			}

			// We can't check the State property unless we have a valid connection pointer
			if (g_pConRSIMMS != NULL) {

				// If there were errors on this connection object, clear them
				ClearErrorsCollectionRemoteDataConnection(g_pConRSIMMS, TRUE);

				// See if we need to re-affirm an open connection
				// (a.walling 2011-09-07 18:01) - PLID 45380 - This was checking whether we need to affirm the connection to the global connection
				if (/*NeedAffirmConnection(acAffirmNow) && */ (g_pConRSIMMS->State != adStateClosed)) {
					try {
						// Affirm the connection
						g_pConRSIMMS->Execute("SELECT 0", NULL, adCmdText);
					} catch (_com_error e) {
						// Failed to affirm the connection so close it (we'll re-open it below)
						g_pConRSIMMS->Close();
					}
				}

				// If the connection is closed (for whatever reason: maybe we haven't even opened it yet, or maybe we closed it above because there were errors)
				if (g_pConRSIMMS->State == adStateClosed) {
					// Practice isn't open so open it

					// Give the user feedback in the form of a dialog that we are trying to connect to sql
					CShowConnectingFeedbackDlg* pdlgConnecting = new CShowConnectingFeedbackDlg;
					pdlgConnecting->SetWaitMessage("Plese wait while Practice connects to your ResultSet database...");

					// Build the correct connection string
					g_strLastConnString = GetConnString();

					// Set the connection timeout
					g_pConRSIMMS->ConnectionTimeout = GetConnectionTimeout();

					// Make the connection
					try {
						HR(g_pConRSIMMS->Open(_bstr_t(g_strLastConnString), "","",NULL));
					} catch (_com_error e) {
						switch (e.Error()) {
						case E_FAIL:
							// (c.haag 2006-02-20 12:24) - PLID 17472 - If HandleException returns 1, it means we
							// reconnected to Practice
							if (pdlgConnecting) { delete pdlgConnecting; pdlgConnecting = NULL; }
							if (0 == HandleException(e, "Error connecting to database.", __LINE__, __FILE__, "", "")) {
								throw;
							} else {
								return g_pConRSIMMS;
							}
							break;
						default:
							if (pdlgConnecting) { delete pdlgConnecting; pdlgConnecting = NULL; }
							throw;
							break;
						}
					}

					// Set the command operation timeout
					g_pConRSIMMS->CommandTimeout = GetCommandTimeout();
					// Set the default cursor location to client-side
					g_pConRSIMMS->CursorLocation = adUseClient;

					if (pdlgConnecting) { delete pdlgConnecting; pdlgConnecting = NULL; }

				} // if (g_pConRSIMMS->State == adStateClosed) {
			} // if (g_pConRSIMMS != NULL) {
		}
		catch (_com_error e) {
			// You may breakpoint this spot for debugging purposes
			throw e;
		}
		catch (...) {
			// You may breakpoint this spot for debugging purposes
			throw;
		}
		return g_pConRSIMMS;
	}

	// (j.jones 2010-04-23 09:43) - PLID 11596 - moved to GlobalUtils
	//CString GetThisComputerName()

	//////////////////////////////////////////////////////////////////
	// Public functions
	BOOL IsLinkEnabled()
	{
		try {
			
			// (j.jones 2010-04-23 09:45) - PLID 11596 - changed the user/computer name calculation to be a global function
			//(e.lally 2011-08-26) PLID 44950 - auto-create this pref so we can bulk cache it.
			BOOL bEnabled = (BOOL)GetRemotePropertyInt("ShowRSIMMSThumbs", 0, 0, GetCurrentUserComputerName(), true);
			return bEnabled;		
		}
		NxCatchAll("Error in RSIMMSLink::IsLinkEnabled");
		return FALSE;
	}

	void EnsureNotLink()
	{
		try {
			if (NULL != g_pConRSIMMS) {
				if (g_pConRSIMMS->State != adStateClosed) {
					g_pConRSIMMS->Close();
				}
				g_pConRSIMMS.Release();
			}
		}
		NxCatchAll("Error in RSIMMSLink::EnsureNotLink")
	}

	long GetInternalPatientID(const CString& strPatFirst, const CString& strPatLast)
	{	
		try {
			if (!IsLinkEnabled()) {
				return -1;
			}
			_RecordsetPtr prs = CreateParamRecordset(GetRSIMMSData(),
				"SELECT patnt_ik FROM patnt WHERE frst_nm = {STRING} AND lst_nm = {STRING} and bDelRecycle = 1"
				,strPatFirst, strPatLast);
			if (prs->eof) {
				// If we get here, no matching names were found
				return -1;
			} else {
				// Get the ID
				long nPatientID = AdoFldLong(prs, "patnt_ik");
				// Now see if there's another patient with the same name
				prs->MoveNext();
				if (!prs->eof) {
					// Yes, there is. Fail because we don't support handling duplicate names.
					return -1;
				} else {
					// No, there is one and only one matching patient.
					return nPatientID;
				}
			}
		}
		NxCatchAll("Error in RSIMMSLink::GetInternalPatientID");
		return -1;
	}

	long GetImageCount(long nInternalPatientID)
	{	
		try {
			if (!IsLinkEnabled()) {
				return 0;
			}
			_RecordsetPtr prs = CreateParamRecordset(GetRSIMMSData(), 
				FormatString("SELECT COUNT(*) AS Cnt FROM (%s) SubQ", GetBaseQuery()),
				nInternalPatientID);
			long nCount = AdoFldLong(prs, "Cnt");
			return nCount;
		}
		NxCatchAll("Error in RSIMMSLink::GetImageCount");
		return 0;
	}

	HBITMAP LoadImage(long nInternalPatientID, long nIndex, OUT CString* pstrFullPathName)
	{
		try {
			if (!IsLinkEnabled()) {
				return NULL;
			}
			else if (nIndex < 0) {
				ASSERT(FALSE); // Invalid index; too low
				return NULL;
			}
			_RecordsetPtr prs = CreateParamRecordset(GetRSIMMSData(), 
				FormatString(GetBaseQuery()),
				nInternalPatientID);
			FieldsPtr f = prs->Fields;
			long nCurImageIndex = 0;
			while (!prs->eof && nCurImageIndex < nIndex) {
				prs->MoveNext();
				nCurImageIndex++;
			}
			if (nCurImageIndex != nIndex) {
				ASSERT(FALSE); // Invalid index; too high
				return NULL;
			}

			CString strID = AsString(AdoFldLong(f, "photo_ik"));
			CString strExt = AdoFldString(f, "file_ext", "");
			strExt.MakeLower(); // There's a bug in the viewer where it assumes extensions are lower case
			CString strFullPathName = GetImagePath() ^ strID + "."  + strExt;
			if (pstrFullPathName) {
				*pstrFullPathName = strFullPathName;
			}
			return LoadBitmapFromFile(strFullPathName);
		}
		NxCatchAll("Error in RSIMMSLink::LoadImage");
		return NULL;
	}
};