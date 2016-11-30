// ResponsiblePartyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GlobalDataUtils.h"
#include "ResponsiblePartyDlg.h"
#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;
using namespace NXDATALISTLib;

enum EResponsiblePartyColumns {
	erpRespID,
	erpRelation,
	erpName
};

/////////////////////////////////////////////////////////////////////////////
// CResponsiblePartyDlg dialog


CResponsiblePartyDlg::CResponsiblePartyDlg(CWnd* pParent)
	: CNxDialog(CResponsiblePartyDlg::IDD, pParent) // (d.moore 2007-04-25 10:20) - PLID 25754 - Changed inheritence from CDialog to CNxDialog.
{
	//{{AFX_DATA_INIT(CResponsiblePartyDlg)
		m_ID = -1;
	//}}AFX_DATA_INIT
}

// (d.moore) - PLID 25754
void CResponsiblePartyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX); // (d.moore 2007-04-25 10:20) - PLID 25754 - Changed CDialog to CNxDialog.
	//{{AFX_DATA_MAP(CResponsiblePartyDlg)
	DDX_Control(pDX, IDC_RESP_PRIMARY, m_btnPrimaryParty);
	DDX_Control(pDX, IDC_PATIENT_BKG, m_bkg);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeditAddress1Box);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeditAddress2Box);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeditZipBox);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeditCityBox);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeditStateBox);
	DDX_Control(pDX, IDC_EMPLOYER_SCHOOL_BOX, m_nxeditEmployerSchoolBox);
	DDX_Control(pDX, IDC_PHONE_BOX, m_nxeditPhoneBox);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_COPY_PAT_INFO, m_btnCopyPatInfo);
	DDX_Control(pDX, IDC_MAKE_NEW_RESP_PARTY, m_btnMakeNewRespParty);
	DDX_Control(pDX, IDC_DELETE_RESP_PARTY, m_btnDeleteRespParty);
	DDX_Control(pDX, IDC_MAIDEN_NAME_BOX, m_nxeditMaidenName);
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResponsiblePartyDlg, CNxDialog) // (d.moore 2007-04-25 10:20) - PLID 25754 - Changed CDialog to CNxDialog.
	//{{AFX_MSG_MAP(CResponsiblePartyDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MAKE_NEW_RESP_PARTY, OnMakeNewRespParty)
	ON_BN_CLICKED(IDC_COPY_PAT_INFO, OnCopyPatInfo)
	ON_BN_CLICKED(IDC_DELETE_RESP_PARTY, OnDeleteRespParty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResponsiblePartyDlg message handlers

BOOL CResponsiblePartyDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (d.moore 2007-04-25 10:20) - PLID 25754 - Changed CDialog to CNxDialog.

	try {
		// (d.singleton 2012-06-18 13:12) - PLID 51029 add text limit to address 1 and 2
		m_nxeditAddress1Box.SetLimitText(150);
		m_nxeditAddress2Box.SetLimitText(150);
		// (d.singleton 2013-11-12 17:02) - PLID 59442 - need to have a maiden name text box for the resp party dialog.  it will only show on the dialog if the relationship is set to mother.
		m_nxeditMaidenName.SetLimitText(194);

		// (c.haag 2008-04-30 15:50) - PLID 29847 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCopyPatInfo.AutoSet(NXB_MODIFY);
		m_btnMakeNewRespParty.AutoSet(NXB_NEW);
		m_btnDeleteRespParty.AutoSet(NXB_DELETE);

		m_bReady = false;

		m_brush.CreateSolidBrush(PaletteColor(m_color));

		m_bkg.SetColor(m_color);

		// (d.moore 2007-04-25 10:32) - PLID 25754 - The following three function calls all
		//  were modified when changing this classes inheritence from CDialog to CNxDialog.
		m_nxtBirthDate = GetDlgItemUnknown(IDC_BIRTH_DATE_BOX);
		m_RelateCombo = BindNxDataListCtrl(IDC_RELATE_COMBO,GetRemoteData(),FALSE);
		m_GenderCombo = BindNxDataListCtrl(IDC_GENDER_LIST,GetRemoteData(),FALSE);

		IColumnSettingsPtr(m_RelateCombo->GetColumn(m_RelateCombo->InsertColumn(0,"Relation","Relation",-1,csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;

		IRowSettingsPtr pRow;
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Self");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Child");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Spouse");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Other");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Unknown");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Employee");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Organ Donor");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Cadaver Donor");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Life Partner");
		m_RelateCombo->AddRow(pRow);

		// (j.jones 2011-06-15 17:00) - PLID 40959 - the following are no longer valid entries in our system,
		// and no longer exist in data
		// (j.jones 2012-01-26 08:42) - PLID 47753 - we added these back in because this field is never used
		// in an ANSI export, so it doesn't have to follow the ANSI rules
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Grandparent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Grandchild");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Nephew Or Niece");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Adopted Child");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Foster Child");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Ward");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Stepchild");
		m_RelateCombo->AddRow(pRow);	
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Handicapped Dependent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Sponsored Dependent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Dependent of a Minor Dependent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Significant Other");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Mother");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Father");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Other Adult");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Emancipated Minor");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Injured Plaintiff");   // (s.dhole 2010-08-31 15:13) - PLID 40114 All our relationship lists say "Injured Plantiff" instead of "Injured Plaintiff"
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Child Where Insured Has No Financial Responsibility");
		m_RelateCombo->AddRow(pRow);

		//setup gender combo
		IColumnSettingsPtr(m_GenderCombo->GetColumn(m_GenderCombo->InsertColumn(0, _T("Gender"), _T("Gender"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		pRow = m_GenderCombo->GetRow(-1);
		_variant_t var = _bstr_t("");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("Male");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("Female");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);

		EnableItems(FALSE);

		// (d.moore 2007-04-25 10:32) - PLID 25754 - Modified function call when changing this classes inheritence from CDialog to CNxDialog.
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_RESP, GetRemoteData(), false);
		CString strWhere;
		strWhere.Format("PatientID = %li", GetActivePatientID());
		m_dlList->WhereClause = (LPCTSTR)strWhere;
		m_dlList->Requery();

		/*
		_RecordsetPtr rs = CreateRecordset("SELECT PatientID, PersonID FROM ResponsiblePartyT WHERE PatientID = %li",GetActivePatientID());

		if(!rs->eof) {
		m_ID = AdoFldLong(rs, "PersonID");
		}
		else {
		m_ID = -1;
		}
		rs->Close();
		*/

		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);


		// (j.gruber 2009-10-06 17:02) - PLID 35826 - reset here in case they changed the preference
		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
		} else {
			ChangeZOrder(IDC_ZIP_BOX, IDC_ADDRESS2_BOX);
		}

		//Load(); Load on requery finished.
	}NxCatchAll(__FUNCTION__);

	return false;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CResponsiblePartyDlg::OnOK() 
{
	if(!Save())
		return;
	
	// (d.moore 2007-04-25 10:37) - PLID 25754 - The call to CDialog::OnOK() should
	//  remain unchanged since CNxDialog intentionally does not pass this call along
	//  to the base class or handle it.
	CDialog::OnOK();
}

