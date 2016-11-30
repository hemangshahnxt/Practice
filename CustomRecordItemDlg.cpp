// CustomRecordItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "CustomRecordItemDlg.h"
#include "EMRTextDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CCustomRecordItemDlg dialog


CCustomRecordItemDlg::CCustomRecordItemDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCustomRecordItemDlg::IDD, (CNxView*)pParent)
{
	//{{AFX_DATA_INIT(CCustomRecordItemDlg)
		m_ID = -1;
		m_index = -1;
		m_nEmrInfoID = -1;
	//}}AFX_DATA_INIT
}


void CCustomRecordItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordItemDlg)
	DDX_Control(pDX, IDC_DATA_TEXTBOX, m_Text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomRecordItemDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustomRecordItemDlg)
	ON_BN_CLICKED(IDC_BTN_ZOOM_EMR_TEXT, OnBtnZoomEmrText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordItemDlg message handlers
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) 
{
	CWnd *pWnd = CWnd::FromHandle(hwndChild);
	int nID = pWnd->GetDlgCtrlID();
	return TRUE;
}

BOOL CCustomRecordItemDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		EnumChildWindows(GetSafeHwnd(), EnumChildProc, 0);

		m_InfoCombo = BindNxDataListCtrl(IDC_INFO_LIST,false);
		m_DataCombo = BindNxDataListCtrl(IDC_DATA_LIST,false);

		//when we start the dialog, we will show only they combos (the data list will be empty)
		m_Text.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_ZOOM_EMR_TEXT)->ShowWindow(SW_HIDE);		

		m_InfoCombo->PutWhereClause(_bstr_t(m_strInfoWhereClause));
		m_InfoCombo->Requery();

		//JJ - I do this now because if anything (such as the user editing EMR items)
		//blindly requeries the datacombo, I want there to be no data in it.
		CString str;
		str.Format("EMRInfoID = -1");
		m_DataCombo->PutWhereClause(_bstr_t(str));

		IRowSettingsPtr pRow = m_InfoCombo->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t("<Remove Item>"));
		pRow->PutValue(2,(long)-1);
		m_InfoCombo->AddRow(pRow);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CCustomRecordItemDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCustomRecordItemDlg)
	ON_EVENT(CCustomRecordItemDlg, IDC_INFO_LIST, 16 /* SelChosen */, OnSelChosenInfoList, VTS_I4)
	ON_EVENT(CCustomRecordItemDlg, IDC_DATA_LIST, 16 /* SelChosen */, OnSelChosenDataList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCustomRecordItemDlg::OnSelChosenInfoList(long nRow) 
{
	if(nRow==-1)
		return;

	CString str;

	try {

		m_nEmrInfoID = m_InfoCombo->GetValue(nRow,0).lVal;

		ChangeInfoItem(m_nEmrInfoID);

		//if a data list and not text
		if(m_InfoCombo->GetValue(nRow,2).iVal != 1) {

			long DataID = -1;
			if(m_DataCombo->GetCurSel() != -1)
				DataID = m_DataCombo->GetValue(m_DataCombo->GetCurSel(),0).lVal;
		
			if(DataID == -1) {
				//if DataID -1, see if a default ID exists
				_RecordsetPtr rs = CreateRecordset("SELECT EmrDataID AS DefaultData FROM EMRInfoDefaultsT "
					"WHERE EmrInfoID = %li",m_nEmrInfoID);
				if(!rs->eof) {
					_variant_t var = rs->Fields->Item["DefaultData"]->Value;
					if(var.vt == VT_I4)
						SetDefaultData(var.lVal);
				}
				rs->Close();
			}
		}

		if(m_nEmrInfoID == -1)
			m_InfoCombo->PutCurSel(-1);

		// Tell our parent that we just changed the selection
		GetParent()->PostMessage(NXM_EMR_ITEM_CHANGED, m_index, m_nEmrInfoID);

	}NxCatchAll("Error selecting info from list.");	
}

void CCustomRecordItemDlg::ChangeInfoItem(long InfoID)
{
	m_nEmrInfoID = InfoID;
	long nRow = m_InfoCombo->SetSelByColumn(0,m_nEmrInfoID);
	int datatype;
	if(nRow == -1) {
		//Sigh, we'll have to check the data.
		_RecordsetPtr rsDataType = CreateRecordset("SELECT Name, DataType FROM EmrInfoT WHERE ID = %li", InfoID);
		m_InfoCombo->PutComboBoxText((LPCTSTR)AdoFldString(rsDataType, "Name"));
		datatype = AdoFldByte(rsDataType, "DataType");
	}
	else {
		datatype = m_InfoCombo->GetValue(nRow,2).iVal;
	}

	if(datatype!=2) {
		//text, or some crazy level 2 type which we will just treat as text.
		GetDlgItem(IDC_DATA_LIST)->ShowWindow(SW_HIDE);
		m_Text.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_ZOOM_EMR_TEXT)->ShowWindow(SW_SHOW);
	}
	else {
		//list
		long DataID = -1;
		if(m_DataCombo->GetCurSel() != -1)
			DataID = m_DataCombo->GetValue(m_DataCombo->GetCurSel(),0).lVal;
		m_Text.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_ZOOM_EMR_TEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATA_LIST)->ShowWindow(SW_SHOW);
		CString str;
		str.Format("EMRInfoID = %li",InfoID);
		m_DataCombo->PutWhereClause(_bstr_t(str));
		m_DataCombo->Requery();

		if(DataID != -1)
			m_DataCombo->SetSelByColumn(0,DataID);
	}
}

