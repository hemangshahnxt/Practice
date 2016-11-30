// Custom1Dlg.cpp : implementation file
#include "stdafx.h"
#include "Custom1Dlg.h"
#include "MainFrm.h"
#include "GlobalUtils.h"
//#include "LabelControl.h"
//#include "SelectStatusDlg.h"
#include "NxStandard.h"
#include "EditComboBox.h"
#include "GlobalDataUtils.h"
#include "NxSecurity.h"
#include "InternationalUtils.h"
#include "PatientsRc.h"
#include "ContactView.h" // (k.messina 2010-04-12 11:15) - PLID 37957
#include "OHIPUtils.h"
#include "AlbertaHLINKUtils.h"
#include "MultiSelectDlg.h" // (j.armen 2011-06-21 17:14) - PLID 11490 - Added MultiSelect dlg for Custom Lists

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog
CCustom1Dlg::CCustom1Dlg(CWnd* pParent)
	: CPatientDialog(CCustom1Dlg::IDD, pParent)
	,
	m_custom1Checker(NetUtils::CustomList1, true),
	m_custom2Checker(NetUtils::CustomList2, true),
	m_custom3Checker(NetUtils::CustomList3, true),
	m_custom4Checker(NetUtils::CustomList4, true),
	m_custom5Checker(NetUtils::CustomList5, true),
	m_custom6Checker(NetUtils::CustomList6, true),
	m_customContactChecker(NetUtils::CustomContacts)
{
	//{{AFX_DATA_INIT(CCustom1Dlg)
	//}}AFX_DATA_INIT
	// (a.walling 2010-10-12 14:40) - PLID 40908
	m_id = -1;
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//m_bSettingBox = false;
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//m_bAutoRefreshing = false;

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Patient_Setup/edit_custom_field_labels.htm";
}

