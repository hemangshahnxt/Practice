// ProcedureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProcedureDlg.h"
#include "GlobalDataUtils.h"
#include "GetNewIDName.h"
#include "Client.h"
#include "AddProcedureDlg.h"
#include "EditComboBox.h"
#include "ProcedureGroupsDlg.h"
#include "ProcedureDiscountSetupDlg.h"
#include "AuditTrail.h"
#include "GlobalSchedUtils.h"
#include "SelectMasterDlg.h"

#include "EmrActionDlg.h"
#include "CustomRecordActionDlg.h"
#include "GlobalDrawingUtils.h"
#include "MultiSelectDlg.h"
#include "ProcedureLadderAssignment.h"
#include "AdministratorRc.h"
#include "InactiveProceduresDlg.h"
#include "InactiveProcedureWarningDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_DELETE  38704
/////////////////////////////////////////////////////////////////////////////
// CProcedureDlg dialog


CProcedureDlg::CProcedureDlg(CWnd* pParent)
	: CNxDialog(CProcedureDlg::IDD, pParent /*= NULL*/),
	m_procedureNameChecker(NetUtils::AptPurposeT),
	m_CPTCodeChecker(NetUtils::CPTCodeT),
	m_ProductChecker(NetUtils::Products),
	m_ladderChecker(NetUtils::Ladders),
	m_ContactChecker(NetUtils::ContactsT)
{
	//{{AFX_DATA_INIT(CProcedureDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Scheduler_Setup/add_procedures.htm";
}


void CProcedureDlg::DoDataExchange(CDataExchange* pDX)
{
	// (z.manning, 05/12/2008) - PLID 29702 - Converted custom fields from rich text to edit controls
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureDlg)
	DDX_Control(pDX, IDC_RADIO_PROCEDURE_CPT_CODES, m_btnServiceCodes);
	DDX_Control(pDX, IDC_CHECK_RECUR, m_btnRecurring);
	DDX_Control(pDX, IDC_MASTER_PROC, m_btnMasterProc);
	DDX_Control(pDX, IDC_ADVANCED_LADDER_ASSIGNMENT, m_btnAdvancedLadderAssignment);
	DDX_Control(pDX, IDC_PROC_EMR_ACTION, m_btnProcEmrAction);
	DDX_Control(pDX, IDC_NEW, m_btnNew);
	DDX_Control(pDX, IDC_MAKE_NON_PROCEDURAL, m_btnMakeNonProcedural);
	DDX_Control(pDX, IDC_EDIT_SECTIONS_BTN, m_btnEditSections);
	DDX_Control(pDX, IDC_EDIT_PROC_GROUP, m_btnEditProcGroup);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_DEFAULT, m_btnDefault);
	DDX_Control(pDX, IDC_BTN_INACTIVATE, m_btnInactivate);
	DDX_Control(pDX, IDC_BTN_SHOW_INACTIVE_PROCEDURES, m_btnShowInactiveProcedures);
	DDX_Control(pDX, IDC_BTN_SETUP_DISCOUNTS, m_btnSetupDiscounts);
	DDX_Control(pDX, IDC_ANESTHESIA_SETUP, m_btnAnesthesiaSetup);
	DDX_Control(pDX, IDC_MULTI_LADDER, m_MultiLadder);
	DDX_Control(pDX, IDC_RIGHT_PROCEDURE, m_btnRightProcedure);
	DDX_Control(pDX, IDC_LEFT_PROCEDURE, m_btnLeftProcedure);
	DDX_Control(pDX, IDC_DETAIL_PROC, m_btnDetailProc);
	DDX_Control(pDX, IDC_RADIO_PROCEDURE_PRODUCTS, m_btnProcedureProducts);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_OFFICIAL_TERM, m_nxeditOfficialTerm);
	DDX_Control(pDX, IDC_CUSTOM1, m_nxeditCustom1);
	DDX_Control(pDX, IDC_ARRIVAL_TIME, m_nxeditArrivalTime);
	DDX_Control(pDX, IDC_LADDER_CAPTION, m_nxstaticLadderCaption);
	DDX_Control(pDX, IDC_GROUP_CAPTION, m_nxstaticGroupCaption);
	DDX_Control(pDX, IDC_MASTER_LIST_CAPTION, m_nxstaticMasterListCaption);
	DDX_Control(pDX, IDC_CUSTOM1_ADMIN, m_nxstaticCustom1Admin);
	DDX_Control(pDX, IDC_CUSTOM2_ADMIN, m_nxstaticCustom2Admin);
	DDX_Control(pDX, IDC_CUSTOM4_ADMIN, m_nxstaticCustom4Admin);
	DDX_Control(pDX, IDC_CUSTOM5_ADMIN, m_nxstaticCustom5Admin);
	DDX_Control(pDX, IDC_CUSTOM3_ADMIN, m_nxstaticCustom3Admin);
	DDX_Control(pDX, IDC_CUSTOM6_ADMIN, m_nxstaticCustom6Admin);
	DDX_Control(pDX, IDC_CUSTOM2, m_nxeditCustom2);
	DDX_Control(pDX, IDC_CUSTOM4, m_nxeditCustom4);
	DDX_Control(pDX, IDC_CUSTOM5, m_nxeditCustom5);
	DDX_Control(pDX, IDC_CUSTOM3, m_nxeditCustom3);
	DDX_Control(pDX, IDC_CUSTOM6, m_nxeditCustom6);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcedureDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProcedureDlg)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_MAKE_NON_PROCEDURAL, OnMakeNonProcedural)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	ON_BN_CLICKED(IDC_BTN_INACTIVATE, OnInactivate)
	ON_BN_CLICKED(IDC_BTN_SHOW_INACTIVE_PROCEDURES, OnShowInactiveProcedures)
	ON_BN_CLICKED(IDC_EDIT_PROC_GROUP, OnEditProcGroup)
	ON_BN_CLICKED(IDC_EDIT_SECTIONS_BTN, OnEditSectionsBtn)
	ON_BN_CLICKED(IDC_ANESTHESIA_SETUP, OnAnesthesiaSetup)
	ON_BN_CLICKED(IDC_MASTER_PROC, OnMasterProc)
	ON_BN_CLICKED(IDC_DETAIL_PROC, OnDetailProc)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_LEFT_PROCEDURE, OnLeftProcedure)
	ON_BN_CLICKED(IDC_RIGHT_PROCEDURE, OnRightProcedure)
	ON_BN_CLICKED(IDC_PROC_EMR_ACTION, OnProcEmrAction)
	ON_BN_CLICKED(IDC_BTN_SETUP_DISCOUNTS, OnBtnSetupDiscounts)
	ON_BN_CLICKED(IDC_CHECK_RECUR, OnCheckProcRecurs)
	ON_BN_CLICKED(IDC_RADIO_PROCEDURE_CPT_CODES, OnRadioProcedureCptCodes)
	ON_BN_CLICKED(IDC_RADIO_PROCEDURE_PRODUCTS, OnRadioProcedureProducts)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_ADVANCED_LADDER_ASSIGNMENT, OnAdvancedLadderAssignment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CProcedureDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProcedureDlg)
	ON_EVENT(CProcedureDlg, IDC_PROCEDURE, 16 /* SelChosen */, OnSelChosenProcedure, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_CPT_COMBO, 16 /* SelChosen */, OnSelChosenCptCombo, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_CPT_LIST, 7 /* RButtonUp */, OnRButtonUpCptList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_PROC_GROUP, 16 /* SelChosen */, OnSelChosenProcGroup, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_PROC_GROUP, 18 /* RequeryFinished */, OnRequeryFinishedProcGroup, VTS_I2)
	ON_EVENT(CProcedureDlg, IDC_NURSES, 16 /* SelChosen */, OnSelChosenNurses, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_ANESTH, 16 /* SelChosen */, OnSelChosenAnesth, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_DEF_ANESTHESIA, 16 /* SelChosen */, OnSelChosenDefAnesthesia, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_MASTER_PROCS, 16 /* SelChosen */, OnSelChosenMasterProcs, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_PROCEDURE_PRODUCT_COMBO, 16 /* SelChosen */, OnSelChosenProcedureProductCombo, VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_NURSES, 20 /* TrySetSelFinished */, OnTrySetSelFinishedNurses, VTS_I4 VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_ANESTH, 20 /* TrySetSelFinished */, OnTrySetSelFinishedAnesth, VTS_I4 VTS_I4)
	ON_EVENT(CProcedureDlg, IDC_LADDER_COMBO, 16 /* SelChosen */, OnSelChosenLadderCombo, VTS_DISPATCH)
	ON_EVENT(CProcedureDlg, IDC_LADDER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedLadderCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcedureDlg message handlers

/*
void CProcedureDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CRect rect = &(lpDrawItemStruct->rcItem);
	CDC *pDC	= CDC::FromHandle(lpDrawItemStruct->hDC);

	if (nIDCtl != IDC_COLOR)
	{	CNxDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return;
	}
	
	POINT pt;
	CBrush br, *temp;
	br.CreateSolidBrush(m_color);
	pt.x = pt.y = 5;
	temp = (CBrush *)pDC->SelectObject(br);
	pDC->RoundRect(rect, pt);
	pDC->SelectObject(temp);
}*/

BOOL CProcedureDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_rightClicked = 0;

		m_cptList = BindNxDataListCtrl(IDC_CPT_LIST, false);//needs a selection to load
		m_cptCombo = BindNxDataListCtrl(IDC_CPT_COMBO);
		m_productCombo = BindNxDataListCtrl(IDC_PROCEDURE_PRODUCT_COMBO);
		m_ladderCombo = BindNxDataList2Ctrl(IDC_LADDER_COMBO);
		m_procedureCombo = BindNxDataListCtrl(IDC_PROCEDURE);

		m_pProcGroupCombo = BindNxDataListCtrl(IDC_PROC_GROUP);

		m_pNurse = BindNxDataListCtrl(IDC_NURSES);
		m_pAnesth = BindNxDataListCtrl(IDC_ANESTH);

		IRowSettingsPtr pRow = m_pNurse->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Default Nurse>"));
		m_pNurse->InsertRow(pRow, 0);

		pRow = m_pAnesth->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Default Anesthesiologist>"));
		m_pAnesth->InsertRow(pRow, 0);

		m_btnLeftProcedure.AutoSet(NXB_LEFT);
		m_btnLeftProcedure.EnableWindow(FALSE);
		m_btnRightProcedure.AutoSet(NXB_RIGHT);
		if(m_procedureCombo->CurSel == m_procedureCombo->GetRowCount()-1) {
			m_btnRightProcedure.EnableWindow(FALSE);
		}

		// (z.manning, 04/16/2008) - PLID 29566 - Set button styles
		m_btnNew.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnMakeNonProcedural.AutoSet(NXB_DELETE);		
		m_btnDefault.AutoSet(NXB_MODIFY);
		m_btnInactivate.AutoSet(NXB_MODIFY); // (c.haag 2008-12-03 17:54) - PLID 10776
		m_btnSetupDiscounts.AutoSet(NXB_MODIFY);
		m_btnProcEmrAction.AutoSet(NXB_MODIFY);
		m_btnEditSections.AutoSet(NXB_MODIFY);

		m_pAnesthesia = BindNxDataListCtrl(IDC_DEF_ANESTHESIA);

		pRow = m_pAnesthesia->GetRow(-1);
		pRow->PutValue(0, _bstr_t("<No Default Anesthesia>"));
		m_pAnesthesia->InsertRow(pRow, 0);

		m_procedureCombo->CurSel = 0;

		m_pMasterProcList = BindNxDataListCtrl(IDC_MASTER_PROCS);

		m_color = GetNxColor(GNC_ADMIN, 0);
		m_brush.CreateSolidBrush(PaletteColor(m_color));

		// (d.moore 2007-06-11 10:52) - PLID 14670 - NxLabel used when multiple ladders are selected.
		// Hyperlink for displaying multiple selected purpose values.
		m_MultiLadder.SetText("");
		m_MultiLadder.SetType(dtsHyperlink);
		ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
		m_MultiLadder.SetColor(m_color);
		// (d.moore 2007-06-11 12:49) - PLID 14670 - Make sure that there are no stored Ladder ID values.
		m_rgLadderID.RemoveAll();

		CheckRadioButton(IDC_RADIO_PROCEDURE_CPT_CODES, IDC_RADIO_PROCEDURE_PRODUCTS, IDC_RADIO_PROCEDURE_CPT_CODES);
		OnRadioCodeTypeChanged();

		//DRT 5/11/2004 - PLID 12273 - Change the displayed text depending
		//	on your NexForms license.
		CLicense::ECheckForLicenseReason ecflr = CLicense::cflrSilent;
		if (g_pLicense->CheckForLicense(CLicense::lcNexForms, ecflr))
			SetDlgItemText(IDC_EDIT_SECTIONS_BTN, "NexF&orms Content");
		else
			SetDlgItemText(IDC_EDIT_SECTIONS_BTN, "F&orms Content");


		if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)){
			GetDlgItem(IDC_DEFAULT)->ShowWindow(SW_HIDE);
			//(e.lally 2009-08-10) PLID 35156 - Make detail/master setup available to everyone.
			//GetDlgItem(IDC_MASTER_PROC)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_DETAIL_PROC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROC_GROUP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_PROC_GROUP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_GROUP_CAPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LADDER_CAPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LADDER_COMBO)->ShowWindow(SW_HIDE);
			//(e.lally 2009-08-10) PLID 35158 - Hide the advanced ladder setup button too
			GetDlgItem(IDC_ADVANCED_LADDER_ASSIGNMENT)->ShowWindow(SW_HIDE);
		}

		if(!g_pLicense || !g_pLicense->HasEMR(CLicense::cflrSilent)) {
			GetDlgItem(IDC_PROC_EMR_ACTION)->ShowWindow(SW_HIDE);
		}
		else {
			if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
				SetDlgItemText(IDC_PROC_EMR_ACTION, "EMR Action...");
			}
			else {
				SetDlgItemText(IDC_PROC_EMR_ACTION, "Custom Record Action...");
			}
		}

		((CNxEdit*)GetDlgItem(IDC_NAME))->SetLimitText(100);
		((CNxEdit*)GetDlgItem(IDC_OFFICIAL_TERM))->SetLimitText(255);
		// (z.manning, 09/07/2007) - PLID 18629 - All ProcedureT.Custom1 - Custom6 are now ntext, so we
		// don't need to limit the amount of text that the user can enter.
		//((CNxEdit*)GetDlgItem(IDC_CUSTOM1))->SetLimitText(255);

		EnableAppropriateFields();

	} NxCatchAll("Error In: CProcedureDlg::OnInitDialog");
	return TRUE;
}

