// MultipleRevCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultipleRevCodesDlg.h"
#include "AdvRevCodeSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMultipleRevCodesDlg dialog


CMultipleRevCodesDlg::CMultipleRevCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultipleRevCodesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultipleRevCodesDlg)
		m_nServiceID = -1;
		m_bIsInv = FALSE;
	//}}AFX_DATA_INIT
}


void CMultipleRevCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultipleRevCodesDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultipleRevCodesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultipleRevCodesDlg)
	ON_BN_CLICKED(IDC_BTN_ADV_REVCODE_SETUP, OnBtnAdvRevcodeSetup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultipleRevCodesDlg message handlers

BOOL CMultipleRevCodesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 05/01/2008) - PLID 29864 - Set button styles
	m_btnOk.AutoSet(NXB_CLOSE);
	
	m_InsCoList = BindNxDataListCtrl(this,IDC_COMBO_INSCOS,GetRemoteData(),true);
	m_RevCodeList = BindNxDataListCtrl(this,IDC_UB92_REVCODES,GetRemoteData(),true);

	IRowSettingsPtr pRow = m_RevCodeList->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,"");
	pRow->PutValue(2,"{No Code Selected}");
	m_RevCodeList->AddRow(pRow);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMultipleRevCodesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMultipleRevCodesDlg)
	ON_EVENT(CMultipleRevCodesDlg, IDC_COMBO_INSCOS, 16 /* SelChosen */, OnSelChosenComboInscos, VTS_I4)
	ON_EVENT(CMultipleRevCodesDlg, IDC_UB92_REVCODES, 16 /* SelChosen */, OnSelChosenUb92Revcodes, VTS_I4)
	ON_EVENT(CMultipleRevCodesDlg, IDC_COMBO_INSCOS, 18 /* RequeryFinished */, OnRequeryFinishedComboInscos, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMultipleRevCodesDlg::OnSelChosenComboInscos(long nRow) 
{
	Load();
}

void CMultipleRevCodesDlg::OnSelChosenUb92Revcodes(long nRow) 
{
	Save();
}

void CMultipleRevCodesDlg::OnOK() 
{
	Save();
	
	CDialog::OnOK();
}

void CMultipleRevCodesDlg::Save()
{
	if(m_InsCoList->CurSel==-1)
		return;

	CString str;
	try {

		long nInsuranceCompanyID = VarLong(m_InsCoList->GetValue(m_InsCoList->GetCurSel(),0));

		if(m_RevCodeList->CurSel != -1 && VarLong(m_RevCodeList->GetValue(m_RevCodeList->GetCurSel(),0)) != -1) {

			long nRevCodeID = VarLong(m_RevCodeList->GetValue(m_RevCodeList->GetCurSel(),0));

			str.Format(
				"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
				"UPDATE ServiceRevCodesT SET UB92CategoryID = %li WHERE InsuranceCompanyID = %li AND ServiceID = %li",
				nRevCodeID, nInsuranceCompanyID, m_nServiceID);
			_variant_t var;
			GetRemoteData()->Execute(_bstr_t(str),&var,adCmdText);
			if (var.lVal == 0) {
				str.Format("INSERT INTO ServiceRevCodesT (ServiceID, InsuranceCompanyID, UB92CategoryID) VALUES (%li, %li, %li)",
					m_nServiceID, nInsuranceCompanyID, nRevCodeID);
				ExecuteSql(str);
			}
		}
		else {
			//if empty, remove the record
			ExecuteSql("DELETE FROM ServiceRevCodesT WHERE InsuranceCompanyID = %li AND ServiceID = %li",nInsuranceCompanyID,m_nServiceID);
			m_RevCodeList->CurSel = -1;
		}

	} NxCatchAll("Error saving revenue code.");
}

void CMultipleRevCodesDlg::Load()
{
	_RecordsetPtr rs;
	_variant_t var;

	try {

		if(m_InsCoList->CurSel == -1) {
			m_RevCodeList->CurSel = -1;
			return;
		}

		if (m_InsCoList->GetRowCount() == 0) {
			m_RevCodeList->CurSel = -1;
		}
		else {

			long nInsuranceCompanyID = VarLong(m_InsCoList->GetValue(m_InsCoList->GetCurSel(),0));

			rs = CreateRecordset("SELECT UB92CategoryID FROM ServiceRevCodesT WHERE InsuranceCompanyID = %li AND ServiceID = %li",
				nInsuranceCompanyID, m_nServiceID);
			if (!rs->eof) {
				var = rs->Fields->GetItem("UB92CategoryID")->Value;
				if (var.vt == VT_NULL)
					m_RevCodeList->CurSel = -1;
				else
					m_RevCodeList->SetSelByColumn(0, var.lVal);
			}
			else
				m_RevCodeList->CurSel = -1;
			rs->Close();
		}
	}
	NxCatchAll("Error loading revenue code.");

	UpdateData(FALSE);
}

void CMultipleRevCodesDlg::OnRequeryFinishedComboInscos(short nFlags) 
{
	m_InsCoList->CurSel = 0;

	if(m_InsCoList->CurSel != -1) {
		Load();
	}
}

void CMultipleRevCodesDlg::OnBtnAdvRevcodeSetup() 
{
	CAdvRevCodeSetupDlg dlg(this);
	dlg.m_bIsInv = m_bIsInv;
	dlg.DoModal();

	Load();
}