void CCustom1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom1Dlg)
	DDX_Control(pDX, IDC_CUSTOM_CHECK6, m_CustomCheck6);
	DDX_Control(pDX, IDC_CUSTOM_CHECK5, m_CustomCheck5);
	DDX_Control(pDX, IDC_CUSTOM_CHECK4, m_CustomCheck4);
	DDX_Control(pDX, IDC_CUSTOM_CHECK3, m_CustomCheck3);
	DDX_Control(pDX, IDC_CUSTOM_CHECK2, m_CustomCheck2);
	DDX_Control(pDX, IDC_CUSTOM_CHECK1, m_CustomCheck1);
	DDX_Control(pDX, IDC_CUSTOM_CHECK_BKG, m_checkBkg);
	DDX_Control(pDX, IDC_CUSTOM_DATE_BKG, m_dateBkg);
	DDX_Control(pDX, IDC_CUSTOM_LIST_BKG, m_listBkg);
	DDX_Control(pDX, IDC_CUSTOM_NOTE_BKG, m_noteBkg);
	DDX_Control(pDX, IDC_CUSTOM_TEXT_BKG, m_textBkg);
	DDX_Control(pDX, IDC_CUSTOM_TEXT1, m_nxeditCustomText1);
	DDX_Control(pDX, IDC_CUSTOM_TEXT2, m_nxeditCustomText2);
	DDX_Control(pDX, IDC_CUSTOM_TEXT3, m_nxeditCustomText3);
	DDX_Control(pDX, IDC_CUSTOM_TEXT4, m_nxeditCustomText4);
	DDX_Control(pDX, IDC_CUSTOM_TEXT5, m_nxeditCustomText5);
	DDX_Control(pDX, IDC_CUSTOM_TEXT6, m_nxeditCustomText6);
	DDX_Control(pDX, IDC_CUSTOM_TEXT7, m_nxeditCustomText7);
	DDX_Control(pDX, IDC_CUSTOM_TEXT8, m_nxeditCustomText8);
	DDX_Control(pDX, IDC_CUSTOM_TEXT9, m_nxeditCustomText9);
	DDX_Control(pDX, IDC_CUSTOM_TEXT10, m_nxeditCustomText10);
	DDX_Control(pDX, IDC_CUSTOM_TEXT11, m_nxeditCustomText11);
	DDX_Control(pDX, IDC_CUSTOM_TEXT12, m_nxeditCustomText12);
	DDX_Control(pDX, IDC_CUSTOM_NOTE, m_nxeditCustomNote);
	DDX_Control(pDX, IDC_CUSTOM_TEXT1_LABEL, m_nxstaticCustomText1Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT2_LABEL, m_nxstaticCustomText2Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT3_LABEL, m_nxstaticCustomText3Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT4_LABEL, m_nxstaticCustomText4Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT5_LABEL, m_nxstaticCustomText5Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT6_LABEL, m_nxstaticCustomText6Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT7_LABEL, m_nxstaticCustomText7Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT8_LABEL, m_nxstaticCustomText8Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT9_LABEL, m_nxstaticCustomText9Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT10_LABEL, m_nxstaticCustomText10Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT11_LABEL, m_nxstaticCustomText11Label);
	DDX_Control(pDX, IDC_CUSTOM_TEXT12_LABEL, m_nxstaticCustomText12Label);
	// (j.armen 2011-06-21 17:15) - PLID 11490 - Bindings for labels that are displayed on multiple selection
	DDX_Control(pDX, IDC_CUSTOM_MULTI_LIST1, m_nxlCustomMultiList1Label);
	DDX_Control(pDX, IDC_CUSTOM_MULTI_LIST2, m_nxlCustomMultiList2Label);
	DDX_Control(pDX, IDC_CUSTOM_MULTI_LIST3, m_nxlCustomMultiList3Label);
	DDX_Control(pDX, IDC_CUSTOM_MULTI_LIST4, m_nxlCustomMultiList4Label);
	DDX_Control(pDX, IDC_CUSTOM_MULTI_LIST5, m_nxlCustomMultiList5Label);
	DDX_Control(pDX, IDC_CUSTOM_MULTI_LIST6, m_nxlCustomMultiList6Label);
	DDX_Control(pDX, IDC_CUSTOM_LIST1_LABEL, m_nxstaticCustomList1Label);
	DDX_Control(pDX, IDC_CUSTOM_LIST2_LABEL, m_nxstaticCustomList2Label);
	DDX_Control(pDX, IDC_CUSTOM_LIST3_LABEL, m_nxstaticCustomList3Label);
	DDX_Control(pDX, IDC_CUSTOM_LIST4_LABEL, m_nxstaticCustomList4Label);
	DDX_Control(pDX, IDC_CUSTOM_LIST5_LABEL, m_nxstaticCustomList5Label);
	DDX_Control(pDX, IDC_CUSTOM_LIST6_LABEL, m_nxstaticCustomList6Label);
	DDX_Control(pDX, IDC_CUSTOM_CONTACT1_LABEL, m_nxstaticCustomContact1Label);
	DDX_Control(pDX, IDC_CUSTOM_MEMO1_LABEL, m_nxstaticCustomMemo1Label);
	DDX_Control(pDX, IDC_CUSTOM_CHECK1_LABEL, m_nxstaticCustomCheck1Label);
	DDX_Control(pDX, IDC_CUSTOM_CHECK2_LABEL, m_nxstaticCustomCheck2Label);
	DDX_Control(pDX, IDC_CUSTOM_CHECK3_LABEL, m_nxstaticCustomCheck3Label);
	DDX_Control(pDX, IDC_CUSTOM_CHECK4_LABEL, m_nxstaticCustomCheck4Label);
	DDX_Control(pDX, IDC_CUSTOM_CHECK5_LABEL, m_nxstaticCustomCheck5Label);
	DDX_Control(pDX, IDC_CUSTOM_CHECK6_LABEL, m_nxstaticCustomCheck6Label);
	DDX_Control(pDX, IDC_CUSTOM_DATE1_LABEL, m_nxstaticCustomDate1Label);
	DDX_Control(pDX, IDC_CUSTOM_DATE2_LABEL, m_nxstaticCustomDate2Label);
	DDX_Control(pDX, IDC_CUSTOM_DATE3_LABEL, m_nxstaticCustomDate3Label);
	DDX_Control(pDX, IDC_CUSTOM_DATE4_LABEL, m_nxstaticCustomDate4Label);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCustom1Dlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustom1Dlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CUSTOM_CHECK1, OnCustomCheck1)
	ON_BN_CLICKED(IDC_CUSTOM_CHECK2, OnCustomCheck2)
	ON_BN_CLICKED(IDC_CUSTOM_CHECK3, OnCustomCheck3)
	ON_BN_CLICKED(IDC_CUSTOM_CHECK4, OnCustomCheck4)
	ON_BN_CLICKED(IDC_CUSTOM_CHECK5, OnCustomCheck5)
	ON_BN_CLICKED(IDC_CUSTOM_CHECK6, OnCustomCheck6)
	// (j.armen 2011-06-21 17:16) - PLID 11490 - Added handling when multi select labels are clicked or hovered over
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_1, OnEditCustomList1)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_2, OnEditCustomList2)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_3, OnEditCustomList3)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_4, OnEditCustomList4)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_5, OnEditCustomList5)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_LIST_6, OnEditCustomList6)
	ON_BN_CLICKED(IDC_GOTO_CUSTOM_CONTACT, OnGotoCustomContact)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CCustom1Dlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCustom1Dlg)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_LIST1, 16 /* SelChosen */, OnSelChosenCustomList1, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_LIST2, 16 /* SelChosen */, OnSelChosenCustomList2, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_LIST3, 16 /* SelChosen */, OnSelChosenCustomList3, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_LIST4, 16 /* SelChosen */, OnSelChosenCustomList4, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_LIST5, 16 /* SelChosen */, OnSelChosenCustomList5, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_LIST6, 16 /* SelChosen */, OnSelChosenCustomList6, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_CONTACT1, 16 /* SelChosen */, OnSelChosenCustomContact1, VTS_I4)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_DATE1, 1 /* KillFocus */, OnKillFocusCustomDate1, VTS_NONE)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_DATE2, 1 /* KillFocus */, OnKillFocusCustomDate2, VTS_NONE)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_DATE3, 1 /* KillFocus */, OnKillFocusCustomDate3, VTS_NONE)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_DATE4, 1 /* KillFocus */, OnKillFocusCustomDate4, VTS_NONE)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_CONTACT1, 18 /* RequeryFinished */, OnRequeryFinishedCustomContact1, VTS_I2)
	ON_EVENT(CCustom1Dlg, IDC_CUSTOM_CONTACT1, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCustomContact1, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg message handlers							

void CCustom1Dlg::SetColor(OLE_COLOR nNewColor)
{
	m_checkBkg.SetColor(nNewColor);
	m_dateBkg.SetColor(nNewColor);
	m_listBkg.SetColor(nNewColor);
	m_noteBkg.SetColor(nNewColor);
	m_textBkg.SetColor(nNewColor);

	CPatientDialog::SetColor(nNewColor);
}

BOOL CCustom1Dlg::OnInitDialog() 
{
	try {
		m_nxtDate1 = GetDlgItemUnknown(IDC_CUSTOM_DATE1);
		m_nxtDate2 = GetDlgItemUnknown(IDC_CUSTOM_DATE2);
		m_nxtDate3 = GetDlgItemUnknown(IDC_CUSTOM_DATE3);
		m_nxtDate4 = GetDlgItemUnknown(IDC_CUSTOM_DATE4);
		m_bSavingDate1 = false;
		m_bSavingDate2 = false;
		m_bSavingDate3 = false;
		m_bSavingDate4 = false;

		m_CustomList1 = BindNxDataListCtrl(IDC_CUSTOM_LIST1);
		m_CustomList2 = BindNxDataListCtrl(IDC_CUSTOM_LIST2);
		m_CustomList3 = BindNxDataListCtrl(IDC_CUSTOM_LIST3);
		m_CustomList4 = BindNxDataListCtrl(IDC_CUSTOM_LIST4);
		m_CustomList5 = BindNxDataListCtrl(IDC_CUSTOM_LIST5);
		m_CustomList6 = BindNxDataListCtrl(IDC_CUSTOM_LIST6);
		m_CustomContact1 = BindNxDataListCtrl(IDC_CUSTOM_CONTACT1);

		IRowSettingsPtr pRow;
		pRow = m_CustomList1->GetRow(-1);
		// (j.armen 2011-06-21 17:17) - PLID 11490 - Changed no selection value to -1
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<No Item Selected>");
		m_CustomList1->InsertRow(pRow, 0);
		m_CustomList2->InsertRow(pRow, 0);
		m_CustomList3->InsertRow(pRow, 0);
		m_CustomList4->InsertRow(pRow, 0);
		m_CustomList5->InsertRow(pRow, 0);
		m_CustomList6->InsertRow(pRow, 0);
		
		// (j.armen 2011-06-21 17:21) - PLID 11490 - Added sentinal for selection multiple items
		pRow = m_CustomList1->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1,"<Multiple Items>");
		m_CustomList1->InsertRow(pRow, 0);
		m_CustomList2->InsertRow(pRow, 0);
		m_CustomList3->InsertRow(pRow, 0);
		m_CustomList4->InsertRow(pRow, 0);
		m_CustomList5->InsertRow(pRow, 0);
		m_CustomList6->InsertRow(pRow, 0);
		
		//the custom contact "no contact selected" row is added in OnRequeryFinishedCustomContact1

		// (j.jones 2010-05-04 14:06) - PLID 32325 - added caching
		g_propManager.CachePropertiesInBulk("CCustom1Dlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'UseOHIP' OR "
			"Name = 'OHIP_HealthNumberCustomField' OR "
			// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
			"Name = 'UseAlbertaHLINK' OR "
			"Name = 'Alberta_PatientULICustomField' "
			")",
			_Q(GetCurrentUserName()));

		// (a.walling 2010-10-12 14:54) - PLID 40908 - Don't load the id until UpdateView is called
		/*
		m_id = GetActivePatientID();
		*/

		m_changed = false;
		CNxDialog::OnInitDialog();

		SecureControls();
		GetDlgItem (IDC_CUSTOM_TEXT1)->SetFocus();

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}
										
void CCustom1Dlg::SetLabel(const long &FieldID)
{
	try {
		// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
		CRect rect;
		_RecordsetPtr CustomLabelsSet = CreateParamRecordset("SELECT * FROM CustomFieldsT WHERE ID = {INT}",FieldID);
		if(CustomLabelsSet->eof)
			return;
		CString WhatShowsInPat = CString(CustomLabelsSet->Fields->Item["Name"]->Value.bstrVal);
		switch(FieldID) {
		case 41:
			SetDlgItemText(IDC_CUSTOM_CHECK1_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CHECK1_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 42:
			SetDlgItemText(IDC_CUSTOM_CHECK2_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CHECK2_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 43:
			SetDlgItemText(IDC_CUSTOM_CHECK3_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CHECK3_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 44:
			SetDlgItemText(IDC_CUSTOM_CHECK4_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CHECK4_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 45:
			SetDlgItemText(IDC_CUSTOM_CHECK5_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CHECK5_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 46:
			SetDlgItemText(IDC_CUSTOM_CHECK6_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CHECK6_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 21:
			SetDlgItemText(IDC_CUSTOM_LIST1_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_LIST1_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 22:
			SetDlgItemText(IDC_CUSTOM_LIST2_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_LIST2_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 23:
			SetDlgItemText(IDC_CUSTOM_LIST3_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_LIST3_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 24:
			SetDlgItemText(IDC_CUSTOM_LIST4_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_LIST4_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 25:
			SetDlgItemText(IDC_CUSTOM_LIST5_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_LIST5_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 26:
			SetDlgItemText(IDC_CUSTOM_LIST6_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_LIST6_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 51:
			SetDlgItemText(IDC_CUSTOM_DATE1_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_DATE1_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 52:
			SetDlgItemText(IDC_CUSTOM_DATE2_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_DATE2_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 53:
			SetDlgItemText(IDC_CUSTOM_DATE3_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_DATE3_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 54:
			SetDlgItemText(IDC_CUSTOM_DATE4_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_DATE4_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 11:
			SetDlgItemText(IDC_CUSTOM_TEXT1_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT1_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 12:
			SetDlgItemText(IDC_CUSTOM_TEXT2_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT2_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 13:
			SetDlgItemText(IDC_CUSTOM_TEXT3_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT3_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 14:
			SetDlgItemText(IDC_CUSTOM_TEXT4_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT4_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 15:
			SetDlgItemText(IDC_CUSTOM_TEXT5_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT5_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 16:
			SetDlgItemText(IDC_CUSTOM_TEXT6_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT6_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 90: // (a.walling 2007-07-03 09:01) - PLID 15491 - Added more custom text fields
			SetDlgItemText(IDC_CUSTOM_TEXT7_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT7_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 91:
			SetDlgItemText(IDC_CUSTOM_TEXT8_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT8_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 92:
			SetDlgItemText(IDC_CUSTOM_TEXT9_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT9_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 93:
			SetDlgItemText(IDC_CUSTOM_TEXT10_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT10_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 94:
			SetDlgItemText(IDC_CUSTOM_TEXT11_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT11_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 95:
			SetDlgItemText(IDC_CUSTOM_TEXT12_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_TEXT12_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 17:
			SetDlgItemText(IDC_CUSTOM_MEMO1_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_MEMO1_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		case 31:
			SetDlgItemText(IDC_CUSTOM_CONTACT1_LABEL, ConvertToControlText(WhatShowsInPat));
			GetDlgItem(IDC_CUSTOM_CONTACT1_LABEL)->GetWindowRect(rect);
			ScreenToClient(rect);
			InvalidateRect(rect);
			break;
		}
		//Invalidate();
	}
	NxCatchAll("Could not load " + FieldID);
}

void CCustom1Dlg::PopulateCustomDataLabelsTable()
{
	/*
	CString tmpFieldStr, tmpShownStr;

	BEGIN_TRANS("Custom1");
		for(int i=1; i<7; i++){
			tmpFieldStr.Format("CheckLabel%li",i);
			tmpShownStr.Format("Custom Check Label %li",i);
			ExecuteSql("INSERT INTO CustomFieldsT (FieldName,Name) VALUES ('%s','%s')",tmpFieldStr,tmpShownStr);
		}
		for(int j=1; j<7; j++){
			m_CustomLabelsSet->AddNew();
			tmpFieldStr.Format("ComboLabel%li",j);
			m_CustomLabelsSet->Fields->Item["FieldName"]->Value = _variant_t(tmpFieldStr);
			tmpShownStr.Format("Custom Check Label %li",i);
			m_CustomLabelsSet->Fields->Item["Name"]->Value = _variant_t(tmpShownStr);
			m_CustomLabelsSet->Update();
		}
		for(int k=1; k<5; k++){
			m_CustomLabelsSet->AddNew();
			tmpFieldStr.Format("DateLabel%li",k);
			m_CustomLabelsSet->Fields->Item["FieldName"]->Value = _variant_t(tmpFieldStr);
			tmpShownStr.Format("Custom Check Label %li",i);
			m_CustomLabelsSet->Fields->Item["Name"]->Value = _variant_t(tmpShownStr);
			m_CustomLabelsSet->Update();
		}
		for(int l=1; l<7; l++){
			m_CustomLabelsSet->AddNew();
			tmpFieldStr.Format("TextLabel%li",l);
			m_CustomLabelsSet->Fields->Item["FieldName"]->Value = _variant_t(tmpFieldStr);
			tmpShownStr.Format("Custom Check Label %li",i);
			m_CustomLabelsSet->Fields->Item["Name"]->Value = _variant_t(tmpShownStr);
			m_CustomLabelsSet->Update();
		}
		m_CustomLabelsSet->AddNew();
		m_CustomLabelsSet->Fields->Item["FieldName"]->Value = _variant_t("CustomContact1");
		m_CustomLabelsSet->Fields->Item["Name"]->Value = _variant_t("Custom Contact");
		m_CustomLabelsSet->Update();

		m_CustomLabelsSet->AddNew();
		m_CustomLabelsSet->Fields->Item["FieldName"]->Value = _variant_t("MemoLabel1");
		m_CustomLabelsSet->Fields->Item["Name"]->Value = _variant_t("Custom Memo Box");
		m_CustomLabelsSet->Update();
	END_TRANS_CATCH_ALL("Custom1");
	*/
}
 
long CCustom1Dlg::GetLabelName(int nID)
{
	switch (nID) {
	
	// Checkbox labels
	case IDC_CUSTOM_CHECK1_LABEL: case IDC_CUSTOM_CHECK1: return 41; break;
	case IDC_CUSTOM_CHECK2_LABEL: case IDC_CUSTOM_CHECK2: return 42; break;
	case IDC_CUSTOM_CHECK3_LABEL: case IDC_CUSTOM_CHECK3: return 43; break;
	case IDC_CUSTOM_CHECK4_LABEL: case IDC_CUSTOM_CHECK4: return 44; break;
	case IDC_CUSTOM_CHECK5_LABEL: case IDC_CUSTOM_CHECK5: return 45; break;
	case IDC_CUSTOM_CHECK6_LABEL: case IDC_CUSTOM_CHECK6: return 46; break;
	
	// Combo labels
	case IDC_CUSTOM_LIST1_LABEL: case IDC_CUSTOM_LIST1: return 21; break;
	case IDC_CUSTOM_LIST2_LABEL: case IDC_CUSTOM_LIST2: return 22; break;
	case IDC_CUSTOM_LIST3_LABEL: case IDC_CUSTOM_LIST3: return 23; break;
	case IDC_CUSTOM_LIST4_LABEL: case IDC_CUSTOM_LIST4: return 24; break;
	case IDC_CUSTOM_LIST5_LABEL: case IDC_CUSTOM_LIST5: return 25; break;
	case IDC_CUSTOM_LIST6_LABEL: case IDC_CUSTOM_LIST6: return 26; break;
	case IDC_CUSTOM_CONTACT1_LABEL: case IDC_CUSTOM_CONTACT1: return 31; break;
	
	// Date labels
	case IDC_CUSTOM_DATE1_LABEL: case IDC_CUSTOM_DATE1: return 51; break;
	case IDC_CUSTOM_DATE2_LABEL: case IDC_CUSTOM_DATE2: return 52; break;
	case IDC_CUSTOM_DATE3_LABEL: case IDC_CUSTOM_DATE3: return 53; break;
	case IDC_CUSTOM_DATE4_LABEL: case IDC_CUSTOM_DATE4: return 54; break;
	
	// Memo label
	case IDC_CUSTOM_MEMO1_LABEL: case IDC_CUSTOM_NOTE: return 17; break;
	
	// Text labels
	case IDC_CUSTOM_TEXT1_LABEL: case IDC_CUSTOM_TEXT1: return 11; break;
	case IDC_CUSTOM_TEXT2_LABEL: case IDC_CUSTOM_TEXT2: return 12; break;
	case IDC_CUSTOM_TEXT3_LABEL: case IDC_CUSTOM_TEXT3: return 13; break;
	case IDC_CUSTOM_TEXT4_LABEL: case IDC_CUSTOM_TEXT4: return 14; break;
	case IDC_CUSTOM_TEXT5_LABEL: case IDC_CUSTOM_TEXT5: return 15; break;
	case IDC_CUSTOM_TEXT6_LABEL: case IDC_CUSTOM_TEXT6: return 16; break;

	case IDC_CUSTOM_TEXT7_LABEL: case IDC_CUSTOM_TEXT7: return 90; break; // (a.walling 2007-07-03 08:58) - PLID 15491 - Added more custom text fields
	case IDC_CUSTOM_TEXT8_LABEL: case IDC_CUSTOM_TEXT8: return 91; break;
	case IDC_CUSTOM_TEXT9_LABEL: case IDC_CUSTOM_TEXT9: return 92; break;
	case IDC_CUSTOM_TEXT10_LABEL: case IDC_CUSTOM_TEXT10: return 93; break;
	case IDC_CUSTOM_TEXT11_LABEL: case IDC_CUSTOM_TEXT11: return 94; break;
	case IDC_CUSTOM_TEXT12_LABEL: case IDC_CUSTOM_TEXT12: return 95; break;
	
	// Bad Id given
	default:
		ASSERT(FALSE);
		return 0;
		break;
	}
}

// (a.walling 2010-12-02 13:20) - PLID 41315
bool CCustom1Dlg::IsLabel(int nID)
{
	switch (nID) {
	
	// Checkbox labels
	case IDC_CUSTOM_CHECK1_LABEL:
	case IDC_CUSTOM_CHECK2_LABEL:
	case IDC_CUSTOM_CHECK3_LABEL:
	case IDC_CUSTOM_CHECK4_LABEL:
	case IDC_CUSTOM_CHECK5_LABEL:
	case IDC_CUSTOM_CHECK6_LABEL:
	
	// Combo labels
	case IDC_CUSTOM_LIST1_LABEL:
	case IDC_CUSTOM_LIST2_LABEL:
	case IDC_CUSTOM_LIST3_LABEL:
	case IDC_CUSTOM_LIST4_LABEL:
	case IDC_CUSTOM_LIST5_LABEL:
	case IDC_CUSTOM_LIST6_LABEL:
	case IDC_CUSTOM_CONTACT1_LABEL:
	
	// Date labels
	case IDC_CUSTOM_DATE1_LABEL:
	case IDC_CUSTOM_DATE2_LABEL:
	case IDC_CUSTOM_DATE3_LABEL:
	case IDC_CUSTOM_DATE4_LABEL:
	
	// Memo label
	case IDC_CUSTOM_MEMO1_LABEL:
	
	// Text labels
	case IDC_CUSTOM_TEXT1_LABEL:
	case IDC_CUSTOM_TEXT2_LABEL:
	case IDC_CUSTOM_TEXT3_LABEL:
	case IDC_CUSTOM_TEXT4_LABEL:
	case IDC_CUSTOM_TEXT5_LABEL:
	case IDC_CUSTOM_TEXT6_LABEL:

	case IDC_CUSTOM_TEXT7_LABEL:
	case IDC_CUSTOM_TEXT8_LABEL:
	case IDC_CUSTOM_TEXT9_LABEL:
	case IDC_CUSTOM_TEXT10_LABEL:
	case IDC_CUSTOM_TEXT11_LABEL:
	case IDC_CUSTOM_TEXT12_LABEL:
		return true;
	}

	return false;		
}

CString CCustom1Dlg::GetLabelNameDefault(int nID)
{
	switch (nID) {
	
	// Checkbox labels
	case IDC_CUSTOM_CHECK1_LABEL: case IDC_CUSTOM_CHECK1:  return "Custom Check 1"; break;
	case IDC_CUSTOM_CHECK2_LABEL: case IDC_CUSTOM_CHECK2:  return "Custom Check 2"; break;
	case IDC_CUSTOM_CHECK3_LABEL: case IDC_CUSTOM_CHECK3:  return "Custom Check 3"; break;
	case IDC_CUSTOM_CHECK4_LABEL: case IDC_CUSTOM_CHECK4:  return "Custom Check 4"; break;
	case IDC_CUSTOM_CHECK5_LABEL: case IDC_CUSTOM_CHECK5:  return "Custom Check 5"; break;
	case IDC_CUSTOM_CHECK6_LABEL: case IDC_CUSTOM_CHECK6:  return "Custom Check 6"; break;
	
	// Combo labels
	case IDC_CUSTOM_LIST1_LABEL: case IDC_CUSTOM_LIST1:  return "Custom Combo 1"; break;
	case IDC_CUSTOM_LIST2_LABEL: case IDC_CUSTOM_LIST2:  return "Custom Combo 2"; break;
	case IDC_CUSTOM_LIST3_LABEL: case IDC_CUSTOM_LIST3:  return "Custom Combo 3"; break;
	case IDC_CUSTOM_LIST4_LABEL: case IDC_CUSTOM_LIST4:  return "Custom Combo 4"; break;
	case IDC_CUSTOM_LIST5_LABEL: case IDC_CUSTOM_LIST5:  return "Custom Combo 5"; break;
	case IDC_CUSTOM_LIST6_LABEL: case IDC_CUSTOM_LIST6:  return "Custom Combo 6"; break;
	
	// Date labels
	case IDC_CUSTOM_DATE1_LABEL: case IDC_CUSTOM_DATE1:  return "Custom Date 1"; break;
	case IDC_CUSTOM_DATE2_LABEL: case IDC_CUSTOM_DATE2:  return "Custom Date 2"; break;
	case IDC_CUSTOM_DATE3_LABEL: case IDC_CUSTOM_DATE3:  return "Custom Date 3"; break;
	case IDC_CUSTOM_DATE4_LABEL: case IDC_CUSTOM_DATE4:  return "Custom Date 4"; break;
	
	// Memo label
	case IDC_CUSTOM_MEMO1_LABEL: case IDC_CUSTOM_NOTE:  return "Custom Memo 1"; break;
	
	// Text labels
	case IDC_CUSTOM_TEXT1_LABEL: case IDC_CUSTOM_TEXT1:  return "Custom Text 1"; break;
	case IDC_CUSTOM_TEXT2_LABEL: case IDC_CUSTOM_TEXT2:  return "Custom Text 2"; break;
	case IDC_CUSTOM_TEXT3_LABEL: case IDC_CUSTOM_TEXT3:  return "Custom Text 3"; break;
	case IDC_CUSTOM_TEXT4_LABEL: case IDC_CUSTOM_TEXT4:  return "Custom Text 4"; break;
	case IDC_CUSTOM_TEXT5_LABEL: case IDC_CUSTOM_TEXT5:  return "Custom Text 5"; break;
	case IDC_CUSTOM_TEXT6_LABEL: case IDC_CUSTOM_TEXT6:  return "Custom Text 6"; break;
	case IDC_CUSTOM_TEXT7_LABEL: case IDC_CUSTOM_TEXT7:  return "Custom Text 7"; break; // (a.walling 2007-07-03 08:58) - PLID 15491 - Added more custom text fields
	case IDC_CUSTOM_TEXT8_LABEL: case IDC_CUSTOM_TEXT8:  return "Custom Text 8"; break;
	case IDC_CUSTOM_TEXT9_LABEL: case IDC_CUSTOM_TEXT9:  return "Custom Text 9"; break;
	case IDC_CUSTOM_TEXT10_LABEL: case IDC_CUSTOM_TEXT10:  return "Custom Text 10"; break;
	case IDC_CUSTOM_TEXT11_LABEL: case IDC_CUSTOM_TEXT11:  return "Custom Text 11"; break;
	case IDC_CUSTOM_TEXT12_LABEL: case IDC_CUSTOM_TEXT12:  return "Custom Text 12"; break;


	case IDC_CUSTOM_CONTACT1_LABEL: case IDC_CUSTOM_CONTACT1:  return "Custom Contact"; break;
	
	// Bad Id given
	default:
		ASSERT(FALSE);
		return "";
		break;
	}
}

/* This version of this function is the new way which should be put back in once
there is time to put it both in General 1 and Contacts module also.  I took it out temporarily in order to 
for the release

void CCustom1Dlg::RenameCustomField(CString strOld, int nID) {
	CString strNew = strOld;

	long LabelID = GetLabelName(nID);

	BOOL bCancelled = FALSE, bSuccess = FALSE;
	while (!bCancelled && !bSuccess) {

		if (InputBox(this, "Enter new name for \"" + strOld + "\"", strNew, "") == IDOK) {
		
			// Make the data change
			strNew.TrimLeft(); strNew.TrimRight();
			if (strNew.GetLength() == 0) {
				strNew = strOld;
			}
			else if(strNew.GetLength() > 50) {
				strNew = strOld;
				AfxMessageBox("The label name you entered is too long.");
			}

			long field;
			field = GetLabelName(nID);

			// (j.jones 2007-08-02 08:38) - PLID 26866 - removed this functionality, deemed too dangerous
			/*
			//ask to see what they want to do 
			if (!IsRecordsetEmpty("SELECT FieldID FROM CustomFieldDataT WHERE FieldID = %li",field)) {
				 
				if (IDYES == AfxMessageBox("Do you want to clear the existing data for ALL patients? Selecting \"No\" will only rename the label and continue to use the existing data.", MB_YESNO)) {
				
					CString str, strSQL;
					strSQL.Format("SELECT PersonID FROM CustomFieldDataT WHERE FieldID = '%d'", field);
					long nRecordCount = GetRecordCount(strSQL);
					if (nRecordCount > 1)
					{
						str.Format("There are %d records of legacy data you will be removing information from by doing this!!!\nARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?",
							nRecordCount);
					}
					else
					{
						str = "There is potentially 1 record of legacy data you will be removing information from by doing this!!!\nARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?";
					}
					if (IDYES == AfxMessageBox(str, MB_YESNO))
					{	
						//ok, now we can do everything

						//I'm using this instead of the macro because I need to know if it failed or not
						try {
							// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction (so it doesn't show up in my searches!)
							CSqlTransaction trans("RenameCustomField");
							trans.Begin()
							ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %li", _Q(strNew), LabelID);
							ExecuteSql("DELETE FROM CustomFieldDataT WHERE FieldID = %li",field);
							trans.Commit();

							//if we got here, we succeded
							bSuccess = TRUE;
							SetLabel(field);

							// Update the view
							BeginWaitCursor();
							UpdateView();
							EndWaitCursor();

						}NxCatchAllCall("Error in CCustom1Dlg::CallModalDialog", RollbackTrans("RenameCustomField");)
					}
				}
				else {

					//just do the updating of the name
					ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %li", _Q(strNew), LabelID);
					SetLabel(field);
					// Update the view
					BeginWaitCursor();
					UpdateView();
					EndWaitCursor();
					bSuccess = TRUE;
				}
			}
			else /*{

				//we can just update
				//just do the updating of the name
				ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %li", _Q(strNew), LabelID);
				SetLabel(field);
				// Update the view
				BeginWaitCursor();
				UpdateView();
				EndWaitCursor();
				bSuccess = TRUE;
			}
				
		}
		else {

			//they cancelled
			bCancelled = TRUE;
		}				
	}
}*/

/*This version of this function is the old way*/
void CCustom1Dlg::RenameCustomField(CString strOld, const int nID) {
	// Show the input dialog
	CString strNew = ConvertFromControlText(strOld);
	long LabelID = GetLabelName(nID);
	int nRes;
	long field;

	const bool bInsert = (strOld == ""); // true if insert, false if update

	do {
		if (bInsert)
			nRes = InputBoxLimited(this, "Enter name for blank custom field", strNew, "", 50, false, false, NULL);
		else
			nRes = InputBoxLimited(this, "Enter new name for \"" + strOld + "\"", strNew, "", 50, false, false, NULL);

		if (nRes == IDCANCEL)
			return;

		strNew.TrimLeft();
		strNew.TrimRight();

		if (strNew == "")
			AfxMessageBox("Please type in a new name for this custom label or press cancel");
		else if (strNew.GetLength() > 50)
			AfxMessageBox("The label name you entered is too long.");

	} while ((strNew.GetLength() > 50) | (strNew == ""));

	try {

		// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
		if (!bInsert) // If the string isn't empty, we update
		{
			ExecuteParamSql("UPDATE CustomFieldsT SET Name = {STRING} WHERE ID = {INT}", strNew, LabelID);
		}
		// m.carlson:Feb. 5, 2004	PLID #10737
		else	// insert record instead of update
		{
			if (LabelID >= 21 && LabelID <= 26) // type = 11, list
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES ({INT},{STRING},{INT})",LabelID,strNew,11);
			else if (LabelID == 31) // type = 12, contact
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES ({INT},{STRING},{INT})",LabelID,strNew,12);
			else if (LabelID >= 41 && LabelID <= 46) // type = 13, checkbox
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES ({INT},{STRING},{INT})",LabelID,strNew,13);
			else if (LabelID >= 51 && LabelID <= 54) // type = 21, date
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES ({INT},{STRING},{INT})",LabelID,strNew,21);
			else // assume it's type = 1, text field
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES ({INT},{STRING},{INT})",LabelID,strNew,1);
		}
		// (b.cardillo 2006-05-19 17:28) - PLID 20735 - We know the new name for this label, so 
		// update the global cache so it won't have to be reloaded in its entirety.
		SetCustomFieldNameCachedValue(LabelID, strNew);
		// (b.cardillo 2006-07-06 16:27) - PLID 20737 - Notify all the other users as well.
		CClient::RefreshTable_CustomFieldName(LabelID, strNew);

		field = GetLabelName(nID);

		//offer to clear the label

		// (j.jones 2007-08-02 08:38) - PLID 26866 - removed this functionality, deemed too dangerous
		/*
		if (!IsRecordsetEmpty("SELECT FieldID FROM CustomFieldDataT WHERE FieldID = %li",field)
			&& IDYES == AfxMessageBox("Do you want to clear the existing data for ALL patients? Selecting \"No\" will only rename the label and continue to use the existing data.", MB_YESNO))
		{
			CString str, strSQL;
			strSQL.Format("SELECT PersonID FROM CustomFieldDataT WHERE FieldID = '%d'", field);
			long nRecordCount = GetRecordCount(strSQL);
			if (nRecordCount > 1)
			{
				str.Format("There are %d records of legacy data you will be removing information from by doing this!!!\nARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?",
					nRecordCount);
			}
			else
			{
				str = "There is 1 record of legacy data you will be removing information from by doing this!!!\nARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?";
			}
			if (IDYES == AfxMessageBox(str, MB_YESNO))
			{	
				ExecuteSql("DELETE FROM CustomFieldDataT WHERE FieldID = %li",field);
			}
		}
		*/

	} NxCatchAll("Error in changing custom field. CCustom1Dlg::RenameCustomField");
	
	SetLabel(field);

	// Update the view
	BeginWaitCursor();
	UpdateView();
	EndWaitCursor();
}

void CCustom1Dlg::CallModalDialog(int nID)
{
	// (a.walling 2010-12-02 13:20) - PLID 41315
	if (!IsLabel(nID)) {
		return;
	}

	// Find out which label is being edited
	long LabelID = GetLabelName(nID);
	CString strOld;

	if(LabelID == 0)	//didn't click on a label
		return;

	// First ensure that this user has access to this function
	if (!UserPermission(CustomLabel))
		// Just don't do anything (a message is given in UserPermission function -BVB)
		return;

	try
	{
		// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
		if(ReturnsRecordsParam("SELECT * FROM CustomFieldsT WHERE ID = {INT}", LabelID)) {
			// The record was found so modify it
			
			// Get the text that's showing now
			GetDlgItemText(nID,strOld);

			// Create good text to display on the input dialog
			strOld.TrimLeft(); strOld.TrimRight();
			if (strOld.GetLength() == 0) {
				strOld = GetLabelNameDefault(nID);
			}
			
			RenameCustomField(strOld, nID);

		} else {
			// The record wasn't found so we send the null string, to INSERT instead of UPDATE
			RenameCustomField("",nID);
		}

	} NxCatchAll("Error in changing custom field. Custom1Dlg::CallModalDialog");
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CCustom1Dlg::Hotkey(int key)
{
	if (key == VK_ESCAPE)
	{//do undo here
	}

	//unhandled
	return 1;
}

void CCustom1Dlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{	
	static unsigned long labelStamp = 0;

	CWaitCursor pWait;

	try {
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages
		CheckFocus();

		//TES 6/12/03: It is VITAL that we StoreDetails BEFORE getting the patient id; because it will often
		//happen that UpdateView() will be called when switching patients, meaning that we want to save to the
		//old patient, then write to the new.
		StoreDetails();

		IRowSettingsPtr pRow;
		pRow = m_CustomList1->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<No Item Selected>");

		// (j.armen 2011-06-21 17:51) - PLID 11490 - added row for multiple items
		IRowSettingsPtr pRow2;
		pRow2 = m_CustomList1->GetRow(-1);
		pRow2->PutValue(0,(long)-2);
		pRow2->PutValue(1,"<Multiple Items>");

		// (a.walling 2010-10-12 17:58) - PLID 40908
		if (m_custom1Checker.Changed()) { m_ForceRefresh = true; m_CustomList1->Requery(); m_CustomList1->InsertRow(pRow, 0); m_CustomList1->InsertRow(pRow2, 0);}
		if (m_custom2Checker.Changed()) { m_ForceRefresh = true; m_CustomList2->Requery(); m_CustomList2->InsertRow(pRow, 0); m_CustomList2->InsertRow(pRow2, 0);}
		if (m_custom3Checker.Changed()) { m_ForceRefresh = true; m_CustomList3->Requery(); m_CustomList3->InsertRow(pRow, 0); m_CustomList3->InsertRow(pRow2, 0);}
		if (m_custom4Checker.Changed()) { m_ForceRefresh = true; m_CustomList4->Requery(); m_CustomList4->InsertRow(pRow, 0); m_CustomList4->InsertRow(pRow2, 0);}
		if (m_custom5Checker.Changed()) { m_ForceRefresh = true; m_CustomList5->Requery(); m_CustomList5->InsertRow(pRow, 0); m_CustomList5->InsertRow(pRow2, 0);}
		if (m_custom6Checker.Changed()) { m_ForceRefresh = true; m_CustomList6->Requery(); m_CustomList6->InsertRow(pRow, 0); m_CustomList6->InsertRow(pRow2, 0);}
		if (m_customContactChecker.Changed()) { m_ForceRefresh = true; m_CustomContact1->Requery(); m_CustomContact1->InsertRow(pRow, 0); }
		
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();
		
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		// (a.walling 2010-10-12 17:58) - PLID 40908
		if (bForceRefresh || m_ForceRefresh) {
			RestoreAllBoxes();			
		}
		
		SetAllLabels();
	
		m_ForceRefresh = false;
	}
	NxCatchAll("Error updating view");
}

// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
/*CString CCustom1Dlg::LabelToField(int nID)
{
	CString str;
	m_map.Lookup(nID, str);
	return str;
}*/

// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries and got rid of lots of duplicated code
// (s.tullis 2013-10-02 11:33) - PLID 37865 - The Custom Note text box in the Custom tab in the Patients Module correctly does not allow > 255 chars but fails to highlight what was truncated.
void CCustom1Dlg::StoreCustomText(int nID)
{
	int nFieldID = GetLabelName(nID);
	CString strFieldName = GetLabelNameDefault(nID);
	int nFieldType = 1;

	CString str;
	CString strlost = "Characters lost in the truncation: ";
	GetDlgItemText(nID, str);
	if(!str.IsEmpty()) {
		if(str.GetLength() > 255) {
			strlost+=str.Mid(256, str.GetLength()-256);
			str = str.Left(255);
			AfxMessageBox("This edit box can only hold 255 characters of data.  Only the first 255 characters will be saved." );
			MessageBox(strlost);
			
			SetDlgItemText(nID,str);
		}
	}

	_variant_t varStr((LPCTSTR)str);

	StoreCustomField(nFieldID, strFieldName, nFieldType, g_cvarNull, varStr, g_cvarNull);
}

// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries and got rid of lots of duplicated code
void CCustom1Dlg::StoreCustomDate(int nID, bool& bSavingDate, COleDateTime& dtDate, NXTIMELib::_DNxTimePtr nxtDate)
{
	int nFieldID = GetLabelName(nID);
	CString strFieldName = GetLabelNameDefault(nID);
	int nFieldType = 21;

	if(!bSavingDate) {
		bSavingDate = true;

		COleDateTime dt,dttemp;
		dt = nxtDate->GetDateTime();
		
		dttemp.ParseDateTime("01/01/1753");
		if(nxtDate->GetStatus() == 2) {
			AfxMessageBox("Invalid Date Entered");
			RestoreBox(nID);
			GetDlgItem(nID)->SetFocus();
			bSavingDate = false;
			return;
		}
		else if(dt < dttemp) {
			AfxMessageBox("Practice cannot store dates prior to January 1, 1753.");
			RestoreBox(nID);
			GetDlgItem(nID)->SetFocus();
			bSavingDate = false;
			return;
		}
		dtDate = dt;

		_variant_t varDate = g_cvarNull;
		if(nxtDate->GetStatus() == 1) {
			varDate = _variant_t(dtDate, VT_DATE);
		}

		StoreCustomField(nFieldID, strFieldName, nFieldType, g_cvarNull, g_cvarNull, varDate);

		bSavingDate = false;
	}
}

// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries and got rid of lots of duplicated code
void CCustom1Dlg::StoreCustomCheckBox(int nID)
{
	int nFieldID = GetLabelName(nID);
	CString strFieldName = GetLabelNameDefault(nID);
	int nFieldType = 13;

	_variant_t varChecked = g_cvarNull;
	if(((CButton*)GetDlgItem(nID))->GetCheck()) {
		varChecked = _variant_t((long)1, VT_I4);
	}

	StoreCustomField(nFieldID, strFieldName, nFieldType, varChecked, g_cvarNull, g_cvarNull);
}	

// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries and got rid of lots of duplicated code
// (j.armen 2011-06-21 17:23) - PLID 11490 Repurposed this function to handle storing Custom Lists in the new table structure
void CCustom1Dlg::StoreCustomList(NXDATALISTLib::_DNxDataListPtr customList, int nFieldID, UINT customListIDC)
{
	// (j.armen 2011-11-17 10:23) - PLID 11490 - If we don't have write access, we are not allowed to edit the custom lists
	if (!(GetCurrentUserPermissions(bioPatient) & SPT___W_______))
		return;

	CParamSqlBatch sqlBatch;
	_RecordsetPtr rs;
	CArray<long, long> arynSelection;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, FormatString("CustomListDataT.FieldID = %li", nFieldID));

	// (j.armen 2011-06-21 17:38) - PLID 11490 - If we call from a list, get the selection, if we call from a label, do multi select
	int nVal;
	switch(customListIDC)
	{
		case IDC_CUSTOM_LIST1:
		case IDC_CUSTOM_LIST2:
		case IDC_CUSTOM_LIST3:
		case IDC_CUSTOM_LIST4:
		case IDC_CUSTOM_LIST5:
		case IDC_CUSTOM_LIST6:
			if(customList->GetCurSel() == -1)
			{
				nVal = -1;
			}
			else
			{
				nVal = VarLong(customList->GetValue(customList->GetCurSel(), 0));
			}
			break;
		case IDC_CUSTOM_MULTI_LIST1:
		case IDC_CUSTOM_MULTI_LIST2:
		case IDC_CUSTOM_MULTI_LIST3:
		case IDC_CUSTOM_MULTI_LIST4:
		case IDC_CUSTOM_MULTI_LIST5:
		case IDC_CUSTOM_MULTI_LIST6:
			nVal = -2;
			break;
		default:
			ASSERT(FALSE);
			break;
	}
	// (j.armen 2011-06-21 17:39) - PLID 11490 - Start by deleting current custom records for the patient
	sqlBatch.Add("DELETE FROM CustomListDataT WHERE FieldID = {INT} AND PersonID = {INT}; ", nFieldID, m_id);
	switch(nVal)
	{
		// (j.armen 2011-06-21 17:40) - PLID 11490 - Show Multiple Selection
		case -2:
			rs = CreateParamRecordset(
			"SELECT CustomListItemsID FROM CustomListDataT "
			"WHERE FieldID = {INT} AND PersonID = {INT}", nFieldID, m_id);
		
			while(!rs->eof) 
			{
				arynSelection.Add(AdoFldLong(rs, "CustomListItemsID"));
				rs->MoveNext();
			}

			dlg.PreSelect(arynSelection);
			if(IDOK == dlg.Open("CustomListItemsT", FormatString("CustomFieldID = %d", nFieldID), "ID", "Text", "Select Custom List Items", 0))
			{
				dlg.FillArrayWithIDs(arynSelection);
				for(int i = 0; i < arynSelection.GetCount(); i++)
				{
					sqlBatch.Add("INSERT INTO CustomListDataT (PersonID, FieldID, CustomListItemsID) VALUES({INT}, {INT}, {INT})", m_id, nFieldID, arynSelection.GetAt(i));
				}
				sqlBatch.Execute(GetRemoteData());
			}
			break;
		// (j.armen 2011-06-21 17:40) - PLID 11490 - nothing was selected
		case -1:
			sqlBatch.Execute(GetRemoteData());
			break;
		// (j.armen 2011-06-21 17:41) - PLID 11490 - Add the single selected item
		default:
			sqlBatch.Add("INSERT INTO CustomListDataT (PersonID, FieldID, CustomListItemsID) VALUES({INT}, {INT}, {INT}); ", m_id, nFieldID, VarLong(customList->GetValue(customList->GetCurSel(), 0)));
			sqlBatch.Execute(GetRemoteData());
			break;
	}
	// (j.armen 2011-06-21 17:41) - PLID 11490 - Refresh custom fields
	RestoreAllBoxes();
}	

// (j.armen 2011-06-22 08:41) - PLID 11490 - Old functionality of the custom lists now used just for the Custom Contact
void CCustom1Dlg::StoreCustomContact(int nID, NXDATALISTLib::_DNxDataListPtr customList)
{
	int nFieldID = GetLabelName(nID);
	CString strFieldName = GetLabelNameDefault(nID);

	_variant_t varSelected = g_cvarNull;
	if(customList->GetCurSel()!=-1 && VarLong(customList->GetValue(customList->CurSel,0),0) != 0) {
		varSelected = _variant_t(VarLong(customList->GetValue(customList->GetCurSel(),0)), VT_I4);
	} else {
		customList->CurSel = -1;
	}

	StoreCustomField(nFieldID, strFieldName, 12, varSelected, g_cvarNull, g_cvarNull);
}	

// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries and got rid of lots of duplicated code
void CCustom1Dlg::StoreCustomField(int nFieldID, const CString& strFieldName, int nFieldType, const _variant_t& varInt, const _variant_t& varText, const _variant_t& varDate)
{
	// (j.armen 2011-06-23 15:25) - PLID 44253 - In the event of future programming or 
	//some unknown existing function, prevent it from calling this code if it is trying 
	//to save to a custom list
	if(nFieldID >= 21 && nFieldID <=26)
	{
		ASSERT(FALSE);
		return;
	}

	if (varInt.vt == VT_NULL && varText.vt == VT_NULL && varDate.vt == VT_NULL) {
		ExecuteParamSql(
			"DELETE FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}"
			, m_id
			, nFieldID
		);
	} else {
		CParamSqlBatch batch;
		batch.Declare("SET NOCOUNT ON");
		batch.Declare("DECLARE @CreatedCustomFields INT");
		batch.Add(
			"IF NOT EXISTS (SELECT ID FROM CustomFieldsT WHERE ID = {INT})\r\n"
			"BEGIN "
			"SET @CreatedCustomFields = 1 "
			"INSERT INTO CustomFieldsT (ID, Name, Type) VALUES ({INT}, {STRING}, {INT})"
			"END "
			, nFieldID
			, nFieldID
			, strFieldName
			, nFieldType
		);
		batch.Add(
			"UPDATE CustomFieldDataT SET IntParam = {VT_I4}, TextParam = {VT_BSTR}, DateParam = {VT_DATE} WHERE PersonID = {INT} AND FieldID = {INT}"
			, varInt
			, varText
			, varDate
			, m_id
			, nFieldID
		);

		batch.Add(
			"IF @@ROWCOUNT = 0\r\n"
			"INSERT INTO CustomFieldDataT(PersonID, FieldID, IntParam, TextParam, DateParam) VALUES({INT}, {INT}, {VT_I4}, {VT_BSTR}, {VT_DATE})"
			, m_id
			, nFieldID
			, varInt
			, varText
			, varDate
		);

		batch.Declare("SET NOCOUNT OFF");
		batch.Declare("SELECT @CreatedCustomFields AS CreatedCustomFields");

		_RecordsetPtr prs = batch.CreateRecordset(GetRemoteData());
		int nCreatedCustomFields = AdoFldLong(prs, "CreatedCustomFields", 0);
		if (nCreatedCustomFields) {
			SetLabel(nFieldID);
		}
	}
}

void CCustom1Dlg::StoreBox(int nID)
{
	try 
	{
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	/*if (m_bAutoRefreshing && GetDlgItem(nID) == GetFocus()) 
		return;*/

	// (j.jones 2010-05-04 14:06) - PLID 32325 - cache what the current Health Number field is
	long nHealthNumberCustomField = 1;
	if(UseOHIP()) {
		nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
	}
	// (j.jones 2010-11-04 16:25) - PLID 39620 - supported this for Alberta as well
	else if(UseAlbertaHLINK()) {
		nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
	}

	// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries

	switch (nID) 
	{	case IDC_CUSTOM_TEXT1:
		case IDC_CUSTOM_TEXT2:
		case IDC_CUSTOM_TEXT3:
		case IDC_CUSTOM_TEXT4:
		case IDC_CUSTOM_TEXT5:
		case IDC_CUSTOM_TEXT6:
		case IDC_CUSTOM_TEXT7: // (a.walling 2007-07-03 09:05) - PLID 15491 - Added more custom text fields
		case IDC_CUSTOM_TEXT8:
		case IDC_CUSTOM_TEXT9:
		case IDC_CUSTOM_TEXT10:
		case IDC_CUSTOM_TEXT11:
			// (a.walling 2010-11-01 11:57) - PLID 41315 - This used to break here, therefore skipping the below code
		case IDC_CUSTOM_TEXT12:
			StoreCustomText(nID);

			// (j.jones 2010-05-04 14:06) - PLID 32325 - if OHIP is enabled, see if we changed the health card number,
			// and if so, update the patient toolbar
			// (j.jones 2010-11-08 13:59) - PLID 39620 - same for Alberta
			if((UseOHIP() || UseAlbertaHLINK()) && nHealthNumberCustomField == GetLabelName(nID)) {
				CString str;
				GetDlgItemText(nID, str);
				GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetOHIPHealthCardColumn(), _bstr_t(str));
			}
			break;
		case IDC_CUSTOM_NOTE:
			StoreCustomText(nID);

			// (a.walling 2010-11-01 11:57) - PLID 41315 - Apparently this was trying to highlight the part that would be cut off,
			// but the string was already truncated, so it did nothing.
			break;
		case IDC_CUSTOM_DATE1: 
			StoreCustomDate(nID, m_bSavingDate1, m_dtDate1, m_nxtDate1);
			break;
		case IDC_CUSTOM_DATE2:
			StoreCustomDate(nID, m_bSavingDate2, m_dtDate2, m_nxtDate2);
			break;
		case IDC_CUSTOM_DATE3:
			StoreCustomDate(nID, m_bSavingDate3, m_dtDate3, m_nxtDate3);
			break;
		case IDC_CUSTOM_DATE4:
			StoreCustomDate(nID, m_bSavingDate4, m_dtDate4, m_nxtDate4);
			break;
		case IDC_CUSTOM_CHECK1:
		case IDC_CUSTOM_CHECK2:
		case IDC_CUSTOM_CHECK3:
		case IDC_CUSTOM_CHECK4:
		case IDC_CUSTOM_CHECK5:
		case IDC_CUSTOM_CHECK6:
			StoreCustomCheckBox(nID);
			break;
		// (j.armen 2011-06-21 17:42) - PLID 11490 - Custom lists call the new StoreCustomList function
		case IDC_CUSTOM_LIST1:
			StoreCustomList(m_CustomList1, 21, IDC_CUSTOM_LIST1);
			break;
		case IDC_CUSTOM_LIST2:
			StoreCustomList(m_CustomList2, 22, IDC_CUSTOM_LIST2);
			break;
		case IDC_CUSTOM_LIST3:
			StoreCustomList(m_CustomList3, 23, IDC_CUSTOM_LIST3);
			break;
		case IDC_CUSTOM_LIST4:
			StoreCustomList(m_CustomList4, 24, IDC_CUSTOM_LIST4);
			break;
		case IDC_CUSTOM_LIST5:
			StoreCustomList(m_CustomList5, 25, IDC_CUSTOM_LIST5);
			break;
		case IDC_CUSTOM_LIST6:
			StoreCustomList(m_CustomList6, 26, IDC_CUSTOM_LIST6);
			break;
		// (j.armen 2011-06-21 17:43) - PLID 11490 - Custom contacts were using the same StoreCustomList.
		// since the functionality of that function changed, created a new function to take over old functionality
		case IDC_CUSTOM_CONTACT1:
			StoreCustomContact(nID, m_CustomContact1);
			break;
		}
	}
	NxCatchAll("Error in Custom::StoreBox: ");
}

void CCustom1Dlg::StoreDetails()
{
	try {
		//First off, we can't trust m_changed or GetFocus() for NxTimes.
		// (a.walling 2008-10-06 13:16) - PLID 31595 - ASSERTions when casting an invalid COleDateTime to a DATE
		if(m_nxtDate1->GetDateTime() != m_dtDate1.m_dt || m_nxtDate1->GetStatus() == 2) {
			StoreBox(IDC_CUSTOM_DATE1);
		}
		if(m_nxtDate2->GetDateTime() != m_dtDate2.m_dt || m_nxtDate2->GetStatus() == 2) {
			StoreBox(IDC_CUSTOM_DATE2);
		}
		if(m_nxtDate3->GetDateTime() != m_dtDate3.m_dt || m_nxtDate3->GetStatus() == 2) {
			StoreBox(IDC_CUSTOM_DATE3);
		}
		if(m_nxtDate4->GetDateTime() != m_dtDate4.m_dt || m_nxtDate4->GetStatus() == 2) {
			StoreBox(IDC_CUSTOM_DATE4);
		}

		if (!m_changed)
			return;
		m_changed = false;
		if (!GetFocus() || !IsWindow(GetFocus()->m_hWnd))//why are we here?
			return;
		int nID = GetFocus()->GetDlgCtrlID();
		StoreBox(nID);
	} NxCatchAll("Error in StoreDetails");
}

void CCustom1Dlg::RestoreBox(int nID)
{	
	try
	{
		// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
		/*if (m_bAutoRefreshing && GetDlgItem(nID) == GetFocus()) 
			return;*/
		StoreDetails();
		_RecordsetPtr rs(__uuidof(Recordset));
		CString sql;
		_variant_t var;
		// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries
		switch (nID) 
		{	case IDC_CUSTOM_TEXT1:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,11);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT2:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,12);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT3:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,13);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT4:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,14);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT5:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,15);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT6:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,16);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT7: // (a.walling 2007-07-03 09:08) - PLID 15491 - Added more custom text fields
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,90);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT8: 
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,91);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT9: 
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,92);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT10: 
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,93);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT11: 
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,94);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_TEXT12: 
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,95);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_NOTE:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,17);
				if(rs->eof)
					SetSafeDlgText(nID,"");
				else
					SetSafeDlgText(nID, CString(rs->Fields->Item["TextParam"]->Value.bstrVal));
				rs->Close();
				break;
			case IDC_CUSTOM_DATE1:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,51);
				if(rs->eof || rs->Fields->Item["DateParam"]->Value.vt == VT_NULL) {					
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate1 = g_cdtInvalid;
					m_nxtDate1->Clear();
				}
				else {
					m_dtDate1 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate1->SetDateTime(m_dtDate1);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_DATE2:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,52);
				if(rs->eof || rs->Fields->Item["DateParam"]->Value.vt == VT_NULL) {
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate2 = g_cdtInvalid;
					m_nxtDate2->Clear();
				}
				else {
					m_dtDate2 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate2->SetDateTime(m_dtDate2);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_DATE3:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,53);
				if(rs->eof || rs->Fields->Item["DateParam"]->Value.vt == VT_NULL) {
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate3 = g_cdtInvalid;
					m_nxtDate3->Clear();
				}
				else {
					m_dtDate3 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate3->SetDateTime(m_dtDate3);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_DATE4:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,54);
				if(rs->eof || rs->Fields->Item["DateParam"]->Value.vt == VT_NULL) {
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate4 = g_cdtInvalid;
					m_nxtDate4->Clear();
				}
				else {
					m_dtDate4 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate4->SetDateTime(m_dtDate4);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_CHECK1:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,41);
				if(rs->eof)
					((CButton*)GetDlgItem(nID))->SetCheck(0);
				else {
					var = rs->Fields->Item["IntParam"]->Value;
					if(var.vt == VT_NULL)
						((CButton*)GetDlgItem(nID))->SetCheck(0);
					else
						((CButton*)GetDlgItem(nID))->SetCheck(var.lVal);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_CHECK2:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,42);
				if(rs->eof)
					((CButton*)GetDlgItem(nID))->SetCheck(0);
				else {
					var = rs->Fields->Item["IntParam"]->Value;
					if(var.vt == VT_NULL)
						((CButton*)GetDlgItem(nID))->SetCheck(0);
					else
						((CButton*)GetDlgItem(nID))->SetCheck(var.lVal);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_CHECK3:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,43);
				if(rs->eof)
					((CButton*)GetDlgItem(nID))->SetCheck(0);
				else {
					var = rs->Fields->Item["IntParam"]->Value;
					if(var.vt == VT_NULL)
						((CButton*)GetDlgItem(nID))->SetCheck(0);
					else
						((CButton*)GetDlgItem(nID))->SetCheck(var.lVal);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_CHECK4:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,44);
				if(rs->eof)
					((CButton*)GetDlgItem(nID))->SetCheck(0);
				else {
					var = rs->Fields->Item["IntParam"]->Value;
					if(var.vt == VT_NULL)
						((CButton*)GetDlgItem(nID))->SetCheck(0);
					else
						((CButton*)GetDlgItem(nID))->SetCheck(var.lVal);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_CHECK5:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,45);
				if(rs->eof)
					((CButton*)GetDlgItem(nID))->SetCheck(0);
				else {
					var = rs->Fields->Item["IntParam"]->Value;
					if(var.vt == VT_NULL)
						((CButton*)GetDlgItem(nID))->SetCheck(0);
					else
						((CButton*)GetDlgItem(nID))->SetCheck(var.lVal);
				}
				rs->Close();
				break;
			case IDC_CUSTOM_CHECK6:
				rs = CreateParamRecordset("SELECT IntParam, TextParam, DateParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}",m_id,46);
				if(rs->eof)
					((CButton*)GetDlgItem(nID))->SetCheck(0);
				else {
					var = rs->Fields->Item["IntParam"]->Value;
					if(var.vt == VT_NULL)
						((CButton*)GetDlgItem(nID))->SetCheck(0);
					else
						((CButton*)GetDlgItem(nID))->SetCheck(var.lVal);
				}
				rs->Close();
				break;
				// (j.armen 2011-06-22 09:00) - PLID 11490 - Removed functions that attempted to refresh individual boxes as they were never used
			case IDC_CUSTOM_LIST1:
			case IDC_CUSTOM_LIST2:
			case IDC_CUSTOM_LIST3:
			case IDC_CUSTOM_LIST4:
			case IDC_CUSTOM_LIST5:
			case IDC_CUSTOM_LIST6:
				ASSERT(FALSE);
				break;
			case IDC_CUSTOM_CONTACT1:
			{	
				//(e.lally 2005-11-28) PLID 18153 - add ability to make other contacts inactive
				rs = CreateParamRecordset("SELECT IntParam AS Val, Last + ', ' + First + ' ' + Middle AS Name FROM CustomFieldDataT "
					"INNER JOIN PersonT ON CustomFieldDataT.IntParam = PersonT.ID "
					"WHERE CustomFieldDataT.PersonID = {INT} AND FieldID = {INT}",m_id,31);
				if(!rs->eof) {
					if(m_CustomContact1->TrySetSelByColumn(0,rs->Fields->Item["Val"]->Value) == -1){
						m_CustomContact1->PutCurSel(-1);
						m_CustomContact1->PutComboBoxText(_bstr_t(AdoFldString(rs, "Name")));
					}
				}
				rs->Close();
				break;
			}
		}
		m_changed = false;
	}
	NxCatchAll("Could not set box. Custom1Dlg::RestoreBox()");

}


void CCustom1Dlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
/*	if(bShow){
		((CNxEdit *)GetDlgItem(IDC_CUSTOM_TEXT1))->SetSel(0,-1);
		GetDlgItem(IDC_CUSTOM_TEXT1)->SetFocus();}*/
	if (!bShow) {
		GetDlgItem(IDC_CUSTOM_TEXT1)->SetFocus();
	}
	CNxDialog::OnShowWindow(bShow, nStatus);
}

void CCustom1Dlg::OnCustomCheck1() 
{
	StoreBox(IDC_CUSTOM_CHECK1);
}

void CCustom1Dlg::OnCustomCheck2() 
{
	StoreBox(IDC_CUSTOM_CHECK2);
}

void CCustom1Dlg::OnCustomCheck3() 
{
	StoreBox(IDC_CUSTOM_CHECK3);
}

void CCustom1Dlg::OnCustomCheck4() 
{
	StoreBox(IDC_CUSTOM_CHECK4);
}

void CCustom1Dlg::OnCustomCheck5() 
{
	StoreBox(IDC_CUSTOM_CHECK5);
}

void CCustom1Dlg::OnCustomCheck6() 
{
	StoreBox(IDC_CUSTOM_CHECK6);
}

BOOL CCustom1Dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	static WORD command;

	command = HIWORD(wParam);
	if (command == EN_UPDATE) {
		m_changed = true;
	}
	if (command == EN_KILLFOCUS)
	{	if (!m_changed)
			return CNxDialog::OnCommand(wParam, lParam);
		m_changed = false;
		int nID = LOWORD(wParam);
		StoreBox(nID);
	}
	return CNxDialog::OnCommand(wParam, lParam);
}

void CCustom1Dlg::UpdateLabel(UINT nID, const CString &strNewLabelText)
{
	CString strExisting;
	GetDlgItemText(nID, strExisting);
	CString strNew = ConvertToControlText(strNewLabelText);
	if (strExisting != strNew) {
		SetDlgItemText(nID, strNew);
		CRect rc;
		GetDlgItem(nID)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);
	}
}

// (b.cardillo 2006-05-19 14:35) - PLID 20683 - Use the custom field name caching, since this 
// function is called every time the tab is refreshed, even though the results are the same 
// regardless of what patient you're on.  Using the caching makes it so subsequent calls are 
// practically instantaneous and require no data interaction.
void CCustom1Dlg::SetAllLabels()
{
	try {
		UpdateLabel(IDC_CUSTOM_CHECK1_LABEL, GetCustomFieldName(41));
		UpdateLabel(IDC_CUSTOM_CHECK2_LABEL, GetCustomFieldName(42));
		UpdateLabel(IDC_CUSTOM_CHECK3_LABEL, GetCustomFieldName(43));
		UpdateLabel(IDC_CUSTOM_CHECK4_LABEL, GetCustomFieldName(44));
		UpdateLabel(IDC_CUSTOM_CHECK5_LABEL, GetCustomFieldName(45));
		UpdateLabel(IDC_CUSTOM_CHECK6_LABEL, GetCustomFieldName(46));
		UpdateLabel(IDC_CUSTOM_LIST1_LABEL, GetCustomFieldName(21));
		UpdateLabel(IDC_CUSTOM_LIST2_LABEL, GetCustomFieldName(22));
		UpdateLabel(IDC_CUSTOM_LIST3_LABEL, GetCustomFieldName(23));
		UpdateLabel(IDC_CUSTOM_LIST4_LABEL, GetCustomFieldName(24));
		UpdateLabel(IDC_CUSTOM_LIST5_LABEL, GetCustomFieldName(25));
		UpdateLabel(IDC_CUSTOM_LIST6_LABEL, GetCustomFieldName(26));
		UpdateLabel(IDC_CUSTOM_DATE1_LABEL, GetCustomFieldName(51));
		UpdateLabel(IDC_CUSTOM_DATE2_LABEL, GetCustomFieldName(52));
		UpdateLabel(IDC_CUSTOM_DATE3_LABEL, GetCustomFieldName(53));
		UpdateLabel(IDC_CUSTOM_DATE4_LABEL, GetCustomFieldName(54));
		UpdateLabel(IDC_CUSTOM_TEXT1_LABEL, GetCustomFieldName(11));
		UpdateLabel(IDC_CUSTOM_TEXT2_LABEL, GetCustomFieldName(12));
		UpdateLabel(IDC_CUSTOM_TEXT3_LABEL, GetCustomFieldName(13));
		UpdateLabel(IDC_CUSTOM_TEXT4_LABEL, GetCustomFieldName(14));
		UpdateLabel(IDC_CUSTOM_TEXT5_LABEL, GetCustomFieldName(15));
		UpdateLabel(IDC_CUSTOM_TEXT6_LABEL, GetCustomFieldName(16));
		UpdateLabel(IDC_CUSTOM_TEXT7_LABEL, GetCustomFieldName(90));  // (a.walling 2007-07-03 08:59) - PLID 15491 - Added more custom text fields
		UpdateLabel(IDC_CUSTOM_TEXT8_LABEL, GetCustomFieldName(91));
		UpdateLabel(IDC_CUSTOM_TEXT9_LABEL, GetCustomFieldName(92));
		UpdateLabel(IDC_CUSTOM_TEXT10_LABEL, GetCustomFieldName(93));
		UpdateLabel(IDC_CUSTOM_TEXT11_LABEL, GetCustomFieldName(94));
		UpdateLabel(IDC_CUSTOM_TEXT12_LABEL, GetCustomFieldName(95));
		UpdateLabel(IDC_CUSTOM_MEMO1_LABEL, GetCustomFieldName(17));
		UpdateLabel(IDC_CUSTOM_CONTACT1_LABEL, GetCustomFieldName(31));
	}
	NxCatchAll("Could not load labels.");
}

// (j.armen 2011-06-21 17:45) - PLID 11490 - Display the custom lists
void CCustom1Dlg::SetCustomListDisplay(ADODB::_RecordsetPtr rs, NXDATALISTLib::_DNxDataListPtr customList, UINT customListIDC, CNxLabel *customListLabel)
{
	// (j.armen 2011-06-21 17:45) - PLID 11490 - If count > 1 then show the label
	if(AdoFldLong(rs, "ItemCount", 0) > 1)
	{
		ShowDlgItem(customListIDC, SW_HIDE);
		if(customListLabel->GetText().IsEmpty())
		{
			customListLabel->SetText(AdoFldString(rs, "TextParam"));
			customListLabel->SetToolTip(AdoFldString(rs, "TextParam"));
		}
		else
		{
			customListLabel->SetText(customListLabel->GetText() + ", " + AdoFldString(rs, "TextParam"));
			customListLabel->SetToolTip(customListLabel->GetToolTip() + ", " + AdoFldString(rs, "TextParam"));
		}

		customListLabel->SetSingleLine();
		customListLabel->SetType(dtsHyperlink);
		customListLabel->ShowWindow(SW_SHOWNA);
	}
	// (j.armen 2011-06-21 17:46) - PLID 11490 - else show the combo
	else
	{
		customListLabel->ShowWindow(SW_HIDE);
		customList->SetSelByColumn(0, rs->Fields->Item["IntParam"]->Value);
		ShowDlgItem(customListIDC, SW_SHOWNA);
	}
}
//JJ - again, a faster way than calling RestoreBox() several times
void CCustom1Dlg::RestoreAllBoxes()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		CString sql;
		int FieldID;
		_variant_t var;
		ClearBoxes();
		// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
		rs = CreateParamRecordset(
			"SELECT PersonID, FieldID, IntParam, TextParam, DateParam, NULL AS ItemCount FROM CustomFieldDataT WHERE PersonID = {INT} "
			// (j.armen 2011-06-21 17:46) - PLID 11490 - Added record for the custom list data.
			"UNION "
			"SELECT "
				"CustomListDataT.PersonID, "
				"CustomListDataT.FieldID, "
				"CustomListDataT.CustomListItemsID AS IntParam, "
				"CustomListItemsT.Text AS TextParam, "
				"NULL AS DateParam, "
				"ItemCount \r\n"
			"FROM CustomListDataT \r\n"
			"INNER JOIN (SELECT PersonID, FieldID, COUNT(*) AS ItemCount FROM CustomListDataT GROUP BY PersonID, FieldID) CountQ \r\n"
			"	ON CustomListDataT.PersonID = CountQ.PersonID AND CustomListDataT.FieldID = CountQ.FieldID \r\n"
			"INNER JOIN CustomListItemsT ON CustomListDataT.CustomListItemsID = CustomListItemsT.ID \r\n"
			//TES 1/12/2012 - PLID 47516 - SQL 2000 treats the ORDER BY clause as applying to the whole query, so it throws an exception
			// because CustomListItemsT isn't in the first UNION.
			"WHERE CustomListDataT.PersonID = {INT} ORDER BY /*CustomListItemsT.*/TextParam;",m_id, m_id);
		while(!rs->eof) 
		{
			FieldID = rs->Fields->Item["FieldID"]->Value.lVal;
			switch(FieldID) {
			case 11:
				SetSafeDlgText(IDC_CUSTOM_TEXT1, AdoFldString(rs, "TextParam", ""));
				break;
			case 12:
				SetSafeDlgText(IDC_CUSTOM_TEXT2, AdoFldString(rs, "TextParam", ""));
				break;
			case 13:
				SetSafeDlgText(IDC_CUSTOM_TEXT3, AdoFldString(rs, "TextParam", ""));
				break;
			case 14:
				SetSafeDlgText(IDC_CUSTOM_TEXT4, AdoFldString(rs, "TextParam", ""));
				break;
			case 15:
				SetSafeDlgText(IDC_CUSTOM_TEXT5, AdoFldString(rs, "TextParam", ""));
				break;
			case 16:
				SetSafeDlgText(IDC_CUSTOM_TEXT6, AdoFldString(rs, "TextParam", ""));
				break;
			case 90: // (a.walling 2007-07-03 09:02) - PLID 15491 - Added more custom text fields
				SetSafeDlgText(IDC_CUSTOM_TEXT7, AdoFldString(rs, "TextParam", ""));
				break;
			case 91:
				SetSafeDlgText(IDC_CUSTOM_TEXT8, AdoFldString(rs, "TextParam", ""));
				break;
			case 92:
				SetSafeDlgText(IDC_CUSTOM_TEXT9, AdoFldString(rs, "TextParam", ""));
				break;
			case 93:
				SetSafeDlgText(IDC_CUSTOM_TEXT10, AdoFldString(rs, "TextParam", ""));
				break;
			case 94:
				SetSafeDlgText(IDC_CUSTOM_TEXT11, AdoFldString(rs, "TextParam", ""));
				break;
			case 95:
				SetSafeDlgText(IDC_CUSTOM_TEXT12, AdoFldString(rs, "TextParam", ""));
				break;
			case 17:
				SetSafeDlgText(IDC_CUSTOM_NOTE, AdoFldString(rs, "TextParam", ""));
				break;
			case 51:
				if (rs->Fields->Item["DateParam"]->Value.vt != VT_NULL)
				{
					m_dtDate1 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate1->SetDateTime(m_dtDate1);
				}
				else
				{
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate1 = g_cdtInvalid;
					m_nxtDate1->Clear();
				}
				break;
			case 52:
				if (rs->Fields->Item["DateParam"]->Value.vt != VT_NULL)
				{
					m_dtDate2 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate2->SetDateTime(m_dtDate2);
				}
				else
				{
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate2 = g_cdtInvalid;
					m_nxtDate2->Clear();
				}
				break;
			case 53:
				if (rs->Fields->Item["DateParam"]->Value.vt != VT_NULL)
				{
					m_dtDate3 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate3->SetDateTime(m_dtDate3);
				}
				else
				{
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate3 = g_cdtInvalid;
					m_nxtDate3->Clear();
				}
				break;
			case 54:
				if (rs->Fields->Item["DateParam"]->Value.vt != VT_NULL)
				{
					m_dtDate4 = AdoFldDateTime(rs, "DateParam");
					m_nxtDate4->SetDateTime(m_dtDate4);
				}
				else
				{
					// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
					m_dtDate4 = g_cdtInvalid;
					m_nxtDate4->Clear();
				}
				break;
			case 41:
				var = rs->Fields->Item["IntParam"]->Value;
				if(var.vt == VT_NULL)
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK1))->SetCheck(FALSE);
				else
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK1))->SetCheck(var.lVal);
				break;
			case 42:
				var = rs->Fields->Item["IntParam"]->Value;
				if(var.vt == VT_NULL)
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK2))->SetCheck(FALSE);
				else
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK2))->SetCheck(var.lVal);
				break;
			case 43:
				var = rs->Fields->Item["IntParam"]->Value;
				if(var.vt == VT_NULL)
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK3))->SetCheck(FALSE);
				else
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK3))->SetCheck(var.lVal);
				break;
			case 44:
				var = rs->Fields->Item["IntParam"]->Value;
				if(var.vt == VT_NULL)
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK4))->SetCheck(FALSE);
				else
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK4))->SetCheck(var.lVal);
				break;
			case 45:
				var = rs->Fields->Item["IntParam"]->Value;
				if(var.vt == VT_NULL)
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK5))->SetCheck(FALSE);
				else
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK5))->SetCheck(var.lVal);
				break;
			case 46:
				var = rs->Fields->Item["IntParam"]->Value;
				if(var.vt == VT_NULL)
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK6))->SetCheck(FALSE);
				else
					((CButton*)GetDlgItem(IDC_CUSTOM_CHECK6))->SetCheck(var.lVal);
				break;
			// (j.armen 2011-06-21 17:47) - PLID 11490 - Custom lists now call SetCustomListDisplay to determine the display format
			case 21:
				SetCustomListDisplay(rs, m_CustomList1, IDC_CUSTOM_LIST1, &m_nxlCustomMultiList1Label);
				break;
			case 22:
				SetCustomListDisplay(rs, m_CustomList2, IDC_CUSTOM_LIST2, &m_nxlCustomMultiList2Label);
				break;
			case 23:
				SetCustomListDisplay(rs, m_CustomList3, IDC_CUSTOM_LIST3, &m_nxlCustomMultiList3Label);
				break;
			case 24:
				SetCustomListDisplay(rs, m_CustomList4, IDC_CUSTOM_LIST4, &m_nxlCustomMultiList4Label);
				break;
			case 25:
				SetCustomListDisplay(rs, m_CustomList5, IDC_CUSTOM_LIST5, &m_nxlCustomMultiList5Label);
				break;
			case 26:
				SetCustomListDisplay(rs, m_CustomList6, IDC_CUSTOM_LIST6, &m_nxlCustomMultiList6Label);
				break;
			case 31:
				//(e.lally 2005-11-28) PLID 18153 - add ability to make contacts inactive
				// (j.jones 2008-05-22 09:21) - PLID 27100 - accounted for NULLs
				long nIntParam = AdoFldLong(rs, "IntParam", -1);
				if(nIntParam == -1) {
					m_CustomContact1->PutCurSel(-1);
				}
				else {
					if(m_CustomContact1->TrySetSelByColumn(0, nIntParam) == -1){
						m_CustomContact1->PutCurSel(-1);
						// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
						_RecordsetPtr rsCon = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name "
							"FROM PersonT WHERE ID = {INT}",nIntParam);
						if(!rsCon->eof) {
							m_CustomContact1->PutComboBoxText(_bstr_t(AdoFldString(rsCon, "Name")));
						}
					}
				}
				break;
			}
			rs->MoveNext();
		}
		m_changed = false;
		m_ForceRefresh = false;
	}
	NxCatchAll("Could not set box. Custom1Dlg::RestoreAllBoxes()");
}

