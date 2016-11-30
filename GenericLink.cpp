// GenericLink.cpp: implementation of the CGenericLink class.
//
//////////////////////////////////////////////////////////////////////

// CAH 1/3/2002: It is imparative that the generic link and its child
// use the same number of columns, and that each column serves the
// same purpose.

#include "stdafx.h"
#include "globaldatautils.h"
#include "GenericLink.h"
#include "NxErrorDialog.h"
#include "PracticeRc.h"
#include "ShowConnectingFeedbackDlg.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (c.haag 2011-04-20) - PLID 42984 - We now invoke these functions from this source file
// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo
//void ClearErrorsCollectionRemoteDataConnection(_ConnectionPtr& pCon, BOOL bIncludeCallStackNote);
long GetCommandTimeout();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGenericLink::CGenericLink(CWnd *pParent)
	: CNxDialog(0, pParent)
{
	m_strRemotePassword = "";
	m_bLinkEnabled = TRUE;
}

CGenericLink::CGenericLink(int IDD, CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD, pParent)
{
	m_bGotRemotePath = FALSE;
	m_bLinkEnabled = TRUE;
}

CGenericLink::~CGenericLink()
{

}

CString CGenericLink::BrowseRemotePath()
{
	CString strInitPath,
			strInOutPath;
	BOOL bRetry = TRUE;

	CString strBrowseFilter;
	strBrowseFilter.Format("%s Files|*.mde;*.mdb|All Files|*.*|", m_strLinkName);
	CFileDialog dlgBrowse(TRUE, "mdb", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, strBrowseFilter);

	strInitPath = m_strRemotePath; // We need to store this because the next line is a pointer to it
	if (strInitPath.IsEmpty())
		strInitPath = "c:\\";

	dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
	while (bRetry)
	{
		if (dlgBrowse.DoModal() == IDOK) 
		{	// If the user clicked okay, that means she selected a file so remember it
			if (TestConnection(dlgBrowse.GetPathName()))
			{
				return dlgBrowse.GetPathName();
			}
			else
			{
				MsgBox(dlgBrowse.GetPathName() + " is not a valid " + m_strLinkName + " database, or could not be accessed from your computer.");
			}
		} 
		else
			bRetry = FALSE;
	}
	return "";
}

void CGenericLink::EnsureNotRemoteData()
{
	// (c.haag 2006-04-07 10:56) - PLID 20037 - Close our connection to the database
	//
	if (m_pConRemote != NULL) {
		if (m_pConRemote->State == adStateOpen) {
			m_pConRemote->Close();
		}
		m_pConRemote.Release();
	}
}

