// ExportDuplicates.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ExportDuplicates.h"
#include "GlobalDataUtils.h"
#include "Mirror.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CExportDuplicates dialog
//
// Return values:
//	1 - Add the new patient
//  2 - Update to selected patient
//  3 - Skip this patient
//  0 - Stop Exporting
//
/////////////////////////////////////////////////////////////////////////////

bool CExportDuplicates::FindDuplicates(CString first, CString last, CString middle, CString Source, CString Dest, _ConnectionPtr pConn, CString path, BOOL bAllowUpdate, CString strPassword, BOOL bAssumeOneMatchingNameLinks, CString strSSN) {

	CString str,value;
	_RecordsetPtr rs(__uuidof(Recordset));

	m_Source = Source;
	m_Dest = Dest;
	m_pConn = pConn;
	m_bAllowUpdate = bAllowUpdate;
	m_path = path;
	m_strPassword = strPassword;
	m_bOneMatchingNameLinks = FALSE;

	if(m_Dest=="Practice") {
		//search for duplicates in Practice

		//JJ - 5/14/2001 - we decided NOT to search for duplicates based on middle name. I am not going to take "middle"
		//out of the parameter list though, so we can make the change easily if we ever want it back
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE First = '%s' AND Last = '%s'", _Q(first), _Q(last));
		
		if (rs->eof)
			return false;
		m_sql = "SELECT ID, UserDefinedID AS UserID, SocialSecurity AS SSN, BirthDate, HomePhone, WorkPhone, Address1, Address2, City, State, Zip "
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = ";
		while (!rs->eof)
		{	value.Format("%li", AdoFldLong(rs, "ID"));
			m_sql += value + " OR ID = ";
			rs->MoveNext();
		}
		rs->Close();
		m_sql.TrimRight (" OR ID = ");
		m_name = first + ' ' + last;
		return true;
	}
	else if(m_Dest=="Inform") {
		//search for duplicates in Inform

		CString sql,strVersion;
		BOOL IsFullVersion = TRUE;
		sql.Format("SELECT strVersion FROM tblVersion");
		rs->Open(_bstr_t(sql), _variant_t((IDispatch *) m_pConn, true), adOpenStatic, adLockReadOnly, adCmdText);
		if(!rs->eof) {
			strVersion = CString(rs->Fields->Item["strVersion"]->Value.bstrVal);
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

		rs->Close();
		
		str.Format("SELECT ID FROM tblPatient WHERE First = '%s' AND Last = '%s'", _Q(first), _Q(last));
		rs->Open(_bstr_t(str), _variant_t((IDispatch *) m_pConn, true), adOpenStatic, adLockReadOnly, adCmdText);
		
		if (rs->eof)
			return false;

		if(IsFullVersion)
			m_sql = "SELECT ID, ChartNumber AS UserID, SSN, BirthDate, NightPhone AS HomePhone, DayPhone AS WorkPhone, Address1, Address2, City, State, Zip "
				"FROM tblPatient WHERE ID = ";
		else
			m_sql = "SELECT ID, ChartNumber AS UserID, '' AS SSN, '' AS BirthDate, '' AS HomePhone, '' AS WorkPhone, '' AS Address1, '' AS Address2, '' AS City, '' AS State, '' AS Zip "
				"FROM tblPatient WHERE ID = ";

		while (!rs->eof)
		{	value.Format("%li", AdoFldLong(rs, "ID"));
			m_sql += value + " OR ID = ";
			rs->MoveNext();
		}
		rs->Close();
		m_sql.TrimRight (" OR ID = ");
		m_name = first + ' ' + last;
		return true;
	}
	else if(m_Dest=="Mirror") {
		//search for duplicates in Mirror

		str.Format("SELECT RECNUM, PIN FROM M2000 WHERE LASTNAME = \'%s\' AND FIRSTNAME = \'%s\'", _Q(last), _Q(first));


		rs->Open(_variant_t(_bstr_t(str)), 
			_variant_t((IDispatch *) m_pConn), 
			adOpenKeyset, 
			adLockBatchOptimistic,
			adOptionUnspecified);

		if (rs->eof)
			return false;

		// If we are exporting under the assumption that a single first-last
		// name match means to link the two patients together, we do it.
		if (bAssumeOneMatchingNameLinks && rs->GetRecordCount() == 1)
		{
			/*COleVariant vRemoteSSN = rs->Fields->Item["PIN"]->Value;
			CString strRemoteSSN;

			// ...but only if the social security number matches
			strSSN.Remove('-');
			strSSN.Remove(' ');

			// ...but only if the social security number matches
			if (vRemoteSSN.vt != VT_NULL && vRemoteSSN.vt != VT_EMPTY)
			{
				strRemoteSSN = vRemoteSSN.bstrVal;
				strRemoteSSN.Remove('-');
				strRemoteSSN.Remove(' ');
			}
			if (strSSN == strRemoteSSN)
			{*/
				m_bOneMatchingNameLinks = TRUE;
				m_varIDToUpdate = (LPCTSTR)CString(rs->Fields->Item["RECNUM"]->Value.bstrVal);
				return true;
			//}
		}

		m_sql = "SELECT RECNUM AS ID, RECNUM AS UserID, '' AS SSN, DOB AS BirthDate, PHONE1 AS HomePhone, PHONE2 AS WorkPhone, Address1, Address2, City, State, Zip "
			"FROM M2000 WHERE RECNUM = ";
		while (!rs->eof)
		{	value.Format("'%s'", AdoFldString(rs, "RECNUM",""));
			m_sql += value + " OR RECNUM = ";
			rs->MoveNext();
		}
		rs->Close();
		m_sql.TrimRight (" OR RECNUM = ");
		m_name = first + ' ' + last;
		return true;
	}
	else if (m_Dest=="United")
	{
		// Search for duplicates in United
		str.Format("SELECT ID, SSN FROM tblPatient WHERE First = '%s' AND Last = '%s'",
			_Q(first), _Q(last));

		rs->Open(_variant_t(_bstr_t(str)), 
			_variant_t((IDispatch *) m_pConn), 
			adOpenKeyset, 
			adLockBatchOptimistic,
			adOptionUnspecified);

		if (rs->eof)
		{
			rs->Close();
			return false;
		}

		// If we are exporting under the assumption that a single first-last
		// name match means to link the two patients together, we do it.
		if (bAssumeOneMatchingNameLinks && rs->GetRecordCount() == 1)
		{
			/*COleVariant vRemoteSSN = rs->Fields->Item["SSN"]->Value;
			CString strRemoteSSN;

			// ...but only if the social security number matches
			strSSN.Remove('-');
			strSSN.Remove(' ');

			if (vRemoteSSN.vt != VT_NULL && vRemoteSSN.vt != VT_EMPTY)
			{
				strRemoteSSN = vRemoteSSN.bstrVal;
				strRemoteSSN.Remove('-');
				strRemoteSSN.Remove(' ');
			}
			if (strSSN == strRemoteSSN)
			{*/
				m_bOneMatchingNameLinks = TRUE;
				m_varIDToUpdate = rs->Fields->Item["ID"]->Value;
				return true;
			//}
		}

		m_sql = "SELECT ID, ID AS UserID, SSN, BirthDate, DayPhone AS HomePhone, NULL AS WorkPhone, Address1, Address2, City, State, Zip "
			"FROM tblPatient WHERE ID = ";
		while (!rs->eof)
		{	value.Format("%li", AdoFldLong(rs, "ID"));
			m_sql += value + " OR ID = ";
			rs->MoveNext();
		}
		rs->Close();
		m_sql.TrimRight (" OR ID = ");
		m_name = first + ' ' + last;
		return true;
	}
	else return false;
}

CExportDuplicates::CExportDuplicates(CWnd* pParent /*=NULL*/)
	: CNxDialog(CExportDuplicates::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDuplicates)
	//}}AFX_DATA_INIT
}