void CResponsiblePartyDlg::OnMakeNewRespParty() 
{
	try {
		
		// (a.walling 2006-10-13 08:56) - PLID 16059 - This is unnecessary now, just make a new blank party.
		/*
		if(m_ID != -1) {

			//we already have one!

			if(IDNO == MessageBox("You already have a responsible party for this patient.\n"
				"Do you wish to replace this responsible party with a new one?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
			else {
				
				Delete();
			}
		}
		*/
			
		//make a new responsible party

		// (a.walling 2006-10-12 17:58) - PLID 16059 - Let's not save to data if they cancel
		/*
		m_ID = NewNumber("PersonT","ID");
		
		ExecuteSql("INSERT INTO PersonT (ID, Last) VALUES (%li, '<New Responsible Party>')",m_ID);
		ExecuteSql("INSERT INTO ResponsiblePartyT (PersonID, PatientID) VALUES (%li, %li)",m_ID, GetActivePatientID());
		
		m_dlList->Requery();
		m_dlList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		*/

		if (!Save())
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetNewRow();

		pRow->PutValue(erpRespID, (long)-1);
		pRow->PutValue(erpRelation, "");
		pRow->PutValue(erpName, "<New Responsible Party>");
		m_dlList->AddRowSorted(pRow, NULL);
		m_dlList->PutCurSel(pRow);

		long nCount = m_dlList->GetRowCount();
		if (nCount > 1) {
			CString str;
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index
			str.Format("Responsible Party - %li of %li", (long)(pRow->CalcRowNumber() + 1), nCount);
			SetWindowText(str);
		}

		// (a.walling 2006-10-17 17:22) - PLID 16059 - There must be at least ONE Primary resp party.
		bool bPrimary = ReturnsRecords("SELECT PrimaryRespPartyID FROM PatientsT WHERE PersonID = %li AND PrimaryRespPartyID IS NULL", GetActivePatientID()) == TRUE;
		
		m_ID = -1;
		GetDlgItem(IDC_LIST_RESP)->EnableWindow(TRUE);
		EnableItems(TRUE);
		ClearInfo();

		CheckDlgButton(IDC_RESP_PRIMARY, bPrimary);
		if (bPrimary) {
			// (a.walling 2006-10-18 13:06) - PLID 16059 - No one can really unselect a primary,
			//		just delete or make another one.
			GetDlgItem(IDC_RESP_PRIMARY)->EnableWindow(FALSE);
		}
		m_bReady = true;

		if (m_dlList->GetRowCount() == 2) {
			// only remind them if they make a second resp. party, not third or etc.
			long nMultiple = GetRemotePropertyInt("StatementsPrintMultipleParties", 0, 0, "<None>", true);
			if (nMultiple == 0) {
				DontShowMeAgain(this, "There is a preference to print a statement for each of a patient's responsible parties.\r\n\r\nTo enable this, click on the Activities Menu, and go to Configure Patient Statement. Check the box labeled 'Print one statement for each responsible party.'", "RemindMultiRespPartyStatements", "Statements for Multiple Responsible Parties", false, false);
			}
		}

	}NxCatchAll("Error making new responsible party.");
}