bool CProcedureDlg::GetProcedureID (long &id)
{
	int curSel = m_procedureCombo->CurSel;

	if (curSel == -1)
		return false;

	id = VarLong(m_procedureCombo->Value[curSel][0], -1);

	if (id == -1)
		return false;
	else return true;
}

/* (d.moore 2007-06-11 12:51) - PLID 14670 - Function is no longer applicable.
//  It is now possible to have several ladders selected at once.
bool CProcedureDlg::GetLadderID(long &ladder)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_ladderCombo->CurSel;

	if (pRow == NULL)
		return false;

	ladder = VarLong(pRow->GetValue(0), -1);

	if (ladder == -1)
		return false;
	else return true;
}
*/

void CProcedureDlg::Refresh()
{
	try
	{
		_RecordsetPtr rs;
		CString where;
		long id;

		if (!GetProcedureID(id))
			return;

		EnableAppropriateFields();

		//BVB, moved these here, so they are done before the refresh
		if (m_ladderChecker.Changed()) {
			m_ladderCombo->Requery();
			// (d.moore 2007-06-11 12:42) - PLID 14670 - 
			// The function: OnRequeryFinishedLadderCombo() was added to handle 
			// everything that needs to be done after the list requeries.
		}

		if(m_CPTCodeChecker.Changed())
			m_cptCombo->Requery();

		if(m_ProductChecker.Changed())
			m_productCombo->Requery();

		if(m_ContactChecker.Changed()) {
			m_pNurse->Requery();
			m_pAnesth->Requery();

			IRowSettingsPtr pRow = m_pNurse->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _bstr_t("<No Default Nurse>"));
			m_pNurse->InsertRow(pRow, 0);

			pRow = m_pAnesth->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _bstr_t("<No Default Anesthesiologist>"));
			m_pAnesth->InsertRow(pRow, 0);
		}

		m_pAnesthesia->Requery();

		IRowSettingsPtr pRow = m_pAnesthesia->GetRow(-1);
		pRow->PutValue(0, _bstr_t("<No Default Anesthesia>"));
		m_pAnesthesia->InsertRow(pRow, 0);

		// (d.moore 2007-06-11 15:02) - PLID 14670 - Removed reference to LadderTemplateID.
		rs = CreateRecordset("SELECT "
			"ProcedureT.Name, Custom1, Custom2, Custom3, Custom4, Custom5, Custom6, Color, ProcedureGroupID, NurseID, AnesthesiologistID, Anesthesia, ArrivalPrepMinutes, MasterProcedureID, OfficialName, Recur "
			"FROM ProcedureT LEFT JOIN AptPurposeT ON ProcedureT.ID = AptPurposeT.ID "
			"WHERE ProcedureT.ID = %i", id);
		FieldsPtr f = rs->Fields;

		if(rs->eof) {
			HandleException(NULL,"The selected procedure could not be found.");
			return;
		}

		SetDlgItemText(IDC_NAME,		AdoFldString(f, "Name"));
		SetDlgItemText(IDC_CUSTOM1,		AdoFldString(f, "Custom1",""));
		SetDlgItemText(IDC_CUSTOM2,		AdoFldString(f, "Custom2",""));
		SetDlgItemText(IDC_CUSTOM3,		AdoFldString(f, "Custom3",""));
		SetDlgItemText(IDC_CUSTOM4,		AdoFldString(f, "Custom4",""));
		SetDlgItemText(IDC_CUSTOM5,		AdoFldString(f, "Custom5",""));
		SetDlgItemText(IDC_CUSTOM6,		AdoFldString(f, "Custom6",""));
		SetDlgItemText(IDC_ANESTHESIA_ADMIN,	AdoFldString(f, "Anesthesia"));
		SetDlgItemInt(IDC_ARRIVAL_TIME,	AdoFldLong(f, "ArrivalPrepMinutes"));

		//load the nurse and anesthesiologist combos
		//(e.lally 2005-11-28) PLID 18153 - add the ability to make Other contacts inactive
		long nID;
		nID = VarLong(f->Item["NurseID"]->Value, -1);
		if(nID > 0){
			if(m_pNurse->TrySetSelByColumn(0, nID) == -1){
				m_pNurse->PutCurSel(-1);
				//they may have an inactive nurse
				_RecordsetPtr rsNurse = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nID);
				if(!rsNurse->eof) {
					m_pNurse->PutComboBoxText(_bstr_t(AdoFldString(rsNurse, "Name", "")));
				}
			}
		}
		else
			m_pNurse->CurSel = -1;

		nID = VarLong(f->Item["AnesthesiologistID"]->Value, -1);
		if(nID > 0){
			if(m_pAnesth->TrySetSelByColumn(0, nID) == -1){
				m_pAnesth->PutCurSel(-1);
				//they may have an inactive anesthesiologist
				_RecordsetPtr rsAnesth = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nID);
				if(!rsAnesth->eof) {
					m_pAnesth->PutComboBoxText(_bstr_t(AdoFldString(rsAnesth, "Name", "")));
				}
			}
		}
		else
			m_pAnesth->CurSel = -1;
		//

		m_pAnesthesia->SetSelByColumn(0, f->Item["Anesthesia"]->Value);

		//PLID 16974
		CheckDlgButton(IDC_CHECK_RECUR, AdoFldBool(f, "Recur"));

		//load the custom field names
		_RecordsetPtr rsCustom(__uuidof(Recordset));
		CString sql;
		for(int i=63; i <= 68; i++) {
			rsCustom = CreateRecordset("SELECT Name FROM CustomFieldsT Where ID = %d",i);
			if (!rsCustom->eof) {
				switch(i) {
				case 63: SetDlgItemText(IDC_CUSTOM1_ADMIN, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
				case 64: SetDlgItemText(IDC_CUSTOM2_ADMIN, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
				case 65: SetDlgItemText(IDC_CUSTOM3_ADMIN, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
				case 66: SetDlgItemText(IDC_CUSTOM4_ADMIN, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
				case 67: SetDlgItemText(IDC_CUSTOM5_ADMIN, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
				case 68: SetDlgItemText(IDC_CUSTOM6_ADMIN, ConvertToControlText(CString(rsCustom->Fields->Item["Name"]->Value.bstrVal))); break;
				}
			}
			rsCustom->Close();
		}
		//

		// (d.moore 2007-06-11 15:16) - PLID 14670 - Get any values selected for the ladders datalist.
		m_rgLadderID.RemoveAll();
		//(e.lally 2009-08-10) PLID 35158 - Only do this if they have tracking
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)){
			CString strLadderNames;
			_RecordsetPtr rsLadder = CreateRecordset(
				"SELECT ProcedureLadderTemplateT.LadderTemplateID, LadderTemplatesT.NAME "
				"FROM ProcedureLadderTemplateT INNER JOIN LadderTemplatesT "
				"ON ProcedureLadderTemplateT.LadderTemplateID = LadderTemplatesT.ID "
				"WHERE ProcedureLadderTemplateT.ProcedureID=%li "
				"ORDER BY LadderTemplatesT.NAME", id);
			long nLadderTemplateID;
			while (!rsLadder->eof) {
				nLadderTemplateID = AdoFldLong(rsLadder, "LadderTemplateID", 0);
				if (nLadderTemplateID > 0) {
					m_rgLadderID.Add(nLadderTemplateID);
					strLadderNames += AdoFldString(rsLadder, "NAME", "") + ", ";
				}
				rsLadder->MoveNext();
			}
			// Select the selection value for the ladder template datalist, or set the label text if
			//  there is more than one selection.
		
			if (m_rgLadderID.GetSize() == 0) {
				// No selection.
				GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
				m_ladderCombo->SetSelByColumn(0, (long)-1);
				m_MultiLadder.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
				ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
				InvalidateDlgItem(IDC_MULTI_LADDER);
			}
			else if (m_rgLadderID.GetSize() == 1) {
				GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
				m_ladderCombo->SetSelByColumn(0, m_rgLadderID[0]);
				m_MultiLadder.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
				ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
				InvalidateDlgItem(IDC_MULTI_LADDER);
			} else {
				strLadderNames = strLadderNames.Left(strLadderNames.GetLength()-2);
				m_MultiLadder.SetText(strLadderNames);
				m_MultiLadder.SetType(dtsHyperlink);
				ShowDlgItem(IDC_LADDER_COMBO, SW_HIDE);
				ShowDlgItem(IDC_MULTI_LADDER, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LADDER);
			}
		}

		where.Format("ProcedureID = %li", id);
		m_cptList->WhereClause = _bstr_t(where);
		m_cptList->Requery();

		SetDlgItemText(IDC_OFFICIAL_TERM, AdoFldString(f, "OfficialName"));

		m_pMasterProcList->Requery();

		if(rs->Fields->GetItem("MasterProcedureID")->Value.vt == VT_NULL) {
			//This is a master record.
			CheckRadioButton(IDC_MASTER_PROC, IDC_DETAIL_PROC, IDC_MASTER_PROC);
			OnMasterProc();
		}
		else {
			//This is a detail record
			CheckRadioButton(IDC_MASTER_PROC, IDC_DETAIL_PROC, IDC_DETAIL_PROC);
			OnDetailProc();
		}

		//in case we, or another user, changed the name, or added or deleted
		if (m_procedureNameChecker.Changed())
		{	
			m_procedureCombo->Requery();
			m_procedureCombo->SetSelByColumn(0, id);
			if (m_procedureCombo->CurSel == -1)
			{	m_procedureCombo->CurSel = 0;
				UpdateView();//potiential infinite loop
			}
		}

		//Double-check the enabling of the arrows
		if(m_procedureCombo->CurSel < 1) {
			m_btnLeftProcedure.EnableWindow(FALSE);
		}
		else {
			m_btnLeftProcedure.EnableWindow(TRUE);
		}
		if(m_procedureCombo->GetRowCount() == 0 || (m_procedureCombo->CurSel < m_procedureCombo->GetRowCount()-1 && m_procedureCombo->CurSel != -1)) {
			m_btnRightProcedure.EnableWindow(TRUE);
		}
		else {
			m_btnRightProcedure.EnableWindow(FALSE);
		}		
	}
	NxCatchAll("Could not refresh procedure tab");
}

void CProcedureDlg::OnSelChosenProcedure(long nRow) 
{

	try {
		if(nRow == -1) {
			//make sure there are rows
			if(m_procedureCombo->GetRowCount() > 0) {
				//no selection - set it to the first row
				m_procedureCombo->PutCurSel(0);
				nRow = 0;
				//and let it continue from there
			}
			else {
				//we can't refresh, there are no items
				return;
			}
		}

		Refresh();
	} NxCatchAll("Error In: CProcedureDlg::OnSelChosenProcedure");
}

void CProcedureDlg::OnRemoveService(int serviceID)
{
	try
	{
		ExecuteSql("UPDATE ServiceT SET ProcedureID = NULL WHERE ID = %i", serviceID);
		Refresh();
	}
	NxCatchAll("Could not remove Service Code from procedure");
}

void CProcedureDlg::Save(int nID)
{
	CString field;
	long id;
	CString value;

	if (!GetProcedureID(id))
		return;

	GetDlgItemText(nID, value);

	switch (nID)
	{	case IDC_NAME:
		{
			CString strOld = CString(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 1).bstrVal);
			field = "Name";
			value = value.Left(100);
			if(strOld == value)
				return;

			// (z.manning, 08/07/2007) - PLID 26967 - Warn that changing this will have repercusions throughout
			// the program. (Note: normally we should check to make sure this procedure is actually used on 
			// something, but since procedures are used in so many different places, let's just want them no
			// matter what.)
			// (z.manning, 10/17/2007) - PLID 26967 - Ok, per c.majeed, we now warn twice here. Also, this text
			// came pretty much verbatim from her.
			int nResult = MessageBox("WARNING: Changing the name of this procedure will change any existing "
				"Locked and Unlocked EMN’s, Appointments, Tracking Ladders, Reports, Marketing, and other areas "
				"where procedures are used inside the program. ARE SURE YOU WANT TO CHANGE THIS "
				"PROCEDURE NAME?", "Rename Procedure?", MB_YESNO|MB_ICONWARNING);
			if(nResult == IDYES) {
				nResult = MessageBox("ARE YOU ABSOLUTELY CERTAIN YOU WISH TO CHANGE THE NAME OF THIS PROCEDURE, "
					"WHICH WILL CHANGE ALL EXISTING DATA THAT USES THIS PROCEDURE?", "Rename Procedure?", MB_YESNO|MB_ICONEXCLAMATION);
			}

			if(nResult != IDYES) {
				SetDlgItemText(IDC_NAME, strOld);
				return;
			}

			m_procedureCombo->Value[m_procedureCombo->CurSel][1] = _bstr_t(value);
			
			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiSchedProc, id, strOld, value, aepMedium, aetChanged);

			m_procedureNameChecker.Refresh(id);
		}
			break;

		// (z.manning, 06/06/2007) - PLID 18629 - The 6 custom fields used to restrict the amount of text
		// here based on the size of the field in the db. However, these fields are now ntext, so I removed
		// the size restrictions.
		case IDC_CUSTOM1:
			field = "Custom1";
			break;
		case IDC_CUSTOM2:
			field = "Custom2";
			break;
		case IDC_CUSTOM3:
			field = "Custom3";
			break;
		case IDC_CUSTOM4:
			field = "Custom4";
			break;
		case IDC_CUSTOM5:
			field = "Custom5";
			break;
		case IDC_CUSTOM6:
			field = "Custom6";
			break;

		case IDC_ANESTHESIA_ADMIN:
			field = "Anesthesia";
			break;

		case IDC_ARRIVAL_TIME:
			if(value == "") value = "0";
			field = "ArrivalPrepMinutes";
			break;

		case IDC_OFFICIAL_TERM:
			field = "OfficialName";
			break;
	}
	try
	{
		ExecuteSql ("UPDATE ProcedureT SET %s = '%s' WHERE ID = %i", field, _Q(value), id);

		if (field == "Name")
			ExecuteSql ("UPDATE AptPurposeT SET Name = '%s' WHERE ID = %i", _Q(value), id);
	}
	NxCatchAll("could not update Procedure");
}

