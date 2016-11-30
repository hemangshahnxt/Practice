// EditDefaultPOSCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditDefaultPOSCodesDlg.h"
#include "PlaceCodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditDefaultPOSCodesDlg dialog


CEditDefaultPOSCodesDlg::CEditDefaultPOSCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditDefaultPOSCodesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditDefaultPOSCodesDlg)
		m_HCFASetupGroupID = -1;
	//}}AFX_DATA_INIT
}


void CEditDefaultPOSCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDefaultPOSCodesDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditDefaultPOSCodesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditDefaultPOSCodesDlg)
	ON_BN_CLICKED(IDC_EDIT_POS_CODES, OnEditPosCodes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDefaultPOSCodesDlg message handlers

BEGIN_EVENTSINK_MAP(CEditDefaultPOSCodesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditDefaultPOSCodesDlg)
	ON_EVENT(CEditDefaultPOSCodesDlg, IDC_PLACEOFSERVICE_COMBO, 16 /* SelChosen */, OnSelChosenPlaceofserviceCombo, VTS_I4)
	ON_EVENT(CEditDefaultPOSCodesDlg, IDC_COMBO_LOCATIONS, 16 /* SelChosen */, OnSelChosenComboLocations, VTS_I4)
	ON_EVENT(CEditDefaultPOSCodesDlg, IDC_PLACEOFSERVICE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPlaceofserviceCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CEditDefaultPOSCodesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29852 - Set button style
	m_btnOk.AutoSet(NXB_CLOSE);
	m_btnOk.SetWindowText("Close");
	
	m_POSCombo = BindNxDataListCtrl(this,IDC_PLACEOFSERVICE_COMBO,GetRemoteData(),true);
	m_LocationCombo = BindNxDataListCtrl(this,IDC_COMBO_LOCATIONS,GetRemoteData(),true);

	m_LocationCombo->CurSel = 0;
	LoadPOSLink();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDefaultPOSCodesDlg::OnOK() 
{
	SavePOSLink();
	
	CNxDialog::OnOK();
}

void CEditDefaultPOSCodesDlg::OnSelChosenPlaceofserviceCombo(long nRow) 
{
	SavePOSLink();
}

void CEditDefaultPOSCodesDlg::OnSelChosenComboLocations(long nRow) 
{
	LoadPOSLink();
}

void CEditDefaultPOSCodesDlg::OnEditPosCodes() 
{
	try{
		_variant_t var;
		if(m_POSCombo->CurSel!=-1) {
			var = m_POSCombo->GetValue(m_POSCombo->GetCurSel(),0);
		}

		CPlaceCodeDlg dlg(this);
		dlg.DoModal();

		m_POSCombo->Requery();

		if(var.vt!=VT_NULL)
			m_POSCombo->SetSelByColumn(0,var);
		else
			//m.hancock - PLID 17834 - 10/28/05 - Set the selection
			m_POSCombo->CurSel = -1;

	}NxCatchAll("Error setting selection.");
}

void CEditDefaultPOSCodesDlg::LoadPOSLink()
{
	
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_LocationCombo->GetCurSel() == -1)
		return;

	try {

		str.Format("SELECT POSID FROM POSLocationLinkT WHERE HCFASetupGroupID = %li AND LocationID = %li",
			m_HCFASetupGroupID, VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0),-1));
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->GetItem("POSID")->Value;
			if (var.vt == VT_NULL)
				m_POSCombo->CurSel = -1;
			else
				m_POSCombo->SetSelByColumn(0,var);
		}
		else
			m_POSCombo->CurSel = -1;
		rs->Close();
	}
	NxCatchAll("Error in LoadPOSLink");
}

void CEditDefaultPOSCodesDlg::SavePOSLink()
{
	if(m_LocationCombo->CurSel==-1)
		return;

	if(m_POSCombo->CurSel == -1)
		return;

	CString str;
	try {

		//m.hancock - PLID 17834 - 10/28/05 - Check if we have a <No Selection>
		_variant_t var = m_POSCombo->GetValue(m_POSCombo->GetCurSel(),0);
		if(var.vt == VT_NULL)
		{
			//Check if there is a record already
			str.Format("SELECT POSID FROM POSLocationLinkT WHERE HCFASetupGroupID = %li AND LocationID = %li",
			m_HCFASetupGroupID, VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0),-1));
			_RecordsetPtr rs = CreateRecordset(str);
			if (!rs->eof) //A record exists
			{
				//Need to delete the record
				ExecuteSql("DELETE FROM POSLocationLinkT WHERE HCFASetupGroupID = %d AND LocationID = %d",
					m_HCFASetupGroupID, VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0)));
			}
			//Set the selection
			m_POSCombo->CurSel = -1;
			return;
		}
		else
		{
			//Update the record
			str.Format("UPDATE POSLocationLinkT SET POSID = %li WHERE HCFASetupGroupID = %d AND LocationID = %d",
				VarLong(m_POSCombo->GetValue(m_POSCombo->GetCurSel(),0)), m_HCFASetupGroupID, VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0)));
		}
		str.Insert(0, "SET NOCOUNT OFF\r\n"); // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
		GetRemoteData()->Execute(_bstr_t(str),&var,adCmdText);
		if (var.lVal == 0) {
			//Updating failed, so insert a new record
			ExecuteSql("INSERT INTO POSLocationLinkT (HCFASetupGroupID, LocationID, POSID) VALUES ( %d, %d, %li )",
				m_HCFASetupGroupID, VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0)), VarLong(m_POSCombo->GetValue(m_POSCombo->GetCurSel(),0)));
		}

	} NxCatchAll("Error in SavePOSLink");
}

void CEditDefaultPOSCodesDlg::OnRequeryFinishedPlaceofserviceCombo(short nFlags) 
{
	try {
		//m.hancock - PLID 17834 - 10/28/05 - The "Default POS Codes" dialog has no way to unselect
		//a value once it's been selected!  We need to add an entry for "<No Selection>".
		IRowSettingsPtr pRow;
		pRow = m_POSCombo->GetRow(-1);
		_variant_t var;
		var.vt = VT_NULL;
		pRow->PutValue(0,var);
		pRow->PutValue(1,var);
		pRow->PutValue(2,"<No Selection>");
		m_POSCombo->InsertRow(pRow,0);
	} NxCatchAll("Error in OnRequeryFinishedPlaceofserviceCombo");
}