void CGenericLink::EnsureRemoteData()
{
	// (c.haag 2011-04-20) - PLID 42984 - This function has been rewritten to affirm the data connection
	// to United. Every time a query is called, we call a simple failsafe "pre-query" to ensure we have a valid
	// connection to the database. If it fails, then it must be because we could not connect to the database
	// or because the connection was broken. In that case, we reconnect to the data, and then leave the
	// function.
	//
	// All this fanciness is necessary because we use a connection string to connect to the United data. When
	// using this instead of a DSN-based connection, there is anecdotal evidence that some computers have 
	// problems with random disconnects after a while. I don't know exactly why the random disconnects happen,
	// but this new way of doing things should improve robustness to a point where we don't suffer from the small
	// time disconnects.

	// If we don't have a valid connection pointer, create one
	if (m_pConRemote == NULL) {
		m_pConRemote.CreateInstance(__uuidof(Connection));
	}

	// We can't check the State property unless we have a valid connection pointer
	if (m_pConRemote != NULL) {
		
		// If there were errors on this connection object, clear them
		ClearErrorsCollectionRemoteDataConnection(m_pConRemote, TRUE);

		// See if we need to re-affirm an open connection
		if (m_pConRemote->State != adStateClosed) {
			try {
				m_pConRemote->Execute("SELECT TOP 1 uImageID FROM [Image]", NULL, adCmdText);
			} catch (_com_error e) {
				Log("CGenericLink::EnsureRemoteData failed to verify the connection. Error %x: %s", e.Error(), (LPCTSTR)e.Description());
				// Failed to affirm the connection so close it (we'll re-open it below)
				m_pConRemote->Close();
			}
		}

		// If the connection is closed (for whatever reason: maybe we haven't even opened it yet, or maybe we closed it above because there were errors)
		if (m_pConRemote->State == adStateClosed) {
			// The connection isn't open so open it

			// Give the user feedback in the form of a dialog that we are trying to connect to sql
			CShowConnectingFeedbackDlg* pdlgConnecting = new CShowConnectingFeedbackDlg("NexTech Practice Connecting...", "Please wait while Practice connects to your " + m_strLinkName + " database...");

			// Build the correct connection string
			CString strRemotePath = GetRemotePath();
			CString strConnString = "Provider=Microsoft.Jet.OLEDB.4.0;" +
				(GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + m_strRemotePassword + ";") : "") +
				"Data Source=" + strRemotePath + ";";
			Log("Reconnecting to " + m_strLinkName + " Database. Path = %s", strRemotePath);

			// Set the connection timeout. Nothing wrong with making it consistent with the Practice SQL connection timeout.
			m_pConRemote->ConnectionTimeout = GetConnectionTimeout();

			// Make the connection
			try {
				HR(m_pConRemote->Open(_bstr_t(strConnString), "","",NULL));
				// Set the command operation timeout
				m_pConRemote->CommandTimeout = GetCommandTimeout();
				// Set the default cursor location to client-side
				m_pConRemote->CursorLocation = adUseClient;
				// Give our connection string to the datalist
				if (m_dlRemote != NULL) {
					m_dlRemote->ConnectionString = _bstr_t(strConnString);
				}
				// There is no longer a reason for the link to be temporarily disabled
				EnableLink(TRUE);
			}
			catch (_com_error e) 
			{
				// The old code would silently set the connection to NULL. We really should report a problem if we get here.
				HandleException(e, "Error connecting to the " + m_strLinkName + " database:", __LINE__, __FILE__, "", "");
				m_pConRemote = NULL;
			}

			if (pdlgConnecting) { delete pdlgConnecting; pdlgConnecting = NULL; }
		
		} // if (m_pConRemote->State == adStateClosed) {

	} // if (m_pConRemote != NULL) {
	else
	{
		// I don't know how we could get here because the client wouldn't have been able to open Practice either.
	}


	// (c.haag 2011-04-20) - PLID 42984 - This is the old way of connecting. Keeping for reference purposes.
	/*
	CString strRemotePath = GetRemotePath();
	CString strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
				(GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + m_strRemotePassword + ";") : "") +
				"Data Source=" + strRemotePath + ";";
	CShowConnectingFeedbackDlg dlgConnecting;
	dlgConnecting.SetWaitMessage("Please wait while Practice connects to your United database...");
	CWaitCursor wc;

	try {
		//////////////////////////////////////////////////////////////////////////
		// (c.haag 2006-10-11 10:10) - PLID 19985 - This function has been
		// rewritten to fix strange connection issues and for efficiency

		//
		// Create the connection object if it does not exist
		//
		if (NULL == m_pConRemote) {
			m_pConRemote.CreateInstance(__uuidof(Connection));
		}

		//
		// Re-affirm the connection if it's open
		//
		if (m_pConRemote->State != adStateClosed) {
			try {
				m_pConRemote->Execute("SELECT 0", NULL, adCmdText);
			} catch (_com_error e) {
				// Failure! Close the connection so that it will be reopened.
				m_pConRemote->Close();
			}
		}

		// If the connection is closed (for whatever reason: maybe we
		// haven't even opened it yet, or maybe we closed it above because
		// there were errors), then open it and test it
		if (m_pConRemote->State == adStateClosed) {
			HR(m_pConRemote->Open(_bstr_t((LPCTSTR)strCon), "","",NULL));
			m_pConRemote->CursorLocation = adUseClient;

			// So far so good; we've opened the connection. Now run two
			// test queries to ensure the data structure is correct.
			_RecordsetPtr prs(__uuidof(Recordset));

			// (c.haag 2003-10-17 09:04) - Query 1: I select AtHomeYet to distinguish it from
			// an Inform MDB, which has an eerily similar database structure.
			prs->Open("SELECT tblPatient.AtHomeYet, tblPatient.First, tblPatient.Last, tblPatient.uImageDirectory + '\\' + tblPatient.uStampName + '.bmp' AS ImagePath FROM tblPatient",
				_variant_t((IDispatch *)m_pConRemote, true), adOpenForwardOnly, adLockReadOnly, adCmdText);
			prs->Close();

			// (c.haag 2006-10-11 10:07) - Query 2: Make sure we have an image table
			prs->Open("SELECT COUNT(uImageID) AS ImageCount FROM [Image]",
				_variant_t((IDispatch *)m_pConRemote, true), adOpenForwardOnly, adLockReadOnly, adCmdText);
			prs->Close();
		}

		// (c.haag 2006-10-11 10:32) - PLID 19985 - Legacy code starts here
		//////////////////////////////////////////////////////////////////////////

		// Give our connection string to the datalist
		if (m_dlRemote != NULL)
			m_dlRemote->ConnectionString = _bstr_t(strCon);

		// There is no longer a reason for the link to be temporarily disabled
		EnableLink(TRUE);
	}
	catch (_com_error &)
	{
		if (m_pConRemote != NULL) {
			m_pConRemote.Release();
		}
	}*/
}

