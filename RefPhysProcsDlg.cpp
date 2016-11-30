// RefPhysProcsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contactsrc.h"
#include "RefPhysProcsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRefPhysProcsDlg dialog


CRefPhysProcsDlg::CRefPhysProcsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRefPhysProcsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRefPhysProcsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRefPhysProcsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRefPhysProcsDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PROC_WARNING_TEXT, m_nxeditProcWarningText);
	DDX_Control(pDX, IDC_PROC_LIST_CAPTION, m_nxstaticProcListCaption);
	DDX_Control(pDX, IDC_SHOW_PROC_WARNING, m_btnShowProcWarning);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRefPhysProcsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRefPhysProcsDlg)
	ON_BN_CLICKED(IDC_SHOW_PROC_WARNING, OnShowProcWarning)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRefPhysProcsDlg message handlers
using namespace ADODB;

BOOL CRefPhysProcsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_pProcList = BindNxDataListCtrl(this, IDC_REF_PHYS_PROCS, GetRemoteData(), false);
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);	
		// (s.tullis 2014-05-28 13:09) - PLID 56569 - The extra note box on the referring physician proecedures dialog is a multi-lined box, but only allows one line
		m_nxeditProcWarningText.SetLimitText(255);


		CString strFrom;
		strFrom.Format("ProcedureT LEFT JOIN (SELECT * FROM RefPhysProcLinkT WHERE RefPhysID = %li) RefPhysProcLinkQ "
			"ON ProcedureT.ID = RefPhysProcLinkQ.ProcedureID", m_nRefPhysID);
		m_pProcList->FromClause = _bstr_t(strFrom);
		// (c.haag 2008-12-09 17:41) - PLID 32264 - Filter out inactive procedures
		// (c.haag 2008-12-29 9:43) - PLID 32264 - But include ones that were already assigned so that they may be unassigned
		m_pProcList->WhereClause = _bstr_t( FormatString("ProcedureT.Inactive = 0 OR ProcedureT.ID IN (SELECT ProcedureID FROM RefPhysProcLinkT WHERE RefPhysID = %li)", m_nRefPhysID) );
		m_pProcList->Requery();

		_RecordsetPtr rsRefPhys = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name, ShowProcWarning, ProcWarning "
			"FROM PersonT INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID WHERE PersonT.ID = %li", m_nRefPhysID);
		CString strName = AdoFldString(rsRefPhys, "Name");
		SetWindowText(strName);
		SetDlgItemText(IDC_PROC_LIST_CAPTION, "Procedures performed by " + strName + ":");

		CheckDlgButton(IDC_SHOW_PROC_WARNING, AdoFldBool(rsRefPhys, "ShowProcWarning"));
		OnShowProcWarning();
		SetDlgItemText(IDC_PROC_WARNING_TEXT, AdoFldString(rsRefPhys, "ProcWarning", ""));
	}NxCatchAll("Error in CRefPhysProcsDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRefPhysProcsDlg::OnOK() 
{
	try {

		CWaitCursor pWait;

		CString strSql = BeginSqlBatch();
		
		if(IsDlgButtonChecked(IDC_SHOW_PROC_WARNING)) {
			CString strWarning;
			GetDlgItemText(IDC_PROC_WARNING_TEXT, strWarning);
			AddStatementToSqlBatch(strSql, "UPDATE ReferringPhysT SET ShowProcWarning = 1, ProcWarning = '%s' WHERE PersonID = %li", _Q(strWarning), m_nRefPhysID);
		}
		else {
			AddStatementToSqlBatch(strSql, "UPDATE ReferringPhysT SET ShowProcWarning = 0, ProcWarning = NULL WHERE PersonID = %li", m_nRefPhysID);
		}

		//Now clear out the link table, and fill it back in.
		AddStatementToSqlBatch(strSql, "DELETE FROM RefPhysProcLinkT WHERE RefPhysID = %li", m_nRefPhysID);
		for(int i = 0; i < m_pProcList->GetRowCount(); i++) {
			if(VarBool(m_pProcList->GetValue(i, 1))) {
				AddStatementToSqlBatch(strSql, "INSERT INTO RefPhysProcLinkT (RefPhysID, ProcedureID) VALUES (%li, %li)", m_nRefPhysID, VarLong(m_pProcList->GetValue(i,0)));
			}
		}

		ExecuteSqlBatch(strSql);

	}NxCatchAll("Error in CRefPhysProcsDlg::OnOK()");
	CNxDialog::OnOK();
}

void CRefPhysProcsDlg::OnShowProcWarning() 
{
	GetDlgItem(IDC_PROC_WARNING_TEXT)->EnableWindow(IsDlgButtonChecked(IDC_SHOW_PROC_WARNING));
}
