// Zipcodes.cpp : implementation file
//

#include "stdafx.h"
#include "zipcodes.h"
#include "ZipEntry.h"
#include "GlobalDataUtils.h"
#include "GlobalUtils.h"
#include "AuditTrail.h"
#include "RegUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CZipcodes dialog


CZipcodes::CZipcodes(CWnd* pParent)
	: CNxDialog(CZipcodes::IDD, pParent)
	,
	m_zipChecker(NetUtils::ZipCodes, false),
	g_ZipCodePrimaryChecker(NetUtils::PrimaryZipCode)
{
	//{{AFX_DATA_INIT(CZipcodes)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Zip_Codes/add_a_zip_Code.htm";

	m_bDestroying = false;
}

BEGIN_EVENTSINK_MAP(CZipcodes, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CZipcodes)
	ON_EVENT(CZipcodes, IDC_ZIPCODES, 6 /* RButtonDown */, OnRButtonDownZipCodes, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CZipcodes, IDC_ZIPCODES, 9 /* EditingFinishing */, OnEditingFinishingZipcodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CZipcodes, IDC_ZIPCODES, 10 /* EditingFinished */, OnEditingFinishedZipcodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CZipcodes, IDC_ZIPCODES, 18 /* RequeryFinished */, OnRequeryFinishedZipcodes, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CZipcodes::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZipcodes)
	DDX_Control(pDX, IDC_PROMPT, m_btnPromptPrimary);
	DDX_Control(pDX, IDC_SEARCH_ZIP, m_btnSearchZip);
	DDX_Control(pDX, IDC_SEARCH_LABEL, m_lblSearch);
	DDX_Control(pDX, IDC_SEARCH_AREA, m_btnSearchArea);
	DDX_Control(pDX, IDC_SEARCH_CITY, m_btnSearchCity);
	DDX_Control(pDX, IDC_MODIFIED_BTN, m_btnSearchModifiedZips);
	DDX_Control(pDX, IDC_ALL_ZIPS, m_btnSearchAllZips);
	DDX_Control(pDX, IDC_ADD_ZIP, m_btnAddZipCode);
	DDX_Control(pDX, IDC_ZIP_SEARCH, m_nxeditZipSearch);
	DDX_Control(pDX, IDC_DEFAULT_AREA_CODE, m_nxeditDefaultAreaCode);
	DDX_Control(pDX, IDC_ZIPSTATUS, m_nxstaticZipstatus);
	DDX_Control(pDX, IDC_MAKE_PRIMARY, m_btnMakePrimary);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CZipcodes, CNxDialog)
	//{{AFX_MSG_MAP(CZipcodes)
	ON_WM_PAINT()
	ON_COMMAND(ID_ZIP_PRIMARY, OnMakePrimary)
	ON_COMMAND(ID_REM_ZIP_PRIMARY, OnRemPrimary)
	ON_BN_CLICKED(IDC_PROMPT, OnPrompt)
	ON_BN_CLICKED(IDC_MODIFIED_BTN, OnModZip)
	ON_COMMAND(ID_ZIP_DELETE, OnDelete)
	ON_WM_CLOSE()
	ON_COMMAND(ID_ZIP_ADD, OnAddZip)
	ON_BN_CLICKED(IDC_SEARCH_ZIP, OnSearchZip)
	ON_BN_CLICKED(IDC_SEARCH_AREA, OnSearchArea)
	ON_BN_CLICKED(IDC_SEARCH_CITY, OnSearchCity)
	ON_BN_CLICKED(IDC_BTN_SEARCH, OnBtnSearch)
	ON_EN_KILLFOCUS(IDC_DEFAULT_AREA_CODE, OnKillfocusDefaultAreaCode)
	ON_BN_CLICKED(IDC_ALL_ZIPS, OnAllZips)
	ON_MESSAGE(NXM_ZIPCODES_LOADED, OnZipCodesLoaded)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MAKE_PRIMARY, OnMakePrimary)
	ON_BN_CLICKED(IDC_ADD_ZIP, OnAddZip)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZipcodes message handlers

BOOL CZipcodes::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

//	m_addButton.AutoSet(NXB_NEW);
//	m_deleteButton.AutoSet(NXB_DELETE);
	// (z.manning, 04/16/2008) - PLID 29566 - Set button styles
	m_btnAddZipCode.AutoSet(NXB_NEW);
	m_btnMakePrimary.AutoSet(NXB_MODIFY);

	m_pZipCodes = BindNxDataListCtrl(IDC_ZIPCODES, false);
	CheckDlgButton(IDC_SEL_ZIP, 1);
	//m_nOldSize = -1;	

	extern CPracticeApp theApp;

	GetDlgItem(IDC_ZIPSTATUS)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

	m_nSearchType = 1;	//default search to Zip Codes
	CheckDlgButton(IDC_SEARCH_ZIP, true);

	if(GetRemotePropertyInt("PromptZipCodes",0,0,"<None>",TRUE) == 1)
		CheckDlgButton(IDC_PROMPT, true);

	m_pLoadZipCodesThread = NULL;

	//We're going to open up the mdb file here, and just leave it open the entire time this dialog is active - generally people do not use the zip codes unless they need them
	//TODO:  This is code I copied out of an old fix charge resps I had laying around, need to see if there's a better way of doing this.
	/*
	try{
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
		strPath = strPath ^ "ZipCodeList.mdb";
		m_pMDB.CreateInstance("ADODB.Connection");
		m_pMDB->Provider = "Microsoft.Jet.OLEDB.4.0";
		m_pMDB->Open((LPCTSTR)strPath,"", "",adConnectUnspecified);
	} NxCatchAll("Error opening Zip Code List");
	*/

	OnSearchZip();

/*		Code to open a recordset, will be useful later
	_RecordsetPtr rsMDB(__uuidof(Recordset));
	CString sql;
	sql.Format("SELECT * FROM ZipCodeList");

	try{
		rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);
	} catch (_com_error e)
	{
		AfxMessageBox(e.Description());
	}
*/
	return TRUE;
}

void CZipcodes::OnDelete() 
{
	try {
		long nCurSel = m_pZipCodes->CurSel;

		if(nCurSel < 0)
			return;

		//get the UniqueID field out and delete the record
		long nUniqueID = VarLong(m_pZipCodes->GetValue(nCurSel, 5), -1);

		//confirm
		if(AfxMessageBox("Are you SURE you wish to delete this ZipCode?", MB_YESNO) == IDNO)
			return;

		//DRT 8/4/03 - Now we mark things as deleted, don't actually remove them

		//1)  See if it actually exists in our table
		if(nUniqueID == -1 || !ReturnsRecords("SELECT UniqueID FROM ZipCodesT WHERE UniqueID = %li", nUniqueID)) {
			//it does not exist, we need to add it
			CString strZip = VarString(m_pZipCodes->GetValue(nCurSel, 0), "");
			CString strCity = VarString(m_pZipCodes->GetValue(nCurSel, 1), "");
			CString strState = VarString(m_pZipCodes->GetValue(nCurSel, 2), "");
			CString strArea = VarString(m_pZipCodes->GetValue(nCurSel, 3), "");
			long nStaticID = VarLong(m_pZipCodes->GetValue(nCurSel, 4), -1);

			if(nStaticID == -1) {
				//error condition, something seriously screwed up in the data
				MsgBox("The data you are trying to delete could not be properly read.  Please refresh the zipcode list and try again.");
				return;
			}

			// (a.walling 2010-10-04 10:40) - PLID 40573 - Ensure ID is OK
			if (nStaticID == 0xCCCCCCCC || nStaticID == 0xCDCDCDCD) {
				ThrowNxException("Please ensure you are using the latest zip code database");
			}
			ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode, Deleted) values (%li, %li, '%s', '%s', '%s', '%s', 1)", 
				NewNumber("ZipCodesT", "UniqueID"), nStaticID, _Q(strZip), _Q(strCity), _Q(strState), _Q(strArea));
		}
		else {
			//already there, just mark it as deleted
			ExecuteSql("UPDATE ZipCodesT SET Deleted = 1 WHERE UniqueID = %li", nUniqueID);
		}

		//remove the item from the list
		m_pZipCodes->RemoveRow(nCurSel);

	}NxCatchAll("Error deleting ZipCode.");
}

void CZipcodes::OnPaint() 
{
	CPaintDC dc(this);
}