void CResponsiblePartyDlg::OnCopyPatInfo() 
{
	try {

		if(m_ID != -1) {

			//we already have one!
			// (a.walling 2006-10-12 17:01) - PLID 16059 - Clarify this message box...
			if(IDNO == MessageBox("You have already selected a responsible party.\n"
				"Do you wish to replace this responsible party's info with the patient's info?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
			else {
				// (a.walling 2006-10-13 09:08) - PLID 16059 - No need to delete; just update.
				//Delete();
			}
		}
		else {
			//make a new responsible party
			m_ID = NewNumber("PersonT","ID");
			ExecuteSql("INSERT INTO PersonT (ID) VALUES (%li)",m_ID);
			ExecuteSql("INSERT INTO ResponsiblePartyT (PersonID, PatientID, RelationToPatient) VALUES (%li, %li, 'Self')",m_ID, GetActivePatientID());
			//ExecuteSql("UPDATE PatientsT SET ResponsiblePartyID = %li WHERE PersonID = %li",m_ID,GetActivePatientID());
		}

		GetDlgItem(IDC_LIST_RESP)->EnableWindow(TRUE);
		EnableItems(TRUE);

		//copy patient info.
		ExecuteSql("UPDATE PersonT SET PersonT.[First] = PersonT_1.[First], PersonT.Middle = PersonT_1.Middle, PersonT.[Last] = PersonT_1.[Last], PersonT.Address1 = PersonT_1.Address1, PersonT.Address2 = PersonT_1.Address2, PersonT.City = PersonT_1.City, PersonT.State = PersonT_1.State, PersonT.Zip = PersonT_1.Zip, PersonT.HomePhone = PersonT_1.HomePhone, PersonT.BirthDate = PersonT_1.BirthDate, PersonT.Gender = PersonT_1.Gender "
					"FROM PatientsT RIGHT JOIN ResponsiblePartyT ON PatientsT.PersonID = ResponsiblePartyT.PatientID INNER JOIN (SELECT * FROM PersonT) AS PersonT_1 ON PatientsT.PersonID = PersonT_1.ID INNER JOIN PersonT ON ResponsiblePartyT.PersonID = PersonT.ID "
					"WHERE ResponsiblePartyT.PersonID = %li",m_ID);

		ExecuteSql("UPDATE ResponsiblePartyT SET ResponsiblePartyT.Employer = PersonT.Company "
					"FROM PersonT RIGHT JOIN ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PatientID "
					"WHERE ResponsiblePartyT.PersonID = %li",m_ID);

		// (a.walling 2006-10-17 17:22) - PLID 16059 - There must be at least ONE Primary resp party.
		bool bPrimary = ReturnsRecords("SELECT PrimaryRespPartyID FROM PatientsT WHERE PersonID = %li AND PrimaryRespPartyID IS NULL", GetActivePatientID()) == TRUE;
		if (bPrimary) {
			// (a.walling 2006-10-18 10:13) - PLID 16059 - Only check, don't uncheck
			CheckDlgButton(IDC_RESP_PRIMARY, TRUE);
		}

		m_bReady = false;
		m_dlList->Requery();
		m_dlList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}NxCatchAll("Error copying patient information.");
}

void CResponsiblePartyDlg::OnDeleteRespParty() 
{
	try {

		if (m_ID != -1) {
			if(IDNO == MessageBox("Are you sure you wish to delete this responsible party?","Practice",MB_ICONEXCLAMATION|MB_YESNO))
				return;
							
			Delete();
		}

		// (a.walling 2006-10-12 17:01) - PLID 16059 - There may be multiple parties, so just requery.
		m_bReady = false;

		ClearInfo();
		EnableItems(FALSE);
		m_ID = -1;

		m_dlList->Requery();
		//m_dlList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		
	}NxCatchAll("Error deleting responsible party.");
}

void CResponsiblePartyDlg::Load()
{
	try {

		if(m_dlList->GetRowCount() <= 0) {

			EnableItems(FALSE);
			ClearInfo();
		}
		else {

			EnableItems(TRUE);

			// (d.singleton 2013-11-12 17:23) - PLID 59442 - added maiden name		
			_RecordsetPtr rs = CreateRecordset("SELECT First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, Employer, RelationToPatient, Gender, PrimaryRespPartyID, MaidenName "
									"FROM PersonT INNER JOIN ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PersonID "
									"LEFT JOIN PatientsT ON ResponsiblePartyT.PatientID = PatientsT.PersonID WHERE ID = %li", m_ID);

			if(!rs->eof) {
				bool bPrimary;

				if (AdoFldLong(rs, "PrimaryRespPartyID", -1) == m_ID) {
					bPrimary = true;
				}
				else
				{
					// (a.walling 2006-10-18 10:58) - PLID 16059
					// if somehow we are loading a patient who does not have a primary resp party set,
					// set it for them!
					bPrimary = ReturnsRecords("SELECT PrimaryRespPartyID FROM PatientsT WHERE PersonID = %li AND PrimaryRespPartyID IS NULL", GetActivePatientID()) == TRUE;
				}

				SetDlgItemText(IDC_FIRST_NAME_BOX,AdoFldString(rs, "First",""));
				SetDlgItemText(IDC_MIDDLE_NAME_BOX,AdoFldString(rs, "Middle",""));
				SetDlgItemText(IDC_LAST_NAME_BOX,AdoFldString(rs, "Last",""));
				SetDlgItemText(IDC_ADDRESS1_BOX,AdoFldString(rs, "Address1",""));
				SetDlgItemText(IDC_ADDRESS2_BOX,AdoFldString(rs, "Address2",""));
				SetDlgItemText(IDC_CITY_BOX,AdoFldString(rs, "City",""));
				SetDlgItemText(IDC_STATE_BOX,AdoFldString(rs, "State",""));
				SetDlgItemText(IDC_ZIP_BOX,AdoFldString(rs, "Zip",""));
				// (d.singleton 2013-11-12 17:23) - PLID 59442 - added maiden name
				SetDlgItemText(IDC_MAIDEN_NAME_BOX, AdoFldString(rs, "MaidenName", ""));
				CheckDlgButton(IDC_RESP_PRIMARY, bPrimary);
				if (bPrimary) {
					// (a.walling 2006-10-18 13:03) - PLID 16059
					// we can't really un-select the primary checkbox, just select it on another resp. party
					// so disable this box for now.
					GetDlgItem(IDC_RESP_PRIMARY)->EnableWindow(FALSE);
				}

				COleDateTime date;
				_variant_t var = rs->Fields->Item["Birthdate"]->Value;
				if(var.vt == VT_DATE) {
					m_nxtBirthDate->SetDateTime(VarDateTime(var));
				}
				else {
					m_nxtBirthDate->Clear();
				}

				SetDlgItemText(IDC_PHONE_BOX,AdoFldString(rs, "HomePhone",""));
				SetDlgItemText(IDC_EMPLOYER_SCHOOL_BOX,AdoFldString(rs, "Employer",""));
				m_RelateCombo->SetSelByColumn(0,rs->Fields->Item["RelationToPatient"]->Value);

				// (d.singleton 2013-11-12 18:22) - PLID 59442 - need to have a maiden name text box for the resp party dialog.  it will only show on the dialog if the relationship is set to mother.
				EnableMaidenNameBox(m_RelateCombo->GetCurSel());

				var = rs->Fields->Item["Gender"]->Value;
				if (var.vt != VT_NULL)
				{	
					if (VarByte(var,0) == 1)
						m_GenderCombo->CurSel = 1;
					else if (VarByte(var,0) == 2)
						m_GenderCombo->CurSel = 2;
					else
						m_GenderCombo->CurSel = 0;
				}
				else
					m_GenderCombo->CurSel = 0;		

			}
			rs->Close();
		}

		//set the focus to the first field
		CWnd* pWnd = GetDlgItem(IDC_FIRST_NAME_BOX);

		if(pWnd)
			pWnd->SetFocus();


	}NxCatchAll("Error loading responsible party.");
}

BOOL CResponsiblePartyDlg::Save()
{
	try {

		// (a.walling 2006-10-17 11:24) - PLID 16059 - Exit out of here if the list is
		//		requerying or there are no items in the list.
		if (m_dlList->IsRequerying())
			return true;
		long nCount = m_dlList->GetRowCount();
		if (nCount <= 0) {
			return true;
		}

		bool bNewRecord = false;
		long tempID = m_ID;

		if ( (tempID == -1) ) 
		{
			if (m_bReady) // this is a new record!
			{
				tempID = NewNumber("PersonT","ID");
				bNewRecord = true;
			}
		}
		// (d.singleton 2013-11-12 17:23) - PLID 59442 - added maiden name
		CString strFirst, strMiddle, strLast, strAddress1, strAddress2, strCity, strState, strZip, strBirthdate, strHomephone, strEmployer, strMaidenName;

		GetDlgItemText(IDC_FIRST_NAME_BOX,strFirst);
		GetDlgItemText(IDC_MIDDLE_NAME_BOX,strMiddle);
		GetDlgItemText(IDC_LAST_NAME_BOX,strLast);
		GetDlgItemText(IDC_ADDRESS1_BOX,strAddress1);
		GetDlgItemText(IDC_ADDRESS2_BOX,strAddress2);
		GetDlgItemText(IDC_CITY_BOX,strCity);
		GetDlgItemText(IDC_STATE_BOX,strState);
		GetDlgItemText(IDC_ZIP_BOX,strZip);
		GetDlgItemText(IDC_PHONE_BOX,strHomephone);
		GetDlgItemText(IDC_EMPLOYER_SCHOOL_BOX,strEmployer);		

		// (a.walling 2006-10-17 17:14) - PLID 16059 - Primary responsible party for merging to word
		bool bPrimary = (((CButton*)GetDlgItem(IDC_RESP_PRIMARY))->GetCheck() == TRUE);

		if ( (strFirst.IsEmpty() || strLast.IsEmpty()) ) {
			MessageBox("You must enter at least a first and last name.");
			return false;
		}

		if (m_nxtBirthDate->GetStatus() != 1)
				strBirthdate = "NULL";
		else {
			COleDateTime dt, dttemp;
			dttemp.ParseDateTime("01/01/1800");
			dt = m_nxtBirthDate->GetDateTime();
			if(dt.m_status!=COleDateTime::invalid && dt.m_dt >= dttemp.m_dt) {
				//if they entered a birthdate in the future, warn them
				if(dt >= COleDateTime::GetCurrentTime()) {
					if(AfxMessageBox("You have entered a birthdate that is in the future of the current date.  Would you like to correct this?", MB_YESNO) == IDYES) {
						GetDlgItem(IDC_BIRTH_DATE_BOX)->SetFocus();
						return false;
					}
				}
				strBirthdate = "'" + _Q(FormatDateTimeForSql(dt)) + "'";
			}
			else {
				strBirthdate = "NULL";
				m_nxtBirthDate->Clear();
				AfxMessageBox("You have entered an invalid birthdate. Please fix it before exiting.");
				GetDlgItem(IDC_BIRTH_DATE_BOX)->SetFocus();
				return false;
			}
		}

		CString strRelate = "Other";

		if(m_RelateCombo->CurSel != -1)
			strRelate = CString(m_RelateCombo->GetValue(m_RelateCombo->CurSel,0).bstrVal);

		// (d.singleton 2013-11-12 17:23) - PLID 59442 - added maiden name, only save it mother is active relationship
		if(strRelate.CompareNoCase("Mother") == 0) {
			GetDlgItemText(IDC_MAIDEN_NAME_BOX, strMaidenName);
		}
		else {
			strMaidenName = "";
		}

		long Gender = m_GenderCombo->GetCurSel();
		if (Gender == -1) Gender = 0; // (a.walling 2006-10-13 10:29) - PLID 23034 - Fix SQL exception here

		CString strSql = BeginSqlBatch();

		if (bNewRecord)
		{
			AddStatementToSqlBatch(strSql, "INSERT INTO PersonT (ID) VALUES (%li)", tempID);
			AddStatementToSqlBatch(strSql, "INSERT INTO ResponsiblePartyT (PersonID, PatientID) VALUES (%li, %li)", tempID, GetActivePatientID());
		}

		// (a.walling 2006-10-18 09:06) - PLID 16059 - If we are not setting a primary, make sure there is one set already.
		//												Otherwise silently set it for the user.
		if (!bPrimary) {
			// this returns true only if the PrimaryRespPartyID is null
			bPrimary = ReturnsRecords("SELECT PrimaryRespPartyID FROM PatientsT WHERE PersonID = %li AND PrimaryRespPartyID IS NULL", GetActivePatientID()) == TRUE;
		}

		// (a.walling 2006-10-17 17:16) - PLID 16059 - Then set it here
		// (d.singleton 2013-11-12 17:23) - PLID 59442 - added maiden name
		AddStatementToSqlBatch(strSql, "UPDATE ResponsiblePartyT SET RelationToPatient = '%s', Employer = '%s' WHERE PersonID = %li",_Q(strRelate),_Q(strEmployer), tempID);
		AddStatementToSqlBatch(strSql, "UPDATE PersonT SET First = '%s', Middle = '%s', Last = '%s', Address1 = '%s', Address2 = '%s', "
			"City = '%s', State = '%s', Zip = '%s', Birthdate = %s, Homephone = '%s', Gender = %li, MaidenName = '%s' WHERE ID = %li",
			_Q(strFirst),_Q(strMiddle),_Q(strLast),_Q(strAddress1),_Q(strAddress2),_Q(strCity),_Q(strState),_Q(strZip),strBirthdate,
			_Q(strHomephone),Gender,_Q(strMaidenName),tempID);

		ExecuteSqlBatch(strSql);

		if (bPrimary) {
			ExecuteSql("UPDATE PatientsT SET PrimaryRespPartyID = %li WHERE PersonID = %li", tempID, GetActivePatientID());
		}

		m_ID = tempID;

		if (bNewRecord) {
			m_bReady = false;
			m_dlList->Requery();
			m_dlList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->FindByColumn(erpRespID, tempID, NULL, false);
		if (pRow) {
			CString strName;
			strName.Format("%s, %s %s", strLast, strFirst, strMiddle);
			pRow->PutValue(erpName, (_bstr_t)strName);
			pRow->PutValue(erpRelation, (_bstr_t)strRelate);
		}
		else {
			m_bReady = false;
			m_dlList->Requery();
			m_dlList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		return TRUE;

	}NxCatchAll("Error saving responsible party.");

	return FALSE;
}

void CResponsiblePartyDlg::EnableItems(BOOL bEnabled)
{
	GetDlgItem(IDC_FIRST_NAME_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_MIDDLE_NAME_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_LAST_NAME_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ADDRESS1_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ADDRESS2_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CITY_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_STATE_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ZIP_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EMPLOYER_SCHOOL_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_BIRTH_DATE_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PHONE_BOX)->EnableWindow(bEnabled);
	GetDlgItem(IDC_RELATE_COMBO)->EnableWindow(bEnabled);
	GetDlgItem(IDC_GENDER_LIST)->EnableWindow(bEnabled);
	GetDlgItem(IDC_DELETE_RESP_PARTY)->EnableWindow(bEnabled);
	GetDlgItem(IDC_RESP_PRIMARY)->EnableWindow(bEnabled);
	GetDlgItem(IDC_LIST_RESP)->EnableWindow(bEnabled);
	// (d.singleton 2013-11-12 17:10) - PLID 59442 - need to have a maiden name text box for the resp party dialog.  it will only show on the dialog if the relationship is set to mother.
	long nResp = m_RelateCombo->GetCurSel();
	CString strRelationship = "";
	if(nResp != -1) {
		strRelationship = VarString(m_RelateCombo->GetValue(nResp, 0), "");
	}
	if(strRelationship.CompareNoCase("Mother") == 0) {
		//if mother is selected then show the edit box, but only enable if told
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->EnableWindow(bEnabled);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->EnableWindow(bEnabled);
	}
	else {
		//if mother is not selected then disable and hide
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->EnableWindow(FALSE);
	}
}

HBRUSH CResponsiblePartyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor); // (d.moore 2007-04-25 10:20) - PLID 25754 - Changed CDialog to CNxDialog.
	
	if ( (pWnd->GetDlgCtrlID() == IDC_STATIC) || (pWnd->GetDlgCtrlID() == IDC_RESP_PRIMARY) )
	{
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(m_color));
		return m_brush;
	}

	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CResponsiblePartyDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	CString str;

	switch (HIWORD(wParam))
	{	case EN_CHANGE:
			switch (nID = LOWORD(wParam))
			{
				case IDC_MIDDLE_NAME_BOX:
					// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
					// for middle name boxes
					if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
						Capitalize(nID);
					}
					break;
				case IDC_FIRST_NAME_BOX:
				case IDC_LAST_NAME_BOX:
				// (d.singleton 2013-11-12 17:23) - PLID 59442 - need to have a maiden name text box, do same as other name boxes
				case IDC_MAIDEN_NAME_BOX:
				case IDC_ADDRESS1_BOX:
				case IDC_ADDRESS2_BOX:
				case IDC_CITY_BOX:
				case IDC_STATE_BOX:
					Capitalize (nID);
					break;
				case IDC_ZIP_BOX:
					// (d.moore 2007-04-25 10:20) - PLID 25754 - Add capitalization to zip code field.
					//  Canadian (and some European) postal codes are alpha-numeric and must be in caps.
					CapitalizeAll(IDC_ZIP_BOX);
					//FormatItem (nID, "#####-nnnn");
					break;
				case IDC_PHONE_BOX:
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							FormatItem (nID, m_strPhoneFormat);
						}
					}
					break;
			}
			break;
		break;
		case EN_KILLFOCUS:
		switch (LOWORD(wParam))
			{	case IDC_ZIP_BOX:
				// (j.gruber 2009-10-07 17:39) - PLID 35826 - updated for city lookup
				{	// don't check for -1, that just means it hasn't been saved yet.
					if (!m_bLookupByCity) {
						CString city, 
								state,
								tempCity,
								tempState,
								value;
						GetDlgItemText(IDC_ZIP_BOX, value);
						GetDlgItemText(IDC_CITY_BOX, tempCity);
						GetDlgItemText(IDC_STATE_BOX, tempState);
						tempCity.TrimRight();
						tempState.TrimRight();
						// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
						if(!tempCity.IsEmpty() || !tempState.IsEmpty()) {
							MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
							if(AfxMessageBox("You have changed the postal code but the city or state already have data in them.  Would you like to overwrite "
								"this data with that of the new postal code?", MB_YESNO) == IDYES)
							{
								//Just treat them as empty and the code below will fill them.
								tempCity.Empty();
								tempState.Empty();
							}
						}
						if(tempCity == "" || tempState == "") {
							GetZipInfo(value, &city, &state);
							// (s.tullis 2013-10-21 10:17) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
							if(city == "" && state == ""){
								CString str;
								str = value.Left(5);// Get the 5 digit zip code
								GetZipInfo(str, &city, &state);
								// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
								//digit zipcode in the locations tab of Administrator, it looks
								//up the city and state based off the 5 digit code, and then 
								//changes the zip code to 5 digits. It should not change the zip code.
							}
							if(tempCity == "") 
								SetDlgItemText(IDC_CITY_BOX, city);
							if(tempState == "")
								SetDlgItemText(IDC_STATE_BOX, state);
						}
					}
				break;
				}
				// (j.gruber 2009-10-07 17:39) - PLID 35826 - added for city lookup
				case IDC_CITY_BOX:
				{	// don't check for -1, that just means it hasn't been saved yet.
					if (m_bLookupByCity) {
						CString zip, 
								state,
								tempZip,
								tempState,
								value;
						GetDlgItemText(IDC_CITY_BOX, value);
						GetDlgItemText(IDC_ZIP_BOX, tempZip);
						GetDlgItemText(IDC_STATE_BOX, tempState);
						tempZip.TrimRight();
						tempState.TrimRight();
						// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
						if(!tempZip.IsEmpty() || !tempState.IsEmpty()) {
							MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
							if(AfxMessageBox("You have changed the city but the postal code or state already have data in them.  Would you like to overwrite "
								"this data with that of the new city?", MB_YESNO) == IDYES)
							{
								//Just treat them as empty and the code below will fill them.
								tempZip.Empty();
								tempState.Empty();
							}
						}
						if(tempZip== "" || tempState == "") {
							GetCityInfo(value, &zip, &state);
							if(tempZip == "") 
								SetDlgItemText(IDC_ZIP_BOX, zip);
							if(tempState == "")
								SetDlgItemText(IDC_STATE_BOX, state);
						}
					}
				break;
				}

				case IDC_PHONE_BOX:
					SaveAreaCode(LOWORD(wParam));
				break;
			}
		break;

		case EN_SETFOCUS:
			switch (LOWORD(wParam)) {
				case IDC_PHONE_BOX:
					if (ShowAreaCode()) {
						FillAreaCode(LOWORD(wParam));
					}
				break;
			}
		break;
	}		
	return CNxDialog::OnCommand(wParam, lParam); // (d.moore 2007-04-25 10:39) - PLID 25754 - Changed CDialog to CNxDialog.
}

