// UpdatePrefixesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "UpdatePrefixesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MALE 1
#define FEMALE 2

/////////////////////////////////////////////////////////////////////////////
// CUpdatePrefixesDlg dialog


CUpdatePrefixesDlg::CUpdatePrefixesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdatePrefixesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdatePrefixesDlg)
	//}}AFX_DATA_INIT
}


void CUpdatePrefixesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdatePrefixesDlg)
	DDX_Control(pDX, IDC_UPDATE_MALES, m_btnUpdateMales);
	DDX_Control(pDX, IDC_UPDATE_FEMALES, m_btnUpdateFemales);
	DDX_Control(pDX, IDC_MALE_PATIENT_TYPE, m_MaleType);
	DDX_Control(pDX, IDC_FEMALE_PATIENT_TYPE, m_FemaleType);
	DDX_Control(pDX, IDC_UPDATE_PREFIX, m_btnUpdatePrefix);
	DDX_Control(pDX, IDC_CLOSE_UPDATE_PREFIXES, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdatePrefixesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdatePrefixesDlg)
	ON_BN_CLICKED(IDC_UPDATE_PREFIX, OnUpdatePrefix)
	ON_BN_CLICKED(IDC_CLOSE_UPDATE_PREFIXES, OnCloseUpdatePrefixes)
	ON_BN_CLICKED(IDC_UPDATE_FEMALES, OnUpdateFemales)
	ON_BN_CLICKED(IDC_UPDATE_MALES, OnUpdateMales)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdatePrefixesDlg message handlers

void CUpdatePrefixesDlg::OnUpdatePrefix() 
{

	try {

		CWaitCursor  wait;

		CString strSql, strFemaleString, strMaleString;

		//ladies first!!
		if (IsDlgButtonChecked(IDC_UPDATE_FEMALES)) {

			strFemaleString = GetUpdateString(FEMALE);
	
		}

		if (IsDlgButtonChecked(IDC_UPDATE_MALES)) {

			strMaleString = GetUpdateString(MALE);
		}

		//check to make sure that there are no errors in the selections
		if (strFemaleString == "ERROR" || strMaleString == "ERROR") {

			//a selection is wrong
			MsgBox("Please select valid criteria");
		}
		else { 
			
			strSql = strFemaleString + strMaleString;

			strSql.TrimRight();
			strSql.TrimLeft();

			if (! strSql.IsEmpty()) {
	
				ExecuteSqlStd(strSql);
			}
		}

		GetMainFrame()->UpdateAllViews();
		MsgBox("Completed Successfully");

	}NxCatchAll("Error in Update Prefix");
	
}

CString CUpdatePrefixesDlg::GetUpdateString(long nGender){


	//declare the variables
	CString strUpdateTo, strWhereClause, strSql;

	//get what we are updating to
	strUpdateTo = GetNewPrefix(nGender);

	//get our where clause
	strWhereClause = GetWhereClause(nGender);

	//format the SQL statement
	strSql.Format("UPDATE PersonT SET PrefixID = %s WHERE %s;", strUpdateTo, strWhereClause);

	//check to see that we don't have any errors
	if (strUpdateTo == "ERROR" || strWhereClause == "ERROR") {
		return "ERROR";
	}
	else {
		//return the sql statement
		return strSql;
	}

}


CString CUpdatePrefixesDlg::GetNewPrefix(long nGender) {

	//switch on the genders
	switch (nGender) {

		case FEMALE:
			{
				if(m_pFemaleTitle->CurSel == -1 || m_pFemaleTitle->GetValue(m_pFemaleTitle->CurSel, 0).vt != VT_I4) {
					return "ERROR";
				}
				else {
					CString strRet;
					strRet.Format("%li", VarLong(m_pFemaleTitle->GetValue(m_pFemaleTitle->CurSel, 0)));
					return strRet;
				}
			}
		break;


		case MALE:
			{

				if(m_pMaleTitle->CurSel == -1 || m_pMaleTitle->GetValue(m_pMaleTitle->CurSel, 0).vt != VT_I4) {
					return "ERROR";
				}
				else {
					CString strRet;
					strRet.Format("%li", VarLong(m_pMaleTitle->GetValue(m_pMaleTitle->CurSel, 0)));
					return strRet;
				}
			}

		break;


		default: 

			return "ERROR";

		break;
	}

}



CString CUpdatePrefixesDlg::GetWhereClause(long nGender) {

	CString strClause;
	long nCurSel;

	if (nGender == MALE) {

		nCurSel = m_MaleType.GetCurSel();
	}
	else {
		nCurSel = m_FemaleType.GetCurSel();
	}

	switch (nCurSel) {

		case 0:  //all

			strClause = "";
		break;

		case 1:  //Patient 

			strClause = "WHERE CurrentStatus = 1";

		break;

		case 2: //patient/Prospect
			
			strClause = "WHERE CurrentStatus = 3";
		
		break;

		case 3: //prospect

			strClause = "WHERE CurrentStatus = 2";
		break;

		default:  //return error

			return "ERROR";
		
		break;
	}


	//now, put it all together
	CString strWhere;

	strWhere.Format(" (ID IN (SELECT PersonID FROM PatientsT %s)) AND (PersonT.Gender = %li) AND (PersonT.PrefixID Is Null)", strClause, nGender);

	return strWhere;
}



long CUpdatePrefixesDlg::GetBoxID(long nGender) {

	//first, get the id of the box we need 
	if (nGender == MALE) {

		return IDC_MALE_PATIENT_TYPE;
	}
	else if (nGender == FEMALE) {

		return IDC_FEMALE_PATIENT_TYPE;
	}
	else {

		return NULL;
	}

}

void CUpdatePrefixesDlg::OnCloseUpdatePrefixes() 
{

	CDialog::OnOK();
}

void CUpdatePrefixesDlg::OnUpdateFemales() 
{
	if (IsDlgButtonChecked(IDC_UPDATE_FEMALES)) {

		//enable everything else
		m_FemaleType.EnableWindow(TRUE);
		GetDlgItem(IDC_FEMALE_TITLE)->EnableWindow(TRUE);
	}
	else {
		m_FemaleType.EnableWindow(FALSE);
		GetDlgItem(IDC_FEMALE_TITLE)->EnableWindow(FALSE);
	}

	
}

void CUpdatePrefixesDlg::OnUpdateMales() 
{
	if (IsDlgButtonChecked(IDC_UPDATE_MALES)) {

		//enable everything else
		m_MaleType.EnableWindow(TRUE);
		GetDlgItem(IDC_MALE_TITLE)->EnableWindow(TRUE);
	}
	else {
		m_MaleType.EnableWindow(FALSE);
		GetDlgItem(IDC_MALE_TITLE)->EnableWindow(FALSE);
	}
	
}

BOOL CUpdatePrefixesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnUpdatePrefix.AutoSet(NXB_MODIFY);
	m_btnClose.AutoSet(NXB_CLOSE);

	m_pFemaleTitle = BindNxDataListCtrl(this,IDC_FEMALE_TITLE,GetRemoteData(),true);
	m_pMaleTitle = BindNxDataListCtrl(this,IDC_MALE_TITLE,GetRemoteData(),true);
	
	//Disable the windows
	m_MaleType.EnableWindow(FALSE);
	GetDlgItem(IDC_MALE_TITLE)->EnableWindow(FALSE);
	m_FemaleType.EnableWindow(FALSE);
	GetDlgItem(IDC_FEMALE_TITLE)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
