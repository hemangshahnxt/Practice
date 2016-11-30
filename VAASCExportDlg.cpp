// VAASCExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VAASCExportDlg.h"
#include "GlobalUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include <NxPracticeSharedLib\SharedDiagnosisUtils.h>

// (j.jones 2007-07-02 17:23) - PLID 25493 - created

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CVAASCExportDlg dialog


CVAASCExportDlg::CVAASCExportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CVAASCExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVAASCExportDlg)
	//}}AFX_DATA_INIT
}


void CVAASCExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVAASCExportDlg)
	DDX_Control(pDX, IDC_BTN_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_BTN_EXPORT_LIST, m_btnExportList);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_VA_ASC_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_VA_ASC_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_PROGRESS_CTRL, m_progressbar);
	DDX_Control(pDX, IDC_EDIT_HOSPITAL_IDENTIFIER, m_nxeditEditHospitalIdentifier);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVAASCExportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CVAASCExportDlg)
	ON_BN_CLICKED(IDC_BTN_EXPORT, OnBtnExport)
	ON_BN_CLICKED(IDC_BTN_EXPORT_LIST, OnBtnExportList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVAASCExportDlg message handlers

BOOL CVAASCExportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (r.gonet 04/03/2014) - PLID 61640 - Added a bulk cache and added the existing properties used in this dialog to it.
		g_propManager.BulkCache("VAASCExportDlg", propbitNumber | propbitText | propbitDateTime, 
			"(Username = '<None>' OR Username = '%s') AND ( "
			"	Name = 'ASCHospitalIdentifier' OR "
			"	Name = 'ICD10GoLiveDateOverride_VAASC' " // (r.gonet 03/20/2014) - PLID 61640 - Hidden setting to override the ICD-10 go live date for VAASC
			") ",
			_Q(GetCurrentUserName())); 

		// (j.jones 2008-05-08 11:13) - PLID 29953 - set button styles for modernization
		m_btnExport.AutoSet(NXB_EXPORT);
		m_btnExportList.AutoSet(NXB_EXPORT);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_strAllowedCPTCodes = 
			//Colonosocopy
			"'45355', '45378', '45379', '45380', '45382', '45383', '45384', '45385', "
			//Laparoscopy
			"'49320', '49321', '49322', '49323', '49329', '58550', '58551', '58559', '58560', '58561', '58563', '58578', "
			"'58579', '58660', '58661', '58662', '58670', '58671', '58672', '58673', '58679', '47562', '47563', '47564', "
			"'47570', '47579', '49650', '49651', '49659', "
			//Breast Surgery
			"'19102', '19103', '19110', '19112', '19120', '19125', '19126', '19140', '19160', '19290', '19291', '19499', "
			"'19316', '19318', '19324', '19325', '19328', '19330', '19340', '19342', '19350', '19355', '19357', '19361', "
			"'19364', '19366', '19367', '19368', '19369', '19370', '19371', '19380', '19396', "
			//Hernia Repair
			"'49495', '49496', '49500', '49501', '49505', '49507', '49520', '49521', '49525', '49540', '49550', '49553', "
			"'49555', '49557', '49560', '49561', '49565', '49566', '49568', '49570', '49572', '49580', '49582', '49585', "
			"'49587', '49590', '49600', '49605', '49606', '49610', '49611', "
			//Liposuction
			"'15876', '15877', '15878', '15879', "
			//Facial Surgery
			"'15825', '15828', '15820', '15821', '15822', '15823', "
			//Knee Arthroscopy
			"'29871', '29874', '29875', '29876', '29877', '29879', '29880', '29881', '29882', '29883', '29884', '29885', "
			"'29886', '29887', '29888', '29889'";
	
		m_POSCombo = BindNxDataList2Ctrl(this, IDC_VA_ASC_POS_COMBO, GetRemoteData(), true);
		m_POS2Combo = BindNxDataList2Ctrl(this, IDC_VA_ASC_ADDL_POS_COMBO, GetRemoteData(), true);

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtTo(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
		COleDateTime dtFrom = dtTo;
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(60,0,0,0);
		dtFrom = dtFrom - dtSpan;

		m_dtFrom.SetValue(_variant_t(dtFrom));
		m_dtTo.SetValue(_variant_t(dtTo));

		m_progressbar.SetRange(1,100);	
		m_progressbar.SetStep(1);
		m_progressbar.SetPos(0);

		int nPOSRow = -1;

		IRowSettingsPtr pPOSRow = m_POSCombo->GetNewRow();
		pPOSRow->PutValue(0, (long)-1);
		pPOSRow->PutValue(1, _bstr_t(" {All Places Of Service}"));
		m_POSCombo->AddRowSorted(pPOSRow, NULL);

		nPOSRow = m_POSCombo->SetSelByColumn(0, (long)-1);

		IRowSettingsPtr pPOS2Row = m_POS2Combo->GetNewRow();
		pPOS2Row->PutValue(0, (long)-1);
		pPOS2Row->PutValue(1, _bstr_t(" {No Secondary Place Of Service}"));
		m_POS2Combo->AddRowSorted(pPOS2Row, NULL);

		m_POS2Combo->SetSelByColumn(0, (long)-1);

		OnSelChosenVaAscPosCombo(pPOSRow);

		SetDlgItemText(IDC_EDIT_HOSPITAL_IDENTIFIER, GetRemotePropertyText("ASCHospitalIdentifier", "", 0, "<None>", true));

	}NxCatchAll("Error in CVAASCExportDlg::OnInitDialog();");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVAASCExportDlg::OnBtnExport() 
{
	try {

		ExportData();

	}NxCatchAll("Error opening export file.");
}

CString CVAASCExportDlg::StripNonNum(CString strIn)
{
	CString strOut = "";

	for(int i=0;i<strIn.GetLength();i++) {
		if(strIn.GetAt(i)>='0' && strIn.GetAt(i)<='9')
			strOut += strIn.GetAt(i);
	}

	return strOut;
}

CString CVAASCExportDlg::OutputString(CString strIn, long nLength, BOOL bIsNumeric /*= FALSE*/)
{
	if(bIsNumeric)
		strIn = StripNonNum(strIn);

	long nStrLen = strIn.GetLength();

	if(nStrLen > nLength)
		strIn = strIn.Left(nLength);

	if(nStrLen < nLength) {
		for(int i=0; i < nLength - nStrLen; i++) {
			if(!bIsNumeric)
				strIn += " ";
			else
				strIn = "0" + strIn;
		}
	}

	return strIn;
}

void CVAASCExportDlg::ExportData()
{
	try {

		CWaitCursor pWait;

		CString str;
		_variant_t var;

		long nPOSID = -1;
		IRowSettingsPtr pRow = m_POSCombo->GetCurSel();
		if(pRow != NULL)
			nPOSID = VarLong(pRow->GetValue(0),-1);

		long nPOSTwoID = -1;
		pRow = m_POS2Combo->GetCurSel();
		if(pRow != NULL)
			nPOSTwoID = VarLong(pRow->GetValue(0),-1);

		CString strHospID = "";
		GetDlgItemText(IDC_EDIT_HOSPITAL_IDENTIFIER, strHospID);
		strHospID.TrimRight();
		if(strHospID.IsEmpty()) {
			AfxMessageBox("You must enter a VHI Provider number for the export.");
			return;
		}
		else {
			//save it for the next time we use the export
			long nRecordsAffected = 0;

			ExecuteSql(&nRecordsAffected, adCmdText, 
				"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
				"UPDATE ConfigRT SET TextParam = '%s' WHERE Name = 'ASCHospitalIdentifier'",_Q(strHospID));

			if(nRecordsAffected == 0)
				ExecuteSql("INSERT INTO ConfigRT (Name, TextParam) VALUES ('ASCHospitalIdentifier','%s')", _Q(strHospID));
		}

		CSqlFragment sqlPOSTwoFilter;
		if(nPOSTwoID != -1) {
			sqlPOSTwoFilter = CSqlFragment(" OR BillsT.Location = {INT}", nPOSTwoID);
		}

		CSqlFragment sqlPOSFilter;
		if(nPOSID != -1) {
			sqlPOSFilter = CSqlFragment("AND (BillsT.Location = {INT} {SQL})", nPOSID, sqlPOSTwoFilter);
		}

		//this pulls up all bills in the date range with at least one of the required CPT codes
		// (b.spivey, May 22, 2013) - PLID 56877 - Never used this RaceID field, but if I don't remove it then we'll get errors. 
		// (j.jones 2014-03-14 08:55) - PLID 61178 - diag codes are now in another recordset later in the export
		_RecordsetPtr rsBills = CreateParamRecordset(FormatString("SELECT PersonT.ID AS PatientID, BillsT.ID AS BillID, "
				"SocialSecurity, BirthDate, BillsT.HospFrom, BillsT.HospTo, "
				"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, UserDefinedID, Gender, "
				"BillsT.Date AS BillDate, PersonT.Company "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID "
				"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.Date >= {OLEDATETIME} AND BillsT.Date < DATEADD(day, 1, {OLEDATETIME}) "
				"{SQL} "
				"AND BillsT.ID IN (SELECT BillID FROM ChargesT "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"	WHERE Deleted = 0 AND ISNUMERIC(Left(CPTCodeT.Code,5)) = 1 "
				"	AND Len(CPTCodeT.Code) >= 5 AND Len(CPTCodeT.Code) <= 7 "
				"	AND Left(CPTCodeT.Code,5) IN (%s)) "
				"ORDER BY BillsT.Date, BillsT.ID", m_strAllowedCPTCodes),
				VarDateTime(m_dtFrom.GetValue()), VarDateTime(m_dtTo.GetValue()),
				sqlPOSFilter);

		if(rsBills->eof) {
			AfxMessageBox("There are no applicable bills to export in the given date range for the selected place of service.");
			return;
		}

		//Now open the file
		CFileException	e;
		CArchive ar( &m_fExport, CArchive::store, 4096, NULL );

		CString strDefName = "VHIExport.txt";

		CFileDialog SaveAs(FALSE,NULL,strDefName);
		// (j.armen 2011-10-25 15:57) - PLID 46139 - We are prompting the user, so use the practice path
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		CString dir = GetEnvironmentDirectory();
		SaveAs.m_ofn.lpstrInitialDir = (LPCSTR)dir;
		SaveAs.m_ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
		if (SaveAs.DoModal() == IDCANCEL) {
			return;
		}

		CString strFileName = SaveAs.GetPathName();

		try 
		{
			m_fExport.Open(strFileName, CFile::modeCreate | CFile::modeWrite, &e);
		}
		catch (CFileException* e) 
		{
			MessageBox(strFileName + ": Access Violation\nFile may be in use");
			m_fExport.Close();
			if (e) e->Delete();
			return;
		}

		m_progressbar.SetRange(1,(short)(rsBills->GetRecordCount()));
		m_progressbar.SetStep(1);

		CWaitCursor pWait2;

		//now export the file

		while(!rsBills->eof) {

			long nPatientID = AdoFldLong(rsBills, "PatientID");
			long nBillID = AdoFldLong(rsBills, "BillID");

			CString strOutput = "";

			//This data should be one line per patient visit

			/////////////////////////////////////////////////////////////////////////////////
			//Name									Output		Position

			/////////////////////////////////////////////////////////////////////////////////
			//1. Provider Identifier				PIC X(6)	1 6

			// (j.jones 2009-10-27 10:19) - PLID 36057 - renamed field for 2009
			
			//taken from the edit box on the dialog
			strOutput += OutputString(strHospID,6);

			/////////////////////////////////////////////////////////////////////////////////
			//2. Provider NPI						PIC X(10)	7 16

			// (j.jones 2009-10-27 10:22) - PLID 36057 - now uses the group NPI

			CString strProvNPI, strLocNPI;
			_RecordsetPtr rsNPI = CreateParamRecordset("SELECT ProvidersT.NPI AS ProvNPI, "
				"LocationsT.NPI AS LocNPI "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"WHERE ChargesT.BillID = {INT} AND ChargesT.LineID = 1 AND LineItemT.Deleted = 0", nBillID);
			if(!rsNPI->eof) {
				strProvNPI = AdoFldString(rsNPI, "ProvNPI","");
				strLocNPI = AdoFldString(rsNPI, "LocNPI","");
			}
			rsNPI->Close();
			
			//use the bill location NPI
			strOutput += OutputString(strLocNPI,10);

			/////////////////////////////////////////////////////////////////////////////////
			//3. Operating Physician Identifier		PIC X(10)	17 26

			//use the NPI of the first provider on the bill
			// (a.walling 2009-02-20 17:48) - PLID 33175 - Need NPI now instead
			strOutput += OutputString(strProvNPI,10);

			/////////////////////////////////////////////////////////////////////////////////
			//4. Record Type						PIC X		27 27

			//Record Type: 1 - UB04, 2 - HCFA

			//send a 2
			strOutput += OutputString("2",1);

			/////////////////////////////////////////////////////////////////////////////////
			//5. Patient Control Number				PIC X(20)	28 47

			str.Format("%li",AdoFldLong(rsBills, "UserDefinedID"));
			strOutput += OutputString(str,20);

			/////////////////////////////////////////////////////////////////////////////////
			//6. Medical Record Number				PIC X(23)	48 70

			//both 5 and 6 are optional, but in Albert's previous file they
			//filled in 5 and not 6, so don't fill in 6
			/*
			str.Format("%li",AdoFldLong(rsBills, "UserDefinedID"));
			*/
			strOutput += OutputString("",23);

			/////////////////////////////////////////////////////////////////////////////////
			//7. Patient Identifier (SSN)			PIC X(9)	71 79
			str = AdoFldString(rsBills, "SocialSecurity","");
			str = StripNonNum(str);
			strOutput += OutputString(str,9);

			/////////////////////////////////////////////////////////////////////////////////
			//8. Patient Sex						PIC X		80 80

			long nGender = AdoFldByte(rsBills, "Gender",0);
			if(nGender == 1)
				str = "M";
			else if(nGender == 2)
				str = "F";
			else
				str = "U";

			strOutput += OutputString(str,1);

			/////////////////////////////////////////////////////////////////////////////////
			//9. Date of Birth MMDDYYYY				PIC 9(8)	81 88

			str = "";
			var = rsBills->Fields->Item["BirthDate"]->Value;
			if(var.vt == VT_DATE) {
				COleDateTime dt = var.date;
				str = dt.Format("%m%d%Y");
			}

			strOutput += OutputString(str,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//10. Patient Street Address			PIC X(40)	89 128

			// (j.jones 2009-10-27 10:30) - PLID 36057 - new for 2009

			str = AdoFldString(rsBills, "Address1","");
			str += " ";
			str += AdoFldString(rsBills, "Address2","");
			strOutput += OutputString(str,40);

			/////////////////////////////////////////////////////////////////////////////////
			//11. Patient City						PIC X(30)	129 158

			// (j.jones 2009-10-27 10:30) - PLID 36057 - new for 2009

			str = AdoFldString(rsBills, "City","");
			strOutput += OutputString(str,30);

			/////////////////////////////////////////////////////////////////////////////////
			//12. Patient Zip						PIC X(9)	159 167

			// (j.jones 2009-10-27 10:30) - PLID 36057 - new for 2009

			str = AdoFldString(rsBills, "Zip","");
			str = StripNonNum(str);
			strOutput += OutputString(str,9);

			/////////////////////////////////////////////////////////////////////////////////
			//13. Status at Discharge				PIC 9(2)	168 169
			
			//Albert's office always said "At Home" and always sent 01,
			//we'll need to fill this eventually but until then, 01 is a good default
			str = "01";
			strOutput += OutputString(str,2,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//14. Admission Date MM/DD/YYYY			PIC 9(8)	170 177

			//we could use the hospitalized date, but until told otherwise let's
			//use the bill date since we will always have it and it is most likely correct
			COleDateTime dtBillDate = VarDateTime(rsBills->Fields->Item["BillDate"]->Value);
			str = dtBillDate.Format("%m%d%Y");
			CString strDate = FormatDateTimeForSql(dtBillDate);

			strOutput += OutputString(str,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//15. Admission Hour					PIC 9(2)	178 179

			//now this one is tricky, we don't track admission hour,
			//the best way to fill it is to see if the patient has an appointment on the day
			//of the bill, and if not, just use 8am as a filler

			//I should also note that there were various records in Albert's previous file
			//that had this field blank, so technically that is allowed
			
			str = "8";
			//use the date from above
			if(!strDate.IsEmpty()) {
				_RecordsetPtr rsAppt = CreateRecordset("SELECT TOP 1 StartTime FROM AppointmentsT "
					"WHERE StartTime >= '%s' AND StartTime < DateAdd(day,1,'%s') AND Status <> 4 ORDER BY StartTime", strDate, strDate);
				if(!rsAppt->eof) {
					COleDateTime dt = AdoFldDateTime(rsAppt, "StartTime");
					//get the hour (24-hour format)
					str = dt.Format("%H");
				}
				rsAppt->Close();
			}
			strOutput += OutputString(str,2,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//get all the payor information in one recordset

			CString strPayorNameA, strPayorNameB, strPayorNameC;
			CString strRelationA, strRelationB, strRelationC;

			_RecordsetPtr rsInsured = CreateParamRecordset("SELECT TOP 3 Name, RelationToPatient "
				"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"WHERE RespTypeID <> -1 AND PatientID = {INT} ORDER BY RespTypeID", nPatientID);
			long nCount = 0;

			if(rsInsured->eof) {
				//have at least the Self Pay then
				strPayorNameA = "Self Pay";
				strRelationA = "18";
			}

			while(!rsInsured->eof) {
				nCount++;
				CString strPayorName = AdoFldString(rsInsured, "Name","");								
				CString str = AdoFldString(rsInsured, "RelationToPatient","");

				if(str == "Child")
					str = "19";
				else if(str == "Self")
					str = "18";
				else if (str == "Spouse")
					str = "01";
				else if (str == "Other")
					str = "G8";
				else if (str == "Employee")
					str = "20";
				else if (str == "Unknown")
					str = "21";
				else if (str == "Organ Donor")
					str = "39";
				else if (str == "Cadaver Donor")
					str = "40";
				else if (str == "Life Partner")
					str = "53";
				else
					str = "18";

				// (j.jones 2011-06-15 17:00) - PLID 40959 - the following are no longer valid entries in our system,
				// and no longer exist in data
				/*
				else if (str == "Grandparent")
					str = "04";
				else if (str == "Grandchild")
					str = "05";
				else if (str == "Nephew Or Niece")
					str = "07";
				else if (str == "Adopted Child")
					str = "09";
				else if (str == "Foster Child")
					str = "10";
				else if (str == "Ward")
					str = "15";
				else if (str == "Stepchild")
					str = "17";
				else if (str == "Handicapped Dependent")
					str = "22";
				else if (str == "Sponsored Dependent")
					str = "23";
				else if (str == "Dependent of a Minor Dependent")
					str = "24";
				else if (str == "Significant Other")
					str = "29";
				else if (str == "Mother")
					str = "32";
				else if (str == "Father")
					str = "33";
				else if (str == "Other Adult")
					str = "34";
				else if (str == "Emancipated Minor")
					str = "36";
				else if (str == "Injured Plaintiff")   // (s.dhole 2010-08-31 15:13) - PLID 40114 All our relationship lists say "Injured Plantiff" instead of "Injured Plaintiff"
					str = "41";
				else if (str == "Child Where Insured Has No Financial Responsibility")
					str = "43";
				*/

				if(nCount==1) {
					strPayorNameA = strPayorName;
					strRelationA = str;
				}
				else if(nCount==2) {
					strPayorNameB = strPayorName;
					strRelationB = str;
				}
				else if(nCount==3) {
					strPayorNameC = strPayorName;
					strRelationC = str;
				}
				rsInsured->MoveNext();
			}
			rsInsured->Close();

			/////////////////////////////////////////////////////////////////////////////////
			//16. Payor Identifier A				PIC X(25)	180 204
			strOutput += OutputString(strPayorNameA,25);

			/////////////////////////////////////////////////////////////////////////////////
			//17. Payor Code A						PIC X(2)	205 206

			// (j.jones 2009-10-27 10:40) - PLID 36057 - new for 2009
			
			//this field says for VHHA only
			strOutput += OutputString("",2);

			/////////////////////////////////////////////////////////////////////////////////
			//18. Payor Identifier B				PIC X(25)	207 231
			strOutput += OutputString(strPayorNameB,25);

			/////////////////////////////////////////////////////////////////////////////////
			//19. Payor Code B						PIC X(2)	232 233

			// (j.jones 2009-10-27 10:40) - PLID 36057 - new for 2009
			
			//this field says for VHHA only
			strOutput += OutputString("",2);

			/////////////////////////////////////////////////////////////////////////////////
			//20. Payor Identifier C				PIC X(25)	234 258
			strOutput += OutputString(strPayorNameC,25);

			/////////////////////////////////////////////////////////////////////////////////
			//21. Payor Code C						PIC X(2)	259 260

			// (j.jones 2009-10-27 10:40) - PLID 36057 - new for 2009
			
			//this field says for VHHA only
			strOutput += OutputString("",2);

			/////////////////////////////////////////////////////////////////////////////////
			//22. Patient Relationship to Insured A	PIC X(2)	261 262
			strOutput += OutputString(strRelationA,2);

			/////////////////////////////////////////////////////////////////////////////////
			//23. Patient Relationship to Insured B	PIC X(2)	263 264
			strOutput += OutputString(strRelationB,2);

			/////////////////////////////////////////////////////////////////////////////////
			//24. Patient Relationship to Insured B	PIC X(2)	265 266
			strOutput += OutputString(strRelationC,2);

			/////////////////////////////////////////////////////////////////////////////////
			//25. Employer Identifier				PIC X(24)	267 290

			//In Albert's old file, they sent EMPLOYED or UNEMPLOYED, nothing else,
			//but the specifications say to send the Employer's name
			str = AdoFldString(rsBills, "Company","");
			strOutput += OutputString(str,24);

			/////////////////////////////////////////////////////////////////////////////////
			//26. Employment Status Code			PIC X		291 291
			
			//there are absolutely no instructions on what values to use here, 
			//Albert's old file had 9 for almost everything, so we'll use that here
			strOutput += OutputString("9",1);

			/////////////////////////////////////////////////////////////////////////////////
			//27. Admission Diagnosis				PIC X(7)	292 298

			//Albert's old file left this blank
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//28. Reason For Visit Code 1			PIC X(7)	299 305

			// (j.jones 2009-10-27 10:45) - PLID 36057 - new field for 2009
			// we leave this blank
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//29. Reason For Visit Code 2			PIC X(7)	306 312

			// (j.jones 2009-10-27 10:45) - PLID 36057 - new field for 2009
			// we leave this blank
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//30. Reason For Visit Code 3			PIC X(7)	313 319

			// (j.jones 2009-10-27 10:45) - PLID 36057 - new field for 2009
			// we leave this blank
			strOutput += OutputString("",7);

			// (j.jones 2014-03-14 09:03) - PLID 61178 - This export uses ICD-9 codes for report periods
			// before 10/1/2014, and ICD-10 codes for report periods on or after 10/1/2014.
			// Our export goes off of the bill date, not charge data, so the most precise check is to
			// just look at this bill's date.
			// (r.gonet 04/03/2014) - PLID 61640 - ICD-10 go live date was bumped up to 10/1/2015.
			std::vector<CString> aryDiagCodes;
			{
				// (r.gonet 04/03/2014) - PLID 61640 - Moved the hard coded default ICD-10 go live dates into a library function and we now 
				// have the ability to override the go-live date for this export if necessary.
				COleDateTime dtICD10 = GetDefaultICD10GoLiveDate();
				dtICD10 = GetRemotePropertyDateTime("ICD10GoLiveDateOverride_VAASC", &dtICD10, 0, 0, false);
				
				//if ICD-9, this filters on the ICD9DiagID column and the ICD10 bit to be false
				//if ICD-10, this filters on the ICD10DiagID column and the ICD10 bit to be true
				CString strDiagQuery = "SELECT TOP 18 DiagCodes.CodeNumber "
					"FROM BillDiagCodeT "
					"INNER JOIN DiagCodes ON BillDiagCodeT.ICD10DiagID = DiagCodes.ID "
					"WHERE BillDiagCodeT.BillID = {INT} AND DiagCodes.ICD10 = {INT} "
					"ORDER BY BillDiagCodeT.OrderIndex";
				if(dtBillDate < dtICD10) {
					//join on ICD-9, not ICD-10
					strDiagQuery.Replace("BillDiagCodeT.ICD10DiagID", "BillDiagCodeT.ICD9DiagID");
				}
				
				_RecordsetPtr rsDiagCodes = CreateParamRecordset(strDiagQuery, nBillID, dtBillDate < dtICD10 ? 0 : 1);
				while(!rsDiagCodes->eof) {
					CString strDiagCode = VarString(rsDiagCodes->Fields->Item["CodeNumber"]->Value);
					strDiagCode.TrimLeft(); strDiagCode.TrimRight();
					strDiagCode.Replace(".", "");
					if(!strDiagCode.IsEmpty()) {
						aryDiagCodes.push_back(strDiagCode);
					}
					rsDiagCodes->MoveNext();
				}
				rsDiagCodes->Close();
			}

			/////////////////////////////////////////////////////////////////////////////////
			//31. Principal Diagnosis				PIC X(8)	320 327
			str = "";
			if(aryDiagCodes.size() > 0) {
				str = aryDiagCodes[0];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//32. Other Diagnosis 1					PIC X(8)	328 335
			str = "";
			if(aryDiagCodes.size() > 1) {
				str = aryDiagCodes[1];
			}
			strOutput += OutputString(str,8);
			
			/////////////////////////////////////////////////////////////////////////////////
			//33. Other Diagnosis 2					PIC X(8)	336 343
			str = "";
			if(aryDiagCodes.size() > 2) {
				str = aryDiagCodes[2];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//34. Other Diagnosis 3					PIC X(8)	344 351
			str = "";
			if(aryDiagCodes.size() > 3) {
				str = aryDiagCodes[3];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//35. Other Diagnosis 4					PIC X(8)	352 359
			str = "";
			if(aryDiagCodes.size() > 4) {
				str = aryDiagCodes[4];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//36. Other Diagnosis 5					PIC X(8)	360 367
			str = "";
			if(aryDiagCodes.size() > 5) {
				str = aryDiagCodes[5];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//37. Other Diagnosis 6					PIC X(8)	368 375
			str = "";
			if(aryDiagCodes.size() > 6) {
				str = aryDiagCodes[6];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//38. Other Diagnosis 7					PIC X(8)	376 383
			str = "";
			if(aryDiagCodes.size() > 7) {
				str = aryDiagCodes[7];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//39. Other Diagnosis 8					PIC X(8)	384 391
			str = "";
			if(aryDiagCodes.size() > 8) {
				str = aryDiagCodes[8];
			}
			strOutput += OutputString(str,8);

			// (j.jones 2009-10-27 10:45) - PLID 36057 - new diag fields for 2009

			/////////////////////////////////////////////////////////////////////////////////
			//40. Other Diagnosis 9					PIC X(8)	392 399
			str = "";
			if(aryDiagCodes.size() > 9) {
				str = aryDiagCodes[9];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//41. Other Diagnosis 10				PIC X(8)	400 407
			str = "";
			if(aryDiagCodes.size() > 10) {
				str = aryDiagCodes[10];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//42. Other Diagnosis 11				PIC X(8)	408 415
			str = "";
			if(aryDiagCodes.size() > 11) {
				str = aryDiagCodes[11];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//43. Other Diagnosis 12				PIC X(8)	416 423
			str = "";
			if(aryDiagCodes.size() > 12) {
				str = aryDiagCodes[12];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//44. Other Diagnosis 13				PIC X(8)	424 431
			str = "";
			if(aryDiagCodes.size() > 13) {
				str = aryDiagCodes[13];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//45. Other Diagnosis 14				PIC X(8)	432 439
			str = "";
			if(aryDiagCodes.size() > 14) {
				str = aryDiagCodes[14];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//46. Other Diagnosis 15				PIC X(8)	440 447
			str = "";
			if(aryDiagCodes.size() > 15) {
				str = aryDiagCodes[15];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//47. Other Diagnosis 16				PIC X(8)	440 455
			str = "";
			if(aryDiagCodes.size() > 16) {
				str = aryDiagCodes[16];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//48. Other Diagnosis 17				PIC X(8)	456 463
			str = "";
			if(aryDiagCodes.size() > 17) {
				str = aryDiagCodes[17];
			}
			strOutput += OutputString(str,8);

			/////////////////////////////////////////////////////////////////////////////////
			//49. External Cause of Injury 1 (ECODE)	PIC X(8)	464 471

			//Albert's old file left this blank
			strOutput += OutputString("",8);

			/////////////////////////////////////////////////////////////////////////////////
			//50. External Cause of Injury 2 (ECODE)	PIC X(8)	472 479

			// (j.jones 2009-10-27 10:45) - PLID 36057 - new field for 2009, left blank
			strOutput += OutputString("",8);

			/////////////////////////////////////////////////////////////////////////////////
			//51. External Cause of Injury 3 (ECODE)	PIC X(8)	480 487

			// (j.jones 2009-10-27 10:45) - PLID 36057 - new field for 2009, left blank
			strOutput += OutputString("",8);

			/////////////////////////////////////////////////////////////////////////////////
			//52. Principal Procedure (ICD-9)		PIC X(7)	488 494

			//Albert's old file left this blank
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//53. Other Procedure A (ICD-9)			PIC X(7)	495 501

			//Albert's old file left these blank
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//54. Other Procedure B (ICD-9)			PIC X(7)	502 508
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//55. Other Procedure C (ICD-9)			PIC X(7)	509 515
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//56. Other Procedure D (ICD-9)			PIC X(7)	516 522
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//57. Other Procedure E (ICD-9)			PIC X(7)	523 529
			strOutput += OutputString("",7);

			/////////////////////////////////////////////////////////////////////////////////
			//load up all the charge information in one recordset

			CString strCPTCode1, strCPTCode2, strCPTCode3, strCPTCode4, strCPTCode5, strCPTCode6;
			CString strCPTModifier1a, strCPTModifier1b, strCPTModifier1c, strCPTModifier1d;
			CString strCPTModifier2a, strCPTModifier2b, strCPTModifier2c, strCPTModifier2d;
			CString strCPTModifier3a, strCPTModifier3b, strCPTModifier3c, strCPTModifier3d;
			CString strCPTModifier4a, strCPTModifier4b, strCPTModifier4c, strCPTModifier4d;
			CString strCPTModifier5a, strCPTModifier5b, strCPTModifier5c, strCPTModifier5d;
			CString strCPTModifier6a, strCPTModifier6b, strCPTModifier6c, strCPTModifier6d;
			CString strCPTDate1, strCPTDate2, strCPTDate3, strCPTDate4, strCPTDate5, strCPTDate6;
			CString strQuantity1, strQuantity2, strQuantity3, strQuantity4, strQuantity5, strQuantity6;
			CString strChargeAmt1, strChargeAmt2, strChargeAmt3, strChargeAmt4, strChargeAmt5, strChargeAmt6;

			//only get legitimate 5-digit numeric CPT codes
			//sigh... we also need to support CPT codes with 6 or 7 characters if the first 5 are legitimate,
			//the last character or two is for revision indicators
			_RecordsetPtr rsCharges = CreateParamRecordset(FormatString("SELECT TOP 6 "
				"Code, CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, LineItemT.Date, "
				"Convert(int,Round(Quantity,0)) AS Quantity, Round(dbo.GetChargeTotal(ChargesT.ID),0) AS ChargeTotal "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"WHERE Deleted = 0 AND BillID = {INT} "
				"AND ISNUMERIC(Left(CPTCodeT.Code,5)) = 1 AND Len(CPTCodeT.Code) >= 5 AND Len(CPTCodeT.Code) <= 7 "
				"AND Left(CPTCodeT.Code,5) IN (%s) "
				"ORDER BY LineID", m_strAllowedCPTCodes), nBillID);

			nCount = 0;

			while(!rsCharges->eof) {

				nCount++;

				CString strCode = AdoFldString(rsCharges, "Code","");
				CString strCPTModifier1 = AdoFldString(rsCharges, "CPTModifier","");
				CString strCPTModifier2 = AdoFldString(rsCharges, "CPTModifier2","");
				CString strCPTModifier3 = AdoFldString(rsCharges, "CPTModifier3","");
				CString strCPTModifier4 = AdoFldString(rsCharges, "CPTModifier4","");
				
				COleDateTime dt = AdoFldDateTime(rsCharges, "Date");
				CString strDate = dt.Format("%m/%d/%Y");

				long nQuantity = AdoFldLong(rsCharges, "Quantity");
				CString strQuantity;
				strQuantity.Format("%li",nQuantity);

				COleCurrency cyAmount = AdoFldCurrency(rsCharges, "ChargeTotal");
				CString strAmount = cyAmount.Format();

				if(nCount==1) {
					strCPTCode1 = strCode;
					strCPTModifier1a = strCPTModifier1;
					strCPTModifier1b = strCPTModifier2;
					strCPTModifier1c = strCPTModifier3;
					strCPTModifier1d = strCPTModifier4;
					strCPTDate1 = strDate;
					strQuantity1 = strQuantity;
					strChargeAmt1 = strAmount;
				}
				else if(nCount==2) {
					strCPTCode2 = strCode;
					strCPTModifier2a = strCPTModifier1;
					strCPTModifier2b = strCPTModifier2;
					strCPTModifier2c = strCPTModifier3;
					strCPTModifier2d = strCPTModifier4;
					strCPTDate2 = strDate;
					strQuantity2 = strQuantity;
					strChargeAmt2 = strAmount;
				}
				else if(nCount==3) {
					strCPTCode3 = strCode;
					strCPTModifier3a = strCPTModifier1;
					strCPTModifier3b = strCPTModifier2;
					strCPTModifier3c = strCPTModifier3;
					strCPTModifier3d = strCPTModifier4;
					strCPTDate3 = strDate;
					strQuantity3 = strQuantity;
					strChargeAmt4 = strAmount;
				}
				else if(nCount==4) {
					strCPTCode4 = strCode;
					strCPTModifier4a = strCPTModifier1;
					strCPTModifier4b = strCPTModifier2;
					strCPTModifier4c = strCPTModifier3;
					strCPTModifier4d = strCPTModifier4;
					strCPTDate4 = strDate;
					strQuantity4 = strQuantity;
					strChargeAmt4 = strAmount;
				}
				else if(nCount==5) {
					strCPTCode5 = strCode;
					strCPTModifier5a = strCPTModifier1;
					strCPTModifier5b = strCPTModifier2;
					strCPTModifier5c = strCPTModifier3;
					strCPTModifier5d = strCPTModifier4;
					strCPTDate5 = strDate;
					strQuantity5 = strQuantity;
					strChargeAmt5 = strAmount;
				}
				else if(nCount==6) {
					strCPTCode6 = strCode;
					strCPTModifier6a = strCPTModifier1;
					strCPTModifier6b = strCPTModifier2;
					strCPTModifier6c = strCPTModifier3;
					strCPTModifier6d = strCPTModifier4;
					strCPTDate6 = strDate;
					strQuantity6 = strQuantity;
					strChargeAmt6 = strAmount;
				}
				rsCharges->MoveNext();
			}
			rsCharges->Close();

			/////////////////////////////////////////////////////////////////////////////////
			//58. Procedure 1 (CPT)					PIC X(5)	530 534
			strOutput += OutputString(strCPTCode1,5);

			/////////////////////////////////////////////////////////////////////////////////
			//59. Procedure 2 (CPT)					PIC X(5)	535 539
			strOutput += OutputString(strCPTCode2,5);

			/////////////////////////////////////////////////////////////////////////////////
			//60. Procedure 3 (CPT)					PIC X(5)	540 544
			strOutput += OutputString(strCPTCode3,5);

			/////////////////////////////////////////////////////////////////////////////////
			//61. Procedure 4 (CPT)					PIC X(5)	545 549
			strOutput += OutputString(strCPTCode4,5);

			/////////////////////////////////////////////////////////////////////////////////
			//62. Procedure 5 (CPT)					PIC X(5)	550 554
			strOutput += OutputString(strCPTCode5,5);

			/////////////////////////////////////////////////////////////////////////////////
			//63. Procedure 6 (CPT)					PIC X(5)	555 559
			strOutput += OutputString(strCPTCode6,5);

			/////////////////////////////////////////////////////////////////////////////////
			//64. CPT Modifier 1-a					PIC X(2)	560 561
			strOutput += OutputString(strCPTModifier1a,2);

			/////////////////////////////////////////////////////////////////////////////////
			//65. CPT Modifier 1-b					PIC X(2)	562 563
			strOutput += OutputString(strCPTModifier1b,2);

			/////////////////////////////////////////////////////////////////////////////////
			//66. CPT Modifier 1-c					PIC X(2)	564 565
			strOutput += OutputString(strCPTModifier1c,2);

			/////////////////////////////////////////////////////////////////////////////////
			//67. CPT Modifier 1-d					PIC X(2)	566 567
			strOutput += OutputString(strCPTModifier1d,2);

			/////////////////////////////////////////////////////////////////////////////////
			//68. CPT Modifier 2-a					PIC X(2)	568 569
			strOutput += OutputString(strCPTModifier2a,2);

			/////////////////////////////////////////////////////////////////////////////////
			//69. CPT Modifier 2-b					PIC X(2)	570 571
			strOutput += OutputString(strCPTModifier2b,2);

			/////////////////////////////////////////////////////////////////////////////////
			//70. CPT Modifier 2-c					PIC X(2)	572 573
			strOutput += OutputString(strCPTModifier2c,2);

			/////////////////////////////////////////////////////////////////////////////////
			//71. CPT Modifier 2-d					PIC X(2)	574 575
			strOutput += OutputString(strCPTModifier2d,2);

			/////////////////////////////////////////////////////////////////////////////////
			//72. CPT Modifier 3-a					PIC X(2)	576 577
			strOutput += OutputString(strCPTModifier3a,2);

			/////////////////////////////////////////////////////////////////////////////////
			//73. CPT Modifier 3-b					PIC X(2)	578 579
			strOutput += OutputString(strCPTModifier3b,2);

			/////////////////////////////////////////////////////////////////////////////////
			//74. CPT Modifier 3-c					PIC X(2)	580 581
			strOutput += OutputString(strCPTModifier3c,2);

			/////////////////////////////////////////////////////////////////////////////////
			//75. CPT Modifier 3-d					PIC X(2)	582 583
			strOutput += OutputString(strCPTModifier3d,2);

			/////////////////////////////////////////////////////////////////////////////////
			//76. CPT Modifier 4-a					PIC X(2)	584 585
			strOutput += OutputString(strCPTModifier4a,2);

			/////////////////////////////////////////////////////////////////////////////////
			//77. CPT Modifier 4-b					PIC X(2)	586 587
			strOutput += OutputString(strCPTModifier4b,2);

			/////////////////////////////////////////////////////////////////////////////////
			//78. CPT Modifier 4-c					PIC X(2)	588 589
			strOutput += OutputString(strCPTModifier4c,2);

			/////////////////////////////////////////////////////////////////////////////////
			//79. CPT Modifier 4-d					PIC X(2)	590 591
			strOutput += OutputString(strCPTModifier4d,2);

			/////////////////////////////////////////////////////////////////////////////////
			//80. CPT Modifier 5-a					PIC X(2)	592 593
			strOutput += OutputString(strCPTModifier5a,2);

			/////////////////////////////////////////////////////////////////////////////////
			//81. CPT Modifier 5-b					PIC X(2)	594 595
			strOutput += OutputString(strCPTModifier5b,2);

			/////////////////////////////////////////////////////////////////////////////////
			//82. CPT Modifier 5-c					PIC X(2)	596 597
			strOutput += OutputString(strCPTModifier5c,2);

			/////////////////////////////////////////////////////////////////////////////////
			//83. CPT Modifier 5-d					PIC X(2)	598 599
			strOutput += OutputString(strCPTModifier5d,2);

			/////////////////////////////////////////////////////////////////////////////////
			//84. CPT Modifier 6-a					PIC X(2)	600 601
			strOutput += OutputString(strCPTModifier6a,2);

			/////////////////////////////////////////////////////////////////////////////////
			//85. CPT Modifier 6-b					PIC X(2)	602 603
			strOutput += OutputString(strCPTModifier6b,2);

			/////////////////////////////////////////////////////////////////////////////////
			//86. CPT Modifier 6-c					PIC X(2)	604 605
			strOutput += OutputString(strCPTModifier6c,2);

			/////////////////////////////////////////////////////////////////////////////////
			//87. CPT Modifier 6-d					PIC X(2)	606 607
			strOutput += OutputString(strCPTModifier6d,2);

			/////////////////////////////////////////////////////////////////////////////////
			//88. Procedure 1 From Date				PIC 9(8)	608 615
			strOutput += OutputString(strCPTDate1,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//89. Principal Procedure Date or Procedure 1 Thru Date		PIC 9(8)	616 623
			strOutput += OutputString(strCPTDate1,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//90. Procedure 2 From Date				PIC 9(8)	624 631
			strOutput += OutputString(strCPTDate2,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//91. Other Procedure A Date or Procedure 2 Thru Date		PIC 9(8)	632 639
			strOutput += OutputString(strCPTDate2,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//92. Procedure 3 From Date				PIC 9(8)	640 647
			strOutput += OutputString(strCPTDate3,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//93. Other Procedure B Date or Procedure 3 Thru Date		PIC 9(8)	648 655
			strOutput += OutputString(strCPTDate3,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//94. Procedure 4 From Date				PIC 9(8)	656 663
			strOutput += OutputString(strCPTDate4,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//95. Other Procedure C Date or Procedure 4 Thru Date		PIC 9(8)	664 671
			strOutput += OutputString(strCPTDate4,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//96. Procedure 5 From Date				PIC 9(8)	672 679
			strOutput += OutputString(strCPTDate5,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//97. Other Procedure D Date or Procedure 5 Thru Date		PIC 9(8)	680 687
			strOutput += OutputString(strCPTDate5,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//98. Procedure 6 From Date				PIC 9(8)	688 695
			strOutput += OutputString(strCPTDate6,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//99. Other Procedure E Date or Procedure 6 Thru Date		PIC 9(8)	696 703
			strOutput += OutputString(strCPTDate6,8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//100. Revenue Center Code 1			PIC 9(4)	704 707

			//Albert's old data file thankfully left all these blank
			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//101. Revenue Center Units 1			PIC S(7)	708 714

			strOutput += OutputString(strQuantity1,7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//102. Revenue Center Charges 1			PIC S(8)	715 722

			strOutput += OutputString(strChargeAmt1,8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//103. Revenue Center Code 2			PIC 9(4)	723 726

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//104. Revenue Center Units 2			PIC S(7)	727 733

			strOutput += OutputString(strQuantity2,7,TRUE);
				
			/////////////////////////////////////////////////////////////////////////////////
			//105. Revenue Center Charges 2			PIC S(8)	734 741

			strOutput += OutputString(strChargeAmt2,8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//106. Revenue Center Code 3			PIC 9(4)	742 745

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//107. Revenue Center Units 3			PIC S(7)	746 752

			strOutput += OutputString(strQuantity3,7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//108. Revenue Center Charges 3			PIC S(8)	753 76

			strOutput += OutputString(strChargeAmt3,8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//109. Revenue Center Code 4			PIC 9(4)	761 764

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//110. Revenue Center Units 4			PIC S(7)	765 771

			strOutput += OutputString(strQuantity4,7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//111. Revenue Center Charges 4			PIC S(8)	772 779

			strOutput += OutputString(strChargeAmt4,8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//112. Revenue Center Code 5			PIC 9(4)	780 783

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//113. Revenue Center Units 5			PIC S(7)	784 790

			strOutput += OutputString(strQuantity5,7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//114. Revenue Center Charges 5			PIC S(8)	791 798

			strOutput += OutputString(strChargeAmt5,8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//115. Revenue Center Code 6			PIC 9(4)	799 802

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//116. Revenue Center Units 6			PIC S(7)	803 809

			strOutput += OutputString(strQuantity6,7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//117. Revenue Center Charges 6			PIC S(8)	810 817

			strOutput += OutputString(strChargeAmt6,8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//118. Revenue Center Code 7			PIC 9(4)	818 821

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//119. Revenue Center Units 7			PIC S(7)	822 828

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//120. Revenue Center Charges 7			PIC S(8)	829 836

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//121. Revenue Center Code 8			PIC 9(4)	837 840

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//122. Revenue Center Units 8			PIC S(7)	841 847

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//123. Revenue Center Charges 8			PIC S(8)	848 855

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//124. Revenue Center Code 9			PIC 9(4)	856 859

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//125. Revenue Center Units 9			PIC S(7)	860 866

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//126. Revenue Center Charges 9			PIC S(8)	867 874

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//127. Revenue Center Code 10			PIC 9(4)	875 878

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//128. Revenue Center Units 10			PIC S(7)	879 88

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//129. Revenue Center Charges 10		PIC S(8)	886 893

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//130. Revenue Center Code 11			PIC 9(4)	894 897

			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//131 Revenue Center Units 11			PIC S(7)	898 904

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//132 Revenue Center Charges 11			PIC S(8)	905 912

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//133 Revenue Center Code 12			PIC 9(4)	913 916

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//134 Revenue Center Units 12			PIC S(7)	917 923

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//135 Revenue Center Charges 12			PIC S(8)	924 931

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//136 Revenue Center Code 13			PIC 9(4)	932 935

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//137 Revenue Center Units 13			PIC S(7)	936 942

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//138 Revenue Center Charges 13			PIC S(8)	943 950

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//139 Revenue Center Code 14			PIC 9(4)	951 954

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//140 Revenue Center Units 14			PIC S(7)	955 961

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//141 Revenue Center Charges 14			PIC S(8)	962 969

			strOutput += OutputString("",8,TRUE);
						
			/////////////////////////////////////////////////////////////////////////////////
			//142 Revenue Center Code 15			PIC 9(4)	970 973

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//143 Revenue Center Units 15			PIC S(7)	974 980

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//144 Revenue Center Charges 15			PIC S(8)	981 988

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//145 Revenue Center Code 16			PIC 9(4)	989 992

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//146 Revenue Center Units 16			PIC S(7)	993 999

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//147 Revenue Center Charges 16			PIC S(8)	1000 1007

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//148 Revenue Center Code 17			PIC 9(4)	1008 1011

			strOutput += OutputString("",4,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//149 Revenue Center Units 17			PIC S(7)	1012 1018

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//150 Revenue Center Charges 17			PIC S(8)	1019 1026

			strOutput += OutputString("",8,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//151 Revenue Center Code 18			PIC 9(4)	1027 1030

			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//152 Revenue Center Units 18			PIC S(7)	1031 1037

			strOutput += OutputString("",7,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//153 Revenue Center Charges 18			PIC S(8)	1038 1045

			strOutput += OutputString("",8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//154 Revenue Center Code 19			PIC 9(4)	1046 1049

			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//155 Revenue Center Units 19			PIC S(7)	1050 1056

			strOutput += OutputString("",7,TRUE);
			
			/////////////////////////////////////////////////////////////////////////////////
			//156 Revenue Center Charges 19			PIC S(8)	1057 1064

			strOutput += OutputString("",8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//157 Revenue Center Code 20			PIC 9(4)	1065 1068

			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//158 Revenue Center Units 20			PIC S(7)	1069 1075

			strOutput += OutputString("",7,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//159 Revenue Center Charges 20			PIC S(8)	1076 1083

			strOutput += OutputString("",8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//160 Revenue Center Code 21			PIC 9(4)	1084 1087 

			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//161 Revenue Center Units 21			PIC S(7)	1088 1094

			strOutput += OutputString("",7,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//162 Revenue Center Charges 21			PIC S(8)	1095 1102

			strOutput += OutputString("",8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//163 Revenue Center Code 22			PIC 9(4)	1103 1106

			strOutput += OutputString("",4,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//164 Revenue Center Units 22			PIC S(7)	1107 1113

			strOutput += OutputString("",7,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//165 Revenue Center Charges 22			PIC S(8)	1114 1121

			strOutput += OutputString("",8,TRUE);

			/////////////////////////////////////////////////////////////////////////////////
			//166. Total Charges					PIC S(8)	1122 1129

			str = "";
			_RecordsetPtr rsBillTotal = CreateParamRecordset(FormatString("SELECT Sum(ChargeTotal) AS BillTotal FROM (SELECT TOP 6 "
				"Round(dbo.GetChargeTotal(ChargesT.ID),0) AS ChargeTotal "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"WHERE Deleted = 0 AND BillID = {INT} "
				"AND ISNUMERIC(Left(CPTCodeT.Code,5)) = 1 AND Len(CPTCodeT.Code) >= 5 AND Len(CPTCodeT.Code) <= 7 "
				"AND Left(CPTCodeT.Code,5) IN (%s)) AS ChargesQ", m_strAllowedCPTCodes), nBillID);
			if(!rsBillTotal->eof) {
				COleCurrency cyTotal = AdoFldCurrency(rsBillTotal, "BillTotal", COleCurrency(0,0));
				str = cyTotal.Format();
			}
			rsBillTotal->Close();

			strOutput += OutputString(str,8,TRUE);

			ASSERT(strOutput.GetLength() == 1129);

			strOutput += "\n";
			m_fExport.WriteString(strOutput);

			m_progressbar.StepIt();

			rsBills->MoveNext();
		}
		rsBills->Close();

		m_fExport.Close();

		m_progressbar.SetRange(1,100);
		m_progressbar.SetPos(100);

		AfxMessageBox("The VHI export is complete.");

		m_progressbar.SetPos(0);
		
	}NxCatchAll("Error opening VHI export file.");
}

void CVAASCExportDlg::OnBtnExportList() 
{
	try {

		CWaitCursor pWait;

		CString str;
		_variant_t var;

		long nPOSID = -1;
		IRowSettingsPtr pRow = m_POSCombo->GetCurSel();
		if(pRow != NULL)
			nPOSID = VarLong(pRow->GetValue(0),-1);

		long nPOSTwoID = -1;
		pRow = m_POS2Combo->GetCurSel();
		if(pRow != NULL)
			nPOSTwoID = VarLong(pRow->GetValue(0),-1);

		CString strHospID = "";
		GetDlgItemText(IDC_EDIT_HOSPITAL_IDENTIFIER, strHospID);
		strHospID.TrimRight();
		if(strHospID.IsEmpty()) {
			AfxMessageBox("You must enter a Hospital Identifier number for the export.");
			return;
		}
		else {
			//save it for the next time we use the export
			SetRemotePropertyText("ASCHospitalIdentifier", strHospID, 0, "<None>");
		}

		CSqlFragment sqlPOSTwoFilter;
		if(nPOSTwoID != -1) {
			sqlPOSTwoFilter = CSqlFragment(" OR BillsT.Location = {INT}", nPOSTwoID);
		}

		CSqlFragment sqlPOSFilter;
		if(nPOSID != -1) {
			sqlPOSFilter = CSqlFragment("AND (BillsT.Location = {INT} {SQL})", nPOSID, sqlPOSTwoFilter);
		}

		//this pulls up all bills in the date range with at least one of the required CPT codes
		_RecordsetPtr rsBills = CreateParamRecordset(FormatString("SELECT PersonT.ID AS PatientID, BillsT.ID AS BillID, "
				"UserDefinedID, Last + ', ' + First + ' ' + Middle AS PatientName, "
				"BillsT.Date AS BillDate " 
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID "
				"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.Date >= {OLEDATETIME} AND BillsT.Date < DATEADD(day, 1, {OLEDATETIME}) "
				"{SQL} "
				"AND BillsT.ID IN (SELECT BillID FROM ChargesT "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"	WHERE Deleted = 0 AND ISNUMERIC(Left(CPTCodeT.Code,5)) = 1 "
				"	AND Len(CPTCodeT.Code) >= 5 AND Len(CPTCodeT.Code) <= 7 "
				"	AND Left(CPTCodeT.Code,5) IN (%s)) "
				"ORDER BY BillsT.Date, BillsT.ID", m_strAllowedCPTCodes),
				VarDateTime(m_dtFrom.GetValue()), VarDateTime(m_dtTo.GetValue()),
				sqlPOSFilter);

		if(rsBills->eof) {
			AfxMessageBox("There are no applicable bills to export in the given date range for the selected place of service.");
			return;
		}

		//Now open the file
		CFileException	e;
		CArchive ar( &m_fExport, CArchive::store, 4096, NULL );

		CString strDefName = "VHIExportedList.txt";

		CFileDialog SaveAs(FALSE,NULL,strDefName);
		// (j.armen 2011-10-25 15:57) - PLID 46139 - We are prompting the user, so use the practice path
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		CString dir = GetEnvironmentDirectory();
		SaveAs.m_ofn.lpstrInitialDir = (LPCSTR)dir;
		SaveAs.m_ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
		if (SaveAs.DoModal() == IDCANCEL) {
			return;
		}

		CString strFileName = SaveAs.GetPathName();

		try 
		{
			m_fExport.Open(strFileName, CFile::modeCreate | CFile::modeWrite, &e);
		}
		catch (CFileException *e) 
		{
			MessageBox(strFileName + ": Access Violation\nFile may be in use");
			m_fExport.Close();
			if (e) e->Delete();
			return;
		}

		m_progressbar.SetRange(1,(short)(rsBills->GetRecordCount()));
		m_progressbar.SetStep(1);

		CWaitCursor pWait2;

		//now export the list

		CString strOut;
		strOut += OutputString("Patient ID", 15);
		strOut += OutputString("Patient Name", 40);
		strOut += OutputString("Bill Date", 12);
		strOut += OutputString("Bill Total", 15);
		strOut += OutputString("CPT Codes", 15);
		strOut += "\n";
		m_fExport.WriteString(strOut);

		while(!rsBills->eof) {

			long nPatientID = AdoFldLong(rsBills, "PatientID");
			long nBillID = AdoFldLong(rsBills, "BillID");

			CString strOutput = "";
			strOutput += OutputString(AsString(AdoFldLong(rsBills, "UserDefinedID")), 15);
			strOutput += OutputString(AdoFldString(rsBills, "PatientName"), 40);
			strOutput += OutputString(FormatDateTimeForSql(AdoFldDateTime(rsBills, "BillDate"), dtoDate),12);

			_RecordsetPtr rsBillTotal = CreateRecordset("SELECT Sum(ChargeTotal) AS BillTotal FROM (SELECT TOP 6 "
				"Round(dbo.GetChargeTotal(ChargesT.ID),0) AS ChargeTotal "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"WHERE Deleted = 0 AND BillID = %li "
				"AND ISNUMERIC(Left(CPTCodeT.Code,5)) = 1 AND Len(CPTCodeT.Code) >= 5 AND Len(CPTCodeT.Code) <= 7 "
				"AND Left(CPTCodeT.Code,5) IN (%s)) AS ChargesQ", nBillID, m_strAllowedCPTCodes);
			if(!rsBillTotal->eof) {
				strOutput += OutputString(AsString(rsBillTotal->Fields->Item["BillTotal"]->Value), 15);
			}
			rsBillTotal->Close();			

			//only get legitimate 5-digit numeric CPT codes
			//sigh... we also need to support CPT codes with 6-7 characters if the first 5 are legitimate,
			//the last character or two is for revision indicators
			_RecordsetPtr rsCharges = CreateRecordset("SELECT TOP 6 "
				"Code, CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, LineItemT.Date, "
				"Convert(int,Round(Quantity,0)) AS Quantity, Round(dbo.GetChargeTotal(ChargesT.ID),0) AS ChargeTotal "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"WHERE Deleted = 0 AND BillID = %li "
				"AND ISNUMERIC(Left(CPTCodeT.Code,5)) = 1 AND Len(CPTCodeT.Code) >= 5 AND Len(CPTCodeT.Code) <= 7 "
				"AND Left(CPTCodeT.Code,5) IN (%s) "
				"ORDER BY LineID", nBillID, m_strAllowedCPTCodes);

			while(!rsCharges->eof) {

				strOutput += OutputString(AdoFldString(rsCharges, "Code",""), 8);
				
				rsCharges->MoveNext();
			}
			rsCharges->Close();			

			strOutput += "\n";
			m_fExport.WriteString(strOutput);

			m_progressbar.StepIt();

			rsBills->MoveNext();
		}
		rsBills->Close();

		m_fExport.Close();

		m_progressbar.SetRange(1,100);
		m_progressbar.SetPos(100);

		AfxMessageBox("The list export is complete.");

		m_progressbar.SetPos(0);

	}NxCatchAll("Error opening export list.");
}

BEGIN_EVENTSINK_MAP(CVAASCExportDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CVAASCExportDlg)
	ON_EVENT(CVAASCExportDlg, IDC_VA_ASC_POS_COMBO, 16 /* SelChosen */, OnSelChosenVaAscPosCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CVAASCExportDlg::OnSelChosenVaAscPosCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		//disable the secondary POS combo if the first one is showing all
		long nPOSID = -1;
		if(pRow != NULL)
			nPOSID = VarLong(pRow->GetValue(0), -1);

		m_POS2Combo->Enabled = nPOSID != -1;

	}NxCatchAll("Error in CVAASCExportDlg::OnSelChosenVaAscPosCombo");
}

