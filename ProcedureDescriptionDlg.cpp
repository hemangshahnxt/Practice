// ProcedureDescriptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "ProcedureDescriptionDlg.h"
#include "Globalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CProcedureDescriptionDlg dialog


CProcedureDescriptionDlg::CProcedureDescriptionDlg(long nServiceID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CProcedureDescriptionDlg::IDD, pParent)
{
	m_nServiceID = nServiceID;
}


void CProcedureDescriptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureDescriptionDlg)
	DDX_Control(pDX, IDC_PROCEDURE_DESCRIPTION, m_nxeditProcedureDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcedureDescriptionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProcedureDescriptionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcedureDescriptionDlg message handlers

void CProcedureDescriptionDlg::OnOK() 
{
	try {
		//save the description
		CString strDesc;
		GetDlgItemText(IDC_PROCEDURE_DESCRIPTION, strDesc);

		ExecuteSql("UPDATE ServiceT SET ProcedureDescription = '%s' WHERE ID = %li", _Q(strDesc), m_nServiceID);
		
		CDialog::OnOK();
	}NxCatchAll("Error Saving description");
	
	
}

void CProcedureDescriptionDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CProcedureDescriptionDlg::OnInitDialog() 
{
	 CNxDialog::OnInitDialog();
	
	try {
		//load the description
		_RecordsetPtr rsDesc = CreateRecordset("SELECT ProcedureDescription FROM ServiceT WHERE ID = %li", m_nServiceID);

		CString strDesc = AdoFldString(rsDesc, "ProcedureDescription", "");
		SetDlgItemText(IDC_PROCEDURE_DESCRIPTION, strDesc);

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

	}NxCatchAll("Error Loading Description");
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