void CZipcodes::OnRButtonDownZipCodes(long nRow, long nCol, long x, long y, long nFlags) 
{
	m_pZipCodes->CurSel = nRow;

	if(nRow == -1)
		return;

	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();
	pMenu->InsertMenu(-1, MF_BYPOSITION, ID_ZIP_ADD, "Add Zip Code");

	//if it's primary, add option to remove.  Otherwise just show 'Make Primary'
	if(m_pZipCodes->GetValue(m_pZipCodes->CurSel, 6).boolVal == 0)
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_ZIP_PRIMARY, "Make Primary Zip Code");
	else
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_REM_ZIP_PRIMARY, "Remove Primary Zip Code");

	//DRT 8/4/03 - You can now mark any zip code as deleted
	pMenu->InsertMenu(-1, MF_BYPOSITION, ID_ZIP_DELETE, "Delete Zip Code");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}

void CZipcodes::OnEditingFinishingZipcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try{
		_RecordsetPtr rs;
		CString strEntered;
		switch(nCol){
		case 2:
			strEntered = strUserEntered;
			//TES 10/10/2003: Don't trim to two characters!  What about our clients overseas?
			strEntered = strEntered.Left(20);
			SetVariantString(*pvarNewValue, (LPCTSTR)strEntered);
			break;
		case 3:
			strEntered = strUserEntered;
			strEntered = strEntered.Left(3);
			SetVariantString(*pvarNewValue, (LPCTSTR)strEntered);
			break;
		}

	}NxCatchAll("Error 100: CZipcodes::OnEditingFinishingZipcodes");
}

void CZipcodes::OnEditingFinishedZipcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
//1/17/03 - DRT - I don't know why I didn't do this originally, but when you edited a code, it only wrote the zipcode and the edited field to the changeable data.
//			It really should have written all the fields, so there's no reason to lookup in 2 db's (which it wasn't doing anyways).  I just made it write the whole
//			thing now.

	if(bCommit) {
		long nCurSel = m_pZipCodes->CurSel;
		bool bStatic = false;
		try{
			CString strZip = VarString(m_pZipCodes->GetValue(nCurSel, 0), "");
			long nID = -1;
			if(m_pZipCodes->GetValue(nCurSel, 4).vt == VT_I4) {
				nID = VarLong(m_pZipCodes->GetValue(nCurSel, 4));
				bStatic = true;
				// (a.walling 2010-10-04 10:40) - PLID 40573 - Ensure ID is OK
				if (nID == 0xCCCCCCCC || nID == 0xCDCDCDCD) {
					ThrowNxException("Please ensure you are using the latest zip code database");
				}
			}
			else
				nID = VarLong(m_pZipCodes->GetValue(nCurSel, 5),-1);	//what to do if the static id is null...

			if(nID == -1) {
				//if it's still -1, then it's not a static id, nor is it in the internal list.  Perhaps it's a ghost of some sort
				AfxMessageBox("Error editing zip codes.  Please refresh the list and try again.");
				return;
			}

			//If the value hasn't changed, do nothing (in particular, don't add the code to our modified list).
			if(VarString(varNewValue,"") != VarString(varOldValue,"")) {
				switch(nCol){
				case 1:
					if(bStatic) {
						//record is being edited from the static list
						if(ReturnsRecords("SELECT StaticID FROM ZipCodesT WHERE StaticID = %li", nID))
							ExecuteSql("UPDATE ZipCodesT SET City = '%s' WHERE StaticID = %li", _Q(VarString(varNewValue, "")), nID);
						else
							ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode) values (%li, %li, '%s', '%s', '%s', '%s')", NewNumber("ZipCodesT", "UniqueID"), nID, _Q(strZip), _Q(VarString(varNewValue, "")), _Q(CString(m_pZipCodes->GetValue(nCurSel, 2).bstrVal)), _Q(CString(m_pZipCodes->GetValue(nCurSel, 3).bstrVal)));
					}
					else {
						//record exists ONLY in practice
						if(ReturnsRecords("SELECT UniqueID FROM ZipCodesT WHERE UniqueID = %li", nID))
							ExecuteSql("UPDATE ZipCodesT SET City = '%s' WHERE UniqueID = %li", _Q(VarString(varNewValue, "")), nID);
						else
							ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode) values (%li, NULL, '%s', '%s', '%s', '%s')", NewNumber("ZipCodesT", "UniqueID"), _Q(strZip), _Q(VarString(varNewValue, "")), _Q(CString(m_pZipCodes->GetValue(nCurSel, 2).bstrVal)), _Q(CString(m_pZipCodes->GetValue(nCurSel, 3).bstrVal)));
					}
					break;
				case 2:
					if(bStatic) {
						//record is being edited from the static list
						if(ReturnsRecords("SELECT StaticID FROM ZipCodesT WHERE StaticID = %li", nID))
							ExecuteSql("UPDATE ZipCodesT SET State = '%s' WHERE StaticID = %li", _Q(VarString(varNewValue, "")), nID);
						else
							ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, State, City, AreaCode) values (%li, %li, '%s', '%s', '%s', '%s')", NewNumber("ZipCodesT", "UniqueID"), nID, _Q(strZip), _Q(VarString(varNewValue, "")), _Q(CString(m_pZipCodes->GetValue(nCurSel, 1).bstrVal)), _Q(CString(m_pZipCodes->GetValue(nCurSel, 3).bstrVal)));
					}
					else {
						//record exists ONLY in practice
						if(ReturnsRecords("SELECT UniqueID FROM ZipCodesT WHERE UniqueID = %li", nID))
							ExecuteSql("UPDATE ZipCodesT SET State = '%s' WHERE UniqueID = %li", _Q(VarString(varNewValue, "")), nID);
						else
							ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, State, City, AreaCode) values (%li, NULL, '%s', '%s', '%s', '%s')", NewNumber("ZipCodesT", "UniqueID"), _Q(strZip), _Q(VarString(varNewValue, "")), _Q(CString(m_pZipCodes->GetValue(nCurSel, 1).bstrVal)), _Q(CString(m_pZipCodes->GetValue(nCurSel, 3).bstrVal)));
					}
					break;
				case 3:
					if(bStatic) {
						//record is being edited from the static list
						if(ReturnsRecords("SELECT StaticID FROM ZipCodesT WHERE StaticID = %li", nID))
							ExecuteSql("UPDATE ZipCodesT SET AreaCode = '%s' WHERE StaticID = %li", _Q(VarString(varNewValue, "")), nID);
						else
							ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, AreaCode, City, State) values (%li, %li, '%s', '%s', '%s', '%s')", NewNumber("ZipCodesT", "UniqueID"), nID, _Q(strZip), _Q(VarString(varNewValue, "")), _Q(CString(m_pZipCodes->GetValue(nCurSel, 1).bstrVal)), _Q(CString(m_pZipCodes->GetValue(nCurSel, 2).bstrVal)));
					}
					else {
						//record exists ONLY in practice
						if(ReturnsRecords("SELECT UniqueID FROM ZipCodesT WHERE UniqueID = %li", nID))
							ExecuteSql("UPDATE ZipCodesT SET AreaCode = '%s' WHERE UniqueID = %li", _Q(VarString(varNewValue, "")), nID);
						else
							ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, AreaCode, City, State) values (%li, NULL, '%s', '%s', '%s', '%s')", NewNumber("ZipCodesT", "UniqueID"), _Q(strZip), _Q(VarString(varNewValue, "")), _Q(CString(m_pZipCodes->GetValue(nCurSel, 1).bstrVal)), _Q(CString(m_pZipCodes->GetValue(nCurSel, 2).bstrVal)));
					}
					break;
				}
				CClient::RefreshTable(NetUtils::ZipCodes);
			}
		}NxCatchAll("Error 100: CZipcodes::OnEditingFinishedZipcodes");	
	}
}

void CZipcodes::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (z.manning, 5/19/2006, PLID 20726) - We may still have focus on a field that has been changed,
	// so let's kill it's focus to ensure that no changes are lost.
	// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
	CheckFocus();

	if (m_zipChecker.Changed() || g_ZipCodePrimaryChecker.Changed())
	{
		StopZipRequery(250); // this is OK if it is not running, wait at most 1/4 of a second
		if(IsDlgButtonChecked(IDC_SEARCH_ZIP))
			LoadZiplist();	//just load the zip list
		else if(IsDlgButtonChecked(IDC_MODIFIED_BTN))
			m_pZipCodes->Requery();
		//otherwise nothing is checked, so don't do anything (because that shouldn't ever happen)
	}

	SetDlgItemText(IDC_DEFAULT_AREA_CODE,GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
}