void CResponsiblePartyDlg::Capitalize(int ID)
{
	CString		value;
	int			x1, 
				x2;
	static bool IsRunning = false;
	
	if (IsRunning)
		return;
	IsRunning = true;
	CNxEdit *tmpEdit = (CNxEdit *)GetDlgItem(ID);
	tmpEdit->GetSel (x1, x2);
	GetDlgItemText (ID, value);
	SetUppersInString(tmpEdit, value);
	tmpEdit->SetWindowText (value);
	SetDlgItemText (ID, value);
	tmpEdit->SetSel (x1, x2);
	IsRunning = false;
}

void CResponsiblePartyDlg::FormatItem(int ID, CString format)
{
	CString value;
	GetDlgItemText(ID, value);
	FormatItemText(GetDlgItem(ID), value, format);
}

void CResponsiblePartyDlg::FillAreaCode(long nPhoneID)  {

	
		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nPhoneID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 17:35) - PLID 35826 - updated for city lookup
			CString strAreaCode, strZip, strCity;
			GetDlgItemText(IDC_ZIP_BOX, strZip);
			GetDlgItemText(IDC_CITY_BOX, strCity);
			BOOL bResult = FALSE;
			if (!m_bLookupByCity) {
				bResult  = GetZipInfo(strZip, NULL, NULL, &strAreaCode);
			}
			else {
				bResult = GetCityInfo(strCity, NULL, NULL, &strAreaCode);
			}
			if (bResult) {
				SetDlgItemText(nPhoneID, strAreaCode);
				
				//set the member variable 
				m_strAreaCode.Format("(%s) ###-####", strAreaCode);


				//set the cursor
				::PostMessage(GetDlgItem(nPhoneID)->GetSafeHwnd(), EM_SETSEL, 5, 5);
			}
			else {
				//set the member variable to be blank
				m_strAreaCode = "";
			}
			
	  	}
		else {

			//set the member variable to be blank
			m_strAreaCode = "";
		}

}


