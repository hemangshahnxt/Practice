// VisitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "VisitDlg.h"
#include "NewVisitDlg.h"
#include "DateTimeUtils.h"

//m.hancock - 6-29-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
#include "EditComboBox.h"
#include "MultiSelectDlg.h"
#include "GlobalDrawingUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CVisitDlg dialog


CVisitDlg::CVisitDlg(BOOL bIsNewRecord, CWnd* pParent)
	: CNxDialog(CVisitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVisitDlg)
	//}}AFX_DATA_INIT
	m_bNewRecord = bIsNewRecord;
}


void CVisitDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVisitDlg)
	DDX_Control(pDX, IDC_RIGHT_EYE, m_btnRightEye);
	DDX_Control(pDX, IDC_LEFT_EYE, m_btnLeftEye);
	DDX_Control(pDX, IDC_CONTACTS_CHECK, m_btnWearsContacts);
	DDX_Control(pDX, IDC_MONOVISION, m_btnMonovision);
	DDX_Control(pDX, IDC_SURGERY_DATE, m_dtPicker);
	DDX_Control(pDX, IDC_PROCEDURE_TYPE, m_nxeditProcedureType);
	DDX_Control(pDX, IDC_OPERATION_TYPE, m_nxeditOperationType);
	DDX_Control(pDX, IDC_LASER_USED, m_nxeditLaserUsed);
	DDX_Control(pDX, IDC_EQUIPMENT_USED, m_nxeditEquipmentUsed);
	DDX_Control(pDX, IDC_KERATOME_TYPE, m_nxeditKeratomeType);
	DDX_Control(pDX, IDC_POWER, m_nxeditPower);
	DDX_Control(pDX, IDC_BLADE_TYPE, m_nxeditBladeType);
	DDX_Control(pDX, IDC_NUM_PULSES, m_nxeditNumPulses);
	DDX_Control(pDX, IDC_PAT_AGE, m_nxeditPatAge);
	DDX_Control(pDX, IDC_PUPIL_LIGHT, m_nxeditPupilLight);
	DDX_Control(pDX, IDC_PUPIL_DARK, m_nxeditPupilDark);
	DDX_Control(pDX, IDC_CUSTOM_1, m_nxeditCustom1);
	DDX_Control(pDX, IDC_CUSTOM_2, m_nxeditCustom2);
	DDX_Control(pDX, IDC_CUSTOM_3, m_nxeditCustom3);
	DDX_Control(pDX, IDC_CUSTOM_4, m_nxeditCustom4);
	DDX_Control(pDX, IDC_CONTACT_LENS_TYPE, m_nxeditContactLensType);
	DDX_Control(pDX, IDC_CONTACT_LENS_USE, m_nxeditContactLensUse);
	DDX_Control(pDX, IDC_COMPLAINTS_BOX, m_nxeditComplaintsBox);
	DDX_Control(pDX, IDC_PATIENT_NAME, m_nxstaticPatientName);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST1, m_nxstaticCustlist1);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST2, m_nxstaticCustlist2);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST3, m_nxstaticCustlist3);
	DDX_Control(pDX, IDC_STATIC_CUSTLIST4, m_nxstaticCustlist4);
	DDX_Control(pDX, IDC_STATIC_VISIT_TESTS_NOTE, m_nxstaticVisitTestsNote);
	DDX_Control(pDX, IDC_NEW_VISIT_BTN, m_btnNewVisit);
	DDX_Control(pDX, IDC_DELETE_VISIT_BTN, m_btnDeleteVisit);
	DDX_Control(pDX, IDOK, m_btnOk);
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

//	ON_EVENT(CVisitDlg, IDC_SURGERY_DATE, 2 /* Change */, OnChangeSurgeryDate, VTS_NONE)
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

BEGIN_MESSAGE_MAP(CVisitDlg, CNxDialog)
	//{{AFX_MSG_MAP(CVisitDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SURGERY_DATE, OnChangeSurgeryDate)
	ON_BN_CLICKED(IDC_NEW_VISIT_BTN, OnNewVisitBtn)
	ON_BN_CLICKED(IDC_LEFT_EYE, OnLeftEye)
	ON_BN_CLICKED(IDC_RIGHT_EYE, OnRightEye)
	ON_BN_CLICKED(IDC_CONTACTS_CHECK, OnContactsCheck)
	ON_BN_CLICKED(IDC_MONOVISION, OnMonovision)
	ON_BN_CLICKED(IDC_DELETE_VISIT_BTN, OnDeleteVisitBtn)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_1, OnEditCustomList1)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_2, OnEditCustomList2)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_3, OnEditCustomList3)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_4, OnEditCustomList4)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_CLOSE, OnClose)
	// (a.walling 2010-10-12 15:27) - PLID 40906 - Dead code - this button is not even on any dialog
	//ON_BN_CLICKED(IDC_SAVE_VISIT, OnSave)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisitDlg message handlers

