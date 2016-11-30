// InformLink.cpp : implementation file
//

#include "stdafx.h"
#include "InformLink.h"
#include "MsgBox.h"
#include "ExportDuplicates.h"
#include "GlobalDataUtils.h"
#include "EditPrefixesDlg.h"
#include "PracticeRc.h"
#include "ShowConnectingFeedbackDlg.h"
#include "GlobalDrawingUtils.h"
#include "WellnessDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInformLink dialog

static void SetStatusBarText(HWND hWnd, LPCSTR lpszText)
{
	::PostMessage(hWnd, SB_SETTEXT, (255|0), (LPARAM)lpszText);
}

CInformLink::CInformLink()
	: CNxDialog(CInformLink::IDD, NULL)
{
	//{{AFX_DATA_INIT(CInformLink)
	//}}AFX_DATA_INIT
	m_pThread = NULL;

	//TES 5/24/2013 - PLID 56860 - Initialize variables!  We default IsFullVersion to TRUE, just like CheckInformVersion() does.
	IsFullVersion = TRUE;
	m_bCheckedVersion = FALSE;
}

CInformLink::~CInformLink()
{
	KillThread();
}

void CInformLink::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInformLink)
	DDX_Control(pDX, IDC_LINK_PREFIXES, m_btnLinkPrefixes);
	DDX_Control(pDX, IDC_LINK, m_link);
	DDX_Control(pDX, IDOK, m_Close);
	DDX_Control(pDX, IDC_UNLINK, m_unlink);
	DDX_Control(pDX, IDC_GET_INFORM_PATH, m_getInformPath);
	DDX_Control(pDX, IDC_EXPORT, m_SendToInform);
	DDX_Control(pDX, IDC_IMPORT, m_SendToPractice);
	DDX_Control(pDX, IDC_INF_REM_ALL, m_infRemoveAll);
	DDX_Control(pDX, IDC_INF_REM_ONE, m_infRemove);
	DDX_Control(pDX, IDC_INF_ADD_ONE, m_infAdd);
	DDX_Control(pDX, IDC_PRAC_REM_ALL, m_pracRemoveAll);
	DDX_Control(pDX, IDC_PRAC_REM_ONE, m_pracRemove);
	DDX_Control(pDX, IDC_PRAC_ADD_ONE, m_pracAdd);
	DDX_Control(pDX, IDC_LABEL2, m_nxstaticLabel2);
	DDX_Control(pDX, IDC_PRAC_COUNT, m_nxstaticPracCount);
	DDX_Control(pDX, IDC_LABEL8, m_nxstaticLabel8);
	DDX_Control(pDX, IDC_PRAC_SELECT_COUNT, m_nxstaticPracSelectCount);
	DDX_Control(pDX, IDC_LABEL7, m_nxstaticLabel7);
	DDX_Control(pDX, IDC_INF_COUNT, m_nxstaticInfCount);
	DDX_Control(pDX, IDC_LABEL1, m_nxstaticLabel1);
	DDX_Control(pDX, IDC_INF_SELECT_COUNT, m_nxstaticInfSelectCount);
	DDX_Control(pDX, IDC_LABEL3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_SHOW_ADVANCED, m_btnShowAdvanced);
	DDX_Control(pDX, IDC_NEW_PAT_CHECK, m_btnNewPat);
	DDX_Control(pDX, IDC_CHECK_DISABLEINFORM, m_btnDisable);
	//}}AFX_DATA_MAP
}

