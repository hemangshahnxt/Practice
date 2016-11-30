// EditComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"
#include "EditComboBox.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define QUOTE_NOTES_COMBO 1
#define PAYMENT_DESCRIPTION_COMBO 2
// (j.gruber 2012-11-15 14:01) - PLID 53752 - taking out payment category since it has its own dialog
//#define PAYMENT_CATEGORY_COMBO 3
//(e.lally 2007-07-09) PLID 26590 - Depreciating payment credit card combo functionality for its own edit dlg
//#define PAYMENT_CARD_COMBO 4
// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
//#define PATIENT_TYPE_COMBO 5
#define STATEMENT_NOTE_COMBO 6
#define PROCEDURE_GROUP_COMBO 7
#define MEDSCHEDEVENT_COMBO 8
#define ANESTHESIA_COMBO 9
#define LWSUBJECT_COMBO 10
// (j.jones 2009-10-15 10:28) - PLID 34327 - split race editing into its own dialog
//#define RACE_COMBO	11
#define CANCEL_REASON_COMBO 12
#define NO_SHOW_REASON_COMBO 13
#define LAB_ANATOMIC_LOCATION_COMBO 14
#define LAB_CLINICAL_DIAGNOSIS_COMBO 15
#define LAB_DESCRIPTION_COMBO 16
#define POS_PRINTER_RECEIPT_TYPE_COMBO 17
// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
#define SHIPPING_METHOD_COMBO 18
// (j.gruber 2008-04-02 13:27) - PLID 29443 - Implementation dialog status box
#define IMP_EMR_STATUS_LIST  19
// Note: Custom Lists are designated as numbers 20 through 50
#define CUSTOM_LISTS 20

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Note: I allowed for expansion of custom lists in the future, so
// Outcomes Custom Lists are designated as numbers 50 through 59.
#define OUTCOMES_CUSTOM_LISTS 50
#define OUTCOMES_CUSTOM_LISTS_END 59

// (c.haag 2007-11-12 08:30) - PLID 28010 - Inventory return dialog types
#define INVEDITRETURN_REASON_COMBO	60
#define INVEDITRETURN_METHOD_COMBO	61

// (j.gruber 2008-05-22 12:00) - PLID 30146 - Implementation Dialog's EMR Type List
#define IMP_EMR_TYPE_LIST	62

// (z.manning 2008-10-24 09:33) - PLID 31807 - Labs to be ordered list
#define LAB_TO_BE_ORDERED_COMBO	63

// (j.jones 2009-01-23 09:14) - PLID 32822 - Inventory Order Methods
#define INVEDITORDER_ORDERMETHOD_COMBO	64

// (a.walling 2009-05-04 08:26) - PLID 33751 - Problem Chronicity Combo (reluctantly)
#define PROBLEM_CHRONICITY_COMBO 65

// (j.gruber 2009-05-08 09:38) - PLID 34202 - Provider Types
#define PROVIDER_TYPES 66
// (d.thompson 2009-05-12) - PLID 34232 - Immunization Routes and Sites
#define IMMUNIZATION_ROUTES	67
#define	IMMUNIZATION_SITES	68

// (a.walling 2009-05-28 15:51) - PLID 34389 - EmrDataCodesT
#define EMRDATACODES_COMBO 69

//TES 11/10/2009 - PLID 36128 - AnatomyQualifiersT
#define LAB_ANATOMY_QUALIFIERS_COMBO	70

// (a.walling 2010-01-18 12:53) - PLID 36955 - LabLOINCT
#define LAB_LOINC 71

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_SET_DEFAULT		53654
#define ID_REMOVE_DEFAULT	53655

//(j.deskurakis 2013-01-24) - PLID 53151 - modified
// (f.dinatale 2010-07-08) - PLID 39527 - Enumerations for updating the HL7 Integration Info tables.
#define INTEGRATION_BILLTO		72
#define INTEGRATION_LABTYPES	73

// (d.lange 2010-12-30 18:02) - PLID 29065 - Biopsy Type
#define LAB_BIOPSY_TYPE	75

//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
#define	GLASSES_ORDER_HISTORY_NOTES	76

// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
#define	CONTACT_LENS_TYPE	77
// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
#define	CONTACT_LENS_MANUFACTURE	78
// (b.spivey, August 17, 2012) - PLID 52130 - new list type for Smart Stamp Categories. 
#define EMR_IMAGE_STAMP_CATEGORIES	79
// (r.gonet 07/07/2014) - PLID 62520 - BillStatusNoteT
#define BILL_STATUS_NOTES_COMBO		80
			

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditComboBox dialog

// (j.armen 2012-06-06 15:48) - PLID 49856 - Constructor for when we just have an ID.  No datalist.
CEditComboBox::CEditComboBox(CWnd* pParent, long nListID, const CString& strTitle)
	: CNxDialog(CEditComboBox::IDD, pParent)
	, m_listID(nListID)
	, m_pFromListV1(NULL)
	, m_pFromListV2(NULL)
	, m_strTitle(strTitle)
{
	m_nCurIDInUse = -1;
	m_bAllowDefault = FALSE;
	m_nLastSelID = -1; // (a.walling 2010-01-18 13:17) - PLID 36955
}

// (j.armen 2012-06-06 15:48) - PLID 49856 - Constructor for DL1
CEditComboBox::CEditComboBox(CWnd* pParent, long nListID, NXDATALISTLib::_DNxDataListPtr pFromListV1, const CString& strTitle)
	: CNxDialog(CEditComboBox::IDD, pParent)
	, m_listID(nListID)
	, m_pFromListV1(pFromListV1)
	, m_pFromListV2(NULL)
	, m_strTitle(strTitle)
{
	m_nCurIDInUse = -1;
	m_bAllowDefault = FALSE;
	m_nLastSelID = -1; // (a.walling 2010-01-18 13:17) - PLID 36955
}

// (j.armen 2012-06-06 15:49) - PLID 49856 - Constructor for DL2
CEditComboBox::CEditComboBox(CWnd* pParent, long nListID, NXDATALIST2Lib::_DNxDataListPtr pFromListV2, const CString& strTitle)
	: CNxDialog(CEditComboBox::IDD, pParent)
	, m_listID(nListID)
	, m_pFromListV1(NULL)
	, m_pFromListV2(pFromListV2)
	, m_strTitle(strTitle)
{
	m_nCurIDInUse = -1;
	m_bAllowDefault = FALSE;
	m_nLastSelID = -1; // (a.walling 2010-01-18 13:17) - PLID 36955
}

void CEditComboBox::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditComboBox)
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel); // (a.walling 2010-01-18 13:00) - PLID 36955
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditComboBox, CNxDialog)
	//{{AFX_MSG_MAP(CEditComboBox)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditComboBox message handlers