void CVisitDlg::OnNewVisitBtn() 
{
	CNewVisitDlg dlg(this);

	dlg.m_nProcID = m_nCurrentID;		//this let's the visits associate with this eye procedure
	int result = dlg.DoModal();

	if(result) {	//something was added
		m_visitsCombo->Requery();
	}

}

BOOL CVisitDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnNewVisit.AutoSet(NXB_NEW);
	m_btnDeleteVisit.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//setup datalists
	m_visitsCombo = BindNxDataListCtrl(IDC_VISITS, false);
	m_procedureCombo = BindNxDataListCtrl(IDC_PROCEDURE_COMBO, true);
	m_providerCombo = BindNxDataListCtrl(IDC_PROV_COMBO, true);
	m_locationCombo = BindNxDataListCtrl(IDC_LOCATION_CMB, true);

	//m.hancock 8/23/2005 - PLID 16756 - Advanced tests for refractive visits
	m_strVisitsWhere.Format("EyeProceduresT.ID = %li", m_nCurrentID);

	GetDlgItem(IDC_CONTACT_LENS_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_CONTACT_LENS_USE)->EnableWindow(FALSE);

	//set custom labels
	_RecordsetPtr CustomLabelsSet = CreateRecordset("SELECT * FROM CustomFieldsT WHERE ID >= 55 AND ID <= 58");
	while(!CustomLabelsSet->eof) {
		CString WhatShowsInPat = CString(CustomLabelsSet->Fields->Item["Name"]->Value.bstrVal);
		switch(CustomLabelsSet->Fields->Item["ID"]->Value.lVal) {
		case 55:
			SetDlgItemText(IDC_CUSTOM_LABEL_1, ConvertToControlText(WhatShowsInPat));
			break;
		case 56:
			SetDlgItemText(IDC_CUSTOM_LABEL_2, ConvertToControlText(WhatShowsInPat));
			break;
		case 57:
			SetDlgItemText(IDC_CUSTOM_LABEL_3, ConvertToControlText(WhatShowsInPat));
			break;
		case 58:
			SetDlgItemText(IDC_CUSTOM_LABEL_4, ConvertToControlText(WhatShowsInPat));
			break;
		}
		CustomLabelsSet->MoveNext();
	}

	//m.hancock - 7-11-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	//Load values for custom list labels
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_1, ConvertToControlText(GetRemotePropertyText("OutcomesCustomListLabel1", "Custom List 1", 0, "<None>", false)));
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_2, ConvertToControlText(GetRemotePropertyText("OutcomesCustomListLabel2", "Custom List 2", 0, "<None>", false)));
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_3, ConvertToControlText(GetRemotePropertyText("OutcomesCustomListLabel3", "Custom List 3", 0, "<None>", false)));
	SetDlgItemText(IDC_CUSTOMLIST_LABEL_4, ConvertToControlText(GetRemotePropertyText("OutcomesCustomListLabel4", "Custom List 4", 0, "<None>", false)));

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

	Load();			//load the info

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVisitDlg::OnLeftEye() 
{
	if(m_nSelectedEye == 1)	//already is set to left
		return;

	m_nSelectedEye = 1;
	
	//m.hancock 8/23/2005 - PLID 16756 - Advanced tests for refractive visits
	//Changed the query to show the last test that was done
	//CString strWhere;
	//strWhere.Format("EyeType = 1 AND EyeProceduresT.ID = %li", m_nCurrentID);
	CString strWhere;
	strWhere = m_strVisitsWhere + " AND EyeTestsT.ID = (SELECT MAX(ID) FROM EyeTestsT WHERE VisitID = EyeVisitsT.ID AND EyeType = 1)";
	m_visitsCombo->WhereClause = _bstr_t(strWhere);
	m_visitsCombo->Requery();
}