void CExportDuplicates::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDuplicates)
	DDX_Control(pDX, IDC_STOP, m_Stop);
	DDX_Control(pDX, IDC_SKIP, m_Skip);
	DDX_Control(pDX, IDC_UPDATE, m_Update);
	DDX_Control(pDX, IDC_ADD_NEW, m_AddNew);
	DDX_Control(pDX, IDC_NAME_LABEL, m_nxstaticNameLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportDuplicates, CNxDialog)
	//{{AFX_MSG_MAP(CExportDuplicates)
	ON_BN_CLICKED(IDC_ADD_NEW, OnAddNew)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDuplicates message handlers

void CExportDuplicates::OnAddNew() 
{
	EndDialog(1);
}

void CExportDuplicates::OnUpdate() 
{
	if(m_Duplicate_List->GetRowCount()==1) {
		m_varIDToUpdate = m_Duplicate_List->GetValue(0,0);
	}
	else if(m_Duplicate_List->GetRowCount()>1 && m_Duplicate_List->GetCurSel()==-1) {
		AfxMessageBox("Please select the patient you wish to update");
		return;
	}
	else if(m_Duplicate_List->GetRowCount()>1 && m_Duplicate_List->GetCurSel()!=-1) {
		m_varIDToUpdate = m_Duplicate_List->GetValue(m_Duplicate_List->GetCurSel(),0);
	}
	else {
		AfxMessageBox("Please select the patient you wish to update");
		return;
	}

	EndDialog(2);
}