bool CResponsiblePartyDlg::SaveAreaCode(long nID) {

	//is the member variable empty
	if (m_strAreaCode.IsEmpty() ) {
		//default to returning true becuase just becauase we didn't do anything with the areacode, doesn't mean they didn't change the number
		return true;
	}
	else {
		//check to see if that is the only thing that is in the box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (strPhone == m_strAreaCode) {
			//if they are equal then erase the area code
			SetDlgItemText(nID, "");
			return false;
		}
		else {
			return true;
		}

	}
	//set out member variable to blank
	m_strAreaCode = "";

}

void CResponsiblePartyDlg::Delete() {

	//delete the party
	ExecuteSql("UPDATE PatientsT SET PrimaryRespPartyID = (SELECT MAX(PersonID) FROM ResponsiblePartyT WHERE PatientID = %li AND PersonID <> %li GROUP BY PatientID) WHERE PersonID = %li", GetActivePatientID(), m_ID, GetActivePatientID());
	ExecuteSql("DELETE FROM ResponsiblePartyT WHERE PersonID = %li",m_ID);
	ExecuteSql("DELETE FROM PersonT WHERE ID = %li",m_ID);		
}

BEGIN_EVENTSINK_MAP(CResponsiblePartyDlg, CNxDialog) // (d.moore 2007-04-25 10:39) - PLID 25754 - Changed CDialog to CNxDialog.
    //{{AFX_EVENTSINK_MAP(CResponsiblePartyDlg)
	ON_EVENT(CResponsiblePartyDlg, IDC_LIST_RESP, 16 /* SelChosen */, OnSelChosenListResp, VTS_DISPATCH)
	ON_EVENT(CResponsiblePartyDlg, IDC_LIST_RESP, 18 /* RequeryFinished */, OnRequeryFinishedListResp, VTS_I2)
	ON_EVENT(CResponsiblePartyDlg, IDC_LIST_RESP, 1 /* SelChanging */, OnSelChangingListResp, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResponsiblePartyDlg, IDC_RELATE_COMBO, 1 /* SelChanging */, OnSelChangingRelateCombo, VTS_PI4)
	ON_EVENT(CResponsiblePartyDlg, IDC_GENDER_LIST, 1 /* SelChanging */, OnSelChangingGenderList, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CResponsiblePartyDlg, IDC_RELATE_COMBO, 16, CResponsiblePartyDlg::SelChosenRelateCombo, VTS_I4)