LPDISPATCH CGenericLink::GetRemoteData()
{
	EnsureRemoteData();
	return m_pConRemote;
}

/*
	Flow of control: Called whenever the window is opened
	or the path is changed.


	1. Gets the path to the remote DB. If fails, forces
	user to browse to it.

	2. Tries to open data based on path information. If
	data is already opened, it's closed first.

*/

// (s.dhole 2009-11-24 14:06) - PLID 35759 Can we make the United link work like the mirror link, where if the connection to the DB is not available or has problems we prompt to disable  the link for the current session rather than prompting to reset the link or throwning nasty exceptions.

_RecordsetPtr CGenericLink::CreateRecordSet(CString strQuery)
{
	_RecordsetPtr prs(__uuidof(Recordset));
	Log("in CGenericLink::CreateRecordSet: %s", strQuery);
	// (c.haag 2011-04-20) - PLID 42984 - This used to directly access m_pConRemote and try to re-run the very same
	// query if the first call failed. We now use GetRemoteData because it's cleaner and more correctly affirms the connection.
	prs->Open(_bstr_t(strQuery), _variant_t(GetRemoteData(), true), adOpenForwardOnly, adLockReadOnly, adCmdText);
	return prs;
}

unsigned long CGenericLink::RefreshData()
{
	CString strRemotePath;
	DWORD dwFileAttribs;
	CString strExceptionError = "Error refreshing data for " + m_strLinkName;

	try {
getpath:
		// Get the path to the remote data
		strRemotePath = GetRemotePath();

		// If the path is blank, don't even try to load
		m_btnRemotePath.SetWindowText(m_strRemotePath);
		if (strRemotePath.IsEmpty())
		{
			// (c.haag 2006-02-09 16:23) - PLID 18874 - If no path is set, force the
			// user to browse to the United data

			//CString str;
			//str.Format("Failed to load remote data from path '%s'", strRemotePath);
			//m_statusBar.SetText(str, 255, 0);
			//return TRUE;

			if (IDYES == MsgBox(MB_YESNO, "The %s link has not been properly set up. Do you want to browse for your %s data now?",
				m_strLinkName, m_strLinkName))
			{
				SetRemotePath(BrowseRemotePath());
				goto getpath;
			}
			return FALSE;
		}

		//
		// (c.haag 2006-04-10 17:09) - PLID 20037 - Close our connection to the database
		//
		EnsureNotRemoteData();

		//
		// Reconnect to the database
		//
		if (NULL == GetRemoteData())
		{
			if (IDYES == MsgBox(MB_YESNO, "The database at %s could not be accessed. Would you like to try again with a different Windows login?",
				strRemotePath))
			{
				if (NO_ERROR == TryAccessNetworkFile(strRemotePath))
					goto getpath;
			}

			if (IDYES == MsgBox(MB_YESNO, "The database at %s is inaccessible, or not a valid %s database. Do you want to browse for your data?",
				strRemotePath, m_strLinkName))
			{
				SetRemotePath(BrowseRemotePath());
				goto getpath;
			}
			else
			{
				CString str;
				str.Format("Failed to load remote data from path '%s'", strRemotePath);
				m_statusBar.SetText(str, 255, 0);
				return TRUE;
			}
		}

		// Check the file attributes to see if it's read only
		dwFileAttribs = GetFileAttributes(m_strRemotePath);
		if (dwFileAttribs != 0xFFFFFFFF && dwFileAttribs & FILE_ATTRIBUTE_READONLY)
		{
			MsgBox("The remote database was found, but is read-only. You will not be able to export patients from Practice.");
		}

		// Now, requery the data lists
		m_dlPractice->Requery();
		m_dlRemote->Requery();

		m_statusBar.SetText("Loading...", 255, 0);
		return TRUE;
	}
	NxCatchAll(strExceptionError);
	return FALSE;
}