BOOL CProcedureDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		int nID = LOWORD(wParam);
		
		CString strNewName;

		switch (HIWORD(wParam))
		{	case EN_CHANGE:
				switch (nID)
				{	case IDC_NAME:
					case IDC_CUSTOM1:
					case IDC_CUSTOM2:
					case IDC_CUSTOM3:
					case IDC_CUSTOM4:
					case IDC_CUSTOM5:
					case IDC_CUSTOM6:
					case IDC_OFFICIAL_TERM:
						m_changed = true;
					default:
						break;
				}//end changed
				break;
			case EN_KILLFOCUS:
				switch (nID)
				{	
				case IDC_NAME:

					if(m_procedureCombo->CurSel == -1) {
						m_changed = false;
						return CNxDialog::OnCommand(wParam, lParam);
					}

					GetDlgItemText(IDC_NAME, strNewName);
					//(e.lally 2008-10-13) PLID 31665 - Put format string inside ReturnsRecords params.
					// (c.haag 2008-12-18 15:27) - PLID 10776 - Show a graceful message if the name corresponds to an inactive procedure
					{
						_RecordsetPtr prs = CreateParamRecordset("SELECT Inactive FROM AptPurposeT LEFT JOIN ProcedureT ON ProcedureT.ID = AptPurposeT.ID WHERE AptPurposeT.Name = {STRING} AND AptPurposeT.ID <> {INT} "
							"UNION SELECT convert(bit,0) AS Inactive FROM ProcedureGroupsT WHERE Name = {STRING}", strNewName, VarLong(m_procedureCombo->Value[m_procedureCombo->CurSel][0]), strNewName);
						if (!prs->eof) {
							if (AdoFldBool(prs, "Inactive", FALSE)) {
								MessageBox("There is already an inactive procedure with this name. Please use another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
							} else {
								MsgBox("All Purposes, Procedures and Procedure Groups must have unique names");
							}
							SetDlgItemText(IDC_NAME, VarString(m_procedureCombo->Value[m_procedureCombo->CurSel][1]));
							m_changed = false;
							return CNxDialog::OnCommand(wParam, lParam);
						}
					}
					
					case IDC_CUSTOM1:
					case IDC_CUSTOM2:
					case IDC_CUSTOM3:
					case IDC_CUSTOM4:
					case IDC_CUSTOM5:
					case IDC_CUSTOM6:
					case IDC_OFFICIAL_TERM:
						m_changed = false;
						Save(nID);
						break;

					case IDC_ANESTHESIA_ADMIN:
					case IDC_ARRIVAL_TIME:
						Save(nID);
						break;

					default:
						break;
				}//end killfocus
				break;
			case 0:
			{
				if (nID == IDM_DELETE)
					OnRemoveService(m_rightClicked);
				break;
			}
		}//	end wParam
	} NxCatchAll("Error In: CProcedureDlg::OnCommand");
	return CNxDialog::OnCommand(wParam, lParam);
}

void CProcedureDlg::OnRButtonUpCptList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		long id;

		if (nRow == -1 || !GetProcedureID(id))
			return;
		
		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		menPopup.InsertMenu(1, MF_BYPOSITION, IDM_DELETE, "Remove");

		CPoint pt(x,y);
		CWnd* pWnd = GetDlgItem(IDC_CPT_LIST);
		if (pWnd != NULL)
		{	pWnd->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			m_rightClicked = VarLong(m_cptList->Value[nRow][0]);
		}
		else HandleException(NULL, "An error ocurred while creating menu");
	} NxCatchAll ("Error In: CProcedureDlg::OnRButtonUpCptList");
}

void CProcedureDlg::OnSelChosenLadderCombo(LPDISPATCH lpRow) 
{
	// (d.moore 2007-06-07 15:18) - PLID 14670 - Converted IDC_LADDER_COMBO to NxDataList2
	//  Also added functionality to select multiple ladders for a procedure.
	long nProcID;
	if (!GetProcedureID(nProcID))
		return;

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		long nLadderID;
		if (pRow == NULL){
			nLadderID = -1;
		} else {
			nLadderID = VarLong(pRow->GetValue(0), -1);
		}
		
		if (nLadderID == -2) {
			// Selected multiple option.
			//(e.lally 2010-07-28) PLID 36199 - always use the advanced configuration now
			OnAdvancedLadderAssignment();
			/*
			OpenLadderMultiList(m_rgLadderID);
			CString strQuery, strInsertQ;
			strQuery.Format("DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID=%li;", nProcID);
			for (long i = 0; i < m_rgLadderID.GetSize(); i++) {
				strInsertQ.Format(
					"INSERT INTO ProcedureLadderTemplateT(ProcedureID, LadderTemplateID) "
					"VALUES(%li, %li)", nProcID, m_rgLadderID[i]);
				strQuery += strInsertQ;
			}
			ExecuteSqlStd(strQuery);
			*/
		}
		else if (nLadderID > 0){
			// Should be just a single selection.
			m_rgLadderID.RemoveAll();
			m_rgLadderID.Add(nLadderID);
			ExecuteSql(
				"DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID=%li;"
				"INSERT INTO ProcedureLadderTemplateT(ProcedureID, LadderTemplateID) "
				"VALUES(%li, %li);", nProcID, nProcID, nLadderID);
		} else {
			// No Selection
			m_rgLadderID.RemoveAll();
			ExecuteSql("DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID=%li", nProcID);
		}
	}
	NxCatchAll("Could not save phase tracking ladder");
}

