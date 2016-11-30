// LetterWriting.cpp : implementation file
//

//Assumes no patient can have the last name "MultiPatDoc" which signifies a multiple patient document
//Assumes templates are less than 4GB in size
#include "stdafx.h"
#include "practice.h"
#include "LetterWriting.h"
#include "LetterWritingRc.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "Groups.h"
#include "FilterEditDlg.h"
#include "SuperBill.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "NxException.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "MergeEngine.h"
#include "SelectPacketDlg.h"
#include "EditComboBox.h"
#include "SingleSelectDlg.h"
#include "DontShowDlg.h"
#include "MsgBox.h"
#include "PatientReminderSenthistoryUtils.h"

//TES 11/7/2007 - PLID 27981 - VS2008 - VS 2008 doesn't recognize this file, which is fine, because nothing
// appears to actually need it.
//#include <fstream.h>

#include "GlobalDataUtils.h"

#include "EMRSetupDlg.h"
#include "EMRUtils.h"

using namespace NXDATALISTLib;

///////////////////////
// Some handy utilities

CString CreateTempIDTable(const CString &strSqlSource, const CStringArray &aryFieldNames, const CStringArray &aryFieldTypes, BOOL bIncludeRecordNumber, BOOL bSkipDups, OPTIONAL OUT long *pnRecordCount)
{
	ASSERT(aryFieldNames.GetSize() == aryFieldTypes.GetSize());
	if (aryFieldNames.GetSize() != aryFieldTypes.GetSize()) {
		ThrowNxException(
			"CreateTempIDTable: Incorrect list of field names and types.  The entry count "
			"for the 'Names' list (%li) did not match the entry count for the 'Types' list (%li).", 
			aryFieldNames.GetSize(), aryFieldTypes.GetSize());
	}

	// Calculate the decl string and the name list string based on the array entries
	CString strFieldDecl, strFieldNameOutputList, strFieldNameInputList;
	long nCount = aryFieldNames.GetSize();
	for (long i=0; i<nCount; i++) {
		// Add the "name type" to the decl
		CString strCurField;
		strCurField.Format("%s %s, ", aryFieldNames.GetAt(i), aryFieldTypes.GetAt(i));
		strFieldDecl += strCurField;
		// Add the "name" to the field name list
		strFieldNameOutputList += "" + aryFieldNames.GetAt(i) + ", ";
		// Add the "Q.name" to the field name list
		strFieldNameInputList += "Q." + aryFieldNames.GetAt(i) + ", ";
	}
	// Finish the decl
	if (bIncludeRecordNumber) {
		strFieldDecl += "RowNumber INT IDENTITY";
	} else {
		// Just drop off the ", " at the ends of the field decl
		if (strFieldDecl.Right(2) == ", ") {
			strFieldDecl.Delete(strFieldDecl.GetLength() - 2, 2);
		}
	}
	// Finish the field name lists by dropping off the ", " at the end
	if (strFieldNameOutputList.Right(2) == ", ") {
		strFieldNameOutputList.Delete(strFieldNameOutputList.GetLength() - 2, 2);
	}
	if (strFieldNameInputList.Right(2) == ", ") {
		strFieldNameInputList.Delete(strFieldNameInputList.GetLength() - 2, 2);
	}

	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it

	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#TempMerge%lu", GetTickCount());
	ExecuteSql("CREATE TABLE %s (%s)", strTempT, strFieldDecl);

	// (a.walling 2011-05-27 10:58) - PLID 43866 - Added SET NOCOUNT OFF here, since it may be ON, and
	// if we don't get a proper record count we'll end up merging anyway yet encountering a EOF/BOF error
	// when there are no records returned!

	// Loop through the datalist
	ExecuteSql(pnRecordCount, ADODB::adCmdText, 
		"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
		"INSERT INTO %s (%s) SELECT %s %s FROM (%s) Q", 
		strTempT, // The dest table (e.g. #TempWhatever123)
		strFieldNameOutputList, // The field name list (e.g. "ID, PatientID, etcID")
		(bSkipDups ? "DISTINCT" : ""), // DISTINCT or not, depending on bSkipDups
		strFieldNameInputList, // The field name list with the Q. in front (e.g. "Q.ID, Q.PatientID, Q.etcID")
		strSqlSource // The input query (e.g. "SELECT ID, PersonID AS PatientID FROM PatientsT")
		);

	// Return the name of the temp table
	return strTempT;
}

CString CreateTempIDTable(const CString &strSqlSource, const CString &strIDFieldName, BOOL bIncludeRecordNumber /*= TRUE*/, BOOL bSkipDups /*= TRUE*/, OUT long *pnRecordCount /*= NULL*/)
{
	CStringArray aryFieldNames, aryFieldTypes;
	aryFieldNames.Add(strIDFieldName);
	aryFieldTypes.Add("INT");

	return CreateTempIDTable(strSqlSource, aryFieldNames, aryFieldTypes, bIncludeRecordNumber, bSkipDups, pnRecordCount);
}

// (a.walling 2010-08-27 17:18) - PLID 39965 - Removed SQL 7 version

// XML Implementation
//
// Creates a temporary table (will be deleted automatically) and fills it 
// with the IDs from the given datalist (the given column must be of type 
// long integer; NULLs, EMPTYs, or any other data type will cause an 
// exception to be thrown); consecutive duplicates will be ignored if desired
// 
// Example output:
//      ID     |   RowNumber
//    ---------+------------
//    8327     |       1
//    504	   |       2
//    6490	   |       3
//    7976	   |       4
//    24080	   |       5
//
// If bAppointmentBasedMerge is TRUE then the function creates a temporary
// table as above but with one more column called "Patient ID".  This 
// Patient ID column must be directly after the ID column in the data list.
//
// Example output:
//
//      ID     | Patient ID  |   RowNumber
//    ---------+-------------+-------------
//    8327     |    565      |      1
//    504	   |    3565     |      2
//    6490	   |    347      |      3
//    7976	   |    2467     |      4
//    24080	   |    2776     |      5
//
CString CreateTempIDTable_80(_DNxDataListPtr pDataList, short nIDColIndex, BOOL bIncludeRecordNumber /*= TRUE*/, BOOL bSkipDups /*= TRUE*/, OUT long *pnRecordCount /*= NULL*/, BOOL bAppointmentBasedMerge /*= FALSE*/)
{
	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it
	
	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#Temp%luT", GetTickCount());
	ExecuteSql("CREATE TABLE %s (ID int%s%s)", strTempT, bAppointmentBasedMerge ? ", PatientID INT" : "", bIncludeRecordNumber ? ", RowNumber INT IDENTITY" : "");

	// Start at a string length of 2^17 (we'll double this if we go out of bounds)
	long nMaxLen = 1<<17;
	long nLen = 0;
	long nSubLen;
	CString str, strXml;
	TCHAR *pstr = strXml.GetBuffer(nMaxLen);
	// We're building a simple XML string here ('<ROOT><RECORD ID="123" /><RECORD ID="456" /></ROOT>')
	// Or in the case that bAppointmentBasedMerge is TRUE:
	// We're building this XML string ('<ROOT><RECORD ID="123" PatientID="5448"/><RECORD ID="456" PatientID="822"/></ROOT>')
	nSubLen = sprintf(pstr, "<ROOT>");
	pstr += nSubLen;
	nLen += nSubLen;
	// Loop through the datalist
	LONG nLastIDVal = -1;
	long i = 0;
	long p = pDataList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while (p != NULL) {
		// Get the next row in the datalist while at the same time getting this a smart-pointer to the current row
		pDataList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp);
		lpDisp->Release();
		LONG nIDVal = VarLong(pRow->Value[nIDColIndex]), nIDVal2;
		if (bAppointmentBasedMerge)
			nIDVal2 = VarLong(pRow->Value[nIDColIndex+1]);
		if ((!bSkipDups) || i==0 || (nIDVal != nLastIDVal)) { // IF (don't skip duplicates OR this is our first iteration OR this one is different from the last one)
			// Remember this one for the next iteration
			nLastIDVal = nIDVal;

			// Create a string that will represent the <RECORD ID="123" /> part 
			// of our XML (we use "P" instead of "RECORD" because this could be 
			// a huge string and we want it to be as efficient as possible)
			if (bAppointmentBasedMerge)
				str.Format("<P ID=\"%li\" PatientID=\"%li\" />", nIDVal, nIDVal2);
			else
				str.Format("<P ID=\"%li\" />", nIDVal);
			
			// Here's the fun part: if this substring would make our XML larger 
			// than the currently allocated space, we want to double the currently 
			// allocated space until it would fit
			while (nLen+str.GetLength() >= nMaxLen-1) {
				strXml.ReleaseBuffer();
				nMaxLen = nMaxLen * 2;
				pstr = strXml.GetBuffer(nMaxLen) + nLen;
				ASSERT(pstr[0] == '\0');
			}

			// Write the new substring to the end of the big xml string
			nSubLen = sprintf(pstr, str);
			pstr += nSubLen;
			nLen += nSubLen;
			
			// Increment our count
			i++;
		}
	}
	// Close the XML
	strXml.ReleaseBuffer();
	strXml += "</ROOT>";
	
	// Add the rows from the XML into our temp table
	if (bAppointmentBasedMerge)
		ExecuteSql(
			"DECLARE @hDoc AS INT; "  // We need a document handle
//			"DECLARE @doc nvarchar(4000); "
//			"SET @doc = CONVERT(nvarchar, '%s'); " // Put the text into a variable so that it is compatible with international characters
// (a.walling 2006-12-05 18:57) - PLID 23779 - Local vars are limited to 4000 nvarchars, so instead we can just pass the raw xml
//		directly into the stored procedure. It will implicitly become 'text' type and handle a large amount.
			"EXEC sp_xml_preparedocument @hDoc OUTPUT, N'%s'; " // Ask SQL to parse the XML (returns the document handle)
			"INSERT INTO %s (ID, PatientID) SELECT ID, PatientID FROM OPENXML(@hDoc, '/ROOT/P') WITH (ID int, PatientID int); " // Insert into the table
			"EXEC sp_xml_removedocument @hDoc;",  // Release our document handle
			strXml, strTempT);
	else
		ExecuteSql(
			"DECLARE @hDoc AS INT; "  // We need a document handle
//			"DECLARE @doc nvarchar(4000); "
//			"SET @doc = CONVERT(nvarchar, '%s'); " // Put the text into a variable so that it is compatible with international characters
// (a.walling 2006-12-05 18:57) - PLID 23779 - Local vars are limited to 4000 nvarchars, so instead we can just pass the raw xml
//		directly into the stored procedure. It will implicitly become 'text' type and handle a large amount.
			"EXEC sp_xml_preparedocument @hDoc OUTPUT, N'%s'; " // Ask SQL to parse the XML (returns the document handle)
			"INSERT INTO %s (ID) SELECT ID FROM OPENXML(@hDoc, '/ROOT/P') WITH (ID int); " // Insert into the table
			"EXEC sp_xml_removedocument @hDoc;",  // Release our document handle
			strXml, strTempT);

	// If the caller wants it, give the record count
	if (pnRecordCount) {
		*pnRecordCount = i;
	}

	// Return the name of the temp table
	return strTempT;
}