void CZipcodes::OnMakePrimary() 
{
	long nCurSel = m_pZipCodes->CurSel;
	if(nCurSel == -1)
		return;

	IRowSettingsPtr pRow = m_pZipCodes->GetRow(nCurSel);

	bool bStatic = true;

	try {
		//remove anything that was previously primary
		ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 0 WHERE ZipCode = '%s'", VarString(m_pZipCodes->GetValue(nCurSel, 0)));
		//also remove any records that are all nulls (because they were probably only entered for the primary value)
		ExecuteSql("DELETE FROM ZipCodesT WHERE City IS NULL AND State IS NULL AND AreaCode IS NULL AND ZipCode = '%s'", VarString(m_pZipCodes->GetValue(nCurSel, 0)));

		long nID = -1;
		if(m_pZipCodes->GetValue(nCurSel, 4).vt == VT_I4) {
			nID = VarLong(m_pZipCodes->GetValue(nCurSel, 4));
			
			// (a.walling 2010-10-04 10:40) - PLID 40573 - Ensure ID is OK
			if (nID == 0xCCCCCCCC || nID == 0xCDCDCDCD) {
				ThrowNxException("Please ensure you are using the latest zip code database");
			}
		}

		if(nID == -1) {	//internal only code
			nID = VarLong(m_pZipCodes->GetValue(nCurSel, 5),-1);
			bStatic = false;
			if(nID == -1) {
				//STILL not correct, this is bad
				AfxMessageBox("Error setting primary zip code.  Please refresh and try again.");
				return;
			}
		}

		if(bStatic) {	//we're working with a code which came from the static database
			if(ReturnsRecords("SELECT StaticID FROM ZipCodesT WHERE StaticID = %li", nID)) {
				//if the record exists, update the PrimaryZip column
				ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 1 WHERE StaticID = %li", nID);
			}
			else {
				//insert a new record
				ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip) values (%li, %li, '%s', '%s', '%s', '%s', %li)", 
					NewNumber("ZipCodesT", "UniqueID"), nID, _Q(VarString(m_pZipCodes->GetValue(nCurSel, 0))), _Q(VarString(m_pZipCodes->GetValue(nCurSel, 1))), _Q(VarString(m_pZipCodes->GetValue(nCurSel, 2))), _Q(VarString(m_pZipCodes->GetValue(nCurSel, 3))), 1);
			}
		}
		else {	//we're working with a code which was inserted into practice by a user
			if(ReturnsRecords("SELECT UniqueID FROM ZipCodesT WHERE UniqueID = %li", nID)) {
				//if the record exists, update the PrimaryZip column
				ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 1 WHERE UniqueID = %li", nID);
			}
			else {
				//insert a new record
				ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip) values (%li, NULL, '%s', NULL, NULL, NULL, %li)", 
					NewNumber("ZipCodesT", "UniqueID"), VarString(m_pZipCodes->GetValue(nCurSel, 0)), 1);
			}
		}

		// (a.walling 2007-03-19 13:10) - PLID 24245 - Colour the list appropriately without having to requery the datalist.

		// and ensure no other rows are coloured with this same zip code.
		// instead of looking for matchins zips, let's look for other primaries and then match zips

		_variant_t varZip = pRow->GetValue(0);
		
		bool bFound = false;

		long nFoundRow = m_pZipCodes->FindByColumn(6, g_cvarTrue, 0, VARIANT_FALSE);
		long nFirstFoundRow = nFoundRow;

		if (nFoundRow != sriNoRow) {
			while (!bFound) {
				IRowSettingsPtr pCurRow = m_pZipCodes->GetRow(nFoundRow);

				if (pCurRow->GetValue(0) == varZip) {
					// matches the zip code! so un-colorize as needed...
					pCurRow->PutValue(6, g_cvarFalse); /// no longer primary
					bFound = true;
					if (pCurRow->GetValue(4).vt != VT_NULL) {
						// this means that this code has a static ID and exists in the master Zip database
						pCurRow->ForeColor = RGB(0,0,0);
						pCurRow->ForeColorSel = RGB(255,255,255);
					} else {
						// otherwise, this code was manually added by the user. so make it blue.
						pCurRow->ForeColor = RGB(0,0,255);
						pCurRow->ForeColorSel = RGB(0,0,255);
					}
				}

				nFoundRow = m_pZipCodes->FindByColumn(6, g_cvarTrue, nFoundRow + 1, VARIANT_FALSE);

				if (nFoundRow == nFirstFoundRow) {
					bFound = true;
				}
			}
		}

		// colour this row red
		pRow->ForeColor = RGB(255,0,0);
		pRow->ForeColorSel = RGB(255,0,0);
		pRow->PutValue(6, g_cvarTrue); // mark as primary

		g_ZipCodePrimaryChecker.Refresh();

		//now refresh the list so the right one is shown as primary

		// (a.walling 2007-03-19 13:00) - PLID 24245 - Annoying to always refresh. We handle updating in the code above.
/*		if (m_nSearchType == 5) {
			// special case, which prevents such heavy refreshing. so force it by changing the value
			m_nSearchType = -1;
		}
		LoadZiplist();*/
	} NxCatchAll("Error setting primary zip code");

}

void CZipcodes::OnPrompt() 
{
	if(IsDlgButtonChecked(IDC_PROMPT))
		SetRemotePropertyInt("PromptZipCodes",1,0,"<None>");
	else
		SetRemotePropertyInt("PromptZipCodes",0,0,"<None>");
}

BOOL CZipcodes::OnCommand(WPARAM wParam, LPARAM lParam) 
{

	switch(HIWORD(wParam)) {
	case EN_CHANGE:
		//only for zip code search
		switch(LOWORD(wParam)) {		
		case IDC_ZIP_SEARCH:
			{
				/* JMJ 7/28/2003 - functionality replaced by search button
				if(m_nSearchType == 1) {
					CString strZip;
					GetDlgItemText(IDC_ZIP_SEARCH, strZip);

					if(strZip.GetLength() == 3) {
						//starting a new set of zip codes, regrab the whole list
						LoadZiplist();
					}
					else if(strZip.GetLength() > 3) {
						//we're going to remove all the codes which do not meet the new criteria
						TrimZiplist();
					}
					else {
						//there's nothing in the box, so clear the datalist
						m_pZipCodes->Clear();
					}

					m_nOldSize = strZip.GetLength();	//save the size for next time around
				}
				*/
			}
			break;	//IDC_ZIP_SEARCH		
		default:
			break;	//default
		}
	break;	//EN_CHANGE

	case EN_KILLFOCUS:
		//for area code / city search
		switch(LOWORD(wParam)) {
		case IDC_ZIP_SEARCH:
			{
				/* JMJ 7/28/2003 - functionality replaced by search button
				LoadZiplist();
				*/
			}
		break;	//IDC_ZIP_SEARCH
		default:
			break;	//default
		}
	default:
		break;	//default (HIWORD)
	
	break;	//WM_KILLFOCUS
	}
	


	return CNxDialog::OnCommand(wParam, lParam);
}