BEGIN_EVENTSINK_MAP(CInformLink, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInformLink)
	ON_EVENT(CInformLink, IDC_PRAC_PATIENTS, 18 /* RequeryFinished */, OnRequeryFinishedPracPatients, VTS_I2)
	ON_EVENT(CInformLink, IDC_INFORM_PATIENTS, 18 /* RequeryFinished */, OnRequeryFinishedInformPatients, VTS_I2)
	ON_EVENT(CInformLink, IDC_INFORM_PATIENTS, 3 /* DblClickCell */, OnDblClickCellInformPatients, VTS_I4 VTS_I2)
	ON_EVENT(CInformLink, IDC_PRAC_PATIENTS, 3 /* DblClickCell */, OnDblClickCellPracPatients, VTS_I4 VTS_I2)
	ON_EVENT(CInformLink, IDC_PRAC_PATIENTS_SELECTED, 3 /* DblClickCell */, OnDblClickCellPracPatientsSelected, VTS_I4 VTS_I2)
	ON_EVENT(CInformLink, IDC_INFORM_PATIENTS_SELECTED, 3 /* DblClickCell */, OnDblClickCellInformPatientsSelected, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CInformLink, CNxDialog)
	//{{AFX_MSG_MAP(CInformLink)
	ON_BN_CLICKED(IDC_INF_ADD_ONE, OnInfAdd)
	ON_BN_CLICKED(IDC_INF_REM_ALL, OnInfRemAll)
	ON_BN_CLICKED(IDC_INF_REM_ONE, OnInfRem)
	ON_BN_CLICKED(IDC_PRAC_REM_ALL, OnPracRemAll)
	ON_BN_CLICKED(IDC_PRAC_ADD_ONE, OnPracAdd)
	ON_BN_CLICKED(IDC_PRAC_REM_ONE, OnPracRem)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_GET_INFORM_PATH, GetInformPath)
	ON_BN_CLICKED(IDC_SHOW_ADVANCED, OnShowAdvanced)
	ON_BN_CLICKED(IDC_LINK, OnLink)
	ON_BN_CLICKED(IDC_UNLINK, OnUnlink)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_NEW_PAT_CHECK, OnNewPatCheck)
	ON_BN_CLICKED(IDC_CHECK_DISABLEINFORM, OnDisableLinkCheck)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_LINK_PREFIXES, OnLinkPrefixes)
	ON_MESSAGE(NXM_ENSURE_INFORM_DATA, OnEnsureData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInformLink message handlers
BOOL CInformLink::OnInitDialog() 
{
	CRect rcClient;

	m_pracBrush.CreateSolidBrush(PaletteColor(0x00D79D8C));
	m_informBrush.CreateSolidBrush(PaletteColor(0x009CDA9C));

	GetClientRect(rcClient);
	m_statusBar.Create(WS_VISIBLE|WS_CHILD|CBRS_BOTTOM, rcClient, (this), AFX_IDW_STATUS_BAR);
	m_statusBar.SetSimple();
	m_statusBar.SetText("Loading...", 255, 0);

	CNxDialog::OnInitDialog();

	m_informPath = GetRemotePropertyText ("InformDataPath", "", 0, "<None>");
	((CButton*)GetDlgItem(IDC_NEW_PAT_CHECK))->SetCheck(GetPropertyInt("NewPatExportToInform"));
	((CButton*)GetDlgItem(IDC_CHECK_DISABLEINFORM))->SetCheck(GetPropertyInt("InformDisable", 0, 0, false));
	CString	str;

	GetDlgItem(IDC_LINK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_UNLINK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LINK_PREFIXES)->ShowWindow(SW_HIDE);

	m_InformRequeryDone = FALSE;
	m_PracticeRequeryDone = FALSE;

	m_pracAdd.AutoSet(NXB_RIGHT);
	m_pracRemove.AutoSet(NXB_LEFT);
	m_pracRemoveAll.AutoSet(NXB_LLEFT);
	m_infAdd.AutoSet(NXB_RIGHT);
	m_infRemove.AutoSet(NXB_LEFT);
	m_infRemoveAll.AutoSet(NXB_LLEFT);
	m_SendToPractice.AutoSet(NXB_UP);
	m_SendToInform.AutoSet(NXB_EXPORT);
	m_Close.AutoSet(NXB_CLOSE);

	m_informList = GetDlgItem(IDC_INFORM_PATIENTS)->GetControlUnknown();

	if (GetPropertyInt("InformDisable", 0, 0, false))
	{
		SetDlgItemText (IDC_INF_COUNT, "");
		SetDlgItemText (IDC_PRAC_COUNT, "");
		m_statusBar.SetText("Link to Inform is disabled", 255, 0);
		MsgBox("The Link to Inform is disabled on this computer. To re-enable it, click on \"Advanced Options\", then uncheck \"Disable the Link.\"");
	}
	else
	{
		SetDlgItemText (IDC_INF_COUNT, "Loading...");
		SetDlgItemText (IDC_PRAC_COUNT, "Loading...");
	}

	// (c.haag 2003-07-15 11:18) - We have to make the case of needing to browse the
	// data behave consistently with all the links.
/*	if(m_informPath=="" || !DoesExist(m_informPath)) {
		MsgBox("The Inform Link has not been properly set.\n"
			"On the following screen, please locate your Consent.mdb file.");
		GetInformPath();
	}
	else {
		try {

			//open a connection for recordsets we will use
			try {
				informDB->Open(_bstr_t(m_informPath),"","",NULL);
			} catch(_com_error e) {
				//if an error occurred, the database is invalid. Don't crash, just fix it!
				MsgBox("The Inform Link has not been properly set.\n"
					"On the following screen, please locate your Consent.mdb file.");
				GetInformPath();
			}

			//if the database is invalid, try for a new one
			if(!CheckInformVersion())
				GetInformPath();
			else {
				//if valid, connect the inform datalist, if not, GetInformPath will
				str.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;User Id=;Password=;",m_informPath);
				m_informList->PutConnectionString(_bstr_t(str));
				m_informList->Requery();
			}

		}NxCatchAll("Error, Could not set inform path: ");
	}*/

	if (!GetPropertyInt("InformDisable", 0, 0, false))
	{
		//TES 12/24/03: Instead of calling EnsureData here, wait until the window is on the screen, because otherwise 
		//the creation of the "Connecting..." dialog will occasionally cause some other window to become active all of 
		//a sudden if the Inform dialog is not yet visible.
		PostMessage(NXM_ENSURE_INFORM_DATA);
		/*if (!EnsureData())
		{
			// The user cancelled the link
			PostMessage(WM_COMMAND, IDCANCEL);
			return TRUE;
		}*/
	}
	
	m_getInformPath.SetWindowText(m_informPath); 
	
	m_practiceList = BindNxDataListCtrl(IDC_PRAC_PATIENTS,false);
	m_practiceSelected = BindNxDataListCtrl(IDC_PRAC_PATIENTS_SELECTED,false);
	m_informSelected = BindNxDataListCtrl(IDC_INFORM_PATIENTS_SELECTED,false);

	if (!GetPropertyInt("InformDisable", 0, 0, false))
		m_practiceList->Requery();

	m_cross.informList = m_informList;
	m_cross.practiceList = m_practiceList;
	m_cross.statusBar = m_statusBar.GetSafeHwnd();

	SetDlgItemText (IDC_PRAC_SELECT_COUNT, "0");
	SetDlgItemText (IDC_INF_SELECT_COUNT, "0");

	return TRUE;
}

BOOL CInformLink::EnsureData()
{
	BOOL bCancel = FALSE;
	BOOL bSuccess = FALSE;
	CString str;

	do {		
		try {

			CShowConnectingFeedbackDlg dlgConnecting;
			dlgConnecting.SetWaitMessage("Please wait while Practice connects to your Inform database...");

			//open a connection for recordsets we will use
			informDB.CreateInstance(__uuidof(Connection));
			informDB->Provider = "Microsoft.Jet.OLEDB.4.0";

			if (informDB->GetState() == adStateOpen)
				informDB->Close();
			informDB->Open(_bstr_t(m_informPath),"","",NULL);
			str.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;User Id=;Password=;",m_informPath);
			m_informList->PutConnectionString(_bstr_t(str));
			m_informList->Requery();
			bSuccess = TRUE;
		} catch(_com_error e) {
			

			if (e.Error() == 0x80004005) {
				if (IDYES == MsgBox(MB_YESNO, "Practice could not access your Inform data. You may be improperly connected to your Inform server on your Windows network. Would you like to try connecting again?")) {
					if (NO_ERROR == TryAccessNetworkFile(m_informPath))
						continue;
				}
			}

			//if an error occurred, the database is invalid. Don't crash, just fix it!
			MsgBox("The Inform Link has not been properly set.\n"
				"On the following screen, please locate your Consent.mdb file.");
			GetInformPath();
			if (m_informPath=="")
				bCancel = TRUE;
		}
	}
	while (!bCancel && !bSuccess);
	return bSuccess;
}

void CInformLink::GetInformPath() 
{
	CString sql;
	CFileDialog Browse(TRUE);

	try {

		BOOL bPathIsValid = FALSE;

		//do not set the path until it is a valid inform database
		while(!bPathIsValid) {		

			m_getInformPath.GetWindowText(m_informPath);

			//TES 2003-12-30: Since we would only call this function if our inform path is not valid, why are we starting
			//our browsing in the one folder we know it couldn't possibly be in?
			//Browse.m_ofn.lpstrInitialDir = m_informPath;
			Browse.m_ofn.lpstrFilter = "Inform Database Files\0*.mde;*.mdb\0All Files\0*.*\0\0";
			if (Browse.DoModal() == IDCANCEL) 
			{
				m_informPath = "";				
				return;
			}
			m_informPath = Browse.GetPathName();

			if (NO_ERROR != TryAccessNetworkFile(m_informPath))
			{
				m_informPath = "";				
				return;
			}

			//make a connection for recordsets we will use
			if(informDB->State != adStateClosed)
				informDB->Close();

			try {
				informDB->Open(_bstr_t(m_informPath),"","",NULL);
			}catch(_com_error e) {
				//if an error occurred, the database is invalid. Don't crash, just fix it!
				MessageBox("The database you selected is not a valid Inform database.","Practice",MB_OK|MB_ICONEXCLAMATION);
				bPathIsValid = FALSE;
				continue;
			}

			bPathIsValid = CheckInformVersion();

			if(!bPathIsValid)
				informDB->Close();
		}

		//don't save the path until you have a valid one!
		SetRemotePropertyText ("InformDataPath", m_informPath, 0, "<None>");

		//connect the inform datalist
		sql.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;User Id=;Password=;",m_informPath);
		m_informList->PutConnectionString(_bstr_t(sql));
		m_InformRequeryDone = FALSE;
		m_informList->Requery();				

		m_getInformPath.SetWindowText(m_informPath);
			
	}NxCatchAll("Error in GetInformPath()");
}

void CInformLink::OnInfAdd() 
{
	CWaitCursor pWait;

	CString		str;
	_variant_t var;

	if (m_informList->GetCurSel()!=-1)
		m_informSelected->TakeCurrentRow(m_informList);
	str.Format ("%li", m_informSelected->GetRowCount());
	SetDlgItemText (IDC_INF_SELECT_COUNT, str);
}

void CInformLink::OnInfRemAll() 
{
	CWaitCursor pWait;

	m_informList->TakeAllRows(m_informSelected);
	SetDlgItemText (IDC_INF_SELECT_COUNT, "0");
}

void CInformLink::OnInfRem() 
{
	CWaitCursor pWait;
	CString str;

	if (m_informSelected->GetCurSel()!=-1)
		m_informList->TakeCurrentRow(m_informSelected);

	str.Format ("%li", m_informSelected->GetRowCount());
	SetDlgItemText (IDC_INF_SELECT_COUNT, str);
}

void CInformLink::OnPracRemAll() 
{
	CWaitCursor pWait;
	m_practiceList->TakeAllRows(m_practiceSelected);
	SetDlgItemText (IDC_PRAC_SELECT_COUNT, "0");
}

void CInformLink::OnPracAdd() 
{
	CWaitCursor pWait;
	CString str;
	_variant_t var;

	if (m_practiceList->GetCurSel()!=-1)
		m_practiceSelected->TakeCurrentRow(m_practiceList);

	str.Format ("%li", m_practiceSelected->GetRowCount());
	SetDlgItemText (IDC_PRAC_SELECT_COUNT, str);
}

void CInformLink::OnPracRem() 
{
	CWaitCursor pWait;
	CString str;

	if (m_practiceSelected->GetCurSel()!=-1)
		m_practiceList->TakeCurrentRow(m_practiceSelected);

	str.Format ("%li", m_practiceSelected->GetRowCount());
	SetDlgItemText (IDC_PRAC_SELECT_COUNT, str);	
}

BOOL CInformLink::TryExport(long practiceID, CString &informPath, long &ChartNum, long &newID, BOOL &bStop)
{
	long	informCN,
			PracUserDefinedID,
			RefID = 0,
			RefDetailID = 0;
	CString sql,
			ssn,
			first, 
			last,
			middle;
	_RecordsetPtr rsID(__uuidof(Recordset)),rsPractice;
	_variant_t var;

	//TES 5/24/2013 - PLID 56860 - Make sure we've checked the version of Inform
	if(!m_bCheckedVersion) {
		CheckInformVersion();
	}
//try {
	//Make sure we have a name
	rsID = CreateRecordset("SELECT InformID, [First], [Last], [Middle], [SocialSecurity], UserDefinedID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = %li", practiceID);	
	var = rsID->Fields->Item["First"]->Value;
	if (var.vt == VT_BSTR)
		first = CString(var.bstrVal);
	var = rsID->Fields->Item["Last"]->Value;
	if (var.vt == VT_BSTR)
		last = CString(var.bstrVal);
	var = rsID->Fields->Item["Middle"]->Value;
	if (var.vt == VT_BSTR)
		middle = CString(var.bstrVal);
	if (IsFullVersion && rsID->Fields->Item["SocialSecurity"]->Value.vt == VT_BSTR)
		ssn	= CString(rsID->Fields->Item["SocialSecurity"]->Value.bstrVal);
	PracUserDefinedID = AdoFldLong(rsID, "UserDefinedID");
	var = rsID->Fields->Item["InformID"]->Value;
	if (var.vt == VT_I4)
		informCN = var.lVal;
	rsID->Close();

	if (first == "" || last == "") {
		AfxMessageBox("Inform requires both a first and last name, could not export patient \"" + first + " " + last + ".\"");
		bStop = FALSE;
		return FALSE;
	}

	ssn.TrimLeft(" ");
	ssn.TrimRight(" ");
	ssn.Replace("-","");
	ssn.Replace("#","");

	if(informCN>0) {
		sql.Format("SELECT ID FROM tblPatient WHERE ID = %li", informCN);
		rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		if(!rsID->eof) {
			UpdateInform(practiceID);
			//return FALSE so we don't add the row
			bStop = FALSE;
			return FALSE;
		}
		rsID->Close();
	}

	CExportDuplicates dlg(this);
	if(dlg.FindDuplicates(first,last,middle,"Practice","Inform",informDB,m_informPath)) {
		int choice = dlg.DoModal();
		switch(choice) {
		case(1):
			//add new
			//do nothing different
			break;
		case(2):
			//update
			ExecuteSql("UPDATE PatientsT SET InformID = %li WHERE PersonID = %li",dlg.m_varIDToUpdate.lVal,practiceID);
			UpdateInform(practiceID);
			newID = dlg.m_varIDToUpdate.lVal;
			//do this to signify update - saves in variable names, and will be dropped when I redo this
			bStop = TRUE;
			return TRUE;

			/*
			//return FALSE so we don't add the row
			return FALSE;
			*/
			break;
		case(3):
			//skip
			bStop = FALSE;
			return FALSE;
			break;
		default:
			//stop
			bStop = TRUE;
			return FALSE;
			break;
		}
	}

	//Make sure SSN is unique or blank
	if (ssn != "" && IsFullVersion) {
		sql.Format("SELECT SSN FROM tblPatient WHERE SSN = '%s'", ssn);
		rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		if (!rsID->eof)
		{	AfxMessageBox("A patient already entered into inform has the Social Security Number " + ssn + ",\ncould not export patient \"" + first + " " + last + ".\"");
			rsID->Close();
			bStop = FALSE;
			return FALSE;
		}
		rsID->Close();
	}

	//This gets a new Inform patient ID
	sql.Format("SELECT ChartNumber FROM tblPatient WHERE ChartNumber = '%li'", PracUserDefinedID);
	rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
	if (!rsID->eof)
	{	rsID->Close();
		//(e.lally 2009-10-09) PLID 35899 - Ignore numeric chart numbers that are larger than Max Long (2147483647)
		sql.Format("SELECT Max(CLng(IIf(ChartNumber Is Null,0,IIf(IsNumeric(ChartNumber),IIf(CDbl(ChartNumber) <= 2147483647, ChartNumber, 0),0)))) AS MaxChartNum FROM tblPatient");
		rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		informCN = rsID->Fields->Item["MaxChartNum"]->Value.lVal + 1;
	}
	else informCN = PracUserDefinedID;
	rsID->Close();

	//////////////////////////////////////////////////////////////
	//REFERRAL SOURCE

	long PracRefParent = -1;
	CString Refsource = "";

	//first check to see if there even IS a referral source, to bypass this whole chunk of code
	if(IsFullVersion && !IsRecordsetEmpty("SELECT PersonID FROM PatientsT WHERE ReferralID > 0 AND PatientsT.PersonID = %li",practiceID)) {
		
		//okay, now look and see if the referral source is a detailed one
		sql.Format("SELECT ReferralSourceT.Name FROM PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID WHERE ReferralID > 0 AND ReferralSourceT.Name Is Not Null AND PatientsT.PersonID = %li",practiceID);
		rsID = CreateRecordsetStd(sql);
		if (!rsID->eof) {
			Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
			rsID->Close();
			sql.Format("SELECT ID, RefSourceID, Name FROM tblSourceDetOther WHERE Name = '%s'",_Q(Refsource));
			rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
			if(!rsID->eof) {
				RefDetailID = rsID->Fields->Item["ID"]->Value.lVal;
				RefID = rsID->Fields->Item["RefSourceID"]->Value.lVal;
			}
		}
		rsID->Close();

		//if that try was unsuccessful, try looking at parent sources
		if(RefID==0) {
			sql.Format("SELECT ReferralSourceT.Name, ReferralSourceT.Parent FROM PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID WHERE ReferralID > 0 AND ReferralSourceT.Name Is Not Null AND PatientsT.PersonID = %li",practiceID);
			rsID = CreateRecordsetStd(sql);
			if (!rsID->eof) {
				Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
				//get the parent incase we have to go to a third iteration
				PracRefParent = rsID->Fields->Item["Parent"]->Value.lVal;
				rsID->Close();
				sql.Format("SELECT ID, Label FROM tblRefSource WHERE Label = '%s'",_Q(Refsource));
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
				if(!rsID->eof) {
					RefID = rsID->Fields->Item["ID"]->Value.lVal;
				}		
			}
			rsID->Close();
		}

		//and if THAT try was unsuccessful it's time to be sneaky. We could give up now, but that's not the NexTech way. haHA!
		//now check to see if the parent of the selected referral source is itself a detailed or regular referral source
		if(RefID==0 && PracRefParent != -1) {
			//first check to see if the referral source is a detailed one
			sql.Format("SELECT Name FROM ReferralSourceT WHERE PersonID = %li",PracRefParent);
			rsID = CreateRecordsetStd(sql);
			if (!rsID->eof) {
				Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
				rsID->Close();
				sql.Format("SELECT ID, RefSourceID, Name FROM tblSourceDetOther WHERE Name = '%s'",_Q(Refsource));
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
				if(!rsID->eof) {
					RefDetailID = rsID->Fields->Item["ID"]->Value.lVal;
					RefID = rsID->Fields->Item["RefSourceID"]->Value.lVal;
				}		
			}
			rsID->Close();
			
			if(RefID==0) {
				sql.Format("SELECT Name FROM ReferralSourceT WHERE PersonID = %li",PracRefParent);
				rsID = CreateRecordsetStd(sql);
				if (!rsID->eof) {
					Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
					rsID->Close();
					sql.Format("SELECT ID, Label FROM tblRefSource WHERE Label = '%s'",_Q(Refsource));
					rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
					if(!rsID->eof) {
						RefID = rsID->Fields->Item["ID"]->Value.lVal;
					}
				}
				rsID->Close();
			}				
		}
	}

	//////////////////////////////////////////////////////////////

	if(IsFullVersion) {

		//This gets the values
		rsPractice = CreateRecordset("SELECT Archived,(CASE WHEN PrefixT.InformID Is Null THEN 0 ELSE PrefixT.InformID END) AS SalutationID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, "
			"Extension, CellPhone, Email, (CASE WHEN(PersonT.[Gender]=1) THEN 'M' WHEN(PersonT.[Gender]=2) THEN 'F' ELSE '' END) AS MF, "
			"Fax, BirthDate, SocialSecurity, FirstContactDate, Nickname FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID WHERE PersonT.ID = %li", practiceID);
		//safer than doing in the sql format

		long Archived,SalutationID;
		CString First,Last,Middle,Nickname,SocialSecurity,BirthDate,FirstContactDate,Address1,Address2,
			City,State,Zip,HomePhone,WorkPhone,Email,CellPhone,Extension,Fax,NickName,Gender;
		COleDateTime birthdate,firstdate,inputdate;
		
		//this is SO much safer then doing all the rs-> calls inside CreateRecordset
		var = rsPractice->Fields->Item["Archived"]->Value;
		if(var.vt==VT_NULL)
			Archived = 0;
		else
			Archived = var.boolVal;

		Gender = CString(rsPractice->Fields->Item["MF"]->Value.bstrVal);

		var = rsPractice->Fields->Item["SalutationID"]->Value;
		if(var.vt == VT_NULL)
			SalutationID = 0;
		else
			SalutationID = var.lVal;

		var = rsPractice->Fields->Item["First"]->Value;
		if(var.vt==VT_NULL)
			First = "";
		else
			First = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["Last"]->Value;
		if(var.vt==VT_NULL)
			Last = "";
		else
			Last = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["Middle"]->Value;
		if(var.vt==VT_NULL)
			Middle = "";
		else
			Middle = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["Nickname"]->Value;
		if(var.vt==VT_NULL)
			Nickname = "";
		else
			Nickname = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["SocialSecurity"]->Value;
		if(var.vt==VT_NULL)
			SocialSecurity = "NULL";
		else {
			SocialSecurity = CString(var.bstrVal);
			SocialSecurity.Replace(" ","");
			SocialSecurity.Replace("-","");
			SocialSecurity.Replace("#","");
			SocialSecurity = "'" + SocialSecurity + "'";
		}
		if(SocialSecurity=="''")
			SocialSecurity="NULL";

		var = rsPractice->Fields->Item["Address1"]->Value;
		if(var.vt==VT_NULL)
			Address1 = "";
		else
			Address1 = _Q(CString(var.bstrVal));
		if(Address1.GetLength()>30) {
			AfxMessageBox("Inform only supports an Address 1 field that is 30 characters in length.\n"
				"You must edit the account of patient '" + Last + ", " + First + "' before exporting.");
			rsPractice->Close();
			bStop = FALSE;
			return FALSE;
		}

		var = rsPractice->Fields->Item["Address2"]->Value;
		if(var.vt==VT_NULL)
			Address2 = "";
		else
			Address2.Format("'%s'",  _Q(CString(var.bstrVal)));
		if (Address2.IsEmpty() || Address2 == "" || Address2 == "''") {
			Address2 = "NULL";
		}
		if(Address2.GetLength()>50) {
			AfxMessageBox("Inform only supports an Address 2 field that is 50 characters in length.\n"
				"You must edit the account of patient '" + Last + ", " + First + " before exporting.");
			rsPractice->Close();
			bStop = FALSE;
			return FALSE;
		}

		var = rsPractice->Fields->Item["City"]->Value;
		if(var.vt==VT_NULL)
			City = "";
		else
			City = _Q(CString(var.bstrVal));
		if(City.GetLength()>30) {
			AfxMessageBox("Inform only supports a City field that is 30 characters in length.\n"
				"You must edit the account of patient '" + Last + ", " + First + " before exporting.");
			rsPractice->Close();
			bStop = FALSE;
			return FALSE;
		}

		var = rsPractice->Fields->Item["State"]->Value;
		if(var.vt==VT_NULL)
			State = "";
		else
			State = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["Zip"]->Value;
		if(var.vt==VT_NULL)
			Zip = "";
		else
			Zip = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["BirthDate"]->Value;
		if(var.vt==VT_NULL)
			BirthDate = "NULL";
		else {
			birthdate = var.date;
			BirthDate = birthdate.Format("'%m/%d/%Y'");
		}

		var = rsPractice->Fields->Item["FirstContactDate"]->Value;
		if(var.vt==VT_NULL)
			FirstContactDate = "NULL";
		else {
			firstdate = var.date;
			FirstContactDate = firstdate.Format("'%m/%d/%Y'");
		}

		var = rsPractice->Fields->Item["HomePhone"]->Value;
		if(var.vt==VT_NULL)
			HomePhone = "";
		else
			HomePhone = CString(var.bstrVal);

		var = rsPractice->Fields->Item["WorkPhone"]->Value;
		if(var.vt==VT_NULL)
			WorkPhone = "";
		else
			WorkPhone = CString(var.bstrVal);

		var = rsPractice->Fields->Item["Extension"]->Value;
		if(var.vt==VT_NULL)
			Extension = "";
		else {
			Extension = CString(var.bstrVal);
			Extension.Replace("#","");
		}
		if(Extension.GetLength()>6) {
			AfxMessageBox("Inform only supports a phone Extension field that is 6 characters in length.\n"
				"You must edit the account of patient '" + Last + ", " + First + " before exporting.");
			rsPractice->Close();
			bStop = FALSE;
			return FALSE;
		}

		var = rsPractice->Fields->Item["Fax"]->Value;
		if(var.vt==VT_NULL)
			Fax = "";
		else
			Fax = CString(var.bstrVal);

		var = rsPractice->Fields->Item["EMail"]->Value;
		if(var.vt==VT_NULL)
			Email = "";
		else
			Email = _Q(CString(var.bstrVal));

		var = rsPractice->Fields->Item["CellPhone"]->Value;
		if(var.vt==VT_NULL)
			CellPhone = "";
		else
			CellPhone = CString(var.bstrVal);

		rsPractice->Close();

		BOOL bIgnoreSourceDetailInfo = FALSE;

		//JJ - 1-11-2002 - Dr. Salzman's office had InformData missing the SourceDetailID and CellPhone
		//and the version number was no different from other clients. So assume that if one is missing,
		//the other one is.

		try
		{
			informDB->Execute("SELECT TOP 1 SourceDetailID FROM tblPatient", NULL, adCmdText);
		}
		catch(_com_error)
		{
			bIgnoreSourceDetailInfo = TRUE;
		}

		if(!bIgnoreSourceDetailInfo)
			sql.Format("INSERT INTO tblPatient (ChartNumber, RefSourceID, SourceDetailID, SalutationID, Inactive, [First], Middle, [Last], Address1, Address2, City, State, "
				"Zip, NightPhone, DayPhone, DayExt, CellPhone, EMail, [M/F], Fax, "
				"BirthDate, SSN, DateOfContact, Salutation) VALUES "
				"(%li,%li,%li,%li,%li,'%s','%s','%s','%s',%s,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s', %s, %s, %s,'%s')",
				informCN,RefID,RefDetailID,SalutationID,Archived,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,CellPhone,
				Email,Gender,Fax,BirthDate,SocialSecurity,FirstContactDate,Nickname);
		else
			sql.Format("INSERT INTO tblPatient (ChartNumber, RefSourceID, SalutationID, Inactive, [First], Middle, [Last], Address1, Address2, City, State, "
				"Zip, NightPhone, DayPhone, DayExt, [M/F], Fax, "
				"BirthDate, SSN, DateOfContact, Salutation) VALUES "
				"(%li,%li,%li,%li,'%s','%s','%s','%s',%s,'%s','%s','%s','%s','%s','%s','%s','%s', %s, %s, %s,'%s')",
				informCN,RefID,SalutationID,Archived,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,
				Gender,Fax,BirthDate,SocialSecurity,FirstContactDate,Nickname);

		informDB->Execute(_bstr_t(sql),NULL,adCmdText);
	}
	else {
		CString FirstContactDate;
		COleDateTime firstdate;
		rsPractice = CreateRecordset("SELECT FirstContactDate FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE ID = %li", practiceID);
		var = rsPractice->Fields->Item["FirstContactDate"]->Value;
		if(var.vt==VT_NULL)
			FirstContactDate = "NULL";
		else {
			firstdate = var.date;
			FirstContactDate = firstdate.Format("'%m/%d/%Y'");
		}
		rsPractice->Close();
		sql.Format("INSERT INTO tblPatient (ChartNumber, [Last], [First], Middle, DateOfContact) VALUES (%li,'%s','%s','%s', %s)",
			informCN,_Q(last),_Q(first),_Q(middle),FirstContactDate);
		informDB->Execute(_bstr_t(sql),NULL,adCmdText);
	}
		
	long newInformID;
	sql.Format("SELECT ID FROM tblPatient WHERE ChartNumber = '%li'", informCN);
	rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
	if(!rsID->eof) {
		newInformID = rsID->Fields->Item["ID"]->Value.lVal;
		ExecuteSql("UPDATE PatientsT SET InformID = %li WHERE PersonID = %li;",newInformID, practiceID);
	}
	rsID->Close();

	ChartNum = informCN;
	newID = newInformID;

	bStop = FALSE;
	return TRUE;

	//}NxCatchAll("Error in Link To Inform");
}

void CInformLink::OnExport() 
{
	CWaitCursor pWait;

	CString str;
	long practiceID, ChartNum, NewID;
	if(m_informPath=="" || !DoesExist(m_informPath))
		m_informPath = GetRemotePropertyText("InformDataPath", "", 0, "<None>");

	if (!m_practiceSelected->GetRowCount() || !UserPermission(ExportInform))
		return;
	try
	{	while(m_practiceSelected->GetRowCount()>0)
		{	practiceID = m_practiceSelected->GetValue(0,2).lVal;
			BOOL bStop = FALSE;
			if(TryExport(practiceID, m_informPath, ChartNum, NewID, bStop)) {
				//add row to the inform list
				IRowSettingsPtr pPracRow, pInformRow;
				long row = m_practiceSelected->FindByColumn(2,practiceID,0,FALSE);
				if(row!=-1) {
					m_practiceSelected->SetRedraw(FALSE);
					pPracRow = m_practiceSelected->GetRow(row);
					//color Practice row
					pPracRow->PutBackColor(0x00DAFCD1);

					//the combination of bStop being true and TryExport returning true only means we are updating the color
					if(!bStop)
						row = m_informList->AddRow(pPracRow);
					else {
						row = m_informList->FindByColumn(2,NewID,0,FALSE);
					}

					//get it again, otherwise we update both sides
					pInformRow = m_informList->GetRow(row);

					//color Inform row
					pInformRow = m_informList->GetRow(row);
					pInformRow->PutBackColor(0x00FCDAD1);	

					if(!bStop) {
						//update the row with correct info
						pInformRow->PutValue(2,(long)NewID);
						CString str;
						str.Format("%li",ChartNum);
						// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks
						pInformRow->PutValue(3,(LPCTSTR)str);
						
						pPracRow->PutValue(4,(long)NewID);
					}
						
					m_practiceSelected->SetRedraw(TRUE);
				}
			}
			else {
				if(bStop)
					break;
			}
			m_practiceList->TakeRow(m_practiceSelected->GetRow(0));
		}
	}NxCatchAll("Error in Exporting to Inform");

	str.Format ("%li", m_practiceSelected->GetRowCount());
	SetDlgItemText (IDC_PRAC_SELECT_COUNT, str);

	str.Format("%li",m_informList->GetRowCount());
	SetDlgItemText(IDC_INF_COUNT, str);
}

void CInformLink::OnImport() 
{
	CWaitCursor pWait;

	long	informID, 
			practiceID,
			pracuserID,
			RefID = 0;
	CString sql, str, ssn, first, last, middle;
	_RecordsetPtr rsID(__uuidof(Recordset));
	_variant_t var;

	//TES 5/24/2013 - PLID 56860 - Make sure we've checked the version of Inform
	if(!m_bCheckedVersion) {
		CheckInformVersion();
	}

	if (!m_informSelected->GetRowCount() || !UserPermission(ImportInform))
		return;
	try {

		// (j.jones 2010-01-14 16:39) - PLID 31927 - check the default text message privacy field
		long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);
		
		while(m_informSelected->GetRowCount()>0)
		{	informID = m_informSelected->GetValue(0,2).lVal;
	
			//This gets a new Nextech patient ID
			if(IsFullVersion)
				sql.Format("SELECT ChartNumber, First, Last, Middle, SSN FROM tblPatient WHERE ID = %li", informID);// Get Practice ID HereIN 'C:\Documents and Settings\j.jones\Desktop\ConsentLT.mdb'
			else
				sql.Format("SELECT ChartNumber, First, Last, Middle FROM tblPatient WHERE ID = %li", informID);// Get Practice ID HereIN 'C:\Documents and Settings\j.jones\Desktop\ConsentLT.mdb'
			rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
			if(!rsID->eof) {
				var = rsID->Fields->Item["ChartNumber"]->Value;
				//var.ChangeType(VT_I4); didn't work
				if (var.vt == VT_BSTR)
					practiceID = atoi(CString(var.bstrVal));
				else if (var.vt == VT_I4)
					practiceID = var.lVal;
				else if (var.vt ==VT_NULL || var.vt == VT_EMPTY)
					practiceID = -1;
				else
				{	var.ChangeType(VT_I4); //try to convert to an int from whatever, throw exception if can't
					practiceID = var.lVal;
				}
				if(IsFullVersion) {
					var = rsID->Fields->Item["SSN"]->Value;
					if(var.vt==VT_BSTR)
						ssn = CString(var.bstrVal);
				}
				var = rsID->Fields->Item["First"]->Value;
				if(var.vt==VT_BSTR)
					first = _Q(CString(var.bstrVal));
				var = rsID->Fields->Item["Last"]->Value;
				if(var.vt==VT_BSTR)
					last = _Q(CString(var.bstrVal));
				var = rsID->Fields->Item["Middle"]->Value;
				if(var.vt==VT_BSTR)
					middle = _Q(CString(var.bstrVal));
			}
			else {
				//this should never happen, but let's not end the import
				m_informList->TakeRow(m_informSelected->GetRow(0));
				continue;
			}
			rsID->Close();

			CExportDuplicates dlg(this);
			if(dlg.FindDuplicates(first,last,middle,"Inform","Practice",informDB,m_informPath,FALSE)) {
				int choice = dlg.DoModal();
				switch(choice) {
				case(1):
					//add new
					//do nothing different
					break;
				case(3):
					//skip
					m_informList->TakeRow(m_informSelected->GetRow(0));
					continue;
					break;
				default:
					//stop
					m_informList->TakeAllRows(m_informSelected);
					continue;
					break;
				}
			}

			/*
			if(!IsRecordsetEmpty("SELECT ID FROM PersonT WHERE SocialSecurity = '%s' AND First = '%s' AND Last = '%s'",ssn,first,last)) {
				AfxMessageBox(last + ", " + first + " already exists in Practice.");
				m_informList->TakeRow(m_informSelected->GetRow(0));
				continue;
			}
			*/



			//JJ - we don't want to mess up the incrementing of our PersonIDs, so we will definitely make a new one
			//regardless. However, try and keep the ChartNumber as the UserDefined ID. Otherwise, get the next UserDefinedID.
			rsID = CreateRecordset("SELECT UserDefinedID FROM PatientsT WHERE UserDefinedID = %li", practiceID);
			if (rsID->eof && practiceID > 0)
				pracuserID = practiceID;
			else
				pracuserID = NewNumber("PatientsT","UserDefinedID");
			rsID->Close();

			practiceID = NewNumber("PersonT", "ID");

			//default provider
			long provider = -1;
			rsID  = CreateRecordset("SELECT DefaultProviderID FROM LocationsT WHERE ID = %li",GetCurrentLocationID());
			if (!rsID->eof && rsID->Fields->Item["DefaultProviderID"]->Value.vt != VT_NULL)
				provider = rsID->Fields->Item["DefaultProviderID"]->Value.lVal;
			else
			{	rsID->Close();
				rsID = CreateRecordset("SELECT PersonID FROM ProvidersT");
				if (!rsID->eof)
					provider = rsID->Fields->Item["PersonID"]->Value.lVal;
			}
			rsID->Close();

			////////////////////////////////////////////////////
			//import the referral source!
			CString Refsource = "", RefName = "";

			BOOL bIgnoreSourceDetailInfo = FALSE;

			try
			{
				informDB->Execute("SELECT TOP 1 SourceDetailID FROM tblPatient", NULL, adCmdText);
			}
			catch(_com_error)
			{
				bIgnoreSourceDetailInfo = TRUE;
			}

			if(IsFullVersion) {		

				if(!bIgnoreSourceDetailInfo) {
					//first look for detailed referrals
					sql.Format("SELECT tblPatient.RefSourceID AS ReferralID, tblSourceDetOther.Name FROM tblPatient LEFT JOIN tblSourceDetOther ON tblPatient.SourceDetailID = tblSourceDetOther.ID WHERE tblPatient.ID = %li AND tblPatient.RefSourceID > 0 AND tblPatient.SourceDetailID > 0",informID);
					rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
					if (!rsID->eof) {
						CString Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
						rsID->Close();
						sql.Format("SELECT PersonID AS ID FROM ReferralSourceT WHERE Name = '%s'",_Q(Refsource));
						rsID = CreateRecordsetStd(sql);
						if (!rsID->eof) {
							RefID = rsID->Fields->Item["ID"]->Value.lVal;
							RefName = Refsource;
						}
					}
					rsID->Close();
				}

				//else look for "root" referrals
				if(RefName=="") {
					sql.Format("SELECT tblPatient.RefSourceID AS ReferralID, tblRefSource.Label FROM tblPatient LEFT JOIN tblRefSource ON tblPatient.RefSourceID = tblRefSource.ID WHERE tblPatient.ID = %li AND tblPatient.RefSourceID > 0",informID);
					rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
					if (!rsID->eof) {
						CString Refsource = CString(rsID->Fields->Item["Label"]->Value.bstrVal);
						rsID->Close();
						sql.Format("SELECT PersonID AS ID FROM ReferralSourceT WHERE Name = '%s'",_Q(Refsource));
						rsID = CreateRecordsetStd(sql);
						if (!rsID->eof) {
							RefID = rsID->Fields->Item["ID"]->Value.lVal;
							RefName = Refsource;
						}		
					}
					rsID->Close();
				}
				////////////////////////////////////////////////////
			}

			long InformID;

			if(IsFullVersion) {
			//get the values out of inform
				if(!bIgnoreSourceDetailInfo)
					sql.Format("SELECT ID AS InformID, SalutationID, First, Last, Middle, Salutation AS Nickname, SSN AS SocialSecurity, BirthDate, IIf([DateOfContact],[DateOfContact],Now()) AS FirstContactDate, "
						"IIf([M/F]='M',1,IIf([M/F]='F',2,Null)) AS Gender,Address1, Address2, City, State, NightPhone AS HomePhone, DayPhone AS WorkPhone, EMail, CellPhone, "
						"DayExt AS Extension, Fax, Zip, [Inactive] AS Archived FROM tblPatient WHERE ID = %li;", informID);
				else
					sql.Format("SELECT ID AS InformID, SalutationID, First, Last, Middle, Salutation AS Nickname, SSN AS SocialSecurity, BirthDate, IIf([DateOfContact],[DateOfContact],Now()) AS FirstContactDate, "
						"IIf([M/F]='M',1,IIf([M/F]='F',2,Null)) AS Gender,Address1, Address2, City, State, NightPhone AS HomePhone, DayPhone AS WorkPhone, "
						"DayExt AS Extension, Fax, Zip, [Inactive] AS Archived FROM tblPatient WHERE ID = %li;", informID);
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);

				long Archived,Gender;
				CString First,Last,Middle,Nickname,SocialSecurity,BirthDate,FirstContactDate,Address1,Address2,
					City,State,Zip,HomePhone,WorkPhone,Email,CellPhone,Extension,Fax,NickName,Prefix;
				COleDateTime birthdate,firstdate,inputdate;
			
				//this is SO much safer then doing all the rs-> calls inside CreateRecordset
				InformID = rsID->Fields->Item["InformID"]->Value.lVal;

				var = rsID->Fields->Item["Archived"]->Value;
				if(var.vt==VT_NULL)
					Archived = 0;
				else {
					if(var.vt==VT_I4)
						Archived = var.lVal;
					else if(var.vt==VT_BOOL && var.boolVal)
						Archived = 1;
					else
						Archived = 0;
				}

				var = rsID->Fields->Item["Gender"]->Value;
				if(var.vt==VT_NULL)
					Gender = 0;
				else
					Gender = var.lVal;

				var = rsID->Fields->Item["SalutationID"]->Value;
				if(var.vt==VT_NULL)
					Prefix = "NULL";
				else {
					_RecordsetPtr rsPrefix = CreateRecordset("SELECT ID FROM PrefixT WHERE InformID = %li", VarLong(var));
					if(rsPrefix->eof) {
						Prefix = "NULL";
					}
					else {
						Prefix.Format("%li", AdoFldLong(rsPrefix, "ID"));
					}
				}

				var = rsID->Fields->Item["First"]->Value;
				if(var.vt==VT_NULL)
					First = "";
				else
					First = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["Last"]->Value;
				if(var.vt==VT_NULL)
					Last = "";
				else
					Last = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["Middle"]->Value;
				if(var.vt==VT_NULL)
					Middle = "";
				else
					Middle = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["Nickname"]->Value;
				if(var.vt==VT_NULL)
					Nickname = "";
				else
					Nickname = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["SocialSecurity"]->Value;
				if(var.vt==VT_NULL)
					SocialSecurity = "";
				else
					SocialSecurity = CString(var.bstrVal);

				var = rsID->Fields->Item["Address1"]->Value;
				if(var.vt==VT_NULL)
					Address1 = "";
				else
					Address1 = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["Address2"]->Value;
				if(var.vt==VT_NULL)
					Address2 = "";
				else
					Address2 = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["City"]->Value;
				if(var.vt==VT_NULL)
					City = "";
				else
					City = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["State"]->Value;
				if(var.vt==VT_NULL)
					State = "";
				else
					State = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["Zip"]->Value;
				if(var.vt==VT_NULL)
					Zip = "";
				else
					Zip = _Q(CString(var.bstrVal));

				var = rsID->Fields->Item["BirthDate"]->Value;
				if(var.vt==VT_NULL)
					BirthDate = "NULL";
				else {
					birthdate = var.date;
					BirthDate = birthdate.Format("'%m/%d/%Y'");
				}

				var = rsID->Fields->Item["FirstContactDate"]->Value;
				if(var.vt==VT_NULL)
					FirstContactDate = "NULL";
				else {
					firstdate = var.date;
					FirstContactDate = firstdate.Format("'%m/%d/%Y'");
				}

				var = rsID->Fields->Item["HomePhone"]->Value;
				if(var.vt==VT_NULL)
					HomePhone = "";
				else
					HomePhone = CString(var.bstrVal);

				var = rsID->Fields->Item["WorkPhone"]->Value;
				if(var.vt==VT_NULL)
					WorkPhone = "";
				else
					WorkPhone = CString(var.bstrVal);

				var = rsID->Fields->Item["Extension"]->Value;
				if(var.vt==VT_NULL)
					Extension = "";
				else
					Extension = CString(var.bstrVal);

				var = rsID->Fields->Item["Fax"]->Value;
				if(var.vt==VT_NULL)
					Fax = "";
				else
					Fax = CString(var.bstrVal);
				if(bIgnoreSourceDetailInfo || rsID->Fields->Item["EMail"]->Value.vt==VT_NULL)
					Email = "";
				else
					Email = _Q(CString(rsID->Fields->Item["EMail"]->Value.bstrVal));
				
				if(bIgnoreSourceDetailInfo || rsID->Fields->Item["CellPhone"]->Value.vt==VT_NULL)
					CellPhone = "";
				else
					CellPhone = CString(rsID->Fields->Item["CellPhone"]->Value.bstrVal);

				rsID->Close();

				//This does the insert
				try{
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
					CSqlTransaction trans;
					trans.Begin();
					// (j.jones 2010-01-14 16:35) - PLID 31927 - supported defaulting the text message privacy field
					ExecuteSql("INSERT INTO PersonT (Archived,ID,Location,PrefixID,First,Last,Middle,SocialSecurity,BirthDate,FirstContactDate,Gender,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,Fax,Email,CellPhone,InputDate,TextMessage) "
						"VALUES (%li,%li,%li,%s,'%s','%s','%s','%s',%s,%s,%li,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',GetDate(),%li)",Archived,practiceID,GetCurrentLocationID(),Prefix,First,Last,Middle,SocialSecurity,BirthDate,FirstContactDate,Gender,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,Fax,Email,CellPhone,nTextMessagePrivacy);
					ExecuteSql("INSERT INTO PatientsT (InformID,PersonID,UserDefinedID,ReferralID,Nickname,CurrentStatus,MainPhysician) VALUES (%li,%li,%li,%li,'%s',1,%li)",InformID,practiceID,pracuserID,RefID,Nickname,provider);

					// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
					ExecuteParamSql(
						"DECLARE @SecurityGroupID INT\r\n"
						"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
						"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
						"BEGIN\r\n"
						"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, {INT})\r\n"
						"END\r\n", practiceID);

					// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
					UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), practiceID);
					trans.Commit();
				}NxCatchAll("Error in OnImport()");
			}
			else {
				sql.Format("SELECT ID AS InformID, IIf([DateOfContact],[DateOfContact],Now()) AS FirstContactDate FROM tblPatient WHERE ID = %li;", informID);
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
				InformID = rsID->Fields->Item["InformID"]->Value.lVal;
				CString FirstContactDate;
				COleDateTime firstdate;
				if(rsID->Fields->Item["FirstContactDate"]->Value.vt==VT_NULL)
					FirstContactDate = "NULL";
				else {
					firstdate = rsID->Fields->Item["FirstContactDate"]->Value.date;
					FirstContactDate = firstdate.Format("'%m/%d/%Y'");
				}
				rsID->Close();
				try{
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
					CSqlTransaction trans;
					trans.Begin();
					// (j.jones 2010-01-14 16:35) - PLID 31927 - supported defaulting the text message privacy field
					ExecuteSql("INSERT INTO PersonT (Archived,ID,Location,First,Last,Middle,FirstContactDate,InputDate,TextMessage) "
						"VALUES (%li,%li,%li,'%s','%s','%s',%s,GetDate(),%li)",0,practiceID,GetCurrentLocationID(),first,last,middle,FirstContactDate,nTextMessagePrivacy);
					ExecuteSql("INSERT INTO PatientsT (InformID,PersonID,UserDefinedID,CurrentStatus,MainPhysician) VALUES (%li,%li,%li,1,%li)",InformID,practiceID,pracuserID,provider);

					// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
					ExecuteParamSql(
						"DECLARE @SecurityGroupID INT\r\n"
						"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
						"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
						"BEGIN\r\n"
						"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, {INT})\r\n"
						"END\r\n", practiceID);

					// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
					UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), practiceID);
					trans.Commit();
				}NxCatchAll("Error in OnImport()");
			}

			//add the patient to the patient toolbar
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			GetMainFrame()->m_patToolBar.UpdatePatient(practiceID);

			//add row to the practice list
			IRowSettingsPtr pRow;
			long row;
			//row = m_informList->FindByColumn(2,informID,0,FALSE);
			//if(row!=-1) {
				m_informSelected->SetRedraw(FALSE);
				pRow = m_informSelected->GetRow(0);
				//color Inform row
				pRow->PutBackColor(0x00FCDAD1);

				row = m_practiceList->AddRow(pRow);

				//edit the row
				pRow = m_practiceList->GetRow(row);
				pRow->PutValue(2,practiceID);
				pRow->PutValue(3,pracuserID);
				pRow->PutValue(4,InformID);

				
				//color Practice row
				pRow = m_practiceList->GetRow(row);
				pRow->PutBackColor(0x00DAFCD1);
				m_informSelected->SetRedraw(TRUE);
			//}
			m_informList->TakeRow(m_informSelected->GetRow(0));
		}
	}NxCatchAll("Error importing.");

	str.Format ("%li", m_informSelected->GetRowCount());
	SetDlgItemText (IDC_INF_SELECT_COUNT, str);

	str.Format("%li",m_practiceList->GetRowCount());
	SetDlgItemText(IDC_PRAC_COUNT, str);
}