// (a.walling 2010-08-27 17:18) - PLID 39965 - This was a horrible, but robust, way of finding out if the engine we're using supports XML
// But now SQL 2000 is the minimum, so this can go away. Bye!
//BOOL DoesSqlSupportXml()
//{
//	// TODO: This is a horrible (but robust as far as I can tell) way of finding out if the engine we're using supports XML
//	try {
//		ExecuteSql("OPENXML");
//		ASSERT(FALSE); // no matter what I can't see how the above statement can get by without throwing an exception
//		return FALSE;
//	} catch (_com_error e) {
//		// If this was the only error added to the connection's errors collection, then clear the collection
//		extern ADODB::_ConnectionPtr g_ptrRemoteData;
//		if (g_ptrRemoteData != NULL) {
//			ADODB::ErrorsPtr perrs = g_ptrRemoteData->GetErrors();
//			if (perrs != NULL && perrs->GetCount() == 1) {
//				perrs->Clear();
//			}
//		}
//		// Use this error to determine whether xml is supported or not
//		if (e.Error() == -2147217900 && strcmp((LPCTSTR)e.Description(), "Incorrect syntax near the keyword 'OPENXML'.") == 0) {
//			// XML is supported
//			return TRUE;
//		} else {
//			// Any other exception assumes we don't support XML
//			return FALSE;
//		}
//	}
//}

CString CreateTempIDTable(_DNxDataListPtr pDataList, short nIDColIndex, BOOL bIncludeRecordNumber /*= TRUE*/, BOOL bSkipDups /*= TRUE*/, OUT long *pnRecordCount /*= NULL*/, BOOL bAppointmentBasedMerge /*= FALSE*/)
{
	// (a.walling 2010-08-27 17:18) - PLID 39965 - We require at least SQL 2000 now.
	return CreateTempIDTable_80(pDataList, nIDColIndex, bIncludeRecordNumber, bSkipDups, pnRecordCount, bAppointmentBasedMerge);
}

using namespace NXDATALISTLib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CLetterWriting, CNxDialog);

/////////////////////////////////////////////////////////////////////////////
// CLetterWriting dialog

// (m.hancock 2006-09-22 17:12) - PLID 21965 - Enumeration for the sort order combo columns
enum SortOrderComboColumns {
	socID = 0,
	socDescription,
};

CLetterWriting::CLetterWriting(CWnd* pParent)
	: CNxDialog(CLetterWriting::IDD, pParent),
	m_coordChecker(NetUtils::Coordinators)
	, m_groupEditor(*(new CGroups(this)))
{
	//{{AFX_DATA_INIT(CLetterWriting)
	//}}AFX_DATA_INIT
}

// (j.jones 2016-04-15 09:09) - NX-100214 - added destructor
CLetterWriting::~CLetterWriting()
{
	delete &m_groupEditor;
}

void CLetterWriting::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLetterWriting)
	DDX_Control(pDX, IDC_BILL_INFO_CHECK, m_btnIncludeBillInfo);
	DDX_Control(pDX, IDC_CUSTOM_INFO_CHECK, m_btnIncludeCustomInfo);
	DDX_Control(pDX, IDC_DATE_INFO_CHECK, m_btnIncludeDateInfo);
	DDX_Control(pDX, IDC_DOCTOR_INFO, m_btnIncludeDoctorInfo);
	DDX_Control(pDX, IDC_EMR_CHECK, m_btnIncludeEMRInfo);
	DDX_Control(pDX, IDC_INSURANCE_INFO_CHECK, m_btnIncludeInsuranceInfo);
	DDX_Control(pDX, IDC_PERSON_INFO_CHECK, m_btnIncludePersonInfo);
	DDX_Control(pDX, IDC_RESP_INFO_CHECK, m_btnIncludeRespInfo);
	DDX_Control(pDX, IDC_PROCEDURE_INFO_CHECK, m_btnIncludeProcedureInfo);
	DDX_Control(pDX, IDC_PRESCRIPTION_INFO_CHECK, m_btnIncludePrescriptionInfo);
	DDX_Control(pDX, IDC_PRACTICE_INFO_CHECK, m_btnIncludePracticeInfo);
	DDX_Control(pDX, IDC_DONT_EXCLUDE_PRIV_EMAIL, m_btnDoNotIncludePrivateEmail);
	DDX_Control(pDX, IDC_EXPORTONLY_CHECK, m_btnExportCSV);
	DDX_Control(pDX, IDC_MASS_MAIL, m_btnSortForMassMail);
	DDX_Control(pDX, IDC_EMAILHTML_CHECK, m_btnHTMLFormat);
	DDX_Control(pDX, IDC_EMAIL_CHECK, m_btnSendAsEmail);
	DDX_Control(pDX, IDC_DO_NOT_ATTACH, m_btnDoNotAttachToHistory);
	DDX_Control(pDX, IDC_NOINACTIVE_CHECK, m_btnExcludeInactivePatients);
	DDX_Control(pDX, IDC_EXCLUDE_PATIENTS, m_btnDoNotExcludePatientsFromMailings);
	DDX_Control(pDX, IDC_MERGE_TO_PRINTER_CHECK, m_btnMergeDirectToPrinter);
	DDX_Control(pDX, IDC_EDIT_TEMPLATE_BTN, m_editTemplateButton);
	DDX_Control(pDX, IDC_NEW_TEMPLATE_BTN, m_newTemplateButton);
	DDX_Control(pDX, IDC_WRITE_ENVELOPE, m_envelopeButton);
	DDX_Control(pDX, IDC_WRITE_FORM, m_formButton);
	DDX_Control(pDX, IDC_WRITE_LABEL, m_labelButton);
	DDX_Control(pDX, IDC_WRITE_OTHER, m_otherButton);
	DDX_Control(pDX, IDC_WRITE_LETTER, m_letterButton);
	DDX_Control(pDX, IDC_WRITE_PACKET, m_packetButton);
	DDX_Control(pDX, IDC_MERGE, m_nxeditMerge);
	DDX_Control(pDX, IDC_GROUP_LABEL2, m_nxstaticGroupLabel2);
	DDX_Control(pDX, IDC_GROUP_LABEL3, m_nxstaticGroupLabel3);
	DDX_Control(pDX, IDC_GROUPEDITOR, m_btnGroupeditor);
	DDX_Control(pDX, IDC_OPTIONS_GROUPBOX, m_btnOptionsGroupbox);
	DDX_Control(pDX, IDC_INCLUDE_FIELDS_GROUPBOX, m_btnIncludeFieldsGroupbox);
	DDX_Control(pDX, IDC_HIDE_IDENTIFIER_FIELDS_CHECK, m_btnHidePHI);
	DDX_Control(pDX, IDC_PHI_HELP_BTN, m_btnPHIHelp);
	DDX_Control(pDX, IDC_ADD_REMINDER_FOR_PT_MERGED, m_btnAddPatientReminder);
	//}}AFX_DATA_MAP

}