void CZipcodes::LoadZiplist() {
//this function is pretty much a modified Requery (that is much slower) for pulling records from both places

	//DRT 3/6/03 - We now have the ability to search by zip code, area code, or city

	CWaitCursor pWait;

	/* this is unnecessary now (PLID 23754)
	if (m_nSearchType == 5) {
		m_pZipCodes->PutAdoConnection(GetRemoteData());
	}
	*/

	//OK, before we do anything else, if we're showing modified zip codes, then let's not bother with this.
	if(IsDlgButtonChecked(IDC_MODIFIED_BTN)) {
		OnModZip();
		return;
	}

	if(IsDlgButtonChecked(IDC_ALL_ZIPS)) {
		OnAllZips();
		return;
	}

	//OK, we're showing the selected search, let's do this thing.
#ifdef _DEBUG
	LogDetail("CZipCodes::LoadZiplist() Clearing datalist");
#endif
	m_pZipCodes->Clear();
	CString strSearch;

	GetDlgItemText(IDC_ZIP_SEARCH, strSearch);	//get the search criteria from the edit box

	strSearch.TrimLeft(); // it is possible, though unlikely, that the user would purposefully be searching for a space at the end.

	//truncate the search criteria to no more than 100 chars, which is way more than necessary even
	if (strSearch.GetLength() > 100) {
		strSearch = strSearch.Left(100);
	}

	//JMJ 3/9/2004 - PLID 11334 - Don't let them search on an empty string
	//(though I decided searching for a space is fine)
	// (a.walling 2007-01-23 09:48) - PLID 23574 - Searching for a space makes no sense in this context, really.
	if(strSearch.GetLength() == 0) {
		return;
	}

	//DRT 11/19/2003 - PLID 8187 - Changed it so that 
	if(strSearch.GetLength() < 2 && !IsDlgButtonChecked(IDC_SEARCH_ZIP)) {
		//searching for 1 char cities or area codes is silly (and crazy slow)
		AfxMessageBox("Please enter at least 2 characters when searching cities or area codes.");
		return;
	}

	try{
		// (a.walling 2006-12-06 09:55) - PLID 23754
		StopZipRequery(250); // this is OK if the thread is not running. Wait at most 1/4 of a second.

		// (j.gruber 2009-10-07 12:32) - PLID 35827 - modified to support city list
		BOOL bSearchCityFile = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");
		//_RecordsetPtr rsMDB(__uuidof(Recordset));
		if (!bSearchCityFile) {
			ZipcodeUtils::EZipcodeField ezfField = ZipcodeUtils::zfNone;

			//figure out what fields we need to search in the SQL dbs
			if(IsDlgButtonChecked(IDC_SEARCH_ZIP)) {
				ezfField = ZipcodeUtils::zfZip;
			}
			else if(IsDlgButtonChecked(IDC_SEARCH_AREA)) {
				long nOriginal = strSearch.GetLength();
				strSearch = strSearch.SpanIncluding("0123456789");
				if (nOriginal != strSearch.GetLength()) {
					MessageBox("Area codes must be search with only numbers.");
					return;
				}
				ezfField = ZipcodeUtils::zfAreaCode;
			}
			else if(IsDlgButtonChecked(IDC_SEARCH_CITY)) {
				ezfField = ZipcodeUtils::zfCity;
			}

			//sql.Format("SELECT ID, Zip, City, State, Area FROM ZipCodeList WHERE %s LIKE '%s%%'", strField, _Q(strSearch));

			//rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) m_pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);

			// (a.walling 2006-12-06 09:55) - PLID 23754 - Start the zip query for this criteria
			StartZipRequery(ezfField, strSearch);
		}
		else {
			CityZipUtils::ECityZipField ezfField = CityZipUtils::zfNone;

			//figure out what fields we need to search in the SQL dbs
			if(IsDlgButtonChecked(IDC_SEARCH_ZIP)) {
				ezfField = CityZipUtils::zfZip;
			}
			else if(IsDlgButtonChecked(IDC_SEARCH_AREA)) {
				long nOriginal = strSearch.GetLength();
				strSearch = strSearch.SpanIncluding("0123456789");
				if (nOriginal != strSearch.GetLength()) {
					MessageBox("Area codes must be search with only numbers.");
					return;
				}
				ezfField = CityZipUtils::zfAreaCode;
			}
			else if(IsDlgButtonChecked(IDC_SEARCH_CITY)) {
				ezfField = CityZipUtils::zfCity;
			}
			
			StartCityRequery(ezfField, strSearch);
		}


	} NxCatchAll("Error loading zip codes");
}

/*
void CZipcodes::TrimZiplist() {

	CString strZip;
	GetDlgItemText(IDC_ZIP_SEARCH, strZip);
	long nSize = strZip.GetLength();

	if((nSize < m_nOldSize) || (abs(nSize - m_nOldSize) > 1))	{	//backspacing, or if they pasted in more than 1 character, just reload the whole list
		LoadZiplist();
		return;
	}

	try {

		//turn off drawing
		m_pZipCodes->SetRedraw(false);

		//loop through the datalist and make sure everything matches
		for(int i = m_pZipCodes->GetRowCount() - 1; i >= 0; i--) {
			CString str = VarString(m_pZipCodes->GetValue(i, 0));

			if(str.Left(nSize) != strZip) {
				//row no longer matches, remove it
				m_pZipCodes->RemoveRow(i);
			}
		}

		//redraw again
		m_pZipCodes->SetRedraw(true);

	} NxCatchAll("Error modifying zip code list");

}
*/

void CZipcodes::OnClose() 
{

}

void CZipcodes::OnAddZip() 
{
	//add a new zip code, it'll just be entered into practice table
	try {
		CZipEntry dlg;
		if (dlg.DoModal() == IDOK){
			IRowSettingsPtr pRow;
			pRow = m_pZipCodes->GetRow(-1);
			pRow->PutValue(0, _variant_t(dlg.m_strZip));
			pRow->PutValue(1, _variant_t(dlg.m_strCity));
			pRow->PutValue(2, _variant_t(dlg.m_strState));
			pRow->PutValue(3, _variant_t(dlg.m_strAreaCode));
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->PutValue(4, varNull);
			pRow->PutValue(5, _variant_t(dlg.m_nNewID));
			pRow->PutValue(6, long(0));
			m_pZipCodes->InsertRow(pRow, -1);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiZipCreated, -1, "", dlg.m_strZip, aepMedium, aetCreated);

			CClient::RefreshTable(NetUtils::ZipCodes);
		}

	} NxCatchAll("Error adding zip code");
		
}

void CZipcodes::OnRequeryFinishedZipcodes(short nFlags) 
{
	if (nFlags == 0) {
		//we are coloring it ourselves with this type because this takes way too long!
		if (m_nSearchType != 5) {
			CWaitCursor  cw;
			//Now we need to color appropriately.
			for(int i = 0; i < m_pZipCodes->GetRowCount(); i++) {
				if(VarBool(m_pZipCodes->GetValue(i, 6), FALSE)) {
					IRowSettingsPtr pRow = m_pZipCodes->GetRow(i);
					pRow->ForeColor = RGB(255,0,0);
					pRow->ForeColorSel = RGB(255,0,0);
				}
			}
		}

		CString str;
		str.Format("Loaded %li items.", m_pZipCodes->GetRowCount());

		SetDlgItemText(IDC_ZIPSTATUS, str);
	}
}

void CZipcodes::OnRemPrimary() {

	try {

		ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 0 WHERE UniqueID = %li", long(m_pZipCodes->GetValue(m_pZipCodes->CurSel, 5).lVal));

		//update the datalist to know about the change
		m_pZipCodes->PutValue(m_pZipCodes->CurSel, 6, long(0));

		//remove the color
		IRowSettingsPtr pRow;
		pRow = m_pZipCodes->GetRow(m_pZipCodes->CurSel);
		if (pRow->GetValue(4).vt != VT_NULL) {
			pRow->ForeColor = RGB(0,0,0);
			pRow->ForeColorSel = RGB(255,255,255);
		} else {
			// blue if manually added code
			pRow->ForeColor = RGB(0,0,255);
			pRow->ForeColorSel = RGB(0,0,255);
		}

		g_ZipCodePrimaryChecker.Refresh();

	}NxCatchAll("Error removing Primary ZipCode status");
}

void CZipcodes::FilterSearch() 
{
	//DRT 3/6/03 - This is now called when anyone selects a search type, be it "zip code", "area code", or "city"

	//DRT 4/12/2004 - PLID 11745 - If you are waiting on a requery and try to set these before
	//	it finishes, you can get internal requery errors.  Cancel the requery b/c we're just
	//	going to do it again.
	if(m_pZipCodes->IsRequerying())
		m_pZipCodes->CancelRequery();

	//get rid of anything in the datalist clauses, we're not using them (but the modified is)
	m_pZipCodes->FromClause = "";
	m_pZipCodes->WhereClause = "";

	//enable the edit box
	GetDlgItem(IDC_ZIP_SEARCH)->EnableWindow(true);

	//now load the datalist with everything in the search box
	CString strSearch;
	GetDlgItemText(IDC_ZIP_SEARCH, strSearch);

	m_pZipCodes->CancelRequery();

#ifdef _DEBUG
	LogDetail("CZipCodes::LoadZiplist() Clearing datalist");
#endif
	m_pZipCodes->Clear();

	//if there is anything in the box, wipe it out
	SetDlgItemText(IDC_ZIP_SEARCH, "");

	if(m_nSearchType >= 1 && m_nSearchType <= 3)
		GetDlgItem(IDC_ZIP_SEARCH)->SetFocus();
}