UINT InformCrossReference(LPVOID pParam)
{
	InformCrossRef *p = (InformCrossRef *)pParam;

	SetStatusBarText(p->statusBar, "Cross Referencing...");

	try {
		// (a.walling 2007-07-20 11:35) - PLID 26762 - ADO is not threadsafe; create a new connection
		// Open an new connection based on the existing one
		
		_ConnectionPtr pExistingConn = GetRemoteData();
		_ConnectionPtr pNewConn(__uuidof(Connection));

		// Use the same connection timeout
		pNewConn->PutConnectionTimeout(pExistingConn->GetConnectionTimeout());
		// Open the connection
		pNewConn->Open(pExistingConn->ConnectionString, "", "", 0);
		// Use the same command timeout
		pNewConn->PutCommandTimeout(pExistingConn->GetCommandTimeout());

		long lastPracRow = 0, lastInformRow = 0;
		long PracRow = 0, InformRow = 0;
		_variant_t var;
		_RecordsetPtr rsPractice = CreateRecordset(pNewConn, "SELECT PatientsT.PersonID AS ID, PatientsT.InformID FROM PatientsT WHERE PersonID > 0 AND InformID > 0 ORDER BY InformID DESC");
		while (!rsPractice->eof) {

			if (p->stopRequest)
				AfxEndThread(0, 0);
			
			var = rsPractice->Fields->Item["InformID"]->Value;
			InformRow = p->informList->FindByColumn(2,var,lastInformRow,FALSE);
			var = rsPractice->Fields->Item["ID"]->Value;
			PracRow = p->practiceList->FindByColumn(2,var,lastPracRow,FALSE);

			ASSERT(InformRow!=-2); //in these calls, FindbyColumn cannot return -2. If they do, then there
			ASSERT(PracRow!=-2);   //is something wrong with the datalist. Tell Bob immediately.

			if(InformRow==-1 && PracRow!=-1) {
				//not found in inform
				IRowSettingsPtr pRow;			
				pRow = p->practiceList->GetRow(PracRow);
				pRow->BackColor = 0xFFFF;
				rsPractice->MoveNext();
				continue;
			}		
			else if (PracRow!=-1 && InformRow != -1) {
				/*&& (CString(p->practiceList->GetValue(PracRow,1).bstrVal)==CString(p->informList->GetValue(InformRow,1).bstrVal))*/
				IRowSettingsPtr pRow;			
				pRow = p->practiceList->GetRow(PracRow);
				pRow->PutBackColor(0x00DAFCD1);
				pRow = p->informList->GetRow(InformRow);
				pRow->PutBackColor(0x00FCDAD1);
				lastPracRow = PracRow;
				lastInformRow = InformRow;
			}
			rsPractice->MoveNext();
		}
		rsPractice->Close();
		pNewConn->Close();
	// (a.walling 2007-07-20 10:59) - PLID 26762 - We'd like to know about these exceptions. Added NxCatchAllThread rather than ignore them.
	} NxCatchAllThread("Error in InformCrossReference");

	SetStatusBarText(p->statusBar, "Ready");

	return 0;
}