BEGIN_EVENTSINK_MAP(CEditComboBox, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditComboBox)
	ON_EVENT(CEditComboBox, IDC_EDIT_LIST, 6 /* RButtonDown */, OnRButtonDownEditList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditComboBox, IDC_EDIT_LIST, 9 /* EditingFinishing */, OnEditingFinishingEditList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditComboBox, IDC_EDIT_LIST, 10 /* EditingFinished */, OnEditingFinishedEditList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditComboBox, IDC_EDIT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEditList, VTS_I2)
	ON_EVENT(CEditComboBox, IDC_EDIT_LIST, 2 /* SelChanged */, OnSelChangedEditList, VTS_I4)
	ON_EVENT(CEditComboBox, IDC_EDIT_LIST, 8 /* EditingStarting */, OnEditingStartingList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

_bstr_t CEditComboBox::GetFieldName(short nCol)
{
	if(m_pFromListV1 != NULL && m_pFromListV2 != NULL)
		AfxThrowNxException("An error occurred when getting the field name. At least two versions of the list contain values.");
	else if(m_pFromListV1 != NULL)
		return m_pFromListV1->GetColumn(nCol)->GetFieldName();
	else if(m_pFromListV2 != NULL)
		return m_pFromListV2->GetColumn(nCol)->GetFieldName();
	else
		AfxThrowNxException("An error occurred when getting the field name. None of the versions of the list were set.");
	return "";
}
_bstr_t CEditComboBox::GetColumnTitle(short nCol)
{
	if(m_pFromListV1 != NULL && m_pFromListV2 != NULL)
		AfxThrowNxException("An error occurred when getting the column title. At least two versions of the list contain values.");
	else if(m_pFromListV1 != NULL)
		return m_pFromListV1->GetColumn(nCol)->GetColumnTitle();
	else if(m_pFromListV2 != NULL)
		return m_pFromListV2->GetColumn(nCol)->GetColumnTitle();
	else
		AfxThrowNxException("An error occurred when getting the column title. None of the versions of the list were set.");
	return "";

}

void CEditComboBox::OnAdd() 
{
	try {
		//Note:  Some of these lists use GetRowCount() + 1 because there is no ID numbers for the items in each list.
		//		 Make sure to use the NewNumber() function if the data is in a table with ID numbers
		IRowSettingsPtr pRow;
		pRow = m_pList->GetRow(-1);
		CString newVal;
		long ID;
		if(m_listID == QUOTE_NOTES_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Note %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Note %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO QuoteNotesT (Note) VALUES ('%s')",newVal);
			//we must add, select, and then edit, because editing a row that could
			//potentially be off the screen doesn't fly
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}
		else if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Description %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Description %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO PaymentExtraDescT (ExtraDescription) VALUES ('%s')",newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}
		// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
		/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {	//custom list
			ID = NewNumber("PaymentGroupsT","ID");
			newVal.Format("[Item %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[Item %li]",ID);
			}
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO PaymentGroupsT (ID,GroupName) VALUES (%li,'%s')",ID,newVal);
			CClient::RefreshTable(NetUtils::PaymentGroupsT);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(1,_bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}*/
		//(e.lally 2007-07-09) PLID 26590 - Depreciating this functionality for its own edit dlg
		/*
		else if(m_listID == PAYMENT_CARD_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Card %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Card %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO CreditCardNamesT (CardName) VALUES ('%s')",newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}*/
		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
		/*
		else if(m_listID == PATIENT_TYPE_COMBO) {
			ID = NewNumber("GroupTypes", "TypeIndex");
			newVal.Format("[New Type %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Type %li]",ID);
			}
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO GroupTypes (TypeIndex,GroupName) VALUES (%li,'%s')",ID,newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(1,_bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		*/
		else if(m_listID == STATEMENT_NOTE_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Note %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Note %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO StatementNotesT (Note) VALUES ('%s')",newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}
		//m.hancock - 7-11-2005 - PLID 16755 - Changed to allow room for Outcomes Custom Lists
		//else if(m_listID > CUSTOM_LISTS) {	//custom list
		else if((m_listID > CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS)) {	//custom list
			ID = NewNumber("CustomListItemsT","ID");
			newVal.Format("[Item %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[Item %li]",ID);
			}
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO CustomListItemsT (ID,CustomFieldID,Text) VALUES (%li,%li,'%s')",ID,m_listID,newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(1,_bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		//else if((m_listID == OUTCOMES_CUSTOM_LIST1) || (m_listID == OUTCOMES_CUSTOM_LIST2) || (m_listID == OUTCOMES_CUSTOM_LIST3) || (m_listID == OUTCOMES_CUSTOM_LIST4)) {
		else if((m_listID >= OUTCOMES_CUSTOM_LISTS) && (m_listID <= OUTCOMES_CUSTOM_LISTS_END)) {
			ID = NewNumber("OutcomesListItemsT","ID");
			newVal.Format("[Item %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[Item %li]",ID);
			}
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO OutcomesListItemsT (ID,CustomFieldID,Text) VALUES (%li,%li,'%s')",ID,m_listID,newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(1,_bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		else if(m_listID == PROCEDURE_GROUP_COMBO) {
			ID = NewNumber("ProcedureGroupsT", "ID");
			newVal.Format("[New Group %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Group %li]",ID);
			}
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO ProcedureGroupsT (ID, Name) VALUES (%li, '%s')",ID,newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		else if(m_listID == MEDSCHEDEVENT_COMBO) {
			ID = NewNumber("MedSchedEventsT", "ID");
			newVal.Format("[New Event %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Event %li]",ID);
			}
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO MedSchedEventsT (ID, Name) VALUES (%li, '%s')",ID,newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		else if(m_listID == ANESTHESIA_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Anesthesia Type %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Anesthesia Type %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO AnesthesiaTypes (Anesthesia) VALUES ('%s')",newVal);
			//we must add, select, and then edit, because editing a row that could
			//potentially be off the screen doesn't fly
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}
		else if(m_listID == LWSUBJECT_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Subject %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Subject %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO LetterSubjectT (Subject) VALUES ('%s')",newVal);
			//we must add, select, and then edit, because editing a row that could
			//potentially be off the screen doesn't fly
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}
		// (j.jones 2009-10-15 10:28) - PLID 34327 - split race editing into its own dialog
		/*
		else if (m_listID == RACE_COMBO)
		{
			ID = NewNumber("RaceT", "ID");
			newVal.Format("[New Ethnicity %li]",ID);
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO RaceT (ID, Name) VALUES (%d, '%s')",
				ID,newVal);
			//we must add, select, and then edit, because editing a row that could
			//potentially be off the screen doesn't fly
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		*/
		else if (m_listID == CANCEL_REASON_COMBO)
		{
			ID = NewNumber("AptCancelReasonT", "ID");
			newVal.Format("[New Cancel Reason %li]",ID);
			pRow->PutValue(0,ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO AptCancelReasonT (ID, Description) VALUES (%d, '%s')",
				ID,newVal);
			//we must add, select, and then edit, because editing a row that could
			//potentially be off the screen doesn't fly
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		else if (m_listID == NO_SHOW_REASON_COMBO)
		{
			ID = NewNumber("AptNoShowReasonT", "ID");
			newVal.Format("[New No Show Reason %li]", ID);
			pRow->PutValue(0, ID);
			pRow->PutValue(1,_bstr_t(newVal));
			ExecuteSql("INSERT INTO AptNoShowReasonT (ID, Description) VALUES (%li, '%s')", VarLong(ID), _Q(newVal));
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0, _bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		else if (m_listID == LAB_ANATOMIC_LOCATION_COMBO)
		{
			newVal="[New Anatomic Location]";

			_RecordsetPtr prsID = CreateRecordset("SET NOCOUNT OFF \r\n"
				"INSERT INTO LabAnatomyT (Description) VALUES ('%s');"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum", _Q(newVal));
			_variant_t v;
			prsID = prsID->NextRecordset(&v);
			if (NULL == prsID) {
				//If this is NULL, something bad happened
				AfxThrowNxException("The new record could not be added. Please try again or restart NexTech Practice if the problem persists.");
			}
			long nNewID = AdoFldLong(prsID, "NewNum");

			
			pRow->PutValue(0, nNewID);
			pRow->PutValue(1,_bstr_t(newVal));
			//TES 11/6/2009 - PLID 36189 - There is an Inactive column now.
			pRow->PutValue(2,_variant_t(VARIANT_FALSE, VT_BOOL));
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0, _bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		else if (m_listID == LAB_CLINICAL_DIAGNOSIS_COMBO) {

			ID = m_pList->GetRowCount() + 1;

			//(e.lally 2009-08-10) PLID 31811 - Renamed Dissection to Description
			newVal.Format("[New Microscopic Description %li]", ID);
			
			_RecordsetPtr rsID = CreateRecordset("SET NOCOUNT OFF \r\n"
				"INSERT INTO LabClinicalDiagnosisT (Description) VALUES ('%s');"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum", _Q(newVal));
			_variant_t varRecordset;
			rsID = rsID->NextRecordset(&varRecordset);
			if (rsID == NULL) {
				AfxThrowNxException("The new record could not be added. Please try again or restart NexTech Practice if the problem persists.");
			}
			
			ID = AdoFldLong(rsID, "NewNum");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		else if (m_listID == LAB_DESCRIPTION_COMBO) {

			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Diagnosis %li]", ID);
			
			_RecordsetPtr rsID = CreateRecordset("SET NOCOUNT OFF \r\n"
				"INSERT INTO LabDiagnosisT (Description) VALUES ('%s');"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum", _Q(newVal));
			_variant_t varRecordset;
			rsID = rsID->NextRecordset(&varRecordset);
			if (rsID == NULL) {
				AfxThrowNxException("The new record could not be added. Please try again or restart NexTech Practice if the problem persists.");
			}
			
			ID = AdoFldLong(rsID, "NewNum");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		// (j.gruber 2007-05-07 10:19) - PLID 25931 - add support for POS Receipt combo
		else if (m_listID == POS_PRINTER_RECEIPT_TYPE_COMBO) {

			ID = NewNumber("POSReceiptsT", "ID");

			newVal.Format("[New POS Receipt %li]", ID);
			
			_RecordsetPtr rsID = CreateRecordset("INSERT INTO POSReceiptsT (ID, Name, LocationFormat) "
				" VALUES (%li, '%s', '<Name>\r\n<Address1>\r\n<Address2>\r\n<City> <State>, <Zip>')",
				ID, _Q(newVal));
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
		else if (m_listID == SHIPPING_METHOD_COMBO) {

			ID = NewNumber("ShippingMethodT", "ID");

			newVal.Format("[New Shipping Method %li]", ID);
			
			_RecordsetPtr rsID = CreateRecordset("INSERT INTO ShippingMethodT (ID, Description) "
				" VALUES (%li, '%s')",
				ID, _Q(newVal));
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		// (j.gruber 2008-04-02 13:29) - PLID 29443 - Implementation Status List
		else if (m_listID == IMP_EMR_STATUS_LIST) {

			ID = NewNumber("EMRStatusT", "ID");

			newVal.Format("[New EMR Status %li]", ID);
			
			_RecordsetPtr rsID = CreateRecordset("INSERT INTO EMRStatusT (ID, StatusName) "
				" VALUES (%li, '%s')",
				ID, _Q(newVal));
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		// (c.haag 2007-11-12 08:34) - PLID 28010 - Inventory supplier return methods
		else if (m_listID == INVEDITRETURN_METHOD_COMBO) {
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Return Method %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Return Method %li]",ID);
			}

			
			_RecordsetPtr rsID = CreateRecordset("SET NOCOUNT OFF \r\n"
				"INSERT INTO SupplierReturnMethodT (Method) VALUES ('%s');"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum", _Q(newVal));
			_variant_t varRecordset;
			rsID = rsID->NextRecordset(&varRecordset);
			if (rsID == NULL) {
				AfxThrowNxException("The new record could not be added. Please try again or restart NexTech Practice if the problem persists.");
			}
			
			ID = AdoFldLong(rsID, "NewNum");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);	
		}
		// (c.haag 2007-11-12 09:01) - PLID 28010 - Inventory supplier return reasons
		else if (m_listID == INVEDITRETURN_REASON_COMBO) {
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Return Reason %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Return Reason %li]",ID);
			}
			
			_RecordsetPtr rsID = CreateRecordset("SET NOCOUNT OFF \r\n"
				"INSERT INTO SupplierReturnReasonT (Reason) VALUES ('%s');"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum", _Q(newVal));
			_variant_t varRecordset;
			rsID = rsID->NextRecordset(&varRecordset);
			if (rsID == NULL) {
				AfxThrowNxException("The new record could not be added. Please try again or restart NexTech Practice if the problem persists.");
			}
			
			ID = AdoFldLong(rsID, "NewNum");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);	
		}
		// (j.gruber 2008-05-22 12:03) - PLID 30146 - Implemenation EMR Type List
		else if (m_listID == IMP_EMR_TYPE_LIST) {

			ID = NewNumber("EMRTypeT", "ID");

			newVal.Format("[New EMR Type %li]", ID);
			
			_RecordsetPtr rsID = CreateRecordset("INSERT INTO EMRTypeT (ID, TypeName) "
				" VALUES (%li, '%s')",
				ID, _Q(newVal));
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		// (z.manning 2008-10-24 09:34) - PLID 31807
		else if (m_listID == LAB_TO_BE_ORDERED_COMBO)
		{
			newVal = "[New Lab To Be Ordered]";
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO LabsToBeOrderedT (Description) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");

			// (j.jones 2010-06-25 11:53) - PLID 39185 - link with all labs
			ExecuteParamSql("INSERT INTO LabsToBeOrderedLocationLinkT (LocationID, LabsToBeOrderedID) "
				"SELECT ID, {INT} FROM LocationsT WHERE TypeID = 2", ID);
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			// (c.haag 2009-06-04 10:51) - PLID 34288 - Fill in column 2 with an empty string. Otherwise,
			// it will be left as an empty variant; and when OnEditingFinishingEditList calls VarString
			// on that column, an exception would be thrown because of the empty variant.
			pRow->PutValue(2, _bstr_t(""));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (j.jones 2009-01-23 09:14) - PLID 32822 - added Inventory Order Methods
		else if(m_listID == INVEDITORDER_ORDERMETHOD_COMBO)
		{
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Order Method %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Order Method %li]",ID);
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO OrderMethodsT (Method) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (a.walling 2009-05-04 08:38) - PLID 33751 - Problem Chronicity
		else if(m_listID == PROBLEM_CHRONICITY_COMBO)
		{
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Chronicity Status %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Chronicity Status %li]",ID);
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO EMRProblemChronicityT (Name) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			pRow->PutValue(2, _variant_t(VARIANT_FALSE, VT_BOOL));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (a.walling 2009-05-28 15:53) - PLID 34389 - EmrDataCodesT
		else if(m_listID == EMRDATACODES_COMBO)
		{
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Code %li]", ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Code %li]",ID);
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO EmrDataCodesT (Code, Description, DefaultUnit, Vital, Inactive) VALUES ({STRING}, '', '', 0, 0) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			pRow->PutValue(2, _bstr_t(""));
			pRow->PutValue(3, _bstr_t(""));
			pRow->PutValue(4, _variant_t(VARIANT_FALSE, VT_BOOL));
			pRow->PutValue(5, _variant_t(VARIANT_FALSE, VT_BOOL));
			pRow->PutValue(6, _variant_t(VARIANT_FALSE, VT_BOOL));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (j.gruber 2009-05-08 09:42) - PLID 34202 - Provider Types
		else if (m_listID == PROVIDER_TYPES) {

			ID = NewNumber("ProviderTypesT", "ID");

			newVal.Format("[New Provider Type %li]", ID);
			
			_RecordsetPtr rsID = CreateParamRecordset("INSERT INTO ProviderTypesT(ID, Description) "
				" VALUES ({INT}, {STRING})",
				ID, newVal);
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			ID = m_pList->AddRow(pRow);
			ID = m_pList->SetSelByColumn(1, _bstr_t(newVal));
			m_pList->StartEditing(ID, 1);
		}
		// (d.thompson 2009-05-12) - PLID 34232 - Immunization Routes
		else if(m_listID == IMMUNIZATION_ROUTES) {
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Immunization Route %li]", ID);
			while(m_pList->FindByColumn(1, _bstr_t(newVal), 0, FALSE) != -1) {
				ID++;
				newVal.Format("[New Immunization Route %li]",ID);
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO ImmunizationRoutesT (Name) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (d.thompson 2009-05-12) - PLID 34232 - Immunization Sites
		else if(m_listID == IMMUNIZATION_SITES) {
			ID = m_pList->GetRowCount() + 1;

			newVal.Format("[New Immunization Site %li]", ID);
			while(m_pList->FindByColumn(1, _bstr_t(newVal), 0, FALSE) != -1) {
				ID++;
				newVal.Format("[New Immunization Site %li]",ID);
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO ImmunizationSitesT (Name) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		else if (m_listID == LAB_ANATOMY_QUALIFIERS_COMBO)
		{
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			newVal="[New Anatomic Location Qualifier]";

			_RecordsetPtr prsID = CreateRecordset("SET NOCOUNT OFF \r\n"
				"INSERT INTO AnatomyQualifiersT (Name) VALUES ('%s');"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum", _Q(newVal));
			_variant_t v;
			prsID = prsID->NextRecordset(&v);
			if (NULL == prsID) {
				//If this is NULL, something bad happened
				AfxThrowNxException("The new record could not be added. Please try again or restart NexTech Practice if the problem persists.");
			}
			long nNewID = AdoFldLong(prsID, "NewNum");

			
			pRow->PutValue(0, nNewID);
			pRow->PutValue(1,_bstr_t(newVal));
			pRow->PutValue(2,_variant_t(VARIANT_FALSE, VT_BOOL));
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0, _bstr_t(newVal));
			m_pList->StartEditing(ID,1);
		}
		// (a.walling 2010-01-18 13:04) - PLID 36955
		else if(m_listID == LAB_LOINC)
		{
			long nDup = 0;
			do {
				nDup++;
				newVal.Format("[New LOINC %li]",nDup);
			} while(m_pList->FindByColumn(1,_bstr_t(newVal),0,VARIANT_FALSE)!=-1);

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO LabLOINCT (Code, ShortName, LongName) VALUES ({STRING}, '', '') \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");
						
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			pRow->PutValue(2, _bstr_t(""));
			pRow->PutValue(3, _bstr_t(""));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		//(j.deskurakis 2013-01-24) - PLID 53151 - modified
		// (f.dinatale 2010-07-08) - PLID 39527
		// (f.dinatale 2010-08-13) - PLID 39527 - Fixed the issue with non-unique standard names for new items.
		else if(m_listID == INTEGRATION_BILLTO)
		{
			long nCount = 0;

			do{
				nCount++;
				newVal.Format("[New Bill To %li]", nCount);
			}while(m_pList->FindByColumn(1,_bstr_t(newVal),0,VARIANT_FALSE)!=-1);

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO IntegrationBillToT (Description) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n", newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");

			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		else if(m_listID == INTEGRATION_LABTYPES)
		{
			long nCount = 0;

			do{
				nCount++;
				newVal.Format("[New Integration Type %li]", nCount);
			}while(m_pList->FindByColumn(1,_bstr_t(newVal),0,VARIANT_FALSE)!=-1);

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO IntegrationLabTypesT (Description) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n", newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");

			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		
		else if(m_listID == LAB_BIOPSY_TYPE)	// (d.lange 2011-01-03 09:00) - PLID 29065 - Added Biopsy Type
		{
			long nCount = 0;
			do {
				nCount++;
				newVal.Format("[New Biopsy Type %li]", nCount);
			} while(m_pList->FindByColumn(1, _bstr_t(newVal), 0, VARIANT_FALSE) != -1);

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO LabBiopsyTypeT (Description) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n", newVal);

			ID = AdoFldLong(prs->GetFields(), "NewID");

			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			pRow->PutValue(2, _variant_t(VARIANT_FALSE, VT_BOOL));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		else if(m_listID == GLASSES_ORDER_HISTORY_NOTES)
		{
			//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
			// (j.dinatale 2012-04-17 12:27) - PLID 49078 - we now call orders 'optical orders' since its more generic
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Optical Order History Note %li]",ID);
			while(m_pList->FindByColumn(0,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Optical Order History Note %li]",ID);
			}
			pRow->PutValue(0,_bstr_t(newVal));
			ExecuteSql("INSERT INTO GlassesOrderHistoryNotesT (Note) VALUES ('%s')",newVal);
			ID = m_pList->AddRow(pRow);
			m_pList->SetSelByColumn(0,_bstr_t(newVal));
			m_pList->StartEditing(ID,0);
		}
		else if(m_listID == CONTACT_LENS_TYPE)
		{
			// (s.dhole 2012-03-19 16:04) - PLID 49191 Add cotact lens type
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Contact Lens Type %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Contact Lens Type %li]",ID);
			}
		//	pRow->PutValue(0,_bstr_t(newVal));
			
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO GlassesContactLensTypeT (ContactLensType) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n", newVal);
			ID = AdoFldLong(prs->GetFields(), "NewID");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (s.dhole 2012-03-19 16:08) - PLID 48973
		else if(m_listID == CONTACT_LENS_MANUFACTURE)
		{
			// (s.dhole 2012-03-14 10:06) - PLID 48856 Contact Manufacturer 
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Lens Manufacturer %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Lens Manufacturer %li]",ID);
			}
			//pRow->PutValue(0,_bstr_t(newVal));
			
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO ContactLensManufacturersT (Name) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n", newVal);
			ID = AdoFldLong(prs->GetFields(), "NewID");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (b.spivey, August 17, 2012) - PLID 52130 - Adding new image stamp category. 
		else if (m_listID == EMR_IMAGE_STAMP_CATEGORIES) {
			
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Image Stamp Category %li]",ID);
			while(m_pList->FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
				ID++;
				newVal.Format("[New Image Stamp Category %li]",ID);
			}
		
			
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO EmrImageStampCategoryT (Description) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS NewID \r\n", newVal);
			ID = AdoFldLong(prs->GetFields(), "NewID");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);
		}
		// (r.gonet 07/07/2014) - PLID 62520 - Add a new BillStatusNoteT record.
		else if (m_listID == BILL_STATUS_NOTES_COMBO) {
			ID = m_pList->GetRowCount() + 1;
			newVal.Format("[New Bill Status Note %li]", ID);
			while (m_pList->FindByColumn(1, _bstr_t(newVal), 0, FALSE) != -1) {
				ID++;
				newVal.Format("[New Bill Status Note %li]", ID);
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO BillStatusNoteT (TextData) VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID \r\n"
				, newVal);
			ID = AdoFldLong(prs->Fields, "NewID");
			pRow->PutValue(0, ID);
			pRow->PutValue(1, _bstr_t(newVal));
			long nRow = m_pList->AddRow(pRow);
			m_pList->PutCurSel(nRow);
			m_pList->StartEditing(nRow, 1);

			CClient::RefreshTable(NetUtils::BillStatusNoteT, ID);
		}
		
		EnableAppropriateButtons();

	}NxCatchAll("Error in adding new item to list.");
}

void CEditComboBox::OnEdit() 
{
	//(e.lally 2008-10-13) PLID 31665 - Added error handling
	try{
		if(m_pList->GetCurSel()==-1) {
			AfxMessageBox("Please select an entry first.");
			return;
		}
		//if there is only one column (no ID column)
		//(e.lally 2007-07-09) PLID 26590 - Depreciating payment credit card combo functionality for its own edit dlg
		//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
		// (s.dhole 2012-03-14 10:06) - PLID 48856 Contact lens Type of Lens Product    
		if(m_listID == QUOTE_NOTES_COMBO || m_listID == PAYMENT_DESCRIPTION_COMBO || m_listID == STATEMENT_NOTE_COMBO || m_listID == ANESTHESIA_COMBO || m_listID == LWSUBJECT_COMBO
			|| m_listID == GLASSES_ORDER_HISTORY_NOTES ) {
			m_pList->StartEditing(m_pList->GetCurSel(),0);
		}
		//if there are two columns (has an ID column)
		//m.hancock - 7-11-2005 - PLID 16755 - Changed to allow room for Outcomes Custom Lists
		// (j.gruber 2007-05-07 10:19) - PLID 25931 - add support for POS Config combos
		// (c.haag 2007-11-12 09:02) - PLID 28010 - Added support for inventory return combos
		// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
		// (j.gruber 2008-04-02 13:30) - PLID 29443 - added implementation emr status
		// (j.gruber 2008-05-22 12:04) - PLID 30146 - added implementation Type list
		// (z.manning 2008-10-24 09:43) - PLID 31807 - Lab to be ordered
		// (j.jones 2009-01-23 09:14) - PLID 32822 - added Inventory Order Methods		
		// (a.walling 2009-05-04 08:35) - PLID 33751 - Problem Chronicity
		// (j.gruber 2009-05-08 09:43) - PLID 34202 - Provider Types
		// (d.thompson 2009-05-12) - PLID 34232 - Immunization Routes and Sites
		// (a.walling 2009-05-28 15:56) - PLID 34389 - EmrDataCodesT
		//TES 11/10/2009 - PLID 36128 - AnatomyQualifiersT
		// (a.walling 2010-01-18 13:08) - PLID 36955 - LabLOINCT
		// also, this is terrible, but we don't have time to refactor all this code.
		// (f.dinatale 2010-07-08) - PLID 39527 - Integration Links, Types, and LabTypes
		// (d.lange 2011-01-03 09:01) - PLID 29065 - Added Biopsy Type
		// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product    
		// (s.dhole 2012-03-19 16:09) - PLID Contact lens MANUFACTURE) 
		// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer		{
		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated 'PATIENT_TYPE_COMBO' in favor of a new dialog.
		// (b.spivey, August 17, 2012) - PLID 52130 - Added Image Stamp categories. 
		//(j.deskurakis 2013-01-24) - PLID 53151 - Labs In Internal
		// (r.gonet 07/07/2014) - PLID 62520 - Allow editing of a BillStatusNoteT record.
		else if(((m_listID > CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS)) || 
			((m_listID >= OUTCOMES_CUSTOM_LISTS) && (m_listID <= OUTCOMES_CUSTOM_LISTS_END)) || 
			m_listID == PROCEDURE_GROUP_COMBO || 
			m_listID == MEDSCHEDEVENT_COMBO || m_listID == CANCEL_REASON_COMBO || 
			m_listID == NO_SHOW_REASON_COMBO || m_listID == LAB_ANATOMIC_LOCATION_COMBO || m_listID == LAB_CLINICAL_DIAGNOSIS_COMBO ||
			m_listID == LAB_DESCRIPTION_COMBO ||  m_listID == POS_PRINTER_RECEIPT_TYPE_COMBO || m_listID == SHIPPING_METHOD_COMBO|| m_listID == IMP_EMR_STATUS_LIST || m_listID == INVEDITRETURN_METHOD_COMBO ||
			m_listID == INVEDITRETURN_REASON_COMBO || m_listID == IMP_EMR_TYPE_LIST || m_listID == LAB_TO_BE_ORDERED_COMBO ||
			m_listID == INVEDITORDER_ORDERMETHOD_COMBO || m_listID == PROBLEM_CHRONICITY_COMBO || m_listID == PROVIDER_TYPES
			|| m_listID == IMMUNIZATION_ROUTES || m_listID == IMMUNIZATION_SITES || m_listID == EMRDATACODES_COMBO || m_listID == LAB_ANATOMY_QUALIFIERS_COMBO
			|| m_listID == LAB_LOINC || m_listID == INTEGRATION_BILLTO || m_listID == INTEGRATION_LABTYPES
			|| m_listID == LAB_BIOPSY_TYPE ||  m_listID == CONTACT_LENS_TYPE || m_listID == CONTACT_LENS_MANUFACTURE 
			|| m_listID == EMR_IMAGE_STAMP_CATEGORIES
			|| m_listID == BILL_STATUS_NOTES_COMBO) {

			m_pList->StartEditing(m_pList->GetCurSel(),1);
		}
	}NxCatchAll("Error in editing item.");
}

void CEditComboBox::OnDelete() 
{
	//(e.lally 2008-10-13) PLID 31665 - Expanded error handling to include all code in this function
	try {
		if(m_pList->GetCurSel()==-1) {
			AfxMessageBox("Please select an entry first.");
			return;
		}

		if(MessageBox("Are you sure you wish to delete this item?", "Delete?", MB_YESNO | MB_ICONQUESTION) == IDNO)
			return;

		if(m_listID == QUOTE_NOTES_COMBO) {
			ExecuteSql("DELETE FROM QuoteNotesT WHERE Note = '%s'",_Q(CString(m_pList->GetValue(m_pList->GetCurSel(),0).bstrVal)));
		}
		else if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
			ExecuteSql("DELETE FROM PaymentExtraDescT WHERE ExtraDescription = '%s'",_Q(CString(m_pList->GetValue(m_pList->GetCurSel(),0).bstrVal)));
			if(IsDefaultSelected()) {
				SetRemotePropertyText("DefaultPayDesc","",0,"<None>");
			}
		}
		// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
		/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {
			// (c.haag 2007-02-20 14:26) - PLID 24255 - Warn the user if this category is in use by payments
			const long nGroupID = VarLong(m_pList->GetValue(m_pList->GetCurSel(),0));
			if (ReturnsRecords("SELECT 1 FROM PaymentsT INNER JOIN LineItemT ON LineItemT.ID = PaymentsT.ID WHERE PaymentGroupID = %d AND LineItemT.Deleted = 0", nGroupID)) {
				if (IDNO == MessageBox("This payment category is in use by payments. Do you wish to continue?", "Delete?", MB_YESNO | MB_ICONQUESTION)) {
					return;
				}
			}
			// (c.haag 2007-02-20 14:29) - PLID 24255 - Warn the user if this category is in use by e-remittance preferences
			if (ReturnsRecords("SELECT 1 FROM ConfigRT WHERE IntParam = %d AND Name IN ('DefaultERemitPayCategory', 'DefaultERemitAdjCategory')", nGroupID)) {
				if (IDNO == MessageBox("This payment category is used as a default category for E-Remittance processing. Do you wish to continue?", "Delete?", MB_YESNO | MB_ICONQUESTION)) {
					return;
				}
			}

			// (j.jones 2008-07-10 11:59) - PLID 28756 - Warn the user if this category is in use by any insurance company
			if (ReturnsRecords("SELECT TOP 1 PersonID FROM InsuranceCoT WHERE DefaultPayCategoryID = %li OR DefaultAdjCategoryID = %li", nGroupID, nGroupID)) {
				if (IDNO == MessageBox("This payment category is used as a default category for at least one insurance company. Do you wish to continue?", "Delete?", MB_YESNO | MB_ICONQUESTION)) {
					return;
				}
			}

			// (j.jones 2008-07-10 11:59) - PLID 28756 - clear out its usage on insurance companies
			ExecuteParamSql("UPDATE InsuranceCoT SET DefaultPayCategoryID = NULL WHERE DefaultPayCategoryID = {INT}", nGroupID);
			ExecuteParamSql("UPDATE InsuranceCoT SET DefaultAdjCategoryID = NULL WHERE DefaultAdjCategoryID = {INT}", nGroupID);
			ExecuteParamSql("DELETE FROM PaymentGroupsT WHERE ID = {INT}", nGroupID);
			CClient::RefreshTable(NetUtils::PaymentGroupsT);
			if(IsDefaultSelected()) {
				SetRemotePropertyInt("DefaultPayCat",-1,0,"<None>");
			}
		}*/
		//(e.lally 2007-07-09) PLID 26590 - Depreciating this functionality for its own edit dlg
		/*
		else if(m_listID == PAYMENT_CARD_COMBO) {
			CString str;
			str.Format("DELETE FROM CreditCardNamesT WHERE CardName = '%s'",_Q(CString(m_pList->GetValue(m_pList->GetCurSel(),0).bstrVal)));
			ExecuteSql("%s",str);
		}*/
		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
		/*
		else if(m_listID == PATIENT_TYPE_COMBO) {
			
			_RecordsetPtr rsNum = CreateRecordset("SELECT Count(PersonID) as NumOfPats FROM PatientsT WHERE TypeOfPatient = %li", VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
			BOOL bDelete = FALSE;
			if (rsNum->eof){

				//there are no patients of this type, so go ahead and let them delete it
				bDelete = TRUE;
			}
			else {

				//give them a messagebox
				long nNumber = VarLong(AdoFldLong(rsNum, "NumofPats"));
				if (nNumber > 0) {

					CString strMsg;
					strMsg.Format("There are %li patient(s) with this type.  Are you sure you want to delete it?", nNumber);
					if (IDYES == MsgBox(MB_YESNO, strMsg)) {

						bDelete = TRUE;
					}
				}
				else {
					bDelete = TRUE;
				}

			}
			if (bDelete) {

				ExecuteSql("DELETE FROM GroupTypes WHERE TypeIndex = %li",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
				//Let's update patients with this type
				ExecuteSql("UPDATE PatientsT SET TypeOfPatient = NULL WHERE TypeOfPatient = %li", VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
			}
			else {
				return;
			}
		}
		*/
		else if(m_listID == STATEMENT_NOTE_COMBO) {
			ExecuteSql("DELETE FROM StatementNotesT WHERE Note = '%s'",_Q(CString(m_pList->GetValue(m_pList->GetCurSel(),0).bstrVal)));
		}
		else if(m_listID == PROCEDURE_GROUP_COMBO) {
			ExecuteSql("UPDATE ProcedureT SET ProcedureGroupID = -1 WHERE ProcedureGroupID = %li", m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			ExecuteSql("DELETE FROM ProcedureGroupsT WHERE ID = %li", m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
		}
		else if(m_listID == MEDSCHEDEVENT_COMBO) {
			ExecuteSql("DELETE FROM MedSchedDetailsT WHERE EventID = %li",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			ExecuteSql("DELETE FROM MedSchedEventsT WHERE ID = %li",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
		}
		else if(m_listID == ANESTHESIA_COMBO) {
			if(VarString(m_pList->GetValue(m_pList->GetCurSel(), 0), "") == "Local" ||
				VarString(m_pList->GetValue(m_pList->GetCurSel(), 0), "") == "Local With IV Sedation" ||
				VarString(m_pList->GetValue(m_pList->GetCurSel(), 0), "") == "General") {
				if(IDYES != MsgBox(MB_YESNO, "This anesthesia type is used in some pre-defined Letter Writing templates.  Are you sure you wish to delete it?")) {
					return;
				}
			}
			ExecuteSql("DELETE FROM AnesthesiaTypes WHERE Anesthesia = '%s'", _Q(VarString(m_pList->GetValue(m_pList->GetCurSel(), 0), "")));
		}
		else if(m_listID == LWSUBJECT_COMBO) {
			ExecuteSql("DELETE FROM LetterSubjectT WHERE Subject = '%s'", _Q(VarString(m_pList->GetValue(m_pList->GetCurSel(), 0), "")));
		}
		// (j.jones 2009-10-15 10:28) - PLID 34327 - split race editing into its own dialog
		/*
		else if(m_listID == RACE_COMBO) {
			CString strSQL;
			strSQL.Format("SELECT ID FROM PersonT WHERE RaceID = %d", 
				m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			long nCount = GetRecordCount(strSQL);
			if (nCount > 0)
			{
				MsgBox("You may not delete this ethnicity description because there are %d patient(s) associated with it.", nCount);
				return;
			}
			//(e.lally 2006-01-18) PLID 18885 - We have a table now that references the RaceT.ID, we can delete that reference in
				//Tops_EthnicityLinkT without prompting.
			ExecuteSql("DELETE FROM Tops_EthnicityLinkT WHERE PracEthnicityID = %d",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			ExecuteSql("DELETE FROM RaceT WHERE ID = %d", m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
		}
		*/
		else if(m_listID == CANCEL_REASON_COMBO) {	
			CString strSQL;
			strSQL.Format("SELECT CancelReasonID "
						"FROM AppointmentsT "
						"WHERE CancelReasonID = %li", VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
			long nCount = GetRecordCount(strSQL);
			if (nCount > 0)
			{
				MsgBox("You may not delete this cancel reason because there are %li appointments using it.", nCount);
				return;
			}
			ExecuteSql("DELETE FROM AptCancelReasonT WHERE ID = %li",VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
		}
		else if(m_listID == NO_SHOW_REASON_COMBO) {
			ExecuteSql("DELETE FROM AptNoShowReasonT WHERE ID = %li",VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
		}
		//m.hancock - 7-11-2005 - PLID 16755 - Changed to allow room for Outcomes Custom Lists
		//else if(m_listID > CUSTOM_LISTS) {	//custom list
		else if((m_listID > CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS)) {	//custom list
			// (z.manning, 08/07/2007) - PLID 26959 - We should warn if this list element is in use anywhere.
			long nListElementID = VarLong(m_pList->GetValue(m_pList->GetCurSel(), 0));
			// (j.armen 2011-06-22 11:08) - PLID 11490 - data now stored in CustomListDataT
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT COUNT(*) AS Count FROM CustomListDataT WHERE FieldID = {INT} AND CustomListItemsID = {INT}",
				m_listID, nListElementID);

			long nCount = AdoFldLong(prs, "Count", 0);
			if(nCount > 0)
			{
				int nResult = MessageBox(FormatString(
					"This list element has been selected on %li patient(s). Are you sure you want to delete it? "
					"(This can not be undone.)", nCount), NULL, MB_YESNO);
				if(nResult != IDYES) {
					return;
				}
			}
			
			// (z.manning, 08/07/2007) - PLID 26959 - Make sure we also clear out any patient data for this list item.
			// (j.armen 2011-06-22 11:08) - PLID 11490 - Data now stored in CustomListDataT
			ExecuteParamSql("DELETE FROM CustomListDataT WHERE FieldID = {INT} AND CustomListItemsID = {INT}", m_listID, nListElementID);
			//(e.lally 2011-05-05) PLID 43481 - remove NexWeb display setup
			ExecuteParamSql("DELETE FROM NexWebDisplayT WHERE CustomListItemID = {INT}", nListElementID);

			ExecuteSql("DELETE FROM CustomListItemsT WHERE ID = %li", nListElementID);
		}	
		//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		else if(((m_listID >= OUTCOMES_CUSTOM_LISTS) && (m_listID <= OUTCOMES_CUSTOM_LISTS_END))) {
			//Get count of referenced records for EyeVisitsListDataT
			_RecordsetPtr rsCustom = CreateRecordset("SELECT count(*) AS NumRecords FROM EyeVisitsListDataT WHERE ListItemID = %li", m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			int long nRecordCount = AdoFldLong(rsCustom, "NumRecords");

			//Get count of referenced records for OutcomesListDataT
			rsCustom = CreateRecordset("SELECT count(*) AS NumRecords FROM OutcomesListDataT WHERE ListItemID = %li", m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			nRecordCount += AdoFldLong(rsCustom, "NumRecords");

			//Display count to user
			if(nRecordCount > 0)
			{
				CString strMessage;
				strMessage.Format("There are currently %li records that reference this item.  Are you certain you want to delete it and all records that reference it?", nRecordCount);
				if(MessageBox(strMessage, "Delete?", MB_YESNO) == IDNO)
					return;
			}

			//Delete from EyeVisitsListDataT
			ExecuteSql("DELETE FROM EyeVisitsListDataT WHERE ListItemID = %li",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
			
			//Delete from OutcomesListDataT
			ExecuteSql("DELETE FROM OutcomesListDataT WHERE ListItemID = %li",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);

			//Delete the custom list record
			ExecuteSql("DELETE FROM OutcomesListItemsT WHERE ID = %li",m_pList->GetValue(m_pList->GetCurSel(),0).lVal);
		}
		else if(m_listID == LAB_ANATOMIC_LOCATION_COMBO) {
			//TES 2/9/2010 - PLID 37223 - Need to also check for HotSpots
			long nAnatomyID = VarLong(m_pList->GetValue(m_pList->CurSel,0),-1);
			// (c.haag 2011-06-17) - PLID 37307 - Make sure it's not currently in use
			if (m_nCurIDInUse == nAnatomyID)
			{
				MessageBox("This anatomic location cannot be deleted; it is in use by the current lab.", NULL, MB_ICONSTOP);
				return;
			}
			else
			{
				// (z.manning 2011-06-28 12:21) - PLID 44347 - Also get the EMR action count from this query
				_RecordsetPtr rsCounts = CreateParamRecordset(
					"SELECT \r\n"
					"	(SELECT Count(*) FROM LabsT WHERE AnatomyID = {INT}) AS LabCount, \r\n"
					"	(SELECT Count(*) FROM EmrImageHotSpotsT WHERE AnatomicLocationID = {INT}) AS HotSpotCount, \r\n"
					"	(SELECT COUNT(DISTINCT EmrActionID) FROM EmrActionAnatomicLocationsT WHERE LabAnatomyID = {INT}) AS ActionCount \r\n"
					, nAnatomyID, nAnatomyID, nAnatomyID);
				long nLabCount = AdoFldLong(rsCounts, "LabCount", 0);
				long nHotSpotCount = AdoFldLong(rsCounts, "HotSpotCount", 0);
				if(nLabCount > 0 || nHotSpotCount > 0)
				{
					MsgBox("You may not delete this anatomic location because there are %d Lab(s) and %d EMR Image HotSpot(s) associated with it.", 
						nLabCount, nHotSpotCount);
					return;
				}

				// (z.manning 2011-06-28 12:20) - PLID 44347 - Don't allow deletion of anatomic locations that are tied to actions.
				long nActionCount = AdoFldLong(rsCounts, "ActionCount", 0);
				if(nActionCount > 0) {
					MessageBox(FormatString("You may not delete this anatomic location because there are %li EMR action(s)associated with it.", nActionCount), NULL, MB_OK|MB_ICONERROR);
					return;
				}

				ExecuteParamSql("DELETE FROM LabAnatomyT WHERE ID = {INT}",VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
			}
		}
		else if (m_listID == LAB_CLINICAL_DIAGNOSIS_COMBO) {
			// (j.jones 2007-07-19 15:00) - PLID 26751 - removed LabPatientClinicalDiagnosisT
			/*
			CString strSql;
			strSql.Format("SELECT ID FROM LabPatientClinicalDiagnosisT WHERE DiagnosisID = %li ", VarLong(m_pList->GetValue(m_pList->CurSel, 0)));
			long nCount = GetRecordCount(strSql);
			if (nCount > 0) {
				MsgBox("There are currently %li lab(s) associated with this diagnosis, it cannot be deleted", nCount);
				return;
			}
			*/
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			// (j.gruber 2016-02-10 10:55) - PLID 68154 - also delete from LabClinicalDiagnosisLinkT
			ExecuteParamSql("DELETE FROM LabClinicalDiagnosisLinkT WHERE LabClinicalDiagnosisID = {INT}", nID);

			// (j.gruber 2016-02-10 10:55) - PLID 68154 - changed this to a param since I was here.
			ExecuteParamSql("DELETE FROM LabClinicalDiagnosisT WHERE ID = {INT} ", nID);
		}
		else if (m_listID == LAB_DESCRIPTION_COMBO) {
			// (j.jones 2007-07-19 15:00) - PLID 26751 - removed LabPatientDiagnosisT
			/*
			CString strSql;
			strSql.Format("SELECT ID FROM LabPatientDiagnosisT WHERE DiagnosisID = %li ", VarLong(m_pList->GetValue(m_pList->CurSel, 0)));
			long nCount = GetRecordCount(strSql);
			if (nCount > 0) {
				MsgBox("There are currently %li lab(s) associated with this diagnosis, it cannot be deleted", nCount);
				return;
			}
			*/
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			// (j.gruber 2016-02-10 10:55) - PLID 68154 - this is dead code, but I deleted from here just in case
			ExecuteParamSql("DELETE FROM LabClinicalDiagnosisLinkT WHERE LabDiagnosisID = {INT}", nID);

			ExecuteParamSql("DELETE FROM LabDiagnosisT WHERE ID = {INT} ", nID);
		}
		// (j.gruber 2007-05-07 10:20) - PLID 25931 - add support for POS Receipt Printers and receipts
		else if (m_listID == POS_PRINTER_RECEIPT_TYPE_COMBO) {
			CString strSql;
			strSql.Format("SELECT ID FROM POSReceiptsT");
			long nCount = GetRecordCount(strSql);
			if (nCount == 1) {
				MsgBox("This operation will delete the only remaining receipt format, you must have at least one Receipt Format.");
				return;
			}
			ExecuteSql("DELETE FROM POSReceiptsT WHERE ID = %li", VarLong(m_pList->GetValue(m_pList->CurSel, 0)));
			//delete this as a default setting
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'POSReceiptDefaultSetting' AND IntParam = %li", VarLong(m_pList->GetValue(m_pList->CurSel, 0)));
		}
		// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
		else if (m_listID == SHIPPING_METHOD_COMBO) {
			long nMethodID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));
			
			CString strSql;
			_RecordsetPtr prs = CreateParamRecordset("SELECT Top 1 ID FROM OrderT WHERE ShipMethodID = {INT} ", nMethodID);
			if (!prs->eof) {			
				
				MsgBox("You may not delete this Shipping Method because it is in use by existing orders.");
				return;
			}
			ExecuteSql("DELETE FROM ShippingMethodT WHERE ID = %li", nMethodID);			
		}
		// (j.gruber 2008-04-02 13:31) - PLID 29443 - Implemenation EMR Status
		else if (m_listID == IMP_EMR_STATUS_LIST) {
			long nMethodID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));
			
			CString strSql;
			_RecordsetPtr prs = CreateParamRecordset("SELECT Top 1 PersonID FROM NxClientsT WHERE EMRStatusID = {INT} ", nMethodID);
			if (!prs->eof) {			
				
				MsgBox("You may not delete this EMR Status because it is assigned to at least one client.");
				return;
			}
			ExecuteSql("DELETE FROM EMRStatusT WHERE ID = %li", nMethodID);			
		}
		// (c.haag 2007-11-12 09:02) - PLID 28010 - Support inventory return methods
		else if (m_listID == INVEDITRETURN_METHOD_COMBO) {
			long nMethodID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			// We can't delete this row if the selection is in use in data
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM SupplierReturnGroupsT WHERE ReturnMethodID = {INT}", nMethodID);
			if (!prs->eof) {
				MsgBox("You may not delete this method because it is in use by existing supplier returns.");
				return;
			}

			// Now that validation is done, delete the selection
			ExecuteSql("DELETE FROM SupplierReturnMethodT WHERE ID = %d", nMethodID);
		}
		// (c.haag 2007-11-12 09:16) - PLID 28010 - Support inventory return reasons
		else if (m_listID == INVEDITRETURN_REASON_COMBO) {
			long nReasonID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			// We can't delete this row if the selection is in use in data
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM SupplierReturnItemsT WHERE ReturnReasonID = {INT}", nReasonID);
			if (!prs->eof) {
				MsgBox("You may not delete this reason because it is in use by existing supplier returns.");
				return;
			}

			// Now that validation is done, delete the selection
			ExecuteSql("DELETE FROM SupplierReturnReasonT WHERE ID = %d", nReasonID);
		}
		else if (m_listID == IMP_EMR_TYPE_LIST) {
			long nMethodID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));
			
			CString strSql;
			_RecordsetPtr prs = CreateParamRecordset("SELECT Top 1 PersonID FROM NxClientsT WHERE EMRTypeID = {INT} ", nMethodID);
			if (!prs->eof) {			
				
				MsgBox("You may not delete this EMR Type because it is assigned to at least one client.");
				return;
			}
			ExecuteSql("DELETE FROM EMRTypeT WHERE ID = %li", nMethodID);			
		}
		// (z.manning 2008-10-24 09:44) - PLID 31807
		else if (m_listID == LAB_TO_BE_ORDERED_COMBO)
		{
			long nID = VarLong(m_pList->GetValue(m_pList->GetCurSel(), 0));
			// (j.jones 2010-06-25 12:01) - PLID 39185 - clear out LabsToBeOrderedLocationLinkT
			ExecuteParamSql("DELETE FROM LabsToBeOrderedLocationLinkT WHERE LabsToBeOrderedID = {INT}", nID);
			ExecuteParamSql("DELETE FROM LabsToBeOrderedT WHERE ID = {INT}", nID);
		}
		// (j.jones 2009-01-23 09:26) - PLID 32822 - added Inventory Order Methods
		else if(m_listID == INVEDITORDER_ORDERMETHOD_COMBO)
		{
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			//see if this item is in use
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderT WHERE OrderMethodID = {INT}", nID);
			if (!prs->eof) {
				MsgBox("You may not delete this order method because it is in use by existing orders.");
				return;
			}

			//now we can delete the data
			ExecuteParamSql("DELETE FROM OrderMethodsT WHERE ID = {INT}", nID);
		}
		else if (m_listID == PROVIDER_TYPES) {
			long nMethodID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));
			
			CString strSql;
			_RecordsetPtr prs = CreateParamRecordset("SELECT Top 1 ID FROM EMROtherProvidersT WHERE ProvTypeID = {INT} ", nMethodID);
			if (!prs->eof) {			
				
				MsgBox("You may not delete this Provider Type because it is assigned on at least one EMR.");
				return;
			}
			ExecuteParamSql("DELETE FROM ProviderTypesT WHERE ID = {INT}", nMethodID);			
		}
		// (d.thompson 2009-05-12) - PLID 34232 - Immunization Routes
		else if(m_listID == IMMUNIZATION_ROUTES) {
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			//see if this item is in use
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM PatientImmunizationsT WHERE RouteID = {INT}", nID);
			if (!prs->eof) {
				MsgBox("You may not delete this route because it is in use by existing immunizations.");
				return;
			}

			//now we can delete the data
			ExecuteParamSql("DELETE FROM ImmunizationRoutesT WHERE ID = {INT}", nID);
		}
		// (d.thompson 2009-05-12) - PLID 34232 - Immunization Sites
		else if(m_listID == IMMUNIZATION_SITES) {
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			//see if this item is in use
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM PatientImmunizationsT WHERE SiteID = {INT}", nID);
			if (!prs->eof) {
				MsgBox("You may not delete this site because it is in use by existing immunizations.");
				return;
			}

			//now we can delete the data
			ExecuteParamSql("DELETE FROM ImmunizationSitesT WHERE ID = {INT}", nID);
		} else if (m_listID == PROBLEM_CHRONICITY_COMBO) {
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			long nProblemCount = 0;
			long nDeletedProblemCount = 0;
			long nHistoryCount = 0;
			_RecordsetPtr prsHistory = CreateParamRecordset("SELECT "
				"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 0 AND ChronicityID = {INT}) AS ProblemCount, "
				"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 1 AND ChronicityID = {INT}) AS DeletedProblemCount, "
				"(SELECT COUNT(*) FROM EMRProblemHistoryT WHERE ChronicityID = {INT}) AS HistoryCount",
				nID, nID, nID);
			if (!prsHistory->eof) {
				nProblemCount = AdoFldLong(prsHistory, "ProblemCount", 0);
				nDeletedProblemCount = AdoFldLong(prsHistory, "DeletedProblemCount", 0);
				nHistoryCount = AdoFldLong(prsHistory, "HistoryCount", 0); // includes the first/initial status
			}

			CString strMessage;

			if (nProblemCount != 0) {
				strMessage += FormatString("%li problems.\r\n", nProblemCount);
			}
			if (nDeletedProblemCount != 0) {
				strMessage += FormatString("%li deleted problems.\r\n", nDeletedProblemCount);
			}
			if (nHistoryCount != 0) {
				strMessage += FormatString("%li problem history records.\r\n", nHistoryCount);
			}
			// (a.walling 2009-07-20 09:12) - PLID 33751 - If all are 0, but there is one chosen on this (modified / unsaved)
			// problem, then at least warn them of such. I am only doing this if it is not already prevented by one of the other
			// items, so as to reduce complexity for a message of such minor importance.
			if (nProblemCount == 0 && nDeletedProblemCount == 0 && nHistoryCount == 0 && m_nCurIDInUse == nID) {
				strMessage += "The current problem.\r\n";
			}

			if (!strMessage.IsEmpty()) {
				MessageBox(FormatString("This chronicity status cannot be deleted; it is in use by the following data:\r\n\r\n%s", strMessage), NULL, MB_ICONSTOP);
				return;
			} else {
				ExecuteParamSql("DELETE FROM EMRProblemChronicityT WHERE ID = {INT}", nID);
			}
		} else if (m_listID == EMRDATACODES_COMBO) {
			// (a.walling 2009-05-28 15:58) - PLID 34389
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			long nCount = 0;
			_RecordsetPtr prsHistory = CreateParamRecordset(
				"SELECT COUNT(*) AS InfoCount FROM EMRInfoT WHERE DataCodeID = {INT}",
				nID);
			if (!prsHistory->eof) {
				nCount = AdoFldLong(prsHistory, "InfoCount", 0);
			}

			if (nCount > 0) {
				MessageBox(FormatString("This code cannot be deleted; it is in use by %li items.", nCount), NULL, MB_ICONSTOP);
				return;
			} else {
				ExecuteParamSql("DELETE FROM EmrDataCodesT WHERE ID = {INT}", nID);
			}
		}
		else if(m_listID == LAB_ANATOMY_QUALIFIERS_COMBO) {
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			//TES 2/9/2010 - PLID 37223 - Need to also check for HotSpots
			long nQualID = VarLong(m_pList->GetValue(m_pList->CurSel,0),-1);
			// (c.haag 2011-06-17) - PLID 37307 - Make sure it's not currently in use
			if (m_nCurIDInUse == nQualID)
			{
				MessageBox("This anatomic location qualifier cannot be deleted; it is in use by the current lab.", NULL, MB_ICONSTOP);
				return;
			}
			else
			{
				// (a.walling 2014-08-18 14:33) - PLID 62681 - Laterality - handle database changes for referential integrity
				_RecordsetPtr rsCounts = CreateParamRecordset("SELECT (SELECT Count(*) FROM LabsT WHERE AnatomyQualifierID = {INT}) AS LabCount, "
					"(SELECT Count(*) FROM EmrImageHotSpotsT WHERE AnatomicQualifierID = {INT}) AS HotSpotCount, (SELECT DISTINCT Count(EmrActionID) FROM EmrActionAnatomicLocationQualifiersT WHERE AnatomicQualifierID = {INT}) AS ActionCount", 
					nQualID, nQualID, nQualID);
				long nLabCount = AdoFldLong(rsCounts, "LabCount", 0);
				long nHotSpotCount = AdoFldLong(rsCounts, "HotSpotCount", 0);
				long nActions = AdoFldLong(rsCounts, "ActionCount", 0);
				if(nLabCount > 0 || nHotSpotCount > 0)
				{
					MsgBox("You may not delete this anatomic location qualifier because there are %d Lab(s), %d EMR Image HotSpot(s), and %d Actions(s) associated with it.", 
						nLabCount, nHotSpotCount, nActions);
					return;
				}
				ExecuteSql("DELETE FROM AnatomyQualifiersT WHERE ID = %li",VarLong(m_pList->GetValue(m_pList->GetCurSel(),0)));
			}
		} else if (m_listID == LAB_LOINC) {
			// (a.walling 2010-01-18 13:10) - PLID 36955 - This is only used as a reference, no foreign keys or anything.
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			//TES 8/31/2011 - PLID 44538 - This is now linked to LabsToBeOrderedT.
			if (IDNO == MessageBox("Deleting this code will remove the link to any To Be Ordered entries with which it is associated. Do you want to continue?", NULL, MB_ICONQUESTION | MB_YESNO)) {
				return;
			}
			
			//TES 8/31/2011 - PLID 44538 - Also need to clear out any LabsToBeOrderedT records linked to this.
			ExecuteParamSql("UPDATE LabsToBeOrderedT SET LOINCID = NULL WHERE LOINCID = {INT}", nID);
			ExecuteParamSql("DELETE FROM LabLOINCT WHERE ID = {INT}", nID);
		//(j.deskurakis 2013-01-24) - PLID 53151 - modified
		} else if (m_listID == INTEGRATION_BILLTO) {
			// (f.dinatale 2010-07-08) - PLID 39527
			// (f.dinatale 2010-08-13) - PLID 39527 - Removed the double warning
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			_RecordsetPtr prscount = CreateParamRecordset("SELECT Count(*) AS Count FROM IntegrationsT WHERE IntegrationsBillTo = {INT}", nID);
			int nCount = AdoFldLong(prscount, "Count");
			//(j.deskurakis 2013-01-24) - PLID 53151 - modified
			if(nCount == 0){
				ExecuteParamSql("DELETE FROM IntegrationBillToT WHERE IntegrationBillToID = {INT}", nID);
			}else{
				MsgBox("You may not delete this Bill To type because there are %li HL7 Integrations associated with it.", nCount);
				return;
			}
		} else if (m_listID == INTEGRATION_LABTYPES) {
			// (f.dinatale 2010-07-08) - PLID 39527
			// (f.dinatale 2010-08-13) - PLID 39527 - Removed the double warning
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			_RecordsetPtr prscount = CreateParamRecordset("SELECT Count(*) AS Count FROM IntegrationsT WHERE IntegrationLabType = {INT}", nID);
			int nCount = AdoFldLong(prscount, "Count");

			if(nCount == 0){
				ExecuteParamSql("DELETE FROM IntegrationLabTypesT WHERE LabTypeID = {INT}", nID);
			}else{
				MsgBox("You may not delete this integration type because there are %li HL7 Integrations associated with it.", nCount);
				return;
			}
		
		// (d.lange 2011-01-03 09:23) - PLID 29065 - Added Biopsy Type
		} else if (m_listID == LAB_BIOPSY_TYPE) {
			long nBiopsyTypeID = VarLong(m_pList->GetValue(m_pList->CurSel, 0), -1);
			// (c.haag 2011-06-17) - PLID 37307 - Make sure it's not currently in use
			if (m_nCurIDInUse == nBiopsyTypeID)
			{
				MessageBox("This biopsy type cannot be deleted; it is in use by the current lab.", NULL, MB_ICONSTOP);
				return;
			}
			else
			{
				_RecordsetPtr rsCount = CreateParamRecordset("SELECT COUNT(*) AS Count FROM LabsT WHERE BiopsyTypeID = {INT}", nBiopsyTypeID);

				long nCount = AdoFldLong(rsCount, "Count", 0);

				if(nCount > 0) {
					MsgBox("You may not delete this biopsy type because there are %li Lab(s) associated with it.", nCount);
					return;
				}
				ExecuteParamSql("DELETE FROM LabBiopsyTypeT WHERE ID = {INT}", nBiopsyTypeID);
			}
		}
		else if(m_listID == GLASSES_ORDER_HISTORY_NOTES)
		{
			//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
			ExecuteParamSql("DELETE FROM GlassesOrderHistoryNotesT WHERE Note = {STRING}", VarString(m_pList->GetValue(m_pList->CurSel, 0)));
		}
		else if(m_listID == CONTACT_LENS_TYPE )
		{
			// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			_RecordsetPtr prscount = CreateParamRecordset("SELECT Count(*) AS Count FROM GlassesContactLensDataT WHERE GlassesContactLensTypeID = {INT}", nID);
			int nCount = AdoFldLong(prscount, "Count");
			if(nCount == 0){
				ExecuteParamSql("DELETE FROM GlassesContactLensTypeT WHERE ID = {INT}", nID);
			}
			else
			{
				MessageBox(FormatString("This contact lens type cannot be deleted; it is in use by %li item(s).", nCount), NULL, MB_ICONSTOP);
				return;
			}
		}
		else if(m_listID == CONTACT_LENS_MANUFACTURE )
		{
			// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			_RecordsetPtr prscount = CreateParamRecordset("SELECT Count(*) AS Count FROM GlassesContactLensDataT WHERE ContactLensManufacturerID = {INT}", nID);
			int nCount = AdoFldLong(prscount, "Count");
			if(nCount == 0){
				ExecuteParamSql("DELETE FROM ContactLensManufacturersT WHERE ID = {INT}", nID);
			}
			else
			{
				MessageBox(FormatString("This contact lens manufacturer cannot be deleted; it is in use by %li item(s).", nCount), NULL, MB_ICONSTOP);
				return;
		
			}
		}
		// (b.spivey, August 17, 2012) - PLID 52130 - Detecting duplicates and deleting Stamp category. 
		else if (m_listID == EMR_IMAGE_STAMP_CATEGORIES) {

			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));

			_RecordsetPtr prsCount = CreateParamRecordset("SELECT Count(*) AS Count FROM EMRImageStampsT WHERE CategoryID = {INT}", nID);
			int nCount = AdoFldLong(prsCount, "Count");
			if(nCount > 0 
				&& MessageBox(FormatString("This image stamp category is in use on %li stamps. Are you sure you want to delete it?", nCount), "", MB_ICONWARNING|MB_YESNO) == IDNO){
				return; 
			}

			ExecuteParamSql("UPDATE EmrImageStampsT SET CategoryID = NULL WHERE CategoryID = {INT} "
				"DELETE FROM EmrImageStampCategoryT WHERE ID = {INT}", nID, nID);
		}
		// (r.gonet 07/07/2014) - PLID 62520 - Delete a BillStatusNoteT record.
		else if (m_listID == BILL_STATUS_NOTES_COMBO) {
			long nID = VarLong(m_pList->GetValue(m_pList->CurSel, 0));
			
			ExecuteParamSql("DELETE FROM BillStatusNoteT WHERE ID = {INT}", nID);

			CClient::RefreshTable(NetUtils::BillStatusNoteT, nID);
		}

		m_pList->RemoveRow(m_pList->GetCurSel());
		EnableAppropriateButtons();

	}NxCatchAll("Error in deleting item.");
}

void CEditComboBox::OnRButtonDownEditList(long nRow, short nCol, long x, long y, long nFlags) 
{
	//(e.lally 2008-10-13) PLID 31665 - Added error handling
	try{
		m_pList->CurSel = nRow;

		if (nRow == -1 || m_bAllowDefault == FALSE){
			EnableAppropriateButtons();
			return;
		}
		
		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		if(!IsDefaultSelected())
			menPopup.InsertMenu(0, MF_BYPOSITION, ID_SET_DEFAULT, "Set As Default");
		else
			menPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_DEFAULT, "Remove Default");

		CPoint pt(x,y);
		CWnd* pWnd = GetDlgItem(IDC_EDIT_LIST);
		if (pWnd != NULL)
		{	pWnd->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else{
			HandleException(NULL, "An error ocurred while creating menu");
		}
		
		EnableAppropriateButtons();	
	}NxCatchAll("Error in CEditComboBox::OnRButtonDownEditList");
}

void CEditComboBox::OnEditingFinishingEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	//(e.lally 2008-10-13) PLID 31665 - Added error handling, made ReturnsRecords calls take a format string as the parameter
	try{
		if(*pbCommit) {

			// (c.haag 2009-05-07 12:14) - PLID 28550 - We don't want to check for duplicate or
			// empty text when editing optional columns
			BOOL bTestForDuplicateOrEmptyText = TRUE;

			// (a.walling 2010-01-18 13:11) - PLID 36955 - Duplicates are OK
			if (m_listID == LAB_LOINC) {
				bTestForDuplicateOrEmptyText = FALSE;				
			}

			//TES 11/6/2009 - PLID 36189 - Added LAB_ANATOMIC_LOCATION_COMBO
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			// (d.lange 2011-01-03 10:34) - PLID 29065 - Added Biopsy Type
			if (nCol == 2 && (m_listID == LAB_TO_BE_ORDERED_COMBO || m_listID == PROBLEM_CHRONICITY_COMBO || m_listID == LAB_ANATOMIC_LOCATION_COMBO || m_listID == LAB_ANATOMY_QUALIFIERS_COMBO
				|| m_listID == LAB_BIOPSY_TYPE)) {
				// (c.haag 2009-05-07 12:13) - PLID 28550 - If we get here, the user has finished 
				// editing an optional instructional message for the labs to be ordered dropdown. 
				// We don't need to test for duplicate or empty data.
				bTestForDuplicateOrEmptyText = FALSE;
			}

			//TES 7/12/2011 - PLID 44538 - LabsToBeOrderedT has a third column, which we do not want to check for duplicates.
			if(nCol == 3 && (m_listID == LAB_TO_BE_ORDERED_COMBO)) {
				bTestForDuplicateOrEmptyText = FALSE;
			}

			// (a.walling 2009-05-28 15:58) - PLID 34389
			if (m_listID == EMRDATACODES_COMBO && nCol != 1) {
				bTestForDuplicateOrEmptyText = FALSE;
			}

			// (d.singleton 2013-06-06 14:26) - PLID 56716 - allow duplicate and blank data for code column, no reason not to
			if(m_listID == IMMUNIZATION_ROUTES && nCol != 1) {
				bTestForDuplicateOrEmptyText = FALSE;
			}
			if(m_listID == IMMUNIZATION_SITES  && nCol != 1) {
				bTestForDuplicateOrEmptyText = FALSE;
			}
			if (m_listID == BILL_STATUS_NOTES_COMBO && nCol != -1) {
				// We'll handle this on our own.
				bTestForDuplicateOrEmptyText = FALSE;
			}

			if(bTestForDuplicateOrEmptyText && CString(varOldValue.bstrVal)!=strUserEntered) {
				long nDupRow = m_pList->FindByColumn(nCol,_bstr_t(strUserEntered),0,FALSE);
				if(nDupRow != -1) {
					if (m_listID == EMRDATACODES_COMBO) {
						// (j.gruber 2009-11-18 17:03) - PLID 35945 - check to see if it's a TOPS code
						// (j.gruber 2009-12-23 10:01) - PLID 35771 - or a report code
						IRowSettingsPtr pRow = m_pList->GetRow(nDupRow);
						if (pRow) {
							if (VarBool(pRow->GetValue(6), FALSE)) {
								//don't allow it
								MessageBox("This code already exists in the list.");
								*pbCommit = FALSE;
								return;
							}
							else {
								if (IDNO == MessageBox("This code already exists in the list; are you sure you want to create a duplicate?", NULL, MB_YESNO|MB_ICONQUESTION)) {
									*pbCommit = FALSE;
									return;
								}
							}
						}
					} else {
						AfxMessageBox("This data already exists in the list.");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			CString str = strUserEntered;
			str.TrimRight();
			if(bTestForDuplicateOrEmptyText && str=="") {
				AfxMessageBox("Please enter valid data.");
				*pbCommit = FALSE;
				return;
			}

			if(m_listID == PROCEDURE_GROUP_COMBO) {
				if(ReturnsRecords("SELECT ID FROM ProcedureT WHERE Name = '%s'", _Q(str))) {
					AfxMessageBox("There is already a procedure with this name.");
					*pbCommit = FALSE;
					return;
				}
			}

			if(m_listID == LWSUBJECT_COMBO) {
				if(CString(varOldValue.bstrVal) != CString(str)) {
					if(ReturnsRecords("SELECT Subject FROM LetterSubjectT WHERE Subject = '%s'", _Q(str))) {
						AfxMessageBox("There is already a subject with this description.");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (j.jones 2009-10-15 10:28) - PLID 34327 - split race editing into its own dialog
			/*
			if(m_listID == RACE_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM RaceT WHERE Name = '%s'", _Q(str))) {
						AfxMessageBox("There is already an ethnicity with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			*/

			if(m_listID == ANESTHESIA_COMBO) {
				if((VarString(varOldValue, "") == "Local" && CString(strUserEntered) != "Local") || 
					(VarString(varOldValue, "") == "Local With IV Sedation" && CString(strUserEntered) != "Local With IV Sedation") ||
					(VarString(varOldValue, "") == "General" && CString(strUserEntered) != "General") ) {
					if(IDYES != MsgBox(MB_YESNO, "This Anesthesia type is used in some pre-defined Letter Writing templates.  Are you sure you wish to change it?")) {
						*pbCommit = FALSE;
						return;
					}
				}
			}
			if(m_listID == LAB_ANATOMIC_LOCATION_COMBO)
			{
				//TES 11/6/2009 - PLID 36189 - Check if we're actually editing the description
				if(nCol == 1) {
					if(VarString(varOldValue, "") != str) {
						if(ReturnsRecords("SELECT ID FROM LabAnatomyT WHERE Description = '%s'", _Q(str))) {
							AfxMessageBox("There is already an anatomic location with this name.");
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}

			if(m_listID == LAB_CLINICAL_DIAGNOSIS_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM LabClinicalDiagnosisT WHERE Description like '%s'", _Q(str))) {
						AfxMessageBox("There is already an diagnosis with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			if(m_listID == LAB_DESCRIPTION_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM LabDiagnosisT WHERE Description like '%s'", _Q(str))) {
						AfxMessageBox("There is already an description with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (j.gruber 2007-05-07 10:23) - PLID 25931 - add support for POS Receipt List
			if(m_listID == POS_PRINTER_RECEIPT_TYPE_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM POSReceiptsT WHERE Name like '%s'", _Q(str))) {
						AfxMessageBox("There is already an receipt with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
			if(m_listID == SHIPPING_METHOD_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM ShippingMethodT WHERE Description like '%s'", _Q(str))) {
						AfxMessageBox("There is already a shipping method with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			// (j.gruber 2008-04-02 13:32) - PLID 29443 - Implementation EMR Status List
			if(m_listID == IMP_EMR_STATUS_LIST)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM EMRStatusT WHERE StatusName like '%s'", _Q(str))) {
						AfxMessageBox("There is already an EMR Status with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (j.gruber 2008-05-22 12:07) - PLID 30146 - EMR Type List
			if(m_listID == IMP_EMR_TYPE_LIST)
			{
				if(VarString(varOldValue, "") != str) {
					if(ReturnsRecords("SELECT ID FROM EMRTypeT WHERE TypeName like '%s'", _Q(str))) {
						AfxMessageBox("There is already an EMR Type with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (c.haag 2007-11-12 09:20) - PLID 28010 - Support for supplier return methods
			if (m_listID == INVEDITRETURN_METHOD_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM SupplierReturnMethodT WHERE Method = {STRING}",
						str);
					if (!prs->eof) {
						AfxMessageBox("There is already a return method with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			// (c.haag 2007-11-12 09:22) - PLID 28010 - Support for supplier return reasons
			if (m_listID == INVEDITRETURN_REASON_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM SupplierReturnReasonT WHERE Reason = {STRING}",
						str);
					if (!prs->eof) {
						AfxMessageBox("There is already a return reason with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			// (z.manning 2008-10-24 09:46) - PLID 31807
			if(m_listID == LAB_TO_BE_ORDERED_COMBO)
			{
				//TES 8/31/2011 - PLID 44538 - There is an extra column (LOINC dropdown) for this list now.
				if(nCol == 1) {
					if(VarString(varOldValue, "") != str) {
						_RecordsetPtr prs = CreateParamRecordset(
							"SELECT TOP 1 ID FROM LabsToBeOrderedT WHERE Description = {STRING}",
							str);
						if (!prs->eof) {
							AfxMessageBox("There is already a lab to be ordered entry with this name.");
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}

			// (j.jones 2009-01-23 09:30) - PLID 2822 - added Inventory Order Methods
			if(m_listID == INVEDITORDER_ORDERMETHOD_COMBO)
			{
				if(VarString(varOldValue, "") != str) {
					long nID = VarLong(m_pList->GetValue(nRow, 0));
					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderMethodsT WHERE Method = {STRING} AND ID <> {INT}", str, nID);
					if (!prs->eof) {
						AfxMessageBox("There is already an order method with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (a.walling 2009-05-04 08:35) - PLID 33751 - Problem Chronicity			
			if(m_listID == PROBLEM_CHRONICITY_COMBO)
			{
				{
					if( (nCol == 1 && VarString(varOldValue, "") != str) || 
						(nCol == 2 && ( (pvarNewValue == NULL) || (pvarNewValue->vt != varOldValue.vt) || (pvarNewValue->boolVal != varOldValue.boolVal) ) )
						) {
						long nID = VarLong(m_pList->GetValue(nRow, 0));

						long nProblemCount = 0;
						long nDeletedProblemCount = 0;
						long nHistoryCount = 0;
						_RecordsetPtr prsHistory = CreateParamRecordset("SELECT "
							"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 0 AND ChronicityID = {INT}) AS ProblemCount, "
							"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 1 AND ChronicityID = {INT}) AS DeletedProblemCount, "
							"(SELECT COUNT(*) FROM EMRProblemHistoryT WHERE ChronicityID = {INT}) AS HistoryCount",
							nID, nID, nID);
						if (!prsHistory->eof) {
							nProblemCount = AdoFldLong(prsHistory, "ProblemCount", 0);
							nDeletedProblemCount = AdoFldLong(prsHistory, "DeletedProblemCount", 0);
							nHistoryCount = AdoFldLong(prsHistory, "HistoryCount", 0); // includes the first/initial status
						}

						CString strMessage;

						if (nProblemCount != 0) {
							strMessage += FormatString("%li problems.\r\n", nProblemCount);
						}
						if (nDeletedProblemCount != 0) {
							strMessage += FormatString("%li deleted problems.\r\n", nDeletedProblemCount);
						}
						if (nHistoryCount != 0) {
							strMessage += FormatString("%li problem history records.\r\n", nHistoryCount);
						}						
						// (a.walling 2009-07-20 09:12) - PLID 33751 - If all are 0, but there is one chosen on this (modified / unsaved)
						// problem, then at least warn them of such. I am only doing this if it is not already flagged by one of the other
						// items, so as to reduce complexity for a message of such minor importance.
						if (nProblemCount == 0 && nDeletedProblemCount == 0 && nHistoryCount == 0 && m_nCurIDInUse == nID) {
							strMessage += "The current problem.\r\n";
						}

						if (nCol == 1) { // name
							if (!strMessage.IsEmpty()) {
								*pbCommit = FALSE;
								MessageBox(FormatString("This chronicity status cannot be modified; it is in use by the following data:\r\n\r\n%s", strMessage), NULL, MB_ICONSTOP);
								return;
							} else {
								_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRProblemChronicityT WHERE Name = {STRING} AND ID <> {INT}", str, nID);
								if (!prs->eof) {
									AfxMessageBox("There is already a chronicity status with this name.");
									*pbCommit = FALSE;
									return;
								}
							}
						} else if (nCol == 2) { // inactive
							if (varOldValue.vt == VT_BOOL && varOldValue.boolVal == VARIANT_FALSE) {
								if (!strMessage.IsEmpty()) {
									if (IDNO == MessageBox(FormatString("This chronicity status is in use by the following data:\r\n\r\n%s\r\nAre you sure you want to mark it inactive?", strMessage), NULL, MB_YESNO|MB_ICONSTOP)) {
										*pbCommit = FALSE;
										return;
									}
								}
							}
						}
					}
				}
			}

			// (a.walling 2009-05-28 15:59) - PLID 34389 - EmrDataCodesT
			if(m_listID == EMRDATACODES_COMBO)
			{
				if (
					(nCol == 1 && VarString(varOldValue, "") != str) ||
					(nCol == 2 && VarString(varOldValue, "") != str) ||
					(nCol == 3 && VarString(varOldValue, "") != str) ||
					(nCol == 4) ||
					(nCol == 5)
				) {
					long nID = VarLong(m_pList->GetValue(nRow, 0));

					long nCount = 0;
					_RecordsetPtr prsHistory = CreateParamRecordset(
						"SELECT COUNT(*) AS InfoCount FROM EMRInfoT WHERE DataCodeID = {INT}",
						nID);
					if (!prsHistory->eof) {
						nCount = AdoFldLong(prsHistory, "InfoCount", 0);
					}

					if (nCount > 0) {
						if (IDNO == MessageBox(FormatString("This code is in use by %li items; Are you sure you want to modify it?", nCount), NULL, MB_YESNO|MB_ICONSTOP)) {
							*pbCommit = FALSE;
							return;
						}
					}
				}

				// (j.gruber 2010-06-01 13:25) - PLID 38377 - check if it is a bold code, and if so, strongly recommend they not change it
				{					
					BOOL bIsSpecial = VarBool(m_pList->GetValue(nRow, 6), FALSE);
					if (bIsSpecial) {
						CString strCode = VarString(m_pList->GetValue(nRow, 1), "");

						if (strCode.Left(5) == "BOLD_") {
							//(a.wilson 2011-9-2) PLID 45307 - changed bold to export
							if (IDNO == AfxMessageBox("This is an export code, changing this code could affect how your data is exported.  Are you SURE you wish to change this code?", MB_YESNO)) {										
								*pbCommit = FALSE;
								return;
							}
						}
					}
				}
			}

			// (z.manning, 08/07/2007) - PLID 26959 - Warn if changing list elements that are in use.
			// (j.armen 2011-06-22 09:30) - PLID 11490 - Updated to use new CustomListDataT table.
			if((m_listID > CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS))
			{
				if(*pbCommit)
				{
					long nListElementID = VarLong(m_pList->GetValue(nRow, 0));
					_RecordsetPtr prs = CreateParamRecordset(
						"SELECT COUNT(*) AS Count FROM CustomListDataT WHERE FieldID = {INT} AND CustomListItemsID = {INT}",
						m_listID, nListElementID);

					long nCount = AdoFldLong(prs, "Count", 0);
					if(nCount > 0)
					{
						int nResult = MessageBox(FormatString(
							"Changing the name of this list element will change it on %li patient record(s).\r\n\r\n"
							"Are you sure you want to rename it?", nCount), NULL, MB_YESNO);
						if(nResult != IDYES) {
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}

			if(m_listID == LAB_ANATOMY_QUALIFIERS_COMBO)
			{
				//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
				if(nCol == 1) {
					if(VarString(varOldValue, "") != str) {
						if(ReturnsRecords("SELECT ID FROM AnatomyQualifiersT WHERE Name = '%s'", _Q(str))) {
							AfxMessageBox("There is already an anatomic location qualifier with this name.");
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}

			// (a.walling 2010-01-18 13:11) - PLID 36955 - Duplicates are OK, just no blanks
			if (m_listID == LAB_LOINC) {
				if (nCol == 1) {
					CString strEntered = strUserEntered;
					strEntered.TrimLeft();
					strEntered.TrimRight();
					if (strEntered.IsEmpty()) {
						MessageBox("You may not create a blank code");
						*pbCommit = FALSE;
						return;
					}
				}
			}
			//(j.deskurakis 2013-01-24) - PLID 53151 - modified
			// (f.dinatale 2010-07-09) - PLID 39527
			if (m_listID == INTEGRATION_BILLTO) {
				if (nCol == 1) {
					CString strEntered = strUserEntered;
					strEntered.TrimLeft();
					strEntered.TrimRight();
					if (strEntered.IsEmpty()) {
						MessageBox("You may not create a blank type");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (f.dinatale 2010-07-09) - PLID 39527
			if (m_listID == INTEGRATION_LABTYPES) {
				if (nCol == 1) {
					CString strEntered = strUserEntered;
					strEntered.TrimLeft();
					strEntered.TrimRight();
					if (strEntered.IsEmpty()) {
						MessageBox("You may not create a blank type");
						*pbCommit = FALSE;
						return;
					}
				}
			}

			// (d.lange 2011-01-03 09:37) - PLID 29065 - Added Biopsy Type
			if(m_listID == LAB_BIOPSY_TYPE) {
				if(nCol == 1) {
					CString strOldValue = VarString(varOldValue, "");
					strOldValue.TrimRight();
					if(strOldValue != str) {
						if(ReturnsRecords("SELECT ID FROM LabBiopsyTypeT WHERE Description = '%s'", _Q(str))) {
							AfxMessageBox("There is already a biopsy type with this name.");
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}

			if(m_listID == CONTACT_LENS_TYPE )
			{
				// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
				long  nCount =0;
				CString strOldValue = VarString(varOldValue, "");
				strOldValue.TrimRight();
				//s.dhole  skip if there is no change
				if(strOldValue.CompareNoCase(str) != 0) {
					_RecordsetPtr prsRec = CreateParamRecordset(
						"SELECT Count(*) AS Count FROM GlassesContactLensDataT WHERE GlassesContactLensTypeID = {INT}",
						VarLong(m_pList->GetValue(nRow, 0)));
						if (!prsRec->eof) {
							nCount = AdoFldLong(prsRec, "Count", 0);
						}
					if (nCount>0)
					{
						if (IDNO == MessageBox(FormatString("This contact lens type is in use by %li item(s); Are you sure you want to modify it?", nCount), "Edit?", MB_YESNO | MB_ICONQUESTION)) {
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}
			if(m_listID == CONTACT_LENS_MANUFACTURE )
			{
				// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
				long  nCount =0;
				CString strOldValue = VarString(varOldValue, "");
				strOldValue.TrimRight();
				//s.dhole  skip if there is no change
				if(strOldValue.CompareNoCase(str) != 0) {
					_RecordsetPtr prsRec = CreateParamRecordset(
						"SELECT Count(*) AS Count FROM GlassesContactLensDataT WHERE ContactLensManufacturerID = {INT}",
						VarLong(m_pList->GetValue(nRow, 0)));
						if (!prsRec->eof) {
							nCount = AdoFldLong(prsRec, "Count", 0);
						} 
					if (nCount>0)
					{
						if (IDNO == MessageBox(FormatString("This manufacturer is in use by %li item(s); Are you sure you want to modify it?", nCount), "Edit?", MB_YESNO | MB_ICONQUESTION)) {
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}
			// (b.spivey, August 17, 2012) - PLID 52130 - Detecting duplicates when adding. 
			if (m_listID == EMR_IMAGE_STAMP_CATEGORIES)
			{
				CString strOldValue = VarString(varOldValue, "");
				strOldValue.TrimRight();
				// Check for no change, check if in use. 
				if(strOldValue.CompareNoCase(str) != 0 
					&& ReturnsRecordsParam("SELECT ID FROM EmrImageStampCategoryT WHERE Description LIKE {STRING}", str)) {
					AfxMessageBox("An image stamp category with this name already exists.");
					*pbCommit = FALSE;
					return; 
				}
			}
			// (r.gonet 07/07/2014) - PLID 62520 - Prevent the user from making a blank BillStatusNoteT record and prevent them from making a duplicate one.
			if (m_listID == BILL_STATUS_NOTES_COMBO) {
				if (nCol == 1) {
					long nID = VarLong(m_pList->GetValue(nRow, 0));
					CString strOldValue = VarString(varOldValue, "");
					strOldValue.Trim();
					if (strOldValue != str) {
						CString strEntered = str;
						strEntered.TrimLeft();
						strEntered.TrimRight();
						if (strEntered.IsEmpty()) {
							AfxMessageBox("A bill status note cannot be blank.");
							*pbCommit = FALSE;
							return;
						}
						// The collate makes it a case sensitive search.
						if (ReturnsRecordsParam("SELECT ID FROM BillStatusNoteT WHERE TextData LIKE {STRING} COLLATE Latin1_General_BIN AND ID <> {INT}", str, nID)) {
							AfxMessageBox("There is already a bill status note with this text.");
							*pbCommit = FALSE;
							return;
						}
					}
				}
			}
			//(e.lally 2007-07-09) PLID 26590 - Depreciating payment credit card combo functionality for its own edit dlg
			/*
			if(m_listID == PAYMENT_CARD_COMBO && *pbCommit){
				//check to make sure the card name isn't duplicated
				CString strNewVal = VarString(pvarNewValue);
				strNewVal.TrimLeft();
				strNewVal.TrimRight();
				long pCurRowEnum = m_pList->GetFirstRowEnum();
				while(pCurRowEnum != 0){
					IRowSettingsPtr pRow;
					{
						IDispatch *lpDisp;
						m_pList->GetNextRowEnum(&pCurRowEnum, &lpDisp);
						pRow = lpDisp;
						lpDisp->Release();
						lpDisp = NULL;
					}

					ASSERT(pRow != NULL);
					_variant_t var = pRow->GetValue(0);
					
					CString strName;
					if(var.vt != VT_EMPTY){
						strName = VarString(var);
								
						// see if the new value is different from the old, if so make sure the values aren't the same
						if(strNewVal.CompareNoCase(VarString(varOldValue)) != 0 && strNewVal.CompareNoCase(strName) == 0)
						{
							AfxMessageBox("This card type already exists in the list.");
							*pbCommit = FALSE;
							return;
						}
					}
				}
				if(pbCommit){
					// we are going to commit these changes so reflect them in the list
					_variant_t varNewValue;
					varNewValue = _bstr_t((LPCTSTR)strNewVal);
					(*pvarNewValue) = varNewValue.Detach();
				}
			}
			*/
		}
	}NxCatchAllCall("Error while saving changes.",
		*pbCommit = FALSE;
		return;
	);
}

void CEditComboBox::OnEditingFinishedEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try{
		if(bCommit) {
			CString strValue = varNewValue.vt == VT_BSTR ? VarString(varNewValue) : "";
			CString strOldValue = varOldValue.vt == VT_BSTR ? VarString(varOldValue) : "";
			strValue.TrimLeft();
			strValue.TrimRight();
			if(m_listID == QUOTE_NOTES_COMBO) {
				if(strValue.GetLength()>255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE QuoteNotesT SET Note = '%s' WHERE Note = '%s'",_Q(strValue),_Q(CString(varOldValue.bstrVal)));
			}
			else if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
				if(strValue.GetLength()>255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));

					CString strOldDefault = GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE);
					if(strOldDefault != "" && strOldDefault == CString(varOldValue.bstrVal)) {
						//we changed the default
						SetRemotePropertyText("DefaultPayDesc",strValue,0,"<None>");
					}
				}
				ExecuteSql("UPDATE PaymentExtraDescT SET ExtraDescription = '%s' WHERE ExtraDescription = '%s'",_Q(strValue),_Q(CString(varOldValue.bstrVal)));
			}
			// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
			/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {
				if(strValue.GetLength()>150) {
					AfxMessageBox("You entered a value greater than the maximum length (150). The data will be truncated.");
					strValue = strValue.Left(150);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE PaymentGroupsT SET GroupName = '%s' WHERE ID = %li",_Q(strValue),m_pList->GetValue(nRow,0).lVal);
				CClient::RefreshTable(NetUtils::PaymentGroupsT);
			}*/
			//(e.lally 2007-07-09) PLID 26590 - Depreciating payment credit card combo functionality for its own edit dlg
			/*
			else if(m_listID == PAYMENT_CARD_COMBO) {
				if(strValue.GetLength()>50) {
					AfxMessageBox("You entered a value greater than the maximum length (50). The data will be truncated.");
					strValue = strValue.Left(50);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				CString str;
				str.Format("UPDATE CreditCardNamesT SET CardName = '%s' WHERE CardName = '%s'",_Q(strValue),_Q(CString(varOldValue.bstrVal)));
				ExecuteSql("%s",str);
			}*/
			// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
			/*
			else if(m_listID == PATIENT_TYPE_COMBO) {
				if(strValue.GetLength()>100) {
					AfxMessageBox("You entered a value greater than the maximum length (100). The data will be truncated.");
					strValue = strValue.Left(100);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE GroupTypes SET GroupName = '%s' WHERE TypeIndex = %li",_Q(strValue),m_pList->GetValue(nRow,0).lVal);
			}
			*/
			else if(m_listID == STATEMENT_NOTE_COMBO) {
				if(strValue.GetLength()>2000) {
					AfxMessageBox("You entered a value greater than the maximum length (2000). The data will be truncated.");
					strValue = strValue.Left(2000);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE StatementNotesT SET Note = '%s' WHERE Note = '%s'",_Q(strValue),_Q(CString(varOldValue.bstrVal)));
			}
			else if(m_listID == PROCEDURE_GROUP_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE ProcedureGroupsT SET Name = '%s' WHERE ID = %li", _Q(strValue),VarLong(m_pList->GetValue(nRow,0)));
			}
			else if(m_listID == MEDSCHEDEVENT_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE MedSchedEventsT SET Name = '%s' WHERE ID = %li", _Q(strValue),VarLong(m_pList->GetValue(nRow,0)));
			}
			else if(m_listID == ANESTHESIA_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE AnesthesiaTypes SET Anesthesia = '%s' WHERE Anesthesia = '%s'", _Q(strValue), _Q(VarString(varOldValue, "")));
			}
			else if(m_listID == LWSUBJECT_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE LetterSubjectT SET Subject = '%s' WHERE Subject = '%s'", _Q(strValue), _Q(VarString(varOldValue, "")));
			}
			// (j.jones 2009-10-15 10:28) - PLID 34327 - split race editing into its own dialog
			/*
			else if(m_listID == RACE_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE RaceT SET Name = '%s' WHERE ID = %d", _Q(strValue), m_pList->GetValue(nRow,0).lVal);
			}
			*/
			//m.hancock - 7-11-2005 - PLID 16755 - Changed to allow room for Outcomes Custom Lists
			//else if(m_listID > CUSTOM_LISTS) {	//custom list
			else if((m_listID > CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS)) {	//custom list
				if(strValue.GetLength()>100) {
					AfxMessageBox("You entered a value greater than the maximum length (100). The data will be truncated.");
					strValue = strValue.Left(100);
					//put it in the row
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE CustomListItemsT SET Text = '%s' WHERE ID = %li AND CustomFieldID = %li",_Q(strValue),m_pList->GetValue(nRow,0).lVal,m_listID);
			}
			//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
			else if((m_listID >= OUTCOMES_CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS_END)) {
				if(strValue.GetLength()>100) {
					AfxMessageBox("You entered a value greater than the maximum length (100). The data will be truncated.");
					strValue = strValue.Left(100);
					//put it in the row
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE OutcomesListItemsT SET Text = '%s' WHERE ID = %li AND CustomFieldID = %li",_Q(strValue),m_pList->GetValue(nRow,0).lVal,m_listID);
			}
			else if(m_listID == CANCEL_REASON_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				if(VarLong(m_pList->GetValue(nRow,0)) < 0)
					AfxMessageBox("Invalid Cancel Reason ID"); 
				else
					ExecuteSql("UPDATE AptCancelReasonT SET Description = '%s' WHERE ID = %d", _Q(strValue), m_pList->GetValue(nRow,0).lVal);
			}
			else if(m_listID == NO_SHOW_REASON_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE AptNoShowReasonT SET Description = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			else if(m_listID == LAB_ANATOMIC_LOCATION_COMBO) {
				if(nCol == 1) {
					if(strValue.GetLength() > 100) {
						AfxMessageBox("You entered a value greater then the maximum length (100).  The data will be truncated.");
						strValue = strValue.Left(100);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteSql("UPDATE LabAnatomyT SET Description = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
				}
				else if(nCol == 2) {
					//TES 11/6/2009 - PLID 36189 - We now have an Inactive column
					ExecuteParamSql("UPDATE LabAnatomyT SET Inactive = {BIT} WHERE ID = {INT}", VarBool(varNewValue, FALSE), VarLong(m_pList->GetValue(nRow, 0)));
				}
			}
			else if(m_listID == LAB_CLINICAL_DIAGNOSIS_COMBO) {
				ExecuteSql("UPDATE LabClinicalDiagnosisT SET Description = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			else if(m_listID == LAB_DESCRIPTION_COMBO) {
				ExecuteSql("UPDATE LabDiagnosisT SET Description = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (j.gruber 2007-05-07 10:25) - PLID 25931 - add support for POS receipt names
			else if(m_listID == POS_PRINTER_RECEIPT_TYPE_COMBO) {
				if(strValue.GetLength() > 50) {
					AfxMessageBox("You entered a value greater then the maximum length (50).  The data will be truncated.");
					strValue = strValue.Left(50);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE POSReceiptsT SET Name = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
			else if(m_listID == SHIPPING_METHOD_COMBO) {
				if(strValue.GetLength() > 500) {
					AfxMessageBox("You entered a value greater then the maximum length (500).  The data will be truncated.");
					strValue = strValue.Left(500);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE ShippingMethodT SET Description = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			else if(m_listID == IMP_EMR_STATUS_LIST) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE EMRStatusT SET StatusName = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (j.gruber 2008-05-22 12:10) - PLID 30146 - Implementation EMR Type List
			else if(m_listID == IMP_EMR_TYPE_LIST) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE EMRTypeT SET TypeName = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (c.haag 2007-11-12 09:23) - PLID 28010 - Support for supplier return methods
			else if (m_listID == INVEDITRETURN_METHOD_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE SupplierReturnMethodT SET Method = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (c.haag 2007-11-12 09:24) - PLID 28010 - Support for supplier return reasons
			else if (m_listID == INVEDITRETURN_REASON_COMBO) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteSql("UPDATE SupplierReturnReasonT SET Reason = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (z.manning 2008-10-24 09:51) - PLID 31807
			else if (m_listID == LAB_TO_BE_ORDERED_COMBO)
			{
				// (c.haag 2009-05-07 12:03) - PLID 28550 - Support for modifying DisplayInstructions
				if (1 == nCol) {
					if(strValue.GetLength() > 1000) {
						MessageBox("You entered a value greater then the maximum length (1000).  The data will be truncated.");
						strValue = strValue.Left(1000);
						m_pList->PutValue(nRow, nCol, _bstr_t(strValue));
					}
					ExecuteParamSql("UPDATE LabsToBeOrderedT SET Description = {STRING} WHERE ID = {INT}"
						, strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
				else if (2 == nCol) {
					if(strValue.GetLength() > 1000) {
						MessageBox("You entered a value greater then the maximum length (1000).  The data will be truncated.");
						strValue = strValue.Left(1000);
						m_pList->PutValue(nRow, nCol, _bstr_t(strValue));
					}
					ExecuteParamSql("UPDATE LabsToBeOrderedT SET DisplayInstructions = {STRING} WHERE ID = {INT}"
						, strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
				//TES 7/12/2011 - PLID 44538 - Added support for modifying associated LOINC codes
				else if (3 == nCol) {
					ExecuteParamSql("UPDATE LabsToBeOrderedT SET LOINCID = {VT_I4} WHERE ID = {INT}",
						varNewValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
			}
			// (j.jones 2009-01-23 09:30) - PLID 2822 - added Inventory Order Methods
			else if(m_listID == INVEDITORDER_ORDERMETHOD_COMBO)
			{
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteParamSql("UPDATE OrderMethodsT SET Method = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (a.walling 2009-05-04 08:37) - PLID 33751 - Problem Chronicity
			else if(m_listID == PROBLEM_CHRONICITY_COMBO)
			{
				BOOL bInactive = FALSE;
				if (nCol == 1) {
					if(strValue.GetLength() > 255) {
						AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
						strValue = strValue.Left(255);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					
					ExecuteParamSql("UPDATE EMRProblemChronicityT SET Name = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				} else if (nCol == 2) {
					bInactive = VarBool(varNewValue, FALSE);
					
					ExecuteParamSql("UPDATE EMRProblemChronicityT SET Inactive = {BIT} WHERE ID = {INT}", bInactive, VarLong(m_pList->GetValue(nRow, 0)));
				}
			}	
			// (a.walling 2009-05-28 16:04) - PLID 34389
			else if(m_listID == EMRDATACODES_COMBO)
			{
				long nTableID = VarLong(m_pList->GetValue(nRow, 0));

				switch(nCol) {
					case 1: // code						
						if(strValue.GetLength() > 25) {
							AfxMessageBox("You entered a value greater then the maximum length (25).  The data will be truncated.");
							strValue = strValue.Left(25);
							m_pList->PutValue(nRow, nCol, _variant_t(strValue));
						}						
						
						ExecuteParamSql("UPDATE EmrDataCodesT SET Code = {STRING} WHERE ID = {INT}", strValue, nTableID);
						break;
					case 2: // description				
						if(strValue.GetLength() > 255) {
							AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
							strValue = strValue.Left(255);
							m_pList->PutValue(nRow, nCol, _variant_t(strValue));
						}

						ExecuteParamSql("UPDATE EmrDataCodesT SET Description = {STRING} WHERE ID = {INT}", strValue, nTableID);
						break;
					case 3: // default unit				
						if(strValue.GetLength() > 10) {
							AfxMessageBox("You entered a value greater then the maximum length (10).  The data will be truncated.");
							strValue = strValue.Left(10);
							m_pList->PutValue(nRow, nCol, _variant_t(strValue));
						}
						
						ExecuteParamSql("UPDATE EmrDataCodesT SET DefaultUnit = {STRING} WHERE ID = {INT}", strValue, nTableID);
						break;
					case 4: // vital
						{
							BOOL bVital = VarBool(varNewValue, FALSE);
							ExecuteParamSql("UPDATE EmrDataCodesT SET Vital = {BIT} WHERE ID = {INT}", bVital, nTableID);
						}
						break;
					case 5: // inactive
						{
							BOOL bInactive = VarBool(varNewValue, FALSE);
							ExecuteParamSql("UPDATE EmrDataCodesT SET Inactive = {BIT} WHERE ID = {INT}", bInactive, nTableID);
						}
						break;
				}
			}
			// (j.gruber 2009-05-08 09:46) - PLID 34202 - Provider Types
			else if(m_listID == PROVIDER_TYPES) {
				if(strValue.GetLength() > 255) {
					AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}

				if(VarString(varOldValue, "") != strValue) {
					long nID = VarLong(m_pList->GetValue(nRow, 0));
					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM ProviderTypesT WHERE Description = {STRING} AND ID <> {INT}", strValue, nID);
					if (!prs->eof) {
						AfxMessageBox("There is already an provider type with this name.");					
						m_pList->PutValue(nRow, nCol, varOldValue);
						return;
					}				
				}

				ExecuteSql("UPDATE ProviderTypesT SET Description = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (d.thompson 2009-05-14) - PLID 34232 - Immunization Routes			
			else if(m_listID == IMMUNIZATION_ROUTES) {
				if(nCol == 1) {
					if(strValue.GetLength() > 100) {
						AfxMessageBox("You entered a value greater then the maximum length (100).  The data will be truncated.");
						strValue = strValue.Left(100);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteParamSql("UPDATE ImmunizationRoutesT SET Name = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
				// (d.singleton 2013-05-15 16:34) - PLID 56716 - edit the immunizations route and site CEditComboBox settings to include the new "Code" row
				else if(nCol == 2) {
					if(strValue.GetLength() > 50) {
						AfxMessageBox("You entered a value greater then the maximum length (50). The data will be truncated.");
						strValue = strValue.Left(50);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteParamSql("UPDATE ImmunizationRoutesT SET Code = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
			}
			// (d.thompson 2009-05-14) - PLID 34232 - Immunization Sites			
			else if(m_listID == IMMUNIZATION_SITES) {
				if(nCol == 1) {
					if(strValue.GetLength() > 100) {
						AfxMessageBox("You entered a value greater then the maximum length (100).  The data will be truncated.");
						strValue = strValue.Left(100);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteParamSql("UPDATE ImmunizationSitesT SET Name = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
				// (d.singleton 2013-05-15 16:34) - PLID 56716 - edit the immunizations route and site CEditComboBox settings to include the new "Code" row
				else if(nCol == 2) {
					if(strValue.GetLength() > 50) {
						AfxMessageBox("You entered a value greater then the maximum length (50). The data will be truncated.");
						strValue = strValue.Left(50);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteParamSql("UPDATE ImmunizationSitesT SET Code = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
			}
			else if(m_listID == LAB_ANATOMY_QUALIFIERS_COMBO) {
				//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
				if(nCol == 1) {
					if(strValue.GetLength() > 100) {
						AfxMessageBox("You entered a value greater then the maximum length (100).  The data will be truncated.");
						strValue = strValue.Left(100);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteSql("UPDATE AnatomyQualifiersT SET Name = '%s' WHERE ID = %li", _Q(strValue), VarLong(m_pList->GetValue(nRow, 0)));
				}
				else if(nCol == 2) {
					ExecuteParamSql("UPDATE AnatomyQualifiersT SET Inactive = {BIT} WHERE ID = {INT}", VarBool(varNewValue, FALSE), VarLong(m_pList->GetValue(nRow, 0)));
				}
			}
			// (a.walling 2010-01-18 13:15) - PLID 36955
			else if(m_listID == LAB_LOINC)
			{
				if (nCol == 1) {
					if(strValue.GetLength() > 20) {
						AfxMessageBox("You entered a value greater then the maximum length (20).  The data will be truncated.");
						strValue = strValue.Left(20);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					
					ExecuteParamSql("UPDATE LabLOINCT SET Code = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				} else if (nCol == 2 || nCol == 3) {
					if(strValue.GetLength() > 255) {
						AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
						strValue = strValue.Left(255);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					
					if (nCol == 2) {
						ExecuteParamSql("UPDATE LabLOINCT SET ShortName = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
					} else if (nCol == 3) {
						ExecuteParamSql("UPDATE LabLOINCT SET LongName = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
					}
				}
			}
			//(j.deskurakis 2013-01-24) - PLID 53151 - modified
			else if(m_listID == INTEGRATION_BILLTO)
			{
				// (f.dinatale 2010-07-09) - PLID 39527
				if (nCol == 1) {
					if(strValue.GetLength() > 255) {
						AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
						strValue = strValue.Left(255);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					
					ExecuteParamSql("UPDATE IntegrationBillToT SET Description = {STRING} WHERE IntegrationBillToID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
			}	

			else if(m_listID == INTEGRATION_LABTYPES)
			{
				// (f.dinatale 2010-07-09) - PLID 39556
				if (nCol == 1) {
					if(strValue.GetLength() > 255) {
						AfxMessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.");
						strValue = strValue.Left(255);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					
					ExecuteParamSql("UPDATE IntegrationLabTypesT SET Description = {STRING} WHERE LabTypeID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
				}
			}

			// (d.lange 2011-01-03 09:40) - PLID 29065 - Added Biopsy Type
			else if(m_listID == LAB_BIOPSY_TYPE) {
				if(nCol == 1) {
					if(strValue.GetLength() > 100) {
						AfxMessageBox("You entered a value greater then the maximum length (100).  The data will be truncated.");
						strValue = strValue.Left(100);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					ExecuteParamSql("UPDATE LabBiopsyTypeT SET Description = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
					// (d.lange 2011-01-24 16:23) - PLID 29065 - Audit when the name of the biopsy type is changed
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiPatientLabBiopsyType, VarLong(m_pList->GetValue(nRow, 0)), strOldValue, strValue, aepMedium, aetChanged);
				}
				else if(nCol == 2) {
					ExecuteParamSql("UPDATE LabBiopsyTypeT SET Inactive = {BIT} WHERE ID = {INT}", VarBool(varNewValue, FALSE), VarLong(m_pList->GetValue(nRow, 0)));
				}
			}
			else if(m_listID == GLASSES_ORDER_HISTORY_NOTES)
			{
				//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
				if(strValue.GetLength()>255) {
					AfxMessageBox("You entered a value greater than the maximum length (255). The data will be truncated.");
					strValue = strValue.Left(255);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteParamSql("UPDATE GlassesOrderHistoryNotesT SET Note = {STRING} WHERE Note = {STRING}", strValue, VarString(varOldValue));
			}
			else if(m_listID == CONTACT_LENS_TYPE )
			{
				// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
					if(strValue.GetLength()>50) {
					AfxMessageBox("You entered a value greater than the maximum length (50). The data will be truncated.");
					strValue = strValue.Left(50);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteParamSql("UPDATE GlassesContactLensTypeT SET ContactLensType = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
			}
			else if(m_listID == CONTACT_LENS_MANUFACTURE )
			{
				// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
				if(strValue.GetLength()>50) {
					AfxMessageBox("You entered a value greater than the maximum length (50). The data will be truncated.");
					strValue = strValue.Left(50);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue));
				}
				ExecuteParamSql("UPDATE ContactLensManufacturersT SET name = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0)));
			}
			// (b.spivey, August 17, 2012) - PLID  - 
			else if (m_listID == EMR_IMAGE_STAMP_CATEGORIES) {
				if (strValue.GetLength() > 500){
					AfxMessageBox("You entered a value greater than the maximum length (500). The data will be truncated."); 
					strValue = strValue.Left(500);
					m_pList->PutValue(nRow, nCol, _variant_t(strValue)); 
				}
				ExecuteParamSql("UPDATE EmrImageStampCategoryT SET Description = {STRING} WHERE ID = {INT}", strValue, VarLong(m_pList->GetValue(nRow, 0))); 
			}
			// (r.gonet 07/07/2014) - PLID 62520 - Commit any edits to the BillStatusNoteT record.
			else if (m_listID == BILL_STATUS_NOTES_COMBO) {
				if (nCol == 1) {
					if (strValue.GetLength() > 3000) {
						AfxMessageBox("You have entered a value greater than the maximum length (3000). The data will be truncated.");
						strValue = strValue.Left(3000);
						m_pList->PutValue(nRow, nCol, _variant_t(strValue));
					}
					long nID = VarLong(m_pList->GetValue(nRow, 0));
					ExecuteParamSql("UPDATE BillStatusNoteT SET TextData = {STRING} WHERE ID = {INT}", strValue, nID);

					CClient::RefreshTable(NetUtils::BillStatusNoteT, nID);
				}
			}
			
		}
	}NxCatchAll("Error in changing value.");
}

BOOL CEditComboBox::OnInitDialog() 
{
	try{
		// (c.haag 2008-04-22 17:03) - PLID 29751 - NxIconify the buttons
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnCancel.AutoSet(NXB_CANCEL); // (a.walling 2010-01-18 13:01) - PLID 36955

		SetWindowText(m_strTitle);
		m_pList = BindNxDataListCtrl(this,IDC_EDIT_LIST,GetRemoteData(),false);
		m_pList->IsComboBox = FALSE;
		if(m_pFromListV1 != NULL && m_pFromListV2 != NULL)
			AfxThrowNxException("An error occurred while initializing the dialog. At least two versions of the list contain values.");
		else if(m_pFromListV1 != NULL){
			m_pList->PutFromClause(m_pFromListV1->GetFromClause());
			m_pList->PutWhereClause(m_pFromListV1->GetWhereClause());
		}
		else if(m_pFromListV2 != NULL){
			m_pList->PutFromClause(m_pFromListV2->GetFromClause());
			m_pList->PutWhereClause(m_pFromListV2->GetWhereClause());
		}
		else {
			//TES 11/6/2009 - PLID 36189 - Added LAB_ANATOMIC_LOCATION_COMBO
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			
			if (m_listID != LAB_DESCRIPTION_COMBO && m_listID != LAB_CLINICAL_DIAGNOSIS_COMBO &&
				// (c.haag 2007-11-12 10:18) - PLID 28010 - Support for supplier return combo configurations
				m_listID != INVEDITRETURN_METHOD_COMBO && m_listID != INVEDITRETURN_REASON_COMBO &&
				m_listID != PROBLEM_CHRONICITY_COMBO && m_listID != PROVIDER_TYPES && m_listID != EMRDATACODES_COMBO &&
				m_listID != LAB_ANATOMIC_LOCATION_COMBO && m_listID != LAB_ANATOMY_QUALIFIERS_COMBO &&
				// (a.walling 2010-01-18 13:16) - PLID 36955
				m_listID != LAB_LOINC
				// (j.jones 2010-06-25 16:20) - PLID 39185 - this list is no longer provided to us
				&& m_listID != LAB_TO_BE_ORDERED_COMBO
				//(j.deskurakis 2013-01-24) - PLID 53151 - modified
				// (f.dinatale 2010-07-08) - PLID 39556 - Added IntegrationTypesT, IntegrationLabTypesT, and IntegrationLinksT
				&& m_listID != INTEGRATION_BILLTO && m_listID != INTEGRATION_LABTYPES 
				// (d.lange 2011-01-03 09:43) - PLID 29065 - Added Biopsy Type
				&& m_listID != LAB_BIOPSY_TYPE
				//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
				&& m_listID != GLASSES_ORDER_HISTORY_NOTES
				// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
				&& m_listID != CONTACT_LENS_TYPE
				// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
				&& m_listID != CONTACT_LENS_MANUFACTURE 
				// (b.spivey, August 17, 2012) - PLID 52130 - Added Image Stamp Categories. 
				&& m_listID != EMR_IMAGE_STAMP_CATEGORIES
				// (r.gonet 07/07/2014) - PLID 62520 - BillStatusNoteT
				&& m_listID != BILL_STATUS_NOTES_COMBO
				
				) {
				AfxThrowNxException("An error occurred while initializing the dialog. None of the versions of the list were set.");
			}
		}

		// (a.walling 2015-01-06 15:35) - PLID 64528 - CEditComboBox override where and from clauses
		if (!m_strOverrideFromClause.IsEmpty()) {			
			m_pList->PutFromClause((const char*)m_strOverrideFromClause);
		}
		if (!m_strOverrideWhereClause.IsEmpty()) {			
			m_pList->PutWhereClause((const char*)m_strOverrideWhereClause);
		}

		/*m_pList->HavingClause = m_pFromListV1->HavingClause;
		m_pList->GroupByClause = m_pFromListV1->GroupByClause;*/
		if(m_listID == QUOTE_NOTES_COMBO) { //quote notes
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
		}
		else if(m_listID == PAYMENT_DESCRIPTION_COMBO) {	//payment description
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
			m_bAllowDefault = TRUE;
		}
		// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
		/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {	//payment category
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_bAllowDefault = TRUE;
		}*/
		//(e.lally 2007-07-09) PLID 26590 - Depreciating payment credit card combo functionality for its own edit dlg
		/*
		else if(m_listID == PAYMENT_CARD_COMBO) {	//payment credit cards
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
		}*/
		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
		/*
		else if(m_listID == PATIENT_TYPE_COMBO) {   //patient type from general 2 (cosmetic, reconstructive, etc.)
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_bAllowDefault = TRUE;
		}	
		*/
		else if(m_listID == STATEMENT_NOTE_COMBO) { //quote notes
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
		}
		else if(m_listID == PROCEDURE_GROUP_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		else if(m_listID == MEDSCHEDEVENT_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		else if(m_listID == ANESTHESIA_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
		}
		else if(m_listID == LWSUBJECT_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
		}
		// (j.jones 2009-10-15 10:28) - PLID 34327 - split race editing into its own dialog
		/*
		else if(m_listID == RACE_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		*/
		else if(m_listID == CANCEL_REASON_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(0)->DataType = VT_I4;
		}
		else if(m_listID == NO_SHOW_REASON_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		//m.hancock - 7-11-2005 - PLID 16755 - Changed to allow room for Outcomes Custom Lists
		//else if(m_listID > CUSTOM_LISTS) {  //custom list
		else if((m_listID > CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS)) {  //custom list
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		else if((m_listID >= OUTCOMES_CUSTOM_LISTS) && (m_listID < OUTCOMES_CUSTOM_LISTS_END)) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		else if(m_listID == LAB_ANATOMIC_LOCATION_COMBO) {
			//TES 11/6/2009 - PLID 36189 - Specify the FROM clause and column names/titles, we're not pulling from the list any more.
			m_pList->FromClause = "LabAnatomyT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			//TES 11/6/2009 - PLID 36189 - Added Inactive
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Inactive", "Inactive", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftBoolCheckbox;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BOOL;
		}
		else if(m_listID == LAB_CLINICAL_DIAGNOSIS_COMBO) {
			
			if (m_pFromListV1 == NULL && m_pFromListV2 == NULL) {
				//we are coming from admin module
				m_pList->FromClause = "LabClinicalDiagnosisT";
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			}
			else {
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			}
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			//m_pList->GetColumn(1)->PutSortPriority(0);
			//m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GridVisible = true;
		}
		else if(m_listID == LAB_DESCRIPTION_COMBO) {
			if (m_pFromListV1 == NULL && m_pFromListV2 == NULL) {
				//we are coming from admin module
				m_pList->FromClause = "LabDiagnosisT";
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			}
			else {
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
				IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			}
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			//m_pList->GetColumn(1)->PutSortPriority(0);
			//m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GridVisible = true;
		}
		// (j.gruber 2007-05-07 10:27) - PLID 25931 - add support for POS receipt lists
		else if(m_listID == POS_PRINTER_RECEIPT_TYPE_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (j.gruber 2008-02-27 08:37) - PLID 29100 - added shipping method
		else if(m_listID == SHIPPING_METHOD_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (j.gruber 2008-04-02 13:35) - PLID 29443 - Implementation EMR Status 
		else if(m_listID == IMP_EMR_STATUS_LIST) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (j.gruber 2008-05-22 12:11) - PLID 30146 - EMR Type List
		else if(m_listID == IMP_EMR_TYPE_LIST) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (c.haag 2007-11-12 09:25) - PLID 28010 - Added support for return methods
		else if (m_listID == INVEDITRETURN_METHOD_COMBO) {
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, GetFieldName(1), GetColumnTitle(1), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (c.haag 2007-11-12 09:26) - PLID 28010 - Added support for return reasons
		else if (m_listID == INVEDITRETURN_REASON_COMBO) {
			m_pList->FromClause = "SupplierReturnReasonT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Reason", "Reason", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (z.manning 2008-10-24 10:05) - PLID 31807
		else if (m_listID == LAB_TO_BE_ORDERED_COMBO)
		{
			m_pList->PutFromClause("LabsToBeOrderedT");
			// (j.jones 2010-06-25 15:06) - PLID 39185 - we can't necessarily use the where clause from the source we edited from
			m_pList->PutWhereClause("");
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->PutFieldType(cftTextWordWrap);
			IColumnSettingsPtr pCol = m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Lab to be ordered", -1, csVisible|csWidthAuto|csEditable));
			m_pList->PutDisplayColumn("[1]");
			m_pList->PutHeadersVisible(VARIANT_TRUE);
			pCol->PutFieldType(cftTextWordWrap);
			pCol->PutSortPriority(0);
			pCol->PutSortAscending(VARIANT_TRUE);
			pCol->DataType = VT_BSTR;
			// (c.haag 2009-05-07 11:47) - PLID 28550 - We now have a
			// column that contains the display instruction text which pops up
			// when a user selects what is to be ordered.
			pCol = m_pList->GetColumn(m_pList->InsertColumn(2, "DisplayInstructions", "Display Instructions", -1, csVisible|csWidthAuto|csEditable));
			pCol->PutFieldType(cftTextWordWrap);
			pCol->DataType = VT_BSTR;
			//TES 7/12/2011 - PLID 44538 - We now also have an embedded dropdown column that associates the value with a LOINC code
			pCol = m_pList->GetColumn(m_pList->InsertColumn(3, "LOINCID", "LOINC Code", -1, csVisible|csWidthAuto|csEditable));
			pCol->PutFieldType(cftComboSimple);
			pCol->PutComboSource(_bstr_t("SELECT ID, Code + ' - ' + ShortName FROM LabLoincT"));
			pCol->DataType = VT_I4;
		}
		// (j.jones 2009-01-23 09:32) - PLID 2822 - added Inventory Order Methods
		else if(m_listID == INVEDITORDER_ORDERMETHOD_COMBO)
		{
			m_pList->FromClause = "OrderMethodsT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Method", "Method", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (a.walling 2009-05-04 08:29) - PLID 33751 - Problem Chronicity
		else if(m_listID == PROBLEM_CHRONICITY_COMBO)
		{
			m_pList->FromClause = "EMRProblemChronicityT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Name", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Inactive", "Inactive", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftBoolCheckbox;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = VARIANT_TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BOOL;
		}
		// (a.walling 2009-05-28 16:09) - PLID 34389
		else if(m_listID == EMRDATACODES_COMBO)
		{
			// (j.gruber 2009-11-18 16:53) - PLID 35945 - add IsTops to the datalist
			// (j.gruber 2009-12-23 10:02) - PLID 35771 - checked to IsSpecial and included report code
			// (j.gruber 2009-12-23 16:37) - PLID 35767 - added educational resources code
			// (j.gruber 2009-12-23 16:37) - PLID 35769 - added clinical summary code
			//(e.lally 2010-01-14) PLID 36839 - Added 24 more TOPS codes
			// (j.gruber 2010-01-18 10:11) - PLID 36929 - took out the report codes
			// (j.gruber 2010-06-01 13:15) - PLID 38377 - added BOLD codes
			m_pList->FromClause = "(SELECT ID, Code, Description, DefaultUnit, Vital, Inactive, "
				" CASE WHEN Code IN ('TOPS_ProcedureDuration','TOPS_AnesthesiaProvidedBy','TOPS_ModeOfAnesthesia','TOPS_TobaccoUse','TOPS_HasDiabetes',"
				"'TOPS_DiabetesMedication','TOPS_PatientASAStatus', "
				"'TOPS_VTEProphylaxis', 'TOPS_ProcRelatedWtLoss', 'TOPS_PrevBariatricSurg', 'TOPS_LipoplastyVolInfused', 'TOPS_LipoplastyIVIntake', 'TOPS_LipoplastyAspirated', "
				"'TOPS_BreastImplantRight', 'TOPS_BreastImplantLeft', 'TOPS_ImplantMfrRight', 'TOPS_ImplantMfrLeft', "
				"'TOPS_ImplantSerialRight', 'TOPS_ImplantSerialLeft', 'TOPS_ImplantShellRight', 'TOPS_ImplantShellLeft', "
				"'TOPS_ImplantShapeRight', 'TOPS_ImplantShapeLeft', 'TOPS_ImplantFillerRight', 'TOPS_ImplantFillerLeft', "
				"'TOPS_PostOpAdjRight', 'TOPS_PostOpAdjLeft', 'TOPS_ImplantPositionRight', 'TOPS_ImplantPositionLeft', "
				"'TOPS_FillerVolumeRight', 'TOPS_FillerVolumeLeft' "
				" ) OR (Left(Code, 5) = 'BOLD_') THEN Convert(bit, 1) else Convert(bit, 0) END as IsSpecialCode "
				" FROM EMRDataCodesT) Q";				
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Code", "Code", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(3, "DefaultUnit", "Def. Unit", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(4, "Vital", "Vital", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftBoolCheckbox;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(5, "Inactive", "Inactive", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftBoolCheckbox;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(6, "IsSpecialCode", "IsSpecialCode", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = VARIANT_TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BSTR;
			m_pList->GetColumn(3)->DataType = VT_BSTR;
			m_pList->GetColumn(4)->DataType = VT_BOOL;
			m_pList->GetColumn(5)->DataType = VT_BOOL;
			m_pList->GetColumn(6)->DataType = VT_BOOL;
		}		
		// (j.gruber 2009-05-08 09:47) - PLID 34202 - Provider Type
		else if(m_listID == PROVIDER_TYPES) {
			m_pList->FromClause = "ProviderTypesT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (d.thompson 2009-05-14) - PLID 34232 - Immunization routes
		else if(m_listID == IMMUNIZATION_ROUTES) {
			m_pList->FromClause = "ImmunizationRoutesT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Name", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			// (d.singleton 2013-05-15 16:26) - PLID 56716 - edit the immunizations route and site CEditComboBox settings to include the new "Code" row
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Code", "Code", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BSTR;
		}
		// (d.thompson 2009-05-14) - PLID 34232 - Immunization sites
		else if(m_listID == IMMUNIZATION_SITES) {
			m_pList->FromClause = "ImmunizationSitesT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Name", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			// (d.singleton 2013-05-15 16:26) - PLID 56716 - edit the immunizations route and site CEditComboBox settings to include the new "Code" row
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Code", "Code", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BSTR;
		}
		else if(m_listID == LAB_ANATOMY_QUALIFIERS_COMBO) {
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			m_pList->FromClause = "AnatomyQualifiersT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Name", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Inactive", "Inactive", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftBoolCheckbox;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BOOL;
		} else if(m_listID == LAB_LOINC) {
			// (a.walling 2010-01-18 13:01) - PLID 36955 - Set up the LOINC columns		
			GetDlgItem(IDOK)->SetWindowText("OK");
			m_btnOK.AutoSet(NXB_OK);
			GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOWNA);
				
			GetDlgItem(IDOK)->EnableWindow(FALSE);

			m_pList->FromClause = "LabLOINCT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Code", "Code", -1, csVisible|csWidthData|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "ShortName", "Short", -1, csVisible|csWidthData|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(3, "LongName", "Long", -1, csVisible|csWidthData|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BSTR;
			m_pList->GetColumn(3)->DataType = VT_BSTR;
		}
		// (d.lange 2011-01-03 09:46) - PLID 29065 - Added Biopsy Type
		else if(m_listID == LAB_BIOPSY_TYPE) {
			m_pList->FromClause = "LabBiopsyTypeT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(2, "Inactive", "Inactive", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftBoolCheckbox;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
			m_pList->GetColumn(2)->DataType = VT_BOOL;
		}
		else if(m_listID == GLASSES_ORDER_HISTORY_NOTES)
		{
			//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, GetFieldName(0), GetColumnTitle(0), -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextWordWrap;
			m_pList->DisplayColumn = "[0]";
			m_pList->HeadersVisible = FALSE;
			m_pList->GetColumn(0)->PutSortPriority(0);
			m_pList->GetColumn(0)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_BSTR;
		}
		//(j.deskurakis 2013-01-24) - PLID 53151 - modified
		// (f.dinatale 2010-07-08) - PLID 39556
		else if(m_listID == INTEGRATION_BILLTO)
		{
			m_pList->FromClause = "IntegrationBillToT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "IntegrationBillToID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(0)->DataType = VT_I4;
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (f.dinatale 2010-07-08) - PLID 39556
		else if(m_listID == INTEGRATION_LABTYPES)
		{
			m_pList->FromClause = "IntegrationLabTypesT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "LabTypeID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Description", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(0)->DataType = VT_I4;
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		else if(m_listID == CONTACT_LENS_TYPE)
		{
			// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
			m_pList->FromClause = "GlassesContactLensTypeT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "ContactLensType", "Contact Lens Type", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(0)->DataType = VT_I4;
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		else if(m_listID == CONTACT_LENS_MANUFACTURE)
		{
			// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
			m_pList->FromClause = "ContactLensManufacturersT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Name", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(0)->DataType = VT_I4;
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (b.spivey, August 17, 2012) - PLID 52130 - Image stamp categories. 
		else if(m_listID == EMR_IMAGE_STAMP_CATEGORIES) {
			m_pList->FromClause = "EmrImageStampCategoryT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible|csFixedWidth)))->FieldType = cftTextWordWrap;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "Description", "Name", -1, csVisible|csWidthAuto|csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = VARIANT_TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}
		// (r.gonet 07/07/2014) - PLID 62520 - BillStatusNoteT
		else if (m_listID == BILL_STATUS_NOTES_COMBO) {
			m_pList->FromClause = "BillStatusNoteT";
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(0, "ID", "ID", 0, csVisible | csFixedWidth)))->FieldType = cftTextSingleLine;
			IColumnSettingsPtr(m_pList->GetColumn(m_pList->InsertColumn(1, "TextData", "Text", -1, csVisible | csWidthAuto | csEditable)))->FieldType = cftTextSingleLine;
			m_pList->DisplayColumn = "[1]";
			m_pList->HeadersVisible = VARIANT_TRUE;
			m_pList->GetColumn(1)->PutSortPriority(0);
			m_pList->GetColumn(1)->PutSortAscending(TRUE);
			m_pList->GetColumn(0)->DataType = VT_I4;
			m_pList->GetColumn(1)->DataType = VT_BSTR;
		}

		m_pList->Requery();

		// (a.walling 2008-06-04 15:17) - PLID 29900 - Dead code
		//long PatID = GetActivePatientID();

		EnableAppropriateButtons();

		// (j.armen 2012-06-06 15:50) - PLID 49856 - Set the min size and remeber size/position
		SetMinSize(350,350);
		SetMaxSize(1000,700);
		SetRecallSizeAndPosition(FormatString("CEditComboBox-%li", m_listID), false);

		CNxDialog::OnInitDialog();
		
		//Let's keep the dlg centered
		CenterWindow();

		return TRUE;  // return TRUE unless you set the focus to a control
					  // EXCEPTION: OCX Property Pages should return FALSE
	}NxCatchAll("Error in OnInitDialog");
	CDialog::OnCancel();
	return FALSE;
}

BOOL CEditComboBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	//(e.lally 2008-10-13) PLID 31665 - Added error handling
	try{
		switch(wParam) {
			case ID_SET_DEFAULT:
				SetAsDefault();
				break;
			case ID_REMOVE_DEFAULT:
				RemoveDefault();
				break;
		}
	}NxCatchAll("Error in CEditComboBox::OnCommand");
	
	return CDialog::OnCommand(wParam, lParam);
}

void CEditComboBox::OnOK() 
{
	try{
		if(m_pFromListV1 != NULL && m_pFromListV2 != NULL)
			AfxThrowNxException("An error occurred while trying to refresh the list. At least two versions of the list contain values.");
		else if(m_pFromListV1 != NULL)
			m_pFromListV1->Requery();	
		else if(m_pFromListV2 != NULL)
			m_pFromListV2->Requery();
		else {
			//TES 11/6/2009 - PLID 36189 - Added LAB_ANATOMIC_LOCATION_COMBO
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			if (m_listID != LAB_DESCRIPTION_COMBO && m_listID != LAB_CLINICAL_DIAGNOSIS_COMBO &&
				// (c.haag 2007-11-12 10:18) - PLID 28010 - Support for supplier return combo configurations
				m_listID != INVEDITRETURN_METHOD_COMBO && m_listID != INVEDITRETURN_REASON_COMBO &&
				m_listID != PROBLEM_CHRONICITY_COMBO && m_listID != PROVIDER_TYPES && m_listID != EMRDATACODES_COMBO &&
				m_listID != LAB_ANATOMIC_LOCATION_COMBO && m_listID != LAB_ANATOMY_QUALIFIERS_COMBO &&
				// (a.walling 2010-01-18 13:58) - PLID 36955
				m_listID != LAB_LOINC
				// (j.jones 2010-06-25 16:20) - PLID 39185 - this list is no longer provided to us
				&& m_listID != LAB_TO_BE_ORDERED_COMBO
				//(j.deskurakis 2013-01-24) - PLID 53151 - modified
				// (f.dinatale 2010-07-09) - PLID  39556
				&& m_listID != INTEGRATION_BILLTO && m_listID != INTEGRATION_LABTYPES 
				// (d.lange 2011-01-03 09:47) - PLID 29065 - Added Biopsy Type
				&& m_listID != LAB_BIOPSY_TYPE
				//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
				&& m_listID != GLASSES_ORDER_HISTORY_NOTES
				// (s.dhole 2012-03-14 10:06) - PLID 49191 Contact lens Type of Lens Product 
				&& m_listID != CONTACT_LENS_TYPE
				// (s.dhole 2012-03-19 16:04) - PLID 48973 Add cotact lens manufacturer
				&& m_listID != CONTACT_LENS_MANUFACTURE 
				// (b.spivey, August 17, 2012) - PLID 52130 - Image stamp categories. 
				&& m_listID != EMR_IMAGE_STAMP_CATEGORIES
				// (r.gonet 07/07/2014) - PLID 62520 - BillStatusNoteT
				&& m_listID != BILL_STATUS_NOTES_COMBO
				) {
				AfxThrowNxException("An error occurred while trying to refresh the list. None of the versions of the list were set.");
			}
		}

	}NxCatchAll("Error in CEditComboBox::OnOK");
	//We still want to close the dialog.
	CDialog::OnOK();
}

void CEditComboBox::OnCancel() 
{
	try{
		if(m_pFromListV1 != NULL && m_pFromListV2 != NULL)
			AfxThrowNxException("An error occurred while trying to refresh the list. At least two versions of the list contain values.");
		else if(m_pFromListV1 != NULL)
			m_pFromListV1->Requery();	
		else if(m_pFromListV2 != NULL)
			m_pFromListV2->Requery();
		else {
			//TES 11/6/2009 - PLID 36189 - Added LAB_ANATOMIC_LOCATION_COMBO
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			if (m_listID != LAB_DESCRIPTION_COMBO && m_listID != LAB_CLINICAL_DIAGNOSIS_COMBO &&
				// (c.haag 2007-11-12 10:18) - PLID 28010 - Support for supplier return combo configurations
				m_listID != INVEDITRETURN_METHOD_COMBO && m_listID != INVEDITRETURN_REASON_COMBO && 
				m_listID != PROBLEM_CHRONICITY_COMBO && m_listID != PROVIDER_TYPES && m_listID != EMRDATACODES_COMBO &&
				m_listID != LAB_ANATOMIC_LOCATION_COMBO  && m_listID != LAB_ANATOMY_QUALIFIERS_COMBO &&
				// (a.walling 2010-01-18 13:57) - PLID 36955
				m_listID != LAB_LOINC
				// (j.jones 2010-06-25 16:20) - PLID 39185 - this list is no longer provided to us
				&& m_listID != LAB_TO_BE_ORDERED_COMBO
				//(j.deskurakis 2013-01-24) - PLID 53151 - modified
				// (f.dinatale 2010-07-09) - PLID  39556
				&& m_listID != INTEGRATION_BILLTO && m_listID != INTEGRATION_LABTYPES 
				// (d.lange 2011-01-03 09:49) - PLID 29065 - Added Biopsy Type
				&& m_listID != LAB_BIOPSY_TYPE
				//TES 5/25/2011 - PLID 43842 - Standard notes for Glasses Order History changes
				&& m_listID != GLASSES_ORDER_HISTORY_NOTES 
				// (b.spivey, August 17, 2012) - PLID 52130 - Image stamp categories. 
				&& m_listID != EMR_IMAGE_STAMP_CATEGORIES 
				// (r.gonet 07/07/2014) - PLID 62520 - BillStatusNoteT
				&& m_listID != BILL_STATUS_NOTES_COMBO
				) {
				AfxThrowNxException("An error occurred while trying to refresh the list. None of the versions of the list were set.");
			}
		}
	}NxCatchAll("Error in CEditComboBox::OnCancel");
	//We still want to close the dialog.
	CDialog::OnCancel();
}

void CEditComboBox::SetAsDefault()
{
	if(m_pList->CurSel == -1 || !m_bAllowDefault)
		return;

	//lists that allow defaults
	if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
		//first remove colors
		CString strOldDefault = GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,_bstr_t(strOldDefault),0,FALSE);
		if(nRow >= 0)
			IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//save the default
		CString strNewDefault = CString(m_pList->GetValue(m_pList->CurSel,0).bstrVal);
		SetRemotePropertyText("DefaultPayDesc",strNewDefault,0,"<None>");

		//now set colors
		IRowSettingsPtr(m_pList->GetRow(m_pList->CurSel))->PutForeColor(RGB(255,0,0));
	}
	// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
	/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {
		//first remove colors
		long nOldDefault = GetRemotePropertyInt("DefaultPayCat",-1,0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,(long)nOldDefault,0,FALSE);
		if(nRow >= 0)
			IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//save the default
		long nNewDefault = m_pList->GetValue(m_pList->CurSel,0).lVal;
		SetRemotePropertyInt("DefaultPayCat",nNewDefault,0,"<None>");

		//now set colors
		IRowSettingsPtr(m_pList->GetRow(m_pList->CurSel))->PutForeColor(RGB(255,0,0));	
	}*/
	// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
	/*
	else if(m_listID == PATIENT_TYPE_COMBO) {
		//first remove colors
		long nOldDefault = GetRemotePropertyInt("DefaultPatType",-1,0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,(long)nOldDefault,0,FALSE);
		if(nRow >= 0)
			IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//save the default
		long nNewDefault = m_pList->GetValue(m_pList->CurSel,0).lVal;
		SetRemotePropertyInt("DefaultPatType",nNewDefault,0,"<None>");

		//now set colors
		IRowSettingsPtr(m_pList->GetRow(m_pList->CurSel))->PutForeColor(RGB(255,0,0));	
	}
	*/
}

void CEditComboBox::RemoveDefault()
{
	if(m_pList->CurSel == -1 || !m_bAllowDefault)
		return;

	//lists that allow defaults
	if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
		//first remove colors
		CString strOldDefault = GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,_bstr_t(strOldDefault),0,FALSE);
		if(nRow >= 0)
			IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//now remove the default
		SetRemotePropertyText("DefaultPayDesc","",0,"<None>");
	}
	// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
	/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {
		//first remove colors
		long nOldDefault = GetRemotePropertyInt("DefaultPayCat",-1,0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,(long)nOldDefault,0,FALSE);
		if(nRow >= 0)
			IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//now remove the default
		SetRemotePropertyInt("DefaultPayCat",-1,0,"<None>");
	}*/
	// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
	/*
	else if(m_listID == PATIENT_TYPE_COMBO) {
		//first remove colors
		long nOldDefault = GetRemotePropertyInt("DefaultPatType",-1,0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,(long)nOldDefault,0,FALSE);
		if(nRow >= 0)
			IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//now remove the default
		SetRemotePropertyInt("DefaultPatType",-1,0,"<None>");
	}
	*/
}

void CEditComboBox::OnRequeryFinishedEditList(short nFlags) 
{
	//(e.lally 2008-10-13) PLID 31665 - Added error handling
	try{

		// (a.walling 2010-01-18 14:46) - PLID 36955 - Select the initial selection, if available, and if no user selection has already been made
		if (m_listID == LAB_LOINC) {
			if (m_varInitialSel.vt == VT_BSTR) {
				// if they selected something, don't bother
				if (m_pList->CurSel == sriNoRow) {
					long nInitialSelRow = m_pList->TrySetSelByColumn(1, m_varInitialSel);
					EnableAppropriateButtons();
					if (nInitialSelRow >= 0) {
						m_nLastSelID = VarLong(m_pList->GetValue(nInitialSelRow,0), -1);
					}
				}
			}
		}

		//if not m_bAllowDefault, no need to be in this function
		if(!m_bAllowDefault)
			return;

		//lists that allow defaults
		if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
			CString strDefault = GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE);
			int nRow = m_pList->FindByColumn(0,_bstr_t(strDefault),0,FALSE);
			if(nRow >= 0)
				IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(255,0,0));
		}
		// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
		/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {
			long nDefault = GetRemotePropertyInt("DefaultPayCat",-1,0,"<None>",TRUE);
			int nRow = m_pList->FindByColumn(0,(long)nDefault,0,FALSE);
			if(nRow >= 0)
				IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(255,0,0));		
		}*/
		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
		/*
		else if(m_listID == PATIENT_TYPE_COMBO) {
			long nDefault = GetRemotePropertyInt("DefaultPatType",-1,0,"<None>",TRUE);
			int nRow = m_pList->FindByColumn(0,(long)nDefault,0,FALSE);
			if(nRow >= 0)
				IRowSettingsPtr(m_pList->GetRow(nRow))->PutForeColor(RGB(255,0,0));		
		}
		*/
	}NxCatchAll("Error in CEditComboBox::OnRequeryFinishedEditList");
}

//check to see if a given row is a default row
BOOL CEditComboBox::IsDefaultSelected() {

	if(!m_bAllowDefault)
		return FALSE;

	if(m_pList->CurSel == -1)
		return FALSE;

	//lists that allow defaults
	if(m_listID == PAYMENT_DESCRIPTION_COMBO) {
		CString strDefault = GetRemotePropertyText("DefaultPayDesc","",0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,_bstr_t(strDefault),0,FALSE);
		return (nRow == m_pList->CurSel);
	}
	// (j.gruber 2012-11-15 14:02) - PLID 53752 - took out for its own dialog
	/*else if(m_listID == PAYMENT_CATEGORY_COMBO) {
		long nDefault = GetRemotePropertyInt("DefaultPayCat",-1,0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,(long)nDefault,0,FALSE);
		return (nRow == m_pList->CurSel);
	}*/
	// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated in favor of a new dialog.
	/*
	else if(m_listID == PATIENT_TYPE_COMBO) {
		long nDefault = GetRemotePropertyInt("DefaultPatType",-1,0,"<None>",TRUE);
		int nRow = m_pList->FindByColumn(0,(long)nDefault,0,FALSE);
		return (nRow == m_pList->CurSel);
	}
	*/

	return FALSE;
}

void CEditComboBox::EnableAppropriateButtons()
{
	//TES 11/5/2007 - PLID 27978 - VS 2008 - This was just a flat out bug that VS 6.0 didn't catch
	if(m_pList->GetRowCount() == 0 || m_pList->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT)->EnableWindow(FALSE);
		// (a.walling 2010-01-18 14:01) - PLID 36955 - If we are in select mode, disable OK if no selection
		if (m_listID == LAB_LOINC) {
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}
	}
	else{
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT)->EnableWindow(TRUE);
		// (a.walling 2010-01-18 14:01) - PLID 36955 - If we are in select mode, enable OK if there is a selection
		if (m_listID == LAB_LOINC) {
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
	}
}

void CEditComboBox::OnSelChangedEditList(long nNewSel) 
{
	//(e.lally 2008-10-13) PLID 31665 - Added error handling
	try{
		EnableAppropriateButtons();
		// (a.walling 2010-01-18 13:17) - PLID 36955 - Keep track of the last selected item
		if (m_listID == LAB_LOINC) {
			if (nNewSel >= 0) {
				m_nLastSelID = VarLong(m_pList->GetValue(nNewSel,0), -1);
			} else {				
				m_nLastSelID = -1;
			}
		}
	}NxCatchAll("Error while enabling buttons.");
}

// (j.jones 2007-07-19 12:04) - PLID 26749 - added ability to disallow changes to data
void CEditComboBox::OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		if(nRow == -1)
			return;

		// (j.jones 2007-07-20 12:02) - PLID 26749 - disallow editing anatomic locations in use
		if(m_listID == LAB_ANATOMIC_LOCATION_COMBO) {
			//TES 11/6/2009 - PLID 36189 - Check which column we're editing
			if(nCol == 1) {
				//TES 2/9/2010 - PLID 37223 - Need to also check for HotSpots
				long nAnatomyID = VarLong(m_pList->GetValue(nRow,0),-1);
				_RecordsetPtr rsCounts = CreateParamRecordset("SELECT (SELECT Count(*) FROM LabsT WHERE AnatomyID = {INT}) AS LabCount, "
					"(SELECT Count(*) FROM EmrImageHotSpotsT WHERE AnatomicLocationID = {INT}) AS HotSpotCount", 
					nAnatomyID, nAnatomyID);
				long nLabCount = AdoFldLong(rsCounts, "LabCount", 0);
				long nHotSpotCount = AdoFldLong(rsCounts, "HotSpotCount", 0);
				if(nLabCount > 0 || nHotSpotCount > 0)
				{
					MsgBox("You may not edit this anatomic location because there are %d Lab(s) and %d EMR Image HotSpot(s) associated with it.", 
						nLabCount, nHotSpotCount);
					*pbContinue = FALSE;
					return;
				}
				//if m_nCurIDInUse is set, it means the calling Lab is using that AnatomyID, which might not be saved
				else if(nLabCount == 0 && nHotSpotCount == 0 && m_nCurIDInUse == nAnatomyID) {
					//TES 2/10/2010 - PLID 37223 - The current record may be a lab or a hotspot
					if(IDNO == MessageBox("The current lab or hotspot is associated with this anatomic location. Are you sure you wish to rename it?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						*pbContinue = FALSE;
						return;
					}
				}
			}
		}
		// (c.haag 2007-11-14 12:22) - PLID 28010 - Support for supplier return methods
		else if (m_listID == INVEDITRETURN_METHOD_COMBO) {

			// Do not allow changes to methods in use
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM SupplierReturnGroupsT WHERE ReturnMethodID = {INT}", VarLong(m_pList->GetValue(nRow,0), -1));
			if (!prs->eof) {
				MsgBox("You may not rename this method because it is in use by existing supplier returns.");
				*pbContinue = FALSE;
				return;
			}
		}
		// (c.haag 2007-11-14 12:22) - PLID 28010 - Support for supplier return reasons
		else if (m_listID == INVEDITRETURN_REASON_COMBO) {

			// Do not allow changes to reasons in use
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM SupplierReturnItemsT WHERE ReturnReasonID = {INT}", VarLong(m_pList->GetValue(nRow,0), -1));
			if (!prs->eof) {
				MsgBox("You may not rename this reason because it is in use by existing supplier returns.");
				*pbContinue = FALSE;
				return;
			}
		}
		// (j.jones 2009-01-23 09:32) - PLID 2822 - added Inventory Order Methods
		else if(m_listID == INVEDITORDER_ORDERMETHOD_COMBO)
		{
			//don't allow renaming items in use
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderT WHERE OrderMethodID = {INT}", VarLong(m_pList->GetValue(nRow,0), -1));
			if (!prs->eof) {
				MsgBox("You may not rename this order method because it is in use on existing orders.");
				*pbContinue = FALSE;
				return;
			}
		}
		else if(m_listID == LAB_ANATOMY_QUALIFIERS_COMBO) {
			//TES 11/10/2009 - PLID 36128 - Added AnatomyQualifiersT
			if(nCol == 1) {
				//TES 2/9/2010 - PLID 37223 - Need to also check for HotSpots
				long nQualID = VarLong(m_pList->GetValue(nRow,0),-1);
				_RecordsetPtr rsCounts = CreateParamRecordset("SELECT (SELECT Count(*) FROM LabsT WHERE AnatomyQualifierID = {INT}) AS LabCount, "
					"(SELECT Count(*) FROM EmrImageHotSpotsT WHERE AnatomicQualifierID = {INT}) AS HotSpotCount", 
					nQualID, nQualID);
				long nLabCount = AdoFldLong(rsCounts, "LabCount", 0);
				long nHotSpotCount = AdoFldLong(rsCounts, "HotSpotCount", 0);
				if(nLabCount > 0 || nHotSpotCount > 0)
				{
					MsgBox("You may not edit this anatomic location qualifier because there are %d Lab(s) and %d EMR Image HotSpot(s) associated with it.", 
						nLabCount, nHotSpotCount);
					*pbContinue = FALSE;
					return;
				}
				//if m_nCurIDInUse is set, it means the calling Lab is using that AnatomyID, which might not be saved
				else if(nLabCount == 0 && nHotSpotCount == 0&& m_nCurIDInUse == nQualID) {
					//TES 2/10/2010 - PLID 37223 - The current record may be a lab or a hotspot
					if(IDNO == MessageBox("The current lab or hotspot is associated with this anatomic location qualifier. Are you sure you wish to rename it?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						*pbContinue = FALSE;
						return;
					}
				}
			}
		}

	}NxCatchAll("Error in CEditComboBox::OnEditingStartingList");
}