void CZipcodes::OnSearchZip() 
{
	//DRT 3/6/03 - This is the code executed when the user clicks on "Search Zip Codes"
	if(m_nSearchType == 1) {
		//we're already searching zip codes, don't do anything
		return;
	}

	/* this is unnecessary now (PLID 23754)
	if (m_nSearchType == 5) {
		m_pZipCodes->PutAdoConnection(GetRemoteData());
	}
	*/

	//set the label appropriately
	m_lblSearch.SetWindowText("Selected Zip Code:");
	CRect rect;
	GetDlgItem(IDC_SEARCH_LABEL)->GetWindowRect(rect);
	ScreenToClient(rect);
	InvalidateRect(rect);

	//set the search type
	m_nSearchType = 1;

	// stop any requery if necessary
	StopZipRequery(250);

	//filter for the search criteria
	FilterSearch();
}

void CZipcodes::OnSearchArea() 
{
	//DRT 3/6/03 - This is the code executed when the user clicks on "Search Area Codes"
	if(m_nSearchType == 2) {
		//we're already searching area codes, don't do anything
		return;
	}

	/* this is unnecessary now (PLID 23754)
	if (m_nSearchType == 5) {
		m_pZipCodes->PutAdoConnection(GetRemoteData());
	}
	*/

	//set the label appropriately
	m_lblSearch.SetWindowText("Selected Area Code:");
	CRect rect;
	GetDlgItem(IDC_SEARCH_LABEL)->GetWindowRect(rect);
	ScreenToClient(rect);
	InvalidateRect(rect);

	//set the search type
	m_nSearchType = 2;

	// stop any requery if necessary
	StopZipRequery(250);

	//filter for the search criteria
	FilterSearch();
}

void CZipcodes::OnSearchCity() 
{
	//DRT 3/6/03 - This is the code executed when the user clicks on "Search City"
	if(m_nSearchType == 3) {
		//we're already searching cities, don't do anything
		return;
	}

	/* this is unnecessary now (PLID 23754)
	if (m_nSearchType == 5) {
		m_pZipCodes->PutAdoConnection(GetRemoteData());
	}
	*/

	//set the label appropriately
	m_lblSearch.SetWindowText("Selected City:");
	CRect rect;
	GetDlgItem(IDC_SEARCH_LABEL)->GetWindowRect(rect);
	ScreenToClient(rect);
	InvalidateRect(rect);

	//set the search type
	m_nSearchType = 3;

	// stop any requery if necessary
	StopZipRequery(250);

	//filter for the search criteria
	FilterSearch();
}

void CZipcodes::OnModZip() 
{
	//set the search type
	m_nSearchType = 4;

	//disable the edit box
	GetDlgItem(IDC_ZIP_SEARCH)->EnableWindow(false);

	//DRT 4/12/2004 - PLID 11745 - If you are waiting on a requery and try to set these before
	//	it finishes, you can get internal requery errors.  Cancel the requery b/c we're just
	//	going to do it again.
	if(m_pZipCodes->IsRequerying())
		m_pZipCodes->CancelRequery();

	StopZipRequery(250); // this is OK if the thread is not running. Wait at most 1/4 of a second.

	//set the connection
	/* this is unnecessary now (PLID 23754)
	m_pZipCodes->PutAdoConnection(GetRemoteData());
	*/

	//now load the datalist with everything that has been modified
	// (j.gruber 2009-10-07 13:30) - PLID 35827 - took out city because we fill that in now
	m_pZipCodes->FromClause = _bstr_t("ZipCodesT");
	m_pZipCodes->WhereClause = _bstr_t("(State IS NOT NULL OR AreaCode IS NOT NULL) AND Deleted = 0");
	SetDlgItemText(IDC_ZIPSTATUS, "Loading, please wait...");
	GetDlgItem(IDC_ZIPSTATUS)->RedrawWindow();
	m_pZipCodes->Requery();
}

void CZipcodes::OnBtnSearch() 
{
	LoadZiplist();
}

void CZipcodes::OnKillfocusDefaultAreaCode() 
{
	try {

		CString strAreaCode;
		GetDlgItemText(IDC_DEFAULT_AREA_CODE,strAreaCode);

		strAreaCode = strAreaCode.Left(3);

		SetRemotePropertyText("DefaultAreaCode",strAreaCode,0,"<None>");

		SetDlgItemText(IDC_DEFAULT_AREA_CODE,strAreaCode);

	}NxCatchAll("Error saving default area code.");
}

void CZipcodes::OnAllZips() 
{

	try {

		if (m_nSearchType == 5) {
			return;
		}

		// (a.walling 2006-12-12 14:39) - PLID 23838 - Warn the user when loading a huge recordset into a datalist.
		long nZipcodeCount = -1;
		BOOL bSearchCities = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");
		if (!bSearchCities) {
			nZipcodeCount = ZipcodeUtils::GetTotalRecords();
		}
		else {
			nZipcodeCount = CityZipUtils::GetTotalRecords();
		}
			
		if (nZipcodeCount > 100000) {
			CString strMsg;

			strMsg.Format("There are %li records in the database. They will load in the background, but will take a large amount"
				" of processor power and resources to display, which may cause the software to be temporarily unresponsive. We"
				" recommend that you use the various search functions instead.\r\n\r\nAre you sure you want to continue?", nZipcodeCount);

			if (IDNO == MessageBox(strMsg, "Practice", MB_YESNO | MB_ICONEXCLAMATION)) {
				//reset the checkbox
				CheckDlgButton(IDC_ALL_ZIPS, FALSE);

				switch(m_nSearchType) {
				case 1:
					CheckDlgButton(IDC_SEARCH_ZIP, TRUE);
					break;
				case 2:
					CheckDlgButton(IDC_SEARCH_AREA, TRUE);
					break;
				case 3:
					CheckDlgButton(IDC_SEARCH_CITY, TRUE);
					break;
				case 4:
					CheckDlgButton(IDC_MODIFIED_BTN, TRUE);
					break;
				default:
					ASSERT(FALSE);
					CheckDlgButton(IDC_SEARCH_ZIP, TRUE);
					OnSearchZip(); // ensures that the state is synchronized with the zip dialog button.
					break;
				}
				return;
			}
		}

		
		CWaitCursor  cw;

		m_nSearchType = 5;

		GetDlgItem(IDC_ZIP_SEARCH)->EnableWindow(false);

		//DRT 4/12/2004 - PLID 11745 - If you are waiting on a requery and try to set these before
		//	it finishes, you can get internal requery errors.  Cancel the requery b/c we're just
		//	going to do it again.
		if(m_pZipCodes->IsRequerying())
			m_pZipCodes->CancelRequery();

#ifdef _DEBUG
		LogDetail("CZipCodes::LoadZiplist() Clearing datalist");
#endif
		m_pZipCodes->Clear();
		
		/*
		m_pZipCodes->PutAdoConnection(m_pMDB);
		

		//first, load all the zip codes from the mdb
		CString sql;
		sql.Format("(SELECT Zip AS ZipCode, City, State, Area AS AreaCode, NULL AS StaticID, NULL AS UniqueID, NULL AS PrimaryZip FROM ZipCodeList)");
		m_pZipCodes->FromClause = (LPCTSTR)sql;
		m_pZipCodes->WhereClause = "";

		m_pZipCodes->Requery();
		*/

		// (a.walling 2006-12-06 09:54) - PLID 23754 - Start a zip query for all zip codes!
		// all filling of the datalist is handled in OnZipCodesLoaded()
		if (!bSearchCities) {
			StartZipRequery(ZipcodeUtils::zfNone, "");	
		}
		else {
			StartCityRequery(CityZipUtils::zfNone, "");	
		}


		/*

		_RecordsetPtr rsMDB(__uuidof(Recordset));		
		rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) m_pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);

		//turn off redrawing so it's faster
		m_pZipCodes->SetRedraw(false);

		//load all these records into the datalist
		while(!rsMDB->eof) {
			FieldsPtr fields = rsMDB->Fields;
			IRowSettingsPtr pRow;
			pRow = m_pZipCodes->GetRow(-1);
			pRow->PutValue(0, fields->Item["Zip"]->Value);	//zip
			pRow->PutValue(1, fields->Item["City"]->Value);	//city
			pRow->PutValue(2, fields->Item["State"]->Value);	//state
			pRow->PutValue(3, fields->Item["Area"]->Value);	//area
			pRow->PutValue(4, fields->Item["ID"]->Value);	//static id
			_variant_t var;
			var.vt = VT_NULL;
			pRow->PutValue(5, var);	//unique ID will be blank for these
			pRow->PutValue(6, long(0)); //PrimaryZip will be off for these
			//insert into the datalist
			m_pZipCodes->AddRow(pRow);
			rsMDB->MoveNext();
		}

		rsMDB->Close();
		*/


		// we used to take the items from ZipCodesT and add them here; instead we do it when the zip codes are finished loading
		//m_pZipCodes->SetRedraw(true);
	}NxCatchAll("Error loading All Zipcodes");
	
	
}


