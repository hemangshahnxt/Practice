// SalesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "SalesDlg.h"
#include "GlobalDataUtils.h"
#include "TaskEditDlg.h"
#include "MultiSelectDlg.h" // (j.armen 2011-06-28 11:54) - PLID 44342
#include "SpecialtyConfigDlg.h" // (j.luckoski 04/10/12) - PLID 49491 - Specialty Dialog to edit specialties for proposals.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// SalesDlg dialog


CSalesDlg::CSalesDlg(CWnd* pParent)
	: CPatientDialog(CSalesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SalesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSalesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SalesDlg)
	DDX_Control(pDX, IDC_CREATE_TODO, m_btnCreateTodo);
	DDX_Control(pDX, IDC_SEL_REFERRAL, m_btnSelReferral);
	DDX_Control(pDX, IDC_COMPANY, m_nxeditCompany);
	DDX_Control(pDX, IDC_TITLE_BOX, m_nxeditTitleBox);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeditAddress1Box);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeditAddress2Box);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeditZipBox);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeditCityBox);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeditStateBox);
	DDX_Control(pDX, IDC_CALLER_BOX, m_nxeditCallerBox);
	DDX_Control(pDX, IDC_WORK_PHONE_BOX, m_nxeditWorkPhoneBox);
	DDX_Control(pDX, IDC_EXT_PHONE_BOX, m_nxeditExtPhoneBox);
	DDX_Control(pDX, IDC_FAX_PHONE_BOX, m_nxeditFaxPhoneBox);
	DDX_Control(pDX, IDC_BACKLINE_BOX, m_nxeditBacklineBox);
	DDX_Control(pDX, IDC_CELL_PHONE_BOX, m_nxeditCellPhoneBox);
	DDX_Control(pDX, IDC_PAGER_PHONE_BOX, m_nxeditPagerPhoneBox);
	DDX_Control(pDX, IDC_WEBSITE_BOX, m_nxeditWebsiteBox);
	DDX_Control(pDX, IDC_DOC_EMAIL_BOX, m_nxeditDocEmailBox);
	DDX_Control(pDX, IDC_PRAC_EMAIL_BOX, m_nxeditPracEmailBox);
	DDX_Control(pDX, IDC_REF_DATE, m_nxeditRefDate);
	DDX_Control(pDX, IDC_SOURCE_BOX, m_nxeditSourceBox);
	DDX_Control(pDX, IDC_REFERRAL, m_nxeditReferral);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_CUSTOM1_LABEL, m_nxstaticCustom1Label);
	DDX_Control(pDX, IDC_CUSTOM3_LABEL, m_nxstaticCustom3Label);
	DDX_Control(pDX, IDC_CUSTOM1_LABEL3, m_nxstaticCustom1Label3);
	//(a.wilson 2011-4-28) PLID 43355 - controls to handle the information in the boxes
	DDX_Control(pDX, IDC_DISCOUNT_PERCENT_BOX, m_neditDiscountPercent);
	DDX_Control(pDX, IDC_ADDON_DISCOUNT_PERCENT_BOX, m_neditAddOnDiscountPercent);
	DDX_Control(pDX, IDC_SOCIETY_LABEL, m_nxlSocietyLabel); // (j.armen 2011-06-28 11:54) - PLID 44342 - added binding for society label
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSalesDlg, CPatientDialog)
	//{{AFX_MSG_MAP(SalesDlg)
	ON_BN_CLICKED(IDC_EMAIL, OnEmail)
	ON_BN_CLICKED(IDC_EMAIL2, OnEmail2)
	ON_BN_CLICKED(IDC_VIEW_TODO, OnViewTodo)
	ON_BN_CLICKED(IDC_CREATE_TODO, OnCreateTodo)
	ON_BN_CLICKED(IDC_VIEW_OPPORTUNITIES, OnViewOpportunities)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_VISIT_WEBSITE, &CSalesDlg::OnBnClickedVisitWebsite) //(a.wilson 2011-5-6) PLID 9702
	// (j.armen 2011-06-28 11:54) - PLID 44342 - event handling for clicking/mouse over labels
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SalesDlg message handlers


void CSalesDlg::SetColor(OLE_COLOR nNewColor)
{
	((CNxColor*)GetDlgItem(IDC_DEMOGRAPHICS_BKG))->SetColor(nNewColor);
	((CNxColor*)GetDlgItem(IDC_PHONE_BKG))->SetColor(nNewColor);
	((CNxColor*)GetDlgItem(IDC_OTHER_BKG))->SetColor(nNewColor);
	((CNxColor*)GetDlgItem(IDC_REFERENCE_BKG))->SetColor(nNewColor);
	//(a.wilson 2011-4-28) PLID 43355 - to handle what color the background box should be for the current patient
	((CNxColor*)GetDlgItem(IDC_DISCOUNT_BKG))->SetColor(nNewColor);

	CPatientDialog::SetColor(nNewColor);
}