void CCustomRecordItemDlg::LoadInfoID(long InfoID, BOOL bNewItem)
{
	if(bNewItem)
		OnSelChosenInfoList(m_InfoCombo->SetSelByColumn(0,(long)InfoID));
	else
		ChangeInfoItem(InfoID);
}

LRESULT CCustomRecordItemDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {

	case NXM_EMR_CHANGE_ITEM:
		{
			LoadInfoID((long)wParam, (long)lParam == 1 ? TRUE : FALSE);
		}
		break;

	case NXM_EMR_LOAD_DETAIL:
		{
			LoadDetailID((long)wParam, (long)lParam == 1 ? TRUE : FALSE);
		}
		break;
	case NXM_EMR_SET_DEFAULT_DATA:
		{
			SetDefaultData((long)wParam);
		}
		break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

void CCustomRecordItemDlg::OnBtnZoomEmrText() 
{
	CString text;
	GetDlgItemText(IDC_DATA_TEXTBOX,text);

	CEMRTextDlg dlg(this);
	dlg.m_text = text;
	dlg.DoModal();

	text = dlg.m_text;
	SetDlgItemText(IDC_DATA_TEXTBOX,text);
}

void CCustomRecordItemDlg::RefreshList(CString strNewInfoWhereClause)
{
	m_strInfoWhereClause = strNewInfoWhereClause;
	
	//first refresh the InfoCombo
	m_InfoCombo->PutWhereClause(_bstr_t(m_strInfoWhereClause));
	m_InfoCombo->Requery();

	IRowSettingsPtr pRow = m_InfoCombo->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,_bstr_t("<Remove Item>"));
	pRow->PutValue(2,(long)-1);
	m_InfoCombo->AddRow(pRow);

	if(m_nEmrInfoID != -1) {
		//handle the case where they change an info item from a text to a list or vice versa
		//JJ - do NOT call OnSelChosenInfoList, it sends messages that do not need to be sent in this case
		ChangeInfoItem(m_nEmrInfoID);
	}
}

int CCustomRecordItemDlg::GetItemInfo(long &InfoID, int &datatype, long &DataID, CString &text)
{
	try {
		InfoID = m_nEmrInfoID;

		//none selected
		if(m_nEmrInfoID == -1)
			return ITEM_NULL;

		if(m_InfoCombo->CurSel == -1) {
			//Sigh, we'll have to check the data.
			_RecordsetPtr rsDataType = CreateRecordset("SELECT DataType FROM EmrInfoT WHERE ID = %li", m_nEmrInfoID);
			datatype = AdoFldByte(rsDataType, "DataType");
		}
		else {
			datatype = m_InfoCombo->GetValue(m_InfoCombo->GetCurSel(),2).iVal;
		}

		if(datatype != 2) {
			//Force it to be text if it was some crazy level 2 type.
			datatype = 1;
			//text
			GetDlgItemText(IDC_DATA_TEXTBOX,text);
			DataID = -1;
		}
		else {
			//data
			if(m_DataCombo->CurSel==-1)
				return ITEM_EMPTY;

			DataID = m_DataCombo->GetValue(m_DataCombo->GetCurSel(),0).lVal;
			text = "";
		}

		return ITEM_OK;

	}NxCatchAll("Error saving EMR Item");

	return ITEM_ERROR;
}

void CCustomRecordItemDlg::LoadDetailID(long DetailID, BOOL bNewItem)
{
	try {

		_RecordsetPtr rs = CreateRecordset("SELECT DataType, EMRDetailsT.EMRInfoID, Text, EmrDataT.ID AS EMRDataID, EmrInfoT.Name AS InfoName "
			"FROM EMRDetailsT LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID "
			"LEFT JOIN EmrDataT ON EMRSelectT.EMRDataID = EmrDataT.ID "
			"WHERE EMRDetailsT.ID = %li",DetailID);
		if(!rs->eof) {
			_variant_t var = rs->Fields->Item["EMRInfoID"]->Value;
			int row = m_InfoCombo->SetSelByColumn(0,var);
			if(row == -1) {
				//Uh-oh!  Well, still display the name.
				m_InfoCombo->PutComboBoxText((LPCTSTR)AdoFldString(rs, "InfoName"));
			}

			if(bNewItem)
				OnSelChosenInfoList(row);
			else
				ChangeInfoItem(var.lVal);

			var = rs->Fields->Item["DataType"]->Value;
			int datatype;
			if(var.vt==VT_UI1) {
				datatype = var.bVal;
				if(datatype == 2) {
					var = rs->Fields->Item["EmrDataID"]->Value;
					m_DataCombo->TrySetSelByColumn(0,var);
				}
				else {
					var = rs->Fields->Item["Text"]->Value;
					m_Text.SetWindowText(VarString(var,""));
				}
			}
		}
		rs->Close();

		//set the ID to be the detail ID, to reflect that this is a pre-existing item
		m_ID = DetailID;

	}NxCatchAll("Error loading EMR detail.");
}

void CCustomRecordItemDlg::SetDefaultData(long DataID)
{
	OnSelChosenDataList(m_DataCombo->SetSelByColumn(0,(long)DataID));
}

void CCustomRecordItemDlg::OnSelChosenDataList(long nRow) 
{
	if(nRow == -1)
		return;

	try {

		long DataID = VarLong(m_DataCombo->GetValue(m_DataCombo->GetCurSel(),0));

		// Tell our parent that we just changed the selection
		GetParent()->PostMessage(NXM_EMR_DATA_CHANGED, (long)DataID);

	}NxCatchAll("Error processing data.");
}

