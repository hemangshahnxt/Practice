// NewVisitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "NewVisitDlg.h"
#include "EditVisitTypeDlg.h"
#include "EditVisionRatingsDlg.h"
#include "DateTimeUtils.h"

//m.hancock - 6-29-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
#include "EditComboBox.h"
#include "MultiSelectDlg.h"
#include "GlobalDrawingUtils.h"

#include "SelectTestTypeDlg.h"

using namespace ADODB;
using namespace NXDATALISTLib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewVisitDlg dialog


CNewVisitDlg::CNewVisitDlg(CWnd* pParent)
	: CNxDialog(CNewVisitDlg::IDD, pParent)
	, m_LeftEyeDrawDlg(*(new CEyeDrawDlg(this)))
	, m_RightEyeDrawDlg(*(new CEyeDrawDlg(this)))
	, m_nLeftVisitID(-1)
	, m_nRightVisitID(-1)
{

}

CNewVisitDlg::~CNewVisitDlg()
{
	m_LeftEyeDrawDlg.DestroyWindow();
	m_RightEyeDrawDlg.DestroyWindow();
	delete (&m_LeftEyeDrawDlg);
	delete (&m_RightEyeDrawDlg);
}

void CNewVisitDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewVisitDlg)
	DDX_Control(pDX, IDC_VISIT_DATE, m_dtPicker);
	DDX_Control(pDX, IDC_CUSTOM_NV_1, m_nxeditCustomNv1);
	DDX_Control(pDX, IDC_CUSTOM_NV_2, m_nxeditCustomNv2);
	DDX_Control(pDX, IDC_CUSTOM_NV_3, m_nxeditCustomNv3);
	DDX_Control(pDX, IDC_CUSTOM_NV_4, m_nxeditCustomNv4);
	DDX_Control(pDX, IDC_VISIT_NOTES, m_nxeditVisitNotes);
	DDX_Control(pDX, IDC_LEFT_EYE_DRAW, m_nxstaticLeftEyeDraw);
	DDX_Control(pDX, IDC_RIGHT_EYE_DRAW, m_nxstaticRightEyeDraw);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST1, m_nxstaticCustlist1);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST2, m_nxstaticCustlist2);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST3, m_nxstaticCustlist3);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST4, m_nxstaticCustlist4);
	DDX_Control(pDX, IDC_ADD_TEST_BUTTON, m_btnAddTest);
	DDX_Control(pDX, IDC_DELETE_TEST_BUTTON, m_btnDeleteTest);
	DDX_Control(pDX, IDC_SAVE_BUTTON, m_btnSave);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CUSTOM_LABEL_1, m_nxstaticCustomLabel1);
	DDX_Control(pDX, IDC_CUSTOM_LABEL_2, m_nxstaticCustomLabel2);
	DDX_Control(pDX, IDC_CUSTOM_LABEL_3, m_nxstaticCustomLabel3);
	DDX_Control(pDX, IDC_CUSTOM_LABEL_4, m_nxstaticCustomLabel4);
	DDX_Control(pDX, IDC_CUSTOMLIST_LABEL_1, m_nxstaticCustomlistLabel1);
	DDX_Control(pDX, IDC_CUSTOMLIST_LABEL_2, m_nxstaticCustomlistLabel2);
	DDX_Control(pDX, IDC_CUSTOMLIST_LABEL_3, m_nxstaticCustomlistLabel3);
	DDX_Control(pDX, IDC_CUSTOMLIST_LABEL_4, m_nxstaticCustomlistLabel4);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewVisitDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNewVisitDlg)
	ON_BN_CLICKED(IDC_SAVE_BUTTON, OnSaveButton)
	ON_BN_CLICKED(IDC_ADD_VISIT_TYPE, OnAddVisitType)
	ON_BN_CLICKED(IDC_BTN_EDIT_RATINGS, OnBtnEditRatings)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_1, OnEditCustomList1)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_2, OnEditCustomList2)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_3, OnEditCustomList3)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_4, OnEditCustomList4)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_ADD_TEST_BUTTON, OnAddTestButton)
	ON_BN_CLICKED(IDC_DELETE_TEST_BUTTON, OnDeleteTestButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewVisitDlg message handlers

BEGIN_EVENTSINK_MAP(CNewVisitDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewVisitDlg)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST1, 18 /* RequeryFinished */, OnRequeryFinishedCustlist1, VTS_I2)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST2, 18 /* RequeryFinished */, OnRequeryFinishedCustlist2, VTS_I2)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST3, 18 /* RequeryFinished */, OnRequeryFinishedCustlist3, VTS_I2)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST4, 18 /* RequeryFinished */, OnRequeryFinishedCustlist4, VTS_I2)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST1, 16 /* SelChosen */, OnSelChosenCustlist1, VTS_I4)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST2, 16 /* SelChosen */, OnSelChosenCustlist2, VTS_I4)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST3, 16 /* SelChosen */, OnSelChosenCustlist3, VTS_I4)
	ON_EVENT(CNewVisitDlg, IDC_CUSTLIST4, 16 /* SelChosen */, OnSelChosenCustlist4, VTS_I4)
	ON_EVENT(CNewVisitDlg, IDC_TESTS_LIST, 9 /* EditingFinishing */, OnEditingFinishingTestsList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNewVisitDlg::OnSaveButton() 
{
	//save the settings they've entered
	CString leftSph, leftCyl, leftAxis, leftBCVA, leftVA, rightSph, rightCyl, rightAxis, rightBCVA, rightVA, notes, date, custom1, custom2, custom3, custom4;
	long nVisitType = -1;
	long nRatingType = -1;

	//visit type
	if(m_visitTypeCombo->GetCurSel() > -1) {
		nVisitType = VarLong(m_visitTypeCombo->GetValue(m_visitTypeCombo->GetCurSel(), 0));
	}
	else {
		AfxMessageBox("You must choose a visit type.  Please select an item from the type dropdown.");
		return;
	}

	//rating type
	if(m_ratingCombo->GetCurSel() > -1) {
		nRatingType = VarLong(m_ratingCombo->GetValue(m_ratingCombo->GetCurSel(), 0));
	}

	//eye tests
	//m.hancock - 10-25-05 - PLID 16756 - It has become necessary to only allow users to save visits which have associated
	//eye measurements since the visit dialog shows visits based upon the eye measurements.
	if(m_TestsList->GetRowCount() < 1)
	{
		AfxMessageBox("You must enter measurements for at least one eye before saving.");
		return;
	}

	try {

		GetDlgItemText(IDC_CUSTOM_NV_1, custom1);
		GetDlgItemText(IDC_CUSTOM_NV_2, custom2);
		GetDlgItemText(IDC_CUSTOM_NV_3, custom3);
		GetDlgItemText(IDC_CUSTOM_NV_4, custom4);

		GetDlgItemText(IDC_VISIT_NOTES, notes);
		notes = notes.Left(1000);	//trim it so it fits in the field

		//date
		COleDateTime dt;
		dt = m_dtPicker.GetValue();
		date = FormatDateTimeForSql(dt, dtoDate);

		//m.hancock 8/22/05 - PLID 16756 - I changed the saving of data for EyeVisitsT since some of the fields have been moved
		//to the EyeTestsT table to handle the new advanced tests.  Below is the code to update or insert visit records.		
		//m.hancock - 10-26-05 - PLID 16756 - Needed to redesign database commit code
		
		//*Validate test data
		bool bValid = false;
		for(int x=0; x < m_TestsList->GetRowCount(); x++) //For each row of the tests list
		{
			//Get the test type id
			_variant_t var = m_TestsList->GetValue(x, 2);
			if(var.vt == VT_I4)
			{
				long nTestID = VarLong(var);
				//Check if the test type is valid
				if(ValidTestSelection(nTestID))
				{
					//Check if all data has been entered
					if(!IsTestDataEmpty(x))
						bValid = true;
				}
				else
				{
					//Some test types were invalid, so set them as blank
					IRowSettingsPtr pRow = m_TestsList->GetRow(x);
					pRow->PutValue(2, ""); //TestID
				}
			}
		}
		if(!bValid) //Display a message stating the save failed due to invalid types or incomplete data
		{
			MessageBox("Some of the tests have invalid test types or incomplete data.  Please correct them before saving.");
			return;
		}

		//*Generate sql for EyeVisitsT record
		CString strStatement = "";
		CString strSqlQueue = "";
		if(m_nLeftVisitID > -1) //This is an existing visit and just need to update the record
		{
			strStatement.Format("UPDATE EyeVisitsT SET VisitType = %li, Notes = '%s', "
				"Date = '%s', RatingType = %li, Custom1 = '%s', Custom2 = '%s', Custom3 = '%s', Custom4 = '%s' "
				"WHERE ID = %li;\r\n", nVisitType, _Q(notes), date, nRatingType, _Q(custom1), _Q(custom2), _Q(custom3), _Q(custom4), m_nLeftVisitID);
			strSqlQueue += strStatement;
		}
		else //This is a new visit, so we need to insert.
		{
			m_nLeftVisitID = NewNumber("EyeVisitsT", "ID");
			strStatement.Format("INSERT INTO EyeVisitsT (ID, EyeProcedureID, VisitType, Notes, Date, RatingType, Custom1, Custom2, Custom3, Custom4) "
				"VALUES (%li, %li, %li, '%s', '%s', %li, '%s', '%s', '%s', '%s');\r\n", m_nLeftVisitID, m_nProcID, nVisitType, _Q(notes), date, 
				nRatingType, _Q(custom1), _Q(custom2), _Q(custom3), _Q(custom4));
			strSqlQueue += strStatement;
		}

		//*Generate sql for tests
		for(x=0; x < m_TestsList->GetRowCount(); x++) //For each row of the tests list
		{
			//Get values for insertion
			long nID = VarLong(m_TestsList->GetValue(x, 0)); //ID
			long nVisitID = VarLong(m_TestsList->GetValue(x, 1)); //VisitID
			if(nVisitID == -1) //If -1, this was a new visit, so we need to store with the left visit id
				nVisitID = m_nLeftVisitID;
			long nTestID = VarLong(m_TestsList->GetValue(x, 2));//TestID
			long nEyeType = VarLong(m_TestsList->GetValue(x, 3)); //EyeType
			double nSphere = VarDouble(m_TestsList->GetValue(x, 6)); //Sphere
			double nCyl = VarDouble(m_TestsList->GetValue(x, 7)); //Cyl
			long nAxis = VarLong(m_TestsList->GetValue(x, 8)); //Axis

			//Format the VA and BCVA for storage in database
			//For the VA and BCVA, we need to cut the 20/ off and store the right of the /.
			long nVA, nBCVA;
			_variant_t var = m_TestsList->GetValue(x, 4); //VA
			if(var.vt == VT_BSTR) //Is a string
			{
				CString strTemp = VarString(var);
				nVA = atol(strTemp.Right(strTemp.GetLength() - 3)); //Chop off the "20/" and convert to long
			}
			var = m_TestsList->GetValue(x, 5); //BCVA
			if(var.vt == VT_BSTR) //Is a string
			{
				CString strTemp = VarString(var);
				nBCVA = atol(strTemp.Right(strTemp.GetLength() - 3)); //Chop off the "20/" and convert to long
			}

			//Prepare the sql statement
			strStatement.Empty();
			//Check if this is a new record and if it is, we need to insert
			if(nID == -1)
			{
				//Get a new ID for the test; We need to add x because previous records have not been stored, thus NewNumber will return the same number
				nID = NewNumber("EyeTestsT", "ID") + x;

				//Update the row in the tests data list
				IRowSettingsPtr pRow = m_TestsList->GetRow(x);
				pRow->PutValue(0, nID);

				//Prepare the insert statement
				strStatement.Format("INSERT INTO EyeTestsT VALUES (%li, %li, %li, %li, %f, %f, %li, %li, %li);\r\n", nID, nVisitID, 
					nTestID, nEyeType, nSphere, nCyl, nAxis, nVA, nBCVA);
				strSqlQueue += strStatement;
			}
			else //This is an existing record, so we need to update
			{
				//Prepare the update statement
				strStatement.Format("UPDATE EyeTestsT SET TestID = %li, EyeType = %li, Sphere = %f, Cyl = %f, Axis = %li, "
					"VA = %li, BCVA = %li WHERE ID = %li;\r\n", nTestID, nEyeType, nSphere, nCyl, nAxis, nVA, nBCVA, nID);
				strSqlQueue += strStatement;
			}
		}

		//*Generate sql for custom lists
		//m.hancock - 7-8-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		//Delete any existing records
		//m.hancock 8/24/05 - PLID 16756 - I had to change this because only the leftvisitid is active now
		strStatement.Empty();
		strStatement.Format("DELETE FROM EyeVisitsListDataT WHERE VisitID = %li;\r\n", m_nLeftVisitID);
		strSqlQueue += strStatement;
		//Prepare the insert statements for the custom lists
		strStatement = GetCustomListInsertStmt(&m_adwCustomList1)
					 + GetCustomListInsertStmt(&m_adwCustomList2)
					 + GetCustomListInsertStmt(&m_adwCustomList3)
					 + GetCustomListInsertStmt(&m_adwCustomList4);
		strSqlQueue += strStatement;

		//*Commit the data in one lump sql statement
		if(!strSqlQueue.IsEmpty())
			ExecuteSql("%s", strSqlQueue);

		//*Save the eye sketches
		//m.hancock 8/24/05 - PLID 16756 - I had to change this because only the leftvisitid is active now
		if(m_nLeftVisitID != -1) {
			//Original save
			//m_LeftEyeDrawDlg.m_strFileName.Format("%s\\Refractive\\EyeVisit%li.nxd",GetSharedPath(),m_nLeftVisitID);
			m_LeftEyeDrawDlg.m_strFileName.Format("%s\\Refractive\\EyeVisit%li%s.nxd",GetSharedPath(),m_nLeftVisitID, "L");
			m_LeftEyeDrawDlg.Save();
			m_RightEyeDrawDlg.m_strFileName.Format("%s\\Refractive\\EyeVisit%li%s.nxd",GetSharedPath(),m_nLeftVisitID, "R");
			m_RightEyeDrawDlg.Save();
		}

	} NxCatchAll("Error saving information.");
	
	EndDialog(1);
}

BOOL CNewVisitDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnAddTest.AutoSet(NXB_NEW);
	m_btnDeleteTest.AutoSet(NXB_DELETE);
	m_btnSave.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_visitTypeCombo = BindNxDataListCtrl(IDC_VISIT_TYPE_COMBO);
	m_ratingCombo = BindNxDataListCtrl(IDC_VISION_RATING_COMBO);

	//embed the Eye Draw dialogs
	if (m_LeftEyeDrawDlg.GetSafeHwnd() == NULL) {
		m_LeftEyeDrawDlg.Create(IDD_EYE_DRAW_DLG, this);
		CRect rect;
		GetDlgItem(IDC_LEFT_EYE_DRAW)->GetWindowRect(rect);
		ScreenToClient(&rect);
		m_LeftEyeDrawDlg.MoveWindow(rect);
		m_LeftEyeDrawDlg.ShowWindow(SW_SHOW);		
	}

	if (m_RightEyeDrawDlg.GetSafeHwnd() == NULL) {
		m_RightEyeDrawDlg.Create(IDD_EYE_DRAW_DLG, this);
		CRect rect;
		GetDlgItem(IDC_RIGHT_EYE_DRAW)->GetWindowRect(rect);
		ScreenToClient(&rect);
		m_RightEyeDrawDlg.MoveWindow(rect);
		m_RightEyeDrawDlg.ShowWindow(SW_SHOW);		
	}

	if(IsRecordsetEmpty("SELECT ID FROM EyeVisitsT WHERE ID = %li",(m_nLeftVisitID != -1 ? m_nLeftVisitID : m_nRightVisitID)))
		m_bIsNew = TRUE;
	else
		m_bIsNew = FALSE;

	//set custom labels
	_RecordsetPtr CustomLabelsSet = CreateRecordset("SELECT * FROM CustomFieldsT WHERE ID >= 59 AND ID <= 62");
	while(!CustomLabelsSet->eof) {
		CString WhatShowsInPat = CString(CustomLabelsSet->Fields->Item["Name"]->Value.bstrVal);
		switch(CustomLabelsSet->Fields->Item["ID"]->Value.lVal) {
		case 59:
			SetDlgItemText(IDC_CUSTOM_LABEL_1, ConvertToControlText(WhatShowsInPat));
			break;
		case 60:
			SetDlgItemText(IDC_CUSTOM_LABEL_2, ConvertToControlText(WhatShowsInPat));
			break;
		case 61:
			SetDlgItemText(IDC_CUSTOM_LABEL_3, ConvertToControlText(WhatShowsInPat));
			break;
		case 62:
			SetDlgItemText(IDC_CUSTOM_LABEL_4, ConvertToControlText(WhatShowsInPat));
			break;
		}
		CustomLabelsSet->MoveNext();
	}

	//m.hancock - 7-11-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	//Load values for custom list labels
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_1, ConvertToControlText(GetRemotePropertyText("VisitsCustomListLabel1", "Custom List 1", 0, "<None>", false)));
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_2, ConvertToControlText(GetRemotePropertyText("VisitsCustomListLabel2", "Custom List 2", 0, "<None>", false)));
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_3, ConvertToControlText(GetRemotePropertyText("VisitsCustomListLabel3", "Custom List 3", 0, "<None>", false)));
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_4, ConvertToControlText(GetRemotePropertyText("VisitsCustomListLabel4", "Custom List 4", 0, "<None>", false)));

	//m.hancock - 6-29-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	m_custom1Combo = BindNxDataListCtrl(IDC_CUSTLIST1, true);
	m_custom2Combo = BindNxDataListCtrl(IDC_CUSTLIST2, true);
	m_custom3Combo = BindNxDataListCtrl(IDC_CUSTLIST3, true);
	m_custom4Combo = BindNxDataListCtrl(IDC_CUSTLIST4, true);
	GetDlgItem(IDC_STATIC_CUSTLIST1)->GetWindowRect(m_rcCustom1);
	GetDlgItem(IDC_STATIC_CUSTLIST1)->ShowWindow(SW_HIDE);
	ScreenToClient(&m_rcCustom1);
	GetDlgItem(IDC_STATIC_CUSTLIST2)->GetWindowRect(m_rcCustom2);
	GetDlgItem(IDC_STATIC_CUSTLIST2)->ShowWindow(SW_HIDE);
	ScreenToClient(&m_rcCustom2);
	GetDlgItem(IDC_STATIC_CUSTLIST3)->GetWindowRect(m_rcCustom3);
	GetDlgItem(IDC_STATIC_CUSTLIST3)->ShowWindow(SW_HIDE);
	ScreenToClient(&m_rcCustom3);
	GetDlgItem(IDC_STATIC_CUSTLIST4)->GetWindowRect(m_rcCustom4);
	GetDlgItem(IDC_STATIC_CUSTLIST4)->ShowWindow(SW_HIDE);
	ScreenToClient(&m_rcCustom4);

	Load();

	//m.hancock - PLID 16756 - Add advanced test types for eye vists
	m_TestsList = BindNxDataListCtrl(IDC_TESTS_LIST, false);
	
	//Set the from clause
	m_TestsList->FromClause = _bstr_t("EyeTestsT");
	
	if(m_nLeftVisitID == -1)
		m_TestsList->Clear(); //Nothing should be displayed
	else
	{
		//Set the where clause to display tests for this visit
		CString strWhere;
		strWhere.Format("VisitID = %li", m_nLeftVisitID);
		m_TestsList->WhereClause = _bstr_t(strWhere);
		m_TestsList->Requery();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewVisitDlg::OnAddVisitType() 
{
	//save old value
	_variant_t var;
	long CurSel = m_visitTypeCombo->GetCurSel();
	if(CurSel != -1)
		var = m_visitTypeCombo->GetValue(CurSel,0);

	CEditVisitTypeDlg dlg(this);
	dlg.DoModal();

	m_visitTypeCombo->Requery();

	//load old value
	if(CurSel != -1)
		m_visitTypeCombo->TrySetSelByColumn(0,var);
}

void CNewVisitDlg::OnCancel() 
{
	EndDialog(0);
	
	CNxDialog::OnCancel();
}

BOOL CNewVisitDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	CString str, field, value;

	try {

	switch (HIWORD(wParam))
	{
		case EN_KILLFOCUS:			

			switch ((nID = LOWORD(wParam))) {
				case IDC_CUSTOM_1:
					GetDlgItemText(nID,value);
					return CNxDialog::OnCommand(wParam, lParam);
					break;
				case IDC_CUSTOM_2:
					GetDlgItemText(nID,value);
					return CNxDialog::OnCommand(wParam, lParam);
					break;
				case IDC_CUSTOM_3:
					GetDlgItemText(nID,value);
					return CNxDialog::OnCommand(wParam, lParam);
					break;
				case IDC_CUSTOM_4:
					GetDlgItemText(nID,value);
					return CNxDialog::OnCommand(wParam, lParam);
					break;
			}

			//ExecuteSql("UPDATE EyeVisitsT SET %s = '%s' WHERE ID = %li", field, _Q(value), m_nCurrentID);

		break;
	}

	}NxCatchAll("Error saving data.");
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CNewVisitDlg::OnBtnEditRatings() 
{
	//save old value
	_variant_t var;
	long CurSel = m_ratingCombo->GetCurSel();
	if(CurSel != -1)
		var = m_ratingCombo->GetValue(CurSel,0);

	CEditVisionRatingsDlg dlg(this);
	dlg.DoModal();

	m_ratingCombo->Requery();

	//load old value
	if(CurSel != -1)
		m_ratingCombo->TrySetSelByColumn(0,var);
}

BOOL CNewVisitDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		if(ChangeCustomLabel(::GetDlgCtrlID(pMsg->hwnd)))
			return TRUE;
		break;
	default:
		break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

BOOL CNewVisitDlg::ChangeCustomLabel (int nID)
{
	
	int field = GetLabelFieldID(nID);

	if(field == 0)	//didn't click on a changable label
		return false;
	
	if (!UserPermission(CustomLabel))
		return false;
	CString strResult, strPrompt;
	GetDlgItemText(nID, strPrompt);
	strResult = ConvertFromControlText(strPrompt);

	_variant_t var;
	int nResult = InputBoxLimited(this, "Enter new name for " + strPrompt, strResult, "",50,false,false,NULL);

	strResult.TrimRight();
	strResult.TrimLeft();

	if (nResult == IDOK && strResult != "")
	{
		try {

			//m.hancock - 7-11-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
			//Store the string for the custom list label
			CString strPropName;
			switch(field)
			{
			case 54:
				strPropName = "VisitsCustomListLabel1";
				break;
			case 55:
				strPropName = "VisitsCustomListLabel2";
				break;
			case 56:
				strPropName = "VisitsCustomListLabel3";
				break;
			case 57:
				strPropName = "VisitsCustomListLabel4";
				break;
			default:
				{
					if(IsRecordsetEmpty("SELECT * FROM CustomFieldsT WHERE ID = %d", field)) {
						//need to insert this into the table
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) values (%d, '%s', 1)", field, _Q(strResult));
					}
					else {
						//just update what we've got
						ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %d", _Q(strResult), field);
					}
					// (b.cardillo 2006-05-19 17:28) - PLID 20735 - We know the new name for this label, so 
					// update the global cache so it won't have to be reloaded in its entirety.
					SetCustomFieldNameCachedValue(field, strResult);
					// (b.cardillo 2006-07-06 16:27) - PLID 20737 - Notify all the other users as well.
					CClient::RefreshTable_CustomFieldName(field, strResult);
				}
			}

			if(!strPropName.IsEmpty())
				SetRemotePropertyText(strPropName, strResult, 0, "<None>");

			SetDlgItemText(nID, ConvertToControlText(strResult));
			Invalidate();
		}NxCatchAll("Could not change " + strPrompt);
	}

	//success!
	return true;
}