void CInformLink::KillThread()
{
	if (m_pThread)
	{	m_pThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		m_cross.stopRequest = true;
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		delete m_pThread;
		m_pThread = NULL;
	}
}

void CInformLink::CrossReference()
{
	KillThread();
	m_cross.stopRequest = false;
	m_pThread = AfxBeginThread(::InformCrossReference, (void *)&m_cross, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();
}

void CInformLink::OnRequeryFinishedPracPatients(short nFlags) 
{
	CString str;
	m_PracticeRequeryDone = TRUE;
	str.Format ("%li", m_practiceList->GetRowCount());
	SetDlgItemText(IDC_PRAC_COUNT, str);
	if(m_InformRequeryDone && m_PracticeRequeryDone)
		CrossReference();	
}

void CInformLink::OnRequeryFinishedInformPatients(short nFlags) 
{
	CString str;
	m_InformRequeryDone = TRUE;
	str.Format ("%li", m_informList->GetRowCount());
	SetDlgItemText (IDC_INF_COUNT, str);
	if(m_InformRequeryDone && m_PracticeRequeryDone)
		CrossReference();
}

void CInformLink::OnDblClickCellInformPatients(long nRowIndex, short nColIndex) 
{
	OnInfAdd();
}

void CInformLink::OnDblClickCellPracPatients(long nRowIndex, short nColIndex) 
{
	OnPracAdd();
}

void CInformLink::OnShowAdvanced() 
{
	if(((CButton*)GetDlgItem(IDC_SHOW_ADVANCED))->GetCheck()) {
		GetDlgItem(IDC_LINK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_UNLINK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LINK_PREFIXES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHECK_DISABLEINFORM)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_LINK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UNLINK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LINK_PREFIXES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_DISABLEINFORM)->ShowWindow(SW_HIDE);
	}
}

