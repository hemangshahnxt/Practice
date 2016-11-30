// TelevoxOutputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "TelevoxOutputDlg.h"
#include "InternationalUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;

// (z.manning 2008-07-10 16:41) - PLID 20543 - This function loads all televox fields in
// their proper sort order.
void LoadTelevoxFieldsInOrder(OUT CArray<TelevoxField,TelevoxField&> &aryTelevoxFields)
{
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, DisplayName, IsExported \r\n"
		"FROM TelevoxFieldsT \r\n"
		"ORDER BY SortOrder "
		);

	aryTelevoxFields.RemoveAll();
	for(; !prs->eof; prs->MoveNext())
	{
		TelevoxField tf;
		tf.nID = AdoFldLong(prs, "ID");
		tf.strDisplay = AdoFldString(prs, "DisplayName");
		tf.bExport = AdoFldBool(prs, "IsExported");

		aryTelevoxFields.Add(tf);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTelevoxOutputDlg dialog

enum {	//TelevoxOutputList enums
	tolID = 0,
	tolLast,
	tolFirst,
	tolDate,
	tolTime,
};

CTelevoxOutputDlg::CTelevoxOutputDlg(CWnd* pParent)
	: CNxDialog(CTelevoxOutputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTelevoxOutputDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bFailIfNot10Digits = FALSE;
	m_bNoCellNoExport = FALSE;
	m_bNoCellIfNoTextMsg = FALSE;
}


void CTelevoxOutputDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTelevoxOutputDlg)
	DDX_Control(pDX, IDC_HELP_WHY_FAILED, m_lblWhyFailed);
	DDX_Control(pDX, IDC_TELEVOX_OUTPUT_PATH, m_nxeditTelevoxOutputPath);
	DDX_Control(pDX, IDOK, m_btnExport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTelevoxOutputDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTelevoxOutputDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnWhyFailed)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTelevoxOutputDlg message handlers

BOOL CTelevoxOutputDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnExport.AutoSet(NXB_EXPORT);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_pOutput = BindNxDataListCtrl(IDC_OUTPUT_LIST, false);
	m_pFailed = BindNxDataListCtrl(IDC_FAILED_LIST, false);

	m_lblWhyFailed.SetColor(0x00DDDDBB);
	m_lblWhyFailed.SetType(dtsHyperlink);
	m_lblWhyFailed.SetText("Why have these patients failed?");

	//load the path from configrt
	CString strPath = GetRemotePropertyText("TelevoxPath", "", 0, "<None>", false);
	SetDlgItemText(IDC_TELEVOX_OUTPUT_PATH, strPath);

	GeneratePatientInfo();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTelevoxOutputDlg::SetOutputSql(CString strSql)
{
	m_strSql = strSql;
}

void CTelevoxOutputDlg::IncludeEmail(BOOL bInclude)
{
	m_bUseEmail = bInclude;
}

void CTelevoxOutputDlg::OnOK() 
{
	CString strPath;
	GetDlgItemText(IDC_TELEVOX_OUTPUT_PATH, strPath);

	if(strPath.IsEmpty()) {
		MsgBox("You must provide a path to save.");
		return;
	}

	if(m_pFailed->GetRowCount() > 0) {
		if(MsgBox(MB_YESNO, "You have patients that failed validation.  If you continue exporting, these patients will not be sent to the output.  Are you sure you wish to continue?") != IDYES)
			return;
	}

	//firstly we need to test if a file is already there
	try {
		CFile fTest;
		if(fTest.Open(strPath, CFile::modeRead | CFile::shareCompat)) {
			if(MsgBox(MB_YESNO, "There is already a file in this location.  Are you sure you wish to overwrite it?") != IDYES)
				return;
		}
	} catch(CFileException* e) {
		e->ReportError();
		return;
	}

	//create a file at that location
	try {
		CFile fOut(strPath, CFile::modeCreate | CFile::shareDenyWrite | CFile::modeWrite);

		fOut.Write(m_strOutput, m_strOutput.GetLength());
		fOut.Close();

	} catch(CFileException* e) {
		e->ReportError();
		return;
	}

	CString str;
	str.Format("File successfully created at '%s'.", strPath);
	MsgBox(str);

	//remember the path
	SetRemotePropertyText("TelevoxPath", strPath, 0, "<None>");

	CDialog::OnOK();
}

void CTelevoxOutputDlg::OnCancel() 
{
	CDialog::OnCancel();
}

