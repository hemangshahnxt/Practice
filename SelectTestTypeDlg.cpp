// SelectTestTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectTestTypeDlg.h"
#include "EditTestTypesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectTestTypeDlg dialog


CSelectTestTypeDlg::CSelectTestTypeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectTestTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectTestTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectTestTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectTestTypeDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_TEST_TYPE_LABEL, m_nxstaticTestTypeLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectTestTypeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectTestTypeDlg)
	ON_BN_CLICKED(IDC_MODIFY_TEST_TYPES_BTN, OnModifyTestTypesBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectTestTypeDlg message handlers

BOOL CSelectTestTypeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pTestsList = BindNxDataListCtrl(this, IDC_TEST_TYPE, GetRemoteData(), TRUE);

	//m.hancock - 10-26-05 - PLID 16756 - Need to select the first test in the list
	m_pTestsList->PutCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectTestTypeDlg::OnOK() 
{
	long CurSel = m_pTestsList->GetCurSel();
	if(CurSel != -1)
	{
		_variant_t var;
		var = m_pTestsList->GetValue(CurSel,1);
		if(var.vt == VT_I4) {
			m_nSelectedTest = VarLong(var);
		}

		CDialog::OnOK();
	}
	else
		AfxMessageBox("Please select a type for the test that is to be added.");
}

void CSelectTestTypeDlg::OnModifyTestTypesBtn() 
{
	//save old value
	_variant_t var;
	long CurSel = m_pTestsList->GetCurSel();
	if(CurSel != -1)
		var = m_pTestsList->GetValue(CurSel,0);

	CEditTestTypesDlg dlg(this);
	dlg.DoModal();

	m_pTestsList->Requery();

	//load old value
	if(CurSel != -1)
		m_pTestsList->TrySetSelByColumn(0,var);
}
