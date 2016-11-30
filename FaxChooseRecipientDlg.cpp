// FaxChooseRecipientDlg.cpp : implementation file
//
//DRT 7/2/2008 - PLID 30601 - Created.

#include "stdafx.h"
#include "practice.h"
#include "FaxChooseRecipientDlg.h"
#include "GlobalParamUtils.h"


using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaxChooseRecipientDlg dialog


CFaxChooseRecipientDlg::CFaxChooseRecipientDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFaxChooseRecipientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaxChooseRecipientDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nPersonID = -1;
}


void CFaxChooseRecipientDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaxChooseRecipientDlg)
	DDX_Control(pDX, IDC_RECIP_OTHER_NUMBER, m_editOtherNum);
	DDX_Control(pDX, IDC_RECIP_OTHER_NAME, m_editOtherName);
	DDX_Control(pDX, IDC_RECIP_PERSON, m_btnPerson);
	DDX_Control(pDX, IDC_RECIP_REFPHYS, m_btnRefPhys);
	DDX_Control(pDX, IDC_RECIP_PCP, m_btnPCP);
	DDX_Control(pDX, IDC_RECIP_INSCO, m_btnInsCo);
	DDX_Control(pDX, IDC_RECIP_CONTACT, m_btnContact);
	DDX_Control(pDX, IDC_RECIP_LOCATION, m_btnLocation);
	DDX_Control(pDX, IDC_RECIP_OTHER, m_btnOther);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFaxChooseRecipientDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFaxChooseRecipientDlg)
	ON_BN_CLICKED(IDC_RECIP_PERSON, OnRecipPerson)
	ON_BN_CLICKED(IDC_RECIP_REFPHYS, OnRecipRefphys)
	ON_BN_CLICKED(IDC_RECIP_PCP, OnRecipPcp)
	ON_BN_CLICKED(IDC_RECIP_INSCO, OnRecipInsco)
	ON_BN_CLICKED(IDC_RECIP_CONTACT, OnRecipContact)
	ON_BN_CLICKED(IDC_RECIP_LOCATION, OnRecipLocation)
	ON_BN_CLICKED(IDC_RECIP_OTHER, OnRecipOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaxChooseRecipientDlg message handlers

BOOL CFaxChooseRecipientDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Bind datalists
		m_pInsList = BindNxDataList2Ctrl(IDC_RECIP_INSCO_LIST, false);
		m_pContactList = BindNxDataList2Ctrl(IDC_RECIP_CONTACT_LIST, true);
		m_pLocList = BindNxDataList2Ctrl(IDC_RECIP_LOCATION_LIST, true);

		//Setup controls for interface
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Load data
		LoadData();

		//Ensure all controls are enabled properly
		EnsureControls();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFaxChooseRecipientDlg::LoadData()
{
	//Contacts & Locations are global and auto loaded with datalist requeries.

	if(m_nPersonID <= 0) {
		//This means there is no person.  We don't need to load any person-related data, all the person-related
		//	controls will be disabled.

		//Disable everything
		EnableDlgItem(IDC_RECIP_PERSON, FALSE);
		EnableDlgItem(IDC_RECIP_REFPHYS, FALSE);
		EnableDlgItem(IDC_RECIP_PCP, FALSE);
		EnableDlgItem(IDC_RECIP_INSCO, FALSE);

		//And the labels to no text
		SetDlgItemText(IDC_RECIP_PERSON_TEXT, "");
		SetDlgItemText(IDC_RECIP_REFPHYS_TEXT, "");
		SetDlgItemText(IDC_RECIP_PCP_TEXT, "");

		//And the datalist
		EnableDlgItem(IDC_RECIP_INSCO_LIST, FALSE);

		return;
	}

	//These are only loaded for patients

	//1 query to run to gather it all
	CNxParamSqlArray aryParams;
	CString strSql = BeginSqlBatch();

	//Person
	AddParamStatementToSqlBatch(strSql, aryParams, "SELECT First + ' ' + Middle + ' ' + Last AS Name, Fax FROM PersonT WHERE ID = {INT}", m_nPersonID);

	//Ref Phys
	AddParamStatementToSqlBatch(strSql, aryParams, "SELECT First + ' ' + Middle + ' ' + Last + ' ' + Title AS Name, Fax "
		"FROM PatientsT INNER JOIN PersonT ON PatientsT.DefaultReferringPhyID = PersonT.ID WHERE PatientsT.PersonID = {INT}", m_nPersonID);

	//PCP
	AddParamStatementToSqlBatch(strSql, aryParams, "SELECT First + ' ' + Middle + ' ' + Last + ' ' + Title AS Name, Fax "
		"FROM PatientsT INNER JOIN PersonT ON PatientsT.PCP = PersonT.ID WHERE PatientsT.PersonID = {INT}", m_nPersonID);


	//Now load 'em all
	// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
	_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSql, aryParams);

	//Person
	CString strText = "";
	if(!prs->eof) {
		CString strName, strNum;
		strName = AdoFldString(prs, "Name", "");
		strNum = AdoFldString(prs, "Fax", "");

		//Save the values for use in saving
		m_strPatName = strName;
		m_strPatNum = strNum;

		if(strNum.IsEmpty()) {
			//No patient number, we don't want it as an option.  There is no option to ever re-enable this.
			EnableDlgItem(IDC_RECIP_PERSON, FALSE);

			strText = FormatString("%s -- <No Fax Number>", strName);

			//We cannot check the "current person" box for default, so we'll move to Other
			CheckDlgButton(IDC_RECIP_OTHER, TRUE);
		}
		else {
			//Valid patient
			strText = FormatString("%s -- %s", strName, strNum);


			//Check the "current person" box for default
			CheckDlgButton(IDC_RECIP_PERSON, TRUE);
		}
	}
	else {
		//We should fail here, if a bad person record was given
		AfxThrowNxException("Invalid person record loaded for fax recipient.  Please restart Practice and try again.");
	}

	SetDlgItemText(IDC_RECIP_PERSON_TEXT, strText);

	//Ref Phys
	prs = prs->NextRecordset(NULL);
	strText = "<No Record>";
	if(!prs->eof) {
		CString strName, strNum;
		strName = AdoFldString(prs, "Name", "");
		strNum = AdoFldString(prs, "Fax", "");

		//Save the values for use in saving
		m_strRefPhysName = strName;
		m_strRefPhysNum = strNum;

		strText = FormatString("%s -- %s", strName, strNum);
	}
	else {
		//No record.  Just disable the control, there is no capacity for it to ever be re-enabled.
		EnableDlgItem(IDC_RECIP_REFPHYS, FALSE);
	}

	SetDlgItemText(IDC_RECIP_REFPHYS_TEXT, strText);

	//PCP
	prs = prs->NextRecordset(NULL);
	strText = "<No Record>";
	if(!prs->eof) {
		CString strName, strNum;
		strName = AdoFldString(prs, "Name", "");
		strNum = AdoFldString(prs, "Fax", "");

		//Save the values for use in saving
		m_strPCPName = strName;
		m_strPCPNum = strNum;

		strText = FormatString("%s -- %s", strName, strNum);
	}
	else {
		//No record.  Just disable the control, there is no capacity for it to ever be re-enabled.
		EnableDlgItem(IDC_RECIP_PCP, FALSE);
	}

	SetDlgItemText(IDC_RECIP_PCP_TEXT, strText);


	//InsCo - Filter on the current patient's insurance records, and requery the insurance list.
	m_pInsList->WhereClause = _bstr_t(FormatString("InsuredPartyT.PatientID = %li AND Fax <> ''", m_nPersonID));
	m_pInsList->Requery();
}