void CSalesDlg::Save(int nID)
{
	CString field, value;
	double temp; //(a.wilson 2011-6-8) PLID 43355

	if(!m_changed)
		return;
	
	if(!nID) {
		m_changed = false;
		return;
	}

	try{
	
		GetDlgItemText(nID, value);

		switch(nID)
		{

		case IDC_COMPANY:
			field = "Company";
			break;

		case IDC_FIRST_NAME_BOX:
			field = "First";
			break;

		case IDC_MIDDLE_NAME_BOX:
			field = "Middle";
			break;

		case IDC_LAST_NAME_BOX:
			field = "Last";
			break;

		case IDC_ADDRESS1_BOX:
			field = "Address1";
			break;

		case IDC_ADDRESS2_BOX:
			field = "Address2";
			break;

		case IDC_CITY_BOX:
			field = "City";
			break;

		case IDC_STATE_BOX:
			field = "State";
			break;
			
		case IDC_ZIP_BOX:
			field = "Zip";
			break;

		case IDC_CALLER_BOX:	//custom 1
			{
				_RecordsetPtr rs;
				rs = CreateRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 1", m_id);
				if(rs->eof) {	//no record here
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 1"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (1, 'Custom 1', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) (SELECT %li, 1, '%s')", m_id, _Q(value));
				}
				else
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE PersonID = %li AND FieldID = 1", _Q(value), m_id);
				rs->Close();
				return;
			}
			break;

		case IDC_WORK_PHONE_BOX:
			field = "WorkPhone";
			break;

		case IDC_EXT_PHONE_BOX:
			field = "Extension";
			break;

		case IDC_FAX_PHONE_BOX:
			field = "Fax";
			break;

		case IDC_BACKLINE_BOX:
			field = "EmergWPhone";
			break;

		case IDC_CELL_PHONE_BOX:
			field = "CellPhone";
			break;

		case IDC_PAGER_PHONE_BOX:
			field = "Pager";
			break;

		case IDC_WEBSITE_BOX:	//custom 3
			{
				_RecordsetPtr rs;
				rs = CreateRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 3", m_id);
				if(rs->eof)	{ //no record here
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 3"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (3, 'Custom 3', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) (SELECT %li, 3, '%s')", m_id, _Q(value));
				}
				else
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE PersonID = %li AND FieldID = 3", _Q(value), m_id);
				rs->Close();
				return;
			}
			break;

		case IDC_DOC_EMAIL_BOX:
			field = "Email";
			break;

		case IDC_PRAC_EMAIL_BOX:
			{
			ExecuteSql("UPDATE NxClientsT SET PracEmail = '%s' WHERE PersonID = %li", _Q(value), m_id);
			return;
			}
			break;

		case IDC_SOURCE_BOX:	//custom 2
			{
				_RecordsetPtr rs;
				rs = CreateRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 2", m_id);
				if(rs->eof)	{ //no record here
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 2"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (2, 'Custom 2', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) (SELECT %li, 2, '%s')", m_id, _Q(value));
				}
				else
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE PersonID = %li AND FieldID = 2", _Q(value), m_id);
				rs->Close();
				return;
			}
			break;

		case IDC_NOTES:
			field = "Note";
			break;

		case IDC_TITLE_BOX:
			field = "Title";
			break;

		case IDC_REF_DATE:
			if(value == "")
				ExecuteSql("UPDATE NxClientsT SET RefDate = NULL WHERE PersonID = %li", m_id);
			else
				ExecuteSql("UPDATE NxClientsT SET RefDate = '%s' WHERE PersonID = %li", _Q(value), m_id);
			return;
			break;
		//(a.wilson 2011-4-29) PLID 43355 - store the changed percents
		case IDC_DISCOUNT_PERCENT_BOX:	//save the discount percent box
			temp = atof(value);
			if (temp >= 0.00 && temp <= 100.00) {
				ExecuteParamSql("UPDATE SupportDiscountsT SET DiscountPercent = {DOUBLE} WHERE PersonID = {INT}", temp, m_id);
				SetDlgItemText(IDC_DISCOUNT_PERCENT_BOX, FormatString("%.02f", temp));
			} else {
				SetDlgItemText(IDC_DISCOUNT_PERCENT_BOX, "0.00");
			}
			break;
		case IDC_ADDON_DISCOUNT_PERCENT_BOX:	//save the add on discount percent box
			temp = atof(value);
			if (temp >= 0.00 && temp <= 100.00) {
				ExecuteParamSql("UPDATE SupportDiscountsT SET AddOnDiscountPercent = {DOUBLE} WHERE PersonID = {INT}", temp, m_id);
				SetDlgItemText(IDC_ADDON_DISCOUNT_PERCENT_BOX, FormatString("%.02f", temp));
			} else {
				SetDlgItemText(IDC_ADDON_DISCOUNT_PERCENT_BOX, "0.00");
			}
			break;
		default:
			break;

		}

		if(field == "") {
			m_changed = false;
			return;
		}

		ExecuteSql("UPDATE PersonT SET %s = '%s' WHERE ID = %li", field, _Q(value), m_id);
	} NxCatchAll("Error in CSalesDlg::Save()");

	m_changed = false;

}