void CProcedureDlg::OnNew() 
{
	CString name;
	CGetNewIDName dlg(this);
	dlg.m_nMaxLength = 100;
	dlg.m_pNewName = &name;
	dlg.m_strCaption = "Enter a new procedure name";

	if (IDOK != dlg.DoModal())
		return;
	
	name.TrimRight();

	while(name == ""){		//can't enter an empty name!
		AfxMessageBox("You must enter a name for this procedure.");
		if (IDOK != dlg.DoModal())
			return;
		name.TrimRight();
	}
	
	//TODO:  if it is not possible to remove from 1 table without removing from the other, we 
	//		only need one of these checks
	//make sure no procedures exist with this name already
	_RecordsetPtr rs;
	rs = CreateParamRecordset("SELECT Inactive FROM ProcedureT WHERE Name = {STRING} UNION SELECT convert(bit, 0) FROM ProcedureGroupsT WHERE Name = {STRING}", name, name);
	if(!rs->eof)
	{
		// (c.haag 2008-12-18 15:20) - PLID 10776 - Show a graceful message if the procedure is inactive
		if (AdoFldBool(rs, "Inactive", FALSE)) {
			MessageBox("There is already an inactive procedure with this name. Please choose another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
		} else {
			MessageBox("There is already a procedure or procedure group with this name. Please choose another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
		}
		rs->Close();
		return;
	}
	rs->Close();

	rs = CreateParamRecordset("SELECT Name FROM AptPurposeT WHERE Name = {STRING}", name);
	if(!rs->eof)
	{
		MessageBox("There is already an appointment purpose with this name. Please choose another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
		rs->Close();
		return;
	}
	rs->Close();
	
	try
	{
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("New Procedure");
		trans.Begin();

		long id = NewNumber("AptPurposeT", "ID");
		ExecuteSql("INSERT INTO AptPurposeT (ID, Name) SELECT %i, '%s'", id, _Q(name));
		ExecuteSql("INSERT INTO ProcedureT (ID, Name) SELECT %i, '%s'", id, _Q(name));

		trans.Commit();

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiSchedProcCreated, id, "", name, aepMedium, aetCreated);

		m_procedureCombo->Requery();
		m_procedureCombo->SetSelByColumn(0, id);
		m_procedureNameChecker.Refresh(id);
		// (j.jones 2006-05-17 15:58) - the tablechecker will cause UpdateView() to fire
		//UpdateView();
		EnableAppropriateFields();
		return;
	}
	NxCatchAll("Could not create procedure");


}

void CProcedureDlg::OnDelete() 
{
	long id;

	if (!GetProcedureID(id))
		return;

	try
	{
		//First of all, a basic confirmation.
		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to delete this procedure?"))
			return;

		_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 AppointmentPurposeT.PurposeID "
			"FROM AppointmentPurposeT RIGHT OUTER JOIN "
			"AppointmentsT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
			"WHERE (AppointmentPurposeT.PurposeID = %d) AND (AppointmentsT.Status <> 4)", id);

		// (s.tullis 2014-12-12 08:41) - PLID 64440
		if (ReturnsRecordsParam("Select AptPurposeID FROM ScheduleMixRuleDetailsT WHERE AptPurposeID = {INT}", id))
		{
			AfxMessageBox("You may not delete a procedure that exists in a Scheduling Mix Rule Template.");
			return;
		}

		if (!rs->eof) {
			AfxMessageBox("You may not delete a procedure that has been scheduled or exists as a cancelled appointment.\n"
			  "         Use the find/replace on the scheduler setup tab to change existing appointments.");
			return;
		}
	
		// (j.gruber 2008-05-28 12:18) - PLID 28703 - added a check for waiting list
		//most of these will be caught with the above, but you can have a waiting list entry without an appt
		if (ReturnsRecords("SELECT top 1 ID FROM WaitingListPurposeT WHERE PurposeID = %li", id)) {
			AfxMessageBox("You may not delete a procedure that exists in the waiting list.");
			return;
		}

		// (b.cardillo 2011-02-26 11:01) - This should have been closed or just put in its own code block, but wasn't.
		rs->Close();

		// (b.cardillo 2011-02-26 10:15) - PLID 40419 - Check for appointment prototypes that reference this aptpurpose
		{
			// Get the list of prototypes, if any, that depend on this aptpurpose
			CString strReferencedPrototypes = GenerateDelimitedListFromRecordsetColumn(CreateParamRecordset(
					_T("SELECT Name FROM ApptPrototypeT WHERE ID IN (\r\n")
					_T(" SELECT PS.ApptPrototypeID \r\n")
					_T(" FROM ApptPrototypePropertySetT PS \r\n")
					_T(" INNER JOIN ApptPrototypePropertySetAptPurposeSetT PSAPS ON PS.ID = PSAPS.ApptPrototypePropertySetID \r\n")
					_T(" INNER JOIN ApptPrototypePropertySetAptPurposeSetDetailT PSAPSD ON PSAPS.ID = PSAPSD.ApptPrototypePropertySetAptPurposeSetID \r\n")
					_T(" WHERE PSAPSD.AptPurposeID = {INT} \r\n")
					_T(") \r\n")
					_T("ORDER BY Name \r\n")
					, id)
				, AsVariant("Name"), "", "\r\n");
			if (!strReferencedPrototypes.IsEmpty()) {
				AfxMessageBox(FormatString(
					_T("You may not delete this procedure because it is referenced by the following Appointment Prototypes:\r\n\r\n%s")
					, strReferencedPrototypes));
				return;
			}
		}

		if(!CanDeleteProcedure(id))
			return;

		// (s.tullis 2015-07-08 11:17) - PLID 63851 - Check and warn if this purpose is associated with a Template rule details 
		if (!CheckWarnTemplateRuleDetails(FALSE, id))
		{
			return;
		}

		// (b.spivey - February 4th, 2014) - PLID 60563 - If we have default durations, warn before deleting.
		if (ReturnsRecordsParam("SELECT * FROM ProviderSchedDefDurationDetailT WHERE AptPurposeID = {INT} ", id)
			&& AfxMessageBox("Practice has detected default duration sets associated with this procedure as an appointment purpose. "
				"If you delete this procedure then any default durations associated with it will be removed as well. Do you still wish to delete this data?", MB_YESNO|MB_ICONWARNING) != IDYES) {
			return;
		}

	} NxCatchAllCall("Could not validate procedure", return);

	try
	{
		//if there are no EMRs, this is safe to delete
		CString strOld = CString(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 1).bstrVal);

		CString strSql = BeginSqlBatch();
		// (s.tullis 2015-07-08 11:17) - PLID 63851 - Remove the Detail from the rule
		AddStatementToSqlBatch(strSql, "DELETE FROM TemplateRuleDetailsT WHERE (ObjectType =2 OR ObjectType = 102 ) AND ObjectID = %li ", id);
		AddStatementToSqlBatch(strSql, "UPDATE MedScheduleT SET ProcedureID = NULL WHERE ProcedureID = %li", id); // (z.manning, 06/13/2007) - PLID 26317 - Update MedScheduleT.ProcedureID
		AddStatementToSqlBatch(strSql, "DELETE FROM EmrDefaultTemplateProceduresT WHERE ProcedureID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM EMRTemplateProceduresT WHERE ProcedureID = %li", id);
		AddStatementToSqlBatch(strSql, "UPDATE AppointmentsT SET AptPurposeID = NULL WHERE AptPurposeID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM AppointmentPurposeT WHERE PurposeID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM CaseHistoryProceduresT WHERE ProcedureID = %li", id);
		// (j.jones 2009-08-31 15:48) - PLID 17732 - remove links to Preference Cards
		AddStatementToSqlBatch(strSql, "DELETE FROM PreferenceCardProceduresT WHERE ProcedureID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM ProcedureToEMRInfoT WHERE ProcedureID = %li",id);
		AddStatementToSqlBatch(strSql, "UPDATE ServiceT SET ProcedureID = NULL WHERE ProcedureID = %li", id);
		// (j.jones 2009-10-12 15:51) - PLID 35894 - removed MailSent.ProcedureID
		//AddStatementToSqlBatch(strSql, "UPDATE MailSent SET ProcedureID = NULL WHERE ProcedureID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM CredentialedProceduresT WHERE ProcedureID = %li", id);
		// (b.spivey - February 4th, 2014) - PLID 60563 - If we have default durations, delete the whole set. Not just parts of it.
		AddStatementToSqlBatch(strSql,
					"DECLARE @DelTable TABLE "
					"( "
					"	DefaultDurationSetID INT NOT NULL "
					") "
					"	"
					"INSERT INTO @DelTable "
					"SELECT ProviderSchedDefDurationID FROM ProviderSchedDefDurationDetailT WHERE AptPurposeID = %li "
					"	"
					"DELETE Del "
					"FROM ProviderSchedDefDurationDetailT Del "
					"INNER JOIN @DelTable DelT ON Del.ProviderSchedDefDurationID = DelT.DefaultDurationSetID "
					"	"
					"DELETE Del "
					"FROM ProviderSchedDefDurationT Del "
					"INNER JOIN @DelTable DelT ON Del.ID = DelT.DefaultDurationSetID ", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM RefPhysProcLinkT WHERE ProcedureID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM ProcInfoDetailsT WHERE ProcedureID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM ProcedureDiscountsT WHERE ProcedureID = %li", id);
		// (j.gruber 2007-01-12 16:52) - PLID 23673 - Delete from ConfigureAAFPRSSurveyT
		AddStatementToSqlBatch(strSql, "DELETE FROM ConfigureAAFPRSSurveyT WHERE ProcedureID = %li", id);
		// (d.moore 2007-06-18 16:39) - PLID 14670 - We have to delete from ProcedureLadderTemplateT before deleting from ProcedureT
		AddStatementToSqlBatch(strSql, "DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID = %li", id);
		//(e.lally 2008-04-02) - PLID 27841 - Delete any booking alarm detail entries for this purpose.
		AddStatementToSqlBatch(strSql, "DELETE FROM AptBookAlarmDetailsT WHERE AptPurposeID = %li", id);
		// (c.haag 2009-10-12 12:54) - PLID 35722 - MailSentProcedureT
		AddStatementToSqlBatch(strSql, "DELETE FROM MailSentProcedureT WHERE ProcedureID = %li", id);
		//(e.lally 2011-05-02) PLID 43481 - NexWebDisplayT
		AddStatementToSqlBatch(strSql, "DELETE FROM NexWebDisplayT WHERE ProcedureID = %li", id);

		AddStatementToSqlBatch(strSql, "DELETE FROM ProcedureT WHERE ID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM ResourcePurposeTypeT WHERE AptPurposeID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM AptPurposeTypeT WHERE AptPurposeID = %li", id);
		//TES 6/12/2008 - PLID 28078 - Since we're deleting this purpose, we're not going to require allocations for it any more.
		AddStatementToSqlBatch(strSql, "DELETE FROM ApptsRequiringAllocationsDetailT WHERE AptPurposeID = %d", id);
		// (a.walling 2010-06-15 15:49) - PLID 39184 - Clear our any resource set links using this purpose
		AddStatementToSqlBatch(strSql, "DELETE FROM AptResourceSetLinksT WHERE AptPurposeID = %li", id);
		AddStatementToSqlBatch(strSql, "DELETE FROM AptPurposeT WHERE ID = %li", id);
		// (c.haag 2006-11-15 09:28) - PLID 22825 - Delete any related EMRActionsT records
		AddStatementToSqlBatch(strSql, "DELETE FROM EMRActionsT WHERE DestType = 5 AND DestID = %d", id);

		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("Delete Procedure");
		trans.Begin();
		ExecuteSqlBatch(strSql);		
		trans.Commit();

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiSchedProcDeleted, id, strOld, "<Deleted>", aepMedium, aetDeleted);

		m_procedureNameChecker.Refresh(id);
	}
	NxCatchAll("Could not delete procedure");

	// (m.hancock 2006-08-03 09:23) - PLID 16781 - After deleting a row, we need to set the selection to the next procedure
	// in the list, not the first procedure.  To do this, we'll get the current selection, remove the row (not requery), then
	// set the selection back to what the current row was, which will be the next procedure from the one we deleted.
	long nCurSel = m_procedureCombo->CurSel;
	// (m.hancock 2006-10-02 16:31) - PLID 16781 - Check to see if the currently selected row is the last row.  If it is, then 
	// we should set control to the previous row, not the first row in the list.
	if(nCurSel+1 == m_procedureCombo->GetRowCount())
		nCurSel--;
	m_procedureCombo->RemoveRow(m_procedureCombo->CurSel);
	m_procedureCombo->CurSel = nCurSel;

	// (j.jones 2006-05-17 15:58) - the tablechecker will cause UpdateView() to fire
	//UpdateView();
	EnableAppropriateFields();
}

void CProcedureDlg::OnMakeNonProcedural() 
{
	long id;

	if (!GetProcedureID(id))
		return;

	try
	{
		if(!CanDeleteProcedure(id)) return;
	}
	NxCatchAllCall("Could not remove procedure", return);

	if (IDYES != AfxMessageBox("Deleting procedural data will transform this procedure into a non-procedural appointment purpose.  This cannot be undone.  Are you sure?", MB_YESNO))
		return;

	// (b.spivey - February 4th, 2014) - PLID 60563 - If we have default durations, warn before deleting.
	if (ReturnsRecordsParam("SELECT * FROM ProviderSchedDefDurationDetailT WHERE AptPurposeID = {INT} ", id)
		&& AfxMessageBox("Practice has detected default duration sets associated with this procedure as an appointment purpose. "
		"If you delete this procedural data then any default durations associated with it will be removed as well. Do you still wish to delete this data?", MB_YESNO|MB_ICONWARNING) != IDYES) {
		return;
	}

	try
	{
		//if there are no EMRs, this is safe to delete
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("Remove Procedure");
		trans.Begin();

		ExecuteSql("DELETE FROM ProcedureToEMRInfoT WHERE ProcedureID = %li",id);
		ExecuteSql("UPDATE ServiceT SET ProcedureID = NULL WHERE ProcedureID = %li", id);
		ExecuteSql("DELETE FROM CredentialedProceduresT WHERE ProcedureID = %li", id);
		ExecuteSql("DELETE FROM RefPhysProcLinkT WHERE ProcedureID = %li", id);
		//DRT 1/10/2004 - PLID 15236 - A record is placed in ProcInfoDetailsT whether this is in 
		//	use there or not.  We can delete the un-chosen ones - if there are chosen details, 
		//	the CanDeleteProcedure function should prevent us from reaching this point.
		ExecuteSql("DELETE FROM ProcInfoDetailsT WHERE ProcedureID = %li AND Chosen = 0", id);
		//
		
		// (c.haag 2006-11-15 09:28) - PLID 22825 - Delete any related EMRActionsT records
		ExecuteSql("DELETE FROM EMRActionsT WHERE DestType = 5 AND DestID = %d", id);

		// (j.gruber 2007-01-12 16:51) - PLID 23673 - delete from ConfigureAAFPRS table
		ExecuteSql("DELETE FROM ConfigureAAFPRSSurveyT WHERE ProcedureID = %li", id);

		ExecuteSql("DELETE FROM CaseHistoryProceduresT WHERE ProcedureID = %li", id);
		// (j.jones 2009-08-31 15:48) - PLID 17732 - remove links to Preference Cards
		ExecuteParamSql("DELETE FROM PreferenceCardProceduresT WHERE ProcedureID = {INT}", id);
		ExecuteSql("DELETE FROM ProcedureDiscountsT WHERE ProcedureID = %li", id);
		// (d.moore 2007-06-18 16:39) - PLID 14670 - We have to delete from ProcedureLadderTemplateT before deleting from ProcedureT
		ExecuteSql("DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID = %li", id);
		// (c.haag 2009-10-12 12:54) - PLID 35722 - MailSentProcedureT
		ExecuteSql("DELETE FROM MailSentProcedureT WHERE ProcedureID = %li", id);
		ExecuteSql("DELETE FROM ProcedureT WHERE ID = %li", id);
		ExecuteSql("DELETE FROM ResourcePurposeTypeT WHERE AptPurposeID = %i", id);
		ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptPurposeID = %i", id);
		// (b.spivey - February 4th, 2014) - PLID 60563 - If we have default durations, delete the whole set. Not just parts of it.
		ExecuteParamSql(
					"DECLARE @DelTable TABLE "
					"( "
					"	DefaultDurationSetID INT NOT NULL "
					") "
					"	"
					"INSERT INTO @DelTable "
					"SELECT ProviderSchedDefDurationID FROM ProviderSchedDefDurationDetailT WHERE AptPurposeID = {INT} "
					"	"
					"DELETE Del "
					"FROM ProviderSchedDefDurationDetailT Del "
					"INNER JOIN @DelTable DelT ON Del.ProviderSchedDefDurationID = DelT.DefaultDurationSetID "
					"	"
					"DELETE Del "
					"FROM ProviderSchedDefDurationT Del "
					"INNER JOIN @DelTable DelT ON Del.ID = DelT.DefaultDurationSetID ", id);

		trans.Commit();
		m_procedureNameChecker.Refresh(id);

		// (v.maida 08/12/2014) - PLID 55585 - Audit when a user clicks the "Delete Procedural Data" button.
		long nAuditID = BeginNewAuditEvent();
		CString strOldProcName = FormatString("Procedure item: %s", VarString(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 1)));
		CString strNewProcName = FormatString("Removed all procedural data for the '%s' procedure item.", VarString(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 1)));
		AuditEvent(-1, "", nAuditID, aeiProceduralDataDeleted, id, strOldProcName, strNewProcName, aepMedium, aetDeleted);
	}
	NxCatchAll("Could not remove procedural data");

	m_procedureCombo->Requery();
	m_procedureCombo->CurSel = 0;
	// (j.jones 2006-05-17 15:58) - the tablechecker will cause UpdateView() to fire
	//UpdateView();
	EnableAppropriateFields();
}