END_EVENTSINK_MAP()

void CResponsiblePartyDlg::OnSelChosenListResp(LPDISPATCH lpRow) 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	long nCount = m_dlList->GetRowCount();

	if (pRow) {
		if (nCount > 0) {
			CString str;
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index
			str.Format("Responsible Party - %li of %li", (long)(pRow->CalcRowNumber() + 1), nCount);
			SetWindowText(str);
		}

		m_ID = VarLong(pRow->GetValue(erpRespID), -1);
	}
	else { // they have no responsible parties?
		pRow = m_dlList->GetFirstRow();
		if (pRow)
			m_ID = VarLong(pRow->GetValue(erpRespID), -1);
		else
			m_ID = -1;

		m_dlList->PutCurSel(pRow);
		SetWindowText("Responsible Party");
	}

	Load();
}

void CResponsiblePartyDlg::OnRequeryFinishedListResp(short nFlags) 
{
	m_bReady = true;

	long nCount = m_dlList->GetRowCount();
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	if (m_ID == -1) {
		pRow = m_dlList->GetFirstRow();
	}
	else {
		pRow = m_dlList->FindByColumn(erpRespID, m_ID, NULL, false);
		if (pRow == NULL) {
			pRow = m_dlList->GetFirstRow();
		}
	}

	if (nCount <= 0)
	{
		m_bReady = false;
		GetDlgItem(IDC_LIST_RESP)->EnableWindow(FALSE);
	}

	m_dlList->PutCurSel(pRow);
	OnSelChosenListResp(pRow);
}