void CInformLink::OnLink() 
{
	if(m_practiceList->GetCurSel()==-1)
		return;
	if(m_informList->GetCurSel()==-1)
		return;
	try {
		if(IDNO==MessageBox("This action will link the two selected patients together. No data will be transferred.\n"
			"Are you sure you wish to do this?","Inform",MB_YESNO|MB_ICONQUESTION))
			return;
		long PracRow, InformRow, InformID;
		PracRow = m_practiceList->GetCurSel();
		InformRow = m_informList->GetCurSel();
		if(CString(m_practiceList->GetValue(PracRow,1).bstrVal)!=CString(m_informList->GetValue(InformRow,1).bstrVal)) {
			if(IDNO==MessageBox("The selected patients have different names. Are you sure you wish to link them?","Practice",MB_ICONQUESTION|MB_YESNO))
				return;
		}
		InformID = m_informList->GetValue(InformRow,2).lVal;
		if(InformID<0) {
			AfxMessageBox("The patient in the inform list is already linked into practice.");
			return;
		}
		ExecuteSql("UPDATE PatientsT SET InformID = %li WHERE PersonID = %li",InformID,m_practiceList->GetValue(m_practiceList->GetCurSel(),2).lVal);
		IRowSettingsPtr pRow;
		pRow = m_practiceList->GetRow(PracRow);
		pRow->PutBackColor(0x00DAFCD1);
		pRow = m_informList->GetRow(InformRow);
		pRow->PutBackColor(0x00FCDAD1);
		m_practiceList->PutValue(m_practiceList->GetCurSel(),4,(long)InformID);
	}NxCatchAll("Error Linking patient.");
}