//TES 9/15/03: I'm taking out this function because it is far slower than just calling m_pZipCodes->Clear();
/*void CZipcodes::ClearZipBox() {

	m_pZipCodes->SetRedraw(FALSE);
	for(int i =0; i < m_pZipCodes->GetRowCount(); i++) {

		m_pZipCodes->RemoveRow(i);
	}
	m_pZipCodes->SetRedraw(TRUE);

}*/

LRESULT CZipcodes::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::ZipCodes:
			case NetUtils::PrimaryZipCode: {
				try {
					UpdateView();
				} NxCatchAll("Error in CZipcodes::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CZipcodes::OnTableChanged");

	return 0;
}

// (a.walling 2006-12-06 09:57) - PLID 23754 - Our main handler for the Zip Code loaded message.
// this message is posted even when the thread was cancelled. It is our duty to clean up any
// memory here, but mostly to load the ZipSet into the datalist. Also the GUI is updated to show
// how many results were returned.
LRESULT CZipcodes::OnZipCodesLoaded(WPARAM wParam, LPARAM lParam) {

	BOOL bSearchCities = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");
	BOOL bContinue = FALSE;
	CString strField, strCriteria;

	ZipcodeUtils::ZipcodeSet* pZs = NULL;
	ZipcodeUtils::ZipcodeLoadingCriteria* pLoad = NULL;

	CityZipUtils::CityZipSet* pCs = NULL;
	CityZipUtils::CityZipLoadingCriteria* pCityLoad = NULL;

	try {
		// (d.singleton 2011-11-18 10:01) - PLID 18875 - set pLoad and check bFailure to see if we failed to get the ZipCodeList.dat file
		pLoad = (ZipcodeUtils::ZipcodeLoadingCriteria*)lParam;
		if(pLoad->bFailure)
		{
			AfxMessageBox("The city and state related to this zip code could not be loaded, please check the install path for accuracy and make sure the ZipCodeList.dat file exists,  or call NexTech Technical Support for help:  1-866-654-4396");
			SetDlgItemText(IDC_ZIPSTATUS, "");
		}

		if (!bSearchCities) {			
		
			pZs = (ZipcodeUtils::ZipcodeSet *)wParam;
			// (d.singleton 2011-11-18 10:00) - PLID 18875 - no longer need to set pLoad as its done above.
			//pLoad = (ZipcodeUtils::ZipcodeLoadingCriteria*)lParam;
			
			CString strMsg, strTemp;

			if (pLoad) {
				strField = GetDatabaseField(pLoad->ezfField);
				strCriteria = pLoad->lpszCriteria;
			}

			if ((m_bDestroying) || (m_pLoadZipCodesThread == NULL) || (m_hStopLoadingZips == INVALID_HANDLE_VALUE) || (m_hStopLoadingZips != pLoad->hStopLoadingEvent)) {
#ifdef _DEBUG
			LogDetail("CZipCodes::OnZipCodesLoaded(pZs %li, %li) Old message event(%li) Current data (event %li, thread %li)", wParam, lParam, pLoad->hStopLoadingEvent, m_hStopLoadingZips, m_pLoadZipCodesThread);
#endif
					
				CloseHandle(pLoad->hStopLoadingEvent); // close this event

				// if these are not the same events or our thread pointer is gone, then this thread's data is not what we want and therefore useless.
				if (pZs != NULL) {
#ifdef _DEBUG
				LogDetail("CZipCodes::OnZipCodesLoaded(pZs %li, %li) Clearing data", wParam, lParam);
#endif

					ZipcodeUtils::ClearZipcodeSet(pZs);
					pZs = NULL;
	//				old request
				} else {		
	//				old request, no data
				}
				
				ZipcodeUtils::ClearZipcodeThreadData(pLoad);
				pLoad = NULL;

				return 0;
			} else {
	#ifdef _DEBUG	
				LogDetail("CZipCodes::OnZipCodesLoaded(pZs %li, %li) Valid message event(%li) Current data (event %li, thread %li)", wParam, lParam, pLoad->hStopLoadingEvent, m_hStopLoadingZips, m_pLoadZipCodesThread);
	#endif

				// the message is valid, check to see if it's current or not. 
				bool bCurrent = (pLoad->hStopLoadingEvent == m_hStopLoadingZips);
				CloseHandle(pLoad->hStopLoadingEvent); // close the event

				ZipcodeUtils::ClearZipcodeThreadData(pLoad);
				pLoad = NULL;

				if (bCurrent) {
					m_hStopLoadingZips = INVALID_HANDLE_VALUE;
					m_pLoadZipCodesThread = NULL;
				} else {
					// free this memory
					if (pZs != NULL) {
	#ifdef _DEBUG
						LogDetail("**CZipCodes::OnZipCodesLoaded(pZs %li, %li) Clearing data", wParam, lParam);
	#endif
						ZipcodeUtils::ClearZipcodeSet(pZs);
						pZs = NULL;
					}
					return 0;
				}
			}

			if (pZs != NULL) { // this means data was returned (ie, it was not stopped)
				// regardless, we are adding returned data to the list.

				bContinue = TRUE;

				CWaitCursor wc;

				CString str;

				SetDlgItemText(IDC_ZIPSTATUS, "Processing, please wait...");
				GetDlgItem(IDC_ZIPSTATUS)->RedrawWindow();

#ifdef _DEBUG
			LogDetail("CZipCodes::OnZipCodesLoaded(pZs %li, %li) Populating datalist with %li entries", wParam, lParam, (long)pZs->GetSize());
#endif


				for (int i = 0; i < pZs->GetSize(); i++) {
					IRowSettingsPtr pRow;
					pRow = m_pZipCodes->GetRow(-1);
					ZipcodeUtils::ZipcodeInfo* zi = pZs->GetAt(i);

					if (zi) {
						pRow->PutValue(0, AsVariant(zi->Zip));
						pRow->PutValue(1, AsVariant(zi->City));
						pRow->PutValue(2, AsVariant(zi->State));

						str.Format("%03li", zi->nArea);
						pRow->PutValue(3, AsVariant(str));
						pRow->PutValue(4, zi->nID);
						pRow->PutValue(5, g_cvarNull);
						pRow->PutValue(6, g_cvarFalse); //use false for the primary flag instead of null

						m_pZipCodes->InsertRow(pRow, -1);
					}
				}

#ifdef _DEBUG
			LogDetail("CZipCodes::OnZipCodesLoaded(pZs %li, %li) Clearing data", wParam, lParam);
#endif


				ZipcodeUtils::ClearZipcodeSet(pZs); // free all of our data
				pZs = NULL;
			}

		}
		else {
			/***** BEGIN CITY LOOKUP PORTION ******************/
				
			pCs = (CityZipUtils::CityZipSet *)wParam;
			pCityLoad = (CityZipUtils::CityZipLoadingCriteria*)lParam;

			CString strMsg, strTemp;

			if (pCityLoad) {
				strField = GetDatabaseField(pCityLoad->ezfField);
				strCriteria = pCityLoad->lpszCriteria;
			}

			if ((m_bDestroying) || (m_pLoadZipCodesThread == NULL) || (m_hStopLoadingZips == INVALID_HANDLE_VALUE) || (m_hStopLoadingZips != pCityLoad->hStopLoadingEvent)) {
#ifdef _DEBUG
		LogDetail("CZipCodes::OnZipCodesLoaded(pCs %li, %li) Old message event(%li) Current data (event %li, thread %li)", wParam, lParam, pCityLoad->hStopLoadingEvent, m_hStopLoadingZips, m_pLoadZipCodesThread);
#endif
				
				CloseHandle(pCityLoad->hStopLoadingEvent); // close this event

				// if these are not the same events or our thread pointer is gone, then this thread's data is not what we want and therefore useless.
				if (pCs != NULL) {
#ifdef _DEBUG
				LogDetail("CZipCodes::OnZipCodesLoaded(pCs %li, %li) Clearing data", wParam, lParam);
#endif

					CityZipUtils::ClearCityZipSet(pCs);
					pCs = NULL;
	//				old request
				} else {		
//					old request, no data
				}
			
				CityZipUtils::ClearCityZipThreadData(pCityLoad);
				pCityLoad = NULL;

				return 0;
			} else {
#ifdef _DEBUG	
			LogDetail("CZipCodes::OnZipCodesLoaded(pCs %li, %li) Valid message event(%li) Current data (event %li, thread %li)", wParam, lParam, pCityLoad->hStopLoadingEvent, m_hStopLoadingZips, m_pLoadZipCodesThread);
#endif

				// the message is valid, check to see if it's current or not. 
				bool bCurrent = (pCityLoad->hStopLoadingEvent == m_hStopLoadingZips);
				CloseHandle(pCityLoad->hStopLoadingEvent); // close the event

				CityZipUtils::ClearCityZipThreadData(pCityLoad);
				pCityLoad = NULL;

				if (bCurrent) {
					m_hStopLoadingZips = INVALID_HANDLE_VALUE;
					m_pLoadZipCodesThread = NULL;
				} else {
					// free this memory
					if (pCs != NULL) {
#ifdef _DEBUG
		LogDetail("**CZipCodes::OnZipCodesLoaded(pCs %li, %li) Clearing data", wParam, lParam);
#endif
						CityZipUtils::ClearCityZipSet(pCs);
						pCs = NULL;
					}
					return 0;
				}
			}
			
		

			if (pCs != NULL) { // this means data was returned (ie, it was not stopped)
				// regardless, we are adding returned data to the list.
				bContinue = TRUE;
				CWaitCursor wc;

				CString str;

				SetDlgItemText(IDC_ZIPSTATUS, "Processing, please wait...");
				GetDlgItem(IDC_ZIPSTATUS)->RedrawWindow();

#ifdef _DEBUG
			LogDetail("CZipCodes::OnZipCodesLoaded(pCs %li, %li) Populating datalist with %li entries", wParam, lParam, (long)pCs->GetSize());
#endif


				for (int i = 0; i < pCs->GetSize(); i++) {
					IRowSettingsPtr pRow;
					pRow = m_pZipCodes->GetRow(-1);
					CityZipUtils::CityZipInfo* zi = pCs->GetAt(i);

					if (zi) {
						pRow->PutValue(0, AsVariant(zi->Zip));
						pRow->PutValue(1, AsVariant(zi->City));
						pRow->PutValue(2, AsVariant(zi->State));

						str.Format("%03li", zi->nArea);
						pRow->PutValue(3, AsVariant(str));
						pRow->PutValue(4, zi->nID);
						pRow->PutValue(5, g_cvarNull);
						pRow->PutValue(6, g_cvarFalse); //use false for the primary flag instead of null

						m_pZipCodes->InsertRow(pRow, -1);
					}
				}

#ifdef _DEBUG
			LogDetail("CZipCodes::OnZipCodesLoaded(pCs %li, %li) Clearing data", wParam, lParam);
#endif


				CityZipUtils::ClearCityZipSet(pCs); // free all of our data
				pCs = NULL;
			}	

		}


		/***END ZIP/CITY SPLIT ****/	
		if (bContinue) {
		
			_RecordsetPtr rs;
			bool bProcessZips = false;

			// (a.walling 2007-03-19 14:25) - PLID 24245 - Fixing various issues and cleaning up code, also optimizing
			// this (using the DL's FindByColumn function is much, much faster than iterating through every item in the
			// list, esp. when there are thousands of rows!)

			//now we need to lookup everything in the ZipCodesT in practice and see if any of those fit in as well
			//get all the ones that have been modified and add them one by one
			if (m_nSearchType == 5) // All
			{
				rs = CreateRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip, Deleted FROM ZipCodesT WHERE City IS NOT NULL OR State IS NOT NULL OR AreaCode IS NOT NULL");

				bProcessZips = true;
			} else if ( (m_nSearchType >= 1) && (m_nSearchType <= 3) && (!strField.IsEmpty()) && (!strCriteria.IsEmpty())) {
				rs = CreateRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip, Deleted FROM ZipCodesT WHERE %s LIKE '%s%%'", strField, _Q(strCriteria));

				bProcessZips = true;
			} else {
				// don't do anything
				bProcessZips = false;
			}

			if (bProcessZips) {
				while(!rs->eof) {
					FieldsPtr fields = rs->Fields;
					bool bFound = false;
					long id = VarLong(fields->Item["StaticID"]->Value, -1);
					_variant_t varID, varZip, varCity, varState, varArea, varPri, varUnique, varDeleted;
					varID = fields->Item["StaticID"]->Value;
					varUnique = fields->Item["UniqueID"]->Value;
					varZip = fields->Item["ZipCode"]->Value;
					varCity = fields->Item["City"]->Value;
					varState = fields->Item["State"]->Value;
					varArea = fields->Item["AreaCode"]->Value;
					varPri = fields->Item["PrimaryZip"]->Value;
					varDeleted = fields->Item["Deleted"]->Value;

					if(id != -1) {	//row has a static id
						//find the row that matches this one
						long nFoundRow = m_pZipCodes->FindByColumn(4, varID, 0, VARIANT_FALSE);
						if (nFoundRow != sriNoRow) {
							//we need to update this row
							if(VarBool(varDeleted)) {
								//remove this row, it's been deleted
								m_pZipCodes->RemoveRow(nFoundRow);
								bFound = true;
							}
							else {
								//the row is not deleted, carry on
								if(varCity.vt != VT_NULL)
									m_pZipCodes->PutValue(nFoundRow, 1, varCity);
								if(varState.vt != VT_NULL)
									m_pZipCodes->PutValue(nFoundRow, 2, varState);
								if(varArea.vt != VT_NULL)
									m_pZipCodes->PutValue(nFoundRow, 3, varArea);
								if(varPri.vt != VT_NULL) {
									m_pZipCodes->PutValue(nFoundRow, 6, varPri);
									if(VarBool(varPri, FALSE)) {
										//it is the primary, so highlight it
										IRowSettingsPtr pRow;
										pRow = m_pZipCodes->GetRow(nFoundRow);
										pRow->ForeColor = RGB(255,0,0);
										pRow->ForeColorSel = RGB(255,0,0);
									}
								}
								if(varUnique.vt != VT_NULL)
									m_pZipCodes->PutValue(nFoundRow, 5, varUnique);
								bFound = true;
							}
						}
					}

					if(!bFound) {	//item wasn't found in the list
						//this static id isn't in the list, so we have to add it manually

						//IF it's not deleted
						if(!VarBool(varDeleted)) {
							IRowSettingsPtr pRow;
							pRow = m_pZipCodes->GetRow(-1);
							pRow->PutValue(0, varZip);	//zip
							pRow->PutValue(1, varCity);	//city
							pRow->PutValue(2, varState);	//state
							pRow->PutValue(3, varArea);	//area
							pRow->PutValue(4, varID);
							pRow->PutValue(5, varUnique);
							pRow->PutValue(6, varPri);
							if(VarBool(varPri, FALSE)) {
								//it is the primary, so highlight it
								pRow->ForeColor = RGB(255,0,0);
								pRow->ForeColorSel = RGB(255,0,0);
							} else {
								//manually added zip code should show up in blue!
								pRow->ForeColor = RGB(0,0,255);
								pRow->ForeColorSel = RGB(0,0,255);
							}
							//insert into the datalist
							m_pZipCodes->AddRow(pRow);
						}
					}
					bFound = false;

					rs->MoveNext();

				}//end while
			}

//			current request
		} else {
//			current request no data
		}

		CString str;
		str.Format("Loaded %li items.", m_pZipCodes->GetRowCount());

		SetDlgItemText(IDC_ZIPSTATUS, str);
	} NxCatchAllCall("Error in OnZipCodesLoaded()", 
		{ 
			if (bSearchCities) {
				CityZipUtils::ClearCityZipThreadData(pCityLoad); 
				CityZipUtils::ClearCityZipSet(pCs); 
		
			}
			else {
				ZipcodeUtils::ClearZipcodeThreadData(pLoad); 
				ZipcodeUtils::ClearZipcodeSet(pZs); 
			}
		} );
			
	return 0;
}