//macros to get types out of the prs recordset
#define FIELD_STRING(str)	VarString(prs->Fields->Item[_bstr_t(str)]->Value, "")
#define FIELD_DATETIME(str)	VarDateTime(prs->Fields->Item[_bstr_t(str)]->Value)
#define FIELD_LONG(str)		VarLong(prs->Fields->Item[_bstr_t(str)]->Value)

//macro to replace all " with ' in a string
#define REPLACE_QUOTE(str)	str.Replace("\"", "'");

//function to generate the output string and fill in all the datalists so the
//users can view the output before committing their file.
void CTelevoxOutputDlg::GeneratePatientInfo()
{
	//if the string is empty, don't bother trying to create a 
	//recordset, it will fail
	if(m_strSql.IsEmpty()) {
		MsgBox("No patients were found");
		return;
	}

	//generate the query
	try {

		// (z.manning 2008-07-10 11:10) - PLID 20543 - Load the televox field into from data
		CArray<TelevoxField, TelevoxField&> aryTelevoxFields;
		LoadTelevoxFieldsInOrder(aryTelevoxFields);

		_RecordsetPtr prs = CreateRecordset(m_strSql);

		while(!prs->eof) {
			// (a.wilson 2013-02-25 16:19) - PLID 54637 - added prefcontact and prefcontactvalue to fix removing '-' from emails.
			//we must loop through all the records and fill them into the datalists.
			//while we do that, we want to just generate the output string so we
			//only need 1 database read.
			CString strLast = FIELD_STRING("Last");
			CString strFirst = FIELD_STRING("First");
			long nPrefContact = FIELD_LONG("PreferredContact");
			CString strPrefContactValue = FIELD_STRING("PreferredContactValue");
			_variant_t varDate = prs->Fields->Item["Date"]->Value;
			_variant_t varTime = prs->Fields->Item["StartTime"]->Value;
			long nID = FIELD_LONG("UserDefinedID");
			CString strResource = FIELD_STRING("Resource");
			CString strType = FIELD_STRING("TypeName");
			CString strLoc = FIELD_STRING("LocName");
			CString strAddr1 = FIELD_STRING("Address1");
			CString strCity = FIELD_STRING("City");
			CString strState = FIELD_STRING("State");
			CString strZip = FIELD_STRING("Zip");
			CString strWPhone = FIELD_STRING("WorkPhone");
			CString strNotes = FIELD_STRING("Notes");
			CString strEmail = FIELD_STRING("Email");
			CString strCellPhone = FIELD_STRING("CellPhone");
			BOOL bTextMessagePrivacy = AdoFldBool(prs, "TextMessage");

			//Validation
			bool bFailed = false;	//track to see if we fail to output this
			{
				CString temp = "";

				///////////////////////
				//If these fields are split, they are limited to 25 chars each
				if(strLast.GetLength() > 25)
					strLast = strLast.Left(25);

				if(strFirst.GetLength() > 25)
					strFirst = strFirst.Left(25);

				///////////////////////
				//format the phone # to be just numbers
				// (a.wilson 2013-02-25 16:15) - PLID 54637 - only format when the preferredcontact is a phonenumber and not an email.
				if (nPrefContact != 6) {
					strPrefContactValue.Remove('-');
					strPrefContactValue.Remove('(');
					strPrefContactValue.Remove(')');
					strPrefContactValue.Remove(' ');
					// (z.manning 2008-07-10 16:36) - PLID 20543 - The failure if not 10 digtis is now an
					// option rather than required behavior.
					if(m_bFailIfNot10Digits && strPrefContactValue.GetLength() != 10)
						bFailed = true;	//phone length MUST be 10 digits
				} else {
					//remove any beginning and trailing whitespace from emails.
					strPrefContactValue = Trim(strPrefContactValue);
				}

				///////////////////////
				//acct number
				temp.Format("%li", nID);
				if(temp.GetLength() > 15)
					nID = -1;	//if the length is > 15 characters, set it to a junk value, unsupported

				///////////////////////
				//Resource
				if(strResource.GetLength() > 30)
					//resource is limited to 30 chars
					strResource = strResource.Left(30);

				///////////////////////
				//Type
				if(strType.GetLength() > 30)
					//type limited to 30 chars
					strType = strType.Left(30);

				///////////////////////
				//Location
				if(strLoc.GetLength() > 30)
					strLoc = strLoc.Left(30);

				///////////////////////
				//Address
				if(strAddr1.GetLength() > 30)
					strAddr1 = strAddr1.Left(30);

				///////////////////////
				//City
				if(strCity.GetLength() > 15)
					strCity = strCity.Left(30);

				///////////////////////
				//State
				if(strState.GetLength() > 2)
					strState = strState.Left(2);

				///////////////////////
				//Zip
				if(strZip.GetLength() > 10)
					strZip = strZip.Left(10);

				///////////////////////
				//WorkPhone
				strWPhone.Remove('-');
				strWPhone.Remove('(');
				strWPhone.Remove(')');

				///////////////////////
				//Notes
				if(strNotes.GetLength() > 140)
					strNotes = strNotes.Left(140);

				//we need to remove any newlines that might exist in the notes
				strNotes.Remove('\r');
				strNotes.Remove('\n');

				///////////////////////
				//Email
				if(strEmail.GetLength() > 100) {
					if(m_bUseEmail)
						bFailed = true;		//we can't send an email if this is invalid, so this patient fails
					strEmail.Empty();		//there's no point in trimming their email, it will fail to be received
				}

				///////////////////////
				// (z.manning 2008-07-10 12:19) - PLID 20543 - Added cell phone
				strCellPhone.Remove('-');
				strCellPhone.Remove('(');
				strCellPhone.Remove(')');
				strCellPhone.Remove(' ');
				// (z.manning 2008-07-11 13:36) - PLID 30678 - We may want to clear the cell phone field for
				// those who don't want to receive text messages.
				if(m_bNoCellIfNoTextMsg && bTextMessagePrivacy) {
					strCellPhone.Empty();
				}
				// (z.manning 2008-07-10 17:16) - PLID 20543 - Added option to not export if no cell phone.
				if(m_bNoCellNoExport && strCellPhone.IsEmpty()) {
					bFailed = true;
				}
				// (a.wilson 2013-02-25 16:19) - PLID 54637 - replaced strHPhone with PrefContactValue
				//due to formatting, we have to make sure there are no " characters in any strings.  If there are, 
				//we should just replace them with ' characters.
				REPLACE_QUOTE(strLast);
				REPLACE_QUOTE(strFirst);
				REPLACE_QUOTE(strPrefContactValue);
				REPLACE_QUOTE(strResource);
				REPLACE_QUOTE(strType);
				REPLACE_QUOTE(strLoc);
				REPLACE_QUOTE(strAddr1);
				REPLACE_QUOTE(strCity);
				REPLACE_QUOTE(strState);
				REPLACE_QUOTE(strZip);
				REPLACE_QUOTE(strWPhone);
				REPLACE_QUOTE(strNotes);
				REPLACE_QUOTE(strCellPhone);

			}

			//once all our variables are validated correctly, we put them in the appropriate datalist
			//and, if success, into the output string.
			if(bFailed) {
				//put them in the failed list
				IRowSettingsPtr pRow = m_pFailed->GetRow(-1);
				pRow->PutValue(tolID, (long)nID);
				pRow->PutValue(tolLast, _bstr_t(strLast));
				pRow->PutValue(tolFirst, _bstr_t(strFirst));
				pRow->PutValue(tolDate, varDate);
				pRow->PutValue(tolTime, varTime);

				m_pFailed->AddRow(pRow);
			}
			else {
				//success!
				IRowSettingsPtr pRow = m_pOutput->GetRow(-1);
				pRow->PutValue(tolID, (long)nID);
				pRow->PutValue(tolLast, _bstr_t(strLast));
				pRow->PutValue(tolFirst, _bstr_t(strFirst));
				pRow->PutValue(tolDate, varDate);
				pRow->PutValue(tolTime, varTime);

				m_pOutput->AddRow(pRow);

				//we only set the email flag if the patient has an email address, and they checked the box on the export dlg
				int nEmail = 0;
				if(m_bUseEmail && !strEmail.IsEmpty())
					nEmail = 1;

				//format for the output file and send (15 total fields)
				COleDateTime dtDate, dtTime;
				dtDate = VarDateTime(varDate);
				dtTime = VarDateTime(varTime);

				// (z.manning 2008-07-10 11:14) - PLID 20543 - The fields and order are now customizable.
				// (a.wilson 2013-02-25 16:19) - PLID 54637 - replaced strHPhone with PrefContactValue
				for(int nFieldIndex = 0; nFieldIndex < aryTelevoxFields.GetSize(); nFieldIndex++)
				{
					TelevoxField tf = aryTelevoxFields.GetAt(nFieldIndex);
					if(tf.bExport)
					{
						CString strAppend;
						if(tf.strDisplay == "Last Name") { strAppend = '"' + strLast + '"'; }
						else if(tf.strDisplay == "First Name") { strAppend = '"' + strFirst + '"'; }
						else if(tf.strDisplay == "Preferred Contact Number") { strAppend = '"' + strPrefContactValue + '"'; }
						else if(tf.strDisplay == "Appt Date") { strAppend = '"' + FormatDateTimeForInterface(dtDate, 0, dtoDate) + '"'; }
						else if(tf.strDisplay == "Appt Time") { strAppend = '"' + FormatDateTimeForInterface(dtTime, 0, dtoTime) + '"'; }
						else if(tf.strDisplay == "Patient ID") { strAppend = AsString(nID); }
						else if(tf.strDisplay == "Appt Resource") { strAppend = '"' + strResource + '"'; }
						else if(tf.strDisplay == "Appt Type") { strAppend = '"' + strType + '"'; }
						else if(tf.strDisplay == "Appt Location") { strAppend = '"' + strLoc + '"'; }
						else if(tf.strDisplay == "Address") { strAppend = '"' + strAddr1 + '"'; }
						else if(tf.strDisplay == "City") { strAppend = '"' + strCity + '"'; }
						else if(tf.strDisplay == "State") { strAppend = '"' + strState + '"'; }
						else if(tf.strDisplay == "Zip") { strAppend = '"' + strZip + '"'; }
						else if(tf.strDisplay == "Work Phone") { strAppend = '"' + strWPhone + '"'; }
						else if(tf.strDisplay == "Appt Notes") { strAppend = '"' + strNotes + '"'; }
						else if(tf.strDisplay == "Has EMail Flag") { strAppend = AsString((long)nEmail); }
						else if(tf.strDisplay == "EMail Address") { strAppend = '"' + strEmail + '"'; }
						else if(tf.strDisplay == "Cell Phone") { strAppend = '"' + strCellPhone + '"'; }
						// (z.manning 2008-07-11 13:22) - PLID 30678 - Added text message flag
						else if(tf.strDisplay == "Text Message Flag") { strAppend = bTextMessagePrivacy ? "1" : "0"; }
						else {
							ThrowNxException("Invalid televox field display name: %s", tf.strDisplay);
						}

						m_strOutput += strAppend + ',';
					}
				}
				m_strOutput.TrimRight(',');
				m_strOutput += "\r\n";
			}

			prs->MoveNext();
		}

	} NxCatchAllCall("Error in GeneratePatientInfo()", ((CWnd*)GetDlgItem(IDOK))->EnableWindow(FALSE));	//disable the ability to write the file if there is an error
}