void CVisitDlg::OnRightEye() 
{
	if(m_nSelectedEye == 2)	//already set to right
		return;

	m_nSelectedEye = 2;
	
	//m.hancock 8/23/2005 - PLID 16756 - Advanced tests for refractive visits
	//Changed the query to show the last test that was done
	//CString strWhere;
	//strWhere.Format("EyeType = 2 AND EyeProceduresT.ID = %li", m_nCurrentID);
	CString strWhere;
	strWhere = m_strVisitsWhere + " AND EyeTestsT.ID = (SELECT MAX(ID) FROM EyeTestsT WHERE VisitID = EyeVisitsT.ID AND EyeType = 2)";
	m_visitsCombo->WhereClause = _bstr_t(strWhere);
	m_visitsCombo->Requery();
}

void CVisitDlg::Load()
{
	try {

		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT EyeProceduresT.ID, ProcedureDate, ProcedureID, LocationID, ProviderID, Monovision, Contacts, "
			"Complaint, Age, ProcType, OpType, LaserUsed, KeratomeType, BladeType, EquipmentUsed, Power, NumPulses, PupilLight, "
			"PupilDark, CLType, CLUse, Custom1, Custom2, Custom3, Custom4, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName FROM EyeProceduresT INNER JOIN PersonT ON EyeProceduresT.PatientID = PersonT.ID WHERE EyeProceduresT.ID = %li", m_nCurrentID);

		if(rs->eof) {	//serious error if the wrong ID is here
			MessageBox("The wrong ID was passed to this dialog.  Please refresh the refractive tab and try again");
			CDialog::OnCancel();
		}

		//load the fields

		//fill in the static name field
		SetDlgItemText(IDC_PATIENT_NAME, CString(rs->Fields->Item["PatName"]->Value.bstrVal));

		//fill in the procedure combo
		_variant_t procID = rs->Fields->Item["ProcedureID"]->Value;
		if (sriNoRow == m_procedureCombo->SetSelByColumn(0, procID) && VarLong(procID,-1) > 0) {
			// (c.haag 2008-12-18 12:55) - PLID 32539 - If we didn't find the procedure, it may be inactive. Add
			// the inactive row.
			_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", VarLong(procID));
			if (!prs->eof) {
				NXDATALISTLib::IRowSettingsPtr pRow = m_procedureCombo->GetRow(-1);
				pRow->PutValue(0,procID);
				pRow->PutValue(1,prs->Fields->Item["Name"]->Value);
				m_procedureCombo->AddRow(pRow);
				m_procedureCombo->Sort();
				m_procedureCombo->SetSelByColumn(0, procID);
			} else {
				// It was deleted. Nothing we can do.
			}
		}

		//fill in the location combo
		_variant_t locID = rs->Fields->Item["LocationID"]->Value;
		//(e.lally 2006-10-18) PLID 21948 - We need to account for inactive locations.
		if(m_locationCombo->SetSelByColumn(0, locID) == -1) {
			//The location might be inactive, only fill in the name if this is an existing record
			if(!m_bNewRecord) {
				_RecordsetPtr rsLoc = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", VarLong(locID, -1));
				if(!rsLoc->eof){
					m_locationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
					m_nLocationID = VarLong(locID, -1);
				}
				else{
					m_locationCombo->PutCurSel(-1);
					m_nLocationID = -1;
				}
			}
			else {
				m_nLocationID = -1;
			}
		}
		else {
			m_nLocationID = VarLong(locID);
		}

		//fill in the provider combo
		_variant_t provID = rs->Fields->Item["ProviderID"]->Value;
		if(m_providerCombo->SetSelByColumn(0, provID) == -1) {

			//PLID 21414 - only do this if its an existing record
			if (!m_bNewRecord) {
				//they may have an inactive provider
				_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", VarLong(provID, -1));
				if(!rsProv->eof) {
					m_providerCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));

					//PLID 21414 - Set the member variable
					m_nProviderID = VarLong(provID, -1);
				}
				else  {
					m_providerCombo->PutCurSel(-1);
					m_nProviderID = -1;
				}
			}
			else {
				m_nProviderID = -1;
			}
		}
		else {
			m_nProviderID = VarLong(provID);
		}

		//fill in the date picker
		m_dtPicker.SetValue(rs->Fields->Item["ProcedureDate"]->Value);

		//set the checks for monovision + contacts
		if(rs->Fields->Item["Monovision"]->Value.boolVal)
			CheckDlgButton(IDC_MONOVISION, 1);
		if(rs->Fields->Item["Contacts"]->Value.boolVal) {
			CheckDlgButton(IDC_CONTACTS_CHECK, 1);
			GetDlgItem(IDC_CONTACT_LENS_TYPE)->EnableWindow(TRUE);
			GetDlgItem(IDC_CONTACT_LENS_USE)->EnableWindow(TRUE);
		}

		//load the complaint
		CString complaint;
		complaint = CString(rs->Fields->Item["Complaint"]->Value.bstrVal);
		SetDlgItemText(IDC_COMPLAINTS_BOX, complaint);
		
		//load the patients age
		_variant_t var;
		var = rs->Fields->Item["Age"]->Value;
		if(var.vt == VT_I4)
			SetDlgItemInt(IDC_PAT_AGE, AdoFldLong(rs, "Age"));

		//load the procedure type
		CString ProcType;
		ProcType = CString(rs->Fields->Item["ProcType"]->Value.bstrVal);
		SetDlgItemText(IDC_PROCEDURE_TYPE, ProcType);

		//load the operation type
		CString OpType;
		OpType = CString(rs->Fields->Item["OpType"]->Value.bstrVal);
		SetDlgItemText(IDC_OPERATION_TYPE, OpType);

		//load the laser used
		CString LaserUsed;
		LaserUsed = CString(rs->Fields->Item["LaserUsed"]->Value.bstrVal);
		SetDlgItemText(IDC_LASER_USED, LaserUsed);

		//load the keratome type
		CString KeratomeType;
		KeratomeType = CString(rs->Fields->Item["KeratomeType"]->Value.bstrVal);
		SetDlgItemText(IDC_KERATOME_TYPE, KeratomeType);

		//load the blade type
		CString BladeType;
		BladeType = CString(rs->Fields->Item["BladeType"]->Value.bstrVal);
		SetDlgItemText(IDC_BLADE_TYPE, BladeType);

		//load the equipment used
		CString EquipmentUsed;
		EquipmentUsed = CString(rs->Fields->Item["EquipmentUsed"]->Value.bstrVal);
		SetDlgItemText(IDC_EQUIPMENT_USED, EquipmentUsed);

		//load the power
		CString Power;
		Power = CString(rs->Fields->Item["Power"]->Value.bstrVal);
		SetDlgItemText(IDC_POWER, Power);

		//load the number of pulses
		long NumPulses;
		NumPulses = AdoFldLong(rs, "NumPulses",-1);
		if(NumPulses > -1)
			SetDlgItemInt(IDC_NUM_PULSES, NumPulses);

		//load the pupil size (light)
		CString PupilLight;
		PupilLight = CString(rs->Fields->Item["PupilLight"]->Value.bstrVal);
		SetDlgItemText(IDC_PUPIL_LIGHT, PupilLight);

		//load the pupil size (dark)
		CString PupilDark;
		PupilDark = CString(rs->Fields->Item["PupilDark"]->Value.bstrVal);
		SetDlgItemText(IDC_PUPIL_DARK, PupilDark);

		//load the contact lens type
		CString CLType;
		CLType = CString(rs->Fields->Item["CLType"]->Value.bstrVal);
		SetDlgItemText(IDC_CONTACT_LENS_TYPE, CLType);

		//load the contact lens use
		CString CLUse;
		CLUse = CString(rs->Fields->Item["CLUse"]->Value.bstrVal);
		SetDlgItemText(IDC_CONTACT_LENS_USE, CLUse);

		//load the custom info
		CString Custom1, Custom2, Custom3, Custom4;
		Custom1 = AdoFldString(rs, "Custom1","");
		SetDlgItemText(IDC_CUSTOM_1, Custom1);
		Custom2 = AdoFldString(rs, "Custom2","");
		SetDlgItemText(IDC_CUSTOM_2, Custom2);
		Custom3 = AdoFldString(rs, "Custom3","");
		SetDlgItemText(IDC_CUSTOM_3, Custom3);
		Custom4 = AdoFldString(rs, "Custom4","");
		SetDlgItemText(IDC_CUSTOM_4, Custom4);

		//set the check box to the left eye
		m_visitsCombo->WhereClause;
		CheckDlgButton(IDC_LEFT_EYE, 1);
		OnLeftEye();

		//m.hancock - 7-8-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		//Load custom lists
		LoadCustomListData(m_custom1Combo, &m_adwCustomList1, m_rcCustom1, 50);
		LoadCustomListData(m_custom2Combo, &m_adwCustomList2, m_rcCustom2, 51);
		LoadCustomListData(m_custom3Combo, &m_adwCustomList3, m_rcCustom3, 52);
		LoadCustomListData(m_custom4Combo, &m_adwCustomList4, m_rcCustom4, 53);


	} NxCatchAll("Error Loading CVisitDlg");
}