void CExportDuplicates::OnSkip() 
{
	EndDialog(3);
}

void CExportDuplicates::OnStop() 
{
	EndDialog(0);
}

BOOL CExportDuplicates::OnInitDialog() 
{
	extern CPracticeApp theApp;

	CNxDialog::OnInitDialog();

	if (m_bOneMatchingNameLinks)
	{
		EndDialog(2);
		return false;
	}

	m_AddNew.AutoSet(NXB_NEW);
	m_Update.AutoSet(NXB_MODIFY);
	m_Stop.AutoSet(NXB_CANCEL);

	GetDlgItem(IDC_NAME_LABEL)->SetFont(&theApp.m_titleFont);
	SetDlgItemText(IDC_NAME_LABEL, m_name);

	if(m_bAllowUpdate==FALSE)
		m_Update.ShowWindow(SW_HIDE);

	m_sql = "(" + m_sql + ") AS DuplicatesQ";
	if(m_Dest=="Practice") {
		m_varIDToUpdate = (long)-1;
		m_Duplicate_List = BindNxDataListCtrl(this,IDC_DUPLICATES,GetRemoteData(),false);
		m_Duplicate_List->FromClause = _bstr_t(m_sql);
		m_Duplicate_List->Requery();
		if(m_Source=="Inform")
			SetWindowText("Importing from Inform to Practice");
		else if(m_Source=="Mirror")
			SetWindowText("Importing from Mirror to Practice");
	}
	else if(m_Dest=="Inform") {
		m_varIDToUpdate = (long)-1;
		CString str;

		if(m_path=="" || !DoesExist(m_path)) {
			MsgBox("The Inform path cannot be found, it may have been changed since you began your session.");
			EndDialog(0);
			return false;
		}
		else {
			try {
				//connect the inform datalist
				str.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;User Id=;Password=;",m_path);
				m_Duplicate_List = GetDlgItem(IDC_DUPLICATES)->GetControlUnknown();
				m_Duplicate_List->PutConnectionString(_bstr_t(str));
				m_Duplicate_List->FromClause = _bstr_t(m_sql);
				m_Duplicate_List->Requery();

			}NxCatchAll("Error, could not check duplicates.");
		}

		SetWindowText("Exporting from Practice to Inform");
	}
	if(m_Dest=="Mirror") {
		m_varIDToUpdate = "";
		CString str;

		if(m_path=="" || !DoesExist(m_path)) {
			MsgBox("The Mirror path cannot be found, it may have been changed since you began your session.");
			EndDialog(0);
			return false;
		}
		else {
			try {
				//connect the inform datalist
				str.Format("Provider=Microsoft.Jet.OLEDB.4.0;"
						"Data Source=%s;"
						"User Id=admin;"
						"Password=;",m_path);
				m_Duplicate_List = GetDlgItem(IDC_DUPLICATES)->GetControlUnknown();
				m_Duplicate_List->PutConnectionString(_bstr_t(str));
				m_Duplicate_List->FromClause = _bstr_t(m_sql);
				m_Duplicate_List->GetColumn(2)->PutStoredWidth(0);
				m_Duplicate_List->Requery();

			}NxCatchAll("Error, could not check duplicates.");
		}
		SetWindowText("Exporting from Practice to Mirror");
	}
	if (m_Dest == "United")
	{
		m_varIDToUpdate = (long)-1;
		CString str;

		if(m_path=="" || !DoesExist(m_path)) {
			MsgBox("The United path cannot be found, it may have been changed since you began your session.");
			EndDialog(0);
			return false;
		}
		else {
			try {
				//connect the inform datalist
				str.Format("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;User Id=;Jet OLEDB:Database Password=%s;",m_path,m_strPassword);
				m_Duplicate_List = GetDlgItem(IDC_DUPLICATES)->GetControlUnknown();
				m_Duplicate_List->PutConnectionString(_bstr_t(str));
				m_Duplicate_List->FromClause = _bstr_t(m_sql);
				m_Duplicate_List->Requery();

			}NxCatchAll("Error, could not check duplicates.");
		}
		SetWindowText("Exporting from Practice to United");
	}
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