void CInformLink::OnUnlink() 
{
	if(m_practiceList->GetCurSel()==-1)
		return;
	try {
		if(IDNO==MessageBox("This action will break the link between this patient and inform. Data will not be removed.\n"
			"Are you sure you wish to do this?","Inform",MB_YESNO|MB_ICONQUESTION))
			return;
		ExecuteSql("UPDATE PatientsT SET InformID = 0 WHERE PersonID = %li",m_practiceList->GetValue(m_practiceList->GetCurSel(),2).lVal);
		IRowSettingsPtr pRow;
		pRow = m_practiceList->GetRow(m_practiceList->GetCurSel());
		pRow->PutBackColor(0xFFFFFF);
		long row = m_informList->FindByColumn(2,m_practiceList->GetValue(m_practiceList->GetCurSel(),4),0,FALSE);
		if(row!=-1) {
			pRow = m_informList->GetRow(row);
			pRow->PutBackColor(0xFFFFFF);
		}
		m_practiceList->PutValue(m_practiceList->GetCurSel(),4,(long)0);
	}NxCatchAll("Error Unlinking patient.");
}

void CInformLink::OnDblClickCellPracPatientsSelected(long nRowIndex, short nColIndex) 
{
	OnPracRem();
}

void CInformLink::OnDblClickCellInformPatientsSelected(long nRowIndex, short nColIndex) 
{
	OnInfRem();
}