int CNewVisitDlg::GetLabelFieldID(int nID)
{
	int field = 0;

	switch(nID)
	{
	case IDC_CUSTOM_LABEL_1:
		field = 59;
		break;
	case IDC_CUSTOM_LABEL_2:
		field = 60;
		break;
	case IDC_CUSTOM_LABEL_3:
		field = 61;
		break;
	case IDC_CUSTOM_LABEL_4:
		field = 62;
		break;
	//m.hancock - 7-11-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	case IDC_CUSTOMLIST_LABEL_1:
		field = 54;
		break;
	case IDC_CUSTOMLIST_LABEL_2:
		field = 55;
		break;
	case IDC_CUSTOMLIST_LABEL_3:
		field = 56;
		break;
	case IDC_CUSTOMLIST_LABEL_4:
		field = 57;
		break;
	default:
		field = 0;
		break;
	}

	return field;

}

void CNewVisitDlg::Load() {

	try {

		if(m_bIsNew) {

			//set defaults

			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			_variant_t var(dtToday);
			m_dtPicker.SetValue(var);

			//TODO:  Figure out which visit will be next in the order, and auto-select that one.
		}
		else {

			//load the existing data

			//we have a problem here because there are two records that are not connected to each other,
			//but are still part of this dialog - which was not originally intended to be reloaded.

			//TODO - make a mod that puts a relationship of some sort in the data,
			//then we don't need this code
			
			//m.hancock 8/23/05 - PLID 16756 - this query needed to be changed
			//_RecordsetPtr rs = CreateRecordset("SELECT NewEye.ID FROM EyeVisitsT AS NewEye INNER JOIN EyeVisitsT AS OldEye ON NewEye.EyeProcedureID = OldEye.EyeProcedureID WHERE OldEye.ID = %li AND OldEye.VisitType = NewEye.VisitType AND OldEye.EyeType <> NewEye.EyeType AND OldEye.Date = NewEye.Date AND OldEye.Notes = NewEye.Notes",(m_nLeftVisitID != -1 ? m_nLeftVisitID : m_nRightVisitID));
			_RecordsetPtr rs = CreateRecordset("SELECT NewEye.ID FROM EyeVisitsT AS NewEye INNER JOIN EyeVisitsT AS OldEye ON NewEye.EyeProcedureID = OldEye.EyeProcedureID WHERE OldEye.ID = %li AND OldEye.VisitType = NewEye.VisitType AND OldEye.Date = NewEye.Date AND OldEye.Notes = NewEye.Notes",(m_nLeftVisitID != -1 ? m_nLeftVisitID : m_nRightVisitID));
			if(!rs->eof) {
				long nAltID = rs->Fields->Item["ID"]->Value.lVal;
				if(m_nLeftVisitID == -1)
					m_nLeftVisitID = nAltID;
				else
					m_nRightVisitID = nAltID;
			}
			rs->Close();

			//Load the eye sketches
			//m.hancock 8/24/05 - PLID 16756 - I had to change this because only the leftvisitid is active now
			if(m_nLeftVisitID != -1) {
				//Original load command
				//m_LeftEyeDrawDlg.m_strFileName.Format("%s\\Refractive\\EyeVisit%li.nxd",GetSharedPath(),m_nLeftVisitID);
				m_LeftEyeDrawDlg.m_strFileName.Format("%s\\Refractive\\EyeVisit%li%s.nxd",GetSharedPath(),m_nLeftVisitID, "L");
				m_LeftEyeDrawDlg.Load();
				m_RightEyeDrawDlg.m_strFileName.Format("%s\\Refractive\\EyeVisit%li%s.nxd",GetSharedPath(),m_nLeftVisitID, "R");
				m_RightEyeDrawDlg.Load();
			}

			_variant_t varDate = COleDateTime::GetCurrentTime();
			CString leftSph, leftCyl, leftAxis, leftBCVA, leftVA, rightSph, rightCyl, rightAxis, rightBCVA, rightVA, notes, custom1, custom2, custom3, custom4;
			long nVisitType = -1;
			long nRatingType = -1;

			//now load left eye info
			
			rs = CreateRecordset("SELECT * FROM EyeVisitsT WHERE ID = %li", m_nLeftVisitID);
			if(!rs->eof) {
				//VisitType
				nVisitType = AdoFldLong(rs, "VisitType",-1);

				//Notes
				notes = AdoFldString(rs, "Notes","");

				//Date
				varDate = rs->Fields->Item["Date"]->Value;
				if(varDate.vt != VT_DATE)
					varDate = COleDateTime::GetCurrentTime();

				//RatingType
				nRatingType = AdoFldLong(rs, "RatingType",-1);

				//Custom1
				custom1 = AdoFldString(rs, "Custom1","");

				//Custom2
				custom2 = AdoFldString(rs, "Custom2","");

				//Custom3
				custom3 = AdoFldString(rs, "Custom3","");

				//Custom4
				custom4 = AdoFldString(rs, "Custom4","");
			}
			rs->Close();

			//now load right eye info (shared info may be overwritten, but that is okay as it is only duplicated)

			//VisitType
			m_visitTypeCombo->TrySetSelByColumn(0,nVisitType);

			//Notes
			SetDlgItemText(IDC_VISIT_NOTES, notes);

			//Date
			m_dtPicker.SetValue(varDate);

			//RatingType
			m_ratingCombo->TrySetSelByColumn(0,nRatingType);

			//Custom1
			SetDlgItemText(IDC_CUSTOM_NV_1, custom1);
		
			//Custom2
			SetDlgItemText(IDC_CUSTOM_NV_2, custom2);
		
			//Custom3
			SetDlgItemText(IDC_CUSTOM_NV_3, custom3);
		
			//Custom4
			SetDlgItemText(IDC_CUSTOM_NV_4, custom4);
		}

		//m.hancock - 7-8-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		//Load custom lists
		LoadCustomListData(m_custom1Combo, &m_adwCustomList1, m_rcCustom1, 54);
		LoadCustomListData(m_custom2Combo, &m_adwCustomList2, m_rcCustom2, 55);
		LoadCustomListData(m_custom3Combo, &m_adwCustomList3, m_rcCustom3, 56);
		LoadCustomListData(m_custom4Combo, &m_adwCustomList4, m_rcCustom4, 57);

	}NxCatchAll("Error loading information.");
}


