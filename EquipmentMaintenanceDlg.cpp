// EquipmentMaintenanceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GlobalFinancialUtils.h"
#include "EquipmentMaintenanceDlg.h"
#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;


// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_DELETE_MAINT_ITEM	38010

#define COLUMN_ID		0
#define	COLUMN_DATE		1
#define	COLUMN_COST		2
#define COLUMN_NOTES	3

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEquipmentMaintenanceDlg dialog


CEquipmentMaintenanceDlg::CEquipmentMaintenanceDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEquipmentMaintenanceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEquipmentMaintenanceDlg)
		m_ProductID = -1;
	//}}AFX_DATA_INIT
}


void CEquipmentMaintenanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEquipmentMaintenanceDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_COST, m_nxeditCost);
	DDX_Control(pDX, IDC_MAINT_INTERVAL, m_nxeditMaintInterval);
	DDX_Control(pDX, IDC_EQUIPMENT_NAME, m_nxstaticEquipmentName);
	DDX_Control(pDX, IDC_ADD_NEW_MAINT_ENTRY, m_btnAddNewMaintEntry);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEquipmentMaintenanceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEquipmentMaintenanceDlg)
	ON_BN_CLICKED(IDC_ADD_NEW_MAINT_ENTRY, OnAddNewMaintEntry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEquipmentMaintenanceDlg message handlers

BOOL CEquipmentMaintenanceDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:40) - PLID 29820 - NxIconified buttons
		m_btnAddNewMaintEntry.AutoSet(NXB_NEW);
		m_btnOK.AutoSet(NXB_CLOSE);

		m_MaintList = BindNxDataListCtrl(this,IDC_MAINTENANCE_LIST,GetRemoteData(),false);
		CString str;
		str.Format("ProductID = %li",m_ProductID);
		m_MaintList->PutWhereClause(_bstr_t(str));
		m_MaintList->Requery();

		m_IntervalCombo = BindNxDataListCtrl(this,IDC_MAINT_INTERVAL_TYPE,GetRemoteData(),false);

		IRowSettingsPtr pRow = m_IntervalCombo->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("Days"));
		m_IntervalCombo->AddRow(pRow);

		pRow = m_IntervalCombo->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Weeks"));
		m_IntervalCombo->AddRow(pRow);

		pRow = m_IntervalCombo->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,_bstr_t("Months"));
		m_IntervalCombo->AddRow(pRow);

		pRow = m_IntervalCombo->GetRow(-1);
		pRow->PutValue(0,(long)4);
		pRow->PutValue(1,_bstr_t("Years"));
		m_IntervalCombo->AddRow(pRow);

		m_IntervalCombo->CurSel = 2;

		_RecordsetPtr rs = CreateRecordset("SELECT Name, EquipmentCost, MaintInterval, MaintIntervalType FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID WHERE ProductT.ID = %li",m_ProductID);
		if(!rs->eof) {
			SetDlgItemText(IDC_EQUIPMENT_NAME,AdoFldString(rs, "Name",""));
			_variant_t var = rs->Fields->Item["EquipmentCost"]->Value;
			if(var.vt == VT_CY)
				SetDlgItemText(IDC_COST,FormatCurrencyForInterface(var.cyVal));
			var = rs->Fields->Item["MaintInterval"]->Value;
			if(var.vt == VT_I4)
				SetDlgItemInt(IDC_MAINT_INTERVAL,var.lVal);
			var = rs->Fields->Item["MaintIntervalType"]->Value;
			if(var.vt == VT_I4)
				m_IntervalCombo->CurSel = var.lVal - 1;
		}
		rs->Close();
	}
	NxCatchAll("Error in CEquipmentMaintenanceDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEquipmentMaintenanceDlg::OnOK() 
{
	try {

		CString strCost;
		GetDlgItemText(IDC_COST,strCost);

		if(strCost.GetLength() == 0)
			strCost = "NULL";
		else {

			COleCurrency cy = ParseCurrencyFromInterface(strCost);
			if(cy.GetStatus() == COleCurrency::invalid) {
				MsgBox("Please enter a valid amount in the 'Cost Of Equipment' box.");
				return;
			}

			strCost.Format("Convert(money,'%s')",_Q(FormatCurrencyForSql(cy)));
			SetDlgItemText(IDC_COST, FormatCurrencyForInterface(cy));
		}

		long MaintInterval = GetDlgItemInt(IDC_MAINT_INTERVAL);
		
		long IntervalType = m_IntervalCombo->GetCurSel() + 1;

		//TES 7/8/03: Put a WHERE clause in this query!!!
		ExecuteSql("UPDATE ProductT SET EquipmentCost = %s, MaintInterval = %li, MaintIntervalType = %li "
			"WHERE ID = %li",
			strCost,MaintInterval,IntervalType, m_ProductID);

	}NxCatchAll("Error saving equipment maintenance information.");
	
	CDialog::OnOK();
}

void CEquipmentMaintenanceDlg::OnAddNewMaintEntry() 
{
	try {

		long NewID;

		ExecuteSql("INSERT INTO EquipmentMaintenanceT (ID, ProductID, Date, Cost, Notes) VALUES (%li, %li, GetDate(), Convert(money,'$0.00'), '')",
			NewID = NewNumber("EquipmentMaintenanceT","ID"),m_ProductID);
		
		IRowSettingsPtr pRow;
		pRow = m_MaintList->GetRow(-1);
		pRow->PutValue(COLUMN_ID, NewID);
		COleDateTime dt;
		dt = COleDateTime::GetCurrentTime();
		dt.SetDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
		pRow->PutValue(COLUMN_DATE, _variant_t(dt, VT_DATE));
		pRow->PutValue(COLUMN_COST, _bstr_t(FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE)));
		pRow->PutValue(COLUMN_NOTES, _bstr_t(""));
		long nNewRow = m_MaintList->AddRow(pRow);
		if (nNewRow != -1) {
			m_MaintList->CurSel = nNewRow;
			m_MaintList->StartEditing(nNewRow, COLUMN_COST);
		}

	}NxCatchAll("Error adding new maintenance entry.");
}