void CResponsiblePartyDlg::OnSelChangingListResp(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	// (a.walling 2006-10-12 17:33) - PLID 16059 - Cancel the new selection if there was a problem
	if(!Save()) {
		if (lpOldSel != NULL)
			lpOldSel->AddRef(); // this  MUST be done for datalist2s, or anything that interfaces with Iunknown/LPDISPATCH
		*lppNewSel = lpOldSel;
	}
	return;	
}

void CResponsiblePartyDlg::ClearInfo()
{
	SetDlgItemText(IDC_FIRST_NAME_BOX,"");
	SetDlgItemText(IDC_MIDDLE_NAME_BOX,"");
	SetDlgItemText(IDC_LAST_NAME_BOX,"");
	SetDlgItemText(IDC_ADDRESS1_BOX,"");
	SetDlgItemText(IDC_ADDRESS2_BOX,"");
	SetDlgItemText(IDC_CITY_BOX,"");
	SetDlgItemText(IDC_STATE_BOX,"");
	SetDlgItemText(IDC_ZIP_BOX,"");
	m_nxtBirthDate->Clear();
	SetDlgItemText(IDC_PHONE_BOX,"");
	SetDlgItemText(IDC_EMPLOYER_SCHOOL_BOX,"");
	CheckDlgButton(IDC_RESP_PRIMARY, FALSE);
	m_RelateCombo->CurSel = -1;
	m_GenderCombo->CurSel = -1;
	// (d.singleton 2013-11-12 17:20) - PLID 59442 - need to have a maiden name text box for the resp party dialog.
	SetDlgItemText(IDC_MAIDEN_NAME_BOX, "");
}