BOOL CGenericLink::OnInitDialog() 
{
	CRect rcClient;

	CNxDialog::OnInitDialog();

	GetClientRect(rcClient);
	m_statusBar.Create(WS_VISIBLE|WS_CHILD|CBRS_BOTTOM, rcClient, (this), AFX_IDW_STATUS_BAR);
	m_statusBar.SetSimple();	

	m_pracAdd.AutoSet(NXB_RIGHT);
	m_pracRem.AutoSet(NXB_LEFT);
	m_pracRemAll.AutoSet(NXB_LLEFT);
	m_remoteAdd.AutoSet(NXB_RIGHT);
	m_remoteRem.AutoSet(NXB_LEFT);
	m_remoteRemAll.AutoSet(NXB_LLEFT);
	m_btnImport.AutoSet(NXB_UP);
	m_btnExport.AutoSet(NXB_DOWN);

	m_dlPractice = BindNxDataListCtrl(IDC_NEXTECH, false);
	m_dlRemote = BindNxDataListCtrl(IDC_REMOTE, false);
	m_dlPracticeSelected = BindNxDataListCtrl(IDC_EXPORT,false);
	m_dlRemoteSelected = BindNxDataListCtrl(IDC_IMPORT,false);

	////////////////////////////////////////////////////////
	// Disable the export button if the data is read only
	/*if (GetFileAttributes(m_strRemotePath) & FILE_ATTRIBUTE_READONLY)
	{
		GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(FALSE);	
	}
	else {
		GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(TRUE);
	}*/

	return TRUE;
}

void CGenericLink::OnOK()
{
	CloseDlg();
}

void CGenericLink::OnCancel()
{
	CloseDlg();
}

void CGenericLink::OpenDlg()
{
	if (!IsWindow(GetSafeHwnd()) && !Create(IDD))
	{	HandleException(NULL, "Could not create Generic Link window");
		return;
	}

	// (c.haag 2006-02-09 16:28) - PLID 18874 - Don't show the link window if
	// we can't access the data
	if (FALSE == RefreshData())
		return;

	GetMainFrame()->EnableWindow(FALSE);
	EnableWindow(TRUE);
	ShowWindow(SW_SHOW);
}

void CGenericLink::CloseDlg()
{
	// Remove all items from the selected lists
	m_dlPractice->TakeAllRows(m_dlPracticeSelected);
	m_dlRemote->TakeAllRows(m_dlRemoteSelected);

	GetMainFrame()->EnableWindow(TRUE);
	ShowWindow(SW_HIDE);
}

void CGenericLink::OnBtnRemotePath() 
{
	CString strBrowsePath = BrowseRemotePath();
	if (!strBrowsePath.IsEmpty() && strBrowsePath != GetRemotePath())
	{
		SetRemotePath(strBrowsePath);
		RefreshData();
	}
}