void CCustom1Dlg::ClearBoxes()
{
	SetSafeDlgText(IDC_CUSTOM_TEXT1,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT2,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT3,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT4,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT5,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT6,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT7,""); // (a.walling 2007-07-03 09:03) - PLID 15491 - Added more custom text fields
	SetSafeDlgText(IDC_CUSTOM_TEXT8,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT9,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT10,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT11,"");
	SetSafeDlgText(IDC_CUSTOM_TEXT12,"");
	SetSafeDlgText(IDC_CUSTOM_NOTE,"");
	m_nxtDate1->Clear();
	// (a.walling 2010-10-12 18:26) - PLID 41315 - These were setting m_dt to a non-0 value, therefore causing a DELETE query since it wouldn't match the blank NxTime control values
	m_dtDate1 = g_cdtInvalid;
	m_nxtDate2->Clear();
	m_dtDate2 = g_cdtInvalid;
	m_nxtDate3->Clear();
	m_dtDate3 = g_cdtInvalid;
	m_nxtDate4->Clear();
	m_dtDate4 = g_cdtInvalid;
	((CButton*)GetDlgItem(IDC_CUSTOM_CHECK1))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CUSTOM_CHECK2))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CUSTOM_CHECK3))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CUSTOM_CHECK4))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CUSTOM_CHECK5))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_CUSTOM_CHECK6))->SetCheck(0);
	m_CustomList1->PutCurSel(-1);
	m_CustomList2->PutCurSel(-1);
	m_CustomList3->PutCurSel(-1);
	m_CustomList4->PutCurSel(-1);
	m_CustomList5->PutCurSel(-1);
	m_CustomList6->PutCurSel(-1);
	// (j.armen 2011-06-21 17:48) - PLID 11490 - reset labels to hidden and blank before reloading
	m_nxlCustomMultiList1Label.SetText("");
	m_nxlCustomMultiList2Label.SetText("");
	m_nxlCustomMultiList3Label.SetText("");
	m_nxlCustomMultiList4Label.SetText("");
	m_nxlCustomMultiList5Label.SetText("");
	m_nxlCustomMultiList6Label.SetText("");
	m_nxlCustomMultiList1Label.ShowWindow(SW_HIDE);
	m_nxlCustomMultiList2Label.ShowWindow(SW_HIDE);
	m_nxlCustomMultiList3Label.ShowWindow(SW_HIDE);
	m_nxlCustomMultiList4Label.ShowWindow(SW_HIDE);
	m_nxlCustomMultiList5Label.ShowWindow(SW_HIDE);
	m_nxlCustomMultiList6Label.ShowWindow(SW_HIDE);
	// (j.armen 2011-11-29 13:14) - PLID 44253 - set m_changed to false here as it is set true by SetSafeDlgText
	// ShowDlgItem will cause an item to loose focus which will trigger a save to data when the boxes are empty
	m_changed = false;
	ShowDlgItem(IDC_CUSTOM_LIST1, SW_SHOWNA);
	ShowDlgItem(IDC_CUSTOM_LIST2, SW_SHOWNA);
	ShowDlgItem(IDC_CUSTOM_LIST3, SW_SHOWNA);
	ShowDlgItem(IDC_CUSTOM_LIST4, SW_SHOWNA);
	ShowDlgItem(IDC_CUSTOM_LIST5, SW_SHOWNA);
	ShowDlgItem(IDC_CUSTOM_LIST6, SW_SHOWNA);
	m_CustomContact1->PutCurSel(-1);
}