void CProcedureDlg::OnDefault() 
{
	try {
		CAddProcedureDlg dlg(this);
		long nProcedures = GetRemotePropertyInt("DefaultProcedureType", 1);
		if(nProcedures) {
			dlg.m_bProcedures = true;
			GetRemotePropertyArray("DefaultProcedureList", dlg.m_arProcIDs);
		}
		else {
			dlg.m_bProcedures = false;
			GetRemotePropertyArray("DefaultProcedureList", dlg.m_arProcGroupIDs);
		}
		if (IDOK == dlg.DoModal()) {
			if(dlg.m_bProcedures) {
				SetRemotePropertyInt("DefaultProcedureType", 1);
				SetRemotePropertyArray("DefaultProcedureList", dlg.m_arProcIDs);
			}
			else {
				SetRemotePropertyInt("DefaultProcedureType", 0);
				SetRemotePropertyArray("DefaultProcedureList", dlg.m_arProcGroupIDs);
			}
		}
	} NxCatchAll("Error In: CProcedureDlg::OnDefault");
}

void CProcedureDlg::OnEditProcGroup() 
{
	try {
		CProcedureGroupsDlg dlg(this);
		if(m_pProcGroupCombo->CurSel != -1) {
			dlg.m_nProcGroupID = VarLong(m_pProcGroupCombo->GetValue(m_pProcGroupCombo->CurSel,0), -1);
		}
		dlg.DoModal();

		m_pProcGroupCombo->Requery();

		Refresh();
	} NxCatchAll("Error In: CProcedureDlg::OnEditProcGroup");
}


void CProcedureDlg::OnSelChosenProcGroup(long nRow) 
{
	try {
		if (nRow == -1)
		{
			m_pProcGroupCombo->CurSel = 0;
			return;
		}

		long nNewID = VarLong(m_pProcGroupCombo->GetValue(nRow,0), -1);
		if(nNewID == -1) {
			m_pProcGroupCombo->CurSel = -1;
		}
		long nProcedureID;
		if(GetProcedureID(nProcedureID) ) {
			ExecuteSql("UPDATE ProcedureT SET ProcedureGroupID = %li WHERE ID = %li", nNewID, nProcedureID);
		}
	} NxCatchAll("Error In: CProcedureDlg::OnSelChosenProcGroup");
}

void CProcedureDlg::OnRequeryFinishedProcGroup(short nFlags) 
{
	try {
		IRowSettingsPtr	pRow = m_pProcGroupCombo->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<No Group Selected>");
		m_pProcGroupCombo->InsertRow(pRow, 0);
	}NxCatchAll("Error in CProcedureDlg::OnRequeryFinishedProcGroup()");
}

BOOL CProcedureDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		ChangeCustomLabel(::GetDlgCtrlID(pMsg->hwnd));
		return TRUE;
		break;
	default:
		break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

BOOL CProcedureDlg::ChangeCustomLabel (const int nID) {

	_variant_t var;
	int nResult;
	int field = GetLabelFieldID(nID);

	if(field == 0)	//didn't click on a changable label
		return false;
	
	if (!UserPermission(CustomLabel))
		return false;

	CString strResult, strPrompt;
	GetDlgItemText(nID, strPrompt);
	strResult = ConvertFromControlText(strPrompt);

	do {
		nResult = InputBoxLimited(this, "Enter new name for " + strPrompt, strResult, "",50,false,false,NULL);

		if (nResult == IDCANCEL)
			return false;

		strResult.TrimRight();
		strResult.TrimLeft();

		if (strResult == "")
			AfxMessageBox("Please type in a new name for this custom label or press cancel");

		else if(strResult.GetLength() > 50)
			AfxMessageBox("The label name you entered is too long.");
	
	} while ((strResult.GetLength() > 50) | (strResult == ""));

	try {

		if(!IsRecordsetEmpty("SELECT * FROM CustomFieldsT WHERE ID = %d", field))
			// Make the data change
			ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %li", _Q(strResult), field);
		else
			// The record wasn't found so insert it into the database
			// m.carlson:Feb. 2, 2004	PLID #10737
			ExecuteSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES (%li,'%s',1)",field,_Q(strResult));

		// (b.cardillo 2006-05-19 17:28) - PLID 20735 - We know the new name for this label, so 
		// update the global cache so it won't have to be reloaded in its entirety.
		SetCustomFieldNameCachedValue(field, strResult);
		// (b.cardillo 2006-07-06 16:27) - PLID 20737 - Notify all the other users as well.
		CClient::RefreshTable_CustomFieldName(field, strResult);

		SetDlgItemText(nID, ConvertToControlText(strResult));
		Invalidate();

	}NxCatchAll("Error in changing custom field. CProcedureDlg::ChangeCustomLabel");

	//success!
	return true;
}

int CProcedureDlg::GetLabelFieldID(int nID)
{
	int field = 0;

	switch(nID)
	{
	case IDC_CUSTOM1_ADMIN:
		field = 63;
		break;
	case IDC_CUSTOM2_ADMIN:
		field = 64;
		break;
	case IDC_CUSTOM3_ADMIN:
		field = 65;
		break;
	case IDC_CUSTOM4_ADMIN:
		field = 66;
		break;
	case IDC_CUSTOM5_ADMIN:
		field = 67;
		break;
	case IDC_CUSTOM6_ADMIN:
		field = 68;
		break;
	default:
		field = 0;
		break;
	}

	return field;

}

void CProcedureDlg::OnSelChosenNurses(long nRow) 
{
	try {
		CString strNurseID;
		if(nRow == -1) 
			strNurseID = "NULL";
		else 
			strNurseID.Format("%li", VarLong(m_pNurse->GetValue(nRow, 0)));

		if(strNurseID == "-1")
			strNurseID = "NULL";

		long nProcedureID;
		if (GetProcedureID(nProcedureID))
			ExecuteSql("UPDATE ProcedureT SET NurseID = %s WHERE ID = %li", strNurseID, nProcedureID);
	}NxCatchAll("Error in CProcedureDlg::OnSelChosenNurses()");
}

void CProcedureDlg::OnSelChosenAnesth(long nRow) 
{
	try {
		CString strAnesthID;
		if(nRow == -1)
			strAnesthID = "NULL";
		else
			strAnesthID.Format("%li", VarLong(m_pAnesth->GetValue(nRow, 0)));

		if(strAnesthID == "-1")
			strAnesthID = "NULL";

		long nProcedureID;
		if (GetProcedureID(nProcedureID))
			ExecuteSql("UPDATE ProcedureT SET AnesthesiologistID = %s WHERE ID = %li", strAnesthID, nProcedureID);
	}NxCatchAll("Error in CProcedureDlg::OnSelChosenAnesth()");
	
}

void CProcedureDlg::OnEditSectionsBtn() 
{
	try {
		
		// (d.moore 2007-06-22 09:10) - PLID 23861 - Dialog is no longer modal.
		// (d.moore 2007-09-26) - PLID 23861 - The code for opening the dialog
		//  has been moved to the Main Form in order to better handle it now that
		//  the dialog is modeless.
		//  I've changed the dialog so that it does not require a procedure to be
		//  selected in order to open.
		long nProcedureID;
		GetProcedureID(nProcedureID);
		GetMainFrame()->ShowNexFormEditor(nProcedureID);
	} NxCatchAll("CProcedureDlg::OnEditSectionsBtn");
}

void CProcedureDlg::OnAnesthesiaSetup() 
{
	try {
		_variant_t value;
		long curSel = m_pAnesthesia->CurSel;
		if (curSel != -1)
			value = m_pAnesthesia->Value[curSel][0];

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 9, m_pAnesthesia, "Edit Combo Box").DoModal();

		IRowSettingsPtr pRow = m_pAnesthesia->GetRow(-1);
		pRow->PutValue(0, _bstr_t("<No Default Anesthesia>"));
		m_pAnesthesia->InsertRow(pRow, 0);

		if (curSel != -1)
			m_pAnesthesia->SetSelByColumn(0, value);

		//incase we removed the default value, properly save a blank value
		OnSelChosenDefAnesthesia(m_pAnesthesia->CurSel);
	} NxCatchAll("Error In: CProcedureDlg::OnAnesthesiaSetup");
}

void CProcedureDlg::OnSelChosenDefAnesthesia(long nRow) 
{
	try {

		long nID;
		if (!GetProcedureID(nID))
			return;

		CString strAnesth = "";

		if(nRow != -1) {
			strAnesth = VarString(m_pAnesthesia->GetValue(nRow, 0));
			if(strAnesth == "<No Default Anesthesia>")
				strAnesth = "";			
		}

		ExecuteSql("UPDATE ProcedureT SET Anesthesia = '%s' WHERE ID = %li", _Q(strAnesth), nID);

	}NxCatchAll("Error in CProcedureDlg::OnSelChosenDefAnesthesia()");

}

void CProcedureDlg::OnMasterProc() 
{
	try {
		long id;
		if(!GetProcedureID(id))
			return;

		// (d.moore 2007-06-11 16:16) - PLID 14670 - Removed reference to LadderTemplateID in query.
		_RecordsetPtr rs = CreateRecordset("SELECT "
			"ProcedureT.Name, ProcedureGroupID, MasterProcedureID "
			"FROM ProcedureT LEFT JOIN AptPurposeT ON ProcedureT.ID = AptPurposeT.ID "
			"WHERE ProcedureT.ID = %i", id);
		FieldsPtr f = rs->Fields;

		if(rs->eof) {
			HandleException(NULL,"The selected procedure could not be found.");
			return;
		}

		if(f->GetItem("MasterProcedureID")->Value.vt != VT_NULL) {
			//This isn't already a master, let's double check?
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to make the Procedure %s a master-level procedure?\n"
				"Any ladders which are currently tracking %s as a detail procedure will continue to track it as a master procedure.", 
				AdoFldString(f, "Name"), AdoFldString(f, "Name")))  {
				CheckRadioButton(IDC_MASTER_PROC, IDC_DETAIL_PROC, IDC_DETAIL_PROC);
				OnDetailProc();
				return;
			}
			else {
				//OK, they asked for it.
				// (c.haag 2011-03-09) - PLID 42725 - Made this into a batch operation
				CSqlBatch batch;
				// First, set the procedure to be a master procedure (this is legacy code)
				batch.Add("UPDATE ProcedureT SET MasterProcedureID = NULL WHERE ID = %li", id);
				// (c.haag 2011-03-09) - PLID 42725 - Now go through all the ProcInfoDetailsT records that are
				// assigned to this procedure and are NOT chosen, and delete them. If we don't do this, then they
				// will be treated like regular procedures assigned to the PIC! In effect, they would be retroactively
				// added to all patient PIC's that had it as an unchosen procedure detail!
				batch.Add("DELETE FROM ProcInfoDetailsT WHERE ProcedureID = %d AND Chosen = 0", id);
				batch.Execute();
			}
		}

		// (d.moore 2007-10-12) - PLID 23497 - The elipses button for Advanced Ladder Assignment
		//  should be enabled when the procedure is a Master.
		GetDlgItem(IDC_LADDER_CAPTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVANCED_LADDER_ASSIGNMENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_GROUP_CAPTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROC_GROUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PROC_GROUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_MASTER_LIST_CAPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_MASTER_PROCS)->EnableWindow(FALSE);

		/*
		long phaseID = AdoFldLong(f, "LadderTemplateID", -1);
		if (phaseID != -1)
			m_ladderCombo->SetSelByColumn(0, phaseID);
		else m_ladderCombo->CurSel = NULL;
		*/

		long nGroupID = AdoFldLong(f, "ProcedureGroupID", -1);
		if(nGroupID != -1)
			m_pProcGroupCombo->SetSelByColumn(0, nGroupID);
		else
			m_pProcGroupCombo->CurSel = -1;

	}NxCatchAll("Error in CProcedureDlg::OnMasterProc()");
}