void CFaxChooseRecipientDlg::OnOK() 
{
	try {
		//Output data (m_strName, m_strNumber) is based on the type selected.
		if(IsDlgButtonChecked(IDC_RECIP_PERSON)) {
			//We saved the loaded values in member strings for ease of access
			m_strName = m_strPatName;
			m_strNumber = m_strPatNum;
		}
		else if(IsDlgButtonChecked(IDC_RECIP_REFPHYS)) {
			//We saved the loaded values in member strings for ease of access
			m_strName = m_strRefPhysName;
			m_strNumber = m_strRefPhysNum;
		}
		else if(IsDlgButtonChecked(IDC_RECIP_PCP)) {
			//We saved the loaded values in member strings for ease of access
			m_strName = m_strPCPName;
			m_strNumber = m_strPCPNum;
		}
		else if(IsDlgButtonChecked(IDC_RECIP_INSCO)) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsList->GetCurSel();
			if(pRow == NULL) {
				AfxMessageBox("You must select an insurance company to proceed.");
				return;
			}

			//Otherwise, fill the name as the contact name, and the number as the fax #
			m_strName = VarString(pRow->GetValue(2), "");
			m_strNumber = VarString(pRow->GetValue(3), "");
		}
		else if(IsDlgButtonChecked(IDC_RECIP_CONTACT)) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pContactList->GetCurSel();
			if(pRow == NULL) {
				AfxMessageBox("You must select a contact to proceed.");
				return;
			}

			//Otherwise, fill the name as the contact name or company, and the number as the fax #
			m_strNumber = VarString(pRow->GetValue(6), "");

			CString strLast = VarString(pRow->GetValue(1), ""), 
				strFirst = VarString(pRow->GetValue(2), ""), 
				strMiddle = VarString(pRow->GetValue(3), ""), 
				strTitle = VarString(pRow->GetValue(4), ""), 
				strCompany = VarString(pRow->GetValue(5), "");

			if(strLast.IsEmpty() && strFirst.IsEmpty()) {
				//Use company instead of name
				m_strName = strCompany;
			}
			else {
				//Use person name
				m_strName = strFirst;
				if(!strMiddle.IsEmpty()) {
					m_strName += " " + strMiddle + " ";
				}
				else {
					m_strName += " ";
				}
				m_strName += strLast;
				if(!strTitle.IsEmpty()) {
					m_strName += " " + strTitle;
				}
			}
		}
		else if(IsDlgButtonChecked(IDC_RECIP_LOCATION)) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocList->GetCurSel();
			if(pRow == NULL) {
				AfxMessageBox("You must select a location to proceed.");
				return;
			}

			//Otherwise, fill the name with the loc name and the number with the fax number.
			m_strName = VarString(pRow->GetValue(1), "");
			m_strNumber = VarString(pRow->GetValue(2), "");
		}
		else if(IsDlgButtonChecked(IDC_RECIP_OTHER)) {
			//These ARE allowed to be empty, it's the only way to bypass this dialog.
			GetDlgItemText(IDC_RECIP_OTHER_NAME, m_strName);
			GetDlgItemText(IDC_RECIP_OTHER_NUMBER, m_strNumber);
		}

		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