// (j.armen 2011-06-21 17:49) - PLID 11490 - Handle clicking labels
LRESULT CCustom1Dlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) 
		{
		// (j.armen 2011-06-21 17:50) - PLID 11490 - StoreCustomList called from a label
		case IDC_CUSTOM_MULTI_LIST1:	
			StoreCustomList(m_CustomList1, 21, IDC_CUSTOM_MULTI_LIST1);
			break;
		case IDC_CUSTOM_MULTI_LIST2:
			StoreCustomList(m_CustomList2, 22, IDC_CUSTOM_MULTI_LIST2);
			break;
		case IDC_CUSTOM_MULTI_LIST3:
			StoreCustomList(m_CustomList3, 23, IDC_CUSTOM_MULTI_LIST3);
			break;
		case IDC_CUSTOM_MULTI_LIST4:
			StoreCustomList(m_CustomList4, 24, IDC_CUSTOM_MULTI_LIST4);
			break;
		case IDC_CUSTOM_MULTI_LIST5:
			StoreCustomList(m_CustomList5, 25, IDC_CUSTOM_MULTI_LIST5);
			break;
		case IDC_CUSTOM_MULTI_LIST6:
			StoreCustomList(m_CustomList6, 26, IDC_CUSTOM_MULTI_LIST6);
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

void CCustom1Dlg::EditCustomList(_DNxDataListPtr &list, long listID) 
{
	//BVB - this crashed without a selection, 
	//also duplicating this code 6 times is not acceptable

	//_bstr_t			value;
	//long			curSel;

	// (j.armen 2011-06-22 10:54) - PLID 11490 - refreshing the boxes later on, so no need to remember the sel
	//save the current value
	//curSel = list->CurSel;
	//if (curSel != -1)
	//	value = list->Value[curSel][1];

	// CH 3/1: Even though it doesnt LOOK like you added
	// it to the list with this conditional, it still doesnt work!!
	// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
	if (IDOK == CEditComboBox(this, listID, list, "Edit Combo Box").DoModal())
	{
		IRowSettingsPtr pRow;
		pRow = list->GetRow(-1);
		_variant_t var;
		var.vt = VT_NULL;
		// (j.armen 2011-06-21 17:50) - Changed no item selection value to -1
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<No Item Selected>");
		list->InsertRow(pRow,0);

		// (j.armen 2011-06-21 17:51) - added row for multiple items
		pRow = list->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1,"<Multiple Items>");
		list->InsertRow(pRow, 0);

		// (j.armen 2011-06-22 10:54) - PLID 11490 - refreshing the boxes later on, so no need to remember the sel
		//try and set the combo to the old value
		//if (curSel != -1)
		//	list->SetSelByColumn(1, value);

		switch (listID)
		{
		case 21: CClient::RefreshTable(NetUtils::CustomList1); break;
		case 22: CClient::RefreshTable(NetUtils::CustomList2); break;
		case 23: CClient::RefreshTable(NetUtils::CustomList3); break;
		case 24: CClient::RefreshTable(NetUtils::CustomList4); break;
		case 25: CClient::RefreshTable(NetUtils::CustomList5); break;
		case 26: CClient::RefreshTable(NetUtils::CustomList6); break;
		}
	}
	else {
		// (j.armen 2011-06-21 17:51) - PLID 11490 - this was a pre-existing bug.
		// When you cancel the dlg, it still needs to add these rows.
		IRowSettingsPtr pRow;
		pRow = list->GetRow(-1);
		_variant_t var;
		var.vt = VT_NULL;
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<No Item Selected>");
		list->InsertRow(pRow,0);

		pRow = list->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1,"<Multiple Items>");
		list->InsertRow(pRow, 0);

		//DRT 7/25/02
		//if we cancel the dialog, it requeries the list (because changes are made whether you hit ok or cancel)
		//so the list will have no selection.
		// (j.armen 2011-06-22 10:53) - PLID 11490 The lists were not being updated correctly.  
		//Now restore the boxes which will in the end set the sel
		//list->SetSelByColumn(1, value);

	}

	// (j.armen 2011-06-22 10:55) - PLID 11490 - Everything should be set now.  Go ahead and refresh the boxes just to be safe.
	RestoreAllBoxes();
}