/*
void CInformLink::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	CNxDialog::SetControlPositions();
	CNxDialog::Invalidate();	
}
*/

void CInformLink::OnOK() 
{
	if (informDB != NULL)
	{
		try {
		if(informDB->State == adStateOpen)
			informDB->Close();
		}NxCatchAll("Error closing database.");
	}
	GetMainFrame()->UpdateAllViews();
	
	CDialog::OnOK();
}

void CInformLink::OnCancel() 
{
	try {
	if(informDB != NULL && informDB->State == adStateOpen)
		informDB->Close();
	}NxCatchAll("Error closing database.");

	GetMainFrame()->UpdateAllViews();
	
	CDialog::OnCancel();
}

BOOL CInformLink::CheckInformVersion()
{
	
	CString sql,strVersion;
	_RecordsetPtr rsID(__uuidof(Recordset));

	//not only does this function check the version of inform, it also serves to check for a valid inform database
	//return TRUE if valid, FALSE if not

	try {
		//check the two key tables to identify the database as being an inform database
		rsID->Open("SELECT Count(strVersion) AS Total FROM tblVersion", _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		rsID->Close();
		rsID->Open("SELECT Count(ID) AS Total FROM tblPatient", _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		rsID->Close();
	} catch(_com_error e) {
		//THIS is what we want to happen. If the above tables do not exist, then it's not an Inform database.
		MessageBox("The database you selected is not a valid Inform database.","Practice",MB_OK|MB_ICONEXCLAMATION);
		//TES 5/24/2013 - PLID 56860 - We have now checked the Inform version
		m_bCheckedVersion = TRUE;
		return FALSE;
	}

	try {	

		//checking version of Inform, most everyone is full, so assume full if any errors occur
		IsFullVersion = TRUE;

		sql.Format("SELECT strVersion FROM tblVersion");
		rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		if(!rsID->eof) {
			strVersion = CString(rsID->Fields->Item["strVersion"]->Value.bstrVal);
			if(strVersion.Left(1)<"3") {
				IsFullVersion = FALSE;
			}
			else {
				IsFullVersion = TRUE;
			}
		}
		else {
			IsFullVersion = TRUE;
		}

	}NxCatchAll("Error checking Inform version.");

	//TES 5/24/2013 - PLID 56860 - We have now checked the Inform version
	m_bCheckedVersion = TRUE;
	return TRUE;
}

HBRUSH CInformLink::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_LABEL2:
		case IDC_PRAC_COUNT:
		case IDC_LABEL8:
		case IDC_PRAC_SELECT_COUNT:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00D79D8C));
			return m_pracBrush;
		case IDC_INF_COUNT:
		case IDC_LABEL1:
		case IDC_INF_SELECT_COUNT:
		case IDC_LABEL3:
		case IDC_LABEL7:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x009CDA9C));
			return 	m_informBrush;
	}

	return hbr;
}

