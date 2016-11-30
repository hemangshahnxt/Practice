// HL7SelectPatientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7SelectPatientDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHL7SelectPatientDlg dialog


CHL7SelectPatientDlg::CHL7SelectPatientDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7SelectPatientDlg::IDD, pParent)
{
	m_dtBirthDate.SetStatus(COleDateTime::invalid);
	m_bAllowCreateNew = false;
	m_bWillOverwrite = false;
	m_bWillLink = true;
	m_nPreselectedPersonID = -1;
	// (s.dhole 2013-08-30 09:41) - PLID 54572
	m_nGender =-1;
	//{{AFX_DATA_INIT(CHL7SelectPatientDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHL7SelectPatientDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7SelectPatientDlg)
	DDX_Control(pDX, IDC_CREATE_NEW, m_btnCreateNew);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_TOP_CAPTION, m_nxstaticTopCaption);
	DDX_Control(pDX, IDC_HL7_FIRST, m_nxstaticHl7First);
	DDX_Control(pDX, IDC_HL7_MIDDLE, m_nxstaticHl7Middle);
	DDX_Control(pDX, IDC_HL7_LAST, m_nxstaticHl7Last);
	DDX_Control(pDX, IDC_HL7_BIRTHDATE, m_nxstaticHl7Birthdate);
	DDX_Control(pDX, IDC_HL7_SSN, m_nxstaticHl7Ssn);
	DDX_Control(pDX, IDC_HL7_GROUP, m_btnHl7Group);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7SelectPatientDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7SelectPatientDlg)
	ON_BN_CLICKED(IDC_CREATE_NEW, OnCreateNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7SelectPatientDlg message handlers

BOOL CHL7SelectPatientDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 10:07) - PLID 29953 - added nxiconbuttons for modernization
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnCreateNew.AutoSet(NXB_NEW);
	
	CString strTopCaption;
	//TES 7/12/2007 - PLID 26642 - Added a Create New button to this dialog, which should only be available if
	// m_bAllowCreateNew is true.
	//TES 9/23/2008 - PLID 21093 - Add a warning if this patient will not be persistently linked.
	strTopCaption.Format("This %s patient could not be linked to a patient in Practice.  "
		"Please select a Practice patient to link this %s patient to%s:%s",
		m_strGroupName, m_strGroupName, 
		m_bAllowCreateNew ? ", or select 'Create New' to create a new record in Practice for this patient" : "",
		m_bWillLink ? "" : "\r\n\r\nNOTE: This message did not contain a patient identifier.  Therefore, if any future messages "
		"come in for this patient, you will receive this prompt again to link the message to a patient in Practice.");

	if(!m_bAllowCreateNew) {
		GetDlgItem(IDC_CREATE_NEW)->ShowWindow(SW_HIDE);
	}
	SetDlgItemText(IDC_TOP_CAPTION, strTopCaption);
	CString strGroup;
	strGroup.Format("%s Patient", m_strGroupName);
	SetDlgItemText(IDC_HL7_GROUP, strGroup);

	SetDlgItemText(IDC_HL7_FIRST, m_strFirstName);
	SetDlgItemText(IDC_HL7_MIDDLE, m_strMiddleName);
	SetDlgItemText(IDC_HL7_LAST, m_strLastName);
	// (s.dhole 2013-08-13 14:33) - PLID 54572  added gender, phone and sex
	CString strPatientGender; 
	switch(m_nGender) {
		case 0:
			strPatientGender = "Other";
			break;
		case 1:
			strPatientGender = "Male";
			break;
		case 2:
			strPatientGender = "Female";
			break;	
		default:
			strPatientGender="";
			break;
	}
	SetDlgItemText(IDC_HL7_GENDER, strPatientGender);
	SetDlgItemText(IDC_HL7_PHONE, m_strPhone);
	SetDlgItemText(IDC_HL7_ZIP, m_strZip);

	if(m_dtBirthDate.GetStatus() == COleDateTime::valid) {
		SetDlgItemText(IDC_HL7_BIRTHDATE, FormatDateTimeForInterface(m_dtBirthDate, NULL, dtoDate, false));
	}
	//(c.copits 2010-10-19) PLID 40496 - When PID-7 is blank, don't say "Static."
	else {
		SetDlgItemText(IDC_HL7_BIRTHDATE, "");
	}
	SetDlgItemText(IDC_HL7_SSN, m_strSSN);

	m_pSelectedPatient = BindNxDataListCtrl(this, IDC_SELECTED_PRACTICE_PATIENT, GetRemoteData(), true);

	//TES 2/24/2012 - PLID 48395 - If we were given a person ID to preselect, do so.
	if(m_nPreselectedPersonID != -1) {
		long nSel = m_pSelectedPatient->TrySetSelByColumn(0, m_nPreselectedPersonID);
		//TES 2/24/2012 - PLID 48395 - The list may still be requerying, so set the combo box text to show that we've preselected this
		// patient (we know the name matches, otherwise we wouldn't have been told to preselect it.
		if(nSel == NXDATALISTLib::sriNoRowYet_WillFireEvent) {
			m_pSelectedPatient->ComboBoxText = _bstr_t(m_strLastName + ", " + m_strFirstName + " " + m_strMiddleName);
		}
		SetDlgItemText(IDC_PRESELECT_WARNING, "NOTE: Only one patient in your database matched the name given by " + m_strGroupName + ".  "
			"That patient has been automatically selected in the list.  Please ensure that this is the correct patient record before clicking OK.");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7SelectPatientDlg::OnOK() 
{
	if(m_pSelectedPatient->CurSel == -1) {
		//TES 2/24/2012 - PLID 48395 - We may have preselected a patient that just hasn't come up in the requery yet.
		if(m_pSelectedPatient->IsComboBoxTextInUse) {
			m_nPersonID = m_nPreselectedPersonID;
		}
		else {
			MsgBox("Please select a patient.");
		}
		return;
	}
	else {
		m_nPersonID = VarLong(m_pSelectedPatient->GetValue(m_pSelectedPatient->CurSel, 0));
	}
	if(m_bWillOverwrite) {
		//TES 7/12/2007 - PLID 26642 - If they screw this up it will basically write a different patient's demographics
		// over top of an existing patient, so warn them.
		if(IDYES != MsgBox(MB_YESNO, "Warning: Linking the incoming message to an existing patient will cause the existing patient's information, "
			"including their name, to be PERMANENTLY overwritten with information from the incoming message.  "
			"Are you sure you wish to do this?")) {
			return;
		}
	}

	CNxDialog::OnOK();
}

void CHL7SelectPatientDlg::OnCancel() 
{
	if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to cancel?  The HL7 action will not be committed.")) return;
	CNxDialog::OnCancel();
}

void CHL7SelectPatientDlg::OnCreateNew() 
{
	//TES 7/12/2007 - PLID 26642 - They shouldn't be able to click this button if we're not allowed to create new!
	ASSERT(m_bAllowCreateNew);
	//TES 7/12/2007 - PLID 26642 - Set m_nPersonID to -2, which is the sentinel indicating that the caller needs to 
	// create a new patient record.
	m_nPersonID = -2;
	CNxDialog::OnOK();
}