void CSalesDlg::Load()
{
	try{

		_RecordsetPtr rs;	//(a.wilson 2011-4-28) PLID 43355 - updated query with discount information and parameterized
		rs = CreateParamRecordset("SELECT PersonT.Company, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
			"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.WorkPhone, PersonT.Extension, PersonT.Fax, "
			"PersonT.EmergWPhone, PersonT.CellPhone, PersonT.Pager, PersonT.Email, PersonT.Note, PersonT.Title, "
			"NxClientsT.RefRating, NxClientsT.RefDate, NxClientsT.RefStatus, "
			"SupportDiscountsT.TypeID, SupportDiscountsT.AddOnDiscountPercent, SupportDiscountsT.DiscountPercent "
			"FROM PersonT "
			"INNER JOIN NxClientsT ON PersonT.ID = NxClientsT.PersonID "
			"LEFT JOIN SupportDiscountsT ON PersonT.ID = SupportDiscountsT.PersonID "
			"WHERE PersonT.ID = {INT}", m_id);

		if(!rs->eof){
			SetDlgItemText(IDC_COMPANY, CString(rs->Fields->Item["Company"]->Value.bstrVal));
			SetDlgItemText(IDC_FIRST_NAME_BOX, CString(rs->Fields->Item["First"]->Value.bstrVal));
			SetDlgItemText(IDC_MIDDLE_NAME_BOX, CString(rs->Fields->Item["Middle"]->Value.bstrVal));
			SetDlgItemText(IDC_LAST_NAME_BOX, CString(rs->Fields->Item["Last"]->Value.bstrVal));
			SetDlgItemText(IDC_ADDRESS1_BOX, CString(rs->Fields->Item["Address1"]->Value.bstrVal));
			SetDlgItemText(IDC_ADDRESS2_BOX, CString(rs->Fields->Item["Address2"]->Value.bstrVal));
			SetDlgItemText(IDC_CITY_BOX, CString(rs->Fields->Item["City"]->Value.bstrVal));
			SetDlgItemText(IDC_STATE_BOX, CString(rs->Fields->Item["State"]->Value.bstrVal));
			SetDlgItemText(IDC_ZIP_BOX, CString(rs->Fields->Item["Zip"]->Value.bstrVal));
			SetDlgItemText(IDC_WORK_PHONE_BOX, CString(rs->Fields->Item["WorkPhone"]->Value.bstrVal));
			SetDlgItemText(IDC_EXT_PHONE_BOX, CString(rs->Fields->Item["Extension"]->Value.bstrVal));
			SetDlgItemText(IDC_FAX_PHONE_BOX, CString(rs->Fields->Item["Fax"]->Value.bstrVal));
			SetDlgItemText(IDC_BACKLINE_BOX, CString(rs->Fields->Item["EmergWPhone"]->Value.bstrVal));
			SetDlgItemText(IDC_CELL_PHONE_BOX, CString(rs->Fields->Item["CellPhone"]->Value.bstrVal));
			SetDlgItemText(IDC_PAGER_PHONE_BOX, CString(rs->Fields->Item["Pager"]->Value.bstrVal));
			SetDlgItemText(IDC_DOC_EMAIL_BOX, CString(rs->Fields->Item["Email"]->Value.bstrVal));
			SetDlgItemText(IDC_NOTES, CString(rs->Fields->Item["Note"]->Value.bstrVal));
			SetDlgItemText(IDC_TITLE_BOX, CString(rs->Fields->Item["Title"]->Value.bstrVal));
			SetDlgItemVar(IDC_REF_DATE, rs->Fields->Item["RefDate"]->Value);

			//(a.wilson 2011-4-28) PLID 43355	//discount - when selecting a different patient these queries will be 
			//triggered and get the new patients data.
			CString str;
			_variant_t tempVariant = rs->Fields->Item["TypeID"]->Value;

			if (tempVariant.vt == VT_NULL) {
				m_pDiscountType->CurSel = 0;
				SetDlgItemText(IDC_DISCOUNT_PERCENT_BOX, "0.00");
				SetDlgItemText(IDC_ADDON_DISCOUNT_PERCENT_BOX, "0.00");
			} else {
				m_pDiscountType->SetSelByColumn(0, rs->Fields->Item["TypeID"]->Value);
				str.Format("%.2f", VarDouble(rs->Fields->Item["DiscountPercent"]->Value));
				SetDlgItemText(IDC_DISCOUNT_PERCENT_BOX, str);
				str.Format("%.2f", VarDouble(rs->Fields->Item["AddOnDiscountPercent"]->Value));
				SetDlgItemText(IDC_ADDON_DISCOUNT_PERCENT_BOX, str);
			}
		}

		//set reference rating and reference status combos
		if(rs->Fields->Item["RefStatus"]->Value.vt == VT_BSTR)
			m_pRefStatus->SetSelByColumn(0, rs->Fields->Item["RefStatus"]->Value);
		else
			m_pRefStatus->CurSel = -1;

		if(rs->Fields->Item["RefRating"]->Value.vt == VT_BSTR)
			m_pRefRating->SetSelByColumn(0, rs->Fields->Item["RefRating"]->Value);
		else
			m_pRefRating->CurSel = -1;
		//

		rs->Close();

		rs = CreateRecordset("SELECT PracEmail FROM NxClientsT WHERE PersonID = %li", m_id);
		if(!rs->eof)
			SetDlgItemText(IDC_PRAC_EMAIL_BOX, CString(rs->Fields->Item["PracEmail"]->Value.bstrVal));
		else
			SetDlgItemText(IDC_PRAC_EMAIL_BOX, "");

		//custom fields
		rs = CreateRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 2", m_id);	//source
		if(!rs->eof)
			SetDlgItemText(IDC_SOURCE_BOX, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
		else
			SetDlgItemText(IDC_SOURCE_BOX, "");

		rs = CreateRecordset("SELECT Textparam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 1", m_id);	//caller
		if(!rs->eof)
			SetDlgItemText(IDC_CALLER_BOX, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
		else
			SetDlgItemText(IDC_CALLER_BOX, "");

		rs = CreateRecordset("SELECT Textparam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 3", m_id);	//website
		if(!rs->eof)
			SetDlgItemText(IDC_WEBSITE_BOX, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
		else
			SetDlgItemText(IDC_WEBSITE_BOX, "");


		//fill in the datalists
		//alt contact
		rs = CreateRecordset("SELECT IntParam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 31",m_id);
		if(!rs->eof)
			m_altContact->SetSelByColumn(0, rs->Fields->Item["IntParam"]->Value);
		else
			m_altContact->CurSel = 0;

		//status
		rs = CreateRecordset("SELECT TypeOfPatient FROM PatientsT WHERE PersonID = %li", m_id);
		if(!rs->eof)
			m_status->SetSelByColumn(0, rs->Fields->Item["TypeOfPatient"]->Value);
		else
			m_status->CurSel = 0;

		//society
		// (j.armen 2011-06-28 11:55) - PLID 44342 - moved into a seperate function
		RefreshSociety();		

	} NxCatchAll("Error in CSalesDlg::Load()");
}

// (j.armen 2011-06-28 11:56) - PLID 44342 - Function for refreshing the Society box/label
void CSalesDlg::RefreshSociety()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));

		// (j.armen 2011-06-28 11:57) - PLID 44342 - reset the box/label before filling it
		m_society->PutCurSel(-1);
		m_nxlSocietyLabel.SetText("");
		m_nxlSocietyLabel.ShowWindow(SW_HIDE);
		ShowDlgItem(IDC_SOCIETY_BOX, SW_SHOWNA);

		// (j.armen 2011-06-28 11:57) - PLID 44342 - Get data from the custom field
		rs = CreateParamRecordset(
			"SELECT CustomListDataT.PersonID, CustomListDataT.FieldID, CustomListDataT.CustomListItemsID, CustomListItemsT.Text, ItemCount \r\n"
			"FROM CustomListDataT \r\n"
			"INNER JOIN (SELECT PersonID, FieldID, COUNT(*) AS ItemCount FROM CustomListDataT GROUP BY PersonID, FieldID) CountQ \r\n"
			"	ON CustomListDataT.PersonID = CountQ.PersonID AND CustomListDataT.FieldID = CountQ.FieldID \r\n"
			"INNER JOIN CustomListItemsT ON CustomListDataT.CustomListItemsID = CustomListItemsT.ID \r\n"
			"WHERE CustomListDataT.PersonID = {INT} AND CustomListDataT.FieldID = 22 ORDER BY CustomListItemsT.Text;", m_id);
		while(!rs->eof) 
		{
			// (j.armen 2011-06-28 11:57) - PLID 44342 - If we have more than 1 item selected, show the label
			if(AdoFldLong(rs, "ItemCount", 0) > 1)
			{
				ShowDlgItem(IDC_SOCIETY_BOX, SW_HIDE);
				if(m_nxlSocietyLabel.GetText().IsEmpty())
				{
					m_nxlSocietyLabel.SetText(AdoFldString(rs, "Text"));
					m_nxlSocietyLabel.SetToolTip(AdoFldString(rs, "Text"));
				}
				else
				{
					m_nxlSocietyLabel.SetText(m_nxlSocietyLabel.GetText() + ", " + AdoFldString(rs, "Text"));
					m_nxlSocietyLabel.SetToolTip(m_nxlSocietyLabel.GetToolTip() + ", " + AdoFldString(rs, "Text"));
				}

				m_nxlSocietyLabel.SetSingleLine();
				m_nxlSocietyLabel.SetType(dtsHyperlink);
				m_nxlSocietyLabel.ShowWindow(SW_SHOWNA);
			}
			// (j.armen 2011-06-28 11:58) - PLID 44342 - If we have only one item selected, show the box
			else
			{
				m_nxlSocietyLabel.ShowWindow(SW_HIDE);
				m_society->SetSelByColumn(0, rs->Fields->Item["CustomListItemsID"]->Value);
				ShowDlgItem(IDC_SOCIETY_BOX, SW_SHOWNA);
			}
			rs->MoveNext();
		}
		m_changed = false;
		m_ForceRefresh = false;
	}
	NxCatchAll(__FUNCTION__);
}