struct DoomedProcInfo {
	long nLadderID;
	CString strPatName;
	long nPatID;
	CString strProcInfoName;
};

void CProcedureDlg::OnDetailProc() 
{
	try {
		long id;
		if(!GetProcedureID(id))
			return;

		_RecordsetPtr rs = CreateRecordset("SELECT "
			"ProcedureT.Name, MasterProcedureID "
			"FROM ProcedureT INNER JOIN AptPurposeT ON ProcedureT.ID = AptPurposeT.ID "
			"WHERE ProcedureT.ID = %i", id);
		FieldsPtr f = rs->Fields;
		long m_nMasterID;
		if(f->GetItem("MasterProcedureID")->Value.vt == VT_NULL) {
			//This isn't already a detail, let's double check?
			if(ReturnsRecords("SELECT ID FROM ProcedureT WHERE MasterProcedureID = %li", id)) {
				MsgBox("There are Detail procedures associated with this procedure.  Please reassign them before changing the type of this procedure.");
				CheckRadioButton(IDC_MASTER_PROC, IDC_DETAIL_PROC, IDC_MASTER_PROC);
				OnMasterProc();
				return;
			}
			CSelectMasterDlg dlg(this);
			dlg.m_strName = AdoFldString(f, "Name");
			dlg.m_bIsTracked = ReturnsRecords("SELECT ID FROM ProcInfoDetailsT WHERE ProcedureID = %li", id);
			dlg.m_nDetailID = id;
			if(IDOK != dlg.DoModal())  {
				CheckRadioButton(IDC_MASTER_PROC, IDC_DETAIL_PROC, IDC_MASTER_PROC);
				OnMasterProc();
				return;
			}
			else {
				
				CWaitCursor pWait;
				
				//OK, they asked for it.
				// (d.moore 2007-06-11 15:55) - PLID 14670 - Remove any ladders associated with the procedure.
				ExecuteParamSql("UPDATE ProcedureT SET ProcedureGroupID = -1, MasterProcedureID = {INT} WHERE ID = {INT}", dlg.m_nMasterID, id);
				ExecuteParamSql("DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID = {INT}", id);
				m_rgLadderID.RemoveAll();

				// (j.jones 2010-06-23 10:03) - PLID 34158 - do not batch this code together, there can be overlaps that
				// would cause Primary Key violations
				
				// (j.jones 2010-06-23 09:04) - PLID 34158 - now add our new master procedure to any PICs that have our now-detail procedure,
				// that don't already have the new master
				_RecordsetPtr rsProcInfosT = CreateParamRecordset("SELECT ID FROM ProcInfoT WHERE ID IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = {INT}) "
					"AND ID NOT IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = {INT})", id, dlg.m_nMasterID);
				while(!rsProcInfosT->eof) {
					long nProcInfoID = AdoFldLong(rsProcInfosT, "ID");
					// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
					ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) "
						"VALUES ({INT}, {INT})", nProcInfoID, dlg.m_nMasterID);					
					rsProcInfosT->MoveNext();
				}

				//for all PICs that had this procedure, it was a master, now it's a detail, so now
				//it has to be flagged as Chosen, because masters were always this way
				ExecuteParamSql("UPDATE ProcInfoDetailsT SET Chosen = 1 WHERE ProcedureID = {INT}", id);

				//first, add our now-detail procedure to any PICs that have our new master procedure,
				//that don't already have our now-detail procedure
				// (d.thompson 2011-01-10) - PLID 41956 - This needs to happen AFTER we set Chosen above.  Otherwise all 'new procedures' will get selected on all
				//	old ladders, even when the old ladder knows nothing about them.
				rsProcInfosT = CreateParamRecordset("SELECT ID FROM ProcInfoT WHERE ID IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = {INT}) "
					"AND ID NOT IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = {INT})", dlg.m_nMasterID, id);
				while(!rsProcInfosT->eof) {
					// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
					ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) "
						"VALUES ({INT}, {INT})", AdoFldLong(rsProcInfosT, "ID"), id);
					rsProcInfosT->MoveNext();
				}

				// (j.jones 2010-06-23 09:04) - PLID 34158 - last, add the new master's other children to existing ladders
				_RecordsetPtr rsOtherChildren = CreateParamRecordset("SELECT ID FROM ProcedureT WHERE MasterProcedureID = {INT} AND ID <> {INT}",
					dlg.m_nMasterID, id);
				while(!rsOtherChildren->eof) {
					long nOtherProcID = AdoFldLong(rsOtherChildren, "ID");
					rsProcInfosT = CreateParamRecordset("SELECT ID FROM ProcInfoT WHERE ID IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = {INT}) "
						"AND ID NOT IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = {INT})", id, nOtherProcID);
					while(!rsProcInfosT->eof) {
						// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
						ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) "
							"VALUES ({INT}, {INT})", AdoFldLong(rsProcInfosT, "ID"), nOtherProcID);
						rsProcInfosT->MoveNext();
					}
					rsOtherChildren->MoveNext();
				}
				rsOtherChildren->Close();

				m_nMasterID = dlg.m_nMasterID;
			}
		}
		else {
			m_nMasterID = AdoFldLong(f, "MasterProcedureID");
		}

		//(e.lally 2009-08-10) PLID 35158 - Check tracking license
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)){
			GetDlgItem(IDC_LADDER_CAPTION)->EnableWindow(FALSE);
			// (d.moore 2007-10-12) - PLID 23497 - The elipses button for Advanced Ladder 
			//  Assignment should be disabled when the procedure is a Detail.
			GetDlgItem(IDC_ADVANCED_LADDER_ASSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(FALSE);
			m_ladderCombo->CurSel = NULL;
			// (d.moore 2007-06-11 16:10) - PLID 14670 - Disable the multi-select label as well as the NxDataList
			m_MultiLadder.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
			ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
			InvalidateDlgItem(IDC_MULTI_LADDER);

			GetDlgItem(IDC_GROUP_CAPTION)->EnableWindow(FALSE);
			GetDlgItem(IDC_PROC_GROUP)->EnableWindow(FALSE);
			m_pProcGroupCombo->CurSel = -1;
			GetDlgItem(IDC_EDIT_PROC_GROUP)->EnableWindow(FALSE);
		}
		GetDlgItem(IDC_MASTER_LIST_CAPTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_MASTER_PROCS)->EnableWindow(TRUE);

		m_pMasterProcList->SetSelByColumn(0, m_nMasterID);
	}NxCatchAll("Error in CProcedureDlg::OnDetailProc()");

		
}

void CProcedureDlg::OnSelChosenMasterProcs(long nRow) 
{
	try {
		if(nRow == -1) {
			MsgBox("You must select a valid row.");
			OnDetailProc();
			return;
		}

		long id;
		GetProcedureID(id);

		_RecordsetPtr rsMasterID = CreateRecordset("SELECT MasterProcedureID FROM ProcedureT WHERE ID = %li", id);
		long nOldMaster = AdoFldLong(rsMasterID, "MasterProcedureID");
		long nNewMaster = VarLong(m_pMasterProcList->GetValue(nRow, 0));
		rsMasterID->Close();

		if(nOldMaster == nNewMaster)
			return;

		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to change the master procedure for this item?\n"
			"This item will be removed from any ladders where it is currently being tracked, and added to any ladders "
			"being tracked with its new master.  This cannot be undone.")) {
			OnDetailProc();
			return;
		}
		
		ExecuteSql("UPDATE ProcedureT SET MasterProcedureID = %li WHERE ID = %li", nNewMaster, id);
		ExecuteSql("DELETE FROM ProcInfoDetailsT WHERE ProcedureID = %li "
			"AND NOT EXISTS (SELECT ID FROM ProcInfoDetailsT OtherDetails "
			"WHERE OtherDetails.ProcInfoID = ProcInfoDetailsT.ProcInfoID AND ProcedureID = %li)",id, nNewMaster);
		_RecordsetPtr rsNewLadders = CreateRecordset("SELECT ID FROM ProcInfoT WHERE ID IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li) "
					"AND ID NOT IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)", nNewMaster, id);
		while(!rsNewLadders->eof) {
			// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
			ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) "
				"VALUES ({INT}, {INT})", AdoFldLong(rsNewLadders, "ID"), id);
			rsNewLadders->MoveNext();
		}

	}NxCatchAll("Error iin CProcedureDlg::OnSelChosenMasterProcs()");
}

HBRUSH CProcedureDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	try {
		//to avoid drawing problems with transparent text and disabled ites
		//override the NxDialog way of doing text with a non-grey background
		//NxDialog relies on the NxColor to draw the background, then draws text transparently
		//instead, we actually color the background of the STATIC text
		if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() != IDC_OFFICIAL_TERM && pWnd->GetDlgCtrlID() != IDC_CUSTOM1 
			&& pWnd->GetDlgCtrlID() != IDC_NAME && pWnd->GetDlgCtrlID() != IDC_ARRIVAL_TIME) {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(m_color));
			return m_brush;
		} else {
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		}
	} NxCatchAll("Error In: CProcedureDlg::OnCtlColor");
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CProcedureDlg::OnLeftProcedure() 
{
	try {
		if(m_procedureCombo->CurSel > 0) {
			m_procedureCombo->CurSel = m_procedureCombo->CurSel-1;
			OnSelChosenProcedure(m_procedureCombo->CurSel);
			if(m_procedureCombo->CurSel < 1) {
				m_btnLeftProcedure.EnableWindow(FALSE);
			}
			if(m_procedureCombo->CurSel == m_procedureCombo->GetRowCount()-1) {
				m_btnRightProcedure.EnableWindow(FALSE);
			}
			else {
				m_btnRightProcedure.EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CProcedureDlg::OnLeftProcedure()");
}

void CProcedureDlg::OnRightProcedure() 
{
	try {
		if(m_procedureCombo->CurSel < m_procedureCombo->GetRowCount()-1 && m_procedureCombo->CurSel != -1) {
			m_procedureCombo->CurSel = m_procedureCombo->CurSel+1;
			OnSelChosenProcedure(m_procedureCombo->CurSel);
			if(m_procedureCombo->CurSel == m_procedureCombo->GetRowCount()-1) {
				m_btnRightProcedure.EnableWindow(FALSE);
			}
			if(m_procedureCombo->CurSel < 1) {
				m_btnLeftProcedure.EnableWindow(FALSE);
			}
			else {
				m_btnLeftProcedure.EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CProcedureDlg::OnRightProcedure()");
}

void CProcedureDlg::OnProcEmrAction() 
{
	try {
		if(g_pLicense) {
			if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
				if(m_procedureCombo->CurSel == -1) return;
				// (c.haag 2006-04-04 11:15) - PLID 19890 - Make sure the user has proper permissions
				if(!CheckCurrentUserPermissions(bioAdminEMR, sptRead)) return;
				CEmrActionDlg dlg(this);
				dlg.m_SourceType = eaoProcedure;
				dlg.m_nSourceID = VarLong(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 0));
				dlg.DoModal();
			}
			else {
				if(m_procedureCombo->CurSel == -1) return;
				CCustomRecordActionDlg dlg(this);
				dlg.m_SourceType = eaoProcedure;
				dlg.m_nSourceID = VarLong(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 0));
				dlg.DoModal();
			}
		}
	}NxCatchAll("Error in CProcedureDlg::OnProcEmrAction()");
}

void CProcedureDlg::EnableAppropriateFields()
{
	if(m_procedureCombo->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAKE_NON_PROCEDURAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_MASTER_PROC)->EnableWindow(FALSE);
		GetDlgItem(IDC_DETAIL_PROC)->EnableWindow(FALSE);
		GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROC_GROUP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PROC_GROUP)->EnableWindow(FALSE);
		GetDlgItem(IDC_MASTER_PROCS)->EnableWindow(FALSE);
		GetDlgItem(IDC_CPT_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROCEDURE_PRODUCT_COMBO)->EnableWindow(FALSE);
		GetDlgItem(IDC_CPT_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROC_EMR_ACTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_OFFICIAL_TERM)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM1)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM2)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM4)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM5)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM3)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM6)->EnableWindow(FALSE);
		GetDlgItem(IDC_NURSES)->EnableWindow(FALSE);
		GetDlgItem(IDC_ANESTH)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEF_ANESTHESIA)->EnableWindow(FALSE);
		GetDlgItem(IDC_ANESTHESIA_SETUP)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_TIME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SECTIONS_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEFAULT)->EnableWindow(FALSE);

		SetDlgItemText(IDC_NAME, "");
		SetDlgItemText(IDC_OFFICIAL_TERM, "");
		SetDlgItemText(IDC_CUSTOM1, "");
		SetDlgItemText(IDC_CUSTOM2, "");
		SetDlgItemText(IDC_CUSTOM3, "");
		SetDlgItemText(IDC_CUSTOM4, "");
		SetDlgItemText(IDC_CUSTOM5, "");
		SetDlgItemText(IDC_CUSTOM6, "");
		m_pNurse->PutCurSel(-1);
		m_pAnesth->PutCurSel(-1);
		m_pAnesthesia->PutCurSel(-1);
		m_cptList->Clear();
		m_ladderCombo->CurSel = NULL;
		m_pProcGroupCombo->PutCurSel(-1);		
		m_cptCombo->PutCurSel(-1);
		m_productCombo->PutCurSel(-1);
		SetDlgItemText(IDC_ARRIVAL_TIME, "");

	}
	else{
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAKE_NON_PROCEDURAL)->EnableWindow(TRUE);
		GetDlgItem(IDC_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_MASTER_PROC)->EnableWindow(TRUE);
		GetDlgItem(IDC_DETAIL_PROC)->EnableWindow(TRUE);
		GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROC_GROUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PROC_GROUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_CPT_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROCEDURE_PRODUCT_COMBO)->EnableWindow(TRUE);
		GetDlgItem(IDC_CPT_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROC_EMR_ACTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_OFFICIAL_TERM)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM1)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM2)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM4)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM5)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM3)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM6)->EnableWindow(TRUE);
		GetDlgItem(IDC_NURSES)->EnableWindow(TRUE);
		GetDlgItem(IDC_ANESTH)->EnableWindow(TRUE);
		GetDlgItem(IDC_DEF_ANESTHESIA)->EnableWindow(TRUE);
		GetDlgItem(IDC_ANESTHESIA_SETUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_ARRIVAL_TIME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SECTIONS_BTN)->EnableWindow(TRUE);
		GetDlgItem(IDC_DEFAULT)->EnableWindow(TRUE);
	}

	if(m_procedureCombo->GetRowCount() > 0){
		m_procedureCombo->PutEnabled(TRUE);
	}
	else{
		m_procedureCombo->PutEnabled(FALSE);
	}
}