LRESULT CTelevoxOutputDlg::OnWhyFailed(WPARAM wParam, LPARAM lParam)
{

	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
	case IDC_HELP_WHY_FAILED:
		{
			//for now, a nice message box outlining the reasons patients fail is sufficient
			CString str;
			str.Format("Patients fail for one of the following reasons:\r\n"
				" - The patient has an invalid phone format.  Please make sure there are 10 digits in the patients phone number.\r\n"
				" - If you have chosen to email patients, they must have an email address that is less than 100 characters.  If you are not including emails, you will never get a failure due to a bad email address.\r\n"
				" - If you are exporting cell phone numbers and the patient does not have a cell phone number can cause he or she to not be exported.\r\n"
				"\r\n"
				"Please check the patients which have failed for these criteria.");

			MsgBox(str);
		}
		break;

	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}

void CTelevoxOutputDlg::OnBrowse() 
{
	CFileDialog dlg(FALSE, "*.csv", "TelevoxExport.csv", 0, "Comma Separated Values (*.csv)|*.csv|All Files (*.*)|*.*||");
	if(dlg.DoModal() == IDOK) {
		CString strPath = dlg.GetPathName();
		SetDlgItemText(IDC_TELEVOX_OUTPUT_PATH, strPath); 
	}
}

BOOL CTelevoxOutputDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	//if they mouse over the static hyperlink, we need to draw the hand cursor
	CRect rc;
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	((CWnd*)GetDlgItem(IDC_HELP_WHY_FAILED))->GetWindowRect(rc);
	ScreenToClient(&rc);

	if (rc.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