void CInformLink::OnNewPatCheck() 
{
	SetPropertyInt("NewPatExportToInform", ((CButton*)GetDlgItem(IDC_NEW_PAT_CHECK))->GetCheck());
}

void CInformLink::OnDisableLinkCheck()
{
	int nChecked = ((CButton*)GetDlgItem(IDC_CHECK_DISABLEINFORM))->GetCheck();
	if (nChecked)
	{
		if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to disable the Inform Link on your computer? This will not affect the link on any other computer in the office."))
		{
			((CButton*)GetDlgItem(IDC_CHECK_DISABLEINFORM))->SetCheck(0);
			return;
		}
		// Clear the lists
		m_informList->Clear();
		m_practiceList->Clear();
		m_practiceSelected->Clear();
		m_informSelected->Clear();

		SetDlgItemText (IDC_INF_COUNT, "");
		SetDlgItemText (IDC_PRAC_COUNT, "");
		m_statusBar.SetText("Link to Inform is disabled", 255, 0);

		if (informDB != NULL)
		{
			informDB.Release();
			informDB = NULL;
		}
	}
	else if (informDB == NULL)
	{
		if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to enable the Inform Link on your computer? This will not affect the link on any other computer in the office."))
		{
			((CButton*)GetDlgItem(IDC_CHECK_DISABLEINFORM))->SetCheck(1);
			return;
		}

		// Rebuild the lists
		if (!EnsureData())
		{
			// If we can't re-establish the link, disable it again
			if (informDB != NULL)
			{
				informDB.Release();
				informDB = NULL;
			}
			((CButton*)GetDlgItem(IDC_CHECK_DISABLEINFORM))->SetCheck(1);
		}
		else
		{
			m_statusBar.SetText("Loading...", 255, 0);
			SetDlgItemText (IDC_INF_COUNT, "Loading...");
			SetDlgItemText (IDC_PRAC_COUNT, "Loading...");
			m_practiceList->Requery();
		}
	}
	SetPropertyInt("InformDisable", ((CButton*)GetDlgItem(IDC_CHECK_DISABLEINFORM))->GetCheck(), 0);
}

void CInformLink::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
	
	CMainFrame* pMain = GetMainFrame();
	if(pMain) {
		pMain->ActivateFrame();
		pMain->BringWindowToTop();
	}

	SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

void CInformLink::OnLinkPrefixes() 
{
	CEditPrefixesDlg dlg(this);
	dlg.m_bChangeInformIds = true;
	dlg.DoModal();
}

LRESULT CInformLink::OnEnsureData(WPARAM wparam, LPARAM lParam)
{
	if (!EnsureData())
	{
		// The user cancelled the link
		PostMessage(WM_COMMAND, IDCANCEL);
	}
	return 0;
}