// (j.armen 2011-06-28 11:59) - PLID 44342 - Store the society data
void CSalesDlg::StoreSociety(NXDATALISTLib::_DNxDataListPtr customList, UINT customListIDC)
{
	CParamSqlBatch sqlBatch;
	_RecordsetPtr rs;
	CArray<long, long> arynSelection;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "CustomListDataT.FieldID = 22");

	int nVal;
	switch(customListIDC)
	{
		// (j.armen 2011-06-28 11:59) - PLID 44342 - if we are coming from the box, then get the current selection from that box
		case IDC_SOCIETY_BOX:
			if(customList->GetCurSel() == -1)
			{
				nVal = -1;
			}
			else
			{
				nVal = VarLong(customList->GetValue(customList->GetCurSel(), 0));
			}
			break;
		case IDC_SOCIETY_LABEL:
			// (j.armen 2011-06-28 12:00) - PLID 44342 - if we are coming from the label, then we must show the multi select dlg
			nVal = -2;
			break;
		default:
			ASSERT(FALSE);
			break;
	}

	sqlBatch.Add("DELETE FROM CustomListDataT WHERE FieldID = 22 AND PersonID = {INT}; ", m_id);
	switch(nVal)
	{
		// (j.armen 2011-06-28 12:00) - PLID 44342 - Show the multiselect dlg
		case -2:
			rs = CreateParamRecordset(
			"SELECT CustomListItemsID FROM CustomListDataT "
			"WHERE FieldID = 22 AND PersonID = {INT}", m_id);
		
			while(!rs->eof) 
			{
				arynSelection.Add(AdoFldLong(rs, "CustomListItemsID"));
				rs->MoveNext();
			}

			dlg.PreSelect(arynSelection);
			if(IDOK == dlg.Open("CustomListItemsT", "CustomFieldID = 22", "ID", "Text", "Select Custom List Items", 0))
			{
				dlg.FillArrayWithIDs(arynSelection);
				for(int i = 0; i < arynSelection.GetCount(); i++)
				{
					sqlBatch.Add("INSERT INTO CustomListDataT (PersonID, FieldID, CustomListItemsID) VALUES({INT}, 22, {INT})", m_id, arynSelection.GetAt(i));
				}
				sqlBatch.Execute(GetRemoteData());
			}
			break;
		// (j.armen 2011-06-28 12:00) - PLID 44342 - no selection, remove all data
		case -1:
			sqlBatch.Execute(GetRemoteData());
			break;
		// (j.armen 2011-06-28 12:01) - PLID 44342 - insert the single selected item
		default:
			sqlBatch.Add("INSERT INTO CustomListDataT (PersonID, FieldID, CustomListItemsID) VALUES({INT}, 22, {INT}); ", m_id, VarLong(customList->GetValue(customList->GetCurSel(), 0)));
			sqlBatch.Execute(GetRemoteData());
			break;
	}
	RefreshSociety();
}	