void CGenericLink::OnPracAdd() 
{
	CString str;
	_variant_t var;

	if (m_dlPractice->GetCurSel()!=-1)
		m_dlPracticeSelected->TakeCurrentRow(m_dlPractice);

	str.Format ("%li", m_dlPracticeSelected->GetRowCount());
	SetDlgItemText (IDC_NEXTECH_EXPORT_COUNT, str);
	CRect rc;
	GetDlgItem(IDC_NEXTECH_EXPORT_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
}

void CGenericLink::OnPracRemove() 
{
	CString str;

	if (m_dlPracticeSelected->GetCurSel()!=-1)
		m_dlPractice->TakeCurrentRow(m_dlPracticeSelected);

	str.Format ("%li", m_dlPracticeSelected->GetRowCount());
	SetDlgItemText (IDC_NEXTECH_EXPORT_COUNT, str);
	CRect rc;
	GetDlgItem(IDC_NEXTECH_EXPORT_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
}

void CGenericLink::OnPracRemoveAll()
{
	m_dlPractice->TakeAllRows(m_dlPracticeSelected);
	SetDlgItemText (IDC_NEXTECH_EXPORT_COUNT, "0");
	CRect rc;
	GetDlgItem(IDC_NEXTECH_EXPORT_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
}

void CGenericLink::OnRemoteAdd() 
{
	CString		str;
	_variant_t var;

	if (m_dlRemote->GetCurSel()!=-1)
		m_dlRemoteSelected->TakeCurrentRow(m_dlRemote);
	str.Format ("%li", m_dlRemoteSelected->GetRowCount());
	SetDlgItemText (IDC_REMOTE_EXPORT_COUNT, str);
	CRect rc;
	GetDlgItem(IDC_REMOTE_EXPORT_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
}

void CGenericLink::OnRemoteRemove()
{
	CString str;

	if (m_dlRemoteSelected->GetCurSel()!=-1)
		m_dlRemote->TakeCurrentRow(m_dlRemoteSelected);

	str.Format ("%li", m_dlRemoteSelected->GetRowCount());
	SetDlgItemText (IDC_REMOTE_EXPORT_COUNT, str);
	CRect rc;
	GetDlgItem(IDC_REMOTE_EXPORT_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
}

void CGenericLink::OnRemoteRemoveAll()
{
	m_dlRemote->TakeAllRows(m_dlRemoteSelected);
	SetDlgItemText (IDC_REMOTE_EXPORT_COUNT, "0");
	CRect rc;
	GetDlgItem(IDC_REMOTE_EXPORT_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
}

void CGenericLink::OnRequeryFinishedNextech(short nFlags)
{
	if (m_dlRemote->IsRequerying()) return;
	m_statusBar.SetText("Refreshing Links...", 255, 0);
	RefreshColors();
}

void CGenericLink::OnRequeryFinishedRemote(short nFlags)
{
	if (m_dlPractice->IsRequerying()) return;
	m_statusBar.SetText("Refreshing Links...", 255, 0);
	RefreshColors();
}

void CGenericLink::RefreshColors()
{
	CString str;

	str.Format ("%li", m_dlPractice->GetRowCount());
	SetDlgItemText (IDC_NEXTECH_COUNT, str);
	CRect rc;
	GetDlgItem(IDC_NEXTECH_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	str.Format ("%li", m_dlRemote->GetRowCount());
	SetDlgItemText (IDC_REMOTE_COUNT, str);
	GetDlgItem(IDC_REMOTE_COUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	m_statusBar.SetText("Ready", 255, 0);
}


// CAH 1/3/2002: It is imperative that the generic link and its child
// use the same number of columns, and that each column serves the
// same purpose.

void CGenericLink::OnUnlink()
{
	if (IDNO == MsgBox(MB_YESNO, "Are you absolutely sure you wish to unlink all the patients selected in the NexTech Practice database list from their respective entries in %s", m_strLinkName))
		return;

	try
	{
		long p = m_dlPractice->GetFirstSelEnum();
		IRowSettingsPtr pRow = NULL;
	
		// Do for all highlighted items in the NexTech list
		while (p)
		{
			variant_t varNextech, varOldMirror;

			m_dlPractice->GetNextSelEnum(&p, (IDispatch**)&pRow);
			varNextech = pRow->Value[ 4 /* PersonID (User-Defined) */ ];
			
			// Unlink the item
			Unlink(VarLong(varNextech));

			//remove old color
			varOldMirror = pRow->Value[3];
			if (varOldMirror.vt != VT_NULL && varOldMirror.vt != VT_EMPTY)
			{					
				long nOldMirror = m_dlRemote->FindByColumn(2, varOldMirror, 0, FALSE);
				if (nOldMirror != -1)
				{	IRowSettingsPtr pMirrorRow;
					pMirrorRow = m_dlRemote->GetRow(nOldMirror);
					pMirrorRow->BackColor = dlColorNotSet;
				}
			}

			//set new color
			pRow->BackColor = dlColorNotSet;

			//remove remote id from nextech record
			_variant_t vNull;
			vNull.vt = VT_NULL;
			pRow->Value[3] = vNull;
		}
	}
	NxCatchAll("Could not unlink patient");
}
