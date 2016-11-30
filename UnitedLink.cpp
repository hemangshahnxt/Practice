// UnitedLink.cpp : implementation file
//

// CAH 1/3/2002: It is imparative that the generic link and its child
// (in this case, the united link)
// use the same number of columns, and that each column serves the
// same purpose.

#include "stdafx.h"
#include "practice.h"
#include "PracticeRc.h"
#include "UnitedLink.h"
#include "UnitedLinkPatient.h"
#include "GlobalDataUtils.h"
#include "dontshowdlg.h"
#include "client.h"
#include "exportduplicates.h"
#define _WIN32_MSI 110	//USE 1.1 installer API
#include "msi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

static DWORD ParseVersion(char *buffer)
{
	long dot;
	DWORD version = 0;
	char *p = buffer;

	//major
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version = atoi(p) << 24;
	p += dot + 1;

	//minor
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version += atoi(p) << 16;

	return version; 
}

CString GetUnitedExecutePath()
{
	// (c.haag 2006-06-30 11:27) - PLID 21263 - Get the United install path from data.
	// If it does not exist, we have to calculate it
	CString strInstallPath = GetPropertyText("UnitedExecutePath", "");
	if (strInstallPath.IsEmpty()) {
		// (c.haag 2006-07-07 13:47) - This is hard coded. United does not have a
		// process for assigning product codes, so I looked in the registry for one
		// that worked from the latest installation CD
		const char szProductCode[] = "{7E8768C9-C7A6-4B07-9AA0-5631B44C87FE}";
		DWORD nMaxVersion = 0;
		DWORD nSize = MAX_PATH;
		char sz[MAX_PATH];
		//
		// Default the install path to the default local install path
		//
		strInstallPath = "C:\\Program Files\\Uniplast\\uni32.exe";
		//
		// Get the installation path to the United product
		//
		if (ERROR_SUCCESS == MsiGetProductInfo(szProductCode, INSTALLPROPERTY_INSTALLLOCATION, sz, &nSize)) {
			strInstallPath = CString(sz) ^ "uni32.exe";
		}
	}
	SetPropertyText("UnitedExecutePath", strInstallPath);
	return strInstallPath;
}

/////////////////////////////////////////////////////////////////////////////
// CUnitedLink dialog

CUnitedLink::CUnitedLink(CWnd* pParent)
	: CGenericLink(CUnitedLink::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUnitedLink)
	m_strRemotePathProperty = "UnitedLink";
	m_strRemotePassword = "image_access";
	m_strLinkName = "United Imaging";
	//}}AFX_DATA_INIT
}


void CUnitedLink::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUnitedLink)
	DDX_Control(pDX, IDC_SHOW_ADVANCED, m_checkAdvanced);
	DDX_Control(pDX, IDC_BTN_REMOTE_PATH, m_btnRemotePath);
	DDX_Control(pDX, IDC_IMPORT_BTN, m_btnImport);
	DDX_Control(pDX, IDC_EXPORT_BTN, m_btnExport);
	DDX_Control(pDX, IDC_REMOTE_REMOVE_ALL, m_remoteRemAll);
	DDX_Control(pDX, IDC_REMOTE_REMOVE, m_remoteRem);
	DDX_Control(pDX, IDC_REMOTE_ADD, m_remoteAdd);
	DDX_Control(pDX, IDC_PRAC_REMOVE_ALL, m_pracRemAll);
	DDX_Control(pDX, IDC_PRAC_REMOVE, m_pracRem);
	DDX_Control(pDX, IDC_PRAC_ADD, m_pracAdd);
	DDX_Control(pDX, IDC_NEXTECH_COUNT, m_nxstaticNextechCount);
	DDX_Control(pDX, IDC_NEXTECH_EXPORT_COUNT, m_nxstaticNextechExportCount);
	DDX_Control(pDX, IDC_REMOTE_COUNT, m_nxstaticRemoteCount);
	DDX_Control(pDX, IDC_REMOTE_EXPORT_COUNT, m_nxstaticRemoteExportCount);
	DDX_Control(pDX, IDC_STATIC_UNI32_LOCATION, m_nxstaticUni32Location);
	DDX_Control(pDX, IDC_EXPORT_BTN, m_btnExport);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_CHECK_ENABLEUNITEDLINK, m_btnEnableLink);
	DDX_Control(pDX, IDC_NEW_PAT_CHECK, m_btnNewPatExport);
	DDX_Control(pDX, IDC_CHECK_LINK_USERDEFINEDID, m_btnLinkUserID);
	DDX_Control(pDX, IDC_CHECK_UNITEDSHOWIMAGES, m_btnShowImages);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CUnitedLink, CGenericLink)
	//{{AFX_MSG_MAP(CUnitedLink)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_REMOTE_PATH, OnBtnRemotePath)
	ON_BN_CLICKED(IDC_PRAC_ADD, OnPracAdd)
	ON_BN_CLICKED(IDC_PRAC_REMOVE, OnPracRemove)
	ON_BN_CLICKED(IDC_PRAC_REMOVE_ALL, OnPracRemoveAll)
	ON_BN_CLICKED(IDC_REMOTE_ADD, OnRemoteAdd)
	ON_BN_CLICKED(IDC_REMOTE_REMOVE, OnRemoteRemove)
	ON_BN_CLICKED(IDC_REMOTE_REMOVE_ALL, OnRemoteRemoveAll)
	ON_BN_CLICKED(IDC_EXPORT_BTN, OnExportToRemote)
	ON_BN_CLICKED(IDC_IMPORT_BTN, OnImportFromRemote)
	ON_BN_CLICKED(IDC_SHOW_ADVANCED, OnShowAdvancedOptions)
	ON_BN_CLICKED(IDC_COPY_FROM_UNITED, OnCopyFromUnitedList)
	ON_BN_CLICKED(IDC_LINK, OnLink)
	ON_BN_CLICKED(IDC_UNLINK, OnUnlink)
	ON_BN_CLICKED(IDC_NEW_PAT_CHECK, OnNewPatCheck)
	ON_BN_CLICKED(IDC_CHECK_LINK_USERDEFINEDID, OnCheckLinkUserdefinedid)
	ON_BN_CLICKED(IDC_CHECK_UNITEDSHOWIMAGES, OnCheckUnitedshowimages)
	ON_BN_CLICKED(IDC_CHECK_ENABLEUNITEDLINK, OnCheckEnableunitedlink)
	ON_BN_CLICKED(IDC_BTN_UNI32_LOCATION, OnBtnUni32Location)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitedLink message handlers