BOOL CSalesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	IRowSettingsPtr pRow;
	_variant_t var;

	m_btnCreateTodo.AutoSet(NXB_NEW);
	m_btnSelReferral.AutoSet(NXB_MODIFY);

	m_changed = FALSE;
	
	m_altContact = BindNxDataListCtrl(IDC_ALT_CONTACT_LIST);
	m_society = BindNxDataListCtrl(IDC_SOCIETY_BOX);
	m_status = BindNxDataListCtrl(IDC_STATUS);
	//(a.wilson 2011-4-28) PLID 43355 - pointer setup to control the datalist
	m_pDiscountType = BindNxDataListCtrl(IDC_DISCOUNT_TYPE);

	pRow = m_altContact->GetRow(-1);
	var = (long)0;
	pRow->PutValue(0,var);
	pRow->PutValue(1,"<No Contact>");
	m_altContact->InsertRow(pRow,0);
	
	pRow = m_society->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,"<No Society>");
	m_society->InsertRow(pRow,0);

	// (j.armen 2011-06-28 12:01) - PLID 44342 - added entry for multiple societies
	pRow = m_society->GetRow(-1);
	pRow->PutValue(0,(long)-2);
	pRow->PutValue(1, "<Multiple Societies>");
	m_society->InsertRow(pRow, 0);

	pRow = m_status->GetRow(-1);
	var = (long)0;
	pRow->PutValue(0,var);
	pRow->PutValue(1,"<No Status>");
	m_status->InsertRow(pRow,0);

	//fill in the ratings combo
	m_pRefRating = BindNxDataListCtrl(IDC_REF_RATING, false);

	pRow = m_pRefRating->GetRow(-1);
	pRow->PutValue(0,"<No Rating>");
	m_pRefRating->InsertRow(pRow,0);
	pRow = m_pRefRating->GetRow(-1);
	pRow->PutValue(0,"F");
	m_pRefRating->InsertRow(pRow,0);
	pRow = m_pRefRating->GetRow(-1);
	pRow->PutValue(0,"D");
	m_pRefRating->InsertRow(pRow,0);
	pRow = m_pRefRating->GetRow(-1);
	pRow->PutValue(0,"C");
	m_pRefRating->InsertRow(pRow,0);
	pRow = m_pRefRating->GetRow(-1);
	pRow->PutValue(0,"B");
	m_pRefRating->InsertRow(pRow,0);
	pRow = m_pRefRating->GetRow(-1);
	pRow->PutValue(0,"A");
	m_pRefRating->InsertRow(pRow,0);
	//

	//fill in the ratings combo
	m_pRefStatus = BindNxDataListCtrl(IDC_REF_STATUS, false);

	//TODO:  What should these be?
	pRow = m_pRefStatus->GetRow(-1);
	pRow->PutValue(0,"<No Status>");
	m_pRefStatus->InsertRow(pRow,0);
	pRow = m_pRefStatus->GetRow(-1);
	pRow->PutValue(0,"Power User");
	m_pRefStatus->InsertRow(pRow,0);
	pRow = m_pRefStatus->GetRow(-1);
	pRow->PutValue(0,"Normal User");
	m_pRefStatus->InsertRow(pRow,0);
	//

	//a.wilson 2011-4-29 PLID 43355 - add the no discount option to the discount type control and the add on discount type control
	pRow = m_pDiscountType->GetRow(-1);
	pRow->PutValue(0, 0);
	pRow->PutValue(1,"<No Discount>");
	m_pDiscountType->InsertRow(pRow,0);
	
	//(a.wilson 2011-5-2) PLID 43355
	//check permissions to decide whether the user is allowed to edit these controls
	if(!GetCurrentUserPermissions(bioInternalDiscount)) {
		m_neditDiscountPercent.SetReadOnly(TRUE);
		m_neditAddOnDiscountPercent.SetReadOnly(TRUE);
		m_pDiscountType->ReadOnly = TRUE;
	} else {
		m_neditDiscountPercent.SetReadOnly(FALSE);
		m_neditAddOnDiscountPercent.SetReadOnly(FALSE);
		m_pDiscountType->ReadOnly = FALSE;
	}

	m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
	m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CSalesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(SalesDlg)
	ON_EVENT(CSalesDlg, IDC_ALT_CONTACT_LIST, 2 /* SelChanged */, OnSelChangedAltContactList, VTS_I4)
	ON_EVENT(CSalesDlg, IDC_STATUS, 2 /* SelChanged */, OnSelChangedStatus, VTS_I4)
	ON_EVENT(CSalesDlg, IDC_SOCIETY_BOX, 2 /* SelChanged */, OnSelChangedSocietyBox, VTS_I4)
	ON_EVENT(CSalesDlg, IDC_REF_RATING, 16 /* SelChosen */, OnSelChosenRefRating, VTS_I4)
	ON_EVENT(CSalesDlg, IDC_REF_STATUS, 16 /* SelChosen */, OnSelChosenRefStatus, VTS_I4)
	//(a.wilson 2011-4-28) PLID 43355 - Messages to handle the combo boxes for discount types.
	ON_EVENT(CSalesDlg, IDC_DISCOUNT_TYPE, 16 /* SelChosen */, OnSelChosenDiscountType, VTS_I4)
	ON_EVENT(CSalesDlg, IDC_DISCOUNT_TYPE, 1 /* SelChanging */, OnSelChangingDiscountType, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSalesDlg::OnSelChangedAltContactList(long nNewSel) 
{
	try{
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT IntParam FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 31", m_id);
		if(rs->eof) { //no record
			//check CustomFieldsT
			if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 31"))
				ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (31, 'Custom Contact', 12)");

			ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, IntParam) (SELECT %li, 31, %li)", m_id, VarLong(m_altContact->GetValue(nNewSel, 0)));
		}
		else	//update existing record
			ExecuteSql("UPDATE CustomFieldDataT SET IntParam = %li WHERE PersonID = %li AND FieldID = 31", VarLong(m_altContact->GetValue(nNewSel, 0)), m_id);
	} NxCatchAll("Error in OnSelChangedAltContactList()");
}