BEGIN_EVENTSINK_MAP(CEquipmentMaintenanceDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEquipmentMaintenanceDlg)
	ON_EVENT(CEquipmentMaintenanceDlg, IDC_MAINTENANCE_LIST, 10 /* EditingFinished */, OnEditingFinishedMaintenanceList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEquipmentMaintenanceDlg, IDC_MAINTENANCE_LIST, 6 /* RButtonDown */, OnRButtonDownMaintenanceList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEquipmentMaintenanceDlg, IDC_MAINTENANCE_LIST, 9 /* EditingFinishing */, OnEditingFinishingMaintenanceList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEquipmentMaintenanceDlg::OnEditingFinishedMaintenanceList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow == -1)
		return;

	try {

		long ID = m_MaintList->GetValue(nRow,COLUMN_ID).lVal;
		
		switch(nCol) {
			case(COLUMN_DATE):
				if(varNewValue.vt == VT_DATE) {
					ExecuteSql("UPDATE EquipmentMaintenanceT SET Date = '%s' WHERE ID = %li",FormatDateTimeForSql(varNewValue.date),ID);
				}
				break;
			case(COLUMN_COST):
				if(varNewValue.vt == VT_CY) {
					CString strCost = FormatCurrencyForSql(varNewValue.cyVal);
					ExecuteSql("UPDATE EquipmentMaintenanceT SET Cost = Convert(money,'%s') WHERE ID = %li",_Q(strCost),ID);
				}
				break;
			case(COLUMN_NOTES):
				if(varNewValue.vt == VT_BSTR) {
					CString strNote = CString(varNewValue.bstrVal);
					if(strNote.GetLength() > 255) {
						AfxMessageBox("Your note is greater than the maximum length allowed (255).\n"
							"It will be truncated and saved.");
						strNote = strNote.Left(255);
						m_MaintList->PutValue(nRow,COLUMN_NOTES,_bstr_t(strNote));
					}
					ExecuteSql("UPDATE EquipmentMaintenanceT SET Notes = '%s' WHERE ID = %li",_Q(strNote),ID);
				}
				break;
		}

	}NxCatchAll("Error saving changes to maintenance record.");	
}

void CEquipmentMaintenanceDlg::OnRButtonDownMaintenanceList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_MaintList->CurSel = nRow;

	if(nRow == -1)
		return;

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();
	if(nRow != -1) {
		mnu.InsertMenu(0, MF_BYPOSITION, ID_DELETE_MAINT_ITEM, "Delete");
	}

	CPoint pt;
	GetCursorPos(&pt);
	mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, this, NULL);
}

BOOL CEquipmentMaintenanceDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
		case ID_DELETE_MAINT_ITEM:
			try {
				long CurSel = m_MaintList->GetCurSel();
				if(CurSel != -1 && IDYES == MessageBox("Are you sure you wish to permanently delete this record?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					long ID = m_MaintList->GetValue(CurSel,0).lVal;
					ExecuteSql("DELETE FROM EquipmentMaintenanceT WHERE ID = %li",ID);
					m_MaintList->RemoveRow(CurSel);
				}
			}NxCatchAll("Error deleting maintenance record.");
			break;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CEquipmentMaintenanceDlg::OnEditingFinishingMaintenanceList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if (nCol == COLUMN_DATE && pvarNewValue->vt == VT_DATE) {
		if(pvarNewValue->date==(COleDateTime::invalid) || pvarNewValue->date<=1.0) {
			*pbCommit = FALSE;
			*pbContinue = FALSE;
		}
	}
	
	if (pvarNewValue->vt == VT_CY) {
		COleCurrency cy = pvarNewValue->cyVal;
		RoundCurrency(cy);
		pvarNewValue->cyVal = cy;
	}
}