void CCustom1Dlg::OnEditCustomList1() 
{
	EditCustomList(m_CustomList1, 21);
}

void CCustom1Dlg::OnEditCustomList2()
{
	EditCustomList(m_CustomList2, 22);
}

void CCustom1Dlg::OnEditCustomList3()
{
	EditCustomList(m_CustomList3, 23);
}

void CCustom1Dlg::OnEditCustomList4()
{
	EditCustomList(m_CustomList4, 24);
}

void CCustom1Dlg::OnEditCustomList5()
{
	EditCustomList(m_CustomList5, 25);
}

void CCustom1Dlg::OnEditCustomList6()
{
	EditCustomList(m_CustomList6, 26);
}

BOOL CCustom1Dlg::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message) {
	case WM_LBUTTONDBLCLK:
		CallModalDialog(::GetDlgCtrlID(pMsg->hwnd));
		return TRUE;
		break;
	default:
		break;
	}
		
	return CNxDialog::PreTranslateMessage(pMsg);
}

void CCustom1Dlg::OnSelChosenCustomList1(long nRow) 
{
	StoreBox(IDC_CUSTOM_LIST1);
}

void CCustom1Dlg::OnSelChosenCustomList2(long nRow) 
{
	StoreBox(IDC_CUSTOM_LIST2);
}