void CFaxChooseRecipientDlg::OnCancel() 
{
	try {
		//No work to be done, just give up
		CDialog::OnCancel();
	} NxCatchAll("Error in OnCancel");
}

////////////////////////////////////////
//	These functions are all the button handlers for clicking
//		any of the radio buttons.
////////////////////////////////////////
void CFaxChooseRecipientDlg::OnRecipPerson() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipPerson");
}

void CFaxChooseRecipientDlg::OnRecipRefphys() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipRefphys");
}

void CFaxChooseRecipientDlg::OnRecipPcp() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipPcp");
}

void CFaxChooseRecipientDlg::OnRecipInsco() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipInsco");
}

void CFaxChooseRecipientDlg::OnRecipContact() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipContact");
}

void CFaxChooseRecipientDlg::OnRecipLocation() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipLocation");
}

void CFaxChooseRecipientDlg::OnRecipOther() 
{
	try {
		EnsureControls();

	} NxCatchAll("Error in OnRecipOther");
}

////////////////////////////////
//This function ensures that the appropriate controls are enabled/disabled
//	based on what radio buttons are checked.
void CFaxChooseRecipientDlg::EnsureControls()
{
	BOOL bEnableIns = FALSE, bEnableContact = FALSE, bEnableLoc = FALSE, bEnableOther = FALSE;

	if(IsDlgButtonChecked(IDC_RECIP_PERSON)) {
		//Nothing
	}
	else if(IsDlgButtonChecked(IDC_RECIP_REFPHYS)) {
		//Nothing
	}
	else if(IsDlgButtonChecked(IDC_RECIP_PCP)) {
		//Nothing
	}
	else if(IsDlgButtonChecked(IDC_RECIP_INSCO)) {
		bEnableIns = TRUE;
	}
	else if(IsDlgButtonChecked(IDC_RECIP_CONTACT)) {
		bEnableContact = TRUE;
	}
	else if(IsDlgButtonChecked(IDC_RECIP_LOCATION)) {
		bEnableLoc = TRUE;
	}
	else if(IsDlgButtonChecked(IDC_RECIP_OTHER)) {
		bEnableOther = TRUE;
	}

	EnableDlgItem(IDC_RECIP_INSCO_LIST, bEnableIns);
	EnableDlgItem(IDC_RECIP_CONTACT_LIST, bEnableContact);
	EnableDlgItem(IDC_RECIP_LOCATION_LIST, bEnableLoc);
	EnableDlgItem(IDC_RECIP_OTHER_NAME, bEnableOther);
	EnableDlgItem(IDC_RECIP_OTHER_NUMBER, bEnableOther);
}

BEGIN_EVENTSINK_MAP(CFaxChooseRecipientDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFaxChooseRecipientDlg)
	ON_EVENT(CFaxChooseRecipientDlg, IDC_RECIP_INSCO_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRecipInscoList, VTS_I2)
	ON_EVENT(CFaxChooseRecipientDlg, IDC_RECIP_CONTACT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRecipContactList, VTS_I2)
	ON_EVENT(CFaxChooseRecipientDlg, IDC_RECIP_LOCATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRecipLocationList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CFaxChooseRecipientDlg::OnRequeryFinishedRecipInscoList(short nFlags) 
{
	try {
		if(m_pInsList->GetRowCount() == 0) {
			//Disable the radio button if there are no records.
			EnableDlgItem(IDC_RECIP_INSCO, FALSE);
		}

	} NxCatchAll("Error in OnRequeryFinishedRecipInscoList");
}

void CFaxChooseRecipientDlg::OnRequeryFinishedRecipContactList(short nFlags) 
{
	try {
		if(m_pContactList->GetRowCount() == 0) {
			//Disable the radio button if there are no records.
			EnableDlgItem(IDC_RECIP_CONTACT, FALSE);
		}

	} NxCatchAll("Error in OnRequeryFinishedRecipContactList");
}

void CFaxChooseRecipientDlg::OnRequeryFinishedRecipLocationList(short nFlags) 
{
	try {
		if(m_pLocList->GetRowCount() == 0) {
			//Disable the radio button if there are no records.
			EnableDlgItem(IDC_RECIP_LOCATION, FALSE);
		}

	} NxCatchAll("Error in OnRequeryFinishedRecipLocationList");
}