BEGIN_EVENTSINK_MAP(CVisitDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CVisitDlg)
	ON_EVENT(CVisitDlg, IDC_PROV_COMBO, 2 /* SelChanged */, OnSelChangedProvCombo, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_LOCATION_CMB, 2 /* SelChanged */, OnSelChangedLocationCmb, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_PROCEDURE_COMBO, 2 /* SelChanged */, OnSelChangedProcedureCombo, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_VISITS, 4 /* LButtonDown */, OnLButtonDownVisits, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CVisitDlg, IDC_VISITS, 3 /* DblClickCell */, OnDblClickCellVisits, VTS_I4 VTS_I2)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST1, 18 /* RequeryFinished */, OnRequeryFinishedCustlist1, VTS_I2)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST2, 18 /* RequeryFinished */, OnRequeryFinishedCustlist2, VTS_I2)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST3, 18 /* RequeryFinished */, OnRequeryFinishedCustlist3, VTS_I2)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST4, 18 /* RequeryFinished */, OnRequeryFinishedCustlist4, VTS_I2)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST1, 16 /* SelChosen */, OnSelChosenCustlist1, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST2, 16 /* SelChosen */, OnSelChosenCustlist2, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST3, 16 /* SelChosen */, OnSelChosenCustlist3, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_CUSTLIST4, 16 /* SelChosen */, OnSelChosenCustlist4, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_PROV_COMBO, 16 /* SelChosen */, OnSelChosenProvCombo, VTS_I4)
	ON_EVENT(CVisitDlg, IDC_LOCATION_CMB, 16 /* SelChosen */, OnSelChosenLocationCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CVisitDlg::OnSelChangedProvCombo(long nNewSel) 
{
}

void CVisitDlg::OnSelChangedLocationCmb(long nNewSel) 
{
	try {
		
	} NxCatchAll("Error saving Location");
}

void CVisitDlg::OnSelChangedProcedureCombo(long nNewSel) 
{
	try {
		if(nNewSel == sriNoRow){
			if(m_procedureCombo->GetRowCount() <= 0){
				return;
			}
			nNewSel = 0;
			m_procedureCombo->PutCurSel(nNewSel);
		}

		CString proc;
		proc = VarString(m_procedureCombo->GetValue(nNewSel,1),"");
		SetDlgItemText(IDC_PROCEDURE_TYPE,proc);
		
		
	} NxCatchAll("Error saving Procedure");
}

void CVisitDlg::OnContactsCheck() 
{
	if( IsDlgButtonChecked(IDC_CONTACTS_CHECK) ) {
		GetDlgItem(IDC_CONTACT_LENS_TYPE)->EnableWindow(TRUE);
		GetDlgItem(IDC_CONTACT_LENS_USE)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_CONTACT_LENS_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_CONTACT_LENS_USE)->EnableWindow(FALSE);
	}
}

