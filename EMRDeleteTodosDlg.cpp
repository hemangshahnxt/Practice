// EMRDeleteTodosDlg.cpp : implementation file
//
// (c.haag 2008-07-15 09:57) - PLID 30694 - Initial implementation
//

#include "stdafx.h"
#include "patientsrc.h"
#include "EMRDeleteTodosDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
	eclTaskID = 0L,
	eclCategory,
	eclMethod,
	eclPriority,
	eclNotes,
	eclAssignedTo,
	eclDeadline,
	eclDone,
	eclDelete
} EDeleteTodosColumns;

/////////////////////////////////////////////////////////////////////////////
// CEMRDeleteTodosDlg dialog


CEMRDeleteTodosDlg::CEMRDeleteTodosDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRDeleteTodosDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRDeleteTodosDlg)
	//}}AFX_DATA_INIT
}


void CEMRDeleteTodosDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRDeleteTodosDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRDeleteTodosDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRDeleteTodosDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRDeleteTodosDlg message handlers

BOOL CEMRDeleteTodosDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
	
		// Set the button icons
		m_btnOK.AutoSet(NXB_OK);

		// Bind the datalist
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_EMR_TODO_ALARMS, false);
	
		// Set up the Where Clause and requery
		CString strWhere = FormatString("TaskID IN (%s)", ArrayAsString(m_anInputTodoIDs));
		m_dlList->WhereClause = _bstr_t(strWhere);
		m_dlList->Requery();
	}
	NxCatchAll("Error in CEMRDeleteTodosDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEMRDeleteTodosDlg::OnOK() 
{
	try {
		// (c.haag 2008-07-15 10:23) - Build m_anTodoIDsToDelete.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetFirstRow();
		while (NULL != pRow) {
			if (VarBool(pRow->Value[eclDelete])) {
				m_anTodoIDsToDelete.Add( VarLong(pRow->Value[eclTaskID]) );
			}
			pRow = pRow->GetNextRow();
		}
		CNxDialog::OnOK();
	}
	NxCatchAll("Error in CEMRDeleteTodosDlg::OnOK");
}

void CEMRDeleteTodosDlg::OnCancel()
{
	// We don't allow cancelling
}