//m.hancock - 6-29-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::EditCustomList(_DNxDataListPtr &list, long listID, CDWordArray *adwCustomSelection, CRect &rc) 
{
	try {
		_bstr_t			value;
		long			curSel;
		
		//save the current value
		curSel = list->CurSel;
		if (curSel != -1)
			value = list->Value[curSel][1];

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		if (IDOK == CEditComboBox(this, listID, list, "Edit Combo Box").DoModal())
		{
			//try and set the combo to the old value
			if (curSel != -1)
				list->SetSelByColumn(1, value);
			
			//Rebuild the selection array and datalist from what is stored in the db
			LoadCustomListData(list, adwCustomSelection, rc, listID);
		}
		else {
			//DRT 7/25/02
			//if we cancel the dialog, it requeries the list (because changes are made whether you hit ok or cancel)
			//so the list will have no selection.
			list->SetSelByColumn(1, value);
		}
	} NxCatchAll("Error in EditCustomList");
}

//m.hancock - 6-29-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnEditCustomList1() 
{
	EditCustomList(m_custom1Combo, 54, &m_adwCustomList1, m_rcCustom1);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnEditCustomList2() 
{
	EditCustomList(m_custom2Combo, 55, &m_adwCustomList2, m_rcCustom2);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnEditCustomList3() 
{
	EditCustomList(m_custom3Combo, 56, &m_adwCustomList3, m_rcCustom3);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnEditCustomList4() 
{
	EditCustomList(m_custom4Combo, 57, &m_adwCustomList4, m_rcCustom4);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnRequeryFinishedCustlist1(short nFlags) 
{
	RequeryFinishedCustomList(m_custom1Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnRequeryFinishedCustlist2(short nFlags) 
{
	RequeryFinishedCustomList(m_custom2Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnRequeryFinishedCustlist3(short nFlags) 
{
	RequeryFinishedCustomList(m_custom3Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnRequeryFinishedCustlist4(short nFlags) 
{	
	RequeryFinishedCustomList(m_custom4Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Add entries to the custom list for "no selection" and "multiple selections"
void CNewVisitDlg::RequeryFinishedCustomList(_DNxDataListPtr &customCombo)
{
	try {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow;

		// Add a row to allow the user to select "multiple purposes"
		if (customCombo->GetRowCount() > 1)
		{
			pRow = customCombo->Row[-1];
			pRow->Value[0] = (long)-2;
			pRow->Value[1] = _bstr_t(OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS);
			customCombo->InsertRow(pRow, 0);	
		}

		// Add a row to allow the user to select "no purpose"
		pRow = customCombo->Row[-1];
		pRow->Value[0] = (long)-1;
		pRow->Value[1] = _bstr_t(OUTCOMES_CUSTOM_LIST__NO_SELECTION);
		customCombo->InsertRow(pRow, 0);

		// Display our new purpose list to the user
		// and determine which custom list is being used
		if(customCombo == m_custom1Combo) // custom list 1
			RefreshCustomCombo(customCombo, &m_adwCustomList1, m_rcCustom1);
		if(customCombo == m_custom2Combo) // custom list 2
			RefreshCustomCombo(customCombo, &m_adwCustomList2, m_rcCustom2);
		if(customCombo == m_custom3Combo) // custom list 3
			RefreshCustomCombo(customCombo, &m_adwCustomList3, m_rcCustom3);
		if(customCombo == m_custom4Combo) // custom list 4
			RefreshCustomCombo(customCombo, &m_adwCustomList4, m_rcCustom4);

	} NxCatchAll("Error in RequeryFinishedCustomList");
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Select the proper entries on the data list depending on the number of selected items
void CNewVisitDlg::RefreshCustomCombo(_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc)
{
	try {
		//  If nothing is checked, select nothing
		if (adwCustomSelection->GetSize() == 0)
			customCombo->FindByColumn(0, (long)-1, 0, TRUE);
		// If we only have one item checked, lets just select it
		else if (adwCustomSelection->GetSize() == 1)
			customCombo->FindByColumn(0, (long)adwCustomSelection->GetAt(0), 0, TRUE);
		// We have multiple items
		else
			customCombo->PutCurSel(1);

		InvalidateRect(rc); // Paint the hyperlink text list
	} NxCatchAll("Error in RefreshCustomCombo");
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnSelChosenCustlist1(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom1Combo, &m_adwCustomList1, m_rcCustom1);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnSelChosenCustlist2(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom2Combo, &m_adwCustomList2, m_rcCustom2);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnSelChosenCustlist3(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom3Combo, &m_adwCustomList3, m_rcCustom3);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CNewVisitDlg::OnSelChosenCustlist4(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom4Combo, &m_adwCustomList4, m_rcCustom4);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Build an array of the selected items from a multiple-selection
void CNewVisitDlg::SelectionChosenCustomList(long nCurSel, _DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc)
{
	try {
		CString strFrom, strWhere;
		HRESULT hRes;

		if (nCurSel == -1)
		{
			customCombo->FindByColumn(0, (long)-1, 0, TRUE);
			return;
		}

		if (nCurSel > -1 && VarString(customCombo->Value[nCurSel][1], "") != OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS)
		{
			adwCustomSelection->RemoveAll();
			if (VarLong(customCombo->Value[nCurSel][0], -1) > 0) {
				adwCustomSelection->Add( VarLong(customCombo->Value[nCurSel][0]) );			
			}
		}
		else
		{
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "OutcomesListItemsT");

			// Fill the dialog with existing selections
			for (int i=0; i < adwCustomSelection->GetSize(); i++)
			{
				dlg.PreSelect(*adwCustomSelection);
			}

			dlg.m_strNameColTitle = "Items";
			strFrom = "OutcomesListItemsT";
			strWhere = (LPCTSTR)customCombo->WhereClause;

			try
			{
				hRes = dlg.Open(strFrom, strWhere, "OutcomesListItemsT.ID", "OutcomesListItemsT.Text", "Please select items from the list below.");
			} NxCatchAll("Error loading multi-select purpose list");

			// Update our array of selections with this information
			if (hRes == IDOK)
			{
				dlg.FillArrayWithIDs(*adwCustomSelection);
			}

			// Display our new purpose list to the user
			RefreshCustomCombo(customCombo, adwCustomSelection, rc);

			//Show the combo box if it is hidden
			if(customCombo == m_custom1Combo) // custom list 1
				GetDlgItem(IDC_CUSTLIST1)->ShowWindow(SW_SHOW);
			else if(customCombo == m_custom2Combo) // custom list 2
				GetDlgItem(IDC_CUSTLIST2)->ShowWindow(SW_SHOW);
			else if(customCombo == m_custom3Combo) // custom list 3
				GetDlgItem(IDC_CUSTLIST3)->ShowWindow(SW_SHOW);
			else if(customCombo == m_custom4Combo) // custom list 4
				GetDlgItem(IDC_CUSTLIST4)->ShowWindow(SW_SHOW);
		}
	} NxCatchAll("Error in SelectionChosenCustomList");
}

//m.hancock - 7-06-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Draw the hyperlink for a multiple-selection
void CNewVisitDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	try {
		DrawCustomHyperlinkList(&dc, m_rcCustom1, m_custom1Combo, &m_adwCustomList1);
		DrawCustomHyperlinkList(&dc, m_rcCustom2, m_custom2Combo, &m_adwCustomList2);
		DrawCustomHyperlinkList(&dc, m_rcCustom3, m_custom3Combo, &m_adwCustomList3);
		DrawCustomHyperlinkList(&dc, m_rcCustom4, m_custom4Combo, &m_adwCustomList4);
	} NxCatchAllIgnore(); // We're ignoring because paint messages get processed constantly.  If we gave a message to the user, they would never be able to dismiss it.
}

//m.hancock - 7-06-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Hide the necessary custom lists and show the hyperlink if a multiple-selection has been made
void CNewVisitDlg::DrawCustomHyperlinkList(CDC *pdc, CRect &rc, _DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection)
{
	try {
		if (adwCustomSelection->GetSize() <= 1)
			return;
		
		if(customCombo == m_custom1Combo) // custom list 1
			GetDlgItem(IDC_CUSTLIST1)->ShowWindow(SW_HIDE);
		else if(customCombo == m_custom2Combo) // custom list 2
			GetDlgItem(IDC_CUSTLIST2)->ShowWindow(SW_HIDE);
		else if(customCombo == m_custom3Combo) // custom list 3
			GetDlgItem(IDC_CUSTLIST3)->ShowWindow(SW_HIDE);
		else if(customCombo == m_custom4Combo) // custom list 4
			GetDlgItem(IDC_CUSTLIST4)->ShowWindow(SW_HIDE);

		//Get the list of selections
		CString strSelections = GetMultiSelectString(customCombo, adwCustomSelection);

		// (j.jones 2008-05-01 15:54) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, rc, strSelections, dtsHyperlink, false, DT_LEFT, true, false, 0);

	} NxCatchAll("Error in DrawCustomHyperlinkList");
}

//m.hancock - 7-06-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Return a formatted string containing the results of a multiple-selection
CString CNewVisitDlg::GetMultiSelectString(_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection)
{
	CString strSelections = "";
	try {
		//Traverse selections in reverse to maintain order presented in multi-select dialog
		for (long i=adwCustomSelection->GetSize()-1; i > -1; i--)
		{		
			long lRow = customCombo->FindByColumn(0, (long)adwCustomSelection->GetAt(i), 0, FALSE);
			CString strThisSelection = VarString(customCombo->GetValue(lRow, 1));
			if (strSelections.IsEmpty())
				strSelections = strThisSelection;
			else
				strSelections += ", " + strThisSelection;
		}
	} NxCatchAll("Error in GetMultiSelectString");
	return strSelections;
}

//m.hancock - 7-06-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Set the cursor to the pointy-finger hand if the cursor is in the area held by a hyperlink
BOOL CNewVisitDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (m_adwCustomList1.GetSize() > 1)
	{
		if (m_rcCustom1.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (m_adwCustomList2.GetSize() > 1)
	{
		if (m_rcCustom2.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (m_adwCustomList3.GetSize() > 1)
	{
		if (m_rcCustom3.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (m_adwCustomList4.GetSize() > 1)
	{
		if (m_rcCustom4.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

//m.hancock - 7-06-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Handle the clicking of a hyperlink
void CNewVisitDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CNxDialog::OnLButtonDown(nFlags, point);
}

//m.hancock - 7-07-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Handle the actions to be taken when a hyperlink is clicked (for the custom lists) only if the cursor is in the area held by the hyperlink
void CNewVisitDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{	
	// If there's more than one selection and the user clicked in the custom list link area, then pop up the multi-select dialog
	if (m_adwCustomList1.GetSize() > 1)
	{
		if (m_rcCustom1.PtInRect(point)) {
			// Open the multi-select checkbox list
			long nID = m_custom1Combo->FindByColumn(1, _bstr_t(OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS), 0, false);
			if(nID >= 0)
				SelectionChosenCustomList(nID, m_custom1Combo, &m_adwCustomList1, m_rcCustom1);
		}
	}

	if (m_adwCustomList2.GetSize() > 1)
	{
		if (m_rcCustom2.PtInRect(point)) {
			long nID = m_custom2Combo->FindByColumn(1, _bstr_t(OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS), 0, false);
			if(nID >= 0)
				SelectionChosenCustomList(nID, m_custom2Combo, &m_adwCustomList2, m_rcCustom2);
		}
	}

	if (m_adwCustomList3.GetSize() > 1)
	{
		if (m_rcCustom3.PtInRect(point)) {
			long nID = m_custom3Combo->FindByColumn(1, _bstr_t(OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS), 0, false);
			if(nID >= 0)
				SelectionChosenCustomList(nID, m_custom3Combo, &m_adwCustomList3, m_rcCustom3);
		}
	}

	if (m_adwCustomList4.GetSize() > 1)
	{
		if (m_rcCustom4.PtInRect(point)) {
			long nID = m_custom4Combo->FindByColumn(1, _bstr_t(OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS), 0, false);
			if(nID >= 0)
				SelectionChosenCustomList(nID, m_custom4Combo, &m_adwCustomList4, m_rcCustom4);
		}
	}
}

//m.hancock - 7-08-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Insert the selected custom list items into the database
CString CNewVisitDlg::GetCustomListInsertStmt(CDWordArray *adwCustomSelection)
{
	CString strExec="";
	try {
		CString strInsert="", strSelections = "";
		for (long i=adwCustomSelection->GetSize()-1; i > -1; i--)
		{	
			char buffer [sizeof(long)*8+1];	
			ltoa (adwCustomSelection->GetAt(i), buffer, 10);
			if (strSelections.IsEmpty())
				strSelections = buffer;
			else
				strSelections += ", " + (CString)buffer;
		}

		strSelections += " ";
		strSelections.Remove(',');
		strSelections.TrimLeft();
		int nSpace = strSelections.Find(" ");
		while(nSpace > 0)
		{
			strInsert.Empty();
			CString strID = (strSelections.Left(nSpace));
			strSelections = strSelections.Right(strSelections.GetLength() - (nSpace + 1));
			//Create record for left eye
			if(m_nLeftVisitID != -1)
				strInsert.Format("INSERT INTO EyeVisitsListDataT values (%li, %s);\r\n", m_nLeftVisitID, strID);
			//m.hancock 8/24/05 - PLID 16756 - I had to change this because only the leftvisitid is active now
			//Create record for right eye
			/*if(m_nRightVisitID != -1)
			{
				if(strInsert.IsEmpty())
					strInsert.Format("INSERT INTO EyeVisitsListDataT values (%li, %s)", m_nRightVisitID, strID);
				else
				{
					CString strTemp;
					strTemp.Format("INSERT INTO EyeVisitsListDataT values (%li, %s)", m_nRightVisitID, strID);
					strInsert += strTemp;
				}
			}*/
			strExec += strInsert;
			nSpace = strSelections.Find(" ");
		}
	} NxCatchAll("Error in GetCustomListInsertStmt");
	return strExec;
}

//m.hancock - 7-08-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Query the database for all custom list items for this specific outcome and datalist
void CNewVisitDlg::LoadCustomListData(_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc, long nFieldID)
{
	try {
		adwCustomSelection->RemoveAll();
		
		//Load record for left eye
		_RecordsetPtr rsCustom = CreateRecordset("SELECT ListItemID FROM EyeVisitsListDataT INNER JOIN OutcomesListItemsT ON EyeVisitsListDataT.ListItemID = OutcomesListItemsT.ID WHERE VisitID = %li AND CustomFieldID = %li ORDER BY ListItemID desc", m_nLeftVisitID, nFieldID);
		//m.hancock 8/24/05 - PLID 16756 - I had to change this because only the leftvisitid is active now
		//If record for right eye exists, load it instead
		//if(m_nRightVisitID != -1)
		//	rsCustom = CreateRecordset("SELECT ListItemID FROM EyeVisitsListDataT INNER JOIN OutcomesListItemsT ON EyeVisitsListDataT.ListItemID = OutcomesListItemsT.ID WHERE VisitID = %li AND CustomFieldID = %li ORDER BY ListItemID desc", m_nRightVisitID, nFieldID);

		bool bLoaded = false;
		while(!rsCustom->eof)
		{
			CString str;
			str.Format("%li ", AdoFldLong(rsCustom, "ListItemID"));
			adwCustomSelection->Add(atol(str));
			rsCustom->MoveNext();
			bLoaded = true;
		}
		rsCustom->Close();
		if(bLoaded)
			RefreshCustomCombo(customCombo, adwCustomSelection, rc);
	} NxCatchAll("Error in LoadCustomListData");
}

//m.hancock - 8-11-05 - PLID 16756 - Add advanced test types to the New Visit dialog
void CNewVisitDlg::OnAddTestButton() 
{
	//Display test type selection dialog
	CSelectTestTypeDlg dlg(this);
	if(IDOK == dlg.DoModal())
	{
		//The names of the test types could change, so the list of tests should be refreshed to show the updated test names.
		m_TestsList->GetColumn(2)->ComboSource = _bstr_t("SELECT * FROM EyeTestTypesT;");
		m_TestsList->GetColumn(3)->ComboSource = _bstr_t("SELECT 1, 'Left' UNION (SELECT 2, 'Right');");

		//Add a row for the new test
		IRowSettingsPtr pRow = m_TestsList->GetRow(-1);
		pRow->PutValue(0, (long) -1); //ID
		pRow->PutValue(1, (long) m_nLeftVisitID); //VisitID
		pRow->PutValue(2, (long) dlg.m_nSelectedTest); //TestID
		pRow->PutValue(3, (long) 1); //EyeType
		pRow->PutValue(4, "20/"); //VA
		pRow->PutValue(5, "20/"); //BCVA
		pRow->PutValue(6, (double) 0); //Sphere
		pRow->PutValue(7, (double) 0); //Cyl
		pRow->PutValue(8, (long) 0); //Axis
		m_TestsList->AddRow(pRow);
		m_TestsList->StartEditing(m_TestsList->GetRowCount()-1, 3); //Start editing on the eye type
	}
	else
	{
		//The names of the test types could change, so the list of tests should be refreshed to show the updated test names.
		m_TestsList->GetColumn(2)->ComboSource = _bstr_t("SELECT * FROM EyeTestTypesT;");
	}
}

//m.hancock - 8/22/05 - PLID 16756 - add advanced tests to refractive visits
void CNewVisitDlg::OnEditingFinishingTestsList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{	
	//m.hancock - 10-26-05 - PLID 16756 - If the user has cancelled editing, we need to restore the previous value
	if(*pbCommit == FALSE)
		return;

	//Check the entered data for errors
	switch(nCol)
	{
	
	case 2: //TestID
		{
			if(pvarNewValue->vt == VT_I4)
				if(ValidTestSelection(VarLong(pvarNewValue)))
					break;

			//The chosen test type has become invalid
			AfxMessageBox("The selected test type has become invalid.  Please select a new test type.");
			*pbCommit = false;
			*pbContinue = FALSE;
			break;
		}
	
	case 3: //EyeType
		{
			long nEyeType = VarLong(pvarNewValue);
			if(nEyeType != 1 && nEyeType != 2) {
				AfxMessageBox("An improper eye type has been selected.  Please select either left or right.");
				*pbCommit = false;
				*pbContinue = FALSE;
			}
			break;
		}
	
	case 4: //VA
		{
			CString strTemp = VarString(pvarNewValue);
			//The entered data needs to include the "20/"
			bool bValid = false;
			if(strTemp.Find("20/") >= 0)
			{
				//If the entered data is the default text, it is ok
				if(strTemp == "20/")
					bValid = true;
				else
				{
					strTemp = strTemp.Right(strTemp.GetLength() - 3);
					if(strTemp.SpanIncluding("1234567890") == strTemp && !strTemp.IsEmpty()) //is in a numerical format
						bValid = true;
				}
			}
			if(!bValid)
			{
				AfxMessageBox("The Visual Acuity is in an improper format.  Please fix it.");
				*pbCommit = false;
				*pbContinue = FALSE;
			}
 			break;

		}
	
	case 5: //BCVA
		{
			CString strTemp = VarString(pvarNewValue);
			//The entered data needs to include the "20/"
			bool bValid = false;
			if(strTemp.Find("20/") >= 0)
			{
				//If the entered data is the default text, it is ok
				if(strTemp == "20/")
					bValid = true;
				else
				{
					strTemp = strTemp.Right(strTemp.GetLength() - 3);
					if(strTemp.SpanIncluding("1234567890") == strTemp && !strTemp.IsEmpty()) //is in a numerical format
						bValid = true;
				}
			}
			if(!bValid)
			{
				AfxMessageBox("The BCVA is in an improper format.  Please fix it.");
				*pbCommit = false;
				*pbContinue = FALSE;
			}
 			break;
		}
	
	case 6: //Sphere
		{
			if((VarDouble(pvarNewValue)) || (VarDouble(pvarNewValue) == 0)) //check for numerical format
				break;
			else
			{
				AfxMessageBox("The sphere number is in an improper format.  Please fix it.");
				*pbCommit = false;
				*pbContinue = FALSE;
			}
 			break;
		}
	
	case 7: //Cyl
		{
			if((VarDouble(pvarNewValue)) || (VarDouble(pvarNewValue) == 0)) //check for numerical format
				break;
			else
			{
				AfxMessageBox("The Cyl number is in an improper format.  Please fix it.");
				*pbCommit = false;
				*pbContinue = FALSE;
			}
 			break;
		}
	
	case 8: //Axis
		{
			if((VarLong(pvarNewValue)) || (VarLong(pvarNewValue) == 0)) //check for numerical format
				break;
			else
			{
				AfxMessageBox("The Axis is in an improper format.  Please fix it.");
				*pbCommit = false;
				*pbContinue = FALSE;
			}
 			break;
		}
	
	default:
		break;
	}
}

//m.hancock - 8/22/05 - PLID 16756 - add advanced tests to refractive visits
bool CNewVisitDlg::ValidTestSelection(long nTestID)
{
	//Check if the test type is still valid
	if(ReturnsRecords("SELECT TestName FROM EyeTestTypesT WHERE TestID = %li", nTestID))
		return true;
	else
		return false;
}

//m.hancock - 8/22/05 - PLID 16756 - add advanced tests to refractive visits
bool CNewVisitDlg::IsTestDataEmpty(long nRow)
{
	//Get a ptr to the current row we're checking
	IRowSettingsPtr pRow = m_TestsList->GetRow(nRow);

	//We only need to check if the fields contain data since error-checking has already been done.
	//To do this, we only need to check if the variant type is not what we expect it to be.
	_variant_t var = pRow->GetValue(3);
	if(pRow->GetValue(3).vt != VT_I4) //EyeType
	{
		AfxMessageBox("An eye type selection has not been made.  Please select either left or right before saving.");
		return true;
	}
	if((pRow->GetValue(4).vt != VT_I4) && (pRow->GetValue(4).vt != VT_BSTR)) //VA
	{
		AfxMessageBox("The Visual Acuity is empty.  Please enter a value before saving.");
		return true;
	}
	if((pRow->GetValue(5).vt != VT_I4) && (pRow->GetValue(5).vt != VT_BSTR)) //BCVA
	{
		AfxMessageBox("The BCVA is empty.  Please enter a value before saving.");
		return true;
	}
	if(pRow->GetValue(6).vt != VT_R8) //Sphere
	{
		AfxMessageBox("The sphere number is empty.  Please enter a value before saving.");
		return true;
	}
	if(pRow->GetValue(7).vt != VT_R8) //Cyl
	{
		AfxMessageBox("The Cyl number is empty.  Please enter a value before saving.");
		return true;
	}
	if(pRow->GetValue(8).vt != VT_I4) //Axis
	{
		AfxMessageBox("The Axis is either empty.  Please enter a value before saving.");
		return true;
	}

	return false; //Everything looks ok
}

//m.hancock - 8/22/05 - PLID 16756 - add advanced tests to refractive visits
void CNewVisitDlg::OnDeleteTestButton() 
{
	//Check if a test is selected
	long nRow = m_TestsList->GetCurSel();
	if(nRow > -1)
	{ //A test is selected
		//Ask if they're sure they want to delete the test
		if(AfxMessageBox("Are you SURE you wish to delete this test and its data?", MB_YESNO) == IDYES)
		{
			//Get the ID number
			long nID = VarLong(m_TestsList->GetValue(nRow, 0));

			//Remove it from the list
			m_TestsList->RemoveRow(nRow);

			//Check if the test is stored in the database
			//If the test is in the database, also delete it with a sql query
			if(nID != -1)
			{
				try {
					ExecuteSql("DELETE FROM EyeTestsT WHERE ID = %li", nID);
				} NxCatchAll("Error deleting eye test in CNewVisitDlg");
			}

			//Do not requery the list since there could be data that is not stored yet
		}
		//They're afraid of committment and don't really want to delete the test, so just return.
		else
			return;
	}
	else 
	{ //A test is not selected
		AfxMessageBox("You must choose a test to delete.\nPlease select a test from the list before attempting to delete again.");
		return;
	}
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CNewVisitDlg::OnOK()
{
	//Eat the message
}