void CResponsiblePartyDlg::OnSelChangingRelateCombo(long FAR* nNewSel) 
{
	// (a.walling 2006-10-18 11:43) - PLID 23034 - Prevent exception by preventing the empty selection in the first place.

	if (*nNewSel == sriNoRow) {
		*nNewSel = m_RelateCombo->CurSel;
	}
}

void CResponsiblePartyDlg::OnSelChangingGenderList(long FAR* nNewSel) 
{
	// (a.walling 2006-10-18 11:43) - PLID 23034 - Prevent exception by preventing the empty selection in the first place.
	if (*nNewSel == sriNoRow) {
		*nNewSel = m_GenderCombo->CurSel;
	}
}

void CResponsiblePartyDlg::OnCancel() 
{
	// (d.moore 2007-04-25 10:20) - PLID 25754 - Changed inheritence from CDialog to CNxDialog.
	//  I had to add this event handler to get the Cancel button working.
	CDialog::OnCancel();
}

// (d.singleton 2013-11-12 17:20) - PLID 59442 - need to have a maiden name text box for the resp party dialog.  it will only show on the dialog if the relationship is set to mother.
void CResponsiblePartyDlg::SelChosenRelateCombo(long nRow)
{
	try {
		//if they selected mother then show and enable our maiden name box.  only way to get here is if everything else is enabled
		EnableMaidenNameBox(nRow);
	}NxCatchAll(__FUNCTION__);
}

void CResponsiblePartyDlg::EnableMaidenNameBox(long nRow)
{
	if(nRow == -1) {
		return;
	}
	CString strMaidenName = VarString(m_RelateCombo->GetValue(nRow, 0), "");	
	if(strMaidenName.CompareNoCase("Mother") == 0) {
		//if mother is selected then show the edit box, but only enable if told
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->EnableWindow(TRUE);
	}
	else {
		//if mother is not selected then disable and hide
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_MAIDEN_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAIDEN_NAME_BOX)->EnableWindow(FALSE);
	}
}