void CSalesDlg::OnSelChangedStatus(long nNewSel) 
{
	try{
		ExecuteSql("UPDATE PatientsT SET TypeOfPatient = %li WHERE PersonID = %li", VarLong(m_status->GetValue(nNewSel, 0)), m_id);
	} NxCatchAll("Error in OnSelChangedStatus()");
}

void CSalesDlg::OnSelChangedSocietyBox(long nNewSel) 
{
	try{
		// (j.armen 2011-06-28 12:02) - PLID 44342 - now call the store society function
		StoreSociety(m_society, IDC_SOCIETY_BOX);
	} NxCatchAll(__FUNCTION__);
}

void CSalesDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	StoreDetails();

	m_id = GetActivePatientID();

	//DRT - 12/03/02 - Copied this code from the support dialog (including the below comments), because at some point we started
	//				reading things out of NxClientsT on this tab as well.

	//just for this tab we're going to do things a little differently
	//when this tab is loaded it checks to see if an entry exists in NxClientsT (which contains a PersonID to 
	//link with PersonT).  If nothing exists, this function will add an entry with the default values to it
	//This saves us changing a bunch of code elsewhere in the program and conflicts with actual Clients who 
	//don't have a Support tab.
	//If an entry does exist, nothing will happen and the dialog will act as it normally would
	_RecordsetPtr rs(__uuidof(Recordset));
	CString sql;
	sql.Format("SELECT PersonID FROM NxClientsT WHERE PersonID = %li", m_id);
	try{

		rs = CreateRecordsetStd(sql);

		if(rs->eof)
		{
			ExecuteSql("INSERT INTO NxClientsT (PersonID, LicenseKey) (SELECT %li,%li)", m_id, NewNumber("NxClientsT","LicenseKey"));
		}

		rs->Close();
	} NxCatchAll("Error in CSupportDlg::UpdateView()");

	Load();
}