BOOL CUnitedLink::OnInitDialog() 
{
	CGenericLink::OnInitDialog();
	
	m_btnExport.AutoSet(NXB_EXPORT);
	m_btnClose.AutoSet(NXB_CLOSE);

	((CButton*)GetDlgItem(IDC_NEW_PAT_CHECK))->SetCheck(GetPropertyInt("NewPatExportToUnited"));
	((CButton*)GetDlgItem(IDC_CHECK_LINK_USERDEFINEDID))->SetCheck(GetPropertyInt("UnitedLinkIDToUnitedID", 1));
	((CButton*)GetDlgItem(IDC_CHECK_UNITEDSHOWIMAGES))->SetCheck(GetPropertyInt("UnitedShowImages", 1));
	((CButton*)GetDlgItem(IDC_CHECK_ENABLEUNITEDLINK))->SetCheck(GetPropertyInt("UnitedDisable", 0, 0, false));

	if (!GetPropertyInt("UnitedDisable", 0, 0, false))
		m_dlPractice->Requery();

	// (c.haag 2006-06-30 13:06) - PLID 19977 - Set the location of uni32.exe on the form
	SetDlgItemText(IDC_BTN_UNI32_LOCATION, GetUnitedExecutePath());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUnitedLink::OnOK()
{
	CGenericLink::OnOK();
}

void CUnitedLink::OnCancel()
{
	CGenericLink::OnCancel();
}

HBRUSH CUnitedLink::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CUnitedLink::OnBtnRemotePath() 
{
	CGenericLink::OnBtnRemotePath();	
}

void CUnitedLink::OnPracAdd() 
{
	CGenericLink::OnPracAdd();
}

void CUnitedLink::OnPracRemove() 
{
	CGenericLink::OnPracRemove();
}

void CUnitedLink::OnPracRemoveAll() 
{
	CGenericLink::OnPracRemoveAll();
}

void CUnitedLink::OnRemoteAdd() 
{
	CGenericLink::OnRemoteAdd();
}

void CUnitedLink::OnRemoteRemove() 
{
	CGenericLink::OnRemoteRemove();
}

void CUnitedLink::OnRemoteRemoveAll() 
{
	CGenericLink::OnRemoteRemoveAll();
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CUnitedLink, CGenericLink)
    //{{AFX_EVENTSINK_MAP(CUnitedLink)
	ON_EVENT(CUnitedLink, IDC_NEXTECH, 3 /* DblClickCell */, OnDblClickCellNextech, VTS_I4 VTS_I2)
	ON_EVENT(CUnitedLink, IDC_REMOTE, 3 /* DblClickCell */, OnDblClickCellRemote, VTS_I4 VTS_I2)
	ON_EVENT(CUnitedLink, IDC_EXPORT, 3 /* DblClickCell */, OnDblClickCellExport, VTS_I4 VTS_I2)
	ON_EVENT(CUnitedLink, IDC_IMPORT, 3 /* DblClickCell */, OnDblClickCellImport, VTS_I4 VTS_I2)
	ON_EVENT(CUnitedLink, IDC_NEXTECH, 18 /* RequeryFinished */, OnRequeryFinishedNextech, VTS_I2)
	ON_EVENT(CUnitedLink, IDC_REMOTE, 18 /* RequeryFinished */, OnRequeryFinishedRemote, VTS_I2)
	ON_EVENT(CUnitedLink, IDC_NEXTECH, 7 /* RButtonUp */, OnRButtonUpNexTech, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CUnitedLink::OnRButtonUpNexTech(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu menPopup;
	menPopup.m_hMenu = CreatePopupMenu();
	IRowSettingsPtr pRow = m_dlPractice->GetRow(nRow);
	OLE_COLOR clr = pRow->BackColor;
	CWnd* pWnd = GetDlgItem(IDC_NEXTECH);
	CPoint pt;

	pt.x = x;
	pt.y = y;
	pWnd->ClientToScreen(&pt);

	switch (clr)
	{
	case 0x00DAFCD1:
		menPopup.InsertMenu(0, MF_BYPOSITION, 32700 /* Arbitrary number */, "This patient is properly linked with United Imaging.");
		break;
	case 0x000FFFF:
		//
		// TODO: Make the act of clicking on this pop-up bring up the online help.
		//
		menPopup.InsertMenu(0, MF_BYPOSITION, 32700 /* Arbitrary number */, "This patient is incorrectly linked with United Imaging.");
		break;
	default:
		menPopup.InsertMenu(0, MF_BYPOSITION, 32700 /* Arbitrary number */, "This patient is not linked with United Imaging.");
		break;
	}

	menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
}

void CUnitedLink::OnDblClickCellNextech(long nRowIndex, short nColIndex) 
{
	OnPracAdd();
}

void CUnitedLink::OnDblClickCellRemote(long nRowIndex, short nColIndex) 
{
	OnRemoteAdd();
}

void CUnitedLink::OnDblClickCellExport(long nRowIndex, short nColIndex) 
{
	OnPracRemove();
}

void CUnitedLink::OnDblClickCellImport(long nRowIndex, short nColIndex) 
{
	OnRemoteRemove();	
}

void CUnitedLink::OnRequeryFinishedNextech(short nFlags) 
{
	CGenericLink::OnRequeryFinishedNextech(nFlags);
}

void CUnitedLink::OnRequeryFinishedRemote(short nFlags) 
{
	CGenericLink::OnRequeryFinishedRemote(nFlags);
}

// (c.haag 2011-04-20) - PLID 43351 - Cleaned up the enums for both lists
enum {
	epcFirstContactDate = 0,
	epcName,
	epcUserDefinedID,
	epcUnitedID,
	epcID, // PersonT.ID
} PracticeColumn;

enum {
	ercFirstContactDate = 0,
	ercName,
	ercUnitedID,
} RemoteColumn;

void CUnitedLink::OnExportToRemote() 
{
	try {
		_ConnectionPtr pConLocal = ::GetRemoteData();
		_ConnectionPtr pConRemote;

		if (NULL == (pConRemote = GetRemoteData())) {
			MsgBox("The United Imaging database is invalid, or is inaccessible from your computer.\r\n\r\nThe link to United will be disabled for this session of Practice. Please go to Modules=>Link to United to reconnect with the United Imaging database.");
			EnableLink(FALSE);
			return;
		}

		// (c.haag 2011-04-20) - PLID 43351 - Refactored most of the code below.
		// m_dlPracticeSelected corresponds to the list on the top right corner of the dialog;
		// the one that has the list of patients to export from Practice to United.
		CUnitedLinkPatient pat(this, pConLocal, pConRemote, GetRemotePath(), m_strRemotePassword);
		CString strMsg;
		BOOL bStop = FALSE;
		EGenericLinkPersonAction action;
		BOOL bAssumeOneMatchingNameLinks = FALSE;

		if (0 == m_dlPracticeSelected->GetRowCount() || !UserPermission(UnitedExport))
			return;

		strMsg.Format("Are you sure you wish to export the most recent demographics of %d patients from NexTech Practice to United Imaging?", m_dlPracticeSelected->GetRowCount());
		if (IDYES != MessageBox(strMsg, "Patient Export", MB_YESNO))
			return;

		if (m_dlPracticeSelected->GetRowCount() >= 10)
		{
			// Ask the user if he is sure he wants to do the export
			if (IDNO == MessageBox("You are exporting ten or more patients. It is STRONGLY recommended that you make a backup of your United Imaging database before doing this. Are you sure wish to continue with the export?", "United Imaging Integration", MB_YESNO))
				return;

			// Ask the user if he wants to be prompted when duplicate names appear in both databases
			// only once. It will update the patient automatically.
			if (IDYES == MessageBox("Patients that are not linked yet, but have one matching first name, last name, and social security number in United Imaging, will be exported. Normally, you would be prompted as to whether to export this kind of patient as a new patient, or to export the data into an existing patient in United Imaging. Because so many patients are being exported, you may assume that every one of these patients in NexTech Practice should be exported into existing United Imaging patients with the same respective name and SSN.\n\nDo you wish to make this assumption? If you select 'Yes', then you will not be prompted when a patient being exported has one matching name and SSN in United Imaging; it will be assumed the two must be linked together.", "United Imaging Integration", MB_YESNO))
				bAssumeOneMatchingNameLinks = TRUE;
		}

		// Do for all rows in the export-to-United list
		while (!bStop && m_dlPracticeSelected->GetRowCount() > 0)
		{
			IRowSettingsPtr pPracSelectedRow = m_dlPracticeSelected->GetRow(0L);
			const long nPracticeID = VarLong(pPracSelectedRow->GetValue(epcID));
			DWORD dwRemoteID;

			if (pat.Export(nPracticeID, dwRemoteID, bStop, action, bAssumeOneMatchingNameLinks))
			{
				// TODO: Error message?
				bStop = TRUE;
			}
			else if (!bStop)
			{
				// Move the row in the export list back into the Practice list
				m_dlPractice->TakeRow(pPracSelectedRow);

				switch (action)
				{
				case ENone:
					// Nothing happened; the patient was probably skipped.
					break;

				case EAddedRecord:
					// If the patient was added to Practice, add it to the United list
					{
						IRowSettingsPtr pUnitedRow = m_dlRemote->GetRow(-1);
						pUnitedRow->PutValue( ercFirstContactDate, pPracSelectedRow->GetValue(epcFirstContactDate) );
						pUnitedRow->PutValue( ercName, pPracSelectedRow->GetValue(epcName) );
						pUnitedRow->PutValue( ercUnitedID, (long)dwRemoteID );
						pUnitedRow->BackColor = 0x00FCDAD1;
						m_dlRemote->AddRow(pUnitedRow);
						pPracSelectedRow->BackColor = 0x00DAFCD1; // Color the Practice row, too
						// Ensure the United ID is set in the Practice row
						pPracSelectedRow->PutValue( epcUnitedID, (long)dwRemoteID );
					}
					break;

				case EUpdatedRecord:
					// If the patient was updated in United, just make sure it's properly colored
					{
						long nRowIndex = m_dlRemote->FindByColumn(ercUnitedID, (long)dwRemoteID, 0, VARIANT_FALSE);
						if (nRowIndex > -1)
						{
							IRowSettingsPtr pUnitedRow = m_dlRemote->GetRow(nRowIndex);
							pUnitedRow->BackColor = 0x00FCDAD1;
						}
						// Ensure the United ID is set in the Practice row
						pPracSelectedRow->PutValue( epcUnitedID, (long)dwRemoteID );
						// Color the Practice row, too
						pPracSelectedRow->BackColor = 0x00DAFCD1;
					}
					break;
				}
			}
		}

		CString str;

		str.Format ("%li", m_dlPracticeSelected->GetRowCount());
		SetDlgItemText (IDC_NEXTECH_EXPORT_COUNT, str);
		CRect rc;
		GetDlgItem(IDC_NEXTECH_EXPORT_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str.Format ("%li", m_dlRemoteSelected->GetRowCount());
		SetDlgItemText (IDC_REMOTE_EXPORT_COUNT, str);
		GetDlgItem(IDC_REMOTE_EXPORT_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);
	} NxCatchAll("Error in OnExportToRemote()");
}

CString CUnitedLink::GetRemotePath()
{
	// Fail if the remote property was not specified
	if (m_strRemotePathProperty.IsEmpty())
	{
		ASSERT(NULL);
		return m_strRemotePathProperty;
	}

	if (!m_bGotRemotePath)
	{
		m_strRemotePath = GetRemotePropertyText("UnitedDataPath");
		m_bGotRemotePath = TRUE;
/*		if (m_strRemotePath.IsEmpty())
		{
			if (IDYES == MessageBox("You have not set the file path to your United data from Practice. Would you like to do it now?",
				"Practice", MB_YESNO))
			{
				CString strBrowsePath = BrowseRemotePath();
				if (!strBrowsePath.IsEmpty()) {
					SetRemotePath(strBrowsePath);
				} else {
					// The user clicked yes, but then failed to give a valid path so we'll try again next time we're asked
					m_bGotRemotePath = FALSE;
				}
			}
		}*/
	}
	return m_strRemotePath;
}

void CUnitedLink::SetRemotePath(const CString &path)
{
	if (m_strRemotePathProperty.IsEmpty())
	{
		ASSERT(NULL);
		return;
	}

	SetRemotePropertyText("UnitedDataPath", path);
	m_strRemotePath = path;
	m_bGotRemotePath = TRUE;
	//CClient::RefreshTable(NetUtils::UnitedDataPath);
}

void CUnitedLink::OnImportFromRemote() 
{
	try {
		_ConnectionPtr pConLocal = ::GetRemoteData();
		_ConnectionPtr pConRemote;

		if (NULL == (pConRemote = GetRemoteData())) {
			MsgBox("The United Imaging database is invalid, or is inaccessible from your computer.\r\n\r\nThe link to United will be disabled for this session of Practice. Please go to Modules=>Link to United to reconnect with the United Imaging database.");
			EnableLink(FALSE);
			return;
		}

		// (c.haag 2011-04-20) - PLID 43351 - Refactored most of the code below.
		// m_dlRemoteSelected corresponds to the list on the lower right corner of the
		// dialog; the one that has the list of patients to import from United into Practice.
		CUnitedLinkPatient pat(this, pConLocal, pConRemote, GetRemotePath());
		BOOL bStop = FALSE;
		EGenericLinkPersonAction action;

		if (0 == m_dlRemoteSelected->GetRowCount() || !UserPermission(UnitedImport))
			return;

		// Do for all rows in the import-into-Practice list
		while (!bStop && m_dlRemoteSelected->GetRowCount() > 0)
		{
			IRowSettingsPtr pRemoteSelectedRow = m_dlRemoteSelected->GetRow(0L);
			const long nUnitedID = VarLong(pRemoteSelectedRow->GetValue(ercUnitedID));
			DWORD dwPracticeID, dwUserDefinedID;

			if (pat.Import(nUnitedID, dwPracticeID, dwUserDefinedID, bStop, action))
			{
				// TODO: Error message?
				bStop = TRUE;
			}
			else if (!bStop)
			{
				// Move the row in the import list back into the United list
				m_dlRemote->TakeRow(pRemoteSelectedRow);

				switch (action)
				{
				case ENone:
					// Nothing happened; the patient was probably skipped.
					break;

				case EAddedRecord:
					// If the patient was added to Practice, add it to the Practice list
					{
						IRowSettingsPtr pPracticeRow = m_dlPractice->GetRow(-1);
						pPracticeRow->PutValue( epcFirstContactDate, pRemoteSelectedRow->GetValue(ercFirstContactDate) );
						pPracticeRow->PutValue( epcName, pRemoteSelectedRow->GetValue(ercName) );
						pPracticeRow->PutValue( epcUserDefinedID, (long)dwUserDefinedID );
						pPracticeRow->PutValue( epcUnitedID, nUnitedID );
						pPracticeRow->PutValue( epcID, (long)dwPracticeID );
						pPracticeRow->BackColor = 0x00DAFCD1;
						m_dlPractice->AddRow(pPracticeRow);						
						pRemoteSelectedRow->BackColor = 0x00FCDAD1; // Color the United row, too
						//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here (United always imports as an Active Patient)
						CClient::RefreshPatCombo(dwPracticeID, false, CClient::pcatActive,  CClient::pcstPatient);
					}
					break;
	
				case EUpdatedRecord:
					// If the patient was updated in Practice, just make sure it's properly colored
					{
						long nRowIndex = m_dlPractice->FindByColumn(epcID, (long)dwPracticeID, 0, VARIANT_FALSE);
						if (nRowIndex > -1)
						{
							IRowSettingsPtr pPracticeRow = m_dlPractice->GetRow(nRowIndex);
							pPracticeRow->PutValue( epcUnitedID, nUnitedID );
							pPracticeRow->BackColor = 0x00DAFCD1;
						}
						// Color the United row, too
						pRemoteSelectedRow->BackColor = 0x00FCDAD1;						
					}
					break;
				}

			}
		}

		CString str;

		str.Format ("%li", m_dlPracticeSelected->GetRowCount());
		SetDlgItemText (IDC_NEXTECH_EXPORT_COUNT, str);
		CRect rc;
		GetDlgItem(IDC_NEXTECH_EXPORT_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str.Format ("%li", m_dlRemoteSelected->GetRowCount());
		SetDlgItemText (IDC_REMOTE_EXPORT_COUNT, str);
		GetDlgItem(IDC_REMOTE_EXPORT_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);
	} NxCatchAll("Error in OnImportFromRemote()");
}

void CUnitedLink::RefreshColors()
{
	// (c.haag 2011-04-20) - PLID 43351 - Use proper enumerations for column references
	// Color-code the rows in the data lists based on which patients are linked
	_RecordsetPtr rs = CreateRecordset("SELECT PatientsT.PersonID AS ID, PatientsT.UnitedID FROM PatientsT WHERE PersonID > 0 AND UnitedID > 0 ORDER BY UnitedID DESC");
	DWORD dwPracRow = 0, dwUnitedRow = 0;
	DWORD dwLastPracRow = 0, dwLastUnitedRow = 0;
	_variant_t var;

	while (!rs->eof) {
		var = rs->Fields->Item["UnitedID"]->Value;

		dwUnitedRow = m_dlRemote->FindByColumn(ercUnitedID,var,dwLastUnitedRow,FALSE);
	
//		if(dwUnitedRow==-1) {
//			rs->MoveNext();
//			continue;
//		}
		var = rs->Fields->Item["ID"]->Value;
		dwPracRow = m_dlPractice->FindByColumn(epcID,var,dwLastPracRow,FALSE);

		if((dwPracRow!=-1 && dwUnitedRow != -1) /*&& (CString(m_dlPractice->GetValue(dwPracRow,1).bstrVal)==CString(m_dlRemote->GetValue(dwUnitedRow,1).bstrVal))*/) {
			IRowSettingsPtr pRow;			
			pRow = m_dlPractice->GetRow(dwPracRow);
			pRow->PutBackColor(0x00DAFCD1);
			pRow = m_dlRemote->GetRow(dwUnitedRow);
			pRow->PutBackColor(0x00FCDAD1);
			dwLastPracRow = dwPracRow;
			dwLastUnitedRow = dwUnitedRow;
		}
		else if (dwUnitedRow == -1)
		{
			// This means Practice had a United ID, but United does not have it.
			IRowSettingsPtr pRow;			
			pRow = m_dlPractice->GetRow(dwPracRow);
			pRow->PutBackColor(0x000FFFF);
			dwLastPracRow = dwPracRow;
		}

		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

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

long CUnitedLink::GetImageCount(long nUnitedId)
{
	_ConnectionPtr pConRemote;

	if (!IsUnitedEnabled())
		return 0;

	if (NULL == (pConRemote = GetRemoteData())) {
		MsgBox("The United Imaging database is invalid, or is inaccessible from your computer.\r\n\r\nThe link to United will be disabled for this session of Practice. Please go to Modules=>Link to United to reconnect with the United Imaging database.");
		EnableLink(FALSE);
		return 0;
	}

	// Get the connection string
	CString strConn;
	strConn.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;Jet OLEDB:Database Password=%s", GetRemotePath(), GetRemotePassword());
	
	// Get the query string
	CString strSql;
	strSql.Format(
		"SELECT COUNT(uImageID) AS ImageCount FROM [Image] "
		"WHERE uPatientID = %li", nUnitedId);

	// Open the query
	try {
//		_RecordsetPtr prs(__uuidof(Recordset));
		// (c.haag 2003-10-17 09:45) - The united link object should really persist at the
		// global scope because the logic to test the database connection by verifying certain
		// fields exist should have already been done by now. I'll correct that
		// in a future release.
		//
		// (c.haag 2006-10-11 10:29) - My previous comment makes no sense. We should always affirm
		// the connection with United before running a query or batch or queries because we may have
		// lost our connection between successive accesses. We should do it through GetRemoteData()
		// and not through a direct connection object access.
		//
		//prs->Open(_bstr_t(strSql), _variant_t((LPDISPATCH)pConRemote, true), adOpenForwardOnly, adLockReadOnly, adCmdText);
		// (s.dhole 2009-11-24 14:06) - PLID 35759 Can we make the United link work like the mirror link, where if the connection to the DB is not available or has problems we prompt to disable  the link for the current session rather than prompting to reset the link or throwning nasty exceptions.
		_RecordsetPtr prs=CreateRecordSet(strSql);
		
		if ((prs==NULL)|| (prs->eof)) {
			return 0;
		} else {
			// CAH 12/20: Though this is the correct image count, we only allow the reference
			// thumbnail to appear in the count.
			long lRes = AdoFldLong(prs, "ImageCount");

			return ((lRes == 0) ? 0 : 1);
		}
	}
	NxCatchAll("Error getting United image count");
	return 0;
}

HBITMAP CUnitedLink::LoadImage(long nUnitedId, const long &nIndex)
{
	_ConnectionPtr pConRemote;

	if (!IsUnitedEnabled())
		return NULL;

	if (NULL == (pConRemote = GetRemoteData())) {
		MsgBox("The United Imaging database is invalid, or is inaccessible from your computer.\r\n\r\nThe link to United will be disabled for this session of Practice. Please go to Modules=>Link to United to reconnect with the United Imaging database.");
		EnableLink(FALSE);
		return NULL;
	}

	// Get the query string
	CString strSql;
	strSql.Format(

// CAH 12/10: The commented statement will load a list of all patient images. The
// uncommented statement will load just the reference image.
//
//		"SELECT tblPatient.uImageDirectory + '\\' + Image.ViewFile + '.bmp' AS ImagePath "
//		"FROM tblPatient LEFT JOIN [Image] ON tblPatient.ID = Image.uPatientID "
//		"WHERE tblPatient.ID = %li", nUnitedId);

		"SELECT tblPatient.AtHomeYet, tblPatient.uImageDirectory + '\\' + tblPatient.uStampName + '.bmp' AS ImagePath "
		"FROM tblPatient "
		"WHERE tblPatient.ID = %li", nUnitedId);
	
	// Open the query
	// (s.dhole 2009-11-24 14:06) - PLID 35759 Can we make the United link work like the mirror link, where if the connection to the DB is not available or has problems we prompt to disable  the link for the current session rather than prompting to reset the link or throwning nasty exceptions.
	_RecordsetPtr prs = CreateRecordSet(strSql);
	//prs->Open(_bstr_t(strSql), _variant_t((LPDISPATCH)pConRemote, true), adOpenForwardOnly, adLockReadOnly, adCmdText);

	// Load the image at the given 0-based index into the list of images for this patient
	for (long i=0; !prs->eof; i++) {
		if (i == nIndex) {
			HBITMAP hBmp;
			char szDrive[32] = {0};
			char szRoot[512] = {0};

			// Get the image path
			CString strImagePath = AdoFldString(prs, "ImagePath");

			// Load and return the image reference
			CString strFullPath = GetFilePath(GetRemotePath()) ^ strImagePath;

			hBmp = (HBITMAP)::LoadImage(NULL, strFullPath, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);

			if (hBmp) return hBmp;

			// We failed!! Lets look at their INI for guidance.
			if (!GetPrivateProfileString("parameters", "imagedrive", "", szDrive, 32, GetFilePath(GetRemotePath()) ^ "uni.ini"))
				return NULL;
			if (!GetPrivateProfileString("parameters", "imageroot", "", szRoot, 512, GetFilePath(GetRemotePath()) ^ "uni.ini"))
				return NULL;

			strFullPath = CString(szDrive) ^ CString(szRoot) ^ strImagePath;

			return (HBITMAP)::LoadImage(NULL, strFullPath, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADFROMFILE);
		}
		prs->MoveNext();
	}

	// Couldn't find the image
	return NULL;
}

void CUnitedLink::OnShowAdvancedOptions() 
{	
	BOOL bShow;

	if (m_checkAdvanced.GetCheck())
	{
		DontShowMeAgain(this, "Advanced United integration allows power users to directly edit links between Practice and United.\n"
			"Please do not use this feature unless you are familiar with it.", 
			"United Advanced");
		bShow = TRUE;
	}
	else
		bShow = FALSE;

	GetDlgItem(IDC_COPY_FROM_UNITED)->ShowWindow(bShow);
	GetDlgItem(IDC_CHECK_LINK_USERDEFINEDID)->ShowWindow(bShow);
	GetDlgItem(IDC_LINK)->ShowWindow(bShow);
	GetDlgItem(IDC_UNLINK)->ShowWindow(bShow);
	GetDlgItem(IDC_CHECK_ENABLEUNITEDLINK)->ShowWindow(bShow);
	GetDlgItem(IDC_STATIC_UNI32_LOCATION)->ShowWindow(bShow);
	GetDlgItem(IDC_BTN_UNI32_LOCATION)->ShowWindow(bShow);
}

void CUnitedLink::OnCopyFromUnitedList() 
{
	long p = m_dlRemote->GetFirstRowEnum();
	LPDISPATCH pDisp = NULL;

	if (!m_dlRemote->GetRowCount()) return;

	AfxMessageBox("This function will take all of the names that exist in both the NexTech Practice and United Imaging databases and add them to the 'Patients to Export to United' list. This is useful when you are linking patients between NexTech Practice and United Imaging for the first time. No data on either database will be modified.");

	CWaitCursor wc; // (c.haag 2011-04-20) - PLID 43351 - Need a wait cursor here
	while (p)
	{	
		COleVariant varName;
		long lRow;

		m_dlRemote->GetNextRowEnum(&p, &pDisp);

		IRowSettingsPtr pRow(pDisp);
		pDisp->Release();		
		varName = pRow->Value[ercName]; // (c.haag 2011-04-20) - PLID 43351 - Use proper enumerations for column references
		lRow = m_dlPractice->FindByColumn(epcName, varName, 0, FALSE); // (c.haag 2011-04-20) - PLID 43351 - Use proper enumerations
		// (c.haag 2011-04-20) - PLID 43351 - Check for a -1 lRow!
		if (-1 != lRow)
		{
			IRowSettingsPtr pPracticeRow(m_dlPractice->GetRow(lRow));
			m_dlPracticeSelected->TakeRow(pPracticeRow);
		}
	}	
	
	//DRT 7/16/02 - This function wasn't updating the count when it moved things to the right
	CString str;
	str.Format ("%li", m_dlPracticeSelected->GetRowCount());
	SetDlgItemText (IDC_NEXTECH_EXPORT_COUNT, str);
}

void CUnitedLink::OnLink() 
{
	int nSel;
	long p;
	LPDISPATCH pRow;
	IRowSettingsPtr pRowSettings;
	_variant_t	varRemote, varNextech;

	try
	{
		// (c.haag 2006-02-09 16:28) - PLID 18874 - Close the window if we're not connected
		// to United data
		if (NULL == GetRemoteData()) {
			MsgBox("You have been disconnected from your United Imaging database. This window will now close.");
			OnCancel();
			return;
		}

		//////////////////////////////////////////////////////////////////////////
		// If there are any items in the export list, ask if we want a mass link.
		if (m_dlPracticeSelected->GetRowCount() > 0)
		{
			if (IDYES == MsgBox(MB_YESNO, "You have at least one patient name in the export list. Would you like the program to link all of those patients by matching first and last names?"))
			{
				OnMassLink();
				return;
			}
		}

		////////////////////////////////////////////////////////////////
		// Make sure only one item is selected in the united list
		p = m_dlRemote->GetFirstSelEnum();
		nSel = 0;
		while (p)
		{
			m_dlRemote->GetNextSelEnum(&p, &pRow);
			nSel++;
		}
		if (nSel != 1)
		{
			MsgBox("Please select one patient from the United patient list to link");
			return;
		}
		// (c.haag 2011-04-20) - PLID 43351 - Use proper enumerations
		varRemote = m_dlRemote->Value[m_dlRemote->CurSel][ ercUnitedID ];
	}
	NxCatchAllCall("United Link Error 100: Failed to get selected patients from the United patient list", {return;});
	
	try
	{
		////////////////////////////////////////////////////////////////
		// Make sure only one item is selected in the nextech list
		p = m_dlPractice->GetFirstSelEnum();
		nSel = 0;
		while (p)
		{
			m_dlPractice->GetNextSelEnum(&p, &pRow);
			nSel++;
		}
		if (nSel != 1)
		{
			MsgBox("Please select one patient from the NexTech patient list to link");
			return;
		}
		// (c.haag 2011-04-20) - PLID 43351 - Use proper enumerations
		varNextech = m_dlPractice->Value[m_dlPractice->CurSel][ epcID /* PersonID */ ];
	}
	NxCatchAllCall("United Link Error 200: Failed to get selected patients from the NexTech patient list", {return;});

	try {
		// Update the database (this is the big call)
		ExecuteSql("UPDATE PatientsT SET UnitedID = %d WHERE PersonID = %d",
			VarLong(varRemote), VarLong(varNextech));
	}
	NxCatchAllCall("United Link Error 300: Could not link patient", {return;});

	try {
		// Set the new colors
		pRowSettings = m_dlPractice->GetRow(m_dlPractice->CurSel);
		pRowSettings->BackColor = 0x00DAFCD1;
		// (c.haag 2011-04-20) - PLID 43351 - Don't forget to update the United ID
		pRowSettings->PutValue( epcUnitedID, VarLong(varRemote) );

		pRowSettings = m_dlRemote->GetRow(m_dlRemote->CurSel);
		pRowSettings->BackColor = 0x00FCDAD1;

		MsgBox("The link was successfully established");
	}
	NxCatchAll("United Link Error 400: Could not designate list colors");
}

void CUnitedLink::OnMassLink()
{
	LPDISPATCH pDisp = NULL;
	_ConnectionPtr pConRemote;

	if (NULL == (pConRemote = GetRemoteData())) {
		MsgBox("The United Imaging database is invalid, or is inaccessible from your computer.\r\n\r\nThe link to United will be disabled for this session of Practice. Please go to Modules=>Link to United to reconnect with the United Imaging database.");
		EnableLink(FALSE);
		return;
	}

	// (c.haag 2011-04-20) - PLID 43351 - Did some refactoring and cleanup here
	BOOL bStop = FALSE;
	while (!bStop && m_dlPracticeSelected->GetRowCount() > 0)
	{
		IRowSettingsPtr pPracSelectedRow = m_dlPracticeSelected->GetRow(0L);
		CString str, strFirst, strLast;
		_RecordsetPtr prs;
		long lPracticeID = VarLong(pPracSelectedRow->Value[epcID]);
		long lUnitedID = VarLong(pPracSelectedRow->Value[epcUnitedID], -1);
		long lRemoteID = -1;

		if (-1 != lUnitedID)
		{
			// (c.haag 2011-04-20) - PLID 43351 - We never used to check whether a patient is linked with
			// United. If they are, we should skip this patient.
			m_dlPractice->TakeRow(pPracSelectedRow);
		}
		else
		{
			// Find the ID of a United Patient with a matching name
			// (c.haag 2011-04-20) - PLID 43351 - Filter out patients who are already linked in case someone
			// on another terminal has been linking patients too. Also parameterized.
			prs = CreateParamRecordset("SELECT First, Last FROM PersonT INNER JOIN PatientsT ON PatientsT.PersonID = PersonT.ID WHERE PersonT.ID = {INT} AND UnitedID IS NULL", lPracticeID);
			strFirst = AdoFldString(prs, "First", "");
			strLast = AdoFldString(prs, "Last", "");
			prs->Close();
			str.Format("SELECT ID, First, Last, tblPatient.AtHomeYet FROM tblPatient WHERE First = '%s' AND Last = '%s'",
				_Q(strFirst), _Q(strLast));
			prs->Open((LPCTSTR)str, _variant_t(GetRemoteData(), true),
				adOpenStatic, adLockOptimistic, adCmdText);

			// If there is at least one record in United that matches on name...
			if (!(prs->bof && prs->eof))
			{
				prs->MoveLast();
				prs->MoveFirst();

				// If there are multiple records, force the user to choose one.
				if (prs->GetRecordCount() > 1)
				{
					CExportDuplicates dlg(this);

					if(dlg.FindDuplicates(strFirst,strLast,"","Practice", "United", pConRemote, GetRemotePath(), TRUE, m_strRemotePassword, TRUE, ""))
					{
						switch (dlg.DoModal())
						{
							case 1: // Add new
								MsgBox("Add a new patient is not a valid operation in a mass linking. The linking will now stop.");
								bStop = TRUE;
								break;
							case 2: // Update the practice database to link this record.
								lRemoteID = dlg.m_varIDToUpdate.lVal;
								break;
							case 3: // Skip
								break;
							case 0: // Stop
								bStop = TRUE;
								break;
						}
					}
					else
					{
						// (c.haag 2011-04-20) - PLID 43351 - This should never happen because we've just
						// established the fact that there are indeed duplicates. The old code did nothing here;
						// I'm just adding a comment for clarity.
					}
				}
				else {
					// We're not eof and we're not > 1 records, so that must mean we found exactly one matching name.
					lRemoteID = AdoFldLong(prs, "ID");
				}

				// (c.haag 2011-04-20) - PLID 43351 - Update the United ID here.
				if (lRemoteID > -1) {
					ExecuteSql("UPDATE PatientsT SET UnitedID = %d WHERE PersonID = %d",
						lRemoteID, lPracticeID);
					// Update the Practice color and United ID
					pPracSelectedRow->BackColor = 0x00DAFCD1; // Color the Practice row, too
					pPracSelectedRow->PutValue(epcUnitedID, lRemoteID);
					// Update the United color
					long nUnitedRowIndex = m_dlRemote->FindByColumn(ercUnitedID, lRemoteID, 0, VARIANT_FALSE) ;
					if (-1 != nUnitedRowIndex) {
						IRowSettingsPtr pUnitedRow = m_dlRemote->GetRow(nUnitedRowIndex);
						pUnitedRow->BackColor = 0x00FCDAD1;
					} else {
						// Hmm, no row...nothing we can do. Maybe it was added since this window was opened?
					}
				} else {
					// No update was requested
				}
			} // if (!(prs->bof && prs->eof))
			else
			{
				// No records were found. Just skip this patient.
			}

			// Remove the name from the export list unless we've stopped
			if (!bStop)
			{
				m_dlPractice->TakeRow(pPracSelectedRow);
			}
		}
	} // while (!bStop && m_dlPracticeSelected->GetRowCount() > 0)
}

void CUnitedLink::OnUnlink()
{
	CGenericLink::OnUnlink();
}

void CUnitedLink::OnNewPatCheck() 
{
	SetPropertyInt("NewPatExportToUnited", ((CButton*)GetDlgItem(IDC_NEW_PAT_CHECK))->GetCheck());
}

void CUnitedLink::OnCheckLinkUserdefinedid() 
{
	SetPropertyInt("UnitedLinkIDToUnitedID", ((CButton*)GetDlgItem(IDC_CHECK_LINK_USERDEFINEDID))->GetCheck());
}

void CUnitedLink::OnCheckUnitedshowimages() 
{
	SetPropertyInt("UnitedShowImages", ((CButton*)GetDlgItem(IDC_CHECK_UNITEDSHOWIMAGES))->GetCheck());
}

void CUnitedLink::OnCheckEnableunitedlink() 
{
	if (((CButton*)GetDlgItem(IDC_CHECK_ENABLEUNITEDLINK))->GetCheck())
	{
		if (IDNO == MsgBox(MB_YESNO, "This will disable the link to United Imaging on this computer! Are you sure you wish to do this?"))
		{
			((CButton*)GetDlgItem(IDC_CHECK_ENABLEUNITEDLINK))->SetCheck(TRUE);
			return;
		}
		SetPropertyInt("UnitedDisable", TRUE);
		m_statusBar.SetText("The link to United is disabled", 255, 0);
		m_dlPractice->Clear();
		m_dlRemote->Clear();
	}
	else
	{
		if (IDNO == MsgBox(MB_YESNO, "This will enable the link to United Imaging on this computer. Are you sure you wish to do this?"))
		{
			((CButton*)GetDlgItem(IDC_CHECK_ENABLEUNITEDLINK))->SetCheck(FALSE);
			return;
		}
		SetPropertyInt("UnitedDisable", FALSE);
		RefreshData();
	}
}

unsigned long CUnitedLink::RefreshData()
{
	if (GetPropertyInt("UnitedDisable", 0, 0, false))
	{
		MsgBox("The link to United Imaging is disabled on this computer. You must re-enable the link to retrieve the list of United patients.");
		m_statusBar.SetText("The link to United is disabled", 255, 0);
		return -1;
	}
	return CGenericLink::RefreshData();
}

BOOL CUnitedLink::TestConnection(CString strRemotePath)
{
	try {
		_ConnectionPtr pCon(__uuidof(Connection));
		CString strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
				(GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + m_strRemotePassword + ";") : "") +
				"Data Source=" + strRemotePath + ";";	

		// Fail if there is no remote path
		if (strRemotePath.IsEmpty())
			return FALSE;

		// Try connecting
		HR(pCon->Open(_bstr_t((LPCTSTR)strCon), "","",NULL));
		pCon->CursorLocation = adUseClient;

		// Run a test query
		_RecordsetPtr prs(__uuidof(Recordset));
		// (c.haag 2003-10-17 09:04) - I select AtHomeYet to distinguish it from
		// an Inform MDB, which has an eerily similar database structure.
		prs->Open("SELECT tblPatient.AtHomeYet, tblPatient.First, tblPatient.Last, tblPatient.uImageDirectory + '\\' + tblPatient.uStampName + '.bmp' AS ImagePath FROM tblPatient",
			_variant_t((IDispatch *)pCon, true), adOpenForwardOnly, adLockReadOnly, adCmdText);
		prs->Close();
		// (c.haag 2006-10-11 10:10) - We also check for the image table
		prs->Open("SELECT COUNT(uImageID) AS ImageCount FROM [Image]",
			_variant_t((IDispatch *)pCon, true), adOpenForwardOnly, adLockReadOnly, adCmdText);
		prs->Close();

		// Ok we connected and all is well, lets close.
		prs.Release();
		pCon->Close();
		pCon.Release();
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

void CUnitedLink::Unlink(long lNexTechID)
{	
	try
	{
		ExecuteSql("UPDATE PatientsT SET UnitedID = NULL WHERE PersonID = %i", lNexTechID);
	}	
	NxCatchAll("Could not unlink patient");
}

void CUnitedLink::OnBtnUni32Location() 
{
	CFileDialog dlgBrowse(TRUE, "mdb", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "Executable Files|*.exe|All Files|*.*|");
	dlgBrowse.m_ofn.lpstrInitialDir = GetUnitedExecutePath();
	if (dlgBrowse.DoModal() == IDOK) {
		CString strFileName = dlgBrowse.GetPathName();
		SetPropertyText("UnitedExecutePath", strFileName);
		SetDlgItemText(IDC_BTN_UNI32_LOCATION, strFileName);
	}
}

void CUnitedLink::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	// (c.haag 2006-06-30 13:06) - PLID 19977 - Set the location of uni32.exe on the form
	SetDlgItemText(IDC_BTN_UNI32_LOCATION, GetUnitedExecutePath());	
}