bool CProcedureDlg::CanDeleteProcedure(long nProcedureID)
{
	_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ProcedureID FROM EMRProcedureT WHERE ProcedureID = %li", nProcedureID);
	if (!rs->eof)
	{	AfxMessageBox("You may not delete a procedure that has an EMR associated with it.");
		return false;
	}

	// (c.haag 2009-02-16 12:16) - PLID 33100 - Don't let a user delete a procedure in use by marketing costs
	rs = CreateRecordset("SELECT TOP 1 ProcedureID FROM MarketingCostProcedureT WHERE ProcedureID = %li", nProcedureID);
	if (!rs->eof) {
		MsgBox("You may not delete a procedure that is associated with at least one marketing cost.\n");
		return false;
	}

	if (ReturnsRecords("SELECT * FROM EMRTemplateProceduresT WHERE ProcedureID = %li", nProcedureID))
	{	
		if(IDNO == MessageBox("This procedure is referenced by an EMN template. Are you sure you wish to delete this procedure?","Practice",MB_YESNO|MB_ICONEXCLAMATION))
			return false;
	}

	rs = CreateRecordset("SELECT TOP 1 ID FROM ProcedureT WHERE MasterProcedureID = %li", nProcedureID);
	if (!rs->eof) 
	{
		MsgBox("You may not delete a Master procedure to which Detail procedures have been assigned.\n"
			"Please re-assign these details before deleting this procedure.");
		return false;
	}

	if(IsDlgButtonChecked(IDC_DETAIL_PROC)) {
		//Only check ones where we've actually been chosen.
		rs = CreateRecordset("SELECT Count(DISTINCT PatientID) AS PatCount FROM ProcInfoT INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID WHERE PRocInfoDetailsT.ProcedureID = %li AND ProcInfoDetailsT.Chosen = 1", nProcedureID);
	}
	else {
		//Check all
		rs = CreateRecordset("SELECT Count(DISTINCT PatientID) AS PatCount FROM ProcInfoT INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID WHERE PRocInfoDetailsT.ProcedureID = %li", nProcedureID);
	}

	int nCount = AdoFldLong(rs, "PatCount", 0);
	if(nCount > 0){
		CString strMessage;
		strMessage.Format("There are %li patients being tracked for this procedure. It cannot be deleted.", nCount);
		AfxMessageBox(strMessage, MB_OK);
		return false;
	}

	//Now see if this is one of the default procedures for new patients.
	CArray<int,int> arIds;
	bool bProcFound = false;
	if(GetRemotePropertyInt("DefaultProcedureType", 1)) {
		GetRemotePropertyArray("DefaultProcedureList", arIds);
		for(int i = 0; i < arIds.GetSize(); i++) {
			if(arIds.GetAt(i) == nProcedureID) {
				bProcFound = true;
			}
		}
		if(bProcFound) {
			if(IDYES != MsgBox(MB_YESNO, "This procedure is one of the defaults for new patients.  Are you sure you wish to delete it?")) {
					return false;
			}
			else {
				//Take this procedure out of the list of defaults.
				CArray<int,int> arNewIds;
				for(i=0; i < arIds.GetSize(); i++) {
					if(arIds.GetAt(i) != nProcedureID) {
						arNewIds.Add(arIds.GetAt(i));
					}
				}
				SetRemotePropertyArray("DefaultProcedureList", arNewIds);
			}
		}
	}
	else {
		GetRemotePropertyArray("DefaultProcedureList", arIds);
		for(int i = 0; i < arIds.GetSize(); i++) {
			if(ReturnsRecords("SELECT ID FROM ProcedureT WHERE ID = %li AND ProcedureGroupID = %i", nProcedureID, arIds.GetAt(i))) {
				bProcFound = true;
			}
		}
		if(bProcFound) {
			if(IDYES != MsgBox(MB_YESNO, "This procedure is one of the defaults for new patients.  Are you sure you wish to delete it?")) {
					return false;
			}
			//If they're deleting it, we don't need to do anything special.  The group will still exist,
			//just without this proc in it.
		}
	}

	// (j.jones 2006-09-20 11:21) - PLID 22373 - if they have custom records, warn if this procedure is in their setup
	// otheriwse we would delete silently
	if(g_pLicense->HasEMR(CLicense::cflrSilent) == 1 &&
		ReturnsRecords("SELECT * FROM ProcedureToEMRInfoT WHERE ProcedureID = %li", nProcedureID))
	{	
		if(IDNO == MessageBox("This procedure is referenced by the Custom Records setup. Are you sure you wish to delete this procedure?","Practice",MB_YESNO|MB_ICONEXCLAMATION))
			return false;
	}

	// (c.haag 2006-11-15 09:31) - PLID 22825 - Warn the user if the procedure is associated with
	// any EMR actions. This applies to both the Delete and Delete Procedural Data buttons.
	if(ReturnsRecords("SELECT * FROM EMRActionsT WHERE DestType = 5 AND DestID = %li", nProcedureID)) {
		if(IDNO == MessageBox("This procedure is referenced by at least one EMR action. Are you sure you wish to delete this procedure?","Practice",MB_YESNO|MB_ICONEXCLAMATION))
			return false;
	}
	
	return true;
}

void CProcedureDlg::OnBtnSetupDiscounts() 
{
	try {

		if(m_procedureCombo->CurSel == -1)
			return;

		if(m_cptList->GetRowCount() == 0) {
			if(IDNO == MessageBox("For the Recurring Procedure Discounts feature to function properly,\n"
				"you will need to have at least one Service Code associated with this procedure.\n"
				"Until this is done, the discounts you are about to configure will have no effect.\n\n"
				"Are you sure you wish to continue?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
				return;
			}
		}

		CProcedureDiscountSetupDlg dlg(this);
		dlg.m_ProcedureID = VarLong(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 0));
		dlg.m_strProcedureName = VarString(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 1));
		dlg.DoModal();

	}NxCatchAll("Error configuring procedure discounts.");
}


void CProcedureDlg::OnCheckProcRecurs() {

	try {

		long nProcID; 
		
		if (!GetProcedureID(nProcID))
		return;

		long nRecurs;
		nRecurs = IsDlgButtonChecked(IDC_CHECK_RECUR);

		ExecuteSql("UPDATE ProcedureT SET Recur = %li WHERE ID = %li", nRecurs, nProcID);
	}NxCatchAll("Error updating Recur Field");

}

LRESULT CProcedureDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::AptPurposeT:
			case NetUtils::CPTCodeT:
			case NetUtils::Ladders:
			case NetUtils::ContactsT: {
				try {
					UpdateView();
				} NxCatchAll("Error in CProcedureDlg::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CProcedureDlg::OnTableChanged");

	return 0;
}

void CProcedureDlg::OnRadioProcedureCptCodes() 
{
	OnRadioCodeTypeChanged();
}

void CProcedureDlg::OnRadioProcedureProducts() 
{
	OnRadioCodeTypeChanged();
}

void CProcedureDlg::OnRadioCodeTypeChanged()
{
	try {
		if(IsDlgButtonChecked(IDC_RADIO_PROCEDURE_CPT_CODES)) {
			GetDlgItem(IDC_CPT_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_PROCEDURE_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
		}
		else if(IsDlgButtonChecked(IDC_RADIO_PROCEDURE_PRODUCTS)) {
			GetDlgItem(IDC_PROCEDURE_PRODUCT_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_CPT_COMBO)->ShowWindow(SW_HIDE);
		}
	} NxCatchAll("Error In: CProcedureDlg::OnRadioCodeTypeChanged");
}

void CProcedureDlg::OnSelChosenCptCombo(long nRow) 
{
	if(m_cptCombo->GetCurSel() == -1)
		return;

	try
	{	long id, serviceID;

		if (!GetProcedureID(id))
			return;
		
		serviceID = VarLong(m_cptCombo->Value[nRow][0]);

		//Let's find out if this CPT code is already in use.
		_RecordsetPtr rs = CreateRecordset("SELECT ProcedureT.Name FROM ProcedureT INNER JOIN ServiceT ON ProcedureT.ID = ServiceT.ProcedureID WHERE ServiceT.ID = %li AND ProcedureID <> %li", serviceID, id);
		if(!rs->eof) {
			CString strMessage;
			strMessage.Format("This Service Code is already in use by procedure \"%s\".\n\nWould you like to continue?  If you say yes, this Service Code will be removed from the existing procedure.", AdoFldString(rs, "Name"));
			if(MessageBox(strMessage, "Warning", MB_YESNO) == IDNO) {
				return;
			}
		}

		ExecuteSql("UPDATE ServiceT SET ProcedureID = %li WHERE ID = %li", id, serviceID);
		Refresh();
	}
	NxCatchAll("Could not add Service Code to the procedure");
}

void CProcedureDlg::OnSelChosenProcedureProductCombo(long nRow) 
{
	if(m_productCombo->GetCurSel() == -1)
		return;

	try
	{	long id, serviceID;

		if (!GetProcedureID(id))
			return;
		
		serviceID = VarLong(m_productCombo->Value[nRow][0]);

		//Let's find out if this Product is already in use.
		_RecordsetPtr rs = CreateRecordset("SELECT ProcedureT.Name FROM ProcedureT INNER JOIN ServiceT ON ProcedureT.ID = ServiceT.ProcedureID WHERE ServiceT.ID = %li AND ProcedureID <> %li", serviceID, id);
		if(!rs->eof) {
			CString strMessage;
			strMessage.Format("This Inventory Item is already in use by procedure \"%s\".\n\nWould you like to continue?  If you say yes, this Inventory Item will be removed from the existing procedure.", AdoFldString(rs, "Name"));
			if(MessageBox(strMessage, "Warning", MB_YESNO) == IDNO) {
				return;
			}
		}

		ExecuteSql("UPDATE ServiceT SET ProcedureID = %li WHERE ID = %li", id, serviceID);
		Refresh();
	}
	NxCatchAll("Could not add Inventory Item to the procedure");
}

void CProcedureDlg::OnTrySetSelFinishedNurses(long nRowEnum, long nFlags) 
{
	try {
	
		if(nFlags == dlTrySetSelFinishedFailure) {

			long id;

			if (!GetProcedureID(id))
				return;

			_RecordsetPtr rs = CreateRecordset("SELECT NurseID FROM ProcedureT "
				"LEFT JOIN AptPurposeT ON ProcedureT.ID = AptPurposeT.ID "
				"WHERE ProcedureT.ID = %i", id);

			if(rs->eof)
				return;

			FieldsPtr f = rs->Fields;

			long nID;
			nID = VarLong(f->Item["NurseID"]->Value, -1);

			m_pNurse->PutCurSel(-1);
			//they may have an inactive nurse
			_RecordsetPtr rsNurse = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nID);
			if(!rsNurse->eof) {
				m_pNurse->PutComboBoxText(_bstr_t(AdoFldString(rsNurse, "Name", "")));
			}
		}
	}NxCatchAll("Error in CProcedureDlg::OnTrySetSelFinishedNurses");
}