void CCustom1Dlg::OnSelChosenCustomList3(long nRow) 
{
	StoreBox(IDC_CUSTOM_LIST3);
}

void CCustom1Dlg::OnSelChosenCustomList4(long nRow) 
{
	StoreBox(IDC_CUSTOM_LIST4);
}

void CCustom1Dlg::OnSelChosenCustomList5(long nRow) 
{
	StoreBox(IDC_CUSTOM_LIST5);
}

void CCustom1Dlg::OnSelChosenCustomList6(long nRow) 
{
	StoreBox(IDC_CUSTOM_LIST6);
}

void CCustom1Dlg::OnSelChosenCustomContact1(long nRow) 
{
	StoreBox(IDC_CUSTOM_CONTACT1);
}

void CCustom1Dlg::SecureControls()
{
	CWnd* pWnd;
	int i;

	// Return if we have write access
	if (GetCurrentUserPermissions(bioPatient) & SPT___W_______)
		return;

	// No write access. Traverse the controls to disable all edit boxes
	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT)) 
	{
		if (IsEditBox(pWnd))
		{
			((CNxEdit*)pWnd)->SetReadOnly(TRUE);
		}
	}

	// Disable the dates
	GetDlgItem(IDC_CUSTOM_DATE1)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_DATE2)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_DATE3)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_DATE4)->EnableWindow(FALSE);

	// Disable the dropdowns
	GetDlgItem(IDC_CUSTOM_LIST1)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_LIST2)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_LIST3)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_LIST4)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_LIST5)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_LIST6)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_CONTACT1)->EnableWindow(FALSE);

	// Disable checkboxes
	GetDlgItem(IDC_CUSTOM_CHECK1)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_CHECK2)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_CHECK3)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_CHECK4)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_CHECK5)->EnableWindow(FALSE);
	GetDlgItem(IDC_CUSTOM_CHECK6)->EnableWindow(FALSE);
}