void CVisitDlg::OnMonovision() 
{
	
}

void CVisitDlg::OnChangeSurgeryDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
}

void CVisitDlg::OnCancel() {

	CDialog::OnCancel();
}

void CVisitDlg::OnDeleteVisitBtn() 
{
	if(m_visitsCombo->GetCurSel() == -1)
		return;

	if(MessageBox("Are you sure you wish to delete this visit?", "Delete?", MB_YESNO) == IDNO)
		return;

	try {
		//delete the visit!
		long nID = VarLong(m_visitsCombo->GetValue(m_visitsCombo->GetCurSel(), 1));
		//m.hancock - 8/22/05 - PLID 16756 - Need to delete the tests associated with the visit first
		ExecuteSql("DELETE FROM EyeTestsT WHERE VisitID = %li", nID);
		//Then we need to delete the EyeVisitsListData;
		ExecuteSql("DELETE FROM EyeVisitsListDataT WHERE VisitID = %li", nID);
		//Finally, delete the visit
		ExecuteSql("DELETE FROM EyeVisitsT WHERE ID = %li", nID);

	} NxCatchAll("Error deleting visit in CVisitDlg");

	m_visitsCombo->Requery();
}

BOOL CVisitDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	CString str, field, value;

	try {

	switch (HIWORD(wParam))
	{
		case EN_KILLFOCUS:
			
			switch ((nID = LOWORD(wParam))) {
			
				case IDC_COMPLAINTS_BOX:
					GetDlgItemText(nID,value);
					if(value.GetLength() > 1000) {	//must fit in the database
						AfxMessageBox("Your complaint field is too large.  The maximum is 1000 characters.  Saving the first 1000 characters, the rest will be truncated.");
						value = value.Left(1000);
						SetDlgItemText(IDC_COMPLAINTS_BOX, value);
					}	
				break;
			}
		break;
	}

	}NxCatchAll("Error saving data.");
	
	return CNxDialog::OnCommand(wParam, lParam);
}