void CProcedureDlg::OnTrySetSelFinishedAnesth(long nRowEnum, long nFlags) 
{
	try {
	
		if(nFlags == dlTrySetSelFinishedFailure) {

			long id;

			if (!GetProcedureID(id))
				return;

			_RecordsetPtr rs = CreateRecordset("SELECT AnesthesiologistID FROM ProcedureT "
				"LEFT JOIN AptPurposeT ON ProcedureT.ID = AptPurposeT.ID "
				"WHERE ProcedureT.ID = %i", id);

			if(rs->eof)
				return;

			FieldsPtr f = rs->Fields;

			long nID;
			nID = VarLong(f->Item["AnesthesiologistID"]->Value, -1);

			m_pAnesth->PutCurSel(-1);
			//they may have an inactive anesthesiologist
			_RecordsetPtr rsAnesth = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nID);
			if(!rsAnesth->eof) {
				m_pAnesth->PutComboBoxText(_bstr_t(AdoFldString(rsAnesth, "Name", "")));
			}
		}
	}NxCatchAll("Error in CProcedureDlg::OnTrySetSelFinishedAnesth");
}

// (a.walling 2007-04-23 17:51) - PLID 25356 - Moving Procedure Suggested Sales to be individual service code/inventory based


void CProcedureDlg::OnRequeryFinishedLadderCombo(short nFlags) 
{
	try {
	// (d.moore 2007-06-11 12:35) - PLID 14670 - Need to add extra options to the list.
	NXDATALIST2Lib::IRowSettingsPtr pRow2;
	NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_ladderCombo->GetFirstRow();
	
	// <No Ladder Selected>
	pRow2 = m_ladderCombo->GetNewRow();
	pRow2->PutValue(0, (long)-1);
	pRow2->PutValue(1, _bstr_t("<No Ladder Selected>"));			
	m_ladderCombo->AddRowBefore(pRow2, pFirstRow);
	
	// <Multiple Ladders>
	pRow2 = m_ladderCombo->GetNewRow();
	pRow2->PutValue(0, (long)-2);
	pRow2->PutValue(1, _bstr_t("<Multiple Ladders>"));			
	m_ladderCombo->AddRowBefore(pRow2, pFirstRow);
	} NxCatchAll("Error In: CProcedureDlg::OnRequeryFinishedLadderCombo");
}

// (d.moore 2007-06-11 13:01) - PLID 14670 - Open a dialog to select multiple 
//  ladders for a procedure.
//(e.lally 2010-07-28) PLID 36199 - Depreciated, always use the advanced configuration now
/*
void CProcedureDlg::OpenLadderMultiList(CArray<long, long> &rgIdList) 
{
	try {		
		CMultiSelectDlg dlg;
		dlg.PreSelect(rgIdList);
		int nResult = dlg.Open("LadderTemplatesT", "", "ID", "Name", "Select Ladders");
		
		if (nResult == IDOK) {
			rgIdList.RemoveAll();
			dlg.FillArrayWithIDs(rgIdList);
			// Display the selected items in a hyperlink.
			if (rgIdList.GetSize() > 1) {
				// Multiple items were selected from the dialog.
				// Get their ID values.
				CString strNameList = dlg.GetMultiSelectString();
				m_MultiLadder.SetText(strNameList);
				m_MultiLadder.SetType(dtsHyperlink);
				ShowDlgItem(IDC_LADDER_COMBO, SW_HIDE);
				ShowDlgItem(IDC_MULTI_LADDER, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LADDER);
			} 
			else if (rgIdList.GetSize() == 1) {
				// Only a single item was selected from the dialog.
				GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
				m_ladderCombo->SetSelByColumn(0, rgIdList[0]);
				m_MultiLadder.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
				ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
				InvalidateDlgItem(IDC_MULTI_LADDER);
			} else {
				// No selection made.
				GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
				m_ladderCombo->SetSelByColumn(0, (long)-1);
				m_MultiLadder.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
				ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
				InvalidateDlgItem(IDC_MULTI_LADDER);
			}
		}
	} NxCatchAll("Error in CProcedureDlg::OpenLadderMultiList");
}
*/

LRESULT CProcedureDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		// (d.moore 2007-06-11 12:35) - PLID 14670 - Need to reopen the multi-select dialog.
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MULTI_LADDER:
			{
				//(e.lally 2010-07-28) PLID 36199 - always use the advanced configuration now
				OnAdvancedLadderAssignment();
				/*
				long nProcID;
				if (!GetProcedureID(nProcID)) {
					return 0;
				}
				OpenLadderMultiList(m_rgLadderID);
				
				CString strQuery, strInsertQ;
				strQuery.Format("DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID=%li;", nProcID);
				for (long i = 0; i < m_rgLadderID.GetSize(); i++) {
					strInsertQ.Format(
						"INSERT INTO ProcedureLadderTemplateT(ProcedureID, LadderTemplateID) "
						"VALUES(%li, %li)", nProcID, m_rgLadderID[i]);
					strQuery += strInsertQ;
				}
				ExecuteSqlStd(strQuery);
				*/
			} break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll("Error In: CProcedureDlg::OnLabelClick");
	return 0;
}


BOOL CProcedureDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		// (d.moore 2007-06-11 14:44) - PLID 14670 - Added for the ladder multi-select label.
		if (GetDlgItem(IDC_MULTI_LADDER)->IsWindowVisible()) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			
			CRect rc;
			GetDlgItem(IDC_MULTI_LADDER)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	} NxCatchAll("Error In: CProcedureDlg::OnSetCursor");
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}


void CProcedureDlg::OnAdvancedLadderAssignment() 
{
	try {
		// (d.moore 2007-08-28) - PLID 23497 - A new dialog has been added to make
		//  it easier to assing ladders to procedures.

		long nProcedureID = -1;
		
		CProcedureLadderAssignment dlg(this);
		int curSel = m_procedureCombo->CurSel;
		if (curSel != -1) {
			nProcedureID = VarLong(m_procedureCombo->Value[curSel][0], -1);
			dlg.SetSelectedProcedure(nProcedureID);
			
		}	
		dlg.DoModal();
		
		// Now update the ladder display area.
		m_rgLadderID.RemoveAll();
		CString strLadderNames;
		_RecordsetPtr rsLadder = CreateRecordset(
			"SELECT ProcedureLadderTemplateT.LadderTemplateID, LadderTemplatesT.NAME "
			"FROM ProcedureLadderTemplateT INNER JOIN LadderTemplatesT "
			"ON ProcedureLadderTemplateT.LadderTemplateID = LadderTemplatesT.ID "
			"WHERE ProcedureLadderTemplateT.ProcedureID=%li "
			"ORDER BY LadderTemplatesT.NAME", nProcedureID);
		long nLadderTemplateID;
		while (!rsLadder->eof) {
			nLadderTemplateID = AdoFldLong(rsLadder, "LadderTemplateID", 0);
			if (nLadderTemplateID > 0) {
				m_rgLadderID.Add(nLadderTemplateID);
				strLadderNames += AdoFldString(rsLadder, "NAME", "") + ", ";
			}
			rsLadder->MoveNext();
		}
		// Select the current value for the ladder template datalist, or set the label text if
		//  there is more than one selection.
		if (m_rgLadderID.GetSize() == 0) {
			// No selection.
			GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
			m_ladderCombo->SetSelByColumn(0, (long)-1);
			m_MultiLadder.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
			ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
			InvalidateDlgItem(IDC_MULTI_LADDER);
		}
		else if (m_rgLadderID.GetSize() == 1) {
			GetDlgItem(IDC_LADDER_COMBO)->EnableWindow(TRUE);
			m_ladderCombo->SetSelByColumn(0, m_rgLadderID[0]);
			m_MultiLadder.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_LADDER_COMBO, SW_SHOW);
			ShowDlgItem(IDC_MULTI_LADDER, SW_HIDE);
			InvalidateDlgItem(IDC_MULTI_LADDER);
		} else {
			strLadderNames = strLadderNames.Left(strLadderNames.GetLength()-2);
			m_MultiLadder.SetText(strLadderNames);
			m_MultiLadder.SetType(dtsHyperlink);
			ShowDlgItem(IDC_LADDER_COMBO, SW_HIDE);
			ShowDlgItem(IDC_MULTI_LADDER, SW_SHOW);
			InvalidateDlgItem(IDC_MULTI_LADDER);
		}
	} NxCatchAll("Error In: CProcedureDlg::OnAdvancedLadderAssignment");
}

// (c.haag 2008-11-25 15:08) - PLID 10776 - Inactivate the selected procedure
void CProcedureDlg::OnInactivate()
{
	try {
		long nProcedureID = -1;
		if (!GetProcedureID(nProcedureID)) {
			return;
		}

		// (c.haag 2009-01-06 11:04) - PLID 10776 - This class will run queries that find 
		// all relationships between this procedure and other data, and populate member 
		// variables with them. We'll use it to see whether the user can actually inactivate
		// the procedure; and if so, what can be affected.
		CInactiveProcedureWarnings ipw(nProcedureID, VarString(m_procedureCombo->Value[m_procedureCombo->CurSel][1], ""));
		if (ipw.HasErrors() || ipw.HasWarnings()) {
			// If we get here, the user needs to be made aware of certain things before
			// inactivating the procedure. Show a window explaining those things
			CInactiveProcedureWarningDlg dlg(ipw, this);
			UINT nResult = dlg.DoModal();
			if (IDCANCEL == nResult // User pressed "Close"
				|| IDNO == nResult // User pressed "No"
				)
			{
				return;
			}
		}

		// Then give a final warning
		if (IDYES != MsgBox(MB_YESNO | MB_ICONQUESTION, "You will no longer be able to schedule, or manually assign this procedure to new clinical data.\r\n\r\nAre you ABSOLUTELY SURE you wish to inactivate this procedure?")) {
			return;
		}

		// First, inactivate the procedure
		ExecuteParamSql("UPDATE ProcedureT SET Inactive = 1, InactivatedDate = GetDate(), InactivatedBy = {STRING} WHERE ID = {INT}", GetCurrentUserName(), nProcedureID);

		// (c.haag 2009-01-29 12:42) - PLID 10776 - Make sure this is not in the default procedure list
		long nProcedures = GetRemotePropertyInt("DefaultProcedureType", 1);
		if(nProcedures) {
			CArray<int,int> anProcedureIDs;
			GetRemotePropertyArray("DefaultProcedureList", anProcedureIDs);
			for (int i=0; i < anProcedureIDs.GetSize(); i++) {
				if (anProcedureIDs[i] == nProcedureID) {
					anProcedureIDs.RemoveAt(i--);
				}
			}
			SetRemotePropertyArray("DefaultProcedureList", anProcedureIDs);
		}

		// Now audit
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			CString strOld = CString(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 1).bstrVal);
			AuditEvent(-1, "", nAuditID, aeiSchedProcInactivated, nProcedureID, strOld, "<Inactivated>", aepHigh, aetChanged);
		}

		// Now do post-inactivation cleanup. I copied this code from OnDelete for consistency.
		m_procedureNameChecker.Refresh(nProcedureID);

		// (m.hancock 2006-08-03 09:23) - PLID 16781 - After deleting a row, we need to set the selection to the next procedure
		// in the list, not the first procedure.  To do this, we'll get the current selection, remove the row (not requery), then
		// set the selection back to what the current row was, which will be the next procedure from the one we deleted.
		long nCurSel = m_procedureCombo->CurSel;
		// (m.hancock 2006-10-02 16:31) - PLID 16781 - Check to see if the currently selected row is the last row.  If it is, then 
		// we should set control to the previous row, not the first row in the list.
		if(nCurSel+1 == m_procedureCombo->GetRowCount())
			nCurSel--;
		m_procedureCombo->RemoveRow(m_procedureCombo->CurSel);
		m_procedureCombo->CurSel = nCurSel;

		// (j.jones 2006-05-17 15:58) - the tablechecker will cause UpdateView() to fire
		//UpdateView();
		EnableAppropriateFields();
	}
	NxCatchAll("Error in CProcedureDlg::OnInactivate");
}

// (c.haag 2008-11-25 15:29) - PLID 10776 - Show inactive procedures
void CProcedureDlg::OnShowInactiveProcedures()
{
	try {
		CInactiveProceduresDlg dlg(this);
		dlg.DoModal();
		// Refresh the view if at least one procedure was activated
		if (dlg.GetActivatedProcedureCount() > 0) {
			m_procedureNameChecker.Refresh();
			long nSelID = (m_procedureCombo->CurSel > -1) ? VarLong(m_procedureCombo->GetValue(m_procedureCombo->CurSel, 0)) : -1;
			m_procedureCombo->Requery();
			if (nSelID > -1) {
				m_procedureCombo->TrySetSelByColumn(0, nSelID);
			}
			Refresh();
		}
	}
	NxCatchAll("Error in CProcedureDlg::OnShowInactiveProcedures");
}