BOOL CCustom1Dlg::IsEditBox(CWnd* pWnd)
{
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_CUSTOM_TEXT1:
	case IDC_CUSTOM_TEXT2:
	case IDC_CUSTOM_TEXT3:
	case IDC_CUSTOM_TEXT4:
	case IDC_CUSTOM_TEXT5:
	case IDC_CUSTOM_TEXT6:
	case IDC_CUSTOM_TEXT7: // (a.walling 2007-07-03 09:04) - PLID 15491 - Added more custom text fields
	case IDC_CUSTOM_TEXT8:
	case IDC_CUSTOM_TEXT9:
	case IDC_CUSTOM_TEXT10:
	case IDC_CUSTOM_TEXT11:
	case IDC_CUSTOM_TEXT12:
	case IDC_CUSTOM_NOTE:
	case IDC_CUSTOM_DATE1:
	case IDC_CUSTOM_DATE2:
	case IDC_CUSTOM_DATE3:
	case IDC_CUSTOM_DATE4:
		return TRUE;
	}
	return FALSE;
}

void CCustom1Dlg::OnKillFocusCustomDate1() 
{
	StoreBox(IDC_CUSTOM_DATE1);	
}

void CCustom1Dlg::OnKillFocusCustomDate2() 
{
	StoreBox(IDC_CUSTOM_DATE2);
}