BOOL CVisitDlg::PreTranslateMessage(MSG* pMsg) 
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

BOOL CVisitDlg::ChangeCustomLabel (int nID)
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

	strResult.TrimRight(" ");

	if (nResult == IDOK && strResult != "")
	{
		try {

			//m.hancock - 7-11-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
			//Store the string for the custom list label
			CString strPropName;
			switch(field)
			{
			case 50:
				strPropName = "OutcomesCustomListLabel1";
				break;
			case 51:
				strPropName = "OutcomesCustomListLabel2";
				break;
			case 52:
				strPropName = "OutcomesCustomListLabel3";
				break;
			case 53:
				strPropName = "OutcomesCustomListLabel4";
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

int CVisitDlg::GetLabelFieldID(int nID)
{
	int field = 0;

	switch(nID)
	{
	case IDC_CUSTOM_LABEL_1:
		field = 55;
		break;
	case IDC_CUSTOM_LABEL_2:
		field = 56;
		break;
	case IDC_CUSTOM_LABEL_3:
		field = 57;
		break;
	case IDC_CUSTOM_LABEL_4:
		field = 58;
		break;
	//m.hancock - 7-11-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	case IDC_CUSTOMLIST_LABEL_1:
		field = 50;
		break;
	case IDC_CUSTOMLIST_LABEL_2:
		field = 51;
		break;
	case IDC_CUSTOMLIST_LABEL_3:
		field = 52;
		break;
	case IDC_CUSTOMLIST_LABEL_4:
		field = 53;
		break;
	default:
		field = 0;
		break;
	}

	return field;

}

void CVisitDlg::OnLButtonDownVisits(long nRow, short nCol, long x, long y, long nFlags) 
{
	//Changed to open with a double click rather than a single click (like the rest of the program)
}

void CVisitDlg::OnDblClickCellVisits(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	try {

		//column 1 is the VisitID
		_variant_t var = m_visitsCombo->GetValue(nRowIndex, 1);
		if(var.vt != VT_I4)
			return;

		long VisitID = var.lVal;

		CNewVisitDlg dlg(this);

		dlg.m_nProcID = m_nCurrentID;		//this let's the visits associate with this eye procedure

		//m.hancock 8/23/2005 - PLID 16756 - I had to change this because the new structure for refractive visits only uses the leftid
		/*if(m_nSelectedEye == 1)
			dlg.m_nLeftVisitID = VisitID;
		else
			dlg.m_nRightVisitID = VisitID;*/
		dlg.m_nLeftVisitID = VisitID;

		int result = dlg.DoModal();

		if(result) {	//something was added
			m_visitsCombo->Requery();
		}

	}NxCatchAll("Error loading visit data.");

}

bool CVisitDlg::Save() {

	try {

		long nProcID;
		
		//Provider Combo
		//PLID 21414 - Instead of checking the datalist, check the member variable
		if(m_nProviderID == -1){
			MessageBox("Please choose a provider before saving.");
			return false;
		}

		//location
		if(m_nLocationID == -1){
			MessageBox("Please choose a location before saving.");
			return false;
		}

		//procedure info
		if(m_procedureCombo->GetCurSel() != sriNoRow){
			nProcID = VarLong(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 0));
		}
		else {
			MessageBox("Please choose a procedure before saving.");
			return false;
		}		
			
		//Contacts
		long nContacts = IsDlgButtonChecked(IDC_CONTACTS_CHECK);


		//date
		COleDateTime dt;
		CString date;
		dt = m_dtPicker.GetValue();
		date = FormatDateTimeForSql(dt, dtoDate);

		//monovision
		long nMonovision = IsDlgButtonChecked(IDC_MONOVISION);

		//ProcType
		CString strProcType;
		GetDlgItemText(IDC_PROCEDURE_TYPE,strProcType);
								
		//OPType
		CString strOpType;
		GetDlgItemText(IDC_OPERATION_TYPE, strOpType);
						
		//LaserUser
		CString strLaserUsed;
		GetDlgItemText(IDC_LASER_USED, strLaserUsed);
						

		//KeratomeType
		CString strKeratomeType;
		GetDlgItemText(IDC_KERATOME_TYPE, strKeratomeType);

		//BladeType
		CString strBladeType;
		GetDlgItemText(IDC_BLADE_TYPE,strBladeType);
						
						
		//EquipmentUsed
		CString strEquipmentUsed;
		GetDlgItemText(IDC_EQUIPMENT_USED, strEquipmentUsed);

		//Power
		CString strPower;
		GetDlgItemText(IDC_POWER, strPower);
						
		//NumPulses
		long nNumPulses;
		nNumPulses = GetDlgItemInt(IDC_NUM_PULSES);

		//PupilLight
		CString strPupilLight;
		GetDlgItemText(IDC_PUPIL_LIGHT,strPupilLight);

		//PupilDark
		CString strPupilDark;
		GetDlgItemText(IDC_PUPIL_DARK,strPupilDark);
		
						
		//CLType
		CString strCLType;
		GetDlgItemText(IDC_CONTACT_LENS_TYPE, strCLType);
						
		
		//CLUse
		CString strCLUse;
		GetDlgItemText(IDC_CONTACT_LENS_USE, strCLUse);
						
		
		//Complaint
		CString strComplaint;
		GetDlgItemText(IDC_COMPLAINTS_BOX , strComplaint);
						

		//Custom1
		CString strCustom1;
		GetDlgItemText(IDC_CUSTOM_1, strCustom1);
						
		//Custom2
		CString strCustom2;
		GetDlgItemText(IDC_CUSTOM_2, strCustom2);
						
		//Custom3
		CString strCustom3;
		GetDlgItemText(IDC_CUSTOM_3, strCustom3);
						
		//custom 4
		CString strCustom4;
		GetDlgItemText(IDC_CUSTOM_4, strCustom4);
		
		ExecuteSql("UPDATE EyeProceduresT SET ProviderID = %li, LocationID = %li, ProcedureID = %li, ProcType = '%s', Contacts = %li, "
			" Monovision = %li, ProcedureDate = '%s', OpType = '%s', LaserUsed = '%s', KeratomeType = '%s',  "
			" BladeType = '%s', EquipmentUsed = '%s', Power = '%s', NumPulses = %li, PupilLight = '%s', PupilDark = '%s', "
			" CLType = '%s', CLUse = '%s', Complaint = '%s',  Custom1 = '%s', Custom2 = '%s', Custom3 = '%s', Custom4 = '%s' "
			" WHERE ID = %li", m_nProviderID, m_nLocationID, nProcID, _Q(strProcType), nContacts, nMonovision, _Q(date), _Q(strOpType), _Q(strLaserUsed), _Q(strKeratomeType),
			  _Q(strBladeType), _Q(strEquipmentUsed), _Q(strPower), nNumPulses, _Q(strPupilLight), _Q(strPupilDark), _Q(strCLType), _Q(strCLUse), _Q(strComplaint), 
			  _Q(strCustom1), _Q(strCustom2), _Q(strCustom3), _Q(strCustom4), m_nCurrentID);

		//m.hancock - 7-8-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
		//Delete any existing records
		CString strExec;
		strExec.Format("DELETE FROM OutcomesListDataT WHERE OutcomeID = %li", m_nCurrentID);
		ExecuteSql("%s", strExec);
		//Insert values for custom lists
		strExec.Empty();
		strExec = GetCustomListInsertStmt(&m_adwCustomList1)
				+ GetCustomListInsertStmt(&m_adwCustomList2)
				+ GetCustomListInsertStmt(&m_adwCustomList3)
				+ GetCustomListInsertStmt(&m_adwCustomList4);
		if(!strExec.IsEmpty())
			ExecuteSql("%s", strExec);
	
	}NxCatchAll("Error Saving");
	
	return true;

}

void CVisitDlg::OnOK() 
{
	if(!Save()) {
		// there was a problem saving
		return;
	}
	
	CDialog::OnOK();
}

//m.hancock - 6-29-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::EditCustomList(_DNxDataListPtr &list, long listID, CDWordArray *adwCustomSelection, CRect &rc) 
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
void CVisitDlg::OnEditCustomList1() 
{
	EditCustomList(m_custom1Combo, 50, &m_adwCustomList1, m_rcCustom1);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnEditCustomList2() 
{
	EditCustomList(m_custom2Combo, 51, &m_adwCustomList2, m_rcCustom2);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnEditCustomList3() 
{
	EditCustomList(m_custom3Combo, 52, &m_adwCustomList3, m_rcCustom3);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnEditCustomList4() 
{
	EditCustomList(m_custom4Combo, 53, &m_adwCustomList4, m_rcCustom4);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnRequeryFinishedCustlist1(short nFlags) 
{
	RequeryFinishedCustomList(m_custom1Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnRequeryFinishedCustlist2(short nFlags) 
{
	RequeryFinishedCustomList(m_custom2Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnRequeryFinishedCustlist3(short nFlags) 
{
	RequeryFinishedCustomList(m_custom3Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnRequeryFinishedCustlist4(short nFlags) 
{	
	RequeryFinishedCustomList(m_custom4Combo);
}

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Add entries to the custom list for "no selection" and "multiple selections"
void CVisitDlg::RequeryFinishedCustomList(_DNxDataListPtr &customCombo)
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
void CVisitDlg::RefreshCustomCombo(_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc)
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
void CVisitDlg::OnSelChosenCustlist1(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom1Combo, &m_adwCustomList1, m_rcCustom1);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnSelChosenCustlist2(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom2Combo, &m_adwCustomList2, m_rcCustom2);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnSelChosenCustlist3(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom3Combo, &m_adwCustomList3, m_rcCustom3);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
void CVisitDlg::OnSelChosenCustlist4(long nCurSel) 
{
	SelectionChosenCustomList(nCurSel, m_custom4Combo, &m_adwCustomList4, m_rcCustom4);
}

//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Build an array of the selected items from a multiple-selection
void CVisitDlg::SelectionChosenCustomList(long nCurSel, _DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc)
{
	try {
		CString strFrom, strWhere;
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "OutcomesListItemsT");
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
void CVisitDlg::OnPaint() 
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
void CVisitDlg::DrawCustomHyperlinkList(CDC *pdc, CRect &rc, _DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection)
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
CString CVisitDlg::GetMultiSelectString(_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection)
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
BOOL CVisitDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
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
void CVisitDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CNxDialog::OnLButtonDown(nFlags, point);
}

//m.hancock - 7-07-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Handle the actions to be taken when a hyperlink is clicked (for the custom lists) only if the cursor is in the area held by the hyperlink
void CVisitDlg::DoClickHyperlink(UINT nFlags, CPoint point)
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
CString CVisitDlg::GetCustomListInsertStmt(CDWordArray *adwCustomSelection)
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
		while(nSpace > 0) {
			CString strID = (strSelections.Left(nSpace));
			strSelections = strSelections.Right(strSelections.GetLength() - (nSpace + 1));
			strInsert.Format("INSERT INTO OutcomesListDataT values (%li, %s)", m_nCurrentID, strID);
			strExec += strInsert;
			nSpace = strSelections.Find(" ");
		}
	} NxCatchAll("Error in GetCustomListInsertStmt");
	return strExec;
}

//m.hancock - 7-08-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
// Query the database for all custom list items for this specific outcome and datalist
void CVisitDlg::LoadCustomListData(_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc, long nFieldID)
{
	try {
		adwCustomSelection->RemoveAll();
		_RecordsetPtr rsCustom = CreateRecordset("SELECT ListItemID FROM OutcomesListDataT INNER JOIN OutcomesListItemsT ON OutcomesListDataT.ListItemID = OutcomesListItemsT.ID WHERE OutcomeID = %li AND CustomFieldID = %li ORDER BY ListItemID desc;", m_nCurrentID, nFieldID);

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

void CVisitDlg::OnSelChosenProvCombo(long nRow) 
{
	try{
		if (nRow == -1) {
			m_nProviderID = -1;
		}
		else {
			m_nProviderID = VarLong(m_providerCombo->GetValue(nRow, 0));
		}
	}NxCatchAll("Error selecting new provider");
}

void CVisitDlg::OnSelChosenLocationCombo(long nRow) 
{
	try{
		if (nRow == -1) {
			m_nLocationID = -1;
		}
		else {
			m_nLocationID = VarLong(m_locationCombo->GetValue(nRow, 0));
		}
	}NxCatchAll("Error selecting new location");
	
}