// (a.walling 2006-12-13 09:57) - PLID 23754 - Return the corresponding database field.
CString CZipcodes::GetDatabaseField(ZipcodeUtils::EZipcodeField ezfField)
{
	switch (ezfField) {
		case ZipcodeUtils::zfZip:
			return "ZipCode";
			break;
		case ZipcodeUtils::zfAreaCode:
			return "AreaCode";
			break;
		case ZipcodeUtils::zfCity:
			return "City";
			break;
		case ZipcodeUtils::zfNone:
		default:
			return "";
			break;
	}
}

CString CZipcodes::GetDatabaseField(CityZipUtils::ECityZipField ezfField)
{
	switch (ezfField) {
		case CityZipUtils::zfZip:
			return "ZipCode";
			break;
		case CityZipUtils::zfAreaCode:
			return "AreaCode";
			break;
		case CityZipUtils::zfCity:
			return "City";
			break;
		case CityZipUtils::zfNone:
		default:
			return "";
			break;
	}
}

// (a.walling 2006-12-06 09:58) - PLID 23754 - Stop the current requery of zip codes.
//	We set the event and post the thread message and wait nWait ms. If that time expires
//  then we log that the thread is being non-responsive. The event handle and all memory
//  is returned in the NXM_ZIPCODES_LOADED message params so we can handle memory cleanup
//  there. If the thread has finished as we call this, and we are closing the window, then
//  we force handling the message so memory is taken care of.
DWORD CZipcodes::StopZipRequery(long nWait /* = 0 */)
{
	if (!m_bDestroying)
		SetDlgItemText(IDC_ZIPSTATUS, "");
	
	if (m_pLoadZipCodesThread == NULL)
		return false;

	if (m_hStopLoadingZips == INVALID_HANDLE_VALUE)
		return false;

#ifdef _DEBUG
	LogDetail("CZipCodes::StopZipRequery(%li) (event %li, thread %li)", nWait, m_hStopLoadingZips, m_pLoadZipCodesThread);
#endif

	SetEvent(m_hStopLoadingZips); // set this event (it will be freed when the handle is returned in the Zip Codes Loaded handler)
	PostThreadMessage(m_pLoadZipCodesThread->m_nThreadID, WM_QUIT, 0, 0);
	DWORD dwResult = WaitForSingleObject(m_pLoadZipCodesThread, nWait);

	m_pLoadZipCodesThread = NULL;
	m_hStopLoadingZips = INVALID_HANDLE_VALUE;

	if (dwResult == WAIT_TIMEOUT)
	{
#ifdef _DEBUG
		LogDetail("CZipCodes::StopZipRequery(%li) Zip thread is unresponsive!", nWait);
#endif
	}
	else if (dwResult == WAIT_OBJECT_0) { // the thread has finished!
		// if we are in the middle of being destroyed, ensure we handle the message at least so we can clean up memory.
		if (m_bDestroying) {
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				if (msg.message == WM_QUIT) return FALSE;
				else AfxGetApp()->PumpMessage();
			}
		}
	}

	return dwResult;
}