void CSalesDlg::StoreDetails()
{
	try
	{
		if (m_changed)
		{	Save (GetFocus()->GetDlgCtrlID());
			m_changed = false;
		}
	}NxCatchAll("Error in StoreDetails");
}

BOOL CSalesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	CString str;

	switch (HIWORD(wParam))
	{
		case EN_CHANGE:
			switch(nID = LOWORD(wParam))
			{
			case IDC_WORK_PHONE_BOX:
			case IDC_FAX_PHONE_BOX:
			case IDC_BACKLINE_BOX:
			case IDC_CELL_PHONE_BOX:
			case IDC_PAGER_PHONE_BOX:
				GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							FormatItem (nID, m_strPhoneFormat);
						}
					}
					break;
				break;
			case IDC_EXT_PHONE_BOX: 
				GetDlgItemText(nID, str);
				str.TrimRight();
				if (str != "")
					FormatItem (nID, "nnnnnnn");
				break;
			case IDC_ZIP_BOX:
				// (d.moore 2007-04-23 12:11) - PLID 23118 - 
				//  Capitalize letters in the zip code as they are typed in. Canadian postal
				//    codes need to be formatted this way.
				CapitalizeAll(IDC_ZIP_BOX);
				break;
			}
			m_changed = true;
			break;

		case EN_KILLFOCUS:
			Save(LOWORD(wParam));
		break;

		default:
			break;
	}
	
	return CPatientDialog::OnCommand(wParam, lParam);
}

void CSalesDlg::OnSelChosenRefRating(long nNewSel) {

	//save the ref rating
	long nCurSel = m_pRefRating->GetCurSel();
	if(nCurSel == -1)
		return;

	try {
		CString strRefRating = CString(m_pRefRating->GetValue(nNewSel, 0).bstrVal);
		//we don't want to save "<No Rating>", so we'll just toss in an empty string
		if(strRefRating == "<No Rating>")
			strRefRating = "";

		ExecuteSql("UPDATE NxClientsT SET RefRating = '%s' WHERE PersonID = %li", strRefRating, m_id);

	} NxCatchAll("Error saving reference rating.");
}

void CSalesDlg::OnSelChosenRefStatus(long nNewSel) {
	
	//save the rating
	long nCurSel = m_pRefStatus->GetCurSel();
	if(nCurSel == -1)
		return;

	try {
		CString strStatus = CString(m_pRefStatus->GetValue(nNewSel, 0).bstrVal);
		//we don't want to save "<No Rating>", so we'll just toss in an empty string
		if(strStatus == "<No Status>")
			strStatus = "";

		ExecuteSql("UPDATE NxClientsT SET RefStatus = '%s' WHERE PersonID = %li", strStatus, m_id);

	} NxCatchAll("Error saving reference status.");
}

void CSalesDlg::OnEmail() {

	CString str;
	GetDlgItemText(IDC_DOC_EMAIL_BOX, str);
	str.TrimRight();
	str.TrimLeft();
	if (str != "") {
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		if ((int)ShellExecute(GetSafeHwnd(), NULL, "mailto:" + str, 
			NULL, "", SW_MAXIMIZE) < 32)
			AfxMessageBox("Could not e-mail patient");
	}
	else
		AfxMessageBox("Please enter an e-mail address.");

}

void CSalesDlg::OnEmail2() {

	CString str;
	GetDlgItemText(IDC_PRAC_EMAIL_BOX, str);
	str.TrimRight();
	str.TrimLeft();
	if (str != "") {
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		if ((int)ShellExecute(GetSafeHwnd(), NULL, "mailto:" + str, 
			NULL, "", SW_MAXIMIZE) < 32)
			AfxMessageBox("Could not e-mail patient");
	}
	else
		AfxMessageBox("Please enter an e-mail address.");

}