BEGIN_MESSAGE_MAP(CLetterWriting, CNxDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_WRITE_FORM, OnWriteForm)
	ON_BN_CLICKED(IDC_WRITE_LABEL, OnWriteLabel)
	ON_BN_CLICKED(IDC_WRITE_LETTER, OnWriteLetter)
	ON_BN_CLICKED(IDC_WRITE_OTHER, OnWriteOther)
	ON_BN_CLICKED(IDC_WRITE_ENVELOPE, OnWriteEnvelope)
	ON_BN_CLICKED(IDC_NOINACTIVE_CHECK, OnNoinactiveCheck)
	ON_BN_CLICKED(IDC_NEW_TEMPLATE_BTN, OnNewTemplateBtn)
	ON_BN_CLICKED(IDC_EDIT_TEMPLATE_BTN, OnEditTemplateBtn)
	ON_BN_CLICKED(IDC_CUSTOM_INFO_CHECK, OnCustomInfoCheck)
	ON_BN_CLICKED(IDC_DATE_INFO_CHECK, OnDateInfoCheck)
	ON_BN_CLICKED(IDC_INSURANCE_INFO_CHECK, OnInsuranceInfoCheck)
	ON_BN_CLICKED(IDC_PERSON_INFO_CHECK, OnPersonInfoCheck)
	ON_BN_CLICKED(IDC_PRACTICE_INFO_CHECK, OnPracticeInfoCheck)
	ON_BN_CLICKED(IDC_PRESCRIPTION_INFO_CHECK, OnPrescriptionInfoCheck)
	ON_BN_CLICKED(IDC_EMAIL_CHECK, OnEmailCheck)
	ON_BN_CLICKED(IDC_EXPORTONLY_CHECK, OnExportOnlyCheck)
	ON_BN_CLICKED(IDC_EMR_CHECK, OnEMRCheck)
	ON_BN_CLICKED(IDC_WRITE_PACKET, OnWritePacket)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_NEXTECH_FAX, OnNextechFax)
	ON_BN_CLICKED(IDC_BILL_INFO_CHECK, OnBillInfoCheck)
	ON_BN_CLICKED(IDC_PROCEDURE_INFO_CHECK, OnProcedureInfoCheck)
	ON_BN_CLICKED(IDC_DOCTOR_INFO, OnDoctorInfo)
	ON_BN_CLICKED(IDC_RESP_INFO_CHECK, OnRespInfoCheck)
	ON_BN_CLICKED(IDC_EDIT_SUBJECT, OnEditSubject)
	ON_BN_CLICKED(IDC_MASS_MAIL, OnMassMail)
	ON_BN_CLICKED(IDC_HIDE_IDENTIFIER_FIELDS_CHECK, OnHidePHICheck)
	ON_BN_CLICKED(IDC_PHI_HELP_BTN, OnPHIHelpBtn)
	ON_BN_CLICKED(IDC_ADD_REMINDER_FOR_PT_MERGED, &CLetterWriting::OnBnClickedAddReminderForPtMerged)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLetterWriting, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLetterWriting)
	ON_EVENT(CLetterWriting, IDC_SUBJECT_COMBO, 16 /* SelChosen */, OnSelChosenSubjectCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterWriting message handlers
BOOL CLetterWriting::OnInitDialog() 
{
	try {
		// Call base class initialization
		CNxDialog::OnInitDialog();

		// (b.savon 2014-09-02 10:36) - PLID 62791 - Cache dontshow LetterWritingPatientReminder; also do the existing
		g_propManager.CachePropertiesInBulk("TelevoxExportDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'MergeShowDate' "
			"OR Name = 'MergeShowPerson' "
			"OR Name = 'MergeShowPractice' "
			"OR Name = 'MergeShowPrescription' "
			"OR Name = 'MergeShowCustom' "
			"OR Name = 'MergeShowInsurance' "
			"OR Name = 'MergeShowBillInfo' "
			"OR Name = 'MergeShowProcedureInfo' "
			"OR Name = 'MergeShowDoctorInfo' "
			"OR Name = 'MergeShowRespPartyInfo' "
			"OR Name = 'MergeShowEMRInfo' "
			"OR Name = 'NoMergeInactive' "
			"OR Name = 'WordPromptDefaultPath' "
			"OR Name = 'dontshow LetterWritingPatientReminder' "
			")",
			_Q(GetCurrentUserName())
			);

		m_newTemplateButton.AutoSet(NXB_NEW);
		m_editTemplateButton.AutoSet(NXB_MODIFY);

		// (e.lally 2009-06-04) PLID 29908 - Set as the help icon we've been using
		m_btnPHIHelp.SetIcon(IDI_BLUE_QUESTION);

		// Show the appropriate icon on each button
		m_letterButton.SetIcon(IDI_LETTER);
		m_envelopeButton.SetIcon(IDI_ENVELOPE);
		m_formButton.SetIcon(IDI_FORM);
		m_labelButton.SetIcon(IDI_LABEL);
		m_otherButton.SetIcon(IDI_OTHER);
		m_packetButton.SetIcon(IDI_PACKET);

		m_SubjectCombo = BindNxDataListCtrl(IDC_SUBJECT_COMBO);

		// (m.hancock 2006-12-04 13:04) - PLID 21965 - Added combo to select sorting options.
		m_SortOrderCombo = BindNxDataListCtrl(IDC_SORT_ORDER_LIST, false);

		// Set the default checkbox states
		CheckDlgButton(IDC_DATE_INFO_CHECK, GetRemotePropertyInt("MergeShowDate",1,0,"<None>",FALSE));
		CheckDlgButton(IDC_PERSON_INFO_CHECK, GetRemotePropertyInt("MergeShowPerson",1,0,"<None>",FALSE));
		CheckDlgButton(IDC_PRACTICE_INFO_CHECK, GetRemotePropertyInt("MergeShowPractice",1,0,"<None>",FALSE));
		CheckDlgButton(IDC_PRESCRIPTION_INFO_CHECK, GetRemotePropertyInt("MergeShowPrescription",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_CUSTOM_INFO_CHECK, GetRemotePropertyInt("MergeShowCustom",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_INSURANCE_INFO_CHECK, GetRemotePropertyInt("MergeShowInsurance",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_BILL_INFO_CHECK, GetRemotePropertyInt("MergeShowBillInfo",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_PROCEDURE_INFO_CHECK, GetRemotePropertyInt("MergeShowProcedureInfo",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_DOCTOR_INFO, GetRemotePropertyInt("MergeShowDoctorInfo",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_RESP_INFO_CHECK, GetRemotePropertyInt("MergeShowRespPartyInfo",0,0,"<None>",FALSE));
		CheckDlgButton(IDC_EMR_CHECK, GetRemotePropertyInt("MergeShowEMRInfo",0,0,"<None>",FALSE));

		CheckDlgButton(IDC_NOINACTIVE_CHECK, GetPropertyInt("NoMergeInactive", 0, 0, true) ? 1 : 0);
		
		CheckDlgButton(IDC_EMAIL_CHECK, 0);
		CheckDlgButton(IDC_EMAILHTML_CHECK, 0);
		OnEmailCheck();
		
		CheckDlgButton(IDC_EXPORTONLY_CHECK, 0);
		OnExportOnlyCheck();

		try {
			// Requery the signature combo
			m_dlEmployees = BindNxDataListCtrl(IDC_EMPLOYEE_COMBO, true);
			
			// Set the signature to that of the currently logged in user if possible
			m_dlEmployees->SetSelByColumn(0, GetCurrentUserID());
		} NxCatchAll("CLetterWriting::OnInitDialog");

		// Create subform
		m_groupEditor.Create(IDD_GROUPS, this);
		
		// Place it in the appropriate location
		CRect rect;
		GetDlgItem(IDC_GROUPEDITOR)->GetClientRect(&rect);
		m_groupEditor.MoveWindow(rect, FALSE);

		//DRT 11/4/02 - For internal merging to WinFax
		if(IsNexTechInternal()) {
			((CWnd*)GetDlgItem(IDC_NEXTECH_FAX))->ShowWindow(SW_SHOW);
			//initialize the phone book
			if (!m_pBookObj.CreateDispatch("WinFax.SDKPhoneBook")) {
				//this fails on any computers that don't have winfax... which annoys 90% of the office
				//AfxMessageBox("Was unable to create the Phonebook Object");

				//mark it inactive instead
				m_bWFXActive = false;
			}
			else {
				m_bWFXActive = true;
			}
		}

		//DRT 8/5/2004 - PLID 13006 - If they don't have lvl 2 emr licensing, they can't use the EMR checkbox
		// (a.walling 2007-11-28 13:03) - PLID 28044 - Check for expired EMR license
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(!(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2)) {
			//no license
			GetDlgItem(IDC_EMR_CHECK)->EnableWindow(FALSE);
			CheckDlgButton(IDC_EMR_CHECK, FALSE);
		}
		
		// (m.hancock 2006-12-04 10:13) - PLID 21965 - //Add each row to the sort order combo, depending on what is selected.
		if(m_groupEditor.GetSelectedTab() == 1) {
			//Appointment based sort options
			DisplayAppointmentBasedSortOptions();
		}
		else {
			//Default / Person based sort options
			DisplayDefaultSortOptions();
		}
		//Disable sorting option combo
		GetDlgItem(IDC_SORT_ORDER_LIST)->EnableWindow(FALSE);

	} NxCatchAll("Couldn't initialize the letter-writing screen")
	return TRUE;
}

void CLetterWriting::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);

	try {
		SetControlPositions();
		if (m_groupEditor)
		{	CRect rect;
			GetDlgItem(IDC_GROUPEDITOR)->GetClientRect(&rect);
			m_groupEditor.MoveWindow(rect);
			m_groupEditor.SetControlPositions();
		}
		// Since the SetControlPositions automatically calculated our background 
		// region, but it did so PRIOR to our changing the size of the m_groupEditor 
		// window, we have to recalculate it here AFTER the subform size change.
		// (a.walling 2009-02-04 10:14) - PLID 31956 - No longer used
		//SetRgnBg();
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OpenDocument(LPCTSTR strSubFolder /*= NULL*/)
{
	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}

	char path[MAX_PATH];
	path[0] = 0;
	OPENFILENAME ofn;
	CString strInitDir = GetTemplatePath(strSubFolder);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetSafeHwnd();

	// (a.walling 2007-06-14 13:22) - PLID 26342 - Should we support word 2007?
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	static char Filter2007[] = "Microsoft Word Templates (*.dot, *.dotx, *.dotm)\0*.DOT;*.DOTX;*.DOTM\0";
	// Always support Word 2007 templates
	ofn.lpstrFilter = Filter2007;
	
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = strInitDir.GetBuffer(MAX_PATH);
	strInitDir.ReleaseBuffer();
	ofn.lpstrTitle = "Select a merge template";
	ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "dot";



	try {	
		if (::GetOpenFileName(&ofn)) {	
			//CString path = dlg.GetPathName();
		
			//DRT 7/7/03 - Make sure the path we were given is valid
			if(!DoesExist(path)) {
				MsgBox("The document you have selected does not exist.  Please choose a valid template and try again.");
				return;
			}
			//JMM - 8/01/03 - check to see if they want to go directly to the printer
			if (IsDlgButtonChecked(IDC_MERGE_TO_PRINTER_CHECK)) {
				MergeDocument(path, TRUE); 
			}
			else {
				//set to false by default
				MergeDocument(path); 
			}

		}
	} NxCatchAll("Error in CLetterWriting::OpenDocument()");
}

void CLetterWriting::OnWriteForm() 
{
	try {
		OpenDocument ("Forms");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnWriteLabel() 
{
	try {
		OpenDocument("Labels");	
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnWriteLetter() 
{
	try {
		OpenDocument("Letters");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnWriteOther() 
{
	try {
		OpenDocument();
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnWriteEnvelope() 
{
	try {
		OpenDocument("Envelopes");
	}NxCatchAll(__FUNCTION__);
}

long CLetterWriting::CheckAllowClose()
{
	return m_groupEditor.CheckAllowClose();
}

void CLetterWriting::PreClose()
{
	m_groupEditor.PreClose();
}

void CLetterWriting::OnNoinactiveCheck() 
{
	try {
		if (IsDlgButtonChecked(IDC_NOINACTIVE_CHECK)) {
			SetPropertyInt("NoMergeInactive", -1);
		} else {
			SetPropertyInt("NoMergeInactive", 0);
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL BrowseForTemplate(HWND hwndDlgOwner, OUT CString &strFullPathToTemplate)
{
	// Let the user browse for the template starting in the official templates path	
	char path[MAX_PATH];
	path[0] = 0;
	OPENFILENAME ofn;
	CString strInitDir = GetTemplatePath();
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndDlgOwner;
	
	// (a.walling 2007-06-14 13:22) - PLID 26342 - Should we support word 2007?
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	static char Filter2007[] = "Microsoft Word Templates (*.dot, *.dotx, *.dotm)\0*.DOT;*.DOTX;*.DOTM\0";

	ofn.lpstrFilter = Filter2007;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = strInitDir.GetBuffer(MAX_PATH);
	strInitDir.ReleaseBuffer();
	ofn.lpstrTitle = "Select a prototype on which to base the new template";
	ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "dot";

	// Give the browse dialog
	if (::GetOpenFileName(&ofn)) {
		// We got a template path
		strFullPathToTemplate = path;
		return TRUE;
	} else {
		// The user canceled the browse
		return FALSE;
	}
}

//m.hancock - 2/20/2006 - PLID 17239 - Add capability to create word templates from EMR
//Moved to LetterWriting.h
/*
class CLetterWriting_ExtraMergeFields
{
public:
	CString m_strEMRItemFilter;
};
*/

static CString g_strEmrCategoryMergeFieldList;
static CString g_strEmrCategoryEmptyMergeDataList;

static CString g_strEmrItemMergeFieldList;
static CString g_strEmrItemEmptyMergeDataList;

static void Populate_CLetterWriting__ExtraMergeFields_Variables(CLetterWriting_ExtraMergeFields& emf)
{
	GetEmrCategoryMergeFieldList(g_strEmrCategoryMergeFieldList, g_strEmrCategoryEmptyMergeDataList);
	GetEmrItemMergeFieldList(emf.m_strEMRItemFilter, g_strEmrItemMergeFieldList, g_strEmrItemEmptyMergeDataList);
}

static CString CALLBACK CLetterWriting__ExtraMergeFields(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam)
{
	try {
		CLetterWriting_ExtraMergeFields *pemf = (CLetterWriting_ExtraMergeFields *)pParam;
		CString strEMRCategoryList, strEMRItemList;
		if (bFieldNamesInsteadOfData) {
			strEMRCategoryList = g_strEmrCategoryMergeFieldList;
			strEMRItemList = g_strEmrItemMergeFieldList;
		} else {
			strEMRCategoryList = g_strEmrCategoryEmptyMergeDataList;
			strEMRItemList = g_strEmrItemEmptyMergeDataList;
		}
		if (strEMRCategoryList.GetLength() && strEMRItemList.GetLength())
			return strEMRCategoryList + "," + strEMRItemList;
		else
			return strEMRCategoryList + strEMRItemList;

	} NxCatchAllCallIgnore({
		return "";
	});
}

void CLetterWriting::OnNewTemplateBtn() 
{
	try {
		//DRT 3/17/03 - Check for permission first!
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		// Get the path to the template on which to base this new template
		CString strBaseTemplate;
		{
			// First pop up a little menu that lets the user decide between browsing for a 
			// template or just creating a new blank one
			CMenu mnu;
			if (mnu.CreatePopupMenu()) {
				// Prep the popup menu
				mnu.InsertMenu(-1, MF_BYPOSITION, 1, "New &Blank Template");
				mnu.InsertMenu(-1, MF_BYPOSITION, 2, "&New Template Based on...");
				// Show the popup menu to the right of the "New Template" button, and wait for the user to click a menu item
				CRect rc;
				GetDlgItem(IDC_NEW_TEMPLATE_BTN)->GetWindowRect(&rc);
				int nResult = mnu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD, rc.right, rc.top, this);
				// See what the user wants to do
				if (nResult == 1) {
					// User just wants a new blank template
					strBaseTemplate = _T("");
				} else if (nResult == 2) {
					// User wants to browse
					if (!BrowseForTemplate(GetSafeHwnd(), strBaseTemplate)) {
						// The user canceled the browse
						return;
					}
				} else {
					// The user dismissed the menu without choosing anything, meaning they wanted to cancel (i.e. not make a new template)
					ASSERT(nResult == 0);
					return;
				}
			} else {
				// Failed to get the pop-up menu, default to the old way
				if (!BrowseForTemplate(GetSafeHwnd(), strBaseTemplate)) {
					// The user canceled the browse
					return;
				}
			}
		}

		// Set the user's templates path to the official nextech templates path
		//DRT 6/10/2008 - PLID 25501 - There is now a preference whether you should be warned about this.  If you don't want to
		//	be warned, it will not change the path.
		if(GetRemotePropertyInt("WordPromptDefaultPath", 1, 0, GetCurrentUserName(), true)) {
			try {
				// (z.manning 2016-02-11 16:24) - PLID 68230 - No longer Word-specific
				if (!GetWPManager()->PromptDefaultUserTemplatesPath(GetSubRegistryKey(), this))
				{
					// User cancelled the entire process
					return;
				}
			} NxCatchAll("CLetterWriting::OnNewTemplateBtn:SetUserTemplatesLocation");
		}
		
		// (c.haag 2004-05-12 16:00) - PLID 12293 - Allow the user to select EMR merge fields
		// if they are associated with procedures or diagnosis codes.
		// (b.cardillo 2004-06-22 11:02) - PLID 13137 - Build the whole EMR filter string (only 
		// if the emr checkbox is checked).
		CString strEMRFilter;
		if (IsDlgButtonChecked(IDC_EMR_CHECK)) {
			if (!GetEMRFilter(strEMRFilter)) {
				// The user canceled
				return;
			}
		}

		//m.hancock - 4-19-2006 - PLID 20203 - Removed a call to open a messagebox that should not have been here.

		CString strBlankMergeInfo;

		try { 

			CWaitCursor wc;
			// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown

			// Create an empty MergeInfo.nxt
			CLetterWriting_ExtraMergeFields emf;
			long nFlags = CreateMergeFlags(TRUE);
			if (IsDlgButtonChecked(IDC_EMR_CHECK)) {
				emf.m_strEMRItemFilter = strEMRFilter;
				Populate_CLetterWriting__ExtraMergeFields_Variables(emf);
				strBlankMergeInfo = CMergeEngine::CreateBlankMergeInfo(nFlags, CLetterWriting__ExtraMergeFields, &emf);
			}
			else {
				strBlankMergeInfo = CMergeEngine::CreateBlankMergeInfo(nFlags, NULL, NULL);
			}
			if (!strBlankMergeInfo.IsEmpty()) {

				// Open the template
				// (c.haag 2016-01-20) - PLID 68172 - Use the CWordDocument object to perform the business logic
				// (z.manning 2016-02-12 13:49) - PLID 68239 - Use the base document class
				// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
				// (c.haag 2016-04-22 10:51) - NX-100275 - The merge info assignment is now done in CreateTemplate
				// (d.lange 2016-05-23 11:58) - NX-100306 - Pass in the template path
				pApp->CreateTemplate(strBaseTemplate, strBlankMergeInfo, TRUE, nullptr, GetSharedPath() ^ "Templates");

				// We can't delete the merge info text file right now because it is in use, but 
				// it's a temp file so mark it to be deleted after the next reboot
				DeleteFileWhenPossible(strBlankMergeInfo);
				strBlankMergeInfo.Empty();
			} else {
				MsgBox(MB_OK|MB_ICONEXCLAMATION, "OnNewTemplate Error 50\n\nCould not create template");
			}
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		} NxCatchAll("OnNewTemplate Error 200");

		if (!strBlankMergeInfo.IsEmpty()) {
			// This means the file wasn't used and/or it wasn't 
			// marked for deletion at startup, so delete it now
			DeleteFile(strBlankMergeInfo);
		}
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnEditTemplateBtn() 
{
	try {
		//DRT 3/17/03 - Check for permission first!
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		// (c.haag 2004-05-12 16:00) - PLID 12293 - Allow the user to select EMR merge fields
		// if they are associated with procedures or diagnosis codes.
		CString strEMRFilter;
		if(IsDlgButtonChecked(IDC_EMR_CHECK)) {
			if (!GetEMRFilter(strEMRFilter)) {
				// The user canceled
				return;
			}
		}

		char path[MAX_PATH];
		path[0] = 0;
		CString strInitDir = GetTemplatePath();
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetSafeHwnd();
		
		// (a.walling 2007-06-14 13:22) - PLID 26342 - Should we support word 2007?
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		static char Filter2007[] = "Microsoft Word Templates (*.dot, *.dotx, *.dotm)\0*.DOT;*.DOTX;*.DOTM\0";

		ofn.lpstrFilter = Filter2007;
		ofn.lpstrCustomFilter = NULL;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = path;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = strInitDir.GetBuffer(MAX_PATH);
		strInitDir.ReleaseBuffer();
		ofn.lpstrTitle = "Select a template to edit";
		ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = "dot";

		if (!(::GetOpenFileName(&ofn))) {	
			DWORD dwErr = CommDlgExtendedError();
			return;
		}

		CString strMergeInfoFilePath;

		try {
			// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
			pApp->EnsureValid();

			// Create an empty MergeInfo.nxt
			CLetterWriting_ExtraMergeFields emf;
			long nFlags = CreateMergeFlags(TRUE);
			if (IsDlgButtonChecked(IDC_EMR_CHECK)) {
				emf.m_strEMRItemFilter = strEMRFilter;
				Populate_CLetterWriting__ExtraMergeFields_Variables(emf);
				strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, CLetterWriting__ExtraMergeFields, &emf);
			}
			else {
				strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, NULL, NULL);
			}
			if (!strMergeInfoFilePath.IsEmpty()) {

				// Open the template
				// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
				// (c.haag 2016-04-22 10:40) - NX-100275 - OpenTemplate no longer returns a document. We never did anything with it except throw an exception if it were null
				// anyway, and now OpenTemplate does that for us
				pApp->OpenTemplate(path, strMergeInfoFilePath);

				// We can't delete the merge info text file right now because it is in use, but 
				// it's a temp file so mark it to be deleted after the next reboot
				DeleteFileWhenPossible(strMergeInfoFilePath);
				strMergeInfoFilePath.Empty();
			} else {
				AfxThrowNxException("Could not create blank merge info");
			}
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		}NxCatchAll("CLetterWriting::OnEditTemplate");

		if (!strMergeInfoFilePath.IsEmpty()) {
			// This means the file wasn't used and/or it wasn't 
			// marked for deletion at startup, so delete it now
			DeleteFile(strMergeInfoFilePath);
		}	
	}NxCatchAll(__FUNCTION__);
}

long CLetterWriting::CreateMergeFlags(BOOL bShowAll) {

	long nFlags = (bShowAll ? BMS_HIDE_ALL_DATA : 0 ) | BMS_DEFAULT |
		(IsDlgButtonChecked(IDC_PRACTICE_INFO_CHECK) ? 0 : BMS_HIDE_PRACTICE_INFO) |
		(IsDlgButtonChecked(IDC_PERSON_INFO_CHECK) ? 0 : BMS_HIDE_PERSON_INFO) |
		(IsDlgButtonChecked(IDC_DATE_INFO_CHECK) ? 0 : BMS_HIDE_DATE_INFO) |
		(IsDlgButtonChecked(IDC_PRESCRIPTION_INFO_CHECK) ? 0 : BMS_HIDE_PRESCRIPTION_INFO) |
		(IsDlgButtonChecked(IDC_CUSTOM_INFO_CHECK) ? 0 : BMS_HIDE_CUSTOM_INFO) |
		(IsDlgButtonChecked(IDC_INSURANCE_INFO_CHECK) ? 0 : BMS_HIDE_INSURANCE_INFO) |
		(IsDlgButtonChecked(IDC_BILL_INFO_CHECK) ? 0 : BMS_HIDE_BILL_INFO) |
		(IsDlgButtonChecked(IDC_PROCEDURE_INFO_CHECK) ? 0 : BMS_HIDE_PROCEDURE_INFO) |
		(IsDlgButtonChecked(IDC_DOCTOR_INFO) ? 0 : BMS_HIDE_DOCTOR_INFO) |
		(IsDlgButtonChecked(IDC_RESP_INFO_CHECK) ? 0 : BMS_HIDE_RESP_PARTY_INFO) |
		(IsDlgButtonChecked(IDC_EMR_CHECK) ? 0 : BMS_HIDE_EMR_INFO) |
		// (e.lally 2009-06-04) PLID 29908
		(IsDlgButtonChecked(IDC_HIDE_IDENTIFIER_FIELDS_CHECK) ? BMS_HIDE_IDENTIFIER_INFO : 0) |
		(m_groupEditor.m_bAppointmentMerge ? BMS_APPOINTMENT_BASED : 0);

	return nFlags;
}

void CLetterWriting::OnCustomInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowCustom", (IsDlgButtonChecked(IDC_CUSTOM_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnDateInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowDate",(IsDlgButtonChecked(IDC_DATE_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnInsuranceInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowInsurance",(IsDlgButtonChecked(IDC_INSURANCE_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnPersonInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowPerson",(IsDlgButtonChecked(IDC_PERSON_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnPracticeInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowPractice",(IsDlgButtonChecked(IDC_PRACTICE_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnEMRCheck()
{
	try {
		SetRemotePropertyInt("MergeShowEMRInfo",(IsDlgButtonChecked(IDC_EMR_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnPrescriptionInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowPrescription",(IsDlgButtonChecked(IDC_PRESCRIPTION_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnBillInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowBillInfo",(IsDlgButtonChecked(IDC_BILL_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnProcedureInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowProcedureInfo",(IsDlgButtonChecked(IDC_PROCEDURE_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnDoctorInfo() 
{
	try {
		SetRemotePropertyInt("MergeShowDoctorInfo",(IsDlgButtonChecked(IDC_DOCTOR_INFO) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnEmailCheck() 
{
	try {
		if (IsDlgButtonChecked(IDC_EMAIL_CHECK)) {
			// Enable the html checkbox
			GetDlgItem(IDC_EMAILHTML_CHECK)->EnableWindow(TRUE);
			InvalidateDlgItem(IDC_EMAILHTML_CHECK, FALSE);

			// Uncheck the export-only checkbox
			CheckDlgButton(IDC_EXPORTONLY_CHECK, 0);
			OnExportOnlyCheck();
			// Disable the export-only checkbox
			GetDlgItem(IDC_EXPORTONLY_CHECK)->EnableWindow(FALSE);
			InvalidateDlgItem(IDC_EXPORTONLY_CHECK, FALSE);

			//enable the 'exclude email' checkbox
			GetDlgItem(IDC_DONT_EXCLUDE_PRIV_EMAIL)->EnableWindow(TRUE);
			InvalidateDlgItem(IDC_DONT_EXCLUDE_PRIV_EMAIL, FALSE);

		} else {
			// Uncheck the html checkbox
			CheckDlgButton(IDC_EMAILHTML_CHECK, 0);
			// Disable the html checkbox
			GetDlgItem(IDC_EMAILHTML_CHECK)->EnableWindow(FALSE);
			InvalidateDlgItem(IDC_EMAILHTML_CHECK, FALSE);

			// Enable the export-only checkbox
			GetDlgItem(IDC_EXPORTONLY_CHECK)->EnableWindow(TRUE);
			InvalidateDlgItem(IDC_EXPORTONLY_CHECK, FALSE);

			//disable the 'exclude email' checkbox
			GetDlgItem(IDC_DONT_EXCLUDE_PRIV_EMAIL)->EnableWindow(FALSE);
			InvalidateDlgItem(IDC_DONT_EXCLUDE_PRIV_EMAIL, FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnExportOnlyCheck() 
{
	try {
		if (IsDlgButtonChecked(IDC_EXPORTONLY_CHECK)) {
			// Uncheck the email checkbox
			CheckDlgButton(IDC_EMAIL_CHECK, 0);
			OnEmailCheck();
			// Disable the email checkbox
			GetDlgItem(IDC_EMAIL_CHECK)->EnableWindow(FALSE);
			InvalidateDlgItem(IDC_EMAIL_CHECK, FALSE);
		} else {
			// Enable the email checkbox
			GetDlgItem(IDC_EMAIL_CHECK)->EnableWindow(TRUE);
			InvalidateDlgItem(IDC_EMAIL_CHECK, FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnWritePacket() 
{
	try {
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		CSelectPacketDlg dlg(this);
		int nReturn = dlg.DoModal();
		if(nReturn == IDOK) {
			
			CStringArray arystrPaths;

			// Loop through each template, adding the full path of each one to the array
			_RecordsetPtr rsTemplates = CreateRecordset("SELECT Path FROM MergeTemplatesT INNER JOIN PacketComponentsT ON MergeTemplatesT.ID = PacketComponentsT.MergeTemplateID WHERE PacketComponentsT.PacketID = %li ORDER BY ComponentOrder %s", dlg.m_nPacketID, dlg.m_bReverseMerge ? "DESC" : "");
			//Loop through, create an array of the names, merge them all at the end.
			while(!rsTemplates->eof) {
					
				CString strPath = AdoFldString(rsTemplates, "Path");
					
				//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
				if(strPath.GetAt(0) == '\\' && strPath.GetAt(1) != '\\') {
					strPath = GetSharedPath() ^ strPath;
				}

				if(!DoesExist(strPath)) {
					MsgBox("The template '%s' could not be found.  This template will be skipped.", strPath);
				}
				else {
					// Add the full path to the array
					arystrPaths.Add(strPath);
				}
					
				// Move to the next template in the packet
				rsTemplates->MoveNext();
			}

			// Do the merge of the whole batch
			//TES 2/23/2004: Don't forget to check whether they're merging directly to printer!
			MergeDocumentBatch(arystrPaths, IsDlgButtonChecked(IDC_MERGE_TO_PRINTER_CHECK) ? true : false, dlg.m_nPacketID, dlg.m_bSeparateDocuments?true:false);
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CLetterWriting::PrepareMerge(OUT CMergeEngine &mi, OUT CString &strMergeTo, OUT BOOL &bAllowSave)
{
	BOOL bEmailMerge = IsDlgButtonChecked(IDC_EMAIL_CHECK);
	BOOL bNoInactive = IsDlgButtonChecked(IDC_NOINACTIVE_CHECK);
	BOOL bFormatPlainText = IsDlgButtonChecked(IDC_EMAILHTML_CHECK) ? FALSE : TRUE;
	//notice the bSkipExcluded is the opposite of the check box because if the check box is checked it means that they
	//they don't want to skip the excluded patients, but if it't not checked then they want to skip these patients
	BOOL bSkipExcluded = !IsDlgButtonChecked(IDC_EXCLUDE_PATIENTS);
	//DRT 7/30/03 - skip priv email works the same as skipexcluded
	// (m.cable 7/13/2004 13:42) - PLID 13395 - Added a check to see if the email button was checked, if it's not, then we shouldn't
	// care if the other button is checked
	BOOL bSkipPrivEmail = (IsDlgButtonChecked(IDC_EMAIL_CHECK) && !IsDlgButtonChecked(IDC_DONT_EXCLUDE_PRIV_EMAIL));

	// Check to see if there are any records to merge
	long rowCount;
	if (m_groupEditor.m_bAppointmentMerge)
		rowCount = m_groupEditor.m_apptSelected->GetRowCount();
	else
		rowCount = m_groupEditor.m_selected->GetRowCount();
	long nCount = rowCount;
	if (nCount > 0) {
		CString strTempIDTableT;

		if(bNoInactive || bEmailMerge || bSkipExcluded || bSkipPrivEmail)
		{
			CString strWhere;
			//build the where clause for the patients who are not going to be included in the merge
			//check to see if they want to include patients who have the checkbox on general 1 for hiding them from mailings
			if(bSkipExcluded) {
				strWhere += "ExcludeFromMailings = 1 OR ";
			}

			//check to see if they want to include email patients anyways
			if(bSkipPrivEmail) {
				strWhere += "PrivEmail = 1 OR ";
			}

			// check and see if the Inactive patients should be included
			if(bNoInactive)	{
				strWhere += "Archived = 1 OR ";
			}
			
			//next check the email addresses
			if(bEmailMerge) {
				strWhere += "(Len(Email) = 0 OR Email = ' ') OR ";
			}
			
			strWhere.TrimRight("OR ");

			if (m_groupEditor.m_bAppointmentMerge) {
				strTempIDTableT = CreateTempIDTable(m_groupEditor.m_apptSelected, 0, FALSE, TRUE, &nCount, TRUE);
				CStringArray aryFieldNames, aryFieldTypes;
				aryFieldNames.Add("ID");
				aryFieldTypes.Add("INT");
				aryFieldNames.Add("PatientID");
				aryFieldTypes.Add("INT");
				strTempIDTableT = CreateTempIDTable("SELECT DISTINCT ID, PatientID FROM " + strTempIDTableT + 
									" WHERE PatientID NOT IN (SELECT PersonT.ID FROM PersonT WHERE " +
									strWhere + ")", aryFieldNames, aryFieldTypes, TRUE, TRUE, &nCount);
			}
			else {
				strTempIDTableT = CreateTempIDTable(m_groupEditor.m_selected, 0, FALSE);
				strTempIDTableT = CreateTempIDTable("SELECT DISTINCT ID FROM " + strTempIDTableT + 
									" WHERE ID NOT IN (SELECT PersonT.ID FROM PersonT WHERE " +
									strWhere + ")", "ID", TRUE, TRUE, &nCount);
			}
		}
		else {
			if (m_groupEditor.m_bAppointmentMerge)
				//normal merge of all selected appointments
				strTempIDTableT = CreateTempIDTable(m_groupEditor.m_apptSelected, 0, TRUE, TRUE, &nCount, TRUE);
			else
				//normal merge of all selected patients
				strTempIDTableT = CreateTempIDTable(m_groupEditor.m_selected, 0, TRUE, TRUE, &nCount);
		}

		// Make the query into a sub-query and name it MergeGroup (TODO: do we still need to do this?)
		strMergeTo.Format("(SELECT ID, %sRowNumber FROM %s) MergeGroup", m_groupEditor.m_bAppointmentMerge ? "PatientID, " : "", strTempIDTableT);
	}

	BOOL bEmptyDocumentMerge = FALSE;

	if (nCount == 0) {

		CString strNoItemPrompt = "There are no items to merge.  Would you like to create an empty document?";

		//If the list contains items, then we can confuse the user with our message.
		//So let's give a correct account of why there are no items.
		if(rowCount > 0) {
			if(bNoInactive && bEmailMerge && bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (all selected patients are either inactive or have no email address or have been marked to be excluded).\n"
					"Would you like to create an empty document?";
			else if (bNoInactive && !bEmailMerge && bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (all selected patients are inactive or have been marked to be excluded).\n"
					"Would you like to create an empty document?";
			else if (bNoInactive && !bEmailMerge && !bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (all selected patients are inactive).\n"
					"Would you like to create an empty document?";
			else if (!bNoInactive && bEmailMerge && bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (all selected patients have been marked to be excluded or have no email address).\n"
					"Would you like to create an empty document?";
			else if (!bNoInactive && bEmailMerge && !bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (no selected patients have an email address).\n"
					"Would you like to create an empty document?";
			else if (bNoInactive && bEmailMerge && !bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (all selected patients are either inactive or have no email address).\n"
					"Would you like to create an empty document?";
			else if (!bNoInactive && !bEmailMerge && bSkipExcluded)
				strNoItemPrompt = "There are no items to merge (all selected patients have been marked to be excluded).\n"
					"Would you like to create an empty document?";
		}

		// if there are really no records, prompt the user
		if (MsgBox(MB_YESNO|MB_ICONQUESTION, strNoItemPrompt) == IDYES) {
			// The user wants to continue, so merge to just the -25 patient
			// (c.haag 2004-10-11 17:05) - PLID 14293 - We need the parenthesis and subquery
			// name for the later code to properly use strMergeTo
			strMergeTo = "(SELECT ID, 1 AS RowNumber FROM PersonT WHERE ID = -25) MergeGroup";
			bAllowSave = FALSE;
			bEmptyDocumentMerge = TRUE;
		} else {
			// Stop now because the user said to
			return FALSE;
		}
	}

	// Dyanamically modify the fields bound to the letter-writing dialog and 
	// also set the appropriate sorting based on the on-screen checkboxes
	GetDlgItemText(IDC_MERGE, mi.m_strSubjectMatter);
	long nCurSel = m_dlEmployees->CurSel;
	if (nCurSel != -1) {
		CString strFirst = VarString(m_dlEmployees->Value[nCurSel][3], ""); strFirst.TrimLeft(); strFirst.TrimRight();
		CString strMiddle = VarString(m_dlEmployees->Value[nCurSel][4], ""); strMiddle.TrimLeft(); strMiddle.TrimRight();
		CString strLast = VarString(m_dlEmployees->Value[nCurSel][2], ""); strLast.TrimLeft(); strLast.TrimRight();
		CString strTitle = VarString(m_dlEmployees->Value[nCurSel][5], ""); strTitle.TrimLeft(); strTitle.TrimRight();
		CString strEmail = VarString(m_dlEmployees->Value[nCurSel][6], ""); strTitle.TrimLeft(); strTitle.TrimRight();
		mi.m_strSender = strFirst + (strMiddle.IsEmpty()?"":(" "+ strMiddle)) + " " + strLast;
		mi.m_strSenderFirst = strFirst;
		mi.m_strSenderMiddle = strMiddle;
		mi.m_strSenderLast = strLast;
		mi.m_strSenderTitle = strTitle;
		mi.m_strSenderEmail = strEmail;
	}
	mi.m_nFlags = CreateMergeFlags();
	// If we are merging to a blank document we don't want to merge any information. That means if this is an
	// appoitment based merge we don't want to merge any appointment information, so unset the appointment
	// based merge flag.
	if (bEmptyDocumentMerge)
		mi.m_nFlags = mi.m_nFlags & ~BMS_APPOINTMENT_BASED;
	if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

	// (m.hancock 2006-09-22 17:46) - PLID 21965 - Determine the OrderBy clause.
	if(IsDlgButtonChecked(IDC_MASS_MAIL)) {

		//First, check to see if there is a valid sort option selected.
		if(m_SortOrderCombo->GetCurSel() == -1) {
			MsgBox("This merge should be sorted, but a valid sorting option has not been selected.  The merge cannot continue.");
			return FALSE;
		}

		// (m.hancock 2006-12-04 13:24) - PLID 21965 - Determine the sorting option that is currently selected.
		long nSortOption = m_SortOrderCombo->GetValue(m_SortOrderCombo->GetCurSel(), socID);
		switch(nSortOption) {
		case 1: //Zipcode
			mi.m_strOrderBy = "PersonT.Zip, PersonT.City, PersonT.[Last]";
			break;
		case 2: //Appointment date ascending (appt. based)
			//mi.m_strOrderBy = "SubQuery.StartTime ASC, PersonT.[Last], PersonT.[First]";
			// (a.wetta 2007-02-27 13:57) - PLID 24174 - Also make sure that we're sorting the start times within each date
			mi.m_strOrderBy = "SubQuery.Date ASC, SubQuery.StartTime ASC";
			break;
		case 3: //Appointment date descending (appt. based)
			//mi.m_strOrderBy = "SubQuery.StartTime DESC, PersonT.[Last], PersonT.[First]";
			// (a.wetta 2007-02-27 13:57) - PLID 24174 - Also make sure that we're sorting the start times within each date
			mi.m_strOrderBy = "SubQuery.Date DESC, SubQuery.StartTime DESC";
			break;
		case 0:
		default:
			//If we got here, then use the default sort.
			mi.m_strOrderBy = "PersonT.[Last], PersonT.[First]";
			break;
		}
	}

	if (bEmailMerge) {
		if (MsgBox(MB_ICONINFORMATION|MB_YESNO,
			"You have chosen to export this merge, with %li entries, to e-mail.\n\n"
			"If you continue, your default e-mail software will be used by Microsoft "
			"Word to automatically send the results as soon as the merge is complete.  "
			"(NOTE: Any patient or contact with a blank e-mail address will be skipped.)\n\n"
			"Would you like to proceed?", nCount) != IDYES) {
			return FALSE;
		}

		// (j.jones 2011-06-24 13:32) - PLID 33496 - warn if the subject is empty
		if(mi.m_strSubjectMatter.IsEmpty() && MsgBox(MB_ICONINFORMATION|MB_YESNO,
			"You have not entered a Subject for this e-mail merge.\n\nAre you sure you wish to continue?") != IDYES) {
			return FALSE;
		}

		if (bFormatPlainText) {
			mi.m_nFlags = (mi.m_nFlags|BMS_MERGETO_EMAIL_PLAIN|BMS_SAVE_FILE_AND_HISTORY) & ~BMS_MERGETO_SCREEN;
		} else {
			mi.m_nFlags = (mi.m_nFlags|BMS_MERGETO_EMAIL_HTML|BMS_SAVE_FILE_AND_HISTORY) & ~BMS_MERGETO_SCREEN;
		}
		bAllowSave = FALSE;
	}

	if (IsDlgButtonChecked(IDC_EXPORTONLY_CHECK)) {
		mi.m_nFlags |= BMS_EXPORT_ONLY;
	}

	if (m_groupEditor.m_bAppointmentMerge) {
		// By default the m_strResFilter filters out appointments in the past.  We do not want
		// to do that in appt based cases, because we have already determined which appointments should
		// be printed, and this would further filter that list remove things we really should print.
		mi.m_strResFilter = "";
	}

	return TRUE;
}

void CLetterWriting::MergeDocument(const CString &strPath, bool bDirectToPrinter /*= false*/)
{
	// Create an array of one element
	CStringArray arystrPaths;
	arystrPaths.Add(strPath);
	
	// Call the batch version on that one element
	MergeDocumentBatch(arystrPaths, bDirectToPrinter);
}

void CLetterWriting::MergeDocumentBatch(const CStringArray &arystrPaths, bool bDirectToPrinter /*= false*/, long nPacketID /*= -1*/, bool bSeparatePacket /*= false*/)
{
	try {
		// (c.haag 2016-05-31 15:24) - NX-100310 - If we're using virtual channels and merging more than a hundred patients, warn the user
		if (WordProcessorType::VTSCMSWord == GetWordProcessorType() && m_groupEditor.m_selected->GetRowCount() > 100)
		{
			if (IDNO == AfxMessageBox(R"(You are merging documents for over 100 patients. Practice will be inaccessible for long periods of time while performing merges on large numbers of patients, and any network disconnects will cause the merge to fail.

Are you sure you wish to continue this merge?)", MB_YESNO | MB_ICONWARNING))
			{
				return;
			}
		}

		CWaitCursor wc;

		// (a.walling) 5/1/06 PLID 18343 
		long nPacketCategoryID = CMergeEngine::GetPacketCategory(nPacketID);

		// Call the prepare merge only once
		CString strMergeTo;
		CMergeEngine mi;
		long nMergedPacketID = -1;
		BOOL bAllowSave = TRUE;
		if (PrepareMerge(mi, strMergeTo, bAllowSave)) {
			// Decide whether we should save to patient history
			bool bCancel = false;
			if (bAllowSave && !IsDlgButtonChecked(IDC_DO_NOT_ATTACH) && ShouldAttachBatchToPatientHistory(arystrPaths, strMergeTo, bCancel)) {
				if(nPacketID != -1) {
					nMergedPacketID = NewNumber("MergedPacketsT", "ID");
					ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID, SeparateDocuments) VALUES (%li, %li, %i)", nMergedPacketID, nPacketID, bSeparatePacket?1:0);


				}
				mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;


			}

			if(bDirectToPrinter) {
				mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
			}

			if (!bCancel) {
				// Do the merge 
				CString strCurrentTemplateFilePath;
				
				// Loop through each element in the list of paths and run the merge for each
				long nCount = arystrPaths.GetSize();
				for (long i=0; i<nCount; i++) {
					CString strProgress;
					if(nPacketID != -1 && nCount > 1) {
						strProgress.Format("Template %i of %i",i+1,nCount);
					}
					// Merge for this element
					strCurrentTemplateFilePath = arystrPaths.GetAt(i);
					// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
					// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
					BOOL bMergeSuccess;
					if (mi.m_nFlags & BMS_APPOINTMENT_BASED) {
						// The appointment based merge flag is set
						bMergeSuccess = mi.MergeToWord(strCurrentTemplateFilePath, std::vector<CString>(), strMergeTo, "PatientID", nMergedPacketID, nPacketCategoryID, bSeparatePacket, strProgress);
					}
					else {
						bMergeSuccess = mi.MergeToWord(strCurrentTemplateFilePath, std::vector<CString>(), strMergeTo, "", nMergedPacketID, nPacketCategoryID, bSeparatePacket, strProgress);
					}

					if (!bMergeSuccess) {
						return;
					}
				}

				// (c.haag 2004-10-11 17:05) - PLID 14293 - Make sure we don't apply tracking
				// events to the sentinel patient
				if(nPacketID != -1) {
					//We need to track this packet, for each patient.
					_RecordsetPtr rsPats = CreateRecordset("SELECT ID FROM %s WHERE ID <> -25", strMergeTo);
					CDWordArray dwaPatIDs;
					while(!rsPats->eof) {
						dwaPatIDs.Add(AdoFldLong(rsPats, "ID"));
						rsPats->MoveNext();
					}
					rsPats->Close();
					for(int i = 0; i < dwaPatIDs.GetSize(); i++) {
						PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PacketSent, (long)dwaPatIDs.GetAt(i), COleDateTime::GetCurrentTime(), nMergedPacketID);
					}
				}

				// (r.farnworth 2014-09-12 10:35) - PLID 62792 - When user merge documnet on Letterwriting or Recall, Based on "Sent Reminder" option, insert record to PatientRemindersSetT
				if (m_btnAddPatientReminder.GetCheck() == BST_CHECKED){
					long nPatID;
					CString sqlToExecute;

					_RecordsetPtr rsPatsRemind = CreateRecordset("SELECT DISTINCT ID FROM %s WHERE ID <> -25", strMergeTo);
					while (!rsPatsRemind->eof) {
						nPatID = AdoFldLong(rsPatsRemind, "ID");

						CString currentSQL;
						// (s.tullis 2015-10-01 09:23) - PLID 66442 - Added Default Delete Flag
						currentSQL.Format(" INSERT INTO PatientRemindersSentT (PatientID, UserID, ReminderMethod, ReminderDate) "
							" VALUES (%li, %li, %li, GETDATE());\r\n", nPatID, GetCurrentUserID(), srhLetterwriting);
						sqlToExecute += currentSQL;
						rsPatsRemind->MoveNext();
					}

					ExecuteSql(sqlToExecute);
				}

			}
		}
	}NxCatchAll("Error in CLetterWriting::MergeDocumentBatch()");
}

HBRUSH CLetterWriting::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-05-22 11:47) - PLID 27648 - This needs to call the CNxDialog base class rather than CDialog.
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	try {
		switch (nCtlColor)
		{	case CTLCOLOR_STATIC:
			//make anything (STATIC text) with WS_EX_TRANSPARENT
			//appear transparent without subclassing it
				if (pWnd->GetExStyle() & WS_EX_TRANSPARENT)
				{	pDC->SetBkMode (TRANSPARENT);
					return (HBRUSH)GetStockObject(NULL_BRUSH);
				}
				break;
			case CTLCOLOR_BTN:
			case CTLCOLOR_DLG:
			case CTLCOLOR_MSGBOX:
			case CTLCOLOR_EDIT:
			default:
				break;
		}
	}NxCatchAll(__FUNCTION__);
	return hbr;
}

/*
BOOL CLetterWriting::OnEraseBkgnd(CDC* pDC) 
{
	CBrush br;
	
	br.Attach(GetSysColorBrush(COLOR_BTNFACE));
	pDC->FillRgn(&m_rgnBg, &br);
	br.Detach();

	return FALSE;
}

*/

//DRT 11/4/02
//The purpose of the faxing is not to do the actual faxing ourselves (at the moment).  For now all we want to do is create a folder in 
//WinFax and put everyone who exists into that folder, so we can easily merge a document and drag in all the names in a matter of seconds
void CLetterWriting::OnNextechFax() 
{
	//so, you want to fax do you?

	try {

		//quit if winfax did not initialize correctly
		if(!m_bWFXActive) {
			AfxMessageBox("WinFax failed to initialize, ensure it is correctly installed before attempting to create Winfax groups.");
			return;
		}

		//firstly, we need to ask them what they'd like to call this folder
		CString strFolder;

		int nResult;
		do {
			nResult = InputBox(this, "Enter a new folder name:", strFolder, "");
			strFolder.TrimRight();

			if(nResult == IDCANCEL)
				return;
		
		} while (strFolder.IsEmpty());

		//now create it in WinFax
		CString sNexID = GetNexID();	//get the parent for our new folder
		CString strFolderID = m_pBookObj.AddFolder(STANDARDFOLDER_NONE, sNexID, strFolder, "");
		if(strFolderID.IsEmpty()) {
			AfxMessageBox("Failed to add new folder.  Make sure name does not already exist.");
			return;
		}

		//strFolderID is the ID of our folder, for use in adding all the users
		AddUsersToWinFaxFolder(strFolderID);

		//done adding users, we're good to go
		AfxMessageBox("Successfully created group '" + strFolder + "'.  This folder is located under the header 'NexTech Faxing' in WinFax.");

	} NxCatchAll("Error exporting users for WinFax.");

}

CString CLetterWriting::GetNexID()
{
	//searches through all folders and finds one labeled "My Phonebook"
	//then finds a 'NexTech Faxing' underneath it
	//if such a folder does not exist, it creates it, and returns the ID
	CString sRoot = m_pBookObj.GetFolderListFirst(STANDARDFOLDER_WINFAX_ROOT, "");
	do {
		if(m_pBookObj.GetFolderDisplayName(sRoot) == "My Phonebook")
			break;
		else
			sRoot = m_pBookObj.GetFolderListNext();
	} while (sRoot != "");

	//sRoot is now my phonebook or empty
	if(sRoot.IsEmpty()) {
		AfxMessageBox("Failed to find 'My Phonebook'");
		return "";
	}

	//now find "NexTech Faxing"
	CString s = m_pBookObj.GetFolderListFirst(STANDARDFOLDER_NONE, sRoot);
	do {
		if(m_pBookObj.GetFolderDisplayName(s) == "NexTech Faxing")
			break;
		else
			s = m_pBookObj.GetFolderListNext();
	} while (s != "");

	//s is either blank or our folder
	if(s.IsEmpty()) {
		//add the folder
		s = m_pBookObj.AddFolder(STANDARDFOLDER_NONE, sRoot, "NexTech Faxing", "");
	}

	return s;
}

void CLetterWriting::AddUsersToWinFaxFolder(CString strFolderID) {

	//make a temp pointer to the group datalist for use here
	_DNxDataListPtr pGroup = m_groupEditor.m_selected;
	
	//loop through all the users in the datalist and add them into WinFax
	//we also need to lookup their fax numbers to pull those in as well
	for(int i = 0; i < pGroup->GetRowCount(); i++) {
		long nID;
		CString first="", last="", company="", fax="", city="", state="", zip="";

		//get the first, last, company, and ID from the datalist
		nID = VarLong(pGroup->GetValue(i, 0));
		first = VarString(pGroup->GetValue(i, 3), "");
		last = VarString(pGroup->GetValue(i, 2), "");
		company = VarString(pGroup->GetValue(i, 4), "");

		//now for the slow part, lookup the fax number of each person
		_RecordsetPtr rs = CreateRecordset("SELECT Fax, City, State, Zip FROM PersonT WHERE ID = %li", nID);
		if(!rs->eof) {
				fax = AdoFldString(rs, "Fax", "");
				city = AdoFldString(rs, "City", "");
				state = AdoFldString(rs, "State", "");
				zip = AdoFldString(rs, "Zip", "");
		}
		rs->Close();
		//end slowness

		//and add them to our strFolderID in WinFax
		CString strUserID = m_pBookObj.AddUser(STANDARDFOLDER_NONE, strFolderID, first, last, company);
		if(strUserID.IsEmpty())
			AfxMessageBox("Failed to add user '" + first + " " + last + "'.");
		else {
			//set their fax number
			m_pBookObj.SetUserFaxLocal(strUserID, fax);
			//set city
			m_pBookObj.SetUserCity(strUserID, city);
			//set state
			m_pBookObj.SetUserState(strUserID, state);
			//set zip
			m_pBookObj.SetUserPostalCode(strUserID, zip);
		}
	}
}

void CLetterWriting::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{

	if(m_coordChecker.Changed()) {

		long PersonID = -1;
		
		if(m_dlEmployees->GetCurSel() != -1)
			PersonID = VarLong(m_dlEmployees->GetValue(m_dlEmployees->GetCurSel(),0),-1);

		// Requery the signature combo
		m_dlEmployees = BindNxDataListCtrl(IDC_EMPLOYEE_COMBO, true);
			
		// Set the signature to that of the currently logged in user if possible
		if(PersonID == -1)
			PersonID = GetCurrentUserID();
		m_dlEmployees->SetSelByColumn(0, PersonID);
	}
}

void CLetterWriting::OnRespInfoCheck() 
{
	try {
		SetRemotePropertyInt("MergeShowRespPartyInfo",(IsDlgButtonChecked(IDC_RESP_INFO_CHECK) ? 1 : 0),0,"<None>");
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnSelChosenSubjectCombo(long nRow) 
{
	try {
		if(nRow==-1)
			return;

		CString str;
		str = VarString(m_SubjectCombo->GetValue(nRow,0),"");
		GetDlgItem(IDC_MERGE)->SetWindowText(str);
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnEditSubject() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 10, m_SubjectCombo, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnMassMail() 
{
	try {
	// (m.hancock 2006-09-25 10:20) - PLID 21965 - When the sort checkbox is checked or unchecked,
	// we need to enable or disable the sorting combo.
		GetDlgItem(IDC_SORT_ORDER_LIST)->EnableWindow(IsDlgButtonChecked(IDC_MASS_MAIL) ? TRUE : FALSE);
	}NxCatchAll(__FUNCTION__);
}

// (m.hancock 2006-12-04 10:52) - PLID 21965 - Changes the sort option datalist to show options applicable 
// to the specific merge type.
void CLetterWriting::DisplayDefaultSortOptions()
{
	try {
		// (m.hancock 2006-12-04 10:15) - PLID 21965 - Default / Person based sort options.
		
		//Clear the list
		m_SortOrderCombo->Clear();

		//Last Name (Default)
		IRowSettingsPtr pSortOrderRow = m_SortOrderCombo->GetRow(-1);
		pSortOrderRow->PutValue(socID, (long)0);
		pSortOrderRow->PutValue(socDescription, _bstr_t("Last Name"));
		m_SortOrderCombo->AddRow(pSortOrderRow);
		//Select Last Name by default
		m_SortOrderCombo->PutCurSel(0);
		
		//Zipcode
		pSortOrderRow = m_SortOrderCombo->GetRow(-1);
		pSortOrderRow->PutValue(socID, (long)1);
		pSortOrderRow->PutValue(socDescription, _bstr_t("Zipcode"));
		m_SortOrderCombo->AddRow(pSortOrderRow);


	} NxCatchAll("Error in CLetterWriting::DisplayDefaultSortOptions()");
}
// (m.hancock 2006-12-04 10:52) - PLID 21965 - Changes the sort option datalist to show options applicable 
// to the specific merge type.
void CLetterWriting::DisplayAppointmentBasedSortOptions()
{
	try {
		// (m.hancock 2006-12-04 10:15) - PLID 21965 - Appointment based sort options.

		//Clear the list
		m_SortOrderCombo->Clear();

		//Last Name (Default)
		IRowSettingsPtr pSortOrderRow = m_SortOrderCombo->GetRow(-1);
		pSortOrderRow->PutValue(socID, (long)0);
		pSortOrderRow->PutValue(socDescription, _bstr_t("Last Name"));
		m_SortOrderCombo->AddRow(pSortOrderRow);
		//Select Last Name by default
		m_SortOrderCombo->PutCurSel(0);

		//Appointment date ascending
		pSortOrderRow = m_SortOrderCombo->GetRow(-1);
		pSortOrderRow->PutValue(socID, (long)2);
		pSortOrderRow->PutValue(socDescription, _bstr_t("Appointment date (Ascending)"));
		m_SortOrderCombo->AddRow(pSortOrderRow);

		//Appointment date descending
		pSortOrderRow = m_SortOrderCombo->GetRow(-1);
		pSortOrderRow->PutValue(socID, (long)3);
		pSortOrderRow->PutValue(socDescription, _bstr_t("Appointment date (Descending)"));
		m_SortOrderCombo->AddRow(pSortOrderRow);

		//Zipcode
		pSortOrderRow = m_SortOrderCombo->GetRow(-1);
		pSortOrderRow->PutValue(socID, (long)1);
		pSortOrderRow->PutValue(socDescription, _bstr_t("Zipcode"));
		m_SortOrderCombo->AddRow(pSortOrderRow);

	} NxCatchAll("Error in CLetterWriting::DisplayAppointmentBasedSortOptions()");
}
void CLetterWriting::OnHidePHICheck()
{
	try{
		// (e.lally 2009-06-04) PLID 29908 - Use a Dont Show Me Again box to warn the user that this is actually going to remove
		// some data from the output to hopefully alert them that this is not a standard feature.
		if(IsDlgButtonChecked(IDC_HIDE_IDENTIFIER_FIELDS_CHECK)){
			if(IDNO == DontShowMeAgain(this, "Merging with this option selected will remove Protected Health Information from the output. \nAre you sure you wish to enable this?", "LetterWritingHidePHI", "Practice", FALSE, TRUE, FALSE)){
				//They don't want to use this feature, uncheck the box for them.
				CheckDlgButton(IDC_HIDE_IDENTIFIER_FIELDS_CHECK, FALSE);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CLetterWriting::OnPHIHelpBtn()
{
	try{
		// (e.lally 2009-06-04) PLID 29908 - Built in documentation on why we have this feature, the HIPAA definition,
		// and the fields that we are automatically hiding for them.
		CString strMessage = "The option to hide the Protected Health Information (PHI) is typically used when exporting data for "
			"research purposes where the data needs to be anonymous and not contain identifiers.\r\n\r\n"
			" As per HIPAA Standards for Privacy of Individually Identifiable Health Information, 45 CFR Parts 160 and 164, identifiers that shall be removed are: \r\n"
			"1. Names; \r\n"
			"2. Postal address information, other than town or city, state and zip code; \r\n"
			"3. Telephone numbers; \r\n"
			"4. Fax numbers; \r\n"
			"5. Electronic mail addresses; \r\n"
			"6. Social security numbers; \r\n"
			"7. Medical record numbers; \r\n"
			"8. Health plan beneficiary numbers; \r\n"
			"9. Account numbers; \r\n"
			"10. Certificate/license numbers; \r\n"
			"11. Vehicle identifiers and serial numbers, including license plate numbers; \r\n"
			"12. Device identifiers and serial numbers; \r\n"
			"13. Web Universal Resource Locators (URLs); \r\n"
			"14. Internet Protocol (IP) address numbers; \r\n"
			"15. Biometric identifiers, including finger and voice prints; and  \r\n"
			"16. Full face photographic images and any comparable images. \r\n\r\n"

			"This feature will automatically hide data for the following merge fields: \r\n"
			"Person_ID \r\n"
			"Person_First_Name \r\n"
			"Person_Middle_Name \r\n"
			"Person_Middle_Initial \r\n"
			"Person_Last_Name \r\n"
			"Person_First_Middle_Last \r\n"
			"Person_Formal \r\n"
			"Person_Company \r\n"
			"Person_NickName \r\n"
			"Person_Address1 \r\n"
			"Person_Address2 \r\n"
			"Person_Home_Phone \r\n"
			"Person_Work_Phone \r\n"
			"Person_Work_Ext \r\n"
			"Person_Mobile_Phone \r\n"
			"Person_Pager \r\n"
			"Person_Other_Phone \r\n"
			"Person_Fax \r\n"
			"Person_SS_Number \r\n" 
			"Person_Birth_Date \r\n"
			"Person_Patient_Security_Code \r\n"
			"Person_Email \r\n"
			"Person_Insurance_Referral_Auth_Num \r\n"

			"Emergency_Contact_First_Name \r\n"
			"Emergency_Contact_Last_Name \r\n"
			"Emergency_Contact_Home_Phone \r\n"
			"Emergency_Contact_Work_Phone \r\n"
			"Employer_Company_Name \r\n"
			"Employer_First_Name \r\n"
			"Employer_Middle_Name \r\n"
			"Employer_Last_Name \r\n"
			"Employer_Address1 \r\n"
			"Employer_Address2 \r\n"

			"Person_Referring_Pat_First_Name \r\n"
			"Person_Referring_Pat_Last_Name \r\n"
			"Person_Referring_Pat_Address1 \r\n"
			"Person_Referring_Pat_Address2 \r\n"
			"Person_Referring_Pat_Home_Phone \r\n"
			"Person_Referring_Pat_Work_Phone \r\n"
					
			"Person_Last_Referred_Pat_Prefix \r\n"
			"Person_Last_Referred_Pat_First_Name \r\n"
			"Person_Last_Referred_Pat_Last_Name \r\n"
			"Person_Last_Referred_Pat_Address1 \r\n"
			"Person_Last_Referred_Pat_Address2 \r\n"
			"Person_Last_Referred_Pat_Home_Phone \r\n"
			"Person_Last_Referred_Pat_Work_Phone \r\n"
					
			"Person_Primary_First_Name \r\n"
			"Person_Primary_Middle_Name \r\n"
			"Person_Primary_Last_Name \r\n"
			"Person_Primary_Address1 \r\n"
			"Person_Primary_Address2 \r\n"
			"Person_Primary_Employer \r\n"
			"Person_Primary_Phone \r\n"
			"Person_Primary_Ins_Group_Number \r\n"
			"Person_Primary_Ins_ID \r\n"
			"Person_Primary_SSN \r\n"
				
			"Person_Secondary_First_Name \r\n"
			"Person_Secondary_Middle_Name \r\n"
			"Person_Secondary_Last_Name \r\n"
			"Person_Secondary_Address1 \r\n"
			"Person_Secondary_Address2 \r\n"
			"Person_Secondary_Phone \r\n"
			"Person_Secondary_Ins_Group_Number \r\n"
			"Person_Secondary_Ins_ID \r\n"
			"Person_Secondary_SSN \r\n"
			"Person_Secondary_BirthDate \r\n"  
			"Person_Secondary_Employer \r\n"
				
			"Resp_Party_First_Name \r\n"
			"Resp_Party_Middle_Name \r\n"
			"Resp_Party_Middle_Initial \r\n"
			"Resp_Party_Last_Name \r\n"
			"Resp_Party_First_Middle_Last \r\n"
			"Resp_Party_Address1 \r\n"
			"Resp_Party_Address2 \r\n"
			"Resp_Party_Home_Phone \r\n"
			"Resp_Party_Employer \r\n"
			"Resp_Party_BirthDate \r\n"

			"Superbill_ID \r\n"
			"Bill_ID \r\n";
			//Use CMsgBox so that the whole message will be visible.
			CMsgBox dlg(this);
			dlg.m_strWindowText = "Practice - Help";
			dlg.msg = strMessage;
			dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-09-02 11:34) - PLID 62791 - Add Patient Reminder
void CLetterWriting::OnBnClickedAddReminderForPtMerged()
{
	try{
		if (m_btnAddPatientReminder.GetCheck() == BST_CHECKED){
			DontShowMe(this, "LetterWritingPatientReminder");
		}
	}NxCatchAll(__FUNCTION__);
}