// (a.walling 2006-12-06 10:01) - PLID 23754 - Start a new zip query with these params.
//	Call StopZipRequery first. The main duty of this function is to set up the parameter
//	struct for the thread, and to update the GUI with a 'Loading...' status.
void CZipcodes::StartZipRequery(ZipcodeUtils::EZipcodeField ezfField, CString strCriteria)
{
	char *lpszField = NULL, *lpszCriteria = NULL;

	if ((ezfField != ZipcodeUtils::zfNone) && !strCriteria.IsEmpty()) {
		lpszCriteria = new char[strCriteria.GetLength() + 1];	// add one for the null term

		strcpy(lpszCriteria, (LPCTSTR)strCriteria);
	}

	if (m_pLoadZipCodesThread != NULL) {
		StopZipRequery(250); // this will stop the thread wait at most 1/4 of a second for the thread to exit.
		// then we will create a new one anyway, the old one will exit eventually. This is all handled in the Zip Codes Loaded message handler.
		m_pLoadZipCodesThread = NULL;
	}
	
	ZipcodeUtils::ZipcodeLoadingCriteria* pLoad = new ZipcodeUtils::ZipcodeLoadingCriteria;
	memset(pLoad, 0, sizeof(ZipcodeUtils::ZipcodeLoadingCriteria));
	pLoad->pWnd = this;

	// create and reset a stop event for this thread. the handle is returned in the Done message so we can close it.
	m_hStopLoadingZips = CreateEvent(NULL, TRUE, TRUE, NULL);
	ResetEvent(m_hStopLoadingZips);

	pLoad->hStopLoadingEvent = m_hStopLoadingZips;

	pLoad->ezfField = ezfField;
	pLoad->lpszCriteria = lpszCriteria;

	m_pLoadZipCodesThread = AfxBeginThread(ZipcodeUtils::LoadZipcodeAsyncThread, pLoad, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!m_pLoadZipCodesThread)
		return;

#ifdef _DEBUG
	LogDetail("CZipCodes::StartZipRequery(%li, %s) (event %li, thread %li)", (long)ezfField, strCriteria, m_hStopLoadingZips, m_pLoadZipCodesThread);
#endif

	// Execute the thread
	m_pLoadZipCodesThread->m_bAutoDelete = TRUE;
	m_pLoadZipCodesThread->ResumeThread();

	SetDlgItemText(IDC_ZIPSTATUS, "Loading, please wait...");
}

// (j.gruber 2009-10-07 12:28) - PLID 35827 - cheanged for city list
void CZipcodes::StartCityRequery(CityZipUtils::ECityZipField ezfField, CString strCriteria)
{
	char *lpszField = NULL, *lpszCriteria = NULL;

	if ((ezfField != CityZipUtils::zfNone) && !strCriteria.IsEmpty()) {
		lpszCriteria = new char[strCriteria.GetLength() + 1];	// add one for the null term

		strcpy(lpszCriteria, (LPCTSTR)strCriteria);
	}

	if (m_pLoadZipCodesThread != NULL) {
		StopZipRequery(250); // this will stop the thread wait at most 1/4 of a second for the thread to exit.
		// then we will create a new one anyway, the old one will exit eventually. This is all handled in the Zip Codes Loaded message handler.
		m_pLoadZipCodesThread = NULL;
	}
	
	CityZipUtils::CityZipLoadingCriteria* pLoad = new CityZipUtils::CityZipLoadingCriteria;
	memset(pLoad, 0, sizeof(CityZipUtils::CityZipLoadingCriteria));
	pLoad->pWnd = this;

	// create and reset a stop event for this thread. the handle is returned in the Done message so we can close it.
	m_hStopLoadingZips = CreateEvent(NULL, TRUE, TRUE, NULL);
	ResetEvent(m_hStopLoadingZips);

	pLoad->hStopLoadingEvent = m_hStopLoadingZips;

	pLoad->ezfField = ezfField;
	pLoad->lpszCriteria = lpszCriteria;

	m_pLoadZipCodesThread = AfxBeginThread(CityZipUtils::LoadCityZipAsyncThread, pLoad, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!m_pLoadZipCodesThread)
		return;

#ifdef _DEBUG
	LogDetail("CZipCodes::StartCityRequery(%li, %s) (event %li, thread %li)", (long)ezfField, strCriteria, m_hStopLoadingZips, m_pLoadZipCodesThread);
#endif

	// Execute the thread
	m_pLoadZipCodesThread->m_bAutoDelete = TRUE;
	m_pLoadZipCodesThread->ResumeThread();

	SetDlgItemText(IDC_ZIPSTATUS, "Loading, please wait...");
}


// (a.walling 2006-12-06 10:02) - PLID 23754 - I needed this to properly update
//	a label on the dialog.
HBRUSH CZipcodes::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())  {
		case IDC_STATIC:
		case IDC_ZIPSTATUS:
		case IDC_SEARCH_ZIP:
		case IDC_SEARCH_AREA:
		case IDC_SEARCH_CITY:
		case IDC_MODIFIED_BTN:
		case IDC_ALL_ZIPS:
		case IDC_SEARCH_LABEL:
		case IDC_PROMPT:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x008080FF));
			return m_brush;
		break;
		default:
		break;
	}

	return hbr;
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CZipcodes::OnDestroy() 
{
	// (a.walling 2006-12-06 09:39) - PLID 23754 - Clean up anything necessary.

	m_bDestroying = true;
	
	StopZipRequery(0);	// Make sure we stop the thread. Don't wait for it to exit.
		// all memory cleanup is handled within the thread when we cancel it.	

	CNxDialog::OnDestroy();
}