void CCustom1Dlg::OnKillFocusCustomDate3() 
{
	StoreBox(IDC_CUSTOM_DATE3);
}

void CCustom1Dlg::OnKillFocusCustomDate4() 
{
	StoreBox(IDC_CUSTOM_DATE4);
}

void CCustom1Dlg::OnGotoCustomContact() 
{
	try {
		//make sure we have a valid selection
		long nCurSel = m_CustomContact1->GetCurSel();
		long nID = -1;

		//(e.lally 2005-11-28) PLID 18153 - add the ability to make contacts inactive
		if(nCurSel == -1){
			//The contact might be inactive
			// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT IntParam FROM CustomFieldDataT "
				"WHERE PersonID = {INT} AND FieldID = {INT}", m_id, 31);
			if(rs->eof){
				//There is no contact so just return
				return;
			}
			else{
				// (j.jones 2008-05-22 09:21) - PLID 27100 - accounted for NULLs
				nID = AdoFldLong(rs, "IntParam", -1);
			}
		}
		else{
			//get the id
			nID = VarLong(m_CustomContact1->GetValue(nCurSel, 0), -1);
		}

		if(nID == -1) {
			return;
		}

		//do the flipping
		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if(p->FlipToModule(CONTACT_MODULE_NAME)) {

			if (nID != GetActiveContactID())
			{
				p->m_contactToolBar.SetActiveContactID(nID);
			}

			pView = (CNxTabView *)p->GetOpenView(CONTACT_MODULE_NAME);
			if (pView) 
			{	if(pView->GetActiveTab()==0)
					pView->UpdateView();
				else
					pView->SetActiveTab(0);
			}

			// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
			if(IsNexTechInternal()) {
				((CContactView*)GetMainFrame()->GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
			}
		}

	} NxCatchAll("Error in CCustom1Dlg::OnGotoCustomContact()");

}

void CCustom1Dlg::OnRequeryFinishedCustomContact1(short nFlags) 
{
	IRowSettingsPtr pRow = m_CustomContact1->GetRow(-1);
	pRow->PutValue(0,(long)0);
	pRow->PutValue(1,"<No Contact Selected>");
	pRow->PutValue(2,"");
	pRow->PutValue(3,"");
	pRow->PutValue(4,"");
	pRow->PutValue(5,"");
	m_CustomContact1->InsertRow(pRow, 0);
}

void CCustom1Dlg::OnTrySetSelFinishedCustomContact1(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		//must be inactive
		// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT IntParam AS Val, Last + ', ' + First + ' ' + Middle AS Name FROM CustomFieldDataT "
			"INNER JOIN PersonT ON CustomFieldDataT.IntParam = PersonT.ID "
			"WHERE CustomFieldDataT.PersonID = {INT} AND FieldID = {INT}",m_id,31);
		if(!rs->eof) {
			m_CustomContact1->PutCurSel(-1);
			m_CustomContact1->PutComboBoxText(_bstr_t(AdoFldString(rs, "Name")));
		}
		rs->Close();
	}
}

BOOL CCustom1Dlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		// (j.armen 2011-06-22 12:22) - PLID 11490 - if our labels are visiable and enabled with mouse over, show link cursor
		if(m_nxlCustomMultiList1Label.IsWindowVisible() && m_nxlCustomMultiList1Label.IsWindowEnabled())
		{
			m_nxlCustomMultiList1Label.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		if(m_nxlCustomMultiList2Label.IsWindowVisible() && m_nxlCustomMultiList2Label.IsWindowEnabled())
		{
			m_nxlCustomMultiList2Label.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		if(m_nxlCustomMultiList3Label.IsWindowVisible() && m_nxlCustomMultiList3Label.IsWindowEnabled())
		{
			m_nxlCustomMultiList3Label.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		if(m_nxlCustomMultiList4Label.IsWindowVisible() && m_nxlCustomMultiList4Label.IsWindowEnabled())
		{
			m_nxlCustomMultiList4Label.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		if(m_nxlCustomMultiList5Label.IsWindowVisible() && m_nxlCustomMultiList5Label.IsWindowEnabled())
		{
			m_nxlCustomMultiList5Label.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		if(m_nxlCustomMultiList6Label.IsWindowVisible() && m_nxlCustomMultiList6Label.IsWindowEnabled())
		{
			m_nxlCustomMultiList6Label.GetWindowRect(rc);
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