void CSalesDlg::OnViewTodo() {

	CMainFrame *pMainFrame = GetMainFrame();
	if (pMainFrame->GetSafeHwnd()) {
		pMainFrame->ShowTodoList();
	}
}

void CSalesDlg::OnCreateTodo() {

	//DRT 3/4/03 - copied this from followupdlg.cpp, removed the code that adds a row

	/* Create a new todo from the task edit dialog */
	// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
	//to create a todo alarm
	/*
	if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
		return;
	*/

	try {
		CTaskEditDlg dlg(this);
		dlg.m_nPersonID = GetActivePatientID(); // (a.walling 2008-07-07 17:54) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields 
		dlg.m_bIsNew = TRUE;

		long nResult = dlg.DoModal();

		if (nResult != IDOK) return;

		// Added by CH 1/17: Force the next remind
		// time to be 5 minutes if a new task is
		// added so the "Don't remind me again"
		// option will not cause the new task to
		// be forgotten.
		{
			COleDateTime dt = COleDateTime::GetCurrentTime();
			dt += COleDateTimeSpan(0,0,5,0);
			SetPropertyDateTime("TodoTimer", dt);
			// (j.dinatale 2012-10-22 17:59) - PLID 52393 - set our user preference
			SetRemotePropertyInt("LastTimeOption_User", 5, 0, GetCurrentUserName());
			SetTimer(IDT_TODO_TIMER, 5*60*1000, NULL);
		}

	}NxCatchAll("Error in CSalesDlg::OnCreateTodo() ");	

}

//DRT 5/24/2007 - PLID 25892 - Open the opportunities window.  This is managed by the mainfrm.
void CSalesDlg::OnViewOpportunities()
{
	try {
		GetMainFrame()->ShowOpportunityList();

	} NxCatchAll("Error in OnViewOpportunities");
}
//(a.wilson 2011-4-28) PLID 43355 - when the user selects a new discount type from the control we need
//to update the database tables
void CSalesDlg::OnSelChosenDiscountType(long nNewSel)
{
	try {
		long nSelection = VarLong(m_pDiscountType->GetValue(nNewSel, 0));

		if (nSelection == 0) {	//if they set the type to no discount then we want to remove them from the table.
			SetDlgItemText(IDC_DISCOUNT_PERCENT_BOX, "0.00");
			SetDlgItemText(IDC_ADDON_DISCOUNT_PERCENT_BOX, "0.00");
			ExecuteParamSql("DELETE FROM SupportDiscountsT WHERE SupportDiscountsT.PersonID = {INT}", m_id);
		} else {	//update their table information
			CString str;
			double temp, tempAddOn;

			GetDlgItemText(IDC_DISCOUNT_PERCENT_BOX, str);
			temp = atof(str);
			GetDlgItemText(IDC_ADDON_DISCOUNT_PERCENT_BOX, str);
			tempAddOn = atof(str);
			ExecuteParamSql("UPDATE SupportDiscountsT SET SupportDiscountsT.TypeID = {INT}, SupportDiscountsT.DiscountPercent = {DOUBLE}, "
				"SupportDiscountsT.AddOnDiscountPercent = {DOUBLE} WHERE SupportDiscountsT.PersonID = {INT} \r\n "	//update
				"IF @@ROWCOUNT = 0 \r\n "	//if they aren't in the list
				"INSERT INTO SupportDiscountsT (TypeID, PersonID, DiscountPercent, Note, Inactive, AddOnDiscountPercent) "
				"VALUES ({INT}, {INT}, {DOUBLE}, '', 0, {DOUBLE})", nSelection, temp, tempAddOn, m_id, nSelection, m_id, temp, tempAddOn);
		}
	} NxCatchAll("Error saving discount type.");
}
// (a.wilson 2011-4-6) PLID 43355 - to prevent you from selecting the small space in the drop down which could cause errors.
void CSalesDlg::OnSelChangingDiscountType(long FAR* nRow)
{
	try {
		if (*nRow==-1)
		{
			*nRow=m_pDiscountType->CurSel;
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2011-5-6) PLID 9702 - when you click the visit button you will be sent to the clients website.
void CSalesDlg::OnBnClickedVisitWebsite()
{
	try {
		CString strTemp;
		GetDlgItemText(IDC_WEBSITE_BOX, strTemp);	//get the url for the site.

		if (strTemp != "") {
			//executes the url and opens the website in internet explorer.
			::ShellExecute(m_hWnd, _T("open"), strTemp, NULL, NULL, SW_SHOWNORMAL);
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2011-06-28 12:02) - PLID 44342 - event handling for when label is clicked
LRESULT CSalesDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) 
		{
			case IDC_SOCIETY_LABEL:	
				StoreSociety(m_society, IDC_SOCIETY_LABEL);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.armen 2011-06-28 12:02) - PLID 44342- event handling for when mouse over label
BOOL CSalesDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if(m_nxlSocietyLabel.IsWindowVisible() && m_nxlSocietyLabel.IsWindowEnabled())
		{
			m_nxlSocietyLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

